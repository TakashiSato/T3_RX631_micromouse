/**
 * @file  RSPI.c
 * @brief RSPIの挙動を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>
#include "../iodefine.h"
#include "RSPI.h"

/*----------------------------------------------------------------------
	Typedef definitions
 ----------------------------------------------------------------------*/
typedef enum
{   // Values will be used as bit flags.
    RSPI_DO_TX    = 0x1,
    RSPI_DO_RX    = 0x2,
    RSPI_DO_TX_RX = 0x3
} rspi_operation_t;

typedef struct rspi_tcb_s
{
   void     *psrc;
   void     *pdest;
   _UWORD tx_count;
   _UWORD rx_count;
   _UWORD xfr_length;
   _UBYTE  bytes_per_transfer;     /* Source buffer bytes per transfer: 1, 2, or 4. */
   bool     do_rx_now;              /* State flag for valid read data available. */
   bool     do_tx;                  /* State flag for transmit operation. */
   rspi_operation_t  transfer_mode; /* Transmit only, receive only, or transmit-receive. */

   void	(*ChipSelectFunc)(void);
   void	(*ChipDeselectFunc)(void);
} rspi_tcb_t;

/*----------------------------------------------------------------------
	Private global variables
 ----------------------------------------------------------------------*/
//static volatile _UINT g_rxdata[RSPI_NUM_SEQUENCES]; /* Space for fast read of RSPI RX data register. */
//static rspi_tcb_t g_rspi_tcb[RSPI_NUM_SEQUENCES] = {0};
static volatile _UINT g_rxdata; /* Space for fast read of RSPI RX data register. */
static rspi_tcb_t g_rspi_tcb = {0};

// Cycle Operation 関連
static bool spiCycleoperationFlag = false;		// Cycle Operation実行フラグ
static _UBYTE g_registerdCycleDeviceNum = 0;	// Cycle Operationで処理するデバイスの数
static _UBYTE g_cycleTargetPtr = 0;				// Cycle Operationで現在処理するデバイスのナンバー
static void (*g_cycleSendFunc[])(void);			// Cycle Operationでデバイスにコマンドを送る関数


/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
// RSPI0の初期化をし，使用可能状態とする
static void RSPI0_Open(void);
// 読み書きを行う基幹関数
static void RSPI0_WriteReadCommon(void *psrc,
								  void *pdest,
								  _UWORD length,
								  RSPI_DEPENDENCE dep,
								  rspi_operation_t tx_rx_mode);
// RSPI0の割り込み優先度設定
static void RSPI0_IRPrioritySet(_UWORD priority);
// RSPI0の割り込みフラグクリア
static void RSPI0_InterruptsClear(void);
// RSPI0割り込みの有効/無効化
static void RSPI0_InterruptsEnable(bool enabled);
// RSPI0送受信基幹関数
static void RSPI0_TxRxCommon(void);
// Cycle Operation 実行関数
static void RSPI0_CycleOperation(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/** RSPI0を初期化する
 * @param void
 * @retval void
 */
void InitializeRSPI0(void)
{
	RSPI0_Open();
}

/** RSPI0受信データフル割り込み
 * @param void
 * @retval void
 */
void Int_SPRI0(void)
{
	g_rxdata = RSPI0.SPDR.LONG; // Need to read RX data reg ASAP.
	g_rspi_tcb.rx_count++;
	RSPI0_TxRxCommon();
}

/** RSPI0送信データエンプティ割り込み
 * @param void
 * @retval void
 */
void Int_SPTI0(void)
{
	g_rxdata = RSPI0.SPDR.LONG; // Read rx-data register into temp buffer.

	/* If master mode then disable further spti interrupts on first transmit.
	   If slave mode then we do two transmits to fill the double buffer,
	   then disable spti interrupts.
	   The receive interrupt will handle any remaining data. */
	if ((RSPI0.SPCR.BIT.MSTR) || (g_rspi_tcb.tx_count > 0))
	{
	    RSPI0.SPCR.BIT.SPTIE = 0;  /* Disable SPTI interrupt. */
	}

	RSPI0_TxRxCommon();         // Process the data in the common handler.

	if (g_rspi_tcb.transfer_mode & RSPI_DO_RX)
	{    /* Count was incremented in the call to rspi_tx_rx_common. */
	    if ((RSPI0.SPCR.BIT.MSTR) || (g_rspi_tcb.tx_count > 1))
	    {
	        g_rspi_tcb.do_rx_now = true; /* Enables saving of receive data on next receive interrupt. */
	    }
	}
}

/** RSPI0アイドル割り込み
 * @param void
 * @retval void
 */
void Int_SPII0(void)
{

}

/** RSPI0エラー割り込み
 * @param void
 * @retval void
 */
void Int_SPEI0(void)
{
	Printf("SPI0 Error\n");

	/* Identify and clear error condition. */
	if(RSPI0.SPSR.BIT.OVRF) // Overrun error.
	{
	    /* Clear error source: OVRF flag. */
	    RSPI0.SPSR.BIT.OVRF = 0;
	}
	else if (RSPI0.SPSR.BIT.MODF)
	{
	    /* Clear error source: MODF flag. */
	    RSPI0.SPSR.BIT.MODF = 0;
	}
	else if (RSPI0.SPSR.BIT.PERF)
	{
	    /* Clear error source: PERF flag. */
	    RSPI0.SPSR.BIT.PERF = 0;
	}

	/* Disable the RSPI channel (terminates the transfer operation). */
	RSPI0.SPCR.BIT.SPRIE = 0;  /* Disable SPRI interrupt. */
	RSPI0.SPCR.BIT.SPE   = 0;  /* Disable RSPI. */
}

/** RSPI0で読み込みを行う
 * @param *pdest: 読み込みデータ列を格納する配列へのポインタ
 * @param length: 読み込みデータ列の配列の大きさ
 * @retval void
 */
void RSPI0_Read(void *pdest,
        		_UWORD length,
        		RSPI_DEPENDENCE dep)
{
	RSPI0_WriteReadCommon(NULL, pdest, length, dep, RSPI_DO_RX);
}

/** RSPI0で書き込みを行う
 * @param *psrc: 書き込みデータ列を格納する配列へのポインタ
 * @param length: 書き込みデータ列の配列の大きさ
 * @retval void
 */
void RSPI0_Write(void *psrc,
        		 _UWORD length,
        		 RSPI_DEPENDENCE dep)
{
	RSPI0_WriteReadCommon(psrc, NULL, length, dep, RSPI_DO_TX);
}

/** RSPI0で読み書きを行う
 * @param *psrc: 書き込みデータ列を格納する配列へのポインタ
 * @param *pdest: 読み込みデータ列を格納する配列へのポインタ
 * @param length: 書き込みデータ列の配列の大きさ
 * @retval void
 */
void RSPI0_WriteRead(void *psrc,
        			 void *pdest,
        			 _UWORD length,
        			 RSPI_DEPENDENCE dep)
{
	RSPI0_WriteReadCommon(psrc, pdest, length, dep, RSPI_DO_TX_RX);
}

/** Cycle Operation用のコマンドを登録する
 * @param *cyleSendFunc: コマンドを送信する関数へのポインタ(必ずRSPI0_Read,Write,WriteReadを含むこと!)
 * @retval void
 */
void RSPI0_RegisterDeviceForCycleOperation(void (*cycleSendFunc)(void))
{
	g_cycleSendFunc[g_registerdCycleDeviceNum++] = cycleSendFunc;
}

/** CycleOperationを開始する
 * @param void
 * @retval void
 */
void RSPI0_StartCycleOperation(void)
{
	if(spiCycleoperationFlag == false)
	{
		spiCycleoperationFlag = true;
		g_cycleTargetPtr = 0;
		g_cycleSendFunc[0]();
	}
}

/** CycleOperationを停止する
 * @param void
 * @retval void
 */
void RSPI0_StopCycleOperation(void)
{
	spiCycleoperationFlag = false;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** RSPI0の初期化をし，使用可能状態とする
 * @param void
 * @retval void
 */
static void RSPI0_Open(void)
{
	MSTP(RSPI0) = 0;			// RSPIのモジュールストップ状態の解除

	RSPI0.SPCR.BIT.SPE = 0x00;	// RSPI0の機能を無効化

	RSPI0.SPPCR.BYTE = 0x00;	// ループバック無効

	// ビットレートの設定
	RSPI0.SPBR = 2;

	// コントロールレジスタの設定
	RSPI0.SPDCR.BIT.SPFC = 0;	// フレーム数:1フレーム
	RSPI0.SPDCR.BIT.SPRDTD = 0;	// SPDRレジスタは受信バッファを読み出す
	RSPI0.SPDCR.BIT.SPLW = 1;	// SPDRレジスタへはLONGアクセス

	// 遅延の設定
	RSPI0.SPCKD.BIT.SCKDL = 7;	// RSPCK遅延: 8RSPCK
	RSPI0.SSLND.BIT.SLNDL = 7;	// SSLネゲート遅延: 8RSPCK
	RSPI0.SPND.BIT.SPNDL = 7;	// 次アクセス遅延: 8RSPCK+2PCLK

	// パリティの設定
	RSPI0.SPCR2.BYTE = 0x00;	// パリティなし

	// ピン機能設定
	PORT1.PMR.BIT.B7 = 0;			// P17を汎用入出力ポートに設定
	PORTC.PMR.BIT.B6 = 0;			// PC6を汎用入出力ポートに設定
	PORTC.PMR.BIT.B5 = 0;			// PC5を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;			// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;			// PFSレジスタへの書き込みを許可
	MPC.P17PFS.BIT.PSEL = 13;		// P17の機能選択:MISOA(RSPI0MISO)
	MPC.PC6PFS.BIT.PSEL = 13;		// PC6の機能選択:MOSIA(RSPI0MOSI)
	MPC.PC5PFS.BIT.PSEL = 13;		// PC5の機能選択:RSPCKA(RSPI0CLK)
	MPC.PWPR.BYTE = 0x80;			// PFSレジスタ,PFSWEビットへの書き込みを禁止
	PORT1.PMR.BIT.B7 = 1;			// P17を周辺機能として使用
	PORTC.PMR.BIT.B6 = 1;			// PC6を周辺機能として使用
	PORTC.PMR.BIT.B5 = 1;			// PC5を周辺機能として使用

	// RSPI制御レジスタの設定
	RSPI0.SPCR.BIT.SPMS = 1;		// 0:SPI動作(4線式), 1:クロック同期式動作(3線式)
	RSPI0.SPCR.BIT.TXMD = 0;		// 全二重同期式シリアル通信
	RSPI0.SPCR.BIT.MODFEN = 0;		// モードフォルトエラー検出を禁止
	RSPI0.SPCR.BIT.MSTR = 1;		// マスタモード
	RSPI0.SPSCR.BYTE = 0;			// シーケンス制御無効

	RSPI0_IRPrioritySet(10);
	RSPI0_InterruptsEnable(false);
	RSPI0_InterruptsClear();
}

/** 読み書きを行う基幹関数
 * @param *psrc: 書き込みデータ列を格納する配列へのポインタ
 * @param *pdest: 読み込みデータ列を格納する配列へのポインタ
 * @param length: 書き込みデータ列の配列の大きさ
 * @param tx_rx_mode: 読み書きモード
 * @retval void
 */
static void RSPI0_WriteReadCommon(void *psrc,
								  void *pdest,
								  _UWORD length,
								  RSPI_DEPENDENCE dep,
								  rspi_operation_t tx_rx_mode)
{

	RSPI0_InterruptsEnable(false);		// 割り込み無効化

//	g_seq = dep.sequence_number;

	g_rspi_tcb.xfr_length = length;
    g_rspi_tcb.tx_count = 0;
    g_rspi_tcb.rx_count = 0;
    g_rspi_tcb.bytes_per_transfer = dep.format;
    g_rspi_tcb.psrc = psrc;
    g_rspi_tcb.pdest = pdest;
    g_rspi_tcb.transfer_mode = tx_rx_mode;
    g_rspi_tcb.ChipSelectFunc = dep.ChipSelectFunc;
    g_rspi_tcb.ChipDeselectFunc = dep.ChipDeselectFunc;
//    g_rspi_tcb.CycleSendCommand = dep.CycleSendCommand;

    if (tx_rx_mode & RSPI_DO_TX)
    {
        g_rspi_tcb.do_tx = true;
    }
    else
    {
        g_rspi_tcb.do_tx = false;
    }

    g_rspi_tcb.do_rx_now = false;  // Initialize receive state flag.

    /* Wait for channel to be idle before making changes to registers. */
    while (RSPI0.SPSR.BIT.IDLNF)
    {
    }

	RSPI0.SPCMD0.BIT.CPHA = 	dep.cpha_val;
	RSPI0.SPCMD0.BIT.CPOL = 	dep.cpol_val;
	RSPI0.SPCMD0.BIT.SSLKP = 	dep.sslkp_val;
	RSPI0.SPCMD0.BIT.SPB = 	    dep.spb_val;
	RSPI0.SPCMD0.BIT.LSBF =		dep.lsbf_val;
	RSPI0.SPCMD0.BIT.SPNDEN =	dep.spnden_val;
	RSPI0.SPCMD0.BIT.SLNDEN = 	dep.slnden_val;
	RSPI0.SPCMD0.BIT.SCKDEN = 	dep.sckden_val;

	// フラグクリア
	RSPI0.SPSR.BYTE &= 0xFD;

    RSPI0_InterruptsClear();
    RSPI0_InterruptsEnable(true);

    RSPI0.SPCR2.BIT.SPIIE = 0; 		// RSPI0アイドル割り込み
	RSPI0.SPCR.BIT.SPEIE = 1;		// RSPI0エラー割り込み
	RSPI0.SPCR.BIT.SPTIE = 1;		// RSPI0送信割り込み
	RSPI0.SPCR.BIT.SPRIE = 1;		// RSPI0受信割り込み

	RSPI0.SPCR.BIT.SPE = 1;		// RSPI0の機能を有効化
}

/** RSPI0の割り込み優先度設定
 * @param priority: 割り込み優先度(0-15)
 * @retval void
 */
static void RSPI0_IRPrioritySet(_UWORD priority)
{
	IPR(RSPI0, SPTI0) = priority;
	IPR(RSPI0, SPRI0) = priority;
	IPR(RSPI0, SPII0) = priority;
	IPR(ICU, GROUP12) = priority;
}

/** RSPI0の割り込みフラグクリア
 * @param void
 * @retval void
 */
static void RSPI0_InterruptsClear(void)
{
	IR(RSPI0, SPTI0) = 0;
	IR(RSPI0, SPRI0) = 0;
	IR(RSPI0, SPII0) = 0;
	IR(ICU, GROUP12) = 0;
}

/** RSPI0割り込みの有効/無効化
 * @param enabled: true=enable, false=disable
 * @retval void
 */
static void RSPI0_InterruptsEnable(bool enabled)
{
	IEN(RSPI0, SPTI0) = enabled;
	IEN(RSPI0, SPRI0) = enabled;
	IEN(RSPI0, SPII0) = enabled;
	IEN(ICU, GROUP12) = enabled;	// For SPEI
}

/** RSPI0送受信基幹関数
 * @param void
 * @retval void
 */
static void RSPI0_TxRxCommon(void)
{
	void*    psrc      = g_rspi_tcb.psrc;
	void*    pdest     = g_rspi_tcb.pdest;
	_UWORD   tx_count  = g_rspi_tcb.tx_count;
	_UWORD   rx_count  = g_rspi_tcb.rx_count;
	_UBYTE   data_size = g_rspi_tcb.bytes_per_transfer;
	_UINT    rx_data   = g_rxdata;

	/* Service the hardware first to keep it busy. */
	/* Feed the TX. */
	if(tx_count < g_rspi_tcb.xfr_length)   /* Don't write transmit buffer more than length. */
	{
		if (tx_count == 0)
		{
			g_rspi_tcb.ChipSelectFunc();
		}
	    if (g_rspi_tcb.do_tx)
	    {
	        /* Transmit the data. TX data register accessed in long words. */
	        if (RSPI_BYTE_DATA == data_size)
	        {
	            RSPI0.SPDR.LONG = ((_UBYTE *)psrc)[tx_count];
	        }
	        else if(RSPI_WORD_DATA == data_size)
	        {
	            RSPI0.SPDR.LONG = ((_UWORD *)psrc)[tx_count];
	        }
	        else // Must be long data. if(RSPI_LONG_DATA == data_size)
	        {
	            RSPI0.SPDR.LONG = ((_UINT *)psrc)[tx_count];
	        }
	    }
	    else /* Must be RX only mode, so transmit dummy data for clocking.*/
	    {
	        /* TX data register accessed in long words. */
	        RSPI0.SPDR.LONG = RSPI_DUMMY_TXDATA;
	    }
	    g_rspi_tcb.tx_count++;
	}

	/* Store the received data in user buffer.
	 * Receive data not valid until after first transmission is complete. */
	if (g_rspi_tcb.do_rx_now)
	{
	    if (RSPI_BYTE_DATA == data_size)
	    {
	        ((_UBYTE *)pdest)[rx_count-1] = (_UBYTE)rx_data;
	    }
	    else if(RSPI_WORD_DATA == data_size)
	    {
	        ((_UWORD *)pdest)[rx_count-1] = (_UWORD)rx_data;
	    }
	    else  // Must be long data. if(RSPI_LONG_DATA == data_size)
	    {
	        ((_UINT *)pdest)[rx_count-1] = rx_data;
	    }
	}

	/* Check for last data.  */
	 if(rx_count == g_rspi_tcb.xfr_length)
	 {   /* Last data was transferred. */
	     RSPI0.SPCR.BIT.SPRIE = 0;  /* Disable SPRI interrupt. */
	     RSPI0.SPCR.BIT.SPE   = 0;  /* Disable RSPI. */

	     g_rspi_tcb.ChipDeselectFunc();

	     // Cycle Operation フラグがtrueなら，SPI動作を再開し次のデバイスにサイクル処理用コマンドを送信
	     if (spiCycleoperationFlag == true)
	     {
	    	 RSPI0_CycleOperation();
	     }
	 }
}

/** Cycle Operation 実行関数
 * @param void
 * @retval void
 */
static void RSPI0_CycleOperation(void)
{
	// ターゲットを次のデバイスに変更する
	if (++g_cycleTargetPtr >= g_registerdCycleDeviceNum)
	{
		g_cycleTargetPtr = 0;
	}
	// デバイスにコマンドを送る
	g_cycleSendFunc[g_cycleTargetPtr]();
}

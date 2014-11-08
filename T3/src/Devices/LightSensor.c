/**
 * @file  LightSensor.c
 * @brief 光センサの動作を司るクラス
 */
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "LightSensor.h"
#include "../iodefine.h"
#include "../Global.h"
#include "../Peripherals/Timer.h"
#include "../Peripherals/SerialPort.h"

/*----------------------------------------------------------------------
	Private global variables
 ----------------------------------------------------------------------*/
static _UBYTE _tp = 0;	// タスクポインタ
static _UBYTE _sensorCh = 0;	// 処理対象センサチャンネル
static _UWORD _ledOffAdVal[4];	// フィルタ用LEDオフ時A/D値格納
static _UWORD _ledOnAdVal[4];	// フィルタ用LEDオン時A/D値格納

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void LightSensor_InitializePort(void);
static void LightSensor_InitializeADC(void);
static void LightSensor_InitializeDMAC(void);
static void LightSensor_InitializeMTU3(void);
static void LightSensor_SetLedState(_UBYTE ch, E_IR_LED_STATE state);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/* 光センサの初期化
 * @param void
 * @retval void
 */
void LightSensor_Initialize(void)
{
	LightSensor_InitializePort();		// ポートの初期化
	LightSensor_InitializeDMAC();		// DMACの初期化
	LightSensor_InitializeADC();		// 12ビットA/Dコンバータの初期化
	LightSensor_InitializeMTU3();		// MTU3の初期化
}

/** DMA転送終了割り込み
 * @param void
 * @retval void
 */
void LightSensor_IntDMAC0(void)
{
	MTU.TSTR.BIT.CST3 = 0;		// MTU3カウント動作停止
	MTU3.TCNT = 0;				// タイマカウンタの初期化

//	LED発光からフォトトラ反応が安定するまで約50～60us
//	かかるので、case:偶数 ではその待ちを設定。
	switch(_tp)
	{
	case 0:
	case 2:
	case 4:
	case 6:
		LightSensor_SetLedState(_sensorCh, IR_LED_ON);	// IRLED点灯
		DMAC0.DMDAR = (void*)&(_ledOnAdVal[_sensorCh]);	// DMAC転送先アドレス設定
		MTU3.TGRA = PT_DELAY;							// LEDを発光させて少ししてからA/D変換を開始させる
		break;
	case 1:
		LightSensor_SetLedState(_sensorCh, IR_LED_OFF);	// IRLED消灯
		S12AD.ADANS0.WORD = 0x0002;						// AN001を変換対象とする
		DMAC0.DMSAR = (void*)&S12AD.ADDR1;				// DMAC転送元アドレス設定:AN001
		_sensorCh = 1;
		DMAC0.DMDAR = (void*)&(_ledOffAdVal[_sensorCh]);// DMAC転送先アドレス設定
		MTU3.TGRA = GET_INTERVAL;
		break;
	case 3:
		LightSensor_SetLedState(_sensorCh, IR_LED_OFF);	// IRLED消灯
		S12AD.ADANS0.WORD = 0x0004;						// AN002を変換対象とする
		DMAC0.DMSAR = (void*)&S12AD.ADDR2;				// DMAC転送元アドレス設定:AN002
		_sensorCh = 2;
		DMAC0.DMDAR = (void*)&(_ledOffAdVal[_sensorCh]);// DMAC転送先アドレス設定
		MTU3.TGRA = GET_INTERVAL;
		break;
	case 5:
		LightSensor_SetLedState(_sensorCh, IR_LED_OFF);	// IRLED消灯
		S12AD.ADANS0.WORD = 0x0040;						// AN006を変換対象とする
		DMAC0.DMSAR = (void*)&S12AD.ADDR6;				// DMAC転送元アドレス設定:AN006
		_sensorCh = 3;
		DMAC0.DMDAR = (void*)&(_ledOffAdVal[_sensorCh]);// DMAC転送先アドレス設定
		MTU3.TGRA = GET_INTERVAL;
		break;
	case 7:
		LightSensor_SetLedState(_sensorCh, IR_LED_OFF);	// IRLED消灯
		S12AD.ADANS0.WORD = 0x0001;						// AN000を変換対象とする
		DMAC0.DMSAR = (void*)&S12AD.ADDR0;				// DMAC転送元アドレス設定:AN000
		_sensorCh = 0;
		DMAC0.DMDAR = (void*)&(_ledOffAdVal[_sensorCh]);// DMAC転送先アドレス設定
		MTU3.TGRA = GET_INTERVAL;
		break;
	default:
		break;
	}

	// タスクポインタを進める
	_tp++;
	if (_tp >= 8)
	{
		_tp = 0;
	}

	// 次のA/D値転送のためのDMAC設定
	DMAC0.DMSTS.BIT.DTIF = 0;	// DMAC0転送終了割り込みフラグをクリア
	DMAC0.DMCRA = 1;			// 転送回数:1
	DMAC0.DMCNT.BIT.DTE = 1;	// DMA転送を許可

	MTU.TSTR.BIT.CST3 = 1;		// MTU3カウント動作開始
}

/** AD値取得
 * @param ch: 取得するチャンネル
 * @retval _SWORD: AD値
 */
_SWORD LightSensor_GetADValue(_UBYTE ch)
{
	return (_SWORD)(_ledOnAdVal[ch]) - _ledOffAdVal[ch];
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** LightSensorで使うポートの初期化
 * @param void
 * @retval void
 */
static void LightSensor_InitializePort(void)
{
	// LED駆動端子
	PORTA.PMR.BIT.B1 = 0;		// PA1を汎用入出力ポートに設定
	PORTA.PMR.BIT.B3 = 0;		// PA3を汎用入出力ポートに設定
	PORTA.PMR.BIT.B4 = 0;		// PA4を汎用入出力ポートに設定
	PORTA.PMR.BIT.B6 = 0;		// PA6を汎用入出力ポートに設定
	PORTA.PDR.BIT.B1 = 1;		// PA1を出力ポートに設定
	PORTA.PDR.BIT.B3 = 1;		// PA3を出力ポートに設定
	PORTA.PDR.BIT.B4 = 1;		// PA4を出力ポートに設定
	PORTA.PDR.BIT.B6 = 1;		// PA6を出力ポートに設定
	PORTA.PODR.BIT.B1 = 0;		// PA1をLow出力に設定
	PORTA.PODR.BIT.B3 = 0;		// PA3をLow出力に設定
	PORTA.PODR.BIT.B4 = 0;		// PA4をLow出力に設定
	PORTA.PODR.BIT.B6 = 0;		// PA6をLow出力に設定

	// アナログ入力端子
	PORT4.PMR.BIT.B0 = 0;		// P40を汎用入出力ポートに設定
	PORT4.PMR.BIT.B1 = 0;		// P41を汎用入出力ポートに設定
	PORT4.PMR.BIT.B2 = 0;		// P42を汎用入出力ポートに設定
	PORT4.PMR.BIT.B6 = 0;		// P46を汎用入出力ポートに設定
	PORT4.PDR.BIT.B0 = 0;		// P40を入力ポートに設定
	PORT4.PDR.BIT.B1 = 0;		// P41を入力ポートに設定
	PORT4.PDR.BIT.B2 = 0;		// P42を入力ポートに設定
	PORT4.PDR.BIT.B6 = 0;		// P46を入力ポートに設定

	// ピン機能設定
	MPC.PWPR.BIT.B0WI = 0;		// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;		// PFSレジスタへの書き込みを許可
	MPC.P40PFS.BIT.ASEL = 1;	// P40をアナログ端子(AN000)として使用
	MPC.P41PFS.BIT.ASEL = 1;	// P41をアナログ端子(AN001)として使用
	MPC.P42PFS.BIT.ASEL = 1;	// P42をアナログ端子(AN002)として使用
	MPC.P46PFS.BIT.ASEL = 1;	// P46をアナログ端子(AN006)として使用
	MPC.PWPR.BYTE = 0x80;		// PFSレジスタ,PFSWEビットへの書き込みを禁止
}

/* 12ビットA/Dコンバータの初期化
 * @param void
 * @retval void
 */
static void LightSensor_InitializeADC(void)
{
	MSTP(S12AD) = 0;			// 12ビットADCモジュールストップ状態の解除

	// ADCの設定
	S12AD.ADCSR.BIT.EXTRG = 0;	// 同期トリガによるA/D変換の開始を選択
	S12AD.ADCSR.BIT.TRGE = 1;	// 同期，非同期トリガによるA/D変換の開始を許可
	S12AD.ADCSR.BIT.CKS = 3;	// A/D変換クロック(3:PCLK)
	S12AD.ADCSR.BIT.ADIE = 1;	// スキャン終了後の割り込み許可
	S12AD.ADCSR.BIT.ADCS = 0;	// モード選択(0:シングル，1:連続)
	S12AD.ADCER.BIT.ADRFMT = 0;	// ADDRレジスタのフォーマット:右づめ

	// A/D変換開始トリガの選択
	S12AD.ADSTRGR.BIT.ADSTRS = 3;	// MTUn.TGRAとMTUn.TCNTをトリガとする

	// 割り込みレベル設定
	IPR(S12AD, S12ADI0)= 5;

	// 割り込み要求を許可
	IEN(S12AD, S12ADI0) = 1;

	// A/Dチャネル設定
	S12AD.ADANS0.WORD = 0x0001;	// AN000を変換対象とする
}

/* DMACの初期化
 * @param void
 * @retval void
 */
static void LightSensor_InitializeDMAC(void)
{
	MSTP(DMAC) = 0;						// DAMコントローラのモジュールストップ状態の解除

	DMAC.DMAST.BIT.DMST = 0;			// DMAC起動を禁止
	DMAC0.DMCNT.BIT.DTE = 0;			// DMA転送を禁止

	DTC.DTCCR.BIT.RRS = 0;				// 転送情報リードスキップに対するフラグをリセット
	ICU.DMRSR0 = VECT_S12AD_S12ADI0;	// DMA起動要因:S12AD0 ADI0
	DMAC0.DMAMD.WORD = 0x0000;			// 転送先・転送元アドレス固定
	DMAC0.DMTMD.WORD = 0x2101;			// ノーマル転送,16ビット転送,周辺モジュール割り込みトリガ
	DMAC0.DMSAR = (void*)&S12AD.ADDR0;	// 転送元アドレス設定
	DMAC0.DMDAR = (void*)&(_ledOffAdVal)[0];	// 転送先アドレス設定
	DMAC0.DMCSL.BIT.DISEL = 0;			// 転送開始時に起動要因の割り込みフラグをクリア
	DMAC0.DMCRA = 1;					// 転送回数:1

	// 割り込み設定
	DMAC0.DMINT.BIT.DTIE = 1;			// 転送終了割り込みを許可

	// 割り込みレベル設定
	IPR(DMAC, DMAC0I)= 8;

	// 割り込み要求を許可
	IEN(DMAC, DMAC0I) = 1;

	DMAC.DMAST.BIT.DMST = 1;			// DMAC起動を許可
	DMAC0.DMCNT.BIT.DTE = 1;			// DMA転送を許可
}

/** MTU3の初期化
 * @param void
 * @retval void
 */
static void LightSensor_InitializeMTU3(void)
{
	MSTP(MTU3) = 0;				// モジュールストップ状態の解除

	// タイマカウンタの初期化
	MTU3.TCNT = 0;

	// カウンタクロックの選択
	MTU3.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	MTU3.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち上がりエッジでカウント

	// カウンタクリア要因の選択
	MTU3.TCR.BIT.CCLR = 1;		// TGRAコンペアマッチでTCNTカウンタをクリア

	// タイマモードの選択
	MTU3.TMDR.BIT.MD = 0;		// ノーマルモード

	// TGRyの設定
	MTU3.TGRA = GET_INTERVAL;

	// 割り込み許可設定
	MTU3.TIER.BIT.TTGE = 1;		// A/D変換開始要求の発生を許可

	// カウント動作開始
	MTU.TSTR.BIT.CST3 = 1;		// MTU3カウント動作開始
}

/** 赤外LED発光状態の設定
 * @param ch: 設定するチャンネル
 * @param state: 設定する状態
 * @retval void
 */
static void LightSensor_SetLedState(_UBYTE ch, E_IR_LED_STATE state)
{
	switch(ch)
	{
	case 0:
		IR_LED0 = state;
		break;
	case 1:
		IR_LED1 = state;
		break;
	case 2:
		IR_LED2 = state;
		break;
	case 3:
		IR_LED3 = state;
		break;
	default:
		break;
	}
}

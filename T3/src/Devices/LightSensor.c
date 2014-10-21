/**
 * @file  LightSensor.c
 * @brief 光センサの動作を司るクラス
 */
/*----------------------------------------------------------------------
 インクルード
 ----------------------------------------------------------------------*/
#include "LightSensor.h"
#include "../iodefine.h"
#include "../Global.h"
#include "../Peripherals/Timer.h"
#include "../Peripherals/SerialPort.h"

/*----------------------------------------------------------------------
 パブリックメソッド
 ----------------------------------------------------------------------*/
/* 光センサの初期化
 * @param void
 * @retval void
 */
void InitializeLightSensor(void)
{
	InitializeDMAC();		// DMACの初期化
	InitializeADC();		// 12ビットA/Dコンバータの初期化
	InitializeCMT0();		// CMT0の初期化
}

/** CMT0コンペアマッチによる割り込み
 * @param void
 * @retval void
 */
void IntCMT0(void)
{
//	if(PORTE.PODR.BIT.B3)
//	{
//		PORTE.PODR.BIT.B3 = 0;
//	}
//	else
//	{
//		PORTE.PODR.BIT.B3 = 1;		// IRLED点灯
//		WaitUS(50);					// 少し待つ
//
//		S12AD.ADCSR.BIT.ADST = 1;	// A/D変換開始
//	}

	// 電圧監視
	float bat = (float)ad/4096*3*2;
	if(bat < 3.8)
	{
		Printf("!!!LOW BATTERY!!!\n");
		PORT2.PODR.BIT.B7 = ~PORT2.PODR.BIT.B7;
	}

}

/*----------------------------------------------------------------------
 プライベートメソッド
 ----------------------------------------------------------------------*/
/* 12ビットA/Dコンバータの初期化
 * @param void
 * @retval void
 */
static void InitializeADC(void)
{
	MSTP(S12AD) = 0;			// 12ビットADCモジュールストップ状態の解除

	// ADCの設定
	S12AD.ADCSR.BIT.EXTRG = 0;	// 同期トリガによるA/D変換の開始を選択
	S12AD.ADCSR.BIT.TRGE = 0;	// 同期，非同期トリガによるA/D変換の開始を禁止
	S12AD.ADCSR.BIT.CKS = 3;	// A/D変換クロック(3:PCLK)
	S12AD.ADCSR.BIT.ADIE = 1;	// スキャン終了後の割り込み許可
	S12AD.ADCSR.BIT.ADCS = 1;	// モード選択(0:シングル，1:連続)
	S12AD.ADCER.BIT.ADRFMT = 0;	// ADDRレジスタのフォーマット:右づめ

	// 割り込みレベル設定
	IPR(S12AD, S12ADI0)= 8;

	// 割り込み要求を許可
	IEN(S12AD, S12ADI0) = 1;

	// A/Dチャネル設定
	S12AD.ADANS0.WORD = 0x0001;	// AN000を変換対象とする

	// ピン機能設定
	PORT4.PMR.BIT.B0 = 0;		// P40を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;		// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;		// PFSレジスタへの書き込みを許可
	MPC.P40PFS.BIT.ASEL = 1;	// P40をアナログ端子として使用
	MPC.PWPR.BYTE = 0x80;		// PFSレジスタ,PFSWEビットへの書き込みを禁止

	S12AD.ADCSR.BIT.ADST = 1;	// A/D変換開始
}

/* DMACの初期化
 * @param void
 * @retval void
 */
static void InitializeDMAC(void)
{
	MSTP(DMAC) = 0;						// DAMコントローラのモジュールストップ状態の解除

	DMAC.DMAST.BIT.DMST = 0;			// DMAC起動を禁止
	DMAC0.DMCNT.BIT.DTE = 0;			// DMA転送を禁止

	DTC.DTCCR.BIT.RRS = 0;				// 転送情報リードスキップに対するフラグをリセット
	ICU.DMRSR0 = VECT_S12AD_S12ADI0;	// DMA起動要因:S12AD0 ADI0
	DMAC0.DMAMD.WORD = 0x0000;			// 転送先・転送元アドレス固定
	DMAC0.DMTMD.WORD = 0x2101;			// ノーマル転送,16ビット転送,周辺モジュール割り込みトリガ
	DMAC0.DMSAR = (void*)&S12AD.ADDR0;	// 転送元アドレス設定
	DMAC0.DMDAR = (void*)&ad;			// 転送先アドレス設定
	DMAC0.DMCRA = 0;					// フリーランニングモード
	DMAC0.DMCSL.BIT.DISEL = 0;			// 転送開始時に起動要因の割り込みフラグをクリア

	DMAC.DMAST.BIT.DMST = 1;			// DMAC起動を許可
	DMAC0.DMCNT.BIT.DTE = 1;			// DMA転送を許可
}

/** コンペアマッチタイマ0(CMT0)の初期化
 * @param void
 * @retval void
 */
static void InitializeCMT0(void)
{
	MSTP(CMT0) = 0;						// CMTユニット0(CMT0, CMT1)モジュールストップ状態の解除

	CMT.CMSTR0.BIT.STR0 = 0;			// CMT0.CMCNTカウンタのカウント動作停止
	// カウントクロック
	// (0:8分周, 1:32分周, 2:128分周, 3:512分周)
	CMT0.CMCR.BIT.CKS = 3;				// カウントクロック
	CMT0.CMCR.BIT.CMIE = 1;				// コンペアマッチ割り込みの許可
	CMT0.CMCOR = 46875;					// コンペアマッチ周期:46875(=500mSec)
	CMT0.CMCNT = 0;						// タイマカウンタの初期化

	// 割り込みレベル設定
	IPR(CMT0, CMI0)= 6;

	// 割り込み要求を許可
	IEN(CMT0, CMI0) = 1;

	CMT.CMSTR0.BIT.STR0 = 1;			// CMT0.CMCNTカウンタのカウント動作開始
}

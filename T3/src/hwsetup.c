/**
 * @file  hwsetup.c
 * @brief ハードウェアセットアップ
 */
/*----------------------------------------------------------------------
 インクルード
 ----------------------------------------------------------------------*/
#include <machine.h>
#include "iodefine.h"

/*----------------------------------------------------------------------
 プロトタイプ宣言
 ----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
#endif
void HardwareSetup(void);
static void ClockInit(void);
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------
	ソースコード
----------------------------------------------------------------------*/
/** ハードウェアセットアップ関数
 * @param void
 * @retval void
 */
void HardwareSetup(void)
{
	ClockInit();
}

/** クロック初期設定
 * @param void
 * @retval void
 */
void ClockInit(void)
{
	volatile unsigned int i;

	SYSTEM.PRCR.WORD = 0xA50B;		// プロテクトレジスタ解除


	/* 48ピンパッケージではサブクロック・RTCを使用できないので
	 * それらの初期化処理を行う
	 */
	SYSTEM.SOSCCR.BIT.SOSTP = 1;	// サブクロック発振器停止
	RTC.RCR3.BIT.RTCEN = 0;
	RTC.RCR4.BIT.RCKSEL = 1;		// カウントソースにメインクロックを選択
	for (i = 0; i< 0x14A; i++)		// wait over 11ms
	{
	}
	RTC.RCR2.BIT.START = 0;			// RCR2.STARTビットを"0"にする
	for (i = 0; i< 0x14A; i++)		// wait over 11ms
	{
	}
	RTC.RCR2.BIT.RESET = 1;			// RCR2.RESETビットを"1"にする
	for (i = 0; i< 0x14A; i++)		// wait over 11ms
	{
	}
	RTC.RCR1.BYTE = 0x00;			// RCR1.AIE,CIE,PIEの割り込み要求の禁止
	for (i = 0; i< 0x14A; i++)		// wait over 11ms
	{
	}

	/* クロックの設定
	 * (入力クロック12MHzを想定)
	 */
	SYSTEM.PLLCR.BIT.PLIDIV = 0x00;		// PLL入力分周比(1分周)	 = 12MHz
	SYSTEM.PLLCR.BIT.STC = 0x0F;		// PLL周波数逓倍率(16逓倍) = 12 * 16 = 192
	SYSTEM.SCKCR2.BIT.UCK	= 0x03;		// USBクロック(4分周) = 192/4 = 48MHz
	SYSTEM.SCKCR2.BIT.IEBCK	= 0x03;		// IECLK = 192/8 = 24MHz

	SYSTEM.MOSCCR.BYTE = 0x00;			// メインクロック発振動作
	SYSTEM.PLLCR2.BYTE = 0x00;			// PLL回路を動作開始

	for (i = 0; i< 0x14A; i++)			// wait over 11ms
	{
	}

	SYSTEM.SCKCR.LONG = 0x21021211;		// FCK:Flashクロック				= 192 / 4 = 48MHz
										// ICK:システムクロック			= 192 / 2 = 96MHz
										// BCK:外部バスクロック			= 192 / 4 = 48MHz
										// PCKA:周辺モジュールクロックA	= 192 / 2 = 96MHz
										// PCKB:周辺モジュールクロックB	= 192 / 4 = 48MHz

	SYSTEM.SCKCR3.WORD = 0x0400;		// PLL選択
}

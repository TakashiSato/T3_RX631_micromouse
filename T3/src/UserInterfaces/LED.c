/**
 * @file  LED.c
 * @brief LEDの動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "LED.h"
#include "../iodefine.h"
#include "../Peripherals/Timer.h"

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void LED_InitializePort(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/**
 * LEDの初期化
 * @param void
 * @retval void
 */
void LED_Initialize(void)
{
	LED_InitializePort();
}

/**
 * LEDパターンの点灯
 * @param lightPattern: LED点灯パターン(4bit)
 * @retval void
 */
void LED_Disp(_UBYTE lightPattern)
{
	LED0 = ( lightPattern & 0x01 );
	LED1 = ( (lightPattern >> 1) & 0x01 );
	LED2 = ( (lightPattern >> 2) & 0x01 );
	LED3 = ( (lightPattern >> 3) & 0x01 );
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/**
 * LED接続端子の初期化
 * @param void
 * @retval void
 */
static void LED_InitializePort(void)
{
	PORT3.PDR.BIT.B1 = 1;
	PORT2.PDR.BIT.B7 = 1;
	PORTE.PDR.BIT.B3 = 1;
	PORTE.PDR.BIT.B1 = 1;
	PORT3.PODR.BIT.B1 = 0;
	PORT2.PODR.BIT.B7 = 0;
	PORTE.PODR.BIT.B3 = 0;
	PORTE.PODR.BIT.B1 = 0;
}

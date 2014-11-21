/**
 * @file  LED.h
 * @brief LEDの動作を司るクラス
 */

#ifndef __LED_H__
#define __LED_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define LED0	PORTE.PODR.BIT.B3		//LED0接続端子
#define LED1	PORTE.PODR.BIT.B1		//LED1接続端子
#define LED2	PORT2.PODR.BIT.B7		//LED2接続端子
#define LED3	PORT3.PODR.BIT.B1		//LED3接続端子

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void LED_Initialize(void);
void LED_Disp(_UBYTE lightPattern);

#endif

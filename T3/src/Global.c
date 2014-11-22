/**
 * @file  Global.c
 * @brief グローバル変数の宣言をまとめたファイル
 */

/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include "Global.h"
#include "Peripherals/SerialPort.h"
#include "Devices/AD-128160-UART.h"

/*----------------------------------------------------------------------
	プログラム全体で使う定数の定義
 ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
	グローバル変数の定義
 ----------------------------------------------------------------------*/
volatile struct stMouseFlags _MF;

/*----------------------------------------------------------------------
	プログラム全体で使う関数の定義
 ----------------------------------------------------------------------*/
#if SERIAL_TARGET_IS_CONSOLE == 1
	void (*Printf)(_UBYTE*, ...) = SCI_Printf;
	void (*Scanf)(_UBYTE* str, ...) = SCI_Scanf;
#else
	void (*Printf)(_UBYTE*, ...) = AD128160_Printf;
#endif

void (*WaitMS)(_UINT msec) = Timer_WaitMS;
void (*WaitUS)(_UINT usec) = Timer_WaitUS;
void (*DispLED)(_UBYTE lightPattern) = LED_Disp;
void (*PlaySound)(_UINT freq) = Speaker_PlaySound;
bool (*GetSwitchState)(void) = Switch_GetState;
_UBYTE (*ModeSelect)(void) = Switch_ModeSelect;

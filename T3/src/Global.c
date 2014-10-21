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
_UWORD ad;

/*----------------------------------------------------------------------
	プログラム全体で使う関数の定義
 ----------------------------------------------------------------------*/
#if SERIAL_TARGET_IS_CONSOLE == 1
	void (*Printf)(char*, ...) = SCI_Printf;
	void (*Scanf)(char* str, ...) = SCI_Scanf;
#else
	void (*Printf)(char*, ...) = AD128160_Printf;
#endif
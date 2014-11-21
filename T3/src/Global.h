/**
 * @file  Global.h
 * @brief グローバル変数の宣言をまとめたファイル
 */

#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include "typedefine.h"
#include "stdbool.h"
#include "Peripherals/Timer.h"
#include "UserInterfaces/LED.h"
#include "UserInterfaces/Speaker.h"
#include "UserInterfaces/Switch.h"

/*----------------------------------------------------------------------
	プログラム全体で使う定数の宣言
 ----------------------------------------------------------------------*/
#define SERIAL_TARGET_IS_CONSOLE 1		// 0:AD-128160, 1:シリアルポート

/*----------------------------------------------------------------------
	グローバル変数の宣言
 ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
	プログラム全体で使う関数の宣言
 ----------------------------------------------------------------------*/
extern void (*Printf)(_UBYTE* str, ...);
extern void (*Scanf)(_UBYTE* str, ...);
extern void (*WaitMS)(_UINT msec);
extern void (*WaitUS)(_UINT usec);
extern void (*DispLED)(_UBYTE lightPattern);
extern void (*PlaySound)(_UINT freq);
extern bool (*GetSwitchState)(void);

#endif

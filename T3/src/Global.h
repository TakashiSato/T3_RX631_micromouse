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
#define PI		3.14159265358979
#define SERIAL_TARGET_IS_CONSOLE 1		// 0:AD-128160, 1:シリアルポート

/*----------------------------------------------------------------------
	グローバル変数の宣言
 ----------------------------------------------------------------------*/
//==== マウスフラグ ====
extern volatile struct stMouseFlags{
	// ---- モータドライブ動作 ----
	union{
		unsigned char BYTE;
		struct{
			unsigned char STOP	:1;		// モータ停止フラグ(B7)
			unsigned char TURN	:1;		// 超信地旋回フラグ(B6)
			unsigned char SLAL_L:1;		// 左スラロームフラグ(B5)
			unsigned char SLAL_R:1;		// 右スラロームフラグ(B4)
			unsigned char DEF	:1;		// 定速(デフォルト速度)フラグ(B3)
			unsigned char CONS	:1;		// 定速(直前の速度維持)フラグ(B2)
			unsigned char DECL	:1;		// 減速フラグ(B1)
			unsigned char ACCL	:1;		// 加速フラグ(B0)
		}BIT;
	}MOTOR;
	// ---- 走行状態 ----
	union{
		unsigned char BYTE;
		struct{
			unsigned char SCND	:1;		// 二次走行フラグ(B7)
			unsigned char SRCH	:1;		// 探索走行フラグ(B6)
			unsigned char DSRCH	:1;		// 重ね探索フラグ(B5)
			unsigned char TRACE	:1;		// ライントレースフラグ(B4)
			unsigned char DIAG	:1;		// 斜め走行フラグ(B3)
			unsigned char SLAL	:1;		// スラローム走行フラグ(B2)
			unsigned char ADJ	:1;		// 位置補正走行フラグ(B1)
			unsigned char GOAL	:1;		// ゴールフラグ(B0)
		}BIT;
	}STATE;
	// ---- 制御 ----
	union{
		unsigned char BYTE;
		struct{
			unsigned char SIDE	:1;		// 横壁制御フラグ(B7)
			unsigned char FWD	:1;		// 前壁制御フラグ(B6)
			unsigned char EDGE	:1;		// 壁切れ制御フラグ(B5)
			unsigned char EDGT	:1;		// 壁切れ制御トリガフラグ(B4)
			unsigned char EDGF	:1;		// 壁切れ検知フラグ(B3)
			unsigned char SLOG	:1;		// センサログ取得フラグ(B2)
			unsigned char YOBI3	:1;		// 予備フラグ(B1)
			unsigned char YOBI4	:1;		// 予備フラグ(B0)
		}BIT;
	}CTRL;
}_MF;

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
extern _UBYTE (*ModeSelect)(void);

#endif

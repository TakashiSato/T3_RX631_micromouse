/**
 * @file  Search.h
 * @brief の動作を司るクラス
 */

#ifndef __SEARCH_H__
#define __SEARCH_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
// ==== 方向転換用定数 ====
#define DIR_TURN_0		0x00	//0度
#define DIR_TURN_R90	0x01	//右90度回転
#define DIR_TURN_L90	0xff	//左90度回転
#define DIR_TURN_180	0x02	//180度回転

// ==== スタート座標 ====
#define START_X	0
#define START_Y	0
#define GOAL_X	11
#define GOAL_Y 	3

#define ADJUST_NUM 0


/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
// ==== 初期化 ====
void Search_Init();

// ==== 足立法探索 ====
void Search_Adachi();

// ==== 位置情報前進 ====
void Search_AdvancePosition();

// ==== 壁情報取得 ====
void Search_GetWallInfo();

// ==== 進路判定 ====
void Search_ConfirmRoute();

// ==== マップデータ初期化 ====
void Search_MapInit();

// ==== マップデータ書き込み ====
void Search_WriteMap();

// ==== マウスの方向を変更 ====
void Search_TurnDir(_UBYTE);

// ==== マウスの方向を直接設定 ====
void Search_SetDir(_UBYTE);

// ==== 歩数マップ作成 ====
void Search_MakeStepMap();

// ==== 最短経路導出 ====
void Search_MakeRoute();

//// ==== 最短経路を表示 ====
//void Search_PrintRoute();
//
//// ==== ゴール座標をセット ====
//void Search_SetGoal(SCHAR x, SCHAR y);


#endif /* __SEARCH_H__ */

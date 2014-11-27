/**
 * @file  MouseController.h
 * @brief マウスの制御を司るクラス
 */

#ifndef __MOUSECONTROLLER_H__
#define __MOUSECONTROLLER_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define CONTROL_INTERVAL		12000	// 制御周期 12000: 1[mSec]
#define CONTROL_INTERVAL_MSEC	1		// [msec]単位制御周期
#define CONTROL_INTERVAL_SEC	0.001	// [sec]単位制御周期
#define CONTROL_INTERVEL_INVSEC	1000	// [sec]単位制御周期の逆数[1/sec]

// ==== 制御基準値 ====
#define CTRL_REF_MIN_L	-300			// 左制御基準下限
#define CTRL_REF_MAX_L	500				// 左制御基準上限
#define CTRL_REF_MIN_R	-300			// 右制御基準下限
#define CTRL_REF_MAX_R	500				// 右制御基準上限

// ==== 走行距離 ====
#define DR_SEC_HALF			24.0	// 半区画走行
#define DR_ROT_R90			20.0	// 右90度回転
#define DR_ROT_L90			20.0	// 左90度回転
#define DR_ROT_R180			50.0	// 右180度回転
#define DR_ROT_L180			50.0	// 左180度回転
#define DR_CENT_SET			8.0		// 後ろ壁から中央までの距離

#define DEF_V0		8
#define DEF_V_CONST	30
#define DEF_VMIN	8
#define DEF_VMAX	25
#define DEF_ACC		60
#define DEF_ANGV0	0.3
#define DEF_ANGVMIN 0.3
#define DEF_ANGVMAX	0.3
#define DEF_ANGACC	0

/*----------------------------------------------------------------------
	Struct Definitions
 ----------------------------------------------------------------------*/
typedef struct stBinaryTimeSeriesVal
{
	_UWORD Now;
	_UWORD Old;
}BinaryTimeSeriesVal;

typedef struct sFloatTimeSeriesVal
{
	float Now;
	float Old;
}FloatTimeSeriesVal;

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void MouseController_Initialize(void);
#pragma inline(MouseController_IntMTU2TGIA)
void MouseController_IntMTU2TGIA(void);
float MouseController_GetVel(void);
float MouseController_GetAngvel(void);
void MouseController_CheckValue(void);
void MouseControlle_MotorTest(void);

void HalfSectionA(void);
void HalfSectionD(void);
void TurnL90AD(void);
void TurnR90AD(void);
void TurnR180AD(void);
void TurnL180AD(void);
void SetPosition(void);

#endif /* __MOUSECONTROLLER_H__ */

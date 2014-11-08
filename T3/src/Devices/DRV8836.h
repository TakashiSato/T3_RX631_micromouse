/**
 * @file  DRV8836.h
 * @brief モータドライバDRV8836の動作を司るクラス
 */

#ifndef __DRV8836_H__
#define __DRV8836_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include <stdbool.h>
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define MDR_NSLEEP_PORT		PORT1.PODR.BIT.B6
#define MDR_APHASE_PORT		PORT1.PODR.BIT.B4
#define MDR_BPHASE_PORT		PORTB.PODR.BIT.B5
#define MDR_AENBL_TGR		MTU0.TGRC
#define MDR_BENBL_TGR		MTU0.TGRD

/*----------------------------------------------------------------------
	Enum Definitions
 ----------------------------------------------------------------------*/
// モータの種類
typedef enum eMotorType
{
	MOTOR_TYPE_LEFT,
	MOTOR_TYPE_RIGHT,
	MOTOR_TYPE_ERROR
}E_MOTOR_TYPE;

// モータの回転方向
typedef enum eMotorDir
{
	MOTOR_DIR_CW = 0,
	MOTOR_DIR_CCW = 1,
	MOTOR_DIR_ERROR
}E_MOTOR_DIR;

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void DRV8836_Initialize(void);
bool DRV8836_DriveMotor(E_MOTOR_TYPE type, E_MOTOR_DIR dir, _UINT duty);
E_MOTOR_DIR DRV8836_GetMotorDirection(E_MOTOR_TYPE type);
void DRV8836_SetSleep(void);
void DRV8836_Wakeup(void);
_UINT DRV8836_GetMotorDuty(E_MOTOR_TYPE type);


#endif

/**
 * @file  DRV8836.h
 * @brief モータドライバDRV8836の動作を司るクラス
 */

#ifndef __DRV8836_H__
#define __DRV8836_H__
/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include <stdbool.h>
#include "../typedefine.h"

/*----------------------------------------------------------------------
	列挙型の定義
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
	ソースコード
 ----------------------------------------------------------------------*/
/* ==== パブリックメソッド ==== */
/** DRV8836を初期化する
 * @param void
 * @retval void
 */
void DRV8836_Initialize(void);

/** モータ駆動状態を変更する
 * @param type : モータの種類(Left or Right)
 * @param dir : モータの回転方向(Cw or Ccw)
 * @param duty : 設定するduty比 0-100[%]
 * @retval bool : 正常に設定できればtrueを返す
 */
bool DRV8836_DriveMotor(E_MOTOR_TYPE type, E_MOTOR_DIR dir, _UINT duty);

/** モータの回転方向を取得する
 * @param type : モータの種類(Left or Right)
 * @retval 回転方向
 */
E_MOTOR_DIR DRV8836_GetMotorDirection(E_MOTOR_TYPE type);

/** モータドライバをスリープ状態に移行させる
 * @param void
 * @retval void
 */
void DRV8836_SetSleep(void);

/** モータドライバを動作状態に移行させる
 * @param void
 * @retval void
 */
void DRV8836_Wakeup(void);

/** モータのデューティ比を取得する
 * @param type : モータの種類(Left or Right)
 * @retval デューティ比
 */
_UINT DRV8836_GetMotorDuty(E_MOTOR_TYPE type);

/* ==== プライベートメソッド ==== */
/** TPUの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializeTPU(void);

/** DRV8836で使うポートの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializePort(void);

#endif

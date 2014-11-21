/**
 * @file  DRV8836.h
 * @brief モータドライバDRV8836の動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "DRV8836.h"
#include "../iodefine.h"
#include "../Global.h"

/*----------------------------------------------------------------------
	Private global variables
 ----------------------------------------------------------------------*/
static const _UINT PWM_FREQ = 1200;		// PWMの周期(1200:10kHz)
static _UINT _motorA_duty = 0;			// モータAのDuty
static _UINT _motorB_duty = 0;			// モータBのDuty
static E_MOTOR_DIR _motorA_dir = MOTOR_DIR_CW;			// モータAの回転方向
static E_MOTOR_DIR _motorB_dir = MOTOR_DIR_CW;			// モータBの回転方向

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void DRV8836_InitializeMTU(void);
static void DRV8836_InitializePort(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/** DRV8836を初期化する
 * @param void
 * @retval void
 */
void DRV8836_Initialize(void)
{
	// ピン機能設定
	DRV8836_InitializePort();
	// モータドライバをスリープ状態に移行
	DRV8836_SetSleep();
	// MTU初期化
	DRV8836_InitializeMTU();
	// Duty設定
	DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 0);
	DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0);
	// Wait
	WaitMS(100);
	// モータドライバを動作状態に移行
//	DRV8836_Wakeup();
}

/** モータ1の出力PWMのDutyを設定する
 * @param type : モータの種類(Left or Right)
 * @param dir : モータの回転方向(Cw or Ccw)
 * @param duty : 設定するduty比 0-100[%]
 * @retval bool : 正常に設定できればtrueを返す
 */
bool DRV8836_DriveMotor(E_MOTOR_TYPE type, E_MOTOR_DIR dir, _UINT duty)
{
	_UINT tgr;

	// Check param
	if( 100 < duty ||
		(dir != MOTOR_DIR_CW && dir != MOTOR_DIR_CCW) ||
		(type != MOTOR_TYPE_LEFT && type != MOTOR_TYPE_RIGHT))
	{
		return false;
	}

	tgr = (PWM_FREQ / 100) * duty;

	switch(type)
	{
		case MOTOR_TYPE_LEFT:
			_motorA_dir = dir;
			_motorA_duty = duty;
			MDR_APHASE_PORT = dir;
			MDR_AENBL_TGR = tgr;
			break;

		case MOTOR_TYPE_RIGHT:
			_motorB_dir = dir;
			_motorB_duty = duty;
			MDR_BPHASE_PORT = dir;
			MDR_BENBL_TGR = tgr;
			break;

		default:
			return false;
			break;
	}

	return true;
}

/** モータの回転方向を取得する
 * @param type : モータの種類(Left or Right)
 * @retval 回転方向
 */
E_MOTOR_DIR DRV8836_GetMotorDirection(E_MOTOR_TYPE type)
{
	switch(type)
	{
		case MOTOR_TYPE_LEFT:
			return _motorA_dir;
			break;

		case MOTOR_TYPE_RIGHT:
			return _motorB_dir;
			break;

		default:
			break;
	}

	return MOTOR_DIR_ERROR;
}

/** モータのデューティ比を取得する
 * @param type : モータの種類(Left or Right)
 * @retval デューティ比
 */
_UINT DRV8836_GetMotorDuty(E_MOTOR_TYPE type)
{
	switch(type)
	{
		case MOTOR_TYPE_LEFT:
			return _motorA_duty;
			break;

		case MOTOR_TYPE_RIGHT:
			return _motorB_duty;
			break;

		default:
			break;
	}

	return 0;
}

/** モータドライバをスリープ状態に移行させる
 * @param void
 * @retval void
 */
void DRV8836_SetSleep(void)
{
	MDR_NSLEEP_PORT = 0;
}

/** モータドライバを動作状態に移行させる
 * @param void
 * @retval void
 */
void DRV8836_Wakeup(void)
{
	MDR_NSLEEP_PORT = 1;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** MTUの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializeMTU(void)
{
	MSTP(MTU0) = 0;				// MTU0のモジュールストップ状態の解除
	MSTP(MTU1) = 0;				// MTU0のモジュールストップ状態の解除

	// 同期動作の設定
	// MTU0とMTU1のTCNTカウンタは同期動作
	MTU.TSYR.BIT.SYNC0 = 1;
	MTU.TSYR.BIT.SYNC1 = 1;

	// タイマカウンタの初期化
	MTU0.TCNT = 0;
	MTU1.TCNT = 0;

	// カウンタクロックの選択
	MTU0.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	MTU0.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち上がりエッジでカウント
	MTU1.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	MTU1.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち上がりエッジでカウント

	// カウンタクリア要因の選択
	MTU0.TCR.BIT.CCLR = 7;		// 同期クリア(MTU1.TGRAコンペアマッチでTCNTカウンタクリア)
	MTU1.TCR.BIT.CCLR = 1;		// TGRAコンペアマッチでTCNTカウンタをクリア

	// タイマモードの選択
	MTU0.TMDR.BIT.MD = 3;		// PWMモード2
	MTU1.TMDR.BIT.MD = 0;		// ノーマルモード

	// バッファ動作設定
	MTU0.TMDR.BIT.BFA = 1;		// TGRAとTGRCはバッファ動作
	MTU0.TMDR.BIT.BFB = 1;		// TGRBとTGRDはバッファ動作

	// 波形出力レベルの選択
	MTU0.TIORH.BIT.IOA = 5;		// MTIOC0A端子:初期出力High,コンペアマッチでLow出力
	MTU0.TIORH.BIT.IOB = 5;		// MTIOC0B端子:初期出力High,コンペアマッチでLow出力

	// TGRyの設定
	MTU0.TGRA = 0;				// モータ1デューティ比
	MTU0.TGRB = 0;				// モータ2デューティ比
	MTU0.TGRC = 0;				// モータ1デューティ比(バッファ)
	MTU0.TGRD = 0;				// モータ2デューティ比(バッファ)
	MTU1.TGRA = PWM_FREQ;		// PWM周期

	// カウント動作開始
	MTU.TSTR.BIT.CST1 = 1;		// MTU1カウント動作開始
	MTU.TSTR.BIT.CST0 = 1;		// MTU0カウント動作開始
}

/** DRV8836で使うポートの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializePort(void)
{
	// IO端子の設定
	PORT1.PMR.BIT.B4 = 0;			// P14を汎用入出力ポートに設定
	PORT1.PMR.BIT.B6 = 0;			// P16を汎用入出力ポートに設定
	PORTB.PMR.BIT.B5 = 0;			// PB5を汎用入出力ポートに設定
	PORT1.PDR.BIT.B4 = 1;			// P14を出力ポートに設定
	PORT1.PDR.BIT.B6 = 1;			// P16を出力ポートに設定
	PORTB.PDR.BIT.B5 = 1;			// PB5を出力ポートに設定
	PORT1.PODR.BIT.B4 = 0;			// P14をLow出力に設定
	PORT1.PODR.BIT.B6 = 0;			// P16をLow出力に設定
	PORTB.PODR.BIT.B5 = 0;			// PB5をLow出力に設定

	// PWM出力端子の設定
	PORT1.PMR.BIT.B5 = 0;			// P15を汎用入出力ポートに設定
	PORTB.PMR.BIT.B3 = 0;			// PB3を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;			// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;			// PFSレジスタへの書き込みを許可
	MPC.P15PFS.BIT.PSEL = 1;		// P15の機能選択:MTIOC0B
	MPC.PB3PFS.BIT.PSEL = 1;		// PB3の機能選択:MTIOC0A
	MPC.PWPR.BYTE = 0x80;			// PFSレジスタ,PFSWEビットへの書き込みを禁止
	PORT1.PMR.BIT.B5 = 1;			// PB0を周辺機能として使用
	PORTB.PMR.BIT.B3 = 1;			// PB1を周辺機能として使用
}

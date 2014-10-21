/**
 * @file  DRV8836.h
 * @brief モータドライバDRV8836の動作を司るクラス
 */

/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include "DRV8836.h"
#include "../iodefine.h"
#include "../BoardDefine.h"
#include "../Global.h"
#include "../Peripherals/Timer.h"
#include "../Devices/AD-128160-UART.h"

/*----------------------------------------------------------------------
	マクロ定義
 ----------------------------------------------------------------------*/
#define MDR_APHASE_PORT		PORT1.PODR.BIT.B4
#define MDR_BPHASE_PORT		PORT1.PODR.BIT.B5
#define MDR_NSLEEP_PORT		PORT1.PODR.BIT.B6
#define MDR_AENBL_TGR		TPU3.TGRC
#define MDR_BENBL_TGR		TPU3.TGRD

/*----------------------------------------------------------------------
	プライベートプロパティ
 ----------------------------------------------------------------------*/
static const _UINT PWM_FREQ = 1200;		// PWMの周期(1200:10kHz)
static _UINT _motorA_duty = 0;			// モータAのDuty
static _UINT _motorB_duty = 0;			// モータBのDuty
static _UINT _motorA_dir = 0;			// モータAの回転方向
static _UINT _motorB_dir = 0;			// モータBの回転方向

/*----------------------------------------------------------------------
	パブリックメソッド
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
	// TPU初期化
	DRV8836_InitializeTPU();
	// Duty設定
	DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 0);
	DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0);
	// Wait
	WaitMS(100);
	// モータドライバを動作状態に移行
	DRV8836_Wakeup();
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
	プライベートメソッド
 ----------------------------------------------------------------------*/
/** TPUの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializeTPU(void)
{
	MSTP(TPU2) = 0;				// TPU2のモジュールストップ状態の解除
	MSTP(TPU3) = 0;				// TPU3のモジュールストップ状態の解除

	// 同期動作の設定
	// TPU2とTPU3のTCNTカウンタは同期動作
	TPUA.TSYR.BIT.SYNC2 = 1;
	TPUA.TSYR.BIT.SYNC3 = 1;

	// タイマカウンタの初期化
	TPU2.TCNT = 0;

	// カウンタクロックの選択
	TPU2.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	TPU2.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち下がりエッジでカウント
	TPU3.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	TPU3.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち下がりエッジでカウント

	// カウンタクリア要因の選択
	TPU2.TCR.BIT.CCLR = 1;		// TGRAコンペアマッチでTCNTカウンタをクリア
	TPU3.TCR.BIT.CCLR = 3;		// 同期クリア(TPU2.TGRAコンペアマッチでTCNTカウンタクリア)

	// 波形出力レベルの選択
//	TPU2.TIOR.BIT.IOA = 6;		// TIOCA2,TIOCB2端子:出力禁止
//	TPU2.TIOR.BIT.IOB = 6;		// TIOCA2,TIOCB2端子:出力禁止
	TPU3.TIORH.BIT.IOA = 5;		// TIOCA3端子:初期出力High,コンペアマッチでLow出力
	TPU3.TIORH.BIT.IOB = 5;		// TIOCB3端子:初期出力High,コンペアマッチでLow出力

	// TGRyの設定
	TPU2.TGRA = PWM_FREQ;		// PWM周期
	TPU3.TGRA = 0;				// モータ1デューティ比
	TPU3.TGRB = 0;				// モータ2デューティ比
	TPU3.TGRC = 0;				// モータ1デューティ比(バッファ)
	TPU3.TGRD = 0;				// モータ2デューティ比(バッファ)

	// PWMモードの設定
	TPU2.TMDR.BIT.MD = 0;		// モード:通常動作
	TPU3.TMDR.BIT.MD = 3;		// モード:PWMモード2

	// バッファ動作設定
	TPU3.TMDR.BIT.BFA = 1;		// TGRAとTGRCはバッファ動作
	TPU3.TMDR.BIT.BFB = 1;		// TGRBとTGRDはバッファ動作

	// カウント動作開始
	TPUA.TSTR.BIT.CST2 = 1;		// TPU2カウント動作開始
	TPUA.TSTR.BIT.CST3 = 1;		// TPU3カウント動作開始
}

/** DRV8836で使うポートの初期化
 * @param void
 * @retval void
 */
static void DRV8836_InitializePort(void)
{
	// IO端子の設定
	PORT1.PMR.BIT.B4 = 0;			// P14を汎用入出力ポートに設定
	PORT1.PMR.BIT.B5 = 0;			// P15を汎用入出力ポートに設定
	PORT1.PMR.BIT.B6 = 0;			// P16を汎用入出力ポートに設定
	PORT1.PDR.BIT.B4 = 1;			// P14を出力ポートに設定
	PORT1.PDR.BIT.B5 = 1;			// P15を出力ポートに設定
	PORT1.PDR.BIT.B6 = 1;			// P16を出力ポートに設定
	PORT1.PODR.BIT.B4 = 0;			// P14をLow出力に設定
	PORT1.PODR.BIT.B5 = 0;			// P15をLow出力に設定
	PORT1.PODR.BIT.B6 = 0;			// P16をLow出力に設定

	// PWM出力端子の設定
	PORTB.PMR.BIT.B0 = 0;			// PB0を汎用入出力ポートに設定
	PORTB.PMR.BIT.B1 = 0;			// PB1を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;			// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;			// PFSレジスタへの書き込みを許可
	MPC.PB0PFS.BIT.PSEL = 3;		// PB0の機能選択:TIOCA3
	MPC.PB1PFS.BIT.PSEL = 3;		// PB1の機能選択:TIOCB3
	MPC.PWPR.BYTE = 0x80;			// PFSレジスタ,PFSWEビットへの書き込みを禁止
	PORTB.PMR.BIT.B0 = 1;			// PB0を周辺機能として使用
	PORTB.PMR.BIT.B1 = 1;			// PB1を周辺機能として使用
}

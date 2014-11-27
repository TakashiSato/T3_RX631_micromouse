/**
 * @file  MouseController.c
 * @brief マウスの制御を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "MouseController.h"
#include <stdbool.h>
#include "../iodefine.h"
#include "../Global.h"
#include "../Hardware/HardwareParameter.h"
#include "../Devices/AS5055.h"
#include "../Devices/DRV8836.h"
#include "../Devices/LightSensor.h"

/*----------------------------------------------------------------------
	Private Global Variables
 ----------------------------------------------------------------------*/
static FloatTimeSeriesVal _vel;			// 機体速度 [m/s]
static FloatTimeSeriesVal _angvel;		// 機体角速度 [rad/s]

static BinaryTimeSeriesVal _wheelLAng;	// 左車輪回転角度
static BinaryTimeSeriesVal _wheelRAng;	// 右車輪回転角度
static FloatTimeSeriesVal _wheelLDist;	// 左車輪回転量
static FloatTimeSeriesVal _wheelRDist;	// 右車輪回転量
static FloatTimeSeriesVal _wheelLVel;	// 左車輪回転速度
static FloatTimeSeriesVal _wheelRVel;	// 右車輪回転速度

static float _x = 0;
static _UWORD _t = 0;
static bool _driving = false;;

#define LOG_SIZE	500
static	volatile _UWORD wordlogL[LOG_SIZE] = {0};
static	volatile _UWORD wordlogR[LOG_SIZE] = {0};
static	volatile float floatlogL[LOG_SIZE] = {0};
static	volatile float floatlogR[LOG_SIZE] = {0};

static float kp = -0.01, kd = 0.0;						// 比例,微分制御係数格納変数
static float _dl, _dr;
static float _dutyL, _dutyR;
static float _tarv, _taracc, _tarvmax;
static float _tarvmin = DEF_VMIN;
static float _tarangv, _tarangacc, _tarangvmax;
static float _tarangvmin = DEF_VMIN;
static float _kVtoDuty = 0.5;

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void MouseController_InitializeMTU2(void);
static void MouseController_UpdateParameter(void);
static void MouseController_SideWallControl(void);
static void DriveA(float v0, float vmax, float acc, float dist);
static void DriveD(float vmin, float acc, float dist, bool rs);
static void DriveTime(float v0, float ms);
static void TurnA(float angv0, float angvmax, float angacc, float dist);
static void TurnD(float angvmin, float angacc, float dist, bool rs);


/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/**
 * マウスコントローラーの初期化
 * @param void
 * @retval void
 */
void MouseController_Initialize(void)
{
	MouseController_InitializeMTU2();
}

/**
 * 制御用MTU2TGIA割り込み関数
 */
void MouseController_IntMTU2TGIA(void)
{
	MouseController_UpdateParameter();

	if(_driving)
	{
		float xL, xR;

		// 目標速度の算出
		_tarv += _taracc * CONTROL_INTERVAL_SEC;
		if(_tarv > 0)
		{
			_tarv = (_tarv > _tarvmax) ? _tarvmax : _tarv;
			_tarv = (_tarv < _tarvmin) ? _tarvmin : _tarv;
		}
		else
		{
			_tarv = (_tarv < _tarvmax) ? _tarvmax : _tarv;
			_tarv = (_tarv > _tarvmin) ? _tarvmin : _tarv;
		}

		// 目標角速度の算出
		_tarangv += _tarangacc * CONTROL_INTERVAL_SEC;
		if(_tarangv > 0)
		{
			_tarangv = (_tarangv > _tarangvmax) ? _tarangvmax : _tarangv;
			_tarangv = (_tarangv < _tarangvmin) ? _tarangvmin : _tarangv;
		}
		else
		{
			_tarangv = (_tarangv < _tarangvmax) ? _tarangvmax : _tarangv;
			_tarangv = (_tarangv > _tarangvmin) ? _tarangvmin : _tarangv;
		}

		// 横壁制御成分の計算
		MouseController_SideWallControl();

		// 各車輪回転速度を算出し,回転させる
		_dutyL = _kVtoDuty * (_tarv + _dl + HW_TREAD_WIDTH * _tarangv);
		_dutyR = _kVtoDuty * (_tarv + _dr - HW_TREAD_WIDTH * _tarangv);
		if(_dutyL > 0.0)
		{
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, _dutyL);
			xL = _wheelLDist.Now;
		}
		else
		{
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, -_dutyL);
			xL = -_wheelLDist.Now;
		}
		if(_dutyR > 0.0)
		{
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, _dutyR);
			xR = _wheelRDist.Now;
		}
		else
		{
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CCW, -_dutyR);
			xR = -_wheelRDist.Now;
		}
		_t++;
		_x += (xL + xR) / 2.0;
	}
//	else
//	{
//		DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 0);
//		DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0);
//	}
}

/**
 * 機体速度の取得
 * @param void
 * @retval float: 機体速度 [m/s]
 */
float MouseController_GetVel(void)
{
	return _vel.Now;
}

/**
 * 機体角速度の取得
 * @param void
 * @retval float: 機体角速度 [rad/s]
 */
float MouseController_GetAngvel(void)
{
	return _angvel.Now;
}

/**
 * 値チェック
 * @param void
 * @retval void
 */
void MouseController_CheckValue(void)
{
	Printf("L:%4.2f, R: %4.2f\n", _wheelLDist.Now, _wheelRDist.Now);
//	Printf("L:%d, %f, R:%d, %f\n", _wheelLAng.Now, _wheelLVel.Now, _wheelRAng.Now, _wheelRVel.Now);
//	Printf("vel:%f, angvel:%f\n", _vel.Now, _angvel.Now);
}

/**
 * モータテスト
 * @param void
 * @retval void
 */
void MouseControlle_MotorTest(void)
{
	bool loopFlag = true;
	_UBYTE mode = 0;

	while(loopFlag)
	{
		DispLED(0x02);
		mode = ModeSelect();
		WaitMS(500);
		DispLED(0x00);

		switch (mode)
		{
		case 1:
			HalfSectionA();
			HalfSectionD();
			break;
		case 2:
			TurnL90AD();
			break;
		case 3:
			TurnR90AD();
			break;
		case 4:
			TurnL180AD();
			break;
		case 5:
			TurnR180AD();
			break;
		case 6:
			SetPosition();
			break;
		case 7:
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 12.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 12.0);
			WaitMS(100);
			for(int i = 0; i < LOG_SIZE; i++)
			{
				wordlogL[i] = _wheelLAng.Now;
				wordlogR[i] = _wheelRAng.Now;
				floatlogL[i] = _wheelLDist.Now;
				floatlogR[i] = _wheelRDist.Now;
				WaitMS(1);
			}
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 0.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0.0);

			Printf("t,AngL,AngR,DistL,DistR\n");
			for(int i = 0; i < LOG_SIZE; i++)
			{
				Printf("%d,%d,%d,%f,%f\n", i, wordlogL[i], wordlogR[i], floatlogL[i], floatlogR[i]);
				WaitMS(10);
			}
			break;

		case 14:
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 12.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 12.0);
			while(!GetSwitchState())
			{
				Printf("vel:%f, angvel:%f\n", MouseController_GetVel(),	MouseController_GetAngvel());
				WaitMS(100);
			}
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 0.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0.0);
			break;
		case 15:
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 12.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 12.0);
			while(!GetSwitchState())
			{
				Printf("vel:%f, angvel:%f\n", MouseController_GetVel(),	MouseController_GetAngvel());
				WaitMS(100);
			}
			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 0.0);
			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 0.0);
			break;
		default:
			loopFlag = false;
			break;
		}
	}
}

void HalfSectionA(void)
{
	_MF.CTRL.BIT.SIDE = 1;
	DriveA(DEF_V0, DEF_VMAX, DEF_ACC, DR_SEC_HALF);
	_MF.CTRL.BIT.SIDE = 0;
}
void HalfSectionD(void)
{
	_MF.CTRL.BIT.SIDE = 1;
	DriveD(DEF_VMIN, -DEF_ACC, DR_SEC_HALF, true);
	_MF.CTRL.BIT.SIDE = 0;
	WaitMS(400);
}

void TurnL90AD(void)
{
	TurnA(-DEF_ANGV0, -DEF_ANGVMAX, -DEF_ANGACC, DR_ROT_L90/2);
	TurnD(-DEF_ANGVMIN, DEF_ANGACC, DR_ROT_L90/2, true);
	WaitMS(400);
}

void TurnR90AD(void)
{
	TurnA(DEF_ANGV0, DEF_ANGVMAX, DEF_ANGACC, DR_ROT_R90/2);
	TurnD(DEF_ANGVMIN, -DEF_ANGACC, DR_ROT_R90/2, true);
	WaitMS(400);
}

void TurnL180AD(void)
{
	TurnA(-DEF_ANGV0, -DEF_ANGVMAX, -DEF_ANGACC, DR_ROT_L180/2);
	TurnD(-DEF_ANGVMIN, DEF_ANGACC, DR_ROT_L180/2, true);
	WaitMS(400);
}
void TurnR180AD(void)
{
	TurnA(DEF_ANGV0, DEF_ANGVMAX, DEF_ANGACC, DR_ROT_R180/2);
	TurnD(DEF_ANGVMIN, -DEF_ANGACC, DR_ROT_R180/2, true);
	WaitMS(400);
}

void SetPosition(void)
{
	DriveTime(-DEF_V_CONST, 1000);
	WaitMS(100);
	_MF.CTRL.BIT.SIDE = 1;
	DriveA(DEF_V0, DEF_VMAX, DEF_ACC, DR_CENT_SET/2);
	DriveD(DEF_VMIN, -DEF_ACC, DR_CENT_SET/2, true);
	_MF.CTRL.BIT.SIDE = 0;
	WaitMS(100);
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** MTU2の初期化
 * @param void
 * @retval void
 */
static void MouseController_InitializeMTU2(void)
{
	MSTP(MTU2) = 0;				// モジュールストップ状態の解除

	// タイマカウンタの初期化
	MTU2.TCNT = 0;

	// カウンタクロックの選択
	MTU2.TCR.BIT.TPSC = 1;		// タイマプリスケーラ:PCLK/4
	MTU2.TCR.BIT.CKEG = 0;		// 入力クロックエッジ:立ち上がりエッジでカウント

	// カウンタクリア要因の選択
	MTU2.TCR.BIT.CCLR = 1;		// TGRAコンペアマッチでTCNTカウンタをクリア

	// タイマモードの選択
	MTU2.TMDR.BIT.MD = 0;		// ノーマルモード

	// TGRyの設定
	MTU2.TGRA = CONTROL_INTERVAL;

	// 割り込み許可設定
	MTU2.TIER.BIT.TGIEA = 1;	// TGIA割り込み要求を許可

	// 割り込みレベル設定
	IPR(MTU2, TGIA2)= 10;

	// 割り込み要求を許可
	IEN(MTU2, TGIA2) = 1;

	// カウント動作開始
	MTU.TSTR.BIT.CST2 = 1;		// MTU2カウント動作開始
}

/**
 * 機体パラメータの更新
 * @param void
 * @retval void
 */
static void MouseController_UpdateParameter(void)
{
	_SWORD a,b,c;
	_UWORD absa,absb,absc;
	_SWORD wheelVel;

	// 1時刻前状態の保存
	_wheelLAng.Old = _wheelLAng.Now;
	_wheelRAng.Old = _wheelRAng.Now;
	_wheelLDist.Old = _wheelLDist.Now;
	_wheelRDist.Old = _wheelRDist.Now;
	_wheelLVel.Old = _wheelLVel.Now;
	_wheelRVel.Old = _wheelRVel.Now;

	// 左右輪について計算
	for(int i = 0; i < 2; i++)
	{
		// 一時刻前との差分
		if(i == 0)
		{
			_wheelLAng.Now = AS5055_GetAngle(ENC_L);
			a = (_SWORD)_wheelLAng.Now - (_SWORD)_wheelLAng.Old;
		}
		else
		{
			_wheelRAng.Now = AS5055_GetAngle(ENC_R);
			a = (_SWORD)_wheelRAng.Now - (_SWORD)_wheelRAng.Old;
		}
		b = a + (_SWORD)(1 << ENCODER_RESOLUTION);
		c = a - (_SWORD)(1 << ENCODER_RESOLUTION);

		// 絶対値をとる
		absa = ((a >= 0)?  a : -a);
		absb = ((b >= 0)?  b : -b);
		absc = ((c >= 0)?  c : -c);

		// 最小値が正しい変化値となる
//		if(absa <= absb)
//		{
//			if(absa <= absc)	wheelVel = a;
//			else				wheelVel = c;
//		}
//		else if(absb <= absc)	wheelVel = b;
//		else					wheelVel = c;
		wheelVel = ((absa >= absb)?  ((absb >= absc)?	c : b)  : (absa >= absc)?  c : a);

		if(i == 0)
		{
			_wheelLDist.Now = (float)wheelVel / (float)(1 << ENCODER_RESOLUTION) * PI * HW_WHEEL_DIAM;
			_wheelLVel.Now = _wheelLDist.Now * (float)CONTROL_INTERVEL_INVSEC;
		}
		else
		{
			_wheelRDist.Now = -(float)wheelVel / (float)(1 << ENCODER_RESOLUTION) * PI * HW_WHEEL_DIAM;
			_wheelRVel.Now = _wheelRDist.Now * (float)CONTROL_INTERVEL_INVSEC;
		}
	}

	// 機体速度(物理量)の計算
	_vel.Old = _vel.Now;
	_angvel.Old = _angvel.Now;
	_vel.Now = (_wheelLVel.Now + _wheelRVel.Now) / 2;
	_angvel.Now = (_wheelLVel.Now - _wheelRVel.Now) / (2 * HW_TREAD_WIDTH);
}

static void MouseController_SideWallControl()
{
	float dl, dr;
//	float kp = 0.01, kd = 0.0;						// 比例,微分制御係数格納変数
	//float v;
	_SWORD ctrlRefMinL, ctrlRefMinR;				// 制御基準値格納変数

	// ==== 横壁制御フラグがあれば制御 ====
	if( _MF.CTRL.BIT.SIDE ){

		// 制御基準下限値代入
		ctrlRefMinL = CTRL_REF_MIN_L;
		ctrlRefMinR = CTRL_REF_MIN_R;

		// 現在の速度に応じて制御系数を決定する
		//   速度が速いほど　kp小，kd大
		//	 速度が遅いほど　kp大，kd小


/*		// 壁の切れ目で急激な制御をかけないための対策処理
		// センサ微分値が基準値以上なら制御下限値をベースより少し大きな値にする
		if( AD.Delta.Left > EDGE_REF_DIF_L ){
			ctrlRefMinL = AD.Base.Left + 50;
		}
		if( AD.Delta.Right > EDGE_REF_DIF_R ){
			ctrlRefMinR = AD.Base.Right + 50;
		}
*/
		// 壁切れ検知(壁有り->壁なしのとき検知)
//		if( MF.CTRL.BIT.EDGT ){
//			if( -AD.Delta.Left > EDGE_REF_DELTA_L){
//				MF.CTRL.BIT.EDGF = 1;	// 壁切れ
//				Sound.Play(1, 50);
//			}
//			if( -AD.Delta.Right > EDGE_REF_DELTA_R){
//				MF.CTRL.BIT.EDGF = 1;	// 壁切れ
//				Sound.Play(4, 50);
//			}
//		}
//		else{
//			MF.CTRL.BIT.EDGF = 0;
//		}

		// ---- 制御値の決定 ----
		LSVal *lsv = LightSensor_GetValue();

		// 左右センサ(基準からの)差分値が共に制御基準範囲に収まっている時
		if( ( (ctrlRefMinL <= lsv->Dif.Left) && (lsv->Dif.Left <= CTRL_REF_MAX_L) ) && ( (ctrlRefMinR <= lsv->Dif.Right) && (lsv->Dif.Right <= CTRL_REF_MAX_R ) ) ){
			dl = kp * (lsv->Dif.Left - lsv->Dif.Right) + kd * lsv->Delta.Left;
			dr = kp * (lsv->Dif.Right - lsv->Dif.Left) + kd * lsv->Delta.Right;
		}
		// 左右センサ差分値が共に制御基準範囲に収まっていない時
		else if(((ctrlRefMinL > lsv->Dif.Left) || (lsv->Dif.Left > CTRL_REF_MAX_L)) && ((ctrlRefMinR > lsv->Dif.Right) || (lsv->Dif.Right > CTRL_REF_MAX_R))){
			dl = dr = 0;					//制御をかけない
		}
		// 左センサ差分値だけ制御基準範囲に収まっている時
		else if((ctrlRefMinL <= lsv->Dif.Left) && (lsv->Dif.Left <= CTRL_REF_MAX_L)){
			dl =  kp * (1 * lsv->Dif.Left) + kd * lsv->Delta.Left;
			dr = -kp * (1 * lsv->Dif.Left) - kd * lsv->Delta.Left;
		}
		// 右センサ差分値だけ制御基準範囲に収まっている時
		else{
			dl = -kp * (1 * lsv->Dif.Right) - kd * lsv->Delta.Right;
			dr =  kp * (1 * lsv->Dif.Right) + kd * lsv->Delta.Right;
		}
	}

	else{
		// 制御フラグがなければ制御値0
		dl = dr = 0;
	}

	_dl = dl;
	_dr = dr;
}

static void DriveA(float v0, float vmax, float acc, float dist)
{
	_t = 0;
	_x = 0;
	_tarv = v0;
	_tarvmax = vmax;
	_taracc = acc;
	_tarangv = 0;
	_tarangvmin = 0;
	_tarangvmax = 0;
	_tarangacc = 0;

	_driving = true;

	while(_x < dist)
	{
		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
		WaitUS(1);
	}

	_driving = false;
}

static void DriveD(float vmin, float acc, float dist, bool rs)
{
	_t = 0;
	_x = 0;
	_tarvmin = vmin;
	_taracc = acc;
	_tarangv = 0;
	_tarangvmin = 0;
	_tarangvmax = 0;
	_tarangacc = 0;

	_driving = true;

	while(_x < dist)
	{
		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
		WaitUS(1);
	}

	if(rs)
	{
		_tarv = 0;
		_tarvmin = 0;
		_tarvmax = 0;
		_taracc = 0;
	}

	_driving = false;
}

static void DriveTime(float v0, float ms)
{
	_t = 0;
	_x = 0;
	_tarv = v0;
	_tarvmax = v0;
	_taracc = 0;
	_tarangv = 0;
	_tarangvmin = 0;
	_tarangvmax = 0;
	_tarangacc = 0;

	_driving = true;

	while(_t * CONTROL_INTERVAL_MSEC < ms)
	{
		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
		WaitUS(1);
	}

	_tarv = 0;
	_tarvmax = 0;

	_driving = false;
}

static void TurnA(float angv0, float angvmax, float angacc, float dist)
{
	_t = 0;
	_x = 0;
	_tarv = 0;
	_tarvmin = 0;
	_tarvmax = 0;
	_taracc = 0;
	_tarangv = angv0;
	_tarangvmin = angv0;
	_tarangvmax = angvmax;
	_tarangacc = angacc;

	_driving = true;

	while(_x < dist)
	{
		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
		WaitUS(1);
	}
//	Printf("_x:%f\n", _x);

	_driving = false;
}

static void TurnD(float angvmin, float angacc, float dist, bool rs)
{
	_t = 0;
	_x = 0;
	_tarv = 0;
	_tarvmin = 0;
	_tarvmax = 0;
	_taracc = 0;
	_tarangvmin = angvmin;
	_tarangvmax = _tarangv;
	_tarangacc = angacc;

	_driving = true;

	while(_x < dist)
	{
		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
		WaitUS(1);
	}

	if(rs)
	{
		_tarangv = 0;
		_tarangvmin = 0;
		_tarangvmax = 0;
		_tarangacc = 0;
	}
//	Printf("_x:%f\n", _x);

	_driving = false;
}

//static void Slalom(float v0, float angv0, float angvmax, float angacc, float distB, float )
//{
//	_t = 0;
//	_x = 0;
//	_tarv = v0;
//	_tarvmin = v0;
//	_tarvmax = DEF_ANGVMAX;
//	_taracc = 0;
//	_tarangv = 0;
//	_tarangvmax = 0;
//	_tarangacc = 0;
//
//	_driving = true;
//
//	while(_x < dist)
//	{
//		// 何かしら処理がないと無限ループに陥るのでウェイトを入れている
//		WaitUS(1);
//	}
//	_tarangv = angv0;
//	_tarangvmax = angvmax;
//	_tarangacc = angacc;
//
//
//	_driving = false;
//}

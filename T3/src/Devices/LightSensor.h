/**
 * @file  LightSensor.h
 * @brief 光センサの動作を司るクラス
 */

#ifndef __LIGHTSENSOR_H__
#define __LIGHTSENSOR_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"
#include <stdbool.h>

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define IR_LED_FWD_L	PORTA.PODR.BIT.B6
#define IR_LED_LEFT		PORTA.PODR.BIT.B4
#define IR_LED_RIGHT	PORTA.PODR.BIT.B3
#define IR_LED_FWD_R	PORTA.PODR.BIT.B1

// 赤外LEDを発光させてからフォトトランジスタ電圧のA/D変換を開始するまでの遅れ時間
//   600: 50[usec]
#define PT_DELAY		600
// 一つのセンサに対するA/D値を取得してから次のA/D値を取得するまでの間隔
//   2400: 200[usec]
#define GET_INTERVAL	2400

// 一つのA/Dチャンネルに対し指定した回数分A/D変換を行い,その加算値をA/D値として得る(1~4で指定)
#define ADC_SAMPLING_NUM	4

// 壁判断基準値
#define WALL_BASE_FWD_L		2200
#define WALL_BASE_LEFT		2800
#define WALL_BASE_RIGHT		2800
#define WALL_BASE_FWD_R		2200

/*----------------------------------------------------------------------
	Enum Definitions
 ----------------------------------------------------------------------*/
typedef enum eLightSensorChannel
{
	LS_FWD_L = 0,
	LS_LEFT = 1,
	LS_RIGHT = 2,
	LS_FWD_R = 3,
}E_LS_CHANNEL;

/*----------------------------------------------------------------------
	Struct Definitions
 ----------------------------------------------------------------------*/
// ==== 光センサ各チャンネル取得値格納用 ====
typedef struct stLightSensorChannel{
	_SWORD Left;
	_SWORD Right;
	_SWORD FwdL;
	_SWORD FwdR;
}LSChannel;

// ==== 各光センサ値分類用 ====
typedef struct stLightSensorVal{
	LSChannel Base;				// 基準値
	LSChannel Now;				// 現在値
	LSChannel Old;				// 過去値
	LSChannel Dif;				// 現在値と基準値の差
	LSChannel Delta;			// 現在値と過去値の差
}LSVal;

/*----------------------------------------------------------------------
	Enum Definitions
 ----------------------------------------------------------------------*/
// IR LEDオン/オフ状態
typedef enum eIrLedState
{
	IR_LED_ON = 1,
	IR_LED_OFF = 0
}E_IR_LED_STATE;

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void LightSensor_Initialize(void);
#pragma inline(LightSensor_IntDMAC0)
void LightSensor_IntDMAC0(void);
LSVal* LightSensor_GetValue(void);
void LightSensor_GetBaseLR(void);
void LightSensor_ValueCheckMode(bool scion);
float Battery_GetValue(void);

#endif

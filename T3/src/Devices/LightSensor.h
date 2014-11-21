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

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define IR_LED0	PORTA.PODR.BIT.B6
#define IR_LED1	PORTA.PODR.BIT.B4
#define IR_LED2	PORTA.PODR.BIT.B3
#define IR_LED3	PORTA.PODR.BIT.B1

// 赤外LEDを発光させてからフォトトランジスタ電圧のA/D変換を開始するまでの遅れ時間
//   600: 50[usec]
#define PT_DELAY		600
// 一つのセンサに対するA/D値を取得してから次のA/D値を取得するまでの間隔
//   2400: 200[usec]
#define GET_INTERVAL	2400

// 一つのA/Dチャンネルに対し指定した回数分A/D変換を行い,その加算値をA/D値として得る(1~4で指定)
#define ADC_SAMPLING_NUM	4

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
void LightSensor_IntDMAC0(void);
_SWORD LightSensor_GetADValue(_UBYTE ch);

#endif

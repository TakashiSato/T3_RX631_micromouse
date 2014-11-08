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
#define IRLED0	PORTA.PODR.BIT.B1
#define IRLED1	PORTA.PODR.BIT.B3
#define IRLED2	PORTA.PODR.BIT.B4
#define IRLED3	PORTA.PODR.BIT.B6

#define IRLED_ON	1
#define IRLED_OFF	0

// 赤外LEDを発光させてからフォトトランジスタ電圧のA/D変換を開始するまでの遅れ時間
//   600: 50[usec]
#define PT_DELAY		600
// 一つのセンサに対するA/D値を取得してから次のA/D値を取得するまでの間隔
//   2400: 200[usec]
#define GET_INTERVAL	2400

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void LightSensor_Initialize(void);
void LightSensor_IntDMAC0(void);
_UWORD LightSensor_GetADValue(void);

#endif

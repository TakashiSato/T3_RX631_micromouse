/**
 * @file  BatteryWatch.h
 * @brief LiPOバッテリー電圧監視を行うためのクラス
 */

#ifndef __BATTERY_WATCH_H__
#define __BATTERY_WATCH_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void BatteryWatch_Initialize(void);
void BatteryWatch_IntCMT0(void);

#endif

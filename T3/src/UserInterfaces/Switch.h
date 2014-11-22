/**
 * @file  Switch.h
 * @brief スイッチの動作を司るクラス
 */

#ifndef __SWITCH_H__
#define __SWITCH_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"
#include "stdbool.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define SW 						!PORTC.PIDR.BIT.B7
#define SW_WATCH_INTERVAL		50	// スイッチ入力監視周期[ms]
#define SW_SHORT_PRESS_COUNT	1	// 短押し判定する繰り返し周期回数
#define SW_MIDDLE_PRESS_COUNT	4	// 中押し判定する繰り返し周期回数
#define SW_LONG_PRESS_COUNT		10	// 長押し判定する繰り返し周期回数

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void Switch_Initialize(void);
bool Switch_GetState(void);
_UBYTE Switch_ModeSelect(void);

#endif /* __SWITCH_H__ */

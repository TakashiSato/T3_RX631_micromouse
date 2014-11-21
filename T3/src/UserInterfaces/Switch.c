/**
 * @file  Switch.c
 * @brief スイッチの動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "Switch.h"
#include "../iodefine.h"

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void Switch_InitializePort(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/**
 * スイッチを初期化する
 * @param void
 * @retval void
 */
void Switch_Initialize(void)
{
	Switch_InitializePort();
}

/**
 * スイッチの押下状態を取得する
 * @param void
 * @retval スイッチが押されていたらtrue
 */
bool Switch_GetState(void)
{
	if(SW)
	{
		return true;
	}
	return false;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
static void Switch_InitializePort(void)
{
	PORTC.PDR.BIT.B7 = 0;
}

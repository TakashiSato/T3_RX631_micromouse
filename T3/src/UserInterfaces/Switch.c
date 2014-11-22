/**
 * @file  Switch.c
 * @brief スイッチの動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "Switch.h"
#include "../iodefine.h"
#include "../Global.h"

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

/**
 * モードセレクト
 * @param void
 * @retval _UBYTE: モード番号(0-15)
 */
_UBYTE Switch_ModeSelect(void)
{
	_UBYTE mode = 0;
	_UINT cnt = 0;
	bool endFlag = false;

	PlaySound(300);
	PlaySound(100);
	PlaySound(300);
	WaitMS(250);

	while(!endFlag)
	{
		if(Switch_GetState())
		{
			cnt++;
		}
		else
		{
			// 中押し判定されたらモードのインクリメント
			if(cnt >= SW_MIDDLE_PRESS_COUNT)
			{
				mode--;
				if(mode == 0xFF)
				{
					mode = 0x0F;
				}
				PlaySound(200);
			}
			// 短押し判定されたらモードのインクリメント
			else if(cnt >= SW_SHORT_PRESS_COUNT)
			{
				mode++;
				if(mode == 0x10)
				{
					mode = 0;
				}
				PlaySound(100);
			}
			cnt = 0;
		}

		// 長押し判定されたらループから抜ける
		if(cnt >= SW_LONG_PRESS_COUNT)
		{
			endFlag = true;
		}

		DispLED(mode & 0x0F);
		WaitMS(SW_WATCH_INTERVAL);
	}
	DispLED(0x0F);
	PlaySound(500);
	DispLED(0x00);
	PlaySound(300);
	DispLED(0x0F);
	PlaySound(100);
	WaitMS(250);
	DispLED(0x00);

	return mode;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
static void Switch_InitializePort(void)
{
	PORTC.PDR.BIT.B7 = 0;
}

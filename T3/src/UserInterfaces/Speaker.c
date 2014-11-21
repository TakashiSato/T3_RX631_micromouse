/**
 * @file  Speaker.c
 * @brief スピーカーの動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "Speaker.h"
#include "../iodefine.h"
#include "../Global.h"

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void Speaker_InitializePort(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/**
 * スピーカーの初期化
 * @param void
 * @retval void
 */
void Speaker_Initialize(void)
{
	Speaker_InitializePort();
}

/**
 * スピーカーから音を鳴らす
 * @param freq: 鳴らす音の周波数(?)
 * @retval void
 */
void Speaker_PlaySound(_UINT freq)
{
	for (int i = 0; i < 200; i++)
	{
		SPEAKER_PORT = 1;
		WaitUS(freq);
		SPEAKER_PORT = 0;
		WaitUS(freq);
	}
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/**
 * スピーカーで使うポートの初期化
 */
static void Speaker_InitializePort(void)
{
	PORTE.PDR.BIT.B2 = 1;
	PORTE.PODR.BIT.B2 = 0;
}

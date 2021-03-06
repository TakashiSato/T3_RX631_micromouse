/**
 * @file  Timer.c
 * @brief タイマー
 */
/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include "Timer.h"
#include "../BoardDefine.h"

/*----------------------------------------------------------------------
	定数定義
 ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
	ソースコード
 ----------------------------------------------------------------------*/
/** 指定ミリ秒待つ
 * @param msec : 待つミリ秒
 * @retval void
 */
void Timer_WaitMS(_UINT msec)
{
	volatile unsigned long i, j;

	for (i = 0; i < msec; i++)
	{
		for (j = 0; j < MILLI_SEC; j++)
		{
		}
	}
}

/** 指定マイクロ秒待つ
 * @param usec : 待つマイクロ秒
 * @retval void
 */
void Timer_WaitUS(_UINT usec)
{
	volatile unsigned long i, j;

	for (i = 0; i < usec; i++)
	{
		for (j = 0; j < MICRO_SEC; j++)
		{
		}
	}
}

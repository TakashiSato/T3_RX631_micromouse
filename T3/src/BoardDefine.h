/**
 * @file  BoardDefine.h
 * @brief ボード固有情報
 */
/*----------------------------------------------------------------------
	インクルード
----------------------------------------------------------------------*/
#include <machine.h>
#include "iodefine.h"

/*----------------------------------------------------------------------
	定数定義
----------------------------------------------------------------------*/
#define	ON	0
#define	OFF	1

#define CPU_CLOCK		( 96 * 100000 )		// システムクロック
#define PCLK_CLOCK		( CPU_CLOCK / 2 )	// PCLK
#define SECOND			CPU_CLOCK
#define MILLI_SEC		( SECOND / 1000 )
#define MICRO_SEC		( MILLI_SEC / 1000 )

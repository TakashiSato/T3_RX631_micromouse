/**
 * @file  AS5055.h
 * @brief 磁気エンコーダAS5055の動作を司るクラス
 */

#ifndef __AS5055_H__
#define __AS5055_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro definitions
 ----------------------------------------------------------------------*/
// AS5055 レジスタテーブル
#define ASREG_POWER_ON_RESET			0x3F22
#define ASREG_SOFTWARE_RESET			0x3C00
#define ASREG_MASTER_RESET				0x33A5
#define ASREG_CLEAR_ERROR_REGISTER		0x3380
#define ASREG_NO_OPERATION_REGISTER		0x0000
#define ASREG_AUTOMATIC_GAIN_CONTROL	0x3FF8
#define ASREG_ANGULAR_DATA 				0x3FFF
#define ASREG_SYSTEM_CONFIGURATION		0x3F20
#define ASREG_ERROR_STATUS				0x335A

// SPI読み出しフラグ
#define AS_READ_FLAG	0x8000

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void AS5055_Initialize(void);
_UWORD AS5055_GetAngle(void);

#endif

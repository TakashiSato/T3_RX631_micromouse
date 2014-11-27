/**
 * @file  SerialPort.h
 * @brief シリアルコミュニケーションインタフェース
 */

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include <stdarg.h>
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Macro definitions
 ----------------------------------------------------------------------*/
#define SCI_INTERRUPT_LEVEL			3
#define SCI_RECV_RING_BUFFER_SIZE	16	// 受信リングバッファのサイズ
#define SCI_SEND_RING_BUFFER_SIZE	128	// 送信リングバッファのサイズ
#define C_MAX					12		// 整数表示の最大桁数
#define SCAN_MAX				20		// Scanfで入力できる数字の最大桁数
#define FRAC_LENGTH				4		// 小数点表示時のデフォルトで表示する桁数

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
void SCI_Printf(_UBYTE* str, ...);
void SCI_Scanf(_UBYTE* str, ...);
#pragma inline(SCI_IntTXI1)
void SCI_IntTXI1(void);
#pragma inline(SCI_IntTEI1)
void SCI_IntTEI1(void);
#pragma inline(SCI_IntRXI1)
void SCI_IntRXI1(void);
void SCI_InitializeSerialPort(long baudrate);

#endif

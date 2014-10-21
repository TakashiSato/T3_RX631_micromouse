/**
 * @file  SerialPort.h
 * @brief シリアルコミュニケーションインタフェース
 */

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdbool.h>
#include "../typedefine.h"
#include "../Utils/RingBuffer.h"

/*----------------------------------------------------------------------
	定数定義
 ----------------------------------------------------------------------*/
#define SCI_RECV_RING_BUFFER_SIZE	32		// 受信リングバッファのサイズ
#define SCI_SEND_RING_BUFFER_SIZE	256		// 送信リングバッファのサイズ
#define C_MAX					12		// 整数表示の最大桁数
#define SCAN_MAX				20		// Scanfで入力できる数字の最大桁数
#define FRAC_LENGTH				4		// 小数点表示時のデフォルトで表示する桁数

/*----------------------------------------------------------------------
	ソースコード
 ----------------------------------------------------------------------*/
/* ==== プライベートプロパティ ==== */
// リングバッファ
static RING_BUFFER* SCI_RxD;			// 受信データ用リングバッファ構造体
static RING_BUFFER* SCI_TxD;			// 送信データ用リングバッファ構造体

/* ==== パブリックメソッド ==== */
/** Printf関数
 * @param str : printする文字列
 * @param ... : 表示する変数
 */
void SCI_Printf(_UBYTE* str, ...);

/** Scanf関数
 * @param str : scanする文字列
 * @param ... : 格納する変数のアドレス
 */
void SCI_Scanf(_UBYTE* str, ...);

/** SCI1送信データエンプティによる割り込み
 * @param void
 * @retval void
 */
void SCI_IntTXI1(void);

/** SCI1送信終了割り込み
 * @param void
 * @retval void
 */
void SCI_IntTEI1(void);

/** SCI1受信データフルによる割り込み
 * @param void
 * @retval void
 */
void SCI_IntRXI1(void);

/** シリアルコミュニケーションインタフェースの初期化
 * @param baudrate : ボーレート[bps]
 * @retval void
 */
void SCI_InitializeSerialPort(long baudrate);

/** 一文字を送信バッファへ格納
 * @param c : 送信する1文字
 * @retval void
 */
static void SCI_PutChar(_UBYTE c);

/** 文字列を送信バッファへ格納
 * @param str : 送信する文字列
 * @retval void
 */
static void SCI_PutStr(_UBYTE *str);

/** 整数を文字に変換して送信バッファへ格納
 * @param fmt : 数値フォーマット
 * @param var : 整数値
 * @param len : 表示する整数値の長さ
 * @retval void
 */
static void SCI_PutInt(_UBYTE fmt, signed int var, short len);

/** 整数(long型)を文字に変換して送信バッファへ格納
 * @param fmt : 数値フォーマット
 * @param var : 整数値(long)
 * @param len : 表示する整数値の長さ
 * @retval void
 */
static void SCI_PutLong(_UBYTE fmt, signed long var, short len);


/** 浮動小数点数を文字に変換して送信バッファへ格納
 * @param var : 浮動小数点数
 * @param len : 表示する浮動小数点数の整数部の長さ
 * @param fracLen : 表示する浮動小数点数の小数部の長さ
 * @retval void
 */
static void SCI_PutFrac(double var, short len, short fracLen);

/** キーボードから文字列入力を受け付ける
 * @param receive : 受信した文字列を格納する変数
 * @retval void
 */
static void SCI_ScanStr(_UBYTE* receive);

/** キーボードから整数値入力を受け付ける
 * @param receive : 受信した整数値を格納する変数
 * @retval void
 */
static void SCI_ScanInt(int* receive);

/** キーボードから浮動小数点数(double型)入力を受け付ける
 * @param receive : 受信した浮動小数点数を格納する変数
 * @retval void
 */
static void SCI_ScanDouble(double* receive);

/** キーボードから浮動小数点数(float型)入力を受け付ける
 * @param receive : 受信した浮動小数点数を格納する変数
 * @retval void
 */
static void SCI_ScanFloat(float* receive);

/** 一文字受信バッファから読み出し
 * @param buf : 読みだした文字を格納する変数のポインタ
 * @retval bool : 成功すればtrueを返す
 */
static bool SCI_GetChar(_UBYTE* buf);

/** 受信バッファをフラッシュ
 * @param void
 * @retval void
 */
static void SCI_BufferFlash(void);

#endif

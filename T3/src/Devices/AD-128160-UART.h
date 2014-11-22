/**
 * @file  AD-128160-UART.h
 * @brief TFT液晶 AD-128160-UART駆動
 */

// インクルードガード
#ifndef __AD128160UART_H
#define __AD128160UART_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*============================================================
		ヘッダファイルのインクルード
============================================================*/
#include <stdarg.h>
#include "../typedefine.h"
#include "../Utils/RingBuffer.h"

/*============================================================
		マクロの定義
============================================================*/
#define AD128160_SEND_RING_BUFFER_SIZE	512		// 送信リングバッファのサイズ
#define AD128160_C_MAX					12		// 整数表示の最大桁数
#define AD128160_FRAC_LENGTH			4		// 小数点表示時のデフォルトで表示する桁数

/*============================================================
		構造体の定義
============================================================*/
typedef volatile struct stAD128160_MousePosition
{
	_UBYTE x;		// x座標(0-15)
	_UBYTE y;		// y座標(0-15)
}AD128160_MOUSE_POS;

/*============================================================
		列挙型の定義
============================================================*/
#ifndef ENUM_DIRECTION
#define ENUM_DIRECTION
typedef enum eDir
{
	NORTH = 1,
	EAST = 2,
	WEST = 4,
	SOUTH = 8
}DIRECTION;
#endif

/*============================================================
		グローバル変数の定義
============================================================*/
// リングバッファ
static RING_BUFFER* AD128160_TxD;			// 送信データ用リングバッファ構造体

static AD128160_MOUSE_POS AD128160_MousePos;

// 行，列
static _SBYTE _row, _col;

/*============================================================
		関数の定義
============================================================*/
/** シリアルコミュニケーションインタフェースの初期化
 * @param baudrate : ボーレート[bps]
 * @retval void
 */
void AD128160_InitializeSerialPort(long baudrate);

/** SCI1送信終了割り込み
 * @param void
 * @retval void
 */
#pragma inline(AD128160_IntTEI1)
void AD128160_IntTEI1(void);

// ==== 送信データエンプティによる割り込み ====
#pragma inline(AD128160_IntTXI1)
void AD128160_IntTXI();

// ==== 1byte送信バッファへ格納 ====
void AD128160_SendByte(_UBYTE c);

// ==== コマンド（複数バイト）を送信バッファへ格納 ====
void AD128160_SendCommand(_UBYTE* sendDataArray, _UBYTE sendSize);

// ==== AD128160初期化 ====
void AD128160_Initialize();

// ==== 画面表示クリア ====
void AD128160_Clear();

// ==== バックライト明るさ設定 ===
void AD128160_Brightness(_UWORD brightness);

// ==== 2バイトRGBをつくるための関数 ====
_UBYTE AD128160_RGB565(_UBYTE r, _UBYTE g, _UBYTE b);

// ==== カラー設定 ====
void AD128160_SetColor(_UBYTE rgb);

// ==== 文字背景色設定 ====
void AD128160_SetBackGroundColor(int rgb);

// ==== 文字背景色を透明に設定 ====
void AD128160_ClearBackGroundColor();

// ==== 転送済みのBMPを表示する ====
void AD128160_DisplayBMP(_UBYTE x0, _UBYTE y0, _UBYTE bmpNo);

// ==== Baudrateを設定する ====
void AD128160_SetBaudrate(_UDWORD baudrate);

// ==== ドット描画 ====
void AD128160_DrawDot(_UBYTE x0, _UBYTE y0);

// ==== 線描画 ====
void AD128160_DrawLine(_UBYTE x0, _UBYTE y0, _UBYTE x1, _UBYTE y1);

// ==== 四角描画 ====
void AD128160_DrawBox(_UBYTE x0, _UBYTE y0, _UBYTE x1, _UBYTE y1, _UBYTE paint);

// ==== 円描画 ====
void AD128160_DrawCircle(_UBYTE x0, _UBYTE y0, _UBYTE radius, _UBYTE paint);

// ==== 文字表示位置を設定 ====
void AD128160_Locate(_UBYTE row, _UBYTE col);

// ==== 改行する ====
void AD128160_NewLine();

// ==== 一文字表示 ====
void AD128160_PutChar(char c);

// ==== 文字列表示 ====
void AD128160_PutStr(char* str);

// ==== 整数表示(32bit型まで) ====
void AD128160_PutInt(char fmt, _SDWORD var, _SBYTE len);

// ==== 整数表示(64bit型まで) ====
void AD128160_PutLong(_SQWORD var, _SBYTE len);

// ==== 浮動小数点数表示 ====
void AD128160_PutDouble(double var, _SBYTE len, _SBYTE fracLen);

// ==== AD128160用Printf関数 ====
void AD128160_Printf(_UBYTE* str, ...);

// ==== 整数表示特化関数 ====
void AD128160_DisplasyNum(_UBYTE x, _UBYTE y, signed int num, unsigned char len, unsigned char type);

// ==== 迷路上x座標取得 ====
_UBYTE AD128160_GetX();

// ==== 迷路上y座標取得 ====
_UBYTE AD128160_GetY();

// ==== 迷路上(x, y)座標セット ====
void AD128160_SetCoordinate(_SBYTE x, _SBYTE y);

// ==== 迷路表示初期化 ====
void AD128160_InitLabyrinth();

// ==== 迷路壁描画 ====
void AD128160_CreateWall(_UBYTE x, _UBYTE y, DIRECTION dir);

// ==== 指定方角に1歩進める ====
void AD128160_Advance(DIRECTION dir);

#endif

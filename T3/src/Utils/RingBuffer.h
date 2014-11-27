/**
 * @file  RingBuffer.h
 * @brief リングバッファを提供するクラス
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include <stdbool.h>
#include "../typedefine.h"

/*----------------------------------------------------------------------
	Typedef Definitions
 ----------------------------------------------------------------------*/
/** リングバッファの機能を提供するクラスっぽい構造体
 */
typedef struct stRingbuffer
{
	// プロパティ
	_UWORD size;		// バッファサイズ
	_UWORD remain;		// バッファ中の残データ数
	_UWORD read;		// 読み込み位置
	_UWORD write;		// 書き込み位置
	_UBYTE *buff;		// 読み書きデータ

	// メソッド
	void (*Dispose)(struct stRingbuffer* self);
	bool (*Add)(struct stRingbuffer* self, _UWORD data);
	_UBYTE (*Pull)(struct stRingbuffer* self);
	void (*Flash)(struct stRingbuffer* self);
}RING_BUFFER;

/*----------------------------------------------------------------------
	Public Method Declarations
 ----------------------------------------------------------------------*/
/** リングバッファのインスタンスを生成する
 * @param size : 生成するリングバッファのバッファサイズ
 * @retval RING_BUFFER* : 生成したリングバッファのインスタンス
 */
RING_BUFFER* NewRingBuffer(_UWORD size);

#endif

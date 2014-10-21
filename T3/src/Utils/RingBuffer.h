/**
 * @file  RingBuffer.h
 * @brief リングバッファを提供するクラス
 */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__
/*----------------------------------------------------------------------
	インクルード
 ----------------------------------------------------------------------*/
#include <stdbool.h>
#include "../typedefine.h"

/*----------------------------------------------------------------------
	構造体の定義
 ----------------------------------------------------------------------*/
/** リングバッファの機能を提供する構造体
 */
typedef volatile struct stRingbuffer
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
	パブリックメソッド
 ----------------------------------------------------------------------*/
/** リングバッファのインスタンスを生成する
 * @param size : 生成するリングバッファのバッファサイズ
 * @retval RING_BUFFER* : 生成したリングバッファのインスタンス
 */
RING_BUFFER* NewRingBuffer(_UWORD size);

/*----------------------------------------------------------------------
	プライベートメソッド
 ----------------------------------------------------------------------*/
/** リングバッファに割り当てたメモリの解放を行う
 * @param rb : 処理を行うリングバッファ構造体のポインタ
 * @retval void
 */
void RingBuffer_Dispose(RING_BUFFER* rb);

/** リングバッファにデータを追加する
 * @param rb : 対象リングバッファ構造体のポインタ
 * @param data: 追加するデータ(1byte)
 * @retval 成功したらtrueを返す
 */
bool RingBuffer_Add(RING_BUFFER* rb, _UBYTE data);

/** リングバッファからデータを取り出す
 * @param rb : 対象リングバッファ構造体のポインタ
 * @retval 取り出されたデータ
 */
_UBYTE RingBuffer_Pull(RING_BUFFER* rb);

/** リングバッファ内のデータを全消去する
 * @param rb : 対象リングバッファ構造体のポインタ
 * @retval void
 */
void RingBuffer_Flash(RING_BUFFER* rb);

#endif

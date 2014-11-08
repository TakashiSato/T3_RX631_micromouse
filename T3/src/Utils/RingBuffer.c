/**
 * @file  RingBuffer.c
 * @brief リングバッファを提供するクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "RingBuffer.h"
#include "stdlib.h"

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void RingBuffer_Dispose(RING_BUFFER* rb);
static bool RingBuffer_Add(RING_BUFFER* rb, _UBYTE data);
static _UBYTE RingBuffer_Pull(RING_BUFFER* rb);
static void RingBuffer_Flash(RING_BUFFER* rb);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/** リングバッファのインスタンスを生成する
 * @param size : 生成するリングバッファのバッファサイズ
 * @retval RING_BUFFER* : 生成したリングバッファのインスタンス
 */
RING_BUFFER* NewRingBuffer(_UWORD size)
{
	RING_BUFFER* rb = NULL;

	rb = malloc(sizeof(RING_BUFFER));
	if(rb == NULL)
	{
		return NULL;
	}

	rb->size = size;
	rb->remain = 0;
	rb->read = 0;
	rb->write = 0;
	rb->buff = (_UBYTE*)malloc(sizeof(_UBYTE) * size);

	rb->Dispose = RingBuffer_Dispose;
	rb->Add = RingBuffer_Add;
	rb->Pull = RingBuffer_Pull;
	rb->Flash = RingBuffer_Flash;

	return rb;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** リングバッファに割り当てたメモリの解放を行う
 * @param rb : 処理を行うリングバッファ構造体のポインタ
 * @retval void
 */
static void RingBuffer_Dispose(RING_BUFFER* rb)
{
	free(rb->buff);
}

/** リングバッファにデータを追加する
 * @param data: 追加するデータ(1byte)
 * @retval 成功したらtrueを返す
 */
static bool RingBuffer_Add(RING_BUFFER* rb, _UBYTE data)
{
	// バッファフルでなければデータを追加
	if(rb->remain < rb->size)
	{
		rb->buff[rb->write] = data;		// データをバッファに追加
		rb->remain++;				// データ残数のインクリメント
		rb->write++;				// 書き込み位置のインクリメント

		// 書き込み位置とバッファサイズの比較
		if(rb->size <= rb->write)
		{
			rb->write = 0;			// 大きければ書き込み位置を先頭に戻す
		}
	}
	// バッファフルならfalseを返す
	else
	{
		return false;
	}

	return true;
}

/** リングバッファからデータを取り出す
 * @param void
 * @retval 取り出されたデータ
 */
static _UBYTE RingBuffer_Pull(RING_BUFFER* rb)
{
	_UBYTE data = 0x00;

	// データ残量が0より大きければ取り出し
	if(rb->remain > 0)
	{
		data = rb->buff[rb->read];		// バッファからデータを読み出し
		rb->remain--;				// データ残数のデクリメント
		rb->read++;					// 読み出し位置のインクリメント

		// 読み出し位置とバッファサイズの比較
		if(rb->read >= rb->size)
		{
			rb->read = 0;			// 大きければ読み出し位置を先頭に戻す
		}
	}

	return data;
}

/** リングバッファ内のデータを全消去する
 * @param void
 * @retval void
 */
static void RingBuffer_Flash(RING_BUFFER* rb)
{
	rb->read = 0;
	rb->write = 0;
	rb->remain = 0;
}

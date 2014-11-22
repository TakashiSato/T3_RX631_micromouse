/**
 * @file  SerialPort.c
 * @brief シリアルコミュニケーションインタフェース
 */
/*----------------------------------------------------------------------
 インクルード
 ----------------------------------------------------------------------*/
#include "SerialPort.h"
#include "../iodefine.h"

/*----------------------------------------------------------------------
 パブリックメソッド
 ----------------------------------------------------------------------*/
/** Printf関数
 * @param str : printする文字列
 * @param ... : 表示する変数
 */
void SCI_Printf(_UBYTE* str, ...)
{
	va_list args;
	_UBYTE *ptr;
	short len = 0;
	short fracLen = FRAC_LENGTH;	// 小数点表示時の小数点以下桁数（デフォルトでは2）

	va_start(args, str);			// 可変引数の初期化
	// 一文字ずつフォーマット判別
	for (ptr = str; *ptr != '\0'; ptr++)
	{
		// 変数指定子のとき
		if (*ptr == '%')
		{
			ptr++;

			// 表示文字数指定の判別
			if (*ptr == '.')
			{
				ptr++;
				if ((*ptr >= '1') && (*ptr <= '9'))
				{
					fracLen = *ptr++ - '0';					// 数字の文字を数値に変換
					while ((*ptr >= '0') && (*ptr <= '9'))
						fracLen = fracLen * 10 + (*ptr++ - '0');
				}
			}
			else if ((*ptr >= '1') && (*ptr <= '9'))
			{
				len = *ptr++ - '0';					// 数字の文字を数値に変換
				if (*ptr == '.')
				{
					ptr++;
					if ((*ptr >= '1') && (*ptr <= '9'))
					{
						fracLen = *ptr++ - '0';					// 数字の文字を数値に変換
						while ((*ptr >= '0') && (*ptr <= '9'))
							fracLen = fracLen * 10 + (*ptr++ - '0');
					}
				}
				else
				{
					while ((*ptr >= '0') && (*ptr <= '9'))
						len = len * 10 + (*ptr++ - '0');
				}
			}
			else len = -1;

			switch (*ptr)
			{
				case 's':
					SCI_PutStr(va_arg(args, _UBYTE*));
					break;
				case 'c':
					SCI_PutChar(( va_arg(args, signed int) & 0xFF));
					break;
				case 'd':
				case 'x':
					SCI_PutInt(*ptr, va_arg(args, signed int), len);
					break;
				case 'l':
					SCI_PutLong(*ptr, va_arg(args, signed long), len);
					break;
				case 'f':
					SCI_PutFrac(va_arg(args, double), len, fracLen);
					break;
			}
		}
		else if (*ptr == '\n')
		{
			SCI_PutChar(0x0a);			// LFラインフィード
			SCI_PutChar(0x0d);			// CRキャリッジリターン
		}
		else SCI_PutChar(*ptr);
	}va_end(args);					// 可変引数の終了処理
}

/** Scanf関数
 * @param str : scanする文字列
 * @param ... : 格納する変数のアドレス
 */
void SCI_Scanf(_UBYTE* str, ...)
{
	va_list args;
	_UBYTE *ptr;

	va_start(args, str);		// 可変引数の初期化

	// 一文字ずつフォーマット判別
	for (ptr = str; *ptr != '\0'; ptr++)
	{

		// 変数指定子
		if (*ptr == '%')
		{
			ptr++;

			switch (*ptr)
			{
				case 's':
					SCI_ScanStr(va_arg(args, _UBYTE*));
					break;
				case 'd':
					SCI_ScanInt(va_arg(args, int*));
					break;
				case 'f':
					SCI_ScanFloat(va_arg(args, float*));
					break;
				case 'l':
					ptr++;
					if (*ptr == 'f')
					{
						SCI_ScanDouble(va_arg(args, double*));
					}
					break;
				default:
					va_arg(args, void*);
					break;
			}
		}
	}va_end(args);					// 可変引数の終了処理
}

/** SCI1送信データエンプティによる割り込み
 * @param void
 * @retval void
 */
void SCI_IntTXI1(void)
{
	_UBYTE data;

	// データ残量が0より大きければ(送信するデータが残っていたら)
	if (SCI_TxD->remain > 0)
	{
		data = SCI_TxD->Pull(SCI_TxD);	// バッファからデータを読出し
		SCI1.TDR = data;							// 読み出したデータを送信レジスタへ。
	}
	else
	{
		SCI1.SCR.BIT.TIE = 0;	// データ残がなければ送信データエンプティ割り込み禁止
		SCI1.SCR.BIT.TEIE = 1;	// 送信終了割り込み許可
	}
}

/** SCI1送信終了割り込み
 * @param void
 * @retval void
 */
void SCI_IntTEI1(void)
{
	SCI1.SCR.BIT.TEIE = 0;			// 送信終了割り込み禁止
	SCI1.SCR.BIT.TE = 0;			// シリアル送信動作を停止
}

/** SCI1受信データフルによる割り込み
 * @param void
 * @retval void
 */
void SCI_IntRXI1(void)
{
	_UBYTE data;

	data = SCI1.RDR;					// 受信データ取り込み
	SCI_RxD->Add(SCI_RxD, data);		// 受信バッファに追加
}

/*----------------------------------------------------------------------
 プライベートメソッド
 ----------------------------------------------------------------------*/
/** シリアルコミュニケーションインタフェースの初期化
 * @param baudrate : ボーレート[bps]
 * @retval void
 */
void SCI_InitializeSerialPort(long baudrate)
{
	MSTP(SCI1) = 0;					// SCI1モジュールストップ状態の解除
	SCI1.SCR.BYTE &= 0x0B;			// TEIE,RE,TE,RIE,TIEを禁止

	// ピン機能設定
	PORT2.PMR.BIT.B6 = 0;			// P26を汎用入出力ポートに設定
	PORT3.PMR.BIT.B0 = 0;			// P30を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;			// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;			// PFSレジスタへの書き込みを許可
	MPC.P26PFS.BIT.PSEL = 10;		// P26の機能選択:TXD1
	MPC.P30PFS.BIT.PSEL = 10;		// P30の機能選択:RXD1
	MPC.PWPR.BYTE = 0x80;			// PFSレジスタ,PFSWEビットへの書き込みを禁止
	PORT2.PMR.BIT.B6 = 1;			// P26を周辺機能として使用
	PORT3.PMR.BIT.B0 = 1;			// P30を周辺機能として使用

	SCI1.SCR.BIT.CKE = 0;			// クロック:内蔵ボーレートジェネレータ
	SCI1.SIMR1.BIT.IICM = 0;		// シリアルインタフェースモード
	SCI1.SPMR.BIT.CKPH = 0;			// クロック極性反転なし
	SCI1.SPMR.BIT.CKPOL = 0;		// クロック遅れなし

	// 送信/受信フォーマット設定
	SCI1.SMR.BYTE = 0x00;			// クロック:PCLK/1,1stop bit, no parity, 8bit

	// ボーレート設定
	switch(baudrate)
	{
		case 9600:
			SCI1.BRR = 155;	// Baudrate: 9600[bps](誤差0.16[%])
			break;
		case 19200:
			SCI1.BRR = 77;	// Baudrate: 19200[bps](誤差0.16[%])
			break;
		case 38400:
			SCI1.BRR = 38;	// Baudrate: 38400[bps](誤差0.16[%])
			break;
		case 57600:
			SCI1.BRR = 25;	// Baudrate: 57600[bps](誤差0.16[%])
			break;
		case 115200:
			SCI1.BRR = 12;	// Baudrate: 115200[bps](誤差0.16[%])
			break;
		case 250000:
			SCI1.BRR = 5;	// Baudrate: 250000[bps](誤差0.00[%])
			break;
		default:
			SCI1.BRR = 12;	// Baudrate: 115200[bps](誤差0.16[%])
			break;
	}

	// シリアル受信動作を開始
	SCI1.SCR.BIT.RE = 1;			// シリアル受信動作を許可
	SCI1.SCR.BIT.RIE = 1;			// RXI割り込み要求を許可

	// 割り込みレベル設定
	IPR(SCI1, RXI1)= SCI_INTERRUPT_LEVEL;
	IPR(SCI1, TXI1)= SCI_INTERRUPT_LEVEL;
	IPR(SCI1, TEI1)= SCI_INTERRUPT_LEVEL;

	// 割り込み要求を許可
	IEN(SCI1, RXI1) = 1;
	IEN(SCI1, TXI1) = 1;
	IEN(SCI1, TEI1) = 1;

	// リングバッファ初期化
	SCI_TxD = NewRingBuffer(SCI_SEND_RING_BUFFER_SIZE);
	SCI_RxD = NewRingBuffer(SCI_RECV_RING_BUFFER_SIZE);
}

/** 一文字を送信バッファへ格納
 * @param c : 送信する1文字
 * @retval void
 */
static void SCI_PutChar(_UBYTE c)
{
	// 送信バッファにデータを追加
	SCI_TxD->Add(SCI_TxD, c);

	// 送信動作停止中であれば送信開始
	if(SCI1.SCR.BIT.TE == 0)
	{
		SCI1.SCR.BIT.TIE = 1;			// 送信割り込み許可
		SCI1.SCR.BIT.TE = 1;			// シリアル送信動作を開始
	}
}

/** 文字列を送信バッファへ格納
 * @param str : 送信する文字列
 * @retval void
 */
static void SCI_PutStr(_UBYTE *str)
{
	// Nullの検出で終了
	while (*str != '\0')
	{
		if (*str == '\n')
		{	// 改行処理
			SCI_PutChar(0x0a);			// LFラインフィード
			SCI_PutChar(0x0d);			// CRキャリッジリターン
		}
		else
		{
			SCI_PutChar(*str);			// 一文字送信処理
		}
		str++;						// ポインタのインクリメント
	}
}

/** 整数を文字に変換して送信バッファへ格納
 * @param fmt : 数値フォーマット
 * @param var : 整数値
 * @param len : 表示する整数値の長さ
 * @retval void
 */
static void SCI_PutInt(_UBYTE fmt, signed int var, short len)
{
	_UBYTE s[C_MAX], *ptr, z;
	_UBYTE num[] = "0123456789ABCDEF";
	unsigned short b;

	// 正負判定
	if (var < 0)
	{
		SCI_PutChar('-');
		var = -var;
		len--;
	}

	// 数値フォーマット判定
	switch (fmt)
	{
		// 10進数
		case 'd':
			b = 10;
			z = ' ';
			break;
			// 16進数
		case 'x':
			b = 16;
			z = '0';
			SCI_PutStr("0x");
			break;
		default:
			return;
	}

	ptr = s;

	// フォーマットに応じて数値を文字に変換
	do
	{
		len--;
		*ptr++ = num[var % b];
		var /= b;
	} while (var != 0);

	// 桁数合わせ
	for (; len > 0; len--)
		SCI_PutChar(z);

	while (ptr > s)
		SCI_PutChar(*--ptr);
}

/** 整数(long型)を文字に変換して送信バッファへ格納
 * @param fmt : 数値フォーマット
 * @param var : 整数値(long)
 * @param len : 表示する整数値の長さ
 * @retval void
 */
static void SCI_PutLong(_UBYTE fmt, signed long var, short len)
{
	_UBYTE s[C_MAX], *ptr;
	_UBYTE num[] = "0123456789abcdef";

	// 正負判定
	if (var < 0)
	{
		SCI_PutChar('-');
		var = -var;
		len--;
	}

	ptr = s;

	// フォーマットに応じて数値を文字に変換
	do
	{
		len--;
		*ptr++ = num[var % 10];
		var /= 10;
	} while (var != 0);

	// 桁数合わせ
	for (; len > 0; len--)
		SCI_PutChar(' ');

	while (ptr > s)
		SCI_PutChar(*--ptr);
}

/** 浮動小数点数を文字に変換して送信バッファへ格納
 * @param var : 浮動小数点数
 * @param len : 表示する浮動小数点数の整数部の長さ
 * @param fracLen : 表示する浮動小数点数の小数部の長さ
 * @retval void
 */
static void SCI_PutFrac(double var, short len, short fracLen)
{
	int i;
	_UBYTE s[C_MAX], *ptr;
	_UBYTE num[] = "0123456789";
	double index = 1.0;
	unsigned long tempVar;

	// 正負判定
	if (var < 0)
	{
		SCI_PutChar('-');
		var = -var;
		len--;
	}

	ptr = s;

	// ---- 数値を文字に変換 ----
	// 小数部の処理
	for (i = 0; i < fracLen; i++)
		index *= 10.0;
	tempVar = (unsigned long) ((var - (unsigned long) var) * index);
	do
	{
		len--;
		i--;
		*ptr++ = num[tempVar % 10];
		tempVar /= 10;
	} while (tempVar != 0 || i > 0);

	// 小数点の挿入
	*ptr++ = '.';

	// 整数部の処理
	tempVar = (unsigned long) var;
	do
	{
		len--;
		*ptr++ = num[tempVar % 10];
		tempVar /= 10;
	} while (tempVar != 0);

	// 桁数合わせ
	for (; len > 0; len--)
		SCI_PutChar(' ');

	while (ptr > s)
		SCI_PutChar(*--ptr);
}

/** キーボードから文字列入力を受け付ける
 * @param receive : 受信した文字列を格納する変数
 * @retval void
 */
static void SCI_ScanStr(_UBYTE* receive)
{
	int i = 0;
	_UBYTE c;
	bool ret;

	SCI_BufferFlash();								// バッファフラッシュ

	// Enterが押されるまでキーボード入力を受け付ける
	do
	{
		ret = SCI_GetChar(&c);						// 受信バッファからデータを取り出す
		if (ret == true)
		{
			SCI_PutChar(c);
			receive[i] = c;
			i++;
		}
	} while (c != '\r' && c != '\n');

	// 改行文字をnull文字に置換
	if (receive[i] == '\r' || receive[i] == '\n') receive[i] = '\0';
	if (i > 0 && (receive[i - 1] == '\r' || receive[i - 1] == '\n')) receive[i - 1] = '\0';

	SCI_PutChar('\n');
}

/** キーボードから整数値入力を受け付ける
 * @param receive : 受信した整数値を格納する変数
 * @retval void
 */
static void SCI_ScanInt(int* receive)
{
	int i;
	int pow = 1;
	_UBYTE c[SCAN_MAX];

	// 数値を格納する変数の初期化
	*receive = 0;

	// まず文字列の入力を受け付ける
	SCI_ScanStr(c);

	// 文字列になっている数字の桁数を調べる
	for (i = 0; c[i] != '\0'; i++)
		;

	// 数字の文字列を数値に変換
	for (i = i - 1; i >= 0; i--)
	{
		if (c[i] >= '0' && c[i] <= '9')
		{
			*receive += pow * (int) (c[i] - '0');
			pow *= 10;
		}
	}
}

/** キーボードから浮動小数点数(double型)入力を受け付ける
 * @param receive : 受信した浮動小数点数を格納する変数
 * @retval void
 */
static void SCI_ScanDouble(double* receive)
{
	int i, j, k;
	double pow;
	_UBYTE c[SCAN_MAX];

	// 数値を格納する変数の初期化
	*receive = 0;

	// まず文字列の入力を受け付ける
	SCI_ScanStr(c);

	// 文字列になっている数字の桁数(※小数点を一文字に含む）を調べる
	for (i = 0; c[i] != '\0'; i++)
		;

	// 文字列になっている数字の整数部の桁数を調べる
	for (j = 0; c[j] != '.' && j < i; j++)
		;

	// 小数部の文字列を数値に変換
	pow = 1.0;
	for (k = 0; k < i - j - 1; k++)
	{
		pow *= 10.0;
	}
	for (i = i - 1; i > j; i--)
	{
		if (c[i] >= '0' && c[i] <= '9')
		{
			*receive += (double) (c[i] - '0') / pow;
			pow /= 10.0;
		}
	}

	// 整数部の文字列を数値に変換
	pow = 1.0;
	for (j = j - 1; j >= 0; j--)
	{
		if (c[j] >= '0' && c[j] <= '9')
		{
			*receive += (double) (c[j] - '0') * pow;
			pow *= 10.0;
		}
	}
}

/** キーボードから浮動小数点数(float型)入力を受け付ける
 * @param receive : 受信した浮動小数点数を格納する変数
 * @retval void
 */
static void SCI_ScanFloat(float* receive)
{
	int i, j, k;
	float pow;
	_UBYTE c[SCAN_MAX];

	// 数値を格納する変数の初期化
	*receive = 0;

	// まず文字列の入力を受け付ける
	SCI_ScanStr(c);

	// 文字列になっている数字の桁数(※小数点を一文字に含む）を調べる
	for (i = 0; c[i] != '\0'; i++)
		;

	// 文字列になっている数字の整数部の桁数を調べる
	for (j = 0; c[j] != '.' && j < i; j++)
		;

	// 小数部の文字列を数値に変換
	pow = 1.0;
	for (k = 0; k < i - j - 2; k++)
	{
		pow *= 10.0;
	}
	for (i = i - 1; i > j; i--)
	{
		if (c[i] >= '0' && c[i] <= '9')
		{
			*receive += (float) (c[i] - '0') / pow;
			pow /= 10.0;
		}
	}

	// 整数部の文字列を数値に変換
	pow = 1.0;
	for (j = j - 1; j >= 0; j--)
	{
		if (c[j] >= '0' && c[j] <= '9')
		{
			*receive += (float) (c[j] - '0') * pow;
			pow *= 10.0;
		}
	}
}

/** 一文字受信バッファから読み出し
 * @param buf : 読みだした文字を格納する変数のポインタ
 * @retval bool : 成功すればtrueを返す
 */
static bool SCI_GetChar(_UBYTE* buf)
{
	if (SCI_RxD->remain == 0)
	{						// バッファデータが無ければ
		return false;
	}
	else
	{
		*buf = SCI_RxD->Pull(SCI_RxD);		// データのバッファ読み出し
		return true;
	}
}

/** 受信バッファをフラッシュ
 * @param void
 * @retval void
 */
static void SCI_BufferFlash(void)
{
	SCI1.SCR.BIT.RE = 0;		// 受信を禁止
	SCI_RxD->Flash(SCI_RxD);
	SCI1.SCR.BIT.RE = 1;		// 受信を許可
}

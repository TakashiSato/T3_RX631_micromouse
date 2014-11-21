/**
 * @file  AD-128160-UART.c
 * @brief TFT液晶 AD-128160-UART駆動
 */

/*============================================================
		ヘッダファイルのインクルード
============================================================*/
#include "AD-128160-UART.h"
#include "../iodefine.h"
#include "../Global.h"

/*============================================================
		関数の定義
============================================================*/
/*------------------------------------------------------------
	パプリックメソッド
------------------------------------------------------------*/
/** シリアルコミュニケーションインタフェースの初期化
 * @param baudrate : ボーレート[bps]
 * @retval void
 */
void AD128160_InitializeSerialPort(long baudrate)
{
	MSTP(SCI1) = 0;					// SCI1モジュールストップ状態の解除
	SCI1.SCR.BYTE &= 0x0B;			// TEIE,RE,TE,RIE,TIEを禁止

	// ピン機能設定
	PORT2.PMR.BIT.B6 = 0;			// P26を汎用入出力ポートに設定
//	PORT3.PMR.BIT.B0 = 0;			// P30を汎用入出力ポートに設定
	MPC.PWPR.BIT.B0WI = 0;			// PFSWEレジスタへの書き込みを許可
	MPC.PWPR.BIT.PFSWE = 1;			// PFSレジスタへの書き込みを許可
	MPC.P26PFS.BIT.PSEL = 10;		// P26の機能選択:TXD1
//	MPC.P30PFS.BIT.PSEL = 10;		// P30の機能選択:RXD1
	MPC.PWPR.BYTE = 0x80;			// PFSレジスタ,PFSWEビットへの書き込みを禁止
	PORT2.PMR.BIT.B6 = 1;			// P26を周辺機能として使用
//	PORT3.PMR.BIT.B0 = 1;			// P30を周辺機能として使用

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
//	SCI1.SCR.BIT.RE = 1;			// シリアル受信動作を許可
//	SCI1.SCR.BIT.RIE = 1;			// RXI割り込み要求を許可

	// 割り込みレベル設定
//	IPR(SCI1, RXI1)= 5;
	IPR(SCI1, TXI1)= 5;
	IPR(SCI1, TEI1)= 5;

	// 割り込み要求を許可
//	IEN(SCI1, RXI1) = 1;
	IEN(SCI1, TXI1) = 1;
	IEN(SCI1, TEI1) = 1;

	// リングバッファ初期化
	AD128160_TxD = NewRingBuffer(AD128160_SEND_RING_BUFFER_SIZE);
}


/** SCI1送信終了割り込み
 * @param void
 * @retval void
 */
void AD128160_IntTEI1(void)
{
	SCI1.SCR.BIT.TEIE = 0;			// 送信終了割り込み禁止
	SCI1.SCR.BIT.TE = 0;			// シリアル送信動作を停止
}

/*------------------------------------------------------------
		送信データエンプティによる割り込み
------------------------------------------------------------*/
void AD128160_IntTXI(void)
{
	_UBYTE data;

	// データ残量が0より大きければ(送信するデータが残っていたら)
	if( AD128160_TxD->remain > 0 )
	{
		data = AD128160_TxD->Pull(AD128160_TxD);	// バッファからデータを読出し
		SCI1.TDR = data;						// 読み出したデータを送信レジスタへ。
	}
	else
	{
		SCI1.SCR.BIT.TIE = 0;	// データ残がなければ送信データエンプティ割り込み禁止
		SCI1.SCR.BIT.TEIE = 1;	// 送信終了割り込み許可
	}
}

/*------------------------------------------------------------
		1byte送信バッファへ格納
------------------------------------------------------------*/
void AD128160_SendByte(_UBYTE c)
{
	// 送信バッファにデータを追加
	AD128160_TxD->Add(AD128160_TxD, c);

	// 送信動作停止中であれば送信開始
	if(SCI1.SCR.BIT.TE == 0)
	{
		SCI1.SCR.BIT.TIE = 1;			// 送信割り込み許可
		SCI1.SCR.BIT.TE = 1;			// シリアル送信動作を開始
	}
}

/*------------------------------------------------------------
		コマンド（複数バイト）を送信バッファへ格納
------------------------------------------------------------*/
void AD128160_SendCommand(_UBYTE* sendDataArray, _UBYTE sendSize)
{
	int i;

	for (i = 0; i < sendSize; i++)
	{
		AD128160_SendByte(sendDataArray[i]);
	}
}

// ==== AD128160初期化 ====
void AD128160_Initialize()
{
//	AD128160_SetBaudrate(9600);
	AD128160_SetBaudrate(115200);

	// バッファ内の全てのデータを送信するまで待つ
	while(AD128160_TxD->remain != 0)
	{
	}
	// 念のためさらに待つ
	WaitMS(10);

	// USART3を115200bpsに設定
	IEN(SCI1, TEI1) = 0;			// TEI1割り込みを禁止
	IEN(SCI1, TXI1) = 0;			// TXI1割り込みを禁止
	SCI1.BRR = 12;					// Baudrate: 115200[bps](誤差0.16[%])
	IEN(SCI1, TXI1) = 1;			// TXI1割り込みを許可
	IEN(SCI1, TEI1) = 1;			// TEI1割り込みを許可

	AD128160_SetColor(AD128160_RGB565(0, 0, 255));
	AD128160_Clear();
	AD128160_Brightness(50);
	AD128160_Locate(0, 0);
}

/*------------------------------------------------------------
		画面表示クリア
------------------------------------------------------------*/
void AD128160_Clear()
{
	_UBYTE commandArray[] = {0x55, 0x02, 0x80, 0x55, 0xD5, 0xAA};
	AD128160_SendCommand(commandArray, 6);
	_row = 0;
	_col = 0;
}

/*------------------------------------------------------------
		バックライト明るさ設定
------------------------------------------------------------*/
void AD128160_Brightness(_UWORD brightness)
{
	_UBYTE c[2];

	// 明度上限判定
	if(brightness > 500)
	{
		brightness = 500;
	}
	c[0] = (brightness >> 8) & 0xff;
	c[1] = brightness & 0xff;
	_UBYTE sum = c[0] + c[1] + 0x89;

	_UBYTE commandArray[] = {0x55, 0x03, 0x89, c[0], c[1], sum, 0xAA};
	AD128160_SendCommand(commandArray, 7);
}

/*------------------------------------------------------------
		2バイトRGBをつくるための関数
------------------------------------------------------------*/
_UBYTE AD128160_RGB565(_UBYTE r, _UBYTE g, _UBYTE b)
{
	_UBYTE rgb;

	rgb = (r & 0xF8) << 8;        /* RRRRR----------- */
	rgb |= (g & 0xFC) << 3;       /* -----GGGGGG----- */
	rgb |= b >> 3;                /* -----------BBBBB */

	return rgb;
}

/*------------------------------------------------------------
		カラー設定
------------------------------------------------------------*/
void AD128160_SetColor(_UBYTE rgb)
{
	_UBYTE c[2];
	c[0] = (rgb >> 8) & 0xff;
	c[1] = rgb & 0xff;
	_UBYTE sum = c[0] + c[1] + 0x84;

	_UBYTE commandArray[] = {0x55, 0x03, 0x84, c[0], c[1], sum, 0xAA};
	AD128160_SendCommand(commandArray, 7);
}

/*------------------------------------------------------------
		文字背景色設定
------------------------------------------------------------*/
void AD128160_SetBackGroundColor(int rgb)
{
	_UBYTE c[2];
	c[0] = (rgb >> 8) & 0xff;
	c[1] = rgb & 0xff;
	_UBYTE sum = 0x01 + c[0] + c[1] + 0x85;

	_UBYTE commandArray[] = {0x55, 0x04, 0x85, 0x01, c[0], c[1], sum, 0xAA};
	AD128160_SendCommand(commandArray, 8);
}

/*------------------------------------------------------------
		文字背景色を透明に設定
------------------------------------------------------------*/
void AD128160_ClearBackGroundColor()
{
	_UBYTE commandArray[] = {0x55, 0x04, 0x85, 0x00, 0x00, 0x00, 0x85, 0xAA};
	AD128160_SendCommand(commandArray, 8);
}

/*------------------------------------------------------------
		転送済みのBMPを表示する
------------------------------------------------------------*/
void AD128160_DisplayBMP(_UBYTE x0, _UBYTE y0, _UBYTE bmpNo)
{
	_UBYTE locateX[2], locateY[2], numOfBMP[2];
	locateX[0] = (x0 >> 8) & 0xff;
	locateX[1] = x0 & 0xff;
	locateY[0] = (y0 >> 8) & 0xff;
	locateY[1] = y0 & 0xff;
	numOfBMP[0] = (bmpNo >> 8) & 0xff;
	numOfBMP[1] = bmpNo & 0xff;
	_UBYTE sum = locateX[0] + locateX[1] + locateY[0] + locateY[1]
					+ numOfBMP[0] + numOfBMP[1] + 0x09;

	_UBYTE commandArray[] =
	{
			0x55, 0x07, 0x09, locateX[0], locateX[1],
			locateY[0], locateY[1], numOfBMP[0], numOfBMP[1], sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 11);
}

/*------------------------------------------------------------
		Baudrateを設定する
------------------------------------------------------------*/
void AD128160_SetBaudrate(_UDWORD baudrate)
{
	_UBYTE c[4];
	c[0] = (baudrate >> 24) & 0xff;
	c[1] = (baudrate >> 16) & 0xff;
	c[2] = (baudrate >> 8) & 0xff;
	c[3] = baudrate & 0xff;
	_UBYTE sum = c[0] + c[1] + c[2] + c[3] + 0x8B;

	_UBYTE commandArray[] = { 0x55, 0x05, 0x8B, c[0], c[1], c[2], c[3], sum, 0xAA };
	AD128160_SendCommand(commandArray, 9);
}

/*------------------------------------------------------------
		ドット描画
------------------------------------------------------------*/
void AD128160_DrawDot(_UBYTE x0, _UBYTE y0)
{
	_UBYTE locateX[2], locateY[2];
	locateX[0] = (x0 >> 8) & 0xff;
	locateX[1] = x0 & 0xff;
	locateY[0] = (y0 >> 8) & 0xff;
	locateY[1] = y0 & 0xff;
	_UBYTE sum = locateX[0] + locateX[1] + locateY[0] + locateY[1] + 0x01;

	_UBYTE commandArray[] =
	{
			0x55, 0x05, 0x01, locateX[0], locateX[1],
			locateY[0], locateY[1], sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 9);
}

/*------------------------------------------------------------
		線描画
------------------------------------------------------------*/
void AD128160_DrawLine(_UBYTE x0, _UBYTE y0, _UBYTE x1, _UBYTE y1)
{
	_UBYTE locateX0[2], locateY0[2], locateX1[2], locateY1[2];
	locateX0[0] = (x0 >> 8) & 0xff;
	locateX0[1] = x0 & 0xff;
	locateY0[0] = (y0 >> 8) & 0xff;
	locateY0[1] = y0 & 0xff;
	locateX1[0] = (x1 >> 8) & 0xff;
	locateX1[1] = x1 & 0xff;
	locateY1[0] = (y1 >> 8) & 0xff;
	locateY1[1] = y1 & 0xff;
	_UBYTE sum = locateX0[0] + locateX0[1] + locateY0[0] + locateY0[1]
					+ locateX1[0] + locateX1[1] + locateY1[0] + locateY1[1] + 0x02;

	_UBYTE commandArray[] =
	{
			0x55, 0x09, 0x02, locateX0[0], locateX0[1], locateY0[0], locateY0[1],
			locateX1[0], locateX1[1], locateY1[0], locateY1[1], sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 13);
}

/*------------------------------------------------------------
		四角描画
------------------------------------------------------------*/
void AD128160_DrawBox(_UBYTE x0, _UBYTE y0, _UBYTE x1, _UBYTE y1, _UBYTE paint)
{
	_UBYTE cmd;
	if (paint == TRUE)
	{
		cmd = 0x04;
	}
	else
	{
		cmd = 0x03;
	}
	_UBYTE locateX0[2], locateY0[2], locateX1[2], locateY1[2];
	locateX0[0] = (x0 >> 8) & 0xff;
	locateX0[1] = x0 & 0xff;
	locateY0[0] = (y0 >> 8) & 0xff;
	locateY0[1] = y0 & 0xff;
	locateX1[0] = (x1 >> 8) & 0xff;
	locateX1[1] = x1 & 0xff;
	locateY1[0] = (y1 >> 8) & 0xff;
	locateY1[1] = y1 & 0xff;
	_UBYTE sum = locateX0[0] + locateX0[1] + locateY0[0] + locateY0[1]
					+ locateX1[0] + locateX1[1] + locateY1[0] + locateY1[1] + cmd;

	_UBYTE commandArray[] =
	{
			0x55, 0x09, cmd, locateX0[0], locateX0[1], locateY0[0], locateY0[1],
			locateX1[0], locateX1[1], locateY1[0], locateY1[1], sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 13);
}

/*------------------------------------------------------------
		円描画
------------------------------------------------------------*/
void AD128160_DrawCircle(_UBYTE x0, _UBYTE y0, _UBYTE radius, _UBYTE paint)
{
	_UBYTE cmd;
	if (paint == TRUE)
	{
		cmd = 0x06;
	}
	else
	{
		cmd = 0x05;
	}
	_UBYTE locateX[2], locateY[2], rad[2];
	locateX[0] = (x0 >> 8) & 0xff;
	locateX[1] = x0 & 0xff;
	locateY[0] = (y0 >> 8) & 0xff;
	locateY[1] = y0 & 0xff;
	rad[0] = (radius >> 8) & 0xff;
	rad[1] = radius & 0xff;
	_UBYTE sum = locateX[0] + locateX[1] + locateY[0] + locateY[1]
					+ rad[0] + rad[1] + cmd;

	_UBYTE commandArray[] =
	{
			0x55, 0x07, cmd, locateX[0], locateX[1], locateY[0], locateY[1],
			rad[0], rad[1], sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 11);
}

/*------------------------------------------------------------
		文字表示位置を設定
------------------------------------------------------------*/
void AD128160_Locate(_UBYTE row, _UBYTE col)
{
	_row = row;
	_col = col;
}

/*------------------------------------------------------------
		改行する
------------------------------------------------------------*/
void AD128160_NewLine()
{
	_col = 0;
	_row++;
	if (_row >= 10)
	{
		_row = 0;
	}
}

/*------------------------------------------------------------
		一文字表示
------------------------------------------------------------*/
void AD128160_PutChar(char c)
{
	if (c == '\n')
	{
		AD128160_NewLine();
	}
	else
	{
		_UBYTE x0 = _col * 8;
		_UBYTE y0 = _row * 16;
		_UBYTE locateX[2], locateY[2];
		locateX[0] = (x0 >> 8) & 0xff;
		locateX[1] = x0 & 0xff;
		locateY[0] = (y0 >> 8) & 0xff;
		locateY[1] = y0 & 0xff;
		_UBYTE sum = locateX[0] + locateX[1] + locateY[0] + locateY[1] + c + 0x0B;

		_UBYTE commandArray[] =
		{
			0x55, 0x06, 0x0B, locateX[0], locateX[1],
			locateY[0], locateY[1], (_UBYTE)c, sum, 0xAA
		};
		AD128160_SendCommand(commandArray, 10);

		_col++;
		if (_col >= 16)
		{
			_row++;
			if (_row >= 10)
			{
				_row = 0;
			}
			_col = 0;
		}
	}
}

/*------------------------------------------------------------
		文字列表示
------------------------------------------------------------*/
void AD128160_PutStr(char* str)
{
	int i;
	_UBYTE strLength = 0;
	for(i = 0; str[i] != '\0'; i++)
	{
		strLength++;
	}

	_UBYTE x0 = _col * 8;
	_UBYTE y0 = _row * 16;
	_UBYTE dataLen = 5 + strLength;

	_UBYTE locateX[2], locateY[2];
	locateX[0] = (x0 >> 8) & 0xff;
	locateX[1] = x0 & 0xff;
	locateY[0] = (y0 >> 8) & 0xff;
	locateY[1] = y0 & 0xff;
	_UBYTE sum = 0x0B + locateX[0] + locateX[1] + locateY[0] + locateY[1];

	_UBYTE commandArray[128];
	commandArray[0] = 0x55;
	commandArray[1] = dataLen;
	commandArray[2] = 0x0B;
	commandArray[3] = locateX[0];
	commandArray[4] = locateX[1];
	commandArray[5] = locateY[0];
	commandArray[6] = locateY[1];
	for (i = 0; i < strLength; i++)
	{
		commandArray[7 + i] = (_UBYTE)str[i];
		sum += (_UBYTE)str[i];
	}
	commandArray[7 + i] = sum;
	commandArray[8 + i] = 0xAA;
	AD128160_SendCommand(commandArray, 9 + i);

	_col += strLength;
	while (_col >= 16)
	{
		_row++;
		if (_row >= 10)
		{
			_row = 0;
		}
		_col -= 16;
	}
	if (_col < 0)
	{
		_col = 0;
	}
}

/*------------------------------------------------------------
		整数表示(32bit型まで)
------------------------------------------------------------*/
void AD128160_PutInt(char fmt, _SDWORD var, _SBYTE len)
{
	char s[AD128160_C_MAX], buf[AD128160_C_MAX], *ptr, z;
	char num[] = "0123456789ABCDEF";
	_SDWORD b;
	_SDWORD count = 0;

	// 正負判定
	if( var < 0 ){
		var = -var;
		len--;
		s[count++] = '-';
	}

	// 数値フォーマット判定
	switch( fmt ){
		// 10進数
		case 'd':
			b = 10;
			z = ' ';
			break;
		// 16進数
		case 'x':
			b = 16;
			z = '0';
			s[count++] = '0';
			s[count++] = 'x';
			break;
		default:
			return;
	}

	ptr = buf;

	// フォーマットに応じて数値を文字に変換
	do{
		len--;
		*ptr++ = num[var % b];
		var /= b;
	} while( var != 0 );

	// 桁数合わせ
	for( ; len > 0; len-- )
	{
		s[count++] = z;
	}

	while( ptr > buf)
	{
		s[count++] = *--ptr;
	}
	s[count] = '\0';

	AD128160_PutStr(s);
}

/*------------------------------------------------------------
		整数表示(64bit型まで)
------------------------------------------------------------*/
void AD128160_PutLong(_SQWORD var, _SBYTE len)
{
	char s[AD128160_C_MAX], buf[AD128160_C_MAX], *ptr;
	char num[] = "0123456789";
	_SDWORD count = 0;

	// 正負判定
	if( var < 0 ){
		var = -var;
		len--;
		s[count++] = '-';
	}

	ptr = buf;

	// フォーマットに応じて数値を文字に変換
	do{
		len--;
		*ptr++ = num[var % 10];
		var /= 10;
	} while( var != 0 );

	// 桁数合わせ
	for( ; len > 0; len-- )
	{
		s[count++] = ' ';
	}

	while( ptr > buf)
	{
		s[count++] = *--ptr;
	}
	s[count] = '\0';

	AD128160_PutStr(s);
}

/*------------------------------------------------------------
		浮動小数点数表示
------------------------------------------------------------*/
void AD128160_PutDouble(double var, _SBYTE len, _SBYTE fracLen)
{
	int i;
	char s[AD128160_C_MAX], buf[AD128160_C_MAX], *ptr;
	char num[] = "0123456789";
	double index = 1.0;
	_SQWORD tempVar;
	_SDWORD count = 0;

	// 正負判定
	if( var < 0 ){
		var = -var;
		len--;
		s[count++] = '-';
	}

	ptr = buf;

	// ---- 数値を文字に変換 ----
	// 小数部の処理
	for( i = 0; i < fracLen; i++ ) index *= 10.0;
	tempVar = (_SQWORD)( (var - (_SQWORD)var) * index );
	do{
		len--;
		i--;
		*ptr++ = num[tempVar % 10];
		tempVar /= 10;
	} while( tempVar != 0 || i > 0);

	// 小数点の挿入
	*ptr++ = '.';

	// 整数部の処理
	tempVar = (_SQWORD)var;
	do{
		len--;
		*ptr++ = num[tempVar % 10];
		tempVar /= 10;
	} while( tempVar != 0 );

	// 桁数合わせ
	for( ; len > 0; len-- )
	{
		s[count++] = ' ';
	}

	while( ptr > buf)
	{
		s[count++] = *--ptr;
	}
	s[count] = '\0';

	AD128160_PutStr(s);
}

/*------------------------------------------------------------
		AD128160用Printf関数
------------------------------------------------------------*/
void AD128160_Printf(_UBYTE* str, ...)
{
	va_list args;
	_UBYTE *ptr;
	_SBYTE len = 0;
	_UBYTE fracLen = AD128160_FRAC_LENGTH;	// 小数点表示時の小数点以下桁数（デフォルトでは2）

	va_start(args, str);						// 可変引数の初期化

	// 一文字ずつフォーマット判別
	for (ptr = str; *ptr != '\0'; ptr++)
	{
		// 変数指定子のとき
		if( *ptr == '%' ){
			ptr++;

			// 表示文字数指定の判別
			if( *ptr == '.' ){
				ptr++;
				if( (*ptr >= '1') && (*ptr <='9') ){
					fracLen = *ptr++ - '0';					// 数字の文字を数値に変換
					while( (*ptr >= '0') && (*ptr <= '9') )
						fracLen = fracLen * 10 + (*ptr++ - '0');
				}
			}
			else if( (*ptr >= '1') && (*ptr <='9') ){
				len = *ptr++ - '0';					// 数字の文字を数値に変換
				if( *ptr == '.' ){
					ptr++;
					if( (*ptr >= '1') && (*ptr <='9') ){
						fracLen = *ptr++ - '0';					// 数字の文字を数値に変換
						while( (*ptr >= '0') && (*ptr <= '9') )
							fracLen = fracLen * 10 + (*ptr++ - '0');
					}
				}
				else{
					while( (*ptr >= '0') && (*ptr <= '9') )
						len = len * 10 + (*ptr++ - '0');
				}
			}
			else
				len = -1;

			switch( *ptr ){
				case 's':
					AD128160_PutStr( va_arg(args, char*) );
					break;
				case 'c':
					AD128160_PutChar( ( va_arg(args, int) & 0xFF ) );
					break;
				case 'd': case 'x':
					AD128160_PutInt( *ptr, va_arg(args, int), len );
					break;
				case 'l':
					AD128160_PutLong( va_arg(args, long long), len );
					break;
				case 'f':
					AD128160_PutDouble( va_arg(args, double), len, fracLen );
					break;
			}
		}
		else if( *ptr == '\n' ){
			AD128160_NewLine();
		}
		else
			AD128160_PutChar(*ptr);
	}
	va_end(args);					// 可変引数の終了処理
}

/*------------------------------------------------------------
		整数表示特化関数
		type 数字表示形式
		   0: 右寄せ（桁数が指定値以下の場合0詰め)
		   1: 左寄せ（桁数が指定値以下の場合0詰めなし)
		   2: 右寄せ（桁数が指定値以下の場合0詰めなし)
------------------------------------------------------------*/
void AD128160_DisplasyNum(_UBYTE row, _UBYTE col, signed int num, unsigned char len, unsigned char type)
{
	_UBYTE x = col * 8;
	_UBYTE y = row * 16;
	_UBYTE locateX[2], locateY[2];
	_UBYTE c[4];


	locateX[0] = (x >> 8) & 0xff;
	locateX[1] = x & 0xff;
	locateY[0] = (y >> 8) & 0xff;
	locateY[1] = y & 0xff;
	c[0] = (num >> 24) & 0xff;
	c[1] = (num >> 16) & 0xff;
	c[2] = (num >> 8) & 0xff;
	c[3] = num & 0xff;
	_UBYTE sum = locateX[0] + locateX[1] + locateY[0] + locateY[1]
	               + c[0] + c[1] + c[2] + c[3] + len + type + 0x0A;

	_UBYTE commandArray[] =
	{
			0x55, 0x0B, 0x0A, locateX[0], locateX[1],
			locateY[0], locateY[1], c[0], c[1], c[2], c[3],
			len, type, sum, 0xAA
	};
	AD128160_SendCommand(commandArray, 16);
}

/*------------------------------------------------------------
		迷路上x座標取得
------------------------------------------------------------*/
_UBYTE AD128160_GetX()
{
	return AD128160_MousePos.x;
}

/*------------------------------------------------------------
		迷路上y座標取得
------------------------------------------------------------*/
_UBYTE AD128160_GetY()
{
	return AD128160_MousePos.y;
}

/*------------------------------------------------------------
		迷路上(x, y)座標セット
------------------------------------------------------------*/
void AD128160_SetCoordinate(_SBYTE x, _SBYTE y)
{
	if(0 <= x && x <= 15 )
	{
		AD128160_MousePos.x = x;
	}
	if(0 <= y && y <= 15)
	{
		AD128160_MousePos.y = y;
	}
}

/*------------------------------------------------------------
		迷路表示初期化
------------------------------------------------------------*/
void AD128160_InitLabyrinth()
{
	int i, j;

	AD128160_SetColor(AD128160_RGB565(0, 0, 0));
	AD128160_Clear();
	AD128160_DrawBox(0, 0, 128, 128, TRUE);
	AD128160_SetColor(AD128160_RGB565(255, 0, 0));
	AD128160_DrawBox(0, 0, 127, 128, FALSE);

	for (i = 0; i < 16; i++)
	{
	  for (j = 0; j < 16; j++)
	  {
			AD128160_DrawDot(i * 8, j * 8);
	  }

	}
	AD128160_CreateWall(0, 0, EAST);
	AD128160_Locate(9, 3);
	AD128160_SetBackGroundColor(AD128160_RGB565(0, 0, 0));
	AD128160_PutStr("Micro Mouse");

	AD128160_DisplayBMP(2, 121, 0);
	AD128160_SetCoordinate(0, 0);
}

/*------------------------------------------------------------
		迷路壁描画
------------------------------------------------------------*/
void AD128160_CreateWall(_UBYTE x, _UBYTE y, DIRECTION dir)
{
	switch(dir)
	{
		case NORTH:
			AD128160_DrawLine(x * 8, 128 - y * 8 - 8, x * 8 + 8, 128 - y * 8 - 8);
			break;
		case EAST:
			AD128160_DrawLine(x * 8 + 8, 128 - y * 8, x * 8 + 8, 128 - y * 8 - 8);
			break;
		case WEST:
			AD128160_DrawLine(x * 8, 128 - y * 8, x * 8, 128 - y * 8 - 8);
			break;
		case SOUTH:
			AD128160_DrawLine(x * 8, 128 - y * 8, x * 8 + 8, 128 - y * 8);
			break;
		default:
			break;
	}
}

/*------------------------------------------------------------
		指定方角へ一歩進める
------------------------------------------------------------*/
void AD128160_Advance(DIRECTION dir)
{
	AD128160_SetColor(AD128160_RGB565(0, 0, 0));
	AD128160_DrawBox(AD128160_GetX() * 8 + 2, 121 - AD128160_GetY() * 8, AD128160_GetX() * 8 + 7, 126 - AD128160_GetY() * 8, TRUE);
	AD128160_SetColor(AD128160_RGB565(255, 0, 0));
	AD128160_DrawBox(0, 0, 127, 128, FALSE);

	switch(dir)
	{
		case NORTH:
			AD128160_SetCoordinate(AD128160_GetX(), AD128160_GetY() + 1);
			AD128160_DisplayBMP(AD128160_GetX() * 8 + 2, 121 - AD128160_GetY() * 8, 0);
			break;
		case EAST:
			AD128160_SetCoordinate(AD128160_GetX() + 1, AD128160_GetY());
			AD128160_DisplayBMP(AD128160_GetX() * 8 + 2, 121 - AD128160_GetY() * 8, 1);
			break;
		case WEST:
			AD128160_SetCoordinate(AD128160_GetX() - 1, AD128160_GetY());
			AD128160_DisplayBMP(AD128160_GetX() * 8 + 2, 121 - AD128160_GetY() * 8, 3);
			break;
		case SOUTH:
			AD128160_SetCoordinate(AD128160_GetX(), AD128160_GetY() - 1);
			AD128160_DisplayBMP(AD128160_GetX() * 8 + 2, 121 - AD128160_GetY() * 8, 2);
			break;
		default:
			break;
	}
}

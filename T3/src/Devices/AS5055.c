/**
 * @file  AS5055.c
 * @brief 磁気エンコーダAS5055の動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "AS5055.h"
#include "../iodefine.h"
#include "../Peripherals/RSPI.h"

/*----------------------------------------------------------------------
	Private global variables
 ----------------------------------------------------------------------*/
static RSPI_DEPENDENCE dep;		// RSPIペリフェラル依存パラメータ
static _UWORD send[2] = {0};	// SPI送信データ格納用
static _UWORD recv[2] = {0};	// SPI送信データ格納用
static _UWORD angle[1];			// 磁気エンコーダ角度情報格納用

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void AS5055_InitializeRSPI0(void);
static void AS5055_InitializePort(void);
static void AS5055_Select(void);
static void AS5055_Deselect(void);
static _UWORD AppendEvenParity(_UWORD command);
static void AS5055_RSPI_Write(_UWORD registerAddress, _UWORD data);
static _UWORD AS5055_RSPI_Read(_UWORD registerAddress);
static void AS5055_CycleSendCommand(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/** AS5055の初期化
 * @param void
 * @retval void
 */
void AS5055_Initialize(void)
{
	// ピン機能設定
	AS5055_InitializePort();
	// RSPI0初期化
	AS5055_InitializeRSPI0();
}

/** 磁気エンコーダの角度情報を取得する
 * @param void
 * @retval _UWORD: 角度情報
 */
_UWORD AS5055_GetAngle(void)
{
	_UWORD ang = (_UWORD)((angle[0] >> 2) & 0x0FFF);
	return ang;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
static void AS5055_InitializeRSPI0(void)
{
	// コマンドレジスタの設定
//	RSPI0.SPCMD0.BIT.BRDV = 3;	// ベースのビットレートの8分周:1Mbps(=T:10nS)
//	RSPI0.SPCMD0.BIT.CPHA = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
//	RSPI0.SPCMD0.BIT.CPOL = 0;	// アイドル時のRSPCK:Low
//	RSPI0.SPCMD0.BIT.SSLKP = 0;	// 転送終了時にSSL信号をネゲート
//	RSPI0.SPCMD0.BIT.SPB = 15;	// データ長:16ビット
//	RSPI0.SPCMD0.BIT.LSBF = 0;	// MSBファースト
//	RSPI0.SPCMD0.BIT.SPNDEN = 1;// 次アクセス遅延はSPNDレジスタの設定値
//	RSPI0.SPCMD0.BIT.SLNDEN = 1;// SSLネゲート遅延はSSLNDレジスタの設定値
//	RSPI0.SPCMD0.BIT.SCKDEN = 1;// RSPCK遅延はSPCKDレジスタの設定値
	dep.brdv_val = 3;	// ベースのビットレートの8分周:1Mbps(=T:10nS)
	dep.cpha_val = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
	dep.cpol_val = 0;	// アイドル時のRSPCK:Low
	dep.sslkp_val = 0;	// 転送終了時にSSL信号をネゲート
	dep.spb_val = 15;	// データ長:16ビット
	dep.lsbf_val = 0;	// MSBファースト
	dep.spnden_val = 1;	// 次アクセス遅延はSPNDレジスタの設定値
	dep.slnden_val = 1;	// SSLネゲート遅延はSSLNDレジスタの設定値
	dep.sckden_val = 1;	// RSPCK遅延はSPCKDレジスタの設定値

	dep.format = RSPI_WORD_DATA;
	dep.ChipSelectFunc = AS5055_Select;
	dep.ChipDeselectFunc = AS5055_Deselect;

	RSPI0_RegisterDeviceForCycleOperation(AS5055_CycleSendCommand);
}

/** AS5055で使うポートの初期化
 * @param void
 * @retval void
 */
static void AS5055_InitializePort(void)
{
	PORTB.PMR.BIT.B0 = 0;			// PB0を汎用入出力ポートに設定
	PORTB.PDR.BIT.B0 = 1;			// PB0:出力
	PORTB.PODR.BIT.B0 = 1;			// PB0:High
}

/** AS5055チップセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_Select(void)
{
	PORTB.PODR.BIT.B0 = 0;
}

/** AS5055チップデセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_Deselect(void)
{
	PORTB.PODR.BIT.B0 = 1;
}

/** 偶数パリティを付加したコマンドを返す
 * @param command: 送信するコマンド
 * @retval _UWORD: パリティが付加されたコマンド
 */
static _UWORD AppendEvenParity(_UWORD command)
{
	_UWORD parity;

	// 偶数パリティの計算
	parity = command ^ (command >> 8);
	parity = parity ^ (parity >> 4);
	parity = parity ^ (parity >> 2);
	parity = parity ^ (parity >> 1);

	// 末尾に偶数パリティを付加してコマンドを返す
	return command | (parity & 0x01);
}

/** AS5055書き込み関数
 * @param registerAddress: 書き込むレジスタアドレス
 * @param data: 書き込むデータ
 * @retval void
 */
static void AS5055_RSPI_Write(_UWORD registerAddress, _UWORD data)
{
	// 末尾に偶数パリティを付加
	send[0] = AppendEvenParity(registerAddress << 1);
	send[1] = AppendEvenParity(data << 2);

	RSPI0_WriteRead(send, recv, 2, dep);
}

/** AS5055読み出し関数
 * @param registerAddress: 読み出すレジスタアドレス
 * @retval _UWORD: 返答
 */
static _UWORD AS5055_RSPI_Read(_UWORD registerAddress)
{
	_UWORD command;

	// 送信用に1ビット左にシフトし，先頭にReadコマンドを付加
	command = (registerAddress << 1) | AS_READ_FLAG;

	// 末尾に偶数パリティを付加
	send[0] = AppendEvenParity(command);

	RSPI0_WriteRead(send, recv, 1, dep);

	return recv[1];
}

/** RSPI0 Cycle Operation で送信するコマンド(角度情報取得)
 * @param void
 * @retval void
 */
static void AS5055_CycleSendCommand(void)
{
	_UWORD command;

	// 角度情報を取得
	// 送信用に1ビット左にシフトし，先頭にReadコマンドを付加
	command = (ASREG_ANGULAR_DATA << 1) | AS_READ_FLAG;

	// 末尾に偶数パリティを付加
	send[0] = AppendEvenParity(command);

	RSPI0_WriteRead(send, angle, 1, dep);
}

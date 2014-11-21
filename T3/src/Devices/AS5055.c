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
static RSPI_DEPENDENCE EncL_dep;	// RSPIペリフェラル依存パラメータ
static RSPI_DEPENDENCE EncR_dep;	// RSPIペリフェラル依存パラメータ
static _UWORD EncL_send[2] = {0};	// SPI送信データ格納用
static _UWORD EncR_send[2] = {0};	// SPI送信データ格納用
//static _UWORD recv[2] = {0};		// SPI送信データ格納用
static _UWORD EncL_angle[1];		// 磁気エンコーダ角度情報格納用
static _UWORD EncR_angle[1];		// 磁気エンコーダ角度情報格納用

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void AS5055_InitializeRSPI0(void);
static void AS5055_InitializePort(void);
static void AS5055_LeftEnc_Select(void);
static void AS5055_LeftEnc_Deselect(void);
static void AS5055_RightEnc_Select(void);
static void AS5055_RightEnc_Deselect(void);
static _UWORD AppendEvenParity(_UWORD command);
//static void AS5055_RSPI_Write(_UWORD registerAddress, _UWORD data);
//static _UWORD AS5055_RSPI_Read(_UWORD registerAddress);
static void AS5055_LeftEnc_CycleSendCommand(void);
static void AS5055_RightEnc_CycleSendCommand(void);

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
 * @param encLR: エンコーダ左or右
 * @retval _UWORD: 角度情報
 */
_UWORD AS5055_GetAngle(E_AS5055_LR encLR)
{
	_UWORD ang = 0xFFFF;

	switch(encLR)
	{
	case ENC_L:
		ang = (_UWORD)((EncL_angle[0] >> 2) & 0x0FFF);
		break;
	case ENC_R:
		ang = (_UWORD)((EncR_angle[0] >> 2) & 0x0FFF);
		break;
	}

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

	// 左エンコーダ用
	EncL_dep.brdv_val = 3;	// ベースのビットレートの8分周:1Mbps(=T:10nS)
	EncL_dep.cpha_val = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
	EncL_dep.cpol_val = 0;	// アイドル時のRSPCK:Low
	EncL_dep.sslkp_val = 0;	// 転送終了時にSSL信号をネゲート
	EncL_dep.spb_val = 15;	// データ長:16ビット
	EncL_dep.lsbf_val = 0;	// MSBファースト
	EncL_dep.spnden_val = 1;	// 次アクセス遅延はSPNDレジスタの設定値
	EncL_dep.slnden_val = 1;	// SSLネゲート遅延はSSLNDレジスタの設定値
	EncL_dep.sckden_val = 1;	// RSPCK遅延はSPCKDレジスタの設定値
	EncL_dep.format = RSPI_WORD_DATA;
	EncL_dep.ChipSelectFunc = AS5055_LeftEnc_Select;
	EncL_dep.ChipDeselectFunc = AS5055_LeftEnc_Deselect;

	RSPI0_RegisterDeviceForCycleOperation(AS5055_LeftEnc_CycleSendCommand);

	// 右エンコーダ用
	EncR_dep.brdv_val = 3;	// ベースのビットレートの8分周:1Mbps(=T:10nS)
	EncR_dep.cpha_val = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
	EncR_dep.cpol_val = 0;	// アイドル時のRSPCK:Low
	EncR_dep.sslkp_val = 0;	// 転送終了時にSSL信号をネゲート
	EncR_dep.spb_val = 15;	// データ長:16ビット
	EncR_dep.lsbf_val = 0;	// MSBファースト
	EncR_dep.spnden_val = 1;	// 次アクセス遅延はSPNDレジスタの設定値
	EncR_dep.slnden_val = 1;	// SSLネゲート遅延はSSLNDレジスタの設定値
	EncR_dep.sckden_val = 1;	// RSPCK遅延はSPCKDレジスタの設定値
	EncR_dep.format = RSPI_WORD_DATA;
	EncR_dep.ChipSelectFunc = AS5055_RightEnc_Select;
	EncR_dep.ChipDeselectFunc = AS5055_RightEnc_Deselect;

	RSPI0_RegisterDeviceForCycleOperation(AS5055_RightEnc_CycleSendCommand);
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
	PORTB.PMR.BIT.B1 = 0;			// PB1を汎用入出力ポートに設定
	PORTB.PDR.BIT.B1 = 1;			// PB1:出力
	PORTB.PODR.BIT.B1 = 1;			// PB1:High
}

/** AS5055(左)チップセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_LeftEnc_Select(void)
{
	PORTB.PODR.BIT.B1 = 0;
}

/** AS5055(左)チップデセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_LeftEnc_Deselect(void)
{
	PORTB.PODR.BIT.B1 = 1;
}

/** AS5055(右)チップセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_RightEnc_Select(void)
{
	PORTB.PODR.BIT.B0 = 0;
}

/** AS5055(右)チップデセレクト関数
 * @param void
 * @retval void
 */
static void AS5055_RightEnc_Deselect(void)
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

///** AS5055書き込み関数
// * @param registerAddress: 書き込むレジスタアドレス
// * @param data: 書き込むデータ
// * @retval void
// */
//static void AS5055_RSPI_Write(_UWORD registerAddress, _UWORD data)
//{
//	// 末尾に偶数パリティを付加
//	send[0] = AppendEvenParity(registerAddress << 1);
//	send[1] = AppendEvenParity(data << 2);
//
//	RSPI0_WriteRead(send, recv, 2, dep);
//}
//
///** AS5055読み出し関数
// * @param registerAddress: 読み出すレジスタアドレス
// * @retval _UWORD: 返答
// */
//static _UWORD AS5055_RSPI_Read(_UWORD registerAddress)
//{
//	_UWORD command;
//
//	// 送信用に1ビット左にシフトし，先頭にReadコマンドを付加
//	command = (registerAddress << 1) | AS_READ_FLAG;
//
//	// 末尾に偶数パリティを付加
//	send[0] = AppendEvenParity(command);
//
//	RSPI0_WriteRead(send, recv, 1, dep);
//
//	return recv[1];
//}

/** RSPI0 Cycle Operation で送信するコマンド(左エンコーダ角度情報取得)
 * @param void
 * @retval void
 */
static void AS5055_LeftEnc_CycleSendCommand(void)
{
	_UWORD command;

	// 角度情報を取得
	// 送信用に1ビット左にシフトし，先頭にReadコマンドを付加
	command = (ASREG_ANGULAR_DATA << 1) | AS_READ_FLAG;

	// 末尾に偶数パリティを付加
	EncL_send[0] = AppendEvenParity(command);

	RSPI0_WriteRead(EncL_send, EncL_angle, 1, EncL_dep);
}

/** RSPI0 Cycle Operation で送信するコマンド(右エンコーダ角度情報取得)
 * @param void
 * @retval void
 */
static void AS5055_RightEnc_CycleSendCommand(void)
{
	_UWORD command;

	// 角度情報を取得
	// 送信用に1ビット左にシフトし，先頭にReadコマンドを付加
	command = (ASREG_ANGULAR_DATA << 1) | AS_READ_FLAG;

	// 末尾に偶数パリティを付加
	EncR_send[0] = AppendEvenParity(command);

	RSPI0_WriteRead(EncR_send, EncR_angle, 1, EncR_dep);
}

/**
 * @file  MPU6500.c
 * @brief 6軸慣性センサMPU-6500の動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "MPU6500.h"
#include "../iodefine.h"
#include "../Global.h"
#include "../Peripherals/RSPI.h"

/*----------------------------------------------------------------------
	Private global variables
 ----------------------------------------------------------------------*/
static RSPI_DEPENDENCE dep;		// RSPIペリフェラル依存パラメータ
static _UBYTE recv[4] = {0};	// SPI送信データ格納用
static _UBYTE send[4] = {0};	// SPI送信データ格納用
static _UBYTE angvel[3];		// 角速度情報格納用

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/
static void InitializeGyro(void);
static void MPU6500_InitializeRSPI0(void);
static void MPU6500_InitializePort(void);
static void MPU6500_Select(void);
static void MPU6500_Deselect(void);
static void MPU6500_RSPI_Write(_UBYTE registerAddress, _UBYTE data);
static _UBYTE MPU6500_RSPI_Read(_UBYTE registerAddress);
static void MPU6500_CycleSendCommand(void);

/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/** MPU6500の初期化
 * @param void
 * @retval void
 */
void MPU6500_Initialize(void)
{
	MPU6500_InitializePort();
	MPU6500_InitializeRSPI0();

	InitializeGyro();
}

/** ジャイロの角速度情報(ヨージャイロ)を取得する
 * @param void
 * @retval _SWORD: ヨージャイロ
 */
_SWORD MPU6500_GetAngVel(void)
{
	_UWORD angv = (((_UWORD)angvel[1] << 8) & 0xFF00) | ((_UWORD)angvel[2] & 0x00FF);
	return (_SWORD)angv;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/
/** ジャイロの初期化
 * @param void
 * @retval void
 */
static void InitializeGyro(void)
{
	// I2C無効化
	MPU6500_RSPI_Write(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);
	WaitMS(10);
	// デバイスリセット
	MPU6500_RSPI_Write(MPUREG_PWR_MGMT_1, BIT_H_RESET);
	WaitMS(100);
	// I2C無効化
	MPU6500_RSPI_Write(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);
	WaitMS(10);
	// センサリセット
	MPU6500_RSPI_Write(MPUREG_SIGNAL_PATH_REST, BIT_GYRO_RST | BIT_ACCEL_RST | BIT_TEMP_RST);
	WaitMS(100);
	// Sample rate divider
	MPU6500_RSPI_Write(MPUREG_SMPLRT_DIV, 0x01);
	WaitMS(10);
	// Low pass filter settings
	MPU6500_RSPI_Write(MPUREG_CONFIG, BITS_DLPF_CFG_5HZ);
	WaitMS(10);
	// Set full scale range for Gyros
	MPU6500_RSPI_Write(MPUREG_GYRO_CONFIG, BITS_FS_2000DPS);
	WaitMS(10);
	// 割り込み無効
	MPU6500_RSPI_Write(MPUREG_INT_ENABLE, 0x00);
	WaitMS(10);
}

/** RSPI0の初期化
 * @param void
 * @retval void
 */
static void MPU6500_InitializeRSPI0(void)
{
//	// コマンドレジスタの設定
//	RSPI0.SPCMD0.BIT.BRDV = 3;	// ビットレート分周設定: 1Mbps
//	RSPI0.SPCMD0.BIT.CPHA = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
//	RSPI0.SPCMD0.BIT.CPOL = 1;	// アイドル時のRSPCK:High
//	RSPI0.SPCMD0.BIT.SSLKP = 0;	// 転送終了時にSSL信号をネゲート
//	RSPI0.SPCMD0.BIT.SPB = 7;	// データ長:8ビット
//	RSPI0.SPCMD0.BIT.LSBF = 0;	// MSBファースト
//	RSPI0.SPCMD0.BIT.SPNDEN = 1;// 次アクセス遅延はSPNDレジスタの設定値
//	RSPI0.SPCMD0.BIT.SLNDEN = 1;// SSLネゲート遅延はSSLNDレジスタの設定値
//	RSPI0.SPCMD0.BIT.SCKDEN = 1;// RSPCK遅延はSPCKDレジスタの設定値
	dep.brdv_val = 3;	// ベースのビットレートの8分周:1Mbps(=T:10nS)
	dep.cpha_val = 1;	// 奇数エッジでデータ変化,偶数エッジでデータサンプル
	dep.cpol_val = 1;	// アイドル時のRSPCK:High
	dep.sslkp_val = 0;	// 転送終了時にSSL信号をネゲート
	dep.spb_val = 7;	// データ長:8ビット
	dep.lsbf_val = 0;	// MSBファースト
	dep.spnden_val = 1;	// 次アクセス遅延はSPNDレジスタの設定値
	dep.slnden_val = 1;	// SSLネゲート遅延はSSLNDレジスタの設定値
	dep.sckden_val = 1;	// RSPCK遅延はSPCKDレジスタの設定値

	dep.format = RSPI_BYTE_DATA;
	dep.ChipSelectFunc = MPU6500_Select;
	dep.ChipDeselectFunc = MPU6500_Deselect;

	RSPI0_RegisterDeviceForCycleOperation(MPU6500_CycleSendCommand);
}

/** MPU6500で使うポートの初期化
 * @param void
 * @retval void
 */
static void MPU6500_InitializePort(void)
{
	PORTC.PMR.BIT.B4 = 0;			// PC4を汎用入出力ポートに設定
	PORTC.PDR.BIT.B4 = 1;			// PC4:出力
	PORTC.PODR.BIT.B4 = 1;			// PC4:High
}

/** MPU6500チップセレクト関数
 * @param void
 * @retval void
 */
static void MPU6500_Select(void)
{
	PORTC.PODR.BIT.B4 = 0;
}

/** MPU6500チップデセレクト関数
 * @param void
 * @retval void
 */
static void MPU6500_Deselect(void)
{
	PORTC.PODR.BIT.B4 = 1;
}

/** MPU6500書き込み関数
 * @param registerAddress: 書き込むレジスタアドレス
 * @param data: 書き込むデータ
 * @retval void
 */
static void MPU6500_RSPI_Write(_UBYTE registerAddress, _UBYTE data)
{
	send[0] = registerAddress;
	send[1] = data;
	RSPI0_Write(send, 2, dep);
}

/** MPU6500読み出し関数
 * @param registerAddress: 読み出すレジスタアドレス
 * @retval _UBYTE: 返答
 */
static _UBYTE MPU6500_RSPI_Read(_UBYTE registerAddress)
{
	send[0] = registerAddress | MPU_READ_FLAG;
	send[1] = 0x00;
	RSPI0_WriteRead(send, recv, 2, dep);

	return recv[1];
}

/** RSPI0 Cycle Operation で送信するコマンド(角速度情報取得)
 * @param void
 * @retval void
 */
static void MPU6500_CycleSendCommand(void)
{
	send[0] = MPUREG_GYRO_ZOUT_H | MPU_READ_FLAG;
	send[1] = 0x00;
	send[2] = 0x00;

	RSPI0_WriteRead(send, angvel, 3, dep);
}

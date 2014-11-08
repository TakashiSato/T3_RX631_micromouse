/**
 * @file  main.c
 * @brief メインファイル
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include <machine.h>
#include "typedefine.h"
#include "iodefine.h"
#include "Global.h"
#include "Peripherals/Timer.h"
#include "Peripherals/SerialPort.h"
#include "Peripherals/RSPI.h"

#include "Devices/AD-128160-UART.h"
#include "Devices/LightSensor.h"
#include "Devices/MPU6500.h"
#include "Devices/AS5055.h"
#include "Devices/DRV8836.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/
#define SW !PORT1.PIDR.BIT.B4		//for Debug
//#define SW !PORTC.PIDR.BIT.B7

/*----------------------------------------------------------------------
	プロトタイプ宣言
----------------------------------------------------------------------*/
#ifdef __cplusplus
#endif
void main(void);
#ifdef __cplusplus
extern "C"
{
	void abort(void);
}
#endif

/*----------------------------------------------------------------------
	ソースコード
----------------------------------------------------------------------*/
/** メイン関数
 * @param void
 * @retval void
 */
void main(void)
{
	if(SERIAL_TARGET_IS_CONSOLE)
	{
		// シリアルポートの初期化(115200[bps])
		SCI_InitializeSerialPort(115200);
	}
	else
	{
		// AD-128160-UARTの初期化
		AD128160_InitializeSerialPort(9600);
		AD128160_Initialize();
	}
	Printf("Hello!\n");

	// 光センサの初期化
	LightSensor_Initialize();

	// RSPIの初期化
//	InitializeRSPI0();

	// MPU6500の初期化
//	MPU6500_Initialize();

	// AS5055の初期化
//	AS5055_Initialize();

	// DRV8836の初期化
	DRV8836_Initialize();
//	DRV8836_Wakeup();
//	DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 5);

	// IOポートの初期化
	PORT3.PDR.BIT.B1 = 1;
//	PORT2.PDR.BIT.B7 = 1;
	PORTE.PDR.BIT.B2 = 1;
//	// タクトスイッチ
	PORT1.PDR.BIT.B4 = 0;	// P14入力 for Debug
//	PORTC.PDR.BIT.B7 = 0;	// PC7入力

	Printf("Start!\n");
	WaitMS(100);

	// SPIサイクル動作開始
//	RSPI0_StartCycleOperation();

	while (1)
	{
//		PORT3.PODR.BIT.B1 = 0;
		PORTE.PODR.BIT.B2 = 0;

		WaitMS(50);

		// スイッチ入力判定
		if(SW)
		{
			Printf("SW\n");
			for(int i=0; i<200; i++)
			{
				PORTE.PODR.BIT.B2 = 1;
				WaitUS(200);
				PORTE.PODR.BIT.B2 = 0;
				WaitUS(200);
			}

//			float bat = (float)ad/4096*3*2;
//			AD128160_Locate(9, 0);
//			Printf(" Bat:%.2f[V]\n", bat);
		}
//
//		// MPU6500用
//		AD128160_Locate(4, 0);
//		Printf("AngVel:%6d\n", MPU6500_GetAngVel());
//
//		// AS5055用
//		AD128160_Locate(5, 0);
//		Printf("   Ang:%6d\n", AS5055_GetAngle());
//
		// DRV8836用
//		if(SW)
//		{
//			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 5);
//			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CCW, 5);
//		}
//		else
//		{
//			DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 3);
//			DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 3);
//		}
//		AD128160_Locate(3, 0);
//		Printf("L Duty:%3d\n", DRV8836_GetMotorDuty(MOTOR_TYPE_LEFT));
//		Printf("L Dir:%d\n", DRV8836_GetMotorDirection(MOTOR_TYPE_LEFT));
//		Printf("R Duty:%3d\n", DRV8836_GetMotorDuty(MOTOR_TYPE_RIGHT));
//		Printf("R Dir:%d\n", DRV8836_GetMotorDirection(MOTOR_TYPE_RIGHT));
//
//		PORT3.PODR.BIT.B1 = 1;

		// LightSensor
		AD128160_Locate(3, 0);
		Printf("AD:%4d\n", LightSensor_GetADValue());

		WaitMS(50);
	}

}

void abort(void)
{

}


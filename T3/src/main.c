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
#include "UserInterfaces/LED.h"
#include "UserInterfaces/Switch.h"
#include "UserInterfaces/Speaker.h"

#include "Devices/AD-128160-UART.h"
#include "Devices/LightSensor.h"
#include "Devices/MPU6500.h"
#include "Devices/AS5055.h"
#include "Devices/DRV8836.h"

/*----------------------------------------------------------------------
	Macro Definitions
 ----------------------------------------------------------------------*/

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
	// LEDの初期化
	LED_Initialize();

	// スイッチの初期化
	Switch_Initialize();

	// スピーカーの初期化
	Speaker_Initialize();

	// シリアル通信環境の初期化
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
//	LightSensor_Initialize();

	// RSPIの初期化
	InitializeRSPI0();

	// MPU6500の初期化
//	MPU6500_Initialize();

	// AS5055の初期化
	AS5055_Initialize();

	// DRV8836の初期化
	DRV8836_Initialize();
	DRV8836_Wakeup();

	Printf("Start!\n");
	PlaySound(500);
	PlaySound(300);
	PlaySound(100);

	// SPIサイクル動作開始
	RSPI0_StartCycleOperation();

	int mode = 0;

	while (1)
	{
		DispLED(0x01);
		WaitMS(50);

		// スイッチ入力判定
		if(GetSwitchState())
		{
			PlaySound(100);

			if(!mode)
			{
				PORT3.PODR.BIT.B1 = 1;
				PORT2.PODR.BIT.B7 = 1;
				PORTE.PODR.BIT.B3 = 1;
				PORTE.PODR.BIT.B1 = 1;
				DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CCW, 10);
				DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 10);
			}
			else
			{
				PORT3.PODR.BIT.B1 = 0;
				PORT2.PODR.BIT.B7 = 0;
				PORTE.PODR.BIT.B3 = 0;
				PORTE.PODR.BIT.B1 = 0;
				DRV8836_DriveMotor(MOTOR_TYPE_LEFT, MOTOR_DIR_CW, 10);
				DRV8836_DriveMotor(MOTOR_TYPE_RIGHT, MOTOR_DIR_CW, 10);
			}
			mode ^= 1;

//			float bat = (float)ad/4096*3*2;
//			AD128160_Locate(9, 0);
//			Printf(" Bat:%.2f[V]\n", bat);
		}

//		// MPU6500用
//		AD128160_Locate(4, 0);
//		Printf("AngVel:%6d\n", MPU6500_GetAngVel());
//
//		// AS5055用
//		AD128160_Locate(5, 0);
		Printf(" LAng:%6d\n", AS5055_GetAngle(ENC_L));
		Printf(" RAng:%6d\n", AS5055_GetAngle(ENC_R));

		// LightSensor用
//		AD128160_Locate(3, 0);
//		Printf("AD0:%5d\n", LightSensor_GetADValue(0));
//		Printf("AD1:%5d\n", LightSensor_GetADValue(1));
//		Printf("AD2:%5d\n", LightSensor_GetADValue(2));
//		Printf("AD3:%5d\n", LightSensor_GetADValue(3));

		DispLED(0x00);
		WaitMS(50);
	}

}

void abort(void)
{

}


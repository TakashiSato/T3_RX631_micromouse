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

#include "Controller/MouseController.h"
#include "Controller/Search.h"

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
//		SCI_InitializeSerialPort(115200);
		SCI_InitializeSerialPort(250000);
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
	InitializeRSPI0();
	DispLED(0x01);
	PlaySound(100);

	// MPU6500の初期化
	MPU6500_Initialize();
	DispLED(0x03);
	PlaySound(100);

	// AS5055の初期化
	AS5055_Initialize();
	DispLED(0x07);
	PlaySound(100);

	// DRV8836の初期化
	DRV8836_Initialize();
	DRV8836_Wakeup();
	DispLED(0x0F);
	PlaySound(100);

//	Printf("Start!\n");
	PlaySound(500);
	PlaySound(300);
	PlaySound(100);

	// SPIサイクル動作開始
//	RSPI0_StartCycleOperation();

	// 制御器の初期化
	MouseController_Initialize();

	while (1)
	{
		DispLED(0x01);
		WaitMS(50);

		// スイッチ入力判定
		if(GetSwitchState())
		{
			_UBYTE mode = ModeSelect();
			WaitMS(100);

			switch(mode)
			{
			case 1:
				LightSensor_GetBaseLR();
				MouseControlle_MotorTest();
				break;
			case 2:
				PlaySound(500);
				PlaySound(500);
				PlaySound(500);
				WaitMS(1000);
				LightSensor_GetBaseLR();
				Search_Adachi();
				break;
			case 3:
				MPU6500_LogMode();
				break;
			case 4:
				// AS5055用
				//AD128160_Locate(5, 0);
				while(!GetSwitchState())
				{
					Printf(" LAng:%6d, ", AS5055_GetAngle(ENC_L));
					Printf(" RAng:%6d\n", AS5055_GetAngle(ENC_R));
					_UBYTE led = (_UBYTE)(AS5055_GetAngle(ENC_L)/512);
					led = (led << 2) | ((_UBYTE)(AS5055_GetAngle(ENC_R)/512) & 0x03);
					DispLED(led);
					WaitMS(100);
				}
				break;
			case 5:
				while(!GetSwitchState())
				{
					Printf("Bat:%f\n", Battery_GetValue());
					WaitMS(100);
				}
				break;
			case 6:
				MouseController_CheckValue();
				break;
			default:
				//	LightSensor用
				LightSensor_GetBaseLR();
//				LightSensor_ValueCheckMode(false);
				LightSensor_ValueCheckMode(true);
				break;
			}
		}

		DispLED(0x00);
		WaitMS(50);
	}

}

void abort(void)
{

}


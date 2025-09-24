/*---------------------------------------------------------------------------  
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2023.01.25
 * C Compiler : 
 
  SERCOM0 / METER
  SERCOM1 / Host
  SERCOM2 / Debug
  PA18 ~ PA25 / input: Device ID Switch
  PA10 / Output: RS485 DIR for METER (0:Input)
  PA11 / Output: RS485 DIR for Host (0:Input)
  
  PA00 / ADC for Light Sensor
  PA01 / Bi- : I2C-SDA for E2PROM
  PA02 / Output: I2C-SCK for E2PROM
  PA03 / Output: E2PROM WP (0:Write protect)
  PB08 / Output: Status LED (0:LED ON)
  PB10 / Input: METER Digital Input (Low Counter)
  PB11 / Output: SSR Control (1: SSR ON)
 
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_MY_DEFINE__
#define __AO_MY_DEFINE__

//#define METER_TEST

#include "stdint.h"
#include <stdlib.h>
#include "ota_manager.h"

//	Per Meter Board
#define MtrBoardMax 1
#define PwrMtrMax 1
#define BmsMax			1
#define WtrMtrMax 1
#define InvMax			1
#define	PyrMtrMax 1
#define	SoilSensorMax 1
#define	AirSensorMax 1


#define MAX_WDT_TRIES				4
#define ROOM_MAX	31
#define CENTER_RECORD_MAX       200

#define METER_POLL_INTERVAL	    50

#define METER_DEVICE_MAX 		18

#define MEMBER_MAX_NO			18

#define RECORD_MAX				5
#define OPEN_DOOR_WORD		0x5A

#define RS485_TIMEOUT_TIME		25
#define WAIT_TIMEOUT_TIME		10
#define POLL_TIMEOUT_TIME		25
#define MAX_RETRY_TIMES		5
#define POLL_DELAY_TIME		2

#define POLLING_TIMEOUT  		20

#define GET_OTA_DONE_TIMEOUT  250

#define RS485_DIR_DELAY_TIME 	12
#define ERROR_RETRY_MAX		5
#define OTA_ERROR_RETRY_MAX		8

#define SYSTEM_ERROR_TIMEOUT		500

#define METER_PW1_Off()		(PD3 = 0)
#define METER_PW1_On()		(PD3 = 1)
#define METER_PW2_Off()		(PC4 = 0)
#define METER_PW2_On()		(PC4 = 1)
#define METER_PW3_Off()		(PE0 = 0)
#define METER_PW3_On()		(PE0 = 1)
#define METER_PW4_Off()		(PE10 = 0)
#define METER_PW4_On()		(PE10 = 1)

#define DIR_HOST_RS485_In()		//(PE11 = 0)
#define DIR_HOST_RS485_Out()	//(PE11 = 1)

#define DIR_DBG_RS485_In()		(PC1 = 0)
#define DIR_DBG_RS485_Out()		(PC1 = 1)

#define LED_G_On()				(PD7 = 0)
#define LED_G_Off()				(PD7 = 1)
#define LED_TGL_G()				(PD7 ^= 1)
#define LED_R_On()				(PF2 = 0)
#define LED_R_Off()				(PF2 = 1)
#define LED_TGL_R()				(PF2 ^= 1)

#define LED_G1_On()				(PE0 = 0)
#define LED_G1_Off()				(PE0 = 1)
#define LED_G1_TOGGLE()			(PE0 ^= 1)
#define LED_R1_On()				(PC4 = 0)
#define LED_R1_Off()				(PC4 = 1)
#define LED_R1_TOGGLE()			(PC4 ^= 1)


#define SYSTEM_SINGLE_MODE				0x00
#define SYSTEM_MULTI_MODE				(0x01<<0)

#define SYSTEM_AUTO_LOCK_ENABLE			0x00
#define SYSTEM_AUTO_LOCK_DISABLE		(0x01 << 1)

#define SYSTEM_INSIDE_LOCK_ENABLE		0x00
#define SYSTEM_INSIDE_LOCK_DISABLE		(0x01 << 2)

#define SYSTEM_LCD_ALWAYS                                 0x00
#define SYSTEM_LCD_SLEEP					(0x01<<5)

#define SYSTEM_POWER_ALWAYS			(0x01<<7)
#define SYSTEM_POWER_CONTROL			0x00


#define SYSTEM_AC_LOSS					(0x01<<6)

#define SYSTEM_TIME_READY				(0x01<<3)

// define Member.mode 
// 扣款中
#define FLAG_IN_CHARGE			(0x01 << 0)
        
// 電源權限管制
#define FLAG_STOP_POWER			(0x01 << 5)
// 開門權限管制
#define FLAG_STOP_DOOR			(0x01 << 6)
// 管理人員
#define FLAG_SUPER_USER			(0x01 << 7)

// define Room Status 
#define FLAG_USER_DATA_READY            (0x01 << 0)
#define FLAG_SYSTEM_TIME_READY        (0x01 << 1)
#define FLAG_POWER_METER_READY      (0x01 << 2)


// State 

// Commands
// Center => Meter 
#define MAX_NODE1_TXQ_LENGTH     	25
#define MAX_NODE1_RXQ_LENGTH     	25
#define MAX_NODE2_TXQ_LENGTH    	25
#define MAX_NODE2_RXQ_LENGTH    	25
#define NODE2_TOKEN_LENGTH		25
#define NODE1_TOKEN_LENGTH		25

#define MAX_HOST_TXQ_LENGTH  	100
#define MAX_HOST_RXQ_LENGTH		100
#define HOST_TOKEN_LENGTH			100
#define MAX_OTA_Q_LENGTH 			100

#define MAX_METER_TXQ_LENGTH    	100
#define MAX_METER_RXQ_LENGTH    	100
#define METER_TOKEN_LENGTH		100

// Response Code (Host)
#define RSP_CMD_RESULT				0xD0
#define RSP_REQ_RESULT				0xD1
#define RSP_MY_TOKEN_FINISH		0xD2
#define RSP_MY_TOKEN_NC			0xD3

#define RSP_NOT_MY_TOKEN			0xE0

#define RSP_ERROR					0xF0
#define RSP_ERROR_HOST_UART		0xF1
#define RSP_ERROR_METER_UART		0xF2

// Response Code (METER)
#define RSP_METER_TOKEN_FINISH		0xA0
#define RSP_METER_TOKEN_NC			0xA1

#define EEPROM_ADDR					0x50
#define EE_ADDR_ROOM_NAME				0x0010
#define EE_ADDR_ROOM_MODE				0x0015
#define EE_ADDR_ROOM_RATE				0x0016
#define EE_ADDR_ROOM_NO				0x0017
#define EE_ADDR_POWER100W			0x0018
#define EE_ADDR_STUDENT_ID			0x0020
#define EE_ADDR_USER_UID				0x0050
#define EE_ADDR_TIME_TAG				0x0075
#define EE_ADDR_USER_VALUE			0x00A0
#define EE_ADDR_BOOTLOADER_SWITCH	0x00B7
#define USER_CHECK_CODE_ADDR			0x00B0
#define EE_ADDR_PORT					0x00F0


#define INIT_S1	0
#define INIT_S2 	1
#define INIT_S3 	2
#define INIT_S4 	3
#define INIT_S5 	4
#define INIT_S6 	5
#define INIT_S7 	6
#define INIT_S8	7
#define INIT_S9	8
#define INIT_S10	9

// Room Mode
#define RM_NULL					0
#define RM_POWER_OFF_READY	1
#define RM_POWER_ON_READY		2
#define RM_FREE_MODE_READY	3
#define RM_STOP_MODE_READY	4
#define RM_SUPER_MODE_READY	5
#define RM_ERROR				6

// Time Index
#define INX_YEAR_H	    0
#define INX_YEAR_L	    1
#define INX_MON		2
#define INX_DAY		3
#define INX_HOUR	4
#define INX_MIN		5
#define INX_SEC		6
#define INX_WEEK	7

#define HOST_INX_TIME_START 		(HOST_TOKEN_LENGTH-10)
#define HOST_INX_TIME_SEC				(HOST_TOKEN_LENGTH-4)

#define INX_TIME_START_YY_H	  (METER_TOKEN_LENGTH-10)
#define INX_TIME_START_YY_L	  (METER_TOKEN_LENGTH-9)
#define INX_TIME_START_M	    (METER_TOKEN_LENGTH-8)
#define INX_TIME_START_D	    (METER_TOKEN_LENGTH-7)
#define INX_TIME_START_H	    (METER_TOKEN_LENGTH-6)
#define INX_TIME_START_MN   	(METER_TOKEN_LENGTH-5)
#define INX_TIME_START_S	    (METER_TOKEN_LENGTH-4)
#define INX_TIME_START_W	    (METER_TOKEN_LENGTH-3)

#define BALANCE_DEC		1
#define BALANCE_ADD	2

#define DOOR_RCD		0x01
#define POWER_RCD		0x02
#define TAG_CHG_RCD		0x03
#define DOOR_RCD_DIR	0x04
#define POWER_RCD_DIR	0x05
#define TAG_CHG_RCD_DIR	0x06


// FLAG for Meter Function 
#define FLAG_METER_INIT				(0x01 << 0)
#define FLAG_METER_CHG_MODE			(0x01 << 1)
#define FLAG_METER_CHG_BALANCE		(0x01 << 2)
#define FLAG_METER_GET_USER_INFO		(0x01 << 3)
#define FLAG_METER_GET_RCD			(0x01 << 4)
#define FLAG_METER_USER_CHG_MODE	(0x01 << 5)
#define FLAG_METER_CHG_USER_DATA	(0x01 << 6)
//#define FLAG_METER_INIT_BIT		(0x01 << 7)


// fgToMeterFlag : Send to Meter
#define TO_METER_CHG_MODE				(0x01 << 0)
#define TO_METER_CHG_BALANCE			(0x01 << 1)
#define TO_METER_OPEN_DOOR				(0x01 << 2)
#define TO_METER_SET_QRCODE				(0x01 << 3)
#define TO_METER_USER_EXIT_CGR			(0x01 << 4)
#define TO_METER_CHG_USER_MODE			(0x01 << 5)
#define FLAG_SYS_RST						(0x01 << 6)
#define FLAG_SYS_INIT						(0x01 << 7)

// FLAG : DataUpdated 
#define FLAG_DATA_PWR_RCD_NEW			(0x01 << 0)
#define FLAG_DATA_DOOR_RCD_NEW			(0x01 << 1)
#define FLAG_DATA_TAG_RCD_NEW			(0x01 << 2)
#define FLAG_NUM_CGR_UPDATED			(0x01 << 3)
#define FLAG_DATA_CHG_BAL				(0x01 << 4)
#define FLAG_METER_VALUE_UPDATED		(0x01 << 5)
#define FLAG_DATA_CHG_MODE				(0x01 << 6)
//#define FLAG_READER_SYS_RESET				(0x01 << 7)
// DataUpdated : FLAG_BAL_UPDATED


// FLAG fgFromMeterFlag :  Meter to Center  
#define FROM_METER_CHG_BALANCE		(0x01 << 0)
#define FROM_METER_CHG_MODE			(0x01 << 1)
//#define FROM_METER_CHG_				(0x01 << 2)

// FLAG fgToMeterRSPFlag
#define TO_METER_RSP_CLEAR_DATAUPDATED 	(0x01 << 7)


// FLAG fgFromHostFlag :  Center to Meter  
#define FROM_HOST_INIT				(0x01 << 0)
#define FROM_HOST_CHG_MODE			(0x01 << 1)
//#define FROM_METER_CHG_				(0x01 << 2)

// FLAG fgToHostFlag : To Center 
#define TO_CENTER_CHG_BALANCE		(0x01 << 0)
#define TO_CENTER_CHG_MODE			(0x01 << 1)
#define TO_CENTER_CHG_USER_MODE		(0x01 << 2)
#define TO_CENTER_PWR_RCD_NEW		(0x01 << 4)
#define TO_CENTER_DOOR_RCD_NEW		(0x01 << 5)
#define TO_CENTER_INIT_METER		(0x01 << 6)
#define TO_CENTER_CHG_USER_INFO		(0x01 << 7)


// State Modet
enum DEFINE_STATE_MODE {
STM_INIT=0,								// 0
STM_NORM,								// 1
STM_POWER_ON_READY,
STM_POWER_OFF_READY,
STM_CHARGE_FREE_WAIT,
STM_CHARGE_STOP_WAIT,
STM_POWER_ON,
STM_POWER_OFF,
STM_CHARGE_FREE,
STM_CHARGE_STOP,
STM_WAIT_ON_ACK,
STM_WAIT_OFF_ACK,
STM_WAIT_MODE,
STM_DELAY_SEND_ON,
STM_DELAY_SEND_OFF,
STM_REGISTER,
STM_DELAY_SEND_FAILURE,
	
        
};

enum DEFINE_RS485_METER_TOKEN {

METER_CMD_ALIVE=0x10,			//	0x10			
METER_CMD_POWER_METER,		//	0x11
METER_CMD_BMS,						// 	0x12
METER_CMD_WATER_METER,		//	0x13
METER_CMD_PYRANOMETER,	//0x14
METER_CMD_SOIL_SENSOR,	
METER_CMD_AIR_SENSOR,	
METER_CMD_INV,
METER_GET_CMD_WATERING,
METER_OTA_CMD,

METER_RSP_ACK=0x30,				//0x30
METER_RSP_SYS_INFO,				//0x31
METER_RSP_USER_DATA,		//0x32
METER_RSP_POWER_DATA,			//0x33
METER_RSP_BMS_DATA,
METER_RSP_WATER_DATA,
METER_RSP_PYR_DATA,			//0x36
METER_RSP_SOIL_DATA,				//0x37
METER_RSP_AIR_DATA,			//0x38
METER_RSP_INV_DATA,	
METER_RSP_WATERING_STATUS,
METER_RSP_FW_STATUS,

	//	Meter OTA
METER_OTA_UPDATE_CMD 		  = 0x50,
METER_SWITCH_FW_CMD,
METER_GET_FW_STATUS_CMD,
METER_REBOOT_CMD,
};


enum DEFINE_RS485_CENTER_TOKEN {

CTR_ALIVE=0x10,						//0x10			
CTR_GET_CMD_POWER_METER,	//0x11
CTR_GET_CMD_BMS,					//0x12
CTR_GET_CMD_WATER_METER,	//0x13
CTR_GET_CMD_INV,					//0x14
CTR_GET_CMD_PYRANOMETER,		//0x15
CTR_GET_CMD_SOIL_SENSOR,	//0x16
CTR_GET_CMD_AIR_SENSOR,			//0x17
CTR_OTA_CMD,				//0x18
CTR_2_METER_OTA_CMD,					//0x19
CTR_SET_WTR_TIME = 0x1A,					//0x1A

CTR_RSP_ACK=0x30,				//0x30
CTR_RSP_SYSTEM_INFO,		//0x31
CTR_RSP_POWER_DATA,			//0x32
CTR_FIRST_RESET_STATUS,	//0x33
CTR_RSP_BMS_DATA=0x35,				//0x35
CTR_RSP_WM_DATA,				//0x36
CTR_RSP_INV_DATA,				//0x37
CTR_RSP_PYR_DATA,				//0x38
CTR_RSP_SOIL_DATA,				//0x39
CTR_RSP_AIR_DATA,				//0x3A
CTR_RSP_WTR_TIME_SETUP,	//0x3B
CTR_RSP_MTR_FW_INFO,

	//------Cenetr  OTA-------//
CTR_OTA_UPDATE_CMD=0x40,		
CTR_SWITCH_FW_CMD,
CTR_GET_FW_STATUS_CMD,
CTR_REBOOT_CMD,
};

enum DEFINE_READER_SATE {
PL_METER_NORM,
PL_MtrBoard_SYSETM,
PL_MtrBoard_SYSETM_RSP,
PL_METER_POWERMETER,
PL_METER_POWERMETER_RSP,
PL_METER_BMS,
PL_METER_BMS_RSP,
PL_METER_WATERMETER,
PL_METER_WATERMETER_RSP,
PL_METER_INVERTER,
PL_METER_INVERTER_RSP,
PL_METER_PYRANOMETER,
PL_METER_PYRANOMETER_RSP,
PL_METER_SOILSENSOR,
PL_METER_SOILSENSOR_RSP,
PL_METER_AIRSENSOR,
PL_METER_AIRSENSOR_RSP,
	
PL_METER_WATERING,
PL_METER_WATERING_RSP,


//	CTR update MTR Cmds
PL_METER_OTA_CMD,
PL_METER_OTA_RSP,
PL_METER_OTA_CONNECT,
PL_METER_OTA_CONNECT_RSP,
PL_METER_OTA_METADATA,
PL_METER_OTA_METADATA_RSP,
PL_METER_OTA_ERASE_BANK,
PL_METER_OTA_ERASE_BANK_RSP,
PL_METER_OTA_SEND_PACK,
PL_METER_OTA_SEND_PACK_RSP,
PL_METER_OTA_RESEND_PACK,
PL_METER_OTA_RESEND_PACK_RSP,

PL_METER_OTA_GET_UPDATE_PWD,
};

enum DEFINE_METER_BOARD_POLLING_STATE{
PL_READY,
PL_POWER_METER_STATE,
PL_BMS_STATE,
PL_WM_STATE,
PL_INV_STATE,
PL_METER_BOARD_OTA_STATE,
};

typedef struct Watering_Setup{
	uint8_t Period_min;
} Watering_Setup_t ;

typedef struct  {    
	uint8_t ErrorRate;
	uint8_t RelayStatus;
	uint32_t TotalWatt;
	uint32_t V;
	uint32_t I;
	uint32_t F;
	uint32_t P;
	uint32_t VA;
	uint32_t PwrFactor;
} MeterData_t;

/***	Bms, WM, INV Data		***/
typedef struct STR_BMS_DATA{
	
		uint8_t	 ErrorRate;
		uint32_t CellStatus;
		uint16_t CellVolt[16];
		
		uint32_t BatWatt;
		uint32_t BatVolt;
		uint32_t BatCurrent;
	
		uint16_t MosTemp;
		uint16_t BatteryTemp[5];
	
		uint8_t StateOfCharge;
		uint8_t BalanceStatus;
		uint8_t	StateOfHealth;
		_Bool PrechargeStatus;
		_Bool ChargeState;
		_Bool DischargeState;
		
} BmsData_t;

typedef struct STR_WM_DATA{
		uint8_t ErrorRate;
		uint8_t ValveState;		
		uint32_t TotalVolume;
} WtrMtrData_t;

typedef struct STR_Pyranometer_DATA{
		uint8_t ErrorRate;
		uint16_t SolarRadiation;
		uint16_t OffsetValue;
} PyrMtrData_t;

typedef struct STR_SoilSensor_DATA{
		uint8_t ErrorRate;
		uint16_t Moisture;
    uint16_t Temperature;
    uint16_t EC;
    uint16_t PH;
    uint16_t Nitrogen;
    uint16_t Phosphorus;
    uint16_t Potassium;
    uint16_t Salinity;
    uint16_t TDS;
    uint16_t Fertility;

    uint16_t EC_Coef;
    uint16_t Salinity_Coef;
    uint16_t TDS_Coef;

    uint16_t Temp_Calib;
    uint16_t Moisture_Calib;
    uint16_t EC_Calib;
    uint16_t PH_Calib;

    uint32_t Fert_Coef;
    int16_t Fert_Deviation;

    uint32_t Nitrogen_Coef;
    int16_t Nitrogen_Deviation;

    uint32_t Phosphorus_Coef;
    int16_t Phosphorus_Deviation;

    uint32_t Potassium_Coef;
    int16_t Potassium_Deviation;

} SoilSensorData_t;

typedef struct STR_AirSensor_DATA{
		uint8_t ErrorRate;
    uint16_t Co2;          
    uint16_t Formaldehyde; 
    uint16_t Tvoc;         
    uint16_t Pm25;         
    uint16_t Pm10;         
    uint16_t Temperature;  
    uint16_t Humidity;

} AirSensorData_t;

typedef struct STR_INV_DATA{
	uint8_t ErrorRate;
	
	uint8_t statusByte1;
	uint8_t statusByte3;
	uint8_t warnByte1;
	uint8_t warnByte2;
	uint8_t faultByte1;
	uint8_t faultByte2;
	uint8_t faultByte3;

	uint16_t InputVolt;
	uint16_t InputFreq;
	uint16_t OutputVolt;
	uint16_t OutputFreq;
	
	uint16_t BatVolt;
	uint8_t BatCapacity;
	uint8_t InvCurrent;
	uint8_t LoadPercentage;
	uint8_t MachineTemp;
	uint8_t MachineStatusCode;
	uint8_t SysStatus;
	
	uint16_t PV_volt;
	uint8_t CtrlCurrent;
	uint8_t CtrlTemp;
	uint8_t CtrlStatusCode;
} InvData_t;



typedef struct RealTimeClock_Data{
	// Year,Month,Day,Hour,Min,Sec,Week
	uint8_t yy_h;
	uint8_t yy_l;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t week;
} RTC_Data_t;

typedef struct TotDeviceErrorRate
{
	uint8_t PowerMeter;	
	uint8_t Bms;
	uint8_t WaterMeter;
	uint8_t Pyranometer;
	uint8_t SoilSensor;
	uint8_t AirSensor;
	uint8_t Inverter;
	
}TotErrorRate_t;

typedef struct DeviceStatus
{
	uint32_t PowerMeterNG;
	uint32_t BmsNG;
	uint32_t WaterMeterNG;	
	uint32_t PyranometerNG;
	uint32_t SoilSensorNG;
	uint32_t AirSensorNG;
	uint8_t  InvNG;
	
}DeviceStatus_t;

#endif 


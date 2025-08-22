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
#include "ota_manager.h"

//	Per Meter Board
#define MtrBoardMax 1
#define PwrMeterMax 1
#define BmsMax			1
#define WtrMeterMax 1
#define InvMax			1



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

#define POLLING_TIMEOUT  		25
#define RS485_DIR_DELAY_TIME 	12
#define ERROR_RETRY_MAX		5

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


// SystemMode : 
// 	bit 0 = 1 => Multi-User / 0 => Sign User 
// 	bit 1 = 1 => Auto Lock / 0 => UnLock
// 	bit 2 = 1 => Inside Lock Enable / 0 => Disable
// 	bit 3 = 1 => System Time Ready ( from PC ) 
// 	bit 4 = 1 => 
// 	bit 5 = 1 => LCD Sleep Enable / 0 => Disable 
// 	bit 6 = 1 => AC Power Loss / 0 => AC Power
// 	bit 7 = 1 => No Power Control / 0 => Power Control 
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
#define INX_YEAR	    0
#define INX_MON		1
#define INX_DAY		2
#define INX_HOUR	    3
#define INX_MIN		4
#define INX_SEC		5

#define HOST_INX_TIME_START 		(HOST_TOKEN_LENGTH-9)
#define HOST_INX_TIME_SEC		(HOST_TOKEN_LENGTH-4)

#define INX_TIME_START_Y	    (METER_TOKEN_LENGTH-9)
#define INX_TIME_START_M	    (METER_TOKEN_LENGTH-8)
#define INX_TIME_START_D	    (METER_TOKEN_LENGTH-7)
#define INX_TIME_START_H	    (METER_TOKEN_LENGTH-6)
#define INX_TIME_START_MN   (METER_TOKEN_LENGTH-5)
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

METER_CMD_ALIVE=0x10,			//0x10			
METER_CMD_SET_USER_INFO,				//0x11
METER_CMD_CHG_ROOM_DATA, 			//0x12
METER_CMD_CHG_USER_DATA, 		//0x13
METER_CMD_GET_USER_DATA,			//0x14
METER_CMD_POWER_METER,			//0x15
METER_CMD_BMS,
METER_CMD_WATER_METER,
METER_CMD_INV,	

METER_RSP_OTA_UPDATE = 0x20,
METER_RSP_ACK=0x30,				//0x30
METER_RSP_SYS_INFO,				//0x31
METER_RSP_USER_DATA,		//0x32
METER_RSP_POWER_DATA,			//0x33
METER_RSP_BMS_DATA,
METER_RSP_WM_DATA,
METER_RSP_INV_DATA,	
// go to  // 


};


enum DEFINE_RS485_CENTER_TOKEN {

CTR_ALIVE=0x10,					//0x10			
CTR_GET_CMD_POWER_METER,			
CTR_GET_CMD_BMS,
CTR_GET_CMD_WATER_METER,
CTR_GET_CMD_INV,

CTR_OTA_UPDATE_CTR,
CTR_OTA_UPDATE_MTR,
	
//	Meter OTA
CMD_MTR_OTA_UPDATE 		  = 0x20,
CMD_MTR_SWITCH_FWVER,
CMD_GET_MTR_FW_STATUS,
CMD_MTR_FW_REBOOT,

CTR_RSP_ACK=0x30,				//0x30
CTR_RSP_SYSTEM_INFO,		//0x31
CTR_RSP_POWER_DATA,			//0x32
CTR_FIRST_RESET_STATUS,	//0x33
CTR_RSP_FW_INFO,				//0x34
CTR_RSP_BMS_DATA,
CTR_RSP_WM_DATA,
CTR_RSP_INV_DATA,

	//------Cenetr  OTA-------//
//CMD_CTR_OTA_UPDATE=0x40,		
//CMD_CTR_SWITCH_FWVER,
//CMD_GET_CTR_FW_STATUS,
//CMD_CTR_FW_REBOOT,

};

enum DEFINE_READER_SATE {
PL_METER_NORM,
PL_METER_POLL1,
PL_METER_POLL2,
PL_METER_POLL3,
PL_METER_POLL4,
PL_METER_POLL5,
PL_METER_POLL6,
PL_METER_POLL7,
PL_METER_POLL8,
PL_METER_POLL9,
PL_METER_POLL10,
PL_METER_POLL11,
PL_METER_POLL12,
	
//	CTR update MTR Cmds
PL_MTR_CMD_CONNECT,
PL_MTR_WAIT_CONNECT_RSP,
PL_MTR_CMD_METADATA,
PL_MTR_WAIT_METADATA_RSP,
PL_MTR_CMD_ERASE_APROM,
PL_MTR_WAIT_ERASE_APROM_RSP,
PL_MTR_CMD_UPDATE_APROM,
PL_MTR_WAIT_UPDATE_APROM_RSP,
PL_MTR_WAIT_MTR_BOOT_RSP,
};

enum DEFINE_METER_BOARD_POLLING_STATE{
PL_READY,
PL_POWER_METER_STATE,
PL_BMS_STATE,
PL_WM_STATE,
PL_INV_STATE,
PL_METER_BOARD_OTA_STATE,
};

typedef struct strUserInfo {
	uint8_t mode;				//  0x01 :  計費中
	uint8_t SID[4];				//學號
	uint8_t UID[4];				// 卡號	
	float  Balance;				//餘額			
} STR_UserInfo;

typedef struct STR_CTR_RECORD {
    uint8_t Status;                 // bit 7 :  0-Door(In) / 1-Power(Out) + bit 0~4 Meter ID 
    uint8_t UID[4];
    uint8_t DateTime[6];    
} STR_CENTER_RECORD ;


typedef struct STR_TAG_RECORD {		
	uint8_t TimeTag[6];
	uint8_t NumInCHG;	
} STR_TAG_CHG ;


typedef struct strRoomInfo {
    uint8_t RoomStatus;    	    
    uint8_t RoomMode;				// 目前計費模式(免費/停用/計費關電/計費開電)
    uint8_t ErrorRate;			
    
} STR_RoomInfo;

typedef struct STR_METER_DATA {    
		uint8_t ErrorRate;
		uint8_t RelayStatus;
		uint32_t TotalWatt;
} MeterData_t;

/***	Bms, WM, INV Data		***/
typedef struct STR_BMS_DATA{
	
		uint8_t	 ErrorRate;
		uint32_t BmsDeviceNG;
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
		uint32_t WMDeviceNG;
		uint8_t ValveState;		
		uint32_t TotalVolume;
} WMData_t;

typedef struct STR_INV_DATA{
	uint8_t ErrorRate;
	uint32_t InvDeviceNG;
	_Bool ChargingFlag;	// 0: not charging, 1: charging
	_Bool FaultFlag;		// 0: no,	1: yes
	_Bool WarnFlag;			// 0: No, 1: Yes
} InvData_t;

typedef struct STR_CTRL_DATA{
	
	_Bool ConnectFlag;	// 0:	disconnect, 	1: connected
	_Bool ChargingFlag;	// 0: not charging, 1: charging
	_Bool FaultFlag;		// 0: no,	1: yes
	_Bool WarnFlag;	// 0: No, 1: Yes
} CtrlData_t;

typedef struct STR_BAT_DATA{
	_Bool Full;		// 0: not full,		1: full
	
	_Bool LoadWarnFlag;
	_Bool TempWarnFlag;
	_Bool LoadTimeoutWarnFlag;
	_Bool LoadOverWarnFlag;
	_Bool BatHighVoltWarnFlag;
	_Bool BatLowVoltWarnFlag;
	_Bool StoreDataErrWarnFlag;
	_Bool StoreOpFailWarnFlag;
	
	_Bool InvFuncErrWarnFlag;
	_Bool PlanShutdownWarnFlag;
	_Bool OutputWarnFlag;
	
	_Bool InvErrFaultFlag;
	_Bool TempOverFaultFlag;
	_Bool TempSensorFaultFlag;
	_Bool LoadTimeoutFaultFlag;
	_Bool LoadErrFaultFlag;
	_Bool LoadOverFaultFlag;
	_Bool BatHighVoltFaultFlag;
	_Bool BatLowVoltFaultFlag;
	_Bool PlanShutdownFaultFlag;
	_Bool OutputErrFaultFlag;
	_Bool ChipStartFailFaultFlag;
	_Bool CurrentSensorFaultFlag;

} BatData_t;


#define SHOW_WAIT_DELAY		0x00
#define SHOW_USER_INFO			0x01
#define SHOW_TIME				0x02
#define SHOW_SYSTEM_OFF		0x03
#define SHOW_VERSION			0x04
#define SHOW_LOW_VALUE			0x05
#define SHOW_LOGO				0x06
#define SHOW_INVALID			0x07
#define SHOW_SETTING			0x08
#define SHOW_FAILURE			0x09
#define SHOW_SYSTEM_FREE		0x0A

#define BmsDeviceMax 0x01
#define WMDeviceMax  0x01
#define InvDeviceMax 0x01

#endif 


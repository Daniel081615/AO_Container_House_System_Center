/*
 * IncFile1.h
 *
 * Created: 2018/6/18 上午 10:27:04
 *  Author: Barry
 */ 


#ifndef _AO_EXTERN_FUNC_H_
#define _AO_EXTERN_FUNC_H_

#include "AO_MyDef.h"
/***
	OTA Functions
***/
extern void FwBankSwitchProcess(_Bool BackupValid);
extern _Bool IsFwValid(FwMeta * Meta);

extern void ReadParameterFromEE(void);
extern void WriteParameterFromEE(void);
extern void ReadUserInfoFromEE(void);
extern void WriteUserInfoToEE(void);
extern void ReadUserValueFromEE(void);
extern void WriteUserValueToEE(void);
extern void WriteRoomInfoToEE(void);
extern void ReadRoomInfoFromEE(void);
extern void PowerCostCal(void);

extern void ResetHostUART(void);
extern void ResetMeterUART(void);
extern void ResetNode2UART(void);

extern void CheckUserValue(uint8_t fnRoomIndex, uint8_t *fnUID);
extern void SendReader_ValueFailure(void);
extern void WriteUserInfoToEE_One(uint8_t fMemberIndex);
extern void WriteUserValueToEE_One(uint8_t fMemberIndex);
//extern void SendReader_PowerDisable(void);
extern void SaveCMD2HostSendQ(void);
extern uint16_t ReadUserValueFromEE_One(uint8_t fMemberIndex);
extern void SendHost_MeterStatus(void);
extern void SendHost_GetInfo(void);
extern void WritePowerToEE(void);
extern void SendHost_RspReaderInformation(void);
extern void SendHost_RspReadEE(void);
extern void SendReader_SystemSW(void);
extern void SendMeter_UserInformation(uint8_t PacketIndex);
extern void SendHost_Ack(void);
extern void SendHost_MeterFwInfo(void);
extern void SendHost_MeterUpdateSuccess(void);
extern void SendHost_CenterUpdateSuccess(void);
extern void RecordAddNew(uint8_t fnRoomIndex);
extern void RoomPowerSetting(uint8_t fnRoomIndex, uint8_t fnStatus );
extern void SendMeter_PollingAlive(void);
extern void SendMeter_GetUserInformation(uint8_t fnMemberIndex);
extern void SendMeter_AliveToken(void);
extern void SendMeter_SystemSW(void);
extern void CalChecksumM(void);

extern uint8_t MyCenterID;

/***	Time func & variable	***/
extern uint8_t CtrSystemTime[8], HostSystemTime[8], WateringTime[4];
extern void GetHostRTC(void);

extern uint8_t fgHostFlag;
extern uint8_t PackageIndex1;
extern uint8_t MeterRCDBuffer[44];
extern uint8_t UserUID[4];
extern uint8_t MeterMemberIndex;
extern uint8_t MeterMessageType;
extern uint8_t MeterWaitTick;
extern uint8_t TickPollingInterval_Meter;
extern uint8_t PollRetryCounter_Meter;
extern uint8_t MeterWaitTick;
extern uint8_t MeterMemberIndex;
extern uint8_t MeterMessageType;
extern uint8_t NowRCD_MeterIndex,NowRCD_Type;
extern uint8_t MeterResult;
extern uint8_t MeterSystemCommand[5];
extern uint8_t MeterBalanceOP;
extern uint8_t QRBuffer[45];
extern uint16_t QRCodeLen;

extern uint8_t HostTxBuffer[HOST_TOKEN_LENGTH];
extern uint8_t MeterTxBuffer[METER_TOKEN_LENGTH];
extern uint8_t TokenHost[HOST_TOKEN_LENGTH];
extern uint8_t TokenMeter[METER_TOKEN_LENGTH];
extern uint8_t HOSTRxQ[MAX_HOST_RXQ_LENGTH];
extern uint8_t HOSTTxQ[MAX_HOST_TXQ_LENGTH];
extern uint8_t METERRxQ[MAX_METER_RXQ_LENGTH];
extern uint8_t METERTxQ[MAX_METER_TXQ_LENGTH];

extern uint8_t HOSTRxQ_wp,HOSTRxQ_rp,HOSTRxQ_cnt;
extern uint8_t HOSTTxQ_wp,HOSTTxQ_rp,HOSTTxQ_cnt;
extern uint8_t METERTxQ_wp,METERTxQ_rp,METERTxQ_cnt;
extern uint8_t METERRxQ_wp,METERRxQ_rp,METERRxQ_cnt;

extern uint8_t NowPollingPwrMtrID,NowPollingBmsID, NowPollingWtrMtrID, NowPollingPyrMtrID, NowPollingSoilSensorID, NowPollingAirSensorID, NowPollingInvID;
extern uint8_t RoomMax;
extern uint8_t HostTokenReady,MeterTokenReady;
extern uint8_t TickHost,TickMeter;
extern uint8_t iTickDelaySendHostCMD,bDelaySendHostCMD,bValueUpdated,bSystemTimeReady;
extern uint8_t CenterRecord_WP,CenterRecord_RP,CenterNewRecordCounter,TickRecord;
extern uint32_t ReaderDeviceError;
extern uint8_t ControlState;
extern uint8_t HostPackageIndex;
extern uint8_t AutoLockTime;			// �۰ʤW�굥�ݮɶ�
extern uint8_t iStatus;
extern uint8_t CheckUserResult;

extern uint8_t MemberIndex;
extern uint8_t CenterID;
extern uint8_t TickPollWating;
extern uint32_t ReaderDeviceError;

extern uint8_t TickHostUart, TickMeterUart;
extern volatile uint8_t NowPollingMtrBoard;
extern uint8_t MeterMode,MeterUser,AlivePacketIndex;
extern uint16_t MeterBalance;
extern uint8_t NodeTestAck;
extern const uint8_t DefinePwrMtrMax[3];

extern uint8_t MeterBoardError;
extern uint8_t PollingCounter,GotMeterRspID, HostDeviceIndex;
extern uint8_t ReaderState,ReaderTick,bSendReaderCommand,ReaderCommandType;

extern _Bool fgMeterAckOK;
extern uint32_t MeterButtonStatus,MeterRelayStatus,MeterDeviceError;

//	Host Cmds devices List
extern _Bool flagMeterOTAUpdate;
extern uint8_t MeterOtaCmdList[MtrBoardMax];
extern uint8_t PwrMtrCmdList[MtrBoardMax][PwrMtrMax];
extern uint8_t BmsCmdList[MtrBoardMax][BmsMax];
extern uint8_t WtrMtrCmdList[MtrBoardMax][WtrMtrMax];
extern uint8_t PyrMtrCmdList[MtrBoardMax][PyrMtrMax];
extern uint8_t SoilSensorCmdList[MtrBoardMax][SoilSensorMax];

extern _Bool bResetUARTQ;

extern uint8_t system_SW_Flag;
extern uint8_t system_SW_Timer;
extern uint16_t BalanceTempData[MEMBER_MAX_NO];
extern uint8_t  BalanceSerialNo[MEMBER_MAX_NO];
extern uint8_t BalanceOpType;
extern uint8_t  CenterPackageChecksum,FormMeterFlag;

extern _Bool fgReaderReset ;
extern uint8_t Tick20mS_ReaderReset;

extern void WDT_Reset_System(void);

extern uint8_t centerResetStatus,MeterDeviceMax;


extern volatile _Bool fgMeterOta;

extern uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len);

/***	Devices Data structures	***/
extern RTC_Data_t RTC_Data;
extern _Bool WATERING_SETTING_FLAG;

extern void CalChecksumH(void);

extern volatile	MeterData_t 			MeterData[MtrBoardMax][PwrMtrMax];
extern volatile	BmsData_t 				BmsData[MtrBoardMax][BmsMax];
extern volatile WtrMtrData_t			WtrMtrData[MtrBoardMax][WtrMtrMax];
extern volatile PyrMtrData_t			PyrMtrData[MtrBoardMax][PyrMtrMax];
extern volatile AirSensorData_t 	AirSensorData[MtrBoardMax][AirSensorMax];
extern volatile SoilSensorData_t 	SoilSensorData[MtrBoardMax][SoilSensorMax];
extern volatile InvData_t 				InvData[MtrBoardMax];
extern volatile	TotErrorRate_t 		TotErrorRate[MtrBoardMax];
extern volatile DeviceStatus_t 		DevicesNG[MtrBoardMax];
extern volatile Watering_Setup_t 	Watering_SetUp[MtrBoardMax];
extern volatile	_Bool 						WateringStatus[MtrBoardMax];

#endif  //_AO_EXTERN_FUNC_H_

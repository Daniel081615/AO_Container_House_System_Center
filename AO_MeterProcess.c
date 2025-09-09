/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * --------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_EE24C.h"
#include "AO_HostProcess.h"

void MeterProcess(void);
void CalChecksumM(void);

void SendMeter_AliveToken(void);
void Meter_AckProcess(void);

void Meter_RSP_SysInformation(void);
void Meter_RSP_PowerData(void);
void Meter_RSP_BmsData(void);
void Meter_RSP_WaterData(void);
void Meter_RSP_PyranometerData(void);
void Meter_RSP_SoilSensorData(void);
void Meter_RSP_AirSensorData(void);
void Meter_RSP_InvData(void);
void Meter_RSP_FWInfo(void);

//	Meter Polling Cmds
void SendMeter_GetPowerData(void);
void SendMeter_GetBmsData(void);
void SendMeter_GetWaterData(void);
void SendMeter_GetPyrMtrData(void);
void SendMeter_GetSoilData(void);
void SendMeter_GetAirData(void);
void SendMeter_GetInvData(void);
void SendMeter_WateringTimeSetup(void);

void SendMeter_MeterOTACmd(uint8_t cmd);
void SendMeterRTC(void);

void PollSuccess_Handler(uint8_t NextPollingState);
void PollingTimeout_Handler(uint8_t nextStateOnRetry, uint8_t nextStateOnMaxRetry);


/*** MeterBootloader ***/
void SendMeterBootloader_ConnectCmd(void);
void SendMeterBootloader_UpdateMetadata (void);
void SendMeterBootloader_CmdUpdateAPROM (void);
void SendMeterBootloader_CmdEraseAPROM (void);

volatile	MeterData_t 			MeterData[MtrBoardMax][PwrMtrMax];
volatile	BmsData_t 				BmsData[MtrBoardMax][BmsMax];
volatile 	WtrMtrData_t			WtrMtrData[MtrBoardMax][WtrMtrMax];
volatile 	PyrMtrData_t			PyrMtrData[MtrBoardMax][PyrMtrMax];
volatile 	AirSensorData_t 	AirSensorData[MtrBoardMax][AirSensorMax];
volatile 	SoilSensorData_t 	SoilSensorData[MtrBoardMax][SoilSensorMax];
volatile 	InvData_t 				InvData[MtrBoardMax];
volatile 	TotErrorRate_t 		TotErrorRate[MtrBoardMax];
volatile 	DeviceStatus_t 		DevicesNG[MtrBoardMax];
volatile 	Watering_Setup_t 	Watering_SetUp[MtrBoardMax];

uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len);
uint8_t MeterPollingState;
uint8_t UserIndex1;
uint8_t ErrorCounter;
uint8_t MtrBoardIdx; 

//*** OTA Update ***//
uint8_t g_packno;
uint8_t lcmd;

uint32_t StartAddress;
uint32_t TotalLen;
uint32_t srclen;


_Bool fgMeterInitCompleted;
_Bool EnPollPwrMtrFlag, EnPollBmsFlag, EnPollWtrMtrFlag, EnPollPyrFlag, EnPollSoilSensorFlag, EnPollAirSensorFlag, EnPollInvFlag;

/***	
	@note 		When finish all devices polling process, poll the consecutive meter board
	@brief 		i.	Meter Polling States: 
							1. Power meter, 
							2. Bms, 
							3. Water meter, 
							4. Inverter, 
							5. Pyranometer
							6. Soil sensor
							7. Air sensor
							8. MeterBoard OTA
						ii.	Send host Cmds to meter devices, by using subcmd.
						
	@note			Add ack process
	@revise 	2025.08.28
***/
void MeterBoardPolling(void)
{

		uint8_t MeterOtaCmd;

    MtrBoardIdx = NowPollingMtrBoard-1;		
		//Meter Cmd List
		MeterOtaCmd = MeterOtaCmdList[OTAMeterID-1];
		
		//	I want to Poll the other board when the polling process of four devices is finished.
		if ((EnPollPwrMtrFlag == FALSE) && (EnPollBmsFlag == FALSE) && 
				(EnPollWtrMtrFlag 				== FALSE) && (EnPollInvFlag == FALSE) && 
				(EnPollPyrFlag 				== FALSE) && (EnPollSoilSensorFlag == FALSE) && 
				(EnPollAirSensorFlag == FALSE))
		{
				//	Start polling over again
				NowPollingPwrMtrID = 1;	
				NowPollingBmsID = 1;
				NowPollingWtrMtrID 	= 1;
				NowPollingInvID = 1;	
				NowPollingPyrMtrID = 1;
				NowPollingSoilSensorID = 1;
				NowPollingAirSensorID = 1;
			
			
				EnPollPwrMtrFlag = 1;
				EnPollBmsFlag = 1;
				EnPollWtrMtrFlag 	= 1;
				EnPollPyrFlag = 1;
				EnPollSoilSensorFlag 	= 1;
				EnPollAirSensorFlag = 1;
				EnPollInvFlag = 1;
				
				NowPollingMtrBoard++;
        if (NowPollingMtrBoard > 1) {
            NowPollingMtrBoard = 1;
        }
				//	Polling all over again
				MeterPollingState = PL_METER_NORM;
		}

    switch ( MeterPollingState )
    {			
        case PL_METER_NORM :
            
            MeterPollingState = PL_MtrBoard_SYSETM ;            
            break;
				
        case PL_MtrBoard_SYSETM :
						if (!EnPollPwrMtrFlag){
								MeterPollingState = PL_METER_POLL3;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_AliveToken();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_MtrBoard_SYSETM_RSP ;
            }
            break;
						
        case PL_MtrBoard_SYSETM_RSP :
            if (MeterRspID == NowPollingMtrBoard)
            {
								PollSuccess_Handler(PL_METER_POLL3);
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_MtrBoard_SYSETM, PL_METER_POLL3);
										if (MeterPollingState == PL_METER_POLL3) {
												NowPollingPwrMtrID++;
												if (NowPollingPwrMtrID > PwrMtrMax){
														EnPollPwrMtrFlag = FALSE;
												}
										}
						}
            break;
						
        case PL_METER_POLL3 :	
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetPowerData();                           
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL4 ;
            }
            break;
						
        case PL_METER_POLL4 :
            if (MeterRspID == NowPollingMtrBoard)
            {		
								PwrMtrCmdList[MtrBoardIdx][NowPollingPwrMtrID-1] = 0;
								PollSuccess_Handler(PL_METER_POLL5);
								//	Poll next power meter, or stop polling
								NowPollingPwrMtrID++;
								if (NowPollingPwrMtrID > PwrMtrMax){
										EnPollPwrMtrFlag = FALSE;
								}
						} else {
                // Timeout 
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL3, PL_METER_POLL5);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL5) {
												NowPollingPwrMtrID++;
												if (NowPollingPwrMtrID > PwrMtrMax){
														EnPollPwrMtrFlag = FALSE;
												}
										}
            }
            break;  
						
        case PL_METER_POLL5 :
						if (!EnPollBmsFlag){
								MeterPollingState = PL_METER_POLL7;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetBmsData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL6;
            }
            break;
						
        case PL_METER_POLL6 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								BmsCmdList[MtrBoardIdx][NowPollingBmsID-1] = 0;
								PollSuccess_Handler(PL_METER_POLL7);
								//	Poll next Bms, or stop polling
								NowPollingBmsID++;
								if (NowPollingBmsID > BmsMax){
										EnPollBmsFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL5,PL_METER_POLL7);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL7) {
												NowPollingBmsID++;
												if (NowPollingBmsID > BmsMax){
														EnPollBmsFlag = FALSE;
												}
										}
            }
            break;						
						
        case PL_METER_POLL7 :	
						if (!EnPollWtrMtrFlag){
								MeterPollingState = PL_METER_POLL9;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetWaterData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL8;
            }
            break;
						
        case PL_METER_POLL8 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								WtrMtrCmdList[MtrBoardIdx][NowPollingWtrMtrID-1] = 0;
								PollSuccess_Handler(PL_METER_POLL9);
								//	Poll next WM, or stop polling
								NowPollingWtrMtrID++;
								if (NowPollingWtrMtrID > WtrMtrMax){
										EnPollWtrMtrFlag = FALSE;
								}
															
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL7,PL_METER_POLL9);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL9) {
												NowPollingWtrMtrID++;
												if (NowPollingWtrMtrID > WtrMtrMax){
														EnPollWtrMtrFlag = FALSE;
												}
										}
            }
            break;								

        case PL_METER_POLL9 :	
						if (!EnPollInvFlag){
								MeterPollingState = PL_METER_POLL11;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetInvData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL10;
            }
            break;
						
        case PL_METER_POLL10 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								PollSuccess_Handler(PL_METER_POLL11);
								//	Poll next Bms, or stop polling
								NowPollingInvID++;
								if (NowPollingInvID > InvMax){
										EnPollInvFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL9,PL_METER_POLL11);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL11) {
												NowPollingInvID++;
												if (NowPollingInvID > InvMax){
														EnPollInvFlag = FALSE;
												}
										}
            }
            break;		

        case PL_METER_POLL11 :	
						if (!EnPollPyrFlag){
								MeterPollingState = PL_METER_POLL13;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetPyrMtrData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL12;
            }
            break;
						
        case PL_METER_POLL12 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								PyrMtrCmdList[MtrBoardIdx][NowPollingPyrMtrID-1] = 0;
								PollSuccess_Handler(PL_METER_POLL13);
								//	Poll next Bms, or stop polling
								NowPollingPyrMtrID++;
								if (NowPollingPyrMtrID > PyrMtrMax){
										EnPollPyrFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL11,PL_METER_POLL13);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL13) {
												NowPollingPyrMtrID++;
												if (NowPollingPyrMtrID > PyrMtrMax){
														EnPollPyrFlag = FALSE;
												}
										}
            }
            break;		

        case PL_METER_POLL13 :	
						if (!EnPollSoilSensorFlag){
								MeterPollingState = PL_METER_POLL15;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetSoilData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL14;
            }
            break;
						
        case PL_METER_POLL14 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								SoilSensorCmdList[MtrBoardIdx][NowPollingSoilSensorID-1] = 0;
								PollSuccess_Handler(PL_METER_POLL15);
								//	Poll next Bms, or stop polling
								NowPollingSoilSensorID++;
								if (NowPollingSoilSensorID > SoilSensorMax){
										EnPollSoilSensorFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL13,PL_METER_POLL15);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL15) {
												NowPollingSoilSensorID++;
												if (NowPollingSoilSensorID > SoilSensorMax){
														EnPollSoilSensorFlag = FALSE;
												}
										}
            }
            break;						

        case PL_METER_POLL15 :	
						if (!EnPollAirSensorFlag){
								MeterPollingState = PL_METER_POLL17;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetAirData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL16;
            }
            break;
        case PL_METER_POLL16 :
            if (MeterRspID == NowPollingMtrBoard)
            {
								PollSuccess_Handler(PL_METER_POLL17);
								//	Poll next Bms, or stop polling
								NowPollingAirSensorID++;
								if (NowPollingAirSensorID > AirSensorMax){
										EnPollAirSensorFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL15,PL_METER_POLL17);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL17) {
												NowPollingAirSensorID++;
												if (NowPollingAirSensorID > AirSensorMax){
														EnPollAirSensorFlag = FALSE;
												}
										}
            }
            break;	
						
				/***
						Process Meter Cmds, if Cmds != CMD_MTR_OTA_UPDATE, poll the next meter
						if Cmd == CMD_MTR_OTA_UPDATE
				 ***/
        case PL_METER_POLL17 :
					
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
								if (MeterOtaCmd != 0) {
										SendMeter_MeterOTACmd(MeterOtaCmd);
										TickPollingInterval_Meter = 0 ;
										MeterPollingState = PL_METER_POLL18 ;
								} 
									else if (WATERING_SETTING_FLAG == TRUE) {
										SendMeter_WateringTimeSetup();
										MeterPollingState = PL_METER_WATERING;
								} 
									else {
										MeterPollingState = PL_METER_NORM;
								}
            }
            break;
						
				case PL_METER_WATERING :
            if (MeterRspID == NowPollingMtrBoard)
            {
								PollSuccess_Handler(PL_METER_NORM);
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL17,PL_METER_NORM);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_NORM) {
												NowPollingAirSensorID++;
										}
            }
						break;

						
				case PL_METER_POLL18:
						if (MeterRspID == NowPollingMtrBoard)
						{
								SendHost_MeterFWInfo();
								MeterOtaCmdList[OTAMeterID-1] = 0;
								PollSuccess_Handler(PL_METER_NORM);
							
								// if MTR OTA Update Cmd & Meter APP flag , go to OTA state machine 
								//	!!!	 Or change to a sequence of OTA update, first go update Center, then when detect Meter APP flag, Automatically update Meter
								if(MeterOtaCmd == CMD_MTR_OTA_UPDATE)
								{
										MeterOtaFlag = 1;
										StartAddress = other.fw_start_addr;
										TotalLen 		 = other.fw_size;
									
										MeterOtaCmdList[OTAMeterID-1] = 0;
										MeterPollingState = PL_MTR_CMD_CONNECT;
										break;
								}
						}
						//	if Timeout, do Update next round
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT)
								PollingTimeout_Handler(PL_METER_POLL17, PL_METER_NORM);
						break;
						
/*+-------------------------------------------------------------+
	|										OTA Update State machine									|
	+-------------------------------------------------------------+	*/
				
				case PL_MTR_CMD_CONNECT:
				
						if (MeterWaitTick > 2){
								MeterRspID = 0xFF ;	/* make sure Center recevied meter Rsp */
								ResetMeterUART();
								SendMeterBootloader_ConnectCmd();
								MeterPollingState = PL_MTR_WAIT_CONNECT_RSP;
						}
						break;
				
				//	Wait for Meter Rsp
				case PL_MTR_WAIT_CONNECT_RSP:
						if (MeterRspID == NowPollingMtrBoard)
						{
								MtrBoardIdx = NowPollingMtrBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << MtrBoardIdx);
							
								MeterPollingState = PL_MTR_CMD_METADATA ;
							
						} 
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT)
								PollingTimeout_Handler(PL_MTR_CMD_CONNECT, PL_METER_NORM);
						break;
							
				case PL_MTR_CMD_METADATA:
						if (MeterWaitTick > 2){
								MeterRspID = 0xFF ;	/* make sure Center recevied meter Rsp */
								ResetMeterUART();
								SendMeterBootloader_UpdateMetadata();
								MeterPollingState = PL_MTR_WAIT_METADATA_RSP;
						}
						break;
						
				case PL_MTR_WAIT_METADATA_RSP:
						if (MeterRspID == NowPollingMtrBoard)
						{
								MtrBoardIdx = NowPollingMtrBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << MtrBoardIdx);
							
								MeterPollingState =  PL_MTR_CMD_ERASE_APROM;
						} 
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT) {
								PollingTimeout_Handler(PL_MTR_CMD_METADATA, PL_METER_NORM);
						}
						break;
				
				case PL_MTR_CMD_ERASE_APROM:
						if (MeterWaitTick > 2)
						{
								MeterRspID = 0xFF ;	/* make sure Center recevied meter Rsp */
								ResetMeterUART();
								SendMeterBootloader_CmdEraseAPROM ();
								MeterPollingState = PL_MTR_WAIT_ERASE_APROM_RSP;
						}
						break;
						
				case PL_MTR_WAIT_ERASE_APROM_RSP:
						if (MeterRspID == NowPollingMtrBoard)
						{
								MtrBoardIdx = NowPollingMtrBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << MtrBoardIdx);
							
								MeterPollingState =  PL_MTR_CMD_UPDATE_APROM;//NextStage;
						} 
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT) {
								PollingTimeout_Handler(PL_MTR_CMD_ERASE_APROM, PL_METER_NORM);
						}
						break;
						
				case PL_MTR_CMD_UPDATE_APROM:
							if (MeterWaitTick > 2)
							{
									MeterRspID = 0xFF ;	/* make sure Center recevied meter Rsp */
									ResetMeterUART();
									SendMeterBootloader_CmdUpdateAPROM ();
									MeterPollingState = PL_MTR_WAIT_UPDATE_APROM_RSP;
							}
						break;
							
				case 	PL_MTR_WAIT_UPDATE_APROM_RSP:
						if (MeterRspID == NowPollingMtrBoard)
						{
								MtrBoardIdx = NowPollingMtrBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << MtrBoardIdx);
							
								//	Prepare data for next package
								StartAddress += srclen;
								TotalLen -= srclen;
							
								if (TotalLen == 0)
								{
										MeterPollingState =  PL_MTR_WAIT_MTR_BOOT_RSP;
								} else {
										MeterPollingState =  PL_MTR_CMD_UPDATE_APROM;
								}
							
								
						} 
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT)
						{
								PollingTimeout_Handler(PL_MTR_CMD_UPDATE_APROM, PL_METER_NORM);
						}
						break;
				case	PL_MTR_WAIT_MTR_BOOT_RSP:
						
						if (MeterRspID == NowPollingMtrBoard)
						{
								MtrBoardIdx = NowPollingMtrBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << MtrBoardIdx);
							
								//	Meter Send Host 0x0ABBC0DD When Boot
								if (TokenMeter[4] == 0x0A &&
										TokenMeter[5] == 0xBB &&
										TokenMeter[6] == 0xC0 &&
										TokenMeter[7] == 0xDD)
								{
										NowPollingPwrMtrID++;
									
										if (NowPollingPwrMtrID > PwrMtrMax )
												NowPollingPwrMtrID = 1 ;
										MeterPollingState = PL_METER_NORM ;
								}
						} 
						else if (TickPollingInterval_Meter > POLLING_TIMEOUT) {
								MeterOtaCmdList[OTAMeterID-1] = CMD_MTR_OTA_UPDATE;
								PollingTimeout_Handler(PL_METER_NORM, PL_METER_NORM);
						}
						break;
							

        default :
            break;
    }			
}

/***
 *	@brief	Handles timeout condition
 ***/

void PollingTimeout_Handler(uint8_t nextStateOnRetry, uint8_t nextStateOnMaxRetry) 
{
    MtrBoardIdx = NowPollingMtrBoard - 1;
    MeterWaitTick = 0;
    PollRetryCounter_Meter++;
	
    if (PollRetryCounter_Meter > ERROR_RETRY_MAX) 
		{
        PollRetryCounter_Meter = 0;
        MeterDeviceError |= (0x01 << MtrBoardIdx);
			
				//	if ota timeout, resent..., when exceed resent tries, quit Ota mode.
				if (MeterOtaFlag == 1) {
            MeterOtaFlag = 0;
        }
				
        MeterPollingState = nextStateOnMaxRetry;
    } else {
        MeterPollingState = nextStateOnRetry;
    }
}

void PollSuccess_Handler(uint8_t NextPollingState) {
    MtrBoardIdx = NowPollingMtrBoard - 1;
    PollRetryCounter_Meter = 0;
    MeterWaitTick = 0;
    ErrorCounter = 0;
    MeterDeviceError &= (~(0x01 << MtrBoardIdx));
		MeterPollingState = NextPollingState;
}


void MeterProcess(void)
{
		uint8_t i,checksum;
		//uint8_t fnRoomIndex;

		if ( TickMeter == 49 )
		{
			TickMeter++;
			ResetMeterUART();	
		}
	
		if( MeterTokenReady )
		{
				MeterTokenReady = 0 ;
				checksum = 0 ;
				
				for(i=1;i<(MAX_METER_RXQ_LENGTH-2);i++)
				{
						checksum += TokenMeter[i];
				}
				
				if ((TokenMeter[0] == 0x55) && (TokenMeter[MAX_METER_RXQ_LENGTH-2] == checksum))
				{
						LED_TGL_G();
						
						MeterRspID = TokenMeter[1];

						if (MeterOtaFlag) 
						{
								g_packno = TokenMeter[3];
						} else {
						
								switch( TokenMeter[2] )
								{
									case METER_RSP_ACK :
										Meter_AckProcess();
										fgMeterAckOK = 1 ;									
										break;
									
									case METER_RSP_SYS_INFO :
										Meter_RSP_SysInformation();
										break;
									
									case METER_RSP_POWER_DATA :
										Meter_RSP_PowerData();							
										break;
									
									case METER_RSP_BMS_DATA:
										Meter_RSP_BmsData();
										break;
									
									case METER_RSP_WATER_DATA:
										Meter_RSP_WaterData();
										break;
									
									case METER_RSP_PYR_DATA:
										Meter_RSP_PyranometerData();
										break;
									
									case METER_RSP_SOIL_DATA:
										Meter_RSP_SoilSensorData();
										break;
									
									case METER_RSP_AIR_DATA:
										Meter_RSP_AirSensorData();
										break;

									case METER_RSP_INV_DATA:
										Meter_RSP_InvData();
										break;									
									
									case METER_RSP_OTA_UPDATE :
										Meter_RSP_FWInfo();
									
									default:					
										break;
								}
						}
				} else {
						ResetMeterUART();
				}
		}
}

void Meter_AckProcess(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx, u8tmp;
    uint32_t u32tmp;

		MtrBoardIdx = TokenMeter[1]-1;
    PktIdx  = 5 ;
	
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;    
    DevicesNG[MtrBoardIdx].PowerMeterNG = u32tmp;

    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ; 	
		DevicesNG[MtrBoardIdx].BmsNG = u32tmp;
	
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;
		DevicesNG[MtrBoardIdx].WaterMeterNG = u32tmp;

    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;
		DevicesNG[MtrBoardIdx].InvNG = u8tmp;
	
}

void Meter_RSP_SysInformation(void)
{    
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint32_t u32tmp;		
	
		MtrBoardIdx = TokenMeter[1]-1;
    PktIdx  = 5 ;
		
		//	PowerMeter Error Status
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;  
		DevicesNG[MtrBoardIdx].PowerMeterNG = u32tmp;
	
		//	BMS Error Status
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;
		DevicesNG[MtrBoardIdx].BmsNG = u32tmp;
	
		//	water meter Error Status
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;
		DevicesNG[MtrBoardIdx].WaterMeterNG = u32tmp;

		//	Pyranometer Error Status
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;
		DevicesNG[MtrBoardIdx].PyranometerNG = u32tmp;

		//	Soil sensor NG Status.
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;
		DevicesNG[MtrBoardIdx].SoilSensorNG = u32tmp;

		//	Soil sensor NG Status.
    u32tmp = (uint32_t)  TokenMeter[PktIdx++] << 24;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0;
		DevicesNG[MtrBoardIdx].AirSensorNG = u32tmp;

		//	Inv Error Status
    DevicesNG[MtrBoardIdx].InvNG = TokenMeter[PktIdx++] ;  		
		
		//	Device Total Error rate
		TotErrorRate[MtrBoardIdx].PowerMeter 	= TokenMeter[PktIdx++];
		TotErrorRate[MtrBoardIdx].Bms					= TokenMeter[PktIdx++];
		TotErrorRate[MtrBoardIdx].WaterMeter  	= TokenMeter[PktIdx++];
		TotErrorRate[MtrBoardIdx].Pyranometer 	= TokenMeter[PktIdx++];
		TotErrorRate[MtrBoardIdx].SoilSensor 	= TokenMeter[PktIdx++];
		TotErrorRate[MtrBoardIdx].AirSensor  	= TokenMeter[PktIdx++];		
		TotErrorRate[MtrBoardIdx].Inverter			= TokenMeter[PktIdx++];
}


void Meter_RSP_PowerData(void)
{
    
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
		uint32_t u32tmp;
    
		DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;

    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
	
    MeterData[MtrBoardIdx][DeviceArrayIdx].ErrorRate = TokenMeter[PktIdx++];    
    MeterData[MtrBoardIdx][DeviceArrayIdx].RelayStatus = TokenMeter[PktIdx++];
	
    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].TotalWatt = u32tmp;
	
    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].V = u32tmp;

    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].I = u32tmp;
	
    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].F = u32tmp;
		
    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].P = u32tmp;

    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++];
		MeterData[MtrBoardIdx][DeviceArrayIdx].VA = u32tmp;
	
    u32tmp = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;
		MeterData[MtrBoardIdx][DeviceArrayIdx].PwrFactor = u32tmp;
}

/***	Process Meter Bms Data	***/
void Meter_RSP_BmsData(void)
{
    
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
    uint32_t u32tmp;
    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;
		
    BmsData[MtrBoardIdx][DeviceArrayIdx].ErrorRate 		 = TokenMeter[PktIdx++] ;
    BmsData[MtrBoardIdx][DeviceArrayIdx].BalanceStatus = TokenMeter[PktIdx++] ;
    BmsData[MtrBoardIdx][DeviceArrayIdx].StateOfCharge = TokenMeter[PktIdx++] ;
    BmsData[MtrBoardIdx][DeviceArrayIdx].StateOfHealth = TokenMeter[PktIdx++] ;
	
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		BmsData[MtrBoardIdx][DeviceArrayIdx].CellStatus = u32tmp;
	
		for (uint8_t i = 0; i < 16; i++) {
				BmsData[MtrBoardIdx][DeviceArrayIdx].CellVolt[i]= TokenMeter[PktIdx++] << 8 ;
				BmsData[MtrBoardIdx][DeviceArrayIdx].CellVolt[i]= TokenMeter[PktIdx++];
		}
		
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		BmsData[MtrBoardIdx][DeviceArrayIdx].BatWatt = u32tmp;
		
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		BmsData[MtrBoardIdx][DeviceArrayIdx].BatVolt = u32tmp;		
		
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		BmsData[MtrBoardIdx][DeviceArrayIdx].BatCurrent = u32tmp;

		for (uint8_t i = 0; i<5; i++)
		{		
				BmsData[MtrBoardIdx][DeviceArrayIdx].CellVolt[i]= TokenMeter[PktIdx++] << 8 ;
				BmsData[MtrBoardIdx][DeviceArrayIdx].CellVolt[i]= TokenMeter[PktIdx++];
		}		
		
		BmsData[MtrBoardIdx][DeviceArrayIdx].MosTemp = TokenMeter[PktIdx++] << 8 ;
		BmsData[MtrBoardIdx][DeviceArrayIdx].MosTemp = TokenMeter[PktIdx++];

}

/***	Process Meter WM Data	***/
void Meter_RSP_WaterData(void)
{
	
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
    uint32_t u32tmp;
    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;

		WtrMtrData[MtrBoardIdx][DeviceArrayIdx].ErrorRate = TokenMeter[PktIdx++];
		WtrMtrData[MtrBoardIdx][DeviceArrayIdx].ValveState = TokenMeter[PktIdx++];

    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		WtrMtrData[MtrBoardIdx][DeviceArrayIdx].TotalVolume = u32tmp;	
	
}

void Meter_RSP_PyranometerData(void)
{
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
    uint16_t u16temp;
    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;
	
		PyrMtrData[MtrBoardIdx][DeviceArrayIdx].ErrorRate = TokenMeter[PktIdx++];

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		PyrMtrData[MtrBoardIdx][DeviceArrayIdx].OffsetValue = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		PyrMtrData[MtrBoardIdx][DeviceArrayIdx].SolarRadiation = u16temp;
	
}
void Meter_RSP_SoilSensorData(void)
{
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
    uint16_t u16temp;
		uint32_t u32tmp;
    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;
	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].ErrorRate = TokenMeter[PktIdx++];

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Moisture = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Temperature = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].EC = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].PH = u16temp;
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Nitrogen = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Phosphorus = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Potassium = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Salinity = u16temp;			

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].TDS = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Fertility = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].EC_Coef = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Salinity_Coef = u16temp;	
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].TDS_Coef = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Temp_Calib = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Moisture_Calib = u16temp;		
	
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].EC_Calib = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].PH_Calib = u16temp;
		
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Fert_Coef = u32tmp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Fert_Deviation = u16temp;

    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Nitrogen_Coef = u32tmp;
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Nitrogen_Deviation = u16temp;

    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Phosphorus_Coef = u32tmp;
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Phosphorus_Deviation = u16temp;		
		
    u32tmp  = (uint32_t) TokenMeter[PktIdx++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[PktIdx++] << 0 ;	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Potassium_Coef = u32tmp;
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		SoilSensorData[MtrBoardIdx][DeviceArrayIdx].Potassium_Deviation = u16temp;		
		
}

void Meter_RSP_AirSensorData(void)
{
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
		uint8_t DeviceArrayIdx;
    uint16_t u16temp;
    
		MtrBoardIdx = TokenMeter[1] -1;
    DeviceArrayIdx  = TokenMeter[4];
    PktIdx = 5 ;
	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].ErrorRate = TokenMeter[PktIdx++];
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Co2 = u16temp;

	  u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Formaldehyde = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Tvoc = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Pm25 = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Pm10 = u16temp;

    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Temperature = u16temp;	
		
    u16temp = (uint16_t) TokenMeter[PktIdx++] << 8 ;
    u16temp |= (uint16_t) TokenMeter[PktIdx++];	
		AirSensorData[MtrBoardIdx][DeviceArrayIdx].Humidity = u16temp;
}

/***	Process Meter WM Data	***/
void Meter_RSP_InvData(void)
{
    uint8_t PktIdx;
		uint8_t MtrBoardIdx;
    uint16_t u16temp;

		MtrBoardIdx = TokenMeter[1] -1;
    PktIdx = 5 ;
		//	communication rate
		InvData[MtrBoardIdx].ErrorRate = TokenMeter[PktIdx++];
	
		//	inverter status flags
		InvData[MtrBoardIdx].statusByte1 = TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].statusByte3 = TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].warnByte1		= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].warnByte2 	= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].faultByte1 	= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].faultByte2	= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].faultByte3	= TokenMeter[PktIdx++];
	
		InvData[MtrBoardIdx].InputVolt 	= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].InputVolt 	|= TokenMeter[PktIdx++];
	
		InvData[MtrBoardIdx].InputFreq		= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].InputFreq 	|= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].OutputVolt 	= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].OutputVolt	|= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].OutputFreq	= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].OutputFreq 	|= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].BatVolt 		= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].BatVolt			|= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].BatCapacity = TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].InvCurrent 	= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].LoadPercentage 		= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].MachineTemp 			= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].MachineStatusCode = TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].SysStatus		= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].PV_volt 		= TokenMeter[PktIdx++] << 8;
		InvData[MtrBoardIdx].PV_volt			|= TokenMeter[PktIdx++];
		
		InvData[MtrBoardIdx].CtrlCurrent 		= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].CtrlTemp				= TokenMeter[PktIdx++];
		InvData[MtrBoardIdx].CtrlStatusCode	= TokenMeter[PktIdx++];
}

/* Get FW info From Meter

4-19 : FWststatus 	(16 bytes)
20-51: FWMetadata1 	(32 bytes)
52-83: FWMetadata2 	(32 bytes)
*/
void Meter_RSP_FWInfo(void)
{
		memcpy(&MeterMetaStatus, &TokenMeter[4], sizeof(FWstatus));
    memcpy(&MeterMetaActive, &TokenMeter[20], sizeof(FWMetadata));
		memcpy(&MeterMetaBackup, &TokenMeter[52], sizeof(FWMetadata));
}

/***
*	@brief	Send meter device Polling Cmds and Host device Cmds 
0 : 0x55
1 : NowPollingMtrBoard
2 : Meter Cmds
3 : Polling device ID
90 - 97: RTC
98 : Checksum(1-97)
99 : 0x0A
 ***/
void SendMeter_AliveToken(void)
{
	
    MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_ALIVE ;	
    MeterTxBuffer[3] = NowPollingPwrMtrID ;

		SendMeterRTC();
	
    CalChecksumM();	
}

void SendMeter_GetPowerData(void)
{
		uint8_t MtrBoardIdx;
    
    MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_POWER_METER;
    MeterTxBuffer[3] = NowPollingPwrMtrID ;
	
		MtrBoardIdx = NowPollingMtrBoard-1;
		//	Host cmd Device 
		MeterTxBuffer[4] = PwrMtrCmdList[MtrBoardIdx][NowPollingPwrMtrID-1];
 
		SendMeterRTC();

    CalChecksumM();	
}

void SendMeter_GetBmsData(void)
{
		uint8_t MtrBoardIdx;

		MeterTxBuffer[1] = NowPollingMtrBoard;
    MeterTxBuffer[2] = METER_CMD_BMS;
    MeterTxBuffer[3] = NowPollingBmsID ;
		
		MtrBoardIdx = NowPollingMtrBoard-1;	
	
		MeterTxBuffer[4] = BmsCmdList[MtrBoardIdx][NowPollingBmsID-1];
	  
		SendMeterRTC();
	
		CalChecksumM();	
}

void SendMeter_GetWaterData(void)
{
		uint8_t MtrBoardIdx;
		
		MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_WATER_METER;
    MeterTxBuffer[3] = NowPollingWtrMtrID ;		
		
		MtrBoardIdx = NowPollingMtrBoard-1;	
	
		MeterTxBuffer[4] = WtrMtrCmdList[MtrBoardIdx][NowPollingWtrMtrID-1];

		SendMeterRTC();
	
		CalChecksumM();	
}

void SendMeter_GetPyrMtrData(void)
{
		uint8_t MtrBoardIdx;
		
		MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_PYRANOMETER;
    MeterTxBuffer[3] = NowPollingPyrMtrID ;		
		
		MtrBoardIdx = NowPollingMtrBoard-1;	
	
		MeterTxBuffer[4] = PyrMtrCmdList[MtrBoardIdx][NowPollingPyrMtrID-1];

		SendMeterRTC();
	
		CalChecksumM();			
}

void SendMeter_GetSoilData(void)
{
		uint8_t MtrBoardIdx;
		
		MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_SOIL_SENSOR;
    MeterTxBuffer[3] = NowPollingSoilSensorID ;		
		
		MtrBoardIdx = NowPollingMtrBoard-1;	
	
		MeterTxBuffer[4] = SoilSensorCmdList[MtrBoardIdx][NowPollingSoilSensorID-1];

		SendMeterRTC();

		CalChecksumM();
}

void SendMeter_GetAirData(void)
{
		uint8_t MtrBoardIdx;
		
		MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_AIR_SENSOR;
    MeterTxBuffer[3] = NowPollingAirSensorID ;		
		
		MtrBoardIdx = NowPollingMtrBoard-1;	

		SendMeterRTC();

		CalChecksumM();
}


void SendMeter_GetInvData(void)
{
		uint8_t MtrBoardIdx;
	
    MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_CMD_INV;
    MeterTxBuffer[3] = NowPollingInvID ;
	
		MtrBoardIdx = NowPollingMtrBoard-1;	
	
		SendMeterRTC();
	
		CalChecksumM();	
}


/* Send Cmd to Meter
0: 0x55
1: OTA MeterID
2: Meter OTA Cmd	(0x17)
3: Sub Cmd
	0x17 : CMD_MTR_OTA_UPDATE 		 
	0x18 : CMD_MTR_SWITCH_FWVER 	
	0x19 : CMD_GET_MTR_FW_STATUS
	0x1A : CMD_MTR_FW_REBOOT
*/
void SendMeter_MeterOTACmd(uint8_t cmd)
{         		
    
    MeterTxBuffer[1] = OTAMeterID;
		MeterTxBuffer[2] = CMD_MTR_OTA_UPDATE;
		MeterTxBuffer[3] = cmd;
		
		SendMeterRTC();
		
		CalChecksumM();	
}

void SendMeter_WateringTimeSetup(void)
{         		
		uint8_t MtrBoardIdx;
		uint8_t WateringPeriod;
		uint16_t u16tmpNow, WateringFinishTime;
		
		MtrBoardIdx = NowPollingMtrBoard-1;	
		u16tmpNow = HostSystemTime[INX_HOUR] *60 | INX_MIN;
	
    MeterTxBuffer[1] = NowPollingMtrBoard;	
    MeterTxBuffer[2] = METER_GET_CMD_WATERING;	
		WateringPeriod = Watering_SetUp[MtrBoardIdx].Period_min;
	
		
		if (WateringFinishTime == 0x0000)
		{	
				WateringFinishTime = u16tmpNow | WateringPeriod;
				//	WateringOnCmd;
		} else {
				if(u16tmpNow < WateringFinishTime)
				{
						//	WateringOnCmd;
				} else {
						//	WateringOffCmd;
						WateringFinishTime = 0x0000;
						WATERING_SETTING_FLAG = FALSE;
				}
		}
		SendMeterRTC();
	
		CalChecksumM();	    
}


/*
	@brief	Send connect massage to meter bootloader do the hand shake
	0:	0x55
	1:	NowPollingMtrBoardID
	2:	MTR_CMD_CONNECT	0xAE
	3: g_packNo 0
 */
void SendMeterBootloader_ConnectCmd (void)
{
		
		MeterTxBuffer[1] = NowPollingMtrBoard;
		MeterTxBuffer[2] = MTR_CMD_CONNECT;
		//	Package number: Get from host, inital value set to 0;
		MeterTxBuffer[3] = 0;
		
		SendMeterRTC();
		CalChecksumM();	
}

/*	
	Meter OTA Process 
 */
void SendMeterBootloader_UpdateMetadata (void)
{
		
		MeterTxBuffer[1] = NowPollingMtrBoard;
		MeterTxBuffer[2] = MTR_CMD_UPDATE_METADATA;
		MeterTxBuffer[3] = g_packno;
		
		//	Send fw_version
    MeterTxBuffer[4] = (other.fw_version & 0xFF000000) >> 24 ;
    MeterTxBuffer[5] = (other.fw_version & 0x00FF0000) >> 16 ;
    MeterTxBuffer[6] = (other.fw_version & 0x0000FF00) >> 8 ;
    MeterTxBuffer[7] = (other.fw_version & 0x000000FF) >> 0 ;   
		
		//	Send fw_crc32
		MeterTxBuffer[8]  = (other.fw_crc32 & 0xFF000000) >> 24 ;
    MeterTxBuffer[9]  = (other.fw_crc32 & 0x00FF0000) >> 16 ;
    MeterTxBuffer[10] = (other.fw_crc32 & 0x0000FF00) >> 8 ;
    MeterTxBuffer[11] = (other.fw_crc32 & 0x000000FF) >> 0 ;   
		
		//	Send_fw_size
		MeterTxBuffer[12] = (other.fw_size & 0xFF000000) >> 24 ;
    MeterTxBuffer[13] = (other.fw_size & 0x00FF0000) >> 16 ;
    MeterTxBuffer[14] = (other.fw_size & 0x0000FF00) >> 8 ;
    MeterTxBuffer[15] = (other.fw_size & 0x000000FF) >> 0 ;  
	
		CalChecksumM();	
}

void SendMeterBootloader_CmdEraseAPROM (void)
{
		MeterTxBuffer[1] = NowPollingMtrBoard;
		MeterTxBuffer[2] = MTR_CMD_UPDATE_APROM;
		MeterTxBuffer[3] = g_packno;
	
		srclen = 92;
		ReadData(StartAddress, StartAddress + srclen, (uint32_t*)&MeterTxBuffer[4]);
		StartAddress += srclen;
		TotalLen -= srclen;
	
		CalChecksumM();	
	
}

void SendMeterBootloader_CmdUpdateAPROM (void)
{		
		MeterTxBuffer[1] = NowPollingMtrBoard;
		MeterTxBuffer[2] = 0x00;
		MeterTxBuffer[3] = g_packno;
	
		srclen = 92;
	
		if (TotalLen < srclen) srclen = TotalLen;
		ReadData(StartAddress, StartAddress + srclen, (uint32_t*)&MeterTxBuffer[4]);
		
		CalChecksumM();	
}

/***
 *	@brief	Process the massage comes from meter Bootloader
 *	@ sync package number.
 *	@ get bootloader respond cmd
 ***/
//void MeterOTAProccess(void)
//{		
//		lcmd = TokenMeter[2];
//		g_packno = TokenMeter[3];
//}

/***
 *	@brief	Send Real Time Clock to Meter system	
 ***/
void SendMeterRTC(void)
{
    for (uint8_t i=0;i<8;i++)
    {		
        MeterTxBuffer[INX_TIME_START_YY_H+i] = CtrSystemTime[i];
    }
}


void CalChecksumM(void)
{
	uint8_t i;
	uint8_t Checksum;

	MeterTxBuffer[0] = 0x55 ;	

	Checksum = 0 ;
	for (i=1;i<(METER_TOKEN_LENGTH-2);i++)
	{
		Checksum += MeterTxBuffer[i];
	}	
	MeterTxBuffer[METER_TOKEN_LENGTH-2] = Checksum ;
	MeterTxBuffer[METER_TOKEN_LENGTH-1] = '\n' ;
	
	_SendStringToMETER(MeterTxBuffer,METER_TOKEN_LENGTH);
}

uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len)
{
	
	uint8_t idx;

	if( (METERTxQ_cnt+len) > MAX_HOST_TXQ_LENGTH )
	{
		return 0x01 ;
	} else {		
		for(idx=0;idx<len;idx++)
		{
			METERTxQ[METERTxQ_wp]=Str[idx];
			METERTxQ_wp++;
			if(METERTxQ_wp>=MAX_HOST_TXQ_LENGTH)
			{
				METERTxQ_wp=0;
			}
			METERTxQ_cnt++;
		}        				
		UART_EnableInt(UART0, (UART_INTEN_THREIEN_Msk ));
	}                   
	return 0x00 ;
}


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
void Meter_RSP_WMData(void);
void Meter_RSP_InvData(void);

//	Meter Polling Cmds
void SendMeter_GetPowerData(void);
void SendMeter_GetBmsData(void);
void SendMeter_GetWMData(void);
void SendMeter_GetInvData(void);

void SendMeter_MeterOTACmd(uint8_t cmd);

void PollSuccess_Handler(uint8_t NextPollingState);
void PollingTimeout_Handler(uint8_t nextStateOnRetry, uint8_t nextStateOnMaxRetry);

/*** MeterBootloader ***/
void SendMeterBootloader_ConnectCmd(void);
void SendMeterBootloader_UpdateMetadata (void);
void SendMeterBootloader_CmdUpdateAPROM (void);
void SendMeterBootloader_CmdEraseAPROM (void);


BmsData_t 	BmsData[BmsDeviceMax];
WMData_t		WMData[WMDeviceMax];
InvData_t 	InvData[InvDeviceMax];
CtrlData_t 	CtrlData[InvDeviceMax];
BatData_t		BatData[InvDeviceMax];

uint8_t _SendStringToMETER(uint8_t *Str, uint8_t len);

uint8_t UserIndex1;
uint8_t ErrorCounter ;
uint8_t RoomIndexReal; 

//*** OTA Update ***//
uint8_t g_packno;
uint8_t lcmd;

uint32_t StartAddress;
uint32_t TotalLen;
uint32_t srclen;
uint8_t MeterPollingState;
_Bool fgMeterInitCompleted;
_Bool EnPollPowerMeterFlag, EnPollBmsFlag, EnPollWMFlag, EnPollInvFlag;

/***	
	@note 		When finish all devices polling process, poll the consecutive meter board
	@brief 		i.	Meter Polling States: 
							1. Power meter, 
							2. Bms, 
							3. Water meter, 
							4. Inverter, 
							5. MeterBoard OTA
						ii.	Send host Cmds to meter devices, by using subcmd.
						
	@note			Timeout logic process
	@revise 	2025.08.21
***/
void MeterBoardPolling(void)
{

		uint8_t MeterOtaCmd;

    RoomIndexReal = NowPollingMeterBoard-1;		
		//Meter Cmd List
		MeterOtaCmd = MeterOtaCmdList[OTAMeterID-1];
		
		//	I want to Poll the other board when the polling process of four devices is finished.
		if ((EnPollPowerMeterFlag == FALSE) && (EnPollBmsFlag == FALSE) && 
				(EnPollWMFlag == FALSE) && (EnPollInvFlag == FALSE))
		{
				//	Start polling over again
				NowPollingPowerMeter = 1;	
				NowPollingBms = 1;
				NowPollingWM 	= 1;
				NowPollingInv = 1;	
				//	
				EnPollPowerMeterFlag = 1;
				EnPollBmsFlag = 1;
				EnPollWMFlag 	= 1;
				EnPollInvFlag = 1;
			
				NowPollingMeterBoard++;
        if (NowPollingMeterBoard > MeterDeviceMax) {
            NowPollingMeterBoard = 1;
        }
		}

    switch ( MeterPollingState )
    {			
        case PL_METER_NORM :
            
            MeterPollingState = PL_METER_POLL1 ;            
            break;
				
        case PL_METER_POLL1 :
						if (!EnPollPowerMeterFlag){
								MeterPollingState = PL_METER_POLL3;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_AliveToken();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL2 ;
            }
            break;
						
        case PL_METER_POLL2 :
            if (MeterRspID == NowPollingMeterBoard)
            {
								PollSuccess_Handler(PL_METER_POLL3);
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL1,PL_METER_POLL3);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL3) {
												NowPollingPowerMeter++;
												if (NowPollingPowerMeter > RoomMax){
														EnPollPowerMeterFlag = FALSE;
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
            if (MeterRspID == NowPollingMeterBoard)
            {		
								PwrMeterCmdList[NowPollingPowerMeter-1] = 0;
								PollSuccess_Handler(PL_METER_POLL5);
								//	Poll next power meter, or stop polling
								NowPollingPowerMeter++;
								if (NowPollingPowerMeter > RoomMax){
										EnPollPowerMeterFlag = FALSE;
								}
						} else {
                // Timeout 
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL3, PL_METER_POLL5);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL5) {
												NowPollingPowerMeter++;
												if (NowPollingPowerMeter > RoomMax){
														EnPollPowerMeterFlag = FALSE;
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
            if (MeterRspID == NowPollingMeterBoard)
            {
								BmsCmdList[NowPollingBms-1] = 0;
								PollSuccess_Handler(PL_METER_POLL7);
								//	Poll next Bms, or stop polling
								NowPollingBms++;
								if (NowPollingBms > BmsDeviceMax){
										EnPollBmsFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL5,PL_METER_POLL7);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL7) {
												NowPollingBms++;
												if (NowPollingBms > BmsDeviceMax){
														EnPollBmsFlag = FALSE;
												}
										}
            }
            break;						
						
        case PL_METER_POLL7 :	
						if (!EnPollWMFlag){
								MeterPollingState = PL_METER_POLL9;
								return;
						}
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
                SendMeter_GetWMData();
                TickPollingInterval_Meter = 0 ;
                MeterPollingState = PL_METER_POLL8;
            }
            break;
        case PL_METER_POLL8 :
            if (MeterRspID == NowPollingMeterBoard)
            {
								WtrMeterCmdList[NowPollingWM-1] = 0;
								PollSuccess_Handler(PL_METER_POLL9);
								//	Poll next WM, or stop polling
								NowPollingWM++;
								if (NowPollingWM > WMDeviceMax){
										EnPollWMFlag = FALSE;
								}
															
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL7,PL_METER_POLL9);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL9) {
												NowPollingWM++;
												if (NowPollingWM > WMDeviceMax){
														EnPollWMFlag = FALSE;
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
            if (MeterRspID == NowPollingMeterBoard)
            {
								InvCmdList[NowPollingInv-1] = 0;
								PollSuccess_Handler(PL_METER_POLL11);
								//	Poll next Bms, or stop polling
								NowPollingInv++;
								if (NowPollingInv > InvDeviceMax){
										EnPollInvFlag = FALSE;
								}
            } else {
                if ( TickPollingInterval_Meter > POLLING_TIMEOUT )
										PollingTimeout_Handler(PL_METER_POLL9,PL_METER_POLL11);
										//	when reaches maximum timeout tries, poll next device
										if (MeterPollingState == PL_METER_POLL11) {
												NowPollingInv++;
												if (NowPollingInv > InvDeviceMax){
														EnPollInvFlag = FALSE;
												}
										}
            }
            break;		
				/***
						Process Meter Cmds, if Cmds != CMD_MTR_OTA_UPDATE, poll the next meter
						if Cmd == CMD_MTR_OTA_UPDATE
				 ***/
        case PL_METER_POLL11 :				
            if ( MeterWaitTick > 2 )
            {							
                MeterRspID = 0xFF ;
                ResetMeterUART();
								if (MeterOtaCmd != 0){
										SendMeter_MeterOTACmd(MeterOtaCmd);
										TickPollingInterval_Meter = 0 ;
										MeterPollingState = PL_METER_POLL12 ;
								} else {
										MeterPollingState = PL_METER_NORM;
								}

            }
            break;
				case PL_METER_POLL12:
						if (MeterRspID == NowPollingMeterBoard)
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
								PollingTimeout_Handler(PL_METER_POLL11, PL_METER_NORM);
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
						if (MeterRspID == NowPollingMeterBoard)
						{
								RoomIndexReal = NowPollingMeterBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << RoomIndexReal);
							
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
						if (MeterRspID == NowPollingMeterBoard)
						{
								RoomIndexReal = NowPollingMeterBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << RoomIndexReal);
							
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
						if (MeterRspID == NowPollingMeterBoard)
						{
								RoomIndexReal = NowPollingMeterBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << RoomIndexReal);
							
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
						if (MeterRspID == NowPollingMeterBoard)
						{
								RoomIndexReal = NowPollingMeterBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << RoomIndexReal);
							
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
						
						if (MeterRspID == NowPollingMeterBoard)
						{
								RoomIndexReal = NowPollingMeterBoard - 1;
								PollRetryCounter_Meter = 0;
								MeterWaitTick = 0;
								ErrorCounter = 0;
								MeterDeviceError &= ~(0x01 << RoomIndexReal);
							
								//	Meter Send Host 0x0ABBC0DD When Boot
								if (TokenMeter[4] == 0x0A &&
										TokenMeter[5] == 0xBB &&
										TokenMeter[6] == 0xC0 &&
										TokenMeter[7] == 0xDD)
								{
										NowPollingPowerMeter++;
									
										if (NowPollingPowerMeter > RoomMax )
												NowPollingPowerMeter = 1 ;
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
    RoomIndexReal = NowPollingMeterBoard - 1;
    MeterWaitTick = 0;
    PollRetryCounter_Meter++;
	
    if (PollRetryCounter_Meter > ERROR_RETRY_MAX) 
		{
        PollRetryCounter_Meter = 0;
        MeterDeviceError |= (0x01 << RoomIndexReal);
			
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
    RoomIndexReal = NowPollingMeterBoard - 1;
    PollRetryCounter_Meter = 0;
    MeterWaitTick = 0;
    ErrorCounter = 0;
    MeterDeviceError &= (~(0x01 << RoomIndexReal));
		MeterPollingState = NextPollingState;
}

/*********************************
 * Define : Host Token 
 * byte 0 : 0x55
 * byte 1 : Node ID
 * byte 2 : System Flag 
 * byte 3 : Command Type
 * byte 4 ~ 22  : Data 0 ~ 
 * byte 23 : Checksum 
 * byte 24 : 0x0A   
  *********************************/

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
									
									case METER_RSP_WM_DATA:
										Meter_RSP_WMData();
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
    uint8_t fnPacketIndex;
    uint32_t u32tmp;
     

    fnPacketIndex  = 5 ;
    u32tmp = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;    
    PowerMeterError = u32tmp;
	
}

void Meter_RSP_SysInformation(void)
{    
    uint8_t fnPacketIndex;
    uint32_t u32tmp;
    
    fnPacketIndex  = 5 ;
    u32tmp = (uint32_t)  TokenMeter[fnPacketIndex++] << 24;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8;
    u32tmp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0;  
    PowerMeterError = u32tmp;
	

	
		
	
}

uint8_t DeviceIndexReal;

void Meter_RSP_PowerData(void)
{
    
    uint8_t fnPacketIndex;
    uint32_t u32temp;
    DeviceIndexReal  = TokenMeter[4];
    fnPacketIndex = 5 ;
	
    MeterData[DeviceIndexReal-1].ErrorRate = TokenMeter[5];    
    MeterData[DeviceIndexReal-1].RelayStatus = TokenMeter[6];
    	
    // Total 
    u32temp = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;

}

/***	Process Meter Bms Data	***/
void Meter_RSP_BmsData(void)
{
    
    uint8_t fnPacketIndex;
    uint32_t u32temp;
    
    DeviceIndexReal  = TokenMeter[4];
    fnPacketIndex = 5 ;
    
    BmsData[DeviceIndexReal].ErrorRate 		 = TokenMeter[fnPacketIndex++] ;
    BmsData[DeviceIndexReal].BalanceStatus = TokenMeter[fnPacketIndex++] ;
    BmsData[DeviceIndexReal].StateOfCharge = TokenMeter[fnPacketIndex++] ;
    BmsData[DeviceIndexReal].StateOfHealth = TokenMeter[fnPacketIndex++] ;
	
    u32temp  = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;	
		BmsData[DeviceIndexReal].CellStatus = u32temp;
	
		for (uint8_t i = 0; i < 16; i++) {
				BmsData[DeviceIndexReal].CellVolt[i]= TokenMeter[fnPacketIndex++] << 8 ;
				BmsData[DeviceIndexReal].CellVolt[i]= TokenMeter[fnPacketIndex++];
		}
		
    u32temp  = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;	
		BmsData[DeviceIndexReal].BatWatt = u32temp;
		
    u32temp  = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;	
		BmsData[DeviceIndexReal].BatVolt = u32temp;		
		
    u32temp  = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;	
		BmsData[DeviceIndexReal].BatCurrent = u32temp;

		for (uint8_t i = 0; i<5; i++)
		{		
				BmsData[DeviceIndexReal].CellVolt[i]= TokenMeter[fnPacketIndex++] << 8 ;
				BmsData[DeviceIndexReal].CellVolt[i]= TokenMeter[fnPacketIndex++];
		}		
		
		BmsData[DeviceIndexReal].MosTemp = TokenMeter[fnPacketIndex++] << 8 ;
		BmsData[DeviceIndexReal].MosTemp = TokenMeter[fnPacketIndex++];

}

/***	Process Meter WM Data	***/
void Meter_RSP_WMData(void)
{
	
	  uint8_t fnPacketIndex;
    uint32_t u32temp;
    
    DeviceIndexReal  = TokenMeter[4];
    fnPacketIndex = 5 ;
		WMData[DeviceIndexReal].ErrorRate = TokenMeter[fnPacketIndex++];
		WMData[DeviceIndexReal].ValveState = TokenMeter[fnPacketIndex++];

    u32temp  = (uint32_t) TokenMeter[fnPacketIndex++] << 24 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 16 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 8 ;
    u32temp |= (uint32_t) TokenMeter[fnPacketIndex++] << 0 ;	
		WMData[DeviceIndexReal].TotalVolume = u32temp;	
	
}

/***	Process Meter WM Data	***/
void Meter_RSP_InvData(void)
{
	
	  uint8_t fnPacketIndex;
    uint32_t u32temp;
    
    DeviceIndexReal  = TokenMeter[4];
    fnPacketIndex = 5 ;		
		//	communication rate
		InvData[DeviceIndexReal].ErrorRate = TokenMeter[fnPacketIndex++];
	
		// 	inverter controller status flags
		CtrlData[DeviceIndexReal].ConnectFlag 	= TokenMeter[fnPacketIndex++];
		CtrlData[DeviceIndexReal].ChargingFlag 	= TokenMeter[fnPacketIndex++];
		CtrlData[DeviceIndexReal].FaultFlag 		= TokenMeter[fnPacketIndex++];
		CtrlData[DeviceIndexReal].WarnFlag 			= TokenMeter[fnPacketIndex++];
	
		//	inverter status flags
		InvData[DeviceIndexReal].ChargingFlag 	= TokenMeter[fnPacketIndex++];
		InvData[DeviceIndexReal].FaultFlag 			= TokenMeter[fnPacketIndex++];
		InvData[DeviceIndexReal].WarnFlag			 	= TokenMeter[fnPacketIndex++];

		//	inverter battery status flags
		BatData[DeviceIndexReal].Full									= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].LoadWarnFlag					= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].TempWarnFlag				 	= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].LoadTimeoutWarnFlag 	= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].LoadOverWarnFlag			= TokenMeter[fnPacketIndex++];	
		BatData[DeviceIndexReal].BatHighVoltWarnFlag	= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].BatLowVoltWarnFlag		= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].StoreDataErrWarnFlag = TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].StoreOpFailWarnFlag	= TokenMeter[fnPacketIndex++];		

		BatData[DeviceIndexReal].InvFuncErrWarnFlag		= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].PlanShutdownWarnFlag = TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].OutputWarnFlag				= TokenMeter[fnPacketIndex++];	
		
		BatData[DeviceIndexReal].InvErrFaultFlag				= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].TempOverFaultFlag			= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].TempSensorFaultFlag		= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].LoadTimeoutFaultFlag 	= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].LoadErrFaultFlag				= TokenMeter[fnPacketIndex++];	
		BatData[DeviceIndexReal].LoadOverFaultFlag			= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].BatHighVoltFaultFlag		= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].BatLowVoltFaultFlag 		= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].PlanShutdownFaultFlag	= TokenMeter[fnPacketIndex++];		
		BatData[DeviceIndexReal].OutputErrFaultFlag			= TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].ChipStartFailFaultFlag = TokenMeter[fnPacketIndex++];
		BatData[DeviceIndexReal].CurrentSensorFaultFlag	= TokenMeter[fnPacketIndex++];			
		
}

/* Get FW info From Meter

4-19 : FWststatus 	(16 bytes)
20-51: FWMetadata1 	(32 bytes)
52-83: FWMetadata2 	(32 bytes)
*/
//void Meter_RSP_FWInfo(void)
//{
//		memcpy(&MeterMetaStatus, &TokenMeter[4], sizeof(FWstatus));
//    memcpy(&MeterMetaActive, &TokenMeter[20], sizeof(FWMetadata));
//		memcpy(&MeterMetaBackup, &TokenMeter[52], sizeof(FWMetadata));
//}

/************************************
0: 0x55
1: Meter ID
2: Command
3: fgToMeterFlag
4: fgMeterRSPFlag
5: Member Index
6: Room Mode
7: Room Unit Price
8: Add/Dec Balance Value(H)
9: Add/Dec Balance Value(L)
10: Open Room Door Double Check Value
11~14: User UID (0) ~ (3)
15: User Mode 
16: Message Type
17: Member Counter
18~18+n: Member1~n Data Checksum
41~47: System Time
48: Checksum
49: 0x0A (\n)
*/
void SendMeter_AliveToken(void)
{
    uint8_t i,u8PackageIndex;

    for (i=0;i<7;i++)
    {		
        MeterTxBuffer[INX_TIME_START_Y+i] = iSystemTime[i];
    }	         		
    //NowPollingMeterBoard = 1 ;
    
    MeterTxBuffer[1] = NowPollingMeterBoard;	
    MeterTxBuffer[2] = METER_CMD_ALIVE ;	
    MeterTxBuffer[3] = NowPollingPowerMeter ;
    //RoomIndexReal = NowPollingPowerMeter - 1 ;
    MeterTxBuffer[5] =  RoomMax;  
    MeterTxBuffer[6] =  (255-RoomMax) ;        
    MeterTxBuffer[7] =  0x30 ;  
    MeterTxBuffer[8] =  (255-0x30) ;
    
    u8PackageIndex = 9 ;

    CalChecksumM();	
	
}



void SendMeter_GetPowerData(void)
{
        
    MeterTxBuffer[1] = NowPollingMeterBoard;	
    MeterTxBuffer[2] = METER_CMD_POWER_METER;
    MeterTxBuffer[3] = NowPollingPowerMeter ;
		//	Host cmd Device 
		MeterTxBuffer[4] = PwrMeterCmdList[NowPollingPowerMeter-1];
 
//    for (i=0;i<7;i++)
//    {
//        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
//    }  

    CalChecksumM();	
}

void SendMeter_GetBmsData(void)
{
    MeterTxBuffer[1] = NowPollingMeterBoard;
    MeterTxBuffer[2] = METER_CMD_BMS;
    MeterTxBuffer[3] = NowPollingBms ;
		MeterTxBuffer[4] = BmsCmdList[NowPollingBms-1];
	    
		CalChecksumM();	
}

void SendMeter_GetWMData(void)
{
    MeterTxBuffer[1] = NowPollingMeterBoard;	
    MeterTxBuffer[2] = METER_CMD_WATER_METER;
    MeterTxBuffer[3] = NowPollingWM ;		
		MeterTxBuffer[4] = WtrMeterCmdList[NowPollingWM-1];
	
		CalChecksumM();	
}

void SendMeter_GetInvData(void)
{
    MeterTxBuffer[1] = NowPollingMeterBoard;	
    MeterTxBuffer[2] = METER_CMD_INV;
    MeterTxBuffer[3] = NowPollingInv ;		
		MeterTxBuffer[4] = InvCmdList[NowPollingInv-1];
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
		uint8_t i;
    for (i=0;i<7;i++)
    {		
        MeterTxBuffer[INX_TIME_START_Y+i] = iSystemTime[i];
    }	         		
    
    MeterTxBuffer[1] = OTAMeterID;
		MeterTxBuffer[2] = CMD_MTR_OTA_UPDATE;
		MeterTxBuffer[3] = cmd;
		
		CalChecksumM();	
}

/*
	@brief	Send connect massage to meter bootloader do the hand shake
	0:	0x55
	1:	NowPollingMeterBoardID
	2:	MTR_CMD_CONNECT	0xAE
	3: g_packNo 0
 */
void SendMeterBootloader_ConnectCmd (void)
{
		
		MeterTxBuffer[1] = NowPollingMeterBoard;
		MeterTxBuffer[2] = MTR_CMD_CONNECT;
		//	Package number: Get from host, inital value set to 0;
		MeterTxBuffer[3] = 0;
		
		CalChecksumM();	
}

/*	
	Meter OTA Process 
 */
void SendMeterBootloader_UpdateMetadata (void)
{
		
		MeterTxBuffer[1] = NowPollingMeterBoard;
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
		MeterTxBuffer[1] = NowPollingMeterBoard;
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
		MeterTxBuffer[1] = NowPollingMeterBoard;
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


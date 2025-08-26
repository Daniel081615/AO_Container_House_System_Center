/*--------------------------------------------------------------------------- 
 * File Name     : AO_HostProcess.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * --------------------------------------------------------------------------*/
#include <stdio.h>
#include "NUC1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_MeterProcess.h"
#include "AO_EE24C.h"
#include "AO_HostProcess.h"

uint8_t CmdType,WaitTime;
_Bool bReSendPowerEnable;
uint8_t LastMessDate[3];

// CTR_SYS_SW ­p®É Reset
uint8_t system_SW_Flag = 0;
uint8_t system_SW_Timer = 0;



void HostProcess(void);
void CalChecksumH(void);
void ClearRespDelayTimer(void);
void HOST_AliveProcess(void);

void SendHost_Ack(void);
void SystemSwitchProcess(void);


void SendHost_SystemInformation(void);
void SendHost_PowerData(void);
void SendHost_BmsData(void);
void SendHost_WMData(void);
void SendHost_InvData(void);

void Host_PowerMeterDataProcess(void);
void Host_BmsDataProcess(void);
void Host_WMDataProcess(void);
void Host_InvDataProcess(void);

//***** OTA Process *****//
void Host_OTAMeterProcess(void);
void Host_OTACenterProcess(void);

//	Read / Store MeterCmdList
void ReadMeterOtaCmdList();
void WriteMeterOtaCmdList(void);

void HOST_GetUserData(void);
void SendHost_FirstReset_Ack(void);

uint8_t AckResult;
uint8_t HostMeterIndex;
uint8_t fgTestInitOK ;
uint8_t SystemSetting;
uint8_t MeterDataUpdated;

/*********************************
 * Define : Host HostToken 
 * byte 0 : 0x55 
 * byte 1 : MyCenterID
 * byte 2 : Command Type
 * byte 3 : System Flag 
 * byte 4 ~ 48  : Data 0 ~ 45
 * byte 49 : Checksum 
 * byte 50 : 0x0A   
  *********************************/
void HostProcess(void)
{
		uint8_t i,checksum;

    if ( TickHost == 49 )
    {
        TickHost++;
        ResetHostUART();	
    }
    if( HostTokenReady )		//Host IRQ Data ready
    {
				HostTokenReady = 0 ;
				checksum = 0 ;

        for(i=1;i<(HOST_TOKEN_LENGTH-2);i++)
        {
            checksum += TokenHost[i];
        }
				
        if ( (TokenHost[0] == 0x55) && (TokenHost[HOST_TOKEN_LENGTH-2] == checksum) )
        {
						
            // System Time updated 
            if ( TokenHost[2] == CTR_ALIVE)
            {
                for (i=0;i<7;i++)
                {		
                    iSystemTime[i] = TokenHost[HOST_INX_TIME_START+i];
                }
                SystemSetting |= FLAG_SYSTEM_TIME_READY ;
            }
            CenterID = TokenHost[1] ;
            if ( (CenterID == MyCenterID) || (CenterID == 0xFF)  )
            {
                if(centerResetStatus)
                {
                    centerResetStatus = 0;			
                    SendHost_FirstReset_Ack();
                    return;
                }		
                
                // Token ready
                LED_TGL_R();
                switch( TokenHost[2] )
                {
                    case CTR_ALIVE :						
                        HOST_AliveProcess();
                        CmdType = CTR_RSP_SYSTEM_INFO ;		// 0x31
                        ClearRespDelayTimer() ;	
                        break;					                                                                                
                    case CTR_GET_CMD_POWER_METER :				// 0x11
                        Host_PowerMeterDataProcess();
                        CmdType = CTR_RSP_POWER_DATA ;
                        ClearRespDelayTimer() ;
                        break;	
										case CTR_GET_CMD_BMS:									// 0x12
												Host_BmsDataProcess();
                        CmdType = CTR_RSP_BMS_DATA ;
                        ClearRespDelayTimer() ;											
												break;
										case CTR_GET_CMD_WATER_METER:					// 0x13
												Host_WMDataProcess();
                        CmdType = CTR_RSP_WM_DATA ;
                        ClearRespDelayTimer() ;
												break;
										case CTR_GET_CMD_INV:									// 0x14
												Host_InvDataProcess();
                        CmdType = CTR_RSP_INV_DATA ;
                        ClearRespDelayTimer() ;												
												break;
										 	// CTR	ota commamd
                    case CTR_OTA_UPDATE_CTR:							// 0x15
                        Host_OTACenterProcess();
                        ClearRespDelayTimer();
                        break;
											// MTR	ota command
										case CTR_OTA_UPDATE_MTR:							// 0x16
												Host_OTAMeterProcess();
												break;
										
                    default:
                        break;
                }
            } else {

            }
			
        } else {
            ResetHostUART();
        }
			}
//		}

    if ( bDelaySendHostCMD )
    {
        if ( iTickDelaySendHostCMD > WaitTime)
        {
            bDelaySendHostCMD = 0 ;
            if ( CenterID == MyCenterID )
            {
                HostTxBuffer[1] = MyCenterID ;
                switch (CmdType)
                {
                    case CTR_RSP_SYSTEM_INFO :
                        SendHost_SystemInformation();
                        break;					
                    case CTR_RSP_ACK :
                        SendHost_Ack();
                        break;		                    
                    case CTR_RSP_POWER_DATA :
                        SendHost_PowerData();
                        break;
                    case CTR_RSP_BMS_DATA :
                        SendHost_BmsData();
                        break;
                    case CTR_RSP_WM_DATA :
                        SendHost_WMData();
                        break;
                    case CTR_RSP_INV_DATA :
                        SendHost_InvData();
                        break;										
                    default :
                        break;
                }
            }
        }
    }	
}

void ClearRespDelayTimer(void)
{
	bDelaySendHostCMD = 1 ;
	iTickDelaySendHostCMD = 0 ;	
	WaitTime= 5;
}


/*
0: 0x55
1: MyCenterID
2: Command
3: fgHostInformation
4: MeterIndex
5: Member Counter 
6: Package Index n

Member 1 - mode (1)
Member 1 - SID (4)
Member 1 - UID (4)
Member 1 - Balance (4)
Member 1 - PWD (8)
Member 2 - mode (1)
Member 2 - SID (4)
Member 2 - UID (4)
Member 2 - Balance (4)
Member 2 - PWD (8)

Checksum
0x0A (\n)
*/
void HOST_AliveProcess(void)
{	
    uint8_t i;	
    uint8_t HostData_P,HostData_N,u8BuferIndex,u8tmp;
    //uint8_t u8RoomIndex,CmdOpenDoor1,CmdOpenDoor2;
    

    for (i=0;i<7;i++)
    {		
        iSystemTime[i] = TokenHost[HOST_INX_TIME_START+i];
    }	
}

/************************************
0: 0x55
1: MyCenterID
2: Command
3: fgHostInformation
4: Meter Cmds


49: Checksum
50: 0x0A (\n)
*/
void Host_PowerMeterDataProcess(void)
{
    uint8_t i;

		
    HostDeviceIndex = TokenHost[3];
		PwrMeterCmdList[HostDeviceIndex-1] = TokenHost[4];
 
//    for (i=0;i<7;i++)
//    {
//        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
//    }  
}

void Host_BmsDataProcess(void)
{
    uint8_t i;
    
    HostDeviceIndex = TokenHost[3];
		BmsCmdList[HostDeviceIndex-1] = TokenHost[4];
 
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }  
}

void Host_WMDataProcess(void)
{
    uint8_t i;
    
    HostDeviceIndex = TokenHost[3];
		WtrMeterCmdList[HostDeviceIndex-1] = TokenHost[4];
 
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }  
}

void Host_InvDataProcess(void)
{
    uint8_t i;
    
    HostDeviceIndex = TokenHost[3];
		InvCmdList[HostDeviceIndex-1] = TokenHost[4];
 
    for (i=0;i<7;i++)
    {
        iSystemTime[i] = TokenHost[INX_TIME_START_Y+i];
    }  
}

/* Process Cenetr OTA Cmd from Host
0: 0x55
1: MyCenterID
2: Center OTA Command (0x13)
3: Sub command
4: Meter Index 
98: Checksum
99: 0x0A
*/
void Host_OTACenterProcess(void)
{
    uint8_t i;
		SYS_UnlockReg();
		FMC_Open();
	
		for (i = 0; i < 7; i++) {
        iSystemTime[i] = TokenHost[HOST_INX_TIME_START + i];
    }
		switch(TokenHost[3])
    {
			
        case CMD_GET_CTR_FW_STATUS:     // 0x42
            SendHost_CenterFWinfo();
            break;

        case CMD_CTR_SWITCH_FWVER:         // 0x41
            WRITE_FW_STATUS_FLAG(SWITCH_FW_FLAG);
            SendHost_CenterFWinfo();
            MarkFwAsActive(FALSE);
            JumpToBootloader();
            break;

        case CMD_CTR_OTA_UPDATE:        // 0x40
            WRITE_FW_STATUS_FLAG(OTA_UPDATE_FLAG);
            SendHost_CenterFWinfo();
            JumpToBootloader();
            break;

        case CMD_CTR_FW_REBOOT:         // 0x43 test
            WRITE_FW_STATUS_FLAG(REBOOT_FW_FLAG);
            SendHost_CenterFWinfo();
            JumpToBootloader();
            break;

        default:
            break;
		SYS_LockReg();
		}
}

/*** Process & store Meter OTA Cmd form Host
	*	@brief	Check if Meter fw in another bank, if not, download it
0: 0x55
1: MyCenterID
2: Meter OTA Command (0x14)
3: Sub command
4: MeterID
98: Checksum
99: 0x0A
***/
void Host_OTAMeterProcess(void){
		uint8_t i;
		SYS_UnlockReg();
		FMC_Open();
	
		for (i = 0; i < 7; i++) {
        iSystemTime[i] = TokenHost[HOST_INX_TIME_START + i];
    }
		
		MeterOtaCmdList[OTAMeterID-1] = TokenHost[3];		
		OTAMeterID = TokenHost[4];
		
		if (TokenHost[3] == CMD_MTR_OTA_UPDATE)
		{
				if (other.flags != THIS_IS_METER_FW_FLAG) {
						//	Download Meter FW in Center Bank
						WRITE_FW_STATUS_FLAG(OTA_UPDATE_FLAG);
						//	Store Ota Cmd in Flash
						WriteMeterOtaCmdList();
						SendHost_CenterFWinfo();
					
						JumpToBootloader();
				}
		}  
}


void SendHost_SystemInformation(void)
{
    uint8_t i;
    
    HostTxBuffer[2] = CTR_RSP_SYSTEM_INFO ; 
    HostTxBuffer[3] = (PowerMeterError & 0xFF000000) >> 24 ;
    HostTxBuffer[4] = (PowerMeterError & 0x00FF0000) >> 16 ;
    HostTxBuffer[5] = (PowerMeterError & 0x0000FF00) >> 8 ;
    HostTxBuffer[6] = (PowerMeterError & 0x000000FF) >> 0 ;   
    HostTxBuffer[7] = (MeterDeviceError & 0x000000FF)  ;
    HostTxBuffer[8] = NowPollingMeterBoard ;
    for ( i=0 ; i<7;i++)
    {
        //HostTxBuffer[fnPacketIndex++] = CenterRecord[tmpRecordIndex].Status  ;
    }

    	
    CalChecksumH();	
	
}
void SendHost_PowerData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnPowerMeterIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_POWER_DATA ;
    HostTxBuffer[3] = HostMeterIndex+1 ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
    fnPowerMeterIndex = HostDeviceIndex-1 ; 
    fnPacketIndex = 5 ; 
    // Byte 5.
    HostTxBuffer[fnPacketIndex++] = MeterData[fnPowerMeterIndex].ErrorRate;        
    HostTxBuffer[fnPacketIndex++] = MeterData[fnPowerMeterIndex].RelayStatus;

		HostTxBuffer[fnPacketIndex++] = (MeterData[fnPowerMeterIndex].TotalWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (MeterData[fnPowerMeterIndex].TotalWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (MeterData[fnPowerMeterIndex].TotalWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  MeterData[fnPowerMeterIndex].TotalWatt & 0x000000FF;	
     
    CalChecksumH();		
}

void SendHost_BmsData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnBmsIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_BMS_DATA ;
    HostTxBuffer[3] = HostMeterIndex+1 ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
    fnBmsIndex = HostDeviceIndex-1 ; 
    fnPacketIndex = 5 ; 
	
		HostTxBuffer[fnPacketIndex++] = BmsData[fnBmsIndex].ErrorRate;			// Communicate rate
		HostTxBuffer[fnPacketIndex++] = BmsData[fnBmsIndex].BalanceStatus;	// Battery mode
		HostTxBuffer[fnPacketIndex++] = BmsData[fnBmsIndex].StateOfCharge;
		HostTxBuffer[fnPacketIndex++] = BmsData[fnBmsIndex].StateOfHealth;	//	SOH
		//idx:9	Cell ststus 
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].CellStatus & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].CellStatus & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].CellStatus & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].CellStatus & 0x000000FF;	
		//idx:13	Cell volt
		for (uint8_t i = 0; i < 16; i++) {
				HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].CellVolt[i] >> 8);
				HostTxBuffer[fnPacketIndex++] = BmsData[fnBmsIndex].CellVolt[i];
		}
		//idx:45	Battery watt 
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].BatWatt & 0x000000FF;	
		//idx:49	Battery voltage 
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatVolt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatVolt & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatVolt & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].BatVolt & 0x000000FF;	
		//idx:53	Battery current 
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatCurrent & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatCurrent & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatCurrent & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].BatCurrent & 0x000000FF;			
		//idx:63	Battery temperature [1-5]	
		for (uint8_t i = 0; i<5; i++)
		{		
				HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].BatteryTemp[i] >> 8);
				HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].BatteryTemp[i];
		}
		//idx:65 	Mos temperature 
		HostTxBuffer[fnPacketIndex++] = (BmsData[fnBmsIndex].MosTemp >> 8);
		HostTxBuffer[fnPacketIndex++] =  BmsData[fnBmsIndex].MosTemp;
		
		CalChecksumH();			
}

void SendHost_WMData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnWMIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_WM_DATA ;
    HostTxBuffer[3] = HostMeterIndex+1 ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
    fnWMIndex = HostDeviceIndex-1 ; 
    fnPacketIndex = 5 ; 
	
		HostTxBuffer[fnPacketIndex++] = WMData[fnWMIndex].ErrorRate;			// Communicate rate
		HostTxBuffer[fnPacketIndex++] = WMData[fnWMIndex].ValveState;			// 0xff : closed, 0x00 : opened
	
		HostTxBuffer[fnPacketIndex++] = (WMData[fnWMIndex].TotalVolume & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (WMData[fnWMIndex].TotalVolume & 0xFF000000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (WMData[fnWMIndex].TotalVolume & 0xFF000000) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  WMData[fnWMIndex].TotalVolume & 0xFF000000;		
    
		CalChecksumH();			
}

void SendHost_InvData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnInvIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_BMS_DATA ;
    HostTxBuffer[3] = HostMeterIndex+1 ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
    fnInvIndex = HostDeviceIndex-1 ; 
    fnPacketIndex = 5 ; 
	
		HostTxBuffer[fnPacketIndex++] = InvData[fnInvIndex].ErrorRate;			// Communicate rate
		//	Controller Data
		HostTxBuffer[fnPacketIndex++] = CtrlData[fnInvIndex].ConnectFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData[fnInvIndex].ChargingFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData[fnInvIndex].FaultFlag;
		HostTxBuffer[fnPacketIndex++] = CtrlData[fnInvIndex].WarnFlag;
		//	Inverter Data
		HostTxBuffer[fnPacketIndex++] = InvData[fnInvIndex].ChargingFlag;	
		HostTxBuffer[fnPacketIndex++] = InvData[fnInvIndex].FaultFlag;
		HostTxBuffer[fnPacketIndex++] = InvData[fnInvIndex].WarnFlag;	
		//	Bat Data
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].Full;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].TempWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadTimeoutWarnFlag;	
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadOverWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].BatHighVoltWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].BatLowVoltWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].StoreDataErrWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].StoreOpFailWarnFlag;
		
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].InvFuncErrWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].PlanShutdownWarnFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].OutputWarnFlag;	
		
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].InvErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].TempOverFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].TempSensorFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadTimeoutFaultFlag;		
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].LoadOverFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].BatHighVoltFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].BatLowVoltFaultFlag;	
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].PlanShutdownFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].OutputErrFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].ChipStartFailFaultFlag;
		HostTxBuffer[fnPacketIndex++] = BatData[fnInvIndex].CurrentSensorFaultFlag;
		
		CalChecksumH();			
}


void SendHost_Ack(void)
{
    uint8_t i,fnPkgIndex;   

    HostTxBuffer[2] = CTR_RSP_ACK ;
    
    fnPkgIndex = 4;
    
    CalChecksumH();		

}

void SendHost_FirstReset_Ack(void)
{
    uint8_t i,fnPkgIndex;
    //uint8_t *tmpAddr;

    HostTxBuffer[2] = CTR_FIRST_RESET_STATUS ;      //0x2C
    fnPkgIndex = 4;

    
    CalChecksumH();
}

/* Send Center FW info to Host 
0:0x55
1: MyCenterID
2: Command
4-19 : FWststatus 	(16 bytes)
20-51: FWMetadata1 	(32 bytes)
52-83: FWMetadata2 	(32 bytes)
*/
//void SendHost_CenterFWinfo(void)
//{
//		uint8_t i;
//		HostTxBuffer[2] = CmdType;

//		memcpy(&HostTxBuffer[4], &g_fw_ctx, sizeof(FWstatus));
//    memcpy(&HostTxBuffer[20], &meta, sizeof(FWMetadata));
//		memcpy(&HostTxBuffer[52], &other, sizeof(FWMetadata));

//		CalChecksumH();
//}


/* Send meter fw info to host 
0:0x55
1: MyCenterID
2: Command
4-19 : FWststatus 	(16 bytes)
20-51: FWMetadata1 	(32 bytes)
52-83: FWMetadata2 	(32 bytes)
*/
//void SendHost_MeterFWInfo(void){
//	
//		uint8_t i;
//		HostTxBuffer[2] = CmdType;

//		memcpy(&HostTxBuffer[4], &MeterMetaStatus, sizeof(FWstatus));
//    memcpy(&HostTxBuffer[20], &MeterMetaActive, sizeof(FWMetadata));
//		memcpy(&HostTxBuffer[52], &MeterMetaBackup, sizeof(FWMetadata));
//	
//		CalChecksumH();
//	
//}

/* When meter update success, send host Update Success Password
0:0x55
1: MyCenterID
2: MeterRspID
4: 0x0A
5: 0xBB
6: 0xC0
7: 0xDD
*/
void SendHost_MeterUpdateSuccsess(void)
{
		HostTxBuffer[1] = MyCenterID;					//MeterID
		HostTxBuffer[2] = MeterRspID;	//MeterID
		HostTxBuffer[4] = 0x0A;		HostTxBuffer[5] = 0xBB;
		HostTxBuffer[6] = 0xC0;		HostTxBuffer[7] = 0xDD;
		
		memcpy(&HostTxBuffer[8], &MeterMetaStatus, sizeof(FWstatus));
		CalChecksumH();
}

/* When Center update success, send host Update Success Password 
0:0x55
1: MyCenterID
4: 0xEE
5: 0xFF
6: 0x5A
7: 0xA5
*/
//void SendHost_CenterUpdateSuccsess(void)
//{
//		HostTxBuffer[1] = MyCenterID;
//		HostTxBuffer[4] = 0xEE;		HostTxBuffer[5] = 0xFF;
//		HostTxBuffer[6] = 0x5A;		HostTxBuffer[7] = 0xA5;
//		
//		memcpy(&HostTxBuffer[8], &meta, sizeof(FWstatus));
//		CalChecksumH();
//}

/***
 *	@breif	Read Meter Ota List from flash, to make sure update meter
 ***/

void ReadMeterOtaCmdList()
{
		for (uint8_t i; i> ROOM_MAX; i++)
		{
				MeterOtaCmdList[i] = FMC_Read(FW_Status_Base + (i+1) * 4);
				//FMC_Erase
				//FMC_Erase_User();
		}
}


/***
 *	@breif	Write Meter Ota List to flash, to make sure Ota Cmds stored
 ***/
void WriteMeterOtaCmdList(void)
{
		SYS_UnlockReg();
		FMC_ENABLE_ISP();
		uint32_t MetaStatus[sizeof(FWstatus)/4];
		
		//	Read FwStatus
		for (uint8_t i; i < 4; i++){
				MetaStatus[i] = FMC_Read(FW_Status_Base + (i*4));
		}
		ReadData(FW_Status_Base, FW_Status_Base + sizeof(FWstatus), (uint32_t*)MetaStatus);
		
		FMC_Erase(FW_Status_Base);
		
		WriteData(FW_Status_Base, FW_Status_Base + sizeof(FWstatus), (uint32_t*)MetaStatus);
		WriteData(FW_Status_Base + sizeof(FWstatus),FW_Status_Base + sizeof(FWstatus) + sizeof(MeterOtaCmdList) , (uint32_t*)MeterOtaCmdList);
		
		SYS_LockReg();
}


void CalChecksumH(void)
{
	uint8_t i;
	uint8_t Checksum;
	DIR_HOST_RS485_Out();
  HostTxBuffer[0] = 0x55 ;
	HostTxBuffer[1] = MyCenterID ;
	Checksum = 0 ;
	for (i=1;i<(HOST_TOKEN_LENGTH-2);i++)
	{
		Checksum += HostTxBuffer[i];
	}	
	HostTxBuffer[HOST_TOKEN_LENGTH-2] = Checksum ;
	HostTxBuffer[HOST_TOKEN_LENGTH-1] = '\n' ;  
	_SendStringToHOST(HostTxBuffer,HOST_TOKEN_LENGTH);	
}

uint8_t _SendStringToHOST(uint8_t *Str, uint8_t len)
{
	
	uint8_t idx;

	if( (HOSTTxQ_cnt+len) > MAX_HOST_TXQ_LENGTH )
	{
		return 0x01 ;
	} else {
		DIR_HOST_RS485_Out();
		for(idx=0;idx<len;idx++)
		{
			HOSTTxQ[HOSTTxQ_wp]=Str[idx];
			HOSTTxQ_wp++;
			if(HOSTTxQ_wp>=MAX_HOST_TXQ_LENGTH)
			{
				HOSTTxQ_wp=0;
			}
			HOSTTxQ_cnt++;
		}        				
		UART_EnableInt(UART1, (UART_INTEN_THREIEN_Msk ));
	}
	while (!(UART1->FIFOSTS & UART_FIFOSTS_TXEMPTYF_Msk));
	return 0x00 ;
}



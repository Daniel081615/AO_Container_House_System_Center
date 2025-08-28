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


void GetHostRTC(void);
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
void SendHost_CenterFWinfo(void);
void SendHost_MeterFWInfo(void);
void SendHost_CenterUpdateSuccsess(void);
void SendHost_MeterUpdateSuccsess(void);



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
								GetHostRTC();
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
                    case CTR_OTA_UPDATE_CTR:							// 0x15
                        Host_OTACenterProcess();
                        ClearRespDelayTimer();
                        break;
										case CTR_OTA_UPDATE_MTR:							// 0x16
												Host_OTAMeterProcess();
												break;
										case CTR_SET_WTR_TIME:
												
												ClearRespDelayTimer();
                    default:
                        break;
                }
            }
			
        } else {
            ResetHostUART();
        }
		}

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


/***	@CenterProcessHostCmdsFunctions	***
*
*	HOST_AliveProcess()
*	Host_PowerMeterDataProcess() 	:	Get power meter Cmd
*	Host_BmsDataProcess() 			 	:	Get Bms Cmd
*	Host_WMDataProcess() 					:	Get water meter Cmd
*	Host_InvDataProcess() 				:	Get inverter Cmd
*	Host_OTACenterProcess()				:	Get center board OTA Cmd
*	Host_OTAMeterProcess()				:	Get meter board OTA Cmd
*	Host_WateringSetupProcess			:	Get	watering time setup
*	@note	"Meter index id is replaced by device ID... Need to be revise"
*
*	Cmd form 
0 : 0x55
1 : CenterID
2 : Center CMD
3 : Meter Idx
4 : Device Idx
5 : Device CMD
 ***																	***/

void HOST_AliveProcess(void)
{	

		GetHostRTC();
}

void Host_PowerMeterDataProcess(void)
{
		HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		PwrMeterCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_BmsDataProcess(void)
{
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		BmsCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_WMDataProcess(void)
{
	
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		WtrMeterCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_InvDataProcess(void)
{
		HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		InvCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_OTACenterProcess(void)
{
		SYS_UnlockReg();
		FMC_Open();
	
		GetHostRTC();
	
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

void Host_OTAMeterProcess(void){

		SYS_UnlockReg();
		FMC_Open();
	
		GetHostRTC();
	
		OTAMeterID = TokenHost[3];
		MeterOtaCmdList[OTAMeterID-1] = TokenHost[4];	
		
		if (TokenHost[4] == CMD_MTR_OTA_UPDATE)
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

void Host_WateringSetupProcess(void)
{
		uint8_t fnPacketIndex;
		GetHostRTC();
		HostMeterIndex = TokenHost[3];	//	MeterID
	
		fnPacketIndex = 6;
		
		Watering_SetUp.Hour 			= TokenHost[fnPacketIndex++];
		Watering_SetUp.Min				= TokenHost[fnPacketIndex++];
	  Watering_SetUp.Period_min = TokenHost[fnPacketIndex++];
}

/***
 *	@brief	Send System Error information to Host
 *	@note		Need to add Error status detect for each device
 ***/
void SendHost_SystemInformation(void)
{
    uint8_t i, fnPacketIndex;
    
    HostTxBuffer[2] = CTR_RSP_SYSTEM_INFO ; 
    HostTxBuffer[3] = (DevicesNG.PowerMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[4] = (DevicesNG.PowerMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[5] = (DevicesNG.PowerMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[6] = (DevicesNG.PowerMeterNG & 0x000000FF) >> 0 ;
	
		fnPacketIndex = 7 ;
    HostTxBuffer[fnPacketIndex++] = (DevicesNG.PowerMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[fnPacketIndex++] = (DevicesNG.PowerMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[fnPacketIndex++] = (DevicesNG.PowerMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[fnPacketIndex++] = (DevicesNG.PowerMeterNG & 0x000000FF) >> 0 ;		
	
    HostTxBuffer[fnPacketIndex++] = (MeterDeviceError & 0x000000FF)  ;
    HostTxBuffer[fnPacketIndex++] = NowPollingMeterBoard ;
    	
    CalChecksumH();	
	
}
void SendHost_PowerData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnMeterBoardIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_POWER_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
    fnMeterBoardIndex = HostDeviceIndex-1 ; 
    fnPacketIndex = 5 ; 
    // Byte 5.
    HostTxBuffer[fnPacketIndex++] = MeterData[fnMeterBoardIndex].ErrorRate;        
    HostTxBuffer[fnPacketIndex++] = MeterData[fnMeterBoardIndex].RelayStatus;

		HostTxBuffer[fnPacketIndex++] = (MeterData[fnMeterBoardIndex].TotalWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[fnPacketIndex++] = (MeterData[fnMeterBoardIndex].TotalWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[fnPacketIndex++] = (MeterData[fnMeterBoardIndex].TotalWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[fnPacketIndex++] =  MeterData[fnMeterBoardIndex].TotalWatt & 0x000000FF;	
	
     
    CalChecksumH();		
}

void SendHost_BmsData(void)
{
    uint8_t fnPacketIndex;
    uint8_t fnBmsIndex;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_BMS_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
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
    HostTxBuffer[3] = HostMeterIndex ; 
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
    HostTxBuffer[3] = HostMeterIndex ; 
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
void SendHost_CenterFWinfo(void)
{
		uint8_t i;
		HostTxBuffer[2] = CmdType;

		memcpy(&HostTxBuffer[4], &g_fw_ctx, sizeof(FWstatus));
    memcpy(&HostTxBuffer[20], &meta, sizeof(FWMetadata));
		memcpy(&HostTxBuffer[52], &other, sizeof(FWMetadata));

		CalChecksumH();
}


/* Send meter fw info to host 
0:0x55
1: MyCenterID
2: Command
4-19 : FWststatus 	(16 bytes)
20-51: FWMetadata1 	(32 bytes)
52-83: FWMetadata2 	(32 bytes)
*/
void SendHost_MeterFWInfo(void){
	
		uint8_t i;
		HostTxBuffer[2] = CmdType;

		memcpy(&HostTxBuffer[4], &MeterMetaStatus, sizeof(FWstatus));
    memcpy(&HostTxBuffer[20], &MeterMetaActive, sizeof(FWMetadata));
		memcpy(&HostTxBuffer[52], &MeterMetaBackup, sizeof(FWMetadata));
	
		CalChecksumH();
	
}

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
void SendHost_CenterUpdateSuccsess(void)
{
		HostTxBuffer[1] = MyCenterID;
		HostTxBuffer[4] = 0xEE;		HostTxBuffer[5] = 0xFF;
		HostTxBuffer[6] = 0x5A;		HostTxBuffer[7] = 0xA5;
		
		memcpy(&HostTxBuffer[8], &meta, sizeof(FWstatus));
		CalChecksumH();
}

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

/***
 *	@brief	Get Real-Time from Host, and store RTC in data flash
 *	(HEX)YY_H, YY_L, MM, DD, Hr, Min, Sec, Week
 ***/
void GetHostRTC(void)
{
		uint16_t u16tempH, u16tempC, TimeGap;
		
		// Year,Month,Day,Hour,Min,Sec,Week
		for (uint8_t i = 0; i < 7; i++) {
				HostSystemTime[i] = TokenHost[HOST_INX_TIME_START + i];
    }
		
		// Hour, Min
		u16tempH = (HostSystemTime[3] * 60) + HostSystemTime[4];
		u16tempC = (CtrSystemTime[3] * 60)  + CtrSystemTime[4];
		
		TimeGap = abs(u16tempH - u16tempC);
		
		if (TimeGap > 3)
		{
				for (uint8_t i = 0; i < 7; i++) 
				{
						CtrSystemTime[i] = HostSystemTime[i];
				}
				
				uint32_t address_rtc_base;
				uint32_t data_buffer[2]; 
				FWstatus Status1;
				address_rtc_base = FW_Status_Base + sizeof(FWstatus);
				memcpy(data_buffer, CtrSystemTime, sizeof(CtrSystemTime));
				
				SYS_UnlockReg();
				FMC_Open();

				ReadData(FW_Status_Base, address_rtc_base, (uint32_t*)&Status1);
				FMC_Erase_User(FW_Status_Base);
				
				WriteData(FW_Status_Base, FW_Status_Base + sizeof(Status1), (uint32_t*)&Status1);
				WriteData(address_rtc_base, address_rtc_base + sizeof(data_buffer), (uint32_t*)&data_buffer);

				SYS_LockReg();
		}
		
		
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



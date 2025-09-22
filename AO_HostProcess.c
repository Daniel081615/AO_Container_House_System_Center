/**--------------------------------------------------------------------------- 
 * File Name  : AO_HostProcess.C
 * Company 		: AOTECH
 * C Compiler : Keil C
 * Last revise Date : 2025.09.11
 * @Problem encountered : 
		1. When meter OTA Update, How can we recognize Fw in the other bank is for Meter?
		2. When there's no meter FW, how to download Fw in Center Bank?
 * --------------------------------------------------------------------------**/
#include <stdio.h>
#include "NUC1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_MeterProcess.h"
#include "AO_EE24C.h"
#include "AO_HostProcess.h"

uint8_t CmdType,WaitTime;

uint8_t system_SW_Flag = 0;
uint8_t system_SW_Timer = 0;



void HostProcess(void);
void HOST_AliveProcess(void);
void Host_PwrMtrDataProcess(void);
void Host_BmsDataProcess(void);
void Host_WtrMtrDataProcess(void);
void Host_InvDataProcess(void);
void Host_PyrMtrDataProcess(void);
void Host_SoilSensorDataProcess(void);
void Host_AirSensorDataProcess(void);
void Host_WateringSetupProcess(void);
void Host_MeterOTAProcess(void);
void Host_OTACenterProcess(void);

void SendHost_Ack(void);
void SendHost_SystemInformation(void);
void SendHost_PowerData(void);
void SendHost_BmsData(void);
void SendHost_WtrMtrData(void);
void SendHost_PyrMtrData(void);
void SendHost_SoilSensorData(void);
void SendHost_AirSensorData(void);
void SendHost_InvData(void);

void SendHost_CenterUpdateSuccess(void);
void SendHost_MeterUpdateSuccess(void);
void SendHost_WateringData(void);

void SendHost_FirstReset_Ack(void);

/**	OTA **/
void SendHost_CenterFWinfo(void);
void SendHost_MeterFwInfo(void);

void CalChecksumH(void);
void GetHostRTC(void);
void ClearRespDelayTimer(void);

uint8_t AckResult;
uint8_t HostMeterIndex;
uint8_t SystemSetting;

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
                        Host_PwrMtrDataProcess();
                        CmdType = CTR_RSP_POWER_DATA ;
                        ClearRespDelayTimer() ;
                        break;	
										case CTR_GET_CMD_BMS:									// 0x12
												Host_BmsDataProcess();
                        CmdType = CTR_RSP_BMS_DATA ;
                        ClearRespDelayTimer() ;											
												break;
										case CTR_GET_CMD_WATER_METER:					// 0x13
												Host_WtrMtrDataProcess();
                        CmdType = CTR_RSP_WM_DATA ;
                        ClearRespDelayTimer() ;
												break;
										case CTR_GET_CMD_INV:									// 0x14
												Host_InvDataProcess();
                        CmdType = CTR_RSP_INV_DATA ;
                        ClearRespDelayTimer() ;												
												break;
										case CTR_GET_CMD_PYRANOMETER:									// 0x15
												Host_PyrMtrDataProcess();
                        CmdType = CTR_RSP_PYR_DATA ;
                        ClearRespDelayTimer() ;												
												break;										
										case CTR_GET_CMD_SOIL_SENSOR:									// 0x16
												Host_SoilSensorDataProcess();
                        CmdType = CTR_RSP_SOIL_DATA ;
                        ClearRespDelayTimer() ;												
												break;
										case CTR_GET_CMD_AIR_SENSOR:									// 0x17
												Host_AirSensorDataProcess();
                        CmdType = CTR_RSP_AIR_DATA ;
                        ClearRespDelayTimer() ;												
												break;								
                    case CTR_OTA_CMD:							// 0x18
                        Host_OTACenterProcess();
                        ClearRespDelayTimer();
                        break;
										case CTR_2_METER_OTA_CMD:							// 0x19
												ClearRespDelayTimer();
												Host_MeterOTAProcess();
												break;
										case CTR_SET_WTR_TIME:								//0x1A
												Host_WateringSetupProcess();
												CmdType = CTR_RSP_WTR_TIME_SETUP;
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
                        SendHost_WtrMtrData();
                        break;
                    case CTR_RSP_INV_DATA :
                        SendHost_InvData();
                        break;
                    case CTR_RSP_PYR_DATA :
                        SendHost_PyrMtrData();
                        break;
                    case CTR_RSP_SOIL_DATA :
                        SendHost_SoilSensorData();
                        break;
                    case CTR_RSP_AIR_DATA :
                        SendHost_AirSensorData();
                        break;
										case CTR_RSP_WTR_TIME_SETUP :
												SendHost_WateringData();
												break;
										
										case CTR_OTA_UPDATE_CMD:
												SYS_UnlockReg();
												FMC_ENABLE_ISP();
												BankStatus[Center].Cmd = BTLD_UPDATE_CENTER;
												WriteFwStatus(&BankStatus[Center]);
												SendHost_CenterFWinfo();
												JumpToBootloader();
												break;

										case CTR_SWITCH_FW_CMD:
												SYS_UnlockReg();
												FMC_ENABLE_ISP();
												SendHost_CenterFWinfo();
												FwBankSwitchProcess(IsFwValid(&BankMeta[Center][Backup]));
												break;

										case CTR_GET_FW_STATUS_CMD:
												SendHost_CenterFWinfo();
												SYS_LockReg();
												break;

										case CTR_REBOOT_CMD:
												SendHost_CenterFWinfo();
												NVIC_SystemReset();
												JumpToBootloader();
												break;
										case CTR_RSP_MTR_FW_INFO:
												SendHost_MeterFwInfo();
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
*	Host_PwrMtrDataProcess() 	:	Get power meter Cmd
*	Host_BmsDataProcess() 			 	:	Get Bms Cmd
*	Host_WtrMtrDataProcess() 					:	Get water meter Cmd
*	Host_InvDataProcess() 				:	Get inverter Cmd
*	Host_OTACenterProcess()				:	Get center board OTA Cmd
*	Host_MeterOTAProcess()				:	Get meter board OTA Cmd
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

/***
 *	@brief	Send System Error information to Host
 *	@note		Need to add Error status detect for each device
 *	@Error	Status display Error
 ***/
void SendHost_SystemInformation(void)
{
    uint8_t i, PktIdx;
		uint8_t MtrBoardIdx;
    
    HostTxBuffer[2] = CTR_RSP_SYSTEM_INFO ; 
    HostTxBuffer[3] = NowPollingMtrBoard ;	
	  HostTxBuffer[4] = (MeterDeviceError & 0x000000FF)  ;
	
		MtrBoardIdx = NowPollingMtrBoard-1;
		PktIdx = 5;
		//	PowerMeter NG Status.
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PowerMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PowerMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PowerMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PowerMeterNG & 0x000000FF) >> 0 ;
		//	BMS NG Status.
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].BmsNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].BmsNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].BmsNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].BmsNG & 0x000000FF) >> 0 ;
		//	WM NG Status.
		HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].WaterMeterNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].WaterMeterNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].WaterMeterNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  DevicesNG[MtrBoardIdx].WaterMeterNG & 0x000000FF ;	
		//	Pyranometer NG Status.
		HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PyranometerNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PyranometerNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].PyranometerNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  DevicesNG[MtrBoardIdx].PyranometerNG & 0x000000FF ;

		//	Soil sensor NG Status.
		HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].SoilSensorNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].SoilSensorNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].SoilSensorNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  DevicesNG[MtrBoardIdx].SoilSensorNG & 0x000000FF ;

		//	Air sensor NG Status.
		HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].AirSensorNG & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].AirSensorNG & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (DevicesNG[MtrBoardIdx].AirSensorNG & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  DevicesNG[MtrBoardIdx].AirSensorNG & 0x000000FF ;

//	INV NG Status.
    HostTxBuffer[PktIdx++] =  DevicesNG[MtrBoardIdx].InvNG & 0x000000FF ;	
		
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].PowerMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].Bms;
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].WaterMeter;
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].Pyranometer;	
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].SoilSensor;
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].AirSensor;		
    HostTxBuffer[PktIdx++] = TotErrorRate[MtrBoardIdx].Inverter;	
		
    CalChecksumH();		
}

void Host_PwrMtrDataProcess(void)
{
		HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		PwrMtrCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_BmsDataProcess(void)
{
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		BmsCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_WtrMtrDataProcess(void)
{
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		WtrMtrCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
 
		GetHostRTC();
}

void Host_PyrMtrDataProcess(void)
{
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		PyrMtrCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];
	
		GetHostRTC();
}

void Host_SoilSensorDataProcess(void)
{
    HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
		SoilSensorCmdList[HostMeterIndex-1][HostDeviceIndex-1] = TokenHost[5];

		GetHostRTC();
}
void Host_AirSensorDataProcess(void)
{
		HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
 
		GetHostRTC();
}

void Host_InvDataProcess(void)
{
		HostMeterIndex = TokenHost[3];
    HostDeviceIndex = TokenHost[4];
 
		GetHostRTC();
}

void Host_OTACenterProcess(void)
{
		SYS_UnlockReg();
		FMC_ENABLE_ISP();

		CmdType = TokenHost[5];
		Get_DualBankStatus(	(FwStatus *)&BankStatus[Center], &BankMeta[Center][Active], &BankMeta[Center][Backup]);

		SYS_LockReg();

		GetHostRTC();
}

/***
 *	Host_MeterOTAProcess()
 *	@Brief	if Meter fw is in center backup bank, MeterPolling process OTA., 
							if "NOT", Go to Center Bootloader then write FW in another center flash bank.

1: Center ID
2: Cmd
3: Meter ID
4: Device ID
5: Device Cmd
 ***/
void Host_MeterOTAProcess(void)
{
		uint8_t MtrBoardIdx;	

		SYS_UnlockReg();
		FMC_ENABLE_ISP();

		CmdType = CTR_RSP_MTR_FW_INFO;
		MtrBoardIdx = TokenHost[3]-1;
		MeterOtaCmdList[MtrBoardIdx] = TokenHost[5];

		WaitTime = 10;
		
		if (MeterOtaCmdList[MtrBoardIdx] == METER_OTA_UPDATE_CMD)
		{
				if (!(BankMeta[Center][Backup].flags == (Fw_MeterFwFlag | Fw_InvalidFlag))) 
				{
						BankStatus[Center].Cmd = BTLD_UPDATE_METER;
						//	Use "OTA_MeterID_Flags" to mark which device need to be update
						BankStatus[Center].OTA_MeterID_Flags = 1 << MtrBoardIdx;
						WriteFwStatus((FwStatus *)&BankStatus[Center]);
						SendHost_CenterFWinfo();
						JumpToBootloader();
				}
		}
		SYS_LockReg();
		GetHostRTC();
}

/*** 
 *	@brief	Host Send center board: meter board Watering time setup info
Get watering time and send watering setup to meter board
 *	@note		do i need to store watering time in flash?? no need
 *	@struct.	Let Center do the command and decision logic
 ***/

void Host_WateringSetupProcess(void)
{
		uint8_t MtrBoardIdx;
		GetHostRTC();
		MtrBoardIdx = TokenHost[3]-1;	//	MeterID
		
		WATERING_SETTING_FLAG = TRUE;
	  Watering_SetUp[MtrBoardIdx].Period_min = TokenHost[6];
}



void SendHost_PowerData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t PwrMtrIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_POWER_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ;
	
		MtrBoardIdx = HostMeterIndex-1;
    PwrMtrIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
    // Byte 5.
    HostTxBuffer[PktIdx++] = MeterData[MtrBoardIdx][PwrMtrIdx].ErrorRate;        
    HostTxBuffer[PktIdx++] = MeterData[MtrBoardIdx][PwrMtrIdx].RelayStatus;

		HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].TotalWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].TotalWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].TotalWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].TotalWatt & 0x000000FF;	
	
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].V & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].V & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].V & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].V & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].I & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].I & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].I & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].I & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].F & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].F & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].F & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].F & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].P & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].P & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].P & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].P & 0x000000FF ;

    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].VA & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].VA & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].VA & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].VA & 0x000000FF ;
	
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].PwrFactor & 0xFF000000) >> 24 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].PwrFactor & 0x00FF0000) >> 16 ;
    HostTxBuffer[PktIdx++] = (MeterData[MtrBoardIdx][PwrMtrIdx].PwrFactor & 0x0000FF00) >> 8 ;
    HostTxBuffer[PktIdx++] =  MeterData[MtrBoardIdx][PwrMtrIdx].PwrFactor & 0x000000FF ;
		
    CalChecksumH();		
}

void SendHost_BmsData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t BmsIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_BMS_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
	
		MtrBoardIdx = HostMeterIndex-1;
    BmsIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = BmsData[MtrBoardIdx][BmsIdx].ErrorRate;			// Communicate rate
		HostTxBuffer[PktIdx++] = BmsData[MtrBoardIdx][BmsIdx].BalanceStatus;	// Battery mode
		HostTxBuffer[PktIdx++] = BmsData[MtrBoardIdx][BmsIdx].StateOfCharge;
		HostTxBuffer[PktIdx++] = BmsData[MtrBoardIdx][BmsIdx].StateOfHealth;	//	SOH
		//idx:9	Cell ststus 
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].CellStatus & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].CellStatus & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].CellStatus & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].CellStatus & 0x000000FF;	
		//idx:13	Cell volt
		for (uint8_t i = 0; i < 16; i++) {
				HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].CellVolt[i] >> 8);
				HostTxBuffer[PktIdx++] = BmsData[MtrBoardIdx][BmsIdx].CellVolt[i];
		}
		//idx:45	Battery watt 
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatWatt & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatWatt & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatWatt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].BatWatt & 0x000000FF;	
		//idx:49	Battery voltage 
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatVolt & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatVolt & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatVolt & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].BatVolt & 0x000000FF;	
		//idx:53	Battery current 
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatCurrent & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatCurrent & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatCurrent & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].BatCurrent & 0x000000FF;			
		//idx:63	Battery temperature [1-5]	
		for (uint8_t i = 0; i<5; i++)
		{		
				HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].BatteryTemp[i] >> 8);
				HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].BatteryTemp[i];
		}
		//idx:65 	Mos temperature 
		HostTxBuffer[PktIdx++] = (BmsData[MtrBoardIdx][BmsIdx].MosTemp >> 8);
		HostTxBuffer[PktIdx++] =  BmsData[MtrBoardIdx][BmsIdx].MosTemp;
		
		CalChecksumH();			
}

void SendHost_WtrMtrData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t WtrMtrIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_WM_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
	
		MtrBoardIdx = HostMeterIndex-1;
    WtrMtrIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = WtrMtrData[MtrBoardIdx][WtrMtrIdx].ErrorRate;			// Communicate rate
		HostTxBuffer[PktIdx++] = WtrMtrData[MtrBoardIdx][WtrMtrIdx].ValveState;			// 0xff : closed, 0x00 : opened
	
		HostTxBuffer[PktIdx++] = (WtrMtrData[MtrBoardIdx][WtrMtrIdx].TotalVolume & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (WtrMtrData[MtrBoardIdx][WtrMtrIdx].TotalVolume & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (WtrMtrData[MtrBoardIdx][WtrMtrIdx].TotalVolume & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  WtrMtrData[MtrBoardIdx][WtrMtrIdx].TotalVolume & 0x000000FF;
    
		CalChecksumH();			
}

void SendHost_PyrMtrData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t PyrMtrIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_PYR_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ;

		MtrBoardIdx = HostMeterIndex-1;
    PyrMtrIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = PyrMtrData[MtrBoardIdx][PyrMtrIdx].ErrorRate;			// Communicate rate
	
		HostTxBuffer[PktIdx++] = PyrMtrData[MtrBoardIdx][PyrMtrIdx].OffsetValue >> 8;
		HostTxBuffer[PktIdx++] = PyrMtrData[MtrBoardIdx][PyrMtrIdx].OffsetValue & 0xff;
	
		HostTxBuffer[PktIdx++] = PyrMtrData[MtrBoardIdx][PyrMtrIdx].SolarRadiation >> 8 ;
		HostTxBuffer[PktIdx++] = PyrMtrData[MtrBoardIdx][PyrMtrIdx].SolarRadiation & 0xff;		
    
		CalChecksumH();			
}


void SendHost_SoilSensorData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t SoilSensorIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_SOIL_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ;

		MtrBoardIdx = HostMeterIndex-1;
    SoilSensorIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = SoilSensorData[MtrBoardIdx][SoilSensorIdx].ErrorRate;			// Communicate rate
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Moisture & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Moisture & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Temperature & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Temperature & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].PH & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].PH & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Salinity & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Salinity & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].TDS & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].TDS & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fertility & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fertility & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC_Coef & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Salinity_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Salinity_Coef & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].TDS_Coef & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].TDS_Coef & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Temp_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Temp_Calib & 0x00FF;
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Moisture_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Moisture_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].EC_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].PH_Calib & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].PH_Calib & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Fert_Deviation & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Coef & 0x000000FF;	
	
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Nitrogen_Deviation & 0x00FF;

		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Phosphorus_Deviation & 0x00FF;
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Coef & 0xFF000000) >> 24 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Coef & 0x00FF0000) >> 16 ;
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Coef & 0x0000FF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Coef & 0x000000FF;	
		
		HostTxBuffer[PktIdx++] = (SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Deviation & 0xFF00) >> 8 ;
		HostTxBuffer[PktIdx++] =  SoilSensorData[MtrBoardIdx][SoilSensorIdx].Potassium_Deviation & 0x00FF;

		CalChecksumH();	

}

void SendHost_AirSensorData(void)
{
		uint8_t MtrBoardIdx;
    uint8_t PktIdx;
    uint8_t AirSensorIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_AIR_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
    HostTxBuffer[4] = HostDeviceIndex ; 
	
		MtrBoardIdx = HostMeterIndex-1;
    AirSensorIdx = HostDeviceIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].ErrorRate;			// Communicate rate
	
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Co2 >> 8 ;
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Co2 & 0xff;

		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Formaldehyde & 0xff >> 8 ;
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Formaldehyde;	
	
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Tvoc >> 8 ;
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Tvoc & 0xff;

		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Pm25 >> 8 ;
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Pm25 & 0xff;	

		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Pm10 >> 8 ;
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Pm10 & 0xff;

		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Temperature >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[MtrBoardIdx][AirSensorIdx].Temperature & 0xff;	
	
		HostTxBuffer[PktIdx++] = AirSensorData[MtrBoardIdx][AirSensorIdx].Humidity >> 8 ;
		HostTxBuffer[PktIdx++] =  AirSensorData[MtrBoardIdx][AirSensorIdx].Humidity & 0xff;
	
		CalChecksumH();	

}

void SendHost_InvData(void)
{
    uint8_t PktIdx;
    uint8_t MtrBoardIdx;
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_INV_DATA ;
    HostTxBuffer[3] = HostMeterIndex ; 
	
    MtrBoardIdx = HostMeterIndex-1 ; 
    PktIdx = 5 ; 
	
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].ErrorRate;			// Communicate rate
		//	Inverter Data
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].statusByte1;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].statusByte3;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].warnByte1;		
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].warnByte2;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].faultByte1;			
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].faultByte2;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].faultByte3;

		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].InputVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].InputVolt & 0x00ff;
		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].InputFreq & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].InputFreq & 0x00ff;		

		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].OutputVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].OutputVolt & 0x00ff;
		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].OutputFreq & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].OutputFreq & 0x00ff;
		
		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].BatVolt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].BatVolt & 0x00ff;

		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].BatCapacity;		
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].InvCurrent;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].LoadPercentage;			
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].MachineTemp;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].MachineStatusCode;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].SysStatus;
		
		HostTxBuffer[PktIdx++] = (InvData[MtrBoardIdx].PV_volt & 0xff00) >> 8 ;
		HostTxBuffer[PktIdx++] =  InvData[MtrBoardIdx].PV_volt & 0x00ff;
		
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].CtrlCurrent;	
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].CtrlTemp;
		HostTxBuffer[PktIdx++] = InvData[MtrBoardIdx].CtrlStatusCode;	
		
		CalChecksumH();			
}

void SendHost_WateringData(void)
{
    uint8_t PktIdx;
    uint8_t MtrBoardIdx;
		uint8_t WateringEndTime[8];
    
    HostTxBuffer[1] = MyCenterID ;
    HostTxBuffer[2] = CTR_RSP_WTR_TIME_SETUP ;
    HostTxBuffer[3] = HostMeterIndex ; 
	
    MtrBoardIdx = HostMeterIndex-1 ; 		
		PktIdx = 4;
		
		WateringEndTime[INX_HOUR] = CtrSystemTime[INX_HOUR];
		WateringEndTime[INX_MIN] = (CtrSystemTime[INX_MIN] + Watering_SetUp[MtrBoardIdx].Period_min);
	
		if (WateringEndTime[INX_MIN] > 60)
		{
				WateringEndTime[INX_HOUR] +=1;
				WateringEndTime[INX_MIN] -= 60;
		} 
		
		HostTxBuffer[PktIdx++] = Watering_SetUp[MtrBoardIdx].Period_min;
		HostTxBuffer[PktIdx++] = WateringEndTime[INX_HOUR];
		HostTxBuffer[PktIdx++] = WateringEndTime[INX_MIN];
		HostTxBuffer[PktIdx++] = CtrSystemTime[INX_SEC];
		HostTxBuffer[PktIdx++] = WateringStatus[MtrBoardIdx];	
	
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


void SendHost_CenterFWinfo(void)
{
		HostTxBuffer[1] = MyCenterID ;
		HostTxBuffer[2] = CmdType;	

		memcpy(&HostTxBuffer[4], 	&BankStatus[Center], sizeof(FwStatus));
    memcpy(&HostTxBuffer[20],	&BankMeta[Center][Active], sizeof(FwMeta));
		memcpy(&HostTxBuffer[52],	&BankMeta[Center][Backup], sizeof(FwMeta));

		CalChecksumH();
}

void SendHost_MeterFwInfo(void)
{
		HostTxBuffer[1] = MyCenterID ;
		HostTxBuffer[2] = MeterOtaCmdList[NowPollingMtrBoard-1];
		HostTxBuffer[3] = NowPollingMtrBoard;

		memcpy(&HostTxBuffer[4], 	&BankStatus[Meter], sizeof(FwStatus));
    memcpy(&HostTxBuffer[20], &BankMeta[Meter][Active], sizeof(FwMeta));
		memcpy(&HostTxBuffer[52], &BankMeta[Meter][Backup], sizeof(FwMeta));
	
		CalChecksumH();
}

/* 
	When meter update success, send host Update Success Password
*/
void SendHost_MeterUpdateSuccess(void)
{
		HostTxBuffer[1] = MyCenterID;					//MeterID
		HostTxBuffer[2] = GotMeterRspID;	//MeterID
		HostTxBuffer[4] = 0x0A;		HostTxBuffer[5] = 0xBB;
		HostTxBuffer[6] = 0xC0;		HostTxBuffer[7] = 0xDD;
		
		memcpy(&HostTxBuffer[8], (FwStatus *)&BankStatus[Meter], sizeof(FwStatus));
		CalChecksumH();
}

/* 
	When Center update success, send host Update Success Password 
*/
void SendHost_CenterUpdateSuccess(void)
{
		HostTxBuffer[1] = MyCenterID;
		HostTxBuffer[4] = 0xEE;		HostTxBuffer[5] = 0xFF;
		HostTxBuffer[6] = 0x5A;		HostTxBuffer[7] = 0xA5;
		
		memcpy(&HostTxBuffer[8], &BankMeta[Center][Active], sizeof(FwStatus));
		CalChecksumH();
}

/***
 *	@brief	Get Real-Time from Host, and store RTC in data flash
 *	(HEX)YY_H, YY_L, MM, DD, Hr, Min, Sec, Week
 ***/
void GetHostRTC(void)
{
		uint16_t u16tmpTimeH, u16tmpTimeC, TimeGap;
		
		// Year,Month,Day,Hour,Min,Sec,Week
		for (uint8_t i = 0; i < 7; i++) {
				HostSystemTime[i] = TokenHost[HOST_INX_TIME_START + i];
    }

		u16tmpTimeH = (HostSystemTime[INX_MIN] * 60) + HostSystemTime[INX_SEC];
		u16tmpTimeC = (CtrSystemTime[INX_MIN] * 60)  + CtrSystemTime[INX_SEC];
		
		TimeGap = abs(u16tmpTimeH - u16tmpTimeC);
		
		if (TimeGap >= 3)
		{
				for (uint8_t i = 0; i < 7; i++) 
				{
						CtrSystemTime[i] = HostSystemTime[i];
				}
		}
//				
//				uint32_t address_rtc_base;
//				uint32_t data_buffer[2]; 
//				FwStatus Status1;
//				address_rtc_base = BANK_STATUS_BASE + sizeof(FwStatus);
//				memcpy(data_buffer, CtrSystemTime, sizeof(CtrSystemTime));
//				
//				SYS_UnlockReg();
//				FMC_Open();

//				ReadData(BANK_STATUS_BASE, address_rtc_base, (uint32_t*)&Status1);
//				FMC_Erase_User(BANK_STATUS_BASE);
//				
//				WriteData(BANK_STATUS_BASE, BANK_STATUS_BASE + sizeof(Status1), (uint32_t*)&Status1);
//				WriteData(address_rtc_base, address_rtc_base + sizeof(data_buffer), (uint32_t*)&data_buffer);

//				SYS_LockReg();
//		}	
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



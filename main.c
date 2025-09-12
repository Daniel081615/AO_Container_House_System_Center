/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 23/10/25 4:34p $
 * @brief    Software Development Template.
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC1261.h"
#include "AO_MyDef.h"
#include "AO_ExternFunc.h"
#include "AO_EE24C.h"
#include "AO_HostProcess.h"
#include "AO_MeterProcess.h"
#include "AO2022_Center_1261.h"

// 0 : 校本部東一
// 1 : 校本部東二
// 2 : 南港
// 3 : 新北
#define AREA_ID     0

#define PLLCTL_SETTING  CLK_PLLCTL_72MHz_HIRC
#define PLL_CLOCK       72000000

#define VERSION	 "AOTECH-2023-01-20"

// Center ID 對應連接Meter模組數量
// CPU
#if 1
#define SMART_CENTER_MAX			9
const uint8_t MeterDeviceMaxTable[SMART_CENTER_MAX]={1,2,2,2,2,2,2,2,2};
#define SMART_ROOM_MAX			9
const uint8_t RoomMaxTable[SMART_ROOM_MAX]={1,30,23,29,31,29,31,29,31};
//const uint8_t RoomMaxTable[SMART_ROOM_MAX]={1,1,23,29,31,29,31,29,31};

#endif


uint8_t RoomMax;
uint8_t MeterDeviceMax;
uint8_t MyCenterID;
uint8_t SystemMode;		// 單人 / 多人 / 負值
uint8_t iStatus;
uint8_t fgHostFlag;
_Bool 	fgReaderSync,fgOpenRoomDoor,fgBalanceUpdated,fgUserChargeModeUpdated;
uint16_t MeterBalanceValueX;	// Add / Dec Balance from Host
uint8_t MeterBalanceOP;	// 0xA5 : Add / 0x5A Dec 
uint8_t HostOldDoorRCDStart;	// Read Door RCD from Meter
uint8_t HostRecordType,HostRecordIndex;
uint8_t HostPackageIndex;
uint8_t MeterResult;
uint8_t AutoLockTime ;			// 自動上鎖等待時間
uint32_t ReaderDeviceError,MeterDeviceError;

// 暫存 Host 端參數
uint8_t Host_MemberCounter,Host_RoomMode,Host_NumInChargeMode;
uint8_t UserUID[4];
uint8_t MeterMemberIndex;
uint8_t MeterMessageType;
uint8_t NowRCD_MeterIndex,NowRCD_Type;

// TICK 
volatile uint32_t   u32TimeTick2;
volatile uint8_t Tick_10mSec;
uint16_t     SystemTick;


uint8_t ErrorRate[ROOM_MAX];

uint8_t CtrSystemTime[8]={0,0,0,0,0,0,0,0};		// Year,Month,Day,Hour,Min,Sec,Week
uint8_t HostSystemTime[8] = {0,0,0,0,0,0,0,0};
uint8_t WateringTime[4] = {0,0,0,0};
uint8_t HostTxBuffer[HOST_TOKEN_LENGTH];
uint8_t MeterTxBuffer[METER_TOKEN_LENGTH];
uint8_t TokenHost[HOST_TOKEN_LENGTH];
uint8_t TokenMeter[METER_TOKEN_LENGTH];


uint8_t 	UID[4];
uint8_t 	NewUser,NowUser,LastUser;
uint8_t 	MemberIndex;
uint8_t 	CenterID;
uint8_t	Tick_20mSec,DelayTick;
uint8_t 	HostTokenReady,MeterTokenReady;
uint8_t 	Node1TokenReady,Node2TokenReady;
uint8_t     CenterRecord_WP,CenterRecord_RP,CenterNewRecordCounter,TickRecord;

uint8_t TickTest;

uint8_t MeterInitState;

_Bool flagMeterOtaUpdate; 

uint8_t ReTryCounter,TickWaitAck;
uint8_t ReaderInitialState;
uint8_t CheckUserResult;
uint8_t PollingMode;
uint8_t TickReader;
uint8_t iTickDelaySendHostCMD,bDelaySendHostCMD,bValueUpdated,bSystemTimeReady;
uint8_t 	DelaySwitchDIR1, DelaySwitchDIR2;
uint8_t 	DelaySwitchDIR3,DelaySwitchDIR4;
uint8_t MeterMode,MeterUser,AlivePacketIndex;
uint16_t MeterBalance;
uint8_t  CenterPackageChecksum,FormMeterFlag;

uint8_t TickRecord;

uint8_t TickHostUart, TickMeterUart;
uint8_t TickNode1Uart, TickNode2Uart;

uint8_t ReaderState,ReaderTick,bSendReaderCommand,ReaderCommandType;
_Bool bRecoverSystem,bInitSystemMode;
_Bool fgMeterAckOK;
_Bool fgDIR485_HOST_In,fgDIR485_METER_In;
_Bool 	fgDIR485_NODE1_In,fgDIR485_NODE2_In ;
uint8_t TickHost, TickMeter;
uint16_t  TickTestDelayTime;

uint8_t GotMeterRspID, HostDeviceIndex;

uint8_t NowPollingMtrBoard;
uint8_t NowPollingPwrMtrID, NowPollingBmsID, NowPollingWtrMtrID, NowPollingPyrMtrID, NowPollingSoilSensorID, NowPollingAirSensorID, NowPollingInvID;

uint8_t TickPollingInterval_Meter;
uint8_t PollRetryCounter_Meter;
uint8_t RoomIndex;
uint8_t NodeTestAck,TestState1,TestState2;
uint8_t TestNodeTick,HostRoomIndex;

uint8_t MeterDeviceMax;
uint8_t MeterWaitTick;
_Bool bResetUARTQ;
uint8_t PackageIndex1;
uint32_t ReaderDeviceError,MeterButtonStatus,MeterRelayStatus;

_Bool fgFirstTimeCheckAC;
_Bool bACPowerStatus;
uint8_t AC_Status_Counter,tick_CheckAC;
uint16_t Tick1S_CheckACLoss;
uint8_t LastSystemMode;
uint8_t Tick_RoomModePro;
uint8_t LastRoomMode,RoomModeCounter;
_Bool fgReaderReset ;
uint8_t Tick20mS_ReaderReset;
uint8_t tick_CalBalance;
uint32_t MeterValueTest;

//Host to meter devices & Ota Cmd List
uint8_t MeterOtaCmdList[MtrBoardMax];
uint8_t PwrMtrCmdList[MtrBoardMax][PwrMtrMax];
uint8_t BmsCmdList[MtrBoardMax][BmsMax];
uint8_t WtrMtrCmdList[MtrBoardMax][WtrMtrMax];
uint8_t PyrMtrCmdList[MtrBoardMax][PyrMtrMax];
uint8_t SoilSensorCmdList[MtrBoardMax][SoilSensorMax];
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
uint16_t SystemTick;
RTC_Data_t RTC_Data;
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint8_t g_u8RxData;
volatile uint8_t g_u8DataLen;
volatile uint8_t g_u8EndFlag = 0;

volatile uint32_t g_u32WDTINTCounts;
volatile uint8_t g_u8IsWDTWakeupINT;

//uint8_t PWRMeterData[ROOM_MAX];
//uint8_t PWRMeterError[ROOM_MAX];

uint32_t ManagerData[10];

uint8_t HostTxBuffer[HOST_TOKEN_LENGTH];
uint8_t MeterTxBuffer[METER_TOKEN_LENGTH];
uint8_t HostToken[HOST_TOKEN_LENGTH];
uint8_t MeterToken[METER_TOKEN_LENGTH];
uint8_t HOSTRxQ[MAX_HOST_RXQ_LENGTH];
uint8_t HOSTTxQ[MAX_HOST_TXQ_LENGTH];
uint8_t METERRxQ[MAX_METER_RXQ_LENGTH];
uint8_t METERTxQ[MAX_METER_TXQ_LENGTH];

uint8_t RoomNumberMax;
uint8_t MyCenterID;
uint8_t NowPollingRoom;
uint8_t Tick1S_CheckMeterBoards;
_Bool WATERING_SETTING_FLAG;

/*

    1. Room's Mode / Now User Index
    2. Member's UID 1 ~ 10
    3. Member's Balance 1 ~ 10
    4. Meter-Total Watt
    5. Meter-V / I / F / Watt / VAR / PF 

*/

	

// Center ID 對應連接Meter模組數量


// 新北高工
#if 1
#define HK_MAX	5
const uint8_t RoomNumberMaxTable[HK_MAX]=
{20, 21, 17, 17, 4};	 // 1F ~ 
#endif 


uint8_t SystemMode;		// 單人 / 多人 / 負值
uint8_t iStatus;
uint8_t fgHostFlag;
uint16_t MeterBalanceValueX;	// Add / Dec Balance from Host
uint8_t MeterBalanceOP;	// 0xA5 : Add / 0x5A Dec 

uint8_t HostRecordType,HostRecordIndex;
uint8_t HostPackageIndex;
uint8_t MeterResult;

// 暫存 Host 端參數

uint8_t MeterMemberIndex;
uint8_t HostPacketIndex;

// TICK 
volatile uint32_t   u32TimeTick2;
volatile uint8_t Tick_10mSec;
uint16_t     SystemTick,Tick1S_HostMissLink;


uint8_t HOSTRxQ_wp,HOSTRxQ_rp,HOSTRxQ_cnt;
uint8_t HOSTTxQ_wp,HOSTTxQ_rp,HOSTTxQ_cnt;
uint8_t METERTxQ_wp,METERTxQ_rp,METERTxQ_cnt;
uint8_t METERRxQ_wp,METERRxQ_rp,METERRxQ_cnt;

uint8_t 	MemberIndex;
uint8_t	Tick_20mSec,DelayTick;
uint8_t 	HostTokenReady,MeterTokenReady;

uint8_t iTickDelaySendHostCMD,bDelaySendHostCMD,bValueUpdated,bSystemTimeReady;

uint8_t TickRecord;

uint8_t TickHostUart, TickMeterUart;

_Bool bRecoverSystem;
_Bool fgMeterAckOK;
uint8_t TickHost, TickMeter;
uint16_t  TickTestDelayTime;
uint8_t GotMeterRspID;

uint8_t NowPollingMtrBoard;
uint8_t TickPollingInterval_Meter;
uint8_t PollRetryCounter_Meter;
uint8_t HostRoomIndex;

uint8_t RoomNumberMax;
uint8_t MeterWaitTick;
_Bool bResetUARTQ;
uint8_t PackageIndex1;
uint32_t ReaderDeviceError,MeterButtonStatus,MeterRelayStatus;
uint8_t TickFromHostUpdates;
_Bool fgInitOK;  

_Bool fgReaderReset ;
uint8_t Tick20mS_ReaderReset;

uint8_t centerResetStatus = 0;
uint16_t TickDirHostRS485Delay;
_Bool		fgDirHostRS485Out2In;		
uint16_t TickDirDebugRS485Delay;
_Bool		fgDirDebugRS485Out2In;		
uint8_t  ErrorCode;



void ResetMeterUART(void);
void ResetHostUART(void);

void ChangeRS485Direct(void);
void ReaderUpdateAllData(void);

void GPIO_Init(void);
void DefaultValue(void);

void ResetMeterUART(void);
void ResetHostUART(void);

void ChangeRS485Direct(void);

void ReadMyCenterID(void);

void RecoverSystemMoniter(void);
void ChangeDirHostRS485(void);
void ScheduleWateringTask(void);

volatile uint8_t MeterOtaFlag;




/**
 * @brief       IRQ Handler for WDT Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The WDT_IRQHandler is default IRQ of WDT, declared in startup_NUC126.s.
 */
void WDT_IRQHandler(void)
{

    
		SYS_UnlockReg();
		FMC_Open();
	
		
		//LED_G_On();
		LED_R1_TOGGLE();LED_TGL_R();
	
		WDT_RESET_COUNTER();
		
		uint32_t BackupBank_addr = (BankStatus[Center].Meta_addr == BANK1_META_BASE) ? BANK2_META_BASE : BANK1_META_BASE;
		
		if (WDT_GET_TIMEOUT_INT_FLAG())
    {
			
        WDT_CLEAR_TIMEOUT_INT_FLAG();
        g_u32WDTINTCounts++;			
			
				BankMeta[Center][Active].WDTRst_counter = g_u32WDTINTCounts;
				WriteMetadata((FwMeta *)&BankMeta[Center][Active], BankStatus[Center].Meta_addr);
			
				if(g_u32WDTINTCounts > MAX_WDT_TRIES)
				{
						BankMeta[Center][Active].flags &= ~(Fw_ActiveFlag | Fw_ValidFlag);
						BankMeta[Center][Active].flags |= Fw_InvalidFlag;
						BankMeta[Center][Backup].flags |= Fw_ActiveFlag;
						WriteMetadata((FwMeta *)&BankMeta[Center][Backup], BackupBank_addr);
						WriteMetadata((FwMeta *)&BankMeta[Center][Active], BankStatus[Center].Meta_addr);
						WDT_Open(WDT_TIMEOUT_2POW4, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
						while (1);
				}
    }
		SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  SysTick Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void SysTick_Handler(void)
{
// 10mSec Interrupt 

	MeterWaitTick++;
	TickPollingInterval_Meter++;
	Tick_10mSec++;
	iTickDelaySendHostCMD++;
	Tick20mS_ReaderReset ++;
	
	u32TimeTick2++;
		
	if(system_SW_Flag == 1)
	{
		system_SW_Timer++;
	}
	
	if(Tick_10mSec >= 10)
	{
		Tick_10mSec = 0 ;	
		TickFromHostUpdates ++ ;	
            
	}
	
	SystemTick++;
	if ( SystemTick > SYSTEM_ERROR_TIMEOUT ) 
	{
		bRecoverSystem = 1 ;
		RecoverSystemMoniter();
	}		
	
	if ( HOSTRxQ_cnt > 0 )
	{
		TickHostUart++;
		if ( TickHostUart > 100 )
		{
			TickHostUart = 0 ;
			ResetHostUART() ;
		}
	}
	
	if ( METERRxQ_cnt > 0 )
	{
		TickMeterUart++;
		if ( TickMeterUart > 100 )
		{
			TickMeterUart = 0 ;
			ResetMeterUART();
		}
	}

	TickTestDelayTime++;
	
	Tick_20mSec++;
	//TickPollWating++;
	u32TimeTick2++;

	if(Tick_20mSec >= 100)
	{				

		
		Tick_20mSec = 0 ;		
		CtrSystemTime[INX_SEC]++;
		if(CtrSystemTime[INX_SEC]>=60) 
		{
			CtrSystemTime[INX_SEC] =0 ;						
			CtrSystemTime[INX_MIN]++;
			if(CtrSystemTime[INX_MIN] >= 60)
			{
				CtrSystemTime[INX_MIN]=0;						
				CtrSystemTime[INX_HOUR]++;
				if(CtrSystemTime[INX_HOUR] >= 24)
				{
					CtrSystemTime[INX_HOUR] = 0 ;
					bValueUpdated |= 0x02 ;
				}		
			}	
		}				
					
	}    
}

/*---------------------------------------------------------------------------------------------------------*/
/*  UART0/UART2 Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void UART02_IRQHandler(void)
{
		uint32_t u32IntSts ;
		uint8_t Rxbuf,i;

		u32IntSts = UART0->INTSTS;
		if(u32IntSts & UART_INTSTS_RDAINT_Msk)
		{		
				/* Get all the input characters */
				while(UART_IS_RX_READY(UART0))
				{
						/* Get the character from UART Buffer */
						Rxbuf = UART_READ(UART0);

						/* Rx completed */		
						METERRxQ[METERRxQ_wp] = Rxbuf;
						METERRxQ_wp++;
						METERRxQ_cnt++;
						if ( METERRxQ_wp >= MAX_METER_RXQ_LENGTH ) METERRxQ_wp=0;
						if ( METERRxQ[0] != 0x55 ) {
								METERRxQ_wp=0;
								METERRxQ_cnt=0;
								METERRxQ_rp=0;
						}	
						if( METERRxQ_cnt >= METER_TOKEN_LENGTH )
						{
								if ( METERRxQ[METER_TOKEN_LENGTH-1] != 0x0A ) {
									METERRxQ_wp=0;
									METERRxQ_cnt=0;
									METERRxQ_rp=0;
									return;
								}
								for(i=0;i<METER_TOKEN_LENGTH;i++)
								{
									TokenMeter[i]=METERRxQ[METERRxQ_rp];
									METERRxQ_cnt--;
									METERRxQ_rp++;
									if(METERRxQ_rp >= MAX_METER_TXQ_LENGTH) METERRxQ_rp = 0 ;
								}
								METERRxQ_wp=0;
								METERRxQ_cnt=0;
								METERRxQ_rp=0;
								MeterTokenReady = 1 ;
								//LED_TGL_G();
						} 
				}	
		}
	
		u32IntSts = UART0->INTSTS;
		if(u32IntSts & UART_INTSTS_THREINT_Msk)
		{
				if(METERTxQ_cnt > 0)
				{      			
					while(UART_IS_TX_FULL(UART0));  /* Wait Tx is not full to transmit data */
					UART_WRITE(UART0, METERTxQ[METERTxQ_rp]);      
					METERTxQ_cnt--;				                   
					METERTxQ_rp++;
					if(METERTxQ_rp >= MAX_METER_TXQ_LENGTH) 
						METERTxQ_rp = 0 ;		
				} else {			
					/* Disable UART RDA and THRE interrupt */
					UART_DisableInt(UART0, (UART_INTEN_THREIEN_Msk));
					UART_EnableInt(UART0, (UART_INTEN_RDAIEN_Msk));
					//LED_TGL_R();
				}                	
		}
}
/*---------------------------------------------------------------------------------------------------------*/
/*  UART1 Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void UART1_IRQHandler(void)
{
	uint32_t u32IntSts;
	uint8_t Rxbuf,i;

	u32IntSts = UART1->INTSTS;
	
	if(u32IntSts & UART_INTSTS_RDAINT_Msk)
	{
	    //printf("\nInput:");

	    /* Get all the input characters */
	    while(UART_IS_RX_READY(UART1))
	    {
	        /* Get the character from UART Buffer */
	        Rxbuf = UART_READ(UART1);

					HOSTRxQ[HOSTRxQ_wp] = Rxbuf;
					HOSTRxQ_wp++;
					HOSTRxQ_cnt++;
					if ( HOSTRxQ_wp >= MAX_HOST_RXQ_LENGTH ) HOSTRxQ_wp=0;
					if ( HOSTRxQ[0] != 0x55 ) {
						HOSTRxQ_wp=0;
						HOSTRxQ_cnt=0;
						HOSTRxQ_rp=0;
					}	
					if( HOSTRxQ_cnt >= HOST_TOKEN_LENGTH )
					{
						if ( HOSTRxQ[HOST_TOKEN_LENGTH-1] != 0x0A ) {
							HOSTRxQ_wp=0;
							HOSTRxQ_cnt=0;
							HOSTRxQ_rp=0;
							return;
						}
						for(i=0;i<HOST_TOKEN_LENGTH;i++)
						{
							TokenHost[i]=HOSTRxQ[HOSTRxQ_rp];
							HOSTRxQ_cnt--;
							HOSTRxQ_rp++;
							if(HOSTRxQ_rp >= MAX_HOST_RXQ_LENGTH) HOSTRxQ_rp = 0 ;
						}
						HOSTRxQ_wp=0;
						HOSTRxQ_cnt=0;
						HOSTRxQ_rp=0;
						HostTokenReady = 1 ;
						//LED_TGL_R();
					} 
			}
	}

	u32IntSts = UART1->INTSTS;
	if(u32IntSts & UART_INTSTS_THREINT_Msk)
	{
		
		if(HOSTTxQ_cnt > 0)
		{      
			DIR_HOST_RS485_Out();
			while(UART_IS_TX_FULL(UART1));  /* Wait Tx is not full to transmit data */
			UART_WRITE(UART1, HOSTTxQ[HOSTTxQ_rp]);      
			HOSTTxQ_cnt--;				                   
			HOSTTxQ_rp++;
			if(HOSTTxQ_rp >= MAX_HOST_TXQ_LENGTH) 
				HOSTTxQ_rp = 0 ;		
		} else {
			TickDirHostRS485Delay = 0 ;
			fgDirHostRS485Out2In = 1 ;			
			/* Disable UART RDA and THRE interrupt */
    			UART_DisableInt(UART1, (UART_INTEN_THREIEN_Msk));
			UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk));
		}                	
	}
}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 */
    uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	uint32_t u32baudrate = 57600 ;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART0 */
	SYS_ResetModule(UART0_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART1, 57600);

	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Select UART function */
	UART0->FUNCSEL = UART_FUNCSEL_UART;

	/* Set UART line configuration */
	UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

	/* Set UART Rx and RTS trigger level */
	UART0->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART0->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART0->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    	}
	
	/* Enable UART RDA and THRE interrupt */
	//UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk));
	UART_EnableInt(UART0, (UART_INTEN_RDAIEN_Msk));
}

void UART1_Init()
{
    uint8_t u8UartClkSrcSel, u8UartClkDivNum;
	uint32_t u32ClkTbl[4] = {__HXT, 0, __LXT, __HIRC};
	uint32_t u32Baud_Div = 0;
	uint32_t u32baudrate = 57600 ;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Reset UART1 */
	SYS_ResetModule(UART1_RST);

	/* Configure UART1 and set UART1 Baudrate */
	//UART_Open(UART1, 57600);

	/* Get UART clock source selection */
	u8UartClkSrcSel = (CLK->CLKSEL1 & CLK_CLKSEL1_UARTSEL_Msk) >> CLK_CLKSEL1_UARTSEL_Pos;

	/* Get UART clock divider number */
	u8UartClkDivNum = (CLK->CLKDIV0 & CLK_CLKDIV0_UARTDIV_Msk) >> CLK_CLKDIV0_UARTDIV_Pos;

	/* Select UART function */
	//UART1->FUNCSEL = UART_FUNCSEL_UART;
	/* Select UART RS485 function mode */
    	UART1->FUNCSEL = UART_FUNCSEL_RS485;

    	/* Set RS485-Master as AUD mode */
    	/* Enable AUD mode to HW control RTS pin automatically */
    	/* It also can use GPIO to control RTS pin for replacing AUD mode */
    	UART1->ALTCTL = UART_ALTCTL_RS485AUD_Msk;

    	/* Set RTS pin active level as high level active */
    	UART1->MODEM = (UART1->MODEM & (~UART_MODEM_RTSACTLV_Msk)) | UART_RTS_IS_HIGH_LEV_ACTIVE;

    	/* Set TX delay time */
    	UART1->TOUT = 0;

	/* Set UART line configuration */
	UART1->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;

	/* Set UART Rx and RTS trigger level */
	UART1->FIFO &= ~(UART_FIFO_RFITL_Msk | UART_FIFO_RTSTRGLV_Msk);

	/* Get PLL clock frequency if UART clock source selection is PLL */
	if(u8UartClkSrcSel == 1)
		u32ClkTbl[u8UartClkSrcSel] = CLK_GetPLLClockFreq();

	/* Set UART baud rate */
	if(u32baudrate != 0)
	{
		u32Baud_Div = UART_BAUD_MODE2_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate);

		if(u32Baud_Div > 0xFFFF)
			UART1->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER((u32ClkTbl[u8UartClkSrcSel]) / (u8UartClkDivNum + 1), u32baudrate));
		else
			UART1->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);
    	}
	
	/* Enable UART RDA and THRE interrupt */
	//UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk | UART_INTEN_THREIEN_Msk));
	UART_EnableInt(UART1, (UART_INTEN_RDAIEN_Msk ));
}


void SYS_Init(void)
{
	/* Set PF multi-function pins for X32_OUT(PF.0) and X32_IN(PF.1) */
	SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF0MFP_Msk)) | SYS_GPF_MFPL_PF0MFP_X32_OUT;
	SYS->GPF_MFPL = (SYS->GPF_MFPL & (~SYS_GPF_MFPL_PF1MFP_Msk)) | SYS_GPF_MFPL_PF1MFP_X32_IN;

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init System Clock                                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/

	/* Enable HIRC, HXT and LXT clock */
	/* Enable Internal RC 22.1184MHz clock */
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk | CLK_PWRCTL_LXTEN_Msk);

	/* Wait for HIRC, HXT and LXT clock ready */
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk | CLK_STATUS_LXTSTB_Msk);

	/* Select HCLK clock source as HIRC and HCLK clock divider as 1 */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

	/* Set core clock as PLL_CLOCK from PLL */
	CLK_SetCoreClock(PLL_CLOCK);
	
#define SYS_IRCTCTL0_AUTO_TRIM_HIRC_ENABLE 0x01
	// Auto Trim HIRC to 22.1184 MHz
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_FREQSEL_Msk)) | (SYS_IRCTCTL0_AUTO_TRIM_HIRC_ENABLE << SYS_IRCTCTL0_FREQSEL_Pos);
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_REFCKSEL_Msk)) | (0x00 << SYS_IRCTCTL0_REFCKSEL_Pos) ;
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_LOOPSEL_Msk)) | (0x11<<SYS_IRCTCTL0_LOOPSEL_Pos) ;
	SYS->IRCTCTL0 = (SYS->IRCTCTL0 & (~SYS_IRCTCTL0_RETRYCNT_Msk)) | (0x11<<SYS_IRCTCTL0_RETRYCNT_Pos) ;

	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);

	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);	
	CLK_EnableModuleClock(UART1_MODULE);
	CLK_EnableModuleClock(UART2_MODULE);

	CLK_EnableModuleClock(ISP_MODULE);
	CLK_EnableModuleClock(CRC_MODULE);
	CLK_EnableModuleClock(WDT_MODULE);

	/* Select UART module clock source as HXT and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	CLK_SetModuleClock(UART2_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));
	//CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);	
	CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, MODULE_NoMsk);

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init I/O Multi-function                                                                                 */
	/*---------------------------------------------------------------------------------------------------------*/

	/* Set PD multi-function pins for UART0 RXD(PD.0) and TXD(PD.1) */
	SYS->GPD_MFPL = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD0MFP_Msk)) | SYS_GPD_MFPL_PD0MFP_UART0_RXD;
	SYS->GPD_MFPL = (SYS->GPD_MFPL & (~SYS_GPD_MFPL_PD1MFP_Msk)) | SYS_GPD_MFPL_PD1MFP_UART0_TXD;

	/* Set PE multi-function pins for UART1 RXD(PE.13) and TXD(PE.12) */
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE13MFP_Msk)) | SYS_GPE_MFPH_PE13MFP_UART1_RXD;
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE12MFP_Msk)) | SYS_GPE_MFPH_PE12MFP_UART1_TXD;	
	SYS->GPE_MFPH = (SYS->GPE_MFPH & (~SYS_GPE_MFPH_PE11MFP_Msk)) | SYS_GPE_MFPH_PE11MFP_UART1_nRTS;	

	/* Set PC multi-function pins for UART2 RXD(PC.3) and TXD(PC.2) */
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC3MFP_Msk)) | SYS_GPC_MFPL_PC3MFP_UART2_RXD;
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC2MFP_Msk)) | SYS_GPC_MFPL_PC2MFP_UART2_TXD;			
	
}

//WDT Init
void WDT_Init(void)
{
		WDT_Open(WDT_TIMEOUT_2POW16, WDT_RESET_DELAY_130CLK, TRUE, FALSE);
		WDT_EnableInt();
		NVIC_EnableIRQ(WDT_IRQn);
		WDT_RESET_COUNTER();
}

void GPIO_Mode_Init(void)
{
	// PA
	/* Configure PB.2 as Output mode and PE.1 as Input mode then close it */
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PA->MODE = (PA->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
	// PB	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);	
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE2_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE3_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE4_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE5_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE5_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE6_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE6_Pos);
	PB->MODE = (PB->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_QUASI << GPIO_MODE_MODE7_Pos);
	//PC
	PC->MODE = (PC->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	PC->MODE = (PC->MODE & (~GPIO_MODE_MODE1_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE1_Pos);
	PC->MODE = (PC->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos);
	// PD
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
	PD->MODE = (PD->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE7_Pos);
	// PE
	PE->MODE = (PE->MODE & (~GPIO_MODE_MODE0_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE0_Pos);
	PE->MODE = (PE->MODE & (~GPIO_MODE_MODE10_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE10_Pos);
	PE->MODE = (PE->MODE & (~GPIO_MODE_MODE11_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE11_Pos);
	// PF
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE2_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE2_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos);
	PF->MODE = (PF->MODE & (~GPIO_MODE_MODE7_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE7_Pos);
	
}

void TestI2C_RWEEPROM(void)
{
	uint8_t TestDataBuf[100];
	uint8_t i;
	
	for(i = 0; i < 100; i++)
	{
		soft_i2c_eeprom_write_byte(EEPROM_ADDR,i,i+10);
	}
	for(i = 0; i < 100; i++)
	{
		TestDataBuf[i] = soft_i2c_eeprom_read_byte(EEPROM_ADDR,i);
	}
	for(i = 0; i < 100; i++)
	{		if ( TestDataBuf[i] != i )
		{
			ErrorCode = 0x01 ;
		}
	}
}


void DefaultValue(void)
{
    
}

 /***			 @MCUMemoryLayout			***
 +------------------------------+ => Max APROM size: 0x00020000
 |															| 
 |					Data Flash					| => @Purpose: Store BankMeta[Center][Active] information of Bank1,2
 |															|	=> Size: 0x1800 ~= 6kb
 +------------------------------+	=> Data flash base addr: 0x0001E800
 |															|
 |					APROM Bank2					| => @Purpose: Store APP code
 |															|	=> Size: 0xE800 ~= 60kb
 +------------------------------+	=> Bank2 base addr: 0x00010000
 |															|
 |					APROM Bank1					| => @Purpose: Store APP code
 |															|	=> Size: 0xE000 ~= 57kb
 +------------------------------+	=> Bank1 base addr: 0x00002000
 |															|
 |					Bootloader					| => @Purpose: The initial code executed on startup. It is responsible for loading and verifying the application in APROM Bank 1 or 2.
 |															|	=> Size: 0x2000 ~= 8kb
 +------------------------------+	=> Bank1 base addr: 0x00000000

 @concern : Beacuse Host will send " @FIXED base " meter Fw, so it will be more, difficult when update thru center bank.	** 2025.08.05 **
						Resolved it with bin patcher in bootloader. ** 2025.09.11**
 ***/

int main()
{
    uint8_t i;

    AO2022_Center_1261_init();                
    SYS_UnlockReg();
	

    SYS_Init();
	
		WDT_Init();
	
		FMC_Open();

    GPIO_Mode_Init();

    DIR_HOST_RS485_In();
    ReadMyCenterID();	
	
    METER_PW1_On();
    METER_PW2_On();
    METER_PW3_On();
    METER_PW4_On();
    LED_G_On();
    LED_G_Off();
    LED_R_On();
    LED_R_Off();

    // For Meter
    UART0_Init();
    // For PC
    UART1_Init();
    //UART2_Init();
#if 0	
	for(i=0;i<10;i++)
		MeterTxBuffer[i] = i +0x30 ;
	
	_SendStringToMETER(MeterTxBuffer,10);
#endif	
    SoftI2cMasterInit();
    /*	
    TestI2C_RWEEPROM();
    	
    for(i=0;i<10;i++)
    	HostTxBuffer[i] = i +0x30 ;

    _SendStringToHOST(HostTxBuffer,10);
    */

    centerResetStatus = 1;
		ReadMyCenterID();
		
		//	Metadata Verification
		VerifyFW(centerResetStatus);		
		SendHost_CenterUpdateSuccsess();

		
		SysTick_Config(PLL_CLOCK/100);
    NVIC_EnableIRQ(SysTick_IRQn);

    /* Lock protected registers */
    SYS_LockReg();
    
    ReaderDeviceError = 0xFFFFFFFF ;
    MeterDeviceError = 0xFFFFFFFF ;
    MeterRelayStatus = 0xFFFFFFFF ;
    
    ReadMyCenterID();
    SystemTick = 0 ;	
    centerResetStatus = 1 ;
    DefaultValue();
    ReadMyCenterID();    
    DIR_HOST_RS485_In();
    MeterDeviceMax = MeterDeviceMaxTable[MyCenterID] ;

    u32TimeTick2=0; 	
    // Delay for System stable
    do {
        SystemTick = 0 ;
				WDT_RESET_COUNTER();	
    } while( u32TimeTick2 < 250 ); 

    SoftI2cMasterInit();	
    ReadMyCenterID();

    u32TimeTick2=0;  
    // Delay for System stable
    do {
    	SystemTick = 0;
			WDT_RESET_COUNTER();
    } while( u32TimeTick2 < 75 ); 

    SystemTick = 0 ;			
    fgFirstTimeCheckAC = 1 ;
    ReadMyCenterID();	
    MeterMemberIndex = 0 ;
    u32TimeTick2=0;   	
    MeterPollingState = PL_METER_NORM ;		
    NowPollingMtrBoard = 1 ;	    

    do
    {
				WDT_RESET_COUNTER();	// Feed Dog
        ReadMyCenterID();
        SystemTick = 0 ;								
        HostProcess();
        MeterProcess();
        SystemTick = 0 ;
        MeterBoardPolling();		               					
        RecoverSystemMoniter();
    }
    while(1);
}


void ReadMyCenterID(void)
{
	
    MyCenterID = 0 ;
    if ( PB2 ) { MyCenterID |= (0x01 << 0); }
    if ( PB3 ) { MyCenterID |= (0x01 << 1); }
    if ( PB4 ) { MyCenterID |= (0x01 << 2); }
    if ( PB5 ) { MyCenterID |= (0x01 << 3); }
    if ( PB6 ) { MyCenterID |= (0x01 << 4); }
    if ( PB7 ) { MyCenterID |= (0x01 << 5); }
    
    MeterDeviceMax = MeterDeviceMaxTable[MyCenterID] ;
    RoomMax  = RoomMaxTable[MyCenterID] ;
	
#if 0
		MyCenterID = 6 ;
		MeterDeviceMax = MeterDeviceMaxTable[MyCenterID] ;
		//RoomNumberMax = 6 ;
#endif
	
}

void RecoverSystemMoniter(void)
{
	if ( bRecoverSystem )
	{
		bRecoverSystem = 0 ;
		LED_G_On();
		ResetHostUART();
		ResetMeterUART();		
		LED_G_Off();
	}
}

void ResetHostUART(void)
{
	HOSTRxQ_cnt = 0 ; 
	HOSTRxQ_wp = 0 ;
	HOSTRxQ_rp = 0 ;
	HOSTTxQ_cnt = 0 ;
	HOSTTxQ_wp = 0 ;
	HOSTTxQ_rp = 0 ;
}
void ResetMeterUART(void)
{
	METERRxQ_wp = 0 ; 
	METERRxQ_rp = 0 ;
	METERRxQ_cnt = 0 ;
	METERTxQ_wp = 0 ; 
	METERTxQ_rp = 0 ;
	METERTxQ_cnt = 0 ;
}


/***
 * @brief Read RTC & Watering setup from Flash memory, and do watering task according to the setting
 * @note 	Make sure flash memory read/write enable 
 ***/
//void ScheduleWateringTask(void)
//{
//    SYS_UnlockReg();
//    FMC_Open();

//    uint32_t rtc_base, watering_base, data[2];
//		
//		//	Read RTC from flash
//    rtc_base = BANK_STATUS_BASE + sizeof(FwStatus);
//    ReadData(rtc_base, rtc_base + sizeof(RTC_Data), (uint32_t*)&data);
//    memcpy(CtrSystemTime, data, sizeof(RTC_Data));
//	
//		//	Read Watering setup from flash
//    watering_base = BANK_STATUS_BASE + sizeof(FwStatus) + sizeof(CtrSystemTime);
//    ReadData(watering_base, watering_base + sizeof(Watering_SetUp), (uint32_t*)&data);
//    memcpy(&Watering_SetUp, data, sizeof(Watering_SetUp));
//	
//    memcpy(&RTC_Data, CtrSystemTime, sizeof(RTC_Data));
//    
//    if (RTC_Data.hour == Watering_SetUp.Hour)
//    {
//        if ((RTC_Data.min >= Watering_SetUp.Min) &&
//            (RTC_Data.min <= (Watering_SetUp.Min + Watering_SetUp.Period_min)))
//        {
//            WATERING_SETTING_FLAG = WATERING_PWD;
//        }
//        else
//        {
//            WATERING_SETTING_FLAG = 0x00000000;
//        }
//    }
//    else
//    {
//        LED_G1_Off();
//    }
//    
//    FMC_Close();
//    SYS_LockReg();
//}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
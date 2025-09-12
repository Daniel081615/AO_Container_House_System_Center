#ifndef __OTA_MANAGER_H__
#define	__OTA_MANAGER_H__

#include "fmc.h"
#include "fmc_user.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

// Firmware flags
#define Fw_InvalidFlag          (1 << 0)
#define Fw_ValidFlag            (1 << 1)
#define Fw_PendingFlag          (1 << 2)
#define Fw_ActiveFlag           (1 << 3)
#define Fw_MeterFwFlag		 			(1 << 4)


/***
 *	@Reminder
 *	ReThink the status flags meaning.
 *	Maybe show the status of Fw running states
			
			1.	"One" Fw avalible
			2.	"Two" Fw available
			3.	No Fw available
			4.	Mtr OTA Update Processing
			5.	Ctr Updateing
	*	@Discussion 
			1.	Maybe Depart FWStatus.status (uint32_t) into status & Cmds (uint16_t + uint16_t) 
					typedef struct {
							uint32_t FW_Addr;
							uint32_t Meta_addr;
							uint16_t status;
							uint16_t cmd;
							uint32_t FWVersion;
					} FwStatus;
					
					i.	status 
						#define	SingleBank	0x0001
						#define	DualBank		0x0002
						#define	MtrOtaProcessing	0x0003 : "Ctr Fw + Mtr Fw"
						
					...	Flag that needs to write in FwStatus ...
					ii.	Cmds
						#define UPDATE_CENTER	0x0001
						#define UPDATE_METER	0x0002
						#define	NOCMD					0x00FF
			
 ***/
 
#define UPDATE_CENTER	0x0001
#define UPDATE_METER	0x0002
#define	NOCMD					0x00FF

#define OTA_UPDATE_FLAG          0xDDCCBBAA
#define FW_VERIFIED			 				 0x5AA5A55A

// Memory addresses
#define BANK1_BASE               0x00002000
#define BANK2_BASE               0x00010000
#define BANK1_META_BASE						0x0001F000
#define BANK2_META_BASE						0x0001F800
#define BANK_STATUS_BASE         0x0001E800


#define MTR_CMD_UPDATE_APROM      0xA0
#define MTR_CMD_UPDATE_CONFIG     0xA1
#define MTR_CMD_READ_CONFIG       0xA2
#define MTR_CMD_ERASE_ALL         0xA3
#define MTR_CMD_SYNC_PACKNO       0xA4
#define	MTR_CMD_UPDATE_METADATA		0xA5
#define MTR_CMD_GET_FWVER         0xA6
#define MTR_CMD_SEL_FW						0xA7	
#define MTR_CMD_RUN_APROM         0xAB
#define MTR_CMD_RUN_LDROM         0xAC
#define MTR_CMD_RESET             0xAD
#define MTR_CMD_CONNECT           0xAE
#define MTR_CMD_GET_DEVICEID      0xB1
#define MTR_CMD_RESEND_PACKET     0xFF

#define Center	0x00
#define Meter		0x01
#define Active	0x00
#define	Backup	0x01
#define MaxBankCnt	2

typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t fw_crc32;
    uint32_t fw_version;
    uint32_t fw_start_addr;
    uint32_t fw_size;
    uint32_t trial_counter;
    uint32_t WDTRst_counter;
    uint32_t meta_crc;
} FwMeta;


typedef struct {
    uint32_t FW_Addr;
    uint32_t Meta_addr;
    uint16_t status;
		uint16_t Cmd;
		uint32_t FWVersion;
} FwStatus;

extern int  WriteFwStatus(FwStatus *status);
extern int  WriteMetadata(FwMeta *meta, uint32_t MetaBase);
extern int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);
extern void BlinkLEDs();
extern void MarkFwFlag(bool mark);
extern void VerifyFW(bool ResetStatus);
extern void JumpToBootloader();

extern void Write_FwOtaCmd(uint16_t cmd);
extern void Write_FwStatus(uint16_t stat);

extern void Get_FwBankMetaInfo(FwStatus *ctx, FwMeta *active, FwMeta *backup);
extern void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);
extern bool FwCheck_CRC(FwMeta *meta);
extern uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

extern volatile FwStatus BankStatus[2];
extern volatile FwMeta BankMeta[2][MaxBankCnt];
extern _Bool	_bBankSwitchFlag;



#endif
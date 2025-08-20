#ifndef __OTA_MANAGER_H__
#define	__OTA_MANAGER_H__

#include "fmc.h"
#include "fmc_user.h"
#include "stdbool.h"
#include "string.h"
#include "stddef.h"

// Firmware flags
#define FW_FLAG_INVALID          (1 << 0)
#define FW_FLAG_VALID            (1 << 1)
#define FW_FLAG_PENDING          (1 << 2)
#define FW_FLAG_ACTIVE           (1 << 3)
#define THIS_IS_METER_FW_FLAG		 (1 << 4)

// OTA update flags
#define OTA_UPDATE_FLAG          0xDDCCBBAA
#define FW_VERIFIED			 				 0x5AA5A55A
#define SWITCH_FW_FLAG	         0xA5A5BEEF
#define REBOOT_FW_FLAG					 0x0A0AA0A0
#define	OTA_FAILED_FLAG					 0xDEADDEAD

// Memory addresses
#define FW1_BASE                 0x00002000
#define FW2_BASE                 0x00010000
#define FW_Status_Base           0x0001E800
#define METADATA_FW1_BASE        0x0001F000
#define METADATA_FW2_BASE        0x0001F800


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

typedef struct __attribute__((packed)) {
    uint32_t flags;
    uint32_t fw_crc32;
    uint32_t fw_version;
    uint32_t fw_start_addr;
    uint32_t fw_size;
    uint32_t trial_counter;
    uint32_t WDTRst_counter;
    uint32_t meta_crc;
} FWMetadata;


typedef struct {
    uint32_t FW_Addr;
    uint32_t FW_meta_Addr;
    uint32_t status;
		uint32_t FWVersion;
} FWstatus;

extern int  WriteFWstatus(FWstatus *status);
extern int  WriteMetadata(FWMetadata *meta, uint32_t meta_base);
extern int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);
extern void BlinkLEDs();
extern void MarkFwAsActive(bool mark);
extern void VerifyFW(bool ResetStatus);
extern void JumpToBootloader();
extern void WRITE_FW_STATUS_FLAG(uint32_t flag);
extern void ReadMetaInfo(FWstatus *ctx, FWMetadata *active, FWMetadata *backup);
extern void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);
extern bool boolFwcheck(void);
extern bool VerifyFirmware(FWMetadata *meta);
extern uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

extern FWstatus g_fw_ctx;
extern FWMetadata meta;
extern FWMetadata other;


extern FWstatus MeterMetaStatus;
extern FWMetadata MeterMetaActive;
extern FWMetadata MeterMetaBackup;

/***********************************************/
/******		OTA Host Functions and defines	******/
/***********************************************/

/* Center OTA Cmds */
enum CenterTokenOTA{
	CMD_CTR_OTA_UPDATE=0x40,			//Jump to Bootloader Cmd
	CMD_CTR_SWITCH_FWVER,
	CMD_GET_CTR_FW_STATUS,
	CMD_CTR_FW_REBOOT,
};

/*OTA Center Functions*/
void SendHost_CenterFWinfo(void);
void SendHost_MeterFWInfo(void);
void SendHost_CenterUpdateSuccsess(void);

/*OTA Meter Functions*/
void CTR_UPDATE_MTR_Process(void);
void Meter_RSP_FWInfo(void);
#endif
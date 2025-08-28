#include "ota_manager.h"
#include "AO_ExternFunc.h"

FWstatus g_fw_ctx = {0};
FWMetadata meta = {0};
FWMetadata other = {0};

FWstatus MeterMetaStatus = {0};
FWMetadata MeterMetaActive = {0};
FWMetadata MeterMetaBackup = {0};

int  WriteFWstatus(FWstatus *status);
int  WriteMetadata(FWMetadata *meta, uint32_t meta_base);
int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);
void BlinkLEDs();
void MarkFwAsActive(bool mark);
void VerifyFW(bool ResetStatus);
void JumpToBootloader();
void WRITE_FW_STATUS_FLAG(uint32_t flag);
void ReadMetaInfo(FWstatus *ctx, FWMetadata *active, FWMetadata *backup);
void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);
bool boolFwcheck(void);
bool VerifyFirmware(FWMetadata *meta);

bool g_fw_metadata_ready = false;

uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

void VerifyFW(bool ResetStatus)
{
		if (ResetStatus){
        bool bfwcheckresult = boolFwcheck();
        MarkFwAsActive(bfwcheckresult);
        if (!bfwcheckresult)
					JumpToBootloader();
    }
}

void ReadMetaInfo(FWstatus *ctx, FWMetadata *active, FWMetadata *backup)
{
    ReadData(FW_Status_Base, FW_Status_Base + sizeof(FWstatus), (uint32_t *)ctx);
    uint32_t addr[2] = { METADATA_FW1_BASE, METADATA_FW2_BASE };
    uint32_t idx = (ctx->FW_meta_Addr == METADATA_FW1_BASE) ? 0 : 1;
    ReadData(addr[idx], addr[idx] + sizeof(FWMetadata), (uint32_t *)active);
    ReadData(addr[1 - idx], addr[1 - idx] + sizeof(FWMetadata), (uint32_t *)backup);
}

bool boolFwcheck(void) 
{	
		ReadMetaInfo(&g_fw_ctx, &meta, &other);
    bool ok = VerifyFirmware(&meta);
    bool valid = (meta.flags == FW_FLAG_PENDING) || 
								((meta.flags & (FW_FLAG_ACTIVE | FW_FLAG_VALID)) == (FW_FLAG_ACTIVE | FW_FLAG_VALID)) ||
									meta.WDTRst_counter <  3;
		BlinkStatusLED( (valid&&ok) ? PD : PF, (valid&&ok) ? 7 : 2, 10, 1500);
    return ok && valid;
}

void MarkFwAsActive(bool mark) 
{		
    uint32_t other_addr = (g_fw_ctx.FW_meta_Addr == METADATA_FW1_BASE) ? METADATA_FW2_BASE : METADATA_FW1_BASE;
    if (mark) {
        meta.flags &= ~FW_FLAG_PENDING;
        meta.flags |= (FW_FLAG_VALID | FW_FLAG_ACTIVE);
				meta.trial_counter += 1;
        other.flags &= ~FW_FLAG_ACTIVE;
				WRITE_FW_STATUS_FLAG(FW_VERIFIED);
    }else if ( (g_fw_ctx.status == SWITCH_FW_FLAG) || (~mark) ) {
        other.flags &= ~FW_FLAG_PENDING;
        other.flags |= (FW_FLAG_VALID | FW_FLAG_ACTIVE);
        meta.flags &= ~FW_FLAG_ACTIVE;
				WRITE_FW_STATUS_FLAG(SWITCH_FW_FLAG);
    }else{
				WRITE_FW_STATUS_FLAG(OTA_FAILED_FLAG);
		}
		
    WriteMetadata(&meta, g_fw_ctx.FW_meta_Addr);
    WriteMetadata(&other, other_addr);	
}

void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms) 
{
    for (uint8_t i = 0; i < times; i++) {
        port->DOUT ^= (1 << pin);
        CLK_SysTickDelay(delay_ms * 1000);
    }
}

void BlinkLEDs()
{
    static uint8_t valid = 0;
    valid ^= 1;
    BlinkStatusLED(valid ? PD : PF, valid ? 7 : 2, 1, 2500);
}


void WRITE_FW_STATUS_FLAG(uint32_t flag) 
{	
		FMC_Open();
    g_fw_ctx.status = flag;
    WriteFWstatus(&g_fw_ctx);
}

bool VerifyFirmware(FWMetadata *meta) 
{	
    if ((meta->fw_start_addr == 0xFFFFFFFF) ||
        (meta->fw_size == 0xFFFFFFFF) ||
        (meta->fw_crc32 == 0xFFFFFFFF)) return false;
    uint32_t crc = CRC32_Calc((const uint8_t *)meta->fw_start_addr, meta->fw_size);
    return (crc == meta->fw_crc32);	
}

int WriteFWstatus(FWstatus *status) {
		return WriteToFlash(status, sizeof(FWstatus), FW_Status_Base, false, 0);
}

int WriteMetadata(FWMetadata *meta, uint32_t meta_base) {
		return WriteToFlash(meta, sizeof(FWMetadata), meta_base, true, offsetof(FWMetadata, meta_crc));
}

int WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset)
{
		FMC_Open();
    if (with_crc32) {
        uint32_t crc = CRC32_Calc((const uint8_t *)data, size - sizeof(uint32_t));
        *(uint32_t *)((uint8_t *)data + crc_offset) = crc;
    }
    
    if (FMC_Proc(FMC_ISPCMD_PAGE_ERASE, base_addr, base_addr + FMC_FLASH_PAGE_SIZE, 0) != 0) goto fail;
    if (FMC_Proc(FMC_ISPCMD_PROGRAM, base_addr, base_addr + size, (uint32_t *)data) != 0) goto fail;

    FMC_DISABLE_ISP();FMC_Close();
    return 0;

fail:
    FMC_DISABLE_ISP();FMC_Close();
    return -1;
		
}

void JumpToBootloader()
{
    WDT_Open(WDT_TIMEOUT_2POW18, WDT_RESET_DELAY_3CLK, TRUE, FALSE);
		WDT_RESET_COUNTER();
    WDT_CLEAR_RESET_FLAG();
	
    __disable_irq();
    SYS_UnlockReg();
    FMC_Open();
		
		WDT->CTL &= ~WDT_CTL_WDTEN_Msk;
    
		FMC_SetVectorPageAddr(FMC_APROM_BASE);
    NVIC_SystemReset();
}

uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) {
	
    uint32_t i, crc_result;
	
    CRC_Open(CRC_32, CRC_CHECKSUM_COM | CRC_CHECKSUM_RVS | CRC_WDATA_RVS, 0xFFFFFFFF, CRC_CPU_WDATA_32);
    for (i = 0; i + 4 <= len; i += 4) {
        uint32_t chunk;
        memcpy(&chunk, pData + i, 4);
        CRC->DAT = chunk;
    }
    if (i < len) {
        uint32_t lastChunk = 0xFFFFFFFF;
        memcpy(&lastChunk, pData + i, len - i);
        CRC->DAT = lastChunk;
    }

    crc_result = CRC_GetChecksum();
    return crc_result;
}
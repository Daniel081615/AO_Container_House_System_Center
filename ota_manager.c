#include "ota_manager.h"
#include "AO_ExternFunc.h"


int  WriteFwStatus(FwStatus *status);
int  WriteMetadata(FwMeta *meta, uint32_t MetaBase);
int  WriteToFlash(void *data, uint32_t size, uint32_t base_addr, bool with_crc32, uint32_t crc_offset);


void BlinkLEDs();
void MarkFwFlag(bool mark);
void VerifyFW(bool ResetStatus);
void JumpToBootloader();
void Write_FwOtaCmd(uint16_t cmd);
void Write_FwStatus(uint16_t stat);

void Get_FwBankMetaInfo(FwStatus *ctx, FwMeta *active, FwMeta *backup);
void BlinkStatusLED(GPIO_T *port, uint32_t pin, uint8_t times, uint32_t delay_ms);


bool FwCheck_CRC(FwMeta *meta);
void FwValidateCRC(void);
bool g_fw_metadata_ready = false;

uint32_t CRC32_Calc(const uint8_t *pData, uint32_t len) ;

volatile FwStatus BankStatus[2] = {0};
volatile FwMeta BankMeta[2][MaxBankCnt] = {0};
_Bool	_bBankSwitchFlag = 0;



void VerifyFW(bool ResetStatus)
{
		SYS_UnlockReg();
		FMC_ENABLE_ISP();
	
		if (ResetStatus){
        Get_FwBankMetaInfo((FwStatus *)&BankStatus[Center], (FwMeta *)&BankMeta[Center][Active], (FwMeta *)&BankMeta[Center][Backup]);
			
//        MarkFwFlag(b_FwValid);
//        if (!b_FwValid)
//					JumpToBootloader();
    }
		
		SYS_LockReg();
		
}

void Get_FwBankMetaInfo(FwStatus *ctx, FwMeta *active, FwMeta *backup)
{
		/**	Get Meta Infos	**/
    ReadData(BANK_STATUS_BASE, BANK_STATUS_BASE + sizeof(FwStatus), (uint32_t *)ctx);
    uint32_t addr[2] = {BANK1_META_BASE, BANK2_META_BASE};
    uint32_t idx = (ctx->Meta_addr == BANK1_META_BASE) ? 0 : 1;
    ReadData(addr[idx], addr[idx] + sizeof(FwMeta), (uint32_t *)active);
    ReadData(addr[1 - idx], addr[1 - idx] + sizeof(FwMeta), (uint32_t *)backup);	
}

void FwValidateCRC(void)
{
		/**	Validate Fw	**/
    bool ok = FwCheck_CRC((FwMeta *)&BankMeta[Center][Active]);
    bool valid = (BankMeta[Center][Active].flags == Fw_PendingFlag) || 
								((BankMeta[Center][Active].flags & (Fw_ActiveFlag | Fw_ValidFlag)) == (Fw_ActiveFlag | Fw_ValidFlag)) ||
									BankMeta[Center][Active].WDTRst_counter <  3;
		
		BlinkStatusLED( (valid&&ok) ? PD : PF, (valid&&ok) ? 7 : 2, 10, 1500);
		
		//	Mark fw flag
		MarkFwFlag(ok && valid);
		
		/**	Mark Bank status	**/
		if ((BankMeta[Center][Active].flags & valid) && 
				(BankMeta[Center][Backup].flags & valid))
		{
		}
}

/***
 *	@brief	Set Valid Fw valid flag, if no fw to switch -> Update Center Fw
 ***/
void MarkFwFlag(bool mark) 
{		
    uint32_t BackupBank_addr = (BankStatus[Center].Meta_addr == BANK1_META_BASE) ? BANK2_META_BASE : BANK1_META_BASE;
		//	Mark valid flags
    if (mark) 
		{	
        BankMeta[Center][Active].flags &= ~Fw_PendingFlag;
        BankMeta[Center][Active].flags |= (Fw_ValidFlag | Fw_ActiveFlag);
				BankMeta[Center][Active].trial_counter += 1;
        BankMeta[Center][Backup].flags &= ~Fw_ActiveFlag;
    }	
			else if ( _bBankSwitchFlag || (~mark)) {
				
				if ((BankMeta[Center][Backup].flags & Fw_MeterFwFlag) || 
						(BankMeta[Center][Backup].flags & Fw_InvalidFlag))
				{
						Write_FwOtaCmd(UPDATE_CENTER);
						JumpToBootloader();
				}
				
        BankMeta[Center][Backup].flags &= ~Fw_PendingFlag;
        BankMeta[Center][Backup].flags |= (Fw_ValidFlag | Fw_ActiveFlag);
        BankMeta[Center][Active].flags &= ~Fw_ActiveFlag;
    }
		
    WriteMetadata((FwMeta *)&BankMeta[Center][Active], BankStatus[Center].Meta_addr);
    WriteMetadata((FwMeta *)&BankMeta[Center][Backup], BackupBank_addr);	
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


void Write_FwOtaCmd(uint16_t cmd) 
{	
		SYS_UnlockReg();
		FMC_Open();
    BankStatus[Center].Cmd = cmd;
    WriteFwStatus((FwStatus *)&BankStatus[Center]);
		SYS_LockReg();
}

void Write_FwStatus(uint16_t stat)
{
		SYS_UnlockReg();
		FMC_Open();
    BankStatus[Center].status = stat;
    WriteFwStatus((FwStatus *)&BankStatus[Center]);
		SYS_LockReg();
}

bool FwCheck_CRC(FwMeta *meta) 
{	
    if ((meta->fw_start_addr == 0xFFFFFFFF) ||
        (meta->fw_size == 0xFFFFFFFF) ||
        (meta->fw_crc32 == 0xFFFFFFFF)) return false;
    uint32_t crc = CRC32_Calc((const uint8_t *)meta->fw_start_addr, meta->fw_size);
    return (crc == meta->fw_crc32);	
}

int WriteFwStatus(FwStatus *status) {
		return WriteToFlash(status, sizeof(FwStatus), BANK_STATUS_BASE, false, 0);
}

int WriteMetadata(FwMeta *meta, uint32_t MetaBase) {
		return WriteToFlash(meta, sizeof(FwMeta), MetaBase, true, offsetof(FwMeta, meta_crc));
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
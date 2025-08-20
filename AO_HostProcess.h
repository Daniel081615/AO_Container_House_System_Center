/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_NODE2_PROCESS_H__
#define __AO_NODE2_PROCESS_H__

#include "stdint.h"
#include "ota_manager.h"
#include "fmc_user.h"
#include "fmc.h"

extern void MeterSystemPolling(void);
extern void MeterBoardPolling(void);

extern void HostProcess(void);
extern void MeterTimeSync(void);
extern void MeterSaveMemberInfo(uint8_t MemberIndex);
extern void MeterSaveRoomInfo(void);
extern void MeterSendNull(void);
extern void SendHost_UserOpenPower(void);
extern void SendHost_UserClosePower(void);
extern void UserData2TXBuf(void);
extern void CalChecksum(void);
extern void SendHost_PowerCounter(void);
extern void SendHost_ValueUpdated(void);
extern void SendHost_BoxOpen(void);
extern void SendHost_MemberInfo(uint8_t UserIndex);
extern void SendHost_RoomInfo(void);
extern void SendHost_Register(void);
extern void MeterTurnOnRoomPower(void);
extern void MeterTurnOffRoomPower(void);
extern void SendHost_GetReocrd(void);
extern void SendHost_MeterValue(void);
extern void SendHost_CenterFWinfo(void);

extern void ReadMeterOtaCmdList();
extern void WriteMeterOtaCmdList(void);

extern uint8_t _SendStringToHOST(uint8_t *Str, uint8_t len);
extern void CalChecksumH(void);

extern uint8_t AckResult;

extern uint8_t g_packno;
extern uint8_t lcmd;




#endif

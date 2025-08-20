/*--------------------------------------------------------------------------- 
 * File Name     : MAIN.C
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2015.7.1
 * C Compiler : Keil C
 * -------------------------------------------------------------------------- 
*/
#ifndef __AO_NODE1_PROCESS_H__
#define __AO_NODE1_PROCESS_H__

extern uint8_t MeterPollingState;
extern void MeterProcess(void);
extern void SendMeter_UserInformation(uint8_t PacketIndex);


#endif

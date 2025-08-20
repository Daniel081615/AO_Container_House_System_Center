/*--------------------------------------------------------------------------------------- 
 * Company 		 : AOTECH
 * Author Name   : Barry Su
 * Starting Date : 2018.5.1
 * C Compiler : Keil C     
 * --------------------------------------------------------------------------------------

 H/W Define 
 UART0 => DBG 
 UART1 => Sensor Hub
 UART2 => RS485
 UART3 => RS232
 P1.08/09/10/14 => LED
 P1.16/17/18/19 => Button

 *------------------------------------------------------*/
#include "NUC1261.h"
#include "AO_MyDef.h"

extern void a_delay_us(uint8_t uSecDelay);
extern void SoftI2cMasterInit(void);
extern void SoftI2cMasterDeInit(void);
extern uint8_t soft_i2c_eeprom_read_byte(uint8_t deviceAddr, uint16_t readAddress) ;
extern _Bool soft_i2c_eeprom_write_byte(uint8_t deviceAddr, uint16_t writeAddress, uint8_t writeByte);



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
#include <stdio.h>
#include "NUC1261.h"
#include "AO_MyDef.h"
#include "AO_EE24C.h"

// PORTC4 & PORTC5
#define TWI_SDA_PIN     5
#define TWI_SCL_PIN      5

#define LOW 0
#define HIGH 1
#define false 0
#define true 1

#define I2C_DELAY_USEC	2
#define I2C_READ 1
#define I2C_WRITE 0

#define EE_SDA_DIR_Out()		PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE4_Pos) 
#define EE_SDA_DIR_In()			PF->MODE = (PF->MODE & (~GPIO_MODE_MODE4_Msk)) | (GPIO_MODE_INPUT << GPIO_MODE_MODE4_Pos);
#define EE_SCL_DIR_Out()			PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE3_Pos) 
#define EE_SCL_DIR_In()			PF->MODE = (PF->MODE & (~GPIO_MODE_MODE3_Msk)) | (GPIO_MODE_INPUT << GPIO_MODE_MODE3_Pos) 

#define EE_SDA_PIN_High()		(PF4 = 1)
#define EE_SDA_PIN_Low()		(PF4 = 0) 
#define EE_SCL_PIN_High()		(PF3 = 1)
#define EE_SCL_PIN_Low()		(PF3 = 0)
#define EE_WP_PIN_High()		__NOP()	
#define EE_WP_PIN_Low()			__NOP()
#define EE_SDA_is_High()			(PF4)

void a_delay_us(uint8_t uSecDelay)
{
    uint8_t i,j;

    for(i=0;i<uSecDelay;i++)
    {
        for(j=0;j<30;j++) 
	{	
		__NOP();
		__NOP();
        }
    }
}

// Initialize SCL/SDA pins and set the bus high
void SoftI2cMasterInit(void) {
	
	EE_SCL_DIR_Out();
	EE_SCL_PIN_High();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_High();
	
}

// De-initialize SCL/SDA pins and set the bus low
void SoftI2cMasterDeInit(void) 
{
	
	EE_SCL_DIR_In();
	EE_SCL_PIN_Low(); 
	EE_SDA_DIR_In();
	EE_SDA_PIN_Low();

}

// Read a byte from I2C and send Ack if more reads follow else Nak to terminate read
uint8_t SoftI2cMasterRead(uint8_t last) 
{
	uint8_t b = 0;
	uint8_t i;
	
	// Make sure pull-up enabled
	EE_SDA_PIN_High();
	EE_SDA_DIR_In();
	
	// Read byte
	for (i = 0; i < 8; i++) {
		// Don't change this loop unless you verify the change with a scope
		b <<= 1;
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_High();
		a_delay_us(I2C_DELAY_USEC);
		if ( EE_SDA_is_High() )	b |= 1 ;
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_Low();
	}
	// Send Ack or Nak
	EE_SDA_DIR_Out();
	if (last) { 
		EE_SDA_PIN_High();
	} else { 
		EE_SDA_PIN_Low();
	}  
	EE_SCL_PIN_High();
	a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_Low();
	EE_SDA_PIN_Low();
	
	return b;
}

// Write a byte to I2C
_Bool SoftI2cMasterWrite(uint8_t data) 
{
	uint8_t m;
	uint8_t rtn;
	
	EE_SDA_DIR_Out();
	// Write byte
	for (m = 0x80; m != 0; m >>= 1) 
	{
		// Don't change this loop unless you verify the change with a scope
		a_delay_us(I2C_DELAY_USEC);
		if (m & data) 
		{ 
			EE_SDA_PIN_High();
		} else { 
			EE_SDA_PIN_Low();
		}
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_High();
		a_delay_us(I2C_DELAY_USEC);
		EE_SCL_PIN_Low();
		
	}      
	// get Ack or Nak
	// Enable pullup
	EE_SDA_PIN_High();
	EE_SDA_DIR_In();
	EE_SCL_PIN_High();
    a_delay_us(I2C_DELAY_USEC);
	rtn = EE_SDA_is_High() ;
	EE_SCL_PIN_Low();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	
	return rtn == 0;
}

// Issue a start condition
_Bool SoftI2cMasterStart(uint8_t addressRW) 
{
 
	EE_SCL_DIR_Out();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();
    a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_Low();
    a_delay_us(I2C_DELAY_USEC);
	
    return SoftI2cMasterWrite(addressRW);
}

// Issue a restart condition
_Bool SoftI2cMasterRestart(uint8_t addressRW) 
{
	
	EE_SCL_DIR_Out();
	EE_SDA_DIR_Out();
	EE_SDA_PIN_High();
	EE_SCL_PIN_High();
    a_delay_us(I2C_DELAY_USEC);
	
    return SoftI2cMasterStart(addressRW);
}

// Issue a stop condition
void SoftI2cMasterStop(void) 
{
	EE_SDA_DIR_Out();
	EE_SDA_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	EE_SCL_PIN_High();
	a_delay_us(I2C_DELAY_USEC);
	EE_SDA_PIN_High();
	a_delay_us(I2C_DELAY_USEC);
}

// Read 1 byte from the EEPROM device and return it
uint8_t soft_i2c_eeprom_read_byte(uint8_t deviceAddr, uint16_t readAddress) {
	uint8_t byteRead = 0;
	
	// Issue a start condition, send device address and write direction bit
	if (!SoftI2cMasterStart((deviceAddr<<1) | I2C_WRITE)) return false;
       a_delay_us(I2C_DELAY_USEC);
	// Send the address to read, 8 bit or 16 bit
	//if (readAddress > 255) {
		if (!SoftI2cMasterWrite((readAddress >> 8))) return false; // MSB
		if (!SoftI2cMasterWrite((readAddress & 0xFF))) return false; // LSB
	//}
	//else {
	//	if (!SoftI2cMasterWrite(readAddress)) return false; // 8 bit
	//}

	// Issue a repeated start condition, send device address and read direction bit
	if (!SoftI2cMasterRestart((deviceAddr<<1) | I2C_READ)) return false;
	a_delay_us(I2C_DELAY_USEC);
	// Read the byte
	byteRead = SoftI2cMasterRead(1);

	// Issue a stop condition
	SoftI2cMasterStop();
   	
	return byteRead;
}

// Write 1 byte to the EEPROM
_Bool soft_i2c_eeprom_write_byte(unsigned char deviceAddr, uint16_t writeAddress, unsigned char writeByte) 
{
	unsigned char j;
	// WP = Low (Disable Write Protection)
	EE_WP_PIN_Low();
	a_delay_us(I2C_DELAY_USEC);
	// Issue a start condition, send device address and write direction bit
	if (!SoftI2cMasterStart((deviceAddr<<1) | I2C_WRITE)) return false;
        a_delay_us(I2C_DELAY_USEC);
	// Send the address to write, 8 bit or 16 bit
	//if ( writeAddress > 255) {
		if (!SoftI2cMasterWrite((writeAddress >> 8))) return false; // MSB
		if (!SoftI2cMasterWrite((writeAddress & 0xFF))) return false; // LSB
	//}
	//else {
	//	if (!SoftI2cMasterWrite(writeAddress)) return false; // 8 bit
	//}

	// Write the byte
	if (!SoftI2cMasterWrite(writeByte)) return false;

	// Issue a stop condition
	SoftI2cMasterStop();
	
	// Delay 10ms for the write to complete, depends on the EEPROM you use
	//_delay_ms(10);
	for ( j=0;j<10;j++) 
	{
		a_delay_us(100); 
	}
	// WP = HIGH (Enable Write Protection)
	EE_WP_PIN_High();
	
	return true;
}

// Read more than 1 byte from a device
// (Optional)
/*bool I2cReadBytes(uint8_t deviceAddr, uint8_t readAddress, byte *readBuffer, uint8_t bytestoRead) {
	// Issue a start condition, send device address and write direction bit
	if (!SoftI2cMasterStart((deviceAddr<<1) | I2C_WRITE)) return false;

	// Send the address to read
	if (!SoftI2cMasterWrite(readAddress)) return false;

	// Issue a repeated start condition, send device address and read direction bit
	if (!SoftI2cMasterRestart((deviceAddr<<1) | I2C_READ)) return false;

	// Read data from the device
	for (uint8_t i = 0; i < bytestoRead; i++) {
		// Send Ack until last byte then send Ack
		readBuffer[i] = SoftI2cMasterRead(i == (bytestoRead-1));
	}

	// Issue a stop condition
	SoftI2cMasterStop();
	
	return true;
}*/


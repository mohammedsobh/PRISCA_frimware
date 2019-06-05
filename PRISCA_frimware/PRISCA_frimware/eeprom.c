/*
 * eeprom.c
 *
 * Created : 6/5/2019 4:44:43 AM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */
#include <avr/io.h>
#include <stdlib.h>
#include "eeprom.h"
#include <avr/eeprom.h>
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address and data registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMWE */
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEWE */
	EECR |= (1<<EEWE);
}
void EEPROM_WriteNBytes(uint16_t v_eepromAddress_u16, char *ptr_ramAddress_u8, uint16_t v_numOfBytes_u16)
{
	while(v_numOfBytes_u16 !=  0)
	{
		EEPROM_write(v_eepromAddress_u16,*ptr_ramAddress_u8); //Write a byte from RAM to EEPROM
		v_eepromAddress_u16++;					   //Increment the Eeprom Address
		ptr_ramAddress_u8++;						   //Increment the RAM Address
		v_numOfBytes_u16--;					   //Decrement NoOfBytes after writing each Byte
	}
}
unsigned char EEPROM_read(unsigned int uiAddress)
{
	{
		/* Wait for completion of previous write */
		while(EECR & (1<<EEWE));
		/* Set up address register */
		EEAR = uiAddress;
		/* Start eeprom read by writing EERE */
		EECR |= (1<<EERE);
		/* Return data from data register */
		return EEDR;	
	}
}
void EEPROM_ReadNBytes(uint16_t v_eepromAddress_16, char *ptr_ramAddress_u8, uint16_t v_numOfBytes_u16)
{
	while(v_numOfBytes_u16 !=  0)
	{
		*ptr_ramAddress_u8 = EEPROM_read(v_eepromAddress_16);//Read a byte from EEPROM to RAM
		v_eepromAddress_16++;						//Increment the EEPROM Address
		ptr_ramAddress_u8++;							//Increment the RAM Address
		v_numOfBytes_u16--;						//Decrement NoOfBytes after Reading each Byte

	}
}
/*
 * eeprom.h
 *
 * Created : 6/5/2019 4:44:43 AM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */
#ifndef EEPROM_H_
#define EEPROM_H_
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
void EEPROM_WriteNBytes(uint16_t v_eepromAddress_u16, char *ptr_ramAddress_u8, uint16_t v_numOfBytes_u16);
unsigned char EEPROM_read(unsigned int uiAddress);
void EEPROM_ReadNBytes(uint16_t v_eepromAddress_16, char *ptr_ramAddress_u8, uint16_t v_numOfBytes_u16);
#endif /* EEPROM_H_ */
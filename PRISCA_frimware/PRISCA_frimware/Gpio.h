
/*
 * Gpio.h
 *
 * Created: 02/06/2018 04:28:17 ã
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */

#ifndef GPIO_H_
#define GPIO_H_

#define A  0x39
#define B  0x36
#define C  0x33
#define D  0x30
#define H  0xff
#include <stdbool.h>
#define input  1
#define output 0

#define set_bin(reg,bit)   ( (*(volatile char*) reg) |= (1<<bit))
#define reset_bin(reg,bit)  ( (*(volatile char*) reg) &=~ (1<<bit))
#define XOR(reg,bit)  ( (*(volatile char*) reg) ^= (1<<bit))

char Casting (int reg);
void pin_direction (char base, char bin, bool state );
char pin_Read(char base,char bin);
void pin_write (char base, char bin ,bool p );
void port_direction (char base, bool state );
char port_Read(char base);
void port_write (char base, bool p );
void pin_XOR (char base, char bin  );
#endif



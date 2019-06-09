/*
 * getTemperture.h
 *
 * Created : 01/06/2019 12:23:00 AM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */
#ifndef GETTEMPERTURE_H_
#define GETTEMPERTURE_H_
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#define T0 0X50	//ADC2
#define T1 0X51 //ADC1
#define T2 0X52	//ADC0
// #define T3 0X53
// #define T4 0X54
// #define T5 0X55
// #define T6 0X56
// #define T7 0X57
int ADC_value(uint8_t ADC_pin);
double getTemp(uint8_t ADC_pin);
#endif /* GETTEMPERTURE_H_ */
/*
 * getTemperture.c
 *
 * Created : 01/06/2019 12:23:00 AM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */
#include "getTemperture.h"

//! Parameters for thermistor
/*************************************************************************/
int Vo;
float R1 = 10000;
float logR2, R2, T, Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
/**************************************************************************/
int ADC_value(uint8_t ADC_pin)
{
	ADMUX = ADC_pin; // reset MUX0 to select ADC0
	ADCSRA |= (1 << ADSC);		  //active reading
	while(ADCSRA && (1<<ADSC)==0);	 // wait the A/D to complete reading and converting
	ADCSRA |=(1<<ADIF);
	return ADC;                // the output of lm35 is vt = T/100 => T = vt*100 ==>so the value = (the analoge volt)*0.00488*100
}
/*! \brief Resets the integrator.
 *	from the data sheet of ntc thermistor the output temperature is calculated 
 *  by these way
 */
double getTemp(uint8_t ADC_pin)
{
	Vo = ADC_value(ADC_pin);
	R2 = R1 * ((1023.0 / (float)Vo) - 1.0);
	logR2 = log(R2);
	T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // the temperature in kelvin
	Tc = T - 273.15;									// the temperature in c
	return (Tc);
}

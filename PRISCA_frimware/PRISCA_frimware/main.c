/*
 * PRISCA_frimware.c
 *
 * Created : 4/23/2019 6:42:43 PM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // library for boolean variable
#include <avr/interrupt.h>
#include "Include.h"
#include "lcd.h"
//! \xref item todo "Todo" "Todo list"
#define K_P 1.00
//! \xref item todo "Todo" "Todo list"
#define K_I 0.00
//! \xref item todo "Todo" "Todo list"
#define K_D 0.00
#define TIME_INTERVAL 157
void Init(void);// initial library to set up the external devices
int ADC_value(uint8_t ADC_pin);// ADC setup
double getTemp(uint8_t ADC_pin);
struct GLOBAL_FLAGS {
	//! True when PID control loop should run one time
	uint8_t pidTimer : 1;
	uint8_t dummy : 7;
	} gFlags = {0, 0};
//! Parameters for regulator
struct PID_DATA pidData;
double x;
double y;
double z;
double F;	//speed of x y z motors
double Fe; //speed of extruder
double SE1,SE2,SB;
double E;
int Vo;
float R1 = 10000;
float logR2, R2, T, Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
char String[80] ; // variable to storage the value of converted integer in it
char output[10];
uint8_t d = 100; //no. of steps per 1mm
volatile int value; // to storage the instantaneous change in ADC converter
int main(void)
{
    Init();
	/*static variables that initialize only once */
	static double old_val_1 = 0;
	static double old_val_2 = 0;
	static double old_val_3 = 0;
	//three variables that contain numbers that convert from string/
	double value_1;
	double value_2;
	double value_3;
	int val = 0;
	while (1)
	{
				 		

		for (int i = 0 ; i < 80 ;i++)
		{
			String [i] = 0;
		}
		Recive_Data(String);
		if (String[0] == 'M')
		{
			val = get_int (String,'M');
			if (val == 105)
			{
				Transmit_Data("ok");
			}
			if (val == 104)
			{
				if (get_int (String,'T') == 0)
				{
					SE1 = get_value(String,'S');
					Transmit_Data("ok");
				}
				else if (get_int (String,'T') == 1)
				{
					SE2 = get_value(String,'S');
					Transmit_Data("ok");
				} 
				else
					Transmit_Data("error extruder temperature");
			}
			if (val == 109)
			{
				while(pid_Controller(SE1,getTemp(T0), &pidData) >= 0 && pid_Controller(SE2,getTemp(T2), &pidData) >= 0 );
				Transmit_Data("ok");
			}
			if (val == 140)
			{
				SB = get_value(String,'S');
				Transmit_Data("ok");
			}
			if (val == 190)
			{
				while(pid_Controller(SB,getTemp(T4), &pidData) >= 0 );
				Transmit_Data("ok");
			}
			if (val == 106)
			{
				OCR2 = get_value(String,'S');
				Transmit_Data("ok");
			}
		}
		else if (String[0] == 'G')
		{
			val = get_int (String,'G');
			if (val == '1')
			{
				value_1 = get_value(String,'X');           //extract first value
				value_2 = get_value(String,'Y'); //call function to extract second value
				value_3 = get_value(String,'Z'); //call function to extract third value
				E = get_value(String,'E');
				if ((E && value_1 && value_2 && value_3) == 0)
				{
					if (get_value(String,'F') != 0)
					{
						Fe = get_value(String,'F');
					}
				} 
				else
				{
					if (get_value(String,'F') != 0)
					{
						F = get_value(String,'F');
					}
				}
				if (!(value_1>200||value_2>200||value_3>500)) // if the values don't skip the plate ,use it

				{
					x = sub_function (&old_val_1, value_1);     //call function to extract first step
					y = sub_function (&old_val_2, value_2);    //call function to extract second step
					z = sub_function (&old_val_3, value_3);    //call function to extract third step
					value_1 = 0;
					value_2 = 0;
					value_3 = 0;
					while (!(x && y && z == 0));
					Transmit_Data("ok");
				}
				}
				if (val == '0')
				{
					value_1 = get_value(String,'X');           //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					value_3 = get_value(String,'Z'); //call function to extract third value
					E = get_value(String,'E');
					if ((E && value_1 && value_2 && value_3) == 0)
					{
						if (get_value(String,'F') != 0)
						{
							F = get_value(String,'F');
						}
					}
					if (!(value_1>107||value_2>107||value_3>500||value_1<-107||value_2<-107||value_3<0)) // if the values don't skip the plate ,use it

					{
						x = sub_function (&old_val_1, value_1);     //call function to extract first step
						y = sub_function (&old_val_2, value_2);    //call function to extract second step
						z = sub_function (&old_val_3, value_3);    //call function to extract third step
						value_1 = 0;
						value_2 = 0;
						value_3 = 0;
						while (!(x && y && z == 0));
						Transmit_Data("ok");
					}
				else
				Transmit_Data("error");
			}
		}
	}
}
void Init(void){
	
	DDRD |= (1<<PD4)|(1<<PD5)|(1<<PD7);
	DDRC = 0XFF;
	ADCSRA = 0x87; //to active A/D pins
	ADMUX |= (1<<REFS0);// external reference volt is selected
	UART_INIT();
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR, K_D * SCALING_FACTOR, &pidData);
	TCNT0 = 0;
	TCCR0 |= (1<<CS02) | (1<<CS00); // PRESCALER 1024
	/*set fast PWM mode with non-inverted output*/
	TCCR0 |= (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS00);
	TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1<<WGM10);
	TCCR1B |= (1<<CS10);
	TCCR2 |= (1<<WGM20) | (1<<WGM21) | (1<<COM21) | (1<<CS20);
	DDRB|=(1<<PB3);  /*set OC0 pin as output*/
	TIMSK = (1<<TOIE0);
	sei();
}
int ADC_value(uint8_t ADC_pin)
{
	ADMUX = ADC_pin; // reset MUX0 to select ADC0
	ADCSRA |= (1 << ADSC);		  //active reading
	while(ADCSRA && (1<<ADSC)==0);	 // wait the A/D to complete reading and converting
	ADCSRA |=(1<<ADIF);
	return ADC;                // the output of lm35 is vt = T/100 => T = vt*100 ==>so the value = (the analoge volt)*0.00488*100
}
double getTemp(uint8_t ADC_pin)
{
	Vo = ADC_value(ADC_pin);
	R2 = R1 * ((1023.0 / (float)Vo) - 1.0);
	logR2 = log(R2);
	T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
	Tc = T - 273.15;
	return (Tc);
}
ISR(TIMER0_OVF_vect)
{
	static uint16_t i = 0;

	if (i < TIME_INTERVAL) 
	{
		i++;
		} else {
		gFlags.pidTimer = 1;
		i               = 0;
	}
	if (gFlags.pidTimer == 1) 
	{
		OCR0 =	255 - pid_Controller(SE1,getTemp(T0), &pidData);
		OCR1A = 255 - pid_Controller(SE2,getTemp(T2), &pidData);
		OCR1B = 255 - pid_Controller(SB,getTemp(T3), &pidData);
		gFlags.pidTimer = FALSE;
	}
	
}
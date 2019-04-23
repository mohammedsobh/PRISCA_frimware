/*
 * PRISCA_frimware.c
 *
 * Created: 4/23/2019 6:42:43 PM
 * Author : Mohammed sobh
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // library for boolean variable
/* FreeRTOS files. */
#include "Include.h"
/* Define all the tasks */
void Init(void);// initial library to set up the external devices
int ADC_value();// ADC setup
double getTemp();
double x;
double y;
double z;
double F;	//speed of x y z motors
double Fe; //speed of extruder
double S;
double E;
int Vo;
double R1 = 10000.00;
double logR2, R2, T;
double c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
char String[80] ; // variable to storage the value of converted integer in it
bool status; // to control the start and stop button
uint8_t d = 100; //no. of steps per 1mm
volatile int value; // to storage the instantaneous change in ADC converter

int main(void)
{
    Init();
	int i = 1 ,j =0;
	/*static variables that initialize only once */
	static double old_val_1 = 0;
	static double old_val_2 = 0;
	static double old_val_3 = 0;
	//three variables that contain numbers that convert from string/
	double value_1;
	double value_2;
	double value_3;
	int val = 0;
	char arr[5];
	while (1)
	{
		for (int i = 0 ; i < 80 ;i++)
		{
			String [i] = 0;
		}
		for (int i = 0 ; i < 5 ;i++)
		{
			arr [i] = 0;
		}
		Recive_Data(String);
		i = 1;
		if (String[0] == 'M')
		{
			while(String[i] != ';')
			{
				arr[j] = String[i];
				j += 1;
				i += 1;
			}
			val = atoi(arr);
			j = 0;
			if (val == 105)
			{
				Transmit_Data("ok");
			}
			if (val == 104)
			{
				S = S_value(String);
				Transmit_Data("ok");
			}
			if (val == 109)
			{
				while(getTemp() != S);
				Transmit_Data("ok");
			}
		}
		else if (String[0] == 'G')
		{
			while(String[i] != ' ')
			{
				arr[j] = String[i];
				j += 1;
				i += 1;
			}
			val = atoi(arr);
			j = 0;
			if (val == '1')
			{
				value_1 = X_value(String);           //extract first value
				value_2 = Y_value (String); //call function to extract second value
				value_3 = Z_value (String); //call function to extract third value
				E = E_value(String);
				if ((E && value_1 && value_2 && value_3) == 0)
				{
					if (F_value(String) != 0)
					{
						Fe = F_value(String);
					}
				} 
				else
				{
					if (F_value(String) != 0)
					{
						F = F_value(String);
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
				}
				if (val == '0')
				{
					value_1 = X_value(String);           //extract first value
					value_2 = Y_value (String); //call function to extract second value
					value_3 = Z_value (String); //call function to extract third value
					if (F_value(String) != 0)
					{
						F = F_value(String);
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
	ADCSRA |= 0x87; //to active A/D pins
	ADMUX |= (1<<REFS0);// external reference volt is selected
	DDRA &= ~(0<<PA0);// use PA0,1 as input for thermistor 
	UART_INIT();
}
int ADC_value(void)
{
	ADMUX &= ~(1 << MUX0); // reset MUX0 to select ADC0
	ADCSRA |= (1 << ADSC);		  //active reading
	while(ADCSRA && (1<<ADSC)==0);	 // wait the A/D to complete reading and converting
	ADCSRA |=(1<<ADIF);
	value=ADC*(4.88E-3);		 // to convert D/A (the analoge volt) * (5/1024) = (the analoge volt)*0.00488
	return ADC;                // the output of lm35 is vt = T/100 => T = vt*100 ==>so the value = (the analoge volt)*0.00488*100
}
double getTemp()
{
	Vo = ADC_value();
	R2 = R1 * (1023.0 / (float)Vo - 1.0);
	logR2 = log(R2);
	T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
	T = T - 273.15;
	return (T);
}
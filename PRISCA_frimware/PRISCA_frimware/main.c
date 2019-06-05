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
#include <math.h>
#include "Include.h"
//! \xref item todo "Todo" "Todo list"
#define K_P 90.00
//! \xref item todo "Todo" "Todo list"
#define K_I 30.00
//! \xref item todo "Todo" "Todo list"
#define K_D 80.00
#define TIME_INTERVAL 157
#define  XSTEP_PER_mm_address	0x00
#define  YSTEP_PER_mm_address	0x05
#define  ZSTEP_PER_mm_address	0x0A
#define  ESTEP_PER_mm_address	0x0F
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
int Vo;
float R1 = 10000;
float logR2, R2, T, Tc;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
char String[80] ; // variable to storage the value of converted integer in it
char nois[80];
char X_pos[10],Y_pos[10],Z_pos[10],E_pos[10];
char TE[10],TB[10];
volatile int value; // to storage the instantaneous change in ADC converter
bool status ;
bool BED_Activ = 0;
bool XEN_DES = 0;
bool YEN_DES = 0;
bool ZEN_DES = 0;
bool EEN_DES = 0;
bool homeSet = 0;
bool extrud = 0;
bool Fextrud = 0;
double Xspm,Yspm,Zspm,Espm,
FXspm = 100.00,FYspm = 100.00,FZspm = 100.00,FEspm = 140.00;
double SE0 = 0.0;
double F;	//speed of x y z motors
double Fe; //speed of extruder
double SB;
double x;
double y;
double z;
double EX;
double B;
double I;
double J;
double R;
long wait;
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
	EEPROM_ReadNBytes(XSTEP_PER_mm_address,X_pos,5);
	EEPROM_ReadNBytes(YSTEP_PER_mm_address,Y_pos,5);
	EEPROM_ReadNBytes(ZSTEP_PER_mm_address,Z_pos,5);
	EEPROM_ReadNBytes(ESTEP_PER_mm_address,E_pos,5);
	Xspm = atof(X_pos);
	Yspm = atof(Y_pos);
	Zspm = atof(Z_pos);
	Espm = atof(E_pos);
	while (1)
	{
		if (status == 0)
		{
			for (int i = 0 ; i < 80 ;i++)
			{
		 		String [i] = 0;
			}
			Recive_Data(String);
		}
		if (String[0] == 'M')
		{
			val = get_int (String,'M');
			switch (val)
			{
				case 0: case 1:
					x = 0;
					y = 0;
					z = 0;
					EX = 0;
					F = 0;
					Fe = 0;
					SE0 = 0.0;
					SB = 0.0;
					Transmit_Data("ok");
					break;
				case 17:
					motor_init();
					Transmit_Data("ok");
					break;
				case 18: case 84:
					wait = (get_int(String ,'S')*1000);
					status = 1;
					XEN_DES = find(String ,'X');
					YEN_DES = find(String ,'Y');
					ZEN_DES = find(String ,'Z');
					EEN_DES = find(String ,'E');
					motor_EN_DES('X',XEN_DES);
					motor_EN_DES('Y',YEN_DES);
					motor_EN_DES('Z',YEN_DES);
					motor_EN_DES('E',EEN_DES);
					if(wait != 0)
					{
						while(wait > 0)
						{
							_delay_ms(1);
							wait --;
						}
						motor_init();
					}
					status = 0;
					Transmit_Data("ok");
					break;
				case 82:
					EX = 0;
					Transmit_Data("ok");
					break;
				case 83:
					old_val_1 = 0;
					old_val_2 = 0;
					old_val_3 = 0;
					x = 0;
					y = 0;
					z = 0;
					Transmit_Data("ok");
					break;
				case 92:case 500:case 502:
					for (int i = 0 ; i < 10 ; i ++)
					{
						X_pos[i] = 0;
						Y_pos[i] = 0;
						Z_pos[i] = 0;
						E_pos[i] = 0;
					}
					if (val == 92)
					{
						Xspm = get_value(String,'X');
						Yspm = get_value(String,'Y');
						Zspm = get_value(String,'Z');
						Espm = get_value(String,'E');
					}
					if (val == 502)
					{
						Xspm = FXspm;
						Yspm = FYspm;
						Zspm = FZspm;
						Espm = FEspm;
					}
					sprintf(X_pos,dtostrf(Xspm, 2,3,"%f"));
					sprintf(Y_pos,dtostrf(Yspm, 2,3,"%f"));
					sprintf(Z_pos,dtostrf(Zspm, 2,3,"%f"));
					sprintf(E_pos,dtostrf(Espm, 2,3,"%f"));
					EEPROM_WriteNBytes(XSTEP_PER_mm_address,X_pos,5); //write 2-bytes of data(Xspm) at 0x00.
					EEPROM_WriteNBytes(YSTEP_PER_mm_address,Y_pos,5); //write 2-bytes of data(Yspm) at 0x02.
					EEPROM_WriteNBytes(ZSTEP_PER_mm_address,Z_pos,5); //write 2-bytes of data(Zspm) at 0x04.
					EEPROM_WriteNBytes(ESTEP_PER_mm_address,E_pos,5); //write 2-bytes of data(E0spm) at 0x06.
					Transmit_Data("ok");
					break;
				case 104:
					SE0 = get_value(String,'S');
					Transmit_Data("ok");
					break;
				case 105:
					for (int i = 0 ; i < 10 ; i ++)
					{
						TE[i] = 0;
						TB[i] = 0;
					}
					sprintf(TE,dtostrf(getTemp(T0), 2,3,"%f"));
					sprintf(TB,dtostrf(getTemp(T2), 2,3,"%f"));
					if (( BED_Activ))
					{
						char TEMP[20]={'T',':',
							TE[0],TE[1],TE[2],TE[3],TE[4],
							' ','E',':','0',' ',
						'B',':',TB[0],TB[1],TB[2],TB[3],TB[4],'\r'};
						Transmit_Data(TEMP);
					} 
					else
					{
						char TEMP[12]={'T',':',
							TE[0],TE[1],TE[2],TE[3],TE[4],
						' ','E',':','0','\r'};
						Transmit_Data(TEMP);
					} 
					break;
				case 106:
					OCR2 = get_value(String,'S');
					Transmit_Data("ok");
					break;
				case 107:
					OCR2 = 0;
					Transmit_Data("ok");
					break;
				case 109:
					B = get_value(String,'B');
					if (B == 0)
					{
						SE0 = get_value(String,'S');
						status = 1;
						while(getTemp(T0) < SE0);
						status = 0;
					} 
					else
					{
						status = 1;
						while(getTemp(T0) < B);
						status = 0;
					}
					
					Transmit_Data("ok");
					break;
				case 112:
					value_1 = 0;  //extract first value
					value_2 = 0; //call function to extract second value
					value_3 = 0; //call function to extract third value
					x = sub_function (&old_val_1, value_1);     //call function to extract first step
					y = sub_function (&old_val_2, value_2);    //call function to extract second step
					z = sub_function (&old_val_3, value_3);    //call function to extract third step
					EX = 0;
					F = 0;
					Fe = 0;
					SE0 = 0;
					SB = 0;
					status = 1;
					double step[5]={x*Xspm,y*Yspm,z*Zspm,0};
					motor_movement(step,F,Fe);
					status = 0;
					Transmit_Data("ok");
					break;
				case 114:
					for (int i = 0 ; i < 10 ; i ++)
					{
						X_pos[i] = 0;
						Y_pos[i] = 0;
						Z_pos[i] = 0;
					}
					sprintf(X_pos,dtostrf(old_val_1, 2,3,"%f"));
					sprintf(Y_pos,dtostrf(old_val_2, 2,3,"%f"));
					sprintf(Z_pos,dtostrf(old_val_3, 2,3,"%f"));
					char pos[21]={'X',
						X_pos[0],X_pos[1],X_pos[2],X_pos[3],X_pos[4],
						' ','Y',
						Y_pos[0],Y_pos[1],Y_pos[2],Y_pos[3],Y_pos[4],
						' ','Z',Z_pos[0],Z_pos[1],Z_pos[2],Z_pos[3],Z_pos[4],'\r'};
					Transmit_Data(pos);
					break;
				case 140:
					BED_Activ = 1;
					SB = get_value(String,'S');
					Transmit_Data("ok");
					break;
				case 190:
					SB = get_value(String,'S');
					status = 1;
					while(getTemp(T2) < SB);
					status = 0;
					Transmit_Data("ok");
					break;
				case 206:
					old_val_1 = 0;
					old_val_2 = 0;
					old_val_3 = 0;
					homeSet = 1;
					Transmit_Data("ok");
					break;
				case 302:
					SE0 = get_value(String,'S');
					if (SE0 <= 25 )
					{
						Fextrud = 1;
					}
				case 501:
					EEPROM_ReadNBytes(XSTEP_PER_mm_address,X_pos,5);
					EEPROM_ReadNBytes(YSTEP_PER_mm_address,Y_pos,5);
					EEPROM_ReadNBytes(ZSTEP_PER_mm_address,Z_pos,5);
					EEPROM_ReadNBytes(ESTEP_PER_mm_address,E_pos,5);
					Xspm = atof(X_pos);
					Yspm = atof(Y_pos);
					Zspm = atof(Z_pos);
					Espm = atof(E_pos);
					Transmit_Data("ok");
					break;
				case 503:
					for (int i = 0 ; i < 10 ; i ++)
					{
						X_pos[i] = 0;
						Y_pos[i] = 0;
						Z_pos[i] = 0;
						E_pos[i] = 0;
					}
					sprintf(X_pos,dtostrf(Xspm, 2,3,"%f"));
					sprintf(Y_pos,dtostrf(Yspm, 2,3,"%f"));
					sprintf(Z_pos,dtostrf(Zspm, 2,3,"%f"));
					sprintf(E_pos,dtostrf(Espm, 2,3,"%f"));
					char acc[33]={'X',
						X_pos[0],X_pos[1],X_pos[2],X_pos[3],X_pos[4],'s','/','m','m',
						' ','Y',
						Y_pos[0],Y_pos[1],Y_pos[2],Y_pos[3],Y_pos[4],
					's','/','m','m',' ','Z',Z_pos[0],Z_pos[1],Z_pos[2],Z_pos[3],Z_pos[4],'s','/','m','m','\r'};
					Transmit_Data(acc);
					break; 				
			}	
		}
		else if (String[0] == 'G')
		{
			val = get_int (String,'G');
			switch (val)
			{
				case 0:case 1:
					motor_init();
					value_1 = get_value(String,'X');           //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					value_3 = get_value(String,'Z'); //call function to extract third value
					EX = get_value(String,'E');
					if ((EX && value_1 && value_2 && value_3) == 0)
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
						status = 1;
						if ((extrud || Fextrud))
						{
							double step[4]={x*Xspm,y*Yspm,z*Zspm,EX*Espm};
							motor_movement(step,F,Fe);
							status = 0;
						} 
						else
						{
							Transmit_Data("error extruder temp.");
							Transmit_Char('\r');
						}
						Transmit_Data("ok");					
					}
					break;
				case 2:case 3:
					value_1 = get_value(String,'X');  //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					I = get_value(String,'I');
					J = get_value(String,'J');
					if (get_value(String,'F') != 0)
					{
						F = get_value(String,'F');
					}
					if (get_value(String,'R') != 0)
					{
						R = get_value(String,'R');
					} 
					else
					{
						R = sqrt(pow(I,2)+pow(J,2));
					}
					status = 1;
					for (int th = 0 ; th <= 360 ; th++)
					{
						double xc = R*cos(th)*cos(th);
						double yc = R*sin(th)*sin(th);
						if ((xc >= value_1) && (yc >= value_2))
							break;
						x = sub_function (&old_val_1, xc);     //call function to extract first step
						y = sub_function (&old_val_2, yc);    //call function to extract second step
						if ((extrud || Fextrud))
						{
							if (val == 2)
							{
								if (get_value(String,'R') != 0)
								{
									double step[4]={x*Xspm,y*Yspm,0,1*Espm};
									motor_movement(step,F,Fe);
								}
								else
								{
									double step[4]={x*Xspm*(-I/I),y*Yspm*(-J/J),0,1*Espm};
									motor_movement(step,F,Fe);
								}
							} 
							else
							{
								if (get_value(String,'R') != 0)
								{
									double step[4]={x*Xspm*-1,y*Yspm*-1,0,1*Espm};
									motor_movement(step,F,Fe);
								}
								else
								{
									double step[4]={x*Xspm*(I/I),y*Yspm*(J/J),0,1*Espm};
									motor_movement(step,F,Fe);
								}
							}
							
						} 
					}
					status = 0;
					break;
				case 28:
					if (!homeSet)
					{
						x = find(String,'X');
						y = find(String,'Y');
						z = find(String,'Z');
						//make auto home
					} 
					Transmit_Data("ok");
					break;
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
	motor_init();
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR, K_D * SCALING_FACTOR, &pidData);
	TCNT0 = 0;
	TCCR0 |= (1<<CS02) | (1<<CS00); // PRESCALER 1024
	/*set fast PWM mode with non-inverted output*/
	TCCR0	|= (1<<WGM00)	| (1<<WGM01)	| (1<<COM01) | (1<<CS00);
	TCCR1A	|= (1<<COM1A1)	| (1<<COM1B1)	| (1<<WGM10);
	TCCR1B	|= (1<<CS10);
	TCCR2	|= (1<<WGM20)	| (1<<WGM21)	| (1<<COM21) | (1<<CS20);
	DDRB	|= (1<<PB3);  /*set OC0 pin as output*/
	TIMSK	|= (1<<TOIE0)	| (1<<TOIE1) ;
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
	if (gFlags.pidTimer == 1 ) 
	{
		OCR0 =	 255 - pid_Controller(SE0,getTemp(T0), &pidData);
		//OCR1A =  255 - pid_Controller(SB,getTemp(T2), &pidData);
		gFlags.pidTimer = FALSE;
		for (int i = 0 ; i < 10 ; i ++)
		{
			TE[i] = 0;
			TB[i] = 0;
		}
		sprintf(TE,dtostrf(getTemp(T0), 2,3,"%f"));
		sprintf(TB,dtostrf(getTemp(T2), 2,3,"%f"));
		if (( BED_Activ))
		{
			char TEMP[20]={'T',':',
				TE[0],TE[1],TE[2],TE[3],TE[4],
				' ','E',':','0',' ',
			'B',':',TB[0],TB[1],TB[2],TB[3],TB[4],'\r'};
			if ((SE0 < getTemp(T0)) && (SB < getTemp(T2)))
			{
				extrud = 1;
			} 
			else
			{
				extrud = 0;
			}
			Transmit_Data(TEMP);
		}
		else
		{
			char TEMP[12]={'T',':',
				TE[0],TE[1],TE[2],TE[3],TE[4],
			' ','E',':','0','\r'};
			if ((SE0 < getTemp(T0)))
			{
				extrud = 1;
			}
			else
			{
				extrud = 0;
			}
			Transmit_Data(TEMP);
		}
	}
}
ISR(TIMER1_OVF_vect)
{
	if (status == 1 && (UCSRA & (1 << RXC)))
	{
		for (int i = 0 ; i < 80 ;i++)
		{
			String [i] = 0;
		}
		Recive_Data(String);
		Transmit_Data("the printer is busy");
	} 	
}
ISR(TIMER2_OVF_vect)
{
	if ( get_int (String,'M') == 112)
	{
		double step[5]={0,0,0,0,0};
		motor_movement(step,F,Fe);
		status = 0;
	}
}
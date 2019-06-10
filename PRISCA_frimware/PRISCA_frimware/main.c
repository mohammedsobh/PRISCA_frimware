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
#include "Include.h" // all working library 
#define K_P 2.00	//GAIN of P term
#define K_I 0.05	//GAIN of I term
#define K_D 0.05	//GAIN of D term
/*********************************************************************************
*This function to make one number of variables used multi times					 *
*because we have two items need to control them temperature (heat bed & extruder)*
*we have																		 *
*BpidData ---> heat bed pid data												 *
*SpidData ---> extruder pid data												 *
*********************************************************************************/
struct pid_data BpidData;
struct pid_data SpidData;
/*********************************************************************************
*we use a eeprom space memory to  store the value of number of step for 1 mm     *
*each motor has its own s/mm value so there are four eeprom address				 *
*XSTEP_PER_mm_address ---> number of s/mm for X motor address		 (0X00)		 *
*YSTEP_PER_mm_address ---> number of s/mm for Y motor address		 (0X05)		 *
*ZSTEP_PER_mm_address ---> number of s/mm for Z motors address		 (0X0A)		 *
*ESTEP_PER_mm_address ---> number of s/mm for Extruder motor address (0X0F)		 *
*********************************************************************************/
#define  XSTEP_PER_mm_address	0x00 
#define  YSTEP_PER_mm_address	0x05
#define  ZSTEP_PER_mm_address	0x0A
#define  ESTEP_PER_mm_address	0x0F
void Init(void);					// initial function to set up the external devices
struct GLOBAL_FLAGS {
	//! True when PID control loop should run one time
	uint8_t pidTimer : 1;
	uint8_t dummy : 7;
	} gFlags = {0, 0};
int Vo;
char String[80] ;									// variable to storage the received uart in it
char X_pos[10],Y_pos[10],Z_pos[10],E_pos[10];		//variables to storage the position of each coordinates in them to send them using uart 
char TE[10],TB[10];									//variables to storage the temperature of heat bed & extruder in them to send them using uart
double STEP[4];										//variable to storage number of steps that each motor should rotat {x,y,z,extruder}
static uint16_t tcon = 0;							//variable to control the period of pid controller
/*********************************************************************************
*these switches to control the printer											 *
*status		---> the printer is busy (1) ornot (0)								 *
*BED_Activ	---> there is heat bed(1) or not (0)								 *
*XEN_DES	---> enable the X motor (0) or not (1)								 *
*YEN_DES	---> enable the Y motor (0) or not (1)								 *
*ZEN_DES	---> enable the Z motor (0) or not (1)								 *
*EEN_DES	---> enable the Extruder motor (0) or not (1)						 *
*homeSet	---> Manual home enabled (1) or not (0)								 *
*extrud		---> depending on the set temperature extrude faliment (1) or not (0)*
*Fextrud	---> cold extrude faliment (1) or not (0)							 *
*ReadTemp	---> send the temperature to uart (1) or not (0)					 *
*********************************************************************************/
bool status			= 0;
bool BED_Activ		= 0;
bool XEN_DES		= 0;
bool YEN_DES		= 0;
bool ZEN_DES		= 0;
bool EEN_DES		= 0;
bool homeSet		= 0;
bool extrud			= 0;
bool Fextrud		= 0;
bool ReadTemp		= 0;
//variable to store number of step per mm on them
/*************************************************************/
double Xspm,Yspm,Zspm,Espm,
FXspm = 100.00,FYspm = 100.00,FZspm = 100.00,FEspm = 140.00;
/************************************************************/
double SE0	= 0.00;		//variable to store the temperature of extruder on it
double F	= 0.00;		//variable to store speed of x y z motors
double FN	= 0.00;		//variable to store new speed of x y z & extruder motors
double Fe	= 0.00;		//variable to store speed of extruder
double SB	= 0.00;		//variable to store the temperature of heat bed on it
double Bt	= 0.00;		//variable to store the minimum temperature of extruder on it
double I	= 0.00;		//variable to store the shift of x coordinates
double J	= 0.00;		//variable to store the shift of y coordinates
double R	= 0.00;		//variable to store the radius of circular motion
long wait	= 0;		//variable to store the time that the printer is sleep
int val		= 0;		//variable to store the number of control gcode
int SUBval	= 0;		//variable to store the number of control gcode for some codes
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
	//to get the s/mm that stored in eeprom
	/*****************************************************/
	EEPROM_ReadNBytes(XSTEP_PER_mm_address,X_pos,5);
	EEPROM_ReadNBytes(YSTEP_PER_mm_address,Y_pos,5);
	EEPROM_ReadNBytes(ZSTEP_PER_mm_address,Z_pos,5);
	EEPROM_ReadNBytes(ESTEP_PER_mm_address,E_pos,5);
	Xspm = atof(X_pos);
	Yspm = atof(Y_pos);
	Zspm = atof(Z_pos);
	Espm = atof(E_pos);
	/****************************************************/
	while (1)
	{
		if (status == 0)
		{
			for (int i = 0 ; i < 80 ;i++)
			{
		 		String [i] = 0;
			}
			Recive_Data(String); // receive uart data and store it in variable (string)
		}
		// this code for run the Gcode depending on codes that in https://docs.google.com/document/d/1-IXL4SPSpeL7-teKqPJBG51-9jkx55wjBWgZDoANAug/edit?usp=sharing&fbclid=IwAR3pC85grjWT5KBqa4N7_nx4Ls8xZIw1vQixjXgDGBRfcvUnp72kgWrgpcQ
		if (String[0] == 'M')
		{
			val = get_int (String,'M');
			switch (val)
			{
				case 0: case 1:
					STEP[0] = 0;
					STEP[1] = 0;
					STEP[2] = 0;
					STEP[3] = 0;
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
					STEP[3] = 0;
					Transmit_Data("ok");
					break;
				case 83:
					old_val_1 = 0;
					old_val_2 = 0;
					old_val_3 = 0;
					STEP[0] = 0;
					STEP[1] = 0;
					STEP[2] = 0;
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
					ReadTemp = 1;
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
					Bt = get_value(String,'B');
					if (Bt == 0)
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
					STEP[0] = sub_function (&old_val_1, value_1)*Xspm;     //call function to extract first step
					STEP[1] = sub_function (&old_val_2, value_2)*Yspm;    //call function to extract second step
					STEP[2] = sub_function (&old_val_3, value_3)*Zspm;    //call function to extract third step
					STEP[3] = 0;
					F = 0;
					Fe = 0;
					SE0 = 0;
					SB = 0;
					status = 1;
					motor_movement(STEP,F,Fe);
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
					SB = get_value(String,'S');
					BED_Activ = 1;
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
					Transmit_Data("ok");
					break;
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
					char acc[44]={'X',
						X_pos[0],X_pos[1],X_pos[2],X_pos[3],X_pos[4],'s','/','m','m',
						' ','Y',
						Y_pos[0],Y_pos[1],Y_pos[2],Y_pos[3],Y_pos[4],
					's','/','m','m',' ','Z',Z_pos[0],Z_pos[1],Z_pos[2],Z_pos[3],Z_pos[4],'s','/','m','m',
					' ','E',E_pos[0],E_pos[1],E_pos[2],E_pos[3],E_pos[4],'s','/','m','m','\r'};
					Transmit_Data(acc);
					break; 				
			}	
		}
		else if (String[0] == 'G')
		{
			val = get_int (String,'G');
			switch (val)
			{
				case 1:
					motor_init();
					value_1 = get_value(String,'X');           //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					value_3 = get_value(String,'Z'); //call function to extract third value
					STEP[3] = get_value(String,'E');
					if ((STEP[3] && value_1 && value_2 && value_3) == 0)
					{
						FN = get_value(String,'F');
						if (FN != 0)
						{
							Fe = FN;
						}
					}
					else
					{
						FN = get_value(String,'F');
						if (FN != 0)
						{
							F = FN;
						}
					}
					if (!(value_1>200||value_2>200||value_3>500)) // if the values don't skip the plate ,use it

					{
						STEP[0] = sub_function (&old_val_1, value_1)*Xspm;     //call function to extract first step
						STEP[1] = sub_function (&old_val_2, value_2)*Yspm;    //call function to extract second step
						STEP[2] = sub_function (&old_val_3, value_3)*Zspm;    //call function to extract third step
						value_1 = 0;
						value_2 = 0;
						value_3 = 0;
						status = 1;
						if ((extrud || Fextrud))
						{
							motor_movement(STEP,F,Fe);
							status = 0;
						} 
						else
						{
							Transmit_Data("error extruder temp.");
							Transmit_Char('\r');
							break;
						}						
					}
					Transmit_Data("ok");
					break;
				case 2:case 3:
					value_1 = get_value(String,'X');  //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					I = get_value(String,'I');
					J = get_value(String,'J');
					FN = get_value(String,'F');
					if (FN != 0)
					{
						F = FN;
					}
					R = get_value(String,'R');
					if (R == 0)
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
						if ((extrud || Fextrud))
						{
							if (val == 2)
							{
								if ((I == 0) && (J == 0))
								{
									STEP[0] = sub_function (&old_val_1, xc)*Xspm;     //call function to extract first step
									STEP[1] = sub_function (&old_val_2, yc)*Yspm;    //call function to extract second step
								}
								else
								{
									STEP[0] = sub_function (&old_val_1, xc)*Xspm*(-I/I);     //call function to extract first step
									STEP[1] = sub_function (&old_val_2, yc)*Yspm*(-J/J);    //call function to extract second step
								}
							} 
							else
							{
								if ((I == 0) && (J == 0))
								{
									STEP[0] = sub_function (&old_val_1, xc)*-Xspm;     //call function to extract first step
									STEP[1] = sub_function (&old_val_2, yc)*-Yspm;    //call function to extract second step
								}
								else
								{
									STEP[0] = sub_function (&old_val_1, xc)*Xspm*(I/I);     //call function to extract first step
									STEP[1] = sub_function (&old_val_2, yc)*Yspm*(J/J);    //call function to extract second step
								}
							}
							STEP[2] = 0;
							STEP[3] = Espm;
							motor_movement(STEP,F,Fe);
						} 
					}
					status = 0;
					break;
				case 28:
					if (!homeSet)
					{
						STEP[0] = find(String,'X');
						STEP[1] = find(String,'Y');
						STEP[2] = find(String,'Z');
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
	pin_direction (D, 2, input );
	DDRC = 0XFF;
	DDRA |= (1<<PA4) | (1<<PA5) | (1<<PA6) | (1<<PA7);
	DDRB |= (1<<PB0) | (1<<PB1);
	ADCSRA = 0x87; //to active A/D pins
	ADMUX |= (1<<REFS0);// external reference volt is selected
	GICR |= 0x40;
	MCUCR |= 0x11;
	UART_INIT();
	motor_init();
	Pid_init(K_P * Scaling, K_I * Scaling, K_D * Scaling, &BpidData);
	Pid_init(K_P * Scaling, K_I * Scaling, K_D * Scaling, &SpidData);
	/*set fast PWM mode with non-inverted output*/
	cli();
	TCCR1A |=(1<<COM1A1)  | (1<<COM1B1) | (1<<WGM10);
	TCCR1B |=  (1<<CS10) | (1<<WGM20);
	TCCR2	|= (1<<WGM20)|(1<<WGM21)|(1<<COM21)|(1<<CS21)|(1<<CS22);
	// Set up timer, enable timer/counter 0 overflow interrupt
	TCCR0 |= (1 << CS00) | (1<< FOC0); // clock source to be used by the Timer/Counter clkI/O
	TIMSK |= (1 << TOIE0);
	TCNT0  = 0;
	sei();
}
ISR(TIMER0_OVF_vect)
{
	if (tcon < Time_Interval) //CONTROL THE INTERVAL BETWEEN EACH PID PROCESSES
	{
		tcon++;
		} else {
		gFlags.pidTimer = 1;
		tcon            = 0;
		
	}
	if (gFlags.pidTimer == 1 ) 
	{
 		OCR1A =	pid_Controller(SE0	,getTemp(T2), &SpidData); //out the pid value to control the temperature of extruder
  		OCR1B = pid_Controller(SB	,getTemp(T0), &BpidData); //out the pid value to control the temperature of heat bed
		gFlags.pidTimer = 0;
		for (int x = 0 ; x < 10 ; x ++)
		{
			TE[x] = 0;
			TB[x] = 0;
		}
		if (ReadTemp) //send the temperature to uart
		{
			sprintf(TE,dtostrf(getTemp(T2), 2,3,"%f"));
			sprintf(TB,dtostrf(getTemp(T0), 2,3,"%f"));
			if ((BED_Activ == 1))
			{
				char TEMP[20]={'T',':',
					TE[0],TE[1],TE[2],TE[3],TE[4],
					' ','E',':','0',' ',
				'B',':',TB[0],TB[1],TB[2],TB[3],TB[4],'\r'};
				if ((SE0 < getTemp(T0)) && (SB < getTemp(T2))) //enable extrude filament
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
	if (status == 1 && (UCSRA & (1 << RXC))) //if the printer busy send ack.
	{
		for (int i = 0 ; i < 80 ;i++)
		{
			String [i] = 0;
		}
		Recive_Data(String);
		Transmit_Data("the printer is busy");
	}
	if (String[0] == 'M' && status == 1)
	{
		SUBval = get_int(String,'M');
		if (SUBval == 112)
		{
			STEP[0] = 0;
			STEP[1] = 0;
			STEP[2] = 0;
			STEP[3] = 0;
			motor_movement(STEP,F,Fe);
			status = 0;
		}
	}
}
ISR (INT0_vect)
{
	pin_write (M_PORT, Z_DIR_PIN ,1 );
	pin_write (M_PORT, Z ,0 );
	_delay_us (300);
	pin_write (M_PORT, Z ,1 );
	_delay_us (300);
}
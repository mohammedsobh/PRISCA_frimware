/*
 * PRISCA_frimware.c
 *
 * Created : 4/23/2019 6:42:43 PM
 * Author  : MOHAMMED SOBH & NORHAN TAREK
 * Company : PRISCA
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // library for boolean variable
#include <avr/interrupt.h>
#include <math.h>
#include <inttypes.h> // needed for type declarations
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
void Init(void);
void init_Stack(void)
{
// 	SPH = 0x04;
// 	SPL = 0xFF;
}
struct GLOBAL_FLAGS {
	//! True when PID control loop should run one time
	uint8_t pidTimer : 1;
	uint8_t dummy : 7;
	} gFlags = {0, 0};
int Vo;
char RXch =0 ;										// variable to storage the received uart in it
char String[80] ;									// variable to storage the received uart in it
char RXch2 =0 ;										// variable to storage the received uart in it
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
// bool START			= 1;
bool STARTP			= 0;
bool TASK			= 0;
//variable to store number of step per mm on them
/*************************************************************/
double SPMM[4],
FSPMM[4] = {100.00,100.00,100.00,140.00};
/************************************************************/
double SE0			= 0.00;		//variable to store the temperature of extruder on it
double F			= 0.00;		//variable to store speed of x y z motors
double Fe			= 0.00;		//variable to store speed of extruder
double SB			= 0.00;		//variable to store the temperature of heat bed on it
double I			= 0.00;		//variable to store the shift of x coordinates
double J			= 0.00;		//variable to store the shift of y coordinates
double R			= 0.00;		//variable to store the radius of circular motion
long wait			= 0;		//variable to store the time that the printer is sleep
int val				= 0;		//variable to store the number of control gcode
int SUBval			= 0;		//variable to store the number of control gcode for some codes
int SUBval2			= 0;		//variable to store the number of control gcode for some codes
int NumberOfLine	= 0;		//variable to count number of line that printed
int NumberOfPLine	= 0;		//variable to count number of line that printed
int CheckSum		= 0;		//variable to count number of line that printed
/*static variables that initialize only once */
static double old_val_1 = 0;
static double old_val_2 = 0;
static double old_val_3 = 0;
//three variables that contain numbers that convert from string/
double value_1;
double value_2;
double value_3;
int main(void)
{
    Init();
	//to get the s/mm that stored in eeprom
	/*****************************************************/
	EEPROM_ReadNBytes(XSTEP_PER_mm_address,X_pos,5);
	EEPROM_ReadNBytes(YSTEP_PER_mm_address,Y_pos,5);
	EEPROM_ReadNBytes(ZSTEP_PER_mm_address,Z_pos,5);
	EEPROM_ReadNBytes(ESTEP_PER_mm_address,E_pos,5);
	SPMM[0] = atof(X_pos);
	SPMM[1] = atof(Y_pos);
	SPMM[2] = atof(Z_pos);
	SPMM[4] = atof(E_pos);
	/****************************************************/
	while (1)
	{
			if (RXch == 'M')
			{
				status = 1;
				cli();
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
						Transmit_Data("ok\0");
					break;
					case 17:
						motor_init();
						Transmit_Data("ok\0");
					break;
					case 18: case 84:
						wait = (get_int(String ,'S')*1000);
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
							sei();
							while(wait > 0)
							{
								_delay_ms(1);
								wait --;
							}
							motor_init();
						}
						Transmit_Data("ok\0");
					break;
					case 109:
						SE0 = get_value(String,'S');
						sei();
						while(getTemp(T0) < SE0);
						Transmit_Data("ok\0");
					break;
					case 190:
						SB = get_value(String,'S');
						sei();
						while(getTemp(T2) < SB);
						Transmit_Data("ok\0");
					break;
					case 82:case 83:
						old_val_1 = 0;
						old_val_2 = 0;
						old_val_3 = 0;
						STEP[0] = 0;
						STEP[1] = 0;
						STEP[2] = 0;
						STEP[3] = 0;
						Transmit_Data("ok\0");
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
							SPMM[0] = get_value(String,'X');
							SPMM[1] = get_value(String,'Y');
							SPMM[2] = get_value(String,'Z');
							SPMM[3] = get_value(String,'E');
						}
						if (val == 502)
						{
							SPMM[0] = FSPMM[0];
							SPMM[1] = FSPMM[1];
							SPMM[2] = FSPMM[2];
							SPMM[3] = FSPMM[3];
						}
						sprintf(X_pos,dtostrf(SPMM[0], 2,3,"%f"));
						sprintf(Y_pos,dtostrf(SPMM[1], 2,3,"%f"));
						sprintf(Z_pos,dtostrf(SPMM[2], 2,3,"%f"));
						sprintf(E_pos,dtostrf(SPMM[3], 2,3,"%f"));
						EEPROM_WriteNBytes(XSTEP_PER_mm_address,X_pos,5); //write 2-bytes of data(Xspm) at 0x00.
						EEPROM_WriteNBytes(YSTEP_PER_mm_address,Y_pos,5); //write 2-bytes of data(Yspm) at 0x02.
						EEPROM_WriteNBytes(ZSTEP_PER_mm_address,Z_pos,5); //write 2-bytes of data(Zspm) at 0x04.
						EEPROM_WriteNBytes(ESTEP_PER_mm_address,E_pos,5); //write 2-bytes of data(E0spm) at 0x06.
						Transmit_Data("ok\0");
					break;
					case 104:
						SE0 = get_value(String,'S');
						Transmit_Data("ok\0");
					break;
					case 106:
						OCR2 = get_value(String,'S');
						Transmit_Data("ok\0");
					break;
					case 107:
						OCR2 = 0;
						Transmit_Data("ok\0");
					break;	
					case 110:
						NumberOfPLine = 0;
						Transmit_Data("ok\0");
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
									' ','Z',Z_pos[0],Z_pos[1],Z_pos[2],Z_pos[3],Z_pos[4],'\0'};
						Transmit_Data(pos);
					break;
					case 140:
						SB = get_value(String,'S');
						if (SB == 0)
						{
							BED_Activ = 0;
						}
						else
						{
							BED_Activ = 1;
						}
						Transmit_Data("ok\0");
					break;		
					case 206:
						old_val_1 = 0;
						old_val_2 = 0;
						old_val_3 = 0;
						homeSet = 1;
						Transmit_Data("ok\0");
					break;
					case 302:
						SE0 = get_value(String,'S');
						if (SE0 <= 25 )
						{
							Fextrud = 1;
						}
						Transmit_Data("ok\0");
					break;
					case 501:
						EEPROM_ReadNBytes(XSTEP_PER_mm_address,X_pos,5);
						EEPROM_ReadNBytes(YSTEP_PER_mm_address,Y_pos,5);
						EEPROM_ReadNBytes(ZSTEP_PER_mm_address,Z_pos,5);
						EEPROM_ReadNBytes(ESTEP_PER_mm_address,E_pos,5);
						SPMM[0] = atof(X_pos);
						SPMM[1] = atof(Y_pos);
						SPMM[2] = atof(Z_pos);
						SPMM[3] = atof(E_pos);
						Transmit_Data("ok\0");
					break;
					case 503:
						for (int i = 0 ; i < 10 ; i ++)
						{
							X_pos[i] = 0;
							Y_pos[i] = 0;
							Z_pos[i] = 0;
							E_pos[i] = 0;
						}
						sprintf(X_pos,dtostrf(SPMM[0], 2,3,"%f"));
						sprintf(Y_pos,dtostrf(SPMM[1], 2,3,"%f"));
						sprintf(Z_pos,dtostrf(SPMM[2], 2,3,"%f"));
						sprintf(E_pos,dtostrf(SPMM[3], 2,3,"%f"));
						char acc[44]={'X',
							X_pos[0],X_pos[1],X_pos[2],X_pos[3],X_pos[4],'s','/','m','m',
							' ','Y',
							Y_pos[0],Y_pos[1],Y_pos[2],Y_pos[3],Y_pos[4],
							's','/','m','m',' ','Z',Z_pos[0],Z_pos[1],Z_pos[2],Z_pos[3],Z_pos[4],'s','/','m','m',
							' ','E',E_pos[0],E_pos[1],E_pos[2],E_pos[3],E_pos[4],'s','/','m','m','\0'};
						Transmit_Data(acc);
					break;
					default:
						RXch2 = RXch ;
					break;
				}
				status = 0;
				RXch = 0;
				sei();
			}
			else if (RXch == 'G')
			{
				cli();
				status = 1;
				switch (val)
				{
					case 0:case 1:
						motor_init();
						STEP[0] = find(String,'X');
						STEP[1] = find(String,'Y');
						STEP[2] = find(String,'Z');
						value_1 = get_value(String,'X');           //extract first value
						value_2 = get_value(String,'Y'); //call function to extract second value
						value_3 = get_value(String,'Z'); //call function to extract third value
						
						if (find(String,'E') && find(String,'F'))
						{
							Fe = get_value(String,'F');
						}
						else if((!find(String,'E')) && find(String,'F'))
						{
							F = get_value(String,'F');
						}
						if (!(value_1>200||value_2>200||value_3>500)) // if the values don't skip the plate ,use it
						{
							if (val == 0)
							{
								STEP[3] = 0;
								status = 1;
								if (STEP[0] == 1)
								{
									STEP[0] = value_1*SPMM[0];     //call function to extract first step
									old_val_1 += value_1;
								}
								if (STEP[1] == 1)
								{
									STEP[1] = value_2*SPMM[1];    //call function to extract second step
									old_val_2 += value_2;
								}
								if (STEP[2] == 1)
								{
									STEP[2] = value_3*SPMM[2];    //call function to extract third step
									old_val_3 += value_3;
								}
								value_1 = 0;
								value_2 = 0;
								value_3 = 0;
								motor_movement(STEP,SPMM,F,Fe);
								Transmit_Data("ok\0");
							} 
							else
							{
								STEP[3] = get_value(String,'E')*SPMM[3];
								if ((extrud || Fextrud))
								{
									status = 1;
									if (STEP[0] == 1)
									{
										STEP[0] = sub_function (&old_val_1, value_1)*SPMM[0];     //call function to extract first step
									}
									if (STEP[1] == 1)
									{
										STEP[1] = sub_function (&old_val_2, value_2)*SPMM[1];    //call function to extract second step
									}
									if (STEP[2] == 1)
									{
										STEP[2] = sub_function (&old_val_3, value_3)*SPMM[2];    //call function to extract third step
									}
									value_1 = 0;
									value_2 = 0;
									value_3 = 0;
									motor_movement(STEP,SPMM,F,Fe);
									Transmit_Data("ok\0");
								}
								else
								{
									Transmit_Data("error extruder temp.\0");
									break;
								}
							}
						}
						else
							Transmit_Data("out of area\0");
					break;
					case 2:case 3:
					value_1 = get_value(String,'X');  //extract first value
					value_2 = get_value(String,'Y'); //call function to extract second value
					I = get_value(String,'I');
					J = get_value(String,'J');
					if (find(String,'F'))
					{
						F = get_value(String,'F');
					}
					R = get_value(String,'R');
					if (R == 0)
					{
						R = sqrt(pow(I,2)+pow(J,2));
					}
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
								STEP[0] = sub_function (&old_val_1, xc)*SPMM[0];     //call function to extract first step
								STEP[1] = sub_function (&old_val_2, yc)*SPMM[1];    //call function to extract second step
							}
							else
							{
								STEP[0] = sub_function (&old_val_1, xc)*SPMM[0]*(-I/I);     //call function to extract first step
								STEP[1] = sub_function (&old_val_2, yc)*SPMM[1]*(-J/J);    //call function to extract second step
							}
						}
						else
						{
							if ((I == 0) && (J == 0))
							{
								STEP[0] = sub_function (&old_val_1, xc)*-SPMM[0];     //call function to extract first step
								STEP[1] = sub_function (&old_val_2, yc)*-SPMM[1];    //call function to extract second step
							}
							else
							{
								STEP[0] = sub_function (&old_val_1, xc)*SPMM[0]*(I/I);     //call function to extract first step
								STEP[1] = sub_function (&old_val_2, yc)*SPMM[1]*(J/J);    //call function to extract second step
							}
						}
						STEP[2] = 0;
						STEP[3] = SPMM[3];
						status = 1;
						motor_movement(STEP,SPMM,F,Fe);
					}
				}
				Transmit_Data("ok\0");
					break;
					case 28:
					// 					if (!homeSet)
					// 					{
					// 						STEP[0] = find(String,'X');
					// 						STEP[1] = find(String,'Y');
					// 						STEP[2] = find(String,'Z');
					// 						//make auto home
					// 					}
					Transmit_Data("ok\0");
					break;
					case 92:
// 					value_1 = get_value(String,'X');           //extract first value
// 					value_2 = get_value(String,'Y'); //call function to extract second value
// 					value_3 = get_value(String,'Z'); //call function to extract third value
// 					STEP[3] = get_value(String,'E');
					//set the middle of the bed to 0,0
					Transmit_Data("ok\0");
					break;
				}
				status = 0;
				RXch = 0;
				STEP[0] = 0;
				STEP[1] = 0;
				STEP[2] = 0;
				STEP[3] = 0;
				sei();
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
	init_Stack();
	motor_init();
	Pid_init(K_P * Scaling, K_I * Scaling, K_D * Scaling, &BpidData);
	Pid_init(K_P * Scaling, K_I * Scaling, K_D * Scaling, &SpidData);
	/*set fast PWM mode with non-inverted output*/
	TCCR1A |=(1<<COM1A1)  | (1<<COM1B1) | (1<<WGM10);
	TCCR1B |=  (1<<CS10) | (1<<WGM20);
	TCCR2	|= (1<<WGM20)|(1<<WGM21)|(1<<COM21)|(1<<CS21)|(1<<CS22);
	// Set up timer, enable timer/counter 0 overflow interrupt
	TCCR0 |= (1 << CS00) | (1<< FOC0); // clock source to be used by the Timer/Counter clkI/O
	TIMSK |= (1 << TOIE0) | (1 << OCIE0);
	TCNT0  = 0;
	OCR0 = 125;
	Recive_Data(String);
	Transmit_Data("welcome we are PRISCA\0");
	sei();
}
ISR(TIMER0_COMP_vect)
{
	if (tcon < Time_Interval) //CONTROL THE INTERVAL BETWEEN EACH PID PROCESSES
	{
		tcon++;
		} else {
		gFlags.pidTimer = 1;
		tcon            = 0;
	}
}

ISR(TIMER0_OVF_vect)
{
	if ((UCSRA & (1 << RXC))) //if the printer busy send ack.
	{
		for (int i = 0 ; i < 80 ;i++)
		{
			String [i] = 0;
		}
		Recive_Data(String);
		if (String[0] == 'N')
		{
				NumberOfPLine ++ ;
				NumberOfLine = get_int(String,'N');
				CheckSum = get_int(String,'*');
				get_SEvalue(String,' ','*');
				RXch = String[0];
				val = get_int(String,String[0]);
				if (val == 105)
				{STARTP  = 1;}
		}
		else
		{
				if (status)
				{
					RXch2 = String[0];
				}
				else
				{			
					RXch = String[0];
				}
				val = get_int(String,String[0]);
		}
	}

	if (RXch2 == 'M' )
	{
		if (val == 112)
		{
			value_1 = 0;  //extract first value
			value_2 = 0; //call function to extract second value
			value_3 = 0; //call function to extract third value
			STEP[0] = sub_function (&old_val_1, value_1)*SPMM[0];     //call function to extract first step
			STEP[1] = sub_function (&old_val_2, value_2)*SPMM[1];    //call function to extract second step
			STEP[2] = sub_function (&old_val_3, value_3)*SPMM[2];    //call function to extract third step
			STEP[3] = 0;
			SE0 = 0;
			SB = 0;
			status = 1;
			motor_movement(STEP,SPMM,F,Fe);
			STEP[0] = 0;
			STEP[1] = 0;
			STEP[2] = 0;
			STEP[3] = 0;
			status = 0;
			Transmit_Data("ok\0");
		}
		if (val == 105)
		{
			for (int x = 0 ; x < 10 ; x ++)
			{
				TE[x] = 0;
				TB[x] = 0;
			}
			sprintf(TE,dtostrf(getTemp(T0), 2,3,"%f"));
			sprintf(TB,dtostrf(getTemp(T2), 2,3,"%f"));
			if ((BED_Activ == 1))
			{
				char TEMP[20]={'T',':',
							TE[0],TE[1],TE[2],TE[3],TE[4],
							' ','E',':','0',' ',
							'B',':',TB[0],TB[1],TB[2],TB[3],TB[4],'\n'};
				Transmit_Data(TEMP);
			}
			else
			{
				char TEMP[15]={'T',':',
							TE[0],TE[1],TE[2],TE[3],TE[4],
							' ','E',':','0','\n'};
				Transmit_Data(TEMP);
			}
			Transmit_Data("ok\0");
		}
	RXch2 = 0;
	}
		if (gFlags.pidTimer == 1)
		{
			OCR1A =	255 - pid_Controller(SE0	,getTemp(T0), &SpidData); //out the pid value to control the temperature of extruder
			OCR1B = 255 - pid_Controller(SB		,getTemp(T2), &BpidData); //out the pid value to control the temperature of heat bed
			gFlags.pidTimer = 0;
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
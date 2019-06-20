/*
 * motor.c
 *
 * Created: 4/26/2019 1:24:14 AM
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */

#include "motor.h"
#include "Gpio.h"
#include <math.h>
#include <stdbool.h>
#include <util/delay.h>
//*****MOTOR_INITILIZATION*****
//make the C port as output, and make it low
//check the steps of x, y, z 
//if they have a negative number, make the motors move in anti_clock_wise
//if they have a positive number, make the motors move in clock_wise
void motor_init ()
{	
	port_direction(M_PORT,output);
	port_write(M_PORT,0);
	pin_direction(EN_DES_PORT,EN_DES_XPIN,output);
	pin_direction(EN_DES_PORT,EN_DES_YPIN,output);
	pin_direction(EN_DES_PORT,EN_DES_ZPIN,output);
	pin_direction(EN_DES_PORT,EN_DES_EPIN,output);
	motor_EN_DES('X',0);
	motor_EN_DES('Y',0);
	motor_EN_DES('Z',0);
	motor_EN_DES('E',0);
}
void motor_movement(double step[4],double spmm[4],double speed,double exspeed)
{
	long stepDuration[4];
	if (speed != 0.0)
	{
		for (int i = 0; i < 3 ; i++)
		{
			stepDuration[i] = (1/(speed*spmm[i]))*60*pow(10,6);
			if (stepDuration[i] < 100)
				stepDuration[i] = 100;
		}
	}
	else
	{
		for (int i = 0; i < 3 ; i++)
		{
			stepDuration[i] = 100;
		}
	}	
	if (exspeed !=0.0)
	{
		stepDuration[3] = (1/(exspeed*spmm[3]))*60*pow(10,6);
		if (stepDuration[3] < 100)
			stepDuration[3] = 100;
	}
	else
		stepDuration[3] = 100;
	int DIR_pin[4] = {X_DIR_PIN,Y_DIR_PIN,Z_DIR_PIN,E_DIR_PIN};
	for (int i = 0; i < 4 ; i++)
	{
		if (step[i]<0)
		{
			pin_write (M_PORT, DIR_pin[i] ,1);
			step [i] = step [i] * -1;
		}
		else
		pin_write (M_PORT, DIR_pin[i] ,0);
	}
	int S ;         // number from 0 to 3 index to the current state
	int index ;     //input from 0 to 7 to  choose next state
	int j [4] = {1,1,1,1} ;
	
	struct State
	{
		int Out;     // make one step to one motor in one unit of time
		int Next[16];
	};
	typedef const struct State STyp; //define STYP from type of struct State
	/*{Current state,{next state}} */
	STyp FSM[4]=
	{
		{X,{E,X,Y,Y,Z,X,Y,Y,E,E,E,Y,E,E,Y,Y}},
		{Y,{E,X,Y,X,Z,X,Y,X,E,E,E,E,E,E,E,E}},
		{Z,{E,X,Y,X,Z,X,Y,X,E,X,Y,X,E,X,Y,X}},
		{E,{E,X,Y,X,Z,X,Y,X,E,X,Y,X,Z,X,Y,X}}
	};	 
		index = 1;
		S = X;
		j [0] = 1;
		j [1] = 1;
		j [2] = 1;
		j [3] = 1;
		//***************MAKING THE 3 MOTORS MOVE TOGETHER*************** 
		//starting with X as the beginning state and make it execute one step 
		//decreases one from the step [X]
		//check if step [X] reaches to 0 make J[S] = 0, else J[S] = 1
		//wait, then calculate index that realize the input from 0 to 7
	    //move to the next state depending on input and current state  
		while ((index != 0 ))  
		    {  
			 pin_XOR(M_PORT,FSM[S].Out);
			 step[S] = step[S] - 1;
			 if (step [S] <= 0)
			  j[S] = 0;
			for (int i = stepDuration[S] ; i>=0 ; i-- )
			{
				 _delay_us(1);
			}
			 index  = ( j[3]<<3) + ( j[2]<<2) +(j[1]<<1) +(j[0]<<0);
			 S = FSM[S].Next[index];
		    }
    }
void motor_EN_DES(char motor,bool stu)
{
	switch (motor)
	{
		case 'X':
			pin_write(EN_DES_PORT,EN_DES_XPIN,stu);
			break;
		case 'Y':
			pin_write(EN_DES_PORT,EN_DES_YPIN,stu);
			break;
		case 'Z':
			pin_write(EN_DES_PORT,EN_DES_ZPIN,stu);
			break;
		case 'E':
			pin_write(EN_DES_PORT,EN_DES_EPIN,stu);
			break;
	}
}
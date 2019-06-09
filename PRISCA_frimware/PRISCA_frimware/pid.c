/*
 * pid.c
 *
 * Created: 09/03/2019 6:56:00 PM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */
/*! \file *********************************************************************
 *
 * \brief General PID implementation for AVR.
 *
 * Discrete PID controller implementation. Set up by giving P/I/D terms
 * to Init_PID(), and uses a struct PID_DATA to store internal values.
 *
 *
 *****************************************************************************/

#include "pid.h"
#include "stdint.h"
/*! \brief Initialization of PID controller parameters.
 *
 *  Initialize the variables used by the PID algorithm.
 *
 *  \param p  Proportional term.
 *  \param i  Integral term.
 *  \param d  Derivate term.
 *  \param pid  Struct with PID status.
 */
void Pid_init(double P, double I, double D, struct pid_data *pid)
{
	// Tuning constants for PID loop
	pid->pid_P = P;
	pid->pid_I = I;
	pid->pid_D = D;
	// Limits to avoid overflow
	pid->MaxError = INT16_MAX/(pid->pid_P + 1);
	pid->MaxSumError = INT32_MAX/(2*(pid->pid_I + 1));
	// Start values for PID controller
	pid->last_error = 0;
	pid->SumError = 0;
}
/*! \brief PID control algorithm.
 *
 *  Calculates output from setpoint, process value and PID status.
 *
 *  \param setPoint  Desired value.
 *  \param processValue  Measured value.
 *  \param pid_st  PID status struct.
 */
int16_t pid_Controller(double setPoint, double processValue, struct pid_data *pid_st)
{
	double error = setPoint - processValue;
	double P_value = error * (pid_st ->pid_D);									// Calculate Pterm and limit error overflow
	pid_st ->SumError = pid_st ->SumError + error ;
	double I_value = pid_st ->SumError * pid_st->pid_I;							// Calculate Iterm and limit integral runaway
	long double D_value = (pid_st ->pid_D)*((error - (pid_st ->last_error)));	// Calculate Dterm
	if (error > (pid_st ->MaxError))
	{
		P_value = INT16_MAX;
	}
	else if(error < -(pid_st ->MaxError))
	{
		P_value = -INT16_MAX;
	}
	if (pid_st ->SumError > (pid_st ->MaxSumError))
	{
		pid_st ->SumError = pid_st ->MaxSumError;
		I_value = INT32_MAX/2;
	}
	else if(pid_st ->SumError < -(pid_st ->MaxSumError))
	{
		pid_st ->SumError = -pid_st ->MaxSumError;
		I_value = -INT32_MAX/2;
	}
	int16_t PID_value = (P_value + I_value + D_value)/Scaling ;
	if(PID_value < 0)
	{    PID_value = 0;    }
	if(PID_value > 255)
	{    PID_value = 255;} 	
	pid_st->last_error = PID_value;
	return(PID_value);
}
/*! \brief Resets the integrator.
 *
 *  Calling this function will reset the integrator in the PID regulator.
 */
void pid_Reset_Integrator(pidData_t *pid_st)
{
	pid_st->SumError = 0;
}


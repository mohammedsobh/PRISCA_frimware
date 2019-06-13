/*
 * pid.h
 *
 * Created: 09/03/2019 6:56:00 PM
 * Author  : MOHAMMED SOBH
 * Company : PRISCA
 */

#ifndef PID_H
#define PID_H

#include "stdint.h"
#define  Time_Interval 150
#define Scaling 124
/*! \brief PID Status
 *
 * Setpoints and data used by the PID control algorithm
 */
typedef struct pid_data
{
	double last_error;	//! Last process value, used to find derivative of process value.
	double pid_P;		//! The Proportional tuning constant, multiplied with SCALING_FACTOR
	double pid_I;		//! The Integral tuning constant, multiplied with SCALING_FACTOR
	double pid_D;		//! The Derivative tuning constant, multiplied with SCALING_FACTOR
	double SumError;	//! Summation of errors, used for integrate calculations
	double MaxError;	//! Maximum allowed error, avoid overflow
	double MaxSumError;	//! Maximum allowed sumerror, avoid overflow
}pidData_t;

void Pid_init(double P, double I, double D, struct pid_data *pid);
int16_t pid_Controller(double setPoint, double processValue, struct pid_data *pid_st);
void    pid_Reset_Integrator(pidData_t *pid_st);
#endif
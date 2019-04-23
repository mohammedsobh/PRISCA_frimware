/*
 * blutooth.c
 *
 * Created: 09/03/2019 6:56:00 PM
 * Author : PRISCA
 */ 
#include "Include.h"
#include <stdlib.h>
//***second_value_function***
//this function take one variable, and search in it about the location of ","
//then put the characters after "," in a new array of characters
//then convert string to integer value ,then return it
double X_value (char* String)
{
	int i = 0,j=0;
	char X_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		X_v [i] = 0;
	}
	while(i < 80)
	{
		if (String [i] == 'X'){
			X_v [j] = String[j+i+1];
			if (String [j+i+2] == ' ')
			break;
			j++;
		}
		else
		i++;
	}
	return (atof(X_v));
}

double Y_value (char* String)
{
    int i = 0,j=0;
	char Y_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		Y_v [i] = 0;
	}
    while(i < 80)
    {
        if (String [i] == 'Y'){
				Y_v [j] = String[j+i+1];
				if (String [j+i+2] == ' ')
				break;
				j++;
		}
		else
		i++;
    }
    return (atof(Y_v));
}
double Z_value (char* String)
{
	int i = 0,j=0;
	char Z_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		Z_v [i] = 0;
	}
	while(i < 80)
	{
		if (String [i] == 'Z'){
			Z_v [j] = String[j+i+1];
			if (String [j+i+2] == ' ')
			break;
			j++;
		}
		else
		i++;
	}	
	return (atof(Z_v));
}
double F_value (char* String)
{
	int i = 0,j=0;
	char F_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		F_v [i] = 0;
	}
	while(i < 80)
	{
		if (String [i] == 'F'){
			F_v [j] = String[j+i+1];
			if (String [j+i+2] == ' ')
			break;
			j++;
		}
		else
		i++;
	}
	return (atof(F_v));
}
double S_value (char* String)
{
	int i = 0,j=0;
	char S_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		S_v [i] = 0;
	}
	while(i < 80)
	{
		if (String [i] == 'S'){
			S_v [j] = String[j+i+1];
			if (String [j+i+2] == ' ')
			break;
			j++;
		}
		else
		i++;
	}
	return (atof(S_v));
}
double E_value (char* String)
{
	int i = 0,j=0;
	char E_v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		E_v [i] = 0;
	}
	while(i < 80)
	{
		if (String [i] == 'E'){
			E_v [j] = String[j+i+1];
			if (String [j+i+2] == ' ')
			break;
			j++;
		}
		else
		i++;
	}
	return (atof(E_v));
}

//***subtraction_function***
//it takes two variables, one call by reference and other call by value
//call by reference is used to make a change in it, to save the new value
//this function return a value that the motor have to move
//this value could be negative or positive, it realize the direction of movements
double sub_function (double *current_pos, double new_pos)
{
    double x = new_pos - *current_pos;
    *current_pos = new_pos;
    return x;
}
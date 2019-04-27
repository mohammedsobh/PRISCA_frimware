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
double get_value (char* String,char ch)
{
	int i = 0,j=0;
	char v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		v [i] = 0;
	}
	while(1)
	{
		if (String [i] == ch){
			v [j] = String[j+i+1];
			if ((String [j+i+2] == ' ') || ( String [j+i+2] == ';'))
			break;
			j++;
		}
		else
		i++;
	}
	return (atof(v));
}
int get_int (char* String,char ch)
{
	int i = 0,j=0;
	char v[10];
	for (int i = 0 ; i < 10 ;i++)
	{
		v [i] = 0;
	}
	while(1)
	{
		if (String [i] == ch){
			v [j] = String[j+i+1];
			if ((String [j+i+2] == ' ') || ( String [j+i+2] == ';'))
			break;
			j++;
		}
		else
		i++;
	}
	return (atoi(v));
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
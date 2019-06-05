/*
 * getValue.h
 *
 * Created : 09/03/2019 6:56:00 PM
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */


#ifndef GETVALUE_H_
#define GETVALUE_H_
#include "Include.h"
#include <stdbool.h>
double get_value (char* String,char ch);
int get_int (char* String,char ch);
double sub_function (double *current_pos, double new_pos);
bool find (char* String,char ch);
#endif /* GETVALUE_H_ */
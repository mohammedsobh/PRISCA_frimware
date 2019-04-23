/*
 * Usart.h
 * Created: 2/10/2019 1:29:30 PM
 *  Author: norhan
 */ 
#ifndef UART_H_
#define UART_H_
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

int k;
long Crystal ();
void UART_INIT();
void Transmit_Char(char ptr_char);
char Recive_Char(void);
void Transmit_Data(char *ptr_string);
void Recive_Data(char *ptr_string);


 #endif /* USART_H_ */
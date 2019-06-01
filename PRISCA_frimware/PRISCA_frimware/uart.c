/*
 * Usart.c
 *
 * Created : 2/10/2019 1:30:58 PM
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */ 

#include "uart.h"
#include <stdbool.h> // library for boolean variable
long Crystal ()
{

	long BAUD = 9600;
	long MyBaud =  ((F_CPU / (16 * BAUD))-1);
	return MyBaud;
}



void UART_INIT()
{
	long t;
	t =Crystal();
	UBRRL =  (uint8_t) (t);
	UBRRH = (uint8_t)(t>>8);
	
	UCSRB = ((1<<4) | (1<<3));     //Enable transmitter and Receiver
	UCSRC = ((1 << URSEL) |(1<<2) | (1<<1));     //Character size is 8_bits
	
}

void Transmit_Char(char ptr_char)
{
	while (!(UCSRA & (1<<5)));    //waits until 'bin 5' equal to 1, becomes Empty.
	UDR = ptr_char;                   //transmits one character.
}

char Recive_Char(void)
{
	while (! (UCSRA & (1 << RXC)));  //waits until 'bin7' equal to 1, have unread data in the receive buffer
	return(UDR); 
}
void Transmit_Data(char *ptr_string)
{
	while(*ptr_string)
	Transmit_Char(*ptr_string++);
}
void Recive_Data(char *ptr_string)
{   
	 	char ch;
	    uint8_t len = 0;
	 	while(1)
	 	{
	 		ch=Recive_Char();    //Receive a char
	 		if(( ch == '\n') || (ch == '\r')) //read till enter key is pressed
	 		{						     //once enter key is pressed null terminate the string
	 			ptr_string[len]=' ';
	 			break;           //and break the loop
	 		}
	        else
	         {
	             ptr_string[len++]=ch; //copy the char into string and increment the index
	         }
	 	}	
}

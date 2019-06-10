
/*
 * Gpio.c
 *
 * Created: 02/06/2018 04:28:17 ã
 * Author  : NORHAN TAREK
 * Company : PRISCA
 */

#include "Gpio.h"
#include "stdint.h"
#include <stdbool.h>
void pin_direction (char base, char bin, bool state )
{
	if (!state)
	{
		set_bin ( (base+1) , bin);
	}
	else
	{
		reset_bin( (base+1) , bin);
	}

}


char pin_Read(char base,char bin)
{
	char t ;
	t = Casting (base)  && (1<<bin);   
	return t;
}

void pin_write (char base, char bin ,bool p )
{
	if (p)
	set_bin ( (base+2) , bin);
	else
	reset_bin ( (base+2) , bin);
}

void port_direction (char base, bool state )
{
	if (!state)
	{
		( (*(volatile char*) (base+1)) = 0xff);
	}
	
	else if (state == input)
	{
		( (*(volatile char*) (base+1)) = 0x00);
	}
}
char port_Read(char base)
{
	char t ;
	t = Casting(base) ;
	return t;
}

void port_write (char base, bool p )
{
	(*(volatile char*) (base+2)) = p ;
}

void pin_XOR (char base, char bin  )
{
	XOR((base+2) , bin);
}

char Casting (int reg)
{
	return ( (*(volatile char*) reg));
}
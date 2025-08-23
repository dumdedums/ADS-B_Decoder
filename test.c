#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "adsb.h"
#include "decode.h"

int main()
{
	union AdsbFrame f1;
	f1.frame[13] = 0x8D;	//0x8D4840D6202CC371C32CE0576098
	f1.frame[12] = 0x48;	//ident message
	f1.frame[11] = 0x40;	//callsign KLM1023_
	f1.frame[10] = 0xD6;
	f1.frame[9] = 0x20;
	f1.frame[8] = 0x2C;
	f1.frame[7] = 0xC3;
	f1.frame[6] = 0x71;
	f1.frame[5] = 0xC3;
	f1.frame[4] = 0x2C;
	f1.frame[3] = 0xE0;
	f1.frame[2] = 0x57;
	f1.frame[1] = 0x60;
	f1.frame[0] = 0x98;

	printf("AdsbFrame is %zu bytes\n", sizeof(union AdsbFrame));
	printf("f1 content: %#X%X%X%X%X%X%X%X%X%X%X%X%X%X\n",
		(unsigned int)f1.frame[13], (unsigned int)f1.frame[12],
		(unsigned int)f1.frame[11], (unsigned int)f1.frame[10],
		(unsigned int)f1.frame[9], (unsigned int)f1.frame[8],
		(unsigned int)f1.frame[7], (unsigned int)f1.frame[6],
		(unsigned int)f1.frame[5], (unsigned int)f1.frame[4],
		(unsigned int)f1.frame[3], (unsigned int)f1.frame[2],
		(unsigned int)f1.frame[1], (unsigned int)f1.frame[0]);

	printf("DF: %u, CA: %u, ICAO: %#X, PI: %#X\n", (unsigned int)f1.df,
		(unsigned int)f1.ca, (unsigned int)f1.icao, (unsigned int)f1.pi);
	printf("Raw Message: %#X%X%X%X%X%X%X\nTC:%u\n", (unsigned int)f1.me.me[6],
		(unsigned int)f1.me.me[5], (unsigned int)f1.me.me[4],
		(unsigned int)f1.me.me[3], (unsigned int)f1.me.me[2],
		(unsigned int)f1.me.me[1], (unsigned int)f1.me.me[0],
		(unsigned int)f1.me.id.tc);

	char callsign[9], type[8];
	callsign[8] = 0;
	getIdent(&f1, callsign, type);
	printf("Callsign: %s, aircraft type: %s\n", callsign, type);

	return 0;
}

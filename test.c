#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "adsb.h"
#include "decode.h"
#include "logger.h"

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

	printf("F1: Callsign and Parity Test\n"
		"DF: %u, CA: %u, ICAO: %#X, PI: %#X\n", (unsigned int)f1.df,
		(unsigned int)f1.ca, (unsigned int)f1.icao, (unsigned int)f1.pi);
	printf("Raw Message: %#X%X%X%X%X%X%X, TC:%u\n", (unsigned int)f1.me.me[6],
		(unsigned int)f1.me.me[5], (unsigned int)f1.me.me[4],
		(unsigned int)f1.me.me[3], (unsigned int)f1.me.me[2],
		(unsigned int)f1.me.me[1], (unsigned int)f1.me.me[0],
		(unsigned int)f1.me.id.tc);

	char callsign[9], type[8];
	callsign[8] = 0;
	getIdent(&f1, callsign, type);
	printf("Callsign: %s, Aircraft Type: %s, Category: %u\n", callsign, type,
		(unsigned int)f1.me.id.cat);

	//parity checks might get skipped in main app to save execution time
	if(parityCheck(&f1))
		printf("Parity Check Failed\n\n");
	else
		printf("Parity Check Success\n\n");

	double olat, olng;	//O'Hare Airport for relative location
	olat = 41.978611;
	olng = -87.904722;

	int falt;
	double tlat, tlng, flat, flng;
	tlat = 52.258;		//relative location for test frame
	tlng = 3.918;

	union AdsbFrame f2;
	f2.frame[13] = 0x8D;	//0x8D40621D58C382D690C8AC2863A7
	f2.frame[12] = 0x40;	//lat should be 52.257202
	f2.frame[11] = 0x62;	//lng should be 3.91937256
	f2.frame[10] = 0x1D;
	f2.frame[9] = 0x58;
	f2.frame[8] = 0xC3;
	f2.frame[7] = 0x82;
	f2.frame[6] = 0xD6;
	f2.frame[5] = 0x90;
	f2.frame[4] = 0xC8;
	f2.frame[3] = 0xAC;
	f2.frame[2] = 0x28;
	f2.frame[1] = 0x63;
	f2.frame[0] = 0xA7;

	printf("F2: Position Test\n"
		"DF: %u, CA: %u, ICAO: %#X, PI: %#X\n", (unsigned int)f2.df,
		(unsigned int)f2.ca, (unsigned int)f2.icao, (unsigned int)f2.pi);

	getAirPos(&f2, tlat, tlng, &falt, &flat, &flng);
	printf("Alt: %d, Latitude: %f, Longitude: %f\n\n", falt, flat, flng);

	double strack;
	double sspeed, slat, slng;
	union AdsbFrame f3;
	f3.frame[13] = 0x8C;	//8C4841753A9A153237AEF0F275BE
	f3.frame[12] = 0x48;
	f3.frame[11] = 0x41;
	f3.frame[10] = 0x75;
	f3.frame[9] = 0x3A;
	f3.frame[8] = 0x9A;
	f3.frame[7] = 0x15;
	f3.frame[6] = 0x32;
	f3.frame[5] = 0x37;
	f3.frame[4] = 0xAE;
	f3.frame[3] = 0xF0;
	f3.frame[2] = 0xF2;
	f3.frame[1] = 0x75;
	f3.frame[0] = 0xBE;

	tlat = 51.990;		//reference for F3 test
	tlng = 4.375;

	switch(getSurfPos(&f3, tlat, tlng, &strack, &sspeed, &slat, &slng))
	{
	case 1:
		printf("track invalid\n");
		break;
	case 2:
		printf("speed invalid\n");
		break;
	case 3:
		printf("track and speed invalid\n");
	}

	printf("F3: Surface Position Test\n"
		"DF: %u, CA: %u, ICAO: %#X, PI: %#X\n"
		"Track: %f, Speed: %f,\nLatitude: %f, Longitude %f\n\n",
		(unsigned int)f3.df, (unsigned int)f3.ca, (unsigned int)f3.icao,
		(unsigned int)f3.pi, strack, sspeed, slat, slng);

	double atrk, aspd;
	int vert;
	union AdsbFrame f4;
	f4.frame[13] = 0x8D;	//8D485020994409940838175B284F
	f4.frame[12] = 0x48;	//Air Vel sub-type 1
	f4.frame[11] = 0x50;
	f4.frame[10] = 0x20;
	f4.frame[9] = 0x99;
	f4.frame[8] = 0x44;
	f4.frame[7] = 0x09;
	f4.frame[6] = 0x94;
	f4.frame[5] = 0x08;
	f4.frame[4] = 0x38;
	f4.frame[3] = 0x17;
	f4.frame[2] = 0x5B;
	f4.frame[1] = 0x28;
	f4.frame[0] = 0x4F;

	switch(getAirVel(&f4, &atrk, &aspd, &vert))
	{
	case 0:
		break;
	default:
		printf("return value error\n");
	}

	printf("F4.1: Airborne Velocity Test\n"
		"DF: %u, CA: %u, ICAO: %#X, PI: %#X\n"
		"Track: %f, Speed: %f, Vertical Rate: %d\n",
		(unsigned int)f4.df, (unsigned int)f4.ca, (unsigned int)f4.icao,
		(unsigned int)f4.pi, atrk, aspd, vert);

	double atrk2, aspd2;
	int vert2;
	union AdsbFrame f42;
	f42.frame[13] = 0x8D;	//8DA05F219B06B6AF189400CBC33F
	f42.frame[12] = 0xA0;	//subtype 3
	f42.frame[11] = 0x5F;
	f42.frame[10] = 0x21;
	f42.frame[9] = 0x9B;
	f42.frame[8] = 0x06;
	f42.frame[7] = 0xB6;
	f42.frame[6] = 0xAF;
	f42.frame[5] = 0x18;
	f42.frame[4] = 0x94;
	f42.frame[3] = 0x00;
	f42.frame[2] = 0xCB;
	f42.frame[1] = 0xC3;
	f42.frame[0] = 0x3F;

	switch(getAirVel(&f42, &atrk2, &aspd2, &vert2))
	{
	case 1:
	case 2:
		break;
	default:
		printf("return value error\n");
	}

	printf("F4.2: Airborne Velocity Test\n"
		"DF: %u, CA: %u, ICAO: %#X, PI: %#X\n"
		"Track: %f, Speed: %f, Vertical Rate: %d\n",
		(unsigned int)f42.df, (unsigned int)f42.ca, (unsigned int)f42.icao,
		(unsigned int)f42.pi, atrk2, aspd2, vert2);

#ifdef MAPPING
	printf("\nGMT MAPPING TEST\n\n");
#endif

	return 0;
}

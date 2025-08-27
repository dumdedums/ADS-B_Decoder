#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "adsb.h"
#include "decode.h"

int main(int argc, char *argv[])
{
	union AdsbFrame f1;
	char f1call[9], f1type[8];
	double f1lat, f1lng, f1trk, f1spd;
	int f1alt, f1vr;

	//O'Hare Airport as relative position, unless specified in args
	double rlat = 41.978611, rlng = -87.904722;

	//detect whether we are getting piped input
	//getopts();

	//piped input from rtl_adsb handler
	while(1)
	{
		if(scanf("%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx\
			%2hhx%2hhx%2hhx",
			&f1.frame[13], &f1.frame[12], &f1.frame[11],
			&f1.frame[10], &f1.frame[9], &f1.frame[8],
			&f1.frame[7], &f1.frame[6], &f1.frame[5], &f1.frame[4],
			&f1.frame[3], &f1.frame[2], &f1.frame[1],
			&f1.frame[0]) == EOF)
		{
			printf("Reading from buffer failed\n");
			return -1;
		}
		if((f1.df == 17 || f1.df == 18) &&	//ADS-B & TIS-B messages
			parityCheck(&f1) == 0)
		{
			switch(f1.me.id.tc)
			{
			case 1: case 2: case 3: case 4:
				getIdent(&f1, f1call, f1type);
				printf("Identification Message\nICAO: %X, \
					Callsign: %s, Aircraft Type: %s\n",
					f1.icao, f1call, f1type);
				break;

			case 5: case 6: case 7: case 8:
				getSurfPos(&f1, rlat, rlng, &f1trk, &f1spd,
					&f1lat, &f1lng);
				printf("Surface Position Message\nICAO: %X, \
					Track: %f, Speed: %f, Position: \
					%f, %f\n",
					f1.icao, f1trk, f1spd, f1lat, f1lng);
				break;

			case 9: case 10: case 11: case 12: case 13: case 14:
			case 15: case 16: case 17: case 18: case 20: case 21:
			case 22:
				getIdent(&f1, f1call, f1type);
				printf("Aerial Position Message\nICAO: %X, \
					Altitude: %d, Position: %f, %f\n",
					f1.icao, f1alt, f1lat, f1lng);
				break;

			case 19:
				getIdent(&f1, f1call, f1type);
				printf("Aerial Velocity Message\nICAO: %X, \
					Track: %f, Speed: %f, Vertical Rate: \
					%d\n",
					f1.icao, f1trk, f1spd, f1vr);
				break;

			//TODO: possible future handling of aircraft status
			}
		}
	}

	//TODO: piped input from rtl_sdr or other raw data stream

	//TODO: directly read from rtl-sdr using library

	return 0;
}

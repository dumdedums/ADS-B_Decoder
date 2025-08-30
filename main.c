#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "adsb.h"
#include "decode.h"
#include "logger.h"

/*
	main
	Runs through logs or grabs live input from RTL-SDR

	Stores a cache of a certain amount of airplanes
	with most recent position data.

	Either sends cached data directly to be turned into an image
	or stores airplane cache periodically in a log file.

	Current hex message format output to read from logs:
	"*<hex data>;\n" dump1090 hex logs might be a different format

	main [-r <lat>,<long>][-d][-c <size>][-p|-b <filename>]
	Arguments:
	-r <latitude>, <longitude>: change relative position
	-d: show debug output
	-c <size>: change airplane cache size (def: 10)
	-p <filename>: piped hex messages from logfile
	-b <filename>: piped binary stream to work with any SDR

	by default the program should use the rtl-sdr drivers to read data
	filename currently has a 20 char limit, open to change later
*/
int main(int argc, char *argv[])
{
	union AdsbFrame f1;
	char f1call[9], f1type[8];
	double f1lat, f1lng, f1trk, f1spd;
	int f1alt, f1vr;

	//cache size for planes
	int cache = 10;

	//stream to read from
	//can be a text file or potentially a named pipe
	FILE *log;
	char filename[20];

	//O'Hare Airport as relative position, unless specified in args
	double rlat = 41.978611, rlng = -87.904722;

	char debug = 0;
	char isBinary = -1;

	int opt;
	char *optstring = "rdcpb";
	//flag detection
	while((opt = getopt(argc, argv, optstring)) != -1)
	{
		switch(opt)
		{
		case 'r':
			sscanf(argv[optind++], "%lf", &rlat);
			sscanf(argv[optind++], "%lf", &rlng);
			printf("relative pos is: %f, %f\n", rlat, rlng);
			break;
		case 'd':
			debug = 1;
			printf("debug mode enabled\n");
			break;
		case 'c':
			sscanf(argv[optind++], "%d", &cache);
			printf("cache size set to %d\n", cache);
			break;
		case 'p':
			if(isBinary != -1)
			{
				printf("can't specify multiple files\n");
				return -1;
			}
			sscanf(argv[optind++], "%20s", filename);
			isBinary = 0;
			printf("filename is %s\n", filename);
			break;
		case 'b':
			if(isBinary != -1)
			{
				printf("can't specify multiple files\n");
				return -1;
			}
			sscanf(argv[optind++], "%20s", filename);
			isBinary = 1;
			printf("filename is %s\n", filename);
			break;
		case '?':
			printf("%c is not a valid option\n", optopt);
		}
	}

	struct plane *planes = (struct plane*)calloc(cache, sizeof(struct plane));

	if(isBinary == -1)
	{
		//TODO: directly read from rtl-sdr using library
		printf("not implemented yet\n");
	}
	else if(isBinary == 0)
	{
		//read input from rtl_adsb.exe data logs
		log = fopen(filename, "r");

		while(fscanf(log, " *%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx"
			"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx;", &f1.frame[13],
			&f1.frame[12], &f1.frame[11], &f1.frame[10],
			&f1.frame[9], &f1.frame[8], &f1.frame[7], &f1.frame[6],
			&f1.frame[5], &f1.frame[4], &f1.frame[3], &f1.frame[2],
			&f1.frame[1], &f1.frame[0]) != EOF)
		{
			if((f1.df == 17 || f1.df == 18) &&	//ADS-B & TIS-B messages
				parityCheck(&f1) == 0)
			{
				if(debug)
					printf("Uncorrupt Message Recieved DF: %d, TC: %d\n"
						"Raw Data: %.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
						"%.2X%.2X%.2X%.2X%.2X%.2X\n", f1.df, f1.me.id.tc,
						f1.frame[13], f1.frame[12], f1.frame[11],
						f1.frame[10], f1.frame[9], f1.frame[8],
						f1.frame[7], f1.frame[6], f1.frame[5], f1.frame[4],
						f1.frame[3], f1.frame[2], f1.frame[1],
						f1.frame[0]);

				switch(f1.me.id.tc)
				{
				case 1: case 2: case 3: case 4:
					getIdent(&f1, f1call, f1type);
					if(debug)
						printf("Identification Message\nICAO: %X, "
							"Callsign: %s, Aircraft Type: %s\n\n",
							f1.icao, f1call, f1type);
					break;

				case 5: case 6: case 7: case 8:
					if(getSurfPos(&f1, rlat, rlng, &f1trk, &f1spd,
						&f1lat, &f1lng) != 0)
					{
						if(debug)
							printf("Grnd Track or Spd Invalid\n");
					}
					if(debug)
						printf("Surface Position Message\nICAO: %X, "
							"Track: %f, Speed: %f, Position: "
							"%f, %f\n\n",
							f1.icao, f1trk, f1spd, f1lat, f1lng);
					break;

				case 9: case 10: case 11: case 12: case 13: case 14:
				case 15: case 16: case 17: case 18: case 20: case 21:
				case 22:
					if(getAirPos(&f1, rlat, rlng, &f1alt, &f1lat,
						&f1lng) != 0)
						printf("No Alt Available\n");
					if(debug)
						printf("Aerial Position Message\nICAO: %X, "
							"Altitude: %d, Position: %f, %f\n\n",
							f1.icao, f1alt, f1lat, f1lng);
					break;

				case 19:
					if(getAirVel(&f1, &f1trk, &f1spd, &f1vr) != 0)
						printf("IAS or TAS Speed\n");
					if(debug)
						printf("Aerial Velocity Message\nICAO: %X, "
							"Track: %f, Speed: %f, Vertical Rate: "
							"%d\n\n",
							f1.icao, f1trk, f1spd, f1vr);
					break;

				//TODO: possible future handling of aircraft status
				default:
					if(debug)
						printf("Status Report\nICAO: %X\n\n", f1.icao);
				}
			}
			else if(debug)
				printf("untranslated: %.2X%.2X%.2X%.2X%.2X%.2X%.2X"
					"%.2X%.2X%.2X%.2X%.2X%.2X%.2X\n\n",
					f1.frame[13], f1.frame[12], f1.frame[11],
					f1.frame[10], f1.frame[9], f1.frame[8],
					f1.frame[7], f1.frame[6], f1.frame[5], f1.frame[4],
					f1.frame[3], f1.frame[2], f1.frame[1],
					f1.frame[0], f1.frame[1], f1.frame[0]);
		}
	}
	else
	{
		//TODO: piped input from rtl_sdr or other raw data stream
		printf("not implemented yet\n");
	}
	printf("Reading complete\n");
	fclose(log);

	return 0;
}

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "adsb.h"
#include "decode.h"
#include "logger.h"

static FILE *logstream = NULL;
static FILE *savestream = NULL;

/*
	termination
	This function is a special termination handler
	if we need to do cleanup when terminating program.

	This might be necessary because the default handling
	for most termination signals closes all files
*/
/*static void term_handler(int sig)
{
	printf("termination handler running\n");
	if(logstream != NULL)
		fclose(logstream);
	if(savestream != NULL)
		fclose(savestream);
	signal(sig, SIG_DFL);
	raise(sig);
	return;
}*/

/*
	main
	Runs through logs or grabs live input from RTL-SDR

	Stores a cache of a certain amount of airplanes
	with most recent position data.

	Either sends cached data directly to be turned into an image
	or stores airplane cache periodically in a log file.

	Current hex message format output to read from logs:
	"*<hex data>;\n" dump1090 hex logs might be a different format

	main [-r <lat>,<long>][-d][-c <size>][-p|-b <filename>][-s <filename>]
	Arguments:
	-r <latitude>, <longitude>: change relative position
	-d: show debug output
	-c <size>: change airplane cache size (def: 10)
	-p <filename>: piped hex messages from named pipe or log file
	-b <filename>: piped binary stream to work with any SDR

	by default the program should use the rtl-sdr drivers to read data
	filename currently has a 20 char limit, open to change later

	In order to accept input from stdin,
	filename should be swapped for "-" for now.
	This is the same syntax that dump1090 uses,
	not that I'm copying them, it's just more standardized.
	I originally thought of using "stdin" as the keyword,
	the problem is that "stdin" could be a real filename.
	Well "-" could be a filename also.
*/
int main(int argc, char *argv[])
{
	union AdsbFrame f1;
	char f1call[9], f1type[8];
	double f1lat, f1lng, f1trk, f1spd;
	int f1alt, f1vr;
	register enum PlaneFlags f1fl;

	//cache size for planes
	int cache = 10;

	//stream to read from
	//can be a text file or potentially a named pipe
	char filename[20], savename[20];
	filename[0] = 0;
	savename[0] = 0;

	//O'Hare Airport as relative position, unless specified in args
	double rlat = 41.978611, rlng = -87.904722;

	char debug = 0;
	char isBinary = -1;
	time_t lastLog;

	int opt;
	char *optstring = "rdcpbs";

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
				printf("can't specify multiple input files\n");
				return -1;
			}
			sscanf(argv[optind++], "%20s", filename);
			isBinary = 1;
			printf("filename is %s\n", filename);
			break;
		case 's':
			sscanf(argv[optind++], "%20s", savename);
			printf("save file is %s\n", savename);
			break;
		case '?':
			printf("%c is not a valid option\n", optopt);
		}
	}

	struct Plane *planes = (struct Plane*)calloc(cache, sizeof(struct Plane));

	if(savename[0] != 0)
		savestream = fopen(savename, "a");

	if(isBinary == -1)
	{
		//TODO: directly read from rtl-sdr using library
		printf("not implemented yet\n");
	}
	else if(isBinary == 0)
	{
		//read input from rtl_adsb.exe data logs
		if(filename[0] == '-' && filename[1] == 0)
			logstream = stdin;
		else
			logstream = fopen(filename, "r");

		time(&lastLog);

		while(fscanf(logstream, " *%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx"
			"%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx;", &f1.frame[13],
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

					logPlane(planes, cache, f1.icao, f1call,
						f1type, f1lat, f1lng, f1trk, f1spd,
						f1alt, f1vr, ICAOFL | IDENTVALID);
					break;

				case 5: case 6: case 7: case 8:
					f1fl = ICAOFL | POSVALID | TRKVALID | SPDVALID;
					switch(getSurfPos(&f1, rlat, rlng, &f1trk, &f1spd,
						&f1lat, &f1lng))
					{
					case 3:
						f1fl -= TRKVALID;
					case 2:
						f1fl -= SPDVALID;
						break;
					case 1:
						f1fl -= TRKVALID;
					}
					if(debug)
						printf("Surface Position Message\nICAO: %X, "
							"Track: %f, Speed: %f, Position: "
							"%f, %f\n\n",
							f1.icao, f1trk, f1spd, f1lat, f1lng);

					logPlane(planes, cache, f1.icao, f1call,
						f1type, f1lat, f1lng, f1trk, f1spd,
						f1alt, f1vr, f1fl);
					break;

				case 9: case 10: case 11: case 12: case 13: case 14:
				case 15: case 16: case 17: case 18: case 20: case 21:
				case 22:
					f1fl = ICAOFL | POSVALID | ALTVALID;
					switch(getAirPos(&f1, rlat, rlng,
						&f1alt, &f1lat, &f1lng))
					{
					case 1:
						f1fl -= ALTVALID;
					}
					if(debug)
						printf("Aerial Position Message\nICAO: %X, "
							"Altitude: %d, Position: %f, %f\n\n",
							f1.icao, f1alt, f1lat, f1lng);

					logPlane(planes, cache, f1.icao, f1call,
						f1type, f1lat, f1lng, f1trk, f1spd,
						f1alt, f1vr, f1fl);
					break;

				case 19:
					f1fl = ICAOFL | TRKVALID | SPDVALID | VERTVALID;
					switch(getAirVel(&f1, &f1trk, &f1spd, &f1vr))
					{
						case 0:
							break;
						case 10:
							f1fl -= TRKVALID;
							f1fl -= SPDVALID;
							break;
						case 14:
						case 13:
							f1fl -= TRKVALID;
						case 12:
						case 11:
							f1fl |= ICAOFL;
							f1fl -= SPDVALID;
							break;
						case 15:
							f1fl = ICAOFL;
							break;
						case 9:
						case 8:
							f1fl -= TRKVALID;
						case 7:
						case 6:
							f1fl |= ICAOFL;
						case 5:
							f1fl -= VERTVALID;
							break;
						case 19:
						case 18:
							f1fl -= TRKVALID;
						case 17:
						case 16:
							f1fl -= SPDVALID;
							f1fl -= VERTVALID;
							f1fl |= IASFL;
							break;
						case 3:
						case 4:
							f1fl -= TRKVALID;
						case 1:
						case 2:
							f1fl |= IASFL;
						default:
							f1fl = 0;
							break;
					}
					if(debug)
						printf("Aerial Velocity Message\nICAO: %X, "
							"Track: %f, Speed: %f, Vertical Rate: "
							"%d\n\n",
							f1.icao, f1trk, f1spd, f1vr);

					logPlane(planes, cache, f1.icao, f1call,
						f1type, f1lat, f1lng, f1trk, f1spd,
						f1alt, f1vr, f1fl);
					break;

				//TODO: possible future handling of aircraft status
				default:
					if(debug)
						printf("Status Report\nICAO: %X\n\n", f1.icao);
				}

				//more efficient to do this in separate thread but whatever
				//displays data every 5 seconds and writes to log file
				if(difftime(time(NULL), lastLog) > 5.)
				{
					time(&lastLog);
					updateDisplay(planes, cache);
					if(savestream)
						logToFile(planes, cache, savestream);
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
	updateDisplay(planes, cache);
	if(savestream)
	{
		logToFile(planes, cache, savestream);
		fclose(savestream);
	}
	fclose(logstream);
	free(planes);

	return 0;
}

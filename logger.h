#pragma once
#include <time.h>

/*
	LOGGER.H
	This contains all functions related to logging airplanes
	into formatted files.
	This also means that it contains structs to represent planes,
	drawing functions to draw currently tracked airplanes to the screen,
	and eventually calling the utility to create maps with
	airplane locations overlayed.
*/

/*
	enum PlaneFlags
	I created this enum to make it very easy to log plane data.
	I can call log and submit every local plane value, even if invalid.
	The function will only update those with valid flags.

	Then the function will make Plane.pflags |= flags.

	When a new plane replaces and old one the pflags are wiped and
	we don't need to wipe all the data values.
	Only reason calloc is used in main is so I don't have to individually
	wipe pflags at start.
*/

extern int changeTimeOnPosition;
extern double rlat, rlng;

enum PlaneFlags {ICAOFL=1, IDENTVALID=2, POSVALID=4, TRKVALID=8,
	SPDVALID=16, ALTVALID=32, VERTVALID=64, IASFL=128, BARFL=256};

struct Plane
{
	//identificaton info
	int icao;
	char call[9];
	char type[8];

	//positional data
	double lat, lng, trk, spd;
	int alt, vert;

	//time of last update to airplane
	//need for clearing cache
	time_t lstUpd;

	//planeflags used for displaying data
	enum PlaneFlags pflags;
};

/*
	logPlane
	This is definitely slower than making 5 or so different log functions.
	Either way the planeflags need to be set, and often need to be checked
	within certain messages anyway.

	This function also will automatically delete the oldest plane and
	replace it with the newest plane if the buffer is full.
*/
void logPlane(struct Plane buf[], int bufsize, int icao, char call[9],
	char type[8], double lat, double lng, double trk, double spd,
	int alt, int vert, enum PlaneFlags fl);

/*
	updateDisplay
	This function will print out all the planes being tracked.
	Should be called every 10 sec or so.
	In the future we should use ncurses or some type of termio
	instead of just printing to the console.
*/
void updateDisplay(const struct Plane buf[], int bufsize);

/*
	logToFile
	Same thing as updateDisplay but to a file
	They will probably run the same helper function.
*/
void logToFile(const struct Plane buf[], int bufsize, FILE *save);

/*
	readLog
	Reads a log file and returns the planes.
	TODO: TESTING
*/
int readLog(FILE *log, struct Plane **planes);

#ifdef MAPPING
/*
	Mapping info
	Dataset has a datatable, which has a datasegment per ICAO code.
	Each record will have 3 Cols and a Text field.
	The text field should contain the time (24hr format), ICAO code,
	and callsign if it has it. If there is a callsign callsign can
	replace ICAO code in text field.
	LON  LAT  ALT  TEXT
*/
/*
	createImage
	On first run we must startup the GMT API

	The Plane data can be the instantaneous data or a collection of
	data from the past (although duplicates should be removed).

	localtime may potentially be used at the same time as updateDisplay
	if they are running on seperate threads. Keep this in mind if adding
	multithreading.
*/
void createImage(const struct Plane buf[], const int bufsize);

/*
	endGMTSession
	print out plot in final output format(s) and exit
*/
void endGMTSession();
#endif

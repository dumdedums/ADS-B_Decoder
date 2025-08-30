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

struct plane
{
	//identificaton info
	unsigned int ICAO;
	char call[9];
	char type[8];

	//positional data
	double lat, lng, trk, spd;
	int alt, vert;

	//time of last update to airplane
	//need for clearing cache
	time_t lstUpd;

	//for determining if IAS/TAS speed + HDG used
	//and also if baro alt is used instead of GNSS height
	char ias	: 1;
	char baro	: 1;

	//time of last update to airplane
	//need for clearing cache
};

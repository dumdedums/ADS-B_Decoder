#pragma once
#include "adsb.h"

/*
	DECODE.H
	This file contains all functions related to getting usable,
	loggable, information off of the ADS-B broadcast messages.

	Keep in mind, everything but the ADS-B frames is pretty much an output.
*/

/*
	parityCheck
	Returns a 0 if there are no problems with the ADS-B frame,
	Returns a 1 if there was a correctable error,
	Returns a -1 if there was an noncorrectable error.

	TODO: Possibly use SSE/AVX and AArch64 equivalent for computing
	parities faster.
*/
int parityCheck(const union AdsbFrame *frame);

/*
	getIdent
	Returns 0 if no errors
	Gets call sign and aircraft type
*/
int getIdent(const union AdsbFrame *frame, char call[8], char type[8]);

/*
	getAirPos
	Returns a 0 if there are no problems.

	Will most likely always return altitude in ft, since that is customary.

	May need two frames to find latitude and longitude,
	but we also can possibly use a local reference,
	so there may be different versions of the function.
*/
int getAirPos(const union AdsbFrame *frame, int *alt, double *lat, *lng);

/*
	getSurfPos
	Returns 0 if there are no problems.
	Returns 1 if ground track is invalid.
	Returns other value for other problems.

	Track is a rounded set of degrees,
	the actual value only has a precision of 360/128,
	or about 2.8 degrees anyway.
*/
int getSurfPos(const union AdsbFrame *frame, int *trk, int *spd, double *lat, *lng);

/*
	getAirVel
	Returns 0 if speed is a ground speed.
	Returns 1 if speed is IAS.
	Returns 2 if speed is TAS.

	Speed is almost always given as ground speed.
	In the air, the track becomes a heading,
	so it shows where the aircraft is pointed, not where it's going.

	Also the magnetic heading is in 360/1024 degrees of accuracy,
	so more accurate than the ground track listed previously.

	The track calculated from ground speed is as accurate as
	the E-W and N-S velocities obtained from GNSS.
*/
int getAirVel(const union AdsbFrame *frame, double *trk, int *spd);

#pragma once
#include "adsb.h"

/*
	DECODE.H
	This file contains all functions related to getting usable,
	loggable, information off of the ADS-B broadcast messages.

	Keep in mind, everything but the ADS-B frames is pretty much an output.

	Positive return values indicate that the function was able to be completed,
	although possibly with different or missing information.

	Negative return values indicate the function was not able to complete,
	for most functions, -1 indicates that the type code is not correct.
*/

/*
	parityCheck
	Returns a 0 if there are no problems with the ADS-B frame,
	Returns a 1 if there was a correctable error,
	Returns a -1 if there was an noncorrectable error.

	Keep in mind, the "parity" is a CRC remainder,
	and this function is actually a CRC computation.

	TODO: Possibly use SSE/AVX and AArch64 equivalent for computing
	CRC faster. Or possibility of using lookup tables and other means
	of computing CRC faster.
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
	Returns 1 if no altitude available.

	Will most likely always return altitude in ft, since that is customary.

	May need two frames to find latitude and longitude,
	but we also can possibly use a local reference,
	so there may be different versions of the function.

	TODO: Gray code conversion for baro alts >50175ft
*/
int getAirPos(const union AdsbFrame *frame, int *alt, double *lat, *lng);

/*
	getSurfPos
	Returns 0 if there are no problems.
	Returns 1 if ground track is invalid.
	Returns 2 if speed invalid.
	Returns 3 if ground track and speed invalid.

	Track is a rounded set of degrees,
	the actual value only has a precision of 360/128,
	or about 2.8 degrees anyway.

	Highest readable ground speed is 174kts.
	If spd == 175, then it is most likely >175.
*/
int getSurfPos(const union AdsbFrame *frame, int *trk, int *spd, double *lat, *lng);

/*
	getAirVel
	Returns 0 if speed is a ground speed.
	Returns 1 if speed is IAS.
	Returns 2 if speed is TAS.
	Returns 3 if IAS and HDG not available.
	Returns 4 if TAS and HDG not available.
	Returns 5 + other codes if vertical rate unavailable.
	(5: GS/NoVR, 6: IAS/NoVR ...)
	Returns 10+ if speed/track not available.
	(10: no spd/trk, 15: no spd/trk/vr)
	Returns -2 if subtype invalid.

	Speed is almost always given as ground speed.
	With IAS/TAS, the track becomes a heading,
	so it shows where the aircraft is pointed, not where it's going.

	Also the magnetic heading is in 360/1024 degrees of accuracy,
	so more accurate than the ground track listed previously.

	The track calculated from ground speed is as accurate as
	the E-W and N-S velocities obtained from GNSS.

	Vertical rate is in ft/min, as is standard, since alt is also in ft.

	There are many questions left about the altitude difference,
	the baro altitude above a certain FL (Flight Level),
	usually FL180 (18000ft) is given in relation to a standard atmosphere,
	and therefore can vary significantly from GNSS height.
	Aircraft also abide by the FL as opposed to GNSS height,
	so when mapping I will probably want to use FL.
	On the other hand, GNSS height is definitely more accurate.

	Currently no altitude difference is reported.
*/
int getAirVel(const union AdsbFrame *frame, double *trk, int *spd, int *vr);

#pragma once
#include <stdint.h>

/*
	ADSB.h
	This file has struct definitions for the ADS-B frames and
	their associated objects.
*/

/*
	AdsbMessage
	In order to insure allignment with the message within the adsbFrame struct,
	I have created a separate union of the message portion in all of its forms.

	Since the messages are sent MSB (Most Significant Bit) first,
	Bit-fields in structs are usually ordered in the LSB first.
	So I input the 112 bit raw data frame Little Endian,
	then I reverse the order of the bit-fields so that they align correctly.

	ADS-B version differences:
	TC=28 added in ver 1
	NUC (Navigational Uncertainty Categories) swapped in favor of NIC (Navigational Integrity Categories)
	TC=28 changed in ver 2, NICb defined in Air Pos messages (TC=9-18)
	NICa and NICc defined in TC=31

	I probably will not worry about the navigational uncertainty/integrity,
	because it requires knowing ADS-B versions of the transponder.
	Part of the navigational uncertainty/integrity is encoded into the typecode,
	which is why there are multiple type codes for certain messages.
*/

/*
	SubSpecFields (deprecated)
	These are the specific fields for ground speed (gs), or airborne speed (as),
	as they relate within the Airborne Velocity (av) struct and message type.
*/

/*union SubSpecFields
{
	struct __attribute__((packed))
	{
		uint32_t vns	: 10;	//North-South velocity
		uint32_t dns	: 1;	//N-S direction (0 South to North, 1 North to South)
		uint32_t vew	: 10;	//E-W velocity
		uint32_t dew	: 1;	//E-W direction (0 West to East, 1 East to West)
	} gs;	//if subtype is 1, speed is decimal value - 1
		//if st=2, speed is 4 * (dec value - 1)
		//in knots

	struct __attribute__((packed))
	{
		uint32_t as	: 10;	//speed = as - 1 or 4*(as-1) (0 means not available)
		uint32_t t	: 1;	//type 0 = IAS, 1 = TAS
		uint32_t hdg	: 10;	//heading = hdg * 360/1024 degrees
		uint32_t sh	: 1;	//heading status (0 means hdg not available)
	} as;	//st=4 is the same as st=2, airpseed is multiplied by 4.
};*/

union AdsbMessage
{
	uint8_t me[7];	//Message (56 bits including TC)

	struct __attribute__((packed))
	{
		uint64_t c8	: 6;
		uint64_t c7	: 6;
		uint64_t c6	: 6;
		uint64_t c5	: 6;
		uint64_t c4	: 6;
		uint64_t c3	: 6;	//of an ASCII character
		uint64_t c2	: 6;	//each char is the lower 6 bits
		uint64_t c1	: 6;	//call sign chars
		uint64_t cat	: 3;	//craft category is determined by both this and exact tc value
		uint64_t tc	: 5;
	} id;	//to be used if tc <= 4

	struct __attribute__((packed))
	{
		uint64_t loncpr		: 17;	//longitude
		uint64_t latcpr		: 17;	//CPR encoded latitude
		uint64_t f		: 1;	//CPR format (even frame or odd frame)
		uint64_t t		: 1;	//time
		uint64_t alt		: 12;	//encoded altitude
		uint64_t saf		: 1;	//single antenna flag
		uint64_t ss		: 2;	//surveillance status
		uint64_t tc		: 5;
	} ab;	//layout of message if of airborne position type
		//used if tc is 9-18 or 20-22
		//alt is baro if 9-18, GNSS alt 20-22
		//Keep in mind, not only is the baro alt in ft and GNSS in meters
		//the 8th bit of the alt for baro indicates whether the altitude is
		//in 25ft increments or 100ft increments.
	
	struct __attribute__((packed))
	{
		uint64_t loncpr		: 17;	//longitude
		uint64_t latcpr		: 17;	//latitude (calculated differently from airborne)
		uint64_t f		: 1;	//even or odd CPR format
		uint64_t t		: 1;	//time
		uint64_t trk		: 7;	//ground track (degrees) = 360*trk / 128
		uint64_t s		: 1;	//ground track status (invalid or valid)
		uint64_t mov		: 7;	//movement (ground speed)
		uint64_t tc		: 5;
	} sp;	//surface position tc 5-8

	struct __attribute__((packed))
	{
		uint64_t diff	: 7;	//GNSS - Baro alt (divided by 25ft)
		uint64_t sdif	: 1;	//sign bit for GNSS alt - Baro alt
		uint64_t res	: 2;
		uint64_t vr	: 9;	//vertical rate
		uint64_t svr	: 1;	//vertical rate sign bit
		uint64_t src	: 1;	//source for vertical rate (GNSS or Baro)
		uint32_t vns	: 10;	//North-South velocity
		uint32_t dns	: 1;	//N-S direction (0 South to North, 1 North to South)
		uint32_t vew	: 10;	//E-W velocity
		uint32_t dew	: 1;	//E-W direction (0 West to East, 1 East to West)
		uint64_t nuc	: 3;	//navigational uncertainty, different between ADS-B versions
		uint64_t ifr	: 1;	//IFR capability flag
		uint64_t ic	: 1;	//intent change flag
		uint64_t st	: 3;	//subtypes 1 and 2 are ground speed, 3 and 4 are TAS or IAS
		uint64_t tc	: 5;
	} avg;	//airborne velocities tc=19 ground speed specific

	struct __attribute__((packed))
	{
		uint64_t diff	: 7;	//GNSS - Baro alt (divided by 25ft)
		uint64_t sdif	: 1;	//sign bit for GNSS alt - Baro alt
		uint64_t res	: 2;
		uint64_t vr	: 9;	//vertical rate
		uint64_t svr	: 1;	//vertical rate sign bit
		uint64_t src	: 1;	//source for vertical rate (GNSS or Baro)
		uint32_t as	: 10;	//speed = as - 1 or 4*(as-1) (0 means not available)
		uint32_t t	: 1;	//type 0 = IAS, 1 = TAS
		uint32_t hdg	: 10;	//heading = hdg * 360/1024 degrees
		uint32_t sh	: 1;	//heading status (0 means hdg not available)
		uint64_t nuc	: 3;	//navigational uncertainty, different between ADS-B versions
		uint64_t ifr	: 1;	//IFR capability flag
		uint64_t ic	: 1;	//intent change flag
		uint64_t st	: 3;	//subtypes 1 and 2 are ground speed, 3 and 4 are TAS or IAS
		uint64_t tc	: 5;
	} ava;	//airborne velocities tc=19 IAS/TAS specific

	struct __attribute__((packed))
	{
		uint64_t res	: 1;
		uint64_t sils	: 1;	//SIL supplement bit
		uint64_t hrd	: 1;	//Horizontal Reference Direction
		uint64_t bai	: 1;	//Baro Alt Integrity (Track Angle on surface)
		uint64_t sil	: 2;	//Source Integrity Level
		uint64_t gva	: 2;	//Geometric Vertical Accuracy (unused on surface)
		uint64_t nacp	: 4;	//Navigational Accuracy Category - position
		uint64_t nica	: 1;	//NIC supplement - A
		uint64_t ver	: 3;	//ADS-B version
		uint64_t om	: 16;	//operational machine code
		uint64_t cc	: 16;	//capacity class code
		uint64_t st	: 3;	//subtype (0 airborne, 1 surface)
		uint64_t tc	: 5;
	} os;	//operational status tc=31
		//ADS-B version 1 doesn't have SILS, ADS-B ver 0 is very different but unused
		//we might not worry about decoding these messages
};

/*
	AdsbFrame
	This union allows access to the 112 bit ADS-B broadcast frame as either
	7 16 bit integers, or the individual components thereof.

	WARNING: The ADS-B messages are transmitted in MSB first,
	the alignment between assigning the 16 bit integers and the data within
	might be incorrect at this moment.
	AFAIK bit-fields are LsB first, so I might have to swap everything at some point.

	Downlink Format:
	17 if sent from a Mode S transponder,
	18 if sent from TIS-B and non-transponders.
	Should be irrelevant for our use.

	Capability:
	0: Level 1 transponder
	1-3: Reserved
	4: Level 2+ transponder, can set CA to 7 on ground
	5: Level 2+ transponder, can set CA to 7 airborne
	6: Level 2+ transponder, can set CA to 7 both
	7: Downlink Request value is 0, flight status 2, 3, 4, or 5
	Should also be irrelevant.

	ICAO Address:
	Unique hex identifier for each aircraft.
	Important to be able to update aircraft positions,
	as call signs may not be as unique.

	Type Codes:
	1-4 Identification
	5-8 Surface Pos
	9-18 Airborne Pos (Baro Alt)
	19 Airborne Vel
	20-22 Airborne Pos (GNSS Height)
	23-27 Reserved
	28 Aircraft Status
	29 Target State and Status
	31 Aircraft Op Status
*/
union AdsbFrame
{
	uint8_t frame[14];	//A broadcast frame is 112 bits long, or 8 bits * 14

	struct __attribute__((packed))
	{
		uint32_t pi	: 24;	//Parity and Interrogator ID
		union AdsbMessage me;	//Message (as described above)
		uint32_t icao	: 24;	//ICAO aircraft address
		uint8_t ca	: 3;	//Transponder Capability
		uint8_t df	: 5;	//Downlink Format
	};
};

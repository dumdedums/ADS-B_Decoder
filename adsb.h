/*
	ADSB.h
	This file has struct definitions for the ADS-B frames and
	their associated objects.
*/

#pragma once
#include <stdint.h>

/*
	adsbMessage
	In order to insure allignment with the message within the adsbFrame struct,
	I have created a separate union of the message portion in all of its forms.

	I have worries about how this will align within the adsbFrame struct,
	I want to be able to paste the raw adsbFrame into the adsbFrame.frame,
	then access individual parts without bit masking and shifting.
	This is arguably greedy and it might be better and more compatible between compilers
	and operating systems to do all the bit masking and shifting in the functions that handle it,
	using helper functions or macro functions even.

	Since the messages are sent MSB (Most Significant Bit) first,
	I might have to completely reverse my ordering of struct variables.
	Bit-fields in structs are usually ordered in the LSB first.
*/
union adsbMessage
{
	uint64_t me;	//Message (raw 51 bits without TC)

	struct __attribute__((packed))
	{
		uint64_t cat	: 3;	//craft category is determined by both this and exact tc value
		uint64_t c1	: 6;	//call sign chars
		uint64_t c2	: 6;	//each char is the lower 6 bits
		uint64_t c3	: 6;	//of an ASCII character
		uint64_t c4	: 6;
		uint64_t c5	: 6;
		uint64_t c6	: 6;
		uint64_t c7	: 6;
		uint64_t c8	: 6;
	} id;	//to be used if tc <= 4

	struct __attribute__((packed))
	{
		uint64_t ss		: 2;	//surveillance status
		uint64_t sas		: 1;	//single antenna flag
		uint64_t alt		: 12;	//encoded altitude
		uint64_t t		: 1;	//time
		uint64_t f		: 1;	//CPR format (even frame or odd frame)
		uint64_t lat-cpr	: 17;	//CPR encoded latitude
		uint64_t lon-cpr	: 17;	//longitude
	} ab;	//layout of message if of airborne position type
		//used if tc is 9-18 or 20-22
		//alt is baro if 9-18, GNSS alt 20-22
		//Keep in mind, not only is the baro alt in ft and GNSS in meters
		//the 8th bit of the alt for baro indicates whether the altitude is
		//in 25ft increments or 100ft increments.
	
	struct __attribute__((packed))
	{
		uint64_t mov		: 7;	//movement (ground speed)
		uint64_t s		: 1;	//ground track status (invalid or valid)
		uint64_t trk		: 7;	//ground track (degrees) = 360*trk / 128
		uint64_t t		: 1;	//time
		uint64_t f		: 1;	//even or odd CPR format
		uint64_t lat-cpr	: 17;	//latitude (calculated differently from airborne)
		uint64_t lon-cpr	: 17;	//longitude
	} sp;	//surface position tc 5-8

	struct __attribute__((packed))
	{
		uint64_t st	: 3;	//subtypes 1 and 2 are ground speed, 3 and 4 are TAS or IAS
		uint64_t ic	: 1;	//intent change flag
		uint64_t ifr	: 1;	//IFR capability flag
		uint64_t nuc	: 3;	//navigational uncertainty, different between ADS-B versions
		uint64_t stsf	: 22;	//subtype specific info
		uint64_t src	: 1;	//source for vertical rate (GNSS or Baro)
		uint64_t svr	: 1;	//vertical rate sign bit
		uint64_t vr	: 9;	//vertical rate
		uint64_t res	: 2;
		uint64_t sdif	: 1;	//sign bit for GNSS alt - Baro alt
		uint64_t diff	: 7;	//GNSS - Baro alt (divided by 25ft)
	} av;	//airborne velocities tc=19
};

/*
	adsbFrame
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
union adsbFrame
{
	uint16_t frame[7];	//A broadcast frame is 112 bits long, or 16 bits * 7

	struct __attribute__((packed))
	{
		uint8_t df	: 5;	//Downlink Format
		uint8_t ca	: 3;	//Transponder Capability
		uint32_t icao	: 24;	//ICAO aircraft address
		uint8_t tc	: 5;	//Type Code (type of message)
		union adsbMessage : 51;	//Message (as described above)
		uint32_t pi	: 24;	//Parity and Interrogator ID
	};
};

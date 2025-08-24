#include "decode.h"
#include <math.h>
#include <string.h>

#define CRC_GEN 0x1FFF409u	//generator for CRC parity check 'u' means unsigned
#define PI 3.141592653589793	//Pi for converting radians to degrees
#define Nz 15.			//num latitude zones

/*
	cprDecode
	Returns 0 upon success
	lat and lng are the return variables
	tf (Type Flag): 0 for airborne, 1 for surface

	format, lon-cpr, and lat-cpr bits are in the same part of frame for
	airborne vs surface messages

	This follows the equations used in Junzi Sun's The 1090Mhz Riddle,
	which he probably got from the NASA paper on Compact Position Reporting (CPR).

	dLat = 360/(4Nz-[odd bit]) for airborne, while dLat = 90/(4Nz-odd) on surf.
	also dLon = 90/n as opposed to whatever is needed for airborne.
	
	dLat is fixed based on whether we are using air or surface position,
	along with even or odd message bit.
	dLng changes with latitude, as latitudinal parallels get shorter near
	north and south poles.

	Longitudes in calculations are from 0-360 degrees,
	as opposed to -180 to 180 degrees.
*/
static int cprDecode(const union AdsbFrame *frame, double rlat, double rlng,
	int tf, double *lat, double *lng)
{
	double dlat, dlng;	//dlat: lat zone size, dlng: long zone size
	int j, m;		//j: lat zone index, m: long zone index
	int NL;			//number of long zones, based on lat
	if(rlng < 0.)
		rlng += 360.;

	//lat-cpr and lon-cpr are going to now be lat and lng
	*lat = (double)frame->me.ab.latcpr / 131072.;	//131072 = 2^17
	*lng = (double)frame->me.ab.loncpr / 131072.;

	dlat = (360. / (4.*Nz - (double)frame->me.ab.f)) / ((double)tf * 3. + 1.);
	j = (int)(floor(rlat / dlat) + floor(fmod(rlat, dlat) /
		dlat - *lat + 0.5));
	*lat = dlat * ((double)j + *lat);

	if(*lat == 0.)		//edge cases
		NL = 59;
	else if(*lat > 87. || *lat < -87.)
		NL = 1;
	else			//replace function with lookup table eventually
		NL = (int)floor(2.*PI / acos(1. - (1.-cos(PI/(2.*Nz))) /
			pow(cos(*lat * PI/180), 2.)));

	dlng = (360. / fmax(1., (double)NL - (double)frame->me.ab.f)) / ((double)tf * 3. + 1.);
	m = (int)(floor(rlng / dlng) + floor(fmod(rlng, dlng) /
		dlng - *lng + 0.5));
	*lng = dlng * ((double)m + *lng);
	if(*lng > 180.)
		*lng = *lng - 360.;

	return 0;
}

int parityCheck(const union AdsbFrame *frame)
{
	uint8_t data[14];
	int i, b, o;
	data[0] = 0;	//will be overwritten by parity code later
	data[1] = 0;
	data[2] = 0;
	for(i = 0;i < 11;i++)	//lowest 24 bits of frame is parity code
		data[i] = frame->frame[i + 3];

	i = 87;		//data bit from 0-87
	b = 13;		//data[] block from 3-13
	o = 7;		//offset bit from 0-7
	do
	{
		if(data[b] & (1 << o))	//checks if leftmost data bit is 1
		{	//bit shifts are done after add/sub, but before bitwise AND
			//25 greatest bits of data XOR CRC_GEN operation
			//since the 25 bits are split between 3 uint8_ts,
			//a little bit of math is done to align the CRC_GEN
			//right shift CRC_GEN by 17-24 bits depending on o for data[b]
			//last data block requires CRC_GEN left shift alignment
			data[b] = data[b] ^ (uint8_t)((CRC_GEN >> 24 - o) & 0xFF);
			data[b-1] = data[b-1] ^ (uint8_t)((CRC_GEN >> 16 - o) & 0xFF);
			data[b-2] = data[b-2] ^ (uint8_t)((CRC_GEN >> 8 - o) & 0xFF);
			data[b-3] = data[b-3] ^ (uint8_t)((CRC_GEN << 7 - o) & 0xFF);
		}
		//the XOR operation is guaranteed to make the leftmost bit 0
		i--;
		b = i / 8 + 3;
		o = i % 8;
	} while(i >= 0);

	if(data[0] == frame->frame[0] && data[1] == frame->frame[1] &&
		data[2] == frame->frame[2])
		return 0;
	return -1;
}

int getIdent(const union AdsbFrame *frame, char call[8], char type[8])
{
	int i;

	if(frame->me.id.tc < 1 || frame->me.id.tc > 4)
		return -1;

	call[0] = (char)frame->me.id.c1;
	call[1] = (char)frame->me.id.c2;
	call[2] = (char)frame->me.id.c3;
	call[3] = (char)frame->me.id.c4;
	call[4] = (char)frame->me.id.c5;
	call[5] = (char)frame->me.id.c6;
	call[6] = (char)frame->me.id.c7;
	call[7] = (char)frame->me.id.c8;

	for(i = 0;i < 8;i++)
	{
		//callsign letters are ASCII chars with 7th bit cut off
		//spaces and numbers are ASCII equivalent
		//all other chars are unused, so as long as parity passes, is valid
		if(call[i] < 27)
			call[i] += 0x40;
	}

	switch(frame->me.id.tc * 010 + frame->me.id.cat)
	{
	case 021:
		strcpy(type, "SEV");	//Surface Emergency Vehicle
		break;
	case 023:
		strcpy(type, "SSV");	//Surface Service Vehicle
		break;
	case 024:
	case 025:
	case 026:
	case 027:
		strcpy(type, "GRDOBS");	//ground obstruction
		break;
	case 031:
		strcpy(type, "GLIDER");
		break;
	case 032:
		strcpy(type, "LTA");	//Lighter Than Air (hot air balloon)
		break;
	case 033:
		strcpy(type, "SKYDIV");
		break;
	case 034:
		strcpy(type, "ULTLIT");	//ultralight, hang or para glider
		break;
	case 036:
		strcpy(type, "UAV");
		break;
	case 037:
		strcpy(type, "SPACE");
		break;
	case 041:
		strcpy(type, "LIGHT");	//sub 7000kg
		break;
	case 042:
		strcpy(type, "MED1");	//7000-34000kg
		break;
	case 043:
		strcpy(type, "MED2");	//34000-136000kg
		break;
	case 044:
		strcpy(type, "HVA");	//High Vortex Aircraft (disturbs nearby)
		break;
	case 045:
		strcpy(type, "HEAVY");	//greater 136000kg
		break;
	case 046:
		strcpy(type, "HIPERF");	//>400kts >5g pulls
		break;
	case 047:
		strcpy(type, "ROTOR");	//helicopters and stuff
		break;
	default:
		strcpy(type, "UNOWEN");
	}

	return 0;
}

int getAirPos(const union AdsbFrame *frame, double rlat, double rlng, int *alt,
	double *lat, double *lng)
{
	int x = 0;	//return value for odd circumstances
	if(frame->me.ab.tc < 9 || frame->me.ab.tc > 22 || frame->me.ab.tc == 19)
		return -1;

	if(frame->me.ab.alt == 0)
		x = 1;	//no alt data
	else
	{
		if(frame->me.ab.tc < 19)
		{
			*alt = (int)(((frame->me.ab.alt & 0xFE0) >> 1) +
				(frame->me.ab.alt & 0xF));
			if(frame->me.ab.alt & 0x010)
				*alt = *alt * 25 - 1000;
			else
			{
				//TODO: gray code conversion
				//this condition only happens if alt > 50175ft
				*alt = 50175;
			}
		}
		else
		{		//round m to ft from GNSS height
			*alt = (int)round((double)frame->me.ab.alt * 3.281);
		}
	}

	cprDecode(frame, rlat, rlng, 0, lat, lng);

	return x;
}

int getSurfPos(const union AdsbFrame *frame, double rlat, double rlng,
	int *trk, double *spd, double *lat, double *lng)
{
	int x = 0;
	if(frame->me.sp.tc < 5 || frame->me.sp.tc > 8)
		return -1;

	if(frame->me.sp.s)	//actual track = TRK * 360 / 128
		*trk = (int)round((double)frame->me.sp.trk * 2.8125);
	else
		x = 1;

	//depending on the MOV value, the actual speed is based off
	//different knot increments, starting from increments of 0.125kt
	//and ending at 5kt increments between MOV values
	//MOV = 0, [125,127] are invalid values
	if(frame->me.sp.mov != 0 && frame->me.sp.mov < 125)
	{
		if(frame->me.sp.mov == 1)
			*spd = 0.;
		else if(frame->me.sp.mov < 9)
			*spd = (double)frame->me.sp.mov * 0.125 - 0.125;
		else if(frame->me.sp.mov < 13)
			*spd = (double)frame->me.sp.mov * 0.25 - 1.25;
		else if(frame->me.sp.mov < 39)
			*spd = (double)frame->me.sp.mov * 0.5 - 4.5;
		else if(frame->me.sp.mov < 94)
			*spd = (double)frame->me.sp.mov - 24.;
		else if(frame->me.sp.mov < 109)
			*spd = (double)frame->me.sp.mov * 2 - 118.;
		else
			*spd = (double)frame->me.sp.mov * 5 - 445.;
	}
	else
		x += 2;

	cprDecode(frame, rlat, rlng, 1, lat, lng);

	return x;
}

int getAirVel(const union AdsbFrame *frame, double *trk, int *spd, int *vr)
{
	int x = 0;
	int vew, vsn;
	if(frame->me.avg.tc != 19)
		return -1;

	if(frame->me.avg.st == 1 || frame->me.avg.st == 2)
	{
		if(frame->me.avg.vew == 0)
		{
			x = 10;
			goto INVALID_VEL;	//velocities invalid if 0
		}

		vew = (int)frame->me.avg.vew - 1;
		if(frame->me.avg.dew)
			vew *= -1;
		vsn = (int)frame->me.avg.vns - 1;
		if(frame->me.avg.dns)
			vsn *= -1;
		if(frame->me.avg.st == 2)	//supersonic - very rare
		{
			vew *= 4;
			vsn *= 4;
		}
		*spd = (int)round(sqrt(pow((double)vew, 2.) + pow((double)vsn, 2.)));

		//potential for errors in atan2 if velocities close to 0
		//see atan2 documentation of math.h in c std lib
		*trk = atan2((double)vew, (double)vsn) * 180. / PI;
		if(*trk < 0)
			*trk += 360;	//atan2 returns val between -pi and pi
	}
	else if(frame->me.ava.st == 3 || frame->me.ava.st == 4)
	{
		if(frame->me.ava.t)
			x = 2;		//TAS
		else
			x = 1;		//IAS

		*spd = (int)frame->me.ava.as - 1;
		if(frame->me.ava.st == 4)	//supersonic
			*spd *= 4;

		if(frame->me.ava.sh)
			*trk = (double)frame->me.ava.hdg * (360. / 1024.);
		else
			x += 2;
	}
	else
		return -2;	//possibility of vert rate still being available
				//most likely corrupted frame
	INVALID_VEL:

	if(frame->me.avg.vr)
	{
		*vr = ((int)frame->me.avg.vr - 1) * 64;
		if(frame->me.avg.svr)
			*vr = *vr * -1;
	}
	else
		x += 5;

	//TODO: Report alt difference?

	return x;
}

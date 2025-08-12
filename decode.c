#include "decode.h"
#include <math.h>

#define CRC_GEN 0x1FFF409	//generator for CRC parity check

int parityCheck(union AdsbFrame *frame)
{
	uint8_t data[14];
	int i;
	for(i = 0;i < 14;i++)	//lowest 24 bits of frame is parity code
		data[i] = frame->frame[i];
	return 0;
}

int getIdent(const union AdsbFrame *frame, char call[8], char type[8])
{
	if(frame->id.tc < 1 || frame->id.tc > 4)
		return -1;

	call[0] = (char)frame->id.c1;
	call[1] = (char)frame->id.c2;
	call[2] = (char)frame->id.c3;
	call[3] = (char)frame->id.c4;
	call[4] = (char)frame->id.c5;
	call[5] = (char)frame->id.c6;
	call[6] = (char)frame->id.c7;
	call[7] = (char)frame->id.c8;

	for(i = 0;i < 8;i++)
	{
		//callsign letters are ASCII chars with 7th bit cut off
		//spaces and numbers are ASCII equivalent
		//all other chars are unused, so as long as parity passes, is valid
		if(call[i] < 27)
			call[i] += 0x40;
	}

	switch(frame->id.tc)
	{
	case 2:
		switch(frame->id.cat)
		{
		case 0:
		case 2:
			type = "UNOWEN";
			break;
		case 1:
			type = "SEV";
			break;
		case 3:
			type = "SSV";
			break;
		default:
			type = "GRDOBS";
		}
		break;
	case 3:
		switch(frame->id.cat)
		{
		case 1:
			type = "GLIDER";
			break;
		case 2:
			type = "LTA";		//Lighter Than Air (hot air balloon)
			break;
		case 3:
			type = "SKYDIV";
			break;
		case 4:
			type = "ULTLIT";	//ultralight, hang or para glider
			break;
		case 6:
			type = "UAV";
			break;
		case 7:
			type = "SPACE";
			break;
		default:
			type = "UNOWEN";
		}
		break;
	case 4:
		switch(frame->id.cat)
		{
		case 1:
			type = "LIGHT";		//sub 7000kg
			break;
		case 2:
			type = "MED1";		//7000-34000kg
			break;
		case 3:
			type = "MED2";		//34000-136000kg
			break;
		case 4:
			type = "HVA";		//High Vortex Aircraft (disturbs nearby)
			break;
		case 5:
			type = "HEAVY";		//greater 136000kg
			break;
		case 6:
			type = "HIPERF";	//>400kts >5g pulls
			break;
		case 7:
			type = "ROTOR";		//helicopters and stuff
			break;
		default:
			type = "UNOWEN";
		}
		break;
	default:
		type = "UNOWEN";
	}

	return 0;
}

int getAirPos(const union AdsbFrame *frame, int *alt, double *lat, *lng)
{
	if(frame->id.tc < 9 || frame->id.tc > 22 || frame->id.tc == 19)
		return -1;
	return 0;
}

int getSurfPos(const union AdsbFrame *frame, int *trk, int *spd, double *lat, *lng)
{
	if(frame->id.tc < 5 || frame->id.tc > 8)
		return -1;
	return 0;
}

int getAirVel(const union AdsbFrame *frame, double *trk, int *spd)
{
	if(frame->id.tc != 19)
		return -1;
	return 0;
}

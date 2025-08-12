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

	switch(frame->id.tc * 010 + frame->id.cat)
	{				//tc * 8 + cat creates unique value for each craft category
	case 021:
		type = "SEV";		//Surface Emergency Vehicle
		break;
	case 023:
		type = "SSV";		//Surface Service Vehicle
		break;
	case 024:
	case 025:
	case 026:
	case 027:
		type = "GRDOBS";	//ground obstruction
		break;
	case 031:
		type = "GLIDER";
		break;
	case 032:
		type = "LTA";		//Lighter Than Air (hot air balloon)
		break;
	case 033:
		type = "SKYDIV";
		break;
	case 034:
		type = "ULTLIT";	//ultralight, hang or para glider
		break;
	case 036:
		type = "UAV";
		break;
	case 037:
		type = "SPACE";
		break;
	case 041:
		type = "LIGHT";		//sub 7000kg
		break;
	case 042:
		type = "MED1";		//7000-34000kg
		break;
	case 043:
		type = "MED2";		//34000-136000kg
		break;
	case 044:
		type = "HVA";		//High Vortex Aircraft (disturbs nearby)
		break;
	case 045:
		type = "HEAVY";		//greater 136000kg
		break;
	case 046:
		type = "HIPERF";	//>400kts >5g pulls
		break;
	case 047:
		type = "ROTOR";		//helicopters and stuff
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

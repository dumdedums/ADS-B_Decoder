#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

void logPlane(struct Plane buf[], int bufsize, int icao, char call[9],
	char type[8], double lat, double lng, double trk, double spd,
	int alt, int vert, enum PlaneFlags fl)
{
	register int i, oldest = 0;
	time_t now;
	double diff, grtDiff = 0.;

	time(&now);
	for(i = 0;i < bufsize;i++)
	{
		if(buf[i].pflags & ICAOFL)
		{
			if(buf[i].icao == icao)
			{
				oldest = i;
				break;
			}
			diff = difftime(now, buf[i].lstUpd);
			if(diff > grtDiff)
			{
				grtDiff = diff;
				oldest = i;
			}
		}
		else
		{
			oldest = i;
			break;
		}
	}

	buf[i].pflags |= fl;
	buf[i].icao = icao;

	if(fl & IDENTVALID)
	{
		strcpy(buf[i].call, call);
		strcpy(buf[i].type, type);
	}
	if(fl & POSVALID)
	{
		buf[i].lat = lat;
		buf[i].lng = lng;
	}
	if(fl & TRKVALID)
	{
		buf[i].trk = trk;
	}
	if(fl & SPDVALID)
	{
		buf[i].spd = spd;
	}
	if(fl & ALTVALID)
	{
		buf[i].alt = alt;
	}
	if(fl & VERTVALID)
	{
		buf[i].vert = vert;
	}

	return;
}

/*
	formatDisplay
	helper function for updating display and logging to file
*/
static char *formatDisplay(struct Plane buf[], int bufsize)
{
	int i;
	char *disp, temp[80], icao[7], call[9], type[7], lat[9], lng[9],
		trk[7], spd[7], alt[7], vert[7];
	disp = (char*)malloc(sizeof(char) * (79*50+1));
	disp[0] = 0;

	for(i = 0;i < 50 && i < bufsize;i++)
	{
		if(buf[i].pflags & ICAOFL == 0)
			break;
		if(buf[i].pflags & IDENTVALID)
		{
			snprintf(icao, 7, "%.6X", buf[i].icao);
			strcpy(call, buf[i].call);
			strcpy(type, buf[i].type);
		}
		else
		{
			strcpy(call, "UNOWEN");
			strcpy(type, "UNOWEN");
		}
		if(buf[i].pflags & POSVALID)
		{
			snprintf(lat, 9, "%.8f", buf[i].lat);
			snprintf(lng, 9, "%.8f", buf[i].lng);
		}
		else
		{
			strcpy(lat, "UNOWEN");
			strcpy(lng, "UNOWEN");
		}
		if(buf[i].pflags & TRKVALID)
		{
			snprintf(trk, 7, "%6f", buf[i].trk);
		}
		else
			strcpy(trk, "UNOWEN");
		if(buf[i].pflags & SPDVALID)
		{
			snprintf(spd, 7, "%6f", buf[i].spd);
		}
		else
			strcpy(spd, "UNOWEN");
		if(buf[i].pflags & ALTVALID)
		{
			snprintf(alt, 7, "%6d", buf[i].alt);
		}
		else
			strcpy(alt, "UNOWEN");
		if(buf[i].pflags & VERTVALID)
		{
			snprintf(vert, 7, "%6f", buf[i].vert);
		}
		else
			strcpy(vert, "UNOWEN");

		//79 chars total per line
		sprintf(temp, "%6s %8s %6s %8s %8s %6s %6s %6s %6s\n",
			icao, call, type, lat, lng, trk, spd, alt, vert);
		memcpy((void*)disp + 79*i, temp, 79);
	}
	disp[79*i] = 0;		//null terminator for string
	return disp;
}

void updateDisplay(struct Plane buf[], int bufsize)
{
	char *disp;
	disp = formatDisplay(buf, bufsize);
	printf(disp);
	free(disp);
	return;
}

void logToFile(struct Plane buf[], int bufsize)
{
	return;
}

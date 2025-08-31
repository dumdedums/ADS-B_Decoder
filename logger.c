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

	buf[oldest].pflags |= fl;
	buf[oldest].icao = icao;

	if(fl & IDENTVALID)
	{
		strcpy(buf[oldest].call, call);
		strcpy(buf[oldest].type, type);
	}
	if(fl & POSVALID)
	{
		buf[oldest].lat = lat;
		buf[oldest].lng = lng;
	}
	if(fl & TRKVALID)
	{
		buf[oldest].trk = trk;
	}
	if(fl & SPDVALID)
	{
		buf[oldest].spd = spd;
	}
	if(fl & ALTVALID)
	{
		buf[oldest].alt = alt;
	}
	if(fl & VERTVALID)
	{
		buf[oldest].vert = vert;
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
	char *disp, temp[70], icao[7], call[9], type[7], lat[9], lng[9],
		trk[7], spd[7], alt[7], vert[7];
	disp = (char*)malloc(sizeof(char) * (79*(bufsize+1)+2));

	sprintf(temp, "%6s %8s %6s %8s %8s %6s %6s %6s %6s\n",
		"ICAO", "CALLSIGN", "TYPE", "LATITUDE", "LNGITUDE", "TRACK",
		"SPEED", "ALT", "CLIMB");
	memcpy((void*)disp, temp, 69);

	for(i = 0;i < bufsize;i++)
	{
		if(buf[i].pflags & ICAOFL)
		{
			snprintf(icao, 7, "%.6X", buf[i].icao);
		}
		else
			break;
		if(buf[i].pflags & IDENTVALID)
		{
			//sprintf to remove trailing spaces in call
			sscanf(buf[i].call, "%s", call);
			strcpy(type, buf[i].type);
		}
		else
		{
			strcpy(call, "UNOWEN");
			strcpy(type, "UNOWEN");
		}
		if(buf[i].pflags & POSVALID)
		{
			snprintf(lat, 9, "% .7f", buf[i].lat);
			snprintf(lng, 9, "% .7f", buf[i].lng);
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
			snprintf(vert, 7, "%6d", buf[i].vert);
		}
		else
			strcpy(vert, "UNOWEN");

		//79 chars total per line
		sprintf(temp, "%6s %8s %6s %8s %8s %6s %6s %6s %6s\n",
			icao, call, type, lat, lng, trk, spd, alt, vert);
		memcpy((void*)disp + 69*(i+1), temp, 69);
	}
	disp[69*(i+1)] = '\n';	//extra newline for easier reading
	disp[69*(i+1)+1] = 0;	//null terminator
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

void logToFile(struct Plane buf[], int bufsize, FILE *save)
{
	int i;
	char call[9];	//temp buffer
	if(save == NULL)
		return;
	//log in the file plane data
	for(i = 0;i < bufsize;i++)
	{
		if(buf[i].pflags & ICAOFL)
			fprintf(save, "%.6X,", buf[i].icao);
		else
			break;
		if(buf[i].pflags & IDENTVALID)
		{
			sscanf(buf[i].call, "%s", call);
			fprintf(save, "%s,%s,", call, buf[i].type);
		}
		else
			fprintf(save, ",,");
		if(buf[i].pflags & POSVALID)
		{
			fprintf(save, "%f,%f,", buf[i].lat, buf[i].lng);
		}
		else
			fprintf(save, ",,");
		if(buf[i].pflags & TRKVALID)
		{
			fprintf(save, "%f,", buf[i].trk);
		}
		else
			fputc(',', save);
		if(buf[i].pflags & SPDVALID)
		{
			fprintf(save, "%f,", buf[i].spd);
		}
		else
			fputc(',', save);
		if(buf[i].pflags & ALTVALID)
		{
			fprintf(save, "%d,", buf[i].alt);
		}
		else
			fputc(',', save);
		if(buf[i].pflags & VERTVALID)
		{
			fprintf(save, "%d\n", buf[i].vert);
		}
		else
			fputc('\n', save);
	}
	return;
}

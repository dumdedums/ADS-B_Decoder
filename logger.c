#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MAPPING
#include "gmt.h"
#include "gmt_resources.h"
#endif

#include "logger.h"

#define LINEWIDTH 78

static void *API = NULL;

void logPlane(struct Plane buf[], int bufsize, int icao, char call[9],
	char type[8], double lat, double lng, double trk, double spd,
	int alt, int vert, enum PlaneFlags fl)
{
	int i, oldest = 0;
	time_t now;
	time_t diff, grtDiff = 0;

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
			diff = now - buf[i].lstUpd;
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

	if(buf[oldest].icao == icao)
		buf[oldest].pflags |= fl;
	else
	{
		buf[oldest].pflags = fl;
		buf[oldest].icao = icao;
	}

	if(changeTimeOnPosition == 0)
		buf[oldest].lstUpd = now;

	if(fl & IDENTVALID)
	{
		strcpy(buf[oldest].call, call);
		strcpy(buf[oldest].type, type);
	}
	if(fl & POSVALID)
	{
		buf[oldest].lat = lat;
		buf[oldest].lng = lng;
		buf[oldest].lstUpd = now;
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
	struct tm *ltime;
	char *disp, temp[LINEWIDTH], icao[7], call[9], type[7], lat[9], lng[9],
		trk[7], spd[7], alt[7], vert[7], timestr[9];
	disp = (char*)malloc(sizeof(char) * (LINEWIDTH*(bufsize+1)+2));

	sprintf(temp, "%6s %8s %6s %8s %8s %6s %6s %6s %6s %8s\n",
		"ICAO", "CALLSIGN", "TYPE", "LATITUDE", "LNGITUDE", "TRACK",
		"SPEED", "ALT", "CLIMB", "TIME");
	memcpy((void*)disp, temp, LINEWIDTH);

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

		ltime = localtime(&buf[i].lstUpd);
		sprintf(timestr, "%.2d:%.2d:%.2d",
			ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

		//79 chars total per line
		sprintf(temp, "%6s %8s %6s %8s %8s %6s %6s %6s %6s %8s\n",
			icao, call, type, lat, lng, trk, spd, alt, vert,
			timestr);
		memcpy((void*)disp + LINEWIDTH*(i+1), temp, LINEWIDTH);
	}
	disp[LINEWIDTH*(i+1)] = '\n';	//extra newline for easier reading
	disp[LINEWIDTH*(i+1)+1] = 0;	//null terminator
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
			fprintf(save, "%d,", buf[i].vert);
		}
		else
			fputc(',', save);
		fprintf(save, "%zd\n", buf[i].lstUpd);
	}
	return;
}

int readLog(FILE *log, struct Plane **planes)
{
	struct Plane *buf;
	int i, j;
	if(log == NULL)
		return 0;
	buf = malloc(sizeof(struct Plane) * 100);
	i = 0;
	while(1)
	{
		for(j = 0;j < 100;j++)
		{
			if(fscanf(log, " %x,", &buf[i*100+j].icao) == EOF)
				goto EXIT_READLOG;
			buf[i*100+j].pflags |= ICAOFL;

			if(fscanf(log, "%8[A-Z0-9],%6[A-Z],",
				buf[i*100+j].call, buf[i*100+j].type))
				buf[i*100+j].pflags |= IDENTVALID;
			else
			{	//get rid of spare commas
				getc(log);
				getc(log);
			}
			if(fscanf(log, "%f,%f,",
				&buf[i*100+j].lat, &buf[i*100+j].lng))
				buf[i*100+j].pflags |= POSVALID;
			else
			{
				getc(log);
				getc(log);
			}
			if(fscanf(log, "%f,", &buf[i*100+j].trk))
				buf[i*100+j].pflags |= TRKVALID;
			else
				getc(log);
			if(fscanf(log, "%f,", &buf[i*100+j].spd))
				buf[i*100+j].pflags |= SPDVALID;
			else
				getc(log);
			if(fscanf(log, "%d,", &buf[i*100+j].alt))
				buf[i*100+j].pflags |= ALTVALID;
			else
				getc(log);
			if(fscanf(log, "%d,", &buf[i*100+j].vert))
				buf[i*100+j].pflags |= VERTVALID;
			else
				getc(log);
			fscanf(log, "%zd", &buf[i*100+j].lstUpd);
		}
		i++;
		buf = realloc(buf, sizeof(struct Plane) * (i+1) * 100);
	}
	EXIT_READLOG:
	*planes = buf;
	//size of filled portion of buffer is:
	//index of last part of buffer worked on (incomplete)
	//if buffer is filled to exact multiple of 100,
	//there are 100 extra unused spaces in buffer (neg side effect)
	//there will always be extra unused spaces
	return i*100+j;
}

#ifdef MAPPING
void createImage(struct Plane buf[], int bufsize)
{
	if(API == NULL)
		API = GMT_Create_Session("PlanePlotter", 2, 0, NULL);
	return;
}

void DestroyGMTSession()
{
	if(API != NULL)
		GMT_Destroy_Session(API);
	return;
}
#endif

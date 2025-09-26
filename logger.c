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

#ifdef MAPPING
static void *API = NULL;
#endif

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
static char *formatDisplay(const struct Plane buf[], int bufsize)
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

		//WARNING: MIGHT NEED TO USE LOCK AROUND localtime
		//IF createImage IS RUNNING IN SEP THREAD
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

void updateDisplay(const struct Plane buf[], int bufsize)
{
	char *disp;
	disp = formatDisplay(buf, bufsize);
	printf(disp);
	free(disp);
	return;
}

void logToFile(const struct Plane buf[], int bufsize, FILE *save)
{
	int i;
	char call[9];	//temp buffer
	static time_t lastLog = 0;
	if(save == NULL)
		return;
	//log in the file plane data
	for(i = 0;i < bufsize;i++)
	{
		//don't log if plane hasn't been updated
		//since the last time it's been logged
		if(buf[i].lstUpd < lastLog)
			continue;

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
	time(&lastLog);
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

#define GMTSETTINGS "MAP_GRID_CROSS_SIZE_PRIMARY 8p \
MAP_TICK_LENGTH_PRIMARY 8p FONT_LABEL 12p MAP_FRAME_TYPE inside"
#define SYMBOLSETTINGS "-C -G+z -Sd8p -Z"
#define LINESETTINGS "-W"
#define TEXTSETTINGS ""

void createImage(const struct Plane buf[], int bufsize)
{
	//self explanatory options
	static char coastOpts[200] = "";
	char beginOpts[50];
	//plane dataset
	struct GMT_DATASET *planeData;
	//param list for plane dataset
	uint64_t params[3];
	//virtual file
	char PLANE_VFILE[GMT_VF_LEN];
	//loop iterators and row/col amounts per segment
	int i, j, icaoCnt = 0;
	int icaoList[bufsize];
	int icaoMent[bufsize];
	//used to make lines shorter, could be replaced with #define
	register struct GMT_DATASEGMENT *S;
	//gets the broken down time for each point
	struct tm *pntTime;

	for(i = 0;i < bufsize;i++)
		icaoList[i] = 0;
	//first run of create image creates API
	//and sets coastOpts
	if(API == NULL)
	{
		API = GMT_Create_Session("GMT_PlanePlot", 2, 0, NULL);
		//set map projection settings including centering on rlng rlat
		sprintf(coastOpts, "-R-60/60/-60/60+un -JL%f/%f/33/45/6i "
			"-Bpa15mf1mg15m -Bsa1df5mg30m -BWeSn -Ia/deepskyblue "
			"-Gtomato -Sdeepskyblue "
			"-Ln.9/.1+w10n+c%f/%f+f+l\"nm\"",
			rlng, rlat, rlng, rlng);
	}

	//make sure file name is unique so files aren't overwritten
	sprintf(beginOpts, "GMT_PlanePlot%zd pdf", time(NULL));
	GMT_Call_Module(API, "begin", 0, (void*)beginOpts);
	GMT_Call_Module(API, "set", 0, (void*)GMTSETTINGS);

	//make CPT
	GMT_Call_Module(API, "makecpt", 0, (void*)"-Cgeo -T0/50000");

	//draw coast, colorbar, and map scale
	GMT_Call_Module(API, "coast", 0, (void*)coastOpts);
	//must set frame to plain for colorbar to print correctly
	GMT_Call_Module(API, "set", 0, (void*)"MAP_FRAME_TYPE plain");
	GMT_Call_Module(API, "colorbar", 0,
		(void*)"-Dn.9/.25+w3i -Bxa -By+l\"ft\"");

	//Figure out amount of data segments
	//(different airplane paths to be drawn)
	for(i = 0;i < bufsize;i++)
	{
		//entries including and below here are unfilled
		if((buf[i].pflags & ICAOFL) == 0)
			break;
		//no position data
		if((buf[i].pflags & POSVALID) == 0)
			continue;
		//count amount of unique ICAOs
		for(j = 0;j < bufsize;j++)
		{
			if(icaoList[j] == 0)
			{
				icaoList[j] = buf[i].icao;
				icaoCnt++;
				//times icao is mentioned
				//becomes row amount
				icaoMent[j] = 1;
				break;
			}
			else if(icaoList[j] == buf[i].icao)
			{
				icaoMent[j]++;
				break;
			}
		}
	}

	params[0] = 1;
	params[1] = icaoCnt;
	params[2] = 0;		//num rows to be set individually
	//unsure if text is included as a column, would be 4 with text
	params[3] = 3;

	planeData = GMT_Create_Data(API, GMT_IS_DATASET, GMT_IS_PLP,
		GMT_WITH_STRINGS, params, NULL, NULL, 0, 0, NULL);

	//allocate rows for each segment
	//(each segment is one plane)
	for(i = 0;i < icaoCnt;i++)
	{
		GMT_Alloc_Segment(API, GMT_WITH_STRINGS, icaoMent[i],
			3, NULL, planeData->table[0]->segment[i]);
		//reuse icaoMent later so reset
		icaoMent[i] = 0;
	}

	//insert data into each segment
	for(i = 0;i < bufsize;i++)
	{
		if((buf[i].pflags & ICAOFL) == 0)
			break;
		if((buf[i].pflags & POSVALID) == 0)
			continue;
		for(j = 0;j < icaoCnt;j++)
		{
			if(icaoList[j] == buf[i].icao)
			{
				S = planeData->table[0]->segment[j];
				//set 3 data fields
				S->data[0][icaoMent[j]] = buf[i].lng;
				S->data[1][icaoMent[j]] = buf[i].lat;
				if(buf[i].pflags & ALTVALID)
					S->data[2][icaoMent[j]] = buf[i].alt;
				else
					S->data[2][icaoMent[j]] = 50000;
				//set text field
				//TODO: set up lock around localtime
				//if createImage is in sep thread
				pntTime = localtime(&buf[i].lstUpd);
				sprintf(*S->text, "%.6X %.2d:%.2d:%.2d",
					buf[i].icao, pntTime->tm_hour,
					pntTime->tm_min, pntTime->tm_sec);
				icaoMent[j]++;
				break;
			}
		}
	}

	//open vfile which is passed to the plotting modules
	GMT_Open_VirtualFile(API, GMT_IS_DATASET, GMT_IS_PLP,
		GMT_IN | GMT_IS_REFERENCE, planeData, PLANE_VFILE);

	//plot lines, then symbols, then text

	//end module creates map
	GMT_Call_Module(API, "end", 0, NULL);
	if(GMT_Destroy_Data(API, &planeData))
		printf("error destroying planeData\n");
	GMT_Close_VirtualFile(API, PLANE_VFILE);
	return;
}

void endGMTSession()
{
	//TODO: free dataset
	//destroying session might free it, but we should also free
	//related plane list we will create.
	if(API != NULL)
		GMT_Destroy_Session(API);
	return;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

int     main(argc, argv)
int     argc;
char    *argv[];
{
	struct   pxmon *pxmon;
	struct   tm *tm, *tp;
	int ii, jj;

	if (argc < 2)
	{
		printf("Usage : pxmon arg\n");
		printf("arg 1:usrmng\n");
		printf("arg 2:prcmng\n");
		printf("arg 3:scamng\n");
		return(0);
	}

	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		printf("pxmon NULL\n");
		return(0);
	}

	tm = (struct tm *)localtime(&pxmon->ctim);

	printf("cdat[%04d/%02d/%02d] ctim[%02d:%02d]\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min);

	if (argv[1][0] == '1')			/* usrmng		*/
	{
		jj = 0;
		for (ii = 0; ii < MAX_DEVN; ii++)
		{
			if (pxmon->usrmng[ii].used == SET_OFF)
				continue;

			tp = (struct tm *)localtime(&pxmon->usrmng[ii].ctim);
			printf("pxid[%07d] [%04d/%02d/%02d %02d:%02d:%02d][%07d][%07d]\n",
				pxmon->usrmng[ii].pxid,
				tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
				tp->tm_hour, tp->tm_min, tp->tm_sec, 
				pxmon->usrmng[ii].rcvc, pxmon->usrmng[ii].sndc);
			jj++;
		}
		printf("-----------------------------------------------------\n");
		printf("total %d users\n", jj);
	}
	else if (argv[1][0] == '2')		/* prcmng		*/
	{
		printf("nprc = %d\n", pxmon->nprc);
		
		for (ii = 0; ii < pxmon->nprc; ii++)
		{
			printf("pid[%d] path[%s], resident[%d], rout[%s]. area[%c]\n",
				pxmon->prcmng[ii].pid,
				pxmon->prcmng[ii].path,
				pxmon->prcmng[ii].resident,
				pxmon->prcmng[ii].rout,
				pxmon->prcmng[ii].area[0]);
		}
	}
	else if (argv[1][0] == '3')		/* scamng		*/
	{
		for (ii = 0; ii < MAX_PNOX_SCA; ii++)
		{
			if (pxmon->scamng[ii].used == SET_OFF)
				continue;

			printf("%02d spid[%07d] sock[%07d] utrd[%07d]\n", ii, 
				pxmon->scamng[ii].spid, pxmon->scamng[ii].sock, 
				pxmon->scamng[ii].utrd);	
		}
	}

	_close_pxmon(-1);
	
	return(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

int     main(argc, argv)
int     argc;
char    *argv[];
{
	struct   pxmon *pxmon;
	int ii, delay; 
	int rb, ra, sb, sa, rtps, stps;

	if (argc < 2)
	{
		printf("Usage : pxtps [sec]\n");
		return(0);
	}

	delay = atoi(argv[1]);

	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		printf("pxmon NULL\n");
		return(0);
	}

	while(1)
	{
		rb = ra = sb = sa = 0;

		for (ii = 0; ii < MAX_DEVN; ii++)
		{
			if (pxmon->usrmng[ii].used == SET_OFF)
				continue;

			rb += pxmon->usrmng[ii].rcvc; 
			sb += pxmon->usrmng[ii].sndc; 
		}

		sleep(delay);

		for (ii = 0; ii < MAX_DEVN; ii++)
		{
			if (pxmon->usrmng[ii].used == SET_OFF)
				continue;

			ra += pxmon->usrmng[ii].rcvc; 
			sa += pxmon->usrmng[ii].sndc; 
		}

		rtps = ra - rb;
		if (rtps < 0)
			rtps = 0;

		stps = sa - sb;
		if (stps < 0)
			stps = 0;

		printf("rcv-tps:%d  snd-tps:%d\n", rtps, stps);
	}

	_close_pxmon(-1);
	return(0);
}

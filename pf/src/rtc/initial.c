#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "pnox.h"
#include "pxmon.h"

#define ycopy(x,y)	memcpy(x,y,strlen(y))

/****************************************************************************
 * NAME : initial
 ***************************************************************************/
int	initial(pxmon)
struct pxmon* pxmon;
{
	int ii, jj, rtno, find_f;

	memset(&pxmon->rtcmng[0], 0x00, sizeof(struct rtcmng) * MAX_PNOX_RTC);

	rtno = 1;
	for (ii = 0; ii < pxmon->nprc && ii < MAX_PNOX_PRC; ii++)
	{
		if (pxmon->prcmng[ii].area[0] != 'U')
			continue;
		if (pxmon->prcmng[ii].rout[0] == ' ' || 
				pxmon->prcmng[ii].rout[0] == '\0')
			continue;

		find_f = 0;
		for (jj = 0; jj < MAX_PNOX_RTC; jj++)
		{
			if (pxmon->rtcmng[jj].used == SET_OFF)
				break;

			if (memcmp(pxmon->rtcmng[jj].ipad, pxmon->prcmng[ii].rout,
				sizeof(pxmon->rtcmng[jj].ipad)) == 0)
			{
				find_f = 1;
				break;
			}
		}

		if (find_f == 0 && jj < MAX_PNOX_RTC)
		{
			pxmon->rtcmng[jj].used = SET_ON;	
			pxmon->rtcmng[jj].rtno = rtno;	
			rtno++;
			memcpy(pxmon->rtcmng[jj].ipad, pxmon->prcmng[ii].rout,
				sizeof(pxmon->rtcmng[jj].ipad));

pxsyslog("pnoxrtc", "rtcmng init index[%d] used[%d] rtno[%d] ipad[%.15s]", 
jj, pxmon->rtcmng[jj].used, pxmon->rtcmng[jj].rtno, pxmon->rtcmng[jj].ipad);
		}
	}

	return(0);
}


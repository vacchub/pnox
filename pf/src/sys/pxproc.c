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

static struct proctbl {
	int stat;
	char proc[8];
	char path[256];
	char desc[256];
} proctbl[] = {
	{ 1, "pnoxsgw", BIN_PATH, "pnox session gateway"	},
	{ 1, "pnoxtpc", BIN_PATH, "pnox fork service"		},
	{ 1, "pnoxrtc", BIN_PATH, "pnox routing client"		},
	{ 0, "", "", "" }
};

int	exec_pxmon(struct pxmon *);
int	do_exec_proc(struct prcmng *);

/****************************************************************************
 * NAME : pxmon_init
 ***************************************************************************/
int	pxmon_init(pxmon)
struct pxmon *pxmon;
{
	FILE *fd;
	struct tm *tm;
	time_t clock;
	char fnam[256], buf[4*1024];
	char arg1[256], arg2[256], arg3[256], arg4[256], arg5[256];
	char arg6[256], arg7[256], arg8[256], arg9[256];
	int ii;

	clock = time(0);
	tm = localtime(&clock);

	pxsyslog("pnoxd", "pxmon_init start");

	pxmon->ctim = clock;
	pxmon->nprc = 0;

	memset(pxmon->home, 0x00, sizeof(pxmon->home));
	sprintf(pxmon->home, "%s", getenv("PNOX_HOME"));
	pxsyslog("pnoxd", "pxmon home[%s]", pxmon->home);

	memset(&pxmon->prcmng[0], 0x00, sizeof(struct prcmng) * MAX_PNOX_PRC);

	/* PF area	*/
	for (ii = 0; proctbl[ii].stat > 0; ii++)
	{
		ycopy(pxmon->prcmng[pxmon->nprc].proc, proctbl[ii].proc);
		sprintf(pxmon->prcmng[pxmon->nprc].path, "%s/%s/%s", 
			getenv("PNOX_HOME"),
			proctbl[ii].path,
			proctbl[ii].proc);
		pxmon->prcmng[pxmon->nprc].narg = 0;
		pxmon->prcmng[pxmon->nprc].resident = 1;
		pxmon->prcmng[pxmon->nprc].area[0] = 'P';

		pxsyslog("pnoxd", "pxmon [%d] proc[%.8s] path[%s] resident[%d]", 
			pxmon->nprc,
			pxmon->prcmng[pxmon->nprc].proc,
			pxmon->prcmng[pxmon->nprc].path,
			pxmon->prcmng[pxmon->nprc].resident);
		pxmon->nprc++;
	}

	/* USER area	*/
	sprintf(fnam, "%s/%s/%s", getenv("PNOX_HOME"), CFG_PATH, "pnoxsvc.cfg");
	fd = fopen(fnam, "r");
	if (fd != NULL)
	{
		while ((fgets(buf, sizeof(buf), fd)) == buf)
		{
			if (buf[0] == '#' || buf[0] == 0)
				continue;

			sscanf(buf, "%s %s %s %s %s %s %s %s %s", arg1, arg2, arg3, arg4, 
				arg5, arg6, arg7, arg8, arg9);

			ycopy(pxmon->prcmng[pxmon->nprc].proc, arg1);
			ycopy(pxmon->prcmng[pxmon->nprc].path, arg2); 
			pxmon->prcmng[pxmon->nprc].narg = atoi(arg3);
			ycopy(pxmon->prcmng[pxmon->nprc].arg1, arg4); 
			ycopy(pxmon->prcmng[pxmon->nprc].arg2, arg5); 
			ycopy(pxmon->prcmng[pxmon->nprc].arg3, arg6); 
			switch(arg7[0])
			{
			case '1' : pxmon->prcmng[pxmon->nprc].resident = 1; break;
			default  :
			case '0' : pxmon->prcmng[pxmon->nprc].resident = 0; break;
			}
			
			if (memcmp(arg8, "NONE", 4) != 0)
				ycopy(pxmon->prcmng[pxmon->nprc].rout, arg8); 
			pxmon->prcmng[pxmon->nprc].area[0] = 'U';

			pxsyslog("pnoxd", "pxmon [%d] proc[%.8s] path[%s] [%d][%s][%s][%s] resident[%d] rout[%s]", 
				pxmon->nprc,
				pxmon->prcmng[pxmon->nprc].proc,
				pxmon->prcmng[pxmon->nprc].path,
				pxmon->prcmng[pxmon->nprc].narg,
				pxmon->prcmng[pxmon->nprc].arg1,
				pxmon->prcmng[pxmon->nprc].arg2,
				pxmon->prcmng[pxmon->nprc].arg3,
				pxmon->prcmng[pxmon->nprc].resident,
				pxmon->prcmng[pxmon->nprc].rout);
			pxmon->nprc++;

			if (pxmon->nprc >= MAX_PNOX_PRC)
				break;
		}

		fclose(fd);
	}

	exec_pxmon(pxmon);

pxsyslog("pnoxd", "pxmon init [%04d/%02d/%02d %02d:%02d:%02d] nprc[%d]", 
tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, 
tm->tm_hour, tm->tm_min, tm->tm_sec, pxmon->nprc);
	return(0);
}

/****************************************************************************
 * NAME : exec_pxmon
 ***************************************************************************/
int	exec_pxmon(pxmon)
struct pxmon *pxmon;
{
	struct prcmng prcmng;
	int ii, cpid;

	for (ii = 0; ii < pxmon->nprc; ii++)
	{
		if (pxmon->prcmng[ii].resident != 1)
			continue;

		memcpy(&prcmng, &pxmon->prcmng[ii], sizeof(struct prcmng));

		cpid = do_exec_proc(&prcmng);
		if (cpid > 0)
			pxmon->prcmng[ii].pid = cpid;
	}

	return(0);
}

/****************************************************************************
 * NAME : do_exec_proc
 ***************************************************************************/
int	do_exec_proc(prcmng)
struct prcmng *prcmng;
{
	int		cpid, retc;
	
	cpid = fork();
	if (cpid < 0)		/* error	*/
	{
		pxsyslog("pnoxd", "do_exec_proc error");
		exit(1);
	}
	else if (cpid == 0)	/* child	*/
	{
		if (prcmng->narg == 0)
		{
			retc = execl(prcmng->path, prcmng->path, (char *)0);
		}
		else if (prcmng->narg == 1)
		{
			retc = execl(prcmng->path, prcmng->path, prcmng->arg1, (char *)0);
		}
		else if (prcmng->narg == 2)
		{
			retc = execl(prcmng->path, prcmng->path, prcmng->arg1, 
				prcmng->arg2, (char *)0);
		}
		else if (prcmng->narg == 3)
		{
			retc = execl(prcmng->path, prcmng->path, prcmng->arg1, 
				prcmng->arg2, prcmng->arg3, (char *)0);
		}
		else
		{
			retc = -1;
		}

		if (retc < 0)
			pxsyslog("pnoxd", "CHECK please [%s]", prcmng->path);
		exit(1);
	}

	/* parent	*/
	pxsyslog("pnoxd", "exec[%s] pid[%d]", prcmng->path, cpid);
	return(cpid);
}


/****************************************************************************
 * NAME : check_proc
 ***************************************************************************/
int	check_proc(pxmon)
struct pxmon *pxmon;
{
	struct prcmng prcmng;
	int retc, status;
	int ii, cpid;

	for (ii = 0; ii < pxmon->nprc; ii++)
	{
		if (pxmon->prcmng[ii].resident != 1)
			continue;

		retc = waitpid(pxmon->prcmng[ii].pid, &status, WNOHANG);
		if (retc < 0)
		{
			memcpy(&prcmng, &pxmon->prcmng[ii], sizeof(struct prcmng));

			cpid = do_exec_proc(&prcmng);
			if (cpid > 0)
				pxmon->prcmng[ii].pid = cpid;
        }
	}

	sleep(1);
	return(0);
}


/******************************************************************************/
/*  Components  : exesca.c                                                    */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "pnox.h"
#include "pxmon.h"

extern struct pxmon *pxmon;

int chck_empty_thread();
int _exec_sca(int);

/***************************************************************************** 
 * NAME	: chck_empty_thread()						    
 * RETURN : -2:full, -1:exe proc, 0 >= : empty		
 *****************************************************************************/
int chck_empty_thread()
{
	int ii, ntrd, retc=-2;

	ntrd = pxmon->ntrd;

	for (ii = 0; ii < MAX_PNOX_SCA; ii++)
	{
		if (pxmon->scamng[ii].used == SET_OFF)
			retc = -1;

		if (pxmon->scamng[ii].used == SET_ON)
		{
			if (pxmon->scamng[ii].utrd < ntrd)
				return(ii);
		}
	}

	return(retc);
}

/***************************************************************************** 
 * NAME	: exec_sca()							     
 *****************************************************************************/
int exec_sca(sock, socknet)
int	sock;
struct	sockaddr_in *socknet;
{
	int indx, ii, devi;
	int pxid=-1;

	indx = chck_empty_thread();
	if (indx == -2)
	{
		pxsyslog("pnoxsgw", "exec_sca full user");
		return(0);
	}
	else if (indx == -1)
	{
		indx = _exec_sca(sock);
		if (indx < 0)
			return(0);
		usleep(50000);
	}

	devi = pxmon->devi;
	if (devi < MAX_DEVN && pxmon->devn[devi] == SET_OFF)
	{
		pxid = devi + 1;
		pxmon->devi++;
		if (pxmon->devi >= MAX_DEVN)
			pxmon->devi = 0;
	}	
	else
	{
		for (ii = 0; ii < MAX_DEVN; ii++)
		{
			devi = (devi + ii) % MAX_DEVN;
	
			if (pxmon->devn[devi] == SET_OFF)
			{
				pxid = devi + 1;
				break;
			}
		}
	}

	if (pxid <= 0)
	{
		pxsyslog("pnoxsgw", "exec_sca pxid error");
		return(-1);
	}

	pxsndfd(pxmon->scamng[indx].sock, (int *)&pxid, sizeof(int), sock);

	return(0);
}

/***************************************************************************** 
 * NAME	: _exec_sca()	    						     
 * DESC	: execute a program    						    
 *****************************************************************************/
int _exec_sca(sock)
int	sock;
{
	char	cmd[256];
	pid_t	xpid;
	int	ii, retc;
	int	pair[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0)
		return(-1);

	switch ((xpid = fork()))
	{
	case -1:	/* error	*/ 
		return(-1);
	case  0:	/* child	*/
		close(pair[1]);
		dup2(pair[0], 0);
		close(pair[0]);

		sprintf(cmd, "%s/%s/%s", getenv("PNOX_HOME"), 
			BIN_PATH, "pnoxsca");
		retc = execl(cmd, cmd, (char *)0);
		if (retc < 0)
		{
			usleep(1000);
			for (ii = 0; ii < MAX_PNOX_SCA; ii++)
			{
				if (pxmon->scamng[ii].spid != getpid())
					continue;

				pxmon->scamng[ii].used = SET_OFF;
				pxmon->scamng[ii].spid = 0;
				break;
			}
			pxsyslog("pnoxsgw", "CHECK please pnoxsca");
			exit(1);
		}
		exit(0);
	default:	/* parent	*/
		close(pair[0]);

		for (ii = 0; ii < MAX_PNOX_SCA; ii++)
		{
			if (pxmon->scamng[ii].used == SET_ON)
				continue;

			pxmon->scamng[ii].used = SET_ON;
			pxmon->scamng[ii].spid = xpid;
			pxmon->scamng[ii].sock = pair[1];
			break;
		}
		break;
	}

	return(ii);
}


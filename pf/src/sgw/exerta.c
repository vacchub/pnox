/******************************************************************************/
/*  Components  : exerta.c                                                    */
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

int _exec_rta(int);

/***************************************************************************** 
 * NAME	: exec_rta()							     
 *****************************************************************************/
int exec_rta(sock, socknet)
int	sock;
struct	sockaddr_in *socknet;
{
	int indx;
	int ntmp=0;

	indx = _exec_rta(sock);
	if (indx < 0)
		return(0);
	usleep(50000);

pxsyslog("pnoxsgw", "exec_rta pxsndfd ing indx[%d]", indx);
	pxsndfd(pxmon->rtamng[indx].sock, (int *)&ntmp, sizeof(int), sock);
pxsyslog("pnoxsgw", "exec_rta pxsndfd ok");

	return(0);
}

/***************************************************************************** 
 * NAME	: _exec_rta()	    						     
 * DESC	: execute a program    						    
 *****************************************************************************/
int _exec_rta(sock)
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
			BIN_PATH, "pnoxrta");
		retc = execl(cmd, cmd, (char *)0);
		if (retc < 0)
		{
			usleep(1000);
			for (ii = 0; ii < MAX_PNOX_RTA; ii++)
			{
				if (pxmon->rtamng[ii].rpid != getpid())
					continue;

				pxmon->rtamng[ii].used = SET_OFF;
				pxmon->rtamng[ii].rpid = 0;
				break;
			}
			pxsyslog("pnoxsgw", "CHECK please pnoxrta");
			exit(1);
		}
		exit(0);
	default:	/* parent	*/
		close(pair[0]);

		for (ii = 0; ii < MAX_PNOX_RTA; ii++)
		{
			if (pxmon->rtamng[ii].used == SET_ON)
				continue;

			pxmon->rtamng[ii].used = SET_ON;
			pxmon->rtamng[ii].rpid = xpid;
			pxmon->rtamng[ii].sock = pair[1];
			break;
		}

		if (ii >= MAX_PNOX_RTA)
			ii = -1; 
		break;
	}

	return(ii);
}

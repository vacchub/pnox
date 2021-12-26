/******************************************************************************/
/*  Components  : pnoxrta.c                                                   */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include "pnox.h"
#include "pxmon.h"

struct pxmon *pxmon;
struct rtamng *rtamng;

int rta_start(int);
int service_rtc(T_HAND *, int);
int service_tr(T_HAND *, int);
int service_poll(int);

/***************************************************************************** 
 * NAME	: main()					
 * DESC	: PNOX routing server
 *****************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	struct  timeval   timeval;
	fd_set	fdlist;
	int retc, ii, cnt, find_f, nfound, myindex=-1;

	signal(SIGTERM, exit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	pxsyslog("pnoxrta", "pnoxrta start");
	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		pxsyslog("pnoxrta", "pnoxrta pxmon open error");
		return(0);
	}

	find_f = 0;
	for (cnt = 0; cnt < 3; cnt++)
	{
		for (ii = 0; ii < MAX_PNOX_RTA; ii++)
		{
			if (pxmon->rtamng[ii].used == SET_OFF)
				continue;
			if (pxmon->rtamng[ii].rpid != getpid())
				continue;
			
			rtamng = (struct rtamng *)&pxmon->rtamng[ii];			
			myindex = ii;
			find_f = 1;
			break;
		}

		if (find_f == 1)
			break;

		usleep(1000);
	}

	if (find_f == 0)
	{
		pxsyslog("pnoxrta", "pnoxrta creat error");
		return(0);
	}

	for (;;)
	{
		if (getppid() <= 1)
			break;

		FD_ZERO(&fdlist);
		FD_SET(0, &fdlist);

		timeval.tv_sec = 5;
		timeval.tv_usec = 0;
		nfound = select(0 + 1, &fdlist, NULL, NULL, &timeval);
		if (nfound <= 0)
			continue;

		if (!FD_ISSET(0, &fdlist))
			continue;
pxsyslog("pnoxrta", "pnoxrta receive event");

		retc = rta_start(myindex);
		if (retc < 0)
		{
			pxsyslog("pnoxrta", "pnoxrta rta_start error");
			break;
		}

		break;
	}

	if (rtamng->sock > 0)
		close(rtamng->sock);	

	rtamng->used = SET_OFF;
	_close_pxmon(-1);
	pxsyslog("pnoxrta", "pnoxrta end");
	return(0);
}

int rta_start(myindex)
int myindex;
{
	T_HAND	tHandle;
	struct  timeval	tval;
	fd_set	clist;
	int 	retc, sock=0, poll_f;
	int 	ntmp, maxfds, nfound;
	char	qpath[256], cmd[256], *homedir;

pxsyslog("pnoxrta", "pnoxrta rta_start start");
	homedir = getenv("PNOX_HOME");
	if (homedir == NULL)
	{
		pxsyslog("pnoxrta", "pnoxrta rta_start homedir error");
		return(-1);
	}

	retc = pxrcvfd(0, (int *)&ntmp, sizeof(int), &sock);
pxsyslog("pnoxrta", "pnoxrta sock[%d]", sock);

	sprintf(cmd, "%s/%s/%.*s", homedir, QUE_PATH, (int)strlen(qpath), qpath);
	unlink(cmd);
	if (errno == ENOENT)
		errno = 0;

	memset(&tHandle, 0x00, sizeof(T_HAND));
	sprintf(qpath, "rta%04d", myindex + 1);
	retc = pxopen(&tHandle, qpath);
	if (retc < 0)
	{
		pxsyslog("pnoxrta", "pnoxrta pxopen error %d", retc);
		return(-1);
	}

	poll_f = 0;
	for (;;)
	{
		FD_ZERO(&clist);
		FD_SET(tHandle.devf, &clist);
		maxfds = tHandle.devf;
		FD_SET(sock, &clist);
		if (sock > maxfds)
			maxfds = sock;

		tval.tv_sec = 10;
		tval.tv_usec = 0;
		nfound = select(maxfds+1, &clist, NULL, NULL, &tval);
		if (nfound < 0)
		{
			pxsyslog("pnoxrta", "nfound retc:%d, errno:%d", nfound, errno);
			break;
		}
		if (nfound == 0)
		{
			retc = service_poll(sock);

			if (poll_f)
			{
				pxsyslog("pnoxrta", "poll recv error");
				break;
			}
			poll_f = 1;
			continue;
		}
		poll_f = 0;

		/* from client	*/
		if (FD_ISSET(sock, &clist))
		{
			retc = service_rtc(&tHandle, sock);
			if (retc < 0)
				break;	
		}

		if (FD_ISSET(tHandle.devf, &clist))
		{
			retc = service_tr(&tHandle, sock);
			if (retc < 0)
				break;	
		}
	}

pxsyslog("pnoxrta", "pnoxrta rta_start end");
	pxclose(&tHandle);
	return(0);
}


/******************************************************************************/
/*  Components  : pnoxtpc.c                                                   */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "pnox.h"
#include "pxmon.h"

#define	MAX_RUN		512		/* maximum running child	*/

int	ntpc;
int	parent;
pid_t   tpcpid[MAX_RUN];	/* maximum pnoxtpc process      */

int     check_proc();
int     do_exec_proc();

/***************************************************************************** 
 * NAME	: main()			
 * DESC	: PNOX fork service	
 *****************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	T_HAND	tHandle;
	M_HAND	mHandle;
	struct	timeval	  timeval;
	int	ii, xpid;
	int status;
	int	retc, maxfds, nfound;
	char	*xenvs, qpath[256];
	char	sndb[65*1024], rcvb[65*1024];
	fd_set	clist;

	signal(SIGTERM, exit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	pxsyslog("pnoxtpc", "pnoxtpc start");

	xenvs = getenv("PNOX_NTPC");
	if (xenvs == NULL)
	{
		pxsyslog("pnoxtpc", "pnoxtpc PNOX_NTPC error");
		exit(0);
	}

	ntpc = atoi(xenvs);
	if (ntpc > MAX_RUN)
		ntpc = MAX_RUN;

	xenvs = getenv("PNOX_HOME");
	if (xenvs == NULL)
	{
		pxsyslog("pnoxtpc", "pnoxtpc PNOX_HOME error");
		exit(0);
	}
	sprintf(qpath, "%s/%s/pnoxtpc", xenvs, QUE_PATH);
	unlink(qpath);
	if (errno == ENOENT)
		errno = 0;

	memset(&tHandle, 0x00, sizeof(T_HAND));
	retc = pxopen(&tHandle, "pnoxtpc");
	if (retc < 0)
	{
		pxsyslog("pnoxtpc", "pnoxtpc pxopen error %d", retc);
		exit(0);
	}

	parent = 1;
	for (ii = 0; ii < ntpc; ii++)
	{
		xpid = fork();
		switch (xpid)
		{
		case -1: continue;
		case  0: 
			parent = 0;
			break;
		default: 
			tpcpid[ii] = xpid;
			continue;
		}
		break;
	}

	for (;;)
	{
		if (getppid() <= 1)
			break;

		if (parent)
		{
			check_proc();
			sleep(3);
			continue;
		}

		FD_ZERO(&clist);
		FD_SET(tHandle.devf, &clist);
		maxfds = tHandle.devf;

		timeval.tv_sec = 5;
		timeval.tv_usec = 0;

		nfound = select(maxfds+1, &clist, NULL, NULL, &timeval);
		if (nfound < 0)
		{
			pxsyslog("pnoxtpc", "select error nfound:%d, errno:%d", nfound, errno);
			break;
		}
		if (nfound == 0)
			continue;
		if (!(FD_ISSET(tHandle.devf, &clist)))
		{
			pxsyslog("pnoxtpc", "ISSET error");
			continue;
		}

		memset(&mHandle, 0x00, sizeof(M_HAND));
		memset(sndb, 0x00, sizeof(sndb));
		memset(rcvb, 0x00, sizeof(rcvb));
		mHandle.sndb = sndb;
		mHandle.rcvb = rcvb;

		retc = pxrecv(&tHandle, &mHandle, 0);
		if (retc < 0)
			continue;
		
		if (mHandle.rcvl <= 0)
			continue;

		memcpy(mHandle.sndb, mHandle.rcvb, mHandle.rcvl);
		mHandle.sndl = mHandle.rcvl;

		retc = pxexec(&tHandle, &mHandle);
		if (retc < 0)
		{
			pxsyslog("pnoxtpc", "pxexec error %d", retc);
			continue;
		}	

		/* wait for completion of child process */
		xpid = wait(&status); 
		if (xpid < 0)
		{
			if (errno != ECHILD)
				pxsyslog("pnoxtpc", "wait fail... %d", errno);
			else
				errno = 0;
			continue;
		}

		if (WEXITSTATUS(status) < 0)
			pxsyslog("pnoxtpc", "%s return %d", mHandle.trnm, WEXITSTATUS(status));
	}

	if (parent)
	{
		pxclose(&tHandle);
		pxsyslog("pnoxtpc", "pnoxtpc end");
	}
	else
	{
		pxsyslog("pnoxtpc", "pnoxtpc child end");
	}
	return(0);
}

/****************************************************************************
 * NAME : do_exec_proc
 ***************************************************************************/
int	do_exec_proc()
{
	int xpid;

	xpid = fork();
	if (xpid < 0)           /* error        */
	{
		pxsyslog("pnoxtpc", "do_exec_proc error");
		return(-1);
	}
	else if (xpid == 0)     /* child        */
	{
		parent = 0;
		return(0);
	}

	/* parent       */
	parent = 1;
	pxsyslog("pnoxtpc", "do_exec_proc pid:%d", xpid);
	return(xpid);
}

/****************************************************************************
 * NAME : check_proc
 ***************************************************************************/
int	check_proc()
{
	int ii, xpid, retc;

	for (ii = 0; ii < ntpc; ii++)
	{
		retc = kill(tpcpid[ii], 0);
		if (retc == -1)
		{
			if (errno == ESRCH)
				errno = 0;
			xpid = do_exec_proc();
			if (!parent)
				break;

			if (xpid > 0)
				tpcpid[ii] = xpid;
		}
	}

	return(0);
}



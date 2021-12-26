/****************************************************************************** 
 * Components  : tpsmain.c
 ******************************************************************************/
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
#include <libgen.h>
#include "pnox.h"
#include "pxmon.h"

#define	MAX_RUN		512		/* maximum running child	*/

static	int	ntps;
static	int	parent;
static	pid_t   tpspid[MAX_RUN];	/* maximum pnoxtps process      */

int		tpsmain(T_HAND *, M_HAND *);
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
	int	retc, maxfds, nfound;
	char	*xenvs, qpath[256], myproc[16], tmpb[16];
	char	sndb[65*1024], rcvb[65*1024];
	fd_set	clist;

	signal(SIGTERM, exit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	if (argc < 2)
	{
		pxsyslog("tpsmain", "tpsmain argument invalid");
		exit(0);
	}

	if (memcmp(argv[1], "-m", 2) != 0 || strlen(argv[1]) <= 2)
	{
		pxsyslog("tpsmain", "tpsmain argument invalid [-m]");
		exit(0);
	}

	pxsyslog("tpsmain", "tpsmain start");

	ntps = atoi(&argv[1][2]);
	if (ntps <= 0)
		ntps = 1;
	if (ntps > MAX_RUN)
		ntps = MAX_RUN;

	memset(myproc, 0x00, sizeof(myproc));
	strcpy(myproc, basename(argv[0]));

	xenvs = getenv("PNOX_HOME");
	if (xenvs == NULL)
	{
		pxsyslog("tpsmain", "tpsmain PNOX_HOME error");
		exit(0);
	}
	sprintf(qpath, "%s/%s/%s", xenvs, QUE_PATH, myproc);
	unlink(qpath);
	if (errno == ENOENT)
		errno = 0;

	memset(&tHandle, 0x00, sizeof(T_HAND));
	retc = pxopen(&tHandle, myproc);
	if (retc < 0)
	{
		pxsyslog("tpsmain", "tpsmain pxopen error %d", retc);
		exit(0);
	}

	parent = 1;
	for (ii = 0; ii < ntps; ii++)
	{
		xpid = fork();
		switch (xpid)
		{
		case -1: continue;
		case  0: 
			parent = 0;
			break;
		default: 
			tpspid[ii] = xpid;
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
		tHandle.nusr = 0;

		timeval.tv_sec = 10;
		timeval.tv_usec = 0;

		nfound = select(maxfds+1, &clist, NULL, NULL, &timeval);
		if (nfound < 0)
		{
			pxsyslog("tpsmain", "select error nfound:%d, errno:%d", nfound, errno);
			break;
		}
		if (nfound == 0)
			continue;

		if (!FD_ISSET(tHandle.devf, &clist))
			continue;

		memset(&mHandle, 0x00, sizeof(M_HAND));
		memset(sndb, 0x00, sizeof(sndb));
		memset(rcvb, 0x00, sizeof(rcvb));
		mHandle.sndb = sndb;
		mHandle.rcvb = rcvb;

		retc = pxrecv(&tHandle, &mHandle, 1);
		if (retc < 0)
			continue;

		if (mHandle.rcvl <= 0)
			continue;

#if 0
		memcpy(mHandle.sndb, mHandle.rcvb, mHandle.rcvl);
		mHandle.sndl = mHandle.rcvl;
#endif

		retc = tpsmain(&tHandle, &mHandle);
		if (retc < 0)
    	{
        	pxsyslog("tpsmain", "tpsmain continue %d", retc);
			continue;
		}

		memset(tmpb, 0x00, sizeof(tmpb));
		sprintf(tmpb, "u%07d", tHandle.pxid);
		memcpy(tHandle.call, tmpb, sizeof(tHandle.call));
#if 0
pxsyslog("tpsmain", "tpsmain call name [%d][%.8s]", tHandle.nusr, tHandle.call);
#endif

		retc = pxsend(&tHandle, &mHandle);
    	if (retc < 0)
    	{
        	pxsyslog("tpsmain", "tpsmain error %d %d", retc, errno);
			continue;
    	}
	}

	if (parent)
	{
		pxclose(&tHandle);
		pxsyslog("tpsmain", "tpsmain end");
	}
	else
	{
		pxsyslog("tpsmain", "tpsmain child end");
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
		pxsyslog("tpsmain", "do_exec_proc error");
		return(-1);
	}
	else if (xpid == 0)     /* child        */
	{
		parent = 0;
		return(0);
	}

	/* parent       */
	parent = 1;
	pxsyslog("tpsmain", "do_exec_proc pid:%d", xpid);
	return(xpid);
}

/****************************************************************************
 * NAME : check_proc
 ***************************************************************************/
int	check_proc()
{
	int ii, xpid, retc;

	for (ii = 0; ii < ntps; ii++)
	{
		retc = kill(tpspid[ii], 0);
		if (retc == -1)
		{
			if (errno == ESRCH)
				errno = 0;
			xpid = do_exec_proc();
			if (!parent)
				break;

			if (xpid > 0)
				tpspid[ii] = xpid;
		}
	}

	return(0);
}

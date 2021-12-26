/******************************************************************************/
/*  Components  : pnoxrtc.c                                                   */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "pnox.h"
#include "pxmon.h"

T_HAND  tHandle[MAX_PNOX_RTC];
int	rtcpid[MAX_PNOX_RTC];
int	runsock[MAX_PNOX_RTC];
struct  pxmon *pxmon;
int nrtc;
int parent;

int	initial(struct pxmon *);
int	check_proc();	
int sock_init(struct pxmon *, int);

int service_rta(T_HAND *, int);
int service_tr(T_HAND *, int);

/*****************************************************************************
 * NAME	: main()							    
 * DESC	: PNOX routing client	 				   
 *****************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	struct  timeval   timeval;
	int maxfds, nfound;
	int retc, ii, xpid, myindex;
	char *xenvs, qpath[256];
	fd_set  clist;

	signal(SIGTERM, exit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

	pxsyslog("pnoxrtc", "pnoxrtc start");

	xenvs = getenv("PNOX_HOME");
    if (xenvs == NULL)
    {
        pxsyslog("pnoxrtc", "pnoxrtc PNOX_HOME error");
        exit(0);
    }

	for (ii = 0; ii < MAX_PNOX_RTC; ii++)
	{
		rtcpid[ii] = 0;
		sprintf(qpath, "%s/%s/rtc%04d", xenvs, QUE_PATH, ii + 1);
			unlink(qpath);	
	}
	if (errno == ENOENT)
        errno = 0;

	pxmon = _open_pxmon(-1);
    if (pxmon == NULL)
    {
        pxsyslog("pnoxrtc", "pxmon open error");
		sleep(2);
        return(0);
    }

	initial(pxmon);

	parent = 1;
	for (ii = 0; ii < MAX_PNOX_RTC; ii++)
	{
		if (pxmon->rtcmng[ii].used == SET_OFF)
		{
			nrtc = ii;
			break;
		}

		xpid = fork();
		switch (xpid)
        {
        case -1: continue;
        case  0: 
			parent = 0;
			myindex = ii;

			memset(&tHandle[myindex], 0x00, sizeof(T_HAND));
			sprintf(qpath, "rtc%04d", myindex + 1);
			retc = pxopen(&tHandle[myindex], qpath);
			if (retc < 0)
			{
				pxsyslog("pnoxrtc", "pxopen error retc:%d, idx:%d", 
					retc, myindex);
				sleep(1);
				exit(0);
			}

			retc = sock_init(pxmon, myindex);
			if (retc < 0)
			{
				pxsyslog("pnoxrtc", "sock_init error retc:%d, idx:%d", 
					retc, myindex);
				sleep(1);
				exit(0);
			}
			runsock[myindex] = retc;
			break;
        default:
            rtcpid[ii] = xpid;
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
            retc = check_proc();
			if (retc < 0)
			{
				sleep(1);
				break;
			}

            sleep(3);
            continue;
        }

        FD_ZERO(&clist);
        FD_SET(tHandle[myindex].devf, &clist);
        maxfds = tHandle[myindex].devf;
		
        FD_SET(runsock[myindex], &clist);
		if (runsock[myindex] > maxfds)
			maxfds = runsock[myindex];

        timeval.tv_sec = 10;
        timeval.tv_usec = 0;
        nfound = select(maxfds+1, &clist, NULL, NULL, &timeval);
		if (nfound < 0)
		{
			pxsyslog("pnoxrtc", "nfound error errno:%d", errno);
			break;
		}
		if (nfound == 0)
			continue;

        if (FD_ISSET(runsock[myindex], &clist))
		{
			retc = service_rta(&tHandle[myindex], runsock[myindex]);
			if (retc < 0)
				break;
		}

		if (FD_ISSET(tHandle[myindex].devf, &clist))
		{
			retc = service_tr(&tHandle[myindex], runsock[myindex]);
			if (retc < 0)
				break;
		}
	}

	if (!parent)
	{
		retc = pxclose(&tHandle[myindex]);	
		pxsyslog("pnoxrtc", "pnoxrtc child end");
		return(0);
	}

	_close_pxmon(-1);
	
	pxsyslog("pnoxrtc", "pnoxrtc end");
	return(0);
}

/****************************************************************************
 * NAME : check_proc
 ***************************************************************************/
int check_proc()
{
    int ii, retc;

    for (ii = 0; ii < nrtc; ii++)
    {
        retc = kill(rtcpid[ii], 0);
        if (retc == -1)
        {
			pxsyslog("pnoxrtc", "check_proc end");
			return(-1);
        }
    }
    return(0);
}

/****************************************************************************
 * NAME : sock_init()
 ***************************************************************************/
int sock_init(pxmon, index)
struct pxmon *pxmon;
int index;
{
	struct	rtcmng *rtcmng;
	struct  sockaddr_in sock_in;
    int		sock;
    int		ii, retc, option;

	rtcmng = &pxmon->rtcmng[index];
	if (rtcmng == NULL)
        return(-1);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return(-2);

    option = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));
    option = 65 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &option, sizeof(int));
    option = 65 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &option, sizeof(int));
#if 1
    option = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(int));
    fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

    sock_in.sin_addr.s_addr = inet_addr(rtcmng->ipad);
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(PORT_RTA);

#if 0
    if (connect(sock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
    {
		pxsyslog("pnoxrtc", "connect fail [%.15s] errno:%d", 
			rtcmng->ipad, errno);
        return(-3);
    }
#else
	for (ii = 0; ii < 30000; ii++)
	{
		retc = connect(sock, (struct sockaddr *)&sock_in, sizeof(sock_in));
		if (retc >= 0)
			break;
		usleep(100);
	}
	if (retc < 0)
	{
		pxsyslog("pnoxrtc", "connect fail [%.15s] errno:%d", 
			rtcmng->ipad, errno);
        return(-3);
	}
#endif

	pxsyslog("pnoxrtc", "connect OK [%s]", rtcmng->ipad);
    return(sock);
}

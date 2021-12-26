/******************************************************************************/
/*  Components  : pnoxsca.c                                                   */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "pnox.h"
#include "pxmon.h"

char	myproc_usr[MAX_DEVN];

struct	pxmon *pxmon;
struct scamng *scamng;
pthread_mutex_t muxtrd;

void sig_exit(int);
void *exec_thread(void *);
int addutrd();
int delutrd();

int service_user(T_HAND *, int);
int service_svr(T_HAND *, int);
int service_poll(int);

/***************************************************************************** 
 * NAME	: main()					
 * DESC	: PNOX session client agent
 *****************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	pthread_t strd;
	pthread_attr_t tattr;
	size_t stacks;
	struct  timeval   timeval;
	fd_set	fdlist;
	int retc, ii, cnt, find_f, nfound, uflg;
	char	cmd[256], *homedir;

	signal(SIGTERM, sig_exit);
	signal(SIGKILL, sig_exit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	for (ii = 0; ii < MAX_DEVN; ii++)
	{
		myproc_usr[ii] = 0;
	}

	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		pxsyslog("pnoxsca", "pnoxsca pxmon open error");
		return(0);
	}

	find_f = 0;
	for (cnt = 0; cnt < 3; cnt++)
	{
		for (ii = 0; ii < MAX_PNOX_SCA; ii++)
		{
			if (pxmon->scamng[ii].used == SET_OFF)
				continue;
			if (pxmon->scamng[ii].spid != getpid())
				continue;
			
			scamng = (struct scamng *)&pxmon->scamng[ii];			
			find_f = 1;
			break;
		}

		if (find_f == 1)
			break;

		usleep(1000);
	}

	if (find_f == 0)
	{
		pxsyslog("pnoxsca", "pnoxsca creat error");
		return(0);
	}

	if (pthread_mutex_init(&muxtrd, NULL) != 0)
	{
		pxsyslog("pnoxsca", "pnoxsca mutex init error");
		return(0);
	}

	uflg = 0;
	scamng->utrd = 0;

	pthread_attr_init(&tattr);
	pthread_attr_getstacksize(&tattr, &stacks);
	if (stacks < (512 * 1024))
		stacks = 512 * 1024;
	pthread_attr_setstacksize(&tattr, stacks);

	for (;;)
	{
		if (getppid() <= 1)
			break;

		if (scamng->utrd > 0)
			uflg = 1;

		if (uflg && scamng->utrd == 0)
			break;

		if (scamng->utrd == pxmon->ntrd)
		{
			usleep(100000);
			continue;
		}

		if (scamng->utrd > pxmon->ntrd)
		{
			pxsyslog("pnoxsca", "pnoxsca utrd OVER");
			usleep(100000);
			continue;
		}

		FD_ZERO(&fdlist);
		FD_SET(0, &fdlist);

		timeval.tv_sec = 5;
		timeval.tv_usec = 0;
		nfound = select(0 + 1, &fdlist, NULL, NULL, &timeval);
		if (nfound <= 0)
			continue;

		if (!FD_ISSET(0, &fdlist))
			continue;

		addutrd();
		retc = pthread_create(&strd, &tattr, exec_thread, (void *)NULL);
		if (retc < 0)
		{
			pxsyslog("pnoxsca", "pnoxsca pthread_create error");
			continue;
		}

		/* parent thread wait thread-create	*/
		usleep(10000);
	}

	if (scamng->sock > 0)
		close(scamng->sock);	

	if (pthread_mutex_destroy(&muxtrd) != 0)
		pxsyslog("pnoxsca", "pnoxsca mutex destroy error");

	scamng->used = SET_OFF;
	_close_pxmon(-1);

	homedir = getenv("PNOX_HOME");
	if (homedir == NULL)
		exit(0);

	for (ii = 0; ii < MAX_DEVN; ii++)
	{
		if (myproc_usr[ii] == 1)
		{
			sprintf(cmd, "%s/%s/u%07d", homedir, QUE_PATH, ii+1);
			unlink(cmd);
			pxsyslog("pnoxsca", "pnoxsca exit pxid[%d]", ii+1);
		}
	}
	return(0);
}

void *exec_thread(free)
void *free;
{
	T_HAND	tHandle;
	struct  timeval	tval;
	pthread_t ptrd;
	fd_set	clist;
	int 	retc, sock=0, poll_f;
	int 	pxid, maxfds, nfound;
	char	cmd[256], *homedir;

	ptrd = pthread_self();
	retc = pthread_detach(ptrd);
	homedir = getenv("PNOX_HOME");
	if (retc != 0 || homedir == NULL)
	{
		pxsyslog("pnoxsca", "pnoxsca pthread_detach error");
		delutrd();
		pthread_exit((void *)0);
		return((void *)0);
	}

	retc = pxrcvfd(0, (int *)&pxid, sizeof(int), &sock);
	if (pxid <= 0 || pxid > MAX_DEVN)
	{
		pxsyslog("pnoxsca", "pnoxsca pxid[%d] error", pxid);
		delutrd();
		pthread_exit((void *)0);
		return((void *)0);
	}

	pxmon->devn[pxid - 1] = SET_ON;

	memset(&tHandle, 0x00, sizeof(T_HAND));
	tHandle.pxid = pxid;

	sprintf(cmd, "%s/%s/u%07d", homedir, QUE_PATH, pxid);
	unlink(cmd);
	if (errno == ENOENT)
		errno = 0;

	retc = pxopen(&tHandle, NULL);
	if (retc < 0)
	{
		pxsyslog("pnoxsca", "pnoxsca pxopen error %d", retc);
		pxmon->devn[pxid - 1] = SET_OFF;
		delutrd();
		pthread_exit((void *)0);
		return((void *)0);
	}

	pxmon->usrmng[tHandle.pxid - 1].used = SET_ON;
	pxmon->usrmng[tHandle.pxid - 1].pxid = tHandle.pxid;
	pxmon->usrmng[tHandle.pxid - 1].rcvc = 0;
	pxmon->usrmng[tHandle.pxid - 1].sndc = 0;
	pxmon->usrmng[tHandle.pxid - 1].ctim = time(0);
	myproc_usr[tHandle.pxid - 1] = 1;

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
			pxsyslog("pnoxsca", "nfound retc:%d, errno:%d", nfound, errno);
			break;
		}
		if (nfound == 0)
		{
			retc = service_poll(sock);

			if (poll_f)
			{
				pxsyslog("pnoxsca", "poll recv error pxid[%d]", tHandle.pxid);
				break;
			}
			poll_f = 1;
			continue;
		}
		poll_f = 0;

		/* from client	*/
		if (FD_ISSET(sock, &clist))
		{
			retc = service_user(&tHandle, sock);
			if (retc < 0)
			{
				break;	
			}
		}

		if (FD_ISSET(tHandle.devf, &clist))
		{
			retc = service_svr(&tHandle, sock);
			if (retc < 0)
				break;	
		}
	}

	pxmon->usrmng[tHandle.pxid - 1].used = SET_OFF;
	pxmon->usrmng[tHandle.pxid - 1].pxid = 0;
	pxmon->usrmng[tHandle.pxid - 1].ctim = 0;
	myproc_usr[tHandle.pxid - 1] = 0;

	pxclose(&tHandle);
	pxmon->devn[pxid - 1] = SET_OFF;
	delutrd();
	pthread_exit((void *)0);
	return((void *)0);
}

int addutrd()
{
	pthread_mutex_lock(&muxtrd);
	scamng->utrd++;	
	pthread_mutex_unlock(&muxtrd);
	return(0);
}

int delutrd()
{
	pthread_mutex_lock(&muxtrd);
	scamng->utrd--;	
	pthread_mutex_unlock(&muxtrd);
	return(0);
}

/*****************************************************************************
 * NAME : sig_exit()
 * DESC : signal control
 *****************************************************************************/
void	sig_exit(sig)
int sig;
{
	pxsyslog("pnoxsca", "pnoxsca kill [%d]", scamng->spid);
	scamng->used = SET_OFF;
	scamng->spid = 0;
	scamng->sock = 0;
	scamng->utrd = 0;

	exit(0);
}

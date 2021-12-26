/***********************************************************
 * NAME : pnoxd.c
 * DESC : pnox platform daemon
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include "pnox.h"
#include "pxmon.h"

static struct pxversion pxversion = { 0, 1 };

static struct	pxmon *pxmon;
static int	u_sock;		/* event socket		*/

void daemonize();
void sig_daemon(int);
int pnox_sock_init();
int pnox_event(char, char *);
int already_running();
int pxmon_init(struct pxmon*);
int check_proc(struct pxmon*);

/***************************************************************************** 
 * NAME : main()
 * DESC : pnox main procedure
 *****************************************************************************/
int	main(argc, argv)
int	argc;
char	*argv[];
{
	struct	timeval timeval;
	struct	eventmsg eventmsg;
	fd_set	rlist;
	char	*homedir, pnox_home[256], base[256];
	int	elen, nfound, xchk;

	homedir = getenv("PNOX_HOME");
	if (homedir == NULL)
	{
		memset(base, 0x00, sizeof(base));
		memcpy(base, argv[0], strlen(argv[0])); 
		homedir = dirname(base);
		if (homedir != NULL)
		{
			if (homedir[0] != '/')
			{
				printf("enter full path please...\n");
				exit(0);
			}
			xchk = strlen(homedir);
			if (xchk > 4 && strcmp(&homedir[xchk-3], BIN_PATH) == 0)
				homedir[xchk-4] = '\0';
			strcpy(pnox_home, homedir);
		}
	}
	else
		strcpy(pnox_home, homedir);

	pxputenv(pnox_home);
	pxsyslog("pnoxd", "%s pnox system loading...", pnox_home);
	pxsyslog("pnoxd", "pnoxd PNOX_HOME[%s]", getenv("PNOX_HOME"));
	daemonize();

	if (already_running())
	{
		pxsyslog("pnoxd", "pnox already running");
		exit(0);
	}

	if (pnox_sock_init() < 0)
	{
		pxsyslog("pnoxd", "sock error");
		exit(0);
	}

	pxmon = _open_pxmon(IPC_CREAT);
	if (pxmon == NULL)
	{
		pxsyslog("pnoxd", "pxmon error");
		exit(0);
	}

	pxmon_init(pxmon);

	for(;;)
	{
		check_proc(pxmon);

		FD_ZERO(&rlist);
		FD_SET(u_sock, &rlist);
		timeval.tv_sec = 2;
		timeval.tv_usec = 0;
		nfound = select(u_sock+1, &rlist, NULL, NULL, &timeval);
		if (nfound <= 0)
			continue;

		elen = read(u_sock, &eventmsg, sizeof(eventmsg));
		if (elen != sizeof(eventmsg))
			continue;

		if (pnox_event(eventmsg.opt, eventmsg.arg) < 0)
			break;
	}

	_close_pxmon(IPC_RMID);

	pxsyslog("pnoxd", "pnox end");
	return(0);
}

/***************************************************************************** 
 * NAME : daemonize()
 * DESC : pnox daemonize
 *****************************************************************************/
void	daemonize()
{
	int ii;
	char home_path[256];

	if (getppid() == 1) 
	{
		pxsyslog("pnoxd", "pnoxd daemonize getppid error");
		exit(0); 	/* already a daemon */
	}

	ii = fork();
	if (ii < 0) 
	{
		pxsyslog("pnoxd", "pnoxd daemonize fork error");
		exit(1); 
	}
	if (ii > 0) 
	{
		exit(0);
	}

	/* child (daemon) continues */
	setsid(); 			/* obtain a new process group */

#if 1
/*
	for (ii = getdtablesize(); ii >= 0; --ii) {
*/
	for (ii = 0; ii < getdtablesize(); ii++) {
		close(ii); 		/* close all descriptors */
	}
#endif
	ii = open("/dev/null", O_RDWR); 
	dup(ii);			/* stdout		*/
	dup(ii); 			/* stderr		*/
	umask(077); 			/* set newly created file permissions */

	sprintf(home_path, "%s/%s", getenv("PNOX_HOME"), BIN_PATH);
	chdir(home_path); 		/* change running directory */

	/* first instance continues */
	signal(SIGUSR1, sig_daemon);	/* talkzone		*/
	signal(SIGHUP, sig_daemon);	/* catch hangup signal 	*/
	signal(SIGTERM, sig_daemon); 	/* catch kill signal 	*/
	signal(SIGKILL, sig_daemon); 	/* catch kill signal 	*/
	signal(SIGCHLD, sig_daemon); 	
	signal(SIGTSTP, SIG_IGN); 	/* ignore tty signals 	*/
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
}

/***************************************************************************** 
 * NAME : sig_daemon()
 * DESC : signal control
 *****************************************************************************/
void	sig_daemon(sig)
int sig;
{
	switch(sig) 
	{
	case SIGHUP:
		pxsyslog("pnoxd", "pnox receive SIGHUP signal");
		break;
	case SIGCHLD:
		pxsyslog("pnoxd", "pnox receive SIGCHLD signal");
		break;
	case SIGTERM:
	case SIGKILL:
	default:
		_close_pxmon(IPC_RMID);
		pxsyslog("pnoxd", "pnox system terminating...");
		/* term_proc(&proc_table); */
		exit(0);
	}
}

/***************************************************************************** 
 * NAME : pnox_sock_init() 
 * DESC : initialize socket to receive event from other process 
 *****************************************************************************/
int	pnox_sock_init()
{
	struct  sockaddr_un sock_svr;
	char    u_path[128];

	sprintf(u_path, "%s/%s/PXIPC", getenv("PNOX_HOME"), IPC_PATH);
	unlink(u_path);
	u_sock = -1;

	u_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	bzero(&sock_svr, sizeof(sock_svr));
	sock_svr.sun_family = AF_UNIX;
	strcpy(sock_svr.sun_path, u_path);
pxsyslog("pnoxd", "[%s]", sock_svr.sun_path);
	if (bind(u_sock, (struct sockaddr *)&sock_svr, sizeof(sock_svr)) < 0)
	{
		close(u_sock);
		u_sock = -1;
		return(-1);
	}

	fcntl(u_sock, F_SETFL, O_NONBLOCK);
	return(0);
}

/***************************************************************************** 
 * NAME : pnox_event() 
 * DESC : event control
 *****************************************************************************/
int	pnox_event(char opt, char *arg)
{
	struct eventmsg event_t;
	char cmd[256];

	switch (opt)
	{
#if 0
	case 'q' :
		for (ii = proc_table.nrec - 1; ii >= 0; ii--)		
		{
			kill(proc_table.p[ii].pid, SIGKILL);
		}
		break;
	case 'r' :
		for (ii = 0; ii < proc_table.nrec; ii++)		
		{
			if (proc_table.p[ii].pid == atoi(arg))
			{
				kill(atoi(arg), SIGKILL);
				break;
			}
		}
		break;
#endif
	case 'k' :
		return(-1);
	case 'v' :
		memset(&event_t, 0x00, sizeof(struct eventmsg));
		event_t.opt = 'v';
		memcpy(event_t.arg, (struct pxversion *)&pxversion, 
			sizeof(struct pxversion));
		sprintf(cmd, "%s/%s/PXVER", getenv("PNOX_HOME"), IPC_PATH);
		pxsvrsnd(cmd, (char *)&event_t, sizeof(struct eventmsg));
	} 

	return(0);
}

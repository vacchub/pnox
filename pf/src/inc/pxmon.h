#ifndef _PXMON_H
#define _PXMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define	MAX_DEVN		100000
#define	MAX_PNOX_PRC	100
#define	MAX_PNOX_SCA	100
#define	MAX_SCA_TRD		256
#define	MAX_PNOX_RTC	100
#define	MAX_PNOX_RTA	1000
#define	MAX_PXSEND		512

/*************************************************************************
 * PNOX USER MANAGER
 ************************************************************************/
struct	usrmng {
	int		used;		/* 1:use, 0:not (set pnoxsca)	*/
	int		pxid;
	int		rcvc;		/* receive count				*/
	int		sndc;		/* send count					*/
	time_t	ctim;		/* user connect time 			*/	
};

/*************************************************************************
 * PNOX PROCESS MANAGER
 ************************************************************************/
struct	prcmng {
	int		pid;			/* process id			*/
	char	proc[8];		/* process name			*/
	char	path[256];		/* process path			*/
	int		narg;			/* arg count			*/
	char	arg1[64];		/* arg 1				*/
	char	arg2[64];		/* arg 2				*/
	char	arg3[64];		/* arg 3				*/
	int		resident;		/* resident:1, no:0		*/
	char	rout[15];		/* routing IP			*/
	char	area[1];		/* P:pf, U:user			*/
	char	desc[256];		/* process desc			*/
};

/*************************************************************************
 * PNOX SCA MANAGER
 ************************************************************************/
struct	scamng {
	int	used;			/* 1:use, 0:not (set pnoxsgw)	*/
	int	spid;			/* sca process id (set pnoxsgw)	*/
	int	sock;			/* sgw sock fd (set pnoxsgw)	*/
	int	utrd;			/* used thread (set pnoxsca)	*/
};

/*************************************************************************
 * PNOX RTC MANAGER
 ************************************************************************/
struct	rtcmng {
	int		used;			/* 1:use, 0:not (set pnoxrtc)	*/
	int		cpid;			/* rtc process id (set pnoxrtc)	*/
	int		rtno;			/* rtc number					*/
	char	ipad[15];		/* rtc connect IP				*/
};

/*************************************************************************
 * PNOX RTA MANAGER
 ************************************************************************/
struct	rtamng {
	int	used;			/* 1:use, 0:not (set pnoxsgw)	*/
	int	rpid;			/* rta process id (set pnoxsgw)	*/
	int	sock;			/* sgw sock fd (set pnoxsgw)	*/
};

/*************************************************************************
 * PNOX MONITORING
 ************************************************************************/
struct	pxmon {
	time_t	ctim;							/* create time		*/	
	int		logf;							/* 0:N, 1:Y			*/
	int		devi;							/* devn index		*/
	char	home[128];						/* home path		*/	
	char	devn[MAX_DEVN];					/* make pxid		*/
	struct	usrmng usrmng[MAX_DEVN];		/* user manager		*/
	int		nprc;							/* procmng no		*/
	struct	prcmng prcmng[MAX_PNOX_PRC];	/* process manager	*/
	int		ntrd;							/* thread no		*/
	struct	scamng scamng[MAX_PNOX_SCA];	/* sca manager     	*/
	struct	rtcmng rtcmng[MAX_PNOX_RTC];	/* rtc manager		*/
	struct	rtamng rtamng[MAX_PNOX_RTA];	/* rta manager		*/
};


/*************************************************************************
 * PROTOTYPE
 ************************************************************************/

struct pxmon* _open_pxmon(int);
void _close_pxmon(int);
int homedir_pxmon(char *, struct pxmon *);

#endif

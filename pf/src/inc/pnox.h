#ifndef _PNOX_H
#define _PNOX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "pxmon.h"

#define SET_ON		1
#define SET_OFF		0

#define BIN_PATH	"bin"
#define CFG_PATH	"cfg"
#define LOG_PATH	"log"
#define QUE_PATH	"que"
#define IPC_PATH	"ipc"

#define MAX_EVENT_ARG   32

struct eventmsg {
	char opt;                       /* event option         */
	char arg[MAX_EVENT_ARG];        /* event argument       */
};

/*************************************************************************
 * PNOX HEADER
 ************************************************************************/
struct	pnoxhdr {
	char	chkf[2];		/* 0x7F, 0x7F			*/
	char	type[4];		/* 0 : message type		*/
							/* 'Q':query			*/
							/* 'R':rts				*/
							/* 'P':polling			*/
							/* 1=filler				*/
							/* 2=filler				*/
							/* 3=filler				*/
	char	trnm[8];		/* service name			*/
	char	dlen[5];		/* data lenth			*/
	char	data[1];		/* data					*/
};

#define PNOXHDR_L	sizeof(struct pnoxhdr) - 1

#define	MASK_CHKF	0x7F

#define TYPE0_QUERY		'Q'
#define TYPE0_RTS		'R'
#define TYPE0_POLL		'P'

/*************************************************************************
 * TRANSACTION HANDLE
 ************************************************************************/
typedef struct {
	int		pxid;			/* pnox id								*/
	char	home[128];		/* pnox homedir							*/
	char	name[8];		/* ipc name - myname(pxopen)			*/
	char	call[8];		/* ipc name caller - call proc(pxsend)	*/
	char	base[8];		/* ipc name - base name					*/
	int		rtaf;			/* is rta ?								*/
	int		devf;			/* socket								*/
	int		nusr;			/* send user no.						*/
	int		*ulst;			/* send user list						*/
} T_HAND;


#define MAX_BUF_LEN	4*1024*1024
#define MAX_RCV_LEN	64*1024
#define MAX_SND_LEN	16*1024

struct  iochdr {			/* I/O control header	*/
	int		pxid;
	char	type[4];			/* 0 : message type				*/
								/* 'Q':query					*/
								/* 'R':rts						*/
								/* 'P':polling					*/
								/* 1=filler						*/
								/* 2=filler						*/
								/* 3=filler						*/
	char	name[8];
	char	trnm[8];
};

/*************************************************************************
 * MESSAGE HANDLE
 ************************************************************************/
typedef struct {
	struct	iochdr	iochdr;		/* pnox i/o control header		*/
	char	type[4];			/* 0 : message type				*/
								/* 'Q':query					*/
								/* 'R':rts						*/
								/* 'P':polling					*/
								/* 1=filler						*/
								/* 2=filler						*/
								/* 3=filler						*/
	char	trnm[8];			/* TR name						*/
	int		rcvl;				/* receive message length		*/
	char	*rcvb;				/* receive message buffer		*/
	int		sndl;				/* send(reply) message length	*/
	char	*sndb;				/* send(reply) message buffer	*/	
} M_HAND;

/*************************************************************************
 * PNOX VERSION
 ************************************************************************/
struct	pxversion {		/* pnox platform version		*/
	char major;
	char minor;
};


/*************************************************************************
 * PORT info
 ************************************************************************/
#define PORT_SCA	18311
#define PORT_RTA	18312


/*************************************************************************
 * PNOX errno
 ************************************************************************/
#define XREADY      -1          /* system is not ready          */
#define XOPEN       -2          /* plase open first             */
#define XBUSY       -3          /* service is busy              */
#define XREGISTER   -4          /* unregistered service         */
#define XPERM       -5          /* no permission                */
#define XNOENT      -6          /* no such service              */
#define XCONNECT    -7          /* connect error                */
#define XSESSION    -8          /* no session to receive        */
#define XCHANNEL    -9          /* no i/o channel               */
#define XINVAL      -10         /* invalid arguments            */
#define XSVCDOWN    -11         /* service is down              */
#define XSVCBUSY    -12         /* service is busy              */
#define XMANY       -13         /* to many process              */
#define XEXEC       -14         /* exec error                   */
#define XIO         -15         /* i/o error                    */
#define XNOMSG      -16         /* no message to receive        */
#define XNOBUFF     -17         /* no buffer                    */
#define XROOM       -18         /* not enough memory            */
#define XTIMEOUT    -19         /* receive timeout              */
#define XABORTED    -20         /* aborted without reply        */


/*************************************************************************
 * PROTOTYPE
 ************************************************************************/

int	pxopen(T_HAND *, char *);
int	pxcall(T_HAND *, M_HAND *, int);
int	pxsend(T_HAND *, M_HAND *);
int	pxrecv(T_HAND *, M_HAND *, int);
int	pxclose(T_HAND *);
int	pxexec(T_HAND *, M_HAND *);
int	pxrout(T_HAND *, M_HAND *);
int	pxsndfd(int, void *, size_t, int);
int	pxrcvfd(int, void *, size_t, int *);
int	pxreadenv(char *);
int	pxtoken(char *, int);
int	pxsyslog(char *, const char *, ...);
int	pxhexlog(char *, char *, char *, int);
int	pxsvrsnd(char *, char *, int);
int	pxputenv(char *);
int	pxlogout(int);
int	getpxid(T_HAND *);
int	ispxid(int);
char *pxerrname(int);

#endif

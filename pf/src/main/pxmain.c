/******************************************************************************
 * NAME : pxmain()
 * DESC : pxmain
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

#define	K	1024

int pnoxsvc(T_HAND *, M_HAND *);

/******************************************************************************
 * NAME : pxmain()
 * DESC : pxmain
 ******************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	T_HAND	tHandle;
	M_HAND	mHandle;
	struct	timeval timeval;
	fd_set  rlist;
	int		retc, nfound, nlen;
	struct	iochdr	*iochdr;
	char	bufa[65 * 1024];
	char	buft[65 * 1024];
	char	sndb[65 * 1024];
	char	rcvb[65 * 1024];

	FD_ZERO(&rlist);
    FD_SET(0, &rlist);
    timeval.tv_sec = 5;
    timeval.tv_usec = 0;
    nfound = select(0 + 1, &rlist, NULL, NULL, &timeval);
    if (nfound <= 0)
    {
		pxsyslog("pxmain", "select error %d", errno);
        return(-1);
    }	

	memset(bufa, 0x00, sizeof(bufa));
	nlen = 0;
	for (;;)
	{
		memset(buft, 0x00, sizeof(buft));
		retc = read(0, buft, sizeof(buft));
		if (retc <= 0)
			break; 
		
		memcpy(&bufa[nlen], buft, retc);
		nlen += retc;	
	}

	memset(&tHandle, 0x00, sizeof(T_HAND));
	memset(&mHandle, 0x00, sizeof(M_HAND));

	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));

	mHandle.sndb = sndb;
	mHandle.sndl = 0;
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;

	iochdr = (struct iochdr *)bufa;
	tHandle.pxid = iochdr->pxid;
	memcpy(tHandle.call, iochdr->name, sizeof(tHandle.call));
	tHandle.nusr = 0;
	
	mHandle.type[0] = iochdr->type[0];
	mHandle.type[1] = iochdr->type[1];
	mHandle.type[2] = iochdr->type[2];
	mHandle.type[3] = iochdr->type[3];
	memcpy(mHandle.trnm, iochdr->trnm, sizeof(mHandle.trnm));

	mHandle.rcvl = nlen - sizeof(struct iochdr);
	memcpy(mHandle.rcvb, &bufa[sizeof(struct iochdr)], mHandle.rcvl);
	
	tHandle.ulst = (int *)malloc(sizeof(int) * MAX_PXSEND);

	retc = pnoxsvc(&tHandle, &mHandle);	

	retc = pxsend(&tHandle, &mHandle);
	if (retc < 0)
	{
		free(tHandle.ulst);
		pxsyslog("pxmain", "pxsend error %d %d", retc, errno);
		return(-1);
	}

	free(tHandle.ulst);

	return(0);
}

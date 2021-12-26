/***********************************************************
 * NAME : mbmtalk.c
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "mbmtalk.h"

extern int l_token(char *, int);

static int _attach_mode;

struct mbmtalk *_mbmtalk = NULL;

/*****************************************************************************
 * NAME : l_mtalkopen()
 *****************************************************************************/
struct	mbmtalk *l_mtopen(mode)
int	mode;
{
	int     shmkey, shmid;
	char    *shmadd;

	if (_mbmtalk != NULL)
	{
		if (mode == _attach_mode)
			return(_mbmtalk);
		shmdt((char *)_mbmtalk);
		_mbmtalk = NULL;
	}

	if ((shmkey = l_token("mbmtalk", 'M')) == -1)
		return (NULL);
	if ((shmid = shmget(shmkey, 0, S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP)) == -1)
		return (NULL);
	if (mode == O_RDONLY)
		shmadd = shmat(shmid, (char *)0, SHM_RDONLY);
	else
		shmadd = shmat(shmid, (char *)0, 0);
	if (shmadd == (char *)-1)
		return (NULL);

	_mbmtalk = (struct mbmtalk *)shmadd;
	_attach_mode = mode;
	return (_mbmtalk);
}

/*****************************************************************************
 * NAME : l_mtread()
 *****************************************************************************/
struct	mjmtalk *l_mtread(mbmtalk, pxid)
struct	mbmtalk *mbmtalk;
int	pxid;
{
	int	ii, jj;

	for (ii = 0; ii < mbmtalk->vrec; ii++)
	{
		for (jj = 0; jj < 2; jj++)
		{
			if (mbmtalk->mjmtalk[ii].pxid[jj] == pxid)
			{
				return(&mbmtalk->mjmtalk[ii]);
			}
		}
	}
	
	return(NULL);
}

/*****************************************************************************
 * NAME : l_mtaddr()
 *****************************************************************************/
struct	mjmtalk *l_mtaddr(mbmtalk, pxid1, pxid2)
struct	mbmtalk *mbmtalk;
int	pxid1, pxid2;
{
	int	ii, jj, indx;
	struct	mjmtalk a_mjmtalk;

	memset(&a_mjmtalk, 0x00, sizeof(struct mjmtalk));
	a_mjmtalk.pxid[0] = pxid1;
	a_mjmtalk.pxid[1] = pxid2;
	for (ii = 0; ii < mbmtalk->vrec; ii++)
	{
		for (jj = 0; jj < 2; jj++)
		{
			if (mbmtalk->mjmtalk[ii].pxid[jj] == pxid1)
				return(&mbmtalk->mjmtalk[ii]);
			if (mbmtalk->mjmtalk[ii].pxid[jj] == pxid2)
				return(&mbmtalk->mjmtalk[ii]);
		}
	}

	if (mbmtalk->vrec >= MAX_MJMTALK)
		return(NULL);

	indx = mbmtalk->vrec;
	memset(&mbmtalk->mjmtalk[indx], 0x00, sizeof(struct mjmtalk));
	memcpy(&mbmtalk->mjmtalk[indx], &a_mjmtalk, sizeof(struct mjmtalk));
	
	mbmtalk->vrec++;

	return(&mbmtalk->mjmtalk[indx]);
}
	
/*****************************************************************************
 * NAME : l_mtdelr()
 *****************************************************************************/
int	l_mtdelr(mbmtalk, pxid)
struct	mbmtalk *mbmtalk;
int pxid;
{
	int	ii, jj, indx;

	indx = -1;
	for (ii = 0; ii < mbmtalk->vrec; ii++)
	{
		for (jj = 0; jj < 2; jj++)
		{
			if (mbmtalk->mjmtalk[ii].pxid[jj] == pxid)
			{
				indx = ii;
				break;
			}
		}
	}

	if (indx < 0)
		return -1;

	for (ii = indx; ii < mbmtalk->vrec - 1; ii++)
	{
		memcpy(&mbmtalk->mjmtalk[ii], &mbmtalk->mjmtalk[ii+1], sizeof(struct mjmtalk));
	}
	memset(&mbmtalk->mjmtalk[mbmtalk->vrec - 1], 0x00, sizeof(struct mjmtalk));	

	mbmtalk->vrec--;
	
	return 0;
}

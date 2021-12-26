#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "pnox.h"

int test0001(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	struct	tm *tm;
	time_t	t1;
	char	buff[1024];	

	t1 = time(NULL);
	tm = localtime(&t1);

	/*
	memcpy(mHandle->sndb, mHandle->rcvb, mHandle->rcvl);
	mHandle->sndl = mHandle->rcvl;
	*/

{FILE *fp; fp = fopen("/proj/pnox/app/log/test0001.log", "a");
fprintf(fp, "RECV %04d/%02d/%02d %02d:%02d:%02d [%05d] nick[%.30s][%s]\n", 
tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec,
mHandle->rcvl, &mHandle->rcvb[0], &mHandle->rcvb[30]);
fclose(fp);}

	memset(buff, 0x00, sizeof(buff));
	sprintf(buff, "%.30s : %s", &mHandle->rcvb[0], &mHandle->rcvb[30]);
	memcpy(mHandle->sndb, buff, strlen(buff));
	mHandle->sndl = strlen(buff);

{FILE *fp; fp = fopen("/proj/pnox/app/log/test0001.log", "a");
fprintf(fp, "SEND %04d/%02d/%02d %02d:%02d:%02d [%05d][%.*s]\n", 
tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec,
mHandle->sndl, mHandle->sndl, mHandle->sndb);
fclose(fp);}

	return(0);
}


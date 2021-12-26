#include <stdio.h>
#include "pnox.h"

int test0002(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	char buff[1024];

{FILE * fp; fp = fopen("/pnox/app/log/test0001.log", "a");
fprintf(fp, "rcvl[%d] rcvb[%s]\n", mHandle->rcvl, mHandle->rcvb);
fclose(fp);}

	memset(buff, 0x00, sizeof(buff));
	memcpy(buff, mHandle->rcvb, mHandle->rcvl);
	sprintf(&buff[strlen(buff)], "%04d", getpxid(tHandle)); 

	memcpy(mHandle->sndb, buff, strlen(buff));
	mHandle->sndl = strlen(buff);

	tHandle->nusr = 1;
	*(tHandle->ulst + 0) = getpxid(tHandle);

	return(0);
}


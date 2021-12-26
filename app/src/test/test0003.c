#include <stdio.h>
#include "pnox.h"

int test0003(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	char buff[1024];
	int a;

{FILE * fp; fp = fopen("/pnox/app/log/test0001.log", "a");
fprintf(fp, "rcvl[%d] rcvb[%s]\n", mHandle->rcvl, mHandle->rcvb);
fclose(fp);}

	memset(buff, 0x00, sizeof(buff));
	memcpy(buff, mHandle->rcvb, mHandle->rcvl);
	a = atoi(buff);

	sprintf(mHandle->sndb, "%d", a+5);
	mHandle->sndl = strlen(buff);
	return(0);
}


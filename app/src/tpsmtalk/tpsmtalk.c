#include <stdio.h>
#include "pnox.h"

int mtalk001(T_HAND *, M_HAND *);
int mtalk002(T_HAND *, M_HAND *);

int tpsmain(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	int trid, retc=0;
	char temp[8];

	memset(temp, 0x00, sizeof(temp));
	memcpy(temp, mHandle->rcvb, 4);
	trid = atoi(temp);

{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk001.log", "a");
fprintf(fp, "trid[%d] [%d][%.*s]\n", trid, mHandle->rcvl, mHandle->rcvl, mHandle->rcvb);
fclose(fp);}

	switch (trid) 
	{
	case 1001 : retc = mtalk001(tHandle, mHandle);	break;
	case 1002 : retc = mtalk002(tHandle, mHandle);	break;
	default   : break;
	}

	return(retc);
}


#include <stdio.h>
#include "pnox.h"
#include "pxmon.h"

int main(int argc, char *argv[])
{
	T_HAND tHandle;
	M_HAND mHandle;
	char    pnox_home[256], cmd[256], temp[16];;
	int		pxid[MAX_PXSEND];
	int		ii, retc, maxid;
	char    bufa[65 * 1024];
	char    buft[65 * 1024];
	char    sndb[65 * 1024];
	char    rcvb[65 * 1024];
	char buff[1024];

	if (argc != 3)
		return(0);

	memset(&tHandle, 0x00, sizeof(T_HAND));
    retc = pxopen(&tHandle, "pibd6003");
    if (retc < 0)
    {
        printf("pxopen error %d\n", retc);
        return(-1);
    }

	pxid[0] = atoi(argv[1]);
printf("pxid = [%d]\n", pxid[0]);

	memset(&mHandle, 0x00, sizeof(M_HAND));

	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));

	mHandle.type[0] = 'Q';
	mHandle.sndb = sndb;
	mHandle.sndl = 0;
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;
	tHandle.nusr = 1;
	tHandle.ulst = &pxid[0];
	sprintf(temp, "u%07d", pxid[0]);
	memcpy(tHandle.call, temp, 8);

	sprintf(buff, "%s", argv[2]);
	memcpy(mHandle.sndb, buff, strlen(buff));
	mHandle.sndl = strlen(buff);
printf("[%d][%s]\n", mHandle.sndl, mHandle.sndb);

	retc = pxsend(&tHandle, &mHandle);
printf("retc[%d]\n", retc);
	if (retc < 0)
	{
		printf("pxsend error [%d]\n", retc);
		return(-1);
	}
	pxclose(&tHandle);

	return(0);
}

#include <stdio.h>
#include "pnox.h"
#include "pxmon.h"

int main(int argc, char *argv[])
{
	T_HAND tHandle;
	M_HAND mHandle;
	char    pnox_home[256], cmd[256];
	int		pxid[MAX_PXSEND];
	int		ii, retc, maxid;
	char    bufa[65 * 1024];
	char    buft[65 * 1024];
	char    sndb[65 * 1024];
	char    rcvb[65 * 1024];
	char buff[1024];

	if (argc <= 1)
		return(0);

#if 0
	retc = homedir_pxmon(pnox_home, NULL);
	if (retc < 0)
	{
		printf("homedir error %d\n", retc);
		return(0);
	}

	sprintf(cmd, "PNOX_HOME=%s", pnox_home);
	putenv(cmd);
#else
	memset(&tHandle, 0x00, sizeof(T_HAND));
    retc = pxopen(&tHandle, "pibd6002");
    if (retc < 0)
    {
        printf("pxopen error %d\n", retc);
        return(-1);
    }
#endif

	maxid = argc - 1;
	for (ii=0; ii < MAX_PXSEND; ii++)
		pxid[ii] = 0;

	for (ii=0; ii < maxid; ii++)
		pxid[ii] = atoi(argv[ii+1]);

	memset(&mHandle, 0x00, sizeof(M_HAND));

	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));

	mHandle.sndb = sndb;
	mHandle.sndl = 0;
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;
	tHandle.nusr = maxid;
	tHandle.ulst = &pxid[0];

	sprintf(buff, "%s", "ABCDEFG");
	memcpy(mHandle.sndb, buff, strlen(buff));
	mHandle.sndl = strlen(buff);

	retc = pxsend(&tHandle, &mHandle);
	if (retc < 0)
	{
		printf("pxsend error [%d]\n", retc);
		return(-1);
	}

	return(0);
}

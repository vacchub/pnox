#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "pnox.h"
#include "../tpsmtalk/mbmtalk.h"

int pxlogoff(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	struct mbmtalk *mbmtalk = NULL;
	struct mjmtalk *mjmtalk = NULL;
	int pxid;

{FILE *fp; fp=fopen("/proj/pnox/app/log/pxlogoff.log", "a+");
fprintf(fp, "pxlogoff start pxid %d\n", pxid);
fclose(fp);}

	mbmtalk = l_mtopen(O_RDONLY);
	if (mbmtalk == NULL)
	{
		return(0);
	}

	pxid = getpxid(tHandle);
	l_mtdelr(mbmtalk, pxid);

{FILE *fp; fp=fopen("/proj/pnox/app/log/pxlogoff.log", "a+");
fprintf(fp, "pxlogoff end pxid %d\n", pxid);
fclose(fp);}
	return(0);
}


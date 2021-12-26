#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "pnox.h"

struct stmsg {
	char	type;
	int		pxid;
};

extern int l_msgget(char *, int , int);
extern int l_putmsg(int, int, char *, int , int);

int mtalk001(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	struct	stmsg stmsg;
	char	buff[1024];	
	int		qid, rc;
{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk001.log", "a");
fprintf(fp, "mtalk001 start\n");
fclose(fp);}

	qid = l_msgget("mtalkmsg", 0, 0666);
	if (qid < 0)
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "search 실패 -1");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		return(0);
	}
{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk001.log", "a");
fprintf(fp, "qid[%d]\n", qid);
fclose(fp);}

	stmsg.type = mHandle->rcvb[4];
	stmsg.pxid = getpxid(tHandle);
{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk001.log", "a");
fprintf(fp, "type[%c] pxid[%d]\n", stmsg.type, stmsg.pxid);
fclose(fp);}
	rc = l_putmsg(qid, 0, (char *)&stmsg, sizeof(struct stmsg), 0); 
	if (rc < 0)
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "search 실패 -2");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		return(0);
	}

	return(0);
}


#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "pnox.h"
#include "mbmtalk.h"

int mtalk002(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	T_HAND  _tHandle;
	M_HAND  _mHandle;
	struct	mbmtalk	*mbmtalk;
	struct	mjmtalk	*mjmtalk;
	struct	tm *tm;
	time_t	t1;
	char	buff[1024], temp[12];	
	int		retc, pxid[2];

	t1 = time(NULL);
	tm = localtime(&t1);

	/*
	memcpy(mHandle->sndb, mHandle->rcvb, mHandle->rcvl);
	mHandle->sndl = mHandle->rcvl;
	*/

{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk002.log", "a+");
fprintf(fp, "RECV %04d/%02d/%02d %02d:%02d:%02d [%05d] nick[%.30s][%s]\n", 
tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec, 
mHandle->rcvl, &mHandle->rcvb[4], &mHandle->rcvb[34]);
fclose(fp);}

	mbmtalk = l_mtopen(O_RDONLY);
	if (mbmtalk == NULL)
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "서비스 실패 -10");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		return(0);
	}

	mjmtalk = l_mtread(mbmtalk, tHandle->pxid); 
	if (mjmtalk == NULL)
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "[002]상대와의 연결이 종료되었습니다.");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		return(0);
	}

	memset(buff, 0x00, sizeof(buff));
	sprintf(buff, "%.30s : %s", &mHandle->rcvb[4], &mHandle->rcvb[34]);
#if 0
	memcpy(mHandle->sndb, buff, strlen(buff));
	mHandle->sndl = strlen(buff);
#endif

	memset(&_tHandle, 0x00, sizeof(T_HAND));
	memset(&_mHandle, 0x00, sizeof(M_HAND));

	retc = pxopen(&_tHandle, "mtalk002");
	if (retc < 0)
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "메세지 처리실패");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		return(0);
	}

	sprintf(temp, "%s", "mtalk002");
	memcpy(_tHandle.call, temp, 8);                                        

#if 0
	sprintf(temp, "u%07d", getpxid(tHandle));                                      
	memcpy(_tHandle.call, temp, 8);                                        
#endif
	_mHandle.type[0] = 'Q';
	_mHandle.sndb = buff;
	_mHandle.sndl = strlen(buff);
#if 1
	_tHandle.nusr = 2;
	*(_tHandle.ulst + 0) = mjmtalk->pxid[0];
	*(_tHandle.ulst + 1) = mjmtalk->pxid[1];
#else
	pxid[0] = mjmtalk->pxid[0];
	pxid[1] = mjmtalk->pxid[1];
	_tHandle.nusr = 2;
	_tHandle.ulst = &pxid[0];
#endif

	retc = pxsend(&_tHandle, &_mHandle);                                    
	if (retc < 0)                                                         
	{
		memset(buff, 0x00, sizeof(buff));
		sprintf(buff, "%s", "서비스 실패 -12");
		memcpy(mHandle->sndb, buff, strlen(buff));
		mHandle->sndl = strlen(buff);
		pxclose(&_tHandle);
		return(0);
	}

{FILE *fp; fp = fopen("/proj/pnox/app/log/mtalk002.log", "a+");
fprintf(fp, "SEND %04d/%02d/%02d %02d:%02d:%02d [%05d][%.*s]\n", 
tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
tm->tm_hour, tm->tm_min, tm->tm_sec,
_mHandle.sndl, _mHandle.sndl, _mHandle.sndb);
fclose(fp);}

	pxclose(&_tHandle);

	memset(buff, 0x00, sizeof(buff));
	memcpy(mHandle->sndb, buff, strlen(buff));
	mHandle->sndl = 0;
	return(0);
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "pnox.h"
#include "mbmtalk.h"

struct stmsg {
	char    type;
	int     pxid;
};

struct mbmtalk *mbmtalk = NULL;
struct mjmtalk *mjmtalk = NULL;

extern int l_token(char *, int);
extern int l_msgget(char *, int, int);
extern int l_getmsg(int, int *, char *, int , int);

int attach_mbmtalk();

int main(int argc, char *argv[])
{
	T_HAND  _tHandle;
	M_HAND  _mHandle;
	struct	stmsg stmsg;
	int		qid, pxid[2], rc;
	int		recv_l, mtype=0;
	char    buff[1024], temp[12];
	int		retc, ii, index=0;

{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 start\n");
fclose(fp);}
	rc = attach_mbmtalk();
	if (rc < 0)
	{
{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 attach_mbmtalk error %s\n", strerror(errno));
fclose(fp);}
		sleep(2);
		return(0);
	}

	mbmtalk = l_mtopen(O_RDWR);
	if (mbmtalk == NULL)
	{
{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 attach_mbmtalk error %s\n", strerror(errno));
fclose(fp);}
		sleep(2);
		return(0);
	}
	mbmtalk->vrec = 0;
	for (ii = 0; ii < mbmtalk->maxm; ii++)
	{
		mbmtalk->mjmtalk[ii].pxid[0] = 0;
		mbmtalk->mjmtalk[ii].pxid[1] = 0;
	}

	qid = l_msgget("mtalkmsg", 1024*4096, IPC_CREAT|0666);
	if (qid < 0)
	{
{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 l_msgget error %d %s\n", qid, strerror(errno));
fclose(fp);}
		sleep(2);
		return(0);
	}

	memset(&_tHandle, 0x00, sizeof(T_HAND));
	memset(&_mHandle, 0x00, sizeof(M_HAND));


	retc = pxopen(&_tHandle, "mtalk010");
	if (retc < 0)
	{
{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 pxopen error %d %s\n", retc, strerror(errno));
fclose(fp);}
		sleep(2);
		return(0);
	}

	sprintf(temp, "%s", "mtalk010");
	memcpy(_tHandle.call, temp, 8);

	index = 0;
	for (;;)
	{
		if (getppid() == 1)
			break;

		memset(&stmsg, 0x00, sizeof(struct stmsg));
		recv_l = l_getmsg(qid, &mtype, (char *)&stmsg, sizeof(struct stmsg), 2); 
		if (recv_l < 0)
		{
{FILE *fp; fp=fopen("/proj/pnox/app/log/mtalk010.log", "a+");
fprintf(fp, "mtalk010 l_getmsg break\n");
fclose(fp);}
			break;
		}
		if (recv_l == 0)
			continue;

		if (stmsg.type == 'q')
		{
			sprintf(buff, "%s", "상대방이 나갔습니다.");
			_mHandle.type[0] = 'Q';
			_mHandle.sndb = buff;
			_mHandle.sndl = strlen(buff);

			mjmtalk = l_mtaddr(mbmtalk, pxid[0], pxid[1]);
			if (mjmtalk == NULL)
				break;

			_tHandle.nusr = 2;
			*(_tHandle.ulst + 0) = mjmtalk->pxid[0];
			*(_tHandle.ulst + 1) = mjmtalk->pxid[1];
			retc = pxsend(&_tHandle, &_mHandle); 

			retc = l_mtdelr(mbmtalk, stmsg.pxid);
			continue;
		}
		else if (stmsg.type == 'j')
		{
			pxid[index] = stmsg.pxid;

			if (index == 1)
			{
				mjmtalk = l_mtaddr(mbmtalk, pxid[0], pxid[1]);
				if (mjmtalk == NULL)
				{
					break;
				}

				sprintf(buff, "%s", "상대방이 입장 하였습니다.");
				_mHandle.type[0] = 'Q';
				_mHandle.sndb = buff;
				_mHandle.sndl = strlen(buff);

				_tHandle.nusr = 2;
				*(_tHandle.ulst + 0) = mjmtalk->pxid[0];
				*(_tHandle.ulst + 1) = mjmtalk->pxid[1];
				retc = pxsend(&_tHandle, &_mHandle); 
				if (retc < 0)
				{
					sprintf(buff, "%s", "[010]상대와의 연결이 종료되었습니다.");
					_mHandle.type[0] = 'Q';
					_mHandle.sndb = buff;
					_mHandle.sndl = strlen(buff);

					_tHandle.nusr = 2;
					*(_tHandle.ulst + 0) = mjmtalk->pxid[0];
					*(_tHandle.ulst + 1) = mjmtalk->pxid[1];

					retc = pxsend(&_tHandle, &_mHandle); 

					/* 종료	*/
					retc = l_mtdelr(mbmtalk, stmsg.pxid);
				}
				index = 0;
			}
			else
			{
				index++;
			}
		}
	}

	pxclose(&_tHandle);

	return(0);
}

/*****************************************************************************
 * NAME : attach_mbmtalk()
 *****************************************************************************/
int attach_mbmtalk()
{
	int shmsz, shmky, shmid;
	char    *shmad;
	struct  shmid_ds shmid_ds;

	shmsz = sizeof(struct mbmtalk) + (sizeof(struct mjmtalk) * MAX_MJMTALK);
	shmky = l_token("mbmtalk", 'M');
	shmid = shmget(shmky, 0, 0666);
	if (shmid != -1)
	{
		shmctl(shmid, IPC_STAT, &shmid_ds);
#if 0
		sprintf(logmsg, "mbmtalk SHM size CHECK [%d][%d]", (int)shmid_ds.shm_segsz,
				shmsz);
		l_logmsg(p_name, logmsg);
#endif
		if (shmid_ds.shm_segsz != shmsz)
		{
			shmctl(shmid, IPC_RMID, &shmid_ds);
			shmid = -1;
		}
	}

	if (shmid == -1)
	{
		shmid = shmget(shmky, shmsz, 0666|IPC_CREAT);
#if 0
		load_f = 1;
#endif
	}
	shmad = (char *)shmat(shmid, (char *)0, 0);
	if (shmad == (char *)-1)
	{
#if 0
		l_logmsg(p_name, "SHM mbmtalk ATTACH FAILURE");
#endif
		return(-1);                                              
	}                                                            
	mbmtalk = (struct mbmtalk *)shmad;                             
	mbmtalk->maxm = MAX_MJMTALK;                                     
#if 0
	if (mbmtalk->ctim == 0)                                       
		load_f = 1;                                              
#endif
	return(0);                                                   
}            


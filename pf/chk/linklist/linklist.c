/******************************************************************************/
/*  Components  : linklist.c                                                  */
/*  Description : 통신DATA를 LIKE LISTBUFF로 버퍼링함                         */
/*  Rev. History: Ver   Date    Description                                   */
/*                ----  ------- ----------------------------------------------*/
/*                1.0   2001-12 Initial version                               */
/******************************************************************************/
/* Procedure Name       Functional Description                                */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <errno.h>
#include "list.h"

#define LIST_ST_LEN     sizeof(struct _list_st)
#define MAX_MEMSIZE     (50 * 1024 * 1024)
#define BUF_SIZE(x)     (LIST_ST_LEN + x + 3)
typedef struct _list_st list_st;

static  list_st *start = (list_st *)NULL;
static  list_st *end = (list_st *)NULL;
static  int     nlist = 0;
static  memfree(list_st *bf);

static  int     semid = -1;
static  int     semkey = -1;
#define _PERM_LINK      (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
/* Semaphore operation for linklist I/O                         */
struct  sembuf  _lock_link[2] = {       /* locking for writer           */
	{ 0,  0,        0 },            /* wait for sem#0 to become 0   */
	{ 0,  1, SEM_UNDO }             /* then increment sem# 0 by 1   */
};

struct  sembuf  _unlock_link[1] = {     /* unlocking for writer         */
	{ 0, -1, SEM_UNDO }             /* decrement sem# 0 by 1(to 0)  */
};

int l_linkinit(char* nm)
{
	semkey = l_token(nm, 'S');
	semid = semget(semkey, 2, _PERM_LINK|IPC_CREAT);
	_linkunlock();
}

/*************************************************************************
 * NAME : _linklock()
 * DESC : locking
 *************************************************************************/
int _linklock()
{
	int     rc;

	if (semkey == -1)
		return (-1);
	if (semid == -1)
		semid = semget(semkey, 2, _PERM_LINK|IPC_CREAT);
	if (semid == -1)
		return (-1);

	rc = semop(semid, &_lock_link[0], 2);
	return (rc);
}

/*************************************************************************
 * NAME : _linkunlock()
 * DESC : Unlocking
 *************************************************************************/
int _linkunlock()
{
        union   semun {
                int     val;
                struct  semid_ds *dsb;
                ushort  *arr;
        } semun;
        int     rc;
        ushort  semval[6];

        if (semid == -1)
                return (0);
        semun.arr = semval;
#if     (defined(AIX) || defined(HPUX))
        if ((rc = semctl(semid, 2, GETALL, semval)) != 0)
#else
        if ((rc = semctl(semid, 2, GETALL, semun)) != 0)
#endif
                return (0);
        if (semval[0] == 0)
                return (0);
        rc = semop(semid, &_unlock_link[0], 1);
        return (rc);
}

int     l_addlist(int p, int len)
{
        list_st *newp, *tmp;
        int     ii;

        _linklock();
        for(ii = 0; ii< 10;ii++)
        {
                newp = (list_st *) malloc(BUF_SIZE(len));
                if(newp != NULL)
                        break;
        }
        if(newp == NULL)
        {
                _linkunlock();
                return -1;
        }
        newp->pid = p;
        newp->next = NULL;

        if( start == NULL ) 
	{
                end = newp;
                start = newp;
        }
        else 
	{
                newp->prev = end;
                tmp = end;
                end = newp;
                if( start != NULL )
                        tmp->next = newp;
                else
                {
                        end = newp;
                        start = newp;
                }
        }
        nlist++;
        _linkunlock();
        return  nlist;
}

int	l_removelist(int p)
{
        list_st *tmp;
	int 	ii;
	
        _linklock();

	for(tmp = end, ii = nlist; ii > 0; ii--)
	{
		if(tmp->pid != p)
		{
			tmp = tmp->prev;
			continue;
		}
		
		if(tmp == start)
		{
			start = tmp->next;
			start->prev = NULL;
		}
		else if(tmp == end)
		{
			end = tmp->prev;
			end->next = NULL;
		}
		else
		{
			tmp->prev->next = tmp->next;
			tmp->next->prev = tmp->prev;
		}

		nlist--;
		memfree(tmp);		

		_linkunlock();
		return 0;
	}

	_linkunlock();
	return -1;
}

int 	l_getindxlist(int indx)
{
        list_st *tmp;
	int	ii;

	if(indx < 0 || indx >= nlist)
		return -1;

        _linklock();

	for(tmp = start, ii = 0; ii < indx; ii++)
		tmp = tmp->next;

        _linkunlock();
	return tmp->pid;
}

int     l_getlist(int p)
{
        int     len;
        list_st *newp;

        if(start == (list_st *)NULL)
                return -1;
        _linklock();
        newp = start;
        start = newp->next;

        p = newp->pid;
        nlist--;
        memfree(newp);
        _linkunlock();
        return len;
}

int     l_removeall()
{
        list_st *tp, *np;
        for(tp=start; tp; tp=np) 
	{
                np = tp->next;
                memfree(tp);
        }
        nlist = 0;
        start = end = NULL;
        return 1;
}

memfree(list_st *bf)
{
        if(bf != NULL)
                free(bf);
        bf = NULL;
}

l_getnlist()
{
	return nlist;
}

/******************************************************************************/
/*  Components  : ipcsub.c                                                    */
/*  Description : Common subroutines for IPC (message queue, shared memory,..)*/
/*  Rev. History: Ver   Date    Description                                   */
/*                ----  ------- ----------------------------------------------*/
/*                1.0   09-05   Initial version                               */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

/******************************************************************************/
/* Procedure Name          Functional Description                             */
/*----------------------------------------------------------------------------*/
/* l_token()       IPC를 위한 시스템에서 유일한 TOKEN VALUE를 산출            */
/* l_msgget()      입력된 QUEUE NAME의 MESSAGE QUEUE를 얻는다.                */
/* l_shmat()       Attach exist shared memory.                                */
/* l_putmsg()      TRANSACTION MESSAGE를 입력된 QUEUE ID 로 전달              */
/* l_putmsgx()     TRANSACTION MESSAGE를 입력된 QUEUE NAME의 QUEUE로 전달     */
/* l_getmsg()      TRANSACTION MESSAGE를 입력된 QUEUE ID에서 수신             */
/* l_getmsgx()     TRANSACTION MESSAGE를 입력된 QUEUE NAME의 QUEUE로          */
/*                 부터 수신 처리                                             */
/******************************************************************************/
/*********************** 16 bit CRC TABLE for TOKEN ***************************/
static unsigned short s_crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

#define CRC(data, accum) ((accum>>8)^s_crctab[(accum^(data&0x00ff))&0x00ff])


static  struct  queue_table {
        char    queue_nm[16];                   /* queue name           */
        int     queue_id;                       /* queue id             */
} s_queue_table[32];

static  int l_name2id();

static  int     s_queue_n = 0;                  /* valid queue table #  */
static  char    s_io_buf[1024*1024];            /* i/o buffer           */
static  struct  msgbuf *s_msgbuf;               /* msg i/o struct ptr   */


/******************************************************************************/
/* NAME : l_token()                                                           */
/* DESC : 입력된 IPC NAME 및 TYPE에 의하여 유일한 IPC-ID를 산출한다.          */
/*        TOKEN 값은 CRC TABLE에 의하여, IPC-NAME에 해당하는 16 BIT CRC를 계산*/
/*        하여 IPC-TYPE을 16 BIT << SHIFT한 값으로 한다.                      */
/* IN   : ipc_name      MESSAGE QUEUE 및 SHREAD MEMORY의 ID 명.               */
/*        ipc_type      'Q' = MESSAGE QUEUE, 'M' = SHARED MEMORY              */
/* RET  : TOKEN VALUE.                                                        */
/******************************************************************************/
int l_token(ipc_name, ipc_type)
char    *ipc_name;
int     ipc_type;
{
        unsigned char    w_cc;
        int      w_crc,  w_token;
        register int ii;

        switch (ipc_type)
        {
        case 'Q': break;                /* message queue */
        case 'M': break;                /* shared memory */
        case 'S': break;                /* semaphore     */
        default:  errno = EFAULT;       /* bad address   */
                  return (-1);
        }
	
	w_crc = 0;
        for (ii = 0; ii < strlen(ipc_name); ii++)
        {
                w_cc  = ipc_name[ii];
                w_crc = CRC(w_cc, w_crc);
        }
        w_token = (ipc_type << 16) | w_crc;
        return (w_token);
}

/******************************************************************************/
/* NAME :  l_msgget()                                                         */
/* DESC :  Get message queue ID ny queue-name(ipc_name).                      */
/* IN   :  queue_name   message queue 명 				      */
/*         size         message queue의 크기 (최대 64K)                       */
/*         mode         msgget() system call의 MessageFlag                    */
/*                      if IPC_CREAT bit on, creat message queue and adjust   */
/*                      queue size, else ignore message queue size.           */
/* RET  :  message queue ID.                                                  */
/******************************************************************************/
l_msgget(queue_name, size, mode)
char    *queue_name;
int     size, mode;
{
        struct  msqid_ds w_msqid_ds;
        int     w_qkey, w_qid;
        int     w_retc;

        if (size >= (1024*1024))
                size = (1024*1024-1);
        if (mode & IPC_CREAT && size == 0)
        {
                errno = EFAULT;                 /* Oh! no queue size    */
                return (-1);
        }
        if ((w_qkey = l_token(queue_name, 'Q')) == -1)
                return (-1);
        if ((w_qid = msgget(w_qkey, mode)) == -1)
        {
                return (-1);
        }

	if (mode & IPC_CREAT)
        {
                w_retc = msgctl(w_qid, IPC_STAT, &w_msqid_ds);
                if (w_retc == 0 && w_msqid_ds.msg_qbytes != size)
                {
                        w_msqid_ds.msg_qbytes = size;
                        w_retc = msgctl(w_qid, IPC_SET, &w_msqid_ds);
                }
        }
        return (w_qid);
}

static  struct  {
        key_t    shm_key;
        char    *shm_xxx;
} shmtbl[32];
/******************************************************************************/
/* NAME :  l_shmat()                                                          */
/* DESC :  Attach exist shared memory.                                        */
/* IN   :  shm_name     shared memory id name				      */
/* RET  :  Attached memory pointer. If error, return NULL.                    */
/******************************************************************************/
char    *l_shmat(shm_name)
char    *shm_name;
{
        int     w_mkey, w_mid;
        int     w_retc;
        char    *w_pointer;
        int     ii, jj;

        if ((w_mkey = l_token(shm_name, 'M')) == -1)
                return (NULL);
        for (ii = 0, jj = -1; ii < 32; ii++)
        {
                if (shmtbl[ii].shm_key == 0)
                {
                        if (jj == -1)
                                jj = ii;
                }
                if (w_mkey == shmtbl[ii].shm_key)
                        return(shmtbl[ii].shm_xxx);
        }
	if ((w_mid = shmget(w_mkey, 0, S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP)) == -1)
                return (NULL);
        w_pointer = shmat(w_mid, (char *)0, 0);
        if (jj != -1 && w_pointer != (char *)-1)
        {
                shmtbl[jj].shm_key = w_mkey;
                shmtbl[jj].shm_xxx = w_pointer;
        }
        return (w_pointer);
}


/******************************************************************************/
/* NAME : l_putmsg()                                                          */
/* DESC : 입력된 TRANSACTION MESSAGE를 qid 의 MESSAGE QUEUE로 송신 한다.      */
/* IN   : qid           송신할 MESSAGE queue id로 sbpc5003_token()에 의하여   */
/*                      RETURN된 QUEUE id 값이다.                             */
/*        mtype         송신할 MESSAGE TYPE                                   */
/*        trans_b       송신할 TRANSACTION MESSAGE BUFFER                     */
/*        trans_l       송신할 TRANSACTION MESSAGE BUFFER의 DATA LENGTH       */
/*        wait_mode     송신할 MESSAGE QUEUE가 FULL일때, FULL 상태가 해제될때 */
/*                      까지 WAIT할지를 지정 한다(1/0 : 대기/즉시 RETURN)     */
/* RET  : 0 -> 정상 송신, -1 -> ERROR                                         */
/******************************************************************************/
l_putmsg(qid, mtype, trans_b, trans_l, wait_mode)
int     qid, mtype;
char    *trans_b;
int     trans_l, wait_mode;
{
        int     w_retc;

        s_msgbuf = (struct msgbuf *)s_io_buf;
        if (mtype == 0)
                mtype = 1;              /* set default mtype    */
        if (trans_l > 4095)
        {                               /* Oh! size over        */
                errno = E2BIG;
                return (-1);
        }
	s_msgbuf->mtype = mtype;
        memcpy(s_msgbuf->mtext, trans_b, trans_l);
        if (wait_mode)
                w_retc = msgsnd(qid, s_io_buf, trans_l, 0);
        else
                w_retc = msgsnd(qid, s_io_buf, trans_l, IPC_NOWAIT);

        return (w_retc);
}

/******************************************************************************/
/* NAME : l_putmsgx()                                                         */
/* DESC : 입력된 TRANSACTION MESSAGE를 QUEUE 명에 의한 MESSAGE QUEUE로        */
/*        전달 한다.                                                          */
/* IN   : queue_name    송신할 MESSAGE QUEUE명으로 프로그램 ID명으로 사용된다 */
/*        mtype         송신할 MESSAGE TYPE                                   */
/*        trans_b       송신할 TRANSACTION MESSAGE BUFFER                     */
/*        trans_l       송신할 TRANSACTION MESSAGE BUFFER의 DATA LENGTH       */
/*        wait_mode     송신할 MESSAGE QUEUE가 FULL일때, FULL 상태가 해제될때 */
/*                      까지 WAIT할지를 지정 한다(1/0 : 대기/즉시 RETURN)     */
/* RET  : 0 -> 정상 송신, -1 -> ERROR                                         */
/******************************************************************************/
l_putmsgx(queue_name, mtype, trans_b, trans_l, wait_mode)
char    *queue_name;
int     mtype;
char    *trans_b;
int     trans_l, wait_mode;
{
        int     w_retc;
        int     w_qid;

        w_qid = l_name2id(queue_name);
        if (w_qid == -1)
                return (-1);

        return (l_putmsg(w_qid, mtype, trans_b, trans_l, wait_mode));
}
/******************************************************************************/
/* NAME : l_getmsg()                                                          */
/* DESC : 입력된 TRANSACTION MESSAGE를 qid 의 MESSAGE QUEUE로부터 수신 한다.  */
/* IN   : qid           수신할 MESSAGE queue id로 에 의하여 l_name2id         */
/*                      RETURN된 QUEUE id 값이다.                             */
/*        trans_b       수신할 TRANSACTION MESSAGE BUFFER                     */
/*        bsize         수신할 TRANSACTION MESSAGE BUFFER의 크기              */
/*        timeout       수신할 MESSAGE가 있을때 까지의 대기 시간              */
/*                      if (-1), 수신시까지 대기                              */
/*                      if (0),  즉시 RETURN                                  */
/* OUT  : mtype         수신한 MESSAGE의 MESSAGE TYPE                         */
/*        trans_b       수신된 TRANSACTION MESSAGE DATA                       */
/* RET  : 수신한 MESSAGE DATA의 LENGTH                                        */
/******************************************************************************/
l_getmsg(qid, mtype, trans_b, bsize, timeout)
int     qid, *mtype;
char    *trans_b;
int     bsize, timeout;
{
        extern  void _alarm_handler();
        int     w_nque, w_timeout;
        int     w_retc, w_bsize;

        s_msgbuf  = (struct msgbuf *)s_io_buf;
        w_timeout = timeout;
        if (timeout == 0)
                w_retc = msgrcv(qid,s_io_buf,sizeof(s_io_buf),0,IPC_NOWAIT|MSG_NOERROR);
        else if (timeout < 0)
                w_retc = msgrcv(qid,s_io_buf,sizeof(s_io_buf),0,MSG_NOERROR);
        else
        {
                signal(SIGALRM, _alarm_handler);
                alarm(timeout);
                w_retc = msgrcv(qid,s_io_buf,sizeof(s_io_buf),0,MSG_NOERROR);
                alarm(0);
        }
        if (w_retc <= 0)
        {
                if (w_retc == -1 &&
                    (errno == EAGAIN || errno == ENOMSG || errno == EINTR))
                        w_retc = 0;
                return (w_retc);
        }
	w_bsize = bsize;
        if (w_bsize < w_retc)
                w_retc = w_bsize;

        memcpy(trans_b, s_msgbuf->mtext, w_retc);       /* 수신 MSG      */
        *mtype = s_msgbuf->mtype;                       /* 수신 MSG TYPE */

        return (w_retc);                                /* 수신 LENGTH   */
}


/******************************************************************************/
/* NAME : l_getmsgx()                                                         */
/* DESC : 입력된 TRANSACTION MESSAGE를 QUEUE NAME으로 지정된 QUEUE에서 수신   */
/* IN   : queue_name    수신할 MESSAGE QUEUE NAME으로 CALL한 프로그램의       */
/*                      프로그램 ID. 이다.                                    */
/*        trans_b       수신할 TRANSACTION MESSAGE BUFFER                     */
/*        bsize         수신할 TRANSACTION MESSAGE BUFFER의 크기              */
/*        timeout       수신할 MESSAGE가 있을때 까지의 대기 시간              */
/*                      if (-1), 수신시까지 대기                              */
/*                      if (0),  즉시 RETURN                                  */
/* OUT  : mtype         수신한 MESSAGE의 MESSAGE TYPE                         */
/*        trans_b       수신된 TRANSACTION MESSAGE DATA                       */
/* RET  : 수신한 MESSAGE DATA의 LENGTH                                        */
/******************************************************************************/
l_getmsgx(queue_name, mtype, trans_b, bsize, timeout)
char    *queue_name;
int     *mtype;
char    *trans_b;
int     bsize, timeout;
{
        int     w_qid;

        w_qid = l_name2id(queue_name);
        if (w_qid == -1)
                return (-1);
        return (l_getmsg(w_qid, mtype, trans_b, bsize, timeout));
}

/******************************************************************************/
/* NAME : l_name2id()                                                         */
/* DESC : IPC name을 QID 값으로 계산한다. 한 프로그램에서 연속적으로 QUEUE    */
/*        NAME으로 TRANSACTION MESSAGE의 송수신을 하는 경우, CALL 시 마다     */
/*        msgget() CALL에 의하여 QID를 얻는 반복적인 PROCESSING을 피하기 위해 */
/*        공통 프로그램에서 최대 32개의 QUEUE NAME 및 QUEUE ID를 관리 하여    */
/*        신속한 처리를 할 수 있도록 하며, 개별 프로그램에서는 QUEUE 명으로   */
/*        편리한 사용을 제공토록 한다.                                        */
/* IN  : queue_name     MESSAGE QUEUE 명 (프로그램 ID 명)                     */
/* RET : message queue id                                                     */
/* NOTE: 공통 프로그램에서만 사용                                             */
/******************************************************************************/
static int l_name2id(queue_name)
char    *queue_name;
{
        int      w_qid;
        register int ii;

        for (ii = 0; ii < s_queue_n && ii < 32; ii++)
        {
                if (strcmp(s_queue_table[ii].queue_nm, queue_name) == 0)
                        return (s_queue_table[ii].queue_id);
        }
        w_qid = l_msgget(queue_name, 0, 0666);
        if (w_qid == -1)
                return (-1);
        if (s_queue_n < 32)
        {
                strcpy(s_queue_table[ii].queue_nm, queue_name);
                s_queue_table[ii].queue_id = w_qid;
                s_queue_n++;
        }
        return(w_qid);
}

/****************************************************************************/
/* NAME : _alarm_handler()                                                  */
/* DESC : SIGALRM handler                                                   */
/****************************************************************************/
void _alarm_handler(int signo)
{
        signal(SIGALRM, _alarm_handler);
}

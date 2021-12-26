#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <errno.h>

int main()
{
	char	aaa[16] = "ABC";
	char	bbb[16] = "DEF";
    pid_t   xpid;
    int     pair[2], retc;
    struct  iovec   iovec[2], iocc[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0)
        return(-1);

    iovec[0].iov_base = (void *)aaa;
    iovec[0].iov_len  = strlen(aaa);
    iovec[1].iov_base = (void *)bbb;
    iovec[1].iov_len  = strlen(bbb);

    retc = writev(pair[1], iovec, 2);
    if (retc < 0)
    {
        close(pair[0]);
        close(pair[1]);
        return(-1);
    }
printf("writev [%d]\n", retc);

    switch ((xpid = fork()))
    {
    case -1:    /* error    */
        return(-1);
    case  0:    /* child    */
        close(pair[1]);
		memset(&iocc, 0x00, sizeof(iocc));
		retc = readv(pair[0], iocc, 2);
printf("readv [%d][%d]\n", iocc[0].iov_len, iocc[1].iov_len);
        close(pair[0]);
        exit(0);
    default:    /* parent   */
        close(pair[0]);
        close(pair[1]);
        break;
    }

    return(0);
}


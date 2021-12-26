#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>

static struct cmsghdr   *cptr = NULL;  /* malloc'ed first time */

#if 0
/* ! @brief Control Message for sending/receiving FD. */
typedef union _cmsg_fd
{
    struct cmsghdr cmsg;
    char data[CMSG_SPACE(sizeof(int))];
} cmsg_fd;
#endif

int main(int argc, char* argv[])
{
    int sockfd[2];
    int fd, ofd;
    int res;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1)
        return EXIT_FAILURE;

    if (fork())
    {
        sendFD(sockfd[0], fd, (char*)&fd, sizeof(fd));
        close(1);
        printf("standard output is closed\n");
        wait(&res);
    }
    else
    {
        close(1);
        recvFD(sockfd[1], &fd, (char*)&ofd, sizeof(ofd));
        printf("received fd: [%d], extra data: [%d] \n", fd,  ofd);
        if ( fd > -1 )
        {
            printf("error ah, ah standard output is testing...\n");
            dup2(fd, 1);   /* 받은 fd를 자식의 표준출력에 복사한다. */
            printf("hello, world!\n"); 
        }
    }

    return EXIT_SUCCESS;
}

int recvFD(int sockfd, int *fd, char* buf, size_t buflen)
{
    struct msghdr msg;
    struct iovec iov;
#if 0
    cmsg_fd cmsg;
#endif

    *fd = -1;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

#if 0
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    msg.msg_flags = 0;
#endif

    if (-1 == recvmsg(sockfd, &msg, 0))
        return -1;

    if ( SOL_SOCKET != cptr->cmsg_level || SCM_RIGHTS != cptr->cmsg_type )
    {
	printf("invalid control message\n");
        return -1;
    }

    memcpy(fd, (int *)CMSG_DATA(cptr), sizeof(fd));
    return 0;
}

int sendFD(int sockfd, int fd, const char* buf, size_t buflen)
{
    struct msghdr msg;
    struct iovec iov;
#if 0
    cmsg_fd cmsg;
#endif

    iov.iov_base = (void*)buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL; 
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

#if 0
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    msg.msg_flags = 0;
#endif

    cptr->cmsg_len = CMSG_LEN(sizeof(int));
    cptr->cmsg_level = SOL_SOCKET;
    cptr->cmsg_type = SCM_RIGHTS;

    memcpy((int *)CMSG_DATA(cptr), &fd, sizeof(int));

    if ( -1 == sendmsg (sockfd, &msg, 0) )
        return -1;

    return 0;
}



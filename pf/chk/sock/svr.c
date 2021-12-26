#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
        
#define port 7769
#define BUFSZ   4 * 1024

char	*message = "this is test server\n";
                
int	main(argc, argv)        
int     argc;           
char    *argv[];        
{                       
        struct  sockaddr_in     socknet;
        struct  sockaddr_un     sockcli;
        int     sock, csock, argl;
        int     ii, rlen;
        char    buff[BUFSZ];
                
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
                return(-1);

	argl = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &argl, sizeof(argl));
        argl = BUFSZ;
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &argl, sizeof(int));
        argl = BUFSZ;   
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &argl, sizeof(int));
        socknet.sin_family = AF_INET;
        socknet.sin_addr.s_addr = INADDR_ANY;
        socknet.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *)&socknet, sizeof(socknet)) != 0)
        {
                close(sock);
                return(-1);
        }

        listen(sock, 32);

        argl = sizeof(struct sockaddr_in);
        csock = accept(sock, (struct sockaddr *)&sockcli, (unsigned int *) &argl);
        if (csock < 0)
	{
		close(sock);
		return(-1);
	}

	for (ii = 0; ii < 3; ii++)
	{
		memset(buff, 0x00, sizeof(buff));
		rlen = read(csock, (char *)buff, sizeof(buff));
		if (rlen <= 0)
		{
			sprintf(buff, "read error errno=%d\n", errno);
			write(csock, (char *)buff, strlen(buff));
			continue;
		}

		write(csock, (char *)buff, strlen(buff));
	}
#if 0
	write(csock, (char *)message, strlen(message));
#endif

        close(sock);
        close(csock);
        return(0);
}

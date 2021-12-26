#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "pnox.h"

#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
 
int lockfile(int);

/****************************************************************************
 * NAME : already_running
 ***************************************************************************/
int	already_running()
{
	int     fd;
	char    buf[16];
	char	path[128];

	sprintf(path, "%s/%s", getenv("PNOX_HOME"), IPC_PATH);
	mkdir(path, 0755);
	sprintf(path, "%s/%s/pnoxd.pid",getenv("PNOX_HOME"), IPC_PATH);
	fd = open(path, O_RDWR|O_CREAT, LOCKMODE);
	if (fd < 0) 
	{
		pxsyslog("pnoxd", "can't open %s: %s", path, strerror(errno));
		return(1);
	}

	if (lockfile(fd) < 0) 
	{
		if (errno == EACCES || errno == EAGAIN) 
		{
			close(fd);
			return(1);
		}
		pxsyslog("pnoxd", "can't lock %s: %s", path, strerror(errno));
		return(1);
	}

	ftruncate(fd, 0);
pxsyslog("pnoxd", "pnoxd pid[%d]", getpid());
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf)+1);
	return(0);
}

/****************************************************************************
 * NAME : lockfile
 ***************************************************************************/
int	lockfile(fd)
int	fd;
{
	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

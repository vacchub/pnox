#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "pnox.h"

/*****************************************************************************/
/* NAME	: pxputenv()					 	     	     */
/*****************************************************************************/
int	pxputenv(pnox_home)
char	*pnox_home;
{
	FILE *fd;
	char fnam[256], buf[256], cmd[256];
	char *xenvs;

	sprintf(fnam, "%s/%s/%s", pnox_home, CFG_PATH, "pnoxenv.cfg");
        fd = fopen(fnam, "r");
        if (fd == NULL)
		return(-1);

	while ((fgets(buf, sizeof(buf), fd)) == buf)
	{
		if (buf[0] == '#' || buf[0] == 0 || buf[0] == '\n')
			continue;
		if (strlen(buf) <= 0)
			continue;

		sprintf(cmd, "%.*s", (int)strlen(buf), buf);
		if (cmd[strlen(cmd) - 1] == '\n')
			cmd[strlen(cmd) - 1] = '\0';

		xenvs = (char *)malloc(256);
		strcpy(xenvs, cmd);
		putenv(xenvs);
	}

	fclose(fd);
	return(0);
}

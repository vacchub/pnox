#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include "pnox.h"
#include "pxmon.h"

void print_usage();
int confirm_option(char);

int main(argc, argv)
int	argc;
char	*argv[];
{
	struct eventmsg eventmsg;
	char	option;
	char    pnox_home[256], cmd[256];
	int	retc;
	
	if (argc < 2)
	{
		print_usage();	
		exit(0);
	}

	retc = homedir_pxmon(pnox_home, NULL);
	if (retc < 0)
	{
		printf("homedir error %d\n", retc);
		return(0);
	}

	if (strlen(argv[1]) == 2)
	{
		if (confirm_option(argv[1][1]) < 0)
			return(-1);

		option = argv[1][1];
	}
	else
		return(-1);

	memset(&eventmsg, 0x00, sizeof(struct eventmsg));
	eventmsg.opt = option;
	if (argc == 2)
		memcpy(eventmsg.arg, "", sizeof(eventmsg.arg));
	else if (argc == 3)
		memcpy(eventmsg.arg, argv[2], sizeof(eventmsg.arg));

	sprintf(cmd, "%s/%s/PXIPC", pnox_home, IPC_PATH);
	retc = pxsvrsnd(cmd, (char *)&eventmsg, sizeof(struct eventmsg));
	if (retc < 0)
	{
		printf("%s fail...\n", argv[0]);
		return(-1);
	}

	return(0);
}

void print_usage()
{
	printf("Usage : pxinit -[q|r|k] [argument]\n");
	printf("    -q : pnox reload all\n");
	printf("    -r : pnox reload once [PID]\n");
	printf("    -k : pnox kill\n");
}

int confirm_option(char opt)
{
	switch (opt)
	{
	case 'q' : return(0);
	case 'r' : return(0);
	case 'k' : return(0);
	}

	return(-1);
}

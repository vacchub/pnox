/***********************************************************
 * NAME : pxusleep.c
 * DESC : pnox platform usleep
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(argc, argv)
int     argc;
char    *argv[];
{
	int delay;

	if (argc < 2)
	{
		printf("Usage : %s usleep\n", argv[0]);
		return(0);
	}

	delay = atoi(argv[1]);
	usleep(delay);

	return(0);
}

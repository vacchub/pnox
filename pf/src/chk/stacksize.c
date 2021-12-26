#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

size_t sizeofstack;
pthread_attr_t attr;

int main(int argc, char *argv[]) 
{
	pthread_attr_init(&attr);
	sizeofstack = 1024;
#if 0
	pthread_attr_setstacksize(&attr, sizeofstack*30);
#endif
	pthread_attr_getstacksize(&attr, &sizeofstack);
	printf("stack size= %d\n", sizeofstack);
	exit(0);
}

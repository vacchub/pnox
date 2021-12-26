#include <stdio.h>
#include <curses.h>
#include <signal.h>

void terminate();

int main()
{
	int ii;

	initscr();

	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);
	signal(SIGSTOP, terminate);
	signal(SIGKILL, terminate);
	signal(SIGSEGV, terminate);

	for (ii = 0;; ii++)
	{
		clear();
		mvprintw(0, 0, "ii = [%d]", ii);
		move(20, 0);
		refresh();
		sleep(1);
	}

	terminate();
	return 0;
}

void terminate()
{
	endwin();
	exit(0);
}

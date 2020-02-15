#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

const char loading_string[] = "-\\|/";
volatile int go;

void stop(int signo)
{
	go = 0;
}

int main()
{
	go = 1;
	signal(SIGINT, stop); 
	struct winsize w;
	unsigned count = 0;
	printf("\e[?25l");
	while(go)
	{
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		printf("\e[2J");
		printf("\e[%u;%uH%c", w.ws_row/2, w.ws_col/2, loading_string[count]);
		printf("\e[0;0H");
		fflush(stdout);
		count = (count + 1) % 4;
		usleep(300000);
	}
	printf("\e[2J\e[0;0H\e[?25h");
	return 1;
}

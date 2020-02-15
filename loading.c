/*
 * Copyright 2020 Matthew Brown
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

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

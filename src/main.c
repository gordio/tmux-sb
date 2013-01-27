/*
 * Gordienko Oleg aka Gordio <admin@gordio.pp.ua>
 * Last edited: 2013-01-21
 * Licensed: MIT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "server.h"
#include "client.h"
#include "main.h"



int main(int argc, const char *argv[], const char *envp[])
{
	/*
	// CPU
	long double a[4], b[4], cpuLoad;
	FILE *cpu_fd = open_file("/proc/stat");

	fscanf(cpu_fd, "cpu  %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
	rewind(cpu_fd);
	setbuf(cpu_fd, NULL);

	// Wait 1 second
	sleep(1);

	fscanf(cpu_fd, "cpu  %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
	rewind(cpu_fd);
	setbuf(cpu_fd, NULL);

	// find delta and find prc
	cpuLoad = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
	cpuLoad *= 100;
	*/

	bool server_runned = false;
	pid_t pid = -1;

	if (chdir("/tmp/")) {
		errx(1, "Can't change directory to /tmp");
	}

	// Check sfdet exist
	struct stat file_stat;
	if (stat("/tmp/" SOCKET_NAME, &file_stat) == 0) {
		server_runned = true;
	}


	// Run server if don't exist
	if (!server_runned) {
		pid = fork();
		if (pid < 0) {
			errx(1, "Fork error.");
		}

		if (pid == 0) {
			start_server(SOCKET_NAME);
		}
	}

	client();

	return 0;
}

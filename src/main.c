/*
 * Gordienko Oleg aka Gordio <admin@gordio.pp.ua>
 * Last edited: 2013-01-21
 * Licensed: MIT
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#include "server.h"
#include "main.h"


inline FILE *
open_file(char *name)
{
	FILE *fd = NULL;

	fd = fopen(name, "r");
	if (!fd) {
		fprintf(stderr, "Failed to open file '%s'", name);
		exit(EXIT_FAILURE);
	}

	return fd;
}

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


	struct sysinfo sysInfo;
	sysinfo(&sysInfo);

	// MEM
	unsigned int memTotal, memFree, memBuffed, memCached, memPrc;
	FILE *ram_fd = open_file("/proc/meminfo");

	fscanf(ram_fd, "MemTotal: %u kB\n", &memTotal);
	fscanf(ram_fd, "MemFree: %u kB\n", &memFree);
	fscanf(ram_fd, "Buffers: %u kB\n", &memBuffed);
	fscanf(ram_fd, "Cached: %u kB\n", &memCached);

	memPrc = (memFree + memCached) / (memTotal / 100);

	// SWAP
	unsigned long swpFree, swpTotal, swpPrc;

	swpTotal = sysInfo.totalswap * sysInfo.mem_unit;
	swpFree = sysInfo.freeswap * sysInfo.mem_unit;
	if (swpTotal != 0) {
		swpPrc = swpFree / (swpTotal / 100);

		printf("CPU:%3.Lf%% #[fg=green]|#[fg=default] MEM:%3.0i%% SWAP:%2.1lu%%",
				cpuLoad, 100 - memPrc, 100 - swpPrc);
	} else {
		printf("CPU:%3.Lf%% #[fg=green]|#[fg=default] MEM:%3.0i%%",
				cpuLoad, 100 - memPrc);
	}


	fclose(ram_fd);
	fclose(cpu_fd);
	*/

	struct stat file_stat;

	
	if (chdir("/tmp/")) {
		errx(1, "Can't change directory to /tmp");
	}

	if (stat(SOCKET_NAME, &file_stat) == -1) {
		err(1, "Server created;");
	}

	pid_t pid;


	pid = fork();
	if (pid < 0) {
		errx(1, "Fork error.");
	}

	if (pid == 0) {
		start_server();
	} else {
	}

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/dvtm-statusbar"
#define UPDATE_SEC 1 // WARNING: UPDATE_SEC > 0
#define NET_INTERFACE "ppp0"


char buf[256];

FILE *
open_file(char *name)
{
	FILE *fd;
	if (!(fd = fopen(name, "r"))) {
		fprintf(stderr, "Failed to open file '%s'", name);
		exit(EXIT_FAILURE);
	}

	return fd;
}

void
get_net_data(FILE *fd, char *iface, uint64_t *in, uint64_t *out)
{
	fscanf(fd, "%*[^\n] %*[^\n] "); /* skip two lines */
	do {
		// read interface line
		fscanf(fd, "%7[^:]:%lu %*u %*u %*u %*u %*u %*u %*u"
			  "%lu %*[^\n] ", buf, in, out);
		// check seted interface?
		if (strcmp(buf, iface) == 0) {
			break;
		}
	} while (!feof(fd));
	// go to start and clean buffer
	rewind(fd);
	setbuf(fd, NULL);
}

int main(int argc, const char *argv[], const char *envp[])
{
	// NET
	uint64_t net_total_in_old = -1, net_total_out_old = -1;
	uint64_t net_total_in = 0, net_total_out = 0;
	float net_speed_in, net_speed_out;
	FILE *net_fd = open_file("/proc/net/dev");

	// MEM
	struct sysinfo memInfo;
	unsigned long memFree, memTotal, memPrc;
	unsigned long swpFree, swpTotal, swpPrc;

	// TIME
	time_t current;
	struct tm *timeptr;

	// CPU
	long double a[4], b[4], cpuLoad;
	FILE *cpu_fd = open_file("/proc/stat");

	// FIFO
	int fifo;
	fifo = mkfifo(FIFO_NAME, 0777);
	if (!access(FIFO_NAME, F_OK) == 0) {
		fprintf(stderr, "Can't create FIFO'\n");
		exit(EXIT_FAILURE);
	}

	if ((fifo = open(FIFO_NAME, O_WRONLY)) == -1) {
		fprintf(stderr, "Can't open FIFO\n");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		// NET HACK: save old value
		if (net_total_in_old == -1) {
			get_net_data(net_fd, NET_INTERFACE, &net_total_in, &net_total_out);
		}


		// CPU
		fscanf(cpu_fd, "cpu  %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
		rewind(cpu_fd);
		setbuf(cpu_fd, NULL);

		/* SET SYSTEM SLEEP */
		sleep(UPDATE_SEC);

		fscanf(cpu_fd, "cpu  %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
		rewind(cpu_fd);
		setbuf(cpu_fd, NULL);

		cpuLoad = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
		cpuLoad *= 100;


		// MEM
		sysinfo(&memInfo);
		memTotal = memInfo.totalram * memInfo.mem_unit;
		memFree = memInfo.freeram * memInfo.mem_unit;
		memPrc = memFree / (memTotal / 100);
		// SWAP
		swpTotal = memInfo.totalswap * memInfo.mem_unit;
		swpFree = memInfo.freeswap * memInfo.mem_unit;
		swpPrc = swpFree / (swpTotal / 100);


		// NET
		net_total_in_old = net_total_in;
		net_total_out_old = net_total_out;
		get_net_data(net_fd, NET_INTERFACE, &net_total_in, &net_total_out);
		net_speed_in = (float)(net_total_in - net_total_in_old) / 1024;
		net_speed_out = (float)(net_total_out - net_total_out_old) / 1024;
		// this is magic - sorry bad english
		net_speed_in /= UPDATE_SEC;
		net_speed_out /= UPDATE_SEC;


		// TIME
		current = time(NULL);
		timeptr = localtime(&current);

		// OUTPUT
		sprintf(buf, "DOWN:%5.2fKb UP:%5.2fKb | "
				"SWAP:%3.2lu%% MEM:%3.2lu%% | "
				"CPU:%3.Lf%% | %.2d:%.2d\n",
				net_speed_in, net_speed_out,
				100 - swpPrc, 100 - memPrc,
				cpuLoad, timeptr->tm_hour, timeptr->tm_min);
		write(fifo, buf, strlen(buf));
	}

	close(fifo);
	fclose(net_fd);
	fclose(cpu_fd);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h> // with sys/socket.h for cross
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

#include "main.h"
#include "server.h"

static int init_sock(struct sockaddr_un *addr);
static void deinit_sock(int sock, struct sockaddr_un *addr);
inline static FILE * open_file(char *name);


void
start_server(const int update_interval, const char *file)
{
	int sock, csock;
	struct sockaddr_un addr;
	char *buf;
	socklen_t addr_len;

	// mem
	FILE *ram_fd;
	unsigned int memTotal, memFree, memBuffed, memCached, memPrc;
	// swap
	struct sysinfo sysInfo;
	unsigned long swpFree, swpTotal, swpPrc;
	// cpu
	FILE *cpu_fd;
	long double a[4], b[4], cpuLoad = 0;


	if (!(buf = malloc(BUF_SIZE))) {
		err(1, "Can't allocate buffer %i KiB.", BUF_SIZE/1024);
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, file, (sizeof addr.sun_path) - 1);

	sock = init_sock(&addr);

	ram_fd = open_file("/proc/meminfo");


	while (true) {
		addr_len = sizeof addr;

		// wait connection
		if (-1 == (csock = accept(sock, (struct sockaddr *) &addr, &addr_len))) {
			errx(1, "Accept error %i: %s", errno, strerror(errno));
		}

		// memory
		if (1 != fscanf(ram_fd, "MemTotal: %u kB\n", &memTotal) ||
			1 != fscanf(ram_fd, "MemFree: %u kB\n", &memFree) ||
			1 != fscanf(ram_fd, "Buffers: %u kB\n", &memBuffed) ||
			1 != fscanf(ram_fd, "Cached: %u kB\n", &memCached)) {
			errx(-1, "Fail parse mem file.");
		}
		memPrc = (memFree + memCached) / (memTotal / 100);
		rewind(ram_fd);
		setbuf(ram_fd, NULL);

		// swap
		sysinfo(&sysInfo);
		swpTotal = sysInfo.totalswap * sysInfo.mem_unit;
		swpFree = sysInfo.freeswap * sysInfo.mem_unit;

		if (swpTotal != 0) {
			swpPrc = swpFree / (swpTotal / 100);

			snprintf(buf, BUF_SIZE - 1, "CPU:%3.Lf%% #[fg=green]|#[fg=default] MEM:%3.0i%% SWAP:%2.1lu%%",
					cpuLoad, 100 - memPrc, 100 - swpPrc);
		} else {
			snprintf(buf, BUF_SIZE - 1, "CPU:%3.Lf%% #[fg=green]|#[fg=default] MEM:%3.0i%%",
					cpuLoad, 100 - memPrc);
		}

		buf[BUF_SIZE - 1] = '\0'; // hard deny overflow
		send(csock, buf, strlen(buf), 0);
	}

	deinit_sock(sock, &addr);

	fclose(ram_fd);
}

static int
init_sock(struct sockaddr_un *addr)
{
	int sock;


	// Create socket
	if (-1 == (sock = socket(AF_UNIX, SOCK_STREAM, 0))) {
		errx(1, "Can't create socket.");
	}

	// Bind to file
	if (-1 == bind(sock, (struct sockaddr *) addr, sizeof *addr)) {
		errx(1, "Can't bind socket to file '%s': %s", addr->sun_path, strerror(errno));
	}

	// Listening
	if (-1 == listen(sock, LISTEN_BACKLOG)) {
		errx(1, "Can't list socket: %s", strerror(errno));
	}

	return sock;
}

static void
deinit_sock(int sock, struct sockaddr_un *addr)
{
	close(sock);
	unlink(addr->sun_path);
}

inline static FILE *
open_file(char *name)
{
	FILE *fd = NULL;

	if (!(fd = fopen(name, "r"))) {
		errx(1, "Failed to open file '%s': %s", name, strerror(errno));
	}

	return fd;
}

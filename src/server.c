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


#ifndef MIN
	#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
	#define MAX(a,b) ((a)>(b)?(a):(b))
#endif


static int init_sock(struct sockaddr_un *addr);
static void deinit_sock(int sock, struct sockaddr_un *addr);
inline static FILE * open_file(char *name);


void
start_server(const char *file)
{
	int ssock, csock;
	struct sockaddr_un addr;
	char *buf;
	socklen_t addr_len;

	// mem
	FILE *ram_fd = NULL;
	unsigned int mem_total, mem_free, mem_buffered, mem_cached, mem_prc;
	// swap
	struct sysinfo sysInfo;
	unsigned long swp_free, swp_total, swp_prc;
	// cpu
	FILE *cpu_fd = NULL;
	unsigned long int a[9], b[9], work_period, total_period;
	int cpu_prc = 0;


	if (!(buf = malloc(BUF_SIZE))) {
		err(1, "Can't allocate buffer %i KiB.", BUF_SIZE/1024);
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, file, (sizeof addr.sun_path) - 1);

	ssock = init_sock(&addr);

	ram_fd = open_file("/proc/meminfo");
	cpu_fd = open_file("/proc/stat");


	// get "old" values
	if (7 != fscanf(cpu_fd, "cpu  %lu %lu %lu %lu %lu %lu %lu", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6])) {
		errx(1, "Fail parse cpu stat.");
	} else {
		// work (user + nice + system)
		b[7] = b[0] + b[1] + b[2];
		// total
		b[8] = b[7] + b[3] + b[4] + b[5] + b[6];
	}
	rewind(cpu_fd);
	setbuf(cpu_fd, NULL);
	

	while (true) {
		addr_len = sizeof addr;

		// wait connection
		if (-1 == (csock = accept(ssock, (struct sockaddr *) &addr, &addr_len))) {
			errx(1, "Accept error %i: %s", errno, strerror(errno));
		}

		// cpu  nice user system irq soft_irq io_wait steal guest
		if (7 != fscanf(cpu_fd, "cpu  %lu %lu %lu %lu %lu %lu %lu", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6])) {
			errx(1, "Fail parse cpu stat.");
		}
		// work (user + nice + system)
		a[7] = a[0] + a[1] + a[2];
		// total
		a[8] = a[7] + a[3] + a[4] + a[5] + a[6];
		// periods
		work_period = a[7] - b[7];
		total_period = a[8] - b[8];
		cpu_prc = ((float)work_period / total_period) * 100.0;
		// min/max value
		cpu_prc = MIN(100, MAX(0, cpu_prc));
		// a -> b
		memcpy(b, a, sizeof b);

		// reset fd
		rewind(cpu_fd);
		setbuf(cpu_fd, NULL);

		// memory
		if (1 != fscanf(ram_fd, "MemTotal: %u kB\n", &mem_total) ||
			1 != fscanf(ram_fd, "MemFree: %u kB\n", &mem_free) ||
			1 != fscanf(ram_fd, "Buffers: %u kB\n", &mem_buffered) ||
			1 != fscanf(ram_fd, "Cached: %u kB\n", &mem_cached)) {
			errx(-1, "Fail parse mem file.");
		}
		mem_prc = (mem_free + mem_cached) / (mem_total / 100);

		rewind(ram_fd);
		setbuf(ram_fd, NULL);

		// swap
		sysinfo(&sysInfo);

		if (sysInfo.totalswap != 0) {
			// swap exist
			swp_total = sysInfo.totalswap * sysInfo.mem_unit;
			swp_free = sysInfo.freeswap * sysInfo.mem_unit;
			swp_prc = swp_free / (swp_total / 100);

			snprintf(buf, BUF_SIZE - 1,
					"CPU:%3.u%% #[fg=green]|#[fg=default] MEM:%3.0i%% SWAP:%2.1lu%%",
					cpu_prc, 100 - mem_prc, 100 - swp_prc);
		} else {
			// no swap (exclude divide by zero)
			snprintf(buf, BUF_SIZE - 1,
					"CPU:%3.u%% #[fg=green]|#[fg=default] MEM:%3.0i%%",
					cpu_prc, 100 - mem_prc);
		}

		buf[BUF_SIZE - 1] = '\0'; // hard deny overflow
		send(csock, buf, strlen(buf), 0);

		// close client socket
		close(csock);
	}

	deinit_sock(ssock, &addr);

	fclose(cpu_fd);
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
	if (-1 == listen(sock, CLIENTS_QUEUE)) {
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

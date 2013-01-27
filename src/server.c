#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h> // with sys/socket.h for cross
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>

#include "main.h"
#include "server.h"


void
start_server(const int update_interval, const char *file)
{
	int sock;
	struct sockaddr addr;
	socklen_t addr_len;
	char *buf;


	if (!(buf = malloc(BUF_SIZE))) {
		err(1, "Can't allocate buffer %i KiB.", BUF_SIZE);
	}

	memset(&addr, 0, sizeof addr);
	addr.sa_family = AF_UNIX;
	strncpy(addr.sa_data, file, (sizeof addr.sa_data) - 1);

	// Create socket
	if (-1 == (sock = socket(AF_UNIX, SOCK_DGRAM, 0))) {
		errx(1, "Can't create socket.");
	}

	// bind to file
	if (-1 == bind(sock, &addr, sizeof addr)) {
		errx(1, "Can't bind socket to file '%s': %s", addr.sa_data, strerror(errno));
	}


	/*while (true) {*/
		int len;
		addr_len = sizeof addr;
		len = recvfrom(sock, buf, sizeof buf, 0, &addr, &addr_len);
		printf("[%i] %s\n", len, buf);
	/*}*/

	// destroy all
	close(sock);
	unlink(addr.sa_data);
}

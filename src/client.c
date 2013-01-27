#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <err.h>
#include <errno.h>

#include "main.h"
#include "client.h"


void
client(void)
{
	int sock;
	struct sockaddr addr;
	char *buf;

	if (!(buf = malloc(BUF_SIZE))) {
		errx(-2, "Can't alloc buffer: %s", strerror(errno));
	}
	
	if (-1 == (sock = socket(AF_UNIX, SOCK_DGRAM, 0))) {
		errx(-2, "Can't create socket: %s", strerror(errno));
	}

	addr.sa_family = AF_UNIX;
	strncpy(addr.sa_data, SOCKET_NAME, sizeof addr.sa_data);

	strcpy(buf, "test");
	sendto(sock, buf, sizeof buf, 0, &addr, sizeof addr);
}

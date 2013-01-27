#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"
#include "client.h"


void
client(void)
{
	int sock;
	struct sockaddr_un addr;
	char *buf;

	if (!(buf = malloc(BUF_SIZE))) {
		errx(-2, "Can't alloc buffer: %s", strerror(errno));
	}
	
	if (-1 == (sock = socket(AF_UNIX, SOCK_STREAM, 0))) {
		errx(-2, "Can't create socket: %s", strerror(errno));
	}

	// init socket addr
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_NAME, sizeof addr.sun_path);

	int cur_retr = 0;
	while (cur_retr < CLIENT_RET_COUNT) {
		if (-1 == connect(sock, (struct sockaddr *) &addr, sizeof addr)) {
			if (cur_retr < CLIENT_RET_COUNT) {
				// have retry count available - only show error
				fprintf(stderr, "Can't connect. Retry %i/%i\n", cur_retr + 1, CLIENT_RET_COUNT);
			} else {
				// error, exit
				errx(1, "Fatal. Can't connect: %s", strerror(errno));
			}
		} else {
			// connect success
			cur_retr = CLIENT_RET_COUNT;
		}
		// wait before retry
		usleep(CLIENT_PAUSE_UTIME);
		cur_retr++;
	}

	int len;
	len = recv(sock, buf, BUF_SIZE - 1, 0);

	buf[BUF_SIZE - 1] = '\0'; // hard deny overflow
	printf("%s\n", buf);
}

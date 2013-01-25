#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main.h"
#include "server.h"


void
start_server(void)
{
	int sock, sock_fd;
	struct sockaddr sock_addr;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		errx(1, "Can't create socket:");
	}

	sock_addr.sa_family = AF_UNIX;
	strncpy(sock_addr.sa_data, SOCKET_NAME, sizeof(sock_addr.sa_data));
	sock_fd = bind(sock, &sock_addr, strlen(sock_addr.sa_data) +
			sizeof(sock_addr.sa_family));
	if (sock_fd < 0) {
		errx(1, "Failed bind socket to file '%s'.\n", sock_addr.sa_data);
	}
	
}

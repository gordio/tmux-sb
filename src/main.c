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
	bool server_runned = false;
	pid_t pid = -1;


	if (chdir("/tmp/")) {
		errx(1, "Can't change directory to /tmp");
	}

	// Check socket exist
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

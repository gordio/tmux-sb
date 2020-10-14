#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/shm.h>


int init()
{
	printf("INIT: Done\n");
	return true;
}

char * query(char *q)
{
	// char msg[10] = "Hi";
	char *mem = shmat(shmid2, (void*)0, 0);
	// strncpy("Hi", msg, 3);
	return msg;
}

int deinit()
{
	return true;
}

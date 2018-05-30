/*
Description: A web server written in C
Author: Viduni Wickramarachchi

Instructions : The port number should be passed as an argument
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define EOL "\r\n"
#define EOL_SIZE 2

int main(int argc, char *argv[]) {

	int sockfd, newsockfd, portno, pid;
	socklen_t clilen;
	struct socketaddr_in server_addr, client_addr;

	//Check whether the port is passed as an argument
	if(argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
}



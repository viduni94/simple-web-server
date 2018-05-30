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
	socklen_t clientlen;
	struct socketaddr_in server_addr, client_addr;

	//Check whether the port is passed as an argument
	if(argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	//socket creation
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  //IPv4 domain, TCP (sock_stream), Protocol value for IP
	if(sockfd < 0)
		error("Error in opening socket");
	bzero((char *) &server_addr, sizeof(server_addr));  //Copies zeroes to the size of the server address to the server address buffer
	portno = atoi(argv[1]);  //Get the port number from the argument, atoi converts string argument to integer

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portno);  //htons converts unsigned short integer hostshort from host byte order to network byte order

	//Bind the socket to the address and the port number
	if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		error("Error in binding");
	//Listening to any client requests with a maximum of 5 connections in the queue
	listen(sockfd, 5);
	clientlen = sizeof(client_addr);

	//Makes the server run continuously and create a seperate process for each connection
	while(1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);
		if(newsockfd < 0)
			error("Error in accepting");
		pid = fork();
		if(pid < 0) 
			error("Error in fork");
		if(pid == 0) { //child process
			close(sockfd);
			connection(newsockfd);
			exit(0);
		} else {
			close(newsockfd);
		}
	} //end of while loop

	close(sockfd);
	return 0;

}



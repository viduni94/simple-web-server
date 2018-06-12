/*
Description: A web server written in C with the use of standard libraries
Author: Viduni Wickramarachchi

Instructions: The port number should be passed as an argument
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

#define BACKLOG 5

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
	listen(sockfd, BACKLOG);
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
			//Process request
			process_request(newsockfd);

			//Print message to show that a connection was received (inet_ntop converts IP to human readable format, get_in_addr gets IP address)
			inet_ntop(client_addr.sin_family, get_in_addr((struct sockaddr *) &client_addr), s, sizeof s);
			printf("Server: Received a connection from %s\n", s);

			exit(0);
		} else {
			close(newsockfd);
		}
	} //end of while loop

	close(sockfd);
	return 0; //Unreachable code

}

//Customized error function
void error(const char *msg) {
	perror(msg);
	exit(1);
}

//Function to process the GET request, check for supported media types, serves files from the hardware and send error codes
int process_request(int fd) {
	char request[500], resource[500], *pointer;
	int fd1, length;

	//Check if requesrt is received
	if(receive_new(fd, request) == 0) {
		printf("Request Receiving Failed\n");
	}
	printf("%s\n", request);

	//Check for a valid request
	//strstr returns the location of HTTP in request and NULL if not present
	pointer = strstr(request, " HTTP/");
	if(pointer == NULL) {
		printf("Not a HTTP request\n");
	} else {
		*pointer = 0;
		pointer = NULL;

		if(strncmp(request, "GET ", 4) == 0) {
			pointer = request + 4;
		}

		if(pointer == NULL) {
			printf("Unknown Request\n");
		} else {
			if(ptr[strlen(ptr) - 1] == '/') {
				strcat(pointer, "index.html");
			}
			strcpy(resource, webroot());
			strcat(resource, pointer);

			//Searches for the first occurence of '.' in the string 'pointer'
			char* s = strchr(pointer, '.');

			int i;
			for(i=0; extensions[i].ext != NULL; i++) {
				if(strcmp(s+1, extensions[i].ext) == 0) {
					fd1 = open(resource, 0_RDONLY, 0);
					printf("Opening \"%s\"\n", resource);
					if(fd1 == -1) {
						printf("404 file not found error\n");
						send_response(fd, "HTTP/1.1 404 Not Found\r\n");
						send_response(fd, "Server : Web Server in C\r\n\r\n");
						send_response(fd, "<html><head><title>404 Not Found</title></head>");
						send_response(fd, "<body><p>404 Not Found: The requested resource could not be found!</p></body></html>");
					//Handling php requests
					} else if(strcmp(extensions[i].ext, "php") == 0) {
						php_cgi(resource, fd);
					}
				}
			}
		}
	}

}

int receive_new() {

}

char* webroot() {

}

void php_cgi(char* script_path, int fd) {

}



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

//Structure for extensions and media types
typedef struct {
	char *ext;
	char *mediatype;
} extn;

int process_request(int fd);
int receive_new(int fd, char *buffer);
char* webroot();
void send_response(int fd, char *msg);
int get_file_size(int fd);
void php_cgi(char* script_path, int fd);

int main(int argc, char *argv[]) {

	int sockfd, newsockfd, portno, pid;
	socklen_t clientlen;
	struct sockaddr_in server_addr, client_addr;

	//Check whether the port is passed as an argument
	if(argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}

	//socket creation
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  //IPv4 domain, TCP (sock_stream), Protocol value for IP
	if(sockfd < 0)
		perror("Error in opening socket");
	bzero((char *) &server_addr, sizeof(server_addr));  //Copies zeroes to the size of the server address to the server address buffer
	portno = atoi(argv[1]);  //Get the port number from the argument, atoi converts string argument to integer

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portno);  //htons converts unsigned short integer hostshort from host byte order to network byte order

	//Bind the socket to the address and the port number
	if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 )
		perror("Error in binding");
	//Listening to any client requests with a maximum of 5 connections in the queue
	listen(sockfd, BACKLOG);
	clientlen = sizeof(client_addr);

	//Makes the server run continuously and create a seperate process for each connection
	while(1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);
		if(newsockfd < 0)
			perror("Error in accepting");
		pid = fork();
		if(pid < 0)
			perror("Error in fork");
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

//Possible media types
extn extensions[] ={
	{"html","text/html" },
	{"php", "text/html" },
	{"txt", "text/plain" },
	{"jpg", "image/jpg" },
	{"jpeg","image/jpeg"},
	{"png", "image/png" },
	{"zip", "image/zip" },
	{"gz",  "image/gz"  },
	{"tar", "image/tar" },
	{"pdf","application/pdf"},
	{0,0}
};

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
					//Open only for reading
					fd1 = open(resource, O_RDONLY, 0);
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
						sleep(1);
						close(fd);
						exit(1);
					} else {
						printf("200 OK, Content-Type: %s\n\n", extensions[i].mediatype);
						send_response(fd, "HTTP/1.1 200 OK\r\n");
						send_response(fd, "Server :  Web Server in C\r\n\r\n");

						//If the request is a GET request
						if(pointer == request + 4) {
							if((length = get_file_size(fd1)) == -1)
								printf("Error in getting size! \n");
							size_t total_bytes = 0;
							ssize_t bytes_sent;
							while(total_bytes < length) {
								//Zero copy optimization
								if((bytes_sent = sendfile(fd, fd1, 0, length - total_bytes)) <= 0) {
									//EINTR - If a signal occurred while the system call was in progress
									//EAGAIN - When performing non-blocking I/O - There is no data available right now, try again later.
									if(errno == EINTR || errno == EAGAIN) {
										continue;
									}
									perror("sendfile");
									return -1;
								}
								total_bytes += bytes_sent;
							}
						}
					}
					break;
				}
				int size = sizeof(extensions) / sizeof(extensions[0]);
				if(i == size-1) {
					printf("415 Unsupported Media Type\n");
			        send_response(fd, "HTTP/1.1 415 Unsupported Media Type\r\n");
			        send_response(fd, "Server : Web Server in C\r\n\r\n");
			        send_response(fd, "<html><head><title>415 Unsupported Media Type</head></title>");
			        send_response(fd, "<body><p>415 Unsupported Media Type!</p></body></html>");
				}
			}
			close(fd);
		}
	}
	/* Makes the full-duplex connection on the socket associated with the
	file descriptor socket to be shut down */
	shutdown(fd, SHUT_RDWR);
}

//Function to receive the buffer until EOL is reached
int receive_new(int fd, char *buffer) {
	char *p buffer; //Pointer to the buffer
	int eol_matched = 0;

	//Receive 1 byte at a time and store it at pointer p (file descriptor, buffer_start, length, flag)
	while(recv(fd, p, 1, 0) != 0) {
		if(*p == EOL[eol_matched]) { //if the byte matched with the first EOL byte : '\r'
			++eol_matched;
			if(eol_matched == EOL_SIZE) { //if both bytes  matches with EOL
				*(p+1-EOL_SIZE) = '\0'; //End the string
				return (strlen(buffer)); //return the bytes received
			}
		} else {
			eol_matched = 0;
		}
		p++; //Increment the pointer to receive the next byte
	}
	return(0);
}


//Returns the webroot location
char* webroot() {
	//open the file 'conf'
	FILE *in = fopen("conf", "rt");
	//read the first line
	char buff[1000];
	fgets(buff, 1000, in);
	//close the stream
	fclose(in);
	//search the last occurrence of new line
	char* newline_pointer = strrchr(buff, '\n');
	if(newline_pointer != NULL)
		*newline_pointer = '\0'; //Assigns the end of line
	return strdup(buff); //Duplicates the buffer

}

//Function to handle PHP requests
void php_cgi(char* script_path, int fd) {
	send_response(fd, "HTTP/1.1 200 OK\n Server: Web server in C\n Connection: close");

	//Duplicates the file descriptor, making them aliases, and deletes the old file descriptor
	dup2(fd, STDOUT_FILENO);
	char script[500];
	strcpy(script, "SCRIPT_FILENAME=");
	strcat(script, script_path);

	//Adds setting to the environment
	putenv("GATEWAY_INTERFACE=CGI/1.1");
	putenv(script);
	putenv("QUERY_STRING=");
	putenv("REQUEST_METHOD=GET");
	putenv("REDIRECT_STATUS=true");
	putenv("SERVER_PROTOCOL=HTTP/1.1");
	putenv("REMOTE_HOST=127.0.0.1");

	//replaces the current running process with a new process
	execl("/usr/bin/php-cgi", "php-cgi", NULL);

}

//Get file size
int get_file_size(int fd) {
	struct stat stat_struct;
	//fstat determines information about files based on the file descriptor
	if(fstat(fd, &stat_struct) == -1)
		return(1);
	//returns file size in bytes
	return (int)stat_struct.st_size;
}

//Function to send response
void send_response(int fd, char *msg) {
	int len = strlen(msg);
	if(send(fd, msg, len, 0) == -1) {
		printf("Error in send\n");
	}
}



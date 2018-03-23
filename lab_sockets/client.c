#include <stdio.h>
#include <stdlib.h>
#include <errno.h> //errno
#include <limits.h> //check port argument size
#include <string.h> //strlen
#include <sys/socket.h> //sockets
#include <sys/types.h>
#include <arpa/inet.h> //inet_addr
#include "read_line.h"

#define MAX_BUF 1024


int main(int argc, char **argv){

	char *port_arg;
	const char* server_name = "localhost";
	int mysock, port;
	struct sockaddr_in server_addr;
	char request_buf[MAX_BUF], reply_buf[2*MAX_BUF];
	errno = 0;

	if(argc!=2){
		printf("[ERROR] Incorrect number of arguments.\n");
	}


	long conv = strtol(argv[1], &port_arg, 10);

	if (errno != 0 || *port_arg != '\0' || conv > INT_MAX) {
		printf("[ERROR] Invalid argument.\n");
		return -1;
	}
	else {
    		// No error
    		port = conv;
    		printf("%d\n", port);
	}

	//Configure socket
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, server_name, &server_addr.sin_addr);
	server_addr.sin_port = htons(port);

	//Create socket
	mysock = socket(PF_INET, SOCK_STREAM,0);
	if (mysock == -1) {
		printf("[ERROR] Error crearting socket\n");
		return -1;
	}
	printf("Socket created\n");

	//Connect to sever
	if (connect(mysock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("[ERROR] Connection failed.\n");
		return -1;
	}
	printf("Connection to server success.\n");


	//Communicate with server
	while(1){

		printf("Type to send a message to the server:\n");
		if(readLine(0, request_buf, MAX_BUF)<0){
			printf("[ERROR] Error reading message.\n");
		}

		if(send_msg(mysock, request_buf, strlen(request_buf)) < 0){
			printf("[ERROR] Error sending message\n");
		}

		if(recv(mysock, reply_buf, MAX_BUF,0) < 0){
			printf("[ERROR] Error receiving message\n");
			break;
		}

		printf("Server reply:\n%s\n",reply_buf);

	}

	close(mysock);
	return 0;
}

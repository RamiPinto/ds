#include <stdio.h>
#include <stdlib.h>
#include <errno.h> //errno
#include <string.h> //strlen
#include <limits.h> //check port argument size
#include <sys/socket.h> //sockets
#include <sys/types.h>
#include <arpa/inet.h> //inet addr
#include "read_line.h"

#define MAX_BUF 1024

int main(int argc, char **argv){

	char * port_arg;
	int socket_desc , c, port;
  struct sockaddr_in server , client;

	//Check arguments
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

	//Socket used for the server configuration
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	//Create socket
    	socket_desc = socket(PF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
        	printf("[ERROR]Could not create socket");
		return 1;
    	}
	printf("Socket created.\n");


	//Bind
    	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        	perror("bind failed. Error");
        	return 1;
    	}
    	printf("Bind done.\n");

    	//Listen
    	if(listen(socket_desc , 3)<0){
				printf("[Error] Could not open socket for listening.\n");
				return 1;
			}

	//Accept and incoming connection
    	printf("Waiting for incoming connections...\n");
    	c = 0;

	//accept connection from an incoming client
	while(0==0){
			int client_sock;
    	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    	if (client_sock < 0){
        	perror("[ERROR]accept failed");
        	return 1;
    	}
    	printf("Connection accepted.\nClient connected with ip address: %s\n",inet_ntoa(client.sin_addr));

			int n = 0;
			int len = 0, maxlen = 100;
			char client_message[MAX_BUF];
			char * message = client_message;

			while ((n = recv(client_sock, message, maxlen, 0)) > 0) {
				message += n;
				maxlen -= n;
				len += n;

				printf("received: '%s'\n", client_message);

				// echo received content back
				send(client_sock,client_message, len, 0);
			}
    	//Receive a message from client
    	/*while( (read_size = recv_msg(client_sock , &client_message , 2*MAX_BUF)) > 0 ){

				pbuffer += n;
				maxlen -= n;
				len += n;

					//Send the message back to client
        	send_msg(client_sock , client_message , strlen(client_message));
    	}

    	if(read_size == 0){
        puts("Client disconnected");
        fflush(stdout);
    	}
   	else if(read_size == -1){
        	perror("recv failed");
    	}

			printf("Client message:\n%s\n",client_message);*/
			close(client_sock);

	}

	close(socket_desc);

    	return 0;

}

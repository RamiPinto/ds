#include <stdio.h>
#include <stdlib.h>
#include <errno.h> //errno
#include <string.h> //strlen
#include <limits.h> //check port argument size
#include <sys/socket.h> //sockets
#include <sys/types.h>
#include <arpa/inet.h> //inet addr
#include <unistd.h> // for close, write, read

#define MAX_BUF 1024

int main(int argc, char **argv){

	char * port_arg;
	int socket_desc , c, port;
	struct sockaddr_in server , client;

	//Check arguments
	if(argc!=2){
		printf("[ERROR] Incorrect number of arguments.\n");
		return -1;
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

	//Socket used for server configuration
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
	while(1){
		int client_sock;
		client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
		if (client_sock < 0){
			perror("[ERROR]accept failed");
			return 1;
		}
		printf("Connection accepted.\nClient connected with ip address: %s\n",inet_ntoa(client.sin_addr));

		int n = 0;
		int len = 0, maxlen = 100;
		char client_message[MAX_BUF] = { '\0' };

		while ((n = recv(client_sock, &client_message, maxlen, 0)) > 0) {
			maxlen -= n;
			len += n;

			printf("received: '%s'\n", client_message);
			int32_t one = htonl(1);
			char *data = (char*) &one;
			// echo received content back
			if(write(client_sock, data, sizeof(int))<=0){
				printf("[ERROR] Error sending message\n");
			}

			*client_message = '\0';
		}

		close(client_sock);

	}

	close(socket_desc);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h> //errno
#include <string.h> //strlen
#include <limits.h> //check port argument size
#include <sys/socket.h> //sockets
#include <sys/types.h>
#include <arpa/inet.h> //inet addr
#include <pthread.h> //threads
#include <unistd.h> // for close
#include "services.h"

pthread_mutex_t mutex_msg;
int busy = TRUE;  //TRUE =1
pthread_cond_t cond_msg;

user_t *usr_list;

void *connection_handler(void *);
ssize_t read_line(int fd, void *buffer, int n);

int main(int argc, char **argv){

	char * port_arg;
	char * pflag = "-p";
	int socket_desc , c, port, client_sock;
	struct sockaddr_in s_server , s_client;

	pthread_t thid;
	pthread_attr_t t_attr;  /*thread atributes*/


	//Check arguments
	if(argc!=3){
		printf("[ERROR] Incorrect number of arguments. Try ./server -p <port>\n");
		return -1;
	}

	if (strcmp(argv[1], pflag) != 0){
		printf("[ERROR] Unknown option %s. Try ./server -p <port>\n", argv[1]);
		return -1;
	}

	long conv = strtol(argv[2], &port_arg, 10);

	if (errno != 0 || *port_arg != '\0' || conv > INT_MAX) {
		printf("[ERROR] Invalid argument.\n");
		return -1;
	}

	port = conv;

	//Socket used for server configuration
	memset(&s_server, 0, sizeof(s_server));
	s_server.sin_family = AF_INET;
	s_server.sin_port = htons(port);
	s_server.sin_addr.s_addr = htonl(INADDR_ANY);

	//Create socket
	if ((socket_desc = socket(PF_INET , SOCK_STREAM , 0)) == -1){
		printf("[ERROR] Could not create socket");
		return -1;
	}
	printf("Socket created.\n");

	//Bind
	if(bind(socket_desc,(struct sockaddr *)&s_server , sizeof(s_server)) < 0){
		perror("[Error] bind failed");
		return -1;
	}
	printf("Bind done.\n");

	//Listen
	if(listen(socket_desc , 3)<0){
		printf("[Error] Could not open socket for listening.\n");
		return -1;
	}

	c = sizeof(struct sockaddr_in);

	pthread_mutex_init(&mutex_msg, NULL);
	pthread_cond_init(&cond_msg, NULL);
	pthread_attr_init(&t_attr);

	/* thread atributes */
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);


	//Accept connection from an incoming client. When a client is accepted, a thread is created for him, while the parent branch keeps listening
	//for new conections
	while(1){

		fprintf(stderr, "%s", "s> ");

		client_sock = accept(socket_desc, (struct sockaddr *)&s_client, (socklen_t*)&c);
		if (client_sock < 0){
			perror("[ERROR]accept failed");
			return 1;
		}
		printf("Connection accepted.\nClient connected with ip address: %s\n",inet_ntoa(s_client.sin_addr));

		// Here we create a thread to handle the connection with the client. Meanwhile, the parent process
		// will still listening

		if( pthread_create( &thid , &t_attr ,  connection_handler , (void*) &client_sock) < 0){
			perror("Could not create thread");
			return 1;
		}

		/*Critical section - wait for thread to manage request*/
		pthread_mutex_lock(&mutex_msg);
		while (busy == TRUE){
			pthread_cond_wait(&cond_msg, &mutex_msg);
		}
		pthread_mutex_unlock(&mutex_msg);
		busy = TRUE;


		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( thid , NULL);

	} // End While



	close(socket_desc);
	return 0;
}

void *connection_handler(void *socket_desc)
{
	int s_local;
	char service_msg[MAX_BUF] = { '\0' };
	char user_msg[MAX_BUF] = { '\0' };
	//char port_msg[MAX_BUF] = { '\0' };

	pthread_mutex_lock(&mutex_msg);

	//Get the socket descriptor
	s_local = *(int*)socket_desc;
	busy = FALSE;

	pthread_cond_signal(&cond_msg);
	pthread_mutex_unlock(&mutex_msg);

	//Receive a message from client
	if (read_line(s_local, service_msg, MAX_BUF) < 0) { // Requested operation.
		printf("[ERROR] Cannot read client request\n"); // Error.
		bzero(service_msg, MAX_BUF); // Clean buffer.
	}

	if (read_line(s_local, user_msg, MAX_BUF) < 0) { // Requested operation.
		printf("[ERROR] Cannot read client request\n"); // Error.
		bzero(service_msg, MAX_BUF); // Clean buffer.
	}

	printf("Received message: %s %s\n", service_msg, user_msg);
	int32_t one = htonl(1);
	char *data = (char*) &one;
	//Send the message back to client
	if(write(s_local , data, sizeof(int))<0){
		printf("[ERROR] Error sending reply\n");
	}

	*service_msg = '\0';
	*user_msg = '\0';


	close(s_local);
	pthread_exit(NULL);
	return 0;
}

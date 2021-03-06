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
#include <inttypes.h> //for print int_32
#include <sys/stat.h>  //files
#include <fcntl.h>	//files
#include "services.h"


pthread_mutex_t mutex_msg;
int busy = TRUE;  //TRUE =1
pthread_cond_t cond_msg;

user_t *usr_list;
unsigned int msg_id = 0;

void *connection_handler(void *);

int main(int argc, char **argv){

	char * port_arg;
	char * pflag = "-p";
	int socket_desc , c, port, client_sock;
	struct sockaddr_in s_server , s_client;
	targs_t *myargs; //For passing arguments to the thread

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

	//Bind
	if(bind(socket_desc,(struct sockaddr *)&s_server , sizeof(s_server)) < 0){
		perror("[Error] bind failed");
		return -1;
	}

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

		// Here we create a thread to handle the connection with the client. Meanwhile, the parent process will still listening

		//Fill arguments
		myargs = malloc(sizeof(targs_t));
		myargs->client_sdesc = client_sock;
		myargs->client_ip = s_client.sin_addr;

		//Create thread
		if( pthread_create( &thid , &t_attr ,  connection_handler , (void*) myargs) < 0){
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


		//TODO: check if needed: Now join the thread , so that we dont terminate before the thread
		pthread_join( thid , NULL);

	} // End While



	close(socket_desc);
	return 0;
}

void *connection_handler(void *myargs)
{
	targs_t *temp_args = (targs_t *) myargs;
	int s_local, temp_result = 2, client_port = 0, file;
	struct in_addr clientip_local;
	uint32_t result = htonl(2);
	char * reply, local_msgid[sizeof(unsigned int)] = "";

	char service_msg[MAX_BUF] = { '\0' }, sender_msg[MAX_BUF] = { '\0' }, receiver_msg[MAX_BUF] = { '\0' }, content_msg[MAX_BUF] = { '\0' };
	char filename_msg[MAX_BUF] = { '\0' }, filelength_msg[MAX_BUF] = { '\0' }, filebuf_msg[FILE_BUF_SIZE] = { '\0' }, md5_msg[FILE_BUF_SIZE] = { '\0' };

	memset(&clientip_local, 0, sizeof(struct in_addr));

	pthread_mutex_lock(&mutex_msg);

	//Get the socket descriptor
	s_local = (int) temp_args->client_sdesc;
	clientip_local = (struct in_addr) temp_args->client_ip;

	free(myargs);

	//Receive a message from client
	if (read_line(s_local, service_msg, MAX_BUF) < 0) { // Get requested service
		printf("[ERROR] Cannot read client request\n");
	}
	else if (read_line(s_local, sender_msg, MAX_BUF) < 0) { // Get client sending the request
		printf("[ERROR] Cannot read client request\n");
	}
	else{

		switch(keyfromstring(service_msg)){
			case REGISTER:
				result = htonl(register_usr(&usr_list, sender_msg));
			break;

			case UNREGISTER:
				result = htonl(remove_usr(&usr_list, sender_msg));
			break;

			case CONNECT:
				//Get port
				if (read_line(s_local, content_msg, MAX_BUF) < 0) { // Requested operation.
					printf("[ERROR] Cannot read client request\n");
				}
				else{
					client_port = atoi(content_msg);
					result = htonl(connect_usr(usr_list, sender_msg, clientip_local, client_port));
				}
			break;

			case DISCONNECT:
				result = htonl(disconnect_usr(usr_list, sender_msg));
			break;

			case SEND:
				if (read_line(s_local, receiver_msg, MAX_BUF) < 0) { // Get receivers name
					printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, content_msg, MAX_BUF) < 0) { // Get content of the message
						printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, md5_msg, MAX_BUF) < 0) { // Get content of the message
						printf("[ERROR] Cannot read client request\n");
				}
				else if(userConnected(usr_list, sender_msg) == TRUE){
					//Get message id and store the message
					msg_id = (msg_id + 1)%UINT_MAX;
					temp_result = store_msg(usr_list, msg_id, sender_msg, receiver_msg, content_msg, (char *) "", 0, md5_msg);

					if(temp_result != 0){
						msg_id = (msg_id - 1)%UINT_MAX;
					}
					else{
						sprintf(local_msgid, "%d", msg_id);
					}

					result = htonl(temp_result);
				}
			break;

			case SENDATTACH:
				if (read_line(s_local, receiver_msg, MAX_BUF) < 0) { // Get receiver name
					printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, filename_msg, MAX_BUF) < 0) { // Get file name
						printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, filelength_msg, MAX_BUF) < 0) { // Get file length
						printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, content_msg, MAX_BUF) < 0) { // Get message content
						printf("[ERROR] Cannot read client request\n");
				}
				else if (read_line(s_local, md5_msg, MAX_BUF) < 0) { // Get message content
						printf("[ERROR] Cannot read client request\n");
				}
				else{
					char pathfileout[MAX_PATH_NAME]="default";
					char buff[MAX_BUF] = { '\0' };
					char *temp_len, *path;
					int n = 0, maxlen = 0, file_len = 0;

					//Copy file
					path = getcwd(buff, MAX_BUF);
					sprintf(pathfileout, "%s//%s//%s",path,"files", (char *)filename_msg);

					maxlen = (int) strtol( filelength_msg, &temp_len, 10);
					file_len = maxlen;

					//Open file to write
					if((file = open(pathfileout, O_RDWR | O_CREAT, 0664))<0){
						perror("Error opening file");
					}

					//Write content
					while(maxlen >0){
						n = recv(s_local,&filebuf_msg,maxlen,0);
						maxlen-=n;
						write(file,filebuf_msg,n);
					}

					close(file);

					//Get message id and store the message
					msg_id = (msg_id + 1)%UINT_MAX;
					temp_result = store_msg(usr_list, msg_id, sender_msg, receiver_msg, content_msg, pathfileout,file_len, md5_msg);

					if(temp_result != 0){
						msg_id = (msg_id - 1)%UINT_MAX;
					}
					else{
						sprintf(local_msgid, "%d", msg_id);
					}

					result = htonl(temp_result);
				}
			break;

			case ERROR:
				printf("[ERROR] Cannot get service code\n");
				result = htonl(2);
			break;

			default:
				printf("[ERROR] Error executing requested service\n");
				result = htonl(2);
			break;
		}
	}

	//Send connect service result back to client
	reply = (char*) &result;
	//printf("Service request result:" "%" PRIu32 "\n", ntohl(result));
	if(write_line(s_local , reply, sizeof(int))<0){
		printf("[ERROR] Cannot send reply\n");
	}

	if((keyfromstring(service_msg) == SEND || keyfromstring(service_msg) == SENDATTACH ) && temp_result == 0){	//If send success then send msg id
		reply = (char*) local_msgid;
		if(write_line(s_local , reply, sizeof(unsigned int))<0){
			printf("[ERROR] Cannot send reply\n");
		}
	}

	//Free buffers
	*service_msg = '\0'; *sender_msg = '\0'; *receiver_msg = '\0'; *content_msg = '\0';
	*filename_msg = '\0'; *filelength_msg = '\0'; *filebuf_msg = '\0', *md5_msg = '\0';


	close(s_local);
	busy = FALSE;

	pthread_cond_signal(&cond_msg);
	pthread_mutex_unlock(&mutex_msg);

	pthread_exit(NULL);
	return 0;
}

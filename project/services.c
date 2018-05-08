#include <stdio.h>
#include <string.h> //strcpy, memcpy
#include <stdlib.h> //exit
#include <errno.h> //perror, errno
#include <unistd.h> //read, write
#include "services.h"


/*
* @brief: Check if an user is already registered
* @param: First element of the users list and name of the user to check
* @return: TRUE (1) if the user is registeres. FALSE (0) if it is not.
*/
int userExists(user_t *head, char *name)
{
	user_t *temp_user = head;

	while(temp_user != NULL){
		if(strcmp(temp_user->usr_name, name) == 0){
			return TRUE;
		}
		temp_user = temp_user->next_usr;
	}

	return FALSE;
}


/*
* @brief: Register a new user in the system if it doesn't exist
* @param: Reference to the first element of the user list and the user data: name, ip address and port
* @return: 0 if success, 1 if user exists and 2 in any other case.
*/
int register_usr(user_t **head_ref, char *name)
{
	if(userExists(*head_ref, name) == FALSE){	//User doesn't exist already

		//Allocate memory for new user
		user_t *new_user = (user_t *) malloc(sizeof(user_t));

		//Fill data
		new_user->usr_name = name;
		new_user->status = OFF;
		new_user->msg = NULL;
		new_user->last_msg = 0;

		//Insert user
		new_user->next_usr = (*head_ref);
		(*head_ref) = new_user;

		//Return success
		printf("REGISTER %s OK\n", name);
		return 0;

	}
	//User already exists
	printf("REGISTER %s FAIL\n", name);
	return 1;

}

/*
* @brief: Remove a registered user from the user list
* @param: Reference to the first element of the user list and the user name
* @return: 0 if success, 1 if user doesn't exist and 2 in any other case.
*/
int remove_usr(user_t **head_ref, char *name)
{
	user_t *temp_user = *head_ref, *prev_user;

	if(userExists(*head_ref, name) == TRUE){

		//If the first node is the user to be removed
		if(temp_user != NULL && strcmp(temp_user->usr_name,name) == 0){
			*head_ref = temp_user->next_usr;
			free(temp_user);
			printf("UNREGISTER %s OK\n", name);
			return 0;
		}

		//Search user
		while(temp_user != NULL && strcmp(temp_user->usr_name,name) != 0){
			prev_user = temp_user;
			temp_user = temp_user->next_usr;
		}

		//Remove user
		prev_user->next_usr = temp_user->next_usr;
		free(temp_user);
		printf("UNREGISTER %s OK\n", name);
		return 0;
	}

	//User is not register
	printf("UNREGISTER %s FAIL\n", name);
	return 1;
}

/*
* @brief: Connect an user to the system and store its ip address and port
* @param: Pointer to first element of the user list and the user name, ip address and port number.
* @return: 0 if success, 1 if user doesn't exist, 2 if user is already connected and 3 in any other case.
*/
int connect_usr(user_t *head, char *name, int socket)
{
	user_t *temp_user = head;
	if(userExists(head, name) == TRUE){

		while(temp_user != NULL){

			if(strcmp(temp_user->usr_name, name) == 0){

				if(temp_user->status == OFF){	//User not connected
					temp_user->status = ON;
					temp_user->socket = socket;

					if(send_pending_msg(head,name) != 0){
						printf("CONNECT %s FAIL\n", name);
						return 3;
					}
					printf("CONNECT %s OK\n", name);
					return 0;
				}
				else{	//User already connected
					printf("CONNECT %s FAIL\n", name);
					return 2;
				}
			}
			temp_user = temp_user->next_usr;
		}

	}
	printf("CONNECT %s FAIL\n", name);
	return 1;
}

/*
* @brief: Disconnect an user from the system and remove its ip address and port
* @param: Pointer to first element of the user list and the user name
* @return: 0 if success, 1 if user doesn't exist, 2 if user is not connected and 3 in any other case.
*/
int disconnect_usr(user_t *head, char *name)
{

	user_t *temp_user = head;
	if(userExists(head, name) == TRUE){

		while(temp_user != NULL){

			if(strcmp(temp_user->usr_name, name) == 0){

				if(temp_user->status == ON){	//User connected
					temp_user->status = OFF;
					temp_user->socket = 0;
					//temp_user->ip = NULL;
					//temp_user->port = 0;
					printf("DISCONNECT %s OK\n", name);
					return 0;
				}
				else{	//User not connected
					printf("DISCONNECT %s FAIL\n", name);
					return 2;
				}
			}
			temp_user = temp_user->next_usr;
		}

	}
	printf("DISCONNECT %s FAIL\n", name);
	return 1;
}

/*user_t *get_user(user_t *head, char *name)
{
	user_t *myusr;


	return myusr;
}*/


/*
* @brief: Store a message at the end of the receivers pending message queue and, if the receiver is connected, all pending messages are sent
* @param: Pointer to first element of the user list and the message data: message id, sender and receiver names and the message content
* @return: 0 if success, 1 if user doesn't exist and 2 in any other case.
*/
int send_msg(user_t *head, int msg_id, char *sender, char *receiver, char *content)
{
	user_t *temp_user = head;
	msg_t *new_msg = NULL;

	if(userExists(head, sender) == TRUE && userExists(head, receiver) == TRUE){

		while(temp_user != NULL){

			if(strcmp(temp_user->usr_name, receiver) == 0){ //Search receiver
				//Allocate memory for new message
				msg_t *head_msg = temp_user->msg;
				new_msg = (msg_t *) malloc(sizeof(msg_t));
				if(new_msg == NULL){	//Unable to allocate memory
					return 2;
				}

				//Fill data
				new_msg->id = msg_id;
				strcpy(new_msg->content, content);
				strcpy(new_msg->sender, sender);
				new_msg->next_msg = NULL;

				//Store at the end
				if(head_msg == NULL){	//Check first insertion
					(temp_user->msg) = new_msg;
				}
				else{
					if(head_msg->next_msg == NULL){	//Check second insertion
						head_msg->next_msg = new_msg;
					}
					else{
						while(head_msg->next_msg != NULL){	//Check rest
							head_msg = head_msg->next_msg;
						}
						head_msg->next_msg = new_msg;
					}
				}

				//Check receiver status
				if(temp_user->status == OFF){
					printf("MESSAGE %d FROM %s TO %s STORED\n", msg_id, sender, receiver);
					return 0;
				}
				else{
					return send_pending_msg(head, receiver);
				}
			}

			temp_user = temp_user->next_usr;
		}

	}

	//Not registered users
	return 1;
}

/*
* @brief: Send all pending messages to the receiver one by one in order
* @param: Pointer to first element of the user list and the receiver name
* @return: 0 if success and 2 if error.
*/
int send_pending_msg(user_t *head, char *receiver)
{
	user_t *temp_user = head;
	msg_t *temp_msg;
	int mysock;
	char *op = "\0", *sender  = "\0", *msg_id  = "\0", *content  = "\0";

	//Search receiver in list
	while(temp_user != NULL && temp_user->status == ON){
		if(strcmp(temp_user->usr_name, receiver) == 0){
			temp_msg = temp_user->msg;
			mysock = temp_user->socket;

			while(temp_msg != NULL){

				//TODO: Connect to client in each message
				//Fill message data to send
				strcpy(op, "SEND_MESSAGE");
				strcpy(sender, temp_msg->sender);
				if(sprintf(msg_id, "%d", temp_msg->id) < 0) return 2;
				strcpy(content, temp_msg->content);

				//Send message
				if(write_line(mysock, op , strlen(op)) < 0) return 2;
				if(write_line(mysock, sender , strlen(sender)) < 0) return 2;
				if(write_line(mysock, msg_id , strlen(msg_id)) < 0) return 2;
				if(write_line(mysock, content , strlen(content)) < 0) return 2;

				printf("SEND MESSAGE %s FROM %s TO %s\n", msg_id, sender, receiver);

				//Next message
				temp_msg = temp_msg->next_msg;

				//Free sent message
				free(temp_user->msg);

				//TODO: Close the conection after each message and mark as disconnected
			}

		}
		temp_user = temp_user->next_usr;
	}

	return 0;
}

void printUsers(user_t *head){
	user_t *temp_user = head;
	int i;

	if(head != NULL){
		printf("**LIST OF USERS**\n");

		for(i=0;temp_user!=NULL;i++){
			printf("%d. User %s - Status %d", i, temp_user->usr_name, temp_user->status);
			temp_user = temp_user->next_usr;
		}
	}
	else{
		printf("Empty users list\n");
	}

}

void printMessages(user_t *head, char *name){

	user_t *temp_user = head;
	msg_t *temp_msg;
	int i;

	while(temp_user != NULL){

		if(strcmp(temp_user->usr_name, name) == 0){
			temp_msg = temp_user->msg;
			printf("\n**LIST OF MESSAGES OF %s**\n", name);

			for(i=0;temp_msg!=NULL;i++){
				printf("\n%d. MESSAGE %d FROM %s\n%s\n", i,temp_msg->id, temp_msg->sender, temp_msg->content);
				temp_msg = temp_msg->next_msg;
			}
			exit(0);
		}

	}
	printf("Empty message list\n");

}
/*
int init_usr(user_t *head)
{
	return -1;
}

int init_msg(user_t *head)
{
	return -1;
}*/

/*AUXILIARY FUNCTIONS*/
ssize_t read_line(int fd, void *buffer, int n) {
	int last_read;
	int total_read;
	char *buff;
	char c;

	if (n < 0 || !buffer) {
		errno = EINVAL;
		return -1;
	}

	buff = buffer;
	bzero(buff, strlen(buff));

	// We try to fill the buffer until (n - 1) to add in the end '\0'.
	for (total_read = 0; total_read < (n - 1); ++total_read) {
		last_read = read(fd, &c, 1); // Read 1 byte.

		if (last_read == -1) { // Checking read errors.
			if (errno == EINTR) continue; // interrupted -> restart read().
			else return -1;
		} else if (last_read == 0) { // We have reached EOF.
			break;
		} else {
			if (c == '\n' || c == '\0') break;
			*buff++ = c;
		}
	}

	*buff = '\0'; // Set the delimiter.
	return total_read;
}

ssize_t write_line(int fd, void *buffer, size_t n) {
	int last_write = 0;
	size_t length = n;
	char *buff;
	char nl = '\0';

	if (!buffer) {
		errno = EINVAL;
		return -1;
	}

	buff = (char *) buffer;

	while (length > 0 && last_write >= 0) {
		last_write = write(fd, buff, length);
		length -= (size_t ) last_write;
		buff += last_write;
	}

	if (n > 1) write(fd, &nl, 1);

	if (last_write < 0) return -1; // Error.
	else return 0; // Write done.
}

/*
reply = htonl(1);
reply_msg = (char*) &reply;
if(write(s_local , data, sizeof(int))<0){
	printf("[ERROR] Error sending reply\n");
}
*/

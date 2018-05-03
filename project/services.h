#ifndef _LIBRARY_SERVICES_H
#define _LIBRARY_SERVICES_H

#include <stdint.h>
#include <netinet/in.h>

#define MAX_BUF 256
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0


typedef struct message{
	int id;
	char content[MAX_BUF];
	char sender[MAX_BUF];
	struct message *next_msg;
} msg_t;

typedef struct user{
	char *usr_name;
	int status;	//0 not conected, 1 connected
	//struct in_addr ip; // IP to send messages.
	//int port; // Port to send messages.
	int socket;
	msg_t *msg;
	int last_msg;
	struct user *next_usr;
} user_t;


int userExists(user_t *head, char *name);
int register_usr(user_t **head_ref, char *name);
int remove_usr(user_t **head_ref, char *name);
int connect_usr(user_t *head, char *name, int socket);
int disconnect_usr(user_t *head, char *name);
//user_t *get_user(user_t *head, char *name);

int send_pending_msg(user_t *head, char *receiver);
int send_msg(user_t *head, int msg_id, char *sender, char *receiver, char *content);

/*DEBUGGING FUNCTIONS*/
void printUsers(user_t *head);
void printMessages(user_t *head, char *name);
//int init_msg(user_t *head); //Restore message list
//int init_usr(user_t *head);	//Restore user list

/*AUXILIARY FUNCTIONS*/
ssize_t write_line(int fd, void *buffer, size_t n);
ssize_t read_line(int fd, void *buffer, int n);



#endif

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
	struct in_addr ip; // IP to send messages.
	int port; // Port to send messages.
	msg_t *msg;
	int last_msg;
	struct user *next_usr;
} user_t;


int init_usr(user_t *head);	//Restore user list
int register_usr(user_t *head, char *name, struct in_addr ip, int port);
int remove_usr(user_t *head, char *name);
user_t get_user(user_t *head, char *name);
int userExists(user_t *head, char *name);

int init_msg(user_t *head); //Restore message list
int add_msg(user_t *head, char *sender, char *content);
int pop_msg(user_t *head);


#endif

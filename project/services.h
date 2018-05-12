#ifndef _LIBRARY_SERVICES_H
#define _LIBRARY_SERVICES_H

#include <stdint.h>
#include <netinet/in.h>

#define MAX_BUF 256
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
#define SERVER_NAME "90ba4f4e30c3bc345088ef67ce3864dd" //MD5 hashing of server name "/server"

/*SWITCH KEYWORDS*/
#define ERROR -1
#define REGISTER 1
#define UNREGISTER 2
#define CONNECT 3
#define DISCONNECT 4
#define SEND 5


typedef struct message{
	unsigned int id;
	char content[MAX_BUF];
	char sender[MAX_BUF];
	struct message *next_msg;
} msg_t;

typedef struct user{
	char usr_name[MAX_BUF];
	int status;	//0 not conected, 1 connected
	struct sockaddr_in socket;	//IP and port
	msg_t *msg;
	unsigned int last_msg;
	struct user *next_usr;
} user_t;

typedef struct {
	char *key;
	int val;
} opkey_t;

typedef struct{
	int client_sdesc;
	struct sockaddr_in client_saddr;
} targs_t;


/*SERVICES*/
int register_usr(user_t **head_ref, char *name);
int remove_usr(user_t **head_ref, char *name);
int connect_usr(user_t *head, char *name, struct sockaddr_in socket);
int disconnect_usr(user_t *head, char *name);

int send_pending_msg(user_t *head, char *receiver);
int store_msg(user_t *head, int msg_id, char *sender, char *receiver, char *content);

/*SEARCH FUNCTIONS*/
int userExists(user_t *head, char *name);
//int userConnected(user_t *head, char *name);
char * searchUserNameByIp(user_t *head, struct sockaddr_in socket_sender);

/*AUXILIARY FUNCTIONS*/
//ssize_t write_line(int fd, void *buffer, size_t n);
ssize_t read_line(int fd, void *buffer, int n);
int keyfromstring(char *key);
int comp_sockaddr(struct sockaddr_in sa, struct sockaddr_in sb);

/*DEBUGGING FUNCTIONS*/
void printUsers(user_t *head);
void printMessages(user_t *head, char *name);
//int init_msg(user_t *head); //Restore message list
//int init_usr(user_t *head);	//Restore user list

#endif

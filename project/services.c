#include <stdio.h>
#include <string.h> //strcpy, memcpy
#include "services.h"

int init_usr(user_t *head){
	return -1;
}

int register_usr(user_t *head, char *name, struct in_addr ip, int port){
	return -1;
}

int remove_usr(user_t *head, char *name){
	return -1;
}

user_t get_user(user_t *head, char *name){
	user_t usr;
	return usr;
}
int userExists(user_t *head, char *name){
	return -1;
}

int init_msg(user_t *head){
	return -1;
}

int add_msg(user_t *head, char *sender, char *content){
	return -1;
}

int pop_msg(user_t *head){
	return -1;
}

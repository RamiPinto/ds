#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcpy
#include <mqueue.h> //message queues
#include "keys.h"

#define MAXSIZE 256
#define TRUE 1
#define FALSE 0

struct request{
  int key;
  char value1[MAXSIZE];
  float value2;
  char q_name[MAXSIZE]; //client queue name where the server sends the reply
};

int main(int argc, char **argv){
  mqd_t q_server; //Server message queue
  mqd_t q_client; //CLient message queue

  struct request req;
  int res;
  struct mq_attr attr;

  attr.mq_maxmsg = 1;
  attr.mq_msgsize = sizeof(int);
  if((q_client = mq_open("/CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr))==-1){
  	perror("[ERROR CLIENT] Cannot create client message queue.");
	return 1;
  }

  if((q_server = mq_open("/SERVER", O_WRONLY))==-1){
  	perror("[ERROR CLIENT] Cannot create server message queue.");
	return 1;
  }

  /*Fill in request*/
  req.key=5;
  strcpy(req.value1, "value1 test");
  req.value2=(float)2.0;
  strcpy(req.q_name, "/CLIENT_ONE");
  printf("Request filled.\n");

  mq_send(q_server, (char *)&req, sizeof(struct request), 0);
  mq_receive(q_client, (char *)&res, sizeof(int), 0);
  printf("[CLIENT] Received message: %s\n",(char *)&res);

  mq_close(q_server);
  mq_close(q_client);
  mq_unlink("/CLIENT_ONE");

  return 0;
}

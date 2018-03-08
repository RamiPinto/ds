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
  if((q_client = mq_open("/CLIENT_ONE1", O_CREAT|O_RDWR, 0644, &attr))==-1){
  	perror("[ERROR CLIENT] Cannot create client message queue.");
	return 1;
  }
  printf("[CLIENT] Client queue created.\n");

  if((q_server = mq_open("/SERVER1", O_WRONLY))==-1){
  	perror("[ERROR CLIENT] Cannot open server message queue.");
	return 1;
  }
  printf("[CLIENT] Server queue open.\n");

  /*Fill in request*/
  req.key=5;
  strcpy(req.value1, "value1 test");
  req.value2=(float)2.0;
  strcpy(req.q_name, "/CLIENT_ONE1");
  printf("Request filled.\n");

  if((mq_send(q_server, (char *)&req, sizeof(struct request), 0))==-1){
  	perror("[ERROR] Error sending request.");
	return 1;
  }
  printf("[CLIENT] Request sent to client queue %s -- key: %d, value1= %s, value2= %f\n",req.q_name,req.key,req.value1,req.value2);
  if((mq_receive(q_client, (char *)&res, sizeof(int), 0))==-1){
  	perror("[ERROR] Error receiving server reply.");
	return 1;
  }
  printf("[CLIENT] Received message: %d\n",res);

  mq_close(q_server);
  mq_close(q_client);
  mq_unlink("/CLIENT_ONE1");

  return 0;
}

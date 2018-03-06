#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strcpy
#include <mqueue.h> //message queues
#include "keys.h"

#define MAXSIZE 256

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
  q_client = mq_open("CLIENT_ONE", O_CREAT|O_RDONLY, 0700, &attr);

  q_server = mq_open("ADD_SERVER", O_WRONLY);

  /*Fill in request*/
  req.key=5;
  strcpy(req.value1, "value1 test");
  req.value2=(float)2.0;
  strcpy(req.q_name, "CLIENT_ONE");

  mq_send(q_server, &req, sizeof(struct request), 0);
  mq_receive(q_client, &res, sizeof(int), 0);

  mq_close(q_server);
  mq_close(q_client);
  mq_unlink("CLIENT_ONE");

  return 0;
}

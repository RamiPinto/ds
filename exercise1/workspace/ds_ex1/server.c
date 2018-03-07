#include <stdio.h>
#include <stdlib.h> //atoi, randon, malloc, strtol
#include <pthread.h> //threads
#include <mqueue.h> //message queue
#include <string.h> //memcpy
#include <fcntl.h> // mq flags
#include <sys/stat.h> //mq modes
#include "keys.h" //struct message

#define MAXSIZE 256
#define TRUE 1
#define FALSE 0

/* mutex and condition variables for the message copy */
pthread_mutex_t mutex_msg;
int msg_not_copied = TRUE;  //TRUE =1
pthread_cond_t cond_msg;

struct request{
  int key;
  char value1[MAXSIZE];
  float value2;
  char q_name[MAXSIZE]; //client queue name where the server sends the reply
};

void* process_message(void *msg){
  struct request msg_local; //local message
  mqd_t q_client; //client queue
  int result;

  /*thread copies message to local message*/
  pthread_mutex_lock(&mutex_msg);
  memcpy((char *) &msg_local, (char *)&msg, sizeof(struct request));

  /*Wake up server*/
  msg_not_copied=FALSE; //False = 0

  pthread_cond_signal(&cond_msg);
  pthread_mutex_unlock(&mutex_msg);

  /*Execute client request and prepare reply*/
  result = msg_local.key;

  /*Return result to client by sending it to queue*/
  q_client = mq_open(msg_local.q_name, O_WRONLY);

  if(q_client == -1){
    perror("[SERVER ERROR] Cannot open client queue.");
  }
  else{
    mq_send(q_client, (char *)&result, sizeof(int), 0);
    mq_close(q_client);
  }
  pthread_exit(0);
}

int main(int argc, char **argv){

  mqd_t q_server; /*server queue*/
  struct request msg; /*message to receive*/
  struct mq_attr q_attr;  /*queue atributes*/
  pthread_t thid;
  pthread_attr_t t_attr;  /*thread atributes*/
  q_attr.mq_maxmsg = 20;
  q_attr.mq_msgsize = sizeof(struct request);

  if((q_server = mq_open("/SERVER",  O_CREAT|O_RDONLY,  0700,  &q_attr))==-1){
    perror("[SERVER ERROR]Can’t create server queue");
    return 1;
  }

  pthread_mutex_init(&mutex_msg, NULL);
  pthread_cond_init(&cond_msg, NULL);
  pthread_attr_init(&t_attr);

  /* thread atributes */
  pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
  while (TRUE){
    mq_receive(q_server, (char *)&msg, sizeof(struct request), 0);

    pthread_create(&thid, &t_attr, process_message, &msg);

    /*Critical section - wait for thread to copy message*/
    pthread_mutex_lock(&mutex_msg);
    while (msg_not_copied){
      pthread_cond_wait(&cond_msg, &mutex_msg);
    }
    msg_not_copied = TRUE;
    pthread_mutex_unlock(&mutex_msg);
  }

  return 0;
}

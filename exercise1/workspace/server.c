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

struct Node {
	struct request myreq;
	struct Node *next;
};

int s_init(struct Node** head_ref){
	
	struct Node* current = *head_ref;
	struct Node* next;

	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}

	*head_ref = NULL;
	return 0;
}

int s_set_value(struct Node **head_ref, struct request *req){
	
	//Allocate node
	struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));

	//insert data
	//memcpy((char *)new_node->myreq,(char *) req, sizeof(struct request));
	new_node->myreq=(*req);
	new_node->next = (*head_ref);
	(*head_ref)=new_node;

	return 0;
}

struct request* s_get_value(struct Node* node, int key){

	struct request* myrequest=NULL;
	
	*myrequest = node->myreq;
	
	while(node != NULL && myrequest->key != key){
		node = node->next;
		*myrequest = node->myreq;	
	}
	if(node==NULL){
		return NULL;
	}
	
	return myrequest;
}

int s_modify_value(struct Node* node, struct request *myrequest){

	struct request* node_req=(struct request*)malloc(sizeof(struct request));
	char * temp_v1;
	float temp_v2;

	*node_req = node->myreq;

	while(node != NULL && node_req->key != myrequest->key){
		node = node->next;
		*node_req = node->myreq;	
	}
	if(node==NULL){
		return -1;
	}

	temp_v1 = myrequest->value1;
	temp_v2= myrequest->value2;
	memcpy(node_req->value1,temp_v1,sizeof(temp_v1)+1);
	node_req->value2 = temp_v2;
	node->myreq = *node_req;
	free(node_req);
	return 0;
}


int s_delete_key(struct Node **head_ref, int key){

	struct Node* temp = *head_ref, *prev;
	struct request *myrequest = (struct request*) malloc(sizeof(struct request));
	
	*myrequest = temp->myreq;

	if(temp != NULL && myrequest->key == key){
		*head_ref = temp->next;
		free(temp);
		return 0;
	}
	
	while(temp != NULL && myrequest->key != key){
		prev = temp;
		temp = temp->next;
		*myrequest = temp->myreq;	
	}
	if(temp==NULL){
		return -1;
	}
	prev->next=temp->next;
	free(temp);
	free(myrequest);

	return 0;
}

int s_num_items(struct Node *node){
	int counter;

	for(counter=1;node!=NULL;counter++){
		node = node->next;
	}

	return counter;
}

void print_list(struct Node *node){
	
	struct request* current_req = (struct request*) malloc(sizeof(struct request));
	int i;

  	//memcpy((char *) node, (char *)&mynode, sizeof(struct Node));
  	//memcpy((char *) &current_req, (char *)&mynode.myreq, sizeof(struct request));
	*current_req = node->myreq;
	printf("Printing list:\n");
	for(i=0;node!= NULL;i++){
		printf("Element %d: key %d - value1 %s - value2 %f\n",i,current_req->key, current_req->value1, current_req->value2);
	node = node->next;
	}
	free(current_req);

}



void* process_message(void *msg){
  struct request msg_local; //local message
  mqd_t q_client; //client queue
  int result=0;
  struct Node* head = NULL;

  /*thread copies message to local message*/
  pthread_mutex_lock(&mutex_msg);
  memcpy((char *) &msg_local, (char *)msg, sizeof(struct request));

  /*Wake up server*/
  msg_not_copied=FALSE; //False = 0

  pthread_cond_signal(&cond_msg);
  pthread_mutex_unlock(&mutex_msg);

  /*Execute client request and prepare reply*/
  result = msg_local.key;
  /*TEST*/
  s_set_value(&head,&msg_local);
  print_list(head);
  printf("[SERVER] Server process is managing message with key %d.\n",result);
  
  /*Return result to client by sending it to queue*/
  if((q_client = mq_open(msg_local.q_name, O_WRONLY))==-1){
  perror("[SERVER ERROR] Cannot open client queue.");
	//return -1;
  }
  else{
    printf("[SERVER] Open client queue success. Sending reply...\n");
    mq_send(q_client, (char *)&result, sizeof(int), 0);
    mq_close(q_client);
  }

  printf("[SERVER] Request managed. Exiting...\n");
  pthread_exit(0);
}

int main(int argc, char **argv){

  mqd_t q_server; /*server queue*/
  char * server_name = "/SERVER1";
  int flags = O_RDWR | O_CREAT;
  //mode_t mode;

  struct request msg; /*message to receive*/

  struct mq_attr q_attr;  /*queue atributes*/
  q_attr.mq_flags=0;
  q_attr.mq_maxmsg = 10;
  q_attr.mq_msgsize = sizeof(struct request);
  q_attr.mq_curmsgs=0;

  pthread_t thid;
  pthread_attr_t t_attr;  /*thread atributes*/

  if((q_server = mq_open(server_name, flags, 0644, &q_attr))==-1){
    perror("[SERVER ERROR]Can’t create server queue");
    return 1;
  }

  pthread_mutex_init(&mutex_msg, NULL);
  pthread_cond_init(&cond_msg, NULL);
  pthread_attr_init(&t_attr);

  /* thread atributes */
  pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
  printf("[SERVER] Waiting requests...\n");
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

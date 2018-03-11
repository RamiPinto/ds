#include <stdio.h>
#include <string.h> //strcpy, memcpy
#include <mqueue.h> //messsage queues
#include "keys.h"


mqd_t q_server;
mqd_t q_client;
struct mq_attr attr;

int open_client(void){
	attr.mq_maxmsg = 1;
	attr.mq_msgsize = sizeof(struct request);

	if((q_client = mq_open("/CLIENT_ONE1", O_CREAT|O_RDWR, 0644, &attr))==-1){
		perror("[ERROR] Cannot open client message queue");
		return -1;
	}
	//printf("Client queue opened.\n");
	return 0;
}

int connect_server(void){

	if((q_server = mq_open("/SERVER1", O_WRONLY))==-1){
		perror("[ERROR] Cannot connect to server");
		return -1;
	}
	return 0;
}

int close_conection(void){
	if(mq_close(q_server)==-1) return -1;
	if(mq_close(q_client)==-1) return -1;
	if(mq_unlink("/CLIENT_ONE1")==-1) return -1;
	return 0;
}


int send(struct request * req){
	
	int result=0;
	result = mq_send(q_server, (char *)req, sizeof(struct request),0);	
	
	return result;
}

int receive(struct request * reply){
	int result;
	result = mq_receive(q_client, (char *)reply, sizeof(struct request),0);
	return result;
}


int init(){
	
	struct request req;
	struct request reply;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;

	//Fill request
	req.key=0;
	strcpy(req.value1, "init");
	req.value2 = (float)0.0;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 0;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1) return -1;
	
	if(close_conection()==-1) return -1;

	printf("Initialization of the system. Previous values have been deleted.\n");

	return 0;
}


int set_value(int key, char *value1, float value2){
	
	struct request req;
	struct request reply;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;

	//Fill request
	req.key=key;
	strcpy(req.value1, value1);
	req.value2 = (float) value2;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 1;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1) return -1;
	if(reply.fcode==1){
		printf("[ERROR] Repeated key. Cannot insert values\n");
		return -1;
	}

	if(close_conection()==-1) return -1;
	printf("Element with key %d inserted.\n", key);
	return 0;
}


int get_value(int key, char **value1, float *value2){

	struct request req;
	struct request reply;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;

	//Fill request
	req.key=key;
	strcpy(req.value1, "get");
	req.value2 = (float)0.0;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 2;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1){
	       	return -1;
	}
	else{
		*value1=reply.value1;
		*value2=reply.value2;
	}

	if(close_conection()==-1) return -1;
	printf("Values of the element with key %d obtained.\n", key);	
	return 0;
}


int modify_value(int key, char *value1, float *value2){
	
	struct request req;
	struct request reply;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;

	//Fill request
	req.key=key;
	strcpy(req.value1, value1);
	req.value2 = (float)*value2;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 3;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1) return -1;

	if(close_conection()==-1) return -1;
	printf("Element with key %d modified.\n", key);
	return 0;
}


int delete_key(int key){
	
	struct request req;
	struct request reply;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;

	//Fill request
	req.key=key;
	strcpy(req.value1, "delete");
	req.value2 = (float)0.0;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 4;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1) return -1;

	if(close_conection()==-1) return -1;
	printf("Element with key %d deleted.\n", key);
	return 0;
}


int num_items(){
		
	struct request req;
	struct request reply;
	int result;
	if(open_client()==-1)return -1;
	if(connect_server()==-1) return -1;
	//Fill request
	req.key=0;
	strcpy(req.value1, "count");
	req.value2 = (float)0.0;
	strcpy(req.q_name,"/CLIENT_ONE1");
	req.fcode = 5;

	if(send(&req)==-1) return -1;
	if(receive(&reply)==-1) return -1;
	if(reply.fcode==-1) return -1;
	
	result = reply.fcode;

	if(close_conection()==-1) return -1;
	printf("Number of itmes stored calculated.\n");
	return result;
}


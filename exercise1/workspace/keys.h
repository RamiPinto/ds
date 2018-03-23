#ifndef _LIBRARY_KEYS_H
#define _LIBRARY_KEYS_H

int init();
int set_value(int key, char *value1, float value2);
int get_value(int key, char **value1, float *value2);
int modify_value(int key, char *value1, float *value2);
int delete_key(int key);
int num_items();

struct request{
	int key;
	char value1[256];
	float value2;
	char q_name[256]; //client queue name where the server sends the reply

	//Code for the function:
	//Request: -1 Error - 0 init - 1 set_value - 2 get_value - 3 modify_value - 4 delete_key - 5 num_items
	//Reply: -1 Error - 0 success
	int fcode;
};

#endif

#include <stdio.h>
#include "keys.h"


int main(){

	int key1 = 1;
	int key2 = 2;
	char *value1 = "Hello world!";
	float value2 = 1.0;
	float new_value2 = 5.0;
	int items;

	init();
	set_value(key1, value1, value2);	//set value success
	set_value(key1, "Bye", new_value2);	//set value error: repeated key
	set_value(3,"three", (float) 3.0);	//set value success

	init();
	get_value(key2, &value1, &value2);	//get value error: empty list
	modify_value(key1, "Modified", &new_value2);	//modify error: empty list
	delete_key(key1);	//delete key error: empty list
	items = num_items();	//Number of items = 0
	printf("Number of items: %d\n",items);
	set_value(key1, value1, value2);
	set_value(key2, "Bye", (float) 2.0);
	get_value(key2, &value1, &value2);	//get value success
	printf("The obtained values were: value1 - %s and value2 - %f\n",value1, value2);
	get_value(8, &value1, &value2);	//get value error: no key

	modify_value(key1, "Modified", &new_value2);	//modify success
	modify_value(8, "Modified", &new_value2);	//modify error: no key
	get_value(key1, &value1, &value2);
	printf("The obtained values were: value1 - %s and value2 - %f\n",value1, value2);

	items = num_items();
	printf("Number of items: %d\n",items);
	delete_key(key1);	//delete key success
	delete_key(8);	//delete key error: no key
	items = num_items();
	printf("Number of items: %d\n",items);

	return 0;
}

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
	set_value(key1, value1, value2);
	set_value(key2, "Bye", new_value2);
	set_value(3,"three", (float) 3.0);
	
	init();
	set_value(key1, value1, value2);
	set_value(key2, "Bye", (float) 2.0);
	get_value(key2, &value1, &value2);
	printf("The obtained values are: value1 - %s and value2 - %f\n",value1, value2);
	
	modify_value(key1, "Modified", &new_value2);
	get_value(key1, &value1, &value2);
	printf("The obtained values are: value1 - %s and value2 - %f\n",value1, value2);
	
	items = num_items();
	printf("Number of items: %d\n",items);
	delete_key(key1);
	items = num_items();
	printf("Number of items: %d\n",items);

	return 0;
}

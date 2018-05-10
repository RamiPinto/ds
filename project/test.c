#include <stdio.h>
#include <stdlib.h>
#include "services.h"

user_t *usr_list;

int  main(int argc, char **argv)
{
	printUsers(usr_list);
	register_usr(&usr_list, "ramiro");
	register_usr(&usr_list, "ramiro");
	printUsers(usr_list);
	register_usr(&usr_list, "test1");
	printUsers(usr_list);

	return 0;
}

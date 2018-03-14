#include <stdio.h>
#include <stdlib.h>
#include "read_line.h"

#define MAX_LINE	256



int  main(int argc, char **argv)
{
	char buffer[MAX_LINE];
	int err = 0;
	int n;
	

	while (err != -1) {
		n = readLine(0, buffer, MAX_LINE);
		if (n!=-1)
			write(1, buffer, n);
	}

	exit(0);
}

	

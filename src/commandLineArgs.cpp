#include "commandLineArgs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT_SMTP 25

int CommandLineArgs::GetPort()
{
	for (int i = 1; i < argc; i++) {
		if ((argv[i][0] == '-') && (argv[i][1] == 'p')) {
			bool digits_only = true;
			i++;
			if (!argv[i])
				break;
			for (int j = 0; argv[i][j]; j++)
				if ((argv[i][j] > '9') || (argv[i][j] < '0'))
					digits_only = false;
			if (digits_only == true) {
				return atoi(argv[i]);
			} else {
				printf("Port is incorrect, default value will be used\n");
				return DEFAULT_PORT_SMTP;
			}
		}
	}
	printf("Port was not specified, default value will be used\n");
	return DEFAULT_PORT_SMTP;
}

char* CommandLineArgs::GetConfigLocation()
{
	for (int i = 1; i < argc; i++)
		if ((argv[i][0] == '-') && (argv[i][1] == 'c')) {
			if (!argv[i+1])
				return NULL;
			else {
				int len = strlen(argv[i+1]);
				char* path = new char[len+1];
				memcpy(path, argv[i+1], len+1);
				return path;
			}
		}
	return NULL;
}
#include "commandLineArgs.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

CommandLineArgs::~CommandLineArgs()
{
	free(config_path);
}

int CommandLineArgs::ProcessArgs()
{
	const char* flags = "-c:";
	int res;
	if (argc == 1)
		return -1;
	do {
		res = getopt(argc, argv, flags);
		switch(res) {
			case 'c':
				config_path = strdup(optarg);
				break;
			default:
				break;
		}
	} while (res != -1);
	return 0;
}


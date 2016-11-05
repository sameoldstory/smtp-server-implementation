#include "commandLineArgs.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

CommandLineArgs::~CommandLineArgs()
{
	free(config_path);
}

void CommandLineArgs::ProcessArgs()
{
	const char* flags = "-c:";
	int res;
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
}


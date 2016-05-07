#ifndef SERVER_COMMANDLINEARGS_H
#define SERVER_COMMANDLINEARGS_H

#include <stdio.h>
#include <stdlib.h>
#include "constantValues.h"

class CommandLineArgs {
	int argc;
	char** argv;
public:
	CommandLineArgs(int argc_, char** argv_): argc(argc_), argv(argv_) {};
	int GetPort();
};

#endif
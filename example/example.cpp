#include "server.h"
#include "commandLineArgs.h"
#include <stdio.h>

int main(int argc, char** argv) {
	CommandLineArgs cl_args(argc, argv);
	if (cl_args.ProcessArgs() == -1) {
		puts("Specify parameters: -c configname");
		return -1;
	}
	Server serv(cl_args.GetConfigPath());
	serv.Run();
	return 0;
}
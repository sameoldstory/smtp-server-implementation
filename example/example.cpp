#include "server.h"
#include "commandLineArgs.h"
#include <stdio.h>

int main(int argc, char** argv) {
	CommandLineArgs cl_args(argc, argv);
	if (cl_args.ProcessArgs() == -1) {
		puts("Specify parameters: -c configname");
		return -1;
	}
	ServerConfiguration config(cl_args.GetConfigPath());
	config.Configure();
	Server serv(config);
	serv.Run();
	return 0;
}
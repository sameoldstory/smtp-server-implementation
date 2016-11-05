#include "server.h"
#include "commandLineArgs.h"

int main(int argc, char** argv) {
	CommandLineArgs cl_args(argc, argv);
	cl_args.ProcessArgs();
	Server serv(cl_args.GetConfigPath());
	serv.Run();
	return 0;
}
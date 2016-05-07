#include "server.h"
#include "commandLineArgs.h"


int main(int argc, char** argv) {
	CommandLineArgs cl_args(argc, argv);
	Server serv(cl_args.GetPort());
	serv.Run();
	return 0;
}
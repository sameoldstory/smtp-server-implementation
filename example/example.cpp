#include "mainLoop.h"
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
	MainLoop main_loop(config, config.GetQueuePath(), config.GetServerName());
	main_loop.Prepare();
	main_loop.Run();
	return 0;
}
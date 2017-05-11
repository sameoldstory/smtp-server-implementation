#include "mainLoop.h"
#include "commandLineArgs.h"
#include "configuration.h"
#include "SMTPServer.h"
#include <stdio.h>

int main(int argc, char** argv) {
	CommandLineArgs cl_args(argc, argv);
	if (cl_args.ProcessArgs() == -1) {
		puts("Specify parameters: -c configname");
		return -1;
	}
	Configuration config(cl_args.GetConfigPath());
	config.Configure();
	SMTPServer smtp(config);
	MainLoop main_loop(smtp);
	main_loop.Init();
	main_loop.Run();
	return 0;
}
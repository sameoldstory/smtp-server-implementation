#ifndef SERVER_COMMANDLINEARGS_H
#define SERVER_COMMANDLINEARGS_H


class CommandLineArgs {
	int argc;
	char** argv;
	char* config_path;
public:
	CommandLineArgs(int argc_, char** argv_):
		argc(argc_), argv(argv_), config_path(0) {};
	int ProcessArgs();
	char* GetConfigPath() const {return config_path;}
	~CommandLineArgs();
};


#endif
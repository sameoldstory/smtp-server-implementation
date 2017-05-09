#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "server.h"
#include "queueManager.h"

class Configuration;

class MainLoop {
	Configuration& config;
	Server smtp_server;
	QueueManager queue_manager;
	struct timeval check_queue_t;
	struct timeval tm;
	void SetCheckTime();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
public:
	MainLoop(Configuration& _config, char* path, char* server_name);
	~MainLoop();
	void Prepare();
	void Run();
};

#endif
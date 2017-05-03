#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "server.h"
#include "queueManager.h"

class MainLoop {
	int listening_sock;
	int port;
	sockaddr_in address;
	Server smtp_server;
	QueueManager queue_manager;
	struct timeval check_queue_t;
	struct timeval tm;
	void SetCheckTime();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
	void CreateListeningSocket();
public:
	MainLoop(ServerConfiguration& _config, char* path, char* server_name);
	~MainLoop();
	void Prepare();
	void Run();
};

#endif
#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

class TCPServer;

#include <sys/time.h>

class MainLoop {
	TCPServer& server;
	struct timeval check_queue_t;
	struct timeval tm;
	void SetCheckTime();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
public:
	MainLoop(TCPServer& _server);
	~MainLoop();
	void Init();
	void Run();
};

#endif
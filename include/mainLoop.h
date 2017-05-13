#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

class TCPServer;
class EventHandler;

#include <sys/time.h>

class MainLoop {
	TCPServer* server;
	EventHandler* handler;
	struct timeval scheduled;
	struct timeval timeout;
	void SetTimer();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
public:
	MainLoop(TCPServer* _server, EventHandler* _handler);
	~MainLoop();
	void Init();
	void Run();
};

#endif
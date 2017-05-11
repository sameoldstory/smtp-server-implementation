#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

class SMTPServer;

#include <sys/time.h>

class MainLoop {
	SMTPServer& smtp_server;
	struct timeval check_queue_t;
	struct timeval tm;
	void SetCheckTime();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
public:
	MainLoop(SMTPServer& _server);
	~MainLoop();
	void Init();
	void Run();
};

#endif
#include "mainLoop.h"
#include "TCPSession.h"
#include "exceptions.h"
#include "configuration.h"
#include "TCPServer.h"
#include "eventHandler.h"
#include "timeval.h"
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void MainLoop::SetTimer()
{
	gettimeofday(&scheduled, NULL);
	scheduled.tv_sec += handler->GetTimeout();
	timeval_subtract_curr_t(&timeout, &scheduled);
}

MainLoop::MainLoop(TCPServer* _server, EventHandler* _handler):
	server(_server), handler(_handler)
{
	SetTimer();
}

void MainLoop::PrepareSetsForSelect(fd_set* read, fd_set* write) const
{
	memcpy(read, &(server->fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(server->fdsets.writefds), sizeof(fd_set));
}

void MainLoop::Init()
{
	try {
		server->Init();
	} catch(const char* s) {
		printf("%s\n", s);
	}
	catch(FatalException e) {
		printf("Fatal exception caught\n");
	}
	catch(ConfigError e) {
		printf("ConfigError\n");
	}
	catch(...){
		printf("Unknown error occured\n");
	}
}

void MainLoop::Run()
{
	fd_set readfds, writefds;
	struct timeval timeout = {handler->GetTimeout(), 0};
	int res;

	for(;;) {

		PrepareSetsForSelect(&readfds, &writefds);

		if (1 == timeval_subtract_curr_t(&timeout, &scheduled)) {
			handler->Run();
			SetTimer();
		}

		res = select(server->fdsets.max_fd + 1, &readfds, &writefds, NULL, &timeout);
		if (res == -1) {
			perror("select");
			return;
		}

		if (res == 0) {
			handler->Run();
			SetTimer();
		}

		server->Run(&readfds, &writefds);
	}
}

MainLoop::~MainLoop()
{

}

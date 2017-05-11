#include "mainLoop.h"
#include "TCPSession.h"
#include "exceptions.h"
#include "configuration.h"
#include "SMTPServer.h"
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void MainLoop::SetCheckTime()
{
	/*
	gettimeofday(&check_queue_t, NULL);
	check_queue_t.tv_sec += config.GetTimeout();
	timeval_subtract_curr_t(&check_queue_t, &tm);
	*/
}

MainLoop::MainLoop(SMTPServer& _server):
	smtp_server(_server)
{
	SetCheckTime();
}

void MainLoop::PrepareSetsForSelect(fd_set* read, fd_set* write) const
{
	memcpy(read, &(smtp_server.fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(smtp_server.fdsets.writefds), sizeof(fd_set));
}

void MainLoop::Init()
{
	try {
		smtp_server.Init();
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
	//struct timeval tm = {0, 0};
	int res;

	for(;;) {

		PrepareSetsForSelect(&readfds, &writefds);

		/*
		res = timeval_subtract_curr_t(&check_queue_t, &tm);
		printf("check queue, sec: %lu usec: %d\n", check_queue_t.tv_sec, check_queue_t.tv_usec);
		printf("sec: %lu usec: %d\n", tm.tv_sec, tm.tv_usec);
		if (res == 1) {
			// call QueueManager
			SetCheckTime();
		}

		res = select(fdsets.max_fd + 1, &readfds, &writefds, NULL, &tm);
		*/

		res = select(smtp_server.fdsets.max_fd + 1, &readfds, &writefds, NULL, NULL);
		if (res == -1) {
			perror("select");
			printf("sec: %lu usec: %d\n", tm.tv_sec, tm.tv_usec);
			return;
		}

		if (res == 0) {
			// call QueueManager
			SetCheckTime();
		}

		/*
		if (tcp_ptr) {
			tcp_ptr->ServeAsSMTPServerSession(queue_manager);
		}
		*/
		smtp_server.Run(&readfds, &writefds);
	}
}

MainLoop::~MainLoop()
{

}

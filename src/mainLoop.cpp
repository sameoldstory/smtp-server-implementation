#include "mainLoop.h"
#include "TCPSession.h"
#include "exceptions.h"
#include "configuration.h"
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

MainLoop::MainLoop(Configuration& _config, char* path, char* server_name):
	config(_config), smtp_server(_config.GetPort()), queue_manager(path, server_name, _config.mailbox_manager)
{
	SetCheckTime();
}

void MainLoop::PrepareSetsForSelect(fd_set* read, fd_set* write) const
{
	memcpy(read, &(smtp_server.fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(smtp_server.fdsets.writefds), sizeof(fd_set));
}

void MainLoop::Prepare()
{
	try {
		int sock = smtp_server.CreateListeningSocket();
		smtp_server.fdsets.AddListeningSock(sock);
		queue_manager.CreateMailQueueDir();

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
	sockaddr_in addr;
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

		if (smtp_server.HasIncomingConnection(&readfds)) {
			int fd = smtp_server.AcceptConnection(&addr);
			TCPSession* s = smtp_server.AddSession(&addr, fd);
			s->ServeAsSMTPServerSession(queue_manager);
		}

		smtp_server.IterateThroughSessions(readfds, writefds);
	}
}

MainLoop::~MainLoop()
{

}

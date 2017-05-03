#include "mainLoop.h"
#include "TCPSession.h"
#include "exceptions.h"
#include <sys/select.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define DEFAULT_BACKLOG 5

void MainLoop::SetCheckTime()
{
	/*
	gettimeofday(&check_queue_t, NULL);
	check_queue_t.tv_sec += config.GetTimeout();
	timeval_subtract_curr_t(&check_queue_t, &tm);
	*/
}

MainLoop::MainLoop(ServerConfiguration& _config, char* path, char* server_name):
	listening_sock(-1), port(-1), smtp_server(_config), queue_manager(path, server_name, _config.mailbox_manager)
{
	port = _config.GetPort();
	SetCheckTime();
}

void MainLoop::PrepareSetsForSelect(fd_set* read, fd_set* write) const
{
	memcpy(read, &(smtp_server.fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(smtp_server.fdsets.writefds), sizeof(fd_set));
}

void MainLoop::CreateListeningSocket()
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	listening_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_sock == -1) {
		perror("socket");
		throw FatalException();
	}
	int opt = 1;
	setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(listening_sock, (sockaddr*) &address, sizeof(address))) {
		perror("bind");
		throw FatalException();
	}
	if (listen(listening_sock, DEFAULT_BACKLOG) == -1) {
		perror("listen");
		throw FatalException();
	}
}

void MainLoop::Prepare()
{
	try {

		CreateListeningSocket();
		smtp_server.fdsets.AddListeningSock(listening_sock);
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

		if (FD_ISSET(listening_sock, &readfds)) {
			int fd = smtp_server.AcceptConnection(&addr, listening_sock);
			TCPSession* s = smtp_server.AddSession(&addr, fd);
			s->ServeAsSMTPServerSession(queue_manager);
		}

		smtp_server.IterateThroughSessions(readfds, writefds);
	}
}

MainLoop::~MainLoop()
{
	if (listening_sock != -1) {
		shutdown(listening_sock, 2);
		close(listening_sock);
	}
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "exceptions.h"
#include "server.h"
#include "TCPSession.h"

class SMTPServerSession;

#define DEFAULT_BACKLOG 5
#define MAX_SESSIONS 16

ReadyIndicators::ReadyIndicators()
{
	max_fd = 0;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
}

void ReadyIndicators::AddListeningSock(int sock)
{
	FD_SET(sock, &readfds);
	max_fd = sock;
}

void ReadyIndicators::AddSessionSock(int sock)
{
	FD_SET(sock, &readfds);
	FD_SET(sock, &writefds);
	if (sock > max_fd)
		max_fd = sock;
}

void ReadyIndicators::DeleteSessionSock(int sock)
{
	FD_CLR(sock, &readfds);
	FD_CLR(sock, &writefds);
}

void ReadyIndicators::ClearWritefds(int sock)
{
	FD_CLR(sock, &writefds);
}

void ReadyIndicators::SetWritefds(int sock)
{
	FD_SET(sock, &writefds);
}

Server::Server(char* conf_path): listening_sock(-1), port(-1),
	sessions(NULL), config(conf_path), fdsets()
{
	sessions = new TCPSession*[MAX_SESSIONS];
	for (int i = 0; i < MAX_SESSIONS; i++)
		sessions[i] = NULL;
}

void Server::CreateListeningSocket()
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

int Server::AcceptConnection(sockaddr_in* addr)
{
	socklen_t size = INET_ADDRSTRLEN;
	int fd = accept(listening_sock, (sockaddr*) addr, &size);
	if (fd == -1)
		throw "Accept Failed";
	return fd;
}

TCPSession* Server::AddSession(sockaddr_in* addr, int fd)
{
	int i;
	for (i = 0; sessions[i]; i++) {

	}
	if (i == MAX_SESSIONS) {
		// handle situation when number of sessions is exceeded
	} else {
		sessions[i] = new TCPSession(fd, *addr);
		fdsets.AddSessionSock(fd);
		puts("Session added");
	}
	return sessions[i];
}

void Server::DeleteSession(TCPSession** ptr)
{
	int fd = (*ptr)->GetSocketDesc();
	delete *ptr;
	*ptr = NULL;
	shutdown(fd, SHUT_RDWR);
	close(fd);
	fdsets.DeleteSessionSock(fd);
	puts("Session deleted");
}

void Server::PrepareSetsForSelect(fd_set* read, fd_set* write) const
{
	memcpy(read, &(fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(fdsets.writefds), sizeof(fd_set));
}

void Server::MainLoop()
{
	fd_set readfds, writefds;
	sockaddr_in addr;

	for(;;) {

		PrepareSetsForSelect(&readfds, &writefds);

		int res = select(fdsets.max_fd + 1, &readfds, &writefds, NULL, NULL);
		if (res < 1) {
			perror("select");
			return;
		}

		if (FD_ISSET(listening_sock, &readfds)) {
			int fd = AcceptConnection(&addr);
			TCPSession* s = AddSession(&addr, fd);
			s->ServeAsSMTPServerSession(&config);
		}

		for (int i = 0; i < MAX_SESSIONS; i++) {
			if (!sessions[i])
				continue;
			int fd = sessions[i]->GetSocketDesc();

			if (sessions[i]->NeedsToWrite() && FD_ISSET(fd, &writefds)) {
				sessions[i]->ProcessWriteOperation();
				fdsets.ClearWritefds(fd);
				if (sessions[i]->NeedsToBeClosed())
					DeleteSession(&(sessions[i]));
			}

			if (sessions[i] && FD_ISSET(fd, &readfds)) {
				sessions[i]->ProcessReadOperation();
				fdsets.SetWritefds(fd);
			}
		}
	}
}

void Server::ConfigureServer()
{
	if (!config.GetConfigPath()) {
		printf("Server can't be launched: specify path for configuration file with -c key\n");
		throw FatalException();
	}
	if (!config.OpenConfig()) {
		printf("Config file can not be opened\n");
		throw FatalException();
	}
	config.ExtractInfoFromConfig();
	config.CloseConfig();
}

void Server::CreateMailQueueDir()
{
	char* path = config.GetQueuePath();
	int res = mkdir(path, 0700);
	if (res == -1) {
		if (errno == EEXIST)
			puts("CreateMailQueueDir: already exists");
		else {
			puts("Could not create directory for Mail Queue");
			throw FatalException();
		}
	}
}

void Server::Run()
{
	try {
		ConfigureServer();
		port = config.GetPort();
		CreateListeningSocket();
		fdsets.AddListeningSock(listening_sock);
		// this server method should be moved to QueueManager later
		CreateMailQueueDir();
		MainLoop();

	} catch(const char* s) {
		printf("%s\n", s);
	}
	catch(FatalException e) {
		printf("Fatal exception caught\n");
	}
	catch(...){
		printf("Unknown error occured\n");
	}
}

void Server::EmptyAllocatedMemory()
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		delete sessions[i];
	delete[] sessions;
}

Server::~Server()
{
	EmptyAllocatedMemory();
	if (listening_sock != -1) {
		shutdown(listening_sock, 2);
		close(listening_sock);
	}
}





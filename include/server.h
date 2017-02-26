#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "serverConfiguration.h"
#include <arpa/inet.h>

class TCPSession;

struct ReadyIndicators {
	int max_fd;
	fd_set readfds;
	fd_set writefds;
	ReadyIndicators();
	void AddListeningSock(int sock);
	void AddSessionSock(int sock);
	void DeleteSessionSock(int sock);
	void ClearWritefds(int sock);
	void SetWritefds(int sock);
};

class Server {
	int listening_sock;
	int port;
	sockaddr_in address;
	TCPSession** sessions;
	ServerConfiguration config;
	ReadyIndicators fdsets;
	void ConfigureServer();
	void CreateListeningSocket();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
	void CreateMailQueueDir();
	void MainLoop();
	int AcceptConnection(sockaddr_in* cl_addr);
	TCPSession* AddSession(sockaddr_in* addr, int fd);
	void DeleteSession(TCPSession**);
	void EmptyAllocatedMemory();
public:
	Server(char* config_path);
	void Run();
	~Server();
};

#endif
#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "serverConfiguration.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include "queueManager.h"

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
	TCPSession** sessions;
	ServerConfiguration& config;
	int ConnectToHost(sockaddr_in* cl_addr, char* host);
	void DeleteSession(TCPSession**);
	void ProcessSession(TCPSession*& s_ptr, fd_set& readfds, fd_set& writefds);
public:
	ReadyIndicators fdsets;
	Server(ServerConfiguration& _config);
	int AcceptConnection(sockaddr_in* cl_addr, int sock);
	TCPSession* AddSession(sockaddr_in* addr, int fd);
	void IterateThroughSessions(fd_set& readfds, fd_set& writefds);
	~Server();
};

#endif
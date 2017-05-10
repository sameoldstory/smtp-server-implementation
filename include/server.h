#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

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
	int listening_sock;
	int port;
	sockaddr_in address;
	TCPSession** sessions;
	TCPSession* AddSession(sockaddr_in* addr, int fd);
	void DeleteSession(TCPSession**);
	void ProcessSession(TCPSession*& s_ptr, fd_set* readfds, fd_set* writefds);
	int AcceptConnection(sockaddr_in* cl_addr);
public:
	ReadyIndicators fdsets;
	int CreateListeningSocket();
	void IterateThroughActiveSessions(fd_set* readfds, fd_set* writefds);
	TCPSession* NewIncomingConnection(fd_set* readfds);
	Server(int _port);
	~Server();
};

#endif
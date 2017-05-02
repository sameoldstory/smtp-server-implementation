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
	int listening_sock;
	int port;
	sockaddr_in address;
	TCPSession** sessions;
	ServerConfiguration& config;
	ReadyIndicators fdsets;
	QueueManager queue_manager;
	struct timeval check_queue_t;
	struct timeval tm;
	void SetCheckTime();
	void CreateListeningSocket();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
	void ProcessSession(TCPSession*& s_ptr, fd_set& readfds, fd_set& writefds);
	void MainLoop();
	int AcceptConnection(sockaddr_in* cl_addr);
	int ConnectToHost(sockaddr_in* cl_addr, char* host);
	TCPSession* AddSession(sockaddr_in* addr, int fd);
	void DeleteSession(TCPSession**);
	void EmptyAllocatedMemory();
public:
	Server(ServerConfiguration& _config, char* queue_path, char* server_name);
	void Run();
	~Server();
};

#endif
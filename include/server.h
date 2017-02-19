#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include "serverConfiguration.h"
#include "SMTPServerSession.h"
#include <stdio.h>

#define BUF_SIZE_SERV 1024

class Client {
	int fd;
	sockaddr_in* cl_addr;
	char buf[BUF_SIZE_SERV];
	bool need_to_write;
	SMTPServerSession smtp;
public:
	Client(int fd_, sockaddr_in* cl_addr_,int sizebuf,ServerConfiguration* config_);
	short int ProcessReadOperation();
	short int ProcessWriteOperation();
	bool NeedsToWrite() const {return need_to_write;}
	void FulfillNeedToWrite() {need_to_write = false;}
	int GetSocketDesc() const {return fd;}
	char* GetIpString() const;
	char* GetHostname() const;
	bool NeedsToBeClosed() const {return smtp.LastMessage();}
	~Client();
};

struct ReadyIndicators {
	int max_fd;
	fd_set readfds;
	fd_set writefds;
	ReadyIndicators();
	void AddListeningSock(int sock);
	void AddClientSock(int sock);
	void DeleteClientSock(int sock);
	void ClearWritefds(int sock);
	void SetWritefds(int sock);
};

class Server {
	int listening_sock;
	int port;
	sockaddr_in address;
	Client** clients_array;
	ServerConfiguration config;
	ReadyIndicators fdsets;
	void ConfigureServer();
	void CreateListeningSocket();
	void PrepareSetsForSelect(fd_set* read, fd_set* write) const;
	void CreateMailQueueDir();
	void MainLoop();
	void AddClient();
	void DeleteClient(Client**);
	void EmptyAllocatedMemory();
public:
	Server(char* config_path);
	void Run();
	~Server();
};

#endif
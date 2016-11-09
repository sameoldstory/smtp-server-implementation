#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include "serverConfiguration.h"
#include "SMTPsession.h"

#define BUF_SIZE_SERV 1024

class Client {
	int fd;
	sockaddr_in* cl_addr;
	char buf [BUF_SIZE_SERV];
	bool need_to_write;
	SMTPsession smtp;
public:
	Client(int fd_, sockaddr_in* cl_addr_,int sizebuf):
		fd(fd_), cl_addr(cl_addr_), need_to_write(false), smtp(sizebuf) {};
	short int ProcessReadOperation();
	short int ProcessWriteOperation();
	bool NeedsToWrite() {return need_to_write;}
	void FulfillNeedToWrite() {need_to_write = false;}
	int GetSocketDesc() {return fd;}
	~Client() {delete cl_addr;}
};


class Server {
	int listening_sock;
	int port;
	sockaddr_in address;
	Client** clients_array;
	ServerConfiguration config;
	void ConfigureServer();
	void CreateListeningSocket();
	void MainLoop();
	void AddClient();
	void DeleteClient(int);
	void EmptyAllocatedMemory();
public:
	Server(char* config_path);
	void Run();
	~Server();
};

#endif
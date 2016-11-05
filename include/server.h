#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include "serverConfiguration.h"

class SMTPsessionList;

#if 0
class Client {
	int fd;
	sockaddr_in* client_addr;
	FileBuffer in_buf;
public:
};
#endif

class Server {
	int listening_sock;
	int port;
	sockaddr_in address;
	SMTPsessionList* clients;
	ServerConfiguration config;
	void ConfigureServer();
	void CreateListeningSocket();
	void ListeningModeOn();
	void AddClient(int, sockaddr_in*);
	void DeleteClient(int);
	void EmptyAllocatedMemory();
public:
	Server(char* config_path);
	void Run();
	~Server();
};

#endif
#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include "serverConfiguration.h"

class SMTPsessionList;

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
	Server(int port_, char* conf_addr);
	void Run();
	~Server();
};

#endif
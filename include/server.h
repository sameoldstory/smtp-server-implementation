#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>

class SMTPsessionList;

class Server {
	int listening_sock;
	int port;
	sockaddr_in address;
	SMTPsessionList* clients;
	void CreateListeningSocket();
	void ListeningModeOn();
	void AddClient(int, sockaddr_in*);
	void DeleteClient(int);
	void EmptyAllocatedMemory();
public:
	Server(int port_);
	void Run();
	~Server();
};

#endif
#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <arpa/inet.h>

#define BUF_SIZE_SERV 1024

class ServerConfiguration;

class TCPSessionDriver;

class QueueManager;

class TCPSession {
	int fd;
	sockaddr_in addr;
	char buf[BUF_SIZE_SERV];
	TCPSessionDriver* session_driver;
public:
	TCPSession(int fd_, sockaddr_in cl_addr_);
	~TCPSession();

	short int ProcessReadOperation();
	short int ProcessWriteOperation();
	bool NeedsToWrite() const;
	bool NeedsToBeClosed() const;
	void Serve(TCPSessionDriver* driver);

	int GetSocketDesc() const {return fd;}
	char* GetIpString() const;
	char* GetHostname() const;

	// this is temporary method
	TCPSessionDriver* GetSessionDriverPtr() {
		return session_driver;
	}
};

#endif
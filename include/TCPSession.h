#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include <arpa/inet.h>

#define BUF_SIZE_SERV 1024

class ServerConfiguration;

class TCPSessionDriver;

class TCPSession {
	int fd;
	sockaddr_in addr;
	char buf[BUF_SIZE_SERV];
	TCPSessionDriver* session_driver;
public:
	TCPSession(int fd_, sockaddr_in cl_addr_);
	short int ProcessReadOperation();
	short int ProcessWriteOperation();
	bool NeedsToWrite() const;
	int GetSocketDesc() const {return fd;}
	char* GetIpString() const;
	char* GetHostname() const;
	bool NeedsToBeClosed() const;
	void ServeAsSMTPServerSession(ServerConfiguration* config_);
	void ServeAsSMTPClientSession(char* ehlo, char* sender,
		char* rcpt, int _fd);
	// this is temporary method
	TCPSessionDriver* GetSessionDriverPtr() {
		return session_driver;
	}
	~TCPSession();
};

#endif
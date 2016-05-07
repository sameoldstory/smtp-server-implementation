#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"

struct sockaddr_in;

class SMTPsession {
	int fd;
	sockaddr_in* client_addr;
	SocketBuffer in_buf;
	enum {
		start
	} state;
public:
	SMTPsession(int fd_, sockaddr_in* addr): fd(fd_), client_addr(addr),
	 in_buf(fd_), state(start) {}
	int GetSocketDesc() const {return fd;}
	void Start() const;
	bool Resume();
	char* GetIpString(char* buf) const;
	~SMTPsession();
};

struct SMTPsessionList {
	SMTPsession session;
	SMTPsessionList* next;
	SMTPsessionList(int fd, sockaddr_in* addr, SMTPsessionList* next_ = 0):
	 session(fd, addr), next(next_) {};
};

#endif
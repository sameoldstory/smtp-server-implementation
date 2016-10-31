#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"

struct sockaddr_in;

class SMTPsession {
	int fd;
	bool auth;
	sockaddr_in* client_addr;
	FileBuffer in_buf;
	void ProcessCommand(char*);
	void ProcessHelo(char*);
	void ProcessMail(char*);
	void ProcessRcpt(char*);
	void ProcessData(char*);
	void ProcessQuit(char*);
	void ProcessRset(char*);
	enum {
		start
	} state;
public:
	SMTPsession(int fd_, sockaddr_in* addr): fd(fd_), auth(false), client_addr(addr),
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
#ifndef CLIENT_SMTPSESSION_H
#define CLIENT_SMTPSESSION_H

#include "sessionHierarchy.h"
#include "buffer.h"

#define CLIENT_SESSION_BUF_SIZE 1024

enum client_state {start, helo, mail, rcpt, data, data_message, quit, end};

class ServerConfiguration;

struct SessionArgs {
	char* ehlo;
	char* mail;
	char* rcpt;
	SessionArgs(char*, char*, char*);
	~SessionArgs();
};

class SMTPClientSession: public SMTPSession {
	ParseBuffer in_buf;
	SessionArgs args;
	int fd;
	client_state state;
	char* message;
	char next_msg[CLIENT_SESSION_BUF_SIZE];
	void ProcessResponse(char* str);
public:
	SMTPClientSession(int buf_size, char* ehlo, char* sender, char* rcpt, int _fd);
	char* GetMessage();
	bool SessionFinished() const;
	void EndSession();
	bool HandleInput(int portion, char* buf);
	~SMTPClientSession();
};

#endif
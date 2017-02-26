#ifndef CLIENT_SMTPSESSION_H
#define CLIENT_SMTPSESSION_H

class ServerConfiguration;

struct SessionArgs {
	char* ehlo;
	char* mail;
	char* rcpt;
	SessionArgs(char*, char*, char*);
	~SessionArgs();
};

class SMTPClientSession {
	SessionArgs args;
public:
	SMTPClientSession(char*, char*, char*);
	~SMTPClientSession();
};

#endif
#ifndef CLIENT_SMTPSESSION_H
#define CLIENT_SMTPSESSION_H

class ServerConfiguration;

struct SessionArgs {
	char* ehlo;
	char* mail;
	char* rcpt;
	~SessionArgs();
};

class SMTPClientSession {
	SessionArgs args;
	ServerConfiguration* config;
public:
	SMTPClientSession(ServerConfiguration*, char*, char*);
	~SMTPClientSession();
};

#endif
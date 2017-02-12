#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"

class ServerConfiguration;

class SMTPServerSession {
	ParseBuffer in_buf;
	ServerConfiguration* config;
	bool need_to_write;
	char* msg_for_client;
	char* client_domain;
	char* mail_from;
	char** recipients;
	int recipients_count;
	bool CorrectMail(char*) const;
	char* ExtractFromAngleBrackets(char*) const;
	void AddRecipient(char*);
	void ProcessCommand(char*);
	void ProcessEhlo(char*);
	void ProcessMail(char*);
	void ProcessRcpt(char*);
	void ProcessData(char*);
	void ProcessQuit(char*);
	void ProcessEmail();
	void ProcessUnknownCmd(char*);
	void PrintStringArgs() const;
	// name of state corresponds to the last received command
	// start state is the state before any command
	enum {
		start, helo, mail, rcpt, datastart, datafinish, quit
	} state;
public:
	SMTPServerSession(int buf_size, ServerConfiguration* config_);
	char* GetMessage();
	bool LastMessage() const {if (state == quit) return true; else return false;}
	void EndSession();
	bool HandleInput(int portion, char* buf);
	~SMTPServerSession();
};

#endif
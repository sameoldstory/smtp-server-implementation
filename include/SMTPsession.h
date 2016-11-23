#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"

class SMTPsession {
	bool auth;
	ParseBuffer in_buf;
	void ProcessCommand(char*);
	void ProcessHelo(char*);
	void ProcessMail(char*);
	void ProcessRcpt(char*);
	void ProcessData(char*);
	void ProcessQuit(char*);
	void ProcessRset(char*);
	// name of state corresponds to the last received command
	// start state is the state before any command
	enum {
		start
	} state;
public:
	SMTPsession(int buf_size): auth(false),
	 in_buf(buf_size), state(start) {}
	//void Start() const;
	//bool Resume();
	char* GetMessage();
	void EndSession();
	bool HandleInput(int portion, char* buf);
	~SMTPsession() {}
};

#endif
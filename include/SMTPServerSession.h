#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"
#include "sessionHierarchy.h"

class ServerConfiguration;

// TODO: Recipient should be replaced by Mailbox struct

struct Recipient {
	char* email;
	const char* opt;
	struct Recipient *next;
	Recipient(char*, const char*);
	~Recipient();
};

struct ServerSessionInfo {
	char* client_domain;
	char* mail_from;
	Recipient* recipients;
	void AddRecipient(char*, const char*);
	ServerSessionInfo();
	~ServerSessionInfo();
};

struct SenderInfo {
	char* host;
	char* ip_addr;
	SenderInfo(char*, char*);
	~SenderInfo();
};

//TODO: MessageSaver should be moved to queueManager

class MessageSaver {
	static int counter;
	SenderInfo sender_info;
	ServerSessionInfo* session_info;
	int msg_d;
	char* queue_path;
	char* filename;
	char* server_name;
	int OpenFile(const char*) const;
	void WriteTimeInfoToServiceFile(int) const;
	void WriteRecipientsInfoToServiceFile(int) const;
	void GenerateFileName();
	void GenerateServiceFile() const;
	void GenerateMessageFile();
	void AddReceiveLineToMessageFile() const;
	void AddFromLineToReceive(char*&) const;
	void AddByLineToReceive(char*&) const;
	void AddForLineToReceive(char*&) const;
	void AddDateLineToReceive(char*&, int) const;
public:
	MessageSaver(ServerSessionInfo*, const char*, char*, char*, char*);
	void PrepareForMsgSaving();
	void WriteLineToFile(char*);
	~MessageSaver();
};

// TODO: ServerConfiguration should be replaced by MailboxManager

class SMTPServerSession: public SMTPSession {
	ParseBuffer in_buf;
	ServerConfiguration* config;
	ServerSessionInfo session_info;
	MessageSaver msg_saver;
	bool need_to_write;
	char* msg_for_client;
	bool CorrectMail(char*) const;
	char* ExtractFromAngleBrackets(char*) const;
	void ProcessCommand(char*);
	void ProcessEhlo(char*);
	void ProcessMail(char*);
	void ProcessRcpt(char*);
	void ProcessData(char*);
	void ProcessQuit(char*);
	void ProcessEmail();
	void ProcessUnknownCmd(char*);
	enum {
		start, helo, mail, rcpt, datastart, datafinish, quit
	} state;
public:
	SMTPServerSession(int, ServerConfiguration*, char*, char*);
	char* GetMessage();
	bool SessionFinished() {if (state == quit) return true; else return false;}
	void EndSession();
	bool HandleInput(int portion, char* buf);
	~SMTPServerSession();
};

#endif
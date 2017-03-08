#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"
#include "sessionHierarchy.h"

class MailboxManager;
class ServerConfiguration;
struct Mailbox;

struct ServerSessionInfo {
	char* client_domain;
	char* mail_from;
	Mailbox* recipients;
	void AddRecipient(Mailbox* box);
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

class SMTPServerSession: public SMTPSession {
	ParseBuffer in_buf;
	MailboxManager* mailbox_manager;
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
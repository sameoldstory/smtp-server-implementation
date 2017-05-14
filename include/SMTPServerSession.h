#ifndef SERVER_SMTPSESSION_H
#define SERVER_SMTPSESSION_H

#include "buffer.h"
#include "sessionHierarchy.h"

class MailboxManager;
struct Mailbox;
class QueueProcessor;

struct SMTPSessionInfo {
	char* client_domain;
	char* mail_from;
	Mailbox* recipients;
	void AddRecipient(Mailbox* box);
	SMTPSessionInfo();
	~SMTPSessionInfo();
};

struct SenderInfo {
	char* host;
	char* ip_addr;
	SenderInfo(char*, char*);
	~SenderInfo();
};

class MessageSaver {
	QueueProcessor* queue_processor;
	SenderInfo sender_info;
	SMTPSessionInfo* session_info;
	int msg_d;
	char* filename;
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
	MessageSaver(QueueProcessor* _queue_processor, SMTPSessionInfo*, char*, char*);
	void PrepareForMsgSaving();
	void WriteLineToFile(const char*);
	Mailbox* GetMailbox(char* name);
	char* GetFilename() const {return filename;}
	void FinishSaving();
	~MessageSaver();
};

class SMTPServerSession: public SMTPSession {
	ParseBuffer in_buf;
	SMTPSessionInfo session_info;
	MessageSaver msg_saver;
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
	SMTPServerSession(QueueProcessor* _queue_processor, int, char*, char*);
	char* GetMessage();
	char* GetFilename() const {return msg_saver.GetFilename();}
	bool SessionFinished() const {if (state == quit) return true; else return false;}
	void EndSession();
	bool HandleInput(int portion, char* buf);
	~SMTPServerSession();
};

#endif
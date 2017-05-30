#ifndef SERVER_QUEUE_PROCESSOR_H
#define SERVER_QUEUE_PROCESSOR_H

class MailboxManager;

class SMTPServer;

typedef unsigned char u_char;

class QueueManager {

	friend class MessageSaver;

	SMTPServer* server;

	int counter;
	char* path;
	char* server_name;
	MailboxManager& mailboxes;

	bool OpenEnvAndMsgFiles(char* id, int& env, int& msg);

	void DeleteMessage(char* id);

	bool ForwardMessage(char* id, char* str, int fd_msg);
	bool DeliverToTheMailbox(char* id);

	char** GetMxRecords(char* domain);
public:

	QueueManager(char* _path, char* _server_name, MailboxManager& _manager);
	~QueueManager();

	void SMTPServerIsDead();

	void BindWithServer(SMTPServer* smtp) {server = smtp;};
	void CreateMailQueueDir();

	void ProcessSingleMessage(char* id);
	void ProcessQueue();
};

#endif
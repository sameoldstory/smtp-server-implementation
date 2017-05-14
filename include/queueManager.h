#ifndef SERVER_QUEUE_PROCESSOR_H
#define SERVER_QUEUE_PROCESSOR_H

class MailboxManager;

class SMTPServer;

class QueueManager {
	friend class MessageSaver;

	SMTPServer* server;

	int counter;
	char* path;
	char* server_name;
	MailboxManager& mailboxes;

public:

	QueueManager(char* _path, char* _server_name, MailboxManager& _manager);
	~QueueManager();

	void BindWithServer(SMTPServer* smtp) {server = smtp;};
	void CreateMailQueueDir();

	void ProcessSingleMessage(char* id);
	void ProcessQueue();
};

#endif
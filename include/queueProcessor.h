#ifndef SERVER_QUEUE_PROCESSOR_H
#define SERVER_QUEUE_PROCESSOR_H

#include "eventHandler.h"
class MailboxManager;

class QueueProcessor: public EventHandler {
	friend class MessageSaver;
	int timeout;

	int counter;
	char* path;
	char* server_name;
	MailboxManager& mailboxes;
public:
	QueueProcessor(int seconds, char* _path, char* _server_name, MailboxManager& _manager);
	virtual ~QueueProcessor();

	virtual void Run();
	virtual int GetTimeout();

	void CreateMailQueueDir();
	void ProcessMessage(char* id);
};

#endif
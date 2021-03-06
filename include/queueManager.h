#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

class MailboxManager;

class QueueManager {
friend class MessageSaver;
private:
	int counter;
	char* path;
	char* server_name;
	MailboxManager& mailboxes;
	void TrySendingMessage();
public:
	QueueManager(char* _queue_path, char* _server_name, MailboxManager& _manager);
	~QueueManager();

	void CreateMailQueueDir();
	void ProcessMessage(char* id);
	void GoThroughQueue();
};

#endif
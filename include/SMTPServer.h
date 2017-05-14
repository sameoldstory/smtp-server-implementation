#ifndef SERVER_SMTP_SERVER_H
#define SERVER_SMTP_SERVER_H

#include "TCPServer.h"

class Configuration;
class QueueManager;

class SMTPServer: public TCPServer
{
	Configuration* config;
	QueueManager* queue_manager;
protected:
	virtual TCPSession* NewConnection();
public:
	SMTPServer(Configuration* _config, QueueManager* _queue_manager);
	virtual ~SMTPServer();

	virtual void Init();
	virtual void HandleEvent();
	void NewClientSession(char* host, char* sender, char* rcpt, int fd);
};

#endif
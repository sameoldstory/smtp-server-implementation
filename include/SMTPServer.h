#ifndef SERVER_SMTP_SERVER_H
#define SERVER_SMTP_SERVER_H

#include "TCPServer.h"
#include "queueManager.h"

class Configuration;

class SMTPServer: public TCPServer
{
	Configuration* config;
	QueueManager queue_manager;
protected:
	virtual TCPSession* NewConnection();
public:
	SMTPServer(Configuration* _config);
	virtual void Init();
	virtual ~SMTPServer();
};

#endif
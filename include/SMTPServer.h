#ifndef SERVER_SMTP_SERVER_H
#define SERVER_SMTP_SERVER_H

#include "TCPServer.h"

class Configuration;
class QueueProcessor;

class SMTPServer: public TCPServer
{
	Configuration* config;
	QueueProcessor* queue_processor;
protected:
	virtual TCPSession* NewConnection();
public:
	SMTPServer(Configuration* _config, QueueProcessor* _queue_processor);
	virtual ~SMTPServer();

	virtual void Init();
	void NewClientSession(char* host, char* sender, char* rcpt, int fd);
};

#endif
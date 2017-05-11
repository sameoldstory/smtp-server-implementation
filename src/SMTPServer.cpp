#include "SMTPServer.h"
#include "TCPSession.h"
#include "configuration.h"

SMTPServer::SMTPServer(Configuration& _config): Server(_config.GetPort()), config(_config),
	queue_manager(_config.GetQueuePath(), _config.GetServerName(), _config.mailbox_manager)
{

}

TCPSession* SMTPServer::NewConnection()
{
	TCPSession* s = Server::NewConnection();
	if (s) {
		s->ServeAsSMTPServerSession(queue_manager);
	}
	return s;
}

void SMTPServer::Init()
{
	Server::Init();
	queue_manager.CreateMailQueueDir();
}

SMTPServer::~SMTPServer()
{

}

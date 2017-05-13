#include "SMTPServer.h"
#include "TCPSession.h"
#include "configuration.h"
#include "SMTPServerSession.h"

SMTPServer::SMTPServer(Configuration* _config): TCPServer(_config->GetPort()), config(_config),
	queue_manager(_config->GetQueuePath(), _config->GetServerName(), _config->mailbox_manager)
{

}

TCPSession* SMTPServer::NewConnection()
{
	TCPSession* tcp = TCPServer::NewConnection();
	if (tcp) {
		SMTPServerSession* driver = new SMTPServerSession(&queue_manager, BUF_SIZE_SERV, tcp->GetHostname(), tcp->GetIpString());
		tcp->Serve(driver);
	}
	return tcp;
}

void SMTPServer::Init()
{
	TCPServer::Init();
	queue_manager.CreateMailQueueDir();
}

SMTPServer::~SMTPServer()
{

}

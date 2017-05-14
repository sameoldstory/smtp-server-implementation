#include "SMTPServer.h"
#include "TCPSession.h"
#include "configuration.h"
#include "SMTPServerSession.h"
#include "SMTPClientSession.h"
#include "queueProcessor.h"

SMTPServer::SMTPServer(Configuration* _config, QueueProcessor* _queue_processor):
	TCPServer(_config->GetPort()), config(_config), queue_processor(_queue_processor)
{

}

TCPSession* SMTPServer::NewConnection()
{
	TCPSession* tcp = TCPServer::NewConnection();
	if (tcp) {
		SMTPServerSession* driver = new SMTPServerSession(queue_processor, BUF_SIZE_SERV, tcp->GetHostname(), tcp->GetIpString());
		tcp->Serve(driver);
	}
	return tcp;
}

void SMTPServer::NewClientSession(char* host, char* sender, char* rcpt, int fd)
{
	TCPSession* tcp = ConnectToRemoteHost(host);

	SMTPClientSession* driver = new SMTPClientSession(BUF_SIZE_SERV, config->GetServerName(), sender, rcpt, fd);
	tcp->Serve(driver);
}

void SMTPServer::Init()
{
	TCPServer::Init();
	queue_processor->CreateMailQueueDir();
}

SMTPServer::~SMTPServer()
{

}

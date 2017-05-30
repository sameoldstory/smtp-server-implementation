#include "SMTPServer.h"
#include "TCPSession.h"
#include "configuration.h"
#include "SMTPServerSession.h"
#include "SMTPClientSession.h"
#include "queueManager.h"
#include "exceptions.h"

SMTPServer::SMTPServer(Configuration* _config, QueueManager* _queue_manager):
	TCPServer(_config->GetPort()), config(_config), queue_manager(_queue_manager)
{

}

void SMTPServer::QueueManagerIsDead()
{
	queue_manager = NULL;
}

TCPSession* SMTPServer::NewConnection()
{
	TCPSession* tcp = TCPServer::NewConnection();
	if (tcp) {
		SMTPServerSession* driver = new SMTPServerSession(queue_manager, BUF_SIZE_SERV, tcp->GetHostname(), tcp->GetIpString());
		tcp->Serve(driver);
	}
	return tcp;
}

void SMTPServer::NewClientSession(char* host, char* sender, char* rcpt, int fd)
{
	TCPSession* tcp = ConnectToRemoteHost(host);

	SessionArgs* args = new SessionArgs(config->GetServerName(), sender, rcpt);
	SMTPClientSession* driver = new SMTPClientSession(BUF_SIZE_SERV, args, fd);
	tcp->Serve(driver);
}

void SMTPServer::HandleEvent()
{
	queue_manager->ProcessQueue();
}

void SMTPServer::Init()
{
	TCPServer::Init();
	if (!queue_manager) {
		throw FatalException();
	}
	queue_manager->CreateMailQueueDir();
}

SMTPServer::~SMTPServer()
{
	if (queue_manager) {
		queue_manager->SMTPServerIsDead();
	}
}

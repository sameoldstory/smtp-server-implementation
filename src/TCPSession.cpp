#include "TCPSession.h"
#include "SMTPServerSession.h"
#include "SMTPClientSession.h"
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

TCPSession::TCPSession(int fd_, sockaddr_in addr_):
	fd(fd_), addr(addr_), session_driver(NULL)
{

}

TCPSession::~TCPSession()
{
	delete session_driver;
}

char* TCPSession::GetIpString() const
{
	return inet_ntoa(addr.sin_addr);
}

char* TCPSession::GetHostname() const
{
	struct hostent* res =
		gethostbyaddr(&addr.sin_addr.s_addr, sizeof(unsigned long), AF_INET);
	return res->h_name;
}

bool TCPSession::NeedsToBeClosed() const
{
	return session_driver->SessionFinished();
}

bool TCPSession::NeedsToWrite() const
{
	return session_driver->NeedsToWrite();
}

//return values are just like in read system call

short int TCPSession::ProcessReadOperation()
{
	int portion = read(fd, buf, sizeof(buf)-1);
	if (portion == -1) {
		printf("Reading error: session %d\n", fd);
		return -1;
	}
	if (portion == 0) {
		printf("Session ended: session %d\n", fd);
		session_driver->EndSession();
		return 0;
	}
	buf[portion] = '\0';
	printf("Command: %s\n", buf);
	session_driver->HandleInput(portion, buf);
	return 1;
}

short int TCPSession::ProcessWriteOperation()
{
	char* message = session_driver->GetMessage();
	printf("Response for fd %d: %s\n", fd, message);
	return write(fd, message, strlen(message));
}

void TCPSession::ServeAsSMTPServerSession(ServerConfiguration* config_)
{
 	if (session_driver)
 		throw "TCPSession already provides some service";
 	session_driver = new
 		SMTPServerSession(sizeof(buf), config_, GetHostname(), GetIpString());
}

void TCPSession::ServeAsSMTPClientSession(char* ehlo, char* sender, char* rcpt,
	int _fd)
{
	if (session_driver)
 		throw "TCPSession already provides some service";
 	session_driver = new
 		SMTPClientSession(sizeof(buf), ehlo, sender, rcpt, _fd);
}

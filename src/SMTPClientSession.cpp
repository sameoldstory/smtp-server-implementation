#include "SMTPClientSession.h"
#include "serverConfiguration.h"
#include <string.h>
#include <stdlib.h>

SessionArgs::~SessionArgs()
{
	free(ehlo);
	free(mail);
	free(rcpt);
}

SessionArgs::SessionArgs(char* _ehlo, char* _sender, char* _rcpt)
{
	ehlo = strdup(_ehlo);
	mail = strdup(_sender);
	rcpt = strdup(_rcpt);
}

SMTPClientSession::SMTPClientSession(char* ehlo, char* sender, char* rcpt):
	args(ehlo, sender, rcpt)
{

}

SMTPClientSession::~SMTPClientSession()
{

}
#include "SMTPClientSession.h"
#include "serverConfiguration.h"
#include <string.h>
#include <stdlib.h>

SessionArgs::~SessionArgs()
{
	free(mail);
	free(rcpt);
}

SMTPClientSession::SMTPClientSession(ServerConfiguration* _config, char* sender, char* rcpt):
	config(_config)
{
	args.ehlo = config->GetServerName();
	args.mail = strdup(sender);
	args.rcpt = strdup(rcpt);
}

SMTPClientSession::~SMTPClientSession()
{

}
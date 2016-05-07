#include "SMTPsession.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#define SMTP_GREETING "220 my.mailserver.ru\r\n"

void SMTPsession::Start() const
{
	write(fd, SMTP_GREETING, strlen(SMTP_GREETING));
}

SMTPsession::~SMTPsession()
{
	delete client_addr;
}

//false if connection was closed by client

bool SMTPsession::Resume()
{
	if (0 == in_buf.Read())
		return false;
		//disconnection; something should be done about it
	char* str = in_buf.ExtractUntilCRLF();
	while(str) {
		printf("%s\n", str);
		str = in_buf.ExtractUntilCRLF();
	}
	return true;
}

char* SMTPsession::GetIpString(char* buf) const
{
	inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), buf, INET_ADDRSTRLEN);
	return buf;
}
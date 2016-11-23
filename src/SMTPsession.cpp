#include "SMTPsession.h"
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SMTP_GREETING "220 my.mailserver.ru\r\n"
#define MIN_CMD_LEN 4

#if 0
void SMTPsession::Start() const
{
	write(fd, SMTP_GREETING, strlen(SMTP_GREETING));
}

//false if connection was closed by client
// this function should delete str afterwards
bool SMTPsession::Resume()
{
	if (0 == in_buf.Read())
		return false;
		//disconnection;
	char* str = in_buf.ExtractUntilCRLF();
	while(str) {
		//printf("%s\n", str);
		ProcessCommand(str);
		// we need to take care of str
		delete[] str;
		str = in_buf.ExtractUntilCRLF();
	}
	return true;
}
#endif

char* SMTPsession::GetMessage()
{	
	//returned message should be string that ends with /0 
	//return "here will be message to client";
	return NULL;
}

void SMTPsession::EndSession()
{
	// 
}

bool SMTPsession::HandleInput(int portion, char* buf)
{	
	bool need_to_write = false;
	in_buf.EatData(portion, buf);
	char* str = strdup(in_buf.ExtractUntilCRLF());
	while(str) {
		//printf("%s\n", str);
		ProcessCommand(str);
		// we need to take care of str
		free(str);
		str = in_buf.ExtractUntilCRLF();
	}
	return need_to_write;
}

void SMTPsession::ProcessCommand(char* str)
{
	int cmdlen = MIN_CMD_LEN;
 	for (int i = 0; i < cmdlen; i++)
        str[i]=toupper(str[i]);
 	if (0 == strncmp(str,"HELO",cmdlen))
 		ProcessHelo(str+cmdlen);
 	if (0 == strncmp(str, "MAIL",cmdlen))
 		ProcessMail(str+cmdlen);
 	else if (0 == strncmp(str, "RCPT",cmdlen))
 		ProcessRcpt(str+cmdlen);
 	else if (0 == strncmp(str, "DATA",cmdlen))
 		ProcessData(str+cmdlen);
 	else if (0 == strncmp(str, "QUIT",cmdlen))
 		ProcessQuit(str+cmdlen);
 	else if (0 == strncmp(str, "RSET",cmdlen))
 		ProcessRset(str+cmdlen);
 	else {
 		//command unknown
 	}
}

void SMTPsession::ProcessHelo(char* str)
{
	printf("in ProcessHelo\n");
	printf("%s\n", str);
}

void SMTPsession::ProcessMail(char* str)
{
	printf("in ProcessMail\n");
	printf("%s\n", str);
}

void SMTPsession::ProcessRcpt(char* str)
{

}

void SMTPsession::ProcessData(char* str)
{

}

void SMTPsession::ProcessQuit(char* str)
{

}

void SMTPsession::ProcessRset(char* str)
{

}
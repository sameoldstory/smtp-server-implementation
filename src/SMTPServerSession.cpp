#include "SMTPServerSession.h"
#include "serverConfiguration.h"
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN_CMD_LEN 4
#define SMTP_GREETING "220 whatever.pomoyka.info service is ready\r\n"
#define SMTP_EHLO_ANSWER

SMTPServerSession::SMTPServerSession(int buf_size, ServerConfiguration* config_):
	in_buf(buf_size), config(config_), need_to_write(true), client_domain(NULL),
	mail_from(NULL), recipients(NULL)
{
	state = start;
	recipients_count = 0;
	msg_for_client = strdup(SMTP_GREETING);
	recipients = NULL;
}

SMTPServerSession::~SMTPServerSession()
{
	free(msg_for_client);
	free(client_domain);
	free(mail_from);

	if (recipients) {
		for(int i = 0; i < recipients_count; i++)
			free(recipients[i]);
		delete[] recipients;
	}
}

char* SMTPServerSession::GetMessage()
{
	need_to_write = false;
	return msg_for_client;
}

void SMTPServerSession::EndSession()
{
	//
}

bool SMTPServerSession::CorrectMail(char*)
{
	//here we check that argument of Mail From command is correct email address
	return true;
}

char* SMTPServerSession::ExtractFromAngleBrackets(char* str)
{
	int len = strlen(str);
	if (str[0] != '<' || str[len-1] != '>')
		return NULL;
	str[len-1] = '\0';
	return str + 1;
}

void SMTPServerSession::AddRecipient(char* str)
{
	//temporary stupid code
	recipients_count++;
	char** tmp = new char*[recipients_count];
	for (int i = 0; i < recipients_count-1; i++) {
		tmp[i] = recipients[i];
		free(recipients[i]);
	}
	tmp[recipients_count-1] = strdup(str);
	delete[] recipients;
	recipients = tmp;
}

bool SMTPServerSession::HandleInput(int portion, char* buf)
{
	in_buf.EatData(portion, buf);
	if (state != datastart) {
		need_to_write = true;
		char* str = in_buf.ExtractUntilCRLF();
		free(msg_for_client);
		ProcessCommand(str);
	} else {
		ProcessEmail();
	}
	return need_to_write;
}

void SMTPServerSession::ProcessCommand(char* str)
{
	int cmdlen = MIN_CMD_LEN;
 	for (int i = 0; i < cmdlen; i++)
        str[i]=toupper(str[i]);
 	if (0 == strncmp(str,"EHLO",cmdlen))
 		ProcessEhlo(str+cmdlen);
 	else if (0 == strncmp(str, "MAIL",cmdlen))
 		ProcessMail(str+cmdlen);
 	else if (0 == strncmp(str, "RCPT",cmdlen))
 		ProcessRcpt(str+cmdlen);
 	else if (0 == strncmp(str, "DATA",cmdlen))
 		ProcessData(str+cmdlen);
 	else if (0 == strncmp(str, "QUIT",cmdlen))
 		ProcessQuit(str+cmdlen);
 	else {
 		ProcessUnknownCmd(str);
 	}
}

void SMTPServerSession::ProcessUnknownCmd(char* str)
{
	msg_for_client = strdup("500 Command is not recognized\r\n");
}

void SMTPServerSession::ProcessEhlo(char* str)
{
	if (state != start) {
		msg_for_client = strdup("500 did not expect EHLO command\r\n");
		return;
	}
	char* word = in_buf.ExtractWordFromLine(str);
	if (!word) {
		msg_for_client = strdup("501 domain address is not specified\r\n");
	} else {
		msg_for_client = strdup("250 ok\r\n");
		client_domain = strdup(word);
		state = helo;
	}
}

void SMTPServerSession::ProcessMail(char* str)
{
	if (state != helo) {
		msg_for_client = strdup("500 MAIL: Bad sequence of commands\r\n");
		return;
	}
	char* word = in_buf.ExtractWordFromLine(str);
	if (!word || strncmp(word, "FROM:", MIN_CMD_LEN+1)) {
		msg_for_client = strdup("501 Wrong syntax for MAIL command\r\n");
		return;
	}
	word = word + strlen("FROM:");
	if (!strlen(word))
		word = in_buf.ExtractWordFromLine(str);
	word = ExtractFromAngleBrackets(word);
	if (!word) {
		msg_for_client = strdup("501 Wrong syntax for MAIL command\r\n");
		return;
	}
	if(CorrectMail(word)) {
		mail_from = strdup(word);
		state = mail;
		msg_for_client = strdup("250 ok\r\n");
	} else {
		msg_for_client = strdup("501 Email address is not valid\r\n");
	}
}

void SMTPServerSession::ProcessRcpt(char* str)
{
	if (state != mail && state != rcpt) {
		msg_for_client = strdup("503 RCPT: Bad sequence of commands\r\n");
		return;
	}
	char* word = in_buf.ExtractWordFromLine(str);
	if (!word || strncmp(word, "TO:", strlen("TO:"))) {
		msg_for_client = strdup("501 Wrong syntax for RCPT command\r\n");
		return;
	}
	word = word + strlen("TO:");
	if (!strlen(word))
		word = in_buf.ExtractWordFromLine(str);
	word = ExtractFromAngleBrackets(word);
	if (!word) {
		msg_for_client = strdup("501 Wrong syntax for RCPT command\r\n");
		return;
	}
	if (!config->MailboxLocal(word)) {
		msg_for_client = strdup("550 User not found\r\n");
	} else {
		state = rcpt;
		AddRecipient(word);
		msg_for_client = strdup("250 ok\r\n");
	}
}

void SMTPServerSession::ProcessData(char* str)
{
	if (state != rcpt) {
		msg_for_client = strdup("503 DATA: Bad sequence of commands\r\n");
		return;
	}
	state = datastart;
	msg_for_client = strdup("354 Start mail input, end with <CRLF>.<CRLF>\r\n");
}

void SMTPServerSession::ProcessEmail()
{
	char* str = in_buf.ExtractUntilCRLF();
	while(str) {
		if (!strcmp(str, ".")) {
			state = datafinish;
			need_to_write = true;
			free(msg_for_client);
			msg_for_client = strdup("250 ok\r\n");
			return;
		}
		str = in_buf.ExtractUntilCRLF();
	}
}

void SMTPServerSession::PrintStringArgs()
{
	puts("SMTPServerSession::PrintStringArgs");

	if (msg_for_client)
		puts(msg_for_client);
	else
		puts("no msg_for_client");

	if (client_domain)
		puts(client_domain);
	else
		puts("no client_domain");

	if (mail_from)
		puts(mail_from);
	else
		puts("no mail_from");

	puts("recipients");
	if (recipients) {
		for (int i = 0; i < recipients_count; i++)
			puts(recipients[i]);
	}
	puts("SMTPServerSession::PrintStringArgs getting out");
}

void SMTPServerSession::ProcessQuit(char* str)
{
	state = quit;
	msg_for_client = strdup("221 Closing connection\r\n");
	//PrintStringArgs();
}


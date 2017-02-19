#include "SMTPServerSession.h"
#include "serverConfiguration.h"
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>

#define MIN_CMD_LEN 4
#define SMTP_GREETING "220 whatever.pomoyka.info service is ready\r\n"
#define TEMP_BUF_SIZE 256
#define RECEIVED_HEADER_BUF_SIZE 2048
#define TIME_STEP 60
#define DEFAULT_DELIVERY_STATUS "new"

int MessageSaver::counter = 0;

SenderInfo::SenderInfo(char* _host, char* _ip)
{
	host = strdup(_host);
	ip_addr = strdup(_ip);
}

SenderInfo::~SenderInfo()
{
	free(host);
	free(ip_addr);
}

ServerSessionInfo::ServerSessionInfo(): client_domain(NULL),
	mail_from(NULL), recipients(NULL), recipients_count(0)
{

}

ServerSessionInfo::~ServerSessionInfo()
{
	free(client_domain);
	free(mail_from);
	if (recipients) {
	for(int i = 0; i < recipients_count; i++)
		free(recipients[i]);
		delete[] recipients;
	}
}

void ServerSessionInfo::AddRecipient(char* new_recipient)
{
	//temporary stupid code
	recipients_count++;
	char** tmp = new char*[recipients_count+1];
	for (int i = 0; i < recipients_count-1; i++)
		tmp[i] = recipients[i];
	tmp[recipients_count-1] = strdup(new_recipient);
	tmp[recipients_count] = NULL;
	delete[] recipients;
	recipients = tmp;
}

MessageSaver::MessageSaver(ServerSessionInfo* _info, const char* _path, char* host, char* ip, char* serv_name):
	sender_info(host, ip), session_info(_info), msg_d(-1), filename(0), server_name(serv_name)
{
	queue_path = strdup(_path);
	counter++;
}

MessageSaver::~MessageSaver()
{
	free(filename);
	free(queue_path);
	close(msg_d);
}

void MessageSaver::GenerateFileName()
{
	char buf[TEMP_BUF_SIZE];
	sprintf(buf, "%lu_%d", time(NULL), counter);
	int len = strlen(buf)+1;
	free(filename);
	filename = (char*)malloc(len);
	memcpy(filename, buf, len);
}

int MessageSaver::OpenFile(const char* extension) const
{
	char buf[TEMP_BUF_SIZE];
	sprintf(buf, "%s%s.%s", queue_path, filename, extension);
	int fd = open(buf, O_CREAT|O_WRONLY);
	if (fd == -1)
		throw "MessageSaver::OpenFile: could not open file";
	return fd;
}

void MessageSaver::WriteTimeInfoToServiceFile(int fd) const
{
	char buf[TEMP_BUF_SIZE];
	sprintf(buf, "%lu\n", time(NULL)/TIME_STEP);
	write(fd, buf, strlen(buf));
}

void MessageSaver::WriteRecipientsInfoToServiceFile(int fd) const
{
	char buf[TEMP_BUF_SIZE];
	for (int i = 0; session_info->recipients[i]; i++) {
		sprintf(buf, "%s %s\n", session_info->recipients[i], DEFAULT_DELIVERY_STATUS);
		write(fd, buf, strlen(buf));
	}
}

void MessageSaver::GenerateServiceFile() const
{
	int fd = OpenFile("env");
	WriteTimeInfoToServiceFile(fd);
	WriteRecipientsInfoToServiceFile(fd);
	close(fd);
}

void MessageSaver::GenerateMessageFile()
{
	msg_d = OpenFile("msg");
	AddReceiveLineToMessageFile();
}

void MessageSaver::AddFromLineToReceive(char* & buf) const
{
	sprintf(buf, "Received: from %s (%s [%s])\n",
	session_info->client_domain,
	sender_info.host,
	sender_info.ip_addr);
	buf += strlen(buf);
}

void MessageSaver::AddByLineToReceive(char* & buf) const
{
	sprintf(buf, "by %s (Ceres) with ESMTP id %d",
		server_name, counter);
	buf += strlen(buf);
}

void MessageSaver::AddForLineToReceive(char* & buf) const
{
	sprintf(buf, "\nfor <%s>", session_info->recipients[0]);
	buf += strlen(buf);
}

void MessageSaver::AddDateLineToReceive(char* & buf, int size) const
{
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(buf, size, "%a, %d %b %G %T %z (%Z)\n", t);
}

void MessageSaver::AddReceiveLineToMessageFile() const
{
	char buf[RECEIVED_HEADER_BUF_SIZE];
	char* tmp = buf;
	AddFromLineToReceive(tmp);
	AddByLineToReceive(tmp);
	if (session_info->recipients_count == 1)
		AddForLineToReceive(tmp);
	sprintf(tmp, ";\n");
	tmp += strlen(";\n");
	AddDateLineToReceive(tmp, sizeof(buf)-strlen(tmp)-1);
	write(msg_d, buf, strlen(buf));
}

void MessageSaver::PrepareForMsgSaving()
{
	GenerateFileName();
	GenerateServiceFile();
	GenerateMessageFile();
}

void MessageSaver::WriteLineToFile(char* str)
{
	int len = strlen(str);
	str[len] = '\n';
	int err = write(msg_d, str, len+1);
	if (err == -1)
		perror("MessageSaver::WriteLineToFile");
}

SMTPServerSession::SMTPServerSession(int buf_size, ServerConfiguration* config_, char* host, char* ip):
	in_buf(buf_size), config(config_), session_info(),
	msg_saver(&session_info, config_->GetQueuePath(), host, ip, config->GetServerName()),
	need_to_write(true)
{
	state = start;
	msg_for_client = strdup(SMTP_GREETING);
}

SMTPServerSession::~SMTPServerSession()
{
	free(msg_for_client);
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

bool SMTPServerSession::CorrectMail(char*) const
{
	//here we check that argument of Mail From command is correct email address
	return true;
}

char* SMTPServerSession::ExtractFromAngleBrackets(char* str) const
{
	int len = strlen(str);
	if (str[0] != '<' || str[len-1] != '>')
		return NULL;
	str[len-1] = '\0';
	return str + 1;
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
		session_info.client_domain = strdup(word);
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
		session_info.mail_from = strdup(word);
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
		session_info.AddRecipient(word);
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
	msg_saver.PrepareForMsgSaving();
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
			msg_saver.WriteLineToFile(str);
			return;
		}
		msg_saver.WriteLineToFile(str);
		str = in_buf.ExtractUntilCRLF();
	}
}

void SMTPServerSession::ProcessQuit(char* str)
{
	state = quit;
	msg_for_client = strdup("221 Closing connection\r\n");
}


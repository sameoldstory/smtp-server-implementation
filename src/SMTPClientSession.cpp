#include "SMTPClientSession.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

/*
QueueManager will open .env and .msg files
extract information: .env(type, mail_from, rcpt_to), .msg(data)
new forward jhon@whatever.pomoyka.info jhon@anywhere.com
*/

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

SMTPClientSession::SMTPClientSession(int buf_size, char* ehlo, char* sender,
	char* rcpt, int _fd): SMTPSession(false), in_buf(CLIENT_SESSION_BUF_SIZE), args(ehlo, sender, rcpt), fd(_fd),
 	state(start), message(NULL)
{
}

SMTPClientSession::~SMTPClientSession()
{
	free(message);
}

//TODO somewhere in SMTPServerSession I write zero bytes to file, needs fix
char* SMTPClientSession::GetMessage()
{
	char buf[1024];
	need_to_write = false;
	free(message);
	int res;
	switch(state) {
	case start:
	case end:
		throw "SMTPClientSession: should not get here\n";
	case helo:
		sprintf(buf, "EHLO %s\r\n", args.ehlo);
		message = strdup(buf);
		break;
	case mail:
		sprintf(buf, "MAIL FROM:<%s>\r\n", args.mail);
		message = strdup(buf);
		break;
	case rcpt:
		sprintf(buf, "RCPT TO:<%s>\r\n", args.rcpt);
		message = strdup(buf);
		break;
	case data:
		message = strdup("DATA\r\n");
		res = read(fd, next_msg, sizeof(next_msg)-1);
		if (res <= 0) {
			state = quit;
			GetMessage();
		}
		next_msg[res] = '\0';
		break;
	case data_message:
		message = strdup(next_msg);
		res = read(fd, next_msg, sizeof(next_msg)-1);
		if (res == -1) {
			state = quit;
			GetMessage();
		}
		if (res != 0) {
			next_msg[res] = '\0';
			need_to_write = true;
		}
		break;
	case quit:
		message = strdup("QUIT\r\n");
		state = end;
		break;
	}
	return message;
}

bool SMTPClientSession::SessionFinished() const
{
	if (state == end)
		return true;
	else
		return false;
}

void SMTPClientSession::EndSession()
{

}

bool SMTPClientSession::HandleInput(int portion, char* buf)
{
	in_buf.EatData(portion, buf);
	char* str = in_buf.ExtractUntilCRLF();
	if (!str) {
		return false;
	}
	need_to_write = true;
	ProcessResponse(str);
	return need_to_write;
}

// opening and closing files is done by queue manager

void SMTPClientSession::ProcessResponse(char* str)
{
	if (state == data) {
		if (strncmp(str, "354", sizeof("354"))) {
			int int_state = int(state);
			state = (client_state)++int_state;
			return;
		}
	}
	if (state == helo) {
		bool err = false;
		while(str) {
			if (str[0] != '2')
				err = true;
			str = in_buf.ExtractUntilCRLF();
		}
		if (err == true)
			state = quit;
		else
			state = mail;
		return;
	}
	if (str[0] == '2') {
		int int_state = int(state);
		state = (client_state)++int_state;
		return;
	} else {
		state = quit;
		return;
	}
}

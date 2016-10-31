#include "serverConfiguration.h"
#include "buffer.h"
#include "exceptions.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

ServerConfiguration::ServerConfiguration(char* config_path_):
	config_path(config_path_), fd(-1), buf(NULL), server(NULL), domain(NULL),
	mailboxes(NULL)
{}

ServerConfiguration::~ServerConfiguration()
{
	MailboxList* tmp;
	while(mailboxes) {
		tmp = mailboxes->next;
		delete mailboxes;
		mailboxes = tmp;
	}
	if (config_path)
		delete[] config_path;
	if (server)
		delete[] server;
	if (domain)
		delete[] domain;
	if (buf)
		delete buf;
}

bool ServerConfiguration::OpenConfig()
{
	fd = open(config_path, O_RDONLY);
	if (fd == -1) {
		perror(config_path);
		printf("Unable to open config file\n");
		return false;
	}
	return true;
}

bool ServerConfiguration::CloseConfig()
{
	if (fd == -1)
		return false;
	if (close(fd) == -1)
		return false;
	else
		return true;
}

bool ServerConfiguration::InitializeBuffer()
{
	if (fd == -1)
		return false;
	buf = new FileBuffer(fd);
	return true;
}

void ServerConfiguration::AddMailboxToList(MailboxList* m)
{
	if (!mailboxes)
		mailboxes = m;
	else {
		m->next = mailboxes;
		mailboxes = m;
	}
}

void ServerConfiguration::PrintMailboxes()
{
	MailboxList* tmp = mailboxes;
	while(tmp) {
		printf("%s\n", tmp->box.name);
		for (int i; i < tmp->box.param_numb; i++)
			printf("%s\n", tmp->box.params[i]);
		printf("---\n");
		tmp = tmp->next;
	}
}

MailboxList* ServerConfiguration::CreateMailbox(char* name, char* opt_line)
{
	char** params  = NULL, * word = buf->ExtractWordFromLine(opt_line);
	mail_option opt;
	int numb_opt = 0;
	char tmp_buf[1024];
	if (!word) {
		sprintf(tmp_buf, "Option [deliver/forward/trap] wasn't specified for mailbox: %s",
		name);
		delete[] name;
		throw ConfigError(tmp_buf);
	} else {
		if (!strcmp("deliver", word)) {
			opt = deliver;
		} else if (!strcmp("trap", word)) {
			opt = trap;
		} else if (!strcmp("forward", word)) {
			opt = forward;
		} else {
			sprintf(tmp_buf, "Unknown option \'%s\' for mailbox %s", word, name);
			delete[] name;
			delete[] word;
			throw ConfigError(tmp_buf);
		}
		delete[] word;
	}
	word = buf->ExtractWordFromLine(opt_line);
	while (word) {
		if (numb_opt == 0) {
			params  = new char*[1];
			params [0] = word;
		} else {
			char** tmp = new char*[numb_opt+1];
			for (int i = 0; i < numb_opt; i++)
				tmp[i] = params [i];
			tmp[numb_opt] = word;
			delete[] params ;
			params  = tmp;
		}
		numb_opt++;
		word = buf->ExtractWordFromLine(opt_line);
	}
	MailboxList* new_box = new MailboxList(name, opt, params, numb_opt);
	return new_box;
}

void ServerConfiguration::ExtractInfoFromConfig()
{
	bool mailboxes = false;
	while (buf->Read()) {
		char* line = buf->ExtractUntilEOL();
		while(line) {
			//comments and empty lines are ignored
			if ((line[0] == '#') || (line[0] == '\0')) {}
			else {
				char* word = buf->ExtractWordFromLine(line);
				if (word && !strcmp(word, "server_name")) {
					delete[] word;
					word = buf->ExtractWordFromLine(line);
					server = word;
				} else if (word && !strcmp(word, "domain_name")) {
					delete[] word;
					word = buf->ExtractWordFromLine(line);
					domain = word;
				} else if (word && !strcmp(word, "mailboxes_start")) {
					delete[] word;
					mailboxes = true;
				} else if (word && !strcmp(word, "mailboxes_end")) {
					delete[	] word;
					mailboxes = false;
				} else if (mailboxes && word) {
					try {
						AddMailboxToList(CreateMailbox(word, line));
					} catch (ConfigError e) {
						e.Print();
					}
				}
			}
			if (line)
				delete[] line;
			line = buf->ExtractUntilEOL();
		}
	}
}

Mailbox::~Mailbox()
{
	if (name)
		delete[] name;
	if (params) {
		for (int i = 0; i <= param_numb; i++)
			delete[] params[i];
	}
	delete[] params;
}


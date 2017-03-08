#include "serverConfiguration.h"
#include "buffer.h"
#include "exceptions.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_BUF_SIZE 1024

Mailbox::Mailbox(const Mailbox& box): name(strdup(box.name)),
	option(box.option), param_numb(box.param_numb), next(NULL)
{
	params = new char*[param_numb];
	for (int i = 0; i < param_numb; i++)
		params[i] = strdup(box.params[i]);
}

Mailbox::~Mailbox()
{
	free(name);
	if (params) {
		for (int i = 0; i < param_numb; i++)
			delete[] params[i];
	}
	delete[] params;
}

const char* Mailbox::GetOptionAsString() const
{
	static const char* options[] = {"forward", "deliver", "trap"};
	return options[option];
}

MailboxManager::MailboxManager(): mailboxes(NULL)
{

}

MailboxManager::~MailboxManager()
{
	Mailbox* tmp;
	while(mailboxes) {
		tmp = mailboxes->next;
		delete mailboxes;
		mailboxes = tmp;
	}
}

void MailboxManager::PrintMailboxes() const
{
	Mailbox* tmp = mailboxes;
	while(tmp) {
		printf("%s\n", tmp->name);
		for (int i = 0; i < tmp->param_numb; i++)
			printf("%s\n", tmp->params[i]);
		printf("---\n");
		tmp = tmp->next;
	}
}

Mailbox* MailboxManager::GetMailbox(char* box_name) const
{
	Mailbox* tmp = mailboxes;
	while(tmp) {
		if (!strcmp(tmp->name, box_name))
			return tmp;
		tmp = tmp->next;
	}
	return NULL;
}

// TODO: refactor MailboxManager::AddMailbox

void MailboxManager::AddMailbox(char* name, char* opt_line, ParseBuffer& buf)
{
	char** params = NULL, *tmp_word;
	mail_option opt;
	int numb_opt = 0;
	char tmp_buf[1024];
	tmp_word = buf.ExtractWordFromLine(opt_line);
	if (!tmp_word) {
		sprintf(tmp_buf, "Option [deliver/forward/trap] wasn't specified for mailbox: %s",
		name);
		delete[] name;
		throw ConfigError(tmp_buf);
	} else {
		if (!strcmp("deliver", tmp_word)) {
			opt = deliver;
		} else if (!strcmp("trap", tmp_word)) {
			opt = trap;
		} else if (!strcmp("forward", tmp_word)) {
			opt = forward;
		} else {
			sprintf(tmp_buf, "Unknown option \'%s\' for mailbox %s", tmp_word, name);
			delete[] name;
			throw ConfigError(tmp_buf);
		}
	}
	tmp_word = buf.ExtractWordFromLine(opt_line);
	while (tmp_word) {
		if (numb_opt == 0) {
			params  = new char*[1];
			params[0] = strdup(tmp_word);
		} else {
			char** tmp = new char*[numb_opt+1];
			for (int i = 0; i < numb_opt; i++)
				tmp[i] = params[i];
			tmp[numb_opt] = strdup(tmp_word);
			delete[] params;
			params  = tmp;
		}
		numb_opt++;
		tmp_word = buf.ExtractWordFromLine(opt_line);
	}

	Mailbox* new_box = new Mailbox(name, opt, numb_opt, params);

	if (!mailboxes)
		mailboxes = new_box;
	else {
		new_box->next = mailboxes;
		mailboxes = new_box;
	}
}

ServerConfiguration::ServerConfiguration(char* config_path_):
	fd(-1), port(-1), buf(CONFIG_BUF_SIZE), server(NULL), domain(NULL),
	queue_path(NULL), mailbox_manager()
{
	config_path = strdup(config_path_);
}

ServerConfiguration::~ServerConfiguration()
{
	free(config_path);
	free(server);
	free(domain);
	free(queue_path);
}

int ServerConfiguration::ConvertStringToNumber(char* port_str) const
{
	int len = strlen(port_str);
	if (len == 0)
		throw ConfigError("Port was not specified");
	bool digits_only = true;
	for (int i = 0; i < len; i++)
		if ((port_str[i] > '9') || (port_str[i] < '0'))
			digits_only = false;
	if (digits_only == true)
		return atoi(port_str);
	else
		throw ConfigError("Incorrect symbols were used to specify port");
	throw ConfigError("Port was not specified");
}

bool ServerConfiguration::OpenConfig()
{
	fd = open(config_path, O_RDONLY);
	if (fd == -1) {
		perror(config_path);
		exit(1);
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

// TODO: ServerConfiguration::ExtractInfoFromConfig is horrible. needs to be refactored

void ServerConfiguration::ExtractInfoFromConfig()
{
	bool mailboxes = false;
	char tmp_buf[CONFIG_BUF_SIZE];
	int res = read(fd, &(tmp_buf[0]), CONFIG_BUF_SIZE);
	while (res) {
		buf.EatData(res, &(tmp_buf[0]));
		char* line = buf.ExtractUntilEOL();
		while(line) {
			//comments and empty lines are ignored
			if ((line[0] == '#') || (line[0] == '\0')) {}
			else {
				char* word = buf.ExtractWordFromLine(line);
				if (word && !strcmp(word, "port")) {
					word = buf.ExtractWordFromLine(line);
					port = ConvertStringToNumber(word);
				} else if (word && !strcmp(word, "server_name")) {
					word = buf.ExtractWordFromLine(line);
					server = strdup(word);
				} else if (word && !strcmp(word, "domain_name")) {
					word = buf.ExtractWordFromLine(line);
					domain = strdup(word);
				} else if (word && !strcmp(word, "mailboxes_start")) {
					mailboxes = true;
				} else if (word && !strcmp(word, "queue_path")) {
					word = buf.ExtractWordFromLine(line);
					queue_path = strdup(word);
				}
				else if (word && !strcmp(word, "mailboxes_end")) {
					mailboxes = false;
				} else if (mailboxes && word) {
					try {
						mailbox_manager.AddMailbox(strdup(word), line, buf);
					} catch (ConfigError e) {
						e.Print();
					}
				}
			}
			line = buf.ExtractUntilEOL();
		}
		res = read(fd, &(tmp_buf[0]), CONFIG_BUF_SIZE);
	}
	if (res == -1) {
		perror(config_path);
		exit(1);
	}
}


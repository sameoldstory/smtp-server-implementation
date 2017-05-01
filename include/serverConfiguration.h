#ifndef SERVER_CONFIGURATION_H
#define SERVER_CONFIGURATION_H

#include "buffer.h"
#include "types.h"

struct Mailbox {
	char* name;
	mail_option option;
	int param_numb;
	char** params;
	Mailbox* next;
	Mailbox(char* name_, mail_option opt, int numb, char** params_ = 0):
		name(name_), option(opt), param_numb(numb), params(params_), next(0) {}
	Mailbox(const Mailbox& box);
	const char* GetOptionAsString() const;
	~Mailbox();
};

class MailboxManager {
	friend class ServerConfiguration;
	Mailbox* mailboxes;
protected:
	void AddMailbox(char* name, char* opt_line, ParseBuffer& buf);
public:
	Mailbox* GetMailbox(char* box_name) const;
	void PrintMailboxes() const;
	MailboxManager();
	~MailboxManager();
};

class ServerConfiguration {
	char* config_path;
	int fd;
	int port;
	int timeout;
	ParseBuffer buf;
	char* server;
	char* domain;
	char* queue_path;
	int ConvertStringToNumber(char* port_str) const;
	bool OpenConfig();
	void ExtractInfoFromConfig();
	bool CloseConfig();
public:
	MailboxManager mailbox_manager;
	ServerConfiguration(char* config_path_);
	void Configure();
	int GetPort() const {return port;}
	int GetTimeout() const {return timeout;}
	char* GetServerName() const {return server;}
	char* GetDomainName() const {return domain;}
	char* GetQueuePath() const {return queue_path;}
	~ServerConfiguration();
};

#endif
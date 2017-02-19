#ifndef SERVER_CONFIGURATION_H
#define SERVER_CONFIGURATION_H

#include "buffer.h"

enum mail_option {forward, deliver, trap};

struct Mailbox {
	char* name;
	mail_option option;
	int param_numb;
	char** params;
	Mailbox(char* name_, mail_option opt, int numb, char** params_ = 0):
		name(name_), option(opt), param_numb(numb), params(params_) {}
	~Mailbox();
};

struct MailboxList {
	Mailbox box;
	MailboxList* next;
	MailboxList(char* name_, mail_option opt, char** params, int param_num, MailboxList* next_ = 0):
		box(name_, opt, param_num, params), next(next_) {}
};

class ServerConfiguration {
	char* config_path;
	int fd;
	int port;
	ParseBuffer buf;
	char* server;
	char* domain;
	char* queue_path;
	MailboxList* mailboxes;
	MailboxList* CreateMailbox(char* name, char* opt_line);
	void AddMailboxToList(MailboxList* m);
	int ConvertStringToNumber(char* port_str) const;
public:
	ServerConfiguration(char* config_path_);
	char* GetConfigPath() const {return config_path;};
	bool OpenConfig();
	void ExtractInfoFromConfig();
	bool CloseConfig();
	bool MailboxLocal(char*) const;
	void PrintMailboxes() const;
	void PrintEverything() const;
	int GetPort() const {return port;}
	char* GetServerName() const {return server;}
	char* GetDomainName() const {return domain;}
	char* GetQueuePath() const {return queue_path;}
	~ServerConfiguration();
};


#endif
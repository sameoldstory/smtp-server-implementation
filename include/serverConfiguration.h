#ifndef SERVER_CONFIGURATION_H
#define SERVER_CONFIGURATION_H

class FileBuffer;

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
	FileBuffer* buf;
	char* server;
	char* domain;
	MailboxList* mailboxes;
	MailboxList* CreateMailbox(char* name, char* opt_line);
	void AddMailboxToList(MailboxList* m);
	int ConvertStringToNumber(char* port_str);
public:
	ServerConfiguration(char* config_path_);
	char* GetConfigPath() {return config_path;};
	bool OpenConfig();
	bool InitializeBuffer();
	void ExtractInfoFromConfig();
	bool CloseConfig();
	void PrintMailboxes();
	int GetPort() {return port;}
	~ServerConfiguration();
};


#endif
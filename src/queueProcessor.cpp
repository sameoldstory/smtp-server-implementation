#include "queueProcessor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "exceptions.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

QueueProcessor::QueueProcessor(int seconds, char* _path, char* _server_name, MailboxManager& _manager):
	timeout(seconds), counter(0), mailboxes(_manager)
{
	path = strdup(_path);
	server_name = strdup(_server_name);
}

QueueProcessor::~QueueProcessor()
{
	free(path);
	free(server_name);
}

void QueueProcessor::CreateMailQueueDir()
{
	int res = mkdir(path, 0700);
	if (res == -1) {
		if (errno == EEXIST)
			puts("CreateMailQueueDir: already exists");
		else {
			puts("Could not create directory for Mail Queue");
			throw FatalException();
		}
	}
}

void QueueProcessor::ProcessMessage(char* id)
{
	// open .env and .msg files
	char buf[1024];
	snprintf(buf, sizeof(buf), "%s%s.env", path, id);
	int env = open(buf, O_RDONLY);
	snprintf(buf, sizeof(buf), "%s%s.msg", path, id);
	int msg = open(buf, O_RDONLY);
	if (env == -1 || msg == -1) {
		char err_msg[1024];
		snprintf(err_msg, sizeof(err_msg), "Queue manager can't open files with message id: %s", id);
		throw err_msg;
		close(env);
		close(msg);
	}

	// analyze service file here

	close(env);
	close(msg);
}

void QueueProcessor::Run()
{
	printf("QueueProcessor::Run\n");
}

int QueueProcessor::GetTimeout()
{
	return timeout;
}
#include "queueManager.h"
#include <sys/stat.h>
#include <stdio.h>
#include "exceptions.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

QueueManager::QueueManager(char* _path, char* _server_name, MailboxManager& _manager):
	counter(0), path(_path), server_name(_server_name), mailboxes(_manager)
{

}

QueueManager::~QueueManager()
{
	free(path);
}

void QueueManager::CreateMailQueueDir()
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

void QueueManager::TrySendingMessage()
{

}

void QueueManager::GoThroughQueue()
{

}
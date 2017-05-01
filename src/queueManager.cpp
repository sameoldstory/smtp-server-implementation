#include "queueManager.h"
#include <sys/stat.h>
#include <stdio.h>
#include "exceptions.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

QueueManager::QueueManager(): queue_path(NULL)
{

}

QueueManager::~QueueManager()
{
	free(queue_path);
}

void QueueManager::SetQueuePath(char* path)
{
	queue_path = strdup(path);
}

void QueueManager::CreateMailQueueDir()
{
	int res = mkdir(queue_path, 0700);
	if (res == -1) {
		if (errno == EEXIST)
			puts("CreateMailQueueDir: already exists");
		else {
			puts("Could not create directory for Mail Queue");
			throw FatalException();
		}
	}
}




#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

class QueueManager {
	char* queue_path;
public:
	QueueManager();
	~QueueManager(); ;
	void SetQueuePath(char* path);
	void CreateMailQueueDir();
};

#endif
#ifndef SERVER_QUEUE_PROCESSOR_H
#define SERVER_QUEUE_PROCESSOR_H

#include "eventHandler.h"

class QueueProcessor: public EventHandler {
	int timeout;
public:
	QueueProcessor(int seconds);
	virtual void Run();
	virtual int GetTimeout();
	virtual ~QueueProcessor() {}
};

#endif
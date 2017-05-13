#ifndef SERVER_EVENT_HANDLER_H
#define SERVER_EVENT_HANDLER_H

class EventHandler
{
public:
	virtual ~EventHandler() {}
	virtual void Run() = 0;
	virtual int GetTimeout() = 0;
};

#endif
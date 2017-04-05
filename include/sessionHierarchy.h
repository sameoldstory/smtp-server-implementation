#ifndef SMTP_SESSION_H
#define SMTP_SESSION_H

class TCPSessionDriver {
public:
	virtual bool HandleInput(int len, char* buf) = 0;
	virtual void EndSession() = 0;
	virtual char* GetMessage() = 0;
	virtual bool SessionFinished() const = 0;
	virtual bool NeedsToWrite() = 0;
	virtual ~TCPSessionDriver() {}
};

class SMTPSession: public TCPSessionDriver {
protected:
	bool need_to_write;
public:
	SMTPSession(bool _need): need_to_write(_need) {}
	bool NeedsToWrite() {return need_to_write;}
	virtual ~SMTPSession() {}
};

#endif
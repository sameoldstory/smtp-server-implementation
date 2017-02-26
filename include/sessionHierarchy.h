#ifndef SMTP_SESSION_H
#define SMTP_SESSION_H

class TCPSessionDriver {
public:
	virtual bool HandleInput(int len, char* buf) = 0;
	virtual void EndSession() = 0;
	virtual char* GetMessage() = 0;
	virtual bool SessionFinished() = 0;
	virtual ~TCPSessionDriver() {}
};

class SMTPSession: public TCPSessionDriver {
public:
	virtual ~SMTPSession() {}
};

#endif
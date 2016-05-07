#ifndef SERVER_BUFFER_H
#define SERVER_BUFFER_H

class SocketBuffer {
	char* data;
	int len;
	int maxlen;
	int fd;
public:
	SocketBuffer(int fd_);
	char* ExtractUntilCRLF();
	bool Read();
	~SocketBuffer();
};

#endif
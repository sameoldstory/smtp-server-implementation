#ifndef SERVER_BUFFER_H
#define SERVER_BUFFER_H

class FileBuffer {
	char* data;
	int len;
	int maxlen;
	int fd;
public:
	FileBuffer(int fd_);
	char* ExtractWordFromLine(char* & line);
	char* ExtractUntilCRLF();
	char* ExtractUntilEOL();
	bool Read();
	~FileBuffer();
};

#endif
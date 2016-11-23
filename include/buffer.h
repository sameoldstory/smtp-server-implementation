#ifndef SERVER_BUFFER_H
#define SERVER_BUFFER_H

//#define DEFAULT_BUF_SIZE 2048

class ParseBuffer {
	char* data;
	int len;
	int maxlen;
	char* untileol;
	char* untilcrlf;
	char* word;
public:
	ParseBuffer(int maxlen_);
	char* ExtractWordFromLine(char* & line);
	char* ExtractUntilCRLF();
	char* ExtractUntilEOL();
	void EatData(int portion, char* buf);
	void PrintData();
	~ParseBuffer();
};

#endif
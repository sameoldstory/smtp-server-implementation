#ifndef SERVER_BUFFER_H
#define SERVER_BUFFER_H

#define MAX_WORD_SIZE 1024

class ParseBuffer {
	char* data;
	int len;
	int maxlen;
	char* ret_string;
	char word[MAX_WORD_SIZE];
	void Resize();
public:
	ParseBuffer(int maxlen_);
	char* ExtractWordFromLine(char* & line);
	char* ExtractUntilCRLF();
	char* ExtractUntilEOL();
	//char* GetAllAvailableData();
	void EatData(int portion, char* buf);
	void PrintData();
	~ParseBuffer();
};

#endif



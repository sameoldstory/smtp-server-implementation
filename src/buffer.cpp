#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// for now there will be a lot of pointless new and delete[] operators
// it will be optimized a bit later 

ParseBuffer::ParseBuffer(int maxlen_):len(0), maxlen(maxlen_), 
	untileol(NULL), untilcrlf(NULL), word(NULL)
{
	data = new char [maxlen];
}

ParseBuffer::~ParseBuffer()
{	
	delete[] data;
	if (untileol)
		delete[] untileol;
	if (untilcrlf)
		delete[] untilcrlf;
}

void ParseBuffer::PrintData()
{
	data[len] = '\0';
	printf("%s\n", data);
}

void ParseBuffer::EatData(int portion, char* buf)
{
	if (len+portion > maxlen) {
		maxlen += maxlen;
		char* tmp = new char[maxlen];
		memcpy(tmp, data, len);
		delete[] data;
		data = tmp;
	}
	memcpy(data+len, buf, portion);
	len = len + portion;
}

//returns NULL if \r\n isn't here

char* ParseBuffer::ExtractUntilCRLF()
{
	if (!len)
		return NULL;
	for (int i = 0; i < len-1; i++) {
		if ((data[i] == '\r') && (data[i+1] == '\n')) {
			if (untilcrlf) {
				delete[] untilcrlf;
				untilcrlf = NULL;
			}
			untilcrlf = new char[i+1];
			memcpy(untilcrlf, data, i);
			untilcrlf[i] = '\0';
			len -= i + 2;
			memmove(data, data+i+2, len);
			return untilcrlf;
		}
	}
	return NULL;
}

char* ParseBuffer::ExtractUntilEOL()
{
	if (!len)
		return NULL;
	for (int i = 0; i < len-1; i++) {
		if (untileol) {
			delete[] untileol;
			untileol = NULL;
		}
		if (data[i] == '\n')  {
			untileol = new char[i+1];
			memcpy(untileol, data, i);
			untileol[i] = '\0';
			len -= i + 1;
			memmove(data, data+i+1, len);
			return untileol;
		}
	}
	return NULL;
}

char* ParseBuffer::ExtractWordFromLine(char* & line)
{
	int beg = 0, i = 0, len = strlen(line);
	while (i <= len) {
		if ((line[i] == ' ') || (line[i] == '\t') || (line[i] == '\0')) {
			if (beg == i)
				beg++;
			else {
				if (word) {
					delete[] word;
					word = NULL;
				}
				word = new char [i - beg + 1];
				memcpy(word, line, i-beg);
				word[i - beg] = '\0';
				while ((line[i] == ' ') || (line[i] == '\t'))
					i++;
				memmove(line, line+i, len-i+1);
				return word;
			}
		}
		i++;
	}
	return NULL;
}


#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

ParseBuffer::ParseBuffer(int maxlen_):len(0), maxlen(maxlen_)
{
	data = new char [maxlen];
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
			char* str = new char[i+1];
			memcpy(str, data, i);
			str[i] = '\0';
			len -= i + 2;
			memmove(data, data+i+2, len);
			return str;
		}
	}
	return NULL;
}

char* ParseBuffer::ExtractUntilEOL()
{
	if (!len)
		return NULL;
	for (int i = 0; i < len-1; i++) {
		if (data[i] == '\n')  {
			char* str = new char[i+1];
			memcpy(str, data, i);
			str[i] = '\0';
			len -= i + 1;
			memmove(data, data+i+1, len);
			return str;
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
				char* word = new char [i - beg + 1];
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


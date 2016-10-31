#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define DEFAULT_BUF_SIZE 1024

FileBuffer::FileBuffer(int fd_): len(0), maxlen(DEFAULT_BUF_SIZE), fd(fd_)
{
	data = new char [maxlen];
}

FileBuffer::~FileBuffer()
{
	delete[] data;
}

//returns false if connection was closed, true otherwise

bool FileBuffer::Read()
{
	int portion = read(fd, data+len, maxlen-len);
	if (portion == -1) {
		perror("read");
		//probably need some error handling here
	} else if (portion == 0) {
		return false;
	} else {
		len += portion;
	}
	return true;
}

//returns NULL if \r\n isn't here

char* FileBuffer::ExtractUntilCRLF()
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

char* FileBuffer::ExtractUntilEOL()
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

char* FileBuffer::ExtractWordFromLine(char* & line)
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


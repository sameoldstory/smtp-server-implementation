#include "buffer.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "constantValues.h"

SocketBuffer::SocketBuffer(int fd_): len(0), maxlen(DEFAULT_BUF_SIZE), fd(fd_)
{
	data = new char [maxlen];
}

SocketBuffer::~SocketBuffer()
{
	delete[] data;
}

//returns false if connection was closed, true otherwise

bool SocketBuffer::Read()
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

char* SocketBuffer::ExtractUntilCRLF()
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
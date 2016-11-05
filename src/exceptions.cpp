#include "exceptions.h"
#include <string.h>
#include <stdio.h>

ConfigError::ConfigError(const char* msg)
{
	int len = strlen(msg)+1;
 	message = new char[len];
 	memcpy(message, msg, len);
}

void ConfigError::Print() const
{
	printf("%s\n", message);
	delete[] message;
}
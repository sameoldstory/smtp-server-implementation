#ifndef SERVER_EXCEPTIONS_H
#define SERVER_EXCEPTIONS_H

class FatalException {
};

class ConfigError {
	char* message;
public:
	ConfigError(const char* msg);
	void Print() const;
};

#endif

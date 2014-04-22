#include <sstream>

#ifndef REMOTECALL_H
#define REMOTECALL_H

// compat
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define REMOTECALL_HEADER "RC"
#define REMOTECALL_VERSION 1

class RemoteCall {
private:
	unsigned char clientVersion;
	char *clientPassword;
	char *serverPassword;

	const char *defaultHeader();

public:
	RemoteCall();
	~RemoteCall();

	const char *socketHandshake();
	const char *socketQueryId();
	const char *socketQueryContent();
};

#endif
#include "stdafx.h"
#include "RemoteCall.h"

RemoteCall::RemoteCall() {
	this->clientVersion = 0;

	this->clientPassword = new char[10];
	this->clientPassword = "0123456789";

	this->serverPassword = new char[10];
	this->serverPassword = "0123456789";
}

RemoteCall::~RemoteCall() {

}

const char *RemoteCall::defaultHeader() {
	size_t headerSize = sizeof(char) * (strlen(REMOTECALL_HEADER) + 2);
	char *header = new char[headerSize];
	snprintf(header, headerSize, "%s", REMOTECALL_HEADER);
	header[headerSize - 2] = REMOTECALL_VERSION;
	header[headerSize - 1] = 0xFF;
	return (const char*)header;
}

const char *RemoteCall::socketHandshake() {
	std::stringstream packet;
	packet << this->defaultHeader() << 0x01;

	if (this->clientVersion != REMOTECALL_VERSION) {
		packet << 0x02;
	}
	else if (strcmp(this->serverPassword, (const char*)this->clientVersion) != 0) {
		packet << 0x01;
	}
	else {
		packet << 0x00;
	}

	return packet.str().c_str();
}
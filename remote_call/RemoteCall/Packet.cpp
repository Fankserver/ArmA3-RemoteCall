#include "Packet.hpp"

// Constructor
RemoteCall::Packet::Packet() {
	strncpy(this->identfier, "RC", sizeof(this->identfier));
	this->version = 0x01;
	this->spacer = 0xFF;
	this->command = NULL;
	this->contentLength = 0;
	this->content = NULL;
}

// Destructor
RemoteCall::Packet::~Packet() {
	if (this->content != NULL) {
		delete[] this->content;
	}
}

// Allocator
void RemoteCall::Packet::_allocContent(size_t _size) {
	if (this->content != NULL) {
		delete[] this->content;
	}

	this->content = new char[_size];
	this->contentLength = _size;
}

// Command
bool RemoteCall::Packet::hasCommand() {
	return (this->command != NULL ? true : false);
}
unsigned char RemoteCall::Packet::getCommand() {
	return this->command;
}
void RemoteCall::Packet::setCommand(unsigned char _command) {
	this->command = _command;
}

// Content
bool RemoteCall::Packet::hasContent() {
	return (this->contentLength > 0 ? true : false);
}
const char* RemoteCall::Packet::getContent() {
	if (this->content != NULL) {
		return this->content;
	}
	else {
		return "";
	}
}
void RemoteCall::Packet::setContent(const char _content) {
	this->_allocContent(1);
	this->content[0] = _content;
}
void RemoteCall::Packet::setContent(const char *_content) {
	this->_allocContent(strlen(_content));
	memcpy(this->content, _content, strlen(_content));
}
void RemoteCall::Packet::setContent(short int _content) {
	char contentC[2];
	memcpy(contentC, &_content, 2);

	this->_allocContent(2);
	memcpy(this->content, contentC, 2);
}

// Serialize
void RemoteCall::Packet::serialize(char *&_content, int *_contentLength) {
	size_t serialSize = 5 + this->contentLength + 1;
	*_contentLength = serialSize - 1;
	char *content = new char[serialSize];
	memset(content, NULL, serialSize);

	// Header
	strncpy(content, this->identfier, sizeof(this->identfier));
	content[sizeof(this->identfier) + 0] = this->version;
	content[sizeof(this->identfier) + 1] = this->spacer;
	content[sizeof(this->identfier) + 2] = this->command;

	// Body
	if (this->contentLength > 0) {
		strcpy(content + (sizeof(this->identfier) + 3), this->content);
	}

	// Return
	_content = content;
}
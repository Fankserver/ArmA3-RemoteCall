#include <sstream>
#include <thread>
#include <iostream>
#include <map>
#include <memory>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#endif

#include "SQF.hpp"

#ifndef REMOTECALL_H
#define REMOTECALL_H

// compat
#ifdef _MSC_VER
#define snprintf _snprintf
#define itoa _itoa
#endif

#define REMOTECALL_VERSION 1
#define REMOTECALL_SOCKBUFFER 512
#define REMOTECALL_OUTPUTBUFFER 4096
#define REMOTECALL_PACKETSIZE 5

enum RemoteCallCommands {
	HandshakePassword = 0x00
	,HandshakeResponse
	,QueryContentLength = 0x10
	,QueryContentLengthResponse
	,QueryContent
	,QueryResponseId
	,QueryResponseResult
};
enum RemoteCallHandshake {
	PASSWORD_CORRECT = 0x00
	,PASSWORD_INCORRECT
	,VERSION_INCORRECT
	,COMMAND_INCORRECT
};
enum RemoteCallQueryContentError {
	CONTENT_OK = 0x00
	,CONTENT_TooShort
	,CONTENT_TooLong
	,CONTENT_WaitForContent
};
enum RemoteCallError {
	OK = 0
	,ErrorPassword
	,ErrorVersion
	,ErrorCommand
	,ErrorProtocol
	,ErrorUnhandled
};

class RemoteCall {
private:
	struct serverS {
		int port;
		char *password;
	};
	struct clientS {
		bool loggedIn;
		bool isQueryBuffer;
		unsigned short int queryBufferLength;
		char *queryBuffer;
	};
	struct packetS {
		char identfier[2];
		unsigned char version;
		unsigned char spacer;
		unsigned char command;
		char *content;
	};

private:
	serverS server;
	std::thread socketThread;
	std::map<int, std::string> queryStack;

	int tempQueryId;
	std::string tempQuery;

	// Packet management
	void _createPacket(packetS *Packet);
	bool _unpackPacket(char *Receive, int ReceiveLength, packetS *Packet);
	int _validatePacket(packetS *Packet);
	void _processPacket(clientS *Client, packetS *Packet, packetS *PacketDest, int *PacketDestLength);

	void _initServerSocket();
	void _initClientSocket(SOCKET Socket);

	int _addQuery(char *Query);
	std::string _buildQuerySQF(int _bufferSize);

public:
	RemoteCall();
	~RemoteCall();

	void initServer();
	std::string getStackItem();
};

#endif
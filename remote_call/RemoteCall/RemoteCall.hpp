#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <regex>
#include <thread>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <Strsafe.h>
#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#pragma comment (lib, "DbgHelp.lib")
#endif

// compat
#ifdef _MSC_VER
#define snprintf _snprintf
#define itoa _itoa
#endif

#include "SQF.hpp"

#ifndef REMOTECALL_H
#define REMOTECALL_H

#define REMOTECALL_VERSION 1
#define REMOTECALL_SOCKBUFFER 1024
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
		std::string ip;
		std::string password;
	};
	struct clientS {
		bool loggedIn;
		bool isQueryBuffer;
		size_t queryBufferLength;
		char *queryBuffer;
	};
	struct packetS {
		char identfier[2];
		unsigned char version;
		unsigned char spacer;
		unsigned char command;
		char *content;
	};
	struct queryS {
		int id;
		std::string content;
	};

private:
	serverS server;
	std::thread socketThread;

	std::vector<std::shared_ptr<queryS>> queryStack;
	std::mutex queryStackMutex;

	std::shared_ptr<queryS> tempQuery;

	// Packet management
	void _createPacket(packetS *Packet);
	bool _unpackPacket(const char *Receive, int ReceiveLength, packetS *Packet);
	int _validatePacket(packetS *Packet);
	void _processPacket(clientS *Client, packetS *Packet, packetS *PacketDest, int *PacketDestLength);

	void _initServerSocket();
	void _initClientSocket(SOCKET Socket);

	int _addQuery(const char *Query, size_t QuerySize);
	std::string _buildQuerySQF(int _bufferSize);

	void _log(const char *Message);
	void _readConfig();

public:
	RemoteCall();
	~RemoteCall();

	void initServer();
	std::string getStackItem(int OutputBuffer);
};


LONG WINAPI CrashHandler(EXCEPTION_POINTERS *exPointer);
#endif
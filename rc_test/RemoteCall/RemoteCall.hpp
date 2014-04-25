#include <sstream>
#include <thread>
#include <iostream>
#include <map>
#include <memory>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib") // Need to link with Ws2_32.lib
#endif

#ifndef REMOTECALL_H
#define REMOTECALL_H

// compat
#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define REMOTECALL_VERSION 1
#define REMOTECALL_SOCKBUFFER 512

enum RemoteCallCommands {
	HandshakePassword = 0x00
	,HandshakeResponse
	,Query = 0x10
	,QueryResponseId
	,QueryResponseResult
};
enum RemoteCallError {
	OK = 0
	,ErrorVersion
	,ErrorPassword
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
		char *password;
		bool loggedIn;
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

	// Packet management
	void _createPacket(packetS *Packet);
	bool _unpackPacket(char *Receive, int ReceiveLength, packetS *Packet);
	int _validatePacket(packetS *Packet);
	void _processPacket(clientS *Client, packetS *Packet, packetS *PacketDest);

	void _initServerSocket();
	void _initClientSocket(SOCKET Socket);

	int _addQuery(char *Query);

public:
	RemoteCall();
	~RemoteCall();

	void initServer();
	std::string getStackItem();
};

#endif
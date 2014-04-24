#include <sstream>
#include <thread>
#include <iostream>
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
	char *header;

	std::thread socketThread;

	void _initServerSocket();
	void _initClientSocket(SOCKET Socket);

	// Packet management
	void _createPacket(packetS *Packet);
	bool _unpackPacket(char *Receive, int ReceiveLength, packetS *Packet);
	bool _validatePacket(packetS *Packet);
	void _processPacket(clientS *Client, packetS *Packet, packetS *PacketDest);

public:
	RemoteCall();
	~RemoteCall();

	void initServer();

	const char *socketHandshake();
	const char *socketQueryId();
	const char *socketQueryContent();
};

#endif
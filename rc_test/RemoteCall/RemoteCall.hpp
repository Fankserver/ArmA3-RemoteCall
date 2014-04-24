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

class RemoteCall {
private:
	struct serverS {
		int port;
		char *password;
	};
	struct clientS {
		unsigned char version;
		char *password;
	};
	struct packetS {
		char identfier[2];
		unsigned char version;
		char spacer;
		char command;
		char *content;
	};

private:
	serverS server;
	char *header;

	std::thread socketThread;

	void _buildHeader();
	void _initServerSocket();
	void _initClientSocket(SOCKET Socket);
	bool _unpackPacket(char *Receive, int ReceiveLength, packetS *Packet);

public:
	RemoteCall();
	~RemoteCall();

	void initServer();

	const char *socketHandshake();
	const char *socketQueryId();
	const char *socketQueryContent();
};

#endif
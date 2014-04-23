#include "RemoteCall.hpp"

RemoteCall::RemoteCall() {
	this->server.password = new char[10];
	this->server.password = "0123456789";
}

RemoteCall::~RemoteCall() {

}

// private
void RemoteCall::_buildHeader() {
	size_t headerSize = sizeof(char) * 4;
	this->header = new char[headerSize];
	snprintf(this->header, headerSize, "RC");
	this->header[headerSize - 2] = REMOTECALL_VERSION;
	this->header[headerSize - 1] = 0xFF;
}

void RemoteCall::_initServerSocket() {
	// Winsockets
#ifdef WIN32
	int iResult;
	SOCKET serverSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	// Initialize Winsock
	WSADATA winsockData;
	iResult = WSAStartup(MAKEWORD(2, 2), &winsockData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char serverPort[5];
	itoa(3310, serverPort, 10);
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	printf("Server startup: %s", this->server.port);

	while (1) {
		SOCKET clientsocket = SOCKET_ERROR;

		while (clientsocket == SOCKET_ERROR) {
			clientsocket = accept(serverSocket, NULL, NULL);
		}

		printf("Client Connected.\n");

		//std::thread(&RemoteCall::_initClientSocket, _object, (LPVOID)clientsocket);
	}
#endif
}

void RemoteCall::_initClientSocket(RemoteCall *_object, LPVOID _socket) {
	clientS client;
	printf("client init");
}

// public
void RemoteCall::initServer() {
	this->server.port = 3310;
	this->socketThread = std::thread(&RemoteCall::_initServerSocket, this);
}

//const char *RemoteCall::socketHandshake() {
//	std::stringstream packet;
//	packet << this->header << 0x01;
//
//	if (this->clientVersion != REMOTECALL_VERSION) {
//		packet << 0x02;
//	}
//	else if (strcmp(this->serverPassword, (const char*)this->clientVersion) != 0) {
//		packet << 0x01;
//	}
//	else {
//		packet << 0x00;
//	}
//
//	return packet.str().c_str();
//}
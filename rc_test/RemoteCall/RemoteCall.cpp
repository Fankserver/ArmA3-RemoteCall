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

bool RemoteCall::_unpackPacket(char *_receive, int _receiveLength, packetS *_packet) {
	char *packet = new char[_receiveLength];
	strncpy(packet, _receive, _receiveLength);

	if (_receiveLength >= 5) {
		if (strcmp(_packet->identfier, "") == 0) {
			memcpy(_packet, _receive, _receiveLength);
			strncpy(_packet->identfier, _packet->identfier, 2);
			if (_receiveLength >= 6) {
				int contentLength = _receiveLength - 5;
				_packet->content = new char[contentLength];
				strncpy(_packet->content, _receive + 5, contentLength);
				_packet->content[contentLength] = '\0';
			}
			else {
				_packet->content = new char[1];
				strcpy(_packet->content, "");
			}
			std::cout << "Identifier: " << _packet->identfier[0] << _packet->identfier[1] << std::endl;
			std::cout << "Version: " << (int)_packet->version << std::endl;
			std::cout << "Spacer: " << (int)_packet->spacer << std::endl;
			std::cout << "Command: " << (int)_packet->command << std::endl;
			std::cout << "Content: " << _packet->content << std::endl;
		}
	}
	else {
		return false;
	}

	return true;
}

// Winsockets
#ifdef WIN32
void RemoteCall::_initServerSocket() {
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

	listen(serverSocket, 10);
	std::cout << "Server startup: " << this->server.port << std::endl;

	while (1) {
		SOCKET clientSocket = SOCKET_ERROR;

		while (clientSocket == SOCKET_ERROR) {
			clientSocket = accept(serverSocket, NULL, NULL);
		}

		std::cout << "Client connected" << std::endl;
		std::thread(std::bind(&RemoteCall::_initClientSocket, this, clientSocket)).detach();
	}
}

void RemoteCall::_initClientSocket(SOCKET _socket) {
	clientS client;
	int iResult;
	int recvBufLen = REMOTECALL_SOCKBUFFER;
	char recvbuf[REMOTECALL_SOCKBUFFER];

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(_socket, recvbuf, recvBufLen, 0);
		if (iResult > 0) {
			packetS packet;
			memset(&packet, 0, sizeof(packetS));
			std::cout << "Bytes received: " << iResult << std::endl;

			this->_unpackPacket(recvbuf, iResult, &packet);
			int iSendResult = send(_socket, "1", 1, 0);
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << "\n";
				closesocket(_socket);
				WSACleanup();
				return;
			}
			
			std::cout << "Bytes sent: " << iSendResult << std::endl;
		}
		else if (iResult == 0) {
			std::cout << "Connection closing...\n";
		}
		else  {
			std::cout << "recv failed with error: " << WSAGetLastError() << "\n";
			closesocket(_socket);
			WSACleanup();
		}
	} while (iResult > 0);
}
#endif

// public
void RemoteCall::initServer() {
	this->server.port = 3310;
	this->socketThread = std::thread(std::bind(&RemoteCall::_initServerSocket, this));
	this->socketThread.detach();
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
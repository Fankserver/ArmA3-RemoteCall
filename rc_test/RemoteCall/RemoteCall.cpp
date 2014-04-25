#include "RemoteCall.hpp"

RemoteCall::RemoteCall() {
	this->server.password = new char[10];
	this->server.password = "0123456789";

	this->server.port = 3310;

	this->tempQueryId = 0;
	this->tempQuery = "";
}

RemoteCall::~RemoteCall() {

}

// private
void RemoteCall::_createPacket(packetS *_packet) {
	strncpy(_packet->identfier, "RC", 2);
	_packet->version = REMOTECALL_VERSION;
	_packet->spacer = 0xFF;
	_packet->command = 0x00;
	_packet->content = new char[1];
	_packet->content = '\0';
}
bool RemoteCall::_unpackPacket(char *_receive, int _receiveLength, packetS *_packet) {
	// create limited packet
	char *packet = new char[_receiveLength];
	strncpy(packet, _receive, _receiveLength);

	if (_receiveLength >= 5) {
		// Is new packet
		if (strcmp(_packet->identfier, "") == 0) {
			// wrap the packet
			memcpy(_packet, _receive, _receiveLength);

			// contains content
			if (_receiveLength >= 6) {

				// Allocate new content
				int contentLength = _receiveLength - 5;
				_packet->content = new char[contentLength];

				// copy content
				strncpy(_packet->content, _receive + 5, contentLength);
				_packet->content[contentLength] = '\0';
			}

			// command only
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
		else {
			// Allocate new content
			int contentLength = strlen(_packet->content) + _receiveLength;
			char *newContent = (char *)malloc(contentLength);

			// copy and concat
			strcpy(newContent, _packet->content);
			strncat(newContent, _receive, _receiveLength);
			
			// release old & store new content
			free(_packet->content);
			_packet->content = newContent;
		}
	}
	else {
		return false;
	}

	return true;
}
int RemoteCall::_validatePacket(packetS *_packet) {
	// Valid RemoteCall packet
	if (
		_packet->identfier[0] == 'R' && _packet->identfier[1] == 'C'
		&& (int)_packet->version > 0
		&& (int)_packet->spacer == 0xFF
		&& (int)_packet->command >= 0
	) {
		// Wrong version
		if (_packet->version != REMOTECALL_VERSION) {
			return RemoteCallError::ErrorVersion;
		}

		// Check client command
		if (
			_packet->command == RemoteCallCommands::HandshakePassword
			|| _packet->command == RemoteCallCommands::QueryContentLength
			|| _packet->command == RemoteCallCommands::QueryContent
		) {
			return RemoteCallError::OK;
		}
		else {
			return RemoteCallError::ErrorCommand;
		}
	}
	else {
		return RemoteCallError::ErrorProtocol;
	}

	return RemoteCallError::ErrorUnhandled;
}
void RemoteCall::_processPacket(clientS *_client, packetS *_packet, packetS *_packetDest) {
	// Client logged in
	if (_client->loggedIn) {
		if (_packet->command == RemoteCallCommands::QueryContentLength) {
			unsigned short int queryLength = (unsigned short int)_packet->content;
			if (queryLength > 0) {
				// create temp buffer
				_client->isQueryBuffer = true;
				_client->queryBufferLength = queryLength;
				_client->queryBuffer = new char[queryLength];
				_client->queryBuffer[0] = '\0';
			}
			else {
				// wrong query length
			}
		}
		else if (_packet->command == RemoteCallCommands::QueryContent) {
			int queryId = this->_addQuery(_packet->content);
			if (queryId > 0) {
				_packetDest->command = RemoteCallCommands::QueryResponseId;
				_packetDest->content = new char[5];
				itoa(queryId, _packetDest->content, 5);
				std::cout << "Add Query (" << queryId << "): " << _packet->content << std::endl;
			}
			else {
				std::cout << "Query full!" << std::endl;
			}
		}
	}
	else {
		if (_packet->command == RemoteCallCommands::HandshakePassword) {
			if (strcmp(_packet->content, this->server.password) == 0) {
				_client->loggedIn = true;
				std::cout << "Client logged in" << std::endl;
			}
		}
		else {
			std::cout << "Wrong command client not logged in" << std::endl;
		}
	}
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

	client.password = "";
	client.loggedIn = false;
	client.isQueryBuffer = false;
	
	// Receive until the peer shuts down the connection
	do {
		iResult = recv(_socket, recvbuf, recvBufLen, 0);
		if (iResult > 0) {
			packetS packet;
			memset(&packet, 0, sizeof(packetS));
			this->_unpackPacket(recvbuf, iResult, &packet);

			int packetError = this->_validatePacket(&packet);
			switch (packetError) {
				case RemoteCallError::OK: {
					packetS *responsePacket = new packetS;
					this->_createPacket(responsePacket);
					this->_processPacket(&client, &packet, responsePacket);

					int iSendResult = send(_socket, (char*)responsePacket, 1, 0);
					if (iSendResult == SOCKET_ERROR) {
						closesocket(_socket);
						WSACleanup();
						return;
					}

					free(responsePacket->content);
					free(responsePacket->identfier);
					free(responsePacket);

					break;
				}
				case RemoteCallError::ErrorPassword: {
					packetS *responsePacket = new packetS;
					this->_createPacket(responsePacket);
					this->_processPacket(&client, &packet, responsePacket);

					int iSendResult = send(_socket, (char*)responsePacket, 1, 0);
					if (iSendResult == SOCKET_ERROR) {
						closesocket(_socket);
						WSACleanup();
						return;
					}

					free(responsePacket->content);
					free(responsePacket->identfier);
					free(responsePacket);

					break;
				}
				default: {
					// unexpected error
					closesocket(_socket);
					iResult = 0;
				}
			}
		}
		else if (iResult == 0) {
			std::cout << "Connection closing...\n";
		}
		else  {
			closesocket(_socket);
			WSACleanup();
		}
	} while (iResult > 0);

	free(client.password);
	free(recvbuf);
}
#endif

int RemoteCall::_addQuery(char *_query) {
	int queryId = 0;
	for (int i = 1; i <= 10000; i++) {
		try {
			this->queryStack.at(i);
		}
		catch (...) {
			queryId = i;
			this->queryStack.insert(std::pair<int, std::string>(queryId, _query));
			break;
		}
	}

	return queryId;
}
std::string RemoteCall::_buildQuerySQF(int _bufferSize) {
	size_t bufferSize = _bufferSize - 64; // remote overhead from buffer
	std::string ret = "[0,1,\"\"]";

	if (this->tempQuery.size() > 0) {
		SQF sqfQuery;
		sqfQuery.push(this->tempQueryId);

		// tempQuery nosplit
		if (this->tempQuery.size() <= bufferSize) {
			sqfQuery.push(1);
			sqfQuery.push(this->tempQuery.c_str());

			// finish
			this->tempQuery = "";
			this->tempQueryId = 0;
		}
		else {
			sqfQuery.push(0);
			sqfQuery.push(this->tempQuery.substr(0, bufferSize).c_str());
			this->tempQuery.erase(0, bufferSize);
		}
		ret = sqfQuery.toArray();
	}

	return ret;
}

// public
void RemoteCall::initServer() {
	this->socketThread = std::thread(std::bind(&RemoteCall::_initServerSocket, this));
	this->socketThread.detach();
}

std::string RemoteCall::getStackItem() {
	std::string returnString;

	if (this->tempQuery.size() > 0) {
		returnString = this->_buildQuerySQF(REMOTECALL_OUTPUTBUFFER);
	}
	else if (this->queryStack.size() > 0) {
		// Get first query item
		std::map<int, std::string>::iterator queryItem;
		queryItem = this->queryStack.begin();

		// Copy values
		int queryId = queryItem->first;
		std::string query = queryItem->second;

		// Delete item from stack
		this->queryStack.erase(queryItem);

		// replace Quote with double quote
		std::stringstream queryStream;
		for (size_t i = 0; i <= query.length(); i++) {
			queryStream << query[i];
			if (query[i] == '"') {
				queryStream << '"';
			}
		}
		query = queryStream.str();
		this->tempQuery = query;

		returnString = this->_buildQuerySQF(REMOTECALL_OUTPUTBUFFER);
	}

	return returnString;
}
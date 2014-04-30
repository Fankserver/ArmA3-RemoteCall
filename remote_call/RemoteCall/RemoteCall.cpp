#include "RemoteCall.hpp"

RemoteCall::RemoteCall() {
	this->_readConfig();
	std::cout << "Server startup on port " << this->server.port << std::endl;
}

RemoteCall::~RemoteCall() {
	this->tempQuery.reset();
}

// private
void RemoteCall::_createPacket(packetS *_packet) {
	memset(_packet, 0, sizeof(packetS));
	strncpy(_packet->identfier, "RC", 2);
	_packet->version = REMOTECALL_VERSION;
	_packet->spacer = 0xFF;
	_packet->command = 0x00;
	_packet->content = NULL;
}
bool RemoteCall::_unpackPacket(const char *_receive, int _receiveLength, packetS *_packet) {
	// long enought to be a packet
	if (_receiveLength >= REMOTECALL_PACKETSIZE) {
		// Is new packet
		if (strcmp(_packet->identfier, "") == 0) {
			// wrap the packet
			memcpy(_packet, _receive, sizeof(packetS));

			// contains content
			if (_receiveLength >= 6) {

				// Allocate new content
				int contentLength = _receiveLength - REMOTECALL_PACKETSIZE;
				_packet->content = new char[contentLength];

				// copy content
				memcpy(_packet->content, _receive + REMOTECALL_PACKETSIZE, contentLength);
				_packet->content[contentLength] = '\0';
			}

			// command only
			else {
				_packet->content = new char[1];
				strcpy(_packet->content, "");
			}
		}
	}
	else {
		return false;
	}

	return true;
}
int RemoteCall::_validatePacket(packetS *_packet) {
	this->_log("00106");
	// Valid RemoteCall packet
	if (
		_packet->identfier[0] == 'R' && _packet->identfier[1] == 'C'
		&& (int)_packet->version > 0
		&& (int)_packet->spacer == 0xFF
		&& (int)_packet->command >= 0
	) {
		this->_log("00100");
		// Wrong version
		if (_packet->version != REMOTECALL_VERSION) {
			this->_log("00101");
			return RemoteCallError::ErrorVersion;
		}

		// Check client command
		if (
			_packet->command == RemoteCallCommands::HandshakePassword
			|| _packet->command == RemoteCallCommands::QueryContentLength
			|| _packet->command == RemoteCallCommands::QueryContent
		) {
			this->_log("00102");
			return RemoteCallError::OK;
		}
		else {
			this->_log("00103");
			return RemoteCallError::ErrorCommand;
		}
	}
	else {
		this->_log("00104");
		return RemoteCallError::ErrorProtocol;
	}

	this->_log("00105");
	return RemoteCallError::ErrorUnhandled;
}
void RemoteCall::_processPacket(clientS *_client, packetS *_packet, packetS *_packetDest, int *_packetDestLength) {
	// Client logged in
	if (_client->loggedIn) {
		// Query content length
		if (_packet->command == RemoteCallCommands::QueryContentLength) {
			_packetDest->command = RemoteCallCommands::QueryContentLengthResponse;
			_packetDest->content = new char[1];
			*_packetDestLength = REMOTECALL_PACKETSIZE + 1;

			// query buffer in use
			if (_client->isQueryBuffer) {
				_packetDest->content[0] = RemoteCallQueryContentError::CONTENT_WaitForContent;
			}
			else {
				unsigned short int queryLength = 0;
				memcpy(&queryLength, _packet->content, 2);
				if (queryLength > 0) {
					if (queryLength < 131072) {
						_client->isQueryBuffer = true;

						// create temp buffer
						_client->queryBufferLength = sizeof(char) + queryLength;
						char *queryBuffer = (char*)malloc(_client->queryBufferLength);
						memset(queryBuffer, 0, _client->queryBufferLength);
						if (queryBuffer == NULL) {
							this->_log("Alloc failed");
							exit(1);
						}
						_client->queryBuffer = queryBuffer;

						// wrong query length
						_packetDest->content[0] = RemoteCallQueryContentError::CONTENT_OK;
					}
					else {
						// wrong query length
						_packetDest->content[0] = RemoteCallQueryContentError::CONTENT_TooLong;
					}
				}
				else {
					// wrong query length
					_packetDest->content[0] = RemoteCallQueryContentError::CONTENT_TooShort;
				}
			}
		}

		// Query content
		else if (_packet->command == RemoteCallCommands::QueryContent) {
			if (_client->isQueryBuffer) {
				size_t packetContentLength = (sizeof(char)* (strlen(_packet->content) + strlen(_client->queryBuffer)) >= _client->queryBufferLength ? ((_client->queryBufferLength - (sizeof(char)* strlen(_client->queryBuffer))) - 1) : sizeof(char)* strlen(_packet->content));
				strncat(_client->queryBuffer, _packet->content, packetContentLength);

				if (strlen(_client->queryBuffer) == (_client->queryBufferLength - 1)) {
					short int queryId = this->_addQuery(_client->queryBuffer, _client->queryBufferLength);
					if (queryId > 0) {
						_packetDest->command = RemoteCallCommands::QueryResponseId;
						_packetDest->content = new char[2];
						*_packetDestLength = REMOTECALL_PACKETSIZE + 2;
						memcpy(_packetDest->content, &queryId, 2);
					}
					else {
						// Query full
					}

					_client->isQueryBuffer = false;
					_client->queryBufferLength = 0;
					free(_client->queryBuffer);
				}
			}
		}
	}

	// Client not logged in
	else {
		_packetDest->command = RemoteCallCommands::HandshakeResponse;
		_packetDest->content = new char[1];
		*_packetDestLength = REMOTECALL_PACKETSIZE + 1;
		if (_packet->command == RemoteCallCommands::HandshakePassword) {
			if (this->server.password.compare(_packet->content) == 0) {
				_client->loggedIn = true;

				_packetDest->content[0] = RemoteCallHandshake::PASSWORD_CORRECT;
			}
			else {
				_packetDest->content[0] = RemoteCallHandshake::PASSWORD_INCORRECT;
			}
		}
		else {
			_packetDest->content[0] = RemoteCallHandshake::COMMAND_INCORRECT;
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
		std::stringstream errorMsg;
		errorMsg << "WSAStartup failed with error: " << iResult;
		this->_log(errorMsg.str().c_str());
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char serverPort[5];
	itoa(this->server.port, serverPort, 10);
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);
	if (iResult != 0) {
		std::stringstream errorMsg;
		errorMsg << "getaddrinfo failed with error: " << iResult;
		this->_log(errorMsg.str().c_str());
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
		std::stringstream errorMsg;
		errorMsg << "socket failed with error: " << WSAGetLastError();
		this->_log(errorMsg.str().c_str());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::stringstream errorMsg;
		errorMsg << "bind failed with error: " << WSAGetLastError();
		this->_log(errorMsg.str().c_str());
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	listen(serverSocket, 10);
	while (true) {
		SOCKET clientSocket = SOCKET_ERROR;

		if ((clientSocket = accept(serverSocket, NULL, NULL)) != SOCKET_ERROR) {
			std::thread client(std::bind(&RemoteCall::_initClientSocket, this, clientSocket));
			client.detach();
		}
		else {

		}
	}
}
void RemoteCall::_initClientSocket(SOCKET _socket) {
	clientS client;
	memset(&client, 0, sizeof(clientS));
	int iResult;
	int recvBufLen = REMOTECALL_SOCKBUFFER;
	char *recvbuf = new char[REMOTECALL_SOCKBUFFER];

	client.loggedIn = false;
	client.isQueryBuffer = false;

	// Receive until the peer shuts down the connection
	do {
		this->_log("00001");
		iResult = recv(_socket, recvbuf, recvBufLen, NULL);
		if (iResult > 0) {
			packetS packet;
			memset(&packet, 0, sizeof(packetS));
			this->_log("00002");
			this->_unpackPacket(recvbuf, iResult, &packet);
			this->_log("00003");

			int packetError = this->_validatePacket(&packet);
			this->_log("00004");
			switch (packetError) {
				case RemoteCallError::OK: {
					this->_log("00005");
					int responsePacketLength = 0;
					packetS *responsePacket = new packetS;
					this->_log("00006");
					this->_createPacket(responsePacket);
					this->_log("00007");
					this->_processPacket(&client, &packet, responsePacket, &responsePacketLength);
					this->_log("00008");

					if (responsePacket->command != 0) {
						this->_log("00009");
						char *tempPacket = new char[responsePacketLength];
						this->_log("00010");
						memcpy(tempPacket, responsePacket, responsePacketLength);
						this->_log("00011");
						strncpy(tempPacket + REMOTECALL_PACKETSIZE, responsePacket->content, responsePacketLength - REMOTECALL_PACKETSIZE);
						this->_log("00012");
						int iSendResult = send(_socket, tempPacket, responsePacketLength, 0);
						this->_log("00013");
						if (iSendResult == SOCKET_ERROR) {
							this->_log("SEND FAILED");
							closesocket(_socket);
							iResult = 0;
						}

						this->_log("00014");
						delete[] tempPacket;
						this->_log("00015");
					}

					this->_log("00016");
					delete[] responsePacket->content;
					this->_log("00017");
					delete responsePacket;
					this->_log("00018");
					break;
				}
				case RemoteCallError::ErrorVersion: {
					int responsePacketLength = REMOTECALL_PACKETSIZE + 1;
					packetS *responsePacket = new packetS;
					this->_createPacket(responsePacket);

					responsePacket->command = RemoteCallCommands::HandshakeResponse;
					responsePacket->content = new char[1];
					responsePacket->content[0] = RemoteCallError::ErrorVersion;

					char *tempPacket = new char[responsePacketLength];
					memcpy(tempPacket, responsePacket, responsePacketLength);
					strncpy(tempPacket + REMOTECALL_PACKETSIZE, responsePacket->content, responsePacketLength - REMOTECALL_PACKETSIZE);

					int iSendResult = send(_socket, tempPacket, responsePacketLength, 0);
					if (iSendResult == SOCKET_ERROR) {
						this->_log("SEND FAILED");
						closesocket(_socket);
						iResult = 0;
					}

					delete[] tempPacket;
					delete[] responsePacket->content;
					delete responsePacket;
					break;
				}
				default: {
					// unexpected error
					closesocket(_socket);
					iResult = 0;
				}
			}
		}
		else  {
			this->_log("00019");
			closesocket(_socket);
		}
	} while (iResult > 0);

	this->_log("CLOSED");

	delete[] recvbuf;
}
#endif

int RemoteCall::_addQuery(const char *_query, size_t _querySize) {
	int queryId = 0;

	this->queryStackMutex.lock();

	if (this->queryStack.size() > 0) {
		// Get current queryIds
		std::vector<int> queryIds;
		for (std::vector<std::shared_ptr<queryS>>::iterator it = this->queryStack.begin(); it != this->queryStack.end(); it++) {
			queryIds.push_back((*it)->id);
		}
		std::sort(queryIds.begin(), queryIds.end());

		// Get lowest queryId
		int i = 1;
		for (std::vector<int>::iterator it = queryIds.begin(); it != queryIds.end(); it++) {
			if (*it != i) {
				break;
			}
			i++;
		}
		queryId = i;
	}
	else {
		queryId = 1;
	}

	// Add query to stack
	std::shared_ptr<queryS> newQuery(new queryS);
	newQuery->id = queryId;
	newQuery->content.assign(_query, _querySize);
	this->queryStack.push_back(newQuery);
	
	this->queryStackMutex.unlock();

	return queryId;
}
std::string RemoteCall::_buildQuerySQF(int _bufferSize) {
	size_t bufferSize = _bufferSize - 16; // remote overhead from buffer
	std::string ret = "[]";

	if (this->tempQuery) {
		SQF sqfQuery;
		sqfQuery.push(this->tempQuery->id);

		// tempQuery nosplit
		if (this->tempQuery->content.size() <= bufferSize) {
			sqfQuery.push(1);
			sqfQuery.push(this->tempQuery->content.c_str());

			// finish
			this->tempQuery.reset();
		}
		else {
			sqfQuery.push(0);
			sqfQuery.push(this->tempQuery->content.substr(0, bufferSize).c_str());
			this->tempQuery->content.erase(0, bufferSize);
		}
		ret = sqfQuery.toArray();
	}

	return ret;
}

void RemoteCall::_log(const char *_message) {
#ifdef _USRDLL
	std::ofstream logFile("rc.log", std::ios::out |std::ios::app);
	logFile << _message << std::endl;
	logFile.close();
#else
	std::cout << _message << std::endl;
#endif
}

void RemoteCall::_readConfig() {
	// Prepare RegEx
	std::regex regExConfigEntry("^(\\w+)[ \\t]*=[ \\t]*\"(.*)\";$");

	// Set default values
	this->server.port = 3310;

	// Read config file
	std::ifstream configFile("C:\\remote_call.cfg");
	if (configFile.is_open()) {
		std::string configLine;

		while (std::getline(configFile, configLine)) {
			std::smatch regExMatch;
			std::regex_match(configLine, regExMatch, regExConfigEntry);
			if (regExMatch.size() > 0) {
				if (regExMatch[1].compare("port") == 0) {
					this->server.port = atoi(regExMatch[2].str().c_str());
				}
				else if (regExMatch[1].compare("password") == 0) {
					this->server.password = regExMatch[2].str();
				}
			}
			else {
				std::stringstream logEntry;
				logEntry << "Config line missmatch: " << configLine;
				this->_log(logEntry.str().c_str());
			}
		}

		configFile.close();
	}

	if (this->server.password.compare("") == 0) {
		exit(1);
	}
}

// public
void RemoteCall::initServer() {
	this->socketThread = std::thread(std::bind(&RemoteCall::_initServerSocket, this));
	this->socketThread.detach();
}

std::string RemoteCall::getStackItem(int _outputBuffer) {
	std::string returnString = "[]";

	if (this->tempQuery) {
		returnString = this->_buildQuerySQF(_outputBuffer);
	}
	else if (this->queryStack.size() > 0) {
		// Lock queryStack
		this->queryStackMutex.lock();

		// Get first query item
		std::vector<std::shared_ptr<queryS>>::iterator queryItem;
		this->tempQuery = *this->queryStack.begin();
		this->queryStack.erase(this->queryStack.begin());

		// Unlock queryStack
		this->queryStackMutex.unlock();

		// replace Quote with double quote
		std::stringstream queryStream;
		for (size_t i = 0; i <= this->tempQuery->content.length(); i++) {
			queryStream << this->tempQuery->content[i];
			if (this->tempQuery->content[i] == '"') {
				queryStream << '"';
			}
		}
		this->tempQuery->content = queryStream.str();

		returnString = this->_buildQuerySQF(_outputBuffer);
	}

	return returnString;
}
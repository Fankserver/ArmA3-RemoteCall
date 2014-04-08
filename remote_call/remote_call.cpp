// remote_call.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include <shellapi.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3310"

std::vector<std::shared_ptr<std::string>> shString;
std::thread *worker;

extern "C" {
	__declspec (dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function);
}

std::string getProfileFolder() {
	std::string profileFolder = "";
	LPTSTR cmdLine = GetCommandLine();
	int numCmdLineArgs = 0;
	LPTSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &numCmdLineArgs);

	std::vector<std::string> commandLine;
	commandLine.reserve(numCmdLineArgs);

	for (int i = 0; i < numCmdLineArgs; i++) {
		std::wstring args(cmdLineArgs[i]);
		std::string utf8(args.begin(), args.end());
		commandLine.push_back(utf8);
	}

	for (std::vector<std::string>::iterator it = commandLine.begin(); it != commandLine.end(); it++) {
		std::string starter = "-profiles=";
		if (it->length() < starter.length()) {
			continue;
		}

		std::string compareMe = it->substr(0, starter.length());
		if (compareMe.compare(starter) != 0) {
			continue;
		}

		profileFolder = it->substr(compareMe.length());
	}

	return profileFolder;
}
int getServerPort() {
	std::string profileFolder = "";
	LPTSTR cmdLine = GetCommandLine();
	int numCmdLineArgs = 0;
	LPTSTR *cmdLineArgs = CommandLineToArgvW(cmdLine, &numCmdLineArgs);

	std::vector<std::string> commandLine;
	commandLine.reserve(numCmdLineArgs);

	for (int i = 0; i < numCmdLineArgs; i++) {
		std::wstring args(cmdLineArgs[i]);
		std::string utf8(args.begin(), args.end());
		commandLine.push_back(utf8);
	}

	for (std::vector<std::string>::iterator it = commandLine.begin(); it != commandLine.end(); it++) {
		std::string starter = "-port=";
		if (it->length() < starter.length()) {
			continue;
		}

		std::string compareMe = it->substr(0, starter.length());
		if (compareMe.compare(starter) != 0) {
			continue;
		}

		profileFolder = it->substr(compareMe.length());
	}

	return atoi(profileFolder.c_str());
}

void socketListener() {
	std::ofstream logFile;
	logFile.open("remote_call.socket.log");
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	logFile << "Initialize Winsock\n";
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		logFile << "WSAStartup failed with error: " << iResult << "\n";
		logFile.close();
		exit(1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char stringPort[6];
	sprintf_s(stringPort, "%d", getServerPort() + 1000);
	logFile << "Resolve the server address and port " << stringPort << "\n";
	iResult = getaddrinfo(NULL, (PCSTR)stringPort, &hints, &result);
	if (iResult != 0) {
		logFile << "getaddrinfo failed with error: " << iResult << "\n";
		logFile.close();
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server
	logFile << "Create a SOCKET for connecting to server\n";
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		logFile << "socket failed with error: " << WSAGetLastError() << "\n";
		logFile.close();
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	logFile << "Setup the TCP listening socket\n";
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		logFile << "bind failed with error: " << WSAGetLastError() << "\n";
		logFile.close();
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// Listening on a Socket
	logFile << "Listening on a Socket\n";
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		logFile << "Listen failed with error: " << WSAGetLastError() << "\n";
		logFile.close();
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// Accept a client socket
	logFile << "Accept a client socket\n";
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		logFile << "accept failed: " << WSAGetLastError() << "\n";
		logFile.close();
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	logFile.close();

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			logFile << "Bytes received: " << iResult << "\n";

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, "1", 1, 0);
			if (iSendResult == SOCKET_ERROR) {
				logFile << "send failed with error: " << WSAGetLastError() << "\n";
				logFile.close();
				closesocket(ClientSocket);
				WSACleanup();
				exit(1);
			}
			logFile << "Bytes sent: " << iSendResult << "\n";


			std::shared_ptr<std::string> newWorkerItem (new std::string);
			newWorkerItem->append(recvbuf, iResult);
			shString.push_back(newWorkerItem);
		}
		else if (iResult == 0) {
			logFile << "Connection closing...\n";
		}
		else  {
			logFile << "recv failed with error: " << WSAGetLastError() << "\n";
			logFile.close();
			closesocket(ClientSocket);
			WSACleanup();
			exit(1);
		}

	} while (iResult > 0);


	// shutdown the connection since we're done
	logFile << "shutdown the connection since we're done\n";
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		logFile << "shutdown failed with error: " << WSAGetLastError() << "\n";
		logFile.close();
		closesocket(ClientSocket);
		WSACleanup();
		exit(1);
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	logFile.close();
}

void __stdcall RVExtension(char *output, int outputSize, const char *function) {
	if (strcmp(function, "Init") == 0) {
		worker = new std::thread(socketListener);

		strncpy_s(output, outputSize, "diag_log 'remote_call startup';", _TRUNCATE);
	}
	else if (strcmp(function, "WorkerStack") == 0) {
		if (shString.size() > 0) {
			std::string sqf = *shString.front();
			shString.erase(shString.begin());

			strncpy_s(output, outputSize, sqf.c_str(), _TRUNCATE);
		}
	}
	else if (strcmp(function, "WorkerStackCount") == 0) {
		std::string sqf;
		sqf.append((char *)shString.size());
		strncpy_s(output, outputSize, sqf.c_str(), _TRUNCATE);
	}
}
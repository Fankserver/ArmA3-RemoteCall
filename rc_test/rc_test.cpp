#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "3333"

DWORD WINAPI ProcessClient(LPVOID lpParameter)
{
	SOCKET AcceptSocket = (SOCKET)lpParameter;

	// Send and receive data.
	int bytesSent;
	int bytesRecv = SOCKET_ERROR;
	char sendbuf[2000] = "";
	char recvbuf[2000] = "";

	char timebuf[128];

	sprintf(sendbuf, "Hello, it's a test server (commands: 1, 2, exit)\n");
	bytesSent = send(AcceptSocket, sendbuf, strlen(sendbuf), 0);

	if (bytesSent == SOCKET_ERROR) {
		printf("Error at send hello: %ld\n", WSAGetLastError());
		goto fin;
	}

	while (true) {
		_strtime(timebuf);
		ZeroMemory(recvbuf, sizeof(recvbuf));

		bytesRecv = recv(AcceptSocket, recvbuf, 32, 0);
		printf("%s Client said: %s\n", timebuf, recvbuf);

		if (strcmp(recvbuf, "1") == 0) {
			sprintf(sendbuf, "You typed ONE\n");
			//printf("Sent '%s'\n", sendbuf);
			bytesSent = send(AcceptSocket, sendbuf, strlen(sendbuf), 0);

			if (bytesSent == SOCKET_ERROR) {
				printf("Error at send: %ld\n", WSAGetLastError());
				goto fin;
			}
		}
		else if (strcmp(recvbuf, "2") == 0) {
			sprintf(sendbuf, "You typed TWO\n");
			//printf("Sent '%s'\n", sendbuf);
			bytesSent = send(AcceptSocket, sendbuf, strlen(sendbuf), 0);

			if (bytesSent == SOCKET_ERROR) {
				printf("Error at send: %ld\n", WSAGetLastError());
				goto fin;
			}
		}
		else if (strcmp(recvbuf, "exit") == 0) {
			printf("Client has logged out\n", WSAGetLastError());
			goto fin;
		}
		else {
			sprintf(sendbuf, "unknown command\n");
			//printf("Sent '%s'\n", sendbuf);
			bytesSent = send(AcceptSocket, sendbuf, strlen(sendbuf), 0);

			if (bytesSent == SOCKET_ERROR) {
				printf("Error at send: %ld\n", WSAGetLastError());
				goto fin;
			}
		}
	}

fin:
	printf("Client processed\n");

	closesocket(AcceptSocket);
	return 0;
}

void main(void) {
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	while (1) {
		SOCKET AcceptSocket = SOCKET_ERROR;

		while (AcceptSocket == SOCKET_ERROR) {
			AcceptSocket = accept(ListenSocket, NULL, NULL);
		}

		printf("Client Connected.\n");

		DWORD dwThreadId;
		CreateThread(NULL, 0, ProcessClient, (LPVOID)AcceptSocket, 0, &dwThreadId);
	}

	/*
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				exit(1);
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			exit(1);
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		exit(1);
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	*/
}
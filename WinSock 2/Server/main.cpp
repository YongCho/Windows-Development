#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")


#define DEFAULTBUFFLEN 512

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult;
	int iSendResult;
	char recvbuf[DEFAULTBUFFLEN];
	int recvbuflen = DEFAULTBUFFLEN;
	struct addrinfo *result = NULL, *ptr = NULL, hints;  // delete ptr if not needed
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	char *serverPort;


	if (argc == 2) {
		serverPort = argv[1];
	}
	else {
		printf("Usage: main.exe <listening port number>\nExample: main.exe 22222\n");
		return 0;
	}

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;          // IPv4
	hints.ai_socktype = SOCK_STREAM;    // a stream socket
	hints.ai_protocol = IPPROTO_TCP;    // TCP protocol
	hints.ai_flags = AI_PASSIVE;        // AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function. When the AI_PASSIVE flag is set and nodename parameter (first parameter) to the getaddrinfo function is a NULL pointer, the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses.

	// Resolve the local address and port to be used by the server.
	iResult = getaddrinfo(NULL, serverPort, &hints, &result);

	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// For a server to accept client connections, it must be bound to a network address within the system.
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// After the socket is bound to an IP address and port on the system, the server must then 
	// listen on that IP address and port for incoming connection requests.
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}


	// Normally a server application would be designed to listen for connections from multiple clients. For high-performance servers, multiple threads are commonly used to handle the multiple client connections.
	// There are several different programming techniques using Winsock that can be used to listen for multiple client connections.One programming technique is to create a continuous loop that checks for connection requests using the listen function(see Listening on a Socket).If a connection request occurs, the application calls the accept, AcceptEx, or WSAAccept function and passes the work to another thread to handle the request.Several other programming techniques are possible.
	// Note that this basic example is very simple and does not use multiple threads.The example also just listens for and accepts only a single connection.
	printf("Waiting for client to connect...\n");
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Client connected.\n");		
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection.
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender.
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	printf("Done");

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	getchar();
	return 0;
}
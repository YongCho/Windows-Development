#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")


#define DEFAULTBUFFLEN 512

void printAddrInfo(struct addrinfo *result);
int main(int argc, char *argv[])
{
	WSADATA wsaData;
	int iResult;
	char* serverName;
	char* serverPort;
	if (argc == 3) {
		serverName = argv[1];
		serverPort = argv[2];
	}
	else {
		printf("Usage: main.exe <Server address> <Server port number>\nExample: main.exe 127.0.0.1 22222\n");
		return 0;
	}
	printf("Server Name: %s, Server Port: %s\n", serverName, serverPort);

	// Initialize use of WS2_32.dll. Use WSACleanup() to terminate the use of the dll.
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Call the getaddrinfo function requesting the IP address for the server name.
	// Input parameter 'hints' provides hints about the type of socket the caller supports.
	// Output parameter 'result' is a pointer to a linked list of one or more addrinfo structures 
	// that contains response information about the host.
	iResult = getaddrinfo(serverName, serverPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	printAddrInfo(result);

	printf("Creating socket...\n");
	// Create a SOCKET object called ConnectSocket.
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to an address until one succeeds.
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		printf("Connecting...\n");
		// Create a socket for connecting to server.
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		// Check for errors to ensure that the socket is a valid socket.
		if (ConnectSocket == INVALID_SOCKET) {
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		// Connect to server.	
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}


	int recvbuflen = DEFAULTBUFFLEN;

	char *sendbuf = "XYXYXYXYXYXYXYXY";
	char recvBuf[DEFAULTBUFFLEN];

	// Send an initial buffer.
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes sent: %ld\n", iResult);

	// Shutdown the sending side of the socket since no more data will be sent.
	// The client can still use the ConnectSocket for receiving data.
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Receive until peer closes the connection.
	do {
		iResult = recv(ConnectSocket, recvBuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("Recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);




	printf("Done");
	// Cleanup.
	closesocket(ConnectSocket);
	WSACleanup();
	getchar();
	return 0;
}




// Debug function that prints out the linked list returned by getaddrinfo.
// Reference: getaddrinfo function in MSDN
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
void printAddrInfo(struct addrinfo *result)
{
	// DEBUG
	// Print out all addresses returned from getaddrinfo.
	struct addrinfo *ptr;
	int i = 1;
	struct sockaddr_in  *sockaddr_ipv4;
	struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;
	char ipstringbuffer[46];
	DWORD ipbufferlength = 46;
	INT iRetval;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			InetNtop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, ipbufferlength);
			printf("\tIPv4 address %s\n", ipstringbuffer);
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, ipbufferlength);
			printf("\tIPv6 address %s\n", ipstringbuffer);
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS (NetBIOS)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
		printf("\tSocket type: ");
		switch (ptr->ai_socktype) {
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_socktype);
			break;
		}
		printf("\tProtocol: ");
		switch (ptr->ai_protocol) {
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_protocol);
			break;
		}
		printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
		printf("\tCanonical name: %s\n", ptr->ai_canonname);
	}
}
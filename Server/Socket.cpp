//Socket.cpp
#include "Socket.h"
#include <iostream>

Socket::Socket()
{
    sock = SOCKET_ERROR;
}

Socket::Socket(SOCKET _sock, std::string _dstIP)
{
    sock = _sock;
    dstIP = _dstIP;
}

void Socket::setUpSocket()
{
    WSADATA wsaData;
    if(WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR)
    {
        eventLog("WSAStartup Error. Shutting down server", "0.0.0.0");
        std::cerr << "Socket Initialization: Error with WSAStartup\n";
        system("pause");
        WSACleanup();
        exit(10);
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        eventLog("Socket initialization error. Shutting down Server.", "0.0.0.0");
        std::cerr << "Socket Initialization: Error creating socket"<<std::endl;
        system("pause");
        WSACleanup();
        exit(11);
    }
}

void Socket::SendData(std::string input)
{
    char buffer[STRLEN];
    memset(buffer,0,STRLEN);
    for(unsigned int x = 0; x < input.length(); x++)
    {
        buffer[x] = input[x];
    }
    send(sock, buffer, strlen(buffer), 0);
    eventLog("Sent: " + (std::string)buffer, dstIP);
}

bool Socket::RecvData(std::string &input)
{
	char buffer[STRLEN];
	memset(buffer, 0, STRLEN);
	int i = recv(sock, buffer, STRLEN, 0);
    if (i == SOCKET_ERROR)
    {
        eventLog("Client disconnected unexpectedly.","0.0.0.0");
        std::cout << "Client disconnected unexpectedly\n";
        return false;
    }
    else
    {
        input.reserve(i);
        input = buffer;
        eventLog("Received: " + (std::string)buffer, dstIP);
        return true;
    }
}

void Socket::CloseSocket()
{
    eventLog("Closed Socket", dstIP);
    closesocket(sock);
    sock = INVALID_SOCKET;
}

void Socket::Listen(int numOfConnections)
{
    if (listen(sock, numOfConnections) == SOCKET_ERROR)
    {
        eventLog("ServerSocket: Error listening on socket", dstIP);
        std::cerr << "ServerSocket: Error listening on socket\n";
        system("pause");
        WSACleanup();
        exit(15);
    }
}

SOCKET Socket::Accept(std::string &IP)
{
    sockaddr_in cAddress;
    cAddress.sin_family = AF_INET;
    int size = sizeof(cAddress);
    SOCKET temp = accept(sock,(SOCKADDR*)&cAddress,&size);
    IP = (std::string)inet_ntoa(cAddress.sin_addr);
    return temp;
}

void Socket::Bind(int port)
{
    sockaddr_in myAddress;
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddress.sin_port = htons(port);

    if (bind(sock,(SOCKADDR*) &myAddress, sizeof( myAddress)) == SOCKET_ERROR)
    {
        std::cerr << "Socket: Failed to bind\n";
        eventLog("Socket failed to bind", "0.0.0.0");
        system("pause");
        WSACleanup();
        exit(14);
    }
}

//Name: Connect
//Parameters: 2
//    ip    - the IP the server is connecting to
//    port  - The port that the server will connect to
//Returns: 2
//    True  - Connection Failure
//    False - Connection failure
bool Socket::Connect(std::string ip, int port)
{
    sockaddr_in myAddress;
    char ipAddress[16];
    memset(ipAddress,0,16);
    for(int x = 0;x < 16; x++)
    {
        ipAddress[x] = ip[x];
    }

	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = inet_addr(ipAddress);
	myAddress.sin_port = htons(port);

	if (connect(sock, (SOCKADDR*) &myAddress,sizeof(myAddress)) == SOCKET_ERROR)
	{
	    eventLog("Failed to connect", ipAddress);
		std::cerr << "Failed to connect to: " << ipAddress << ":" << port << std::endl;
		WSACleanup();
		return false;
	}
	return true;
}

SOCKET Socket::getSocket()
{
    return sock;
}

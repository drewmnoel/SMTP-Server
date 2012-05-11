//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: Socket.cpp
//Purpose: Definition of Socket.h

#include "Socket.h"
#include <iostream>

//Name: Socket
//Parameters: none
//Returns: none
//Purpose: Default constructor
Socket::Socket()
{
    sock = SOCKET_ERROR;
}

//Name: Socket
//Parameters: 2
//    _sock  - The socket of the client/DNS server
//    _dstIP - The IP of the client/DNS server
//Returns: none
//Purpose: Initializing constructor. Set up client socket/IP
Socket::Socket(SOCKET _sock, std::string _dstIP)
{
    sock = _sock;
    dstIP = _dstIP;
}

Socket::~Socket()
{
    CloseSocket();
    WSACleanup();
}

//Name: setUpSocket
//Parameters: none
//Returns: none
//Purpose: WSAStartup, Setting up the server socket. Shutdown server on failure
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

//Name: SendData
//Parameters: 1
//    input - The string to be sent to the client/DNS server
//Returns: none
//Purpose: Send a string of data to the client/DNS server
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

//Name: RecvData
//Parameters: 1
//    input - The string received from the client/DNS server
//Returns: 2
//    True - Properly received data
//    False - Did not properly receive data
//Purpose: Receive a string of data from the client/DNS server
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

//Name: CloseSocket
//Parameters: none
//Returns: none
//Purpose: Close the socket associated with the client/DNS server
void Socket::CloseSocket()
{
    eventLog("Closed Socket", dstIP);
    closesocket(sock);
    sock = INVALID_SOCKET;
}

//Name: Listen
//Parameters: 1
//    numOfConnections - The current number of connections the server is maintaining
//Returns: none
//Purpose: Listen for new connections
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

//Name: Accept
//Parameters: 1
//    IP - The IP address that the server will connect to
//Returns: The socket for the Client/DNS server
//Purpose: Create the socket for whoever we will connect to
SOCKET Socket::Accept(std::string &IP)
{
    sockaddr_in cAddress;
    cAddress.sin_family = AF_INET;
    int size = sizeof(cAddress);
    SOCKET temp = accept(sock,(SOCKADDR*)&cAddress,&size);
    IP = (std::string)inet_ntoa(cAddress.sin_addr);
    return temp;
}

//Name: Bind
//Parameters: 1
//    port - The port that the server will use to initialize connections
//Returns: none
//Purpose: Bind the port for connections or close server upon failure
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
//Purpose: Connect to client or DNS Server. Closes connection/socket on failure
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

//Socket.cpp
#include "Socket.h"

Socket::Socket()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		cerr << "Socket Initialization: Error with WSAStartup\n";
		system("pause");
		WSACleanup();
		exit(10);
	}

	//Create a socket
	mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mySocket == INVALID_SOCKET)
	{
		cerr << "Socket Initialization: Error creating socket" << endl;
		system("pause");
		WSACleanup();
		exit(11);
	}

	myBackup = mySocket;
}

Socket::~Socket()
{
	WSACleanup();
}

bool Socket::SendData(char *buffer)
{
	send(mySocket, buffer, strlen(buffer), 0);
	return true;
}

bool Socket::RecvData(char *buffer, int size)
{
	int i = recv(mySocket, buffer, size, 0);
	buffer[i] = '\0';
	return true;
}

void Socket::CloseConnection()
{
	closesocket(mySocket);
	mySocket = myBackup;
}

//Socket.cpp
#include "Socket.h"
#include <cstdio>

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

bool Socket::SendData(string input)
{
    char buffer[STRLEN];
    memset(buffer,0,STRLEN);
    for(unsigned int x = 0;x < input.length();x++)
    {
        buffer[x] = input[x];
    }
    send(mySocket, buffer, strlen(buffer), 0);
    return true;
}

bool Socket::RecvData(char *buffer, int *size)
{
	int i = recv(mySocket, buffer, (*size), 0);
	buffer[i] = '\0';
    printf("%s\n",buffer);
	return true;
}

bool Socket::RecvData(string &input)
{
	char buffer[STRLEN];
	memset(buffer, 0, STRLEN);
	int i = recv(mySocket, buffer, STRLEN, 0);
    input.reserve(i);
    input = buffer;
	return true;
}

void Socket::CloseConnection()
{
	closesocket(mySocket);
	mySocket = myBackup;
}

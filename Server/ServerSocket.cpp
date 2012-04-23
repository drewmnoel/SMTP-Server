//Socket.cpp
#include "ServerSocket.h"
ServerSocket::ServerSocket()
{
	done = false;
	authed = false;
}

bool ServerSocket::Auth(void)
{
	SendData("LOGIN");

	char buffer[STRLEN];
	int i = recv(mySocket, buffer, STRLEN, 0);
	buffer[i] = '\0';
	if (strcmp(buffer, "ADMIN") == 0)
	{
		authed = true;
		SendData("WELCOME");
		return true;
	}

	SendData("UNWELCOME");
	return false;
}

void ServerSocket::Bind(int port)
{
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = inet_addr("0.0.0.0");
	myAddress.sin_port = htons(port);

	if (bind(mySocket, (SOCKADDR*) &myAddress,
			sizeof(myAddress)) == SOCKET_ERROR)
	{
		cerr << "ServerSocket: Failed to connect\n";
		system("pause");
		WSACleanup();
		exit(14);
	}
}

void ServerSocket::GetAndSendMessage()
{
	char message[STRLEN];

	cout << ">>> ";

	cin.get(message, STRLEN);

	cin.ignore(1);

	SendData(message);
}

void ServerSocket::Listen()
{
	if (listen(mySocket, 1) == SOCKET_ERROR)
	{
		cerr << "ServerSocket: Error listening on socket\n";
		system("pause");
		WSACleanup();
		exit(15);
	}

	sockaddr_in client_info;
	int size = sizeof(client_info);

	acceptSocket = SOCKET_ERROR;

	while ((signed int)acceptSocket == SOCKET_ERROR)
	{
		acceptSocket = accept(myBackup, (sockaddr*) &client_info, &size);
	}

	clientPort = ntohs(client_info.sin_port);
	clientAddress = inet_ntoa(client_info.sin_addr);

	mySocket = acceptSocket;
}

void ServerSocket::RecvAndDisplayMessage()
{
	char message[STRLEN];
	memset(message, 0, STRLEN);
	RecvData(message, STRLEN);
}

bool ServerSocket::RecvData(char *buffer, int size)
{
	int i = recv(mySocket, buffer, size, 0);
	buffer[i] = '\0';

	cout << "<<< " << buffer;

	// Convert to lower-case to compare
	for (int j = 0; j < i; j++)
		buffer[j] = tolower(buffer[j]);

	// Process commands
	if (strncmp(buffer, "quit", 4) == 0)
	{
		// Client wants us to go away!
		done = true;
	}
	return true;
}

int ServerSocket::SendData(const char* buffer)
{
	return send(mySocket, buffer, strlen(buffer), 0);
}

int ServerSocket::SendData(char* buffer, int size)
{
	return send(mySocket, buffer, size, 0);
}

int ServerSocket::SendData(int value)
{
	char buffer[STRLEN];
	memset(buffer, 0, STRLEN);
	sprintf(buffer, "%d", value);
	return SendData(buffer);
}

int ServerSocket::SendData(string value)
{
	char buffer[STRLEN];
	memset(buffer, 0, STRLEN);
	sprintf(buffer, "%s", value.c_str());
	return SendData(buffer);
}

void ServerSocket::StartHosting(int port)
{
	Bind(port);
	Listen();
}

bool ServerSocket::isOver()
{
	return (done || !authed);
}

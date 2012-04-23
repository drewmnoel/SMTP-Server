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
	if (strncmp(buffer, "list", 4) == 0)
	{
		// Client wants a dir list!
		// We'll just send them the current one
		dirList("./");
	}
	else if (strncmp(buffer, "send", 4) == 0)
	{
		// Client wants a file!
		sendFile(buffer);
	}
	else if (strncmp(buffer, "quit", 4) == 0)
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

void ServerSocket::dirList(string dir)
{
	string theList;

	theList = list(dir).c_str();

	// Sends the length of the list
	SendData(theList.length());

	// Waits for an "OK"
	RecvAndDisplayMessage();

	unsigned long sent = 0;
	while (sent != theList.length())
	{
		sent += SendData(theList.substr(sent, STRLEN));
	}
}

bool ServerSocket::isOver()
{
	return (done || !authed);
}

void ServerSocket::sendFile(string filename)
{
	// Chop off the "send " part to get the filename
	filename = filename.substr(5, filename.length());

	// Chop off the newline character
	//filename = filename.substr(0, filename.length() - 1);

	// Lock in the current directory
	filename = "./" + filename;

	ifstream fin(filename.c_str(), ios::binary);

	// Send 0 (could not open)
	if (!fin.is_open())
	{
		SendData(0);
		return;
	}

	// Get the file size to send
	fin.seekg(0, ios::end);
	unsigned long size = fin.tellg();
	fin.seekg(0, ios::beg);

	// Send the size and wait for an OK
	SendData(size);
	RecvAndDisplayMessage();

	// UNLEASH THE KRAKEN
	char buffer[STRLEN];
	unsigned long sent = 0;

	// Send as much as we can at once
	while (sent + STRLEN <= size)
	{
		fin.read(buffer, STRLEN);

		sent += SendData(buffer, STRLEN);
	}
	if (sent != size)
	{
		// Send those pesky remaining bytes... THE HARD WAY
		fin.read(buffer, size - sent);
		SendData(buffer, size - sent);
	}
}

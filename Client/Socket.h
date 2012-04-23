//Socket.h
#pragma once
#include <iostream>
#include "WinSock2.h"

using namespace std;

const int STRLEN = 256;

class Socket
{
protected:
	WSADATA wsaData;
	SOCKET mySocket;
	SOCKET myBackup;
	SOCKET acceptSocket;
	sockaddr_in myAddress;
	long clientPort, port;
	char* clientAddress;
	char *address;
public:
	Socket();
	~Socket();
	bool SendData(char*);
	bool RecvData(char*, int);
	int getPort();
	char* getAddr();
	void CloseConnection();
	void GetAndSendMessage();
};

//Socket.h
#pragma once
#include <ctime>
#include <iostream>
#include <fstream>
#include "WinSock2.h"
#include "Socket.h"

using namespace std;

string list(string);

class ServerSocket: public Socket
{
private:
	bool done;
	bool authed;
public:
	ServerSocket();

	bool Auth(void);
	void Bind(int port);
	void GetAndSendMessage();
	void Listen();
	void RecvAndDisplayMessage();
	bool RecvData(char *buffer, int size);
	int SendData(const char*);
	int SendData(char*, int);
	int SendData(int);
	int SendData(string);
	void StartHosting(int port);
	void dirList(string);
	bool isOver();
	void sendFile(string);
};

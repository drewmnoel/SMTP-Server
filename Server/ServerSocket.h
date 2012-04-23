//Socket.h
#pragma once
#include <ctime>
#include <iostream>
#include <fstream>
#include <windows.h>
#include "Socket.h"

using namespace std;

class ServerSocket: public Socket
{
private:
	bool done;
public:
	ServerSocket();

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
	bool isOver();
};
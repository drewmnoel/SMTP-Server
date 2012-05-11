//Socket.h
#pragma once
#include <string>
#include "WinSock2.h"
#include "iniReader.h"

void eventLog(std::string, std::string);
const int STRLEN = 256;

class Socket
{
private:
	SOCKET sock;
	std::string dstIP;
public:
	Socket();
	~Socket();
	Socket(SOCKET,std::string);
	void setUpSocket();
	void SendData(std::string);
	bool RecvData(std::string&);
	void CloseSocket();
	bool Connect(std::string,int);
	void Bind(int);
	void Listen(int);
	SOCKET Accept(std::string&);
	SOCKET getSocket();
};

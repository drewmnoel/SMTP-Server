//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: Socket.h
//Purpose: Declaration of the Socket class

#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include "iniReader.h"

//Logging function
void eventLog(std::string, std::string);

//The max allowed length of char[]
const int STRLEN = 256;

class Socket
{
private:
	SOCKET sock;
	std::string dstIP;

public:
    //Constructors
	Socket();
	Socket(SOCKET,std::string);

	//Server socket setup
	void setUpSocket();
	void Bind(int);
	void Listen(int);
    SOCKET Accept(std::string&);

    //Connecting to client/DNS Server
	bool Connect(std::string,int);

	//Accessor for socket
    SOCKET getSocket();

	//Sending/Receiving data
	void SendData(std::string);
	bool RecvData(std::string&);

    //Closing a socket
	void CloseSocket();

};

//Socket.h
#pragma once
#include <string>
#include "WinSock2.h"

const int STRLEN = 256;

SOCKET setUpSocket();
void SendData(SOCKET,std::string);
bool RecvData(SOCKET,std::string&);
void CloseSocket(SOCKET&);
bool Connect(SOCKET,std::string,int);
void Bind(SOCKET,int);
void Listen(SOCKET, int);
SOCKET Accept(SOCKET,std::string&);

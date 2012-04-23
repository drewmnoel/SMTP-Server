//ClientSocket.h
#pragma once
#include <windows.h>
#include "Socket.h"
#include <string>

class ClientSocket: public Socket
{
public:
	bool ConnectToServer(const char *ipAddress, int port);
	bool GetFile(std::string);
	bool GetDir();
	bool Login();
	bool Command();
};

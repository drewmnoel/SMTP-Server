//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ClientThread.cpp
//Purpose: Declaration of the ClientThread class

#pragma once
#include "Socket.h"
#include <windows.h>
#include <string>

struct comm
{
	SOCKET dns;
	SOCKET client;
	std::string clientInfo;
};

class ClientThread
{
public:
	ClientThread();
	void run(LPVOID);
	void checkSource();
private:
	bool forwarded;
	comm *Socks;
	Socket *dns;
	Socket *client;
};

extern HANDLE dnsLock;
extern HANDLE fileLock;
extern HANDLE eventLock;
extern std::string DNS_IP;
extern int Message_Queue;
extern std::string registeredName;

DWORD WINAPI runClient(LPVOID toPass);

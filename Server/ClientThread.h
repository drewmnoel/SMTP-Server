//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ClientThread.cpp
//Purpose: Declaration of the ClientThread class

#pragma once
#include "Socket.h"
#include <regex>
#include <string>
using std::regex;

struct comm
{
	SOCKET dns;
	SOCKET client;
	std::string clientInfo;
};

class ClientThread
{
public:
    //Default constructor
	ClientThread();

	//Setup the DNS socket and perform client operations
	void run(LPVOID);

	//Method to make sure the IP given is a server
	void checkSource();
private:
	bool forwarded;
	comm *Socks;
	Socket *dns;
	Socket *client;
};

//Start mutexes
extern HANDLE dnsLock;
extern HANDLE fileLock;
extern HANDLE eventLock;
//End mutexes

//IP of DNS server
extern std::string DNS_IP;
//The message queue number
extern int Message_Queue;
//The name registered by the server
extern std::string registeredName;

//The method that runs in the client thread
DWORD WINAPI runClient(LPVOID toPass);

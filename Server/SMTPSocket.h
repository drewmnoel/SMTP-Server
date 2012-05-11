#pragma once
#include <regex>
#include "Socket.h"
#include <iostream>
using namespace std;

class SMTPSocket : protected Socket
{
private:
	std::string DNS_NAME;
	std::string DNS_NAME_BACKUP;
	
	int DNS_PORT;
	int PORT;
public:
	void run();
	
protected:
	void setUp();
	SOCKET dnsRegister();
};

extern std::string DNS_IP;
extern std::string registeredName;
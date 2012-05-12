//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: SMTPSocket.cpp
//Purpose: Declaration of the SMTPSocket class

#pragma once
#include <regex>
#include "Socket.h"

class SMTPSocket : protected Socket
{
private:
    //The names we attempt to register
	std::string DNS_NAME;
	std::string DNS_NAME_BACKUP;

    //Ports used by the DNS server and our SMTP Server
	int DNS_PORT;

public:
    //Method to setup client connection and SMTP session
	void run();

protected:
    //Method to setup our server socket
	void setUp();

	//Method to register our domain name witht he DNS server
	SOCKET dnsRegister();

};

//The IP of the DNS server
extern std::string DNS_IP;

//The name that we successfully registered with the DNS server or a failure default
extern std::string registeredName;

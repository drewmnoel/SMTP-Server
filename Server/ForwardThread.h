//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ForwardThread.h
//Purpose: Declaration of the ForwardThread class

#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include "ClientThread.h"
using std::string;
using std::fstream;
using std::ios;
using std::cerr;
using std::endl;
using std::stringstream;
class ForwardThread
{
public:
    //Default constructor
	ForwardThread();

	//Method to setup DNS socket
	void run(LPVOID);

	//Method to perform a DNS lookup on an IP
	string dnsLookup(string);

private:
    //Struct containing client/IP socket and DNS sokcet
	comm *Socks;
	Socket *dns;
	Socket relay;

	//Whether or not the chosen relay is online/working/etc.
	bool validRelay;

};

//Function to perform the file handling thread
DWORD WINAPI runFile(LPVOID);

//The name that we successfully registered with the DNS server or a failure default
extern std::string registeredName;
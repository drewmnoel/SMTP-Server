//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ForwardThread.h
//Purpose: Declaration of the ForwardThread class

#pragma once
#include <windows.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "ClientThread.h"
using namespace std;

class ForwardThread
{
public:
    //Default constructor
	ForwardThread();

	//Method to setup DNS socket
	void run(LPVOID);

	//Method to perform a DNS lookup on an IP
	void dnsLookup(string);

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

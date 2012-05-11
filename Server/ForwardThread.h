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
	ForwardThread();
	void run(LPVOID);
	void dnsLookup(string);
private:
	comm *Socks;
	Socket *dns;
	Socket *relay;
	bool validRelay;
};

DWORD WINAPI runFile(LPVOID);
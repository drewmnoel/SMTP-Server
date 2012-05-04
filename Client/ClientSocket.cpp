//ClientSocket.cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "ClientSocket.h"

bool ClientSocket::ConnectToServer(const char *ipAddress, int port)
{
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = inet_addr(ipAddress);
	myAddress.sin_port = htons(port);

	if (connect(mySocket, (SOCKADDR*) &myAddress,
			sizeof(myAddress)) == SOCKET_ERROR)
	{
		std::cerr << "Failed to connect to: " << ipAddress << std::endl;
		WSACleanup();
		return false;
	}
	return true;
}

bool ClientSocket::ConnectToServer(std::string ip, int port)
{
    char ipAddress[16];
    memset(ipAddress,0,16);
    for(int x = 0;x < 16; x++)
    {
        ipAddress[x] = ip[x];
    }

	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = inet_addr(ipAddress);
	myAddress.sin_port = htons(port);

	if (connect(mySocket, (SOCKADDR*) &myAddress,
			sizeof(myAddress)) == SOCKET_ERROR)
	{
		std::cerr << "Failed to connect to: " << ipAddress << std::endl;
		WSACleanup();
		return false;
	}
	return true;
}

bool ClientSocket::Command()
{
	std::string command;
	std::cout << "Command: ";
	std::getline(std::cin, command);
	for (unsigned int x = 0; x < command.substr(0, command.find(" ")).length(); x++)
	{
		command[x] = tolower(command[x]);
	}
	if (command.substr(0, command.find(" ")) == "quit")
	{
		//SendData("quit");
		return true;
	}
	else
	{
		cout << "Unknown Command\n";
		return false;
	}

}

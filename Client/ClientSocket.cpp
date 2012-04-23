//ClientSocket.cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <fstream>
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

bool ClientSocket::GetFile(std::string input)
{

	// Chop off the "send " part to get the filename
	std::string filename = input.substr(5);

	// Chop off the newline character
	//filename = filename.substr(0, filename.length() - 1);

	// Lock in the current directory
	filename = "./" + filename;

	fstream file(filename.c_str(), ios::out | ios::binary);

	char buffer[STRLEN];
	int totalsize = 0;
	int totalrecv = 0;

	//get length of all of the list
	SendData((char*) input.c_str());
	RecvData(buffer, sizeof(buffer));
	totalsize = atoi(buffer);
	SendData("OK");

	while (totalrecv + STRLEN <= totalsize)
	{
		totalrecv += RecvData(buffer, STRLEN);
		file.write(buffer, STRLEN);
	}
	// Send those pesky remaining bytes... THE HARD WAY
	if (totalrecv != totalsize)
	{
		RecvData(buffer, STRLEN);
		file.write(buffer, totalsize - totalrecv);
	}
	//fin.read(buffer, size - sent);
	//SendData(buffer, size - sent);

	//for(int x = 0;x < totalsize;x += STRLEN){
	//    recsize = RecvData(buffer,sizeof(buffer));
	//    std::cout << recsize << std::endl;
	//    //std::cout << totalrecv << std::endl;
	//    file.write(buffer,recsize);
	//}
	file.close();

	return true;
}

bool ClientSocket::GetDir()
{
	char buffer[STRLEN];
	int totalsize = 0;
	int totalrecv = 0;

	//get length of all of the list
	SendData("LIST");
	RecvData(buffer, sizeof(buffer));
	totalsize = atoi(buffer);
	SendData("OK");

	for (int x = 0; x <= (totalsize + (totalsize % STRLEN)); x += STRLEN)
	{
		totalrecv += RecvData(buffer, sizeof(buffer));
		std::cout << buffer;
	}
	//std::cout << totalrecv;

	return true;
}

bool ClientSocket::Login()
{
	char buffer[STRLEN];

	RecvData(buffer, sizeof(buffer));
	if (strcmp(buffer, "LOGIN") == 0)
	{
		std::cout << "Authorization Code:";
		std::cin >> buffer;
		cin.ignore(1000, '\n');
		SendData(buffer);
	}

	RecvData(buffer, sizeof(buffer));
	if (strcmp(buffer, "WELCOME") == 0)
	{
		std::cout << "Login Sucessful\n";
		return true;
	}
	else
	{
		std::cout << "Login Unsucessful\n";
		return false;
	}
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
	if (command.substr(0, command.find(" ")) == "list")
	{
		//std::cout << "LIST\n";
		GetDir();
		return false;
	}
	else if (command.substr(0, command.find(" ")) == "send")
	{
		//std::cout << "SEND\n";
		GetFile(command);
		return false;
	}
	else if (command.substr(0, command.find(" ")) == "quit")
	{
		SendData("quit");
		return true;
	}
	else
	{
		cout << "Unknown Command\n";
		return false;
	}

}

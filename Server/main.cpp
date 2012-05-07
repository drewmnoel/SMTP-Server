#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <sstream>
#include <regex>
#include <ctime>
#include <fstream>
#include "Socket.h"
#include "iniReader.h"

//#define MESSAGE_SIZE

using namespace std;

SOCKET dnsRegister(string, string, string);
DWORD WINAPI ClientThread(LPVOID lpParam);
DWORD WINAPI fileThread(LPVOID lpParam);
void eventLog(string info, string ip);

HANDLE dnsLock;
HANDLE fileLock;

string registered_name;
int Message_Queue = 0;

struct clientAndDns
{
	SOCKET client;
	string cIP;
	SOCKET dns;
};

string DNS_NAME_BACKUP;
string DNS_NAME;
string DNS_IP;
int DNS_PORT;
int PORT;

int main()
{
	parseIniFile("sample.ini");

	DNS_NAME_BACKUP = getOptionToString("DNS_NAME_BACKUP");
	DNS_NAME = getOptionToString("DNS_NAME");
	DNS_IP = getOptionToString("DNS_IP");
	DNS_PORT = getOptionToInt("DNS_PORT");
	PORT = getOptionToInt("PORT");

	//register dns name and keep socket open
	SOCKET dnsSocket = dnsRegister(DNS_IP, DNS_NAME, DNS_NAME_BACKUP);

	//create mutex on dns socket
	dnsLock = CreateMutex(NULL, FALSE, NULL);
	if (dnsLock == NULL)
	{
		return (1);
	}
	eventLog("Created DNS mutex", "0.0.0.0");
    	
	fileLock = CreateMutex(NULL, FALSE, NULL);
	if (fileLock == NULL)
	{
		return (1);
	}
	eventLog("Created file mutex", "0.0.0.0");
	
	//Set up a listening socket
	SOCKET server = setUpSocket();
	Bind(server, PORT);
	Listen(server, 99);
    eventLog("Successfully set up server socket. Listening", "0.0.0.0");
    
	HANDLE hThread;
	DWORD dwThreadId;

	//Set up a socket to the DNS server
	clientAndDns justDns;
	justDns.dns = dnsSocket;
	HANDLE readFile = CreateThread(NULL, 0, fileThread, (LPVOID) &justDns, 0,
			&dwThreadId);

	while (1)
	{
		//Bind server socket
		clientAndDns temp;
		temp.client = Accept(server, temp.cIP);
		temp.dns = dnsSocket;

		//Set up a client thread
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID) &temp, 0,
				&dwThreadId);
		if (hThread == NULL)
		{
            eventLog("CreateThread() failed: %d\n" + (int) GetLastError(), temp.cIP);
			printf("CreateThread() failed: %d\n", (int) GetLastError());
			break;
		}
		else
		    eventLog("Started thread " + GetCurrentThreadId(), temp.cIP);
		CloseHandle(hThread);
	}

	system("pause");
	return 0;
}

SOCKET dnsRegister(string ip, string name, string backup)
{
	SOCKET temp;
	temp = setUpSocket();
	Connect(temp, ip, DNS_PORT);
	string response;

	//Send domain request to DNS server
	SendData(temp, "iam " + name);
	eventLog("Sent iam " + name + " to dns server", ip);
	RecvData(temp, response);
	if (response == "5")
	{
		//Send backup request to DNS server
		cout << "Name already taken. Trying backup name" << endl;
		eventLog("Name \"" + name + "\" already taken. Trying backup name", "0.0.0.0");
		response = "";
		SendData(temp, "iam " + backup);
		RecvData(temp, response);
		if (response == "5")
		{
		    //Kill if both names are taken
			cout << "Both names taken quitting server" << endl;
			eventLog("Both names taken. Quitting server.", "0.0.0.0");
			exit(1);
		}
		else
		{
			//Register backup
            cout << "Using backup name" << endl;
            eventLog("Using backup name", "0.0.0.0");
			registered_name = DNS_NAME_BACKUP;
			return temp;
		}
	}
	//Register primary
	cout << "First name registered successfully" << endl;
    eventLog("First name registered successfully", "0.0.0.0");
	registered_name = DNS_NAME;
	return temp;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	fstream fin;
	clientAndDns *temp = (clientAndDns*) lpParam;
	SOCKET client = temp->client;
	SOCKET dns = temp->dns;
	string clientIP = temp->cIP;
	regex from("MAIL FROM:<[\\w.]+@[\\w.]+>");
	regex to("RCPT TO:<[\\w.]+@[\\w.]+>");
    bool validRelay;
	stringstream completeMessage;

	DWORD dwWaitResult = WaitForSingleObject(dnsLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		//Test IP against DNS
		SendData(dns, "who " + clientIP);
		eventLog("Sent \"who " + clientIP + "\""), DNS_IP);
		string response;
		RecvData(dns, response);
		bool forwarded = false;

		//If 0, it's from a server, if not, it's from a client
		if (response == "0")
		{
            eventLog("Message is from a server",clientIP);
			forwarded = true;
		}
		if (forwarded == false)
		    eventLog("Message is from a client",clientIP);

		printf("message is from a %s\n", (forwarded) ? "server" : "client");
	}
	ReleaseMutex(dnsLock);

	//here is where we send and recieve data from the client
	SendData(client, "220 " + registered_name + " ESMTP Postfix");
	eventLog("220 " + registered_name + " ESMTP Postfix",clientIP);
    string data = "";
	RecvData(client, data);
	if (data.substr(0, 4) != "HELO")
	{
		SendData(client, "221");
		return 0;
	}
	SendData(client, "250 Hello " + data.substr(5) + ", I am glad to meet you");
	eventLog("250 Hello " + data.substr(5) + ", I am glad to meet you", clientIP);
    RecvData(client, data);
	while (data != "DATA")
	{
		if (regex_match(data, from))
		{
			completeMessage << data << endl;
			eventLog("FROM 250 OK", clientIP);
			SendData(client,"250 OK");
		}
		else if (regex_match(data, to))
		{
			completeMessage << data << endl;
			eventLog("RCPT 250 OK", clientIP);
			SendData(client, "250 OK");
		}
		else
		{
			SendData(client, "500 Command Syntax Error");
			eventLog("500 Command Syntax Error", clientIP);
		}
		RecvData(client, data);
	}
	SendData(client, "354 End data with <CR><LF>.<CR><LF>");
	eventLog("354 End data with <CR><LF>.<CR><LF>",clientIP);
	completeMessage << data << endl;
	while (data != ".")
	{
        eventLog("Receiving data \"" + data + "\"", clientIP);
		RecvData(client, data);
		completeMessage << data;
	}
	completeMessage << endl << "." << endl;
	SendData(client, "250 OK: queued as " + ++Message_Queue);
	RecvData(client, data);
	if (data == "QUIT")
	{
		SendData(client, "221 BYE");
		eventLog("221 BYE", clientIP);
		return 0;
	}
	//end send receive area

	dwWaitResult = WaitForSingleObject(fileLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		//write the email down
		fin.open("master_baffer.woopsy", ios::app);
		fin << completeMessage.str() << endl;
		fin.close();
	}
	ReleaseMutex(fileLock);

	return 0;
}

DWORD WINAPI fileThread(LPVOID lpParam)
{
	clientAndDns *temp2 = (clientAndDns*) lpParam;
	SOCKET dns = temp2->dns;
	string clientData, userName, user;
	stringstream toFile;
	bool forward;
	bool validRelay;
	SOCKET relay;

	DWORD dwWaitResult = WaitForSingleObject(fileLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		//we have control of the file now to read
		fstream fin("master_baffer.woopsy", ios::in);
		getline(fin, clientData);
		if (clientData == "True")
			forward = true;
		else
			forward = false;
		toFile << clientData << endl;

		getline(fin, clientData);
		toFile << clientData << endl;

		while (clientData != "DATA")
		{
			getline(fin, clientData);
			toFile << clientData << endl;
		}
		while (clientData != ".")
		{
			getline(fin, clientData);
			toFile << clientData << endl;
		}
		fin.close();
		clientData = "";

		if (!forward)
		{
			if (userName == "alex" || userName == "dan" || userName == "drew"
					|| userName == "scott")
			{
				fin.open((userName + ".txt").c_str(), ios::app);
				fin << toFile.str();
				fin.close();
			}
			else
			{
				cerr << "No user \"" << userName << "\" exists on this server."
						<< endl;
			}
		}

		//We are forwarding the message
		else
		{
			DWORD dwWaitResult = WaitForSingleObject(dnsLock, INFINITE);
			if (dwWaitResult == WAIT_OBJECT_0)
			{
				//dns stuff after we parse the to line
				string forwardDomain;
				//TODO: Fill in forwardDomain
				eventLog("Sent \"who " + forwardDomain + "\"", DNS_IP);
				SendData(dns, "who " + forwardDomain);
				string response;
				RecvData(dns, response);
			    if (response == "3")
                {
               		cout << "Domain not registered\n";
               		//*Put it at the end of the file
                    validRelay = false;
                    return 0;
            	}
			    else if (response == "4")
                {
              		cout << "Bad command\n";
              		validRelay = false;
              		return 0;
            	}
            	else
                {
            	    SOCKET relay;
                    if (!Connect(relay,response,PORT)) {
                 	   cout << "Connection to relay failed\n";
                 	   validRelay = true;
                 	   return 0;
                 	}
                 	else
                 	    validRelay = false;
            	}
                //check if its an ip or an invalid address and cannot sent it
            }
            ReleaseMutex(dnsLock);

            //now that we have an ip we can continue or we can end it here if invalid or whatever
            if (validRelay)
            {
                while (clientData != ".")
                {
                    getline(toFile, clientData);
                    SendData(relay, clientData + "\n");
                }
            }
		}
	}
	ReleaseMutex(fileLock);
//	return;
}

//name: eventLog
//Parameters: Info to be logged
//Returns: none
//Purpose: Keep a log of all server activities
void eventLog(string info, string ip)
{
    //time_t dia; //A buffer to store the date
    struct tm * timeinfo;
    time_t hora; //A buffer to store the time
    fstream fout;
    fout.open("server_log.txt", ios::app);
    time(&hora)
    timeinfo = localtime(&hora);
    if (info != "")
    {
        fout.open("server_log.csv", ios::app);
        fout << "\""
             << (string)asctime(timeinfo)
             << "\",\""
             << ip
             << "\",\""
             << info
             << "\"\n";
        fout.close();
    }
}

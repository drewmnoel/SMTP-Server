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

//start prototypes
SOCKET dnsRegister(string, string, string);
DWORD WINAPI ClientThread(LPVOID lpParam);
DWORD WINAPI fileThread(LPVOID lpParam);
void eventLog(string info, string ip);
//end prototypes

//start Mutexs
HANDLE dnsLock;
HANDLE fileLock;
HANDLE eventLock;
//end mutexs

//star global vars
string registered_name;
int Message_Queue = 0;
string DNS_NAME_BACKUP;
string DNS_NAME;
string DNS_IP;
int DNS_PORT;
int PORT;
//end global vars

//client and file thread arg struct
struct clientAndDns
{
	SOCKET client;
	string cIP;
	SOCKET dns;
};

int main()
{
    //start config
	parseIniFile("sample.ini");
    eventLog("Server settings loaded from config", "0.0.0.0");
	DNS_NAME_BACKUP = getOptionToString("DNS_NAME_BACKUP");
	DNS_NAME = getOptionToString("DNS_NAME");
	DNS_IP = getOptionToString("DNS_IP");
	DNS_PORT = getOptionToInt("DNS_PORT");
	PORT = getOptionToInt("PORT");
	//end config

	//register dns name
	SOCKET dnsSocket = dnsRegister(DNS_IP, DNS_NAME, DNS_NAME_BACKUP);

    //start mutex creation
	eventLock = CreateMutex(NULL, FALSE, NULL);
	if (eventLock == NULL)
	{
		return (1);
	}
	eventLog("Created eventlog mutex", "0.0.0.0");

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
    //end mutex creation

	//start server startup
	SOCKET server = setUpSocket();
	Bind(server, PORT);
	Listen(server, 99);
    eventLog("Successfully set up server socket. Listening", "0.0.0.0");
    //end server startup

	//create struct to pass the dns socket to the master buffer reader thread
	clientAndDns justDns;
	justDns.dns = dnsSocket;
	//CreateThread(NULL, 0, fileThread, (LPVOID) &justDns, 0, &dwThreadId);

	DWORD dwThreadId;

	while (1)
	{
		//create struct to pass to client thread
		clientAndDns temp;
		temp.client = Accept(server, temp.cIP);
		temp.dns = dnsSocket;

		//Set up a client thread
		if(CreateThread(NULL, 0, ClientThread, (LPVOID) &temp, 0, &dwThreadId) == NULL)
		{
            eventLog("CreateThread() failed: " + (int)GetLastError(), temp.cIP);
			printf("CreateThread() failed: %d\n", (int)GetLastError());
			break;
		}
		else
		{
		    eventLog("Client accepted", temp.cIP);
		}
	}
    //a thread couldnt be created so we just killed the whole server
	system("pause");
	return 0;
}

//register the dns name with the dns server
SOCKET dnsRegister(string ip, string name, string backup)
{
	SOCKET temp;
	temp = setUpSocket();
	Connect(temp, ip, DNS_PORT);
	string response;

	//Send domain request to DNS server
	SendData(temp, "iam " + name);
	eventLog("Sent iam " + name + " to dns server", ip);
	if(!RecvData(temp, response))
	{
	    eventLog("Quitting server dns connection was severed", "0.0.0.0");
	    exit(1);
	}
    if (regex_match(response, (regex)"5\n*"))
    {
        //request a backup name from the DNS server
        cout << "Name already taken. Trying backup name" << endl;
        eventLog("Name \"" + name + "\" already taken. Trying backup name", "0.0.0.0");
        SendData(temp, "iam " + backup);
        if(!RecvData(temp, response))
        {
            eventLog("Quitting server dns connection was severed", "0.0.0.0");
            exit(1);
        }
        if (regex_match(response, (regex)"5\n*"))
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
    //put the struct passed into vars we can use
	clientAndDns *temp = (clientAndDns*) lpParam;
	SOCKET client = temp->client;
	SOCKET dns = temp->dns;
	string clientIP = temp->cIP;

	//other vars we need
    bool validRelay;
	stringstream completeMessage;
	string response;
	bool forwarded = false;

	if (WaitForSingleObject(dnsLock, INFINITE) == WAIT_OBJECT_0)
	{
		//Test IP against DNS
		SendData(dns, "who " + clientIP);
		eventLog("Sent \"who " + clientIP + "\"", DNS_IP);

		if(!RecvData(dns, response))
		{
		    eventLog("Quitting server dns connection was severed", "0.0.0.0");
		    return 0;
		}
        //If 0, it's from a server, if not, it's from a client
        if (regex_match(response, (regex)"0\n*"))
        {
            eventLog("Message is from a server",clientIP);
            forwarded = true;
        }
        else
        {
            eventLog("Message is from a client",clientIP);
        }
        printf("message is from a %s\n", ((forwarded) ? "server" : "client"));
	}
	ReleaseMutex(dnsLock);

	//here is where we send and recieve data from the client
	SendData(client, "220 " + registered_name + " ESMTP Postfix");
	eventLog("220 " + registered_name + " ESMTP Postfix",clientIP);
    string data = "";
	if(!RecvData(client, data))
	{
	    eventLog("client disconnect",clientIP);
        return 0;
	}
    if (!regex_match(data,(regex)"HELO .*\n*"))
    {
        SendData(client, "221 Closing Transmission Channel");
        return 0;
    }
    SendData(client, "250 Hello " + ((data[(data.length()-1)] = '\n') ? data.substr(5,(data.length()-6)) : data.substr(5)) + ", I am glad to meet you");
    eventLog("Sent 250 Hello " + ((data[(data.length()-1)] = '\n') ? data.substr(5,(data.length()-6)) : data.substr(5)) + ", I am glad to meet you", clientIP);
    if(!RecvData(client, data))
    {
        eventLog("client disconnect",clientIP);
        return 0;
    }
    while (!regex_match(data,(regex)"DATA\n*"))
    {
        if (regex_match(data, (regex)"MAIL FROM:<.+@.+>\n*"))
        {
            completeMessage << (regex_match(data,(regex)"*\n") ? data : data + "\n");
            eventLog("Sent FROM 250 OK", clientIP);
            SendData(client,"250 OK");
        }
        else if (regex_match(data,(regex)"RCPT TO:<.+@.+>\n*"))
        {
            completeMessage << (regex_match(data,(regex)"*\n") ? data : data + "\n");
            eventLog("Sent RCPT 250 OK", clientIP);
            SendData(client, "250 OK");
        }
        else
        {
            SendData(client, "500 Command Syntax Error");
            eventLog("Sent 500 Command Syntax Error", clientIP);
        }
        if(!RecvData(client, data))
        {
            eventLog("client disconnect",clientIP);
            return 0;
        }
    }
    SendData(client, "354 End data with <CR><LF>.<CR><LF>");
    eventLog("Sent 354 End data with <CR><LF>.<CR><LF>",clientIP);
    completeMessage << (regex_match(data,(regex)"*\n") ? data : data + "\n");
    while (!regex_match(data,(regex)"\\.\n*"))
    {
        eventLog("Received data \"" + data + "\"", clientIP);
        if(!RecvData(client, data))
        {
            eventLog("client disconnect",clientIP);
            return 0;
        }
        completeMessage << data;
    }
    SendData(client, "250 OK: queued as " + (++Message_Queue));
    eventLog("Sent 250 OK: queued as " + (++Message_Queue), clientIP);

    //write dat message down
    if (WaitForSingleObject(fileLock, INFINITE) == WAIT_OBJECT_0)
	{
		//write the email down
		fin.open("master_baffer.woopsy", ios::out | ios::app);
		fin << completeMessage.str() << endl;
		fin.close();
	}
	ReleaseMutex(fileLock);

    //now look for the quit
    while(!regex_match(data,(regex)"QUIT\n*"))
    {
        if(!RecvData(client, data))
        {
            eventLog("client disconnect after message queued",clientIP);
            return 0;
        }
    }
    SendData(client, "221 BYE");
    eventLog("Sent 221 BYE", clientIP);
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
		//Get the first line which tells us if it's a client or not
		getline(fin, clientData);
		if (clientData == "True")
			forward = true;
		else
			forward = false;
		//Store in the stringstream
		toFile << clientData << endl;
        //Get the next line which would be the TO & store
		getline(fin, clientData);
		toFile << clientData << endl;

        //Keep reading in until you get to DATA
		while (clientData != "DATA")
		{
			getline(fin, clientData);
			toFile << clientData << endl;
		}
		//Keep reading in until the end of message marker
		while (clientData != ".")
		{
			getline(fin, clientData);
			toFile << clientData << endl;
		}
		//Close the file and clear the clientData buffer
		fin.close();
		clientData = "";

        //The user is local
		if (!forward)
		{
			if (userName == "alex" || userName == "dan" || userName == "drew"
					|| userName == "scott")
			{
                //Open the correct user file and append the string stream into it
				fin.open((userName + ".txt").c_str(), ios::app);
				fin << toFile.str();
                eventLog("Stored entire message in \"" + userName + ".txt\"", "0.0.0.0");
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

				eventLog("Attempting to forward message", response);
			    if (response == "3")
                {
                    eventLog("Domain not registered", "0.0.0.0");
               		cout << "Domain not registered\n";
               		//*Put it at the end of the file
                    validRelay = false;
                    return 0;
            	}
			    else if (response == "4")
                {
                    eventLog("DNS Bad Command", "0.0.0.0");
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
    DWORD dwWaitResult = WaitForSingleObject(eventLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
        //time_t dia; //A buffer to store the date
        struct tm * timeinfo;
        time_t hora; //A buffer to store the time
        fstream fout("server_log.csv", ios::out | ios::app);
        if (info != "")
        {
            time(&hora);
            timeinfo = localtime(&hora);
            fout << "\"" << (string)asctime(timeinfo) << "\",\"" << ip << "\",\"" << info << "\"\n";
        }
        fout.close();
	}
	ReleaseMutex(eventLock);
}

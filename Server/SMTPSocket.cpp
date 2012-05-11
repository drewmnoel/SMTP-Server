//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: SMTPSocket.cpp
//Purpose: Definition of the SMTPSocket class

#include "SMTPSocket.h"
#include "ClientThread.h"
#include "ForwardThread.h"

//Name: setUp
//Parameters: none
//Returns: none
//Purpose: Fill in global variables with config or set defaults
void SMTPSocket::setUp()
{
    //Set all of the global variables by parsing the ocnfig file
	parseIniFile("sample.ini");
    eventLog("Server settings loaded from config", "0.0.0.0");
    DNS_NAME_BACKUP = getOptionToString("DNS_NAME_BACKUP");
    DNS_NAME = getOptionToString("DNS_NAME");
    DNS_IP = getOptionToString("DNS_IP");
    DNS_PORT = getOptionToInt("DNS_PORT");
    PORT = getOptionToInt("PORT");

    //WSAstartup and Socket() for server
    setUpSocket();
    Bind(PORT);
    Listen(99);
}

//Name: run
//Parameters: none
//Returns: none
//Purpose: Setup a client connection and SMTP session
void SMTPSocket::run()
{
	setUp();

	comm Socks;
	Socks.dns = dnsRegister();
	Socks.client = SOCKET_ERROR;

    CreateThread(NULL, 0, runFile, (LPVOID) &Socks, 0, NULL);
	while (1)
	{
		Socks.client = Accept(Socks.clientInfo);

        //Failed to create client thread
		if(CreateThread(NULL, 0, runClient, (LPVOID) &Socks, 0, NULL) == NULL)
		{
            eventLog("CreateThread() failed: " + (int)GetLastError(), Socks.clientInfo);
			break;
		}
		//Succeeded in creating client thread
		else
		{
		    eventLog("Client accepted", Socks.clientInfo);
		}
	}
}

//Name: dnsRegister
//Parameters: none
//Returns: A temporary socket used by the server
//Purpose: Register the server with the DNS server. Shutdown server if both names fail
SOCKET SMTPSocket::dnsRegister()
{
    //Socket of the server
	Socket temp;
	temp.setUpSocket();

	//Connect to the DNS server
	temp.Connect(DNS_IP, DNS_PORT);
	string response;

	//Send domain request to DNS server
	temp.SendData("iam " + DNS_NAME);

	//The DNS server was closed unexpectedly
	if(!temp.RecvData(response))
	{
	    eventLog("DNS connection was severed. Shutting down server", "0.0.0.0");
	    exit(1);
	}
    if (regex_match(response, (regex)"5\n*"))
    {
        //request a DNS_DNS_NAME_BACKUP DNS_NAME from the DNS server
        eventLog("Primary name already taken. Trying backup name", 0.0.0.0)
        temp.SendData("iam " + DNS_NAME_BACKUP);
        if(!temp.RecvData(response))
        {
            eventLog("DNS connection was severed. Shutting down server", "0.0.0.0");
            exit(1);
        }
        if (regex_match(response, (regex)"5\n*"))
        {
            //Kill if both name are taken
            eventLog("Both name taken. Shutting down server.", "0.0.0.0");
            exit(1);
        }
        else
        {
            //Register backup
            eventLog("Using backup name", "0.0.0.0");
            registeredName = DNS_NAME_BACKUP;
            return temp.getSocket();
        }
    }
    //Register primary
    eventLog("Primary name registered successfully", "0.0.0.0");
    registeredName = DNS_NAME;
    return temp.getSocket();
}

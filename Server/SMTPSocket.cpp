//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: SMTPSocket.cpp
//Purpose: Definition of the SMTPSocket class

#include "SMTPSocket.h"
#include "ClientThread.h"
#include "ForwardThread.h"

void SMTPSocket::setUp()
{
	parseIniFile("sample.ini");
    eventLog("Server settings loaded from config", "0.0.0.0");
    DNS_NAME_BACKUP = getOptionToString("DNS_NAME_BACKUP");
    DNS_NAME = getOptionToString("DNS_NAME");
    DNS_IP = getOptionToString("DNS_IP");
    DNS_PORT = getOptionToInt("DNS_PORT");
    PORT = getOptionToInt("PORT");

    setUpSocket();
    Bind(PORT);
    Listen(99);
}
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

		if(CreateThread(NULL, 0, runClient, (LPVOID) &Socks, 0, NULL) == NULL)
		{
            eventLog("CreateThread() failed: " + (int)GetLastError(), Socks.clientInfo);
			break;
		}
		else
		{
		    eventLog("Client accepted", Socks.clientInfo);
		}
	}
}

SOCKET SMTPSocket::dnsRegister()
{
	Socket temp;
	temp.setUpSocket();
	temp.Connect(DNS_IP, DNS_PORT);
	string response;

	//Send domain request to DNS server
	temp.SendData("iam " + DNS_NAME);
	if(!temp.RecvData(response))
	{
	    eventLog("Quitting server dns connection was severed", "0.0.0.0");
	    exit(1);
	}
    if (regex_match(response, (regex)"5\n*"))
    {
        //request a DNS_DNS_NAME_BACKUP DNS_NAME from the DNS server
        cout << "Name already taken. Trying backup name" << endl;
        temp.SendData("iam " + DNS_NAME_BACKUP);
        if(!temp.RecvData(response))
        {
            eventLog("Quitting server dns connection was severed", "0.0.0.0");
            exit(1);
        }
        if (regex_match(response, (regex)"5\n*"))
        {
            //Kill if both name are taken
            eventLog("Both name taken. Quitting server.", "0.0.0.0");
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
    eventLog("First name registered successfully", "0.0.0.0");
    registeredName = DNS_NAME;
    return temp.getSocket();
}

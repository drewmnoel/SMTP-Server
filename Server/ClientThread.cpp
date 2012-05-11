#include "ClientThread.h"
#include <iostream>
#include <regex>
using namespace std;

ClientThread::ClientThread()
{
	forwarded = false;
}

void ClientThread::run(LPVOID info)
{
	//put the struct passed into vars we can use
	Socks = (comm*) info;
	dns = new Socket(Socks->dns, DNS_IP);
	client = new Socket(Socks->client, Socks->clientInfo);

	//other vars we need
	stringstream completeMessage;
	ofstream fout;
	string data;
	char mBuff[2];

	checkSource();

    completeMessage << ((forwarded) ? "true\n" : "false\n");

	// Perform the handshake
	client->SendData("220 " + registeredName + " ESMTP Postfix\n");
	if(!client->RecvData(data))
	{
	    eventLog("client disconnect",Socks->clientInfo);
        return;
	}
    if (!regex_match(data,(regex)"HELO .*\n*"))
    {
        client->SendData("221 Closing Transmission Channel\n");
        return;
    }
    client->SendData("250 Hello " + ((data[(data.length()-1)] == '\n') ? data.substr(5,(data.length()-6)) : data.substr(5)) + ", I am glad to meet you\n");


    while (!regex_match(data,(regex)"DATA\n*"))
    {
		if(!client->RecvData(data))
		{
			eventLog("client disconnect",Socks->clientInfo);
			return;
		}
        if (regex_match(data, (regex)"MAIL FROM:<.+@.+>\n*") || regex_match(data,(regex)"RCPT TO:<.+@.+>\n*"))
        {
            completeMessage << data;
            client->SendData("250 OK\n");
        }
        else
        {
            client->SendData("500 Command Syntax Error\n");
        }
        
        if(!client->RecvData(data))
        {
            eventLog("client disconnect",Socks->clientInfo);
            return;
        }
    }
    client->SendData("354 End data with <CR><LF>.<CR><LF>\n");
    completeMessage << data;
    while (!regex_match(data,(regex)"\\.\n*"))
    {
        eventLog("Received data \"" + (regex_match(data,(regex)"*\n") ? data : data.substr(0,(data.length() -1))) + "\"", Socks->clientInfo);
        if(!client->RecvData(data))
        {
            eventLog("client disconnect",Socks->clientInfo);
            return;
        }
        completeMessage << data;
    }
    Message_Queue += 1;
    sprintf(mBuff,"%d",Message_Queue);
    client->SendData("250 OK: queued as " + (string)mBuff + (string)"\n");

    //write dat message down
    if (WaitForSingleObject(fileLock, INFINITE) == WAIT_OBJECT_0)
	{
		fout.open("master_baffer.woopsy", ios::app);
		fout << completeMessage.str() << endl;
		fout.close();
	}
	ReleaseMutex(fileLock);

    //now look for the quit
    while(!regex_match(data,(regex)"QUIT\n*"))
    {
        if(!client->RecvData(data))
        {
            eventLog("client disconnect after message queued",Socks->clientInfo);
            return;
        }
    }
    client->SendData("221 BYE\n");
    shutdown(Socks->client, 0);
    return;
}

void ClientThread::checkSource()
{
	std::string response;
	if (WaitForSingleObject(dnsLock, INFINITE) == WAIT_OBJECT_0)
	{
		//Test IP against DNS
		dns->SendData("who " + Socks->clientInfo);

		if(!dns->RecvData(response))
		{
		    eventLog("Quitting server dns connection was severed", "0.0.0.0");
		    return;
		}

        //If 0, it's from a server, if not, it's from a client
        if (regex_match(response, (regex)"0\n*"))
        {
            eventLog("Message is from a server",Socks->clientInfo);
            forwarded = true;
        }
        else
        {
            eventLog("Message is from a client",Socks->clientInfo);
        }
        printf("message is from a %s\n", ((forwarded) ? "server" : "client"));
	}
	ReleaseMutex(dnsLock);
}

DWORD WINAPI runClient(LPVOID toPass)
{
	ClientThread newThread;
	newThread.run(toPass);
	return 0;
}
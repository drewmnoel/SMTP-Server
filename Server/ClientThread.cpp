//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ClientThread.cpp
//Purpose: Definition of the ClientThread class

#include "ClientThread.h"
#include <iostream>
#include <regex>
using namespace std;

//Name: ClientThread
//Parameters: none
//Returns: none
//Purpose: Default constructor
ClientThread::ClientThread()
{
	forwarded = false;
}

//Name: run
//Parameters: 1
//    info - A comm struct containing the socket for the client and DNS server
//Returns: none
//Purpose: Setup DNS and client sockets. Perform SMTP protocol with client
void ClientThread::run(LPVOID info)
{
	//put the struct passed into vars we can use
	Socks = (comm*) info;
	dns = new Socket(Socks->dns, DNS_IP);
	client = new Socket(Socks->client, Socks->clientInfo);

	//We store the entire message that we build in this
	stringstream completeMessage;
	ofstream fout;
	string data;
	char mBuff[2];

    //Verify the IP we intend to send to
	checkSource();

    //Build the complete message
    completeMessage << ((forwarded) ? "true\n" : "false\n");

	//Perform the handshake
	//Meet and greet
	client->SendData("220 " + registeredName + " ESMTP Postfix\n");
	if(!client->RecvData(data))
	{
	    eventLog("Client disconnected unexpectedly", Socks->clientInfo);
        return;
	}
    if (!regex_match(data,(regex)"HELO .*\n*"))
    {
        client->SendData("221 Closing Transmission Channel\n");
        return;
    }
    client->SendData("250 Hello " + ((data[(data.length()-1)] == '\n') ? data.substr(5,(data.length()-6)) : data.substr(5)) + ", I am glad to meet you\n");

    //After meet and greet. Start sending and receiving
    if(!client->RecvData(data))
    {
        eventLog("Client disconnected unexpectedly", Socks->clientInfo);
        return;
    }

    //Receive the MAIL FROM/RPCT TO/bad commands until client sends DATA
    while (!regex_match(data,(regex)"DATA\n*"))
    {
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
            eventLog("Client disconnected unexpectedly", Socks->clientInfo);
            return;
        }
    }
    //Client will beign sending the message
    client->SendData("354 End data with <CR><LF>.<CR><LF>\n");
    completeMessage << data;

    //Keep building up the complete message until the delimiter is reached
    while (data.substr(data.length()-3) != "\n.\n")
    {
        //Possible double log?
        eventLog("Received data \"" + (regex_match(data,(regex)"*\n") ? data : data.substr(0,(data.length() -1))) + "\"", Socks->clientInfo);
        if(!client->RecvData(data))
        {
            eventLog("Client disconnected unexpectedly", Socks->clientInfo);
            return;
        }
        //Actual building of complete message
        completeMessage << data;
    }

    //Increment the message queue
    Message_Queue += 1;
    sprintf(mBuff,"%d",Message_Queue);
    client->SendData("250 OK: queued as " + (string)mBuff + (string)"\n");

    //Write dat message down in the master_baffer
    if (WaitForSingleObject(fileLock, INFINITE) == WAIT_OBJECT_0)
	{
		fout.open("master_baffer.woopsy", ios::app);
		fout << completeMessage.str() << endl;
		fout.close();
	}
	ReleaseMutex(fileLock);

    //Now look for the quit
    while(!regex_match(data,(regex)"QUIT\n*"))
    {
        if(!client->RecvData(data))
        {
            eventLog("Client disconnect after message queued",Socks->clientInfo);
            return;
        }
    }
    //Close this bad boy down and say goodnye
    client->SendData("221 BYE\n");
    shutdown(Socks->client, 0);
    return;
}

//Name: checkSource
//Parameters: none
//Returns: none
//Purpose: Verify that an IP is a server
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

//Name: runClient
//Parameters: 1
//    toPass - ????????
//Returns: ID for client thread
//Purpose: Start up the client thread
DWORD WINAPI runClient(LPVOID toPass)
{
	ClientThread newThread;
	newThread.run(toPass);
	return 0;
}

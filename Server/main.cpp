#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <sstream>
#include <regex>
#include <fstream>
#include "Socket.h"

#define DNS_NAME        "servera"
#define DNS_NAME_BACKUP "server1"
#define DNS_IP          "129.21.112.89"
#define DNS_PORT        53
#define PORT            25
//#define MESSAGE_SIZE


using namespace std;

SOCKET dnsRegister(string,string,string);
DWORD WINAPI ClientThread(LPVOID lpParam);
DWORD WINAPI fileThread(LPVOID lpParam);

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

int main()
{

    //register dns name and keep socket open
    SOCKET dnsSocket = dnsRegister(DNS_IP,DNS_NAME,DNS_NAME_BACKUP);

    //create mutex on dns socket
    dnsLock = CreateMutex(NULL, FALSE, NULL);
    if (dnsLock == NULL)
	{
		return (1);
	}
	fileLock = CreateMutex(NULL, FALSE, NULL);
    if (fileLock == NULL)
	{
		return (1);
	}

    SOCKET server = setUpSocket();
    Bind(server,PORT);
    Listen(server,99);

    HANDLE hThread;
    DWORD dwThreadId;

    clientAndDns justDns;
    justDns.dns = dnsSocket;
    HANDLE readFile = CreateThread(NULL, 0, fileThread, (LPVOID)&justDns, 0, &dwThreadId);

    while(1)
    {
        clientAndDns temp;
        temp.client = Accept(server,temp.cIP);
        temp.dns = dnsSocket;

        hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)&temp, 0, &dwThreadId);
        if (hThread == NULL)
        {
            printf("CreateThread() failed: %d\n", (int)GetLastError());
            break;
        }
        CloseHandle(hThread);
    }

    system("pause");
	return 0;
}

SOCKET dnsRegister(string ip,string name, string backup)
{
    SOCKET temp;
    temp = setUpSocket();
    Connect(temp,ip,DNS_PORT);

    string response;

    SendData(temp,"iam " + name);
    RecvData(temp,response);
    if(response == "5")
    {
        cout << "Name already taken trying backup name"<< endl;
        response = "";
        SendData(temp,"iam " + backup);
        RecvData(temp,response);
        if(response == "5")
        {
            cout << "Both names taken quitting server"<< endl;
            exit(1);
        }
        else{
            registered_name = DNS_NAME_BACKUP;
            return temp;
        }
    }
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

    stringstream completeMessage;

    DWORD dwWaitResult = WaitForSingleObject(dnsLock, INFINITE);
    if(dwWaitResult == WAIT_OBJECT_0)
    {
        SendData(dns,"who " + clientIP);
        string response;
        RecvData(dns,response);
        bool fowarded = false;

        if (response == "0"){
            fowarded = true;
        }
        printf("message is from a %s\n",(fowarded) ? "server" : "client");
    }
    ReleaseMutex(dnsLock);

    //here is where we send and recieve data from the client
    SendData(client,"220 " + registered_name + " ESMTP Postfix");
    string data = "";
    RecvData(client,data);
    if(data.substr(0,4) != "HELO")
    {
        SendData(client,"221");
        return 0;
    }
    SendData(client,"250 Hello " + data.substr(5) + ", I am glad to meet you");
    RecvData(client,data);
    while(data != "DATA")
    {
        if(regex_match(data,from))
        {
            completeMessage << data << endl;
            SendData(client,"250 OK");
        }
        else if(regex_match(data,to))
        {
            completeMessage << data << endl;
            SendData(client,"250 OK");
        }
        else
        {
            SendData(client,"500 Command Syntax Error");
        }
        RecvData(client,data);
    }
    SendData(client,"354 End data with <CR><LF>.<CR><LF>");
    completeMessage << data << endl;
    while (data != ".")
    {
        RecvData(client,data);
        completeMessage << data;
    }
    completeMessage << endl << "." << endl;
    SendData(client,"250 OK: queued as " + ++Message_Queue);
    RecvData(client,data);
    if(data == "QUIT")
    {
        SendData(client,"221 BYE");
        return 0;
    }
    //end send receive area

    dwWaitResult = WaitForSingleObject(fileLock, INFINITE);
    if(dwWaitResult == WAIT_OBJECT_0)
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
    DWORD dwWaitResult = WaitForSingleObject(fileLock, INFINITE);
    if(dwWaitResult == WAIT_OBJECT_0)
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
            if (userName == "alex" || userName == "dan" || userName == "drew" || userName == "scott")
            {
                fin.open((userName + ".txt").c_str(), ios::app);
                fin << toFile.str();
                fin.close();
            }
            else
            {
                cerr << "No user \"" << userName << "\" exists on this server." << endl;
            }
        } 
        //We are forwarding the message
        else
        {
            DWORD dwWaitResult = WaitForSingleObject(dnsLock, INFINITE);
            if(dwWaitResult == WAIT_OBJECT_0)
            {
                //dns stuff after we parse the to line
            	string forwardDomain;
            	//TODO: Fill in forwardDomain
                SendData(dns,"who " + forwardDomain);
                string response;
                RecvData(dns,response);
			if (response == "3") {
               		cout << "Domain not registered\n";
               		//*Put it at the end of the file
              		 return 0;
            	} 
			else if (response == "4") {
              		cout << "Bad command\n";
              		return 0;
            	}
            	else {
                	 SOCKET relay;
                 	if (!Connect(relay,response,PORT)) {
                 	   cout << "Connection to relay failed\n";
                 	   return 0;
                 	}
            	}
                //check if its an ip or an invalid address and cannot sent it
            }
            ReleaseMutex(dnsLock);

            //now that we have an ip we can continue or we can end it here if invalid or whatever 
        }
    }
    ReleaseMutex(fileLock);
}

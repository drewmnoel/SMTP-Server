#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include "Socket.h"

#define DNS_NAME        "servera"
#define DNS_NAME_BACKUP "server1"
#define DNS_IP          "127.0.0.1"
#define DNS_PORT        53
#define PORT            25


using namespace std;

SOCKET dnsRegister(string,string,string);
DWORD WINAPI ClientThread(LPVOID lpParam);




HANDLE dnsLock;
HANDLE fileLock;

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
            return temp;
        }
    }
    return temp;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
    clientAndDns *temp = (clientAndDns*) lpParam;
    SOCKET client = temp->client;
    SOCKET dns = temp->dns;
    string clientIP = temp->cIP;

    SendData(client,"Im Alive");
    printf("now im dead");
    return 0;
}

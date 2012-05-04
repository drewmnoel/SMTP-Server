#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "ClientSocket.h"

#define DEFAULT_BUFFER = 255;

using namespace std;

int main()
{
    string dnsIP = "127.0.0.1";
    string input;
    ClientSocket dnsSocket;
    ClientSocket smtpSocket;
    string smtpIP;
    string data;

    dnsSocket.ConnectToServer(dnsIP,53);

    cout << "SMTP Sever name: ";
    getline(cin,input);

    dnsSocket.SendData("who " + input);
    dnsSocket.RecvData(smtpIP);

    smtpSocket.ConnectToServer(smtpIP,25);

    data = "";

    while(data != "250 ok")
    {
        cin.ignore();
        cout << "From: ";
        getline(cin,input);
        smtpSocket.SendData("MAIL FROM:<" + input + ">");
        smtpSocket.RecvData(data);
        if (data != "250 ok")
        {
            cout << "Please Re-enter ";
        }
    }

    data = "";
    int Rcpt;
    cout << "Number of Reciepents: ";
    cin >> Rcpt;
    for(int x = 0;x < Rcpt;x++)
    {
        while(data != "250 ok")
        {
            cin.ignore();
            cout << "To: ";
            getline(cin,input);
            smtpSocket.SendData("RCPT TO:<" + input + ">");
            smtpSocket.RecvData(data);
            if (data != "250 ok")
            {
                cout << "Please Re-enter ";
            }
        }
    }

    data = "";



	return 0;
}

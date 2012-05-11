//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: main.cpp
//Purpose: Main file for the server
#include "SMTPSocket.h"
using namespace std;
//start prototypes
DWORD WINAPI clientThread(LPVOID lpParam);
DWORD WINAPI fileThread(LPVOID lpParam);
//end prototypes

//start Mutexs
HANDLE dnsLock;
HANDLE fileLock;
HANDLE eventLock;
//end mutexs

string registeredName;
int Message_Queue;
string DNS_IP;
int PORT;

int main()
{
    //Initialize Message_Queue
	Message_Queue = 0;

	//Setup mutexes
	eventLock = CreateMutex(NULL, FALSE, NULL);
	dnsLock = CreateMutex(NULL, FALSE, NULL);
	fileLock = CreateMutex(NULL, FALSE, NULL);

    //Shutdown server if mutex setup failed
	if (!eventLock || !dnsLock || !fileLock)
	{
	    eventLog("Failed to setup mutexes", "0.0.0.0");
		return (1);
	}

	eventLog("Created mutexes", "0.0.0.0");

    //Call constructor for server socket
	SMTPSocket server;
	server.run();

    //All's well that ends well
	system("pause");
	return 0;
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

        //Open the server log file
        ofstream fout("server_log.csv", ios::app);

        //As long as the message is not blank, log it
        if (info != "")
        {
            //Expand upon the DNS response codes for logging purposes
            if (info == "0")
                info = "0 - Name registered successfully";
            else if (info == "3")
                info = "3 - Domain not registered";
            else if (info == "4")
                info = "4 - Bad command";
            else if (info == "5")
                info = "5 - Error encountered";

                time(&hora);
                timeinfo = localtime(&hora);
                string date = (string)asctime(timeinfo);
                fout << "\"" << date.substr(0,(date.length() - 1)) << "\",\"" << ip << "\",\"" << info << "\"\n";
        }
        //Close the file
        fout.close();
	}
	//Release the mutex
	ReleaseMutex(eventLock);
}

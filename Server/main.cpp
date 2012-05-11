#include "SMTPSocket.h"

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
std::string DNS_IP;

int main()
{
	Message_Queue = 0;
	eventLock = CreateMutex(NULL, FALSE, NULL);
	dnsLock = CreateMutex(NULL, FALSE, NULL);
	fileLock = CreateMutex(NULL, FALSE, NULL);

	if (!eventLock || !dnsLock || !fileLock)
	{
		return (1);
	}

	eventLog("Created mutexes", "0.0.0.0");

	SMTPSocket server;
	server.run();

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
        ofstream fout("server_log.csv", ios::app);
        if (info != "")
        {
            time(&hora);
            timeinfo = localtime(&hora);
            string date = (string)asctime(timeinfo);
            fout << "\"" << date.substr(0,(date.length() - 1) << "\",\"" << ip << "\",\"" << info << "\"\n";
        }
        fout.close();
	}
	ReleaseMutex(eventLock);
}

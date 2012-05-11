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

/*
DWORD WINAPI fileThread(LPVOID lpParam)
{
	clientAndDns *temp = (clientAndDns*) lpParam;
	SOCKET dns = temp->dns;
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
				fin.open((userName + ".txt").c_str(), ios::out | ios::app);
				fin << toFile.str();
                eventLog("Stored entire message in " + userName + ".txt", "0.0.0.0");
				fin.close();
			}
			else
			{
				cerr << "No user " << userName << " exists on this server."
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
				SendData(dns, "who " + forwardDomain);
				string response;
				RecvData(dns, response);

			    if (response == "3")
                {
                    eventLog("Domain not registered", "0.0.0.0");
               		cout << "Domain not registered\n";
               		//Put it at the end of the file
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
            	    SOCKET relay = SOCKET_ERROR;
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
	return 0;
}
*/
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
            fout << "\"" << (string)asctime(timeinfo) << "\",\"" << ip << "\",\"" << info << "\"\n";
        }
        fout.close();
	}
	ReleaseMutex(eventLock);
}

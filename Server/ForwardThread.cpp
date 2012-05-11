#include "ForwardThread.h"
void ForwardThread::run(LPVOID info)
{
	Socks = (comm*) info;
	dns = new Socket(Socks->dns, DNS_IP);

	ifstream fin;
	bool forward;
	stringstream fileBuffer;
	string clientData, userName;

	// Get the file mutex
	DWORD dwWaitResult = WaitForSingleObject(fileLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		fstream fin("master_baffer.woopsy", ios::in);

		// True means server, false means local user
		getline(fin, clientData);
		if (clientData == "true")
			forward = true;
		else
			forward = false;

		//Store in the stringstream
		fileBuffer << clientData << endl;

        //Get the next line which would be the TO & store
		getline(fin, clientData);
		fileBuffer << clientData << endl;

        //Keep reading in until you get to DATA
		while (clientData != "DATA")
		{
			getline(fin, clientData);
			fileBuffer << clientData << endl;
		}

		//Keep reading in until the end of message marker
		while (clientData != ".")
		{
			getline(fin, clientData);
			fileBuffer << clientData << endl;
		}

		//Close the file and clear the clientData buffer
		fin.close();
		clientData = "";

        //The user is local
		if (!forward)
		{
			if (userName == "alex" || userName == "dan" || userName == "drew"
					|| userName == "scott" || userName == "rich")
			{
                //Open the correct user file and append the string stream into it
				fin.open((userName + ".txt").c_str(), ios::out | ios::app);
				fin << fileBuffer.str();
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
			//TODO: Get the destination out of the RCPT TO:<x@y>
			dnsLookup("");
            //now that we have an ip we can continue or we can end it here if invalid or whatever
            if (validRelay)
            {
                while (clientData != ".")
                {
                    getline(fileBuffer, clientData);
                    relay->SendData(clientData + "\n");
                }
            }
		}
	}
	ReleaseMutex(fileLock);
	return;
}

void ForwardThread::dnsLookup(string toLookup)
{
	DWORD dwWaitResult = WaitForSingleObject(dnsLock, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		dns->SendData("who " + toLookup);
		
		string response;
		dns->RecvData(response);

		if (response == "3")
		{
			eventLog("Domain not registered", "0.0.0.0");
			cout << "Domain not registered\n";
			//*Put it at the end of the file
			validRelay = false;
			return;
		}
		else if (response == "4")
		{
			eventLog("DNS Bad Command", "0.0.0.0");
			cout << "Bad command\n";
			validRelay = false;
			return;
		}
		else
		{
			relay = new Socket();
			if (!relay->Connect(response,25)) {
				cout << "Connection to relay failed\n";
				validRelay = true;
				return;
			}
			else
				validRelay = false;
		}
		//check if its an ip or an invalid address and cannot sent it
	}
	ReleaseMutex(dnsLock);
}

DWORD WINAPI fileThread(LPVOID lpParam)
{
	return 0;
}
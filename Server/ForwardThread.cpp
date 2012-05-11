//Project.setAuthors("Richard Couillard", "Alexander Leary", "Daniel Mercado", "Scott Fenwick");
//Assignment: SMTP
//File: ForwardThread.cpp
//Purpose: Definition of the ForwardThread class

#include "ForwardThread.h"

extern int PORT;

ForwardThread::ForwardThread()
{
}

void ForwardThread::run(LPVOID info)
{
	Socks = (comm*) info;
	dns = new Socket(Socks->dns, DNS_IP);

	fstream fin;
	bool finalDestination;
	stringstream fileBuffer, restOfFile;
	string clientData, userName, destServer;

	while(1)
	{
		Sleep(1000);
		// Get the file mutex
		if (WaitForSingleObject(fileLock, INFINITE) == WAIT_OBJECT_0)
		{
			
			fin.open("master_baffer.woopsy", ios::in);
			if(!fin.is_open())
			{
				ReleaseMutex(fileLock);
				continue;
			}

			// True means server, false means local user
			clientData = "";
			while(clientData == "" && !fin.eof())
				getline(fin, clientData);

			if(fin.eof())
			{
				ReleaseMutex(fileLock);
				continue;
			}
			if (clientData == "true")
				finalDestination = true;
			else
				finalDestination = false;



			//Store in the stringstream
			//fileBuffer << clientData << endl;

	        //Get the next line which would be the TO & store
			getline(fin, clientData);
			fileBuffer << clientData << endl;

	        //Keep reading in until you get to DATA
			while (clientData != "DATA")
			{
				getline(fin, clientData);
				fileBuffer << clientData << endl;

				// See if we got a RCPT TO
				if(clientData[0] == 'R')
				{
					destServer = clientData.substr(clientData.find('@')+1, clientData.length()-clientData.find('@')-2);
					int start = clientData.find("<");
                    int length = clientData.find("@") - start;
                    userName = clientData.substr(++start, --length);
				}
			}

			//Keep reading in until the end of message marker
			while (clientData != ".")
			{
				getline(fin, clientData);
				fileBuffer << clientData << endl;
			}

			// Get the rest of the file
			restOfFile << fin.rdbuf(); 

			fin.close();

			remove("master_baffer.woopsy");
			fin.open("master_baffer.woopsy", ios::out);
			fin << restOfFile.rdbuf();
			fin.close();

			clientData = "";

			/* We have read the entire message into memory */
			/* Read the username */


	        //The user is local
			if (finalDestination)
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

			//We are finalDestinationing the message
			else
			{
				dnsLookup(destServer);

				relay.SendData("HELO " + registeredName);
	            Sleep(50);

	            //now that we have an ip we can continue or we can end it here if invalid or whatever
	            if (validRelay)
	            {
	                while (clientData != ".")
	                {
	                    getline(fileBuffer, clientData);
	                    relay.SendData(clientData + "\n");
	                    Sleep(50);
	                }
	            }
			}
			ReleaseMutex(fileLock);
		
		}
	}
}

//Name: dnsLookup
//Parameters: 1
//    toLookup - The IP to perform the lookup for
//Returns: none
//Purpose: Determine the name associated with an IP address
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
			//*Put it at the end of the file
			validRelay = false;
			return;
		}
		else if (response == "4")
		{
			eventLog("DNS Bad Command", "0.0.0.0");
			validRelay = false;
			return;
		}
		else
		{
			relay = Socket();
			relay.setUpSocket();
			if (!relay.Connect(response,PORT)) 
			{
				validRelay = false;
				return;
			}
			else
				validRelay = true;
		}
		//check if its an ip or an invalid address and cannot sent it
		ReleaseMutex(dnsLock);
	}
}

DWORD WINAPI runFile(LPVOID lpParam)
{
	ForwardThread myThread;
	myThread.run(lpParam);
	return 0;
}

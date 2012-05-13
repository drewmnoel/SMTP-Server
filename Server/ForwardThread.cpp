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
		validRelay = true;

		fileBuffer.clear();
		fileBuffer.str("");

		restOfFile.str("");
		restOfFile.clear();

		clientData = "";
		Sleep(1000);
		// Get the file mutex
		std::cout << "Waiting for file lock...\n";
		if (WaitForSingleObject(fileLock, INFINITE) == WAIT_OBJECT_0)
		{
			fin.open("master_baffer.woopsy", ios::in);
			if(!fin.is_open())
			{
				fin.close();
				ReleaseMutex(fileLock);
				continue;
			}

			// True means server, false means local user
			clientData = "";
			while(clientData == "" && !fin.eof())
				getline(fin, clientData);

			if(fin.eof())
			{
				fin.close();
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
			fin << restOfFile.str();
			fin.close();

			clientData = "";

			/* We have read the entire message into memory */
			/* Read the username */


	        //The user is local
			if (finalDestination || destServer == registeredName)
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
				string destIP = dnsLookup(destServer);
				if (destIP == "")
				{
					validRelay = false;
				}
				else
				{
					relay = Socket();
					relay.setUpSocket();

					if (validRelay && !relay.Connect(destIP,PORT)) 
					{
						validRelay = false;
					}
					else
						validRelay = true;
				}

	            //now that we have an ip we can continue or we can end it here if invalid or whatever
	            bool inData = false;
	            if (validRelay)
	            {
	            	relay.RecvData(clientData);
					relay.SendData("HELO " + registeredName + "\n");
	                while (clientData != "." && !fileBuffer.eof())
	                {
	                	if(clientData == "DATA")
	                	{
	                		inData = true;
	                		relay.RecvData(clientData);
	                	}
	                	else if(!inData)
	                	{
	                		relay.RecvData(clientData);
	                	}

                		getline(fileBuffer, clientData);

                		if(clientData == ".")
                			Sleep(250);
                		relay.SendData(clientData + "\n");
                		std::cout << "Loop: " << clientData << "\n";
	                }
	           		getline(fileBuffer, clientData);

	            	relay.RecvData(clientData);

		            relay.SendData("QUIT\n");
		            relay.RecvData(clientData);
		            relay.CloseSocket();
		        }
		        else
		        {
		        	std::cout << "Something went wrong, not forwarding!\n";
					fin.open("master_baffer.woopsy", ios::out);
					fin << restOfFile.str();
					fin << "false\n";
					fin << fileBuffer.str() << std::endl;
					fin.close();
		        }
			}
		}
		ReleaseMutex(fileLock);
	}
}

//Name: dnsLookup
//Parameters: 1
//    toLookup - The IP to perform the lookup for
//Returns: none
//Purpose: Determine the name associated with an IP address
string ForwardThread::dnsLookup(string toLookup)
{
	string retval;
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
			retval = "";
		}
		else if (response == "4")
		{
			eventLog("DNS Bad Command", "0.0.0.0");
			validRelay = false;
			retval = "";
		}
		else
		{
			retval = response;
		}

		//check if its an ip or an invalid address and cannot sent it
		ReleaseMutex(dnsLock);
	}
	return retval;
}

DWORD WINAPI runFile(LPVOID lpParam)
{
	ForwardThread myThread;
	myThread.run(lpParam);
	return 0;
}

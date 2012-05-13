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
	string clientData;
	string destServer[10];
	string userName [10];
	string mark[10];
	int toNumber = 0;
    stringstream temp;
    vector<string> sentTo;

	while(1)
	{
		while(sentTo.size() != 0)
			sentTo.pop_back();

		validRelay = true;
		bool notFowarded = false;

		fileBuffer.clear();
		fileBuffer.str("");

		temp.clear();
		temp.str("");

		restOfFile.str("");
		restOfFile.clear();

		clientData = "";
		toNumber = 0;
		Sleep(1000);
		// Get the file mutex

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
                    if(clientData.substr(clientData.length() - 1) != "x")
                    {
                        destServer[toNumber] = clientData.substr(clientData.find('@')+1, clientData.length()-clientData.find('@')-2);
                        int start = clientData.find("<");
                        int length = clientData.find("@") - start;
                        userName [toNumber] = clientData.substr(++start, --length);
                        mark[toNumber] = "";
                        toNumber++;
                    }
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

            for(int x = 0;x < toNumber;x++)
            {
            	// Check to see if we have sent to this server already
            	vector<string>::iterator it;
            	it = find(sentTo.begin(), sentTo.end(), destServer[x]);
            	if(it != sentTo.end() && destServer[x] != registeredName)
            		continue;

            	sentTo.push_back(destServer[x]);
                //The user is local
                if (destServer[x] == registeredName)
                {
                        if (userName [x] == "alex" || userName [x] == "dan" || userName [x] == "drew"
                                || userName [x] == "scott" || userName [x] == "rich")
                        {
                        	if(mark[x] != "x")
                        	{
                            	//Open the correct user file and append the string stream into it
                            	fin.open((userName [x] + ".txt").c_str(), ios::out | ios::app);
                            	temp.str("");
                            	string linebyline;
                            	fileBuffer.clear();
                            	while(!fileBuffer.eof())
                            	{
                            	    getline(fileBuffer,linebyline);
                            	    if(linebyline.substr(0,4) == "RCPT")
                            	    {
                            	        if(linebyline.substr(linebyline.length() - 1) == "x")
                            	        {
                            	            temp << linebyline.substr(0,linebyline.length() - 1) << "\n";
                            	        }
                            	        else
                            	        {
                                            temp << linebyline << "\n";
                            	        }
                            	    }
                            	    else
                            	    {
                            	        temp << linebyline << "\n";
                            	    }
                            	}
                            	fileBuffer.str("");
                            	fileBuffer.clear();
                            	fileBuffer << temp.str();

                            	fin << fileBuffer.str();
                            	eventLog("Stored entire message in " + userName [x] + ".txt", "0.0.0.0");
                            	fin.close();
                            	mark[x] = "x";
                            }
                        }
                }

                //We are finalDestinationing the message
                else if(!finalDestination)
                {
                    string destIP = dnsLookup(destServer[x]);
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

                        temp.str("");
                        string linebyline;
                        while(!fileBuffer.eof())
                        {
                            getline(fileBuffer,linebyline);
                            if(linebyline.substr(0,4) == "RCPT")
                            {
                                if(linebyline.substr(linebyline.length() - 1) == "x")
                                {
                                    temp << linebyline.substr(0,linebyline.length() - 1) << "\n";
                                }
                                else
                                {
                                    temp << linebyline << "\n";
                                }
                            }
                            else
                            {
                                temp << linebyline << "\n";
                            }
                        }
                        fileBuffer.str("");
                        fileBuffer.clear();
                        fileBuffer << temp.str();

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

                            relay.SendData(clientData + "\n");

                        }
                        getline(fileBuffer, clientData);

                        relay.RecvData(clientData);

                        relay.SendData("QUIT\n");
                        relay.RecvData(clientData);
                        relay.CloseSocket();
                        mark[x] = "x";
                    }
                    else
                    {
                        notFowarded = true;
                    }
                }
            }

            if(notFowarded)
            {
                fin.open("master_baffer.woopsy", ios::out);
                fin << restOfFile.str();
                fin << "false\n";

                //mark things then write
                for(int i = 0;i < toNumber;i++)
                {
                	string tempToStr;
                    if(mark[i] == "x")
                    {
                    	temp.clear();
                    	temp.str("");
                    	temp << fileBuffer.str();
                    	tempToStr = temp.str();
                    	fileBuffer.str("");

                    	if(tempToStr.find("RCPT TO:<" + userName[i] + "@" + destServer[i] + ">\n") != string::npos)
                    	{
                        	fileBuffer << tempToStr.replace(tempToStr.find("RCPT TO:<" + userName[i] + "@" + destServer[i] + ">\n"),(11 + userName[i].length() + destServer[i].length()),"RCPT TO:<" + userName[i] + "@" + destServer[i] + ">x");
                    	}
                		else
                		{
                			fileBuffer << tempToStr;
                		}
                    }
                }

                fin << fileBuffer.str() << std::endl;
                fin.close();
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

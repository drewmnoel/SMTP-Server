//------------------------------------------------------------------------------
//Course:           4050-212
//Modified By :     Alexander Leary
//Name:             Socket.cpp
//File Purpose:     This is the classes header file for the server portion of 
//                  the project containing Socket and ServerSocket 
//------------------------------------------------------------------------------

#pragma once
#include <iostream>
#include <limits>
#include <fstream>
#include <ctime>
#include <dirent.h>
#include "WinSock2.h"
#include <windows.h>

using namespace std;

const int STRLEN = 256;

class Socket
{
    private:
        ofstream logOut;
    protected:
        WSADATA wsaData;
        SOCKET mySocket;
        SOCKET myBackup;
        SOCKET acceptSocket;
        sockaddr_in myAddress;
        sockaddr_in clientSocket;
        int client_length;
        DIR *dirPtr;
		DWORD dwThreadId;
		HANDLE hThread;
        struct dirent *dirStruct;
		
    public:
        Socket();
        ~Socket();
        bool SendData( char* );
	bool RecvData( char* );
        bool RecvData( char*, int );
        void CloseConnection( );
        void GetAndSendMessage( );
        void logInfo( sockaddr_in* socket, char* );
};

class ServerSocket : public Socket
{
    private:
        void loginVerify( );
    public:
		void Bind( int );
        void Listen( int );
		void Accept( );
        void StartHosting( int, int ); 
        void commands( const char* );
        void listDir( );
        void sendFile( char* );
};

class ClientSocket : public Socket
{
	public:
		void ConnectToServer( const char*, int );
};

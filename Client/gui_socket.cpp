//------------------------------------------------------------------------------
//Course:           4050-212
//Modified By :     Alexander Leary
//Name:             Socket.cpp
//File Purpose:     This is the classes source file for the server portion of 
//                  the project containing Socket and ServerSocket 
//------------------------------------------------------------------------------

//Be sure to properly exit the program so that a recieve loop doesn't occur, 
//it kinda mucks up the log file with entries

#include "gui_socket.h"

//server

struct passedInfo
{
	SOCKET passedSocket;
	int conCount;
};
HANDLE ghMutex; 

Socket::Socket()
{
    if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != NO_ERROR )
    {
        cerr<<"Socket Initialization: Error with WSAStartup\n";
        system("pause");
        WSACleanup();
        exit(10);
    }

    //Create a socket
    mySocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( mySocket == INVALID_SOCKET )
    {
        cerr<<"Socket Initialization: Error creating socket"<<endl;
        system("pause");
        WSACleanup();
        exit(11);
    }

    myBackup = mySocket;
    
    //logOut.open("logfile.csv", ios::out | ios::app);
	//std::string title = "\"New Session\"\n\"Date\",\"Time\",\"Source Address\",\"Port\",\"Message\"\n";
	//logOut.is_open() ? logOut<<title : std::cerr<<"Error Opening File\n";
}

Socket::~Socket( )
{
	CloseHandle( ghMutex );
    WSACleanup( );
    //logOut.close( );
}

bool Socket::SendData( char *buffer )
{
    send( mySocket, buffer, strlen( buffer ), 0 );
    logInfo( &myAddress, buffer );
    return true;
}

bool Socket::RecvData( char *buffer, int size )
{
    memset( buffer, '\0', 256); 
    int i = recv( mySocket, buffer, size, 0 );
    buffer[i] = '\0';
    logInfo(&clientSocket, buffer);
    return true;
}

bool Socket::RecvData( char *buffer )
{
    memset( buffer, '\0', 256); 
    int i = recv( mySocket, buffer, sizeof( buffer ), 0 );
    buffer[i] = '\0';
    logInfo(&clientSocket, buffer);
    return true;
}

void Socket::CloseConnection()
{
    //cout<<"CLOSE CONNECTION"<<endl;
    closesocket( mySocket );
    mySocket = myBackup;
}

void Socket::GetAndSendMessage( )
{
    char message[STRLEN];
    cout<<"Send > ";
    memset( message, '\0', 256);
    cin.get( message, STRLEN );
    message[256] = '\0';
    SendData( message );
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    fflush(stdin);
}

DWORD WINAPI ReceiveCommand( LPVOID lpParam ) 
{
	cout << "Thread created: " << GetCurrentThreadId() << endl;
	struct passedInfo *current = ( struct passedInfo* ) lpParam;
	SOCKET current_client = current->passedSocket;
	char sendBuf[100], recvBuf[100];
	int res;
	strcpy( sendBuf, "Hello little friend" );
	send( current_client, sendBuf, sizeof( sendBuf ), 0 );
	
	while( true ) 
	{
		DWORD dwWaitResult = WaitForSingleObject( ghMutex, INFINITE );
		switch ( dwWaitResult ) 
        {
			case WAIT_OBJECT_0:
				memset( sendBuf, '\0', strlen( sendBuf ) );
				memset( recvBuf, '\0', strlen( recvBuf ) );
				
				cout << "---Waiting---\n";
				res = recv( current_client, recvBuf, sizeof( recvBuf ), 0 );
				recvBuf[strlen( recvBuf )] = '\0';
				cout << "Received in thread " << GetCurrentThreadId( ) <<": " << recvBuf;
				
				if( strcmp( "quit\n\0", recvBuf ) == 0 )
				{
					cout << "Ending Thread " << GetCurrentThreadId( ) << endl;
					ReleaseMutex( ghMutex ); fflush( stdin );
					//TerminateThread( hThread, 0 );
					return 0;
				}
				else if( strcmp( "Hello\n\0", recvBuf ) == 0 )
				{
					cout << "Send in thread " << GetCurrentThreadId() <<": Hello to you to\n";
					send( current_client, "Hello to you to\0", sizeof( "Hello to you to\0" ), 0 );
					fflush( stdin );
				}
				else if( strcmp( "message\n\0", recvBuf ) == 0 )
				{
					do {
						cout << "Send in thread " << GetCurrentThreadId() << ": " << recvBuf << endl;
						send( current_client, recvBuf, sizeof( recvBuf ), 0 );
						memset( recvBuf, '\0', 100 );
						recv( current_client, recvBuf, sizeof( recvBuf ), 0 );
						cout << "Received in thread " << GetCurrentThreadId( ) <<": " << recvBuf;
					} while( strcmp( ".\n\0", recvBuf ) != 0 );
					strcpy( sendBuf, "Echo mode end\n" );
					send( current_client, sendBuf, sizeof( sendBuf ), 0 );
				}
				else if( res == 0 )
				{
					cout << "Ending Thread " << GetCurrentThreadId( ) << endl;
					ReleaseMutex( ghMutex ); fflush( stdin );
					//TerminateThread( hThread, 0 );
					return 0;
				}
				else
				{
					cout << "Send in thread " << GetCurrentThreadId() <<": "; 
					cin.get( sendBuf, 100 );
					send( current_client, sendBuf, sizeof( sendBuf ), 0);
					cin.clear( );
					cin.ignore( numeric_limits<streamsize>::max( ), '\n' );
				}
				ReleaseMutex( ghMutex );
				break;
			case WAIT_ABANDONED: 
				return FALSE;
		}
    }
	cout << "Thread ended\r\n";
	return 0;
}

//------------------------------------------------------------------------------
//Meathod:   Log::logInfo()
//Purpose:   Logs connection information for a sockaddr_in and a corresponding 
//           message. Created for future use, specifically threading.
//Variables: date, time, socket->sin_addr, socket->sin_port
//Returns:   void
//------------------------------------------------------------------------------
void Socket::logInfo(sockaddr_in *socket, char *buffer) 
{
    char date[10], time[10];
	logOut << "\"" << _strdate( date ) << "\",\"" << _strtime( time ) << "\"";
    logOut << ",\"" << inet_ntoa( socket->sin_addr ) << "\"";
    logOut << ",\"" << htons( socket->sin_port ) << "\"";
    logOut << ",\"" << buffer << "\"\n";
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::Bind()
//Purpose:   Binds the a socket with a given port
//Variables: myAddress, mySocket
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::Bind( int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    myAddress.sin_port = htons( port );

    if ( bind ( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress) ) == SOCKET_ERROR )
    {
        cerr << "ServerSocket: Failed to connect\n";
        system( "pause" );
        WSACleanup( );
        exit( 14 );
    }
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::Listen()
//Purpose:   Listens for connections on a bound port
//Variables: mySocket
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::Listen( int connections )
{
    if ( listen ( mySocket, connections ) == SOCKET_ERROR )
    {
        cerr << "ServerSocket: Error listening on socket\n";
        system( "pause" );
        WSACleanup( );
        exit( 15 );
    }
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::Accept()
//Purpose:   Accept connections on port that is being listened on
//Variables: clientSocket, acceptSocket, acceptedInfo, hThread, dwThreadId
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::Accept( )
{    
    clientSocket.sin_family = AF_INET;
    client_length = sizeof(clientSocket);
    
	ghMutex = CreateMutex( NULL, FALSE, NULL );
    if ( ghMutex == NULL )
	{
		cerr << "Mutex Error\n";
        system( "pause" );
        WSACleanup( );
		exit( 16 );
	}
	int count = 0;
	while( true )
	{
		struct passedInfo acceptedInfo;
		acceptSocket = accept( myBackup, (SOCKADDR*) &clientSocket, &client_length );
		
		while ( acceptSocket == SOCKET_ERROR )
		{
			acceptSocket = accept( myBackup, (SOCKADDR*) &clientSocket, &client_length );
		}
		count++;
		acceptedInfo.passedSocket = acceptSocket;
		acceptedInfo.conCount = count;
		cout << acceptedInfo.conCount << endl;
        hThread = CreateThread( NULL, 0, ReceiveCommand, (LPVOID)&acceptedInfo, 0, &dwThreadId);
        if ( hThread == NULL )
        {
            cerr << "CreateThread Error\n";
            break;
        }			
    }
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::StartHosting()
//Purpose:   Runs the Bind, Listen, and Accept meathods
//Variables: port
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::StartHosting( int port, int connections )
{
    Bind( port );
    Listen( connections );
	Accept( );
}

//------------------------------------------------------------------------------
//Meathod:   SocketServer::loginVerify()
//Purpose:   Logs connection information for a sockaddr_in and a corresponding 
//           message. Created for future use, specifically threading.
//Variables: date, time, socket->sin_addr, socket->sin_port
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::loginVerify( ) 
{
     char user[STRLEN] = "username, password\0";
     char login[STRLEN];
	 char message[STRLEN] = "LOGIN\0";
     SendData( message );
     RecvData( login, STRLEN );
     if( strcmp( login, user ) == 0 )
          send( mySocket, "WELCOME\0", sizeof( "WELCOME\0" ), 0 );
     else {
          send( mySocket, "UNWELCOME\0", sizeof( "UNWELCOME\0" ), 0 );
          WSACleanup( );
          exit( 17 );
     }             
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::commands()
//Purpose:   Checks which commands have been sent to the server and executes the 
//           appropriate command
//Variables: recMessage, input, selection
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::commands( const char* recMessage ) 
{
    std::string input = recMessage, selection = input.substr( 0, 4 );
     
    for( int x = 0; x < 4; x++ )
        selection[x] = toupper( selection[x] );
         
    if ( selection == "LIST" ) 
    {
        listDir( );
    }
    else if ( selection == "SEND" ) 
    {
        char file[STRLEN]; 
        strncpy( file, recMessage + 5, strlen( recMessage ) );
        sendFile( file );
    }
    else if ( selection == "QUIT" ) 
    {
        send( mySocket, "Server Exiting\0", sizeof( "Server Exiting\0" ), 0 );
        exit( 0 );
    }
    else 
    {
        send( mySocket, "ERROR\0", sizeof( "ERROR\0" ), 0 );
    }
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::listDir()
//Purpose:   list the contents of the directory
//Variables: recMessage, input, selection
//Returns:   void
//Reference: https://www.linuxquestions.org/questions/programming-9/c-list-files-in-directory-379323/
//------------------------------------------------------------------------------
void ServerSocket::listDir( ) 
{
    char formated[ STRLEN ];
    char next[ STRLEN ];
    dirPtr = opendir("./"); //syntax c:\\directory\\subdirectory
    while ( ( dirStruct = readdir ( dirPtr ) ) != NULL ) {
        strncpy( formated, dirStruct->d_name, strlen( dirStruct->d_name ) );
        formated [ strlen( dirStruct->d_name ) ] = '\0';
        formated [ strlen( dirStruct->d_name ) + 1 ] = '\n';
        send( mySocket, formated, sizeof(formated) , 0 );
        recv( mySocket, next, STRLEN, 0 );
        memset( formated, '\0', 256 );
    }
    send( mySocket, "END_OF_DIRECTORY_LISTING\0", sizeof("END_OF_DIRECTORY_LISTING\0"), 0 );
    closedir( dirPtr );
}

//------------------------------------------------------------------------------
//Meathod:   ServerSocket::sendFile()
//Purpose:   Opens and sends a file to the client
//Variables: recMessage, input, selection
//Returns:   void
//------------------------------------------------------------------------------
void ServerSocket::sendFile( char* fileName ) 
{
    ifstream fileRead;
    fileRead.open( fileName, ios::in | ios::binary );
    
    char endFile[STRLEN] = "EOFEOFEOFEOFEOFEOF\0";
    char section[STRLEN];
    char next[STRLEN];
    int count = 0;
    
    memset( section, '\0', 256 ); 
    
    if( fileRead.is_open( ) ) 
	{
        send( mySocket, fileName, strlen(fileName), 0);
        while( fileRead.good( ) ) 
		{
            while( count < 255 && fileRead.peek( ) != EOF )
            {
                
				section[ count++ ] = fileRead.get( );
            }
            
            send( mySocket, section, sizeof( section ), 0 );
            recv( mySocket, next, STRLEN, 0 );
            memset( section, '\0', 256 );
            count = 0;
        }
        
        send( mySocket, endFile, sizeof( endFile ), 0 );
        recv( mySocket, section, STRLEN, 0 );
        
        if( strcmp( section, "EOF OK\0" ) == 0 )
        {
            send( mySocket, "OK\0", sizeof( "OK\0" ), 0 );
        }
        else
        {
            send( mySocket, "ERROR\0", sizeof( "ERROR\0" ), 0 );
        }
            
        fileRead.close( );
    }
    else 
    {
        send( mySocket, "Unable to Open File\0", strlen( "Unable to Open File\0" ), 0);
    }
}

//------------------------------------------------------------------------------
//Meathod:   ClientSocket::ConnectToServer()
//Purpose:   Opens as connection to the server
//Variables: ipAddress, port, myAddress, mySocket, myAddres
//Returns:   void
//------------------------------------------------------------------------------
void ClientSocket::ConnectToServer( const char *ipAddress, int port )
{
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr( ipAddress );
    myAddress.sin_port = htons( port );
    
    //cout<<"CONNECTED"<<endl;

    if ( connect( mySocket, (SOCKADDR*) &myAddress, sizeof( myAddress ) ) == SOCKET_ERROR )
    {
        cerr << "ClientSocket: Failed to connect\n";
        system( "pause" );
        WSACleanup( );
        exit( 20 );
    } 
}

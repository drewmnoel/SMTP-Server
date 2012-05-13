#include "SMTPClient.h"

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::SMTPClient( )
//Purpose:   Default Constructor
//Variables: messageOut, messageIn, convert
//Returns:   constructor
//------------------------------------------------------------------------------
SMTPClient::SMTPClient( )
{
	memset( messageOut, '\0', 128 );
	memset( messageIn, '\0', 128 );
	memset( convert, '\0', 128 );
	memset( endKey, '\0', 3 );
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::~SMTPClient( )
//Purpose:   Default Destructor
//Variables: none
//Returns:   destructor
//------------------------------------------------------------------------------
SMTPClient::~SMTPClient( )
{

}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieve220( )
//Purpose:   Starts communicating  with the server by reciving an identifying
//			 message; e.g. 220 smtp.example.com ESMTP Postfix
//Variables: messageIn
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::recieve220( HWND popup )
{
	if( RecvData( messageIn, 128 ) )
	{
		if( checkError( messageIn ) )
			return false;
		if( strncmp( messageIn, "220", 3 ) != 0)
		{
			return false;
		}
		//MessageBox( popup, messageOut, "220", MB_OK );
	}
	else
	{
		MessageBox( popup, messageIn, "Error Recieving Data", MB_OK );
		return false;
	}

	memset( messageIn, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::sendHELO( )
//Purpose:   Identifies the client name with the server that it is communicating
//			 with, e.g. HELO relay.example.org
//Variables: messageOut, convert, hwndFrom
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::sendHELO( HWND hwndFrom, HWND popup )
{
	strcpy( messageOut, "HELO " );
	hwndToChar( hwndFrom, convert );
	removeUser( convert );
	strcat( messageOut, convert );
	strcat( messageOut, "\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "HELO", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(HELO)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageIn, '\0', 128 );
	memset( convert, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieve250( )
//Purpose:   Recieves an 250 Ok signal from the server to verify last
//			 communication.
//Variables: message
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::recieve250( HWND popup )
{
	if( RecvData( messageIn, 128 ) )
	{
		if( checkError( messageIn ) )
			return false;
		if( strncmp( messageIn, "250", 3 ) != 0)
		{
			return false;
		}
		//MessageBox( popup, messageOut , "250 Ok", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Recieving Data(250 Ok)",
							"Error Recieving Data", MB_OK );
		return false;
	}

	memset( messageIn, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::mailFrom( )
//Purpose:   Sends the from address portion of the header in the mail message.
//Variables: messageOut, hwndFrom
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::sendMailFrom( HWND hwndFrom, HWND popup )
{
	strcpy( messageOut, "MAIL FROM:<" );
	hwndToChar( hwndFrom, convert );
	strcat( messageOut, convert );
	strcat( messageOut, ">\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "MAIL FROM", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(MAIL FROM)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageOut, '\0', 128 );
	memset( convert, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::sendRecieptTo( )
//Purpose:   Sends the reciept to address portion of the header in the mail
//			 message. Can handle multiple addresses using ";" as delimiter.
//Variables: hwndTo, popup, convert, messageOut, allAddresses, singleAddresses
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::sendRecieptTo( HWND hwndTo, HWND popup )
{
	hwndToChar( hwndTo, convert );
	strcat( convert, ";" );
	string allAddresses = convert;
	string singleAddress;
	size_t pos = 0;

	do {
		pos = allAddresses.find_first_of( ";" );
		singleAddress = allAddresses.substr( 0, pos );
		strcpy( messageOut, "RCPT TO:<");
		strcat( messageOut, singleAddress.c_str( ) );
		strcat( messageOut, ">\n\0" );

		if( SendData( messageOut ) )
		{
			//MessageBox( popup, messageOut, "RCPT TO", MB_OK );
		}
		else
		{
			MessageBox( popup, "Error Sending Data(RECP TO)",
								"Error Sending Data", MB_OK );
			return false;
		}

		memset( convert, '\0', 128 );
		allAddresses.erase( 0, pos + 1 );
		if( allAddresses.find_first_of( " " ) == 0 )
			allAddresses.erase( 0, 1 );
		recieve250( popup );
	} while( allAddresses.length( ) > 1 );

	memset( messageOut, '\0', 128 );
	memset( convert, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::data( )
//Purpose:   Notifies the server that the client is ready to start sending the
//			 message body;
//Variables: popup, messageOut
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::data( HWND popup )
{
	strcpy( messageOut, "DATA\n\0" );
	if( SendData( messageOut ) )
	{
		//MessageBox( popup, Message, "DATA", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(DATA)",
							"Error Sending Data", MB_OK );
		return false;
	}
	memset( messageOut, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieve354()
//Purpose:   Recives from the server the terminating character to transmit to
//			 notify the server the client is finished sending data;
//Variables: popup, messageIn, findEndKey, endKey
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::recieve354( HWND popup )
{
	if( RecvData( messageIn, 128 ) )
	{
		if( checkError( messageIn ) )
			return false;
		if( strncmp( messageIn, "354", 3 ) != 0)
		{
			return false;
		}
		else
		{
			char* findEndKey = strstr( messageIn, "<CR><LF>" );
			strncpy( endKey, findEndKey + 8, 1 );
			strcat( endKey, "\n\0" );
			//MessageBox( popup, endKey, "354 End Data", MB_OK );
			//std::cout << endKey << std::endl;
		}
	}
	else
	{
		MessageBox( popup, "Error Recieving Data(354 Terminating Character)",
							"Error Recieving Data", MB_OK );
		return false;
	}
	memset( messageIn, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieve220()
//Purpose:   Starts communicating  with the server by reciving an identifying
//			 message; e.g. 220 smtp.example.com ESMTP Postfix
//Variables: messageOut
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::sendMessage( HWND hwndTo, HWND hwndFrom, HWND hwndSubject, HWND hwndEdit, HWND popup )
{
	strcpy( messageOut, "From: <" );
	hwndToChar( hwndFrom, convert );
	strcat( messageOut, convert );
	strcat( messageOut, ">\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "DATA", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(From <>)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageOut, '\0', 128 );
	memset( convert, '\0', 128 );

	strcpy( messageOut, "To: <" );
	hwndToChar( hwndTo, convert );
	strcat( messageOut, convert );
	strcat( messageOut, ">\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "DATA", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(To <>)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageOut, '\0', 128 );
	memset( convert, '\0', 128 );

	char timebuf [80];
	time_t tod;
    struct tm* timeinfo;
    time( &tod );
    timeinfo = localtime( &tod );
    strftime (timebuf,80,"\"%a %b %d, %Y\",\"%H:%M:%S\"",timeinfo);
	strcpy( messageOut, "Time: " );
	//char date[10]; _strdate( date );
	//strcat( messageOut, date );
	//strcat( messageOut, " " );
	//char time[10]; _strtime( time );
	//strcat( messageOut, time );
	strcat( messageOut, timebuf );
	strcat( messageOut, "\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "DATA", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Sending Data(Date)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageOut, '\0', 128 );

	strcpy( messageOut, "Subject: " );
	hwndToChar( hwndSubject, convert );
	strcat( messageOut, convert );
	strcat( messageOut, "\n\0" );

	if( SendData( messageOut ) )
	{
		//MessageBox( popup, messageOut, "DATA", MB_OK );
	}
	else {
		MessageBox( popup, "Error Sending Data(Subject)",
							"Error Sending Data", MB_OK );
		return false;
	}

	memset( messageOut, '\0', 128 );
	memset( convert, '\0', 128 );

	char* convertBody;
	char* messageBody;

	try {
		convertBody = (char*)malloc(4096);
		messageBody = (char*)malloc(4096);
	} catch( bad_alloc& ) {
		MessageBox( popup, "Error Allocating Memory",
							"Error Allocating Memory", MB_OK );
		return false;
	}
	memset( messageBody, '\0', sizeof( messageBody ) );
	memset( convertBody, '\0', sizeof( convertBody ) );

	strcpy( messageBody, "\n" );
	hwndToChar( hwndEdit, convertBody );
	strcat( messageBody, convertBody );
	strcat( messageBody, "\n\0" );

	if( SendData( messageBody ) )
	{
		//MessageBox( popup, messageBody, "DATA", MB_OK );
		//cout << messageBody << endl;
	}
	else
	{
		MessageBox( popup, "Error Sending Data(Body)",
							"Error Sending Data", MB_OK );
		return false;
	}

	free( convertBody );
	free( messageBody );
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieve220()
//Purpose:   Send the end data character to the server after the message has
//			 finished transmitting.
//Variables: endKey
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::return354( HWND popup )
{
	if( SendData( endKey ) )
	{
		//MessageBox( popup, endKey, "End Data", MB_OK );
		//cout << endKey << endl;
	}
	else
	{
		MessageBox( popup, "Error Sending Data(QUIT)",
							"Error Sending Data", MB_OK );
		return false;
	}
	memset( endKey, '\0', sizeof( endKey ) );
	return false;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieveQueue()
//Purpose:   Recieves the message queue number from the server
//Variables: popup, messageIn
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::recieveQueue( HWND popup )
{
	if( RecvData( messageIn, 128 ) )
	{
		if( checkError( messageIn ) )
			return false;
		else if( strncmp( messageIn, "250", 3 ) != 0)
		{
			return false;
		}
		cout << messageIn << endl;
		//MessageBox( popup, messageIn , "queued as", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Recieving Data(Queue)",
							"Error Recieving Data", MB_OK );
		return false;
	}
	memset( messageIn, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::sendQuit()
//Purpose:   Starts communicating  with the server by reciving an identifying
//			 message; e.g. 220 smtp.example.com ESMTP Postfix
//Variables: message
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::sendQuit( HWND popup )
{
		strcpy( messageOut, "QUIT\n\0" );
		if( SendData( messageOut ) )
		{
			//MessageBox( popup, messageOut, "End Data", MB_OK );
		}
		else
		{
			MessageBox( popup, "Error Sending Data(QUIT)",
							"Error Sending Data", MB_OK );
			return false;
		}
		memset( messageOut, '\0', 128 );
		return true;
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::recieveEnd()
//Purpose:   Starts communicating  with the server by reciving an identifying
//			 message; e.g. 220 smtp.example.com ESMTP Postfix
//Variables: message
//Returns:   bool
//------------------------------------------------------------------------------
bool SMTPClient::recieveEnd( HWND popup )
{
	if( RecvData( messageIn, 128 ) )
	{
		if( checkError( messageIn ) )
			return false;
		else if( strncmp( messageIn, "221", 3 ) != 0)
		{
			return false;
		}
		//MessageBox( popup, messageIn , "End", MB_OK );
	}
	else
	{
		MessageBox( popup, "Error Recieving Data(End)",
							"Error Recieving Data", MB_OK );
		return false;
	}
	memset( messageIn, '\0', 128 );
	return true;
}

//------------------------------------------------------------------------------
//Meathod:  Socket::ClientSocket::SMTPClient::removeUser( char* )
//Purpose:   Remove the user id from the email address
//Variables: temp, send
//Returns:   void
//------------------------------------------------------------------------------
bool SMTPClient::checkError( char* temp )
{
	static HWND error;
	if( strncmp( temp, "421", 3 ) == 0 )
	{
		MessageBox( error, "421: Service Not Available", "Error Code", MB_OK );
		return true;
	}
	else if( strncmp( temp, "447", 3 ) == 0 )
	{
		MessageBox( error, "447: Outgoing Message Timeout", "Error Code", MB_OK );
		return true;
	}
	else if( strncmp( temp, "500", 3 ) == 0 )
	{
		MessageBox( error, "500: Command Syntax Error", "Error Code", MB_OK );
		return true;
	}
	else if ( strncmp( temp, "251", 3) == 0 )
	{
		MessageBox( error, "251: User Not Local", "Error Code", MB_OK );
		return true;
	}
	else if ( strncmp( temp, "221", 3) == 0 )
	{
		MessageBox( error, "221: Server Ending Connection", "Error Code", MB_OK );
		return true;
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::removeUser( char* )
//Purpose:   Remove the user id from the email address
//Variables: temp, send
//Returns:   void
//------------------------------------------------------------------------------
void SMTPClient::removeUser( char* temp )
{
	for( int x = 0; x < strlen( temp ); x++ )
	{
		if( temp[x] == '@' )
		{
			char send[strlen( temp ) - x];
			strncpy( send, temp + x + 1, strlen( temp ) - x );
			strcpy( temp, send );
			break;
		}
	}
}

//------------------------------------------------------------------------------
//Meathod:  Socket::ClientSocket::SMTPClient::hwndToChar( HWND, char* )
//Purpose:   Converts the text of a window to a char array
//Variables: convert, temp, length
//Returns:   void
//------------------------------------------------------------------------------
void SMTPClient::hwndToChar( HWND convert, char* temp )
{
	int length = GetWindowTextLength( convert ) + 1;
	GetWindowText( convert, temp, length );
	temp[length] = '\0';
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::DNSPoll( HWND )
//Purpose:   Extracts the domain name from the input address and sends a DNS 
//			 query. Will eventually return an ip address to go along with it.
//Variables: hwndTemp, IPAddress, connect, popup, temp, send
//Returns:   void
//------------------------------------------------------------------------------
void SMTPClient::DNSPoll( HWND hwndTemp, char* IPAddress )
{
	static HWND popup;
	char temp[128]; 
	hwndToChar( hwndTemp, temp );
	removeUser( temp );
}

//------------------------------------------------------------------------------
//Meathod:   Socket::ClientSocket::SMTPClient::SendDataToServer( HWND )
//Purpose:   Sends Data to the Server
//Variables: send, IPAddress, connect, popup, temp, send
//Returns:   void
//------------------------------------------------------------------------------
void SMTPClient::SendDataToServer( HWND send, char* IPAddress )
{
	static HWND popup;
	char temp[128]; memset( temp, '\0', 128 );
	hwndToChar( send, temp );
	MessageBox( popup, temp, "The DNS Query is for", MB_OK );
    //connect.ConnectToServer( IPAddress, 53 );
	//connect.SendData( send );
	//connect.RecvData( send, length );
}

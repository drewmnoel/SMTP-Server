#include <windows.h>
#include "gui_socket.h"
#include <ctime>
// Define control identifiers
#define IDC_TO 1001
#define IDC_LABEL_TO 1002

#define IDC_FROM 1003
#define IDC_LABEL_FROM 1004

#define IDC_SUBJECT 1005
#define IDC_LABEL_SUBJECT 1006

#define IDC_EDIT 1007
#define IDC_BUTTON 1008
#define IDC_CHECK 1009

 #define YELLOW  RGB(255,255,0)

void DNSPoll( HWND, char*, ClientSocket* );
void SendDataToServer( HWND, char*, ClientSocket* );
void hwndToChar( HWND, char* );
bool checkError( char* );
void removeUser( char* );
void sendMessageBody( ClientSocket* );

static HINSTANCE hInstance = NULL;
char szClassName[] = "ControlClasses";
// A function to set a control's text to
//     the default font

int SetDefaultFont(int identifier, HWND hwnd)
{
    SendDlgItemMessage( hwnd, identifier, WM_SETFONT,
						(WPARAM)GetStockObject(DEFAULT_GUI_FONT),
						MAKELPARAM(TRUE, 0));
    return 0;
}

// A function to create static text
HWND CreateStatic(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
{
    HWND hStaticTemp;
    hStaticTemp = CreateWindowEx( 0, "STATIC", tempText,
									WS_CHILD | WS_VISIBLE,
									x, y, width, height, hwnd, 
									(HMENU)identifier, hInstance, NULL);
    return hStaticTemp;
}

// A function to create an address box
HWND CreateAddress(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
{
    HWND hAddressTemp;
    hAddressTemp = CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT", tempText,
									WS_CHILD | WS_VISIBLE,
									x, y, width, height, hwnd, 
									(HMENU)identifier, hInstance, NULL);
    return hAddressTemp;
}

// A function to create a textarea
HWND CreateEdit(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
{
    HWND hEditTemp;
    hEditTemp = CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT", tempText,
									WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL| ES_MULTILINE | WS_VSCROLL,
									x, y, width, height, hwnd, 
									(HMENU)identifier, hInstance, NULL);
    return hEditTemp;
}

// A function to create a button
HWND CreateButton(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
{
    HWND hButtonTemp;
    hButtonTemp = CreateWindowEx( 0, "BUTTON", tempText, 
									WS_CHILD | WS_VISIBLE, 
									x, y, width, height, hwnd, 
									(HMENU)identifier, hInstance, NULL );
    return hButtonTemp;
}

// A function to create a checkbox
HWND CreateCheck(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
{
    HWND hCheckTemp;
    hCheckTemp = CreateWindowEx( 0, "BUTTON", tempText,
									WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
									x, y, width, height, hwnd, 
									(HMENU)identifier, hInstance, NULL);
    return hCheckTemp;
}

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
//char szClassName[ ] = "WindowsApp";

int WINAPI WinMain( HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil )
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default color as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    /*
       CreateWindowEx - The CreateWindowEx function creates an overlapped, 
       pop-up, or child window with an extended window style
       
       http://msdn.microsoft.com/en-us/library/ms632680(VS.85).aspx
       
    */
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           "Mail Client",       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nFunsterStil);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    /*
       The GetMessage function retrieves a message from the calling thread's message queue
    */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     
    // Control handles
	static HWND hwndTo;
	static HWND hwndToLabel;
	
	static HWND hwndFrom;
	static HWND hwndFromLabel;
	
	static HWND hwndSubject;
	static HWND hwndSubjectLabel;
	
    static HWND hwndEdit;
    static HWND hwndButton;
    static HWND hwndCheck;
    switch (message)
    {
		//(char* tempText, int x, int y, int width, int height, int identifier, HWND hwnd)
        case WM_CREATE:
			// To Fields
			char Label[128]; memset( Label, '\0', 128 );
			strcpy( Label, "To: " );
			hwndToLabel = CreateStatic( Label, 10, 10, 75, 20, IDC_LABEL_TO, hwnd );
			SetDefaultFont( IDC_LABEL_TO, hwnd );
			memset( Label, '\0', 128 ); strcpy( Label, "to@To.com" );
			hwndTo = CreateAddress( Label, 85, 10, 200, 20, IDC_TO, hwnd );
			SetDefaultFont( IDC_TO, hwnd );
			
			// From Fields
			memset( Label, '\0', 128 ); strcpy( Label, "From: " );
			hwndFromLabel = CreateStatic( Label, 10, 30, 75, 20, IDC_LABEL_FROM, hwnd );
			SetDefaultFont( IDC_LABEL_FROM, hwnd );
			memset( Label, '\0', 128 ); strcpy( Label, "from@From.com" );
			hwndFrom = CreateAddress( Label, 85, 30, 200, 20, IDC_FROM, hwnd );
			SetDefaultFont( IDC_FROM, hwnd );
			
			// Subect
			memset( Label, '\0', 128 ); strcpy( Label, "Subject: " );
			hwndSubjectLabel = CreateStatic( Label, 10, 55, 75, 20, IDC_LABEL_SUBJECT, hwnd );
			SetDefaultFont( IDC_LABEL_SUBJECT, hwnd );
			memset( Label, '\0', 128 ); strcpy( Label, "Test Message" );
			hwndSubject = CreateAddress( Label, 85, 55, 200, 20, IDC_SUBJECT, hwnd );
			SetDefaultFont( IDC_SUBJECT, hwnd );
			
			// Message Body
			memset( Label, '\0', 128 ); strcpy( Label, "test test" );
			hwndEdit = CreateEdit( Label, 10, 80, 506, 215, IDC_EDIT, hwnd );
			SetDefaultFont( IDC_EDIT, hwnd );
			
			// Send Message Button
			memset( Label, '\0', 128 ); strcpy( Label, "Send Message" );
			hwndButton = CreateButton( Label, 10, 300, 100, 25, IDC_BUTTON, hwnd );
			SetDefaultFont( IDC_BUTTON, hwnd );
			// Reset Fields CheckBox
			memset( Label, '\0', 128 ); strcpy( Label, "Reset Fields" );
			hwndCheck = CreateCheck( Label, 130, 300, 75, 20, IDC_CHECK, hwnd );
			SetDefaultFont(IDC_CHECK, hwnd);
			memset( Label, '\0', 128 );
        break;
		
        case WM_COMMAND:
			// If a button is clicked...
			//HIWORD macro retrieves the high-order word from the given 32-bit value. 
			//The parent window of the button receives the BN_CLICKED notification code 
			//through the WM_COMMAND message.
			/*
			  wParam - The low-order word contains the button's control identifier. 
					   The high-order word specifies the notification message. 
			  lParam - A handle to the button. 
			*/
			if(HIWORD(wParam) == BN_CLICKED)
			{
				//LOWORD macro retrieves the low-order word from the specified value. 
				switch( LOWORD( wParam ) )
				{
					// ...And the button is the one with the
					//     identifier IDC_BUTTON...
					case IDC_BUTTON:
						LRESULT checkState;
						
						static HWND popup;
						char IPAddress[20] = "127.0.0.1";
						char Message[128]; memset( Message, '\0', sizeof( Message ) );
						char convert[128]; memset( convert, '\0', sizeof( convert ) );
						static char endKey[1]; memset( endKey, '\0', sizeof( endKey ) );
						//http://msdn.microsoft.com/en-us/library/system.windows.forms.textbox.autocompletesource.aspx
						
						//ClientSocket connectDNS;
						//connectDNS.ConnectToServer( IPAddress, 53 );
						//DNSPoll( hwnd, IPAddress, &connectDNS );
						
						ClientSocket connectSMTP;
						connectSMTP.ConnectToServer( "127.0.0.1", 25 );
						
						if( connectSMTP.RecvData( Message, 128 ) ) { //220 smtp.example.com ESMTP Postfix
							if( checkError( Message ) )
								return 1;
							if( strncmp( Message, "220", 3 ) != 0) {
								SetWindowText(hwndFrom, "");
								return 1;
							}
							MessageBox( popup, Message, "220", MB_OK );
						}
						else {
							MessageBox( popup, Message, "Error", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						
						
						strcpy( Message, "HELO " );
						hwndToChar( hwndFrom, convert );
						removeUser( convert );
						strcat( Message, convert );
						strcat( Message, "\0" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "HELO", MB_OK );
						else {
							MessageBox( popup, "Error", "HELO", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						
						if( connectSMTP.RecvData( Message, 128 ) ) {
							if( checkError( Message ) )
								break;
							if( strncmp( Message, "250", 3 ) != 0) {
								SetWindowText(hwndFrom, "");
								return 1;
							}
							MessageBox( popup, Message , "250 Ok", MB_OK );
							}
						else {
							MessageBox( popup, "Error" , "250 Ok", MB_OK );
							return 1;
						} 
						memset( Message, '\0', sizeof( Message ) );
						
						
						strcpy( Message, "MAIL FROM:<" );
						hwndToChar( hwndFrom, convert );
						strcat( Message, convert );
						strcat( Message, ">" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "MAIL FROM", MB_OK );
						else {
							MessageBox( popup, "Error", "MAIL FROM", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						
						if( connectSMTP.RecvData( Message, 128 ) ) {
							if( checkError( Message ) )
								break;
							if( strncmp( Message, "250", 3 ) != 0) {
								SetWindowText(hwndFrom, "");
								return 1;
							}
							MessageBox( popup, Message , "250 Ok", MB_OK );
							}
						else {
							MessageBox( popup, "Error" , "250 Ok", MB_OK );
							return 1;
						} 
						memset( Message, '\0', sizeof( Message ) );
						
						
						strcpy( Message, "RCPT TO:<" );
						hwndToChar( hwndTo, convert );
						strcat( Message, convert );
						strcat( Message, ">\0" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "RCPT TO", MB_OK );
						else {
							MessageBox( popup, "Error", "RCPT TO", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						
						if( connectSMTP.RecvData( Message, 128 ) ) {
							if( checkError( Message ) )
								break;
							if( strncmp( Message, "250", 3 ) != 0) {
								SetWindowText(hwndFrom, "");
								return 1;
							}
							MessageBox( popup, Message , "250 Ok", MB_OK );
							}
						else {
							MessageBox( popup, "Error" , "250 Ok", MB_OK );
							return 1;
						} 
						memset( Message, '\0', sizeof( Message ) );
						
						
						strcpy( Message, "DATA\0" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "DATA", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						
						
						if( connectSMTP.RecvData( Message, 128 ) ) {
							if( checkError( Message ) )
								break;
							if( strncmp( Message, "354", 3 ) != 0) 
							{
								SetWindowText(hwndFrom, "");
								return 1;
							}
							else
							{
								char* findEndKey = strstr( Message, "<CR><LF>" );
								strncpy( endKey, findEndKey + 8, 1 );
								strcat( endKey, "\0" );
								MessageBox( popup, endKey, "354 End Data", MB_OK );
							}
						}
						memset( Message, '\0', sizeof( Message ) );
						
						/*
						 *
						 *
						 *
						 */
						 
						strcpy( Message, "From: <" );
						hwndToChar( hwndFrom, convert );
						strcat( Message, convert );
						strcat( Message, ">\0" );
						
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "From: <>", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						strcpy( Message, "To: <" );
						hwndToChar( hwndTo, convert );
						strcat( Message, convert );
						strcat( Message, ">\0" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "To: <>", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						strcpy( Message, "Date: " );
						char date[10]; _strdate( date );
						strcat( Message, date );
						strcat( Message, " " );
						char time[10]; _strtime( time );
						strcat( Message, time );
						strcat( Message, "\0");
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "Date: ", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						
						strcpy( Message, "Subject: " );
						hwndToChar( hwndSubject, convert );
						strcat( Message, convert );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, Message, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "Subject: ", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						memset( convert, '\0', sizeof( convert ) );
						
						char* convertBody = (char*)malloc(4096); memset( convertBody, '\0', sizeof( convertBody ) ); 
						char* messageBody = (char*)malloc(4096); memset( messageBody, '\0', sizeof( messageBody ) );
						strcpy( messageBody, "\n" );
						hwndToChar( hwndEdit, convertBody );
						strcat( messageBody, convertBody );
						strcat( messageBody, "\n" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, messageBody, "DATA", MB_OK );
						else {
							MessageBox( popup, "Error", "Body: ", MB_OK );
							return 1;
						}
						free( convertBody );
						free( messageBody );
						
						/*
						 *
						 *
						 *
						 */
						 
						if( connectSMTP.SendData( endKey ) )
							MessageBox( popup, endKey, "End Data", MB_OK );
						else {
							MessageBox( popup, "Error", "End Data: ", MB_OK );
							return 1;
						}
						
						if( connectSMTP.RecvData( Message ) ) {
							if( checkError( Message ) )
								break;
							char* findQueued = strstr( Message, "queued as " );
							char queued[100]; memset( queued, '\0', sizeof( queued ) );
							strncpy( queued, findQueued + 9, strlen( Message ) - 17 );
							MessageBox( popup, queued , "queued as", MB_OK );
						}
						else {
							MessageBox( popup, "Error", "Queued: ", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						
						strcpy( Message, "QUIT\n" );
						if( connectSMTP.SendData( Message ) )
							MessageBox( popup, "QUIT", "End Data", MB_OK );
						else {
							MessageBox( popup, "Error", "Quit: ", MB_OK );
							return 1;
						}
						
						if( connectSMTP.RecvData( Message, 128 ) ) {
							if( checkError( Message ) )
								break;
							MessageBox( popup, Message , "End", MB_OK );
						}
						else {
							MessageBox( popup, "Error", "End: ", MB_OK );
							return 1;
						}
						memset( Message, '\0', sizeof( Message ) );
						
						// Check the checkbox and clear the fields if checked
						checkState = SendDlgItemMessage( hwnd, IDC_CHECK, BM_GETCHECK, 0, 0 ); 
						if(checkState == BST_CHECKED)
						{
							SetWindowText(hwndFrom, "");
							SetWindowText(hwndTo, "");
							SetWindowText(hwndEdit, "");						
						}
					break;
				}

			}
        break;
		
        case WM_DESTROY:
			PostQuitMessage(0);
        break;
        
        default:
			return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


//------------------------------------------------------------------------------
//Function:  DNSPoll( HWND )
//Purpose:   Extracts the domain name from the input address and sends a DNS 
//			 query. Will eventually return an ip address to go along with it.
//Variables: hwndTemp, IPAddress, connect, popup, temp, send
//Returns:   void
//------------------------------------------------------------------------------
void DNSPoll( HWND hwndTemp, char* IPAddress, ClientSocket* connect )
{
	static HWND popup;
	char temp[128]; 
	hwndToChar( hwndTemp, temp );
	removeUser( temp );
}

//------------------------------------------------------------------------------
//Function:  SendDataToServer( HWND )
//Purpose:   Sends Data to the Server
//Variables: send, IPAddress, connect, popup, temp, send
//Returns:   void
//------------------------------------------------------------------------------
void SendDataToServer( HWND send, char* IPAddress, ClientSocket* connect )
{
	static HWND popup;
	char temp[128]; memset( temp, '\0', 128 );
	hwndToChar( send, temp );
	MessageBox( popup, temp, "The DNS Query is for", MB_OK );
    //connect.ConnectToServer( IPAddress, 53 );
	//connect.SendData( send );
	//connect.RecvData( send, length );
}

//------------------------------------------------------------------------------
//Function:  hwndToChar( HWND, char* )
//Purpose:   Converts the text of a window to a char array
//Variables: convert, temp, length
//Returns:   void
//------------------------------------------------------------------------------
void hwndToChar( HWND convert, char* temp )
{
	int length = GetWindowTextLength( convert ) + 1; 
	GetWindowText( convert, temp, length );
	temp[length] = '\0';
}

//------------------------------------------------------------------------------
//Function:  removeUser( char* )
//Purpose:   Remove the user id from the email address
//Variables: temp, send
//Returns:   void
//------------------------------------------------------------------------------
void removeUser( char* temp )
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
//Function:  removeUser( char* )
//Purpose:   Remove the user id from the email address
//Variables: temp, send
//Returns:   void
//------------------------------------------------------------------------------
bool checkError( char* temp )
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

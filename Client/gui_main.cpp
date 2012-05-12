#include <windows.h>
#include <iostream>
#include "SMTPClient.h"
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
			memset( Label, '\0', 128 ); strcpy( Label, "to@To.com; to@from.com" );
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
						char IPAddress[20] = "129.21.112.89";
						char Message[128]; memset( Message, '\0', 128 );
						char convert[128]; memset( convert, '\0', 128 );
						static char endKey[1]; memset( endKey, '\0', sizeof( endKey ) );
						//http://msdn.microsoft.com/en-us/library/system.windows.forms.textbox.autocompletesource.aspx
						
						//ClientSocket connectDNS;
						//connectDNS.ConnectToServer( IPAddress, 53 );
						//DNSPoll( hwnd, IPAddress, &connectDNS );
						
						SMTPClient connectSMTP;
						connectSMTP.ConnectToServer( IPAddress, 8080 );
						
						if( !connectSMTP.recieve220( popup ) )
							return 1;
						
						if( !connectSMTP.sendHELO( hwndFrom, popup ) )
							return 1;
						
						if( !connectSMTP.recieve250( popup ) )
							return 1;
						
						if( !connectSMTP.sendMailFrom( hwndFrom, popup ) )
							return 1;
						
						if( !connectSMTP.recieve250( popup ) )
							return 1;
						
						if( !connectSMTP.sendRecieptTo( hwndTo, popup ) )
							return 1;

						//no need for recieve250 after this meathod	
						
						if( !connectSMTP.data( popup ) )
							return 1;
						
						if( !connectSMTP.recieve354( popup ) )
							return 1;
						 
						if( !connectSMTP.sendMessage( hwndTo, hwndFrom, hwndSubject, hwndEdit, popup ) )
							return 1;

						if( !connectSMTP.return354( popup ) ){}
						 	// 1;

						if( !connectSMTP.recieveQueue( popup ) )
							return 1;
						
						if( !connectSMTP.sendQuit( popup ) )
							return 1;

						if( !connectSMTP.recieveEnd( popup ) )
							return 1;
						
						strcpy( Message, "Message Sent" );
						MessageBox( popup, Message, "End", MB_OK );
						memset( Message, '\0', 128 );
						
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

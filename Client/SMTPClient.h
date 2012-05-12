#include "gui_socket.h"

#include "windows.h"

class SMTPClient : public ClientSocket
{
	private:
		char messageOut[128];
		char messageIn[128];
		char convert[128];
		char endKey[1];
		//static HWND popup;
	protected:
		bool checkError( char* );
		void removeUser( char* );
		void hwndToChar( HWND, char* );
	public:
		SMTPClient( );
        ~SMTPClient( );
		bool recieve220( HWND );
		bool sendHELO( HWND, HWND );
		bool recieve250( HWND );
		bool sendMailFrom( HWND, HWND );
		bool sendRecieptTo( HWND, HWND );
		bool data( HWND );
		bool recieve354( HWND );
		bool sendMessage( HWND, HWND, HWND, HWND, HWND );
		bool return354( HWND );
		bool recieveQueue( HWND );
		bool sendQuit( HWND );
		bool recieveEnd( HWND );
};
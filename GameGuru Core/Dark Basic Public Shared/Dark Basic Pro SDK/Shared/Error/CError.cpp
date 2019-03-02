// include definition
#include <stdio.h>
#include "cerror.h"
#include "globstruct.h"
#pragma comment ( lib, "user32.lib" )
#define DB_PRO 1

// Handler Passed into DLL
CRuntimeErrorHandler* g_pErrorHandler = NULL;

// Externals
extern bool g_bCascadeQuitFlag;
extern char g_strErrorClue[512];

void timestampactivity(int i, char* desc_s);

void Error1 ( char* szMessage )
{
	if(g_pErrorHandler)
		if(g_pErrorHandler->dwErrorCode==0)
			RunTimeError(RUNTIMEERROR_GENERICERROR, szMessage);
}

void Message ( int iID, char* szMessage, char* szTitle )
{
	MessageBox ( NULL, szMessage, szTitle, MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_TOPMOST );
}

void RunTimeError ( DWORD dwErrorCode )
{
	//C++Conversion
	char szErrorString[256];
	sprintf ( szErrorString , "Runtime Error: %i (%s)" , dwErrorCode, g_strErrorClue );
	timestampactivity(0, szErrorString); // PE:
	MessageBox ( NULL, szErrorString, "Error", MB_OK | MB_TOPMOST );
	ExitProcess ( 0 );

	// Assign Run Time Error To Global Error Handler (lee - 240715 - only first error!)
	/*if ( g_pErrorHandler ) 
	{
		if ( g_pErrorHandler->dwErrorCode==0 )
		{
			// record first runtime error code, then trigger an exit of the EXE
			g_pErrorHandler->dwErrorCode = dwErrorCode;
			g_bCascadeQuitFlag = true;
		}
	}*/
}

void RunTimeWarning ( DWORD dwErrorCode )
{
}

void RunTimeSoftWarning ( DWORD dwErrorCode )
{
}

void RunTimeError(DWORD dwErrorCode, LPSTR pExtraErrorString)
{
	if ( pExtraErrorString ) 
	{
		char szErrorString[256];
		sprintf ( szErrorString , "Runtime Error: %i : %s (%s)" , dwErrorCode, pExtraErrorString, g_strErrorClue );
		timestampactivity(0, szErrorString); // PE:
		MessageBox ( NULL, szErrorString, "Error", MB_OK | MB_TOPMOST );
		ExitProcess ( 0 );
	}
	else
	{
		RunTimeError ( dwErrorCode );
	}
}

void LastSystemError()
{
	LPVOID lpBuffer;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					   NULL, GetLastError ( ), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					   (LPTSTR)&lpBuffer, 0, NULL)) return;
	MessageBox ( NULL, (LPCTSTR)lpBuffer, "System Error", MB_OK );
	LocalFree ( lpBuffer );
}

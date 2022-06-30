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

#ifndef NOSTEAMORVIDEO
void timestampactivity(int i, char* desc_s);
#endif

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
	#ifndef NOSTEAMORVIDEO
	timestampactivity(0, szErrorString); // PE:
	#endif
	MessageBox ( NULL, szErrorString, "Error", MB_OK | MB_TOPMOST );
	//Debug trick , trigger Visual Studio breakpoint. Then use F10 to go to the place the error was generated. also comment out ExitProcess ( 0 );
	__debugbreak();
	ExitProcess ( 0 );
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
		#ifndef NOSTEAMORVIDEO
		timestampactivity(0, szErrorString); // PE:
		#endif
		MessageBox ( NULL, szErrorString, "Error", MB_OK | MB_TOPMOST );
		__debugbreak();
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


//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#include <windows.h> 
#include <windowsx.h>

#include "ftp.h"
#include "HTTPComm.h"
#include "cftpc.h"
#include "wininet.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"

#pragma comment ( lib, "wininet.lib" )

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// GLOBALS ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

extern GlobStruct*			g_pGlob;
///extern PTR_FuncCreateStr		g_pCreateDeleteStringFunction;
extern HWND					GlobalHwndCopy;
extern DBPRO_GLOBAL char					m_pWorkString[_MAX_PATH];

// new HTTP data globals
char								g_pFeedbackAreaString [ 1024 ];

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK void FTPConstructor ( void )
{
	GlobalHwndCopy = g_pGlob->hWnd;
}

DARKSDK void FTPDestructor ( void )
{
}

DARKSDK void FTPSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void FTPPassCoreData( LPVOID pGlobPtr )
{
	// Held in Core, used here..
	g_pGlob = (GlobStruct*)pGlobPtr;
	///g_pCreateDeleteStringFunction = g_pGlob->CreateDeleteString;
}

DARKSDK LPSTR FTPGetReturnStringFromWorkString(void)
{
	LPSTR pReturnString=NULL;
	if(m_pWorkString)
	{
		DWORD dwSize=strlen(m_pWorkString);
		g_pGlob->CreateDeleteString((char**)&pReturnString, dwSize+1);
		strcpy(pReturnString, m_pWorkString);
	}
	return pReturnString;
}

DARKSDK void FTPConnectEx( DWORD dwString, DWORD dwString2, DWORD dwString3, int iUseWindow )
{
	if(!FTP_ConnectEx((char*)dwString, (char*)dwString2, (char*)dwString3, iUseWindow))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCONNECTIONFAILED);
}

DARKSDK void FTPConnect( DWORD dwString, DWORD dwString2, DWORD dwString3 )
{
	if(!FTP_Connect((char*)dwString, (char*)dwString2, (char*)dwString3))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCONNECTIONFAILED);
}

DARKSDK void FTPSetDir( DWORD dwString )
{
	if(!FTP_SetDir((char*)dwString))
		RunTimeSoftWarning(RUNTIMEERROR_FTPPATHCANNOTBEFOUND);
}

DARKSDK void FTPFindFirst(void)
{
	FTP_FindFirst();
}

DARKSDK void FTPFindNext(void)
{
	FTP_FindNext();
}

DARKSDK void FTPPutFileCore( DWORD dwString )
{
	if(!FTP_PutFile((char*)dwString))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCANNOTPUTFILE);
}

DARKSDK void FTPPutFile( DWORD dwString )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, (LPSTR)dwString);
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	FTPPutFileCore( (DWORD)VirtualFilename );
	g_pGlob->Encrypt( VirtualFilename );
}

DARKSDK void FTPDeleteFile( DWORD dwString )
{
	if(!FTP_DeleteFile((char*)dwString))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCANNOTDELETEFILE);
}

DARKSDK void FTPGetFile( DWORD dwString, DWORD dwString2 )
{
	if(!FTP_GetFile((char*)dwString, (char*)dwString2, 0, 0))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCANNOTGETFILE);
}

DARKSDK void FTPGetFile( DWORD dwString, DWORD dwString2, int iFlag )
{
	if(!FTP_GetFile((char*)dwString, (char*)dwString2, iFlag, 1))
		RunTimeSoftWarning(RUNTIMEERROR_FTPCANNOTGETFILE);
}

DARKSDK void FTPDisconnect(void)
{
	FTP_Disconnect(0);
}

DARKSDK void FTPDisconnectEx( int iFlag )
{
	FTP_Disconnect(iFlag);
}

DARKSDK void FTPProceed(void)
{
	FTP_ControlDownload(0);
}

DARKSDK void FTPTerminate(void)
{
	FTP_TerminateDownload();
}

DARKSDK int FTPGetStatus(void)
{
	return FTP_GetStatus();
}

DARKSDK int FTPGetFailure(void)
{
	return FTP_GetFailureState();
}

DARKSDK char* FTPGetError( char* pDestStr )
{
	FTP_GetError(m_pWorkString);
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	LPSTR pReturnString=FTPGetReturnStringFromWorkString();
	return pReturnString;
}

DARKSDK char* FTPGetDir( char* pDestStr )
{
	FTP_GetDir(m_pWorkString);
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	LPSTR pReturnString=FTPGetReturnStringFromWorkString();
	return pReturnString;
}

DARKSDK char* FTPGetFileName( char* pDestStr )
{
	FTP_GetFileName(m_pWorkString);
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
	LPSTR pReturnString=FTPGetReturnStringFromWorkString();
	return pReturnString;
}

DARKSDK int FTPGetFileType(void)
{
	return FTP_GetFileType();
}

DARKSDK int FTPGetFileSize(void)
{
	return FTP_GetFileSize();
}

DARKSDK int FTPGetProgress(void)
{
	return FTP_GetProgress();
}

// HTTP Implementations

DARKSDK void HTTPConnect ( LPSTR pUrl )
{
	char* lpUrl = pUrl;
	if (lpUrl && *lpUrl)
		HTTP_Connect ( lpUrl );
}

DARKSDK void HTTPConnect ( DWORD dwUrl, DWORD port )
{
	char* lpUrl = (char*)dwUrl;
	if (lpUrl && *lpUrl)
		HTTP_Connect ( lpUrl, port );
}

DARKSDK void HTTPConnect ( DWORD dwUrl, DWORD port, int secure )
{
	char* lpUrl = (char*)dwUrl;
	if (lpUrl && *lpUrl)
		HTTP_Connect ( lpUrl, port, secure );
}

DARKSDK LPSTR HTTPRequestData ( char* pDestStr, LPSTR dwVerb, LPSTR dwObjectName, LPSTR dwPostData, DWORD dwAccessFlag )
{
	char* lpVerb = (char*)dwVerb;
	char* lpObjectName = (char*)dwObjectName;
	char* lpPostData = (char*)dwPostData;

	// 20120418 IanM - Ensure that the verb is set
	if (!lpVerb || !*lpVerb)
	{
		if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);
		return NULL;
	}

	// 20120418 IanM - Don't care about these so much, just as long as they are non-NULL
	if (!lpObjectName)
		lpObjectName = "";
	if (!lpPostData)
		lpPostData = "";

	// default HTTP comm strings
	DWORD dwPostDataSize = 0;
	if ( lpPostData ) dwPostDataSize = strlen ( lpPostData );
	LPSTR pHeader = new char[256];
	wsprintf ( pHeader, "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n", dwPostDataSize );
	DWORD dwHeaderSize = -1L;

	// send data to get data
	LPSTR pReturnData = HTTP_RequestData ( lpVerb, lpObjectName, pHeader, dwHeaderSize, lpPostData, dwPostDataSize, dwAccessFlag );

	// delete old string
	if(pDestStr) g_pGlob->CreateDeleteString((char**)&pDestStr, 0);

	// make new string
	LPSTR pReturnString = NULL;
	DWORD dwSize = strlen(g_pFeedbackAreaString);
	if ( pReturnData ) dwSize = strlen(pReturnData);
	g_pGlob->CreateDeleteString((char**)&pReturnString, dwSize+1);
	if ( pReturnData ) 
		strcpy ( pReturnString, pReturnData );
	else
		strcpy ( pReturnString, g_pFeedbackAreaString );

	// delete temp return data
	SAFE_DELETE(pReturnData);
	SAFE_DELETE(pHeader);

	// return new string
	return pReturnString;
}

DARKSDK LPSTR HTTPRequestData ( LPSTR dwVerb, LPSTR dwObjectName, LPSTR dwPostData )
{
	// 20120416 IanM - Cleared default security type
	return HTTPRequestData ( NULL, dwVerb, dwObjectName, dwPostData, 0 );
}

DARKSDK void HTTPDisconnect ( void )
{
	// disconnect when data exchange complete
	if ( HTTP_Disconnect ( ) )
		return;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// DARK SDK SECTION //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifdef DARKSDK_COMPILE

void ConstructorFTP ( void )
{
	Constructor ( );
}

void DestructorFTP ( void )
{
	Destructor ( );
}

void SetErrorHandlerFTP ( LPVOID pErrorHandlerPtr )
{
	SetErrorHandler ( pErrorHandlerPtr );
}

void PassCoreDataFTP ( LPVOID pGlobPtr )
{
	PassCoreData ( pGlobPtr );
}

void dbFTPConnect ( char* szString, char* szString2, char* szString3, int iUseWindow )
{
	ConnectEx ( ( DWORD ) szString, ( DWORD ) szString2, ( DWORD ) szString3, iUseWindow );
}

void dbFTPConnect ( char* szString, char* szString2, char* szString3 )
{
	Connect ( ( DWORD ) szString, ( DWORD ) szString2, ( DWORD ) szString3 );
}

void dbFTPDisconnect ( void )
{
	Disconnect ( );
}

void dbFTPDisconnect ( int iFlag )
{
	DisconnectEx ( iFlag );
}

void dbFTPSetDir ( char* szString )
{
	SetDir ( ( DWORD ) szString );
}

void dbFTPPutFile ( char* szString )
{
	PutFile ( ( DWORD ) szString );
}

void dbFTPGetFile ( char* szString, char* szString2 )
{
	GetFile ( ( DWORD ) szString, ( DWORD ) szString2 );
}

void dbFTPGetFile ( char* szString, char* szString2, int iFlag )
{
	GetFile ( ( DWORD ) szString, ( DWORD ) szString2, iFlag );
}

void dbFTPDeleteFile ( char* szString )
{
	DeleteFile ( ( DWORD ) szString );
}

void dbFTPFindFirst ( void )
{
	FindFirst ( );
}

void dbFTPFindNext ( void )
{
	FindNext ( );
}

void dbFTPTerminate ( void )
{
	Terminate ( );
}

void dbFTPProceed ( void )
{
	Proceed ( );
}

char* dbGetFTPError ( void )
{
	static char* szReturn = NULL;
	DWORD		 dwReturn = GetError ( NULL );

	szReturn = ( char* ) dwReturn;

	return szReturn;
}

char* dbGetFTPDir ( void )
{
	static char* szReturn = NULL;
	DWORD		 dwReturn = GetDir ( NULL );

	szReturn = ( char* ) dwReturn;

	return szReturn;
}

char* dbGetFTPFileName ( void )
{
	static char* szReturn = NULL;
	DWORD		 dwReturn = GetFileName ( NULL );

	szReturn = ( char* ) dwReturn;

	return szReturn;
}

int	dbGetFTPFileType ( void )
{
	return GetFileType ( );
}

int	dbGetFTPFileSize ( void )
{
	return GetFileSize ( );
}

int	dbGetFTPProgress ( void )
{
	return GetProgress ( );
}

int	dbGetFTPStatus ( void )
{
	return GetStatus ( );
}

int	dbGetFTPFailure ( void )
{
	return GetFailure ( );
}

#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
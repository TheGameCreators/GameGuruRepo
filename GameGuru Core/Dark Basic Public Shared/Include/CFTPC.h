#ifndef _CFTP_H_
#define _CFTP_H_

//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// DEFINES ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifndef DARKSDK_COMPILE
	#define DARKSDK __declspec ( dllexport )
	#define DBPRO_GLOBAL 
#else
	#define DARKSDK static
	#define DBPRO_GLOBAL static
#endif

//#define SAFE_DELETE( p )       { if ( p ) { delete ( p );       ( p ) = NULL; } }
//#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
//#define SAFE_DELETE_ARRAY( p ) { if ( p ) { delete [ ] ( p );   ( p ) = NULL; } }

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifdef DARKSDK_COMPILE
		void	ConstructorFTP		( void );
		void	DestructorFTP		( void );
		void	SetErrorHandlerFTP	( LPVOID pErrorHandlerPtr );
		void	PassCoreDataFTP		( LPVOID pGlobPtr );
#endif

DARKSDK void	FTPConstructor			( void );
DARKSDK void	FTPDestructor			( void );
DARKSDK void	FTPSetErrorHandler		( LPVOID pErrorHandlerPtr );
DARKSDK void	FTPPassCoreData		( LPVOID pGlobPtr );

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// FTP

DARKSDK void	FTPConnectEx			( DWORD dwString, DWORD dwString2, DWORD dwString3, int iUseWindow );
DARKSDK void	FTPConnect				( DWORD dwString, DWORD dwString2, DWORD dwString3 );
DARKSDK void	FTPDisconnect			( void );
DARKSDK void	FTPDisconnectEx		( int iFlag );

DARKSDK void	FTPSetDir				( DWORD dwString );
DARKSDK void	FTPPutFile				( DWORD dwString );
DARKSDK void	FTPGetFile				( DWORD dwString, DWORD dwString2 );
DARKSDK void	FTPGetFile				( DWORD dwString, DWORD dwString2, int iFlag );
DARKSDK void	FTPDeleteFile			( DWORD dwString );
DARKSDK void	FTPFindFirst			( void );
DARKSDK void	FTPFindNext			( void );
DARKSDK void	FTPTerminate			( void );
DARKSDK void	FTPProceed				( void );

DARKSDK DWORD	GetFTPError			( DWORD pDestStr );
DARKSDK DWORD	GetFTPDir				( DWORD pDestStr );
DARKSDK DWORD	GetFTPFileName			( DWORD pDestStr );
DARKSDK int		GetFTPFileType			( void );
DARKSDK int		GetFTPFileSize			( void );
DARKSDK int		GetFTPProgress			( void );
DARKSDK int		GetFTPStatus			( void );
DARKSDK int		GetFTPFailure			( void );

DARKSDK void	HTTPConnect			( LPSTR dwUrl );
DARKSDK LPSTR	HTTPRequestData		( LPSTR dwVerb, LPSTR dwObjectName, LPSTR dwPostData );
DARKSDK void	HTTPDisconnect		( void );

#ifdef DARKSDK_COMPILE
		 void	dbFTPConnect		( char* szString, char* szString2, char* szString3, int iUseWindow );
		 void	dbFTPConnect		( char* szString, char* szString2, char* szString3 );
		 void	dbFTPDisconnect		( void );
		 void	dbFTPDisconnect		( int iFlag );

		 void	dbFTPSetDir			( char* dwString );
		 void	dbFTPPutFile		( char* dwString );
		 void	dbFTPGetFile		( char* dwString, char* dwString2 );
		 void	dbFTPGetFile		( char* dwString, char* dwString2, int iFlag );
		 void	dbFTPDeleteFile		( char* dwString );
		 void	dbFTPFindFirst		( void );
		 void	dbFTPFindNext		( void );
		 void	dbFTPTerminate		( void );
		 void	dbFTPProceed		( void );

		 char*	dbGetFTPError		( void );
		 char*	dbGetFTPDir			( void );
		 char*	dbGetFTPFileName	( void );
		 int	dbGetFTPFileType	( void );
		 int	dbGetFTPFileSize	( void );
		 int	dbGetFTPProgress	( void );
		 int	dbGetFTPStatus		( void );
		 int	dbGetFTPFailure		( void );
#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#endif _CFTP_H_
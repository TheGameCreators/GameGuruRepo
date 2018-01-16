#ifndef _CFILE_H_
#define _CFILE_H_

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// DEFINES ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//#ifndef DARKSDK_COMPILE
//	#define DARKSDK __declspec ( dllexport )
//	#define DBPRO_GLOBAL 
//#else
//	#define DARKSDK static
//	#define DBPRO_GLOBAL static
//#endif
#undef DARKSDK
#define DARKSDK

//#define SAFE_DELETE( p )       { if ( p ) { delete ( p );       ( p ) = NULL; } }
//#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
//#define SAFE_DELETE_ARRAY( p ) { if ( p ) { delete [ ] ( p );   ( p ) = NULL; } }

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifdef DARKSDK_COMPILE
	void ConstructorFile ( void );
	void DestructorFile  ( void );
	void SetErrorHandlerFile ( LPVOID pErrorHandlerPtr );
	void PassCoreDataFile( LPVOID pGlobPtr );
#endif

DARKSDK void	FileConstructor						( void );
DARKSDK void	FileDestructor						( void );
DARKSDK void	FileSetErrorHandler					( LPVOID pErrorHandlerPtr );
DARKSDK void	FilePassCoreData					( LPVOID pGlobPtr );

DARKSDK void	SetDir							( LPSTR pString );
DARKSDK void	Dir								( void );
DARKSDK void	DriveList						( void );
DARKSDK void	ChecklistForFiles				( void );
DARKSDK void	ChecklistForDrives				( void );
DARKSDK void	FindFirst						( void );
DARKSDK void	FindNext						( void );
DARKSDK int		CanMakeFile						( DWORD pFilename );
DARKSDK void	MakeFile						( DWORD pFilename );
DARKSDK void	DeleteAFile						( LPSTR pFilename );
DARKSDK void	CopyAFile						( LPSTR pFilename, LPSTR pFilename2 );
DARKSDK void	RenameAFile						( DWORD pFilename, DWORD pFilename2 );
DARKSDK void	MoveAFile						( DWORD pFilename, DWORD pFilename2 );
DARKSDK void	WriteByteToFile					( DWORD pFilename, int iPos, int iByte );
DARKSDK int		ReadByteFromFile				( DWORD pFilename, int iPos );
DARKSDK void	MakeDirectory					( LPSTR pFilename );
DARKSDK void	DeleteDir						( DWORD pFilename );
DARKSDK void	DeleteDirEx						( DWORD pFilename, int iFlag );
DARKSDK void	ExecuteFile						( LPSTR pFilename, LPSTR pFilename2, LPSTR pFilename3 );
DARKSDK void	ExecuteFile						( LPSTR pFilename, LPSTR pFilename2, LPSTR pFilename3, int iFlag );
DARKSDK DWORD	ExecuteFileIndi					( DWORD pFilename, DWORD pFilename2, DWORD pFilename3 );
DARKSDK DWORD	ExecuteFileIndi					( DWORD pFilename, DWORD pFilename2, DWORD pFilename3, int iPriority );
DARKSDK void	StopExecutable					( DWORD hIndiExecuteFileProcess );

DARKSDK void	WriteFilemapValue				( LPSTR pFilemapname, DWORD dwValue );
DARKSDK void	WriteFilemapString				( LPSTR pFilemapname, LPSTR pString );
DARKSDK int		ReadFilemapValue				( LPSTR pFilemapname );
DARKSDK LPSTR	ReadFilemapString				( LPSTR pFilemapname );

DARKSDK void	OpenToRead						( int f, LPSTR pFilename );
DARKSDK void	OpenToWrite						( int f, LPSTR pFilename );
DARKSDK void	CloseFile						( int f );
DARKSDK int		ReadByte						( int f );
DARKSDK int		ReadWord						( int f );
DARKSDK int		ReadLong						( int f );
DARKSDK float	ReadFloat						( int f );
DARKSDK LPSTR	ReadString						( int f );
DARKSDK void	ReadFileBlock					( int f, DWORD pFilename );
DARKSDK void	SkipBytes						( int f, int iSkipValue );
DARKSDK void	ReadDirBlock					( int f, DWORD pFilename );
DARKSDK void	WriteByte						( int f, int iValue );
DARKSDK void	WriteWord						( int f, int iValue );
DARKSDK void	WriteLong						( int f, int iValue );
DARKSDK void	WriteFloat						( int f, float fValue );
DARKSDK void	WriteString						( int f, LPSTR pString );
DARKSDK void	WriteFileBlock					( int f, DWORD pFilename );
DARKSDK void	WriteFileBlockEx				( int f, DWORD pFilename, int iFlag );
DARKSDK void	WriteDirBlock					( int f, DWORD pFilename );

DARKSDK void	ReadMemblock					( int f, int mbi );
DARKSDK void	MakeMemblockFromFile			( int mbi, int f );
DARKSDK void	WriteMemblock					( int f, int mbi );
DARKSDK void	MakeFileFromMemblock			( int f, int mbi );

DARKSDK LPSTR	GetDir							( void );
DARKSDK LPSTR	GetFileName						( void );
DARKSDK int		GetFileType						( void );
DARKSDK LPSTR	GetFileDate						( void );
DARKSDK LPSTR	GetFileCreation					( void );
DARKSDK int		FileExist						( LPSTR pFilename );
DARKSDK int		FileSize						( LPSTR pFilename );
DARKSDK int		PathExist						( LPSTR pFilename );
DARKSDK int		FileOpen						( int f );
DARKSDK int		FileEnd							( int f );

DARKSDK LPSTR	Appname							( void );
DARKSDK DWORD	Windir							( DWORD pDestStr );
DARKSDK LPSTR	Mydocdir						( void );
DARKSDK int		GetExecutableRunning			( DWORD hIndiExecuteFileProcess );

// new ones added in that were duped in 
DARKSDK void FFindCloseFile(void);
DARKSDK void FFindFirstFile(void);
DARKSDK int FGetFileReturnValue(void);
DARKSDK void FFindNextFile(void);
DARKSDK int FGetActualTypeValue(int flagvalue);

DARKSDK BOOL DB_FileExist(char* Filename);

#ifdef DARKSDK_COMPILE
		void	dbSetDir						( char* pString );
		void	dbDir							( void );
		void	dbDriveList						( void );
		void	dbPerformCheckListForFiles		( void );
		void	dbPerformCheckListForDrives		( void );
		void	dbFindFirst						( void );
		void	dbFindNext						( void );
		void	dbMakeFile						( char* pFilename );
		void	dbDeleteFile					( char* pFilename );
		void	dbCopyFile						( char* pFilename, char* pFilename2 );
		void	dbRenameFile					( char* pFilename, char* pFilename2 );
		void	dbMoveFile						( char* pFilename, char* pFilename2 );
		void	dbWriteByteToFile				( char* pFilename, int iPos, int iByte );
		int		dbReadByteFromFile				( char* pFilename, int iPos );
		void	dbMakeDirectory					( char* pFilename );
		void	dbDeleteDirectory				( char* pFilename );
		void	dbDeleteDirectory				( char* pFilename, int iFlag );
		void	dbExecuteFile					( char* pFilename, char* pFilename2, char* pFilename3 );
		void	dbExecuteFile					( char* pFilename, char* pFilename2, char* pFilename3, int iFlag );
		DWORD	dbExecuteExecutable				( char* pFilename, char* pFilename2, char* pFilename3 );
		DWORD	dbExecuteExecutable				( char* pFilename, char* pFilename2, char* pFilename3, int iPriority );
		void	dbStopExecutable				( DWORD hIndiExecuteFileProcess );

		void	dbWriteFilemapValue				( char* pFilemapname, DWORD dwValue );
		void	dbWriteFilemapString			( char* pFilemapname, char* pString );
		DWORD	dbReadFilemapValue				( char* pFilemapname );
		char*	dbReadFilemapString				( char* pFilemapname );

		void	dbOpenToRead					( int f, char* pFilename );
		void	dbOpenToWrite					( int f, char* pFilename );
		void	dbCloseFile						( int f );
		int		dbReadByte						( int f );
		int		dbReadWord						( int f );
		int		dbReadFile						( int f );
		float	dbReadFloat						( int f );
		char*	dbReadString					( int f );
		void	dbReadFileBlock					( int f, char* pFilename );
		void	dbSkipBytes						( int f, int iSkipValue );
		void	dbReadDirBlock					( int f, char* pFilename );
		void	dbWriteByte						( int f, int iValue );
		void	dbWriteWord						( int f, int iValue );
		void	dbWriteLong						( int f, int iValue );
		void	dbWriteFloat					( int f, float fValue );
		void	dbWriteString					( int f, char* pString );
		void	dbWriteFileBlock				( int f, char* pFilename );
		void	dbWriteFileBlockEx				( int f, char* pFilename, int iFlag );
		void	dbWriteDirBlock					( int f, char* pFilename );

		void	dbReadMemblock					( int f, int mbi );
		void	dbMakeMemblockFromFile			( int mbi, int f );
		void	dbWriteMemblock					( int f, int mbi );
		void	dbMakeFileFromMemblock			( int f, int mbi );

		char*	dbGetDir						( void );
		char*	dbGetFileName					( void );
		int		dbGetFileType					( void );
		char*	dbGetFileDate					( void );
		char*	dbGetFileCreation				( void );
		int		dbFileExist						( char* pFilename );
		int		dbFileSize						( char* pFilename );
		int		dbPathExist						( char* pFilename );
		int		dbFileOpen						( int f );
		int		dbFileEnd						( int f );

		char*	dbAppname						( void );
		char*	dbWindir						( void );
		int		dbExecutableRunning				( DWORD hIndiExecuteFileProcess );

		// lee - 300706 - GDK fixes
		unsigned char dbReadByte				( int f, unsigned char* pByte );
		WORD	dbReadWord						( int f, WORD* pWord );
		int		dbReadFile						( int f, int* pInteger );
		float	dbReadFloat						( int f, float* pFloat );
		char*	dbReadString					( int f, char* pString );
		int		dbReadLong						( int f, int* pInteger );
		void	dbCD							( char* pPath );
		void	dbWriteFile						( int f, int iValue );

#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#endif _CFILE_H_
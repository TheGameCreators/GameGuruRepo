#define _CRT_SECURE_NO_WARNINGS
#define ZIP_PASSWORD "mypassword"
#define DARKSDK	
	
#include "stdafx.h"
#include "Zlib\cZip.h"
#include "core.h"
#include "Enchancements.h"
#include "CObjectsC.h"
#include "CImageC.h"
#include "CBitmapC.h"

#include "CFileC.h"

const long	MAX_VOLUME_SIZE_BYTES	= 100000000;	// maximum size of the zip
const int	ZIP_COMPRESS_LEVEL		= 9;			// maximum compression level

// file block structure
struct sFileBlock
{
	cZip*	pZip;
	bool			bEncrypted;
	char			szName     [ 255 ];
	char			szZipName  [ 255 ];
	char			szPassword [ 255 ];
	int				iCompression;
	int				iFileCount;
	
	sFileBlock ( )
	{
		pZip = NULL;

		bEncrypted = false;
		
		memset ( szName,     0, sizeof ( szName     ) );
		memset ( szZipName,  0, sizeof ( szZipName  ) );
		memset ( szPassword, 0, sizeof ( szPassword ) );
		
		strcpy ( szPassword, "default" );

		iCompression = 9;

		iFileCount = 0;
	}
};

sFileBlock	g_FileBlocks    [ 255 ];
char	    g_TempDirectory [ 255 ];
char        g_szRestoreDir  [ 255 ];

#define MAX_ZIP	255

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// FUNCTIONS ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////



/*
	// export names for string table -

		CREATE FILE BLOCK%LS%?CreateFileBlock@@YAXHK@Z%id, filename
		DELETE FILE BLOCK%L%?DeleteFileBlock@@YAXH@Z%ID

		SET FILE BLOCK KEY%LS%?SetFileBlockKey@@YAXHK@Z%ID, Key
		SET FILE BLOCK COMPRESSION%LL%?SetFileBlockCompression@@YAXHH@Z%ID, Flag

		ADD FILE TO BLOCK%LS%?AddFileToBlock@@YAXHK@Z%ID, File
		ADD OBJECT TO BLOCK%LLS%?AddObjectToBlock@@YAXHHK@Z%id, object, filename
		ADD IMAGE TO BLOCK%LLS%?AddImageToBlock@@YAXHHK@Z%id, object, filename
		ADD BITMAP TO BLOCK%LLS%?AddBitmapToBlock@@YAXHHK@Z%id, bitmap, filename

		REMOVE FILE FROM BLOCK%LS%?RemoveFileFromBlock@@YAXHK@Z%ID, File

		SAVE FILE BLOCK%L%?SaveFileBlock@@YAXH@Z%id
		OPEN FILE BLOCK%SL%?OpenFileBlock@@YAXKH@Z%File, ID
		OPEN FILE BLOCK%SLS%?OpenFileBlock@@YAXKHK@Z%File, ID, Key
		
		PERFORM CHECKLIST FOR FILE BLOCK DATA%L%?PerformCheckListForFileBlockData@@YAXH@Z%ID
		EXTRACT FILE FROM BLOCK%LSS%?ExtractFileFromBlock@@YAXHKK@Z%id, filename, path

		LOAD OBJECT FROM BLOCK%LSL%?LoadObjectFromBlock@@YAXHKH@Z%ID, File, Object
		LOAD BITMAP FROM BLOCK%LSL%?LoadBitmapFromBlock@@YAXHKH@Z%ID, File, Bitmap
		LOAD IMAGE FROM BLOCK%LSL%?LoadImageFromBlock@@YAXHKH@Z%ID, File, Image
		LOAD SOUND FROM BLOCK%LSL%?LoadSoundFromBlock@@YAXHKH@Z%ID, File, Sound
		LOAD FILE FROM BLOCK%LSL%?LoadFileFromBlock@@YAXHKH@Z%ID, File, File ID
		LOAD MEMBLOCK FROM BLOCK%LSL%?LoadMemblockFromBlock@@YAXHKH@Z%ID, File, Memblock
		LOAD ANIMATION FROM BLOCK%LSL%?LoadAnimationFromBlock@@YAXHKH@Z%ID, File, Animation
		
		GET FILE BLOCK EXISTS[%LL%?GetFileBlockExists@@YAHH@Z%ID
		GET FILE BLOCK SIZE[%LL%?GetFileBlockSize@@YAHH@Z%ID
		GET FILE BLOCK COUNT[%LL%?GetFileBlockCount@@YAHH@Z%ID
		GET FILE BLOCK DATA EXISTS[%LLS%?GetFileBlockDataExists@@YAHHK@Z%ID, File
		GET FILE BLOCK KEY[%LL%?GetFileBlockKey@@YAHH@Z%ID
		GET FILE BLOCK COMPRESSION[%LL%?GetFileBlockCompression@@YAHH@Z%ID
*/

void CheckID ( int iID )
{
	if ( iID < 0 || iID > 255 )
	{
		Error ( 7 );
		return;
	}
}

void CheckData ( int iID )
{
	// ensure the zip file is valid
	if ( !g_FileBlocks [ iID ].pZip )
	{
		// display a runtime error if something is wrong
		Error ( 11 );
		return;
	}
}

void SetupFileBlocks ( void )
{
	// set up the temp directory for file extraction
	GetWindowsDirectory ( g_TempDirectory, 255 );
	strcat ( g_TempDirectory, "\\temp\\" );
}

void DestroyFileBlocks ( void )
{
	// destroy file blocks
	for ( int i = 0; i < MAX_ZIP; i++ )
	{
		// check the file block
		if ( g_FileBlocks [ i ].pZip )
		{
			char szOriginalName [ 256 ] = "";

			for ( int y = 0; y < (int)strlen ( g_FileBlocks [ i ].szName ); y++ )
			{
				if ( g_FileBlocks [ i ].szName [ y ] == '.' )
				{
					memcpy ( szOriginalName, g_FileBlocks [ i ].szName, sizeof ( char ) * y );
					break;
				}
			}

			// destroy it
			delete g_FileBlocks [ i ].pZip;
			g_FileBlocks [ i ].pZip = NULL;

			strcat ( szOriginalName, ".000" );
			DeleteFile ( szOriginalName );
		}
	}
}

void SetTempDirectory ( void )
{
	// set the temporary directory
	GetCurrentDirectory ( 255, g_szRestoreDir );
	SetCurrentDirectory ( g_TempDirectory );
}

void RestoreDirectory ( void )
{
	// restore the original directory
	SetCurrentDirectory ( g_szRestoreDir );
}

void GetZipName ( int iID, char* szFileName )
{
	for ( int i = strlen ( szFileName ), c = 0; i > 0; i--, c++ )
	{
		if ( szFileName [ i ] == '.' )
		{
			memcpy ( g_FileBlocks [ iID ].szZipName, szFileName, sizeof ( char ) * i );
			strcat ( g_FileBlocks [ iID ].szZipName, ".zip" );
		}
	}
}

void CreateFileBlock ( int iID, char* szFilename )
{
	// create a new file block

	// check ID of file block
	CheckID ( iID );

	// safely delete any currently existing zip file
	SAFE_DELETE ( g_FileBlocks [ iID ].pZip );

	DeleteFile ( szFilename );

	g_FileBlocks [ iID ].pZip = NULL;

	g_FileBlocks [ iID ].bEncrypted = false;
	
	memset ( g_FileBlocks [ iID ].szName,     0, sizeof ( g_FileBlocks [ iID ].szName     ) );
	memset ( g_FileBlocks [ iID ].szZipName,  0, sizeof ( g_FileBlocks [ iID ].szZipName  ) );
	memset ( g_FileBlocks [ iID ].szPassword, 0, sizeof ( g_FileBlocks [ iID ].szPassword ) );
	
	strcpy ( g_FileBlocks [ iID ].szPassword, ZIP_PASSWORD );

	g_FileBlocks [ iID ].iCompression = 9;

	g_FileBlocks [ iID ].iFileCount = 0;

	// create a new zip archive
	g_FileBlocks [ iID ].pZip = new cZip;

	// check memory was allocated
	if ( !g_FileBlocks [ iID ].pZip )
	{
		// display error on failure
		Error ( 11 );
		return;
	}

	// attempt to open the zip file	
	// 280305 - do not create in span mode
	//g_FileBlocks [ iID ].pZip->Open ( ( char* ) dwFilename, CZipArchive::createSpan, MAX_VOLUME_SIZE_BYTES );
	//g_FileBlocks [ iID ].pZip->Open ( ( char* ) dwFilename, CZipArchive::create, MAX_VOLUME_SIZE_BYTES );
	g_FileBlocks [ iID ].pZip->Open ( szFilename , ZIP_PASSWORD );

	// store the name of the zip in the file block structure
	strcpy ( g_FileBlocks [ iID ].szName, szFilename );
}

void AddObjectToBlock ( int iID, int iObject, DWORD dwFilename )
{
	/*// check ID of file block
	CheckID ( iID );

	// add an object to a block
	AddOrObtainResourceFromBlock ( iID, iObject, dwFilename, 0, 0 );*/
}

void AddImageToBlock ( int iID, int iImage, DWORD dwFilename )
{
	// check ID of file block
	/*CheckID ( iID );

	// add an image to the block
	AddOrObtainResourceFromBlock ( iID, iImage, dwFilename, 1, 0 );*/
}

void AddBitmapToBlock ( int iID, int iBitmap, DWORD dwFilename )
{
	// check ID of file block
	/*CheckID ( iID );

	// add an image to the block
	AddOrObtainResourceFromBlock ( iID, iBitmap, dwFilename, 2, 0 );*/
}

void AddOrObtainResourceFromBlock ( int iID, int iIndex, DWORD dwFilename, int iType, int iMode )
{
}

void SetFileBlockKey ( int iID, char* dwKey )
{
	// Dave - don't need this anymore, password is set when opening the zip
	return;
}

void SetFileBlockCompression ( int iID, int iLevel )
{
	// set the file block compression

	// check ID of file block
	CheckID ( iID );

	// check the file block
	CheckData ( iID );

	// set the compression level
	g_FileBlocks [ iID ].iCompression = iLevel;
}

void AddFileToBlock ( int iID, char* szFile )
{
	// adds a file to a file block

	// check ID of file block
	CheckID ( iID );
	
	// check the file block is ok
	CheckData ( iID );

	char newFileName[1024];
	strcpy ( newFileName ,  szFile );
	char* p = newFileName;
	for ( int i = 0 ; i < (int)strlen (  newFileName ) ; i++ )
	{
		if ( p[i] == '\\' ) p[i] = '/';
	}

	// add a file to the block
	g_FileBlocks [ iID ].pZip->Add ( newFileName, newFileName );
}

void RemoveFileFromBlock ( int iID, char* dwFile )
{
	/*
	// 280305 - remove a file from a file block

	// check ID of file block
	CheckID ( iID );
	
	// check the file block is ok
	CheckData ( iID );

	int iIndex = g_FileBlocks [ iID ].pZip->FindFile ( ( char* ) dwFile );
	g_FileBlocks [ iID ].pZip->DeleteFile ( iIndex );
	*/
}

void SaveFileBlock ( int iID )
{
	// saves the file block

	// check ID of file block
	CheckID ( iID );

	// check the file block
	CheckData ( iID );

	// close the file block
	g_FileBlocks [ iID ].pZip->Close ( );

	GetZipName ( iID, g_FileBlocks [ iID ].szName );

	MoveFile ( g_FileBlocks [ iID ].szZipName, g_FileBlocks [ iID ].szName );
}

void CloseFileBlock	( int iID )
{
	// check ID of file block
	CheckID ( iID );

	// close the file block
	g_FileBlocks [ iID ].pZip->Close ( );

	// safely delete any currently existing zip file
	SAFE_DELETE ( g_FileBlocks [ iID ].pZip );
}

void Encryption ( int iID, int iMode )
{
/*
	// encrypt a file

	// check ID of file block
	CheckID ( iID );

	if ( g_FileBlocks [ iID ].bEncrypted && iMode == 0 )
		return;

	if ( iMode == 0 )
		g_FileBlocks [ iID ].bEncrypted = true;

	if ( iMode == 1 )
		g_FileBlocks [ iID ].bEncrypted = false;

	const size_t	bufferSize = 1024;
	size_t			readRet    = 0;
	int				encRet;

	char	szNewFile  [ 255 ];
	char	readBuffer [ bufferSize ];
	char	outBuffer  [ bufferSize ];
	char	_password  [ 100 ];

	strncpy ( _password, g_FileBlocks [ iID ].szPassword, 100 );
	
	BlowFishEnc encryption ( _password );

	strcpy ( szNewFile, g_FileBlocks [ iID ].szName );
	strcat ( szNewFile, ".fb" );

	FILE* readFile  = GG_fopen ( g_FileBlocks [ iID ].szName, "rb" );
	FILE* writeFile = GG_fopen ( szNewFile, "wb" );

	while ( !feof ( readFile ) )
	{
		readRet = fread ( readBuffer, sizeof ( char ), bufferSize, readFile );

		if ( iMode == 0 )
			encRet = encryption.encryptStream ( readBuffer, ( DWORD ) readRet, outBuffer );

		if ( iMode == 1 )
		{
			encRet = encryption.decryptStream(readBuffer, (DWORD)readRet, outBuffer);
			if ( feof(readFile) )
			{
				int pos = 0;
				while ((pos < 8) && ((outBuffer + encRet - pos)[0] == 0)) pos++;
				if (pos) encRet -= (pos - 1);
			}
		}

		fwrite ( outBuffer, sizeof ( char ), encRet, writeFile );
	}

	fflush ( writeFile );

	fclose ( writeFile );
	fclose ( readFile );

	ZeroMemory ( outBuffer,  bufferSize );
	ZeroMemory ( readBuffer, bufferSize );

	DeleteFile ( g_FileBlocks [ iID ].szName );
	CopyFile   ( szNewFile, g_FileBlocks [ iID ].szName, false );
	DeleteFile ( szNewFile );
	*/
}

void ExtractFileFromBlock ( int iID, char* szFilename, char* szPath )
{
	// extracts a file from the block
	
	// check ID of file block
	CheckID ( iID );

	// check the file block data
	CheckData ( iID );

	// get the index of the file
	/*int iIndex = g_FileBlocks [ iID ].pZip->FindFile ( ( char* ) dwFilename );

	// attempt to extract the file
	if ( !g_FileBlocks [ iID ].pZip->ExtractFile ( iIndex, ( char* ) dwPath ) )
	{
		// display error on failure - silent failure
		return;
	}*/

	char szExtractedFileName[1024];
	sprintf ( szExtractedFileName , "%s\\%s" , szPath , szFilename );

	for ( int i = 0 ; i < (int)strlen ( szExtractedFileName ) ; i++ )
	{
		if ( szExtractedFileName[i] == '\\' ) szExtractedFileName[i] = '/';
	}

	for ( int i = (int)strlen ( szExtractedFileName ) - 1 ; i > 0 ; i-- )
	{
		if ( szExtractedFileName[i] == '/' )
		{
			szExtractedFileName[i] = '\0';
			break;
		}
	}

	// make folders needed to extract the file
	char delims [ ] = "\\/";
	char* result = NULL;
	result = strtok ( szExtractedFileName , delims );	

	char pathBuilder[1024] = "";

	while ( result != NULL )
	{
		strcat ( pathBuilder , result );
		if ( strstr ( result , ":" ) == NULL )
		{
			CreateDirectory ( pathBuilder , NULL );
		}
		strcat ( pathBuilder , "\\" );

		char szTok [ 256 ] = "";
		strcpy ( szTok, result );
		result = strtok ( NULL, delims );
	}

	sprintf ( szExtractedFileName , "%s\\%s" , szPath , szFilename );

	char newFileName[1024];
	strcpy ( newFileName , szFilename );
	char* p = ( char* ) newFileName;
	for ( int i = 0 ; i < (int)strlen ( ( char* ) newFileName ) ; i++ )
	{
		if ( p[i] == '\\' ) p[i] = '/';
	}

	if ( !g_FileBlocks [ iID ].pZip->Extract ( ( char* ) newFileName , szExtractedFileName ) )
	{
		// display error on failure - silent failure
		return;
	}
}

void OpenFileBlock ( char* szFile, int iID )
{
	// open a file block

	// check ID of file block
	CheckID ( iID );

	// set the key to default
	//char* szKey = "default";
	char* szKey = ZIP_PASSWORD;

	// open the block
	OpenFileBlock ( szFile, iID, szKey );
}

void OpenFileBlock ( char* szFile, int iID, char* szKey )
{
	// open a file block from disk

	// check ID of file block
	CheckID ( iID );

	// safely delete any existing file block
	SAFE_DELETE ( g_FileBlocks [ iID ].pZip );

	// store name and password in file block structure
	strcpy ( g_FileBlocks [ iID ].szName,     szFile );
	strcpy ( g_FileBlocks [ iID ].szPassword, szKey  );

	// MoveFile ( g_FileBlocks [ iID ].szZipName, g_FileBlocks [ iID ].szName );
	
	// create a new zip archive
	g_FileBlocks [ iID ].pZip = new cZip;

	// check file is okay
	if ( !g_FileBlocks [ iID ].pZip )
	{
		// error on failure
		Error ( 11 );
		return;
	}

	/* 250215 - we rename ZIP files FPM to stay proprietory
	// lee - 290306 - u6rc3 - reject if not ZIP ext, as regular files crash pZip interface
	LPSTR lpFilename = ( char* ) dwFile;
	if ( strnicmp ( lpFilename + strlen(lpFilename) - 4, ".zip", 4 )!=NULL )
	{
		Error ( 132 );
		return;
	}
	*/

	// open the zip and set the password
	if ( !g_FileBlocks [ iID ].pZip->Open ( szFile, ZIP_PASSWORD ) )
	{
//		MessageBox ( NULL, "failed to open zip", "error", MB_OK );
		Error ( 133 );
		return;
	}

	//g_FileBlocks [ iID ].pZip->SetPassword ( ( char* ) dwKey );

	//g_FileBlocks [ iID ].iFileCount = g_FileBlocks [ iID ].pZip->GetNoEntries ( );
	g_FileBlocks [ iID ].iFileCount = g_FileBlocks [ iID ].pZip->GetFileCount();
}

void OpenFileBlockNoPw(char* szFile, int iID, char* szKey)
{
	// open a file block from disk

	// check ID of file block
	CheckID(iID);

	// safely delete any existing file block
	SAFE_DELETE(g_FileBlocks[iID].pZip);

	// store name and password in file block structure
	strcpy(g_FileBlocks[iID].szName, szFile);
	strcpy(g_FileBlocks[iID].szPassword, szKey);

	// MoveFile ( g_FileBlocks [ iID ].szZipName, g_FileBlocks [ iID ].szName );

	// create a new zip archive
	g_FileBlocks[iID].pZip = new cZip;

	// check file is okay
	if (!g_FileBlocks[iID].pZip)
	{
		// error on failure
		Error(11);
		return;
	}

	/* 250215 - we rename ZIP files FPM to stay proprietory
	// lee - 290306 - u6rc3 - reject if not ZIP ext, as regular files crash pZip interface
	LPSTR lpFilename = ( char* ) dwFile;
	if ( strnicmp ( lpFilename + strlen(lpFilename) - 4, ".zip", 4 )!=NULL )
	{
		Error ( 132 );
		return;
	}
	*/

	// open the zip and set the password
	if (!g_FileBlocks[iID].pZip->Open(szFile, szKey))
	{
		//		MessageBox ( NULL, "failed to open zip", "error", MB_OK );
		Error(133);
		return;
	}

	//g_FileBlocks [ iID ].pZip->SetPassword ( ( char* ) dwKey );

	//g_FileBlocks [ iID ].iFileCount = g_FileBlocks [ iID ].pZip->GetNoEntries ( );
	g_FileBlocks[iID].iFileCount = g_FileBlocks[iID].pZip->GetFileCount();
}

void DeleteFileBlock ( int iID )
{
	// delete the file block

	// check ID of file block
	CheckID ( iID );

	// check the data
	CheckData ( iID );

	// clear the file block
	SAFE_DELETE ( g_FileBlocks [ iID ].pZip );
}

void PerformCheckListForFileBlockData ( int iID )
{
	// fill the checklist with a list of files contained within
	// the file block

	// check ID of file block
	CheckID ( iID );

	// ensure the file block is valid
	CheckData ( iID );

	if ( !g_pGlob )
	{
		Error ( 1 );	
		return;
	}

	// set checklist properties
	g_pGlob->checklistexists     = true;											// set exist flag on
	g_pGlob->checklisthasstrings = true;											// set strings on
	g_pGlob->checklisthasvalues  = true;											// set strings on
	g_pGlob->checklistqty        = g_FileBlocks [ iID ].pZip->GetFileCount();	// set the number of entries in the checklist

	// go through each item in the checklist
	for ( int i = 0; i < g_pGlob->checklistqty; i++ )
	{
		// expand the checklist so we can add a string
		GlobExpandChecklist ( i, 255 );

		// local variables
		char file[1024];	// string name of the file

		// get the file name from the file information structure
		strcpy ( file , g_FileBlocks [ iID ].pZip->GetFilename(i) );

		// copy the file name into the checklist
		strcpy ( g_pGlob->checklist [ i ].string, file );
	}
}

void LoadObjectFromBlock ( int iID, DWORD dwFile, int iObject )
{
	// check ID of file block
	CheckID ( iID );

	// load an object from a block
	AddOrObtainResourceFromBlock ( iID, iObject, dwFile, 0, 1 );
}

void LoadBitmapFromBlock ( int iID, DWORD dwFile, int iBitmap )
{
	// check ID of file block
	CheckID ( iID );

	// load an image from the block
	AddOrObtainResourceFromBlock ( iID, iBitmap, dwFile, 2, 1 );
}

void LoadImageFromBlock	( int iID, DWORD dwFile, int iImage )
{
	// check ID of file block
	CheckID ( iID );

	// load an image from the block
	AddOrObtainResourceFromBlock ( iID, iImage, dwFile, 1, 1 );
}

void LoadSoundFromBlock	( int iID, DWORD dwFile, int iSound )
{

}

void LoadFileFromBlock ( int iID, DWORD dwFile, int iFile )
{

}

void LoadMemblockFromBlock ( int iID, DWORD dwFile, int iMemblock )
{

}

void LoadAnimationFromBlock	( int iID, DWORD dwFile, int iAnimation )
{

}

int GetFileBlockExists ( int iID )
{
	// return true if the file block exists

	// check ID of file block
	CheckID ( iID );

	if ( g_FileBlocks [ iID ].pZip )
		return 1;

	return 0;
}

int GetFileBlockSize ( int iID )
{
	// get the size of the file block

	// check ID of file block
	CheckID ( iID );

	// check the data
	CheckData ( iID );

	// save the file block
	SaveFileBlock ( iID );

	// get size
	DWORD dwSize;

	HANDLE hfile = GG_CreateFile ( g_FileBlocks [ iID ].szName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	if ( hfile == INVALID_HANDLE_VALUE )
		return -1;
	
	dwSize = GetFileSize ( hfile, NULL );
	CloseHandle ( hfile );

	// return the size
	return ( int ) dwSize;
}

int GetFileBlockCount ( int iID )
{
	/*// check ID of file block
	CheckID ( iID );

	// ensure the file block is valid
	CheckData ( iID );

	return g_FileBlocks [ iID ].pZip->GetNoEntries ( );*/
	return 0;
}

int GetFileBlockDataExists ( int iID, DWORD dwFile )
{
/*	// check ID of file block
	CheckID   ( iID );
	CheckData ( iID );
	int iIndex = g_FileBlocks [ iID ].pZip->FindFile ( ( char* ) dwFile );

	// lee - 240306 - u6b4 - true if not negative
	if ( iIndex>=0 )
		return 1;

	// not found
	return 0;*/
	return 0;
}

int GetFileBlockKey	( int iID )
{
	// check ID of file block
	CheckID   ( iID );
	CheckData ( iID );

	return 1;
}

int GetFileBlockCompression ( int iID )
{
	// check ID of file block
	CheckID ( iID );

	CheckData ( iID );

	return g_FileBlocks [ iID ].iCompression;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

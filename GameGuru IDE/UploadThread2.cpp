#include "stdafx.h"

#include "UploadThread2.h"
#include "Editor.h"
#include "UploadGame.h"
#include <atlimage.h>

UploadThread2::UploadThread2( )
{
	m_pProgressBar = NULL;
	m_pParent = NULL;

	m_sModelFilename = "";
	m_sThumbnail = "";
	m_sDescription = "";

	m_bUploading = false;
	m_bUploadSuccessful = false;
	m_bIsPacking = false;
	m_bCancelled = false;
	iClubID = 0;
}

UploadThread2::~UploadThread2( )
{
	m_Server.CancelTransfer( );
	Join();
}

void UploadThread2::SetFields( CUploadGame *parent, CProgressCtrl *progress, CString model, CString thumbnail, CString description, int iClub )
{
	m_pParent = parent;
	m_pProgressBar = progress;
	m_sModelFilename = model;
	m_sThumbnail = thumbnail;
	m_sDescription = description;
	iClubID = iClub;
}

void UploadThread2::Cancel()
{
	if ( m_bIsPacking ) 
	{
		MessageBox( NULL, "Cannot cancel during the packing process, please wait", "Warning", 0 );
		return;
	}
	m_bCancelled = true;
	m_Server.CancelTransfer( );
	m_pParent = NULL;
	m_Server.SetProgress( NULL, NULL ); 
}

unsigned UploadThread2::Run( )
{
	m_bCancelled = false;
	m_bUploading = true;
	m_bUploadSuccessful = false;
	sError = "";

	// FPGC - 010610 - get game name only
	char pGameName[512];
	strcpy ( pGameName, "packedgame" );
	char* pGameNameOnly = NULL;
	pGameNameOnly = (char*)strrchr( m_sModelFilename, '\\' );
	if ( pGameNameOnly )
	{
		pGameNameOnly++;
		strcpy ( pGameName, pGameNameOnly );
	}

	// FPGC - 010610 - determine if folder specified is actual game folder, or some other folder
	char pTestExistenceOfGameEXE[512];
	strcpy ( pTestExistenceOfGameEXE, m_sModelFilename );
	strcat ( pTestExistenceOfGameEXE, "\\" );
	strcat ( pTestExistenceOfGameEXE, pGameName );
	strcat ( pTestExistenceOfGameEXE, ".exe" );
	HANDLE hFileExist = CreateFile ( pTestExistenceOfGameEXE, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFileExist==INVALID_HANDLE_VALUE )
	{
		sError = "Not a game folder. Select folder containing the game executable.";
		m_bUploading = false;
		return 0; 
	}
	CloseHandle ( hFileExist );

	m_bIsPacking = true;
	if ( m_pParent ) m_pParent->SetMessage( "Packing Game Files..." );

	//pack game files
	char szParams [ MAX_PATH ];
	sprintf_s( szParams, MAX_PATH, "Packgame.exe \"%s\"", m_sModelFilename );

	STARTUPINFO startupinfo;
	GetStartupInfo( &startupinfo );
	PROCESS_INFORMATION processinfo;
	if ( !CreateProcess( NULL, szParams, NULL, NULL, FALSE, 0, NULL, NULL, &startupinfo, &processinfo ) )
	{
		char str[256]; char str2[256]; 
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, str, 256, NULL );
		sprintf_s( str2, 256, "Error packing game folder: %s", str );
		sError = str2;
		m_bUploading = false;
		m_bIsPacking = false;
		return 0;
	}
	
	DWORD dwResult = WaitForSingleObject( processinfo.hProcess, 180000 );
	switch( dwResult )
	{
		case WAIT_TIMEOUT: 
			{
				sError = "The pack game process is not responding after 3 minutes, please try again.";
				m_pParent->SetMessage( sError );
				m_bUploading = false;
				m_bIsPacking = false;
				TerminateProcess ( processinfo.hProcess, 0 );
				return 0; 
				break;
			}
		case WAIT_FAILED: 
			{
				sError = "Error waiting for the pack game process, please try again.";
				m_pParent->SetMessage( sError );
				m_bUploading = false;
				m_bIsPacking = false;
				TerminateProcess ( processinfo.hProcess, 0 );
				return 0; 
				break;
			}
	}

	m_bIsPacking = false;

	m_Server.SetProgress( NULL, NULL );
	if ( m_pParent ) m_pParent->SetMessage( "Logging In..." );

	// FPGC - 010610 - use game folder name, not packedgame text
	//m_sModelFilename += "\\packedgame.zip";
	m_sModelFilename += "\\";
	m_sModelFilename += pGameName;
	m_sModelFilename += ".zip";

	//check upload is allowed first
	const char* szResult = m_Server.UploadToClubCheck( theApp.m_szUploadKey, m_sDescription, m_sModelFilename, iClubID );
	if ( !szResult )
	{
		char str [ 256 ];
		sprintf_s( str, 256, "Failed to upload game: %s", m_Server.GetLastError( ) );
		sError = str;
		//MessageBox( NULL, str, "Error", 0 );
		m_bUploading = false;
		return 0;
	} 
	else if ( strncmp( szResult, "<success>", strlen("<success>") ) != 0 ) 
	{
		sError = szResult;
		//MessageBox( NULL, szResult, "Upload", 0 );
		m_bUploading = false;
		return 0;
	}

	delete [] szResult;

	//send files
	m_Server.SetProgress( m_pProgressBar, NULL );
	if ( m_pParent ) m_pParent->SetMessage( "Uploading..." );

	szResult = m_Server.UploadToClub( theApp.m_szUploadKey, m_sDescription, m_sModelFilename, m_sThumbnail, iClubID );
	if ( !szResult )
	{
		char str [ 256 ];
		sprintf_s( str, 256, "Failed to upload game: %s", m_Server.GetLastError( ) );
		//MessageBox( NULL, str, "Error", 0 );

		// sError invalid in XP and crashes OS - perhaps clicking CANCEL kills the thread?
		//sError = str;

		m_bUploading = false;
		return 0;
	}
	else if ( strncmp( szResult, "<success>", strlen("<success>") ) == 0 ) 
	{
		//MessageBox( NULL, "Upload Successful.", "Upload", 0 );
		// sError invalid in XP and crashes OS
		// sError = "";
		m_bUploadSuccessful = true;
	}
	else 
	{
		//MessageBox( NULL, szResult, "Upload", 0 );
		// sError invalid in XP and crashes OS
		// sError = szResult;
	}

	delete [] szResult;
	
	m_bUploading = false;
	return 0;
}
#include "stdafx.h"
#include "UpdateCheckThread.h"
#include "AutoUpdate.h"
#include <afxinet.h>
#include "Editor.h"
//#include "HTTPComm.h"

// Global variables
extern CString sFPSCUpdateRating;
extern CString sFPSCUpdateFilename;
extern CString sFPSCUpdateLocalPath;
extern bool g_bX9InterfaceMode;
extern bool g_bSantitizedTeenKidFriendly;

UpdateCheckThread::UpdateCheckThread( )
{
	m_iMode = -1;
	m_pStatusBox = NULL;
	m_pProgress = NULL;
	m_pOwner = NULL;
	m_bCancel = false;
}

UpdateCheckThread::~UpdateCheckThread( )
{
	
}

void UpdateCheckThread::SetDownloadFile( const TCHAR* szFilename, CProgressCtrl *pProgress, CEdit *pStatusBox, CAutoUpdate *pOwner )
{
	m_iMode = 1;
	_tcscpy_s( m_szFilename, 256, szFilename );
	m_pProgress = pProgress;
	m_pStatusBox = pStatusBox;
	m_pOwner = pOwner;
	m_bCancel = false;
}

void UpdateCheckThread::SetStartUpCheck( )
{
	m_iMode = 0;
	m_bCheckCompleted = false;
}

int UpdateCheckThread::GetMode( )
{
	return m_iMode;
}

void UpdateCheckThread::CancelDownload( )
{
	m_bCancel = true;
}

unsigned int UpdateCheckThread::Run( )
{
	/*
	if ( m_iMode == 0 )
	{
		// FPGC - 280809 - we are running in X9 mode
		// AND MUST POINT TO CORRECT SERVER RIGHT AT THE BEGINNING!
		if ( g_bX9InterfaceMode==true )
		{
			// set new files and paths
			sFPSCUpdateRating = _T("");
			sFPSCUpdateFilename = _T("GameGuruUpdate.exe");
			sFPSCUpdateLocalPath = _T("\\Documents\\FPGC Files\\");

			// ensure FPGC folder exists (so auto update can work), if not there, make them
			char lpOldDir [ MAX_PATH ];
			GetCurrentDirectory ( MAX_PATH, lpOldDir );
			CString sDocPath = theApp.m_szUserPath;
			sDocPath += sFPSCUpdateLocalPath;
			if ( !SetCurrentDirectory( sDocPath.GetString() ) )
			{
				if ( SetCurrentDirectory( theApp.m_szUserPath ) )
				{
					if ( !SetCurrentDirectory( _T("Documents") ) )
					{
						CreateDirectory( _T("Documents"), NULL );
						SetCurrentDirectory( _T("Documents") );
					}
					if ( !SetCurrentDirectory( _T("FPGC Files") ) )
						CreateDirectory( _T("FPGC Files"), NULL );
				}
			}
			SetCurrentDirectory( lpOldDir );
		}

		// check for latest version value direct from server
		_tcscpy_s( m_szLatestVersion, 64, _T("n/a") );
		FILE *pFile = NULL;
		try 
		{
			// X10 V107 - 010308 - used to write 'latest version.ini' in program files, should be in writable area!
			// if the UPDATES folder does not exist (for time update) it will create it here
			CString sFullPathToWriteTemp = theApp.m_szUserPath;
			sFullPathToWriteTemp += sFPSCUpdateLocalPath;//_T("\\Documents\\FPSC X10 Files\\Updates");
			if ( !SetCurrentDirectory( sFullPathToWriteTemp.GetString() ) )  CreateDirectory( sFullPathToWriteTemp.GetString(), NULL );
			sFullPathToWriteTemp += _T( "\\latest version.ini" );

			CInternetSession cInet;
			CFtpConnection *pFTP = cInet.GetFtpConnection( _T("ftp.fpscreator.com"), _T("anonymous"), NULL, 21 );
			if ( !pFTP ) throw 1;

			// FPGC - 280809 - has own FTP area for FPSC/FPGC updates (differs from X10)
			if ( g_bX9InterfaceMode==true )
			{
				BOOL result = pFTP->SetCurrentDirectory ( _T("FPSCX9") );
				if ( !result ) throw 2;
			}

			BOOL result = pFTP->GetFile( _T("Update.txt"), sFullPathToWriteTemp, FALSE );
			if ( !result ) throw 2;

			int error = _tfopen_s( &pFile, sFullPathToWriteTemp, _T("rb") );
			if ( !error )
			{
				_fgetts( m_szLatestVersion, 64, pFile );
			}

			if ( pFile ) 
			{
				fclose( pFile );
				_tremove( sFullPathToWriteTemp );
				pFile = NULL;
			}
		}
		catch(...)
		{
			if ( pFile ) fclose( pFile );
			pFile = NULL;
		}

		m_bCheckCompleted = true;
	}
	else if ( m_iMode == 1 )
	{
		FILE *pLocalFile = NULL;
		HINTERNET hInet, hInetConnect, hFtpFile;

		try
		{
			hInet = InternetOpen( _T("InternetConnection"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
			if ( !hInet )
			{
				m_pStatusBox->SetWindowText( GetLanguageData( _T("Auto Update"), _T("FailedInit") ) );
				throw 1;
			}

			hInetConnect = InternetConnect( hInet, _T("ftp.fpscreator.com"), INTERNET_DEFAULT_FTP_PORT, _T("anonymous"), NULL, INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0 );
			if ( !hInetConnect )
			{
				m_pStatusBox->SetWindowText( GetLanguageData( _T("Auto Update"), _T("FailedConnect") ) );
				throw 2;
			}

			// FPGC - 280809 - has own FTP area for FPSC/FPGC updates (differs from X10)
			if ( g_bX9InterfaceMode==true )
			{
				BOOL hFtpCWD = FtpSetCurrentDirectory ( hInetConnect, _T("FPSCX9") );
				if ( !hFtpCWD ) throw 2;
			}

			hFtpFile = FtpOpenFile( hInetConnect, sFPSCUpdateFilename, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 0 );
			if ( !hFtpFile )
			{
				TCHAR str [ 256 ];
				_stprintf_s( str, 256, _T("%s: %d\r\n\r\n"), GetLanguageData( _T("Auto Update"), _T("FailedGet") ), GetLastError( ) );

				TCHAR error [ 256 ];
				DWORD dwError, dwLen;
				InternetGetLastResponseInfo( &dwError, error, &dwLen );

				_tcscat_s( str, 256, error );
				m_pStatusBox->SetWindowText( str );
				throw 3;
			}

			DWORD dwSizeHigh = 0;
			DWORD dwSizeLow = FtpGetFileSize( hFtpFile, &dwSizeHigh );
			float fSize = (float) dwSizeLow;

			CString sFullPath = theApp.m_szUserPath;
			sFullPath += sFPSCUpdateLocalPath;//_T("\\Documents\\FPSC X10 Files\\Updates");
			if ( !SetCurrentDirectory( sFullPath.GetString() ) ) 
				CreateDirectory( sFullPath.GetString(), NULL );

			sFullPath += _T( "\\" );
			sFullPath += sFPSCUpdateFilename;

			_tfopen_s( &pLocalFile, sFullPath.GetString(), _T("wb") );
			if ( !pLocalFile )
			{
				m_pStatusBox->SetWindowText( GetLanguageData( _T("Auto Update"), _T("LocalFile") ) );
				throw 3;
			}

			bool bFinished = false;

			TCHAR pBuffer [ 20480 ];
			DWORD dwReceived = 0;
			DWORD dwTotalReceived = 0;
			TCHAR *szBytes = _T("");
			TCHAR *szKiloBytes = _T("KB");
			TCHAR *szMegaBytes = _T("MB");
			TCHAR *szUnits = szBytes;
			int iUnitMode = 0;

			if ( fSize > 2097152 )
			{
				szUnits = szMegaBytes;
				iUnitMode = 2;
				fSize /= 1048576;
			}
			else if ( fSize > 10240 )
			{
				szUnits = szKiloBytes;
				iUnitMode = 1;
				fSize /= 1024;
			}
			

			while ( !bFinished && !m_bCancel )
			{
				BOOL bResult = InternetReadFile( hFtpFile, pBuffer, 20480, &dwReceived );
				if ( !bResult )
				{
					TCHAR str [ 256 ];
					_stprintf_s( str, 256, _T("%s: %d\r\n\r\n"), GetLanguageData( _T("Auto Update"), _T("FailedDownload") ), GetLastError( ) );

					TCHAR error [ 256 ];
					DWORD dwError, dwLen;
					InternetGetLastResponseInfo( &dwError, error, &dwLen );

					_tcscat_s( str, 256, error );
					m_pStatusBox->SetWindowText( str );
					throw 4;
				}

				if ( dwReceived == 0 ) bFinished = true;
				dwTotalReceived += dwReceived;

				if ( dwReceived > 0 )
				{
					fwrite( pBuffer, 1, dwReceived, pLocalFile );
					m_pProgress->SetPos( (int) ((dwTotalReceived*100.0f) / dwSizeLow) );
					TCHAR str [ 256 ];
					float fTotalReceived = (float)dwTotalReceived;
					if ( iUnitMode == 1 ) fTotalReceived /= 1024;
					else if ( iUnitMode == 2 ) fTotalReceived /= 1048576;
					_stprintf_s( str, 256, _T("%s: %.1f/%.1f%s"), GetLanguageData( _T("Auto Update"), _T("Downloading") ), fTotalReceived, fSize, szUnits );
					m_pStatusBox->SetWindowText( str );
				}
			}

			if ( m_bCancel ) throw 5;

			if ( pLocalFile ) fclose( pLocalFile );
			pLocalFile = NULL;

			InternetCloseHandle( hFtpFile );
			InternetCloseHandle( hInetConnect );
			InternetCloseHandle( hInet );

			m_pStatusBox->SetWindowText( GetLanguageData( _T("Auto Update"), _T("Complete") ) );

			m_iMode = -1;
			m_pOwner->DownloadComplete( );
		}
		catch(...)
		{
			if ( pLocalFile ) fclose( pLocalFile );

			if ( hFtpFile ) InternetCloseHandle( hFtpFile );
			if ( hInetConnect ) InternetCloseHandle( hInetConnect );
			if ( hInet ) InternetCloseHandle( hInet );
		}
	}
	*/
	return 0;
}
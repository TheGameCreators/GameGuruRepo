// DownloadStoreItems.cpp : implementation file
//

#include <afx.h>
#include "stdafx.h"
#include "DownloadStoreItems.h"
#include "..\Editor.h"
#include <atlimage.h>
#include "..\LoginDialog2.h"
#include ".\..\zip\ZipArchive.h"
#include "urlDecode.h"
#include "DownloadStoreItemsFile.h"
#include "..\SerialCode\md5.h"
#include "cleanupFolders.h"

#define DOWNLOADSTOREITEMS_TIMEOUT 30000

char userSteamID[256];
int haveSteamUserID = 0;

// made global as it doesnt debug inside timer otherwise
char copyFrom[MAX_PATH];

// how many retries
int downloadTriesBeforeGiveUp = 50;
int bytesDownloaded = 0;
// rem next line is just useful in testing resume
//int downloadFailTest = 0;
const long	MAX_VOLUME_SIZE_BYTES	= 100000000;	// maximum size of the zip
void DownloadProcessExtractOrCleanZip ( LPSTR pFilename , bool bCleanItUp )
{
	//bCleanItUp = true; // test cleanup!

	// record current dir
	char pOldDir[MAX_PATH];
	GetCurrentDirectory ( MAX_PATH, pOldDir );

	// destination based on root dir
	char pDestinationPath [ MAX_PATH ];
	strcpy ( pDestinationPath, theApp.m_pRootDir );
	strcat ( pDestinationPath, "\\Files\\editors\\TEMP\\" );

	// jump to destination path (where zip is)
	SetCurrentDirectory ( pDestinationPath );

	// open zip file
	CZipArchive* pZip = new CZipArchive;
	if ( pZip )
	{
		//pZip->SetPassword ( "SomeKey" ); // we are not password protecting these zips
		if ( pZip->Open ( pFilename, CZipArchive::open, MAX_VOLUME_SIZE_BYTES )==true )
		{
			// extract all files
			WORD wFileCount = pZip->GetNoEntries ( );
			for ( WORD wIndex=0; wIndex<wFileCount; wIndex++ )
			{
				if ( bCleanItUp==true )
				{
					// delete this file from TEMP 
					CZipFileHeader Info;
					pZip->GetFileInfo(Info,wIndex);
					CString szFileToDelete = Info.GetFileName();
					LPSTR pFileToDelete = szFileToDelete.GetBuffer(szFileToDelete.GetLength());
					DeleteFile(pFileToDelete);
				}
				else
				{
					// attempt to extract the file
					if ( pZip->ExtractFile ( wIndex, pDestinationPath )==false )
					{
						// failed to extract file
					}
				}
			}

			// finally close zip file
			pZip->Close ( );
		}

		// free resource
		if ( pZip ) { delete pZip; pZip = NULL; }

		// and finally delete zip
		if ( bCleanItUp==true ) DeleteFile(pFilename);
	}

	// restore current directory
	SetCurrentDirectory ( pOldDir );
}

char *replaceString(char *st, char *orig, char *repl) 
{
	static char buffer[MAX_PATH];
	char *ch;
	if (!(ch = strstr(st, orig)))
	return st;
	strncpy(buffer, st, ch-st);  
	buffer[ch-st] = 0;
	sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
	return buffer;
 }

// CDownloadStoreItems dialog

IMPLEMENT_DYNAMIC(CDownloadStoreItems, CDialog)

// When app runs
CDownloadStoreItems::CDownloadStoreItems(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadStoreItems::IDD, pParent)
{
	m_bIsLoggedIn = false;
	strcpy ( m_szUserName , "" );
	strcpy ( m_szPassword , "" );
	strcpy ( m_szUserID , "" );
	m_bFirstTimeLogin = true;
	m_StoreItems.clear();
	m_busyCheckingList = false;
	m_TimerMode = 0;
	m_pTimer = NULL;
	m_hInternet = NULL;
	m_hConnect = NULL;
	m_hRequest = NULL;
	m_callDownloadFiles = false;
}

//Destructor
CDownloadStoreItems::~CDownloadStoreItems()
{
}

// 1. When the download items is selected from menu
void CDownloadStoreItems::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT3, m_editStatus);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_PROGRESS_SPINNER, m_ProgressSpinner);
	DDX_Control(pDX, IDC_STATICDESC, m_Lines[0]);
	DDX_Control(pDX, IDC_STATICDOWN, m_Lines[1]);
	DDX_Control(pDX, IDC_BUTTON3, m_Lines[2]);
	DDX_Control(pDX, IDOK, m_Lines[3]);
	DDX_Control(pDX, IDCANCEL, m_Lines[4]);

	SetWindowText(GetLanguageData("Download Store", "Title"));
	m_Lines[0].SetWindowText(GetLanguageData("Download Store", "Desc"));
	m_Lines[1].SetWindowText(GetLanguageData("Download Store", "Downloading"));
	m_Lines[2].SetWindowText(GetLanguageData("Download Store", "Change"));
	m_Lines[3].SetWindowText(GetLanguageData("Download Store", "Download"));
	m_Lines[4].SetWindowText(GetLanguageData("Download Store", "Close"));
	
}

BEGIN_MESSAGE_MAP(CDownloadStoreItems, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CDownloadStoreItems::OnUploadClicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CDownloadStoreItems::OnChangeLoginClicked)
	ON_BN_CLICKED(IDCANCEL, &CDownloadStoreItems::OnBnClickedCancel)
END_MESSAGE_MAP()

// 2. show the dialog after being selected from menu
BOOL CDownloadStoreItems::OnInitDialog( )
{
	if ( !CDialog::OnInitDialog( ) ) return FALSE;

	this->CenterWindow();
	this->SetRedraw();
	this->RedrawWindow();
	this->SetWindowPos ( &CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE   );
	this->SetRedraw();
	this->RedrawWindow();

	//m_editDescription.SetWindowText( "A game made in First Person Game Creator" );
	//SetTimer( 3, 1000, NULL );

	// if user is logged in, grab the list of files they have, if not pop up the login box
	if ( !UserLogin() )
	{
		m_busyCheckingList = false;
		m_editStatus.SetWindowTextA( "Not Logged in to the Game Guru Store." );
		//OnChangeLoginClicked();
	}
	else
	{
		m_busyCheckingList = true;
		m_callDownloadFiles = false;
		UpdateList();
	}

	m_ProgressBar.SetPos(0);
	m_ProgressSpinner.StopSpinning();

	return TRUE;
}

bool CDownloadStoreItems::UpdateList()
{
	 // first delete item list
	m_StoreItems.clear();

	m_Lines[3].EnableWindow ( 0 );

	// reset downloaded
	bytesDownloaded = 0;

	//m_DownloadButton.SetWindowTextA ( "HELLO JONES" );

	char folder [ MAX_PATH ];
	sprintf ( folder , "%s\\Files\\editors\\TEMP" , theApp.m_szDirectory );
	CreateDirectoryA ( folder , NULL );

	m_NumberOfFilesToDownload = 0;
	
	if ( !m_bIsLoggedIn )
	{
		m_busyCheckingList = false;
		//::MessageBox ( NULL, "You must login first!", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
		return false;
	}

	char listURL[256];         
	sprintf ( listURL , "userPurchasedV2.php?sc=[downloadkeygoesere]&uid=%s" , m_szUserID );

	 m_hInternet = InternetOpen( TEXT("FPSCDownload"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL,0 );
		if ( m_hInternet != NULL )
		{

			DWORD timeout = DOWNLOADSTOREITEMS_TIMEOUT;
			InternetSetOption ( m_hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( m_hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( m_hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( m_hInternet, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( m_hInternet, INTERNET_OPTION_DATA_SEND_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( m_hInternet, INTERNET_OPTION_FROM_CACHE_TIMEOUT, &timeout, sizeof ( timeout ) );

			m_hConnect = InternetConnect( m_hInternet, TEXT("api.tgcstore.net"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL );

			if ( m_hConnect != NULL )
			{
				const char* lplpszAcceptTypes[] = {"text/xml", "application/xml", "application/xhtml+xml", NULL};

				m_hRequest = HttpOpenRequest( m_hConnect , "GET", listURL , NULL, NULL, lplpszAcceptTypes,
				INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID|
				INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_NO_CACHE_WRITE|
				INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
				INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_KEEP_CONNECTION |
				INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE, 0);

				//HINTERNET hRequest = HttpOpenRequest( hConnect, TEXT("GET"), TEXT(loginURL), NULL, NULL, 0, 0, 1 );
				if ( m_hRequest != NULL )
				{
					BOOL bSend = HttpSendRequest(m_hRequest, NULL,0, NULL,0);
					if ( bSend )
					{
						m_TimerMode = 1;
						m_pTimer = SetTimer ( 63, 1, NULL );
						return true;
					}
					else
					{
						::MessageBox ( NULL, "Could not connect to the server (send request failed)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
						m_busyCheckingList = false;
						return false;
					}
					InternetCloseHandle(m_hRequest);
				}
				else
				{
					::MessageBox ( NULL, "Could not connect to the server (open request failed - 256)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
					m_busyCheckingList = false;
					return false;
				}
				InternetCloseHandle(m_hConnect);
			}
			else
			{
				::MessageBox ( NULL, "Could not connect to the server (connect failed (error 264))", "Connection Error", MB_OK | MB_SYSTEMMODAL );
				m_busyCheckingList = false;
				return false;
			}
			InternetCloseHandle(m_hInternet);
		}
		else
		{
			::MessageBox ( NULL, "Could not connect to the internet", "Connection Error", MB_OK | MB_SYSTEMMODAL );
			m_busyCheckingList = false;
			return false;
		}

		//if ( m_NumberOfFilesToDownload == 0 )
			//m_editStatus.SetWindowTextA( "To access the online store, close this dialogue and click on the bottom left icon. You will need to register a store account before you can download extra game assets." );

		return true;
}

// CDownloadStoreItems message handlers
// This is actually download not upload - nervous to change it as it may screw up the mfc stuff for the button, so upload it is for now
void CDownloadStoreItems::OnUploadClicked()
{
	if ( !m_bIsLoggedIn )
	{
		::MessageBox ( NULL, "User not registered on the Game Guru Store, please click the Game Guru Store button in the bottom left of the editor and login using your Steam account.", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
		m_editStatus.SetWindowTextA( "Not Logged in to the Game Guru Store." );
		return;
	}
	if ( m_busyCheckingList || m_TimerMode > 0 ) return;

	m_busyCheckingList = true;
	m_callDownloadFiles = true;
	UpdateList();

}

sStoreDownloadItem storeItem;

//FILE* wf= NULL;

void CDownloadStoreItems::OnTimer( UINT_PTR nIDEvent )
{
	if ( nIDEvent == 63 )
	{
		if ( m_TimerMode == 0 )
		{
			m_Lines[3].EnableWindow ( 1 );
			KillTimer ( m_pTimer );	
			return;
		}

		if ( m_TimerMode == 1 )
		{
			m_ProgressSpinner.StartSpinning();
			m_ProgressBar.SetPos(0);
			m_ProgressSpinner.UpdateWindow();

			TCHAR szString [ 3 ] [ MAX_PATH ];
			GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Grabbing" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
			m_editStatus.SetWindowTextA( szString[0] );

			m_TimerMode = 2;
			return;
		}

		if ( m_TimerMode == 2 )
		{
			sprintf ( m_fileName , "%s\\Files\\editors\\TEMP\\temp.txt" , theApp.m_szDirectory );
			m_file = fopen ( m_fileName , "wb" );
		
			if ( m_file == NULL )
			{
				m_TimerMode = 0;
				KillTimer ( m_pTimer );
				InternetCloseHandle(m_hConnect);
				InternetCloseHandle(m_hInternet);
				InternetCloseHandle(m_hRequest);
				m_busyCheckingList = false;
				m_callDownloadFiles = false;

				m_ProgressSpinner.StopSpinning();
				m_editStatus.SetWindowTextA( "Download Error: Updating List Failed" );
				m_editStatus.RedrawWindow();
				::MessageBox ( NULL, "Download Error: Updating List Failed", "Downloader Error", MB_OK | MB_SYSTEMMODAL );

			}
			else
			{
				m_TimerMode = 3;
			}
			return;
		}

		if ( m_TimerMode == 3 )
		{

					char  szData[4096];
					DWORD dwBytesRead;
							
					BOOL bRead = InternetReadFile( m_hRequest, szData, 4095, &dwBytesRead );
					// finished with the file, now we load it in, parse it and make our list
					if ( bRead == FALSE  ||  dwBytesRead == 0 )
					{
						m_TimerMode = 4;
					}
					fwrite( szData , 1 , dwBytesRead , m_file );
					szData[dwBytesRead] = 0;

					return;
		}

		if ( m_TimerMode == 4 )
		{

			fclose ( m_file );

			InternetCloseHandle(m_hConnect);
			InternetCloseHandle(m_hInternet);
			InternetCloseHandle(m_hRequest);

			m_NumberOfFilesToDownload = 0;

			m_TimerMode = 5;			

			// check if there are no files to download and skip to 6
			m_file = fopen ( m_fileName , "r" );
			char line1[4096];
			char line2[4096];
			fgets ( line1 , 4096 , m_file);
			fgets ( line2 , 4096 , m_file);
			fclose ( m_file );

			if ( strcmp( "<objects></objects>" , line1 ) == 0 || strcmp( "</objects>" , line2 ) == 0 ) 
			{
				m_TimerMode = 6;
				return;
			}
			
			m_file = fopen ( m_fileName , "r" );
			return;

		}
		

		if ( m_TimerMode == 5 )
		{		
			char line[4096*10];			

			if ( m_file )
			{
				for ( int checkAfewFiles = 0 ; checkAfewFiles < 5 ; checkAfewFiles++ )
				{
					if ( fgets ( line , 4096*10 , m_file ) != NULL )
					{

						/*if ( wf == NULL )
							wf = fopen ( "f:\\filelist.txt" , "w" );

						fwrite ( line , strlen(line) , sizeof(char) , wf );
						fwrite ( "\n" , 1 , sizeof(char) , wf );*/

						char * pch;
						pch = strtok (line,"<>");
						while (pch != NULL)
						{
							if ( strnicmp ( pch , "id" , strlen("id") ) == 0 && strnicmp ( pch , "identifiers" , strlen("identifiers") ) != 0 )
							{
								pch = strtok (NULL, "<>");
								strcpy ( storeItem.id , pch );
								storeItem.files.clear();
							}
							if ( strnicmp ( pch , "name" , strlen("name") ) == 0 )
							{
								pch = strtok (NULL, "<>");
								strcpy ( storeItem.name , pch );
							}
							if ( strnicmp ( pch , "download" , strlen("download") ) == 0 )
							{
								pch = strtok (NULL, "<>");
								char tempChar[MAX_PATH];
								strcpy ( tempChar , pch );
								char* pChar = url_decode ( tempChar );
								strcpy ( storeItem.url , pChar );
								free ( pChar );
							}
							if ( strnicmp ( pch , "item" , strlen("item") ) == 0 )
							{
								pch = strtok (NULL, "<>");
								sDownloadItem item;
								strcpy ( item.fileName , pch );
								storeItem.files.push_back(item);
							}
							if ( strnicmp ( pch , "checksum" , strlen("checksum") ) == 0 )
							{
								pch = strtok (NULL, "<>");
								//sDownloadItem item;
								strcpy ( storeItem.checksum , pch );
							}
							// end of this storeitem, now we can check if any files are missing, if so we need to add this storeitem to our list
							if ( strnicmp ( pch , "/object" , strlen("/object") ) == 0 )
							{
								bool fileMissing = false;

								if ( m_NumberOfFilesToDownload == 0 )
								{
									char status[256];
									if ( m_callDownloadFiles )
										sprintf ( status , "Checking list: %s" , storeItem.name );
									else
										sprintf ( status , "Checking for %s" , storeItem.name );
									m_editStatus.SetWindowTextA( status );
								}

								// First lets check if we have an md5 file, if not we need to download
								// If we do have the file lets ensure the numbers match up, if not we need the file
								char infoFileName[ MAX_PATH ];

								sprintf ( infoFileName , "%s\\Files\\editors\\DownloadInfo\\%s.txt" , theApp.m_szDirectory , storeItem.id );

								FILE* file = fopen ( infoFileName , "r" );
								if ( !file )
								{
									// no file to check against, so we need to download to ensure the latest version
									fileMissing = true;
								}
								else
								{
									char line[ MAX_PATH ];
									fgets ( line , MAX_PATH-1 , file );
									fclose ( file );

									if ( strcmp ( line , storeItem.checksum ) != 0 )
										fileMissing = true;
								}

								for ( int c = 0 ; c < (int)storeItem.files.size() ; c++ )
								{
									// Get rid of Purchased/TGC path
									char* replaced = replaceString ( storeItem.files[c].fileName , "Purchased/TGC/" , "" );
									strcpy ( storeItem.files[c].fileName , replaced );
								}

								// last check is to scan all files to see if any are missing - unless we have already decided to download the file anyway
								if ( fileMissing == false )
								{
									for ( int c = 0 ; c < (int)storeItem.files.size() ; c++ )
									{
										char fileToCheck [ MAX_PATH ];
										sprintf ( fileToCheck , "%s\\Files%s" , theApp.m_szDirectory , storeItem.files[c].fileName );

										// if file is missing we need to download the object
										if ( !FileExists ( fileToCheck ) )
										{
											fileMissing = true;
											break;
										}
									}
								}

								// add the storeitem into the list
								if ( fileMissing && strcmp ( storeItem.url , "" ) != 0 )
								{
									m_StoreItems.push_back(storeItem);
									m_NumberOfFilesToDownload = m_StoreItems.size();

									TCHAR szString [ 3 ] [ MAX_PATH ];
									GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Prep" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
									GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Number" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
									GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Access" ), _T ( "" ), szString [ 2 ], MAX_PATH, theApp.m_szLanguage );

									char status[256];
									if ( m_callDownloadFiles )
										//sprintf ( status , "Preparing to download files: %d.  To access the online store, close this dialogue and click on the bottom left icon. You will need to register a store account before you can download extra game assets." , m_NumberOfFilesToDownload );
										sprintf ( status , "%s: %d.  %s." , szString [ 0 ], m_NumberOfFilesToDownload, szString [ 2 ] );
									else
										//sprintf ( status , "Number of files to download: %d.  To access the online store, close this dialogue and click on the bottom left icon. You will need to register a store account before you can download extra game assets." , m_NumberOfFilesToDownload );
										sprintf ( status , "%s: %d.  %s." , szString [ 1 ], m_NumberOfFilesToDownload, szString [ 2 ] );
									m_editStatus.SetWindowTextA( status );
								}
							}

							pch = strtok (NULL, "<>");
						}
					}
					else
					{
						fclose ( m_file );
						DeleteFile ( m_fileName );
						m_TimerMode = 6;
						return;
					}

				}
			}

			return;

		}

		if ( m_TimerMode == 6 )
		{

			if ( m_NumberOfFilesToDownload == 0 )
				m_editStatus.SetWindowTextA( "Nothing to download. To access the online store, close this dialogue and click on the bottom left icon. You will need to sign in with your Steam account before you can download extra game assets." );


			m_Lines[3].EnableWindow ( 1 );
			m_TimerMode = 0;
			KillTimer ( m_pTimer );
			InternetCloseHandle(m_hConnect);
			InternetCloseHandle(m_hInternet);
			InternetCloseHandle(m_hRequest);
			m_busyCheckingList = false;
			m_ProgressSpinner.StopSpinning();

			if ( m_callDownloadFiles )
			{
				m_callDownloadFiles = false;
				m_TimerMode = 1;
				m_pTimer = SetTimer ( 64, 1, NULL );
			}

		}

		return;
	}

	//===================================================================================================================

	
	if ( nIDEvent == 64 )
	{

		if ( m_TimerMode == 0 )
		{
			KillTimer ( m_pTimer );	
			return;
		}

		if ( m_TimerMode == 1 )
		{

			if ( m_NumberOfFilesToDownload == 0  )
			{

				m_TimerMode = 0;
				KillTimer ( m_pTimer );			
				::MessageBox ( NULL, "No files to download!", "Nothing to download", MB_OK | MB_SYSTEMMODAL );

				return;
			}

			m_ProgressSpinner.StartSpinning();
			m_ProgressSpinner.UpdateWindow();

			downloadTriesBeforeGiveUp = 10;
			// reset bytes downloaded to enable resuming of files
			bytesDownloaded = 0;

			char folder [ MAX_PATH ];
			sprintf ( folder , "%s\\Files\\editors\\TEMP" , theApp.m_szDirectory );
			CreateDirectoryA ( folder , NULL );
			sprintf ( folder , "%s\\Files\\editors\\DownloadInfo" , theApp.m_szDirectory );
			CreateDirectoryA ( folder , NULL );

			if ( !m_bIsLoggedIn )
			{
				m_TimerMode = 0;
				KillTimer ( m_pTimer );
				::MessageBox ( NULL, "You must login first!", "Login Error", MB_OK | MB_SYSTEMMODAL );
				return;
			}

			m_hInternet = InternetOpen( TEXT("FPSCDownload"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL,0 );
			if ( m_hInternet != NULL )
			{

				DWORD timeout = 4000;
				InternetSetOption ( m_hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof ( timeout ) );
				InternetSetOption ( m_hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof ( timeout ) );
				InternetSetOption ( m_hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof ( timeout ) );

				char tServerName[512];
				if ( m_storeCount >= (int)m_StoreItems.size() ) m_storeCount = 0;
				strcpy ( tServerName , &m_StoreItems[m_storeCount].url[8] );
				int tSlashCount = 0;
				for ( int tfindLoop = 0 ; tfindLoop < (int)strlen(tServerName) ; tfindLoop++ )
				{
					if ( tServerName[tfindLoop] == '/' )
					{
						tSlashCount++;
						if ( tSlashCount == 1 )
						{
							tServerName[tfindLoop] = '\0';
							break;
						}
					}
				}
				m_hConnect = InternetConnect( m_hInternet, TEXT(tServerName), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL );

				if ( m_hConnect != NULL )
				{
					m_ProgressAmount = 0;		

					m_storeCount = -1;
					m_TimerMode = 2;
				}
				else
				{
					InternetCloseHandle(m_hInternet);
					m_TimerMode = 0;
					KillTimer ( m_pTimer );
					::MessageBox ( NULL, "Could not connect to the server (connect failed - 650)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
					DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "InternetConnect failed 624" );
					return;
				}
			}
			else
			{
				m_TimerMode = 0;
				KillTimer ( m_pTimer );
				::MessageBox ( NULL, "Could not connect to the internet", "Connection Error - 659", MB_OK | MB_SYSTEMMODAL );
				DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "InternetOpen failed 633");
				return;
			}

		}

		if ( m_TimerMode == 2 )
		{
			if ( ++m_storeCount >= (int)m_StoreItems.size() )
			{
				m_TimerMode = 0;
				KillTimer ( m_pTimer );
				InternetCloseHandle(m_hConnect);
				InternetCloseHandle(m_hInternet);
				InternetCloseHandle(m_hRequest);

				TCHAR szString [ 2 ] [ MAX_PATH ];
				GetPrivateProfileString ( _T ( "Download Store" ), _T ( "AllFiles" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Complete" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );

				m_ProgressSpinner.StopSpinning();
				m_StoreItems.clear();
				m_editStatus.SetWindowTextA( szString[0] );
				m_editStatus.RedrawWindow();
				m_StoreItems.clear();
				::MessageBox ( NULL, szString[0], szString[1], MB_OK | MB_SYSTEMMODAL );
				return;
			}

			m_FileProgress = (float)bytesDownloaded;
			int tCount = 0;
			int tSlashCount = 0;
			for ( int tfindLoop = 0 ; tfindLoop < (int)strlen(m_StoreItems[m_storeCount].url) ; tfindLoop++ )
			{
				if ( m_StoreItems[m_storeCount].url[tfindLoop] == '/' )
				{
					tSlashCount++;
					if ( tSlashCount == 3 )
					{
						tCount = tfindLoop+1;
						break;
					}
				}
			}
			strcpy ( m_listURL , &m_StoreItems[m_storeCount].url[tCount] );

			const char* lplpszAcceptTypes[] = {"text/xml", "application/xml", "application/xhtml+xml", NULL};

			DWORD dwFlags = INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID|
				INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_NO_CACHE_WRITE|
				INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
				INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_KEEP_CONNECTION |
				INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE;
			
			m_hRequest = HttpOpenRequest( m_hConnect , "GET", m_listURL , NULL, NULL, lplpszAcceptTypes, dwFlags, 0);	

			char lpszHeaders[256];
			sprintf ( lpszHeaders , "Referer: google.com" );
			BOOL bReqResult = HttpAddRequestHeadersA ( m_hRequest , lpszHeaders, strlen ( lpszHeaders ) , HTTP_ADDREQ_FLAG_ADD || HTTP_ADDREQ_FLAG_REPLACE );

			// to enable resuming of downloads
			if ( bytesDownloaded > 0 )
			{
				char lpszHeaders[256];
				sprintf ( lpszHeaders , "Range: bytes=-%d", m_FileLength-bytesDownloaded );
				BOOL bReqResult = HttpAddRequestHeadersA ( m_hRequest , lpszHeaders, strlen ( lpszHeaders ) , HTTP_ADDREQ_FLAG_ADD || HTTP_ADDREQ_FLAG_REPLACE );
			}

			if ( m_hRequest != NULL )
			{
				BOOL bSend = HttpSendRequest(m_hRequest, NULL,0, NULL,0);
				if ( bSend )
				{
					// Get the size of the requested file
					char achQueryBuf[16];
					m_FileLength = 0xffffffff;
					DWORD dwQueryBufLen = sizeof(achQueryBuf);
					BOOL bQuery = ::HttpQueryInfo(m_hRequest, HTTP_QUERY_CONTENT_LENGTH, achQueryBuf, &dwQueryBufLen, NULL);

					if (bQuery)
					{
						// The query succeeded, grab size of file
						m_FileLength = (DWORD)atol(achQueryBuf);
					}

					char status[1024];
					sprintf ( status , "Downloading %s, %d file of %d..." , m_StoreItems[m_storeCount].name , m_storeCount+1 , m_StoreItems.size() );
					m_editStatus.SetWindowTextA( status );
					m_editStatus.SetRedraw(1);
					m_editStatus.RedrawWindow();

					//m_editStatus.SetRedraw(1);
					//m_editStatus.RedrawWindow();
					//m_ProgressSpinner.UpdateWindow();
					m_ProgressSpinner.StartSpinning();
					
					char fileName [ MAX_PATH ];
					sprintf ( fileName , "%s\\Files\\editors\\TEMP\\_%d_.zip" , theApp.m_szDirectory , m_storeCount );
					// only open the file if we are not resuming
					if ( bytesDownloaded == 0 )
						m_file = fopen ( fileName , "wb" );

					m_TimerMode = 3;

					if ( m_file == NULL )
					{						
						m_ProgressSpinner.StopSpinning();
						m_editStatus.SetWindowTextA( "Download Failed" );
						m_editStatus.RedrawWindow();

						if ( --downloadTriesBeforeGiveUp ) 
						{

							m_ProgressSpinner.StopSpinning();
							char ts[MAX_PATH];
							sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
							m_editStatus.SetWindowTextA( ts );
							m_editStatus.RedrawWindow();

							m_storeCount--;
							m_TimerMode = 2;
							InternetCloseHandle(m_hRequest);
							return;
						}

						bytesDownloaded = 0;

						DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "Download failed 744");

						m_TimerMode = 0;
						KillTimer ( m_pTimer );
						InternetCloseHandle(m_hConnect);
						InternetCloseHandle(m_hInternet);
						InternetCloseHandle(m_hRequest);
						::MessageBox ( NULL, "Could not connect to the server (send request failed)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
						return;
					}
				}
				else
				{					
					m_ProgressSpinner.StopSpinning();
					m_editStatus.SetWindowTextA( "Download Failed" );					
					m_editStatus.RedrawWindow();

					if ( --downloadTriesBeforeGiveUp ) 
					{

						m_ProgressSpinner.StopSpinning();
						char ts[MAX_PATH];
						sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
						m_editStatus.SetWindowTextA( ts );
						m_editStatus.RedrawWindow();

						m_storeCount--;
						m_TimerMode = 2;
						InternetCloseHandle(m_hRequest);
						return;
					}

					bytesDownloaded = 0;

					DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "Download failed - did not receive data 776");

					m_TimerMode = 0;
					KillTimer ( m_pTimer );
					InternetCloseHandle(m_hConnect);
					InternetCloseHandle(m_hInternet);
					InternetCloseHandle(m_hRequest);
					::MessageBox ( NULL, "Could not connect to the server (send request failed)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
					return;
				}
			}
			else
			{
				m_ProgressSpinner.StopSpinning();
				m_editStatus.SetWindowTextA( "Download Failed" );				
				m_editStatus.RedrawWindow();

				if ( --downloadTriesBeforeGiveUp ) 
				{

					m_ProgressSpinner.StopSpinning();
					char ts[MAX_PATH];
					sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
					m_editStatus.SetWindowTextA( ts );
					m_editStatus.RedrawWindow();

					m_storeCount--;
					m_TimerMode = 2;
					return;
				}

				bytesDownloaded = 0;

				DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "HttpOpenRequest failed 807");

				m_TimerMode = 0;
				KillTimer ( m_pTimer );
				InternetCloseHandle(m_hConnect);
				InternetCloseHandle(m_hInternet);
				::MessageBox ( NULL, "Could not connect to the server (open request failed)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
				return;
			}

		}

		if ( m_TimerMode == 3 )
		{
								
			::SendMessage ( NULL , WM_PAINT , 0 , 0 );

			char  szData[1024*40];
			DWORD dwBytesRead;
							
			BOOL bRead = InternetReadFile( m_hRequest, szData, sizeof(szData) , &dwBytesRead );

			if ( bRead == false )
			{				
				m_ProgressSpinner.StopSpinning();
				m_editStatus.SetWindowTextA( "Download Failed, Connection timed out." );				
				m_editStatus.RedrawWindow();

				if ( --downloadTriesBeforeGiveUp ) 
				{

					m_ProgressSpinner.StopSpinning();
					char ts[MAX_PATH];
					sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
					m_editStatus.SetWindowTextA( ts );
					m_editStatus.RedrawWindow();

					m_storeCount--;
					m_TimerMode = 2;
					InternetCloseHandle(m_hRequest);
					return;
				}

				bytesDownloaded = 0;

				DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "InternetReadFile failed 850");

				m_TimerMode = 0;
				KillTimer ( m_pTimer );

				InternetCloseHandle(m_hConnect);
				InternetCloseHandle(m_hInternet);
				InternetCloseHandle(m_hRequest);

				fclose ( m_file );
				char fileName [ MAX_PATH ];
				sprintf ( fileName , "%s\\Files\\editors\\TEMP\\_%d_.zip" , theApp.m_szDirectory , m_storeCount );
				DeleteFile ( fileName );

				::MessageBox ( NULL, "Connection timed out. Error 866", "Connection Error", MB_OK | MB_SYSTEMMODAL );
				return;
			}

			// finished with the file
			if ( dwBytesRead == 0 )
			{
				fclose ( m_file );

				// reset bytes downloaded
				bytesDownloaded = 0;

				char fileName [ MAX_PATH ];
				sprintf ( fileName , "%s\\Files\\editors\\TEMP\\_%d_.zip" , theApp.m_szDirectory , m_storeCount );

				// MD5 Checksum the file
				FILE* file = fopen ( fileName , "rb" );
				MD5 md5 ( file );
				fclose ( file );			
				char* md5String = md5.hex_digest ( );

				// Checksum passed - we have the file!
				if ( strcmp ( md5String , m_StoreItems[m_storeCount].checksum ) == 0 )
				{
					char infoFileName[ MAX_PATH ];

					sprintf ( infoFileName , "%s\\Files\\editors\\DownloadInfo\\%s.txt" , theApp.m_szDirectory , m_StoreItems[m_storeCount].id );

					FILE* file = fopen ( infoFileName , "w" );
					fputs ( m_StoreItems[m_storeCount].checksum , file );
					fclose ( file );

					m_ProgressSpinner.StopSpinning();
					m_editStatus.SetWindowTextA( "Download Succeeded." );
					m_editStatus.RedrawWindow();

					m_NumberOfFilesToDownload--;
					m_ProgressAmount = ( ((float)m_storeCount+1.0f) / (float)m_StoreItems.size() ) * 100.0f;
					m_ProgressBar.SetPos((int)m_ProgressAmount);
				
					m_TimerMode = 4;
					return;
				}
				else // Checksum failed
				{

					char fileName [ MAX_PATH ];
					sprintf ( fileName , "%s\\Files\\editors\\TEMP\\_%d_.zip" , theApp.m_szDirectory , m_storeCount );
					DeleteFile ( fileName );

					if ( --downloadTriesBeforeGiveUp ) 
					{

						m_ProgressSpinner.StopSpinning();
						char ts[MAX_PATH];
						sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
						m_editStatus.SetWindowTextA( ts );
						m_editStatus.RedrawWindow();
						InternetCloseHandle(m_hRequest);

						m_storeCount--;
						m_TimerMode = 2;
					}
					else 
					{
						bytesDownloaded = 0;
						m_ProgressSpinner.StopSpinning();
						char ts[MAX_PATH];
						sprintf ( ts , "Download of %s failed..." , m_StoreItems[m_storeCount].name );
						DownloadStoreItemsLog(m_StoreItems[m_storeCount].id, "checksum failed 929" );
						m_editStatus.SetWindowTextA( ts );
						m_editStatus.RedrawWindow();

						m_TimerMode = 0;
						KillTimer ( m_pTimer );

						InternetCloseHandle(m_hConnect);
						InternetCloseHandle(m_hInternet);
						InternetCloseHandle(m_hRequest);

						::MessageBox ( NULL, "Download Failed. You may have temporarily lost server connection. Try clicking the DOWNLOAD button to resume your download activity. (Error 943)", "Connection Error", MB_OK | MB_SYSTEMMODAL );
						return;
					}
				}

			}

			m_ProgressAmount = ( ((float)m_storeCount+0.0f) / (float)m_StoreItems.size() ) * 100.0f;
			m_FileProgress += dwBytesRead;
			// track how far through the file we are to allow resuming
			bytesDownloaded += dwBytesRead;
			float FileProgressAmount = ( ((float)m_FileProgress) / (float)m_FileLength ) * 100.0f;
			FileProgressAmount = FileProgressAmount * (100.0f / (float)m_StoreItems.size() ) / 100.0f;
			m_ProgressBar.SetPos((int)m_ProgressAmount + int(FileProgressAmount));

			fwrite( szData , 1 , dwBytesRead , m_file );
			fflush(m_file);

			// resume test code, might be handy so leaving it here for now
			/*
			switch ( downloadFailTest )
			{
				case 0:
				{
					if ( bytesDownloaded > 1024*10 )
					{
						downloadFailTest = 1;
						m_ProgressSpinner.StopSpinning();
						char ts[MAX_PATH];
						sprintf ( ts , "Retrying %s..." , m_StoreItems[m_storeCount].name );
						m_editStatus.SetWindowTextA( ts );
						m_editStatus.RedrawWindow();
						InternetCloseHandle(m_hRequest);

						m_storeCount--;
						m_TimerMode = 2;
					}
				}
				break;
			}
			*/
		}

		if ( m_TimerMode == 4 )
		{
				// Now extract files from the zip
				char status[1024];
				sprintf ( status , "Extracting %s, %d file of %d downloaded..." , m_StoreItems[m_storeCount].name , m_storeCount+1 , m_StoreItems.size() );
				m_editStatus.SetWindowTextA( status );
				m_editStatus.SetRedraw(1);
				m_editStatus.RedrawWindow();

				sprintf ( m_fileToExtract , "_%d_.zip" , m_storeCount );
				DownloadProcessExtractOrCleanZip ( m_fileToExtract , false );
				m_TimerMode = 5;
				m_fileCountLoop = -1;
				return;

		}

		if ( m_TimerMode == 5 )
		{
			if ( ++m_fileCountLoop >= (int)m_StoreItems[m_storeCount].files.size() )
			{
				m_TimerMode = 6;
				return;
			}

			// copy the file to the correct location, making folders as needed
			char buildPath[MAX_PATH];
			strcpy ( buildPath, theApp.m_szDirectory );
			strcat ( buildPath, "\\Files" );

			char originalPath[MAX_PATH];
			char tempPath[MAX_PATH];
			strcpy ( originalPath , m_StoreItems[m_storeCount].files[m_fileCountLoop].fileName );
			strcpy ( tempPath , m_StoreItems[m_storeCount].files[m_fileCountLoop].fileName );

			// make all the folders we will need
			char * pch;
			pch = strtok (tempPath,"/\\");
			while (pch != NULL)
			{

				strcat ( buildPath, "\\" );
				strcat ( buildPath, pch );

				// create a folder as long as there is not a . in the name!
				// if there is a . then its the file and we copy it
				if ( strstr ( pch , "." ) == NULL )
					CreateDirectoryA ( buildPath , NULL );
				else
				{
					//char copyFrom[MAX_PATH];
					// zips vary sometimes so we will try the two methods they use folder structure wise
					// original items unzipped to the root of their main folder (like entitybank)
					// where the newer gun items unzip to their exact structure
					// since i wont know which they will choose, i will attemt to copy from both locations

					//method 1
					strcpy ( copyFrom, theApp.m_szDirectory );
					strcat ( copyFrom, "\\Files\\editors\\TEMP" );
					strcat ( copyFrom, tempPath );
					strcat ( copyFrom, "\\" );
					strcat ( copyFrom, pch );

					// Udpdate the users on progress
					char copyString[MAX_PATH];
					sprintf ( copyString , "Copying file %s, %d file of %d downloaded..." , pch , m_storeCount+1 , m_StoreItems.size() );
					m_editStatus.SetWindowTextA( copyString );
					m_editStatus.SetRedraw(1);
					m_editStatus.RedrawWindow();

					CopyFile ( copyFrom , buildPath , false );

					//method 2
					strcpy ( copyFrom, theApp.m_szDirectory );
					strcat ( copyFrom, "\\Files\\editors\\TEMP" );
					strcat ( copyFrom,  m_StoreItems[m_storeCount].files[m_fileCountLoop].fileName );
					//strcat ( copyFrom, "\\" );
					//strcat ( copyFrom, pch );

					CopyFile ( copyFrom , buildPath , false );
				}

											
				pch = strtok (NULL, "/\\");
			}

			return;

		}

		if ( m_TimerMode == 6 )
		{

			// Clean up extracted files and delete zip
			DownloadProcessExtractOrCleanZip ( m_fileToExtract , true );

			m_TimerMode = 2;
			return;
		}



	} // end of timer event
}

void CDownloadStoreItems::OnChangeLoginClicked()
{
	// using steam login now so return out
	return;

	if ( m_busyCheckingList ) return;

	if ( m_TimerMode != 0 )
	{
		TCHAR szString [ 2 ] [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Wait" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Progress" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
		::MessageBox ( NULL, szString[0], szString[1], MB_OK | MB_SYSTEMMODAL );
		return;
	}

	CLoginDialog2 cLoginBox;
	cLoginBox.DoModal();
	if ( UserLogin() )
	{
		m_busyCheckingList = true;
		m_callDownloadFiles = false;
		UpdateList();
	}
	else
	{
		m_editStatus.SetWindowTextA( "Not Logged in to the Game Guru Store." );
		m_busyCheckingList = false;
	}
}

void CDownloadStoreItems::OnBnClickedCancel()
{
	if ( m_TimerMode != 0 )
	{
		TCHAR szString [ 2 ] [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Wait" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Progress" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
		::MessageBox ( NULL, szString[0], szString[1], MB_OK | MB_SYSTEMMODAL );
		return;
	}

	// Clean up empty folders
	cleanupFolders();

	//if download in progress
	//int hResult = MessageBox( "Are you sure you want to cancel your upload?", "Confirm", MB_YESNO );
	//if ( hResult = IDYES ) 
	//{
	//	OnCancel();
	//}
	OnCancel();
}

// Attempt to log the user in
bool CDownloadStoreItems::UserLogin()
{
	if ( m_busyCheckingList ) return false;

	if ( theApp.GetFileMapData ( 6145 ) == 1 )
	{
		theApp.GetFileMapDataString ( 6149 , userSteamID );

		if ( strcmp ( userSteamID, "" ) != 0 )
		{
			haveSteamUserID = 1;
		}
		else
		{
			::MessageBox ( NULL, "Could not connect to Steam. Please check your internet connection and try again." , "Connection Error" , MB_OK | MB_SYSTEMMODAL );
			m_editStatus.SetWindowTextA( "Could not connect to Steam. Please check your internet connection and try again." );
		}
	}

	TCHAR szString [ 2 ] [ MAX_PATH ];
	GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Wait" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
	GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Progress" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
	if ( m_TimerMode != 0 )
	{
		::MessageBox ( NULL, szString[0], szString[1], MB_OK | MB_SYSTEMMODAL );
		return false;
	}

	GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Logging" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
	m_editStatus.SetWindowTextA(szString[0] );

	m_busyCheckingList = true;

	m_bIsLoggedIn = false;

	// grab login info from the app
	//strcpy ( m_szUserName , theApp.m_sUsername );
	//strcpy ( m_szPassword , theApp.m_sPassword );

	//if ( strcmp ( m_szUserName , "" ) == 0 || strcmp ( m_szPassword , "" ) == 0 )
	if ( strcmp ( userSteamID , "" ) == 0 )
	{
		if ( !m_bFirstTimeLogin )
		{
			GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Invalid" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
			GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Error" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
			::MessageBox ( NULL, szString[0], szString[1], MB_OK | MB_SYSTEMMODAL );
			m_editStatus.SetWindowTextA( "Not Logged in." );
		}

		m_editStatus.SetWindowTextA( "Not Logged in to the Game Guru Store." );
		m_busyCheckingList = false;
		m_bFirstTimeLogin = false;

		return false;
	}

	//char* pURLEncodedLoginName = url_encode ( m_szUserName );
	//char* pURLEncodedPassword = url_encode ( m_szPassword );

	char loginURL[MAX_PATH];
	//sprintf ( loginURL , "attemptLogin.php?user=%s&pass=%s" , pURLEncodedLoginName , pURLEncodedPassword );
	sprintf ( loginURL , "attemptLogin.php?steamid=%s" , userSteamID );

	//free ( pURLEncodedLoginName );
	//free ( pURLEncodedPassword );

	 HINTERNET hInternet = InternetOpen( TEXT("FPSCDownload"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL,0 );
		if ( hInternet != NULL )
		{

			DWORD timeout = 4000;
			InternetSetOption ( hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof ( timeout ) );
			InternetSetOption ( hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof ( timeout ) );

			HINTERNET hConnect = InternetConnect( hInternet, TEXT("api.tgcstore.net"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL );

			if ( hConnect != NULL )
			{
				const char* lplpszAcceptTypes[] = {"text/xml", "application/xml", "application/xhtml+xml", NULL};

				HINTERNET hRequest = HttpOpenRequest( hConnect , "GET", loginURL , NULL, NULL, lplpszAcceptTypes,
				INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID|SECURITY_FLAG_IGNORE_UNKNOWN_CA|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID|
				INTERNET_FLAG_NO_AUTO_REDIRECT|INTERNET_FLAG_PRAGMA_NOCACHE|INTERNET_FLAG_NO_CACHE_WRITE|
				INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
				INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS | INTERNET_FLAG_KEEP_CONNECTION |
				INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE, 0);

				//HINTERNET hRequest = HttpOpenRequest( hConnect, TEXT("GET"), TEXT(loginURL), NULL, NULL, 0, 0, 1 );
				if ( hRequest != NULL )
				{
					BOOL bSend = HttpSendRequest(hRequest, NULL,0, NULL,0);
					if ( bSend )
					{
						while ( 1 )
						{
							char  szData[256];
							DWORD dwBytesRead;
							BOOL bRead = InternetReadFile( hRequest, szData, sizeof(szData)-1, &dwBytesRead );
							if ( bRead == FALSE  ||  dwBytesRead == 0 )
							{

								char * pch;
								pch = strtok (szData,"<>");
								while (pch != NULL)
								{
									if ( strnicmp ( pch , "userid" , strlen("userid") ) == 0 )
									{
										pch = strtok (NULL, "<>");
										strcpy ( m_szUserID , pch );
										m_bIsLoggedIn = true;

										TCHAR szString [ 3 ] [ MAX_PATH ];
										GetPrivateProfileString ( _T ( "Download Store" ), _T ( "Grabbing" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
										m_editStatus.SetWindowTextA( szString[0] );

										return true;
									}
									// grab the error message from the site if possible
									if ( strnicmp ( pch , "error" , strlen("error") ) == 0 )
									{
										pch = strtok (NULL, "<>");

										::MessageBox ( NULL, pch , "InternetConnect", MB_OK | MB_SYSTEMMODAL );
										m_busyCheckingList = false;
										return false;
									}
									pch = strtok (NULL, "<>");
								}

								break;
							}
							szData[dwBytesRead] = 0;
						}
					}
					else
					{
						::MessageBox ( NULL, "Could not connect to the server (send request failed: Error 1233)", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
						m_busyCheckingList = false;
						return false;
					}
					InternetCloseHandle(hRequest);
				}
				else
				{
					::MessageBox ( NULL, "Could not connect to the server (open request failed: Error 1241)", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
					m_busyCheckingList = false;
					return false;
				}
				InternetCloseHandle(hConnect);
			}
			else
			{
				::MessageBox ( NULL, "Could not connect to the server (connect failed: Error 1249)", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
				m_busyCheckingList = false;
				return false;
			}
			InternetCloseHandle(hInternet);
		}
		else
		{
			::MessageBox ( NULL, "Could not connect to the internet (Error 1257)", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
			m_busyCheckingList = false;
			return false;
		}

	::MessageBox ( NULL, "Login failed - please try again (Error 1262)", "InternetConnect", MB_OK | MB_SYSTEMMODAL );
	m_busyCheckingList = false;
	return false;
}

void DownloadStoreItemsLog( char* s, char* s2)
{ 
	char logText[512];
	//
	char timestampstring[256] = "";
	time_t t = time ( NULL );
	struct tm tm = *localtime ( &t );
	sprintf ( timestampstring, "%i/%i/%i %i:%i", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min );
	//
	sprintf ( logText, "%s - %s - %s" , s, s2, timestampstring );
	char logfilename[512];
	sprintf ( logfilename , "%s\\Files\\editors\\DownloadStoreItemsLog.txt" , theApp.m_szDirectory );
	FILE* logFile = NULL;
	logFile = fopen ( logfilename , "w" );
	if ( logFile )
	{
		fputs ( logText , logFile );
		fputs ( "\n" , logFile );
		fclose ( logFile );
	}
}


// UploadGame.cpp : implementation file
//

#include "stdafx.h"
#include "UploadGame.h"
#include "Editor.h"
#include <atlimage.h>
#include "LoginDialog2.h"

// CUploadGame dialog

IMPLEMENT_DYNAMIC(CUploadGame, CDialog)

CUploadGame::CUploadGame(CWnd* pParent /*=NULL*/)
	: CDialog(CUploadGame::IDD, pParent)
{
	m_bRunning = false;
	m_sThumbnailFile = "";
	m_bUpdateMessage = false;
	pClubList = NULL;
	iChosenClub = 0;
}

CUploadGame::~CUploadGame()
{
	ClearClubList();
}

void CUploadGame::AddClub( int id, const char *name )
{
	sClub *pNewClub = new sClub();
	pNewClub->iClubID = id;
	strcpy_s( pNewClub->szName, 50, name );
	pNewClub->pNextClub = pClubList;
	pClubList = pNewClub;
}

void CUploadGame::ClearClubList() 
{ 
	while( pClubList ) 
	{
		sClub *temp = pClubList;
		pClubList = pClubList->pNextClub;
		delete temp;
	}
}

int CUploadGame::CountClubs() 
{ 
	int count = 0;
	sClub *pClub = pClubList;
	while( pClub ) 
	{
		count++;
		pClub = pClub->pNextClub;
	}

	return count;
}

int CUploadGame::FindClubID( const char *name )
{
	sClub *pClub = pClubList;
	while( pClub ) 
	{
		if ( strcmp( name, pClub->szName ) == 0 ) return pClub->iClubID;
		pClub = pClub->pNextClub;
	}

	return 0;
}

void CUploadGame::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_editDescription);
	DDX_Control(pDX, IDC_EDIT2, m_editGameFolder);
	DDX_Control(pDX, IDC_IMAGE_THUMBNAIL, m_ImageThumbnail);
	DDX_Control(pDX, IDC_EDIT3, m_editStatusMessages);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_PROGRESS_SPINNER, m_ProgressSpinner);
	DDX_Control(pDX, IDOK, m_btnUpload);
	DDX_Control(pDX, IDC_UPDATE_IMAGE2, m_ImageUpload);
	DDX_Control(pDX, IDC_SPACE_USED, m_textSpaceUsed);
	DDX_Control(pDX, IDC_COMBO1, m_comboUploadTo);
	DDX_Control(pDX, IDC_UPLOAD_TO, m_textUploadTo);
}


BEGIN_MESSAGE_MAP(CUploadGame, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CUploadGame::OnBrowseThumbnail)
	ON_BN_CLICKED(IDC_BUTTON2, &CUploadGame::OnBrowseGameFolder)
	ON_BN_CLICKED(IDOK, &CUploadGame::OnUploadClicked)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON3, &CUploadGame::OnChangeLoginClicked)
	ON_BN_CLICKED(IDCANCEL, &CUploadGame::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CUploadGame::OnChooseClub)
END_MESSAGE_MAP()

BOOL CUploadGame::OnInitDialog( )
{
	if ( !CDialog::OnInitDialog( ) ) return FALSE;

	CImage image;
	HRESULT hr = image.Load( "Files\\editors\\gfx\\Default Thumbnail.jpg" );
	if ( !FAILED(hr) ) 
	{ 
		m_sThumbnailFile = "Files\\editors\\gfx\\Default Thumbnail.jpg";

		CWindowDC wndDC( this );
		CDC dc;	dc.CreateCompatibleDC( &wndDC );

		CBitmap bmpThumb;
		bmpThumb.CreateBitmap( 64,64, 1, 32, NULL );
		CBitmap *oldBmp = dc.SelectObject( &bmpThumb );
		
		::SetStretchBltMode( image.GetDC(), HALFTONE ); image.ReleaseDC();
		image.StretchBlt( dc.GetSafeHdc( ), 0,0, 64,64 );

		dc.SelectObject( oldBmp );
		m_ImageThumbnail.CopyBitmap( &bmpThumb );
	}

	m_ImageUpload.LoadBitmap( IDB_UPDATE_LOGO1 );
	m_editDescription.SetWindowText( "A game made in First Person Game Creator" );

	SetTimer( 3, 1000, NULL );

	return TRUE;
}

void CUploadGame::UpdateSpaceUsed()
{
	if ( theApp.m_sUsername.GetLength() > 0 && theApp.m_sPassword.GetLength() > 0 )
	{
		HTTPComm server;
		if ( server.Login2( theApp.m_sUsername, theApp.m_sPassword ) ) 
		{
			//get upload usage and limits
			LPCSTR szReply = server.GetClubLimits( theApp.m_szUploadKey, iChosenClub );

			if ( !szReply )
			{
				char str [ 256 ];
				sprintf_s( str, 256, "Failed to get upload limits: %s", server.GetLastError( ) );
				m_editStatusMessages.SetWindowText( str );
			}
			else
			{
				if ( strncmp( szReply, "<success>", strlen("<success>") ) != 0 ) 
				{
					m_editStatusMessages.SetWindowText( szReply );
				}
				else
				{
					const char *szStart = szReply + 9;
					const char *szMid = strchr( szStart, ':' );
					DWORD dwLength = szMid - szStart;
					
					//used
					char szUsed[50];
					strncpy_s( szUsed, 50, szStart, dwLength );
					szUsed[ dwLength ] = '\0';

					szMid++;
					char szUsed2[50];
					sprintf_s( szUsed2, 50, "Space Used: %s MB of %s MB", szUsed, szMid );
					m_textSpaceUsed.SetWindowText( szUsed2 );
				}

				delete [] szReply;
			}
		}
	}
}

void CUploadGame::UpdateClubsList()
{
	while (m_comboUploadTo.GetCount() > 0) m_comboUploadTo.DeleteString(0);
	ClearClubList();

	if ( theApp.m_sUsername.GetLength() > 0 && theApp.m_sPassword.GetLength() > 0 )
	{
		HTTPComm server;
		if ( server.Login2( theApp.m_sUsername, theApp.m_sPassword ) ) 
		{
			//get upload usage and limits
			LPCSTR szReply = server.GetClubList( theApp.m_szUploadKey );

			if ( !szReply )
			{
				char str [ 256 ];
				sprintf_s( str, 256, "Failed to get club list: %s", server.GetLastError( ) );
				m_editStatusMessages.SetWindowText( str );
			}
			else
			{
				if ( strncmp( szReply, "<success>", strlen("<success>") ) != 0 ) 
				{
					m_editStatusMessages.SetWindowText( szReply );
				}
				else
				{
					//correct format is <success>ID:name;ID:name;ID:name;
					const char *szStart = szReply + 9;
					const char *szNext = strchr( szStart, ';' );
					while( szNext )
					{
						const char *szMid = strchr( szStart, ':' );
						if ( szMid > szNext ) break; //colon should appear before semi-colon

						DWORD dwLength = szMid - szStart;
						
						//used
						char szID[11];
						strncpy_s( szID, 11, szStart, dwLength );
						szID[ dwLength ] = '\0';
						int iID = atoi( szID );

						szMid++;
						dwLength = szNext - szMid;

						char szName[50];
						strncpy_s( szName, 50, szMid, dwLength );
						szName[ dwLength ] = '\0';

						AddClub( iID, szName );
						szStart = szNext + 1;
						szNext = strchr( szStart, ';' );
					}
				}

				delete [] szReply;
			}
		}
	}

	int iNumClubs = CountClubs();
	if ( iNumClubs > 1 ) 
	{
		int iIndex = -1;
		int iCount = 0;
		sClub *pClub = pClubList;
		while ( pClub )
		{
			//keep the chosen club selected if it still exists
			if ( iChosenClub == pClub->iClubID ) iIndex = iCount;
			iCount++;
			m_comboUploadTo.AddString(pClub->szName);
			pClub = pClub->pNextClub;
		}

		//existing selection no longer valid, chose first club
		if ( iIndex < 0 )
		{
			iIndex = 0;
			iChosenClub = pClubList->iClubID; 
			UpdateSpaceUsed(); 
		}

		//items added in reverse order
		iIndex = (iCount-1) - iIndex;
		
		m_comboUploadTo.SetCurSel(iIndex);
		m_comboUploadTo.ShowWindow( SW_SHOW );
		m_textUploadTo.ShowWindow( SW_SHOW );
	}
	else 
	{
		// FPGC - 230610 - crashed on XP (but not Vista/7, how odd)
		if ( pClubList )
			iChosenClub = pClubList->iClubID;
		else
			iChosenClub = 0;

		UpdateSpaceUsed(); 
	}
}

unsigned CUploadGame::Run( )
{
	m_bRunning = true;
	DoModal( );
	m_bRunning = false;
	return 0;
}

// CUploadGame message handlers

void CUploadGame::OnBrowseThumbnail()
{
	CFileDialog fileSelect( TRUE, NULL, NULL, OFN_FILEMUSTEXIST, "JPG Files (*.jpg)|*.jpg||" );
	int result = (int) fileSelect.DoModal();
	if ( result != IDOK ) return;

	CString sFullPath = fileSelect.GetPathName( );
	m_sThumbnailFile = sFullPath.GetString();

	CImage image;
	HRESULT hr = image.Load( sFullPath );
	if ( FAILED(hr) ) { MessageBox( "Failed to load thumbnail" ); return; }

	//if ( image.GetHeight() != 64 ) { MessageBox( "Thumbnails must be 64x64 pixels" ); return; }
	//if ( image.GetWidth() != 64 ) { MessageBox( "Thumbnails must be 64x64 pixels" ); return; }

	CWindowDC wndDC( this );
	CDC dc;	dc.CreateCompatibleDC( &wndDC );

	CBitmap bmpThumb;
	bmpThumb.CreateBitmap( 64,64, 1, 32, NULL );
	CBitmap *oldBmp = dc.SelectObject( &bmpThumb );

	::SetStretchBltMode( image.GetDC(), HALFTONE ); image.ReleaseDC();
	image.StretchBlt( dc.GetSafeHdc( ), 0,0, 64,64 );
	
	if ( image.GetHeight() != 64 || image.GetWidth() != 64 )
	{
		CImage image2;
		image2.Create( 64, 64, 32 );
		image.StretchBlt( image2.GetDC(), 0,0, 64,64 ); image2.ReleaseDC();
		DeleteFile( "thumbnail.jpg" );
		image2.Save( "thumbnail.jpg" );
	}
	else 
	{
		CopyFile( m_sThumbnailFile, "thumbnail.jpg", FALSE );
	}
	
	dc.SelectObject( oldBmp );
	m_ImageThumbnail.CopyBitmap( &bmpThumb );
}

void CUploadGame::OnBrowseGameFolder()
{
}

void CUploadGame::OnUploadClicked()
{
	if ( m_UploadThread.IsUploading( ) )
	{
		int hResult = MessageBox( "Are you sure you want to cancel your upload?", "Confirm", MB_YESNO );
		if ( hResult == IDYES ) m_UploadThread.Cancel();
		return;
	}

	m_editStatusMessages.SetWindowText( "" );

	CString sGameFolder;
	CString sThumbnail;
	CString sDescription;

	m_editGameFolder.GetWindowText( sGameFolder );
	m_editDescription.GetWindowText( sDescription );

	if ( sGameFolder.GetLength() == 0 ) { MessageBox( "You must enter a game folder to upload.", "Error" ); return; }
	if ( sDescription.GetLength() == 0 ) { MessageBox( "You must enter a description of your game.", "Error" ); return; }

	//check thumbnail has been converted to 64x64
	CImage image;
	HRESULT hr = image.Load( m_sThumbnailFile );
	if ( FAILED(hr) ) { MessageBox( "Failed to load thumbnail" ); return; }

	if ( image.GetHeight() != 64 || image.GetWidth() != 64 )
	{
		::SetStretchBltMode( image.GetDC(), HALFTONE );
		image.ReleaseDC();
		
		CImage image2;
		image2.Create( 64, 64, 32 );
		image.StretchBlt( image2.GetDC(), 0,0, 64,64 ); image2.ReleaseDC();
		DeleteFile( "thumbnail.jpg" );
		image2.Save( "thumbnail.jpg" );
	}
	else
	{
		CopyFile( m_sThumbnailFile, "thumbnail.jpg", FALSE );
	}

	//login
	if ( theApp.m_sUsername.GetLength() == 0 || theApp.m_sPassword.GetLength() == 0 ) 
	{
		CLoginDialog2 cLoginBox;
		cLoginBox.DoModal();
		if ( !cLoginBox.Completed() ) return;
		UpdateClubsList();
	}

	//m_editStatusMessages.SetWindowText( "Logging In..." );

	HTTPComm server;
	while ( !server.Login2( theApp.m_sUsername, theApp.m_sPassword ) ) 
	{
		CLoginDialog2 cLoginBox;
		cLoginBox.DoModal();
		if ( !cLoginBox.Completed() ) return;
	}

	m_ProgressSpinner.StartSpinning( );
	m_ProgressBar.SetPos( 0 );	

	m_UploadThread.SetFields( this, &m_ProgressBar, sGameFolder, "thumbnail.jpg", sDescription, iChosenClub );
	m_UploadThread.Start();

	m_editStatusMessages.SetWindowText( "Starting Process..." );

	m_btnUpload.SetWindowText( "Cancel" );

	SetTimer( 1, 500, NULL );
	SetTimer( 2, 600000, NULL ); //10 minutes
}

void CUploadGame::SetMessage( LPCSTR szMsg )
{
	m_bUpdateMessage = true;
	m_sNewMessage = szMsg;
}

void CUploadGame::OnTimer( UINT_PTR nIDEvent )
{
	if ( nIDEvent == 1 )
	{
		if ( !m_UploadThread.IsUploading( ) )
		{
			KillTimer( 1 );
			KillTimer( 2 );
			m_ProgressSpinner.StopSpinning();
			m_btnUpload.SetWindowText( "Upload" );
			if ( m_UploadThread.IsSuccessful() ) m_editStatusMessages.SetWindowText( "Upload Successful." );
			else m_editStatusMessages.SetWindowText( m_UploadThread.GetError() );
			this->FlashWindow( TRUE );
			if ( !m_UploadThread.Cancelled() ) UpdateSpaceUsed();
		}
		else
		{
			if ( m_bUpdateMessage )
			{
				m_editStatusMessages.SetWindowText( m_sNewMessage );
				m_bUpdateMessage = false;
			}
		}
	}

	if ( nIDEvent == 2 )
	{
		//refresh server session data
		HTTPComm server;
		server.RefreshSession();
	}

	if ( nIDEvent == 3 )
	{
		KillTimer( 3 );
		UpdateClubsList();
		UpdateSpaceUsed();
	}
}
void CUploadGame::OnChangeLoginClicked()
{
	CLoginDialog2 cLoginBox;
	cLoginBox.DoModal();

	UpdateClubsList();
}

void CUploadGame::OnBnClickedCancel()
{
	if ( m_UploadThread.IsUploading( ) )
	{
		int hResult = MessageBox( "Are you sure you want to cancel your upload?", "Confirm", MB_YESNO );
		if ( hResult = IDYES ) 
		{
			m_UploadThread.Cancel();
			OnCancel();
		}
	}
	else OnCancel();
}

void CUploadGame::OnChooseClub()
{
	CString sSel;
	m_comboUploadTo.GetLBText( m_comboUploadTo.GetCurSel(), sSel );
	iChosenClub = FindClubID( sSel.GetString() );

	UpdateSpaceUsed();
}

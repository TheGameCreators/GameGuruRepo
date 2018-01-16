//
// cFront.cpp : implementation file
//

// Includes
//#define CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "editor.h"
#include "cFront.h"
#include "cdialogfront.h"

// From serial code check code
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Debug defines
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// externals
extern bool g_bFreeVersion;
extern int g_iUpgradeOffer;
extern int g_iDaysHavePassed;
extern CEditorApp theApp;
extern bool g_bNetBookVersion;
extern LPSTR g_lpNetBookError;

/////////////////////////////////////////////////////////////////////////////
// cFront dialog

cFront::cFront(CWnd* pParent /*=NULL*/)
	: CDialog(cFront::IDD, pParent)
{
	//{{AFX_DATA_INIT(cFront)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pMessageData = NULL;
	m_dwTime = 0;
	m_bSerialCodeCheck = true;
}

void cFront::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cFront)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	DDX_Control ( pDX, IDC_STATIC_1, m_EditTop );
	DDX_Control ( pDX, IDC_STATIC_2, m_EditMiddle );
	DDX_Control ( pDX, IDC_SERIALCODE, m_SerialCode );
	DDX_Control ( pDX, IDC_OKCODE, m_OK );
	DDX_Control ( pDX, IDC_QUITCODE, m_Quit );
}

BEGIN_MESSAGE_MAP(cFront, CDialog)
	//{{AFX_MSG_MAP(cFront)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OKCODE, OnStnClickedOK)
	ON_BN_CLICKED(IDC_QUITCODE, OnStnClickedQUIT)
END_MESSAGE_MAP()

// cFront message handlers

int cFront::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Start message handler for initial splash dialog
	m_Message.Start ( _T ( "FPSSPLASH" ), 1024 * 2, 1, this );

	// success
	return 0;
}

void cFront::HandleMessages ( void )
{
	// control when dialog closes
	bool bTerminateModal = false;

	// check if ptrs valid
	cIPC* pIPC = m_Message.GetIPC ( );
	if ( !pIPC ) return;

	// skip if mapeditor not waiting for interface
	if ( !pIPC->SkipIfEventNotWaiting () )
		return;

	// ready is set to 1 in filemap
	DWORD dwReady = 1;
	pIPC->SendBuffer ( &dwReady, 0, sizeof ( dwReady ) );

	// declare ansi and unicode strings
	char    szTextANSI [ 255 ] = "";
	CString szTextUNI;

	// get title value from filemap
	DWORD dwTitle = 0;
	pIPC->ReceiveBuffer ( &dwTitle, 4, sizeof ( dwTitle ) );
	if ( dwTitle==1 )
	{
		// SHOW PROGRESS OF LOADING
		pIPC->ReceiveBuffer ( &szTextANSI, 1000, sizeof ( szTextANSI ) );
		szTextUNI = szTextANSI;
		if ( m_bSerialCodeCheck==false ) 
			m_EditMiddle.SetWindowText ( szTextUNI );
	}
	if ( dwTitle==2 )
	{
		// FINALLY END DIALOG
		pIPC->ReceiveBuffer ( &szTextANSI, 1000, sizeof ( szTextANSI ) );
		szTextUNI = szTextANSI;
		if ( m_bSerialCodeCheck==false )
			m_EditMiddle.SetWindowText ( szTextUNI );

		// flag value in filemap [20] to zero
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 20, sizeof ( dwValue ) );

		// Sleep for a short while (no need now, version number shown at start of text sequence)
		//Sleep ( 1500 );
	}

	// retreive valie [8] from filemap (close flag)
	DWORD dwClose = 0;
	pIPC->ReceiveBuffer ( &dwClose, 8, sizeof ( dwClose ) );
	if ( dwClose )
	{
		// by default, trigger terminate flag if close returns a non-zero value
		bTerminateModal=true;
	}

	// only when update completely done, signal that event complete
	pIPC->AllowSyncEventNow();

	// termination of modal here, if flagged
	if ( bTerminateModal )
	{
		// For BETA, prompt disclaimer to user
		/*
		LPSTR g_pBETAMessage = "\
Welcome to FPSCR V1.01!\n\
\n\
NOTE: The new Multiplayer feature will NOT work for you in this build.\n\
Multiplayer requires you to have a Steam account and we're still finalising this part of Reloaded.\n\
\n\
Check the change log in the Help menu for details of what's new in this version.\n";
		MessageBox ( g_pBETAMessage, "FPS Creator Reloaded", MB_OK );
		*/

		// end initial dialog
		if ( m_bSerialCodeCheck==false )
		{
			// only if serial check complete and successful, otherwise wait for serial dialog to close
			// this from the function below..OnStnClickedOK
			EndDialog ( 1 );
		}
	}
}

BOOL cFront::OnInitDialog() 
{
	// default dialog inits
	CDialog::OnInitDialog();

	// check if serial code available and valid, if not use serial code check
	//m_bSerialCodeCheck = true;
	//if ( theApp.IsSerialCodeValid ( NULL ) ) m_bSerialCodeCheck = false;
	//will replace this with active token check using new Server Activation System
	m_bSerialCodeCheck = false;

	// get splash text for this dialog
	if ( m_bSerialCodeCheck==true )
	{
		TCHAR szString1 [ MAX_PATH ];
		TCHAR szString2 [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Splash" ), _T ( "Techsupport" ), _T ( "" ), szString1, MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Splash" ), _T ( "Enterserialcode" ), _T ( "" ), szString2, MAX_PATH, theApp.m_szLanguage );
		m_EditMiddle.SetWindowText ( szString1 );
		m_EditTop.SetWindowText ( szString2 );
	}
	else
	{
		// Version Name
		strcpy_s( theApp.m_szVersion, 64, "" );
		FILE *pFile = NULL;
		int error = _tfopen_s( &pFile, _T("version.ini"), _T("rb") );
		if ( !error )
		{
			fgets( theApp.m_szVersion, 64, pFile );
			fclose( pFile );
			pFile = NULL;
		}
		CString szTextUNI;
		szTextUNI = "Game Guru ";
		szTextUNI += theApp.m_szVersion;
		if ( m_bSerialCodeCheck==false )
			m_EditMiddle.SetWindowText ( szTextUNI );
	}

	// button texts
	TCHAR szStringOK [ MAX_PATH ];
	TCHAR szStringQUIT [ MAX_PATH ];
	GetPrivateProfileString ( _T ( "Splash" ), _T ( "SerialCodeOK" ), _T ( "" ), szStringOK, MAX_PATH, theApp.m_szLanguage );
	GetPrivateProfileString ( _T ( "Splash" ), _T ( "SerialCodeQUIT" ), _T ( "" ), szStringQUIT, MAX_PATH, theApp.m_szLanguage );
	m_OK.SetWindowText ( szStringOK );
	m_Quit.SetWindowText ( szStringQUIT );
	
	// set correct artwork folder and load the dialog image
	SetCurrentDirectory ( theApp.m_szDirectory );
	SetCurrentDirectory ( theApp.m_szLanguagePath );
	SetCurrentDirectory ( "artwork" );
	m_hDCSRC = CreateCompatibleDC ( NULL ); 

	// changing image nice idea but would require new media (art and more files to logo version)
	LPSTR pInitialImage = "splsh16.bmp";
	//if ( m_bSerialCodeCheck==true ) pInitialImage = "SerialCodeCheck.bmp";
	m_hBMP = LoadImage ( NULL, pInitialImage, IMAGE_BITMAP, 512, 256, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );

	// restore current working folder
	SetCurrentDirectory ( theApp.m_szDirectory );

	// Hide Serial Code controls initially
	if ( m_bSerialCodeCheck==false )
	{
		m_EditTop.ShowWindow ( SW_HIDE );
		m_SerialCode.ShowWindow ( SW_HIDE );
		m_Quit.ShowWindow ( SW_HIDE );
		m_OK.ShowWindow ( SW_HIDE );
	}

	// V117 - 280110 - detect if NETBOOK failure, if so, only offer to quit
	if ( g_bNetBookVersion==true && m_bSerialCodeCheck==true && g_lpNetBookError )
	{
		m_SerialCode.ShowWindow ( SW_HIDE );
		m_OK.ShowWindow ( SW_HIDE );
		m_EditTop.ShowWindow ( SW_SHOW );
		char lpFrontMessage[512];
		wsprintf ( lpFrontMessage, "Application has failed to authenticate (%s), try re-installing this application to correct the problem and ensure you are connected to the internet.", g_lpNetBookError );
		m_EditTop.SetWindowText ( lpFrontMessage );
		m_Quit.ShowWindow ( SW_SHOW );
		m_Quit.SetWindowText ( "QUIT" );
		m_Quit.SetWindowPos ( NULL, 230, 308, 0, 0, SWP_NOSIZE );
		if ( g_lpNetBookError ) { delete g_lpNetBookError; g_lpNetBookError=NULL; }
	}

	// FPGC also offers quit
	#if FPGCSERIAL
		m_Quit.SetWindowText ( "QUIT" );
	#endif

	// successful init
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void cFront::OnPaint() 
{
	// device context for painting
	CPaintDC dc(this);

	// paint image to the dialog
	m_hOld = SelectObject ( m_hDCSRC, m_hBMP );
	RECT rect1 = {3, 2, 512, 256};
	RECT rect2;
	GetClientRect ( &rect2 );
	if ( rect2.right > 530 )
	{
		rect2.left = (rect2.right - 512)/2;
		rect2.top = (rect2.bottom - 256)/2;
		rect1.left += rect2.left;
		rect1.top += rect2.top / 2;
	}
	StretchBlt ( dc, rect1.left, rect1.top, rect1.right, rect1.bottom, m_hDCSRC, 0, 0, 512, 256, SRCCOPY );
	SelectObject ( m_hDCSRC, m_hOld );

	// Do not call CDialog::OnPaint() for painting messages
}

void cFront::OnStnClickedOK()
{
	/*
	// get serial code from edit field
	CString szStringWithCodeIn;
	m_SerialCode.GetWindowText ( szStringWithCodeIn );

	// language strings
	TCHAR szString1 [ MAX_PATH ];
	TCHAR szString2 [ MAX_PATH ];
	TCHAR szString3 [ MAX_PATH ];
	GetPrivateProfileString ( _T ( "Splash" ), _T ( "NotValid" ), _T ( "" ), szString1, MAX_PATH, theApp.m_szLanguage );
	GetPrivateProfileString ( _T ( "Splash" ), _T ( "EnterCode" ), _T ( "" ), szString2, MAX_PATH, theApp.m_szLanguage );
	GetPrivateProfileString ( _T ( "Splash" ), _T ( "NotEnoughDigits" ), _T ( "" ), szString3, MAX_PATH, theApp.m_szLanguage );

	// primary errors
	char pError[512];
	strcpy ( pError, szString1 );
	if ( strlen(szStringWithCodeIn) == 0 ) strcpy ( pError, szString2 );
	else if ( strlen(szStringWithCodeIn) != 16 ) strcpy ( pError, szString3 );

	// check it against validator if correct number of digits
	LPSTR pTryString = szStringWithCodeIn.GetBuffer();
	if ( strlen(szStringWithCodeIn) == 16 && theApp.IsSerialCodeValid ( pTryString )==true )
	{
		// code is valid - switch off serial code check
		m_bSerialCodeCheck = false;
		EndDialog ( 1 );

		// FPGC - 230909 - switch FREE to FULL title text
		g_bFreeVersion = false;
		TCHAR szStringA [ MAX_PATH ];
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		GetPrivateProfileString ( _T("Application"), _T("Name"), _T(""), szStringA, MAX_PATH, theApp.m_szLanguageVariant );
		pFrame->SetTitle ( szStringA );
		pFrame->SetWindowText ( szStringA );
	}
	else
	{
		// prompt when serial code determined to be invalid
		TCHAR szString [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Splash" ), _T ( "InvalidTitle" ), _T ( "" ), szString, MAX_PATH, theApp.m_szLanguage );
		MessageBox ( pError, szString, MB_ICONEXCLAMATION | MB_OK );
	}
	*/
}

void cFront::OnStnClickedQUIT()
{
	//no serial code to proceed
	m_bSerialCodeCheck = false;

	// immediate exit of interface
	EndDialog ( 1 );

	/*
	// FPGC - 190909 - was QUIT, now switches to a FREE mode, except if NETBOOK quit
	if ( g_bNetBookVersion==true )
	{
		// delayed quit message
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		if ( pFrame ) pFrame->SetTimer( 99, 500, NULL );
//		TerminateProcess ( theApp.m_DBProApplication.hProcess, 0 );
//		PostQuitMessage ( 0 );
	}

	// FPGC also quits
	#ifdef FPGCSERIAL
	 // delayed quit message
	 CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
	 if ( pFrame ) pFrame->SetTimer( 99, 500, NULL );
	#endif
	*/
}

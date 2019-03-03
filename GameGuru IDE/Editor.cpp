//
// Editor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Editor.h"
#include "Splash.h"
#include "cFront.h"
#include "mmsystem.h"

#include "cLibraryWindow.h"
#include "cLibraryWindowNew.h"
#include "cBuildGame.h"
#include "cTestGame.h"
#include "cFPIWizard.h"
#include "cPopup.h"

#include "FileDialog.h"
#include <tlhelp32.h>
#include "enumprocess.h"
#include "cdialogend.h"

// 200807 - added to provide Vista only code
#include "VistaCheck.cpp"

// 280807 - added Help Wizard
#include "HelpWizard.h"
#include <afxinet.h>		//AutoUpdate
#include "UpdateCheckThread.h"
#include "res\messages.h"

#include <math.h>
#include "SerialCode\md5.h"
char* pKeyCodeX9 = "00V1";
int iKeyX9A = 4;
int iKeyX9B = 9;
int iKeyX9C = 0;
int iKeyX9D = 1;
char* pKeyCodeX10 = "0X10";
int iKeyX10A = 7;
int iKeyX10B = 2;
int iKeyX10C = 3;
int iKeyX10D = 9;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Externals
extern void DemoMessageBox ( void );
extern bool g_bEditingActive;
extern bool g_bAlsoRemoveRestrictedContentToggle;

// Global to control interface use
bool		g_bStopInterfaceOperations		= false;
bool		g_bWebsiteLink					= false;
bool		g_bFreeVersion					= true;
int			g_iUpgradeOffer					= 0;
int			g_iDaysHavePassed				= 0;
bool		g_bRemoteSessionNotAllowed		= false;

// Netbook Integration
//#include "adpcore.h"
bool		g_bNetBookVersion				= false;
LPSTR		g_lpNetBookError				= NULL;
bool		g_bNetBookVersionFirstTime		= true;
bool		g_bNetBookVersionFirstTimeRes	= false;

// X9, X10, X9(FPGC)
bool		g_bStandaloneNoNetMode			= false;
bool		g_bX9InterfaceMode				= true;
bool		g_bSantitizedTeenKidFriendly	= true;
bool		g_bDisablePrefabMode			= false;
bool		g_bDisableStoreMode				= false;

// Other globals
bool		g_bCenterMousePositionAndHide	= false;
bool		g_bInTestGame					= false;
HWND		g_hwndSteamUI					= NULL;
HWND		g_hwndRealDBPApp				= NULL;


/////////////////////////////////////////////////////////////////////////////
// CEditorApp

BEGIN_MESSAGE_MAP(CEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CEditorApp)
	ON_COMMAND(ID_HELP_INTERACTIVETUTORIAL, OnInteractiveTutorial)
	ON_COMMAND(ID_HELP_EDITORKEYBOARDSHORTCUTS, OnAppEditorKeyboardShortcuts)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_ABOUTTHISSOMETHING, OnAppAboutThisSomething)
	ON_COMMAND(ID_APP_EXIT, OnAppExit1)
	ON_COMMAND(ID_EDIT_COPY_1, OnEditCopy1)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_CUT_1, OnEditCut1)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTE_1, OnEditPaste1)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT_1, OnUpdateEditCut1)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY_1, OnUpdateEditCopy1)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_1, OnUpdateEditPaste1)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_FILE_BUILDGAME, OnUpdateBuildGame) // CRAPY, see MAINFRM.CPP :)
	ON_UPDATE_COMMAND_UI(ID_FILE_DOWNLOADSTOREITEMS, OnDownloadStoreItems)
	ON_UPDATE_COMMAND_UI(ID_FILE_IMPORTMODEL, OnImportModel)
	ON_UPDATE_COMMAND_UI(ID_FILE_CHARACTERCREATOR33003, OnCharacterCreator)
	
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_EDITORBASICSPART1, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_EDITORBASICSPART2, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_MAKINGAQUICKGAME, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_TESTLEVELMODE, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_TESTLEVEL3DEDITMODE, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_THEWIDGETTOOL, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_3DEDITMODE, OnTutorials)
	ON_UPDATE_COMMAND_UI(ID_TUTORIALS_HOSTINGJOININGMULTIPLAYERGAMES, OnTutorials)
//	ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEWFLAT, OnUpdateFileNewFlat)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_EX, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS_EX, OnUpdateFileSaveAsEx)
	ON_UPDATE_COMMAND_UI(ID_FILE_PREFERENCES, OnUpdateFilePreferences)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateFileMruFile1)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE2, OnUpdateFileMruFile2)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE3, OnUpdateFileMruFile3)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE4, OnUpdateFileMruFile4)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE5, OnUpdateFileMruFile5)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE6, OnUpdateFileMruFile6)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE7, OnUpdateFileMruFile7)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE8, OnUpdateFileMruFile8)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE9, OnUpdateFileMruFile9)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE10, OnUpdateFileMruFile10)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE11, OnUpdateFileMruFile11)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE12, OnUpdateFileMruFile12)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE13, OnUpdateFileMruFile13)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE14, OnUpdateFileMruFile14)
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE15, OnUpdateFileMruFile15)
	/*
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateViewOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEW, OnUpdateViewView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREFAB, OnUpdateViewPrefab)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAW, OnUpdateViewDraw)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAYPOINT, OnUpdateViewWaypoint)

	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEST_GAME, OnUpdateViewTestGame)
	*/
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_NEWFLAT, OnFileNewFlat)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	
	
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)

	

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

// CEditorApp construction

CString GetLanguageData ( CString group, CString key )
{
	SetCurrentDirectory ( theApp.m_szDirectory );
	char szText [ MAX_PATH ] = "";
	if ( stricmp ( group, "general" )==NULL
	||	 stricmp ( group, "debug" )==NULL 
	||	 stricmp ( group, "demo" )==NULL 
	||	 stricmp ( group, "application" )==NULL 
	||	 stricmp ( group, "help" )==NULL 
	||	 stricmp ( group, "about" )==NULL )
		GetPrivateProfileString ( group, key, _T ( "" ), szText, MAX_PATH, theApp.m_szLanguageVariant );
	else
		GetPrivateProfileString ( group, key, _T ( "" ), szText, MAX_PATH, theApp.m_szLanguage );

	CString string = szText;
	if ( string.GetLength() < 1 ) string = key;
	return string;
}


void ConvertWideToANSI ( BYTE* pData, CString* pUNI, char* pANSI )
{
	strcpy ( pANSI, *pUNI );

	if ( pData )
		memcpy ( pData, &pANSI [ 0 ], sizeof ( char ) * 255 );
}

void ConvertANSIToWide ( BYTE* pData, char* pANSI, CString* pUNI )
{
	memcpy ( &pANSI [ 0 ], pData, sizeof ( char ) * 255 );
	*pUNI = pANSI;
}

void ConvertANSIToWide ( char* pANSI, CString* pUNI )
{
	*pUNI = pANSI;
}

CEditorApp::CEditorApp() : CBCGWorkspace (TRUE /* m_bResourceSmartUpdate */)
{
	m_pHelpWizardDlg	= NULL;		//HELPW - initial value
	m_bHelpWizardVisible = true;

	m_pUpdateThread		= NULL;		//AutoUpdate

	m_bDisplayMouse		= TRUE;
	m_lastPoint.x		= 0;
	m_lastPoint.y		= 0;
	m_bEnd				= FALSE;
	m_bDisable			= true;
	m_bFocus			= false;
	m_bEntityFocus		= false;
	m_iRecover			= 0;
	m_dwRecoverTimer	= 0;
	m_bAppHasStartedRunning = false;
	m_bDoNotNeedToCheckForCrashesAnymore = false;
	//m_bGameOrBuildInProgress = false;
	m_bRestart			= false;
	m_bSplashScreen		= true;
	m_bBuildGameDisplay = false;
	m_bTestGameCancel	= false;

	m_sUsername = "";
	m_sPassword = "";

	memset ( m_bTest, 1, sizeof ( m_bTest ) );

	InitializeCriticalSectionAndSpinCount( &m_csDirectoryChanges, 500 );

	//get any saved username/password
	char szUsername [ 256 ] = "";
	if ( theApp.GetRegistryValue( "Software\\GameGuru\\Settings", "username", szUsername, 256 ) )
	{
		m_sUsername = szUsername;
	}
	
	char szPassword [ 256 ] = "";
	if ( theApp.GetRegistryValue( "Software\\GameGuru\\Settings", "password", szPassword, 256 ) )
	{
		char *szDecrypt1 = theApp.Decrypt( szPassword );
		char *szDecrypt2 = theApp.Decrypt( szDecrypt1 );

		m_sPassword = szDecrypt2;

		delete [] szDecrypt1;
		delete [] szDecrypt2;
	}
}

CEditorApp::~CEditorApp( )
{
	DeleteCriticalSection( &m_csDirectoryChanges );

	//HELPW - clear any help dialog running
	if ( m_pHelpWizardDlg ) 
	{
		if ( m_bHelpWizardVisible ) 
		{
			m_pHelpWizardDlg->EndDialog( 0 );
			m_pHelpWizardDlg->Join();
		}
		m_pHelpWizardDlg->Terminate( );
		delete m_pHelpWizardDlg;
		m_pHelpWizardDlg = NULL;
	}
	//HELPW - end

	//AutoUpdate
	if ( m_pUpdateThread )
	{
		m_pUpdateThread->CancelDownload( );
		m_pUpdateThread->Join( );
		delete m_pUpdateThread;
		m_pUpdateThread = NULL;
	}

	m_sUsername = "";
	m_sPassword = "";
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEditorApp object

CEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CEditorApp initialization


BOOL CALLBACK EnumWindowsProcEx(HWND hwnd,LPARAM lParam );

BOOL CALLBACK EnumWindowsProcLee( HWND hWnd, LPARAM lParam )
{
	// see the class name
	char pClass [ 256 ];
	GetClassName ( hWnd, pClass, 256 );
	if ( strnicmp ( pClass, "Guru", 4 )==NULL )
	{
		// list any with FPSC prefix
		strcat ( (char*)lParam, pClass );
	}
	return TRUE;//continue
}

unsigned long process_break_id = 0x2214;

BOOL CALLBACK EnumWindowsProcFindSteam( HWND hWnd, LPARAM lParam )
{
    unsigned long process_id = 0;
    GetWindowThreadProcessId(hWnd, &process_id);
	char pClass [ 256 ];
	GetClassName ( hWnd, pClass, 256 );
	if ( process_id==process_break_id )
	{
		// record Steam HWND
		g_hwndSteamUI = hWnd;
	}
	return TRUE;//continue
}

void CEditorApp::SetRegistryValue( const char *szLocation, const char *szKey, const char *szValue )
{
	if ( !szLocation || strlen(szLocation) < 1 ) return;
	if ( !szKey || strlen(szKey) < 1 ) return;
	if ( !szValue || strlen(szValue) < 1 ) return;
	
	HKEY hKey = 0;
	
	try 
	{
		LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, szLocation, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegSetValueEx( hKey, szKey, 0, REG_SZ, (BYTE*)szValue, strlen(szValue)+1 );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );
	}
	catch ( int )
	{
		MessageBox( NULL, GetLanguageData( _T("TGC Store"), _T("SetRegFailed") ), m_szErrorString, 0 );
		RegCloseKey( hKey );
	}
}

void CEditorApp::SetRegistryValue( const char *szLocation, const char *szKey, DWORD dwValue )
{
	if ( !szLocation || strlen(szLocation) < 1 ) return;
	if ( !szKey || strlen(szKey) < 1 ) return;
	
	HKEY hKey = 0;
	
	try 
	{
		LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, szLocation, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegSetValueEx( hKey, szKey, 0, REG_DWORD, (BYTE*)&dwValue, 4 );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );
	}
	catch ( int )
	{
		MessageBox( NULL, GetLanguageData( _T("TGC Store"), _T("SetRegFailed") ), m_szErrorString, 0 );
		RegCloseKey( hKey );
	}
}

bool CEditorApp::GetRegistryValue( const char *szLocation, const char *szKey, char *szValue, DWORD dwSize, HKEY hRoot )
{
	if ( !szLocation || strlen(szLocation) < 1 ) return false;
	if ( !szKey || strlen(szKey) < 1 ) return false;
	if ( !szValue ) return false;

	HKEY hKey = 0;
	DWORD dwDataSize = dwSize;
	DWORD dwType = REG_SZ;

	try
	{
		LONG ls = RegOpenKeyEx( hRoot, szLocation, NULL, KEY_QUERY_VALUE, &hKey );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( hKey, szKey, NULL, &dwType, (BYTE*)szValue, &dwDataSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );

		return true;
	}
	catch (int)
	{
		RegCloseKey( hKey );

		return false;
	}
}

bool CEditorApp::GetRegistryValue( const char *szLocation, const char *szKey, DWORD *dwValue )
{
	if ( !szLocation || strlen(szLocation) < 1 ) return false;
	if ( !szKey || strlen(szKey) < 1 ) return false;
	if ( !dwValue ) return false;

	HKEY hKey = 0;
	DWORD dwDataSize = 4;
	DWORD dwType = REG_DWORD;

	try
	{
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, szLocation, NULL, KEY_QUERY_VALUE, &hKey );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( hKey, szKey, NULL, &dwType, (BYTE*)dwValue, &dwDataSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );

		return true;
	}
	catch (int)
	{
		RegCloseKey( hKey );

		return false;
	}
}

bool CEditorApp::DeleteRegistryValue( const char *szLocation, const char *szKey )
{
	if ( !szLocation || strlen(szLocation) < 1 ) return false;
	if ( !szKey || strlen(szKey) < 1 ) return false;

	HKEY hKey = 0;

	try 
	{
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, szLocation, 0, KEY_SET_VALUE, &hKey );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegDeleteValue( hKey, szKey );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );
		return true;
	}
	catch ( int )
	{
		//MessageBox( NULL, GetLanguageData( _T("TGC Store"), _T("DelRegFailed") ), theApp.m_szErrorString, 0 );
		RegCloseKey( hKey );
		return false;
	}
}

char* CEditorApp::Encrypt( const char *szString )
{
	const unsigned char *szPtr = (const unsigned char*) szString;

	DWORD dwLength = strlen( szString );
	DWORD dwNewLength = dwLength * 2 + 1;
	char *szNewString = new char [ dwNewLength + 1 ];

	int iCypher [ 10 ];
	iCypher [ 0 ] = -24;
	iCypher [ 1 ] = 144;
	iCypher [ 2 ] = 108;
	iCypher [ 3 ] = -62;
	iCypher [ 4 ] = 88;
	iCypher [ 5 ] = -194;
	iCypher [ 6 ] = -12;
	iCypher [ 7 ] = 111;
	iCypher [ 8 ] = -47;
	iCypher [ 9 ] = 65;
	
	for ( DWORD i = 0; i < dwNewLength; i++ )
	{
		if ( i % 2 == 0 )
		{
			//even
			int value = 0;
			DWORD index = i/2;
			if ( index > 0 ) value += szPtr [ index - 1 ];
			if ( index < dwLength ) value += szPtr [ index ];

			value *= 11;
			value += iCypher [ index % 10 ];
			if ( value < 0 ) value *= -1;
			value %= 224;
			value += 32;

			szNewString [ i ] = value;
		}
		else
		{
			//odd
			DWORD index = (i - 1) / 2;
			
			int value = 0;
			value = szPtr [ index ];
			value -= 32;
			value += iCypher [ index % 10 ];
			value %= 224;
			if ( value < 0 ) value += 224;
			value += 32;

			szNewString [ i ] = value;
		}
	}

	szNewString [ dwNewLength ] = '\0';

	return szNewString;
}

char* CEditorApp::Decrypt( const char *szString )
{
	const unsigned char *szPtr = (const unsigned char*) szString;

	int iCypher [ 10 ];
	iCypher [ 0 ] = -24;
	iCypher [ 1 ] = 144;
	iCypher [ 2 ] = 108;
	iCypher [ 3 ] = -62;
	iCypher [ 4 ] = 88;
	iCypher [ 5 ] = -194;
	iCypher [ 6 ] = -12;
	iCypher [ 7 ] = 111;
	iCypher [ 8 ] = -47;
	iCypher [ 9 ] = 65;

	DWORD dwLength = strlen( szString );
	DWORD dwNewLength = (dwLength-1) / 2;
	char *szNewString = new char [ dwNewLength + 1 ];
	
	for ( DWORD i = 0; i < dwNewLength; i++ )
	{
		DWORD index = (i*2) + 1;
		
		int value = 0;
		value = szPtr [ index ];
		value -= 32;
		value -= iCypher [ i % 10 ];
		if ( value < 0 ) value += 224;
		value = (value % 224);
		value += 32;

		szNewString [ i ] = value;
	}

	szNewString [ dwNewLength ] = '\0';

	return szNewString;
}

bool CEditorApp::GetDefaultBrowserPath( char *szValue, DWORD dwSize )
{
	if ( !szValue ) return false;

	HKEY hKey = 0;
	DWORD dwDataSize = dwSize;
	DWORD dwType = REG_SZ;

	try
	{
		LONG ls = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\Clients\\StartMenuInternet", NULL, KEY_QUERY_VALUE, &hKey );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( hKey, NULL, NULL, &dwType, (BYTE*)szValue, &dwDataSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );
	}
	catch (int)
	{
		RegCloseKey( hKey );

		return false;
	}

	hKey = 0;
	dwDataSize = dwSize;
	dwType = REG_SZ;
	CString sPath = "SOFTWARE\\Clients\\StartMenuInternet\\";
	sPath += szValue;
	sPath += "\\shell\\open\\command";

	try
	{
		LONG ls = RegOpenKeyEx( HKEY_LOCAL_MACHINE, sPath, NULL, KEY_QUERY_VALUE, &hKey );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( hKey, NULL, NULL, &dwType, (BYTE*)szValue, &dwDataSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( hKey );
	}
	catch (int)
	{
		RegCloseKey( hKey );

		return false;
	}

	return true;
}

void CEditorApp::OpenUploadGame( )
{
	if ( m_UploadForm.IsRunning() ) return;

	DWORD dwState = 0;
	BOOL bOnline = InternetGetConnectedState( &dwState, 0 );
	if ( !bOnline )
	{
		MessageBox( NULL, GetLanguageData( "TGC Store", "NoInternet" ), theApp.m_szErrorString, 0 );
		return;
	}

	m_UploadForm.Start( );
}

void CEditorApp::OpenDownloadStoreItems( )
{
	DWORD dwState = 0;
	BOOL bOnline = InternetGetConnectedState( &dwState, 0 );
	if ( !bOnline )
	{
		MessageBox( NULL, GetLanguageData( "TGC Store", "NoInternet" ), theApp.m_szErrorString, 0 );
		return;
	}
	m_DSIForm.DoModal();
}

void CEditorApp::OpenStore( bool bGotoAddons )
{
	// 140414 - seems to find IE no matter what default browser is set in OS
	char szBrowser [ MAX_PATH ];
	GetDefaultBrowserPath( szBrowser, MAX_PATH );

	//  Just call WWW Website
	//LPSTR pGoToLink = "http://tgcstore.net/en/";
	//ShellExecute ( NULL, "open", szBrowser, pGoToLink, "", SW_SHOWMAXIMIZED );

	// Just shell execute the address directly
	//ShellExecuteW( NULL, L"open", L"http://tgcstore.net/en/" , NULL, NULL, SW_SHOWMAXIMIZED );
	//ShellExecuteW( NULL, L"open", L"http://en.tgcstore.net/?steam=true" , NULL, NULL, SW_SHOWMAXIMIZED );
	ShellExecuteW( NULL, L"open", L"https://www.tgcstore.net/" , NULL, NULL, SW_SHOWMAXIMIZED );
	
	/*
	DWORD dwState = 0;
	BOOL bOnline = InternetGetConnectedState( &dwState, 0 );
	if ( !bOnline )
	{
		MessageBox( NULL, GetLanguageData( "TGC Store", "NoInternet" ), theApp.m_szErrorString, 0 );
		return;
	}

	CRect rectMain;
	theApp.GetMainWnd()->GetWindowRect( rectMain );
	
	HINSTANCE StoreHandle = LoadLibrary("GameCreatorStore.dll");
	if ( !StoreHandle ) 
	{
		MessageBox( NULL, "Failed to load Store DLL", "Error", 0 );
		return;
	}

	void (*SetDownloadMode)(int) = ( void (*)(int) ) GetProcAddress( StoreHandle, "?SetDownloadMode@@YAXH@Z" );
	void (*SetGotoAddons)(int) = ( void (*)(int) ) GetProcAddress( StoreHandle, "?SetGotoAddons@@YAXH@Z" );
	void (*SetFilters1)(int,int,int,int) = ( void (*)(int,int,int,int) ) GetProcAddress( StoreHandle, "?SetFilters1@@YAXHHHH@Z" );
	void (*SetFilters2)(int,int,int,int) = ( void (*)(int,int,int,int) ) GetProcAddress( StoreHandle, "?SetFilters2@@YAXHHHH@Z" );
	void (*SetFilters4)(int,int,int,int) = ( void (*)(int,int,int,int) ) GetProcAddress( StoreHandle, "?SetFilters4@@YAXHHHH@Z" );
	void (*OpenStore)(char*,int,int,int,int) = ( void (*)(char*,int,int,int,int) ) GetProcAddress( StoreHandle, "?OpenStore@@YAXPADHHHH@Z" );
	bool (*GetX9Serial)(char*,DWORD) = ( bool (*)(char*,DWORD) ) GetProcAddress( StoreHandle, "?GetX9Serial@@YA_NPADK@Z" );
	if ( !SetDownloadMode ) { MessageBox( NULL, "Invalid Function Pointer: SetDownloadMode", "Error", 0 ); return; }
	if ( !SetGotoAddons ) { MessageBox( NULL, "Invalid Function Pointer: SetGotoAddons", "Error", 0 ); return; }
	if ( !SetFilters1 )	{ MessageBox( NULL, "Invalid Function Pointer: SetFilters1", "Error", 0 ); return; }
	if ( !SetFilters2 )	{ MessageBox( NULL, "Invalid Function Pointer: SetFilters2", "Error", 0 ); return; }
	if ( !SetFilters4 )	{ MessageBox( NULL, "Invalid Function Pointer: SetFilters4", "Error", 0 ); return; }
	if ( !OpenStore ) { MessageBox( NULL, "Invalid Function Pointer: OpenStore", "Error", 0 ); return; }
	if ( !GetX9Serial ) { MessageBox( NULL, "Invalid Function Pointer: GetX9Serial", "Error", 0 ); return; }

	// 6=XNA now Reloaded
	SetDownloadMode( 6 );

	//sets if the store opens on the addons page
	SetGotoAddons( bGotoAddons ? 1 : 0 );

	//show x9, show x10, show purchased, show unpurchased
	SetFilters1( 0,0,1,1 );

	//show entities, show segments, show music, show addons
	SetFilters2( 1,0,1,0 );

	// set kid friendly flag (adult rated)
	//if ( g_bSantitizedTeenKidFriendly==true )
	//	SetFilters4( 0, 1, 1, 1 );
	//else
	//	SetFilters4( 1, 1, 1, 1 );
	SetFilters4( 0, 0, 0, 0 );

    //Download Folder, posX, posY, sizeX, sizeY
	//folder must exist, highly recommend an absolute path
	//window size defaults to 1100x800 centered if the resolution supports it
	//otherwise the store will be maximised.
	//OpenStore( theApp.m_szDirectory, -1,-1, -1,-1 );
	OpenStore( theApp.m_szDirectory, rectMain.left, rectMain.top, rectMain.Width(), rectMain.Height() );

	char szSerial [ 32 ];
	bool bSerial = GetX9Serial( szSerial, 32 );

	if ( bSerial )
	{
		char pUserFile [ 512 ];
		wsprintf ( pUserFile, "%s\\%s", theApp.m_szDirectory, "userdetails.ini" );
		char pUserSerialCode [ 512 ];
		wsprintf ( pUserSerialCode, "" );
		BOOL bSuccess = WritePrivateProfileString( _T("PERSONAL DATA"), _T("YourSerialCode"), szSerial, pUserFile );
		if ( !bSuccess )
		{
			char str [ 256 ];
			sprintf_s( str, 256, "%s: %s", GetLanguageData( "TGC Store", "WriteSerialFailed" ), szSerial );
			MessageBox( NULL, str, theApp.m_szDirectory, 0 );
		}

		// Support for BUILD GAME item (serial code read from USERDETAILS.INI (else free version))
		// CheckSerialCodeValidity();
	}

	// free store DLL
    FreeLibrary( StoreHandle );
	*/
}

bool CEditorApp::IsProcessRunning ( char* szProcess )
{
	/* for whatever reason this can return FALSE, even when app is resident
	CEnumProcess				tmp;
	CEnumProcess::CProcessEntry entry;
	bool						bLoop	 = false;
	bool						bProcess = false;
	
	tmp.SetMethod ( 0x1 );

	// run through all processes
	for ( bLoop = tmp.GetProcessFirst ( &entry ); bLoop; bLoop = tmp.GetProcessNext ( &entry ) )
	{
		// convert the string to lowercase
		_strlwr ( entry.lpFilename );

		// set up our strings
		CString		process				= entry.lpFilename;
		char		szTextANSI [ 255 ]	= "";
		char		seps[ ]				= ".\\//;";
		char*		token;

		// convert the process name from wide to ansi
		ConvertWideToANSI ( NULL, &process, szTextANSI );

		m_Debug.Write ( szTextANSI );

		// get the first token
		token = strtok ( szTextANSI, seps );

		// extract each token
		while ( token != NULL )
		{
			// see if the map editor is in the list
			if ( _strnicmp ( token, szProcess, strlen ( token ) ) == 0 )
			{
				bProcess = true;
			}

			// get the next token
			token = strtok ( NULL, seps );
		}

		if ( bProcess )
			break;
	}
	LPARAM lParam = 0;
	EnumWindows(EnumWindowsProcLee, lParam);
	*/

	// Simply look for class name of desired applications
	bool bProcess = false;
	HWND hWndFind = FindWindow ( szProcess, NULL );
	if ( hWndFind ) bProcess = true;

	return bProcess;
}

void CEditorApp::CheckForDBProApplications ( void )
{
	// check the running processes and determine if either of the
	// db pro applications are not running

	// only fo check once a second
	if ( timeGetTime()-m_dwCheckProcessesTimer > 1000 )
	{
		// check processes 
		m_dwCheckProcessesTimer = timeGetTime();

		// local variables	
		bool bMapEditor = false;	// state of map editor
		//m_bGameOrBuildInProgress = false;	// state of game
		
		// check to see if any of these processes are running
		bMapEditor = IsProcessRunning ( "Game Guru12345" );//"FPSC Map Editor12345" );
		if ( bMapEditor==false ) bMapEditor = IsProcessRunning ( "Guru-MapEditor" );
		//m_bGameOrBuildInProgress = IsProcessRunning ( "FPSC Game12345" );
		//if ( m_bGameOrBuildInProgress==false ) m_bGameOrBuildInProgress = IsProcessRunning ( "FPSC-Game" );

		// if neither are running then sleep for a while
		if ( m_bDoNotNeedToCheckForCrashesAnymore==false )
		{
			if ( bMapEditor==false && m_bAppHasStartedRunning==true )
			{
				if ( m_iRecover==0 )
				{
					m_dwRecoverTimer = timeGetTime();
					m_iRecover=1;
				}
				else
				{
					// if no map or game after X seconds, message..
					if ( timeGetTime() - m_dwRecoverTimer > 6000 ) // 300114 - was 10 seconds / 060614 - was 3 seconds
						m_iRecover=2;
				}
			}
			else
			{
				// one app must have started running before drop-out recover code can kick in
				m_bAppHasStartedRunning = true;
				m_iRecover=0;
			}
		}

		// we have waited long enough but something is wrong
		if ( m_iRecover==2 )
		{
			// trigger the map editor to relaunch (copied from UpdateRestart function)
			cIPC* pIPC	  = m_Message.GetIPC ( );
			char szEditorEXE [ 255 ] = "Guru-MapEditor.exe";
			pIPC->SendBuffer ( &szEditorEXE, 1000, sizeof ( szEditorEXE ) );
			char szEditorEXECMDLINE [ 255 ] = "-r";
			pIPC->SendBuffer ( &szEditorEXECMDLINE, 1256, sizeof ( szEditorEXECMDLINE ) );

			// restore IDE if in test game
			if ( g_bInTestGame==true )
			{
				RestoreIDEEditorView();
			}

			// restore mouse if it was hidden
			if ( g_bCenterMousePositionAndHide==true )
			{
				g_bCenterMousePositionAndHide = false;
				ShowCursor(TRUE);
			}

			// reset handle to DBP App
			g_hwndRealDBPApp = NULL;

			// close down all operations for the reset, then reset
			g_bStopInterfaceOperations = false;
			m_bAppHasStartedRunning = false;
			m_bRestart = true;
			m_Message.Stop ( );
			if ( m_pDocumentA ) m_pDocumentA->SetQuitMessage ( false );
			if ( pMainFrame ) SendMessage ( pMainFrame->m_hWnd, WM_CLOSE_LEVEL_WINDOW, 0, 0 );
			m_Message.Restart ( );
			m_iRecover = 0;
		}
	}
}

void CEditorApp::UpdatePopup ( void )
{
	// the popup dialog for displaying text information

	// local variables
	DWORD dwPopup = 0;						// popup
	DWORD dwValue = 0;						// reset value
	cIPC* pIPC	  = m_Message.GetIPC ( );	// message pointer

	// check message pointer is valid
	if ( !pIPC )
		return;

	// extract the popup value from offset 750
	pIPC->ReceiveBuffer ( &dwPopup, 750, sizeof ( dwPopup ) );

	// see if we need to display the popup
	if ( dwPopup )
	{
		// reset the popup display value
		pIPC->SendBuffer ( &dwValue, 750, sizeof ( dwValue ) );
		pIPC->AllowSyncEventNow();

		// now display the popup dialog (only if not already got one)
		//if ( g_popptr==NULL )
		//{
			// set the popup display value to true
			m_bPopupDisplay = true;
			cPopup pop;
			pop.DoModal();
			//g_popptr = new cPopup();//&pop;
		//	if ( g_popptr )
		//	{
		//		g_popptr->DoModal();//pop.DoModal ( );
		//		g_popptr = NULL;
		//	}
			// finally reset the display value
			m_bPopupDisplay = false;
		//}
	}
}

void CEditorApp::UpdateTermination ( void )
{
	// see if we need to terminate the db pro app ( the map editor )
	// and then launch the game executable

	// local variables
	cIPC*	pIPC							= m_Message.GetIPC ( );	// message pointer
	DWORD	dwTerminateDBProApplication		= 0;					// terminate flag
	DWORD	dwValue							= 0;					// reset value
	BYTE	byClearData [ 1024 * 6 ]		= { 0 };
	char	szExeANSI	[ 255 ]				= "";
	char	szParamANSI [ 255 ]				= "";
	char	dataA		[ 255 ]				= "";
	char	dataB		[ 255 ]				= "";
	char	szDebug		[ 255 ]				= "";
	CString szParamUNI						= "";
	CString szExeUNI						= "";

	// check message pointer
	if ( !pIPC )
		return;

	// extract the terminate value from the message data
	pIPC->ReceiveBuffer ( &dwTerminateDBProApplication, 920, sizeof ( dwTerminateDBProApplication ) );

	// now see if we need to terminate the db pro app
	if ( dwTerminateDBProApplication )
	{
		// stop the messages from processing while we're going through
		// this section of code
		m_Message.Stop ( );

		// reset this message to 0
		pIPC->SendBuffer ( &dwValue, 920, sizeof ( dwValue ) );

		// set the focus flag to true
 		m_bFocus = true;

		// output debug information
		theApp.m_Debug.Write ( "\nCALLING GAME" );

		// terminate the db pro app using the handle
		if ( TerminateProcess ( m_DBProApplication.hProcess, 0 ) )
		{
			theApp.m_Debug.Write ( "\nPROCESS TERMINATED" );
		}
		else
		{
			theApp.m_Debug.Write ( "\nFAILED TO TERMINATE PROCESS" );
		}

		// turn off the quit message
		m_pDocumentA->SetQuitMessage ( false );

		// set the current directory of the app
		EnterCriticalSection( &theApp.m_csDirectoryChanges );
		SetCurrentDirectory ( theApp.m_szDirectory );
		LeaveCriticalSection( &theApp.m_csDirectoryChanges );

		// wait till the map editor has completely shut down before
		// attempting to launch the game, this code has helped to resolve
		// errors where the test game would fail
		while ( 1 )
		{
			bool bMapEditor = false;

			bMapEditor = IsProcessRunning ( "Game Guru12345" );//"FPSC Map Editor12345" );

			if ( !bMapEditor )
				break;
		}

		theApp.m_Debug.Write ( "*** GURU HAS ENDED" );
		theApp.m_Debug.Write ( "*** NOW GOING TO CALL GAMEGURU" );

		// get the name of the exe we need to run and it's parameters
		pIPC->ReceiveBuffer ( &dataA [ 0 ], 1000, sizeof ( dataA ) );
		pIPC->ReceiveBuffer ( &dataB [ 0 ], 1256, sizeof ( dataB ) );

		// convert into wide character set
		ConvertANSIToWide ( ( BYTE* ) &dataA [ 0 ], szExeANSI,   &szExeUNI );
		ConvertANSIToWide ( ( BYTE* ) &dataB [ 0 ], szParamANSI, &szParamUNI );

		// print name of executable
		sprintf ( szDebug, "Shell execute - executable - %s", szExeANSI   );
		theApp.m_Debug.Write ( szDebug );

		// print parameters
		sprintf ( szDebug, "Shell execute - parameters - %s", szParamANSI );
		theApp.m_Debug.Write ( szDebug );
		
		// reset recover counter
		theApp.m_iRecover = 0;
		theApp.m_dwRecoverTimer = 0;
		//theApp.m_bGameOrBuildInProgress = false;

		// clear the message buffer
		pIPC->SendBuffer ( &byClearData [ 0 ], 0, sizeof ( byClearData ) );

		// reset file map states
		theApp.SetFileMapData (  44, 1 );
		theApp.SetFileMapData ( 486, 1 );

		// 050315 - 015 - changed to pass in the working directory as when steam has done an update for some people the working directory is wrong
		CString szDirUNI;
		ConvertANSIToWide ( theApp.m_szDirectory, &szDirUNI );
		
		// now attempt to launch the executable
		theApp.m_hInstance = ShellExecute ( NULL, _T ( "open" ), szExeUNI, szParamUNI, szDirUNI, SW_SHOWNORMAL );

		// check everything went ok
		if ( ( int ) theApp.m_hInstance <= 32 )
		{
			theApp.m_Debug.Write ( "Shell execute - failed" );
		}
		else
		{
			theApp.m_Debug.Write ( "Shell execute - success" );
		}
		
		// reset member variables
		m_bAppRunning = false;
		m_bInvalidate = true;

		// restart the message checker
		m_Message.Restart ( );
	}
}

void CEditorApp::UpdateQuit	( void )
{
	// see if we need to quit

	// local variables
	cIPC* pIPC	  = theApp.m_Message.GetIPC ( );	// get the message pointer
	DWORD dwCheck = 0;								// check value
	DWORD dwValue = 0;								// reset value

	// return if the pointer is invalid
	if ( !pIPC )
		return;

	// get the value from the buffer
	pIPC->ReceiveBuffer ( &dwCheck, 912, sizeof ( dwCheck ) );

	// if this value is 1 we need to quit
	if ( dwCheck == 1 )
	{
		// output debug information
		m_Debug.Write ( "Message 912 - UpdateQuit" );

		// set the end flag to true so the app knows it
		// can quit in it's loops
		m_bEnd = true;

		// reset the quit value
		pIPC->SendBuffer ( &dwValue, 912, sizeof ( dwValue ) );
	}
}

void CEditorApp::UpdateRestart ( void )
{
	// see if we need to restart the editor
	bool bRepeatAndSleep = true;
	while ( bRepeatAndSleep )
	{
		// only do once by default
		bRepeatAndSleep = false;

		// local variables
		cIPC* pIPC = theApp.m_Message.GetIPC ( );		// get the message handler
		if ( !pIPC ) return;

		// get the value
		DWORD dwCheck	= 0;
		pIPC->ReceiveBuffer ( &dwCheck, 924, sizeof ( dwCheck ) );
		if ( dwCheck == 1 )
		{
			// Set event before possible reload of mapeditor
			pIPC->AllowSyncEventNow();

			// A small pause before launch restart - time for old app(s) to end
			Sleep(500);

			// TEST GAME Ends Here, so reactivate the stopinterface flag
			g_bStopInterfaceOperations = false;

			// Signal that no apps are supposed to be running, so no drop-out recover error!
			m_bAppHasStartedRunning = false;

			// output debug information
			m_Debug.Write ( "Message 924 - UpdateRestart" );

			// set the restart flag to true
			m_bRestart = true;

			// stop the messages
			m_Message.Stop ( );
		
			// turn off the quit message (so the NewDocument function can reload mapeditor exe)
			m_pDocumentA->SetQuitMessage ( false );
			
			// send a message to the main frame to quit
			if ( pMainFrame )
				SendMessage ( pMainFrame->m_hWnd, WM_CLOSE_LEVEL_WINDOW, 0, 0 );
			
			// reset the check value
			dwCheck = 0;
			pIPC->SendBuffer ( &dwCheck, 924, sizeof ( dwCheck ) );

			// finally restart the message pump
			m_Message.Restart ( );

			// Restore mouse if disabled (due to test game mode)
			if ( g_bCenterMousePositionAndHide==true )
			{
				g_bCenterMousePositionAndHide = false;
				ShowCursor(TRUE);
			}
		}

		// if playing test game, can sleep and stay in here to save CPU cycles from interface activity
		if ( g_bStopInterfaceOperations ) 
		{
			Sleep ( 500 );
			bRepeatAndSleep = true;
			CheckForDBProApplications ( );
		}
	}
}

void CEditorApp::UpdateEntityWindow ( void )
{
	// update the entity window

	// local variables
	cIPC* pIPC		= theApp.m_Message.GetIPC ( );	// get the message pointer
	DWORD dwCheck	= 0;							// check value

	// get the value from the offset
	pIPC->ReceiveBuffer ( &dwCheck, 978, sizeof ( dwCheck ) );

	// act when the value is true
	if ( dwCheck )
	{
		// send a message to the frame to display or hide the entity window
		if ( pMainFrame )
			SendMessage ( pMainFrame->m_hWnd, WM_ENTITY_WINDOW, dwCheck, 0 );
		
		// reset the check value to 0
		dwCheck = 0;

		// update the buffer with the new value
		pIPC->SendBuffer ( &dwCheck, 978, sizeof ( dwCheck ) );
	}
}

void CEditorApp::UpdateBuildGame ( void )
{
	// ipc ptr
	cIPC* pIPC = theApp.m_Message.GetIPC ( );
	if ( !pIPC ) return;

	DWORD dwCheck = 0;
	pIPC->ReceiveBuffer ( &dwCheck, 758, sizeof ( dwCheck ) );
	if ( dwCheck==2 )
	{
		// if build game selected, go modal!
		
		// reset trigger
		DWORD dwClear = 0;
		pIPC->SendBuffer ( &dwClear, 758, sizeof ( dwClear ) );

		// Launch BUILDGAME dialog
		theApp.m_bBuildGameDisplay = true;
		cBuildGame game;
		pIPC->AllowSyncEventNow();
		game.DoModal ( );
		theApp.m_bBuildGameDisplay = false;
	}
	if ( dwCheck==3 )
	{
		// 220114 - simply report success
		char str[MAX_PATH];
		TCHAR szString [ 3 ] [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Standalone" ), _T ( "Title" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Standalone" ), _T ( "Body1" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Standalone" ), _T ( "Body2" ), _T ( "" ), szString [ 2 ], MAX_PATH, theApp.m_szLanguage );
		wsprintf ( str, "%s '\\Documents\\Game Guru Files\\My Games' %s.", szString [ 1 ], szString [ 2 ] );
		MessageBox ( NULL, str, szString [ 0 ], MB_OK | MB_APPLMODAL | MB_TOPMOST | MB_SETFOREGROUND );

		
		// reset trigger
		DWORD dwClear = 0;
		pIPC->SendBuffer ( &dwClear, 758, sizeof ( dwClear ) );
	}
}

void CEditorApp::UpdatePreferences ( void )
{
	/* disabled for now (754 now used for stock level triggers)
	// ipc ptr
	cIPC* pIPC = theApp.m_Message.GetIPC ( );
	if ( !pIPC ) return;

	// if prefs selected
	DWORD dwCheck = 0;
	pIPC->ReceiveBuffer ( &dwCheck, 754, sizeof ( dwCheck ) );
	if ( dwCheck==2 )
	{
		// clear flag
		DWORD dwClear = 0;
		pIPC->SendBuffer ( &dwClear, 754, sizeof ( dwCheck ) );

		// launch prefs
		theApp.m_bBuildGameDisplay = true;
		cPreferences pref;
		pIPC->AllowSyncEventNow();
		pref.DoModal ( );
		theApp.m_bBuildGameDisplay = false;
		theApp.SetFileMapData ( 754, 0 );
	}
	*/
}

void CEditorApp::RestoreIDEEditorView ( void )
{
	// restore windows
	((CMainFrame*)theApp.GetMainWnd())->m_wndMenuBar.ShowWindow(SW_SHOW);
	((CMainFrame*)theApp.GetMainWnd())->m_wndToolBar.ShowWindow(SW_SHOW);
	//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarView.ShowWindow(SW_SHOW);
	//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarDraw.ShowWindow(SW_SHOW);
	//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarSegment.ShowWindow(SW_SHOW);
	//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarWaypoint.ShowWindow(SW_SHOW);
	//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarGame.ShowWindow(SW_SHOW);
	((CMainFrame*)theApp.GetMainWnd())->m_wndStatusBar.ShowWindow(SW_SHOW);
	((CMainFrame*)theApp.GetMainWnd())->m_wndOutput.ShowWindow(SW_SHOW);
	((CMainFrame*)theApp.GetMainWnd())->m_wndWorkSpace.ShowWindow(SW_HIDE);

	// Now get all sub-windows to find their place again
	((CMainFrame*)theApp.GetMainWnd())->RecalcLayout(TRUE);

	// Default docking of individual toolbars and windows
	CRect rectA;
	((CMainFrame*)theApp.GetMainWnd())->m_wndToolBar.GetWindowRect(&rectA);
	rectA.OffsetRect(1,0);
	//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarGame,		AFX_IDW_DOCKBAR_TOP,	&rectA);
	//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarWaypoint,	AFX_IDW_DOCKBAR_TOP,	&rectA);
	//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarSegment,	AFX_IDW_DOCKBAR_TOP,	&rectA );
	//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarDraw,		AFX_IDW_DOCKBAR_TOP,	&rectA );
	//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarView,     AFX_IDW_DOCKBAR_TOP,	&rectA );
	((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndOutput,			AFX_IDW_DOCKBAR_LEFT,	&rectA );
	((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndWorkSpace );

	// make it a real full screen window and resize it to fit
	SetWindowLong ( ((CMainFrame*)theApp.GetMainWnd())->m_hWnd, GWL_STYLE, theApp.dwRestoreGUIWindowMode );
	theApp.GetMainWnd()->SetWindowPos(NULL, 0, 0, 1280, 768-22, SWP_SHOWWINDOW );

	// Maximise to get status bar back too
	((CMainFrame*)theApp.GetMainWnd())->ShowWindow (SW_SHOWNORMAL);
	((CMainFrame*)theApp.GetMainWnd())->ShowWindow (SW_MAXIMIZE);
}

void CEditorApp::UpdateTestGame ( void )
{
	// get communication ptr to chat with mapeditor
	cIPC* pIPC = theApp.m_Message.GetIPC ( );
	if ( !pIPC ) return;

	// 970 = Allows mapeditor to trigger windows to be restored from fullscreen Test Level
	DWORD dwCheckEndTextGame = 0;
	pIPC->ReceiveBuffer ( &dwCheckEndTextGame, 970, sizeof ( dwCheckEndTextGame ) );
	if ( dwCheckEndTextGame==1 )
	{
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 970, sizeof ( dwValue ) );
		pIPC->AllowSyncEventNow();

		// restore windows
		RestoreIDEEditorView();
		((CMainFrame*)theApp.GetMainWnd())->m_wndMenuBar.ShowWindow(SW_SHOW);
		((CMainFrame*)theApp.GetMainWnd())->m_wndToolBar.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarView.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarDraw.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarSegment.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarWaypoint.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarGame.ShowWindow(SW_SHOW);
		//((CMainFrame*)theApp.GetMainWnd())->m_wndStatusBar.ShowWindow(SW_SHOW);
		((CMainFrame*)theApp.GetMainWnd())->m_wndOutput.ShowWindow(SW_SHOW);
		((CMainFrame*)theApp.GetMainWnd())->m_wndWorkSpace.ShowWindow(SW_HIDE);

		// Now get all sub-windows to find their place again
		((CMainFrame*)theApp.GetMainWnd())->RecalcLayout(TRUE);

		// Default docking of individual toolbars and windows
		CRect rectA;
		((CMainFrame*)theApp.GetMainWnd())->m_wndToolBar.GetWindowRect(&rectA);
		rectA.OffsetRect(1,0);
		//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarGame,		AFX_IDW_DOCKBAR_TOP,	&rectA);
		//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarWaypoint,	AFX_IDW_DOCKBAR_TOP,	&rectA);
		//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarSegment,	AFX_IDW_DOCKBAR_TOP,	&rectA );
		//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarDraw,		AFX_IDW_DOCKBAR_TOP,	&rectA );
		//((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndToolBarView,     AFX_IDW_DOCKBAR_TOP,	&rectA );
		((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndOutput,			AFX_IDW_DOCKBAR_LEFT,	&rectA );
		((CMainFrame*)theApp.GetMainWnd())->DockControlBar ( &((CMainFrame*)theApp.GetMainWnd())->m_wndWorkSpace );

		// make it a real full screen window and resize it to fit
		SetWindowLong ( ((CMainFrame*)theApp.GetMainWnd())->m_hWnd, GWL_STYLE, theApp.dwRestoreGUIWindowMode );
		//SetWindowLong ( ((CMainFrame*)theApp.GetMainWnd())->m_hWnd, GWL_EXSTYLE, 0 );
		theApp.GetMainWnd()->SetWindowPos(NULL, 0, 0, 1280, 768-22, SWP_SHOWWINDOW );

		// Maximise to get status bar back too
		((CMainFrame*)theApp.GetMainWnd())->ShowWindow (SW_SHOWNORMAL);
		((CMainFrame*)theApp.GetMainWnd())->ShowWindow (SW_MAXIMIZE);

		// exiting test game
		g_bInTestGame = false;
	}

	// 974 = Allows mapeditor to center mouse position for good mouselook functionality
	DWORD dwCenterMousePointer = 0;
	DWORD dwMouseX = 0, dwMouseY = 0;
	pIPC->ReceiveBuffer ( &dwCenterMousePointer, 974, sizeof ( dwCenterMousePointer ) );
	if ( dwCenterMousePointer==1 )
	{
		// HIDE
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 974, sizeof ( dwValue ) );
		if ( g_bCenterMousePositionAndHide==false )
		{
			g_bCenterMousePositionAndHide = true;
			ShowCursor(FALSE);
		}
	}
	if ( dwCenterMousePointer==2 )
	{
		// SHOW
		pIPC->ReceiveBuffer ( &dwMouseX, 982, sizeof ( dwMouseX ) );
		pIPC->ReceiveBuffer ( &dwMouseY, 986, sizeof ( dwMouseY ) );
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 974, sizeof ( dwValue ) );
		if ( g_bCenterMousePositionAndHide==true )
		{
			g_bCenterMousePositionAndHide = false;
			ShowCursor(TRUE);
			SetCursorPos ( dwMouseX, dwMouseY );
		}
	}
	if ( g_bCenterMousePositionAndHide==true )
	{
		SetCursorPos ( 320, 240 );
	}
}

void CEditorApp::UpdateFPIWizard ( void )
{
	cIPC* pIPC = theApp.m_Message.GetIPC ( );

	DWORD dwCheck = 0;
	pIPC->ReceiveBuffer ( &dwCheck, 990, sizeof ( dwCheck ) );
	if ( dwCheck )
	{
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 990, sizeof ( dwValue ) );
		
		theApp.m_bBuildGameDisplay = true;
		cFPIWizard	wizard;
		pIPC->AllowSyncEventNow();
		wizard.DoModal ( );
		theApp.m_bBuildGameDisplay = false;
	}
}

void CEditorApp::UpdateProject ( void )
{
	cIPC* pIPC = theApp.m_Message.GetIPC ( );

	DWORD dwCheck = 0;

	pIPC->ReceiveBuffer ( &dwCheck, 416, sizeof ( dwCheck ) );

	//memcpy ( &dwCheck, &m_pData [ 416 ], sizeof ( DWORD ) );

	if ( dwCheck == 1 )
	{
		::PostMessage (NULL, WM_MYMESSAGE, 0, 0);

		char szTitle [ 255 ] = "";

		pIPC->ReceiveBuffer ( &szTitle [ 0 ], 1000, sizeof ( szTitle ) );

		//memcpy ( &szTitle [ 0 ], &m_pData [ 1000 ], sizeof ( char ) * 255 );
		CString title = szTitle;

		m_szProjectName = title;

		SendMessage ( pMainFrame->m_hWnd, WM_MYMESSAGE, 0, 0 );

		DWORD dwValue = 0;

		pIPC->SendBuffer ( &dwValue, 416, sizeof ( dwValue ) );

		//memcpy ( &m_pData [ 416 ], &dwValue, sizeof ( DWORD ) );
	}
}

void CEditorApp::HandleMessages ( void )
{
	// replace this with findwindow (it might be better!)
	CheckForDBProApplications ( );
}


void CEditorApp::OnFileOpen ( )
{
	// 110105
	if ( theApp.m_bTestGameCancel )
		return;

	SetFileMapData ( 400, 1 );
	SetFileMapData ( 912, 0 );
}

void CEditorApp::OnFileNewType ( int iNewType )
{
	SetCurrentDirectory ( theApp.m_szDirectory );

	// check that we're ready to go for creating a new file
	if ( m_bFileNewValid )
	{
		CDocManager*    pDoc		= theApp.m_pDocManager;
		POSITION		pos			= pDoc->GetFirstDocTemplatePosition ( );
		CDocTemplate*	pTemplate	= pDoc->GetNextDocTemplate ( pos );

		SetFileMapData ( 408, iNewType );

		if ( !m_bSetupFrame )
		{
			pTemplate->OpenDocumentFile ( NULL );
				
			// update file mappingu
			SetFileMapData ( 200, 1 );
			SetFileMapData ( 204, 0 );

			// get a pointer to the main window
			TCHAR szString [ 4 ] [ MAX_PATH ];
			CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "A" ), _T ( "" ), szString [ 0 ], MAX_PATH, theApp.m_szLanguage );
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "B" ), _T ( "" ), szString [ 1 ], MAX_PATH, theApp.m_szLanguage );
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "C" ), _T ( "" ), szString [ 2 ], MAX_PATH, theApp.m_szLanguage );
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "D" ), _T ( "" ), szString [ 3 ], MAX_PATH, theApp.m_szLanguage );

			// update the buttons with the new text
			pFrame->m_wndMenuBar.SetButtonText ( 1, szString [ 0 ] );
			pFrame->m_wndMenuBar.SetButtonText ( 2, szString [ 1 ] );
			pFrame->m_wndMenuBar.SetButtonText ( 3, szString [ 2 ] );
			pFrame->m_wndMenuBar.SetButtonText ( 7, szString [ 3 ] );

			// update the menu bar
			pFrame->m_wndMenuBar.AdjustLayout ( );

			// flag setup frame var
			m_bSetupFrame = true;
		}
	}
}

void CEditorApp::OnFileNew ( )
{
	OnFileNewType ( 1 );
}

int g_iBoysLifeQuit = 0;

void CEditorApp::OnFileNewFlat ( )
{
	/*
	// demo version end splash
	#ifdef FPSC_DEMO
	cDialogEnd end;
	m_bDoNotNeedToCheckForCrashesAnymore = true;
	g_iBoysLifeQuit = 1;
	end.DoModal ( );
	g_iBoysLifeQuit = 0;
	#endif

	// identical to FILENEW (mapeditor detect difference from menu ID returned from selection)
	// FREE VERSION
	//#ifdef FPSC_DEMO
	#ifdef FREEVERSION
		DemoMessageBox();
	#else
	#endif
	*/
	OnFileNewType ( 2 );
}

void CEditorApp::ClearFileMap ( void )
{
	cIPC* pIPC			= m_Message.GetIPC ( );
	BYTE  data [ 6144 ] = "0";

	pIPC->SendBuffer ( &data, 0, sizeof ( data ) );

	//pIPC->ReceiveBuffer ( &dwTitle, 4, sizeof ( dwTitle ) );


	/*
	HANDLE	hFileMap = NULL;	// handle to file map
	LPVOID	lpVoid   = NULL;	// pointer to data
	BYTE*	pData    = NULL;	// byte pointer to data

	CString string = "FPSEXCHANGE";

	// attempt to open the file map
	hFileMap = OpenFileMapping ( FILE_MAP_ALL_ACCESS, TRUE, string );

	// silent failure
	if ( !hFileMap )
		return;

	// get the data contained within the file map
	lpVoid = MapViewOfFile ( hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0 );

	//lpVoid = theApp.m_pfnGetFileMapData ( "FPSEXCHANGE" );

	// fail if invalid data
	if ( !lpVoid )
		return;

	// cast void pointer to byte pointer
	pData = ( BYTE* ) lpVoid;

	memset ( &pData [ 0 ], 0, 1024 );

	// now unmap the file and close the handle
	UnmapViewOfFile ( lpVoid );
	CloseHandle     ( hFileMap );
	*/
}

DWORD CEditorApp::GetFileMapData ( int iOffset )
{
	cIPC* pIPC	  = m_Message.GetIPC ( );
	DWORD dwValue = 0;
	pIPC->ReceiveBuffer ( &dwValue, iOffset, sizeof ( dwValue ) );
	return dwValue;
}

void CEditorApp::GetFileMapDataString ( int iOffset, LPSTR string )
{
	cIPC* pIPC	  = m_Message.GetIPC ( );
	pIPC->ReceiveBuffer ( string, iOffset, 256 );
}


void CEditorApp::SetFileMapData ( int iOffset, DWORD dwData )
{
	cIPC* pIPC	  = m_Message.GetIPC ( );
	pIPC->SendBuffer ( &dwData, iOffset, sizeof ( dwData ) );
}

void CEditorApp::SetFileMapDataString ( int iOffset, LPTSTR pString )
{
	cIPC* pIPC	  = m_Message.GetIPC ( );

	char szDestination [ 255 ] = "";
	
	pIPC->SendBuffer ( &szDestination, iOffset, sizeof ( szDestination ) );
		
	strcpy ( szDestination, pString );
		
	pIPC->SendBuffer ( &szDestination, iOffset, sizeof ( szDestination ) );

	/*
	// set the file map data at the specified offset with the given data value

	// local variables
	HANDLE	hFileMap = NULL;	// handle to file map	
	LPVOID	lpVoid   = NULL;	// pointer to data
	BYTE*	pData    = NULL;	// byte pointer to data

	CString string = "FPSEXCHANGE";

	// attempt to open the file map
	hFileMap = OpenFileMapping ( FILE_MAP_ALL_ACCESS, TRUE, string );
	
	// silent failure
	if ( !hFileMap )
		return;

	// get the data contained within the file map
	lpVoid = MapViewOfFile ( hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0 );

	//lpVoid = theApp.m_pfnGetFileMapData ( "FPSEXCHANGE" );

	// fail if invalid data
	if ( !lpVoid )
		return;

	// cast void pointer to byte pointer
	pData = ( BYTE* ) lpVoid;

	

	{
		char szDestination [ MAX_PATH ];
		//int  iLen     = wcslen ( pString );
		//int  nResult = WideCharToMultiByte ( CP_ACP, 0, pString, -1, szDestination, iLen * sizeof ( CHAR ), NULL, NULL );

		strcpy ( szDestination, pString );
		
		//szDestination [ iLen ] = 0;
	
		memset ( &pData [ iOffset ], 0, sizeof ( char ) * 255 );

		memcpy ( &pData [ iOffset ], szDestination, sizeof ( char ) * strlen ( szDestination ) );
		
		int c = 0;
	}

	// now unmap the file and close the handle
	UnmapViewOfFile ( lpVoid );
	CloseHandle     ( hFileMap );
	*/
}

//Dave - removed mouse change and using mouse is shown
//bool g_bMouseChange = false;
bool g_bMouseIsShown = true;

void CEditorApp::UpdateMouse ( void )
{
	// get mouse visibility flag
	cIPC* pIPC	  = m_Message.GetIPC ( );

	DWORD dwValue = 0;
	pIPC->ReceiveBuffer ( &dwValue, 44, sizeof ( dwValue ) );

	if ( dwValue == 1 )
		m_bDisplayMouse = TRUE;
	else
		m_bDisplayMouse = FALSE;

	DWORD dwControlMouse = 0;
	DWORD dwNewX		 = 0;
	DWORD dwNewY		 = 0;

	pIPC->ReceiveBuffer ( &dwControlMouse, 48, sizeof ( dwControlMouse ) );
	pIPC->ReceiveBuffer ( &dwNewX,         52, sizeof ( dwNewX         ) );
	pIPC->ReceiveBuffer ( &dwNewY,         56, sizeof ( dwNewY         ) );

	DWORD dwWidth  = 0;
	DWORD dwHeight = 0;

	pIPC->ReceiveBuffer ( &dwWidth,  704, sizeof ( dwWidth  ) );
	pIPC->ReceiveBuffer ( &dwHeight, 708, sizeof ( dwHeight ) );

	if ( dwControlMouse == 1 )
	{
		RECT rect;
		
		m_pEditorView->GetClientRect  ( &rect );
		m_pEditorView->ClientToScreen ( &rect );

		POINT point;
		GetCursorPos ( &point );

		m_lastPoint.x = rect.left + ( dwWidth / 2 );
		m_lastPoint.y = rect.top + ( dwHeight / 2 );

		POINT newPoint;

		newPoint.x = point.x - m_lastPoint.x;
		newPoint.y = point.y - m_lastPoint.y;
		
		theApp.SetFileMapData (  8, newPoint.x );
		theApp.SetFileMapData ( 12, newPoint.y );
		
		SetCursorPos ( rect.left + ( dwWidth / 2 ), rect.top + ( dwHeight / 2 ) );

		// 120105
		if ( g_bMouseIsShown == true )
		{
			while(ShowCursor(false)>=0);
			g_bMouseIsShown = false;
			//g_bMouseChange = true;
		}

		m_bDisplayMouse = FALSE;
		m_bDisplayMouseInQuickPreview = true;
	}
	else if ( dwControlMouse == 2 )
	{
		// Dave for importer hiding mouse 7th Aug 2014
		if ( g_bMouseIsShown == true )
		{
			while(ShowCursor(false)>=0);
			g_bMouseIsShown = false;
			//g_bMouseChange = false;
		}

		m_bDisplayMouse = FALSE;
		m_bDisplayMouseInQuickPreview = true;
	}
	else
	{
		if ( g_bMouseIsShown == false )
		{
			while(ShowCursor(true)<0);
			//g_bMouseChange = false;
			g_bMouseIsShown = true;
		}
		m_bDisplayMouseInQuickPreview = FALSE;
	}

	//
	// Put key detection here as it gets called ALL THE TIME
	//

	// detect which window is foreground
	HWND hForeWnd = GetForegroundWindow();
	if ( this->pMainFrame ) 
	{
		if ( this->pMainFrame->m_hWnd==hForeWnd ) 
		{
			// foreground is IDE/test game
		}
		else
		{
			hForeWnd = NULL;
		}
	}

	// flag to indicate if we have input focus for IDE/test game
	if ( hForeWnd!=NULL )
		theApp.SetFileMapData ( 148, 1 );
	else
		theApp.SetFileMapData ( 148, 0 );

	// hack
	BYTE* lpData = (BYTE*)theApp.m_Message.GetIPC()->GetPtr();
	DWORD dwViewIt = lpData[100];

	// has key been pressed
	bool bKeyDetected = false;
	if ( g_bEditingActive==false && hForeWnd!=NULL )
	{
		for ( int iVKCode=0; iVKCode<256; iVKCode++ )
		{
			if ( iVKCode==VK_LSHIFT
			|| iVKCode==VK_RSHIFT
			|| iVKCode==VK_LCONTROL
			|| iVKCode==VK_RCONTROL ) continue;
			// 091215 - F10 is reserved for IDE menu bar functionality
			if ( g_bInTestGame==false && iVKCode == 121 ) continue;
			// if key held down, flag it
			SHORT dwKeyValue = ::GetAsyncKeyState ( iVKCode );
			if ( dwKeyValue & 0x8000 )
			{
				theApp.SetFileMapData ( 100, iVKCode );
				bKeyDetected = true;
			}
		}

		// KEY UP
		#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

		if ( KEY_UP(VK_RETURN) )
			theApp.SetFileMapData ( 108, 0 );

		if ( KEY_UP (VK_SHIFT) )
			theApp.SetFileMapData ( 112, 0 );

		if ( KEY_UP (VK_CONTROL) )
			theApp.SetFileMapData ( 116, 0 );

		if ( KEY_UP (VK_UP) )
			theApp.SetFileMapData ( 120, 0 );
		if ( KEY_UP (87) )
			theApp.SetFileMapData ( 120, 0 );

		if ( KEY_UP (VK_DOWN) )
			theApp.SetFileMapData ( 124, 0 );
		if ( KEY_UP (83) )
			theApp.SetFileMapData ( 124, 0 );

		if ( KEY_UP (VK_LEFT) )
			theApp.SetFileMapData ( 128, 0 );
		if ( KEY_UP (65) )
			theApp.SetFileMapData ( 128, 0 );

		if ( KEY_UP (VK_RIGHT) )
			theApp.SetFileMapData ( 132, 0 );
		if ( KEY_UP (68) )
			theApp.SetFileMapData ( 132, 0 );

		if ( KEY_UP (VK_SPACE) )
			theApp.SetFileMapData ( 136, 0 );

		// KEY DOWN

		if ( ::GetAsyncKeyState (VK_RETURN) & 0x8000 )
			theApp.SetFileMapData ( 108, 1 );

		if ( ::GetAsyncKeyState (VK_SHIFT) & 0x8000 )
			theApp.SetFileMapData ( 112, 1 );

		if ( ::GetAsyncKeyState (VK_CONTROL) & 0x8000 )
			theApp.SetFileMapData ( 116, 1 );

		if ( ::GetAsyncKeyState (VK_UP) & 0x8000 )
			theApp.SetFileMapData ( 120, 1 );
		if ( ::GetAsyncKeyState (87) & 0x8000 )
			theApp.SetFileMapData ( 120, 1 );

		if ( ::GetAsyncKeyState (VK_DOWN) & 0x8000 )
			theApp.SetFileMapData ( 124, 1 );
		if ( ::GetAsyncKeyState (83) & 0x8000 )
			theApp.SetFileMapData ( 124, 1 );

		if ( ::GetAsyncKeyState (VK_LEFT) & 0x8000 )
			theApp.SetFileMapData ( 128, 1 );
		if ( ::GetAsyncKeyState (65) & 0x8000 )
			theApp.SetFileMapData ( 128, 1 );

		if ( ::GetAsyncKeyState (VK_RIGHT) & 0x8000 )
			theApp.SetFileMapData ( 132, 1 );
		if ( ::GetAsyncKeyState (68) & 0x8000 )
			theApp.SetFileMapData ( 132, 1 );

		if ( ::GetAsyncKeyState (VK_SPACE) & 0x8000 )
			theApp.SetFileMapData ( 136, 1 );
	}

	// 091215 - if no key pressed, reset kscancode value sent to GameGuru MapEditor
	if (bKeyDetected == false)
		theApp.SetFileMapData(100, 0);
}

void CEditorApp::UpdateToolBar ( void )
{
	/*
	DWORD		dwUndo	= ( DWORD ) m_pData [ 466 ];
	DWORD		dwRedo	= ( DWORD ) m_pData [ 470 ];
	DWORD		dwCut	= ( DWORD ) m_pData [ 474 ];
	DWORD		dwCopy	= ( DWORD ) m_pData [ 478 ];
	DWORD		dwClear	= ( DWORD ) m_pData [ 482 ];
	DWORD		dwBuildGame	= ( DWORD ) m_pData [ 486 ];
	*/

	cIPC* pIPC = m_Message.GetIPC ( );

	DWORD dwUndo		= 0;
	DWORD dwRedo		= 0;
	DWORD dwCut			= 0;
	DWORD dwCopy		= 0;
	DWORD dwClear		= 0;
	DWORD dwBuildGame	= 0;

	pIPC->ReceiveBuffer ( &dwUndo,      466, sizeof ( dwUndo      ) );
	pIPC->ReceiveBuffer ( &dwRedo,      470, sizeof ( dwRedo      ) );
	pIPC->ReceiveBuffer ( &dwCut,       474, sizeof ( dwCut       ) );
	pIPC->ReceiveBuffer ( &dwCopy,		478, sizeof ( dwCopy      ) );
	pIPC->ReceiveBuffer ( &dwClear,     482, sizeof ( dwClear     ) );
	pIPC->ReceiveBuffer ( &dwBuildGame, 486, sizeof ( dwBuildGame ) );

	if ( dwUndo )
		m_bTest [ 0 ] = true;
	else
		m_bTest [ 0 ] = false;
	
	if ( dwRedo )
		m_bTest [ 1 ] = true;
	else
		m_bTest [ 1 ] = false;

	if ( dwCut )
		m_bTest [ 2 ] = true;
	else
		m_bTest [ 2 ] = false;
	
	if ( dwCopy )
		m_bTest [ 3 ] = true;
	else
		m_bTest [ 3 ] = false;

	if ( dwClear )
		m_bTest [ 4 ] = true;
	else
		m_bTest [ 4 ] = false;

	if ( dwBuildGame )
		m_bTest [ 5 ] = true;
	else
		m_bTest [ 5 ] = false;


	// update the toolbar

	//DWORD		dwToolbar	= ( DWORD ) m_pData [ 300 ];
	//DWORD		dwButton	= ( DWORD ) m_pData [ 304 ];
	//DWORD		dwState		= ( DWORD ) m_pData [ 308 ];

	DWORD		dwToolbar	= 0;
	DWORD		dwButton	= 0;
	DWORD		dwState		= 0;

	pIPC->ReceiveBuffer ( &dwToolbar, 300, sizeof ( dwToolbar ) );
	pIPC->ReceiveBuffer ( &dwButton,  304, sizeof ( dwButton  ) );
	pIPC->ReceiveBuffer ( &dwState,   308, sizeof ( dwState   ) );

	CMainFrame* pFrame		= NULL;

	if ( dwCut || dwCopy || dwClear )
	{
		
	}

	if ( dwToolbar > 0 && dwToolbar < 10 )
	{
		pFrame = ( CMainFrame* ) theApp.m_pMainWnd;

		if ( dwToolbar == 1 )
		{
		
		}

		if ( pFrame )
		{
			pFrame->SetToolbarButtonState ( dwToolbar, dwButton, dwState );

			/////////////
			DWORD dwValue = 0;

			pIPC->SendBuffer ( &dwValue, 300, sizeof ( dwValue ) );

			//memcpy ( &m_pData [ 300 ], &dwValue, sizeof ( DWORD ) );
			/////////////

			//m_pData [ 300 ] = 0;
		}
	}

	if ( dwToolbar == 10 )
	{
		pFrame = ( CMainFrame* ) theApp.m_pMainWnd;

		//m_pData [ 300 ] = 0;

		/////////////
		DWORD dwValue = 0;

		pIPC->SendBuffer ( &dwValue, 300, sizeof ( dwValue ) );

		//memcpy ( &m_pData [ 300 ], &dwValue, sizeof ( DWORD ) );
		/////////////
	}

	
	{
		DWORD dwDisable = 0;

		//memcpy ( &dwDisable, &m_pData [ 850 ], sizeof ( DWORD ) );

		pIPC->ReceiveBuffer ( &dwDisable, 850, sizeof ( dwDisable ) );

		if ( dwDisable )
		{
			//theApp.m_Debug.Write ( "HandleMessages - message 850" );
			//theApp.m_Debug.Write ( "HandleMessages - disable icons" );

			m_bDisable = false;

			m_bTest [ 4 ] = false;
			m_bTest [ 3 ] = false;
			m_bTest [ 0 ] = false;
			m_bTest [ 2 ] = false;

			//theApp.m_Debug.Write ( "HandleMessages - message 850 - end" );

		}
		else
			m_bDisable = true;
	}
}

void CEditorApp::UpdateIDEForegroundFocus ( int iForeFocusFlag )
{
	static int g_iLastForeFocusFlag = 0;
	if ( iForeFocusFlag != g_iLastForeFocusFlag )
	{
		g_iLastForeFocusFlag = iForeFocusFlag;
		cIPC* pIPC = m_Message.GetIPC ( );
		DWORD dwForegroundFocus = iForeFocusFlag * 10;
		pIPC->SendBuffer ( &dwForegroundFocus, 596, sizeof ( dwForegroundFocus ) );
	}
}

void CEditorApp::UpdateStatusBar ( void )
{
	// status bar
	cIPC* pIPC = m_Message.GetIPC ( );

	SetCurrentDirectory ( theApp.m_szDirectory );

	DWORD			dwLayer = 0;
	DWORD			dwBrush = 0;
	pIPC->ReceiveBuffer ( &dwLayer, 600, sizeof ( dwLayer ) );
	pIPC->ReceiveBuffer ( &dwBrush, 612, sizeof ( dwBrush ) );

	CMainFrame*		pFrame  = ( CMainFrame* ) theApp.m_pMainWnd;

	SetCurrentDirectory ( theApp.m_szDirectory );

	theApp.pMainFrame->OnMouseMoveXX ( 0, CPoint ( 0, 0 ) );

}

void CEditorApp::UpdateLibraryWindow ( void )
{
	cIPC* pIPC = m_Message.GetIPC ( );
	DWORD dwAddImage			= 0;
	DWORD dwDeleteImage			= 0;
	DWORD dwLibraryTab			= 0;
	DWORD dwDeleteImageIndex	= 0;
	DWORD dwActiveTab        	= 0;
	DWORD dwScroll           	= 0;
	DWORD dwDeleteAllImages		= 0;

	pIPC->ReceiveBuffer ( &dwAddImage,			500, sizeof ( dwAddImage			) );
	pIPC->ReceiveBuffer ( &dwDeleteImage,		504, sizeof ( dwDeleteImage			) );
	pIPC->ReceiveBuffer ( &dwLibraryTab,		508, sizeof ( dwLibraryTab			) );
	pIPC->ReceiveBuffer ( &dwDeleteImageIndex,	512, sizeof ( dwDeleteImageIndex	) );
	pIPC->ReceiveBuffer ( &dwActiveTab,			534, sizeof ( dwActiveTab			) );
	pIPC->ReceiveBuffer ( &dwScroll,			538, sizeof ( dwScroll				) );
	pIPC->ReceiveBuffer ( &dwDeleteAllImages,	542, sizeof ( dwDeleteAllImages		) );

	TCHAR szImageFileName [ 255 ];
	TCHAR szImageName     [ 255 ];

	EnterCriticalSection( &m_csDirectoryChanges );
	SetCurrentDirectory ( theApp.m_szDirectory );

	// add an image to library
	if ( dwAddImage == 1 )
	{
		char szA [ 255 ];
		char szB [ 255 ];

		pIPC->ReceiveBuffer ( &szA [ 0 ], 1000, sizeof ( szA ) );
		pIPC->ReceiveBuffer ( &szB [ 0 ], 1256, sizeof ( szB ) );

		CString a = szA;
		CString b = szB;

		if ( strlen(szB) > 0 ) b = szB;
		else
		{
			strcpy_s( szB, 255, szA );

			char *szSlash = strrchr( szB, '\\' );
			if ( szSlash ) szSlash++;
			else szSlash = szB;

			char *szDot = strrchr( szSlash, '.' );
			if ( szDot ) *szDot = '\0';

			b = szSlash;
		}
		
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		strcpy ( szImageFileName, a );
		strcpy ( szImageName, b );
		pFrame->AddBitmapToWorkspace ( dwLibraryTab, &szImageFileName [ 0 ], &szImageName [ 0 ] );

		// reset string
		strcpy ( szImageName, _T ( "" ) );
		SetFileMapDataString ( 1000, szImageName );
		SetFileMapDataString ( 1256, szImageName );

		// reset
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 500, sizeof ( dwValue ) );
	}
	LeaveCriticalSection( &m_csDirectoryChanges );

	// remove image from library
	if ( dwDeleteImage == 1 )
	{
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		pFrame->RemoveBitmapFromWorkspace ( dwLibraryTab, dwDeleteImageIndex );
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 504, sizeof ( dwValue ) );
	}

	if ( dwActiveTab )
	{
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		pFrame->m_wndOutput.m_NewBrowser.m_tabCtrl.ActivateSSLPage ( dwActiveTab - 1 );
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 534, sizeof ( dwValue ) );
	}

	if ( dwScroll )
	{
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 538, sizeof ( dwValue ) );
	}

	if ( dwDeleteAllImages )
	{
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		pFrame->m_wndOutput.m_NewBrowser.RemoveAllBitmaps ( dwActiveTab );
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 542, sizeof ( dwValue ) );
	}
}

void CEditorApp::UpdateBrowserWindow ( void )
{
	cIPC* pIPC = m_Message.GetIPC ( );
	DWORD dwRun = 0;
	pIPC->ReceiveBuffer ( &dwRun, 800, sizeof ( dwRun ) );
	if ( dwRun == 1 )
	{
		m_Message.Stop ( );
		CMainFrame* pFrame = ( CMainFrame* ) theApp.m_pMainWnd;
		
		//pFrame->m_wndMenuBar.ResetAll ();								//Removed to stop the Menubar changing size when this was opened.
		
		//Hide menubar and toolbars when in library
		pFrame->m_wndMenuBar.ShowWindow(SW_HIDE);					//Hides Menu Bar
		pFrame->m_wndToolBar .ShowWindow(SW_HIDE);					//Hides Toolbars...
		//pFrame->m_wndToolBarGame .ShowWindow(SW_HIDE);
		//pFrame->m_wndToolBarDraw .ShowWindow(SW_HIDE);
		//if ( pFrame->m_wndToolBarEntity ) pFrame->m_wndToolBarEntity.ShowWindow (SW_HIDE);
		//if ( pFrame->m_wndToolBarMode ) pFrame->m_wndToolBarMode.ShowWindow (SW_HIDE);
		//pFrame->m_wndToolBarSegment .ShowWindow (SW_HIDE);
		//pFrame->m_wndToolBarView .ShowWindow (SW_HIDE);
		//pFrame->m_wndToolBarWaypoint .ShowWindow (SW_HIDE);
		pFrame->m_wndStatusBar .ShowWindow (SW_HIDE);
		pFrame->RecalcLayout(TRUE);									//Recalc layout to use the toolbar space.


		// V117 - 260410 - hide left-side panel (confusing when in library window mode)
		pMainFrame->m_wndOutput.ShowWindow(SW_HIDE);

		// launch library window
		cLibraryWindow library;
		pIPC->AllowSyncEventNow();
		library.DoModal ( );
		m_Message.Restart ( );
		
		// V117 - 260410 - show left-side panel after modal mode
		pMainFrame->m_wndOutput.ShowWindow(SW_SHOW);

		//Show menubar and toolbars when exit library
		pFrame->m_wndMenuBar. ShowWindow(SW_SHOW);				//Show menubars
		pFrame->m_wndToolBar .ShowWindow(SW_SHOW);				//Show Toolbars
		//pFrame->m_wndToolBarGame .ShowWindow(SW_SHOW);
		//pFrame->m_wndToolBarDraw .ShowWindow(SW_SHOW);
		//if ( pFrame->m_wndToolBarEntity ) pFrame->m_wndToolBarEntity.ShowWindow (SW_SHOW);
		//if ( pFrame->m_wndToolBarMode ) pFrame->m_wndToolBarMode.ShowWindow (SW_SHOW);
		//if ( pFrame->m_wndToolBarSegment ) pFrame->m_wndToolBarSegment.ShowWindow (SW_SHOW);
		//pFrame->m_wndToolBarView .ShowWindow (SW_SHOW);
		//pFrame->m_wndToolBarWaypoint .ShowWindow (SW_SHOW);
		pFrame->m_wndStatusBar .ShowWindow (SW_SHOW);			//Show Statusbar
		pFrame->RecalcLayout(TRUE);								//Recalculates layout to chack correct.

		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 800, sizeof ( dwValue ) );
	}
}

extern WPARAM  wParam1;
BOOL CALLBACK EnumWindowsProc1(HWND hwnd,LPARAM lParam );


void CEditorApp::UpdateFileOpenAndSave ( void )
{
	cIPC* pIPC = m_Message.GetIPC ( );

	DWORD dwOpen	 = 0;
	DWORD dwSave	 = 0;
	DWORD dwFileList = 0;

	pIPC->ReceiveBuffer ( &dwOpen,	   424, sizeof ( dwOpen	    ) );
	pIPC->ReceiveBuffer ( &dwSave,	   428, sizeof ( dwSave	    ) );
	pIPC->ReceiveBuffer ( &dwFileList, 438, sizeof ( dwFileList ) );

	//DWORD dwOpen = ( DWORD ) m_pData [ 424 ];
	//DWORD dwSave = ( DWORD ) m_pData [ 428 ];
	//DWORD dwFileList = ( DWORD ) m_pData [ 438 ];

	char szA [ 255 ];
	char szB [ 255 ];
	char szC [ 255 ];
	
	pIPC->ReceiveBuffer ( &szA [ 0 ], 1000, sizeof ( szA ) );
	pIPC->ReceiveBuffer ( &szB [ 0 ], 1256, sizeof ( szB ) );
	pIPC->ReceiveBuffer ( &szC [ 0 ], 1512, sizeof ( szC ) );

	//memcpy ( &szA [ 0 ], &m_pData [ 1000 ], sizeof ( char ) * 255 );
	//memcpy ( &szB [ 0 ], &m_pData [ 1256 ], sizeof ( char ) * 255 );
	//memcpy ( &szC [ 0 ], &m_pData [ 1512 ], sizeof ( char ) * 255 );
	
	CString szDirectory = szA;
	CString szFilter    = szB;
	CString szTitle     = szC;

	if ( dwFileList == 1 )
	{
		// 091215 - only add to recent file list if directory/file EXISTS
		HANDLE hFileExist = CreateFile(szA, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFileExist != INVALID_HANDLE_VALUE)
		{
			// Close File
			CloseHandle(hFileExist);
			AddToRecentFileList(szDirectory);
		}

		BYTE data [ 1000 ] = { 0 };

		pIPC->SendBuffer ( &data, 1000, sizeof ( data ) );

		//memset ( &m_pData [ 1000 ], 0, sizeof ( char ) * 255 );

		/////////////
		DWORD dwValue = 0;

		pIPC->SendBuffer ( &dwValue, 438, sizeof ( dwValue ) );

		//memcpy ( &m_pData [ 438 ], &dwValue, sizeof ( DWORD ) );
		/////////////

	}

	if ( dwOpen == 1 || dwSave == 1 )
	{
		//theApp.m_Debug.Write ( "UpdateFileOpenAndSave" );

		bool bType = true;

		if ( dwSave )
			bType = false;

		m_bFocus = true;
		
		m_Message.Stop ( );

		TCHAR				szFile [ 260 ];
		CString				newName;
		cNewCustomFileDialog	dialog ( bType, szDirectory, szTitle, szFilter );

	

		pIPC->AllowSyncEventNow();
		if ( dialog.DoModal ( ) == IDOK )
		{
			//theApp.m_Debug.Write ( "UpdateFileOpenAndSave - okay button clicked" );

			newName = dialog.GetPathName ( );
		}
		else
		{
			//theApp.m_Debug.Write ( "UpdateFileOpenAndSave - cancel button clicked" );
		}
		
		// 170105
		//wcscpy ( szFile, newName );
		strcpy ( szFile, newName );

		//theApp.m_Debug.Write ( "UpdateFileOpenAndSave - set file map string" );
		
		EnterCriticalSection( &m_csDirectoryChanges );

		SetCurrentDirectory  ( theApp.m_szDirectory );
		SetFileMapDataString ( 1000, szFile );

		//theApp.m_Debug.Write ( "UpdateFileOpenAndSave - reset file map message" );

		if ( dwOpen )
		{
			DWORD dwValue = 0;
			//memcpy ( &m_pData [ 424 ], &dwValue, sizeof ( DWORD ) );
			
			pIPC->SendBuffer ( &dwValue, 424, sizeof ( dwValue ) );

		}

		if ( dwSave )
		{
			/////////////
			DWORD dwValue = 0;
			//memcpy ( &m_pData [ 428 ], &dwValue, sizeof ( DWORD ) );

			pIPC->SendBuffer ( &dwValue, 428, sizeof ( dwValue ) );

			/////////////

			
		}

		LeaveCriticalSection( &m_csDirectoryChanges );

		//theApp.m_Debug.Write ( "UpdateFileOpenAndSave - restart messages" );

		m_Message.Restart ( );

		m_bFocus = false;

	}

}

void CEditorApp::UpdateMessageBoxes ( void )
{
	cIPC* pIPC = m_Message.GetIPC ( );

	char szMessage [ 255 ];
	char szTitle   [ 255 ];
	pIPC->ReceiveBuffer ( &szMessage [ 0 ], 1000, sizeof ( szMessage ) );
	pIPC->ReceiveBuffer ( &szTitle   [ 0 ], 1256, sizeof ( szTitle   ) );

	DWORD dwDisplay = 0;
	pIPC->ReceiveBuffer ( &dwDisplay,        900, sizeof ( dwDisplay ) );
	if ( dwDisplay != 0 )
	{
		// yes no dialog
		m_Message.Stop ( );
		CString message = szMessage;
		CString title   = szTitle;
		m_bFocus = true;
		int iID = 0;
		if ( dwDisplay == 1 ) iID = ::MessageBox ( NULL, message, title, MB_YESNO | MB_ICONQUESTION );
		if ( dwDisplay == 2 ) iID = ::MessageBox ( NULL, message, title, MB_YESNOCANCEL | MB_ICONQUESTION );
		m_bFocus = false;
		m_Message.Restart ( );

		// yes no
		if ( iID == IDYES )
		{
			DWORD dwValue = 1;
			pIPC->SendBuffer ( &dwValue, 904, sizeof ( dwValue ) );
		}
		if ( iID == IDNO )
		{
			DWORD dwValue = 0;
			pIPC->SendBuffer ( &dwValue, 904, sizeof ( dwValue ) );
		}
		// cancel
		if ( dwDisplay == 2 )
		{
			if ( iID == IDCANCEL )
			{
				DWORD dwValue = 2;
				pIPC->SendBuffer ( &dwValue, 904, sizeof ( dwValue ) );
			}
		}

		// reset flag
		DWORD dwValue = 0;
		pIPC->SendBuffer ( &dwValue, 900, sizeof ( dwValue ) );
	}
}

void CEditorApp::LoadSettings ( void )
{
	TCHAR szTrace      [ MAX_PATH ];
	TCHAR szProcess    [ MAX_PATH ];
	TCHAR szActivation [ MAX_PATH ];

	m_bDebugTrace      = false;
	m_bDebugProcess    = false;
	m_bDebugActivation = false;

	GetPrivateProfileString ( _T ( "Debug" ), _T ( "Trace"      ), _T ( "" ), szTrace,      MAX_PATH, theApp.m_szLanguageVariant );
	GetPrivateProfileString ( _T ( "Debug" ), _T ( "Process"    ), _T ( "" ), szProcess,    MAX_PATH, theApp.m_szLanguageVariant );
	GetPrivateProfileString ( _T ( "Debug" ), _T ( "Activation" ), _T ( "" ), szActivation, MAX_PATH, theApp.m_szLanguageVariant );

	if ( strcmp ( szTrace, "1" ) == 0 )
		m_bDebugTrace = true;

	if ( strcmp ( szProcess, "1" ) == 0 )
		m_bDebugProcess = true;

	if ( strcmp ( szActivation, "1" ) == 0 )
		m_bDebugActivation = true;
}

void CEditorApp::LoadLanguage ( void )
{
	// vars
	TCHAR szLanguage [ MAX_PATH ];
	TCHAR szPath     [ MAX_PATH ];

	// 140907 - init check
	GetPrivateProfileString ( _T ( "LOCALIZATION" ), _T ( "language" ), _T ( "" ), szLanguage, MAX_PATH, _T ( "./userdetails.ini" ) );
	if ( strlen ( szLanguage ) < 1 ) strcpy ( szLanguage, "english" );
	if ( strlen ( szLanguage )==0 )
	{
		// X10 - 121207 - We can assume that USERDETAILS is missing or corrupt, so we replace with default ENGLISH install
		// MessageBox ( NULL, "userdetails.ini has not specified the language to use", "Error", MB_OK );
		strcpy ( szLanguage, "English" );
	}

	//store the language name to pass to PHP
	strcpy_s( theApp.m_szLanguageName, 40, szLanguage );

	// generic text files for interface
	strcpy ( szPath, _T ( "./files/languagebank/" ) );
	strcat ( szPath, szLanguage );
	strcpy ( theApp.m_szLanguagePath, szPath );
	strcat ( szPath, "/textfiles" );
	strcat ( szPath, "/ide-words.ini" );
	strcpy ( theApp.m_szLanguage, theApp.m_szDirectory );
	strcat ( theApp.m_szLanguage, "\\" );
	strcat ( theApp.m_szLanguage, (szPath+2) );

	// variant specific text files for interface
	strcpy ( szPath, _T ( "./files/languagebank/" ) );
	strcat ( szPath, szLanguage );
	strcpy ( theApp.m_szLanguagePath, szPath );
	strcat ( szPath, "/textfiles" );
	strcat ( szPath, "/ide-words.ini" );
	strcpy ( theApp.m_szLanguageVariant, theApp.m_szDirectory );
	strcat ( theApp.m_szLanguageVariant, "\\" );
	strcat ( theApp.m_szLanguageVariant, (szPath+2) );

	// get error string (see if can access text file)
	GetPrivateProfileString( _T( "Getting Started" ), _T( "Error" ), _T(""), m_szErrorString, 64, theApp.m_szLanguage );	// LANGCHANGE - pre-load frequently used string
}

bool CEditorApp::CheckForMultipleInstances ( void )
{
	if ( m_bDebugProcess == false )
		return false;

	theApp.m_Debug.Write ( "Testing for multiple versions" );

	HANDLE hSnapShot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );

	PROCESSENTRY32 processInfo;
	processInfo.dwSize=sizeof(PROCESSENTRY32);
	int index=0;

	int iCount = 0;

	while(Process32Next(hSnapShot,&processInfo)!=FALSE)
	{
		// 170105
		//if ( wcscmp ( processInfo.szExeFile, _T ( "FPSCreator.exe" ) ) == 0 )
		if ( strcmp ( processInfo.szExeFile, _T ( "GameGuru.exe" ) ) == 0 )
		{
			iCount++;
			//CloseHandle(hSnapShot);
			//return FALSE;
		}
	}

	CloseHandle(hSnapShot);

	if ( iCount > 1 )
	{
		PostQuitMessage ( 0 );
		return TRUE;

		//Sleep ( 1000 );
	}

	return FALSE;

	theApp.m_Debug.Write ( "None found - okay to proceed" );
}

BOOL CEditorApp::InitInstance ( )
{
	// get command line
	strcpy ( theApp.m_CommandLineString, GetCommandLine() );
	strrev(theApp.m_CommandLineString);
	for ( int n=0; n<(int)strlen(theApp.m_CommandLineString); n++ )
	{
		if ( strnicmp ( theApp.m_CommandLineString+n, "exe.", 4 )==NULL )
		{
			theApp.m_CommandLineString[n]=0; 
			if ( theApp.m_CommandLineString[n-1]==34 ) theApp.m_CommandLineString[n-1]=0; 
			if ( theApp.m_CommandLineString[n-2]==32 ) theApp.m_CommandLineString[n-2]=0; 
			break;
		}
	}
	strrev(theApp.m_CommandLineString);

	// get current folder
	char pCurrentDir [ _MAX_PATH ];
	GetCurrentDirectory ( _MAX_PATH, pCurrentDir );
	strcpy ( m_pRootDir, pCurrentDir );

	// 010415 - check for existance of appid file
	g_bStandaloneNoNetMode = false;
	HANDLE hCheckExistFile = CreateFile ( "steam_appid.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hCheckExistFile!=INVALID_HANDLE_VALUE )
	{
		// it exists, this is Steam version of GameGuru
		CloseHandle ( hCheckExistFile );
	}
	else
	{
		// if missing, this is a standalone none steam version with no store or MP features
		g_bStandaloneNoNetMode = true;
	}

	// 050416 - also if in parental control mode, make the same removals to protect child
	if ( IsParentalControlActive()==true )
	{
		// prevent child from going to MP, viewing the store, accessing MP games via the menu
		g_bStandaloneNoNetMode = true;
	}

	// FPGC - 160909 - determine if X9, X10 or X9 (FPGC)
	g_bX9InterfaceMode = false;
	char pX9orX10Dir [ _MAX_PATH ];
	strcpy ( pX9orX10Dir, pCurrentDir );
	strcat ( pX9orX10Dir, "\\guru-game.exe" );
	HANDLE hFile = CreateFile ( pX9orX10Dir, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile!=INVALID_HANDLE_VALUE )
	{
		// this is X9
		CloseHandle ( hFile );
		g_bX9InterfaceMode = true;

		// determine if Kid friendly (equipment folder exists in gamecore)
		g_bSantitizedTeenKidFriendly = false;
		if ( SetCurrentDirectory( pCurrentDir ) )
			if ( SetCurrentDirectory( "Files" ) )
				if ( SetCurrentDirectory( "gamecore" ) )
					if ( SetCurrentDirectory( "equipment" ) )
						g_bSantitizedTeenKidFriendly = true;

		// restore folder
		SetCurrentDirectory ( pCurrentDir );
	}
	else
	{
		// this is X10, the exe is in a BIN folder
		g_bX9InterfaceMode = false;
		g_bSantitizedTeenKidFriendly = false;
	}

	// V105 - 220107 - Detect DirectX OCTOBER 2006 or later, or else editor does not work
	// X10 - 031007 - Detect DirectX 10 JUNE 2007 or later, or else software will not work
	bool bQuitEarly = false;
	char pWinDir [ _MAX_PATH ];
	GetWindowsDirectory ( pWinDir, _MAX_PATH );
	if ( strlen ( pWinDir ) > 0 )
	{
		strcat ( pWinDir, "\\system32\\d3dx10_34.dll" );
		OFSTRUCT os;
		if ( OpenFile ( pWinDir, &os, OF_EXIST )==HFILE_ERROR )
		{
			MessageBoxA ( NULL, "You need to reinstall this product as the required DirectX files are missing", "DirectX Files Not Installed", MB_OK );
			bQuitEarly = true;
		}
	}
	if ( bQuitEarly==true )
		return false;

	// 200807 - Vista Check
	if ( Is_Win_Vista_Or_Later()==TRUE )
	{
		// switch off Aero (as it messes up anti-alias mode which solves stretch issue on text)
		// 181207 - does not look great for a Logo app (just made text bigger)
		// DisableComposition();
	}

	// application start up

	// only create if no mutex exists for the GAME GURU INTERFACE
	m_hOnlyOneEditorMutex = OpenMutex ( MUTEX_ALL_ACCESS, FALSE, "THERECANBEONLYONEGAMEGURU" );
	if ( m_hOnlyOneEditorMutex == NULL )
	{
		m_hOnlyOneEditorMutex = CreateMutex (NULL, FALSE, "THERECANBEONLYONEGAMEGURU" );
		if ( m_hOnlyOneEditorMutex == NULL )
			return FALSE; // failed to create mutex for editor
	}
	else
		return FALSE; // editor already present in OS

	// debug info
	theApp.m_Debug.Write ( "InitInstance..." );

	// local variables
	CCommandLineInfo	cmdInfo;	// command line infor

	// 250105
	{
		theApp.m_Debug.Write ( "Getting current directory..." );

		char szFile [ 256 ] = "";
		char szEXE  [ 256 ] = "";
		char szDir  [ 256 ] = "";

		HMODULE mod = GetModuleHandle ( "gameguru.exe" );
		GetModuleFileName ( mod, szFile, 256 );

		int c = 0;

		for ( int i = strlen ( szFile ); i > 0; i--, c++ )
		{
			if ( szFile [ i ] == '\\' || szFile [ i ] == '/' )
			{
				//memcpy ( szEXE, &szFile [ i + 1 ], ( strlen ( szFile ) - i ) * sizeof ( char ) );

				memcpy ( szDir, &szFile, ( strlen ( szFile ) - c ) * sizeof ( char ) );

				break;
			}
		}

		theApp.m_Debug.Write ( szDir );

		SetCurrentDirectory ( szDir );

		
		strcpy ( m_szDirectory, szDir );

		i = 0;
	}

	//get user profile path
	strcpy_s( m_szUserPath, MAX_PATH, getenv( "USERPROFILE" ) );

	theApp.m_Debug.Write ( "Loading language data..." );

	// load the language file
	LoadLanguage ( );

	LoadSettings ( );

	// FPGC - 121009 - added full path to interface log (useful for continual debugging)
	char pInterfaceLogFile[_MAX_PATH];
	strcpy ( pInterfaceLogFile, m_szDirectory );
	strcat ( pInterfaceLogFile, "\\" );
	strcat ( pInterfaceLogFile, "gameguru.log" );
	m_Debug.Start ( pInterfaceLogFile );

	// proceed no further if multiple editors exist.
	if ( CheckForMultipleInstances ( ) )
		return TRUE;

	m_bAppRunning = false;
	m_bSetupFrame = false;

	theApp.m_Debug.Write ( "Starting file map..." );
	// Dave - 4+256 added to send steam id from mapeditor
	// Dave - added another 4+256 for sending text input
	m_Message.Start ( _T ( "FPSEXCHANGE" ), 6144+4+256+256+4, 1, this );
	SetFileMapData ( 44, 1 );
	SetFileMapData ( 486, 1 );
	SetFileMapData ( 850, 0 );
	// Set no data in (0) and steam id to null
	SetFileMapData ( 6145, 0 );
	SetFileMapDataString ( 6149, "" );

	// set the visual look of the application
	CBCGVisualManager::SetDefaultManager ( RUNTIME_CLASS ( CBCGVisualManagerXP ) );

	// set file valid flag to false
	m_bFileNewValid = false;

	theApp.m_Debug.Write ( "Parse command line..." );

	// parse the command line
	ParseCommandLine ( cmdInfo );

	// show the splash screen
	//CSplashWnd::EnableSplashScreen ( cmdInfo.m_bShowSplash );

	theApp.m_Debug.Write ( "AfxOleInit..." );

	// initialize OLE libraries
	if ( !AfxOleInit ( ) )
	{
		AfxMessageBox ( IDP_OLE_INIT_FAILED );
		return FALSE;
	}

	theApp.m_Debug.Write ( "AfxEnableControlContainer..." );

	// set up container and enable 3D controls
	AfxEnableControlContainer ( );
	//Enable3dControls          ( ); .NET depreciated

	theApp.m_Debug.Write ( "SetRegistryKey..." );

	// registry settings
	// 140105
	SetRegistryKey         ( _T ( "GameGuru" ) ); //FPSCreator
	LoadStdProfileSettings ( );
	SetRegistryBase        ( _T ( "Settings" ) );

	theApp.m_Debug.Write ( "EnableUserTools..." );

	// enable user defined tools
	EnableUserTools ( ID_TOOLS_ENTRY, ID_USER_TOOL1, ID_USER_TOOL10 );

	theApp.m_Debug.Write ( "Main Init..." );

	// set up managers
	InitMouseManager       ( );
	InitContextMenuManager ( );
	InitKeyboardManager    ( );
//	InitSkinManager        ( );
//	// set up skin manager
//	GetSkinManager ( )->EnableSkinsDownload ( _T ( "http://www.bcgsoft.com/Skins" ) );

	// register the application's document templates, document templates
	// serve as the connection between documents, frame windows and views

	theApp.m_Debug.Write ( "CMultiDocTemplate..." );

	CMultiDocTemplate* pDocTemplate;

	pDocTemplate = new CMultiDocTemplate (
											IDR_EDITORTYPE,
											RUNTIME_CLASS ( CEditorDoc  ),
											RUNTIME_CLASS ( CChildFrame ),
											RUNTIME_CLASS ( CEditorView )
										 );

	pDocTemplate->SetContainerInfo ( IDR_EDITORTYPE_CNTR_IP );

	AddDocTemplate ( pDocTemplate );

	// create main MDI Frame window
	pMainFrame = new CMainFrame;
	theApp.m_Debug.Write ( "LoadFrame..." );
	if ( !pMainFrame->LoadFrame ( IDR_MAINFRAME ) )
		return FALSE;

	m_pMainWnd = pMainFrame;

	// enable drag / drop open
	m_pMainWnd->DragAcceptFiles ( );

	// enable DDE execute open
	// 140105
	//EnableShellOpen        ( );
	//RegisterShellFileTypes ( TRUE );

	theApp.m_Debug.Write ( "ProcessShellCommand..." );

	// dispatch commands specified on the command line
	if ( !ProcessShellCommand ( cmdInfo ) )
		return FALSE;

	m_nCmdShow = SW_SHOWMAXIMIZED;

	// rhe main window has been initialized, so show and update it
	pMainFrame->ShowWindow   ( m_nCmdShow );
	pMainFrame->UpdateWindow ( );

	m_bFileNewValid = true;

	theApp.m_Debug.Write ( "OnFileNew..." );

	// before we start things up, detect any old FPSC-MapEditor.exe processes/windows
	// and remove them so we don't get that freeze issue for users who crashed out/etc
	char* szProcess = "Game Guru12345";//FPSC Map Editor12345";
	HWND hWndFind = FindWindow ( szProcess, NULL );
	if ( hWndFind )
	{
		// and keep repeating until ALL instances of FPSC-MapEditor.exe are gone
		while ( hWndFind )
		{
			// kill process
			DWORD dwProcess = 0;
			GetWindowThreadProcessId ( hWndFind, &dwProcess );
			//HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,false,dwProcess);
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,false,dwProcess);
			if ( hProcess ) 
			{
				DWORD dwSuccess = TerminateProcess(hProcess,0);
				WaitForSingleObject ( hProcess, 2000 );
				if ( dwSuccess==0 )
				{
					char str[256];
					FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, str, 256, NULL );
					TRACE("%s\n", str);
				}
			}

			// finds any more
			hWndFind = FindWindow ( szProcess, NULL );
		}
	}

	// automatically start a new project (launches FPSC-MapEditor.exe here)
	OnFileNewFlat(); // speed up initial load by using flat terrain

	theApp.m_Debug.Write ( "DoModal..." );

	/* 131017 - disable completely from IDE
	cFront front;
	theApp.m_Message.GetIPC()->AllowSyncEventNow();
	front.DoModal ( );	
	*/
	// Get version name
	strcpy_s( theApp.m_szVersion, 64, "" );
	FILE *pFile = NULL;
	int error = _tfopen_s( &pFile, _T("version.ini"), _T("rb") );
	if ( !error )
	{
		fgets( theApp.m_szVersion, 64, pFile );
		fclose( pFile );
		pFile = NULL;
	}

	theApp.m_Debug.Write ( "App Started..." );

	//HELPW - create and start the help wizard
	if ( m_pHelpWizardDlg )
	{
		m_pHelpWizardDlg->Terminate();
		delete m_pHelpWizardDlg;
		m_pHelpWizardDlg = NULL;
	}

	/* 051114 - remove geting started and auto update

	HKEY keyHelp = 0;
	DWORD dwValue = 1;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType;

	try
	{
		LONG ls = RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\FPSCreator\\Editor\\Settings", NULL, KEY_QUERY_VALUE, &keyHelp );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( keyHelp, "show_help_on_startup", NULL, &dwType, (BYTE*)&dwValue, &dwSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( keyHelp );
	}
	catch (int)
	{
		//MessageBoxA( NULL, "Failed to get registry value", "Help Wizard Error", 0 );
		RegCloseKey( keyHelp );
	}

	// call getting started if flag is one (default)
	if ( dwValue==1 )
	{
		theApp.m_bHelpWizardVisible = true;
		theApp.m_pHelpWizardDlg = new CHelpWizard();
		theApp.m_pHelpWizardDlg->Start( );
	}
	else
	{
		// X10 - 221107 - only show this message on second run (after reg key was created)
		// and only when the getting started flag has been switched off (to avoid multiple windows on startup)
		DWORD dwType;
		HKEY keyHelp = 0;
		DWORD dwValue = 0;
		DWORD dwSize = sizeof(DWORD);
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), NULL, KEY_QUERY_VALUE, &keyHelp );
		if ( ls == ERROR_SUCCESS ) RegQueryValueEx( keyHelp, _T("check_updates_on_startup"), NULL, &dwType, (BYTE*)&dwValue, &dwSize );
		RegCloseKey( keyHelp );
		if ( dwValue==2 )
		{
			int result = MessageBoxA( pMainFrame->m_hWnd, GetLanguageData( _T("Getting Started"), _T("UpdateCheck") ), GetLanguageData( _T("Auto Update"), _T("AutoUpdate") ), MB_YESNO );
			if ( result == IDYES ) dwValue = 1;
			else dwValue = 0;
			try 
			{
				LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), 0, NULL, 0, KEY_SET_VALUE, NULL, &keyHelp, NULL );
				if ( ls != ERROR_SUCCESS ) throw 0;
				ls = RegSetValueEx( keyHelp, _T("check_updates_on_startup"), 0, REG_DWORD, (BYTE*)&dwValue, 4 );
				if ( ls != ERROR_SUCCESS ) throw 1;
				RegCloseKey( keyHelp );
			}
			catch ( int )
			{
				MessageBoxA( pMainFrame->m_hWnd, GetLanguageData( _T("Auto Update"), _T("SetFailed") ), theApp.m_szErrorString, 0 );
				RegCloseKey( keyHelp );
			}
			RegCloseKey( keyHelp );
		}
	}

	//HELPW - end

	//AutoUpdate
	keyHelp = 0;
	DWORD dwCheck = 0;
	dwSize = sizeof(DWORD);

	try
	{
		LONG ls = RegOpenKeyExA( HKEY_CURRENT_USER, "Software\\FPSCreator\\Editor\\Settings", NULL, KEY_QUERY_VALUE, &keyHelp );
		if ( ls != ERROR_SUCCESS ) throw 0;

		ls = RegQueryValueEx( keyHelp, "check_updates_on_startup", NULL, &dwType, (BYTE*)&dwCheck, &dwSize );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( keyHelp );
	}
	catch (int)
	{
		//MessageBoxA( NULL, "Failed to get registry value", "Help Wizard Error", 0 );
		RegCloseKey( keyHelp );
	    dwCheck = 0;
	}

	if ( dwCheck==1 )
	{
		if ( m_pUpdateThread )	
		{
			m_pUpdateThread->CancelDownload( );
			m_pUpdateThread->Join( );
			delete m_pUpdateThread;
		}

		m_pUpdateThread = new UpdateCheckThread();
		m_pUpdateThread->SetStartUpCheck( );
		m_pUpdateThread->Start( );

		GetMainWnd( )->SetTimer( 1, 1000, NULL );

	}
	//AutoUpdate - end
	*/

	/*
	// tie the parent window to the DBP app (connect two local-inputs)
	HWND hwndTHChild = FindWindow ( "Game Guru12345", NULL );
	//HWND hwndTHParent = FindWindow ( NULL, "Game Guru" );
	DWORD dwTHParent, dwTHChild, dwChildProcessID;
	dwTHChild = GetWindowThreadProcessId ( hwndTHChild, &dwChildProcessID );
	dwTHParent = GetCurrentThreadId();
	
	DWORD dwCurrentProcessID = GetCurrentProcessId();
	HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(INVALID_HANDLE_VALUE != hThreadSnapshot)
	{
		THREADENTRY32 objThreadEntry32;
		objThreadEntry32.dwSize = sizeof(objThreadEntry32);
		if (Thread32First(hThreadSnapshot, &objThreadEntry32))
		{
			do
			{
				if (dwCurrentProcessID == objThreadEntry32.th32OwnerProcessID)
				{
					char szThread[1024];
					sprintf(szThread, "Thread ID : %d\n", objThreadEntry32.th32ThreadID);
					OutputDebugString(szThread);
				}
			}
			while(Thread32Next(hThreadSnapshot, &objThreadEntry32));
		}
		CloseHandle(hThreadSnapshot);
	}

	//BOOL bResult = AttachThreadInput ( dwTHParent, dwTHChild, TRUE );
	BOOL bResult = AttachThreadInput ( dwTHChild, dwTHParent, TRUE );
	bResult = bResult;
	*/

	// IDE HAS the foreground right now, but we will hand it over to the DBP App
	/* nope
	HWND hwndDBPApp = FindWindow ( "Game Guru12345", NULL );
	if ( hwndDBPApp )
	{
		//hwndDBPApp->SetFocus();
		//hwndDBPApp->SetActiveWindow();
		SetForegroundWindow(hwndDBPApp);
		DWORD process_id = 0;
		GetWindowThreadProcessId(hwndDBPApp, &process_id);
		AllowSetForegroundWindow(process_id);
	}
	*/

	// finished
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEditorApp message handlers

int CEditorApp::ExitInstance ( ) 
{
	// called when the application ends

	// end mutex so editor can be used again
	if ( m_hOnlyOneEditorMutex )
	{
		CloseHandle ( m_hOnlyOneEditorMutex );
		m_hOnlyOneEditorMutex = NULL;
	}

	// clean up the interface library
	BCGCBCleanUp ( );

	// signal the end of threads
	m_bEnd = TRUE;
	
	// call the base class function
	return CWinApp::ExitInstance ( );
}

void CEditorApp::OnAppAbout ( )
{
	// 110105
	if ( theApp.m_bTestGameCancel )
		return;

	// 270105 - force redraw
	theApp.m_bBuildGameDisplay = true;

	CAboutDlg aboutDlg;
	theApp.m_Message.GetIPC()->AllowSyncEventNow();
	aboutDlg.DoModal ( );

	// 270105 - force redraw
	theApp.m_bBuildGameDisplay = false;
}

void CEditorApp::OnAppAboutThisSomething ( )
{
	// later..
}

void CEditorApp::PreLoadState ( )
{
	GetMouseManager       ( )->AddView ( IDR_EDITORTYPE, _T ( "My view" ), IDR_EDITORTYPE );
	GetContextMenuManager ( )->AddMenu ( _T ( "My menu" ), IDR_CONTEXT_MENU );
}

// Key Entry Variables used for sending to MapEditor since it is not picking up messages anymore
char WindowsTextEntry[256] = "";
DWORD g_dwWindowsTextEntryPos = 0;
DWORD g_dwWindowsTextEntrySize = 256;
unsigned char g_cKeyPressed;
int g_iEntryCursorState;

BOOL CEditorApp::PreTranslateMessage ( MSG* pMsg )
{
	if ( CSplashWnd::PreTranslateAppMessage ( pMsg ) )
		return TRUE;

	// forward messages allowing ENTRY$() to function in DBP App
	// C++ CONVERSION - now only sends for use in INPUT as Entry() info is passed via filemap
	// This used to be commented out, but now back in to ensure Inkey works
	if ( g_hwndRealDBPApp==NULL ) g_hwndRealDBPApp = FindWindow ( "Game Guru12345", NULL );
	if ( g_hwndRealDBPApp )
	{
		// Fix for inkey sticking
		if ( pMsg->message==WM_CHAR || pMsg->message==WM_KEYUP )
			PostMessage ( g_hwndRealDBPApp, pMsg->message, pMsg->wParam, pMsg->lParam );			
	}

	/*
	// can forward messages direct to the Steam Overlay UI
	LPARAM lParam = 0;
	EnumWindows(EnumWindowsProcFindSteam, lParam);
	if ( pMsg->message==WM_KEYDOWN )
	{
		HWND hwndTry = FindWindow(NULL,"Steam");
		if ( hwndTry ) SendMessage ( hwndTry, pMsg->message, pMsg->wParam, pMsg->lParam );
		hwndTry = hwndTry;
	}
	*/

	// Send to MapEditor using filemap
	if ( pMsg->message==WM_CHAR )
	{

		// check if ptrs valid
		cIPC* pIPC = m_Message.GetIPC ( );
		if ( !pIPC ) CWinApp::PreTranslateMessage ( pMsg );;

		// If win string cleared externally (InputDLL)
		if( WindowsTextEntry[0] == 0 )
			g_dwWindowsTextEntryPos=0;

		// Key that was pressed
		g_cKeyPressed = (unsigned char)pMsg->wParam;
		//g_cInkeyCodeKey = g_cKeyPressed;

		if(g_dwWindowsTextEntryPos>g_dwWindowsTextEntrySize-4)
		{
			g_dwWindowsTextEntryPos = 0;
			WindowsTextEntry[0] = 0;
		}

		// leeadd - 020605 - Add/Remove from entry$() string
		// leeafix - 110206 - created behaviour disaster in U59 - place this behaviour in ENTRY$(1) parameter of CINPUT.CPP
//			if ( g_cKeyPressed==8 )
//			{
//				// Remove character from entry
//				if ( g_dwWindowsTextEntryPos>0 )
//				{
//					g_dwWindowsTextEntryPos--;
//					g_pGlob->pWindowsTextEntry[g_dwWindowsTextEntryPos]=0;
//				}
//			}
		// Add character to entry string, skip return presses
		if ( g_cKeyPressed != '\r' )
		{
			WindowsTextEntry[g_dwWindowsTextEntryPos]=g_cKeyPressed;
			g_dwWindowsTextEntryPos++;
			WindowsTextEntry[g_dwWindowsTextEntryPos]=0;
		}

		// 1 means "I want to have the text sent"
		if ( theApp.GetFileMapData ( 6147 ) == 1 )
		{
			theApp.SetFileMapDataString ( 6153 , WindowsTextEntry );
			// 2 means "text is ready to be picked u
			theApp.SetFileMapData ( 6147 , 2 );

			g_dwWindowsTextEntryPos = 0;
			WindowsTextEntry[0] = 0;
		}
		// 3 means "clear all key entry"
		else if ( theApp.GetFileMapData ( 6147 ) == 3 )
		{
			g_dwWindowsTextEntryPos = 0;
			WindowsTextEntry[0] = 0;

			// Add character to entry string
			if ( g_cKeyPressed != '\r' )
			{
				WindowsTextEntry[g_dwWindowsTextEntryPos]=g_cKeyPressed;
				g_dwWindowsTextEntryPos++;
				WindowsTextEntry[g_dwWindowsTextEntryPos]=0;
			}

			theApp.SetFileMapDataString ( 6153 , WindowsTextEntry );
			// 2 means "text is ready to be picked u
			theApp.SetFileMapData ( 6147 , 2 );

			g_dwWindowsTextEntryPos = 0;
			WindowsTextEntry[0] = 0;

		}

	}

	return CWinApp::PreTranslateMessage ( pMsg );
}

BOOL CEditorApp::OnOpenRecentFile(UINT nID)
{
	// 110105
	//if ( theApp.m_bTestGameCancel )
	//	return FALSE;

	SetFileMapData ( 442, 1 );

	//return CWinApp::OnOpenRecentFile ( nID );

	int nIndex = nID - ID_FILE_MRU_FILE1;

	CString file = ( *m_pRecentFileList ) [ nIndex ];

	TCHAR szFileName [ MAX_PATH ];

	// 170105
	//wcscpy ( szFileName, file );
	strcpy ( szFileName, file );

	SetFileMapDataString ( 1000, szFileName );

	//m_pRecentFileList [ 0 ] = _T ( "" );


	//return CWinApp::OnOpenRecentFile ( nID );
	return TRUE;
}

/*
BOOL CWinApp::OnOpenRecentFile(UINT nID)
{
	ASSERT_VALID(this);
	ASSERT(m_pRecentFileList != NULL);

	ASSERT(nID >= ID_FILE_MRU_FILE1);
	ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
	int nIndex = nID - ID_FILE_MRU_FILE1;
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE2("MRU: open file (%d) '%s'.\n", (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

	if (OpenDocumentFile((*m_pRecentFileList)[nIndex]) == NULL)
		m_pRecentFileList->Remove(nIndex);

	return TRUE;
}
*/

CDocument* CEditorApp::OpenDocumentFile ( LPCTSTR lpszFileName ) 
{
	//theApp.m_Debug.Write ( "OpenDocumentFile" );
	
	
	SetFileMapDataString ( 1000, ( LPTSTR ) lpszFileName );

	CDocument* pDocument = NULL;

	//theApp.m_Debug.Write ( "OpenDocumentFile - check OpenDocumentFile valid" );

	if ( !m_bSetupFrame )
	{
		

		CDocManager*    pDoc		= theApp.m_pDocManager;
		POSITION		pos			= pDoc->GetFirstDocTemplatePosition ( );
		CDocTemplate*	pTemplate	= pDoc->GetNextDocTemplate ( pos );

		//theApp.m_Debug.Write ( "OpenDocumentFile - Call base function for OpenDocumentFile" );

		pDocument = pTemplate->OpenDocumentFile ( NULL );
	}

	SetFileMapData ( 400, 1 );

	TCHAR szFileName [ MAX_PATH ];

	//170105
	//wcscpy ( szFileName, lpszFileName );
	strcpy ( szFileName, lpszFileName );

	SetFileMapDataString ( 1000, szFileName );

	m_bSetupFrame = true;

	//theApp.m_Debug.Write ( "OpenDocumentFile - return" );

	return pDocument;
	
	return CWinApp::OpenDocumentFile ( lpszFileName );
}

void CEditorApp::OnAppExit1() 
{
	// TODO: Add your command handler code here
	
	SetFileMapData ( 908, 1 );
	return;

		// clean up the interface library
	BCGCBCleanUp ( );

	// signal the end of threads
	m_bEnd = TRUE;

	// call the base class function
	CWinApp::ExitInstance ( );
}

int CEditorApp::Run() 
{
	return CWinApp::Run();
}

BOOL CEditorApp::OnIdle(LONG lCount) 
{
	// FPSEXCHANGE Event Monitor
	cIPC* pIPC = m_Message.GetIPC ( );
	if ( pIPC )
	{
		// 261015 - can detect if the main window gets to foreground here (continually called even in background)
		HWND hWnd = GetForegroundWindow();
		if ( hWnd==this->pMainFrame->m_hWnd )
			UpdateIDEForegroundFocus(1);
		else
			UpdateIDEForegroundFocus(0);

		// non-critical filemap activity
		// lee, what process for mapeditor GETTING values from interface
		UpdateMouse ( );

		// perform if mapeditor waiting for interface
		if ( pIPC->SkipIfEventNotWaiting () )
		{
			// Update from filemap
			/*
			if ( m_bGameOrBuildInProgress==true )
			{
				UpdateBuildGame       ( );
				UpdateTestGame        ( );
				UpdateRestart         ( );
			}
			else
			{
			*/
				UpdateToolBar         ( );
				UpdateStatusBar       ( );
				UpdateLibraryWindow   ( );
				UpdateBrowserWindow   ( );
				UpdateMessageBoxes    ( );
				UpdateFileOpenAndSave ( );
				UpdatePopup           ( );
				UpdateEntityWindow    ( );
				UpdateBuildGame       ( );
				UpdatePreferences     ( );
				UpdateTestGame        ( );
				UpdateFPIWizard       ( );
				UpdateProject         ( );
				UpdateRestart         ( );
				UpdateTermination	  ( );
			//}

			// 041115 - if trigger editor to go fullscreen (for video playback)
			if ( theApp.GetFileMapData ( 970 ) == 2 )
			{
				// clear trigger flag
				DWORD dwValue = 0;
				pIPC->SendBuffer ( &dwValue, 970, sizeof ( dwValue ) );
				pIPC->AllowSyncEventNow();

				// force IDE to full screen editor view
				((CMainFrame*)theApp.GetMainWnd())->TestOrMultiplayerGame ( 2 );
			}

			// Exit message
			if ( theApp.GetFileMapData ( 908 ) == 1 )
			{
				// perform quit code
				UpdateQuit ( );
			}

			// Set event
			pIPC->AllowSyncEventNow();
		}
	}

	// flag to trigger window quit
	if ( m_bEnd )
	{
		// demo version end splash
		#ifdef FPSC_DEMO
 		cDialogEnd end;
		m_bDoNotNeedToCheckForCrashesAnymore = true;
		pIPC->AllowSyncEventNow();
 		end.DoModal ( );
		#endif

		// Actual exit here
		m_bEnd = false;
		this->pMainFrame->Quit ( );
	}

	// when idle, and weblink requested, do so
	if ( g_bWebsiteLink )
	{
		// why is this so slow? 20/30 seconds..
		g_bWebsiteLink=false;
		TCHAR szLink [ MAX_PATH ];
		GetPrivateProfileString ( _T ( "Help" ), _T ( "Website" ), _T ( "" ), szLink, MAX_PATH, theApp.m_szLanguageVariant );
		ShellExecute ( NULL, "open", szLink, NULL, NULL, SW_SHOWNORMAL );
	}

	// X10 - 121207 - should not run under Remote Desktop (DX10 does not like it)
	if( GetSystemMetrics(SM_REMOTESESSION) != 0 ) // SM_REMOTESESSION = 0x1000
	{
		//"Direct3D does not work over a remote session."
		if ( g_bRemoteSessionNotAllowed==false )
		{
			// required user prompt
			if ( this->pMainFrame )
			{
				LPSTR pTheMessage = "Does not work over fast user switching or a remote session!";
				MessageBox ( this->pMainFrame->m_hWnd, pTheMessage, "Software Error", MB_OK );
			}

			// only show one message box, and try to shut down.
			g_bRemoteSessionNotAllowed = true;

			// Write to NT LOG file (Vista logo requirement)
			// The EventLog entry is done in the installer (only admin can write to HKEY_LOCAL)
			HANDLE hEventLog = RegisterEventSource ( NULL, "FPS Creator X10" );
			if ( hEventLog )
			{
				LPCSTR* pStringArray = (LPCSTR*)new LPSTR[1];
				if ( pStringArray )
				{
					pStringArray[0] = new char[256];
					if ( pStringArray[0] )
					{
						strcpy ( (char*)pStringArray[0], "Software does not work over a remote session!" );
						::ReportEvent (	hEventLog, EVENTLOG_ERROR_TYPE, CATEGORY_ONE, EVENT_ERROR_NOREMOTESESSION, NULL, 1, 0, pStringArray, NULL );
						delete pStringArray[0];
					}
					delete pStringArray;
				}
			}
		}

		// Exit application until quit
		OnAppExit1();
	}

	return CWinApp::OnIdle(lCount);
}

void CEditorApp::OnEditCopy1() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 458, 1 );
	
}

void CEditorApp::OnEditCopy() 
{
	theApp.SetFileMapData ( 458, 1 );
}

void CEditorApp::OnEditCut() 
{
	theApp.SetFileMapData ( 454, 1 );
}

void CEditorApp::OnEditClear() 
{
	theApp.SetFileMapData ( 462, 1 );
}

void CEditorApp::OnEditCut1() 
{
	theApp.SetFileMapData ( 454, 1 );
}

void CEditorApp::OnEditPaste() 
{

}

void CEditorApp::OnEditPaste1() 
{
	theApp.SetFileMapData ( 462, 1 );
}

void CEditorApp::OnEditUndo() 
{
	theApp.SetFileMapData ( 446, 1 );
}

void CEditorApp::OnEditRedo() 
{
	theApp.SetFileMapData ( 450, 1 );
}

void CEditorApp::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 4 ] );
}

void CEditorApp::OnUpdateBuildGame(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}


void CEditorApp::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 3 ] );
}

void CEditorApp::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 0 ] );
}

void CEditorApp::OnUpdateEditCut1(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 2 ] );
}

void CEditorApp::OnUpdateEditCopy1(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 3 ] );
}

void CEditorApp::OnUpdateEditPaste1(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bTest [ 4 ] );
}

void CEditorApp::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable ( m_bTest [ 0 ] );
}

void CEditorApp::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	//pCmdUI->Enable ( m_bTest [ 1 ] );
}

void CEditorApp::OnHelpWebsite() 
{
	g_bWebsiteLink=true;
}

void CEditorApp::OnTutorials(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileNewFlat(CCmdUI* pCmdUI) 
{
//	// FREE VERSION
//	if ( g_bFreeVersion )
//		pCmdUI->Enable ( false );
//	else
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileSaveAsEx(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFilePreferences(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnDownloadStoreItems(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnImportModel(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnCharacterCreator(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile1(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile2(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile3(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile4(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile5(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile6(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile7(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile8(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile9(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile10(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile11(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile12(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile13(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile14(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
}

void CEditorApp::OnUpdateFileMruFile15(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );

}

bool CEditorApp::IsParentalControlActive ( void )
{
	g_bAlsoRemoveRestrictedContentToggle = false;
	bool bParentalControlActive = false;
	FILE *pFile = NULL;
	int error = _tfopen_s( &pFile, _T("parentalcontrolactive.ini"), _T("rb") );
	if ( !error )
	{
		bParentalControlActive = true;
		fclose( pFile );
		pFile = NULL;
	}
	else
	{
		error = _tfopen_s( &pFile, _T("vrqcontrolmode.ini"), _T("rb") );
		if ( !error )
		{
			bParentalControlActive = true;
			g_bAlsoRemoveRestrictedContentToggle = true;
			fclose( pFile );
			pFile = NULL;
		}
		else
		{
			error = _tfopen_s( &pFile, _T("educontrolmode.ini"), _T("rb") );
			if ( !error )
			{
				bParentalControlActive = true;
				g_bAlsoRemoveRestrictedContentToggle = true;
				fclose( pFile );
				pFile = NULL;
			}
		}
	}
	return bParentalControlActive;
}

/*
void CEditorApp::OnUpdateViewToolbar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}

void CEditorApp::OnUpdateViewOutput(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}

void CEditorApp::OnUpdateViewView(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}

void CEditorApp::OnUpdateViewPrefab(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}

void CEditorApp::OnUpdateViewDraw(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}



void CEditorApp::OnUpdateViewWaypoint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );

	
	//pCmdUI->SetCheck  ( 0 );
	//pCmdUI->SetCheck (pm_wndToolBarWaypoint.GetStyle () & WS_VISIBLE);
}


void CEditorApp::OnUpdateViewStatusBar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );
	pCmdUI->SetCheck  ( m_bDisable );
}

void CEditorApp::OnUpdateViewTestGame(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( m_bDisable );

	pCmdUI->SetCheck  ( m_bDisable );
}
*/

/* remove serial code system
// X9 and X10 Serial Code Functions

bool CheckSerialCodePerByte( LPSTR pTryCodeOrNULL,int pA,int pB,int pC,int pD,int pE,int pF,int pG,int pH,int pI,int pJ,int pK,int pL,int pM,int pN,int pO,int pP )
{
	// if match, true
	int iMatches = 0;
	if ( pTryCodeOrNULL[0]==pA ) iMatches+=3;
	if ( pTryCodeOrNULL[2]==pC ) iMatches+=3;
	if ( pTryCodeOrNULL[4]==pE ) iMatches+=3;
	if ( pTryCodeOrNULL[6]==pG ) iMatches+=3;
	if ( pTryCodeOrNULL[8]==pI ) iMatches+=3;
	if ( pTryCodeOrNULL[10]==pK ) iMatches+=3;
	if ( pTryCodeOrNULL[12]==pM ) iMatches+=3;
	if ( pTryCodeOrNULL[14]==pO ) iMatches+=3;
	if ( pTryCodeOrNULL[1]==pB ) iMatches+=3;
	if ( pTryCodeOrNULL[3]==pD ) iMatches+=3;
	if ( pTryCodeOrNULL[5]==pF ) iMatches+=3;
	if ( pTryCodeOrNULL[7]==pH ) iMatches+=3;
	if ( pTryCodeOrNULL[9]==pJ ) iMatches+=3;
	if ( pTryCodeOrNULL[11]==pL ) iMatches+=3;
	if ( pTryCodeOrNULL[13]==pN ) iMatches+=3;
	if ( pTryCodeOrNULL[15]==pP ) iMatches+=3;
	if ( iMatches>=48 ) return true; else return false;
}

bool CEditorApp::IsSerialCodeValid ( LPSTR pTryCodeOrNULLOrig )
{
	// if NULL, simply checking for a valid code in the registry,
	// if not NULL, then we are going to write it to the registry if it is valid
	bool bValid = false;

	// treat empty string as no string
	LPSTR pTryCodeOrNULL = pTryCodeOrNULLOrig;
	if ( pTryCodeOrNULL!=NULL )
		if ( strcmp ( pTryCodeOrNULL, "" )==NULL )
			pTryCodeOrNULL = NULL;

	// check existing
	if ( pTryCodeOrNULL!=NULL )
	{
		// mask the initial ones
		DWORD dw1a = '1';
		DWORD dw1b = '1';
		DWORD dw0a = '0';
		DWORD dw0b = '0';

		// also ensure the serial code passed in conforms to 00V1, 0X10 or 00v1
		bool bBlackList = false;
		if ( pTryCodeOrNULL[0] != '0' ) bBlackList = true;
		if ( pTryCodeOrNULL[1] != '0' && pTryCodeOrNULL[1] != 'X' ) bBlackList = true;
		if ( pTryCodeOrNULL[2] != 'V' && pTryCodeOrNULL[2] != 'v' && pTryCodeOrNULL[2] != '1' ) bBlackList = true;
		if ( pTryCodeOrNULL[3] != '1' && pTryCodeOrNULL[3] != '0' ) bBlackList = true;

		// black listed codes 220508
		if ( dw0a != '0' ) bBlackList = true;
		if ( dw0b != '0' ) bBlackList = true;
		if ( dw1a != '1' ) bBlackList = true;
		if ( dw1b != '1' ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V100030031232e" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','0','0','3','0','0','3','1','2','3','2','e' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V1004004337a76" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','0','4','0','0','4','3','3','7','a','7','6' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V117850951d643" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'1','7','8','5','0','9','5','1','d','6','4','3' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V1042002338900" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','4','2','0','0','2','3','3','8','9','0','0' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "0X100000140429b3" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,'X',dw1b,'0','0','0','0','0','1','4','0','4','2','9','b','3' )==true ) bBlackList = true;
		// 080310 - submitted by WizardOfID
		if ( stricmp ( pTryCodeOrNULL, "00V101124020f6e7" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','1','1','2','4','0','2','0','f','6','e','7' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V1011403426f78" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','1','1','4','0','3','4','2','6','f','7','8' )==true ) bBlackList = true;
		// 160910 - found on http://www.siliconguide.com/qa/forum/messages/1584-1.shtml
		if ( stricmp ( pTryCodeOrNULL, "00V10000xl87dk9k" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','0','0','0','x','l','8','7','d','k','9','k' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V1011403426f78" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','1','1','4','0','3','4','2','6','f','7','8' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "0X1000022341bc8e" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,'X',dw1b,'0','0','0','0','2','2','3','4','1','b','c','8','e' )==true ) bBlackList = true;
		if ( stricmp ( pTryCodeOrNULL, "00V101124020f6e7" )==NULL ) bBlackList = bBlackList;
		if ( CheckSerialCodePerByte( pTryCodeOrNULL, dw0a,dw0a,'V',dw1b,'0','1','1','2','4','0','2','0','f','6','e','7' )==true ) bBlackList = true;
		if ( bBlackList==false )
		{
			// check newly entered code for validity (if not a black listed code)
			#ifdef FPGCSERIAL
			 if ( FPGCSerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			#else
			 if ( X9SerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			 if ( X10SerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			#endif

			// FPGC - 230909 - if still false, the serial code in the USERDETAILS is black listed or wrong
			if ( bValid==false )
			{
				// try the registry as a second line of attack
				pTryCodeOrNULL = NULL; 
			}
		}
	}
	if ( bValid==false && pTryCodeOrNULL==NULL )
	{
		// used to hold the temp serial code taken from registry
		char pMySerialCodeFromRegistry [ 512 ];
		strcpy ( pMySerialCodeFromRegistry, "" );

		// take existing code from registry and see if valid
		HKEY keyCode = 0;
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), NULL, KEY_QUERY_VALUE, &keyCode );
		if ( ls == ERROR_SUCCESS )
		{
			DWORD dwType=REG_SZ;
			DWORD dwSize=512;
			#ifdef FPGCSERIAL
			 ls = RegQueryValueEx ( keyCode, _T("my_fpgc_serial_code"), NULL, &dwType, (LPBYTE)pMySerialCodeFromRegistry, &dwSize );
			#else
			 if ( g_bX9InterfaceMode==true ) 
				ls = RegQueryValueEx ( keyCode, _T("my_serial_code"), NULL, &dwType, (LPBYTE)pMySerialCodeFromRegistry, &dwSize );
			 else
				ls = RegQueryValueEx ( keyCode, _T("my_x10_serial_code"), NULL, &dwType, (LPBYTE)pMySerialCodeFromRegistry, &dwSize );
			#endif
			if ( strlen(pMySerialCodeFromRegistry) > 0 ) pTryCodeOrNULL = pMySerialCodeFromRegistry;
		}
		if ( keyCode!=0 ) RegCloseKey( keyCode );

		// is it valid
		if ( pTryCodeOrNULL )
		{
			#ifdef FPGCSERIAL
			 if ( FPGCSerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			#else
			 if ( X9SerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			 if ( X10SerialCodeCheck ( pTryCodeOrNULL )==true ) bValid = true;
			#endif
		}
	}

	// is it valid
	if ( bValid==true && pTryCodeOrNULL!=NULL )
	{
		// valid code, write into registry
		HKEY keyCode = 0;
		LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), 0, NULL, 0, KEY_SET_VALUE, NULL, &keyCode, NULL );
		if ( ls == ERROR_SUCCESS )
		{
			#ifdef FPGCSERIAL
			 ls = RegSetValueEx( keyCode, _T("my_fpgc_serial_code"), 0, REG_SZ, (BYTE*)pTryCodeOrNULL, strlen(pTryCodeOrNULL)+1 );
			#else
 			 if ( g_bX9InterfaceMode==true ) 
				ls = RegSetValueEx( keyCode, _T("my_serial_code"), 0, REG_SZ, (BYTE*)pTryCodeOrNULL, strlen(pTryCodeOrNULL)+1 );
			 else
				ls = RegSetValueEx( keyCode, _T("my_x10_serial_code"), 0, REG_SZ, (BYTE*)pTryCodeOrNULL, strlen(pTryCodeOrNULL)+1 );
			#endif
		}
		if ( keyCode!=0 ) RegCloseKey( keyCode );

		// and flag as valid to pass this check
		bValid = true;
	}

	// return success if code is valid
	return bValid;
}

void CEditorApp::CheckSerialCodeValidity ( void ) 
{
	// Read pUserSerialCode
	char pUserFile [ 512 ];
	wsprintf ( pUserFile, "%s\\%s", theApp.m_szDirectory, "userdetails.ini" );
	char pUserSerialCode [ 512 ];
	wsprintf ( pUserSerialCode, "" );
	GetPrivateProfileString( _T("PERSONAL DATA"), _T("YourSerialCode"), _T(""), pUserSerialCode, 512, pUserFile );

	// V111 - 140608 - ensure anyone entering lower case 00v1 manually is auto-corrected (reduce support issues)
	if ( strlen ( pUserSerialCode ) > 5 )
		if ( strncmp ( pUserSerialCode, "00v1", 4 )==NULL )
			pUserSerialCode [ 2 ] = 'V';

	// Check pUserSerialCode
	if ( IsSerialCodeValid ( pUserSerialCode )==true )
	{
		// Only COMPLETE VERSION has a valid serial code, else it is a free
		// version that does not have a serial code (or is an invalid code).
		g_bFreeVersion = false;
	}
	else
	{
		// INVALID or no serial code means FREE VERSION
		g_bFreeVersion = true;

		// V110 BETA3 - 060608 - time stamp first use of free version for upgrade calculation
		HKEY keyCode = 0;
		char pMyFreeVersionStartDate [ 512 ];
		strcpy ( pMyFreeVersionStartDate, "" );
		LONG ls = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\FPSCreator\\Editor\\Settings"), NULL, KEY_QUERY_VALUE, &keyCode );
		if ( ls == ERROR_SUCCESS )
		{
			DWORD dwType=REG_SZ;
			DWORD dwSize=512;
			ls = RegQueryValueEx ( keyCode, _T("uniquevalue"), NULL, &dwType, (LPBYTE)pMyFreeVersionStartDate, &dwSize );
		}
		if ( keyCode!=0 ) RegCloseKey( keyCode );
		if ( strcmp ( pMyFreeVersionStartDate, "" )==NULL )
		{
			// work out start date
			int iDaysSince2008 = (int)X9GetTodaysDays();
			wsprintf ( pMyFreeVersionStartDate, "%d", iDaysSince2008 );

			// write start date for new free version usage
			keyCode = 0;
			ls = RegCreateKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\FPSCreator\\Editor\\Settings"), 0, NULL, 0, KEY_SET_VALUE, NULL, &keyCode, NULL );
			if ( ls == ERROR_SUCCESS )
			{
				ls = RegSetValueEx( keyCode, _T("uniquevalue"), 0, REG_SZ, (LPBYTE)pMyFreeVersionStartDate, strlen(pMyFreeVersionStartDate)+1 );
			}
			if ( keyCode!=0 ) RegCloseKey( keyCode );
		}
		else
		{
			// work out how many days have passed since start date
			int iDaysSince2008Start = atoi((LPSTR)pMyFreeVersionStartDate);
			int iDaysNow = (int)X9GetTodaysDays();
			g_iDaysHavePassed = iDaysNow - iDaysSince2008Start;
			if ( g_iDaysHavePassed < 60 )
			{
				if ( g_iDaysHavePassed < 45 )
					g_iUpgradeOffer = 1;
				else
					g_iUpgradeOffer = 2;
			}
			else
				g_iUpgradeOffer = 0;
		}
	}
}

DWORD CEditorApp::X9GetTodaysDays ( void )
{
	DWORD dwCurrent = 0;
	CTime time = CTime::GetCurrentTime();
	int iMonth = time.GetMonth ( );
	int iDay   = time.GetDay   ( );
	int iYear  = time.GetYear  ( ) - 2000;
	iYear *= 365;
	iMonth *= 31;
	return dwCurrent = iYear + iMonth + iDay;
}

bool CEditorApp::NetBookAuthentication ( LPSTR pSerialCodeStringToCheck )
{
	// needs tobe a code to be valid
	if ( !pSerialCodeStringToCheck )
		return false;

	// only if serial code matches special NETBOOK code and it fails do we return false
	if ( stricmp ( pSerialCodeStringToCheck, "00v10000001425df" )==NULL )
	{
		// it is netbook, authentication required
		g_bNetBookVersion = true;
		if ( g_lpNetBookError ) delete g_lpNetBookError;
		g_lpNetBookError = new char[512];
		strcpy ( g_lpNetBookError, "failure" );

		// use appupplugin.dll to authenticate
		if ( g_bNetBookVersionFirstTime==true )
		{
			g_bNetBookVersionFirstTime = false;
			HMODULE hAppUpPlugin = LoadLibrary ( "appupplugin.dll" );
			if ( hAppUpPlugin )
			{
				typedef int ( *APPROVEAPPFUNC ) ( DWORD, DWORD, DWORD, DWORD, DWORD* );
				APPROVEAPPFUNC ApproveApp = ( APPROVEAPPFUNC ) GetProcAddress ( hAppUpPlugin, "?ApproveApp@@YAHKKKKPAD@Z" );
				//int iPasswordValue = ApproveApp ( 0, 0, 0, 0, (DWORD*)"FPSCreatorPassword321" );
				int iPasswordValue = ApproveApp ( 0x4D491B1C, 0x2F794443, 0x8E2B3940, 0xF8AC165A, (DWORD*)"FPSCreatorPassword321" );
				if ( iPasswordValue==983 )
				{
					// if pass above authentication tests, success
					g_bNetBookVersionFirstTimeRes = true;
				}
				FreeLibrary ( hAppUpPlugin );
			}
		}
		if ( g_bNetBookVersionFirstTimeRes==true )
		{
			if ( g_lpNetBookError )
			{
				delete g_lpNetBookError;
				g_lpNetBookError=NULL;
			}
			return true;
		}
		return false;
	}
	else
	{
		// not netbook, pass
		return true;
	}
}

bool CEditorApp::FPGCSerialCodeCheck ( LPSTR pSerialCodeStringToCheck )
{
	// if no string instant failure
	if ( !pSerialCodeStringToCheck )
		return false;

	// if no string instant failure
	if ( strlen(pSerialCodeStringToCheck)<16 )
		return false;

	// serial code contains start and end date, only return true if it is
	// being run between these two dates

	// get scramble value
	char pCode [ 256 ];
	strcpy ( pCode, pSerialCodeStringToCheck );
	int scramble = pCode[strlen(pCode)-1] - '0';
	pCode[strlen(pCode)-1]=0;

	// unscamble string
	for ( int s=0; s<=scramble; s++ )
	{
		char temp[256];
		for ( DWORD c=0; c<strlen(pCode); c++ )
		{
			char cc=pCode[c];
			if ( cc=='A' )
				cc='Z';
			else
			{
				if ( cc=='0' )
					cc='9';
				else
					cc=cc-1;
			}
			temp[c]=cc;
		}
		temp[c]=0;
		strcpy ( pCode, temp );
	}

	// seperate data from string
	char usethistemp[256];
	char startstring[256];
	char finishstring[256];
	char customer[256];
	strcpy ( startstring, pCode ); startstring[4]=0;
	strcpy ( usethistemp, pCode ); strcpy ( pCode, usethistemp + 4 );
	strcpy ( finishstring, pCode+strlen(pCode)-4 );
	pCode [ strlen(pCode)-4 ] = 0;
	strcpy ( customer, pCode );

	// work out date from strings
	char num[256];
	int smonth = startstring[0]-64;
	strcpy ( num, startstring+1 ); num[2]=0;
	int sday = atoi(num);
	int syear=10+(startstring[3]-64);
	int stotal = (syear*365) + (smonth*31) + sday;
	int fmonth = finishstring[0]-64;
	strcpy ( num, finishstring+1 ); num[2]=0;
	int fday = atoi(num);
	int fyear=10+(finishstring[3]-64);
	int ftotal = (fyear*365) + (fmonth*31) + fday;

	// if todays date is inside the above range, return true
	CTime time = CTime::GetCurrentTime();
	int iMonth = time.GetMonth ( );
	int iDay   = time.GetDay   ( );
	int iYear  = time.GetYear  ( ) - 2000;
	int iThisTotal = (iYear*365) + (iMonth*31) + iDay;

	// reject anything that is NOT a date
	bool bRejectNotADate = false;
	if ( sday < 1 || sday > 31 ) bRejectNotADate = true;
	if ( fday < 1 || fday > 31 ) bRejectNotADate = true;
	if ( smonth < 1 || smonth > 12 ) bRejectNotADate = true;
	if ( fmonth < 1 || fmonth > 12 ) bRejectNotADate = true;
	if ( syear < 11 || syear > 61 ) bRejectNotADate = true;
	if ( fyear < 11 || fyear > 61 ) bRejectNotADate = true;

	//char prompt[256];
	//sprintf ( prompt, "%d %d %d - %d %d %d - %d %d %d : this=%d : start=%d : end=%d", iMonth, iDay, iYear, smonth, sday, syear, fmonth, fday, fyear, iThisTotal, stotal, ftotal );
	//MessageBox ( NULL, prompt, prompt, MB_OK );

	if ( bRejectNotADate==false )
	{
		if ( iThisTotal>=stotal && iThisTotal<=ftotal )
		{
			// this todays date is within start and end dates of license, so allow...
			return true;
		}
	}

	// serial code failure
	return false;
}

bool CEditorApp::X9SerialCodeCheck ( LPSTR pSerialCodeStringToCheck )
{
	// if no string instant failure
	if ( !pSerialCodeStringToCheck )
		return false;

	// Copied from the SERIAL DLL source code (amended for function call use here)
	int iTestStructureA = 0;

	// serial to check
	char szSerial [ 600 ] = "";
	strcpy ( szSerial, pSerialCodeStringToCheck );

	// check serial code
	if ( szSerial [ 0 ] == pKeyCodeX9[0] && szSerial [ 1 ] == pKeyCodeX9[1] && szSerial [ 2 ] == pKeyCodeX9[2] && szSerial [ 3 ] == pKeyCodeX9[3] )
	{
		char szNumberCheck [ 9 ] = "";
		sprintf ( 
					szNumberCheck,
					"%c%c%c%c%c%c%c%c",
					szSerial [  4 ],
					szSerial [  5 ],
					szSerial [  6 ],
					szSerial [  7 ],
					szSerial [  8 ],
					szSerial [  9 ],
					szSerial [ 10 ],
					szSerial [ 11 ]
				);

		MD5 numberCheck;
		unsigned int len = strlen ( ( char* ) szNumberCheck );
		numberCheck.update   ( ( unsigned char* ) szNumberCheck, len );
		numberCheck.finalize ( );

		char szVerifyCheck [ 600 ] = "";
		sprintf ( szVerifyCheck, "%s", numberCheck.hex_digest ( ) );
		if ( szVerifyCheck [ iKeyX9A ] == szSerial [ 12 ] )
			if ( szVerifyCheck [ iKeyX9B ] == szSerial [ 13 ] )
				if ( szVerifyCheck [ iKeyX9C ] == szSerial [ 14 ] )
					if ( szVerifyCheck [ iKeyX9D ] == szSerial [ 15 ] )
						iTestStructureA++;
	}

	// check code for validity
	if ( iTestStructureA > 0 )
	{
		// V117 - 280110 - intercept for NETBOOK authentication (specific code)
		return NetBookAuthentication ( pSerialCodeStringToCheck );
	}
	else
		return false;
}

bool CEditorApp::X10SerialCodeCheck ( LPSTR pSerialCodeStringToCheck )
{
	// if no string instant failure
	if ( !pSerialCodeStringToCheck )
		return false;

	// Copied from the SERIAL DLL source code (amended for function call use here)
	int iTestStructureA = 0;

	// serial to check
	char szSerial [ 600 ] = "";
	strcpy ( szSerial, pSerialCodeStringToCheck );

	// X10 - 280907 - users often enter lower case 0x10, so convert to uppercase here for convenience
	if ( szSerial [ 1 ]=='x' ) szSerial [ 1 ] = 'X';

	// check serial code
	if ( szSerial [ 0 ] == pKeyCodeX10[0] && szSerial [ 1 ] == pKeyCodeX10[1] && szSerial [ 2 ] == pKeyCodeX10[2] && szSerial [ 3 ] == pKeyCodeX10[3] )
	{
		char szNumberCheck [ 9 ] = "";
		sprintf ( 
					szNumberCheck,
					"%c%c%c%c%c%c%c%c",
					szSerial [  4 ],
					szSerial [  5 ],
					szSerial [  6 ],
					szSerial [  7 ],
					szSerial [  8 ],
					szSerial [  9 ],
					szSerial [ 10 ],
					szSerial [ 11 ]
				);

		MD5 numberCheck;
		unsigned int len = strlen ( ( char* ) szNumberCheck );
		numberCheck.update   ( ( unsigned char* ) szNumberCheck, len );
		numberCheck.finalize ( );

		char szVerifyCheck [ 600 ] = "";
		sprintf ( szVerifyCheck, "%s", numberCheck.hex_digest ( ) );
		if ( szVerifyCheck [ iKeyX10A ] == szSerial [ 12 ] )
			if ( szVerifyCheck [ iKeyX10B ] == szSerial [ 13 ] )
				if ( szVerifyCheck [ iKeyX10C ] == szSerial [ 14 ] )
					if ( szVerifyCheck [ iKeyX10D ] == szSerial [ 15 ] )
						iTestStructureA++;
	}

	// check code for validity
	if ( iTestStructureA > 0 )
	{
		// V117 - 280110 - intercept for NETBOOK authentication (specific code)
		return NetBookAuthentication ( pSerialCodeStringToCheck );
	}
	else
		return false;
}
*/

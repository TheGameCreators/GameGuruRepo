// HelpWizard.cpp : implementation file
//

#include "stdafx.h"
#include "HelpWizard.h"
#include "Editor.h"


// CHelpWizard dialog

IMPLEMENT_DYNAMIC(CHelpWizard, CDialog)

CHelpWizard::CHelpWizard(CWnd* pParent /*=NULL*/)
	: CDialog(CHelpWizard::IDD, pParent)
{
	m_bInit = false;
}

CHelpWizard::~CHelpWizard()
{
	theApp.m_bHelpWizardVisible = false;
	theApp.m_pHelpWizardDlg = NULL;
}

BOOL CHelpWizard::OnInitDialog()
{
	if ( !CDialog::OnInitDialog() ) return FALSE;

	/*
	// 191107 - add getting started title text
	SetWindowText ( GetLanguageData ( "Help Menu", "E" ) );

	m_tabCtrl.InsertItem( 0, GetLanguageData( _T("Getting Started"), _T("Intro") ) );
	m_tabCtrl.InsertItem( 1, GetLanguageData( _T("Getting Started"), _T("Videos") ) );
	m_tabCtrl.InsertItem( 2, GetLanguageData( _T("Getting Started"), _T("Samples") ) );
	m_tabCtrl.Init( );

	m_btnClose.SetWindowText( GetLanguageData( _T("Dialog Buttons"), _T("Close") ) );
	m_btnShowOnStartUp.SetWindowText( GetLanguageData( _T("Dialog Buttons"), _T("CheckBox1") ) );

	HKEY keyHelp = 0;
	DWORD dwValue = 1;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType;

	int varthrow=-1;
	{
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), NULL, KEY_QUERY_VALUE, &keyHelp );
		if ( ls != ERROR_SUCCESS ) varthrow = 0;

		ls = RegQueryValueEx( keyHelp, _T("show_help_on_startup"), NULL, &dwType, (BYTE*)&dwValue, &dwSize );
		if ( ls != ERROR_SUCCESS ) varthrow = 0;

		RegCloseKey( keyHelp );
	}
	if ( varthrow!=-1 )
	{
		//MessageBoxA( "Failed to get registry value", "Help Wizard Error", 0 );
	}

	if ( dwValue ) m_btnShowOnStartUp.SetCheck( 1 );
	else m_btnShowOnStartUp.SetCheck( 0 );

	CRect rect;
	GetClientRect( &rect );

	m_bInit = true;
	*/

	return TRUE;
}

void CHelpWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
	DDX_Control(pDX, IDC_CHECK1, m_btnShowOnStartUp);
	DDX_Control(pDX, IDOK, m_btnClose);
}

unsigned CHelpWizard::Run( )
{
	/*
	theApp.m_bHelpWizardVisible = true;
	DoModal( );
	theApp.m_bHelpWizardVisible = false;

	HKEY keyHelp = 0;
	DWORD dwValue = 0;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwType;

	int varthrow=-1;
	{
		LONG ls = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), NULL, KEY_QUERY_VALUE, &keyHelp );
		if ( ls != ERROR_SUCCESS ) varthrow = 0;

		ls = RegQueryValueEx( keyHelp, _T("check_updates_on_startup"), NULL, &dwType, (BYTE*)&dwValue, &dwSize );
		if ( ls != ERROR_SUCCESS ) varthrow = 1;

		RegCloseKey( keyHelp );
	}
	if ( varthrow!=-1 )
	{
		// X10 - 291007 - this means created for first time
		varthrow=-1;
		{
			LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), 0, NULL, 0, KEY_SET_VALUE, NULL, &keyHelp, NULL );
			if ( ls != ERROR_SUCCESS ) varthrow = 0;
			DWORD dwAskForAutoUpdateNextTime = 2;
			// FREE VERSION
			#ifdef FREEVERSION
			 dwAskForAutoUpdateNextTime=0; // in free version, no update check on startup
			#endif
			ls = RegSetValueEx( keyHelp, _T("check_updates_on_startup"), 0, REG_DWORD, (BYTE*)&dwAskForAutoUpdateNextTime, 4 );
			if ( ls != ERROR_SUCCESS ) varthrow = 1;
			RegCloseKey( keyHelp );
		}
		if ( varthrow!=-1 )
		{
			MessageBoxA( GetLanguageData( _T("Auto Update"), _T("SetFailed") ), theApp.m_szErrorString, 0 );
		}
	}


	// 221107 - dwValue==2 case moved to Editor.cpp so not dependent on gettingstarted
	*/

	// complete
	return 0;
}


BEGIN_MESSAGE_MAP(CHelpWizard, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CHelpWizard::OnTabChange)
	ON_BN_CLICKED(IDC_CHECK1, &CHelpWizard::OnShowStartupClicked)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CHelpWizard message handlers

void CHelpWizard::OnOK( )
{
	m_tabCtrl.OnClosing( );

	CDialog::OnOK( );
}

void CHelpWizard::OnCancel( )
{
	m_tabCtrl.OnClosing( );

	CDialog::OnCancel( );
}

void CHelpWizard::OnTabChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	

	*pResult = 0;
}

void CHelpWizard::OnSize( UINT nType, int cx, int cy )
{
	/* 141207 - cannot resize gs dialog
	if ( !m_bInit ) return;

	CDialog::OnSize( nType, cx, cy );

	CRect btnRect;
	m_btnClose.GetClientRect( &btnRect );
	m_btnClose.SetWindowPos( NULL, cx - btnRect.Width() - 7, cy - btnRect.Height() - 3, 0,0, SWP_NOSIZE );

	m_btnShowOnStartUp.GetClientRect( &btnRect );
	m_btnShowOnStartUp.SetWindowPos( NULL, 14, cy - btnRect.Height() - 8, 0,0, SWP_NOSIZE );

	m_tabCtrl.SetWindowPos( NULL, 0,0, cx-21, cy-50, SWP_NOMOVE );
	*/
}

void CHelpWizard::OnShowStartupClicked()
{
	/*
	//HKEY_CURRENT_USER\Software\FPSCreator\Editor\Settings

	HKEY keyHelp = 0;
		
	try 
	{
		//MessageBoxA( NULL, "Attempting to change registry", "Info", 0 );
		LONG ls = RegCreateKeyEx( HKEY_CURRENT_USER, _T("Software\\FPSCreator\\Editor\\Settings"), 0, NULL, 0, KEY_SET_VALUE, NULL, &keyHelp, NULL );
		if ( ls != ERROR_SUCCESS ) throw 0;

		DWORD value = m_btnShowOnStartUp.GetCheck();

		ls = RegSetValueEx( keyHelp, _T("show_help_on_startup"), 0, REG_DWORD, (BYTE*)&value, 4 );
		if ( ls != ERROR_SUCCESS ) throw 1;

		RegCloseKey( keyHelp );
	}
	catch ( int )
	{
		MessageBox( GetLanguageData( _T("Auto Update"), _T("SetFailed") ), theApp.m_szErrorString, 0 );
		RegCloseKey( keyHelp );
	}
	*/
}

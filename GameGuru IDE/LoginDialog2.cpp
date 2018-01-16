// LoginDialog.cpp : implementation file
//

#include "stdafx.h"

//#ifdef TGC_STORE

#include "LoginDialog2.h"
#include "Editor.h"
//#include "MarketWindow.h"

// CLoginDialog2 dialog

IMPLEMENT_DYNAMIC(CLoginDialog2, CDialog)

CLoginDialog2::CLoginDialog2(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDialog2::IDD, pParent)
{
	szUsername = 0;
	szPassword = 0;
	m_bOK = false;
}

CLoginDialog2::~CLoginDialog2()
{
	if ( szUsername ) delete [] szUsername;
	if ( szPassword ) delete [] szPassword;
}

void CLoginDialog2::SetUsername( const char* name )
{
	DWORD dwLength = (DWORD) strlen( name );
	
	if ( szUsername ) delete [] szUsername;
	szUsername = new char [ dwLength + 1 ];
	strcpy_s( szUsername, dwLength + 1, name );
	szUsername [ dwLength ] = '\0';
}

void CLoginDialog2::SetPassword( const char* name )
{
	DWORD dwLength = (DWORD) strlen( name );
	
	if ( szPassword ) delete [] szPassword;
	szPassword = new char [ dwLength + 1 ];
	strcpy_s( szPassword, dwLength + 1, name );
	szPassword [ dwLength ] = '\0';
}

void CLoginDialog2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME, m_edtUsername);
	DDX_Control(pDX, IDC_PASSWORD, m_edtPassword);
	DDX_Control(pDX, IDC_LOGIN_USERNAME, m_txtUsername);
	DDX_Control(pDX, IDC_LOGIN_PASSWORD, m_txtPassword);
	DDX_Control(pDX, IDC_LOGIN_STATEMENT, m_txtStatement);

	//DDX_Control(pDX, IDC_LOGIN_REGISTER, m_Lines[0]);
	//DDX_Control(pDX, IDC_LOGIN_STATEMENT2, m_Lines[1]);
	DDX_Control(pDX, IDOK, m_Button[0]);
	DDX_Control(pDX, IDCANCEL, m_Button[1]);

	SetWindowText ( GetLanguageData ( "Download Store", "LoginTitle" ) );
	m_txtUsername.SetWindowText ( GetLanguageData ( "Download Store", "Username" ) );
	m_txtPassword.SetWindowText ( GetLanguageData ( "Download Store", "Password" ) );
	m_txtStatement.SetWindowText ( GetLanguageData ( "Download Store", "Statement1" ) );
	//m_Lines [ 0 ].SetWindowText ( GetLanguageData ( "Download Store", "Register" ) );
	//m_Lines [ 1 ].SetWindowText ( GetLanguageData ( "Download Store", "Statement2" ) );
	m_Button [ 0 ].SetWindowText ( GetLanguageData ( "Download Store", "OK" ) );
	m_Button [ 1 ].SetWindowText ( GetLanguageData ( "Download Store", "Cancel" ) );
}

BOOL CLoginDialog2::OnInitDialog( )
{
	if ( !CDialog::OnInitDialog( ) ) return FALSE;

	//SetWindowText( GetLanguageData( "TGC Store", "LoginDetails" ) );

	SetUsername( theApp.m_sUsername );
	m_edtUsername.SetWindowText( szUsername );

	SetPassword( theApp.m_sPassword );
	m_edtPassword.SetWindowText( szPassword );

	if ( strlen( szUsername ) > 0 ) SetTimer( 2, 500, NULL );
	else SetTimer( 1, 500, NULL );

	//m_txtUsername.SetWindowText( GetLanguageData( _T("TGC Store"), _T("Email") ) );
	//m_txtPassword.SetWindowText( GetLanguageData( _T("TGC Store"), _T("Password") ) );
	//m_txtStatement.SetWindowText( GetLanguageData( _T("TGC Store"), _T("LoginPrompt") ) );
	
	m_edtPassword.SetPasswordChar( '*' );

	m_bOK = false;

	return TRUE;
}


BEGIN_MESSAGE_MAP(CLoginDialog2, CDialog)
	ON_BN_CLICKED(IDOK, &CLoginDialog2::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLoginDialog2 message handlers

void CLoginDialog2::OnBnClickedOk()
{
	char str [ 256 ];

	m_edtUsername.GetWindowText( str, 256 );
	DWORD dwLength = (DWORD) strlen(str);
	szUsername = new char [ dwLength + 1 ];
	strcpy_s( szUsername, dwLength + 1, str );
	szUsername [ dwLength ] = '\0';

	m_edtPassword.GetWindowText( str, 256 );
	dwLength = (DWORD) strlen(str);
	szPassword = new char [ dwLength + 1 ];
	strcpy_s( szPassword, dwLength + 1, str );
	szPassword [ dwLength ] = '\0';

	theApp.m_sUsername = szUsername;
	theApp.m_sPassword = szPassword;

	theApp.SetRegistryValue( "Software\\GameGuru\\Settings", "username", szUsername );	 //FPSCGameCreator

	char *szEnc1 = theApp.Encrypt( szPassword );
	char *szEnc2 = theApp.Encrypt( szEnc1 );
	theApp.SetRegistryValue( "Software\\GameGuru\\Settings", "password", szEnc2 ); //FPSCGameCreator

	delete [] szEnc1;
	delete [] szEnc2;

	m_bOK = true;

	OnOK();
}

void CLoginDialog2::OnTimer( UINT_PTR nIDEvent )
{
	if ( nIDEvent == 1 )
	{
		KillTimer( 1 );
		m_edtUsername.SetFocus( );
	}

	if ( nIDEvent == 2 )
	{
		KillTimer( 2 );
		m_edtPassword.SetFocus( );
	}
}

//#endif

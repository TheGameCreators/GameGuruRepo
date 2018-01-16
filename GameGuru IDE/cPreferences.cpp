// cPreferences.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "cPreferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Externals
extern bool g_bFreeVersion;
extern bool g_bX9InterfaceMode;
extern bool g_bSantitizedTeenKidFriendly;

/////////////////////////////////////////////////////////////////////////////
// cPreferences dialog


cPreferences::cPreferences(CWnd* pParent /*=NULL*/)
	: CDialog(cPreferences::IDD, pParent)
{
	//{{AFX_DATA_INIT(cPreferences)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bOK     = false;
	m_bCancel = false;
	m_bStart  = false;
}


void cPreferences::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(cPreferences)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	DDX_Control ( pDX, IDOK,						m_Buttons [ 0 ] );
	DDX_Control ( pDX, IDCANCEL,					m_Buttons [ 1 ] );
	DDX_Control ( pDX, IDC_VSYNC,					m_VSYNC );

	// Control visibility of preference items
	m_VSYNC.ShowWindow ( SW_SHOW );

	/* from languagebank
	SetWindowText ( GetLanguageData ( "Preferences", "Title" ) );
	m_Lines [ 0 ].SetWindowText ( GetLanguageData ( "Preferences", "Box1" ) );
	m_Lines [ 1 ].SetWindowText ( GetLanguageData ( "Preferences", "Box2" ) );
	m_Lines [ 2 ].SetWindowText ( GetLanguageData ( "Preferences", "Box3" ) );
	m_Lines [ 3 ].SetWindowText ( GetLanguageData ( "Preferences", "Box4" ) );
	m_Lines [ 4 ].SetWindowText ( GetLanguageData ( "Preferences", "Box5" ) );
	m_Lines [ 5 ].SetWindowText ( GetLanguageData ( "Preferences", "Box6" ) );
	m_DynamicLighting.SetWindowText		( GetLanguageData ( "Preferences", "Item1" ) );
	m_NoLightMapping.SetWindowText		( GetLanguageData ( "Preferences", "Item2" ) );
	m_QuickLightMapping.SetWindowText	( GetLanguageData ( "Preferences", "Item3" ) );
	m_FullLightMapping.SetWindowText	( GetLanguageData ( "Preferences", "Item4" ) );
	m_NoEffects.SetWindowText			( GetLanguageData ( "Preferences", "Item5" ) );
	m_EssentialEffects.SetWindowText	( GetLanguageData ( "Preferences", "Item6" ) );
	m_Low.SetWindowText					( GetLanguageData ( "Preferences", "Item7" ) );
	m_Medium.SetWindowText				( GetLanguageData ( "Preferences", "Item8" ) );
	m_High.SetWindowText				( GetLanguageData ( "Preferences", "Item9" ) );
	m_NoFloor.SetWindowText				( GetLanguageData ( "Preferences", "Item10" ) );
	m_ShowEntity.SetWindowText			( GetLanguageData ( "Preferences", "Item11" ) );
	m_ShowDebug.SetWindowText			( GetLanguageData ( "Preferences", "Item12" ) );
	m_Lines [ 5 ].SetWindowText			( GetLanguageData ( "Preferences", "Box6" ) );
	m_Lines [ 5 ].SetWindowText			( GetLanguageData ( "Preferences", "Box6" ) );
	m_SinglePlayer.SetWindowText		( GetLanguageData ( "Preferences", "Item13" ) );
	m_MultiPlayer.SetWindowText			( GetLanguageData ( "Preferences", "Item14" ) );
	m_Buttons [ 0 ].SetWindowText		( GetLanguageData ( "Preferences", "Button1" ) );
	m_Buttons [ 1 ].SetWindowText		( GetLanguageData ( "Preferences", "Button2" ) );
	*/

	/*
	// fpgc - 160909 - setup dialog flags based on mode
	if ( g_bX9InterfaceMode==false || g_bFreeVersion==true )
	{
		// X10 hide buttons not used
		m_NoEffects.ShowWindow ( SW_HIDE );
		m_EssentialEffects.ShowWindow ( SW_HIDE );
		m_FullEffects.ShowWindow ( SW_HIDE );
		m_Lines [ 1 ].ShowWindow ( SW_HIDE );
	}
	else
	{
		m_NoEffects.ShowWindow ( SW_SHOW );
		m_EssentialEffects.ShowWindow ( SW_SHOW );
		m_FullEffects.ShowWindow ( SW_HIDE );
	}

	// FPGC - 160909 - remove arena options if kid mode
	if ( g_bSantitizedTeenKidFriendly==true || g_bFreeVersion==true )
	{
		m_SinglePlayer.ShowWindow ( SW_HIDE );
		m_MultiPlayer.ShowWindow ( SW_HIDE );
		m_Lines [ 5 ].ShowWindow ( SW_HIDE );
	}

	// FPGC - 160909 - hide water default level for now
	if ( g_bX9InterfaceMode==true )
	{
		m_DynamicLighting.ShowWindow ( SW_HIDE );
	}
	*/
}


BEGIN_MESSAGE_MAP(cPreferences, CDialog)
	//{{AFX_MSG_MAP(cPreferences)
	ON_BN_CLICKED(IDC_VSYNC, OnVSYNC)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// cPreferences message handlers

void cPreferences::HandleMessages ( void )
{
	// temp flag
	bool bTerminateDialog = false;

	// check if ptrs valid
	cIPC* pIPC = m_Message.GetIPC ( );
	if ( !pIPC ) return;

	// flag that prefs activity started
	DWORD dwReady = 1;
	pIPC->SendBuffer ( &dwReady, 0, sizeof ( DWORD ) );

	// skip if mapeditor not waiting for interface
	if ( !pIPC->SkipIfEventNotWaiting () )
		return;

	m_pIPC = pIPC;
	m_Controls.Update ( m_pIPC );

	DWORD dwButton = 0;
	if ( m_bOK )
	{
		dwButton = 1;
		pIPC->SendBuffer ( &dwButton, 4, sizeof ( DWORD ) );
	}
	if ( m_bCancel )
	{
		dwButton = 1;
		pIPC->SendBuffer ( &dwButton, 8, sizeof ( DWORD ) );
	}

	DWORD dwClose = 0;
	pIPC->ReceiveBuffer ( &dwClose, 12, sizeof ( DWORD ) );
	if ( dwClose )
	{
		dwClose = 0;
		pIPC->SendBuffer ( &dwClose, 12, sizeof ( DWORD ) );
		pIPC->SendBuffer ( &dwClose, 4, sizeof ( DWORD ) );
		pIPC->SendBuffer ( &dwClose, 8, sizeof ( DWORD ) );
		pIPC->SendBuffer ( &dwClose, 0, sizeof ( DWORD ) );
		bTerminateDialog=true;
	}

	// only when update completely done, signal that event complete
	pIPC->AllowSyncEventNow();

	// only at end
	if ( bTerminateDialog )
	{
		CDialog::OnCancel ( );
		EndDialog ( 1 );
	}
}

BOOL cPreferences::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// control object assignment
	m_Controls.AddRadioButton ( &m_VSYNC,				1  );

	// start IPC
	m_Message.Start ( _T ( "FPSPREFERENCES" ), 1024, 1, this );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void cPreferences::OnOK() 
{
	m_bOK = true;
}

void cPreferences::OnCancel() 
{
	m_bCancel = true;
}

void cPreferences::OnVSYNC() 
{
	if ( m_Controls.m_bRadioClicked [ 1 ] )
	{
		m_Controls.SetRadio ( 1, false );
		m_VSYNC.SetCheck ( FALSE );
	}
	else
	{
		m_Controls.SetRadio ( 1, true );
		m_VSYNC.SetCheck ( TRUE );
	}
}

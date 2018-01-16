// Tab3.cpp : implementation file
//


#include "stdafx.h"
#include "Tab3.h"
#include "Editor.h"
#include "HelpWizard.h"

// Externals
extern bool g_bX9InterfaceMode;

#define _CRT_SECURE_NO_DEPRECATE

// CTab3 dialog

IMPLEMENT_DYNAMIC(CTab3, CDialog)

CTab3::CTab3(CWnd* pParent /*=NULL*/)
	: CDialog(CTab3::IDD, pParent)
{
	m_bInit = false;
}

CTab3::~CTab3()
{
}

void CTab3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_SamplesList);
	DDX_Control(pDX, IDC_BUTTON1, m_btnLoad);
}

BOOL CTab3::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE) pMsg->wParam=NULL ;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CTab3, CDialog)
	ON_WM_SIZE()

	ON_NOTIFY( NM_DBLCLK, IDC_LIST1, OnDblClickList )
	ON_NOTIFY( NM_CLICK, IDC_LIST1, OnClickList )
	ON_BN_CLICKED(IDC_BUTTON1, &CTab3::OnLoadClicked)
END_MESSAGE_MAP()


// CTab3 message handlers

BOOL CTab3::OnInitDialog( )
{
	if ( !CDialog::OnInitDialog( ) ) return FALSE;

	m_SamplesList.Init( );

	m_btnLoad.SetWindowText( GetLanguageData( _T("Dialog Buttons"), _T("Open") ) );

	m_bInit = true;

	return TRUE;
}

void CTab3::OnSize( UINT nType, int cx, int cy )
{
	if ( !m_bInit ) return;

	CDialog::OnSize( nType, cx, cy );

	CRect clientRect, btnRect;
	GetClientRect( &clientRect );

	if ( m_btnLoad.m_hWnd )
	{
		m_btnLoad.GetClientRect( &btnRect );
		m_SamplesList.SetWindowPos( NULL, clientRect.left, clientRect.top, clientRect.Width(), clientRect.Height() - 27, NULL );
		m_SamplesList.Arrange( LVA_ALIGNTOP );
		m_btnLoad.SetWindowPos( NULL, clientRect.right - btnRect.Width() - 1, clientRect.bottom - btnRect.Height() - 1, 0,0, SWP_NOSIZE );
	}
}

void CTab3::OnClickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_SamplesList.OnClickList( pNMHDR, pResult );
}

void CTab3::OnDblClickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	//MessageBox( "Sample Double-Clicked" );
	OnLoadClicked( );
}
void CTab3::OnLoadClicked()
{
	TCHAR szFilename [ 256 ] = _T("");
	m_SamplesList.GetFilenameSelected( 256, szFilename );
	if ( _tcslen( szFilename ) <= 1 ) return;
	theApp.m_pHelpWizardDlg->ShowWindow( SW_MINIMIZE );
	/* X9 or X10
	if ( g_bX9InterfaceMode==true )
	{
		// FPGC - 190909 - combo interface
		char pAbsFilenameToSampleFPM[_MAX_PATH];
		strcpy ( pAbsFilenameToSampleFPM, theApp.m_szDirectory );
		// FPGC - 240510 - if no prefabs, this is the FPGC variant, so use tutorials not samples
		if ( g_bDisablePrefabMode==true )
			strcat ( pAbsFilenameToSampleFPM, "\\files\\mapbank\\tutorials\\" );
		else
			strcat ( pAbsFilenameToSampleFPM, "\\files\\mapbank\\samples\\" );
		strcat ( pAbsFilenameToSampleFPM, szFilename );
		theApp.SetFileMapDataString ( 1000, pAbsFilenameToSampleFPM );
		theApp.SetFileMapData ( 442, 1 );
		theApp.SetFileMapDataString ( 1000, pAbsFilenameToSampleFPM );
	}
	else
	{
		// X10 from MyDocs folder
		char pMyDocumentsFolderFile[_MAX_PATH];
		SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, 0, pMyDocumentsFolderFile );
		strcat ( pMyDocumentsFolderFile, "\\FPSC X10 Files\\mapbank\\samples\\" );
		strcat ( pMyDocumentsFolderFile, szFilename );
		theApp.SetFileMapDataString ( 1000, pMyDocumentsFolderFile );
		theApp.SetFileMapData ( 442, 1 );
		theApp.SetFileMapDataString ( 1000, pMyDocumentsFolderFile );
	}
	*/

	// eventually replace with MyDocs code
	char pAbsFilenameToSampleFPM[_MAX_PATH];
	strcpy ( pAbsFilenameToSampleFPM, theApp.m_szDirectory );
	strcat ( pAbsFilenameToSampleFPM, "\\files\\mapbank\\samples\\" );
	strcat ( pAbsFilenameToSampleFPM, szFilename );
	theApp.SetFileMapDataString ( 1000, pAbsFilenameToSampleFPM );
	theApp.SetFileMapData ( 442, 1 );
	theApp.SetFileMapDataString ( 1000, pAbsFilenameToSampleFPM );
}

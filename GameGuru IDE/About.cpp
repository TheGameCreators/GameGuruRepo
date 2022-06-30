#include "stdafx.h"
#include "Editor.h"

#include "about.h"


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
		ON_WM_DESTROY()
		ON_WM_CREATE()
		ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_QUITCODE, &CAboutDlg::OnStnClickedAboutLine1)
	ON_STN_CLICKED(IDC_ABOUT_LINE2, &CAboutDlg::OnStnClickedAboutLine2)
END_MESSAGE_MAP()


CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_COMPANY_URL, m_btnURL);
	DDX_Control(pDX, IDC_COMPANY_URL2, m_btnURL1);

	DDX_Control ( pDX, IDC_ABOUT_LINE1, m_Line1 );
	DDX_Control ( pDX, IDC_ABOUT_LINE2, m_Line2 );
	DDX_Control ( pDX, IDC_ABOUT_LINE3, m_Line3 );
	DDX_Control ( pDX, IDC_ABOUT_LINE4, m_Line4 );
	DDX_Control ( pDX, IDC_ABOUT_LINE5, m_Line5 );

	DDX_Control ( pDX, IDOK, m_OK );
	//}}AFX_DATA_MAP

	SetWindowText ( GetLanguageData ( "About", "Title" ) );
	m_Line1.SetWindowText ( "" );
	m_Line2.SetWindowText ( GetLanguageData ( "About", "Line2" ) );
	m_Line3.SetWindowText ( GetLanguageData ( "About", "Line3" ) );
	m_Line4.SetWindowText ( GetLanguageData ( "About", "Line4" ) );
	m_Line5.SetWindowText ( GetLanguageData ( "About", "Line5" ) );
	m_OK.SetWindowText ( GetLanguageData ( "About", "Button" ) );

	m_btnURL1.SetWindowText ( GetLanguageData ( "Help", "Website" ) );
	m_btnURL.SetWindowText ( GetLanguageData ( "Help", "CompanyURL" ) );
	m_btnURL1.SetURL ( GetLanguageData ( "Help", "Website" ) );
	m_btnURL.SetURL ( GetLanguageData ( "Help", "CompanyURL" ) );

	// FPGC - 210510 - size web buttons to center them in about box
	m_btnURL.SizeToContent(TRUE, TRUE);
	m_btnURL1.SizeToContent(TRUE, TRUE);

	// Branding
	#ifdef GGBRANDED
	m_btnURL.ShowWindow(SW_HIDE);
	m_Line5.ShowWindow(SW_HIDE);
	#else
	#endif
}

int CAboutDlg::OnCreate ( LPCREATESTRUCT lpCreateStruct )
{
	// create the dialog

	// call the implementation
	if ( CDialog::OnCreate ( lpCreateStruct ) == -1 )
		return -1;

	SetCurrentDirectory ( theApp.m_szDirectory );
	SetCurrentDirectory ( theApp.m_szLanguagePath );
	SetCurrentDirectory ( "artwork" );
	#ifdef GGBRANDED
	SetCurrentDirectory ( "branded" );	
	#endif
	m_hDCSRC = CreateCompatibleDC ( NULL ); 
	m_hBMP = LoadImage ( NULL, "aboutsplash.bmp", IMAGE_BITMAP, 320, 160, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );
	m_hBMPLogo1 = LoadImage ( NULL, "TGCBadge.bmp", IMAGE_BITMAP, 160, 80, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );
	#ifdef GGBRANDED
	m_hBMPLogo2 = LoadImage ( NULL, "TGCBadge2.bmp", IMAGE_BITMAP, 160, 80, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );
	m_hBMPLogo3 = LoadImage ( NULL, "TGCBadge3.bmp", IMAGE_BITMAP, 160, 80, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );
	#endif
	SetCurrentDirectory ( theApp.m_szDirectory );

	// all done
	return 0;
}

BOOL CAboutDlg::OnInitDialog() 
{
	// default dialog inits
	CDialog::OnInitDialog();

	// update build number
	char versionstring[512];
	wsprintf ( versionstring, "Build : %s", theApp.m_szVersion );
	m_Line1.SetWindowText ( versionstring );

	// complete
	return TRUE;
}

void CAboutDlg::OnDestroy ( )
{
	// dialog is about to be destroyed
	// call default destruction
	CDialog::OnDestroy ( );
}

void CAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	m_hOld = SelectObject ( m_hDCSRC, m_hBMP );
	RECT rect1 = {124, 60, 0, 0};
	MapDialogRect ( &rect1 );
	BitBlt ( dc, rect1.left-160, rect1.top-80, 320, 160, m_hDCSRC, 0, 0, SRCCOPY );

	#ifdef GGBRANDED
	SelectObject ( m_hDCSRC, m_hBMPLogo1 );
	RECT rect2 = {124, 280, 0, 0};
	MapDialogRect ( &rect2 );
	BitBlt ( dc, rect2.left-80, rect2.top-160, 160, 80, m_hDCSRC, 0, 0, SRCCOPY );
	SelectObject ( m_hDCSRC, m_hBMPLogo2 );
	rect2 = {124, 280, 0, 0};
	MapDialogRect ( &rect2 );
	BitBlt ( dc, rect2.left-167, rect2.top-65, 160, 80, m_hDCSRC, 0, 0, SRCCOPY );
	SelectObject ( m_hDCSRC, m_hBMPLogo3 );
	rect2 = {124, 280, 0, 0};
	MapDialogRect ( &rect2 );
	BitBlt ( dc, rect2.left+7, rect2.top-65, 160, 80, m_hDCSRC, 0, 0, SRCCOPY );
	#else
	SelectObject ( m_hDCSRC, m_hBMPLogo1 );
	RECT rect2 = {124, 280, 0, 0};
	MapDialogRect ( &rect2 );
	BitBlt ( dc, rect2.left-80, rect2.top-40, 160, 80, m_hDCSRC, 0, 0, SRCCOPY );
	#endif

	SelectObject ( m_hDCSRC, m_hOld );
}

void CAboutDlg::OnStnClickedAboutLine1()
{
	// TODO: Add your control notification handler code here
}

void CAboutDlg::OnStnClickedAboutLine2()
{
	// TODO: Add your control notification handler code here
}

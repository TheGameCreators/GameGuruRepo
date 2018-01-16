// cLibraryWindow.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "cLibraryWindow.h"
#include "Poster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int m_iScrollEntity   = 0;
int m_iScrollPrefab   = 0;
int m_iScrollSegments = 0;

extern char		m_szLastPathName [ 255 ];

cLibraryWindow::cLibraryWindow ( CWnd* pParent ) : CDialog ( cLibraryWindow::IDD, pParent )
{
	//{{AFX_DATA_INIT(cLibraryWindow)
	//}}AFX_DATA_INIT
}

void cLibraryWindow::DoDataExchange ( CDataExchange* pDX )
{
	CDialog::DoDataExchange ( pDX );
	
	//{{AFX_DATA_MAP(cLibraryWindow)
	DDX_Control ( pDX, IDC_TREE1,     m_Tree    );
	DDX_Control ( pDX, IDC_LIST2,     m_Image   );
	DDX_Control ( pDX, IDC_SPLIT_MID, m_spliter );

	DDX_Control ( pDX, IDOK, m_Ok );
	DDX_Control ( pDX, IDCANCEL, m_Cancel );
	//}}AFX_DATA_MAP

	m_Ok.SetWindowText     ( GetLanguageData ( "Library", "Button1" ) );
	m_Cancel.SetWindowText ( GetLanguageData ( "Library", "Button2" ) );

	DDX_Control(pDX, IDC_BUTTON1, m_btnSearch);
	DDX_Control(pDX, IDC_TEXT_SEARCH, m_txtSearch);
	DDX_Control(pDX, IDC_PROGRESS_SEARCH, m_ProgressSpinner);
}

BEGIN_MESSAGE_MAP ( cLibraryWindow, CDialog )
	//{{AFX_MSG_MAP(cLibraryWindow)
	ON_WM_CREATE ( )
	ON_NOTIFY    ( TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree )
	ON_NOTIFY    ( TVN_ITEMEXPANDING, IDC_TREE1, OnItemexpandingTree )
	ON_NOTIFY    ( NM_DBLCLK, IDC_LIST2, OnClickList )
	ON_NOTIFY    ( NM_CLICK, IDC_LIST2, OnClickList1 )
	ON_NOTIFY	 ( NM_RCLICK, IDC_LIST2, OnRClickList )
	ON_WM_RBUTTONUP()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP ( )

int cLibraryWindow::OnCreate ( LPCREATESTRUCT lpCreateStruct )
{
	if ( CDialog::OnCreate ( lpCreateStruct ) == -1 )
		return -1;

	return 0;
}

BOOL cLibraryWindow::OnInitDialog ( )
{
	CDialog::OnInitDialog ( );

	m_Image.Start ( );
	m_Tree.Start  ( &m_Image, m_iScrollPrefab );
	
	char szBlankImage [ 255 ];
	CString string = "FPSEXCHANGE";

	memset ( szBlankImage, 0, sizeof ( szBlankImage ) );
	cIPC* pIPC	= theApp.m_Message.GetIPC ( );
	pIPC->ReceiveBuffer ( &szBlankImage [ 0 ], 2280, sizeof ( szBlankImage ) );

	CString title = szBlankImage;
	SetWindowText ( title );

	m_spliter.addWindow ( IDC_TREE1,sp_StyleRight      );
	m_spliter.addWindow ( IDC_LIST2,sp_StyleLeft       );
	m_spliter.setStyle  ( sp_StyleLeft | sp_StyleRight );

	return TRUE;
}

void cLibraryWindow::OnSelchangedTree ( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_Tree.OnSelchangedTree ( pNMHDR, pResult );
	CString sPath = m_Tree.GetSelectionPath ( );
	UpdateImageList ( sPath );
}

void cLibraryWindow::OnItemexpandingTree ( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_Tree.OnItemexpandingTree ( pNMHDR, pResult );
}

void cLibraryWindow::OnClickList1 ( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_Image.OnClickList ( pNMHDR, pResult );

	// get path to blank (to work out size of root)
	int iSkipRoot = m_Tree.m_sPath.GetLength();
	LPNMITEMACTIVATE pItemAct = ( LPNMITEMACTIVATE ) pNMHDR;
	if ( pItemAct->iItem > -1 )
	{
		DWORD dwItemIndex = pItemAct->iItem;
		LPSTR pFullPathStr = m_Image.m_szImageFileNames[dwItemIndex] + iSkipRoot;
		if ( iSkipRoot < strlen(m_Image.m_szImageFileNames[dwItemIndex]) ) 
		{
			LPSTR pFullPathStrFinal = pFullPathStr;
			if ( *((char*)pFullPathStr-2)=='\\' ) pFullPathStrFinal = pFullPathStr-2;
			if ( *((char*)pFullPathStr-1)=='\\' ) pFullPathStrFinal = pFullPathStr-1;
			if ( *((char*)pFullPathStr-0)=='\\' ) pFullPathStrFinal = pFullPathStr-0;
			if ( *((char*)pFullPathStr+1)=='\\' ) pFullPathStrFinal = pFullPathStr+1;
			if ( *((char*)pFullPathStr+2)=='\\' ) pFullPathStrFinal = pFullPathStr+2;
			CString pNewTitle = CString("Game Guru - ");
			char pCropName[_MAX_PATH];
			strcpy ( pCropName, pFullPathStrFinal+1 );
			pCropName[strlen(pCropName)-4]=0;
			pNewTitle = pNewTitle + CString(pCropName);
			theApp.GetMainWnd()->SetWindowText ( pNewTitle );
		}
		else
			theApp.GetMainWnd()->SetWindowText ( CString("Game Guru") );
	}
}

void cLibraryWindow::OnClickList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_Image.OnClickList ( pNMHDR, pResult );

	if ( m_Image.m_bNewPosterSelected )
	{
		CPoster dNewPoster;
		dNewPoster.DoModal( );
		m_Image.Refresh( );
		return;
	}

	int a = strspn( m_szLastPathName, "Entity" );
	int b = strspn( m_szLastPathName, "Prefab" );
	int c = strspn( m_szLastPathName, "Segments" );

	if ( a ) m_iScrollEntity   = m_Image.GetScrollPos ( SB_VERT );
	if ( b ) m_iScrollPrefab   = m_Image.GetScrollPos ( SB_VERT );
	if ( c ) m_iScrollSegments = m_Image.GetScrollPos ( SB_VERT );

	m_Tree.SaveTreeState();

	EndDialog ( 1 );
}

void cLibraryWindow::OnRClickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_Image.OnRClickList( pNMHDR, pResult );
}

void cLibraryWindow::UpdateImageList ( CString sPath )
{
	CSize size;
	size.cx = 0;

	int a = strspn( m_szLastPathName, "Entity" );
	int b = strspn( m_szLastPathName, "Prefab" );
	int c = strspn( m_szLastPathName, "Segments" );

	if ( a ) size.cy = m_iScrollEntity;
	if ( b ) size.cy = m_iScrollPrefab;
	if ( c ) size.cy = m_iScrollSegments;

	// 231107 - Commented out by Paul - solving Spanish scroll bug reported by Rick
	// m_Image.Scroll ( size );
}

void cLibraryWindow::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog ::OnShowWindow(bShow, nStatus);
	DWORD dwWidth = 0;
	DWORD dwHeight = 0;

	RECT rect;
	theApp.m_pEditorView->GetWindowRect ( &rect );

	RECT apprect;
	theApp.m_pMainWnd->GetWindowRect ( &apprect );
	apprect=apprect;

	// V117 - 260410 - now hides output window (my entity/segments), so can use ALL width
	rect.left = apprect.left;

	cIPC* pIPC			= theApp.m_Message.GetIPC ( );
	pIPC->ReceiveBuffer ( &dwWidth,  704, sizeof ( dwWidth  ) );
	pIPC->ReceiveBuffer ( &dwHeight, 708, sizeof ( dwHeight ) );
	SetWindowPos ( NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0 );

	dwWidth = rect.right - rect.left;
	dwHeight = rect.bottom - rect.top;

	int iTreePercent     = 25;
	int iSplitterPercent = 2;
	int iImagePercent    = 70;

	int iMins = 50;

	int iTreeX          = ( dwWidth  / 100 );
	int iTreeY          = ( dwHeight / 100 );
	int iTreeWidth      = ( dwWidth  / 100 ) * iTreePercent;
	int iTreeHeight     = ( rect.bottom - rect.top ) - iMins;
	
	int iSplitterX      = iTreeX + iTreeWidth;
	int iSplitterY      = ( dwHeight / 100  );
	int iSplitterWidth  = 2;
	int iSplitterHeight = ( rect.bottom - rect.top ) - iMins;

	int iImageX         = iTreeX + iTreeWidth + 5;
	int iImageY         = ( dwHeight / 100  );
	int iImageWidth     = dwWidth - iTreeWidth - 20;
	int iImageHeight    = ( rect.bottom - rect.top ) - iMins;

	m_Tree.SetWindowPos     ( NULL, iTreeX, iTreeY, iTreeWidth, iTreeHeight, 0 );
	m_spliter.SetWindowPos  ( NULL, iSplitterX, iSplitterY, iSplitterWidth, iSplitterHeight, 0 );
	m_Image.SetWindowPos    ( NULL, iImageX, iImageY, iImageWidth, iImageHeight, 0 );
	m_Ok.SetWindowPos ( NULL, dwWidth - 260, dwHeight - 35, 0, 0, SWP_NOSIZE );
	m_Cancel.SetWindowPos ( NULL, dwWidth - 140, dwHeight - 35, 0, 0, SWP_NOSIZE );
	
	m_Image.UpdateWindow ( );
}

void cLibraryWindow::OnOK() 
{
	int a = strspn( m_szLastPathName, "Entity" );
	int b = strspn( m_szLastPathName, "Prefab" );
	int c = strspn( m_szLastPathName, "Segments" );
	if ( a ) m_iScrollEntity   = m_Image.GetScrollPos ( SB_VERT );
	if ( b ) m_iScrollPrefab   = m_Image.GetScrollPos ( SB_VERT );
	if ( c ) m_iScrollSegments = m_Image.GetScrollPos ( SB_VERT );
	m_Tree.SaveTreeState();
	CDialog ::OnOK();
}

void cLibraryWindow::OnCancel() 
{
	int a = strspn( m_szLastPathName, "Entity" );
	int b = strspn( m_szLastPathName, "Prefab" );
	int c = strspn( m_szLastPathName, "Segments" );
	if ( a ) m_iScrollEntity   = m_Image.GetScrollPos ( SB_VERT );
	if ( b ) m_iScrollPrefab   = m_Image.GetScrollPos ( SB_VERT );
	if ( c ) m_iScrollSegments = m_Image.GetScrollPos ( SB_VERT );

	m_Tree.SaveTreeState();
	theApp.SetFileMapDataString ( 1512, _T("") );
	
	CDialog ::OnCancel();
}

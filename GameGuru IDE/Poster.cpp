// Poster.cpp : implementation file
//

#include "stdafx.h"
#include "Poster.h"
#include <atlimage.h>
#include "Editor.h"


// CPoster dialog

IMPLEMENT_DYNAMIC(CPoster, CDialog)

CPoster::CPoster(CWnd* pParent /*=NULL*/)
	: CDialog(CPoster::IDD, pParent)
{
	m_bDragging = false;
	m_sCurrentFile = "";
	m_bInit = false;
}

CPoster::~CPoster()
{
}

void CPoster::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, m_ChosenImage);
	DDX_Control(pDX, IDC_EDIT1, m_editCurrentFile);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnMake);
	DDX_Control(pDX, IDC_BUTTON1, m_btnBrowse);
	DDX_Control(pDX, IDC_FILE_TEXT, m_TxtFile);
	DDX_Control(pDX, IDC_WIDTH_TEXT, m_txtWidth);
	DDX_Control(pDX, IDC_HEIGHT_TEXT, m_txtHeight);
	DDX_Control(pDX, IDC_RADIO_WALL, m_RadioWall);
	DDX_Control(pDX, IDC_RADIO_FLOOR, m_RadioFloor);
	DDX_Control(pDX, IDC_RADIO_CEILING, m_RadioCeiling);
	DDX_Control(pDX, IDC_EDIT2, m_editWidth);
	DDX_Control(pDX, IDC_EDIT3, m_editHeight);
	DDX_Control(pDX, IDC_DEPTH_TEXT, m_txtDepth);
	DDX_Control(pDX, IDC_EDIT4, m_editDepth);
	DDX_Control(pDX, IDC_SETTINGS_BOX, m_txtSettingsBox);
	DDX_Control(pDX, IDC_EDIT5, m_editName);
	DDX_Control(pDX, IDC_NAME_TEXT, m_txtName);
	DDX_Control(pDX, IDC_HINT_TEXT, m_txtHint);
}


BEGIN_MESSAGE_MAP(CPoster, CDialog)
	ON_BN_CLICKED(IDOK, &CPoster::OnMake)
	ON_BN_CLICKED(IDC_BUTTON1, &CPoster::OnBrowse)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// CPoster message handlers

BOOL CPoster::OnInitDialog()
{
	if ( !CDialog::OnInitDialog( ) ) return FALSE;

	// 221107 - add translated title text
	SetWindowText ( GetLanguageData ( "Poster Dialog", "NewPoster" ) );

	m_editWidth.SetWindowText( _T("50") );
	m_editHeight.SetWindowText( _T("50") );
	m_editDepth.SetWindowText( _T("1") );
	m_RadioWall.SetCheck( 1 );

	m_TxtFile.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Image") ) );
	m_btnBrowse.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Browse") ) );
	m_txtSettingsBox.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Settings") ) );
	
	m_txtName.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Name") ) );
	m_txtWidth.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Width") ) );
	m_txtHeight.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Height") ) );
	m_txtDepth.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Depth") ) );
	
	m_RadioWall.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Wall") ) );
	m_RadioFloor.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Floor") ) );
	m_RadioCeiling.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Ceiling") ) );

	m_txtHint.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Hint") ) );

	m_btnMake.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Create") ) );
	m_btnCancel.SetWindowText( GetLanguageData( _T("Poster Dialog"), _T("Cancel") ) );

	CString sFolderPath = theApp.m_szDirectory;
	sFolderPath += _T("\\Files\\entitybank\\user\\posters\\");

	for ( int i = 0; i < 100; i++ )
	{
		CString sFile, sFullPath;
		sFile.Format( "Poster%02d*", i );
		sFullPath = sFolderPath + sFile;

		WIN32_FIND_DATA fileInfo;
		HANDLE hFile = FindFirstFile( sFullPath, &fileInfo );
		if ( hFile == INVALID_HANDLE_VALUE )
		{
			sFile.Format( "Poster%02d", i );
			m_editName.SetWindowText( sFile );
			break;
		}
	}

	m_bInit = true;
	return TRUE;
}

void CPoster::OnSizing( UINT nSide, LPRECT lpRect )
{
	if ( lpRect->right - lpRect->left < 355 ) lpRect->right = lpRect->left + 355;
	if ( lpRect->bottom - lpRect->top < 390 ) lpRect->bottom = lpRect->top + 390;
	CDialog::OnSizing( nSide, lpRect );
}

void CPoster::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize( nType, cx, cy );
	if ( !m_bInit ) return;

	m_editCurrentFile.SetWindowPos( NULL, 0,0, cx-156,21, SWP_NOMOVE );
	m_btnBrowse.SetWindowPos( NULL, cx-102,11, 0,0, SWP_NOSIZE );
	
	m_ChosenImage.SetWindowPos( NULL, 0,0, cx-120,cy-106, SWP_NOMOVE );
	
	m_txtSettingsBox.SetWindowPos( NULL, cx-102,41, 0,0, SWP_NOSIZE );
	m_txtName.SetWindowPos( NULL, cx-90,60, 0,0, SWP_NOSIZE );
	m_editName.SetWindowPos( NULL, cx-90,75, 0,0, SWP_NOSIZE );
	m_txtWidth.SetWindowPos( NULL, cx-90,101, 0,0, SWP_NOSIZE );
	m_editWidth.SetWindowPos( NULL, cx-90,116, 0,0, SWP_NOSIZE );
	m_txtHeight.SetWindowPos( NULL, cx-90,142, 0,0, SWP_NOSIZE );
	m_editHeight.SetWindowPos( NULL, cx-90,157, 0,0, SWP_NOSIZE );
	m_txtDepth.SetWindowPos( NULL, cx-90,183, 0,0, SWP_NOSIZE );
	m_editDepth.SetWindowPos( NULL, cx-90,198, 0,0, SWP_NOSIZE );

	m_RadioWall.SetWindowPos( NULL, cx-89,228, 0,0, SWP_NOSIZE );
	m_RadioFloor.SetWindowPos( NULL, cx-89,247, 0,0, SWP_NOSIZE );
	m_RadioCeiling.SetWindowPos( NULL, cx-89,266, 0,0, SWP_NOSIZE );

	m_txtHint.SetWindowPos( NULL, 11,cy-56, 0,0, SWP_NOSIZE );

	m_btnCancel.SetWindowPos( NULL, 11,cy-32, 0,0, SWP_NOSIZE );
	m_btnMake.SetWindowPos( NULL, cx-105,cy-32, 0,0, SWP_NOSIZE );

	m_ChosenImage.CancelDragging( );
	m_ChosenImage.Invalidate( );

	Invalidate();
}

void CPoster::OnLButtonDown( UINT nFlags, CPoint point )
{
	CDialog::OnLButtonDown( nFlags, point );

	CRect rectImage;
	m_ChosenImage.GetWindowRect( &rectImage );
	ScreenToClient( &rectImage );

	if ( rectImage.PtInRect( point ) )
	{
		SetCapture( );
		m_bDragging = true;
		ptStart.x = point.x - rectImage.left;
		ptStart.y = point.y - rectImage.top;

		m_ChosenImage.StartDragging( ptStart.x, ptStart.y );
	}
}

void CPoster::OnLButtonUp( UINT nFlags, CPoint point )
{
	ReleaseCapture( );
	CDialog::OnLButtonUp( nFlags, point );
	if ( !m_bDragging ) return;
	m_bDragging = false;

	//m_ChosenImage.EndDragging( );
	//m_ChosenImage.Invalidate( );
}

void CPoster::OnMouseMove( UINT nFlags, CPoint point )
{
	CDialog::OnMouseMove( nFlags, point );
	if ( !m_bDragging ) return;

	CRect rectImage;
	m_ChosenImage.GetWindowRect( &rectImage );
	ScreenToClient( &rectImage );

	if ( point.x > rectImage.right-2 ) point.x = rectImage.right-2;
	if ( point.x < rectImage.left ) point.x = rectImage.left;
	if ( point.y > rectImage.bottom-2 ) point.y = rectImage.bottom-2;
	if ( point.y < rectImage.top ) point.y = rectImage.top;

	ptEnd.x = point.x - rectImage.left;
	ptEnd.y = point.y - rectImage.top;

	int iTotalWidth = m_ChosenImage.GetZoomedWidth( );
	int iTotalHeight = m_ChosenImage.GetZoomedWidth( );

	int iSelWidth = abs(ptEnd.x - ptStart.x);
	int iSelHeight = abs(ptEnd.y - ptStart.y);
	float fAspect = iSelWidth / (float) iSelHeight;

	int iNewWidth = 0;
	int iNewHeight = 0;
	
	if ( iTotalWidth > iTotalHeight )
	{
		iNewWidth = (iSelWidth*100) / iTotalWidth;
		iNewHeight = (int) (iNewWidth / fAspect);
	}
	else
	{
		iNewHeight = (iSelHeight*100) / iTotalHeight;
		iNewWidth = (int) (iNewHeight * fAspect);
	}

	char str [ 16 ];
	sprintf_s( str, 16, "%d", iNewWidth );
	m_editWidth.SetWindowText( str );
	sprintf_s( str, 16, "%d", iNewHeight );
	m_editHeight.SetWindowText( str );

	m_ChosenImage.ContinueDragging( ptEnd.x, ptEnd.y );
	m_ChosenImage.Invalidate( );
}

void CPoster::OnMake()
{
	CString sWidth, sHeight, sDepth, sName;
	m_editWidth.GetWindowText( sWidth );
	m_editHeight.GetWindowText( sHeight );
	m_editDepth.GetWindowText( sDepth );
	m_editName.GetWindowText( sName );

	if ( sWidth.GetLength( ) < 1 || sHeight.GetLength( ) < 1 || sDepth.GetLength( ) < 1 )
	{
		MessageBox( GetLanguageData( _T("Poster Dialog"), _T("NoSize") ), theApp.m_szErrorString );
		return;
	}

	int iWidth, iHeight, iDepth;
	iWidth = _ttoi( sWidth.GetString() );
	iHeight = _ttoi( sHeight.GetString() );
	iDepth = _ttoi( sDepth.GetString() );

	if ( sName.GetLength( ) < 1 )
	{
		MessageBox( GetLanguageData( _T("Poster Dialog"), _T("NoName") ), theApp.m_szErrorString );
		return;
	}

	if ( sName.FindOneOf( _T("\\/:*\"<>|.") ) >= 0 )
	{
		MessageBox( GetLanguageData( _T("Poster Dialog"), _T("InvalidName") ), theApp.m_szErrorString );
		return;
	}

	CBitmap *pSelection = m_ChosenImage.EndDragging( );
	if ( !pSelection )
	{
		MessageBox( GetLanguageData( _T("Poster Dialog"), _T("NoImage") ), theApp.m_szErrorString );
		return;
	}

	int iType = 0;
	if ( m_RadioFloor.GetCheck( ) ) iType = 1;
	if ( m_RadioCeiling.GetCheck( ) ) iType = 2;

	MakeFPE( sName, iWidth, iHeight, iDepth, iType );
	CImage img;
	img.Attach( *pSelection );
	
	// save texture
	CString sFullPath = theApp.m_szDirectory;
	sFullPath += _T("\\Files\\entitybank\\user\\posters\\");
	sFullPath += sName;
	sFullPath += _T("_D.jpg");
	img.Save( sFullPath );

	// save thumbnail too
	sFullPath = theApp.m_szDirectory;
	sFullPath += _T("\\Files\\entitybank\\user\\posters\\");
	sFullPath += sName;
	sFullPath += _T(".bmp");
	img.Save( sFullPath );

	delete pSelection;
	OnOK();
}

void CPoster::MakeFPE( CString sName, int width, int height, int depth, int type )
{
	//type 0 = wall, 1 = floor, 2 = ceiling
	//name has no file extension or path attached

	// Poster folder
	CString sPosterFolder = theApp.m_szDirectory;
	sPosterFolder += _T("\\Files\\entitybank\\user\\posters\\");

	// Prepare filename
	char pFPEFilename[_MAX_PATH];
	strcpy ( pFPEFilename, sPosterFolder );
	strcat ( pFPEFilename, sName );
	strcat ( pFPEFilename, ".fpe" );

	// Create FPE text file
	DWORD BytesWritten=0;
	char str [ _MAX_PATH ];
	char pCR [ 3 ] = { 13, 10, 0 };
	HANDLE hFile = CreateFile(pFPEFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		wsprintf ( str, ";Entity Spec" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Header" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "desc          = %s", sName ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";AI" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "aiinit        = appear1.fpi" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "aimain	       = default.fpi" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "aidestroy     = disappear1.fpi" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Spawn" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "spawnmax      = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "spawndelay    = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "spawnqty      = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Orientation" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "model         = %s.x", sName ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "offx          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "offy          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "offz          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		if ( type==1 || type==2 )
		{
			wsprintf ( str, "rotx          = 90" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		}
		else
		{
			wsprintf ( str, "rotx          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		}
		wsprintf ( str, "roty          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "rotz          = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "materialindex = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "defaultstatic = 1" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		if ( type==0 )
		{
			// wall
			wsprintf ( str, "forwardfacing = 1" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
			wsprintf ( str, "defaultheight = 50" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		}
		if ( type==1 )
		{
			// floor
			wsprintf ( str, "forwardfacing = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
			wsprintf ( str, "defaultheight = 5" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		}
		if ( type==2 )
		{
			// ceiling
			wsprintf ( str, "forwardfacing = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
			wsprintf ( str, "defaultheight = 95" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		}
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";New hard scaling" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "hardscalex    = %d", width ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "hardscaley    = %d", height ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "hardscalez    = %d", depth ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Visualinfo" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "textured      = %s_D.jpg", sName ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "effect        = effectbank\\bumpent\\bumpent.fx" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Identity Details" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "strength      = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "isimmobile    = 1" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Decals" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "decalmax      = 1" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "decal0        = dust" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, ";Animationinfo" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "animmax       = 0" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		wsprintf ( str, "" ); WriteFile(hFile, str, strlen(str), &BytesWritten, NULL); WriteFile(hFile, pCR, 2, &BytesWritten, NULL);
		CloseHandle(hFile);
		hFile=NULL;
	}

	// Delete any existing model associated with name, and DBO
	char pXFilename[_MAX_PATH];
	strcpy ( pXFilename, sPosterFolder );
	strcat ( pXFilename, sName );
	strcat ( pXFilename, ".dbo" );
	DeleteFile ( pXFilename );
	strcpy ( pXFilename, sPosterFolder );
	strcat ( pXFilename, sName );
	strcat ( pXFilename, ".x" );
	DeleteFile ( pXFilename );

	// Name of 
	char pOriginalPosterFilename[_MAX_PATH];
	strcpy ( pOriginalPosterFilename, sPosterFolder );
	strcat ( pOriginalPosterFilename, "poster.x" );

	// Copy poster.x as new model for this entity
	CopyFile ( pOriginalPosterFilename, pXFilename, FALSE );

	// Also copy over _N and _S from poster originals too
	char pExtraTextureFilename[_MAX_PATH];
	strcpy ( pExtraTextureFilename, sPosterFolder );
	strcat ( pExtraTextureFilename, sName );
	strcat ( pExtraTextureFilename, "_N.dds" );
	strcpy ( pOriginalPosterFilename, sPosterFolder );
	strcat ( pOriginalPosterFilename, "poster_N.dds" );
	CopyFile ( pOriginalPosterFilename, pExtraTextureFilename, FALSE );
	strcpy ( pExtraTextureFilename, sPosterFolder );
	strcat ( pExtraTextureFilename, sName );
	strcat ( pExtraTextureFilename, "_S.dds" );
	strcpy ( pOriginalPosterFilename, sPosterFolder );
	strcat ( pOriginalPosterFilename, "poster_S.dds" );
	CopyFile ( pOriginalPosterFilename, pExtraTextureFilename, FALSE );
}

void CPoster::OnBrowse()
{
	TCHAR str [ 256 ];
	_tcscpy_s( str, 256, GetLanguageData( _T("Poster Dialog"), _T("Images") ) );
	_tcscat_s( str, 256, " (*.bmp, *.jpg, *.png)|*.bmp; *.jpg; *.png||" );

	CFileDialog dFile( TRUE, _T(".bmp"), NULL, OFN_FILEMUSTEXIST, str );
	int result = dFile.DoModal( );
	if ( result != IDOK ) return;

	m_sCurrentFile = dFile.GetPathName( );

	CImage image;
	if ( FAILED(image.Load( m_sCurrentFile )) )
	{
		MessageBox( GetLanguageData( _T("Poster Dialog"), _T("FailedImage") ), theApp.m_szErrorString );
		m_sCurrentFile = "";
		return;
	}

	CWindowDC wndDC( this );
	CDC dc;
	dc.CreateCompatibleDC( &wndDC );

	CBitmap bmp;
	bmp.CreateBitmap( image.GetWidth(),image.GetHeight(), 1, 32, NULL );
	CBitmap *oldBmp = dc.SelectObject( &bmp );

	::SetStretchBltMode( image.GetDC(), HALFTONE );
	::SetBrushOrgEx( image.GetDC(), 0,0, NULL );
	image.BitBlt( dc.GetSafeHdc( ), 0,0 );
	dc.SelectObject( oldBmp );
	m_ChosenImage.CopyBitmap( &bmp );

	image.ReleaseDC();
	image.ReleaseDC();

	m_editCurrentFile.SetWindowText( m_sCurrentFile.GetString( ) );
}

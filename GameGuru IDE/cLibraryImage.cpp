// cLibraryImage.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "cLibraryImage.h"
#include "windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Globals for blacklist string array
LPSTR* g_pBlackList = NULL;
int g_iBlackListMax = 0;

cLibraryImage::cLibraryImage ( )
{
	m_bNewPosterSelected = false;
	sLastPath = "";
	m_bPosterMode = false;
}

cLibraryImage::~cLibraryImage ( )
{
}


BEGIN_MESSAGE_MAP ( cLibraryImage, CListCtrl )
	//{{AFX_MSG_MAP(cLibraryImage)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLLCLIPBOARD()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID__DELETEPOSTER, &cLibraryImage::OnDeletePoster)
END_MESSAGE_MAP ( )

int cLibraryImage::Start ( void )
{
	m_Images.Create     ( 64, 64, ILC_COLORDDB, 1, 16 );
	m_Images.SetBkColor ( RGB ( 255, 255, 255 ) );
		
	this->DeleteAllItems ( );
	this->SetImageList   ( &m_Images, LVSIL_NORMAL );
	this->SetItemCount   ( 64 );

	// resets
	m_dwImageCount = 0;
	m_szImageFileNames = NULL;
	m_szImageExtra = NULL;

	return 1;
}

#include "cDialogEnd.h"
extern int g_iBoysLifeQuit;

void cLibraryImage::OnClickList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	m_bNewPosterSelected = false;

	// retrieve message info.
	LPNMITEMACTIVATE pItemAct = ( LPNMITEMACTIVATE ) pNMHDR;

	// determine which item receives the click
	LVHITTESTINFO  hitTest;

	// determine item clicked on
	memset ( &hitTest, '\0', sizeof ( LVHITTESTINFO ) );
	hitTest.pt = pItemAct->ptAction;
	this->SendMessage ( LVM_SUBITEMHITTEST, 0, ( LPARAM ) &hitTest );

	// reset strings
	theApp.SetFileMapDataString ( 1512, "" );
	theApp.SetFileMapDataString ( 1256, "" );

	if ( hitTest.iItem > -1 )
	{
		if ( _tcscmp( m_szImageExtra [ hitTest.iItem ], GetLanguageData( _T("Poster Dialog"), _T("NewItem") ) ) == 0 )
		{
			m_bNewPosterSelected = true;
			return;
		}
	}

	// fill strings if item valid
	if ( hitTest.iItem > -1 )
	{
		// get file only
		bool bItemInvalid=false;
		int iLength = strlen( m_szImageFileNames [ hitTest.iItem ] );
		for ( int n=iLength; n>0; n-- )
		{
			if ( m_szImageFileNames [ hitTest.iItem ][n]=='\\' || m_szImageFileNames [ hitTest.iItem ][n]=='/' )
			{
				// set invalid if underscore here
				if ( m_szImageFileNames [ hitTest.iItem ][n+1]=='_' ) bItemInvalid=true;
				break;
			}
		}

		
		#ifdef FPSC_DEMO
		if ( bItemInvalid )
		{
			cDialogEnd end;
			g_iBoysLifeQuit = 1;
			end.DoModal ( );
			g_iBoysLifeQuit = 0;
		}
		#endif

		// if string starts with _ underscore, it is a NULL item and should not fill strings
		if ( bItemInvalid==false )
		{
			
			
			theApp.SetFileMapDataString ( 1512, m_szImageFileNames [ hitTest.iItem ] );
			theApp.SetFileMapDataString ( 1256, m_szImageExtra [ hitTest.iItem ] );

		}
	}
}

void cLibraryImage::OnRClickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	if ( !m_bPosterMode ) return;
	
	LPNMITEMACTIVATE pItemAct = ( LPNMITEMACTIVATE ) pNMHDR;

	// determine which item receives the click
	LVHITTESTINFO  hitTest;

	// determine item clicked on
	memset ( &hitTest, '\0', sizeof ( LVHITTESTINFO ) );
	hitTest.pt = pItemAct->ptAction;
	this->SendMessage ( LVM_SUBITEMHITTEST, 0, ( LPARAM ) &hitTest );

	ClientToScreen( &hitTest.pt );

	if ( hitTest.iItem > -1 )
	{
		if ( _tcscmp( GetItemText( hitTest.iItem,0 ), GetLanguageData(_T("Poster Dialog"), _T("NewItem")) ) == 0 ) return;

		m_iDeletingItem = hitTest.iItem;

		//theApp.ShowPopupMenu (IDR_POSTER_MENU, hitTest.pt, this);
		CMenu contextMenu;
		CMenu *pPopupMenu;
		
		contextMenu.LoadMenu( IDR_POSTER_MENU );
		pPopupMenu = contextMenu.GetSubMenu( 0 );

		CString sDelete = GetLanguageData( _T("Poster Dialog"), _T("Delete") );
		if ( sDelete.Compare( _T("") ) == 0 ) sDelete = _T("Delete Poster");

		MENUITEMINFO info = {0};
		info.cbSize = sizeof( MENUITEMINFO );
		info.fMask = MIIM_STRING;
		info.fState = MFS_ENABLED;
		info.fType = MIIM_STRING;
		info.dwTypeData = sDelete.GetBuffer( );

		pPopupMenu->SetMenuItemInfo( 0, &info, TRUE );
		pPopupMenu->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, hitTest.pt.x , hitTest.pt.y , this ); 
	}
}

extern int m_iScroll;

struct sFileList
{
	char	szFullPath	  [ MAX_PATH ];
	char	szDisplayName [ MAX_PATH ];
	char	szFileName    [ MAX_PATH ];
	char	szImageName   [ MAX_PATH ];
	char	szExtension   [ 4 ];
};

vector < sFileList > g_FileList;
vector < LONGLONG > g_FileListDate;
TCHAR g_szPurchasedDirectory [ MAX_PATH ];

// Limit hot many items in RECENT list
int recentFolderFileCount = 0;

void cLibraryImage::LoadImagesRec ( CString sPurchasedDir )
{	
	// Vars for filemap comms
	CString string = "FPSEXCHANGE";
	char szBlankImage [ 255 ];
	cIPC* pIPC = theApp.m_Message.GetIPC ( );
	memset ( szBlankImage, 0, sizeof ( szBlankImage ) );
	pIPC->ReceiveBuffer ( &szBlankImage [ 0 ], 2024, sizeof ( char ) * 255 );

	// add any files in here (recursive calls itself)
	BOOL bRC = TRUE;
	WIN32_FIND_DATA FindFileData;
	CString strPattern = sPurchasedDir + "\\*.*";
	HANDLE hFind = ::FindFirstFile(strPattern, &FindFileData);
	while(bRC)
	{
		// grab a file
		bRC = ::FindNextFile(hFind, &FindFileData);
		if(bRC && recentFolderFileCount < 100 )
		{
			// temp file item to define
			sFileList file;

			// filter off the system files and directories
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  &&
				!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)     &&
				!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)     &&
				!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY))
			{
				// add display name to item entry
				LPSTR pNameOnly = FindFileData.cFileName;
				strcpy ( file.szExtension, "" );
				memset ( file.szDisplayName, 0, sizeof ( file.szDisplayName ) );
				if ( strlen(pNameOnly) > 4 )
				{
					strcpy ( file.szDisplayName, pNameOnly );
					file.szDisplayName [ strlen(pNameOnly) - 4 ] = 0;
					strncpy ( file.szExtension, pNameOnly + strlen(pNameOnly) - 3, 4 );
				}

				// can only be FPE
				if ( strcmp ( file.szExtension, "fpe" )==NULL )
				{
					// absolute path to entity
					strcpy ( file.szFullPath, sPurchasedDir );
					strcat ( file.szFullPath, "\\" );
					strcat ( file.szFullPath, FindFileData.cFileName );

					// just file
					strcpy ( file.szFileName, FindFileData.cFileName );

					// create absolute path to thumbnail image file
					strcpy ( file.szImageName, sPurchasedDir );
					strcat ( file.szImageName, "\\" );
					strcat ( file.szImageName, file.szDisplayName );
					strcat ( file.szImageName, ".bmp" );

					// get date value if file exists
					LONGLONG dateValue = 0;
					WIN32_FILE_ATTRIBUTE_DATA fileInfo;
					if ( GetFileAttributesEx(file.szImageName, GetFileExInfoStandard, &fileInfo) )
					{
						dateValue = ((LARGE_INTEGER*)&fileInfo.ftCreationTime)->QuadPart;
					}

					// open the image file to see if exists, if not, use blank
					FILE* fp = fopen ( file.szImageName, "rb" );
					if ( !fp ) strcpy ( file.szImageName, szBlankImage );
					if ( fp ) fclose ( fp );

					// Limit how many files as it crashes with too many
					++recentFolderFileCount;

					// finally add to view
					g_FileList.push_back ( file );
					g_FileListDate.push_back ( dateValue );
				}
			}
			else
			{
				if ( !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)  &&
				     !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)  &&
				     !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) )
				{
					if ( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						// nest into folder and do it all again inside there (recursive)
						if ( strcmp ( FindFileData.cFileName, ".")!=NULL && strcmp ( FindFileData.cFileName, "..")!=NULL )
							LoadImagesRec ( sPurchasedDir + "\\" + FindFileData.cFileName );
					}
				}
			}
		}  
		else
		{
			// eat last error
			DWORD err = ::GetLastError ( );
		}
	}

	// finished scanning regular files
	::FindClose ( hFind );
}

void cLibraryImage::LoadImages ( CString sPath )
{
	// Create blacklist near where we are going to use it (below)
	if ( g_pBlackList == NULL && theApp.IsParentalControlActive()==true )
	{
		// first pass count, second one writes into array
		for ( int iPass = 0; iPass < 2; iPass++ )
		{
			// reset count
			g_iBlackListMax = 0;

			// open blacklist file and go through all contents
			FILE* fp = fopen ( "parentalcontrolblacklist.ini", "rt" );
			if ( fp )
			{
				char c;
				fread ( &c, sizeof ( char ), 1, fp );
				while ( !feof ( fp ) )
				{
					// get string from file
					char szEntNameFromFile [ MAX_PATH ] = "";
					int iOffset = 0;
					while ( !feof ( fp ) && c!=13 && c!=10 )
					{
						szEntNameFromFile [ iOffset++ ] = c;
						fread ( &c, sizeof ( char ), 1, fp );
					}
					szEntNameFromFile [ iOffset ] = 0;

					// skip beyond CR
					while ( !feof ( fp ) && (c==13 || c==10) )
						fread ( &c, sizeof ( char ), 1, fp );

					// count or write
					if ( iPass==0 )
					{
						// count
						g_iBlackListMax++;
					}
					else
					{
						// write into array
						g_pBlackList[g_iBlackListMax] = new char[512];
						strlwr ( szEntNameFromFile );
						strcpy ( g_pBlackList[g_iBlackListMax], szEntNameFromFile );
						g_iBlackListMax++;
					}
				}
				fclose ( fp );
			}
			
			// at end, create dynamic string array
			if ( iPass==0 ) g_pBlackList = new LPSTR[g_iBlackListMax];
		}
	}

	// Vars for filemap comms
	CString string = "FPSEXCHANGE";
	char szBlankImage [ 255 ];
	char szTokens [ 10 ] [ 255 ];
	int  iToken = 0;
	char szFilter [ 255 ];
	cIPC* pIPC = theApp.m_Message.GetIPC ( );
	memset ( szFilter,     0, sizeof ( szFilter     ) );
	memset ( szTokens,     0, sizeof ( szTokens     ) );
	memset ( szBlankImage, 0, sizeof ( szBlankImage ) );
	pIPC->ReceiveBuffer ( &szFilter     [ 0 ], 1768, sizeof ( char ) * 255 );
	pIPC->ReceiveBuffer ( &szBlankImage [ 0 ], 2024, sizeof ( char ) * 255 );

	// legacy poster mode (active if user\posters folder detected)
	bool bIsPosterFolder = false;
	m_bPosterMode = false;
	if ( sPath.Find( "user\\posters" ) >= 0 )
	{
		bIsPosterFolder = true;
		m_bPosterMode = true;
	}

	// reset page to zero images and no filenames
	m_iImageCount = 0;
	//memset ( m_szImageFileNames, 0, sizeof ( m_szImageFileNames ) );
	if ( m_szImageExtra != NULL )
	{
		for ( DWORD dwI=0; dwI<m_dwImageCount; dwI++ )
			if ( m_szImageExtra[dwI] != NULL )
				delete m_szImageExtra[dwI];
		delete m_szImageExtra;
	}
	if ( m_szImageFileNames != NULL )
	{
		for ( DWORD dwI=0; dwI<m_dwImageCount; dwI++ )
			if ( m_szImageFileNames[dwI] != NULL )
				delete m_szImageFileNames[dwI];
		delete m_szImageFileNames;
	}
	m_dwImageCount = 0;

	// get current folder in TCHAR and ASC (remove \ at end)
	TCHAR szDirectory [ MAX_PATH ];
	GetCurrentDirectory ( MAX_PATH, szDirectory );
	char szFullPath [ MAX_PATH ];
	int iLen = strlen ( sPath );
	strcpy ( szFullPath, sPath );
	if ( szFullPath [ iLen - 1 ] == '\\' && szFullPath [ iLen - 2 ] == '\\' )
		szFullPath [ iLen - 1 ] = 0;

	// Ensure we are in the correct entity folder
	SetCurrentDirectory ( sPath );

	// determine if regular file items or RECENT file items (full date ordered list)
	g_FileList.clear();
	g_FileListDate.clear();
	if ( sPath.Right(17).MakeLower()==CString("purchased\\recent\\") )
	{
		// file count cap as too many is causing a crash for some users
		recentFolderFileCount = 0;
		// step back into the main PURCHASED folder (all of it)
		SetCurrentDirectory ( sPath + "\\.." );
		GetCurrentDirectory ( MAX_PATH, g_szPurchasedDirectory );
		CString sPurchasedDir = CString(g_szPurchasedDirectory);

		// pattern to get ALL files in PURCHASED folders
		LoadImagesRec ( sPurchasedDir );

		// now sort the list 
		for ( int iI=0; iI<g_FileListDate.size(); iI++ )
		{
			for ( int iJ=0; iJ<(int)g_FileListDate.size(); iJ++ )
			{
				if ( iI < iJ )
				{
					if ( g_FileListDate[iI] < g_FileListDate[iJ] )
					{
						sFileList storeF = g_FileList[iI];
						LONGLONG storeD = g_FileListDate[iI];
						g_FileList[iI] =  g_FileList[iJ];
						g_FileListDate[iI] = g_FileListDate[iJ];
						g_FileList[iJ] = storeF;
						g_FileListDate[iJ] = storeD;
					}
				}
			}
		}
	}
	else
	{
		// Can add ALL files when no filter needed
		bool bAddAllFiles = false;
		if ( strcmp ( szFilter, "*.*" ) == 0 ) bAddAllFiles = true;
		if ( bAddAllFiles==false )
		{
			// grab filter string (comma delimited)
			char* token;
			char  seps[ ] = ",";
			token = strtok ( szFilter, seps );
			while ( token != NULL )
			{
				strcpy ( szTokens [ iToken++ ], token );
				token = strtok ( NULL, seps );
			}
		}
	
		// scan all files in this entity folder
		CString					strPath, strPattern;
		CFile					ImgFile;	
		BYTE					data[8] = {'\0'};	
		BOOL					bRC = TRUE;
		HANDLE					hFind = NULL;
		WIN32_FIND_DATA			FindFileData;
		CString					FileNames [ 256 ];
		int						iIndex = 0;
		
		// pattern to get ALL files
		strPattern = sPath + "\\*.*";
		hFind = ::FindFirstFile(strPattern, &FindFileData);
		while(bRC)
		{
			// grab a file
			bRC = ::FindNextFile(hFind, &FindFileData);
			if(bRC)
			{
				// temp file item to define
				sFileList file;

				// filter off the system files and directories
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  &&
					!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)     &&
					!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)     &&
					!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY))
				{
					// 050416 - if in parental mode, scan to ensure entity is not blacklisted
					bool bAddThisEntity = true;
					if ( g_pBlackList != NULL )
					{
						char pThisEntityFilename[512];
						strcpy ( pThisEntityFilename, FindFileData.cFileName );
						strlwr ( pThisEntityFilename );
						for ( int iBlackListIndex=0; iBlackListIndex<g_iBlackListMax; iBlackListIndex++ )
						{
							if ( strncmp ( g_pBlackList[iBlackListIndex], pThisEntityFilename, strlen(pThisEntityFilename)-4 )==NULL )
							{
								// this entity has been banned by parents
								bAddThisEntity = false;
								break;
							}
						}
					}

					// whether to add or not
					if ( bAddThisEntity == true )
					{
						// work on file to get name
						strPath = sPath + "\\" + FindFileData.cFileName;
						char szFile [ MAX_PATH ];
						char szExt [ 4 ];
						strcpy ( szFile, FindFileData.cFileName );
						memset ( szExt, 0, sizeof ( szExt ) );
						if ( strlen(szFile) > 4 )
						{
							szExt [ 0 ] = szFile [ strlen ( szFile ) - 3 ];
							szExt [ 1 ] = szFile [ strlen ( szFile ) - 2 ];
							szExt [ 2 ] = szFile [ strlen ( szFile ) - 1 ];
						}

						// add display name to item entry
						memset ( file.szDisplayName, 0, sizeof ( file.szDisplayName ) );
						if ( strlen(szFile) > 4 )
							memcpy ( file.szDisplayName, szFile, sizeof ( char ) * ( strlen ( szFile ) - 4 ) );
						else
							strcpy ( file.szDisplayName, szFile );
						strcpy ( file.szFullPath, szFullPath );

						// choose whether to add (based on filter above)
						bool bAdd = true;
						if ( bAddAllFiles==false )
						{
							bAdd = false;
							for ( int i = 0; i < iToken; i++ )
								if ( strcmp ( szExt, szTokens [ i ] ) == 0 )
									bAdd = true;
						}
						if ( bAdd )
						{
							// we should add this item to the view
							strcpy ( file.szFileName,  szFile );
							strcpy ( file.szExtension, szExt );
							if ( strcmp ( szExt, "bmp" ) == 0 )
							{
								// thumbnail
								strcpy ( file.szImageName, szFile );
							}
							else
							{
								// no thumbnail, so assume one
								strcpy ( file.szImageName, szBlankImage );
								strcpy ( file.szImageName,file.szDisplayName );
								strcat ( file.szImageName, ".bmp" );

								// open the image file to see if exists, if not, use blank
								FILE* fp = fopen ( file.szImageName, "rb" );
								if ( !fp ) strcpy ( file.szImageName, szBlankImage );
								if ( fp ) fclose ( fp );
							}

							// add final name to full path
							strcat ( file.szFullPath, file.szFileName );

							// loc file system stores alternative entity names elsewhere
							char szOriginalPath [ MAX_PATH ] = "";
							GetCurrentDirectory ( MAX_PATH, szOriginalPath );
							SetCurrentDirectory ( theApp.m_szDirectory );
							char szLOCFile [ MAX_PATH ] = "";
							char szLOCFull [ MAX_PATH ] = "";
							for ( int i = 0; i < (int)strlen ( file.szFileName ); i++ )
							{
								if ( file.szFileName [ i ] == '.' ) break;
								szLOCFile [ i ] = file.szFileName [ i ];
							}
							strcat ( szLOCFile, ".loc" );

							// get the corresponding loc file path
							char szItem [ 255 ] [ MAX_PATH ];
							char  seps[ ] = "\\/";
							char* token;
							int iToken = 0;
							int iFileIndex = 0;
							char szPath [ MAX_PATH ] = "";
							strcpy ( szPath, file.szFullPath );
							token = strtok ( szPath, seps );
							while ( token != NULL )
							{
								_strlwr ( token );
								if ( strcmp ( token, "files" ) == 0 ) iFileIndex = iToken;
								strcpy ( &szItem [ iToken++ ] [ 0 ], token );
								token = strtok ( NULL, seps );
							}
							strcpy ( szLOCFull, theApp.m_szLanguagePath );
							strcat ( szLOCFull, "/textfiles/library/" );
							for ( i = iFileIndex + 1; i < iToken - 1; i++ )
							{
								strcat ( szLOCFull, szItem [ i ] );
								strcat ( szLOCFull, "/" );
							}
							strcat ( szLOCFull, szLOCFile );

							// now open this file and extract the text
							FILE* fp = fopen ( szLOCFull, "rt" );
							if ( fp )
							{
								char szData [ MAX_PATH ] = "";
								int iOffset = 0;
								char c;
								while ( !feof ( fp ) )
								{
									fread ( &c, sizeof ( char ), 1, fp );
									szData [ iOffset++ ] = c;
								}
								szData [ strlen ( szData ) - 1 ] = 0;
								szData [ strlen ( szData ) - 1 ] = 0;
								fclose ( fp );
								strcpy ( file.szDisplayName, szData );
							}
							
							// restore to original entity folder
							SetCurrentDirectory ( szOriginalPath );

							// finally add to view
							g_FileList.push_back ( file );
							g_FileListDate.push_back ( (LONGLONG)0 );
						}
					}
				}
			}  
			else
			{
				// eat last error
				DWORD err = ::GetLastError ( );
			}
		}

		// finished scanning regular files
		::FindClose ( hFind );
	}

	// Ensure we are in the correct entity folder
	SetCurrentDirectory ( sPath );

	// clear view and set defaults
	m_Images.DeleteImageList ( );
	m_Images.Create     ( 64, 64, ILC_COLORDDB, 1, 16 );
	m_Images.SetBkColor ( RGB ( 255, 255, 255 ) );
	this->DeleteAllItems ( );
	this->SetImageList ( &m_Images, LVSIL_NORMAL );

	// 130416 - create correctly sized string arrays
	m_dwImageCount = g_FileList.size ( );
	m_szImageExtra = new TCHAR*[m_dwImageCount];
	for ( DWORD dwI=0; dwI<m_dwImageCount; dwI++ )
	{
		m_szImageExtra[dwI] = new TCHAR[256];
		memset ( m_szImageExtra[dwI], 0, sizeof(TCHAR)*256 );
	}
	m_szImageFileNames = new TCHAR*[m_dwImageCount];
	for ( DWORD dwI=0; dwI<m_dwImageCount; dwI++ )
	{
		m_szImageFileNames[dwI] = new TCHAR[256];
		memset ( m_szImageFileNames[dwI], 0, sizeof(TCHAR)*256 );
	}

	// addition vars
	int iA = 0, iB = 0;

	// go through all files in list we added above
	for ( int i = 0; i < (int)g_FileList.size ( ); i++ )
	{
		// get file details
		CString bitmapLoad = g_FileList [ i ].szImageName;
		CString name       = g_FileList [ i ].szDisplayName;
		CString filename   = g_FileList [ i ].szFullPath;
		strcpy ( m_szImageExtra [ m_iImageCount ], name );
		strcpy ( m_szImageFileNames [ m_iImageCount++ ], filename );

		// load thumbnail image
		CBitmap	bitmap;
		HANDLE hImage = ::LoadImage ( NULL, bitmapLoad, IMAGE_BITMAP, 64, 64, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_CREATEDIBSECTION );
		if ( !bitmap.Attach ( hImage ) )
		{
			theApp.m_Debug.Write ( "cLibraryImage::LoadImages - failed to attach image" );
		}
		m_Images.Add ( &bitmap, RGB ( 0, 0, 0 ) );
		bitmap.DeleteObject();

		// add this item to the view
		this->InsertItem ( iA++, name, iB++ );
	}

	// poster system
	if ( bIsPosterFolder )
	{
		CBitmap bmp;
		bmp.LoadBitmap( IDB_ADD_POSTER );
		m_Images.Add ( &bmp, RGB ( 0, 0, 0 ) );
		this->InsertItem ( iA++, GetLanguageData( _T("Poster Dialog"), _T("NewItem") ), iB++ );
		strcpy ( m_szImageExtra [ m_iImageCount ], GetLanguageData( _T("Poster Dialog"), _T("NewItem") ) );
		strcpy ( m_szImageFileNames [ m_iImageCount++ ], _T("") );
	}

	// record last path for next pass
	sLastPath = sPath;
}

void cLibraryImage::Refresh( )
{
	if ( sLastPath.GetLength( ) <= 1 ) return;

	LoadImages( sLastPath );
}

void cLibraryImage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl ::OnHScroll(nSBCode, nPos, pScrollBar);
}

void cLibraryImage::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl ::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL cLibraryImage::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CListCtrl ::OnMouseWheel(nFlags, zDelta, pt);
}

void cLibraryImage::OnVScrollClipboard(CWnd* pClipAppWnd, UINT nSBCode, UINT nPos) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl ::OnVScrollClipboard(pClipAppWnd, nSBCode, nPos);
}

void cLibraryImage::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl ::OnMouseMove(nFlags, point);
}

void cLibraryImage::OnDeletePoster()
{
	int result = MessageBox( GetLanguageData( _T("Poster Dialog"), _T("ConfirmDelete") ), GetLanguageData( _T("Poster Dialog"), _T("Confirm") ), MB_YESNO );
	if ( result != IDYES ) return;

	CString sItemName = GetItemText( m_iDeletingItem, 0 );

	CString sPosterFile = theApp.m_szDirectory;
	sPosterFile += _T("\\Files\\entitybank\\user\\posters\\");
	sPosterFile += sItemName;

	CString sFile = sPosterFile + _T(".bin");
	DeleteFile( sFile );
	sFile = sPosterFile + _T(".bmp");
	DeleteFile( sFile );
	sFile = sPosterFile + _T(".dbo");
	DeleteFile( sFile );
	sFile = sPosterFile + _T(".fpe");
	DeleteFile( sFile );
	sFile = sPosterFile + _T(".x");
	DeleteFile( sFile );
	sFile = sPosterFile + _T("_D.jpg");
	DeleteFile( sFile );
	sFile = sPosterFile + _T("_D.dds");
	DeleteFile( sFile );
	sFile = sPosterFile + _T("_N.dds");
	DeleteFile( sFile );
	sFile = sPosterFile + _T("_S.dds");
	DeleteFile( sFile );

	Refresh( );
}

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Editor.h"
#include "MainFrm.h"
#include "Splash.h"
#include "cTestGame.h"
#include "cBuildGame.h"
#include "cPreferences.h"
#include "cFPIWizard.h"
#include "cCredits.h"
#include "HelpWizard.h"
#include "AutoUpdate.h"	//AutoUpdate
#include "UpdateCheckThread.h"
//#include "TGC Store\MarketWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Externals
extern bool g_bFreeVersion;
extern bool g_bX9InterfaceMode;
extern bool g_bSantitizedTeenKidFriendly;
extern bool g_bNetBookVersion;
extern bool g_bInTestGame;

// Global
DWORD	g_dwToggleRestartManagerFlag	= 0;
bool g_bAlsoRemoveRestrictedContentToggle = false;

// access to flag whether we are standalone or not
extern bool g_bStandaloneNoNetMode;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)


BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_TIMER()	//AutoUpdate
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomIn)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomOut)
	ON_COMMAND(ID_VIEW_INCREASE_SHROUD, OnViewIncreaseShroud)
	ON_COMMAND(ID_VIEW_DECREASE_SHROUD, OnViewDecreaseShroud)
	ON_COMMAND(ID_VIEW_LAYERS, OnViewLayers)
	ON_COMMAND(ID_VIEW_MOVEUP_LAYER, OnViewMoveUpLayer)
	ON_COMMAND(ID_VIEW_MOVEDOWN_LAYER, OnViewMoveDownLayer)
	ON_COMMAND(ID_VIEW_OVERVIEW, OnViewOverview)
	ON_COMMAND(ID_VIEW_CLOSEUP, OnViewCloseup)
	ON_COMMAND(ID_MODE_PAINT, OnModePaint)
	ON_COMMAND(ID_MODE_SELECT, OnModeSelect)
	ON_COMMAND(ID_MODE_ART, OnModeArt)
	ON_COMMAND(ID_MODE_ENTITY, OnModeEntity)
	ON_COMMAND(ID_MODE_WAYPOINT, OnModeWaypoint)
	ON_COMMAND(ID_PROPERTY_ROTATE, OnPropertyRotate)
	ON_COMMAND(ID_PROPERTY_MIRROR, OnPropertyMirror)
	ON_COMMAND(ID_PROPERTY_FLIP, OnPropertyFlip)
	ON_COMMAND(ID_DRAW_A, OnDrawA)
	ON_COMMAND(ID_DRAW_B, OnDrawB)
	ON_COMMAND(ID_DRAW_C, OnDrawC)
	ON_COMMAND(ID_DRAW_D, OnDrawD)
	ON_COMMAND(ID_DRAW_STATE_NONE, OnDrawStateNone)
	ON_COMMAND(ID_DRAW_PICK, OnDrawPick)
	ON_COMMAND(ID_SELECTION_CUT, OnSelectionCut)
	ON_COMMAND(ID_SELECTION_COPY, OnSelectionCopy)
	ON_COMMAND(ID_SELECTION_PASTE, OnSelectionPaste)
	ON_COMMAND(ID_SELECTION_CLEAR, OnSelectionClear)
	ON_COMMAND(ID_SELECTION_CLEAR_NOPASTE, OnSelectionClearNopaste)
	ON_COMMAND(ID_SELECTION_REPLACE, OnSelectionReplace)
	ON_COMMAND(ID_TERRAIN_RAISE, OnTerrainRaise)
	ON_COMMAND(ID_TERRAIN_LEVEL, OnTerrainLevel)
	ON_COMMAND(ID_TERRAIN_STORE, OnTerrainStore)
	ON_COMMAND(ID_TERRAIN_BLEND, OnTerrainBlend)
	ON_COMMAND(ID_TERRAIN_RAMP, OnTerrainRamp)
	ON_COMMAND(ID_TERRAIN_TEXTURE, OnTerrainGround)
	ON_COMMAND(ID_TERRAIN_MUD, OnTerrainMud)
	ON_COMMAND(ID_TERRAIN_SEDIMENT, OnTerrainSediment)
	ON_COMMAND(ID_TERRAIN_ROCK, OnTerrainRock)
	ON_COMMAND(ID_TERRAIN_GRASS, OnTerrainGrass)
	ON_COMMAND(ID_ENTITY_DELETE, OnEntityDelete)
	ON_COMMAND(ID_ENTITY_ROTATE_X, OnEntityRotateX)
	ON_COMMAND(ID_ENTITY_ROTATE_Y, OnEntityRotateY)
	ON_COMMAND(ID_ENTITY_ROTATE_Z, OnEntityRotateZ)
	ON_COMMAND(ID_WAYPOINT_CREATE, OnWaypointCreate)
	ON_COMMAND(ID_VIEW_ROTATE, OnViewRotate)
	ON_COMMAND(ID_VIEW_PREFAB, OnViewPrefab)
	ON_COMMAND(ID_VIEW_SELECTION, OnViewSelection)
	ON_COMMAND(ID_VIEW_DRAW, OnViewDraw)
	ON_COMMAND(ID_VIEW_ROTATE_ENTITY, OnViewRotateEntity)
	ON_COMMAND(ID_VIEW_WAYPOINT, OnViewWaypoint)
	ON_COMMAND(ID_VIEW_TEST_GAME, OnViewTestGame)
	ON_COMMAND(ID_TEST_GAME, OnTestGame)
	ON_COMMAND(ID_MULTIPLAYER_GAME, OnMultiplayerGame)
	ON_UPDATE_COMMAND_UI(ID_TEST_MAP, OnUpdateTestMap)
	ON_UPDATE_COMMAND_UI(ID_TEST_GAME, OnUpdateTestGame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomIn)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomOut)
	ON_UPDATE_COMMAND_UI(ID_VIEW_INCREASE_SHROUD, OnUpdateViewIncreaseShroud)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DECREASE_SHROUD, OnUpdateViewDecreaseShroud)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LAYERS, OnUpdateViewLayers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVEUP_LAYER, OnUpdateViewMoveupLayer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVEDOWN_LAYER, OnUpdateViewMovedownLayer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OVERVIEW, OnUpdateViewOverview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CLOSEUP, OnUpdateViewCloseUp)
	ON_UPDATE_COMMAND_UI(ID_MODE_PAINT, OnUpdateModePaint)
	ON_UPDATE_COMMAND_UI(ID_MODE_SELECT, OnUpdateModeSelect)
	ON_UPDATE_COMMAND_UI(ID_MODE_ART, OnUpdateModeArt)
	ON_UPDATE_COMMAND_UI(ID_MODE_ENTITY, OnUpdateModeEntity)
	ON_UPDATE_COMMAND_UI(ID_MODE_WAYPOINT, OnUpdateModeWaypoint)
	ON_UPDATE_COMMAND_UI(ID_WAYPOINT_SELECT, OnUpdateModeWaypointSelect)
	ON_UPDATE_COMMAND_UI(ID_PROPERTY_ROTATE, OnUpdatePropertyRotate)
	ON_UPDATE_COMMAND_UI(ID_PROPERTY_MIRROR, OnUpdatePropertyMirror)
	ON_UPDATE_COMMAND_UI(ID_PROPERTY_FLIP, OnUpdatePropertyFlip)
	ON_UPDATE_COMMAND_UI(ID_DRAW_A, OnUpdateDrawA)
	ON_UPDATE_COMMAND_UI(ID_DRAW_B, OnUpdateDrawB)
	ON_UPDATE_COMMAND_UI(ID_DRAW_C, OnUpdateDrawC)
	ON_UPDATE_COMMAND_UI(ID_DRAW_D, OnUpdateDrawD)
	ON_UPDATE_COMMAND_UI(ID_DRAW_PICK, OnUpdateDrawPick)
	ON_UPDATE_COMMAND_UI(ID_DRAW_STATE_NONE, OnUpdateDrawStateNone)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_CUT, OnUpdateSelectionCut)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_COPY, OnUpdateSelectionCopy)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_PASTE, OnUpdateSelectionPaste)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_CLEAR, OnUpdateSelectionClear)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_CLEAR_NOPASTE, OnUpdateSelectionClearNoPaste)
	ON_UPDATE_COMMAND_UI(ID_SELECTION_REPLACE, OnUpdateSelectionReplace)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_LINE, OnUpdateSegmentLine)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_BOX, OnUpdateSegmentBox)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_CIRCLE, OnUpdateSegmentCircle)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_DECREASE, OnUpdateSegmentDecrease)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_INCREASE, OnUpdateSegmentIncrease)
	//ON_UPDATE_COMMAND_UI(ID_SEGMENT_SPRAY, OnUpdateSegmentSpray)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_RAISE, OnUpdateTerrainRaise)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_LEVEL, OnUpdateTerrainLevel)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_STORE, OnUpdateTerrainStore)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_BLEND, OnUpdateTerrainBlend)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_RAMP, OnUpdateTerrainRamp)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_TEXTURE, OnUpdateTerrainGround)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_MUD, OnUpdateTerrainMud)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_SEDIMENT, OnUpdateTerrainSediment)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_ROCK, OnUpdateTerrainRock)
	ON_UPDATE_COMMAND_UI(ID_TERRAIN_GRASS, OnUpdateTerrainGrass)
	ON_UPDATE_COMMAND_UI(ID_ENTITY_DELETE, OnUpdateEntityDelete)
	ON_UPDATE_COMMAND_UI(ID_ENTITY_ROTATE_X, OnUpdateEntityRotateX)
	ON_UPDATE_COMMAND_UI(ID_ENTITY_ROTATE_Y, OnUpdateEntityRotateY)
	ON_UPDATE_COMMAND_UI(ID_ENTITY_ROTATE_Z, OnUpdateEntityRotateZ)
	ON_UPDATE_COMMAND_UI(ID_WAYPOINT_CREATE, OnUpdateWaypointCreate)
	ON_UPDATE_COMMAND_UI(ID_LEVEL_RESIZELEVEL, OnUpdateLevelResizelevel)
	ON_UPDATE_COMMAND_UI(ID_FILE_PREFERENCES, OnUpdateFilePreferences)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATE_ENTITY, OnUpdateViewRotateEntity)
	ON_COMMAND(ID_FILE_SAVE_EX, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS_EX, OnFileSaveAs)
	ON_COMMAND(ID_EDIT_COPY_1, OnEditCopy1)
	ON_COMMAND(ID_EDIT_CUT_1, OnEditCut1)
	ON_COMMAND(ID_EDIT_PASTE_1, OnEditPaste1)
	//ON_COMMAND(ID_EDIT_REDO_1, OnEditRedo1)
	//ON_COMMAND(ID_EDIT_UNDO_1, OnEditUndo1)
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_COMMAND(ID_WAYPOINT_SELECT, OnWaypointSelect)
	ON_COMMAND(ID_TEST_MAP, OnTestMap)
	ON_COMMAND(ID_FILE_BUILDGAME, OnFileBuildgame)
	ON_COMMAND(ID_FILE_DOWNLOADSTOREITEMS, OnDownloadStoreItems)
	ON_COMMAND(ID_FILE_IMPORTMODEL, OnImportModel)
	ON_COMMAND(ID_FILE_CHARACTERCREATOR33003, OnCharacterCreator)

	ON_COMMAND(ID_TUTORIALS_EDITORBASICSPART1, OnTutorial1)
	ON_COMMAND(ID_TUTORIALS_EDITORBASICSPART2, OnTutorial2)
	ON_COMMAND(ID_TUTORIALS_MAKINGAQUICKGAME, OnTutorial3)
	ON_COMMAND(ID_TUTORIALS_TESTLEVELMODE, OnTutorial4)
	ON_COMMAND(ID_TUTORIALS_TESTLEVEL3DEDITMODE, OnTutorial5)
	ON_COMMAND(ID_TUTORIALS_THEWIDGETTOOL, OnTutorial6)
	ON_COMMAND(ID_TUTORIALS_3DEDITMODE, OnTutorial7)
	ON_COMMAND(ID_TUTORIALS_HOSTINGJOININGMULTIPLAYERGAMES, OnTutorial8)
	ON_COMMAND(ID_VIDEOTUTORIALS_SHOWVIDEOMENU, OnTutorialVideoMenu)
	ON_COMMAND(ID_TUTORIALS_WHAT, OnTutorialWhat)
	ON_COMMAND(ID_TUTORIALS_DEMOS, OnTutorialDemos)
	ON_COMMAND(ID_TUTORIALS_CREATE, OnTutorialCreate)
	ON_COMMAND(ID_TUTORIALS_FINISH, OnTutorialFinish)
	ON_COMMAND(ID_TUTORIALS_COMMUNITYVIDEOS, OnTutorialCommunity)

	ON_COMMAND(ID_DEMOGAMES_GAMEGURUGAMESPACKVIDEO, OnDemoGamesVideo)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER, OnDemoGamesS1)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33006, OnDemoGamesS2)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33007, OnDemoGamesS3)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33008, OnDemoGamesS4)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33009, OnDemoGamesS5)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33010, OnDemoGamesS6)
	ON_COMMAND(ID_DEMOGAMES_SINGLEPLAYER33011, OnDemoGamesS7)
	ON_COMMAND(ID_DEMOGAMES_MULTIPLAYER, OnDemoGamesM1)
	ON_COMMAND(ID_DEMOGAMES_MULTIPLAYER33013, OnDemoGamesM2)
	ON_COMMAND(ID_DEMOGAMES_MULTIPLAYER33014, OnDemoGamesM3)
	ON_COMMAND(ID_DEMOGAMES_MULTIPLAYER33015, OnDemoGamesM4)
	ON_COMMAND(ID_DEMOGAMES_MULTIPLAYER33016, OnDemoGamesM5)

	ON_COMMAND(ID_MOREMEDIA_GAMEGURUSTORE, OnMoreMediaStore)
	ON_COMMAND(ID_MOREMEDIA_MEGAPACK1, OnMoreMediaMP1)
	ON_COMMAND(ID_MOREMEDIA_MEGAPACK2, OnMoreMediaMP2)
	ON_COMMAND(ID_MOREMEDIA_MEGAPACK3, OnMoreMediaMP3)
	ON_COMMAND(ID_MOREMEDIA_BUILDINGSPACK, OnMoreMediaBP)
	ON_COMMAND(ID_MOREMEDIA_DEATHVALLEY, OnMoreMediaDVP)
	ON_COMMAND(ID_MOREMEDIA_FANTASY, OnMoreMediaFP)
	ON_COMMAND(ID_MOREMEDIA_SCIFIPACK, OnMoreMediaFSP)

	ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
	ON_COMMAND(ID_LEVEL_RESIZELEVEL, OnLevelResizelevel)
	ON_WM_CLOSE()
//	ON_WM_QUERYENDSESSION()
//	ON_WM_ENDSESSION()
	ON_COMMAND(ID_TERRAINMENU_RAISE, OnTerrainRaise)
	ON_COMMAND(ID_TERRAINMENU_LEVEL, OnTerrainLevel)
	ON_COMMAND(ID_TERRAINMENU_STORE, OnTerrainStore)
	ON_COMMAND(ID_TERRAINMENU_BLEND, OnTerrainBlend)
	ON_COMMAND(ID_TERRAINMENU_RAMP, OnTerrainRamp)
	ON_COMMAND(ID_TERRAINMENU_GROUND, OnTerrainGround)
	ON_COMMAND(ID_TERRAINMENU_MUD, OnTerrainMud)
	ON_COMMAND(ID_TERRAINMENU_SEDIMENT, OnTerrainSediment)
	ON_COMMAND(ID_TERRAINMENU_ROCK, OnTerrainRock)
	ON_COMMAND(ID_TERRAINMENU_GRASS, OnTerrainGrass)
	ON_COMMAND(ID_EDITING_CLIPBOARDEDIT, OnEditingClipboardedit)
	ON_COMMAND(ID_EDITING_ENTITYMODE, OnEditingEntitymode)
	ON_COMMAND(ID_EDITING_MOVEDOWNALAYER, OnEditingMovedownalayer)
	ON_COMMAND(ID_EDITING_MOVEUPALAYER, OnEditingMoveupalayer)
	ON_COMMAND(ID_EDITING_SEGMENTMODE, OnEditingSegmentmode)
	ON_COMMAND(ID_EDITING_VIEWENTIRELEVEL, OnEditingViewentirelevel)
	ON_COMMAND(ID_EDITING_ZOOMIN, OnEditingZoomin)
	ON_COMMAND(ID_EDITING_ZOOMOUT, OnEditingZoomout)
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_ENTITY_VIEW, OnEntityView)
	ON_COMMAND(ID_MARKER_VIEW, OnMarkerView)
	ON_COMMAND(ID_SEGMENT_VIEW, OnSegmentView)
	ON_UPDATE_COMMAND_UI(ID_SEGMENT_VIEW, OnUpdateSegmentView)
	ON_UPDATE_COMMAND_UI(ID_MARKER_VIEW, OnUpdateMarkerView)
	ON_UPDATE_COMMAND_UI(ID_ENTITY_VIEW, OnUpdateEntityView)
	ON_COMMAND(ID_APP_CREDITS, OnAppCredits)
	ON_COMMAND(ID_HELP_VIDEOSTUTORIALS, OnHelpVideosTutorials)
	ON_COMMAND(ID_HELP_LUASCRIPTINGADVICE, OnHelpLUAScriptingAdvice)
	ON_COMMAND(ID_HELP_TOGGLEPARENTALCONTROL, OnHelpToggleParentalControl)
	ON_COMMAND(ID_HELP_READUSERMANUAL, OnHelpReadUserManual)
	ON_COMMAND(ID_STANDALONE_EASTERGAME, OnStandaloneEasterGame)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATEAPP()
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATE, OnUpdateViewRotate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREFAB, OnUpdateViewPrefab)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTION, OnUpdateViewSelection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAW, OnUpdateViewDraw)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAYPOINT, OnUpdateViewWaypoint)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_WINDOW_MANAGER, OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_REGISTERED_MESSAGE(BCGM_TOOLBARMENU, OnToolbarContextMenu)
	ON_COMMAND(ID_VIEW_WORKSPACE, OnViewWorkspace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WORKSPACE, OnUpdateViewWorkspace)
	ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
	ON_COMMAND(ID_VIEW_VIEW, OnViewView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateViewOutput)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIEW, OnUpdateViewView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATE, OnUpdateViewRotate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREFAB, OnUpdateViewPrefab)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SELECTION, OnUpdateViewSelection)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DRAW, OnUpdateViewDraw)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATE_ENTITY, OnUpdateViewEntity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_WAYPOINT, OnUpdateViewWaypoint)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEST_GAME, OnUpdateViewTestGame)
	ON_MESSAGE( WM_MYMESSAGE, OnThreadFinishedTest )
	ON_MESSAGE( WM_ENTITY_WINDOW, OnUpdateEntityWindow )
	ON_MESSAGE( WM_CLOSE_LEVEL_WINDOW, OnCloseLevelWindow )
	//ON_COMMAND(ID_HELP_HELPWIZARD, &CMainFrame::OnHelpwizard)
	//ON_COMMAND(ID_HELP_CHECKFORUPDATES, &CMainFrame::OnHelpCheckForUpdates)
	ON_COMMAND(ID_HELP_ABOUTBETA, &CMainFrame::OnAboutBETA)
	ON_COMMAND(ID_HELP_TGCFORUMS, &CMainFrame::OnTGCForums)
	ON_COMMAND(ID_HELP_STEAMFORUMS, &CMainFrame::OnSteamForums)
	ON_COMMAND(ID_HELP_TGCSTORE, &CMainFrame::OnTgcStoreHelpClicked)
	ON_COMMAND(ID_HELP_VIEWITEMLICENSES, &CMainFrame::OnHelpViewitemlicenses)
	ON_COMMAND(ID_HELP_WATCHSTOREVIDEO, &CMainFrame::OnHelpWatchstorevideo)
END_MESSAGE_MAP()

static UINT indicators [ ] =
{
	ID_SEPARATOR,           // status line indicator
		//ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

LRESULT CMainFrame::OnCloseLevelWindow (WPARAM wParam, LPARAM lParam )
{
	theApp.m_pDocumentA->OnCloseDocument ( );
	theApp.OnFileNew ( );
	return 0;
}

LRESULT CMainFrame::OnUpdateEntityWindow (WPARAM wParam, LPARAM lParam )
{
	if ( wParam == 1 )
	{
		ShowEntityWindow ( true );
	}

	if ( wParam == 2 )
	{
		ShowEntityWindow ( false );
	}
	return 0;
}

LRESULT CMainFrame::OnThreadFinishedTest (WPARAM wParam, LPARAM lParam )
{

	POSITION pos = theApp.GetFirstDocTemplatePosition ( );
	CDocTemplate* pTemplate = theApp.GetNextDocTemplate ( pos );
	POSITION posA = pTemplate->GetFirstDocPosition ( );
	CDocument* pDoc = pTemplate->GetNextDoc ( posA );
	
	pDoc->SetTitle ( theApp.m_szProjectName );

	int c = 0;
	return 0;
}

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	memset ( m_bTest, 1, sizeof ( m_bTest ) );

	memset ( m_bTest, 0, sizeof ( m_bTest ) );

	/*
	// 170105
	wcscpy ( &m_szStatus [ 0 ] [ 0 ], _T ( "" ) );
	wcscpy ( &m_szStatus [ 1 ] [ 0 ], _T ( "" ) );
	wcscpy ( &m_szStatus [ 2 ] [ 0 ], _T ( "" ) );
	wcscpy ( &m_szStatus [ 3 ] [ 0 ], _T ( "" ) );
	*/

	strcpy ( &m_szStatus [ 0 ] [ 0 ], _T ( "" ) );
	strcpy ( &m_szStatus [ 1 ] [ 0 ], _T ( "" ) );
	strcpy ( &m_szStatus [ 2 ] [ 0 ], _T ( "" ) );
	strcpy ( &m_szStatus [ 3 ] [ 0 ], _T ( "" ) );
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam )
{
	return FALSE;

	TCHAR szBuffer  [ 256 ];
	TCHAR szDestroy [ 256 ];

	SetCurrentDirectory ( theApp.m_szDirectory );
	//GetPrivateProfileString ( _T ( "General" ), _T ( "Destroy Window" ), _T ( "" ), szDestroy, MAX_PATH, theApp.m_szLanguageVariant );
	strcpy ( szDestroy, "Guru Map Editor" );

	GetWindowText ( hwnd, szBuffer, 256 );

	// 170105
	//if ( wcscmp ( szBuffer, szDestroy ) == 0 )
	if ( strcmp ( szBuffer, szDestroy ) == 0 )
	{
		// 211204
		//SendMessage ( hwnd, WM_DESTROY, 0, 0 );

		// 211204
		if ( theApp.m_bAppRunning == false )
			return TRUE;

		SendMessage ( hwnd, WM_DESTROY, 0, 0 );
	}

	return TRUE;
}

CMainFrame::~CMainFrame()
{
	// 211204
	//EnumWindows ( EnumWindowsProc, 0 );
}

void CMainFrame::AddBitmapToWorkspace ( int iTab, LPTSTR szBitmap, LPTSTR szName )
{
	m_wndOutput.AddBitmap ( iTab, szBitmap, szName );
}

void CMainFrame::RemoveBitmapFromWorkspace ( int iTab, int iIndex )
{
	m_wndOutput.RemoveBitmap ( iTab, iIndex );
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// base create
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// enable Office XP look
	CBCGVisualManager::SetDefaultManager (RUNTIME_CLASS (CBCGVisualManagerXP));

	// create menubar
	if (!m_wndMenuBar.Create (this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;
	}

	// Set menubar gripper and borders
	m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndMenuBar.EnableMenuShadows ( FALSE );
	m_wndMenuBar.SetDefaultMenuResId ( IDR_MAINFRAME );

	// 091215 - stop F10 popping up invisible menu bar
	//m_wndMenuBar.EnablePopupMode(FALSE); - not in this version!

	// Toolbar creation
	CClientDC dc (this);
	BOOL bIsHighColor = dc.GetDeviceCaps (BITSPIXEL) > 8;
	UINT uiToolbarHotID = bIsHighColor ? IDB_TOOLBARCOLD256 : 0;
	UINT uiToolbarColdID = bIsHighColor ? IDB_TOOLBAR256 : 0;

	// Create toolbar for main window
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME, IDB_TOOLBARCOLD256, 0, TRUE, 0, 0, uiToolbarHotID))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	// Set tool bar tooltips from external text file
	m_wndToolBar.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "A" ) );
	m_wndToolBar.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "B" ) );
	m_wndToolBar.SetToolBarBtnTooltip ( 2,  GetLanguageData ( "Tooltips", "C" ) );

	// Build individual toolbar
	if (!m_wndToolBarView.Create(this,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|
		CBRS_GRIPPER | CBRS_BORDER_3D,
		IDR_VIEW) || !m_wndToolBarView.LoadToolBar(IDR_VIEW, IDB_BITMAP1, 0, TRUE, 0, 0, IDB_BITMAP1))
	{
		TRACE0("Failed to create build toolbar\n");
		return FALSE;
	}
	
	// Build individual toolbar
	if (!m_wndToolBarDraw.Create(this,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|
		CBRS_GRIPPER | CBRS_BORDER_3D,
		IDR_VIEW) || !m_wndToolBarDraw.LoadToolBar(IDR_DRAW, IDB_BITMAP3, 0, TRUE, 0, 0, IDB_BITMAP3))
		
	{
		TRACE0("Failed to create build toolbar\n");
		return FALSE;
	}

	// Build individual toolbar
	if (!m_wndToolBarSegment.Create(this,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|
		CBRS_GRIPPER | CBRS_BORDER_3D,
		IDR_VIEW) || !m_wndToolBarSegment.LoadToolBar(IDR_SEGMENT, IDB_BITMAP5, 0, TRUE, 0, 0, IDB_BITMAP5))
	{
		TRACE0("Failed to create build toolbar\n");
		return FALSE;
	}

	// Build individual toolbar
	if (!m_wndToolBarWaypoint.Create(this,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC| CBRS_GRIPPER | CBRS_BORDER_3D,
		IDR_VIEW) || !m_wndToolBarWaypoint.LoadToolBar(IDR_WAYPOINT, IDB_BITMAP7, 0, TRUE, 0, 0, IDB_BITMAP7))
	{
		TRACE0("Failed to create build toolbar\n");
		return FALSE;
	}

	// Build individual toolbar
	if ( g_bStandaloneNoNetMode==true )
	{
		if (!m_wndToolBarGame.Create(this,
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|
			CBRS_GRIPPER | CBRS_BORDER_3D,
			IDR_VIEW) || !m_wndToolBarGame.LoadToolBar(IDR_GAME2, IDB_BITMAP8, 0, TRUE, 0, 0, IDB_BITMAP8))
		{
			TRACE0("Failed to create build toolbar\n");
			return FALSE;
		}
	}
	else
	{
		if (!m_wndToolBarGame.Create(this,
			WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_HIDE_INPLACE|CBRS_SIZE_DYNAMIC|
			CBRS_GRIPPER | CBRS_BORDER_3D,
			IDR_VIEW) || !m_wndToolBarGame.LoadToolBar(IDR_GAME1, IDB_BITMAP8, 0, TRUE, 0, 0, IDB_BITMAP8))
		{
			TRACE0("Failed to create build toolbar\n");
			return FALSE;
		}
	}

	//Set the menu bar to a sensible size											//ADDED 05.09.13
	m_wndMenuBar.SetSizes (CSize(20,25),CSize(16,18));								//ADDED 05.09.13

	//Remove toolbar and menubar grippers (Locks Position)
	m_wndMenuBar .SetBarStyle (m_wndMenuBar .GetBarStyle () & ~(CBRS_GRIPPER ));
	m_wndToolBar .SetBarStyle (m_wndToolBar .GetBarStyle () & ~(CBRS_GRIPPER));
	m_wndToolBarView .SetBarStyle (m_wndToolBarView .GetBarStyle () & ~(CBRS_GRIPPER ));
	m_wndToolBarDraw.SetBarStyle (m_wndToolBarDraw .GetBarStyle () & ~(CBRS_GRIPPER ));
	m_wndToolBarSegment.SetBarStyle (m_wndToolBarSegment .GetBarStyle () & ~(CBRS_GRIPPER ));
	m_wndToolBarWaypoint .SetBarStyle (m_wndToolBarWaypoint .GetBarStyle () & ~(CBRS_GRIPPER ));
	m_wndToolBarGame.SetBarStyle (m_wndToolBarGame .GetBarStyle () & ~(CBRS_GRIPPER ));

	// Create status bar
	DWORD dwStyle = RBBS_GRIPPERALWAYS | RBBS_FIXEDBMP | RBBS_BREAK;
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_wndStatusBar.SetPaneWidth(0,40);
	m_wndStatusBar.SetPaneWidth(1,80);
	m_wndStatusBar.SetPaneWidth(2,65);
	m_wndStatusBar.SetPaneWidth(3,70);

	// Create entity properties window
	if (!m_wndWorkSpace.Create (_T("Workspace"), this, CSize (350, 200), FALSE, ID_VIEW_WORKSPACE, WS_CHILD | CBRS_LEFT ))
	{
		TRACE0("Failed to create entity properties window\n");
		return -1;
	}

	// Create output window
	if (!m_wndOutput.Create (	GetLanguageData ( "Library", "Name" ) , this, CSize (150, 500),
								FALSE , ID_VIEW_OUTPUT, WS_CHILD | CBRS_BOTTOM))
	{
		TRACE0("Failed to create output window\n");
		return -1;
	}

	// Enable full docking features for editor
	EnableDocking							( CBRS_ALIGN_ANY );
	m_wndWorkSpace.EnableDocking			( CBRS_ALIGN_ANY );
	m_wndOutput.EnableDocking				( CBRS_ALIGN_ANY );

	// Individual toolbars can only be docked at the top (on the main toolbar strip)
	m_wndMenuBar.EnableDocking				( CBRS_ALIGN_TOP );
	m_wndToolBar.EnableDocking				( CBRS_ALIGN_TOP );
	m_wndToolBarView.EnableDocking			( CBRS_ALIGN_TOP );
	m_wndToolBarDraw.EnableDocking			( CBRS_ALIGN_TOP );
	m_wndToolBarSegment.EnableDocking		( CBRS_ALIGN_TOP );
	m_wndToolBarWaypoint.EnableDocking		( CBRS_ALIGN_TOP );
	m_wndToolBarGame.EnableDocking			( CBRS_ALIGN_TOP );
	
	// Default docking of menubar and toolbar
	DockControlBar ( &m_wndMenuBar );
	DockControlBar ( &m_wndToolBar );
	
	// Based on above, allow default dockings now
	RecalcLayout(TRUE);

	// Default docking of individual toolbars and windows
	CRect rectA;
	m_wndToolBar.GetWindowRect(&rectA);
	rectA.OffsetRect(1,0);
	DockControlBar ( &m_wndToolBarGame,		AFX_IDW_DOCKBAR_TOP,	&rectA);
	DockControlBar ( &m_wndToolBarWaypoint,	AFX_IDW_DOCKBAR_TOP,	&rectA);
	DockControlBar ( &m_wndToolBarSegment,	AFX_IDW_DOCKBAR_TOP,	&rectA );
	DockControlBar ( &m_wndToolBarDraw,		AFX_IDW_DOCKBAR_TOP,	&rectA );
	DockControlBar ( &m_wndToolBarView,     AFX_IDW_DOCKBAR_TOP,	&rectA );
	DockControlBar ( &m_wndOutput,			AFX_IDW_DOCKBAR_LEFT,	&rectA );
	DockControlBar ( &m_wndWorkSpace );

	// Rename the View toolbar
	CString strMainToolbarTitle;
	m_wndToolBarView.SetWindowText (_T("View"));
	
	// FREE VERSION - serial code read from USERDETAILS.INI (else free version)
	//TCHAR szStringA [ MAX_PATH ];
	//theApp.CheckSerialCodeValidity();
	//GetPrivateProfileString ( _T("Application"), _T("Name"), _T(""), szStringA, MAX_PATH, theApp.m_szLanguageVariant );
	// Tag 'FREE' to the main window title if detected as a free version
	//if ( g_bFreeVersion && g_bNetBookVersion==false ) strcat ( szStringA, " Free" );
	//SetTitle ( szStringA );
	//SetWindowText ( szStringA );

	// Main Title
	SetTitle ( "Game Guru" );
	SetWindowText ( "Game Guru" );

	// Enable windows manager
	EnableWindowsDialog (ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Show the splash screen window
	CSplashWnd::ShowSplashScreen(this);

	// Record the pointers to individual toolbars (for later manipulation)
	m_pToolBarList [ 0 ] = &m_wndToolBar;
	m_pToolBarList [ 1 ] = &m_wndToolBarView;
	m_pToolBarList [ 3 ] = &m_wndToolBarDraw;
	m_pToolBarList [ 5 ] = &m_wndToolBarSegment;
	m_pToolBarList [ 7 ] = &m_wndToolBarWaypoint;
	m_pToolBarList [ 8 ] = &m_wndToolBarGame;

	// complete
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnViewCustomize()
{
	//------------------------------------
	// Create a customize toolbars dialog:
	//------------------------------------
	CBCGToolbarCustomize* pDlgCust = new CBCGToolbarCustomize (this,
		TRUE /* Automatic menus scaning */
		, (	BCGCUSTOMIZE_MENU_SHADOWS | BCGCUSTOMIZE_TEXT_LABELS | 
			BCGCUSTOMIZE_LOOK_2000 | BCGCUSTOMIZE_MENU_ANIMATIONS |
			BCGCUSTOMIZE_SELECT_SKINS)
		);

	pDlgCust->Create ();
}

LRESULT CMainFrame::OnToolbarContextMenu(WPARAM,LPARAM lp)
{
	// 210205 - don't show right mouse button popup menu
	return 0;

	CPoint point (BCG_GET_X_LPARAM(lp), BCG_GET_Y_LPARAM(lp));

	CMenu menu;
	VERIFY(menu.LoadMenu (IDR_POPUP_TOOLBAR));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	CBCGPopupMenu* pPopupMenu = new CBCGPopupMenu;
	pPopupMenu->Create (this, point.x, point.y, pPopup->Detach ());

	return 0;
}

afx_msg LRESULT CMainFrame::OnToolbarReset(WPARAM /*wp*/,LPARAM)
{
	return 0;
}

BOOL CMainFrame::OnShowPopupMenu (CBCGPopupMenu* pMenuPopup)
{
	//---------------------------------------------------------
	// Replace ID_VIEW_TOOLBARS menu item to the toolbars list:
	//---------------------------------------------------------
    CMDIFrameWnd::OnShowPopupMenu (pMenuPopup);
	if ( pMenuPopup )
	{
		// get the parent button of the popup menu
		CBCGToolbarMenuButton* pParent = pMenuPopup->GetParentButton ( );
		if ( pParent )
		{
			// get number of items in menu
			int iCount = pMenuPopup->GetMenuItemCount ( );

			// array to store strings for menu items
			TCHAR szPopup [ 11 ] [ MAX_PATH ];

			// array of buttons
			CBCGToolbarMenuButton* pButtons [ 20 ];

			// get menu list from language file
			TCHAR szMenu  [ 11 ] [ MAX_PATH ];
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "A" ), _T ( "" ), szMenu [ 0 ], MAX_PATH, theApp.m_szLanguage );	// file
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "B" ), _T ( "" ), szMenu [ 1 ], MAX_PATH, theApp.m_szLanguage );	// terrain
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "C" ), _T ( "" ), szMenu [ 2 ], MAX_PATH, theApp.m_szLanguage );	// view
			GetPrivateProfileString ( _T ( "Menu" ), _T ( "D" ), _T ( "" ), szMenu [ 3 ], MAX_PATH, theApp.m_szLanguage );	// help

			// FILE menu update
			if ( strcmp ( pParent->m_strText, szMenu [ 0 ] ) == 0 )
			{
				// get strings for file menu from language file
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "A" ), _T ( "" ), szPopup [ 0 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "B" ), _T ( "" ), szPopup [ 1 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "C" ), _T ( "" ), szPopup [ 2 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "D" ), _T ( "" ), szPopup [ 3 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "E" ), _T ( "" ), szPopup [ 4 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "F" ), _T ( "" ), szPopup [ 5 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "G" ), _T ( "" ), szPopup [ 6 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "File Menu" ), _T ( "H" ), _T ( "" ), szPopup [ 7 ], MAX_PATH, theApp.m_szLanguage );
					
				pMenuPopup->GetMenuItem ( 0 )->m_strText = szPopup [ 0 ];
				pMenuPopup->GetMenuItem ( 1 )->m_strText = szPopup [ 1 ];
				pMenuPopup->GetMenuItem ( 2 )->m_strText = szPopup [ 2 ];
				pMenuPopup->GetMenuItem ( 3 )->m_strText = szPopup [ 3 ];
				pMenuPopup->GetMenuItem ( 4 )->m_strText = szPopup [ 4 ];
				pMenuPopup->GetMenuItem ( 6 )->m_strText = szPopup [ 5 ];
				pMenuPopup->GetMenuItem ( 7 )->m_strText = szPopup [ 6 ];
				pMenuPopup->GetMenuItem ( iCount - 1 )->m_strText = szPopup [ 7 ];	

				// hide the preferences until we can properly save current level and state and then reset editor for VSYNC
				pMenuPopup->RemoveItem(11);

				// remove DOWNLOAD STORE ITEMS item
				if ( g_bStandaloneNoNetMode==true )
				{
					pMenuPopup->RemoveItem(7);
				}
			}

			// EDIT menu update
			if ( strcmp ( pParent->m_strText, szMenu [ 1 ] ) == 0 )
			{
				// get strings for edit menu from language file
				GetPrivateProfileString ( _T ( "Edit Menu" ), _T ( "A" ), _T ( "" ), szPopup [ 0 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Edit Menu" ), _T ( "B" ), _T ( "" ), szPopup [ 1 ], MAX_PATH, theApp.m_szLanguage );

				pButtons [ 0 ]            = pMenuPopup->GetMenuItem ( 0 );			// undo
				pButtons [ 1 ]            = pMenuPopup->GetMenuItem ( 1 );			// redo
				pButtons [ 0 ]->m_strText = szPopup [ 0 ];							// undo
				pButtons [ 1 ]->m_strText = szPopup [ 1 ];							// redo
			}

			// TERRAIN menu
			int iTerrainMenuIndexInParentMenu = 2;
			if ( strcmp ( pParent->m_strText, szMenu [ iTerrainMenuIndexInParentMenu ] ) == 0 )
			{
				// get strings for edit menu from language file
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "A" ), _T ( "" ), szPopup [ 0 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "B" ), _T ( "" ), szPopup [ 1 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "C" ), _T ( "" ), szPopup [ 2 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "D" ), _T ( "" ), szPopup [ 3 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "E" ), _T ( "" ), szPopup [ 4 ], MAX_PATH, theApp.m_szLanguage );
				//GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "F" ), _T ( "" ), szPopup [ 5 ], MAX_PATH, theApp.m_szLanguage );
				//GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "G" ), _T ( "" ), szPopup [ 6 ], MAX_PATH, theApp.m_szLanguage );
				//GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "H" ), _T ( "" ), szPopup [ 7 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "I" ), _T ( "" ), szPopup [ 8 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Terrain Menu" ), _T ( "J" ), _T ( "" ), szPopup [ 9 ], MAX_PATH, theApp.m_szLanguage );

				pButtons [ 0 ]            = pMenuPopup->GetMenuItem ( 0 );			// terrain
				pButtons [ 1 ]            = pMenuPopup->GetMenuItem ( 1 );			// terrain
				pButtons [ 2 ]            = pMenuPopup->GetMenuItem ( 2 );			// terrain
				pButtons [ 3 ]            = pMenuPopup->GetMenuItem ( 3 );			// terrain
				pButtons [ 4 ]            = pMenuPopup->GetMenuItem ( 4 );			// terrain
				//pButtons [ 5 ]            = pMenuPopup->GetMenuItem ( 5 );			// paint
				//pButtons [ 6 ]            = pMenuPopup->GetMenuItem ( 6 );			// paint
				//pButtons [ 7 ]            = pMenuPopup->GetMenuItem ( 7 );			// paint
				pButtons [ 8 ]            = pMenuPopup->GetMenuItem ( 5 );			// paint
				pButtons [ 9 ]            = pMenuPopup->GetMenuItem ( 6 );			// paint

				pButtons [ 0 ]->m_strText = szPopup [ 0 ];
				pButtons [ 1 ]->m_strText = szPopup [ 1 ];
				pButtons [ 2 ]->m_strText = szPopup [ 2 ];
				pButtons [ 3 ]->m_strText = szPopup [ 3 ];
				pButtons [ 4 ]->m_strText = szPopup [ 4 ];
				//pButtons [ 5 ]->m_strText = szPopup [ 5 ];
				//pButtons [ 6 ]->m_strText = szPopup [ 6 ];
				//pButtons [ 7 ]->m_strText = szPopup [ 7 ];
				pButtons [ 8 ]->m_strText = szPopup [ 8 ];
				pButtons [ 9 ]->m_strText = szPopup [ 9 ];
			}

			// HELP menu update
			int iHelpMenuIndexInParentMenu = 3;
			if ( strcmp ( pParent->m_strText, szMenu [ iHelpMenuIndexInParentMenu ] ) == 0 )
			{
				// get strings for edit menu from language file
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "A" ), _T ( "" ), szPopup [ 0 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "B" ), _T ( "" ), szPopup [ 1 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "C" ), _T ( "" ), szPopup [ 2 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "D" ), _T ( "" ), szPopup [ 3 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "E" ), _T ( "" ), szPopup [ 4 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "F" ), _T ( "" ), szPopup [ 5 ], MAX_PATH, theApp.m_szLanguage );
				GetPrivateProfileString ( _T ( "Help Menu" ), _T ( "G" ), _T ( "" ), szPopup [ 6 ], MAX_PATH, theApp.m_szLanguage );
			
				pButtons [ 0 ]            = pMenuPopup->GetMenuItem ( 1 );	
				pButtons [ 1 ]            = pMenuPopup->GetMenuItem ( 2 );	
				pButtons [ 2 ]            = pMenuPopup->GetMenuItem ( 3 );	
				pButtons [ 3 ]            = pMenuPopup->GetMenuItem ( 4 );	
				pButtons [ 4 ]            = pMenuPopup->GetMenuItem ( 5 );	
				pButtons [ 5 ]            = pMenuPopup->GetMenuItem ( 6 );	

				pButtons [ 0 ]->m_strText = szPopup [ 1 ];	
				pButtons [ 1 ]->m_strText = szPopup [ 2 ];	
				pButtons [ 2 ]->m_strText = szPopup [ 3 ];	
				pButtons [ 3 ]->m_strText = szPopup [ 4 ];	
				pButtons [ 4 ]->m_strText = szPopup [ 5 ];	
				pButtons [ 5 ]->m_strText = szPopup [ 6 ];	

				pMenuPopup->RemoveItem(11);//separator
				pMenuPopup->RemoveItem(10);//store

				if ( g_bStandaloneNoNetMode==true )
				{
					if ( g_bAlsoRemoveRestrictedContentToggle == true )
					{
						pMenuPopup->RemoveItem(9);
						pMenuPopup->RemoveItem(7);
						pMenuPopup->RemoveItem(5);
						pMenuPopup->RemoveItem(4);
						pMenuPopup->RemoveItem(3);
						pMenuPopup->RemoveItem(3);
					}
					else
					{
						// leaves toggle restricted mode alone
						pMenuPopup->RemoveItem(5);
						pMenuPopup->RemoveItem(4);
						pMenuPopup->RemoveItem(3);
						pMenuPopup->RemoveItem(3);
					}
				}
				else
				{
					pMenuPopup->RemoveItem(3);//vote
				}
			}

			// VIDEO TUTORIALS
			if ( strcmp ( pParent->m_strText, "Video Tutorials" ) == 0 )
			{
				if ( g_bStandaloneNoNetMode==true )
				{
					pMenuPopup->RemoveItem(11);
				}
			}

			// GETTING STARTED TUTORIALS
			if ( strcmp ( pParent->m_strText, "Getting Started" ) == 0 )
			{
				if ( g_bStandaloneNoNetMode==true )
				{
					pMenuPopup->RemoveItem(1);
				}
			}

			// DEMO GAMES
			if ( strcmp ( pParent->m_strText, "Demo Games" ) == 0 )
			{
				if ( g_bStandaloneNoNetMode==true )
				{
					pMenuPopup->RemoveItem(12);
					pMenuPopup->RemoveItem(11);
					pMenuPopup->RemoveItem(10);
					pMenuPopup->RemoveItem(9);
					pMenuPopup->RemoveItem(8);

					// also remove any violent games
					if ( theApp.IsParentalControlActive()==true )
					{
						//pMenuPopup->RemoveItem(7);
						pMenuPopup->RemoveItem(6);
						pMenuPopup->RemoveItem(5);
						pMenuPopup->RemoveItem(3);
						pMenuPopup->RemoveItem(1);
						pMenuPopup->RemoveItem(0);
					}
				}
			}

			// MORE MEDIA
			if ( strcmp ( pParent->m_strText, "More Media" ) == 0 )
			{
				if ( g_bStandaloneNoNetMode==true )
				{
					pMenuPopup->GetMenuItem(0)->m_strText = "Free Community Media";
					pMenuPopup->RemoveItem(7);
					pMenuPopup->RemoveItem(6);
					pMenuPopup->RemoveItem(5);
					pMenuPopup->RemoveItem(4);
					pMenuPopup->RemoveItem(3);
					pMenuPopup->RemoveItem(2);
					pMenuPopup->RemoveItem(1);
				}
			}
		}
	}
	
    if (pMenuPopup != NULL && pMenuPopup->GetMenuBar ()->CommandToIndex (ID_VIEW_TOOLBARS) >= 0)
    {
		if (CBCGToolBar::IsCustomizeMode ())
		{
			//----------------------------------------------------
			// Don't show toolbars list in the cuztomization mode!
			//----------------------------------------------------
			return FALSE;
		}

		pMenuPopup->RemoveAllItems ();

		CMenu menu;
		VERIFY(menu.LoadMenu (IDR_POPUP_TOOLBAR));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		pMenuPopup->GetMenuBar ()->ImportFromMenu (*pPopup, TRUE);

		// get number of items in menu
		int iCount = pMenuPopup->GetMenuItemCount ( );

		// array to store strings for menu items
		TCHAR szPopup [ 11  ] [ MAX_PATH ];

		// array of buttons
		CBCGToolbarMenuButton* pButtons [ 20 ];

		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "A" ), _T ( "" ), szPopup [ 0 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "B" ), _T ( "" ), szPopup [ 1 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "C" ), _T ( "" ), szPopup [ 2 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "D" ), _T ( "" ), szPopup [ 3 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "E" ), _T ( "" ), szPopup [ 4 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "F" ), _T ( "" ), szPopup [ 5 ], MAX_PATH, theApp.m_szLanguage );
		GetPrivateProfileString ( _T ( "Toolbar Menu" ), _T ( "G" ), _T ( "" ), szPopup [ 6 ], MAX_PATH, theApp.m_szLanguage );

		pButtons [ 0 ]            = pMenuPopup->GetMenuItem ( 0 );				// new window
		pButtons [ 1 ]            = pMenuPopup->GetMenuItem ( 1 );				// cascade
		pButtons [ 2 ]            = pMenuPopup->GetMenuItem ( 2 );				// tile
		pButtons [ 3 ]            = pMenuPopup->GetMenuItem ( 3 );				// new window
		pButtons [ 4 ]            = pMenuPopup->GetMenuItem ( 4 );				// cascade
		pButtons [ 5 ]            = pMenuPopup->GetMenuItem ( 5 );				// tile
		pButtons [ 6 ]            = pMenuPopup->GetMenuItem ( 6 );				// tile

		pButtons [ 0 ]->m_strText = szPopup [ 0 ];
				
		pButtons [ 1 ]->m_strText = szPopup [ 1 ];
		pButtons [ 2 ]->m_strText = szPopup [ 2 ];
		pButtons [ 3 ]->m_strText = szPopup [ 3 ];
		pButtons [ 4 ]->m_strText = szPopup [ 4 ];
		pButtons [ 5 ]->m_strText = szPopup [ 5 ];
		pButtons [ 6 ]->m_strText = szPopup [ 6 ];
    }

	return TRUE;
}

void CMainFrame::SetWorkspaceVisible ( bool bVisible )
{
	ShowControlBar (&m_wndWorkSpace,
					!(m_wndWorkSpace.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::ShowEntityWindow ( bool bVisible )
{
	if ( bVisible )
	{
		m_wndWorkSpace.SetVisible ( TRUE );
		m_wndOutput.ShowWindow ( 0 );
		m_wndMenuBar.EnableWindow ( 0 );
		m_wndToolBar.EnableWindow ( 0 );
		m_wndToolBarGame.EnableWindow ( 0 );
	}
	else
	{
		m_wndWorkSpace.SetVisible ( FALSE );
		m_wndOutput.ShowWindow ( 1 );
		m_wndMenuBar.EnableWindow ( 1 );
		m_wndToolBar.EnableWindow ( 1 );
		m_wndToolBarGame.EnableWindow ( 1 );
	}
	RecalcLayout ();
}

void CMainFrame::SetEntityVisible ( bool bVisible )
{
	ShowControlBar (&m_wndOutput, WS_VISIBLE, FALSE);
	RecalcLayout ();
}

void CMainFrame::OnViewWorkspace() 
{
	ShowControlBar (&m_wndWorkSpace,
					!(m_wndWorkSpace.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewWorkspace(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndWorkSpace.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewOutput() 
{
	ShowControlBar (&m_wndOutput,
					!(m_wndOutput.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::OnViewView() 
{
	ShowControlBar (&m_wndToolBarView,
					!(m_wndToolBarView.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewOutput(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndOutput.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnUpdateViewView(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndToolBarView.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnWindowManager() 
{
	ShowWindowsDialog ();
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// STANDARD
	m_wndToolBar.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "1" ) );
	m_wndToolBar.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "2" ) );
	m_wndToolBar.SetToolBarBtnTooltip ( 2,  GetLanguageData ( "Tooltips", "3" ) );

	// VIEW
	m_wndToolBarView.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "9" ) );
	m_wndToolBarView.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "10" ) );

	// ENTITY OR TERRAIN MODE
	m_wndToolBarDraw.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "18" ) );
	m_wndToolBarDraw.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "20" ) );
	m_wndToolBarDraw.SetToolBarBtnTooltip ( 2,  GetLanguageData ( "Tooltips", "19" ) );

	// ART (TERRAIN)
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "26" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "27" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 2,  GetLanguageData ( "Tooltips", "28" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 3,  GetLanguageData ( "Tooltips", "29" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 4,  GetLanguageData ( "Tooltips", "30" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 5,  GetLanguageData ( "Tooltips", "31" ) );
	m_wndToolBarSegment.SetToolBarBtnTooltip ( 6,  GetLanguageData ( "Tooltips", "35" ) );
	//m_wndToolBarSegment.SetToolBarBtnTooltip ( 7,  GetLanguageData ( "Tooltips", "36" ) );
	//m_wndToolBarSegment.SetToolBarBtnTooltip ( 8,  GetLanguageData ( "Tooltips", "37" ) );
	//m_wndToolBarSegment.SetToolBarBtnTooltip ( 9,  GetLanguageData ( "Tooltips", "38" ) );

	// WAYPOINT
	m_wndToolBarWaypoint.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "32" ) );
	m_wndToolBarWaypoint.SetToolBarBtnTooltip ( 1,  GetLanguageData ( "Tooltips", "33" ) );

	// TEST AND MULTIPLAYER GAME
	m_wndToolBarGame.SetToolBarBtnTooltip ( 0,  GetLanguageData ( "Tooltips", "34" ) );
	if ( g_bStandaloneNoNetMode == false )
	{
		m_wndToolBarGame.SetToolBarBtnTooltip ( 2,  GetLanguageData ( "Tooltips", "39" ) );
	}
}

void CMainFrame::OnSize( UINT nType, int cx, int cy )
{
    if ( !m_wndToolBar.m_hWnd ) return;

    CRect rectA;
    CSize size = m_wndToolBar.CalcFixedLayout( FALSE, TRUE );

    m_wndToolBar.GetWindowRect(&rectA);
    rectA.right = size.cx; rectA.left = 0;
    
    DockControlBar ( &m_wndToolBar, AFX_IDW_DOCKBAR_TOP, &rectA );
    rectA.OffsetRect( m_wndToolBar.CalcFixedLayout( FALSE, TRUE ).cx,0 );
    
    DockControlBar ( &m_wndToolBarView, AFX_IDW_DOCKBAR_TOP, &rectA );
    rectA.OffsetRect( m_wndToolBarView.CalcFixedLayout( FALSE, TRUE ).cx,0 );

    DockControlBar ( &m_wndToolBarSegment, AFX_IDW_DOCKBAR_TOP,  &rectA );
    rectA.OffsetRect( m_wndToolBarSegment.CalcFixedLayout( FALSE, TRUE ).cx,0 );

    DockControlBar ( &m_wndToolBarDraw, AFX_IDW_DOCKBAR_TOP,  &rectA );
    rectA.OffsetRect( m_wndToolBarDraw.CalcFixedLayout( FALSE, TRUE ).cx,0 );

    DockControlBar (&m_wndToolBarWaypoint,AFX_IDW_DOCKBAR_TOP,&rectA);
    rectA.OffsetRect( m_wndToolBarWaypoint.CalcFixedLayout( FALSE, TRUE ).cx,0 );

    DockControlBar (&m_wndToolBarGame,AFX_IDW_DOCKBAR_TOP,&rectA );

    CMDIFrameWnd::OnSize( nType, cx,cy );
}

void CMainFrame::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMDIFrameWnd::OnMouseMove(nFlags, point);
}

void CMainFrame::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CMDIFrameWnd::OnLButtonUp(nFlags, point);
}

void CMainFrame::SetStatusBarText ( int iIndex, LPTSTR pszText )
{
	strcpy ( &m_szStatus [ iIndex ] [ 0 ], _T ( "" ) );
	strcpy ( &m_szStatus [ iIndex ] [ 0 ], pszText );
}

void CMainFrame::OnMouseMoveXX(UINT nFlags, CPoint point)
{
	// update status bar text (strings taken from file map of DBP app)
	char szA [ 255 ];
	char szB [ 255 ];
	char szC [ 255 ];
	char szD [ 255 ];
	cIPC* pIPC			= theApp.m_Message.GetIPC ( );
	pIPC->ReceiveBuffer ( &szA, 4000, sizeof ( char ) * 255 );
	pIPC->ReceiveBuffer ( &szB, 4256, sizeof ( char ) * 255 );
	pIPC->ReceiveBuffer ( &szC, 4512, sizeof ( char ) * 255 );
	pIPC->ReceiveBuffer ( &szD, 4768, sizeof ( char ) * 255 );
	CString a = szA;
	CString b = szB;
	CString c = szC;
	CString d = szD;
	if ( m_wndStatusBar )
	{
		m_wndStatusBar.SetPaneText ( 0, a, TRUE );
		m_wndStatusBar.SetPaneText ( 1, b, TRUE );
		m_wndStatusBar.SetPaneText ( 2, c, TRUE );
		m_wndStatusBar.SetPaneText ( 3, d, TRUE );
	}
}

void CMainFrame::OnViewZoomIn() 
{
	theApp.SetFileMapData ( 60, 1 );

	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnViewZoomOut() 
{
	theApp.SetFileMapData ( 60, 1 );

	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnViewIncreaseShroud() 
{
	theApp.SetFileMapData ( 60, 1 );

	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnViewDecreaseShroud() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 4 );
}

void CMainFrame::OnViewLayers() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 5 );
}

void CMainFrame::OnViewMoveUpLayer() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 6 );
}

void CMainFrame::OnViewMoveDownLayer() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 7 );
}

void CMainFrame::OnViewOverview() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 8 );
}

void CMainFrame::OnViewCloseup() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 2 );
	theApp.SetFileMapData ( 204, 9 );
}

void CMainFrame::OnModePaint() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnModeSelect() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnModeArt() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnModeEntity() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnModeWaypoint() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnUpdateViewMode(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndToolBarMode.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnUpdateViewRotate(CCmdUI* pCmdUI)
{
}

void CMainFrame::OnPropertyRotate() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 3 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnPropertyMirror() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 3 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnPropertyFlip() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 3 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnDrawA() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnDrawB() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnDrawC() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnDrawD() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 4 );
}

void CMainFrame::OnDrawStateNone() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 5 );
}

void CMainFrame::OnDrawPick() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 6 );
}

void CMainFrame::OnSelectionCut() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnSelectionCopy() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnSelectionPaste() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnSelectionClear() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 5 );
}

void CMainFrame::OnSelectionClearNopaste() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 6 );
}

void CMainFrame::OnSelectionReplace() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 5 );
	theApp.SetFileMapData ( 204, 4 );
}

void CMainFrame::OnTerrainRaise() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnTerrainLevel() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnTerrainStore() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnTerrainBlend() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 4 );
}

void CMainFrame::OnTerrainRamp() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 5 );
}

void CMainFrame::OnTerrainGround() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 6 );//6 );
}

void CMainFrame::OnTerrainMud() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 6);//7 );
}

void CMainFrame::OnTerrainSediment() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 6);//8 );
}

void CMainFrame::OnTerrainRock() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 6 );
}

void CMainFrame::OnTerrainGrass() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 6 );
	theApp.SetFileMapData ( 204, 10 );
}

void CMainFrame::OnEntityDelete() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 7 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnEntityRotateX() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 7 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnEntityRotateY() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 7 );
	theApp.SetFileMapData ( 204, 3 );
}

void CMainFrame::OnEntityRotateZ() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 7 );
	theApp.SetFileMapData ( 204, 4 );
}

void CMainFrame::OnWaypointCreate() 
{
	theApp.SetFileMapData ( 60, 1 );

	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 200, 8 );
	theApp.SetFileMapData ( 204, 1 );
}

void CMainFrame::OnViewRotate() 
{
}

void CEditorApp::OnAppEditorKeyboardShortcuts ( )
{
	// set flag to trigger F1 in editor
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 21 );
	theApp.SetFileMapData ( 204, 1 );
}

void CEditorApp::OnInteractiveTutorial ( )
{
	// set flag to trigger interactive tutorial in editor
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 21 );
	theApp.SetFileMapData ( 204, 2 );
}

void CMainFrame::OnViewPrefab() 
{
	// TODO: Add your command handler code here
		ShowControlBar (&m_wndToolBarDraw,
					!(m_wndToolBarDraw.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewPrefab(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndToolBarDraw.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewSelection() 
{
}

void CMainFrame::OnUpdateViewSelection(CCmdUI* pCmdUI)
{
}

void CMainFrame::OnViewDraw() 
{
	// TODO: Add your command handler code here
			ShowControlBar (&m_wndToolBarSegment,
					!(m_wndToolBarSegment.GetStyle () & WS_VISIBLE),
					FALSE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewDraw(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndToolBarSegment.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnViewRotateEntity() 
{
}

void CMainFrame::OnUpdateViewEntity(CCmdUI* pCmdUI)
{
}

void CMainFrame::OnViewWaypoint() 
{

	

	ShowControlBar (&m_wndToolBarWaypoint,
		!(m_wndToolBarWaypoint.GetStyle () & WS_VISIBLE),
		FALSE);
	RecalcLayout ();
}

void CMainFrame::OnViewTestGame() 
{
	// TODO: Add your command handler code here
	ShowControlBar (&m_wndToolBarGame,
		!(m_wndToolBarGame.GetStyle () & WS_VISIBLE),
		FALSE);
	RecalcLayout ();
}

void CMainFrame::OnUpdateViewTestGame(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ( theApp.m_bDisable );
	pCmdUI->SetCheck (m_wndToolBarGame.GetStyle () & WS_VISIBLE);
}

void CMainFrame::OnUpdateViewWaypoint(CCmdUI* pCmdUI)
{
	pCmdUI->Enable ( theApp.m_bDisable );

	pCmdUI->SetCheck (m_wndToolBarWaypoint.GetStyle () & WS_VISIBLE);

	//pCmdUI->SetCheck (!WS_VISIBLE);
}

BOOL CALLBACK EnumWindowsProcEx(HWND hwnd,LPARAM lParam );

void CMainFrame:: Quit ( void )
{
	// X10 - 030208 - delay the interface exit so restart manager can kill mapeditor
	//PostQuitMessage ( 0 );
	Sleep(100);
	SetTimer ( 99, 500, NULL );
}

void gShowHideTaskBar(BOOL bHide /*=FALSE*/)
{
	CRect rectWorkArea = CRect(0,0,0,0);
	CRect rectTaskBar = CRect(0,0,0,0);
	CWnd* pWnd = CWnd::FindWindow("Shell_TrayWnd", "");
	if( bHide )
	{
		// Code to Hide the System Task Bar
		SystemParametersInfo(SPI_GETWORKAREA,
			0,
			(LPVOID)&rectWorkArea,
			0);
 
		if( pWnd )
		{
			pWnd->GetWindowRect(rectTaskBar);
			rectWorkArea.bottom += rectTaskBar.Height();
			SystemParametersInfo(SPI_SETWORKAREA,
				0,
				(LPVOID)&rectWorkArea,
				0);
 
			pWnd->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		// Code to Show the System Task Bar
		SystemParametersInfo(SPI_GETWORKAREA,
			0,
			(LPVOID)&rectWorkArea,
			0);

		if( pWnd )
		{
			pWnd->GetWindowRect(rectTaskBar);
			rectWorkArea.bottom -= rectTaskBar.Height();
			SystemParametersInfo(SPI_SETWORKAREA,
				0,
				(LPVOID)&rectWorkArea,
				0);
 
			pWnd->ShowWindow(SW_SHOW);
		}
	}
}

void CMainFrame::TestOrMultiplayerGame ( int iMultiplayerMode ) 
{
	// set the file map to instruct the mapeditor to launch TEST LEVEL
	theApp.SetFileMapData ( 200, 9 );
	theApp.SetFileMapData ( 204, 1+iMultiplayerMode );

	// make child window full screen view for Test Level mode
	HMONITOR hmon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	if (GetMonitorInfo(hmon, &mi))
	{
		// hide all the BCG sub-windows so only the main frame remains
		m_wndMenuBar.ShowWindow(SW_HIDE);
		m_wndToolBar.ShowWindow(SW_HIDE);
		m_wndToolBarView.ShowWindow(SW_HIDE);
		m_wndToolBarDraw.ShowWindow(SW_HIDE);
		m_wndToolBarSegment.ShowWindow(SW_HIDE);
		m_wndToolBarWaypoint.ShowWindow(SW_HIDE);
		m_wndToolBarGame.ShowWindow(SW_HIDE);		
		m_wndWorkSpace.ShowWindow(SW_HIDE);
		m_wndStatusBar.ShowWindow(SW_HIDE);
		m_wndOutput.ShowWindow(SW_HIDE);

		// make it a real full screen window and resize it to fit
		theApp.dwRestoreGUIWindowMode = GetWindowLong ( m_hWnd, GWL_STYLE );
		SetWindowLong ( m_hWnd, GWL_STYLE, WS_POPUP );
		theApp.GetMainWnd()->SetWindowPos(&wndTop, 0, 0, mi.rcMonitor.right, mi.rcMonitor.bottom, SWP_SHOWWINDOW );
		int iTitleSizeY = GetSystemMetrics(SM_CYSIZE);
		int iXBorder = GetSystemMetrics(SM_CXSIZEFRAME);
		int iYBorder = GetSystemMetrics(SM_CYSIZEFRAME);
		int iThinBorder = GetSystemMetrics(SM_CYFIXEDFRAME);
		theApp.m_pDocumentA->GetFirstFrame()->SetWindowPos ( &wndTop, 0, 0, mi.rcMonitor.right, mi.rcMonitor.bottom, SWP_SHOWWINDOW );

		// flag test game
		g_bInTestGame = true;
	}
}

void CMainFrame::OnTestGame() 
{
	TestOrMultiplayerGame(0);
}

void CMainFrame::OnMultiplayerGame() 
{
	TestOrMultiplayerGame(1);
}

void CMainFrame::OnUpdateTestGame ( CCmdUI* pCmdUI )
{
	//SetToolbarButtonState ( 9, 1, 1 );

	//pCmdUI->Enable ( m_bTest [ 9 ] [ 0 ] );
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateTestMap ( CCmdUI* pCmdUI )
{
	//SetToolbarButtonState ( 9, 1, 1 );

	//pCmdUI->Enable ( m_bTest [ 9 ] [ 0 ] );
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::SetToolbarButtonState ( int iToolbar, int iButton, int iState )
{
	m_bTest [ iToolbar ] [ iButton ] = iState;
}

void CMainFrame::OnUpdateViewZoomIn(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewZoomOut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewIncreaseShroud(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewDecreaseShroud(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewLayers(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewMoveupLayer(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewMovedownLayer(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewOverview(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateViewCloseUp(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateModePaint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdateModeSelect(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdateModeArt(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdateModeEntity(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdateModeWaypoint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdatePropertyRotate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdatePropertyFlip(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdatePropertyMirror(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawA(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawB(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawC(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawD(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawStateNone(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateDrawPick(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionClearNoPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSelectionReplace(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateTerrainRaise(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainLevel(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainStore(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainBlend(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainRamp(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainGround(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainMud(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainSediment(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainRock(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
void CMainFrame::OnUpdateTerrainGrass(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

/*
void CMainFrame::OnUpdateSegmentLine(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSegmentBox(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );	
}

void CMainFrame::OnUpdateSegmentCircle(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSegmentDecrease(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );	
}

void CMainFrame::OnUpdateSegmentIncrease(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateSegmentSpray(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}
*/

void CMainFrame::OnUpdateEntityDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateEntityRotateX(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateEntityRotateY(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );	
}

void CMainFrame::OnUpdateEntityRotateZ(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateWaypointCreate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateModeWaypointSelect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable ( theApp.m_bDisable );
}


void CMainFrame::OnUpdateLevelResizelevel(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdateFilePreferences(CCmdUI* pCmdUI) 
{
}


/*
void CMainFrame::OnUpdateViewRotate(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}
*/

/*
void CMainFrame::OnUpdateViewPrefab(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}
*/

/*
void CMainFrame::OnUpdateViewSelection(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}
*/

/*
void CMainFrame::OnUpdateViewDraw(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}
*/

void CMainFrame::OnUpdateViewRotateEntity(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

/*
void CMainFrame::OnUpdateViewWaypoint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}
*/
void CMainFrame::OnFileSave() 
{
	theApp.SetFileMapData       ( 404, 1 );

	//OnFileSaveAs ( );
}

void CMainFrame::OnFileSaveAs() 
{
	theApp.SetFileMapData       ( 434, 1 );
}

void CMainFrame::OnEditCopy1() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 962, 1 );
}

void CMainFrame::OnEditCut1() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 958, 1 );
}

void CMainFrame::OnEditPaste1() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 966, 1 );
}

void CMainFrame::OnHelpIndex() 
{
	//TCHAR szLink [ MAX_PATH ];
	//GetPrivateProfileString ( _T ( "Help" ), _T ( "Index" ), _T ( "" ), szLink, MAX_PATH, theApp.m_szLanguageVariant );
	//HINSTANCE hInstance = ShellExecute ( NULL, _T ( "open" ), szLink, _T ( "" ), NULL, SW_SHOWNORMAL );
}

void CMainFrame::OnWaypointSelect() 
{
	// TODO: Add your command handler code here

	theApp.SetFileMapData ( 60, 1 );

	theApp.SetFileMapData ( 200, 8 );
	theApp.SetFileMapData ( 204, 2 );
	
}

void CMainFrame::OnTestMap() 
{
	// TODO: Add your command handler code here

	theApp.SetFileMapData ( 60, 1 );

	theApp.SetFileMapData ( 200, 9 );
	theApp.SetFileMapData ( 204, 2 );
	
}

void CMainFrame::LaunchGameCreatorStore( bool bGotoAddons )
{
	// lee - 130709 - switched to DLL store
	theApp.OpenStore( bGotoAddons );

	/* do not need inline store any more
	DWORD dwState = 0;
	BOOL bOnline = InternetGetConnectedState( &dwState, 0 );
	if ( !bOnline )
	{
		MessageBox( "Could not detect an internet connection", theApp.m_szErrorString, MB_OK );
	}
	else
	{
		char str [ 256 ];
		theApp.GetMainWnd()->GetWindowText( str, 256 );
		theApp.m_iAppX = 0;
		theApp.m_iAppY = 0;
		theApp.m_iAppWidth = 0;
		theApp.m_iAppHeight = 0;
		theApp.m_bGotoAddons = bGotoAddons;
		theApp.GetMainWnd()->SetWindowText( "FPS Creator - Game Creator Store" );
		CMarketWindow TGCStore;
		TGCStore.DoModal();
		theApp.m_bGotoAddons = false;
		theApp.GetMainWnd()->SetWindowPos( NULL, theApp.m_iAppX,theApp.m_iAppY, theApp.m_iAppWidth,theApp.m_iAppHeight, 0 );
		theApp.GetMainWnd()->SetWindowText( "FPS Creator" );
		// V110 - 110608 - add FREE if the free version
		// X10 - if ( g_bFreeVersion ) theApp.GetMainWnd()->SetWindowText( "FPS Creator Free" );
	}
	*/
}

#include "cDialogEnd.h"
extern int g_iBoysLifeQuit;

bool GetDefaultBrowserPath( char *szValue, DWORD dwSize )
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

//void CMainFrame::DemoMessageBox ( void )
void DemoMessageBox ( void )
{
	/*
	char pMessageText[] = "Dieses Feature ist nicht in der Demo-Version enthalten. Weiterführende Informationen zum Spiel finden Sie hier?";
	if ( MessageBox ( NULL, pMessageText, "FPS Creator X10", MB_YESNO )==IDYES )
	{
		char szBrowser [ MAX_PATH ];
		GetDefaultBrowserPath( szBrowser, MAX_PATH );
		LPSTR pGoToLink = "http://www.sqoops.de/games/A11967";
		ShellExecute ( NULL, "open", szBrowser, pGoToLink, "", SW_SHOW );
	}
	*/
}

void CMainFrame::OnFileBuildgame() 
{
	/* no free version right now
	// FREE VERSION
	if ( g_bFreeVersion )
	{
		// display message and allow to cancel jump to upgrade store
		char pMessageText[] = "\
You can upgrade FPS Creator with the ability to create standalone executables\n \
of your games, which include multi-level and multiplayer arena games. The upgrade \
also grants a royalty free license to sell what you make.\n \
\n \
To upgrade:\n \
\n \
* Click on the “Game Creator Store” button\n \
* Create a store account or log-in if you already have one\n \
* Buy the “Build Game Upgrade” from the TGC/Add-ons section\n \
\n \
Once purchased you will see a new Build Game menu item in the File menu.\n \
Would you like to go to the Game Creator Store now?";

		// display message box, jump to store if click YES
		if ( MessageBox ( pMessageText, GetLanguageData( "File Menu", "I" ), MB_YESNO )==IDYES )
		{
			// V110 BETA4 - 070608 - launch Game Creator Store module
			LaunchGameCreatorStore( true );
		}
		else
		{
			// FPGC - 230909 - Second Bite At The Cherry (offer it for free)
		char pTrialPayText[] = "\
Instead of buying the upgrade, how about getting it absolutely FREE?\n \
\n\
TrialPay are offering you the incredible opportunity to obtain FPS Creator fully featured \
and includes additional game building media plus of course the ability to save standalone games \
and multiplayer games.\n \
\n\
Would you like to learn more about this offer?";

			// display message box, jump to store if click YES
			if ( MessageBox ( pTrialPayText, GetLanguageData( "File Menu", "I" ), MB_YESNO )==IDYES )
			{
				// FPGC - 230909 - launch trial pay website
				char szBrowser [ MAX_PATH ];
				theApp.GetDefaultBrowserPath( szBrowser, MAX_PATH );
				ShellExecute ( NULL, "open", szBrowser, "http://www.trialpay.com/productpage/?c=014cd22&tid=9ahOL99", "", SW_SHOW );
			}
		}
	}
	else
	{
	*/

	// flag Save Standalone (BUILDGAME) has been selected
	theApp.SetFileMapData ( 758, 1 );
}

void CMainFrame::OnDownloadStoreItems()
{
	// launch download store items
	theApp.OpenDownloadStoreItems();
}

void CMainFrame::OnImportModel()
{
	// trigger import model feature of editor
	theApp.SetFileMapData ( 762, 1 );
}

void CMainFrame::OnCharacterCreator()
{
	// trigger character creator feature of editor
	theApp.SetFileMapData ( 762, 2 );
}

void CMainFrame::OnTutorial1()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/rXm0e4uRP10" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial2()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/zLt6UbwYxeA" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial3()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/9fHU0296E-g" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial4()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/RWJDpUGmmXg" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial5()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/x3m9JVNOmp8" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial6()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/yyG665d76xA" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial7()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/S4c4QGNwL_A" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorial8()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/7KCyyqxJNTw" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnTutorialVideoMenu()
{
	// trigger to show the video menu (getting started dialog)
	theApp.SetFileMapData ( 754, 1001 );
}
void CMainFrame::OnTutorialWhat()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/ewe82RBLItU" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorialDemos()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/bXTn9Tqu4qc" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorialCreate()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/Emrdof1cDl8" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorialFinish()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/OPxsxGzuivo" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnTutorialCommunity()
{
	//ShellExecuteW( NULL, L"open", L"https://www.game-guru.com/tutorials/1030" , NULL, NULL, SW_SHOWMAXIMIZED );
	ShellExecuteW( NULL, L"open", L"https://www.game-guru.com/live-streams" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnDemoGamesVideo()
{
	ShellExecuteW( NULL, L"open", L"https://youtu.be/Z3nJ9lroY4o" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnDemoGamesS1()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 1 );
}
void CMainFrame::OnDemoGamesS2()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 2 );
}
void CMainFrame::OnDemoGamesS3()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 3 );
}
void CMainFrame::OnDemoGamesS4()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 4 );
}
void CMainFrame::OnDemoGamesS5()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 5 );
}
void CMainFrame::OnDemoGamesS6()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 6 );
}
void CMainFrame::OnDemoGamesS7()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 7 );
}
void CMainFrame::OnDemoGamesM1()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 21 );
}
void CMainFrame::OnDemoGamesM2()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 22 );
}
void CMainFrame::OnDemoGamesM3()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 23 );
}
void CMainFrame::OnDemoGamesM4()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 24 );
}
void CMainFrame::OnDemoGamesM5()
{
	// trigger a stock map to load
	theApp.SetFileMapData ( 754, 25 );
}

void CMainFrame::OnMoreMediaStore()
{
	if ( g_bStandaloneNoNetMode==true )
		ShellExecuteW( NULL, L"open", L"https://forum.game-guru.com/board/25" , NULL, NULL, SW_SHOWMAXIMIZED );
	else
		ShellExecuteW( NULL, L"open", L"https://www.tgcstore.net/" , NULL, NULL, SW_SHOWMAXIMIZED );
		//ShellExecuteW( NULL, L"open", L"http://en.tgcstore.net/?steam=true" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaMP1()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/321140/" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaMP2()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/321141/ " , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaMP3()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/321142/" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaBP()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/365520/" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaDVP()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/365540/" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaFP()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/365530/" , NULL, NULL, SW_SHOWMAXIMIZED );
}
void CMainFrame::OnMoreMediaFSP()
{
	ShellExecuteW( NULL, L"open", L"http://store.steampowered.com/app/404570/" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnFilePreferences() 
{
	// flag PREFS DIALOG (not used for the moment, and 754 now used to trigger stock levels to load (above))
	//theApp.SetFileMapData ( 754, 1 );
}

void CMainFrame::OnLevelResizelevel() 
{
	// TODO: Add your command handler code here
	m_LevelResize.DoModal ( );	
}

void CMainFrame::OnClose() 
{
	theApp.SetFileMapData ( 908, 1 );
	theApp.m_Debug.Write ( "CMainFrame::OnClose - 908 = 1" );
}

BOOL CMainFrame::OnQueryEndSession() 
{
	// Must return TRUE for logo compliance
	Sleep ( 100 );
	return TRUE;
}

void CMainFrame::OnEndSession(BOOL bExit) 
{
	// Must be present for logo compliance - force an exit no prompt
	// X10 - 020108 - moved to timer function so mapeditor has chance to close
	Sleep ( 500 );
	SetTimer( 99, 1000, NULL );
}

void CMainFrame::OnEditingSegmentmode() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 1 );
}


void CMainFrame::OnEditingEntitymode() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 2 );
}

void CMainFrame::OnEditingClipboardedit() 
{
	// TODO: Add your command handler code here
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 3 );
}

void CMainFrame::OnEditingZoomin() 
{
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 4 );
}

void CMainFrame::OnEditingZoomout() 
{
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 5 );
}


void CMainFrame::OnEditingMoveupalayer() 
{
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 6 );
}

void CMainFrame::OnEditingMovedownalayer() 
{
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 7 );
}

void CMainFrame::OnEditingViewentirelevel() 
{
	theApp.SetFileMapData ( 712, 1 );
	theApp.SetFileMapData ( 716, 8 );
}

WPARAM  wParam1 = 0;

BOOL CALLBACK EnumWindowsProc1(HWND hwnd,LPARAM lParam )
{
	
	TCHAR szBuffer  [ 256 ];
	
	GetWindowText ( hwnd, szBuffer, 256 );

	// 100105
	if ( theApp.GetFileMapData ( 908 ) == 1 )
		return FALSE;

	_strlwr ( szBuffer );
	if ( strcmp ( szBuffer, _T ( "Game Guru" ) ) == 0 ) //"fpsc map editor" ) ) == 0 )
	{
		LPARAM  lParam = 0;
		lParam = ( LPARAM ) hwnd;
	}

	return TRUE;
}


void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	// 080205 - winme comment out test
	//return;

	//if ( theApp.m_bDebugActivation )
	//	return;

	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	if ( theApp.GetFileMapData ( 424 ) == 1 )
		return;
	
	if ( theApp.GetFileMapData ( 750 ) == 1 )
		return;

	if ( theApp.m_bPopupDisplay )
		return;

	
	
	if ( theApp.m_bBrowserDisplay )
	{
		theApp.m_bAppActive = true;

		//theApp.m_Debug.Write ( "OnActivate - active" );
		wParam1 = WA_ACTIVE;

		if ( theApp.m_pEditorView )
			theApp.m_pEditorView->SetTimer( 0, 1, NULL );

		EnumWindows ( EnumWindowsProc1, 0 );

		return;
	}

	// 110105
	if ( theApp.m_bBuildGameDisplay )
	{
		return;

		theApp.m_bAppActive = true;

		//theApp.m_Debug.Write ( "OnActivate - active" );
		wParam1 = WA_ACTIVE;

		if ( theApp.m_pEditorView )
			theApp.m_pEditorView->SetTimer( 0, 1, NULL );

		EnumWindows ( EnumWindowsProc1, 0 );

		return;
	}

	if ( theApp.m_bBuildGameDisplay )
	{
		return;

		theApp.m_bAppActive = true;

		//theApp.m_Debug.Write ( "OnActivate - active" );
		wParam1 = WA_ACTIVE;

		if ( theApp.m_pEditorView )
			theApp.m_pEditorView->SetTimer( 0, 1, NULL );

		EnumWindows ( EnumWindowsProc1, 0 );

		return;
	}
	

	if ( pWndOther )
	{
		char szPath [ 255 ] = "";
		char szFinal [ 255 ] = "";

		CString szTitle;
		pWndOther->GetWindowText ( szTitle );

		ConvertWideToANSI ( NULL, &szTitle, szPath );

		sprintf ( szFinal, "WINDOW = %s", szPath );

		//theApp.m_Debug.Write ( szFinal );
	}

	if ( nState == WA_INACTIVE )
	{
		if ( theApp.m_bDebugActivation )
			return;

		theApp.m_bAppActive = false;

		//theApp.m_Debug.Write ( "OnActivate - deactive" );
		wParam1 = WA_INACTIVE;

		if ( theApp.m_pEditorView )
			theApp.m_pEditorView->KillTimer ( 0 );

		EnumWindows ( EnumWindowsProc1, 0 );
	}
	else
	{
		theApp.m_bAppActive = true;

		//theApp.m_Debug.Write ( "OnActivate - active" );
		wParam1 = WA_ACTIVE;

		if ( theApp.m_pEditorView )
			theApp.m_pEditorView->SetTimer( 0, 1, NULL );

		EnumWindows ( EnumWindowsProc1, 0 );
	}
}

void CMainFrame::OnEntityView() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 7 );
}

void CMainFrame::OnMarkerView() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 8 );
}

void CMainFrame::OnSegmentView() 
{
	theApp.SetFileMapData ( 60, 1 );
	theApp.SetFileMapData ( 200, 4 );
	theApp.SetFileMapData ( 204, 9 );
}

void CMainFrame::OnUpdateSegmentView(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateMarkerView(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnUpdateEntityView(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable ( theApp.m_bDisable );
}

void CMainFrame::OnAppCredits() 
{
	// 270105 - force redraw
	theApp.m_bBuildGameDisplay = true;
	cCredits credits;
	credits.DoModal ( );
	theApp.m_bBuildGameDisplay = false;
}

void CMainFrame::OnHelpVideosTutorials()
{
	// Go to link
	ShellExecuteW( NULL, L"open", L"http://www.twitch.tv/gamegurulee" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnHelpLUAScriptingAdvice()
{
	// Go to link
	ShellExecuteW( NULL, L"open", L"https://steamcommunity.com/sharedfiles/filedetails/?id=398177770&insideModal=1" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnHelpToggleParentalControl()
{
	// trigger Parental Control popup
	theApp.SetFileMapData ( 754, 1002 );
}

void CMainFrame::OnHelpReadUserManual()
{
	// Go to link
	//ShellExecuteW( NULL, L"open", L"https://www.game-guru.com/downloads/pdfs/GameGuru%20-%20Getting%20Started%20Guide.pdf" , NULL, NULL, SW_SHOWMAXIMIZED );
	ShellExecuteW( NULL, L"open", L"Files\\languagebank\\english\\artwork\\GameGuru - Getting Started Guide.pdf" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnStandaloneEasterGame()
{
	// Run Standalone Game!
	char exepath[2014];
	char exefile[2014];
	GetCurrentDirectory ( 1024, exepath );
	strcat ( exepath, "\\Standalones\\Easter Game\\" );
	strcpy ( exefile, exepath );
	strcat ( exefile, "Easter Game.exe" );
	ShellExecute ( NULL, "open", exefile , NULL, exepath, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIFrameWnd::OnSetFocus(pOldWnd);
	/* no steam overlay for now
	CWnd* hwndDBPApp = FindWindow ( "Game Guru12345", NULL );
	if ( hwndDBPApp )
	{
		hwndDBPApp->SetFocus();
		hwndDBPApp->SetForegroundWindow();
		DWORD process_id = 0;
		GetWindowThreadProcessId(hwndDBPApp->m_hWnd, &process_id);
		AllowSetForegroundWindow(process_id);
	}
	*/
}

void CMainFrame::OnKillFocus(CWnd* pNewWnd) 
{
	CMDIFrameWnd::OnKillFocus(pNewWnd);
}

/*
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	if ( bActive == FALSE )
	{
		theApp.m_Debug.Write ( "OnActivateApp - deactive" );

		// 221204
		wParam1 = WA_INACTIVE;
		EnumWindows ( EnumWindowsProc1, 0 );

	}
	else
	{
		theApp.m_Debug.Write ( "OnActivateApp - active" );

		// 221204
		 //wParam1 = WA_ACTIVE;
		//EnumWindows ( EnumWindowsProc1, 0 );

	}

	CMDIFrameWnd::OnActivateApp(bActive, hTask);	
}
*/

void CMainFrame::ActivateFrame(int nCmdShow) 
{
	// TODO: Add your specialized code here and/or call the base class

	//theApp.m_Debug.Write ( "ActivateFrame" );

	//wParam1 = WA_ACTIVE;
	//	EnumWindows ( EnumWindowsProc1, 0 );

	//theApp.m_Debug.Write ( "ActivateFrame" );
	
	CMDIFrameWnd::ActivateFrame(nCmdShow);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CMDIFrameWnd::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	
}

void CMainFrame::OnEnterIdle( UINT, CWnd* )
{
	int lee=42;
}


//HELPW - create a new help menu option and point it to this function
/*
void CMainFrame::OnHelpwizard()
{
	if ( theApp.m_pHelpWizardDlg )
	{
		if ( theApp.m_bHelpWizardVisible ) 
		{
			//dialog created an open
			theApp.m_bHelpWizardVisible = true;
			theApp.m_pHelpWizardDlg->ShowWindow( SW_RESTORE );
		}
		else 
		{
			//dialog created but closed, restart thread
			theApp.m_bHelpWizardVisible = true;
			theApp.m_pHelpWizardDlg->Start( );
		}
	}
	else
	{
		//dialog not yet created (true if not opened on start-up)
		theApp.m_bHelpWizardVisible = true;
		theApp.m_pHelpWizardDlg = new CHelpWizard();
		theApp.m_pHelpWizardDlg->Start( );
	}
}
//HELPW - end

//AutoUpdate - create a new help menu option and point it to this function
void CMainFrame::OnHelpCheckForUpdates()
{
	// FREE VERSION
	#ifdef FREEVERSION

		DemoMessageBox();

	#else

		CAutoUpdate cAutoUpdateDlg;
		cAutoUpdateDlg.DoModal( );

	#endif
}
*/

void CMainFrame::OnAboutBETA()
{
	// call Help for Beta link
	char szBrowser [ MAX_PATH ];
	theApp.GetDefaultBrowserPath( szBrowser, MAX_PATH );

	// Just shell execute the address directly
	//ShellExecuteW( NULL, L"open", L"http://fpscreator.thegamecreators.com/changelog1_009.php" , NULL, NULL, SW_SHOWMAXIMIZED );
	//ShellExecuteW( NULL, L"open", L"http://fpscreator.thegamecreators.com/changelog1_01.php" , NULL, NULL, SW_SHOWMAXIMIZED );
	//ShellExecuteW( NULL, L"open", L"http://steamcommunity.com/app/266310/announcements/" , NULL, NULL, SW_SHOWMAXIMIZED );
	ShellExecuteW( NULL, L"open", L"https://www.game-guru.com/feature-vote" , NULL, NULL, SW_SHOWMAXIMIZED );
	

	/*
	// V117 - 200510 - use alternative user manual filename if provided
	TCHAR szUserManualFile [ MAX_PATH ];
	GetPrivateProfileString ( _T ( "Comments" ), _T ( "UserManualFile" ), _T ( "" ), szUserManualFile, MAX_PATH, theApp.m_szLanguage );
	CString sFullPath = theApp.m_szDirectory;
	if ( stricmp ( szUserManualFile, "" )!=NULL )
	{
		// used by FPGC and variants
		sFullPath += "\\Docs\\" + CString(szUserManualFile);
	}
	else
	{
		// V110 - 110608 - add FREE if the free version
		if ( g_bFreeVersion )
			sFullPath += "\\Docs\\FPS Creator Manual (Free).pdf";
		else
			sFullPath += "\\Docs\\FPS Creator Manual.pdf";
	}

	ShellExecute( NULL, "open", sFullPath.GetString(), NULL, NULL, SW_SHOW );
	*/
}

void CMainFrame::OnTGCForums()
{
	// call Help for Beta link
	char szBrowser [ MAX_PATH ];
	theApp.GetDefaultBrowserPath( szBrowser, MAX_PATH );
	ShellExecuteW( NULL, L"open", L"https://forum.game-guru.com/" , NULL, NULL, SW_SHOWMAXIMIZED );
}

void CMainFrame::OnSteamForums()
{
	// call Help for Beta link
	char szBrowser [ MAX_PATH ];
	theApp.GetDefaultBrowserPath( szBrowser, MAX_PATH );
	ShellExecuteW( NULL, L"open", L"http://steamcommunity.com/app/266310/discussions/" , NULL, NULL, SW_SHOWMAXIMIZED );
}

//AutoUpdate
void CMainFrame::OnTimer( UINT_PTR nIDEvent )
{
	// FREE VERSION
	#ifdef FREEVERSION
	// no auto update popup in free version
	if ( nIDEvent == 1 )
	{
		KillTimer( 1 );
	}
	#else
	// auto update popup
	if ( nIDEvent == 1 )
	{
		// if no update thread in operation, kill redundant timer
		if ( !theApp.m_pUpdateThread ) KillTimer( 1 );

		// if update thread registers as finished checking (for latest version)
		if ( theApp.m_pUpdateThread->CheckFinished( ) )
		{
			// end timer now its done the job
			KillTimer( 1 );

			// get current version from local file
			char szCurrentVersion [ 64 ];
			strcpy_s( szCurrentVersion, 64, "n/a" );
			FILE *pFile = NULL;
			int error = fopen_s( &pFile, "version.ini", "rb" );
			if ( !error )
			{
				fgets( szCurrentVersion, 64, pFile );
				fclose( pFile );
				pFile = NULL;
			}

			// get latest version from remote file
			const char *szLatestVersion = theApp.m_pUpdateThread->GetLatestVersion( );

			// compare current and latest versions
			if ( strcmp( szLatestVersion, "n/a" ) != 0 )//&& strcmp( szCurrentVersion, szLatestVersion ) != 0 )
			{
				// FPGC - 280809 - need more sophisticated version checker for (1.20.000 > 1.15)
				char lpCurrentValue[64];
				strcpy ( lpCurrentValue, szCurrentVersion );
				if ( strlen(lpCurrentValue) > 4 ) lpCurrentValue[4]=0;
				char lpLatestValue[64];
				strcpy ( lpLatestValue, szLatestVersion );
				if ( strlen(lpLatestValue) > 4 ) lpLatestValue[4]=0;
				float fCurrentValue = (float)atof ( lpCurrentValue );
				float fLatestValue = (float)atof ( lpLatestValue );
				if ( fLatestValue > fCurrentValue )
				{
					// if latest greater than current, call up auto update dialog
					int result = MessageBox( GetLanguageData ( "Auto Update", "UpdateDesc" ), GetLanguageData ( "Auto Update", "Update" ), MB_YESNO );
					if ( result == IDYES )
					{
						CAutoUpdate cAutoUpdateDlg;
						cAutoUpdateDlg.DoModal( );
					}
				}
			}

			// at end of session, release update thread
			delete theApp.m_pUpdateThread;
			theApp.m_pUpdateThread = NULL;
		}
	}
	#endif

	// X10 - 020108 - timer to trigger termination of the interface (one second after mapeditor terminates)
	// because when the shutdown message arrives (vista logo requirement), must shut down mapeditor first (interface needs
	// to stick around for 1 second to handle the exchange of closedown messages)
	if ( nIDEvent == 99 )
	{
		// Now terminate the interface due to vista logo shutdown message
		if ( g_dwToggleRestartManagerFlag==0 )
		{
			// first pass set quit flags
			Sleep ( 100 );
			theApp.SetFileMapData ( 896, 1 );
			theApp.SetFileMapData ( 908, 1 );
			g_dwToggleRestartManagerFlag=1;
		}
		else
		{
			// other passes, quit interface
			Sleep ( 100 );
			PostQuitMessage(0);
		}
	}
}

void CMainFrame::OnTgcStoreHelpClicked()
{
	// V110 BETA4 - 070608 - launch Game Creator Store module
	LaunchGameCreatorStore();
}

void CMainFrame::OnHelpViewitemlicenses()
{
	// Both X9 and X10 copy media into program files (where the media is)
	// so we keep legacy bevaviour from 2005 in defiance of Vista/Win7
	CString sPath = theApp.m_szDirectory;
	sPath += "\\licenses";

	if ( !SetCurrentDirectory( sPath ) )
	{
		// leefix - 210508 - missed out space : MessageBox( GetLanguageData( "TGCStore", "NoLicense" ) );
		MessageBox( GetLanguageData( "TGC Store", "NoLicense" ) );
	}
	else 
	{
		ShellExecute( NULL, "open", "explorer.exe", sPath, NULL, SW_SHOWNORMAL );
	}
}

void CMainFrame::OnHelpWatchstorevideo()
{
	char szBrowser [ MAX_PATH ];
	theApp.GetDefaultBrowserPath( szBrowser, MAX_PATH );
	//ShellExecute ( NULL, "open", szBrowser, "http://www.youtube.com/watch?v=PgWOlP52c-E", "", SW_SHOW );

	// Just shell execute the address directly
	ShellExecuteW( NULL, L"open", L"http://www.youtube.com/watch?v=PgWOlP52c-E" , NULL, NULL, SW_SHOWMAXIMIZED );
}

//----------------------------------------------------
//----------------------------------------------------
//--- GAMEGURU - M-GridEdit
//----------------------------------------------------

// Includes 
#include "stdafx.h"
#include "gameguru.h"
#include "M-WelcomeSystem.h"
#include "M-Widget.h"

#ifdef VRTECH
//Windows Mixed Reality Support
#include "GGVR.h"
#endif

#ifdef ENABLEIMGUI
//PE: GameGuru IMGUI.
#include "..\Imgui\imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "..\Imgui\imgui_internal.h"
#include "..\Imgui\imgui_impl_win32.h"
#include "..\Imgui\imgui_gg_dx11.h"
#endif

#ifdef PETESTING
#include "..\Imgui\imgui_demo.cpp"
#endif

#ifdef VRTECH
#include "M-CharacterCreatorPlusTTS.h"
#endif

#include <algorithm>
#include <string>
#include <time.h>

#include <wininet.h>
#include <mmsystem.h>
#include "ShlObj.h"
#include "sha1.h"
#include "sha2.h"

#include "miniz.h"

#ifdef ENABLEIMGUI
int iGenralWindowsFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove;
bool bBoostIconColors = false;
#endif


bool bTrashcanIconActive = false, bTrashcanIconActive2 = false;

int iWidgetSelection = 0;
bool bRotScaleAlreadyUpdated = false;
float fEditorGridSizeX = 100.0f;
float fEditorGridSizeY = 100.0f;
float fEditorGridSizeZ = 100.0f;
int old_iMSAASampleCount = -1;
int old_iShadowSpotCascadeResolution = -1;
int old_iShadowPointResolution = -1;
bool bUpdateOcean = false;
bool bEditorLight = false;
cStr sNextLevelToLoad;
float fMouseWheelZoomFactor = 1.0;

// Defines
//#define ENABLETUTORIALVIDEOS

// 
//  GAMEGURU MAP EDITOR EXECUTABLE CODE
// 

//Check if we are in f9 mode
extern bool g_occluderf9Mode;
//Skip terrain rendering when going into importer mode
extern bool g_bSkipTerrainRender;
// extern to global that toggles when load map removed from entities
extern bool g_bBlackListRemovedSomeEntities;
extern bool gbWelcomeSystemActive;
extern int g_iWelcomeLoopPage;
extern int g_trialStampDaysLeft;
int g_tstoreprojectmodifiedstatic = 0;

#ifdef ENABLEIMGUI
void process_entity_library(void);
void process_entity_library_v2(void);
// can prevent app from quitting out while in test game
extern bool g_bDisableQuitFlag;
extern bool bEnableWeather;
char cImGuiDebug[2048] = "\0";
bool bLaunchTestGameAfterLoad = false;
char pLaunchAfterSyncPreSelectModel[MAX_PATH] = "\0";
char pLaunchAfterSyncLastImportedModel[MAX_PATH] = "\0";
#endif

#ifdef ENABLEIMGUI
#ifdef VRTECH
extern bool g_bCharacterCreatorPlusActivated;
#else
bool g_bCharacterCreatorPlusActivated;
#endif
int iOldLaunchAfterSync = 0;
int iSkibFramesBeforeLaunch = 0;
bool bForceKey = false;
int iForceScancode = -1;
cstr csForceKey = "";
bool bForceKey2 = false;
cstr csForceKey2 = "";
bool bForceUndo = false;
bool bForceRedo = false;
int iLaunchAfterSync = 0;
DWORD gWindowSizeXOld = 0;
DWORD gWindowSizeYOld = 0;
DWORD gWindowSizeAddY = 0;
DWORD gWindowSizeAddX = 0;
DWORD gWindowVisibleOld = 0;
DWORD gWindowPosXOld = 0;
DWORD gWindowPosYOld = 0;
DWORD gWindowMaximized = 0;
int xmouseold = 0, ymouseold = 0;

extern bool bImGuiInTestGame;
extern bool bBlockImGuiUntilNewFrame;
extern bool bImGuiFrameState;
extern bool bImGuiReadyToRender;
extern bool bImGuiInitDone;
extern ImVec2 OldrenderTargetSize;
extern ImVec2 OldrenderTargetPos;
extern ImVec2 renderTargetAreaPos;
extern ImVec2 renderTargetAreaSize;
extern bool bImGuiRenderTargetFocus;
extern bool bImGuiGotFocus;
extern bool g_bCascadeQuitFlag;
extern int ImGuiStatusBar_Size;
extern char defaultWriteFolder[260];
bool bEntityGotFocus = false;
char cDirectOpen[260];
bool imgui_is_running = false;
int refresh_gui_docking = 0;
ImGuiID dock_main_tabs, dock_tools_windows;
cstr RedockNextWindow;
ImGuiViewport* viewport;
int toolbar_size;
bool g_bInTutorialMode = false;
int g_iCountdownToAlphaBetaMessage = 0;

// for copy and paste operations
//struct sEntityClipboard
//{
//	int iEntityIndex;
//};
//std::vector<sEntityClipboard> g_EntityClipboard;
int g_EntityClipboardAnchorEntityIndex = -1;
std::vector<int> g_EntityClipboard;

class cFolderItem
{
public:
	struct sFolderFiles {
		sFolderFiles * m_dropptr; //Need to be the first entry for drag/drop.
		cStr m_sName;
		cStr m_sNameFinal;
		cStr m_sPath;
		cStr m_sFolder;
		UINT iFlags;
		int iPreview; //Preview image.
		int iBigPreview; //Preview image.
		int id;
		int iAnimationFrom = 0;
		bool bPreviewProcessed;
		long last_used;
		bool bSorted;
		bool bFavorite;
		time_t m_tFileModify;
		cstr m_Backdrop;
		sFolderFiles * m_pNext;
		sFolderFiles * m_pNextTime;
		sFolderFiles * m_pCustomSort;
		cFolderItem *pNewFolder;
		cStr m_sBetterSearch;
		cStr m_sPhoneticSearch;
		int uniqueId;
	};
	cStr m_sFolder;
	cStr m_sFolderFullPath;
	int m_iEntityOffset;
	cFolderItem *m_pNext;
	cFolderItem *m_pSubFolder;
	sFolderFiles * m_pFirstFile;
	bool m_bFilesRead;
	bool visible;
	bool alwaysvisible;
	bool deletethisentry;
	bool bIsCustomFolder;
	char cfolder[256]; //PE: Only for faster sorting.
	time_t m_tFolderModify;
	float m_fLastTimeUpdate;
	UINT iFlags;
	int count;
	int iType;
	cFolderItem() { m_pNext = 0; iType = 0; iFlags = 0; m_bFilesRead = false; m_pFirstFile = NULL; m_pNext = NULL; m_pSubFolder = NULL; m_fLastTimeUpdate = 0; m_iEntityOffset = 0; }
	~cFolderItem() { }
};

extern preferences pref;

extern cFolderItem MainEntityList;

bool bExport_Standalone_Window = false;
bool bExport_SaveToGameCloud_Window = false;
bool bExternal_Entities_Window = false;
int iDisplayLibraryType = 0;
int iLastDisplayLibraryType = -1;
cstr sStartLibrarySearchString = "";
int iLibraryStingReturnToID = 0;
int iSelectedLibraryStingReturnID = -1;
cstr sSelectedLibrarySting = "";
//bool bExternal_Entities_Init = false;
bool bEntity_Properties_Window = false;
bool bProperties_Window_Block_Mouse = false;
bool bCheckForClosing = false;
bool bCheckForClosingForce = false;
bool bBuilder_Properties_Window = false;
bool bBuilder_Left_Window = false;
bool bTerrain_Tools_Window = false;
bool bWaypoint_Window = false;
bool bDownloadStore_Window = false;
bool bImporter_Window = false;
bool bHelpVideo_Window = false;
bool bHelp_Window = false;
extern char cForceTutorialName[1024];
bool bHelp_Menu_Image_Window = false;
bool bAbout_Window = false;
bool bCredits_Window = false;
bool bBug_Reporting_Window = false;
bool bBug_RefreshBugList = false;
bool bAbout_Window_First_Run = false;
bool bCredits_Window_First_Run = true;
bool bAbout_Init = false;
bool Entity_Tools_Window = true;
bool bInfo_Window = false;
bool bInfo_Reload = false;
bool bInfo_Window_First_Run = true;
cstr cInfoMessage = "";
cstr cInfoImage = "", cInfoImageLast = "";
int iInfoUniqueId = 0;

int media_icon_size_leftpanel = 64;
int iColumnsWidth_leftpanel = 110;
int iColumns_leftpanel = 0;
bool bDisplayText_leftpanel = true;
float fFontSize_leftpanel = 1.0;

#define MAXTEXTINPUT 1024
char cTmpInput[MAXTEXTINPUT + 1];
int grideleprof_uniqui_id = 35000;
//entityeleproftype backup_grideleprof;  //PE: We dont use a cancel system anymore.
cFolderItem::sFolderFiles *pDragDropFile = NULL;
int iOldgridentity = -1;
float fPropertiesColoumWidth = 100.0f;
bool bTriggerMessage = false;
char cTriggerMessage[MAX_PATH] = "\0";
int iMessageTimer = 0;
ImVec4 drawCol_back;
ImVec4 drawCol_normal;
ImVec4 drawCol_hover;
ImVec4 drawCol_Down;

std::vector<cstr> tutorial_list; //unsorted.
std::map<std::string, std::string> tutorial_files;
std::map<std::string, std::string> tutorial_videos;
std::map<std::string, std::string> tutorial_description;
std::vector<cstr> about_text; //unsorted.

bool g_bRuntimePhysicsDebugging = true;
bool bTutorial_Init = false;
int current_tutorial = -1;
int selected_tutorial = 0;
bool bVideoResumePossible = false;
bool bVideoPerccentStart = false;
int iVideoFindFirstFrame = 0;
int iVideoDelayExecute = 0;
bool bTutorialCheckAction = false;
int bDelayedTutorialCheckAction = -1;
#define TUTORIALMAXTEXT 1024
#define TUTORIALMAXSTEPS 20
char cForceTutorialName[1024] = "\0";
char cTutorialName[TUTORIALMAXTEXT] = "\0";
cstr cVideoDescription = "";
struct ActiveTutorial {
	bool bActive = false;
	int iSteps = 0;
	char cStartText[TUTORIALMAXTEXT] = "\0";
	char cVideoPath[TUTORIALMAXTEXT] = "\0";
	bool bVideoReady = false;
	bool bVideoInit = false;
	int bVideoID = 0;
	char cStepHeader[TUTORIALMAXSTEPS][TUTORIALMAXTEXT] = { "\0" };
	char cStepText[TUTORIALMAXSTEPS][TUTORIALMAXTEXT] = { "\0" };
	char cStepAction[TUTORIALMAXSTEPS][TUTORIALMAXTEXT] = { "\0" };
	ImVec2 vOffsetPointer[TUTORIALMAXSTEPS] = { ImVec2(0, 0) };
	int iCurrent_Step = 0;
	float fScore = 0;
} tut;
bool bTutorialRendered = false;
bool bSmallVideoFrameStart = true;
bool bSetTutorialSectionLeft = false;

//Tooltip object code.
int iLastTooltipSelection = -1;
int iTooltipTimer = 0;
int iTooltipHoveredTimer = 0;
int iTooltipLastObjectId = 0;
bool iTooltipAlreadyLoaded = true;
bool iTooltipObjectReady = false;
float lastKeyTime = 0;
char cHelpMenuImage[MAX_PATH];
bool bLostFocus = false;
bool bRenderTargetModalMode = false;
int iStartupTime = 0;
cstr CurrentWinTitle = "";
//int speech_ids[5];

extern bool bWaypointDrawmode;
extern float custom_back_color[4];
extern bool bUpdateVeg;
extern int iLastUpdateVeg;

bool TutorialNextAction(void);
bool CheckTutorialPlaceit(void);
bool CheckTutorialAction(const char * action, float x_adder = 0.0f );
void RenderToPreview(int displayobj);
void CheckTooltipObjectDelete(void);
void get_tutorials(void);
void SmallTutorialVideo(char *tutorial,char* combo_items[] = NULL, int combo_entries = 0,int iVideoSection = 0);
void SetVideoVolume(float volume);
void ProcessPreferences(void);
void CloseAllOpenTools(bool bTerrainTools = true);
void CloseAllOpenToolsThatNeedSave(void);
void imgui_shooter_tools(void);
float ApplyPivot(sObject* pObject, int iMode, GGVECTOR3 vecValue, float fValue);
void imgui_terrain_loop(void);
void imgui_terrain_loop_v2(void);

char * imgui_setpropertystring2(int group, char* data_s, char* field_s, char* desc_s);
int imgui_setpropertylist2(int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype);
char * imgui_setpropertylist2c(int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype);
char * imgui_setpropertyfile2(int group, char* data_s, char* field_s, char* desc_s, char* within_s);
char * imgui_setpropertyfile2_dlua(int group, char* data_s, char* field_s, char* desc_s, char* within_s);

void ParseLuaScript(entityeleproftype *tmpeleprof, char * script);
int DisplayLuaDescription(entityeleproftype *tmpeleprof);
int DisplayLuaDescriptionOnly(entityeleproftype *tmpeleprof);
float fDisplaySegmentText(char *text);
void SpeechControls(int speech_entries, bool bUpdateMainString, entityeleproftype *edit_grideleprof=NULL);
void RedockWindow(char *name);
void CheckMinimumDockSpaceSize(float minsize);
void generic_preloadfiles(void);
void CloseDownEditorProperties(void);

#endif

bool bExternal_Entities_Init = false; //moved  //cyb
int speech_ids[5]; //moved //cyb

// moved here so Classic would compile
bool Shooter_Tools_Window_Active = false;
void DeleteWaypointsAddedToCurrentCursor(void);
void Add_Grid_Snap_To_Position(void);
float ImGuiGetMouseX(void);
float ImGuiGetMouseY(void);
void RotateAndMoveRubberBand(int iActiveObj, float fMovedActiveObjectX, float fMovedActiveObjectY, float fMovedActiveObjectZ, GGQUATERNION quatRotationEvent); //float fMovedActiveObjectRX, float fMovedActiveObjectRY, float fMovedActiveObjectRZ);
void SetStartPositionsForRubberBand(int iActiveObj);

void set_inputsys_mclick(int value)
{
	t.inputsys.mclick = value;
	//char pDebugMouseClick[32];
	//sprintf(pDebugMouseClick, "inputsys.mclick = %d", t.inputsys.mclick);
	//timestampactivity(0, pDebugMouseClick);
}

// GLOBAL to know when in welcome area
int iTriggerWelcomeSystemStuff = 0;

int iCountDownToShowQuickStartDialog = 0;

int gguishadereffectindex = 0;
// mapeditor inits and loop call
void mapeditorexecutable_init ( void )
{
	//  Means we are in the editor (1) or in standalone game (0)
	timestampactivity(0,"ide input mode");
	g.globals.ideinputmode = 1;

	// VR for now to speed up launch
	bool bSkipVRForNow = false;

	// No VR or RIFTMODE in Editor Mode
	g.globals.riftmode = 0;
	#ifdef VRTECH
	g.vrglobals.GGVREnabled = 0; 
	if (bSkipVRForNow == true)
	{
		timestampactivity(0, "VR System disabled to improve launch speed (temporary)");
		g.vrglobals.GGVRUsingVRSystem = 1;
	}
	else
	{
		g.vrglobals.GGVRUsingVRSystem = 1;
		if (g.gvrmode == 2) g.vrglobals.GGVREnabled = 1; // OpenVR (Steam)
		if (g.gvrmode == 3) g.vrglobals.GGVREnabled = 2; // Windows Mixed Reality (Microsoft)
		char pVRSystemString[1024];
		sprintf(pVRSystemString, "choose VR system with mode %d", g.vrglobals.GGVREnabled);
		timestampactivity(0, pVRSystemString);
		int iErrorCode = GGVR_ChooseVRSystem(g.vrglobals.GGVREnabled, g.gproducelogfiles, "");// cstr(g.fpscrootdir_s + "\\GGWMR.dll").Get() );
		if (iErrorCode > 0)
		{
			// if VR headset is not present, switch VR off to speed up non-VR rendering (especially for debug)
			char pErrorStr[1024];
			sprintf(pErrorStr, "Error Choosing VR System : Code %d", iErrorCode);
			timestampactivity(0, pErrorStr);
			timestampactivity(0, "switching VR off, headset not detected");
			g.vrglobals.GGVREnabled = 0;
		}
		else
		{
			//PE: Only if we use vr.
			if (g.gvrmode > 0)
			{
				// Give portal enough time to start its launch, then get rid of GameWindow until we need it!
				//Sleep(1900); seems daft pausing every launch just in case WMR Portal is not already active, reduce
				Sleep(10);
				CloseWindow(g_pGlob->hOriginalhWnd);
				Sleep(10);
				g_pGlob->hOriginalhWnd = NULL;
				SetWindowPos(g_pGlob->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
		}
	}
	#endif
	
	//  Set device to get multisampling AA active in editor
	t.multisamplingfactor=0;
	t.multimonitormode=0;

	//  Init app
	timestampactivity(0,"sync states");
	SyncOn (   ); SyncRate (  0 );
	t.strwork = "" ; t.strwork = t.strwork + t.strarr_s[475]+" - [Editor]";
	timestampactivity(0,"window states");
	ShowWindow ( ); WindowToFront (  t.strwork.Get() );
	AlwaysActiveOff ( );

	#ifdef VRTECH
	// start thread loader for for generic (startup) files (multi-threaded loading)
	generic_preloadfiles();
	#endif

	// So entirely replace fixed function rendering, use this shader effect
	g.guishadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_basic.fx");
	gguishadereffectindex = g.guishadereffectindex;
	g.guidiffuseshadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_diffuse.fx");
	g.guiwireframeshadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_wireframe.fx");
	g.guidepthshadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_showdepth.fx");

	//  Camera aspect ratio adjustment for desktop resolution
	timestampactivity(0,"camera states");
	t.aspect_f=GetDesktopWidth() ; t.aspect_f=t.aspect_f/GetDesktopHeight();
	SetCameraAspect ( t.aspect_f );

	//  Set editor to use a true 1;1 pixel mapping for Text ( , Steam GUI and other overlay images )
	// moved higher up
	//timestampactivity(0,"pixel states");
	//SetChildWindowTruePixel ( 1 );
	//common_refreshDisplaySize ( );

	// 111115 - base start memory for GameGuru (overwritten if g.grestoreeditorsettings==0)
	timestampactivity(0,"memory states");
	g.gamememactuallyusedstart=SMEMAvailable(1);

	#ifdef VRTECH
	// Reset texture/profile in EBE folder
	ebe_restoreebedefaulttextures();
	#endif

	//  Early editor only inits
	timestampactivity(0,"pre widget init state");
	t.tsplashstatusprogress_s="WIDGET INIT";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	timestampactivity(0,"widget status update");
	version_splashtext_statusupdate ( );
	widget_init ( );

	t.tsplashstatusprogress_s="SLIDERS INIT";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	sliders_init ( );

	//  Generic asset loading common to editor and game
	t.tresetforstartofeditor=1;
	t.tsplashstatusprogress_s="LOAD COMMON ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	common_loadcommonassets ( 0 );

	//  Initialise meshes and editor resources
	t.tsplashstatusprogress_s="INIT EDITOR RESOURCES";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	t.lastgrideditselect=-1;
	g.gmapeditmode = 1;
	editor_init ( );

	//  Load resource file which has test game memory usage data contained
	t.tsplashstatusprogress_s="LOAD MAIN RESOURCES";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	loadresource();

	//  Call visuals loop once to set shader constants
	t.tsplashstatusprogress_s="UPDATE VISUAL SETTINGS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	t.visuals=t.editorvisuals;
	t.visuals.refreshshaders=1;
	visuals_loop ( );
	visuals_shaderlevels_update ( );

	// Load map editior settings
	t.bTriggerNewMapAtStart = true;
	t.bIgnoreFirstCallToNewLevel = false;
	if ( g.grestoreeditorsettings == 1 ) 
	{
		t.tsplashstatusprogress_s="RESTORE LAST PROJECT";
		timestampactivity(0,t.tsplashstatusprogress_s.Get());
		version_splashtext_statusupdate ( );
		//popup_text_close();
		t.tfile_s = g.mysystem.editorsGridedit_s+"cfg.cfg";//"editors\\gridedit\\cfg.cfg";
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			//  Load last Editor CFG Settings
			t.skipfpmloading=1;
			editor_loadcfg ( );

			//  load project specified in CFG (worklevel.fpm?)
			mapfile_loadproject_fpm ( );

			//  Now wipe config (in case we fail to load in restart, we avoid an infinite loop)
			if (  FileExist(t.tfile_s.Get()) == 1  ) DeleteAFile ( t.tfile_s.Get() );

			//  load in current files in LEVELBANK\TESTMAP (not from FPM)
			gridedit_load_map ( );
			t.skipfpmloading=0;
		}
	}
	else
	{
		// Start Splash (only one which does not wait for Sync ( -as interface not avail. in debug) )
		t.tsplashstatusprogress_s="";
		timestampactivity(0,t.tsplashstatusprogress_s.Get());
		version_splashtext_statusupdate ( );
		//version_splashtext ( );
	}

	//  trigger zoom to aquire camera range for editor
	t.updatezoom=1;

	//  Start resource bar must accurately reflect ALL data loaded by editor
	if ( g.grestoreeditorsettings==0 )
	{
		t.gamememactuallyusedstarttriggercount = 5; // 111115 - one trigger to get STARTMEM at beginning of GameGuru session execution (honest value)
	}

	//  Var to control machine independent speed
	game_timeelapsed_init ( );
	t.tsl_f=Timer();

	// IDE announcement system (note VR Quest has this option)
	#ifdef ENABLEIMGUI
	// (note VR Quest has this option)
	iTriggerWelcomeSystemStuff = 1;
	#else
	iTriggerWelcomeSystemStuff = 0;
	#endif
	if ( g.gshowannouncements == 1 )
	{
		welcome_init(1);
		welcome_init(0);
		welcome_show(WELCOME_ANNOUNCEMENTS);
	}

	// only show front dialogs if not resuming from previous session
	int iCountDownToShowQuickStartDialog = 0;
	if ( g.grestoreeditorsettings == 0 ) 
	{
		// Welcome quick start page
		g.quickstartmenumode = 0;
		if ( g.iFreeVersionModeActive != 0 )
		{
			editor_showquickstart ( 0 );
			welcome_free();
		}
		else
		{
			if (g.gshowonstartup == 1 || g.iTriggerSoftwareToQuit != 0) 
			{
				editor_showquickstart(0);
				welcome_free(); //PE: We must always close it for level auto load to work.
			}
			else
			{
				welcome_free();
			}
		}
	}
	else
	{
		// always need to close down loading splash
		welcome_free();
	}

	// After 5 minutes, trigger another trial reminder
	DWORD dwStartOfEditingSession = timeGetTime() + (1000*60*5);
	DWORD dwSecondReminder = 0;

	#ifdef ENABLEIMGUI
	//Load needed images.
	image_preload_files_reset(); //PE: At this point we have no more thread loaded images to use.
	SetMipmapNum(1); //PE: mipmaps not needed.
	image_setlegacyimageloading(true);
	LoadImage("editors\\uiv3\\shape.png", TOOL_SHAPE);
	LoadImage("editors\\uiv3\\level.png", TOOL_LEVELMODE);
	LoadImage("editors\\uiv3\\storedlevel.png", TOOL_STOREDLEVEL);
	LoadImage("editors\\uiv3\\blendmode.png", TOOL_BLENDMODE);
	LoadImage("editors\\uiv3\\rampmode.png", TOOL_RAMPMODE);
	LoadImage("editors\\uiv3\\painttexture.png", TOOL_PAINTTEXTURE);
	LoadImage("editors\\uiv3\\paintgrass.png", TOOL_PAINTGRASS);
	LoadImage("editors\\uiv3\\entity.png", TOOL_ENTITY);
	LoadImage("editors\\uiv3\\markers.png", TOOL_MARKERS);
	LoadImage("editors\\uiv3\\waypoints.png", TOOL_WAYPOINTS);
	LoadImage("editors\\uiv3\\newwaypoints.png", TOOL_NEWWAYPOINTS);
	LoadImage("editors\\uiv3\\testgame.png", TOOL_TESTGAME);
	LoadImage("editors\\uiv3\\vrmode.png", TOOL_VRMODE);
	LoadImage("editors\\uiv3\\socialvr.png", TOOL_SOCIALVR);
	LoadImage("editors\\uiv3\\newlevel.png", TOOL_NEWLEVEL);
	LoadImage("editors\\uiv3\\loadlevel.png", TOOL_LOADLEVEL);
	LoadImage("editors\\uiv3\\savelevel.png", TOOL_SAVELEVEL);
	LoadImage("editors\\uiv3\\rounding_overlay_style0-h.png", ROUNDING_OVERLAY);
	LoadImage("editors\\uiv3\\ebe-block.png", EBE_BLOCK);
	LoadImage("editors\\uiv3\\ebe-column.png", EBE_COLUMN);
	LoadImage("editors\\uiv3\\ebe-cube.png", EBE_CUBE);
	LoadImage("editors\\uiv3\\ebe-floor.png", EBE_FLOOR);
	LoadImage("editors\\uiv3\\ebe-new.png", EBE_NEW);
	LoadImage("editors\\uiv3\\ebe-row.png", EBE_ROW);
	LoadImage("editors\\uiv3\\ebe-stairs.png", EBE_STAIRS);
	LoadImage("editors\\uiv3\\ebe-wall.png", EBE_WALL);
	LoadImage("editors\\uiv3\\builder.png", TOOL_BUILDER);
	LoadImage("editors\\uiv3\\ccp.png", TOOL_CCP);
	LoadImage("editors\\uiv3\\import.png", TOOL_IMPORT);
	LoadImage("editors\\uiv3\\media-play.png", MEDIA_PLAY);
	LoadImage("editors\\uiv3\\media-pause.png", MEDIA_PAUSE);
	LoadImage("editors\\uiv3\\media-refresh.png", MEDIA_REFRESH);
	LoadImage("editors\\uiv3\\media-record.png", MEDIA_RECORD);
	LoadImage("editors\\uiv3\\media-recording.png", MEDIA_RECORDING);
	LoadImage("editors\\uiv3\\media-recordprocessing.png", MEDIA_RECORDPROCESSING);
	LoadImage("editors\\uiv3\\pointer2.png", TUTORIAL_POINTER);
	LoadImage("editors\\uiv3\\pointer3.png", TUTORIAL_POINTERUP);	
	#ifdef USETOOLBARHEADER
	LoadImage("editors\\uiv3\\theader.png", TOOL_HEADER);
	#endif
	LoadImage("editors\\uiv3\\ABOUT-Logo.png", ABOUT_LOGO);
	LoadImage("editors\\uiv3\\ABOUT-TGC.png", ABOUT_TGC);
	#ifdef PRODUCTV3
	LoadImage("editors\\uiv3\\ABOUT-Country.png", ABOUT_HB);
	#endif
	LoadImage("editors\\uiv3\\ebe-control1.png", EBE_CONTROL1);
	LoadImage("editors\\uiv3\\ebe-control2.png", EBE_CONTROL2);
	LoadImage("editors\\uiv3\\shape-up.png", TOOL_SHAPE_UP);
	LoadImage("editors\\uiv3\\shape-down.png", TOOL_SHAPE_DOWN);
	LoadImage("editors\\uiv3\\drawwaypoints.png", TOOL_DRAWWAYPOINTS);
	LoadImage("editors\\uiv3\\dotcircle.png", TOOL_DOTCIRCLE);
	LoadImage("editors\\uiv3\\dotcircles.png", TOOL_DOTCIRCLE_S);
	LoadImage("editors\\uiv3\\dotcirclem.png", TOOL_DOTCIRCLE_M);
	#ifdef PRODUCTV3
	#else
	LoadImage("editors\\uiv3\\circle.png", TOOL_CIRCLE);
	LoadImage("editors\\uiv3\\circles.png", TOOL_CIRCLE_S);
	LoadImage("editors\\uiv3\\circlem.png", TOOL_CIRCLE_M);
	#endif



	image_setlegacyimageloading(false);
	SetMipmapNum(-1);
	ChangeGGFont("editors\\uiv3\\Roboto-Medium.ttf",15);

	extern char launchLoadOnStartup[260];
	if (strlen(launchLoadOnStartup) > 0 ) 
	{
		strcpy(cDirectOpen, launchLoadOnStartup);
		if (strlen(cDirectOpen) > 0) {

			t.returnstring_s = cDirectOpen;
			if (t.returnstring_s != "")
			{
				if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) == ".fpm")
				{
					t.gridentity = 0;
					t.inputsys.constructselection = 0;
					t.inputsys.domodeentity = 1;
					t.grideditselect = 5;
					editor_refresheditmarkers();

					g.projectfilename_s = t.returnstring_s;
					gridedit_load_map();

					//Locate player start marker.
					for (t.e = 1; t.e <= g.entityelementlist; t.e++)
					{
						if (t.entityelement[t.e].bankindex > 0)
						{
							if (t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 1 && t.entityprofile[t.entityelement[t.e].bankindex].lives != -1)
							{
								//Point camera.
								t.obj = t.entityelement[t.e].obj;
								if (t.obj > 0) {
									float offsetx = ((float)GetDesktopWidth() - renderTargetAreaSize.x) * 0.25f;
									t.cx_f = ObjectPositionX(t.obj) + offsetx; //t.editorfreeflight.c.x_f;
									t.cy_f = ObjectPositionZ(t.obj); //t.editorfreeflight.c.z_f;
								}
								break;
							}
						}
					}

				}
			}
		}

		iLastUpdateVeg = 0;
		bUpdateVeg = true;
	}
	else 
	{
	}

	#ifdef USERENDERTARGET
	//PE: redirect all to image.
	SetCameraToImage(0, g.postprocessimageoffset, GetDisplayWidth(), GetDisplayHeight(), 2);
	imgui_is_running = true;
	#endif

	//Allow drag drop from outside.
	//DragAcceptFiles(g_pGlob->hWnd, TRUE); - cannot find this with 64 bit compile!

	//Make sure we have envmap.
	visuals_justshaderupdate();
	t.visuals.refreshskysettingsfromlua = true;
	cubemap_generateglobalenvmap();
	t.visuals.refreshskysettingsfromlua = false;

	t.visuals.VegQuantity_f = t.gamevisuals.VegQuantity_f;
	t.visuals.VegWidth_f = t.gamevisuals.VegWidth_f;
	t.visuals.VegHeight_f = t.gamevisuals.VegHeight_f;
	grass_setgrassgridandfade();

	if (!(ObjectExist(t.tGrassObj) == 1 && GetMeshExist(t.tGrassObj) == 1))
		grass_init();

	t.terrain.grassupdateafterterrain = 1;
	grass_loop();
	t.terrain.grassupdateafterterrain = 0;
	ShowVegetationGrid();
	visuals_justshaderupdate();

	// Moved last so we can load levels before main loop.
	// start thread loader for Character Creator texture files (multi-threaded loading) (saves 2s if started CCP)
	#ifdef VRTECH
	timestampactivity(0, "preload CCP textures early");
	charactercreatorplus_preloadinitialcharacter();
	#endif

	//  Main loop
	iStartupTime = Timer();
	timestampactivity(0, "Guru Map Editor Loop Starts");

	//Default to TERRAIN TOOL panel
	bForceKey = true;
	csForceKey = "t";
	bForceKey2 = true;
	csForceKey2 = "6";
	t.inputsys.domodeterrain = 1; t.inputsys.dowaypointview = 0;
	t.terrain.terrainpaintermode = 1;// 6; start off in sculpt mode
	bTerrain_Tools_Window = true;
	t.gridentitymarkersmodeonly = 0; 
	t.grideditselect = 0;
	#else
	timestampactivity(0,"Guru Map Editor Loop Starts");
	#endif
	
	// trigger an alha/beta prompt

}

#ifdef ENABLEIMGUI
void mapeditorexecutable_loop_leavetestgame(void)
{
	bBlockImGuiUntilNewFrame = true;
	SetCameraToImage(0, g.postprocessimageoffset, GetDisplayWidth(), GetDisplayHeight(), 2); //switch back to render target.
	iLaunchAfterSync = 0;
	bImGuiInTestGame = false;
	sky_show(); //Restore skybox.
	iLastUpdateVeg = 0; //Veg: update any changes from F9
	bUpdateVeg = true;
}
#endif

int back_iLastResolutionWidth = 0;
int back_iLastResolutionHeight = 0;

bool commonexecutable_loop_for_game(void)
{
#ifdef ENABLEIMGUI //#ifndef PRODUCTCLASSIC //cyb
	// called from both mapeditor(test game) and standalone game
	if (iLaunchAfterSync == 201)
	{
		//As we set 201 launch testgame after we have a imgui frame, we need to set Scissors here.
		bImGuiInTestGame = true;
		g_bDisableQuitFlag = true;

		//PE: Use the actual screen resolution, not the windows size.
		extern ImVec2 fImGuiScissorTopLeft;
		extern ImVec2 fImGuiScissorBottomRight;
		fImGuiScissorTopLeft = { 0, 0 };
		fImGuiScissorBottomRight = { (float)GetSystemMetrics(SM_CXSCREEN),  (float)GetSystemMetrics(SM_CYSCREEN) };

		//PE: Used by LUA.
		extern DWORD g_dwScreenWidth;
		extern DWORD g_dwScreenHeight;
		g_dwScreenWidth = fImGuiScissorBottomRight.x;
		g_dwScreenHeight = fImGuiScissorBottomRight.y;
		g_pGlob->iScreenWidth = fImGuiScissorBottomRight.x;
		g_pGlob->iScreenHeight = fImGuiScissorBottomRight.y;


		g_bDisableQuitFlag = false;
		t.postprocessings.fadeinvalue_f = 0.0f; //PE: Make sure we trigger default settings like music / volume ...


		iLaunchAfterSync = 202;
		return true;
	}
	if (iLaunchAfterSync == 202)
	{
		bImGuiInTestGame = true;
		g_bDisableQuitFlag = true;
		if (editor_previewmap_loopcode(0) == true)
		{
			// when loop ends, run code after loop
			bImGuiInTestGame = false;
			bBlockImGuiUntilNewFrame = true;
			editor_previewmap_afterloopcode(0);
			mapeditorexecutable_loop_leavetestgame();

			// mapeditor or standalone game
			if (t.game.gameisexe == 1)
			{
				// trigger exit from game
				iLaunchAfterSync = 0;
				PostQuitMessage(0);
			}
			else
			{
				// map editor restore

				// continue
				iSkibFramesBeforeLaunch = 2;
				iLaunchAfterSync = 203;
			}

			//PE: Reset if we have any hanging keys from test game.
			ImGuiIO& io = ImGui::GetIO();
			io.KeySuper = false;
			io.KeyCtrl = false;
			io.KeyAlt = false;
			io.KeyShift = false;
			for (int iTemp = 0; iTemp < 256; iTemp++)
			{
				io.KeysDown[iTemp] = 0;
			}
			io.MouseDown[0] = 0; //PE: Mouse (release) is also loast inside blocking dialogs. Reset!
			io.MouseDown[1] = 0;
			io.MouseDown[2] = 0;
			io.MouseDown[3] = 0;

		}
		g_bDisableQuitFlag = false;
		return true;
	}
#endif

	// allow continue on to editor if appropriate
	return false;
}

void gridedit_setsmartobjectvisibilityinrubberband(bool bVisible)
{
	if (g.entityrubberbandlist.size() > 0)
	{
		for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
		{
			int e = g.entityrubberbandlist[i].e;
			if (t.entityprofile[t.entityelement[e].bankindex].ischildofgroup != 0)
			{
				if (t.entityprofile[t.entityelement[e].bankindex].ismarker != 0)
				{
					if (bVisible)
						ShowObject(t.entityelement[e].obj);
					else
						HideObject(t.entityelement[e].obj);
				}
			}
		}
	}
}

void mapeditorexecutable_loop(void)
{
	#ifdef ENABLEIMGUI
	bSmallVideoFrameStart = true;
	// special modes used when in test game or standalone game
	if (commonexecutable_loop_for_game() == true) return;

	//PE: Some function require we have a empty imgui , so launch here.
	extern bool g_bNoSwapchainPresent;


	switch(iLaunchAfterSync)
	{
		case 1:  // Test Game
		case 20: // VR Test Game

			if (iLaunchAfterSync == 20 && g.gvrmode == 0)
			{
				HWND hThisWnd = GetForegroundWindow();
				MessageBoxA(hThisWnd, "You are not in VR mode. You need to exit the software. When you restart, select VR MODE ON to enable VR.", "Not in VR Mode", MB_OK);
				iLaunchAfterSync = 0;
			}
			else
			{
				bImGuiInTestGame = true;
				bool bTestInVRMode = false;
				if (iLaunchAfterSync == 20) bTestInVRMode = true;
				RunCode(0); //switch to backbuffer 
				// g.projectmodified = 1; if just testing, do not assume a modification!
				// ensure threads loading resources are silent before test game
				image_preload_files_wait();
				object_preload_files_wait();
				image_preload_files_reset();
				object_preload_files_reset();
				if ( bTestInVRMode == false )
				{
					editor_previewmap(0);
					mapeditorexecutable_loop_leavetestgame();
				}
				else
				{
					RunCode(0); //switch to backbuffer
					editor_previewmap(1);
					mapeditorexecutable_loop_leavetestgame();
					SetCameraToImage(0, g.postprocessimageoffset, GetDisplayWidth(), GetDisplayHeight(), 2); //switch back to render target.
					bImGuiInTestGame = false;
				}
				iLaunchAfterSync = 0;
				sky_show(); //Restore skybox.
				iLastUpdateVeg = 0; //Veg: update any changes from F9
				bUpdateVeg = true;
				break;
			}
			break;

		case 30: //Create thumbnail.
		{
			if (iTooltipLastObjectId > 0) {
				int drawobj = g.entitybankoffset + iTooltipLastObjectId;
				g_bNoSwapchainPresent = true; //dont present backbuffer to HWND.
				RenderToPreview(drawobj);
			}
			iLaunchAfterSync = 31;
			break;
		}
		case 31: //Switch back to presenting the swapchain.
		{
			g_bNoSwapchainPresent = false;
			iLaunchAfterSync = 0;
			break;
		}

		case 21: //Social VR.
		{
			bImGuiInTestGame = true;
			RunCode(0); //switch to backbuffer
			editor_multiplayermode();
			bBlockImGuiUntilNewFrame = true;
			SetCameraToImage(0, g.postprocessimageoffset, GetDisplayWidth(), GetDisplayHeight(), 2); //switch back to render target.
			bImGuiInTestGame = false;
			iLaunchAfterSync = 0;
			sky_show(); //Restore skybox.
			iLastUpdateVeg = 0; //Veg: update any changes from F9
			bUpdateVeg = true;
			break;
		}

		default:
			break;
	}

	//Display Weather. ALLOW_WEATHER_IN_EDITOR
	#ifdef VRTECH
	extern bool bEnableWeather; //cyb
	if (bEnableWeather) 
	{
		update_env_particles();
		ravey_particles_update();
	}
	#endif

	//PE: Imgui variables.
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//PE: To solve problem with ALT-TAB away and key "release" are sent to another app.
	HWND tmpHwnd = GetFocus();
	if (bLostFocus == false && tmpHwnd != g_pGlob->hWnd) 
	{
		bLostFocus = true;
		io.KeySuper = false;
		io.KeyCtrl = false;
		io.KeyAlt = false;
		io.KeyShift = false;

		t.inputsys.keyreturn = 0;
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.keycontrol = 0;
		t.inputsys.keyspace = 0;
		t.inputsys.kscancode = 0;

		for (int iTemp = 0; iTemp < 273; iTemp++)
			io.KeysDown[iTemp] = 0;
	}
	if (tmpHwnd == g_pGlob->hWnd) 
	{
		if (bLostFocus) {
			//We got focus again.
			//Looks like its fine to reset keys on lost only.
			//If there is a problem, they can also be reset here.
		}
		bLostFocus = false;
	}

	// set when showonstartup.ini does not exist and is created (first run sorts out UI panels)
	// and it seems, fixes the issue of 'bImGuiGotFocus' being true on some laptops on first run!?
	if (g.gfirsttimerun == 1)
	{
		g.gfirsttimerun = 0;
		refresh_gui_docking = 0;
		pref.vStartResolution = { 1280,800 };
		pref.iMaximized = 1;
		SetWindowSize(pref.vStartResolution.x, pref.vStartResolution.y);
		float centerx = (GetDesktopWidth()*0.5) - (pref.vStartResolution.x*0.5);
		float centery = ((float)(GetDesktopHeight()*0.5) - (float)(pref.vStartResolution.y*0.5)) * 0.5f;
		if (centerx < 0) centerx = 0;
		if (centery < 0) centery = 0;
		SetWindowPosition(centerx, centery);
		MaximiseWindow();
	}


	ImVec4 style_back = ImGui::GetStyle().Colors[ImGuiCol_Text];
	ImVec4 style_winback = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	style_back = ImVec4(1.0, 1.0, 1.0, 1.0);

	bBoostIconColors = false;

	if (pref.current_style == 3) 
	{
		drawCol_back = ImColor(255, 255, 255, 128)*style_back;
		drawCol_normal = ImColor(255, 255, 255, 255);
		drawCol_hover = ImColor(180, 180, 180, 230);
	}
	else if (pref.current_style == 25) {
		drawCol_back = ImColor(255, 255, 255, 128)*style_back;
		drawCol_normal = ImColor(255, 255, 255, 255);
		drawCol_hover = ImColor(180, 180, 180, 230);
		bBoostIconColors = true;
	}
	else 
	{
		drawCol_back = ImColor(255, 255, 255, 128)*style_back;
		drawCol_normal = ImColor(220, 220, 220, 230)*style_back;
		drawCol_hover = ImColor(255, 255, 255, 255)*style_back;
	}
	drawCol_Down = ImColor(180, 180, 160, 255)*style_back;
	ImVec4 drawCol_active = ImColor(120, 220, 120, 220)*style_back;
	ImVec4 drawCol_tmp = ImColor(220, 220, 220, 220)*style_back;
	ImVec4 drawCol_header = ImColor(255, 255, 255, 255)*style_back;

	bool toolbar_gradiant = false;
	#ifdef USETOOLBARGRADIENT
	toolbar_gradiant = true;
	#endif

	#ifdef USETOOLBARCOLORS
	ImVec4 drawCol_back_gg = ImVec4(147/255.0, 196 / 255.0, 125 / 255.0, 1.0);
	ImVec4 drawCol_back_terrain = ImVec4(244 / 255.0, 163 / 255.0, 29 / 255.0, 1.0);
	ImVec4 drawCol_back_terrain_tools = ImVec4(244 / 255.0, 163 / 255.0, 29 / 255.0, 1.0);
	ImVec4 drawCol_back_entities = ImVec4(138 / 255.0, 142 / 255.0, 200 / 255.0, 1.0);
	ImVec4 drawCol_back_waypoint = ImVec4(164 / 255.0, 84 / 255.0, 40 / 255.0, 1.0);
	ImVec4 drawCol_back_test = ImVec4(245 / 255.0, 228 / 255.0, 64 / 255.0, 1.0);
	ImVec4 drawCol_back_tools = ImVec4(244 / 255.0, 163 / 255.0, 29 / 255.0, 1.0);
	#else
	#ifdef USETOOLBARHEADER
	ImVec4 drawCol_back_gg = style_winback * ImVec4(1.0, 1.0, 1.0, 0.75);
	ImVec4 drawCol_back_terrain = style_winback * ImVec4(1.0, 1.0, 1.0, 0.85);
	ImVec4 drawCol_back_terrain_tools = style_winback * ImVec4(1.0, 1.0, 1.0, 0.75);
	ImVec4 drawCol_back_entities = style_winback * ImVec4(1.0, 1.0, 1.0, 0.85);
	ImVec4 drawCol_back_waypoint = style_winback * ImVec4(1.0, 1.0, 1.0, 0.75);
	ImVec4 drawCol_back_tools = style_winback * ImVec4(1.0, 1.0, 1.0, 0.85);
	ImVec4 drawCol_back_test = style_winback * ImVec4(1.0, 1.0, 1.0, 0.75);
	#else
	int adder = 6;
	ImVec4 drawCol_back_test = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_tools = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_waypoint = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_entities = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_terrain_tools = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_terrain = ImColor(255, 255, 255, adder)*style_back; adder += 6;
	ImVec4 drawCol_back_gg = ImColor(255, 255, 255, adder)*style_back;
	ImVec4 drawCol_toogle = drawCol_back_gg;
	if (pref.current_style == 25) {
		drawCol_back_test = ImColor(255, 255, 255,0);
		drawCol_back_tools = ImColor(255, 255, 255, 0);
		drawCol_back_waypoint = ImColor(255, 255, 255, 0);
		drawCol_back_entities = ImColor(255, 255, 255, 0);
		drawCol_back_terrain_tools = ImColor(255, 255, 255, 0);
		drawCol_back_terrain = ImColor(255, 255, 255, 0);
		drawCol_back_gg = ImColor(255, 255, 255, 0);
		drawCol_toogle = ImColor(255, 255, 255, 16);
	}
	#endif
	#endif

	ImVec4 drawCol_back_active = ImColor(255, 255, 255, 160); //*style_back;
	if (pref.current_style == 25) {
		drawCol_back_active = ImColor(128, 128, 128, 128); //*style_back;
	}


	static bool bLastImGuiGotFocus;
	bLastImGuiGotFocus = bImGuiGotFocus;

	bImGuiGotFocus = false; //PE: Set this if any of the imgui windows got focus.
	bImGuiReadyToRender = false;

	// Start the Dear ImGui frame
	if (!bImGuiFrameState)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		extern bool bSpriteWinVisible;
		bSpriteWinVisible = false;
		bImGuiFrameState = true;
		bTutorialRendered = false;
		bBlockImGuiUntilNewFrame = false;
	}

	float fontSize = 0.0f;
	ImVec2 mCharAdvance = ImVec2(0, 0);
	int iOldRounding = 0;
	if (bImGuiFrameState) {
		fontSize = ImGui::CalcTextSize("#").x;
		mCharAdvance = ImVec2(fontSize, ImGui::GetTextLineHeightWithSpacing());
	}

	//PE: Additional resets, not done in direct input.
	t.inputsys.doartresize = 0;
	t.inputsys.dosave = 0; t.inputsys.doopen = 0; t.inputsys.donew = 0; t.inputsys.donewflat = 0; t.inputsys.dosaveas = 0;
	if (bImGuiFrameState) {

		// LEELEE disable interaction with main editor if Welcome system is active!
		if (iTriggerWelcomeSystemStuff != 0)
		{
			// LEELEE disable input to IMGUI menu/toolbar/panels while Welcome System is active
			//PE: Displaying it as Modal will disable everything but the welcome system.
			bRenderTargetModalMode = true;
		}
		else
		{
			bRenderTargetModalMode = false;
		}

		int icon_size = 60;
		ImVec2 iToolbarIconSize = { (float)icon_size, (float)icon_size };
		static bool dockingopen = true;
		float fsy = ImGui::CalcTextSize("#").y;
		toolbar_size = icon_size + (fsy*2.0) + 2;
		ImVec2 viewPortPos = ImGui::GetMainViewport()->Pos;
		ImVec2 viewPortSize = ImGui::GetMainViewport()->Size;
		ImGuiStatusBar_Size = fsy*2.0;

		//PE: Render toolbar.

		iOldRounding = ImGui::GetStyle().WindowRounding;
		ImGui::GetStyle().WindowRounding = 0.0f;


		//#################
		//#### Toolbar ####
		//#################

		static float toolbar_offset_center = 0;
		int current_mode = 0;

		if (t.grideditselect == 6) 
		{
			extern bool bWaypointDrawmode;
			if(bWaypointDrawmode)
				current_mode = TOOL_DRAWWAYPOINTS;
			else
				current_mode = TOOL_WAYPOINTS;
		}
		else if (t.grideditselect == 5) 
		{
			{
				if (t.gridentitymarkersmodeonly == 0)
					current_mode = TOOL_ENTITY;
				else
					current_mode = TOOL_MARKERS;
			}
		}
		else 
		{
			if (t.terrain.terrainpaintermode >= 1 && t.terrain.terrainpaintermode <= 5)
			{
				if (t.terrain.terrainpaintermode == 1)  current_mode = TOOL_SHAPE;
				if (t.terrain.terrainpaintermode == 2)  current_mode = TOOL_LEVELMODE;
				if (t.terrain.terrainpaintermode == 3)  current_mode = TOOL_STOREDLEVEL;
				if (t.terrain.terrainpaintermode == 4)  current_mode = TOOL_BLENDMODE;
				if (t.terrain.terrainpaintermode == 5)  current_mode = TOOL_RAMPMODE;
			}
			else
			{
				if (t.terrain.terrainpaintermode == 6)  current_mode = TOOL_PAINTTEXTURE;
				if (t.terrain.terrainpaintermode == 7)  current_mode = TOOL_PAINTTEXTURE;
				if (t.terrain.terrainpaintermode == 8)  current_mode = TOOL_PAINTTEXTURE;
				if (t.terrain.terrainpaintermode == 9)  current_mode = TOOL_PAINTTEXTURE;
				if (t.terrain.terrainpaintermode == 10) current_mode = TOOL_PAINTGRASS;
			}
		}

		//	current_mode = TOOL_CCP;
		if(bBuilder_Properties_Window || t.ebe.on == 1)
			current_mode = TOOL_BUILDER;
		if (bImporter_Window && t.importer.importerActive == 1)
			current_mode = TOOL_IMPORT;

		if ( t.gridentity > 0 && t.entityprofile[t.gridentity].isebe != 0) {
			current_mode = TOOL_BUILDER;
		}
		//PE: Now toggle.
		//if(Visuals_Tools_Window)
		//	current_mode = TOOL_VISUALS;

		//PE: Make sure we dont place the toolbar in its own viewport.
		ImGui::SetNextWindowPos(ImVec2(0, 0) + viewPortPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, toolbar_size));
		//ImGuiWindowFlags_NoBackground

		if (pref.current_style == 25)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.26f, 0.35f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.11f, 0.16f, 0.22f, 1.00f)); //org ImVec4(0.58f, 0.58f, 0.58f, 1.00f); // ImGui::PopStyleColor();
		}

		ImGui::Begin("Toolbar", NULL , ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);


		ImVec4 drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		float fDiv = 1.0f / 255.0f;
		//ImVec4 drawCol_Divider_Selected = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
		//ImVec4 drawCol_Divider_Selected = ImVec4(fDiv * 142.0f, fDiv * 184.0f, fDiv * 212.0f, 1.0f);
		ImVec4 drawCol_Divider_Selected = ImVec4(fDiv * 177.0f, fDiv * 206.0f, fDiv * 225.0f, 1.0f);
		if (pref.current_style == 25) {
			drawCol_hover = ImVec4(fDiv * 142.0f, fDiv * 184.0f, fDiv * 212.0f, 1.0f);
		}

		ImGui::GetStyle().WindowRounding = iOldRounding;

		#ifdef USETOOLBARHEADER
		ID3D11ShaderResourceView* lpTexture = GetImagePointerView(TOOL_HEADER);
		if (lpTexture) {
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			window->DrawList->AddImage((ImTextureID)lpTexture, viewPortPos, viewPortPos + ImVec2(1920, 200), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(drawCol_header));
		}
		#endif

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		#ifdef CENTERETOOLBAR		
		if(toolbar_offset_center > 0 && toolbar_offset_center < ImGui::GetWindowSize().x)
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + ( (ImGui::GetWindowSize().x * 0.5) - (toolbar_offset_center*0.5) ) , ImGui::GetCursorPos().y));
		#endif

		float cursorpos = ImGui::GetCursorPos().x;

		#ifdef ADDGGTOOLBAR


		//PE: Load icon removed for now.
		if (ImGui::ImgBtn(TOOL_LOADLEVEL, iToolbarIconSize, drawCol_back_gg, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant)) {
			CloseAllOpenToolsThatNeedSave();

			iLaunchAfterSync = 2; //Load
			iSkibFramesBeforeLaunch = 2;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Open Level");

		ImGui::SameLine();
		if (ImGui::ImgBtn(TOOL_SAVELEVEL, iToolbarIconSize, drawCol_back_gg, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors)) {

			CloseAllOpenToolsThatNeedSave();
			if (bTutorialCheckAction) TutorialNextAction();

			iLaunchAfterSync = 3; //Save
			iSkibFramesBeforeLaunch = 2;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Save Level");
		ImGui::SameLine();

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));
		#endif

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImVec4 tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram];
		if(pref.current_style == 3 )
			tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_Button];

		ImVec2 tool_selected_padding = { 1.0, 1.0 };

		CheckTutorialAction("TOOL_SHAPE", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_SHAPE) drawCol_tmp = drawCol_back_terrain*drawCol_back_active; else drawCol_tmp = drawCol_back_terrain;
		if (current_mode == TOOL_SHAPE && pref.current_style >= 0) window->DrawList->AddRect( (window->DC.CursorPos - tool_selected_padding) , window->DC.CursorPos + tool_selected_padding + iToolbarIconSize , ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_SHAPE, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "1";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Shape Mode");
		ImGui::SameLine();

		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		CheckTutorialAction("TOOL_LEVELMODE", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_LEVELMODE) drawCol_tmp = drawCol_back_terrain*drawCol_back_active; else drawCol_tmp = drawCol_back_terrain;
		if (current_mode == TOOL_LEVELMODE && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_LEVELMODE, iToolbarIconSize, drawCol_tmp, drawCol_normal, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "2";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Level Mode");
		ImGui::SameLine();

		CheckTutorialAction("TOOL_STOREDLEVEL", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_STOREDLEVEL) drawCol_tmp = drawCol_back_terrain * drawCol_back_active; else drawCol_tmp = drawCol_back_terrain;
		if (current_mode == TOOL_STOREDLEVEL && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_STOREDLEVEL, iToolbarIconSize, drawCol_tmp, drawCol_normal, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "3";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Stored Level Mode");
		ImGui::SameLine();

		CheckTutorialAction("TOOL_BLENDMODE", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_BLENDMODE) drawCol_tmp = drawCol_back_terrain * drawCol_back_active; else drawCol_tmp = drawCol_back_terrain;
		if (current_mode == TOOL_BLENDMODE && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_BLENDMODE, iToolbarIconSize, drawCol_tmp, drawCol_normal, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "4";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Blend Mode");
		ImGui::SameLine();

		CheckTutorialAction("TOOL_RAMPMODE", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_RAMPMODE) drawCol_tmp = drawCol_back_terrain * drawCol_back_active; else drawCol_tmp = drawCol_back_terrain;
		if (current_mode == TOOL_RAMPMODE && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_RAMPMODE, iToolbarIconSize, drawCol_tmp, drawCol_normal, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "5";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Ramp Mode");
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));
		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		CheckTutorialAction("TOOL_PAINTTEXTURE", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_PAINTTEXTURE) drawCol_tmp = drawCol_back_terrain_tools * drawCol_back_active; else drawCol_tmp = drawCol_back_terrain_tools;
		if (current_mode == TOOL_PAINTTEXTURE && pref.current_style == 25) drawCol_Selection = drawCol_Divider_Selected;
		if (current_mode == TOOL_PAINTTEXTURE && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_PAINTTEXTURE, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "6";
			bTerrain_Tools_Window = true;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Paint Texture");
		ImGui::SameLine();
		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		CheckTutorialAction("TOOL_PAINTGRASS", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_PAINTGRASS) drawCol_tmp = drawCol_back_terrain_tools * drawCol_back_active; else drawCol_tmp = drawCol_back_terrain_tools;
		if (current_mode == TOOL_PAINTGRASS && pref.current_style == 25) drawCol_Selection = drawCol_Divider_Selected;
		if (current_mode == TOOL_PAINTGRASS && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_PAINTGRASS, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors))
		{
			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "t";
			bForceKey2 = true;
			csForceKey2 = "0";
			bTerrain_Tools_Window = true;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Paint Grass");

		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));
		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		CheckTutorialAction("TOOL_ENTITY", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_ENTITY || current_mode == TOOL_SHOOTER) drawCol_tmp = drawCol_back_entities * drawCol_back_active; else drawCol_tmp = drawCol_back_entities;
		if ((current_mode == TOOL_ENTITY || current_mode == TOOL_SHOOTER ) && pref.current_style == 25) drawCol_Selection = drawCol_Divider_Selected;
		if ((current_mode == TOOL_ENTITY || current_mode == TOOL_SHOOTER ) && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_ENTITY, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "e";
			Entity_Tools_Window = true;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Object Tools (E)"); //Entity Mode

		ImGui::SameLine();
		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y ));
		CheckTutorialAction("TOOL_MARKERS", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_MARKERS) drawCol_tmp = drawCol_back_entities * drawCol_back_active; else drawCol_tmp = drawCol_back_entities;
		if (current_mode == TOOL_MARKERS && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_MARKERS, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bForceKey = true;
			csForceKey = "m";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Markers Mode (M)");
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));

		CheckTutorialAction("TOOL_WAYPOINTS", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_WAYPOINTS) drawCol_tmp = drawCol_back_waypoint * drawCol_back_active; else drawCol_tmp = drawCol_back_waypoint;
		if (current_mode == TOOL_WAYPOINTS && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_WAYPOINTS, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bWaypoint_Window = true;
			bForceKey = true;
			csForceKey = "p";
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Waypoint Editing Mode (P)");
		ImGui::SameLine();

		CheckTutorialAction("TOOL_DRAWWAYPOINTS", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_DRAWWAYPOINTS) drawCol_tmp = drawCol_back_waypoint * drawCol_back_active; else drawCol_tmp = drawCol_back_waypoint;
		if (current_mode == TOOL_DRAWWAYPOINTS && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_DRAWWAYPOINTS, iToolbarIconSize, drawCol_back_waypoint, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bWaypoint_Window = true;
			bForceKey = true;
			csForceKey = "p";

			extern int iDrawPoints;
			extern int iWaypointDeleteMode;

			bWaypointDrawmode = true;
			iWaypointDeleteMode = 1;
			iDrawPoints = 0;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Draw Waypoint Path");
		ImGui::SameLine();

		/*
		CheckTutorialAction("TOOL_NEWWAYPOINTS", -10.0f); //Tutorial: check if we are waiting for this action
		if (ImGui::ImgBtn(TOOL_NEWWAYPOINTS, iToolbarIconSize, drawCol_back_waypoint, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			bWaypoint_Window = true;
			bForceKey = true;
			csForceKey = "p";
			t.inputsys.domodewaypointcreate = 1;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Create New Waypoint");
		ImGui::SameLine();
		*/
		#ifdef VRTECH
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));
		CheckTutorialAction("TOOL_CCP", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_CCP) drawCol_tmp = drawCol_back_tools * drawCol_back_active; else drawCol_tmp = drawCol_back_tools;
		if (current_mode == TOOL_CCP && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_CCP, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant))
		{
			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			//Make sure any selection are removed
			t.gridentity = 0;
			t.inputsys.constructselection = 0;
			t.inputsys.domodeentity = 1;
			t.grideditselect = 5;
			editor_refresheditmarkers();

			#ifndef VRTECH
			if (t.characterkit.loaded == 0)  t.characterkit.loaded = 1;
			#else
			RedockNextWindow = "Character Creator##PropertiesWindow";
			g_bCharacterCreatorPlusActivated = true; //cyb
			ImGui::SetWindowFocus(TABENTITYNAME);
			#endif

		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Character Creator");
		ImGui::SameLine();
		#endif

		CheckTutorialAction("TOOL_BUILDER", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_mode == TOOL_BUILDER) drawCol_tmp = drawCol_back_tools * drawCol_back_active; else drawCol_tmp = drawCol_back_tools;
		if (current_mode == TOOL_BUILDER && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_BUILDER, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant)) {

			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			DeleteWaypointsAddedToCurrentCursor();
			//CheckTooltipObjectDelete();
			CloseDownEditorProperties();
			t.inputsys.constructselection = 0;
			if (t.ebebank_s[1].Len() > 0) 
			{
				t.addentityfile_s = t.ebebank_s[1].Get();
				if (t.addentityfile_s != "")
				{
					entity_adduniqueentity(false);
					t.tasset = t.entid;
					if (t.talreadyloaded == 0)
					{
						editor_filllibrary();
					}
				}
				t.inputsys.constructselection = t.tasset;
				t.gridentity = t.entid;
				t.inputsys.constructselection = t.entid;
				t.inputsys.domodeentity = 1;
				t.grideditselect = 5;
				editor_refresheditmarkers();

				//NewSite, make sure we are in entity mode.
				bForceKey = true;
				csForceKey = "e";
				bBuilder_Left_Window = true;
				#ifdef USELEFTPANELSTRUCTUREEDITOR
				ImGui::SetWindowFocus("Structure Editor##LeftPanel");
				#endif
			}
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Structure Editor");
		ImGui::SameLine();

		CheckTutorialAction("TOOL_IMPORT", -10.0f); //Tutorial: check if we are waiting for this action
		drawCol_Selection = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		if (current_mode == TOOL_IMPORT) drawCol_tmp = drawCol_back_tools * drawCol_back_active; else drawCol_tmp = drawCol_back_tools;
		if (current_mode == TOOL_IMPORT && pref.current_style == 25) drawCol_Selection = drawCol_Divider_Selected;
		if (current_mode == TOOL_IMPORT && pref.current_style >= 0) window->DrawList->AddRect((window->DC.CursorPos - tool_selected_padding), window->DC.CursorPos + tool_selected_padding + iToolbarIconSize, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
		if (ImGui::ImgBtn(TOOL_IMPORT, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down, 0, 0, 0, 0, false, toolbar_gradiant))
		{
			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			//Make sure any selection are removed
			t.gridentity = 0;
			t.inputsys.constructselection = 0;
			t.inputsys.domodeentity = 1;
			t.grideditselect = 5;
			editor_refresheditmarkers();

			iLaunchAfterSync = 8; //Import model
			iSkibFramesBeforeLaunch = 5;
			ImGui::SetWindowFocus(TABENTITYNAME);
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Importer");
		ImGui::SameLine();

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 2.0f, ImGui::GetCursorPos().y));
		float precise_icon_width = ImGui::GetCursorPos().x;

		CheckTutorialAction("TOOL_TESTGAME", -10.0f); //Tutorial: check if we are waiting for this action
		//if (ImGui::ImgBtn(TOOL_TESTGAME, iToolbarIconSize, drawCol_back_test, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {
		if (ImGui::ImgBtn(TOOL_TESTGAME, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors))
		{
			CloseAllOpenTools(false);
			if (bTutorialCheckAction) TutorialNextAction();
			iLaunchAfterSync = 1;
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Test Level");
		ImGui::SameLine();

		precise_icon_width = ImGui::GetCursorPos().x - precise_icon_width;

		#ifdef VRTECH
		CheckTutorialAction("TOOL_VRMODE", -10.0f); //Tutorial: check if we are waiting for this action
		//if (ImGui::ImgBtn(TOOL_VRMODE, iToolbarIconSize, drawCol_back_test, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant)) {
		if (ImGui::ImgBtn(TOOL_VRMODE, iToolbarIconSize, drawCol_tmp, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant,false,false,false, bBoostIconColors))
		{
			CloseAllOpenTools();
			if (bTutorialCheckAction) TutorialNextAction();

			iLaunchAfterSync = 20; //Test game VR.
		}
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0)
		{
			ImGui::SetTooltip("%s", "Test Level in VR");
		}
		ImGui::SameLine();
		#endif

		if (ImGui::ImgBtn(TOOL_SOCIALVR, iToolbarIconSize, drawCol_back_test, drawCol_normal*drawCol_Selection, drawCol_hover, drawCol_Down,0, 0, 0, 0, false, toolbar_gradiant))
		{
			if (bWaypointDrawmode) { bWaypointDrawmode = false; }
			if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
			if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
			if (bEntity_Properties_Window) bEntity_Properties_Window = false;
			if (t.ebe.on == 1) ebe_hide();
			iLaunchAfterSync = 21; //Social VR
		}
		#ifdef PRODUCTV3
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Social VR Mode");
		#else
		if (ImGui::IsItemHovered() && iSkibFramesBeforeLaunch == 0) ImGui::SetTooltip("%s", "Multiplayer Mode");
		#endif
		ImGui::SameLine();

		toolbar_offset_center = ImGui::GetCursorPos().x - cursorpos;


		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		if (pref.current_style == 25) {
			ImGui::PopStyleColor(2);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.43f, 0.57f, 1.00f)); //org ImVec4(0.58f, 0.58f, 0.58f, 1.00f); // ImGui::PopStyleColor();
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File") )
			{
				if (ImGui::MenuItem("New Random Level")) 
				{
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					#ifdef ALPHAEXPIRESYSTEM
					MessageBoxA ( NULL, "PERLIN Random Terrain disabled for this Build", "Build Message", MB_OK );
					#else
					iLaunchAfterSync = 6;
					iSkibFramesBeforeLaunch = 5;
					#endif
				}
				if (ImGui::MenuItem("New Flat Level")) 
				{
					CloseAllOpenTools();
					iLaunchAfterSync = 5;
					iSkibFramesBeforeLaunch = 5;
				}

				if (ImGui::MenuItem("Open", "CTRL+O") )
				{
					CloseAllOpenToolsThatNeedSave();
					//#ifdef ALPHAEXPIRESYSTEM
					//MessageBoxA ( NULL, "Level loading has been disabled for this Build", "Build Message", MB_OK );
					//#else
					iLaunchAfterSync = 2;
					iSkibFramesBeforeLaunch = 5;
					//#endif
				}
				if (ImGui::MenuItem("Save", "CTRL+Q") )
				{
					CloseAllOpenToolsThatNeedSave();
					//#ifdef ALPHAEXPIRESYSTEM
					//MessageBoxA ( NULL, "Level saving has been disabled for this Build", "Build Message", MB_OK );
					//#else
					iLaunchAfterSync = 3; //Save
					iSkibFramesBeforeLaunch = 5;
					//#endif
				}
				if (ImGui::MenuItem("Save As...", ""))//CTRL+R" ) )//F12") )
				{
					CloseAllOpenToolsThatNeedSave();
					//#ifdef ALPHAEXPIRESYSTEM
					//MessageBoxA ( NULL, "Level saving has been disabled for this Build", "Build Message", MB_OK );
					//#else
					iLaunchAfterSync = 4; //Save As
					iSkibFramesBeforeLaunch = 5;
					//#endif
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Save Standalone")) 
				{
					// Save Standalone
					if (bWaypointDrawmode) { bWaypointDrawmode = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					int iRet;
					iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						bExport_Standalone_Window = true;
					}
				}
				#ifdef VRTECH
				if (ImGui::MenuItem("Save to Level Cloud"))
				{
					#ifdef ALPHAEXPIRESYSTEM
					MessageBoxA ( NULL, "Save to Level Cloud has been disabled for this Build", "Build Message", MB_OK );
					#else
					if (bWaypointDrawmode) { bWaypointDrawmode = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					// Save Standalone
					int iRet;
					iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						bExport_SaveToGameCloud_Window = true;
					}
					#endif
				}
				#else
				
				if (ImGui::MenuItem("Download Store Items"))
				{
					if (bWaypointDrawmode) { bWaypointDrawmode = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false;
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					//Download Store Items
					int iRet;
					iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						bDownloadStore_Window = true;
					}
				}
				#endif
				if (ImGui::MenuItem("Character Creator")) 
				{
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					#ifndef VRTECH
					if (t.characterkit.loaded == 0)  t.characterkit.loaded = 1;
					#else
					g_bCharacterCreatorPlusActivated = true;
					#endif
				}
				if (ImGui::MenuItem("Structure Editor")) 
				{
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					DeleteWaypointsAddedToCurrentCursor();
					CloseDownEditorProperties();
					t.inputsys.constructselection = 0;
					if (t.ebebank_s[1].Len() > 0) 
					{
						t.addentityfile_s = t.ebebank_s[1].Get();
						if (t.addentityfile_s != "")
						{
							entity_adduniqueentity(false);
							t.tasset = t.entid;
							if (t.talreadyloaded == 0)
							{
								editor_filllibrary();
							}
						}
						t.inputsys.constructselection = t.tasset;
						t.gridentity = t.entid;
						t.inputsys.constructselection = t.entid;
						t.inputsys.domodeentity = 1;
						t.grideditselect = 5;
						editor_refresheditmarkers();

						//NewSite, make sure we are in entity mode.
						bForceKey = true;
						csForceKey = "e";
						bBuilder_Left_Window = true;
						#ifdef USELEFTPANELSTRUCTUREEDITOR
						ImGui::SetWindowFocus("Structure Editor##LeftPanel");
						#endif
					}
				}
				
				if (ImGui::MenuItem("Import Model")) 
				{
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					iLaunchAfterSync = 8; //Import model
					iSkibFramesBeforeLaunch = 5;
				}

				ImGui::Separator();
				//for (int ii = 0; ii < REMEMBERLASTFILES; ii++) { //reverse
				for (int ii = REMEMBERLASTFILES-1; ii >= 0; ii--) {
					if (strlen(pref.last_open_files[ii]) > 0) {
						char tmp[260];
						strcpy(tmp, pref.last_open_files[ii]);
						int pos = strlen(tmp);
						while (pos > 0 && tmp[pos] != '\\') pos--;
							
						//std::string s_tmp = std::to_string(1+ii); //Reverse
						std::string s_tmp = std::to_string(REMEMBERLASTFILES-ii);
						s_tmp += ": ";
						s_tmp += &tmp[pos+1];

						if (ImGui::MenuItem( s_tmp.c_str() )) {
							if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
							if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
							if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
							if (bEntity_Properties_Window) bEntity_Properties_Window = false;
							if (t.ebe.on == 1) ebe_hide();

							strcpy(cDirectOpen, pref.last_open_files[ii]);
							iLaunchAfterSync = 7; //Direct open.
							iSkibFramesBeforeLaunch = 5;
						}
					}
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) 
				{
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false; 
					if (t.ebe.on == 1) ebe_hide();
					int iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						g_bCascadeQuitFlag = true;
					}
				}

				ImGui::EndMenu();
			}
			else
			{
				if(pref.bAutoOpenMenuItems)
					if (ImGui::IsItemHovered()) 
						ImGui::OpenPopup("File");
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo","CTRL+Z")) 
				{
					t.inputsys.doundo = 1;
					bForceUndo = true;
				}
				if (ImGui::MenuItem("Redo", "CTRL+Y")) 
				{
					t.inputsys.doredo = 1;
					bForceRedo = true;
				}

				ImGui::Separator();


				if (ImGui::BeginMenu("Window Views")) 
				{
					#ifndef DISABLETUTORIALS
					if (bHelp_Window) 
					{
						if (ImGui::MenuItem("Hide Tutorial")) {
							bHelpVideo_Window = false;
							bHelp_Window = false;
						}
					}
					else 
					{
						if (ImGui::MenuItem("Show Tutorial")) {
							bHelpVideo_Window = true;
							bHelp_Window = true;
							bSetTutorialSectionLeft = false;

						}
					}
					#endif
					#ifdef USELEFTPANELSTRUCTUREEDITOR
					if (bBuilder_Left_Window) {
						if (ImGui::MenuItem("Hide Structure Editor")) {
							bBuilder_Left_Window = false;
						}
					}
					else {
						if (ImGui::MenuItem("Show Structure Editor")) {
							bBuilder_Left_Window = true;
						}
					}
					#endif
//#ifndef PRODUCTCLASSICIMGUI //PE: welcome now work.
					if (iTriggerWelcomeSystemStuff == 0) 
					{
						if (ImGui::MenuItem("Show Welcome Screen")) 
						{
							editor_showquickstart(1);
							iTriggerWelcomeSystemStuff = 99;
						}
					}
					else 
					{
						if (ImGui::MenuItem("Hide Welcome Screen")) 
						{
							welcome_free();
							iTriggerWelcomeSystemStuff = 7;
						}
					}
//#endif
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Change Color Scheme")) 
				{
					#ifdef ENABLEIMGUI
					if (ImGui::MenuItem("Light Style")) { // VRQ/Classic = Light Style
						myLightStyle(NULL);
						pref.current_style = 3;
					}
					if (ImGui::MenuItem("Dark Style")) {
						myStyle2(NULL);
						pref.current_style = 0;
					}
					#endif
					if (ImGui::MenuItem("Darker Style")) {
						myDarkStyle(NULL);
						pref.current_style = 1;
					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Reset Layout")) 
				{
					//Full Desktop Size
					if (ImGui::MenuItem("Full Desktop Size")) 
					{
						refresh_gui_docking = 0;
						pref.vStartResolution = { 1280,800 };
						pref.iMaximized = 1;
						SetWindowSize(pref.vStartResolution.x, pref.vStartResolution.y);
						float centerx = (GetDesktopWidth()*0.5) - (pref.vStartResolution.x*0.5);
						float centery = ((float)(GetDesktopHeight()*0.5) - (float)(pref.vStartResolution.y*0.5)) * 0.5f;
						if (centerx < 0) centerx = 0;
						if (centery < 0) centery = 0;
						SetWindowPosition(centerx, centery);
						MaximiseWindow();
					}

					if (ImGui::MenuItem("Current Window Size")) {
						refresh_gui_docking = 0;
					}
					//PE: SetWindowSize is ruin the winodw
					if (ImGui::MenuItem("Size 1024x768")) {
						refresh_gui_docking = 0;
						pref.vStartResolution = { 1024,768 };
						pref.iMaximized = 0;
						SetWindowSize(pref.vStartResolution.x, pref.vStartResolution.y);
						float centerx = (GetDesktopWidth()*0.5) - (pref.vStartResolution.x*0.5);
						float centery = ((float)(GetDesktopHeight()*0.5) - (float)(pref.vStartResolution.y*0.5)) * 0.5f;
						if (centerx < 0) centerx = 0;
						if (centery < 0) centery = 0;
						SetWindowPosition(centerx, centery);
						RestoreWindow();
					}
					if (ImGui::MenuItem("Size 1280x800")) {
						refresh_gui_docking = 0;
						pref.vStartResolution = { 1280,800 };
						pref.iMaximized = 0;
						SetWindowSize(pref.vStartResolution.x, pref.vStartResolution.y);
						float centerx = (GetDesktopWidth()*0.5) - (pref.vStartResolution.x*0.5);
						float centery = ((float)(GetDesktopHeight()*0.5) - (float)(pref.vStartResolution.y*0.5)) * 0.5f;
						if (centerx < 0) centerx = 0;
						if (centery < 0) centery = 0;
						SetWindowPosition(centerx, centery);
						RestoreWindow();
					}
					ImGui::EndMenu();
				}


				ImGui::EndMenu();
			}
			else
			{
				if (pref.bAutoOpenMenuItems)
					if (ImGui::IsItemHovered())
						ImGui::OpenPopup("Edit");
			}

			if (ImGui::BeginMenu("Terrain"))
			{
				if (ImGui::MenuItem("Shape Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "1";
				}
				if (ImGui::MenuItem("Level Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "2";
				}
				if (ImGui::MenuItem("Stored Level Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "3";
				}
				if (ImGui::MenuItem("Blend Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "4";
				}
				if (ImGui::MenuItem("Ramp Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "5";
				}
				if (ImGui::MenuItem("Paint Texture")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "6";
				}
				if (ImGui::MenuItem("Paint Grass")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "t";
					bForceKey2 = true;
					csForceKey2 = "0";
				}
				ImGui::EndMenu();
			}
			else
			{
				if (ImGui::IsItemHovered()) ImGui::OpenPopup("Terrain");
			}

			if (ImGui::BeginMenu("Entities"))
			{
				if (ImGui::MenuItem("Entity Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "e";
				}
				if (ImGui::MenuItem("Marker Mode")) {
					if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					bForceKey = true;
					csForceKey = "m";
				}
				//PE: if we change text "waypoint" it should be done everywhere, like lua/help ... until then:
				if (ImGui::MenuItem("Waypoint Mode")) { //Follow text used: was Path Mode
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					bWaypoint_Window = true;
					bForceKey = true;
					csForceKey = "p";
				}
				if (ImGui::MenuItem("Draw Waypoint Path")) { //Follow text used, was: Draw New Path
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					bWaypoint_Window = true;
					bForceKey = true;
					csForceKey = "p";

					extern int iDrawPoints;
					extern int iWaypointDeleteMode;

					bWaypointDrawmode = true;
					iWaypointDeleteMode = 1;
					iDrawPoints = 0;

				}
				//Old waypoint system.
//					if (ImGui::MenuItem("Create New Path")) {
//						if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
//						if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false;
//						if (bEntity_Properties_Window) bEntity_Properties_Window = false;
//						if (t.ebe.on == 1) ebe_hide();
//						bWaypoint_Window = true;
//						bForceKey = true;
//						csForceKey = "p";
//						t.inputsys.domodewaypointcreate = 1;
//					}

				ImGui::EndMenu();
			}
			else
			{
				if (ImGui::IsItemHovered()) ImGui::OpenPopup("Entities");
			}


			if (ImGui::BeginMenu("Test Level"))
			{
				if (ImGui::MenuItem("Test Level")) 
				{
					if (bWaypointDrawmode) { bWaypointDrawmode = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();

					iLaunchAfterSync = 1;
				}
				#ifdef VRTECH
				if (ImGui::MenuItem("Test Game in VR")) 
				{
					if (bWaypointDrawmode) { bWaypointDrawmode = false; }
					//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
					if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
					if (bEntity_Properties_Window) bEntity_Properties_Window = false;
					if (t.ebe.on == 1) ebe_hide();
					iLaunchAfterSync = 20; //Test game VR.
				}
				#endif
				#ifdef PRODUCTV3
				if (ImGui::MenuItem("Social VR")) {
				#else
				if (ImGui::MenuItem("Multiplayer Mode")) {
				#endif
				#ifdef ALPHAEXPIRESYSTEM
				MessageBoxA(NULL, "Multiplayer Mode not available in build", "Not In Build", MB_OK);
				#else
				if (bWaypointDrawmode) { bWaypointDrawmode = false; }
				//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
				if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
				if (bEntity_Properties_Window) bEntity_Properties_Window = false;
				if (t.ebe.on == 1) ebe_hide();
				iLaunchAfterSync = 21; //Social VR
				//Always switch back to entity mode after test game.
				//bForceKey = true;
				//csForceKey = "e";
				//Entity_Tools_Window = true;
				#endif
				}
				ImGui::EndMenu();
			}
			else
			{
				if (ImGui::IsItemHovered()) ImGui::OpenPopup("Test Level");
			}


			if (ImGui::BeginMenu("Help"))
			{

				image_setlegacyimageloading(true);
				if (ImGui::MenuItem("Editor Shortcuts","F1")) {
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\quick-help.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					bHelp_Menu_Image_Window = true;
				}

				#ifdef VRTECH
				if (ImGui::MenuItem("Level Shortcuts")) {
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\testgamelayout.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					bHelp_Menu_Image_Window = true;
				}
				if (ImGui::MenuItem("VR Controls")) {
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\testgamelayout-vr.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					bHelp_Menu_Image_Window = true;
				}
				#else
				if (ImGui::MenuItem("Level Shortcuts")) {
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\testgamelayout-1024x768.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					bHelp_Menu_Image_Window = true;
				}
				if (ImGui::MenuItem("Multiplayer Controls")) {
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\testgamelayoutmp-1024x768.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					bHelp_Menu_Image_Window = true;
				}
				#endif

				if (ImGui::MenuItem("Read User Manual")) 
				{
					///Files/languagebank/english/artwork/GameGuru%20-%20Getting%20Started%20Guide.pdf
					#ifdef VRTECH
					ExecuteFile("https://gameguru-max.document360.io/docs", "", "", 0);
					#else
					cstr pPDFPath = g.fpscrootdir_s + "\\Files\\languagebank\\english\\artwork\\GameGuru - Getting Started Guide.pdf";
					ExecuteFile(pPDFPath.Get(), "", "", 0);
					#endif
				}
				#ifndef DISABLETUTORIALS
				if (ImGui::MenuItem("Getting Started Tutorial"))
				{
					bHelpVideo_Window = true;
					bHelp_Window = true;
					bSetTutorialSectionLeft = false;
					strcpy(cForceTutorialName, "01 - Getting started");
				}
				#endif
				if (ImGui::MenuItem("About")) {
					bAbout_Window = true;
					bAbout_Window_First_Run = true;
				}
				image_setlegacyimageloading(false);

				ImGui::EndMenu();
			}
			else
			{
				if (pref.bAutoOpenMenuItems)
					if (ImGui::IsItemHovered()) 
						ImGui::OpenPopup("Help");
			}

			ImGui::EndMenuBar();
		}


		if (pref.current_style == 25)
			ImGui::PopStyleColor(); //ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.11f, 0.16f, 0.22f, 1.00f)); //org ImVec4(0.58f, 0.58f, 0.58f, 1.00f);

		//Process systemwide shortcut keys.

		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::GetTime() - lastKeyTime >= 0.125) 
		{ 
			//small delay between key input.
			auto ctrl = io.KeyCtrl;
			auto alt = io.ConfigMacOSXBehaviors ? io.KeyCtrl : io.KeyAlt;
			auto shift = io.KeyShift;

			int iExecuteCTRLkey = 0;
			//PE: No repeat on these keys.
			static bool bWaitOnGRelease = false;
			if (bWaitOnGRelease && ImGui::IsKeyReleased(71))
				bWaitOnGRelease = false;

			//LB: ensure CTRL+Z can release Z and repress so all does not happen at once
			static bool bWaitOnZRelease = false;
			if (bWaitOnZRelease && ImGui::IsKeyReleased(90))
			{
				t.inputsys.undokeypress = 0;
				bWaitOnZRelease = false;
			}

			if (ctrl && !shift && !alt && ImGui::IsKeyPressed(89) || iExecuteCTRLkey == 'Y' ) //Y
			{ 
				//CTRL Y - redo
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				bForceRedo = true;
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(78) || iExecuteCTRLkey == 'N') //N
			{
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				CloseAllOpenTools();
				iLaunchAfterSync = 5;
				iSkibFramesBeforeLaunch = 5;
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(73) || iExecuteCTRLkey == 'I') //I - Importer
			{
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				DeleteWaypointsAddedToCurrentCursor();
				CloseDownEditorProperties();
				CloseAllOpenTools();
				iLaunchAfterSync = 8; //Import model
				iSkibFramesBeforeLaunch = 5;
				if (bExternal_Entities_Window)
				{
					bCheckForClosingForce = true;
				}
			}
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(79) || iExecuteCTRLkey == 'O' ) //O
			{
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				CloseAllOpenTools();
				iLaunchAfterSync = 2;
				iSkibFramesBeforeLaunch = 5;
			}
			//else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(83) || iExecuteCTRLkey == 'S') //S
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(81) || iExecuteCTRLkey == 'Q') //Q
			{
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				CloseAllOpenToolsThatNeedSave();
				iLaunchAfterSync = 3; //Save
				iSkibFramesBeforeLaunch = 5;
			}
			/*
			//else if (!ctrl && !shift && !alt && ImGui::IsKeyPressed(0x7B)) // 0x70 = F1 0x7B = F12
			else if (!ctrl && !shift && !alt && ImGui::IsKeyPressed(82) || iExecuteCTRLkey == 'R') //R
			{
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				CloseAllOpenTools();
				iLaunchAfterSync = 4; //Save As
				iSkibFramesBeforeLaunch = 5;
			}
			*/
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(88) || iExecuteCTRLkey == 'X') //X , "CUT"
			{
				if (!bLastImGuiGotFocus || iExecuteCTRLkey == 'X')
				{
					lastKeyTime = (float)ImGui::GetTime();
					iExecuteCTRLkey = 0;


					t.widget.deletebuttonselected = 1;
					widget_show_widget();
				}
			}
			/* is this a thing? - only CTRL+Z documented in the UI
			else if (!ctrl && !shift && alt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace)))
			{ 
				//ALT BACKSPACE - undo
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				if (t.inputsys.undokeypress == 0)
				{
					bForceUndo = true;
					bWaitOnZRelease = true;
				}
			}
			*/
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(90) && bWaitOnZRelease==false) 
			{ 
				//CTRL Z - undo
				lastKeyTime = (float)ImGui::GetTime();
				iExecuteCTRLkey = 0;
				bForceUndo = true;
				bWaitOnZRelease = true;
			}
			#ifdef GROUPINGFEATURE
			else if (ctrl && !shift && !alt && ImGui::IsKeyPressed(71)) //CTRL+G
			{
				//CTRL G - Group.
				if (!bWaitOnGRelease)
					CreateNewGroup(-1);
				bWaitOnGRelease = true;
			}
			else if (ctrl && shift && !alt && ImGui::IsKeyPressed(71)) //CTRL+SHIFT+G
			{
				//CTRL+SHIFT G - UnGroup.
				if(!bWaitOnGRelease)
					UnGroupSelected();
				bWaitOnGRelease = true;
			}
			#endif

			static bool bReadyToProcessF1Key = true;
			if(bReadyToProcessF1Key && ImGui::IsKeyPressed(0x70)) { // 0x70 = F1
				bReadyToProcessF1Key = false;
				if (bHelp_Menu_Image_Window)
					bHelp_Menu_Image_Window = false;
				else {
					image_setlegacyimageloading(true);
					strcpy(cHelpMenuImage, "languagebank\\english\\artwork\\quick-help.png");
					LoadImage(cHelpMenuImage, HELPMENU_IMAGE);
					image_setlegacyimageloading(false);
					bHelp_Menu_Image_Window = true;
				}
			}
			else {
				bReadyToProcessF1Key = true;
			}
			

		}

		ImGui::End();

		//####################
		//#### Status bar ####
		//####################

#define ADDCONTROLSTOSTAUSBAR

		int iOldWindowBorderSize = ImGui::GetStyle().WindowBorderSize;
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().WindowBorderSize = 1.0f;

		float paddingy = ImGui::GetStyle().WindowPadding.y;
		//float startposy = viewPortSize.y - ImGuiStatusBar_Size - 2.0; //(ImGui::GetStyle().WindowBorderSize*2.0)
		float startposy = viewPortSize.y - 32 - 2.0;
		ImGui::SetNextWindowPos(viewPortPos + ImVec2(0.0f, startposy), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, ImGuiStatusBar_Size));
		//ImGuiWindowFlags_NoDocking,ImGuiWindowFlags_MenuBar

		if (pref.current_style == 25)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.26f, 0.35f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.11f, 0.16f, 0.22f, 1.00f)); //org ImVec4(0.58f, 0.58f, 0.58f, 1.00f); // ImGui::PopStyleColor();
		}

		ImGui::Begin("Statusbar", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 10.0f, ImGui::GetCursorPos().y + (fsy*0.5)));
		ImGui::Text("%s", t.laststatusbar_s.Get());
		ImGui::SameLine();
		//Align right.
		int align_checkbox = 96;
		int align_light_checkbox = 106;
		int align_combo_size = 120;
#ifndef ADDCONTROLSTOSTAUSBAR
		align_light_checkbox = 0;
		align_checkbox = 0;
#endif
#ifdef ADDCONTROLSTOSTAUSBAR

		float fTextSize = ImGui::CalcTextSize(t.statusbar_s.Get()).x * 1.05;
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - fTextSize - align_combo_size - align_checkbox - align_light_checkbox, ImGui::GetCursorPos().y ));
		ImGui::Text(t.statusbar_s.Get());

		ImGui::SameLine();
		ImVec2 vPos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - align_combo_size - align_checkbox - align_light_checkbox, ImGui::GetCursorPos().y - 5));

		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, 4.0f));


		ImGui::SameLine();
//		ImVec2 vPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(vPos);
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - align_combo_size - align_checkbox , ImGui::GetCursorPos().y-5 ));
		bool bTopDownStatus = ! (bool) t.editorfreeflight.mode;
		if (ImGui::Checkbox(" Top Down", &bTopDownStatus)) {
			bForceKey = true;
			if(!bTopDownStatus)
				csForceKey = "f";
			else
				csForceKey = "g";
		}
		ImGui::SameLine();

		ImGui::SetCursorPos(vPos);

		const char* items_align[] = { "NORMAL", "SNAP", "GRID"};
		int item_current_type_selection = 0;
		item_current_type_selection = t.gridentitygridlock;

		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - align_combo_size, ImGui::GetCursorPos().y-5.0));
		ImGui::PushItemWidth(align_combo_size-10);
		if (ImGui::Combo("##BehavioursSimpleInput", &item_current_type_selection, items_align, IM_ARRAYSIZE(items_align))) {
			t.gridentitygridlock = item_current_type_selection;
		}
		ImGui::PopItemWidth();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Alignment");
//			if (t.gridentitygridlock == 0)  t.statusbar_s = t.statusbar_s + "NORMAL";
//			if (t.gridentitygridlock == 1)  t.statusbar_s = t.statusbar_s + "SNAP";
//			if (t.gridentitygridlock == 2)  t.statusbar_s = t.statusbar_s + "GRID";

#endif

		ImGui::End();
		ImGui::GetStyle().WindowRounding = iOldRounding;
		ImGui::GetStyle().WindowBorderSize = iOldWindowBorderSize;

		if (pref.current_style == 25) {
			ImGui::PopStyleColor(2);
		}


		//######################################################################
		//#### Default dockspace setup, how is our windows split on screen. ####
		//######################################################################

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking; //ImGuiWindowFlags_MenuBar
		viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, toolbar_size));
		ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, toolbar_size + ImGuiStatusBar_Size));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpaceAGK", &dockingopen, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);


		static ImGuiID dock_id_bottom;
		//We cant make all windows dock if all windows is NOT undocked first (.ini setup problem ), so refresh_gui_docking == 2
		if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) == NULL || refresh_gui_docking == 2)
		{
			//Default docking setup.
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			//int ImGuiDockNodeFlags_Dockspace = 1 << 10;
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodePos(dockspace_id, viewport->Pos + ImVec2(0, toolbar_size));
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size - ImVec2(0, toolbar_size+ ImGuiStatusBar_Size));

			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.12f, NULL, &dock_main_id); //Toolbar


			ImGuiID dock_id_right;
			if(viewport->Size.x > 1300)
				dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.14f, NULL, &dock_main_id); //0.20f
			else if (viewport->Size.x < 1100)
				dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.23f, NULL, &dock_main_id); //0.20f
			else
				dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.21f, NULL, &dock_main_id); //0.20f


			ImGuiID dock_id_right2out;
			ImGuiID dock_id_right2 = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, &dock_id_right2out, &dock_main_id); //0.20f

			// create dock ID for above Tutorial Help Window (for Tutorial Video area)
			ImGuiID dock_id_right2below = ImGui::DockBuilderSplitNode(dock_id_right2out, ImGuiDir_Down, 0.26f, NULL, NULL); //0.18

			// leelee, technically not allowed by IMGUI but I fudged the IMGUI code to allow it - seems to work fine!
			ImGuiID dock_id_right3below = ImGui::DockBuilderSplitNode(dock_id_right2out, ImGuiDir_Down, 0.60f, NULL, NULL); //(0.65) PE: Video area must have room for a normal mp4.


			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, NULL, &dock_main_id); //0.15f
			ImGuiID dock_id_left_down = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.15f, NULL, &dock_id_left); //0.15f
			ImGuiID dock_id_left_down_large = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.30f, NULL, &dock_id_left); //0.15f

			ImGuiID dock_id_left2out;
			ImGuiID dock_id_left2 = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.30f, &dock_id_left2out, &dock_main_id); //0.20f
			ImGuiID dock_id_left2below = ImGui::DockBuilderSplitNode(dock_id_left2out, ImGuiDir_Down, 0.26f, NULL, NULL); //0.18
			ImGuiID dock_id_left3below = ImGui::DockBuilderSplitNode(dock_id_left2out, ImGuiDir_Down, 0.60f, NULL, NULL); //(0.65) PE: Video area must have room for a normal mp4.

			// Disable tab bar for custom toolbar and statusbar
			ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_top);
			node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

			ImGui::DockBuilderDockWindow(TABEDITORNAME, dock_main_id);
			ImGui::DockBuilderDockWindow(TABENTITYNAME, dock_id_left);

#ifdef USELEFTPANELSTRUCTUREEDITOR
			ImGui::DockBuilderDockWindow("Structure Editor##LeftPanel", dock_id_left);
#endif
			ImGui::DockBuilderDockWindow("Tutorial Video##HelpVideoWindow", dock_id_right2below);
			ImGui::DockBuilderDockWindow("Tutorial Steps##HelpWindow", dock_id_right3below);

			ImGui::DockBuilderDockWindow("Tutorial Video##LeftHelpVideoWindow", dock_id_left2below);
			ImGui::DockBuilderDockWindow("Tutorial Steps##LeftHelpWindow", dock_id_left3below);

			ImGui::DockBuilderDockWindow("Entity Properties##PropertiesWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Character Creator##PropertiesWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Structure Properties##BuilderPropertiesWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Importer##ImporterWindow", dock_id_right);

			ImGui::DockBuilderDockWindow("Terrain Tools##TerrainToolsWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Sculpt Terrain##TerrainToolsWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Paint Terrain##TerrainToolsWindow", dock_id_right);
			ImGui::DockBuilderDockWindow("Add Vegetation##TerrainToolsWindow", dock_id_right);

			ImGui::DockBuilderDockWindow("Waypoints##WaypointsToolsWindow", dock_id_right);

#ifdef USE_ENTITY_TOOL_WINDOW
			ImGui::DockBuilderDockWindow("Object Tools##EntityToolsWindow", dock_id_right);
#endif
			//ImGuiDockNodeFlags_AutoHideTabBar
			dock_main_tabs = dock_main_id;
			dock_tools_windows = dock_id_right;
			ImGui::DockBuilderFinish(dockspace_id);
		}

		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 vOldWindowMinSize = style.WindowMinSize;
		style.WindowMinSize.x = 150.0f;

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		style.WindowMinSize = vOldWindowMinSize;

		ImGui::End();


		if (dock_main_tabs == 0)
			dock_main_tabs = dockspace_id;

		//#######################
		//#### Tutorial Help ####
		//#######################
		if (bHelp_Window && bHelpVideo_Window == false) bHelp_Window = false;
		//PE: Always read in all tutorials.
		if (!bTutorial_Init) 
		{
			//Reset everything.
			tut.bActive = false;
			tut.iSteps = 0;
			strcpy(tut.cStartText, "");
			strcpy(tut.cVideoPath, "");
			tut.bVideoReady = false;
			tut.bVideoInit = false;
			bVideoResumePossible = false;
			for (int il = 0; il < TUTORIALMAXSTEPS; il++) { //Reset
				strcpy(tut.cStepHeader[il], "");
				strcpy(tut.cStepText[il], "");
				strcpy(tut.cStepAction[il], "");
				tut.vOffsetPointer[il] = ImVec2(0, 0);
			}

			get_tutorials();

			//use first entry.
			std::map<std::string, std::string>::iterator it = tutorial_files.begin();
				
			if (it->first.length() > 0) {
				strcpy(cTutorialName, it->first.c_str());
			}

			bTutorial_Init = true;
		}
			
		if (bHelp_Window && current_tutorial != selected_tutorial) 
		{
			current_tutorial = selected_tutorial;

			//Reset everything.
			tut.bActive = false;
			tut.iCurrent_Step = 0;
			tut.iSteps = 0;
			strcpy(tut.cStartText, "");
			strcpy(tut.cVideoPath, "");
			tut.bVideoReady = false;
			tut.bVideoInit = false;
			bVideoResumePossible = false;

			for (int il = 0; il < TUTORIALMAXSTEPS; il++) { //Reset
				strcpy(tut.cStepHeader[il], "");
				strcpy(tut.cStepText[il], "");
				strcpy(tut.cStepAction[il], "");
				tut.vOffsetPointer[il] = ImVec2(0, 0);
			}

			//Read in selected tutorial.
			int count_tut = 0;

			//Find filename to use:
			cstr tut_filename = "";// editors\\uiv3\\tutorial.txt";
			if (tutorial_files.size() > 0) 
			{
				for (std::map<std::string, std::string>::iterator it = tutorial_files.begin(); it != tutorial_files.end(); ++it) 
				{
					if (it->first.length() > 0) 
					{
						if (count_tut++ >= selected_tutorial) 
						{
							tut_filename = it->second.c_str();
							break;
						}
					}
				}
			}

			//Reset active tutorial.
			FILE* fTut = GG_fopen(tut_filename.Get(), "r");
			if (fTut)
			{
				char ctmp[TUTORIALMAXTEXT];
				bool bStart = false;
				while (!feof(fTut))
				{
					fgets(ctmp, TUTORIALMAXTEXT-1, fTut);
					if (strlen(ctmp) > 0 && ctmp[strlen(ctmp) - 1] == '\n')
						ctmp[strlen(ctmp) - 1] = 0;

					if (strncmp(ctmp, "TUT:", 4) == 0)
					{
						if( bStart )
							break; // new section exit.
						//Always take first entry.
//							if (count_tut++ >= selected_tutorial)
						bStart = true;
					}
					if (bStart) {
						//Add to active tutorial.
						bool bFound = false;

						if (strncmp(ctmp, "VIDEO:", 6) == 0)
						{
							strcpy(tut.cVideoPath, &ctmp[7]);

							char resolved[MAX_PATH];
							int retval = GetFullPathNameA(tut.cVideoPath, MAX_PATH, resolved, NULL);
							if (retval > 0) {
								strcpy(tut.cVideoPath, resolved);
							}
							bFound = true;
						}
						if (strncmp(ctmp, "START:", 6) == 0)
						{
							strcpy(tut.cStartText, &ctmp[7]);
							bFound = true;
						}

						if (!bFound) {
							for (int il = 1; il < TUTORIALMAXSTEPS; il++) {

								cstr cmp = "STEP"; cmp = cmp + Str(il); cmp = cmp + "-HEADER:";
								if (strncmp(ctmp, cmp.Get(), cmp.Len()) == 0) {
									strcpy(tut.cStepHeader[il - 1], &ctmp[cmp.Len() + 1]);
									bFound = true;
								}
								cmp = "STEP"; cmp = cmp + Str(il); cmp = cmp + "-TEXT:";
								if (strncmp(ctmp, cmp.Get(), cmp.Len()) == 0) {
									strcpy(tut.cStepText[il - 1], &ctmp[cmp.Len() + 1]);
									bFound = true;
								}
								cmp = "STEP"; cmp = cmp + Str(il); cmp = cmp + "-ACTION:";
								if (strncmp(ctmp, cmp.Get(), cmp.Len()) == 0) {
									strcpy(tut.cStepAction[il - 1], &ctmp[cmp.Len() + 1]);
									bFound = true;
								}
								cmp = "STEP"; cmp = cmp + Str(il); cmp = cmp + "-OFFSETX:";
								if (strncmp(ctmp, cmp.Get(), cmp.Len()) == 0) {
									tut.vOffsetPointer[il - 1].x = atof(&ctmp[cmp.Len() + 1]);
									bFound = true;
								}
								cmp = "STEP"; cmp = cmp + Str(il); cmp = cmp + "-OFFSETY:";
								if (strncmp(ctmp, cmp.Get(), cmp.Len()) == 0) {
									tut.vOffsetPointer[il - 1].y = atof(&ctmp[cmp.Len() + 1]);
									bFound = true;
								}
								if (bFound)
								{
									if (tut.iSteps < il)
										tut.iSteps = il;
									break;
								}
							}
						}
					}
				}
				fclose(fTut);
				if (tut.iSteps > 0) 
				{
					//Add Tutorial Complete
					strcpy(tut.cStepHeader[tut.iSteps], "Final Tutorial Step");
					strcpy(tut.cStepText[tut.iSteps], "");
					strcpy(tut.cStepAction[tut.iSteps], "-=DONE=-");
					tut.iSteps++;
				}
			}
		}

		if (refresh_gui_docking == 0)
		{
			//Make sure window is setup in docking space.
			ImGui::Begin("Tutorial Video##HelpVideoWindow", &bHelpVideo_Window, iGenralWindowsFlags);
			ImGui::End();
			ImGui::Begin("Tutorial Steps##HelpWindow", &bHelp_Window, iGenralWindowsFlags);
			ImGui::End();
			ImGui::Begin("Tutorial Video##LeftHelpVideoWindow", &bHelpVideo_Window, iGenralWindowsFlags);
			ImGui::End();
			ImGui::Begin("Tutorial Steps##LeftHelpWindow", &bHelp_Window, iGenralWindowsFlags);
			ImGui::End();
		}
		else if (bHelp_Window && tutorial_files.size() > 0)
		{
			char cTutWindowVideoName[256];
			char cTutWindowStepsName[256];

			if (bSetTutorialSectionLeft)
			{
				strcpy(cTutWindowVideoName, "Tutorial Video##LeftHelpVideoWindow");
				strcpy(cTutWindowStepsName, "Tutorial Steps##LeftHelpWindow");
			}
			else
			{
				strcpy(cTutWindowVideoName, "Tutorial Video##HelpVideoWindow");
				strcpy(cTutWindowStepsName, "Tutorial Steps##HelpWindow");
			}

			if( iVideoFindFirstFrame > 0) {
				if (iVideoFindFirstFrame == 1) {
					PauseAnim(tut.bVideoID);
					bVideoResumePossible = false;
				}
				iVideoFindFirstFrame--;
			}

			switch (iVideoDelayExecute) {
				case 1: //Play restart
				{
					iVideoDelayExecute = 0;
					StopAnimation(tut.bVideoID);
					PlayAnimation(tut.bVideoID);
					SetRenderAnimToImage(tut.bVideoID, true);
					UpdateAllAnimation();
					Sleep(50); //Sleep so we get a video texture in the next call.
					UpdateAllAnimation();
					SetVideoVolume(100.0);
					bVideoResumePossible = false;
					break;
				}
				case 2: //Resume
				{
					iVideoDelayExecute = 0;
					ResumeAnim(tut.bVideoID);
					break;
				}
				case 3: //Pause
				{
					iVideoDelayExecute = 0;
					PauseAnim(tut.bVideoID);
					bVideoResumePossible = true;
					break;
				}
				default:
					break;
			}

			static bool bVideoPlayerMaximized = false;

			if (bVideoPlayerMaximized) 
			{
				ImGui::SetNextWindowSize(ImVec2(48 * ImGui::GetFontSize(), 46 * ImGui::GetFontSize()), ImGuiCond_Once);
				ImGui::SetNextWindowPosCenter(ImGuiCond_Once);
				ImGui::Begin("Tutorial Video##Videos2MaxSize", &bVideoPlayerMaximized, 0);
			}
			else 
			{
				ImGui::Begin(cTutWindowVideoName, &bHelpVideo_Window, iGenralWindowsFlags);
			}
			ImGui::Indent(10);
			ImGui::PushItemWidth(-10);

			if (ImGui::BeginCombo("##SelectYourTutorial", cTutorialName) ) // The second parameter is the label previewed before opening the combo.
			{
				int vloop = 0;
				for (std::map<std::string, std::string>::iterator it = tutorial_files.begin(); it != tutorial_files.end(); ++it)
				{
					if (it->first.length() > 0)
					{
						bool is_selected = false;
						if (strcmp(it->first.c_str(), cTutorialName) == 0)
							is_selected = true;
						if (ImGui::Selectable(it->first.c_str(), is_selected))
						{
							//Change Tutorial.
							strcpy(cTutorialName, it->first.c_str());
							selected_tutorial = vloop;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
						vloop++;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			// and a force tutorial mode
			bool bForceASelection = false;
			if (strlen(cForceTutorialName) > 0)
			{
				strcpy(cTutorialName, cForceTutorialName);
				strcpy(cForceTutorialName, "");
				int vloop = 0;
				for (std::map<std::string, std::string>::iterator it = tutorial_files.begin(); it != tutorial_files.end(); ++it)
				{
					if (it->first.length() > 0)
					{
						if (strcmp(it->first.c_str(), cTutorialName) == 0)
						{
							strcpy(cTutorialName, it->first.c_str());
							selected_tutorial = vloop;
						}
						vloop++;
					}
				}
				bForceASelection = true;
			}

			// use video panel (wait until videos GOOD and other Rick-Requests)
			ImVec4 oldImGuiCol_ChildWindowBg = ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg];
			#ifdef ENABLETUTORIALVIDEOS
			{
				if (!tut.bVideoInit)
				{
					if (tut.bVideoID > 0) {
						if (AnimationExist(tut.bVideoID)) {
							if (AnimationPlaying(tut.bVideoID))
								StopAnimation(tut.bVideoID);

							DeleteAnimation(tut.bVideoID);
							tut.bVideoID = 0;
						}
					}

					t.tvideofile_s = tut.cVideoPath;
					tut.bVideoID = 0;
					t.text_s = Lower(Right(t.tvideofile_s.Get(), 4));
					if (t.text_s == ".ogv" || t.text_s == ".mp4")
					{
						tut.bVideoID = 32;
						for (int itl = 1; itl <= 32; itl++)
						{
							if (AnimationExist(itl) == 0) { tut.bVideoID = itl; break; }
						}
						if (LoadAnimation(t.tvideofile_s.Get(), tut.bVideoID, g.videoprecacheframes, g.videodelayedload, 1) == false)
						{
							tut.bVideoID = -999;
						}
					}
					if (tut.bVideoID > 0) {
						PlaceAnimation(tut.bVideoID, -1, -1, -1, -1);
						SetRenderAnimToImage(tut.bVideoID, true);
						//Try to get first frame.
						StopAnimation(tut.bVideoID);
						PlayAnimation(tut.bVideoID);
						SetRenderAnimToImage(tut.bVideoID, true);
						iVideoFindFirstFrame = 4;
						UpdateAllAnimation();
						bVideoResumePossible = false;
						bVideoPerccentStart = false;
					}
					tut.bVideoInit = true;
				}

				float fRatio = 1.0f / ((float)GetDesktopWidth() / (float)GetDesktopHeight());

				ID3D11ShaderResourceView* lpVideoTexture = GetAnimPointerView(tut.bVideoID);
				float fVideoW = GetAnimWidth(tut.bVideoID);
				float fVideoH = GetAnimHeight(tut.bVideoID);
				if (tut.bVideoInit && tut.bVideoID > 0 && lpVideoTexture) {
					fRatio = 1.0f / (fVideoW / fVideoH);
				}

				float videoboxheight = (ImGui::GetContentRegionAvail().x - 10.0) * fRatio;

				oldImGuiCol_ChildWindowBg = ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg];
				ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
				ImGui::BeginChild("Video##TutorialVideo", ImVec2(ImGui::GetContentRegionAvail().x - 10.0, videoboxheight), true, iGenralWindowsFlags);
				window = ImGui::GetCurrentWindow();
				ImRect image_bb(window->DC.CursorPos, window->DC.CursorPos + ImGui::GetContentRegionAvail());
				if (lpVideoTexture) {
					SetRenderAnimToImage(tut.bVideoID, true);
					float animU = GetAnimU(tut.bVideoID);
					float animV = GetAnimV(tut.bVideoID);
					ImVec2 uv0 = ImVec2(0, 0);
					ImVec2 uv1 = ImVec2(animU, animV);
					window->DrawList->AddImage((ImTextureID)lpVideoTexture, image_bb.Min, image_bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
				}

				if( !(tut.bVideoID > 0 && AnimationExist(tut.bVideoID) && AnimationPlaying(tut.bVideoID) ))
				{
					//Display a play button.
					ImVec2 vOldPos = ImGui::GetCursorPos();
					float fPlayButSize = ImGui::GetContentRegionAvail().x * 0.15;
					float fCenterX = (ImGui::GetContentRegionAvail().x*0.5) - (fPlayButSize*0.5);
					float fCenterY = (videoboxheight*0.5) - (fPlayButSize*0.5);
					ImGui::SetCursorPos(ImVec2(fCenterX, fCenterY));
					ImVec4 vColorFade = { 1.0,1.0,1.0,0.5 };
					if (ImGui::ImgBtn(MEDIA_PLAY, ImVec2(fPlayButSize, fPlayButSize), ImColor(255, 255, 255, 0), drawCol_normal*vColorFade, drawCol_hover*vColorFade, drawCol_Down*vColorFade, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
					{
						bVideoPerccentStart = true;
						if (bVideoResumePossible) {
							iVideoDelayExecute = 2; //resume
						}
						else {
							iVideoDelayExecute = 1; //play - restart.
						}
					}
					if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Play");

					ImGui::SetCursorPos(vOldPos);
				}

				if (ImGui::IsMouseHoveringRect(image_bb.Min, image_bb.Max)) {
					if (ImGui::IsMouseDoubleClicked(0))
					{
						bVideoPlayerMaximized = 1 - bVideoPlayerMaximized;
					}
				}

				ImGui::EndChild();
				ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = oldImGuiCol_ChildWindowBg;

				if (tut.bVideoID > 0) {
					if (AnimationExist(tut.bVideoID)) {

						//ImGui::SameLine();
						float fdone = GetAnimPercentDone(tut.bVideoID) / 100.0f;
						if (!bVideoPerccentStart) fdone = 0.0f;

						ImGui::ProgressBar(fdone, ImVec2(ImGui::GetContentRegionAvail().x - 10, 6), "");

#define MEDIAICONSIZE 20

						if (ImGui::ImgBtn(MEDIA_PLAY, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
						{
							bVideoPerccentStart = true;
							if (bVideoResumePossible) {
								iVideoDelayExecute = 2; //resume
							}
							else {
								iVideoDelayExecute = 1; //play - restart.
							}
						}
						if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Play");
						ImGui::SameLine();
						if (ImGui::ImgBtn(MEDIA_PAUSE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
						{
							iVideoDelayExecute = 3; // pause
						}
						if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Pause");
						ImGui::SameLine();
						if (ImGui::ImgBtn(MEDIA_REFRESH, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
						{
							bVideoPerccentStart = true;
							iVideoDelayExecute = 1; //play - restart.
						}
						if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Restart");



						if (!bVideoPlayerMaximized)
						{
							ImGui::SameLine();
							if (ImGui::ImgBtn(MEDIA_MAXIMIZE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true,false,false,false,false, bBoostIconColors))
							{
								bVideoPlayerMaximized = true;
							}
							if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Maximize");
						}
						else
						{
							ImGui::SameLine();
							if (ImGui::ImgBtn(MEDIA_MINIMIZE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true,false,false,false,false, bBoostIconColors))
							{
								bVideoPlayerMaximized = false;
							}
							if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Minimize");
						}



					}
				}
			}

			if (bVideoPlayerMaximized) {
				std::map<std::string, std::string>::iterator it = tutorial_description.find(cTutorialName);
				if (it != tutorial_description.end()) {
					cVideoDescription = it->second.c_str();
					ImGui::Separator();
					ImGui::Text("Description");
					ImGui::TextWrapped(cVideoDescription.Get());
				}
				bImGuiGotFocus = true;
			}

			if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
				//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
				ImGui::Text("");
				ImGui::Text("");
			}

			#endif

			ImGui::End();

			///

			ImGui::Begin(cTutWindowStepsName, &bHelp_Window, iGenralWindowsFlags);

			ImGui::PushItemWidth(-10);
			if (ImGui::StyleButton(tut.cStartText, ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 0.0f))) {
				tut.bActive = true;
				tut.iCurrent_Step = 0;
				//PE: Minimize video player.
				bVideoPlayerMaximized = false;
			}
			ImGui::PopItemWidth();

			for (int il = 0; il < tut.iSteps; il++) {

				int additional_lines = 0;
				char line_split[TUTORIALMAXTEXT], *line_found = NULL, *line_start = NULL;
				strcpy(line_split, tut.cStepText[il]);
				line_start = line_found = &line_split[0];
				while ((line_found = (char *)pestrcasestr(line_found, "\\n"))) {
					line_found++;
					line_found++;
					additional_lines++;
				}
				float stepboxheight = mCharAdvance.y * (3 + additional_lines);

				cstr uniqueid = "##STEP";
				uniqueid += Str(il);

				float fOldChildRounding = ImGui::GetStyle().ChildRounding;
				ImGui::GetStyle().ChildRounding = 10.0f;

				if (tut.bActive && tut.iCurrent_Step == il) { //Set current step color.
					ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = ImGui::GetStyle().Colors[ImGuiCol_Button];
					static int last_scroll_set = -1;
					if (tut.iCurrent_Step != last_scroll_set) {
						last_scroll_set = tut.iCurrent_Step;
						ImGui::SetScrollHereY();
					}
				}

				ImGui::BeginChild(uniqueid.Get(), ImVec2(ImGui::GetContentRegionAvail().x-10.0, stepboxheight) , true, iGenralWindowsFlags);
					
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, mCharAdvance.y*0.5 ));

				ImGui::SetWindowFontScale(1.15);
				ImGui::TextCenter(tut.cStepHeader[il]);
				ImGui::SetWindowFontScale(1.0);

				//Update Score:
				tut.fScore = (float) tut.iCurrent_Step / (float) (tut.iSteps-1) * 100.0f;
				if (tut.fScore <= 0.99f) tut.fScore = 0.0f;
				if (tut.fScore >= 99.9f) {
					tut.fScore = 100.0f;
					strcpy(tut.cStepText[tut.iSteps - 1], "COMPLETE - Well Done!");
				}
				else {
					strcpy(tut.cStepText[tut.iSteps - 1], "INCOMPLETE");
				}

//					if(tut.iCurrent_Step >= tut.iSteps-1)
//						sprintf(tut.cStepText[tut.iSteps-1], "Score: %.0f", tut.fScore);
					
				strcpy(line_split, tut.cStepText[il]);
				line_start = line_found = &line_split[0];

				while ((line_found = (char *)pestrcasestr(line_found, "\\n"))) {
					*line_found = 0;
					ImGui::TextCenter(line_start);
					line_found++;
					line_found++;
					line_start = line_found;
				}
				ImGui::TextCenter(line_start);

				ImGui::EndChild();

				ImGui::GetStyle().ChildRounding = fOldChildRounding;
				ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = oldImGuiCol_ChildWindowBg;

				ImGui::Spacing();
			}

			ImGui::Indent(-10);

			//Debug info.
			//ImGui::TextCenter("Steps: %ld" , tut.iSteps );
			//ImGui::TextCenter("Current: %ld" , tut.iCurrent_Step );
			//ImGui::Text("current_tutorial: %ld", current_tutorial);

			if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
				//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
				ImGui::Text("");
				ImGui::Text("");
			}


			ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
			if (ImGui::IsMouseHoveringRect(bbwin.Min, bbwin.Max))
			{
				bImGuiGotFocus = true;
			}
			if (ImGui::IsAnyItemFocused()) {
				bImGuiGotFocus = true;
			}
			//Tutorial really small min, as we have multiply dock to the same side.
			CheckMinimumDockSpaceSize(20.0f);

			ImGui::End();

		}
		else {
			//Help window closed , check if we ned to free any videos.
			if (tut.bVideoID > 0) {
				current_tutorial = -1; //make sure to reopen when window visible again.
				if (AnimationExist(tut.bVideoID)) {
					if (AnimationPlaying(tut.bVideoID))
						StopAnimation(tut.bVideoID);
					DeleteAnimation(tut.bVideoID);
					tut.bVideoID = 0;
					bVideoResumePossible = false;
				}
			}
		}

		//###############################
		//#### Welcome Screen Window ####
		//###############################

		//#############################
		//#### Market place Window ####
		//#############################

		//#####################
		//#### Info Window ####
		//#####################
		
		iInfoUniqueId = 500001;
		if (refresh_gui_docking == 0) {
			ImGui::SetNextWindowPosCenter(ImGuiCond_Once);
			bool bTmp = true;
			ImGui::Begin("Information##InformationWindow", &bTmp, 0);
			ImGui::End();
		}
		else if (bInfo_Window) {
			if (bInfo_Window_First_Run)
			{
				ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
				bInfo_Window_First_Run = false;
			}
			if (bInfo_Reload || cInfoImageLast != cInfoImage )
			{
				//Load new image.
				//cInfoImage
				image_setlegacyimageloading(true);
				//  Load editor images
				SetMipmapNum(1); //PE: mipmaps not needed.
				if (GetImageExistEx(INFOIMAGE))
					DeleteImage(INFOIMAGE);
				LoadImage(cInfoImage.Get(), INFOIMAGE);
				if (!GetImageExistEx(INFOIMAGE))
				{
					//Get default information image.
					LoadImage("tutorialbank\\information-default.jpg", INFOIMAGE);
				}
				SetMipmapNum(-1);
				image_setlegacyimageloading(false);
				cInfoImageLast = cInfoImage;
			}
			ImGui::Begin("Information##InformationWindow", &bInfo_Window, 0);

			if (GetImageExistEx(INFOIMAGE))
			{
				float fRegionWidth = ImGui::GetContentRegionAvailWidth();
				float img_w = ImageWidth(INFOIMAGE);
				float img_h = ImageHeight(INFOIMAGE);
				float fRatio = img_h / img_w;
				ImGui::ImgBtn(INFOIMAGE, ImVec2(fRegionWidth, fRegionWidth*fRatio), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
			}
			  
			//Get image and information text.

			ImGui::TextWrapped(cInfoMessage.Get());
			ImGui::End();
		}
		else
		{
			bInfo_Reload = true; //Reload on new run.
		}

		//###############
		//#### About ####
		//###############

		//PE: PRODUCTV3 version.
		if (refresh_gui_docking == 0 ) {
			#ifdef VRTECH
			ImGui::SetNextWindowSize(ImVec2(28 * ImGui::GetFontSize(), 44 * ImGui::GetFontSize()), ImGuiCond_Once); //ImGuiCond_FirstUseEver
			#else
			ImGui::SetNextWindowSize(ImVec2(28 * ImGui::GetFontSize(), 34 * ImGui::GetFontSize()), ImGuiCond_Once); //ImGuiCond_FirstUseEver
			#endif
			ImGui::SetNextWindowPosCenter(ImGuiCond_Once);
			ImGui::Begin("About##AboutWindow", &bAbout_Window, 0);
			ImGui::End();
		}
		else if (bAbout_Window) {
			if (!bAbout_Init) {

				FILE* fAbout = GG_fopen("editors\\uiv3\\about.txt", "r");
				if (fAbout)
				{
					char ctmp[MAX_PATH];
					while (!feof(fAbout))
					{
						fgets(ctmp, MAX_PATH - 1, fAbout);
						if (strlen(ctmp) > 0 && ctmp[strlen(ctmp) - 1] == '\n')
							ctmp[strlen(ctmp) - 1] = 0;

						about_text.push_back(&ctmp[0]);
					}
					fclose(fAbout);
				}
				bAbout_Init = true;
			}
			if (bAbout_Window_First_Run)
			{
				#ifdef VRTECH
				ImGui::SetNextWindowSize(ImVec2(28 * ImGui::GetFontSize(), 44 * ImGui::GetFontSize()), ImGuiCond_Always); //ImGuiCond_FirstUseEver
				#else
				ImGui::SetNextWindowSize(ImVec2(28 * ImGui::GetFontSize(), 34 * ImGui::GetFontSize()), ImGuiCond_Always); //ImGuiCond_FirstUseEver
				#endif
				ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
				bAbout_Window_First_Run = false;
			}

			ImGui::Begin("About##AboutWindow", &bAbout_Window, 0);

			ImGui::Text("");

			float fRegionWidth = ImGui::GetWindowContentRegionWidth();
			float img_w = ImageWidth(ABOUT_LOGO);
			float img_h = ImageHeight(ABOUT_LOGO);

			#ifdef VRTECH
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((fRegionWidth*0.5) - (img_w*0.5), 0.0f));
			ImGui::ImgBtn(ABOUT_LOGO, ImVec2(img_w, img_h), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
			ImGui::TextCenter("");
			#endif

			char pBuildText[1024];
			sprintf(pBuildText, "Build: %s", g.version_s.Get());
			ImGui::TextCenter(pBuildText);
			ImGui::TextCenter("");

			#ifdef VRTECH
			for (int vloop = 0; vloop < about_text.size(); vloop++) {

				if (pestrcasestr(about_text[vloop].Get(), "https://") || pestrcasestr(about_text[vloop].Get(), "http://")) {
					ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((fRegionWidth*0.5) - (150.0f*0.5), 0.0f));
					char *tmp;
					cstr sUrl = about_text[vloop].Upper();
					if (pestrcasestr(about_text[vloop].Get(), "https://"))
						tmp = sUrl.Get() + 8;
					else
						tmp = sUrl.Get() + 7;
					if (tmp[strlen(tmp) - 1] == '/')
						tmp[strlen(tmp) - 1] = 0;
					ImGui::SetWindowFontScale(0.90);
					if (ImGui::StyleButton(tmp, ImVec2(150.0f, 0.0f))) {
						ExecuteFile(about_text[vloop].Get(), "", "", 0);
					}
					ImGui::SetWindowFontScale(1.0);
				}
				else {
					ImGui::TextCenter(about_text[vloop].Get());
				}

			}
			ImGui::Text("");
			#else
			ImGui::Text("");
			ImGui::TextCenter("(c)Copyright 2005 - 2022 The Game Creators Ltd.");
			ImGui::TextCenter("All Rights Reserved.");
			ImGui::Text("");
			ImGui::Text("");
			#endif

			float fTotalWidth = ImageWidth(ABOUT_TGC);
			#ifdef PRODUCTV3
			fTotalWidth += ImageWidth(ABOUT_HB);
			fTotalWidth += 30.0f;
			#endif

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((fRegionWidth*0.5) - (fTotalWidth*0.5), 0.0f));
			img_w = ImageWidth(ABOUT_TGC);
			img_h = ImageHeight(ABOUT_TGC);
			ImGui::ImgBtn(ABOUT_TGC, ImVec2(img_w, img_h), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
			#ifdef PRODUCTV3
			ImGui::SameLine();
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(20.0f,0.0f));
			img_w = ImageWidth(ABOUT_HB);
			img_h = ImageHeight(ABOUT_HB);
			ImGui::ImgBtn(ABOUT_HB, ImVec2(img_w, img_h), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
			#endif

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((fRegionWidth*0.5) - (fTotalWidth*0.5), 0.0f));
			ImGui::SetWindowFontScale(0.90);
			if (ImGui::StyleButton("THEGAMECREATORS.COM", ImVec2(ImageWidth(ABOUT_TGC),0))) {
				ExecuteFile("https://www.thegamecreators.com", "", "", 0);
			}
			#ifdef PRODUCTV3
			ImGui::SetWindowFontScale(1.0);
			ImGui::SameLine();
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(20.0f, 0.0f));
			ImGui::SetWindowFontScale(0.90);
			if (ImGui::StyleButton("HAMILTONBUHL.COM", ImVec2(ImageWidth(ABOUT_HB), 0))) {
				ExecuteFile("https://www.hamiltonbuhl.com", "", "", 0);
			}
			#endif
			ImGui::SetWindowFontScale(1.0);
			ImGui::Text("");

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((fRegionWidth*0.5) - (100.0f*0.5), 0.0f));
			if (ImGui::StyleButton("OK", ImVec2(100.0f, 0.0f))) {
				bAbout_Window = false;
			}
			ImGui::Text("");
			bImGuiGotFocus = true;
			ImGui::End();
		}

		//##################
		//#### Importer ####
		//##################

		if (refresh_gui_docking == 0 && !bImporter_Window) 
		{
			//Make sure window is setup in docking space.
			ImGui::Begin("Importer##ImporterWindow", &bImporter_Window, iGenralWindowsFlags);
			ImGui::End();
		}
		imgui_importer_loop();

		//#########################
		//#### Help Menu Image ####
		//#########################
		static bool bReadyToProcessMouse = false;
		if(bHelp_Menu_Image_Window) {
				
			if (GetImageExistEx(HELPMENU_IMAGE)) {
				ImGui::OpenPopup("Help##HelpMenuImage");

				float img_w = ImageWidth(HELPMENU_IMAGE);
				float img_h = ImageHeight(HELPMENU_IMAGE);

				ImGui::SetNextWindowPosCenter(ImGuiCond_Always);

				if (ImGui::BeginPopupModal("Help##HelpMenuImage", &bHelp_Menu_Image_Window, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings )) { //ImGuiWindowFlags_AlwaysAutoResize
					//@Lee if you only want 1:1 pixel remove the below (but it could go outside windows).
					//@Lee if 1:1 is possible it will do it.
					if (img_w > viewPortSize.x || img_h > viewPortSize.y) {
						float fRatio = 1.0f / (img_w / img_h);
						img_w = viewPortSize.x;
						img_h = viewPortSize.x * fRatio;
						if (img_h > viewPortSize.y) {
							float fRatio = 1.0f / (img_h / img_w);
							img_h = viewPortSize.y;
							img_w = viewPortSize.y * fRatio;
						}
					}
					ImGui::ImgBtn(HELPMENU_IMAGE, ImVec2(img_w, img_h), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
					bImGuiGotFocus = true;
					ImGui::EndPopup();
				}

			}
			else {
				bHelp_Menu_Image_Window = false;
			}

			//Close no matter where is clicked.
			ImGuiIO& io = ImGui::GetIO();
			if (ImGui::IsKeyPressed(27)) {
				bHelp_Menu_Image_Window = false;
			}
			if (bReadyToProcessMouse && ImGui::IsMouseReleased(0) ) {
				bHelp_Menu_Image_Window = false;
			}
			if (io.MouseClicked[0] > 0) {
				bReadyToProcessMouse = true; //next frame
			}
		}
		else {
			bReadyToProcessMouse = false;
		}

		//###########################
		//#### Export Standalone ####
		//###########################

		if (bExport_Standalone_Window) {
			static char cStandalonePath[MAX_PATH] = "\0";
			static int iStandaloneCycle = 0;
			if (cStandalonePath[0] == 0) {
				g.exedir_s = g.myownrootdir_s;//GetDir();
				if(cstr(Right(g.myownrootdir_s.Get(), 1)) == "\\" )
					g.exedir_s += "My Games\\";
				else
					g.exedir_s += "\\My Games\\";
				strcpy(cStandalonePath, g.exedir_s.Get());
			}

			ImGui::OpenPopup("Save Standalone##SaveStandaloneWindow");

			//ImGui::SetNextWindowPos(viewPortPos + ImVec2(180, 140), ImGuiCond_Appearing);// ImGuiCond_Once); //ImGuiCond_FirstUseEver
			ImGui::SetNextWindowSize(ImVec2(34 * ImGui::GetFontSize(), 24 * ImGui::GetFontSize()), ImGuiCond_Once); //ImGuiCond_FirstUseEver (was 13)
			ImGui::SetNextWindowPosCenter(ImGuiCond_Appearing);// ImGuiCond_Once);

			if (ImGui::BeginPopupModal("Save Standalone##SaveStandaloneWindow", &bExport_Standalone_Window, 0)) { //ImGuiWindowFlags_AlwaysAutoResize

				ImGui::Indent(10);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));

				ImGui::Text("Choose where you would like your standalone to be saved:");

				float col_start = 80.0f;

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
				ImGui::Text("Path");
				ImGui::SameLine();
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
				ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));
				//ImGui::PushItemWidth(-10);

				float path_gadget_size = ImGui::GetFontSize()*2.0;

				ImGui::PushItemWidth(-10 - path_gadget_size);
				ImGui::InputText("##InputPathCCP", &cStandalonePath[0], 250, ImGuiInputTextFlags_ReadOnly);
				if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;
				ImGui::PopItemWidth();

				ImGui::SameLine();
				ImGui::PushItemWidth(path_gadget_size);
				if (ImGui::StyleButton("...##ccppath")) {
					//PE: filedialogs change dir so.
					cStr tOldDir = GetDir();
					char * cFileSelected;
					cstr fulldir = cStandalonePath;
					cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_DIR, "All\0*.*\0", fulldir.Get() , NULL);

					SetDir(tOldDir.Get());

					if (cFileSelected && strlen(cFileSelected) > 0) {
						strcpy(cStandalonePath, cFileSelected);
						if (cStandalonePath[strlen(cStandalonePath) - 1] != '\\')
							strcat(cStandalonePath, "\\");
					}
				}
				ImGui::PopItemWidth();


				// Save or Cancel button
				ImGui::Indent(-10);
				float save_gadget_size = ImGui::GetFontSize()*10.0;
				float w = ImGui::GetWindowContentRegionWidth();
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5), 0.0f));
				if (iStandaloneCycle == 0) 
				{
					extern float g_mapfile_fProgress;
					g_mapfile_fProgress = 0.0f;

					if (ImGui::StyleButton("SAVE STANDALONE", ImVec2(save_gadget_size, 0))) {
						g.exedir_s = cStandalonePath;
						iStandaloneCycle = 1;
					}
				}
				else 
				{
					if (ImGui::StyleButton("CANCEL", ImVec2(save_gadget_size, 0))) {
						iStandaloneCycle = 5;
					}
				}

				ImGui::Indent(10);

				float fdone = (float)mapfile_savestandalone_getprogress() / 100.0f;

				if (iStandaloneCycle == 1) fdone = 0.01f;

				if (fdone > 0.0f) {
					char tmp[32];
					sprintf(tmp, "Progress: %.0f%%", fdone*100.0f);
					ImGui::ProgressBar(fdone, ImVec2(ImGui::GetContentRegionAvail().x - 10, 26), tmp); //, ""
				}

				if (iStandaloneCycle == 2)
				{
					// start save standalone creation
					mapfile_savestandalone_start();
					iStandaloneCycle = 3;
				}
				if (iStandaloneCycle == 3)
				{
					// run standalone creation calls
					if (mapfile_savestandalone_continue() == 1)
					{
						// complete standalone creation
						iStandaloneCycle = 4;
					}
				}
				if (iStandaloneCycle == 4)
				{
					// complete standalone creation
					mapfile_savestandalone_finish();
					iStandaloneCycle = 0;
					strcpy(cTriggerMessage, "Save Standalone Done");
					bTriggerMessage = true;
					bExport_Standalone_Window = false; //Close window.
				}
				if (iStandaloneCycle == 5)
				{
					// cancel standalone creation
					mapfile_savestandalone_restoreandclose();
					iStandaloneCycle = 0;
					strcpy(cTriggerMessage, "Save Standalone Cancelled");
					bTriggerMessage = true;
					bExport_Standalone_Window = false; //Close window.
				}
				if (iStandaloneCycle == 1) iStandaloneCycle = 2;

				ImGui::Indent(-10);

				bImGuiGotFocus = true;

				ImGui::EndPopup();
			}
		}

		//############################
		//#### Save To Level Cloud ###
		//############################
		#ifdef VRTECH
		// allows flag to be reset if user closes save cloud popup directly
		static bool bSaveToGameCloudInitList = false;
		if (bExport_SaveToGameCloud_Window == false) bSaveToGameCloudInitList = false;
		if (bExport_SaveToGameCloud_Window)
		{
			// made static so retains last strings and values given
			static char pLicenseID[MAX_PATH];
			static char pShortLicenseID[MAX_PATH];
			static char cSaveToGameCloudPath[MAX_PATH] = "\0";
			static int iSaveToGameCloudCycle = 0;

			//bool bJustWantToDeleteSomeOldFiles = false;
			//bool bInternalModeToDeleteGames = false;
			//if (1)
			//{
			//	// use this code to WIPE ALL LEVELS FROM ALL CLOUDS ON SERVER!
			//	bJustWantToDeleteSomeOldFiles = true;
			//	bInternalModeToDeleteGames = true;
			//}
			if (cSaveToGameCloudPath[0] == 0)
			{
				strcpy(cSaveToGameCloudPath, g.fpscrootdir_s.Get());
				strcat(cSaveToGameCloudPath, "\\mapbank\\");
			}

			ImGui::OpenPopup("Save To Level Cloud##SaveToLevelCloudWindow");
			ImGui::SetNextWindowSize(ImVec2(34 * ImGui::GetFontSize(), 26 * ImGui::GetFontSize()), ImGuiCond_Once); //ImGuiCond_FirstUseEver
			ImGui::SetNextWindowPosCenter(ImGuiCond_Once);

			if (ImGui::BeginPopupModal("Save To Level Cloud##SaveToLevelCloudWindow", &bExport_SaveToGameCloud_Window, 0)) 
			{
				ImGui::Indent(10);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));

				ImGui::Text("Choose the level you would like to upload:");

				float col_start = 80.0f;

				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
				ImGui::Text("File");
				ImGui::SameLine();
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));

				ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));

				float path_gadget_size = ImGui::GetFontSize()*2.0;

				ImGui::PushItemWidth(-10 - path_gadget_size);
				ImGui::InputText("##InputPathCCP", &cSaveToGameCloudPath[0], 250, ImGuiInputTextFlags_ReadOnly);
				if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;
				ImGui::PopItemWidth();

				ImGui::SameLine();
				ImGui::PushItemWidth(path_gadget_size);
				if (ImGui::StyleButton("...##ccppath"))
				{
					cStr tOldDir = GetDir();
					char * cFileSelected;
					cstr currentfile = cSaveToGameCloudPath;
					cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "All\0*.*\0", currentfile.Get(), NULL);
					SetDir(tOldDir.Get());

					if (cFileSelected && strlen(cFileSelected) > 0) 
					{
						strcpy(cSaveToGameCloudPath, cFileSelected);
					}
				}
				ImGui::PopItemWidth();
				ImGui::Indent(-10);

				float save_gadget_size = ImGui::GetFontSize()*10.0;
				float w = ImGui::GetWindowContentRegionWidth();
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5), 0.0f));

				static int iCloudLevelListCount = 0;
				static char** pCloudLevelList = NULL;
				static char** pCloudLevelListDisplay = NULL;
				extern std::vector<cstr> g_gamecloud_gamelist; //cyb
				static int iTeacherCode = 0;

				if (iSaveToGameCloudCycle == 0) 
				{
					// initially get game list
					if (bSaveToGameCloudInitList == false)
					{
						// as it speeds up potential delete game step as getlist moved earlier
						bSaveToGameCloudInitList = true;
						//mp_gamecloud_getlist(); //cyb

						// LICENSE-ID is used to schools/users can identify their game from all the others
						strcpy(pLicenseID, "NOSITE");
						cstr SiteName_s;
						char pObfSitename[1024];
						strcpy(pObfSitename, "12345-12345-12345-12345");
						if (FileExist(cstr(g.fpscrootdir_s + "\\cleverbooksmode.ini").Get()) == 1)
						{
							OpenToRead(1, cstr(g.fpscrootdir_s + "\\cleverbooksmode.ini").Get());
							cstr ActuallyUserEmailAddress_s = ReadString(1);
							// convert to a serial key format = 12345-12345-12345-12345
							SiteName_s = Left(Upper(ActuallyUserEmailAddress_s.Get()), 23);
							char pSiteNameConverted[25];
							memset(pSiteNameConverted, 0, sizeof(pSiteNameConverted));
							strcpy(pSiteNameConverted, SiteName_s.Get());
							for (int n = 0; n < 23; n++)
							{
								if (pSiteNameConverted[n] >= '0' && pSiteNameConverted[n] <= '9')
								{
									// numerics okay
								}
								else
								{
									// cap everything else into upper case alpha characters
									if (pSiteNameConverted[n] < 'A') pSiteNameConverted[n] = 'A';
									if (pSiteNameConverted[n] > 'Z') pSiteNameConverted[n] = 'Z';
								}
							}
							while (strlen(pSiteNameConverted) < 23) strcat(pSiteNameConverted, "Z");
							// to ensure emails contribute to unique first-five-digits, compount repeated sets of five on the first five
							for (int first = 0; first < 5; first++)
							{
								for (int rest = 5 + first; rest < 23; rest += 5)
								{
									int iShiftPlaces = pSiteNameConverted[rest] - 'A';
									for (int shift = 0; shift < iShiftPlaces; shift++)
									{
										pSiteNameConverted[first] = pSiteNameConverted[first] + 1;
										if (pSiteNameConverted[first] > 'Z')
											pSiteNameConverted[first] = 'A';
									}
								}
							}
							strrev(pSiteNameConverted);
							strcpy(pObfSitename, pSiteNameConverted);
							CloseFile(1);
							strcpy(pLicenseID, pObfSitename);
						}
						else
						{
							if (FileExist(cstr(g.fpscrootdir_s + "\\vrqcontrolmode.ini").Get()) == 1)
							{
								OpenToRead(1, cstr(g.fpscrootdir_s + "\\vrqcontrolmode.ini").Get());
								SiteName_s = ReadString(1);
								strcpy(pObfSitename, SiteName_s.Get());
								for (int n = 0; n < strlen(pObfSitename); n++)
								{
									if (pObfSitename[n] == '-')
										pObfSitename[n] = 'Z';
									else
										pObfSitename[n] = pObfSitename[n] + 1;
								}
								CloseFile(1);
								strcpy(pLicenseID, pObfSitename);
							}
						}

						// recreate list for combo
						if (pCloudLevelList)
						{
							for (int l = 0; l < iCloudLevelListCount; l++)
								delete pCloudLevelList[l];
							delete pCloudLevelList;
							for (int l = 0; l < iCloudLevelListCount; l++)
								delete pCloudLevelListDisplay[l];
							delete pCloudLevelListDisplay;
							iCloudLevelListCount = 0;
							pCloudLevelList = NULL;
						}
						if (pCloudLevelList == NULL)
						{
							iCloudLevelListCount = 1;
							for (int l = 0; l < g_gamecloud_gamelist.size(); l++)
							{
								LPSTR pLevelFile = g_gamecloud_gamelist[l].Get();
								if (strnicmp (pLevelFile, pLicenseID, strlen(pLicenseID)) == NULL)
								{
									iCloudLevelListCount++;
								}
							}
							pCloudLevelList = new char*[iCloudLevelListCount];
							pCloudLevelList[0] = new char[256];
							strcpy (pCloudLevelList[0], "");
							pCloudLevelListDisplay = new char*[iCloudLevelListCount];
							pCloudLevelListDisplay[0] = new char[256];
							strcpy (pCloudLevelListDisplay[0], "No Level Selected");
							iCloudLevelListCount = 1;
							if (strlen(pLicenseID) > 0)
							{
								for (int l = 0; l < g_gamecloud_gamelist.size(); l++)
								{
									LPSTR pLevelFile = g_gamecloud_gamelist[l].Get();
									if (strnicmp (pLevelFile, pLicenseID, strlen(pLicenseID)) == NULL)
									{
										pCloudLevelList[iCloudLevelListCount] = new char[1024];
										strcpy (pCloudLevelList[iCloudLevelListCount], pLevelFile);
										pCloudLevelListDisplay[iCloudLevelListCount] = new char[1024];
										strcpy (pCloudLevelListDisplay[iCloudLevelListCount], pLevelFile + strlen(pLicenseID) + 1);
										iCloudLevelListCount++;
									}
								}
							}
						}

						// ensures teacher can delete a level, then return software to user safely!
						iTeacherCode = 0; 
				
						// now create short license ID (five digits)
						strcpy(pShortLicenseID, pLicenseID);
						pShortLicenseID[5] = 0;
					}

					extern float g_mapfile_fProgress;
					g_mapfile_fProgress = 0.0f;

					// only used a few times
					//if (bInternalModeToDeleteGames == true)
					//{
					//	if (ImGui::StyleButton("DELETE LEVEL CLOUD", ImVec2(save_gadget_size, 0)))
					//	{
					//		bJustWantToDeleteSomeOldFiles = true;
					//		iSaveToGameCloudCycle = 1;
					//	}
					//}
					//else
					{
						if (ImGui::StyleButton("SAVE TO LEVEL CLOUD", ImVec2(save_gadget_size, 0)))
						{
							if (FileExist(cSaveToGameCloudPath) == 1)
								iSaveToGameCloudCycle = 1;
							else
								MessageBoxA(NULL, "You must specify an FPM to save to level cloud", "Save To Level Cloud", MB_OK);
						}
					}

					// instructions for Oculus levels
					ImGui::Indent(10);
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
					ImGui::TextWrapped("As the Standalone VR Headset runs on different graphics hardware, ensure your levels only contain a small number of characters and no imported models. This ensures the VR experience remains at full speed.");

					// about the school code
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
					ImGui::TextWrapped("When you launch the Standalone VR Player for the first time, you will be required to enter a School Code. Enter one of these codes:");

					// the code
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
					ImGui::Text(pShortLicenseID);
					ImGui::Text(pLicenseID);

					// new delete cloud levels feature
					ImGui::Text("");
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
					ImGui::TextWrapped("You can delete levels from the level cloud. Use your Teacher Code to view all the levels that can be deleted.");
					if (iTeacherCode != 12345)
					{
						ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5), 0.0f));
						ImGui::PushItemWidth(save_gadget_size);
						if (ImGui::InputInt("##Teacher Code", &iTeacherCode, 0, 0, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue))
						{
						}
						ImGui::PopItemWidth();
					}
					else
					{
						// present combo of all levels
						static int iCurrentlySelectedCloudLevel = 0;
						if (bSaveToGameCloudInitList == true)
						{
							ImGui::PushItemWidth(w-20);
							if (ImGui::Combo("##Cloud Levels", &iCurrentlySelectedCloudLevel, pCloudLevelListDisplay, iCloudLevelListCount, 20))
							{
							}
							ImGui::PopItemWidth();
						}

						// button to delete
						ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5), 0.0f));
						if (ImGui::StyleButton("DELETE LEVEL", ImVec2(save_gadget_size, 0)))
						{
							if (iCurrentlySelectedCloudLevel == 0)
							{
								MessageBoxA (NULL, "You must first select a level from the list above", "Delete Level Confirmation", MB_OK);
							}
							else
							{
								LPSTR pLevelToDelete = pCloudLevelList[iCurrentlySelectedCloudLevel];
								LPSTR pLevelToDeleteDisplay = pCloudLevelListDisplay[iCurrentlySelectedCloudLevel];
								char pMessage[2048];
								sprintf(pMessage, "Are you sure you want to delete %s?", pLevelToDeleteDisplay);
								if (MessageBoxA(NULL, pMessage, "Delete Level Confirmation", MB_YESNO) == IDYES)
								{
									// delete level in cloud
									//mp_gamecloud_delete(pLevelToDelete); //cyb

									// and force a refresh of the list
									bSaveToGameCloudInitList = false;
								}
							}
						}
					}
				}
				else 
				{
					if (ImGui::StyleButton("CANCEL", ImVec2(save_gadget_size, 0)))
					{
						iSaveToGameCloudCycle = 5;
					}
				}

				ImGui::Indent(10);

				float fdone = 0;//cyb // (float)mp_gamecloud_getprogress() / 100.0f;

				if (iSaveToGameCloudCycle == 1) fdone = 0.01f;

				if (fdone > 0.01f && fdone < 1.0f)
				{
					char tmp[32];
					sprintf(tmp, "Uploading: %.0f%%", fdone*100.0f);
					ImGui::ProgressBar(fdone, ImVec2(ImGui::GetContentRegionAvail().x - 10, 26), tmp); //, ""
				}

				if (iSaveToGameCloudCycle == 2)
				{
					// get file only, strip path (to check game list for duplicates)
					char pFileOnly[MAX_PATH];
					strcpy(pFileOnly, cSaveToGameCloudPath);
					for (int n = strlen(cSaveToGameCloudPath); n > 0; n--)
					{
						if (cSaveToGameCloudPath[n] == '/' || cSaveToGameCloudPath[n] == '\\')
						{
							strcpy(pFileOnly, cSaveToGameCloudPath + n + 1);
							break;
						}
					}

					//

					// Construct filename that will be used for the upload
					char pFinalFilenameToUse[MAX_PATH];
					//if (bJustWantToDeleteSomeOldFiles == false)
					{
						// normal use 
						strcpy(pFinalFilenameToUse, pLicenseID);
						strcat(pFinalFilenameToUse, "-");
						strcat(pFinalFilenameToUse, pFileOnly);
					}
					//else
					//{
					//	// to delete old filenames
					//	strcpy(pFinalFilenameToUse, pFileOnly);
					//}

					// can remove ALL game files
					//if (bInternalModeToDeleteGames==true)
					//{
					//	mp_gamecloud_deleteALLgamefiles(NULL);
					//	iSaveToGameCloudCycle = 4;
					//}
					//else
					{
						// start save to level cloud, first check existing games up there
						if (0)//(mp_gamecloud_overwriteexisting(pFinalFilenameToUse) != -1) //cyb
						{
							//if (bJustWantToDeleteSomeOldFiles == false)
							{
								int iUploadResult = 0;//cyb // mp_gamecloud_upload(true, cSaveToGameCloudPath, pFinalFilenameToUse);
								if (iUploadResult != -1)
								{
									if (iUploadResult == 1)
										iSaveToGameCloudCycle = 4;
									else
										iSaveToGameCloudCycle = 3;
								}
								else
									iSaveToGameCloudCycle = 6;
							}
							//else
							//	iSaveToGameCloudCycle = 6;
						}
						else
							iSaveToGameCloudCycle = 6;
					}
				}
				if (iSaveToGameCloudCycle == 3)
				{
					// upload cycle
					int iResultAsync = 0; //cyb // mp_gamecloud_upload(false, cSaveToGameCloudPath, NULL);
					if (iResultAsync != 0 )
					{
						// complete standalone creation
						if (iResultAsync == 1 )
							iSaveToGameCloudCycle = 4;
						else
							iSaveToGameCloudCycle = 6;
					}
				}
				if (iSaveToGameCloudCycle == 4)
				{
					// complete save to level cloud
					iSaveToGameCloudCycle = 0;
					bSaveToGameCloudInitList = false;
					strcpy(cTriggerMessage, "Save to Level Cloud Complete");
					bTriggerMessage = true;
					bExport_SaveToGameCloud_Window = false; //Close window.
				}
				if (iSaveToGameCloudCycle == 5)
				{
					// cancel standalone creation
					iSaveToGameCloudCycle = 0;
					bSaveToGameCloudInitList = false;
					strcpy(cTriggerMessage, "Save to Level Cloud Cancelled");
					bTriggerMessage = true;
					bExport_SaveToGameCloud_Window = false; //Close window.
				}
				if (iSaveToGameCloudCycle == 6)
				{
					// cancel popup
					iSaveToGameCloudCycle = 0;
					bSaveToGameCloudInitList = false;
					//strcpy(cTriggerMessage, mp_gamecloud_geterror()); //cyb
					bTriggerMessage = true;
					bExport_SaveToGameCloud_Window = false; //Close window.
				}
				if (iSaveToGameCloudCycle == 1) iSaveToGameCloudCycle = 2;

				ImGui::Indent(-10);

				bImGuiGotFocus = true;

				ImGui::EndPopup();
			}
		}
		#endif
		//########################
		//#### Download Store ####
		//########################

		//Waypoints##WaypointsToolsWindow
		if (refresh_gui_docking == 0 && !bDownloadStore_Window) 
		{
			//Make sure window is setup in docking space.
			ImGui::SetNextWindowSize(ImVec2(40 * ImGui::GetFontSize(), 30 * ImGui::GetFontSize()), ImGuiCond_Once);
			ImGui::SetNextWindowPosCenter(ImGuiCond_Once);

			ImGui::Begin("Download Store Items##DownloadStoreWindow", &bDownloadStore_Window, 0);
			ImGui::End();
		}
		//imgui_download_store(); //cyb


		//#####################
		//#### Preferences ####
		//#####################


		//########################
		//#### Waypoint Tools ####
		//########################

		//Waypoints##WaypointsToolsWindow
		if (refresh_gui_docking == 0 && !bWaypoint_Window) 
		{
			//Make sure window is setup in docking space.
			ImGui::Begin("Waypoints##WaypointsToolsWindow", &bWaypoint_Window, iGenralWindowsFlags);
			ImGui::End();
		}
		waypoint_imgui_loop();

		//#######################
		//#### Terrain Tools ####
		//#######################
		static bool bTerrainToolsDocked = true;

		if (!bTerrainToolsDocked || (refresh_gui_docking == 0 && !bTerrain_Tools_Window ))
		{
			//Make sure window is setup in docking space.
			bool bTrue = true;
			ImGui::Begin("Terrain Tools##TerrainToolsWindow", &bTerrain_Tools_Window, iGenralWindowsFlags);
			ImGui::End();
			bTerrainToolsDocked = true;
		}
		else
		{
			imgui_terrain_loop();
		}

		//############################
		//#### Builder Properties ####
		//############################

		if (refresh_gui_docking == 0 && !bBuilder_Properties_Window) 
		{
			//Make sure window is setup in docking space.
			ImGui::Begin("Structure Properties##BuilderPropertiesWindow", &bBuilder_Properties_Window, iGenralWindowsFlags);
			ImGui::End();
		}
		else {
			if(!bBuilder_Properties_Window)
				if (t.ebe.on == 1) ebe_hide();
		}
		imgui_ebe_loop();


		//###########################
		//#### Character Creator ####
		//###########################

		#ifdef VRTECH
		if (refresh_gui_docking == 0 && !g_bCharacterCreatorPlusActivated) 
		{
			//Make sure window is setup in docking space.
			ImGui::Begin("Character Creator##PropertiesWindow", &g_bCharacterCreatorPlusActivated, iGenralWindowsFlags);  //cyb
			ImGui::End();
		}
		charactercreatorplus_imgui();
		#endif

		//###########################
		//#### Entity Properties ####
		//###########################
			
		static int iOldPickedEntityIndex = -1;

		if (refresh_gui_docking == 0) ImGui::SetNextWindowPos(viewPortPos + ImVec2(400, 140), ImGuiCond_Always); //ImGuiCond_FirstUseEver,ImGuiCond_Once
		if (refresh_gui_docking == 0) ImGui::SetNextWindowSize(ImVec2(30 * ImGui::GetFontSize(), 40 * ImGui::GetFontSize()), ImGuiCond_Always); //ImGuiCond_FirstUseEver

		if (refresh_gui_docking == 0) 
		{
			//Need to be here while first time docking.
			ImGui::Begin("Entity Properties##PropertiesWindow", &bEntity_Properties_Window, iGenralWindowsFlags);
			ImGui::End();
		}
		else if (bEntity_Properties_Window) {
				

			if (t.widget.pickedEntityIndex > 0 && t.cameraviewmode == 2) {

				//We are in properties mode.
				grideleprof_uniqui_id = 35000;

				static int current_loaded_script = -1;
				static int current_selected_script = 0;
				static bool current_loaded_script_has_dlua = false;

				int iParentEntid = t.ttrygridentity;
					
				if (iOldPickedEntityIndex != t.widget.pickedEntityIndex) 
				{
					//New item , backup grideleprof , for cancel function.
					//PE: NOTE this do not backup position ...
					//backup_grideleprof = t.grideleprof; //PE: We dont use a cancel system anymore.


					iOldPickedEntityIndex = t.widget.pickedEntityIndex;

					//t.gridentity can be changed when keys like "r" is used , so make a backup.
					iOldgridentity = t.gridentity;

					// get voices sets
					#ifdef VRTECH
					if (g_voiceList_s.size() == 0)
					{
						if (0)//cyb //(CreateListOfVoices() > 0) 
						{
							pCCPVoiceSet = g_voiceList_s[0].Get();
							CCP_SelectedToken = g_voicetoken[0];
						}
					}
					#endif

					// entity may have voice preferences set to check that
					#ifdef VRTECH
					pCCPVoiceSet = t.grideleprof.voiceset_s.Get();
					CCP_Speak_Rate = t.grideleprof.voicerate;
					if (strlen(pCCPVoiceSet) > 0)
					{
						// find token for this voiceset
						for (int n = 0; n < g_voiceList_s.size(); n++)
						{
							if (stricmp(g_voiceList_s[n].Get(), pCCPVoiceSet) == NULL)
							{
								CCP_SelectedToken = g_voicetoken[n];
								break;
							}
						}
					}
					else
					{
						// default if blank
						if ( g_voiceList_s.size() > 0 ) 
							pCCPVoiceSet = g_voiceList_s[0].Get();
						else
							pCCPVoiceSet = "";
						if (g_voicetoken.size() > 0)
							CCP_SelectedToken = g_voicetoken[0];
						else
							CCP_SelectedToken = NULL;
						CCP_Speak_Rate = 0;
					}
					#endif
					//Make sure to read DLUA.
					current_loaded_script = -1;
				}


				ImGui::Begin("Entity Properties##PropertiesWindow", &bEntity_Properties_Window, iGenralWindowsFlags);

				int media_icon_size = 64;
				ImGui::BeginChild("##cEntitiesPropertiesHeader", ImVec2(0, 0),false, iGenralWindowsFlags);
					
				ImGui::SetWindowFontScale(0.90);
				ImGui::PushItemWidth(ImGui::GetFontSize()*10.0);

				if (bEntity_Properties_Window)
				{
					t.gridentity = iOldgridentity;

					//Collect the flags to use.
					imgui_set_openproperty_flags(t.gridentity);
					int tflagtext = 0, tflagimage=0;
					if (t.entityprofile[t.gridentity].ismarker == 3)
					{
						if (!t.entityprofile[t.gridentity].markerindex <= 1)
						{
							#ifdef VRTECH
							#else
								if (t.entityprofile[t.gridentity].markerindex == 2) tflagtext = 1;
								if (t.entityprofile[t.gridentity].markerindex == 3) tflagimage = 1;
							#endif
						}
					}

					LPSTR pAIRoot = "scriptbank\\";
					if (t.tflagai == 1)
					{
						if (g.quickparentalcontrolmode == 2)
						{
							if (t.entityprofile[t.gridentity].ismarker == 0)
							{
								if (t.tflagchar == 1)
									pAIRoot = "scriptbank\\people\\";
								else
									pAIRoot = "scriptbank\\objects\\";
							}
							else
							{
								pAIRoot = "scriptbank\\markers\\";
							}
						}
					}
						

					if (t.tsimplecharview == 1)
					{
						//  Wizard (simplified) property editing
						t.group = 0;
						if (ImGui::StyleCollapsingHeader("Character Info", ImGuiTreeNodeFlags_DefaultOpen)) {

							t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), t.strarr_s[413].Get(), "Choose a unique name for this character");
							t.grideleprof.aimain_s = imgui_setpropertylist2(t.group, t.controlindex, t.grideleprof.aimain_s.Get(), "Behaviour", "Select a behaviour for this character", 11);
							t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), "Voiceover", "Select t.a WAV or OGG file this character will use during their behavior", "audiobank\\");
							t.grideleprof.ifused_s = imgui_setpropertystring2(t.group, t.grideleprof.ifused_s.Get(), "If Used", "Sometimes used to specify the name of an entity to be activated");
						}
					}
					else
					{

						bool bUnfoldAdvanced = false;
						ImGui::SetWindowFontScale(1.0);
						float fRegionWidth = ImGui::GetWindowContentRegionWidth();
						float textwidth;
						fPropertiesColoumWidth = 90.0f;
						int adv_flasgs = ImGuiTreeNodeFlags_DefaultOpen;
						if (g.vrqcontrolmode > 0) 
						{
							//Simple version.
							adv_flasgs = ImGuiTreeNodeFlags_None;
								
							//##################################################################
							//#### Simple , perhaps based on current .lua script.           ####
							//#### We need unique id here so add ##SimpleInput to all items ####
							//##################################################################
								
							if (t.entityprofile[t.gridentity].ischaracter > 0) {
								//Chars.

								if (ImGui::StyleCollapsingHeader("Character Properties", ImGuiTreeNodeFlags_DefaultOpen)) {

									//Display icon.
									if (t.entityprofile[iParentEntid].iThumbnailSmall > 0) {
										float w = ImGui::GetWindowContentRegionWidth();
										ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (media_icon_size*0.5), 0.0f));
										ImGui::ImgBtn(t.entityprofile[iParentEntid].iThumbnailSmall, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
									}

									ImGui::Indent(10);

									t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
									//bSoundSet , Male/Female
									//PE: Type removed.
									//t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), "Type", t.strarr_s[255].Get(), "audiobank\\voices\\");

									ImGui::Indent(-10);
								}

								int speech_entries = 0;
								bool bUpdateMainString = false;

								for (int speech_loop = 0; speech_loop < 5; speech_loop++)
									speech_ids[speech_loop] = -1;
								//behavior
								if (ImGui::StyleCollapsingHeader("Character Behavior", ImGuiTreeNodeFlags_DefaultOpen)) {

									ImGui::Indent(10);

									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Behaviors");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));

									ImGui::PushItemWidth(-10);

									// scan PEOPLE folder for complete list of script
									std::vector<cstr> scriptList_s; scriptList_s.clear();
									std::vector<cstr> scriptListTitle_s; scriptListTitle_s.clear();
									cstr oldDir_s = GetDir();
									SetDir(g.fpscrootdir_s.Get());
									SetDir("Files\\scriptbank\\people");
									ChecklistForFiles();
									for ( int f = 1; f <= ChecklistQuantity(); f++)
									{
										cstr tfile_s = ChecklistString(f);
										LPSTR pFilename = tfile_s.Get();
										if (tfile_s != "." && tfile_s != "..")
										{
											if (strnicmp(pFilename + strlen(pFilename) - 4, ".lua", 4) == NULL)
											{
												// create a readable title from file
												char pTitleName[256];
												strcpy(pTitleName, pFilename);
												pTitleName[strlen(pTitleName) - 4] = 0;
												for (int n = 0; n < strlen(pTitleName); n++)
												{
													if (n == 0)
													{
														if (pTitleName[n] >= 'a' && pTitleName[n] <= 'z' )
															pTitleName[n] -= ('a' - 'A');
													}
													else
													{
														if (pTitleName[n] >= 'A' && pTitleName[n] <= 'Z' )
															pTitleName[n] += ('a' - 'A');
													}
													if (pTitleName[n] == '_') pTitleName[n] = ' ';
												}

												// add script and title to list
												scriptList_s.push_back(cstr("people\\")+tfile_s);
												scriptListTitle_s.push_back(cstr(pTitleName));
											}
										}
									}
									scriptList_s.push_back(cstr(""));
									scriptListTitle_s.push_back(cstr("Custom"));
									SetDir(oldDir_s.Get());

									// and create items list
									static int g_scriptpeople_item_count = 0;
									static char** g_scriptpeople_items = NULL;
									if (g_scriptpeople_item_count != scriptList_s.size())
									{
										if (g_scriptpeople_items)
										{
											for (int i = 0; i < g_scriptpeople_item_count; i++) SAFE_DELETE(g_scriptpeople_items[i]);
											SAFE_DELETE(g_scriptpeople_items);
										}
										g_scriptpeople_item_count = scriptList_s.size();
										g_scriptpeople_items = new char*[g_scriptpeople_item_count];
										for (int i = 0; i < g_scriptpeople_item_count; i++)
										{
											g_scriptpeople_items[i] = new char[256];
											strcpy(g_scriptpeople_items[i], scriptListTitle_s[i].Get());
										}
									}

									int item_current_type_selection = g_scriptpeople_item_count - 1; //Default Custom.
									for (int i = 0; i < g_scriptpeople_item_count - 1; i++) 
									{
										if (pestrcasestr(t.grideleprof.aimain_s.Get(), scriptList_s[i].Get())) 
										{
											item_current_type_selection = i;
											break;
										}
									}

									if (current_loaded_script != item_current_type_selection) 
									{
										//Load in lua and check for custom properties.
										cstr script_name = "scriptbank\\";
										if (item_current_type_selection < g_scriptpeople_item_count - 1)
											script_name += (char *) scriptList_s[item_current_type_selection].Get();
										else
											script_name += t.grideleprof.aimain_s;
										//Try to parse script.
										ParseLuaScript(&t.grideleprof,script_name.Get());
										current_loaded_script = item_current_type_selection;

										if (t.grideleprof.PropertiesVariableActive == 1) 
										{
											bUpdateMainString = true;
											current_loaded_script_has_dlua = true;
										}
										else 
										{
											if (current_loaded_script_has_dlua) 
											{
												//Reset t.grideleprof.soundset4_s that contain the dlua calls.
												t.grideleprof.soundset4_s = "";
												current_loaded_script_has_dlua = false;
											}
										}
									}

									if (ImGui::Combo("##BehavioursSimpleInput", &item_current_type_selection, g_scriptpeople_items, g_scriptpeople_item_count, 20)) 
									{
										if (item_current_type_selection < g_scriptpeople_item_count - 1) 
										{
											t.grideleprof.aimain_s = scriptList_s[item_current_type_selection].Get();
										}
										else 
										{
											t.grideleprof.aimain_s = "";
										}
									}
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Select Character Behavior");

									ImGui::PopItemWidth();

									if (item_current_type_selection == g_scriptpeople_item_count - 1) 
									{
										//Custom script , display directly.
										std::string ms = t.strarr_s[417].Get();
										ms = "Script";
										cstr aim = t.grideleprof.aimain_s;
										t.grideleprof.aimain_s = imgui_setpropertyfile2(t.group, t.grideleprof.aimain_s.Get(), (char *)ms.c_str(), t.strarr_s[207].Get(), pAIRoot);
										if (aim != t.grideleprof.aimain_s)
											current_loaded_script = -1;
									}

									if (t.grideleprof.PropertiesVariableActive == 1) {
										speech_entries = DisplayLuaDescription(&t.grideleprof);
									}
									else {
										if (t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0) {
											DisplayLuaDescriptionOnly(&t.grideleprof);
										}
									}

									ImGui::Indent(-10);
								}

								#ifdef VRTECH
								if(speech_entries > 0)
								{
									//@Lee all SPEECH control is moved to this function.
									SpeechControls(speech_entries, bUpdateMainString);
								}
								#endif

								if (ImGui::StyleCollapsingHeader("Customize", ImGuiTreeNodeFlags_DefaultOpen)) {

									ImGui::Indent(10);

									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Move Speed");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
									ImGui::PushItemWidth(-10);
									ImGui::MaxSliderInputInt("##Movement SpeedSimpleInput", &t.grideleprof.speed, 1, 500, "Set Movement Speed");

									if (t.playercontrol.thirdperson.enabled == 1) t.tanimspeed_f = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed;
									else t.tanimspeed_f = t.grideleprof.animspeed;

									ImGui::PopItemWidth();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Anim Speed");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
									ImGui::PushItemWidth(-10);
									int tmpint = t.tanimspeed_f;
									ImGui::MaxSliderInputInt("##Animation Speed Simple", &tmpint, 1, 500, "Set Animation Speed");

									t.tanimspeed_f = tmpint;
									ImGui::PopItemWidth();
									if (t.playercontrol.thirdperson.enabled == 1) t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed = t.tanimspeed_f;
									else t.grideleprof.animspeed = t.tanimspeed_f;

									ImGui::Indent(-10);
								}
							}
							else if (t.tflaglight == 1)
							{
								if (ImGui::StyleCollapsingHeader("Name", ImGuiTreeNodeFlags_DefaultOpen)) {

									//Display icon.
									if (t.entityprofile[iParentEntid].iThumbnailSmall > 0) {
										float w = ImGui::GetWindowContentRegionWidth();
										ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (media_icon_size*0.5), 0.0f));
										ImGui::ImgBtn(t.entityprofile[iParentEntid].iThumbnailSmall, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
									}

									ImGui::Indent(10);

									//Name and color setup only.
									ImGui::Text("");
									t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
									//ImGui::Text("");

									ImGui::Indent(-10);

								}

								if (ImGui::StyleCollapsingHeader("Customize", ImGuiTreeNodeFlags_DefaultOpen)) {

									ImGui::Indent(10);

									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Light Distance");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
									ImGui::PushItemWidth(-10);

									ImGui::SliderInt("##Light RangeSimpleInput", &t.grideleprof.light.range, 1, 3000);
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", t.strarr_s[250].Get() );
									ImGui::PopItemWidth();


									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Light Color");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
									ImGui::PushItemWidth(-10);
									ImGui::Text(""); //place it below text, so it get larger.

									float colors[5];
									colors[3] = ((t.grideleprof.light.color & 0xff000000) >> 24) / 255.0f;
									colors[0] = ((t.grideleprof.light.color & 0x00ff0000) >> 16) / 255.0f;
									colors[1] = ((t.grideleprof.light.color & 0x0000ff00) >> 8) / 255.0f;
									colors[2] = (t.grideleprof.light.color & 0x000000ff) / 255.0f;
									ImGui::ColorPicker3("##Light ColorSimpleInput", colors, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoLabel);
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", t.strarr_s[251].Get() );

									colors[0] *= 255.0f;
									colors[1] *= 255.0f;
									colors[2] *= 255.0f;
									colors[3] *= 255.0f;
									t.grideleprof.light.color = 0xff000000 + ((unsigned int)colors[0] << 16) + ((unsigned int)colors[1] << 8) + +((unsigned int)colors[2]);
									ImGui::PopItemWidth();

									// update the light live

									ImGui::Indent(-10);
								}
							}
							else if (t.entityprofile[t.gridentity].ismarker == 1) 
{
								//Start Marker.
								if (ImGui::StyleCollapsingHeader("Name", ImGuiTreeNodeFlags_DefaultOpen))
								{
									//Display icon.
									if (t.entityprofile[iParentEntid].iThumbnailSmall > 0) {
										float w = ImGui::GetWindowContentRegionWidth();
										ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (media_icon_size*0.5), 0.0f));
										ImGui::ImgBtn(t.entityprofile[iParentEntid].iThumbnailSmall, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
									}

									ImGui::Indent(10);

									//Name only.
									ImGui::Text("");
									t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());

									ImGui::Indent(-10);

								}

								// DLUA support added here.
								int speech_entries = 0;
								bool bUpdateMainString = false;
								for (int speech_loop = 0; speech_loop < 5; speech_loop++)
									speech_ids[speech_loop] = -1;
								if (current_loaded_script != current_selected_script) 
								{
									//Load in lua and check for custom properties.
									cstr script_name = "scriptbank\\";
									script_name += t.grideleprof.aimain_s;
									//Try to parse script.
									ParseLuaScript(&t.grideleprof, script_name.Get());
									current_loaded_script = current_selected_script;

									if (t.grideleprof.PropertiesVariableActive == 1) {
										bUpdateMainString = true;
										current_loaded_script_has_dlua = true;
									}
									else {
										if (current_loaded_script_has_dlua) {
											//Reset t.grideleprof.soundset4_s that contain the dlua calls.
											t.grideleprof.soundset4_s = "";
											current_loaded_script_has_dlua = false;
										}
									}

								}

								// Markers behaviours
								if (t.grideleprof.PropertiesVariableActive == 1 || t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0)
								{
									if (ImGui::StyleCollapsingHeader("Behaviors", ImGuiTreeNodeFlags_DefaultOpen)) {

										ImGui::Indent(10);

										if (t.grideleprof.PropertiesVariableActive == 1) {
											speech_entries = DisplayLuaDescription(&t.grideleprof);
										}
										else {
											if (t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0) {
												DisplayLuaDescriptionOnly(&t.grideleprof);
											}
										}

										ImGui::Indent(-10);
									}
								}
								#ifdef VRTECH
								if (speech_entries > 0)
								{
									// all SPEECH control is moved to this function.
									SpeechControls(speech_entries, bUpdateMainString);
								}
								#endif

								if (ImGui::StyleCollapsingHeader("Customize", ImGuiTreeNodeFlags_DefaultOpen)) {

									ImGui::Indent(10);

									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text("Player Speed");
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));

									ImGui::PushItemWidth(-10);

									//ImGui::SliderInt("##Movement SpeedSimpleInput", &t.grideleprof.speed, 1, 500);
									ImGui::MaxSliderInputInt("##Movement SpeedSimpleInput", &t.grideleprof.speed, 1, 500, "Set Player Speed");
									//if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Set Player Speed");

									if (t.playercontrol.thirdperson.enabled == 1) t.tanimspeed_f = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed;
									else t.tanimspeed_f = t.grideleprof.animspeed;

									ImGui::PopItemWidth();

									//  Has Weapon
									if (t.tflaghasweapon == 1 && t.playercontrol.thirdperson.enabled == 0 && g.quickparentalcontrolmode != 2)
									{
										t.grideleprof.hasweapon_s = imgui_setpropertylist2c(t.group, t.controlindex, t.grideleprof.hasweapon_s.Get(), "Attachment", t.strarr_s[209].Get(), 1);
									}

									ImGui::Indent(-10);
								}
							}
							else 
							{
								//#################
								//#### Objects ####
								//################# 

								t.tokay = 1;
								if (ObjectExist(g.entitybankoffset + t.gridentity) == 1)
								{
									if (GetNumberOfFrames(g.entitybankoffset + t.gridentity) > 0)
									{
										t.tokay = 0;
									}
								}

								int speech_entries = 0;
								bool bUpdateMainString = false;

								for (int speech_loop = 0; speech_loop < 5; speech_loop++)
									speech_ids[speech_loop] = -1;

								//health.lua
								cstr aimain = t.grideleprof.aimain_s.Lower();
								//new: trigger anyting not a marker.
								if (t.entityprofile[t.gridentity].ismarker == 0 )// || aimain == "key.lua" || aimain == "objects\\key.lua" || aimain == "door.lua" || aimain == "default.lua" || aimain == "health.lua" || aimain == "pickuppable.lua" ) ) 
								{
									if (ImGui::StyleCollapsingHeader("Name", ImGuiTreeNodeFlags_DefaultOpen))
									{
										//Display icon.
										if (t.entityprofile[iParentEntid].iThumbnailSmall > 0) {
											float w = ImGui::GetWindowContentRegionWidth();
											ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (media_icon_size*0.5), 0.0f));
											ImGui::ImgBtn(t.entityprofile[iParentEntid].iThumbnailSmall, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
										}

										ImGui::Indent(10);

										t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());

										ImGui::Indent(-10);
									}
										
									// Object behaviours
									if (ImGui::StyleCollapsingHeader("Behaviors", ImGuiTreeNodeFlags_DefaultOpen))
									{
										ImGui::Indent(10);

										// scan OBJECTS folder for complete list of script
										std::vector<cstr> scriptList_s; scriptList_s.clear();
										std::vector<cstr> scriptListTitle_s; scriptListTitle_s.clear();
										cstr oldDir_s = GetDir();
										SetDir(g.fpscrootdir_s.Get());
										SetDir("Files\\scriptbank\\objects");
										ChecklistForFiles();
										for ( int f = 1; f <= ChecklistQuantity(); f++)
										{
											cstr tfile_s = ChecklistString(f);
											LPSTR pFilename = tfile_s.Get();
											if (tfile_s != "." && tfile_s != "..")
											{
												if (strnicmp(pFilename + strlen(pFilename) - 4, ".lua", 4) == NULL)
												{
													// create a readable title from file
													char pTitleName[256];
													strcpy(pTitleName, pFilename);
													pTitleName[strlen(pTitleName) - 4] = 0;
													for (int n = 0; n < strlen(pTitleName); n++)
													{
														if (n == 0)
														{
															if (pTitleName[n] >= 'a' && pTitleName[n] <= 'z' )
																pTitleName[n] -= ('a' - 'A');
														}
														else
														{
															if (pTitleName[n] >= 'A' && pTitleName[n] <= 'Z' )
																pTitleName[n] += ('a' - 'A');
														}
														if (pTitleName[n] == '_') pTitleName[n] = ' ';
													}

													// add script and title to list
													scriptList_s.push_back(cstr("objects\\")+tfile_s);
													scriptListTitle_s.push_back(cstr(pTitleName));
												}
											}
										}
										scriptList_s.push_back(cstr(""));
										scriptListTitle_s.push_back(cstr("Custom"));
										SetDir(oldDir_s.Get());

										// and create items list
										static int g_scriptobjects_item_count = 0;
										static char** g_scriptobjects_items = NULL;
										if (g_scriptobjects_item_count != scriptList_s.size())
										{
											if (g_scriptobjects_items)
											{
												for (int i = 0; i < g_scriptobjects_item_count; i++) SAFE_DELETE(g_scriptobjects_items[i]);
												SAFE_DELETE(g_scriptobjects_items);
											}
											g_scriptobjects_item_count = scriptList_s.size();
											g_scriptobjects_items = new char*[g_scriptobjects_item_count];
											for (int i = 0; i < g_scriptobjects_item_count; i++)
											{
												g_scriptobjects_items[i] = new char[256];
												strcpy(g_scriptobjects_items[i], scriptListTitle_s[i].Get());
											}
										}

										// find selection
										int item_current_type_selection = g_scriptobjects_item_count - 1; //Default Custom.
										for (int i = 0; i < g_scriptobjects_item_count - 1 ; i++) 
										{
											if (pestrcasestr(t.grideleprof.aimain_s.Get(), scriptList_s[i].Get())) 
											{
												item_current_type_selection = i;
												break;
											}
										}
												
										ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
										ImGui::Text("Behaviors");
										ImGui::SameLine();
										ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
										ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
										ImGui::PushItemWidth(-10);

										if (ImGui::Combo("##Behaviours2SimpleInput", &item_current_type_selection, g_scriptobjects_items, g_scriptobjects_item_count, 20 ))
										{
											if (item_current_type_selection >= 0) 
												t.grideleprof.aimain_s = scriptList_s[item_current_type_selection];
											else 
												t.grideleprof.aimain_s = "default.lua";
											aimain = t.grideleprof.aimain_s.Lower();
										}
										if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Select Object Behavior");
										ImGui::PopItemWidth();

										if (current_loaded_script != item_current_type_selection) 
										{
											//Load in lua and check for custom properties.
											cstr script_name = "scriptbank\\";
											if (item_current_type_selection < g_scriptobjects_item_count - 1) //PE: Need to check for custom
												script_name += scriptList_s[item_current_type_selection];
											else
												script_name += t.grideleprof.aimain_s;
											//Try to parse script.
											ParseLuaScript(&t.grideleprof, script_name.Get());
											current_loaded_script = item_current_type_selection;

											if (t.grideleprof.PropertiesVariableActive == 1) 
											{
												bUpdateMainString = true;
												current_loaded_script_has_dlua = true;
											}
											else 
											{
												if (current_loaded_script_has_dlua) 
												{
													//Reset t.grideleprof.soundset4_s that contain the dlua calls.
													t.grideleprof.soundset4_s = "";
													current_loaded_script_has_dlua = false;
												}
											}
										}

										if (item_current_type_selection == g_scriptobjects_item_count - 1)
										{
											//Custom Behaviours , display directly.
											std::string ms = t.strarr_s[417].Get();
											ms = "Script";
											cstr aim = t.grideleprof.aimain_s;
											t.grideleprof.aimain_s = imgui_setpropertyfile2(t.group, t.grideleprof.aimain_s.Get(), (char *)ms.c_str(), t.strarr_s[207].Get(), pAIRoot);
											aimain = t.grideleprof.aimain_s.Lower();
											if (aim != t.grideleprof.aimain_s)
												current_loaded_script = -1;
										}

										if (t.grideleprof.PropertiesVariableActive == 1) 
										{
											speech_entries = DisplayLuaDescription(&t.grideleprof);
										}
										else 
										{
											if (t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0) 
											{
												DisplayLuaDescriptionOnly(&t.grideleprof);
											}
										}

										ImGui::Indent(-10);
									}
									#ifdef VRTECH
									if (speech_entries > 0)
									{
										// all SPEECH control is moved to this function.
										SpeechControls(speech_entries, bUpdateMainString);
									}
									#endif
								}
								else
									bUnfoldAdvanced = true;
							}
						}

						ImGui::SetWindowFontScale(1.0); //0.90

						fPropertiesColoumWidth = 120.0f; //Advanced coloum need to be larger as we have large fields.

						// All objects need 'certain fields' as pretty commmon  to have it for the script
						if ( t.entityprofile[t.gridentity].ismarker == 0 ) // so as not to interfere with markers
						{
							bool bSound0Mentioned = false;
							bool bSound1Mentioned = false;
							bool bSound2Mentioned = false;
							bool bSound3Mentioned = false;
							bool bVideoSlotMentioned = false;
							bool bIfUsedMentioned = false;
							bool bUseKeyMentioned = false;
							char pCaptureAnyScriptDesc[10240+(80*300)+(80*300)];
							strcpy(pCaptureAnyScriptDesc, t.grideleprof.PropertiesVariable.VariableDescription.Get());
							for (int i = 0; i < t.grideleprof.PropertiesVariable.iVariables; i++)
							{
								strcat(pCaptureAnyScriptDesc, t.grideleprof.PropertiesVariable.VariableSectionDescription[i]);
							}
							for (int i = 0; i < t.grideleprof.PropertiesVariable.iVariables; i++)
							{
								strcat(pCaptureAnyScriptDesc, t.grideleprof.PropertiesVariable.VariableSectionEndDescription[i]);
							}
							if (strstr(pCaptureAnyScriptDesc, "<Sound0>") != 0) bSound0Mentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<Sound1>") != 0) bSound1Mentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<Sound2>") != 0) bSound2Mentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<Sound3>") != 0) bSound3Mentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<Video Slot>") != 0) bVideoSlotMentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<If Used>") != 0) bIfUsedMentioned = true;
							if (strstr(pCaptureAnyScriptDesc, "<Use Key>") != 0) bUseKeyMentioned = true;		

							//LB: Better if it looks associated with the Behavior comnponent
							//if (ImGui::StyleCollapsingHeader("Media", ImGuiTreeNodeFlags_DefaultOpen))
							//{
							//	ImGui::Indent(10);
								if ( bVideoSlotMentioned == true ) t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), "Video Slot", t.strarr_s[601].Get(), "videobank\\");
								if ( bSound0Mentioned == true ) t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), "Sound0", t.strarr_s[254].Get(), "audiobank\\");
								if ( bSound1Mentioned == true ) t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), "Sound1", t.strarr_s[254].Get(), "audiobank\\");
								if ( bSound2Mentioned == true ) t.grideleprof.soundset2_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset2_s.Get(), "Sound2", t.strarr_s[254].Get(), "audiobank\\");
								if ( bSound3Mentioned == true ) t.grideleprof.soundset3_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset3_s.Get(), "Sound3", t.strarr_s[254].Get(), "audiobank\\");
								if ( bIfUsedMentioned == true ) t.grideleprof.ifused_s = imgui_setpropertystring2(t.group, t.grideleprof.ifused_s.Get(), t.strarr_s[437].Get(), t.strarr_s[226].Get());
								if ( bUseKeyMentioned == true ) t.grideleprof.usekey_s = imgui_setpropertystring2(t.group, t.grideleprof.usekey_s.Get(), t.strarr_s[436].Get(), t.strarr_s[225].Get());
								//	ImGui::Indent(-10);
							//}
						}

						// And finally the ADVANCED section
						bool bAdvencedOpen = false;
						if (g.vrqcontrolmode > 0) {
							if(bUnfoldAdvanced)
								bAdvencedOpen = true;
							else if (ImGui::StyleCollapsingHeader("Advanced", adv_flasgs)) { //ImGuiTreeNodeFlags_None //ImGuiTreeNodeFlags_DefaultOpen
								bAdvencedOpen = true;
							}
						}
						else 
						{
							bAdvencedOpen = true;
						}
						if (bAdvencedOpen) 
						{
							t.group = 0;
							if (ImGui::StyleCollapsingHeader(t.strarr_s[412].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {

								if (bUnfoldAdvanced) {
									//Display icon.
									if (t.entityprofile[iParentEntid].iThumbnailSmall > 0) {
										float w = ImGui::GetWindowContentRegionWidth();
										ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (media_icon_size*0.5), 0.0f));
										ImGui::ImgBtn(t.entityprofile[iParentEntid].iThumbnailSmall, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
									}
								}


								if (bUnfoldAdvanced)
								{
									if (t.entityprofile[t.gridentity].ischaracter > 0)
									{
										//478 missing.
										//t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), t.strarr_s[478].Get(), t.strarr_s[204].Get());
										t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
									}
									else
									{
										if (t.entityprofile[t.gridentity].ismarker > 0)
										{
											if (t.entityprofile[t.gridentity].islightmarker > 0) {
												//483 missing.
												//t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), t.strarr_s[483].Get(), t.strarr_s[204].Get());
												t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
											}
											else {
												//479 missing.
												//t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), t.strarr_s[479].Get(), t.strarr_s[204].Get());
												t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
											}
										}
										else {
											//t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), t.strarr_s[413].Get(), t.strarr_s[204].Get());
											t.grideleprof.name_s = imgui_setpropertystring2(t.group, t.grideleprof.name_s.Get(), "Name", t.strarr_s[204].Get());
										}
									}
								}

								if (t.entityprofile[t.gridentity].ismarker == 0 || t.entityprofile[t.gridentity].islightmarker == 1)
								{
									if (g.gentitytogglingoff == 0)
									{
										t.tokay = 1;
										if (ObjectExist(g.entitybankoffset + t.gridentity) == 1)
										{
											if (GetNumberOfFrames(g.entitybankoffset + t.gridentity) > 0)
											{
												t.tokay = 0;
											}
										}
										if (t.tokay == 1)
										{
											//PE: 414=Static Mode
											t.gridentitystaticmode = imgui_setpropertylist2(t.group, t.controlindex, Str(t.gridentitystaticmode), t.strarr_s[414].Get(), t.strarr_s[205].Get(), 0);
										}
									}
								}

								// 101016 - Additional General Parameters
								if (t.tflagchar == 0 && t.tflagvis == 1)
								{
									if (t.tflagsimpler == 0)
									{
										t.grideleprof.isocluder = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.isocluder), "Occluder", "Set to YES makes this object an occluder", 0); ++t.controlindex;
										t.grideleprof.isocludee = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.isocludee), "Occludee", "Set to YES makes this object an occludee", 0); ++t.controlindex;
									}
									// these will be back when EBE needs doors and windows
									//setpropertystring2(t.group,Str(t.grideleprof.parententityindex),"Parent Index","Selects another entity element to be a parent") ; ++t.controlindex;
									//setpropertystring2(t.group,Str(t.grideleprof.parentlimbindex),"Parent Limb","Specifies the limb index of the parent to connect with") ; ++t.controlindex;
								}

								// 281116 - added Specular Control per entity
								if (t.tflagvis == 1)
								{
									if (t.tflagsimpler == 0)
									{
										t.grideleprof.specularperc = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.specularperc), "Specular", "Set specular percentage to modulate entity specular effect"));
									}
								}

								if (ImGui::IsAnyItemFocused()) {
									bImGuiGotFocus = true;
								}
							}

							int speech_entries = 0;
							//Add DLUA here id
							if (bUnfoldAdvanced && t.entityprofile[t.gridentity].ismarker > 1) {
								//ismarker = 1 has its own function.
								//DLUA support added here.
								bool bUpdateMainString = false;
								for (int speech_loop = 0; speech_loop < 5; speech_loop++)
									speech_ids[speech_loop] = -1;
								if (current_loaded_script != current_selected_script) {
									//Load in lua and check for custom properties.
									cstr script_name = "scriptbank\\";
									script_name += t.grideleprof.aimain_s;
									//Try to parse script.
									ParseLuaScript(&t.grideleprof, script_name.Get());
									current_loaded_script = current_selected_script;

									if (t.grideleprof.PropertiesVariableActive == 1) {
										bUpdateMainString = true;
										current_loaded_script_has_dlua = true;
									}
									else {
										if (current_loaded_script_has_dlua) {
											//Reset t.grideleprof.soundset4_s that contain the dlua calls.
											t.grideleprof.soundset4_s = "";
											current_loaded_script_has_dlua = false;
										}
									}

								}

								if (t.grideleprof.PropertiesVariableActive == 1 || t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0)
								{
									if (ImGui::StyleCollapsingHeader("Behavior", ImGuiTreeNodeFlags_DefaultOpen)) {

										ImGui::Indent(10);

										if (t.grideleprof.PropertiesVariableActive == 1) {
											speech_entries = DisplayLuaDescription(&t.grideleprof);
										}
										else {
											if (t.grideleprof.PropertiesVariable.VariableDescription.Len() > 0) {
												DisplayLuaDescriptionOnly(&t.grideleprof);
											}
										}

										ImGui::Indent(-10);
									}
								}
								#ifdef VRTECH
								if (speech_entries > 0)
								{
									// all SPEECH control is moved to this function.
									SpeechControls(speech_entries, bUpdateMainString);
								}
								#endif
							}

							t.group = 1;
							if (ImGui::StyleCollapsingHeader(t.strarr_s[415].Get(), ImGuiTreeNodeFlags_DefaultOpen))
							{
								//  Basic AI
								if (t.tflagai == 1)
								{
									// can redirect to better folders if in g.quickparentalcontrolmode
									LPSTR pAIRoot = "scriptbank\\";
									if (g.quickparentalcontrolmode == 2)
									{
										if (t.entityprofile[t.gridentity].ismarker == 0)
										{
											if (t.tflagchar == 1)
												pAIRoot = "scriptbank\\people\\";
											else
												pAIRoot = "scriptbank\\objects\\";
										}
										else
										{
											pAIRoot = "scriptbank\\markers\\";
										}
									}
									cstr tmpvalue;
									tmpvalue = imgui_setpropertyfile2(t.group, t.grideleprof.aimain_s.Get(), t.strarr_s[417].Get(), t.strarr_s[207].Get(), pAIRoot);
									if (t.grideleprof.aimain_s != tmpvalue) 
									{
										t.grideleprof.aimain_s = tmpvalue;
										current_loaded_script = -1;
									}
								}


								//  Has Weapon
								if (t.tflaghasweapon == 1 && t.playercontrol.thirdperson.enabled == 0 && g.quickparentalcontrolmode != 2)
								{
									t.grideleprof.hasweapon_s = imgui_setpropertylist2c(t.group, t.controlindex, t.grideleprof.hasweapon_s.Get(), t.strarr_s[419].Get(), t.strarr_s[209].Get(), 1);
								}

								//  Is Weapon (FPGC - 280809 - filtered fpgcgenre=1 is shooter genre)
								if (t.tflagweap == 1 && g.fpgcgenre == 1)
								{
									t.grideleprof.damage = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.damage), t.strarr_s[420].Get(), t.strarr_s[210].Get()));
									t.grideleprof.accuracy = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.accuracy), t.strarr_s[421].Get(), "Increases the inaccuracy of conical distribution by 1/100th of t.a degree"));
									if (t.grideleprof.weaponisammo == 0)
									{
										t.grideleprof.reloadqty = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.reloadqty), t.strarr_s[422].Get(), t.strarr_s[212].Get()));
										t.grideleprof.fireiterations = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.fireiterations), t.strarr_s[423].Get(), t.strarr_s[213].Get()));
										t.grideleprof.range = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.range), "Range", "Maximum range of bullet travel"));
										t.grideleprof.dropoff = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.dropoff), "Dropoff", "Amount in inches of vertical dropoff per 100 feet of bullet travel"));
									}
									else
									{
										t.grideleprof.lifespan = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.lifespan), t.strarr_s[424].Get(), t.strarr_s[214].Get()));
										t.grideleprof.throwspeed = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.throwspeed), t.strarr_s[425].Get(), t.strarr_s[215].Get()));
										t.grideleprof.throwangle = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.throwangle), t.strarr_s[426].Get(), t.strarr_s[216].Get()));
										t.grideleprof.bounceqty = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.bounceqty), t.strarr_s[427].Get(), t.strarr_s[217].Get()));
										t.grideleprof.explodeonhit = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.explodeonhit), t.strarr_s[428].Get(), t.strarr_s[218].Get(), 0);
									}
									if (t.tflagsimpler == 0)
									{
										t.grideleprof.usespotlighting = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.usespotlighting), "Spot Lighting", "Set whether emits dynamic spot lighting", 0);
									}
								}

								//  Is Character
								if (t.tflagchar == 1)
								{
									if (t.tflagsimpler == 0)
									{

										// 020316 - special check to avoid offering can take weapon if no HUD.X
										t.tfile_s = cstr("gamecore\\guns\\") + t.grideleprof.hasweapon_s + cstr("\\HUD.X");
										if (FileExist(t.tfile_s.Get()) == 1)
										{
											t.grideleprof.cantakeweapon = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.cantakeweapon), t.strarr_s[429].Get(), t.strarr_s[219].Get(), 0);
											//Take Weapon's Ammo
											cstr fieldname = t.strarr_s[430];
											if (fieldname == "Take Weapon's Ammo") fieldname = "Take Weapon Ammo"; //Need to be shorter.
											t.grideleprof.quantity = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.quantity), fieldname.Get() , t.strarr_s[220].Get()));
										}
										t.grideleprof.rateoffire = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.rateoffire), t.strarr_s[431].Get(), t.strarr_s[221].Get()));
									}
								}
								if (t.tflagquantity == 1 && g.quickparentalcontrolmode != 2)
								{
									t.grideleprof.quantity = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.quantity), t.strarr_s[432].Get(), t.strarr_s[222].Get()));
								}

								//  AI Extra
								if (t.tflagvis == 1 && t.tflagai == 1)
								{
									if (t.tflagchar == 1)
									{
										t.grideleprof.coneangle = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.coneangle), t.strarr_s[434].Get(), t.strarr_s[224].Get()));
										t.grideleprof.conerange = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.conerange), "View Range", "The range within which the AI may see the player. Zero triggers the characters default range."));
										t.grideleprof.ifused_s = imgui_setpropertystring2(t.group, t.grideleprof.ifused_s.Get(), t.strarr_s[437].Get(), t.strarr_s[226].Get());
										if (g.quickparentalcontrolmode != 2)
											t.grideleprof.isviolent = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.isviolent), "Blood Effects", "Sets whether blood and screams should be used", 0);
										if (t.tflagsimpler == 0)
										{
											t.grideleprof.colondeath = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.colondeath), "End Collision", "Set to NO switches off collision when die", 0);
										}
									}
									else
									{
										if (t.tflagweap == 0 && t.tflagammo == 0)
										{
											//t.propfield[t.group] = t.controlindex;
											//++t.group; startgroup(t.strarr_s[435].Get()); t.controlindex = 0;
											t.grideleprof.usekey_s = imgui_setpropertystring2(t.group, t.grideleprof.usekey_s.Get(), t.strarr_s[436].Get(), t.strarr_s[225].Get());
											if (t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1)
											{
												// only one level - no winzone chain option
											}
											else
											{
												t.grideleprof.ifused_s = imgui_setpropertystring2(t.group, t.grideleprof.ifused_s.Get(), t.strarr_s[437].Get(), t.strarr_s[226].Get());
											}
										}
									}
								}
								if (t.tflagifused == 1)
								{
									if (t.tflagusekey == 1)
									{
										t.grideleprof.usekey_s = imgui_setpropertystring2(t.group, t.grideleprof.usekey_s.Get(), t.strarr_s[436].Get(), t.strarr_s[225].Get());
									}
									if (t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1)
									{
										// only one level - no winzone chain option
									}
									else
									{
										t.grideleprof.ifused_s = imgui_setpropertystring2(t.group, t.grideleprof.ifused_s.Get(), t.strarr_s[438].Get(), t.strarr_s[227].Get());
									}
								}

							}

							if (t.tflagspawn == 1)
							{
								t.group = 1;
								if (ImGui::StyleCollapsingHeader(t.strarr_s[439].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {

									t.grideleprof.spawnatstart = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.spawnatstart), t.strarr_s[562].Get(), t.strarr_s[563].Get(), 0);
									//     `setpropertystring2(group,Str(grideleprof.spawnmax),strarr$(440),strarr$(231)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnupto),strarr$(441),strarr$(232)) ; inc controlindex
									//     `setpropertylist2(group,controlindex,Str(grideleprof.spawnafterdelay),strarr$(442),strarr$(233),0) ; inc controlindex
									//     `setpropertylist2(group,controlindex,Str(grideleprof.spawnwhendead),strarr$(443),strarr$(234),0) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawndelay),strarr$(444),strarr$(235)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawndelayrandom),strarr$(564),strarr$(565)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnqty),strarr$(445),strarr$(236)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnqtyrandom),strarr$(566),strarr$(567)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnvel),strarr$(568),strarr$(569)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnvelrandom),strarr$(570),strarr$(571)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnangle),strarr$(572),strarr$(573)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnanglerandom),strarr$(574),strarr$(575)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.spawnlife),strarr$(576),strarr$(577)) ; inc controlindex

								}
							}


							//  Statistics
							if ((t.tflagvis == 1 || t.tflagobjective == 1 || t.tflaglives == 1 || t.tflagstats == 1) && t.tflagweap == 0 && t.tflagammo == 0)
							{
								t.group = 1;
								if (ImGui::StyleCollapsingHeader(t.strarr_s[451].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {

									#ifdef VRTECH
									#else
										if (t.tflaglives == 1)
										{
										t.grideleprof.lives = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.lives), t.strarr_s[452].Get(), t.strarr_s[242].Get()));
										}
										if (t.tflagvis == 1 || t.tflagstats == 1)
										{
										if (t.tflaglives == 1)
										{
											t.grideleprof.strength = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.strength), t.strarr_s[453].Get(), t.strarr_s[243].Get()));
										}
										else
										{
											if (t.tflagnotionofhealth == 1)
											{
												t.grideleprof.strength = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.strength), t.strarr_s[454].Get(), t.strarr_s[244].Get()));
											}
										}
										if (t.tflagplayersettings == 1)
										{
											if (g.quickparentalcontrolmode != 2)
											{
												t.grideleprof.isviolent = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.isviolent), "Blood Effects", "Sets whether blood and screams should be used", 0);
											}
											if (t.tflagnotionofhealth == 1)
											{
												t.playercontrol.regenrate = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.regenrate), "Regeneration Rate", "Sets the increase value at which the players health will restore"));
												t.playercontrol.regenspeed = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.regenspeed), "Regeneration Speed", "Sets the speed in milliseconds at which the players health will regenerate"));
												t.playercontrol.regendelay = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.regendelay), "Regeneration Delay", "Sets the delay in milliseconds after last damage hit before health starts regenerating"));
											}
										}
										t.grideleprof.speed = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.speed), t.strarr_s[455].Get(), t.strarr_s[245].Get()));
										if (t.playercontrol.thirdperson.enabled == 1)
										{
											t.tanimspeed_f = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed;
										}
										else
										{
											t.tanimspeed_f = t.grideleprof.animspeed;
										}
										//477 missing.
										//t.tanimspeed_f = atof(imgui_setpropertystring2(t.group, Str(t.tanimspeed_f), t.strarr_s[477].Get(), "Sets the default speed of any animation associated with this entity"));
										t.tanimspeed_f = atof(imgui_setpropertystring2(t.group, Str(t.tanimspeed_f), "Anim Speed", "Sets the default speed of any animation associated with this entity"));

										if (t.playercontrol.thirdperson.enabled == 1)
										{
											t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed = t.tanimspeed_f;
										}
										else
										{
											t.grideleprof.animspeed = t.tanimspeed_f;
										}

										}
										if (t.tflaghurtfall == 1) {
										t.grideleprof.hurtfall = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.hurtfall), t.strarr_s[456].Get(), t.strarr_s[246].Get()));
										}
									#endif
									if (t.tflagplayersettings == 1)
									{
										t.playercontrol.jumpmax_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.jumpmax_f), "Jump Speed", "Sets the jump speed of the player which controls overall jump height"));
										t.playercontrol.gravity_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.gravity_f), "Gravity", "Sets the modified force percentage of the players own gravity"));
										t.playercontrol.fallspeed_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.fallspeed_f), "Fall Speed", "Sets the maximum speed percentage at which the player will fall"));
										t.playercontrol.climbangle_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.climbangle_f), "Climb Angle", "Sets the maximum angle permitted for the player to ascend a slope"));
										if (t.playercontrol.thirdperson.enabled == 0)
										{
											t.playercontrol.wobblespeed_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.wobblespeed_f), "Wobble Speed", "Sets the rate of motion applied to the camera when moving"));
											t.playercontrol.wobbleheight_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.wobbleheight_f * 100), "Wobble Height", "Sets the degree of motion applied to the camera when moving")) / 100.0f;
											t.playercontrol.footfallpace_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.footfallpace_f * 100), "Footfall Pace", "Sets the rate at which the footfall sound is played when moving")) / 100.0f;
										}
										t.playercontrol.accel_f = atof(imgui_setpropertystring2(t.group, Str(t.playercontrol.accel_f * 100), "Acceleration", "Sets the acceleration curve used when t.moving from t.a stood position")) / 100.0f;
									}
									if (t.tflagmobile == 1) 
									{
										t.grideleprof.isimmobile = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.isimmobile), t.strarr_s[457].Get(), t.strarr_s[247].Get(), 0);
									}
									if (t.tflagmobile == 1)
									{
										if (t.tflagsimpler == 0)
										{
											t.grideleprof.lodmodifier = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.lodmodifier), "LOD Modifier", "Modify when the LOD transition takes effect. The default value is 0, increase this to a percentage reduce the LOD effect."));
										}
									}
								}
							}

							//  Team field
#ifdef PHOTONMP
#else
							if (t.tflagteamfield == 1)
							{
								//							setpropertylist3(t.group, t.controlindex, Str(t.grideleprof.teamfield), "Team", "Specifies any team affiliation for multiplayer start marker", 0); ++t.controlindex;
							}
#endif



							//  Physics Data (non-multiplayer)
							if (t.entityprofile[t.gridentity].ismarker == 0 && t.entityprofile[t.gridentity].islightmarker == 0)
							{
								//t.propfield[t.group] = t.controlindex;
								//++t.group; startgroup(t.strarr_s[596].Get()); t.controlindex = 0;
								t.group = 1;
								if (ImGui::StyleCollapsingHeader(t.strarr_s[596].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {

									if (t.grideleprof.physics != 1)  t.grideleprof.physics = 0;
									t.grideleprof.physics = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.physics), t.strarr_s[580].Get(), t.strarr_s[581].Get(), 0);
									t.grideleprof.phyalways = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.phyalways), t.strarr_s[582].Get(), t.strarr_s[583].Get(), 0);
									t.grideleprof.phyweight = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.phyweight), t.strarr_s[584].Get(), t.strarr_s[585].Get()));
									t.grideleprof.phyfriction = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.phyfriction), t.strarr_s[586].Get(), t.strarr_s[587].Get()));
									//     `setpropertystring2(group,Str(grideleprof.phyforcedamage),strarr$(588),strarr$(589)) ; inc controlindex
									//     `setpropertystring2(group,Str(grideleprof.rotatethrow),strarr$(590),strarr$(591)) ; inc controlindex
									if (t.tflagsimpler == 0)
									{
										t.grideleprof.explodable = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.explodable), t.strarr_s[592].Get(), t.strarr_s[593].Get(), 0);
										t.grideleprof.explodedamage = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.explodedamage), t.strarr_s[594].Get(), t.strarr_s[595].Get()));
									}
								}
							}

							//  Ammo data (FPGC - 280809 - filtered fpgcgenre=1 is shooter genre
							if (g.fpgcgenre == 1)
							{
								if (t.tflagammo == 1 || t.tflagammoclip == 1)
								{
									if (ImGui::StyleCollapsingHeader(t.strarr_s[459].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {
										t.grideleprof.quantity = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.quantity), t.strarr_s[460].Get(), t.strarr_s[249].Get()));
									}

								}
							}

							//  Light data
							if (t.tflaglight == 1)
							{
								if (ImGui::StyleCollapsingHeader(t.strarr_s[461].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {
									t.grideleprof.light.range = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.light.range), t.strarr_s[462].Get(), t.strarr_s[250].Get())); //PE: 462=Light Range
										
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
									ImGui::Text(t.strarr_s[463].Get());
									ImGui::SameLine();
									ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
									ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
									ImGui::PushItemWidth(-10);

									float colors[5];
									colors[3] = ((t.grideleprof.light.color & 0xff000000) >> 24) / 255.0f;
									colors[0] = ((t.grideleprof.light.color & 0x00ff0000) >> 16) / 255.0f;
									colors[1] = ((t.grideleprof.light.color & 0x0000ff00) >> 8) / 255.0f;
									colors[2] = (t.grideleprof.light.color & 0x000000ff) / 255.0f;
									ImGui::ColorEdit3("##LightColorSetupField", colors, 0);
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", t.strarr_s[251].Get());
									colors[0] *= 255.0f;
									colors[1] *= 255.0f;
									colors[2] *= 255.0f;
									colors[3] *= 255.0f;
									t.grideleprof.light.color = 0xff000000 + ((unsigned int)colors[0] << 16) + ((unsigned int)colors[1] << 8) + +((unsigned int)colors[2]);

									ImGui::PopItemWidth();

									//setpropertycolor2(t.group, t.grideleprof.light.color, t.strarr_s[463].Get(), t.strarr_s[251].Get()); ++t.controlindex; //PE: 463=Light Color
									if (t.tflagsimpler == 0)
									{
										t.grideleprof.usespotlighting = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.usespotlighting), "Spot Lighting", "Change dynamic light to spot lighting", 0);
									}
								}
							}

							//  Decal data
							if (t.tflagtdecal == 1)
							{
								t.propfield[t.group] = t.controlindex;

								//  FPGC - 300710 - could never change base decal, so comment out this property (entity denotes decal choice)
								//     `inc group ; startgroup(strarr$(464)) ; controlindex=0
								//     `setpropertyfile2(group,grideleprof.basedecal$,strarr$(465),strarr$(252),"gamecore\\decals\\") ; inc controlindex

								//  Decal Particle data
								if (t.tflagdecalparticle == 1)
								{
									//++t.group; startgroup("Decal Particle"); t.controlindex = 0;
									if (ImGui::StyleCollapsingHeader("Decal Particle", ImGuiTreeNodeFlags_DefaultOpen)) {

										t.grideleprof.particleoverride = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.particleoverride), "Custom Settings", "Whether you wish to override default settings", 0);
										t.grideleprof.particle.offsety = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.offsety), "OffsetY", "Vertical adjustment of start position"));
										t.grideleprof.particle.scale = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.scale), "Scale", "A value from 0 to 100, denoting size of particle"));
										t.grideleprof.particle.randomstartx = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randomstartx), "Random Start X", "Random start area"));
										t.grideleprof.particle.randomstarty = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randomstarty), "Random Start Y", "Random start area"));
										t.grideleprof.particle.randomstartz = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randomstartz), "Random Start Z", "Random start area"));
										t.grideleprof.particle.linearmotionx = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.linearmotionx), "Linear Motion X", "Constant motion direction"));
										t.grideleprof.particle.linearmotiony = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.linearmotiony), "Linear Motion Y", "Constant motion direction"));
										t.grideleprof.particle.linearmotionz = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.linearmotionz), "Linear Motion Z", "Constant motion direction"));
										t.grideleprof.particle.randommotionx = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randommotionx), "Random Motion X", "Random motion direction"));
										t.grideleprof.particle.randommotiony = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randommotiony), "Random Motion Y", "Random motion direction"));
										t.grideleprof.particle.randommotionz = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.randommotionz), "Random Motion Z", "Random motion direction"));
										t.grideleprof.particle.mirrormode = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.mirrormode), "Mirror Mode", "Set to one to reverse the particle"));
										t.grideleprof.particle.camerazshift = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.camerazshift), "Camera Z Shift", "Shift t.particle towards camera"));
										t.grideleprof.particle.scaleonlyx = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.scaleonlyx), "Scale Only X", "Percentage X over Y scale"));
										t.grideleprof.particle.lifeincrement = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.lifeincrement), "Life Increment", "Control lifespan of particle"));
										t.grideleprof.particle.alphaintensity = atol(imgui_setpropertystring2(t.group, Str(t.grideleprof.particle.alphaintensity), "Alpha Intensity", "Control alpha percentage of particle"));
										//  V118 - 060810 - knxrb - Decal animation setting (Added animation choice setting).
										t.grideleprof.particle.animated = imgui_setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.particle.animated), "Animated Particle", "Sets whether the t.particle t.decal Texture is animated or static.", 0);
									}
								}
							}

							// moved Particle to main

							// Sound
							if (t.tflagsound == 1 || t.tflagsoundset == 1 || tflagtext == 1 || tflagimage == 1)
							{
								cstr group_text;
								if (tflagtext == 1 || tflagimage == 1)
								{
									if (tflagtext == 1) group_text = "Text";
									if (tflagimage == 1) group_text = "Image";
								}
								else
								{
									group_text = "Media";
								}
									
								if (speech_entries > 0)
								{
								}

								if (ImGui::StyleCollapsingHeader(group_text.Get(), ImGuiTreeNodeFlags_DefaultOpen))
								{
									if (g.fpgcgenre == 1)
									{
										if (t.entityprofile[t.gridentity].ischaracter > 0) 
										{
											t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), "Sound0", t.strarr_s[254].Get(), "audiobank\\");
										}
										else 
										{
											if (g.vrqcontrolmode != 0)
											{
												if (t.tflagsound == 1 && t.tflagsoundset != 1) 
												{
													//PE: changed from 469 to 467 , should be sound0
													t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[467].Get(), t.strarr_s[253].Get(), "audiobank\\");
												}
											}
											else
											{
												if (t.tflagsound == 1 && t.tflagsoundset != 1) 
												{
													t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[467].Get(), t.strarr_s[253].Get(), "audiobank\\");
												}
											}
											if (t.tflagsoundset == 1) 
											{
												t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[469].Get(), t.strarr_s[255].Get(), "audiobank\\voices\\");
											}
											if (tflagtext == 1) 
											{
												t.grideleprof.soundset_s = imgui_setpropertystring2(t.group, t.grideleprof.soundset_s.Get(), "Text to Appear", "Enter text to appear in-game");
											}
											if (tflagimage == 1) 
											{
												t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), "Image File", "Select image to appear in-game", "scriptbank\\images\\imagesinzone\\");
											}
										}

										if (t.tflagnosecond == 0)
										{
											if (t.tflagsound == 1 || t.tflagsoundset == 1)
											{
												//We got some missing translations.
												if (t.strarr_s[468] == "") t.strarr_s[468] = "Sound1";
												if (t.strarr_s[480] == "") t.strarr_s[480] = "Sound2";
												if (t.strarr_s[481] == "") t.strarr_s[481] = "Sound3";
												if (t.strarr_s[482] == "") t.strarr_s[482] = "Sound4";
												t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), t.strarr_s[468].Get(), t.strarr_s[254].Get(), "audiobank\\");
												t.grideleprof.soundset2_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset2_s.Get(), t.strarr_s[480].Get(), t.strarr_s[254].Get(), "audiobank\\");
												t.grideleprof.soundset3_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset3_s.Get(), t.strarr_s[481].Get(), t.strarr_s[254].Get(), "audiobank\\");
												if (t.grideleprof.PropertiesVariableActive != 1) 
												{
													t.grideleprof.soundset4_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset4_s.Get(), t.strarr_s[482].Get(), t.strarr_s[254].Get(), "audiobank\\");
												}
											}
										}
									}
									else
									{
										if (t.tflagsoundset == 1)
										{
											t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[469].Get(), t.strarr_s[255].Get(), "audiobank\\voices\\");
										}
										else
										{
											t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[467].Get(), t.strarr_s[253].Get(), "audiobank\\"); ++t.controlindex;
										}
										t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), t.strarr_s[468].Get(), t.strarr_s[254].Get(), "audiobank\\"); ++t.controlindex;
									}
								}
							}

							// Video
							if (t.tflagvideo == 1)
							{
								if (ImGui::StyleCollapsingHeader(t.strarr_s[597].Get(), ImGuiTreeNodeFlags_DefaultOpen)) {

									//t.grideleprof.soundset_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset_s.Get(), t.strarr_s[467].Get(), t.strarr_s[599].Get(), "audiobank\\");
									t.grideleprof.soundset1_s = imgui_setpropertyfile2(t.group, t.grideleprof.soundset1_s.Get(), "Video Slot", t.strarr_s[601].Get(), "videobank\\");
								}
							}

							//  Third person settings
							if (t.tflagplayersettings == 1 && t.playercontrol.thirdperson.enabled == 1)
							{
								if (ImGui::StyleCollapsingHeader("Third Person", ImGuiTreeNodeFlags_DefaultOpen)) {

									t.livegroupforthirdperson = t.group;
									t.playercontrol.thirdperson.cameralocked = imgui_setpropertylist2(t.group, t.controlindex, Str(t.playercontrol.thirdperson.cameralocked), "Camera Locked", "Fixes camera height and angle for third person view", 0);
									t.playercontrol.thirdperson.cameradistance = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.thirdperson.cameradistance), "Camera Distance", "Sets the distance of the third person camera"));
									t.playercontrol.thirdperson.camerashoulder = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.thirdperson.camerashoulder), "Camera X Offset", "Sets the distance to shift the camera over shoulder"));
									t.playercontrol.thirdperson.cameraheight = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.thirdperson.cameraheight), "Camera Y Offset", "Sets the vertical height of the third person camera. If more than twice the camera distance, camera collision disables"));
									t.playercontrol.thirdperson.camerafocus = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.thirdperson.camerafocus), "Camera Focus", "Sets the camera X angle offset to align focus of the third person camera"));
									t.playercontrol.thirdperson.cameraspeed = atol(imgui_setpropertystring2(t.group, Str(t.playercontrol.thirdperson.cameraspeed), "Camera Speed", "Sets the retraction speed percentage of the third person camera"));
									t.playercontrol.thirdperson.camerafollow = imgui_setpropertylist2(t.group, t.controlindex, Str(t.playercontrol.thirdperson.camerafollow), "Run Mode", "If set to yes, protagonist uses WASD t.movement mode", 0);
									t.playercontrol.thirdperson.camerareticle = imgui_setpropertylist2(t.group, t.controlindex, Str(t.playercontrol.thirdperson.camerareticle), "Show Reticle", "Show the third person 'crosshair' reticle Dot ( ", 0);
								}
							}

						}
					} //Advenced open


					if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
						//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
						ImGui::Text("");
						ImGui::Text("");
					}


					ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
					if (ImGui::IsMouseHoveringRect(bbwin.Min, bbwin.Max))
					{
						bImGuiGotFocus = true;
					}
					if (ImGui::IsAnyItemFocused()) {
						bImGuiGotFocus = true;
					}
				}
//					ImGui::Text("iParentEntid: %ld pEIndex: %ld", iParentEntid, t.widget.pickedEntityIndex );
				ImGui::PopItemWidth();
				ImGui::SetWindowFontScale(1.0);

				ImGui::EndChild();

				CheckMinimumDockSpaceSize(250.0f);

				ImGui::End();

				if(!bEntity_Properties_Window) //Window closed.
					iOldPickedEntityIndex = -1;

				if (t.inputsys.mclick == 1 && ImGui::IsWindowHovered()) {
					//Click start , block until mouse is release.
					bProperties_Window_Block_Mouse = true;
				}

			}
			else {
				iOldPickedEntityIndex = -1;
			}
		}
		else 
		{
			//PRoperties closed , check if we need to exit zoommode.
			if (t.gridentityinzoomview > 0) 
			{
				t.tpressedtoleavezoommode = 2; //Exit zoom and save.
				if (t.grideditselect < 3 || t.grideditselect > 4) 
				{
					//Make sure to exit fast.
					int igridentity = t.gridentity;

					if( iOldgridentity != t.gridentity && iOldgridentity > -1)
						t.gridentity = iOldgridentity;

					int olges = t.grideditselect;
					t.grideditselect = 4;
					editor_viewfunctionality();
					t.grideditselect = olges;
					t.gridentity = igridentity;
				}
			}

			//PE: Bug if open properties, close and delete object, then add object and properties, failed and use old id.
			iOldPickedEntityIndex = -1;
		}


		//####################################
		//#### Procedural Level Generator ####
		//####################################

		#ifdef PROCEDURALTERRAINWINDOW
		//No resetting needed fixed.
		void procedural_new_level(void);
		procedural_new_level();
		#endif

		//###########################
		//#### External Entities ####
		//###########################

		static std::map<std::string, std::int32_t> entity_folders;
		if (refresh_gui_docking == 0) ImGui::SetNextWindowPos(viewPortPos + ImVec2(180, 140), ImGuiCond_Always); //ImGuiCond_FirstUseEver
		if (refresh_gui_docking == 0) ImGui::SetNextWindowSize(ImVec2(54 * ImGui::GetFontSize(), 33 * ImGui::GetFontSize()), ImGuiCond_Always); //ImGuiCond_FirstUseEver
		ImGuiWindowFlags ex_window_flags = 0;
		if (refresh_gui_docking == 0 && !bExternal_Entities_Window)
		{
			ImGui::Begin("##Object Library ExternalWindow", &bExternal_Entities_Window, ex_window_flags);
			ImGui::End();
		}

		process_entity_library();
		
		static char cFullWritePath[MAX_PATH];

		if (!bExternal_Entities_Init) 
		{
			//First get those in document folder.
			strcpy(cFullWritePath, "entitybank");
			GG_GetRealPath(cFullWritePath, 1);
			LPSTR pOld = GetDir();
			cStr CurrentPath = cStr(pOld)+ cStr("\\entitybank");
			bool bSkipDocWriteFolder = false;
			if (strnicmp(CurrentPath.Get(), cFullWritePath, CurrentPath.Len()) == 0) {
				//Same Dirs.
				bSkipDocWriteFolder = true;
			}

			if (!bSkipDocWriteFolder)
			{
				GetMainEntityList(cFullWritePath, "", NULL, "w:", true , 0 );
				SetDir(pOld);

				cFolderItem *pLastFolder = &MainEntityList;
				while (pLastFolder->m_pNext)
				{
					pLastFolder = pLastFolder->m_pNext;
				}

				GetMainEntityList("entitybank", "", pLastFolder, "", false , 0);
			}
			else {
				GetMainEntityList("entitybank", "", NULL, "", true , 0);
			}
			SetDir(pOld);

			//PE: Append other files types.

			SetDir(pOld);
			bExternal_Entities_Init = true;

			//Sort folder entrys.
			cFolderItem *pNewFolder = (cFolderItem *)&MainEntityList;
			cFolderItem *m_pfirstFolder = NULL;
			int mc = 0;
			while (pNewFolder->m_pNext) 
			{
				if (!m_pfirstFolder)
					m_pfirstFolder = pNewFolder->m_pNext;
				pNewFolder = pNewFolder->m_pNext;
				mc++;
			}
			if (mc > 1) 
			{		
				//#### SORT ####
				char ** cptr = new char *[mc + 1];
				cFolderItem *m_pSortFolder = m_pfirstFolder->m_pNext;
				int mc2 = 0;
				for (int a = 0; a < mc; a++) {
					if (m_pSortFolder) {
						cptr[a] = (char *)m_pSortFolder;
						m_pSortFolder = m_pSortFolder->m_pNext;
						mc2++;
					}
				}

				qsort(cptr, mc2, sizeof(cptr[0]), cstring_cmp_folder);

				m_pSortFolder = m_pfirstFolder->m_pNext;
				m_pfirstFolder->m_pNext = (cFolderItem *)cptr[0];
				for (int a = 0; a < mc2; a++) {
					m_pSortFolder = (cFolderItem *)cptr[a];
					if (m_pSortFolder) {
						if (a + 1 < mc2) m_pSortFolder->m_pNext = (cFolderItem *)cptr[a + 1];
					}
				}
				delete[] cptr;
				if (m_pSortFolder) m_pSortFolder->m_pNext = NULL;
			}

		}

		//########################
		//#### Level Entities ####
		//########################


		if (refresh_gui_docking == 0) 
		{
			bool bOpen = true;
			ImGui::Begin(TABENTITYNAME, &bOpen, iGenralWindowsFlags);
			ImGui::End();
			ImGui::Begin("Current Objects##AdditionalIconsWindow", &bOpen, iGenralWindowsFlags);
			ImGui::End();
		}
		if (refresh_gui_docking > 0) 
		{
			static int current_sort_order = 0;
			bool bToolTipActive = true;

			int iWinFlags = 0;
			bool bAlwaysOpen = true;
			ImGui::Begin(TABENTITYNAME, &bAlwaysOpen, iGenralWindowsFlags | ImGuiWindowFlags_NoScrollbar); //, &bAlwaysOpen, iWinFlags);
			static char cSearchEntities[1024] = "\0";

			ImGui::BeginChild("##ChirlEntitiesLeftPanel", ImVec2(ImGui::GetWindowSize().x - 2.0f, fsy*3.0), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavInputs); //ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar

			ImGui::PushItemWidth(-1);

			CheckTutorialAction("+##+", 8.0f); //Tutorial: check if we are waiting for this action

			if (ImGui::StyleButton("+##+", ImVec2(ImGui::GetWindowSize().x *0.5, fsy*1.5)))
			{
				if ( 1 )
				{
					if (bTutorialCheckAction) TutorialNextAction(); //Clicked get next tutorial action.

					//Open Add item page.
					//Clear any selection marks.
					cFolderItem *pSearchFolder = &MainEntityList;
					pSearchFolder = pSearchFolder->m_pNext;
					while (pSearchFolder) {
						if (pSearchFolder->m_pFirstFile) {
							cFolderItem::sFolderFiles * searchfiles = pSearchFolder->m_pFirstFile->m_pNext;
							while (searchfiles) {
								searchfiles->iFlags = 0;
								searchfiles = searchfiles->m_pNext;
							}
						}
						pSearchFolder = pSearchFolder->m_pNext;
					}

					bExternal_Entities_Window = true;
					iDisplayLibraryType = 0;
				}
			}
			if (bToolTipActive && ImGui::IsItemHovered()) ImGui::SetTooltip("Click here to add a new object to the game level (This feature is not yet complete)");

		
			ImGui::SameLine();
			ImGui::PopItemWidth();

			//	Display info icon to give user more information on adding objects to the level.
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(-6.0, 0));
			if (ImGui::ImgBtn(ICON_INFO, ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()), ImColor(0, 0, 0, 0), ImColor(220, 220, 220, 220), ImColor(255, 255, 255, 255),
				ImColor(180, 180, 160, 255), -1, 0, 0, 0, false, false, false, false, false))
			{
				bInfo_Window = true;
				cInfoMessage = "By pressing the 'Add' button, you can browse the object library for whatever you would like to add to your game. \nYou also have the option to download more objects, import your own models or even create your own character.";
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click here to learn how to add objects to your level.");

			ImGui::PushItemWidth(-1);

			//	ImGui::Begin("Information##AddObjectInformation", &bInformationWindow);
		
				////	Display the tutorial on adding objects to the world (not yet implemented).
				//if (GetImageExistEx(INFOIMAGE))
				//{
				//	float fRegionWidth = ImGui::GetContentRegionAvailWidth();
				//	float img_w = ImageWidth(INFOIMAGE);
				//	float img_h = ImageHeight(INFOIMAGE);
				//	float fRatio = img_h / img_w;
				//	ImGui::ImgBtn(INFOIMAGE, ImVec2(fRegionWidth, fRegionWidth*fRatio), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0),
				//		ImVec4(1.0, 1.0, 1.0, 1.0), 0, 0, 0, 0, false);
				//}
				
				//ImGui::End();

			
			
			
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(-6.0, 0));

			float fDropDownWidth = 0.0f;
			ImGui::PushItemWidth(-6 - fDropDownWidth);
			window = ImGui::GetCurrentWindow();

			ImVec2 current_pos = ImGui::GetCursorPos();
			float current_item_width = ImGui::GetItemRectSize().x;

			#ifdef V2SEARCHBAR
			//ImGuiStyle& style = ImGui::GetStyle();
			float fOldSpacing = style.FramePadding.x;
			style.FramePadding.x = 22.0; //Make room for search icon.
			ImVec2 vSearchPos = ImGui::GetCursorPos();
			#endif

			ImVec2 search_icon_pos = window->DC.CursorPos + ImVec2(ImGui::GetContentRegionAvailWidth() - 10.0 - 16.0 - fDropDownWidth,2.0f);
			ImGui::SetItemAllowOverlap();
			if (ImGui::InputText(" ##cSearchEntities", &cSearchEntities[0], MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (strlen(cSearchEntities) > 1)
				{
					bool already_there = false;
					for (int l = 0; l < MAXSEARCHHISTORY; l++) {
						if (strcmp(cSearchEntities, pref.small_search_history[l]) == 0) {
							already_there = true;
							break;
						}
					}
					if (!already_there) {
						bool foundspot = false;
						for (int l = 0; l < MAXSEARCHHISTORY; l++) {
							if (strlen(pref.small_search_history[l]) <= 0) {
								strcpy(pref.small_search_history[l], cSearchEntities);
								foundspot = true;
								break;
							}
						}
						if (!foundspot) {
							//Move entry list.
							for (int l = 0; l < MAXSEARCHHISTORY; l++) {
								strcpy(pref.small_search_history[l], pref.small_search_history[l + 1]);
							}
							strcpy(pref.small_search_history[MAXSEARCHHISTORY - 1], cSearchEntities);
						}
					}
				}

			}
			if (!pref.iTurnOffEditboxTooltip && ImGui::IsItemHovered() && bToolTipActive ) ImGui::SetTooltip("%s", "Search: Press Enter to Save Search String");
			if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;

			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImVec2 restore_pos = ImGui::GetCursorPos();
			current_pos.y = ImGui::GetCursorPosY();

			#ifdef V2SEARCHBAR
			style.FramePadding.x = fOldSpacing;
			//Only display closebut if we have room.
			if (restore_pos.x > 110 ) // ? not sure if we only activate after search begin ? && strlen(cSearchEntities) > 0
			{
				ImGui::SetItemAllowOverlap();
				if (ImGui::CloseButton(ImGui::GetCurrentWindow()->GetID("#ClearSearchv2"), ImGui::GetWindowPos() + ImGui::GetCursorPos() + ImVec2(-38, 0)))
				{
					strcpy(cSearchEntities, "");
				}
				ImGui::SameLine();
			}
			#endif

			static bool bSearchWinToggle = false;
			bool search_img_hovered = false, search_img_held = false;
			ID3D11ShaderResourceView* lpTexture = GetImagePointerView(TOOL_ENT_SEARCH);
			if (lpTexture)
			{
				ImGui::SetItemAllowOverlap();
				#ifdef V2SEARCHBAR
				ImVec2 search_icon_pos = ImGui::GetWindowPos() + vSearchPos + ImVec2(3.0, 3.0);
				#endif
				ImRect bb(search_icon_pos, search_icon_pos + ImVec2(16, 16));
				ImGui::PushID(TOOL_ENT_SEARCH);
				const ImGuiID id = window->GetID("#image");
				ImGui::PopID();
				ImGui::ItemSize(bb);
				if (ImGui::ItemAdd(bb, id)) {
					bool pressed = ImGui::ButtonBehavior(bb, id, &search_img_hovered, &search_img_held);
					if (pressed) {
						bSearchWinToggle = 1 - bSearchWinToggle;
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
					}
					ImGuiWindow* window = ImGui::GetCurrentWindow();
					window->DrawList->AddImage((ImTextureID)lpTexture, search_icon_pos, search_icon_pos + ImVec2(16, 16), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0, 1.0, 1.0, 1.0)));
				}
			}

			if (bSearchWinToggle)
			{
				//Do we have a search history.
				bool display_history = true; //false;
				//for (int l = 0; l < MAXSEARCHHISTORY; l++) {
				//	if (strlen(pref.small_search_history[l]) > 0) {
				//		display_history = true;
				//		break;
				//	}
				//}
				if (display_history)
				{
					ImGui::SameLine();
					//ImGui::SetCursorPos(current_pos);
					
					ImGui::SetNextWindowPos(ImGui::GetWindowPos() + current_pos);
					//ImGui::SetNextWindowSize({ current_item_width, 0 });
					if (ImGui::Begin("##searchselectpopup", &bSearchWinToggle, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Tooltip )) //| ImGuiWindowFlags_Tooltip
					{
						#ifndef V2SEARCHBAR
						if( ImGui::StyleButton("Clear Search Field",ImVec2(ImGui::GetContentRegionAvailWidth(),0.0f) ) )
						{
							strcpy(cSearchEntities, "");
							bSearchWinToggle = false;
						}
						ImGui::Separator();
						#endif

						//ImGui::Indent(10);
						for (int l = 0; l < MAXSEARCHHISTORY; l++) {
							if (strlen(pref.small_search_history[l]) > 0) {
								bool is_selected = false;
								if (ImGui::Selectable(pref.small_search_history[l], is_selected)) {
									strcpy(cSearchEntities, pref.small_search_history[l]);
									bSearchWinToggle = false;
								}
								//if (is_selected)
								//	ImGui::SetItemDefaultFocus();
							}
						}
						//ImGui::Indent(-10);
					}
					ImGui::End();
				}
			}

			ImGui::SetCursorPos(current_pos);

			if (!search_img_hovered)
			{
				if (ImGui::IsMouseReleased(0)) //ImGui::IsAnyMouseDown())
				{
					bSearchWinToggle = false;
				}
			}

			//if (strlen(cSearchEntities) > 0) {
			//}

			ImGui::EndChild();

			ImVec2 content_avail = ImVec2(0.0, 0.0);
			ImGui::BeginChild("##MainEntitiesLeftPanel", content_avail , false, iGenralWindowsFlags); //, false, ImGuiWindowFlags_HorizontalScrollbar);


			//static std::map<std::string, std::int32_t> sorted_files;
			static std::vector<std::pair<std::string,int>> sorted_entity_files;

			static int last_entidmaster = 0;
			static int last_include_icon_set = -1;

			int iMasterEntid = g.entidmaster;
			if (last_entidmaster != iMasterEntid)
			{
				//Sort new list.
				sorted_entity_files.clear();
				if (iMasterEntid >= 1)
				{
					//Sort list.
					for (t.entid = 1; t.entid <= iMasterEntid; t.entid++)
					{
						//std::string stmp = t.entityprofile[t.entid].model_s.Get();
						std::string stmp = Lower(t.entityprofileheader[t.entid].desc_s.Get());
						stmp += "###"; //We need it to be unique so add this.
						stmp += t.entityprofile[t.entid].model_s.Get();
						stmp += "###";
						stmp += std::to_string(t.entid);
						int itmp = t.entid;
						sorted_entity_files.push_back(std::make_pair(stmp, itmp));
					}
					std::sort(sorted_entity_files.begin(), sorted_entity_files.end());
				}
				//PE: Add seperate window instead of this, keep it here as its usefull.
				if (0) 
				{
					//PE: Add additional files here. , include commands like seperator , text ...
					//std::string sFind = "ZZZZ-Seperator";
					//sorted_entity_files.insert(std::make_pair(sFind, 999999));
					//sFind = "ZZZZ-This is a text";
					//sorted_entity_files.insert(std::make_pair(sFind, 999998));
					//int val = 999999;
					//auto mFind = std::find_if(sorted_entity_files.begin(), sorted_entity_files.end(), [val](const auto& mo) {return mo.second == val; });
					//if (mFind != sorted_entity_files.end())
					//{
					//	//Found.
					//}
				}
				last_entidmaster = iMasterEntid;
			}

			int uniqueId = 15000;

			int preview_count = 0;
			media_icon_size_leftpanel = 64;
			iColumnsWidth_leftpanel = 110;
			iColumns_leftpanel = 0;
			bDisplayText_leftpanel = true;
			fFontSize_leftpanel = SMALLFONTSIZE;
			ImGui::SetWindowFontScale(fFontSize_leftpanel);
			float fWinWidth = ImGui::GetWindowSize().x - 10.0; // Flicker - ImGui::GetCurrentWindow()->ScrollbarSizes.x;
			if (iColumnsWidth_leftpanel >= fWinWidth && fWinWidth > media_icon_size_leftpanel) {
				iColumnsWidth_leftpanel = fWinWidth;
				fFontSize_leftpanel = SMALLESTFONTSIZE;
				ImGui::SetWindowFontScale(fFontSize_leftpanel);
			}
			if (fWinWidth <= media_icon_size_leftpanel + 10) {
				iColumnsWidth_leftpanel = media_icon_size_leftpanel;
				fFontSize_leftpanel = SMALLESTFONTSIZE;
				ImGui::SetWindowFontScale(fFontSize_leftpanel);
			}
			if (fWinWidth <= 42) {
				media_icon_size_leftpanel = 32;
				iColumnsWidth_leftpanel = media_icon_size_leftpanel + 16;
				bDisplayText_leftpanel = false;
			}
			iColumns_leftpanel = (int)(ImGui::GetWindowSize().x / (iColumnsWidth_leftpanel));
			if (iColumns_leftpanel <= 1)
				iColumns_leftpanel = 1;

			#ifdef ADD_DETAIL_LEFT_PANEL_ENTITY_LIST
			if(current_sort_order == 4) //PE: Detailed display in one column.
				iColumns_leftpanel = 1;
			#endif

			if (!sorted_entity_files.empty())
			{
				ImGui::Columns(iColumns_leftpanel, "mycolumns4entities", false);  //false no border

				bool bHoveredUsed = false;
				for (int iloop = 0; iloop < 2; iloop++) 
				{
					for (std::vector< std::pair<std::string, std::int32_t>>::iterator it = sorted_entity_files.begin(); it != sorted_entity_files.end(); ++it) 
					{
						if (it->second == 999999)
						{
							if (iloop == 1)
							{
								//Seperator.
								if (iColumns_leftpanel == 1)
								{
									ImGui::Separator();
									preview_count++;
									ImGui::NextColumn();
								}
								else 
								{
									if (iColumns_leftpanel == 1) 
									{
										ImGui::Separator();
										preview_count++;
										ImGui::NextColumn();
									}
									else 
									{
										for (int i = preview_count % iColumns_leftpanel;i < iColumns_leftpanel;i++)
										{
											preview_count++;
											ImGui::NextColumn();
										}
										ImGui::Separator();
									}
								}
							}
						}
						else if (it->second == 999998)
						{
							if (iloop == 1)
							{
								std::string sString = it->first;
								replaceAll(sString,"ZZZZ-", "");
								ImGui::Text(sString.c_str());
								preview_count++;
								ImGui::NextColumn();
							}
						}
						else if (it->second > 0) 
						{
							#ifdef ADD_DETAIL_LEFT_PANEL_ENTITY_LIST
							if (iloop == 0 && current_sort_order == 4)
							{
								//Display detailed list of entities.

								ImGui::SetWindowFontScale(1.0);

								bool DisplayEntry = true;
								char cName[512];
								strcpy(cName, t.entityprofileheader[it->second].desc_s.Get());

								if (strlen(cSearchEntities) > 0)
								{
									//PE: This will search the desc. and the object name.
									if (!pestrcasestr(cName, cSearchEntities))
										DisplayEntry = false;
								}

								bool bUseWideThumb = false;
								int iTextureID = t.entityprofile[it->second].iThumbnailSmall;
								if (t.entityprofile[it->second].iThumbnailLarge > 0)
								{
									bUseWideThumb = true;
									iTextureID = t.entityprofile[it->second].iThumbnailLarge;
								}

								if (t.entityprofile[it->second].groupreference != -1)
								{
									//PE: Dont display smartobjects as they are not really a object.
									DisplayEntry = false;
								}

								if (DisplayEntry && iTextureID > 0)
								{

									ImGui::PushID(uniqueId++);
									float fFramePadding = (iColumnsWidth_leftpanel - media_icon_size_leftpanel)*0.5;
									float fCenterX = ImGui::GetContentRegionAvail().x * 0.5;
									ImVec2 vIconSize = { (float)media_icon_size_leftpanel , (float)media_icon_size_leftpanel };

									if (!bUseWideThumb)
									{
										//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (fCenterX - (media_icon_size_leftpanel*0.5)), ImGui::GetCursorPosY()));
									}
									else
									{
										//512x288
										float fRatio = 288.0f / 512.0f;
										float fImageWidth = ImGui::GetContentRegionAvail().x - 4.0f;
										vIconSize = { fImageWidth ,fImageWidth*fRatio };
									}

									char *cFind = strstr(cName, "###");
									if (cFind)
										cFind[0] = '\0';

									if (t.entityprofile[it->second].groupreference != -1)
									{
										strcat(cName, " (Smart Object)");
									}
									//if (ImGui::ImgBtn(iTextureID, vIconSize, drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true, false, false, false, true, false))

									ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow; //Got sub selections.

									//node_flags = ImGuiTreeNodeFlags_Leaf; //No sub selections.

									bool bSelected = false;

									//Find selection here.
									//if (bSelected)
									//	node_flags |= ImGuiTreeNodeFlags_Selected;
									//else
									//	node_flags &= ~ImGuiTreeNodeFlags_Selected;

									ImGui::PushItemWidth(-20.0); //PE: Room for a icon.
									std::string treename = cName;
									//treename[0] = toupper(treename[0]);
									bool TreeNodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)(it->second + 99000), node_flags, treename.c_str());
									bool bHovered = ImGui::IsItemHovered();
									ImGui::PopItemWidth();

									//bDraggingActive = false;
									if (!bHoveredUsed && bHovered && bToolTipActive && !bDraggingActive)
									{
										bHoveredUsed = true;
										ImGui::BeginTooltip();
										ImGui::ImgBtn(iTextureID, vIconSize, drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true, false, false, false, true, false);
										//ImGui::Text(treename.c_str());
										ImGui::EndTooltip();
									}

									bool bTreeNodeSelected = false;

									//if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
									//{
									//	//Only allow drag/drop.
									//	//bTreeNodeSelected = true;
									//}

									if (TreeNodeOpen) {
										ImGui::Indent(-5);
										//Display any sub nodes
										DoTreeNodeEntity(it->second);
										ImGui::Indent(5);
										ImGui::TreePop();
									}

									if(bTreeNodeSelected)
									{
										//Only if we are not dragging in a trashcan.
										if (bToolTipActive)
										{
											if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
											if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false;
											if (bImporter_Window) { importer_quit(); bImporter_Window = false; }


											DeleteWaypointsAddedToCurrentCursor();
											//CheckTooltipObjectDelete();
											CloseDownEditorProperties();
											//Make sure we are in entity mode.
											bForceKey = true;
											csForceKey = "e";
											t.gridentity = it->second;
											t.inputsys.constructselection = it->second;
											t.inputsys.domodeentity = 1;
											t.grideditselect = 5;
											editor_refresheditmarkers();
										}
									}

									ImGui::PopID();
									preview_count++;
									ImGui::NextColumn();
								}

							}
							else
							{
							#endif
								if ((iloop == 0 && t.entityprofile[it->second].ismarker == 0) || (iloop == 1 && t.entityprofile[it->second].ismarker > 0))
								{
									bool DisplayEntry = true;
									char cName[512];
									strcpy(cName, t.entityprofileheader[it->second].desc_s.Get());

									if (strlen(cSearchEntities) > 0)
									{
										//PE: This will search the desc. and the object name.
										if (!pestrcasestr(cName, cSearchEntities))
											DisplayEntry = false;
									}

									bool bUseWideThumb = false;
									int iTextureID = t.entityprofile[it->second].iThumbnailSmall;
									#ifdef USEWIDEICONSEVERYWHERE
									if (t.entityprofile[it->second].iThumbnailLarge > 0)
									{
										bUseWideThumb = true;
										iTextureID = t.entityprofile[it->second].iThumbnailLarge;
									}
									#endif
									if (DisplayEntry && iTextureID > 0)
									{
										// get ready to overlay a smart object icon
										ImVec2 vSmartObjectIconPos = ImGui::GetCursorPos();

										ImGui::PushID(uniqueId++);
										float fFramePadding = (iColumnsWidth_leftpanel - media_icon_size_leftpanel)*0.5;
										//float fCenterX = iColumnsWidth * 0.5;
										float fCenterX = ImGui::GetContentRegionAvail().x * 0.5;
										ImVec2 vIconSize = { (float)media_icon_size_leftpanel , (float)media_icon_size_leftpanel };

										//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(fFramePadding, 2.0f));
										if (!bUseWideThumb)
										{
											ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (fCenterX - (media_icon_size_leftpanel*0.5)), ImGui::GetCursorPosY()));
										}
										else
										{
											//512x288
											float fRatio = 288.0f / 512.0f;
											float fImageWidth = ImGui::GetContentRegionAvail().x - 4.0f;
											vIconSize = { fImageWidth ,fImageWidth*fRatio };
										}

										// Entity Left Panel.
										if (ImGui::ImgBtn(iTextureID, vIconSize, drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true, false, false, false, true, false))
										{
											//Only if we are not dragging in a trashcan.
											if (bToolTipActive)
											{
												if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
												//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
												if (bImporter_Window) { importer_quit(); bImporter_Window = false; }


												DeleteWaypointsAddedToCurrentCursor();
												//CheckTooltipObjectDelete();
												CloseDownEditorProperties();
												//Make sure we are in entity mode.
												bForceKey = true;
												csForceKey = "e";
												t.gridentity = it->second;
												t.inputsys.constructselection = it->second;
												t.inputsys.domodeentity = 1;
												t.grideditselect = 5;
												editor_refresheditmarkers();
											}
										}


										char *cFind = strstr(cName, "###");
										if (cFind)
											cFind[0] = '\0';
										if (ImGui::IsItemHovered() && bToolTipActive) ImGui::SetTooltip("%s", cName);

										if (bDisplayText_leftpanel)
										{
											#ifdef USEWIDEICONSEVERYWHERE
											ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, -5.0f));
											#endif
											ImGui::TextCenter("%s", cName); //no wrap.
										}

										// show when object is a smart object

										ImGui::PopID();
										preview_count++;
										ImGui::NextColumn();
									}
								}

							#ifdef ADD_DETAIL_LEFT_PANEL_ENTITY_LIST
							}
							#endif
						}
					}
				}
				ImGui::Columns(1);
			}

			ImGui::SetWindowFontScale(1.00);
			if (ImGui::IsWindowHovered() || ImGui::IsAnyItemHovered())
				bImGuiGotFocus = true;

			//PE: display additional debug information.
			//ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate); //cyb
			//ImGui::Text("DrawCalls: %d", g_pGlob->dwNumberOfPrimCalls);
			//ImGui::Text("Poly: %d", g_pGlob->dwNumberOfPolygonsDrawn);
			//ImGui::Text("triggerfindundercursor: %d", triggerfindundercursor);
			//ImGui::Text("bImGuiRenderTargetFocus: %d", (int) bImGuiRenderTargetFocus);
			//ImGui::Text("t.inputsys.mclick: %d", (int)t.inputsys.mclick);
			//ImGui::Text("t.onedrag: %d", t.onedrag);
			//ImGui::Text("io.WantCaptureKeyboard: %d", (int) io.WantCaptureKeyboard);
			//ImGui::Text("io.WantCaptureMouse: %d", (int)io.WantCaptureMouse);
			//ImGui::Text("IsAnyItemActive(): %d", (int)ImGui::IsAnyItemActive());
			//ImGui::Text("IsMouseClicked: %d", (int)ImGui::IsMouseClicked(0));
			//ImGui::Text("IsMouseDown: %d", (int)ImGui::IsMouseDown(0));
			//ImGui::Text("MouseClick(): %d", (int)MouseClick());
			//ImGui::Text("GetMousePos x,y: %d , %d", (int)ImGui::GetMousePos().x, (int)ImGui::GetMousePos().y);
			

//				ImGui::Text("object_preload_still_running: %d", (int)object_preload_still_running());

			//PE: enable below for statup time and memory dump function.
//				if (ImGui::StyleButton("Memory Dump")) {
//					timestampactivity(0, "Memory Dump:");
//					DumpImageList(); // PE: Dump image usage after level.
//				}
			//ImGui::Text("Startup Time: %d", iStartupTime);
			//extern int LoadImageCoreRetainNameTime;
			//ImGui::Text("LoadImageTime: %d", LoadImageCoreRetainNameTime);


			//Test code for direct center hit
			/*
			float placeatx_f, placeatz_f;
			extern ImVec2 OldrenderTargetSize;
			extern ImVec2 OldrenderTargetPos;
			ImVec2 vCenterPos = { (OldrenderTargetSize.x*0.5f) + OldrenderTargetPos.x , (OldrenderTargetSize.y*0.45f) + OldrenderTargetPos.y };

			int omx = t.inputsys.xmouse, omy = t.inputsys.ymouse, oldgridentitysurfacesnap = t.gridentitysurfacesnap, oldonedrag = t.onedrag;;
			bool owdm = bWaypointDrawmode;

			//Always target terrain only.
			float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
			float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
			t.inputsys.xmouse = (vCenterPos.x - renderTargetAreaPos.x) * RatioX;
			t.inputsys.ymouse = (vCenterPos.y - renderTargetAreaPos.y) * RatioY;

			t.gridentitysurfacesnap = 0; t.onedrag = 0; bWaypointDrawmode = false;
			input_calculatelocalcursor();
			placeatx_f = t.inputsys.localx_f;
			placeatz_f = t.inputsys.localy_f;
			t.onedrag = oldonedrag;
			bWaypointDrawmode = owdm;
			t.gridentitysurfacesnap = oldgridentitysurfacesnap;
			t.inputsys.xmouse = omx;
			t.inputsys.ymouse = omy;
			//Restore real input.
			input_calculatelocalcursor();
			if (t.inputsys.picksystemused == 1 || t.inputsys.localcurrentterrainheight_f < 100.0f)
			{

			}
//				PickScreen2D23D(vCenterPos.x, vCenterPos.y, 500);
//				placeatx_f = CameraPositionX() + GetPickVectorX();
//				placeatz_f = CameraPositionZ() + GetPickVectorZ();
			float terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, placeatx_f, placeatz_f, 1);

			if (ObjectExist(g.editorwaypointoffset + 0) == 0)
			{
				MakeObjectSphere(g.editorwaypointoffset + 0, 25);
				SetObjectCollisionOff(g.editorwaypointoffset + 0);
				SetAlphaMappingOn(g.editorwaypointoffset + 0, 25);
				DisableObjectZRead(g.editorwaypointoffset + 0);
				//			DisableObjectZWrite(g.editorwaypointoffset + 0);
				HideObject(g.editorwaypointoffset + 0);
				SetObjectMask(g.editorwaypointoffset + 0, 1);
				SetObjectEffect(g.editorwaypointoffset + 0, g.guishadereffectindex);
				SetObjectEmissive(g.editorwaypointoffset + 0, Rgb(255, 255, 255));
				SetObjectMask(g.editorwaypointoffset + 0, 1);
			}
			//  Show where we draw waypoints
			if (ObjectExist(g.editorwaypointoffset + 0) == 1)
			{
				PositionObject(g.editorwaypointoffset + 0, placeatx_f, terrain_height + 5, placeatz_f);
				ShowObject(g.editorwaypointoffset + 0);
			}
			*/


			if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
				//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
				ImGui::Text("");
				ImGui::Text("");
				ImGui::Text("");
			}

			ImGui::EndChild();

			//Drag/Drop models.
//			if (ImGui::BeginDragDropTarget())
//			{
//				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MODEL_DROP_TARGET", 0)) // ImGuiDragDropFlags_AcceptNoDrawDefaultRect
//				{
//					AddPayLoad((ImGuiPayload*)payload, false);
//				}
//			}

			// used to hide game elements so UI not too cluttered when in SHOOTER/RPG/PUZZLE GAMEPLAY panel mode
			//PE: Changed in new design to display all 15 icons. so bHideGameElementsWhenInSpecialGameplayMode always false.
			bool bHideGameElementsWhenInSpecialGameplayMode = false;



			//CheckMinimumDockSpaceSize(100.0f);


			//Drag/Drop to remove objects.
			ImRect bb = { ImGui::GetWindowContentRegionMin()+ImGui::GetWindowPos(),ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos() };


			if (ImGui::BeginDragDropTargetCustom(bb, 12345))
			{
				//ImGui::Text("BeginDragDropTargetCustom");
				//Hightlight Here!
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MODEL_DROP_TARGET", 0)) // ImGuiDragDropFlags_AcceptNoDrawDefaultRect
				{
					AddPayLoad((ImGuiPayload*)payload, false);
				}
			}

			ImGui::End();



		}

		//################################
		//#### EBE BUILDER LEFT PANEL ####
		//################################

#ifdef USELEFTPANELSTRUCTUREEDITOR
		if (refresh_gui_docking == 0) {
			//Need to be here while first time docking.
			ImGui::Begin("Structure Editor##LeftPanel", &bBuilder_Left_Window, 0);
			ImGui::End();
		}
		if (bBuilder_Left_Window) {

			ImGui::Begin("Structure Editor##LeftPanel", &bBuilder_Left_Window);

			extern char ActiveEBEFilename[260];
			int preview_count = 0;
			float contentarea = ImGui::GetWindowSize().x * ImGui::GetWindowSize().y;
			int media_icon_size = 64;
			int iColumnsWidth = 110;
			bool bNoText = false;
			if (contentarea > 90000) {
				media_icon_size = 64;
				iColumnsWidth = 110;
			}
			else if (contentarea > 80000) {
				media_icon_size = 48;
				iColumnsWidth = 110 - 16;
			}
			else if (contentarea > 40000) {
				media_icon_size = 32;
				iColumnsWidth = 110 - 16 - 16;
			}
			else {
				media_icon_size = 16;
				iColumnsWidth = 110 - 16 - 16 - 16 - 16;
				bNoText = true;
			}

			bool bDisplayText = true;
			ImGui::SetWindowFontScale(SMALLFONTSIZE);
			float fWinWidth = ImGui::GetWindowSize().x - 10.0; // Flicker - ImGui::GetCurrentWindow()->ScrollbarSizes.x;
			if (iColumnsWidth >= fWinWidth && fWinWidth > media_icon_size) {
				iColumnsWidth = fWinWidth;
				ImGui::SetWindowFontScale(SMALLESTFONTSIZE);
			}
			if (fWinWidth <= media_icon_size + 10) {
				iColumnsWidth = media_icon_size;
				ImGui::SetWindowFontScale(SMALLESTFONTSIZE);
			}
			if (fWinWidth <= 42) {
				//				media_icon_size = 32;
				iColumnsWidth = media_icon_size + 16;
				bDisplayText = false;
			}

			int iColumns = (int)(ImGui::GetWindowSize().x / (iColumnsWidth));
			if (iColumns <= 1)
				iColumns = 1;

			ImGui::Columns(iColumns, "mycolumns4entities", false);  //false no border

			float fFramePadding = (iColumnsWidth - media_icon_size)*0.5;
			float fCenterX = iColumnsWidth * 0.5;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(fFramePadding, 2.0f));

			for (int il = 0; il < t.ebebankmax; il++) {
				int icon = TOOL_BUILDER;
				char tmp[MAX_PATH];
				strcpy(tmp, t.ebebank_s[il].Get());
				int pos = strlen(tmp);
				while (pos > 0 && tmp[pos] != '\\') pos--;
				if (pos > 0) {
					cstr Text;
					strcpy(&tmp[0], &tmp[pos + 1]);
					if (pestrcasestr(tmp, "new site.")) { icon = EBE_NEW; Text = "Add New Site"; }
					else if (pestrcasestr(tmp, "cube.")) { icon = EBE_CUBE; Text = "Cube"; }
					else if (pestrcasestr(tmp, "floor.")) { icon = EBE_FLOOR; Text = "Floor"; }
					else if (pestrcasestr(tmp, "wall.")) { icon = EBE_WALL; Text = "Wall"; }
					else if (pestrcasestr(tmp, "column.")) { icon = EBE_COLUMN; Text = "Column"; }
					else if (pestrcasestr(tmp, "row.")) { icon = EBE_ROW; Text = "Row"; }
					else if (pestrcasestr(tmp, "stairs.")) { icon = EBE_STAIRS; Text = "Stairs"; }
					else if (pestrcasestr(tmp, "block.")) { icon = EBE_BLOCK; Text = "Block"; }
					else {
						icon = TOOL_BUILDER;
						int pos2 = 0;
						while (tmp[pos2] != '.' && pos2 < strlen(tmp)) pos2++;
						if (pos2 > 0)
							tmp[pos2] = 0;
						Text = tmp;
					}

					if (strlen(ActiveEBEFilename) > 0 && pestrcasestr(t.ebebank_s[il].Get(), ActiveEBEFilename)) {
						ImVec4 bg_col = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram]; // { 0.0, 0.0, 0.0, 1.0 };
						ImVec2 padding = { 4.0, 4.0 };
						ImGuiWindow* window = ImGui::GetCurrentWindow();
						const ImRect image_bb((window->DC.CursorPos - padding) + ImVec2(fFramePadding, 2.0f), window->DC.CursorPos + padding + ImVec2(fFramePadding, 2.0f) + ImVec2(media_icon_size, media_icon_size));
						window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(bg_col), 0.0f, 15, 3.0f);
					}

					if (ImGui::ImgBtn(icon, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true))
					{
						if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
						if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
						if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false;

						DeleteWaypointsAddedToCurrentCursor();
						//CheckTooltipObjectDelete();
						CloseDownEditorProperties();

						t.addentityfile_s = t.ebebank_s[il].Get();
						if (t.addentityfile_s != "")
						{
							entity_adduniqueentity(false);
							t.tasset = t.entid;
							if (t.talreadyloaded == 0)
							{
								editor_filllibrary();
							}
						}
						t.inputsys.constructselection = t.tasset;

						t.gridentity = t.entid;
						t.inputsys.constructselection = t.entid;
						t.inputsys.domodeentity = 1;
						t.grideditselect = 5;
						editor_refresheditmarkers();

						if (icon != EBE_NEW) {
							LPSTR pPBFEBEFile = t.ebebank_s[il].Get();
							ebe_loadpattern(pPBFEBEFile);
							t.inputsys.constructselection = 0;
						}
						else {
							//NewSite, make sure we are in entity mode.
							bForceKey = true;
							csForceKey = "e";
						}

					}
					if (ImGui::IsItemHovered() && Text != "") ImGui::SetTooltip("%s", Text.Get());

					if (!bNoText) {
						int iTextWidth = ImGui::CalcTextSize(Text.Get()).x;
						if (iTextWidth < iColumnsWidth)
							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (fCenterX - (iTextWidth*0.5)), ImGui::GetCursorPosY()));
						ImGui::TextWrapped(Text.Get());
					}
					ImGui::NextColumn();


				}
			}


			ImGui::PopStyleVar();
			ImGui::SetWindowFontScale(1.00);
			ImGui::Columns(1);

			CheckMinimumDockSpaceSize(200.0f);

			ImGui::End();
		}
#endif

		//##############################
		//#### Bug Reporting system ####
		//##############################



		//######################################
		//#### Handle Change to Hand cursor ####
		//######################################

		//if (refresh_gui_docking >= 4)
		//{
		//	//PE: Set hand if we can pick and move a object.
		//	if (iReusePickEntityID > 0 && t.entityelement[iReusePickEntityID].editorlock == 0)
		//	{
		//		if(ImGui::GetMouseCursor() == 0) //Only if arrow cursor.
		//			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		//	}
		//}


		//###########################
		//#### VISULS LEFT PANEL ####
		//###########################


		bImGuiReadyToRender = true;

		if (refresh_gui_docking < 4) 
		{
			refresh_gui_docking++;
		}
		else 
		{
			if (!bImGuiInitDone)
			{
			}
			bImGuiInitDone = true;
			static bool bLeftPanelSelectedAsDefault = false;
			if (!bLeftPanelSelectedAsDefault) {
				//ImGui::SetWindowFocus(TABENTITYNAME);
				//PE: Start in terrain tools.
				ImGui::SetWindowFocus("Terrain Tools##TerrainToolsWindow");

				bLeftPanelSelectedAsDefault = true;
			}

		}

		//Some need launch after we have bImGuiReadyToRender , so prompt will work.
		if (iSkibFramesBeforeLaunch == 0) 
		{
			switch (iLaunchAfterSync)
			{
				case 2: //Open
					iLaunchAfterSync = 0;
					int iRet;
					iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						//t.returnstring_s must have full path to .fpm.

						//PE: filedialogs change dir so.
						cStr tOldDir = GetDir();
						char * cFileSelected;
						cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "fpm\0*.fpm\0", g.mysystem.mapbankAbs_s.Get() , NULL, true);
						SetDir(tOldDir.Get());
						if (cFileSelected && strlen(cFileSelected) > 0) 
						{
							t.returnstring_s = cFileSelected;
							if (t.returnstring_s != "")
							{
								if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) == ".fpm")
								{
									t.gridentity = 0;
									t.inputsys.constructselection = 0;
									t.inputsys.domodeentity = 1;
									t.grideditselect = 5;
									editor_refresheditmarkers();

									g.projectfilename_s = t.returnstring_s;
									gridedit_load_map();

									iLaunchAfterSync = 80; //Update env
									iSkibFramesBeforeLaunch = 5;

									int firstempty = -1;
									int i = 0;
									for (; i < REMEMBERLASTFILES; i++) {
										if (firstempty == -1 && strlen(pref.last_open_files[i]) <= 0)
											firstempty = i;

										if (strlen(pref.last_open_files[i]) > 0 && pestrcasestr(g.projectfilename_s.Get(), pref.last_open_files[i])) { //already there
											break;
										}
									}
									if (i >= REMEMBERLASTFILES) {
										if (firstempty == -1) {
											//No empty slots , rotate.
											for (int ii = 0; ii < REMEMBERLASTFILES-1; ii++) {
												strcpy(pref.last_open_files[ii], pref.last_open_files[ii + 1]);
											}
											strcpy(pref.last_open_files[REMEMBERLASTFILES-1], g.projectfilename_s.Get());
										}
										else
											strcpy(pref.last_open_files[firstempty], g.projectfilename_s.Get());
									}


									//Locate player start marker.
									for (t.e = 1; t.e <= g.entityelementlist; t.e++)
									{
										if (t.entityelement[t.e].bankindex > 0)
										{
											if (t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 1 && t.entityprofile[t.entityelement[t.e].bankindex].lives != -1)
											{
												//Point camera.
												t.obj = t.entityelement[t.e].obj;
												if (t.obj > 0) {
													float offsetx = ((float)GetDesktopWidth() - renderTargetAreaSize.x) * 0.25f;
													t.cx_f = ObjectPositionX(t.obj) + offsetx; //t.editorfreeflight.c.x_f;
													t.cy_f = ObjectPositionZ(t.obj); //t.editorfreeflight.c.z_f;
												}
												break;
											}
										}
									}

								}
							}
						}
					}
					iLastUpdateVeg = 0;
					bUpdateVeg = true;
					break;
				case 503: //Save the actual map here!
				{
					iLaunchAfterSync = 0;
					gridedit_save_map();
					g.projectmodified = 0; gridedit_changemodifiedflag();
					g.projectmodifiedstatic = 0;

					break;
				}
				case 3: //Save
					iLaunchAfterSync = 0;
					if (g.projectmodified == 1)
					{
						if (t.ebe.on == 1)
							ebe_hide(); //Make sure we have last ebe changes.

						//  yes save first
						if (g.projectfilename_s == "")
						{
							t.returnstring_s = "";
							cStr tOldDir = GetDir();
							char * cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_SAVE, "fpm\0*.fpm\0", g.mysystem.mapbankAbs_s.Get(), NULL,true);
							SetDir(tOldDir.Get());
							if (cFileSelected && strlen(cFileSelected) > 0) {
								t.returnstring_s = cFileSelected;
							}
							if (t.returnstring_s != "")
							{
								if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) != ".fpm")  t.returnstring_s = t.returnstring_s + ".fpm";
								g.projectfilename_s = t.returnstring_s;
								bool oksave = true;
								if (FileExist(g.projectfilename_s.Get())) {
									oksave = overWriteFileBox(g.projectfilename_s.Get());
								}
								if (oksave) 
								{

									iLaunchAfterSync = 503; //Do the actualy save here.
									iSkibFramesBeforeLaunch = 3;

									//gridedit_save_map();

									//Add newly saved fpm level to recent list.
									int firstempty = -1;
									int i = 0;
									for (; i < REMEMBERLASTFILES; i++) {
										if (firstempty == -1 && strlen(pref.last_open_files[i]) <= 0)
											firstempty = i;

										if (strlen(pref.last_open_files[i]) > 0 && pestrcasestr(g.projectfilename_s.Get(), pref.last_open_files[i])) { //already there
											break;
										}
									}
									if (i >= REMEMBERLASTFILES) {
										if (firstempty == -1) {
											//No empty slots , rotate.
											for (int ii = 0; ii < REMEMBERLASTFILES - 1; ii++) {
												strcpy(pref.last_open_files[ii], pref.last_open_files[ii + 1]);
											}
											strcpy(pref.last_open_files[REMEMBERLASTFILES - 1], g.projectfilename_s.Get());
										}
										else
											strcpy(pref.last_open_files[firstempty], g.projectfilename_s.Get());
									}

								}
							}
						}
						else
						{
							iLaunchAfterSync = 503; //Do the actualy save here.
							iSkibFramesBeforeLaunch = 3;
							//gridedit_save_map();
						}

						//g.projectmodified = 0; gridedit_changemodifiedflag();
						//g.projectmodifiedstatic = 0;
					}
					break;

				case 4: //Save As
				{
					iLaunchAfterSync = 0;
					if (t.ebe.on == 1)
						ebe_hide(); //Make sure we have last ebe changes.

					t.returnstring_s = "";
					cStr tOldDir = GetDir();
					char * cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_SAVE, "fpm\0*.fpm\0", g.mysystem.mapbankAbs_s.Get(), NULL,true);
					SetDir(tOldDir.Get());
					if (cFileSelected && strlen(cFileSelected) > 0) {
						t.returnstring_s = cFileSelected;
					}
					if (t.returnstring_s != "")
					{
						if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) != ".fpm")  t.returnstring_s = t.returnstring_s + ".fpm";
						g.projectfilename_s = t.returnstring_s;

						bool oksave = true;
						if (FileExist(g.projectfilename_s.Get())) {
							oksave = overWriteFileBox(g.projectfilename_s.Get());
						}
						if (oksave) 
						{
							//Add newly saved fpm level to recent list.
							int firstempty = -1;
							int i = 0;
							for (; i < REMEMBERLASTFILES; i++) {
								if (firstempty == -1 && strlen(pref.last_open_files[i]) <= 0)
									firstempty = i;
								if (strlen(pref.last_open_files[i]) > 0 && pestrcasestr(g.projectfilename_s.Get(), pref.last_open_files[i])) { //already there
									break;
								}
							}
							if (i >= REMEMBERLASTFILES) {
								if (firstempty == -1) {
									//No empty slots , rotate.
									for (int ii = 0; ii < REMEMBERLASTFILES - 1; ii++) {
										strcpy(pref.last_open_files[ii], pref.last_open_files[ii + 1]);
									}
									strcpy(pref.last_open_files[REMEMBERLASTFILES - 1], g.projectfilename_s.Get());
								}
								else
									strcpy(pref.last_open_files[firstempty], g.projectfilename_s.Get());
							}

							iLaunchAfterSync = 503; //Do the actualy save here.
							iSkibFramesBeforeLaunch = 3;

//							gridedit_save_map();

						}
					}

					//g.projectmodified = 0; gridedit_changemodifiedflag();
					//g.projectmodifiedstatic = 0;

					break;
				}

				case 5: // New flatten level
				{
					iLaunchAfterSync = 0;
					int iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						#ifdef PROCEDURALTERRAINWINDOW

						bProceduralLevel = true;

						#else


						t.inputsys.donewflat = 1;
						t.inputsys.donew == 1;
						gridedit_new_map();
						t.inputsys.donewflat = 0;
						t.inputsys.donew = 0;

						iLaunchAfterSync = 80; //Update env
						iSkibFramesBeforeLaunch = 5;


						#endif // else PROCEDURALTERRAINWINDOW
					}
					#ifndef PROCEDURALTERRAINWINDOW
					iLastUpdateVeg = 0;
					bUpdateVeg = true;
					#endif

					break;
				}

				case 6: // New level
				{
					iLaunchAfterSync = 0;
					int iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						t.inputsys.donewflat = 0;
						t.inputsys.donew == 1;
						gridedit_new_map();
						t.inputsys.donewflat = 0;
						t.inputsys.donew = 0;

						iLaunchAfterSync = 80; //Update env
						iSkibFramesBeforeLaunch = 5;

					}
					iLastUpdateVeg = 0;
					bUpdateVeg = true;
					break;
				}

				case 7: // Direct Open
				{
					iLaunchAfterSync = 0;
					int iRet;
					iRet = AskSaveBeforeNewAction();
					if (iRet != 2)
					{
						if (strlen(cDirectOpen) > 0) {

							t.returnstring_s = cDirectOpen;
							if (t.returnstring_s != "")
							{
								if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) == ".fpm")
								{
									t.gridentity = 0;
									t.inputsys.constructselection = 0;
									t.inputsys.domodeentity = 1;
									t.grideditselect = 5;
									editor_refresheditmarkers();

									g.projectfilename_s = t.returnstring_s;
									gridedit_load_map();

									iLaunchAfterSync = 80; //Update env
									iSkibFramesBeforeLaunch = 5;

									//Locate player start marker.
									for (t.e = 1; t.e <= g.entityelementlist; t.e++)
									{
										if (t.entityelement[t.e].bankindex > 0)
										{
											if (t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 1 && t.entityprofile[t.entityelement[t.e].bankindex].lives != -1)
											{
												//Point camera.
												t.obj = t.entityelement[t.e].obj;
												if (t.obj > 0) {
													float offsetx = ((float)GetDesktopWidth() - renderTargetAreaSize.x) * 0.25f;
													t.cx_f = ObjectPositionX(t.obj) + offsetx; //t.editorfreeflight.c.x_f;
													t.cy_f = ObjectPositionZ(t.obj); //t.editorfreeflight.c.z_f;
												}
												break;
											}
										}
									}

								}
							}
						}
					}
					iLastUpdateVeg = 0;
					bUpdateVeg = true;
					break;
				}

				case 8: //Import model.
				{
					iLaunchAfterSync = 0;
					cStr tOldDir = GetDir();
					char * cFileSelected;
					cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "All\0*.*\0X\0*.x\0DBO\0*.dbo\0OBJ\0*.obj\0FBX\0*.fbx\0GLTF\0*.gltf\0GLB\0*.glb\0\0\0", NULL, NULL , true );
					SetDir(tOldDir.Get());
					if (cFileSelected && strlen(cFileSelected) > 0) 
					{
						char szModelPath[ MAX_PATH ];
						strcpy( szModelPath, cFileSelected );
						const char* pExtension = strrchr( szModelPath, '.' );
						if ( !pExtension )
						{
							strcpy(cTriggerMessage, "File extension not found");
							bTriggerMessage = true;
						}
						else
						{
							t.returnstring_s = szModelPath;
							bool bPermittedFormat = false;
							if (stricmp(pExtension, ".x") == NULL) bPermittedFormat = true;
							if (stricmp(pExtension, ".dbo") == NULL) bPermittedFormat = true;
							if (stricmp(pExtension, ".obj") == NULL) bPermittedFormat = true;
							if (stricmp(pExtension, ".fbx") == NULL) bPermittedFormat = true;
							if (bPermittedFormat == true)
							{
								// load the model
								t.timporterfile_s = t.returnstring_s;
								importer_loadmodel();
							}
							else
							{
								strcpy(cTriggerMessage, "This is not a supported model file.");
								bTriggerMessage = true;
							}
						}
					}
					if (bDelayedTutorialCheckAction == TOOL_IMPORT) 
					{
						bDelayedTutorialCheckAction = -1;
						TutorialNextAction();
					}
					// clear 'pLaunchAfterSyncPreSelectModel' as this is a one time use until set again
					break;
				}

				case 80: //Update envmap
				{
					iLaunchAfterSync = 0;
					//Make sure we have envmap.
					visuals_justshaderupdate();
					t.visuals.refreshskysettingsfromlua = true;
					cubemap_generateglobalenvmap();
					t.visuals.refreshskysettingsfromlua = false;
					//extern bool bFullVegUpdate;
					//bFullVegUpdate = true;
					bUpdateVeg = true;


					break;
				}

				case 81: //Delayed window focus.
				{
					iLaunchAfterSync = 0;
					break;
				}

				default:
					break;
			}
		}
		else
		{
			iSkibFramesBeforeLaunch--;
		}

		//##########################
		//#### Triger A Message ####
		//##########################

		if (bTriggerMessage ) {

			if (iMessageTimer == 0 || Timer() - iMessageTimer > 4100)
				iMessageTimer = Timer();

			ImGui::SetNextWindowPos(OldrenderTargetPos + ImVec2(50, 50), ImGuiCond_Always); //ImGuiCond_Always
			ImGui::SetNextWindowSize(ImVec2(OldrenderTargetSize.x-100, 0), ImGuiCond_Always); //ImGuiCond_Always
			bool winopen = true;

			ImVec4* style_colors = ImGui::GetStyle().Colors;
			ImVec4 oldBgColor = style_colors[ImGuiCol_WindowBg];
			ImVec4 oldTextColor = style_colors[ImGuiCol_Text];

			float fader = ((float)Timer() - (float)iMessageTimer) / 1000.0f;
			fader -= 1.0;
			if (fader < 0) {
				fader = 0.0001;
			}
			fader /= 3.0;

			fader = 1.0 - fader;
			if (fader < 0.1) {
				bTriggerMessage = false;
			}
			style_colors[ImGuiCol_WindowBg].x = 0.0;
			style_colors[ImGuiCol_WindowBg].y = 0.0;
			style_colors[ImGuiCol_WindowBg].z = 0.0;
			//style_colors[ImGuiCol_WindowBg].w *= (fader*0.25);
			style_colors[ImGuiCol_WindowBg].w *= (fader*0.5);

			style_colors[ImGuiCol_Text].x = 1.0;
			style_colors[ImGuiCol_Text].y = 1.0;
			style_colors[ImGuiCol_Text].z = 1.0;
			style_colors[ImGuiCol_Text].w *= fader;

			ImGui::Begin("##Messageinfo", &winopen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
			ImGui::SetWindowFontScale(2.0);
			ImGui::Text(" ");
			//Center Text.
			float fTextSize = ImGui::CalcTextSize(cTriggerMessage).x;
			ImGui::SetCursorPos(ImVec2( (ImGui::GetWindowSize().x*0.5) - (fTextSize*0.5) , ImGui::GetCursorPos().y));

			ImGui::Text(cTriggerMessage);
			ImGui::Text(" ");
			ImGui::SetWindowFontScale(1.0);
			ImGui::End();
			style_colors[ImGuiCol_WindowBg] = oldBgColor;
			style_colors[ImGuiCol_Text] = oldTextColor;
		}


	}

#endif

	// 191015 - Trigger quick start dialog when editor flowing
	if ( iCountDownToShowQuickStartDialog > 0 )
	{
		iCountDownToShowQuickStartDialog--;
		if ( iCountDownToShowQuickStartDialog == 0 )
		{
			// insert parental control prompt if not used before
			if ( g.quickparentalcontrolmode == 0 )
			{
				// 050416 - ensure once time entrance to parental control from boot-up
				g.quickparentalcontrolmode = 1;
				editor_showparentalcontrolpage ();
			}
		}
	}
	#ifdef VRTECH
	#else
	else
	{
		#ifdef FREETRIALVERSION
			if ( g.iFreeVersionModeActive == 2 && dwStartOfEditingSession > 0 && timeGetTime() > dwStartOfEditingSession )
			{
			welcome_init(1);
			welcome_init(0);
			welcome_init(2);
			g.iFreeVersionModeActiveDuringEditor = 1;
			welcome_show(WELCOME_FREETRIALINTROAPP);
			g.iFreeVersionModeActiveDuringEditor = 0;
			welcome_free();
			if (dwSecondReminder == 0)
			{
				dwSecondReminder = 1;
				dwStartOfEditingSession = timeGetTime() + (1000*60*60);
			}
			else
				dwStartOfEditingSession = 0;
			}
		#endif
	}
	#endif

	//  Do not get start memory until processed one loop (below)
	if (  t.gamememactuallyusedstarttriggercount>0 ) 
	{
		--t.gamememactuallyusedstarttriggercount;
		if (  t.gamememactuallyusedstarttriggercount == 0 ) 
		{
			g.gamememactuallyusedstart=SMEMAvailable(1);
		}
	}

	//  Machine Independent Speed
	game_timeelapsed ( );
	t.ts_f=(Timer()-t.tsl_f)/50.0 ; t.tsl_f=Timer();

	//  Send SteamID to the editor if needed
	mp_sendSteamIDToEditor ( );

	//  User input calls
	#ifdef ENABLEIMGUI
	static bool imgui_onetime_init = false;
	if (g.globals.ideinputmode == 1) {

		#ifndef USEOLDIDE
		imgui_input_getcontrols();
		#else
		input_getcontrols();
		#endif
	}
	else 
	{
		input_getcontrols();
	}

	if (!imgui_onetime_init) 
	{
		imgui_onetime_init = true;
	}
	#else
	//  User input calls
	input_getcontrols();
	#endif

	#ifdef ENABLEIMGUI
	// could not launch Welcome system before IMGUI inits, so flagged to happen here
	if (iTriggerWelcomeSystemStuff > 0 && iTriggerWelcomeSystemStuff < 6) iTriggerWelcomeSystemStuff++;
	if (iTriggerWelcomeSystemStuff > 5)
	{
		// Init or Cycle
		if (iTriggerWelcomeSystemStuff == 6)
		{
			// only show front dialogs if not resuming from previous session
			if (g.grestoreeditorsettings == 0)
			{
				// Welcome quick start page
				g.quickstartmenumode = 0;
				if (g.iFreeVersionModeActive != 0)
				{
					editor_showquickstart(0);
					iTriggerWelcomeSystemStuff = 99;
				}
				else
				{
					if (g.gshowonstartup == 1 || g.iTriggerSoftwareToQuit != 0)
					{
						editor_showquickstart(0);
						iTriggerWelcomeSystemStuff = 99;
					}
					else
					{
						welcome_free();
						iTriggerWelcomeSystemStuff = 7;
					}
				}
			}
			else
			{
				// always need to close down loading splash
				welcome_free();
				iTriggerWelcomeSystemStuff = 7;
			}
		}
		else if (iTriggerWelcomeSystemStuff == 7)
		{
			//Exit wait for mouse release before closing welcome screen.
			if(t.inputsys.mclick == 0)
				iTriggerWelcomeSystemStuff = 0;
		}
		else
		{
			// Cycle - handle welcome loop
			if (welcome_cycle() == true)
			{
				welcome_free();
				iTriggerWelcomeSystemStuff = 7;
			}
		}
		
		//Make sure we dont sent input to rendertarget when in welcome.
		t.inputsys.xmouse = 500000;
		t.inputsys.ymouse = 0;
		t.inputsys.xmousemove = 0;
		t.inputsys.ymousemove = 0;
		set_inputsys_mclick(0);// t.inputsys.mclick = 0;
		t.inputsys.zmouse = 0;
		t.inputsys.wheelmousemove = 0;
		t.inputsys.activemouse = 0;
		t.syncthreetimes = 1;
		t.inputsys.k_s = "";
		t.inputsys.keyreturn = 0;
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.keycontrol = 0;
		t.inputsys.keyspace = 0;
		t.inputsys.kscancode = 0;

	}
	#endif

	// calc local cursor
	bool bPickActive = true;
	if(bPickActive)
		input_calculatelocalcursor ( );

	// Character Creator Plus
	#ifdef VRTECH
	if ( g_bCharacterCreatorPlusActivated == true )
	{
		// character creator plus character edited in situ
		charactercreatorplus_loop();
	}
	#endif

	//  Importer or Main Editor
	if ( t.importer.loaded != 0 || (t.interactive.active == 1 && (t.interactive.pageindex<21 || t.interactive.pageindex>90)) )
	{
		//  Importer control or Interactive Mode
		if (  t.importer.loaded != 0 ) 
		{
			importer_loop ( );
			importer_draw ( );
		}
	}
	else
	{
		//  Character Kit Active
		#ifdef VRTECH
		//if(0) // t.characterkit.loaded !=  0 ) 
		//{
			// bye bye character creator, welcome plus!
			//characterkit_loop ( );
			//characterkit_draw ( );
		//}
		//else
		#else
		if ( t.characterkit.loaded !=  0 ) 
		{
			characterkit_loop ( );
			characterkit_draw ( );
		}
		else
		#endif
		{
			// give editor a chance to init, then switch to EBE for quick coding!
			// Editor Controls
			terrain_terraintexturesystempainterentry();
			editor_constructionselection();


			if ( t.grideditselect == 3 || t.grideditselect == 4 ) 
			{
				// Entity controls
				editor_viewfunctionality ( );
			}
			else
			{
				editor_mainfunctionality ( );
				if ( t.grideditselect == 0 ) 
				{
					// Terrain controls
					t.terrain.camx_f=t.cx_f ; t.terrain.camz_f=t.cy_f;
					t.terrain.zoom_f=t.gridzoom_f*0.12;
					terrain_editcontrol ( );
				}
				else
				{
					if ( t.ebe.on == 1 )
					{
						// Easy Building Editor
						ebe_loop();
					}
					else
					{
						//  Non-terrain controls
						gridedit_mapediting ( );
						terrain_editcontrol_auxiliary ( );
					}
				}
			}

			editor_overallfunctionality ( );
			terrain_detectendofterraintexturesystempainter ( );

			//  Handle visual components
			editor_detect_invalid_screen ( );
			editor_visuals ( );
			editor_undergroundscan ( );

			//  Ensure entity animations speeds are controlled
			entity_loopanim ( );

			//  Widget control
			widget_loop ( );

			//  Character creator loop
			///characterkit_updateAllCharacterCreatorEntitiesInMap ( );

			//  Ensure lighting is updated as lighting is edited and moved
			lighting_loop ( ); 

			//  Only show terrain cursor if in terrain edit mode
			if (  t.grideditselect == 0 && t.inputsys.mclick != 2 && t.inputsys.mclick != 4 && t.interactive.insidepanel == 0 ) 
			{
				terrain_cursor ( );
			}
			else
			{
				terrain_cursor_off ( );
			}

			//  Render terrain elements (shadowupdatepacer as shadow calc is expensive, time slice it)
			t.terrain.gameplaycamera=0;
			if ( Timer() > (int)t.editor.shadowupdatepacer ) 
			{
				t.editor.shadowupdatepacer=Timer()+20;
				terrain_shadowupdate ( );
			}
			terrain_update ( );
			terrain_waterineditor ( );

			//  IDE Communications (only when mouse not in 3D view)
			if (  t.inputsys.xmouse == 500000  )  t.inputsys.residualidecount = 10;
			if (  t.inputsys.residualidecount>0  )  t.inputsys.residualidecount = t.inputsys.residualidecount-1;
			if (  t.inputsys.residualidecount>0 ) 
			{
				//  Interface code
				interface_handlepropertywindow ( );

				//  Handle save standalone (cannot wait after 758 as straight into domodal)
				#ifdef FPSEXCHANGE
				OpenFileMap(3, "FPSEXCHANGE");
				for (t.idechecks = 1; t.idechecks <= 3; t.idechecks++)
				{
					if (t.idechecks == 1) { t.virtualfileindex = 758; t.tvaluetocheck = 1; }
					if (t.idechecks == 2) { t.virtualfileindex = 762; t.tvaluetocheck = 1; }
					if (t.idechecks == 3) { t.virtualfileindex = 762; t.tvaluetocheck = 2; }
					t.tokay = GetFileMapDWORD(3, t.virtualfileindex);
					if (t.tokay == t.tvaluetocheck)
					{
						SetEventAndWait(3);
						//CloseFileMap (  3 );
						if (t.idechecks == 1)
						{
							// Save Standalone
							gridedit_intercept_savefirst();
							OpenFileMap(3, "FPSEXCHANGE");
							if (t.editorcanceltask == 0)
							{
								popup_text(t.strarr_s[82].Get());
								gridedit_load_map(); // 190417 - ensures levelbank contents SAME as level 1 FPM!

								// new dialog to handle save standalone
								popup_text_close();
								//mapfile_savestandalone ( );
								//suggest new init code to load just what the save standalone dialog needs
								welcome_init(1);
								welcome_init(2);
								welcome_init(0);
								welcome_show(WELCOME_SAVESTANDALONE);
								welcome_free();
								SetFileMapDWORD(3, t.virtualfileindex, 3);
							}
							else
							{
								SetFileMapDWORD(3, t.virtualfileindex, 0);
							}
						}
						if (t.idechecks == 2)
						{
							// (dave) Skip terrain rendering - it causes a crash in debug
							g_bSkipTerrainRender = true;

							//  Import Model
							gridedit_import_ask();
							SetFileMapDWORD(3, t.virtualfileindex, 0);
						}
						if (t.idechecks == 3)
						{
							// Character Creator Plus
							//this is now old code, and will be removed during clean-up after first functionality draft complete
							//g_bCharacterCreatorPlusActivated = true; //if (t.characterkit.loaded == 0)  t.characterkit.loaded = 1;
							#ifndef VRTECH
							if (t.characterkit.loaded == 0)  t.characterkit.loaded = 1;
							#endif
							SetFileMapDWORD(3, t.virtualfileindex, 0);
						}
						SetEventAndWait(3);
					}
				}
				#endif

				// Handle auto trigger stock level loader
				#ifdef FPSEXCHANGE
				OpenFileMap (  3, "FPSEXCHANGE" );
				t.tleveltoautoload=GetFileMapDWORD( 3, 754 );
				if (  t.tleveltoautoload>0 ) 
				{
					// clear flag on performing this action
					SetFileMapDWORD (  3, 754, 0 );
					SetEventAndWait (  3 );

					// trigger event or load a map
					if ( t.tleveltoautoload>=1001 )
					{
						// trigger events
						if ( t.tleveltoautoload==1001 ) 
						{
							g.quickstartmenumode = 0;
							editor_showquickstart ( 1 );
						}
						if ( t.tleveltoautoload==1002 ) 
						{
							editor_showparentalcontrolpage();
						}
					}
					else
					{
						// load a map
						t.tlevelautoload_s="";
						switch (  t.tleveltoautoload ) 
						{
							case 1 : t.tlevelautoload_s = "The Big Escape.fpm" ; break ;
							case 2 : t.tlevelautoload_s = "Cartoon Antics.fpm" ; break ;
							case 3 : t.tlevelautoload_s = "Get To The River.fpm" ; break ;
							case 4 : t.tlevelautoload_s = "The Heirs Revenge.fpm" ; break ;
							case 5 : t.tlevelautoload_s = "Morning Mountain Stroll.fpm" ; break ;
							case 6 : t.tlevelautoload_s = "The Asylum.fpm" ; break ;
							case 7 : t.tlevelautoload_s = "Gem World.fpm" ; break ;
							case 21 : t.tlevelautoload_s = "Bridge Battle (MP).fpm" ; break ;
							case 22 : t.tlevelautoload_s = "Camp Oasis (MP).fpm" ; break ;
							case 23 : t.tlevelautoload_s = "Devils Hill (MP).fpm" ; break ;
							case 24 : t.tlevelautoload_s = "Sunset Island (MP).fpm" ; break ;
							case 25 : t.tlevelautoload_s = "The Beach (MP).fpm" ; break ;
						}
						t.tlevelautoload_s=g.mysystem.mapbankAbs_s+t.tlevelautoload_s;//g.fpscrootdir_s+"\\Files\\mapbank\\"+t.tlevelautoload_s;

						//  ask to save first if modified project open
						t.editorcanceltask=0;
						if (  g.projectmodified == 1 ) 
						{
							//  If project modified, ask if want to save first
							gridedit_intercept_savefirst ( );
						}
						if (  t.editorcanceltask == 0 ) 
						{
							if (  t.tlevelautoload_s != "" ) 
							{
								if (  cstr(Lower(Right(t.tlevelautoload_s.Get(),4))) == ".fpm" ) 
								{
									g.projectfilename_s=t.tlevelautoload_s;
									gridedit_load_map ( );

								}
							}
						}
					}
				}
				#endif
			}

			// 111115 - keep track of memory between sessions with simpler SYSMEM minus STARTMEM calculation
			g.gamememactuallyused = SMEMAvailable(1) - g.gamememactuallyusedstart;

			// 111115 - and introduce sliding effect to hide flicker due to reading direct system memory value
			t.tmempercdest_f = (g.gamememactuallyused+0.0f) / (g.gamememactualmaxrightnow+0.0f);
			if ( t.tmemperc_f < t.tmempercdest_f-0.01f )
			{
				t.tmemperc_f = t.tmemperc_f + 0.01f;
			}
			else
			{
				if ( t.tmemperc_f > t.tmempercdest_f+0.01f )
				{
					t.tmemperc_f = t.tmemperc_f - 0.01f;
				}
			}

			if (  t.tmemperc_f > 1.0f  )  t.tmemperc_f = 1.0;
			if (  g.ghidememorygauge == 0 ) 
			{
				// (Dave) - check the image exists
				if ( ImageExist ( g.editorimagesoffset+2 ) == 1 )
				{
					Ink (  Rgb(0,0,0),0  ); Box (  2,2,102,18 );
					Ink (  Rgb(0,255,0),0  ); Box (  2,2,3+(99*t.tmemperc_f),18 );
					PasteImage (  g.editorimagesoffset+2,2,2,1 );
				}
			}
			//  End of Character Creator branch
		}
		//  Import/Editor branch
	}

	//  Constantly checking if VIDMEM invalidated
	editor_detect_invalid_screen ( );

	// 191015 - test level click prompt
	if ( g.showtestlevelclickprompt > 0 )
	{
		if ( timeGetTime() > g.showtestlevelclickprompt )
		{
			g.showtestlevelclickprompt = 0;
		}
		int iXPos = 630;
		int iYPos = abs ( cos( timeGetTime()/500.0f )*35.0f );
		PasteImage ( g.editorimagesoffset+61, iXPos - (ImageWidth(g.editorimagesoffset+61)/2), 50+iYPos );
	}

	//  Update screen (if mouse in 3D are)
	if (  t.recoverdonotuseany3dreferences == 0 ) 
		{
		//  editor super chuggy
		if (  t.inputsys.activemouse == 1 ) 
		{
			//  constant update
			//if ( gbWelcomeSystemActive == false )
			//{
			SyncRate ( 0 ); SyncMask ( 1 ); Sync ( ); SleepNow ( 5 );
			//}
		}
		else
		{
			//  check for PAINT message
			#ifdef FPSEXCHANGE
			OpenFileMap (  3, "FPSEXCHANGE" );
			SetEventAndWait (  3 );
			if (  GetFileMapDWORD( 3, 60 ) == 1 ) 
			{
				SetFileMapDWORD (  3,60,0  ); t.syncthreetimes=3;
				SetEventAndWait (  3 );
			}
			#endif
			SyncRate (  0 );
		
			if (  t.syncthreetimes>0 ) {  --t.syncthreetimes; Sync ( ); }
	
			SleepNow ( 10 );
		}

		//  Detect if resolution changed (windows)
		editor_detect_invalid_screen ( );
	}
	
	#ifdef VRTECH
	if (g_bCascadeQuitFlag) 
	{
		int iRet = AskSaveBeforeNewAction();
		if (iRet == 2)
		{
			g_bCascadeQuitFlag = false;
		}
		else {
			PostQuitMessage(0);
		}
	}
	#endif
}

void mapeditorexecutable_finish(void)
{
	// End map editor program (moved above chdir and pref writes)
	common_justbeforeend();

	// Come out of Files folder
	SetCurrentDirectoryA(g.fpscrootdir_s.Get());

#ifdef ENABLEIMGUI
	if (t.game.set.ismapeditormode == 1) {

		// seems this command no longer detecting if user maximised the window? Rely on settings set elsewhere (avoids always setting to minimized)
		//if (IsZoomed(g_pGlob->hWnd)) {
		//	pref.iMaximized = 1;
		//}
		//else {
		//	pref.iMaximized = 0;
		//}

		cstr prefile = defaultWriteFolder;
		prefile += "gameguru.pref";

		FILE* preffile = GG_fopen(prefile.Get(), "wb+");
		if (preffile) {
			fwrite(&pref, 1, sizeof(pref), preffile);
			fclose(preffile);
		}

		if (pref.save_layout) {
			char cmLayoutFile[MAX_PATH];
			sprintf(cmLayoutFile, "%suiv3.layout", defaultWriteFolder);
			ImGui::SaveIniSettingsToDisk(cmLayoutFile);
		}
	}
#endif

	#ifdef VRTECH
	// ensure this gets called when leave software too
	//extern HMODULE hGGWMRDLL;
	//PE: GGVR_DeleteHolographicSpace identifier not found.
	//	if( hGGWMRDLL) //PE: Only if dll is loaded.
	//		GGVR_DeleteHolographicSpace;
	#endif

	// final exit
	ExitProcess ( 0 );
}

void mapeditorexecutable(void)
{
	mapeditorexecutable_init();

	// main loop
	#ifdef ENABLEIMGUI
	while (!g_bCascadeQuitFlag)
	#else
	#ifdef VRTECH
	// start thread loader for Character Creator texture files (multi-threaded loading) (saves 2s if started CCP)
	timestampactivity(0, "preload CCP textures early");
	charactercreatorplus_preloadinitialcharacter();
	#endif
	while ( 1 )
	#endif
	{
		mapeditorexecutable_loop();
	}
	mapeditorexecutable_finish();
}

#ifdef ENABLEIMGUI
int AskSaveBeforeNewAction(void)
{
	int iAction = 0;
	if (g.projectmodified == 1)
	{
		iAction = askBoxCancel("Do you wish to save first?", "Confirmation"); //1==Yes 2=Cancel 0=No

		if (iAction == 1)
		{
			//  yes save first
			if (g.projectfilename_s == "")
			{
				t.returnstring_s = "";
				cStr tOldDir = GetDir();
				char * cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_SAVE, "fpm\0*.fpm\0", g.mysystem.mapbankAbs_s.Get(), NULL, true);
				SetDir(tOldDir.Get());
				if (cFileSelected && strlen(cFileSelected) > 0) {
					t.returnstring_s = cFileSelected;
				}
				if (t.returnstring_s != "")
				{
					if (cstr(Lower(Right(t.returnstring_s.Get(), 4))) != ".fpm")  t.returnstring_s = t.returnstring_s + ".fpm";
					g.projectfilename_s = t.returnstring_s;
					bool oksave = true;
					if (FileExist(g.projectfilename_s.Get())) {
						oksave = overWriteFileBox(g.projectfilename_s.Get());
					}
					if (oksave) {
						gridedit_save_map();
					}
				}
			}
			else
			{
				gridedit_save_map();
			}

			g.projectmodified = 0; gridedit_changemodifiedflag();
			g.projectmodifiedstatic = 0;
		}
	}
	return iAction;

}
#endif

void editor_detect_invalid_screen ( void )
{
	if (  GetDisplayInvalid() != 0 || MatrixExist(g.m4_projection) == 0 ) 
	{
		if (  1 ) 
		{
			#ifdef FPSEXCHANGE
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetFileMapDWORD (  1, 900, 1 );
			SetFileMapString (  1, 1256, t.strarr_s[622].Get() );
			SetFileMapString (  1, 1000, t.strarr_s[623].Get() );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD(1, 900) == 1 ) 
			{
				SetEventAndWait (  1 );
			}
			t.tokay=GetFileMapDWORD(1, 904);
			if (  t.tokay == 1 ) 
			{
				//  no references to 3D objects (all gone now)
				t.recoverdonotuseany3dreferences=1;
				//  save now
				gridedit_save_map_ask ( );
			}
			//  call a new map editor
			OpenFileMap (  2, "FPSEXCHANGE" );
			SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
			SetFileMapString (  2, 1256, "-r" );
			SetFileMapDWORD (  2, 994, 0 );
			SetFileMapDWORD (  2, 924, 1 );
			SetEventAndWait (  2 );
			#endif
			//  free steam to unload the module in effect
			mp_free ( );
			//  end this old mapeditor
			common_justbeforeend();
			ExitProcess ( 0 );
		}
	}
}

void editor_showhelppage ( int iHelpType )
{
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
	return;
	#else

	// image to use
	int iEditorHelpImage = 1;
	int iDivideBy = 1;
	#ifdef VRTECH
	 switch ( iHelpType )
	 {
		case 1 : iEditorHelpImage = 1; break;
		case 2 : iEditorHelpImage = 27; iDivideBy = 2; break;
		case 3 : iEditorHelpImage = 28; iDivideBy = 2; break;
	 }
	#else
	 switch (iHelpType)
	 {
	  case 1: iEditorHelpImage = 1; break;
	  case 11: iEditorHelpImage = 27; break;
	 }
	#endif
	#ifdef FPSEXCHANGE
	 OpenFileMap (  1, "FPSEXCHANGE" );
	 SetEventAndWait (  1 );
	#endif
	do
	{
		t.inputsys.mclick=MouseClick();
		FastSync (  );
	} while ( !(  t.inputsys.mclick == 0 ) );
	t.inputsys.kscancode=0;
	t.asx_f=1.0;
	t.asy_f=1.0;
	t.imgx_f=ImageWidth(g.editorimagesoffset+iEditorHelpImage)/iDivideBy*t.asx_f;
	t.imgy_f=ImageHeight(g.editorimagesoffset+iEditorHelpImage)/iDivideBy*t.asy_f;
	Sprite (  123,-10000,-10000,g.editorimagesoffset+iEditorHelpImage );
	SizeSprite (  123,t.imgx_f,t.imgy_f );
	t.lastmousex=MouseX() ; t.lastmousey=MouseY();
	t.tpressf1toleave=0;
	float fImgTopLeftX = (GetChildWindowWidth(0) - t.imgx_f) / 2;
	float fImgTopLeftY = (GetChildWindowHeight(0) - t.imgy_f) / 2;
	while (  t.inputsys.kscancode == 0 && EscapeKey() == 0 ) 
	{
		#ifdef FPSEXCHANGE
		 t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
		#else
		 t.inputsys.kscancode = ScanCode();
		#endif
		if (  t.inputsys.kscancode == 0 )
		{
			if (  t.tpressf1toleave == 1  )  t.tpressf1toleave = 2;
		}
		if (  t.inputsys.kscancode == 112 ) 
		{
			if (  t.tpressf1toleave == 0  )  t.tpressf1toleave = 1;
			if (  t.tpressf1toleave == 2 ) 
			{
				//  allow the F1 press through to exit
			}
			else
			{
				t.inputsys.kscancode=0;
			}
		}
		#ifdef FPSEXCHANGE
		 if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
		 if (GetFileMapDWORD(1, 20) != 0) break;
		 if (  GetFileMapDWORD( 1, 516 )>0  )  break;
		#endif
		t.terrain.gameplaycamera=0;
		terrain_shadowupdate ( );
		terrain_update ( );
		PasteSprite (  123, fImgTopLeftX, fImgTopLeftY);
		Sync (  );
	}

	// clicking on left of MAX promo goes to special link
	if (iHelpType == 11)
	{
		// clicked here
		float fMouseX = GetFileMapDWORD(1, 0);
		float fMouseY = GetFileMapDWORD(1, 4);
		fMouseX = ((fMouseX + 0.0) / 800.0)*(GetDesktopWidth() + 0.0);
		fMouseY = ((fMouseY + 0.0) / 600.0)*(GetDesktopHeight() + 0.0);
		if (fMouseX >= fImgTopLeftX && fMouseX <= fImgTopLeftX+340)
		{
			ExecuteFile("https://bit.ly/3OpmZRE", "", "", 0);		 
		}
	}

	//  once we are in the help page, can use filemap to detect key (so F1 on/off issue does not occur)
	do
	{
		#ifdef FPSEXCHANGE
		 t.inputsys.kscancode = GetFileMapDWORD(1, 100);
		#else
		 t.inputsys.kscancode = ScanCode();
		#endif
		FastSync (  );
	} while ( !(  t.inputsys.kscancode == 0 ) );
	//PE: Make sure we dont sent mouse input to whatever is below page.
	do
	{
		t.inputsys.mclick = MouseClick();
		FastSync();
	} while (!(t.inputsys.mclick == 0));
	#endif
}

void editor_showparentalcontrolpage ( void )
{
	// allow parental control to be activated and deactivated
	#ifdef FPSEXCHANGE
	 OpenFileMap (  1, "FPSEXCHANGE" );
	 SetEventAndWait (  1 );
	#endif
	do
	{
		set_inputsys_mclick(MouseClick());// t.inputsys.mclick = MouseClick();
		FastSync (  );
	} 
	while ( !(  t.inputsys.mclick == 0 ) );
	t.inputsys.kscancode=0;
	t.asx_f=1.0;
	t.asy_f=1.0;
	t.imgx_f=ImageWidth(g.editorimagesoffset+8)*t.asx_f;
	t.imgy_f=ImageHeight(g.editorimagesoffset+8)*t.asy_f;
	Sprite (  123,-10000,-10000,g.editorimagesoffset+8 );
	SizeSprite (  123,t.imgx_f,t.imgy_f );
	t.lastmousex=MouseX() ; t.lastmousey=MouseY();
	t.tpressf1toleave=0;
	int iStayInParentalControlDialog = 1;
	cstr ParentFourDigitCode = "";
	int digitcode[5];
	digitcode[0] = 0;
	digitcode[1] = 0;
	digitcode[2] = 0;
	digitcode[3] = 0;
	int digitcodeindex = 0;
	int tpressedbefore = 0;
	bool bParentalToggleForcesQuit = false;
	while ( iStayInParentalControlDialog == 1 && t.inputsys.kscancode != 27 ) 
	{
		#ifdef FPSEXCHANGE
		 t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
		 if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
		 if (  GetFileMapDWORD( 1, 516 )>0  )  break;
		#else
		 t.inputsys.kscancode = ScanCode();
		#endif
		int tnewkeycode = 0; // numpad detection
		if ( t.inputsys.kscancode == 45 ) tnewkeycode = 48;
		if ( t.inputsys.kscancode == 35 ) tnewkeycode = 49;
		if ( t.inputsys.kscancode == 40 ) tnewkeycode = 50;
		if ( t.inputsys.kscancode == 34 ) tnewkeycode = 51;
		if ( t.inputsys.kscancode == 37 ) tnewkeycode = 52;
		if ( t.inputsys.kscancode == 12 ) tnewkeycode = 53;
		if ( t.inputsys.kscancode == 39 ) tnewkeycode = 54;
		if ( t.inputsys.kscancode == 36 ) tnewkeycode = 55;
		if ( t.inputsys.kscancode == 38 ) tnewkeycode = 56;
		if ( t.inputsys.kscancode == 33 ) tnewkeycode = 57;
		if ( tnewkeycode > 0 )
		{
			t.inputsys.kscancode = tnewkeycode;
		}
		if ( t.inputsys.kscancode >= 48 && t.inputsys.kscancode <= 57 )
		{
			if ( tpressedbefore == 0 && digitcodeindex < 4 )
			{
				if ( g.quickparentalcontrolmode == 1 )
				{
					// enable lock - entering password
					digitcode[digitcodeindex] = t.inputsys.kscancode;
					digitcodeindex++;
					ParentFourDigitCode = ParentFourDigitCode + "*";
					tpressedbefore = 1;
					if ( digitcodeindex == 4 )
					{
						g.quickparentalcontrolmode = 2;
						g.quickparentalcontrolmodepassword[0] = digitcode[0];
						g.quickparentalcontrolmodepassword[1] = digitcode[1];
						g.quickparentalcontrolmodepassword[2] = digitcode[2];
						g.quickparentalcontrolmodepassword[3] = digitcode[3];
						iStayInParentalControlDialog = 0;
						bParentalToggleForcesQuit = true;
						break;
					}
				}
				if ( g.quickparentalcontrolmode == 2 )
				{
					// disable lock - confirming password
					digitcode[digitcodeindex] = t.inputsys.kscancode;
					digitcodeindex++;
					ParentFourDigitCode = ParentFourDigitCode + "*";
					tpressedbefore = 1;
					if ( digitcodeindex == 4 )
					{
						// real password or secret backdoor
						bool bPasswordOkay = false;
						if (digitcode[0]==g.quickparentalcontrolmodepassword[0]
						&&	digitcode[1]==g.quickparentalcontrolmodepassword[1]
						&&	digitcode[2]==g.quickparentalcontrolmodepassword[2]
						&&	digitcode[3]==g.quickparentalcontrolmodepassword[3] ) bPasswordOkay = true;
						if (digitcode[0]==57 // 9119
						&&	digitcode[1]==49
						&&	digitcode[2]==49
						&&	digitcode[3]==57 ) bPasswordOkay = true;
						if ( bPasswordOkay == true )
						{
							g.quickparentalcontrolmode = 1;
							iStayInParentalControlDialog = 0;
							bParentalToggleForcesQuit = true;
							break;
						}
						else
						{
							// try again
							ParentFourDigitCode = "";
							digitcodeindex = 0;
						}
					}
				}

			}
		}
		else
		{
			tpressedbefore = 0;
		}

		t.terrain.gameplaycamera=0;
		terrain_shadowupdate ( );
		terrain_update ( );
		int iDialogTop = (GetChildWindowHeight(0)-t.imgy_f)/2;
		PasteSprite ( 123, (GetChildWindowWidth(0)-t.imgx_f)/2, iDialogTop );
		LPSTR pRCMTitle = "RESTRICTED CONTENT MODE : OFF";
		if ( g.quickparentalcontrolmode == 2 ) pRCMTitle = "RESTRICTED CONTENT MODE : ON";
		pastebitmapfontcenter ( pRCMTitle, GetChildWindowWidth(0)/2, iDialogTop + 20, 3, 255 );
		pastebitmapfontcenter ( "This feature will control visibility of restricted content such as", GetChildWindowWidth(0)/2, iDialogTop + 70, 1, 255 );
		pastebitmapfontcenter ( "blood, violence and gore which may be offensive to some users.", GetChildWindowWidth(0)/2, iDialogTop + 95, 1, 255 );
		pastebitmapfontcenter ( "If you do not want this, press escape now.", GetChildWindowWidth(0)/2, iDialogTop + 120, 1, 255 );
		pastebitmapfontcenter ( ParentFourDigitCode.Get(), GetChildWindowWidth(0)/2, iDialogTop + (t.imgy_f/2), 4, 255 );
		if ( g.quickparentalcontrolmode == 2 )
		{
			pastebitmapfontcenter ( "ENTER YOUR FOUR DIGIT PASSWORD TO DEACTIVATE CONTENT LOCK", GetChildWindowWidth(0)/2, iDialogTop + t.imgy_f - 70, 2, 255 );
			pastebitmapfontcenter ( "OR PRESS [ESCAPE] TO CANCEL", GetChildWindowWidth(0)/2, iDialogTop + t.imgy_f - 50, 2, 255 );
		}
		else
		{
			pastebitmapfontcenter ( "ENTER FOUR DIGIT PASSWORD TO ACTIVATE CONTENT LOCK", GetChildWindowWidth(0)/2, iDialogTop + t.imgy_f - 70, 2, 255 );
			pastebitmapfontcenter ( "OR PRESS [ESCAPE] TO ENTER REGULAR MODE", GetChildWindowWidth(0)/2, iDialogTop + t.imgy_f - 50, 2, 255 );
		}
		pastebitmapfontcenter ( "YOU CAN ACCESS THIS OPTION AGAIN FROM THE HELP MENU", GetChildWindowWidth(0)/2, iDialogTop + t.imgy_f - 30, 2, 255 );
		Sync ( );
	}
	do
	{
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
		FastSync (  );
	} 
	while ( (  t.inputsys.kscancode > 3 ) ); //PE: We can keep getting virtual keys <= 3.
	//PE: Make sure we dont sent mouse input to whatever is below page.
	do
	{
		set_inputsys_mclick(MouseClick());// t.inputsys.mclick = MouseClick();
		FastSync();
	} while (!(t.inputsys.mclick == 0));

	// only a mode of 2 carries the digit code for activated
	if ( g.quickparentalcontrolmode != 2 )
	{
		digitcode[0] = 0;
		digitcode[1] = 0;
		digitcode[2] = 0;
		digitcode[3] = 0;
	}

	// 050416 - flag file to control parental control mode
	t.tfile_s=g.fpscrootdir_s+"\\parentalcontrolmode.ini";
	DeleteAFile (  t.tfile_s.Get() );
	if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
	OpenToWrite (  1,t.tfile_s.Get() );
	WriteString (  1, cstr(g.quickparentalcontrolmode).Get() );
	WriteByte (  1, digitcode[0] );
	WriteByte (  1, digitcode[1] );
	WriteByte (  1, digitcode[2] );
	WriteByte (  1, digitcode[3] );
	CloseFile (  1 );
	t.tfile_s = g.fpscrootdir_s+"\\parentalcontrolactive.ini";
	if ( g.quickparentalcontrolmode == 2 )
	{
		// ensure file exists for IDE benefit
		OpenToWrite ( 1,t.tfile_s.Get() );
		WriteString ( 1, "123" );
		CloseFile ( 1 );
	}
	else
	{
		// delete this file to show IDE no parental control in effect
		DeleteAFile (  t.tfile_s.Get() );
	}

	// force the product to quit if change parental control setting
	if ( bParentalToggleForcesQuit == true )
	{
		MessageBoxA ( GetForegroundWindow(), "In order for the restricted content mode chosen to take effect, you must exit GameGuru and restart", "GameGuru Restart", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST );
	}
}

void editor_freezeanimations ( void )
{
	// go through all objects and freeze their animations
	if ( t.fStoreObjAnimSpeeds==NULL )
	{
		t.fStoreObjAnimSpeeds = new float[210000];
		for ( int iObj = 1; iObj < 210000; iObj++ )
		{
			if ( ObjectExist ( iObj )==1 )
			{
				sObject* pObject = GetObjectData ( iObj );
				t.fStoreObjAnimSpeeds [ iObj ] = pObject->fAnimSpeed;
				pObject->fAnimSpeed = 0.0f;
			}
		}
	}
}

void editor_unfreezeanimations ( void )
{
	// go through all objects and restore all animation speeds from freeze step above
	if ( t.fStoreObjAnimSpeeds )
	{
		for ( int iObj = 1; iObj < 210000; iObj++ )
		{
			if ( ObjectExist ( iObj )==1 )
			{
				sObject* pObject = GetObjectData ( iObj );
				pObject->fAnimSpeed = t.fStoreObjAnimSpeeds [ iObj ];
			}
		}
		delete t.fStoreObjAnimSpeeds;
		t.fStoreObjAnimSpeeds = NULL;
	}
}

void editor_showquickstart ( int iForceMainOpen )
{
	// open welcome system
	editor_freezeanimations();
	if ( gbWelcomeSystemActive == false )
	{
		welcome_init(1);
		welcome_staticbackdrop();
		welcome_init(2);
	}
	welcome_init(0);

	// if first time run for VRQ
	if ( g.vrqTriggerSerialCodeEntrySystem == 1 )
	{
		#ifdef ALPHAEXPIRESYSTEM
		 PostQuitMessage(0);
		#else
		 welcome_show(WELCOME_SERIALCODE);
		#endif
	}
	else
	{
		// Welcome system not syncronous any more
		//if (g.iTriggerSoftwareToQuit != 0)
		//{
		//	welcome_show(WELCOME_EXITAPP);
		//}
		//else
		{
			if (g.iFreeVersionModeActive == 1)
			{
				welcome_show(WELCOME_FREEINTROAPP);
			}
			if (g.iFreeVersionModeActive == 2)
			{
				welcome_show(WELCOME_FREETRIALINTROAPP);
			}
		}

		// if welcome not deactivated
		if (g.gshowonstartup != 0 || iForceMainOpen == 1)
		{
			// if no announcement wanting to share news
			if (g_iWelcomeLoopPage != WELCOME_ANNOUNCEMENTS)
			{
				// start welcome page
				if (g.vrqcontrolmode == 0)
					welcome_show(WELCOME_MAIN);
				else
					welcome_show(WELCOME_MAINVR);
			}
			if (strlen(t.tlevelautoload_s.Get()) > 0)
			{
				//Trigger load level.
				welcome_free();
				t.tlevelautoload_s = "";
			}
		}
	}

	//  reset before leave
	t.inputsys.kscancode=0;
	set_inputsys_mclick(0);// t.inputsys.mclick = 0;
	t.inputsys.xmouse=0;
	t.inputsys.ymouse=0;
}

void editor_previewmapormultiplayer_initcode ( int iUseVRTest )
{
	//  store if project modified
	t.storeprojectmodified=g.projectmodified;
	g_tstoreprojectmodifiedstatic = g.projectmodifiedstatic; 

	//  flag that we clicked TEST GAME
	t.interactive.testgameused=1;

	g.tabmodehidehuds = 0; //Enable HUD if lua disabled it in prev session.

	//  Before launch test game, check if enough contiguous
	checkmemoryforgracefulexit();

#ifdef ENABLEIMGUI
#ifndef USEOLDIDE
	//PE: Test game mode.
//	extern DWORD gWindowSizeX;
//	extern DWORD gWindowSizeY;
	extern DWORD gWindowVisible;

	gWindowSizeXOld = GetChildWindowWidth(-1);
	gWindowSizeYOld = GetChildWindowHeight(-1);

	RECT rect = { NULL };
	GetWindowRect(g_pGlob->hWnd, &rect);

	gWindowPosXOld = rect.left;
	gWindowPosYOld = rect.top;

	gWindowVisibleOld = gWindowVisible; //SW_MAXIMIZE
	if (IsZoomed(g_pGlob->hWnd))
		gWindowMaximized = 1;
	else
		gWindowMaximized = 0;
#endif
#endif

	//  First call will toggle keyboard/mouse back to BACKGROUND (to capture all direct data)
	SetWindowModeOn (  );

#ifdef ENABLEIMGUI
#ifndef USEOLDIDE
	//PE: Test game mode.
	SetWindowSettings(0, 0, 0);
	SetWindowPos(g_pGlob->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetForegroundWindow(g_pGlob->hWnd);
	SetWindowSize(GetDesktopWidth(), GetDesktopHeight());
	ShowWindow(); MaximiseWindow();

	//Hide any windows outside main viewport.
	ImGui::HideAllViewPortWindows();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	//SetWindowLong(g_pGlob->hWnd, GWLP_WNDPROC, (LONG)WindowProc);

#endif
#endif

	//  center mouse pointer in editor (and hide it)
	game_hidemouse ( );

	//  hide widget if was highlighted when tested game
	widget_hide ( );

	// hide EBE if active when click test game button
	ebe_hide();

	// hide texture terrain painter panels
	terrain_paintselector_hide();

	//  switch off any IDE entity highlighting

	editor_restoreentityhighlightobj ( );

	// switch off any rubber band entity highlighting
	gridedit_clearentityrubberbandlist();

	// 210917 - refresh HLSL shaders (flagged as doing shader work)
	if ( g.gforceloadtestgameshaders == 1 )
	{
		// go through all loaded effects and reload them
		for ( t.t = -5 ; t.t <= g.effectbankmax; t.t++ )
		{
			char pEffectFilename[1024];
			strcpy ( pEffectFilename, "effectbank\\reloaded\\" );
			if ( t.t == -5 ) 
			{ 
				t.tteffectid = t.terrain.effectstartindex+0; 
				if ( g.gpbroverride == 1 )
					strcat ( pEffectFilename, "apbr_terrain.fx"); 
				else
					strcat ( pEffectFilename, "terrain_basic.fx"); 
			}
			if ( t.t == -4 ) 
			{ 
				t.tteffectid = t.terrain.effectstartindex+2; 
				if ( g.gpbroverride == 1 )
					strcat ( pEffectFilename, "apbr_veg.fx"); 
				else
					strcat ( pEffectFilename, "vegetation_basic.fx"); 
			}
			if ( t.t == -3 ) 
			{ 
				t.tteffectid = g.thirdpersonentityeffect; 
				if ( g.gpbroverride == 1 )
					strcat ( pEffectFilename, "apbr_basic.fx"); 
				else
					strcat ( pEffectFilename, "entity_basic.fx"); 
			}
			if ( t.t == -2 ) 
			{ 
				t.tteffectid = g.thirdpersoncharactereffect; 
				if ( g.gpbroverride == 1 )
					strcat ( pEffectFilename, "apbr_animwithtran.fx"); 
				else
					strcat ( pEffectFilename, "character_basic.fx"); 
			}
			if ( t.t == -1 ) { t.tteffectid = g.staticlightmapeffectoffset; strcat ( pEffectFilename, "static_basic.fx"); }
			if ( t.t == 0 ) { t.tteffectid = g.staticshadowlightmapeffectoffset; strcat ( pEffectFilename, "shadow_basic.fx"); }
			if ( t.t > 0 ) { t.tteffectid = g.effectbankoffset+t.t; strcpy ( pEffectFilename, t.effectbank_s[t.t].Get()); }
			if ( GetEffectExist ( t.tteffectid ) == 1 ) 
			{
				// gather all objects that use this effect
				int iObjListMax = 0;
				DWORD** pObjList = new DWORD* [ g_iObjectListCount ];
				memset ( pObjList, 0, sizeof(DWORD*)*g_iObjectListCount );
				for ( DWORD dwObject = 0; dwObject < (DWORD)g_iObjectListCount; dwObject++ )
				{
					sObject* pObject = g_ObjectList [ dwObject ];
					if ( pObject )
					{ 
						bool bAnyMeshUsingEffect = false;
						for ( DWORD dwMesh = 0; dwMesh < (DWORD)pObject->iMeshCount; dwMesh++ )
						{
							if ( pObject->ppMeshList [ dwMesh ]->pVertexShaderEffect == m_EffectList [ t.tteffectid ]->pEffectObj )
							{
								bAnyMeshUsingEffect = true;
							}
						}
						if ( bAnyMeshUsingEffect == true )
						{
							DWORD* pPerObjData = new DWORD[1+pObject->iMeshCount];
							memset ( pPerObjData, 0, sizeof(DWORD)*(1+pObject->iMeshCount) );
							pObjList[iObjListMax] = pPerObjData;
							*(pPerObjData+0) = dwObject;
							DWORD dwObjDataIndex = 1;
							for ( DWORD dwFrameIndex = 0; dwFrameIndex < (DWORD)pObject->iFrameCount; dwFrameIndex++ )
							{
								if ( pObject->ppFrameList [ dwFrameIndex ]->pMesh )
								{
									if ( pObject->ppFrameList [ dwFrameIndex ]->pMesh->pVertexShaderEffect == m_EffectList [ t.tteffectid ]->pEffectObj )
									{
										*(pPerObjData+dwObjDataIndex) = 1+dwFrameIndex;
										dwObjDataIndex++;
									}
								}
							}
							iObjListMax++;
						}
					}
				}

				// delete the old effect and load a new one
				DeleteEffect ( t.tteffectid );
				LoadEffect ( pEffectFilename, t.tteffectid, 0 );
				filleffectparamarray ( t.tteffectid );

				// set the new effects to each object in the list
				if ( iObjListMax > 0 )
				{
					for ( int iObjListIndex = 0; iObjListIndex < iObjListMax; iObjListIndex++ )
					{
						DWORD* pPerObjData = pObjList[iObjListIndex];
						DWORD dwObject = *(pPerObjData+0);
						sObject* pObject = g_ObjectList [ dwObject ];
						if ( pObject )
						{
							for ( DWORD dwObjDataIndex = 0; dwObjDataIndex < (DWORD)pObject->iMeshCount; dwObjDataIndex++ )
							{
								DWORD dwFrameIndex = *(pPerObjData+1+dwObjDataIndex);
								if ( dwFrameIndex > 0 )
								{
									dwFrameIndex--;
									SetLimbEffect ( dwObject, dwFrameIndex, t.tteffectid );
								}
							}
						}
						SAFE_DELETE(pPerObjData);
					}
				}
				SAFE_DELETE(pObjList);

				// by default, set to first technique
				SetEffectTechnique ( t.tteffectid, NULL );
			}
		}

		// also reestablish links with constant vars for terrain and veh shaders
		//t.terrain.iForceTerrainVegShaderUpdate = 1;
		// re-assign params for reloaded terrain and veg
		terrain_applyshader();
		grass_applyshader();
	}
	
	//  set-up test game screen prompt assets
	if ( t.game.runasmultiplayer == 1 ) 
	{
		loadscreenpromptassets(2);
		#ifdef PRODUCTV3
		 printscreenprompt("ENTERING SOCIAL VR");
		#else
		 printscreenprompt("ENTERING MULTIPLAYER MODE");
		#endif
	}
	else
	{
		loadscreenpromptassets(iUseVRTest);
		printscreenprompt("LAUNCHING TEST LEVEL");
	}

	//  Save editor configuration
	timestampactivity(0,"PREVIEWMAP: Save config");
	editor_savecfg ( );

	// And save the level to levelbank\testmap before launch preview
	// so we can restore to this level if it crashes
	g.gpretestsavemode=1;
	gridedit_save_test_map ( );
	g.gpretestsavemode=0;

	// Now saves all part-files into temp FPM file (which multiplayer can pick up later)
	if ( t.game.runasmultiplayer == 1 ) 
	{
		//  save temp copy of current level
		g.projectfilename_s=g.mysystem.editorsGrideditAbs_s+"worklevel.fpm";//g.fpscrootdir_s+"\\Files\\editors\\gridedit\\worklevel.fpm";
		editor_savecfg ( );
		mapfile_saveproject_fpm ( );
	}

	// GCStore could have assed assets since the last 'test game' so refresh internal lists
	sky_init ( );
	terrain_initstyles ( );
	grass_initstyles();

	// Re-acquire indices now the lists have changed
	// takes visuals.sky$ visuals.terrain$ visuals.vegetation$
	visuals_updateskyterrainvegindex ( );

	// Ensure game visuals settings used
	t.gamevisuals.skyindex=t.visuals.skyindex;
	t.gamevisuals.sky_s=t.visuals.sky_s;
	t.gamevisuals.lutindex = t.visuals.lutindex;
	t.gamevisuals.lut_s = t.visuals.lut_s;
	t.gamevisuals.terrainindex=t.visuals.terrainindex;
	t.gamevisuals.terrain_s=t.visuals.terrain_s;
	t.gamevisuals.vegetationindex=t.visuals.vegetationindex;
	t.gamevisuals.vegetation_s=t.visuals.vegetation_s;
	t.gamevisuals.iEnvironmentWeather = t.visuals.iEnvironmentWeather;


	// copy game visuals to visuals for use in level play
	t.visuals=t.gamevisuals;
	t.visuals.refreshshaders=1;
	t.visuals.refreshvegtexture=1;

	// Hide camera while prepare test map
	t.storecx_f=CameraPositionX();
	t.storecy_f=CameraPositionY();
	t.storecz_f=CameraPositionZ();

	// default start position is edit-camera XZ
	t.terrain.playerx_f = CameraPositionX(0);
	t.terrain.playerz_f = CameraPositionZ(0);
	if (t.terrain.TerrainID > 0)
	{
		t.terrain.playery_f = BT_GetGroundHeight(t.terrain.TerrainID, t.terrain.playerx_f, t.terrain.playerz_f) + 150.0;
	}
	else
	{
		t.terrain.playery_f = g.gdefaultterrainheight + 150.0;
	}
	t.terrain.playerax_f = 0.0;
	t.terrain.playeray_f = 0.0;
	t.terrain.playeraz_f = 0.0;

	// store all editor entity positions and rotations
	//timestampactivity(0, "t.storedentityelementlist:");
	t.storedentityelementlist=g.entityelementlist;
	t.storedentityviewcurrentobj=g.entityviewcurrentobj;
	Dim (  t.storedentityelement,g.entityelementlist );
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.storedentityelement[t.e]=t.entityelement[t.e];
	}
	//timestampactivity(0, "t.storedentityelementlist DONE:");

	// hide all markers
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		t.obj=t.entityelement[t.e].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1 ) 
			{
				if (  t.entityprofile[t.entid].ismarker != 0 ) 
				{
					//  all markers must be hidden
					HideObject (  t.obj );
				}
				if (  t.entityprofile[t.entid].addhandlelimb>0 ) 
				{
					//  hide decal handles
					HideLimb (  t.obj,t.entityprofile[t.entid].addhandlelimb );
				}
			}
		}
	}

	// ensure all locked entity transparency resolves
	for ( t.tte = 1 ; t.tte<=  g.entityelementlist; t.tte++ )
	{
		if ( t.entityelement[t.tte].editorlock == 1 || t.entityelement[t.tte].underground == 1 ) 
		{
			t.tobj=t.entityelement[t.tte].obj;
			if ( t.tobj>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					if ( t.entityelement[t.tte].underground == 1  )  t.entityelement[t.tte].isclone = 1;
					entity_converttoinstance ( );
				}
			}
		}
	}

	//PE: start any animations that are not in editor mode.
	for (t.tte = 1; t.tte <= g.entityelementlist; t.tte++)
	{
		t.entid = t.entityelement[t.tte].bankindex;
		t.tttsourceobj = g.entitybankoffset + t.entityelement[t.tte].bankindex;
		t.tobj = t.entityelement[t.tte].obj;
		if (t.tobj > 0)
		{
			if (ObjectExist(t.tobj) == 1)
			{
				//PE: Possible fix for issues:
				//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/206
				//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/273
				//PE: need testing.
				if (t.entityprofile[t.entid].ischaracter == 1) {
					//Char should always have z depth , but somehow its removed somewhere.
					EnableObjectZDepth(t.tobj);
				}


				//FULLBOUNDS
				if (t.entityprofile[t.entid].startanimingame > 0) {
					if (t.entityprofile[t.entid].animmax > 0) {
						t.q = t.entityprofile[t.entid].startanimingame - 1;
						SetObjectFrame(t.tttsourceobj, 0);
						LoopObject(t.tttsourceobj, t.entityanim[t.entid][t.q].start, t.entityanim[t.entid][t.q].finish);
						SetObjectFrame(t.tobj, 0);
						LoopObject(t.tobj, t.entityanim[t.entid][t.q].start, t.entityanim[t.entid][t.q].finish);
					}
				}
				else {

				}
			}
		}
	}

	// hide all waypoints and zones
	waypoint_hideall ( );

	// hide editor objects too
	for ( t.obj = t.editor.objectstartindex+1; t.obj <= t.editor.objectstartindex+1+10 ;  t.obj++ ) //?//t.editor.objectstartindex+10;
	{
		if ( ObjectExist(t.obj) == 1 ) 
		{
			HideObject (  t.obj );
		}
	}

	// hide any EBE site markers (limb zeros)
	for ( t.tte = 1; t.tte <= g.entityelementlist; t.tte++ )
	{
		int iIndex = t.entityelement[t.tte].bankindex;
		if ( t.entityprofile[iIndex].isebe != 0 ) 
		{
			t.tobj = t.entityelement[t.tte].obj;
			if ( t.tobj>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					HideLimb ( t.tobj, 0 );
				}
			}
		}
	}

	// ensure no collision from legacy engine
	AutomaticCameraCollision (  0,0,0 );
	SetGlobalCollisionOff (  );

	// Setup game view camera
	SetCameraFOV ( 75 );
	g.grav_f=-5.0;

	// store original terrain heights
	if ( t.terrain.TerrainID>0 ) 
	{
		for ( t.z = 0 ; t.z<=  1024; t.z++ )
		{
			for ( t.x = 0 ; t.x<=  1024; t.x++ )
			{
				t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,t.x*50.0,t.z*50.0,1);
				t.terrainmatrix[t.x][t.z]=t.h_f;
			}
		}
	}

	// Create heightmap from this terrain (for quad reduction)
	if ( t.terrain.TerrainID>0 ) 
	{
		t.terrain.terrainregionupdate=0;
		terrain_refreshterrainmatrix ( );
		t.theightfile_s=g.mysystem.levelBankTestMap_s+"heightmap.dds"; //"levelbank\\testmap\\heightmap.dds";
		terrain_createheightmapfromheightdata ( );
	}

	// full speed
	SyncRate ( 0 );

	// Work out the amount of memory used for the TEST GAME session
	t.tmemorybeforetestgame=SMEMAvailable(1);

	#ifdef VRTECH
	// in VR, if controller powered down, need to jog it back to life
	if ( g.vrglobals.GGVREnabled == 2 )
	{
		//PE: GGVR_ReconnectWithHolographicSpaceControllers(); identifier not found.
//		GGVR_ReconnectWithHolographicSpaceControllers();
	}
	#endif

	//
	// launch game root with IDE 'test at cursor position' settings
	//
	//t.game.gameisexe=0; only set right at start!
	t.game.set.resolution=0;
	t.game.set.initialsplashscreen=0;
	t.game.set.ismapeditormode=0;
	extern int tgamesetismapeditormode;
	tgamesetismapeditormode = 0;


	// game loop init code
	game_masterroot_initcode ( iUseVRTest );
}

bool editor_previewmapormultiplayer_loopcode ( int iUseVRTest )
{
	bool bEndThisLoop = false;
	#ifdef VRTECH
	g_bDisableQuitFlag = true;
	bEndThisLoop = game_masterroot_loopcode ( iUseVRTest );
	g_bDisableQuitFlag = false;
	#else
	game_masterroot ( 0 );
	bEndThisLoop = true;
	#endif
	return bEndThisLoop;
}

void editor_previewmapormultiplayer_afterloopcode ( int iUseVRTest )
{
	// game after loop code
	game_masterroot_afterloopcode ( iUseVRTest );

	t.terrain.skysundirectionx_f = t.terrain.sundirectionx_f;
	t.terrain.skysundirectiony_f = t.terrain.sundirectiony_f;
	t.terrain.skysundirectionz_f = t.terrain.sundirectionz_f;

	t.game.set.ismapeditormode=1;
	extern int tgamesetismapeditormode;
	tgamesetismapeditormode = 1;

	// Restore entities (remove light map objects for return to IDE editor)
	lm_restoreall ( );

	//PE: Hide any hit decals.
	decal_hide();

	// restore any EBE site markers (limb zeros)
	for ( t.tte = 1; t.tte <= g.entityelementlist; t.tte++ )
	{
		int iIndex = t.entityelement[t.tte].bankindex;
		if ( t.entityprofile[iIndex].isebe != 0 ) 
		{
			t.tobj = t.entityelement[t.tte].obj;
			if ( t.tobj>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					ShowLimb ( t.tobj, 0 );
				}
			}
		}
	}

	// Revert mode to only render NEAR technique
	visuals_restoreterrainshaderforeditor ( );
	BT_ForceTerrainTechnique ( 1 );

	// editor speed max
	SyncMask ( 1 );
	SyncRate ( 0 );

	//PE: release mouse so all monitors can be used.
	ClipCursor(NULL);

	// restore mouse pos and visbility
	game_showmouse ( );

	// prompt informing user we are saving the level changes
	if ( t.conkit.modified == 1 ) 
	{
		popup_text("Saving level changes");
	}

	//  show all waypoints and zones
	waypoint_restore ( );

	// 101115 - restore all characters to use regular character shader
	game_setup_character_shader_entities ( false );

	// if additional entities added, remove and restore orig count
	if ( g.entityelementlist>t.storedentityelementlist ) 
	{
		for ( t.e = t.storedentityelementlist+1 ; t.e<= g.entityelementlist ; t.e++ )
		{
			t.obj=t.entityelement[t.e].obj;
			if ( t.obj>0 ) 
			{
				if ( ObjectExist(t.obj) == 1 ) 
				{
					DeleteObject ( t.obj );
				}
			}
			t.entityelement[t.e].obj=0;
			t.entityelement[t.e].bankindex=0;
		}
		g.entityelementlist=t.storedentityelementlist;
		g.entityviewcurrentobj=t.storedentityviewcurrentobj;
	}

	// restore all editor entity positions and rotations
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.obj=t.entityelement[t.e].obj;
		if ( t.obj>0 ) 
		{
			if ( ObjectExist(t.obj) == 1 ) 
			{
				// only if still exists - could have been deleted
				t.entityelement[t.e]=t.storedentityelement[t.e];
			}
		}
	}
	UnDim ( t.storedentityelement );

	// restore entity positions and rotations
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		t.obj=t.entityelement[t.e].obj;
		if ( t.obj>0 ) 
		{
			if ( ObjectExist(t.obj) == 1 ) 
			{
				if ( t.entityprofile[t.entid].ismarker == 0 ) 
				{
					PositionObject (  t.obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
					RotateObject (  t.obj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
					//And scale.
					ScaleObject(t.obj, 100 + t.entityelement[t.e].scalex, 100 + t.entityelement[t.e].scaley, 100 + t.entityelement[t.e].scalez);
					ShowObject (  t.obj );
					EnableObjectZDepth(t.obj);
				}
				if ( t.entityprofile[t.entid].addhandlelimb>0 ) 
				{
					ShowLimb ( t.obj,t.entityprofile[t.entid].addhandlelimb );
				}
			}
		}
	}


	// show all markers
	t.gridentityhidemarkers=0;
	editor_updatemarkervisibility ( );

	// ensure all locked entity transparency resolves
	for ( t.tte = 1 ; t.tte<=  g.entityelementlist; t.tte++ )
	{
		if ( t.entityelement[t.tte].editorlock == 1 || t.entityelement[t.tte].underground == 1 ) 
		{
			t.tobj=t.entityelement[t.tte].obj;
			if ( t.tobj>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					//PE: Re-enable transparent on locked entities.
					if(t.entityelement[t.tte].editorlock == 1)
					{
							t.entityelement[t.tte].isclone = 0;
							entity_converttoclonetransparent();
					}
					else 
					{
						if (t.entityelement[t.tte].underground == 1) t.entityelement[t.tte].isclone = 1;
						entity_converttoinstance();
					}
				}
			}
		}
	}

	//PE: disable any animations that should not be in editor.
	for (t.tte = 1; t.tte <= g.entityelementlist; t.tte++)
	{
		t.entid = t.entityelement[t.tte].bankindex;
		t.tttsourceobj = g.entitybankoffset + t.entityelement[t.tte].bankindex;
		t.tobj = t.entityelement[t.tte].obj;
		if (t.tobj > 0)
		{
			if (ObjectExist(t.tobj) == 1)
			{
				if (t.entityprofile[t.entid].startanimingame > 0) 
				{
					if (t.entityprofile[t.entid].animmax > 0) 
					{
						t.q = 0;
						SetObjectFrame(t.tttsourceobj, 0);
						StopObject(t.tttsourceobj);
						SetObjectFrame(t.tobj, 0);
						StopObject(t.tobj);
					}
				}
			}
		}

		//PE: pframe is lost on clone objects, recreate.
		if (t.entityprofile[t.entid].ismarker == 0 && t.entityprofile[t.entid].isebe == 0)
		{
			if (t.entityelement[t.tte].isclone == 1 && t.entityelement[t.tte].underground == 0)
			{
				if (t.entityelement[t.tte].editorlock == 0)
				{
					//t.tobj = t.tentityobj; t.tte = t.tentitytoselect;
					entity_converttoinstance();
				}
			}
		}

		if (t.tobj > 0 && t.entityprofile[t.entid].ischaracter == 1) 
		{
			//Restore any character animations for editor.
			if (GetNumberOfFrames(t.tobj) > 0)
			{
				SetObjectFrame(t.tobj, 0);
				SetObjectFrame(t.tttsourceobj, 0);
				if (t.entityprofile[t.entid].animmax > 0 && t.entityprofile[t.entid].playanimineditor > 0 && t.entityprofile[t.entid].ischaractercreator == 0)
				{
					t.q = t.entityprofile[t.entid].playanimineditor - 1;
					LoopObject(t.tobj, t.entityanim[t.entid][t.q].start, t.entityanim[t.entid][t.q].finish);
					//PE: We are now a instance, so also set master object.
					LoopObject(t.tttsourceobj, t.entityanim[t.entid][t.q].start, t.entityanim[t.entid][t.q].finish);
				}
				else
				{
					LoopObject(t.tobj); StopObject(t.tobj);
					LoopObject(t.tttsourceobj); StopObject(t.tttsourceobj);
				}
			}

		}
		else 
		{
		}
	}

	// signal that we have finished Test Level, restore mapeditor windows
	#ifdef FPSEXCHANGE
	 OpenFileMap (  1, "FPSEXCHANGE" );
	 SetFileMapDWORD (  1, 970, 1 );
	 SetEventAndWait (  1 );
	#endif

	// LB101019 - this can cause a freeze due to DirectInput bug when screensaver/hybernate kicks in
	// as the key pressed when waking up the boot screen remains in the key buffer even though key not pressed
	// though does get cleared later somehow (replace DX Input at some point to avoid this issue)
	// wait until all mouse activity over and escape key released
	//while ( MouseClick() != 0 ) {}
	//while ( ScanCode() != 0 ) {}

	// Restore camera
	editor_restoreeditcamera ( );
	t.updatezoom=1;

	// restore object visibilities
	editor_refresheditmarkers ( );

	// 130320 - ensure water height change is not saved out to root (messes up when reload software; underwater)
	float fStoreWaterLevel = g.gdefaultwaterheight;
	g.gdefaultwaterheight = GGORIGIN_Y;

	// remember game states for next time
	visuals_save ( );

	//PE: Before doing this, we should make sure that t.visuals LUA changes is not part of t.visuals.
#ifdef PRODUCTCLASSIC
	//PE: Restore settings from sliders. so .lua changes is not included.
	t.slidersmenuindex = t.slidersmenunames.visuals;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.water;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.camera;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.posteffects;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.qualitypanel;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.worldpanel;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.graphicoptions;
	sliders_write(true);
	t.slidersmenuindex = t.slidersmenunames.shaderoptions;
	sliders_write(true);

#endif


	t.gamevisuals=t.visuals;

	// and restore as would otherwise interfere with ?
	g.gdefaultwaterheight = fStoreWaterLevel;

	// restore shader constants with editor visuals (and bring back some settings we want to retain)
	t.visuals=t.editorvisuals;
	t.visuals.skyindex=t.gamevisuals.skyindex;
	t.visuals.sky_s=t.gamevisuals.sky_s;
	t.visuals.lutindex = t.gamevisuals.lutindex;
	t.visuals.lut_s = t.gamevisuals.lut_s;
	t.visuals.terrainindex=t.gamevisuals.terrainindex;
	t.visuals.terrain_s=t.gamevisuals.terrain_s;
	t.visuals.vegetationindex=t.gamevisuals.vegetationindex;
	t.visuals.vegetation_s=t.gamevisuals.vegetation_s;
	t.visuals.iEnvironmentWeather = t.gamevisuals.iEnvironmentWeather;


	// and refresh assets based on restore
	t.visuals.refreshshaders=1;
	visuals_loop ( );
	visuals_shaderlevels_update ( );

	// use infinilights to show dynamic lighting in editor
	lighting_init ( );

	// Second call will toggle keyboard/mouse back to FOREGROUND
	SetWindowModeOn ( );

	#ifdef ENABLEIMGUI
	#ifndef USEOLDIDE
	//PE: Need to restore original settings.
	//PE: Setup the window here. pos size. Docking ?
	SetWindowSettings(5, 1, 1);
	SetForegroundWindow(g_pGlob->hWnd);
	SetWindowSize(gWindowSizeXOld+ gWindowSizeAddX, gWindowSizeYOld+ gWindowSizeAddY); //PE: test
	SetWindowPosition(gWindowPosXOld, gWindowPosYOld);
	ShowWindow();
	if (gWindowMaximized == 1 )
		MaximiseWindow();
	else
		RestoreWindow();

	//PE: enable outside windows again.
	ImGui::ShowAllViewPortWindows();
	LRESULT CALLBACK ImguiWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	#ifdef USERENDERTARGET
	//PE: keep it at current resolution for now.
	//SetCameraToImage(0, 21, OldrenderTargetSize.x, OldrenderTargetSize.y, 2);
	#endif
	#endif
	#endif

	// Close popup message
	if ( t.conkit.modified == 1 ) 
	{
		SleepNow ( 1000 );
		popup_text_close();
		t.conkit.modified=0;
	}

	// Ensure no terrain/entity editing carried back
	t.terrain.terrainpainteroneshot=0;

	// Set editor to use a true 1;1 pixel mapping for Text ( , Steam GUI and other overlay images )
	SetChildWindowTruePixel ( 1 );
	//common_refreshDisplaySize ( );

	// restore if project modified
	t.tignoreinvalidateobstacles=1;
	g.projectmodified = t.storeprojectmodified ; if ( g.projectmodified == 1 ) gridedit_changemodifiedflag ( );
	g.projectmodifiedstatic = g_tstoreprojectmodifiedstatic; 
	t.tignoreinvalidateobstacles=0;

	// Something is clipping objects when returning to editor
	editor_loadcfg();
	editor_refreshcamerarange();
}

void editor_previewmapormultiplayer(int iUseVRTest)
{
	// for non-MAX scenarios (single function call)
	bool bRunLoop = true;
	editor_previewmapormultiplayer_initcode(iUseVRTest);
	while ( bRunLoop == true)
	{
		if (editor_previewmapormultiplayer_loopcode(iUseVRTest) == true) bRunLoop = false;
	}
	editor_previewmapormultiplayer_afterloopcode(iUseVRTest);
	t.postprocessings.fadeinvalue_f = 1.0f;
}

void editor_multiplayermode ( void )
{
	// check we are not in the importer or character creator
	editor_checkIfInSubApp ( );

	//  Record last edited project
	t.storeprojectfilename_s=g.projectfilename_s;

	//  Set multiplayer flags here
	t.game.runasmultiplayer=1;
	#ifdef VRTECH
	editor_previewmapormultiplayer ( 1 );
	#else
	editor_previewmapormultiplayer ( 0 );
	#endif

	// PE: I cant restore editor after multiplayer mode ? SO:
	// call a new map editor
	#ifdef FPSEXCHANGE
	#ifdef PRODUCTV3
	// not for VRQ - allow it to return normally
	#else
	OpenFileMap(2, "FPSEXCHANGE");
	SetFileMapString(2, 1000, "Guru-MapEditor.exe");
	SetFileMapString(2, 1256, "-r");
	SetFileMapDWORD(2, 994, 0);
	SetFileMapDWORD(2, 924, 1);
	SetEventAndWait(2);
	// Terminate fragmented EXE
	common_justbeforeend();
	ExitProcess(0);
	#endif
	#endif

	// As multiplayer can load OTHER things, restore level to state before we clicked MM button
	t.tfile_s=g.mysystem.editorsGridedit_s+"cfg.cfg";//"editors\\gridedit\\cfg.cfg";
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		timestampactivity(0,"reloading your level after MM button");
		t.skipfpmloading=0;
		g.projectfilename_s=g.mysystem.editorsGrideditAbs_s+"worklevel.fpm";//g.fpscrootdir_s+"\\Files\\editors\\gridedit\\worklevel.fpm";
		editor_loadcfg ( );
		gridedit_load_map ( );

		//  added to solve fog issue when go in and out of MP menu
		visuals_editordefaults ( );
		t.visuals.refreshshaders=1;
	}

	//  restore last edited project
	g.projectfilename_s=t.storeprojectfilename_s;
	gridedit_updateprojectname ( );

	//editor_restoreeditcamera();
	//SyncMaskOverride(0xFFFFFFFF);

}

void editor_previewmap ( int iUseVRTest )
{
	//  check if we are in the importer or character creator, if we are, don't test ma
	editor_checkIfInSubApp ( );
	//  Set single player test game flags here
	t.game.runasmultiplayer=0;
	#ifdef VRTECH
	editor_previewmapormultiplayer ( iUseVRTest );
	#else
	editor_previewmapormultiplayer ( 0 );
	#endif
}

void editor_previewmap_initcode(int iUseVRTest)
{
	editor_previewmapormultiplayer_initcode ( iUseVRTest );
}

bool editor_previewmap_loopcode(int iUseVRTest)
{
	// loop ended
	return editor_previewmapormultiplayer_loopcode ( iUseVRTest );
}

void editor_previewmap_afterloopcode(int iUseVRTest)
{
	editor_previewmapormultiplayer_afterloopcode ( iUseVRTest );
}

void input_getfilemapcontrols ( void )
{
	//  Update triggers and issue actions through filemapping system
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	#endif

	#ifdef FPSEXCHANGE
	 // only if the IDE has foreground focus
	 DWORD dwForegroundFocusForIDE = GetFileMapDWORD( 1, 596 );
	 if ( dwForegroundFocusForIDE == 10 )
	 {
		t.inputsys.xmouse=GetFileMapDWORD( 1, 0 );
		t.inputsys.ymouse=GetFileMapDWORD( 1, 4 );
		t.inputsys.xmousemove=GetFileMapDWORD( 1, 8 );
		t.inputsys.ymousemove=GetFileMapDWORD( 1, 12 );
		SetFileMapDWORD (  1, 8, 0 );
		SetFileMapDWORD (  1, 12, 0 );
		t.inputsys.wheelmousemove=GetFileMapDWORD( 1, 16 );
		set_inputsys_mclick(GetFileMapDWORD(1, 20));// t.inputsys.mclick = GetFileMapDWORD(1, 20);
		if (GetFileMapDWORD(1, 28) == 1)  set_inputsys_mclick(2);// t.inputsys.mclick = 2;
		if (MouseClick() == 4)  set_inputsys_mclick(4);// t.inputsys.mclick = 4;
		if (t.interactive.insidepanel == 1)  set_inputsys_mclick(0);// t.inputsys.mclick = 0;
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
	 }
	#else
	 t.inputsys.xmouse = MouseX();
	 t.inputsys.ymouse = MouseY();
	 t.inputsys.xmousemove = MouseMoveX();
	 t.inputsys.ymousemove = MouseMoveY();
	 t.inputsys.wheelmousemove = MouseMoveZ();
	 t.inputsys.mclick = MouseClick();
	 if (t.interactive.insidepanel == 1) t.inputsys.mclick = 0;
	 t.inputsys.kscancode = ScanCode();
	#endif

	//  extra mappings
	input_extramappings ( );

	//  Control keys direct from keyboard
	#ifdef FPSEXCHANGE
	 t.inputsys.keyreturn=GetFileMapDWORD( 1, 108 );
	 t.inputsys.keyshift=GetFileMapDWORD( 1, 112 );
	 t.inputsys.keytab = 0;
	 t.inputsys.keyup=GetFileMapDWORD( 1, 120 );
	 t.inputsys.keydown=GetFileMapDWORD( 1, 124 );
	 t.inputsys.keyleft=GetFileMapDWORD( 1, 128 );
	 t.inputsys.keyright=GetFileMapDWORD( 1, 132 );
	 t.inputsys.keycontrol=GetFileMapDWORD( 1, 116 );
	 t.inputsys.keyalt=GetFileMapDWORD( 1, 136 );
	#else
	t.inputsys.keyreturn = ReturnKey();
	t.inputsys.keyshift = ShiftKey();
	t.inputsys.keytab = 0;
	t.inputsys.keyup = UpKey();
	t.inputsys.keydown = DownKey();
	t.inputsys.keyleft = LeftKey();
	t.inputsys.keyright = RightKey();
	t.inputsys.keycontrol = ControlKey();
	t.inputsys.keyalt = 0;
	#endif
	if (  t.inputsys.kscancode == 32  )  t.inputsys.keyspace = 1; else t.inputsys.keyspace = 0;

	// will release keypress flag if no key AND no SHIFT!!
	if (t.inputsys.kscancode == 0 && t.inputsys.keyshift == 0) t.inputsys.keypressallowshift = 0;

	//  W,A,S,D in editor for scrolling about (easier for user)
	if (  t.inputsys.kscancode == 87  )
		t.inputsys.keyup = 1;
	if (  t.inputsys.kscancode == 65  )  t.inputsys.keyleft = 1;
	if (  t.inputsys.kscancode == 83  )  t.inputsys.keydown = 1;
	if (  t.inputsys.kscancode == 68  )  t.inputsys.keyright = 1;

	//  fake mousemove values for low-response systems (when in zoomed in mode)
	if (  t.grideditselect == 4 ) 
	{
		if (  t.inputsys.keyshift == 1 ) 
		{
			if (  t.inputsys.keyleft == 1  )  t.inputsys.xmousemove = -10;
			if (  t.inputsys.keyright == 1  )  t.inputsys.xmousemove = 10;
			if (  t.inputsys.keyup == 1  )  t.inputsys.ymousemove = -10;
			if (  t.inputsys.keydown == 1  )  t.inputsys.ymousemove = 10;
			set_inputsys_mclick(2);// t.inputsys.mclick = 2;
			t.inputsys.keyleft=0;
			t.inputsys.keyright=0;
			t.inputsys.keyup=0;
			t.inputsys.keydown=0;
		}
	}

	//  special trigger when click LIBRARY TAB, force into respective mode
	#ifdef FPSEXCHANGE
	if (  GetFileMapDWORD( 1, 546 ) == 1 ) 
	{
		t.ttabindex=GetFileMapDWORD( 1, 520 );
		if (  t.ttabindex == 0  )  t.inputsys.domodeentity = 1;
		if (  t.ttabindex == 1  )  t.inputsys.domodeentity = 1;
		if (  t.ttabindex == 2  )
		{
			if ( t.ebe.active == 0 )
			{
				// need to select a site (using entity creation and placement first)
			}
			else
			{
				// reset for tool work
				ebe_reset();
			}
		}
		else
		{
			// When click non-Builder tab, should leave builder mode
			ebe_hide();

			//PE: If first entity, shader have not yet had constant set , so update shaders.
			//PE: Prevent new created ebe from disappering when clicking away from "builder".
			visuals_justshaderupdate();
		}
		SetFileMapDWORD (  1, 546, 0 );
	}
	#endif

	//  recent file list
	#ifdef FPSEXCHANGE
	t.trecentfilechoice=GetFileMapDWORD( 1, 442 );
	if (  t.trecentfilechoice>0 ) 
	{
		//  retain choice for action at end of subroutine
		t.trecentfilechoice_s=GetFileMapString( 1, 1000 );
		SetFileMapDWORD (  1, 442, 0 );
	}
	#endif

	//  termination trigger
	#ifdef FPSEXCHANGE
	if (  GetFileMapDWORD( 1, 908 ) == 1 ) 
	{
		// show outtro message if free version mode
		if ( g.iFreeVersionModeActive == 1 || ( g.iFreeVersionModeActive == 2 && g_trialStampDaysLeft > 0 ) )
		{
			t.inputsys.ignoreeditorintermination = 1;
			welcome_init(1);
			welcome_init(0);
			if ( g.iFreeVersionModeActive == 1 ) welcome_show(WELCOME_FREEINTROAPP);
			if ( g.iFreeVersionModeActive == 2 ) welcome_show(WELCOME_FREETRIALINTROAPP);
			t.inputsys.ignoreeditorintermination = 0;
		}

		// Here we ask if changes should be saved, etc
		gridedit_intercept_savefirst_noreload ( );
		OpenFileMap (  1,"FPSEXCHANGE" );
		if (  t.editorcanceltask == 0 ) 
		{
			//  go ahead, confirmed, end interface program
			SetFileMapDWORD (  1, 912, 1 );
			SetEventAndWait (  1 );
			//  close down Steam hook
			mp_free ( );
			//  end editor program
			timestampactivity(0,"Terminated because 908=1");
			common_justbeforeend();
			ExitProcess ( 0 );
		}
		else
		{
			//  carry on with interface
			SetFileMapDWORD (  1, 908, 0 );
			SetEventAndWait (  1 );
			//CloseFileMap (  1 );
		}
	}
	#endif

	//  EDIT MENU
	#ifdef FPSEXCHANGE
	if (  GetFileMapDWORD( 1, 446 ) == 1 ) 
	{  
		t.inputsys.doundo = 1; 
		SetFileMapDWORD (  1, 446, 0 ); 
	}
	if (  GetFileMapDWORD( 1, 450 ) == 1 ) { t.inputsys.doredo = 1  ; SetFileMapDWORD (  1, 450, 0 ); }
	if (  GetFileMapDWORD( 1, 454 ) == 1 ) { t.inputsys.tselcontrol = 1  ; t.inputsys.tselcut = 1 ; t.inputsys.tselcopy = 1 ; SetFileMapDWORD (  1, 454, 0 ); }
	if (  GetFileMapDWORD( 1, 458 ) == 1 ) { t.inputsys.tselcontrol = 1  ; t.inputsys.tselcopy = 1 ; SetFileMapDWORD (  1, 458, 0 ); }
	#endif

	//  Get toolbar triggers
	#ifdef FPSEXCHANGE
	t.inputsys.doartresize=0;
	t.toolbarset=GetFileMapDWORD( 1, 200 );
	t.toolbarindex=GetFileMapDWORD( 1, 204 );
	if (  t.toolbarindex>0 ) 
	{
		if (  t.toolbarset == 2 ) 
		{
			//  ZOOM IN and ZOOM OUT
			switch (  t.toolbarindex ) 
			{
			case 1 : t.inputsys.kscancode = 188 ; break ;
			case 2 : t.inputsys.kscancode = 190 ; break ;
			}		//~   endif
		}
		if (  t.toolbarset == 4 ) 
		{
			//  EDIT MODE SELECTOR (entity/terrain)
			switch (  t.toolbarindex ) 
			{
			case 7 : t.inputsys.kscancode = Asc("E") ; break ;
			case 8 : t.inputsys.kscancode = Asc("M") ; break ;
			case 9 : t.inputsys.kscancode = Asc("T") ; break ;
			}		//~   endif
		}
		if (  t.toolbarset == 6 ) 
		{
			//  TERRAIN TOOLS (sculpt,flatten,paint)
			t.inputsys.domodeterrain=1;
			switch (  t.toolbarindex ) 
			{
			case 1 : t.inputsys.kscancode = Asc("1") ; break ;
			case 2 : t.inputsys.kscancode = Asc("2") ; break ;
			case 3 : t.inputsys.kscancode = Asc("3") ; break ;
			case 4 : t.inputsys.kscancode = Asc("4") ; break ;
			case 5 : t.inputsys.kscancode = Asc("5") ; break ;
			case 6 : t.inputsys.kscancode = Asc("6") ; break ;
			case 7 : t.inputsys.kscancode = Asc("7") ; break ;
			case 8 : t.inputsys.kscancode = Asc("8") ; break ;
			case 9 : t.inputsys.kscancode = Asc("9") ; break ;
			case 10 : t.inputsys.kscancode = Asc("0") ; break ;
			}
		}
		if (  t.toolbarset == 8 ) 
		{
			//  wayppoint
			t.inputsys.domodewaypoint=1;
			switch (  t.toolbarindex ) 
			{
				case 1 :
					t.inputsys.domodewaypointcreate=1;
				break;
			}		//~   endif
		}
		if (  t.toolbarset == 9 ) 
		{
			//  rem LAUNCH TEST GAME
			switch (  t.toolbarindex ) 
			{
				case 1 :
					editor_previewmap ( 0 );
				break;
				case 2 :
					editor_multiplayermode ( );
				break;
				#ifdef VRTECH
				case 3 :
					if ( g.gvrmode == 0 )
					{
						HWND hThisWnd = GetForegroundWindow();
						MessageBoxA ( hThisWnd, "You are not in VR mode. You need to exit the software. When you restart, select VR MODE ON to enable VR.", "Not in VR Mode", MB_OK );
						OpenFileMap (  1, "FPSEXCHANGE" );
						SetFileMapDWORD (  1, 970, 1 );
						SetEventAndWait (  1 );
					}
					else
					{
						editor_previewmap ( 1 );
					}
				break;
				#endif
			}
		}
		if ( t.toolbarset == 21 ) 
		{
			// HELP MENU Actions
			#ifdef VRTECH
			switch ( t.toolbarindex ) 
			{
				case 1 : editor_showhelppage ( 1 );  break;
				case 2 : editor_showhelppage ( 2 );  break;
				case 3 : editor_showhelppage ( 3 );  break;
			}
			#else
			switch (  t.toolbarindex ) 
			{
				case 1 : editor_showhelppage ( 0 ); break;
				case 2 : 
					if (  t.interactive.active == 0  )  
						t.interactive.active = 2;
					break;
			}
			#endif
		}
	}
	#endif

	// Clear toolbar index and deltas
	#ifdef FPSEXCHANGE
	SetFileMapDWORD (  1, 200, 0 );
	SetFileMapDWORD (  1, 204, 0 );
	SetFileMapDWORD (  1, 8, 0 );
	SetFileMapDWORD (  1, 12, 0 );
	SetFileMapDWORD (  1, 16, 0 );
	#endif

	// Deactivate mouse if leave 3d area
	if ( t.inputsys.xmouse == -1 && t.inputsys.ymouse == -1 ) 
	{
		t.inputsys.xmouse=500000;
		t.inputsys.ymouse=0;
		t.inputsys.xmousemove=0;
		t.inputsys.ymousemove=0;
		t.inputsys.activemouse=0;
		set_inputsys_mclick(0);// t.inputsys.mclick = 0;
		t.syncthreetimes=2;
	}
	else
	{
		if (  t.inputsys.activemouse == 0 ) 
		{
			//  was out, now back in
			editor_refresheditmarkers ( );
		}
	}
	t.inputsys.activemouse=1;

	// Convert FILE MAP COMM VALUES to DX INPUT CODES
	t.t_s="" ; t.tt=0;
	switch ( t.inputsys.kscancode ) 
	{
		case 9 : t.tt = 15 ; break ;
		case 32 : t.tt = 57 ; break ;
		case 33 : t.tt = 201 ; break ;
		case 34 : t.tt = 209 ; break ;
		case 37 : t.tt = 203 ; break ;
		case 38 : t.tt = 200 ; break ;
		case 39 : t.tt = 205 ; break ;
		case 40 : t.tt = 208 ; break ;
		case 42 : t.tt = 16 ; break ;
		case 46 : t.tt = 211 ; break ;
		case 54 : t.tt = 16 ; break ;
		case 112 : t.tt = 59 ; break ;
		case 113 : t.tt = 60 ; break ;
		case 114 : t.tt = 61 ; break ;
		case 115 : t.tt = 62 ; break ;
		case 123 : t.tt = 88 ; break ;
		case 187 : t.tt = 13 ; break ;
		case 188 : t.tt = 51 ; break ;
		case 189 : t.tt = 12 ; break ;
		case 190 : t.tt = 52 ; break ;
		case 192 : t.tt = 40 ; break ;
		case 219 : t.tt = 26 ; break ;
		case 220 : t.tt = 86 ; break ;
		case 221 : t.tt = 27 ; break ;
		case 222 : t.tt = 43 ; break ;
		case 1001 : t.tt = 13 ; break ;
		case 1002 : t.tt = 12 ; break ;
	}

	// 031215 - then remap to new scancodes (from keymap)
	t.tt = g.keymap[t.tt];

	// and temp back into IDE key values (for last bit)
	int ttt = 0;
	switch ( t.tt )
	{
		case 15 : ttt = 9 ; break ;
		case 57 : ttt = 32 ; break ;
		case 201 : ttt = 33 ; break ;
		case 209 : ttt = 34 ; break ;
		case 203 : ttt = 37 ; break ;
		case 200 : ttt = 38 ; break ;
		case 205 : ttt = 39 ; break ;
		case 208 : ttt = 40 ; break ;
		case 16 : ttt = 42 ; break ;
		case 211 : ttt = 46 ; break ;
		case 59 : ttt = 112 ; break ;
		case 60 : ttt = 113 ; break ;
		case 61 : ttt = 114 ; break ;
		case 62 : ttt = 115 ; break ;
		case 88 : ttt = 123 ; break ;
		case 13 : ttt = 187 ; break ;
		case 51 : ttt = 188 ; break ;
		case 12 : ttt = 189 ; break ;
		case 52 : ttt = 190 ; break ;
		case 40 : ttt = 192 ; break ;
		case 26 : ttt = 219 ; break ;
		case 86 : ttt = 220 ; break ;
		case 27 : ttt = 221 ; break ;
		case 43 : ttt = 222 ; break ;
	}
	// then create proper inkey chars from revised (if any) scancodes
	switch ( ttt )
	{
		case 16 : t.t_s = "q"; break;
		case 57 : t.t_s = " "; break;
		case 107 : t.t_s = "="; break;
		case 109 : t.t_s = "-"; break;
		case 187 : t.t_s = "="; break;
		case 188 : t.t_s = ","; break;
		case 189 : t.t_s = "-"; break;
		case 190 : t.t_s = "."; break;
		case 192 : t.t_s = "'"; break;
		case 219 : t.t_s = "["; break;
		case 220 : t.t_s = "\\"; break;
		case 221 : t.t_s = "]"; break;
		case 222 : t.t_s = "#"; break;
	}
	if (  t.inputsys.kscancode >= Asc("A") && t.inputsys.kscancode <= Asc("Z")  )  t.t_s = Lower(Chr(t.inputsys.kscancode));
	if (  t.inputsys.kscancode >= Asc("0") && t.inputsys.kscancode <= Asc("9")  )  t.t_s = Lower(Chr(t.inputsys.kscancode));
	if (  t.t_s != ""  )  t.tt = 1;

	//  Get menu triggers
	t.inputsys.dosave=0 ; t.inputsys.doopen=0 ; t.inputsys.donew=0 ; t.inputsys.donewflat=0 ; t.inputsys.dosaveas=0;
	#ifdef FPSEXCHANGE
	if (  GetFileMapDWORD( 1, 404 ) == 1 ) { t.inputsys.dosave = 1  ; SetFileMapDWORD (  1, 404, 0 ); }
	if (  GetFileMapDWORD( 1, 408 ) == 1 ) { t.inputsys.donew = 1  ; SetFileMapDWORD (  1, 408, 0 ); }
	if (  GetFileMapDWORD( 1, 408 ) == 2 ) 
	{ 
		if ( t.bIgnoreFirstCallToNewLevel == true )
		{
			// 280317 - editor calls to create new map, but can load in default.fpm at start (when welcome screen active)
			t.bIgnoreFirstCallToNewLevel = false;
		}
		else
		{
			t.inputsys.donewflat = 1; 
		}
		SetFileMapDWORD (  1, 408, 0 ); 
	}
	if (  GetFileMapDWORD( 1, 434 ) == 1 ) { t.inputsys.dosaveas = 1  ; SetFileMapDWORD (  1, 434, 0 ); }
	if (  GetFileMapDWORD( 1, 400 ) == 1 ) { t.inputsys.doopen = 1  ; t.inputsys.donew = 0 ; t.inputsys.donewflat = 0 ; SetFileMapDWORD (  1, 400, 0 ); }
	#else
	if (t.bTriggerNewMapAtStart == true)
	{
		t.bTriggerNewMapAtStart = false;
		t.inputsys.donew = 1;
	}
	#endif

	// select items from editing to see values
	#ifdef FPSEXCHANGE
	SetEventAndWait (  1 );
	t.tindex1=GetFileMapDWORD( 1, 712 );
	if (  t.tindex1>0 ) 
	{
		t.tt=1 ; t.t_s="";
		t.tindex2=GetFileMapDWORD( 1, 716 );
		if (  t.tindex1 == 2 ) 
		{
			t.inputsys.domodeterrain=1;
			switch (  t.tindex2 ) 
			{
				case 1 : t.t_s = "1" ; break ;
				case 2 : t.t_s = "2" ; break ;
				case 3 : t.t_s = "3" ; break ;
				case 4 : t.t_s = "6" ; break ;
				case 5 : t.t_s = "9" ; break ;
				case 6 : t.t_s = "0" ; break ;
			}
		}
		if (  t.tindex1 == 1 ) 
		{
			switch (  t.tindex2 ) 
			{
				case 1 : t.t_s = "t" ; break ;
				case 2 : t.t_s = "e" ; break ;
			}
		}
		SetFileMapDWORD (  1, 712, 0 );
		SetFileMapDWORD (  1, 716, 0 );
		SetEventAndWait (  1 );
	}
	#endif

	//  Record final translated key values
	t.inputsys.k_s=t.t_s ; t.inputsys.kscancode=t.tt;

	//  Determine if Library Selection Made
	#ifdef FPSEXCHANGE
	if ( GetFileMapDWORD( 1, 516 ) > 0 )
	{
		SetFileMapDWORD ( 1, 516, 0 );
		t.clickedonworkspace = GetFileMapDWORD( 1, 520 );
		t.clickeditemonworkspace = GetFileMapDWORD( 1, 524 );
		if ( t.clickeditemonworkspace != -1 ) 
		{
			// if in EBE tool, and switch to Entity/Marker tab, reactivate EBE
			if ( t.clickedonworkspace == 2 ) 
			{
				if ( t.ebe.active != 0 )
				{
					// switch on EBE tool visuals
					ebe_reset();
				}
				else
				{
					// not active, but need to be in entity mode for placement of site
					t.inputsys.domodeentity = 1; 
					t.grideditselect = 5; 
				}
			}
			else
			{
				ebe_hide();
			}

			// check if Entity/Marker/Builder tab selected
			if ( t.clickedonworkspace == 1 || t.clickedonworkspace == 2 ) 
			{
				if ( t.clickedonworkspace == 1 || (t.clickedonworkspace == 2 && t.clickeditemonworkspace == 0) )
				{
					if ( t.clickedonworkspace == 1 )
					{
						// Selected Marker (playerstart,light,trigger,emission)
						t.addentityfile_s = t.markerentitybank_s[1+t.clickeditemonworkspace];
					}
					else
					{
						// Or Builder 'Add New Site' which produces an entity we can use
						t.addentityfile_s = t.ebebank_s[1+t.clickeditemonworkspace];
						t.inputsys.domodeentity = 1; // ensure can position entity when select site
					}
					if ( t.addentityfile_s != "" ) 
					{
						#ifdef ENABLEIMGUI
						DeleteWaypointsAddedToCurrentCursor();
						//CheckTooltipObjectDelete();
						CloseDownEditorProperties();
						#endif
						entity_adduniqueentity ( false );
						t.tasset=t.entid;
						if ( t.talreadyloaded == 0 ) 
						{
							editor_filllibrary ( );
						}
					}
					t.inputsys.constructselection = t.tasset;
				}
				if ( t.clickedonworkspace == 2 && t.clickeditemonworkspace > 0 )
				{
					#ifdef ENABLEIMGUI
					CloseDownEditorProperties();
					#endif
					// selected builder tool icon - load in pattern for cube-insertion
					LPSTR pPBFEBEFile = t.ebebank_s[1+t.clickeditemonworkspace].Get();
					// loads painting pattern
					ebe_loadpattern ( pPBFEBEFile );
					t.inputsys.constructselection = 0;
				}
			}
			if ( t.clickedonworkspace == 0 ) 
			{
				if (  t.clickeditemonworkspace == 0 ) 
				{
					t.tnewadd=0;
					if (  t.clickedonworkspace == 0 ) 
					{
						#ifdef ENABLEIMGUI
						DeleteWaypointsAddedToCurrentCursor();
						//CheckTooltipObjectDelete();
						CloseDownEditorProperties();
						#endif

						//  [new entity]
						entity_addtoselection ( );
						t.tnewadd=t.entnewloaded;
						t.tasset=t.entid;
					}
					//  add asset to library
					if ( t.tnewadd == 1 ) editor_filllibrary ( );
					//  use as current asset
					t.inputsys.constructselection=t.tasset;
				}
				else
				{
					//  select existing asset
					if (  t.clickedonworkspace == 0 ) 
					{
						//  entity uses array to indicate the real entity index (to exclude markers - see above)
						if (  t.clickeditemonworkspace >= 0 && t.clickeditemonworkspace <= ArrayCount(t.locallibraryent) ) 
						{
							t.inputsys.constructselection=t.locallibraryent[t.clickeditemonworkspace];
						}
						else
						{
							t.inputsys.constructselection=0;
						}
					}
					else
					{
						//  direct relationship between list index and choice
						t.inputsys.constructselection=t.clickeditemonworkspace;
					}
				}
			}

			//  Workspace index determines editing mode
			if (  t.clickedonworkspace  == 0 ) { t.inputsys.domodeentity = 1 ; t.grideditselect  =  5; }
			if (  t.clickedonworkspace  == 1 ) { t.inputsys.domodeentity = 1 ; t.grideditselect  =  5; }
			editor_refresheditmarkers ( );
		}
	}
	#endif

	// Ensure status bar is constantly updated
	#ifdef FPSEXCHANGE
	++t.interfacestatusbarupdate;
	if ( t.interfacestatusbarupdate>30 ) 
	{
		// cursor position
		if ( g.gridlayershowsingle == 1 ) 
		{
			t.t_s = "" ; t.t_s=t.t_s +"CLIP="+Str(int(t.clipheight_f));
		}
		else
		{
			t.t_s="CLIP OFF";//"ALL" ; t.t_s=t.strarr_s[80]+":"+t.t_s;
		}
		SetFileMapString (  1, 4256, t.t_s.Get() );
		 t.strwork = "" ; t.strwork = t.strwork + "X:"+Str(t.inputsys.mmx)+"  "+"Z:"+Str(t.inputsys.mmy);
		SetFileMapString (  1, 4512, t.strwork.Get() );
		if (  t.gridentitygridlock == 0  )  t.tpre_s = "NORMAL";
		if (  t.gridentitygridlock == 1  )  t.tpre_s = "SNAP";
		if (  t.gridentitygridlock == 2  )  t.tpre_s = "GRID";
		SetFileMapString (  1, 4768, t.tpre_s.Get() );
		SetEventAndWait (  1 );
		//  editing mode
		if (  t.grideditselect == 0 ) 
		{
			t.statusbar_s=t.strarr_s[332];
			terrain_getpaintmode ( );
			t.statusbar_s=t.statusbar_s+" "+t.mode_s;
		}
		if (  t.grideditselect  ==  1  )  t.statusbar_s = t.strarr_s[336];
		if (  t.grideditselect == 2 ) 
		{
			//  art tools not used any more
		}
		if (  t.grideditselect == 3 ) 
		{
			//  map view mode not used any more
		}
		if (  t.grideditselect == 4 )  t.statusbar_s = t.strarr_s[343];
		if (  t.grideditselect == 5 ) 
		{
			t.statusbar_s=t.strarr_s[344];
			t.statusbar_s=t.statusbar_s+"  Entity : "+t.relaytostatusbar_s;
		}
		if (  t.grideditselect == 6 ) 
		{
			//  add waypoint status
			t.statusbar_s="Waypoint Mode (LMB=Drag Point  SHIFT+LMB=Clone Point  SHIFT+RMB=Remove Point)";
		}
		//  only update infrequently
		t.interfacestatusbarupdate=0;
	}
	#endif

	//  Update status bar out of action subroutines
	gridedit_updatestatusbar ( );

	// Action after filemap activity
	if ( t.trecentfilechoice>0 ) 
	{
		// save first
		gridedit_intercept_savefirst ( );
		if ( t.editorcanceltask == 0 ) 
		{
			// go ahead, load direct (skip the open dialog)
			g.projectfilename_s=t.trecentfilechoice_s;
			gridedit_load_map ( );

		}
		t.trecentfilechoice=0;
	}
}

void editor_handlepguppgdn ( void )
{
	// changes and returns 't.tupdownstepvalue_f'
	bool bWidgetMove = false;
	if (t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209)
	{
		float fEntityStepSize = 5.0f;
		if (t.gridentity > 0 && t.gridentityobj > 0)
		{
			if (ObjectExist(t.gridentityobj) == 1)
				fEntityStepSize = ObjectSizeY(t.gridentityobj, 1);
		}
		else
		{
			if (t.widget.pickedEntityIndex > 0)
			{
				int wobj = t.entityelement[t.widget.pickedEntityIndex].obj;
				if (t.widget.activeObject > 0)
					wobj = t.widget.activeObject;

				if (ObjectExist(wobj) == 1)
					fEntityStepSize = ObjectSizeY(wobj, 1);

				//Make sure to highlight all objects the object belong to.
			}
		}
		if (t.gridentitygridlock >= 1)
			t.tupdownstepvalue_f = 0.0;
		else
			t.tupdownstepvalue_f = 1.0;
		if (t.gridentitygridlock > 0)
		{
			if (t.inputsys.keypressallowshift == 0 && (t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209))
			{
				if (t.gridentitygridlock == 1)
					t.tupdownstepvalue_f = fEntityStepSize;
				if (t.gridentitygridlock == 2)  t.tupdownstepvalue_f = 100.0;
				t.inputsys.keypressallowshift = 1;
			}
			else
			{
				t.tupdownstepvalue_f = 0;
			}
		}
			if (t.inputsys.kscancode == 201)  t.gridentityposy_f += t.tupdownstepvalue_f;
			if (t.inputsys.kscancode == 209)  t.gridentityposy_f -= t.tupdownstepvalue_f;
	}
}

#ifdef ENABLEIMGUI
void imgui_input_getcontrols(void)
{
	//  Some actions are directly triggered by input subroutine
	t.inputsys.doload = 0;
	t.inputsys.domodeterrain = 0;
	t.inputsys.domodeentity = 0;
	t.inputsys.domodemarker = 0;
	t.inputsys.domodewaypoint = 0;
	t.inputsys.doundo = 0;
	t.inputsys.doredo = 0;
	t.inputsys.tselcontrol = 0;
	t.inputsys.tselcut = 0;
	t.inputsys.tselcopy = 0;
	t.inputsys.tseldelete = 0;

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//io.AddMousePosEvent(mouse_x, mouse_y);  // update mouse position //cyb
	//io.AddMouseButtonEvent(0, mouse_b[0]);  // update mouse button states
	//io.AddMouseButtonEvent(1, mouse_b[1]);  // update mouse button states


	#ifdef USERENDERTARGET
	//PE: Take everything from imgui.

	float itmpmousex = ImGui::GetMousePos().x;
	float itmpmousey = ImGui::GetMousePos().y;
	int iSecureZone = 4;
	RECT winpos = { 0,0,0,0 };

	if (pref.bDisableMultipleViewport) {
		//PE: Mouse coords is different when using non Multiple Viewport.
		//PE: TODO we need to match the Multiple Viewport mouse coord system here:
		//POINT p;
		//GetCursorPos(&p);
		//itmpmousex = p.x;
		//itmpmousey = p.y;
	}

	//PE: Must be relative to windows pos, or nothing work if you have a window placed at the rigth of the screen.
	GetWindowRect(g_pGlob->hWnd, &winpos);

	if (bImGuiRenderTargetFocus && itmpmousex >= (renderTargetAreaPos.x+iSecureZone) && itmpmousey >= (renderTargetAreaPos.y + iSecureZone) &&
		itmpmousex <= renderTargetAreaPos.x + (renderTargetAreaSize.x - iSecureZone) && itmpmousey <= renderTargetAreaPos.y + (renderTargetAreaSize.y - ImGuiStatusBar_Size - iSecureZone ))
	{
		// Figure out mouse scale values.
		itmpmousex -= renderTargetAreaPos.x;
		itmpmousey -= renderTargetAreaPos.y;
		float RatioX = ((float) GetDisplayWidth() / (float) renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1) );
		float RatioY = ((float) GetDisplayHeight() / (float) renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1) );
		itmpmousex *= RatioX;
		itmpmousey *= RatioY;

		t.inputsys.activemouse = 1;
		t.inputsys.xmouse = (int)itmpmousex;
		t.inputsys.ymouse = (int)itmpmousey;
		t.inputsys.zmouse = io.MouseWheel; // MouseZ();
		t.inputsys.xmousemove = t.inputsys.xmouse - xmouseold;
		t.inputsys.ymousemove = t.inputsys.ymouse - ymouseold;

		xmouseold = t.inputsys.xmouse;
		ymouseold = t.inputsys.ymouse;

		t.inputsys.wheelmousemove = io.MouseWheel; //MouseMoveZ();
		
		set_inputsys_mclick(io.MouseDown[0] + (io.MouseDown[1] * 2.0) + (io.MouseDown[2] * 3.0) + (io.MouseDown[3] * 4.0));// t.inputsys.mclick = io.MouseDown[0] + (io.MouseDown[1] * 2.0) + (io.MouseDown[2] * 3.0) + (io.MouseDown[3] * 4.0); //  MouseClick();
		
		//cyb //set_inputsys_mclick(MouseClick());// t.inputsys.mclick = io.MouseDown[0] + (io.MouseDown[1] * 2.0) + (io.MouseDown[2] * 3.0) + (io.MouseDown[3] * 4.0); //  MouseClick();

		t.inputsys.k_s = Lower(Inkey());

		//  Control keys direct from keyboard
		t.inputsys.keyreturn = io.KeysDown[13]; // ReturnKey();
		t.inputsys.keyshift = io.KeyShift;
		t.inputsys.keytab = io.KeysDown[0x09]; //TAB!
		t.inputsys.keyleft = io.KeysDown[37]; // LeftKey();
		t.inputsys.keyright = io.KeysDown[39]; //RightKey();
		t.inputsys.keyup = io.KeysDown[38]; //UpKey();
		t.inputsys.keydown = io.KeysDown[40]; // DownKey();
		t.inputsys.keycontrol = io.KeyCtrl; //ControlKey();
		t.inputsys.keyspace = io.KeysDown[32]; //SpaceKey();

		//PE: We need raw scancodes. just take it from imgui.
		t.inputsys.kscancode = 0;
		for (int iTemp = 0; iTemp < 256; iTemp++)
		{
			if (iTemp != 16 && iTemp != 17 && iTemp != 18) 
			{ 
				//shift,control (added 18, not sure what it is)
				if (io.KeysDown[iTemp] > 0)
				{
					t.inputsys.kscancode = iTemp;
					break;
				}
			}
		}

	}
	else 
	{
		//No input to DX11.
		t.inputsys.activemouse = 1;
		t.inputsys.zmouse = 0;

		t.inputsys.xmousemove = 0;
		t.inputsys.ymousemove = 0;

		xmouseold = t.inputsys.xmouse;
		ymouseold = t.inputsys.ymouse;

		t.inputsys.wheelmousemove = 0;

		set_inputsys_mclick(0);// t.inputsys.mclick = 0;

		t.inputsys.k_s = "";
		//t.inputsys.kscancode = ScanCode();

		//  Control keys direct from keyboard
		t.inputsys.keyreturn = 0;
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.keycontrol = 0;
		t.inputsys.keyspace = 0;
		t.inputsys.kscancode = 0;
	}

	int mcursor = ImGui::GetMouseCursor();

	if (bBuilder_Properties_Window) 
	{
		//Disable some keys.
	}

	if (0) //(g_bCharacterCreatorPlusActivated)  //cyb
	{
		//Disable some keys.
		if( t.inputsys.kscancode == Asc("t") || t.inputsys.kscancode == Asc("T") )
			t.inputsys.kscancode = 0;
		if( t.inputsys.k_s == "t" || t.inputsys.k_s == "T" ) 
			t.inputsys.k_s = "";
		if (t.inputsys.kscancode == Asc("p") || t.inputsys.kscancode == Asc("P"))
			t.inputsys.kscancode = 0;
		if (t.inputsys.k_s == "p" || t.inputsys.k_s == "P")
			t.inputsys.k_s = "";

	}
	if (bEntity_Properties_Window) 
	{
		//Disable all key input when in Properties.
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.kscancode = 0;
		t.inputsys.k_s = "";
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
	}

	if (ImGui::IsAnyItemActive() && t.inputsys.mclick != 1 ) 
	{
		//A widget got focus, like textinput , disable all keys.
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.kscancode = 0;
		t.inputsys.k_s = "";
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
	}

	// 060320 - somehow, some laptops set 'bImGuiGotFocus' to true (or mcursor>0), 
	// wiping out click detection in Welcome screen, so added condition to prevent this erasure!
	if ( bImGuiGotFocus || ( mcursor > 0 && mcursor != ImGuiMouseCursor_Hand) )
	{
		//No GG input when using imgui.
		t.inputsys.xmouse = 500000;
		t.inputsys.ymouse = 0;
		t.inputsys.xmousemove = 0;
		t.inputsys.ymousemove = 0;
		set_inputsys_mclick(0);// t.inputsys.mclick = 0;
		t.inputsys.zmouse = 0;
		t.inputsys.wheelmousemove = 0;
		t.inputsys.activemouse = 0;
		t.syncthreetimes = 1;
		t.inputsys.k_s = "";
		//  Control keys direct from keyboard
		t.inputsys.keyreturn = 0;
		t.inputsys.keyshift = 0;
		t.inputsys.keytab = 0;
		t.inputsys.keyleft = 0;
		t.inputsys.keyright = 0;
		t.inputsys.keyup = 0;
		t.inputsys.keydown = 0;
		t.inputsys.keycontrol = 0;
		t.inputsys.keyspace = 0;
		t.inputsys.kscancode = 0;

	}
	#else
	input_getdirectcontrols();
	#endif

	input_extramappings();

	//  Flag reset
	t.inputsys.dorotation = 0;
	t.inputsys.domirror = 0;
	t.inputsys.doflip = 0;
	t.inputsys.doentityrotate = 0;
	t.inputsys.dozoomin = 0;
	t.inputsys.dozoomout = 0;
	t.inputsys.doscrollleft = 0;
	t.inputsys.doscrollright = 0;
	t.inputsys.doscrollup = 0;
	t.inputsys.doscrolldown = 0;
	t.inputsys.domapresize = 0;
	t.inputsys.dogroundmode = -1;
	t.inputsys.dozoomview = 0;
	t.inputsys.dozoomviewmovex = 0;
	t.inputsys.dozoomviewmovey = 0;
	t.inputsys.dozoomviewmovez = 0;
	t.inputsys.dozoomviewrotatex = 0;
	t.inputsys.dozoomviewrotatey = 0;
	t.inputsys.dozoomviewrotatez = 0;
	t.inputsys.dosinglelayer = 0;
	t.inputsys.tselfloor = 0;
	t.inputsys.tselpaste = 0;
	t.inputsys.tselwipe = 0;
	t.inputsys.dosaveandrun = 0;

	//PE: Map additional keys.
	if (t.inputsys.kscancode == 32)  t.inputsys.keyspace = 1; else t.inputsys.keyspace = 0;
	if (t.inputsys.kscancode == 0 && t.inputsys.keyshift == 0) t.inputsys.keypressallowshift = 0;

	//  W,A,S,D in editor for scrolling about (easier for user)
	if (t.inputsys.kscancode == 87)
		t.inputsys.keyup = 1;
	if (t.inputsys.kscancode == 65)  t.inputsys.keyleft = 1;
	if (t.inputsys.kscancode == 83)  t.inputsys.keydown = 1;
	if (t.inputsys.kscancode == 68)  t.inputsys.keyright = 1;

	if (t.inputsys.keycontrol == 1)
	{
		if (t.inputsys.k_s == "")  t.inputsys.undokeypress = 0;
		if (t.inputsys.k_s == "z" && t.inputsys.undokeypress == 0) 
		{
			t.inputsys.doundo = 1; 
			t.inputsys.undokeypress = 1;
		}
		if (t.inputsys.k_s == "y" && t.inputsys.undokeypress == 0) 
		{
			t.inputsys.doredo = 1; 
			t.inputsys.undokeypress = 1;
		}
	}

	//  Convert to DX INPUT CODES
//###
	t.t_s = ""; t.tt = 0;
	switch (t.inputsys.kscancode)
	{
	case 9: t.tt = 15; break;
	case 32: t.tt = 57; break;
	case 33: t.tt = 201; break;
	case 34: t.tt = 209; break;
	case 37: t.tt = 203; break;
	case 38: t.tt = 200; break;
	case 39: t.tt = 205; break;
	case 40: t.tt = 208; break;
	case 42: t.tt = 16; break;
	case 46: t.tt = 211; break;
	case 54: t.tt = 16; break;
	case 112: t.tt = 59; break;
	case 113: t.tt = 60; break;
	case 114: t.tt = 61; break;
	case 115: t.tt = 62; break;
	case 123: t.tt = 88; break;
	case 187: t.tt = 13; break;
	case 188: t.tt = 51; break;
	case 189: t.tt = 12; break;
	case 190: t.tt = 52; break;
	case 192: t.tt = 40; break;
	case 219: t.tt = 26; break;
	case 220: t.tt = 86; break;
	case 221: t.tt = 27; break;
	case 222: t.tt = 43; break;
	case 1001: t.tt = 13; break;
	case 1002: t.tt = 12; break;
	}
	// 031215 - then remap to new scancodes (from keymap)
	t.tt = g.keymap[t.tt];
	// and temp back into IDE key values (for last bit)
	int ttt = 0;
	switch (t.tt)
	{
	case 15: ttt = 9; break;
	case 57: ttt = 32; break;
	case 201: ttt = 33; break;
	case 209: ttt = 34; break;
	case 203: ttt = 37; break;
	case 200: ttt = 38; break;
	case 205: ttt = 39; break;
	case 208: ttt = 40; break;
	case 16: ttt = 42; break;
	case 211: ttt = 46; break;
	case 59: ttt = 112; break;
	case 60: ttt = 113; break;
	case 61: ttt = 114; break;
	case 62: ttt = 115; break;
	case 88: ttt = 123; break;
	case 13: ttt = 187; break;
	case 51: ttt = 188; break;
	case 12: ttt = 189; break;
	case 52: ttt = 190; break;
	case 40: ttt = 192; break;
	case 26: ttt = 219; break;
	case 86: ttt = 220; break;
	case 27: ttt = 221; break;
	case 43: ttt = 222; break;
	}
	// then create proper inkey chars from revised (if any) scancodes
	switch (ttt)
	{
	case 16: t.t_s = "q"; break;
	case 57: t.t_s = " "; break;
	case 107: t.t_s = "="; break;
	case 109: t.t_s = "-"; break;
	case 187: t.t_s = "="; break;
	case 188: t.t_s = ","; break;
	case 189: t.t_s = "-"; break;
	case 190: t.t_s = "."; break;
	case 192: t.t_s = "'"; break;
	case 219: t.t_s = "["; break;
	case 220: t.t_s = "\\"; break;
	case 221: t.t_s = "]"; break;
	case 222: t.t_s = "#"; break;
	}

	if (t.inputsys.kscancode >= Asc("A") && t.inputsys.kscancode <= Asc("Z"))  t.t_s = Lower(Chr(t.inputsys.kscancode));
	if (t.inputsys.kscancode >= Asc("0") && t.inputsys.kscancode <= Asc("9"))  t.t_s = Lower(Chr(t.inputsys.kscancode));
	if (t.t_s != "")  t.tt = 1;

	t.inputsys.k_s = t.t_s; t.inputsys.kscancode = t.tt;
//####

//  Input conditional flags
	if (t.inputsys.kscancode == 0) {
		t.inputsys.keypress = 0;
		if (iForceScancode > 0) {
			if (iForceScancode == 13)
				t.inputsys.keyreturn = 1;
			t.inputsys.kscancode = iForceScancode;
			iForceScancode = -1;
		}
		else if (bForceKey) {
			bForceKey = false;
			t.inputsys.k_s = csForceKey;
			t.inputsys.keycontrol = 0;
			t.inputsys.keyshift = 0;
			t.inputsys.keytab = 0;
			t.inputsys.kscancode = Asc(csForceKey.Get());
		}
		else if (bForceKey2) {
			bForceKey2 = false;
			t.inputsys.k_s = csForceKey2;
			t.inputsys.keycontrol = 0;
			t.inputsys.keyshift = 0;
			t.inputsys.keytab = 0;
			t.inputsys.kscancode = Asc(csForceKey2.Get());
		}

	}
	if (bForceUndo) 
	{
		t.inputsys.doundo = 1;
		t.inputsys.undokeypress = 1;
		bForceUndo = false;
	}
	if (bForceRedo) 
	{
		t.inputsys.doredo = 1;
		t.inputsys.undokeypress = 1;
		bForceRedo = false;
	}

	#ifdef ENABLEIMGUI
	// When old in welcome system, do not allow regular edit keys to work!
	if (iTriggerWelcomeSystemStuff != 0)
		return;
	#endif

	//  Construction Keys
	if (t.inputsys.keycontrol == 0)
	{
		// can get marker mode from anywhere
		if ((t.inputsys.kscancode == Asc("M") || t.inputsys.k_s == "m") && t.inputsys.keypress == 0)
		{
			t.inputsys.domodemarker = 1;
			t.inputsys.keypress = 1;
		}

		if ((t.grideditselect == 4 && t.gridentityinzoomview>0) || t.grideditselect == 5)
		{
			if (t.inputsys.k_s == "b" && t.inputsys.keypress == 0)
			{
				t.inputsys.keypress = 1; 
				t.gridentitygridlock = t.gridentitygridlock + 1;
				if (t.gridentitygridlock>2)  
					t.gridentitygridlock = 0;
			}
			if (t.inputsys.k_s == "y" && t.inputsys.keypress == 0 && g.gentitytogglingoff == 0)
			{
				// only if not EBE
				if (t.entityprofile[t.gridentity].isebe == 0)
				{
					//PE: This dont work, t.gridentity = 0 ?
					t.ttrygridentitystaticmode = 1 - t.gridentitystaticmode;
					t.ttrygridentity = t.gridentity; editor_validatestaticmode();
				}
				t.inputsys.keypress = 1;
			}
			if (t.inputsys.k_s == "u" && t.inputsys.keypress == 0)
			{
				//  control auto-flatten
				t.inputsys.keypress = 1;
				t.gridedit.autoflatten = 1 - t.gridedit.autoflatten;
			}
			if (t.inputsys.k_s == "i" && t.inputsys.keypress == 0)
			{
				//  control entity spray mode
				t.inputsys.keypress = 1;
				t.gridedit.entityspraymode = 1 - t.gridedit.entityspraymode;
			}
			// except when in EBE mode which handles - and + keys for material changing
			if (t.ebe.on == 0)
			{
				if (t.inputsys.k_s == "-" && t.inputsys.keypress == 0) { t.gridentitymodifyelement = 1; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "=" && t.inputsys.keypress == 0) { t.gridentitymodifyelement = 2; t.inputsys.keypress = 1; }
			}
		}

		//  editing mode
		if (t.inputsys.k_s == "t")
		{
			t.inputsys.domodeterrain = 1; t.inputsys.dowaypointview = 0;
			bTerrain_Tools_Window = true;
		}
		if (t.inputsys.k_s == "v")
		{
		}
		if (t.inputsys.k_s == "e" || t.inputsys.k_s == "o")
		{
			t.inputsys.domodeentity = 1; t.inputsys.dowaypointview = 0;
		}
		if (t.inputsys.k_s == "p")
		{
			bWaypoint_Window = true;
			t.inputsys.domodewaypoint = 1;
			t.inputsys.dowaypointview = 0;
		}
		if (t.inputsys.keyspace == 1 && t.inputsys.keypress == 0) { t.inputsys.dowaypointview = 1 - t.inputsys.dowaypointview; t.inputsys.keypress = 1; t.lastgrideditselect = -1; editor_refresheditmarkers(); }

		//  NUM-ROTATE CONTROLS
		if (t.inputsys.k_s == "r" && t.inputsys.keypress == 0) {
			t.inputsys.dorotation = 1; t.inputsys.keypress = 1;
		}
		if (t.grideditselect != 4 && t.grideditselect != 0)
		{
			{
				if (t.inputsys.k_s == "1" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 1; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "2" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 2; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "3" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 3; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "4" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 4; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "5" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 5; t.inputsys.keypress = 1; }
				if (t.inputsys.k_s == "6" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 6; t.inputsys.keypress = 1; }
				if (t.inputsys.keyshift == 0)
				{
					if (t.inputsys.k_s == "0" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 98; t.inputsys.keypress = 1; }
				}
				else
				{
					if (t.inputsys.k_s == "0" && t.inputsys.keypress == 0) { t.inputsys.doentityrotate = 99; t.inputsys.keypress = 1; }
				}
			}
		}

		//  Editing of Map
		if (t.inputsys.k_s == ",")  t.inputsys.dozoomin = 1;
		if (t.inputsys.k_s == ".")  t.inputsys.dozoomout = 1;

		//  TAB Key causes layer edit view control
		if (t.inputsys.kscancode == 15 && t.inputsys.keypress == 0) { t.inputsys.dosinglelayer = 1; t.inputsys.keypress = 1; }

		//  F1 for help page
		if (t.inputsys.kscancode == 59) editor_showhelppage(0);

		// F2-F4 in editor to control widget mode
		if (t.inputsys.kscancode >= 60 && t.inputsys.kscancode <= 62)
		{
			t.toldmode = t.widget.mode;
			if (t.inputsys.kscancode == 60) t.widget.mode = 0;
			if (t.inputsys.kscancode == 61) t.widget.mode = 1;
			if (t.inputsys.kscancode == 62) t.widget.mode = 2;
			if ( t.toldmode != t.widget.mode ) widget_show_widget ( );
		}
	}
	else
	{
		if (t.inputsys.k_s == "r")  t.inputsys.dorotation = 1;
	}

	//  Key Map Scroll and Resize
	if (t.inputsys.keyshift == 0)
	{
		if (t.inputsys.keyleft == 1)  t.inputsys.doscrollleft = 3;
		if (t.inputsys.keyright == 1)  t.inputsys.doscrollright = 3;
		if (t.inputsys.keyup == 1) 
			t.inputsys.doscrollup = 3;
		if (t.inputsys.keydown == 1)  t.inputsys.doscrolldown = 3;
	}
	else
	{
		if (t.inputsys.keyleft == 1)  t.inputsys.doscrollleft = 20;
		if (t.inputsys.keyright == 1)  t.inputsys.doscrollright = 20;
		if (t.inputsys.keyup == 1)
			t.inputsys.doscrollup = 20;
		if (t.inputsys.keydown == 1)  t.inputsys.doscrolldown = 20;
	}

	//  Mouse Wheel control (170616 - but not when in EBE mode as its used for grid layer control)
	if (t.ebe.on == 0)
	{
		if (t.grideditselect == 4)
		{
			//  Zoomed in View
			t.zoomviewcamerarange_f -= (t.inputsys.wheelmousemove / 10.0);
		}
		else
		{
			//  Non-Zoomed in View
			if (t.inputsys.keycontrol == 0)
			{
				if (t.inputsys.wheelmousemove<0)
					t.inputsys.dozoomout = 1;
				if (t.inputsys.wheelmousemove>0)
					t.inputsys.dozoomin = 1;
			}
		}
	}

	//  UndoRedo Keys
	if (t.inputsys.keycontrol == 1)
	{
		if (t.inputsys.k_s == "")  t.inputsys.undokeypress = 0;
		if (t.inputsys.k_s == "z" && t.inputsys.undokeypress == 0) { t.inputsys.doundo = 1; t.inputsys.undokeypress = 1; }
		if (t.inputsys.k_s == "y" && t.inputsys.undokeypress == 0) { t.inputsys.doredo = 1; t.inputsys.undokeypress = 1; }
	}

	//  Controls only when in zoomview
	if (t.grideditselect == 4)
	{
		//  orient arrowkey movement to camera angle
		t.tca_f = WrapValue(CameraAngleY());
		if (t.tca_f >= 360 - 45 || t.tca_f <= 45)
		{
			t.txa = 1; t.txb = 2; t.txc = 0; t.txd = 0;
			t.tza = 0; t.tzb = 0; t.tzc = 2; t.tzd = 1;
		}
		else
		{
			if (t.tca_f >= 180 - 45 && t.tca_f <= 180 + 45)
			{
				t.txa = 2; t.txb = 1; t.txc = 0; t.txd = 0;
				t.tza = 0; t.tzb = 0; t.tzc = 1; t.tzd = 2;
			}
			else
			{
				if (t.tca_f <= 180)
				{
					t.txa = 0; t.txb = 0; t.txc = 2; t.txd = 1;
					t.tza = 2; t.tzb = 1; t.tzc = 0; t.tzd = 0;
				}
				else
				{
					t.txa = 0; t.txb = 0; t.txc = 1; t.txd = 2;
					t.tza = 1; t.tzb = 2; t.tzc = 0; t.tzd = 0;
				}
			}
		}
		t.inputsys.dozoomviewmovex = 0; t.inputsys.dozoomviewmovez = 0;
		if (t.inputsys.keyleft == 1) { t.inputsys.dozoomviewmovex += t.txa; t.inputsys.dozoomviewmovez += t.tza; }
		if (t.inputsys.keyright == 1) { t.inputsys.dozoomviewmovex += t.txb; t.inputsys.dozoomviewmovez += t.tzb; }
		if (t.inputsys.keyup == 1)
		{
			t.inputsys.dozoomviewmovex += t.txc;
			t.inputsys.dozoomviewmovez += t.tzc;
		}
		if (t.inputsys.keydown == 1) { t.inputsys.dozoomviewmovex += t.txd; t.inputsys.dozoomviewmovez += t.tzd; }
		//  control rotation
		if (t.inputsys.k_s == "1" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatex = 1; t.inputsys.keypress = 1; }
		if (t.inputsys.k_s == "2" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatex = 2; t.inputsys.keypress = 1; }
		if (t.inputsys.k_s == "3" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatey = 1; t.inputsys.keypress = 1; }
		if (t.inputsys.k_s == "4" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatey = 2; t.inputsys.keypress = 1; }
		if (t.inputsys.k_s == "5" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatez = 1; t.inputsys.keypress = 1; }
		if (t.inputsys.k_s == "6" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatez = 2; t.inputsys.keypress = 1; }
		if (t.inputsys.keyshift == 0)
		{
			if (t.inputsys.k_s == "0" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatex = 98; t.inputsys.keypress = 1; }
		}
		else
		{
			if (t.inputsys.k_s == "0" && t.inputsys.keypress == 0) { t.inputsys.dozoomviewrotatex = 99; t.inputsys.keypress = 1; }
		}
	}
	if (t.grideditselect == 4 || t.grideditselect == 5)
	{
		//  control finder (toggled using gridentityautofind value)
		if (t.inputsys.keyreturn == 1)
		{
			if (t.gridentityautofind == 0) { t.gridentityautofind = 3; }
			if (t.gridentityautofind == 1) { t.gridentityautofind = 2; }
		}
		else
		{
			if (t.gridentityautofind == 3) { t.gridentityautofind = 1; t.gridentityusingsoftauto = 0; t.gridentitysurfacesnap = 0; }
			if (t.gridentityautofind == 2) { t.gridentityautofind = 0; t.gridentityposoffground = 0; t.gridentityusingsoftauto = 1; t.gridentitysurfacesnap = 0; }
		}
		if (t.gridentityautofind == 1 && t.gridentity>0)
		{
			t.gridentitydroptoground = 1 + t.entityprofile[t.gridentity].forwardfacing;
		}
		else
		{
			t.gridentitydroptoground = 0;
		}
		//  control height
		if (t.grideditselect == 4)
		{
			//  move entity through zoomview system
			if (t.inputsys.kscancode == 201) { t.inputsys.dozoomviewmovey = 2; t.gridentityposoffground = 1; t.gridentityautofind = 0; t.gridentityusingsoftauto = 0; }
			if (t.inputsys.kscancode == 209) { t.inputsys.dozoomviewmovey = 1; t.gridentityposoffground = 1; t.gridentityautofind = 0; t.gridentityusingsoftauto = 0; }
		}
		else
		{
			//  directly move entity (and detatch from terrain) PGUP and PGDN
			if (t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209)
			{
				if (t.widget.activeObject == 0)
				{
					editor_handlepguppgdn();
					t.gridentityposoffground = 1; t.gridentityautofind = 0; t.gridentityusingsoftauto = 0; t.gridentitysurfacesnap = 0;
				}
			}
		}
	}

	//  Create a waypoint when instructed to
	if (t.inputsys.domodewaypointcreate == 1 && t.inputsys.keypress == 0)
	{
		//In freeflight mode t.cx_f,t.cy_f is NOT the same as CameraPositionX() , CameraPositionZ().
		//Search for "Debug c_xy" and enable those lines to see the difference.

		//Changed to this:

		float placeatx_f, placeatz_f;
		placeatx_f = CameraPositionX();
		placeatz_f = CameraPositionZ();

		t.inputsys.domodewaypointcreate = 0;
		t.inputsys.keypress = 1; t.inputsys.domodewaypoint = 1; t.grideditselect = 6;
		if (t.terrain.TerrainID>0)
		{
			g.waypointeditheight_f = BT_GetGroundHeight(t.terrain.TerrainID, placeatx_f, placeatz_f); //
		}
		else
		{
			g.waypointeditheight_f = g.gdefaultterrainheight;
		}
		t.waypointeditstyle = 1; t.waypointeditstylecolor = 0; t.waypointeditentity = 0;
		//t.mx_f = t.cx_f; t.mz_f = t.cy_f;
		t.mx_f = placeatx_f;
		t.mz_f = placeatz_f;
		waypoint_createnew();

		PointCamera(t.mx_f, g.waypointeditheight_f, t.mz_f);
		t.editorfreeflight.c.angx_f = CameraAngleX();
		t.editorfreeflight.c.angy_f = CameraAngleY();
	}


	
	//  fake mousemove values for low-response systems (when in zoomed in mode)
	if (t.grideditselect == 4)
	{
		if (t.inputsys.keyshift == 1)
		{
			if (t.inputsys.keyleft == 1)  t.inputsys.xmousemove = -10;
			if (t.inputsys.keyright == 1)  t.inputsys.xmousemove = 10;
			if (t.inputsys.keyup == 1)
				t.inputsys.ymousemove = -10;
			if (t.inputsys.keydown == 1)  t.inputsys.ymousemove = 10;
			set_inputsys_mclick(2);// t.inputsys.mclick = 2;
			t.inputsys.keyleft = 0;
			t.inputsys.keyright = 0;
			t.inputsys.keyup = 0;
			t.inputsys.keydown = 0;
		}
	}


	//Update statusbar
	//Update statusbar
	++t.interfacestatusbarupdate;
	if (t.interfacestatusbarupdate > 30)
	{
		//  cursor position
		if (g.gridlayershowsingle == 1)
		{
			t.statusbar_s = ""; t.statusbar_s = t.statusbar_s + "CLIP=" + Str(int(t.clipheight_f));
		}
		else
		{
			t.statusbar_s = "CLIP OFF";//"ALL" ; t.t_s=t.strarr_s[80]+":"+t.t_s;
		}

		t.statusbar_s = t.statusbar_s + " | ";
		if (t.inputsys.xmouse == 500000)
		{
			t.strwork = ""; t.statusbar_s = t.statusbar_s + "X: 0 Z: 0";
		}
		else {
			t.strwork = ""; t.statusbar_s = t.statusbar_s + "X:" + Str(t.inputsys.mmx) + " " + "Z:" + Str(t.inputsys.mmy);
		}

		//PE: 17/08/21 reactivated.
		t.statusbar_s = t.statusbar_s + " | ";
		if (t.gridentitygridlock == 0)  t.statusbar_s = t.statusbar_s + "NORMAL";
		if (t.gridentitygridlock == 1)  t.statusbar_s = t.statusbar_s + "SNAP";
		if (t.gridentitygridlock == 2)  t.statusbar_s = t.statusbar_s + "GRID";

		//  editing mode

		//336 = Clipboard Selection Mode (CTRL+C=Copy DELETE=Clear)
		//332 = Terrain Painting Mode:
		//343 = Zoomed In Mode (Right click and drag to view, Left to Exit)
		//344 = Entity Editing Mode (R=Rotate Entity  ENTER=Find Floor/Wall)

		if (t.grideditselect == 0)
		{
			t.laststatusbar_s = t.strarr_s[332];
			terrain_getpaintmode();
			t.laststatusbar_s = t.laststatusbar_s + " " + t.mode_s;
		}
		if (t.grideditselect == 1)  t.laststatusbar_s = t.strarr_s[336];
		if (t.grideditselect == 2)
		{
			//  art tools not used any more
		}
		if (t.grideditselect == 3)
		{
			//  map view mode not used any more
		}

		if (t.grideditselect == 4)  t.laststatusbar_s = t.strarr_s[343];

		if (t.grideditselect == 5)
		{
			t.laststatusbar_s = t.strarr_s[344];
			t.laststatusbar_s = t.laststatusbar_s + " Entity: " + t.relaytostatusbar_s;
		}
		if (t.grideditselect == 6)
		{
			//  add waypoint status
			t.laststatusbar_s = "Waypoint Mode (LMB=Drag Point  SHIFT+LMB=Clone Point  SHIFT+RMB=Remove Point)";
		}


		//  only update infrequently
		t.interfacestatusbarupdate = 0;

		//t.laststatusbar_s = t.statusbar_s;
	}

	cstr WinTitle = "";

	if (strcmp(Lower(Left(g.projectfilename_s.Get(), Len(g.rootdir_s.Get()))), Lower(g.rootdir_s.Get())) == 0)
	{
		WinTitle = Right(g.projectfilename_s.Get(), Len(g.projectfilename_s.Get()) - Len(g.rootdir_s.Get()));
	}
	else
	{
		WinTitle = g.projectfilename_s;
	}

	if (g.projectmodified != 0)  WinTitle = WinTitle + "*";
	if (WinTitle != CurrentWinTitle) {
		//Change windows title
		CurrentWinTitle = WinTitle;
		#ifdef VRTECH
		#ifdef PRODUCTV3
		cstr NewTitle = "VR Quest - ";
		#else
		cstr NewTitle = "GameGuru MAX - ";
		#endif
		if (strnicmp(WinTitle.Get(), "mapbank\\", 8) == 0)
			WinTitle = WinTitle.Get() + 8;
		NewTitle = NewTitle + WinTitle;
		SetWindowTitle(NewTitle.Get());
		#else
		cstr NewTitle = "GameGuru - ";
		if (strnicmp(WinTitle.Get(), "mapbank\\", 8) == 0)
			WinTitle = WinTitle.Get() + 8;
		NewTitle = NewTitle + WinTitle;
		SetWindowTitle(NewTitle.Get());
#endif
	}

	//  Update status bar out of action subroutines
	//gridedit_updatestatusbar();


}
#endif

void input_getcontrols ( void )
{
	//  Some actions are directly triggered by input subroutine
	t.inputsys.doload=0;
	t.inputsys.domodeterrain=0;
	t.inputsys.domodeentity=0;
	t.inputsys.domodemarker=0;
	t.inputsys.domodewaypoint=0;
	t.inputsys.doundo=0;
	t.inputsys.doredo=0;
	t.inputsys.tselcontrol=0;
	t.inputsys.tselcut=0;
	t.inputsys.tselcopy=0;
	t.inputsys.tseldelete=0;

	//  Obtain input data
	if (  g.globals.ideinputmode == 1 ) 
	{
		//input_getfilemapcontrols ( ); //cyb ???
//#ifndef USEOLDIDE
//		imgui_input_getcontrols();
//#else
		input_getfilemapcontrols();
//#endif
	}
	else
	{
		input_getdirectcontrols ( );
	}

	//  Flag reset
	t.inputsys.dorotation=0;
	t.inputsys.domirror=0;
	t.inputsys.doflip=0;
	t.inputsys.doentityrotate=0;
	t.inputsys.dozoomin=0;
	t.inputsys.dozoomout=0;
	t.inputsys.doscrollleft=0;
	t.inputsys.doscrollright=0;
	t.inputsys.doscrollup=0;
	t.inputsys.doscrolldown=0;
	t.inputsys.domapresize=0;
	t.inputsys.dogroundmode=-1;
	t.inputsys.dozoomview=0;
	t.inputsys.dozoomviewmovex=0;
	t.inputsys.dozoomviewmovey=0;
	t.inputsys.dozoomviewmovez=0;
	t.inputsys.dozoomviewrotatex=0;
	t.inputsys.dozoomviewrotatey=0;
	t.inputsys.dozoomviewrotatez=0;
	t.inputsys.dosinglelayer=0;
	t.inputsys.tselfloor=0;
	t.inputsys.tselpaste=0;
	t.inputsys.tselwipe=0;
	t.inputsys.dosaveandrun=0;

	//  Input conditional flags
	if (t.inputsys.kscancode == 0) 
	{
		t.inputsys.keypress = 0;
		#ifdef ENABLEIMGUI
		if (iForceScancode > 0 ) 
		{
			t.inputsys.kscancode = iForceScancode;
			iForceScancode = -1;
		}
		else if (bForceKey) 
		{
			bForceKey = false;
			t.inputsys.k_s = csForceKey;
		}
		#endif
	}


	//  Construction Keys
	if (  t.inputsys.keycontrol == 0 ) 
	{
		// can get marker mode from anywhere
		if ( (t.inputsys.kscancode == Asc("M") || t.inputsys.k_s == "m") && t.inputsys.keypress == 0 ) 
		{
			t.inputsys.domodemarker = 1;
			t.inputsys.keypress = 1; 
		}

		if ( (t.grideditselect == 4 && t.gridentityinzoomview>0) || t.grideditselect == 5 ) 
		{
			if (t.inputsys.k_s == "b" && t.inputsys.keypress == 0)
			{
				t.inputsys.keypress=1; 
				t.gridentitygridlock=t.gridentitygridlock+1;
				if ( t.gridentitygridlock>2 )  
					t.gridentitygridlock = 0;
			}
			if (  t.inputsys.k_s == "y" && t.inputsys.keypress == 0 && g.gentitytogglingoff == 0 ) 
			{
				// only if not EBE
				if ( t.entityprofile[t.gridentity].isebe == 0 )
				{
					t.ttrygridentitystaticmode=1-t.gridentitystaticmode;
					t.ttrygridentity=t.gridentity ; editor_validatestaticmode ( );
				}
				t.inputsys.keypress=1; 
			}
			if (  t.inputsys.k_s == "u" && t.inputsys.keypress == 0 ) 
			{
				//  control auto-flatten
				t.inputsys.keypress=1;
				t.gridedit.autoflatten=1-t.gridedit.autoflatten;
			}
			if (  t.inputsys.k_s == "i" && t.inputsys.keypress == 0 ) 
			{
				//  control entity spray mode
				t.inputsys.keypress=1;
				t.gridedit.entityspraymode=1-t.gridedit.entityspraymode;
			}
			// except when in EBE mode which handles - and + keys for material changing
			if ( t.ebe.on == 0 )
			{
				if (  t.inputsys.k_s == "-" && t.inputsys.keypress == 0 ) { t.gridentitymodifyelement = 1  ; t.inputsys.keypress = 1; }
				if (  t.inputsys.k_s == "=" && t.inputsys.keypress == 0 ) { t.gridentitymodifyelement = 2 ; t.inputsys.keypress = 1; }
			}
		}

		//  editing mode
		if (  t.inputsys.k_s == "t" ) { t.inputsys.domodeterrain = 1  ; t.inputsys.dowaypointview = 0; }
		if (  t.inputsys.k_s == "e" ) { t.inputsys.domodeentity = 1  ; t.inputsys.dowaypointview = 0; }
		if (  t.inputsys.k_s == "p" ) { t.inputsys.domodewaypoint = 1  ; t.inputsys.dowaypointview = 0; }
		if ( t.inputsys.keyspace == 1 && t.inputsys.keypress == 0 ) { t.inputsys.dowaypointview=1-t.inputsys.dowaypointview ; t.inputsys.keypress=1 ; t.lastgrideditselect=-1  ; editor_refresheditmarkers ( ); }

		//  NUM-ROTATE CONTROLS
		if (  t.inputsys.k_s == "r" && t.inputsys.keypress == 0 ) { t.inputsys.dorotation = 1 ; t.inputsys.keypress = 1; }
		if (  t.grideditselect != 4 && t.grideditselect != 0 ) 
		{
			if (  t.inputsys.k_s == "1" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 1  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.k_s == "2" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 2  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.k_s == "3" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 3  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.k_s == "4" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 4  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.k_s == "5" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 5  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.k_s == "6" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 6  ; t.inputsys.keypress = 1; }
			if (  t.inputsys.keyshift == 0 ) 
			{
				if (  t.inputsys.k_s == "0" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 98  ; t.inputsys.keypress = 1; }
			}
			else
			{
				if (  t.inputsys.k_s == "0" && t.inputsys.keypress == 0 ) { t.inputsys.doentityrotate = 99 ; t.inputsys.keypress = 1; }
			}
		}

		// Editing of Map
		if ( t.inputsys.k_s == ","  )  t.inputsys.dozoomin = 1;
		if ( t.inputsys.k_s == "."  )  t.inputsys.dozoomout = 1;

		// TAB Key causes layer edit view control
		if ( t.inputsys.kscancode == 15 && t.inputsys.keypress == 0 ) { t.inputsys.dosinglelayer = 1  ; t.inputsys.keypress = 1; }

		// F1 for help page
		if (t.inputsys.kscancode == 59) editor_showhelppage (1);

		// this is the non-IDE input function (need to consolidate at some point - yucky repeat code!)
	}
	else
	{
		if (  t.inputsys.k_s == "r"  )  t.inputsys.dorotation = 1;
	}

	// track session, and after X minutes, and not shown before, show MAX splash
	static bool g_bMAXSplashShown = false;
	static DWORD g_dwMAXSplashTimer = 0;
	if (g_bMAXSplashShown == false)
	{
		if (g_dwMAXSplashTimer == 0) g_dwMAXSplashTimer = timeGetTime();
		if (timeGetTime() > g_dwMAXSplashTimer + (1000*60*20) ) // 20 minutes
		{
			LPSTR pAlreadyShownSplashFile = "..\\gamegurumaxsplash.dat";
			bool bAlreadyShownThis = false;
			if (FileExist (pAlreadyShownSplashFile) == 0)
			{
				OpenToWrite(2, pAlreadyShownSplashFile);
				WriteString(2, "seen");
				CloseFile(2);
				editor_showhelppage (11);
			}
			g_bMAXSplashShown = true;
		}
	}

	//  Key Map Scroll and Resize
	if (  t.inputsys.keyshift == 0 ) 
	{
		if (  t.inputsys.keyleft == 1  )  t.inputsys.doscrollleft = 3;
		if (  t.inputsys.keyright == 1  )  t.inputsys.doscrollright = 3;
		if (  t.inputsys.keyup == 1  )  t.inputsys.doscrollup = 3;
		if (  t.inputsys.keydown == 1  )  t.inputsys.doscrolldown = 3;
	}
	else
	{
		if (  t.inputsys.keyleft == 1  )  t.inputsys.doscrollleft = 20;
		if (  t.inputsys.keyright == 1  )  t.inputsys.doscrollright = 20;
		if (  t.inputsys.keyup == 1  )  t.inputsys.doscrollup = 20;
		if (  t.inputsys.keydown == 1  )  t.inputsys.doscrolldown = 20;
	}

	//  Mouse Wheel control (170616 - but not when in EBE mode as its used for grid layer control)
	if ( t.ebe.on == 0 )
	{
		if (  t.grideditselect == 4 ) 
		{
			//  Zoomed in View
			t.zoomviewcamerarange_f -= (t.inputsys.wheelmousemove / 10.0);
		}
		else
		{
			//  Non-Zoomed in View
			if (  t.inputsys.keycontrol == 0 ) 
			{
				if (  t.inputsys.wheelmousemove<0  )
					t.inputsys.dozoomout = 1;
				if (  t.inputsys.wheelmousemove>0  )
					t.inputsys.dozoomin = 1;
			}
		}
	}

	//  UndoRedo Keys
	if (  t.inputsys.keycontrol == 1 ) 
	{
		if (  t.inputsys.k_s == ""  )  t.inputsys.undokeypress = 0;
		if (  t.inputsys.k_s == "z" && t.inputsys.undokeypress == 0 ) { t.inputsys.doundo = 1  ; t.inputsys.undokeypress = 1; }
		if (  t.inputsys.k_s == "y" && t.inputsys.undokeypress == 0 ) { t.inputsys.doredo = 1  ; t.inputsys.undokeypress = 1; }
	}

	//  Controls only when in zoomview
	if (  t.grideditselect == 4 ) 
	{
		//  orient arrowkey movement to camera angle
		t.tca_f=WrapValue(CameraAngleY());
		if (  t.tca_f >= 360-45 || t.tca_f <= 45 ) 
		{
			t.txa=1 ; t.txb=2 ; t.txc=0 ; t.txd=0;
			t.tza=0 ; t.tzb=0 ; t.tzc=2 ; t.tzd=1;
		}
		else
		{
			if (  t.tca_f >= 180-45 && t.tca_f <= 180+45 ) 
			{
				t.txa=2 ; t.txb=1 ; t.txc=0 ; t.txd=0;
				t.tza=0 ; t.tzb=0 ; t.tzc=1 ; t.tzd=2;
			}
			else
			{
				if (  t.tca_f <= 180 ) 
				{
					t.txa=0 ; t.txb=0 ; t.txc=2 ; t.txd=1;
					t.tza=2 ; t.tzb=1 ; t.tzc=0 ; t.tzd=0;
				}
				else
				{
					t.txa=0 ; t.txb=0 ; t.txc=1 ; t.txd=2;
					t.tza=1 ; t.tzb=2 ; t.tzc=0 ; t.tzd=0;
				}
			}
		}
		t.inputsys.dozoomviewmovex=0 ; t.inputsys.dozoomviewmovez=0;
		if (  t.inputsys.keyleft == 1 ) { t.inputsys.dozoomviewmovex += t.txa  ; t.inputsys.dozoomviewmovez +=t.tza; }
		if (  t.inputsys.keyright == 1 ) { t.inputsys.dozoomviewmovex += t.txb  ; t.inputsys.dozoomviewmovez += t.tzb; }
		if (  t.inputsys.keyup == 1 ) { t.inputsys.dozoomviewmovex += t.txc  ; t.inputsys.dozoomviewmovez += t.tzc; }
		if (  t.inputsys.keydown == 1 ) { t.inputsys.dozoomviewmovex +=t.txd  ; t.inputsys.dozoomviewmovez += t.tzd; }
		//  control rotation
		if (  t.inputsys.k_s == "1" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatex = 1  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.k_s == "2" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatex = 2  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.k_s == "3" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatey = 1  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.k_s == "4" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatey = 2  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.k_s == "5" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatez = 1  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.k_s == "6" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatez = 2  ; t.inputsys.keypress = 1; }
		if (  t.inputsys.keyshift == 0 ) 
		{
			if (  t.inputsys.k_s == "0" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatex = 98  ; t.inputsys.keypress = 1; }
		}
		else
		{
			if (  t.inputsys.k_s == "0" && t.inputsys.keypress == 0 ) { t.inputsys.dozoomviewrotatex = 99  ; t.inputsys.keypress = 1; }
		}
	}
	if (  t.grideditselect == 4 || t.grideditselect == 5 ) 
	{
		//  control finder (toggled using gridentityautofind value)
		if (  t.inputsys.keyreturn == 1 ) 
		{
			if (  t.gridentityautofind == 0  ) { t.gridentityautofind = 3; }
			if (  t.gridentityautofind == 1  ) { t.gridentityautofind = 2; }
		}
		else
		{
			if (  t.gridentityautofind == 3 ) { t.gridentityautofind = 1  ; t.gridentityusingsoftauto = 0; t.gridentitysurfacesnap = 0; }
			if (  t.gridentityautofind == 2 ) { t.gridentityautofind = 0  ; t.gridentityposoffground = 0 ; t.gridentityusingsoftauto = 1; t.gridentitysurfacesnap = 0; }
		}
		if ( t.gridentityautofind == 1 && t.gridentity>0 ) 
		{
			t.gridentitydroptoground = 1 + t.entityprofile[t.gridentity].forwardfacing;
		}
		else
		{
			t.gridentitydroptoground=0;
		}
		//  control height
		if (  t.grideditselect == 4 ) 
		{
			//  move entity through zoomview system
			if (  t.inputsys.kscancode == 201 ) { t.inputsys.dozoomviewmovey = 2  ; t.gridentityposoffground = 1 ; t.gridentityautofind = 0 ; t.gridentityusingsoftauto = 0; }
			if (  t.inputsys.kscancode == 209 ) { t.inputsys.dozoomviewmovey = 1  ; t.gridentityposoffground = 1 ; t.gridentityautofind = 0 ; t.gridentityusingsoftauto = 0; }
		}
		else
		{
			//  directly move entity (and detatch from terrain) PGUP and PGDN
			if ( t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209 ) 
			{
				if ( t.widget.activeObject == 0 ) 
				{
					editor_handlepguppgdn();
					t.gridentityposoffground=1 ; t.gridentityautofind=0 ; t.gridentityusingsoftauto=0; t.gridentitysurfacesnap=0;
				}
			}
		}
	}

	//  Create a waypoint when instructed to
	if (  t.inputsys.domodewaypointcreate == 1 && t.inputsys.keypress == 0 ) 
	{
		t.inputsys.domodewaypointcreate=0;
		t.inputsys.keypress=1 ; t.inputsys.domodewaypoint=1 ; t.grideditselect=6;
		if (  t.terrain.TerrainID>0 ) 
		{
			g.waypointeditheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.cx_f,t.cy_f);
		}
		else
		{
			g.waypointeditheight_f=g.gdefaultterrainheight;
		}
		t.waypointeditstyle=1 ; t.waypointeditstylecolor=0 ; t.waypointeditentity=0;
		t.mx_f=t.cx_f ; t.mz_f=t.cy_f  ; waypoint_createnew ( );
	}
}

void input_calculatelocalcursor ( void )
{
	// Early warning of resource VIDMEM loss, if matrix gone
	if ( MatrixExist(g.m4_projection) == 0 ) 
	{
		editor_detect_invalid_screen ( );
	}
	// Local cursor calculation
	t.inputsys.picksystemused=0;
	if (  t.grideditselect != 4 ) 
	{
		//  do not change these values if in zoom mode
		SetCurrentCamera (  0 );
		SetCameraRange (  1,300000 );
		t.screenwidth_f=800.0;
		t.screenheight_f=600.0;
		GetProjectionMatrix (  g.m4_projection );
		GetViewMatrix (  g.m4_view );
		t.blank=InverseMatrix(g.m4_projection,g.m4_projection);
		t.blank=InverseMatrix(g.m4_view,g.m4_view);

		#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
		//PE: Do not use (800.0x600.0) Just convert , we need any resolution to work.
		t.tadjustedtoareax_f = ((float) t.inputsys.xmouse / (float)GetDisplayWidth()) / ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
		t.tadjustedtoareay_f = ((float) t.inputsys.ymouse / (float)GetDisplayHeight()) / ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
		#else
		//  work out visible part of full backbuffer (i.e. 1212 of 1360)
		t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
		t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
		//  scale full mouse to fit in visible area
		t.tadjustedtoareax_f = ((t.inputsys.xmouse + 0.0) / 800.0) / t.tadjustedtoareax_f;
		t.tadjustedtoareay_f = ((t.inputsys.ymouse + 0.0) / 600.0) / t.tadjustedtoareay_f;
		#endif

		//  stretch scaled-mouse to projected -1 to +1
		SetVector4 (  g.v4_far,(t.tadjustedtoareax_f*2)-1,-((t.tadjustedtoareay_f*2)-1),0,1 );
		t.tx_f=GetXVector4(g.v4_far);
		t.ty_f=GetYVector4(g.v4_far);
		t.tz_f=GetZVector4(g.v4_far);
		TransformVector4 (  g.v4_far,g.v4_far,g.m4_projection );
		t.tx_f=GetXVector4(g.v4_far);
		t.ty_f=GetYVector4(g.v4_far);
		t.tz_f=GetZVector4(g.v4_far);
		SetVector3 ( g.v3_far, t.tx_f, t.ty_f, t.tz_f );
		TransformVectorCoordinates3 ( g.v3_far, g.v3_far, g.m4_view );
		t.tx_f=GetXVector3(g.v3_far);
		t.ty_f=GetYVector3(g.v3_far);
		t.tz_f=GetZVector3(g.v3_far);
		t.fx_f=CameraPositionX(0);
		t.fy_f=CameraPositionY(0);
		t.fz_f=CameraPositionZ(0);
		t.tx_f=t.tx_f-t.fx_f;
		t.ty_f=t.ty_f-t.fy_f;
		t.tz_f=t.tz_f-t.fz_f;
		t.tt_f=abs(t.tx_f)+abs(t.ty_f)+abs(t.tz_f);
		t.tx_f=t.tx_f/t.tt_f;
		t.ty_f=t.ty_f/t.tt_f;
		t.tz_f=t.tz_f/t.tt_f;
		t.tstep=10000;
		t.tdiststep_f=t.gridzoom_f;
		while ( t.tstep>0 ) 
		{
			t.fx_f=t.fx_f+(t.tx_f*t.tdiststep_f);
			t.fy_f=t.fy_f+(t.ty_f*t.tdiststep_f);
			t.fz_f=t.fz_f+(t.tz_f*t.tdiststep_f);
			// hit ground at
			if ( t.terrain.TerrainID>0 ) 
			{
				if ( BT_GetGroundHeight(t.terrain.TerrainID,t.fx_f,t.fz_f) > t.fy_f )  break;
			}
			else
			{
				if ( 1000.0 > t.fy_f )  break;
			}
			--t.tstep;
		}
		if ( t.tstep == 0 ) 
		{
			// no floor to target, have to get coordinate from pick system
			PickScreen2D23D ( t.inputsys.xmouse, t.inputsys.ymouse, 500 );
			t.fx_f=GetPickVectorX();
			t.fy_f=GetPickVectorY();
			t.fz_f=GetPickVectorZ();
			t.inputsys.picksystemused=1;
		}
		t.tx_f=t.fx_f;
		t.tz_f=t.fz_f;
		t.tilex_f=t.tx_f;
		t.tiley_f=t.tz_f;

		//  restore camera range
		editor_refreshcamerarange ( );
	}
	else
	{
		t.tx_f=t.tilex_f;
		t.tz_f=t.tiley_f;
	}

	//  World cursor position
	t.inputsys.localx_f=t.tx_f;
	t.inputsys.localy_f=t.tz_f;
	t.tx=t.inputsys.localx_f/100.0;
	t.ty=t.inputsys.localy_f/100.0;
	if (  t.tx<0  )  t.tx = 0;
	if (  t.ty<0  )  t.ty = 0;
	if (  t.tx>t.maxx-1  )  t.tx = t.maxx-1;
	if (  t.ty>t.maxy-1  )  t.ty = t.maxy-1;
	t.inputsys.mmx=t.tx ; t.inputsys.mmy=t.ty;

	//  layer height is terrain Floor height
	if (  t.terrain.TerrainID>0 ) 
	{
		t.inputsys.localcurrentterrainheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tx_f,t.tz_f);
	}
	else
	{
		t.inputsys.localcurrentterrainheight_f=g.gdefaultterrainheight;
	}

	// when placing waypoints, include entities as 'ground' to check
	t.inputsys.originallocalx_f = t.inputsys.localx_f;
	t.inputsys.originallocaly_f = t.inputsys.localy_f;

	#ifdef VRTECH
	extern bool bWaypointDrawmode;
	if ( t.gridentitysurfacesnap == 1 || t.onedrag > 0 || bWaypointDrawmode)
	{
		// only when finding place to place entity
		if ( t.gridentity > 0 || t.onedrag > 0 || bWaypointDrawmode)
		{
	#else
	if ( t.gridentitysurfacesnap == 1 || t.onedrag > 0)
	{
		// only when finding place to place entity
		if ( t.gridentity > 0 || t.onedrag > 0)
		{
	#endif
			// get distance of current terrain hit
			float fTDX = t.inputsys.localx_f - CameraPositionX();
			float fTDY = t.inputsys.localcurrentterrainheight_f - CameraPositionY();
			float fTDZ = t.inputsys.localy_f - CameraPositionZ();
			float fTerrDist = sqrt ( fabs(fTDX*fTDX) + fabs(fTDY*fTDY) * fabs(fTDZ*fTDZ) );

			// scan for surface point
			int iEntityOver = findentitycursorobj ( -1 );
			if ( iEntityOver != 0 )
			{
				// get distance of new surface point
				if (t.gridnearcameraclip > 0) 
				{
					fTDX = g.glastpickedx_f - GetFromVectorX();
					fTDY = g.glastpickedy_f - GetFromVectorY();
					fTDZ = g.glastpickedz_f - GetFromVectorZ();
				}
				else 
				{
					fTDX = g.glastpickedx_f - CameraPositionX();
					fTDY = g.glastpickedy_f - CameraPositionY();
					fTDZ = g.glastpickedz_f - CameraPositionZ();
				}
				float fSurfaceDist = sqrt ( fabs(fTDX*fTDX) + fabs(fTDY*fTDY) * fabs(fTDZ*fTDZ) );

				// if surface closer, use that
				if ( fSurfaceDist < fTerrDist )
				{
					t.inputsys.localx_f = g.glastpickedx_f;
					t.inputsys.localcurrentterrainheight_f = g.glastpickedy_f;
					t.inputsys.localy_f = g.glastpickedz_f;
				}
			}
		}
	}


	//  height at which zoom editing happens
	t.layerheight_f=t.zoomviewtargety_f+100.0;
}

void editor_updatemarkervisibility ( void )
{
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entityprofile[t.entid].ismarker != 0 ) 
		{
			t.obj=t.entityelement[t.e].obj;
			if (  t.obj>0 ) 
			{
				if (  ObjectExist(t.obj) == 1 ) 
				{
					if (  t.gridentityhidemarkers == 0 ) 
					{
						ShowObject (  t.obj );
					}
					else
					{
						HideObject (  t.obj );
					}
				}
			}
		}
	}
	if (  t.gridentityhidemarkers == 0 ) 
	{
		waypoint_showall ( );
	}
	else
	{
		waypoint_hideall ( );
	}
}

void editor_disableforzoom ( void )
{
	#ifdef FPSEXCHANGE
	OpenFileMap (  2, "FPSEXCHANGE" );
	SetFileMapDWORD (  2, 850, 1 );
	SetEventAndWait (  2 );
	#endif
}

void editor_enableafterzoom ( void )
{
	#ifdef FPSEXCHANGE
	OpenFileMap (  2, "FPSEXCHANGE" );
	SetFileMapDWORD (  2, 850, 0 );
	SetEventAndWait (  2 );
	editor_cutcopyclearstate ( );
	#endif
}

void editor_init ( void )
{
	image_setlegacyimageloading(true);
	//  Load editor images
	SetMipmapNum(1); //PE: mipmaps not needed.
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\quick-help.png";
	LoadImage (  t.strwork.Get(), g.editorimagesoffset+1 );
	LoadImage (  "editors\\gfx\\memorymeter.png",g.editorimagesoffset+2 );
	LoadImage (  "editors\\gfx\\4.bmp",g.editorimagesoffset+3 );
	LoadImage (  "editors\\gfx\\5.bmp",g.editorimagesoffset+4 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\gurumeditation.png";
	LoadImage (  t.strwork.Get() ,g.editorimagesoffset+5 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\gurumeditationoff.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+6 );

	// +7 reserved (below)

	// Test Game prompt
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\quick-start-testlevel-prompt.png";
	LoadImage ( t.strwork.Get(), g.editorimagesoffset+61 );

	// Cursor for entity highlighting
	LoadImage ( "editors\\gfx\\9.png",g.editorimagesoffset+7 );
	LoadImage ( "editors\\gfx\\13.bmp",g.editorimagesoffset+13 );

	#ifdef VRTECH
	LoadImage ( "editors\\gfx\\14-white.png",g.editorimagesoffset+14 );
	if (!GetImageExistEx(g.editorimagesoffset + 14))
		LoadImage("editors\\gfx\\14.png", g.editorimagesoffset + 14);
	LoadImage ( "editors\\gfx\\14-red.png",g.editorimagesoffset+16 );
	LoadImage ( "editors\\gfx\\14-green.png",g.editorimagesoffset+17 );
	#else
	LoadImage("editors\\gfx\\14.png", g.editorimagesoffset + 14);
	#endif

	LoadImage ( "editors\\gfx\\18.png",g.editorimagesoffset+18 );	
	LoadImage (  "editors\\gfx\\26.bmp",g.editorimagesoffset+26 );
	LoadImage ( "editors\\gfx\\cursor.dds",g.editorimagesoffset+10 );

	//  F9 Edit Mode Graphical Prompts
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-terrain.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+21 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-entity.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+22 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-conkit.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+23 );

	// new images for editor extra help
	image_setlegacyimageloading(true);
	#ifdef VRTECH
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\testgamelayout-noweapons.png";
 	LoadImage (  t.strwork.Get(), g.editorimagesoffset+27 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\testgamelayout-vr.png";
 	LoadImage (  t.strwork.Get(), g.editorimagesoffset+28 );
	#else
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\" + g.language_s + "\\artwork\\gamegurumax.png";
	LoadImage (t.strwork.Get(), g.editorimagesoffset + 27);
	#endif
	image_setlegacyimageloading(false);

	//  Also loaded by interactive mode when active
	#ifdef VRTECH
	///LoadImage (  "languagebank\\neutral\\gamecore\\huds\\interactive\\close-highlight.png",g.interactiveimageoffset+15 );
	#else
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\interactive\\close-highlight.png",g.interactiveimageoffset+15 );
	#endif

	//  for overlays on map editor view
	if (  FileExist("editors\\gfx\\resources.png") == 1 ) 
	{
		LoadImage (  "editors\\gfx\\resources.png",g.editordrawlastimagesoffset+1 );
	}
	if (  FileExist("editors\\gfx\\resourceslow.png") == 1 ) 
	{
		LoadImage (  "editors\\gfx\\resourceslow.png",g.editordrawlastimagesoffset+2 );
	}
	if (  FileExist("editors\\gfx\\resourcesgone.png") == 1 ) 
	{
		LoadImage (  "editors\\gfx\\resourcesgone.png",g.editordrawlastimagesoffset+3 );
	}
	if (  FileExist("editors\\gfx\\resourcesworking.png") == 1 ) 
	{
		LoadImage (  "editors\\gfx\\resourcesworking.png",g.editordrawlastimagesoffset+4 );
	}
	SetMipmapNum(-1);
	image_setlegacyimageloading(false);

	//  Work area entity cursor (placeholder for instance of target expanded by 1.05 to make shell highligher)

	MakeObjectPlane (  t.editor.objectstartindex+5,150,150  ); 
	XRotateObject (  t.editor.objectstartindex+5,90 );
	TextureObject (  t.editor.objectstartindex+5,g.editorimagesoffset+7 );
	SetObjectMask (  t.editor.objectstartindex+5, 1 );
	SetObjectTransparency (  t.editor.objectstartindex+5,2 );
	modifyplaneimagestrip(5,8,1);
	SetObjectCollisionOff (  t.editor.objectstartindex+5 );
	//DisableObjectZDepth (  t.editor.objectstartindex+5 ); //PE: UpdateLayerInner layer 4 do not render bNewZLayerObject in pass 0 so is clipped.
	SetObjectLight (  t.editor.objectstartindex+5,0 );
	HideObject (  t.editor.objectstartindex+5 );
	OffsetLimb (  t.editor.objectstartindex+5,0,0,0,-1 );
	SetObjectEffect ( t.editor.objectstartindex+5, g.guishadereffectindex );

	//  cylinder to indicate resources in editor used (and warning)
	MakeObjectCylinder (  t.editor.objectstartindex+7,50 );
	SetObjectCollisionOff (  t.editor.objectstartindex+7 );
	SetObjectLight (  t.editor.objectstartindex+7,0 );
	DisableObjectZDepth (  t.editor.objectstartindex+7 );
	DisableObjectZRead (  t.editor.objectstartindex+7 );
	TextureObject (  t.editor.objectstartindex+7,g.editordrawlastimagesoffset+1 );
	LockObjectOn (  t.editor.objectstartindex+7 );
	ScaleObject (  t.editor.objectstartindex+7,2,0,2 );
	RotateObject (  t.editor.objectstartindex+7,0,0,90 );
	PositionObject (  t.editor.objectstartindex+7,0,-117.5,200 );
	SetObjectMask (  t.editor.objectstartindex+7, 1 );


	//  Setup camera
	BackdropColor (  Rgb(0,0,0) );
	// `set camera range 10,10000 now set in _editor_overallfunctionality

	SetLightRange (  0,10000 );
	SetAmbientLight (  75 );
	SetCameraFOV (  90 );
	SetAutoCamOff (  );

	//  PositionCamera (  )
	t.gridscale_f=((800/2)/8)/t.gridzoom_f;
	t.workareax=800 ; t.workareay=600;
	#ifdef GGTERRAIN_USE_NEW_TERRAIN
	t.borderx_f=1024.0*50.0 ; t.cx_f=GGORIGIN_X;
	t.bordery_f=1024.0*50.0 ; t.cy_f=GGORIGIN_Z;
	#else
	t.borderx_f=1024.0*50.0 ; t.cx_f=t.borderx_f/2;
	t.bordery_f=1024.0*50.0 ; t.cy_f=t.bordery_f/2;
	#endif
	editor_restoreeditcamera ( );

	//  Reset statu bar Text (  )
	t.statusbar_s="" ; t.laststatusbar_s="";
}

void editor_makeundergroundobj ( void )
{
	//  takes tobj,tobjx#,tobjy#,tobjz#
	t.tobjoffx_f=GetObjectCollisionCenterX(t.tobj);
	t.tobjoffy_f=GetObjectCollisionCenterY(t.tobj);
	t.tobjoffz_f=GetObjectCollisionCenterZ(t.tobj);
	t.tobjsizex_f=ObjectSizeX(t.tobj,1);
	t.tobjsizey_f=ObjectSizeY(t.tobj,1);
	t.tobjsizez_f=ObjectSizeZ(t.tobj,1);
	DeleteObject ( t.tobj );
	MakeObjectBox ( t.tobj,t.tobjsizex_f,t.tobjsizey_f,t.tobjsizez_f );
	//ColorObject ( t.tobj,Rgb(255,255,0) );
	PositionObject ( t.tobj,t.tobjx_f,t.tobjy_f,t.tobjz_f );
	OffsetLimb ( t.tobj,0,t.tobjoffx_f,t.tobjoffy_f,t.tobjoffz_f );
	DisableObjectZRead ( t.tobj );
	SetObjectWireframe ( t.tobj,1 );
	SetObjectMask ( t.tobj, 1 );
	SetObjectEffect ( t.tobj, g.guiwireframeshadereffectindex );
	SetObjectEmissive ( t.tobj, Rgb(255,255,0) );
}

void editor_undergroundscan ( void )
{
	//  will detect entities under the ground and convert them to place holders
	if (  Timer()>t.gundergroundscantime ) 
	{
		t.gundergroundscantime=Timer()+100;
		for ( t.e = 1; t.e <= g.entityelementlist; t.e++ )
		{
			t.tentid=t.entityelement[t.e].bankindex;
			if ( t.entityprofile[t.tentid].ismarker == 0 && t.entityprofile[t.tentid].isebe == 0 ) 
			{
				if ( t.entityelement[t.e].beenmoved == 1 ) 
				{
					t.entityelement[t.e].beenmoved=0;
					t.tobj=t.entityelement[t.e].obj;
					if (  t.tobj>0 ) 
					{
						if (  ObjectExist(t.tobj) == 1 ) 
						{
							// get original vector, transform it and check against terrain floor
							bool bUnderground = true;
							sObject* pObj = GetObjectData(t.tobj);
							for ( int iCorner = 0; iCorner < 8; iCorner++ )
							{
								GGVECTOR3 vec = pObj->collision.vecMin;
								if ( iCorner==1 ) { vec.x = pObj->collision.vecMax.x; }
								if ( iCorner==2 ) { vec.y = pObj->collision.vecMax.y; }
								if ( iCorner==3 ) { vec.x = pObj->collision.vecMax.x; vec.y = pObj->collision.vecMax.y; }
								if ( iCorner==4 ) {                                   vec.z = pObj->collision.vecMax.z; }
								if ( iCorner==5 ) { vec.x = pObj->collision.vecMax.x; vec.z = pObj->collision.vecMax.z; }
								if ( iCorner==6 ) { vec.y = pObj->collision.vecMax.y; vec.z = pObj->collision.vecMax.z; }
								if ( iCorner==7 ) { vec.x = pObj->collision.vecMax.x; vec.y = pObj->collision.vecMax.y; vec.z = pObj->collision.vecMax.z; }
								GGVec3TransformCoord ( &vec, &vec, &pObj->position.matWorld );
								if (  t.terrain.TerrainID>0 ) 
									t.tgrnd_f=BT_GetGroundHeight(t.terrain.TerrainID,vec.x,vec.z);
								else
									t.tgrnd_f=1000.0;
								if ( vec.y > t.tgrnd_f ) 
									bUnderground = false;
							}
							// act if underground status changed (make wireframe)
							if (  t.entityelement[t.e].underground == 0 ) 
							{
								if ( bUnderground == true ) 
								{
									//  entity underground
									t.entityelement[t.e].underground=1;
									t.tobjy_f=ObjectPositionY(t.tobj);
									t.tobjx_f=ObjectPositionX(t.tobj);
									t.tobjz_f=ObjectPositionZ(t.tobj);
									editor_makeundergroundobj ( );
									t.entityelement[t.e].isclone=1;
								}
							}
							else
							{
								if ( bUnderground == false ) 
								{
									//  entity above ground
									t.entityelement[t.e].underground=0;
									t.tte = t.e ; entity_converttoinstance ( );
								}
							}
						}
					}
				}
			}
		}
	}

	//  also detect same for gridentity
	if ( t.gridentity>0 && t.gridentityobj>0 ) 
	{
		if ( t.entityprofile[t.gridentity].isebe == 0 )
		{
			t.tobj=t.gridentityobj;
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				t.tobjx_f=ObjectPositionX(t.tobj);
				t.tobjy_f=ObjectPositionY(t.tobj)+GetObjectCollisionCenterY(t.tobj)+(ObjectSizeY(t.tobj,1)/2)+LimbOffsetY(t.tobj,0);
				t.tobjz_f=ObjectPositionZ(t.tobj);
				if (  t.terrain.TerrainID>0 ) 
				{
					t.tgrnd_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tobjx_f,t.tobjz_f);
				}
				else
				{
					t.tgrnd_f=1000.0;
				}
				if (  t.gridentityunderground == 0 ) 
				{
					if (  t.tobjy_f<t.tgrnd_f ) 
					{
						//  grid entity underground
						t.gridentityunderground=1;
						t.tobj=t.gridentityobj;
						editor_makeundergroundobj ( );
					}
				}
				else
				{
					if (  t.tobjy_f >= t.tgrnd_f ) 
					{
						//  grid entity above ground
						t.gridentityunderground=0;
						gridedit_recreateentitycursor ( );
					}
				}
			}
		}
	}
}

void editor_restoreobjhighlightifnotrubberbanded ( int highlightingtentityobj )
{
	if ( highlightingtentityobj>0 ) 
	{
		if ( ObjectExist(highlightingtentityobj) == 1 ) 
		{
			bool bHighlightedFromRubberBandSelection = false;
			if ( g.entityrubberbandlist.size() > 0 )
			{
				for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
				{
					int e = g.entityrubberbandlist[i].e;
					int tobj = t.entityelement[e].obj;
					if ( highlightingtentityobj == tobj )
						bHighlightedFromRubberBandSelection = true;
				}
			}
			if ( bHighlightedFromRubberBandSelection == false )
			{
				// dehighlight primary highlighted entity
					SetAlphaMappingOn ( highlightingtentityobj, 100 );

				// and also dehighlight any children that may have been highlighted as well
				if ( t.tstoreentityindexofprimaryhightlighted > 0 )
				{
					gridedit_clearentityrubberbandlist();
					t.tstoreentityindexofprimaryhightlighted = 0;
				}

			}
		}
	}
}

void editor_restoreentityhighlightobj ( void )
{
	if ( t.geditorhighlightingtentityobj>0 ) 
	{
		editor_restoreobjhighlightifnotrubberbanded ( t.geditorhighlightingtentityobj );
		t.geditorhighlightingtentityobj=0;
	}
}

void editor_rec_checkifindexinparentchain ( int entityindex, bool* pbPartOfParentChildGroup )
{
	for ( int te = 1; te <= g.entityelementlist; te++ )
	{
		if ( t.entityelement[te].iHasParentIndex == entityindex && t.entityelement[te].obj > 0 )
		{
			*pbPartOfParentChildGroup = true;
			editor_rec_checkifindexinparentchain ( te, pbPartOfParentChildGroup );
		}
	}
}

void editor_rec_addchildrentorubberband ( int entityindex )
{
	for ( int te = 1; te <= g.entityelementlist; te++ )
	{
		if ( t.entityelement[te].iHasParentIndex == entityindex && t.entityelement[te].obj > 0 )
		{
			gridedit_addEntityToRubberBandHighlights ( te );
			editor_rec_addchildrentorubberband ( te );
		}
	}
}

void editor_refreshentitycursor ( void )
{
	//  new highligher for selected entities
	t.tentityobj=0 ; t.tentstaticmode=0;
	if ( t.gridentityobj>0 ) 
	{
		t.tentityobj=t.gridentityobj;
		t.tentstaticmode=t.gridentitystaticmode;
	}
	else
	{
		if ( t.tentitytoselect>0 ) 
		{
			if ( t.tentitytoselect <= ArrayCount(t.entityelement) ) 
			{
				t.tentityobj=t.entityelement[t.tentitytoselect].obj;
				t.tentstaticmode=t.entityelement[t.tentitytoselect].staticflag;
				t.ttentid=t.entityelement[t.tentitytoselect].bankindex;
				if ( t.entityprofile[t.ttentid].ismarker == 0 && t.entityprofile[t.ttentid].isebe == 0 ) 
				{
					if ( t.entityelement[t.tentitytoselect].isclone == 1 && t.entityelement[t.tentitytoselect].underground == 0 ) 
					{
						if ( t.entityelement[t.tentitytoselect].editorlock == 0 ) 
						{
							//  restore clone back to instance if no more entity lock
							t.tobj=t.tentityobj ; t.tte=t.tentitytoselect;
							entity_converttoinstance ( );
						}
					}
				}
			}
		}
	}
	if ( t.tentityobj>0 ) 
	{
		if ( ObjectExist(t.tentityobj) == 1 ) 
		{
			// do not reset if extracted and draggging parent/children around
			if ( t.gridentityextractedindex == 0 )
			{
				editor_restoreentityhighlightobj();
			}

			// if obj is instance, and using entity_basic shader, this sets GlowIntensity constant
			int iAlphaHighlightCode = 100;
			if ( t.tentstaticmode == 0 ) 
			{
				if ( t.gridedit.autoflatten == 1 && t.gridentityobj>0 ) 
					iAlphaHighlightCode = 104;
				else
					iAlphaHighlightCode = 103;
			}
			else
			{
				if ( t.gridedit.autoflatten == 1 && t.gridentityobj>0 ) 
					iAlphaHighlightCode = 102;
				else
					iAlphaHighlightCode = 101;
			}
			SetAlphaMappingOn ( t.tentityobj, iAlphaHighlightCode );
			
			// check if this entity is a parent to children, and highlight them too
			if ( t.tentitytoselect > 0 ) 
			{
				t.tstoreentityindexofprimaryhightlighted = t.tentitytoselect;
				editor_rec_addchildrentorubberband ( t.tentitytoselect );
			}

			// record primary entity object being highlighted
			t.geditorhighlightingtentityobj = t.tentityobj;
		}
	}
	else
	{
		editor_restoreentityhighlightobj ( );
	}
}

void editor_hideall3d ( void )
{
	SetCurrentCamera (  0 );
	PositionCamera (  199999,99999,99999 );
	PointCamera (  199999,100999,99999 );
	if ( gbWelcomeSystemActive == false ) 
	{
		Sync ( ); Sync ( );
	}
}

void editor_restoreeditcamera ( void )
{

	// editor starting camera position - reset camera
	SetCurrentCamera (  0 );
	PositionCamera ( t.cx_f, 600*t.gridzoom_f, t.cy_f );
	PointCamera ( t.cx_f, 0, t.cy_f );
	SetCameraFOV (  90 );
}

void editor_clearlibrary ( void )
{
	// Delete all libraries
	for ( t.tabs = 1; t.tabs <= 3; t.tabs++ )
	{
		SetFileMapDWORD ( 1, 534, t.tabs );
		SetFileMapDWORD ( 1, 542, 1 );
		SetEventAndWait ( 1 );
		while ( GetFileMapDWORD ( 1, 542 ) == 1 ) 
		{
			SetEventAndWait ( 1 );
		}
	}

	// ENTITY TAB
	t.tadd=1;

	// And create default NEW icons
	t.t1_s=t.strarr_s[347] ; t.t2_s="files\\editors\\gfx\\missing.bmp";
	SetFileMapDWORD ( 1, 508, t.tadd );
	SetFileMapString ( 1, 1000, cstr(g.mysystem.root_s+t.t2_s).Get() );
	SetFileMapString ( 1, 1256, t.t1_s.Get() );
	SetFileMapDWORD ( 1, 500, 1 );
	SetEventAndWait ( 1 );
	while ( GetFileMapDWORD(1, 500) == 1 ) 
	{
		SetEventAndWait ( 1 );
	}

	// MARKERS TAB
	t.tadd=2;

	//  Determine if extra ZONES included
	#ifdef VRTECH
	t.tstoryzoneincluded=25;
	if ( g.vrqcontrolmode != 0 )
		t.tstoryzoneincluded=23;
	// Default markers
	for ( t.tt = 0 ; t.tt <= t.tstoryzoneincluded; t.tt++ )
	{
		if ( g.vrqcontrolmode != 0 )
		{
			if (  t.tt == 0 ) { t.t1_s = t.strarr_s[349]  ; t.t2_s = "files\\entitybank\\_markers\\player start.bmp"; }
			if (  t.tt == 1 ) { t.t1_s = t.strarr_s[659]  ; t.t2_s = "files\\entitybank\\_markers\\multiplayer start.bmp"; }
			if (  t.tt == 2 ) { t.t1_s = t.strarr_s[351]  ; t.t2_s = "files\\entitybank\\_markers\\white light.bmp"; }
			if (  t.tt == 3 ) { t.t1_s = t.strarr_s[352]  ; t.t2_s = "files\\entitybank\\_markers\\red light.bmp"; }
			if (  t.tt == 4 ) { t.t1_s = t.strarr_s[353]  ; t.t2_s = "files\\entitybank\\_markers\\green light.bmp"; }
			if (  t.tt == 5 ) { t.t1_s = t.strarr_s[354]  ; t.t2_s = "files\\entitybank\\_markers\\blue light.bmp"; }
			if (  t.tt == 6 ) { t.t1_s = t.strarr_s[355]  ; t.t2_s = "files\\entitybank\\_markers\\yellow light.bmp"; }
			if (  t.tt == 7 ) { t.t1_s = t.strarr_s[356]  ; t.t2_s = "files\\entitybank\\_markers\\purple light.bmp"; }
			if (  t.tt == 8 ) { t.t1_s = t.strarr_s[357]  ; t.t2_s = "files\\entitybank\\_markers\\cyan light.bmp"; }
			if (  t.tt == 9 ) { t.t1_s = t.strarr_s[360]  ; t.t2_s = "files\\entitybank\\_markers\\win zone.bmp"; }
			if (  t.tt == 10 ) { t.t1_s = t.strarr_s[361]  ; t.t2_s = "files\\entitybank\\_markers\\trigger zone.bmp"; }
			if (  t.tt == 11 ) { t.t1_s = "Audio Zone"  ; t.t2_s = "files\\entitybank\\_markers\\audio zone.bmp"; }
			if (  t.tt == 12 ) { t.t1_s = "Video Zone"  ; t.t2_s = "files\\entitybank\\_markers\\video zone.bmp"; }
			if (  t.tt == 13 ) { t.t1_s = "Floor Zone"; t.t2_s = "files\\entitybank\\_markers\\floor zone.bmp"; }
			if (  t.tt == 14 ) { t.t1_s = "Image Zone"; t.t2_s = "files\\entitybank\\_markers\\image zone.bmp"; }
			if (  t.tt == 15 ) { t.t1_s = "Text Zone"; t.t2_s = "files\\entitybank\\_markers\\text zone.bmp"; }
			if (  t.tt == 16 ) { t.t1_s = "Ambience Zone"; t.t2_s = "files\\entitybank\\_markers\\ambience zone.bmp"; }
			if (  t.tt == 17 ) { t.t1_s = "White Spotlight"; t.t2_s = "files\\entitybank\\_markers\\white light spot.bmp"; }
			if (  t.tt == 18 ) { t.t1_s = "Red Spotlight"; t.t2_s = "files\\entitybank\\_markers\\red light spot.bmp"; }
			if (  t.tt == 19 ) { t.t1_s = "Green Spotlight"; t.t2_s = "files\\entitybank\\_markers\\green light spot.bmp"; }
			if (  t.tt == 20 ) { t.t1_s = "Blue Spotlight"; t.t2_s = "files\\entitybank\\_markers\\blue light spot.bmp"; }
			if (  t.tt == 21 ) { t.t1_s = "Yellow Spotlight"; t.t2_s = "files\\entitybank\\_markers\\yellow light spot.bmp"; }
			if (  t.tt == 22 ) { t.t1_s = "Purple Spotlight"; t.t2_s = "files\\entitybank\\_markers\\purple light spot.bmp"; }
			if (  t.tt == 23 ) { t.t1_s = "Cyan Spotlight"; t.t2_s = "files\\entitybank\\_markers\\cyan light spot.bmp"; }
		}
		else
		{
			if (  t.tt == 0 ) { t.t1_s = t.strarr_s[349]  ; t.t2_s = "files\\entitybank\\_markers\\player start.bmp"; }
			if (  t.tt == 1 ) { t.t1_s = t.strarr_s[350]  ; t.t2_s = "files\\entitybank\\_markers\\player checkpoint.bmp"; }
			if (  t.tt == 2 ) { t.t1_s = t.strarr_s[658]  ; t.t2_s = "files\\entitybank\\_markers\\cover zone.bmp"; }
			if (  t.tt == 3 ) { t.t1_s = t.strarr_s[659]  ; t.t2_s = "files\\entitybank\\_markers\\multiplayer start.bmp"; }
			if (  t.tt == 4 ) { t.t1_s = t.strarr_s[351]  ; t.t2_s = "files\\entitybank\\_markers\\white light.bmp"; }
			if (  t.tt == 5 ) { t.t1_s = t.strarr_s[352]  ; t.t2_s = "files\\entitybank\\_markers\\red light.bmp"; }
			if (  t.tt == 6 ) { t.t1_s = t.strarr_s[353]  ; t.t2_s = "files\\entitybank\\_markers\\green light.bmp"; }
			if (  t.tt == 7 ) { t.t1_s = t.strarr_s[354]  ; t.t2_s = "files\\entitybank\\_markers\\blue light.bmp"; }
			if (  t.tt == 8 ) { t.t1_s = t.strarr_s[355]  ; t.t2_s = "files\\entitybank\\_markers\\yellow light.bmp"; }
			if (  t.tt == 9 ) { t.t1_s = t.strarr_s[356]  ; t.t2_s = "files\\entitybank\\_markers\\purple light.bmp"; }
			if (  t.tt == 10 ) { t.t1_s = t.strarr_s[357]  ; t.t2_s = "files\\entitybank\\_markers\\cyan light.bmp"; }
			if (  t.tt == 11 ) { t.t1_s = t.strarr_s[360]  ; t.t2_s = "files\\entitybank\\_markers\\win zone.bmp"; }
			if (  t.tt == 12 ) { t.t1_s = t.strarr_s[361]  ; t.t2_s = "files\\entitybank\\_markers\\trigger zone.bmp"; }
			if (  t.tt == 13 ) { t.t1_s = t.strarr_s[362]  ; t.t2_s = "files\\entitybank\\_markers\\sound zone.bmp"; }
			if (  t.tt == 14 ) { t.t1_s = t.strarr_s[607]  ; t.t2_s = "files\\entitybank\\_markers\\story zone.bmp"; }
			if (  t.tt == 15 ) { t.t1_s = "Floor Zone"; t.t2_s = "files\\entitybank\\_markers\\floor zone.bmp"; }
			if (  t.tt == 16 ) { t.t1_s = "Image Zone"; t.t2_s = "files\\entitybank\\_markers\\image zone.bmp"; }
			if (  t.tt == 17 ) { t.t1_s = "Text Zone"; t.t2_s = "files\\entitybank\\_markers\\text zone.bmp"; }
			if (  t.tt == 18 ) { t.t1_s = "Ambience Zone"; t.t2_s = "files\\entitybank\\_markers\\ambience zone.bmp"; }
			if (  t.tt == 19 ) { t.t1_s = "White Spotlight"; t.t2_s = "files\\entitybank\\_markers\\white light spot.bmp"; }
			if (  t.tt == 20 ) { t.t1_s = "Red Spotlight"; t.t2_s = "files\\entitybank\\_markers\\red light spot.bmp"; }
			if (  t.tt == 21 ) { t.t1_s = "Green Spotlight"; t.t2_s = "files\\entitybank\\_markers\\green light spot.bmp"; }
			if (  t.tt == 22 ) { t.t1_s = "Blue Spotlight"; t.t2_s = "files\\entitybank\\_markers\\blue light spot.bmp"; }
			if (  t.tt == 23 ) { t.t1_s = "Yellow Spotlight"; t.t2_s = "files\\entitybank\\_markers\\yellow light spot.bmp"; }
			if (  t.tt == 24 ) { t.t1_s = "Purple Spotlight"; t.t2_s = "files\\entitybank\\_markers\\purple light spot.bmp"; }
			if (  t.tt == 25 ) { t.t1_s = "Cyan Spotlight"; t.t2_s = "files\\entitybank\\_markers\\cyan light spot.bmp"; }
		}
		SetFileMapDWORD (  1, 508, t.tadd );
		SetFileMapString (  1, 1000, cstr(g.mysystem.root_s+t.t2_s).Get() );
		SetFileMapString (  1, 1256, t.t1_s.Get() );
		SetFileMapDWORD (  1, 500, 1 );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1, 500) == 1 ) 
		{
			SetEventAndWait (  1 );
		}
	}

	//  actual entity names of the markers
	Dim ( t.markerentitybank_s, 30 );
	if ( g.vrqcontrolmode != 0 )
	{
		t.markerentitybank_s[1]="_markers\\player start.fpe";
		t.markerentitybank_s[2]="_markers\\multiplayer start.fpe";
		t.markerentitybank_s[3]="_markers\\white light.fpe";
		t.markerentitybank_s[4]="_markers\\red light.fpe";
		t.markerentitybank_s[5]="_markers\\green light.fpe";
		t.markerentitybank_s[6]="_markers\\blue light.fpe";
		t.markerentitybank_s[7]="_markers\\yellow light.fpe";
		t.markerentitybank_s[8]="_markers\\purple light.fpe";
		t.markerentitybank_s[9]="_markers\\cyan light.fpe";
		t.markerentitybank_s[10]="_markers\\win zone.fpe";
		t.markerentitybank_s[11]="_markers\\trigger zone.fpe";
		t.markerentitybank_s[12] = "_markers\\audio zone.fpe";
		t.markerentitybank_s[13] = "_markers\\video zone.fpe";
		t.markerentitybank_s[14] = "_markers\\floor zone.fpe";
		t.markerentitybank_s[15] = "_markers\\image zone.fpe";
		t.markerentitybank_s[16] = "_markers\\text zone.fpe";
		t.markerentitybank_s[17] = "_markers\\ambience zone.fpe";
		t.markerentitybank_s[18] = "_markers\\white light spot.fpe";
		t.markerentitybank_s[19] = "_markers\\red light spot.fpe";
		t.markerentitybank_s[20] = "_markers\\green light spot.fpe";
		t.markerentitybank_s[21] = "_markers\\blue light spot.fpe";
		t.markerentitybank_s[22] = "_markers\\yellow light spot.fpe";
		t.markerentitybank_s[23] = "_markers\\purple light spot.fpe";
		t.markerentitybank_s[24] = "_markers\\cyan light spot.fpe";
	}
	else
	{
		t.markerentitybank_s[1]="_markers\\player start.fpe";
		t.markerentitybank_s[2]="_markers\\player checkpoint.fpe";
		t.markerentitybank_s[3]="_markers\\cover zone.fpe";
		t.markerentitybank_s[4]="_markers\\multiplayer start.fpe";
		t.markerentitybank_s[5]="_markers\\white light.fpe";
		t.markerentitybank_s[6]="_markers\\red light.fpe";
		t.markerentitybank_s[7]="_markers\\green light.fpe";
		t.markerentitybank_s[8]="_markers\\blue light.fpe";
		t.markerentitybank_s[9]="_markers\\yellow light.fpe";
		t.markerentitybank_s[10]="_markers\\purple light.fpe";
		t.markerentitybank_s[11]="_markers\\cyan light.fpe";
		t.markerentitybank_s[12]="_markers\\win zone.fpe";
		t.markerentitybank_s[13]="_markers\\trigger zone.fpe";
		t.markerentitybank_s[14] = "_markers\\sound zone.fpe";
		t.markerentitybank_s[15] = "_markers\\story zone.fpe";
		t.markerentitybank_s[16] = "_markers\\floor zone.fpe";
		t.markerentitybank_s[17] = "_markers\\image zone.fpe";
		t.markerentitybank_s[18] = "_markers\\text zone.fpe";
		t.markerentitybank_s[19] = "_markers\\ambience zone.fpe";
		t.markerentitybank_s[20] = "_markers\\white light spot.fpe";
		t.markerentitybank_s[21] = "_markers\\red light spot.fpe";
		t.markerentitybank_s[22] = "_markers\\green light spot.fpe";
		t.markerentitybank_s[23] = "_markers\\blue light spot.fpe";
		t.markerentitybank_s[24] = "_markers\\yellow light spot.fpe";
		t.markerentitybank_s[25] = "_markers\\purple light spot.fpe";
		t.markerentitybank_s[26] = "_markers\\cyan light spot.fpe";
	}
	#else
	t.tstoryzoneincluded=27;
	//  Default markers
	for ( t.tt = 0 ; t.tt <= t.tstoryzoneincluded; t.tt++ )
	{
		if (  t.tt == 0 ) { t.t1_s = t.strarr_s[349]  ; t.t2_s = "files\\entitybank\\_markers\\player start.bmp"; }
		if (  t.tt == 1 ) { t.t1_s = t.strarr_s[350]  ; t.t2_s = "files\\entitybank\\_markers\\player checkpoint.bmp"; }
		if (  t.tt == 2 ) { t.t1_s = t.strarr_s[658]  ; t.t2_s = "files\\entitybank\\_markers\\cover zone.bmp"; }
		if (  t.tt == 3 ) { t.t1_s = t.strarr_s[659]  ; t.t2_s = "files\\entitybank\\_markers\\multiplayer start.bmp"; }
		if (  t.tt == 4 ) { t.t1_s = t.strarr_s[351]  ; t.t2_s = "files\\entitybank\\_markers\\white light.bmp"; }
		if (  t.tt == 5 ) { t.t1_s = t.strarr_s[352]  ; t.t2_s = "files\\entitybank\\_markers\\red light.bmp"; }
		if (  t.tt == 6 ) { t.t1_s = t.strarr_s[353]  ; t.t2_s = "files\\entitybank\\_markers\\green light.bmp"; }
		if (  t.tt == 7 ) { t.t1_s = t.strarr_s[354]  ; t.t2_s = "files\\entitybank\\_markers\\blue light.bmp"; }
		if (  t.tt == 8 ) { t.t1_s = t.strarr_s[355]  ; t.t2_s = "files\\entitybank\\_markers\\yellow light.bmp"; }
		if (  t.tt == 9 ) { t.t1_s = t.strarr_s[356]  ; t.t2_s = "files\\entitybank\\_markers\\purple light.bmp"; }
		if (  t.tt == 10 ) { t.t1_s = t.strarr_s[357]  ; t.t2_s = "files\\entitybank\\_markers\\cyan light.bmp"; }
		if (  t.tt == 11 ) { t.t1_s = t.strarr_s[360]  ; t.t2_s = "files\\entitybank\\_markers\\win zone.bmp"; }
		if (  t.tt == 12 ) { t.t1_s = t.strarr_s[361]  ; t.t2_s = "files\\entitybank\\_markers\\trigger zone.bmp"; }
		if ( g.vrqcontrolmode != 0 )
		{
			if (  t.tt == 13 ) { t.t1_s = "Audio Zone"  ; t.t2_s = "files\\entitybank\\_markers\\audio zone.bmp"; }
			if (  t.tt == 14 ) { t.t1_s = "Video Zone"  ; t.t2_s = "files\\entitybank\\_markers\\video zone.bmp"; }
		}
		else
		{
			if (  t.tt == 13 ) { t.t1_s = t.strarr_s[362]  ; t.t2_s = "files\\entitybank\\_markers\\sound zone.bmp"; }
			if (  t.tt == 14 ) { t.t1_s = t.strarr_s[607]  ; t.t2_s = "files\\entitybank\\_markers\\story zone.bmp"; }
		}
		if (  t.tt == 15 ) { t.t1_s = "Floor Zone"; t.t2_s = "files\\entitybank\\_markers\\floor zone.bmp"; }
		if (  t.tt == 16 ) { t.t1_s = "Image Zone"; t.t2_s = "files\\entitybank\\_markers\\image zone.bmp"; }
		if (  t.tt == 17 ) { t.t1_s = "Text Zone"; t.t2_s = "files\\entitybank\\_markers\\text zone.bmp"; }
		if (  t.tt == 18 ) { t.t1_s = "Ambience Zone"; t.t2_s = "files\\entitybank\\_markers\\ambience zone.bmp"; }
		if (  t.tt == 19 ) { t.t1_s = "White Spotlight"; t.t2_s = "files\\entitybank\\_markers\\white light spot.bmp"; }
		if (  t.tt == 20 ) { t.t1_s = "Red Spotlight"; t.t2_s = "files\\entitybank\\_markers\\red light spot.bmp"; }
		if (  t.tt == 21 ) { t.t1_s = "Green Spotlight"; t.t2_s = "files\\entitybank\\_markers\\green light spot.bmp"; }
		if (  t.tt == 22 ) { t.t1_s = "Blue Spotlight"; t.t2_s = "files\\entitybank\\_markers\\blue light spot.bmp"; }
		if (  t.tt == 23 ) { t.t1_s = "Yellow Spotlight"; t.t2_s = "files\\entitybank\\_markers\\yellow light spot.bmp"; }
		if (  t.tt == 24 ) { t.t1_s = "Purple Spotlight"; t.t2_s = "files\\entitybank\\_markers\\purple light spot.bmp"; }
		if (  t.tt == 25 ) { t.t1_s = "Cyan Spotlight"; t.t2_s = "files\\entitybank\\_markers\\cyan light spot.bmp"; }
		if (t.tt == 26) { t.t1_s = t.strarr_s[363]; t.t2_s = "files\\entitybank\\_markers\\Heal zone.bmp"; } // BOTR addition for FPSC 2 GG FPM program
		if (t.tt == 27) { t.t1_s = t.strarr_s[364]; t.t2_s = "files\\entitybank\\_markers\\Hurt zone.bmp"; } // BOTR addition for FPSC 2 GG FPM program

		SetFileMapDWORD (  1, 508, t.tadd );
		SetFileMapString (  1, 1000, cstr(g.mysystem.root_s+t.t2_s).Get() );
		SetFileMapString (  1, 1256, t.t1_s.Get() );
		SetFileMapDWORD (  1, 500, 1 );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1, 500) == 1 ) 
		{
			SetEventAndWait (  1 );
		}
	}

	//  actual entity names of the markers
	Dim ( t.markerentitybank_s, 30 );
	t.markerentitybank_s[1]="_markers\\player start.fpe";
	t.markerentitybank_s[2]="_markers\\player checkpoint.fpe";
	t.markerentitybank_s[3]="_markers\\cover zone.fpe";
	t.markerentitybank_s[4]="_markers\\multiplayer start.fpe";
	t.markerentitybank_s[5]="_markers\\white light.fpe";
	t.markerentitybank_s[6]="_markers\\red light.fpe";
	t.markerentitybank_s[7]="_markers\\green light.fpe";
	t.markerentitybank_s[8]="_markers\\blue light.fpe";
	t.markerentitybank_s[9]="_markers\\yellow light.fpe";
	t.markerentitybank_s[10]="_markers\\purple light.fpe";
	t.markerentitybank_s[11]="_markers\\cyan light.fpe";
	t.markerentitybank_s[12]="_markers\\win zone.fpe";
	t.markerentitybank_s[13]="_markers\\trigger zone.fpe";
	t.markerentitybank_s[16] = "_markers\\floor zone.fpe";
	t.markerentitybank_s[27] = "_markers\\Heal zone.fpe"; // BOTR addition for FPSC 2 GG FPM program
	t.markerentitybank_s[28] = "_markers\\Hurt zone.fpe"; // BOTR addition for FPSC 2 GG FPM program

	if ( g.vrqcontrolmode != 0 )
	{
		t.markerentitybank_s[14] = "_markers\\audio zone.fpe";
		t.markerentitybank_s[15] = "_markers\\video zone.fpe";
	}
	else
	{
		t.markerentitybank_s[14] = "_markers\\sound zone.fpe";
		t.markerentitybank_s[15] = "_markers\\story zone.fpe";
	}
	t.markerentitybank_s[17] = "_markers\\image zone.fpe";
	t.markerentitybank_s[18] = "_markers\\text zone.fpe";
	t.markerentitybank_s[19] = "_markers\\ambience zone.fpe";
	t.markerentitybank_s[20] = "_markers\\white light spot.fpe";
	t.markerentitybank_s[21] = "_markers\\red light spot.fpe";
	t.markerentitybank_s[22] = "_markers\\green light spot.fpe";
	t.markerentitybank_s[23] = "_markers\\blue light spot.fpe";
	t.markerentitybank_s[24] = "_markers\\yellow light spot.fpe";
	t.markerentitybank_s[25] = "_markers\\purple light spot.fpe";
	t.markerentitybank_s[26] = "_markers\\cyan light spot.fpe";
	#endif

	// only if EBE enabled
	if ( g.globals.hideebe == 0 )
	{
		// BUILDER TAB
		t.tadd=3;

		// set maximum to 999
		Dim ( t.ebebank_s, 999 );
		t.ebebankmax = 0;

		// Default builder tool icons
		for ( t.tt = 0; t.tt <= 6; t.tt++ )
		{
			if ( t.tt == 0 ) { t.t1_s = "Add New Site";		t.t2_s = "files\\ebebank\\_builder\\New Site.bmp"; }
			if ( t.tt == 1 ) { t.t1_s = "Cube";				t.t2_s = "files\\ebebank\\_builder\\Cube.bmp"; }
			if ( t.tt == 2 ) { t.t1_s = "Floor";			t.t2_s = "files\\ebebank\\_builder\\Floor.bmp"; }
			if ( t.tt == 3 ) { t.t1_s = "Wall";				t.t2_s = "files\\ebebank\\_builder\\Wall.bmp"; }
			if ( t.tt == 4 ) { t.t1_s = "Column";			t.t2_s = "files\\ebebank\\_builder\\Column.bmp"; }
			if ( t.tt == 5 ) { t.t1_s = "Row";				t.t2_s = "files\\ebebank\\_builder\\Row.bmp"; }
			if ( t.tt == 6 ) { t.t1_s = "Stairs";			t.t2_s = "files\\ebebank\\_builder\\Stairs.bmp"; }
			SetFileMapDWORD ( 1, 508, t.tadd );
			SetFileMapString ( 1, 1000, cstr(g.mysystem.root_s+t.t2_s).Get() );
			SetFileMapString ( 1, 1256, t.t1_s.Get() );
			SetFileMapDWORD ( 1, 500, 1 );
			SetEventAndWait ( 1 );
			while (  GetFileMapDWORD(1, 500) == 1 ) 
			{
				SetEventAndWait (  1 );
			}
		}
		t.ebebank_s[1]="..\\ebebank\\_builder\\New Site.fpe";
		t.ebebank_s[2]="ebebank\\_builder\\Cube.pfb";
		t.ebebank_s[3]="ebebank\\_builder\\Floor.pfb";
		t.ebebank_s[4]="ebebank\\_builder\\Wall.pfb";
		t.ebebank_s[5]="ebebank\\_builder\\Column.pfb";
		t.ebebank_s[6]="ebebank\\_builder\\Row.pfb";
		t.ebebank_s[7]="ebebank\\_builder\\Stairs.pfb";

		// Now scan for extra PFB files not part of default set
		int iFirstFreeSlot = 8;
		LPSTR pOld = GetDir();
		SetDir("ebebank");
		UnDim(t.filelist_s);
		buildfilelist("_builder", "");
		SetDir(pOld);
		int iExtraPFBCount = 0;
		if (ArrayCount(t.filelist_s) > 0)
		{
			for (t.chkfile = 0; t.chkfile <= ArrayCount(t.filelist_s); t.chkfile++)
			{
				t.file_s = t.filelist_s[t.chkfile];
				if (t.file_s != "." && t.file_s != "..")
				{
					if (cstr(Lower(Right(t.file_s.Get(), 4))) == ".pfb")
					{
						// ignore items in default list
						bool bIgnore = false;
						for (int dl = 1; dl < iFirstFreeSlot; dl++)
						{
							LPSTR pThisOne = t.ebebank_s[dl].Get();
							char pNameOnly[256];
							strcpy(pNameOnly, "");
							for (int n = strlen(pThisOne) - 1; n > 0; n--)
							{
								if (pThisOne[n] == '\\' || pThisOne[n] == '/')
								{
									strcpy(pNameOnly, pThisOne + n + 1);
									break;
								}
							}
							if (stricmp(pNameOnly, t.file_s.Get()) == NULL)
								bIgnore = true;
						}
						if (bIgnore == false)
						{
							// add to list
							t.ebebank_s[iFirstFreeSlot + iExtraPFBCount] = cstr("ebebank\\_builder\\") + Left(t.file_s.Get(), Len(t.file_s.Get()));

							// next slot
							iExtraPFBCount++;
							if (iExtraPFBCount > 100) iExtraPFBCount = 100;
						}
					}
				}
			}
			t.strwork = ""; t.strwork = t.strwork + "total extra PFBs=" + Str(iExtraPFBCount);
			timestampactivity(0, t.strwork.Get());
		}
		//  Now sort list into alphabetical order
		for ( t.tgid1 = 0; t.tgid1 < iExtraPFBCount; t.tgid1++ )
		{
			for ( t.tgid2 = 0; t.tgid2 < iExtraPFBCount; t.tgid2++ )
			{
				if (  t.tgid1 != t.tgid2 ) 
				{
					t.tname1_s=Lower(t.ebebank_s[iFirstFreeSlot+t.tgid1].Get());
					t.tname2_s=Lower(t.ebebank_s[iFirstFreeSlot+t.tgid2].Get());
					if ( strlen( t.tname1_s.Get() ) > strlen( t.tname2_s.Get() ) ) 
					{
						//  smallest at top
						t.ebebank_s[iFirstFreeSlot+t.tgid1]=t.tname2_s;
						t.ebebank_s[iFirstFreeSlot+t.tgid2]=t.tname1_s;
					}
				}
			}
		}
		// add to library list
		for ( int n = 0; n < iExtraPFBCount; n++ )
		{
			// create BMP thumbnail
			t.file_s = t.ebebank_s[iFirstFreeSlot+n];
			LPSTR pThisOne = t.file_s.Get();
			char pNameOnly[256];
			strcpy ( pNameOnly, "" );
			for ( int n = strlen(pThisOne)-1; n > 0; n-- )
			{
				if ( pThisOne[n] == '\\' ||  pThisOne[n] == '/' )
				{
					strcpy ( pNameOnly, pThisOne + n + 1 );
					break;
				}
			}
			t.t1_s = Left(pNameOnly,Len(pNameOnly)-4);
			t.t2_s = cstr("files\\") + cstr(Left(t.file_s.Get(),Len(t.file_s.Get())-4)) + cstr(".bmp");
			SetFileMapDWORD ( 1, 508, t.tadd );
			SetFileMapString ( 1, 1000, cstr(g.mysystem.root_s+t.t2_s).Get() );
			SetFileMapString ( 1, 1256, t.t1_s.Get() );
			SetFileMapDWORD ( 1, 500, 1 );
			SetEventAndWait ( 1 );
			while (  GetFileMapDWORD(1, 500) == 1 ) 
			{
				SetEventAndWait (  1 );
			}
		}
		t.ebebankmax = 8 + iExtraPFBCount;
	}

	//  clear counters
	t.locallibraryentidmaster=0;
	t.locallibraryentindex=0;

	//  Ensure start with entity tab
	editor_leftpanelreset ( );
}

void editor_filllibrary ( void )
{
	//  Store place before adds
	SetEventAndWait (  1 );
	t.tstoredtabindex=GetFileMapDWORD( 1, 520 );

	//  Ensure entity list is up to date in library
	while ( t.locallibraryentidmaster<g.entidmaster ) 
	{
		//  only if not marker
		++t.locallibraryentidmaster;
		t.t2_s = t.entityprofileheader[t.locallibraryentidmaster].desc_s;

		// named EBE entities can be shown
		bool bShowEntityInLocalLibrary = true;
		if (t.entityprofile[t.locallibraryentidmaster].isebe != 0)
		{
			if ( stricmp ( t.t2_s.Get(), "new site" ) == NULL ) bShowEntityInLocalLibrary = false;
			if ( strnicmp ( t.t2_s.Get(), "ebe", 3 ) == NULL ) 
			{
				// are the characters after 'ebe' numbers?
				bool bIsNumber = false;
				LPSTR pEntName = t.t2_s.Get();
				for ( int n = 3; n < (int)strlen(pEntName); n++ )
				{
					if ( pEntName[n] >= '0' && pEntName[n] <= '9' )
						bIsNumber = true;
					else
					{
						bIsNumber = false;
						break;
					}
				}
				if ( bIsNumber == true )
				{
					bShowEntityInLocalLibrary = false;
				}
			}
			LPSTR pEntityBankFilename = t.entitybank_s[t.locallibraryentidmaster].Get();
			pEntityBankFilename += strlen(pEntityBankFilename)-4;
			if ( stricmp ( pEntityBankFilename, ".fpe" ) != NULL )
			{
				bShowEntityInLocalLibrary = false;
			}
		}
		if ( bShowEntityInLocalLibrary == true )
		{
			if ( t.entityprofile[t.locallibraryentidmaster].ismarker == 0 || t.entityprofile[t.locallibraryentidmaster].ismarker == 4 ) 
			{
				//  add to actual list
				t.ttext_s=t.entitybank_s[t.locallibraryentidmaster];
				t.tbitmap_s=cstr("files\\entitybank\\")+t.ttext_s;
				t.t1_s = ""; t.t1_s=t.t1_s + Left(t.tbitmap_s.Get(),Len(t.tbitmap_s.Get())-4)+".bmp";
				if (  FileExist( cstr(cstr("..\\")+t.t1_s).Get() ) == 0  )  t.t1_s = "files\\editors\\gfx\\missing.bmp";
				SetFileMapDWORD (  1, 508, 1 );
				SetFileMapString (  1, 1000, Left(cstr(g.mysystem.root_s+t.t1_s).Get(),254) );
				SetFileMapString (  1, 1256, Left(t.t2_s.Get(),254) );
				SetFileMapDWORD (  1, 500, 1 );
				SetEventAndWait (  1 );
				while (  GetFileMapDWORD(1, 500) == 1 ) 
				{
					SetEventAndWait (  1 );
				}

				//  add to internal list array
				++t.locallibraryentindex;
				Dim (  t.locallibraryent,t.locallibraryentindex  );
				t.locallibraryent[t.locallibraryentindex] = t.locallibraryentidmaster;
			}
		}
	}

	//  Restore place after adds
	SetFileMapDWORD (  1, 534, 1+t.tstoredtabindex );
	SetEventAndWait (  1 );
}

void editor_leftpanelreset ( void )
{
	// Reset to GetPoint ( to entity tab )
	SetFileMapDWORD (  1, 534, 1 );
	SetEventAndWait (  1 );
}

void editor_filemapdefaultinitfornew ( void )
{
	//  Open for some Defaults for Editor
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	#endif

	//  Marker Defaults
	g.entidmaster=0;

	//  filllibrary with segments and entities from default prefabs (temp as is above)
	editor_filllibrary ( );
	editor_leftpanelreset ( );
}

void editor_filemapinit ( void )
{
	// Open for some Defaults for Editor
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	// Set default mouse position and visibility
	SetFileMapDWORD (  1, 0, 400 );
	SetFileMapDWORD (  1, 4, 300 );
	SetEventAndWait (  1 );
	#endif

	//  Each selection tab needs a NEW icon
	editor_clearlibrary ( );
	editor_filllibrary ( );
	editor_leftpanelreset ( );
}

void editor_loadcfg ( void )
{
	//  Load existing config file
	cstr cfgfile_s = g.mysystem.editorsGridedit_s + "cfg.cfg";
	if ( FileExist(cfgfile_s.Get()) == 1 ) 
	{
		OpenToRead (  1,cfgfile_s.Get() );
		//  Current Camera Position
		t.cx_f = ReadFloat ( 1 );
		t.cy_f = ReadFloat ( 1 );
		t.gridzoom_f = ReadFloat ( 1 );
		t.gridlayer = ReadLong ( 1 );
		//  Edit Vars
		t.nogridsmart = ReadLong ( 1 );
		t.grideditartmode = ReadLong ( 1 );

		// LB: modes 3 and 4 are view-modes (should not restore into these, no way out!!)
		int iTestGridEditSelect = ReadLong ( 1 );
		if (iTestGridEditSelect == 3 || iTestGridEditSelect == 4)
		{
			t.grideditselect = 0;
		}
		else
		{
			t.grideditselect = iTestGridEditSelect;
		}
		//  Project (only need project name if skipping FPM=using temp.fpm)
		t.temp_s = ReadString ( 1 ); if (  t.skipfpmloading == 1  )  g.projectfilename_s = t.temp_s;
		g.currentFPG_s = ReadString ( 1 );

		//  Shroud Settings
		t.a = ReadLong ( 1 );
		g.gridlayershowsingle = ReadLong ( 1 );

		CloseFile (  1 );
	}

	//  Reset editor slicing for now
	g.gridlayershowsingle=0;

	//  Update editor settings
	editor_refresheditmarkers ( );
	t.refreshgrideditcursor=1;
	t.updatezoom=1;

	//  Current project name stored for next time
	t.currentprojectfilename_s=g.projectfilename_s;

	return;
}

void editor_savecfg ( void )
{
	//  Delete config file
	//t.strwork = "" ; t.strwork = t.strwork + "editors\\gridedit\\cfg.cfg";
	t.strwork = g.mysystem.editorsGridedit_s + "cfg.cfg";
	if (  FileExist( t.strwork.Get() ) == 1  )  DeleteAFile ( t.strwork.Get() );

	//  Save config file
	OpenToWrite (  1, t.strwork.Get() );

	//  Current Camera Position
	if (  t.editorfreeflight.mode == 1 ) 
	{
		//  when save while in free flight mode, use present location
		t.a_f=t.editorfreeflight.c.x_f ; WriteFloat (  1,t.a_f );
		t.a_f=t.editorfreeflight.c.z_f ; WriteFloat (  1,t.a_f );
	}
	else
	{
		WriteFloat (  1,t.cx_f );
		WriteFloat (  1,t.cy_f );
	}
	WriteFloat (  1,t.gridzoom_f );
	WriteLong (  1,t.gridlayer );
	//  Edit Vars
	WriteLong (  1,t.nogridsmart );
	WriteLong (  1,t.grideditartmode );
	WriteLong (  1,t.grideditselect );
	//  Project
	WriteString (  1,g.projectfilename_s.Get() );
	WriteString (  1,g.currentFPG_s.Get() );
	//  Shroud Settings
	WriteLong (  1,g.shroudsize );
	WriteLong (  1,g.gridlayershowsingle );
CloseFile (  1 );

return;

}

void editor_constructionselection ( void )
{
	if ( t.inputsys.constructselection>0 ) 
	{
		//  SINGLE ENTITY
		if ( t.grideditselect == 5 ) 
		{
			if ( t.inputsys.constructselection <= g.entidmaster ) 
			{
				#ifdef ENABLEIMGUI
				CloseDownEditorProperties();
				#endif

				//  first cancel any widget that might be opened
				widget_switchoff ( );

				//PE: Somebody removed this line in Classic ???? Nothing worked ????
				t.gridentity = t.inputsys.constructselection;

				// remove any entity group rubber band highlighting
				{
					// if not a group smart object
					gridedit_clearentityrubberbandlist();
				}


				// the entity ID we are adding
				if ( t.entityprofile[t.gridentity].isebe > 0 )
				{
					// create unique entid and go to entity placement mode
					char pEBEFile[512];
					strcpy ( pEBEFile, t.entitybank_s[t.gridentity].Get());
					t.addentityfile_s = cstr(Left(pEBEFile,strlen(pEBEFile)-4)) + cstr(".fpe");

					#ifdef ENABLEIMGUI
					CloseDownEditorProperties();
					#endif

					// Work out EBE file and check if it exists
					char pFinalPathAndFile[1024];
					sprintf ( pFinalPathAndFile, "entitybank%s.ebe", Left(pEBEFile,strlen(pEBEFile)-4) );
					if ( FileExist ( pFinalPathAndFile ) )
					{
						// by creating one unique to the level, we can save our temp changes to it
						entity_adduniqueentity ( true );
						t.gridentity = t.entid;

						// name only
						char pNameOnly[256];
						strcpy ( pNameOnly, "" );
						for ( int n = strlen(pEBEFile)-1; n > 0; n-- )
						{
							if ( pEBEFile[n] == '\\' ||  pEBEFile[n] == '/' )
							{
								strcpy ( pNameOnly, pEBEFile + n + 1 );
								break;
							}
						}
						t.t1_s = Left(pNameOnly,Len(pNameOnly)-4);

						// give it a unique name
						t.entitybank_s[t.entid] = t.t1_s;
						t.entityprofileheader[t.entid].desc_s = t.t1_s;

						// load EBE data into entityID
						ebe_load_ebefile ( pFinalPathAndFile, t.entid );

						// get path only
						char pFinalPathOnly[1024];
						strcpy ( pFinalPathOnly, pFinalPathAndFile );
						for ( int n = strlen(pFinalPathAndFile); n > 0; n-- )
						{
							if ( pFinalPathAndFile[n] == '\\' || pFinalPathAndFile[n] == '/' )
							{
								pFinalPathOnly[n+1] = 0;
								break;
							}
						}

						// copy unique texture into levelbank\testmap so EDIT can copy over to ebebank
						cstr sUniqueFilename = t.entityprofile[t.entid].texd_s;
						sUniqueFilename = cstr(Left(sUniqueFilename.Get(),strlen(sUniqueFilename.Get())-6));
						cstr sDDSSourceFile = cstr(pFinalPathOnly) + sUniqueFilename + cstr("_D.dds");
						cstr sDDSFile = g.mysystem.levelBankTestMap_s + sUniqueFilename + cstr("_D.dds");
						if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
						CopyFileA ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
						sDDSSourceFile = cstr(pFinalPathOnly) + sUniqueFilename + cstr("_N.dds");
						sDDSFile = g.mysystem.levelBankTestMap_s + sUniqueFilename + cstr("_N.dds");
						if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
						CopyFileA ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
						sDDSSourceFile = cstr(pFinalPathOnly) + sUniqueFilename + cstr("_S.dds");
						sDDSFile = g.mysystem.levelBankTestMap_s + sUniqueFilename + cstr("_S.dds");
						if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
						CopyFileA ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
					}
					else
					{
						// EBE not present, which means user protected it (not an editable EBE any more)
						if ( stricmp ( pEBEFile, "..\\ebebank\\_builder\\New Site.fpe" ) != NULL )
						{
							//New site is called EBE? 
							//if (!(pEBEFile[0] == 'E' && pEBEFile[1] == 'B' && pEBEFile[2] == 'E'))
							if(t.entityprofile[t.gridentity].model_s != "New Site.X" )  //Better way.
							{
								// except New Site of course
								t.entityprofile[t.gridentity].isebe = 0;
							}
						}
					}
				}
				//  select entity profile and start orientation
				t.gridedit.autoflatten=t.entityprofile[t.gridentity].autoflatten;
				t.gridedit.entityspraymode=0;
				t.inputsys.dragoffsetx_f=0;
				t.inputsys.dragoffsety_f=0;
				t.gridentityposoffground=0;

				if ( t.entityprofile[t.gridentity].dontfindfloor != 0 )
				{
					// can set entity to initially ignore floor finder
					t.gridentityusingsoftauto = 0;
				}
				else
				{
					t.gridentityusingsoftauto = 1;
				}
				if ( t.entityprofile[t.gridentity].defaultstatic == 0 && t.entityprofile[t.gridentity].isimmobile == 1 ) 
				{
					//  some entities should NOT use auto-find (some collisionmode entities)
					//  11=no physics  21=player repell feature  50-59=tree collisions
					t.tcollmode=t.entityprofile[t.gridentity].collisionmode;
					if ( t.tcollmode == 11 || t.tcollmode == 12 || t.tcollmode == 21 || (t.tcollmode >= 50 && t.tcollmode <= 59) ) 
					{
						t.gridentityautofind=0;
					}
					else
					{
						t.gridentityautofind=1;
					}
				}
				else
				{
					t.gridentityautofind=0;
				}


				t.gridentityeditorfixed=0;
				if (!bRotScaleAlreadyUpdated)
				{
					t.gridentityrotatex_f = t.entityprofile[t.gridentity].rotx;
					t.gridentityrotatey_f = t.entityprofile[t.gridentity].roty;
					t.gridentityrotatez_f = t.entityprofile[t.gridentity].rotz;
					t.gridentityscalex_f = t.entityprofile[t.gridentity].scale;
					t.gridentityscaley_f = t.entityprofile[t.gridentity].scale;
					t.gridentityscalez_f = t.entityprofile[t.gridentity].scale;
				}
				bRotScaleAlreadyUpdated = false;
				t.ttrygridentitystaticmode=t.entityprofile[t.gridentity].defaultstatic;
				t.ttrygridentity=t.gridentity ; editor_validatestaticmode ( );
				//  Ensure editor zoom refreshes
				t.updatezoom=1;
				//  fill new selection with defaults
				t.sentid=t.entid ; t.entid=t.gridentity  ; entity_fillgrideleproffromprofile() ; t.entid=t.sentid;
				t.grideleproflastname_s=t.grideleprof.name_s;
				//  marker types?
				if ( t.entityprofile[t.gridentity].ismarker == 1 && t.entityprofile[t.gridentity].lives != -1 ) 
				{
					//  selecting new player start marker resets tweakables
					physics_inittweakables ( );
				}
				if ( t.entityprofile[t.gridentity].ismarker == 3 || t.entityprofile[t.gridentity].ismarker == 6 || t.entityprofile[t.gridentity].ismarker == 8 ) 
				{
					//  trigger zone marker(3) or checkpoint marker(6) or floor zone marker(8)
					//  trigger zone has a waypoint zone companion
					if ( t.entityprofile[t.gridentity].ismarker == 8 ) 
						t.waypointeditstyle = 3; // navmeshzone
					else
						t.waypointeditstyle = 2; // normalzone
					t.waypointeditstylecolor=t.entityprofile[t.gridentity].trigger.stylecolor;
					t.waypointeditentity=0;
					t.mx_f=t.cx_f ; t.mz_f=t.cy_f;
					if (  t.terrain.TerrainID>0 ) 
					{
						g.waypointeditheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.mx_f,t.mz_f);
					}
					else
					{
						g.waypointeditheight_f=g.gdefaultterrainheight;
					}
					waypoint_createnew ( );
					t.grideleprof.trigger.waypointzoneindex=t.waypointindex;
				}
			}
		}

		//  In case new 'shader' associated with new entity, refresh just in case (i.e. first entity)
		visuals_justshaderupdate ( );

		//  Construction complete
		t.inputsys.constructselection = 0;
	}
}

void editor_validatestaticmode ( void )
{
	// receives ttrygridentitystaticmode,ttrygridentity
	if ( t.ttrygridentity>0 ) 
	{
		t.gridentitystaticmode=t.ttrygridentitystaticmode;
		bool bSomeShadersForceDynamicMode = false;
		if ( strcmp ( Lower(Right(t.entityprofile[t.ttrygridentity].effect_s.Get(),18)) , "character_basic.fx" ) == 0 ) bSomeShadersForceDynamicMode = true;
		if ( strcmp ( Lower(Right(t.entityprofile[t.ttrygridentity].effect_s.Get(),14)) , "treea_basic.fx" ) == 0 ) bSomeShadersForceDynamicMode = true;
		if ( bSomeShadersForceDynamicMode == true )
		{
			if ( ObjectExist(g.entitybankoffset+t.ttrygridentity) == 1 ) 
			{
				if ( GetNumberOfFrames(g.entitybankoffset+t.ttrygridentity)>0 ) 
				{
					t.gridentitystaticmode=0;
				}
			}
		}
	}
}

void editor_overallfunctionality ( void )
{
	//  Restore current grid view
	if (  t.inputsys.doautozoomview == 1 ) { t.inputsys.doautozoomview = 0  ; t.inputsys.dozoomview = 1; }
	if (  t.inputsys.dozoomview == 1 ) 
	{
		if (  t.cameraviewmode == 2 ) 
		{
			//  mouselook mode off
			#ifdef FPSEXCHANGE
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetFileMapDWORD (  1, 48, 0 );
			SetEventAndWait (  1 );
			#endif
			//  re-enable icons
			editor_enableafterzoom ( );
			//  end zoom mode
			t.grideditselect=t.stgrideditselect  ; editor_refresheditmarkers ( );
			t.inputsys.dozoomview=0;
			t.cameraviewmode=0;
		}
	}

	//  Switch to zoom view
	if (  t.inputsys.dozoomview == 1 ) 
	{
		if (  t.cameraviewmode == 0 ) 
		{
			//  Set camera to track with close-up
			t.stgrideditselect=t.grideditselect;
			t.cameraviewmode = 2;

			//  Mode - Zoom In View
			t.grideditselect=4 ; editor_refresheditmarkers ( );
			t.updatezoom=1;
		}
	}

	//  Get terrain height reading at cursor
	if (  t.terrain.TerrainID>0 ) 
	{
		t.ttterrheighthere_f=BT_GetGroundHeight(t.terrain.TerrainID,t.cx_f,t.cy_f);
	}
	else
	{
		t.ttterrheighthere_f=g.gdefaultterrainheight;
	}
	
	//  ensure zoom never penetrates terrain
	if (  t.updatezoom == 1 || t.inputsys.mclick != 0 ) 
	{
		if (  (600.0*t.gridzoom_f)<(t.ttterrheighthere_f+100) ) 
		{
			t.gridzoom_f=(t.ttterrheighthere_f+100)/600.0;
		}
	}

	//  Recalculate zoom scale for editing
	if (  t.updatezoom == 1 ) 
	{

		//  grid scale for camera cursor location and zoom
		t.gridscale_f=((800/2)/8)/t.gridzoom_f;
		t.inputsys.keypress=1;
		t.updatezoom=0;

		//  gridlayershowsingle creates an alpha slice in entity shaders
		t.gridnearcameraclip=-1;
		if (  t.grideditselect != 4 ) 
		{
			if (  g.gridlayershowsingle == 1 ) 
			{
				t.gridnearcameraclip=t.clipheight_f;
			}
		}

		//  modulate shadow strength based on distance
		t.tcamrange_f=((600.0*t.gridzoom_f)+1000);
		t.toldvisualsshadowmode=t.visuals.shadowmode;
		if (  t.tcamrange_f<4000 ) 
		{
			t.visuals.shadowmode=100;
		}
		else
		{
			if (  t.tcamrange_f<6000 ) 
			{
				t.visuals.shadowmode=(6000-t.tcamrange_f)/20.0;
			}
			else
			{
				t.visuals.shadowmode=0;
			}
		}
		if (  t.toldvisualsshadowmode != t.visuals.shadowmode ) 
		{
			visuals_justshaderupdate ( );
		}

		//  adjust clipping range of camera to match
		editor_refreshcamerarange ( );

		//  Ensure the slicing clip does not go
		if (  t.gridnearcameraclip == -1 ) 
		{
			t.gridtrueslicey_f=CameraPositionY(0);
		}
		else
		{
			t.gridtrueslicey_f=t.gridnearcameraclip;
		}

		//  feed alpha slicing height into all entity shaders
		if (  g.effectbankmax>0 ) 
		{
			for ( t.t = 1 ; t.t<=  g.effectbankmax; t.t++ )
			{
				t.effectid=g.effectbankoffset+t.t;
				if (  GetEffectExist(t.effectid) == 1 ) 
				{
					if (  t.gridnearcameraclip == -1 ) 
					{
						SetVector4 ( g.terrainvectorindex, 500000, 1, 0, 0 );
					}
					else
					{
						SetVector4 ( g.terrainvectorindex, t.gridtrueslicey_f, 1, 0, 0 );
					}
					SetEffectConstantV (  t.effectid,"EntityEffectControl",g.terrainvectorindex );
				}
			}
		}

	}

	//  use intersect test to find ground/wall and drop entity onto it
	if ( t.inputsys.k_s != "l" ) 
	{
		// but not if holding L key to link entity to a new parent
		if ( t.gridentitysurfacesnap == 1 )
		{
			// no need to find entity, surfacesnap already found best 3D coordinate
		}
		else
		{
			if (t.gridentitydroptoground >= 1 && t.gridentitydroptoground <= 2)
			{
				//PE: Need rubberband support here.
				float fdiff = t.gridentityposy_f;
				t.thardauto = 1; editor_findentityground();
				if (t.gridentityposoffground == 0)
				{
				}
				t.gridentitydroptoground=0;
			}
			else
			{
				t.thardauto=0 ; editor_findentityground ( );
			}
		}
	}

	//  Change layer show mode
	if (  t.inputsys.dosinglelayer == 1 ) 
	{
		g.gridlayershowsingle=g.gridlayershowsingle+1;
		if (  g.gridlayershowsingle>1  )  g.gridlayershowsingle = 0;
		t.updatezoom=1;
	}

	//  ensure assigned third person char object stays with start marker
	if (  t.playercontrol.thirdperson.enabled == 1 ) 
	{
		t.tobj=t.entityelement[t.playercontrol.thirdperson.charactere].obj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				if (  t.gridentity>0 && t.entityprofile[t.gridentity].ismarker == 1 ) 
				{
					//  moving start marker
					t.tstmrkobj=t.gridentityobj;
				}
				else
				{
					//  update char on start marker entity
					t.tstmrke=t.playercontrol.thirdperson.startmarkere;
					t.tstmrkobj=t.entityelement[t.tstmrke].obj;
				}
				if (  t.tstmrkobj>0 ) 
				{
					if (  ObjectExist(t.tstmrkobj) == 1 ) 
					{
						PositionObject (  t.tobj,ObjectPositionX(t.tstmrkobj),ObjectPositionY(t.tstmrkobj),ObjectPositionZ(t.tstmrkobj) );
						RotateObject (  t.tobj,ObjectAngleX(t.tstmrkobj),ObjectAngleY(t.tstmrkobj),ObjectAngleZ(t.tstmrkobj) );
					}
				}
				MoveObject (  t.tobj,-35 );
				if (  t.tstmrkobj>0 ) 
				{
					if (  ObjectExist(t.tstmrkobj) == 1 ) 
					{
						EnableObjectZDepth (  t.tstmrkobj );
						EnableObjectZWrite (  t.tstmrkobj );
						EnableObjectZRead (  t.tstmrkobj );
					}
				}
			}
		}
	}
}

void editor_refreshcamerarange ( void )
{
	t.tcamneardistance_f=CameraPositionY(0)/500.0;
	if ( t.tcamneardistance_f < 10.0  ) t.tcamneardistance_f = 10.0;
	if ( t.widget.activeObject > 0 )
	{
		// 011215 - except when widget shown, we need min distance to avoid clipping widget
		if ( t.tcamneardistance_f > 30.0f ) 
		{
			// to avoid water plane clipping, move water plane away from terrain plate incrementally
			t.terrain.waterlineyadjustforclip_f = (t.tcamneardistance_f-30.0f) * 5;
			t.tcamneardistance_f = 30.0f;
		}
	}
	else
	{
		t.terrain.waterlineyadjustforclip_f = 0.0f;
	}
	if (  t.editorfreeflight.mode == 1 ) 
	{
		// free flight FULL camera distance
		SetCameraRange ( t.tcamneardistance_f, 300000 );
	}
	else
	{
		//  top down camera distance
		//SetCameraRange (  t.tcamneardistance_f,t.tcamrange_f );
		//PE: Test
		SetCameraRange(t.tcamneardistance_f, 300000);
	}
}

void editor_mainfunctionality ( void )
{
	//  Rotation of entity
	if (  t.grideditselect == 5 ) 
	{
		bool bAllowRotate = true;
		//  do not rotate light or trigger entity
		//PE: Allow light rotation rem: t.entityprofile[t.gridentity].ismarker != 2 &&  t.entityprofile[t.gridentity].ismarker != 3
		if ( bAllowRotate && t.entityprofile[t.gridentity].ismarker != 3 )
		{
			if (  t.inputsys.keyshift == 1 ) 
			{
				t.tspeedofrot_f=10.0 ; t.inputsys.keypress=0;
			}
			else
			{
				if (  t.inputsys.keycontrol == 1 ) 
				{
					t.tspeedofrot_f=1.0;
				}
				else
				{
					t.tspeedofrot_f=45.0;
				}
			}
			//PE: Prefer gridentity rotation. as we can now have both active at the same time.
			if (  t.widget.pickedObject != 0 && t.widget.pickedEntityIndex>0 )
			{
				//  Rotation control of widget controlled entity
				if (  t.inputsys.domodeterrain == 0 && t.inputsys.domodeentity == 0 ) 
				{
					if (t.inputsys.dorotation == 1 || 
						(t.inputsys.doentityrotate >= 1 && t.inputsys.doentityrotate <= 6) ||
						t.inputsys.keyreturn == 1 || t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209
						)
					{
					}
					//  avoid interference from terrain/entity mode change
					GGQUATERNION quatRotationEvent = { 0,0,0,0 };
					float fMoveAngX = 0.0f;
					float fMoveAngY = 0.0f;
					float fMoveAngZ = 0.0f;
					float fStoreOrigYAngle = t.entityelement[t.widget.pickedEntityIndex].ry;
					bool bRotateObjectFromKeyPress = false;
					if (t.inputsys.dorotation == 1) { fMoveAngY = fMoveAngY + t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 1) { fMoveAngX = fMoveAngX - t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 2) { fMoveAngX = fMoveAngX + t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 3) { fMoveAngY = fMoveAngY - t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 4) { fMoveAngY = fMoveAngY + t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 5) { fMoveAngZ = fMoveAngZ - t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (t.inputsys.doentityrotate == 6) { fMoveAngZ = fMoveAngZ + t.tspeedofrot_f; bRotateObjectFromKeyPress = true; }
					if (bRotateObjectFromKeyPress == true)
					{
						GGQUATERNION QuatAroundX, QuatAroundY, QuatAroundZ;
						GGQuaternionRotationAxis(&QuatAroundX, &GGVECTOR3(1, 0, 0), GGToRadian(fMoveAngX));
						GGQuaternionRotationAxis(&QuatAroundY, &GGVECTOR3(0, 1, 0), GGToRadian(fMoveAngY));
						GGQuaternionRotationAxis(&QuatAroundZ, &GGVECTOR3(0, 0, 1), GGToRadian(fMoveAngZ));
						quatRotationEvent = QuatAroundX * QuatAroundY * QuatAroundZ;
						int iObj = t.entityelement[t.widget.pickedEntityIndex].obj;
						GGQuaternionRotationAxis(&QuatAroundX, &GGVECTOR3(1, 0, 0), GGToRadian(ObjectAngleX(iObj)));
						GGQuaternionRotationAxis(&QuatAroundY, &GGVECTOR3(0, 1, 0), GGToRadian(ObjectAngleY(iObj)));
						GGQuaternionRotationAxis(&QuatAroundZ, &GGVECTOR3(0, 0, 1), GGToRadian(ObjectAngleZ(iObj)));
						GGQUATERNION quatCurrentOrientation = QuatAroundX * QuatAroundY * QuatAroundZ;
						GGQUATERNION quatNewOrientation;
						GGQuaternionMultiply(&quatNewOrientation, &quatCurrentOrientation, &quatRotationEvent);
						RotateObjectQuat(iObj, quatNewOrientation.x, quatNewOrientation.y, quatNewOrientation.z, quatNewOrientation.w);
						t.entityelement[t.widget.pickedEntityIndex].rx = ObjectAngleX(iObj);
						t.entityelement[t.widget.pickedEntityIndex].ry = ObjectAngleY(iObj);
						t.entityelement[t.widget.pickedEntityIndex].rz = ObjectAngleZ(iObj);
					}

					//PE: Update light data for spot.
					if(t.entityelement[t.widget.pickedEntityIndex].eleprof.usespotlighting)
						lighting_refresh();

					// also update particle emitter

					if ( t.entityelement[t.widget.pickedEntityIndex].obj>0 ) 
					{
						int iTargetCenterObject = t.entityelement[t.widget.pickedEntityIndex].obj;
						if ( ObjectExist ( iTargetCenterObject ) == 1 ) 
						{
							if ( g.entityrubberbandlist.size() > 0 )
							{
								// rotate all the grouped entities and move around Y axis of widget as pivot
								if (bRotateObjectFromKeyPress == true)
								{
									SetStartPositionsForRubberBand(iTargetCenterObject);
									RotateAndMoveRubberBand(iTargetCenterObject, 0, 0, 0, quatRotationEvent);
								}
							}
						}
					}
				}
				//  Find Floor (  control of widget controlled entity or Raise/lower with PGUP and PGDN )
				if (t.inputsys.keyreturn == 1 || t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209)
				{
					t.tforceentityfindfloor = t.widget.pickedEntityIndex;
					t.tforcepguppgdnkeys = 1;
					editor_forceentityfindfloor (false);
				}
			}
			else
			{
				if (  t.inputsys.domodeterrain == 0 && t.inputsys.domodeentity == 0 ) 
				{
					//  avoid interference from terrain/entity mode change
					if (  t.inputsys.dorotation == 1 ) {
						t.gridentityrotatey_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1;
					}
					if (  t.inputsys.doentityrotate == 1 ) { t.gridentityrotatex_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
					if (  t.inputsys.doentityrotate == 2 ) { t.gridentityrotatex_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
					if (  t.inputsys.doentityrotate == 3 ) { t.gridentityrotatey_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
					if (  t.inputsys.doentityrotate == 4 ) { t.gridentityrotatey_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
					if (  t.inputsys.doentityrotate == 5 ) { t.gridentityrotatez_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
					if (  t.inputsys.doentityrotate == 6 ) { t.gridentityrotatez_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
					if (  t.inputsys.doentityrotate >= 98 ) 
					{
						if (  t.inputsys.doentityrotate == 98 ) 
						{
							if (  t.gridentityrotateaxis == 0  )  t.gridentityrotatex_f = 0;
							if (  t.gridentityrotateaxis == 1  )  t.gridentityrotatey_f = 0;
							if (  t.gridentityrotateaxis == 2  )  t.gridentityrotatez_f = 0;
						}
						if (  t.inputsys.doentityrotate == 99 ) 
						{
							t.gridentityrotatex_f=0;
							t.gridentityrotatey_f=0;
							t.gridentityrotatez_f=0;
						}
					}
				}
				t.gridentityrotatex_f=WrapValue(t.gridentityrotatex_f);
				t.gridentityrotatey_f=WrapValue(t.gridentityrotatey_f);
				t.gridentityrotatez_f=WrapValue(t.gridentityrotatez_f);
			}
		}
	}

	//  Load and Save
	if ( t.inputsys.doload == 1 ) gridedit_load_map ( );
	if ( t.inputsys.dosave == 1 ) 
	{
		if (  g.galwaysconfirmsave == 1 ) 
		{
			gridedit_saveas_map ( );
		}
		else
		{
			gridedit_save_map_ask ( );
		}
	}
	if ( t.inputsys.doopen == 1 ) gridedit_open_map_ask ( );
	if ( t.inputsys.donew == 1 || t.inputsys.donewflat == 1 ) gridedit_new_map_ask ( );
	if ( t.inputsys.dosaveas == 1 ) gridedit_saveas_map ( );
	if ( t. inputsys.dosaveandrun==1 ) { t.inputsys.dosaveandrun = 0 ; editor_previewmap ( 0 ); }

	//  Undo \ Redo
	if (t.inputsys.doundo == 1)
	{
		editor_undo ();
	}
	if (t.inputsys.doredo == 1)
	{
		editor_redo ();
	}

	//  Paint Select or Art Mode
	if ( t.inputsys.domodeterrain == 1 || t.inputsys.domodemarker == 1 || t.inputsys.domodeentity == 1  || t.inputsys.domodewaypoint == 1 )
	{
		// select editing mode and refresh
		if ( t.inputsys.domodeterrain == 1 ) { t.inputsys.domodeterrain=0; t.gridentitymarkersmodeonly=0; t.grideditselect=0; }
		if ( t.inputsys.domodemarker == 1 ) { t.inputsys.domodemarker=0; t.gridentitymarkersmodeonly=1; t.grideditselect=5; }
		if ( t.inputsys.domodeentity == 1 ) { t.inputsys.domodeentity=0; t.gridentitymarkersmodeonly=0; t.grideditselect=5; }
		if ( t.inputsys.domodewaypoint == 1 ) { t.inputsys.domodewaypoint=0; t.gridentitymarkersmodeonly=0; t.grideditselect=6; }
		editor_refresheditmarkers ( );
		gridedit_updateprojectname();

		// also deactivate EBE if enter a regular editing mode
		ebe_hide();
	}

	//  Manage waypoints on map
	t.tokay=0;
	if (  t.grideditselect == 5 ) 
	{
		//  entity mode can manipulate waypoint zone style
		if (  t.widget.pickedObject == 0 && t.widget.pickedSection == 0 && t.gridentity == 0 ) 
		{
			//  ensure low interference if editing, etc
			t.tokay=1;
		}
	}
	if (  t.grideditselect == 6 ) 
	{
		//  waypoint mode has access to waypoint editing
		t.tokay=1;
	}
	if (  t.tokay == 1 ) 
	{
		t.mx_f=t.inputsys.localx_f ; t.mz_f=t.inputsys.localy_f ; t.mclick=t.inputsys.mclick;
		g.waypointeditheight_f=t.inputsys.localcurrentterrainheight_f;

		// only detect waypoints when NOT using rubber band
		if ( t.inputsys.rubberbandmode == 0 && t.ebe.on == 0 && t.showeditorelements)
			waypoint_mousemanage ( );
	}

	//  New clip height control
	if (  t.inputsys.keycontrol == 1 ) 
	{
		if (  t.inputsys.wheelmousemove>0 ) 
		{
			t.clipheight_f -= 2.0f ; if (  t.clipheight_f<0.0f  )  t.clipheight_f = 0.0f;
			t.updatezoom=1;
		}
		if (  t.inputsys.wheelmousemove<0 ) 
		{
			t.clipheight_f += 2.0f ; if (  t.clipheight_f>50000.0f )  t.clipheight_f = 50000.0f;
			t.updatezoom=1;
		}
	}

	//  Zoom factor (for top down or freeflight+ControlKey ( ) )
	t.tspecialgridzoomadjustment=0;
	if (  t.editorfreeflight.mode == 0 || t.tspecialgridzoomadjustment != 0 ) 
	{
		if (  ((t.inputsys.dozoomin == 1 && t.inputsys.keypress == 0) || t.tspecialgridzoomadjustment == 1) && t.gridzoom_f>0.3 ) 
		{
			t.updatezoom=1;
			if (  t.inputsys.keyshift == 1 ) 
			{
				t.gridzoom_f -= 0.6f*fMouseWheelZoomFactor;
			}
			else
			{
				t.gridzoom_f -= 0.1f*fMouseWheelZoomFactor;
			}
		}
		if (  ((t.inputsys.dozoomout == 1 && t.inputsys.keypress == 0) || t.tspecialgridzoomadjustment == 2) && t.gridzoom_f<40.0 ) 
		{
			t.updatezoom=1;
			if (  t.inputsys.keyshift == 1 ) 
			{
				t.gridzoom_f += 0.6f*fMouseWheelZoomFactor;
			}
			else
			{
				t.gridzoom_f += 0.1f*fMouseWheelZoomFactor;
			}
		}
	}

	//  Scroll Map
	t.borderx_f=1024.0*50.0;
	t.bordery_f=1024.0*50.0;
	if (  t.inputsys.doscrollleft != 0 ) 
	{
		t.cx_f -= t.gridzoom_f*3*t.inputsys.doscrollleft;
		t.updatezoom=1;
	}
	if (  t.inputsys.doscrollright != 0 ) 
	{
		t.cx_f += t.gridzoom_f*3*t.inputsys.doscrollright;
		t.updatezoom=1;
	}
	if (  t.inputsys.doscrollup != 0 ) 
	{
		t.cy_f += t.gridzoom_f*3*t.inputsys.doscrollup;
		t.updatezoom=1;
	}
	if (  t.inputsys.doscrolldown != 0 ) 
	{
		t.cy_f -= t.gridzoom_f*3*t.inputsys.doscrolldown;
		t.updatezoom=1;
	}

	//  Scroll boundaries
	if (  t.cx_f<0  )  t.cx_f = 0;
	if (  t.cy_f<0  )  t.cy_f = 0;
	if (  t.cx_f>t.borderx_f  )  t.cx_f = t.borderx_f;
	if (  t.cy_f>t.bordery_f  )  t.cy_f = t.bordery_f;

}

float editor_forceentityfindfloor (bool bPredictMode)
{
	// receives; tforceentityfindfloor
	// bPredictMode = set to true when we want to work out where the object will go when forced to floor
	// but without affecting any globals or states
	float fPredictedYPosition = 0.0f;
	int ssgridentityinzoomview, ssgridentitydroptoground;
	float ssgridentityposx_f, ssgridentityposy_f, ssgridentityposz_f;
	int ssgridentityobj, ssthardauto, ssgridentityposoffground, ssgridentityusingsoftauto, ssgridentitysurfacesnap;
	if (bPredictMode == true)
	{
		ssgridentityinzoomview = t.gridentityinzoomview;
		ssgridentitydroptoground = t.gridentitydroptoground;
		ssgridentityposx_f = t.gridentityposx_f;
		ssgridentityposy_f = t.gridentityposy_f;
		ssgridentityposz_f = t.gridentityposz_f;
		ssgridentityobj = t.gridentityobj;
		ssthardauto = t.thardauto;
		ssgridentityposoffground = t.gridentityposoffground;
		ssgridentityusingsoftauto = t.gridentityusingsoftauto;
		ssgridentitysurfacesnap = t.gridentitysurfacesnap;
	}
	t.storegridentityinzoomview = t.gridentityinzoomview;
	t.gridentityinzoomview = t.tforceentityfindfloor;
	t.storegridentityposy_f = t.gridentityposy_f;
	t.gridentityposy_f = t.entityelement[t.gridentityinzoomview].y;
	int iEntPassMax = 1;
	if (g.entityrubberbandlist.size() > 0) iEntPassMax = g.entityrubberbandlist.size();
	for (int iEntPass = 0; iEntPass < iEntPassMax; iEntPass++)
	{
		// which entity are we dealing with
		int e = t.gridentityinzoomview;
		if ((e > 0 && t.entityelement[e].editorlock == 0) || bPredictMode == true)
		{
			// if RETURN key pressed
			if (bPredictMode == true)
				t.gridentityposy_f = ObjectPositionY(t.gridentityobj);
			else
				t.gridentityposy_f = t.entityelement[e].y;
			if (t.inputsys.keyreturn == 1 || bPredictMode == true)
			{
				// store globs in store
				t.storegridentitydroptoground = t.gridentitydroptoground;
				t.storegridentityposx_f = t.gridentityposx_f;
				t.storegridentityposz_f = t.gridentityposz_f;
				t.storegridentityobj = t.gridentityobj;
				t.storegridentityposoffground = t.gridentityposoffground;
				t.gridentitydroptoground = 1;
				if (bPredictMode == true)
				{
					t.gridentityposx_f = ObjectPositionX(t.gridentityobj);
					t.gridentityposz_f = ObjectPositionZ(t.gridentityobj);
				}
				else
				{
					t.gridentityposx_f = t.entityelement[e].x;
					t.gridentityposz_f = t.entityelement[e].z;
					t.gridentityobj = t.entityelement[e].obj;
				}
				t.thardauto = 1; editor_findentityground();
				if (t.gridentityposoffground == 0)
				{
					if (t.terrain.TerrainID > 0)
					{
						t.gridentityposy_f = BT_GetGroundHeight(t.terrain.TerrainID, t.gridentityposx_f, t.gridentityposz_f);
					}
					else
					{
						t.gridentityposy_f = g.gdefaultterrainheight;
					}
					if (t.entityprofile[t.gridentity].ismarker != 0)  t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].offy;
					if (t.entityprofile[t.gridentity].defaultheight != 0)  t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].defaultheight;
				}
				if (bPredictMode == false)
				{
					t.entityelement[e].x = t.gridentityposx_f;
					t.entityelement[e].z = t.gridentityposz_f;
				}

				// restore globs from store
				t.gridentitydroptoground = t.storegridentitydroptoground;
				t.gridentityposx_f = t.storegridentityposx_f;
				t.gridentityposz_f = t.storegridentityposz_f;
				t.gridentityobj = t.storegridentityobj;
				t.gridentityposoffground = t.storegridentityposoffground;
				t.gridentityusingsoftauto = 1;
				t.gridentitysurfacesnap = 0;
				}
			if (bPredictMode == false)
			{
				if (t.tforcepguppgdnkeys == 1)
				{
					editor_handlepguppgdn();
				}
				if (t.entityelement[e].y != t.gridentityposy_f)
				{
					t.entityelement[e].beenmoved = 1;
				}
				t.entityelement[e].y = t.gridentityposy_f;
				if (t.entityelement[e].obj > 0)
				{
					if (ObjectExist(t.entityelement[e].obj) == 1)
					{
						PositionObject(t.entityelement[e].obj, t.entityelement[e].x, t.entityelement[e].y, t.entityelement[e].z);
					}
				}
			}
			else
			{
				// have the prediction for where the object would be placed
				fPredictedYPosition = t.gridentityposy_f;
			}
			}
		}
	t.gridentityposy_f = t.storegridentityposy_f;
	t.gridentityinzoomview = t.storegridentityinzoomview;
	if (bPredictMode == true)
	{
		// restore any changes and return prediction
		t.gridentityinzoomview = ssgridentityinzoomview;
		t.gridentitydroptoground = ssgridentitydroptoground;
		t.gridentityposx_f = ssgridentityposx_f;
		t.gridentityposy_f = ssgridentityposy_f;
		t.gridentityposz_f = ssgridentityposz_f;
		t.gridentityobj = ssgridentityobj;
		t.thardauto = ssthardauto;
		t.gridentityposoffground = ssgridentityposoffground;
		t.gridentityusingsoftauto = ssgridentityusingsoftauto;
		t.gridentitysurfacesnap = ssgridentitysurfacesnap;
		return fPredictedYPosition;
	}
	else
	{
		// regular usage
		return 0.0f;
	}
}

void editor_viewfunctionality ( void )
{
	// map view controls
	if ( t.grideditselect == 3 ) 
	{
		if ( t.inputsys.mclick == 1 ) 
		{
			t.stcx_f=t.inputsys.mmx*100.0;
			t.stcy_f=t.inputsys.mmy*100.0;
			t.cx_f=t.stcx_f ; t.cy_f=t.stcy_f ; t.gridzoom_f=t.stgridzoom_f;
			t.grideditselect=t.stgrideditselect ; editor_refresheditmarkers ( );
			while ( t.inputsys.mclick==1 ) { input_getcontrols() ; Sync() ; }
			t.cameraviewmode=0;
			t.updatezoom=1;
		}
	}

	// zoom view controls
	if ( t.grideditselect == 4 ) 
	{
		// can repos and rotate non-editor-entities
		if ( t.entityelement[t.gridentityinzoomview].editorfixed == 0 ) 
		{
			// position adjustment
			t.tposadjspeed_f=1.0;
			if ( t.inputsys.keycontrol == 1  )  t.tposadjspeed_f = 0.05f;
			if ( t.inputsys.dozoomviewmovex == 1  )  t.zoomviewtargetx_f -= t.tposadjspeed_f;
			if ( t.inputsys.dozoomviewmovex == 2  )  t.zoomviewtargetx_f += t.tposadjspeed_f;
			if ( t.inputsys.dozoomviewmovey == 1  )  t.zoomviewtargety_f -= t.tposadjspeed_f;
			if ( t.inputsys.dozoomviewmovey == 2  )  t.zoomviewtargety_f += t.tposadjspeed_f;
			if ( t.inputsys.dozoomviewmovez == 1  )  t.zoomviewtargetz_f -= t.tposadjspeed_f;
			if ( t.inputsys.dozoomviewmovez == 2  )  t.zoomviewtargetz_f += t.tposadjspeed_f;

			// rotation adjustment
			//PE: rotate lights rem: t.entityprofile[t.gridentity].ismarker != 2 &&
			if ( t.entityprofile[t.gridentity].ismarker != 3 ) 
			{
				if ( t.inputsys.keyshift == 1 ) 
				{
					t.tspeedofrot_f=10.0 ; t.inputsys.keypress=0;
				}
				else
				{
					if ( t.inputsys.keycontrol == 1 ) 
					{
						t.tspeedofrot_f=1.0;
					}
					else
					{
						t.tspeedofrot_f=45.0;
					}
				}
				if ( t.inputsys.dorotation == 1 ) {  t.zoomviewtargetry_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if ( t.inputsys.dozoomviewrotatex == 1 ) { t.zoomviewtargetrx_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
				if ( t.inputsys.dozoomviewrotatex == 2 ) { t.zoomviewtargetrx_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
				if ( t.inputsys.dozoomviewrotatey == 1 ) { t.zoomviewtargetry_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if ( t.inputsys.dozoomviewrotatey == 2 ) { t.zoomviewtargetry_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if ( t.inputsys.dozoomviewrotatez == 1 ) { t.zoomviewtargetrz_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
				if ( t.inputsys.dozoomviewrotatez == 2 ) { t.zoomviewtargetrz_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
				if ( t.inputsys.dozoomviewrotatex >= 98 ) 
				{
					if ( t.inputsys.dozoomviewrotatex == 98 ) 
					{
						if ( t.gridentityrotateaxis == 0  )  t.zoomviewtargetrx_f = 0;
						if ( t.gridentityrotateaxis == 1  )  t.zoomviewtargetry_f = 0;
						if ( t.gridentityrotateaxis == 2  )  t.zoomviewtargetrz_f = 0;
					}
					if ( t.inputsys.dozoomviewrotatex == 99 ) 
					{
						t.zoomviewtargetrx_f=0;
						t.zoomviewtargetry_f=0;
						t.zoomviewtargetrz_f=0;
					}
				}
			}

			// update gridentity vars for visual
			t.gridentityposx_f=t.zoomviewtargetx_f;
			t.gridentityposy_f=t.zoomviewtargety_f;
			t.gridentityposz_f=t.zoomviewtargetz_f;
			t.gridentityrotatex_f=t.zoomviewtargetrx_f;
			t.gridentityrotatey_f=t.zoomviewtargetry_f;
			t.gridentityrotatez_f=t.zoomviewtargetrz_f;


			//PE: We are in properties and have snap mode, the original object position is then moved.
			//PE: We dont want this snap 5x5 grid in properties.
			// aply grid if 5x5
			#ifdef THISHASBEENREMOVED
			if ( t.gridentitygridlock == 1 ) 
			{
				t.gridentityposx_f=(int(t.gridentityposx_f/5)*5);
				t.gridentityposz_f=(int(t.gridentityposz_f/5)*5);
			}
			#endif
		}

		// mouselook mode on/off RMB
		OpenFileMap ( 1, "FPSEXCHANGE" );
		if ( t.inputsys.mclick == 2 ) 
		{
			// center mouse
			#if !defined(ENABLEIMGUI) || defined(USEOLDIDE)
			SetFileMapDWORD (  1, 48, 1 );
			#else
			if (g.mouseishidden == 0) 
			{
				g.mouseishidden = 1;
				t.tgamemousex_f = t.inputsys.xmouse; //MouseX();
				t.tgamemousey_f = t.inputsys.ymouse; //MouseY();
				HideMouse();
				#ifdef USERENDERTARGET

				POINT tmp;
				GetCursorPos(&tmp);
				t.editorfreeflight.storemousex = tmp.x;
				t.editorfreeflight.storemousey = tmp.y;

				ImVec2 setPos = { (OldrenderTargetSize.x*0.5f) + OldrenderTargetPos.x , (OldrenderTargetSize.y*0.5f) + OldrenderTargetPos.y };
				setPos.x = (int)setPos.x;
				setPos.y = (int)setPos.y;
				SetCursorPos(setPos.x, setPos.y);

				float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
				float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
				xmouseold = (setPos.x - renderTargetAreaPos.x) * RatioX;
				ymouseold = (setPos.y - renderTargetAreaPos.y) * RatioY;
				t.inputsys.xmousemove = 0;
				t.inputsys.ymousemove = 0;
				#else
				RECT rect;
				GetWindowRect(g_pGlob->hWnd, &rect);
				SetCursorPos(rect.left + (GetChildWindowWidth() / 2), rect.top + (GetChildWindowHeight() / 2));
				xmouseold = rect.left + (GetChildWindowWidth() / 2); //t.inputsys.xmouse;
				ymouseold = rect.top + (GetChildWindowHeight() / 2); //t.inputsys.xmouse;
				#endif
			}
			else 
			{
				//Center mouse here.
				//PE: imgui this need to be center on imgui window.
				#ifdef USERENDERTARGET
				extern ImVec2 OldrenderTargetSize;
				extern ImVec2 OldrenderTargetPos;
				ImVec2 setPos = { (OldrenderTargetSize.x*0.5f) + OldrenderTargetPos.x , (OldrenderTargetSize.y*0.5f) + OldrenderTargetPos.y };
				SetCursorPos(setPos.x, setPos.y);

				float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
				float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
				xmouseold = (setPos.x - renderTargetAreaPos.x) * RatioX;
				ymouseold = (setPos.y - renderTargetAreaPos.y) * RatioY;

				#else
				RECT rect;
				GetWindowRect(g_pGlob->hWnd, &rect);
				SetCursorPos(rect.left + (GetChildWindowWidth() / 2), rect.top + (GetChildWindowHeight() / 2));
				#endif
			}
			#endif			

			// camera position
			t.zoomviewcameraangle_f += (float) t.inputsys.xmousemove/2.0f;
			t.zoomviewcameraheight_f -= (float) t.inputsys.ymousemove/1.5f;
		}
		else
		{
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
			if (g.mouseishidden == 1) 
			{
				g.mouseishidden = 0;
				ShowMouse();
			}
			#else
			SetFileMapDWORD (  1, 48, 0 );
			#endif
		}
		SetEventAndWait (  1 );

		//  exit zoom view
		#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
		if (!bImGuiGotFocus && bImGuiRenderTargetFocus && t.inputsys.mclick == 1)  t.tpressedtoleavezoommode = 1;

		if (!bImGuiGotFocus && bImGuiRenderTargetFocus && t.inputsys.mclick == 0 && t.tpressedtoleavezoommode == 1)  t.tpressedtoleavezoommode = 2;

		//When properties window open , they should click "apply","cancel".
		if(bProperties_Window_Block_Mouse)
			t.tpressedtoleavezoommode = 0;

		if (bProperties_Window_Block_Mouse) 
		{
			//Must have a release before block is released.
			if (t.inputsys.mclick == 0) 
			{
				bProperties_Window_Block_Mouse = false;
				t.tpressedtoleavezoommode = 0;
			}
		}
		#else
		if ( t.inputsys.mclick == 1  )  t.tpressedtoleavezoommode = 1;
		if ( t.inputsys.mclick == 0 && t.tpressedtoleavezoommode == 1 )  t.tpressedtoleavezoommode = 2;
		#endif

		if ( (t.tpressedtoleavezoommode == 2 || t.inputsys.kscancode == 211) || t.editorinterfaceleave == 1 ) 
		{
			// leave zoomview
			t.inputsys.doautozoomview=1;

			// reset mouse click (must release LMB before zoom mode ends)
			t.tpressedtoleavezoommode=0;

			// close any property window
			interface_closepropertywindow ( );
			t.editorinterfaceleave=0;

			// 310315 - Ensure clipping is restored when return
			t.updatezoom=1;
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
			//Use backup settings before cancel.
			//t.grideleprof = backup_grideleprof;
			#endif

			// place entity on the map
			if ( t.gridentityinzoomview>0 ) 
			{
				// DELETE key deletes entity no matter what (for fixed entities too)
				if ( t.gridentity != 0 && t.inputsys.kscancode == 211 ) 
				{
					// Delete any associated waypoint/trigger zone
					t.waypointindex=t.grideleprof.trigger.waypointzoneindex;
					if (  t.waypointindex>0 ) 
					{
						t.w=t.waypoint[t.waypointindex].start;
						waypoint_delete ( );
					}
					t.grideleprof.trigger.waypointzoneindex=0;

					// And now delete entity from cursor
					if (  t.gridentityobj == 0 ) 
					{
						DeleteObject (  t.gridentityobj );
						t.gridentityobj=0;
					}
					t.gridentityinzoomview = 0;
				}
				else
				{
					// Add entity back into map
					#ifdef VRTECH
					if (iOldgridentity == t.gridentity) 
					{
						gridedit_addentitytomap();
						t.gridentityinzoomview = 0;
					}
					else 
					{
						timestampactivity(0, "t.gridentity!=lastpropertiesid ?");
					}
					#else
					gridedit_addentitytomap();
					t.gridentityinzoomview = 0;
					#endif
					//  Hide widget to make clean return to editor
					t.widget.pickedObject=0  ; widget_updatewidgetobject ( );
				}

				// Reset cursor object settings
				t.refreshgrideditcursor=1;
				t.gridentity=0;
				t.gridedit.autoflatten=0;
				t.gridedit.entityspraymode=0;
				t.gridentityposoffground=0;
				t.gridentityusingsoftauto=1;
				t.gridentitysurfacesnap=1-g.gdisablesurfacesnap;
				t.gridentityautofind=1;
				t.inputsys.dragoffsetx_f=0;
				t.inputsys.dragoffsety_f=0;
				editor_refreshentitycursor ( );

				#ifdef ENABLEIMGUI
				if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
				if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
				bEntity_Properties_Window = false; //Close Properties window.
				#endif
			}
		}
	}
}

void editor_findentityground ( void )
{
	//  for entities that can be moved
	if (  t.entityelement[t.gridentityinzoomview].editorfixed == 0 ) 
	{
		//  finds ground
		if ( t.gridentitydroptoground == 1 || (t.thardauto == 0 && t.gridentityusingsoftauto == 1) ) 
		{

			//PE: MUST disable collision on ALL rubberband objects.
			std::vector<sRubberBandType> entityvisible = g.entityrubberbandlist;
			if (g.entityrubberbandlist.size() > 0)
			{
				for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
				{
					int e = g.entityrubberbandlist[i].e;
					int obj = t.entityelement[e].obj;
					if (obj > 0 && GetVisible(obj))
					{
						entityvisible[i].x = 1;
						HideObject(obj);
					}
					else
					{
						entityvisible[i].x = 0;
					}
				}
			}


			t.tbestdist_f=99999 ; t.tbesty_f=0;
			t.tto_f=t.gridentityposy_f-200.0;
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				if (  t.thardauto == 1 ) 
				{
					//  regular
					if (  t.entityelement[t.e].editorlock == 0 ) 
					{
						//PE: always use camtY as the max when placing objects.
						if (  t.inputsys.keyshift == 0 ) 
						{
							//  if close to building (clipping top of it), do not exceed clip theshold
							t.tfrom_f=t.gridentityposy_f+50000.0;
							if (  t.tfrom_f>t.gridtrueslicey_f  )
								t.tfrom_f = t.gridtrueslicey_f;
						}
						else
						{
							t.tfrom_f=t.gridentityposy_f+75.0;
						}
					}
					else
					{
						t.tfrom_f=t.gridentityposy_f+75.0;
					}
				}
				else
				{
					//  very subtle surface scan (to defeat small floors)
					t.tfrom_f=t.gridentityposy_f+11.0;
				}
				t.tokay=1;
				if (  t.entityprofile[t.entid].addhandlelimb>0  )  t.tokay = 0;
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					//  if third person char, ignore when finding surface
					if (  t.e == t.playercontrol.thirdperson.charactere  )  t.tokay = 0;
					if (  t.e == t.playercontrol.thirdperson.startmarkere  )  t.tokay = 0;
				}
				if (  t.tokay == 1 ) 
				{
					t.obj=t.entityelement[t.e].obj;
					if (  t.obj>0 && t.obj != t.gridentityobj ) 
					{
						if (  ObjectExist(t.obj) == 1 ) 
						{
							if (  GetVisible(t.obj) == 1 ) 
							{
								//  210415 - added distance check to speed up ground scan
								t.tdiffx_f=ObjectPositionX(t.obj)-t.gridentityposx_f;
								t.tdiffz_f=ObjectPositionZ(t.obj)-t.gridentityposz_f;
								t.tdiff_f=Sqrt(abs(t.tdiffx_f*t.tdiffx_f)+abs(t.tdiffz_f*t.tdiffz_f));
								if (  t.tdiff_f<ObjectSize(t.obj)*2 ) 
								{
									if (  IntersectObject(t.obj,t.gridentityposx_f,t.tfrom_f,t.gridentityposz_f,t.gridentityposx_f,t.tto_f,t.gridentityposz_f) != 0 ) 
									{
										t.tdist_f=abs(ChecklistFValueB(6)-t.tfrom_f);
										if (  t.tdist_f<t.tbestdist_f ) 
										{
											t.tbesty_f=ChecklistFValueB(6);
											t.tbestdist_f=t.tdist_f;
										}
									}
								}
							}
						}
					}
				}
			}
			if (  t.tbestdist_f<99999 ) 
			{
				//  found GetPoint (  where our entity will rest vertically )
				t.gridentityposy_f=t.tbesty_f ; t.zoomviewtargety_f=t.tbesty_f;
				//  now need entities own thickness from object 0,0,0 to base
				//  grid of ray casts for good base detect resolution
				if (  t.gridentityobj>0 && t.thardauto == 1 ) 
				{
					if (  ObjectExist(t.gridentityobj) == 1 ) 
					{
						t.ttentsizex_f=ObjectSizeX(t.gridentityobj)/2.0;
						t.ttentsizez_f=ObjectSizeZ(t.gridentityobj)/2.0;
						if (  t.ttentsizex_f<1.0 && t.ttentsizex_f<t.ttentsizez_f  )  t.ttentsizex_f = t.ttentsizez_f;
						if (  t.ttentsizez_f<1.0 && t.ttentsizez_f<t.ttentsizex_f  )  t.ttentsizez_f = t.ttentsizex_f;
						t.stepvaluex_f=ObjectSizeX(t.gridentityobj)/10.0;
						t.stepvaluez_f=ObjectSizeZ(t.gridentityobj)/10.0;
						if (  t.stepvaluex_f<1  )  t.stepvaluex_f = 1.0;
						if (  t.stepvaluez_f<1  )  t.stepvaluez_f = 1.0;
						if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
						MakeObjectBox (  g.entityworkobjectoffset,ObjectSizeX(t.gridentityobj),ObjectSizeY(t.gridentityobj),ObjectSizeZ(t.gridentityobj) );
						PositionObject (  g.entityworkobjectoffset,ObjectPositionX(t.gridentityobj)+GetObjectCollisionCenterZ(t.gridentityobj),ObjectPositionY(t.gridentityobj)+GetObjectCollisionCenterY(t.gridentityobj),ObjectPositionZ(t.gridentityobj)+GetObjectCollisionCenterZ(t.gridentityobj) );
						RotateObject (  g.entityworkobjectoffset,ObjectAngleX(t.gridentityobj),ObjectAngleY(t.gridentityobj),ObjectAngleZ(t.gridentityobj) );
						HideObject (  g.entityworkobjectoffset );
						t.tsmallest_f=99999;
						t.tscbase_f=ObjectPositionY(g.entityworkobjectoffset)-(ObjectSizeY(g.entityworkobjectoffset)*2);
						if (  t.tsmallest_f<99999 ) 
						{
							t.tthickness_f=ObjectPositionY(t.gridentityobj)-(t.tscbase_f+t.tsmallest_f);
						}
						else
						{
							t.tthickness_f=0;
						}
						if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
						t.gridentityposy_f=t.tbesty_f+t.tthickness_f ; t.zoomviewtargety_f=t.tbesty_f+t.tthickness_f;
					}
				}
				//  ensure a 'autofoundYpos' never drops BELOW Floor (  (  (i.e. skull under Floor) ) )

				if (  t.terrain.TerrainID>0 ) 
				{
					t.trygridentityposy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.gridentityposx_f,t.gridentityposz_f);
				}
				else
				{
					t.trygridentityposy_f=g.gdefaultterrainheight;
				}
				if (  t.gridentityposy_f<t.trygridentityposy_f ) 
				{
					t.gridentityposy_f = t.trygridentityposy_f;
					if (  t.entityprofile[t.gridentity].ismarker != 0  )  t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].offy;
					if (  t.entityprofile[t.gridentity].defaultheight != 0  )  t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].defaultheight;
				}
				//  we are sitting on an entity, no need for ground terrain resting
				t.gridentityposoffground=1;
			}
			else
			{
				//  if not find any entities, use terrain ground base
				t.gridentityposoffground=0;
			}

			//PE: Reenable rubberband collision.
			if (entityvisible.size() > 0)
			{
				for (int i = 0; i < (int)entityvisible.size(); i++)
				{
					int e = entityvisible[i].e;
					int obj = t.entityelement[e].obj;
					if (entityvisible[i].x == 1)
					{
						ShowObject(obj);
					}
				}
			}

		}
		else
		{

		}

		//  finds wall
		if (  t.gridentitydroptoground == 2 && t.thardauto == 1 ) 
		{

			t.tbestdist_f=99999 ; t.tbestx_f=0 ; t.tbestz_f=0;
			t.tbesty_f=t.gridentityposy_f+GetObjectCollisionCenterY(t.gridentityobj);
			t.a=t.gridentityrotatey_f;
			t.tfromx=NewXValue(t.gridentityposx_f,t.a,-5.0) ; t.ttox=NewXValue(t.gridentityposx_f,t.a,75.0);
			t.tfromz=NewZValue(t.gridentityposz_f,t.a,-5.0) ; t.ttoz=NewZValue(t.gridentityposz_f,t.a,75.0);
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				t.obj=t.entityelement[t.e].obj;
				if (  t.obj>0 && t.obj != t.gridentityobj ) 
				{
					if (  ObjectExist(t.obj) == 1 ) 
					{
						if (  GetVisible(t.obj) == 1 ) 
						{
							t.tdist_f=IntersectObject(t.obj,t.tfromx,t.tbesty_f,t.tfromz,t.ttox,t.tbesty_f,t.ttoz);
							if (  t.tdist_f != 0 ) 
							{
								if (  t.tdist_f<t.tbestdist_f ) 
								{
									t.tbestx_f=ChecklistFValueA(6);
									t.tbestz_f=ChecklistFValueC(6);
									t.tbestdist_f=t.tdist_f;
								}
							}
						}
					}
				}
			}
			if (  t.tbestdist_f<99999 ) 
			{
				//  found GetPoint (  where our entity will rest on wall )
				//  now need entities own thickness from object 0,0,0 to wall-contact
				t.tbestx_f=NewXValue(t.tbestx_f,t.a+180,-5.0);
				t.tbestz_f=NewZValue(t.tbestz_f,t.a+180,-5.0);
				t.ttox=NewXValue(t.tbestx_f,t.a+180,100.0);
				t.ttoz=NewZValue(t.tbestz_f,t.a+180,100.0);
				if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
				MakeObjectBox (  g.entityworkobjectoffset,ObjectSizeX(t.gridentityobj),ObjectSizeY(t.gridentityobj),ObjectSizeZ(t.gridentityobj) );
				PositionObject (  g.entityworkobjectoffset,ObjectPositionX(t.gridentityobj)+GetObjectCollisionCenterZ(t.gridentityobj),ObjectPositionY(t.gridentityobj)+GetObjectCollisionCenterY(t.gridentityobj),ObjectPositionZ(t.gridentityobj)+GetObjectCollisionCenterZ(t.gridentityobj) );
				RotateObject (  g.entityworkobjectoffset,ObjectAngleX(t.gridentityobj),ObjectAngleY(t.gridentityobj),ObjectAngleZ(t.gridentityobj) );
				HideObject (  g.entityworkobjectoffset );
				t.tgap_f=IntersectObject(g.entityworkobjectoffset,t.tbestx_f,t.tbesty_f,t.tbestz_f,t.ttox,t.tbesty_f,t.ttoz);
				if (  t.tgap_f >= 4.9 ) 
				{
					t.tgapx_f=ChecklistFValueA(6);
					t.tgapz_f=ChecklistFValueC(6);
					t.ttddx_f=t.tgapx_f-ObjectPositionX(t.gridentityobj);
					t.ttddz_f=t.tgapz_f-ObjectPositionZ(t.gridentityobj);
					t.tthickness_f=5.0+Sqrt(abs(t.ttddx_f*t.ttddx_f)+abs(t.ttddz_f*t.ttddz_f));
				}
				else
				{
					t.tthickness_f=5.0;
				}
				t.tbestx_f=NewXValue(t.tbestx_f,t.a+180,t.tthickness_f+0.5);
				t.tbestz_f=NewZValue(t.tbestz_f,t.a+180,t.tthickness_f+0.5);
				t.gridentityposx_f=t.tbestx_f ; t.zoomviewtargetx_f=t.tbestx_f;
				t.gridentityposz_f=t.tbestz_f ; t.zoomviewtargetz_f=t.tbestz_f;
				if (  ObjectExist(g.entityworkobjectoffset) == 1  )  DeleteObject (  g.entityworkobjectoffset );
			}
		}

	}
}

void editor_refresheditmarkers ( void )
{
	//  Deactivate widget if still in effect
	widget_switchoff ( );

	//  Deactivate floating selection of entity
	if ( t.grideditselect != 5 && t.grideditselect != 4 ) 
	{
		if ( t.grideditselect != 5 ) HideObject ( t.editor.objectstartindex+5 );
		t.gridentity=0 ; t.gridentityposoffground=0;
		t.gridentityusingsoftauto=0;
		t.gridentitysurfacesnap=1-g.gdisablesurfacesnap;
		t.gridentityautofind=1;
		t.inputsys.dragoffsetx_f=0;
		t.inputsys.dragoffsety_f=0;
	}

	//  Update entity cursor? (delete many of these as it WAS old shroud updater!)
	t.refreshgrideditcursor=1;

	//  Update clipboard items based on mode
	editor_cutcopyclearstate ( );

	//  Waypoint visibility
	if (  t.grideditselect != t.lastgrideditselect ) 
	{
		t.lastgrideditselect=t.grideditselect;
		if (  t.grideditselect == 6 ) 
		{
			waypoint_showallpaths ( );
		}
		else
		{
			if (  t.inputsys.dowaypointview == 0 ) 
			{
				waypoint_showallpaths ( );
			}
			else
			{
				waypoint_hideallpaths ( );
			}
		}
	}

}

void editor_visuals ( void )
{
	//  Control entity selection and alpha of layers
	if (  t.refreshgrideditcursor == 1 ) 
	{
		gridedit_recreateentitycursor ( );
		t.refreshgrideditcursor=0;
	}
	gridedit_displayentitycursor ( );

	//  Update Camera
	editor_camera ( );
}

void editor_camera ( void )
{
	// Camera Mode
	switch ( t.cameraviewmode ) 
	{
		case 0:

			//  Control free flight camera viewing angle (mouselook)
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
			//PE: Delta already reset , so use t.inputsys.xmousemove,y
			if (g.gminvert == 1)  t.ttmousemovey = t.inputsys.xmousemove*-1; else t.ttmousemovey = t.inputsys.ymousemove;
			t.cammousemovex_f = t.inputsys.xmousemove;
			#else
			if (g.gminvert == 1)  t.ttmousemovey = MouseMoveY()*-1; else t.ttmousemovey = MouseMoveY();
			t.cammousemovex_f = MouseMoveX();
			#endif

			t.cammousemovey_f=t.ttmousemovey;
			if (  t.inputsys.mclick == 0  )  t.inputsys.mclickreleasestate = 0;
			t.trmb=0;
			if (t.inputsys.mclick == 2 && t.inputsys.mclickreleasestate == 0) 
			{
				#if defined(ENABLEIMGUI) && !defined(USEOLDGUI)
				#ifdef USERENDERTARGET
				if (g.mouseishidden == 1)
				{
					ImVec2 setPos = { (OldrenderTargetSize.x*0.5f) + OldrenderTargetPos.x , (OldrenderTargetSize.y*0.5f) + OldrenderTargetPos.y };
					setPos.x = (int)setPos.x;
					setPos.y = (int)setPos.y;
					SetCursorPos(setPos.x, setPos.y);

					float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
					float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
					xmouseold = (setPos.x - renderTargetAreaPos.x) * RatioX;
					ymouseold = (setPos.y - renderTargetAreaPos.y) * RatioY;
				}
				#else
				//PE: imgui this need to be center on imgui window.
				RECT rect;
				GetWindowRect(g_pGlob->hWnd, &rect);
				SetCursorPos(rect.left + (GetChildWindowWidth() / 2), rect.top + (GetChildWindowHeight() / 2));
				xmouseold = rect.left + (GetChildWindowWidth() / 2); //t.inputsys.xmouse;
				ymouseold = rect.top + (GetChildWindowHeight() / 2); //t.inputsys.xmouse;
				#endif
				#endif
				t.trmb = 1;
			}
			if ( t.inputsys.mclick == 4 && t.inputsys.mclickreleasestate == 0  )  t.trmb = 2;
			if ( t.trmblock == 0 ) 
			{
				if ( t.cammousemovex_f != 0 || t.cammousemovex_f != 0 || t.inputsys.kscancode != 0  )  t.trmblock = 1;
			}
			else
			{
				if (  t.inputsys.mclick == 0  )  t.trmblock = 0;
			}
			if (  t.trmblock == 0  )  t.trmb = 0;
			if (  g.globals.disablefreeflight == 1  )  t.trmb = 0;
			if (  t.trmb != 0 ) 
			{
				if (  g.mouseishidden == 0 )
				{
					game_hidemouse ( );
					#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
					POINT tmp;
					GetCursorPos(&tmp);
					t.editorfreeflight.storemousex = tmp.x;
					t.editorfreeflight.storemousey = tmp.y;
					#else
					t.editorfreeflight.storemousex=t.inputsys.xmouse;
					t.editorfreeflight.storemousey=t.inputsys.ymouse;
					#endif
				}
				if (  t.editorfreeflight.mode == 0 ) 
				{
					t.editorfreeflight.mode=1 ; t.updatezoom=1;
					t.editorfreeflight.c.x_f=t.cx_f;
					t.editorfreeflight.c.y_f=(600.0*t.gridzoom_f);
					t.editorfreeflight.c.z_f=t.cy_f;
					t.editorfreeflight.c.angx_f=CameraAngleX();
					t.editorfreeflight.c.angy_f=CameraAngleY();
				}
				else
				{
					if ( t.trmb == 1 ) 
					{
						// rotate with RMB
						#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
						//PE: a bit more smooth.
						t.tRotationDivider_f = 6.0;
						#else
						t.tRotationDivider_f = 5.0;
						#endif
						t.editorfreeflight.c.angx_f=CameraAngleX()+(t.cammousemovey_f/t.tRotationDivider_f);
						t.editorfreeflight.c.angy_f=CameraAngleY()+(t.cammousemovex_f/t.tRotationDivider_f);
						if (  t.editorfreeflight.c.angx_f>180.0f  )  t.editorfreeflight.c.angx_f = t.editorfreeflight.c.angx_f-360.0f;
						if (  t.editorfreeflight.c.angx_f<-89.999f  )  t.editorfreeflight.c.angx_f = -89.999f;
						if (  t.editorfreeflight.c.angx_f>89.999f  )  t.editorfreeflight.c.angx_f = 89.999f;
					}
				}

				#if defined(ENABLEIMGUI)
				//Always display skybox.
				sky_loop();
				#endif

			}
			else
			{
				if (g.mouseishidden == 1)
				{
					t.tideframestartx = 70; t.tideframestarty = 15;
					#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
					//PE: Restore mouse pos.
					SetCursorPos(t.editorfreeflight.storemousex, t.editorfreeflight.storemousey);
					float RatioX = ((float)GetDisplayWidth() / (float)renderTargetAreaSize.x) * ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
					float RatioY = ((float)GetDisplayHeight() / (float)renderTargetAreaSize.y) * ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
					xmouseold = (t.editorfreeflight.storemousex - renderTargetAreaPos.x) * RatioX;
					ymouseold = (t.editorfreeflight.storemousey - renderTargetAreaPos.y) * RatioY;
					t.inputsys.xmouse = xmouseold;
					t.inputsys.xmouse = ymouseold;
					ShowMouse();
					g.mouseishidden = 0;
					#else
					t.inputsys.xmouse = ((t.tideframestartx + t.editorfreeflight.storemousex + 0.0) / 800.0)*(GetDisplayWidth() + 0.0);
					t.inputsys.ymouse = ((t.tideframestarty + t.editorfreeflight.storemousey + 0.0) / 600.0)*(GetDisplayHeight() + 0.0);
					game_showmouse_restore_mouse(); //PE: Will use exact mouse positon stored by editor when hiding mouse. (if available, else t.tideframestartx...)
					#endif
					t.terrain.X_f=999999 ; t.terrain.Y_f=999999;
				}
			}

			// Handle free flight camea movement
			if (t.editorfreeflight.mode == 0)
			{
				t.editorfreeflight.c.x_f = t.cx_f;
				t.editorfreeflight.c.y_f = (600.0*t.gridzoom_f);
				t.editorfreeflight.c.z_f = t.cy_f;
				bool bSwitchToFFView = false;
				if (t.inputsys.k_s == "f" && g.globals.disablefreeflight == 0) bSwitchToFFView = true;
				if (bSwitchToFFView == true )
				{
					//  top down back to last free flight
					t.editorfreeflight.mode=3;
					if (  t.editorfreeflight.sused == 0 ) 
					{
						t.editorfreeflight.sused=1;
						t.editorfreeflight.s=t.editorfreeflight.c;
						t.tcurrenth_f=BT_GetGroundHeight(t.terrain.TerrainID,t.editorfreeflight.c.x_f,t.editorfreeflight.c.z_f);
						t.editorfreeflight.s.y_f = t.tcurrenth_f + 100.0;
						t.editorfreeflight.s.angy_f = .00f;
						t.editorfreeflight.s.angx_f = 20.0f;
					}
				}
			}
			if ( t.editorfreeflight.mode == 1 ) 
			{
				if ( t.inputsys.k_s == "g"  && t.inputsys.keycontrol == 0 )
				{
					// free flight to top down
					t.editorfreeflight.s=t.editorfreeflight.c;
					t.cx_f=t.editorfreeflight.c.x_f;
					t.cy_f=t.editorfreeflight.c.z_f;
					t.editorfreeflight.mode=2;
				}
				if (  t.inputsys.keyup == 1  )  t.plrkeyW = 1; else t.plrkeyW = 0;
				if (  t.inputsys.keyleft == 1  )  t.plrkeyA = 1; else t.plrkeyA = 0;
				if (  t.inputsys.keydown == 1  )  t.plrkeyS = 1; else t.plrkeyS = 0;
				if (  t.inputsys.keyright == 1  )  t.plrkeyD = 1; else t.plrkeyD = 0;

				//  mouse wheel mimmics W and S when no CONTROL key pressed (170616 - but not when in EBE mode as its used for grid layer control)
				int usingWheel = 0;
				if ( t.ebe.active == 0 )
				{
					if (  t.inputsys.keycontrol == 0 ) 
					{
						if (  t.inputsys.wheelmousemove<0 || t.inputsys.dozoomout == 1) { t.plrkeyS = 1; usingWheel = 1; }
						if (  t.inputsys.wheelmousemove>0 || t.inputsys.dozoomin == 1 ) { t.plrkeyW = 1; usingWheel = 1; }
					}
				}
				t.traise_f=0.0;
				if (  t.inputsys.keyshift == 1 ) 
				{
					t.tffcspeed_f=300.0*g.timeelapsed_f;
				}
				else
				{
					if (  t.inputsys.keycontrol == 1 ) 
					{
						t.tffcspeed_f=5.0*g.timeelapsed_f;
					}
					else
					{
						t.tffcspeed_f=35.0*g.timeelapsed_f;
					}
				}
				#if defined(ENABLEIMGUI2)//cyb
				if (g_bCharacterCreatorPlusActivated) {
					//Slow down movement when i CCP.
					t.tffcspeed_f *= 0.25;
				}
				#endif


				//PE: Classic way to fast.
				#ifndef PRODUCTCLASSIC
				// modify movement speed based on camera height
				float height = t.editorfreeflight.c.y_f + 500 - GGORIGIN_Y;
				if ( height < 0 ) height = 0;
				float modifier = height*height * 0.000005f;
				if ( modifier > 10 ) modifier = 10;
				if ( modifier < 1 ) modifier = 1;
				t.tffcspeed_f *= modifier;

				// speed up wheel movement
				if ( usingWheel )
					t.tffcspeed_f *= 4;
				#endif

				if (  t.inputsys.k_s == "]" || t.inputsys.k_s == "\\"  )  t.traise_f = -90;
				if (  t.inputsys.k_s == "["  )  t.traise_f = 90;
				PositionCamera (  t.editorfreeflight.c.x_f,t.editorfreeflight.c.y_f,t.editorfreeflight.c.z_f );
				
				if (  t.plrkeyW == 1  )
					MoveCamera (  t.tffcspeed_f );

				if (  t.plrkeyS == 1  )  MoveCamera (  t.tffcspeed_f*-1 );
				if (  t.plrkeyA == 1 ) { RotateCamera (  0,t.editorfreeflight.c.angy_f-90,0  ) ; MoveCamera (  t.tffcspeed_f ); }
				if (  t.plrkeyD == 1 ) { RotateCamera (  0,t.editorfreeflight.c.angy_f+90,0  ) ; MoveCamera (  t.tffcspeed_f ); }
				if (  t.traise_f != 0 ) { RotateCamera (  t.traise_f,0,0  ) ; MoveCamera (  t.tffcspeed_f ); }
				if (  t.inputsys.mclick == 4 ) 
				{
					//  new middle mouse panning
					RotateCamera (  0,t.editorfreeflight.c.angy_f,0 );
					MoveCamera (  t.cammousemovey_f*-2 );
					if (  t.cammousemovex_f<0 ) { RotateCamera (  0,t.editorfreeflight.c.angy_f-90,0  ) ; MoveCamera (  abs(t.cammousemovex_f*2) ); }
					if (  t.cammousemovex_f>0 ) { RotateCamera (  0,t.editorfreeflight.c.angy_f+90,0  ) ; MoveCamera (  t.cammousemovex_f*2 ); }
				}
				t.editorfreeflight.c.x_f=CameraPositionX();
				t.editorfreeflight.c.y_f=CameraPositionY();
				t.editorfreeflight.c.z_f=CameraPositionZ();

				#if defined(ENABLEIMGUI)
				//Always display skybox.
				sky_loop();
				#endif
			}

			//  view mode transitions
			if (  t.editorfreeflight.mode == 2 ) 
			{
				//  from free flight to top down
				t.tcamheight_f=(600.0*t.gridzoom_f);
				t.editorfreeflight.c.x_f=CurveValue(t.cx_f,CameraPositionX(),10.0);
				t.editorfreeflight.c.y_f=CurveValue(t.tcamheight_f,CameraPositionY(),10.0);
				t.editorfreeflight.c.z_f=CurveValue(t.cy_f,CameraPositionZ(),10.0);
				if (  abs(t.editorfreeflight.c.y_f-t.tcamheight_f)<20.0 ) 
				{
					t.editorfreeflight.mode=0 ; t.updatezoom=1;
				}
			}
			if (  t.editorfreeflight.mode == 3 ) 
			{
				//  from top down to free flight storage
				t.editorfreeflight.c.x_f=CurveValue(t.editorfreeflight.s.x_f,CameraPositionX(),10.0);
				t.editorfreeflight.c.y_f=CurveValue(t.editorfreeflight.s.y_f,CameraPositionY(),10.0);
				t.editorfreeflight.c.z_f=CurveValue(t.editorfreeflight.s.z_f,CameraPositionZ(),10.0);
				if (  abs(t.editorfreeflight.c.x_f-t.editorfreeflight.s.x_f)<20.0 && abs(t.editorfreeflight.c.y_f-t.editorfreeflight.s.y_f)<20.0 && abs(t.editorfreeflight.c.z_f-t.editorfreeflight.s.z_f)<20.0 ) 
				{
					t.editorfreeflight.c.x_f=t.editorfreeflight.s.x_f;
					t.editorfreeflight.c.y_f=t.editorfreeflight.s.y_f;
					t.editorfreeflight.c.z_f=t.editorfreeflight.s.z_f;
					t.editorfreeflight.mode=1 ; t.updatezoom=1;
				}
			}

			//  ensure camera NEVER goes into Floor (  )
			t.tcurrenth_f=BT_GetGroundHeight(t.terrain.TerrainID,t.editorfreeflight.c.x_f,t.editorfreeflight.c.z_f)+10.0;
			if (  t.editorfreeflight.c.y_f<t.tcurrenth_f ) 
			{
				t.editorfreeflight.c.y_f=t.tcurrenth_f;
			}

			//  update camera for free flight or top down modes
			PositionCamera (t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
			if (t.editorfreeflight.mode == 0)
			{
				PointCamera (t.cx_f, 0, t.cy_f);
			}
			if (t.editorfreeflight.mode == 1)
			{
				RotateCamera (t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
			}
			if (t.editorfreeflight.mode == 2)
			{
				t.editorfreeflight.c.angx_f = CurveAngle(90, CameraAngleX(), 10.0);
				t.editorfreeflight.c.angy_f = CurveAngle(0, CameraAngleY(), 10.0);
				RotateCamera (t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
			}
			if (t.editorfreeflight.mode == 3)
			{
				t.editorfreeflight.c.angx_f = CurveAngle(t.editorfreeflight.s.angx_f, CameraAngleX(), 10.0);
				t.editorfreeflight.c.angy_f = CurveAngle(t.editorfreeflight.s.angy_f, CameraAngleY(), 10.0);
				RotateCamera (t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
			}

			//  view mode prompt (top right status Text ( ) )
			if (  t.editorfreeflight.mode == 0 || t.editorfreeflight.mode == 2 ) 
			{
				t.t_s="TOP DOWN VIEW ('F' to toggle)";
			}
			else
			{
				t.t_s="FREE FLIGHT VIEW ('G' to toggle)";
			}
			#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
			//PE: TODO
			//Not really usefull in imgui , to small , we need another way to display this.
			//t.ttxtwid = getbitmapfontwidth(t.t_s.Get(), 3);
			//pastebitmapfont(t.t_s.Get(), GetChildWindowWidth(0) - 8 - t.ttxtwid, 4, 3, 228);
			#else
			t.ttxtwid=getbitmapfontwidth(t.t_s.Get(),2);
			pastebitmapfont(t.t_s.Get(),GetChildWindowWidth(0)-8-t.ttxtwid,4,2,228);
			#endif
		break;

		case 2:
			//  process live updates from
			interface_live_updates ( );

			//  update camera XZ with entity if editing position
			if (  t.gridentityinzoomview>0 ) 
			{
				t.cx_f=t.zoomviewtargetx_f ; t.cy_f=t.zoomviewtargetz_f;
			}

			//  if third person start marker mode, override range and angle
			t.tlayerheight_f=t.layerheight_f;
			if (  t.playercontrol.thirdperson.enabled == 1 ) 
			{
				t.zoomviewcamerarange_f=t.playercontrol.thirdperson.livecameradistance;
				t.zoomviewcameraheight_f=t.playercontrol.thirdperson.livecameraheight;
				t.zoomviewcamerafocus_f=t.playercontrol.thirdperson.livecamerafocus;
				t.zoomviewcamerashoulder_f=t.playercontrol.thirdperson.livecamerashoulder;
				if (  t.gridentityobj>0 ) 
				{
					if (  ObjectExist(t.gridentityobj) == 1 ) 
					{
						t.zoomviewcameraangle_f=(0-ObjectAngleY(t.gridentityobj));
						t.tlayerheight_f=ObjectPositionY(t.gridentityobj);
					}
				}
			}
			else
			{
				t.zoomviewcamerafocus_f=0;
				t.zoomviewcamerashoulder_f=0;
			}

			//  calculate view from position
			t.daa_f=WrapValue(180-t.zoomviewcameraangle_f);
			t.dcx_f=t.cx_f+(Sin(t.daa_f)*t.zoomviewcamerarange_f);
			t.dcy_f=t.tlayerheight_f+t.zoomviewcameraheight_f;
			t.dcz_f=t.cy_f+(Cos(t.daa_f)*t.zoomviewcamerarange_f);
			t.tcx_f=CurveValue(t.dcx_f,CameraPositionX(),4.0);
			t.tcy_f=CurveValue(t.dcy_f,CameraPositionY(),2.0);
			t.tcz_f=CurveValue(t.dcz_f,CameraPositionZ(),4.0);

			//  if target was entity, view center of it
			if (  t.gridentityinzoomview>0 ) 
			{
				t.tobj=t.entityelement[t.gridentityinzoomview].profileobj;
				if (  t.tobj>0 ) 
				{
					t.viewatx_f=t.cx_f ; t.viewaty_f=t.zoomviewtargety_f+ObjectSizeY(t.tobj)/2.0 ; t.viewatz_f=t.cy_f;
				}
				else
				{
					t.viewatx_f=t.cx_f ; t.viewaty_f=t.zoomviewtargety_f+5 ; t.viewatz_f=t.cy_f;
				}
			}
			else
			{
				t.viewatx_f=t.cx_f ; t.viewaty_f=t.zoomviewtargety_f+5 ; t.viewatz_f=t.cy_f;
			}

			//  set smoothed camera view
			PositionCamera (  t.tcx_f,t.tcy_f,t.tcz_f );
			PointCamera (  t.viewatx_f,t.viewaty_f,t.viewatz_f );
			t.tcamax_f=CameraAngleX() ; t.tcamay_f=CameraAngleY() ; t.tcamaz_f=CameraAngleZ();
			RotateCamera (  0,t.tcamay_f+90,0 );
			MoveCamera (  t.zoomviewcamerashoulder_f );
			RotateCamera (  t.tcamax_f-t.zoomviewcamerafocus_f,t.tcamay_f,t.tcamaz_f );

			#if defined(ENABLEIMGUI)
			//Always display skybox.
			sky_loop();
			#endif

		break;
	}
}

void editor_undoredoprojectstate ( void )
{
	// set as modified
	g.projectmodified=1 ; gridedit_changemodifiedflag ( );
	g.projectmodifiedstatic = 1;
}

void editor_cutcopyclearstate ( void )
{
	//  control enabling of UNDO REDO menu items
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetFileMapDWORD (  1, 474, 0 );
	SetFileMapDWORD (  1, 478, 0 );
	SetFileMapDWORD (  1, 482, 0 );
	SetEventAndWait (  1 );
	#endif
}

void editor_undo ( void )
{
	// undo last stage
	if ( t.ebe.on == 1 )
	{
		ebe_undo();
	}
	else
	{
		if (  t.entityundo.undoperformed == 0 ) 
		{
			terrain_undo ( );
			entity_undo ( );
			editor_undoredoprojectstate ( );
			t.entityundo.undoperformed=1;
		}
	}
}

void editor_redo ( void )
{
	if ( t.ebe.on == 1 )
	{
		ebe_redo();
	}
	else
	{
		// redo last stage
		if ( t.entityundo.undoperformed == 1 ) 
		{
			entity_redo ( );
			terrain_redo ( );
			editor_undoredoprojectstate ( );
			t.entityundo.undoperformed=0;
		}
	}
}

void gridedit_showtobjlegend ( void )
{
	t.relaytostatusbar_s="";
	if (  t.tobj>0 ) 
	{
		if (  ObjectExist(t.tobj)>0 ) 
		{
			if (  t.taddstaticlegend == 1 ) 
			{
				//  static
				t.tname_s=t.tname_s+" "+t.strarr_s[608];
			}
			else
			{
				//  dynamic
				t.tname_s=t.tname_s+" "+t.strarr_s[609];
			}
			if (  t.gridedit.autoflatten == 1  )  t.tname_s = t.tname_s+"(autoflatten)";
			if (  t.gridedit.entityspraymode == 1  )  t.tname_s = t.tname_s+"(spray mode)";
			t.relaytostatusbar_s=t.tname_s;
		}
	}
	return;
}

void editor_checkIfInSubApp ( void )
{
	t.result = 0;
	#ifdef VRTECH
	///if ( g_bCharacterCreatorPlusActivated == true ) charactercreatorplus_free(); //if ( t.characterkit.loaded == 1 ) characterkit_free ( );
	#else
	//if (t.characterkit.loaded == 1) characterkit_free();
	if ( t.importer.loaded == 1 ) importer_free ( );
	#endif
}

int findentitycursorobj ( int currentlyover )
{
	// Don't update this every frame as it is extremely intensive. 60->4 fps drop
	// If the user is scrolling, return out also
	if ( currentlyover != -1 )
	{
		//Exit out if within the time limit / scroll keys are pressed etc
		if ( Timer() - t.lastfindentitycursorobjTime < 250 || ( t.inputsys.keyup || t.inputsys.keyleft || t.inputsys.keydown || t.inputsys.keyright ) )
		{
			if ( t.lastfindentitycursorobj > 0 )
			{
				if ( t.entityelement[t.lastfindentitycursorobj].obj > 0 )
				{
					if ( ObjectExist ( t.entityelement[t.lastfindentitycursorobj].obj ) == 0 )
						t.lastfindentitycursorobj = 0;
				}
			}

			return t.lastfindentitycursorobj;
		}

		t.lastfindentitycursorobjTime = Timer();
	}

	float tadjustedtoareax_f = 0;
	float tadjustedtoareay_f = 0;
	float tbestdist_f = 0;
	int efinish = 0;
	float tdist_f = 0;
	float tdstx_f = 0;
	float tdsty_f = 0;
	float tdstz_f = 0;
	int tlayers = 0;
	float tsize_f = 0;
	int estart = 0;
	int result = 0;
	int tokay = 0;
	int e;
	int o;
	int c;
	result=0 ; tbestdist_f=99999.0;

	//PE: This could use some love (really slow), not even sure the tlayers system is actually working as it should.
	if (  currentlyover <= 0 ) { estart = 1  ; efinish = g.entityelementlist; }
	if (  currentlyover >= 1 ) { estart = currentlyover  ; efinish = currentlyover; }
	for ( tlayers = 0 ; tlayers<=  2; tlayers++ )
	{
		for ( e = estart ; e <= efinish; e++ )
		{
			// 301115 - skip if in marker mode and not a marker
			int tentid = t.entityelement[e].bankindex;
			if ( t.gridentitymarkersmodeonly == 1 && t.entityprofile[tentid].ismarker==0 )
				continue;

			// when ignore non seletables on (due to alpha slice), only do this for 'addhandle' entities
			// 190520 - special case, when TAB doing alpha slice, set this flag 
			// (so that any objects with a 'handle' only detect that handle and not the huge
			// invisible quad that stops selection of other entities)
			int iIgnoreSomeNonSelectables = 0;
			if (t.gridnearcameraclip > 0)
				if (t.entityprofile[tentid].addhandlelimb != 0)
					iIgnoreSomeNonSelectables = 1;

			// is entity valid
			o=t.entityelement[e].obj;
			if (  o>0 ) 
			{
				if (  ObjectExist(o) == 1 ) 
				{
					if (  GetVisible(o) == 1 ) 
					{
						tokay=1;
						if ( 1 ) 
						{
							if (  g.gridlayershowsingle == 1 && tlayers == 0 ) 
							{
								//  do not select if TAB slice mode active and entity too big (buildings, walls, etc)
								if (  ObjectSizeX(o)>95 && ObjectSizeY(o)>95 && ObjectSizeZ(o)>95 ) 
								{
									tokay=0;
								}
							}
							if (  tlayers == 0 ) 
							{
								// 041115 - a sphere check falsely ignores things like doors which you are
								// outside of when the doors are closed (animated, etc)
								sObject* pObject = GetObjectData ( o );
								if ( CameraPositionX(0) > pObject->collision.vecMin.x && CameraPositionX(0) < pObject->collision.vecMax.x )
								{
									if ( CameraPositionY(0) > pObject->collision.vecMin.y && CameraPositionY(0) < pObject->collision.vecMax.y )
									{
										if ( CameraPositionZ(0) > pObject->collision.vecMin.z && CameraPositionZ(0) < pObject->collision.vecMax.z )
										{
											tokay=0;
										}
									}
								}
							}
							if ( t.inputsys.keyspace == 0 && g.entityrubberbandlist.size() == 0 )  // 010416 - t.inputsys.keyshift == 0 && g.entityrubberbandlist.size() == 0 ) 
							{
								//  SPACE key can bypass the lock system
								if ( t.entityelement[e].editorlock == 1 ) 
								{
									#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
									//PE: imgui Need testing.
									tadjustedtoareax_f = ((float)t.inputsys.xmouse / (float)GetDisplayWidth()) / ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
									tadjustedtoareay_f = ((float)t.inputsys.ymouse / (float)GetDisplayHeight()) / ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
									#else
									//  work out visible part of full backbuffer (i.e. 1212 of 1360)
									tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
									tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
									//  scale full mouse to fit in visible area
									tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/tadjustedtoareax_f;
									tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/tadjustedtoareay_f;
									#endif
									//  then provide in a format for the pick-from-screen command
									#ifdef DX11
									tadjustedtoareax_f=tadjustedtoareax_f*(GetDisplayWidth()+0.0);
									tadjustedtoareay_f=tadjustedtoareay_f*(GetDisplayHeight()+0.0);
									#else
									tadjustedtoareax_f=tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
									tadjustedtoareay_f=tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
									#endif
									c=PickScreenObjectEx(tadjustedtoareax_f,tadjustedtoareay_f,o,o,0,iIgnoreSomeNonSelectables);
									if (  c != 0  )  g.gentityundercursorlocked = e;
									tokay=0;
								}
							}
						}
						if (  tokay == 1 ) 
						{
							#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
							//PE: imgui Need testing.
							tadjustedtoareax_f = ((float)t.inputsys.xmouse / (float)GetDisplayWidth()) / ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
							tadjustedtoareay_f = ((float)t.inputsys.ymouse / (float)GetDisplayHeight()) / ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
							#else
							//  work out visible part of full backbuffer (i.e. 1212 of 1360)
							tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
							tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
							//  scale full mouse to fit in visible area
							tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/tadjustedtoareax_f;
							tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/tadjustedtoareay_f;
							#endif

							//  then provide in a format for the pick-from-screen command
							#ifdef DX11
							tadjustedtoareax_f=tadjustedtoareax_f*(GetDisplayWidth()+0.0);
							tadjustedtoareay_f=tadjustedtoareay_f*(GetDisplayHeight()+0.0);
							#else
							tadjustedtoareax_f=tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
							tadjustedtoareay_f=tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
							#endif
							#ifdef VRTECH
							c=PickScreenObjectEx(tadjustedtoareax_f,tadjustedtoareay_f,o,o,0,iIgnoreSomeNonSelectables);
							#else
							if(t.gridnearcameraclip == -1)
								c=PickScreenObject(tadjustedtoareax_f,tadjustedtoareay_f,o,o);
							else
								c=PickScreenObjectFromHeight(tadjustedtoareax_f, tadjustedtoareay_f, o, o, t.gridnearcameraclip-2.0f);
							#endif
							if (  c != 0 ) 
							{
								tdstx_f = GetPickVectorX();
								tdsty_f = GetPickVectorY();
								tdstz_f = GetPickVectorZ();
								tdist_f = Sqrt(abs(tdstx_f*tdstx_f)+abs(tdsty_f*tdsty_f)+abs(tdstz_f*tdstz_f));
								bool bIsMarker = false;
								if ( t.entityprofile[t.entityelement[e].bankindex].ismarker != 0 ) { tdist_f = 0; bIsMarker = true; }
								if ( tdist_f<tbestdist_f ) 
								{
									// 201015 - also ensure the point is BELOW any clipping
									float fTryLastPickedY = CameraPositionY() + GetPickVectorY();
									if (t.gridnearcameraclip > 0) {
										fTryLastPickedY = GetFromVectorY() + GetPickVectorY();
									}

									if ( bIsMarker==true || t.gridnearcameraclip == -1 || fTryLastPickedY < t.gridnearcameraclip + 20.0f )
									{
										if (t.gridnearcameraclip > 0) {
											g.glastpickedx_f = GetFromVectorX() + GetPickVectorX();
											g.glastpickedy_f = fTryLastPickedY;
											g.glastpickedz_f = GetFromVectorZ() + GetPickVectorZ();
										}
										else {
											g.glastpickedx_f = CameraPositionX() + GetPickVectorX();
											g.glastpickedy_f = fTryLastPickedY;
											g.glastpickedz_f = CameraPositionZ() + GetPickVectorZ();
										}
										tbestdist_f = tdist_f; result = e;
									}
								}
							}
						}
					}
				}
			}
		}
		if (  t.playercontrol.thirdperson.enabled == 1 ) 
		{
			//  if third person char, redirect to start marker
			if (  result>0 ) 
			{
				if (  result == t.playercontrol.thirdperson.charactere ) 
				{
					result=t.playercontrol.thirdperson.startmarkere;
				}
			}
		}
		if (  tlayers == 0 && result>0 ) 
		{
			if (  currentlyover == -1  )  g.gentityundercursorlocked = 0;
			if ( t.entityelement[result].editorlock == 1 )
			{
				// remove lock visual effect (zwrite)
				t.tentid = t.entityelement[result].bankindex;
				t.tte = result; entity_converttoinstance();
			}
			t.entityelement[result].editorlock=0;
			t.lastfindentitycursorobj = result;
			return result;
		}
	}
	if (  result>0 && currentlyover == -1 ) 
	{
		g.gentityundercursorlocked=0;
		t.entityelement[result].editorlock=0;
	}
	t.lastfindentitycursorobj = result;
	return result;
}

void gridedit_clearentityrubberbandlist ( void )
{
	if ( g.entityrubberbandlist.size() > 0 )
	{
		for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
		{
			int e = g.entityrubberbandlist[i].e;
			//PE: Got exception here: e was to large.
			if (e <= t.entityelement.size()) {
				int tobj = t.entityelement[e].obj;
				if (tobj > 0)
				{
						SetAlphaMappingOn(tobj, 100);
				}
			}
		}
	}
	g.entityrubberbandlist.clear();
}

void gridedit_addEntityToRubberBandHighlights ( int e )
{
	// 011215 - skip if in marker mode and not a marker
	if ( t.gridentitymarkersmodeonly == 1 && t.entityprofile[t.entityelement[e].bankindex].ismarker==0 ) 
		return;

	#ifdef ALLOWSELECTINGLOCKEDOBJECTS
	if (t.entityelement[e].editorlock) return;
	#endif
	// add entity to rubber band
	int tobj = t.entityelement[e].obj;
	bool bEntityIsHighlighted = false;
	if ( g.entityrubberbandlist.size() > 0 )
	{
		for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
		{
			int thise = g.entityrubberbandlist[i].e;
			if ( e == thise ) bEntityIsHighlighted = true;
		}
	}
	if ( bEntityIsHighlighted == false )
	{
		sRubberBandType rubberbandItem;
		rubberbandItem.e = e;
		rubberbandItem.x = t.entityelement[e].x;
		rubberbandItem.y = t.entityelement[e].y;
		rubberbandItem.z = t.entityelement[e].z;
		g.entityrubberbandlist.push_back ( rubberbandItem );
		if ( t.entityelement[e].staticflag == 0 ) 
			SetAlphaMappingOn ( tobj, 103 );
		else
			SetAlphaMappingOn ( tobj, 101 );
	}
}

void gridedit_mapediting ( void )
{
	//  Determine if cursor is at rest, allows performance boost to skip
	//  expensive ray casts to find entities in map
	if (  t.inputsys.atrest != 2 ) 
	{
		t.inputsys.atrest=0;
		if (  t.inputsys.localx_f == t.inputsys.atrestx && t.inputsys.localy_f == t.inputsys.atresty ) 
		{
			t.inputsys.atrest=1;
		}
		//  never at rest when widget menu active
		if (  t.widget.pickedSection != 0  )  t.inputsys.atrest = 0;
	}
	else
	{
		if (  t.inputsys.localx_f != t.inputsys.atrestx || t.inputsys.localy_f != t.inputsys.atresty ) 
		{
			t.inputsys.atrest=0;
		}
	}

	t.inputsys.atrestx=t.inputsys.localx_f;
	t.inputsys.atresty=t.inputsys.localy_f;

	//  flag to determine if character will attach to start marker
	t.inputsys.willmakethirdperson=0;

	//  Only if within map
	if (  t.inputsys.mmx >= 0 && t.inputsys.mmy >= 0 && t.inputsys.mmx<t.maxx && t.inputsys.mmy<t.maxy ) 
	{
		//  Any click inside 3D area constitues some sort of edit
		if ( t.inputsys.mclick != 0 ) 
		{ 
			g.projectmodified = 1; 
			gridedit_changemodifiedflag ( ); 
			// effect on g.projectmodifiedstatic
		}

		//  ENTITY EDIT Handling (onedrag=0 means no waypoint dragging)
		if (  t.grideditselect !=  5  )  t.tentitytoselect  =  0;

		if (  t.grideditselect == 5 && t.onedrag == 0 ) 
		{
			//  Regular entity editing
			t.layer=t.gridlayer ; t.mx=t.inputsys.mmx ; t.my=t.inputsys.mmy;

			if (  t.selstage == 0 ) 
			{
				//  single entity highlight
				t.tshowasstatic=0 ; t.showentityid=0 ; t.tforcedynamic=0 ; t.tentitytoselect=0;

				if ( t.gridentity == 0 ) 
				{
					//  no entity attached to cursor (if RMB, deactivate entity detection for smoother moving)
					if ( t.widget.activeObject == 0 && t.inputsys.xmouse != 500000 && t.inputsys.mclick != 2 && t.inputsys.rubberbandmode == 0 ) 
					{
						if ( t.inputsys.atrest == 1 || t.inputsys.keyspace == 1 )
						{
							t.tentitytoselect = findentitycursorobj(-1);
							t.tlasttentitytoselect = t.tentitytoselect;
							t.inputsys.atrest = 2;
						}
						else
						{
							//  quickly check if over SAME object, if so, keep selection
							if ( findentitycursorobj(t.tlasttentitytoselect)>0 ) 
							{
								//  yep, hovering over same entity
								t.tentitytoselect=t.tlasttentitytoselect;
							}
							else
							{
								//  nope, end hover selection
								t.tentitytoselect=0;
							}
						}
					}
					else
					{
						t.tentitytoselect=0;
					}

					bool bActivateRubberBand = true;

					if (bActivateRubberBand && t.inputsys.mclick == 1 && t.inputsys.rubberbandmode == 0 && t.widget.activeObject == 0 )
					{
						// clear any previous highlights
						if ( t.inputsys.keycontrol ==  0 )
						{
							{
								gridedit_clearentityrubberbandlist();
							}
						}
						else
						{
							// if clicked a single entity WHILE holding control, can add to list
							if ( t.tentitytoselect > 0 )
							{
								gridedit_addEntityToRubberBandHighlights ( t.tentitytoselect );
							}
						}

						// when select entity (widget called up), if parent to children, add them to rubberband so they can all be modified at the same time
						if ( t.tentitytoselect > 0 )
						{
							bool bHasChildren = false;
							t.tstoreentityindexofprimaryhightlighted = 0;
							for ( int te = 1; te <= g.entityelementlist; te++ )
							{
								if ( t.entityelement[te].iHasParentIndex == t.tentitytoselect && t.entityelement[te].obj > 0 )
								{
									gridedit_addEntityToRubberBandHighlights ( te );
									editor_rec_addchildrentorubberband ( te );
									bHasChildren = true;
								}
							}
							if ( bHasChildren == true )
							{
								if ( t.inputsys.k_s != "l" ) 
								{
									gridedit_addEntityToRubberBandHighlights ( t.tentitytoselect );
								}
							}
						}
					}

					if (bActivateRubberBand && t.tentitytoselect > 0 && t.entityelement[t.tentitytoselect].obj > 0 )
					{
						// specific entity highlighted 
						PositionObject (  t.editor.objectstartindex+5,t.entityelement[t.tentitytoselect].x,t.entityelement[t.tentitytoselect].y,t.entityelement[t.tentitytoselect].z );
						t.showentityid=t.entityelement[t.tentitytoselect].bankindex;
						if (  t.entityprofile[t.showentityid].ismarker == 3 || t.entityprofile[t.showentityid].ismarker == 6 || t.entityprofile[t.showentityid].ismarker == 8 ) 
						{
							//  trigger zone or checkpoint
							t.tscale_f=100;
						}
						else
						{
							if (  t.entityprofile[t.showentityid].islightmarker == 1 ) 
							{
								t.tscale_f=(100/3.0)*2*(t.entityelement[t.tentitytoselect].eleprof.light.range/50.0);
							}
							else
							{
								t.tscale_f = get_cursor_scale_for_obj(t.entityelement[t.tentitytoselect].obj);
							}
						}
						ScaleObject (  t.editor.objectstartindex+5,t.tscale_f,t.tscale_f,t.tscale_f );
						t.tshowasstatic=1+t.entityelement[t.tentitytoselect].staticflag;
					}
					else
					{
						// 201015 - if not highlighting an entity, click to start dragging a rubber band box
						if (bActivateRubberBand && t.widget.activeObject == 0 && t.inputsys.xmouse != 500000 )
						{
							if ( t.inputsys.mclick == 1 )
							{
								// start rubber band box
								if ( t.inputsys.rubberbandmode == 0 )
								{
									t.inputsys.rubberbandmode = 1;
									t.inputsys.spacekeynotreleased = 1;
									//PE: Make mouse relative to window pos.
									t.inputsys.rubberbandx = ImGuiGetMouseX();
									t.inputsys.rubberbandy = ImGuiGetMouseY();
								}
							}
						}
					}


					// 201015 - rubber band effect and control
					if (bActivateRubberBand && t.inputsys.rubberbandmode == 1 )
					{
						//bWaitOnMouseRelease
						bool bCancelRubberBand = false;
						if ( t.inputsys.xmouse == 500000 || bCancelRubberBand )
						{
							// mouse left area, cancel rubber band
							t.inputsys.rubberbandmode = 0;
						}
						else
						{
							#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
							//PE: imgui Need testing.
							// draw and detect
							float fMX = (GetChildWindowWidth(-1) + 0.0) / (float)GetDisplayWidth(); //GetChildWindowWidth(1)
							float fMY = (GetChildWindowHeight(-1) + 0.0) / (float)GetDisplayHeight(); //GetChildWindowHeight(-1)
							#else
							// draw and detect
							float fMX = (GetChildWindowWidth(1)+0.0) / 800.0f;
							float fMY = (GetChildWindowHeight(1)+0.0) / 600.0f;
							#endif
						
							// reverse bound box if inside out

							float fCurrentRubberBandX1 = t.inputsys.rubberbandx;
							float fCurrentRubberBandX2 = ImGuiGetMouseX();
							float fCurrentRubberBandY1 = t.inputsys.rubberbandy;
							float fCurrentRubberBandY2 = ImGuiGetMouseY();
							if ( fCurrentRubberBandX1 > fCurrentRubberBandX2 ) { float fStore = fCurrentRubberBandX1; fCurrentRubberBandX1 = fCurrentRubberBandX2; fCurrentRubberBandX2 = fStore; }
							if ( fCurrentRubberBandY1 > fCurrentRubberBandY2 ) { float fStore = fCurrentRubberBandY1; fCurrentRubberBandY1 = fCurrentRubberBandY2; fCurrentRubberBandY2 = fStore; }

							// detect all entities within box and highlight
							for ( int e = 1; e <= g.entityelementlist; e++ )
							{
								// 060116 - if locked and holding space, unlock it now
								if (t.inputsys.keyspace != 0) {
									t.entityelement[e].editorlock = false;
									sObject* pObject;
									if (t.entityelement[e].obj > 0) {
										pObject = g_ObjectList[t.entityelement[e].obj];
									}
								}

								// only if not locked
								if ( t.entityelement[e].editorlock==false )
								{
									int tobj = t.entityelement[e].obj;
									if ( tobj > 0 )
									{
										int iEntityScreenX = GetScreenX(tobj);
										int iEntityScreenY = GetScreenY(tobj);
										if ( iEntityScreenX > fCurrentRubberBandX1*fMX && iEntityScreenX < fCurrentRubberBandX2*fMX )
										{
											if ( iEntityScreenY > fCurrentRubberBandY1*fMY && iEntityScreenY < fCurrentRubberBandY2*fMY )
											{
												// only if not already highlighted
												gridedit_addEntityToRubberBandHighlights ( e );
											}
										}
									}
								}
							}

							// now de-highlight any in the list NOT covered by the boundbox
							if ( g.entityrubberbandlist.size() > 0 )
							{
								int i = 0;
								while ( i < (int)g.entityrubberbandlist.size() )
								{
									bool bThisOneInBox = false;
									int e = g.entityrubberbandlist[i].e;
									int tobj = t.entityelement[e].obj;
									if ( tobj > 0 )
									{
										int iEntityScreenX = GetScreenX(tobj);
										int iEntityScreenY = GetScreenY(tobj);
										if ( iEntityScreenX > fCurrentRubberBandX1*fMX && iEntityScreenX < fCurrentRubberBandX2*fMX )
										{
											if ( iEntityScreenY > fCurrentRubberBandY1*fMY && iEntityScreenY < fCurrentRubberBandY2*fMY )
											{
												bThisOneInBox = true;
											}
										}
									}
									if ( bThisOneInBox == false )
									{
										SetAlphaMappingOn ( tobj, 100 );
										g.entityrubberbandlist.erase(g.entityrubberbandlist.begin() + i);
										i = 0;
									}
									else
									{
										i++;
									}
								}
							}

							// when release mouse while rubber banding
							if ( t.inputsys.mclick == 0 )
							{
								// finish rubber banding
								t.inputsys.rubberbandmode = 0;
								// auto choose an entity to act as the widget achor object
								if (g.entityrubberbandlist.size() > 0)
								{
									if (g.entityrubberbandlist.size() == 1)
									{
										// if only range selected on, make it a regular entity selection
										t.widget.pickedEntityIndex = g.entityrubberbandlist[0].e;
										gridedit_clearentityrubberbandlist();
									}
									else
									{
										t.widget.pickedEntityIndex = g.entityrubberbandlist[0].e;
									}
									t.widget.pickedObject = t.entityelement[t.widget.pickedEntityIndex].obj;
									t.widget.offsetx = 0;
									t.widget.offsety = 0;
									t.widget.offsetz = 0;

								}
							}
							else
							{

								float fX1 = t.inputsys.rubberbandx*fMX;
								float fX2 = ImGuiGetMouseX()*fMX;
								float fY1 = t.inputsys.rubberbandy*fMY;
								float fY2 = ImGuiGetMouseY()*fMY;
								if ( fX2 < fX1 )
								{
									float fSt = fX1;
									fX1 = fX2;
									fX2 = fSt;
								}
								if ( fY2 < fY1 )
								{
									float fSt = fY1;
									fY1 = fY2;
									fY2 = fSt;
								}
								Sprite ( 123, -10000, -10000, g.editorimagesoffset+14 );
								SizeSprite ( 123, fX2-fX1, 2 );
								PasteSprite ( 123, fX1, fY1 );
								SizeSprite ( 123, fX2-fX1, 2 );
								PasteSprite ( 123, fX1, fY2-2 );
								SizeSprite ( 123, 2, fY2-fY1 );
								PasteSprite ( 123, fX1, fY1 );
								SizeSprite ( 123, 2, fY2-fY1 );
								PasteSprite ( 123, fX2-2, fY1 );
							}
						}
					}
				}
				else
				{
					//  entity attached to cursor
					PositionObject (  t.editor.objectstartindex+5,t.gridentityposx_f,t.gridentityposy_f,t.gridentityposz_f );
					if (  ObjectExist(g.entitybankoffset+t.gridentity) == 1 ) 
					{
						if (  t.entityprofile[t.gridentity].ismarker == 3 || t.entityprofile[t.gridentity].ismarker == 6 || t.entityprofile[t.gridentity].ismarker == 8 ) 
						{
							//  trigger zone or checkpoint
							t.tscale_f=100;
						}
						else
						{
							if (  t.entityprofile[t.gridentity].islightmarker == 1 || t.gridedit.entityspraymode == 1) 
							{
								if ( t.gridedit.entityspraymode == 1 )
								{
									t.tscale_f=(100/3.0)*2*(t.gridedit.entitysprayrange/50.0);
								}
								else
								{
									t.tscale_f=(100/3.0)*2*(t.grideleprof.light.range/50.0);
								}
							}
							else
							{
								t.tscale_f = get_cursor_scale_for_obj(g.entitybankoffset+t.gridentity);
							}
						}
						ScaleObject (  t.editor.objectstartindex+5,t.tscale_f,t.tscale_f,t.tscale_f );
					}
					//if (  t.entityprofile[t.gridentity].ischaracter == 1 && t.entityprofile[t.gridentity].isthirdperson == 1 ) 
					if (  t.entityprofile[t.gridentity].ischaracter == 1 ) // 220217 - now for all characters
					{
						// third person char+marker detection (will not work in VR edit mode)
						if ( t.playercontrol.thirdperson.enabled == 0 && g.vrqcontrolmode == 0 ) 
						{
							t.tattachtothis=findentitycursorobj(-1);
							if (  t.tattachtothis>0 ) 
							{
								if (  t.entityprofile[t.entityelement[t.tattachtothis].bankindex].ismarker == 1 ) 
								{
									t.tobj=t.entityelement[t.tattachtothis].obj;
									if (  t.tobj>0 ) 
									{
										if (  ObjectExist(t.tobj) == 1 ) 
										{
											t.tmousemodifierx_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
											t.tmousemodifiery_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
											pastebitmapfontcenter("ATTACH FOR THIRD PERSON CONTROL",GetScreenX(t.tobj)*t.tmousemodifierx_f,(GetScreenY(t.tobj)*t.tmousemodifiery_f)+50,2,255);
											t.inputsys.willmakethirdperson=t.tattachtothis;
										}
									}
								}
							}
						}
					}
					t.tshowasstatic=1+t.gridentitystaticmode;
					t.showentityid=t.gridentity;

					// when holding down "L", and holding an entity, this means we are looking to 'link/associate'
					// if dragging about an entity, continue scanning for pointing at another entity
					// so we can have the option of link/associate it as a parent
					if ( t.tentityoverdraggingcursor > 0 && t.tlastentityoverdraggingcursor > 0 )
					{
						int iWhichEntityObj = findentitycursorobj(-1);
						if ( iWhichEntityObj != t.tlastentityoverdraggingcursor || t.inputsys.k_s != "l" ) 
						{
							if ( t.entityelement[t.tlastentityoverdraggingcursor].obj > 0 )
							{
								// only if not EXTRACTed
								SetAlphaMappingOn ( t.entityelement[t.tlastentityoverdraggingcursor].obj, 100 );
							}
							t.tentityoverdraggingcursor = 0;
						}
						else
							t.tentityoverdraggingcursor = t.tlastentityoverdraggingcursor;
					}
					else
					{
						if ( t.inputsys.k_s == "l" ) 
						{
							t.tentityoverdraggingcursor = findentitycursorobj(-1);
							t.tlastentityoverdraggingcursor = t.tentityoverdraggingcursor;
							if ( t.tentityoverdraggingcursor > 0 )
							{
								if ( t.tentstaticmode != 0 ) 
									SetAlphaMappingOn ( t.entityelement[t.tentityoverdraggingcursor].obj, 101 );
								else
									SetAlphaMappingOn ( t.entityelement[t.tentityoverdraggingcursor].obj, 103 );
							}
						}
						else
							t.tentityoverdraggingcursor = 0;
					}
				}

				if (  t.tshowasstatic>0 && t.inputsys.activemouse == 1 ) 
				{
					t.tentityworkobjectchoice=0;
					if (  t.entityprofile[t.showentityid].ismarker == 1  )  t.tforcedynamic = 1;
					if (  t.entityprofile[t.showentityid].ismarker == 4  )  t.tforcedynamic = 1;
					if (  t.entityprofile[t.showentityid].ismarker == 10  )  t.tforcedynamic = 1;
					if (  t.entityprofile[t.showentityid].ismarker == 3 || t.entityprofile[t.showentityid].ismarker == 6 || t.entityprofile[t.showentityid].ismarker == 8 ) 
					{
						//  trigger area or checkpoint
						HideObject (  t.editor.objectstartindex+5 );
						t.tforcedynamic=1;
					}
					else
					{
						ShowObject (  t.editor.objectstartindex+5 );
						if ( t.entityprofile[t.showentityid].islightmarker == 1 || t.gridedit.entityspraymode == 1) 
						{
							//  light range visual
							TextureObject (  t.editor.objectstartindex+5,0,g.editorimagesoffset+18 );
							YRotateObject (  t.editor.objectstartindex+5,0 );
							if (  t.tshowasstatic == 2 ) 
							{
								t.tentityworkobjectchoice=1820;
								modifyplaneimagestrip(t.editor.objectstartindex+5,2,0);
							}
							else
							{
								t.tentityworkobjectchoice=1821;
								modifyplaneimagestrip(t.editor.objectstartindex+5,2,1);
							}
						}
						else
						{
							if (  t.entityprofile[t.showentityid].forwardfacing == 1 ) 
							{
								//  coloured circle blob with arrow
								TextureObject (  t.editor.objectstartindex+5,0,g.editorimagesoffset+26 );
								YRotateObject (  t.editor.objectstartindex+5,t.gridentityrotatey_f );
								t.tentityworkobjectchoice=2600;
							}
							else
							{
								//  coloured circle blob
								TextureObject (  t.editor.objectstartindex+5,0,g.editorimagesoffset+7 );
								YRotateObject (  t.editor.objectstartindex+5,0 );
								t.tentityworkobjectchoice=700;
							}
							if (  t.gridedit.entityspraymode == 1 ) 
							{
								//  spray mode active
								if (  t.tshowasstatic == 2 ) 
								{
									modifyplaneimagestrip(t.editor.objectstartindex+5,8,2);
									t.tentityworkobjectchoice=1882;
								}
								else
								{
									modifyplaneimagestrip(t.editor.objectstartindex+5,8,4);
									t.tentityworkobjectchoice=1884;
								}
							}
							else
							{
								if (  t.gridedit.autoflatten == 1 ) 
								{
									//  rem purple and cyan for auto-flatten
									if (  t.tshowasstatic == 2 ) 
									{
										modifyplaneimagestrip(t.editor.objectstartindex+5,8,7);
										t.tentityworkobjectchoice=1887;
									}
									else
									{
										modifyplaneimagestrip(t.editor.objectstartindex+5,8,4);
										t.tentityworkobjectchoice=1884;
									}
								}
								else
								{
									//  rem red and green for NONE auto-flatten
									if (  t.tshowasstatic == 2 ) 
									{
										modifyplaneimagestrip(t.editor.objectstartindex+5,8,1);
										t.tentityworkobjectchoice=1881;
									}
									else
									{
										modifyplaneimagestrip(t.editor.objectstartindex+5,8,3);
										t.tentityworkobjectchoice=1883;
									}
								}
							}
						}
						ShowObject (  t.editor.objectstartindex+5 );
					}
					//  show legend of entity hovering over (and static legend)
					t.taddstaticlegend=0;
					if (  t.tentitytoselect>0 ) 
					{
						t.tstatic=t.entityelement[t.tentitytoselect].staticflag;
						t.tentid=t.entityelement[t.tentitytoselect].bankindex;
					}
					else
					{
						t.tstatic=t.gridentitystaticmode;
						t.tentid=t.gridentity;
					}
					if (  t.tstatic == 1 ) 
					{
						t.taddstaticlegend=1;
					}
					else
					{
						t.taddstaticlegend=0;
					}
					t.editor.entityworkobjectchoice=t.tentityworkobjectchoice;
					t.editor.entitytoselect=t.tentitytoselect;
					if ( t.tentitytoselect>0 ) 
					{
						t.tobj=t.entityelement[t.tentitytoselect].obj;
						t.tname_s = "" ; t.tname_s=t.tname_s + "["+Str(t.tentitytoselect)+" {"+Str(t.tobj)+"}] "+t.entityelement[t.tentitytoselect].eleprof.name_s;
						t.ttentid=t.entityelement[t.tentitytoselect].bankindex;
						gridedit_showtobjlegend ( );
					}
					else
					{
						t.tobj=t.gridentityobj ; t.tname_s=t.grideleprof.name_s;
						t.ttentid=t.gridentity;
						gridedit_showtobjlegend ( );
					}
				}
				else
				{
					HideObject (  t.editor.objectstartindex+5 );
				}

				editor_refreshentitycursor ( );

				//  prompt when over locked entity
				if (  g.gentityundercursorlocked>0 ) 
				{
					t.relaytostatusbar_s="LOCKED - Hold SPACEBAR and click to unlock";
				}
				else
				{
					if (  t.tentitytoselect == 0 && t.gridentity == 0 ) 
					{
						t.relaytostatusbar_s="None Selected";
					}
				}

				// Entity Edit Mode
				if ( 1 ) 
				{
					bool bPlaceEntity = false;
					if ((t.widget.duplicatebuttonselected == 2 && t.gridentity == 0))
						bPlaceEntity = true;

					if (!bPlaceEntity)
					{
						{
							if ((t.inputsys.mclick == 1 && t.gridentity != 0))
								bPlaceEntity = true;
						}
					}

					{
						//  entity placement update
						if (t.inputsys.mclick == 2)
						{
							//  070415 - in RMB mode, mouse pos is changed so HIDE gridentity obj as it shifts!
							{
								t.gridentityposx_f = -999000;
								t.gridentityposz_f = -999000;
							}
						}
						else
						{
							{
								t.gridentityposx_f = t.inputsys.localx_f + t.inputsys.dragoffsetx_f;
								t.gridentityposz_f = t.inputsys.localy_f + t.inputsys.dragoffsety_f;
							}
						}

						bool bUseOldYSystem = true;

						//PE: Add snap,grid as functions , so can be used in widget system.
						Add_Grid_Snap_To_Position();

						if (t.tforcedynamic == 1)
						{
							t.gridentitystaticmode = 0;
						}

						//PE: Display red/green box of cursor object. to display static/dynamic.

						// Find ground while placing entity on terrain
						bool bApplyEntityOffsets = false;
						bool bCanUpdateY = true;
						if (t.gridentitysurfacesnap == 1 || t.inputsys.picksystemused == 2)
						{

							// new system to locate grid ent pos at point where mouse touches terrain/entity surface
							// LB: Note, "t.inputsys.localcurrentterrainheight_f" will not account for 'entity surface' if iObjectMoveMode != 2!!
							if (bUseOldYSystem && bCanUpdateY)
							{
								if (t.inputsys.localcurrentterrainheight_f != 0.0f) 
								{
									t.gridentityposy_f = t.inputsys.localcurrentterrainheight_f;
									bApplyEntityOffsets = true;
								}
							}

						}
						else
						{
							if (t.gridentityposoffground == 0)
							{
								if (t.terrain.TerrainID > 0)
								{
									if (bUseOldYSystem)
									{
										t.gridentityposy_f = BT_GetGroundHeight(t.terrain.TerrainID, t.gridentityposx_f, t.gridentityposz_f);
										bApplyEntityOffsets = true;
									}
								}
								else
								{
									if (bUseOldYSystem)
									{
										t.gridentityposy_f = g.gdefaultterrainheight;
										bApplyEntityOffsets = true;
									}
								}
							}
						}
						if (bApplyEntityOffsets == true)
						{
							// for markers, apply Y offset
							if (t.entityprofile[t.gridentity].ismarker != 0)
							{
								if (bUseOldYSystem)
									t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].offy;
							}

							// for entities with default height, apply Y offset
							if (t.entityprofile[t.gridentity].defaultheight != 0)
							{
								if (bUseOldYSystem)
									t.gridentityposy_f = t.gridentityposy_f + t.entityprofile[t.gridentity].defaultheight;
							}
						}


						// Create and manage a ghost object for when selecting objects to move
						// so can be used by special smart positioning mode


						//  move waypoint zone when move trigger entity
						if (t.grideleprof.trigger.waypointzoneindex > 0)
						{
							if (t.gridentity > 0)
							{
								waypoint_movetogrideleprof();
							}
							else
							{
								t.grideleprof.trigger.waypointzoneindex = 0;
							}
						}

						//  control modification of entity element details
						if (t.gridentitymodifyelement == 1)
						{
							if (t.gridedit.entityspraymode == 1)
							{
								if (t.gridedit.entitysprayrange > 0) t.gridedit.entitysprayrange -= 50;
							}
							else
							{
								if (t.grideleprof.light.range > 50) t.grideleprof.light.range -= 50;
							}
							t.gridentitymodifyelement = 0;
						}
						if (t.gridentitymodifyelement == 2)
						{
							if (t.gridedit.entityspraymode == 1)
							{
								if (t.gridedit.entitysprayrange < 1000) t.gridedit.entitysprayrange += 50;
							}
							else
							{
								if (t.grideleprof.light.range < 1000) t.grideleprof.light.range += 50;
							}
							t.gridentitymodifyelement = 0;
						}

					}

					//  extract entity (RMB) or place entity (LMB)
					if (bPlaceEntity)
					{
						// widget closure
						if (  t.widget.duplicatebuttonselected == 2 ) 
						{
							t.tentitytoselect=t.widget.pickedEntityIndex;
							t.widget.duplicatebuttonselected=0;
							t.gridentityautofind=7;
						}
						t.widget.pickedObject=0; 
						widget_updatewidgetobject ( );

						// either add entity to map OR extract one specified by 't.tentitytoselect'
						if ( t.gridentity != 0 ) 
						{
							// ADD ENTITY TO MAP
							// Determine if we will be adding (or moving an entity if spray mode)
							t.tentitybeingsprayed=0;
							t.tentitytomodifyindex=0;
							if ( t.gridedit.entityspraymode == 1 && t.entityprofile[t.gridentity].ismarker == 0 ) 
							{
								//  Scan area of spray and determine if density reached
								t.tpickrandoment=Rnd(t.tcountentinrange);
								t.tcountentinrange=0;
								for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
								{
									if (  t.entityelement[t.e].bankindex == t.gridentity ) 
									{
										//  this entity is same as that sprayed
										t.tdx_f=t.entityelement[t.e].x-t.gridentityposx_f;
										t.tdz_f=t.entityelement[t.e].z-t.gridentityposz_f;
										t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
										if (  t.tdd_f <= t.gridedit.entitysprayrange ) 
										{
											//  this entity in range
											if (  t.tcountentinrange == 0  )  t.tentitytomodifyindex = t.e;
											if (  t.tcountentinrange == t.tpickrandoment  )  t.tentitytomodifyindex = t.e;
											++t.tcountentinrange;
										}
									}
								}
								if (  t.tcountentinrange <= 5  )  t.tentitytomodifyindex = 0;
								t.tentitybeingsprayed=1;
							}

							//  Store original entity cursor settings (for later random feature)
							t.storegridentityposx_f=t.gridentityposx_f;
							t.storegridentityposz_f=t.gridentityposz_f;
							t.storegridentityrotatex_f=t.gridentityrotatex_f;
							t.storegridentityrotatey_f=t.gridentityrotatey_f;
							t.storegridentityrotatez_f=t.gridentityrotatez_f;
							t.storegridentityscalex_f=t.gridentityscalex_f;
							t.storegridentityscaley_f=t.gridentityscaley_f;
							t.storegridentityscalez_f=t.gridentityscalez_f;

							//  Spray allows entity placement to be randomised
							if (  t.tentitybeingsprayed == 1 ) 
							{
								//  rotation and scale back in (also uses native entity ROT values)
								t.gridentityrotatex_f=t.entityprofile[t.gridentity].rotx;
								t.gridentityrotatey_f=t.entityprofile[t.gridentity].roty;
								t.gridentityrotatez_f=t.entityprofile[t.gridentity].rotz;
								t.gridentityscalex_f=t.gridentityscalex_f;
								t.gridentityscaley_f=t.gridentityscaley_f;
								t.gridentityscalez_f=t.gridentityscalez_f;
								if ( t.entityprofile[t.gridentity].ischaracter==0 && t.entityprofile[t.gridentity].noXZrotation==0 )
								{
									// ignore X and Z rotation for characters
									t.gridentityrotatex_f=t.gridentityrotatex_f+10.0-Rnd(20);
									t.gridentityrotatez_f=t.gridentityrotatez_f+10.0-Rnd(20);
									t.gridentityscaley_f=t.gridentityscaley_f+(Rnd(20)-10);
								}
								t.gridentityrotatey_f=t.gridentityrotatey_f+Rnd(360);
								t.ttrandomposx_f=NewXValue(0,Rnd(360),Rnd(t.gridedit.entitysprayrange));
								t.ttrandomposz_f=NewZValue(0,Rnd(360),Rnd(t.gridedit.entitysprayrange));
								t.gridentityposx_f=t.gridentityposx_f+t.ttrandomposx_f;
								t.gridentityposz_f=t.gridentityposz_f+t.ttrandomposz_f;
								if (  t.terrain.TerrainID>0 ) 
								{
									t.gridentityposy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.gridentityposx_f,t.gridentityposz_f);
								}
								else
								{
									t.gridentityposy_f=g.gdefaultterrainheight;
								}

							}

							//  Version Control - stop high resource use
							t.resourceused=2; //version_resourcewarning ( );

							//  Either modify existing entity or place a new one (default behaviour)
							if (  t.tentitytomodifyindex>0 ) 
							{
								//  MODIFY EXISTING ENTITY
								t.entityelement[t.tentitytomodifyindex].x=t.gridentityposx_f;
								t.entityelement[t.tentitytomodifyindex].y=t.gridentityposy_f;
								t.entityelement[t.tentitytomodifyindex].z=t.gridentityposz_f;
								t.entityelement[t.tentitytomodifyindex].rx=t.gridentityrotatex_f;
								t.entityelement[t.tentitytomodifyindex].ry=t.gridentityrotatey_f;
								t.entityelement[t.tentitytomodifyindex].rz=t.gridentityrotatez_f;
								t.tobj=t.entityelement[t.tentitytomodifyindex].obj;
								if (  t.tobj>0 ) 
								{
									if (  ObjectExist(t.tobj) == 1 ) 
									{
										PositionObject (  t.tobj,t.gridentityposx_f,t.gridentityposy_f,t.gridentityposz_f );
										RotateObject (  t.tobj,t.gridentityrotatex_f,t.gridentityrotatey_f,t.gridentityrotatez_f );
									}
								}
							}
							else
							{
								//  PLACE NEW ENTITY
								//  after add, adjust so it auto-finds a Floor (  or wall (convenience) )
								t.gridentitydroptoground=1+t.entityprofile[t.gridentity].forwardfacing;
								if (  t.gridentitydroptoground == 2 ) 
								{
									//  not too convenient for floors, but wall furniture look okay!!
									t.thardauto=1  ; editor_findentityground ( );
								}
								t.gridentitydroptoground=0;

								//  find unique name for this selection (if flagged)
								if (  g.guseuniquelynamedentities == 0 ) 
								{
									//  use same name as original entity
									t.tbase_s=t.grideleprof.name_s;
								}
								else
								{
									t.tokay=0 ; t.tindex=1;
									if (  cstr(Lower(Left(t.grideleprof.name_s.Get(),Len(t.grideleproflastname_s.Get())))) == Lower(t.grideleproflastname_s.Get()) ) 
									{
										t.tbase_s=t.grideleproflastname_s;
									}
									else
									{
										t.tbase_s=t.grideleprof.name_s;
									}
									while (  t.tokay == 0 ) 
									{
										t.tokay=1 ; t.grideleprof.name_s=t.tbase_s ; t.grideleproflastname_s=t.tbase_s;
										if (  t.tindex>1  )  t.grideleprof.name_s = t.grideleprof.name_s+" "+Str(t.tindex);
										for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
										{
											if (  t.entityelement[t.e].bankindex>0 ) 
											{
												if (  cstr(Lower(t.entityelement[t.e].eleprof.name_s.Get())) == cstr(Lower(t.grideleprof.name_s.Get())) ) 
												{
													//  this name exists already, try another
													t.tokay=0 ; break;
												}
											}
										}
										++t.tindex;
									}
								}
								//  player start markers have exclusivity
								if (  t.entityprofile[t.gridentity].ismarker == 1 && t.entityprofile[t.gridentity].lives != -1 ) 
								{
									for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
									{
										if (  t.entityelement[t.e].bankindex>0 ) 
										{
											if (  t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 1 && t.entityprofile[t.entityelement[t.e].bankindex].lives != -1 ) 
											{
												t.tentitytoselect=t.e;
												gridedit_deleteentityfrommap ( );
											}
										}
									}
								}

								//  copy entity to map (keep selection for repeat process)
								if ( g.entityrubberbandlist.size() > 0 )
								{
									{
										// store original ent ID pased down
										int iStoreGridEntity = t.gridentity;

										// add parent entity
										gridedit_addentitytomap();
										int iNewParentEntityIndex = t.e;

										t.entityelement[t.e].iHasParentIndex = t.gridentityhasparent;

										// add children for the parent
										int* piNewEntIndex = new int[g.entityrubberbandlist.size()];
										for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
										{
											int e = g.entityrubberbandlist[i].e;
											if (e == 0)
											{
												t.e = iNewParentEntityIndex;
											}
											else
											{
												// duplicate this child and add to map
												t.gridentity = t.entityelement[e].bankindex;
												gridedit_addentitytomap(); //use entityelement[t.e].

												// update child with pos/rot from source
												t.entityelement[t.e].x = t.entityelement[e].x;
												t.entityelement[t.e].y = t.entityelement[e].y;
												t.entityelement[t.e].z = t.entityelement[e].z;
												t.entityelement[t.e].rx = t.entityelement[e].rx;
												t.entityelement[t.e].ry = t.entityelement[e].ry;
												t.entityelement[t.e].rz = t.entityelement[e].rz;
												t.entityelement[t.e].editorfixed = t.entityelement[e].editorfixed;
												t.entityelement[t.e].staticflag = t.entityelement[e].staticflag;
												t.entityelement[t.e].scalex = t.entityelement[e].scalex;
												t.entityelement[t.e].scaley = t.entityelement[e].scaley;
												t.entityelement[t.e].scalez = t.entityelement[e].scalez;
												t.entityelement[t.e].soundset = t.entityelement[e].soundset;
												t.entityelement[t.e].soundset1 = t.entityelement[e].soundset1;
												t.entityelement[t.e].soundset2 = t.entityelement[e].soundset2;
												t.entityelement[t.e].soundset3 = t.entityelement[e].soundset3;
												t.entityelement[t.e].soundset4 = t.entityelement[e].soundset4;
												t.entityelement[t.e].eleprof = t.entityelement[e].eleprof;
												PositionObject(t.entityelement[t.e].obj, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z);
												RotateObject(t.entityelement[t.e].obj, t.entityelement[t.e].rx, t.entityelement[t.e].ry, t.entityelement[t.e].rz);
											}
											piNewEntIndex[i] = t.e;
										}

										// and once all new entities created, link new parents to them
										for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
										{
											t.e = piNewEntIndex[i];
											int e = g.entityrubberbandlist[i].e;
											if (e != 0)
											{
												// find source parent of this child, if any
												if (t.entityelement[e].iHasParentIndex > 0)
												{
													if (t.entityelement[e].iHasParentIndex == t.gridentityextractedindex)
													{
														// entity was child of parent entity extacted
														t.entityelement[t.e].iHasParentIndex = iNewParentEntityIndex;
													}
													else
													{
														// entity was child of another entity (a child in here)
														for (int ii = 0; ii < (int)g.entityrubberbandlist.size(); ii++)
														{
															int ee = g.entityrubberbandlist[ii].e;
															if (ee > 0)
															{
																if (t.entityelement[e].iHasParentIndex == ee)
																{
																	// entity was child of parent entity extacted
																	t.entityelement[t.e].iHasParentIndex = piNewEntIndex[ii];
																}
															}
														}
													}
												}
											}
										}
										SAFE_DELETE(piNewEntIndex);

										// restore original ent ID
										t.gridentity = iStoreGridEntity;
										t.e = iNewParentEntityIndex;
									}
								}
								else
								{
									#ifdef VRTECH
									//TUT: Add here.
									//TUT: PLACEIT
									CheckTutorialAction("PLACEIT"); //Tutorial: check if we are waiting for this action
									if (bTutorialCheckAction) TutorialNextAction(); //If we are waiting for PLACEIT its done.
									#endif
									gridedit_addentitytomap ( );

								}

								//  if drag char to start marker, assign here
								if (  t.inputsys.willmakethirdperson>0 ) 
								{
									// set this characte as third person and game as TPP 
									t.playercontrol.thirdperson.enabled=1;
									t.playercontrol.thirdperson.charactere=t.tupdatee;
									t.playercontrol.thirdperson.startmarkere=t.inputsys.willmakethirdperson;

									// also change the script of the character to a third person script (by default)
									t.entityelement[t.tupdatee].eleprof.aimain_s = "tpp\\thirdperson.lua";
									t.entityelement[t.tupdatee].eleprof.aimain = 0;
								}

								//  if trigger zone, remove from entity cursor as well
								if (  t.entityprofile[t.gridentity].ismarker == 3 || t.entityprofile[t.gridentity].ismarker == 6 || t.entityprofile[t.gridentity].ismarker == 8 ) 
								{
									//  detatch trigger zone / checkpoint here
									t.grideleprof.trigger.waypointzoneindex=0;
									t.gridentitydelete=1;
								}
								else
								{
									//  update for refresh
									t.refreshgrideditcursor=1;
								}

								// if was targetting a parent for link/associate connection (CTRL down)
								// then make this entity a child of the entity targetted
								if ( t.tentityoverdraggingcursor > 0 )
								{
									// parents influence children when they move, and shift children relatively
									t.entityelement[t.e].iHasParentIndex = t.tentityoverdraggingcursor;
								}
								else
								{
									// if still holding CTRL put place entity down on NON-entity, remove parent link/associated status
									if ( t.inputsys.k_s == "l" ) 
									{
										t.entityelement[t.e].iHasParentIndex = 0;
									}
								}

								//  080415 - if NOT holding SHIFT, delete after one placement
								bool bShiftBeingHeldDown = false;
								if ( t.inputsys.keyshift != 0 ) 
									bShiftBeingHeldDown = true;

								//  260515 - but if in spray mode, continue using entity
								if ( bShiftBeingHeldDown == false && t.gridedit.entityspraymode == 0 ) 
								{
									t.inputsys.kscancode=211;
								}

								t.selstage=1;
							}

							// restore original entity cursor position (after random spray feature)
							t.gridentityposx_f=t.storegridentityposx_f;
							t.gridentityposz_f=t.storegridentityposz_f;
							t.gridentityrotatex_f=t.storegridentityrotatex_f;
							t.gridentityrotatey_f=t.storegridentityrotatey_f;
							t.gridentityrotatez_f=t.storegridentityrotatez_f;
							t.gridentityscalex_f=t.storegridentityscalex_f;
							t.gridentityscaley_f=t.storegridentityscaley_f;
							t.gridentityscalez_f=t.storegridentityscalez_f;

						}
						else
						{
							// EXTRACT ENTITY FROM MAP
							// Set flag so do not instantly delete entity (below)
							t.onetimeentitypickup=1;

							//  extract entity from the map
							if ( t.tentitytoselect>0 ) 
							{
								if ( t.entityelement[t.tentitytoselect].editorfixed == 0 ) 
								{
									t.gridentityeditorfixed=t.entityelement[t.tentitytoselect].editorfixed;
									t.gridentity=t.entityelement[t.tentitytoselect].bankindex;
									t.ttrygridentitystaticmode=t.entityelement[t.tentitytoselect].staticflag;
									t.ttrygridentity=t.gridentity; editor_validatestaticmode ( );
									t.gridedit.autoflatten=t.entityprofile[t.gridentity].autoflatten;
									t.gridedit.entityspraymode=0;
									if ( t.gridentityautofind == 7 ) 
									{
										//  widget extracts without forcing entity to Floor
										t.gridentityautofind=0;
										t.gridentityposoffground=1;
										t.gridentityusingsoftauto=0;
									}
									else
									{
										t.gridentityposoffground=0;
										t.gridentityusingsoftauto=1;
										if ( t.entityprofile[t.gridentity].defaultstatic == 0 && t.entityprofile[t.gridentity].isimmobile == 1 ) 
										{
											t.gridentityautofind=1;
										}
										else
										{
											t.gridentityautofind=0;
										}
									}
									t.gridentitysurfacesnap=0; // surfacesnap off as messes up extract offset for entity
									t.gridentityextractedindex = t.tentitytoselect;
									t.gridentityhasparent = 0;//t.entityelement[t.tentitytoselect].iHasParentIndex; 210317 - break association when extract so can place free of parent
									t.gridentityposx_f=t.entityelement[t.tentitytoselect].x;
									t.gridentityposy_f=t.entityelement[t.tentitytoselect].y;
									t.gridentityposz_f=t.entityelement[t.tentitytoselect].z;
									t.gridentityrotatex_f=t.entityelement[t.tentitytoselect].rx;
									t.gridentityrotatey_f=t.entityelement[t.tentitytoselect].ry;
									t.gridentityrotatez_f=t.entityelement[t.tentitytoselect].rz;
									if (t.entityprofile[t.gridentity].ismarker == 10)
									{
										t.gridentityscalex_f = 100.0f + t.entityelement[t.tentitytoselect].scalex;
										t.gridentityscaley_f = 100.0f + t.entityelement[t.tentitytoselect].scaley;
										t.gridentityscalez_f = 100.0f + t.entityelement[t.tentitytoselect].scalez;
									}
									else
									{
										t.gridentityscalex_f = ObjectScaleX(t.entityelement[t.tentitytoselect].obj);
										t.gridentityscaley_f = ObjectScaleY(t.entityelement[t.tentitytoselect].obj);
										t.gridentityscalez_f = ObjectScaleZ(t.entityelement[t.tentitytoselect].obj);
									}
									t.grideleprof=t.entityelement[t.tentitytoselect].eleprof;
									t.grideleproflastname_s=t.grideleprof.name_s;

									//  Transfer any waypoint association
									t.waypointindex=t.entityelement[t.tentitytoselect].eleprof.trigger.waypointzoneindex;
									t.grideleprof.trigger.waypointzoneindex=t.waypointindex;
									t.waypoint[t.waypointindex].linkedtoentityindex=0;

									//  delete from map (checks grideleprof.trigger.waypointzoneindex too)
									gridedit_deleteentityfrommap ( );
									t.refreshgrideditcursor=1;

									// remove entity index from rubber band selection
									for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
										if ( g.entityrubberbandlist[i].e == t.tentitytoselect )
											g.entityrubberbandlist[i].e = 0;

									//  Ensure grab GetPoint (  does not move entity! )
									//t.inputsys.dragoffsetx_f=t.entityelement[t.tentitytoselect].x-t.inputsys.localx_f;
									//t.inputsys.dragoffsety_f=t.entityelement[t.tentitytoselect].z-t.inputsys.localy_f;
									t.inputsys.dragoffsetx_f=t.entityelement[t.tentitytoselect].x-t.inputsys.originallocalx_f;
									t.inputsys.dragoffsety_f=t.entityelement[t.tentitytoselect].z-t.inputsys.originallocaly_f;
								}
							}
						}
						t.selstage=1;
					}

					//  If EXTRACT button clicked initially, first reposition mouse before operation and to get mouse coord correct
					if ( t.widget.duplicatebuttonselected == 1 && t.gridentity == 0 ) 
					{
						//  work out screen/mouse position from real-world XZ coordinate
						if (  t.inputsys.picksystemused == 0 ) 
						{
							//  only if not using pick stsrem (no Floor (  to target) )
							SetCurrentCamera ( 0 );

							//  fix camera range for correct projection matrix
							SetCameraRange (  1,300000 );
							t.screenwidth_f=800.0;
							t.screenheight_f=600.0;
							GetProjectionMatrix (  g.m4_projection );
							GetViewMatrix (  g.m4_view );

							// works in DX9 (D3DXVec4Transform) but not DX11 (KMATRIX)
							//SetVector4 (  g.v4_far, ObjectPositionX(t.widget.widgetXYObj), ObjectPositionY(t.widget.widgetXYObj), ObjectPositionZ(t.widget.widgetXYObj), 1 );
							//TransformVector4 (  g.v4_far,g.v4_far,g.m4_view );
							//t.tx_f=GetXVector4(g.v4_far);
							//t.ty_f=GetYVector4(g.v4_far);
							SetVector3 ( g.v3_far, ObjectPositionX(t.widget.widgetXYObj), ObjectPositionY(t.widget.widgetXYObj), ObjectPositionZ(t.widget.widgetXYObj) );
							TransformVectorCoordinates3 ( g.v3_far, g.v3_far, g.m4_view );
							t.tx_f=GetXVector3(g.v3_far);
							t.ty_f=GetYVector3(g.v3_far);

							SetVector4 ( g.v4_far, GetXVector3(g.v3_far), GetYVector3(g.v3_far), GetZVector3(g.v3_far), 1 );
							TransformVector4 ( g.v4_far,g.v4_far,g.m4_projection );
							t.tx_f=GetXVector4(g.v4_far);
							t.ty_f=GetYVector4(g.v4_far);
							t.tx_f=t.tx_f/GetWVector4(g.v4_far);
							t.ty_f=t.ty_f/GetWVector4(g.v4_far);

							t.tadjustedtoareax_f=(((t.tx_f+1.0)/2.0)*(GetDisplayWidth()+0.0));
							t.tadjustedtoareay_f=((((t.ty_f*-1)+1.0)/2.0)*(GetDisplayHeight()+0.0));
							t.inputsys.xmouse=t.tadjustedtoareax_f;
							t.inputsys.ymouse=t.tadjustedtoareay_f;
							t.tideframestartx=148 ; t.tideframestarty=96;

							#ifdef ENABLEIMGUI
							//PE: Why do we need this ? everything should already match.
							//PE: IMGUI need fix.
							//PositionMouse (  (t.tideframestartx+t.inputsys.xmouse)*-1,(t.tideframestarty+t.inputsys.ymouse)*-1 );
							#else
							PositionMouse (  (t.tideframestartx+t.inputsys.xmouse)*-1,(t.tideframestarty+t.inputsys.ymouse)*-1 );
							#endif

							editor_refreshcamerarange ( );
						}

						// trigger actual extraction on next cycle
						t.widget.duplicatebuttonselected = 2;
					}
				}

				//  delete selected entity via delete key
				bool bNoDelete = false;
				static bool bWaitOnDelRelease = false;

				if ( !bNoDelete && t.onetimeentitypickup == 0 )
				{
					if ( t.gridentity != 0 ) 
					{
						if ( t.inputsys.kscancode == 211 ) 
						{
							t.inputsys.mclickreleasestate=1;
							t.gridentitydelete=1;
							t.selstage=1;
							t.inputsys.kscancode = 0;
							t.widget.pickedObject = 0; //dont remove widget object.
							bWaitOnDelRelease = true;
						}
					}
					else
					{
						if ( t.inputsys.kscancode == 211 || (t.widget.deletebuttonselected == 1 && t.inputsys.mclick == 0) ) 
						{
							t.widget.deletebuttonselected=0;
							bool bContinueWithDelete = true;
							if ( t.widget.pickedEntityIndex > 0 )
							{
								// specifically avoid deleting child entities if highlighting a parent
								if ( g.entityrubberbandlist.size() > 0 )
								{
									bool bDisableRubberBandMoving = false;
									if (!bDisableRubberBandMoving)
									{
										//LB: to ensure cannot delete objects that are part of a group, 
										// check if the group is a parent group (user can delete child groups okay)
										{
											// delete all entities in rubber band highlight list
											gridedit_deleteentityrubberbandfrommap();
											gridedit_clearentityrubberbandlist();
											t.widget.pickedEntityIndex = 0;
										}
									}
								}
							}
							if (bContinueWithDelete == true)
							{
								if (t.widget.pickedObject > 0)
								{
									// delete a single entity selected by widget
									if (t.widget.pickedEntityIndex > 0)
									{
										t.tentitytoselect = t.widget.pickedEntityIndex;
										gridedit_deleteentityfrommap ();
									}
									t.widget.pickedObject = 0;
									widget_updatewidgetobject ();
								}
								bWaitOnDelRelease = true;
								t.tentitytoselect = 0;
							}
						}
						if ( t.inputsys.keyspace == 0 ) t.inputsys.spacekeynotreleased = 0;
						if ( t.inputsys.keyspace == 1 && t.inputsys.rubberbandmode == 0 && t.inputsys.spacekeynotreleased == 0 )
						{
							// end selection when press SPACE
							gridedit_clearentityrubberbandlist();
							t.widget.pickedEntityIndex = 0;
							if (  t.widget.pickedObject>0 ) 
							{
								t.widget.pickedObject=0;
								widget_updatewidgetobject ( );
							}
							t.tentitytoselect=0;
						}
					}
				}
				else
				{
					if (  t.inputsys.mclick == 0  )  t.onetimeentitypickup = 0;
				}

				bool bDisableWidgetSelection = false;
				// Select widget controlled object
				if (!bDisableWidgetSelection && (t.inputsys.mclick == 1|| iWidgetSelection > 0 ) && t.gridentity == 0)
				{
					if (iWidgetSelection > 0)
					{
						t.tentitytoselect = iWidgetSelection;
					}
					if ( t.tentitytoselect>0 ) 
					{
							iWidgetSelection = 0;
							if (t.widget.pickedObject == 0)
							{
								//PE: respect markers mode only.
								if (t.gridentitymarkersmodeonly == 0 || (t.gridentitymarkersmodeonly == 1 && t.entityprofile[t.entityelement[t.tentitytoselect].bankindex].ismarker != 0))
								{
									t.widget.pickedEntityIndex = t.tentitytoselect;
									t.widget.pickedObject = t.entityelement[t.tentitytoselect].obj;
									#ifndef ALLOWSELECTINGLOCKEDOBJECTS
									t.entityelement[t.widget.pickedEntityIndex].editorlock = 0;
									#endif
									t.widget.offsetx = g.glastpickedx_f - ObjectPositionX(t.widget.pickedObject);
									t.widget.offsety = g.glastpickedy_f - ObjectPositionY(t.widget.pickedObject);
									t.widget.offsetz = g.glastpickedz_f - ObjectPositionZ(t.widget.pickedObject);

									// 271015 - this may not be required as it is duplicated later on..
									if (g.entityrubberbandlist.size() > 0)
										gridedit_moveentityrubberband();
									else
									{
										entity_recordbuffer_move();
									}
								}
							}
					}
				}

				// zoom into entity properties (or EBE EDIT)
				if ( t.widget.propertybuttonselected == 1 ) 
				{
					t.widget.propertybuttonselected = 0;
					if ( t.widget.pickedEntityIndex > 0 ) 
					{
						int entid = t.entityelement[t.widget.pickedEntityIndex].bankindex;
						if ( t.entityprofile[entid].isebe != 0 )
						{
							
							// EBE entity - begin editing this site
							ebe_newsite ( t.widget.pickedEntityIndex );
						}

						//  End widget control of this object
						t.widget.pickedObject=0;
					}
				}
				if (  t.widget.propertybuttonselected == 2 ) 
				{
					// Entity properties or EBE Save
					t.widget.propertybuttonselected = 0;
					if ( t.widget.pickedEntityIndex > 0 ) 
					{
						int entid = t.entityelement[t.widget.pickedEntityIndex].bankindex;
						if ( t.entityprofile[entid].isebe != 0 )
						{
							// EBE entity - begin editing this site
							if ( ebe_save ( t.widget.pickedEntityIndex ) == 1 )
							{
								// Added NEW (not overwritten) - now saved to entitybank\user\ebestructures
								//editor_addEBEtoLibrary ( entid );
							}
				
							// and close widget as Save bit big deal
							widget_switchoff();
						}
						else
						{
							// regular entity
							// prepare zoom-in adjustment vars
							t.tentitytoselect=t.widget.pickedEntityIndex;
							entity_recordbuffer_move ( );
							t.e=t.tentitytoselect;

							t.gridentityinzoomview=t.e;
							t.zoomviewtargetx_f=t.entityelement[t.e].x;
							t.zoomviewtargety_f=t.entityelement[t.e].y;
							t.zoomviewtargetz_f=t.entityelement[t.e].z;
							t.zoomviewtargetrx_f=t.entityelement[t.e].rx;
							t.zoomviewtargetry_f=t.entityelement[t.e].ry;
							t.zoomviewtargetrz_f=t.entityelement[t.e].rz;
							gridedit_updatezoomviewvalues ( );

							//  extract entity from the map
							t.gridentityeditorfixed=t.entityelement[t.e].editorfixed;
							t.gridentity=t.entityelement[t.e].bankindex;
							t.ttrygridentitystaticmode=t.entityelement[t.e].staticflag;
							t.ttrygridentity=t.gridentity ; editor_validatestaticmode ( );
							t.gridentityautofind=0;
							t.gridentityposoffground=1;
							t.gridentityusingsoftauto=0;
							t.gridentitysurfacesnap=1-g.gdisablesurfacesnap;
							t.gridentityhasparent=t.entityelement[t.e].iHasParentIndex;
							t.gridentityposx_f=t.entityelement[t.e].x;
							t.gridentityposy_f=t.entityelement[t.e].y;
							t.gridentityposz_f=t.entityelement[t.e].z;
							t.gridentityrotatex_f=t.entityelement[t.e].rx;
							t.gridentityrotatey_f=t.entityelement[t.e].ry;
							t.gridentityrotatez_f=t.entityelement[t.e].rz;
							if (t.entityprofile[t.gridentity].ismarker == 10)
							{
								t.gridentityscalex_f = 100.0f + t.entityelement[t.e].scalex;
								t.gridentityscaley_f = 100.0f + t.entityelement[t.e].scaley;
								t.gridentityscalez_f = 100.0f + t.entityelement[t.e].scalez;
							}
							else
							{
								t.gridentityscalex_f = ObjectScaleX(t.entityelement[t.e].obj);
								t.gridentityscaley_f = ObjectScaleY(t.entityelement[t.e].obj);
								t.gridentityscalez_f = ObjectScaleZ(t.entityelement[t.e].obj);
							}
							t.grideleprof=t.entityelement[t.e].eleprof;

							//  Transfer any waypoint association
							t.waypointindex=t.entityelement[t.e].eleprof.trigger.waypointzoneindex;
							t.grideleprof.trigger.waypointzoneindex=t.waypointindex;
							t.waypoint[t.waypointindex].linkedtoentityindex=0;

							//  Delete entity from map
							gridedit_deleteentityfrommap ( );
							t.refreshgrideditcursor=1;

							//  simply use its current position (no offset)
							t.inputsys.dragoffsetx_f=0;
							t.inputsys.dragoffsety_f=0;

							//  zoom in to entity for fine detail
							t.cx_f=t.entityelement[t.e].x;
							t.cy_f=t.entityelement[t.e].z*-1.0;
							t.inputsys.doautozoomview=1;
							if (t.zoomviewcamerarange_f > 2000.0f)
							{
								//This can fail after test game. if really large set defaults.
								t.zoomviewcamerarange_f = 175.0f;
								t.zoomviewcameraheight_f = 150.0f;
							}
							//  disable icons that interfere with zoom mode
							editor_disableforzoom ( );
	
							HideObject ( t.editor.objectstartindex+5 );
							t.selstage=1;

							//  prepare entity property handler
							#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
							//PE: Just open the window..
							bEntity_Properties_Window = true;
							#else
							interface_openpropertywindow ( );
							#endif

							//  End widget control of this object
							t.widget.pickedObject=0;
						}
					}
				}

				bool bDisableRubberBandMoving = false;
				// update rubberband selection connected to primary cursor entity
				if ( !bDisableRubberBandMoving && t.gridentity > 0 && g.entityrubberbandlist.size() > 1 && t.fOldGridEntityX > -99999.0f )
				{
					float fMovedActiveObjectX = t.gridentityposx_f - t.fOldGridEntityX;
					float fMovedActiveObjectY = t.gridentityposy_f - t.fOldGridEntityY;
					float fMovedActiveObjectZ = t.gridentityposz_f - t.fOldGridEntityZ;
					float fMovedActiveObjectRX = t.gridentityrotatex_f - t.fOldGridEntityRX;
					float fMovedActiveObjectRY = t.gridentityrotatey_f - t.fOldGridEntityRY;
					float fMovedActiveObjectRZ = t.gridentityrotatez_f - t.fOldGridEntityRZ;

					bool bDoRotateToo = false;
					if ( fabs(fMovedActiveObjectRX) > 0.1f || fabs(fMovedActiveObjectRY) > 0.1f || fabs(fMovedActiveObjectRZ) > 0.1f ) bDoRotateToo = true;
					t.tobj = t.gridentityobj;
					if ( t.tobj>0 ) 
					{
						// rotate all selected around t.tobj, the active object
						GGQUATERNION QuatAroundX, QuatAroundY, QuatAroundZ;
						GGQuaternionRotationAxis(&QuatAroundX, &GGVECTOR3(1, 0, 0), GGToRadian(fMovedActiveObjectRX));
						GGQuaternionRotationAxis(&QuatAroundY, &GGVECTOR3(0, 1, 0), GGToRadian(fMovedActiveObjectRY));
						GGQuaternionRotationAxis(&QuatAroundZ, &GGVECTOR3(0, 0, 1), GGToRadian(fMovedActiveObjectRZ));
						GGQUATERNION quatRotationEvent = QuatAroundX * QuatAroundY * QuatAroundZ;
						SetStartPositionsForRubberBand(t.tobj);
						RotateAndMoveRubberBand(t.tobj, fMovedActiveObjectX, fMovedActiveObjectY, fMovedActiveObjectZ, quatRotationEvent);
					}
				}
				// record all current offsets from primary cursor entity and rubberband selection
				{
					//PE: t.gridentityposx_f can go below -1, if you drag it way out there , and you could loose moving of rubberband (out of sync).
					if (!bDisableRubberBandMoving)
					{
						t.fOldGridEntityX = -99999.0f;
						t.fOldGridEntityY = -99999.0f;
						t.fOldGridEntityZ = -99999.0f;
						t.fOldGridEntityRX = -99999.0f;
						t.fOldGridEntityRY = -99999.0f;
						t.fOldGridEntityRZ = -99999.0f;
					}
				}
				if (!bDisableRubberBandMoving && t.gridentity > 0 && t.gridentityobj > 0 && g.entityrubberbandlist.size() > 1 )
				{
					t.fOldGridEntityX = t.gridentityposx_f;
					t.fOldGridEntityY = t.gridentityposy_f;
					t.fOldGridEntityZ = t.gridentityposz_f;
					t.fOldGridEntityRX = t.gridentityrotatex_f;
					t.fOldGridEntityRY = t.gridentityrotatey_f;
					t.fOldGridEntityRZ = t.gridentityrotatez_f;

					for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
					{
						int e = g.entityrubberbandlist[i].e;
						GGVECTOR3 VecPos;
						VecPos.x = t.entityelement[e].x - t.gridentityposx_f;
						VecPos.y = t.entityelement[e].y - t.gridentityposy_f;
						VecPos.z = t.entityelement[e].z - t.gridentityposz_f;
						int tobj = t.entityelement[e].obj;
						if ( tobj > 0 )
						{
							float fDet = 0.0f;
							sObject* pObject = GetObjectData(tobj);
							GGMATRIX inverseMatrix = pObject->position.matObjectNoTran;
							GGMatrixInverse ( &inverseMatrix, &fDet, &inverseMatrix );
							GGVec3TransformCoord ( &VecPos, &VecPos, &inverseMatrix );
							g.entityrubberbandlist[i].x = VecPos.x;
							g.entityrubberbandlist[i].y = VecPos.y;
							g.entityrubberbandlist[i].z = VecPos.z;
						}
					}
				}

				//  gridentity delete
				if (  t.gridentitydelete == 1 ) 
				{
					//  Delete any associated waypoint/trigger zone
					t.waypointindex=t.grideleprof.trigger.waypointzoneindex;
					if (  t.waypointindex>0 ) 
					{
						t.w=t.waypoint[t.waypointindex].start;
						waypoint_delete ( );
					}
					t.grideleprof.trigger.waypointzoneindex=0;
					//  delete grid entity object and reset
					t.gridentitydelete=0;
					if (  t.gridentityobj>0 ) 
					{
						DeleteObject (  t.gridentityobj );
						t.gridentityobj=0;
					}
					t.refreshgrideditcursor=1;
					t.gridentity=0;
					t.gridentityposoffground=0;
					t.gridentityusingsoftauto=0;
					t.gridentitysurfacesnap=1-g.gdisablesurfacesnap;
					t.gridentityautofind=1;
					t.inputsys.dragoffsetx_f=0;
					t.inputsys.dragoffsety_f=0;
					editor_refreshentitycursor ( );
					t.widget.pickedObject=0;

					{
						bool bDisableRubberBandMoving = false;
						if (!bDisableRubberBandMoving)
						{
							// if rubberband selection, delete all in selection
							gridedit_deleteentityrubberbandfrommap();
						}
					}

					// flag also used to restore highlighting behavior
					t.gridentityextractedindex = 0;

					// when place down, ensure waypoint not affected until release mouse button
					t.mclickpressed = 1;
				}
			}

			if (  t.inputsys.mclick == 0 && t.selstage == 1 ) 
			{
				t.selstage=0;
			}
			if (  t.gridedit.entityspraymode == 1 && t.selstage == 1 ) 
			{
				//  entity spray keeps going while button pressed
				t.selstage=0;
			}
		}

		// this is triggered when set to a negative, and continues to force find surface until zero

	}
}

void gridedit_updatezoomviewvalues ( void )
{
	//  accepts gridentityinzoomview
	if (  t.gridentityinzoomview>0 ) 
	{
		t.zoomviewcameraangle_f=0.0;
		t.zoomviewcameraheight_f=50.0;
		t.zoomviewcamerarange_f=75.0;
		if (  t.entityelement[t.gridentityinzoomview].obj>0 ) 
		{
			if (  ObjectExist(t.entityelement[t.gridentityinzoomview].obj) == 1 ) 
			{
				t.zoomviewcamerarange_f=ObjectSize(t.entityelement[t.gridentityinzoomview].obj,1)*2.0;
				t.zoomviewcameraheight_f=(ObjectSize(t.entityelement[t.gridentityinzoomview].obj,1)/2.0)-100.0;
				if (  t.zoomviewcameraheight_f<5  )  t.zoomviewcameraheight_f = 5;

				// ensure camera always faces the front of an entity
				t.zoomviewcameraangle_f = (0-ObjectAngleY(t.entityelement[t.gridentityinzoomview].obj))+180.0f;
			}
		}
	}
}

void gridedit_save_test_map ( void )
{
	//  Save map data locally only (not to FPM)
	timestampactivity(0,"SAVETESTMAP: Save map");
	mapfile_savemap ( );

	//  Settings specific to the player
	timestampactivity(0,"SAVETESTMAP: Save player config");
	mapfile_saveplayerconfig ( );

	//  Save entity elements
	timestampactivity(0,"SAVETESTMAP: Save elements");
	entity_savebank ( );
	entity_savebank_ebe ( );
	entity_saveelementsdata ( );

	//  Save waypoints
	timestampactivity(0,"SAVETESTMAP: Save waypoints");
	waypoint_savedata ( );

	//  Save editor configuration
	timestampactivity(0,"SAVETESTMAP: Save config");
	editor_savecfg ( );

	//  Save terrain
	timestampactivity(0,"SAVETESTMAP: Save terrain textures");
	#ifdef VRTECH
	t.tfileveg_s = g.mysystem.levelBankTestMap_s + "vegmask.png";// dds";
	t.tfilewater_s = g.mysystem.levelBankTestMap_s + "watermask.png";// dds"; 
	#else
	t.tfileveg_s=g.mysystem.levelBankTestMap_s+"vegmask.dds"; //"levelbank\\testmap\\vegmask.dds";
	t.tfilewater_s=g.mysystem.levelBankTestMap_s+"watermask.dds"; //"levelbank\\testmap\\watermask.dds";
	#endif
	terrain_savetextures ( );
	t.tfileveggrass_s=g.mysystem.levelBankTestMap_s+"vegmaskgrass.dat";
	timestampactivity(0,"SAVETESTMAP: Save terrain veg");
	grass_savegrass ( );
	timestampactivity(0,"SAVETESTMAP: Save terrain height data");
	t.tfile_s=g.mysystem.levelBankTestMap_s+"m.dat";
	terrain_save();

	//  this ensures change flag does not use filemap port 1 (avoid freeze in build game)
	t.lastprojectmodified=0;

	//  Set modification flag
	timestampactivity(0,"SAVETESTMAP: Change modified flag of level");
	g.projectmodified = 0; gridedit_changemodifiedflag ( );
	g.projectmodifiedstatic = 0; 
	timestampactivity(0,"SAVETESTMAP: Complete");
}

void gridedit_save_map ( void )
{

	// Proper saving message to user
	if (  t.recoverdonotuseany3dreferences == 0 ) 
	{
		editor_hideall3d ( );
	}

	// Use large prompt
	t.statusbar_s=t.strarr_s[365]; 
	popup_text(t.statusbar_s.Get());

	// Save only to TESTMAP area (for map testing)
	gridedit_save_test_map ( );

	// Now store all part-files into main FPM project
	mapfile_saveproject_fpm ( );

	// Add Latest project To Recent List
	gridedit_updateprojectname ( );

	// Clear status Text (  )
	t.statusbar_s="" ; popup_text_close();

	// refresh as SAVE can remove entities and segments
	if ( t.entityorsegmententrieschanged == 1 ) 
	{
		// 111115 - and if not exiting GG
		if ( g.savenoreloadflag == 0 )
		{
			gridedit_load_map ( );
		}
		t.entityorsegmententrieschanged=0;
	}
}

void gridedit_updatemapbeforeedit ( void )
{

//  Completely reset filemap (and interface parts ie library)
editor_filemapinit ( );

//  Newly loaded map starts at layer X
t.gridlayer=5 ; t.refreshgrideditcursor=1;

return;

}

void gridedit_clear_settings ( void )
{

//  Default settings
gridedit_clear_configsettings ( );
t.gridscale_f=((800/2)/8)/t.gridzoom_f;
t.currentprojectfilename_s="";
t.gridground=0;
t.gridselection=1;
t.bufferlayer=-1;
g.gridlayershowsingle=0;
t.grideditartwidth=1;
t.grideditartwidthx=1;
t.grideditartwidthy=1;
t.locallibrarysegidmaster=0;
t.locallibraryentidmaster=0;
}

void gridedit_clear_configsettings ( void )
{
	//  defaults
	t.borderx_f=1024.0*50.0 ; t.cx_f=t.borderx_f/2.0;
	t.bordery_f=1024.0*50.0 ; t.cy_f=t.bordery_f/2.0;

	//  Default zoom
	t.gridzoom_f=3.0 ; t.clipheight_f=655;

	//  default grideditselect
	//  0=terrain mode
	//  4=zoom mode
	//  5=entity editing
	//  6=waypoint mode
	t.grideditselect=0;
}

void gridedit_clear_map ( void )
{
	//  Delete ALL light map objects
	lm_deleteall ( );


	//  Delete any old entity objects
	gridedit_deletelevelobjects ( );

	//  Delete any old weapon objects (and reload original data in case last level edited them)
	gun_releaseresources ( );
	gun_scaninall_dataonly ( );

	//  Remove any shader lighting
	lighting_free ( );

	//  delete any conkit objects
	///conkit_saveload_clear ( );

	//  Ensure whether New or Load, physics tweakables for player are reset
	physics_inittweakables ( );

	//  Set modification flag
	g.projectmodified = 0 ; gridedit_changemodifiedflag ( );
	g.projectmodifiedstatic = 0;

	//  ensure no leftovers from last edit session
	t.tlasttentitytoselect=-1;

	//  Must generate super texture when do test level for this new level map
	t.terrain.generatedsupertexture=0;

	//  reset free flight mode
	t.editorfreeflight.mode=0 ; t.updatezoom=1;
	t.editorfreeflight.sused=0;
	t.gridentityhidemarkers=0;
	t.cameraviewmode=0;
}

void gridedit_resetmemortracker ( void )
{
	// 121115 - good place to reset memory tracking
	int iMemoryLostFromActivitySoFar = g.gamememactuallyusedstart - SMEMAvailable(1);
	g.gamememactualmaxrightnow = g.gamememactualmaxrightnow - iMemoryLostFromActivitySoFar;
	g.gamememactuallyusedstart = SMEMAvailable(1);
}

#ifdef VRTECH
void gridedit_emptyallcustomfiles ( void )
{
	ChecklistForFiles();
	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		t.tfile_s = ChecklistString(t.c);
		if ( t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			// only if a CUSTOM file - needs clearing when new level created
			if ( strnicmp ( t.tfile_s.Get(), "CUSTOM_", 7 ) == NULL )
			{
				DeleteAFile ( t.tfile_s.Get() );
			}
		}
	}
}
#endif


void gridedit_new_map ( void )
{

	// ensure tab mode vars reset (no carry from previous session)
	g.tabmode = 0; //TABTAB mode
	g.tabmodeshowfps = 0; //F11 mode
	g.tabmodehidehuds = 0;
	g.mouseishidden = 0;
	t.terrain.terrainpaintermode = 1;

	// reset weather display flag
	#ifdef VRTECH
	bEnableWeather = false;
	#endif

	//  Start time profiling
	timestampactivity(0,"NEWMAP: Starting new map");

	//  No project - new map
	g.projectfilename_s=""; 
	g.projectmodified=0; t.lastprojectmodified=0;
	g.projectmodifiedstatic = 0; 
	gridedit_updateprojectname ( );

	// hide EBE if starting new map
	ebe_hide();
	ebe_hardreset();

	// hide terrain texture panel
	terrain_paintselector_hide(); 

	#ifdef VRTECH
	//if ( gbWelcomeSystemActive == false ) 
	Sync();
	#else
	if ( gbWelcomeSystemActive == false ) Sync();
	#endif

	//  Reset visual settings for new map
	visuals_newlevel ( );

	// Reset all water settings.
	visuals_water_reset();

	//  Ensure default terrain and veg graphics
	terrain_changestyle ( );
	g.vegstyleindex=t.visuals.vegetationindex;
	grass_changevegstyle ( );

	//  Load map data
	editor_hideall3d ( );
	t.statusbar_s=t.strarr_s[366] ; gridedit_updatestatusbar ( );

	//  Clear all settings
	timestampactivity(0,"NEWMAP: _gridedit_clear_settings");
	gridedit_clear_settings ( );

	//  Empty the lightmap folder
	timestampactivity(0,"NEWMAP: _lm_emptylightmapandttsfilesfolder");
	lm_emptylightmapandttsfilesfolder ( );

	#ifdef VRTECH
	// Reset texture/profile in EBE folder
	ebe_restoreebedefaulttextures();
	#endif

	// Empty EBEs from testmap folder
	cstr pStoreOld = GetDir(); 
	if ( PathExist ( g.mysystem.levelBankTestMap_s.Get() ) == 0 )
	{
		// somehow levelbank\testmap folder gone (can be deleted sometimes)
		SetDir ( cstr(g.fpscrootdir_s + "\\Files\\").Get() );
		if ( PathExist ( "levelbank" ) == 0 )
		{
			MakeDirectory ( "levelbank" );
			SetDir ( "levelbank" );
		}
		if ( PathExist ( "testmap" ) == 0 )
		{
			MakeDirectory ( "testmap" );
			SetDir ( "testmap" );
		}
	}
	else
		SetDir ( g.mysystem.levelBankTestMap_s.Get() );

	// Delete any EBE files for new levels
	timestampactivity(0,"NEWMAP: mapfile_emptyebesfromtestmapfolder");
	mapfile_emptyebesfromtestmapfolder(false);

	#ifdef VRTECH
	// Delete any CUSTOM files for new levels, otherwise messes up new asset addition work
	timestampactivity(0,"NEWMAP: gridedit_emptyallcustomfiles");
	gridedit_emptyallcustomfiles ( );
	#endif

	// restore folder to default 
	SetDir ( pStoreOld.Get() );

	// Empty terraintexture files from testmap folder
	SetDir ( g.mysystem.levelBankTestMap_s.Get() );
	if ( FileExist ( "superpalette.ter" ) == 1 ) DeleteFileA ( "superpalette.ter" );
	if ( FileExist ( "Texture_D.dds" ) == 1 ) DeleteFileA ( "Texture_D.dds" );
	if ( FileExist ( "Texture_D.jpg" ) == 1 ) DeleteFileA ( "Texture_D.jpg" );
	if ( FileExist ( "Texture_N.dds" ) == 1 ) DeleteFileA ( "Texture_N.dds" );
	if ( FileExist ( "Texture_N.jpg" ) == 1 ) DeleteFileA ( "Texture_N.jpg" );
	if ( FileExist ( "globalenvmap.dds" ) == 1 ) DeleteFileA ( "globalenvmap.dds" );
	SetDir ( pStoreOld.Get() );

	// ensures new terrain in new map is loaded into terrain texture panel when shown
	terrain_resetfornewlevel();

	//  Ensure no old OBS file and OBS triggers to generate
	timestampactivity(0,"NEWMAP: invalidate any old OBS");
	darkai_invalidateobstacles ( );

	//  Clear map first
	timestampactivity(0,"NEWMAP: _gridedit_clear_map");
	gridedit_clear_map ( );

	// 121115 - Reset memory tracker
	gridedit_resetmemortracker ( );

	//  Delete all assets of map work
	timestampactivity(0,"NEWMAP: _waypoint_deleteall");
	waypoint_deleteall ( );
	mapfile_newmap ( );

	//  Update remaining map data before editing
	timestampactivity(0,"NEWMAP: _gridedit_updatemapbeforeedit");
	gridedit_updatemapbeforeedit ( );

	//  Some default setup for new scene (load markers)
	timestampactivity(0,"NEWMAP: _editor_filemapdefaultinitfornew");
	editor_filemapdefaultinitfornew ( );

	//  Recreate terrain to remove links to old LOD1 objects
	timestampactivity(0,"NEWMAP: _terrain_createactualterrain");
	terrain_createactualterrain ( );

	//  Randomise/Flatten terrain when NEW level created
	if (  t.inputsys.donewflat == 1 ) 
	{
		timestampactivity(0,"NEWMAP: Save newly flattened terrain");
		terrain_flattenterrain ( );
	}
	else
	{
		timestampactivity(0,"NEWMAP: Save newly randomised terrain");
		terrain_randomiseterrain ( );
	}
	t.tfile_s=g.mysystem.levelBankTestMap_s+"m.dat";
	terrain_save();

	timestampactivity(0,"NEWMAP: Save terrain data");
	#ifdef VRTECH
	t.tfileveg_s = g.mysystem.levelBankTestMap_s + "vegmask.png";// dds";
	t.tfilewater_s = g.mysystem.levelBankTestMap_s + "watermask.png";// dds";
	#else
	t.tfileveg_s=g.mysystem.levelBankTestMap_s+"vegmask.dds"; //"levelbank\\testmap\\vegmask.dds";
	t.tfilewater_s=g.mysystem.levelBankTestMap_s+"watermask.dds"; //"levelbank\\testmap\\watermask.dds";
	#endif
	t.tgeneratefreshwatermaskflag=1;
	terrain_generatevegandmaskfromterrain ( );
	timestampactivity(0,"NEWMAP: Save terrain mask data");
	t.tfileveggrass_s=g.mysystem.levelBankTestMap_s+"vegmaskgrass.dat";
	grass_buildblankgrass_fornew ( );

	timestampactivity(0,"NEWMAP: Finish t.terrain generation");
	
	//  Set standard start height for camera
	t.gridzoom_f=3.0 ; t.clipheight_f=655 ; t.updatezoom=1;

	//  Reset cursor
	t. grideditselect = 0 ; editor_refresheditmarkers ( );

	//  Clear status Text (  )
	t.statusbar_s = "" ; gridedit_updatestatusbar ( );

	//  Clear widget status
	t.widget.pickedObject=0 ; widget_updatewidgetobject ( );

	//  Reset UNDO/REDO buffer
	t.entityundo.action=0;
	t.entityundo.entityindex=0;
	t.entityundo.bankindex=0;
	t.entityundo.undoperformed=0;
	t.terrainundo.bufferfilled=0;
	t.terrainundo.mode=0;


	//  Finished new map
	timestampactivity(0,"NEWMAP: Finish creating new map");
}

void gridedit_new_map_quick(void)
{
	// ensure tab mode vars reset (no carry from previous session)
	g.tabmode = 0; //TABTAB mode
	g.tabmodeshowfps = 0; //F11 mode
	g.tabmodehidehuds = 0;
	g.mouseishidden = 0;
	t.terrain.terrainpaintermode = 1;

	//  Start time profiling
	timestampactivity(0, "NEWMAP: Starting new map");

	//  No project - new map
	g.projectfilename_s = "";
	g.projectmodified = 0; t.lastprojectmodified = 0;
	g.projectmodifiedstatic = 0;
	gridedit_updateprojectname();

	// hide EBE if starting new map
	ebe_hide();
	ebe_hardreset();

	// hide terrain texture panel
	terrain_paintselector_hide();

	//  Reset visual settings for new map
	visuals_newlevel();

	// Reset all water settings.
	visuals_water_reset();

	//  Reset visual settings for new map
	t.visuals.refreshshaders = 1;
	
	//  Load map data
	editor_hideall3d();
	t.statusbar_s = t.strarr_s[366]; gridedit_updatestatusbar();

	//  Clear all settings
	timestampactivity(0, "NEWMAP: _gridedit_clear_settings");
	gridedit_clear_settings();

	// Empty EBEs from testmap folder
	cstr pStoreOld = GetDir();
	if (PathExist(g.mysystem.levelBankTestMap_s.Get()) == 0)
	{
		// somehow levelbank\testmap folder gone (can be deleted sometimes)
		SetDir(cstr(g.fpscrootdir_s + "\\Files\\").Get());
		if (PathExist("levelbank") == 0)
		{
			MakeDirectory("levelbank");
			SetDir("levelbank");
		}
		if (PathExist("testmap") == 0)
		{
			MakeDirectory("testmap");
			SetDir("testmap");
		}
	}
	else {
		SetDir(g.mysystem.levelBankTestMap_s.Get());
		//  Empty the lightmap folder
		timestampactivity(0, "NEWMAP: cleantestmapfolder");
		if (FileExist("superpalette.ter") == 1) DeleteFileA("superpalette.ter");
		if (FileExist("Texture_D.dds") == 1) DeleteFileA("Texture_D.dds");
		if (FileExist("Texture_D.jpg") == 1) DeleteFileA("Texture_D.jpg");
		if (FileExist("Texture_N.dds") == 1) DeleteFileA("Texture_N.dds");
		if (FileExist("Texture_N.jpg") == 1) DeleteFileA("Texture_N.jpg");
		if (FileExist("globalenvmap.dds") == 1) DeleteFileA("globalenvmap.dds");
		//  Ensure no old OBS file and OBS triggers to generate
		if (t.tignoreinvalidateobstacles == 0) {
			if (FileExist("map.obs") == 1) DeleteFileA("map.obs");
			t.aisystem.generateobs = 1;
		}

		lm_emptylightmapandttsfilesfolder();
		// Delete any EBE files for new levels
		mapfile_emptyebesfromtestmapfolder(false);
		#ifdef VRTECH
		gridedit_emptyallcustomfiles();
		#endif
	}

	// restore folder to default 
	SetDir(pStoreOld.Get());

	// ensures new terrain in new map is loaded into terrain texture panel when shown
	terrain_resetfornewlevel();

	//  Clear map first
	t.tlasttentitytoselect = -1;
	g.projectmodified = 0; gridedit_changemodifiedflag();
	g.projectmodifiedstatic = 0;


	lighting_free();
	gridedit_deletelevelobjects();

	// 121115 - Reset memory tracker
	gridedit_resetmemortracker();

	//  Delete all assets of map work
	//timestampactivity(0, "NEWMAP: _waypoint_deleteall");
	//waypoint_deleteall();
	mapfile_newmap();

	//  Update remaining map data before editing
	//timestampactivity(0, "NEWMAP: _gridedit_updatemapbeforeedit");
	//gridedit_updatemapbeforeedit();
	if (t.game.gameisexe == 0)
	{
		// for now, it seems the standalone can call this function!!
		editor_clearlibrary();
		g.entidmaster = 0;
		editor_filllibrary();
		editor_leftpanelreset();
		t.gridlayer = 5; t.refreshgrideditcursor = 1;
	}

	//  Recreate terrain to remove links to old LOD1 objects
	timestampactivity(0, "NEWMAP: _terrain_createactualterrain");
	terrain_createactualterrain();

	//  Randomise/Flatten terrain when NEW level created
	if (t.inputsys.donewflat == 1)
	{
		timestampactivity(0, "NEWMAP: Save newly flattened terrain");
		terrain_flattenterrain();
	}
	else
	{
		timestampactivity(0, "NEWMAP: Save newly randomised terrain");
		terrain_randomiseterrain();
	}
	t.tfile_s = g.mysystem.levelBankTestMap_s + "m.dat";
	terrain_save();
	timestampactivity(0, "NEWMAP: Save terrain data");
	t.tfileveg_s = g.mysystem.levelBankTestMap_s + "vegmask.png";// dds";
	t.tfilewater_s = g.mysystem.levelBankTestMap_s + "watermask.png";// dds";
	t.tgeneratefreshwatermaskflag = 1;
	terrain_generatevegandmaskfromterrain();
	t.tfileveggrass_s = g.mysystem.levelBankTestMap_s + "vegmaskgrass.dat";
	grass_buildblankgrass_fornew(); //Delay this ?

	//  Set standard start height for camera
	t.gridzoom_f = 3.0; t.clipheight_f = 655; t.updatezoom = 1;

	//  Reset cursor
	t.grideditselect = 0; editor_refresheditmarkers();

	//  Clear status Text (  )
	t.statusbar_s = ""; gridedit_updatestatusbar();

	//  Clear widget status
	t.widget.pickedObject = 0; widget_updatewidgetobject();

	//  Reset UNDO/REDO buffer
	t.entityundo.action = 0;
	t.entityundo.entityindex = 0;
	t.entityundo.bankindex = 0;
	t.entityundo.undoperformed = 0;
	t.terrainundo.bufferfilled = 0;
	t.terrainundo.mode = 0;


	//  Finished new map
	timestampactivity(0, "NEWMAP: Finish creating new map");
}

void gridedit_updatestatusbar ( void )
{
	//  020315 - 012 - display in the status bar if multiplayer lobbies are currently available
	mp_checkIfLobbiesAvailable ( );
	if (  t.statusbar_s+t.steamStatusBar_s != t.laststatusbar_s.Get() ) 
	{
		t.strwork = ""; t.strwork = t.strwork + t.statusbar_s + t.steamStatusBar_s;
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		SetFileMapString (  1, 4000, t.strwork.Get() );
		SetEventAndWait (  1 );
		#endif
		t.laststatusbar_s=t.statusbar_s+t.steamStatusBar_s;
	}
}

void gridedit_load_map ( void )
{
	//  Load map data
	editor_hideall3d ( );

	// hide terrain texture panel
	terrain_paintselector_hide(); Sync();

	//  Use large prompt
	t.statusbar_s=t.strarr_s[367]; 
	popup_text(t.statusbar_s.Get());

	//  Reset visual settings for new map
	if (  t.skipfpmloading == 0 ) 
	{
		// 131115 - prevent visual settings for game get wiped out if restart session
		// where project loaded from levelbank\testlevel and visuals already filled
		visuals_newlevel ( );
	}

	//  Force the zoom to be updated to prevent black screen bug, due to old camera range
	t.updatezoom=1;

	//  Load FPM project into testmap files area
	t.tloadsuccessfully=1;
	if (  t.skipfpmloading == 1 ) 
	{
		//  replace NEW with RELOAD
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		SetFileMapDWORD (  1, 408, 0 );
		SetEventAndWait (  1 );
		#endif
	}
	else
	{
		//  this setstloadsuccessfully to zero if failed to load FPM (corrupt zipfile)
		mapfile_loadproject_fpm ( );
	}

	//  Loaded successfully
	if ( t.tloadsuccessfully == 1 ) 
	{
		//  Clear map first
		gridedit_clear_map ( );

		// 121115 - Reset memory tracker
		gridedit_resetmemortracker ( );

		//  Determine if FPM is accompanied by .REPLACE file
		t.treplacefilename_s = "" ; t.treplacefilename_s = t.treplacefilename_s + Left(g.projectfilename_s.Get(),Len(g.projectfilename_s.Get())-4)+".replace";
		if (  FileExist(t.treplacefilename_s.Get()) == 1 ) 
		{
			t.editor.replacefilepresent_s=t.treplacefilename_s;
		}
		else
		{
			t.editor.replacefilepresent_s="";
		}

		//  Load entity bank and elements
		popup_text_change(t.strarr_s[611].Get());
		entity_loadbank ( );
		entity_loadelementsdata ( );
		t.editor.replacefilepresent_s="";

		//  Load waypoints
		popup_text_change(t.strarr_s[612].Get());
		waypoint_loaddata ( );
		waypoint_recreateobjs ( );

		//  Load data
		popup_text_change(t.strarr_s[613].Get());
		mapfile_loadmap ( );

		//  Load player settings
		timestampactivity(0,"Load player config");
		mapfile_loadplayerconfig ( );

		//  Load terrain
		popup_text_change(t.strarr_s[610].Get());
		timestampactivity(0, "Create Terrain");
		terrain_createactualterrain ( );
		terrain_loaddata ( );

		//  Update remaining map data before editing
		timestampactivity(0, "Reset Editor.");

		gridedit_updatemapbeforeedit ( );

		//  Load editor configuration
		
		int iOldGE = t.grideditselect;
		editor_loadcfg ( );

		//  Load segments/prefab/entities into window
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		#endif
		editor_filllibrary ( );

		//  Add Latest project To Recent List
		gridedit_updateprojectname ( );
	}
	else
	{
		//  FPM could not be extracted (likely a corrupt zipfile)
		if (  t.tloadsuccessfully == 0 ) 
		{
			t.strwork = ""; t.strwork = t.strwork + t.strarr_s[614]+" : "+Right(g.projectfilename_s.Get(),Len(g.projectfilename_s.Get())-Len(g.fpscrootdir_s.Get()));
			popup_text_change( t.strwork.Get() );
		}
		if (  t.tloadsuccessfully == 2 ) 
		{
			popup_text_change("The FPM was not created with Game Guru");
		}
		SleepNow (  2000 );

		//  Create blank in this case
		t.inputsys.donewflat=1;
		gridedit_new_map ( );
	}

	//  Popup warning if load found some missing files
	if ( g.timestampactivityflagged == 1 ) 
	{
		//  message prompt
		t.statusbar_s=t.strarr_s[368];
		popup_text_change(t.statusbar_s.Get()) ; SleepNow (  2000 );
		g.timestampactivityflagged=0;

		//  copy time stamp log to map bank log
		if (  ArrayCount(t.missingmedia_s) >= 0 ) 
		{
			t.tmblogfile_s = "" ; t.tmblogfile_s=t.tmblogfile_s + Left(g.projectfilename_s.Get(),Len(g.projectfilename_s.Get())-4)+".log";
			if (  FileExist(t.tmblogfile_s.Get()) == 1  )  DeleteAFile (  t.tmblogfile_s.Get() );
			if (  Len(t.tmblogfile_s.Get())>4 ) 
			{
				t.missingmedia_s[0]="MISSING MEDIA:";
				for ( t.m = 1 ; t.m <= ArrayCount(t.missingmedia_s); t.m++ )
				{
					if (  Len(t.missingmedia_s[t.m].Get())>2 ) 
					{
						t.missingmedia_s[t.m]=t.missingmedia_s[t.m]+"=replace"+t.missingmedia_s[t.m];
					}
				}
				SaveArray (  t.tmblogfile_s.Get() ,t.missingmedia_s );
			}
		}
	}
	else
	{
		// if no missing media, is parental control system removing some?
		if ( g_bBlackListRemovedSomeEntities == true ) 
		{
			t.statusbar_s = "Parental Control system has removed some content from this level";
			popup_text_change(t.statusbar_s.Get()) ; SleepNow ( 3000 );
		}
	}

	// free usages
	if (  ArrayCount(t.missingmedia_s) >= 0 ) 
	{
		UnDim (  t.missingmedia_s );
	}
	g.missingmediacounter=0;

	//  Clear status Text (  )
	t.statusbar_s="" ; popup_text_close();

	//  Quick update of cursor
	t.lastgrideditselect=-1 ; editor_refresheditmarkers ( );

	//  Recreate all entities in level
	for ( t.e = 1 ; t.e <=  g.entityelementlist; t.e++ )
	{
		t.tupdatee=t.e ; gridedit_updateentityobj ( );
	}
	lighting_refresh ( );

	//  Ensure newly updated entity does not trigger a terrain update!
	t.terrain.terrainpainteroneshot=0;

	//  Ensure visual indices for sky, terrain and veg up to date (for when we use test game)
	visuals_updateskyterrainvegindex ( );

	//  Refresh any 'shaders' that associat with new entities loaded in
	visuals_justshaderupdate ( );

	//  Generate heightmap texture for cheap shadows (if required)
	//t.terrain.terraintriggercheapshadowrefresh=2;

	//  Ensure editor zoom refreshes
	t.updatezoom=1;

	// 161115 - in any event, ensure we generate super texture for 'distant' terrain texture 
	t.visuals.refreshterrainsupertexture = 2;

}

void gridedit_changemodifiedflag ( void )
{
	// project flag changed, update window Text (  )
	if ( t.game.gameisexe == 0 ) 
	{
		if ( t.lastprojectmodified != g.projectmodified ) 
		{
			t.lastprojectmodified=g.projectmodified;
			gridedit_updateprojectname ( );
		}
		if ( g.projectmodified == 1 && g.projectmodifiedstatic == 1 ) 
		{
			// trigger actions if any modification made
			darkai_invalidateobstacles ( );
			g.projectmodifiedstatic = 0;
		}
	}
}

void gridedit_updateprojectname ( void )
{
	#ifdef FPSEXCHANGE
	OpenFileMap (  1,"FPSEXCHANGE" );
	#endif

	//  add to project title
	if ( strcmp ( Lower(Left(g.projectfilename_s.Get(),Len(g.rootdir_s.Get()))) , Lower(g.rootdir_s.Get()) ) == 0 ) 
	{
		t.tprojname_s=Right(g.projectfilename_s.Get(),Len(g.projectfilename_s.Get())-Len(g.rootdir_s.Get()));
	}
	else
	{
		t.tprojname_s=g.projectfilename_s;
	}
	if (  g.projectmodified != 0  )  t.tprojname_s = t.tprojname_s+"*";

	// 011215 - Add which mode you are in
	int iEditingMode = 0;
	if ( t.grideditselect==0 ) iEditingMode = 1; // terrain
	if ( t.grideditselect==5 && t.gridentitymarkersmodeonly==0 ) iEditingMode = 2; // entity
	if ( t.grideditselect==5 && t.gridentitymarkersmodeonly==1 ) iEditingMode = 3; // markers
	if ( t.grideditselect==6 ) iEditingMode = 4; // waypoints
	switch ( iEditingMode )
	{
		case 1 : t.tprojname_s = t.tprojname_s + cstr("] - [Terrain Editing Mode"); break;
		case 2 : t.tprojname_s = t.tprojname_s + cstr("] - [Entity Editing Mode"); break;
		case 3 : t.tprojname_s = t.tprojname_s + cstr("] - [Marker Only Editing Mode"); break;
		case 4 : t.tprojname_s = t.tprojname_s + cstr("] - [Waypoint Editing Mode"); break;
	}

	// send window title text to IDE
	#ifdef FPSEXCHANGE
	OpenFileMap(1, "FPSEXCHANGE");
	SetFileMapString (  1, 1000, t.tprojname_s.Get() );
	SetFileMapDWORD (  1, 416, 1 );
	SetEventAndWait (  1 );
	while (  GetFileMapDWORD(1, 416) == 1 ) 
	{
		SetEventAndWait (  1 );
	}
	//  add to recent files list
	if (  g.projectfilename_s != "" ) 
	{
		// 091215 - if folder exists
		if ( PathExist(g.projectfilename_s.Get()) == 1 )
		{
			SetFileMapString(1, 1000, g.projectfilename_s.Get());
			SetFileMapDWORD(1, 438, 1);
			SetEventAndWait(1);
			while (GetFileMapDWORD(1, 438) == 1)
			{
				SetEventAndWait(1);
			}
		}
	}
	#endif
}

void gridedit_import_ask ( void )
{
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	do
	{
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );

		//PE: Virtual keys should not be included , as if you press a cancel (in IDE) afer that import , import will not open.
	} while ( (  t.inputsys.kscancode > 3 ) );

	// if not already loaded
	if (  t.importer.loaded == 0 ) 
	{
		OpenFileMap (  1,"FPSEXCHANGE" );
		if ( strlen ( t.timporterpath_s.Get() ) == 0 )
		{
			t.strwork = ""; t.strwork = t.strwork + g.rootdir_s+"entitybank\\";
		}
		else
		{
			t.strwork = t.timporterpath_s + "\\";
		}
		SetFileMapString (  1, 1000, t.strwork.Get() );
		t.tdone = 0;
		while (  t.tdone  !=  2 ) 
		{
			if (  t.tdone  ==  0 ) 
			{
				SetFileMapString ( 1 , 1256 , "Choose an X or FBX file for a new object or an .fpe file for existing (*.*)" );
				SetFileMapString (  1, 1512, "Import New Entity" );
			}
			else
			{
				SetFileMapString (  1, 1256, "Please try again ) You must choose either an X or FBX file or an .fpe file! (*.*)" );
				SetFileMapString (  1, 1512, "Invalid File, Please t.try again" );
			}
			SetFileMapDWORD (  1, 424, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD(1, 424) == 1 ) 
			{
				SetEventAndWait (  1 );
			}
			t.returnstring_s=GetFileMapString(1, 1000);
			t.tdone = 1;
			if (	strcmp ( Lower(Right(t.returnstring_s.Get(),2)) , ".x" ) == 0 
			||		strcmp ( Lower(Right(t.returnstring_s.Get(),4)) ,".fbx" ) == 0 
			||		strcmp ( Lower(Right(t.returnstring_s.Get(),4)) , ".dbo" ) == 0 
			||		strcmp ( Lower(Right(t.returnstring_s.Get(),4)) , ".fpe" ) == 0 
			||		t.returnstring_s  ==  ""  )  
			{
				t.tdone  =  2;
			}
		}

		// refresh 3d view so dialog Box (  (  not left black Box ) )
		for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 );  }

		// if successfully selected a good file extension
		if (  t.returnstring_s != "" ) 
		{
			// load the model
			t.timporterfile_s = t.returnstring_s;
			importer_loadmodel ( );

			// and remember folder we arrived at, so can restore next time we use importer
			LPSTR pReturnedFile = t.timporterfile_s.Get();
			for ( int n = strlen(pReturnedFile); n>0; n-- )
			{
				if ( pReturnedFile[n] == '\\' || pReturnedFile[n] == '/' )
				{
					t.timporterpath_s = t.timporterfile_s;
					LPSTR pImporterPath = t.timporterpath_s.Get();
					pImporterPath[n] = 0;
					break;
				}
			}
		}
	}
	#endif
}

void gridedit_intercept_savefirst ( void )
{
	t.editorcanceltask=0;
	if (  g.projectmodified == 1 ) 
	{
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		SetFileMapString (  1, 1000, t.strarr_s[369].Get() );
		SetFileMapString (  1, 1256, t.strarr_s[370].Get() );
		SetFileMapDWORD (  1, 900, 2 );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1, 900) != 0 ) 
		{
			SetEventAndWait (  1 );
		}
		t.tokay=GetFileMapDWORD(1, 904);
		#endif

		//  refresh 3d view so dialog Box (  (  not left black Box ) )
		for ( t.tsync = 1 ; t.tsync <= 5 ; t.tsync++ ) {  Sync (   ); SleepNow (  10  ); }

		if (  t.tokay == 1 ) 
		{
			//  yes save first
			gridedit_save_map_ask ( );
			g.projectmodified=0  ; gridedit_changemodifiedflag ( );
			g.projectmodifiedstatic = 0;
		}
		if (  t.tokay == 2 ) 
		{
			//  task cancelled
			t.editorcanceltask=1;
		}
	}
}

void gridedit_intercept_savefirst_noreload ( void )
{
	g.savenoreloadflag = 1;
	gridedit_intercept_savefirst();
	g.savenoreloadflag = 0;
}

void gridedit_open_map_ask ( void )
{
	//  SAVE CURRENT (IF ANY)
	t.editorcanceltask=0;
	if (  g.projectmodified == 1 ) 
	{
		//  If project modified, ask if want to save first
		gridedit_intercept_savefirst ( );
	}
	if (  t.editorcanceltask == 0 ) 
	{
		//  OPEN FPM
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		t.strwork = g.mysystem.mapbankAbs_s;		
		SetFileMapString (  1, 1000, t.strwork.Get() );
		SetFileMapString (  1, 1256, t.strarr_s[371].Get() );
		SetFileMapString (  1, 1512, t.strarr_s[372].Get() );
		SetFileMapDWORD (  1, 424, 1 );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1, 424) == 1 ) 
		{
			SetEventAndWait (  1 );
		}
		t.returnstring_s=GetFileMapString(1, 1000);
		#endif

		//  refresh 3d view so dialog Box (  (  not left black Box ) )
		for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }

		if (  t.returnstring_s != "" ) 
		{
			if (  cstr(Lower(Right(t.returnstring_s.Get(),4))) == ".fpm" ) 
			{
				g.projectfilename_s=t.returnstring_s;
				gridedit_load_map ( );
			}
		}
	}
}

void gridedit_new_map_ask ( void )
{
	//  SAVE CURRENT (IF ANY)
	t.editorcanceltask=0;
	if (  g.projectmodified == 1 ) 
	{
		//  If project modified, ask if want to save first
		gridedit_intercept_savefirst ( );
	}

	#ifdef VRTECH
	//  refresh 3d view so dialog Box-  not left black Box
	//if ( gbWelcomeSystemActive == false )
	//{
	//	for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }
	//}
	#else
	//  refresh 3d view so dialog Box-  not left black Box
	if ( gbWelcomeSystemActive == false )
	{
		for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }
	}
	#endif

	if (  t.editorcanceltask == 0 ) 
	{
		//  NEW MAP
		gridedit_new_map ( );
	}
}

void gridedit_save_map_ask ( void )
{
	if (  g.projectfilename_s == "" ) 
	{
		gridedit_saveas_map ( );
	}
	else
	{
		gridedit_save_map ( );
	}
return;

}

void gridedit_saveas_map ( void )
{
	//  SAVE AS DIALOG
	#ifdef FPSEXCHANGE
	OpenFileMap (  1,"FPSEXCHANGE" );
	t.strwork = g.mysystem.mapbankAbs_s;
	SetFileMapString (  1, 1000, t.strwork.Get() );
	SetFileMapString (  1, 1256, t.strarr_s[373].Get() );
	SetFileMapString (  1, 1512, t.strarr_s[374].Get() );
	SetFileMapDWORD (  1, 428, 1 );
	SetEventAndWait (  1 );
	while (  GetFileMapDWORD(1, 428) == 1 ) 
	{
		SetEventAndWait (  1 );
	}
	t.returnstring_s=GetFileMapString(1, 1000);
	#endif

	//  refresh 3d view so dialog Box (  (  not left black Box ) )
	for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }

	if (  t.returnstring_s != "" ) 
	{
		if (  cstr(Lower(Right(t.returnstring_s.Get(),4))) != ".fpm"  )  t.returnstring_s = t.returnstring_s+".fpm";
		g.projectfilename_s=t.returnstring_s;
		gridedit_save_map ( );
	}
}

void gridedit_addentitytomap(void)
{
	// mark as static if it was
	if (t.gridentitystaticmode == 1) g.projectmodifiedstatic = 1;
	entity_addentitytomap();
	entity_recordbuffer_add();

	// if entity is a light, has a probe
}

void gridedit_deleteentityfrommap ( void )
{
	// can intercept delete if char+start marker
	t.tstoretentitytoselect=t.tentitytoselect;
	if (  t.playercontrol.thirdperson.enabled == 1 ) 
	{
		if (  t.gridentity == 0 || (t.gridentity>0 && t.entityprofile[t.gridentity].ismarker != 1) ) 
		{
			t.tstmrke=t.playercontrol.thirdperson.startmarkere;
			if (  t.tentitytoselect == t.tstmrke ) 
			{
				//  first delete char on start marker, and restore marker
				t.tentitytoselect=t.playercontrol.thirdperson.charactere;
				t.tstmrkobj=t.entityelement[t.tstmrke].obj;
				if (  t.tstmrkobj>0 ) 
				{
					if (  ObjectExist(t.tstmrkobj) == 1 ) 
					{
						DisableObjectZDepth (  t.tstmrkobj );
						DisableObjectZWrite (  t.tstmrkobj );
						DisableObjectZRead (  t.tstmrkobj );
					}
				}
				//  and reset third person settings
				t.playercontrol.thirdperson.enabled=0;
				t.playercontrol.thirdperson.charactere=0;
				t.playercontrol.thirdperson.startmarkere=0;
			}
		}
	}

	// if entity is a light, remove its probe

	// mark as static if it was
	if ( t.entityelement[t.tentitytoselect].staticflag == 1 ) g.projectmodifiedstatic = 1;
	entity_recordbuffer_delete ( );
	entity_deleteentityfrommap ( );

	//  restore tentitytoselect in case switched it
	t.tentitytoselect=t.tstoretentitytoselect;
}

void gridedit_deleteentityrubberbandfrommap ( void )
{
	g.entityrubberbandlistundo.clear();

	// will delete all entities in rubber band list, and preserve them into undo buffer
	for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
	{
		t.tentitytoselect = g.entityrubberbandlist[i].e;
		if ( t.tentitytoselect > 0 && t.entityelement[t.tentitytoselect].editorlock == 0)
		{

			if ( t.entityelement[t.tentitytoselect].staticflag == 1 ) g.projectmodifiedstatic = 1;
			gridedit_deleteentityfrommap ( );
			g.entityrubberbandlistundo.push_back ( t.entityundo );
		}
	}
	// special code to point this undo event to the rubberbandlist undo buffer
	if ( g.entityrubberbandlistundo.size() > 0 )
	{
		t.entityundo.entityindex = -123;
		t.entityundo.bankindex = -123;
	}
}

void gridedit_moveentityrubberband ( void )
{
	// will move all entities in rubber band list, and preserve them into undo buffer
	g.entityrubberbandlistundo.clear();
	for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
	{
		t.tentitytoselect = g.entityrubberbandlist[i].e;
		entity_recordbuffer_move ( );
		g.entityrubberbandlistundo.push_back ( t.entityundo );
	}
	// special code to point this undo event to the rubberbandlist undo buffer
	if ( g.entityrubberbandlistundo.size() > 0 )
	{
		t.entityundo.entityindex = -123;
		t.entityundo.bankindex = -123;
	}
}

void gridedit_updateentityobj ( void )
{
	//  moved to m-entity
	entity_updateentityobj ( );
}

void gridedit_recreateentitycursor ( void )
{
	int ele_id = 0;
	//  Entity floating selection
	if ( t.gridentityobj>0 ) 
	{
		//  character creator remove glued objects
		if (  t.toldCursorEntidForCharacterCreator > 0 ) 
		{
			if (  t.entityprofile[t.toldCursorEntidForCharacterCreator].ischaractercreator  ==  1 ) 
			{
				t.tccobj = g.charactercreatorrmodelsoffset+((t.toldCursorEntidForCharacterCreator*3)-t.characterkitcontrol.bankOffset);
				if (  ObjectExist(t.tccobj) == 1 ) 
				{
					UnGlueObject (  g.charactercreatorrmodelsoffset+((t.toldCursorEntidForCharacterCreator*3)-t.characterkitcontrol.bankOffset)+1 );
					UnGlueObject (  g.charactercreatorrmodelsoffset+((t.toldCursorEntidForCharacterCreator*3)-t.characterkitcontrol.bankOffset)+2 );
					UnGlueObject (  t.tccobj );
				}
			}
		}
		if ( ObjectExist(t.gridentityobj) == 1  ) DeleteObject (  t.gridentityobj );
		t.gridentityobj=0;
		t.toldCursorEntidForCharacterCreator = 0;
	}
	if (  t.gridentity>0 ) 
	{
		t.obj=g.entityviewcursorobj;
		t.sourceobj=g.entitybankoffset+t.gridentity;
		if (ObjectExist(t.sourceobj) == 1)
		{
			t.entid=t.gridentity ; t.entobj=t.obj;
			if ( t.entityprofile[t.entid].ischaracter == 1 || t.entityprofile[t.entid].ismarker != 0 || t.entityprofile[t.entid].animmax>0 ) 
			{
				//  Close allows animation independence
				CloneObject ( t.obj, t.sourceobj );

				//  Character creator head
				if ( t.entityprofile[t.entid].ischaractercreator == 1 ) 
				{
					t.toldCursorEntidForCharacterCreator = t.entid;
					t.tSourcebip01_head=getlimbbyname(t.obj, "Bip01_Head");
					if ( t.tSourcebip01_head > 0 ) 
					{
						t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset);
						if ( ObjectExist(t.tccobj) == 1 ) 
						{
							t.tBip01_FacialHair=getlimbbyname(t.tccobj, "Bip01_FacialHair");
							if ( t.tBip01_FacialHair > 0  )  GlueObjectToLimbEx (  g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+1,t.tccobj,t.tBip01_FacialHair,2 );
							t.Bip01_Headgear=getlimbbyname(t.tccobj, "Bip01_Headgear");
							if ( t.Bip01_Headgear > 0  )  GlueObjectToLimbEx (  g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+2,t.tccobj,t.Bip01_Headgear,2 );
							GlueObjectToLimbEx (  t.tccobj,t.obj,t.tSourcebip01_head,2 );
						}
					}
				}
			}
			else
			{
				//  Instance creation cheaper
				InstanceObject (  t.obj,t.sourceobj );
			}
			t.gridentityunderground=0;

			// other entity attributes
			if ( t.entityprofile[t.entid].ismarker != 0 && t.entityprofile[t.entid].ismarker != 11 ) //Allow cullmode on 11
			{
				// special setup for marker objects
				SetObjectTransparency ( t.obj, 2 );
				SetObjectCull ( t.obj, 1 );
			}
			else
			{
				if (t.entityprofile[t.entid].cullmode >= 0)
				{
					if (t.entityprofile[t.entid].cullmode != 0)
					{
						//  cull mode OFF used for single sided polygon models (Reloaded)
						//  to help with palm tree leaves
						SetObjectCull(t.obj, 0);
					}
					else
					{
						SetObjectCull(t.obj, 1);
					}
				}
				//  set transparency mode
				{
					if (t.entityprofile[t.entid].transparency >= 0)
					{
						SetObjectTransparency(t.obj, t.entityprofile[t.entid].transparency);
					}
				}
				// 051115 - only if not using limb visibility for hiding decal arrow
				if ( t.entityprofile[t.entid].addhandlelimb==0 )
				{
					//  set LOD attributes for entities
					entity_calculateentityLODdistances ( t.entid, t.obj, 0 );
				}
			}
			if (t.obj > 0 && GetNumberOfFrames(t.obj)>0 )
			{
				SetObjectFrame (  t.obj,0 );
				if (  t.entityprofile[t.entid].animmax>0 && t.entityprofile[t.entid].playanimineditor>0 && t.entityprofile[t.entid].ischaractercreator == 0 ) 
				{
					t.q=t.entityprofile[t.entid].playanimineditor-1;
					LoopObject (  t.obj,t.entityanim[t.entid][t.q].start,t.entityanim[t.entid][t.q].finish );
				}
				else
				{
					LoopObject (  t.obj  ); StopObject (  t.obj );
				}
			}
		}
		else
		{
			MakeObjectCube (  t.obj,25 );
		}
		//  ensure new object ONLY interacts with main camera and shadow camera
		//PE: 130217 added t.entityprofile[t.gridentity].zdepth == 0 to prevent decals from calling technique11 DepthMap
		if (  t.entityprofile[t.gridentity].ismarker != 0 || t.entityprofile[t.gridentity].zdepth == 0 )
		{
			SetObjectMask (  t.obj, 1 );
		}
		else
		{
			SetObjectMask (  t.obj, 1+(1<<31) );
		}
		//  pivot alignment
		if (  t.entityprofile[t.gridentity].fixnewy != 0 ) 
		{
			RotateObject (  t.obj,0,t.entityprofile[t.gridentity].fixnewy,0 );
			FixObjectPivot (  t.obj );
		}
		// scale
		t.tescale=t.entityprofile[t.gridentity].scale;
		if (t.tescale > 0)
		{
			ScaleObject (t.obj, t.tescale, t.tescale, t.tescale);
			if (t.entityprofile[t.gridentity].ismarker == 10)
			{
				// and then reset scale for particle markers
				ScaleObject(t.obj, t.entityprofile[t.gridentity].scale, t.entityprofile[t.gridentity].scale, t.entityprofile[t.gridentity].scale);
			}
		}

		SetObjectCollisionOff (  t.obj );
		if ( g.entityrubberbandlist.size() == 1 )
		{
			{
				// only dehighlight if single extract, not if a rubber band / linked entities extraction
				editor_restoreobjhighlightifnotrubberbanded(t.obj);
			}
		}
		t.gridentityobj=t.obj;


		

	}

	editor_refreshentitycursor ( );
}

void gridedit_displayentitycursor ( void )
{
	//  create entity foating selection
	if (  t.gridentity>0 ) 
	{
		t.obj=t.gridentityobj;
		if (  ObjectExist(t.obj) == 1 ) 
		{
			PositionObject (  t.obj,t.gridentityposx_f,t.gridentityposy_f,t.gridentityposz_f );
			RotateObject (  t.obj,t.gridentityrotatex_f,t.gridentityrotatey_f,t.gridentityrotatez_f );
			t.tfinalscalex_f=t.gridentityscalex_f;
			t.tfinalscaley_f=t.gridentityscaley_f;
			t.tfinalscalez_f=t.gridentityscalez_f;
			ScaleObject ( t.obj, t.tfinalscalex_f, t.tfinalscaley_f, t.tfinalscalez_f );
			if (t.entityprofile[t.gridentity].ismarker == 10)
			{
				ScaleObject (t.obj, t.entityprofile[t.gridentity].scale, t.entityprofile[t.gridentity].scale, t.entityprofile[t.gridentity].scale);
			}
			if (  t.gridentity>0 )
			{
				if (  t.entityprofile[t.gridentity].ischaracter == 0 ) 
				{
					t.tanimspeed_f=t.entityprofile[t.gridentity].animspeed;
					SetObjectSpeed (  t.obj,g.timeelapsed_f*t.tanimspeed_f );
				}
			}
		}
	}

	//  if entity cursor light, instantly feed into shader OVERRIDING LIGHT ZERO
	lighting_override ( );
}

void gridedit_deletelevelobjects ( void )
{
	// clear OBJ values in entityelements (as all objects are being removed)
	if ( g.entityelementlist>0 ) 
	{
		for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
		{

			t.obj = t.entityelement[t.e].obj;
			if ( t.obj > 0 ) 
			{
				if ( ObjectExist(t.obj) == 1 ) DeleteObject (  t.obj );
			}

			//  clear any character creator objects associated with this entity
			#ifdef VRTECH
			#else
			t.ccobjToDelete=t.obj;
			characterkit_deleteEntity ( );
			#endif

			t.entityelement[t.e].obj=0;
			t.entityelement[t.e].bankindex=0;
			deleteinternalsound(t.entityelement[t.e].soundset) ; t.entityelement[t.e].soundset=0;
			deleteinternalsound(t.entityelement[t.e].soundset1) ; t.entityelement[t.e].soundset1=0;
			deleteinternalsound(t.entityelement[t.e].soundset2) ; t.entityelement[t.e].soundset2=0;
			deleteinternalsound(t.entityelement[t.e].soundset3) ; t.entityelement[t.e].soundset3=0;
			deleteinternalsound(t.entityelement[t.e].soundset4) ; t.entityelement[t.e].soundset4=0;
		}
	}
	UnDim (  t.entityelement );
	#ifdef VRTECH
	#else
	UnDim (  t.entityshadervar );
	UnDim (  t.entitydebug_s );
	#endif
	g.entityelementmax=100;
	Dim (  t.entityelement,g.entityelementmax  );
	#ifdef VRTECH
	#else
	Dim2(  t.entityshadervar,g.entityelementmax, g.globalselectedshadermax  );
	Dim (  t.entitydebug_s,g.entityelementmax  );
	#endif
	g.entityelementlist=0;

	//  delete all objects used for level edit
	for ( t.obj = g.entityviewstartobj ; t.obj <= g.entityviewendobj; t.obj++ )
	{
		if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
	}

	//  also delete all entitybank references
	entity_deletebank ( );

	//  Indicate no level objects
	g.entityviewendobj=0;

	//  270215 - 011 - Create new entities from the beginning
	g.entityviewcurrentobj=g.entityviewstartobj;
}

void modifyplaneimagestrip ( int objno, int texmax, int texindex )
{
	float s_f = 0;
	float u_f = 0;

	//  Lock the vertex data of the object
	LockVertexDataForLimbCore (  objno,0,1 );

	//  adjust UV data
	s_f=1.0/texmax ; u_f=texindex*s_f;
	SetVertexDataUV (  0,u_f+s_f,0.0 );
	SetVertexDataUV (  1,u_f,0.0 );
	SetVertexDataUV (  2,u_f+s_f,1.0 );
	SetVertexDataUV (  3,u_f,0.0 );
	SetVertexDataUV (  4,u_f,1.0 );
	SetVertexDataUV (  5,u_f+s_f,1.0 );

	//  Unlock the vertex data of the object
	UnlockVertexData (  );

//endfunction

}


// 
//  BUILD GAME and PREFERENCES code removed on 180215
// 

#ifdef ENABLEIMGUI
void imgui_set_openproperty_flags(int iMasterID)
{
	//  Open property window
	t.editorinterfaceactive = t.e;

	//  Setup usage flags
	t.tsimplecharview = 0;
	t.tflaglives = 0; t.tflaglight = 0; t.tflagobjective = 0; t.tflagtdecal = 0; t.tflagdecalparticle = 0; t.tflagspawn = 0; t.tflagifused = 0;
	t.tflagnewparticle = 0;
	t.tflagvis = 0; t.tflagchar = 0; t.tflagweap = 0; t.tflagammo = 0; t.tflagai = 1; t.tflagsound = 0; t.tflagsoundset = 0; t.tflagnosecond = 0;
	t.tflagmobile = 0; t.tflaghurtfall = 0; t.tflaghasweapon = 0; t.tflagammoclip = 0; t.tflagstats = 0; t.tflagquantity = 0;
	t.tflagvideo = 0;
	t.tflagplayersettings = 0;
	t.tflagusekey = 0;
	t.tflagteamfield = 0;
	int tflagtext = 0;
	int tflagimage = 0;

	//  If its static and arena mode, only do optional visuals, ignore rest
	t.tstatic = 0;

	// 070510 - simplified character properties
	if (g.gsimplifiedcharacterediting == 1 && t.entityprofile[iMasterID].ischaracter == 1)
	{
		//  flag the simple character properties layout (FPGC)
		t.tsimplecharview = 1;
	}
	else
	{
		//  FPGC - 260310 - new entitylight indicated with new flag
		if (t.entityprofile[iMasterID].islightmarker == 1)
		{
			t.tflaglight = 1;
		}
		else
		{
			if (t.entityprofile[iMasterID].ismarker == 0)
			{
				t.tflagvis = 1; t.tflagmobile = 1; t.tflagobjective = 1; t.tflagsound = 1; t.tflagstats = 1; t.tflagspawn = 1;
				// 070115 - removed until UBER character (multiweapon) is ready for action
				// t.entityprofile[iMasterID].ischaracter>0 then t.tflagchar = 1  ) ; t.tflaghasweapon = 1 ; t.tflagsoundset = 1 ; t.tflagsound = 0
				if (t.entityprofile[iMasterID].ischaracter > 0) { t.tflagchar = 1; t.tflagsoundset = 1; t.tflagsound = 0; }
				if (Len(t.entityprofile[iMasterID].isweapon_s.Get()) > 2) { t.tflagweap = 1; t.tflagammoclip = 1; t.tflagsound = 0; }
				if (t.entityprofile[iMasterID].isammo > 0) { t.tflagammo = 1; t.tflagobjective = 0; t.tflagsound = 0; }
				t.tflagusekey = 1;
			}
			else
			{
				if (t.entityprofile[iMasterID].ismarker == 1)
				{
					t.tflagai = 0;
					//  FPGC - 160909 - filtered fpgcgenre=1 is shooter genre
					if (g.fpgcgenre == 1)
					{
						//  Shooter legacy properties for player start
						if (t.entityprofile[iMasterID].lives > 0)
						{
							t.tflagstats = 1; t.tflaglives = 1; t.tflagsoundset = 1; t.tflaghurtfall = 1; t.tflaghasweapon = 1; t.tflagquantity = 1;
							t.tflagplayersettings = 1;
							t.tflagnosecond = 1;
						}
						else
						{
							t.tflagsound = 1; t.tflagnosecond = 1;
						}
					}
					else
					{
						//  Other genre's have no ammo quantity and weapon is renamed as equipment
						if (t.entityprofile[iMasterID].lives == -1)
						{
							//  checkpint marker is type 1
							t.tflagsound = 1; t.tflagnosecond = 1;
						}
						else
						{
							t.tflagstats = 1; t.tflaglives = 1; t.tflagsoundset = 1; t.tflaghurtfall = 1; t.tflaghasweapon = 1;
						}
					}
				}
				if (t.entityprofile[iMasterID].ismarker == 3 || t.entityprofile[iMasterID].ismarker == 6 || t.entityprofile[iMasterID].ismarker == 8)
				{
					t.tflagnosecond = 1; t.tflagifused = 1;
				}
				if (t.entityprofile[iMasterID].ismarker == 4) { t.tflagtdecal = 1; t.tflagdecalparticle = 1; }
				if (t.entityprofile[iMasterID].ismarker == 10) { t.tflagnewparticle = 1; }
				if (t.entityprofile[iMasterID].ismarker == 3)
				{
					if (t.entityprofile[iMasterID].markerindex <= 1)
					{
						if (t.entityprofile[iMasterID].markerindex == 1)
						{
							// video
							t.tflagvideo = 1;
						}
						else
						{
							// sound
							t.tflagsound = 1;
						}
					}
					else
					{
						if (t.entityprofile[iMasterID].markerindex == 2) tflagtext = 1;
						if (t.entityprofile[iMasterID].markerindex == 3) tflagimage = 1;
					}
				}
				if (t.entityprofile[iMasterID].ismarker == 7)
				{
					//  multiplayer start marker
					t.tflagstats = 1;
					t.tflaghurtfall = 1;
					t.tflagplayersettings = 1;
					t.tflagteamfield = 1;
				}
				if (t.entityprofile[iMasterID].ismarker == 8)
				{
					// floor zone marker
					t.tflagsound = 0;
				}
				if (t.entityprofile[iMasterID].ismarker == 9)
				{
					// cover zone marker
					t.tflagifused = 1;
				}
			}
		}
	}

	// parental control removes weapons and violence properties
	if (g.quickparentalcontrolmode == 2)
	{
		t.tflagweap = 0;
		t.tflagammo = 0;
	}

	//PE: New flags check.
	// special VR mode can remove even more
	t.tflagnotionofhealth = 1;
	t.tflagsimpler = 0;
	//if ( bVRQ2ZeroViolenceMode == true )
	//{
	//	t.tflaglives=0; 
	//	t.tflaghurtfall=0; 
	//	t.tflaghasweapon=0; 
	//	t.tflagammoclip=0;
	//	t.tflagnotionofhealth=0;
	//	t.tflagsimpler = 1;
	//}


}
#endif

// 
//  PROPERTIES
// 

void interface_openpropertywindow ( void )
{
	//  Open proprty window
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetFileMapDWORD (  1, 978, 1 );
	SetFileMapDWORD (  1, 458, 0 );
	SetEventAndWait (  1 );
	t.editorinterfaceactive=t.e;

	//  open the entity file map
	OpenFileMap (  2, "FPSENTITY" );
	SetEventAndWait (  2 );

	//  wait until the entity window is read
	if (  GetFileMapDWORD( 2, ENTITY_SETUP )  ==  1 ) 
	{
		#ifdef VRTECH
		// special VRQ2 mode also hides concepts of lives, health, blood, violence (substitute health for strength)
		bool bVRQ2ZeroViolenceMode = false;
		if ( g.vrqcontrolmode != 0 ) bVRQ2ZeroViolenceMode = true;//if ( g.gvrmode == 3 ) bVRQ2ZeroViolenceMode = true;
		#endif

		//  Setup usage flags
		t.tsimplecharview=0;
		t.tflaglives=0 ; t.tflaglight=0 ; t.tflagobjective=0 ; t.tflagtdecal=0 ; t.tflagdecalparticle=0 ; t.tflagspawn=0 ; t.tflagifused=0;
		t.tflagnewparticle = 0;
		t.tflagvis=0 ; t.tflagchar=0 ; t.tflagweap=0 ; t.tflagammo=0 ; t.tflagai=1 ; t.tflagsound=0 ; t.tflagsoundset=0 ; t.tflagnosecond=0;
		t.tflagmobile=0 ; t.tflaghurtfall=0 ; t.tflaghasweapon=0 ; t.tflagammoclip=0 ; t.tflagstats=0 ; t.tflagquantity=0;
		t.tflagvideo=0;
		t.tflagplayersettings=0;
		t.tflagusekey=0;
		t.tflagteamfield=0;
		int tflagtext=0;
		int tflagimage=0;

		//  If its static and arena mode, only do optional visuals, ignore rest
		t.tstatic=0;

		// 070510 - simplified character properties
		if (  g.gsimplifiedcharacterediting == 1 && t.entityprofile[t.gridentity].ischaracter == 1 ) 
		{
			//  flag the simple character properties layout (FPGC)
			t.tsimplecharview=1;
		}
		else
		{
			//  FPGC - 260310 - new entitylight indicated with new flag
			if (  t.entityprofile[t.gridentity].islightmarker == 1 ) 
			{
				t.tflaglight=1;
			}
			else
			{
				if (  t.entityprofile[t.gridentity].ismarker == 0 ) 
				{
					t.tflagvis=1 ; t.tflagmobile=1 ; t.tflagobjective=1 ; t.tflagsound=1 ; t.tflagstats=1 ; t.tflagspawn=1;
					// 070115 - removed until UBER character (multiweapon) is ready for action
					// t.entityprofile[t.gridentity].ischaracter>0 then t.tflagchar = 1  ) ; t.tflaghasweapon = 1 ; t.tflagsoundset = 1 ; t.tflagsound = 0
					if (  t.entityprofile[t.gridentity].ischaracter>0 ) { t.tflagchar = 1  ; t.tflagsoundset = 1 ; t.tflagsound = 0; }
					if (  Len(t.entityprofile[t.gridentity].isweapon_s.Get())>2 ) { t.tflagweap = 1  ; t.tflagammoclip = 1 ; t.tflagsound = 0; }
					if (  t.entityprofile[t.gridentity].isammo>0 ) { t.tflagammo = 1  ; t.tflagobjective = 0 ; t.tflagsound = 0; }
					t.tflagusekey=1;
				}
				else
				{
					if (  t.entityprofile[t.gridentity].ismarker == 1 ) 
					{
						t.tflagai=0;
						//  FPGC - 160909 - filtered fpgcgenre=1 is shooter genre
						if (  g.fpgcgenre == 1 ) 
						{
							//  Shooter legacy properties for player start
							if (  t.entityprofile[t.gridentity].lives>0 ) 
							{
								t.tflagstats=1 ; t.tflaglives=1 ; t.tflagsoundset=1 ; t.tflaghurtfall=1 ; t.tflaghasweapon=1 ; t.tflagquantity=1;
								t.tflagplayersettings=1;
								t.tflagnosecond=1;
							}
							else
							{
								t.tflagsound=1 ; t.tflagnosecond=1;
							}
						}
						else
						{
							//  Other genre's have no ammo quantity and weapon is renamed as equipment
							if (  t.entityprofile[t.gridentity].lives == -1 ) 
							{
								//  checkpint marker is type 1
								t.tflagsound=1 ; t.tflagnosecond=1;
							}
							else
							{
								t.tflagstats=1 ; t.tflaglives=1 ; t.tflagsoundset=1 ; t.tflaghurtfall=1 ; t.tflaghasweapon=1;
							}
						}
					}
					if (  t.entityprofile[t.gridentity].ismarker == 3 || t.entityprofile[t.gridentity].ismarker == 6 || t.entityprofile[t.gridentity].ismarker == 8 ) 
					{
						t.tflagnosecond=1 ; t.tflagifused=1;
						#ifdef VRTECH
						#else
						t.tflagsound=1;
						#endif
					}
					if (  t.entityprofile[t.gridentity].ismarker == 4 ) { t.tflagtdecal = 1  ; t.tflagdecalparticle = 1; }
					if (  t.entityprofile[t.gridentity].ismarker == 10 ) { t.tflagnewparticle = 1; }
					if (  t.entityprofile[t.gridentity].ismarker == 3 ) 
					{
						if (  t.entityprofile[t.gridentity].markerindex <= 1 ) 
						{
							if (  t.entityprofile[t.gridentity].markerindex == 1 ) 
							{
								// video
								t.tflagvideo=1;
							}
							else
							{
								// sound
								t.tflagsound=1;
							}
						}
						else
						{
							if ( t.entityprofile[t.gridentity].markerindex == 2 ) tflagtext=1;
							#ifdef VRTECH
							#else
							 if ( t.entityprofile[t.gridentity].markerindex == 3 ) tflagimage=1;
							#endif
						}
					}
					if (  t.entityprofile[t.gridentity].ismarker == 7 ) 
					{
						//  multiplayer start marker
						t.tflagstats=1;
						t.tflaghurtfall=1;
						t.tflagplayersettings=1;
						t.tflagteamfield=1;
					}
					if (  t.entityprofile[t.gridentity].ismarker == 8 ) 
					{
						// floor zone marker
						t.tflagsound=0;
					}
					if (  t.entityprofile[t.gridentity].ismarker == 9 ) 
					{
						// cover zone marker
						t.tflagifused = 1;
					}
				}
			}
		}

		// parental control removes weapons and violence properties
		if ( g.quickparentalcontrolmode == 2 )
		{
			t.tflagweap = 0;
			t.tflagammo = 0;
		}

		#ifdef VRTECH
		// special VR mode can remove even more
		t.tflagnotionofhealth = 1;
		t.tflagsimpler = 0;
		if ( bVRQ2ZeroViolenceMode == true )
		{
			t.tflaglives=0; 
			t.tflaghurtfall=0; 
			t.tflaghasweapon=0; 
			t.tflagammoclip=0;
			t.tflagnotionofhealth=0;
			t.tflagsimpler = 1;
		}
		#endif

		//  set array and counters to track scope of contents of each group
		Dim (  t.propfield,16  );
		for ( t.t = 0 ; t.t <= 16 ; t.t++ ) t.propfield[t.t]=0 ; 

		//  set the window title
		setpropertybase(ENTITY_WINDOW_TITLE,t.strarr_s[411].Get());

		//  FPGC - 070510 - open entity properties filemap and wait for signal to write
		OpenFileMap ( 3, "ENTITYPROPERTIES" );
		g.g_filemapoffset = 8;
		if ( DLLExist(1) == 0 )  DLLLoad (  "Kernel32.dll", 1 );
		while (  GetFileMapDWORD(3,0)  ==  1 ) 
		{
			CallDLL (  1,"Sleep",10 );
		}

		if (  t.tsimplecharview == 1 ) 
		{
			//  Wizard (simplified) property editing
			t.group=0 ; startgroup("Character Info") ; t.controlindex=0;
			#ifdef VRTECH
			setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[413].Get(),"Choose a unique name for this character") ; ++t.controlindex;
			#else
			setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[478].Get(),"Choose a unique name for this character") ; ++t.controlindex;
			#endif
			setpropertylist2(t.group,t.controlindex,t.grideleprof.aimain_s.Get(),"Behaviour","Select a behaviour for this character",11) ; ++t.controlindex;
			setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),"Voiceover","Select t.a WAV or OGG file this character will use during their behavior","audiobank\\") ; ++t.controlindex;
			setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),"If Used","Sometimes used to specify the name of an entity to be activated") ; ++t.controlindex;
		}
		else
		{
			//  Name
			t.group=0 ; startgroup(t.strarr_s[412].Get()) ; t.controlindex=0;
			#ifdef VRTECH
			if ( t.entityprofile[t.gridentity].ischaracter > 0 )
			{
				setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[478].Get(),t.strarr_s[204].Get());
			}
			else
			{
				if ( t.entityprofile[t.gridentity].ismarker > 0 )
				{
					if ( t.entityprofile[t.gridentity].islightmarker > 0 )
						setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[483].Get(),t.strarr_s[204].Get());
					else
						setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[479].Get(),t.strarr_s[204].Get());
				}
				else
					setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[413].Get(),t.strarr_s[204].Get());
			}
			++t.controlindex;
			#else
			setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[413].Get(),t.strarr_s[204].Get()) ; ++t.controlindex;
			#endif
			if (  t.entityprofile[t.gridentity].ismarker == 0 || t.entityprofile[t.gridentity].islightmarker == 1 ) 
			{
				if (  g.gentitytogglingoff == 0 ) 
				{
					t.tokay=1;
					if (  ObjectExist(g.entitybankoffset+t.gridentity) == 1 ) 
					{
						if (  GetNumberOfFrames(g.entitybankoffset+t.gridentity)>0 ) 
						{
							t.tokay=0;
						}
					}
					if (  t.tokay == 1 ) 
					{
						//PE: 414=Static Mode
						setpropertylist2(t.group,t.controlindex,Str(t.gridentitystaticmode),t.strarr_s[414].Get(),t.strarr_s[205].Get(),0) ; ++t.controlindex;
					}
				}

				//LB: add flag to control whether the entity reflects in the water (uses castshadow = -2 to say YES)
				int iReflectUsingCastVar = 0;
				if (t.grideleprof.castshadow == 1) iReflectUsingCastVar = 0;
				if (t.grideleprof.castshadow == 0) iReflectUsingCastVar = 0;
				if (t.grideleprof.castshadow == -1) iReflectUsingCastVar = 1;
				if (t.grideleprof.castshadow == -2) iReflectUsingCastVar = 2;
				setpropertylist2(t.group, t.controlindex, Str(iReflectUsingCastVar), "Shadows and Reflection", "Sets the shadow casting and reflection behavior of this entity", 6); ++t.controlindex;
			}

			// 101016 - Additional General Parameters
			if ( t.tflagchar == 0 && t.tflagvis == 1 ) 
			{
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocluder),"Occluder","Set to YES makes this object an occluder",0) ; ++t.controlindex;
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocludee),"Occludee","Set to YES makes this object an occludee",0) ; ++t.controlindex;
				}
				#else
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocluder),"Occluder","Set to YES makes this entity an occluder",0) ; ++t.controlindex;
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocludee),"Occludee","Set to YES makes this entity an occludee",0) ; ++t.controlindex;
				#endif
				
				// these will be back when EBE needs doors and windows
				//setpropertystring2(t.group,Str(t.grideleprof.parententityindex),"Parent Index","Selects another entity element to be a parent") ; ++t.controlindex;
				//setpropertystring2(t.group,Str(t.grideleprof.parentlimbindex),"Parent Limb","Specifies the limb index of the parent to connect with") ; ++t.controlindex;
			}

			// 281116 - added Specular Control per entity
			if ( t.tflagvis == 1 ) 
			{
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					setpropertystring2(t.group,Str(t.grideleprof.specularperc),"Specular","Set specular percentage to modulate object specular effect")  ; ++t.controlindex; 
				}
				#else
				setpropertystring2(t.group,Str(t.grideleprof.specularperc),"Specular","Set specular percentage to modulate entity specular effect")  ; ++t.controlindex; 
				#endif
			}

			//  Basic AI
			if (  t.tflagai == 1 ) 
			{
				// can redirect to better folders if in g.quickparentalcontrolmode
				LPSTR pAIRoot = "scriptbank\\";
				if ( g.quickparentalcontrolmode == 2 )
				{
					if ( t.entityprofile[t.gridentity].ismarker == 0 ) 
					{
						if ( t.tflagchar == 1 )
							pAIRoot = "scriptbank\\people\\";
						else
							pAIRoot = "scriptbank\\objects\\";
					}
					else
					{
						pAIRoot = "scriptbank\\markers\\";
					}
				}

				t.propfield[t.group]=t.controlindex;
				#ifdef VRTECH
				++t.group ; startgroup(t.strarr_s[415].Get()) ; t.controlindex=0;
				#else
				++t.group ; startgroup("AI System") ; t.controlindex=0;
				#endif
				setpropertyfile2(t.group,t.grideleprof.aimain_s.Get(),t.strarr_s[417].Get(),t.strarr_s[207].Get(),pAIRoot) ; ++t.controlindex;
			}

			//  Has Weapon
			if (  t.tflaghasweapon == 1 && t.playercontrol.thirdperson.enabled == 0 && g.quickparentalcontrolmode != 2 ) 
			{
				setpropertylist2(t.group,t.controlindex,t.grideleprof.hasweapon_s.Get(),t.strarr_s[419].Get(),t.strarr_s[209].Get(),1) ; ++t.controlindex;
			}

			//  Is Weapon (FPGC - 280809 - filtered fpgcgenre=1 is shooter genre)
			if (  t.tflagweap == 1 && g.fpgcgenre == 1 ) 
			{
				setpropertystring2(t.group,Str(t.grideleprof.damage),t.strarr_s[420].Get(),t.strarr_s[210].Get()) ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.grideleprof.accuracy),t.strarr_s[421].Get(),"Increases the inaccuracy of conical distribution by 1/100th of t.a degree") ; ++t.controlindex;
				if (  t.grideleprof.weaponisammo == 0 ) 
				{
					setpropertystring2(t.group,Str(t.grideleprof.reloadqty),t.strarr_s[422].Get(),t.strarr_s[212].Get()) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.fireiterations),t.strarr_s[423].Get(),t.strarr_s[213].Get()) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.range),"Range","Maximum range of bullet travel") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.dropoff),"Dropoff","Amount in inches of vertical dropoff per 100 feet of bullet travel") ; ++t.controlindex;
				}
				else
				{
					setpropertystring2(t.group,Str(t.grideleprof.lifespan),t.strarr_s[424].Get(),t.strarr_s[214].Get()) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.throwspeed),t.strarr_s[425].Get(),t.strarr_s[215].Get()) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.throwangle),t.strarr_s[426].Get(),t.strarr_s[216].Get()) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.bounceqty),t.strarr_s[427].Get(),t.strarr_s[217].Get()) ; ++t.controlindex;
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.explodeonhit),t.strarr_s[428].Get(),t.strarr_s[218].Get(),0) ; ++t.controlindex;
				}
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.usespotlighting),"Spot Lighting","Set whether emits dynamic spot lighting",0) ; ++t.controlindex;
				}
				#else
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.usespotlighting),"Spot Lighting","Set whether emits dynamic spot lighting",0) ; ++t.controlindex;
				#endif
			}

			//  Is Character
			if (  t.tflagchar == 1 ) 
			{
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					// 020316 - special check to avoid offering can take weapon if no HUD.X
					t.tfile_s = cstr("gamecore\\guns\\") + t.grideleprof.hasweapon_s + cstr("\\HUD.X");
					if ( FileExist(t.tfile_s.Get()) == 1 ) 
					{
						setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.cantakeweapon),t.strarr_s[429].Get(),t.strarr_s[219].Get(),0) ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.grideleprof.quantity),t.strarr_s[430].Get(),t.strarr_s[220].Get()) ; ++t.controlindex;
					}
					setpropertystring2(t.group,Str(t.grideleprof.rateoffire),t.strarr_s[431].Get(),t.strarr_s[221].Get()) ; ++t.controlindex;
				}
				#else
				// 020316 - special check to avoid offering can take weapon if no HUD.X
				t.tfile_s = cstr("gamecore\\guns\\") + t.grideleprof.hasweapon_s + cstr("\\HUD.X");
				if ( FileExist(t.tfile_s.Get()) == 1 ) 
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.cantakeweapon),t.strarr_s[429].Get(),t.strarr_s[219].Get(),0) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.quantity),t.strarr_s[430].Get(),t.strarr_s[220].Get()) ; ++t.controlindex;
				}
				setpropertystring2(t.group,Str(t.grideleprof.rateoffire),t.strarr_s[431].Get(),t.strarr_s[221].Get()) ; ++t.controlindex;
				#endif
			}
			if ( t.tflagquantity == 1 && g.quickparentalcontrolmode != 2 ) 
			{ 
				setpropertystring2(t.group,Str(t.grideleprof.quantity),t.strarr_s[432].Get(),t.strarr_s[222].Get())  ; ++t.controlindex; 
			}

			//  AI Extra
			if (  t.tflagvis == 1 && t.tflagai == 1 ) 
			{
				if (  t.tflagchar == 1 ) 
				{
					setpropertystring2(t.group,Str(t.grideleprof.coneangle),t.strarr_s[434].Get(),t.strarr_s[224].Get()) ; ++t.controlindex;
					#ifdef VRTECH
					setpropertystring2(t.group,Str(t.grideleprof.conerange),t.strarr_s[476].Get(),"The range within which the AI may see the player. Zero triggers the characters default range.") ; ++t.controlindex;
					#else
					setpropertystring2(t.group,Str(t.grideleprof.conerange),"View Range","The range within which the AI may see the player. Zero triggers the characters default range.") ; ++t.controlindex;
					#endif
					setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[226].Get()) ; ++t.controlindex;
					if ( g.quickparentalcontrolmode != 2 )
					{
						setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isviolent),"Blood Effects","Sets whether blood and screams should be used",0) ; ++t.controlindex;
					}
					#ifdef VRTECH
					if ( t.tflagsimpler == 0 )
					{
						setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.colondeath),"End Collision","Set to NO switches off collision when die",0) ; ++t.controlindex;
					}
					#else
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.colondeath),"End Collision","Set to NO switches off collision when die",0) ; ++t.controlindex;
					#endif
				}
				else
				{
					if (  t.tflagweap == 0 && t.tflagammo == 0 ) 
					{
						t.propfield[t.group]=t.controlindex;
						++t.group ; startgroup(t.strarr_s[435].Get()) ; t.controlindex=0;
						setpropertystring2(t.group,t.grideleprof.usekey_s.Get(),t.strarr_s[436].Get(),t.strarr_s[225].Get()) ; ++t.controlindex;
						#ifdef VRTECH
						if ( t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1 )
						{
							// only one level - no winzone chain option
						}
						else
						{
							setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[226].Get()) ; ++t.controlindex;
						}
						#else
						setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[226].Get()) ; ++t.controlindex;
						#endif
					}
				}
			}
			if (  t.tflagifused == 1 ) 
			{
				if (  t.tflagusekey == 1 ) 
				{
					setpropertystring2(t.group,t.grideleprof.usekey_s.Get(),t.strarr_s[436].Get(),t.strarr_s[225].Get()) ; ++t.controlindex;
				}
				#ifdef VRTECH
				if ( t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1 )
				{
					// only one level - no winzone chain option
				}
				else
				{
					setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[227].Get()) ; ++t.controlindex;
				}
				#else
				setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[227].Get()) ; ++t.controlindex;
				#endif
			}

			//  Spawn Settings
			if (  t.tflagspawn == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup(t.strarr_s[439].Get()) ; t.controlindex=0;
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.spawnatstart),t.strarr_s[562].Get(),t.strarr_s[563].Get(),0) ; ++t.controlindex;
				//     `setpropertystring2(group,Str(grideleprof.spawnmax),strarr$(440),strarr$(231)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnupto),strarr$(441),strarr$(232)) ; inc controlindex
				//     `setpropertylist2(group,controlindex,Str(grideleprof.spawnafterdelay),strarr$(442),strarr$(233),0) ; inc controlindex
				//     `setpropertylist2(group,controlindex,Str(grideleprof.spawnwhendead),strarr$(443),strarr$(234),0) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawndelay),strarr$(444),strarr$(235)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawndelayrandom),strarr$(564),strarr$(565)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnqty),strarr$(445),strarr$(236)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnqtyrandom),strarr$(566),strarr$(567)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnvel),strarr$(568),strarr$(569)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnvelrandom),strarr$(570),strarr$(571)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnangle),strarr$(572),strarr$(573)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnanglerandom),strarr$(574),strarr$(575)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.spawnlife),strarr$(576),strarr$(577)) ; inc controlindex
			}

			//  Visual
			//    `if tflagvis=1
			//     `propfield(group)=controlindex
			//     `inc group ; startgroup(strarr$(446)) ; controlindex=0
			//     `setpropertyfile2(group,grideleprof.texd$,strarr$(447),strarr$(237),"") ; inc controlindex
			//     `setpropertyfile2(group,grideleprof.texaltd$,strarr$(448),strarr$(238),"") ; inc controlindex
			//     `setpropertyfile2(group,grideleprof.effect$,strarr$(578),strarr$(579),"effectbank\\") ; inc controlindex
			//     `setpropertystring2(group,Str(grideleprof.transparency),strarr$(449),strarr$(240)) ; inc controlindex
			//     `setpropertystring2(group,Str(grideleprof.reducetexture),strarr$(450),strarr$(241)) ; inc controlindex
			//    `endif
			//if ( t.tflagvis == 1 ) // more engine needs improving to allow on the spot changes to shader!
			//{
			//	setpropertyfile2(t.group,t.grideleprof.effect_s.Get(),t.strarr_s[578].Get(),t.strarr_s[579].Get(),"effectbank\\"); ++t.controlindex;
			//	setpropertystring2(t.group,Str(t.grideleprof.transparency),t.strarr_s[449].Get(),t.strarr_s[240].Get()); ++t.controlindex;
			//}

			//  Statistics
			if (  (t.tflagvis == 1 || t.tflagobjective == 1 || t.tflaglives == 1 || t.tflagstats == 1) && t.tflagweap == 0 && t.tflagammo == 0 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup(t.strarr_s[451].Get()) ; t.controlindex=0;
				#ifdef VRTECH
				#else
				 if (  t.tflaglives == 1 ) { setpropertystring2(t.group,Str(t.grideleprof.lives),t.strarr_s[452].Get(),t.strarr_s[242].Get())  ; ++t.controlindex; }
				 if (  t.tflagvis == 1 || t.tflagstats == 1 ) 
				 {
					if (  t.tflaglives == 1 ) 
					{
						setpropertystring2(t.group,Str(t.grideleprof.strength),t.strarr_s[453].Get(),t.strarr_s[243].Get()) ; ++t.controlindex;
					}
					else
					{
						#ifdef VRTECH
						if ( t.tflagnotionofhealth == 1 )
						{
							setpropertystring2(t.group,Str(t.grideleprof.strength),t.strarr_s[454].Get(),t.strarr_s[244].Get()) ; ++t.controlindex;
						}
						#else
						setpropertystring2(t.group,Str(t.grideleprof.strength),t.strarr_s[454].Get(),t.strarr_s[244].Get()) ; ++t.controlindex;
						#endif
					}
					if (  t.tflagplayersettings == 1 ) 
					{
						if ( g.quickparentalcontrolmode != 2 )
						{
							setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isviolent),"Blood Effects","Sets whether blood and screams should be used",0) ; ++t.controlindex;
						}
						#ifdef VRTECH
						if ( t.tflagnotionofhealth == 1 )
						{
							setpropertystring2(t.group,Str(t.playercontrol.regenrate),"Regeneration Rate","Sets the increase value at which the players health will restore")  ; ++t.controlindex;
							setpropertystring2(t.group,Str(t.playercontrol.regenspeed),"Regeneration Speed","Sets the speed in milliseconds at which the players health will regenerate") ; ++t.controlindex;
							setpropertystring2(t.group,Str(t.playercontrol.regendelay),"Regeneration Delay","Sets the delay in milliseconds after last damage hit before health starts regenerating") ; ++t.controlindex;
						}
						#else
						setpropertystring2(t.group,Str(t.playercontrol.regenrate),"Regeneration Rate","Sets the increase value at which the players health will restore")  ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.playercontrol.regenspeed),"Regeneration Speed","Sets the speed in milliseconds at which the players health will regenerate") ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.playercontrol.regendelay),"Regeneration Delay","Sets the delay in milliseconds after last damage hit before health starts regenerating") ; ++t.controlindex;
						#endif
					}
					setpropertystring2(t.group,Str(t.grideleprof.speed),t.strarr_s[455].Get(),t.strarr_s[245].Get()) ; ++t.controlindex;
					//PE: we cant do this , as t.playercontrol.thirdperson.enabled is a global and will trigger for ALL objects.
					//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/310
//					if (  t.playercontrol.thirdperson.enabled == 1 ) 
//					{
//						t.tanimspeed_f=t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed;
//					}
//					else
//					{
						t.tanimspeed_f=t.grideleprof.animspeed;
//					}
					#ifdef VRTECH
					setpropertystring2(t.group,Str(t.tanimspeed_f),t.strarr_s[477].Get(),"Sets the default speed of any animation associated with this entity"); ++t.controlindex;
					#else
					setpropertystring2(t.group,Str(t.tanimspeed_f),"Anim Speed","Sets the default speed of any animation associated with this entity"); ++t.controlindex;
					#endif
				 }
				 if (  t.tflaghurtfall == 1 ) { setpropertystring2(t.group,Str(t.grideleprof.hurtfall),t.strarr_s[456].Get(),t.strarr_s[246].Get())  ; ++t.controlindex; }
				#endif
				if (  t.tflagplayersettings == 1 ) 
				{
					setpropertystring2(t.group,Str(t.playercontrol.jumpmax_f),"Jump Speed","Sets the jump speed of the player which controls overall jump height") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.gravity_f),"Gravity","Sets the modified force percentage of the players own gravity") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.fallspeed_f),"Fall Speed","Sets the maximum speed percentage at which the player will fall") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.climbangle_f),"Climb Angle","Sets the maximum angle permitted for the player to ascend a slope") ; ++t.controlindex;
					if (  t.playercontrol.thirdperson.enabled == 0 ) 
					{
						setpropertystring2(t.group,Str(t.playercontrol.wobblespeed_f),"Wobble Speed","Sets the rate of motion applied to the camera when moving") ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.playercontrol.wobbleheight_f*100),"Wobble Height","Sets the degree of motion applied to the camera when moving") ; ++t.controlindex;
						setpropertystring2(t.group, Str(t.playercontrol.footfallpace_f * 100), "Footfall Pace", "Sets the rate at which the footfall sound is played when moving"); ++t.controlindex;
						setpropertystring2(t.group, Str(t.playercontrol.footfallvolume_f * 100), "Footfall Volume", "Sets the percentage volume at which the footfall sound is played"); ++t.controlindex;
					}
					setpropertystring2(t.group,Str(t.playercontrol.accel_f*100),"Acceleration","Sets the acceleration curve used when t.moving from t.a stood position") ; ++t.controlindex;
				}
				if ( t.tflagmobile == 1 ) { setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isimmobile),t.strarr_s[457].Get(),t.strarr_s[247].Get(),0); ++t.controlindex; }
				if ( t.tflagmobile == 1 ) 
				{ 
					#ifdef VRTECH
					if ( t.tflagsimpler == 0 )
					{
						setpropertystring2(t.group,Str(t.grideleprof.lodmodifier),"LOD Modifier","Modify when the LOD transition takes effect. The default value is 0, increase this to a percentage reduce the LOD effect.") ; ++t.controlindex; 
					}
					#else
					setpropertystring2(t.group,Str(t.grideleprof.lodmodifier),"LOD Modifier","Modify when the LOD transition takes effect. The default value is 0, increase this to a percentage reduce the LOD effect.") ; ++t.controlindex; 
					#endif
				}
			}

			//  Team field
			#ifdef PHOTONMP
			#else
			if (  t.tflagteamfield == 1 ) 
			{
				setpropertylist3(t.group,t.controlindex,Str(t.grideleprof.teamfield),"Team","Specifies any team affiliation for multiplayer start marker",0) ; ++t.controlindex;
			}
			#endif

			//  Physics Data (non-multiplayer)
			if (  t.entityprofile[t.gridentity].ismarker == 0 && t.entityprofile[t.gridentity].islightmarker == 0 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup(t.strarr_s[596].Get()) ; t.controlindex=0;
				if (  t.grideleprof.physics != 1  )  t.grideleprof.physics = 0;
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.physics),t.strarr_s[580].Get(),t.strarr_s[581].Get(),0) ; ++t.controlindex;
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.phyalways),t.strarr_s[582].Get(),t.strarr_s[583].Get(),0) ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.grideleprof.phyweight),t.strarr_s[584].Get(),t.strarr_s[585].Get()) ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.grideleprof.phyfriction),t.strarr_s[586].Get(),t.strarr_s[587].Get()) ; ++t.controlindex;
				//     `setpropertystring2(group,Str(grideleprof.phyforcedamage),strarr$(588),strarr$(589)) ; inc controlindex
				//     `setpropertystring2(group,Str(grideleprof.rotatethrow),strarr$(590),strarr$(591)) ; inc controlindex
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.explodable),t.strarr_s[592].Get(),t.strarr_s[593].Get(),0) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.explodedamage),t.strarr_s[594].Get(),t.strarr_s[595].Get()) ; ++t.controlindex;
				}
				#else
				setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.explodable),t.strarr_s[592].Get(),t.strarr_s[593].Get(),0) ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.grideleprof.explodedamage),t.strarr_s[594].Get(),t.strarr_s[595].Get()) ; ++t.controlindex;
				#endif
			}

			//  Ammo data (FPGC - 280809 - filtered fpgcgenre=1 is shooter genre
			if (  g.fpgcgenre == 1 ) 
			{
				if (  t.tflagammo == 1 || t.tflagammoclip == 1 ) 
				{
					t.propfield[t.group]=t.controlindex;
					++t.group ; startgroup(t.strarr_s[459].Get()) ; t.controlindex=0;
					setpropertystring2(t.group,Str(t.grideleprof.quantity),t.strarr_s[460].Get(),t.strarr_s[249].Get()) ; ++t.controlindex;
				}
			}

			//  Light data
			if (  t.tflaglight == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup(t.strarr_s[461].Get()) ; t.controlindex=0; //PE: 461=Light
				setpropertystring2(t.group,Str(t.grideleprof.light.range),t.strarr_s[462].Get(),t.strarr_s[250].Get()) ; ++t.controlindex; //PE: 462=Light Range
				setpropertycolor2(t.group,t.grideleprof.light.color,t.strarr_s[463].Get(),t.strarr_s[251].Get()) ; ++t.controlindex; //PE: 463=Light Color
				#ifdef VRTECH
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.usespotlighting), "Spot Lighting", "Change dynamic light to spot lighting", 0); ++t.controlindex;
				}
				#else
				setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.usespotlighting), "Spot Lighting", "Change dynamic light to spot lighting", 0); ++t.controlindex;
				#endif
			}

			//  Decal data
			if (  t.tflagtdecal == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;

				//  FPGC - 300710 - could never change base decal, so comment out this property (entity denotes decal choice)
				//     `inc group ; startgroup(strarr$(464)) ; controlindex=0
				//     `setpropertyfile2(group,grideleprof.basedecal$,strarr$(465),strarr$(252),"gamecore\\decals\\") ; inc controlindex

				//  Decal Particle data
				if (  t.tflagdecalparticle == 1 ) 
				{
					++t.group ; startgroup("Decal Particle") ; t.controlindex=0;
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.particleoverride),"Custom Settings","Whether you wish to override default settings",0) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.offsety),"OffsetY","Vertical adjustment of start position") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.scale),"Scale","A value from 0 to 100, denoting size of particle") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randomstartx),"Random Start X","Random start area") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randomstarty),"Random Start Y","Random start area") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randomstartz),"Random Start Z","Random start area") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.linearmotionx),"Linear Motion X","Constant motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.linearmotiony),"Linear Motion Y","Constant motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.linearmotionz),"Linear Motion Z","Constant motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randommotionx),"Random Motion X","Random motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randommotiony),"Random Motion Y","Random motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.randommotionz),"Random Motion Z","Random motion direction") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.mirrormode),"Mirror Mode","Set to one to reverse the particle") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.camerazshift),"Camera Z Shift","Shift t.particle towards camera") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.scaleonlyx),"Scale Only X","Percentage X over Y scale") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.lifeincrement),"Life Increment","Control lifespan of particle") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.particle.alphaintensity),"Alpha Intensity","Control alpha percentage of particle") ; ++t.controlindex;
					//  V118 - 060810 - knxrb - Decal animation setting (Added animation choice setting).
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.particle.animated),"Animated Text (  ( ure","Sets whether the t.particle t.decal Texture is animated or static.", 0)  ; ++t.controlindex;
				}
			}

			// Sound
			if ( t.tflagsound == 1 || t.tflagsoundset == 1 || tflagtext == 1 || tflagimage == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ;
				if ( tflagtext == 1 || tflagimage == 1 )
				{
					if ( tflagtext == 1 ) startgroup("Text");
					if ( tflagimage == 1 ) startgroup("Image");
				}
				else
				{
					startgroup("Media");
				}
				t.controlindex=0;
				if ( g.fpgcgenre == 1 ) 
				{
					if ( g.vrqcontrolmode != 0 )
					{
						#ifdef VRTECH
						if ( t.tflagsound == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[253].Get(),"audiobank\\")  ; ++t.controlindex; }
						#else
						if ( t.tflagsound == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),"Audio",t.strarr_s[253].Get(),"audiobank\\")  ; ++t.controlindex; }
						#endif
					}
					else
					{
						if ( t.tflagsound == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[467].Get(),t.strarr_s[253].Get(),"audiobank\\")  ; ++t.controlindex; }
					}
					if ( t.tflagsoundset == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[255].Get(),"audiobank\\voices\\")  ; ++t.controlindex; }
					if ( tflagtext == 1 ) { setpropertystring2(t.group,t.grideleprof.soundset_s.Get(),"Text to Appear","Enter text to appear in-level") ; ++t.controlindex; }
					if ( tflagimage == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),"Image File","Select image to appear in-level","scriptbank\\images\\imagesinzone\\") ; ++t.controlindex; }
					if ( t.tflagnosecond == 0 ) 
					{
						if ( t.tflagsound == 1 || t.tflagsoundset == 1 )
						{ 
							setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),t.strarr_s[468].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							#ifdef VRTECH
							setpropertyfile2(t.group,t.grideleprof.soundset2_s.Get(),t.strarr_s[480].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset3_s.Get(),t.strarr_s[481].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset4_s.Get(),t.strarr_s[482].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							#else
							setpropertyfile2(t.group,t.grideleprof.soundset2_s.Get(),"Sound2",t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset3_s.Get(),"Sound3",t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset4_s.Get(),"Sound4",t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							#endif
						}
					}
				}
				else
				{
					if ( t.tflagsoundset == 1 ) 
					{
						setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[255].Get(),"audiobank\\voices\\") ; ++t.controlindex;
					}
					else
					{
						setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[467].Get(),t.strarr_s[253].Get(),"audiobank\\") ; ++t.controlindex;
					}
					setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),t.strarr_s[468].Get(),t.strarr_s[254].Get(),"audiobank\\") ; ++t.controlindex;
				}
			}

			// Video
			if ( t.tflagvideo == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup(t.strarr_s[597].Get()) ; t.controlindex=0;
				#ifdef VRTECH
				setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[599].Get(),"audiobank\\") ; ++t.controlindex;
				setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),"Video Slot",t.strarr_s[601].Get(),"videobank\\") ; ++t.controlindex;
				#else
				setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),"Audio",t.strarr_s[599].Get(),"audiobank\\") ; ++t.controlindex;
				setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),"Video",t.strarr_s[601].Get(),"videobank\\") ; ++t.controlindex;
				#endif
			}

			//  Third person settings
			if (  t.tflagplayersettings == 1 && t.playercontrol.thirdperson.enabled == 1 ) 
			{
				t.propfield[t.group]=t.controlindex;
				++t.group ; startgroup("Third Person") ; t.controlindex=0;
				t.livegroupforthirdperson=t.group;
				setpropertylist2(t.group,t.controlindex,Str(t.playercontrol.thirdperson.cameralocked),"Camera Locked","Fixes camera height and angle for third person view",0) ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.playercontrol.thirdperson.cameradistance),"Camera Distance","Sets the distance of the third person camera") ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.playercontrol.thirdperson.camerashoulder),"Camera X Offset","Sets the distance to shift the camera over shoulder") ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.playercontrol.thirdperson.cameraheight),"Camera Y Offset","Sets the vertical height of the third person camera. If more than twice the camera distance, camera collision disables") ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.playercontrol.thirdperson.camerafocus),"Camera Focus","Sets the camera X angle offset to align focus of the third person camera") ; ++t.controlindex;
				setpropertystring2(t.group,Str(t.playercontrol.thirdperson.cameraspeed),"Camera Speed","Sets the retraction speed percentage of the third person camera") ; ++t.controlindex;
				setpropertylist2(t.group,t.controlindex,Str(t.playercontrol.thirdperson.camerafollow),"Run Mode","If set to yes, protagonist uses WASD t.movement mode",0) ; ++t.controlindex;
				setpropertylist2(t.group,t.controlindex,Str(t.playercontrol.thirdperson.camerareticle),"Show Reticle","Show the third person 'crosshair' reticle Dot ( ",0)  ; ++t.controlindex;
			}

		}

		//  End of data
		t.propfield[t.group]=t.controlindex;
		t.propfieldgroupmax=t.group;

		//  FPGC - 070510 - finish bulk entity properties population
		SetFileMapDWORD (  3,g.g_filemapoffset,0  ); g.g_filemapoffset += 4;
		SetFileMapDWORD (  3,0,1 );

	}

	//  FPGC - 070510 - close bulk file map
	SetEventAndWait ( 2 );
	#endif
}

void interface_copydatatoentity ( void )
{
	//  go through all active fields
	for ( t.iGroup = 0 ; t.iGroup<=  t.propfieldgroupmax; t.iGroup++ )
	{
		for ( t.iControl = 0 ; t.iControl<=  t.propfield[t.iGroup]-1; t.iControl++ )
		{

			//  Get data
			t.tfield_s = getpropertyfield(t.iGroup,t.iControl);
			t.tdata_s = getpropertydata(t.iGroup,t.iControl);

			//  If tdata$ was absolute file, truncate to remove first part
			if ( t.tdata_s.Get()[1] == ':' )
			{
				t.chopthis_s=g.rootdir_s;
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[413].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				#ifdef VRTECH
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				#endif
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[416].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[561].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[417].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[418].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[433].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[464].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"gamecore\\decals\\";
				LPSTR pPreferredFolder = "audiobank\\voices\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  
				{
					// 151016 - determine if data points to voices folder
					if ( strnicmp ( t.tdata_s.Get() + strlen(t.chopthis_s.Get()), pPreferredFolder, strlen(pPreferredFolder) ) == NULL )
					{
						// default soundset entry into voices folder
						t.chopthis_s = t.chopthis_s + "audiobank\\voices\\";
					}
					else
					{
						// allow normal WAV sounds to be placed in character SoundSet slot (zombies)
						t.chopthis_s = t.chopthis_s;
						pPreferredFolder = NULL;
					}
				}
				t.tdata_s=Right(t.tdata_s.Get(),Len(t.tdata_s.Get())-Len(t.chopthis_s.Get()));
				if ( cstr(Lower(t.tfield_s.Get())) == cstr(Lower(t.strarr_s[464].Get())) 
				||	(cstr(Lower(t.tfield_s.Get())) == cstr(Lower(t.strarr_s[469].Get())) && pPreferredFolder != NULL) ) 
				{
					//  get path (folder name) only
					t.tdata_s=getpath(t.tdata_s.Get()) ; t.tdata_s=Left(t.tdata_s.Get(),Len(t.tdata_s.Get())-1);
				}
			}

			//  All YES and NO strings are auto converted if value expected
			t.tokay=1;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[413].Get()) ) == 0 )  t.tokay = 0;
			#ifdef VRTECH
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.tokay = 0;
			#endif
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[436].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[437].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[464].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.tokay = 0;
			if (  t.tokay == 1 ) 
			{
				if (  strcmp ( Lower(t.tdata_s.Get()) , Lower(t.strarr_s[470].Get()) ) == 0 )  t.tdata_s = "1";
				if (  strcmp ( Lower(t.tdata_s.Get()) , Lower(t.strarr_s[471].Get()) ) == 0 )  t.tdata_s = "0";
				if (  strcmp ( Lower(t.tdata_s.Get()) , Lower("no") ) == 0 )  t.tdata_s = "0";
				if (  strcmp ( Lower(t.tdata_s.Get()) , Lower("a") ) == 0 )  t.tdata_s = "1";
				if (  strcmp ( Lower(t.tdata_s.Get()) , Lower("b") ) == 0 )  t.tdata_s = "2";
				if (strcmp (Lower(t.tdata_s.Get()), Lower("Shadow and Reflect")) == 0)  t.tdata_s = "0";
				if (strcmp (Lower(t.tdata_s.Get()), Lower("No Shadow No Reflect")) == 0)  t.tdata_s = "1";
				if (strcmp (Lower(t.tdata_s.Get()), Lower("No Shadow and Reflect")) == 0)  t.tdata_s = "2";		
			}

			//  FPGC - 070510 - add behaviour folder back, along with FPI (from combo friendly name to script filename)
			if (  cstr(Lower(t.tfield_s.Get())) == "behaviour" ) 
			{
				t.tdata_s = ""; t.tdata_s=t.tdata_s+"behaviours\\"+t.tdata_s+".fpi";
			}

			//  Clipped alternative
			t.tdataclipped_s=Left(t.tdata_s.Get(),63);

			//  get field data
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[413].Get()) ) == 0 )  t.grideleprof.name_s = t.tdataclipped_s;
			#ifdef VRTECH
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.grideleprof.name_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.grideleprof.name_s = t.tdataclipped_s;
			#endif
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[414].Get()) ) == 0 )  t.gridentitystaticmode = ValF(t.tdata_s.Get());

			//LB: set whether the entity can reflect in the water
			if (strcmp(Lower(t.tfield_s.Get()), Lower("Shadows and Reflection")) == 0)
			{
				int iReflectUsingCastVar = ValF(t.tdata_s.Get());
				if (iReflectUsingCastVar == 0) t.grideleprof.castshadow = 0;
				if (iReflectUsingCastVar == 1) t.grideleprof.castshadow = -1;
				if (iReflectUsingCastVar == 2) t.grideleprof.castshadow = -2;
			}

			// if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[561].Get()) ) == 0 )  t.grideleprof.aiinit_s = t.tdataclipped_s; //PE: Not used anymore.
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[417].Get()) ) == 0 )  t.grideleprof.aimain_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , "behaviour" ) == 0 )  t.grideleprof.aimain_s = t.tdataclipped_s;
			//if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[418].Get()) ) == 0 )  t.grideleprof.aidestroy_s = t.tdataclipped_s;  //PE: Not used anymore.
			//if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[433].Get()) ) == 0 )  t.grideleprof.aishoot_s = t.tdataclipped_s; //PE: Not used anymore.
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[434].Get()) ) == 0 )  t.grideleprof.coneangle = ValF(t.tdata_s.Get());
			#ifdef VRTECH
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[476].Get()) ) == 0 )  t.grideleprof.conerange = ValF(t.tdata_s.Get());
			#else
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("View Range") ) == 0 )  t.grideleprof.conerange = ValF(t.tdata_s.Get());
			#endif
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[419].Get()) ) == 0 )  t.grideleprof.hasweapon_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[436].Get()) ) == 0 )  t.grideleprof.usekey_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[437].Get()) ) == 0 )  t.grideleprof.ifused_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[562].Get()) ) == 0 )  t.grideleprof.spawnatstart = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[440].Get()) ) == 0 )  t.grideleprof.spawnmax = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[441].Get()) ) == 0 )  t.grideleprof.spawnupto = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[442].Get()) ) == 0 )  t.grideleprof.spawnafterdelay = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[443].Get()) ) == 0 )  t.grideleprof.spawnwhendead = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[444].Get()) ) == 0 )  t.grideleprof.spawndelay = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[445].Get()) ) == 0 )  t.grideleprof.spawnqty = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[564].Get()) ) == 0 )  t.grideleprof.spawndelayrandom = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[566].Get()) ) == 0 )  t.grideleprof.spawnqtyrandom = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[568].Get()) ) == 0 )  t.grideleprof.spawnvel = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[570].Get()) ) == 0 )  t.grideleprof.spawnvelrandom = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[572].Get()) ) == 0 )  t.grideleprof.spawnangle = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[574].Get()) ) == 0 )  t.grideleprof.spawnanglerandom = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[576].Get()) ) == 0 )  t.grideleprof.spawnlife = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[447].Get()) ) == 0 )  t.grideleprof.texd_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[448].Get()) ) == 0 )  t.grideleprof.texaltd_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[578].Get()) ) == 0 )  t.grideleprof.effect_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[449].Get()) ) == 0 )  t.grideleprof.transparency = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[450].Get()) ) == 0 )  t.grideleprof.reducetexture = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[454].Get()) ) == 0 )  t.grideleprof.strength = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[453].Get()) ) == 0 )  t.grideleprof.strength = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[457].Get()) ) == 0 )  t.grideleprof.isimmobile = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("LOD Modifier") ) == 0 ) 
			{
				// 301115 - new LOD Modifie value for this entity parent, so propagate to ALL other entities of this parent
				t.grideleprof.lodmodifier = ValF(t.tdata_s.Get());
				int iThisBankIndex = t.gridentity;
				if ( t.entityprofile[iThisBankIndex].addhandlelimb==0 )
				{
					for ( int e=1; e<=g.entityelementlist; e++ )
					{
						if ( t.entityelement[e].bankindex==iThisBankIndex )
						{
							t.entityelement[e].eleprof.lodmodifier = t.grideleprof.lodmodifier;
							entity_calculateentityLODdistances ( iThisBankIndex, t.entityelement[e].obj, t.entityelement[e].eleprof.lodmodifier );
						}
					}
					int iParentSrcObj = g.entitybankoffset + iThisBankIndex;
					entity_calculateentityLODdistances ( iThisBankIndex, iParentSrcObj, t.grideleprof.lodmodifier );
				}
			}
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Occluder") ) == 0 )  t.grideleprof.isocluder = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Occludee") ) == 0 )  t.grideleprof.isocludee = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Specular") ) == 0 )  t.grideleprof.specularperc = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("End Collision") ) == 0 )  t.grideleprof.colondeath = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Parent Index") ) == 0 )  t.grideleprof.parententityindex = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Parent Limb") ) == 0 )  t.grideleprof.parentlimbindex = ValF(t.tdata_s.Get());

			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[429].Get()) ) == 0 )  t.grideleprof.cantakeweapon = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[430].Get()) ) == 0 )  t.grideleprof.quantity = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[431].Get()) ) == 0 )  t.grideleprof.rateoffire = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[420].Get()) ) == 0 )  t.grideleprof.damage = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[421].Get()) ) == 0 )  t.grideleprof.accuracy = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[422].Get()) ) == 0 )  t.grideleprof.reloadqty = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[423].Get()) ) == 0 )  t.grideleprof.fireiterations = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Range") ) == 0 )  t.grideleprof.range = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Dropoff") ) == 0 )  t.grideleprof.dropoff = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower("Spot Lighting") ) == 0 )  t.grideleprof.usespotlighting = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[424].Get()) ) == 0 )  t.grideleprof.lifespan = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[425].Get()) ) == 0 )  t.grideleprof.throwspeed = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[426].Get()) ) == 0 )  t.grideleprof.throwangle = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[427].Get()) ) == 0 )  t.grideleprof.bounceqty = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[428].Get()) ) == 0 )  t.grideleprof.explodeonhit = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[455].Get()) ) == 0 )  t.grideleprof.speed = ValF(t.tdata_s.Get());
			#ifdef VRQUEST
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[477].Get()) ) == 0 ) 
			{
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed=ValF(t.tdata_s.Get());
				}
				else
				{
					t.grideleprof.animspeed=ValF(t.tdata_s.Get());
				}
			}
			#else
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Anim Speed") ) == 0 ) 
			{
				//PE: we cant do this , as t.playercontrol.thirdperson.enabled is a global and will trigger for ALL objects.
				//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/310
//				if (  t.playercontrol.thirdperson.enabled == 1 ) 
//				{
//					t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed=ValF(t.tdata_s.Get());
//				}
//				else
//				{
					t.grideleprof.animspeed=ValF(t.tdata_s.Get());
//				}
			}
			#endif
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[432].Get()) ) == 0 )  t.grideleprof.quantity = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[460].Get()) ) == 0 )  t.grideleprof.quantity = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[452].Get()) ) == 0 )  t.grideleprof.lives = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[456].Get()) ) == 0 )  t.grideleprof.hurtfall = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Blood Effects")  ) == 0 ) t.grideleprof.isviolent = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Regeneration Rate")  ) == 0 ) t.playercontrol.regenrate = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Regeneration Speed")  ) == 0 ) t.playercontrol.regenspeed = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Regeneration Delay")  ) == 0 ) t.playercontrol.regendelay = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Jump Speed")  ) == 0 ) t.playercontrol.jumpmax_f = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Gravity")  ) == 0 ) t.playercontrol.gravity_f = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Fall Speed")  ) == 0 ) t.playercontrol.fallspeed_f = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Climb Angle")  ) == 0 ) t.playercontrol.climbangle_f = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Wobble Speed")  ) == 0 ) t.playercontrol.wobblespeed_f = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Wobble Height")  ) == 0 ) t.playercontrol.wobbleheight_f = ValF(t.tdata_s.Get())/100.0;
			if (strcmp (Lower(t.tfield_s.Get()), Lower("Footfall Pace")) == 0) t.playercontrol.footfallpace_f = ValF(t.tdata_s.Get()) / 100.0;
			if (strcmp (Lower(t.tfield_s.Get()), Lower("Footfall Volume")) == 0) t.playercontrol.footfallvolume_f = ValF(t.tdata_s.Get()) / 100.0;	
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Acceleration")  ) == 0 ) t.playercontrol.accel_f = ValF(t.tdata_s.Get())/100.0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera Locked")  ) == 0 ) t.playercontrol.thirdperson.cameralocked = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera Distance")  ) == 0 ) t.playercontrol.thirdperson.cameradistance = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera Y Offset")  ) == 0 ) t.playercontrol.thirdperson.cameraheight = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera Focus")  ) == 0 ) t.playercontrol.thirdperson.camerafocus = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera Speed")  ) == 0 ) t.playercontrol.thirdperson.cameraspeed = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Camera X Offset")  ) == 0 ) t.playercontrol.thirdperson.camerashoulder = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Run Mode")  ) == 0 ) t.playercontrol.thirdperson.camerafollow = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Show Reticle")  ) == 0 ) t.playercontrol.thirdperson.camerareticle = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[458].Get()) ) == 0 )  t.grideleprof.isobjective = ValF(t.tdata_s.Get());
			// if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[464].Get()) ) == 0 )  t.grideleprof.basedecal_s = t.tdataclipped_s; //PE: Not used anymore.

			//  FPGC - 300710 - read data changes back into grideleprof
			if (  strcmp ( Lower(t.tfield_s.Get()) , "custom settings"  ) == 0 ) t.grideleprof.particleoverride = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "offsety"  ) == 0 )  t.grideleprof.particle.offsety = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "scale"  ) == 0 )  t.grideleprof.particle.scale = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random start x"  ) == 0 )  t.grideleprof.particle.randomstartx = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random start y"  ) == 0 )  t.grideleprof.particle.randomstarty = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random start z"  ) == 0 )  t.grideleprof.particle.randomstartz = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "linear motion x"  ) == 0 )  t.grideleprof.particle.linearmotionx = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "linear motion y"  ) == 0 )  t.grideleprof.particle.linearmotiony = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "linear motion z"  ) == 0 )  t.grideleprof.particle.linearmotionz = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random motion x"  ) == 0 )  t.grideleprof.particle.randommotionx = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random motion y"  ) == 0 )  t.grideleprof.particle.randommotiony = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "random motion z"  ) == 0 )  t.grideleprof.particle.randommotionz = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "mirror mode"  ) == 0 )  t.grideleprof.particle.mirrormode = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "camera z shift"  ) == 0 )  t.grideleprof.particle.camerazshift = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "scale only x"  ) == 0 )  t.grideleprof.particle.scaleonlyx = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "life increment"  ) == 0 )  t.grideleprof.particle.lifeincrement = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "alpha intensity"  ) == 0 )  t.grideleprof.particle.alphaintensity = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , "animated texture"  ) == 0 )  t.grideleprof.particle.animated , ValF(t.tdata_s.Get()) ;

			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[467].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[468].Get()) ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;
			#ifdef VRTECH
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[480].Get()) ) == 0 )  t.grideleprof.soundset2_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[481].Get()) ) == 0 )  t.grideleprof.soundset3_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[482].Get()) ) == 0 )  t.grideleprof.soundset4_s = t.tdataclipped_s;
			#else
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Sound2") ) == 0 )  t.grideleprof.soundset2_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Sound3") ) == 0 )  t.grideleprof.soundset3_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Sound4") ) == 0 )  t.grideleprof.soundset4_s = t.tdataclipped_s;
			#endif
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[598].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[600].Get()) ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , "voiceover"  ) == 0 ) t.grideleprof.soundset1_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[462].Get()) ) == 0 )  t.grideleprof.light.range = ValF(t.tdata_s.Get());
			#ifdef VRTECH
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Text to Appear") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Image File") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Video Slot") ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;
			#else
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Text String") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Image File") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Audio") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Video") ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;
			#endif

			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[580].Get()) ) == 0 )  t.grideleprof.physics = ValF(t.tdata_s.Get());
			if (  t.grideleprof.physics != 1  )  t.grideleprof.physics = 2;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[582].Get()) ) == 0 )  t.grideleprof.phyalways = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[584].Get()) ) == 0 )  t.grideleprof.phyweight = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[586].Get()) ) == 0 )  t.grideleprof.phyfriction = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[588].Get()) ) == 0 )  t.grideleprof.phyforcedamage = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[590].Get()) ) == 0 )  t.grideleprof.rotatethrow = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[592].Get()) ) == 0 )  t.grideleprof.explodable = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[594].Get()) ) == 0 )  t.grideleprof.explodedamage = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("team")  ) == 0 )  t.grideleprof.teamfield = ValF(t.tdata_s.Get());

			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[463].Get()) ) == 0 ) 
			{
				t.tr_s=t.tdata_s;
				for ( t.t = 1 ; t.t<=  Len(t.tr_s.Get()); t.t++ )
				{
					if (  t.tr_s.Get()[t.t-1] == ' ' ) { t.tr_s = Left(t.tr_s.Get(),t.t) ; break; }
				}
				t.tdata_s=Right(t.tdata_s.Get(),(Len(t.tdata_s.Get())-Len(t.tr_s.Get())));
				t.tg_s=t.tdata_s;
				for ( t.t = 1 ; t.t<=  Len(t.tg_s.Get()); t.t++ )
				{
					if (  t.tg_s.Get()[t.t-1] == ' ' ) { t.tg_s = Left(t.tg_s.Get(),t.t)  ; break; }
				}
				t.tb_s=Right(t.tdata_s.Get(),(Len(t.tdata_s.Get())-Len(t.tg_s.Get())));
				t.grideleprof.light.color=Rgb(ValF(t.tr_s.Get()),ValF(t.tg_s.Get()),ValF(t.tb_s.Get()));
			}

		}
	}
}

void interface_closepropertywindow ( void )
{
	//  Close proprty window
	#ifdef FPSEXCHANGE
	if (  t.editorinterfaceactive>0 )
	{
		//  Close dialog
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 978, 2 );
		SetFileMapDWORD (  1, 462, 0 );
		SetEventAndWait (  1 );
		t.editorinterfaceactive=0;
	}
	#endif
}

void interface_handlepropertywindow ( void )
{
	#ifdef FPSEXCHANGE
	//  If interface active
	if (  t.editorinterfaceactive>0 ) 
	{
		//  Open for management
		OpenFileMap (  2, "FPSENTITY" );
		SetEventAndWait (  2 );

		//  if APPLY clicked, copy data to entity
		if (  GetFileMapDWORD( 2, 112 ) == 1 ) 
		{
			interface_copydatatoentity ( );
			SetFileMapDWORD (  2, 112, 0 );
			SetEventAndWait (  2 );
			t.editorinterfaceleave=1;
			t.interactive.applychangesused=1;
		}

		//  see if the user clicked on the close button
		if (  GetFileMapDWORD( 2, 108 )  ==  1 ) 
		{
			SetFileMapDWORD (  2, 108, 0 );
			SetEventAndWait (  2 );
			t.editorinterfaceleave=1;
		}

		//  see if the user clicked on the CANCEL button
		if (  GetFileMapDWORD( 2, 116 )  ==  1 ) 
		{
			SetFileMapDWORD (  2, 116, 0 );
			SetEventAndWait (  2 );
			t.editorinterfaceleave=1;
		}
	}
	#endif
}

void interface_live_updates(void)
{
	#ifdef FPSEXCHANGE
	//  constantly open access to properties values
	//  so can represent the values prior to using APPLY CHANGES
	if (Timer() > t.lastliveupdatestimer)
	{
		t.lastliveupdatestimer = Timer() + 200;
		OpenFileMap(2, "FPSENTITY");
		SetEventAndWait(2);
		t.iGroup = t.livegroupforthirdperson;
		t.iControl = 1; t.tfield_s = getpropertyfield(t.iGroup, t.iControl); t.tdata_s = getpropertydata(t.iGroup, t.iControl);
		if (cstr(Lower(t.tfield_s.Get())) == Lower("Camera Distance"))  t.playercontrol.thirdperson.livecameradistance = ValF(t.tdata_s.Get());
		t.iControl = 2; t.tfield_s = getpropertyfield(t.iGroup, t.iControl); t.tdata_s = getpropertydata(t.iGroup, t.iControl);
		if (cstr(Lower(t.tfield_s.Get())) == Lower("Camera X Offset"))  t.playercontrol.thirdperson.livecamerashoulder = ValF(t.tdata_s.Get());
		t.iControl = 3; t.tfield_s = getpropertyfield(t.iGroup, t.iControl); t.tdata_s = getpropertydata(t.iGroup, t.iControl);
		if (cstr(Lower(t.tfield_s.Get())) == Lower("Camera Y Offset"))  t.playercontrol.thirdperson.livecameraheight = ValF(t.tdata_s.Get());
		t.iControl = 4; t.tfield_s = getpropertyfield(t.iGroup, t.iControl); t.tdata_s = getpropertydata(t.iGroup, t.iControl);
		if (cstr(Lower(t.tfield_s.Get())) == Lower("Camera Focus"))  t.playercontrol.thirdperson.livecamerafocus = ValF(t.tdata_s.Get());
		//CloseFileMap (  2 );
	}
	#endif
}

// 
//  Interface Properties Functions
// 

#ifdef ENABLEIMGUI
char* imgui_setpropertyfile2_ex_dlua(int group, char* data_s, char* field_s, char* desc_s, char* within_s, int* piEditedField, char* pButtonControlIfBlocked )
{
	char *cRet;
	cstr ldata_s = data_s, ldesc_s = desc_s, lfields_s = field_s, lwithin_s = within_s;

	if (cstr(data_s) == "" || !data_s)  ldata_s = "";
	if (cstr(desc_s) == "" || !desc_s)  ldesc_s = "";
	if (cstr(field_s) == "" || !field_s)  lfields_s = "";
	if (cstr(within_s) == "" || !within_s)  lwithin_s = "";

	std::string uniquiField = "";
	uniquiField = uniquiField + "##" + lfields_s.Get();
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

	if (lfields_s != "") {
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
		ImGui::Text(lfields_s.Get());
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
		ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
	}
	strcpy(cTmpInput, ldata_s.Get());

	ImGui::PushItemWidth(-10 - ((ImGui::GetFontSize()*2.0)*3.0) - 2  ); //-6 padding.

	ImGui::InputText(uniquiField.c_str(), &cTmpInput[0], MAXTEXTINPUT);
	if (!pref.iTurnOffEditboxTooltip && ImGui::IsItemHovered() && ldesc_s != "") ImGui::SetTooltip("%s", ldesc_s.Get());
	if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;

	ImGui::PopItemWidth();
	ImGui::SameLine();

	uniquiField = "...";
	uniquiField = uniquiField + "##";
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

	ImGui::PushItemWidth(ImGui::GetFontSize()*2.0);

	bool bAudio = false;
	bool bImage = false;
	bool bVideo = false;
	bool bScript = false;
	bool bParticle = false;
	bool bUseNewAudioWindow = false;
#ifdef USENEWMEDIASELECTWINDOWS
	//bUseNewAudioWindow = true;
	if (pestrcasestr(lwithin_s.Get(), "audiobank"))
	{
		bUseNewAudioWindow = true;
		bAudio = true;
	}
	if (pestrcasestr(lwithin_s.Get(), "\\imagesinzone"))
	{
		bUseNewAudioWindow = true;
		bImage = true;
	}
	if (pestrcasestr(lwithin_s.Get(), "imagebank"))
	{
		bUseNewAudioWindow = true;
		bImage = true;
	}
	if (pestrcasestr(lwithin_s.Get(), "videobank"))
	{
		bUseNewAudioWindow = true;
		bVideo = true;
	}
	if (pestrcasestr(lwithin_s.Get(), "scriptbank"))
	{
		bUseNewAudioWindow = true;
		bScript = true;
	}
	if (pestrcasestr(lwithin_s.Get(), "particlesbank"))
	{
		bUseNewAudioWindow = true;
		bParticle = true;
	}

#endif

	if (bUseNewAudioWindow )
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		bool bProceed = true;
		if (piEditedField)
		{
			if (*piEditedField == 2)
			{
				bProceed = false;
			}
		}
		if (!bProceed)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::StyleButton(uniquiField.c_str(), ImVec2(ImGui::GetFontSize()*1.48, 0)) || iSelectedLibraryStingReturnID == window->GetID(uniquiField.c_str()))
		{
			if (bProceed == true)
			{
				cStr tOldDir = GetDir();
				if (iSelectedLibraryStingReturnID == window->GetID(uniquiField.c_str()))
				{
					//Update selected var.
					//MessageBoxA(NULL, sSelectedLibrarySting.Get(), "ReturnVar", 0);

					char * cFileSelected = sSelectedLibrarySting.Get();

					SetDir(tOldDir.Get());

					if (cFileSelected && strlen(cFileSelected) > 0)
					{
						if (piEditedField) *piEditedField = 1;
						std::string relative = cFileSelected;
						std::string fullpath = tOldDir.Get();
						fullpath += "\\";
						if (pestrcasestr(lwithin_s.Get(), "scriptbank"))
							fullpath += lwithin_s.Get();
						replaceAll(relative, fullpath, "");
						strcpy(cTmpInput, relative.c_str());
					}

					iSelectedLibraryStingReturnID = -1; //disable.
					sSelectedLibrarySting = "";
				}
				else
				{
					bExternal_Entities_Window = true;
					iDisplayLibraryType = 0;
					if (bAudio)
						iDisplayLibraryType = 1;
					if (bImage)
						iDisplayLibraryType = 2;
					if (bVideo)
						iDisplayLibraryType = 3;
					if (bScript)
						iDisplayLibraryType = 4;
					if(bParticle)
						iDisplayLibraryType = 5;

					iLibraryStingReturnToID = window->GetID(uniquiField.c_str());
				}
			}
		}

		if (!bProceed)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

	}
	else
	{
		if (ImGui::StyleButton(uniquiField.c_str(), ImVec2(ImGui::GetFontSize()*1.48, 0)))
		{
			bool bProceed = true;
			if (piEditedField)
			{
				if (*piEditedField == 2)
				{
					MessageBoxA(NULL, pButtonControlIfBlocked, "Notification", MB_OK);
					bProceed = false;
				}
			}
			if (bProceed == true)
			{
				cStr tOldDir = GetDir();
				char * cFileSelected;
				cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "All\0*.*\0", lwithin_s.Get(), NULL);
				SetDir(tOldDir.Get());
				if (cFileSelected && strlen(cFileSelected) > 0)
				{
					if (piEditedField) *piEditedField = 1;
					std::string relative = cFileSelected;
					std::string fullpath = tOldDir.Get();
					fullpath += "\\";
					if (pestrcasestr(lwithin_s.Get(), "scriptbank"))
						fullpath += lwithin_s.Get();
					replaceAll(relative, fullpath, "");
					strcpy(cTmpInput, relative.c_str());
				}
			}
		}
	}
	ImGui::PopItemWidth();

	return &cTmpInput[0];

}

char* imgui_setpropertyfile2_dlua(int group, char* data_s, char* field_s, char* desc_s, char* within_s)
{
	return imgui_setpropertyfile2_ex_dlua(group, data_s, field_s, desc_s, within_s, NULL, NULL);
}

char * imgui_setpropertyfile2(int group, char* data_s, char* field_s, char* desc_s, char* within_s)
{
	char *cRet;
	cstr ldata_s = data_s, ldesc_s = desc_s, lfields_s = field_s, lwithin_s = within_s;

	if (cstr(data_s) == "" || !data_s)  ldata_s = "";
	if (cstr(desc_s) == "" || !desc_s)  ldesc_s = "";
	if (cstr(field_s) == "" || !field_s)  lfields_s = "";
	if (cstr(within_s) == "" || !within_s)  lwithin_s = "";

	std::string uniquiField = "";
	uniquiField = uniquiField + "##" + lfields_s.Get();
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

	if (lfields_s != "") {
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
		ImGui::Text(lfields_s.Get());
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
		ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
	}
	strcpy(cTmpInput, ldata_s.Get());

	bool bSoundSet = false;
	if (pestrcasestr(lfields_s.Get(), "soundset") || lfields_s == "Type") {
		bSoundSet = true;
	}
	if (bSoundSet && t.entityprofile[t.gridentity].ischaracter > 0) {

		ImGui::PushItemWidth(-10);

		//Only displayt Male,FeMale selection.
		const char* items[] = { "Male", "Female" };
		int item_current_type_selection = 0; //Default Custom.
		if (pestrcasestr(cTmpInput, "Female")) {
			item_current_type_selection = 1;
		}
		if (ImGui::Combo(uniquiField.c_str(), &item_current_type_selection, items, IM_ARRAYSIZE(items))) {
			strcpy(cTmpInput, items[item_current_type_selection]);
		}
		if (ImGui::IsItemHovered() && ldesc_s != "") ImGui::SetTooltip("%s", ldesc_s.Get());

		ImGui::PopItemWidth();
		return &cTmpInput[0];

	}

	ImGui::PushItemWidth( -10 - (ImGui::GetFontSize()*2.0) ); //-6 padding.

	ImGui::InputText(uniquiField.c_str(), &cTmpInput[0], MAXTEXTINPUT);
	if (ImGui::IsItemHovered() && ldesc_s != "") ImGui::SetTooltip("%s", ldesc_s.Get());
	if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;

	ImGui::PopItemWidth();
	ImGui::SameLine();

	uniquiField = "...";
	uniquiField = uniquiField + "##";
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

	ImGui::PushItemWidth(ImGui::GetFontSize()*2.0);

	if (ImGui::StyleButton(uniquiField.c_str(), ImVec2(ImGui::GetFontSize()*1.48, 0))) { //ImVec2(ImGui::GetFontSize()*2.0,0)
		//PE: filedialogs change dir so.
		cStr tOldDir = GetDir();
		char * cFileSelected;
		//NOC_FILE_DIALOG_DIR
//		if(bSoundSet)
//			cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_DIR, "All\0*.*\0", lwithin_s.Get(), NULL);
//		else
			cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "All\0*.*\0", lwithin_s.Get(), NULL);

		SetDir(tOldDir.Get());

		if (cFileSelected && strlen(cFileSelected) > 0) {
			std::string relative = cFileSelected;
			std::string fullpath = tOldDir.Get();
			fullpath += "\\";

			// scriptbank\  //
			if (bSoundSet || pestrcasestr(lwithin_s.Get(), "scriptbank")) {
				if( pestrcasestr(cFileSelected,".lua"))
					fullpath += "scriptbank\\"; //lwithin_s.Get(); PE: This can change in parent mode 2
			}

			replaceAll(relative, fullpath , "");
			strcpy(cTmpInput, relative.c_str() );

			if (bSoundSet) {
				char *found = (char *) pestrcasestr(cTmpInput, "\\");
				if (found)
					found[0] = 0;
				//Remove everything after \\

			}
		}
		//File Selector.
	}

	ImGui::PopItemWidth();

	return &cTmpInput[0];

}

char * imgui_setpropertystring2(int group, char* data_s, char* field_s, char* desc_s)
{
	char *cRet;
	cstr ldata_s = data_s, ldesc_s = desc_s , lfields_s = field_s;

	if (cstr(data_s) == "" || !data_s)  ldata_s = "";
	if (cstr(desc_s) == "" || !desc_s)  ldesc_s = "";
	if (cstr(field_s) == "" || !field_s)  lfields_s = "";

	std::string uniquiField = ""; //lfields_s.Get();
	uniquiField = uniquiField + "##" + lfields_s.Get();
	uniquiField  = uniquiField+ std::to_string(grideleprof_uniqui_id++);

	if (lfields_s != "") {
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
		ImGui::Text(lfields_s.Get());
		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
		ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
	}
	ImGui::PushItemWidth(-10);

	int inputFlags = 0;
//	if (!ImGui::IsWindowHovered()) //Not needed input is already disabled in rendertarget.
//		inputFlags = ImGuiInputTextFlags_ReadOnly;

	strcpy(cTmpInput, ldata_s.Get());
	if (ImGui::InputText(uniquiField.c_str(), &cTmpInput[0], MAXTEXTINPUT, inputFlags)) {
		bImGuiGotFocus = true;
	}
	if (ImGui::IsItemHovered() && ldesc_s != "" ) ImGui::SetTooltip("%s", ldesc_s.Get());
	if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;

	ImGui::PopItemWidth();

	return &cTmpInput[0];
}

char * imgui_setpropertylist2c(int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype)
{
	cstr ldata_s = data_s, ldesc_s = desc_s, lfields_s = field_s;

	if (cstr(data_s) == "" || !data_s)  ldata_s = "";
	if (cstr(desc_s) == "" || !desc_s)  ldesc_s = "";
	if (cstr(field_s) == "" || !field_s)  lfields_s = "";

	int current_selection = atoi(ldata_s.Get());


	int listmax = 0;
	listmax = 0;
	if (listtype == 0)
	{
		listmax = 1;
		t.list_s[0] = t.strarr_s[471];
		t.list_s[1] = t.strarr_s[470];
	}
	if (listtype == 1)
	{
//		listmax = fillgloballistwithweapons();
		listmax = fillgloballistwithweaponsQuick();
		for (int n = 0; n <= listmax; n++)
		{
			if (ldata_s == t.list_s[n]) {
				current_selection = n;
				break;
			}
		}
	}
	if (listtype == 11)
	{
		listmax = fillgloballistwithbehaviours();
		for (int n = 0; n <= listmax; n++)
		{
			if (ldata_s == t.list_s[n]) {
				current_selection = n;
				break;
			}
		}
	}

	const char* current_item = t.list_s[current_selection].Get();

	std::string uniquiField = ""; //lfields_s.Get()
	uniquiField = uniquiField + "##" + lfields_s.Get();
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);


	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
	ImGui::Text(lfields_s.Get());
	ImGui::SameLine();
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
	ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));

	ImGui::PushItemWidth(-10);

	if (ImGui::BeginCombo(uniquiField.c_str() , current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n <= listmax; n++)
		{
			bool is_selected = (current_item == t.list_s[n].Get()); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(t.list_s[n].Get(), is_selected)) {
				current_selection = n;
				current_item = t.list_s[n].Get();
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}
	
	ImGui::PopItemWidth();
	return t.list_s[current_selection].Get();
}

int imgui_setpropertylist2(int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype)
{
	cstr ldata_s = data_s, ldesc_s = desc_s, lfields_s = field_s;

	if (cstr(data_s) == "" || !data_s)  ldata_s = "";
	if (cstr(desc_s) == "" || !desc_s)  ldesc_s = "";
	if (cstr(field_s) == "" || !field_s)  lfields_s = "";

	int current_selection = atoi(ldata_s.Get());


	int listmax = 0;

	listmax = 0;
	if (listtype == 0)
	{
		listmax = 1;
		t.list_s[0] = t.strarr_s[471];
		t.list_s[1] = t.strarr_s[470];
	}
	if (listtype == 1)
	{
		listmax = fillgloballistwithweapons();
	}
	if (listtype == 11)
	{
		listmax = fillgloballistwithbehaviours();
	}

	const char* current_item = t.list_s[current_selection].Get();
	
	std::string uniquiField = ""; //lfields_s.Get()
	uniquiField = uniquiField + "##" + lfields_s.Get();
	uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
	ImGui::Text(lfields_s.Get());
	ImGui::SameLine();
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
	ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));

	ImGui::PushItemWidth(-10);

	if (ImGui::BeginCombo(uniquiField.c_str() , current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n <= listmax; n++)
		{
			bool is_selected = (current_item == t.list_s[n].Get() ); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(t.list_s[n].Get(), is_selected)) {
				current_selection = n;
				current_item = t.list_s[n].Get();
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", ldesc_s.Get() );

	ImGui::PopItemWidth();
	return current_selection;
}
#endif



#ifdef FPSEXCHANGE
//char * imgui_setpropertyfile2(int group, char* data_s, char* field_s, char* desc_s, char* within_s)
//{
//	char *cRet;
//	return cRet;
//}

void startgroup ( char* s_s )
{
	if (  cstr(s_s) == ""  )  s_s = "";
	SetFileMapDWORD (  3,g.g_filemapoffset,2  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(s_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,s_s  ); g.g_filemapoffset += ((Len(s_s)+3)/4 )*4;
}

void endgroup ( void )
{
	SetFileMapDWORD (  3,g.g_filemapoffset,0  ); g.g_filemapoffset += 4;
}

void setpropertystring2 ( int group, char* data_s, char* field_s, char* desc_s )
{
	if (  cstr(data_s) == ""  )  data_s = "";
	SetFileMapDWORD (  3,g.g_filemapoffset,3  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,group  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(field_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,field_s  ); g.g_filemapoffset += ((Len(field_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(data_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,data_s  ); g.g_filemapoffset += ((Len(data_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(desc_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,desc_s  ); g.g_filemapoffset += ((Len(desc_s)+3)/4 )*4;
}

void setpropertycolor2 ( int group, int dataval, char* field_s, char* desc_s )
{
	cstr data_s =  "";
	data_s=data_s+Str(RgbR(dataval))+" "+Str(RgbG(dataval))+" "+Str(RgbB(dataval));
	SetFileMapDWORD (  3,g.g_filemapoffset,4  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,group  );  g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(field_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,field_s ); g.g_filemapoffset += ((Len(field_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(data_s.Get())  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,data_s.Get()); g.g_filemapoffset += ((Len(data_s.Get())+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(desc_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,desc_s  ); g.g_filemapoffset += ((Len(desc_s)+3)/4 )*4;
//endfunction

}

void setpropertyfile2 ( int group, char* data_s, char* field_s, char* desc_s, char* within_s )
{
	cstr s_s =  "";
	if (  cstr(data_s) == ""  )  data_s = "";
	SetFileMapDWORD (  3,g.g_filemapoffset,5  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,group  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(field_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,field_s  ); g.g_filemapoffset += ((Len(field_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(data_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,data_s  ); g.g_filemapoffset += ((Len(data_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(desc_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,desc_s ); g.g_filemapoffset += ((Len(desc_s)+3)/4 )*4;
	s_s = g.rootdir_s+within_s;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(s_s.Get())  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,s_s.Get()  ); g.g_filemapoffset += ((Len(s_s.Get())+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(t.strarr_s[321].Get())  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,t.strarr_s[321].Get() ); g.g_filemapoffset += ((Len(t.strarr_s[321].Get())+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(t.strarr_s[322].Get()) ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,t.strarr_s[322].Get() ); g.g_filemapoffset += ((Len(t.strarr_s[322].Get())+3)/4 )*4;
//endfunction

}

void setpropertylist2 ( int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype )
{
	int listmax = 0;
	if ( strcmp ( data_s , "" ) == 0  )  strcpy ( data_s , "" );
	if (  listtype == 0 ) 
	{
		//  yesno
		if (  strcmp ( data_s , "0" ) == 0  )  strcpy ( data_s , t.strarr_s[471].Get() );
		if (  strcmp ( data_s , "1" ) == 0  )  strcpy ( data_s , t.strarr_s[470].Get() );
	}
	if (listtype == 6)
	{
		if (strcmp (data_s, "0") == 0)  strcpy (data_s, "Shadow and Reflect");
		if (strcmp (data_s, "1") == 0)  strcpy (data_s, "No Shadow No Reflect");
		if (strcmp (data_s, "2") == 0)  strcpy (data_s, "No Shadow and Reflect");
	}
	if (  listtype == 11 )
	{
		//  behaviours (trim scriptbank behaviours and .fpi)
		strcpy ( data_s , Right(data_s,Len(data_s)-Len("behavioursx")) );
		strcpy ( data_s , Left(data_s,Len(data_s)-4) );
		t.strwork = "" ; t.strwork = t.strwork + Upper(Left(data_s,1))+Lower(Right(data_s,Len(data_s)-1));
		strcpy ( data_s , t.strwork.Get() );
	}
	SetFileMapDWORD (  3,g.g_filemapoffset,6  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,group  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,controlindex  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(field_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,field_s  ); g.g_filemapoffset += ((Len(field_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(data_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,data_s  ); g.g_filemapoffset += ((Len(data_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(desc_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,desc_s  ); g.g_filemapoffset += ((Len(desc_s)+3)/4 )*4;
	listmax=0;
	if (  listtype == 0 ) 
	{
		listmax=1;
		t.list_s[0]=t.strarr_s[471];
		t.list_s[1]=t.strarr_s[470];
	}
	if (  listtype == 1 ) 
	{
		listmax=fillgloballistwithweapons();
	}
	if (listtype == 6)
	{
		listmax = 2;
		t.list_s[0] = "Shadow and Reflect";
		t.list_s[1] = "No Shadow No Reflect";
		t.list_s[2] = "No Shadow and Reflect";	
	}
	if (  listtype == 11 )
	{
		listmax=fillgloballistwithbehaviours();
	}
	SetFileMapDWORD (  3,g.g_filemapoffset,listmax  ); g.g_filemapoffset += 4;
	for ( int i = 0 ; i<=  listmax; i++ )
	{
		SetFileMapDWORD (  3,g.g_filemapoffset,Len(t.list_s[ i ].Get())  ); g.g_filemapoffset += 4;
		SetFileMapString (  3,g.g_filemapoffset,t.list_s[ i ].Get()  ); g.g_filemapoffset += ((Len(t.list_s[ i ].Get())+3)/4 )*4;
	}
//endfunction

}

void setpropertylist3 ( int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype )
{
	int listmax = 0;
	if (  strcmp ( data_s , "" ) == 0  )  strcpy ( data_s , "" );
	if (  strcmp ( data_s , "0" ) == 0  )  strcpy ( data_s , "No" );
	if (  strcmp ( data_s , "1" ) == 0  )  strcpy ( data_s , "A" );
	if (  strcmp ( data_s , "2" ) == 0  )  strcpy ( data_s , "B" );
	SetFileMapDWORD (  3,g.g_filemapoffset,6  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,group  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,controlindex  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(field_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,field_s  ); g.g_filemapoffset += ((Len(field_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(data_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,data_s  ); g.g_filemapoffset += ((Len(data_s)+3)/4 )*4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(desc_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,desc_s  ); g.g_filemapoffset += ((Len(desc_s)+3)/4 )*4;
	listmax=2;
	Dim (  t.list_s,2  );
	t.list_s[0]="No";
	t.list_s[1]="A";
	t.list_s[2]="B";
	SetFileMapDWORD (  3,g.g_filemapoffset,listmax  ); g.g_filemapoffset += 4;
	for ( int i = 0 ; i<=  listmax; i++ )
	{
		SetFileMapDWORD (  3,g.g_filemapoffset,Len(t.list_s[ i ].Get())  ); g.g_filemapoffset += 4;
		SetFileMapString (  3,g.g_filemapoffset,t.list_s[ i ].Get()  ); g.g_filemapoffset += ((Len(t.list_s[ i ].Get())+3)/4 )*4;
	}
//endfunction

}

void setpropertybase ( int code, char*  s_s )
{
	if ( strcmp ( s_s , "" ) == 0  )  strcpy ( s_s , "" );
	SetFileMapString (  2, STRING_A, s_s );
	SetFileMapString (  2, STRING_B, "" );
	SetFileMapString (  2, STRING_C, "" );
	SetFileMapDWORD (  2, code, 1 );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, code )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
//endfunction

}

void setpropertystring ( int group, char* data_s, char* field_s, char* desc_s )
{
	if (  strcmp ( data_s , "" )  )  strcpy ( data_s , "" );
	SetFileMapString (  2, STRING_A, field_s );
	SetFileMapString (  2, STRING_B, data_s );
	SetFileMapString (  2, STRING_C, desc_s );
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapDWORD (  2, ENTITY_ADD_EDIT_BOX, 1 );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_ADD_EDIT_BOX )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
//endfunction

}

void setpropertycolor ( int group, int dataval, char* field_s, char* desc_s )
{
	cstr data_s =  "";
	data_s=data_s+Str(RgbR(dataval))+" "+Str(RgbG(dataval))+" "+Str(RgbB(dataval));
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapString (  2, STRING_A, field_s );
	SetFileMapString (  2, STRING_B, data_s.Get() );
	SetFileMapString (  2, STRING_C, desc_s );
	SetFileMapDWORD (  2, ENTITY_ADD_COLOR_PICKER, 1 );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_ADD_COLOR_PICKER )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
//endfunction

}

void setpropertyfile ( int group, char* data_s, char* field_s, char* desc_s, char* within_s )
{
	if ( strcmp ( data_s , "" ) == 0 )  strcpy ( data_s , "" );
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapString (  2, STRING_A, field_s );
	SetFileMapString (  2, STRING_B, data_s );
	SetFileMapString (  2, STRING_C, desc_s );
	t.strwork = "" ; t.strwork = t.strwork + g.rootdir_s+within_s;
	SetFileMapString (  2, 2024, t.strwork.Get() );
	SetFileMapString (  2, 2280, t.strarr_s[321].Get() );
	SetFileMapString (  2, 2536, t.strarr_s[322].Get() );
	SetFileMapDWORD (  2, ENTITY_ADD_FILE_PICKER, 1 );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_ADD_FILE_PICKER )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
}
#endif

int fillgloballistwithweaponsQuick(void)
{
	int retvalue = 0;
	int gunid = 0;
	Dim(t.list_s, 1 + g.gunmax);
	t.list_s[0] = "";
	int iListCount = 0; // 020316 - v1.13b1 - list can exclude player weapons with no HUDs
	for (gunid = 1; gunid <= g.gunmax; gunid++)
	{
		//For drop down , so quick with no file checks.
		iListCount++;
		t.list_s[iListCount] = t.gun[gunid].name_s;
	}

	// 190416 - sort weapons into alpha order
	for (int iSortA = 0; iSortA <= iListCount; iSortA++)
	{
		for (int iSortB = 0; iSortB <= iListCount; iSortB++)
		{
			if (iSortA != iSortB && strcmp(t.list_s[iSortA].Get(), t.list_s[iSortB].Get()) < 0)
			{
				// swap over for bubble sort
				cstr pStoreA = t.list_s[iSortA];
				t.list_s[iSortA] = t.list_s[iSortB];
				t.list_s[iSortB] = pStoreA;
			}
		}
	}

	// return valid gun name count
	return iListCount;
}

int fillgloballistwithweapons ( void )
{
	int retvalue = 0;
	int gunid = 0;
	Dim (  t.list_s,1+g.gunmax  );
	t.list_s[0] = "";
	int iListCount = 0; // 020316 - v1.13b1 - list can exclude player weapons with no HUDs
	for ( gunid = 1; gunid <= g.gunmax; gunid++ )
	{
		// 020316 - v1.13b1 - quickly check the existence of the HUD.X file to see if we exclude (later change when weapon changes for characters)
		t.tfile_s = cstr("gamecore\\guns\\") + t.gun[gunid].name_s + cstr("\\HUD.X");
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			iListCount++;
			t.list_s[iListCount] = t.gun[gunid].name_s;
		}
	}

	// 190416 - sort weapons into alpha order
	for ( int iSortA = 0; iSortA <= iListCount; iSortA++ )
	{
		for ( int iSortB = 0; iSortB <= iListCount; iSortB++ )
		{
			if ( iSortA != iSortB && strcmp ( t.list_s[iSortA].Get(), t.list_s[iSortB].Get() ) < 0 )
			{
				// swap over for bubble sort
				cstr pStoreA = t.list_s[iSortA];
				t.list_s[iSortA] = t.list_s[iSortB];
				t.list_s[iSortB] = pStoreA;
			}
		}
	}

	// return valid gun name count
	return iListCount;
}

int fillgloballistwithbehaviours_init ( void )
{
	cstr storedir_s =  "";
	int retvalue = 0;
	cstr file_s =  "";
	int c = 0;
	retvalue=0;
	t.strwork = "" ; t.strwork = t.strwork + g.rootdir_s+"scriptbank\\behaviours";
	if (  PathExist( t.strwork.Get() ) == 1 ) 
	{
		storedir_s=GetDir();
		SetDir (  t.strwork.Get() );
		ChecklistForFiles ();
		Dim (  t.behaviourlist_s,ChecklistQuantity( ) );
		for ( c = 1 ; c<=  ChecklistQuantity(); c++ )
		{
			file_s=ChecklistString(c);
			if (  strcmp ( Lower(Right(file_s.Get(),4)) , ".fpi" ) == 0 ) 
			{
				++retvalue;
				t.strwork = "" ; t.strwork=t.strwork+Left(file_s.Get(),Len(file_s.Get())-4);
				file_s = t.strwork;
				t.behaviourlist_s[retvalue] = ""; t.behaviourlist_s[retvalue]=t.behaviourlist_s[retvalue]+Upper(Left(file_s.Get(),1))+Lower(Right(file_s.Get(),Len(file_s.Get())-1));
			}
		}
		SetDir (  storedir_s.Get() );
	}
//endfunction retvalue
	return retvalue;
}

int fillgloballistwithbehaviours ( void )
{
	int behaviourlistmax = 0;
	int retvalue = 0;
	int n;
	retvalue=behaviourlistmax;
	Dim (  t.list_s,retvalue  );
	t.list_s[0]="";
	if (  retvalue>0 ) 
	{
		for ( n = 1 ; n<=  retvalue; n++ )
		{
			t.list_s[n-1]=t.behaviourlist_s[n];
		}
		retvalue=retvalue-1;
	}
//endfunction retvalue
	return retvalue
;
}

void setpropertylist ( int group, int controlindex, char* data_s, char* field_s, char* desc_s, int listtype )
{
	int listmax = 0;
	if ( strcmp ( data_s , "" ) == 0  )  strcpy ( data_s , "" );
	if (  listtype == 0 ) 
	{
		if (  strcmp ( data_s , "0" ) == 0  )  strcpy ( data_s , t.strarr_s[471].Get() );
		if (  strcmp ( data_s , "1" ) == 0  )  strcpy ( data_s , t.strarr_s[470].Get() );
	}
	SetFileMapString (  2, STRING_A, field_s );
	SetFileMapString (  2, STRING_B, data_s );
	SetFileMapString (  2, STRING_C, desc_s );
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, controlindex );
	SetFileMapDWORD (  2, ENTITY_ADD_LIST_BOX, 1 );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_ADD_LIST_BOX )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
	listmax=0;
	if (  listtype == 0 ) 
	{
		t.list_s[0]=t.strarr_s[471];
		t.list_s[1]=t.strarr_s[470];
		listmax=1;
	}
	if (  listtype == 1 ) 
	{
		listmax=fillgloballistwithweapons();
	}
	if (  listtype == 11 ) 
	{
		listmax=fillgloballistwithbehaviours();
	}
	for ( int i = 0 ; i<=  listmax; i++ )
	{
		SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
		SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, controlindex );
		SetFileMapString (  2, STRING_A, t.list_s[ i ].Get() );
		SetFileMapDWORD (  2, ENTITY_ADD_ITEM_TO_LIST_BOX, 1 );
		SetEventAndWait (  2 );
		while (  GetFileMapDWORD( 2, ENTITY_ADD_ITEM_TO_LIST_BOX )  ==  1 ) 
		{
			SetEventAndWait (  2 );
		}
	}
//endfunction

}

// 
//  Interface Properties Expressions
// 

char* getpropertyfield ( int group, int iControl )
{
	cstr field_s =  "";
	SetFileMapDWORD (  2, ENTITY_GET_CONTROL_NAME, 1 );
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, iControl );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_GET_CONTROL_NAME )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
	field_s = GetFileMapString( 2, STRING_A );
//endfunction field$
	strcpy ( t.szreturn , field_s.Get() );
	return t.szreturn;
}

char* getpropertydata ( int group, int iControl )
{
	cstr data_s =  "";
	SetFileMapDWORD (  2, ENTITY_GET_CONTROL_CONTENTS, 1 );
	SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
	SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, iControl );
	SetEventAndWait (  2 );
	while (  GetFileMapDWORD( 2, ENTITY_GET_CONTROL_CONTENTS )  ==  1 ) 
	{
		SetEventAndWait (  2 );
	}
	data_s = GetFileMapString( 2, STRING_B );
//endfunction data$
	strcpy ( t.szreturn , data_s.Get() );
	return t.szreturn;
;
}


//COMMON INTERFACE FUNCTIONS


void set_progress_position ( int  item, int  position )
{
			SetFileMapDWORD (  1, SET_PROGRESS_ITEM, item );
			SetFileMapDWORD (  1, SET_PROGRESS_POSITION, position );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, SET_PROGRESS_ITEM ) ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

char* get_list_box ( int  item, int  index )
{
	cstr contents_s =  "";
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, SET_LIST_INDEX, index );
			SetFileMapDWORD (  1, GET_LIST_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, GET_LIST_ITEM )  ==  1 )
			{
				SetEventAndWait (  1 );
			}
			contents_s = GetFileMapString (  1, STRING_A );
//endfunction contents$
	strcpy ( t.szreturn , contents_s.Get() );
	return t.szreturn;
}

void set_radio_state ( int  item, int  state )
{
			if (  state == 0  )  state = 2;
			SetFileMapDWORD (  1, SET_RADIO_ITEM, item );
			SetFileMapDWORD (  1, SET_RADIO_STATE, state );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, SET_RADIO_STATE ) > 0 )  
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

int get_radio_state ( int  item )
{
			int state = 0;
			SetFileMapDWORD (  1, SET_RADIO_ITEM, item );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, SET_RADIO_ITEM ) > 0 ) 
			{
				SetEventAndWait (  1 );
			}
			state = GetFileMapDWORD ( 1, GET_RADIO_ITEM );
//endfunction state
	return state;
}

void set_edit_item ( int  item, char*  text_s )
{
			SetFileMapDWORD (  1, SET_EDIT_ITEM, item );
			SetFileMapString (  1, STRING_A, text_s );
			SetFileMapDWORD (  1, SET_EDIT_TEXT, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, SET_EDIT_TEXT )  ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

char* get_edit_item ( int  item )
{
	cstr text_s =  "";
			SetFileMapDWORD (  1, SET_EDIT_ITEM, item );
			SetFileMapDWORD (  1, GET_EDIT_TEXT, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, GET_EDIT_TEXT )  ==  1 )
			{
				SetEventAndWait (  1 );
			}
			text_s = GetFileMapString ( 1, STRING_A );
//endfunction text$
	strcpy ( t.szreturn , text_s.Get() );
	return t.szreturn;
}

void browse ( char*  title_s, char*  directory_s, char*  filter_s )
{
			SetFileMapString (  1, STRING_A, title_s );
			SetFileMapString (  1, STRING_B, directory_s );
			SetFileMapString (  1, STRING_C, filter_s );
			SetFileMapDWORD (  1, BROWSE_DISPLAY, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, BROWSE_FILE_SELECTED ) ==  0 )
			{
				SetEventAndWait (  1 );
			}
			SetFileMapDWORD (  1, BROWSE_FILE_SELECTED, 0 );
			SetFileMapDWORD (  1, BUTTON_CLICKED, 0 );
//endfunction

}

char* browse_for_folder ( char*  directory_s )
{
			cstr text_s =  "";
			SetFileMapString (  1, STRING_A, directory_s );
			SetFileMapDWORD (  1, 200, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (  1, 204 )  ==  0 )
			{
				SetEventAndWait (  1 );
			}
			SetFileMapDWORD (  1, 204, 0 );
			SetEventAndWait (  1 );
			text_s = GetFileMapString ( 1, STRING_A );
			SetFileMapDWORD (  1, BUTTON_CLICKED, 0 );
			SetEventAndWait (  1 );
//endfunction text$
	strcpy ( t.szreturn , text_s.Get() );
	return t.szreturn;
}

void add_list_item ( int  item, char*  text_s )
{
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapString (  1, STRING_A, text_s );
			SetFileMapDWORD (  1, ADD_LIST_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, ADD_LIST_ITEM )  ==  1 )
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

char* get_list_item ( int  item, int  index )
{
	cstr text_s =  "";
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, SET_LIST_INDEX, index );
			SetFileMapDWORD (  1, GET_LIST_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, GET_LIST_ITEM )  ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
			text_s = GetFileMapString ( 1, STRING_A );
//endfunction text$
	strcpy ( t.szreturn , text_s.Get() );
	return t.szreturn;
}

void delete_list_item ( int  item, int  index )
{
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, SET_LIST_INDEX, index );
			SetFileMapDWORD (  1, DELETE_LIST_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (   1, DELETE_LIST_ITEM )  ==  1  ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

void clear_list ( int  item )
{
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, LIST_CLEAR, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (  1, LIST_CLEAR )  ==  1  ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

void select_list_item ( int  item, int  selectionindex )
{
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, LIST_SELECT_ITEM_INDEX, selectionindex );
			SetFileMapDWORD (  1, LIST_SELECT_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (  1, LIST_SELECT_ITEM )  ==  1  ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

int get_list_item_selection ( int  item )
{
			int selection = -1;
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, GET_LIST_SELECTION, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (  1, GET_LIST_SELECTION )  ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
			selection = GetFileMapDWORD( 1, LIST_SELECTION );
//endfunction selection
	return selection
;
}

void insert_list_item ( int  item, int  position, char*  text_s )
{
			SetFileMapDWORD (  1, SET_LIST_ITEM, item );
			SetFileMapDWORD (  1, LIST_INSERT_POSITION, position );
			SetFileMapString (  1, STRING_A, text_s );
			SetFileMapDWORD (  1, LIST_INSERT_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD (  1, LIST_INSERT_ITEM )  ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}

void add_combo_box ( int  item, char*  text_s )
{
			SetFileMapDWORD (  1, SET_COMBO_ITEM, item );
			SetFileMapString (  1, STRING_A, text_s );
			SetFileMapDWORD (  1, ADD_COMBO_ITEM, 1 );
			SetEventAndWait (  1 );
			while (  GetFileMapDWORD ( 1, ADD_COMBO_ITEM )  ==  1 ) 
			{
				SetEventAndWait (  1 );
			}
//endfunction

}


//Property Functions


void add_group ( char*  name_s )
{
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapDWORD (  2, ENTITY_ADD_GROUP, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_GROUP )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_edit_box ( int  group, char*  name_s, char*  contents_s, char*  description_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapString (  2, STRING_B, contents_s );
			SetFileMapString (  2, STRING_C, description_s );
			SetFileMapDWORD (  2, ENTITY_ADD_EDIT_BOX, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_EDIT_BOX )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_color_picker ( int  group, char*  name_s, char*  contents_s, char*  description_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapString (  2, STRING_B, contents_s );
			SetFileMapString (  2, STRING_C, description_s );
			SetFileMapDWORD (  2, ENTITY_ADD_COLOR_PICKER, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_COLOR_PICKER )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_file_picker_ex ( int  group, char*  name_s, char*  contents_s, char*  description_s, char*  dir_s, char*  filter_s, char*  title_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapString (  2, STRING_B, contents_s );
			SetFileMapString (  2, STRING_C, description_s );
			SetFileMapString (  2, 2024, dir_s );
			if (  filter_s != "" ) 
			{
				SetFileMapString (  2, 2280, filter_s );
			}
			else
			{
				SetFileMapString (  2, 2280, t.strarr_s[323].Get() );
			}
			if (  cstr(title_s) != "" ) 
			{
				SetFileMapString (  2, 2536, title_s );
			}
			else
			{
				SetFileMapString (  2, 2536, t.strarr_s[324].Get() );
			}
			SetFileMapDWORD (  2, ENTITY_ADD_FILE_PICKER, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_FILE_PICKER )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_file_picker ( int  group, char*  name_s, char*  contents_s, char*  description_s, char*  dir_s )
{
	add_file_picker_ex( group, name_s, contents_s, description_s, dir_s, "", "" );
//endfunction

}

void add_font_picker ( int  group, char*  name_s, char*  contents_s, char*  description_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapString (  2, STRING_B, contents_s );
			SetFileMapString (  2, STRING_C, description_s );
			SetFileMapDWORD (  2, ENTITY_ADD_FONT_PICKER, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_FONT_PICKER )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_list_box ( int  group, char*  name_s, char*  contents_s, char*  description_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapString (  2, STRING_A, name_s );
			SetFileMapString (  2, STRING_B, contents_s );
			SetFileMapString (  2, STRING_C, description_s );
			SetFileMapDWORD (  2, ENTITY_ADD_LIST_BOX, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_LIST_BOX )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

void add_item_to_list_box ( int  group, int  control, char*  item_s )
{
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, control );
			SetFileMapString (  2, STRING_A, item_s );
			SetFileMapDWORD (  2, ENTITY_ADD_ITEM_TO_LIST_BOX, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_ADD_ITEM_TO_LIST_BOX )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
//endfunction

}

char* get_control_name ( int  group, int  control )
{
	cstr name_s =  "";
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, control );
			SetFileMapDWORD (  2, ENTITY_GET_CONTROL_NAME, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_GET_CONTROL_NAME )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
			name_s =  GetFileMapString( 2, STRING_A );
//endfunction name$
	strcpy ( t.szreturn , name_s.Get() );
	return t.szreturn;
}

char* get_control_contents ( int  group, int  control )
{
	cstr contents_s =  "";
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, control );
			SetFileMapDWORD (  2, ENTITY_GET_CONTROL_CONTENTS, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_GET_CONTROL_CONTENTS )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
			contents_s = GetFileMapString( 2, STRING_B );
//endfunction contents$
	strcpy ( t.szreturn , contents_s.Get() );
	return t.szreturn;
}

char* get_control_description ( int  group, int  control )
{
	cstr description_s =  "";
			SetFileMapDWORD (  2, ENTITY_SET_GROUP_INDEX, group );
			SetFileMapDWORD (  2, ENTITY_SET_CONTROL_INDEX, control );
			SetFileMapDWORD (  2, ENTITY_GET_CONTROL_DESCRIPTION, 1 );
			SetEventAndWait (  2 );
			while (  GetFileMapDWORD( 2, ENTITY_GET_CONTROL_DESCRIPTION )  ==  1 ) 
			{
				SetEventAndWait (  2 );
			}
			description_s = GetFileMapString( 2, STRING_C );
//endfunction description$
	strcpy ( t.szreturn , description_s.Get() );
	return t.szreturn;
}

//Memory check behaviour for MAP EDITOR / TEST GAME

void checkmemoryforgracefulexit ( void )
{
	int recoverdonotuseany3dreferences = 0;
	int ttogglebannertimer = 0;
	int tredscreencount = 0;
	int tsmemavailable = 0;
	int ttogglebanner = 0;
	int tokay = 0;

	//  if cannot create 100MB of contiguous memory, we're nearing the max fragmentation level
	if (  t.game.gameisexe == 0 && g.globals.memorydetector == 1 ) 
	{
	tsmemavailable=SMEMAvailable(1);
	if (  tsmemavailable>1600000 ) 
	{

		//  The Red Screen of Resurrection
		t.strwork = ""; t.strwork = t.strwork + "checkmemoryforgracefulexit - memory detector "+Str(tsmemavailable)+" Kb";
		timestampactivity(0, t.strwork.Get() );
		tredscreencount=Timer()+2000;
		ttogglebannertimer=Timer()+450;
		while (  Timer()<tredscreencount ) 
		{
			CLS (  Rgb(128,0,0) );
			if (  Timer()>ttogglebannertimer ) 
			{
				ttogglebannertimer=Timer()+450;
				ttogglebanner=1-ttogglebanner;
			}
			PasteImage (  g.editorimagesoffset+5+ttogglebanner,(GetDisplayWidth()-ImageWidth(g.editorimagesoffset+5))/2,(GetDisplayHeight()-ImageHeight(g.editorimagesoffset+5))/2 );
			Sync (  );
		}

		#ifdef FPSEXCHANGE
		//  close conmunication with editor
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1,974,2 );

		//  Before we 'BIN OUT', signal IDE that we wish to return to the IDE editor state
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 970, 1 );
		SetEventAndWait (  1 );

		//  message Box (  - resolution has been changed - must restart - save changes? )
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 900, 1 );
		SetFileMapString (  1, 1256, t.strarr_s[622].Get() );
		SetFileMapString (  1, 1000, t.strarr_s[623].Get() );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1, 900) == 1 ) 
		{
			SetEventAndWait (  1 );
		}
		tokay=GetFileMapDWORD(1, 904);
		#endif

		if (  tokay == 1 ) 
		{
			//  no references to 3D objects
			recoverdonotuseany3dreferences=1;
			//  save as now
			gridedit_saveas_map ( );
		}

		//  call a new map editor
		#ifdef FPSEXCHANGE
		OpenFileMap (  2, "FPSEXCHANGE" );
		SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
		SetFileMapString (  2, 1256, "-r" );
		SetFileMapDWORD (  2, 994, 0 );
		SetFileMapDWORD (  2, 924, 1 );
		SetEventAndWait (  2 );
		#endif

		//  Terminate fragmented EXE
		common_justbeforeend();
		ExitProcess ( 0 );
	}
	}
}

int get_cursor_scale_for_obj ( int tObj )
{
	t.tSizeX_f = ObjectSizeX(tObj,1);
	t.tSizeZ_f = ObjectSizeZ(tObj,1);
	t.tscale_f= Sqrt(t.tSizeX_f*t.tSizeX_f + t.tSizeZ_f*t.tSizeZ_f)*3.0;
	return t.tscale_f;
}

#ifdef ENABLEIMGUI
void AddPayLoad(ImGuiPayload* payload, bool addtocursor)
{
	extern cFolderItem::sFolderFiles *pDragDropFile;
	if (pDragDropFile) {

		IM_ASSERT(payload->DataSize == sizeof(cFolderItem::sFolderFiles *));
		cFolderItem::sFolderFiles * payload_n = (cFolderItem::sFolderFiles *) payload->Data;
		payload_n = payload_n->m_dropptr;
		if (payload_n) {
			//Add the item.
			CloseDownEditorProperties();
			t.inputsys.constructselection = 0;

			t.addentityfile_s = payload_n->m_sFolder.Get();
			if (t.addentityfile_s != "")
			{
				entity_adduniqueentity(false);
				t.tasset = t.entid;
				if (t.talreadyloaded == 0)
				{
					editor_filllibrary();
				}
			}
			if (addtocursor) {

				bool bNormalMasterAdd = true;
				if (bNormalMasterAdd)
				{
					//PE: TODO check if t.entid is valid here.
					//Make sure we are in entty mode.
					bForceKey = true;
					csForceKey = "e";
					t.inputsys.constructselection = t.tasset;
					t.gridentity = t.entid;
					t.inputsys.constructselection = t.entid;
					t.inputsys.domodeentity = 1;
					t.grideditselect = 5;
					editor_refresheditmarkers();
				}
				//PE: Removed in design "Keep window open when dragging in objects to level".
				if(bExternal_Entities_Window)
					bCheckForClosing = true;
			}

		}

		pDragDropFile = NULL;
	}

}

bool TutorialNextAction(void)
{
	tut.iCurrent_Step++;
	return true;
}


//Check if we need to point in the game scene.
bool bReadyForMouseRelease = false;
bool CheckTutorialPlaceit(void)
{
	if (tut.bActive && t.inputsys.mclick == 0 && t.gridentity != 0 ) //
		return CheckTutorialAction("PLACEIT"); //Tutorial: check if we are waiting for this action
	if (tut.bActive && t.gridentity == 0) //terrain
	{
		if (CheckTutorialAction("PLACEIT")) {
			if (t.inputsys.mclick == 0) {
				if (bReadyForMouseRelease) {
					//released.
					TutorialNextAction();
					bReadyForMouseRelease = false;
				}
			}
			else {
				bReadyForMouseRelease = true;
			}
		}
		else bReadyForMouseRelease = false;
		return false;
	}
	else {
		bReadyForMouseRelease = false;
	}
	return false;
}

bool CheckTutorialAction(const char * action, float x_adder)
{
	g_bInTutorialMode = false;
	if (bHelp_Window && tutorial_files.size() >= selected_tutorial) 
	{
		if (tut.bActive) 
		{
			//Tutorial Active.
			g_bInTutorialMode = true;
			if (tut.iCurrent_Step >= 0 && tut.iCurrent_Step < TUTORIALMAXSTEPS) {
				if (pestrcasestr(tut.cStepAction[tut.iCurrent_Step], action)) {

					if (!bTutorialRendered && bImGuiFrameState && !bImGuiReadyToRender) {

						ImVec2 viewPortPos = ImGui::GetMainViewport()->Pos;
						ImVec2 viewPortSize = ImGui::GetMainViewport()->Size;

						//Display the pointer here.
						bTutorialRendered = true; //Make sure we only render one window.
		
						ImVec2 oldpos = ImGui::GetCursorPos();

						ImGuiStyle &st = ImGui::GetStyle();
						float oldborder = st.PopupBorderSize;

						static float sincounter = 0.0f;
						ImGuiWindow* window = ImGui::GetCurrentWindow();
						ImVec2 pos = window->DC.CursorPos;

						//if ((pos.y-32.0f) > viewPortPos.y) //Dont allow it to display outside main viewport
						//{
							float icon_additional_size = 32.0f;
							st.PopupBorderSize = 0;
							pos.x += x_adder + 4;
							pos.x -= (icon_additional_size*0.60);
							//pos.y -= 80.0f;
							pos.y += 80.0f; // pointer points up now (could make this a toggle mode)
							pos.y -= icon_additional_size;
							pos.y += sin(sincounter) * 22.0f;
							sincounter = sincounter + (5.5f*t.ElapsedTime_f);
							if (sincounter >= 360.0f) sincounter -= 360.0f;

							if (strcmp(tut.cStepAction[tut.iCurrent_Step], "PLACEIT") == 0) {
								pos = OldrenderTargetPos + ImVec2((OldrenderTargetSize.x*0.5f) - 64.0f, 60 + (sin(sincounter) * 22.0f));
								pos += tut.vOffsetPointer[tut.iCurrent_Step];
								//ImGui::SetNextWindowPos(OldrenderTargetPos + ImVec2((OldrenderTargetSize.x*0.5f) - 64.0f, 80 + (sin(sincounter) * 22.0f)));
							}

							ID3D11ShaderResourceView* lpTexture = GetImagePointerView(TUTORIAL_POINTERUP);
							if (lpTexture) 
							{
								ImGuiWindow* window = ImGui::GetCurrentWindow();
								ImGui::GetForegroundDrawList()->AddImage((ImTextureID)lpTexture, pos , pos + ImVec2(64+icon_additional_size, 64+icon_additional_size), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0, 1.0, 1.0, 1.0)));
							}

							/*
							//This system will make non transparent window when outside main viewport. (has its own main window).
							ImGui::SetNextWindowPos(pos);
							ImGui::SetNextWindowBgAlpha(0.0f);
							ImGuiWindowFlags flags = ImGuiNextWindowDataFlags_HasBgAlpha | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking;
							ImGui::Begin("##TutorialPointerWindow", NULL, flags);
							ImGui::ImgBtn(TUTORIAL_POINTER, ImVec2(64, 64), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false);
							ImGui::End();
							*/
						//}
						st.PopupBorderSize = oldborder;
						ImGui::SetCursorPos(oldpos);
					}
					bTutorialCheckAction = true;
					return true;
				}
			}
		}
	}
	//selected_tutorial
	bTutorialCheckAction = false;
	return false;
}


float ApplyPivot(sObject* pObject, int iMode, GGVECTOR3 vecValue, float fValue)
{
	if (pObject->position.bApplyPivot)
	{
		GGVec3TransformCoord(&vecValue, &vecValue, &pObject->position.matPivot);

		if (iMode == 0) return vecValue.x;
		if (iMode == 1) return vecValue.y;
		if (iMode == 2) return vecValue.z;
	}

	return fValue;
}


void RenderToPreview(int displayobj)
{
	float oldx_f, oldy_f, oldz_f, oldangx_f, oldangy_f;
	int entid = displayobj - g.entitybankoffset;

	// prepare for thumb , set camera.
	oldx_f = t.editorfreeflight.c.x_f;
	oldy_f = t.editorfreeflight.c.y_f;
	oldz_f = t.editorfreeflight.c.z_f;
	oldangx_f = t.editorfreeflight.c.angx_f;
	oldangy_f = t.editorfreeflight.c.angy_f;

	float fLargestY = ObjectSizeY(displayobj,1); // Also add scale (,1)
	float fLargestX = ObjectSizeX(displayobj,1);
	float fLargestZ = ObjectSizeZ(displayobj,1);

	float fOffsetX = 0.0f,fOffsetZ = 0.0f;

	float terrain_height = BT_GetGroundHeight(t.terrain.TerrainID, GGORIGIN_X, GGORIGIN_Z, 1);

	sObject* pObject = g_ObjectList[displayobj];
	if (pObject && t.entityprofile[entid].ischaracter != 1) {
		float fAdjustScaleX = 1.0, fAdjustScaleZ = 1.0;
		if (pObject->pInstanceOfObject)
		{
			fAdjustScaleX = pObject->position.vecScale[0];
			fAdjustScaleZ = pObject->position.vecScale[2];
			pObject = pObject->pInstanceOfObject;
		}
		float fValue = (pObject->collision.vecMax[0] + pObject->collision.vecMin[0]);
		fValue = ApplyPivot(pObject, 0, GGVECTOR3(pObject->collision.vecMax - pObject->collision.vecMin), fValue);
		fValue = fValue * pObject->position.vecScale[0] * fAdjustScaleX;
		fOffsetX = fValue * 0.5f;

		fValue = (pObject->collision.vecMax[2] + pObject->collision.vecMin[2]);
		fValue = ApplyPivot(pObject, 2, GGVECTOR3(pObject->collision.vecMax - pObject->collision.vecMin), fValue);
		fValue = fValue * pObject->position.vecScale[2] * fAdjustScaleZ;
		fOffsetZ = fValue * 0.5f;
	}
	float fLargest = fLargestX;

	if (fLargestZ > fLargest)
		fLargest = fLargestZ;
	
	fLargest += (fLargestY * 0.2);
	//Prevent camera for getting to far away.
	if (fLargest >= 1500) fLargest = 1500;
	if (fLargestY >= 1500) fLargestY = 1500;

	t.editorfreeflight.c.x_f = 25650 + (fLargest*2.4);
	if(fLargestY < 10.0f)
		t.editorfreeflight.c.y_f = terrain_height + 80 + (fLargestY*1.85);
	else
		t.editorfreeflight.c.y_f = terrain_height + 50 + (fLargestY*1.85);
	t.editorfreeflight.c.z_f = 25550 - (fLargest*2.4);
	t.editorfreeflight.c.angx_f = 13;
	t.editorfreeflight.c.angy_f = -180;
	t.editorfreeflight.s = t.editorfreeflight.c;

	//Preview object could be reused so store old pos.
	float fOldObjPosX = ObjectPositionX(displayobj), fOldObjPosY = ObjectPositionY(displayobj), fOldObjPosZ = ObjectPositionZ(displayobj);
	bool bDisplayObjVisible = false;
	bool bWaterVisible = false;

	if (g_ObjectList[t.terrain.objectstartindex + 2] && g_ObjectList[t.terrain.objectstartindex + 2]->bVisible)
		bWaterVisible = true;

	if (g_ObjectList[displayobj] && g_ObjectList[displayobj]->bVisible)
		bDisplayObjVisible = true;

	//custom clear color.
	custom_back_color[0] = 119.0f/255.0f; custom_back_color[1] = 154.0f / 255.0f; custom_back_color[2] = 181.0f / 255.0f; custom_back_color[3] = 1.0f;

	//Hide everything.
	widget_hide();
	ebe_hide();
	terrain_paintselector_hide();

	editor_restoreentityhighlightobj();
	gridedit_clearentityrubberbandlist();
	waypoint_hideall();

	//  "hide" all entities in map by moving them out the way
	for (t.tcce = 1; t.tcce <= g.entityelementlist; t.tcce++)
	{
		t.tccentid = t.entityelement[t.tcce].bankindex;
		if (t.tccentid > 0)
		{
			t.tccsourceobj = t.entityelement[t.tcce].obj;
			if (ObjectExist(t.tccsourceobj) == 1)
			{
				PositionObject(t.tccsourceobj, 0, 0, 0);
			}
		}
	}


	PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
	RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
	PointCamera(GGORIGIN_X + fOffsetX, terrain_height, GGORIGIN_Z + fOffsetZ);
	
	if(fLargestY > 1000.0f)
		t.editorfreeflight.c.angx_f = CameraAngleX() - 9.0f; //6.0
	else if (fLargestY > 500.0f)
		t.editorfreeflight.c.angx_f = CameraAngleX() - 8.0f; //6.0
	else if (fLargestY > 150.0f)
		t.editorfreeflight.c.angx_f = CameraAngleX() - 7.0f; //6.0
	else if (fLargestY > 100.0f)
		t.editorfreeflight.c.angx_f = CameraAngleX() - 6.0f; //6.0
	else if (fLargestY > 50.0f)
		t.editorfreeflight.c.angx_f = CameraAngleX() - 5.0f; //6.0
	else
		t.editorfreeflight.c.angx_f = CameraAngleX() - 4.0f; //6.0
	t.editorfreeflight.c.angy_f = CameraAngleY();
	RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);

	PositionObject(displayobj, GGORIGIN_X, terrain_height, GGORIGIN_Z);

	HideObject(t.terrain.objectstartindex + 2); //hide water

	if (t.entityprofile[entid].ismarker != 0 || t.entityprofile[entid].zdepth == 0)
	{
		SetObjectMask(displayobj, 1);
	}
	else
	{
		SetObjectMask(displayobj, 1 + (1 << 31));
	}

	ShowObject(displayobj);
	
	visuals_justshaderupdate();

	int iOldSpeedshadows = g.globals.speedshadows;
	g.globals.speedshadows = 0; //We need all cascades.
	terrain_shadowupdate();
	g.globals.speedshadows = iOldSpeedshadows;

	if (t.terrain.TerrainID > 0)
	{
		BT_SetCurrentCamera(t.terrain.gameplaycamera);
		BT_UpdateTerrainCull(t.terrain.TerrainID);
		BT_UpdateTerrainLOD(t.terrain.TerrainID);
		if (g.globals.riftmode > 0)
		{}
		else
		{
			terrain_renderonly();
		}
	}

	bImGuiInTestGame = true; //just reuse this to prevent imgui rendering.
	FastSync();
	bImGuiInTestGame = false;

	if(bWaterVisible)
		ShowObject(t.terrain.objectstartindex + 2);
	if( bDisplayObjVisible )
		ShowObject(displayobj);
	else
		HideObject(displayobj);



	PositionObject(displayobj, fOldObjPosX, fOldObjPosY, fOldObjPosZ);
	
	// delete previous thumbnail
	if (GetImageExistEx(g.importermenuimageoffset + 50))
	{
		image_setlegacyimageloading(true);
		DeleteImage(g.importermenuimageoffset + 50);
		image_setlegacyimageloading(false);
	}

	// we can't grab from the backbuffer when we use a camera image.
	extern DBPRO_GLOBAL CCameraManager m_CameraManager;
	DBPRO_GLOBAL tagCameraData* m_mycam;
	m_mycam = m_CameraManager.GetData(0);
	float thumbnail_dimension = 512;
	if (m_mycam)
	{
		extern GlobStruct* g_pGlob;
		LPGGSURFACE	pTmpSurface = g_pGlob->pCurrentBitmapSurface;
		g_pGlob->pCurrentBitmapSurface = m_mycam->pCameraToImageSurface;

		ImVec2 grab = ImVec2((m_mycam->viewPort3D.Width*0.5) - (thumbnail_dimension*0.5) , (m_mycam->viewPort3D.Height*0.5) - (thumbnail_dimension*0.5) );

		GrabImage(g.importermenuimageoffset + 50, grab.x, grab.y, grab.x+thumbnail_dimension, grab.y + thumbnail_dimension, 0);
		g_pGlob->pCurrentBitmapSurface = pTmpSurface;
	}


	//Restore camera.
	t.editorfreeflight.c.x_f = oldx_f;
	t.editorfreeflight.c.y_f = oldy_f;
	t.editorfreeflight.c.z_f = oldz_f;
	t.editorfreeflight.c.angx_f = oldangx_f;
	t.editorfreeflight.c.angy_f = oldangy_f;
	PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
	RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);


	//Display everything again.
	t.inputsys.dowaypointview = 0;
	
	//Restore.
	waypoint_restore();
	t.gridentityhidemarkers = 0;
	editor_updatemarkervisibility();
	editor_refresheditmarkers();

	//  put all entities back where they were
	for (t.tcce = 1; t.tcce <= g.entityelementlist; t.tcce++)
	{
		t.tccentid = t.entityelement[t.tcce].bankindex;
		if (t.tccentid > 0)
		{
			t.tccsourceobj = t.entityelement[t.tcce].obj;
			if (ObjectExist(t.tccsourceobj) == 1)
			{
				PositionObject(t.tccsourceobj, t.entityelement[t.tcce].x, t.entityelement[t.tcce].y, t.entityelement[t.tcce].z);
			}
		}
	}


	g.globals.speedshadows = 0; //We need all cascades.
	terrain_shadowupdate(); //Remove preview objects shadow from cascades.
	g.globals.speedshadows = iOldSpeedshadows;

	//Turn off custom clear color.
	custom_back_color[0] = 0.0f; custom_back_color[1] = 0.0f; custom_back_color[2] = 0.0f; custom_back_color[3] = 0.0f;

	bImGuiInTestGame = true;
	FastSync();
	bImGuiInTestGame = false;

}


void CheckTooltipObjectDelete(void)
{
	if (!iTooltipAlreadyLoaded) {

		t.tentitytoselect = iTooltipLastObjectId;
		t.entobj = g.entitybankoffset + iTooltipLastObjectId;
		//entity_deleteentityfrommap(); //We dont actually have a entityelement
		if (ObjectExist(g.entitybankoffset + iTooltipLastObjectId)) {
			DeleteObject(g.entitybankoffset + iTooltipLastObjectId);
		}
		iTooltipLastObjectId = 0;
	}
}

void get_tutorials(void)
{
	cStr tOldDir = GetDir();

	tutorial_files.clear();
	
	SetDir("tutorialbank");

	ChecklistForFiles();
	for (int i = 1; i <= ChecklistQuantity(); i++)
	{
		if (ChecklistValueA(i) == 0)
		{
			cstr file_s = ChecklistString(i);
			if (cstr(Left(file_s.Get(), 1)) != ".")
			{
				cstr ext = Lower(Right(file_s.Get(), 4));
				if (ext == ".tut") 
				{
					//Read file and get tut: entry.
					//Read in TUT: entrie.
					FILE* fTut = GG_fopen(file_s.Get(), "r");
					if (fTut)
					{
						bool bVideoAdded = false;
						bool bTutorialAdded = false;
						bool bDescriptionAdded = false;
						char ctmp[TUTORIALMAXTEXT];
						char cVideoPath[MAX_PATH] = "\0";
						char cTutorialSet[TUTORIALMAXTEXT] = "\0";
						char cTutorialDescription[TUTORIALMAXTEXT] = "\0";
						while (!feof(fTut))
						{
							fgets(ctmp, TUTORIALMAXTEXT - 1, fTut);
							if (strlen(ctmp) > 0 && ctmp[strlen(ctmp) - 1] == '\n')
								ctmp[strlen(ctmp) - 1] = 0;

							if (strncmp(ctmp, "TUT:", 4) == 0)
							{
								if (!strlen(cTutorialName) > 0)
									strcpy(cTutorialName, &ctmp[5]);

								strcpy(cTutorialSet, &ctmp[5]);
								cstr path = "tutorialbank\\";
								path += file_s;
								tutorial_files.insert(std::make_pair(&ctmp[5], path.Get()));
								bTutorialAdded = true;
							}
							if (strncmp(ctmp, "DESC:", 5) == 0)
							{
								bDescriptionAdded = true;
								strcpy(cTutorialDescription, &ctmp[6]);
								std::string clean_string = cTutorialDescription;
								replaceAll(clean_string, "’", "'"); //Replace UTF8. 0xE2 0x90 0x99
								strcpy(cTutorialDescription, clean_string.c_str());
							}
							if (strncmp(ctmp, "VIDEO:", 6) == 0)
							{
								strcpy(cVideoPath, &ctmp[7]);
								SetDir(tOldDir.Get());
								char resolved[MAX_PATH];
								int retval = GetFullPathNameA(cVideoPath, MAX_PATH, resolved, NULL);
								if (retval > 0) {
									strcpy(cVideoPath, resolved);
								}
								SetDir("tutorialbank");
								bVideoAdded = true;
							}
							if (bTutorialAdded && bVideoAdded && bDescriptionAdded)
								break;

						}
						fclose(fTut);
						if (bTutorialAdded && bVideoAdded) {
							tutorial_videos.insert(std::make_pair(cTutorialSet, cVideoPath));
						}
						if (bTutorialAdded && bDescriptionAdded) {
							tutorial_description.insert(std::make_pair(cTutorialSet, cTutorialDescription));
						}
						if (bTutorialAdded && !bDescriptionAdded) {
							//If no desc , just add title.
							tutorial_description.insert(std::make_pair(cTutorialSet, cTutorialSet));
						}

						if (bTutorialAdded && !bVideoAdded) {
							//Add default video path. must always be the same as tutorial_files
							tutorial_videos.insert(std::make_pair(cTutorialSet, ""));
						}
					}
				}
			}
		}
	}
	SetDir(tOldDir.Get());

}
#endif

void generic_preloadfiles(void)
{
	//PE: We might have to edit this list when we have the final media to use.
	timestampactivity(0, "preload generic textures early");
	image_preload_files_start();

	image_preload_files_add("effectbank\\reloaded\\media\\water.dds");
	
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\fireball\\fireball_D.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\fireball\\fireball_N.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\fireball\\fireball_S.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\magicbolt_D.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\magicbolt_N.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\magicbolt_S.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\explode.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\smoke.dds");
	image_preload_files_add("gamecore\\projectiletypes\\fantasy\\magicbolt\\trail.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\handgrenade\\handgrenade_D.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\handgrenade\\handgrenade_N.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\handgrenade\\handgrenade_S.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\rpggrenade\\rpggrenade_D.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\rpggrenade\\rpggrenade_N.dds");
	image_preload_files_add("gamecore\\projectiletypes\\modern\\rpggrenade\\rpggrenade_S.dds");

	image_preload_files_add("skybank\\clear\\clear_F.dds",1);
	image_preload_files_add("skybank\\clear\\clear_L.dds",1);
	image_preload_files_add("skybank\\clear\\clear_U.dds",1);
	image_preload_files_add("skybank\\clear\\clear_B.dds",1);
	image_preload_files_add("skybank\\clear\\clear_R.dds",1);
	image_preload_files_add("skybank\\clear\\clear_D.dds",1);
	image_preload_files_add("skybank\\clear\\clear_CLOUDS.dds",1);
	image_preload_files_add("skybank\\cloudportal.dds",1);

	image_preload_files_add("effectbank\\explosion\\animatedspark.dds");
	image_preload_files_add("effectbank\\explosion\\explosion2.dds");
	image_preload_files_add("effectbank\\explosion\\fireball.dds");
	image_preload_files_add("effectbank\\explosion\\rollingsmoke.dds");
	image_preload_files_add("effectbank\\explosion\\explosion3.dds");
	image_preload_files_add("effectbank\\explosion\\darksmoke.dds");
	image_preload_files_add("effectbank\\explosion\\rubble.dds");
	image_preload_files_add("effectbank\\explosion\\concretechunk.dds");
	image_preload_files_add("effectbank\\explosion\\metalchunk.dds");

	image_preload_files_add("effectbank\\particles\\flare.dds");
	image_preload_files_add("effectbank\\particles\\64smoke2.dds");
	image_preload_files_add("effectbank\\particles\\flame.dds");
	
	//At this point.
	//Thread Job Done Time: 1657 
	//Add more know textures we need later.

	image_preload_files_add("terrainbank\\lush\\texture_D.dds");
	image_preload_files_add("effectbank\\reloaded\\media\\blank_N.dds");
	image_preload_files_add("effectbank\\reloaded\\media\\circle2.dds");
	image_preload_files_add("vegbank\\weedy 01\\grass.dds");


	image_preload_files_add("effectbank\\reloaded\\media\\blank_none_S.dds");
	image_preload_files_add("effectbank\\reloaded\\media\\blank_O.dds");
	image_preload_files_add("effectbank\\reloaded\\media\\materials\\0_Gloss.dds");
	image_preload_files_add("editors\\gfx\\cursor.dds");

	

	//PNG Test.
	image_preload_files_add("languagebank\\english\\artwork\\quick-start-testlevel-prompt.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\quick-help.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\testgamelayout-vr.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\testgamelayout-noweapons.png",1);

	image_preload_files_add("languagebank\\english\\artwork\\gurumeditation.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\gurumeditationoff.png",1);

	image_preload_files_add("editors\\gfx\\memorymeter.png",1);
	image_preload_files_add("editors\\gfx\\4.bmp",1);
	image_preload_files_add("editors\\gfx\\5.bmp",1);
	image_preload_files_add("editors\\gfx\\13.bmp",1);
	image_preload_files_add("editors\\gfx\\26.bmp",1);

	image_preload_files_add("editors\\gfx\\9.png",1);
	image_preload_files_add("editors\\gfx\\14.png",1);
	image_preload_files_add("editors\\gfx\\18.png",1);

	image_preload_files_add("languagebank\\english\\artwork\\f9-help-terrain.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\f9-help-entity.png",1);
	image_preload_files_add("languagebank\\english\\artwork\\f9-help-conkit.png",1);

	///image_preload_files_add("languagebank\\neutral\\gamecore\\huds\\interactive\\close-highlight.png",1);

	image_preload_files_add("editors\\gfx\\resources.png",1);
	image_preload_files_add("editors\\gfx\\resourceslow.png",1);

	image_preload_files_add("editors\\gfx\\resourcesgone.png",1);
	image_preload_files_add("editors\\gfx\\resourcesworking.png",1);


	image_preload_files_finish();
}

#ifdef ENABLEIMGUI
void CloseDownEditorProperties(void)
{
	if (t.gridentityinzoomview > 0) 
	{
		t.tpressedtoleavezoommode = 2; //Exit zoom and save.
		int olges = t.grideditselect;
		//Make sure to exit fast. and restore cursor object.
		int igridentity = t.gridentity;
		if (iOldgridentity != t.gridentity && iOldgridentity > -1)
			t.gridentity = iOldgridentity;

		t.grideditselect = 4;
		editor_viewfunctionality();
		t.grideditselect = olges;
		t.gridentity = igridentity;
	}
}

void FormatTTS(LPSTR pFormattedTTS, LPSTR pFormattedTTSOut)
{
	memset(pFormattedTTSOut, 0, 1000);
	int nout = 0;
	for (int n = 0; n < strlen(pFormattedTTS); n++)
	{
		if ( pFormattedTTS[n] >= 32 && pFormattedTTS[n] <= 255 )
			pFormattedTTSOut[nout++] = pFormattedTTS[n];
	}
	pFormattedTTSOut[nout] = 0;
}

bool g_bVoiceSettingsChanged = false;
int g_iVoiceSettingsUpdateSpeechID = 0;

#ifdef VRTECH
void SpeechControls(int speech_entries, bool bUpdateMainString, entityeleproftype *edit_grideleprof)
{
	if (!edit_grideleprof)
	{
		edit_grideleprof = &t.grideleprof;
	}

	if (ImGui::StyleCollapsingHeader("Speech Control", ImGuiTreeNodeFlags_DefaultOpen)) {

		bool sapi_available = false;
		if (g_voiceList_s.size() > 0)
			sapi_available = true;

		ImGui::Indent(10);
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));

		if (sapi_available) 
		{
			ImGui::Text("Voice");
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
			//Combo
			ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
			ImGui::PushItemWidth(-10);
			if (ImGui::BeginCombo("##SelectVoiceCCP", pCCPVoiceSet)) // The second parameter is the label previewed before opening the combo.
			{
				int size = g_voiceList_s.size();
				for (int vloop = 0; vloop < size; vloop++) {

					bool is_selected = false;
					if (strcmp(g_voiceList_s[vloop].Get(), pCCPVoiceSet) == 0)
						is_selected = true;

					if (ImGui::Selectable(g_voiceList_s[vloop].Get(), is_selected)) 
					{
						//Change Voice set
						pCCPVoiceSet = g_voiceList_s[vloop].Get();
						CCP_SelectedToken = g_voicetoken[vloop];
						edit_grideleprof->voiceset_s = pCCPVoiceSet;
						if (g_bVoiceSettingsChanged == false)
						{
							g_bVoiceSettingsChanged = true;
							g_iVoiceSettingsUpdateSpeechID = 0;
						}
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Select Voice to Use For Speak");

			ImGui::PopItemWidth();

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
			ImGui::Text("Rate");
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));

			ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
			ImGui::PushItemWidth(-10);
			int iOldRate = CCP_Speak_Rate;
			ImGui::SliderInt("##speakrate", &CCP_Speak_Rate, -5, 5);
			if (CCP_Speak_Rate != iOldRate)
			{
				edit_grideleprof->voicerate = CCP_Speak_Rate;
				if (g_bVoiceSettingsChanged == false)
				{
					g_bVoiceSettingsChanged = true;
					g_iVoiceSettingsUpdateSpeechID = 0;
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Set Speak Rate");

			ImGui::PopItemWidth();

		}
		//LOOP speech_entries
		//Use unique IDs

		// monitor lip sync generator
		bool bLipSyncGenerationBusy = false;
		float fProgressOfGeneration = 0;//cyb // GetWAVtoLIPProgress();
		if (fProgressOfGeneration > 0.0f && fProgressOfGeneration < 1.0f)
			bLipSyncGenerationBusy = true;

		for (int SpeechLoop = 0; SpeechLoop < speech_entries; SpeechLoop++)
		{
			if (speech_ids[SpeechLoop] >= 0)
			{
				cstr tmpvar = edit_grideleprof->PropertiesVariable.Variable[speech_ids[SpeechLoop]];
				tmpvar = tmpvar.Lower();

				//Display soundset file entry.
				int iButtonControlAndState = 0; // 0-can edit button, 1-buttom used, 2-background task in progress (so should not press button until done)
				if ( bLipSyncGenerationBusy == true ) iButtonControlAndState = 2;
				LPSTR pButtonControlIfBlocked = "Still Generating Lip Sync Data";
				cstr used_soundset;
				if (tmpvar == "speech1" || tmpvar == "speech 1") 
				{
					edit_grideleprof->soundset1_s = imgui_setpropertyfile2_ex_dlua(t.group, edit_grideleprof->soundset1_s.Get(), "SPEECH 1", t.strarr_s[254].Get(), "audiobank\\", &iButtonControlAndState, pButtonControlIfBlocked );
					used_soundset = edit_grideleprof->soundset1_s;
				}
				else if (tmpvar == "speech2" || tmpvar == "speech 2") 
				{
					edit_grideleprof->soundset2_s = imgui_setpropertyfile2_ex_dlua(t.group, edit_grideleprof->soundset2_s.Get(), "SPEECH 2", t.strarr_s[254].Get(), "audiobank\\", &iButtonControlAndState, pButtonControlIfBlocked );
					used_soundset = edit_grideleprof->soundset2_s;
				}
				else if (tmpvar == "speech3" || tmpvar == "speech 3") 
				{
					edit_grideleprof->soundset3_s = imgui_setpropertyfile2_ex_dlua(t.group, edit_grideleprof->soundset3_s.Get(), "SPEECH 3", t.strarr_s[254].Get(), "audiobank\\", &iButtonControlAndState, pButtonControlIfBlocked );
					used_soundset = edit_grideleprof->soundset3_s;
				}
				else if (tmpvar == "speech0" || tmpvar == "speech 0") 
				{
					edit_grideleprof->soundset_s = imgui_setpropertyfile2_ex_dlua(t.group, edit_grideleprof->soundset_s.Get(), "SPEECH", t.strarr_s[254].Get(), "audiobank\\", &iButtonControlAndState, pButtonControlIfBlocked );
					used_soundset = edit_grideleprof->soundset_s;
				}
				else 
				{
					edit_grideleprof->soundset_s = imgui_setpropertyfile2_ex_dlua(t.group, edit_grideleprof->soundset_s.Get(), "SPEECH 4", t.strarr_s[254].Get(), "audiobank\\", &iButtonControlAndState, pButtonControlIfBlocked );
					used_soundset = edit_grideleprof->soundset_s;
				}
				if (iButtonControlAndState == 1)
				{
					// user changed one of the speech fields, so generate LIP file for it
					//ConvertWAVtoLIP(used_soundset.Get()); //cyb
				}

				std::string uniquiField = ">";
				uniquiField = uniquiField + "##";
				uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFontSize()*2.0);

				int iButImageSize = 16;
				ImGui::PushID(grideleprof_uniqui_id++);
				if (ImGui::ImgBtn(MEDIA_PLAY, ImVec2(iButImageSize, iButImageSize), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false, false, false, false, true, bBoostIconColors))
				{
					// the sound we will use for the preview
					bool bJustStopped = false;
					int iFreeSoundID = g.temppreviewsoundoffset;
					if (SoundExist(iFreeSoundID) == 1 && SoundPlaying(iFreeSoundID) == 1)
					{
						// stop currently playing preview
						StopSound(iFreeSoundID);
						bJustStopped = true;
					}
					if (used_soundset.Len() > 0) 
					{
						// play custom wav file directly.
						if (SoundExist(iFreeSoundID) == 1) DeleteSound(iFreeSoundID);
						if (FileExist(used_soundset.Get()) == 1 && bJustStopped==false )
						{
							LoadSound(used_soundset.Get(), iFreeSoundID, 0, 1);
							if ( SoundExist(iFreeSoundID)==1 )
								PlaySound(iFreeSoundID);
						}
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Play");
				ImGui::PopItemWidth();
				ImGui::PopID();

				// Recording button
				static bool g_bRecordingSound = false;
				static cstr g_recordingFile_s;
				uniquiField = "o";
				uniquiField = uniquiField + "##";
				uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);
				ImGui::SameLine();
				ImGui::PushItemWidth(ImGui::GetFontSize()*2.0);
				ImGui::PushID(grideleprof_uniqui_id++);
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() - 6.0, ImGui::GetCursorPosY()));
				int iRecordBtnImg = MEDIA_RECORD;
				if (g_bRecordingSound==true) iRecordBtnImg = MEDIA_RECORDING;
				if (bLipSyncGenerationBusy==true) iRecordBtnImg = MEDIA_RECORDPROCESSING;
				if (ImGui::ImgBtn(iRecordBtnImg, ImVec2(iButImageSize, iButImageSize), drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true, false, false, false, true))
				{
					if (g_bRecordingSound == false)
					{
						// can only start recording once any lip sync progress has finished
						if (bLipSyncGenerationBusy == true)
						{
							MessageBoxA(NULL, "Cannot start recording until lip sync generation finished", "Notification", MB_OK);
						}
						else
						{
							// choose a unique recording name
							int iRecordingNum = 1;
							while (iRecordingNum < 9999)
							{
								// find a free recording WAV filename
								g_recordingFile_s = cstr("audiobank\\recordings\\Recording-") + cstr(iRecordingNum) + ".wav";
								if (FileExist(g_recordingFile_s.Get()) == 0)
									break;
								iRecordingNum++;
							}

							// start recording
							cstr absWAVPath_s = g.fpscrootdir_s + "\\Files\\" + g_recordingFile_s;
							char pRealAbsWAVForRecording[MAX_PATH];
							strcpy(pRealAbsWAVForRecording, absWAVPath_s.Get());
							GG_GetRealPath(pRealAbsWAVForRecording, 1);
							//RecordWAV(pRealAbsWAVForRecording); //cyb
							g_bRecordingSound = true;
						}
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Record");
				ImGui::PopItemWidth();
				ImGui::PopID();
				if (g_bRecordingSound == true)
				{
					if (0) //cyb //( RecordWAVProgress() >= 1.0f && bLipSyncGenerationBusy == false )
					{
						// end recording mode
						g_bRecordingSound = false;

						// stop recording and save
						if (tmpvar == "speech1" || tmpvar == "speech 1") edit_grideleprof->soundset1_s = g_recordingFile_s;
						if (tmpvar == "speech2" || tmpvar == "speech 2") edit_grideleprof->soundset2_s = g_recordingFile_s;
						if (tmpvar == "speech3" || tmpvar == "speech 3") edit_grideleprof->soundset3_s = g_recordingFile_s;
						if (tmpvar == "speech0" || tmpvar == "speech 0") edit_grideleprof->soundset_s = g_recordingFile_s;

						// generate LIP file from recording
						cstr absWAVPath_s = g.fpscrootdir_s + "\\Files\\" + g_recordingFile_s;
						//ConvertWAVtoLIP(absWAVPath_s.Get()); //cyb
						bLipSyncGenerationBusy = true;
					}
				}

				if (sapi_available) 
				{
					//The edit_grideleprof->soundset_s to use is already known so:
					//Store the actual text entered in the DLUA value fields.
					ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
					ImGui::PushItemWidth(-10);

					ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());

					uniquiField = "";
					uniquiField = uniquiField + "##speakTTStext";
					uniquiField = uniquiField + std::to_string(grideleprof_uniqui_id++);

					// detect any editing of TTS text
					static bool bDetectWhenFinishedEditingTTSText = false;
					if (ImGui::InputTextMultiline(uniquiField.c_str(), &edit_grideleprof->PropertiesVariable.VariableValue[speech_ids[SpeechLoop]][0], 1024, ImVec2(0, ImGui::GetFontSize()*3.0f)))
					{
						// flagged any time TTS text changes
						bDetectWhenFinishedEditingTTSText = true;
					}
					if (ImGui::MaxIsItemFocused()) bImGuiGotFocus = true;

					// can also trigger regeneration of text when Voice Settings changed
					if (g_bVoiceSettingsChanged == true)
					{
						// go through all speeches and regeneate them one by one
						float fProgressOfGeneration = 0;//cyb // GetWAVtoLIPProgress();
						if (fProgressOfGeneration > 0.0f && fProgressOfGeneration < 1.0f)
						{
							// but if lip sync busy, we wait until free!
						}
						else
						{
							// when speech loop comes around to the next speech to refresh, trigger it now
							if (g_iVoiceSettingsUpdateSpeechID == SpeechLoop)
							{
								// regenerate this speech TTS
								bDetectWhenFinishedEditingTTSText = true;
								g_iVoiceSettingsUpdateSpeechID++;
								if (g_iVoiceSettingsUpdateSpeechID >= speech_entries)
								{
									// when no more speeches to refresh, finish voice settings cascade refresh
									g_bVoiceSettingsChanged = false;
								}
							}
						}
					}
					// only proceed when LIP sync not busy
					bool bLipSyncBusy = false;
					float fProgressOfGeneration = 0;//cyb GetWAVtoLIPProgress();
					if (fProgressOfGeneration > 0.0f && fProgressOfGeneration < 1.0f) bLipSyncBusy = true;
					if (ImGui::IsItemActive() == false && bDetectWhenFinishedEditingTTSText==true && bLipSyncBusy==false)
					{
						// and reset as we are now doing the conversion and LIP file creation below
						bDetectWhenFinishedEditingTTSText = false;

						// this is the text we want to turn into WAV
						cstr TTSText_s = edit_grideleprof->PropertiesVariable.VariableValue[speech_ids[SpeechLoop]];

						// first, create a location to store the level-based TTS recordings
						// which will be in levelbank\ttsfiles\*.wav (keeps them local to FPM and transportable to Players)
						cstr pOldDir = GetDir();
						char pRealRoot[MAX_PATH];
						strcpy(pRealRoot, g.fpscrootdir_s.Get());
						strcat(pRealRoot, "\\Files\\levelbank\\");
						GG_GetRealPath(pRealRoot, 1);
						SetDir(pRealRoot);
						if (PathExist("testmap") == 0) MakeDirectory("testmap");
						SetDir("testmap");
						if (PathExist("ttsfiles") == 0) MakeDirectory("ttsfiles");
						SetDir("ttsfiles");

						// format typed text into something we can store as a reference in the TTS table below
						char pFormattedTTS[1000];
						int iInputTextMax = TTSText_s.Len();
						if (iInputTextMax > 999) iInputTextMax = 999;
						memcpy(pFormattedTTS, TTSText_s.Get(), iInputTextMax);
						pFormattedTTS[iInputTextMax] = 0;
						strcat(pFormattedTTS, pCCPVoiceSet);
						strcat(pFormattedTTS, cstr('A'+CCP_Speak_Rate).Get());
						char pFormattedTTSOut_Keeper[1000];
						FormatTTS(pFormattedTTS, pFormattedTTSOut_Keeper);

						// prepare two absolute paths for later
						char pRelLocationOfWAV[MAX_PATH];
						strcpy(pRelLocationOfWAV, "levelbank\\testmap\\ttsfiles\\"); // WAV added below

						// load in TTS table to see what TTS text we already have recordings for
						bool bIsTTSUnique = true;
						int iTTSTableMax = 0;
						std::vector <cstr> tempLines_s;
						Dim ( tempLines_s, 9999 );
						LPSTR pTTSTableFile = "ttstable.txt";
						if (FileExist(pTTSTableFile) == 1)
						{
							OpenToRead(1,pTTSTableFile);
							while (FileEnd(1) == 0)
							{
								tempLines_s[iTTSTableMax] = ReadString(1);
								iTTSTableMax++;
							}
							iTTSTableMax--;
							CloseFile(1);
						}
						if (iTTSTableMax > 0)
						{
							for (int line = 0; line < iTTSTableMax; line++)
							{
								// get the ref part of this line (and the WAV part for later)
								char pWAVItem[1001];
								char pRefItem[1001];
								strcpy(pWAVItem, "");
								strcpy(pRefItem, tempLines_s[line].Get());
								for (int n = 0; n < strlen(pRefItem); n++)
								{
									if (pRefItem[n] == 9)
									{
										strcpy(pWAVItem, pRefItem+n+1);
										pRefItem[n] = 0;
										break;
									}
								}

								// does it match what we are looking to add in
								if (stricmp(pRefItem, pFormattedTTSOut_Keeper) == NULL)
								{
									// yes, found the TTS we want already in the table
									if (FileExist(pWAVItem) == 1)
									{
										strcat(pRelLocationOfWAV, pWAVItem);
										bIsTTSUnique = false;
										break;
									}
								}
							}
						}

						// determine if what we have is unique
						if ( bIsTTSUnique == true )
						{
							// if so, create a unique file name entry for this one
							cstr pTTSFile;
							int iTTSNum = 1;
							while (iTTSNum < 9999)
							{
								pTTSFile = cstr("TTS") + cstr(iTTSNum) + ".wav";
								if (FileExist(pTTSFile.Get()) == 0)
									break;
								else
									iTTSNum++;
							}
							if (iTTSNum <= 9999)
							{
								// complete path new TTS WAV
								strcat(pRelLocationOfWAV, pTTSFile.Get());

								// add to table for future reference and potential use
								LPSTR pTTSTableFile = "ttstable.txt";
								if (FileExist(pTTSTableFile) == 1) DeleteFileA(pTTSTableFile);
								OpenToWrite(1,pTTSTableFile);
								for ( int i = 0; i<iTTSTableMax; i++ )
								{
									WriteString(1, tempLines_s[i].Get());
								}
								char pNewLine[1000];
								strcpy(pNewLine, pFormattedTTSOut_Keeper);
								pNewLine[strlen(pFormattedTTSOut_Keeper)+0] = 9;
								pNewLine[strlen(pFormattedTTSOut_Keeper)+1] = 0;
								strcat(pNewLine, pTTSFile.Get());
								WriteString(1,pNewLine);
								CloseFile(1);

								// need to be back in Files\\ folder for conversion to work properly
								SetDir(pOldDir.Get());

								// turn TEXT into WAV, store in this folder
								LPSTR pWAVFilename = pRelLocationOfWAV;
								if (FileExist(pWAVFilename)) DeleteFileA(pWAVFilename);
								LPSTR pWhatToSay = TTSText_s.Get();
								CComPtr<ISpVoice> spVoice;
								HRESULT hr = spVoice.CoCreateInstance(CLSID_SpVoice);
								if (SUCCEEDED(hr))
								{
									hr = spVoice->SetVoice(CCP_SelectedToken);
									if (SUCCEEDED(hr))
									{
										char pFinalWAVFilename[MAX_PATH];
										strcpy(pFinalWAVFilename, pWAVFilename);
										//ConvertTXTtoWAVMeatyPart(spVoice, CCP_SelectedToken, CCP_Speak_Rate, pWhatToSay, pFinalWAVFilename); //cyb
									}
								}
							}
						}
						else
						{
							// if not, the 'pRelLocationOfWAV' carries the previous WAV we can use from the TTS table
						}

						// change field of SPEECH X to [use text speech] - indicating its using the internal TTS wav created
						if (tmpvar == "speech1" || tmpvar == "speech 1") edit_grideleprof->soundset1_s = pRelLocationOfWAV;
						if (tmpvar == "speech2" || tmpvar == "speech 2") edit_grideleprof->soundset2_s = pRelLocationOfWAV;
						if (tmpvar == "speech3" || tmpvar == "speech 3") edit_grideleprof->soundset3_s = pRelLocationOfWAV;
						if (tmpvar == "speech0" || tmpvar == "speech 0") edit_grideleprof->soundset_s = pRelLocationOfWAV;

						// restore original folder for LIP file creation
						SetDir(pOldDir.Get());

						// can begin the WAV to LIP file now as we have the WAV file created
						//ConvertWAVtoLIP(pRelLocationOfWAV); //cyb

						// and before we leave, take the opportunity to scan ALL entities and see if there are
						// any entries in the TTS table (and associated WAVs) that are not needed (probably due to recent change above)
						char pRealTTSFilesFolder[MAX_PATH];
						strcpy(pRealTTSFilesFolder, g.fpscrootdir_s.Get());
						strcat(pRealTTSFilesFolder, "\\Files\\levelbank\\testmap\\");
						SetDir(pRealTTSFilesFolder);
						if (PathExist("ttsfiles") == 0) MakeDirectory("ttsfiles");
						SetDir("ttsfiles");

						// load in latest TTS (given above activity of possible addition of new one)
						std::vector <cstr> tempRefs_s;
						Dim ( tempRefs_s, 9999 );
						iTTSTableMax = 0;
						pTTSTableFile = "ttstable.txt";
						if (FileExist(pTTSTableFile) == 1)
						{
							OpenToRead(1,pTTSTableFile);
							while (FileEnd(1) == 0)
							{
								char pRefItem[1001];
								tempLines_s[iTTSTableMax] = ReadString(1);
								strcpy ( pRefItem, tempLines_s[iTTSTableMax].Get() );
								for (int n = 0; n < strlen(pRefItem); n++)
								{
									if (pRefItem[n] == 9)
									{
										pRefItem[n] = 0;
										break;
									}
								}
								tempRefs_s[iTTSTableMax] = pRefItem;
								iTTSTableMax++;
							}
							iTTSTableMax--;
							CloseFile(1);
						}

						// start an array to hold flag as to whether to keep TTS table entry
						// and go through to auto-accept the one we've just added above (so it does not get deleted)
						bool* pbKeepInTable = new bool[iTTSTableMax+1];
						for (int iTTS = 0; iTTS < iTTSTableMax; iTTS++)
						{
							pbKeepInTable[iTTS] = false;
							if (stricmp(pFormattedTTSOut_Keeper, tempRefs_s[iTTS].Get()) == NULL)
								pbKeepInTable[iTTS] = true;
						}

						// go through ALL entities in current level (and all speech TTS texts buried in each one)
						for ( int e2 = 1; e2 <= g.entityelementlist; e2++ )
						{
							// need voice and speak rate from this entity
							LPSTR pVoiceSet2 = t.entityelement[e2].eleprof.voiceset_s.Get();
							int iSpeakRate2 = t.entityelement[e2].eleprof.voicerate;
							if (strlen(pVoiceSet2) == 0)
							{
								// default to first voice at startard rate
								pVoiceSet2 = g_voiceList_s[0].Get();
								iSpeakRate2 = 0;
							}

							int speech_ids2[5];
							for ( int n2 = 0; n2 < 5; n2++ ) speech_ids2[n2] = -1;
							int speech_entries2 = 0;
							for (int i2 = 0; i2 < t.entityelement[e2].eleprof.PropertiesVariable.iVariables; i2++)
							{
								cstr tmpvar = t.entityelement[e2].eleprof.PropertiesVariable.Variable[i2];
								tmpvar = tmpvar.Lower();
								if (speech_entries2 <= 3)
								{
									if (tmpvar == "speech1" || tmpvar == "speech 1") speech_ids2[speech_entries2++] = i2;
									if (tmpvar == "speech2" || tmpvar == "speech 2") speech_ids2[speech_entries2++] = i2;
									if (tmpvar == "speech3" || tmpvar == "speech 3") speech_ids2[speech_entries2++] = i2;
									if (tmpvar == "speech0" || tmpvar == "speech 0") speech_ids2[speech_entries2++] = i2;
								}
							}
							for (int iSpeechLoop2 = 0; iSpeechLoop2 < 4; iSpeechLoop2++)
							{
								LPSTR pSpeechItem = NULL;
								if (speech_ids2[iSpeechLoop2] != -1)
								{
									if (iSpeechLoop2 == 0) pSpeechItem = t.entityelement[e2].eleprof.PropertiesVariable.VariableValue[speech_ids2[iSpeechLoop2]];
									if (iSpeechLoop2 == 1) pSpeechItem = t.entityelement[e2].eleprof.PropertiesVariable.VariableValue[speech_ids2[iSpeechLoop2]];
									if (iSpeechLoop2 == 2) pSpeechItem = t.entityelement[e2].eleprof.PropertiesVariable.VariableValue[speech_ids2[iSpeechLoop2]];
									if (iSpeechLoop2 == 3) pSpeechItem = t.entityelement[e2].eleprof.PropertiesVariable.VariableValue[speech_ids2[iSpeechLoop2]];
									if (pSpeechItem)
									{
										// ensure the TTS string is formatted so can be matched against table (who's refs are formatted) [nice to do a hash here?]
										char pSpeechItemFormatted[1000];
										strcpy(pSpeechItemFormatted, pSpeechItem);
										strcat(pSpeechItemFormatted, pVoiceSet2);
										strcat(pSpeechItemFormatted, cstr('A'+iSpeakRate2).Get());
										char pSpeechItemFormattedOut[1000];
										FormatTTS(pSpeechItemFormatted, pSpeechItemFormattedOut);

										// a TTS text in one of the entity sound slots
										for (int iTTS = 0; iTTS < iTTSTableMax; iTTS++)
										{
											// compare the table entry with this entities TTS
											if (stricmp(pSpeechItemFormattedOut, tempRefs_s[iTTS].Get()) == NULL)
											{
												// found a match, flag this table entry as a keeper
												pbKeepInTable[iTTS] = true;
											}
										}
									}
								}
							}
						}
						// finally create a new table of only the keepers
						if (iTTSTableMax > 0)
						{
							// before table creation, check for duplicates (can happen when editing an entity over and over)
							for (int i1 = 0; i1 < iTTSTableMax; i1++)
							{
								if (pbKeepInTable[i1] == true)
								{
									for (int i2 = 0; i2 < iTTSTableMax; i2++)
									{
										if (i1 != i2)
										{
											if (pbKeepInTable[i2] == true)
											{
												// comparing two entries, both valid and not same index
												// ensure primary (i) survives at expense of any identical entries (i2)
												LPSTR i1Ref = tempRefs_s[i1].Get();
												LPSTR i2Ref = tempRefs_s[i2].Get();
												if (stricmp(i1Ref, i2Ref) == NULL)
												{
													// remove duplicate
													pbKeepInTable[i2] = false;

													// also delete WAV and LIP files associated with this duplicate
													char pWAVItem[1001];
													strcpy(pWAVItem, "");
													char pRefItem[1001];
													strcpy(pRefItem, tempLines_s[i2].Get());
													for (int n = 0; n < strlen(pRefItem); n++)
													{
														if (pRefItem[n] == 9)
														{
															strcpy(pWAVItem, pRefItem+n+1);
															break;
														}
													}
													if (FileExist(pWAVItem) == 1) DeleteFileA(pWAVItem);
													char pLIPItem[1001];
													strcpy(pLIPItem, pWAVItem); 
													pLIPItem[strlen(pLIPItem) - 4] = 0;
													strcat(pLIPItem, ".lip");
													if (FileExist(pLIPItem) == 1) DeleteFileA(pLIPItem);

													// and finally reroute entities that used this WAV to the primary one
													for (int e2 = 1; e2 <= g.entityelementlist; e2++)
													{
														for (int s2 = 0; s2 < 4; s2++)
														{
															LPSTR pThisSlotsWAV = NULL;
															if (s2 == 0) pThisSlotsWAV = t.entityelement[e2].eleprof.soundset_s.Get();
															if (s2 == 1) pThisSlotsWAV = t.entityelement[e2].eleprof.soundset1_s.Get();
															if (s2 == 2) pThisSlotsWAV = t.entityelement[e2].eleprof.soundset2_s.Get();
															if (s2 == 3) pThisSlotsWAV = t.entityelement[e2].eleprof.soundset3_s.Get();
															if (pThisSlotsWAV)
															{
																if (stricmp(pThisSlotsWAV, pWAVItem) == NULL)
																{
																	cstr newWAV_s = (LPSTR)tempLines_s[i1].Get()+strlen(tempRefs_s[i1].Get());
																	if (s2 == 0) t.entityelement[e2].eleprof.soundset_s = newWAV_s;
																	if (s2 == 1) t.entityelement[e2].eleprof.soundset1_s = newWAV_s;
																	if (s2 == 2) t.entityelement[e2].eleprof.soundset2_s = newWAV_s;
																	if (s2 == 3) t.entityelement[e2].eleprof.soundset3_s = newWAV_s;
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}

							// final creation of table
							LPSTR pTTSTableFile = "ttstable.txt";
							if (FileExist(pTTSTableFile) == 1) DeleteFileA(pTTSTableFile);
							OpenToWrite(1, pTTSTableFile);
							for (int i = 0; i < iTTSTableMax; i++)
							{
								if (pbKeepInTable[i] == true)
								{
									// keep this in table
									WriteString(1, tempLines_s[i].Get());
								}
								else
								{
									// remove from table - and also delete the associated WAV file
									char pWAVItem[1001];
									strcpy(pWAVItem, "");
									char pRefItem[1001];
									strcpy(pRefItem, tempLines_s[i].Get());
									for (int n = 0; n < strlen(pRefItem); n++)
									{
										if (pRefItem[n] == 9)
										{
											strcpy(pWAVItem, pRefItem+n+1);
											break;
										}
									}
									if (FileExist(pWAVItem) == 1) DeleteFileA(pWAVItem);
									char pLIPItem[1001];
									strcpy(pLIPItem, pWAVItem); 
									pLIPItem[strlen(pLIPItem) - 4] = 0;
									strcat(pLIPItem, ".lip");
									if (FileExist(pLIPItem) == 1) DeleteFileA(pLIPItem);
								}
							}
							CloseFile(1);
						}

						// and free resources
						SAFE_DELETE(pbKeepInTable);

						// restore original folder when finished
						SetDir(pOldDir.Get());

						// trigger a main string update
						bUpdateMainString = true;
					}

					if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Enter Text to Speak");

					ImGui::PopItemWidth();

				}
			}
		}

		//Update DLUA if changed. this is stored in soundset4_s.
		//Update soundset4_s when we have changes.
		entityeleproftype *tmpeleprof = edit_grideleprof;
		if (bUpdateMainString) 
		{
			cstr sLuaScriptName = tmpeleprof->PropertiesVariable.VariableScript;
			sLuaScriptName += "_properties(";
			//Check if we need to update with new default values.
			if (tmpeleprof->PropertiesVariable.iVariables > 0) 
			{
				tmpeleprof->soundset4_s = sLuaScriptName;
				//Add varables.
				for (int i = 0; i < tmpeleprof->PropertiesVariable.iVariables; i++) 
				{

					char val[3];
					val[0] = tmpeleprof->PropertiesVariable.VariableType[i] + '0';
					val[1] = 0;

					tmpeleprof->soundset4_s += val;
					tmpeleprof->soundset4_s += "\"";
					std::string clean_string = tmpeleprof->PropertiesVariable.VariableValue[i];
					replaceAll(clean_string, "\"", ""); //cant use "
					tmpeleprof->soundset4_s += (char *) clean_string.c_str();
					//tmpeleprof->soundset4_s += tmpeleprof->PropertiesVariable.VariableValue[i];
					tmpeleprof->soundset4_s += "\"";
					if (i < tmpeleprof->PropertiesVariable.iVariables - 1)
						tmpeleprof->soundset4_s += ",";
				}
				tmpeleprof->soundset4_s += ")";
			}
		}

		ImGui::Indent(-10);
	}
}
#endif

void RedockWindow(char *name)
{
	if (refresh_gui_docking >= 4 ) {

		ImGuiID dockspace_id;

		//dock it.
		int winNodeId = ImGui::GetWindowDockID();
		dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockBuilderDockWindow(name, dock_tools_windows);
		int winNodeId2 = ImGui::GetWindowDockID();
		if (winNodeId != 0 && winNodeId2 != 0 && winNodeId != winNodeId2) {
			//Somthing wrong we cant rebuild.
			//Change window size to normal and undock:
			//m_editor->resizewindownext = true;
		}
		else {
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}
}

int pehuntingbug = 0;
void CheckMinimumDockSpaceSize(float minsize)
{
	//Only make these change when not resizing/moving ...
	int mcursor = ImGui::GetMouseCursor();
	if (mcursor == 0 || !ImGui::IsAnyMouseDown() ) {
		if (ImGui::IsWindowDocked()) {
			int winNodeId = ImGui::GetWindowDockID();
			ImGuiDockNode* testnode = ImGui::DockBuilderGetNode(winNodeId);
			if (testnode->Size.x < minsize && testnode->Size.y > 80.0f ) {
				//Something wrong , adjust.
				if (minsize == 50.0f) {
					//PE: Found it this happens when you use minimize :) , just need to check for that.

					//PE: Bug left panel get small. add breakpoint here to check.
					pehuntingbug = 1;
				}
				//PE: When i got it this was the values ? 12,32
				//PE: testnode->SizeRef was the correct values.
				if (testnode->Size.x != 12 && testnode->Size.x != 13 && testnode->Size.y != 32) {
					testnode->Size.x = minsize;
					testnode->SizeRef.x = minsize;
				}
				//ImGui::DockNodeTreeUpdatePosSize(testnode, testnode->Pos, testnode->Size,false);
			}
			if (testnode->Size.y < 20.0f) {
				//Something wrong , adjust.
				if (minsize == 50.0f) {
					//PE: Bug left panel get small. add breakpoint here to check.
					pehuntingbug = 2;
				}
				if (testnode->Size.x != 12 && testnode->Size.x != 13 && testnode->Size.y != 32) {
					testnode->Size.y = 50.0f;
					testnode->SizeRef.y = 50.0f;
				}
				//ImGui::DockNodeTreeUpdatePosSize(testnode, host_window->Pos, host_window->Size);
			}
		}
	}
}
#endif



//PE: Using t.gridentityposx_f,t.gridentityposy_f,t.gridentityposz_f,t.gridentity,t.gridentityobj
void Add_Grid_Snap_To_Position(void)
{
	//  grid system for entities
	if (t.gridentitygridlock == 1)
	{
		//Snap.
		//  small grid lock for better alignments
		t.gridentityposx_f = (int(t.gridentityposx_f / 5) * 5);
		t.gridentityposz_f = (int(t.gridentityposz_f / 5) * 5);
		//  special snap-to when edge of entity gets near another
		if (t.gridentity > 0)
		{
			t.tobj = t.gridentityobj;
			if (t.tobj > 0)
			{
				if (ObjectExist(t.tobj) == 1 )
				{
					t.tsrcx_f = t.gridentityposx_f;
					t.tsrcy_f = t.gridentityposy_f;
					t.tsrcz_f = t.gridentityposz_f;
					t.tsrcradius_f = ObjectSize(t.tobj, 1);
					t.tfindclosest = -1; t.tfindclosestbest_f = 999999;
					for (t.e = 1; t.e <= g.entityelementlist; t.e++)
					{
						t.ttobj = t.entityelement[t.e].obj;
						if (t.ttobj > 0 && t.tobj != t.ttobj ) //PE: Never check ourself.
						{
							if (t.entityelement[t.e].bankindex == t.gridentity)
							{
								t.tdiffx_f = t.entityelement[t.e].x - t.tsrcx_f;
								t.tdiffy_f = t.entityelement[t.e].y - t.tsrcy_f;
								t.tdiffz_f = t.entityelement[t.e].z - t.tsrcz_f;
								t.tdiff_f = Sqrt(abs(t.tdiffx_f*t.tdiffx_f) + abs(t.tdiffy_f*t.tdiffy_f) + abs(t.tdiffz_f*t.tdiffz_f));
								t.tthisradius_f = ObjectSize(t.ttobj, 1);
								if (t.tdiff_f < t.tsrcradius_f + t.tthisradius_f && t.tdiff_f < t.tfindclosestbest_f)
								{
									t.tfindclosestbest_f = t.tdiff_f;
									t.tfindclosest = t.e;
								}
							}
						}
					}
					if (t.tfindclosest != -1)
					{
						//  go through 6 magnet points of the src entity
						t.tmag1sizex_f = ObjectSizeX(t.tobj, 1) / 2;
						t.tmag1sizey_f = ObjectSizeY(t.tobj, 1) / 2;
						t.tmag1sizez_f = ObjectSizeZ(t.tobj, 1) / 2;
						t.tmag2sizex_f = ObjectSizeX(t.entityelement[t.tfindclosest].obj, 1) / 2;
						t.tmag2sizey_f = ObjectSizeY(t.entityelement[t.tfindclosest].obj, 1) / 2;
						t.tmag2sizez_f = ObjectSizeZ(t.entityelement[t.tfindclosest].obj, 1) / 2;
						if (ObjectExist(g.entityworkobjectoffset) == 0) { MakeObjectCube(g.entityworkobjectoffset, 40); HideObject(g.entityworkobjectoffset); }
						t.tbestmag_f = 99999; t.tbestmag2id = -1;
						for (t.magid = 1; t.magid <= 6; t.magid++)
						{
							t.tmagx_f = t.gridentityposx_f;
							t.tmagy_f = t.gridentityposy_f;
							t.tmagz_f = t.gridentityposz_f;
							PositionObject(g.entityworkobjectoffset, t.tmagx_f, t.tmagy_f, t.tmagz_f);
							RotateObject(g.entityworkobjectoffset, ObjectAngleX(t.tobj), ObjectAngleY(t.tobj), ObjectAngleZ(t.tobj));
							if (t.magid == 1)  MoveObjectLeft(g.entityworkobjectoffset, t.tmag1sizex_f);
							if (t.magid == 2)  MoveObjectRight(g.entityworkobjectoffset, t.tmag1sizex_f);
							if (t.magid == 3)  MoveObjectUp(g.entityworkobjectoffset, t.tmag1sizey_f);
							if (t.magid == 4)  MoveObjectDown(g.entityworkobjectoffset, t.tmag1sizey_f);
							if (t.magid == 5)  MoveObject(g.entityworkobjectoffset, t.tmag1sizez_f);
							if (t.magid == 6)  MoveObject(g.entityworkobjectoffset, t.tmag1sizez_f*-1);
							t.tmagx_f = ObjectPositionX(g.entityworkobjectoffset);
							t.tmagy_f = ObjectPositionY(g.entityworkobjectoffset);
							t.tmagz_f = ObjectPositionZ(g.entityworkobjectoffset);
							t.ttobj = t.entityelement[t.tfindclosest].obj;
							for (t.mag2id = 1; t.mag2id <= 6; t.mag2id++)
							{
								t.tmag2x_f = t.entityelement[t.tfindclosest].x;
								t.tmag2y_f = t.entityelement[t.tfindclosest].y;
								t.tmag2z_f = t.entityelement[t.tfindclosest].z;
								PositionObject(g.entityworkobjectoffset, t.tmag2x_f, t.tmag2y_f, t.tmag2z_f);
								RotateObject(g.entityworkobjectoffset, ObjectAngleX(t.ttobj), ObjectAngleY(t.ttobj), ObjectAngleZ(t.ttobj));
								if (t.mag2id == 1)  MoveObjectLeft(g.entityworkobjectoffset, t.tmag2sizex_f);
								if (t.mag2id == 2)  MoveObjectRight(g.entityworkobjectoffset, t.tmag2sizex_f);
								if (t.mag2id == 3)  MoveObjectUp(g.entityworkobjectoffset, t.tmag2sizey_f);
								if (t.mag2id == 4)  MoveObjectDown(g.entityworkobjectoffset, t.tmag2sizey_f);
								if (t.mag2id == 5)  MoveObject(g.entityworkobjectoffset, t.tmag2sizez_f);
								if (t.mag2id == 6)  MoveObject(g.entityworkobjectoffset, t.tmag2sizez_f*-1);
								t.tmag2x_f = ObjectPositionX(g.entityworkobjectoffset);
								t.tmag2y_f = ObjectPositionY(g.entityworkobjectoffset);
								t.tmag2z_f = ObjectPositionZ(g.entityworkobjectoffset);
								//  are magnets close enough together to snap?
								t.tdiffx_f = t.tmag2x_f - t.tmagx_f;
								t.tdiffy_f = t.tmag2y_f - t.tmagy_f;
								t.tdiffz_f = t.tmag2z_f - t.tmagz_f;
								t.tdiff_f = Sqrt(abs(t.tdiffx_f*t.tdiffx_f) + abs(t.tdiffy_f*t.tdiffy_f) + abs(t.tdiffz_f*t.tdiffz_f));
								if (t.tdiff_f < 25.0)
								{
									//  yes, maybe snap to this edge
									if (t.tdiff_f < t.tbestmag_f)
									{
										t.tbestmag_f = t.tdiff_f;
										t.tbestmag2id = t.mag2id;
										t.tbestmag2x_f = t.tmag2x_f + (t.gridentityposx_f - t.tmagx_f);
										t.tbestmag2y_f = t.tmag2y_f + (t.gridentityposy_f - t.tmagy_f);
										t.tbestmag2z_f = t.tmag2z_f + (t.gridentityposz_f - t.tmagz_f);
									}
								}
							}
						}
						if (t.tbestmag2id != -1)
						{
							t.gridentityposx_f = t.tbestmag2x_f;
							t.gridentityposy_f = t.tbestmag2y_f;
							t.gridentityposz_f = t.tbestmag2z_f;
						}
					}
				}
			}
		}
	}
	if (t.gridentitygridlock == 2)
	{
		if (t.entityprofile[t.gridentity].isebe != 0)
		{
			// align EBE structure to match the 100x100 grid
//						t.gridentityposx_f = 0+(int(t.gridentityposx_f/ iEditorGridSizeX)*iEditorGridSizeX);
//						t.gridentityposz_f = 0+(int(t.gridentityposz_f/ iEditorGridSizeZ)*iEditorGridSizeZ);
			t.gridentityposx_f = 0 + (int(t.gridentityposx_f / fEditorGridSizeX)*fEditorGridSizeX);
			t.gridentityposz_f = 0 + (int(t.gridentityposz_f / fEditorGridSizeZ)*fEditorGridSizeZ);
		}
		else
		{
			// align a regular entity to respect its center
//						t.gridentityposx_f = 50+(int(t.gridentityposx_f/ iEditorGridSizeX)*iEditorGridSizeX);
//						t.gridentityposz_f = 50+(int(t.gridentityposz_f/ iEditorGridSizeZ)*iEditorGridSizeZ);
			t.gridentityposx_f = (fEditorGridSizeX*0.5) + (int(t.gridentityposx_f / fEditorGridSizeX)*fEditorGridSizeX);
			t.gridentityposz_f = (fEditorGridSizeZ*0.5) + (int(t.gridentityposz_f / fEditorGridSizeZ)*fEditorGridSizeZ);
		}

		// 130517 - new EBE entity offset to align with 0,0,0 cornered entities from Aslum level and Store (Martin)
		if (t.entityprofile[t.gridentity].isebe != 0)
		{
			if (g.gdefaultebegridoffsetx != 50)
			{
				t.gridentityposx_f -= (g.gdefaultebegridoffsetx - 50);
				t.gridentityposz_f -= (g.gdefaultebegridoffsetz - 50);
			}
		}
	}

}

#ifdef ENABLEIMGUI
void DisplaySmallImGuiMessage(char *text)
{
	ImGui::SetNextWindowPos(OldrenderTargetPos + ImVec2(50, 50), ImGuiCond_Always); //ImGuiCond_Always
	ImGui::SetNextWindowSize(ImVec2(OldrenderTargetSize.x - 100, 0), ImGuiCond_Always); //ImGuiCond_Always
	bool winopen = true;

	ImVec4* style_colors = ImGui::GetStyle().Colors;
	ImVec4 oldBgColor = style_colors[ImGuiCol_WindowBg];
	ImVec4 oldTextColor = style_colors[ImGuiCol_Text];

	float fader = 0.75;
	style_colors[ImGuiCol_WindowBg].x = 0.0;
	style_colors[ImGuiCol_WindowBg].y = 0.0;
	style_colors[ImGuiCol_WindowBg].z = 0.0;
	style_colors[ImGuiCol_WindowBg].w *= (fader*0.25);

	style_colors[ImGuiCol_Text].x = 1.0;
	style_colors[ImGuiCol_Text].y = 1.0;
	style_colors[ImGuiCol_Text].z = 1.0;
	style_colors[ImGuiCol_Text].w *= fader;

	ImGui::Begin("##TriggerSmallMessageinfo", &winopen, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs);
	ImGui::SetWindowFontScale(1.5);
//	ImGui::Text(" ");
	//Center Text.
	float fTextSize = ImGui::CalcTextSize(text).x;
	ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x*0.5) - (fTextSize*0.5), ImGui::GetCursorPos().y));

	ImGui::Text(text);
//	ImGui::Text(" ");
	ImGui::SetWindowFontScale(1.0);
	ImGui::End();
	style_colors[ImGuiCol_WindowBg] = oldBgColor;
	style_colors[ImGuiCol_Text] = oldTextColor;
}

//PE: We can have more then one open at the same time. but only one can play.
#define MAXTUTORIALS 100
bool bSmallVideoReady[MAXTUTORIALS];
bool bSmallVideoResumePossible[MAXTUTORIALS];
bool bSmallVideoInit[MAXTUTORIALS];
int iSmallVideoSlot[MAXTUTORIALS];
int iSmallVideoThumbnail[MAXTUTORIALS];
bool bSmallVideoPerccentStart[MAXTUTORIALS];
int iSmallVideoDelayExecute[MAXTUTORIALS];
int iSmallVideoFindFirstFrame[MAXTUTORIALS];
bool bSmallStartInit[MAXTUTORIALS];
bool bSmallVideoMaximized[MAXTUTORIALS];
cstr cSmallComboSelection[MAXTUTORIALS];
cstr cSmallVideoPath ="";
cstr cSmallVideoDescription = "";
int iCurrentVideoSectionPlaying = 0;

void SmallTutorialThumbLoad(int index)
{
	if (iSmallVideoThumbnail[index] == 0)
	{
		iSmallVideoThumbnail[index] = -1;
		t.tvideofile_s = cSmallVideoPath;
		t.text_s = Lower(Right(t.tvideofile_s.Get(), 4));
		if (t.text_s == ".ogv" || t.text_s == ".mp4")
		{
			cStr Thumb_s = Lower(Left(t.tvideofile_s.Get(), strlen(t.tvideofile_s.Get()) - 4)); Thumb_s += ".jpg";
			if (FileExist(Thumb_s.Get()))
			{
				int iVideoThumbImage = g.videothumbnailsimageoffset + index;
				if (ImageExist(iVideoThumbImage) == 1) DeleteImage(iVideoThumbImage);
				image_setlegacyimageloading(true);
				LoadImage(Thumb_s.Get(), iVideoThumbImage);
				image_setlegacyimageloading(false);
				if (ImageExist(iVideoThumbImage) == 1) iSmallVideoThumbnail[index] = iVideoThumbImage;
			}
		}
	}
}

void SmallTutorialVideoInit(int index)
{
	if (!bSmallVideoInit[index])
	{
		t.tvideofile_s = cSmallVideoPath;
		iSmallVideoSlot[index] = 0;
		iSmallVideoThumbnail[index] = 0;
		t.text_s = Lower(Right(t.tvideofile_s.Get(), 4));
		if (t.text_s == ".ogv" || t.text_s == ".mp4")
		{
			for (int itl = 1; itl <= 32; itl++)
			{
				if (AnimationExist(itl) == 0) { iSmallVideoSlot[index] = itl; break; }
			}
			if (LoadAnimation(t.tvideofile_s.Get(), iSmallVideoSlot[index], g.videoprecacheframes, g.videodelayedload, 1) == false)
			{
				iSmallVideoSlot[index] = -999;
			}
			// also load in the thumnb image for this video
			cStr Thumb_s = Lower(Left(t.tvideofile_s.Get(), strlen(t.tvideofile_s.Get())-4)); Thumb_s += ".jpg";
			if (FileExist(Thumb_s.Get()))
			{
				int iVideoThumbImage = g.videothumbnailsimageoffset + index;
				if (ImageExist(iVideoThumbImage) == 1) DeleteImage(iVideoThumbImage);
				LoadImage(Thumb_s.Get(), iVideoThumbImage);
				if (ImageExist(iVideoThumbImage) == 1) iSmallVideoThumbnail[index] = iVideoThumbImage;
			}
		}
		if (iSmallVideoSlot[index] > 0) 
		{
			PlaceAnimation(iSmallVideoSlot[index], -1, -1, -1, -1);
			bSmallVideoResumePossible[index] = false;
			bSmallVideoPerccentStart[index] = false;
		}
		bSmallVideoInit[index] = true;
	}
}

void SmallTutorialVideo(char *tutorial, char* combo_items[], int combo_entries,int iVideoSection)
{
	int iVideoEntry = -1;

	cSmallVideoPath = "";
	cSmallVideoDescription = "";
	int iCurrentVideoEntry = -1;
	if (tutorial_videos.size() > 0)
	{
		int i = 0;
		for (std::map<std::string, std::string>::iterator it = tutorial_videos.begin(); it != tutorial_videos.end(); ++it)
		{
			if (it->first.length() > 0)
			{
				if (strcmp(it->first.c_str(), tutorial) == 0)
				{
					iVideoEntry = i;
					iCurrentVideoEntry = i;
					cSmallVideoPath = it->second.c_str();
					if (cSmallComboSelection[iVideoEntry].Len() > 0) {
						//PE: Overwrite settings.
						int il = 0;
						for (std::map<std::string, std::string>::iterator it = tutorial_videos.begin(); it != tutorial_videos.end(); ++it) {
							if (it->first.length() > 0 && strcmp(it->first.c_str(), cSmallComboSelection[iVideoEntry].Get()) == 0 ) {
								cSmallVideoPath = it->second.c_str();
								iCurrentVideoEntry = il;
								break;
							}
							il++;
						}
					}
					break;
				}
			}
			i++;
		}
	}
	
	if (iVideoEntry >= 0 && cSmallVideoPath.Len() > 0 ) {

		//PE: Auto launch maximized , if first time in a section.

		if (iCurrentVideoSectionPlaying > 0 && iVideoSection > 0 && iCurrentVideoSectionPlaying != iVideoSection)
		{
			//PE: Pause any playing video , when changing section.
			for (int i = 0; i < MAXTUTORIALS;i++) {
				if (iSmallVideoSlot[i] > 0) {
					if (AnimationExist(iSmallVideoSlot[i])) {
						if (AnimationPlaying(iSmallVideoSlot[i]))
						{
							PauseAnim(iSmallVideoSlot[i]);
							bSmallVideoResumePossible[i] = false;
							iCurrentVideoSectionPlaying = 0;
						}
					}
				}
			}
		}

		if (iVideoSection > 0 && iVideoSection < 20)
		{
			if (0) // This feature is not yet available
			{
				if (pref.iPlayedVideoSection[iVideoSection] == 0)
				{
					pref.iPlayedVideoSection[iVideoSection] = 1;
					bSmallVideoMaximized[iVideoEntry] = true;
					//PE: Perhaps auto start here ?
				}
			}
		}


		//PE: Only delete one video on first run, if same tutorials use same iVideoEntry.
		if (bSmallVideoFrameStart && !bSmallVideoInit[iVideoEntry])
		{
			if (iSmallVideoSlot[iVideoEntry] > 0) {
				if (AnimationExist(iSmallVideoSlot[iVideoEntry])) {
					if (AnimationPlaying(iSmallVideoSlot[iVideoEntry]))
						StopAnimation(iSmallVideoSlot[iVideoEntry]);
					DeleteAnimation(iSmallVideoSlot[iVideoEntry]);
					iSmallVideoSlot[iVideoEntry] = 0;
					bSmallVideoPerccentStart[iVideoEntry] = false;
					bSmallVideoFrameStart = false;
					bSmallVideoResumePossible[iVideoEntry] = false;
				}
			}
		}

		//PE: Dont init first in this system , wait until we need it.
/*
		if (!bSmallVideoInit[iVideoEntry])
		{
			
			if (iSmallVideoSlot[iVideoEntry] > 0) {
				if (AnimationExist(iSmallVideoSlot[iVideoEntry])) {
					if (AnimationPlaying(iSmallVideoSlot[iVideoEntry]))
						StopAnimation(iSmallVideoSlot[iVideoEntry]);
					DeleteAnimation(iSmallVideoSlot[iVideoEntry]);
					iSmallVideoSlot[iVideoEntry] = 0;
				}
			}

			t.tvideofile_s = cSmallVideoPath;
			iSmallVideoSlot[iVideoEntry] = 0;
			t.text_s = Lower(Right(t.tvideofile_s.Get(), 4));
			if (t.text_s == ".ogv" || t.text_s == ".mp4")
			{
				for (int itl = 1; itl <= 32; itl++)
				{
					if (AnimationExist(itl) == 0) { iSmallVideoSlot[iVideoEntry] = itl; break; }
				}
				if (LoadAnimation(t.tvideofile_s.Get(), iSmallVideoSlot[iVideoEntry], g.videoprecacheframes, g.videodelayedload, 1) == false)
				{
					iSmallVideoSlot[iVideoEntry] = -999;
				}
			}
			if (iSmallVideoSlot[iVideoEntry] > 0) {
				PlaceAnimation(iSmallVideoSlot[iVideoEntry], -1, -1, -1, -1);
				SetRenderAnimToImage(iSmallVideoSlot[iVideoEntry], true);
				//Try to get first frame.
				StopAnimation(iSmallVideoSlot[iVideoEntry]);
				PlayAnimation(iSmallVideoSlot[iVideoEntry]);
				SetRenderAnimToImage(iSmallVideoSlot[iVideoEntry], true);
				iSmallVideoFindFirstFrame[iVideoEntry] = 4;
				SetVideoVolume(0);
				UpdateAllAnimation();
				bSmallVideoResumePossible[iVideoEntry] = false;
				bSmallVideoPerccentStart[iVideoEntry] = false;
			}
			bSmallVideoInit[iVideoEntry] = true;
		}
*/
		if (1) //iSmallVideoSlot[iVideoEntry] > 0)
		{

			if (iSmallVideoFindFirstFrame[iVideoEntry] > 0) {
				if (iSmallVideoFindFirstFrame[iVideoEntry] == 1) {
					PauseAnim(iSmallVideoSlot[iVideoEntry]);
					iCurrentVideoSectionPlaying = 0;
					bSmallVideoResumePossible[iVideoEntry] = false;
					SetVideoVolume(100.0);
				}
				iSmallVideoFindFirstFrame[iVideoEntry]--;
			}

			switch (iSmallVideoDelayExecute[iVideoEntry]) {

				case 1: //Play restart
				{
					//PE: We can only start one video per frame.
					if (bSmallVideoFrameStart) {
						bSmallVideoFrameStart = false;
						iSmallVideoDelayExecute[iVideoEntry] = 0;
						SmallTutorialVideoInit(iVideoEntry);
						if (iSmallVideoSlot[iVideoEntry] > 0) {
							StopAnimation(iSmallVideoSlot[iVideoEntry]);
							PlayAnimation(iSmallVideoSlot[iVideoEntry]);
							SetRenderAnimToImage(iSmallVideoSlot[iVideoEntry], true);
							UpdateAllAnimation();
							Sleep(50); //Sleep so we get a video texture in the next call.
							UpdateAllAnimation();
							SetVideoVolume(100.0);
							bSmallVideoResumePossible[iVideoEntry] = false;
							bSmallVideoPerccentStart[iVideoEntry] = true;
							iCurrentVideoSectionPlaying = iVideoSection;
						}
					}
					break;
				}
				case 2: //Resume
				{
					SmallTutorialVideoInit(iVideoEntry);
					if (iSmallVideoSlot[iVideoEntry] > 0) {
						iSmallVideoDelayExecute[iVideoEntry] = 0;
						ResumeAnim(iSmallVideoSlot[iVideoEntry]);
						iCurrentVideoSectionPlaying = iVideoSection;
					}
					break;
				}
				case 3: //Pause
				{
					SmallTutorialVideoInit(iVideoEntry);
					if (iSmallVideoSlot[iVideoEntry] > 0) {
						iSmallVideoDelayExecute[iVideoEntry] = 0;
						PauseAnim(iSmallVideoSlot[iVideoEntry]);
						bSmallVideoResumePossible[iVideoEntry] = true;
						iCurrentVideoSectionPlaying = 0;
					}
					break;
				}
				default:
					break;
			}

			int combo_current_type_selection = 0;

			if (combo_items && combo_entries > 0 ) {
				if (cSmallComboSelection[iVideoEntry].Len() > 0)
				{
					for (int i = 0; i < combo_entries;i++)
					{
						if (combo_items[i])
						{
							if (strcmp(cSmallComboSelection[iVideoEntry].Get(), combo_items[i]) == 0)
							{
								combo_current_type_selection = i;
								break;
							}
						}
					}
				}
			}
			if (combo_items && combo_entries > 0)
			{
				std::map<std::string, std::string>::iterator it = tutorial_description.find(combo_items[combo_current_type_selection]);
				if (it != tutorial_description.end()) {
					cSmallVideoDescription = it->second.c_str();
				}
			}
			else
			{
				std::map<std::string, std::string>::iterator it = tutorial_description.find(tutorial);
				if (it != tutorial_description.end()) {
					cSmallVideoDescription = it->second.c_str();
				}
			}
			//bSmallVideoMaximized[iVideoEntry] = true; //For now.
			bool bMustEndWindow = false;
			if (bSmallVideoMaximized[iVideoEntry]) {
				
				//	Display the maximised tutorial video window.
				ImGui::SetNextWindowSize(ImVec2(55 * ImGui::GetFontSize(), 44 * ImGui::GetFontSize()), ImGuiCond_Once);
				ImGui::SetNextWindowPosCenter(ImGuiCond_Once);

				ImGui::Begin("Tutorial Video##VideosMaxSize", &bSmallVideoMaximized[iVideoEntry], 0);
				bMustEndWindow = true;
				ImGui::Indent(10);
			}


			if (combo_items && combo_entries > 0 ) {
				//Display combo.
				ImGui::PushItemWidth(-10);
				cstr sUniqueLabel = cstr("##TutorialSimpleInput") + cstr(iVideoEntry);
				if (ImGui::Combo(sUniqueLabel.Get(), &combo_current_type_selection, combo_items, combo_entries)) {
					if (combo_items[combo_current_type_selection])
					{
						cSmallComboSelection[iVideoEntry] = combo_items[combo_current_type_selection];
						bSmallVideoInit[iVideoEntry] = false;
						bSmallVideoFrameStart = false; //PE: Wait until next frame.
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select Tutorial Video");
				ImGui::PopItemWidth();
			}

			float fRatio = 1.0f / ((float)GetDesktopWidth() / (float)GetDesktopHeight());

			ID3D11ShaderResourceView* lpVideoTexture = NULL;
			if (bSmallVideoInit[iVideoEntry] )
				lpVideoTexture = GetAnimPointerView(iSmallVideoSlot[iVideoEntry]);
			float fVideoW = GetAnimWidth(iSmallVideoSlot[iVideoEntry]);
			float fVideoH = GetAnimHeight(iSmallVideoSlot[iVideoEntry]);
			if (bSmallVideoInit[iVideoEntry] && iSmallVideoSlot[iVideoEntry] > 0 && lpVideoTexture) {
				fRatio = 1.0f / (fVideoW / fVideoH);
			}

			int iActiveID = iVideoEntry;
			if (iCurrentVideoEntry >= 0 && iCurrentVideoEntry < MAXTUTORIALS && iCurrentVideoEntry != iVideoEntry) iActiveID = iCurrentVideoEntry;

			float videoboxheight = (ImGui::GetContentRegionAvail().x - 10.0) * fRatio;

			ImVec4 oldImGuiCol_ChildWindowBg = ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg];
			ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
			ImGui::BeginChild("Video##TutorialVideo", ImVec2(ImGui::GetContentRegionAvail().x - 10.0, videoboxheight), true, iGenralWindowsFlags);
			bool bToogleMinMax = false;
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			ImRect image_bb(window->DC.CursorPos, window->DC.CursorPos + ImGui::GetContentRegionAvail());
			image_bb.Floor();

			bool bIsPlaying = false;
			//PE: We always play into iVideoEntry not iActiveID.
			if (iSmallVideoSlot[iVideoEntry] > 0)
			{
				if (AnimationExist(iSmallVideoSlot[iVideoEntry]) && AnimationPlaying(iSmallVideoSlot[iVideoEntry]))
					bIsPlaying = true;
			}

			bool bVideoAreaPressed = false;

			if (lpVideoTexture) 
			{
				SetRenderAnimToImage(iSmallVideoSlot[iVideoEntry], true);
				float animU = GetAnimU(iSmallVideoSlot[iVideoEntry]);
				float animV = GetAnimV(iSmallVideoSlot[iVideoEntry]);
				ImVec2 uv0 = ImVec2(0, 0);
				ImVec2 uv1 = ImVec2(animU, animV);
				//window->DrawList->AddImage((ImTextureID)lpVideoTexture, image_bb.Min, image_bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

				ImGui::PushID(lpVideoTexture);
				const ImGuiID id = window->GetID("#image");
				ImGui::PopID();
				ImGui::ItemSize(image_bb);
				if (ImGui::ItemAdd(image_bb, id))
				{
					window->DrawList->AddImage((ImTextureID)lpVideoTexture, image_bb.Min, image_bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

					bool hovered, held;
					bVideoAreaPressed = ImGui::ButtonBehavior(image_bb, id, &hovered, &held);
				}

			}
			else 
			{
				// Display thumbnail of the video (from .jpg)

				SmallTutorialThumbLoad(iActiveID);
				ID3D11ShaderResourceView* lpTexture = NULL;
				if (iSmallVideoThumbnail[iActiveID] > 0) lpTexture = GetImagePointerView(iSmallVideoThumbnail[iActiveID]);

				//if (lpTexture) window->DrawList->AddImage((ImTextureID)lpTexture, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

				ImGui::PushID(lpTexture);
				const ImGuiID id = window->GetID("#image");
				ImGui::PopID();
				ImGui::ItemSize(image_bb);
				if (ImGui::ItemAdd(image_bb, id))
				{
					if (lpTexture) window->DrawList->AddImage((ImTextureID)lpTexture, image_bb.Min, image_bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
					bool hovered, held;
					bVideoAreaPressed = ImGui::ButtonBehavior(image_bb, id, &hovered, &held);
				}

				//Display a play button.
				ImVec2 vOldPos = ImGui::GetCursorPos();
				float fPlayButSize = ImGui::GetContentRegionAvail().x * 0.15;
				float fCenterX = (ImGui::GetContentRegionAvail().x*0.5) - (fPlayButSize*0.5);
				float fCenterY = (videoboxheight*0.5) - (fPlayButSize*0.5);
				ImGui::SetCursorPos(ImVec2(fCenterX, fCenterY));
				ImVec4 vColorFade = { 1.0,1.0,1.0,0.5 };
				if (ImGui::ImgBtn(MEDIA_PLAY, ImVec2(fPlayButSize, fPlayButSize), ImColor(255, 255, 255, 0), drawCol_normal*vColorFade, drawCol_hover*vColorFade, drawCol_Down*vColorFade, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
				{
					bSmallVideoPerccentStart[iVideoEntry] = true;
					bSmallVideoResumePossible[iVideoEntry] = false;
					iSmallVideoDelayExecute[iVideoEntry] = 1; //force play - restart.
					bSmallVideoFrameStart = false; //PE: Wait until next frame.
					//bVideoAreaPressed = true;
				}
				if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Play");

				ImGui::SetCursorPos(vOldPos);
			}
			//PE: Double click trigger an inver, so also invert here.
			if (bVideoAreaPressed || (ImGui::IsMouseDoubleClicked(0) && ImGui::IsMouseHoveringRect(image_bb.Min, image_bb.Max)) )
			{
				//Video pause/play
				if (bIsPlaying)
				{
					//Pause
					bSmallVideoPerccentStart[iVideoEntry] = true;
					iSmallVideoDelayExecute[iVideoEntry] = 3; // pause
					bSmallVideoFrameStart = false; //PE: Wait until next frame.
				}
				else
				{
					//Play
					bSmallVideoPerccentStart[iVideoEntry] = true;
					if (bSmallVideoResumePossible[iVideoEntry]) {
						iSmallVideoDelayExecute[iVideoEntry] = 2; //resume
					}
					else {
						iSmallVideoDelayExecute[iVideoEntry] = 1; //play - restart.
					}
					bSmallVideoFrameStart = false; //PE: Wait until next frame.
				}
			}

			if (ImGui::IsMouseHoveringRect(image_bb.Min, image_bb.Max))
			{
				if (ImGui::IsMouseDoubleClicked(0) && !bVideoAreaPressed )
				{
					bSmallVideoMaximized[iVideoEntry] = 1 - bSmallVideoMaximized[iVideoEntry];
				}
				/*
				else
				{
					if (ImGui::IsMouseReleased(0))
					{
						if (bIsPlaying)
						{
							//Pause
							bSmallVideoPerccentStart[iVideoEntry] = true;
							iSmallVideoDelayExecute[iVideoEntry] = 3; // pause
							bSmallVideoFrameStart = false; //PE: Wait until next frame.
						}
						else
						{
							//Play
							bSmallVideoPerccentStart[iVideoEntry] = true;
							if (bSmallVideoResumePossible[iVideoEntry]) {
								iSmallVideoDelayExecute[iVideoEntry] = 2; //resume
							}
							else {
								iSmallVideoDelayExecute[iVideoEntry] = 1; //play - restart.
							}
							bSmallVideoFrameStart = false; //PE: Wait until next frame.
						}
					}
				}
				*/
			}

			ImGui::EndChild();
			ImGui::GetStyle().Colors[ImGuiCol_ChildWindowBg] = oldImGuiCol_ChildWindowBg;

			{
				float fdone = GetAnimPercentDone(iSmallVideoSlot[iVideoEntry]) / 100.0f;
				if (!bSmallVideoPerccentStart[iVideoEntry]) fdone = 0.0f;

				ImGui::ProgressBar(fdone, ImVec2(ImGui::GetContentRegionAvail().x - 10, 6), "");

				#define MEDIAICONSIZE 20

				if (bIsPlaying)
				{
					if (ImGui::ImgBtn(MEDIA_PAUSE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false, false, false, false, false, bBoostIconColors))
					{
						bSmallVideoPerccentStart[iVideoEntry] = true;
						iSmallVideoDelayExecute[iVideoEntry] = 3; // pause
						bSmallVideoFrameStart = false; //PE: Wait until next frame.
					}
					if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Pause");
				}
				else
				{
					if (ImGui::ImgBtn(MEDIA_PLAY, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false, false, false, false, false, bBoostIconColors))
					{
						bSmallVideoPerccentStart[iVideoEntry] = true;
						if (bSmallVideoResumePossible[iVideoEntry]) {
							iSmallVideoDelayExecute[iVideoEntry] = 2; //resume
						}
						else {
							iSmallVideoDelayExecute[iVideoEntry] = 1; //play - restart.
						}
						bSmallVideoFrameStart = false; //PE: Wait until next frame.
					}
					if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Play");
				}
				ImGui::SameLine();
				if (ImGui::ImgBtn(MEDIA_REFRESH, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, false,false,false,false,false, bBoostIconColors))
				{
					bSmallVideoPerccentStart[iVideoEntry] = true;
					iSmallVideoDelayExecute[iVideoEntry] = 1; //play - restart.
					bSmallVideoFrameStart = false; //PE: Wait until next frame.
				}
				if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Restart");

				if (!bSmallVideoMaximized[iVideoEntry])
				{
					ImGui::SameLine();
					if (ImGui::ImgBtn(MEDIA_MAXIMIZE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true,false,false,false,false, bBoostIconColors))
					{
						bSmallVideoMaximized[iVideoEntry] = true;
					}
					if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Maximize");
				}
				else
				{
					ImGui::SameLine();
					if (ImGui::ImgBtn(MEDIA_MINIMIZE, ImVec2(MEDIAICONSIZE, MEDIAICONSIZE), ImColor(255, 255, 255, 0), drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true,false,false,false,false, bBoostIconColors))
					{
						bSmallVideoMaximized[iVideoEntry] = false;
					}
					if (ImGui::windowTabVisible() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Minimize");
				}
			}

			if (bMustEndWindow) {

				if (cSmallVideoDescription.Len() > 0) {
					ImGui::Separator();
					ImGui::Text("Description");
					ImGui::TextWrapped(cSmallVideoDescription.Get());
				}

				bImGuiGotFocus = true;
				ImGui::Indent(-10);

				if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
					//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
					ImGui::Text("");
					ImGui::Text("");
				}

				ImGui::End();
			}

		}
	}
	return;
}

// Useful function
UINT OpenURLForGETPOST(LPSTR pServerName, LPSTR* pDataReturned, DWORD* pReturnDataSize, LPSTR pAuthHeader, LPSTR pszPostData, LPSTR pVerb, LPSTR urlWhere)
{
	// create large area to drop reply into
	int i100MB = 102400000;
	*pDataReturned = new char[i100MB];
	memset(*pDataReturned, 0, i100MB);

	UINT iError = 0;
	unsigned int dwDataLength = 0;
	HINTERNET m_hInet = InternetOpenA("InternetConnection", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInet == NULL)
	{
		iError = GetLastError();
	}
	else
	{
		unsigned short wHTTPType = INTERNET_DEFAULT_HTTPS_PORT;
		HINTERNET m_hInetConnect = InternetConnectA(m_hInet, pServerName, wHTTPType, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
		if (m_hInetConnect == NULL)
		{
			iError = GetLastError();
		}
		else
		{
			int m_iTimeout = 2000;
			InternetSetOption(m_hInetConnect, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)&m_iTimeout, sizeof(m_iTimeout));
			HINTERNET hHttpRequest = HttpOpenRequestA(m_hInetConnect, pVerb, urlWhere, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
			if (hHttpRequest == NULL)
			{
				iError = GetLastError();
			}
			else
			{
				HttpAddRequestHeadersA(hHttpRequest, "Content-Type: application/x-www-form-urlencoded", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
				if (pAuthHeader) HttpAddRequestHeadersA(hHttpRequest, pAuthHeader, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
				int bSendResult = 0;
				if (pszPostData)
					bSendResult = HttpSendRequest(hHttpRequest, NULL, -1, (void*)(pszPostData), strlen(pszPostData));
				else
					bSendResult = HttpSendRequest(hHttpRequest, NULL, -1, NULL, 0);
				if (bSendResult == 0)
				{
					iError = GetLastError();
				}
				else
				{
					int m_iStatusCode = 0;
					char m_szContentType[150];
					unsigned int dwBufferSize = sizeof(int);
					unsigned int dwHeaderIndex = 0;
					HttpQueryInfo(hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (void*)&m_iStatusCode, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex);
					dwHeaderIndex = 0;
					unsigned int dwContentLength = 0;
					HttpQueryInfo(hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (void*)&dwContentLength, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex);
					dwHeaderIndex = 0;
					unsigned int ContentTypeLength = 150;
					HttpQueryInfo(hHttpRequest, HTTP_QUERY_CONTENT_TYPE, (void*)m_szContentType, (LPDWORD)&ContentTypeLength, (LPDWORD)&dwHeaderIndex);
					char pBuffer[20000];
					for (;;)
					{
						unsigned int written = 0;
						if (!InternetReadFile(hHttpRequest, (void*)pBuffer, 2000, (LPDWORD)&written))
						{
							// error
						}
						if (written == 0) break;
						if (dwDataLength + written > 102400000) written = 102400000 - dwDataLength;
						memcpy(*pDataReturned + dwDataLength, pBuffer, written);
						dwDataLength = dwDataLength + written;
						if (dwDataLength >= 102400000) break;
					}
					InternetCloseHandle(hHttpRequest);
				}
			}
			InternetCloseHandle(m_hInetConnect);
		}
		InternetCloseHandle(m_hInet);
	}
	if (iError > 0)
	{
		char *szError = 0;
		if (iError > 12000 && iError < 12174)
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA("wininet.dll"), iError, 0, (char*)&szError, 0, 0);
		else
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, iError, 0, (char*)&szError, 0, 0);
		if (szError)
		{
			LocalFree(szError);
		}
	}

	// complete
	*pReturnDataSize = dwDataLength;
	return iError;
}

//#####################################
//#### Process Preferences window. ####
//#####################################


void CloseAllOpenTools(bool bTerrainTools)
{
	if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
	if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
	//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
	if (bEntity_Properties_Window) bEntity_Properties_Window = false;
	if (t.ebe.on == 1) ebe_hide();
	if (bTerrainTools)
	{
		if (bTerrain_Tools_Window) bTerrain_Tools_Window = false;
		//if (Visuals_Tools_Window) Visuals_Tools_Window = false; //PE: Now toggle.
	}

}

void CloseAllOpenToolsThatNeedSave(void)
{
	if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
	//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb
	if (t.ebe.on == 1) ebe_hide();
}

#endif

//PE: This also fixed TW TASK "BETA 3 - When placing an Audio marker, an outline of another appeared off - screen.Not sure what I did and I can't seem to replicate it. I can't select the outline.".

void DeleteWaypointsAddedToCurrentCursor(void)
{
	//PE: Delete any waypoints added to current cursor.
	if (t.grideditselect == 5 && t.gridentity > 0 && t.grideleprof.trigger.waypointzoneindex > 0)
	{
		t.waypointindex = t.grideleprof.trigger.waypointzoneindex;
		if (t.waypointindex > 0)
		{
			t.w = t.waypoint[t.waypointindex].start;
			waypoint_delete();
		}
		t.grideleprof.trigger.waypointzoneindex = 0;
	}
}

// New Logic System - Visual Relational Lines

///

float ImGuiGetMouseX( void )
{
	return(t.inputsys.xmouse);
}

float ImGuiGetMouseY(void)
{
	return(t.inputsys.ymouse);
}

#ifdef ENABLEIMGUI
void process_entity_library(void)
{
	if (bExternal_Entities_Window)
	{
		ImGuiWindowFlags ex_window_flags = 0;
		ImGuiIO& io = ImGui::GetIO();

		static int uniqueId = 4000; //PE: Also used for imageID for previews.
		static int loaded_images = 0;
		static bool multi_selections = false;
		int multi_selections_count = 0;
		static int lf_multi_selections_count = 0;
		int olduniqueId = uniqueId;
		bool bReleaseIconsDynamic = false;
		uniqueId = 4000;
#ifdef DYNAMICLOADUNLOAD
		static int max_load_persync = 200; //First time only , changed later to 15
		bReleaseIconsDynamic = true;
#else
		int max_load_persync = 2000;
#endif

		int preview_count = 0;
		int media_icon_size = 64;
		int iColumnsWidth = 110;

		time_t tCurrentTimeSec;
		time(&tCurrentTimeSec);

		ImGui::Begin("Entity Library##ExternalWindow", &bExternal_Entities_Window, ex_window_flags);

		bool bAddSelectionToGame = false;
		bool bIsWeDocked = ImGui::IsWindowDocked();
		static int current_tab = -1;


		CheckTutorialAction("TABMARKERS", 54.0f); //Tutorial: check if we are waiting for this action
		if (current_tab == 1 && bTutorialCheckAction)
			TutorialNextAction(); //Clicked - selected the tab markers.
		CheckTutorialAction("TABENTITIES", -10.0f); //Tutorial: check if we are waiting for this action
		if (current_tab == 0 && bTutorialCheckAction)
			TutorialNextAction(); //Clicked - selected the tab Entities.

		ImGui::SetItemAllowOverlap();
		if (ImGui::BeginTabBar("entlibtabbar"))
		{
			static int iCurrentFilter = 0;
			static char cSearchAllEntities[3][MAX_PATH] = { "\0","\0","\0" };

			for (int i = 0; i < 2; i++) {

				cStr sTabHeader;
				if (i == 0) sTabHeader = " Entities ";
				if (i == 1) sTabHeader = " Markers! ";


				if (ImGui::BeginTabItem(sTabHeader.Get()))
				{
					if (current_tab != i) {
						//Tab changed.
						//timestampactivity(0, "Entity Library Tab change");
						current_tab = i;
						iCurrentFilter = 0;
					}

					static char cAllFilters[10][MAX_PATH];
					char cFilter[MAX_PATH], cHeader[MAX_PATH];
					int splitsections = 1;

					if (i == 1) {
						splitsections = 5;
					}

					int control_wrap_width = 70;
					strcpy(cFilter, "");
					strcpy(cHeader, "");
					//PE: Debug dynamic icon load unload.
					//ImGui::Text("Entities: %ld , in memory: %ld", olduniqueId-4000, loaded_images);
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3.0));
					ImGui::Text("Filter: ");
					ImGui::SameLine();
					ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3.0));
					if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
						ImGui::Text(""); //NewLine
					bool rb_change = false;
					ImGui::RadioButton("All ", &iCurrentFilter, 0);
					ImGui::SameLine();
					if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
						ImGui::Text(""); //NewLine

					if (i == 0) {

						CheckTutorialAction("BUTCHARACTER", -28.0f); //Tutorial: check if we are waiting for this action
						if (ImGui::RadioButton("Characters ", &iCurrentFilter, 1)) {
							//On click remove search.
							strcpy(cSearchAllEntities[i], "");
							if (bTutorialCheckAction)
								TutorialNextAction(); //Clicked - on "Characters"
						}
						ImGui::SameLine();
						if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
							ImGui::Text(""); //NewLine

						CheckTutorialAction("BUTBUILDINGS", -28.0f); //Tutorial: check if we are waiting for this action
						if (ImGui::RadioButton("Buildings", &iCurrentFilter, 2)) {
							//On click remove search.
							strcpy(cSearchAllEntities[i], "");
							if (bTutorialCheckAction)
								TutorialNextAction(); //Clicked - on "Buildings"
						}
						ImGui::SameLine();

						if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
							ImGui::Text(""); //NewLine


						CheckTutorialAction("BUTOBJECTS", -28.0f); //Tutorial: check if we are waiting for this action
						if (ImGui::RadioButton("Objects", &iCurrentFilter, 6)) {
							//On click remove search.
							strcpy(cSearchAllEntities[i], "");
							if (bTutorialCheckAction)
								TutorialNextAction(); //Clicked - on "objects"
						}

						if (iCurrentFilter == 1) strcpy(cFilter, "Character");
						if (iCurrentFilter == 2) strcpy(cFilter, "Building");
						//								if (iCurrentFilter == 3) strcpy(cFilter, "Foliage");
						//								if (iCurrentFilter == 4) strcpy(cFilter, "Cartoon");
						//								if (iCurrentFilter == 5) strcpy(cFilter, "Fixtures");
						if (iCurrentFilter == 6) strcpy(cFilter, "*");

						strcpy(cAllFilters[0], "Character");
						strcpy(cAllFilters[1], "Building");
						//								strcpy(cAllFilters[2], "Foliage");
						//								strcpy(cAllFilters[3], "Cartoon");
						//								strcpy(cAllFilters[4], "Fixtures");

					}
					if (i == 1) {
						ImGui::RadioButton("Players ", &iCurrentFilter, 1);
						ImGui::SameLine();
						if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
							ImGui::Text(""); //NewLine
						ImGui::RadioButton("Zones", &iCurrentFilter, 2);
						ImGui::SameLine();
						if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
							ImGui::Text(""); //NewLine
						ImGui::RadioButton("Lights", &iCurrentFilter, 3);
						ImGui::SameLine();
						if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
							ImGui::Text(""); //NewLine
						ImGui::RadioButton("Spot Lights", &iCurrentFilter, 4);


						if (iCurrentFilter == 1) { strcpy(cFilter, "player"); strcpy(cHeader, "Player Positions"); }
						if (iCurrentFilter == 2) { strcpy(cFilter, "zone.fpe"); strcpy(cHeader, "Zones"); }
						if (iCurrentFilter == 3) { strcpy(cFilter, " light.fpe"); strcpy(cHeader, "Lights"); }
						if (iCurrentFilter == 4) { strcpy(cFilter, "spot.fpe"); strcpy(cHeader, "Spot Lights"); }
						if (iCurrentFilter > 0) splitsections = 1;
					}

					ImGui::SameLine();

					if (ImGui::GetCursorPosX() + control_wrap_width > ImGui::GetWindowSize().x)
						ImGui::Text(""); //NewLine

					float fXWidth = ImGui::GetFontSize()*1.5;
					if (i == 0)
						ImGui::PushItemWidth(-38 - fXWidth);
					else
						ImGui::PushItemWidth(-1 - fXWidth - 4.0);

					ImGui::Text(" Search: ");
					ImGui::SameLine();

					bEntityGotFocus = false;
					if (ImGui::InputText("##cSearchAllEntities", &cSearchAllEntities[i][0], MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						if (strlen(cSearchAllEntities[i]) > 1) {
							bool already_there = false;
							for (int l = 0; l < MAXSEARCHHISTORY; l++) {
								if (strcmp(cSearchAllEntities[i], pref.search_history[l]) == 0) {
									already_there = true;
									break;
								}
							}
							if (!already_there) {
								bool foundspot = false;
								for (int l = 0; l < MAXSEARCHHISTORY; l++) {
									if (strlen(pref.search_history[l]) <= 0) {
										strcpy(pref.search_history[l], cSearchAllEntities[i]);
										foundspot = true;
										break;
									}
								}
								if (!foundspot) {
									//Move entry list.
									for (int l = 0; l < MAXSEARCHHISTORY; l++) {
										strcpy(pref.search_history[l], pref.search_history[l + 1]);
									}
									strcpy(pref.search_history[MAXSEARCHHISTORY - 1], cSearchAllEntities[i]);
								}
							}
						}
					}
					ImGui::SameLine();
					if (ImGui::StyleButton("X##deletesearch"))
					{
						strcpy(cSearchAllEntities[i], "");
					}
					//}
					ImGui::PopItemWidth();


					//Combo dropdown. Use folder names as seach.
					if (i == 0) {
						ImGui::SameLine();
						static char * current_combo_entry = "\0";
						int comboflags = ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLarge;
						ImGui::PushItemWidth(-24);
						if (ImGui::BeginCombo("##combolastsearch", current_combo_entry, comboflags))
						{

							//Do we have a search history.
							bool display_history = false;
							for (int l = 0; l < MAXSEARCHHISTORY; l++) {
								if (strlen(pref.search_history[l]) > 0) {
									display_history = true;
									break;
								}
							}
							if (display_history) {
								ImGui::Text("Search History:");
								ImGui::Indent(10);
								for (int l = 0; l < MAXSEARCHHISTORY; l++) {
									if (strlen(pref.search_history[l]) > 0) {
										bool is_selected = (current_combo_entry == pref.search_history[l]);
										if (ImGui::Selectable(pref.search_history[l], is_selected)) {
											current_combo_entry = (char *)pref.search_history[l];
											strcpy(cSearchAllEntities[i], pref.search_history[l]);
										}
										if (is_selected)
											ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::Indent(-10);
							}
							cFolderItem *pNewFolder = &MainEntityList;
							pNewFolder = pNewFolder->m_pNext;
							if (pNewFolder) {

								ImGui::Text("Folders:");
								ImGui::Indent(10);

								cStr path_remove = pNewFolder->m_sFolderFullPath.Get();
								int ipath_remove_len = path_remove.Len();

								ImVec4* style_colors = ImGui::GetStyle().Colors;
								ImVec2 wsize = ImGui::GetWindowSize();

								int line_count = 0;
								while (pNewFolder)
								{
									cStr path = pNewFolder->m_sFolderFullPath.Get();
									bool bDoubleEntityBank = false;
									char *finde = (char *)pestrcasestr(path.Get(), "\\entitybank"); //Support entitybank inside entitybank.
									if (finde)
									{
										finde += 11;
										finde = (char *)pestrcasestr(finde, "\\entitybank");
										if (finde) bDoubleEntityBank = true;
									}
									if (!bDoubleEntityBank && path.Right(11) == "\\entitybank")
									{
										ipath_remove_len = path.Len();
									}
									else
									{
										char *final_name = path.Get();
										final_name += ipath_remove_len;
										if (*final_name == '\\')
											final_name++;

										std::string dir_name = final_name;
										replaceAll(dir_name, "\\", " - ");

										if (dir_name.length() > 0 && pNewFolder->m_pFirstFile && !pestrcasestr(dir_name.c_str(), "_markers"))
										{
											bool is_selected = (current_combo_entry == pNewFolder->m_sFolderFullPath.Get());
											if (ImGui::Selectable(dir_name.c_str(), is_selected))
											{
												current_combo_entry = (char *)pNewFolder->m_sFolderFullPath.Get();
												//Make a search entry.
												strcpy(cSearchAllEntities[i], dir_name.c_str());

											}
											if (is_selected)
												ImGui::SetItemDefaultFocus();
											line_count++;
										}
									}
									pNewFolder = pNewFolder->m_pNext;
								}
								ImGui::Indent(-10);
							}
							ImGui::EndCombo();
						}

						ImGui::PopItemWidth();
					}
					ImGui::Separator();

					if (strlen(cSearchAllEntities[i]) > 0 && i == 1) {
						iCurrentFilter = 0;
						splitsections = 1;
					}

					ImGui::BeginChild("##cSearchAllEntitiesBegin", ImVec2(0, 0),false, iGenralWindowsFlags);

					ImVec2 oldCursor = ImGui::GetCursorPos();
					std::string insert_text;
					ImVec2 insert_text_width;

					if (lf_multi_selections_count > 0) {
						//Display insert button.
						insert_text = " Add ";
						insert_text += std::to_string(lf_multi_selections_count);
						insert_text += " Objects to Level ";
						insert_text_width = ImGui::CalcTextSize(insert_text.c_str());
						ImGui::SetCursorPos(ImVec2(0, (ImGui::GetWindowSize().y + ImGui::GetScrollY()) - insert_text_width.y - 10.0f));

						ImGui::Spacing();
						ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (insert_text_width.x + 18.0f));

						ImGui::SetItemAllowOverlap();
						if (ImGui::StyleButton(insert_text.c_str())) {
							//Inset all selected items.
							bAddSelectionToGame = true;
						}
					}

					ImGui::SetCursorPos(oldCursor);
					if (i == 0)
						ImGui::TextCenter("Click to add an object to your level. Control+Click to select multiple items.");

					int iIconVisiblePosY = ImGui::GetWindowSize().y + ImGui::GetScrollY() + media_icon_size;

					bool bFirstShiftHasBeenSeen = false;
					bool bAnySelectedItemsAvailable = false;
					static cFolderItem::sFolderFiles * firstShiftFile = NULL;
					static cFolderItem::sFolderFiles * lastShiftFile = NULL;

					for (int splitloop = 0; splitloop < splitsections; splitloop++) {

						if (iCurrentFilter == 0) {
							if (i == 1) strcpy(cFilter, "");
							if (i == 1) strcpy(cHeader, "");
							if (i == 1 && splitloop == 0) { strcpy(cFilter, "player"); strcpy(cHeader, "Player Positions"); }
							if (i == 1 && splitloop == 1) { strcpy(cFilter, "zone.fpe"); strcpy(cHeader, "Zones"); }
							if (i == 1 && splitloop == 2) { strcpy(cFilter, " light.fpe"); strcpy(cHeader, "Lights"); }
							if (i == 1 && splitloop == 3) { strcpy(cFilter, "spot.fpe"); strcpy(cHeader, "Spot Lights"); }
							if (i == 1 && splitloop == 4) { strcpy(cFilter, "*"); strcpy(cHeader, "Others"); }
							if (i == 1) strcpy(cAllFilters[splitloop], cFilter);
							if (strlen(cSearchAllEntities[i]) > 0 && i == 1) {
								strcpy(cHeader, "");
							}
						}
						cFolderItem *pNewFolder = &MainEntityList;
						pNewFolder = pNewFolder->m_pNext;
						if (pNewFolder)
						{
							//We start at the "entitybank" entry that we use to parce the others.
							cStr path_remove = pNewFolder->m_sFolderFullPath.Get();
							int ipath_remove_len = path_remove.Len();
							pNewFolder = pNewFolder->m_pNext;
							while (pNewFolder)
							{
								//PE: Full path can now change in the middle of the list , so:
								cStr path = pNewFolder->m_sFolderFullPath.Get();
								LPSTR pPathSearch = path.Get();
								LPSTR pFind = "\\entitybank";
								for (int n = 0; n < strlen(pPathSearch); n++)
								{
									if (strnicmp(pPathSearch + n, pFind, strlen(pFind)) == NULL)
									{
										ipath_remove_len = n + strlen(pFind);
										break;
									}
								}

								if (1)
								{
									bool isMarkers = false;
									bool bDisplayEverythingHere = false;
									bool bHideEverythingHere = false;
									if (i == 0 && cFilter[0] == '*')
									{
										if (pestrcasestr(pNewFolder->m_sFolderFullPath.Get(), cAllFilters[0]))
											bHideEverythingHere = true;
										if (pestrcasestr(pNewFolder->m_sFolderFullPath.Get(), cAllFilters[1]))
											bHideEverythingHere = true;
									}
									else if (strlen(cFilter) > 0)
									{
										if (pestrcasestr(pNewFolder->m_sFolderFullPath.Get(), cFilter))
											bDisplayEverythingHere = true;
									}
									if (strlen(cSearchAllEntities[i]) > 0)
									{
										//When search disable fixed tags search.
										bDisplayEverythingHere = false;
										bHideEverythingHere = false;
									}
									char *final_name = path.Get();
									final_name += ipath_remove_len;
									if (*final_name == '\\')
										final_name++;

									std::string path_for_filename = final_name;
									std::string dir_name = final_name;
									replaceAll(dir_name, "\\", " - ");

									if (pestrcasestr(dir_name.c_str(), "_markers"))
										isMarkers = true;

									if (!isMarkers && i == 0 && !bDisplayEverythingHere && !bHideEverythingHere && strlen(cSearchAllEntities[i]) > 0) {
										if (pestrcasestr(pNewFolder->m_sFolderFullPath.Get(), cSearchAllEntities[i]))
											bDisplayEverythingHere = true;
										else if (pestrcasestr(dir_name.c_str(), cSearchAllEntities[i]))
											bDisplayEverythingHere = true;

									}

									//PE: Check here if we need to reload the folder, for new files.
									if (pNewFolder->m_fLastTimeUpdate < Timer())
									{
										pNewFolder->m_fLastTimeUpdate = Timer() + 4000; //Check every 4-6 sec.
										pNewFolder->m_fLastTimeUpdate += rand() % 2000; //Make sure we dont check folders in same cycle.
										struct stat sb;
										if (stat(pNewFolder->m_sFolderFullPath.Get(), &sb) == 0) {
											if (sb.st_mtime != pNewFolder->m_tFolderModify) {
												pNewFolder->m_tFolderModify = sb.st_mtime;
												RefreshEntityFolder(pNewFolder->m_sFolderFullPath.Get(), pNewFolder);
											}
										}
									}
									if (pNewFolder->m_pFirstFile)
									{

										bool bHeaderDisplayed = false;
										bool bDisplayText = true;
										float fWinWidth = ImGui::GetWindowSize().x - 10.0; // Flicker - ImGui::GetCurrentWindow()->ScrollbarSizes.x;
										if (iColumnsWidth >= fWinWidth && fWinWidth > media_icon_size)
										{
											iColumnsWidth = fWinWidth;
										}
										int iColumns = (int)(ImGui::GetWindowSize().x / (iColumnsWidth));
										if (iColumns <= 1)
											iColumns = 1;

										cFolderItem::sFolderFiles * myfiles = pNewFolder->m_pFirstFile->m_pNext;
										while (myfiles)
										{
											std::string sFinal = Left(myfiles->m_sName.Get(), Len(myfiles->m_sName.Get()) - 4);

											if (splitloop == 0 && myfiles->iFlags == 1)
												multi_selections_count++;

											bool bIsVisible = true;
											if (i == 0 && isMarkers) bIsVisible = false;
											if (i == 1 && !isMarkers) bIsVisible = false;

											if (bIsVisible && strlen(cSearchAllEntities[i]) > 0) {
												if (!pestrcasestr(myfiles->m_sName.Get(), cSearchAllEntities[i]))
													bIsVisible = false;
											}
											else if (i == 1 && cFilter[0] == '*') {
												//Others not already displayed.
												for (int fl = 0; fl < splitloop; fl++) {
													if (pestrcasestr(myfiles->m_sName.Get(), cAllFilters[fl]))
														bIsVisible = false;
												}
											}
											else if (strlen(cFilter) > 0 && cFilter[0] != '*') {
												if (!pestrcasestr(myfiles->m_sName.Get(), cFilter))
													bIsVisible = false;
											}


											if (bDisplayEverythingHere)
												bIsVisible = true;
											if (bHideEverythingHere)
												bIsVisible = false;

											ImGui::PushID(uniqueId + preview_count);
											if (splitloop == 0)
												uniqueId++;

											int textureId = 0;
											if (myfiles->iPreview <= 0)
											{
												//Only Visible.
												int gcpy = ImGui::GetCursorPosY();
												if (!bReleaseIconsDynamic || (splitloop == 0 && (bIsVisible || isMarkers) && (gcpy < iIconVisiblePosY && gcpy >= ImGui::GetScrollY() - media_icon_size || isMarkers)))
												{
													myfiles->last_used = (long)tCurrentTimeSec;
													if (max_load_persync-- >= 0)
													{
														//Load preview.
														std::string sImgName = myfiles->m_sPath.Get();
														sImgName = sImgName + "\\" + Left(myfiles->m_sName.Get(), Len(myfiles->m_sName.Get()) - 4);
														sImgName += ".bmp";
														myfiles->iPreview = uniqueId; //TOOL_ENTITY; //Just for testing.
														SetMipmapNum(1); //PE: mipmaps not needed.
														image_setlegacyimageloading(true);
														
														//cyb
														char *fp = new char[sImgName.length() + 1];
														strcpy(fp, sImgName.c_str());
														if (FileExist(fp) == 0)
														{
															sImgName = (g.mysystem.root_s + "files\\editors\\gfx\\missing.bmp").Get();
														}
														delete[] fp;
													
														LoadImage((char *)sImgName.c_str(), myfiles->iPreview);
														image_setlegacyimageloading(false);
														SetMipmapNum(-1);

														if (!GetImageExistEx(myfiles->iPreview))
														{
															myfiles->iPreview = TOOL_ENTITY;
															textureId = TOOL_ENTITY;
														}
														else {
															loaded_images++;
															textureId = myfiles->iPreview;
														}
													}
													else
														textureId = TOOL_ENTITY;
												}
												else {
													textureId = TOOL_ENTITY;
												}
											}
											else
											{
												//PE: Only delete in first run. so we dont delete a image that has already been sent to rendering.
												if (splitloop == 0 && bReleaseIconsDynamic) {
													//Only NOT Visible with a preview image..
													int gcpy = ImGui::GetCursorPosY();
													if (!isMarkers && (!(gcpy < iIconVisiblePosY && gcpy >= ImGui::GetScrollY() - media_icon_size) || !bIsVisible)) {

														if ((long)tCurrentTimeSec - myfiles->last_used > 20) {
															//Delete Image not visible for 20 sec.
															if (GetImageExistEx(myfiles->iPreview) && myfiles->iPreview >= 4000 && myfiles->iPreview < UIV3IMAGES) { //PE: Need to protect system images after tool img range has changed. (myfiles->iPreview can be a system icon)
																image_setlegacyimageloading(true);
																DeleteImage(myfiles->iPreview);
																image_setlegacyimageloading(false);
																myfiles->iPreview = 0;
																loaded_images--;
															}
															textureId = TOOL_ENTITY;
														}
														else
															textureId = myfiles->iPreview;
													}
													else {
														//Still visible update time.
														if (bIsVisible || isMarkers)
															myfiles->last_used = (long)tCurrentTimeSec;
														textureId = myfiles->iPreview;
													}
												}
												else
													textureId = myfiles->iPreview;
											}

											//Is object visible
											if (bIsVisible) {

												if (myfiles->iPreview > 0 && !GetImageExistEx(myfiles->iPreview)) {
													myfiles->iPreview = 0;
													textureId = TOOL_ENTITY;
												}

												if (!bHeaderDisplayed) {

													if (!isMarkers && i == 0)
													{
														ImGui::SetWindowFontScale(1.25);
														LPSTR pFinalHeaderTitle = (LPSTR)dir_name.c_str();
														if (stricmp(pFinalHeaderTitle, "user") == NULL) pFinalHeaderTitle = "Custom Assets";
														if (stricmp(pFinalHeaderTitle, "user - charactercreatorplus") == NULL) pFinalHeaderTitle = "Custom Characters";
														if (stricmp(pFinalHeaderTitle, "user - ebestructures") == NULL) pFinalHeaderTitle = "Custom Structures";
														ImGui::Text("%s", pFinalHeaderTitle);
														ImGui::Spacing();
													}
													else if (strlen(cHeader) > 0)
													{
														ImGui::SetWindowFontScale(1.25);
														ImGui::Text("%s", cHeader);
														ImGui::Spacing();
													}

													ImGui::Columns(iColumns, "filescolumns4entities", false);  //false no border
													bHeaderDisplayed = true;
												}
												ImGui::SetWindowFontScale(SMALLFONTSIZE);

												float fFramePadding = (iColumnsWidth - media_icon_size)*0.5;
												float fCenterX = iColumnsWidth * 0.5;

												ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(fFramePadding, 2.0f));

												if (myfiles->iFlags == 1) {
													ImVec4 bg_col = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram]; // { 0.0, 0.0, 0.0, 1.0 };
													ImVec2 padding = { 6.0, 6.0 };
													ImGuiWindow* window = ImGui::GetCurrentWindow();
													const ImRect image_bb((window->DC.CursorPos - padding) + ImVec2(fFramePadding, 2.0f), window->DC.CursorPos + padding + ImVec2(fFramePadding, 2.0f) + ImVec2(media_icon_size, media_icon_size));
													window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(bg_col), 0.0f, 15, 3.0f);
													bAnySelectedItemsAvailable = true;
												}

												CheckTutorialAction(sFinal.c_str(), 13.0f); //Tutorial: check if we are waiting for this action

												//PE: Support Shift fo selecting may items.
												if (!io.KeyShift) {
													//firstShiftFile = NULL;
													lastShiftFile = NULL;
												}

												if (firstShiftFile && lastShiftFile) {

													if (myfiles == firstShiftFile)
														bFirstShiftHasBeenSeen = true;
													if (!bFirstShiftHasBeenSeen && myfiles == lastShiftFile)
													{
														//Swap around, last is first.
														cFolderItem::sFolderFiles * tmpShiftFile = lastShiftFile;
														lastShiftFile = firstShiftFile;
														firstShiftFile = tmpShiftFile;
														bFirstShiftHasBeenSeen = true;
													}

													static bool bStartShiftActive = false;
													myfiles->iFlags = 0;
													if (myfiles == firstShiftFile) {
														bStartShiftActive = true;
														myfiles->iFlags = 1;
													}
													if (myfiles == lastShiftFile) {
														bStartShiftActive = false;
														myfiles->iFlags = 1;
													}
													if (bStartShiftActive)
														myfiles->iFlags = 1;
												}


												if (ImGui::ImgBtn(textureId, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true))
												{

													if (bTutorialCheckAction) TutorialNextAction(); //Clicked get next tutorial action.

													//If ctrl , just mark them.

													if (io.KeyShift) {
														if (firstShiftFile)
														{
															lastShiftFile = myfiles;
														}
														else
														{
															firstShiftFile = myfiles;
															multi_selections = true;
															myfiles->iFlags = 1;
														}
													}
													else if (io.KeyCtrl) {
														//Mark object.
														multi_selections = true;
														if (myfiles->iFlags == 0) {
															myfiles->iFlags = 1;
															firstShiftFile = myfiles;
														}
														else
															myfiles->iFlags = 0;
													}
													else
													{
														if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
														if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
														//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb

														//Make sure we are in entity mode.
														bForceKey = true;
														csForceKey = "e";

														DeleteWaypointsAddedToCurrentCursor();
														CheckTooltipObjectDelete();
														CloseDownEditorProperties();

														std::string sFpeName = path_for_filename.c_str();
														sFpeName = sFpeName + "\\" + myfiles->m_sName.Get();
														t.addentityfile_s = sFpeName.c_str();
														if (t.addentityfile_s != "")
														{
															entity_adduniqueentity(false);
															t.tasset = t.entid;
															if (t.talreadyloaded == 0)
															{
																editor_filllibrary();
															}
														}
														t.inputsys.constructselection = t.tasset;

														t.gridentity = t.entid;
														t.inputsys.constructselection = t.entid;
														t.inputsys.domodeentity = 1;
														t.grideditselect = 5;

														editor_refresheditmarkers();
														//PE: Close window for now.
														bCheckForClosing = true;
													}
												}

												//if (!bEntity_Properties_Window && !g_bCharacterCreatorPlusActivated && !bImporter_Window && i == 0 && ImGui::IsItemHovered()) {
												if (!bEntity_Properties_Window && !bImporter_Window && i == 0 && ImGui::IsItemHovered()) { //cyb

													iTooltipHoveredTimer = Timer();
													static void* additionalcheck = NULL;

													if (iLastTooltipSelection != textureId || (additionalcheck != myfiles)) {

														//Check if we need to delete a old tooltip object.
														if (iTooltipLastObjectId > 0 && iTooltipLastObjectId != textureId) {
															CheckTooltipObjectDelete();
														}
														additionalcheck = myfiles;
														iTooltipTimer = iTooltipHoveredTimer;
														iLastTooltipSelection = textureId;
														iTooltipObjectReady = false;

													}
													else {
														if (iTooltipHoveredTimer - iTooltipTimer > 2000) { // 2 sec before starting.
															if (iTooltipObjectReady) {
																if (iTooltipLastObjectId > 0) {

																	if (GetImageExistEx(g.importermenuimageoffset + 50)) {
																		float TooltipImageSize = 320.0f;
																		float ImgX = ImageWidth(g.importermenuimageoffset + 50);
																		float ImgY = ImageHeight(g.importermenuimageoffset + 50);
																		float Ratio = TooltipImageSize / ImgX;
																		ImgY *= Ratio;
																		ImVec2 cursor_pos = ImGui::GetIO().MousePos;
																		ImVec2 tooltip_offset(10.0f, ImGui::GetFontSize()*1.5);
																		ImVec2 tooltip_position = cursor_pos;
																		if (tooltip_position.x + TooltipImageSize > GetDesktopWidth())
																			tooltip_position.x -= TooltipImageSize;
																		if (tooltip_position.y + TooltipImageSize > GetDesktopHeight())
																			tooltip_position.y -= (TooltipImageSize + ImGui::GetFontSize()*3.0);
																		tooltip_position.x += tooltip_offset.x;
																		tooltip_position.y += tooltip_offset.y;
																		ImGui::SetNextWindowPos(tooltip_position);
																		ImGui::SetNextWindowContentWidth(TooltipImageSize);
																		ImGui::BeginTooltip();
																		float icon_ratio;
																		ImGui::ImgBtn(g.importermenuimageoffset + 50, ImVec2(TooltipImageSize, ImgY), ImVec4(0.0, 0.0, 0.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false);
																		//char hchar[MAX_PATH];
																		//ImGui::Text("%s", hchar);
																		ImGui::EndTooltip();

																	}
																	else
																		ImGui::SetTooltip("%s", sFinal.c_str());
																}
															}
															else {
																//Generate Thumbnail of object.
																std::string sFpeName = path_for_filename.c_str();
																sFpeName = sFpeName + "\\" + myfiles->m_sName.Get();
																t.addentityfile_s = sFpeName.c_str();


																	t.entdir_s = "entitybank\\";
																	if (cstr(Lower(Left(t.addentityfile_s.Get(), 11))) == "entitybank\\")
																	{
																		t.addentityfile_s = Right(t.addentityfile_s.Get(), Len(t.addentityfile_s.Get()) - 11);
																	}
																	if (cstr(Lower(Left(t.addentityfile_s.Get(), 8))) == "ebebank\\")
																	{
																		t.entdir_s = "";
																	}

																	t.talreadyloaded = 0;
																	for (t.t = 1; t.t <= g.entidmaster; t.t++)
																	{
																		if (t.entitybank_s[t.t] == t.addentityfile_s) { t.talreadyloaded = 1; t.entid = t.t; }
																	}
																	if (t.talreadyloaded == 0)
																	{

																		//  Allocate one more entity item in array
																		if (g.entidmaster > g.entitybankmax - 4)
																		{
																			Dim(t.tempentitybank_s, g.entitybankmax);
																			for (t.t = 0; t.t <= g.entitybankmax; t.t++) t.tempentitybank_s[t.t] = t.entitybank_s[t.t];
																			++g.entitybankmax;
																			UnDim(t.entitybank_s);
																			Dim(t.entitybank_s, g.entitybankmax);
																			for (t.t = 0; t.t <= g.entitybankmax - 1; t.t++) t.entitybank_s[t.t] = t.tempentitybank_s[t.t];
																		}

																		//  Add entity to bank
																		++g.entidmaster; entity_validatearraysize();
																		t.entitybank_s[g.entidmaster] = t.addentityfile_s;

																		if (ObjectExist(g.entitybankoffset + g.entidmaster)) {
																			DeleteObject(g.entitybankoffset + g.entidmaster);
																		}

																		//  Load extra entity
																		t.entid = g.entidmaster;
																		t.ent_s = t.entitybank_s[t.entid];
																		t.entpath_s = getpath(t.ent_s.Get());

																		extern bool g_bGracefulWarningAboutOldXFiles;
																		g_bGracefulWarningAboutOldXFiles = true;
																		entity_load();
																		g_bGracefulWarningAboutOldXFiles = false;
																		HideObject(g.entitybankoffset + g.entidmaster);
																		if (t.entityprofile[g.entidmaster].ischaracter == 1) {
																			RotateObject(g.entitybankoffset + g.entidmaster, 0, 180, 0);
																		}
																		g.entidmaster--; //Dont actual add it.

																		//entity_load can change folder by creating a dbo , so update timestamp without refresh.
																		struct stat sb;
																		if (stat(pNewFolder->m_sFolderFullPath.Get(), &sb) == 0) {
																			if (sb.st_mtime != pNewFolder->m_tFolderModify) {
																				pNewFolder->m_tFolderModify = sb.st_mtime;
																			}
																		}


																		//Create a new thumbnail.
																	}

																	iTooltipLastObjectId = t.entid;
																	iTooltipAlreadyLoaded = t.talreadyloaded;
																	iTooltipObjectReady = true;
																	iLaunchAfterSync = 30; //Generate the thumb.
															}
														}
													}
												}

												if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
												{

													CheckTooltipObjectDelete();
													CloseDownEditorProperties();
													t.inputsys.constructselection = 0;

													myfiles->m_dropptr = myfiles;

													std::string sFpeName = path_for_filename.c_str();
													sFpeName = sFpeName + "\\" + myfiles->m_sName.Get();

													myfiles->m_sFolder = sFpeName.c_str();
													ImGui::SetDragDropPayload("DND_MODEL_DROP_TARGET", myfiles, sizeof(void *));
													ImGui::ImgBtn(textureId, ImVec2(media_icon_size, media_icon_size), drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, 0, 0, 0, 0, true);
													//ImGui::Text("%s", myfiles->m_sName.Get());
													ImGui::SetCursorPos(oldCursor);
													pDragDropFile = myfiles;
													ImGui::EndDragDropSource();
												}

												ImGui::PopStyleVar();

												if (bDisplayText) {
													int iTextWidth = ImGui::CalcTextSize(sFinal.c_str()).x;
													if (iTextWidth < iColumnsWidth)
														ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (fCenterX - (iTextWidth*0.5)), ImGui::GetCursorPosY()));
													ImGui::TextWrapped("%s", sFinal.c_str());
												}

												ImGui::NextColumn();
											}
											ImGui::PopID();
											preview_count++;

											myfiles = myfiles->m_pNext;
										}

										ImGui::Columns(1);

										ImGui::SetWindowFontScale(1.0);
									}
								}
								pNewFolder = pNewFolder->m_pNext;
							}

						}
					}

					if (!bAnySelectedItemsAvailable) {
						//PE: We got no selections , we can reset first shift seen.
						if (!io.KeyShift) {
							firstShiftFile = NULL;
						}
					}
					ImGui::SetWindowFontScale(1.0);

					ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
					if (ImGui::IsMouseHoveringRect(bbwin.Min, bbwin.Max))
					{
						bImGuiGotFocus = true;
						bEntityGotFocus = true;
					}
					if (ImGui::IsAnyItemFocused()) {
						bImGuiGotFocus = true;
						bEntityGotFocus = true;
					}


					if (lf_multi_selections_count > 0) {
						ImGui::SetCursorPos(ImVec2(0, (ImGui::GetWindowSize().y + ImGui::GetScrollY()) - insert_text_width.y - 10.0f));
						ImGui::Spacing();
						ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (insert_text_width.x + 18.0f));
						ImGui::SetItemAllowOverlap();
						if (ImGui::StyleButton(insert_text.c_str())) {
							bAddSelectionToGame = true;
						}

					}

					ImGui::EndChild();

					ImGui::EndTabItem();
				}

				ImRect bbwin(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize());
				if (ImGui::IsMouseHoveringRect(bbwin.Min, bbwin.Max))
				{
					bImGuiGotFocus = true;
					bEntityGotFocus = true;
				}
				if (ImGui::IsAnyItemFocused()) {
					bImGuiGotFocus = true;
					bEntityGotFocus = true;
				}

			}
		}

#ifdef DYNAMICLOADUNLOAD
		max_load_persync = 10; // 15 to slow try 10
#endif
		lf_multi_selections_count = multi_selections_count; //Use last frames count.

		ImGui::EndTabBar();


		if (bAddSelectionToGame) {

			DeleteWaypointsAddedToCurrentCursor();
			CheckTooltipObjectDelete();
			CloseDownEditorProperties();

			//Remove any selections.
			t.inputsys.constructselection = 0;
			if (t.gridentityobj > 0)
			{
				DeleteObject(t.gridentityobj);
				t.gridentityobj = 0;
			}
			t.refreshgrideditcursor = 1;
			t.gridentity = 0;
			t.gridentityposoffground = 0;
			t.gridentityusingsoftauto = 0;
			editor_refresheditmarkers();

			cFolderItem *pSearchFolder = &MainEntityList;
			pSearchFolder = pSearchFolder->m_pNext;
			cStr path_remove;
			int ipath_remove_len;
			if (pSearchFolder) {
				path_remove = pSearchFolder->m_sFolderFullPath.Get();
				ipath_remove_len = path_remove.Len();
			}
			while (pSearchFolder) {

				cStr path = pSearchFolder->m_sFolderFullPath.Get();
				bool bDoubleEntityBank = false;
				char *finde = (char *)pestrcasestr(path.Get(), "\\entitybank"); //Support entitybank inside entitybank.
				if (finde)
				{
					finde += 11;
					finde = (char *)pestrcasestr(finde, "\\entitybank");
					if (finde) bDoubleEntityBank = true;
				}

				if (!bDoubleEntityBank && path.Right(11) == "\\entitybank") {
					ipath_remove_len = path.Len();
				}
				else
				{
					if (pSearchFolder->m_pFirstFile) {

						cFolderItem::sFolderFiles * searchfiles = pSearchFolder->m_pFirstFile->m_pNext;
						while (searchfiles) {
							if (searchfiles->iFlags == 1) {
								if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
								if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
								//if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false; //cyb

								//Insert.
								cStr path = pSearchFolder->m_sFolderFullPath.Get();
								char *final_name = path.Get();
								final_name += ipath_remove_len;
								if (*final_name == '\\')
									final_name++;
								std::string path_for_filename = final_name;

								std::string sFpeName = path_for_filename.c_str();
								sFpeName = sFpeName + "\\" + searchfiles->m_sName.Get();

								t.addentityfile_s = sFpeName.c_str();
								if (t.addentityfile_s != "")
								{
									entity_adduniqueentity(false);
									t.tasset = t.entid;
									if (t.talreadyloaded == 0)
									{
										editor_filllibrary();
									}
								}

								searchfiles->iFlags = 0;
							}
							searchfiles = searchfiles->m_pNext;
						}
					}
				}
				pSearchFolder = pSearchFolder->m_pNext;
			}
			bCheckForClosing = true;
		}

		bool bAreWeOverLapping = false;
		if (!bIsWeDocked) {
			//If we are over the rendertarget hide window.
			float itmpmousex = ImGui::GetWindowPos().x;
			float itmpmousey = ImGui::GetWindowPos().y;
			int iSecureZone = 4;
			if (bImGuiRenderTargetFocus && itmpmousex >= (renderTargetAreaPos.x + iSecureZone) && itmpmousey >= (renderTargetAreaPos.y + iSecureZone) &&
				itmpmousex <= renderTargetAreaPos.x + (renderTargetAreaSize.x - iSecureZone) && itmpmousey <= renderTargetAreaPos.y + (renderTargetAreaSize.y - ImGuiStatusBar_Size - iSecureZone))
			{
				bAreWeOverLapping = true;
			}
			itmpmousex = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
			itmpmousey = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
			if (bExternal_Entities_Window && bImGuiRenderTargetFocus && itmpmousex >= (renderTargetAreaPos.x + iSecureZone) && itmpmousey >= (renderTargetAreaPos.y + iSecureZone) &&
				itmpmousex <= renderTargetAreaPos.x + (renderTargetAreaSize.x - iSecureZone) && itmpmousey <= renderTargetAreaPos.y + (renderTargetAreaSize.y - ImGuiStatusBar_Size - iSecureZone))
			{
				bAreWeOverLapping = true;
			}
			itmpmousex = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
			itmpmousey = ImGui::GetWindowPos().y;
			if (bExternal_Entities_Window && bImGuiRenderTargetFocus && itmpmousex >= (renderTargetAreaPos.x + iSecureZone) && itmpmousey >= (renderTargetAreaPos.y + iSecureZone) &&
				itmpmousex <= renderTargetAreaPos.x + (renderTargetAreaSize.x - iSecureZone) && itmpmousey <= renderTargetAreaPos.y + (renderTargetAreaSize.y - ImGuiStatusBar_Size - iSecureZone))
			{
				bAreWeOverLapping = true;
			}
			itmpmousex = ImGui::GetWindowPos().x;
			itmpmousey = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
			if (bExternal_Entities_Window && bImGuiRenderTargetFocus && itmpmousex >= (renderTargetAreaPos.x + iSecureZone) && itmpmousey >= (renderTargetAreaPos.y + iSecureZone) &&
				itmpmousex <= renderTargetAreaPos.x + (renderTargetAreaSize.x - iSecureZone) && itmpmousey <= renderTargetAreaPos.y + (renderTargetAreaSize.y - ImGuiStatusBar_Size - iSecureZone))
			{
				bAreWeOverLapping = true;
			}
		}
		if (!bIsWeDocked && bCheckForClosing) {
			if (bAreWeOverLapping)
				bExternal_Entities_Window = false;
		}

		//Remove window on right click if we are overlapping.
		if (bAreWeOverLapping && t.inputsys.mclick == 2)
			bExternal_Entities_Window = false;


		bCheckForClosing = false;

		CheckMinimumDockSpaceSize(250.0f);

		ImGui::End();

	}

}


#endif //VRTECH

void FixEulerZInverted(float &ax, float &ay, float &az)
{
	if (ax < 0.0f) ax += 360.0f;
	if (ay < 0.0f) ay += 360.0f;
	if (az < 0.0f) az += 360.0f;
	if (ax > 360.0f) ax -= 360.0f;
	if (ay > 360.0f) ay -= 360.0f;
	if (az > 360.0f) az -= 360.0f;
	bool bZFlipped = false, bXFlipped = false, bXDeadPos = false , bZDeadPos = false;
	if (az >= 179.5f && az <= 180.5f) bZFlipped = true; // 180
	if (ax >= 179.5f && ax <= 180.5f) bXFlipped = true; // 180
	if (ax >= 89.5f && ax <= 90.5f) bXDeadPos = true; // 90
	if (az >= 299.5f && az <= 300.5f) bZDeadPos = true; // 300 Got a 299.7 , so lowered to 299.5.
	if (bZFlipped && bXFlipped)
	{
		//PE: When both z and x flipped it count backward, so 180-ay = real Y without x,z.
		ax = 0.0f;
		az = 0.0f;
		ay = (180.0 - ay);
	}
	else if (!bXDeadPos && bZFlipped)
	{
		ax -= 180.0f;
		ay = (180.0 - ay);
		az = 0.0f;
	}
	else if (bXDeadPos && bZDeadPos)
	{
		//Y dont change. z is just moved to x
		ax = az + 90;
		az = 0.0f;
	}
	if (ax < 0.0f) ax += 360.0f;
	if (ay < 0.0f) ay += 360.0f;
	if (az < 0.0f) az += 360.0f;
	if (ax > 360.0f) ax -= 360.0f;
	if (ay > 360.0f) ay -= 360.0f;
	if (az > 360.0f) az -= 360.0f;
	return;
}

void SetStartPositionsForRubberBand(int iActiveObj)
{
	// for multiple objects
	if (g.entityrubberbandlist.size() > 0)
	{
		// for each object in the selection
		for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
		{
			int e = g.entityrubberbandlist[i].e;
			int tobj = t.entityelement[e].obj;
			if (tobj > 0)
			{
				if (ObjectExist(tobj) == 1)
				{
					// store starting position and orientation of objects
					g.entityrubberbandlist[i].x = ObjectPositionX(tobj) - ObjectPositionX(iActiveObj);
					g.entityrubberbandlist[i].y = ObjectPositionY(tobj) - ObjectPositionY(iActiveObj);
					g.entityrubberbandlist[i].z = ObjectPositionZ(tobj) - ObjectPositionZ(iActiveObj);
					GGQUATERNION QuatAroundX, QuatAroundY, QuatAroundZ;
					GGQuaternionRotationAxis(&QuatAroundX, &GGVECTOR3(1, 0, 0), GGToRadian(ObjectAngleX(tobj)));
					GGQuaternionRotationAxis(&QuatAroundY, &GGVECTOR3(0, 1, 0), GGToRadian(ObjectAngleY(tobj)));
					GGQuaternionRotationAxis(&QuatAroundZ, &GGVECTOR3(0, 0, 1), GGToRadian(ObjectAngleZ(tobj)));
					g.entityrubberbandlist[i].quatAngle = QuatAroundX * QuatAroundY * QuatAroundZ;
				}
			}
		}
	}
}

void RotateAndMoveRubberBand(int iActiveObj, float fMovedActiveObjectX, float fMovedActiveObjectY, float fMovedActiveObjectZ, GGQUATERNION quatRotationEvent )//float fMovedActiveObjectRX, float fMovedActiveObjectRY, float fMovedActiveObjectRZ)
{
	// for multiple objects
	if (g.entityrubberbandlist.size() > 0)
	{
		// for each object in the selection
		for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
		{
			int e = g.entityrubberbandlist[i].e;
			int tobj = t.entityelement[e].obj;
			if (tobj > 0 && t.entityelement[e].editorlock == 0)
			{
				if (ObjectExist(tobj) == 1)
				{
					if (tobj != iActiveObj)
					{
						// this object rotyation quat
						GGQUATERNION quatThisOrientation = g.entityrubberbandlist[i].quatAngle;

						// apply the rotation event to the object angle
						GGQUATERNION quatNewOrientation;
						GGQuaternionMultiply(&quatNewOrientation, &quatThisOrientation, &quatRotationEvent);

						// rotate this object with final quat and get new entity rotation eulers
						RotateObjectQuat(tobj, quatNewOrientation.x, quatNewOrientation.y, quatNewOrientation.z, quatNewOrientation.w);
						t.entityelement[e].rx = ObjectAngleX(tobj);
						t.entityelement[e].ry = ObjectAngleY(tobj);
						t.entityelement[e].rz = ObjectAngleZ(tobj);

						// apply the rotation event to the position
						GGVECTOR3 positionalOffset;
						positionalOffset.x = g.entityrubberbandlist[i].x;
						positionalOffset.y = g.entityrubberbandlist[i].y;
						positionalOffset.z = g.entityrubberbandlist[i].z;
						GGMATRIX matRotatePositions;
						GGMatrixRotationQuaternion(&matRotatePositions, &quatRotationEvent);
						GGVec3TransformCoord(&positionalOffset, &positionalOffset, &matRotatePositions);

						// if object was static, flag that static object moved
						if (t.entityelement[e].staticflag == 1) g.projectmodifiedstatic = 1;

						// put new adjusted positions back
						t.entityelement[e].x = ObjectPositionX(iActiveObj) + positionalOffset.x;
						t.entityelement[e].y = ObjectPositionY(iActiveObj) + positionalOffset.y;
						t.entityelement[e].z = ObjectPositionZ(iActiveObj) + positionalOffset.z;

						// update entity to new offset position if any movement
						t.entityelement[e].x = t.entityelement[e].x + fMovedActiveObjectX;
						t.entityelement[e].y = t.entityelement[e].y + fMovedActiveObjectY;
						t.entityelement[e].z = t.entityelement[e].z + fMovedActiveObjectZ;

						// finally update latest object position
						PositionObject(tobj, t.entityelement[e].x, t.entityelement[e].y, t.entityelement[e].z);

						// update light data for spot
						if (t.entityelement[e].eleprof.usespotlighting)	lighting_refresh();

						// move zones and lights if in group
						widget_movezonesandlights(e);
					}
				}
			}
		}
	}
}


/* old broken method
void RotateRubberBandYOrg(int iActiveObj, float fOldActiveObjectRX, float fOldActiveObjectRY, float fOldActiveObjectRZ, float fAngle0, float fAngle1, float fAngle2)
{
	// leelee, all this has been duplicated three times now (need a single call to handle modification pos, rot and scale of rubber band grouped entities at some fine time)
	if (g.entityrubberbandlist.size() > 0)
	{
		// rotate all the grouped entities and move around Y axis of widget as pivot
		for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
		{
			int e = g.entityrubberbandlist[i].e;
			GGVECTOR3 VecPos;
			VecPos.x = t.entityelement[e].x - ObjectPositionX(iActiveObj);
			VecPos.y = t.entityelement[e].y - ObjectPositionY(iActiveObj);
			VecPos.z = t.entityelement[e].z - ObjectPositionZ(iActiveObj);
			// transform offset with current inversed orientation of primary object
			int tobj = t.entityelement[e].obj;
			if (tobj > 0)
			{
				float fDet = 0.0f;
				sObject* pObject = GetObjectData(tobj);
				GGMATRIX inverseMatrix = pObject->position.matObjectNoTran;
				GGMatrixInverse(&inverseMatrix, &fDet, &inverseMatrix);
				GGVec3TransformCoord(&VecPos, &VecPos, &inverseMatrix);
				//PE: Rubberband .x is int , this gives float rounding errors, changed to float.
				g.entityrubberbandlist[i].x = VecPos.x;
				g.entityrubberbandlist[i].y = VecPos.y;
				g.entityrubberbandlist[i].z = VecPos.z;
			}
		}

		// rotate all the grouped entities and move around Y axis of widget as pivot
		float fMovedActiveObjectRX = fAngle0 - fOldActiveObjectRX;
		float fMovedActiveObjectRY = fAngle1 - fOldActiveObjectRY;
		float fMovedActiveObjectRZ = fAngle2 - fOldActiveObjectRZ;
		for (int i = 0; i < (int)g.entityrubberbandlist.size(); i++)
		{
			int e = g.entityrubberbandlist[i].e;
			int tobj = t.entityelement[e].obj;
			if (tobj > 0)
			{
				if (ObjectExist(tobj) == 1)
				{
					if (tobj != iActiveObj)
					{
						// a fix for entities that have been inverted
						if (t.entityelement[e].rx == 180 && t.entityelement[e].rz == 180)
						{
							t.entityelement[e].rx = 0;
							t.entityelement[e].ry = t.entityelement[e].ry;
							t.entityelement[e].rz = 0;
							RotateObject(tobj, t.entityelement[e].rx, t.entityelement[e].ry, t.entityelement[e].rz);
						}

						// rotate the entity
						RotateObject(tobj, ObjectAngleX(tobj) + fMovedActiveObjectRX, ObjectAngleY(tobj) + fMovedActiveObjectRY, ObjectAngleZ(tobj) + fMovedActiveObjectRZ);
						t.entityelement[e].rx = ObjectAngleX(tobj);
						t.entityelement[e].ry = ObjectAngleY(tobj);
						t.entityelement[e].rz = ObjectAngleZ(tobj);
						if (t.entityelement[e].staticflag == 1) g.projectmodifiedstatic = 1;

						// move the entity around a pivot point
						GGVECTOR3 VecPos;
						VecPos.x = g.entityrubberbandlist[i].x;
						VecPos.y = g.entityrubberbandlist[i].y;
						VecPos.z = g.entityrubberbandlist[i].z;
						sObject* pObject = GetObjectData(tobj);

						GGVec3TransformCoord(&VecPos, &VecPos, &pObject->position.matObjectNoTran);
						t.entityelement[e].x = ObjectPositionX(iActiveObj) + VecPos.x;
						t.entityelement[e].y = ObjectPositionY(iActiveObj) + VecPos.y;
						t.entityelement[e].z = ObjectPositionZ(iActiveObj) + VecPos.z;
						PositionObject(tobj, t.entityelement[e].x, t.entityelement[e].y, t.entityelement[e].z);

						// move zones and lights if in group
						widget_movezonesandlights(e);
					}
				}
			}
		}
	}
}
*/


//PE: This should only run in standalone to precache all slow loading objects.
cstr GenericFileCacheName = "";
void CreateGenericFileCacheName(char *file)
{
	std::string cache_name = file;
	std::transform(cache_name.begin(), cache_name.end(), cache_name.begin(), [](unsigned char c) { return ::tolower(c); });
	replaceAll(cache_name, ".x", ".dbo");
	replaceAll(cache_name, ".png", ".dds");
	replaceAll(cache_name, ".fpm", ".obs");
	replaceAll(cache_name, "entitybank\\", "");
	replaceAll(cache_name, "gamecore\\", "");
	replaceAll(cache_name, "\\", "_");
	replaceAll(cache_name, "/", "_");
	replaceAll(cache_name, "\"", ""); //Got this when deleting a file in user, if fpe not found and already in list.
	std::string cache_final_name = cache_name;
	cache_final_name = g.mysystem.cachebank_s.Get() + cache_final_name;
	GenericFileCacheName = cache_final_name.c_str();
}
void CreateCacheXFile(char *pFilename,void* pBlock, DWORD dwBlockSize)
{
	if (t.game.gameisexe == 0) return;
	CreateGenericFileCacheName(pFilename);
	DBOSaveBlockFile(GenericFileCacheName.Get(), pBlock, dwBlockSize);
}
bool bCheckCacheXFile(LPSTR pFilename, DWORD* pdwBlockSize, void** ppDBOBlock)
{
	if (t.game.gameisexe == 0) return false;
	CreateGenericFileCacheName(pFilename);
	if (FileExist(GenericFileCacheName.Get()))
	{
		return LoadDBODataBlock(GenericFileCacheName.Get(), pdwBlockSize, ppDBOBlock);
	}
	else
		return false;
}

bool bCopyOBSFileToCache(void)
{
	if (t.game.gameisexe == 1)
	{
		CreateGenericFileCacheName(g.projectfilename_s.Get());
		t.tobsfile_s = g.mysystem.levelBankTestMap_s + "map.obs";
		CopyFile(t.tobsfile_s.Get(), GenericFileCacheName.Get(), false);
		return true;
	}
	return false;
}

bool bLoadOBSFileFromCache( void )
{
	if (t.game.gameisexe == 1)
	{
		CreateGenericFileCacheName(g.projectfilename_s.Get());
		if (FileExist(GenericFileCacheName.Get()) == 1)
		{
			AILoadObstacleData(0, GenericFileCacheName.Get());
			return true;
		}
	}
	return false;
}
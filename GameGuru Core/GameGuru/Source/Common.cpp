//----------------------------------------------------
//--- GAMEGURU - Common
//----------------------------------------------------

// Includes 
#include "stdafx.h"
#include "gameguru.h"
#include <stdio.h>
#include <stdlib.h>
#include "shellapi.h"
#include "time.h"
#include "direct.h"
#include <wininet.h>
#include "M-WelcomeSystem.h"
#include "Common-Keys.h"
#include "CFtpC.h"
#include "..\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Objects\ShadowMapping\cShadowMaps.h"
#include "..\..\Dark Basic Public Shared\Include\CObjectsC.h"

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


//220mb saved.
#define REDUCEMEMUSE

// Used for Free Weekend Promotion Build 
//#define STEAMOWNERSHIPCHECKFREEWEEKEND

// core externs to globals
extern LPSTR gRefCommandLineString;
extern bool gbAlwaysIgnoreShaderBlobFile;
extern bool g_VR920RenderStereoNow;
extern float g_fVR920Sensitivity;

#ifdef VRTECH
extern bool g_bDisableVRDetectionByUserRequest;
extern bool bStartNewPrompt;
#else
extern bool bStartNewPrompt;
#endif

#ifdef ENABLEIMGUI //cyb
extern bool bStartNewPrompt;
#endif


// Globals
int g_PopupControlMode = 0;
int g_trialStampDaysLeft = 0;
char g_trialDiscountCode[1024];
char g_trialDiscountExpires[1024];
int tgamesetismapeditormode = 1; //PE: Need access to this.

//LB: 32bit converter path to file
char g_pRootFolderConverter[MAX_PATH];

// Externs
// to enable the use of _e_ in standalone
void SetCanUse_e_ ( int flag );
#ifdef VRTECH
char g_pCloudKeyErrorString[10240];
char g_pCloudKeyExpiresDate[11];
bool g_bCloudKeyIsHomeEdition = false;
#endif

// global to store abs path to converter
char g_pAbsPathToConverter[MAX_PATH];

// C++ CONVERSION: g contains all variables that were defined as global in dbpro source
Sglobals g;

// C++ CONVERSION: t contains all variables that were considered temporary and subject to change between routines
Stemps t;

#ifdef VRTECH
void SetCanUse_e_ ( int flag );
void SetWorkshopFolder ( LPSTR pFolder );
#endif

//Subroutines

void common_init ( void )
{
	// if a user needs to decrypt their media from an old GG Classic Game, use this code
	//#define EXTRACTENCRYPTEDMEDIA
	/*
	#ifdef EXTRACTENCRYPTEDMEDIA
	// store current folder to restore later
	cStr pOldDir = GetDir();
	// Create DBPDATA folder for this process
	GetTempPathA(_MAX_PATH, g_WindowsTempDirectory);
	_chdir(g_WindowsTempDirectory);
	_mkdir("dbpdata");
	// path to location of all files we want to decrypt
	LPSTR pPathToClassicEncryptedFiles = "D:\\Hunted One Step Too Far DECRYPTED";
	// set to work path and add everything from Files to a list
	SetDir(pPathToClassicEncryptedFiles);
	addallinfoldertocollection("Files","");
	// go through all files in list and decrypt the ones marked _e_
	SetDir("Files");
	SetCanUse_e_(1);
	for ( int f = 1; f <= g.filecollectionmax; f++ )
	{
		LPSTR pThisFile = t.filecollection_s[f].Get();
		if (strstr(pThisFile, "\\_e_") != NULL)
		{
			// found an encrypted file
			char pVirtualFilename[MAX_PATH];
			strcpy(pVirtualFilename, pThisFile);

			// decrypt file
			g_pGlob->Decrypt( pVirtualFilename );

			// if temp created successfully
			if (FileExist(pVirtualFilename) == 1)
			{
				// create new filename for the unencrypted version (dasdasdsa\\dsadsad\\_e_dsadas.xxx)
				char pActualFile[MAX_PATH];
				strcpy(pActualFile, "");
				char pNewDecryptedFilename[MAX_PATH];
				strcpy(pNewDecryptedFilename, pThisFile);
				for (int n = strlen(pNewDecryptedFilename)-1; n>0; n--)
				{
					if (pNewDecryptedFilename[n] == '\\' || pNewDecryptedFilename[n] == '/')
					{
						strcpy(pActualFile, pNewDecryptedFilename + n + 4);
						pNewDecryptedFilename[n+1] = 0;
						break;
					}
				}
				if (strlen(pActualFile) > 0)
				{
					// copy new decrypted file
					strcat(pNewDecryptedFilename, pActualFile);
					//MessageBoxA(NULL, pNewDecryptedFilename, pNewDecryptedFilename, MB_OK);
					CopyFileA(pVirtualFilename, pNewDecryptedFilename, FALSE);

					// if copy was successful, delete old encrypted file
					if (FileExist(pNewDecryptedFilename) == 1)
					{
						DeleteFileA(pThisFile);
					}
				}
			}
		}
	}
	SetCanUse_e_(0);
	// reset the above list 
	g.filecollectionmax = 0;
	Dim ( t.filecollection_s,500 );
	// restore folder and continue
	SetDir(pOldDir.Get());
	#endif
	*/

	/*
	// copy contents of playermedia to 'expansionobb'
	bool bConvertPlayerMediaToOBBRoot = false;
	if (bConvertPlayerMediaToOBBRoot == true)
	{
		// store current folder to restore later
		cStr pOldDir = GetDir();
		// create OBB folder
		char pOBBFolder[MAX_PATH];
		strcpy(pOBBFolder, pOldDir.Get());
		strcat(pOBBFolder, "\\Files\\expansionobb\\");
		// find all files in playermedia
		SetDir("Files");
		addallinfoldertocollection("playermedia", "");
		SetDir("playermedia");
		for (int f = 1; f <= g.filecollectionmax; f++)
		{
			LPSTR pThisFile = t.filecollection_s[f].Get();
			if (strlen(pThisFile)>0)
			{
				// convert full relative path to underscored name
				char pUnderscored[MAX_PATH];
				strcpy(pUnderscored, pThisFile);
				for (int n = 0; n < strlen(pUnderscored); n++)
				{
					if (pUnderscored[n] == '\\' || pUnderscored[n] == '/')
						pUnderscored[n] = '_';
				}

				// destination
				char pDestFile[MAX_PATH];
				strcpy(pDestFile, pOBBFolder);
				strcat(pDestFile, pUnderscored);

				//MessageBoxA(NULL, pThisFile, pUnderscored, MB_OK);
				if (FileExist(pDestFile) == 1) DeleteFileA(pDestFile);
				CopyFileA(pThisFile, pDestFile, FALSE);
			}
		}
		// reset the above list 
		g.filecollectionmax = 0;
		Dim (t.filecollection_s, 500);
		// restore folder and continue
		SetDir(pOldDir.Get());
		// and we are done
		PostQuitMessage(0);
		return;
	}
	*/

	// work out and store absolute path to converter in root folder
	GetCurrentDirectoryA(MAX_PATH, g_pAbsPathToConverter);
	strcat(g_pAbsPathToConverter, "\\Guru-Converter.exe");

	// determines if EDITOR or GAME right away!
	FPSC_VeryEarlySetup();


	// ensures the DWORD to INT conversion always produces a positive value
	SetLocalTimerReset();

	// no more blue, only black
	if ( CameraExist ( 0 ) == 1 ) BackdropColor ( 0 );

	// lee - 170117 - deactivate old school DB collision system (so can use the collision.bActive for ray cast deactivation)
	GlobalColOff();

	// C++ CONVERSION: initialise globals
	// these were the globals previously defined in types
	common_init_globals();

	//  Init app
	SyncOn ( ); SyncRate ( 0 ); FogOff ( );

	//  initialise character animation system
	//t.charanimstate as charanimstatetype;
	char_init ( );
	t.aisystem.usingphysicsforai=1;

	//  Initialise physics tweakables
	physics_inittweakables ( );

	//  Get Actual screen resolution from desktop resolution
	//  Scene Commander - renamed SW and SH to ScreenW and ScreenH as names too short.

	// C++ CONVERSION: incase t.ScreenW is not 0
	t.ScreenW = 0;

	if (  t.ScreenW == 0 || t.ScreenH == 0 ) 
	{
		t.ScreenW = GetSystemMetrics ( 0 );
		t.ScreenH= GetSystemMetrics ( 1 );
	}
	t.Kernel32 = 1;
	t.memptr = PerformanceTimer();

	#ifdef VRTECH
	// some important resets
	strcpy_s ( g_pCloudKeyErrorString, 10240, "Unknown Validation Error");
	strcpy_s ( g_pCloudKeyExpiresDate, 11, "");
	#endif

	//  flashlight
	g.flashlighton = 0;
	g.flashlightrange = 350;
	g.flashlightred = 255;

	//  limit raycast activity from enemies
	g.gnumberofraycastsallowedincycle = 0;
	g.gnumberofraycastslastoneused = 0;

	//  wobble
	g.wobble_f = 0.0;

	//  new deaths
	g.tiltondeath = 0;
	g.tilton = 0;
	g.tiltspeed_f = 0.0;
	g.temptilt = 0;
	g.thud = 0;
	g.tiltbounce = 10;
	g.justdone = 0;
	//  video
	g.unskip = 0;
	//  guns
	g.crosshairon = 1;
	g.forcedslot = 0;
	//  armour
	g.armour = 100;
	g.armouron = 0;
	g.armx = 18;
	g.army = 8;
	g.bodyon = 0;
	//  air
	g.airon = 0;
	g.airleft = 100;
	g.airmax = 100;
	g.drowntime = 2000;
	g.airtime = 2000;
	g.airtimer = Timer();
	g.drowntimer = Timer();
	g.airx = 24;
	g.airy = 8;
	g.instantdrown = 1;
	g.lastsetair = 0;
	g.drowned = 0;
	//  god mode
	g.isimmune = 0;
	//  new syncrate
	//  compass - knxrb
	g.compassOn = 0;
	g.compassX = 80;
	g.compassY = GetDisplayHeight()- 80;
	g.spritesPasted = 0;
	g.gameStarted = 0;
	g.madeCompass = 0;
	g.needleSpin = 0;
	g.compassSpin = 1;
	g.compassobject = 666666;
	g.needleobject = 666667;
	//  dark ai radar
	g.darkradar = 0;
	g.radarx = GetDisplayWidth()-80;
	g.radary = GetDisplayHeight()-80;
	g.maderadar = 0;
	g.rotateblip = 1;
	g.radarrange = 45;
	g.radarobject = 666669;
	g.blipstart = 666670;
	//  player speed mod
	g.speedmod_f = 100.0;
	//  radar/compass object as objective
	g.objectivemode = 0;
	g.istheobjective = 0;
	g.objectivex = 80;
	g.objectivey = GetDisplayHeight() - 80;
	g.madeobjective = 0;
	g.objectiveobject = 666691;
	g.maxslots = 10;
	//  Scene Commander water performance
	g.waterflec = 400;
	//  Scene Commander culling
	g.cullmode = 1;
	g.cullmodi = 650;
	g.plrfootfall = 1;
	g.forcealtswap = 0;
	g.moveplrx_f = 0.0;
	g.moveplry_f = 0.0;
	g.moveplrz_f = 0.0;
	g.noholster = 1;
	g.noairon = 0;
	g.drowndamage = 1;
	g.pickrange_f = 75.0;
	g.lastpickrange_f = 75.0;
	g.laststrength_f = 4000.0;
	g.lastthrow_f = 100.0;
	g.flashr = 255;
	g.flashg = 255;
	g.flashb = 255;
	g.flashrange = 600;
	g.playerdammult_f = 0.0;
	g.resetonreload = 0;
	g.ecam = 0;
	g.lastcam = 0;
	g.custstart = 0;
	g.custend = 0;
	g.plrreloading = 0;
	g.lockangle = 9999;
	g.fieldoffire = 45;
	g.plrcamoffsetx_f = 0.0;
	g.plrcamoffsety_f = 0.0;
	g.plrcamoffsetz_f = 0.0;
	g.plroffsetanglex_f = 0.0;
	g.plroffsetangley_f = 0.0;
	g.plroffsetanglez_f = 0.0;
	g.linkx = 0;
	g.linky = 0;
	g.linkz = 0;
	g.plrcamoffseton = 0;
	g.eplayercam = 0;
	g.decalrange = 800;
	g.cullmodelast = g.cullmode;
	g.cullmodechange = 0;
	//  Scene Commander - made global as otherwise it is being ignored in functions
	g.weaponammoindex = 0;
	g.ammooffset = 0;
	g.timeelapsed_f = 0.0;
	g.gentityundercursorlocked = 0;

	// `global syncrate=80 rem rem out to fix speed

	g.plrdistance_f = 0.0;
	g.pmaxX_f = 0.0;
	g.pmaxY_f = 0.0;
	g.pmaxZ_f = 0.0;
	g.sizechange = 0;
	g.firstturnjump = 0;
	g.alwaysshowair = 0;
	//  Scene Commander - mouse button timers, for tracking firing and also for conditions which seemed a logical extention condition.
	g.lmbheld = 0;
	g.lmbheldtime = 0;
	g.rmbheld = 0;
	g.rmbheldtime = 0;
	g.jamadjust = 0;
	g.screengrabtimer = Timer();
	g.forcecrouch = 0;

	//  Used to record last best pick 3D coordinate (exact widget pos)

	//  scene commander - average FPS for smoother movement
	g.nextave = 1;
	Dim ( t.fpsstore , 80 ) ; for ( t.f = 1 ; t.f<= 80 ; t.f++ ) t.fpsstore[t.f]=80;
	//  Scene Commander end variables

	// set executable root folder
	if ( g.exeroot_s != "" ) SetDir ( g.exeroot_s.Get() );

	//  Special build flags for 'genre' switch
	g.fpgchud_s = "";
	g.fpgchuds_s = "";
	g.fpgcgenre = 0;
	if ( t.runengineinframe == 1 ) 
	{
		//  FPSC - 260210 - engine in frame to run FPSC in an ActiveX frame
		g.fpgchud_s="gun";
		g.fpgchuds_s="guns";
		g.fpgcgenre=1;
	}
	else
	{
		if ( PathExist("files\\gamecore\\equipment") == 1 ) 
		{
			// FPGC - Equipment Only Genre (no weapons)
			g.fpgchud_s="equipment";
			g.fpgchuds_s="equipment";
			g.fpgcgenre=0;
		}
		else
		{
			// GameGuru Shooter Genre
			g.fpgchud_s="gun";
			g.fpgchuds_s="guns";
			g.fpgcgenre=1;
		}
	}
	g.exeroot_s = GetDir();

	//  Hud Layers
	g.hudHName_s = "";
	
	//  - Global array for string variables
	Dim (  t.uservars,  1 );

	//  Hockeykid - 250210 - Dark AI added type for containers/layers
	Dim ( t.container,20);

	//  NEXTGENBRANCH ; DarkVoices
	t.nextgenbranch=1;

	//  Time stamp outside level scope reset
	g.timestampactivitymax = 0;
	g.timestampactivityflagged = 0;
	g.timestampactivityindex = 0;
	g.timestampactivitymemthen = 0;
	g.timestampactivityvideomemthen = 0;
	Dim (  t.timestampactivity_s,1000  );
	//  speed up
	g.timebasepercycle_f = 0;
	g.timebasepercyclestamp = Timer();
	g.timestampactivitymemthen = SMEMAvailable(1);

	//  FPGC - 090909 - mising media collector
	Dim (  t.missingmedia_s, 1  );
	Undim ( t.missingmedia_s );
	g.missingmediacounter = 0;

	//  FPSCV104RC9 - loading time readout to file
	g.gloadreportstate = 0;
	g.gloadreporttime = 0;
	g.gloadreportlasttime = 0;
	g.gloadreportindex = 0;
	g.loadreportarraydimmed = 0;

	//  Scene Commander - set up new animation textures
	g.animationimagestart = 666699;
	Dim (  t.animations,10  );
	for ( t.f = 1 ; t.f<= 10; t.f++ )
	{
		t.animations[t.f].img=g.animationimagestart+t.f;
	}

	//  Hockeykid - 250610 - Ai Factions
	g.FactionArrayMax = 20;
	g.mutualfactionoff = 0;

	//  Resource meter structures
	Dim (  t.resourcemeter,5  );
	Dim (  t.resourcemeter_f,5  );
	Dim (  t.resourcemeterdest_f,5  );

	//  game memory tracker (test game creates, editor uses to show in meter)
	Dim (  t.gamememtable,0  );
	Undim ( t.gamememtable );
	g.ghidememorygauge = 0;
	g.gamememactuallyused = 0;
	g.gamememactuallyusedstart = 0;
	g.gamememactualmax = (102400*(10-4));
	g.gamememactualmaxrightnow = g.gamememactualmax;
	g.gamememactualprompttime = 0;
	g.gamememactualprompt_s = "";
	g.gamememactuallyusedrt = 0;
	g.gamememresourceid = 0;
	g.mymousex = GetDisplayWidth()/2;
	g.mymousey = GetDisplayHeight()/2;

	//  Data structure to old player save data
	//t.saveplayerstate as saveplayerstatetype;
	g.hudhaveplayername = 0;
	g.localipaddress_s = "";
	g.serveripaddress_s = "";
	g.playername_s = "";
	g.soundfrequencymodifier = 0;
	//  Team Death Match - Code used by kind permission on Plystire.
	g.cap =0;
	//  V109 BETA3 - added to control change to player jump height
	g.playerdefaultjumpheight = 50;

	//  V118 - store range, aspect and fov globally!
	g.realrange_f = 9000.0f;
	g.realaspect_f = 4.0f/3.0f;
	g.realfov_f = 75.0f;
	
	//t.saveload as saveloadtype;
	Dim (  t.saveloadslot_s,9  );
	//t.saveloadgameposition as saveloadgamepositiontype;
	Dim (  t.saveloadgamepositionplayerinventory,100  );
	Dim (  t.saveloadgamepositionplayerobjective,99 );
	//Dim (  t.saveloadgamepositionentity,g.entityelementmax  ); //PE: Not used.
	Dim (  t.saveloadgamepositionweaponslot,20  );
	g.gsaveloadobjectivesloaded = 0;
	g.mefrozentype = 0;
	g.mefrozen = 0;

	Dim (  t.material,100  );
	g.gmaterialmax = 0;

	//  Debris usage on a per-game basis
	Dim (  t.debrisshapeindexused,8  );


	//  Init FPSC then leap to SETUP.INI loader
	t.leavegamedataalone=0;
	FPSC_Full_Data_Init ( );
	FPSC_Setup();

	// DBPEngine has internal loop called from inside FPSC_Setup(), so quit when get here
	ExitProcess ( 0 );
}

//Against Steam Policy!
//bool g_bOfferSteamReviewReminder = false;


bool bSkipAllGameLogic = false;
void common_loop_logic(void)
{
	if (bSkipAllGameLogic) return;

	if ( t.game.gameisexe == 0 )
		mapeditorexecutable_loop();
	else
		gameexecutable_loop();
}


void common_finish(void)
{
	if ( t.game.gameisexe == 0 )
		mapeditorexecutable_finish();
	else
		gameexecutable_finish();
}

const char *pestrcasestr(const char *arg1, const char *arg2)
{
	if (!arg1)
		return NULL;
	if (!arg2)
		return NULL;
	if (strlen(arg2) > strlen(arg1))
		return NULL;

	const char *a, *b;
	for (;*arg1;*arg1++) {

		a = arg1;
		b = arg2;

		while ((*a++ | 32) == (*b++ | 32))
			if (!*b)
				return (arg1);
	}
	return(NULL);
}

// New function to initialise all globals that were previously set up in types outside of any function/subroutine
void common_init_globals ( void )
{
	// Grab current folder
	g.fpscrootdir_s = GetDir();
	g.mydocumentsdir_s = Mydocdir();
	g.mydocumentsdir_s += "\\";
	 #ifdef VRTECH
	  #ifdef PRODUCTV3
	   g.myfpscfiles_s = "VR Quest Files";
	  #endif
	 #else
	  g.myfpscfiles_s = "Game Guru Files";
	 #endif
 	 g.myownrootdir_s = g.mydocumentsdir_s+g.myfpscfiles_s+"\\";

	//LB: 32bit for access to 32bit converter of X files
	strcpy (g_pRootFolderConverter, GetDir());
	strcat (g_pRootFolderConverter, "\\");
	strcat (g_pRootFolderConverter, "Guru-Converter.exe");

	#ifdef VRTECH
	// Store globally (for custom content loading inside SteamCheckForWorkshop)
	SetWorkshopFolder ( g.fpscrootdir_s.Get() );
	#endif

	//  Image Resources
	//  1-20 images used somewhere (terrain heightdata?)
	g.postprocessimageoffset = 21;
	//  +0 = camera zero image (central)
	//  +1 = light ray camera image
	//  +2 = sun image
	//  +3 = camera six image (left eye)
	//  +4 = camera seven image (right eye)
	//  +5 = dynamic terrain shadow camera image
	//  +6 = dynamic terrain shadow height map image
	g.titlesimageoffset = 300;
	//  +0 = backdrop image
	g.savescreenshotimage = 387;
	g.quaddefaultimage = 388;
	g.testgamesplashimage = 389;
	// `global bitmapfontimagetart=390 moved further down to make MP name labels in FRONT of grass

	g.effectmenuimagestart = 400;
	g.muzzlebankoffset = 500;
	g.imagebankoffset = 550;
	g.prompt3dimageoffset = 799;
	g.promptimageimageoffset = 800;
	g.hudlayersimageoffset = 900;
	//  +1,2,3,4 = Jetpack textures (x8)
	//  .. 32 (for all 8 jet pack textures)
	g.weaponsimageoffset = 1000;
	g.particlesimageoffset = 1400; 
	// reserve 200 particles 1400-1599
	g.ebeimageoffset = 1900;
	g.texturebankoffset = 2000;
	// ARG! Texture bank will eventually EAT into 3000 relatively quickly! Movec from 3000 to 43000
	//PE: 43000+ reserved for UIV3.
	//PE: 44000+ reserved for UIV3 preview images.
	//PE: 50000+ to be used for internal images inside dbo's.
	g.internalshadowdynamicterrain = 59950;
	g.internalshadowdebugimagestart = 59951;
	g.internalocclusiondebugimagestart = 59961;
	g.conkitimagebankoffset = 60000;
	g.luadrawredirectimageoffset = 62999;
	g.tempimageoffset = 63000;
	g.widgetimagebankoffset = 63100;
	g.huddamageimageoffset = 63200;
	//g.importerextraimageoffset = 63249;	//	For sphere in model importer.
	g.importermenuimageoffset = 63250; // was 63400
	g.importerextraimageoffset = 63300;
	g.slidersmenuimageoffset = 63500;
	g.terrainimageoffset = 63600;
	g.gamehudimagesoffset = 64700;
	g.editorimagesoffset = 65110;
	g.editordrawlastimagesoffset = 75100;
	g.interactiveimageoffset = 75110;
	g.videothumbnailsimageoffset = 84900;
	g.explosionsandfireimagesoffset = 85000;
	g.bitmapfontimagetart = 89500;
	g.panelimageoffset = 90000;
	//PE: As g.charactercreatorimageoffset substract the -t.characterkitcontrol.bankOffset , it can overwrite lower ID like g.bitmapfontimagetart. Add 2000
	//PE: g.charactercreatorimageoffset still goes as low as 90020 so they are used.
	g.charactercreatorimageoffset = 92020;
	g.charactercreatorEditorImageoffset = 95000;
	g.LUAImageoffset = 96000;
	g.LUAImageoffsetMax = 105999;
	#ifdef VRTECH
	g.perentitypromptimageoffset = 110000; // allow 10,000 slots
	#endif

	// Sprite ( Resource markers )
	g.ammopanelsprite = 63400;
	g.healthpanelsprite = 63401;
	g.steamchatpanelsprite = 63410;
	g.LUASpriteoffset = 63500;
	g.LUASpriteoffsetMax = 73499;
	g.ebeinterfacesprite = 74001;
	g.terrainpainterinterfacesprite = 74051;

	//  Mesh Resources
	g.meshgeneralwork2 = 991;
	g.meshebemarker = 992;
	g.meshebe = 993;
	g.meshebe1 = 994;
	g.meshebe2 = 995;
	g.meshsteam = 996;
	g.meshgeneralwork = 997;
	g.meshlightmapwork = 998;
	g.meshlightmapwork2 = 999;
	g.meshbankoffset = 1000;

	//  Effect Resources
	g.terraineffectoffset = 1;
	g.decaleffectoffset = 796;
	g.staticlightmapeffectoffset = 797;
	g.staticshadowlightmapeffectoffset = 798;
	g.jetpackeffectoffset = 799;
	g.quadeffectoffset = 800;
	g.postprocesseffectoffset = 901;
	g.postprocessobjectoffset0laststate = 0;
	//  +X = see postprocessimages for assignment
	g.effectbankoffset = 1000;
	g.explosionandfireeffectbankoffset = 1100;
	#ifdef VRTECH
	g.lightmappbreffectillum = 1295;
	g.controllerpbreffect = 1296;
	#else
	g.lightmappbreffectillum = 1296;
	#endif
	g.lightmappbreffect = 1297;
	g.thirdpersonentityeffect = 1298;
	g.thirdpersoncharactereffect = 1299;
	g.charactercreatoreffectbankoffset = 1300;

	//  Sound Resources
	g.soundbankoffset = 1;
	#ifdef VRTECH
	g.soundbankoffsetfinish = 8798;
	g.temppreviewsoundoffset = 8799;
	#else
	g.soundbankoffsetfinish = 8799;
	#endif
	g.titlessoundoffset = 8800;
	g.weaponssoundoffset = 8900;
	g.playercontrolsoundoffset = 9000;
	g.silentsoundoffset = 10000;
	g.materialsoundoffset = 10001;
	g.materialsoundoffsetend = 11000;
	g.explodesoundoffset = 11001;
	g.musicsoundoffset = 22000;
	g.musicsoundoffsetend = 22999;
	g.charactersoundoffset = 23000;
	g.explosionsandfiresoundoffset = 24000;
	g.projectilesoundoffset = 25000;
	g.steamsoundoffset = 30000;
	g.globalsoundoffset = 40000;

	//  Object Resources
	//  1-10 - editor objects
	g.entitybankmax = 100;
	g.ghostcursorobjectoffset = 3497;
	g.waypointdetectworkobject = 3498;
	g.entityworkobjectoffset = 3499;
	g.entityattachmentsoffset = 3500;
	g.entityattachmentindex = 0;
	#ifdef VRTECH
	g.video3dobjectoffset = 5998;
	#endif
	g.prompt3dobjectoffset = 5999;
	g.terrainobjectoffset = 6000;
	g.hudlayersbankoffset = 16000;
	//  +1 ; jetpack1
	g.hudbankoffset = 16050;
	g.gunbankoffset = 16100;
	g.brassbankoffset = 16250; // was 16150 - increased space for guns in level (more than 50)
	g.smokebankoffset = 16300;
	g.decalbankoffset = 16450;
	g.decalelementoffset = 16500;
	g.fragmentobjectoffset = 17000;
	g.explodedecalobjstart = 17500;
	g.characterkitobjectoffset = 17800;
	g.shadowdebugobjectoffset = 17890;
	g.importermenuobjectoffset = 17900;	
	g.importerextraobjectoffset = 17990;
	g.editorwaypointoffset = 18001;
	g.editorwaypointoffsetmax = 18499;
	g.debugobjectoffset = 18500;
	g.gamerealtimeobjoffset = 19300;
	g.gamerealtimeobjoffsetmax = 27999;
	g.conkitobjectbankoffset = 28000; // CONKIT REDUNDANT
	//  DON'T insert a new value in here. Conkit uses entitybankoffset as it's upper limit for object IDs
	g.entitybankoffset = 50000;
	g.temporarymeshobject = 65500;
	g.temporarydarkaiobject = 65535; // used as hard coded value in debug recastdetour!
	g.temporarydarkaiobjectend = 65634; // and reserved 100 objects for 65535 thru 65634
	g.luadrawredirectobjectoffset = 69498;
	g.projectorsphereobjectoffset = 69499;
	g.tempobjectoffset = 69500; 
	g.temp2objectoffset = 69501;
	g.instancestampworkobject = 69991;
	g.darkaiobsboxobject = 69992;
	g.tempimporterlistobject = 69993;
	g.entityviewcursorobj = 70000;
	g.entityviewstartobj = 70001;
	g.entityviewendobj = 0;
	g.entityviewcurrentobj = g.entityviewstartobj;
	//PE: 87000 is used for draw call optimizer.
	g.weaponsobjectoffset = 90000;
	g.widgetobjectoffset = 91000;
	g.lightmappedobjectoffset = 92000;
	g.lightmappedobjectoffsetfinish = 92000;
	g.lightmappedterrainoffset = -1;
	g.lightmappedterrainoffsetfinish = -1;
	g.lightmappedobjectoffsetlast = 150000;
	g.postprocessobjectoffset = 150001;
	//  +0 = post process quad
	//  +1 = light ray scatter quad
	//  +2 = light ray sun plane object quad
	//  +3 = virtual reality RIFT second eye quad
	//  +5 = dynamic terrain shadow camera image
	//  [be aware anything added after 150001 might mess up post process?!] 
	#ifdef VRTECH
	g.batchobjectoffset = 85000; //160001;
	#else
	g.batchobjectoffset = 160001;
	#endif
	g.explosionsandfireobjectoffset = 170001;
	g.raveyparticlesobjectoffset = 180001;
	g.ebeobjectbankoffset = 189901;
	g.occlusionboxobjectoffset = 190001;
	g.occlusionboxobjectoffsetfinish = 199999;
	g.steamplayermodelsoffset = 200000;
	g.charactercreatorrmodelsbankoffset = 200000;
	g.charactercreatorrmodelsoffset = 201000;
	g.charactercreatorrmodelsoffsetEnd = 203000;
	#ifdef VRTECH
	g.perentitypromptoffset = 210000; // allow 10,000 slots
	#endif


	//  Particle Resources
	g.particlebankoffset = 1;

	//  Vector Resources
	g.m4_view = 1;
	g.m4_projection = 2;
	g.m4_viewproj = 3;
	g.v4_near = 4;
	g.v4_far = 5;
	g.v3_far = 6;
	g.universalvectorindex = 10;
	g.terrainvectorindex = 11;
	g.terrainvectorindex1 = 12;
	g.terrainvectorindex2 = 13;
	g.terrainvectorindex3 = 14;
	g.vegetationvectorindex = 15;
	g.weaponvectorindex = 16;
	g.generalvectorindex = 20;
	g.widgetvectorindex = 30;
	g.widgetStartMatrix = 40;
	g.characterkitvector = 46;
	g.ragdollvectoroffset = 100;

	//  Bitmap Resources
	g.terrainworkbitmapindex = 2;

	//  Camera Resources
	//  0 - main camera
	//  1 - reserved [for possible refraction camera]
	//  2 - reflection camera
	//  3 - post process camera
	//  4 - light ray camera
	//  5 - NOT USED FROM MAR2018 dynamic terrain shadow texture cam (cheap shadow)
	//  6 - left eye camera [rift]
	//  7 - right eye camera [rift]
	//  9 - map editor
	//  21 - generating CONKIT previews
	//  29 - work bitmap(create/delete)
	//  31 - shadow cameras+
	//  32 - used sometmies
	
	//Resource Bank Arrays
	

	//  Resource Banks
	g.soundbankmax = 0;
	Dim (  t.soundbank_s,10  );
	g.imagebankmax = 0;
	Dim (  t.imagebank_s,500  );
	g.effectbankmax = 0;
	Dim (  t.effectbank_s,100  );
	g.texturebankmax = 0;
	Dim (  t.texturebank_s,100  );
	g.gunbankmax = 0;
#ifdef REDUCEMEMUSE
	g.gunbankmaxlimit = 400;
#else
	g.gunbankmaxlimit = 1000;
#endif
	Dim ( t.gunbank_s, g.gunbankmaxlimit );
	g.muzzlebankmax = 0;
	Dim (  t.muzzlebank_s,100  );
	g.brassbankmax = 0;
	Dim (  t.brassbank_s,100  );
	g.smokebankmax = 0;
	Dim (  t.smokebank_s,100  );
	g.luabankmax = 0;
	Dim (  t.luabank_s,100  );

	
	//Global General Arrays
	

	//  General purpose global arrays
	Dim (  t.filelist_s,0  );

	//  General purpose global variables
	g.setupfilename_s = "setup.ini";
	g.fpgchud_s = "gun";
	g.fpgchuds_s = "guns";
	g.fpgcgenre = 1;
	g.lowfpswarning = 0;
	g.aidetectnearbymode = 0;
	g.aidetectnearbycount = 0;
	g.gphysicssessionactive = 0;

	//t.interactive as interactivetype;
	Dim (  t.interactivesequencemaxhistory,10  );
	Dim (  t.tutorialmaps_s,24  );

	//t.promptimage as promptimagetype;
	t.promptimage.show=0;

	//t.luaText as luatexttype;

	//t.luaPanel as luapaneltype;

	//t.characterkitcontrol as characterkitcontroltype;

	Dim (  t.ccSamplePointX,3 );
	Dim (  t.ccSamplePointY,3 );
	Dim (  t.ccSampleSprite,3 );

	Dim (  t.mp_respawn_timed,MP_RESPAWN_TIME_OBJECT_LIST_SIZE  );

	Dim (  t.mp_destroyedObjectList,MP_DESTROYED_OBJECT_LIST_SIZE  );
	Dim (  t.mp_bullets,160   );
	Dim (  t.mp_bullets_send_time,160  );
	Dim (  t.mp_attachmentobjects,100   );
	Dim (  t.mp_gunobj,100   );
	Dim (  t.mp_gunname,100  );
	Dim (  t.mp_team,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_joined,MP_MAX_NUMBER_OF_PLAYERS );
	Dim (  t.mp_kills,MP_MAX_NUMBER_OF_PLAYERS   );
	Dim (  t.mp_deaths,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_lastIdleY,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_lastIdleReset,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_reload,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerShooting,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerAttachmentIndex,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerIsRagdoll,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerAttachmentObject,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerHasSpawned,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_oldAppearance,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playingAnimation,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playingRagdoll,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_oldplayerx,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_oldplayery,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_oldplayerz,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_meleePlaying,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_jetpackparticles,MP_MAX_NUMBER_OF_PLAYERS  );

	Dim (  t.mp_isDying,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_jetpackOn,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_lobbies_s,MP_MAX_NUMBER_OF_LOBBIES  );
	Dim ( t.mp_playerEntityID,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim ( t.mp_forcePosition,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_health,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_chat,MP_MAX_CHAT_LINES  );
	Dim (  t.mp_subbedItems,20  );
	Dim (  t.mp_playerAvatars_s,MP_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.mp_playerAvatarOwners_s,MP_MAX_NUMBER_OF_PLAYERS  );
	#ifdef VRTECH
	Dim (  t.mp_playerAvatarLoaded,MP_MAX_NUMBER_OF_PLAYERS  );
	#endif

	Dim (  t.mpmultiplayerstart,MP_MAX_NUMBER_OF_PLAYERS );

	//  RealSense Constants
	//-- Note; colour stream currently does nothing and should not be initialised;
	//-- Note; face recognition currently not fully implemented;
	//-- Note; USING_GESTURE is needed to enable depth image for body mass and finger tracking;
	//t.USING_GESTURE = 1 << 0;
	//t.USING_VOICE_RECOGNITION = 1 << 1;
	//t.USING_FACE_RECOGNITION = 1 << 2;
	//t.USING_BODY_MASS = 1 << 3;
	//t.USING_FINGER_TRACKING = 1 << 4;
	//t.USING_COLOUR_STREAM = 1 << 5;
	//g.FEATURES = t.USING_GESTURE | t.USING_VOICE_RECOGNITION | t.USING_FINGER_TRACKING | t.USING_BODY_MASS;
	//t.realsense as realsensetype;

	g.weaponSystem.numWeapons = WEAPON_MAXWEAPONS;
	g.weaponSystem.numProjectileBases = WEAPON_PROJECTILETYPES;
	g.weaponSystem.numProjectiles = WEAPON_MAXPROJECTILES;
	g.weaponSystem.numSounds = WEAPON_MAXSOUNDS;
	Dim (  t.Weapon , WEAPON_MAXWEAPONS  );
	Dim2(  t.WeaponAnimation , WEAPON_MAXWEAPONS ,  WEAPON_MAXANIMATIONS  );
	Dim (  t.WeaponProjectileBase,WEAPON_PROJECTILETYPES  );
	Dim (  t.WeaponProjectile,WEAPON_MAXPROJECTILES  );
	Dim (  t.WeaponSound,WEAPON_MAXSOUNDS  );

	//t.conkit as conkittype;
	t.conkit.editmodeactive=0;
	t.conkit.entityeditmode=0;
	t.conkit.objectstartnumber=g.conkitobjectbankoffset;
	t.conkit.imagestartnumber=g.conkitimagebankoffset;

	//t.lighting as lightingtype;

	//t.luaglobal as luaglobaltype;

	//t.widget as widgettype;
	t.widget.imagestart=g.widgetimagebankoffset;
	//  +1 = pos button
	//  +2 = rot button
	//  +3 = scl button
	//  +4 = prp button

	//t.characterkit as characterkitttype;
	t.characterkit.objectstart=g.characterkitobjectoffset;
	Dim (  t.characterkit_meshes_s,4  );
	Dim (  t.characterkit_diffuse_s,4 );
	Dim (  t.characterkit_normal_s,4  );
	Dim (  t.characterkit_mask_s,4 );
	//  1 - body obj
	//  2 - head obj
	//  0 - common specular
	//  1 - common occlusion
	//  2 - common illumination/blank
	//  11 - body diffuse
	//  12 - body normal
	//  13 - head diffuse
	//  14 - head normal
	g.characterkitbodymax = 0;
	g.characterkitbodyindex = 0;
	Dim (  t.characterkitbodybank_s , 1 );
	g.characterkitheadmax = 0;
	g.characterkitheadindex = 0;
	Dim (  t.characterkitheadbank_s , 1 );

	Dim (  t.importerTabs, 12  );

	Dim ( t.importerTextures, IMPORTERTEXTURESMAX  );

	Dim (  t.importerCollision,MACIMPORTERCOLLISIONSHAPES );
	Dim (  t.importerGridObject,10  );
	Dim (  t.selectedObjectMarkers,10  );

	Dim (  t.importerShaderFiles , IMPORTERSHADERFILESMAX  );
	Dim (  t.importerScriptFiles , IMPORTERSCRIPTFILESMAX  );

	g.obsmax = 10;
	g.obsindex = 0;
	Dim (  t.obs , g.obsmax );

	//t.terrainundo as terrainundotype;
	Dim2(  t.terrainundobuffer,1024, 1024  );
	Dim2(  t.terrainredobuffer,1024, 1024 );

	t.terrainundo.mode=0;

	//t.entityundo as entityundotype;
	t.entityundo.undoperformed=0;
	t.entityundo.action=0;

	Dim (  t.screenblood , 41 );

	Dim (  t.damagemarker,40  );

	//t.huddamage as huddamagetype;
	t.huddamage.indicator=g.huddamageimageoffset;
	t.huddamage.bloodstart=g.huddamageimageoffset+1;
	t.huddamage.bloodtotal=0;
	t.huddamage.immunity=0;

	g.globals.riftmode = 0;
	g.globals.occlusionmode = 0;
	g.globals.occlusionsize = 5000;
	t.aisystem.obstacleradius = 18;

	#ifdef VRTECH
	g.globals.generateassetitinerary = 0;
	#endif
	g.globals.generatehelpfromdocdoc = 0;

	t.postprocessings.fadeinvalue_f=0;

	t.game.ignoretitle=0;

	Dim2(  t.titlesbutton,20, 10  );
	g.titlesettings.graphicsettingslevel=2;
	Dim2(  t.titlesbar,20, 10  );
	g.titlessavefile_s = "settings.ini";
	
	//  Visual settings
	//g.cheapshadowhistorypacer_f = 0;

	//t.visuals as visualstype;
	//t.editorvisuals as visualstype;
	//t.gamevisuals as visualstype;

	//t.editor as editortype;
	t.editor.objectstartindex=0;
	//  objectstartindex; (1-10)
	//  +5 = Work area entity cursor
	//  +7 = cylinder to indicate resources

	g.hudlayerlistmax = 10;
	Dim (  t.hudlayerlist,g.hudlayerlistmax );

	g.ragdollvectorindex = g.ragdollvectoroffset;
	Dim (  t.ragdollvector,0  );
	g.RagdollsMax = 0;
	Dim (  t.Ragdolls,g.RagdollsMax );
	t.Ragdolls[g.RagdollsMax].obj=0;

	//  Hockeykid - 250610 - Ai Factions
	g.FactionArrayMax = 20;
	g.mutualfactionoff = 0;

	Dim (  t.material , 100 );
	g.gmaterialmax = 0;

	g.gentityprofileversion = 20100721;

	Dim2(  t.charactergunpose,100, 36  );

	g.maxslots = 10;
	g.autoswap = 1;

	Dim (  t.ammopool,100 );

	g.decalmax = 10;
	Dim (  t.decal,g.decalmax );
	t.decal[1].name_s="";
	g.decalelementmax = 499;
	Dim (  t.decalelement,g.decalelementmax );

	Dim (  t.weaponslot,10 );
	Dim (  t.weaponammo,20 );
	Dim (  t.weaponclipammo,20 );
	Dim (  t.weaponhud,10 );
	for ( t.ws = 1 ; t.ws <= 10 ; t.ws++ ) t.weaponslot[t.ws].pref=0 ;

	//  Gun Data
	g.noholster = 1;
	Dim (  t.gunslots_s, 10 );
	Dim ( t.listkey , 32 );
	for ( t.n = 0 ; t.n <= 32 ; t.n++ ) t.listkey[t.n]=0 ;
	Dim (  t.list_s, 100   );
	Dim (  t.smokeframe_f,30  );
	g.firemode = 0;
	g.bulletlimbsmax = 0;

	g.maxgunsinengine = g.gunbankmaxlimit;
	Dim ( t.gun,g.maxgunsinengine  );
	Dim2 ( g.firemodes,g.maxgunsinengine, 1 );
	Dim2 ( t.gunsound,g.maxgunsinengine, 25 );
	Dim3 ( t.gunsoundcompanion,g.maxgunsinengine, 25, 2 );
	Dim2 ( t.gunsounditem,g.maxgunsinengine, 200  );

	Dim (  t.brassfallcount_f,30 );
	g.autoloadgun = 0;
	g.gunslotmax = 0;

	#ifdef VRTECH
	Dim (  t.soundloopcheckpoint,65535 );
	#else
	Dim ( t.soundloopgamemenu, 65535 );
	#endif
	Dim ( t.soundloopcheckpoint, 65535 );

	g.playermax = 1;
	g.playertrailmax = 0;
	g.arrowkeyson = 1;
	g.jumponkey = 1;
	g.crouchonkey = 1;
	g.peekonkeys = 1;
	g.walkonkeys = 1;
	g.runkeys = 1;
	g.playeraction = 0;
	g.forcemove = 0;
	g.forcedamageon = 1;
	Dim (  t.player,g.playermax  );
	Dim2(  t.playersound,g.playermax, 520  );
	Dim2(  t.playersoundtimeused,g.playermax, 520  );
	Dim (  t.playersoundset_s,g.playermax  );
	Dim (  t.playermovementstep,g.playermax   );
	Dim2(  t.playerinventory,g.playermax, 100  );
	t.playersoundset_s[1]="";
	Dim (  t.playersoundsetindex,g.playermax  );
	g.soundsetlistmax = 0;
	Dim (  t.soundsetlist_s,g.soundsetlistmax  );
	Dim (  t.soundsetlist,g.soundsetlistmax  );
	Dim (  t.soundvolumes,2 );
	Dim (  t.playerobjective,99 );

	g.slidersprotoworkmode = 0;
	g.slidersmenumax = 0;
	Dim ( t.slidersmenu, 50 );
	Dim2( t.slidersmenuvalue, 50, 20 );
	g.sliderspecialview = 0;
	g.slidersmenufreshclick = 0;
	g.slidersmenudropdownscroll_f = 1;

	Dim (  t.nearestlightindex,4  );

	t.editorfreeflight.mode=0;

#ifdef REDUCEMEMUSE
	g.objmetamax = 300000;
#else
	g.objmetamax = 500000;
#endif
	Dim (  t.objmeta,g.objmetamax );

	Dim (  t.objinterestlist, 1   );
	Undim ( t.objinterestlist );

	//  Populate Infini Lights On The Fly In The Map Editor (see _realtimelightmapping_refreshinfiniwithe)
	Dim (  t.infinilight,0 );
	Undim ( t.infinilight );
	g.infinilightmax = 0;

	Dim2(  t.bitmapfont,10, 255 );

	Dim (  t.effectparamarray,1300 );
	Dim2( t.terrainmatrix ,terrain_chunk_size+3 , terrain_chunk_size+3 );
	t.terrain.waterlineyadjustforclip_f=0.0f;
	t.terrain.adjaboveground_f=30.0; // 070116 - caued spawned entity drop from sky 50.0f
	t.terrain.superflat=0;

	// 100417 - actually used for terrain default generation
	t.terrain.waterliney_f = 0;

	Dim2(  t.vegarea,t.terrain_vegarea_size+3, t.terrain_vegarea_size+3  );
	t.terrain.grassmemblock = 44;
	MakeMemblock (  t.terrain.grassmemblock,MAXTEXTURESIZE*MAXTEXTURESIZE );

	//  Resource start indices
	// `terrain.paintcameraindex=11

	t.terrain.objectstartindex=g.terrainobjectoffset;
	//  +0 - sphere acting as paint brush for paint camera
	//  +1 - RESERVED - NOT USED NOW [[plane to paste vegshadow to paint camera]]
	//  +2 - water plane for editor
	//  +3 - terrain object itself
	//  +4 - sky Box (  object )
	//  +5 - water plane for in-game
	//  +6 - grass parent object (and mesh index)
	//  +7 - grass mesh index for veg creation
	//  +8 - sky Box (  second object (for skybox fades) )
	//  +9 - sky Box (  skyscroll plane )
	//  +10 - sky Box (  skyscroll cloud portal image )
	//  +101/999 - unused
	//  +1000/5096 - terrain Physics Collision Meshes (LOD0=4096/LOD1=1024)
	//  +5097/6199 - unused but can be filled when terrain physics meshes reduced
	//  +6201/10000 - grass objects that cover all terrain
	//  10000 - MAX OBJECT USAGE

	t.terrain.imagestartindex = g.terrainimageoffset;
	//  +0 - water texture
	//  +1 - RESERVED - OLD [[[veg shadow RT texture]]] NOT USED NOW
	//  +2 - NEW veg shadow RT image
	//  +3 - initial height map texture
	//  +4 - water mask
	//  +5 - refraction camera image
	//  +6 - reflection camera image
	//  +7 - water normal texture
	//  +8 - grass texture
	//  +9 - skyscoll texture
	//  +13 - terrain diffuse map (4x4)
	//  +14 - R=AO, G=Gloss, B=hEight, A=Detail (AGED)
	//  +15 - Color Specular map
	//  +17 - highlighter texture
	//  +21 - terrain normal map (4x4)
	//  +24 - optional detail map for terrain
	//  +31 - PBR global env map
	//  +32 - PBR IBR curve lookup

	t.terrain.effectstartindex=g.terraineffectoffset;
	//  +0 - terrain lighting shader NON-PBR
	//  +1 - water shader
	//  +2 - vegetation shader NON-PBR
	//  +3 - terrain color shader
	//  +4 - sky shader
	//  +5 - terrain shader PBR
	//  +6 - vegetation shader PBR
	//  +9 - skyscroll shader

	//t.sky as skytype;
	g.skymax = 0;
	g.skyindex = 0;
	Dim ( t.skybank_s,1 );

	g.lutmax = 0;
	g.lutindex = 0;
	Dim(t.lutbank_s, 1);

	//  Terrain Texture Structure
	g.terrainstylemax = 0;
	g.terrainstyle_s = "";
	g.terrainstyleindex = 1;
	Dim ( t.terrainstylebank_s,g.terrainstylemax );

	//  Vegetation Texture Structure
	g.vegstylemax = 0;
	g.vegstyle_s = "";
	g.vegstyleindex = 1;
	Dim ( t.vegstylebank_s,g.vegstylemax );

	//t.gridedit as gridedittype;
	t.gridedit.autoflatten=0;
	t.gridedit.entityspraymode=0;
	t.gridedit.entitysprayrange=200;

	t.aisystem.terrainobsnum=1;
	t.aisystem.obs=2;

	//  Resource start indices
	t.aisystem.objectstartindex=1000;
	//  +0 - AI player ghost object
	//  +0001/1000 - visual character object
	//  +1001/2000 - AI entity ghost object (not currently used now)
	//  +2001/3000 - debug object to show an AI Go To
	//  +3001/4000 - debug objects to create AI entity ghost objs
	//  +3001 - debugentitymesh
	//  +3002 - debugentitymesh2
	//  +3003 - debugentityworkobj
	//  +3004 - debugentityworkobj2
	//  +3005 - debugconeofsightobj
	//  +3100 - building object
	//  +4001/5000 - visual character VWEAP object
	t.aisystem.debugentitymesh=t.aisystem.objectstartindex+3001;
	t.aisystem.debugentitymesh2=t.aisystem.objectstartindex+3002;
	t.aisystem.debugentityworkobj=t.aisystem.objectstartindex+3003;
	t.aisystem.debugentityworkobj2 = t.aisystem.objectstartindex + 3004;
	t.aisystem.debugconeofsightobj = t.aisystem.objectstartindex + 3005;	
	t.aisystem.imagestartindex=111;
	//  +0 - default character D texture (proto)
	//  +1 - default character I texture (proto)
	//  +2 - default character N texture (proto)
	//  +3 - default character S texture (proto)
	//  +4 - default building D texture (proto)
	//  +5 - default building N texture (proto)
	//  +6 - default building S texture (proto)
	//  +7 - default building S texture (proto)
	//  +11/99 - default character images (proto)
	t.aisystem.effectstartindex=4;
	//  +0 - character shader
	//  +1 - building shader
	t.aisystem.soundstartindex=1;
	//  +0 - player shot
	//  +1/1000 - character shot
	//  +1001 - player material footfalls

	g.charanimindex = 0;
	g.charanimindexmax = 0;

	//t.weapons as weaponstype;
	t.weapons.objectstartindex=3900;
	//  +0 - shot projection object for accurate Line (  of sight (hidden) )
	t.weapons.imagestartindex=3900;
	//  +0 -
	t.weapons.effectstartindex=5;
	//  +0 -
	t.weapons.soundstartindex=g.weaponssoundoffset;

	//  soundstartindex
	t.playercontrol.soundstartindex=g.playercontrolsoundoffset;
	//  0 - reserved
	//  1 - player sound ; finalmoan
	//  2 - player sound ; hurt1
	//  3 - player sound ; hurt2
	//  4 - player sound ; hurt3
	//  5 - player sound ; land
	//  6 - player sound ; leap
	//  7 - player sound ; spawn
	//  8 - player sound ; punched1
	//  9 - player sound ; punched2
	//  10 - player sound ; punched3
	//  11 - player sound ; drown
	//  12 - player sound ; gasp
	//  13 - player sound ; water in
	//  14 - player sound ; water out
	//  15 - player sound ; swim
	//  16 - player sound ; pickup ammo from corpse
	//  17 - player sound; heart beat
	//  21 - AI character die 1
	//  22 - AI character die 2
	//  23 - AI character die 3
	//  24 - AI character die 4
	//  25 - Bullet whiz 1
	//  26 - Bullet whiz 2
	//  27 - Bullet whiz 3
	//  28 - Bullet whiz 4
	//  to 99 - reserved for player sounds (will be deleted if new player soundset used)
	//  100 through to 995 - other sounds as required
	Dim (  t.musictrack,MUSICSYSTEM_MAXTRACKS );

	g.characterSoundCount = 0;
	g.characterSoundBankCount = 0;
	
	g.characterSoundStackSize = 0;
	
	g.characterSoundCurrentPlayingNumber = 0;
	g.characterSoundCurrentPlayingType_s = "";
	g.characterSoundPrevPickedNumber = 0;
	
	Dim (  t.characterSoundName,CHARACTERSOUND_MAX_BANK  );
	Dim3(  t.characterSound,CHARACTERSOUND_MAX_BANK, CHARACTERSOUND_SIZE, CHARACTERSOUND_MAX_BANK_MAX_SOUNDS+1 );
	Dim (  t.characterSoundStackEntity,CHARACTERSOUND_STACK_SIZE  );
	Dim (  t.characterSoundStackType_s,CHARACTERSOUND_STACK_SIZE  );
	
	g.camshake_f = 0.0;
	
	//  flash
	g.lightrange = 100;
	g.lightmax = 600;
	g.lightmin = 0;
	g.lightspeed = 6;
	//  Maxemit=10 emitters, with totalpart of 260 for each emitter.
	g.totalpart = 260;
	g.maxemit = 10;
	//  Max Debris
	g.debrismax = 30;

	//  make and prepare particle and debris objects
	Dim (  t.ravey_particle_emitters,RAVEY_PARTICLE_EMITTERS_MAX  );
	Dim (  t.ravey_particles,RAVEY_PARTICLES_MAX  );
	g.ravey_particles_next_particle = 0;
	g.ravey_particles_old_time = 0;
	g.ravey_particles_time_passed = 0;
}

//  Subroutine to completely construct FPSCData
void FPSC_Full_Data_Init ( void )
{
	//  performance counters
	g.deactivatecollision = 0;
	g.entitysystemdisabled = 0;
	g.lightingsystemdisabled = 0;
	g.gameperftimetracker=Timer();

	//  Water
	g.waterobj = 11;
	g.wateron = 0;
	g.oldwaterheight_f = -1.0;
	g.prevwaterheight_f = -1.0;
	g.waterheight_f = -1.0;
	g.tupdatewater = 0;
	g.waterfx = 11;
	g.waterbump_f = 0.2f;
	g.playerunderwater = 0;
	g.tnearsurfaceofwater = 100;
	g.waterred = 255;
	g.watergreen = 255;
	g.waterblue = 255;
	g.excludewatercams = 0;
	//  Scene Commander
	g.watercurrent = 0;
	g.waterflow = 1;

	//  logic control
	g.logicprioritycount = 0;
	g.logicprioritymax_f = 0.0;

	//  memory counters
	Dim (  t.mshot,500  );
	g.mshoti = 0;
	g.mshotmem = 0;
	g.mshotfirst = 0;
	g.mshotmemlargest = 0;
	g.lastmshoti = 0;
	g.lastmshotmem = 0;
	//  TDM - Plystire
	//  workload counters
	Dim2(  t.wshot,400, 4  );
	Dim (  t.wshotmax,4  );
	g.wshoti = 0;

	//  raw Text (  for HUD )
	g.grawtextr = 255;
	g.grawtextg = 0;
	g.grawtextb = 255;
	g.grawtextx = 50;
	g.grawtexty = 50;
	g.grawtextsize = 0;
	g.grawtextsizelast = 0;
	g.grawtextfont_s = "";
	g.grawtextfontlast_s = "";
	g.grawtextcount = 0;

	//Editors Data
	//  Browser Folder History
	Dim (  t.browserfolderhistory_s,10  );
	g.localdesc_s = "";

	//  Globals for FPG handling
	g.currentSMFPGtype = 1;
	g.currentSMFPG_s = "mygame.fpg";
	g.currentAMFPGtype = 2;
	g.currentAMFPG_s = "myarena.fpg";
	g.currentFPGtype = g.currentSMFPGtype;
	g.currentFPG_s = g.currentSMFPG_s;

	//  Other structres
	Dim (  t.taunt_s ,30 );

	//  Project working on 
	g.projectfilename_s = "";
	g.projectmodified = 0;
	g.projectmodifiedstatic = 0;

	//Global Data and Arrays

	//  additional globals for BUILD GAME speed-ups
	g.globalsmallsound = 0;
	g.currentlyintheAISCIPTloader = 0;

	g.particlebankmax = 0;
	g.materialsoundmax = 0;
	g.explodesoundmax = 0;
	Dim (  t.entitybank_s,g.entitybankmax  );

	//Map Data

	//  Define 50Kx50K Area (500*100)
	t.maxx=500 ; t.maxy=500;

	//  Visible-Col-Map used for per-cycle quick entity collision checks
	t.viscolx=160 ; t.viscoly=20 ; t.viscolz=160;
	Dim3(  t.viscolmap,t.viscolx, t.viscoly, t.viscolz  );

	//  Prepare entity reference map (references entityelementlist indexes)
	Dim3(t.refmap, t.layermax, t.maxx, t.maxy);


	//  Default settings
	g.gridlayershowsingle = 0;
	t.gridzoom_f=1.0;
	t.gridground=0;
	t.gridselection=1;
	t.nogridsmart=-1;
	t.gridlayer=5;
	t.bufferlayer=-1;
	t.grideditartwidth=1;
	t.grideditartwidthx=1;
	t.grideditartwidthy=1;
	t.locallibrarysegidmaster=0;
	t.locallibraryentidmaster=0;
	t.locallibraryentindex=0;
	Dim (  t.locallibraryent,t.locallibraryentindex  );

	//  resource counter to help prevent kids adding crazy amounts of stuff
	g.editorresourcecounter_f = 0;
	g.editorresourcecounterpacer = 0;

	g.animmax = 700;
	g.footfallmax = 200;
	g.entidmastermax = 100;

	//Dave fix - 100 was not enough for some stress test levels
	Dim2(  t.entityphysicsbox,MAX_ENTITY_PHYSICS_BOXES*2, MAX_ENTITY_PHYSICS_BOXES  );
	Dim2(  t.entitybodypart,100, 100  );
	Dim2(  t.entityappendanim, 100, 100 );
	Dim2(  t.entityanim,100, g.animmax );
	Dim2(  t.entityfootfall,100, g.footfallmax  );
	Dim (  t.entityprofileheader,100  );
	Dim (  t.entityprofile,100 );
	Dim2(  t.entitydecal_s,100, 100 );
	Dim2(  t.entitydecal,100, 100 );
	Dim2(  t.entityblood,100, BLOODMAX );

	g.entityelementlist = 0;
	g.entityelementmax = 100;
	Dim (  t.entityelement,g.entityelementmax  );
	Dim2(  t.entitybreadcrumbs,g.entityelementmax, 50  );

	//  New entity based shader variable array
	g.globalselectedshadermax = 4;
	g.globalselectedshadervar = 1;

	//  Segment and EntityProfile/EntityelementList Vars
	g.preidmaster = 0;
	g.segidmaster = 0;
	g.entidmaster = 0;
	g.entityelementlist = 0;
	g.aiindexmaster = 0;
	g.waypointmax = 0;
	g.wayppointoneonlyflaw = 0;
	g.gheadshotdamage = 65500;

	//  V109 BETA3 - 210408 - AI variables
	g.aivariablemode = 0;
	g.aivariableindex = 0;
	Dim (  t.aiglobals,99  );
	Dim2(  t.ailocals,1, 99   );
	Dim2(  t.aiuserlocals,1, 99  );

	//  AI Counters
	g.actstringmax = 0;
	g.conindexcount = 0;
	g.aicondseqcount = 0;
	g.actindexcount = 0;
	g.aiactseqcount = 0;
	g.hudmax = 0;
	g.hudfadeoutoneatatime = 0;
	g.internalloaderhud = 0;
	g.internaleyehud = 0;
	g.internalfaderhud = 0;

	//  AI BC Sound
	//Dim (  t.aiactionseq,10000 ); //PE: Not used.
	Dim (  t.aiaction,500 );
	//Dim (  t.actstring_s,g.actstringmax  ); //PE: Got crash here, not used anyway ?

	//  AI conditions
	//Dim (  t.aiconditionseq,10000  ); //PE: Not used.
	Dim (  t.aicond,500 );

	//  AI Library List
	Dim (  t.ailist,200 );

	//  AI Library Count
	if (  t.leavegamedataalone == 0 ) 
	{
		Dim (  t.scriptbank_s,100  );
		g.aiindexmaster=0;
	}

	Dim (  t.waypointcoord,1000 );
	Dim (  t.waypoint,10 );
	g.waypointeditheight_f = 0;
	g.waypointcoordmax = 0;
	g.waypointmax = 0;

	//  Infini lights
	Dim (  t.infinilight,0 );
	Undim(t.infinilight);
	Dim (  t.infinilightshortlist,0  );
	Undim (t.infinilightshortlist);

	//  Shadow Lights Data Structure
	Dim (  t.shadowlight,0 );

	//  Data structure for Bit-Fragments
	Dim (  t.bitdetails,10  );
	Dim2(  t.bitoffset,10, 8 );

	//  Explosion Data Structure
	g.explodermax = 4;
	Dim (  t.exploder,g.explodermax );

	//  GUI Visual Settings
	t.guivisualsettings.ambienceoverride=-1;

	//  HUD
	Dim (  t.hud,10 );
	g.saveloadgamehudmax = 0;
	Dim (  t.saveloadgamehud,10 );

	//  FPSCV104 Fog globals
	g.hudfognear = -2000.0;
	g.hudfogfar = -10000.0;

	//  Water Fog globals
	g.waterfogfar = 2000.0;
	g.waterfogred = 55;
	g.waterfoggreen = 65;
	g.waterfogblue = 75;

	//  LightRay Addition
	//  LRMod globals
	g.rotvar_f = 0.0;
	g.lrsamples = 1;
	g.lroldsamples = g.lrsamples;
	g.lrswitchsamples = 1;
	g.lrbloomactive = 1;
	g.lrswitchbloomactive = 1;
	g.lroldbloomactive = g.lrbloomactive;
	g.lrdebugdeactive = 0;

	//  World Physics Settings
	g.physicson = 1;
	g.physicsdebug = 0;
	g.physicsgravx_f = 0.0;
	g.physicsgravy_f = -40.0;
	g.physicsgravz_f = 0.0;
	g.physicsplayerweight_f = 500.0;
	g.grav_f = 0;
	g.camerapositionx = 0;
	g.camerapositiony = 0;
	g.camerapositionz = 0;
	g.cameraspeed = 0;
	g.camerapickup = 1;
	g.cameraholding = 0;
	g.camerapickupkeyrelease = 0;
	g.camerareach_f = 0;
	g.camerareachatrun_f = 0;
	g.camerareachmax_f = 75;
	g.camerapickedangle_f = 0;
	g.camerapicked = 0;
	g.camerapickede = 0;
	g.camerapickeddrop = 0;
	g.camerapickedthrown = 0;
	g.camerathrow_f = 100.0;
	g.camerathrowelev_f = 0.0;
	g.cameradampen_f = 1.0;
	g.cameradampenactive = 0;
	g.cameracarryweight_f = 4000;
	Dim (  t.phyobjvelocity_f,1  );
	Dim (  t.phylasttravelled_f,1  );
	Dim (  t.phylastfloorstop_f,1  );
	Dim (  t.phyobjsounding,1  );
	Dim (  t.phyobjremove,1  );
	Dim (  t.phyobjele,1  );
	Dim (  t.shadowobj,1  );

	//  Respawn array for arena game
	Dim (  t.respawn,16 );
	g.respawnmax = 0;

	//  Multiplayer globals and structures
	g.hudiplistmax = -1;
	Dim2(  t.hudiplist_s,20, 1  );
	g.repeatsamelevel = 0;
	g.winnersname_s = "";
	g.servername_s = "FPSC Creator Portal";
	//  talktoaster arrays
	Dim (  t.talkscript_s,10  );
	Dim (  t.talkscriptcount,10  );
	Dim (  t.talkscriptwho,10  );
	//  Characters (chosen is indexed by iLocalEL, list is flaglist of used identities)
	g.multiplayermax = 16;
	Dim (  t.characterchosen,g.multiplayermax  );
	Dim (  t.characterchoiceentityindex,g.multiplayermax  );
	Dim (  t.characterlist_s,g.multiplayermax );
	Dim (  t.characterlist,g.multiplayermax  );
	Dim (  t.characterlistentity,g.multiplayermax  );
	//  Dead reckoning temp arrays
	Dim (  t.cpx_f,4  );
	Dim (  t.cpy_f,4  );
	Dim (  t.cpz_f,4  );
	Dim (  t.stategetready,g.multiplayermax  );
	Dim (  t.statex,g.multiplayermax  );
	Dim (  t.statey,g.multiplayermax  );
	Dim (  t.statez,g.multiplayermax  );
	Dim (  t.statea,g.multiplayermax  );
	Dim (  t.stateanim,g.multiplayermax  );
	Dim (  t.stateanimdir,g.multiplayermax  );
	Dim (  t.statecolmaterialtype,g.multiplayermax  );
	Dim (  t.stateanimwait,g.multiplayermax  );
	Dim (  t.statewhodidit,g.multiplayermax  );
	Dim (  t.stateplayagain,g.multiplayermax  );
	Dim (  t.stateviewy,g.multiplayermax  );
	Dim (  t.statesviewdy,g.multiplayermax );
	Dim (  t.stateweapon,g.multiplayermax  );
	Dim (  t.stateannounce,g.multiplayermax  );
	Dim (  t.statesx,g.multiplayermax  );
	Dim (  t.statesy,g.multiplayermax  );
	Dim (  t.statesz,g.multiplayermax  );
	Dim (  t.statesa,g.multiplayermax  );
	Dim (  t.statesvel,g.multiplayermax  );
	Dim (  t.statetx,g.multiplayermax  );
	Dim (  t.statety,g.multiplayermax  );
	Dim (  t.statetz,g.multiplayermax  );
	Dim (  t.statedx,g.multiplayermax  );
	Dim (  t.statedy,g.multiplayermax  );
	Dim (  t.statedz,g.multiplayermax  );
	Dim (  t.statemove,g.multiplayermax  );
	Dim (  t.statelag,g.multiplayermax  );
	Dim (  t.statemsgap,g.multiplayermax  );
	Dim (  t.statemytimer,g.multiplayermax  );
	Dim2(  t.statecodeupdate,g.multiplayermax, 4  );
	Dim (  t.stateraycastpace,g.multiplayermax  );

	//  Server Scores
	Dim (  t.frags,g.multiplayermax );
}

void FPSC_SetDefaults ( void )
{
	//  Very first task is find and load BUILD.INI (if flagged) 
	g.gcompilestandaloneexe = 0;
	g.gpretestsavemode = 0;

	//  Find and load SETUP.INI settings as global states
	g.grealgameviewstate = 0;
	g.gmultiplayergame = 0;
	g.gexitpromptreportmodestate = 0;
	g.gdebugphysicsstate = 0;
	g.gdebugreportstepthroughstate = 0;
	g.gshowentitygameinfostate = 0;
	g.gshowdebugtextingamestate = 0;
	g.gincludeonlyvideo = 0;
	g.gincludeonlyname_s = "";
	g.gignorefastbone = 0;
	g.glightmappingstate = 0;
	g.glightmappingold = 0;
	g.glightshadowsstate = 0;
	g.glightambientr = 0;
	g.glightambientg = 0;
	g.glightambientb = 0;
	g.glightsunx = 0;
	g.glightsuny = -1;
	g.glightsunz = 0;
	g.glightsunr = 0;
	g.glightsung = 0;
	g.glightsunb = 0;
	g.glightzerorange = 2000;
	g.glightatten = 16000;
	g.glightmaxsize = -1;
	g.glightboost = 4;
	g.glighttexsize = 512;
	g.glightquality = 5;
	g.glightblurmode = 1;
	g.glightthreadmax = 4;
	g.gdynamiclightingstate = 0;
	g.gdynamicshadowsstate = 1;
	g.gdividetexturesize = 0;
	g.goptimizemode = 0;
	g.ghsrmode = 0;
	g.guseskystate = 0;
	g.gusefloorstate = 0;
	g.guseenvsoundsstate = 0;
	g.guseweaponsstate = 0;
	g.gdisplaywidth = t.ScreenW;
	g.gdisplayheight = t.ScreenH;
	g.gdisplaydepth = 32;
	g.gsetupwidth = -1;
	g.gsetupheight = -1;
	g.gsetupdepth = 32;
	g.gfullscreen = 0;
	g.gvsync = 1;
	g.gvrmode = 0;
	g.gvrmodefordevelopers = 0;
	g.gvrmodeoriginal = 0;
	g.gvrmodemag = 100;
	g.gvroffsetangx = 0;
	g.gvrwmroffsetangx = 0;
	g.gmousesensitivity = 100;
	g.guniquesignature = 0;
	g.ggameobjectivetype = 0;
	g.ggameobjectivevalue = 0;
	g.goneshotkills = 0;
	g.numberofplayers = 16;
	g.gspawnrandom = 0;
	g.guniquegamecode_s = "";
	g.guseuniquelynamedentities = 0;
	g.gexportassets = 0;
	g.gproducelogfiles = 0;
	g.gproducelogfilesdir_s = "";
	g.gpbroverride = 0;
	g.memskipwatermask = 0;
	
	g.maxtotalmeshlights = 20;
	g.maxpixelmeshlights = 10;
	g.terrainoldlight = 0;
	g.terrainusevertexlights = 1;
	g.maxterrainlights = 10;
	g.terrainlightfadedistance = 4000;
	g.showstaticlightinrealtime = 0;

	g.standalonefreememorybetweenlevels = 0;
	g.videoprecacheframes = 2;
	g.videodelayedload = 1;
	g.includeassetstore = 0;
	g.skipupdatecheck = 0;

	g.aidisabletreeobstacles = 0;
	g.aidisableobstacles = 0;
	g.skipunusedtextures = 0;

	g.lowestnearcamera = 2; //PE: default , use setup.ini lowestnearcamera to adjust.
	g.memgeneratedump = 0;
	g.underwatermode = 0;
	g.usegrassbelowwater = 1;
	g.editorsavebak = 0;
	g.gproducetruevidmemreading = 0;
	g.gcharactercapsulescale_f = 1.0;
	g.ggodmodestate = 0;
	g.glevelmax = 1;
	g.level = 1;
	g.glocalserveroverride_s = "";
	g.gbloodonfloor = 0;
	g.gimageblockmode = 0;
	g.shroudsize = 40;
	g.shroudsizedefaultsize = g.shroudsize;
	g.gxbox = 0;
	g.gxboxinvert = 0;
	g.gxboxcontrollertype = 0;
	g.gxboxmag = 1.0;
	g.gsuspendscreenprompts = 0;
	g.gforceloadtestgameshaders = 0;
	g.gshowalluniquetextures = 0;
	g.gaspectratio = 0;
	g.realaspect_f = 1.33f;
	g.gnewblossershaders = 1;
	g.gpostprocessing = 0;
	g.gpostprocessingnotransparency = 0;
	g.gfinalrendercameraid = 0;
	g.gshowaioutlines = 0;
	g.gairadius = 20;
	g.gdisablepeeking = 0;
	g.gsystemmemorycapoff = 0;
	g.gentitytogglingoff = 0;
	g.gextracollisionbuilddisabled = 1;
	g.galwaysconfirmsave = 0;
	g.gsimplifiedcharacterediting = 0;
	g.gantialias = 0;
	g.gminvert = 0;
	g.gdisablerightmousehold = 0;
	g.gparticlesnotused = 0;
	g.gautores = 0;
	g.guseoggoff = 0;
	g.gcapfpson = 0;
	g.createsplashsound = 1;
	g.ghardwareinfomode = 0;
	g.gprofileinstandalone = 0;
	g.greflectionrendersize = 512;
	g.gadapterordinal = 0;
	#ifdef VRTECH
	g.gadapterd3d11only = 0;
	#endif
	g.ghideallhuds = 0;
	g.gskipobstaclecreation = 0;
	g.gskipterrainobstaclecreation = 0;
	g.gdeletetxpcachesonexit = 0;
	g.gdisablesurfacesnap = 0;
	g.gdefaultterrainheight = GGORIGIN_Y+100;
	g.gdefaultwaterheight = GGORIGIN_Y;
	g.gdefaultebegridoffsetx = 50;
	g.gdefaultebegridoffsetz = 50;
	g.allowcpuanimations = 0;
	g.ggunmeleekey = 0;
	g.ggunaltswapkey1 = 47;
	g.ggunaltswapkey2 = 0;
	g.gzoomholdbreath = 16;
	g.fTerrainBrushSizeMax = 2000.0f;
	g.fLightmappingQuality = 1.5f;
	g.fLightmappingBlurLevel = 1.0f;
	g.iLightmappingSizeTerrain = 2048;
	g.iLightmappingSizeEntity = 512;
	g.fLightmappingSmoothAngle = 45.0f;
	g.iLightmappingExcludeTerrain = 0;
	g.iLightmappingDeactivateDirectionalLight = 0;
	g.fLightmappingAmbientR = 0.25f;
	g.fLightmappingAmbientG = 0.25f;
	g.fLightmappingAmbientB = 0.25f;
	g.iLightmappingAllTerrainLighting = 0;

	//Dynamic res
	t.DisableDynamicRes = false;
}

void FPSC_LoadSETUPINI ( bool bUseMySystemFolder )
{
	//  SETUP Info
	if ( bUseMySystemFolder == true ) 
	{
		// this means we are using the new Windows 10 style write folder in separate location
		cstr mysystemfolder_s = "My System";
		t.tfile_s = g.myownrootdir_s + "\\" + mysystemfolder_s + "\\" + g.setupfilename_s;
		if ( FileExist ( t.tfile_s.Get() ) == 0 )
		{
			// we need to copy the original from the read-only location to the new write location before using it
			CopyFileA ( g.setupfilename_s.Get(), t.tfile_s.Get(), FALSE );
		}
	}
	else
	{
		// can use the local relative location of SETUP.INI (such as a Steam IDE or standalone game)
		t.tfile_s = g.setupfilename_s;
	}

	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		//  Load Data from file
		Dim (  t.data_s,999  );
		LoadArray (  t.tfile_s.Get(),t.data_s );
		for ( t.l = 0 ; t.l<=  999; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if (  cstr(Lower(Left(t.line_s.Get(),4))) == ";end"  )  break;
				if (  cstr(Left(t.line_s.Get(),1)) != ";" ) 
				{
					// take fieldname and values
					for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
					{
						if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1; break; }
					}
					t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
					t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));
					for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
					{
						if ( t.value_s.Get()[t.c] == ',' ) { t.mid = t.c+1; break; }
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value2_s=removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid));
					if (  Len(t.value2_s.Get())>0  )  t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;

					// All SETUP.INI Fields:

					#ifdef VRTECH
					// DOCDOC: generatehelpfromdocdoc = Set to 1 to generate new assetitinerary file every time, 2 to prevent CUSTOM_* being added to FPM (dev)
					t.tryfield_s = "generateassetitinerary" ; if (  t.field_s == t.tryfield_s  )  g.globals.generateassetitinerary = t.value1;			
					#endif

					// DOCDOC: generatehelpfromdocdoc = Enable GameGuru to generate new DOCDOC Help (when source relatively available).
					t.tryfield_s = "generatehelpfromdocdoc" ; if (  t.field_s == t.tryfield_s  )  g.globals.generatehelpfromdocdoc = t.value1;

					// DOCDOC: superflatterrain = Set to 1 will force a simplified terrain geometry that is completely flat
					t.tryfield_s = "superflatterrain" ; if (  t.field_s == t.tryfield_s  )  t.terrain.superflat = t.value1;

					// DOCDOC: riftmode = Discontinued
					t.tryfield_s = "riftmode" ; if (  t.field_s == t.tryfield_s  )  g.globals.riftmode = t.value1;

					// DOCDOC: smoothcamerakeys = Add a smoothing function to the position and angle of the main camera
					t.tryfield_s = "smoothcamerakeys" ; if (  t.field_s == t.tryfield_s  )  g.globals.smoothcamerakeys = t.value1;

					// DOCDOC: memorydetector = Activates extra memory usage and monitoring code
					t.tryfield_s = "memorydetector" ; if (  t.field_s == t.tryfield_s  )  g.globals.memorydetector = t.value1;

					// DOCDOC: occlusionmode = Enables the use of the occlusion system to skip rendering of hidden entities
					//t.tryfield_s = "occlusionmode"; if (t.field_s == t.tryfield_s)  g.globals.occlusionmode = t.value1;
					g.globals.occlusionmode = 0; //LB: 32 bit work saw that this relied on CPU rendering using 32 bit ASM - no longer supported for now

					// DOCDOC: occlusionsize = Sets the size of the margins around occluders to occlude less of the scene
					t.tryfield_s = "occlusionsize" ; if (  t.field_s == t.tryfield_s  )  g.globals.occlusionsize = t.value1;

					// DOCDOC: obstacleradius = Sets the size of the radius around AI entities for wall avoidance. Default is 18.
					t.tryfield_s = "obstacleradius" ; if (  t.field_s == t.tryfield_s  )  t.aisystem.obstacleradius = t.value1;

					// DOCDOC: showdebugcollisonboxes = Renders the collision boxes associated with physics collision created by model importer
					t.tryfield_s = "showdebugcollisonboxes" ; if (  t.field_s == t.tryfield_s  ) g.globals.showdebugcollisonboxes = t.value1;

					// DOCDOC: hideebe = Hide the Builder menu from the main IDE
					t.tryfield_s = "hideebe" ; if (  t.field_s == t.tryfield_s  ) g.globals.hideebe = t.value1;

					// DOCDOC: hidedistantshadows = Causes more distant shadows to be hidden to improve performance
					t.tryfield_s = "hidedistantshadows" ; if (  t.field_s == t.tryfield_s  ) g.globals.hidedistantshadows = t.value1;

					#ifdef VRTECH
					#else
					// DOCDOC: terrainshadows = Enables terrain to cast shadow maps at a cost of performance
					t.tryfield_s = "terrainshadows" ; if (  t.field_s == t.tryfield_s  ) g.globals.terrainshadows = t.value1;
					#endif

					// DOCDOC: realshadowresolution = Size of the texture plate dimension to render the shadow onto. Default is 2048.
					t.tryfield_s = "realshadowresolution" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowresolution = t.value1;
          
					t.tryfield_s = "speedshadows"; if (t.field_s == t.tryfield_s) g.globals.speedshadows = t.value1;
					t.tryfield_s = "drawcalloptimizer"; if (t.field_s == t.tryfield_s) g.globals.drawcalloptimizer = t.value1;
					t.tryfield_s = "forcenowaterreflection"; if (t.field_s == t.tryfield_s) g.globals.forcenowaterreflection = t.value1;
					
					t.tryfield_s = "flashlightshadows"; if (t.field_s == t.tryfield_s) 
					{
						g.globals.flashlightshadows = t.value1;
						if (g.globals.flashlightshadows > 1) g.globals.flashlightshadows = 1;
						if (g.globals.flashlightshadows < 0) g.globals.flashlightshadows = 0;
					}

					// DOCDOC: realshadowcascadecount = Set the number of shadow cascades to use. Default is 4, Min is 2 and Max is 8.
					t.tryfield_s = "realshadowcascadecount" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascadecount = t.value1;
					if (g.globals.realshadowcascadecount < 2) g.globals.realshadowcascadecount = 2; //PE: Limit cascades.
					if (g.globals.realshadowcascadecount > 8) g.globals.realshadowcascadecount = 8; //PE: Limit cascades.
					if (g.globals.flashlightshadows == 1) 
					{
						if (g.globals.realshadowcascadecount > 7) g.globals.realshadowcascadecount = 7; //PE: Limit cascades.
					}

					#ifdef VRTECH
					// DOCDOC: speedshadows = Sets the internal shadow rendering technique. Default is 2.
					t.tryfield_s = "speedshadows"; if (t.field_s == t.tryfield_s) g.globals.speedshadows = t.value1;

					// DOCDOC: drawcalloptimizer = Set to 1 to activate the automatic batching of entities to improve performance
					t.tryfield_s = "drawcalloptimizer"; if (t.field_s == t.tryfield_s) g.globals.drawcalloptimizer = t.value1;

					// DOCDOC: forcenowaterreflection = Set to 1 to switch off water reflection internally for improved performance
					t.tryfield_s = "forcenowaterreflection"; if (t.field_s == t.tryfield_s) g.globals.forcenowaterreflection = t.value1;					

					// DOCDOC: flashlightshadows = Set to 1 to activate an additional shadow cast from the flashlight (press F to activate flashlight)
					t.tryfield_s = "flashlightshadows"; if (t.field_s == t.tryfield_s) 
					{
						g.globals.flashlightshadows = t.value1;
						if (g.globals.flashlightshadows > 1) g.globals.flashlightshadows = 1;
						if (g.globals.flashlightshadows < 0) g.globals.flashlightshadows = 0;
					}
					#endif

					// DOCDOC: realshadowcascade0 thru realshadowcascade7 = Set the distance as a percentage when cascade kicks in
					t.tryfield_s = "realshadowcascade0" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[0] = t.value1;
					t.tryfield_s = "realshadowcascade1" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[1] = t.value1;
					t.tryfield_s = "realshadowcascade2" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[2] = t.value1;
					t.tryfield_s = "realshadowcascade3" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[3] = t.value1;
					t.tryfield_s = "realshadowcascade4" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[4] = t.value1;
					t.tryfield_s = "realshadowcascade5" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[5] = t.value1;
					t.tryfield_s = "realshadowcascade6" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[6] = t.value1;
					t.tryfield_s = "realshadowcascade7" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[7] = t.value1;

					// DOCDOC: realshadowsize0 thru realshadowsize7 = Not Used
					t.tryfield_s = "realshadowsize0"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[0] = t.value1;
					t.tryfield_s = "realshadowsize1"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[1] = t.value1;
					t.tryfield_s = "realshadowsize2"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[2] = t.value1;
					t.tryfield_s = "realshadowsize3"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[3] = t.value1;
					t.tryfield_s = "realshadowsize4"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[4] = t.value1;
					t.tryfield_s = "realshadowsize5"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[5] = t.value1;
					t.tryfield_s = "realshadowsize6"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[6] = t.value1;
					t.tryfield_s = "realshadowsize7"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[7] = t.value1;

					// DOCDOC: realshadowdistance = Sets the camera depth distance of the shadow render. Default is 5000.
					t.tryfield_s = "realshadowdistance"; if (t.field_s == t.tryfield_s) 
					{
						g.globals.realshadowdistance = t.value1;
						g.globals.realshadowdistancehigh = t.value1;
					}

					// DOCDOC: editorusemediumshadows = Sets the editor to render medium level shadows while editing
					t.tryfield_s = "editorusemediumshadows"; if (t.field_s == t.tryfield_s)  g.globals.editorusemediumshadows = t.value1;

					// DOCDOC: hidememorygauge = Hides the memory gauge that displays in the top left of the editor area
					t.tryfield_s = "hidememorygauge" ; if (  t.field_s == t.tryfield_s  )  g.ghidememorygauge = t.value1;

					// DOCDOC: hidelowfpswarning = Prevents the 'low FPS warning' prompt when playing the game
					t.tryfield_s = "hidelowfpswarning" ; if (  t.field_s == t.tryfield_s  )  g.globals.hidelowfpswarning = t.value1;

					// DOCDOC: hardwareinfomode = Enables extra information to be displayed when F11 is pressed in game
					t.tryfield_s = "hardwareinfomode" ; if (  t.field_s == t.tryfield_s  )  g.ghardwareinfomode = t.value1;

					// DOCDOC: profileinstandalone = Enables the debug options to remain in a standalone game
					t.tryfield_s = "profileinstandalone" ; if (  t.field_s == t.tryfield_s  )  g.gprofileinstandalone = t.value1;

					// DOCDOC: allowfragmentation = Set to 0 to force game to relaunch at end of game, 1 to never relaunch the game and 2 to relaunch after every level. Default is 1.
					t.tryfield_s = "allowfragmentation" ; if (  t.field_s == t.tryfield_s  )  t.game.allowfragmentation = t.value1;

					#ifdef VRTECH
					#else
					// DOCDOC: standalonefreememorybetweenlevels = Enables the deletion of textures from the previous level before loading the next one.
					t.tryfield_s = "standalonefreememorybetweenlevels"; if (t.field_s == t.tryfield_s)  g.standalonefreememorybetweenlevels = t.value1;
					#endif

					// DOCDOC: reflectionrendersize = Sets the size of the texture plate dimension for rendering the reflections in water. Default is 512.
					t.tryfield_s = "reflectionrendersize" ; if (  t.field_s == t.tryfield_s  )  g.greflectionrendersize = t.value1;

					// DOCDOC: ignoretitlepage = Forces the title page to be skipped in standalone games
					t.tryfield_s = "ignoretitlepage" ; if (  t.field_s == t.tryfield_s  )  t.game.ignoretitle = t.value1;

					// DOCDOC: deactivateconkit = Not Used
					t.tryfield_s = "deactivateconkit" ; if (  t.field_s == t.tryfield_s  )  g.globals.deactivateconkit = t.value1;

					// DOCDOC: disablefreeflight = Disables the ability to use the free flight mode in the editor
					t.tryfield_s = "disablefreeflight" ; if (  t.field_s == t.tryfield_s  )  g.globals.disablefreeflight = t.value1;

					// DOCDOC: enableplrspeedmods = Enable the ability of weapons to affect the total running speed of the player
					t.tryfield_s = "enableplrspeedmods" ; if (  t.field_s == t.tryfield_s  )  g.globals.enableplrspeedmods = t.value1;

					// DOCDOC: disableweaponjams = Disables the capability of weapons to 'jam' while being repeatedly fired
					t.tryfield_s = "disableweaponjams" ; if (  t.field_s == t.tryfield_s  )  g.globals.disableweaponjams = t.value1;

					// DOCDOC: adapterordinal = Force the choice of DirectX Adapter to use. Set 1-98 to choose an adapter at that index, 99 to prefer the first non-Intel adapter from the list. Default is 0.
					t.tryfield_s = "adapterordinal" ; if (  t.field_s == t.tryfield_s  )  g.gadapterordinal = t.value1;

					// DOCDOC: hideallhuds = Forces all display HUDs to hide when in the game
					t.tryfield_s = "hideallhuds" ; if (  t.field_s == t.tryfield_s  )  g.ghideallhuds = t.value1;

					#ifdef VRTECH
					// DOCDOC: adapterd3d11only = Set to 1 to change the feature levels requested when DirectX is initialised
					t.tryfield_s = "adapterd3d11only" ; if (  t.field_s == t.tryfield_s  )  g.gadapterd3d11only = t.value1;
					#endif

					// DOCDOC: skipobstaclecreation = Speed up level preparation time by skipping AI obstacle creation. AI will not have pathfinding. Default is 0.
					t.tryfield_s = "skipobstaclecreation" ; if (  t.field_s == t.tryfield_s  )  g.gskipobstaclecreation = t.value1;

					// DOCDOC: skipterrainobstaclecreation = Skips the creation of AI obstacles related to the terrain. Default is 0.
					t.tryfield_s = "skipterrainobstaclecreation" ; if (  t.field_s == t.tryfield_s  )  g.gskipterrainobstaclecreation = t.value1;

					// DOCDOC: vsync = Enables the refresh of the render to match the current adapter refresh rate.
					t.tryfield_s = "vsync" ; if (  t.field_s == t.tryfield_s  )  g.gvsync = t.value1;

					// DOCDOC: fullscreen = Attempts to request a full-screen mode from the DirectX adapter.
					t.tryfield_s = "fullscreen" ; if (  t.field_s == t.tryfield_s  )  g.gfullscreen = t.value1;

					// DOCDOC: width = Not Used
					t.tryfield_s = "width" ; if (  t.field_s == t.tryfield_s ) g.gdisplaywidth = t.value1 ; t.newwidth = t.value1 ; g.gsetupwidth = t.value1;

					// DOCDOC: height - Not Used
					t.tryfield_s = "height" ; if (  t.field_s == t.tryfield_s ) g.gdisplayheight = t.value1 ; t.newheight = t.value1 ; g.gsetupheight = t.value1;

					// DOCDOC: depth = Not Used
					t.tryfield_s = "depth" ; if (  t.field_s == t.tryfield_s ) g.gdisplaydepth = t.value1 ; t.newdepth = t.value1 ; g.gsetupdepth = t.value1;

					// DOCDOC: aspectratio = Not Used
					t.tryfield_s = "aspectratio" ; if (  t.field_s == t.tryfield_s ) g.gaspectratio = t.value1 ; t.newaspectratio = t.value1;

					// DOCDOC: realgameview = Not Used
					t.tryfield_s = "realgameview" ; if (  t.field_s == t.tryfield_s  )  g.grealgameviewstate = t.value1;

					// DOCDOC: multiplayergame = Not Used
					t.tryfield_s = "multiplayergame" ; if (  t.field_s == t.tryfield_s  )  g.gmultiplayergame = t.value1;

					// DOCDOC: exitpromptreport = Not Used
					t.tryfield_s = "exitpromptreport" ; if (  t.field_s == t.tryfield_s  )  g.gexitpromptreportmodestate = t.value1;

					// DOCDOC: debugphysics = Not Used
					t.tryfield_s = "debugphysics" ; if (  t.field_s == t.tryfield_s  )  g.gdebugphysicsstate = t.value1;

					// DOCDOC: debugreportstepthrough = Not Used
					t.tryfield_s = "debugreportstepthrough" ; if (  t.field_s == t.tryfield_s  )  g.gdebugreportstepthroughstate = t.value1;

					// DOCDOC: showentitygameinfo = Displays extra information over the entity in test game
					t.tryfield_s = "showentitygameinfo" ; if (  t.field_s == t.tryfield_s  )  g.gshowentitygameinfostate = t.value1;

					// DOCDOC: showdebugtextingame = Not Used
					t.tryfield_s = "showdebugtextingame" ; if (  t.field_s == t.tryfield_s  )  g.gshowdebugtextingamestate = t.value1;

					// DOCDOC: includeonlyvideo = Enables image loading filter to only load files specified by includeonlyname
					t.tryfield_s = "includeonlyvideo" ; if (  t.field_s == t.tryfield_s  )  g.gincludeonlyvideo = t.value1;

					// DOCDOC: includeonlyname = Sets the name that the includeonlyvideo mode uses to filter all image loading
					t.tryfield_s = "includeonlyname" ; if (  t.field_s == t.tryfield_s  )  g.gincludeonlyname_s = t.value_s;

					// DOCDOC: ignorefastbone = Not Used
					t.tryfield_s = "ignorefastbone" ; if (  t.field_s == t.tryfield_s  )  g.gignorefastbone = t.value1;

					// DOCDOC: loadreport = Not Used
					t.tryfield_s = "loadreport" ; if (  t.field_s == t.tryfield_s  )  g.gloadreportstate = t.value1;

					// DOCDOC: usingmysystemfolder = [BETA] Moves all temporary files to a user writable location (for Safe Mode Compatibility)
					t.tryfield_s = "usingmysystemfolder" ; if (  t.field_s == t.tryfield_s  )  g.mysystem.bUsingMySystemFolder = t.value1;

					// DOCDOC: optimizemode = Not Used
					t.tryfield_s = "optimizemode" ; if (  t.field_s == t.tryfield_s  )  g.goptimizemode = t.value1;

					// DOCDOC: lightmapping = Not Used 
					t.tryfield_s = "lightmapping" ; if (  t.field_s == t.tryfield_s  )  g.glightmappingstate = t.value1;

					// DOCDOC: lightmapsize = Not Used
					t.tryfield_s = "lightmapsize" ; if (  t.field_s == t.tryfield_s  )  t.glightmapsize = t.value1;

					// DOCDOC: lightmapquality = Not Used
					t.tryfield_s = "lightmapquality" ; if (  t.field_s == t.tryfield_s  )  t.glightmapquality = t.value1;

					// DOCDOC: lightmapold = Not Used
					t.tryfield_s = "lightmapold" ; if (  t.field_s == t.tryfield_s  )  g.glightmappingold = t.value1;

					// DOCDOC: lightmapshadows = Not Used
					t.tryfield_s = "lightmapshadows" ; if (  t.field_s == t.tryfield_s  )  g.glightshadowsstate = t.value1;

					// DOCDOC: lightmapambientr = Not Used
					t.tryfield_s = "lightmapambientr" ; if (  t.field_s == t.tryfield_s  )  g.glightambientr = t.value1;

					// DOCDOC: lightmapambientg = Not Used
					t.tryfield_s = "lightmapambientg" ; if (  t.field_s == t.tryfield_s  )  g.glightambientg = t.value1;

					// DOCDOC: lightmapambientb = Not Used
					t.tryfield_s = "lightmapambientb" ; if (  t.field_s == t.tryfield_s  )  g.glightambientb = t.value1;

					// DOCDOC: lightmapsunx = Not Used
					t.tryfield_s = "lightmapsunx" ; if (  t.field_s == t.tryfield_s  )  g.glightsunx = t.value1;

					// DOCDOC: lightmapsuny = Not Used
					t.tryfield_s = "lightmapsuny" ; if (  t.field_s == t.tryfield_s  )  g.glightsuny = t.value1;

					// DOCDOC: lightmapsunz = Not Used
					t.tryfield_s = "lightmapsunz" ; if (  t.field_s == t.tryfield_s  )  g.glightsunz = t.value1;

					// DOCDOC: lightmapsunr = Not Used
					t.tryfield_s = "lightmapsunr" ; if (  t.field_s == t.tryfield_s  )  g.glightsunr = t.value1;

					// DOCDOC: lightmapsung = Not Used
					t.tryfield_s = "lightmapsung" ; if (  t.field_s == t.tryfield_s  )  g.glightsung = t.value1;

					// DOCDOC: lightmapsunb = Not Used
					t.tryfield_s = "lightmapsunb" ; if (  t.field_s == t.tryfield_s  )  g.glightsunb = t.value1;

					// DOCDOC: lightmapzerorange = Not Used
					t.tryfield_s = "lightmapzerorange" ; if (  t.field_s == t.tryfield_s  )  g.glightzerorange = t.value1;

					// DOCDOC: lightmapatten = Not Used
					t.tryfield_s = "lightmapatten" ; if (  t.field_s == t.tryfield_s  )  g.glightatten = t.value1;

					// DOCDOC: lightmapmaxsize = Not Used
					t.tryfield_s = "lightmapmaxsize" ; if (  t.field_s == t.tryfield_s  )  g.glightmaxsize = t.value1;

					// DOCDOC: lightmapboost = Not Used
					t.tryfield_s = "lightmapboost" ; if (  t.field_s == t.tryfield_s  )  g.glightboost = t.value1;

					// DOCDOC: lightmaptexsize = Not Used
					t.tryfield_s = "lightmaptexsize" ; if (  t.field_s == t.tryfield_s  )  g.glighttexsize = t.value1;

					// DOCDOC: lightmapquality = Not Used
					t.tryfield_s = "lightmapquality" ; if (  t.field_s == t.tryfield_s  )  g.glightquality = t.value1;

					// DOCDOC: lightmapblurmode = Not Used
					t.tryfield_s = "lightmapblurmode" ; if (  t.field_s == t.tryfield_s  )  g.glightblurmode = t.value1;

					// DOCDOC: lightmapthreadmax = Not Used
					t.tryfield_s = "lightmapthreadmax" ; if (  t.field_s == t.tryfield_s  )  g.glightthreadmax = t.value1;

					// DOCDOC: bloodonfloor = Not Used
					t.tryfield_s = "bloodonfloor" ; if (  t.field_s == t.tryfield_s  )  g.gbloodonfloor = t.value1;

					// DOCDOC: imageblockmode = Not Used
					t.tryfield_s = "imageblockmode" ; if (  t.field_s == t.tryfield_s  )  g.gimageblockmode = t.value1;

					// DOCDOC: showalluniquetextures = Not Used
					t.tryfield_s = "showalluniquetextures" ; if (  t.field_s == t.tryfield_s  )  g.gshowalluniquetextures = t.value1;

					// DOCDOC: systemmemorycapoff = Not Used
					t.tryfield_s = "systemmemorycapoff" ; if (  t.field_s == t.tryfield_s  )  g.gsystemmemorycapoff = t.value1;

					// DOCDOC: entitytogglingoff = Disables ability to use the Y key to toggle entity between static and dynamic
					t.tryfield_s = "entitytogglingoff" ; if (  t.field_s == t.tryfield_s  )  g.gentitytogglingoff = t.value1;

					// DOCDOC: extracollisionbuilddisabled = Not Used
					t.tryfield_s = "extracollisionbuilddisabled" ; if (  t.field_s == t.tryfield_s  )  g.gextracollisionbuilddisabled = t.value1;

					// DOCDOC: alwaysconfirmsave = Asks user if they wish to save level before exiting editor
					t.tryfield_s = "alwaysconfirmsave" ; if (  t.field_s == t.tryfield_s  )  g.galwaysconfirmsave = t.value1;

					// DOCDOC: simplifiedcharacterediting = Remove violent-related properties from character entities
					t.tryfield_s = "simplifiedcharacterediting" ; if (  t.field_s == t.tryfield_s  )  g.gsimplifiedcharacterediting = t.value1;

					// DOCDOC: useoggoff = Not Used
					t.tryfield_s = "useoggoff" ; if (  t.field_s == t.tryfield_s  )  g.guseoggoff = t.value1;

					// DOCDOC: cullmode = Not Used
					t.tryfield_s = "cullmode" ; if (  t.field_s == t.tryfield_s  )  g.cullmode = t.value1;

					// DOCDOC: capfpson = Not Used
					t.tryfield_s = "capfpson" ; if (  t.field_s == t.tryfield_s  )  g.gcapfpson = t.value1;

					// DOCDOC: disabledynamicres = Not Used
					t.tryfield_s = "disabledynamicres"; 
					if (  t.field_s == t.tryfield_s  ) 
					{
						if ( t.value1 == 1 )
							t.DisableDynamicRes = true;
						else
							t.DisableDynamicRes = false;
					}

					// DOCDOC: deletetxpcachesonexit = Enables temporary file deletion when creating custom textures for Builder and Terrain
					t.tryfield_s = "deletetxpcachesonexit" ; if (  t.field_s == t.tryfield_s  )  g.gdeletetxpcachesonexit = t.value1;

					// DOCDOC: disablesurfacesnap = Disables the editor ability for some entities to locate surfaces to snap to
					t.tryfield_s = "disablesurfacesnap" ; if (  t.field_s == t.tryfield_s  )  g.gdisablesurfacesnap = t.value1;

					// DOCDOC: defaultterrainheight = Sets the height at which a flat terrain is created by default. Default is 600.
					t.tryfield_s = "defaultterrainheight" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultterrainheight = t.value1;

					// DOCDOC: defaultwaterheight = Sets the height of the built-in water plane. Default is 500.
					t.tryfield_s = "defaultwaterheight" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultwaterheight = t.value1;

					// DOCDOC: defaultebegridoffsetx = Sets the grid X offset applied to Builder entities when placing them. Default is 50.
					t.tryfield_s = "defaultebegridoffsetx" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultebegridoffsetx = t.value1;

					// DOCDOC: defaultebegridoffsetz = Sets the grid Z offset applied to Builder entities when placing them. Default is 50.
					t.tryfield_s = "defaultebegridoffsetz" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultebegridoffsetz = t.value1;

					// DOCDOC: xbox = Sets whether the XBOX Style Controller should be detected and used to control the player
					t.tryfield_s = "xbox" ; if (  t.field_s == t.tryfield_s  )  g.gxbox = t.value1;

					// DOCDOC: xboxinvert = Inverts the Y axis of the mouselook stick of an XBOX Style Controller
					t.tryfield_s = "xboxinvert" ; if (  t.field_s == t.tryfield_s  )  g.gxboxinvert = t.value1;

					// DOCDOC: xboxcontrollertype = Sets the type of XBOX Style Controller used by the game, 1 is the old XBOX controller, 2 is the new XBOX Controller, 3 is the Dual Action F310 Controller.
					t.tryfield_s = "xboxcontrollertype" ; if (  t.field_s == t.tryfield_s  )  g.gxboxcontrollertype = t.value1;

					// DOCDOC: xboxmag = Amplifies the sensitivity of the input values coming from the XBOX Style Controller. Default is 100.
					t.tryfield_s = "xboxmag" ; if (  t.field_s == t.tryfield_s  )  g.gxboxmag = (0.0+t.value1)/100.0;

					// DOCDOC: mousesensitivity = Not Used
					t.tryfield_s = "mousesensitivity" ; if (  t.field_s == t.tryfield_s ) g.gmousesensitivity = t.value1  ; t.newmousesensitivity = t.value1;

					// VRMode
					// 0 : off
					// 1 : VR920/iWear
					#ifdef VRTECH
					// 2 : GGVR (OpenVR)
					// 3 : GGVR (Microsoft WMR)
					// 4 : RESERVED - HOLDING VALUE (see code)
					#endif
					// 5 : detects VR920/iWear (switches OFF if not found)
					// 6 : special case, side by side rendering
					#ifdef VRTECH
					t.tryfield_s = "vrmode" ; 
					if (  t.field_s == t.tryfield_s  )  
					{ 
						g.gvrmode = t.value1; 
						g.gvrmodeoriginal = t.value1; 
						if ( g.gvrmode != 0 )
						{
							#ifndef GURULIGHTMAPPER
							HWND hThisWnd = g_pGlob->hWnd;
							#endif
						}
					}
					t.tryfield_s = "vrmodefordevelopers"; if (t.field_s == t.tryfield_s)  g.gvrmodefordevelopers = t.value1;
					t.tryfield_s = "vrmodemag" ; if (  t.field_s == t.tryfield_s  )  g.gvrmodemag = t.value1;
					t.tryfield_s = "vroffsetangx" ; if (  t.field_s == t.tryfield_s  )  g.gvroffsetangx = t.value1;
					t.tryfield_s = "vrwmroffsetangx" ; if (  t.field_s == t.tryfield_s  )  g.gvrwmroffsetangx = t.value1;
					#endif

					// DOCDOC: dynamiclighting = Not Used
					t.tryfield_s = "dynamiclighting" ; if (  t.field_s == t.tryfield_s  )  g.gdynamiclightingstate = t.value1;

					// DOCDOC: dynamicshadows = Not Used
					t.tryfield_s = "dynamicshadows" ; if (  t.field_s == t.tryfield_s ) g.gdynamicshadowsstate = t.value1  ; t.newdynamicshadows = t.value1;

					// DOCDOC: dividetexturesize = Divides the size of the loaded textures by this value. Default is 0 for no division.
					t.tryfield_s = "dividetexturesize" ; if (  t.field_s == t.tryfield_s ) g.gdividetexturesize = t.value1  ; t.newdividetexturesize = t.value1;

					// DOCDOC: producelogfiles = Sets whether the editor and game produces .LOG files which time stamp and track events within the engine
					t.tryfield_s = "producelogfiles" ; if (  t.field_s == t.tryfield_s  )  g.gproducelogfiles = t.value1;

					// DOCDOC: producelogfilesdir = Set a new folder for where the .LOG files will be saved out. Default is the root folder.
					t.tryfield_s = "producelogfilesdir" ; if (  t.field_s == t.tryfield_s  )  g.gproducelogfilesdir_s = t.value_s;

					// DOCDOC: pbroverride = Activates the PBR rendering system. Set to 0 to disable PBR and revert to the DNS texture system. Default is 1.
					t.tryfield_s = "pbroverride" ; if (  t.field_s == t.tryfield_s  )  g.gpbroverride = t.value1;

					// DOCDOC: underwatermode = Activates the advanced underwater rendering and activity mode.
					t.tryfield_s = "underwatermode"; if (t.field_s == t.tryfield_s)  g.underwatermode = t.value1;

					// DOCDOC: usegrassbelowwater = Renders grass below the water line.
					t.tryfield_s = "usegrassbelowwater"; if (t.field_s == t.tryfield_s)  g.usegrassbelowwater = t.value1;

					// DOCDOC: memskipwatermask = Disables the generation of a water mask which reduces the alpha of water as it reaches the shoreline.
					t.tryfield_s = "memskipwatermask"; if (t.field_s == t.tryfield_s)  g.memskipwatermask = t.value1;

					#ifdef VRTECH
					// DOCDOC: standalonefreememorybetweenlevels = Enables the deletion of textures from the previous level before loading the next one.
					t.tryfield_s = "standalonefreememorybetweenlevels"; if (t.field_s == t.tryfield_s)  g.standalonefreememorybetweenlevels = t.value1;
					#endif

					// DOCDOC: videoprecacheframes = Set the amount of pre-caching each video in a game level uses, lowering memory usage. Default is 1.
					t.tryfield_s = "videoprecacheframes"; if (t.field_s == t.tryfield_s)  g.videoprecacheframes = t.value1;

					// DOCDOC: videodelayedload = Delays the loading of videos until they are needed in game, saving memory.
					t.tryfield_s = "videodelayedload"; if (t.field_s == t.tryfield_s)  g.videodelayedload = t.value1;

					// DOCDOC: includeassetstore = Activates the option to download store items direct into the software
					t.tryfield_s = "includeassetstore"; if (t.field_s == t.tryfield_s)  g.includeassetstore = t.value1;		

					// DOCDOC: includeassetstore = Disables check for new updates when this is set to one
					t.tryfield_s = "skipupdatecheck"; if (t.field_s == t.tryfield_s)  g.skipupdatecheck = t.value1;		
					
					// DOCDOC: aidisabletreeobstacles = Disable all AI obstacles for trees, improving level preparation time.
					t.tryfield_s = "aidisabletreeobstacles"; if (t.field_s == t.tryfield_s)  g.aidisabletreeobstacles = t.value1;

					// DOCDOC: aidisableobstacles = Disables all AI obstacles for all entities, improving level preparation time.
					t.tryfield_s = "aidisableobstacles"; if (t.field_s == t.tryfield_s)  g.aidisableobstacles = t.value1;

					// DOCDOC: skipunusedtextures = Skips loading of detail and height textures which are not overly used by most assets.
					t.tryfield_s = "skipunusedtextures"; if (t.field_s == t.tryfield_s)  g.skipunusedtextures = t.value1;

					// DOCDOC: lowestnearcamera = Reduce Z flicker issues by increasing this value. Recommended range for this is 8-14. Default is 1.
					t.tryfield_s = "lowestnearcamera"; if (t.field_s == t.tryfield_s)  g.lowestnearcamera = t.value1;

					// DOCDOC: editorsavebak = Enables the editor to save a .BAK file for .FPM level files
					t.tryfield_s = "editorsavebak"; if (t.field_s == t.tryfield_s)  g.editorsavebak = t.value1;

					// DOCDOC: terrainoldlight = Set to 1 to use the old terrain lighting system. Default is 0 for more than 3 lights on terrain.
					t.tryfield_s = "terrainoldlight"; if (t.field_s == t.tryfield_s)  g.terrainoldlight = t.value1;

					// DOCDOC: terrainusevertexlights = Set terrain to use vertex lighting instead of per pixel lighting for improved performance.
					t.tryfield_s = "terrainusevertexlights"; if (t.field_s == t.tryfield_s)  g.terrainusevertexlights = t.value1;

					// DOCDOC: showstaticlightinrealtime = Renders any static lights in the real-time scene in addition to existing dynamic lights.
					t.tryfield_s = "showstaticlightinrealtime"; if (t.field_s == t.tryfield_s)  g.showstaticlightinrealtime = t.value1;

					// DOCDOC: maxtotalmeshlights = Set the maximum number of lights to be used in the scene. Range is 4-38. Default is 38.
					t.tryfield_s = "maxtotalmeshlights"; 
					if (t.field_s == t.tryfield_s)  
					{
						g.maxtotalmeshlights = t.value1;
						#ifdef VRTECH
						if (g.maxtotalmeshlights > 38 ) g.maxtotalmeshlights = 38;
						#else
						if (g.maxtotalmeshlights > 79) g.maxtotalmeshlights = 79;
						#endif
						if (g.maxtotalmeshlights < 4) g.maxtotalmeshlights = 4; //PE: Lowest to support old system on terrain
					}

					// DOCDOC: maxpixelmeshlights = Set the maximum number of per pixel lights to be used in the scene. Range is 0-38. Default is 12.
					t.tryfield_s = "maxpixelmeshlights"; 
					if (t.field_s == t.tryfield_s)  
					{
						g.maxpixelmeshlights = t.value1;
						#ifdef VRTECH
						if (g.maxpixelmeshlights > 38) g.maxpixelmeshlights = 38; //PE: Leave 2 vertex based lights per mesh.
						#else
						if (g.maxpixelmeshlights > 80) g.maxpixelmeshlights = 80; //PE: Leave 2 vertex based lights per mesh.
						#endif
					}

					// DOCDOC: maxterrainlights = Set the maximum number of terrain lights to be used in the scene. Range is 0-40. Default is 20.
					t.tryfield_s = "maxterrainlights"; 
					if (t.field_s == t.tryfield_s)  
					{
						g.maxterrainlights = t.value1;
						if (g.maxterrainlights > 40) g.maxterrainlights = 40;
					}

					// DOCDOC: terrainlightfadedistance = Sets the distance at which terrain lights will fade out. Min is 600. Default is 4500.
					t.tryfield_s = "terrainlightfadedistance"; 
					if (t.field_s == t.tryfield_s)  
					{
						g.terrainlightfadedistance = t.value1;
						if (g.terrainlightfadedistance < 600 ) g.terrainlightfadedistance = 600; //PE: Need atleast a 600 fade distance.
					}

					// DOCDOC: memskipibr = Set to skip the loading of the IBR file, used to pre-process lighting values for PBR rendering.
					t.tryfield_s = "memskipibr"; if (t.field_s == t.tryfield_s)  g.memskipibr = t.value1;

					// DOCDOC: memgeneratedump = Enable the dumping of a list of images loaded after each level.
					t.tryfield_s = "memgeneratedump"; if (t.field_s == t.tryfield_s)  g.memgeneratedump = t.value1;

					// DOCDOC: producetruevidmemreading = Adds better video memory usage stats to the .LOG file when produced
					t.tryfield_s = "producetruevidmemreading" ; if (  t.field_s == t.tryfield_s  )  g.gproducetruevidmemreading = t.value1;

					// DOCDOC: charactercapsulescale = Sets a global scaling percentage to any character physics capsules created. Default is 100.
					t.tryfield_s = "charactercapsulescale" ; if (  t.field_s == t.tryfield_s  )  g.gcharactercapsulescale_f = (t.value1+0.0)/100.0;

					// DOCDOC: hsrmode = Not Used
					t.tryfield_s = "hsrmode" ; if (  t.field_s == t.tryfield_s  )  g.ghsrmode = t.value1;

					// DOCDOC: newblossershaders = Not Used
					t.tryfield_s = "newblossershaders" ; if (  t.field_s == t.tryfield_s  )  g.gnewblossershaders = t.value1;

					// DOCDOC: postprocessing = Enables the use of post processing when rendering the game
					t.tryfield_s = "postprocessing" ; if (  t.field_s == t.tryfield_s ) g.gpostprocessing = t.value1  ; t.newpostprocessing = t.value1;

					// DOCDOC: showaioutlines = Not Used
					t.tryfield_s = "showaioutlines" ; if (  t.field_s == t.tryfield_s  )  g.gshowaioutlines = t.value1;

					// DOCDOC: airadius = Sets the global radius for all AI bots in the game, within which they will not collide with each other.
					t.tryfield_s = "airadius" ; if (  t.field_s == t.tryfield_s  )  g.gairadius = t.value1;

					// DOCDOC: disablepeeking = Not Used
					t.tryfield_s = "disablepeeking" ; if (  t.field_s == t.tryfield_s  )  g.gdisablepeeking = t.value1;

					// DOCDOC: antialias = Not Used
					t.tryfield_s = "antialias" ; if (  t.field_s == t.tryfield_s ) g.gantialias = t.value1  ; t.newantialias = t.value1;

					// DOCDOC: invmouse = Inverts the Y axis of the mouse input data.
					t.tryfield_s = "invmouse" ; if (  t.field_s == t.tryfield_s ) g.gminvert = t.value1  ; t.newmouseinvert = t.value1;

					// DOCDOC: disablerightmousehold = Not Used
					t.tryfield_s = "disablerightmousehold" ; if (  t.field_s == t.tryfield_s  )  g.gdisablerightmousehold = t.value1;

					// DOCDOC: disableparticles = Not Used
					t.tryfield_s = "disableparticles" ; if (  t.field_s == t.tryfield_s ) g.gparticlesnotused = t.value1  ; t.newparticlesused = t.value1;

					// DOCDOC: autores = Not Used
					t.tryfield_s = "autores" ; if (  t.field_s == t.tryfield_s ) g.gautores = t.value1  ; t.newautores = t.value1;

					// DOCDOC: terrainbrushsizemax = Sets the maximum size the terrain brush is allowed to go. Default is 2000.
					t.tryfield_s = "terrainbrushsizemax"; if ( t.field_s == t.tryfield_s ) g.fTerrainBrushSizeMax = t.value1;

					// DOCDOC: allowcpuanimations = Enables the ability for entities to specify CPU bone animations instead of GPU animations.
					t.tryfield_s = "allowcpuanimations"; if ( t.field_s == t.tryfield_s ) g.allowcpuanimations = t.value1;

					// DOCDOC: lightmappingquality = Sets the lightmapping quality level. Default is 500.
					t.tryfield_s = "lightmappingquality"; if ( t.field_s == t.tryfield_s ) g.fLightmappingQuality = t.value1/100.0f;

					// DOCDOC: lightmappingblurlevel = Sets the amount of blurring applied to the final lightmap. Default is 100.
					t.tryfield_s = "lightmappingblurlevel"; if ( t.field_s == t.tryfield_s ) g.fLightmappingBlurLevel = t.value1/100.0f;

					// DOCDOC: lightmappingsizeterrain = Sets the size of the texture plate used to store the terrain lightmap. Default is 2048.
					t.tryfield_s = "lightmappingsizeterrain"; if ( t.field_s == t.tryfield_s ) g.iLightmappingSizeTerrain = t.value1;

					// DOCDOC: lightmappingsizeentity = Sets the size of the texture plate used to store the entities lightmaps. Default is 1024.
					t.tryfield_s = "lightmappingsizeentity"; if ( t.field_s == t.tryfield_s ) g.iLightmappingSizeEntity = t.value1;

					// DOCDOC: lightmappingsmoothangle = Sets the angle within which smoothing will be applied to the edge. Default is 45.
					t.tryfield_s = "lightmappingsmoothangle"; if ( t.field_s == t.tryfield_s ) g.fLightmappingSmoothAngle = t.value1;

					// DOCDOC: lightmappingexcludeterrain = Set this to skip all terrain lightmapping. Default is 0.
					t.tryfield_s = "lightmappingexcludeterrain"; if ( t.field_s == t.tryfield_s ) g.iLightmappingExcludeTerrain = t.value1;

					// DOCDOC: lightmappingdeactivatedirectionallight = Disable any directional lighting from the sun within the lightmapping process. Default is 0.
					t.tryfield_s = "lightmappingdeactivatedirectionallight"; if ( t.field_s == t.tryfield_s ) g.iLightmappingDeactivateDirectionalLight = t.value1;

					// DOCDOC: lightmappingambientred = Sets the ambient Red color percentage to be applied during the lightmapping process. Default is dark grey, 25.
					t.tryfield_s = "lightmappingambientred"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientR = t.value1/100.0f;

					// DOCDOC: lightmappingambientgreen = Sets the ambient Green color percentage to be applied during the lightmapping process. Default is dark grey, 25.
					t.tryfield_s = "lightmappingambientgreen"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientG = t.value1/100.0f;

					// DOCDOC: lightmappingambientblue = Sets the ambient Blue color percentage to be applied during the lightmapping process. Default is dark grey, 25.
					t.tryfield_s = "lightmappingambientblue"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientB = t.value1/100.0f;

					// DOCDOC: lightmappingallterrainlighting = If no directional lightmapping, set this to force lightmap all the terrain area. Default is 0.
					t.tryfield_s = "lightmappingallterrainlighting"; if ( t.field_s == t.tryfield_s ) g.iLightmappingAllTerrainLighting = t.value1;

					// DOCDOC: suspendscreenprompts = Prevent screen prompts from being rendered to the screen. Default is 0.
					t.tryfield_s = "suspendscreenprompts" ; if (  t.field_s == t.tryfield_s  )  g.gsuspendscreenprompts = t.value1;

					// DOCDOC: forceloadtestgameshaders = Set to 1 to generate new .BLOB files for all loaded shaders, 2 to force all shaders to have new .BLOB files. Default is 0.
					// 0 - off by default
					// 1 - generate new .BLOB files when a shader is loaded
					// 2 - scan effectbank folder and generate ALL NEW .BLOB files
					t.tryfield_s = "forceloadtestgameshaders" ; if (  t.field_s == t.tryfield_s  )  g.gforceloadtestgameshaders = t.value1;				

					// DOCDOC: reloadweapongunspecs = Forces a reload of the gun data file in case of buying and using weapons through the store. Default is 0.
					t.tryfield_s = "reloadweapongunspecs"; if (t.field_s == t.tryfield_s)  g.reloadWeaponGunspecs = t.value1;

					// DOCDOC: usesky = Not Used
					t.tryfield_s = "usesky" ; if (  t.field_s == t.tryfield_s  )  g.guseskystate = t.value1;

					// DOCDOC: usefloor = Not Used
					t.tryfield_s = "usefloor" ; if (  t.field_s == t.tryfield_s  )  g.gusefloorstate = t.value1;

					// DOCDOC: useenvsounds = Not Used
					t.tryfield_s = "useenvsounds" ; if (  t.field_s == t.tryfield_s  )  g.guseenvsoundsstate = t.value1;

					// DOCDOC: useweapons = Not Used
					t.tryfield_s = "useweapons" ; if (  t.field_s == t.tryfield_s  )  g.guseweaponsstate = t.value1;

					// DOCDOC: godmode = Enables the use of God Mode, which increases player health to 99999 when the 'I' key is pressed in game.
					t.tryfield_s = "godmode" ; if (  t.field_s == t.tryfield_s  )  g.ggodmodestate = 0;

					// DOCDOC: uniquesignature = Not Used
					t.tryfield_s = "uniquesignature" ; if (  t.field_s == t.tryfield_s  )  g.guniquesignature = t.value1;

					// DOCDOC: gameobjectivetype = Not Used
					t.tryfield_s = "gameobjectivetype" ; if (  t.field_s == t.tryfield_s  )  g.ggameobjectivetype = t.value1;

					// DOCDOC: gameobjectivevalue = Not Used
					t.tryfield_s = "gameobjectivevalue" ; if (  t.field_s == t.tryfield_s  )  g.ggameobjectivevalue = t.value1;

					// DOCDOC: oneshotkills = Not Used
					t.tryfield_s = "oneshotkills" ; if (  t.field_s == t.tryfield_s  )  g.goneshotkills = t.value1;

					// DOCDOC: maxplayers = Not Used
					t.tryfield_s = "maxplayers" ; if (  t.field_s == t.tryfield_s  )  g.numberofplayers = t.value1;

					// DOCDOC: spawnrandom = Not Used
					t.tryfield_s = "spawnrandom" ; if (  t.field_s == t.tryfield_s  )  g.gspawnrandom = t.value1;

					// DOCDOC: uniquegamecode = Not Used
					t.tryfield_s = "uniquegamecode" ; if (  t.field_s == t.tryfield_s  )  g.guniquegamecode_s = t.value_s;

					// DOCDOC: useuniquelynamedentities = Set to 1 so editor will assign unique names to added entities. Default is 0.
					t.tryfield_s = "useuniquelynamedentities" ; if (  t.field_s == t.tryfield_s  )  g.guseuniquelynamedentities = t.value1;

					// DOCDOC: exportassets = Enables the ability for save standalone to include the FPE along with the entities other resources.
					t.tryfield_s = "exportassets" ; if (  t.field_s == t.tryfield_s  )  g.gexportassets = t.value1;

					// DOCDOC: localserver = Not Used
					t.tryfield_s = "localserver" ; if (  t.field_s == t.tryfield_s  )  g.glocalserveroverride_s = t.value_s;

					// DOCDOC: title = Not Used
					t.tryfield_s = "title" ; if (  t.field_s == t.tryfield_s  )  t.titlefpi_s = t.value_s;

					// DOCDOC: global = Not Used
					t.tryfield_s = "global" ; if (  t.field_s == t.tryfield_s  )  t.setupfpi_s = t.value_s;

					// DOCDOC: gamewon = Not Used
					t.tryfield_s = "gamewon" ; if (  t.field_s == t.tryfield_s  )  t.gamewonfpi_s = t.value_s;

					// DOCDOC: gameover = Not Used
					t.tryfield_s = "gameover" ; if (  t.field_s == t.tryfield_s  )  t.gameoverfpi_s = t.value_s;

					// DOCDOC: levelfpi1 = Not Used
					t.tryfield_s = "levelfpi1" ; if (  t.field_s == t.tryfield_s ) t.loadingfpi_s == t.value_s ; t.levelfpiinsetup = t.l;

					// DOCDOC: hudr = Not Used
					t.tryfield_s = "hudr" ; if (  t.field_s == t.tryfield_s  )  g.r_f = t.value1;

					// DOCDOC: hudg = Not Used
					t.tryfield_s = "hudg" ; if (  t.field_s == t.tryfield_s  )  g.g_f = t.value1;

					// DOCDOC: hudb = Not Used
					t.tryfield_s = "hudb" ; if (  t.field_s == t.tryfield_s  )  g.b_f = t.value1;

					// DOCDOC: autoswaptrue = Not Used
					t.tryfield_s = "autoswaptrue" ; if (  t.field_s == t.tryfield_s  )  g.autoswap = t.value1;

					// DOCDOC: messagetime = Not Used
					t.tryfield_s = "messagetime" ; if (  t.field_s == t.tryfield_s  )  g.messagetime = t.value1;

					// DOCDOC: allowscope = Not Used
					t.tryfield_s = "allowscope" ; if (  t.field_s == t.tryfield_s  )  g.allowscope_s = t.value1;

					// DOCDOC: serverhostname = Not Used
					t.tryfield_s = "serverhostname" ; if (  t.field_s == t.tryfield_s  )  g.serverhostname = t.value_s;

					// DOCDOC: alwaysrun = Not Used
					t.tryfield_s = "alwaysrun" ; if (  t.field_s == t.tryfield_s  )  g.alwaysrun = t.value1;

					// DOCDOC: matchtype = Not Used
					t.tryfield_s = "matchtype" ; if (  t.field_s == t.tryfield_s  )  g.multi_match_type = t.value1;

					// DOCDOC: multiradar = Not Used
					t.tryfield_s = "multiradar";if (  t.field_s == t.tryfield_s  )  g.darkradar = t.value1;

					// DOCDOC: multicompass = Not Used
					t.tryfield_s = "multicompass";;if (  t.field_s == t.tryfield_s  )  g.compassOn = t.value1;

					// DOCDOC: multicompassx = Not Used
					t.tryfield_s = "multicompassx";if (  t.field_s == t.tryfield_s  )  g.compassX = t.value1;

					// DOCDOC: multicompassy = Not Used
					t.tryfield_s = "multicompassy";if (  t.field_s == t.tryfield_s  )  g.compassY = t.value1;

					// DOCDOC: multiradarx = Not Used
					t.tryfield_s = "multiradarx";if (  t.field_s == t.tryfield_s  )  g.radarx = t.value1;

					// DOCDOC: multiradary = Not Used
					t.tryfield_s = "multiradary";if (  t.field_s == t.tryfield_s  )  g.radary = t.value1;

					// DOCDOC: levelmax = Not Used
					if ( t.field_s == "levelmax"  )  g.glevelmax = t.value1;
					if ( g.glevelmax>0 ) 
					{
						for ( t.v = 1 ; t.v<=  g.glevelmax; t.v++ )
						{
							// DOCDOC: levelfpm = Not Used
							sprintf ( t.szwork , "levelfpm%s" , Str(t.v) );
							t.tryfield_s = t.szwork;
							if (  t.field_s == t.tryfield_s  )  t.levelfpm_s = t.value_s;

							// DOCDOC: levelfpi = Not used
							sprintf ( t.szwork , "levelfpi%s" , Str(t.v) );
							t.tryfield_s=t.szwork;
							if (  t.field_s == t.tryfield_s ) 
							{
								t.levelfpi_s=t.value_s;
								Dim ( t.level_s,t.v );
								t.level_s[t.v].fpm_s=t.levelfpm_s;
								t.level_s[t.v].fpi_s=t.levelfpi_s;
							}
						}
					}

					// DOCDOC: melee key = Not Used
					if ( t.field_s == "melee key"  )  g.ggunmeleekey = t.value1;

					// DOCDOC: switchtoalt = Not Used
					if ( t.field_s == "switchtoalt" ) 
					{
						if ( t.value1 != 0 ) g.ggunaltswapkey1 = t.value1;
						g.ggunaltswapkey2 = t.value2;
						if (  t.value2 == 0  )  g.ggunaltswapkey2 = -1;
					}

					// DOCDOC: zoomholdbreath = The keymap value to be used to hold breath while zooming. Default is 16 (letter Q).
					if ( t.field_s == "zoomholdbreath"  )  g.gzoomholdbreath = t.value1;

					// DOCDOC: key1 thru key11 = Old style mapping of control keys to be used in the game, now depreciated.
					for ( t.num = 1 ; t.num<=  11; t.num++ )
					{
						sprintf ( t.szwork , "key%s" , Str(t.num) );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.listkey[t.num] = t.value1;
					}

					for ( t.num = 1 ; t.num<=  11; t.num++ )
					{
						// DOCDOC: keyup = Assigns a new keymap value to represent the indicated control action. Default is 17.
						if (  t.num == 1  )  t.tryfield_s = "keyup";
						// DOCDOC: keydown = Assigns a new keymap value to represent the indicated control action. Default is 31.
						if (  t.num == 2  )  t.tryfield_s = "keydown";
						// DOCDOC: keyleft = Assigns a new keymap value to represent the indicated control action. Default is 30.
						if (  t.num == 3  )  t.tryfield_s = "keyleft";
						// DOCDOC: keyright = Assigns a new keymap value to represent the indicated control action. Default is 32.
						if (  t.num == 4  )  t.tryfield_s = "keyright";
						// DOCDOC: keyjump = Assigns a new keymap value to represent the indicated control action. Default is 57.
						if (  t.num == 5  )  t.tryfield_s = "keyjump";
						// DOCDOC: keycrouch = Assigns a new keymap value to represent the indicated control action. Default is 46.
						if (  t.num == 6  )  t.tryfield_s = "keycrouch";
						// DOCDOC: keyenter = Assigns a new keymap value to represent the indicated control action. Default is 28.
						if (  t.num == 7  )  t.tryfield_s = "keyenter";
						// DOCDOC: keyreload = Assigns a new keymap value to represent the indicated control action. Default is 19.
						if (  t.num == 8  )  t.tryfield_s = "keyreload";
						// DOCDOC: keypeekleft = Assigns a new keymap value to represent the indicated control action. Default is 16.
						if (  t.num == 9  )  t.tryfield_s = "keypeekleft";
						// DOCDOC: keypeekright = Assigns a new keymap value to represent the indicated control action. Default is 18.
						if (  t.num == 10  )  t.tryfield_s = "keypeekright";
						// DOCDOC: keyrun = Assigns a new keymap value to represent the indicated control action. Default is 42.
						if (  t.num == 11  )  t.tryfield_s = "keyrun";
						if (  t.field_s == t.tryfield_s  )  t.listkey[t.num] = t.value1;
					}

					// DOCDOC: slot1 thru slot9 = Pre-assign weapon ID values to the nine available gun slots in the game
					for ( t.num = 1 ; t.num<=  9; t.num++ )
					{
						sprintf ( t.szwork , "slot%i" , t.num );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.gunslots_s[t.num] = t.value_s;
					}

					// DOCDOC: taunt1 thru taunt30 = Not used
					for ( t.num = 1 ; t.num<=  30; t.num++ )
					{
						sprintf ( t.szwork , "taunt%i" , t.num );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.taunt_s[t.num] = t.value_s;
					}

					// DOCDOC: language = Sets the language folder inside 'languagebank' to use for the game, defaults to 'English'.
					t.tryfield_s = "language" ; if (  t.field_s == t.tryfield_s  )  g.language_s = t.value_s;

					// DOCDOC: graphicslowterrain = Pre-assign the terrain shader level to use when the in-game menu selects LOW for graphics.
					t.tryfield_s = "graphicslowterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowterrain_s = t.value_s;

					// DOCDOC: graphicslowentity = Pre-assign the entity shader level to use when the in-game menu selects LOW for graphics.
					t.tryfield_s = "graphicslowentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowentity_s = t.value_s;

					// DOCDOC: graphicslowgrass = Pre-assign the grass shader level to use when the in-game menu selects LOW for graphics.
					t.tryfield_s = "graphicslowgrass" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowgrass_s = t.value_s;

					// DOCDOC: graphicsmediumterrain = Pre-assign the terrain shader level to use when the in-game menu selects MEDIUM for graphics.
					t.tryfield_s = "graphicsmediumterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumterrain_s = t.value_s;

					// DOCDOC: graphicsmediumentity = Pre-assign the entity shader level to use when the in-game menu selects MEDIUM for graphics.
					t.tryfield_s = "graphicsmediumentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumentity_s = t.value_s;

					// DOCDOC: graphicsmediumgrass = Pre-assign the grass shader level to use when the in-game menu selects MEDIUM for graphics.
					t.tryfield_s = "graphicsmediumgrass" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumgrass_s = t.value_s;

					// DOCDOC: graphicshighterrain = Pre-assign the terrain shader level to use when the in-game menu selects HIGH for graphics.
					t.tryfield_s = "graphicshighterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicshighterrain_s = t.value_s;

					// DOCDOC: graphicshighentity = Pre-assign the entity shader level to use when the in-game menu selects HIGH for graphics.
					t.tryfield_s = "graphicshighentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicshighentity_s = t.value_s;

					// DOCDOC: graphicshighgrass = Pre-assign the grass shader level to use when the in-game menu selects HIGH for graphics.
					t.tryfield_s = "graphicshighgrass" ; if (  t.field_s == t.tryfield_s  )  g.graphicshighgrass_s = t.value_s;
				}
			}
		}
		UnDim (  t.data_s );

		//  V118 - 160810 - knxrb - Auto Resolution
		if (  g.gautores == 1 ) 
		{
			g.gdisplaywidth = GetDesktopWidth();
			g.gdisplayheight = GetDesktopHeight();
		}
	}
	else
	{
		//  No SETUP.INI, default is a standalone game
		g.grealgameviewstate=1;
		g.gdynamiclightingstate=1;
		g.guseskystate=1;
		g.gusefloorstate=0;
		g.guseenvsoundsstate=1;
		g.guseweaponsstate=1;
	}

	// send shadowmap details to engine
	int rs0 = g.globals.realshadowsize[0];
	int rs1 = g.globals.realshadowsize[1];
	int rs2 = g.globals.realshadowsize[2];
	int rs3 = g.globals.realshadowsize[3];
	int rs4 = g.globals.realshadowsize[4];
	int rs5 = g.globals.realshadowsize[5];
	int rs6 = g.globals.realshadowsize[6];
	int rs7 = g.globals.realshadowsize[7];
	float dist = g.globals.realshadowdistance;
	int flash = g.globals.flashlightshadows;
	int speed = g.globals.speedshadows;
	InitShadowMapDetails(rs0, rs1, rs2, rs3, rs4, rs5, rs6, rs7, dist, flash, speed);

	// special global flag which can affect how shaders are loaded
	if ( g.gforceloadtestgameshaders != 0 ) gbAlwaysIgnoreShaderBlobFile = true;

	#ifdef VRTECH
	// new feature which scans entitybank folders and confirms there are no missing model or texture files
	/* useful - we can make this a feature
	LPSTR pOriginalDir = GetDir();
	cstr pReport = "";
	bool bFixAnyEntityIssues = true;
	if ( bFixAnyEntityIssues == true )
	{
		g.filecollectionmax = 0;
		Undim ( t.filecollection_s );
		Dim ( t.filecollection_s, 500 );
		SetDir ( "Files\\entitybank" );
		addallinfoldertocollection("Objects - Copy","");
	}
	bool bRunScanOnEntityFiles = true;
	if ( bRunScanOnEntityFiles == true )
	{
		// first scan main folders
		SetDir ( pOriginalDir );
		SetDir ( "Files\\entitybank\\Objects" );
		LPSTR pEntityBankDir = GetDir();
		std::vector<cstr> folderlist;
		folderlist.clear();
		ChecklistForFiles();
		for ( int c = 1; c <= ChecklistQuantity(); c++ )
		{
			if ( ChecklistValueA(c) == 1 )
			{
				LPSTR pFilename = ChecklistString(c);
				if ( strcmp ( pFilename, "." ) != NULL && strcmp ( pFilename, ".." ) != NULL )
				{
					folderlist.push_back ( pFilename );
				}
			}
		}
		if ( folderlist.size() > 0 )
		{
			for ( int f = 0; f < folderlist.size(); f++ )
			{
				LPSTR pFolderName = folderlist[f].Get();
				SetDir ( pEntityBankDir );
				SetDir ( pFolderName );
				ChecklistForFiles();
				for ( int c = 1; c <= ChecklistQuantity(); c++ )
				{
					LPSTR pFilename = ChecklistString(c);
					if ( strcmp ( pFilename + strlen(pFilename) - 4, ".fpe" ) == NULL )
					{
						// found entity file
						pReport = cstr("VALIDATE : ") + pFolderName + "\\" + pFilename + " : ";

						// find model and texture
						LPSTR pModelFile = NULL;
						LPSTR pTextureFile = NULL;
						std::vector<cstr> entityFileData;
						Dim ( entityFileData, 999 );
						LoadArray ( pFilename, entityFileData );
						for ( int l = 0; l <= 999; l++ )
						{
							// get this line
							cstr line_s = entityFileData[l];
							LPSTR pLinePtr = line_s.Get();
							if ( Len(pLinePtr) > 0 ) 
							{
								// found model filename
								if ( strnicmp (pLinePtr, "model", 5 ) == NULL )
								{
									pLinePtr = strstr ( pLinePtr, "model" );
									if ( pLinePtr )
									{
										pLinePtr = strstr ( pLinePtr+5, "=" );
										if ( pLinePtr )
										{
											pLinePtr = pLinePtr + 1;
											while ( *pLinePtr == 32 ) pLinePtr++;
											pModelFile = new char[1024];
											strcpy ( pModelFile, pLinePtr );
										}
									}
								}

								// found textured filename
								if ( strnicmp (pLinePtr, "textured", 8 ) == NULL )
								{
									pLinePtr = strstr ( pLinePtr, "textured" );
									if ( pLinePtr )
									{
										pLinePtr = strstr ( pLinePtr+8, "=" );
										if ( pLinePtr )
										{
											pLinePtr = pLinePtr + 1;
											while ( *pLinePtr == 32 ) pLinePtr++;
											pTextureFile = new char[1024];
											strcpy ( pTextureFile, pLinePtr );
										}
									}
								}
							}

							// can quit when we have both model filename and textured filename
							if ( pModelFile && pTextureFile ) break;
						}

						// check if all files available
						bool bReportIt = false;
						bool bModelOkay = false;
						bool bTextureOkay = false;
						if ( pModelFile && FileExist ( pModelFile ) == 1 ) bModelOkay = true;
						if ( pTextureFile )
						{
							char pStoreOrigTextureFilename[1024];
							strcpy ( pStoreOrigTextureFilename, pTextureFile );
							if ( FileExist ( pTextureFile ) == 0 ) 
							{
								pTextureFile[strlen(pTextureFile)-4] = 0;
								strcat ( pTextureFile, ".dds" );
							}
							if ( FileExist ( pTextureFile ) == 0 ) 
							{
								pTextureFile[strlen(pTextureFile)-4] = 0;
								strcat ( pTextureFile, ".png" );
							}
							if ( FileExist ( pTextureFile ) == 0 ) 
							{
								pTextureFile[strlen(pTextureFile)-4] = 0;
								strcat ( pTextureFile, ".jpg" );
							}
							if ( FileExist ( pTextureFile ) == 0 ) 
								strcpy ( pTextureFile, pStoreOrigTextureFilename );
							else
								bTextureOkay = true;
						}
						if ( bModelOkay == false || bTextureOkay == false )
						{
							if ( pModelFile == NULL ) 
							{
								pReport = pReport + "MODEL NAME NOT GIVEN ";
								bReportIt = true;
							}
							else
							{
								if ( bModelOkay == false )
								{
									if ( bFixAnyEntityIssues == true )
									{
										LPSTR pFindMissingFile = FindFileFromEntityBank(pModelFile);
										if ( pFindMissingFile )
										{
											CopyFile ( pFindMissingFile, pModelFile, TRUE );
											if ( FileExist ( pModelFile ) == 1 )
											{
												// only if copied okay can remove from source
												//DeleteFile ( pFindMissingFile );
											}
											pReport = pReport + "MODEL FILE COPIED OVER " + pModelFile + " ";
											delete pFindMissingFile;
											bReportIt = true;
										}
										else
										{
											pReport = pReport + "MODEL FILE UNKNOWN " + pModelFile + " ";
											bReportIt = true;
										}
									}
									else
									{
										pReport = pReport + "MODEL FILE MISSING ";
										bReportIt = true;
									}
								}
							}
							if ( pTextureFile == NULL ) 
							{
								pReport = pReport + "TEXTURE NAME NOT GIVEN ";
								bReportIt = true;
							}
							else
							{
								if ( bTextureOkay == false )
								{
									if ( bFixAnyEntityIssues == true )
									{
										LPSTR pFindMissingFile = FindFileFromEntityBank(pTextureFile);
										if ( pFindMissingFile )
										{
											CopyFile ( pFindMissingFile, pTextureFile, TRUE );
											if ( FileExist ( pTextureFile ) == 1 )
											{
												// only if copied okay can remove from source
												//DeleteFile ( pFindMissingFile );
											}
											pReport = pReport + "TEXTURE FILE COPIED OVER " + pTextureFile + " ";
											delete pFindMissingFile;
											bReportIt = true;
										}
										else
										{
											pReport = pReport + "TEXTURE FILE UNKNOWN " + pTextureFile + " ";
											bReportIt = true;
										}
									}
									else
									{
										pReport = pReport + "TEXTURE FILE MISSING ";
										bReportIt = true;
									}
								}
							}
						}
						else
						{
							pReport = pReport + "VALID";
						}
						if ( bReportIt == true )
						{
							timestampactivity(0, pReport.Get());
						}

						// free usages if any
						if ( pModelFile ) delete pModelFile;
						if ( pTextureFile ) delete pTextureFile;
					}
				}
			}
		}
	}
	SetDir ( pOriginalDir );
	*/
	#endif
}

#ifdef VRTECH
LPSTR FindFileFromEntityBank ( LPSTR pFindThisFilename )
{
	// look through entire file collection for this file
	for ( int f = 1; f <= g.filecollectionmax; f++ )
	{
		LPSTR pFile = t.filecollection_s[f].Get();
		LPSTR pFileNameOnly = NULL;
		for ( int n = strlen(pFile); n > 0; n-- )
		{
			if ( pFile[n] == '\\' || pFile[n] == '/' )
			{
				pFileNameOnly = pFile+n+1;
				break;
			}
		}
		if ( pFileNameOnly )
		{
			if ( stricmp ( pFileNameOnly, pFindThisFilename ) == NULL )
			{
				// found the file!
				LPSTR pReturnAbsPathToFile = new char[2048];
				strcpy ( pReturnAbsPathToFile, g.fpscrootdir_s.Get() );
				strcat ( pReturnAbsPathToFile, "\\Files\\entitybank\\Objects - Copy\\" );
				strcat ( pReturnAbsPathToFile, pFile );
				return pReturnAbsPathToFile;
			}
		}
	}
	return NULL;
}
#endif

void FPSC_LoadKEYMAP ( void )
{
	// look in editors\keymap\ to find a non-default.ini to prefer
	if (PathExist("Files\\editors\\keymap") == 1 )
	{
		LPSTR pOldDir = GetDir();
		SetDir("Files\\editors\\keymap");
		ChecklistForFiles();
		cstr useThisKeyMapFile = "Files\\editors\\keymap\\default.ini";
		for (t.c = 1; t.c <= ChecklistQuantity(); t.c++)
		{
			t.tfile_s = ChecklistString(t.c);
			if (Len(t.tfile_s.Get()) > 2)
			{
				if (stricmp(t.tfile_s.Get(), "default.ini") != NULL)
				{
					useThisKeyMapFile = "Files\\editors\\keymap\\";
					useThisKeyMapFile += t.tfile_s;
					break;
				}
			}
		}
		SetDir(pOldDir);

		//  editors\keymap\default.ini
		t.tfile_s = useThisKeyMapFile;
		if ( FileExist(t.tfile_s.Get()) == 1 )
		{
			Dim (  t.data_s,999  );
			LoadArray (  t.tfile_s.Get(),t.data_s );
			for ( t.l = 0 ; t.l<=  999; t.l++ )
			{
				t.line_s=t.data_s[t.l];
				if (  Len(t.line_s.Get())>0 ) 
				{
					if (  cstr(Lower(Left(t.line_s.Get(),4))) == ";end"  )  break;
					if (  cstr(Left(t.line_s.Get(),1)) != ";" ) 
					{
						//  take fieldname and values
						for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
						{
							if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1; break; }
						}
						t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
						t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));
						for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
						{
							if ( t.value_s.Get()[t.c] == ',' ) { t.mid = t.c+1; break; }
						}
						t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
						t.value2_s=removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid));
						if (  Len(t.value2_s.Get())>0  )  t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;

						// try to match with key256 thru key1
						for ( t.num = 256; t.num >= 1; t.num-- )
						{
							sprintf ( t.szwork, "key%s", Str(t.num) );
							t.tryfield_s = t.szwork;
							if ( strnicmp ( t.field_s.Get(), t.tryfield_s.Get(), strlen(t.tryfield_s.Get()) ) == NULL )  
							{
								// and ensure its the exact value, is there a non-num after matched part
								char pDigitAfterNumber = t.field_s.Get()[strlen(t.szwork)];
								char pStartNum = '1';
								char pFinishNum = '0';
								if ( pDigitAfterNumber<pStartNum || pDigitAfterNumber>pFinishNum )
								{
									g.keymap[t.num] = (unsigned char)t.value1;
									break;
								}
							}
						}
					}
				}
			}
			UnDim ( t.data_s );
		}
	}
}

void common_switchtomysystemfolder ( void )
{
	g.fpscrootdir_s = g.mysystem.root_s;
	g.mysystem.levelBankTestMapAbs_s = g.fpscrootdir_s+"\\Files\\"+g.mysystem.levelBankTestMap_s;
	g.mysystem.editorsGrideditAbs_s = g.fpscrootdir_s+"\\Files\\"+g.mysystem.editorsGridedit_s;
	g.mysystem.mapbankAbs_s = g.fpscrootdir_s+"\\Files\\"+g.mysystem.mapbank_s;
	g.rootdir_s = g.fpscrootdir_s + "\\Files\\";
	LPSTR pStDir = GetDir();
	SetDir ( g.rootdir_s.Get() );
	g.rootdir_s = cstr(GetDir()) + "\\";
	SetDir ( pStDir );
	g.currentmeshdir_s = g.rootdir_s+"meshbank\\";
	g.currententitydir_s = g.rootdir_s+"entitybank\\";
	g.currenttexdir_s = g.rootdir_s+"texturebank\\";
	g.currentfxdir_s = g.rootdir_s+"effectbank\\";
	g.currentpredir_s = g.rootdir_s+"prefabs\\";
	g.currentsegdir_s = g.rootdir_s+"segments\\";
	g.currentvideodir_s = g.rootdir_s+"videobank\\";
}

void GenerateDOCDOCHelpFiles ( void )
{
	// init string list for help file
	std::vector<cstr> pHelpItems;

	// load in a source file to scan
	LPSTR pSourceFile = "..\\..\\GameGuru Core\\GameGuru\\Source\\Common.cpp";
	if ( FileExist ( pSourceFile ) == 1 )
	{
		// read source file, look for DOCDOC
		if ( FileOpen(1) == 1 ) CloseFile (  1 );
		if ( FileExist(pSourceFile) == 1 ) 
		{
			OpenToRead ( 1, pSourceFile );
			while ( FileEnd(1) == 0 )
			{
				LPSTR pLine = ReadString (1);
				LPSTR pToken = "// DOCDOC: ";
				LPSTR pDocDocLine = strstr ( pLine, pToken );
				if ( pDocDocLine != NULL )
				{
					cstr sRestOfLine = cstr(pDocDocLine+strlen(pToken));
					if ( strlen( sRestOfLine.Get() ) > 6 )
					{
						// advance past token and collect rest as valid help line
						pHelpItems.push_back ( sRestOfLine );
					}
				}
			}
			CloseFile ( 1 );
		}

		// save new help file in DOCS folder
		LPSTR pHelpFile = "..\\Docs\\SETUP INI Description.txt";
		if ( FileExist ( pHelpFile ) == 1) DeleteFileA ( pHelpFile );
		OpenToWrite ( 1, pHelpFile );
		WriteString ( 1, "SETUP.INI Field Descriptions" );
		WriteString ( 1, "============================" );
		WriteString ( 1, "" );
		for ( int n = 0; n < pHelpItems.size(); n++ )
		{
			WriteString ( 1, pHelpItems[n].Get() );
		}
		CloseFile ( 1 );
	}
}

UINT GetURLData ( LPSTR pDataReturned, DWORD* pReturnDataSize, LPSTR urlWhere )
{
	UINT iError = 0;
	unsigned int dwDataLength = 0;
	HINTERNET m_hInet = InternetOpenA( "InternetConnection", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( m_hInet == NULL )
	{
		iError = GetLastError( );
	}
	else
	{
		unsigned short wHTTPType = INTERNET_DEFAULT_HTTPS_PORT;
		HINTERNET m_hInetConnect = InternetConnectA( m_hInet, "www.thegamecreators.com", wHTTPType, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		if ( m_hInetConnect == NULL )
		{
			iError = GetLastError( );
		}
		else
		{
			int m_iTimeout = 2000;
			InternetSetOption( m_hInetConnect, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)&m_iTimeout, sizeof(m_iTimeout) );  
			HINTERNET hHttpRequest = HttpOpenRequestA( m_hInetConnect, "GET", urlWhere, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0 );
			if ( hHttpRequest == NULL )
			{
				iError = GetLastError( );
			}
			else
			{
				HttpAddRequestHeadersA( hHttpRequest, "Content-Type: application/x-www-form-urlencoded", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE );
				int bSendResult = 0;
				bSendResult = HttpSendRequest( hHttpRequest, NULL, -1, NULL, 0 );//(void*)(m_szPostData), strlen(m_szPostData) );
				if ( bSendResult == 0 )
				{
					iError = GetLastError( );
				}
				else
				{
					int m_iStatusCode = 0;
					char m_szContentType[150];
					unsigned int dwBufferSize = sizeof(int);
					unsigned int dwHeaderIndex = 0;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (void*)&m_iStatusCode, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int dwContentLength = 0;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (void*)&dwContentLength, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int ContentTypeLength = 150;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_CONTENT_TYPE, (void*)m_szContentType, (LPDWORD)&ContentTypeLength, (LPDWORD)&dwHeaderIndex );
					char pBuffer[ 20000 ];
					for(;;)
					{
						unsigned int written = 0;
						if( !InternetReadFile( hHttpRequest, (void*) pBuffer, 2000, (LPDWORD)&written ) )
						{
							// error
						}
						if ( written == 0 ) break;
						if ( dwDataLength + written > 10240 ) written = 10240 - dwDataLength;
						memcpy( pDataReturned + dwDataLength, pBuffer, written );
						dwDataLength = dwDataLength + written;
						if ( dwDataLength >= 10240 ) break;
					}
					InternetCloseHandle( hHttpRequest );
				}
			}
			InternetCloseHandle( m_hInetConnect );
		}
		InternetCloseHandle( m_hInet );
	}
	if ( iError > 0 )
	{
		char *szError = 0;
		if ( iError > 12000 && iError < 12174 ) 
			FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandleA("wininet.dll"), iError, 0, (char*)&szError, 0, 0 );
		else 
			FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, iError, 0, (char*)&szError, 0, 0 );
		if ( szError )
		{
			LocalFree( szError );
		}
	}

	// complete
	*pReturnDataSize = dwDataLength;
	return iError;
}

void FPSC_VeryEarlySetup(void)
{
	// Determine if GAME or MAPEDITOR here, so can set display mode accordingly
	g.trueappname_s = "Guru-Game";
	#ifdef VRTECH
	// still called guru-mapeditor.exe for now
	if (strcmp(Lower(Right(Appname(), 18)), "guru-mapeditor.exe") == 0
	||  strcmp(Lower(Right(Appname(), 16)), "vr quest app.exe") == 0
	||  strcmp(Lower(Right(Appname(), 15)), "gamegurumax.exe") == 0)
	#else
	if (strcmp(Lower(Right(Appname(), 18)), "guru-mapeditor.exe") == 0)
	#endif
	{
		g.trueappname_s = "Guru-MapEditor";
	}
}

void FPSC_Setup ( void )
{
	// prepare all default values 
	InitCTextC(); //PE: Init new CTextC.
	FPSC_SetDefaults();

	// moved detection of EDITOR vs Game further up
	FPSC_VeryEarlySetup();

	// quick Start state
	g.gfirsttimerun = 0;
	g.gshowonstartup = 0;
	///MessageBox(NULL, "", "", MB_OK);
	g.gshowannouncements = 1;
	if ( g.trueappname_s == "Guru-MapEditor" ) 
	{
		// startup file
		t.tfile_s="showonstartup.ini";
		if ( FileOpen(1) == 1 ) CloseFile (  1 );
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			OpenToRead (  1,t.tfile_s.Get() );
			t.tshowonstart_s = ReadString (1);
			#ifdef VRTECH
			#else
			cstr videoPlayedMax_s = ReadString (1);
			int iVideoPlayedMax = atoi(videoPlayedMax_s.Get());
			for ( int n=0; n<10; n++ )
			{
				cstr videoPlayedItem_s = ReadString (1);
				g.videoMenuPlayed[n] = atoi(videoPlayedItem_s.Get());
			}
			#endif
			CloseFile (  1 );
			g.gshowonstartup=ValF(t.tshowonstart_s.Get());
		}
		else
		{
			// first time run (for welcome and serial code system)
			g.gfirsttimerun=1;
			#ifdef VRTECH
			g.gshowonstartup=1;
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, "1" );
			CloseFile (  1 );
			#else
			g.gshowonstartup=0;
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, "0" );
			WriteString ( 1, "0" );
			for ( int n=0; n<10; n++ )
			{
				WriteString ( 1, "0" );
			}
			CloseFile (  1 );
			#endif
		}

		// news announcement flag
		t.tfile_s="showannouncements.ini";
		if ( FileOpen(1) == 1 ) CloseFile (  1 );
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			OpenToRead ( 1, t.tfile_s.Get() );
			cstr tshowannouncements_s = ReadString (1);
			CloseFile ( 1 );
			g.gshowannouncements = ValF(tshowannouncements_s.Get());
		}
		else
		{
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, "1" );
			CloseFile (  1 );
		}
	}

	// 050416 - get parental control flag and any password
	#ifdef VRTECH
	 g.quickparentalcontrolmode = 0;
	#else
	 g.quickparentalcontrolmode = 0;
	 g.quickparentalcontrolmodepassword[0] = 0;
	 g.quickparentalcontrolmodepassword[1] = 0;
	 g.quickparentalcontrolmodepassword[2] = 0;
	 g.quickparentalcontrolmodepassword[3] = 0;
	 t.tfile_s="parentalcontrolmode.ini";
	 if (  FileExist(t.tfile_s.Get()) == 1 ) 
	 {
		if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
		OpenToRead (  1,t.tfile_s.Get() );
		cstr quickparentalcontrolmode_s = ReadString ( 1 );
		g.quickparentalcontrolmodepassword[0] = ReadByte ( 1 );
		g.quickparentalcontrolmodepassword[1] = ReadByte ( 1 );
		g.quickparentalcontrolmodepassword[2] = ReadByte ( 1 );
		g.quickparentalcontrolmodepassword[3] = ReadByte ( 1 );
		CloseFile (  1 );
		g.quickparentalcontrolmode=ValF(quickparentalcontrolmode_s.Get());
	 }
	 else
	 {
		g.quickparentalcontrolmode=0;
	 }
	#endif

	// 050416 - get VRQ control flag and serial code
	g.vrqorggcontrolmode = 0;
	g.vrqcontrolmode = 0;
	g.vrqcontrolmodeserialcode = "";
	g.cleverbooksmodeusername = "";
	g.cleverbooksmodepassword = "";
	g.cleverbooksmodeentryindex = 0;
	g.vrqTriggerSerialCodeEntrySystem = 0;
	g.iTriggerSoftwareToQuit = 0;
	g.bCleverbooksBundleMode = false;
	if (g.trueappname_s != "Guru-MapEditor")
	{
		// standalone games need no serial code
	}
	else
	{
		#ifdef PRODUCTV3
		t.tfile_s = "cleverbooksmode.ini";
		if (FileExist(t.tfile_s.Get()) == 1)
		{
			g.bCleverbooksBundleMode = true;
		}
		else
		{
			t.tfile_s = "vrqcontrolmode.ini";
			if (FileExist(t.tfile_s.Get()) == 0)
			{
				OpenToWrite(1, t.tfile_s.Get());
				WriteString(1, "");
				CloseFile(1);
			}
		}
		g.vrqorggcontrolmode = 1;
		#else
		#ifdef VRTECH
		t.tfile_s = "ggcontrolmode.ini";
		if (FileExist(t.tfile_s.Get()) == 0)
		{
			OpenToWrite(1, t.tfile_s.Get());
			#ifdef ALPHAEXPIRESYSTEM
			WriteString(1, "ALPHA");
			#else
			WriteString(1, "");
			#endif
			CloseFile(1);
		}
		g.vrqorggcontrolmode = 2;
		#else
		t.tfile_s = "educontrolmode.ini";
		if (FileExist(t.tfile_s.Get()) == 1)
		{
			g.vrqorggcontrolmode = 2;
		}
		#endif
		#endif

		#ifndef PRODUCTCLASSIC
		if (g.vrqorggcontrolmode > 0)
		{
			// cleverbooks or vrq standalone
			if (g.bCleverbooksBundleMode == true)
			{
				// read username and password (default is "username" "password" if not used yet)
				g.vrqcontrolmode = 1;
				if (FileOpen(1) == 1) CloseFile(1);
				OpenToRead(1, t.tfile_s.Get());
				g.cleverbooksmodeusername = ReadString(1);
				g.cleverbooksmodepassword = ReadString(1);
				CloseFile(1);
			}
			else
			{
				// read serial code from controlmode file
				g.vrqcontrolmode = 1;
				if (FileOpen(1) == 1) CloseFile(1);
				OpenToRead(1, t.tfile_s.Get());
				g.vrqcontrolmodeserialcode = ReadString(1);
				CloseFile(1);
			}

			// ensure serial code has not expired, otherwise ask for new serial code
			bool bHaveValidAuthCredentialsStored = false;
			if ( g.bCleverbooksBundleMode == true)
			{
				// fresh install stores username as "username"
				if (stricmp ( g.cleverbooksmodeusername.Get(), "username" ) != NULL )
					bHaveValidAuthCredentialsStored = true;
			}
			else
			{
				if (strlen(g.vrqcontrolmodeserialcode.Get()) > 1)
					bHaveValidAuthCredentialsStored = true;
			}
			if ( bHaveValidAuthCredentialsStored == true )
			{
				// determine valid code
				int iValidCode = 0;
				if ( g.bCleverbooksBundleMode == true )
					iValidCode = common_isserialcodevalid(g.cleverbooksmodepassword.Get(), g.cleverbooksmodeusername.Get());
				else
					iValidCode = common_isserialcodevalid(g.vrqcontrolmodeserialcode.Get(), NULL);

				// is code valid
				if (iValidCode <= 0)
				{
					// serial code expired
					#ifdef ALPHAEXPIRESYSTEM
					///MessageBoxA(NULL, "Build has expired! Find out more at https://www.game-guru.com/max", "Activation Error", MB_OK);
					#else
					#ifdef CLOUDKEYSYSTEM
					// Allow UI to offer up cloud key dialog now - message already given with more detailed info on error earlier
					#else
					MessageBox(NULL, "Your code has expired, obtain an updated code to continue using the software.", "License Not Found", MB_OK);
					#endif
					#endif
					if (iValidCode == -1)
					{
						// no internet connection so cannot check key, just quit!
						g.iTriggerSoftwareToQuit = 4;
					}
					else
					{
						g.vrqTriggerSerialCodeEntrySystem = 1;
						g.iTriggerSoftwareToQuit = 1;
					}
				}
				else
				{
					// serial code valid - continue
					g.vrqTriggerSerialCodeEntrySystem = 0;
				}
			}
			else
			{
				// no serial code found, ask for one
				g.vrqTriggerSerialCodeEntrySystem = 1;
				g.iTriggerSoftwareToQuit = 1;
			}
			#ifndef PRODUCTV3
			g.vrqTriggerSerialCodeEntrySystem = 0;
			g.iTriggerSoftwareToQuit = 0;
			#endif

			// all VRQ is restricted content mode
			g.quickparentalcontrolmode = 2;
		}
		#endif

		#ifdef VRTECH
		#else

		#ifndef PRODUCTCLASSIC
		else
		{
			// no serial code found, ask for one
			g.vrqTriggerSerialCodeEntrySystem = 1;
			g.iTriggerSoftwareToQuit = 1;
		}
		#endif
		#endif
		//g.vrqTriggerSerialCodeEntrySystem = 0;
		//g.iTriggerSoftwareToQuit = 0;

		#ifndef PRODUCTCLASSIC
		// all VRQ is restricted content mode
		g.quickparentalcontrolmode = 2;
		#endif
	}

	/*
	#ifdef VRTECH
	#else
	else
	{
		// if non-VRQ, ensure Steam file present, otherwise exit software
		if ( g.trueappname_s == "Guru-MapEditor" ) 
		{
			#ifdef FREETRIALVERSION
			 // Does not need Steam files
			#else
			 if ( FileExist("steam_appid.txt") == 0 ) 
			 {
				MessageBox ( NULL, "Root file missing from installation.", "System File Not Found", MB_OK );
				g.iTriggerSoftwareToQuit = 1;
			 }
			#endif
		}
	}
	#endif
	*/

	// standalones need to know we are running in VR flavor
	#ifdef VRTECH
	 if ( g.iTriggerSoftwareToQuit == 0 )
	 {
		 g.vrqcontrolmode = 1;
	 }
	#endif

	//  Review Request Reminder state
	g.reviewRequestReminder = 0;
	#ifndef VRTECH
	g.reviewRequestMinuteCounter = 0;
	g.dwReviewRequestTimeStart = timeGetTime();
	t.tfile_s="reviewrequestreminder.ini";
	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		// read existing review status 
		if ( FileOpen(1) == 1 ) CloseFile ( 1 );
		OpenToRead ( 1,t.tfile_s.Get() );
		t.reviewRequestReminder_s = ReadString ( 1 );
		t.reviewRequestMinuteCounter_s = ReadString ( 1 );
		CloseFile ( 1 );
		g.reviewRequestReminder = ValF(t.reviewRequestReminder_s.Get());
		g.reviewRequestMinuteCounter = ValF(t.reviewRequestMinuteCounter_s.Get());
	}
	else
	{
		// create new review status file
		if (FileOpen(1) == 1) CloseFile(1);
		OpenToWrite(1, t.tfile_s.Get());
		WriteString(1, "1");
		WriteString(1, "0");
		CloseFile(1);
	}
	#endif

	//  Version Control - TEST GAME Mode
	g.gtestgamemodefromeditor = 0;
	g.gtestgamemodefromeditorokaypressed = 0;
	//version_commandlineprompt ( );

	//  get version information from version file
	g.version_s = "";
	g.gversion = 10000;
	if (  FileExist("version.ini") == 1 ) 
	{
		Dim (  t.data_s,99  );
		LoadArray (  "version.ini",t.data_s );
		t.version_s="";
		for ( t.n = 1 ; t.n<=  Len(t.data_s[0].Get()); t.n++ )
		{
			t.c_s=Mid(t.data_s[0].Get(),t.n);
			if (  t.c_s != "."  )  t.version_s = t.version_s+t.c_s;
		}
		g.version_s = t.data_s[0].Get();
		g.gversion=ValF(t.version_s.Get());
		UnDim (  t.data_s );
	}

	//  defeat 'cumilative virtual memory limit of 1.8GB' by terminating
	//  FPSC-Game.exe each complete level, and re-launching to start new heap each time
	g.startbuildinglevelfromdir_s = GetDir();
	g.startbuildingleveloverride = 1;

	t.tcommandline_s = gRefCommandLineString;
	if (  strcmp ( Lower(Left(t.tcommandline_s.Get(),3)) , "-bl" ) == 0 ) 
	{
		g.startbuildingleveloverride=ValF(Right(t.tcommandline_s.Get(),Len(t.tcommandline_s.Get())-3));
		t.tcommandline_s="-b";
	}

	//  COMMANDLINE Info
	g.grestoreeditorsettings = 0;
	if ( strcmp ( Lower(gRefCommandLineString) , "-r" ) == 0 ) 
	{
		//  RESTORE MAP Mode - exited editor prematurely
		g.grestoreeditorsettings=1;
	}

	// can reload standalone to clear fragmentation 
	g.iStandaloneIsReloading = 0;
	g.sStandaloneIsReloadingLevel = "";
	g.sStandaloneIsReloadingLevelGameStatChange = "";
	if ( strcmp ( Lower(Left(gRefCommandLineString,17)) , "-reloadstandalone" ) == 0 ) 
	{
		if ( strcmp ( Lower(Left(gRefCommandLineString,22)) , "-reloadstandalonelevel" ) == 0 ) 
		{
			// load into a specific level
			g.sStandaloneIsReloadingLevel = gRefCommandLineString + 22;
			// and extract any advancelevelfilename (from LOAD GAME functionality)
			char pSliceUp[2048];
			strcpy ( pSliceUp, g.sStandaloneIsReloadingLevel.Get() );
			LPSTR pAdvance = strstr ( pSliceUp, ":" );
			if ( pAdvance > 0 )
			{
				g.sStandaloneIsReloadingLevelGameStatChange = pAdvance+1;
				*pAdvance = 0;
				g.sStandaloneIsReloadingLevel = pSliceUp;
			}
			g.iStandaloneIsReloading = 2;
		}
		else
		{
			// load to the main menu
			g.iStandaloneIsReloading = 1;
		}
	}

	// Check and load SETUP.INI defaults
	g.mysystem.bUsingMySystemFolder = false; //PE: Need to be false by default.

	FPSC_LoadSETUPINI(false);
	if ( g.mysystem.bUsingMySystemFolder == true ) FPSC_LoadSETUPINI(true);
	FPSC_LoadKEYMAP();

	// 250917 - set default CPU animation flag for engine
	if ( g.allowcpuanimations == 0 )
		SetDefaultCPUAnimState ( 3 );
	else
		SetDefaultCPUAnimState ( 0 );

	// set adapter ordinal for next time display mode is set (below)
	if ( g.gadapterordinal>0 ) 
	{
		ForceAdapterOrdinal ( g.gadapterordinal );
	}
	#ifdef VRTECH
	ForceAdapterD3D11ONLY ( g.gadapterd3d11only );
	#endif

	// true nae of app to log
	backuptimestampactivity();
	sprintf(t.szwork, "Name of true app: %s", g.trueappname_s.Get());
	timestampactivity(0, t.szwork);

	//  Indicate a time stamp for app start time
	timestampactivity(0,"Flag to produce log files set in SETUP.INI (producelogfiles=1)");

	//  FPGC - 020810 - sub-PS2.0 cards CANNOT have post processing
	if (  g.gpostprocessing>0 ) 
	{
		t.tpsv_f=GetPixelShaderVersion();
		if (  t.tpsv_f<2.0  )  g.gpostprocessing = 0;
	}
	sprintf ( t.szwork , "postprocessing flag at %i and pixel shader version of %.2f" , g.gpostprocessing , t.tpsv_f );
	timestampactivity(0,t.szwork);

	//  Language neutral ( USERDETAILS Info )
	g.language_s = "";
	t.tfile_s="userdetails.ini";
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		Dim (  t.data_s,999  );
		LoadArray (  t.tfile_s.Get() ,t.data_s);
		for ( t.l = 0 ; t.l<=  999; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if (  cstr(Lower(Left(t.line_s.Get(),4))) == ";end"  )  break;
				if (  cstr(Left(t.line_s.Get(),1)) != ";" )
				{
					//  take fieldname and values
					for ( t.c = 0 ; t.c <  Len(t.line_s.Get()); t.c++ )
					{
						if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1 ; break; }
					}
					t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
					t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));
					for ( t.c = 0 ; t.c <  Len(t.value_s.Get()); t.c++ )
					{
						if (  t.value_s.Get()[t.c] == ',' ) { t.mid = t.c+1  ; break; }
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value2_s=removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid));
					if (  Len(t.value2_s.Get())>0  )  t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;
	
					//  localization data
					t.tryfield_s = "language" ; if (  t.field_s == t.tryfield_s  )  g.language_s = t.value_s;
				}
			}
		}
		UnDim (  t.data_s );
	}

	//  Physics debug mode (to see all collision shapes)
	if (  g.gdebugphysicsstate == 1  )  g.physicsdebug = 1;

	//  Forced switches
	if (  g.gcompilestandaloneexe == 1 ) 
	{
		//  Cannot be real game, must optimize
		g.grealgameviewstate=0;
		g.goptimizemode=1;

		//  Wipe any imported unique code (generate when build EXE)
		g.guniquegamecode_s="";
	}
	if (  g.gtestgamemodefromeditor == 1 ) 
	{
		g.grealgameviewstate=0;
	}

	// The MyGames folder precedes Files
	g.exedir_s = g.myownrootdir_s;//GetDir();
	g.exedir_s += "\\MyGames\\";
	sprintf ( t.szwork , "The executable folder will be: %s" , g.exedir_s.Get() );
	timestampactivity(0,t.szwork);

	// record location of lightmapper executable
	g.originalrootdir_s = GetDir();
	g.lightmapperexefolder_s = GetDir();

	// First task for any program is to enter the Files Folder
	t.tnopathprotomode=0;
	if ( PathExist("Files") == 1 ) 
		SetDir ( "Files" );
	else
		t.tnopathprotomode=1;
	sprintf ( t.szwork , "Entering 'Files' folder, now at: %s" , GetDir() );
	timestampactivity(0,t.szwork);

	//  Basic globals for all programs
	g.gmapeditmode = 0;

	//  Establish global files and folders
	g.imgext_s = "tga";
	g.rootdir_s = GetDir();
	g.rootdir_s +="\\";
	g.browserexe_s = "Guru-Browser.exe";
	g.browsername_s = "Guru TEST Browser";
	g.segeditexe_s = "Guru-Segments.exe";
	g.segeditname_s = "Guru TEST Segment Editor";
	g.gameexe_s = "Guru-Game.exe";
	g.gamename_s = "Guru Game";

	//  Default directories
	g.currentmeshdir_s = g.rootdir_s+"meshbank\\";
	g.currententitydir_s = g.rootdir_s+"entitybank\\";
	g.currenttexdir_s = g.rootdir_s+"texturebank\\";
	g.currentfxdir_s = g.rootdir_s+"effectbank\\";
	g.currentpredir_s = g.rootdir_s+"prefabs\\";
	g.currentsegdir_s = g.rootdir_s+"segments\\";
	g.currentvideodir_s = g.rootdir_s+"videobank\\";

	//  rem Right away set LEGACY CONVERT MODE (LOAD OBJECT should not load extra UV layers)
	timestampactivity(0,"set legacy convert mode");
	SetLegacyMode (  1 );

	//  reserve chunks of memory early to avoid fragmentation errors
	timestampactivity(0,"_obs_memorychunkinit");
	obs_memorychunkinit ( );

	//  And finally switch the resolution if different from default
	if ( 1 ) 
	{
		timestampactivity(0, g.trueappname_s.Get());
		if ( g.trueappname_s == "Guru-MapEditor" )
		{
			// MAP EDITOR MODE
			timestampactivity(0, "detected map editor is in effect");
			t.game.gameisexe=0;
			t.game.set.ismapeditormode=1;
			tgamesetismapeditormode = 1;
			// set backbuffer for editor
			t.bkwidth=GetDesktopWidth() ; t.bkheight=GetDesktopHeight();
			#ifdef VRTECH
			t.thevrmodeflag = 0; if ( g.gvrmode == 1 || g.gvrmode == 5 || g.gvrmode == 6 ) t.thevrmodeflag = 1;
			#else
			t.thevrmodeflag = 0; 
			if ( g.gvrmode != 0  ) t.thevrmodeflag = 1;
			#endif
			timestampactivity(0, "setting display mode via DirectX");
			if ( t.thevrmodeflag != 0 )
			{
				SetDisplayModeVR ( GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(), g.gvsync,0,0,0,0,t.thevrmodeflag );
			}
			else
			{
				SetDisplayModeMODBACKBUFFER (  GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(),g.gvsync,0,0,t.bkwidth,t.bkheight );
			}
			//PE: Res changed here. set to our window size again.

			// allow _e_ usage override
			#ifdef VRTECH
			SetCanUse_e_(1);
			#else
			if ( FileExist ( cstr(g.exeroot_s + cstr("\\leeandraveyrock.txt")).Get() ) == 1 )
				SetCanUse_e_(1);
			#endif

			// if flag set to generate DOCDOC help, do this here
			if ( g.globals.generatehelpfromdocdoc == 1 )
			{
				timestampactivity(0, "GenerateDOCDOCHelpFiles");
				GenerateDOCDOCHelpFiles();
			}
		}
		else
		{
			// NOT MAP EDITOR - GAME EXECUTABLE
			timestampactivity(0, "detected game executable in effect");


			//  STANDALONE GAME MODE
			t.game.gameisexe=1;
			t.game.onceonlyshadow=1;
			t.game.set.ismapeditormode=0;
			tgamesetismapeditormode = 0;
			// Allow _e_ usage
			SetCanUse_e_(1);
	
			//  do not need for loading page
			timestampactivity(0,"_game_hidemouse");
			game_hidemouse ( );
	
			//  Set FULLSCREEN MODE (or keep windowed mode)
			if (  g.gfullscreen == 1  )  SetWindowModeOff (  );
	
			//  set resolution for game
			timestampactivity(0,"set resolution for game");
			t.tdisplaymodeselected=0;
			if ( g.globals.riftmode>0 ) 
			{
				g.globals.riftmoderesult = 0;
				if ( g.globals.riftmoderesult == 0 ) 
				{
					//  Rift Display Mode
					g.gdisplaywidth=0;
					g.gdisplayheight=0;
					SetDisplayMode (  g.gdisplaywidth, g.gdisplayheight, 32, g.gvsync );
					t.tdisplaymodeselected=1;
				}
				else
				{
					sprintf ( t.szwork , "Rif Mode Result: %s" , Str(g.globals.riftmoderesult) );
					timestampactivity(0,t.szwork);
				}
			}
			timestampactivity(0,"set display mode");
			if ( t.tdisplaymodeselected == 0 ) 
			{
				// If rift not selected display mode, choose default
				if ( t.tnopathprotomode == 0 || g.gvrmode != 0 ) 
				{
					// detect -1,-1 in which case use GetDesktopWidth (  and height )
					if ( g.gdisplaywidth == -1 || g.gdisplayheight == -1 ) { g.gdisplaywidth = GetDesktopWidth() ; g.gdisplayheight = GetDesktopHeight(); }
					if ( g.gdisplaywidth != 640 || g.gdisplayheight != 480 || g.gdisplaydepth != 32 || g.gvrmode != 0 ) 
					{
						#ifdef VRTECH
						t.thevrmodeflag = 0; if ( g.gvrmode == 1 || g.gvrmode == 5 || g.gvrmode == 6 ) t.thevrmodeflag = 1;
						#else
						t.thevrmodeflag = 0 ; if (  g.gvrmode != 0  )  t.thevrmodeflag = 1;
						#endif
						if ( t.thevrmodeflag != 0 )
						{
							SetDisplayModeVR ( g.gdisplaywidth, g.gdisplayheight, g.gdisplaydepth, g.gvsync,0,0,0,0,t.thevrmodeflag );
						}
						else
						{
							// Replaced the call with this since the previous one screws up
							t.bkwidth=GetDesktopWidth() ; t.bkheight=GetDesktopHeight();
							SetDisplayModeMODBACKBUFFER (  GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(),g.gvsync,0,0,t.bkwidth,t.bkheight );
							g.gdisplaywidth=GetDesktopWidth();
							g.gdisplayheight=GetDesktopHeight();
						}
					}
				}
			}
		}

		// transfer SETUP.INI VRMODEMAG sensitivity setting to main engine
		g_fVR920Sensitivity = g.gvrmodemag / 100.0f;
	
		//  option use use correct aspect ratio?
		if (  g.gaspectratio == 1 ) 
		{
			t.aspect_f=GetDesktopWidth() ; t.aspect_f=t.aspect_f/GetDesktopHeight();
			g.realaspect_f=t.aspect_f;
		}
	}

	#ifdef ENABLEIMGUI
	//PE: IMGUI init.
	if (t.game.set.ismapeditormode == 1) 
	{
		//Setup default write folder and load preferences.
		extern char defaultWriteFolder[260];
		extern preferences pref;
		FindAWriteablefolder();

		bool bResetPreferences = true;
		char prefile[MAX_PATH];
		strcpy(prefile, defaultWriteFolder);
		strcat(prefile, "gameguru.pref");

		//Load setup.
		FILE* preffile = fopen(prefile, "rb");
		if (preffile)
		{
			size_t size = fread(&pref, 1, sizeof(pref), preffile);
			//Valid pref:
			if (pref.szCheckFile[0] == 'G' && pref.szCheckFile[9] == 'P')
			{
				bResetPreferences = false;
			}
			pref.launched++;
			fclose(preffile);
		}

		if (bResetPreferences)
		{
			//Init prefereences defaults.
			strcpy(pref.szCheckFile, "GAMEGURU-PREFS");
			pref.launched = 0;
			pref.current_style = 1; // now "blue" was 0 , Max default to style "Dark Style"
			pref.vStartResolution = { 1280,800 };
			for (int i = 0; i < 10; i++)
			{
				strcpy(pref.last_open_files[i], "");
			}

			for (int i = 0; i < 15; i++)
				strcpy(pref.search_history[i], "");

			for (int i = 0; i < 15; i++)
				strcpy(pref.small_search_history[i], "");

			for (int i = 0; i < 10; i++)
				strcpy(pref.last_import_files[i], "");

			for (int il = 0; il < 16; il++)
			{
				pref.vSaved_Light_Palette_R[il] = 1.0f;
				pref.vSaved_Light_Palette_G[il] = 1.0f;
				pref.vSaved_Light_Palette_B[il] = 1.0f;
				pref.iSaved_Light_Type[il] = -1;
				pref.iSaved_Light_Range[il] = 0;
				pref.fSaved_Light_ProbeScale[il] = 1.0f;
			}

			for (int il = 0; il < 16; il++)
			{
				strcpy(pref.Saved_Particle_Name[il], "");
				pref.Saved_bParticle_Preview[il] = true;
				pref.Saved_bParticle_Show_At_Start[il] = true;
				pref.Saved_bParticle_Looping_Animation[il] = true;
				pref.Saved_bParticle_Full_Screen[il] = false;
				pref.Saved_fParticle_Fullscreen_Duration[il] = 10.0f;
				pref.Saved_fParticle_Fullscreen_Fadein[il] = 1.0f;
				pref.Saved_fParticle_Fullscreen_Fadeout[il] = 1.0f;
				strcpy(pref.Saved_Particle_Fullscreen_Transition[il], "");
				pref.Saved_fParticle_Speed[il] = 1.0f;
				pref.Saved_fParticle_Opacity[il] = 1.0f;
			}

			pref.save_layout = true;
			pref.iMaximized = 1;
		}

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigViewportsNoTaskBarIcon = true;

		// no layout saving until complete UI work
		extern int refresh_gui_docking;
		refresh_gui_docking = 0; // reset layout.
		if (pref.save_layout) 
		{
			static char cLayoutFile[MAX_PATH];
			sprintf(cLayoutFile, "%suiv3.layout", defaultWriteFolder);
			io.IniFilename = &cLayoutFile[0]; //Enable saving.
			ImGuiContext& g = *GImGui;
			extern int refresh_gui_docking;
			if (DoesFileExist(cLayoutFile)) {
				refresh_gui_docking = 4; // dont update layout.
			}
			if (pref.current_version != V3VERSION) {
				pref.current_version = V3VERSION;
				refresh_gui_docking = 0; // reset layout.
			}
		}
		else {
			io.IniFilename = NULL; //Disable saving imgui.ini
		}

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		myDarkStyle(NULL); //for bordersize,padding ...
		myStyle2(NULL); //additional settings before change.

		//Restore style from preferences.
		if(pref.current_style == 0)
			myStyle2(NULL);
		else if (pref.current_style == 1)
			myDarkStyle(NULL);
		else if (pref.current_style == 2)
			ImGui::StyleColorsClassic();
		else if (pref.current_style == 3)
			myLightStyle(NULL);

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
		}

		// Setup Platform/Renderer bindings
		ImGui_ImplWin32_Init(g_pGlob->hWnd);
		ImGui_ImplDX11_Init(m_pD3D, m_pImmediateContext);
	}

	extern bool bImGuiInTestGame;
	if(t.game.gameisexe == 1)
		bImGuiInTestGame = true;
	#endif

	t.newwidth=g.gdisplaywidth ; t.newheight=g.gdisplayheight ; t.newdepth=g.gdisplaydepth;
	g.gratiox_f = g.gdisplaywidth;
	g.gratioy_f = g.gdisplayheight;
	
	//  realaspect# also used in VISUALS.DBA
	timestampactivity(0,"set camera aspect");
	SetCameraAspect (  g.realaspect_f );

	//  special case, VRMODE=5 detects VR920 (switches OFF if not found)
	sprintf ( t.szwork , "Special VR-Mode Flag: %s" , Str(g.gvrmode));
	timestampactivity(0,t.szwork);
	if (  g.gvrmode == 5 ) 
	{
		t.vr920exist=ResetLeftEye();
		if (  t.vr920exist == 1 ) 
			g.gvrmode=4;
		else
			g.gvrmode=0;
	}

	//  FPGC - 130411 - special case, VRMODE=6 assumes new device
	g.vrsidebysidestereo = 0;
	if (  g.gvrmode == 6 ) 
	{
		g.vrsidebysidestereo=1;
		ResizeSprite ( 0, 0, 0.5 );
		g.gvrmode=4;
	}

	//  Set editor to use a true 1:1 pixel mapping
	timestampactivity(0,"pixel states");
	SetChildWindowTruePixel ( 1 );

	// 201017 - only for standalones
	if ( t.game.gameisexe == 1 )
	{
		//  Must show the window (editor EXE hides by default)
		SetWindowSettings ( 0, 0, 0 );
		WindowToFront ( "Game Guru Standalone Executable" );
		SetWindowSize ( GetDesktopWidth(), GetDesktopHeight() );
		ShowWindow(); MaximiseWindow();
		DisableEscapeKey();

		// clear screen
		Cls(); Sync();
		Cls(); Sync();
	}

	// 230517 - after display created, before any visual elements, load all core shaders
	SETUPLoadAllCoreShadersFIRST(g.gforceloadtestgameshaders);

	// loading sequence needs welcome asset art (even if welcome panel not on startup)
	if ( t.game.gameisexe == 0 )
	{
		timestampactivity(0, "welcome_init(1)");
		welcome_init(1);
		timestampactivity(0, "welcome_staticbackdrop();");
		//PE: This will resize backbuffer after a few call to sync so we dont get strange results later. from: SetChildWindowTruePixel ( 1 );
		welcome_staticbackdrop();
		timestampactivity(0, "welcome_init(2)");
		welcome_init(2);
		common_loadfonts();
		welcome_updatebackdrop("");
	}
	else
		common_loadfonts();

	//  reposition default radar,compass and objective based on new screen resolution
	g.radarx =(g.gdisplaywidth/100.0)*90;
	g.radary = (g.gdisplayheight/100.0)*90;
	g.compassX =(g.gdisplaywidth/100.0)*10;
	g.compassY = (g.gdisplayheight/100.0)*90;
	g.objectivex= g.compassX;
	g.objectivey= g.compassY;
	g.armx = (g.gdisplaywidth/100.0);
	g.army = (g.gdisplayheight/100.0);
	g.airx = (g.gdisplaywidth/100.0);
	g.airy = (g.gdisplayheight/100.0);

	//  Ensure default language chosen
	if (  g.language_s == ""  )  g.language_s = "english";

	//  Indicate a time stamp for app start time
	sprintf ( t.szwork , "Just about to read languagebank\\%s\\textfiles\\guru-wordcount.ini" , g.language_s.Get() );
	timestampactivity(0,t.szwork);

	// Translation Component (load strarr data)
	if ( t.tnopathprotomode == 0 ) 
	{
		// 250618 - this is the old translation system, capable of translating interface into many languages
		t.stdir_s=GetDir();
		sprintf ( t.szwork , "languagebank\\%s\\textfiles\\" , g.language_s.Get() );
		SetDir ( t.szwork );
		sprintf ( t.szwork , "Language File Path:%s (exist=%s)" , GetDir() , Str(FileExist("guru-wordcount.ini")) );
		timestampactivity(0,t.szwork);
		if (  FileExist("guru-wordcount.ini") == 1 ) 
		{
			OpenToRead (  1,"guru-wordcount.ini" );
			t.wordlibmax_s = ReadString (  1 ); g.wordlibmax=ValF(t.wordlibmax_s.Get());
			CloseFile (  1 );
			t.strarrmax=1;
			Dim (  t.strarr_s,g.wordlibmax  );
			Dim (  t.wordlib_s,g.wordlibmax  );
			LoadArray (  "guru-words.txt",t.wordlib_s );
			for ( t.n = 0 ; t.n<=  g.wordlibmax; t.n++ )
			{
				for ( t.c = 1 ; t.c<=  Len(t.wordlib_s[t.n].Get()); t.c++ )
				{
					if (  cstr(Mid(t.wordlib_s[t.n].Get(),t.c)) == "=" ) 
					{
						t.strarri=ValF(Left(t.wordlib_s[t.n].Get(),t.c-1));
						if (  t.strarri <= ArrayCount(t.strarr_s) ) 
						{
							t.strarr_s[t.strarri]=Right(t.wordlib_s[t.n].Get(),Len(t.wordlib_s[t.n].Get())-t.c);
							if (  t.strarri>t.strarrmax  )  t.strarrmax = t.strarri;
						}
						break;
					}
				}
			}
			UnDim (  t.wordlib_s );
		}
		else
		{
			//  language file missing
			ExitPrompt (  "FAIL","CANNOT FIND LANGUAGE FILE!" );
			ExitProcess ( 0 );
		}
		SetDir (  t.stdir_s.Get() );
	}

	// MapEditor or Standalone Game
	bool bIsThisMapEditor = false;
	sprintf ( t.szwork , "trueappname_s=%s" , g.trueappname_s.Get() );
	timestampactivity(0,t.szwork);
	if (  g.trueappname_s == "Guru-MapEditor" ) 
		bIsThisMapEditor = true;
	
	// Common redirections to new My System write/read folder
	cstr mysystemfolder_s = "My System";
	if ( bIsThisMapEditor == false ) g.mysystem.bUsingMySystemFolder = false;
	g.mysystem.root_s = g.myownrootdir_s + "\\" + mysystemfolder_s + "\\";
	if ( g.mysystem.bUsingMySystemFolder == false ) g.mysystem.root_s = g.fpscrootdir_s + "\\";;
	g.mysystem.levelBankTestMap_s = "levelbank\\testmap\\";
	g.mysystem.levelBankTestMapAbs_s = g.fpscrootdir_s+"\\Files\\levelbank\\testmap\\";
	g.mysystem.editorsGridedit_s = "editors\\gridedit\\";
	g.mysystem.editorsGrideditAbs_s = g.fpscrootdir_s+"\\Files\\editors\\gridedit\\";
	g.mysystem.mapbank_s = "mapbank\\";
	g.mysystem.mapbankAbs_s = g.fpscrootdir_s+"\\Files\\mapbank\\";

	g.mysystem.thumbbank_s = g.fpscrootdir_s + "\\Files\\thumbbank\\";
	char pCacheFolder[MAX_PATH];
	strcpy(pCacheFolder, g.mysystem.thumbbank_s.Get());
	GG_GetRealPath(pCacheFolder, 1); //make sure it exists.
	g.mysystem.thumbbank_s = pCacheFolder;

	g.mysystem.cachebank_s = g.fpscrootdir_s + "\\Files\\cachebank\\";
	strcpy(pCacheFolder, g.mysystem.cachebank_s.Get());
	GG_GetRealPath(pCacheFolder, 1); //make sure it exists.
	g.mysystem.cachebank_s = pCacheFolder;

	//  LEAP POINT (detect if running as Guru-Game.exe or Guru-MapEditor.exe)
	if ( bIsThisMapEditor == true ) 
	{
		// MAP EDITOR


		#ifdef VRTECH
		// create itinerary file if first time, or just read it in
		CreateItineraryFile();

		// Write latest location of software to registry (for future patch installers)
		HKEY hKeyNames = 0;
		LPCSTR pSubKeyName = "Software\\VRQuest";
		LPSTR pThisVersion = g.version_s.Get();
		LPSTR pThisPath = g.fpscrootdir_s.Get();
		DWORD dwDisposition;
		DWORD Status = RegCreateKeyExA(HKEY_CURRENT_USER, pSubKeyName, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WRITE, NULL, &hKeyNames, &dwDisposition);
		if ( Status == ERROR_SUCCESS )
		{
			if ( dwDisposition == REG_OPENED_EXISTING_KEY )
			{
				RegCloseKey ( hKeyNames );
				Status = RegOpenKeyExA(HKEY_CURRENT_USER, pSubKeyName, 0L, KEY_WRITE, &hKeyNames);
			}
		}
		if ( hKeyNames != 0 )
		{
			if ( Status == ERROR_SUCCESS )
			{
				Status = RegSetValueExA(hKeyNames, "Version", 0, REG_SZ, (LPBYTE)pThisVersion, (strlen(pThisVersion)+1)*sizeof(char));
				Status = RegSetValueExA(hKeyNames, "LatestInstallPath", 0, REG_SZ, (LPBYTE)pThisPath, (strlen(pThisPath)+1)*sizeof(char));
			}
			RegCloseKey(hKeyNames);
		}
		#endif

		// For My System mode
		if ( g.mysystem.bUsingMySystemFolder == true )
		{
			// Create My System folder nest
			cstr mysystemfolder_s = "My System";
			SetDir ( g.myownrootdir_s.Get() );
			if ( PathExist(mysystemfolder_s.Get()) == 0  ) MakeDirectory ( mysystemfolder_s.Get() );
			if ( PathExist(mysystemfolder_s.Get()) == 1 ) 
			{
				SetDir ( mysystemfolder_s.Get() );
				if ( PathExist("Files") == 0  ) MakeDirectory ( "Files" );
				cstr ggprecompilefile_s = g.fpscrootdir_s + "\\ggprecompile.lua";
				CopyFileA ( ggprecompilefile_s.Get(), "ggprecompile.lua", TRUE );
			}
			SetDir ( g.fpscrootdir_s.Get() );

			// Each time GameGuru runs, ensure core files are copied over to writable area (initial run will take a few minutes)
			welcome_updatebackdrop("Copying files to My System folder");

			// Gather ALL files in original Files folder
			Undim ( t.filecollection_s );
			g.filecollectionmax = 0;
			Dim ( t.filecollection_s, 500 );

			// initial folders in Files
			SetDir ( g.fpscrootdir_s.Get() );
			addallinfoldertocollection("Files","");

			// Copy to System folder if file not exist (leave any existing files alone)
			SetDir ( g.mysystem.root_s.Get() );
			SetDir ( "Files" );
			createallfoldersincollection();
			for ( int f = 1; f <= g.filecollectionmax; f++ )
			{
				LPSTR pDestFile = t.filecollection_s[f].Get();
				cstr pSrcFile = g.fpscrootdir_s + "\\Files\\" + pDestFile;
				CopyFileA ( pSrcFile.Get(), pDestFile, TRUE );
			}

			// Now switch root folder to new home
			common_switchtomysystemfolder();
		}

		// Now display version number and resumer prompt
		LPSTR pFirstTextToShow = g.version_s.Get();
		if ( g.grestoreeditorsettings == 1 ) pFirstTextToShow = "RESUMING PREVIOUS SESSION";
		welcome_updatebackdrop(pFirstTextToShow);

		// Init MP System (and Activate Steam (always so single player can do snapshots and get Steam notifications) if Steam)
		mp_init ( );

		// Init default material sounds
		material_init ( );
		material_startup ( );

		// Write test!
		OpenToWrite ( 1,"testwrite.dat" );
		WriteString ( 1,"we can write to the Game Guru folder! Good." );
		CloseFile ( 1 );
		if ( FileExist("testwrite.dat") == 1 ) 
		{
			timestampactivity(0,"We can write to the Game Guru folder!");
			DeleteAFile ( "testwrite.dat" );
		}
		else
		{
			ExitPrompt ( "Game Guru cannot write files to the Files area. Exit the software, right click on the Game Guru icon, and select 'Run As Administrator'", "Init Error" );
		}

	
		//  New security requires Steam client to be running (for ownership check)
		g.iFreeVersionModeActive = 0;
		#ifdef VRTECH
		#else
		g.iFreeVersionModeActiveDuringEditor = 0;
		#endif
		#ifdef STEAMOWNERSHIPCHECKFREEWEEKEND
		 g.iFreeVersionModeActive = 1;
		 bool bSteamRunningAndGameGuruOwned = false;
		 if ( g.mp.isRunning == 1 )
		 {
			if ( SteamOwned() == true ) 
				bSteamRunningAndGameGuruOwned = true;
		 }
		 if ( bSteamRunningAndGameGuruOwned == false )
		 {
			g.iTriggerSoftwareToQuit = 2;
		 }
		#endif
		#ifdef FREETRIALVERSION
		 // free trial version mode
		 g.iFreeVersionModeActive = 2;
		 // discount code strings
		 strcpy ( g_trialDiscountCode, "" );
		 strcpy ( g_trialDiscountExpires, "" );
		 // countdown to trial ending
		 time_t now = time(0);
		 tm *ltm = localtime(&now);
		 int iDay   = ltm->tm_mday;
		 int iMonth = ltm->tm_mon;
		 int iYear  = ltm->tm_year-100;
		 // work out single value to represent days
		 int iTotalDays = (iYear*365)+(iMonth*31)+iDay;
		 // handle time stamp file
		 LPSTR pTrialStampFile = "..\\trialstamp.txt";
		 if ( FileExist ( pTrialStampFile ) == 0 )
		 {
			// get 7-day discount code
			DWORD dwDataReturnedSize = 0;
			char pDataReturned[10240];
			memset ( pDataReturned, 0, sizeof(pDataReturned) );

			char pGetDataString[1024];
			strcpy ( pGetDataString, "/api/discount/codes/generate?" );
			strcat ( pGetDataString, DISCOUNTKEY );
			strcat ( pGetDataString, "&discount=gamegurutrial" );
			#ifdef VRTECH
			#else
			strcat ( pGetDataString, "&discount_percentage=70" );
			#endif
			UINT iError = GetURLData ( pDataReturned, &dwDataReturnedSize, pGetDataString );
			if ( iError <= 0 && *pDataReturned != 0 && strchr(pDataReturned, '{') != 0 && dwDataReturnedSize < 10240 )
			{
				// break up response string
				// {
				// "success": true,
				// "discount_code": "GGTRIAL507CD20E3B2",
				// "expires": "2019-08-13 10:17:40"
				// }
				char pCodeText[10240];
				strcpy ( pCodeText, "" );
				char pExpiryText[10240];
				strcpy ( pExpiryText, "" );

				// work through data returned
				char pWorkStr[10240];
				strcpy ( pWorkStr, pDataReturned );
				if ( pWorkStr[0]=='{' ) strcpy ( pWorkStr, pWorkStr+1 );
				int n = 10200;
				for (; n>0; n-- ) if ( pWorkStr[n] == '}' ) { pWorkStr[n] = 0; break; }
				char* pChop = strstr ( pWorkStr, "," );
				char pStatusStr[10240];
				strcpy ( pStatusStr, pWorkStr );
				if ( pChop ) pStatusStr[pChop-pWorkStr] = 0;
				if ( pChop[0]==',' ) pChop += 1;
				if ( strstr ( pStatusStr, "success" ) != NULL )
				{
					// success
					// code
					pChop = strstr ( pChop, ":" ) + 2;
					strcpy ( pCodeText, pChop );
					char pEndOfChunk[4];
					pEndOfChunk[0]='"';
					pEndOfChunk[1]=',';
					pEndOfChunk[2]='"';
					pEndOfChunk[3]=0;
					char* pCodeTextEnd = strstr ( pCodeText, pEndOfChunk );
					pCodeText[pCodeTextEnd-pCodeText] = 0;
					pChop += strlen(pCodeText);

					// expiry
					pChop = strstr ( pChop, ":" ) + 2;
					strcpy ( pExpiryText, pChop );
					LPSTR pFindSpaceBetweenDateAndTime = strstr ( pExpiryText, " " );
					if ( pFindSpaceBetweenDateAndTime ) *pFindSpaceBetweenDateAndTime = 0;

					// copy to globals
					strcpy ( g_trialDiscountCode, pCodeText );
					strcpy ( g_trialDiscountExpires, pExpiryText );

					// only when get code can trial countdown start
					// create time stamp
					OpenToWrite ( 1, pTrialStampFile );
					WriteLong ( 1, iTotalDays );
					WriteString ( 1, g_trialDiscountCode );
					WriteString ( 1, g_trialDiscountExpires );
					CloseFile ( 1 );
				}
				else
				{
					// error
					char* pMessageValue = strstr ( pChop, ":" ) + 1;
				}
			}

			// no code, no trial start!
			if ( strcmp ( g_trialDiscountCode, "" ) == NULL )
			{
				strcpy ( g_trialDiscountCode, "No Discount" );
				strcpy ( g_trialDiscountExpires, "Unable To Get Code" );
			}

			// starts at 7 days
			g_trialStampDaysLeft = 7;
		 }
		 else
		 {
			OpenToRead ( 1, pTrialStampFile );
			int iDateTrialFirstUsed = ReadLong ( 1 );
			LPSTR pCode = ReadString ( 1 );
			LPSTR pExpiry = ReadString ( 1 );
			CloseFile ( 1 );
			strcpy ( g_trialDiscountCode, pCode );
			strcpy ( g_trialDiscountExpires, pExpiry );
			g_trialStampDaysLeft = 7-(iTotalDays-iDateTrialFirstUsed);
		 }
		 if ( g_trialStampDaysLeft <= 0 )
		 {
			g.iTriggerSoftwareToQuit = 2;
		 }
		#endif

		// 100718 - generate all new .BLOB files (used when making builds)
		if ( g.gforceloadtestgameshaders == 2 )
		{
			// scan effectsbank folder
			cstr pOldDir = GetDir();
			SetDir("effectbank\\reloaded");
			ChecklistForFiles();
			SetDir(pOldDir.Get());
			cstr ShaderPath = cstr("effectbank\\reloaded\\");
			for ( int c = 1; c < ChecklistQuantity(); c++ )
			{
				cstr file_s = ChecklistString(c);
				LPSTR pFilename = file_s.Get();
				if ( Len ( pFilename ) > 3 )
				{
					if ( strnicmp ( pFilename + strlen(pFilename) - 3, ".fx", 3 ) == NULL )
					{
						// some core shaders are except, but compile the rest
						bool bExempt = false;
						if ( stricmp ( pFilename, "apbr_core.fx" ) == NULL ) bExempt = true;
						if ( stricmp ( pFilename, "cascadeshadows.fx" ) == NULL ) bExempt = true;
						if ( bExempt == false )
						{
							// show which shader is being compiled into a blob
							t.tsplashstatusprogress_s = pFilename;
							timestampactivity(0,t.tsplashstatusprogress_s.Get());
							version_splashtext_statusupdate ( );

							// load shader to create blob file
							cstr ShaderFX_s = ShaderPath + file_s;
							char pShaderBLOB[2048];
							strcpy ( pShaderBLOB, file_s.Get() );
							pShaderBLOB[strlen(pShaderBLOB)-3]=0;
							strcat ( pShaderBLOB, ".blob" );
							cstr ShaderBLOB_s = ShaderPath + pShaderBLOB;
							SETUPLoadShader ( ShaderFX_s.Get(), ShaderBLOB_s.Get(), 0 );
						}
					}
				}
			}
			t.tsplashstatusprogress_s = "Finished compiling";
			timestampactivity(0,t.tsplashstatusprogress_s.Get());
			version_splashtext_statusupdate ( );
			g.iTriggerSoftwareToQuit = 3;
			g.gforceloadtestgameshaders = 0;
		}

		//  Enter Map Editor specific code
		SETUPLoadAllCoreShadersREST(g.gforceloadtestgameshaders,g.gpbroverride);
		#ifdef VRTECH
		material_loadsounds ( 1 );
		#else
		material_loadsounds ( 0 );
		#endif

		// finally launch editor 
		mapeditorexecutable();
		// Free before exit app
		mp_free ( );
	}
	else
	{
		//  ACTUAL GAME EXE

		//  Debug report status
		timestampactivity(0, "main game executable");

		//  Activate Steam (always so single player can do snapshots and get Steam notifications)
		mp_init();

		#ifdef VRTECH
		// VR Mode Initialisation
		g.globals.riftmode = 0;
		g.vrglobals.GGVREnabled = 0;
		g.vrglobals.GGVRUsingVRSystem = 1;
		if (g.gvrmode == 2) g.vrglobals.GGVREnabled = 1; // OpenVR (Steam)
		if (g.gvrmode == 3) g.vrglobals.GGVREnabled = 2; // Windows Mixed Reality (Microsoft)
		char pVRSystemString[1024];
		sprintf(pVRSystemString, "choose VR system with mode %d", g.vrglobals.GGVREnabled);
		timestampactivity(0, pVRSystemString);
		int iErrorCode = GGVR_ChooseVRSystem(g.vrglobals.GGVREnabled, g.gproducelogfiles, "");// cstr(g.fpscrootdir_s + "\\GGWMR.dll").Get());
		if (iErrorCode > 0)
		{
			char pErrorStr[1024];
			sprintf(pErrorStr, "Error Choosing VR System : Code %d", iErrorCode);
			timestampactivity(0, pErrorStr);
			timestampactivity(0, "switching VR off, headset not detected");
			g.vrglobals.GGVREnabled = 0;
		}
		else
		{
			// Give portal enough time to start its launch, then get rid of GameWindow until we need it!
			Sleep(1900);
			CloseWindow(g_pGlob->hOriginalhWnd);
			Sleep(100);
			g_pGlob->hOriginalhWnd = NULL;
			SetWindowPos(g_pGlob->hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		// Need editor 14.PNG for teleport graphic
		LoadImage("editors\\gfx\\14-white.png", g.editorimagesoffset + 14);
		if (!GetImageExistEx(g.editorimagesoffset + 14)) LoadImage("editors\\gfx\\14.png", g.editorimagesoffset + 14);
		LoadImage("editors\\gfx\\14-red.png", g.editorimagesoffset + 16);
		LoadImage("editors\\gfx\\14-green.png", g.editorimagesoffset + 17);
		#endif

		// Init default material sounds
		material_init();
		material_startup();

		//  Set device to get multisampling AA active in editor
		t.multisamplingfactor = 0; t.multimonitormode = 0;

		//  Init app
		SyncOn();
		SyncRate(0);
		SetAutoCamOff();
		AlwaysActiveOff();

		//  Camera aspect ratio adjustment for desktop resolution
		t.aspect_f = GetDesktopWidth(); t.aspect_f = t.aspect_f / GetDesktopHeight();
		SetCameraAspect(t.aspect_f);

		//  set-up test game screen prompt assets (for printscreenprompt())
		#ifdef VRTECH
		int iUseVRTest = 0;
		if (g.vrglobals.GGVREnabled > 0) iUseVRTest = 1;
		loadscreenpromptassets(iUseVRTest);
		#else
		loadscreenpromptassets(0);
		#endif
		printscreenprompt("");

		// delayed material load to after logo splash
		SETUPLoadAllCoreShadersREST(g.gforceloadtestgameshaders, g.gpbroverride);

		//  Generic asset loading common to editor and game
		common_loadfonts();
		common_loadcommonassets(1);

		// This used by 3D prompts in standalone
		g.guishadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_basic.fx");

		//  Load terrain from terrain temp save file
		terrain_createactualterrain();
		t.screenprompt_s = t.screenprompt_s + ".";
		printscreenprompt(t.screenprompt_s.Get());
		timestampactivity(0, "_terrain_load");
		t.tfile_s = g.mysystem.levelBankTestMap_s + "m.dat"; //"levelbank\\testmap\\m.dat";
		if (FileExist(t.tfile_s.Get()) == 1)
		{
			terrain_load();
		}

		//  Call visuals loop once to set shader constants
		t.visuals = t.gamevisuals;
		t.visuals.refreshshaders = 1;
		t.visuals.refreshlutsettings = 1;
		visuals_loop();

		//  Main loop
		timestampactivity(0, "Main Game Executable Loop Starts");

		// after initial steroscopic fake load, switch to true stereo if used
		g_VR920RenderStereoNow = true;

		//  One-off variable settings
		t.game.set.resolution = 0;
		t.game.set.initialsplashscreen = 0;

		//  ensure no collision from DBP!
		AutomaticCameraCollision(0, 0, 0);
		SetGlobalCollisionOff();

		//  Setup game view camera?
		SetCameraFOV(75);
		SetCameraRange(1, 4000);
		g.grav_f = -5.0;

		// temporarily hide main screen (post process will show it when ready)
		SetCameraView(0, 0, 0, 1, 1);

		// full speed
		SyncRate(0);

		//
		//  Launch game in EXE mode
		//
		// Master Root Run and Loop
		#ifdef VRTECH
		game_masterroot(iUseVRTest);
		#else
		game_masterroot(0);
		#endif

		bool bUseFragmentationMainloop = false;

		if (t.game.allowfragmentation == 0 || t.game.allowfragmentation == 2)
		{
			if(t.game.allowfragmentation_mainloop != 0)
				bUseFragmentationMainloop = true;
		}
		 // Only if not quitting standalone
		 if ( t.game.masterloop != 0 || bUseFragmentationMainloop )
		 {
			// 250619 - very large levels can fragment 32 bit memory after a few levels
			// so this mode will restart the executable, and launch the new level
			// crude solution until 64 bit allows greater memory referencing
			if ( t.game.allowfragmentation == 2 )
			{
				// next level load or back to main menu (both require relaunch)
				if ( strlen(t.game.jumplevel_s.Get()) > 0 )
				{
					// next level
					//timestampactivity(0, "Next level...");
					SoundDestructor();
					SetDir("..");
					LPSTR pEXEName = Appname();
					cstr pCommandLineString = cstr("-reloadstandalonelevel") + t.game.jumplevel_s + ":" + Str(t.luaglobal.gamestatechange);
					ExecuteFile ( pEXEName, pCommandLineString.Get(), "", 0 );
					Sleep(8000);
					return;
				}
				else
				{
					// new main menu (except if t.game.masterloop == 0 in which case we are quitting)
					t.game.allowfragmentation = 0;
				}
			}

			// 131115 - standalone game sessions fragment memory over time, so launch new instance
			// of the game executable (with silencing command line) and then quit this 'fragmented'
			// session after a few seconds to allow for a decent transition
			if ( t.game.allowfragmentation == 0 )
			{
				// replaced master loop with EXE relaunch
				//timestampactivity(0, "Relaunch...");
				SoundDestructor();
				SetDir("..");
				LPSTR pEXEName = Appname();
				ExecuteFile ( pEXEName, "-reloadstandalone", "", 0 );
				Sleep(8000);
				return;
			}
		 }

		 // Free before exit app
		 mp_free ( );
	}
}

void common_justbeforeend ( void )
{
	// PE: Dump image usage after level.
	// if (g.memgeneratedump == 1) 
	// {
	//	timestampactivity(0, "DumpImageList QUIT.");
	//	DumpImageList(); 
	// }

	// clear TXP caches before exit
	if ( g.gdeletetxpcachesonexit == 1 )
	{
		// scan "Files\ebebank\default" and delete any textures associated with present "TXP" files
		t.tolddir_s = GetDir();
		SetDir ( "ebebank\\default" );
		ChecklistForFiles (  );
		t.strwork = ""; t.strwork = t.strwork + "Clearing "+Str(ChecklistQuantity())+" TXP cache files";
		timestampactivity(0, t.strwork.Get() );
		for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
		{
			t.tfile_s=ChecklistString(t.c);
			if ( t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				#ifdef VRTECH
				if ( stricmp ( Right ( t.tfile_s.Get(), 4 ), ".txp" ) == NULL && stricmp ( t.tfile_s.Get(), "textures_profile.txp" ) != NULL && stricmp ( t.tfile_s.Get(), "original_profile.txp" ) != NULL ) 
				#else
				if ( stricmp ( Right ( t.tfile_s.Get(), 4 ), ".txp" ) == NULL && stricmp ( t.tfile_s.Get(), "textures_profile.txp" ) != NULL ) 
				#endif
				{
					// delete TXP file
					DeleteAFile ( t.tfile_s.Get() );
	
					// also delete associated textures belonging to this TXP file
					#ifdef VRTECH
					cStr tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_D.jpg");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_N.jpg");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_S.jpg");
					DeleteAFile ( tfilettex_s.Get() );
					#else
					cStr tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_D.dds");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_N.dds");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_S.dds");
					DeleteAFile ( tfilettex_s.Get() );
					#endif
				}
			}
		}
		timestampactivity(0,"Clearing complete.");
		SetDir ( t.tolddir_s.Get() );
	}

	// save number of minutes user been in session (added to global recorded when we entered)
	#ifndef VRTECH
	DWORD dwTimeNow = timeGetTime();
	DWORD dwTimeDifference = dwTimeNow - g.dwReviewRequestTimeStart;
	int moreMinutes = (int)(dwTimeDifference / 1000 / 60);
	g.reviewRequestMinuteCounter += moreMinutes;
	t.tfile_s = g.fpscrootdir_s + "\\reviewrequestreminder.ini";
	if ( FileExist(t.tfile_s.Get()) == 1 ) DeleteAFile (  t.tfile_s.Get() );
	if ( FileOpen(1) ==  1 ) CloseFile ( 1 );
	OpenToWrite ( 1, t.tfile_s.Get() );
	cstr theFlag = cstr(g.reviewRequestReminder);
	cstr theMinutes = cstr(g.reviewRequestMinuteCounter);
	WriteString ( 1, theFlag.Get() );
	WriteString ( 1, theMinutes.Get() );
	CloseFile (  1 );
	#endif
}

void common_loadfonts ( void )
{
	//  Bitmap Font Image Start
	t.tsplashstatusprogress_s="LOADING FONTS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	loadallfonts();
}

void common_loadcommonassets(int iShowScreenPrompts)
{

	//PE: Per country text in standalone.
	t.screenprompt_s = "PREPARING CORE FILES";
	sprintf(t.szwork, "languagebank\\%s\\inittext.ssp", g.language_s.Get());
	auto fp = fopen(t.szwork, "r");
	if (fp)
	{
		char ctmp[512];
		fgets(ctmp, 512, fp);
		fclose(fp);
		if (strlen(ctmp) > 0)
			t.screenprompt_s = ctmp;
	}

	//  Bitmap Font Image Start
	t.tsplashstatusprogress_s = "LOADING FONTS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	timestampactivity(0, "initbitmapfont");
	loadallfonts();

	// loading shaders message more accurate
	t.tsplashstatusprogress_s = "LOADING SHADERS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();

	// choose non-PBR or PBR shaders
	LPSTR pEffectStatic = "effectbank\\reloaded\\entity_basic.fx";
	LPSTR pEffectAnimated = "effectbank\\reloaded\\character_basic.fx";
	if (g.gpbroverride == 1)
	{
		pEffectStatic = "effectbank\\reloaded\\apbr_basic.fx";
		#ifdef VRTECH
		pEffectAnimated = "effectbank\\reloaded\\apbr_animwithtran.fx";
		#else
		pEffectAnimated = "effectbank\\reloaded\\apbr_anim.fx";
		#endif
	}

	// load common lightmapper PBR shader
	if (GetEffectExist(g.lightmappbreffect) == 0)
	{
		LPSTR pLightmapPBREffect = "effectbank\\reloaded\\apbr_lightmapped.fx";
		LoadEffect(pLightmapPBREffect, g.lightmappbreffect, 0);
		filleffectparamarray(g.lightmappbreffect);
	}
	if (GetEffectExist(g.lightmappbreffectillum) == 0)
	{
		LPSTR pLightmapPBREffect = "effectbank\\reloaded\\apbr_lightmapped_illum.fx";
		LoadEffect(pLightmapPBREffect, g.lightmappbreffectillum, 0);
		filleffectparamarray(g.lightmappbreffectillum);
	}

	#ifdef VRTECH
	// load common controller PBR shader
	if (GetEffectExist(g.controllerpbreffect) == 0)
	{
		LPSTR pPBREffect = "effectbank\\reloaded\\apbr_basic.fx";
		LoadEffect(pPBREffect, g.controllerpbreffect, 0);
		filleffectparamarray(g.controllerpbreffect);
	}
	#endif

	// load common third person character shader
	if (GetEffectExist(g.thirdpersoncharactereffect) == 0)
	{
		LoadEffect(pEffectAnimated, g.thirdpersoncharactereffect, 0);
		filleffectparamarray(g.thirdpersoncharactereffect);
	}
	if (GetEffectExist(g.thirdpersonentityeffect) == 0)
	{
		LoadEffect(pEffectStatic, g.thirdpersonentityeffect, 0);
		filleffectparamarray(g.thirdpersonentityeffect);
	}

	// Also preload the entity basic shader so editor does not freeze on first object load
	int tunusedhereid = loadinternaleffect(pEffectStatic);

	//  Setup visual settings
	t.tsplashstatusprogress_s = "INIT VECTORS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	common_vectorsinit();
	//Sync (  );
	t.tsplashstatusprogress_s = "INIT SKY ASSETS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	sky_init();
	//Sync (  );
	t.tsplashstatusprogress_s = "INIT TERRAIN ASSETS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	terrain_initstyles();
	grass_initstyles();
	//Sync (  );
	t.tsplashstatusprogress_s = "INIT GAME VISUAL ASSETS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	visuals_init();
	//Sync (  );
	t.tsplashstatusprogress_s = "INIT DECAL ASSETS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	decal_init();
	//Sync (  );
	t.tsplashstatusprogress_s = "INIT LIGHTMAP ASSETS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	lm_init();
	//Sync (  );

	//  Setup default paths
	t.levelmapptah_s = g.mysystem.levelBankTestMap_s; //"levelbank\\testmap\\";
	g.projectfilename_s = "";

	//  Get list of guns and flak for data
	t.screenprompt_s = t.screenprompt_s + ".";
	if (iShowScreenPrompts == 1) printscreenprompt(t.screenprompt_s.Get());
	t.tsplashstatusprogress_s = "SCANNING G-LIST";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	gun_scaninall_ref();
	gun_scaninall_dataonly();


	//  Load in default player sounds (default style) , PE: 0.5 sec so moved here while loading in background.
	t.tsplashstatusprogress_s = "LOAD PLAYER SOUNDS";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	g.gplayerstyle_s = ""; material_loadplayersounds();

	#ifdef VRTECH
	#else
	// Sky details for terrain lighting
	if ( t.game.gameisexe == 0 )
	{
		t.screenprompt_s=t.screenprompt_s+".";
		if ( iShowScreenPrompts == 1 ) printscreenprompt(t.screenprompt_s.Get());
		t.tsplashstatusprogress_s="SCANNING SKY SETTINGS";
		timestampactivity(0,t.tsplashstatusprogress_s.Get());
		version_splashtext_statusupdate ( );
		sky_skyspec_init ( );
	}
	#endif

	//  Create terrain (eventually default terrain randomised)
	t.screenprompt_s = t.screenprompt_s + ".";
	if (iShowScreenPrompts == 1) printscreenprompt(t.screenprompt_s.Get());
	t.tsplashstatusprogress_s = "CREATING TERRAIN";
	timestampactivity(0, t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate();
	terrain_setupedit();

	#ifdef ENABLEIMGUI
	if (t.game.gameisexe == 0)
	{
		t.terrain.terrainobjectindex = t.terrain.objectstartindex + 3;
		BT_ForceTerrainTechnique(1);
		t.terrain.waterliney_f = g.gdefaultwaterheight;

		if (GetEffectExist(t.terrain.effectstartindex + 0) == 0)
		{
			LPSTR pEffectToUse = "effectbank\\reloaded\\terrain_basic.fx";
			if (g.gpbroverride == 1) pEffectToUse = "effectbank\\reloaded\\apbr_terrain.fx";
			LoadEffect(pEffectToUse, t.terrain.effectstartindex + 0, 0);
			timestampactivity(0, cstr(cstr("Terrain Shader:") + pEffectToUse).Get());
		}
		terrain_assignnewshader();
		SetEffectShadowMappingMode(255);
		SetEffectToShadowMappingEx(t.terrain.terrainshaderindex, g.shadowdebugobjectoffset, g.guidepthshadereffectindex, g.globals.hidedistantshadows, 0, g.globals.realshadowresolution, g.globals.realshadowcascadecount, g.globals.realshadowcascade[0], g.globals.realshadowcascade[1], g.globals.realshadowcascade[2], g.globals.realshadowcascade[3], g.globals.realshadowcascade[4], g.globals.realshadowcascade[5], g.globals.realshadowcascade[6], g.globals.realshadowcascade[7]);
		if (t.game.runasmultiplayer == 1) mp_refresh();
		t.terrain.WaterCamY_f = 0.0;
		BT_ForceTerrainTechnique(1);
		t.terrain.waterliney_f = g.gdefaultwaterheight;

		//Create blank terrain here, while we also load in the background. this can take 2 sec.
		t.inputsys.donewflat = 1;
		t.inputsys.donew == 1;
		gridedit_new_map_quick();
		t.inputsys.donewflat = 0;
		t.inputsys.donew = 0;

		t.terrain.terrainobjectindex = 0; //reload of textures.

		terrain_make_image_only();
		if (GetImageExistEx(t.terrain.imagestartindex + 13) && GetImageExistEx(t.terrain.imagestartindex + 21))
		{
			TextureObject(t.terrain.terrainobjectindex, 2, t.terrain.imagestartindex + 13);
			TextureObject(t.terrain.terrainobjectindex, 4, t.terrain.imagestartindex + 21);
			terrain_generatesupertexture(false);
		}
		else
		{
			terrain_changestyle();
		}
		g.vegstyleindex = t.visuals.vegetationindex;
		grass_changevegstyle();
	}
	else
	{
		terrain_make();
	}
	#else
	terrain_make();
	#endif

	// Sky details for terrain lighting
	if (t.game.gameisexe == 0)
	{
		t.screenprompt_s = t.screenprompt_s + ".";
		if (iShowScreenPrompts == 1) printscreenprompt(t.screenprompt_s.Get());
		t.tsplashstatusprogress_s = "SCANNING SKY SETTINGS";
		timestampactivity(0, t.tsplashstatusprogress_s.Get());
		version_splashtext_statusupdate();
		sky_skyspec_init();
	}

	//  Create post process shader and apply
	t.tsplashstatusprogress_s="INIT POST PROCESSING";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	postprocess_general_init ( );
	//postprocess_forcheapshadows ( );

	//  Initialise ragdoll resources
	t.tsplashstatusprogress_s="INIT RAGDOLL SYSTEM";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	ragdoll_init ( );

	//  temporary call to load projectiles (RPG)
	t.tsplashstatusprogress_s="CREATING P-LIST";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	weapon_projectile_init ( );

	//  explosions and fire
	t.tsplashstatusprogress_s="CREATING E-LIST";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	explosion_init ( );

	//  Load all particle animation images
	t.tsplashstatusprogress_s="LOADING INTO P-LIST";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	ravey_particles_load_images ( );

	//  Load all resources
	t.tsplashstatusprogress_s="CREATING B-LIST";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	blood_damage_init ( );
}

void common_hide_mouse ( void )
{
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	SetFileMapDWORD (  1, 44, 0 );
	SetFileMapDWORD (  1, 48, 2 );
	SetFileMapDWORD (  1, 52, GetChildWindowWidth()/2 );
	SetFileMapDWORD (  1, 56, GetChildWindowHeight()/2 );
	SetFileMapDWORD (  1, 704,GetChildWindowWidth() );
	SetFileMapDWORD (  1, 708,GetChildWindowHeight() );
	#endif
}

void common_show_mouse ( void )
{
	// This does crazy cool stuff
	#ifdef FPSEXCHANGE
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	SetFileMapDWORD (  1, 44, 1 );
	SetFileMapDWORD (  1, 48, 0 );
	SetFileMapDWORD (  1, 52, GetChildWindowWidth()/2 );
	SetFileMapDWORD (  1, 56, GetChildWindowHeight()/2 );
	SetFileMapDWORD (  1, 704,GetChildWindowWidth() );
	SetFileMapDWORD (  1, 708,GetChildWindowHeight() );
	#endif
}

void common_vectorsinit ( void )
{
	//  One-off creation of vectors
	t.r=MakeMatrix(g.m4_view);
	t.r=MakeMatrix(g.m4_projection);
	t.r=MakeMatrix(g.m4_viewproj);
	t.r=MakeVector4(g.v4_far);
	t.r=MakeVector3(g.v3_far);
	t.r=MakeVector4(g.universalvectorindex);
	t.r=MakeVector4(g.terrainvectorindex);
	t.r=MakeVector4(g.terrainvectorindex1);
	t.r=MakeVector4(g.terrainvectorindex2);
	t.r=MakeVector4(g.terrainvectorindex3);
	t.r=MakeVector4(g.vegetationvectorindex);
	t.r=MakeVector4(g.weaponvectorindex);
	t.r=MakeVector4(g.generalvectorindex+1); // ScaleIn;
	t.r=MakeVector4(g.generalvectorindex+2); // Scale;
	t.r=MakeVector4(g.generalvectorindex+3); // HmdWarpParam;
	t.r=MakeVector4(g.generalvectorindex+4); // ScreenCenter;
	t.r=MakeVector4(g.generalvectorindex+5); // Len ( sCenter );
	t.r=MakeVector4(g.postprocesseffectoffset+1);
}

void common_wipeeffectifnotexist ( void )
{
	//  intercept classic shaders and redirect
	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		t.tokay=0;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\bump.fx"  )  t.tokay = 1;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\bumpent.fx"  )  t.tokay = 1;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\illuminationent.fx"  )  t.tokay = 1;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\bumpbone.fx"  )  t.tokay = 2;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\bumpbonerev.fx"  )  t.tokay = 2;
		if (  cstr(Lower(t.tfile_s.Get())) == "effectbank\\ps_2_0\\fastbone.fx"  )  t.tokay = 2;
		if (  t.tokay == 1  )  t.tfile_s = "effectbank\\reloaded\\entity_basic.fx";
		if (  t.tokay == 2  )  t.tfile_s = "effectbank\\reloaded\\character_basic.fx";
	}

	// if file not exist, use universal
	if (  FileExist(t.tfile_s.Get()) == 0 ) 
	{
		// use reloaded universal shader
		t.tfile_s="effectbank\\reloaded\\entity_basic.fx";
		if (  FileExist(t.tfile_s.Get()) == 0 ) 
		{
			// something went VERY wrong
			t.tfile_s="";
		}
	}
}

void common_makeeffecttextureset ( void )
{
	//  used by entitycore and segmentmaker (takes tfile$ and texdir$)
	common_wipeeffectifnotexist ( );

	//  reset output
	t.absolutelyrequirethistexture=0;
	t.ensureclearlayermax=0;
	t.texdir1_s="";
	t.texdir2_s="";
	t.texdir3_s="";
	t.texdir4_s="";
	t.texdir5_s="";

	//  V117 - 110110 - new full shader style means D2 preferred over D (thanks to Mark Blosser PS2 shaders)
	t.texturingtoken_s="D";
	if (  g.gnewblossershaders == 1 ) 
	{
		//  Abolishes use of unshaded D textures
		t.texturingtoken_s="D2";
	}

	//  determine effect textures to use
	if (  t.tfile_s.Get() != "" && t.segnoeffects == 0 ) 
	{

		//  D2=D+I+N+S
		t.tstr0_s=Left(t.texdir_s.Get(),Len(t.texdir_s.Get())-4);
		t.tstr1_s=Left(t.texdir_s.Get(),Len(t.texdir_s.Get())-6);
		t.tstr2_s=Right(t.texdir_s.Get(),4);
		if (  Len(t.tstr2_s.Get()) <= 1  )  t.tstr2_s = ".dds";

		//  D2 must be present to indicate existence of D, N, S, I, etc
		if (  cstr(Upper(Right(t.tstr0_s.Get(),2))) == "D2" ) 
		{
			//  The system used by FPSC is D.I.N.S for 0,1,2,3
			t.hardcodedtexturestages=0;

			//  related to scene universe (lightmap stage present)
			t.ttt_s = "illuminationmap.fx" ; if (  t.ttt_s  == Lower(Right(t.tfile_s.Get(),Len(t.ttt_s.Get()))) )  t.hardcodedtexturestages = 1;
			t.ttt_s = "bump.fx" ; if (  t.ttt_s == Lower(Right(t.tfile_s.Get(),Len(t.ttt_s.Get())))  )  t.hardcodedtexturestages = 2;

			//  related to entities (lightmap stage NOT present)
			t.ttt_s = "illuminationent.fx" ; if (   t.ttt_s == Lower(Right(t.tfile_s.Get(),Len(t.ttt_s.Get())))  )  t.hardcodedtexturestages = 11;
			t.ttt_s = "bumpent.fx" ; if (  t.ttt_s == Lower(Right(t.tfile_s.Get(),Len(t.ttt_s.Get()))) )  t.hardcodedtexturestages = 12;
			t.ttt_s = "bumpbone.fx" ; if ( t.ttt_s == Lower(Right(t.tfile_s.Get(),Len(t.ttt_s.Get())))  )  t.hardcodedtexturestages = 13;

			if (  t.hardcodedtexturestages>0 ) 
			{
				if (  t.hardcodedtexturestages == 1 ) 
				{
					//  [LM]+[DIFF+ILLU]
					t.ensureclearlayermax=2;
					t.texdir1_s=t.tstr1_s+"D2"+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+"D2"+t.tstr2_s;
					t.texdir3_s=t.tstr1_s+"I"+t.tstr2_s;
				}
				if (  t.hardcodedtexturestages == 2 ) 
				{
					//  [LM]+[DIFF+NORM]
					t.ensureclearlayermax=2;
					t.texdir1_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir3_s=t.tstr1_s+"N"+t.tstr2_s;
					//  Absolutely must have a NORMAL MAP, or fail this effect
					t.absolutelyrequirethistexture=3;
				}
				if (  t.hardcodedtexturestages == 11 ) 
				{
					//  [DIFF+ILLU]
					t.ensureclearlayermax=1;
					t.texdir1_s=t.tstr1_s+"D2"+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+"I"+t.tstr2_s;
				}
				if (  t.hardcodedtexturestages == 12 ) 
				{
					//  [DIFF+NORM]
					t.ensureclearlayermax=1;
					t.texdir1_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+"N"+t.tstr2_s;
					//  Absolutely must have a NORMAL MAP, or fail this effect
					t.absolutelyrequirethistexture=2;
				}
				if (  t.hardcodedtexturestages == 13 ) 
				{
					//  [DIFF+NORM+SPEC]
					t.ensureclearlayermax=2;
					t.texdir1_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+"N"+t.tstr2_s;
					t.texdir3_s=t.tstr1_s+"S"+t.tstr2_s;
					//  Absolutely must have a NORMAL MAP, or fail this effect
					t.absolutelyrequirethistexture=2;
				}
			}
			else
			{
				//  Standard DINS system
				if (  t.teffectuseslightmapstage == 1 ) 
				{
					//  [LM]+[DIFF+ILLU+NORM+SPEC+BRIH]
					t.texdir1_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir3_s=t.tstr1_s+"I"+t.tstr2_s;
					t.texdir4_s=t.tstr1_s+"N"+t.tstr2_s;
					t.texdir5_s=t.tstr1_s+"S"+t.tstr2_s;
				}
				else
				{
					//  [DIFF+ILLU+NORM+SPEC+BRIH]
					t.texdir1_s=t.tstr1_s+t.texturingtoken_s+t.tstr2_s;
					t.texdir2_s=t.tstr1_s+"I"+t.tstr2_s;
					t.texdir3_s=t.tstr1_s+"N"+t.tstr2_s;
					t.texdir4_s=t.tstr1_s+"S"+t.tstr2_s;
					t.texdir5_s=t.tstr1_s+"B"+t.tstr2_s;
				}
			}

			//  FPGC - 070710 - some entities do not follow pure naming conventions and are slightly modified
			//  such as Light4_G_G2.tga using the multi-use Light4_I.tga for the illumination, so we account for this
			for ( t.tcheck = 1 ; t.tcheck<=  4; t.tcheck++ )
			{
				if (  t.tcheck == 1  )  t.tcheck_s = t.texdir2_s;
				if (  t.tcheck == 2  )  t.tcheck_s = t.texdir3_s;
				if (  t.tcheck == 3  )  t.tcheck_s = t.texdir4_s;
				if (  t.tcheck == 4  )  t.tcheck_s = t.texdir5_s;
				if (  cstr(Right(Lower(t.tcheck_s.Get()),4)) == ".tga" ) {  t.tcheck_s = Left(t.tcheck_s.Get(),Len(t.tcheck_s.Get())-4); t.tcheck_s += ".dds"; }
				if (  FileExist(t.tcheck_s.Get()) == 0 ) 
				{
					//  assuming the format name_letter_D2.tga was used (therefore name_letter_I.tga)
					//  and the intended 'common support texture' was name_I.tga or name_N.tga or name_S.tga
					//  we can detect for and change it here if the alternative filename exists
					t.tcheck_s=Left(t.tcheck_s.Get(),Len(t.tcheck_s.Get())-7); t.tcheck_s += Right(t.tcheck_s.Get(),5);
					if (  cstr(Right(Lower(t.tcheck_s.Get()),4)) == ".tga"  )  { t.tcheck_s = Left(t.tcheck_s.Get(),Len(t.tcheck_s.Get())-4); t.tcheck_s += ".dds"; }
					if (  FileExist(t.tcheck_s.Get()) == 1 ) 
					{
						if (  t.tcheck == 1  )  t.texdir2_s = t.tcheck_s;
						if (  t.tcheck == 2  )  t.texdir3_s = t.tcheck_s;
						if (  t.tcheck == 3  )  t.texdir4_s = t.tcheck_s;
						if (  t.tcheck == 4  )  t.texdir5_s = t.tcheck_s;
					}
				}
			}
		}
		else
		{
			//  not D2.XXX, so just use texdir$ as base texture
			t.texdir1_s=t.texdir_s;
		}
	}
	else
	{
		//  use no effect this time
		t.tfile_s="";
	}
}

void version_splashtext_statusupdate ( void )
{
	// Update Splash Text (  with update on what is being loaded (startup IDE) )
	if ( t.game.gameisexe != 1 ) 
	{
		// and only if not running standalone
		welcome_updatebackdrop(t.tsplashstatusprogress_s.Get());
	}
}

//Functions

void popup_text_close ( void )
{
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
		return;
	#endif
	OpenFileMap (  2, "FPSPOPUP" );
	SetFileMapDWORD (  2, 8, 1 );
	SetEventAndWait (  2 );
	g_PopupControlMode = 0;
	Sleep(100);
}

void popup_text_change ( char* statusbar_s )
{
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
	//PE: Update prompt.
	popup_text(statusbar_s);
	return;
	#endif

	OpenFileMap (  2, "FPSPOPUP" );
	SetEventAndWait (  2 );
	if (  GetFileMapDWORD( 2, 0 )  ==  1 ) 
	{
		SetFileMapString ( 2, 1000, statusbar_s );
		SetFileMapDWORD ( 2, 4, 1 );
		SetEventAndWait ( 2 );
	}
}

char promptText[1024] = "\0";

void popup_text ( char* statusbar_s )
{
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) && defined(USERENDERTARGET)
	//PE: Update status bar.
	//statusbar_s
	extern bool bImGuiFrameState;
	extern bool bImGuiReadyToRender;
	extern bool imgui_is_running;

	if (!imgui_is_running) { //sent to backbuffer if we are not ready.
		pastebitmapfontcenter(statusbar_s, GetChildWindowWidth(0) / 2, ((GetChildWindowHeight(0) - 30) / 2), 4, 255);
		Sync();
		return;
	}

	if (!bImGuiFrameState)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		bImGuiFrameState = true;

		//######################################################################
		//#### Default dockspace setup, how is our windows split on screen. ####
		//######################################################################

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking; //ImGuiWindowFlags_MenuBar
		ImGuiViewport* viewport;
		viewport = ImGui::GetMainViewport();
		extern int toolbar_size;
		extern int ImGuiStatusBar_Size;
		ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, toolbar_size));
		ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, toolbar_size + ImGuiStatusBar_Size));
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		bool dockingopen = true;
		ImGui::Begin("DockSpaceAGK", &dockingopen, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);
		static ImGuiID dock_id_bottom;

		if (ImGui::DockBuilderGetNode(ImGui::GetID("MyDockspace")) != NULL) 
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		}
		ImGui::End();

		ImGui::Begin(TABENTITYNAME);
		ImGui::End();
		#ifdef USELEFTPANELSTRUCTUREEDITOR
		 extern bool bBuilder_Left_Window;
		 if (bBuilder_Left_Window) {
			ImGui::Begin("Structure Editor##LeftPanel");
			ImGui::End();
		 }
		#endif
		extern bool bHelpVideo_Window;
		extern bool bHelp_Window;
		if (bHelpVideo_Window && bHelp_Window) 
		{
			ImGui::Begin("Tutorial Video##HelpVideoWindow");
			ImGui::End();
			ImGui::Begin("Tutorial Steps##HelpWindow");
			ImGui::End();
		}
	}

	if (bImGuiFrameState && bImGuiReadyToRender) 
	{
		strcpy(promptText, statusbar_s);
		bStartNewPrompt = true;
		if( !imgui_is_running ) //sent to backbuffer if we are not ready.
		pastebitmapfontcenter(statusbar_s, GetChildWindowWidth(0) / 2, ((GetChildWindowHeight(0) - 30) / 2), 4, 255);
		Sync();
		Sync();
		bStartNewPrompt = true;
	}
	return;
	#endif

	if ( g_PopupControlMode == 0 )
	{
		t.strwork = "" ; t.strwork = t.strwork + "1:popup_text "+statusbar_s;
		timestampactivity(0, t.strwork.Get() );
		#ifdef FPSEXCHANGE
		OpenFileMap (  1,"FPSEXCHANGE" );
		SetFileMapDWORD (  1, 750, 1 );
		SetEventAndWait (  1 );
		while (  1 ) 
		{
			OpenFileMap (  2, "FPSPOPUP" );
			SetEventAndWait (  2 );
			if (  GetFileMapDWORD( 2, 0 )  ==  1 ) 
			{
				SetFileMapString (  2, 1000, statusbar_s );
				SetFileMapDWORD (  2, 4, 1 );
				SetEventAndWait (  2 );
				return;
			}
			Sync (  );
		}
		#endif
		g_PopupControlMode = 1;
	}
	else
	{
		popup_text_change ( statusbar_s );
	}
}

void loadresource ( void )
{
	cstr memoryusagetable_s =  "";
	int numberofitems = 0;
	int memused = 0;
	cstr name_s =  "";
	int n = 0;

	//  Load previously captured resource data
	memoryusagetable_s=g.mysystem.editorsGrideditAbs_s+"memusedtable.dat";//g.rootdir_s+"editors\\gridedit\\memusedtable.dat";
	if (  FileExist(memoryusagetable_s.Get()) == 1 ) 
	{
		OpenToRead (  1,memoryusagetable_s.Get() );
			numberofitems = ReadLong (  1 );
			if (  numberofitems>0 ) 
			{
				Dim (  t.gamememtable,numberofitems-1 );
				for ( n = 0 ; n<=  numberofitems-1; n++ )
				{
					name_s = ReadString (  1 ); t.gamememtable[n].name_s=name_s;
					memused = ReadLong ( 1 ); t.gamememtable[n].memused=memused;
				}
			}
			else
			{
				Undim ( t.gamememtable );
			}
		CloseFile (  1 );
	}
}

void saveresource ( void )
{
	cstr memoryusagetable_s =  "";
	int numberofitems = 0;
	int memused = 0;
	cstr name_s =  "";
	int n = 0;

	//  Save out resource captures
	memoryusagetable_s=g.mysystem.editorsGrideditAbs_s+"memusedtable.dat";//g.rootdir_s+"editors\\gridedit\\memusedtable.dat";
	if (  FileExist(memoryusagetable_s.Get()) == 1  )  DeleteAFile (  memoryusagetable_s.Get() );
	OpenToWrite (  1,memoryusagetable_s.Get() );
	numberofitems=1+ArrayCount(t.gamememtable);
	WriteLong (  1,numberofitems );
	if (  numberofitems>0 ) 
	{
		for ( n = 0 ; n<=  numberofitems-1; n++ )
		{
			name_s=t.gamememtable[n].name_s ; WriteString (  1,name_s.Get() );
			memused=t.gamememtable[n].memused ; WriteLong (  1,memused );
		}
	}
	CloseFile (  1 );
	if (  1 ) 
	{
		memoryusagetable_s=g.mysystem.editorsGrideditAbs_s+"memusedtable.log";//g.rootdir_s+"editors\\gridedit\\memusedtable.log";
		if (  FileExist(memoryusagetable_s.Get()) == 1  )  DeleteAFile (  memoryusagetable_s.Get() );
		OpenToWrite (  1,memoryusagetable_s.Get() );
			numberofitems=1+ArrayCount(t.gamememtable);
			sprintf ( t.szwork , "COUNT=%i" , numberofitems );
			WriteString (  1, t.szwork );
			if (  numberofitems>0 ) 
			{
				for ( n = 0 ; n<=  numberofitems-1; n++ )
				{
					sprintf ( t.szwork , "%i:%s[%i]" , n , t.gamememtable[n].name_s.Get() , t.gamememtable[n].memused );
					WriteString (  1, t.szwork );
				}
			}
			WriteString (  1,"END" );
		CloseFile (  1 );
	}
}

int openresource ( char* name_s )
{
	int actuallyused = 0;
	int gamememstamp = 0;
	int n;
	//  find existing slot, or free slot
	if (  Len(name_s)>1 ) 
	{
		actuallyused=0;
		g.gamememresourceid=-1;
		name_s=Lower(name_s);
		if (  ArrayCount(t.gamememtable) >= 0 ) 
		{
			for ( n = 0 ; n<=  ArrayCount(t.gamememtable); n++ )
			{
				if (  t.gamememtable[n].name_s == name_s ) 
				{
					g.gamememresourceid=n ; break;
				}
			}
		}

		//  test game records size of memory usage (start)
		if (  g.gamememresourceid == -1 ) 
		{
			n=ArrayCount(t.gamememtable)+1;
			Dim (  t.gamememtable,n );
			t.gamememtable[n].name_s=name_s;
			t.gamememtable[n].memused=-1;
			g.gamememresourceid=n;
		}
		gamememstamp=SMEMAvailable(1);
	}
	else
	{
		gamememstamp=SMEMAvailable(1);
		g.gamememresourceid=-1;
	}
	return actuallyused;
}

int closeresource ( void )
{
	int gamememstamp = 0;
	int memoryused = 0;
	if (  g.gamememresourceid >= 0 ) 
	{
		//  test game records size of memory usage (end)
		if (  t.gamememtable[g.gamememresourceid].memused == -1 ) 
		{
			//  and only if not filled in do we write the memory used
			memoryused=SMEMAvailable(1)-gamememstamp;
			t.gamememtable[g.gamememresourceid].memused=memoryused;
		}
		g.gamememresourceid=-1;
	}
	else
	{
		memoryused=SMEMAvailable(1)-gamememstamp;
	}
	return memoryused;
}

// 
//  SUBROUTINES AND FUNCTIONS FOR DEBUG ONLY
// 

void hide3d ( void )
{
	int o;
	BackdropOff ( ); SyncOff ( );
	SetCurrentBitmap (  0 );
	for ( o = 1 ; o <= 50000; o++ )
	{
		if ( ObjectExist(o) == 1 ) 
		{
			HideObject ( o );
		}
	}
}

void show3d ( void )
{
	int o;
	BackdropOn ( ); SyncOn ( );
	for ( o = 1 ; o <= 50000; o++ )
	{
		if (  ObjectExist(o) == 1 ) 
		{
			ShowObject (  o );
		}
	}
}

void debugfilename ( char* tfile_s, char* desc_s )
{
	cstr tryfile_s =  "";
	int texit = 0;
	texit=0;
	tryfile_s=tfile_s;
	if ( cstr( Lower(Right(tfile_s,3))) == "tga"  )  { tryfile_s = Left(tryfile_s.Get(),Len(tryfile_s.Get())-3); tryfile_s += g.imgext_s; }
	if (  FileExist(tfile_s) == 0 && FileExist(tryfile_s.Get()) == 0 ) 
	{
		sprintf ( t.szwork , "%s%s) info:%s" , t.strarr_s[53].Get() , tfile_s , desc_s );
		timestampactivity(0, t.szwork);
		g.timestampactivityflagged=1;
		//  FPGC - 090909 - add this to missingmedia report
		++g.missingmediacounter;
		Dim (  t.missingmedia_s,g.missingmediacounter  );
		t.missingmedia_s[g.missingmediacounter]=tfile_s;
	}
}

void debugstring ( char* tfile_s, char* desc_s )
{
	sprintf ( t.szwork , "%s%s) info:%s" , t.strarr_s[54].Get() , tfile_s , desc_s );
	timestampactivity(0, t.szwork);
}

void debugseevar ( int var )
{
	sprintf ( t.szwork , "%s%i)" , t.strarr_s[55].Get() , var );
	timestampactivity(0, t.szwork);
}

//Progress Report Debug Function

void debugviewprogressmax ( int progressmax )
{
	int gprogresscounter = 0;
	Dim (  t.mshot,progressmax  );
	g.lastmshotmem=0;
	g.gprogressmax=progressmax;
	gprogresscounter=0;
}

void debugviewtext ( int progress, char* gamedebugviewtext_s )
{
	int thisisaonetimeglobalbeforethisbuildends = 0;
	int gamedebugviewlastmem = 0;
	int gamedebugviewtime = 0;
	int gamedebugviewmem = 0;
	cstr gamedebugview_s =  "";
	float tscrhighdiff_f = 0;
	float tscrwiddiff_f = 0;
	float progress_f = 0;
	int progressh = 0;
	int progressw = 0;
	int progressx = 0;
	int progressy = 0;
	cstr thediff_s =  "";
	cstr thetime_s =  "";
	cstr themem_s =  "";
	int guiUsed = 0;
	cstr stat1_s =  "";
	cstr stat2_s =  "";
	cstr stat3_s =  "";
	int thediff = 0;
	int thetime = 0;
	cstr stat_s =  "";
	int themem = 0;
	int vmsize = 0;
	int stat2 = 0;
	int tokay = 0;
	int stat = 0;
	int st = 0;
	int t1 = 0;
	int t2 = 0;
	int ty = 0;

	//  leave immediately if mapeditor
	if (  g.gmapeditmode == 1  )  return;

	//  FPSCV104RC9 - loading time readout to file
	if (  g.gloadreportstate == 1 ) 
	{
		if (  g.loadreportarraydimmed == 0 ) { Dim (  t.loadreport_s,100000   ) ; g.loadreportarraydimmed = 1; }
		//  get time, distance from last time, description, etc..
		if (  g.gloadreporttime == 0  )  g.gloadreporttime = Timer();
		thetime=Timer()-g.gloadreporttime;
		thetime_s=Right(Str(1000000+thetime),6);
		thediff=Timer()-g.gloadreportlasttime;
		thediff_s=Right(Str(1000000+thediff),6);
		g.gloadreportlasttime=Timer();
		t.loadreport_s[g.gloadreportindex]=thetime_s+" : "+thediff_s+" : "+gamedebugviewtext_s;
		++g.gloadreportindex ; if (  g.gloadreportindex>10000  )  g.gloadreportindex = 10000;
		if (  progress>g.gprogressmax-10 ) 
		{
			//  only save in last 10 counts before end of progress bar
			sprintf ( t.szwork , "%s%s" , g.rootdir_s.Get() , "\\loadreport.txt" );
			SaveArray (  t.szwork , t.loadreport_s );
		}
	}

	//  progress bar based on progress/gprogressmax (some white and black)
	if (  progress != -1 ) 
	{
		//  Memory counters
		g.mshoti=progress;
		//  FPGC - 111209 - more info to indicate potential reason for a sudden crash (too much memory?)
		sprintf ( t.szwork , "[%s] %s  (%sMB used)" , Right(Str(1000+g.mshoti),3) , gamedebugviewtext_s , Str(SMEMAvailable(2)/1024) );
		strcpy ( gamedebugviewtext_s , t.szwork );
		g.mshotmem=GetMemoryAvailable(0);
		if (  g.lastmshotmem == 0 ) { g.lastmshotmem = g.mshotmem  ; g.mshotfirst = GetMemoryAvailable(0); }
		t.mshot[g.mshoti]=g.lastmshotmem-g.mshotmem;
		if (  t.mshot[g.mshoti]>g.mshotmemlargest  )  g.mshotmemlargest = t.mshot[g.mshoti];
		if (  g.mshoti>g.lastmshoti ) 
		{
			//  deposit memory use result to report
			sprintf ( t.szwork , "%s%sK  %s" , t.strarr_s[56].Get() , Str(t.mshot[g.mshoti]) , gamedebugviewtext_s );
			timestampactivity(0, t.szwork);
			g.lastmshoti=g.mshoti;
			g.lastmshotmem=g.mshotmem;
		}
		//  Game GUI Readout
		if (  progress>g.gprogressmax  )  progress = g.gprogressmax;
		progress_f=progress;
		progress_f=progress_f/(g.gprogressmax+0.0001);
		//  FPGC - 121009 - adjust progress# to account for GetDisplayWidth (  different from 1024 )
		tscrwiddiff_f=GetDisplayWidth();
		tscrwiddiff_f=tscrwiddiff_f/1024.0 ; progress_f=progress_f*tscrwiddiff_f;
		tscrhighdiff_f=GetDisplayHeight();
		tscrhighdiff_f=tscrhighdiff_f/800.0;
		if (  guiUsed  ==  0 ) 
		{
			if (  g.internalloaderhud>0 && g.internalloaderhud <= ArrayCount(t.hud) ) 
			{
							//  Image Expand Bar Progress
							t1=t.hud[g.internalloaderhud].width*progress_f;
							SizeSprite (  g.internalloaderhud,t1,t.hud[g.internalloaderhud].height*tscrhighdiff_f );
							PasteSprite (  g.internalloaderhud,t.hud[g.internalloaderhud].posx,t.hud[g.internalloaderhud].posy );
			}
		}
		SyncMask (  0x1  ); 
		Sync (  );
	}

	//  Report On Progress Percentage
	if (  progress != -1 ) 
	{
		if (  progress>g.gprogressmax  )  progress = g.gprogressmax;
		progress_f=progress ; progress_f=progress_f/(g.gprogressmax+0.0001);
		progress_f=progress_f*100.0;
	}

	//  TestGameFromEditor Mode
	if (  g.gtestgamemodefromeditor == 1 ) 
	{
		//  detect if CANCEL early (while building)
		if (  g.gtestgamemodefromeditorokaypressed == 0 ) 
		{
			#ifdef FPSEXCHANGE
			OpenFileMap (  2, "FPSEXCHANGE" );
			SetEventAndWait (  2 );
			if (  GetFileMapDWORD( 2, 994 )  ==  1 ) 
			{
				//  As can take time, tell user can take time
				OpenFileMap (  3, "FPSTESTGAMEDIALOG" );
				SetFileMapDWORD (  3, 12, 1 );
				SetFileMapString (  3, 1000, t.strarr_s[630].Get() );
				SetEventAndWait (  3 );
				while (  GetFileMapDWORD ( 3, 12 )  ==  1 ) 
				{
					SetEventAndWait (  3 );
				}

				// terminate test game mid-build
				SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
				SetFileMapString (  2, 1256, "-r" );
				SetFileMapDWORD (  2, 994, 2 );
				SetFileMapDWORD (  2, 924, 1 );
				SetEventAndWait (  2 );

				//  Terminate
				if (  1  )  timestampactivity(0,t.strarr_s[57].Get());
				ExitProcess ( 0 );
			}
			else
			{
				//  Update Test Game Dialog progress and status Text (  )
				OpenFileMap (  1, "FPSTESTGAMEDIALOG" );
				if (  progress != -1 ) 
				{
					SetFileMapDWORD (  1, 8, int(progress_f) );
				}
				//  Display Status Text (  )
				SetFileMapString (  1, 1000, gamedebugviewtext_s );
				SetFileMapDWORD (  1, 12, 1 );
				SetEventAndWait (  1 );
			}
			#endif
		}
	}
	//  Build Executable Game Mode
	if (  g.gcompilestandaloneexe == 1 ) 
	{
		//  check if build cancelled
		tokay=0;
		#ifdef FPSEXCHANGE
		OpenFileMap (  2, "FPSEXCHANGE" );
		SetEventAndWait (  2 );
		if (  GetFileMapDWORD( 2, 994 )  ==  1  )  tokay = 1;
		if (  tokay == 1 ) 
		{
			//  terminate build early
			OpenFileMap (  1, "FPSBUILDGAME" );
			SetFileMapDWORD (  1, 108, 1 );
			SetFileMapDWORD (  1, 112, 0 );
			//  close dialog (cannot reload data into it when RELOAD MAPEDITOR)
			SetFileMapDWORD (  1, 24, 1 );
			SetEventAndWait (  1 );
			//  call map editor back
			OpenFileMap (  2, "FPSEXCHANGE" );
			SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
			SetFileMapString (  2, 1256, "-r" );
			SetFileMapDWORD (  2, 994, 0 );
			SetFileMapDWORD (  2, 924, 1 );
			SetEventAndWait (  2 );
			//  terminate
			if (  1  )  timestampactivity(0,t.strarr_s[58].Get());
			ExitProcess( 0 );
		}
		else
		{
			//  game build progress bar and Text (  )
			OpenFileMap (  1, "FPSBUILDGAME" );
			if (  progress != -1 ) 
			{
				SetFileMapDWORD (  1, 108, 1 );
				SetFileMapDWORD (  1, 112, int(progress_f) );
			}
			SetFileMapDWORD (  1, 40, 12 );
			SetFileMapDWORD (  1, 44, 1 );
			if (  g.level>0 && g.level <= g.glevelmax ) 
			{
				sprintf ( t.szwork , "%s%i\\%i : %s" , t.strarr_s[59].Get(), g.level , g.glevelmax , gamedebugviewtext_s );
				SetFileMapString (  1, 1000, t.szwork );
			}
			else
			{
				SetFileMapString (  1, 1000, gamedebugviewtext_s );
			}
			SetEventAndWait (  1 );
		}
		#endif
	}

	//  FPGC - 110210 - some systems having issues with builds exceeding 1.5GB, so provide a graceful cap
	//  FPGC - 050510 - turns out virtual memory address fragmentation crashes apps exceeding 2GB
	if (  g.gsystemmemorycapoff == 0 ) 
	{
		vmsize=(SMEMAvailable(2)/1024);
		if (  vmsize>1850 ) 
		{
			if (  thisisaonetimeglobalbeforethisbuildends == 0 ) 
			{
				while ( 1 )
				{
					//  use a global to ensure this function does not call itself (recursively)
					thisisaonetimeglobalbeforethisbuildends=1;
					sprintf ( t.szwork , "Build process has exceed 1.85GB of virtual memory, using %iMB. Press CANCEL to abort this build!" , vmsize );
					debugviewtext(-1, t.szwork );
					SleepNow (  10 );
				}
				ExitProcess ( 0 );
			}
		}
	}
}

void printvalue ( int x, int y, int value )
{
	int tactualtextwidth = 0;
	float tcenterx_f = 0;
	float ttbitx_f = 0;
	float ttbity_f = 0;
	cstr text_s =  "";
	int ttnumy = 0;
	int ttnum = 0;
	int sid = 0;
	int tt = 0;

	//  prepare Sprite (  for Text (  Print ( ing ) ) )
	sid=g.effectmenuimagestart+4;
	Sprite (  sid,-10000,-10000,g.effectmenuimagestart+31 );
	SetSpriteDiffuse (  sid,255,255,255 );
	SetSpriteAlpha (  sid,255 );
	SizeSprite (  sid,10,10 );

	//  Print (  ValF ( ue ) )
	text_s = Str(value);
	tactualtextwidth=10;
	tcenterx_f=(Len(text_s.Get())*tactualtextwidth)/2;
	for ( tt = 1 ; tt<=  Len(text_s.Get()); tt++ )
	{
		ttnum=-1;
		if (  cstr(Mid(text_s.Get(),tt)) == "."  )  ttnum = 10 ;
		if (  cstr(Mid(text_s.Get(),tt)) == "\\"  )  ttnum = 11 ;
		if (  ttnum == -1  )  ttnum = Asc(Mid(text_s.Get(),tt))-Asc("0");
		if (  ttnum != -1 ) 
		{
			ttnumy=ttnum/4 ; ttnum=ttnum-(ttnumy*4);
			ttbitx_f=(1.0/64.0)*16 ; ttbity_f=(1.0/64.0)*16;
			SetSpriteTextureCoordinates (  sid,0,(ttbitx_f*ttnum),(ttbity_f*ttnumy) );
			SetSpriteTextureCoordinates (  sid,1,(ttbitx_f*ttnum)+ttbitx_f,(ttbity_f*ttnumy) );
			SetSpriteTextureCoordinates (  sid,2,(ttbitx_f*ttnum),(ttbity_f*ttnumy)+ttbity_f );
			SetSpriteTextureCoordinates (  sid,3,(ttbitx_f*ttnum)+ttbitx_f,(ttbity_f*ttnumy)+ttbity_f );
			PasteSprite (  sid,(x-tcenterx_f)+((tt-1)*tactualtextwidth),y );
		}
	}

	//  restore Sprite (  image )
	Sprite (  sid,-10000,-10000,sid );
}

//FUNCTION TO LAUNCH BROWSER

char* browseropen_s ( int browsemode )
{
	int segobjusedforsegeditor = 0;
	cstr baseimagepath_s =  "";
	int browseextcount = 0;
	cstr resultstring_s =  "";
	cstr browsetitle_s =  "";
	cstr filemapname_s =  "";
	cstr baseimage_s =  "";
	cstr extstring_s =  "";
	int browsetype = 0;
	cstr baselib_s =  "";
	cstr tresult_s =  "";
	cstr curdir_s =  "";
	cstr tfile_s =  "";
	cstr tpath_s =  "";
	int tresult = 0;

	//  this way still used by segment editor
	g.localdesc_s="";
	if (  segobjusedforsegeditor == 1 ) 
	{
		int segobjusedforsegeditor = 0;
		cstr baseimagepath_s =  "";
		int browseextcount = 0;
		cstr resultstring_s =  "";
		cstr browsetitle_s =  "";
		cstr filemapname_s =  "";
		cstr baseimage_s =  "";
		cstr extstring_s =  "";
		int browsetype = 0;
		cstr baselib_s =  "";
		cstr tresult_s =  "";
		cstr curdir_s =  "";
		cstr tfile_s =  "";
		cstr tpath_s =  "";
		int tresult = 0;

		//FOR SEGMENT EDITOR

		//  Clear result
		resultstring_s="";

		//  Store directory
		curdir_s=GetDir();

		//  Launch browser in freeze mode
		SetDir ( g.rootdir_s.Get() ); SetDir ("..");

		//  Run if not currently active
		if (  WindowExist(g.browsername_s.Get()) == 0 ) 
		{
			ExecuteFile (  g.browserexe_s.Get(),"","" );
			while (  WindowExist(g.browsername_s.Get()) == 0 ) 
			{
				Sync (  );
			}
		}
		else
		{
			WindowToFront (  g.browsername_s.Get() );
		}

		//  Trigger it to provide correct cateogory for browse
		filemapname_s=g.browsername_s+"(ACTIVE)";
		WriteFilemapValue (  filemapname_s.Get(),1 );
		filemapname_s=g.browsername_s+"(MODE)";
		WriteFilemapValue (  filemapname_s.Get(),browsemode );
		WriteFilemapString (  filemapname_s.Get(),t.strarr_s[42].Get() );

		//  Switch this app to processor friendly
		SyncOff ( ); AlwaysActiveOff ( );

		//  Must wait for response..
		filemapname_s=g.browsername_s+"(ACTIVE)";
		while (  ReadFilemapValue(filemapname_s.Get()) != 2 ) 
		{
		}

		//  This app must wait for..
		tresult=0;
		tresult_s="";
		filemapname_s=g.browsername_s+"(RESULT)";
		while (  tresult == 0 ) 
		{
			if (  WindowExist(g.browsername_s.Get()) == 1 ) 
			{
				WindowToFront (  g.browsername_s.Get() );
				tresult=ReadFilemapValue(filemapname_s.Get());
			}
			else
			{
				break;
			}
		}

		//  Take action based on result
		if (  tresult == 0  )  resultstring_s = "";
		if (  tresult == 1  )  resultstring_s = ReadFilemapString(filemapname_s.Get());
		if (  tresult == 2  )  resultstring_s = "";

		//  Restore primary activity
		WindowToBack (  g.browsername_s.Get() );
		WindowToFront (  );
		SyncOn (   ); AlwaysActiveOn (   ); Sync (  );

		//  set directory to return string (or restore)
		if (  Len(resultstring_s.Get())>0 ) 
		{
			tfile_s=getfile(resultstring_s.Get());
			tpath_s=Left(resultstring_s.Get(),Len(resultstring_s.Get())-Len(tfile_s.Get()));
			SetDir ( g.rootdir_s.Get() ); SetDir ( tpath_s.Get() );
			resultstring_s=GetDir();
			resultstring_s += "\\";
			resultstring_s += tfile_s;
		}
		else
		{
			SetDir (  curdir_s.Get() );
		}

	}
	else
	{
		// FOR MAP EDITOR
		//  Prepare browse type settings
		browsetype=browsemode;
		baseimagepath_s=".\\editors\\gfx\\browser\\";
		if (  browsetype == 1 ) 
		{
			browsetitle_s=t.strarr_s[43];
			baselib_s="";
			baseimage_s="all.bmp";
		}
		if (  browsetype == 2 ) 
		{
			browsetitle_s=t.strarr_s[44];
			baselib_s="texturebank\\";
			baseimage_s="texture.bmp";
		}
		if (  browsetype == 3 ) 
		{
			browsetitle_s=t.strarr_s[45];
			baselib_s="meshbank\\";
			baseimage_s="mesh.bmp";
		}
		if (  browsetype == 4 ) 
		{
			browsetitle_s=t.strarr_s[46];
			baselib_s="audiobank\\";
			baseimage_s="audio.bmp";
		}
		if (  browsetype == 5 ) 
		{
			browsetitle_s=t.strarr_s[47];
			baselib_s="effectbank\\";
			baseimage_s="effect.bmp";
		}
		if (  browsetype == 6 ) 
		{
			browsetitle_s=t.strarr_s[48];
			baselib_s="segments\\";
			baseimage_s="segment.bmp";
		}
		if (  browsetype == 7 ) 
		{
			browsetitle_s=t.strarr_s[49];
			baselib_s="prefabs\\";
			baseimage_s="prefab.bmp";
		}
		if (  browsetype == 8 ) 
		{
			browsetitle_s=t.strarr_s[50];
			baselib_s="mapbank\\";
			baseimage_s="map.bmp";
		}
		if (  browsetype == 9 ) 
		{
			browsetitle_s=t.strarr_s[51];
			baselib_s="entitybank\\";
			baseimage_s="entity.bmp";
		}

		//  Assign filters to browse types
		browseextcount=0;
		Dim (  t.browseext_s,64  );
		if (  browsetype == 1 ) 
		{
			t.browseext_s[browseextcount+1]=".*";
			++browseextcount;
		}
		if (  browsetype == 2 ) 
		{
			t.browseext_s[browseextcount+1]="tga";
			++browseextcount;
		}
		if (  browsetype == 3 ) 
		{
			t.browseext_s[browseextcount+1]="x";
			++browseextcount;
		}
		if (  browsetype == 4 ) 
		{
			t.browseext_s[browseextcount+1]="wav";
			t.browseext_s[browseextcount+2]="mp3";
			browseextcount += 2;
		}
		if (  browsetype == 5 ) 
		{
			t.browseext_s[browseextcount+1]="fx";
			++browseextcount;
		}
		if (  browsetype == 6 ) 
		{
			t.browseext_s[browseextcount+1]="fps";
			++browseextcount;
		}
		if (  browsetype == 7 ) 
		{
			t.browseext_s[browseextcount+1]="fpp";
			++browseextcount;
		}
		if (  browsetype == 8 ) 
		{
			t.browseext_s[browseextcount+1]="fpm";
			++browseextcount;
		}
		if (  browsetype == 9 ) 
		{
			t.browseext_s[browseextcount+1]="fpe";
			++browseextcount;
		}

		//  Build extension string (ie .wav,.mp3)
		extstring_s="";
		for ( t.t = 1 ; t.t <= browseextcount; t.t++ )
		{
			if (  t.t>1  )  extstring_s = extstring_s+",";
			extstring_s=extstring_s+t.browseext_s[t.t];
		}

		//  Call up browser dialog
		sprintf ( t.szwork , "%s\\%s" , g.rootdir_s.Get() , baselib_s.Get() );
		SetFileMapString (  1, 1000, cstr(g.rootdir_s+cstr("\\")+baselib_s).Get() );
		SetFileMapString (  1, 1256, browsetitle_s.Get() );

		//  File Filter
		SetFileMapString (  1, 1768,extstring_s.Get() );

		//  Default image if no thumbnail found
		sprintf ( t.szwork , "%s\\%s%s" , g.rootdir_s.Get() , baseimagepath_s.Get() , baseimage_s.Get() );
		SetFileMapString (  1, 2024, t.szwork );

		//  Window Title
		SetFileMapString (  1, 2280, browsetitle_s.Get() );

		//  Set last location for navigation
		if (  t.browserfolderhistory_s[browsetype] != "" ) 
		{
			SetFileMapString (  1, 2536, t.browserfolderhistory_s[browsetype].Get() );
		}

		//  Wait for dialog session to end
		SetFileMapDWORD (  1, 800, 1 );
		SetEventAndWait (  1 );
		while (  GetFileMapDWORD(1,800) == 1 ) 
		{
			SetEventAndWait (  1 );
		}

		g.localdesc_s=GetFileMapString( 1, 1256 );

		//  return string from browser dialog
		SetDir (  g.rootdir_s.Get() );
		resultstring_s=GetFileMapString( 1, 1512 );
		if (  resultstring_s != "" ) 
		{
			//  Final return string
			SetDir (  g.rootdir_s.Get() );
			//  Store location as we leave browser
			resultstring_s=Right(resultstring_s.Get(),Len(resultstring_s.Get())-Len(g.rootdir_s.Get()));
			t.browserfolderhistory_s[browsetype]=Right(resultstring_s.Get(),Len(resultstring_s.Get())-Len(baselib_s.Get()));
			sprintf ( t.szwork ,"%s\\%s" ,  GetDir() , resultstring_s.Get() );
			resultstring_s=t.szwork;

		}
		else
		{
			//  No return string
		}
		strcpy ( t.szreturn , resultstring_s.Get() );
		return (LPSTR)t.szreturn;
	}
	return NULL;
} 

void loadscreenpromptassets ( int iUseVRTest )
{
	if ( t.levelsforstandalone == 0 )
	{
		int tclosestresheight = 0;
		int tclosestreswidth = 0;
		int tresheight = 0;
		cstr respart_s =  "";
		int treswidth = 0;
		int tclosest = 0;
		cstr tfile_s =  "";
		int tdiff = 0;
		int tres = 0;
		if (  ImageExist(g.testgamesplashimage) == 1  )  DeleteImage (  g.testgamesplashimage );
		if (  ImageExist(g.testgamesplashimage) == 0 ) 
		{
			//  determine the resolution we should use
			tclosest=9999999;
			tclosestreswidth=0 ; tclosestresheight=0;
			for ( tres = 1 ; tres<=  12; tres++ )
			{
				if (  tres == 1 ) { treswidth = 1024  ; tresheight = 768;}
				if (  tres == 2 ) { treswidth = 1152  ; tresheight = 864;}
				if (  tres == 3 ) { treswidth = 1280  ; tresheight = 720;}
				if (  tres == 4 ) { treswidth = 1280  ; tresheight = 800;}
				if (  tres == 5 ) { treswidth = 1280  ; tresheight = 960;}
				if (  tres == 6 ) { treswidth = 1366  ; tresheight = 768;}
				if (  tres == 7 ) { treswidth = 1440  ; tresheight = 900;}
				if (  tres == 8 ) { treswidth = 1600  ; tresheight = 900;}
				if (  tres == 9 ) { treswidth = 1600  ; tresheight = 1200;}
				if (  tres == 10 ) { treswidth = 1680  ; tresheight = 1050;}
				if (  tres == 11 ) { treswidth = 1920  ; tresheight = 1080;}
				if (  tres == 12 ) { treswidth = 1920  ; tresheight = 1200;}
				tdiff=abs(GetDisplayWidth()-treswidth)+abs(GetDisplayHeight()-tresheight);
				if ( tdiff<tclosest ) 
				{
					// 050917 - check if this file exists for consideration
					if ( t.game.gameisexe == 1 ) 
					{
						#ifdef VRTECH
						sprintf ( t.szwork , "languagebank\\%s\\artwork\\watermark\\watermark-%ix%i.jpg", g.language_s.Get(), treswidth, tresheight );			
						#else
						sprintf ( t.szwork , "languagebank\\%s\\artwork\\watermark\\gameguru-watermark-%ix%i.jpg", g.language_s.Get(), treswidth, tresheight );
						#endif
						if ( FileExist ( t.szwork ) == 1 )
						{
							tclosest=tdiff;
							tclosestreswidth=treswidth;
							tclosestresheight=tresheight;
						}
					}
					else
					{
						tclosest=tdiff;
						tclosestreswidth=treswidth;
						tclosestresheight=tresheight;
					}
				}
			}
			if ( t.game.gameisexe == 0 ) 
			{
				// used for test game splash screen
				sprintf ( t.szwork , "%ix%i" , tclosestreswidth , tclosestresheight );
				respart_s = t.szwork;
			}
			else
			{
				if ( tclosest != 9999999 )
				{
					// use closest to current resolution
					#ifdef VRTECH
					 sprintf ( t.szwork , "watermark-%ix%i.jpg" , tclosestreswidth , tclosestresheight );
					#else
					 sprintf ( t.szwork , "gameguru-watermark-%ix%i.jpg" , tclosestreswidth , tclosestresheight );
					#endif
					respart_s = t.szwork;
				}
				else
				{
					// could not find any matching resolution files, just pick any file in the watermark folder
					LPSTR pOldDir = GetDir();
					sprintf ( t.szwork , "languagebank\\%s\\artwork\\watermark", g.language_s.Get() );
					SetDir(t.szwork);
					ChecklistForFiles (  );
					for ( int c = 1 ; c<=  ChecklistQuantity(); c++ )
					{
						if (  ChecklistValueA(c) == 0 ) 
						{
							tfile_s = ChecklistString(c);
							if (  tfile_s != "." && tfile_s != ".." ) 
							{
								respart_s = tfile_s;
								break;
							}
						}
					}
					SetDir(pOldDir);
				}
			}
			if ( t.game.gameisexe == 1 ) 
			{
				if ( g.iStandaloneIsReloading==0 )
				{
					// show splash initially
					tfile_s = respart_s;
					sprintf ( t.szwork, "languagebank\\%s\\artwork\\watermark\\%s", g.language_s.Get(), tfile_s.Get() );
					SetMipmapNum(1);
					LoadImage ( t.szwork, g.testgamesplashimage );
					SetMipmapNum(-1);
				}
				else
				{
					// when replay game, we actually reload the whole EXE (and dont show the init prompt and splash again)
					if ( ImageExist ( g.testgamesplashimage )==1 )
						DeleteImage ( g.testgamesplashimage );
				}
			}
			else
			{
				if ( g.quickparentalcontrolmode == 2 )
				{	
					#ifdef VRTECH
					if ( g.vrqcontrolmode != 0 )
					{
						if ( (g.gvrmode == 3 && iUseVRTest == 1) || iUseVRTest == 2 )
							sprintf ( t.szwork , "languagebank\\%s\\artwork\\testgamelayout-vr.png", g.language_s.Get() );
						else
							sprintf ( t.szwork , "languagebank\\%s\\artwork\\testgamelayout.png", g.language_s.Get() );
					}
					else
					{
						sprintf ( t.szwork , "languagebank\\%s\\artwork\\testgamelayout.png", g.language_s.Get() );
					}
					#else
					sprintf ( t.szwork , "languagebank\\%s\\artwork\\testgamelayout-noweapons.png", g.language_s.Get() );
					#endif
				}
				else
				{
					if (  t.game.runasmultiplayer == 1 ) 
					{
						sprintf ( t.szwork , "testgamelayoutmp-%s.png" , respart_s.Get() );
						tfile_s=t.szwork;
					}
					else
					{
						sprintf ( t.szwork , "testgamelayout-%s.png" , respart_s.Get() );
						tfile_s=t.szwork;
					}
					sprintf ( t.szwork , "languagebank\\%s\\artwork\\%s" , g.language_s.Get() , tfile_s.Get() );
				}
				SetMipmapNum(1); //PE: mipmaps not needed.
				image_setlegacyimageloading(true);
				LoadImage (  t.szwork , g.testgamesplashimage );
				image_setlegacyimageloading(false);
				SetMipmapNum(-1);
			}
		}
	}
}

void printscreenprompt ( char* screenprompt_s )
{
	if ( t.levelsforstandalone == 0 )
	{
		float tscrwidth_f = 0;
		float txoffset_f = 0;
		float tratio_f = 0;
		int tsidemax = 0;
		float twidth_f = 0;
		float tatx_f = 0;
		int tside = 0;
		int s;

		// display a prompt
		if ( ImageExist(g.testgamesplashimage) == 1 ) 
		{
			// switched set sprite and sprite around so the sprite definately exists first
			Sprite ( 1234,-100000,-100000,g.testgamesplashimage );
			SetSprite ( 1234,0,1 );
			tratio_f=(GetDesktopWidth()+0.0)/(GetDesktopHeight()+0.0);
			twidth_f=GetDisplayHeight()*tratio_f;
			tscrwidth_f=GetDisplayWidth();
			txoffset_f=0;
			tsidemax=0;
			#ifdef VRTECH
			if ( g.globals.riftmode > 0 || ( g.gvrmode > 0 && g.gvrmode != 2 && g.gvrmode != 3 ) ) 
			#else
			if ( g.globals.riftmode > 0 || g.gvrmode > 0 ) 
			#endif
			{
				tscrwidth_f=tscrwidth_f/2;
				twidth_f=twidth_f/2;
				txoffset_f=twidth_f/-2;
				tsidemax=1;
			}
			SizeSprite (  1234,twidth_f,GetDisplayHeight() );
		}

		// draw splash and/or text
		for ( s = 0 ; s<=  1; s++ )
		{
			CLS ( 0 );
			for ( tside = 0 ; tside <= tsidemax; tside++ )
			{
				tatx_f=txoffset_f+(tside*twidth_f)+(0-((twidth_f-GetDisplayWidth())/2));
				if ( ImageExist(g.testgamesplashimage) == 1 ) PasteSprite ( 1234, tatx_f, 0 );
				if ( t.game.gameisexe == 1 ) 
				{
					if ( g.gsuspendscreenprompts == 0 )
					{
						if ( g.iStandaloneIsReloading==0 )
							pastebitmapfontcenter(screenprompt_s,tatx_f+(tscrwidth_f/2),(GetDisplayHeight()/1.45),1,255);
						else
							pastebitmapfontcenter("",tatx_f+(tscrwidth_f/2),(GetDisplayHeight()/1.45),1,255);
					}
				}
				else
				{
					pastebitmapfontcenter(screenprompt_s,tatx_f+(tscrwidth_f/2),(GetDisplayHeight()/1.1),1,255);
				}
			}
			Sync (  );
		}
	}

	// steam refresh to keep it live
	mp_refresh ( );

	//  quickly check contiguous memory (as this gets called every time prompt printed)
	checkmemoryforgracefulexit();
}

int mod ( int num, int modulus )
{
	int value = 0;
	value = num-((num/modulus)*modulus);
	return value;
}

void GGBoxGradient ( int iLeft, int iTop, int iRight, int iBottom, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4 )
{
	int iWidth = iRight - iLeft;
	int iHeight = iBottom - iTop;
	Sprite ( 1235, -100000, -100000, g.slidersmenuimageoffset + 8 );
	SetSprite ( 1235, 0, 1 );
	SizeSprite ( 1235, iWidth, iHeight );
	SetSpriteDiffuse ( 1235, dw2, dw3, dw4 );
	SetSpriteAlpha ( 1235, dw1 );
	PasteSprite ( 1235, iLeft, iTop );
}

// copied from Common.cpp (above)
int geditorimagesoffset = 65110;

void InkEx ( int r, int g, int b )
{
	if ( ImageExist ( geditorimagesoffset+14 ) == 0 ) LoadImage (  "editors\\gfx\\14.png", geditorimagesoffset+14 );
	Sprite ( 123, -10000, -10000, geditorimagesoffset+14 );
	SetSpriteDiffuse ( 123, r, g, b );
}
void BoxEx ( int x1, int y1, int x2, int y2 )
{
	if ( x2 < x1 ) { int St = x1; x1 = x2; x2 = St; }
	if ( y2 < y1 ) { int St = y1; y1 = y2; y2 = St; }
	SizeSprite ( 123, x2-x1, y2-y1 );
	PasteSprite ( 123, x1, y1 );
}
void LineEx ( int x1, int y1, int x2, int y2 )
{
	if ( x2 < x1 ) { int St = x1; x1 = x2; x2 = St; }
	if ( y2 < y1 ) { int St = y1; y1 = y2; y2 = St; }
	int width = x2-x1;
	int height = y2-y1;
	if ( width > height )
		SizeSprite ( 123, width, 1+(y2-y1) );
	else
		SizeSprite ( 123, 1+(x2-x1), height );
	PasteSprite ( 123, x1, y1 );
}

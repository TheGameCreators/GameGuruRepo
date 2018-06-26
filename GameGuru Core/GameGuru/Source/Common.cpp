//----------------------------------------------------
//--- GAMEGURU - Common
//----------------------------------------------------

// Includes
#include "gameguru.h"
#include <stdio.h>
#include <stdlib.h>
#include "M-WelcomeSystem.h"
#include "time.h"

// Used for Free Weekend Promotion Build 
//#define STEAMOWNERSHIPCHECKFREEWEEKEND

// core externs to globals
extern LPSTR gRefCommandLineString;
extern bool gbAlwaysIgnoreShaderBlobFile;
extern bool g_VR920RenderStereoNow;
extern float g_fVR920Sensitivity;

// Globals
int g_PopupControlMode = 0;

// to enable the use of _e_ in standalone
void SetCanUse_e_ ( int flag );

// C++ CONVERSION: g contains all variables that were defined as global in dbpro source
// C++ CONVERSION: t contains all variables that were considered temporary and subject to change between routines
Sglobals g;
Stemps t;

// 
//  Common Between Map Editor and Game Runner
// 


//Subroutines

void common_init ( void )
{
	// ensures the DWORD to INT conversion always produces a positive value
	SetLocalTimerReset();

	// no more blue, only black
	BackdropColor ( 0 );

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
		//DLLLoad (  "user32.dll",1 );
			t.ScreenW = GetSystemMetrics ( 0 );
			t.ScreenH= GetSystemMetrics ( 1 );
		//DLLDelete (  1 );
	}

	//  alpha v1.6 call kernel32 dll for better performance freq
	t.Kernel32 =1;
	//DLLLoad (  "kernel32.dll",t.Kernel32 );
	//t.memptr = malloc (16 );
	//CallDLL (  t.Kernel32, "QueryPerformanceFrequency", t.memptr );
	// C++ CONVERSION - commented this next bit out as memptr gets replaced straight after anyway, tried gg dbpro with this whole bit commented out and it ran fine
	//QueryPerformanceFrequency ( &t.memptr );
	t.memptr = PerformanceTimer();
	//DLLDelete (  t.Kernel32 );

	//  Activate RealSense if available
	///realsense_init ( );

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

			/* else regular operations
			// ONLY if not educational equipment based (not FPGC)
			if (  PathExist("files\\gamecore\\guns") == 1 ) 
			{
				//  FPSC - Shooter Genre
				g.fpgchud_s="gun";
				g.fpgchuds_s="guns";
				g.fpgcgenre=1;
			}
			*/
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
	g.timestampactivitymemthen=SMEMAvailable(1);

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
	//Dim (  t.team_frags,0  );
	//Undim ( t.team_frags );
	//  V109 BETA3 - added to control change to player jump height
	g.playerdefaultjumpheight = 50.0f;

	//  V118 - store range, aspect and fov globally!
	g.realrange_f = 9000.0f;
	g.realaspect_f = 4.0f/3.0f;
	g.realfov_f = 75.0f;
	
	//t.saveload as saveloadtype;
	Dim (  t.saveloadslot_s,9  );
	//t.saveloadgameposition as saveloadgamepositiontype;
	Dim (  t.saveloadgamepositionplayerinventory,100  );
	Dim (  t.saveloadgamepositionplayerobjective,99 );
	Dim (  t.saveloadgamepositionentity,g.entityelementmax  );
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

	ExitProcess ( 0 );
}

// C++ CONVERSION: Dave - New function to initialise all globals that were previously set up in types outside of any function/subroutine
void common_init_globals ( void )
{
	// set all variables to 0
	//C++ISSUE - set all variables to 0 and leave the strings lone, dont do memset coz its nuking the strings too
	//memset ( &g, 0, sizeof ( g ) );
	//memset ( &t, 0, sizeof ( t ) );

	//  Grab current folder
	g.fpscrootdir_s = GetDir();
	g.mydocumentsdir_s = Mydocdir();
	g.mydocumentsdir_s += "\\";
	g.myfpscfiles_s = "Game Guru Files";
	g.myownrootdir_s = g.mydocumentsdir_s+g.myfpscfiles_s+"\\";

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
	//PE: 50000+ to be used for internal images inside dbo's.
	g.internalshadowdynamicterrain = 59950;
	g.internalshadowdebugimagestart = 59951;
	g.internalocclusiondebugimagestart = 59961;
	g.conkitimagebankoffset = 60000;
	g.tempimageoffset = 63000;
	g.widgetimagebankoffset = 63100;
	g.huddamageimageoffset = 63200;
	g.characterkitimageoffset = 63250;
	g.importermenuimageoffset = 63400;
	g.slidersmenuimageoffset = 63500;
	g.terrainimageoffset = 63600;
	g.gamehudimagesoffset = 64700;
	g.editorimagesoffset = 65110;
	g.editordrawlastimagesoffset = 75100;
	g.interactiveimageoffset = 75110;
	g.explosionsandfireimagesoffset = 85000;
	g.bitmapfontimagetart = 89500;
	g.panelimageoffset = 90000;
	g.charactercreatorimageoffset = 90020;
	g.charactercreatorEditorImageoffset = 95000;
	g.LUAImageoffset = 96000;
	g.LUAImageoffsetMax = 105999;

	// Sprite ( Resource markers )
	g.ammopanelsprite = 63400;
	g.healthpanelsprite = 63401;
	g.steamchatpanelsprite = 63410;
	g.LUASpriteoffset = 63500;
	g.LUASpriteoffsetMax = 73499;
	g.ebeinterfacesprite = 74001;
	g.terrainpainterinterfacesprite = 74051;

	//  Mesh Resources
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
	g.thirdpersonentityeffect = 1298;
	g.thirdpersoncharactereffect = 1299;
	g.charactercreatoreffectbankoffset = 1300;

	//  Sound Resources
	g.soundbankoffset = 1;
	g.soundbankoffsetfinish = 8799;
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
	g.waypointdetectworkobject = 3498;
	g.entityworkobjectoffset = 3499;
	g.entityattachmentsoffset = 3500;
	g.entityattachmentindex = 0;
	g.prompt3dobjectoffset = 5999;
	g.terrainobjectoffset = 6000;
	g.hudlayersbankoffset = 16000;
	//  +1 ; jetpack1
	g.hudbankoffset = 16050;
	g.gunbankoffset = 16100;
	g.brassbankoffset = 16150;
	g.smokebankoffset = 16200;
	g.decalbankoffset = 16450;
	g.decalelementoffset = 16500;
	g.fragmentobjectoffset = 17000;
	g.explodedecalobjstart = 17500;
	g.characterkitobjectoffset = 17800;
	g.shadowdebugobjectoffset = 17890;
	g.importermenuobjectoffset = 17900;
	g.editorwaypointoffset = 18001;
	g.editorwaypointoffsetmax = 18499;
	g.debugobjectoffset = 18500;
	g.gamerealtimeobjoffset = 19300;
	g.gamerealtimeobjoffsetmax = 27999;
	g.conkitobjectbankoffset = 28000; // CONKIT REDUNDANT
	//  DON'T insert a new value in here. Conkit uses entitybankoffset as it's upper limit for object IDs
	g.entitybankoffset = 50000;
	g.temporarymeshobject = 65500;
	g.temporarydarkaiobject = 65535;
	g.projectorsphereobjectoffset = 69499;
	g.tempobjectoffset = 69500;
	g.instancestampworkobject = 69991;
	g.darkaiobsboxobject = 69992;
	g.tempimporterlistobject = 69993;
	g.entityviewcursorobj = 70000;
	g.entityviewstartobj = 70001;
	g.entityviewendobj = 0;
	g.entityviewcurrentobj = g.entityviewstartobj;
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
	g.batchobjectoffset = 160001;
	g.explosionsandfireobjectoffset = 170001;
	g.raveyparticlesobjectoffset = 180001;
	g.ebeobjectbankoffset = 189901;
	g.occlusionboxobjectoffset = 190001;
	g.occlusionboxobjectoffsetfinish = 199999;
	g.steamplayermodelsoffset = 200000;
	g.charactercreatorrmodelsbankoffset = 200000;
	g.charactercreatorrmodelsoffset = 201000;
	g.charactercreatorrmodelsoffsetEnd = 203000;

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
	g.gunbankmaxlimit = 1000;
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

	Dim (  t.steamworks_respawn_timed,STEAM_RESPAWN_TIME_OBJECT_LIST_SIZE  );

	Dim (  t.steamworks_destroyedObjectList,STEAM_DESTROYED_OBJECT_LIST_SIZE  );
	Dim (  t.steamworks_bullets,160   );
	Dim (  t.steamworks_bullets_send_time,160  );
	Dim (  t.steamworks_attachmentobjects,100   );
	Dim (  t.steamworks_gunobj,100   );
	Dim (  t.steamworks_gunname,100  );
	Dim (  t.steamworks_team,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_joined,STEAM_MAX_NUMBER_OF_PLAYERS );
	Dim (  t.steamworks_kills,STEAM_MAX_NUMBER_OF_PLAYERS   );
	Dim (  t.steamworks_deaths,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_lastIdleY,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_lastIdleReset,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_reload,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerShooting,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerAttachmentIndex,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerIsRagdoll,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerAttachmentObject,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerHasSpawned,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_oldAppearance,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playingAnimation,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playingRagdoll,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_oldplayerx,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_oldplayery,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_oldplayerz,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_meleePlaying,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_jetpackparticles,STEAM_MAX_NUMBER_OF_PLAYERS  );
	// `dim steamworks_isIdling(STEAM_MAX_NUMBER_OF_PLAYERS) as integer

	Dim (  t.steamworks_isDying,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_jetpackOn,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_lobbies_s,STEAM_MAX_NUMBER_OF_LOBBIES  );
	Dim (  t.steamworks_playerEntityID,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_forcePosition,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_health,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_chat,STEAM_MAX_CHAT_LINES  );
	Dim (  t.steamworks_subbedItems,20  );
	Dim (  t.steamworks_playerAvatars_s,STEAM_MAX_NUMBER_OF_PLAYERS  );
	Dim (  t.steamworks_playerAvatarOwners_s,STEAM_MAX_NUMBER_OF_PLAYERS  );

	Dim (  t.steamworksmultiplayerstart,STEAM_MAX_NUMBER_OF_PLAYERS );

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

	//t.importer as importertype;

	//t.lightmapper as lightmappertype;

	//t.mooreneighborhood as mooreneighborhoodtype;
	//t.mooreneighborhood.mode=0;

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
	// now set in ragdoll_setcollisionmask
	//g.tphys_CollisionGroup.group0 = 1;
	//g.tphys_CollisionGroup.group1 = 2;
	//g.tphys_CollisionGroup.group2 = 4;
	//g.tphys_CollisionMask.mask0 = 1;
	//g.tphys_CollisionMask.mask1 = 2;
	//g.tphys_CollisionMask.mask2 = 4;
	g.RagdollsMax = 0;
	Dim (  t.Ragdolls,g.RagdollsMax );
	t.Ragdolls[g.RagdollsMax].obj=0;

	//  Hockeykid - 250610 - Ai Factions
	g.FactionArrayMax = 20;
	g.mutualfactionoff = 0;

	//t.decalglobal as decalglobaltype;

	Dim (  t.material , 100 );
	g.gmaterialmax = 0;

	g.gentityprofileversion = 20100721;

	//t.grideleprof as entityeleproftype;

	Dim2(  t.charactergunpose,100, 36  );

	//t.gunandmelee as gunandmeleetype;

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
	Dim2 ( t.gunsound,g.maxgunsinengine, 15 );
	Dim3 ( t.gunsoundcompanion,g.maxgunsinengine, 15, 2 );
	Dim2 ( t.gunsounditem,g.maxgunsinengine, 50  );

	Dim (  t.brassfallcount_f,30 );
	g.autoloadgun = 0;
	g.gunslotmax = 0;

	//t.playercheckpoint as playercheckpointtype;
	Dim (  t.soundloopcheckpoint,65535 );

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
	//t.slidersmenunames as slidersmenunamestype;
	g.slidersmenumax = 0;
	Dim (  t.slidersmenu,20   );
	Dim2(  t.slidersmenuvalue,20, 20 );
	g.sliderspecialview = 0;
	g.slidersmenufreshclick = 0;
	g.slidersmenudropdownscroll_f = 1;

	Dim (  t.nearestlightindex,4  );

	//t.inputsys as inputsystemtype;

	//t.editorfreeflight as editorfreeflighttype;
	t.editorfreeflight.mode=0;

	g.objmetamax = 500000;
	Dim (  t.objmeta,g.objmetamax );

	//t.objitem as objofinteresttype;
	Dim (  t.objinterestlist, 1   );
	Undim ( t.objinterestlist );

	//  Global data structure to handle real time light mapping control
	//t.rtlm as realtimelightmappingtype;

	//  Populate Infini Lights On The Fly In The Map Editor (see _realtimelightmapping_refreshinfiniwithe)
	Dim (  t.infinilight,0 );
	Undim ( t.infinilight );
	g.infinilightmax = 0;

	Dim2(  t.bitmapfont,10, 255 );

	//t.effectparam as effectparamtype;
	Dim (  t.effectparamarray,1300 );

	Dim2( t.terrainmatrix ,terrain_chunk_size+3 , terrain_chunk_size+3 );
	t.terrain.waterlineyadjustforclip_f=0.0f;
	t.terrain.adjaboveground_f=30.0; // 070116 - caued spawned entity drop from sky 50.0f
	t.terrain.superflat=0;

	// 100417 - actually used for terrain default generation
	t.terrain.waterliney_f = 0;

	//C++ CONVERSION - added one to the chunk size as it is going out of bounds
	// even tho i dim 1 more than is needed, it is going 3 over
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
	Dim (  t.skybank_s,1 );

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

	//t.aisystem as aisystemtype;
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
	//  +3100 - building object
	//  +4001/5000 - visual character VWEAP object
	t.aisystem.debugentitymesh=t.aisystem.objectstartindex+3001;
	t.aisystem.debugentitymesh2=t.aisystem.objectstartindex+3002;
	t.aisystem.debugentityworkobj=t.aisystem.objectstartindex+3003;
	t.aisystem.debugentityworkobj2=t.aisystem.objectstartindex+3004;
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

	//t.playercontrol as playercontroltype;
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

	//t.playerlight as playerlighttype;

	Dim (  t.musictrack,MUSICSYSTEM_MAXTRACKS );

	//t.audioVolume as taudiovolume;

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
	
	// `Explosions and fire

	
	
	
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
	//  place holder object pointers
	//  place holder for emitter sounds
	//  make and prepare particle and debris objects
	
	Dim (  t.ravey_particle_emitters,RAVEY_PARTICLE_EMITTERS_MAX  );

	Dim (  t.ravey_particles,RAVEY_PARTICLES_MAX  );
	g.ravey_particles_next_particle = 0;
	g.ravey_particles_old_time = 0;
	g.ravey_particles_time_passed = 0;

	//Work globals common to all and require early declaration
	

	//  Setup work floats (for HUD decimal detail)
	//t.value1 as float;
	//t.value2 as float;
	//t.value3 as float;
	//t.workhudx as float;
	//t.workhudy as float;
	//t.workhudz as float;


}

//  Subroutine to completely construct FPSCData
void FPSC_Full_Data_Init ( void )
{


//Performance globals


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

//  Test globals until finalise code
g.leedebugvalue_s = "";
// `global universalshaderindex=0



//Map Data


//  Define 50Kx50K Area (500*100)
t.maxx=500 ; t.maxy=500;

//  Visible-Col-Map used for per-cycle quick entity collision checks
t.viscolx=160 ; t.viscoly=20 ; t.viscolz=160;
Dim3(  t.viscolmap,t.viscolx, t.viscoly, t.viscolz  );

//  Prepare entity reference map (references entityelementlist indexes)
Dim3(  t.refmap,t.layermax, t.maxx, t.maxy );
//t.newptrbase as DWORD;
//t.refptrbase as DWORD;
//t.ptrbase as DWORD;
//t.refptr as DWORD;
//t.ptr as DWORD;

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

//  Grid Entity globals
//t.gridentitylight as entitylighttype;
//t.gridentitytrigger as entitytriggertype;

//  resource counter to help prevent kids adding crazy amounts of stuff
g.editorresourcecounter_f = 0;
g.editorresourcecounterpacer = 0;

g.animmax = 700;
g.footfallmax = 200;
g.entidmastermax = 100;
//Dave fix - 100 was not enough for some stress test levels
Dim2(  t.entityphysicsbox,MAX_ENTITY_PHYSICS_BOXES*2, MAX_ENTITY_PHYSICS_BOXES  );
Dim2(  t.entitybodypart,100, 100  );
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
Dim (  t.entitydebug_s,g.entityelementmax  );

//  New entity based shader variable array
g.globalselectedshadermax = 4;
g.globalselectedshadervar = 1;
Dim2(  t.entityshadervar,g.entityelementmax, g.globalselectedshadermax  );

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

Dim (  t.aiactionseq,10000 );
Dim (  t.aiaction,500 );
Dim (  t.actstring_s,g.actstringmax  );

//  AI conditions
Dim (  t.aiconditionseq,10000  );
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
//t.guivisualsettings as guivisualsettingstype;
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

//  End of FULL-DATA-INIT Subroutine
return;


//Initialise using SETUP.INI


}

void FPSC_SetDefaults ( void )
{
	//  Very first task is find and load BUILD.INI (if flagged)
	g.gcompilestandaloneexe = 0;
	g.gpretestsavemode = 0;

	//  Find and load SETUP.INI settings as global states
	g.grealgameviewstate = 0;
	g.gmultiplayergame = 0;
	g.gdebugreportmodestate = 0;
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
	g.gvrmodeoriginal = 0;
	g.gvrmodemag = 100;
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
	g.gpbroverride = 0;
	g.memskipwatermask = 0;
	g.standalonefreememorybetweenlevels = 0;
	g.lowestnearcamera = 6; // default , use setup.ini lowestnearcamera to adjust.
	g.memgeneratedump = 0;
	g.underwatermode = 0;
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
	g.ghideallhuds = 0;
	g.gskipobstaclecreation = 0;
	g.gskipterrainobstaclecreation = 0;
	g.gdeletetxpcachesonexit = 0;
	g.gdisablesurfacesnap = 0;
	g.gdefaultterrainheight = 600;
	g.gdefaultwaterheight = 500;
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

void FPSC_LoadSETUPINI ( void )
{
	//  SETUP Info
	t.tfile_s=g.setupfilename_s;
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
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

					//  new fields for GameGuru (also need to trim out old fields!!)
					t.tryfield_s = "superflatterrain" ; if (  t.field_s == t.tryfield_s  )  t.terrain.superflat = t.value1;
					t.tryfield_s = "riftmode" ; if (  t.field_s == t.tryfield_s  )  g.globals.riftmode = t.value1;
					t.tryfield_s = "smoothcamerakeys" ; if (  t.field_s == t.tryfield_s  )  g.globals.smoothcamerakeys = t.value1;
					t.tryfield_s = "memorydetector" ; if (  t.field_s == t.tryfield_s  )  g.globals.memorydetector = t.value1;
					t.tryfield_s = "occlusionmode" ; if (  t.field_s == t.tryfield_s  )  g.globals.occlusionmode = t.value1;
					t.tryfield_s = "occlusionsize" ; if (  t.field_s == t.tryfield_s  )  g.globals.occlusionsize = t.value1;
					t.tryfield_s = "obstacleradius" ; if (  t.field_s == t.tryfield_s  )  t.aisystem.obstacleradius = t.value1;
					t.tryfield_s = "showdebugcollisonboxes" ; if (  t.field_s == t.tryfield_s  ) g.globals.showdebugcollisonboxes = t.value1;
					t.tryfield_s = "hideebe" ; if (  t.field_s == t.tryfield_s  ) g.globals.hideebe = t.value1;
					t.tryfield_s = "hidedistantshadows" ; if (  t.field_s == t.tryfield_s  ) g.globals.hidedistantshadows = t.value1;
					t.tryfield_s = "realshadowresolution" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowresolution = t.value1;
					t.tryfield_s = "realshadowcascadecount" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascadecount = t.value1;

					if (g.globals.realshadowcascadecount < 2) g.globals.realshadowcascadecount = 2; //PE: Limit cascades.
					if (g.globals.realshadowcascadecount > 8) g.globals.realshadowcascadecount = 8; //PE: Limit cascades.

					t.tryfield_s = "realshadowcascade0" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[0] = t.value1;
					t.tryfield_s = "realshadowcascade1" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[1] = t.value1;
					t.tryfield_s = "realshadowcascade2" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[2] = t.value1;
					t.tryfield_s = "realshadowcascade3" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[3] = t.value1;
					t.tryfield_s = "realshadowcascade4" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[4] = t.value1;
					t.tryfield_s = "realshadowcascade5" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[5] = t.value1;
					t.tryfield_s = "realshadowcascade6" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[6] = t.value1;
					t.tryfield_s = "realshadowcascade7" ; if (  t.field_s == t.tryfield_s  ) g.globals.realshadowcascade[7] = t.value1;

					t.tryfield_s = "realshadowsize0"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[0] = t.value1;
					t.tryfield_s = "realshadowsize1"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[1] = t.value1;
					t.tryfield_s = "realshadowsize2"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[2] = t.value1;
					t.tryfield_s = "realshadowsize3"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[3] = t.value1;
					t.tryfield_s = "realshadowsize4"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[4] = t.value1;
					t.tryfield_s = "realshadowsize5"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[5] = t.value1;
					t.tryfield_s = "realshadowsize6"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[6] = t.value1;
					t.tryfield_s = "realshadowsize7"; if (t.field_s == t.tryfield_s) g.globals.realshadowsize[7] = t.value1;

					t.tryfield_s = "realshadowdistance"; if (t.field_s == t.tryfield_s) {
						g.globals.realshadowdistance = t.value1;
						g.globals.realshadowdistancehigh = t.value1;
					}
					t.tryfield_s = "editorusemediumshadows"; if (t.field_s == t.tryfield_s)  g.globals.editorusemediumshadows = t.value1;
					
					t.tryfield_s = "hidememorygauge" ; if (  t.field_s == t.tryfield_s  )  g.ghidememorygauge = t.value1;
					t.tryfield_s = "hidelowfpswarning" ; if (  t.field_s == t.tryfield_s  )  g.globals.hidelowfpswarning = t.value1;
					t.tryfield_s = "hardwareinfomode" ; if (  t.field_s == t.tryfield_s  )  g.ghardwareinfomode = t.value1;
					t.tryfield_s = "profileinstandalone" ; if (  t.field_s == t.tryfield_s  )  g.gprofileinstandalone = t.value1;
					t.tryfield_s = "allowfragmentation" ; if (  t.field_s == t.tryfield_s  )  t.game.allowfragmentation = t.value1;
					t.tryfield_s = "reflectionrendersize" ; if (  t.field_s == t.tryfield_s  )  g.greflectionrendersize = t.value1;
					t.tryfield_s = "ignoretitlepage" ; if (  t.field_s == t.tryfield_s  )  t.game.ignoretitle = t.value1;
					t.tryfield_s = "deactivateconkit" ; if (  t.field_s == t.tryfield_s  )  g.globals.deactivateconkit = t.value1;
					t.tryfield_s = "disablefreeflight" ; if (  t.field_s == t.tryfield_s  )  g.globals.disablefreeflight = t.value1;
					t.tryfield_s = "fulldebugview" ; if (  t.field_s == t.tryfield_s  )  g.globals.fulldebugview = t.value1;
					t.tryfield_s = "enableplrspeedmods" ; if (  t.field_s == t.tryfield_s  )  g.globals.enableplrspeedmods = t.value1;
					t.tryfield_s = "disableweaponjams" ; if (  t.field_s == t.tryfield_s  )  g.globals.disableweaponjams = t.value1;

					//  Control of display mode
					t.tryfield_s = "adapterordinal" ; if (  t.field_s == t.tryfield_s  )  g.gadapterordinal = t.value1;
					t.tryfield_s = "hideallhuds" ; if (  t.field_s == t.tryfield_s  )  g.ghideallhuds = t.value1;
					t.tryfield_s = "skipobstaclecreation" ; if (  t.field_s == t.tryfield_s  )  g.gskipobstaclecreation = t.value1;
					t.tryfield_s = "skipterrainobstaclecreation" ; if (  t.field_s == t.tryfield_s  )  g.gskipterrainobstaclecreation = t.value1;
					t.tryfield_s = "vsync" ; if (  t.field_s == t.tryfield_s  )  g.gvsync = t.value1;
					t.tryfield_s = "fullscreen" ; if (  t.field_s == t.tryfield_s  )  g.gfullscreen = t.value1;
					t.tryfield_s = "width" ; if (  t.field_s == t.tryfield_s ) g.gdisplaywidth = t.value1 ; t.newwidth = t.value1 ; g.gsetupwidth = t.value1;
					t.tryfield_s = "height" ; if (  t.field_s == t.tryfield_s ) g.gdisplayheight = t.value1 ; t.newheight = t.value1 ; g.gsetupheight = t.value1;
					t.tryfield_s = "depth" ; if (  t.field_s == t.tryfield_s ) g.gdisplaydepth = t.value1 ; t.newdepth = t.value1 ; g.gsetupdepth = t.value1;
					t.tryfield_s = "aspectratio" ; if (  t.field_s == t.tryfield_s ) g.gaspectratio = t.value1 ; t.newaspectratio = t.value1;
	
					//  collect data from fields in setup file
					t.tryfield_s = "realgameview" ; if (  t.field_s == t.tryfield_s  )  g.grealgameviewstate = t.value1;
					t.tryfield_s = "multiplayergame" ; if (  t.field_s == t.tryfield_s  )  g.gmultiplayergame = t.value1;
					t.tryfield_s = "debugreport" ; if (  t.field_s == t.tryfield_s  )  g.gdebugreportmodestate = t.value1;
					t.tryfield_s = "exitpromptreport" ; if (  t.field_s == t.tryfield_s  )  g.gexitpromptreportmodestate = t.value1;
					t.tryfield_s = "debugphysics" ; if (  t.field_s == t.tryfield_s  )  g.gdebugphysicsstate = t.value1;
					t.tryfield_s = "debugreportstepthrough" ; if (  t.field_s == t.tryfield_s  )  g.gdebugreportstepthroughstate = t.value1;
					t.tryfield_s = "showentitygameinfo" ; if (  t.field_s == t.tryfield_s  )  g.gshowentitygameinfostate = t.value1;
					t.tryfield_s = "showdebugtextingame" ; if (  t.field_s == t.tryfield_s  )  g.gshowdebugtextingamestate = t.value1;
					t.tryfield_s = "includeonlyvideo" ; if (  t.field_s == t.tryfield_s  )  g.gincludeonlyvideo = t.value1;
					t.tryfield_s = "includeonlyname" ; if (  t.field_s == t.tryfield_s  )  g.gincludeonlyname_s = t.value_s;
					t.tryfield_s = "ignorefastbone" ; if (  t.field_s == t.tryfield_s  )  g.gignorefastbone = t.value1;
					t.tryfield_s = "loadreport" ; if (  t.field_s == t.tryfield_s  )  g.gloadreportstate = t.value1;

					t.tryfield_s = "optimizemode" ; if (  t.field_s == t.tryfield_s  )  g.goptimizemode = t.value1;
					t.tryfield_s = "lightmapping" ; if (  t.field_s == t.tryfield_s  )  g.glightmappingstate = t.value1;
					t.tryfield_s = "lightmapsize" ; if (  t.field_s == t.tryfield_s  )  t.glightmapsize = t.value1;
					t.tryfield_s = "lightmapquality" ; if (  t.field_s == t.tryfield_s  )  t.glightmapquality = t.value1;
					t.tryfield_s = "lightmapold" ; if (  t.field_s == t.tryfield_s  )  g.glightmappingold = t.value1;
					t.tryfield_s = "lightmapshadows" ; if (  t.field_s == t.tryfield_s  )  g.glightshadowsstate = t.value1;
					t.tryfield_s = "lightmapambientr" ; if (  t.field_s == t.tryfield_s  )  g.glightambientr = t.value1;
					t.tryfield_s = "lightmapambientg" ; if (  t.field_s == t.tryfield_s  )  g.glightambientg = t.value1;
					t.tryfield_s = "lightmapambientb" ; if (  t.field_s == t.tryfield_s  )  g.glightambientb = t.value1;
					t.tryfield_s = "lightmapsunx" ; if (  t.field_s == t.tryfield_s  )  g.glightsunx = t.value1;
					t.tryfield_s = "lightmapsuny" ; if (  t.field_s == t.tryfield_s  )  g.glightsuny = t.value1;
					t.tryfield_s = "lightmapsunz" ; if (  t.field_s == t.tryfield_s  )  g.glightsunz = t.value1;
					t.tryfield_s = "lightmapsunr" ; if (  t.field_s == t.tryfield_s  )  g.glightsunr = t.value1;
					t.tryfield_s = "lightmapsung" ; if (  t.field_s == t.tryfield_s  )  g.glightsung = t.value1;
					t.tryfield_s = "lightmapsunb" ; if (  t.field_s == t.tryfield_s  )  g.glightsunb = t.value1;
					t.tryfield_s = "lightmapzerorange" ; if (  t.field_s == t.tryfield_s  )  g.glightzerorange = t.value1;
					t.tryfield_s = "lightmapatten" ; if (  t.field_s == t.tryfield_s  )  g.glightatten = t.value1;
					t.tryfield_s = "lightmapmaxsize" ; if (  t.field_s == t.tryfield_s  )  g.glightmaxsize = t.value1;
					t.tryfield_s = "lightmapboost" ; if (  t.field_s == t.tryfield_s  )  g.glightboost = t.value1;
					t.tryfield_s = "lightmaptexsize" ; if (  t.field_s == t.tryfield_s  )  g.glighttexsize = t.value1;
					t.tryfield_s = "lightmapquality" ; if (  t.field_s == t.tryfield_s  )  g.glightquality = t.value1;
					t.tryfield_s = "lightmapblurmode" ; if (  t.field_s == t.tryfield_s  )  g.glightblurmode = t.value1;
					t.tryfield_s = "lightmapthreadmax" ; if (  t.field_s == t.tryfield_s  )  g.glightthreadmax = t.value1;
					t.tryfield_s = "bloodonfloor" ; if (  t.field_s == t.tryfield_s  )  g.gbloodonfloor = t.value1;
					t.tryfield_s = "imageblockmode" ; if (  t.field_s == t.tryfield_s  )  g.gimageblockmode = t.value1;

					t.tryfield_s = "showalluniquetextures" ; if (  t.field_s == t.tryfield_s  )  g.gshowalluniquetextures = t.value1;
					t.tryfield_s = "systemmemorycapoff" ; if (  t.field_s == t.tryfield_s  )  g.gsystemmemorycapoff = t.value1;
					t.tryfield_s = "entitytogglingoff" ; if (  t.field_s == t.tryfield_s  )  g.gentitytogglingoff = t.value1;
					t.tryfield_s = "extracollisionbuilddisabled" ; if (  t.field_s == t.tryfield_s  )  g.gextracollisionbuilddisabled = t.value1;
					t.tryfield_s = "alwaysconfirmsave" ; if (  t.field_s == t.tryfield_s  )  g.galwaysconfirmsave = t.value1;
					t.tryfield_s = "simplifiedcharacterediting" ; if (  t.field_s == t.tryfield_s  )  g.gsimplifiedcharacterediting = t.value1;
					t.tryfield_s = "useoggoff" ; if (  t.field_s == t.tryfield_s  )  g.guseoggoff = t.value1;
					t.tryfield_s = "cullmode" ; if (  t.field_s == t.tryfield_s  )  g.cullmode = t.value1;
					t.tryfield_s = "capfpson" ; if (  t.field_s == t.tryfield_s  )  g.gcapfpson = t.value1;
					t.tryfield_s = "disabledynamicres" ; if (  t.field_s == t.tryfield_s  ) 
					{
						if ( t.value1 == 1 )
							t.DisableDynamicRes = true;
						else
							t.DisableDynamicRes = false;
					}
					t.tryfield_s = "deletetxpcachesonexit" ; if (  t.field_s == t.tryfield_s  )  g.gdeletetxpcachesonexit = t.value1;
					t.tryfield_s = "disablesurfacesnap" ; if (  t.field_s == t.tryfield_s  )  g.gdisablesurfacesnap = t.value1;
					t.tryfield_s = "defaultterrainheight" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultterrainheight = t.value1;
					t.tryfield_s = "defaultwaterheight" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultwaterheight = t.value1;
					t.tryfield_s = "defaultebegridoffsetx" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultebegridoffsetx = t.value1;
					t.tryfield_s = "defaultebegridoffsetz" ; if (  t.field_s == t.tryfield_s  )  g.gdefaultebegridoffsetz = t.value1;

					t.tryfield_s = "xbox" ; if (  t.field_s == t.tryfield_s  )  g.gxbox = t.value1;
					t.tryfield_s = "xboxinvert" ; if (  t.field_s == t.tryfield_s  )  g.gxboxinvert = t.value1;
					t.tryfield_s = "xboxcontrollertype" ; if (  t.field_s == t.tryfield_s  )  g.gxboxcontrollertype = t.value1;
					t.tryfield_s = "xboxmag" ; if (  t.field_s == t.tryfield_s  )  g.gxboxmag = (0.0+t.value1)/100.0;
					t.tryfield_s = "mousesensitivity" ; if (  t.field_s == t.tryfield_s ) g.gmousesensitivity = t.value1  ; t.newmousesensitivity = t.value1;

					// VRMode
					// 0 : off
					// 1 : VR920/iWear
					// 5 : detects VR920/iWear (switches OFF if not found)
					// 6 : special case, side by side rendering
					t.tryfield_s = "vrmode" ; if (  t.field_s == t.tryfield_s  )  { g.gvrmode = t.value1; g.gvrmodeoriginal = t.value1; }
					t.tryfield_s = "vrmodemag" ; if (  t.field_s == t.tryfield_s  )  g.gvrmodemag = t.value1;

					t.tryfield_s = "dynamiclighting" ; if (  t.field_s == t.tryfield_s  )  g.gdynamiclightingstate = t.value1;
					t.tryfield_s = "dynamicshadows" ; if (  t.field_s == t.tryfield_s ) g.gdynamicshadowsstate = t.value1  ; t.newdynamicshadows = t.value1;
					t.tryfield_s = "dividetexturesize" ; if (  t.field_s == t.tryfield_s ) g.gdividetexturesize = t.value1  ; t.newdividetexturesize = t.value1;
					t.tryfield_s = "producelogfiles" ; if (  t.field_s == t.tryfield_s  )  g.gproducelogfiles = t.value1;
					t.tryfield_s = "pbroverride" ; if (  t.field_s == t.tryfield_s  )  g.gpbroverride = t.value1;
					t.tryfield_s = "underwatermode"; if (t.field_s == t.tryfield_s)  g.underwatermode = t.value1;
					t.tryfield_s = "memskipwatermask"; if (t.field_s == t.tryfield_s)  g.memskipwatermask = t.value1;
					t.tryfield_s = "standalonefreememorybetweenlevels"; if (t.field_s == t.tryfield_s)  g.standalonefreememorybetweenlevels = t.value1;
					t.tryfield_s = "lowestnearcamera"; if (t.field_s == t.tryfield_s)  g.lowestnearcamera = t.value1;
					
					t.tryfield_s = "memskipibr"; if (t.field_s == t.tryfield_s)  g.memskipibr = t.value1;
					t.tryfield_s = "memgeneratedump"; if (t.field_s == t.tryfield_s)  g.memgeneratedump = t.value1;
					
					t.tryfield_s = "producetruevidmemreading" ; if (  t.field_s == t.tryfield_s  )  g.gproducetruevidmemreading = t.value1;
					t.tryfield_s = "charactercapsulescale" ; if (  t.field_s == t.tryfield_s  )  g.gcharactercapsulescale_f = (t.value1+0.0)/100.0;
					t.tryfield_s = "hsrmode" ; if (  t.field_s == t.tryfield_s  )  g.ghsrmode = t.value1;
					t.tryfield_s = "newblossershaders" ; if (  t.field_s == t.tryfield_s  )  g.gnewblossershaders = t.value1;
					t.tryfield_s = "postprocessing" ; if (  t.field_s == t.tryfield_s ) g.gpostprocessing = t.value1  ; t.newpostprocessing = t.value1;
					t.tryfield_s = "showaioutlines" ; if (  t.field_s == t.tryfield_s  )  g.gshowaioutlines = t.value1;
					t.tryfield_s = "airadius" ; if (  t.field_s == t.tryfield_s  )  g.gairadius = t.value1;
					t.tryfield_s = "disablepeeking" ; if (  t.field_s == t.tryfield_s  )  g.gdisablepeeking = t.value1;
					t.tryfield_s = "antialias" ; if (  t.field_s == t.tryfield_s ) g.gantialias = t.value1  ; t.newantialias = t.value1;
					t.tryfield_s = "invmouse" ; if (  t.field_s == t.tryfield_s ) g.gminvert = t.value1  ; t.newmouseinvert = t.value1;
					t.tryfield_s = "disablerightmousehold" ; if (  t.field_s == t.tryfield_s  )  g.gdisablerightmousehold = t.value1;
					t.tryfield_s = "disableparticles" ; if (  t.field_s == t.tryfield_s ) g.gparticlesnotused = t.value1  ; t.newparticlesused = t.value1;
					t.tryfield_s = "autores" ; if (  t.field_s == t.tryfield_s ) g.gautores = t.value1  ; t.newautores = t.value1;
					t.tryfield_s = "terrainbrushsizemax"; if ( t.field_s == t.tryfield_s ) g.fTerrainBrushSizeMax = t.value1;
					t.tryfield_s = "allowcpuanimations"; if ( t.field_s == t.tryfield_s ) g.allowcpuanimations = t.value1;

					t.tryfield_s = "lightmappingquality"; if ( t.field_s == t.tryfield_s ) g.fLightmappingQuality = t.value1/100.0f;
					t.tryfield_s = "lightmappingblurlevel"; if ( t.field_s == t.tryfield_s ) g.fLightmappingBlurLevel = t.value1/100.0f;
					t.tryfield_s = "lightmappingsizeterrain"; if ( t.field_s == t.tryfield_s ) g.iLightmappingSizeTerrain = t.value1;
					t.tryfield_s = "lightmappingsizeentity"; if ( t.field_s == t.tryfield_s ) g.iLightmappingSizeEntity = t.value1;
					t.tryfield_s = "lightmappingsmoothangle"; if ( t.field_s == t.tryfield_s ) g.fLightmappingSmoothAngle = t.value1;
					t.tryfield_s = "lightmappingexcludeterrain"; if ( t.field_s == t.tryfield_s ) g.iLightmappingExcludeTerrain = t.value1;
					t.tryfield_s = "lightmappingdeactivatedirectionallight"; if ( t.field_s == t.tryfield_s ) g.iLightmappingDeactivateDirectionalLight = t.value1;
					t.tryfield_s = "lightmappingambientred"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientR = t.value1/100.0f;
					t.tryfield_s = "lightmappingambientgreen"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientG = t.value1/100.0f;
					t.tryfield_s = "lightmappingambientblue"; if ( t.field_s == t.tryfield_s ) g.fLightmappingAmbientB = t.value1/100.0f;
					t.tryfield_s = "lightmappingallterrainlighting"; if ( t.field_s == t.tryfield_s ) g.iLightmappingAllTerrainLighting = t.value1;
					t.tryfield_s = "suspendscreenprompts" ; if (  t.field_s == t.tryfield_s  )  g.gsuspendscreenprompts = t.value1;
					t.tryfield_s = "forceloadtestgameshaders" ; if (  t.field_s == t.tryfield_s  )  g.gforceloadtestgameshaders = t.value1;				

					t.tryfield_s = "usesky" ; if (  t.field_s == t.tryfield_s  )  g.guseskystate = t.value1;
					t.tryfield_s = "usefloor" ; if (  t.field_s == t.tryfield_s  )  g.gusefloorstate = t.value1;
					t.tryfield_s = "useenvsounds" ; if (  t.field_s == t.tryfield_s  )  g.guseenvsoundsstate = t.value1;
					t.tryfield_s = "useweapons" ; if (  t.field_s == t.tryfield_s  )  g.guseweaponsstate = t.value1;

					t.tryfield_s = "godmode" ; if (  t.field_s == t.tryfield_s  )  g.ggodmodestate = 0;
					t.tryfield_s = "uniquesignature" ; if (  t.field_s == t.tryfield_s  )  g.guniquesignature = t.value1;
					t.tryfield_s = "gameobjectivetype" ; if (  t.field_s == t.tryfield_s  )  g.ggameobjectivetype = t.value1;
					t.tryfield_s = "gameobjectivevalue" ; if (  t.field_s == t.tryfield_s  )  g.ggameobjectivevalue = t.value1;
					t.tryfield_s = "oneshotkills" ; if (  t.field_s == t.tryfield_s  )  g.goneshotkills = t.value1;
					t.tryfield_s = "maxplayers" ; if (  t.field_s == t.tryfield_s  )  g.numberofplayers = t.value1;
					t.tryfield_s = "spawnrandom" ; if (  t.field_s == t.tryfield_s  )  g.gspawnrandom = t.value1;
					t.tryfield_s = "uniquegamecode" ; if (  t.field_s == t.tryfield_s  )  g.guniquegamecode_s = t.value_s;

					t.tryfield_s = "useuniquelynamedentities" ; if (  t.field_s == t.tryfield_s  )  g.guseuniquelynamedentities = t.value1;
					t.tryfield_s = "exportassets" ; if (  t.field_s == t.tryfield_s  )  g.gexportassets = t.value1;
					t.tryfield_s = "localserver" ; if (  t.field_s == t.tryfield_s  )  g.glocalserveroverride_s = t.value_s;

					//  all FPI screens
					t.tryfield_s = "title" ; if (  t.field_s == t.tryfield_s  )  t.titlefpi_s = t.value_s;
					t.tryfield_s = "global" ; if (  t.field_s == t.tryfield_s  )  t.setupfpi_s = t.value_s;
					t.tryfield_s = "gamewon" ; if (  t.field_s == t.tryfield_s  )  t.gamewonfpi_s = t.value_s;
					t.tryfield_s = "gameover" ; if (  t.field_s == t.tryfield_s  )  t.gameoverfpi_s = t.value_s;
					t.tryfield_s = "levelfpi1" ; if (  t.field_s == t.tryfield_s ) t.loadingfpi_s == t.value_s ; t.levelfpiinsetup = t.l;

					////  TDM - Plystire
					t.tryfield_s = "hudr" ; if (  t.field_s == t.tryfield_s  )  g.r_f = t.value1;
					t.tryfield_s = "hudg" ; if (  t.field_s == t.tryfield_s  )  g.g_f = t.value1;
					t.tryfield_s = "hudb" ; if (  t.field_s == t.tryfield_s  )  g.b_f = t.value1;
					t.tryfield_s = "autoswaptrue" ; if (  t.field_s == t.tryfield_s  )  g.autoswap = t.value1;
					t.tryfield_s = "messagetime" ; if (  t.field_s == t.tryfield_s  )  g.messagetime = t.value1;
					t.tryfield_s = "allowscope" ; if (  t.field_s == t.tryfield_s  )  g.allowscope_s = t.value1;
					t.tryfield_s = "serverhostname" ; if (  t.field_s == t.tryfield_s  )  g.serverhostname = t.value_s;
					t.tryfield_s = "alwaysrun" ; if (  t.field_s == t.tryfield_s  )  g.alwaysrun = t.value1;
					t.tryfield_s = "matchtype" ; if (  t.field_s == t.tryfield_s  )  g.multi_match_type = t.value1;
					t.tryfield_s = "multiradar";if (  t.field_s == t.tryfield_s  )  g.darkradar = t.value1;
					t.tryfield_s = "multicompass";;if (  t.field_s == t.tryfield_s  )  g.compassOn = t.value1;
					t.tryfield_s = "multicompassx";if (  t.field_s == t.tryfield_s  )  g.compassX = t.value1;
					t.tryfield_s = "multicompassy";if (  t.field_s == t.tryfield_s  )  g.compassY = t.value1;
					t.tryfield_s = "multiradarx";if (  t.field_s == t.tryfield_s  )  g.radarx = t.value1;
					t.tryfield_s = "multiradary";if (  t.field_s == t.tryfield_s  )  g.radary = t.value1;

					//  Levels
					if (  t.field_s == "levelmax"  )  g.glevelmax = t.value1;
					if (  g.glevelmax>0 ) 
					{
						for ( t.v = 1 ; t.v<=  g.glevelmax; t.v++ )
						{
							sprintf ( t.szwork , "levelfpm%s" , Str(t.v) );
							t.tryfield_s = t.szwork;
							if (  t.field_s == t.tryfield_s  )  t.levelfpm_s = t.value_s;
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

					//  new global key entries
					if (  t.field_s == "melee key"  )  g.ggunmeleekey = t.value1;
					if (  t.field_s == "switchtoalt" ) 
					{
						if ( t.value1 != 0 ) g.ggunaltswapkey1 = t.value1;
						g.ggunaltswapkey2 = t.value2;
						if (  t.value2 == 0  )  g.ggunaltswapkey2 = -1;
					}
					if (  t.field_s == "zoomholdbreath"  )  g.gzoomholdbreath = t.value1;

					//  all key actions
					for ( t.num = 1 ; t.num<=  11; t.num++ )
					{
						sprintf ( t.szwork , "key%s" , Str(t.num) );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.listkey[t.num] = t.value1;
					}
					for ( t.num = 1 ; t.num<=  11; t.num++ )
					{
						if (  t.num == 1  )  t.tryfield_s = "keyup";
						if (  t.num == 2  )  t.tryfield_s = "keydown";
						if (  t.num == 3  )  t.tryfield_s = "keyleft";
						if (  t.num == 4  )  t.tryfield_s = "keyright";
						if (  t.num == 5  )  t.tryfield_s = "keyjump";
						if (  t.num == 6  )  t.tryfield_s = "keycrouch";
						if (  t.num == 7  )  t.tryfield_s = "keyenter";
						if (  t.num == 8  )  t.tryfield_s = "keyreload";
						if (  t.num == 9  )  t.tryfield_s = "keypeekleft";
						if (  t.num == 10  )  t.tryfield_s = "keypeekright";
						if (  t.num == 11  )  t.tryfield_s = "keyrun";
						if (  t.field_s == t.tryfield_s  )  t.listkey[t.num] = t.value1;
					}

					//  all gun slots
					for ( t.num = 1 ; t.num<=  9; t.num++ )
					{
						sprintf ( t.szwork , "slot%i" , t.num );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.gunslots_s[t.num] = t.value_s;
					}

					for ( t.num = 1 ; t.num<=  30; t.num++ )
					{
						sprintf ( t.szwork , "taunt%i" , t.num );
						t.tryfield_s=t.szwork;
						if (  t.field_s == t.tryfield_s  )  t.taunt_s[t.num] = t.value_s;
					}

					//  localization data
					t.tryfield_s = "language" ; if (  t.field_s == t.tryfield_s  )  g.language_s = t.value_s;

					// get graphic option settings and store in global strings
					t.tryfield_s = "graphicslowterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowterrain_s = t.value_s;
					t.tryfield_s = "graphicslowentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowentity_s = t.value_s;
					t.tryfield_s = "graphicslowgrass" ; if (  t.field_s == t.tryfield_s  )  g.graphicslowgrass_s = t.value_s;
					t.tryfield_s = "graphicsmediumterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumterrain_s = t.value_s;
					t.tryfield_s = "graphicsmediumentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumentity_s = t.value_s;
					t.tryfield_s = "graphicsmediumgrass" ; if (  t.field_s == t.tryfield_s  )  g.graphicsmediumgrass_s = t.value_s;
					t.tryfield_s = "graphicshighterrain" ; if (  t.field_s == t.tryfield_s  )  g.graphicshighterrain_s = t.value_s;
					t.tryfield_s = "graphicshighentity" ; if (  t.field_s == t.tryfield_s  )  g.graphicshighentity_s = t.value_s;
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

	// special global flag which can affect how shaders are loaded
	if ( g.gforceloadtestgameshaders != 0 ) gbAlwaysIgnoreShaderBlobFile = true;
}

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

unsigned char common_shiftright ( unsigned char c )
{
	if ( c == 'Z' ) 
		c = 'A';
	else
		c = c + 1;
	return c;
}

unsigned char common_shiftleft ( unsigned char c )
{
	if ( c == 'A' ) 
		c = 'Z';
	else
		c = c - 1;
	return c;
}

unsigned char common_align ( unsigned char c )
{
	if ( c > 'Z' )
	{
		int overshot = (c - 'Z')-1;
		c = '0' + overshot;
	}
	return c;
}

unsigned char common_unalign ( unsigned char c )
{
	if ( c >= '0' && c <= '9' )
	{
		int overshot = (c - '0')+1;
		c = 'Z' + overshot;
	}
	return c;
}

void common_makeserialcode ( LPSTR pInstituteName, int iFromDD, int iFromMM, int iFromYY, int iToDD, int iToMM, int iToYY, LPSTR pBuildString )
{
	// create a dump of the data
	for ( int c = 0; c < 10; c++ )
	{
		if ( c < strlen(pInstituteName) )
			pBuildString[c] = pInstituteName[c];
		else
			pBuildString[c] = 74+c;
	}
	pBuildString[10] = 0;
	strupr ( pBuildString );
	pBuildString[10] = common_shiftright(pBuildString[1]);
	pBuildString[11] = common_align(66+iFromMM);
	pBuildString[12] = common_align(67+iFromDD);
	pBuildString[13] = common_shiftright(pBuildString[8]);
	pBuildString[14] = common_align(64+iToDD);
	pBuildString[15] = common_shiftright(pBuildString[7]);
	pBuildString[16] = common_align(69+iFromYY);
	pBuildString[17] = common_shiftright(pBuildString[5]);
	pBuildString[18] = common_align(65+iToMM);
	pBuildString[19] = common_shiftright(pBuildString[6]);
	pBuildString[20] = common_align(68+iToYY);
	pBuildString[21] = common_shiftright(pBuildString[3]);

	// return the result as a string
	pBuildString[22] = 0;
}

int common_isserialcodevalid ( LPSTR pSerialCode )
{
	// get serial code into build array
	char pBuildString[10+12+1];
	strcpy ( pBuildString, pSerialCode );

	// extract the DD, MM and YY data from build array
	int iFromYY = common_unalign(pBuildString[16])-69;
	int iToDD = common_unalign(pBuildString[14])-64;
	int iToMM = common_unalign(pBuildString[18])-65;
	int iToYY = common_unalign(pBuildString[20])-68;

	// check if system date within serial code date range
	time_t now = time(0);
    tm *ltm = localtime(&now);
	int iDay   = ltm->tm_mday;
	int iMonth = ltm->tm_mon;
	int iYear  = ltm->tm_year-100;

	// if within the years range of the code (allows any date within the last month)
	int iDateWithinRange = 0;
	if ( iYear >= iFromYY && iYear <= iToYY )
	{
		if ( iYear < iToYY )
		{
			// if not the final year
			iDateWithinRange = 1;
		}
		else
		{
			// if the final year
			if ( iYear == iToYY )
			{
				if ( iMonth <= iToMM )
				{
					// and not an expired month
					iDateWithinRange = 1;
				}
			}
		}
	}

	// return result
	return iDateWithinRange;
}

void common_writeserialcode ( LPSTR pCode )
{
	char pAbsFilePath[1024];
	strcpy ( pAbsFilePath, g.fpscrootdir_s.Get() );
	if ( g.vrqoreducontrolmode == 2 )
		strcat ( pAbsFilePath, "\\educontrolmode.ini" );
	else
		strcat ( pAbsFilePath, "\\vrqcontrolmode.ini" );
	if ( FileExist(pAbsFilePath) == 1 ) DeleteFile ( pAbsFilePath );
	if ( FileOpen(1) == 1 ) CloseFile (  1 );
	OpenToWrite ( 1, pAbsFilePath );
	WriteString ( 1, pCode );
	CloseFile ( 1 );
}

void FPSC_Setup ( void )
{
	// prepare all default values 
	FPSC_SetDefaults();

	// Determine if GAME or MAPEDITOR here, so can set display mode accordingly
	g.trueappname_s = "Guru-Game";
	if (strcmp(Lower(Right(Appname(), 18)), "guru-mapeditor.exe") == 0)
	{
		g.trueappname_s = "Guru-MapEditor";
	}

	//  Quick Start state
	g.gfirsttimerun = 0;
	g.gshowonstartup = 0;
	if ( g.trueappname_s == "Guru-MapEditor" ) 
	{
		t.tfile_s="showonstartup.ini";
		if ( FileOpen(1) == 1 ) CloseFile (  1 );
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			OpenToRead (  1,t.tfile_s.Get() );
			t.tshowonstart_s = ReadString (1);
			cstr videoPlayedMax_s = ReadString (1);
			int iVideoPlayedMax = atoi(videoPlayedMax_s.Get());
			for ( int n=0; n<10; n++ )
			{
				cstr videoPlayedItem_s = ReadString (1);
				g.videoMenuPlayed[n] = atoi(videoPlayedItem_s.Get());
			}
			CloseFile (  1 );
			g.gshowonstartup=ValF(t.tshowonstart_s.Get());
		}
		else
		{
			// first time run
			g.gfirsttimerun=1;
			g.gshowonstartup=0;
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, "0" );
			WriteString ( 1, "0" );
			for ( int n=0; n<10; n++ )
			{
				WriteString ( 1, "0" );
			}
			CloseFile (  1 );
		}
	}

	// 050416 - get parental control flag and any password
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

	// 050416 - get VRQUEST control flag and serial code
	g.vrqoreducontrolmode = 0;
	g.vrqcontrolmode = 0;
	g.vrqcontrolmodeserialcode = "";
	g.vrqTriggerSerialCodeEntrySystem = 0;
	g.iTriggerSoftwareToQuit = 0;
	t.tfile_s="vrqcontrolmode.ini";
	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		g.vrqoreducontrolmode = 1;
	}
	else
	{
		t.tfile_s="educontrolmode.ini";
		if ( FileExist(t.tfile_s.Get()) == 1 ) 
		{
			g.vrqoreducontrolmode = 2;
		}
	}
	if ( g.vrqoreducontrolmode > 0 )
	{
		// read serial code from VRQ controlmode file
		g.vrqcontrolmode = 1;
		if ( FileOpen(1) == 1 ) CloseFile (  1 );
		OpenToRead ( 1, t.tfile_s.Get() );
		g.vrqcontrolmodeserialcode = ReadString ( 1 );
		CloseFile ( 1 );

		// if VRQ, ensure serial code has not expired, otherwise ask for new serial code
		if ( strlen(g.vrqcontrolmodeserialcode.Get()) > 1 )
		{
			// determine FROM and TO dates from serial code
			if ( common_isserialcodevalid(g.vrqcontrolmodeserialcode.Get()) == 0 )
			{
				// serial code expired
				MessageBox ( NULL, "Your serial code has expired, obtain an updated serial code to continue using the software.", "License Not Found", MB_OK );
				g.vrqTriggerSerialCodeEntrySystem = 1;
				g.iTriggerSoftwareToQuit = 1;
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

		// all VRQ is restricted content mode
		g.quickparentalcontrolmode = 2;
	}
	else
	{
		// if non-VRQ, ensure Steam file present, otherwise exit software
		if ( g.trueappname_s == "Guru-MapEditor" ) 
		{
			if ( FileExist("steam_appid.txt") == 0 ) 
			{
				MessageBox ( NULL, "Root file missing from installation.", "System File Not Found", MB_OK );
				g.iTriggerSoftwareToQuit = 1;
			}
		}
	}

	//  Review Request Reminder state
	/*
	g.reviewRequestReminder = 0;
	g.reviewRequestMinuteCounter = 0;
	g.dwReviewRequestTimeStart = timeGetTime();
	t.tfile_s="reviewrequestreminder.ini";
	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		if ( FileOpen(1) == 1 ) CloseFile ( 1 );
		OpenToRead ( 1,t.tfile_s.Get() );
		t.reviewRequestReminder_s = ReadString ( 1 );
		t.reviewRequestMinuteCounter_s = ReadString ( 1 );
		CloseFile ( 1 );
		g.reviewRequestReminder = ValF(t.reviewRequestReminder_s.Get());
		g.reviewRequestMinuteCounter = ValF(t.reviewRequestMinuteCounter_s.Get());
	}
	*/

	//  Version Control - TEST GAME Mode
	g.gtestgamemodefromeditor = 0;
	g.gtestgamemodefromeditorokaypressed = 0;
	version_commandlineprompt ( );

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
	if (  strcmp ( Lower(Left(gRefCommandLineString,17)) , "-reloadstandalone" ) == 0 ) 
		g.iStandaloneIsReloading = 1;

	// Check and load SETUP.INI defaults
	FPSC_LoadSETUPINI();
	FPSC_LoadKEYMAP();

	// 250917 - set default CPU animation flag for engine
	if ( g.allowcpuanimations == 0 )
		SetDefaultCPUAnimState ( 3 );
	else
		SetDefaultCPUAnimState ( 0 );

	//  set adapter ordinal for next time display mode is set (below)
	if (  g.gadapterordinal>0 ) 
	{
		ForceAdapterOrdinal (  g.gadapterordinal );
	}

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

	//  The MyGames folder precedes Files
	g.exedir_s = GetDir();
	g.exedir_s += "\\MyGames\\";
	sprintf ( t.szwork , "The executable folder will be: %s" , g.exedir_s.Get() );
	timestampactivity(0,t.szwork);

	//  First task for any program is to enter the Files Folder
	t.tnopathprotomode=0;
	if (  PathExist("Files") == 1 ) 
		SetDir (  "Files" );
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
		if ( g.trueappname_s == "Guru-MapEditor" ) 
		{
			// MAP EDITOR MODE
			t.game.gameisexe=0;
			t.game.set.ismapeditormode=1;

			// set backbuffer for editor
			t.bkwidth=GetDesktopWidth() ; t.bkheight=GetDesktopHeight();
			t.thevrmodeflag = 0; 
			if ( g.gvrmode != 0  ) t.thevrmodeflag = 1;
			if ( t.thevrmodeflag != 0 ) 
			{
				SetDisplayModeVR ( GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(), g.gvsync,0,0,0,0,t.thevrmodeflag );
			}
			else
			{
				SetDisplayModeMODBACKBUFFER (  GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(),g.gvsync,0,0,t.bkwidth,t.bkheight );
			}

			// allow _e_ usage override
			if ( FileExist ( cstr(g.exeroot_s + cstr("\\leeandraveyrock.txt")).Get() ) == 1 )
				SetCanUse_e_(1);
		}
		else
		{
			//  STANDALONE GAME MODE
			t.game.gameisexe=1;
			t.game.onceonlyshadow=1;
			t.game.set.ismapeditormode=0;

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
						t.thevrmodeflag = 0 ; if (  g.gvrmode != 0  )  t.thevrmodeflag = 1;
						if ( t.thevrmodeflag != 0 ) // CheckDisplayMode(g.gdisplaywidth,g.gdisplayheight,g.gdisplaydepth) == 1 ) 
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
		welcome_init(1);
		welcome_animbackdrop();
		welcome_init(2);
		common_loadfonts();
		LPSTR pFirstTextToShow = g.version_s.Get();
		if ( g.grestoreeditorsettings == 1 ) pFirstTextToShow = "RESUMING PREVIOUS SESSION";
		welcome_updatebackdrop(pFirstTextToShow);
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

	//  Version control on EXE Building
	version_endofinit ( );

	//  Activate Steam (always so single player can do snapshots and get Steam notifications)
	steam_init ( );

	// Init default material sounds
	material_init ( );
	material_startup ( );


	// 
	//  LEAP POINT (detect if running as Guru-Game.exe or Guru-MapEditor.exe)
	// 
	sprintf ( t.szwork , "trueappname_s=%s" , g.trueappname_s.Get() );
	timestampactivity(0,t.szwork);
	if (  g.trueappname_s == "Guru-MapEditor" ) 
	{
		//  MAP EDITOR
		OpenToWrite (  1,"testwrite.dat" );
		WriteString (  1,"we can write to the Game Guru folder! Good." );
		CloseFile (  1 );
		if (  FileExist("testwrite.dat") == 1 ) 
		{
			timestampactivity(0,"We can write to the Game Guru folder!");
			DeleteAFile (  "testwrite.dat" );
		}
		else
		{
			ExitPrompt ( "Game Guru cannot write files to the Program Files area. Exit the software, right click on the Game Guru icon, and select 'Run As Administrator'", "Init Error" );
		}
	
		//  New security requires Steam client to be running (for ownership check)
		#ifdef STEAMOWNERSHIPCHECKFREEWEEKEND
		bool bSteamRunningAndGameGuruOwned = false;
		if ( g.steamworks.isRunning == 1 )
		{
			if ( SteamOwned() == true ) 
				bSteamRunningAndGameGuruOwned = true;
		}
		if ( bSteamRunningAndGameGuruOwned == false )
		{
			g.iTriggerSoftwareToQuit = 2;
		}
		#endif

		//  Enter Map Editor specific code
		SETUPLoadAllCoreShadersREST(g.gforceloadtestgameshaders,g.gpbroverride);
		material_loadsounds ( );
		mapeditorexecutable();
	}
	else
	{
		//  ACTUAL GAME EXE
	
		//  Debug report status
		timestampactivity(0,"main game executable");
	
		//  Set device to get multisampling AA active in editor
		t.multisamplingfactor=0 ; t.multimonitormode=0;
	
		//  Init app
		SyncOn (   ); 
		SyncRate (  0 );
		SetAutoCamOff (  );
		AlwaysActiveOff (  );
	
		//  Camera aspect ratio adjustment for desktop resolution
		t.aspect_f=GetDesktopWidth() ; t.aspect_f=t.aspect_f/GetDesktopHeight();
		SetCameraAspect (  t.aspect_f );
	
		//  set-up test game screen prompt assets (for printscreenprompt())
		loadscreenpromptassets();
		printscreenprompt("");

		// delayed material load to after logo splash
		SETUPLoadAllCoreShadersREST(g.gforceloadtestgameshaders,g.gpbroverride);
	
		//  Generic asset loading common to editor and game
		common_loadfonts();
		common_loadcommonassets ( 1 );

		// This used by 3D prompts in standalone
		g.guishadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_basic.fx");
	
		//  Load terrain from terrain temp save file
		terrain_createactualterrain();
		t.screenprompt_s=t.screenprompt_s+".";
		printscreenprompt(t.screenprompt_s.Get());
		timestampactivity(0,"_terrain_load");
		t.tfile_s="levelbank\\testmap\\m.dat";
		if (  FileExist(t.tfile_s.Get()) == 1 ) 
		{
			terrain_load ( );
		}
	
		//  Call visuals loop once to set shader constants
		t.visuals=t.gamevisuals;
		t.visuals.refreshshaders=1;
		visuals_loop ( );
	
		//  Main loop
		timestampactivity(0,"Main Game Executable Loop Starts");

		// after initial steroscopic fake load, switch to true stereo if used
		g_VR920RenderStereoNow = true;
	
		//  One-off variable settings
		t.game.set.resolution=0;
		t.game.set.initialsplashscreen=0;
	
		//  ensure no collision from DBP!
		AutomaticCameraCollision (  0,0,0 );
		SetGlobalCollisionOff (  );
	
		//  Setup game view camera?
		SetCameraFOV (  75 );
		SetCameraRange (  1,4000 );
		g.grav_f=-5.0;
	
		//  temporarily hide main screen (post process will show it when ready)
		SetCameraView (  0,0,0,1,1 );
	
		//  full speed
		SyncRate (  0 );
	
		//  Launch game in EXE mode
		game_masterroot ( );

		// 131115 - standalone game sessions fragment memory over time, so launch new instance
		// of the game executable (with silencing command line) and then quit this 'fragmented'
		// session after a few seconds to allow for a decent transition
		if ( t.game.masterloop != 0 && t.game.allowfragmentation == 0 )
		{
			// replaced master loop with EXE relaunch
			SetDir("..");
			LPSTR pEXEName = Appname();
			ExecuteFile ( pEXEName, "-reloadstandalone", "", 0 );
			Sleep(8000);
			return;
		}
	}

	//  Free before exit app
	///realsense_free ( );
	steam_free ( );
}

void common_justbeforeend ( void )
{
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
				if ( stricmp ( Right ( t.tfile_s.Get(), 4 ), ".txp" ) == NULL && stricmp ( t.tfile_s.Get(), "textures_profile.txp" ) != NULL ) 
				{
					// delete TXP file
					DeleteAFile ( t.tfile_s.Get() );

					// also delete associated textures belonging to this TXP file
					cStr tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_D.dds");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_N.dds");
					DeleteAFile ( tfilettex_s.Get() );
					tfilettex_s = cstr(Left ( t.tfile_s.Get(), strlen(t.tfile_s.Get())-4 )) + cstr("_S.dds");
					DeleteAFile ( tfilettex_s.Get() );
				}
			}
		}
		timestampactivity(0,"Clearing complete.");
		SetDir ( t.tolddir_s.Get() );
	}

	// save number of minutes user been in session (added to global recorded when we entered)
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
}

void common_loadfonts ( void )
{
	//  Bitmap Font Image Start
	t.tsplashstatusprogress_s="LOADING FONTS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	loadallfonts();
}

void common_loadcommonassets ( int iShowScreenPrompts )
{
	//  Bitmap Font Image Start
	t.tsplashstatusprogress_s="LOADING FONTS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	t.screenprompt_s="PREPARING CORE FILES";
	timestampactivity(0,"initbitmapfont");
	loadallfonts();

	// choose non-PBR or PBR shaders
	LPSTR pEffectStatic = "effectbank\\reloaded\\entity_basic.fx";
	LPSTR pEffectAnimated = "effectbank\\reloaded\\character_basic.fx";
	if ( g.gpbroverride == 1 )
	{
		pEffectStatic = "effectbank\\reloaded\\apbr_basic.fx";
		pEffectAnimated = "effectbank\\reloaded\\apbr_anim.fx";
	}

	// load common third person character shader
	if ( GetEffectExist(g.thirdpersoncharactereffect) == 0 ) 
	{
		LoadEffect ( pEffectAnimated, g.thirdpersoncharactereffect,0 );
		filleffectparamarray(g.thirdpersoncharactereffect);
	}
	if ( GetEffectExist(g.thirdpersonentityeffect) == 0 ) 
	{
		LoadEffect ( pEffectStatic, g.thirdpersonentityeffect,0 );
		filleffectparamarray(g.thirdpersonentityeffect);
	}

	// Also preload the entity basic shader so editor does not freeze on first object load
	int tunusedhereid = loadinternaleffect(pEffectStatic);

	//  Setup visual settings
	t.tsplashstatusprogress_s="INIT VECTORS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	common_vectorsinit ( );
	//Sync (  );
	t.tsplashstatusprogress_s="INIT SKY ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	sky_init ( );
	//Sync (  );
	t.tsplashstatusprogress_s="INIT TERRAIN ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	terrain_initstyles ( );
	//Sync (  );
	t.tsplashstatusprogress_s="INIT GAME VISUAL ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	visuals_init ( );
	//Sync (  );
	t.tsplashstatusprogress_s="INIT DECAL ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	decal_init ( );
	//Sync (  );
	t.tsplashstatusprogress_s="INIT LIGHTMAP ASSETS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	lm_init ( );
	//Sync (  );

	//  Setup default paths
	t.levelmapptah_s="levelbank\\testmap\\";
	g.projectfilename_s="";

	//  Get list of guns and flak for data
	t.screenprompt_s=t.screenprompt_s+".";
	if ( iShowScreenPrompts == 1 ) printscreenprompt(t.screenprompt_s.Get());
	t.tsplashstatusprogress_s="SCANNING G-LIST";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	gun_scaninall_ref ( );
	gun_scaninall_dataonly ( );

	// Sky details for terrain lighting
	if ( t.game.gameisexe == 0 )
	{
		t.screenprompt_s=t.screenprompt_s+".";
		if ( iShowScreenPrompts == 1 ) printscreenprompt(t.screenprompt_s.Get());
		t.tsplashstatusprogress_s="SCANNING SKY SETTINGS";
		timestampactivity(0,t.tsplashstatusprogress_s.Get());
		version_splashtext_statusupdate ( );
		terrain_skyspec_init ( );
	}

	//  Create terrain (eventually default terrain randomised)
	t.screenprompt_s=t.screenprompt_s+".";
	if ( iShowScreenPrompts == 1 ) printscreenprompt(t.screenprompt_s.Get());
	t.tsplashstatusprogress_s="CREATING TERRAIN";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	terrain_setupedit ( );
	terrain_make ( );

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

	//  Load in default player sounds (default style)
	t.tsplashstatusprogress_s="LOAD PLAYER SOUNDS";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );
	g.gplayerstyle_s="" ; material_loadplayersounds ( );

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
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	SetFileMapDWORD (  1, 44, 0 );
	SetFileMapDWORD (  1, 48, 2 );
	SetFileMapDWORD (  1, 52, GetChildWindowWidth()/2 );
	SetFileMapDWORD (  1, 56, GetChildWindowHeight()/2 );
	SetFileMapDWORD (  1, 704,GetChildWindowWidth() );
	SetFileMapDWORD (  1, 708,GetChildWindowHeight() );
}

void common_show_mouse ( void )
{
	// This does crazy cool stuff
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	SetFileMapDWORD (  1, 44, 1 );
	SetFileMapDWORD (  1, 48, 0 );
	SetFileMapDWORD (  1, 52, GetChildWindowWidth()/2 );
	SetFileMapDWORD (  1, 56, GetChildWindowHeight()/2 );
	SetFileMapDWORD (  1, 704,GetChildWindowWidth() );
	SetFileMapDWORD (  1, 708,GetChildWindowHeight() );
	//CloseFileMap (  1 );

return;

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

return;

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

	//  if file not exist, use universal
	if (  FileExist(t.tfile_s.Get()) == 0 ) 
	{
		//  use reloaded universal shader
		t.tfile_s="effectbank\\reloaded\\entity_basic.fx";
		if (  FileExist(t.tfile_s.Get()) == 0 ) 
		{
			//  something went VERY wrong
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

return;

// 
//  VERSIONS (Full)
// 

}

void version_commandlineprompt ( void )
{

	//  Triggers use of file map messaging
//  `if Lower(cl$())="-t"

	//gtestgamemodefromeditor=1
//  `endif


return;

}

void version_endofinit ( void )
{

//  Ensure full game flag never set!!
// `grealgameviewstate=0 `demo versions with noEXEsave do this


	//  Game Engine Demo would send this into filemap of OS
//filemapname$="FPSCComm"
//WriteFilemapValue (  filemapname$,42 )
//WriteFilemapString (  filemapname$,"demo version" )

return;

}

void version_splashtext_statusupdate ( void )
{
	//  Update Splash Text (  with update on what is being loaded (startup IDE) )
	if (  t.game.gameisexe != 1 ) 
	{
		//  and only if not running standalone
		//  takes tsplashstatusprogress$
		//if (  g.grestoreeditorsettings != 1 ) 
		//{
			welcome_updatebackdrop(t.tsplashstatusprogress_s.Get());
			//OpenFileMap (  5, "FPSSPLASH" );
			//SetFileMapDWORD (  5, 4, 1 );
			//SetFileMapString (  5, 1000, t.tsplashstatusprogress_s.Get() );
			//SetEventAndWait (  5 );
		//}
	}
}

void version_splashtext ( void )
{
	// Update Splash Text (  with version number and then close dialog )
	//OpenFileMap (  5, "FPSSPLASH" );
	//SetFileMapDWORD (  5, 4, 2 );
	//SetFileMapString (  5, 1000, t.strarr_s[392].Get() );
	//SetFileMapDWORD (  5, 8, 1 );
	//SetEventAndWait (  5 );
}

void version_onscreenlogos ( void )
{
return;

}

void version_permittestgame ( void )
{

	//  Map Editor launches test game
//  `open file map 1,"FPSEXCHANGE"

//  `set file map string$ 1, 1000, "FPSC-Game.exe"

//  `set file map string$ 1, 1256, "-t"

//  `set file map dword 1, 920, 1

//  `wait for file map event 1

//  `close file map 1


return;

}

void version_resourcewarning ( void )
{
return;

}

void version_universe_saveELEandLGT ( void )
{

	/*       part of save standalone
	//  FPGC - 111209 - right after lightmapping build so know if exited that loop
	debugviewtext(278,"Saving ELE and LGT files..");

	//  Save newer elements list (that includes scene dynamic entities)
	//  V106 also contains light indexes saved when universe is constructed
	\universe.ele" : gosub _entity_saveelementsdata
	entity_saveelementsdata ( );

	//  Save LGT lighting data
	filename_s="levelbank\\testlevel\\universe.lgt";
	if (  FileExist(filename_s) == 1  )  DeleteAFile (  filename_s );
	OpenToWrite (  1,filename_s );
		tinfinimax=ArrayCount(t.infinilight[]);
		WriteLong (  1,tinfinimax );
		for ( t.i = 0 ; t.i<=  tinfinimax; t.i++ )
		{
			WriteLong (  1,t.infinilight[t.i].used );
			WriteLong (  1,t.infinilight[t.i].type );
			WriteFloat (  1,t.infinilight[t.i].x );
			WriteFloat (  1,t.infinilight[t.i].y );
			WriteFloat (  1,t.infinilight[t.i].z );
			WriteFloat (  1,t.infinilight[t.i].range );
			WriteLong (  1,t.infinilight[t.i].id );
			WriteFloat (  1,t.infinilight[t.i].dist );
			WriteLong (  1,t.infinilight[t.i].colrgb.r );
			WriteLong (  1,t.infinilight[t.i].colrgb.g );
			WriteLong (  1,t.infinilight[t.i].colrgb.b );
			WriteLong (  1,t.infinilight[t.i].islit );
		}
	CloseFile (  1 );
	*/    

return;

}

void version_universe_construct ( void )
{
}


/*       part of old standalone t.game builder
}

void version_buildgame ( void )
{

	//  Is called when BUILD GAME selected from file menu
	interface_openbuildgame ; gosub _interface_handlebuildgame ;  gosub _interface_closebuildgame ( );

return;

}

void version_main_game_buildexe ( void )
{

//  Despot file collection
if (  FileExist("..\\buildfiles.ini") == 1  )  DeleteAFile (  "..\\buildfiles.ini" );
SaveArray (  "..\\buildfiles.ini",t.filecollection_s[] );

//  Store root folder
t.rootpath_s=GetDir();

//  Name without EXE
t.exename_s=g.gbuildname_s;
if (  Lower(Right(t.exename_s,4)) == ".exe" ) 
{
	t.exename_s=Left(t.exename_s,Len(t.exename_s)-4);
}

//  Path to EXE
if (  Mid(g.gbuildpath_s,2) == " ) :";
	t.exepath_s=g.gbuildpath_s;
}
else
{
	t.exepath_s=g.exedir_s;
}
if (  Right(t.exepath_s,1) != "\\"  )  t.exepath_s = t.exepath_s+"\\";
if (  PathExist(t.exepath_s) == 0  )  t.exepath_s = t.rootpath_s+"\\..\\MyGames\\";

//  if (  gimageblockmode == 0  )  GUI_CITF(exepath$, exename$)

//  Despot file collection
if (  FileExist("..\\buildfiles.ini") == 1  )  DeleteAFile (  "..\\buildfiles.ini" );
SaveArray (  "..\\buildfiles.ini",t.filecollection_s[] );

//  Store root folder
t.rootpath_s=GetDir();

//  FPSCV104RC7 - user can delete the MyGames folder?
if (  PathExist(t.exepath_s) == 0 ) 
{
	SetDir (  (  t.rootpath_s  ) ); SetDir "..";
	MakeDirectory (  "MyGames" );
}

//  Create game folder
SetDir (  t.exepath_s );
MakeDirectory (  t.exename_s );
SetDir (  t.exename_s );
MakeDirectory (  "Files" );
SetDir (  "Files" );

//  FPSCV10X, ensure gamesaves files are removed (if any)
if (  PathExist("gamesaves") == 1 ) 
{
	SetDir (  "gamesaves" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		tfile_s=ChecklistString(t.c);
		if (  Len(tfile_s)>2 ) 
		{
			if (  FileExist(tfile_s) == 1  )  DeleteAFile (  tfile_s );
		}
	}
	SetDir (  ".." );
}

//  V109 BETA4 - 230408 - delete entries that already exist in IMAGEBLOCK
if (  g.gimageblockmode == 1 ) 
{
	for ( imageblock = les;
t	//~  filesmax=array count(filecollection$())
	t.filesmax=ArrayCount(t.filecollection_s[]);
	for ( t.fileindex = 0 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	{
		t.srcstring_s=Lower(t.filecollection_s[t.fileindex]);
		if (  Len(t.srcstring_s)>0 ) 
		{
			//  and only if NOT in effectbank (some shaders NEED external image files)
			if (  Left(t.srcstring_s,11) != "effectbank\\" ) 
			{
				//  scan imageblock
				for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
				{
					imageblockfile_s=Lower(ChecklistString(t.c));
					if (  imageblockfile_s == t.srcstring_s ) 
					{
						t.filecollection_s[t.fileindex]="";
						t.c=ChecklistQuantity()+1;
					}
				}
			}
		}
	}
}

//  V109 BETA5 - 250408 - build image block CLOSE
if (  g.gcompilestandaloneexe == 1 && g.gimageblockmode == 1  )  close imageblock;

//  ensure file path exists (by creating folders)
t.filesmax=ArrayCount(t.filecollection_s[]);
for ( t.fileindex = 0 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	t.olddir_s=GetDir();
	t.src_s=t.filecollection_s[t.fileindex];
	t.srcstring_s=t.src_s;
	while (  Len(t.srcstring_s)>0 ) 
	{
		for ( t.c = 1 ; t.c<=  Len(t.srcstring_s); t.c++ )
		{
			if (  Mid(t.srcstring_s,t.c) == "\\" || Mid(t.srcstring_s,t.c) == "/" ) 
			{
				t.chunk_s=Left(t.srcstring_s,t.c-1);
				if (  Len(t.chunk_s)>0 ) 
				{
					if (  PathExist(t.chunk_s) == 0  )  MakeDirectory (  t.chunk_s );
					SetDir (  t.chunk_s );
				}
				t.srcstring_s=Right(t.srcstring_s,Len(t.srcstring_s)-t.c);
				break;
			}
		}
		if (  t.c>Len(t.srcstring_s)  )  break;
	}
	SetDir (  t.olddir_s );
}

//  v107 - 070807 - also detect for any HUD.X copies, and also copy texture files in its folder
SetDir (  t.rootpath_s );
for ( t.fileindex = 0 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	t.src_s=t.filecollection_s[t.fileindex];
	if (  Right(Lower(t.src_s),6) == "\\hud.x" ) 
	{
		//  find all files in the folder that shares this file and add to collection
		addallfilestocollection(Left(t.src_s,Len(t.src_s)-5));
	}
}
t.filesmax=ArrayCount(t.filecollection_s[]);

//  CopyAFile (  collection to exe folder )
SetDir (  t.rootpath_s );
debugviewtext(-1,t.strarr_s[393]);
for ( t.fileindex = 0 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	t.src_s=t.filecollection_s[t.fileindex];
	t.dest_s=t.exepath_s+t.exename_s+"\\Files\\"+t.src_s;
	if (  FileExist(t.dest_s) == 1  )  DeleteAFile (  t.dest_s );
	CopyAFile (  t.src_s,t.dest_s );
}

//  copy game engine and rename it
SetDir (  (  t.rootpath_s  ) ); SetDir "..";
t.dest_s=t.exepath_s+t.exename_s+"\\"+t.exename_s+".exe";
if (  FileExist(t.dest_s) == 1  )  DeleteAFile (  t.dest_s );
CopyAFile (  "FPSC-Game.exe",t.dest_s );

//  create a setup.ini file here reflecting game
Dim (  t.setuparr_s,999  );
t.setupfile_s=t.exepath_s+t.exename_s+"\\setup.ini" ; t.i=0;
t.setuparr_s[t.i]="gametype=0" ; inc t.i;
t.setuparr_s[t.i]=" g.serverhostname= FPSC Arena" ; inc t.i;
for ( t.num = 1 ; t.num<=  30; t.num++ )
t.setuparr_s[t.i]="taunt"+Str(t.num)+"="+Str(t.listkey[t.num]) ; inc t.i;
}
t.setuparr_s[t.i]="[GAMERUN]" ; inc t.i;
t.setuparr_s[t.i]="realgameview=1" ; inc t.i;
t.setuparr_s[t.i]="dynamiclighting="+Str(g.gdynamiclightingstate) ; inc t.i;
t.setuparr_s[t.i]="dynamicshadows="+Str(g.gdynamicshadowsstate) ; inc t.i;
t.setuparr_s[t.i]="dividetexturesize="+Str( g.gdividetexturesize ) ; inc t.i;
//  FPGC - 070411 - we can force the standalone games to use VRMODE when using VRQUEST
if (  g.fpgcgenre == 0 ) 
{
	t.setuparr_s[t.i]="vrmode=6" ; inc t.i;
	t.setuparr_s[t.i]="vrmodemag=100" ; inc t.i;
}
else
{
	t.setuparr_s[t.i]="vrmode="+Str( g.gvrmode ) ; inc t.i;
	t.setuparr_s[t.i]="vrmodemag="+Str( g.gvrmodemag ) ; inc t.i;
}
t.setuparr_s[t.i]="mousesensitivity="+Str( g.gmousesensitivity ) ; inc t.i;
t.setuparr_s[t.i]="producelogfiles="+Str( g.gproducelogfiles ) ; inc t.i;
t.setuparr_s[t.i]="hsrmode="+Str( g.ghsrmode ) ; inc t.i;
t.setuparr_s[t.i]="aspectratio="+Str( g.gaspectratio ) ; inc t.i;
t.setuparr_s[t.i]="newblossershaders="+Str( g.gnewblossershaders ) ; inc t.i;
t.setuparr_s[t.i]="postprocessing="+Str( g.gpostprocessing ) ; inc t.i;
t.setuparr_s[t.i]="showaioutlines="+Str( g.gshowaioutlines ) ; inc t.i;
t.setuparr_s[t.i]="airadius="+Str( g.gairadius ) ; inc t.i;
t.setuparr_s[t.i]="disablepeeking="+Str( g.gdisablepeeking ) ; inc t.i;
t.setuparr_s[t.i]="disableparticles="+Str( g.gparticlesnotused ) ; inc t.i;

t.setuparr_s[t.i]="" ; inc t.i;
t.setuparr_s[t.i]="[GAMEMULTIPLAYER]" ; inc t.i;
t.setuparr_s[t.i]="multiplayergame="+Str(g.gmultiplayergame) ; inc t.i;
t.setuparr_s[t.i]="gameobjectivetype="+Str(g.ggameobjectivetype) ; inc t.i;
t.setuparr_s[t.i]="gameobjectivevalue="+Str(g.ggameobjectivevalue) ; inc t.i;
t.setuparr_s[t.i]="oneshotkills="+Str(g.goneshotkills) ; inc t.i;
t.setuparr_s[t.i]="maxplayers="+Str(g.numberofplayers) ; inc t.i;
t.setuparr_s[t.i]="spawnrandom="+Str(g.gspawnrandom) ; inc t.i;
t.setuparr_s[t.i]="uniquegamecode="+g.guniquegamecode_s ; inc t.i;

t.setuparr_s[t.i]="" ; inc t.i;
t.setuparr_s[t.i]="[GAMEDEBUG]" ; inc t.i;
t.setuparr_s[t.i]="usesky=1" ; inc t.i;
t.setuparr_s[t.i]="usefloor="+Str( g.gusefloorstate ) ; inc t.i;
t.setuparr_s[t.i]="useenvsounds=1" ; inc t.i;
t.setuparr_s[t.i]="useweapons=1" ; inc t.i;
t.setuparr_s[t.i]="" ; inc t.i;
t.setuparr_s[t.i]="[GAMEPROFILE]" ; inc t.i;
t.setuparr_s[t.i]="title="+t.titlefpi_s ; inc t.i;
t.setuparr_s[t.i]="global="+t.setupfpi_s ; inc t.i;
t.setuparr_s[t.i]="gamewon="+t.gamewonfpi_s ; inc t.i;
t.setuparr_s[t.i]="gameover="+t.gameoverfpi_s ; inc t.i;
for ( t.num = 1 ; t.num<=  11; t.num++ )
	t.setuparr_s[t.i]="key"+Str(t.num)+"="+Str(t.listkey[t.num]) ; inc t.i;
}
for ( t.num = 1 ; t.num<=  30; t.num++ )
	t.setuparr_s[t.i]="taunt"+Str(t.num)+"="+Str(t.listkey[t.num]) ; inc t.i;
}
for ( t.num = 1 ; t.num<=  9; t.num++ )
	t.setuparr_s[t.i]="slot"+Str(t.num)+"="+t.gunslots_s[t.num] ; inc t.i;
}
t.setuparr_s[t.i]="levelmax="+Str(g.glevelmax) ; inc t.i;
for ( t.num = 1 ; t.num<=  g.glevelmax; t.num++ )
	t.setuparr_s[t.i]="levelfpm"+Str(t.num)+"="+t.level_s[t.num].fpm_s ; inc t.i;
	t.setuparr_s[t.i]="levelfpi"+Str(t.num)+"="+t.level_s[t.num].fpi_s ; inc t.i;
}
if (  FileExist(t.setupfile_s) == 1  )  DeleteAFile (  t.setupfile_s );
SaveArray (  t.setupfile_s,t.setuparr_s[] );
UnDim (  t.setuparr_s[] );

//  Also save out the localisation ptr file
Dim (  t.setuparr_s,2  );
t.setupfile_s=t.exepath_s+t.exename_s+"\\userdetails.ini";
t.setuparr_s[0]="[LOCALIZATION]";
t.setuparr_s[1]="language="+g.language_s;
SaveArray (  t.setupfile_s,t.setuparr_s[] );
UnDim (  t.setuparr_s[] );

//  FPGC - also account for missing folders/files required by ZERO-GENRE
if (  g.fpgcgenre == 0 ) 
{
	//  directory tells EXE it is a non-shooter
	SetDir (  (  (  t.exepath_s  ) ) ); SetDir t.exename_s ; SetDir "Files\\gamecore\\";
	if (  PathExist("equipment") == 0  )  MakeDirectory (  "equipment" );
}
if (  g.fpgcgenre == 1 ) 
{
	//  directory tells EXE it is a shooter
	SetDir (  (  (  t.exepath_s  ) ) ); SetDir t.exename_s ; SetDir "Files\\gamecore\\";
	if (  PathExist("guns") == 0  )  MakeDirectory (  "guns" );
}

//  Cannot depend on FPG being preserved, find from file collection
g.currentFPG_s="mygame";
for ( t.fileindex = 0 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	t.src_s=t.filecollection_s[t.fileindex];
	if (  Right(Lower(t.src_s),14) == "\\titlepage.fpi" ) 
	{
		tsrc_s=Left(t.src_s,Len(t.src_s)-14);
		for ( t.nn = Len(tsrc_s) ; t.nn>=  1 step -1; t.nn+= -1 )
		{
			if (  Mid(tsrc_s,t.nn) == "\\" || Mid(tsrc_s,t.nn) == "/" ) 
			{
				g.currentFPG_s=Right(tsrc_s,Len(tsrc_s)-t.nn);
				t.nn=0 ; break;
			}
		}
	}
}

//  GUI-X9 - knxrb
//  SetDir (  (  exepath$  ) ); SetDir ".."
//  CheckBuildScripts(exepath$+exename$,language$)

//  Restore directory
SetDir (  t.rootpath_s );

return;
*/    


void common_refreshDisplaySize ( void )
{
	//  Mode 1 uses GetDisplayWidth (  mostly, or special case child GetWindowWidth (  for some scenarios (res/fontsize) ) )
	#ifdef DX11
	// DX11 removed present Client Rect functionality, so needs to remain full view
	#else
	SetCameraView (  0,0,GetChildWindowWidth(1), GetChildWindowHeight(1) );
	#endif
}

//Functions

void popup_text_close ( void )
{
	OpenFileMap (  2, "FPSPOPUP" );
	SetFileMapDWORD (  2, 8, 1 );
	SetEventAndWait (  2 );
	g_PopupControlMode = 0;
	Sleep(100);
}

void popup_text_change ( char* statusbar_s )
{
	OpenFileMap (  2, "FPSPOPUP" );
	SetEventAndWait (  2 );
	if (  GetFileMapDWORD( 2, 0 )  ==  1 ) 
	{
		SetFileMapString ( 2, 1000, statusbar_s );
		SetFileMapDWORD ( 2, 4, 1 );
		SetEventAndWait ( 2 );
		//DWORD dwNow = timeGetTime();
		//while (  GetFileMapDWORD( 2, 4 )  ==  1 && timeGetTime() > dwNow + 3000 ) 
		//{
		//	SetEventAndWait ( 2 );
		//}
	}
}

void popup_text ( char* statusbar_s )
{
	if ( g_PopupControlMode == 0 )
	{
		t.strwork = "" ; t.strwork = t.strwork + "1:popup_text "+statusbar_s;
		timestampactivity(0, t.strwork.Get() );
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
				//DWORD dwNow = timeGetTime();
				//while (  GetFileMapDWORD( 2, 4 )  ==  1 && timeGetTime() > dwNow + 3000 ) 
				//{
				//	SetEventAndWait (  2 );
				//}
				return;
			}
			Sync (  );
		}
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
	memoryusagetable_s=g.rootdir_s+"editors\\gridedit\\memusedtable.dat";
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
//endfunction

}

void saveresource ( void )
{
	cstr memoryusagetable_s =  "";
	int numberofitems = 0;
	int memused = 0;
	cstr name_s =  "";
	int n = 0;

	//  Save out resource captures
	memoryusagetable_s=g.rootdir_s+"editors\\gridedit\\memusedtable.dat";
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
		memoryusagetable_s=g.rootdir_s+"editors\\gridedit\\memusedtable.log";
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
//endfunction

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
//   `if gtestgamemodefromeditor=1

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
//   `else

		////  real game or map editor usage tracks existing resources memory usage
		//actuallyused=0
		//if gamememresourceid>=0
		// actuallyused=gamememtable(gamememresourceid).memused
		//endif
//   `endif

	}
	else
	{
		gamememstamp=SMEMAvailable(1);
		g.gamememresourceid=-1;
	}
//endfunction actuallyused
	return actuallyused;
}

int closeresource ( void )
{
	int gamememstamp = 0;
	int memoryused = 0;
	if (  g.gamememresourceid >= 0 ) 
	{
//   `if gtestgamemodefromeditor=1

			//  test game records size of memory usage (end)
			if (  t.gamememtable[g.gamememresourceid].memused == -1 ) 
			{
				//  and only if not filled in do we write the memory used
				memoryused=SMEMAvailable(1)-gamememstamp;
				t.gamememtable[g.gamememresourceid].memused=memoryused;
			}
			g.gamememresourceid=-1;
//   `else

		////  rea game or map editing does nothing here (all happened with openresource)
//   `endif

	}
	else
	{
		memoryused=SMEMAvailable(1)-gamememstamp;
	}
//endfunction memoryused
	return memoryused
;
}


//FILECOLLECTOR FUNCTIONS


/*      
void resetfilecollection ( void )
{
	Undim t.filecollection_s[];
//endfunction

}
void addfiletocollection ( char* filename_s )
{
	if (  t.segobjusedformapeditor == 0 && segobjusedforsegeditor == 0 ) 
	{
		if (  Len(filename_s)>0 ) 
		{
			if (  Mid(filename_s,2) == " ) :";
				//  absolute paths not allowed for filenames!
				debugstring(filename_s,t.strarr_s[41]);
			}
			else
			{
				//  verify file not duplicated
				t.tokay=1;
				for ( chk = 1 ; chk<=  ArrayCount(t.filecollection_s[]); chk++ )
				{
					if (  Lower(t.filecollection_s[chk]) == Lower(filename_s) ) 
					{
						t.tokay=0;
					}
					if (  Lower(Left(t.filecollection_s[chk],Len(t.filecollection_s[chk])-4)) == Lower(Left(filename_s,Len(filename_s)-4)) ) 
					{
						//  if have a DDS, and a TGA of the same name, replace it
						if (  Lower(Right(t.filecollection_s[chk],4)) == ".tga" ) 
						{
							if (  Lower(Right(filename_s,4)) == ".dds" ) 
							{
								t.filecollection_s[chk]=filename_s;
								t.tokay=0 ; break;
							}
						}
						//  Scene Commander - memory reduction - if .wav and passing through here, convert - Not all wav's can be converted due to restrctions
						if (  Lower(Right(filename_s,4)) == ".ogg" || Lower(Right(filename_s,4)) == ".wav" ) 
						{
							sp_s=Lower(Left(t.filecollection_s[chk],Len(t.filecollection_s[chk])-4))+".ogg";
							if (  Lower(Right(t.filecollection_s[chk],4)) == ".wav" && g.guseoggoff == 0 ) 
							{
								//  041212 - Unfortunately OGG sounds cannot be 3D or looped, which means they are only good
								//  for things like sounds you play one-off and are not required to loop, so we create a black list
								//  of folders which contain loopable or 3D sounds below;
								tgamecritical=0;
								if (  Left(filename_s,15) == "audiobank\\music"  )  tgamecritical = 1;
								if (  Left(filename_s,15) == "audiobank\\water"  )  tgamecritical = 1;
								if (  Left(filename_s,19) == "audiobank\\materials"  )  tgamecritical = 1;
								if (  Left(filename_s,13) == "gamecore\\guns"  )  tgamecritical = 1;
								if (  tgamecritical == 0 ) 
								{
									if (  FileExist(sp_s) == 0 ) 
									{
										t.a_s=t.filecollection_s[chk];
										encode to ogg vorbis t.a_s,sp_s;
										//  we do not add, we simply replace the WAV that was already here with the OGG
//           `filename$=sp$ ; tokay=1

										t.filecollection_s[chk]=sp_s;
										t.tokay=0 ; break;
									}
								}
							}
						}
						if (  g.guseoggoff == 1 && Lower(Right(filename_s,4)) == ".wav" ) 
						{
							sp_s=Lower(Left(t.filecollection_s[chk],Len(t.filecollection_s[chk])-4))+".wav";
							//  we do not add, we simply replace the possible OGG that was already here with the WAV (only if the WAV exists)
//        `filename$=sp$ ; tokay=1

							if (  FileExist(sp_s) == 1 ) 
							{
								t.filecollection_s[chk]=sp_s;
								t.tokay=0 ; break;
							}
						}
					}
				}
				//  add file to collection
//     `041212 - handled above and OGG is produced and assigned

//     `twav$=Lower(Left(filename$,Len(filename$)-4))+".wav"

//     `togg$=Lower(Left(filename$,Len(filename$)-4))+".ogg"

//     `if FileExist(togg$) && filename$ == twav$ && Left(twav$,8) != "gamecore" then filename$ == Lower(Left(filename$,Len(filename$)-4))+".ogg"

 FileExist(togg_s) && filename_s == twav_s && Left(twav_s,8) != "gamecore"  )  filename_s = Lower(Left(filename_s,Len(filename_s)-4))+".ogg"

				if (  t.tokay == 1 ) 
				{
					//  file must be completely unique, so add it
					array insert at bottom t.filecollection_s[];
					t.filecollection_s[]=filename_s;
					//  FPGC - 220210 - if .WAV, of .OGG assume a LIPSYNC could be with it (even if ultimately not copied)
					if (  Lower(Right(filename_s,4)) == ".wav" || Lower(Right(filename_s,4)) == ".ogg" ) 
					{
						lipsyncfilename_s=Left(filename_s,Len(filename_s)-4)+".lipsync";
						array insert at bottom t.filecollection_s[];
						t.filecollection_s[]=lipsyncfilename_s;
					}
				}
			}
		}
	}
//endfunction

}
void addallfilestocollection ( char* folder_s )
{
	storedir_s=GetDir();
	SetDir (  folder_s );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		file_s=ChecklistString(t.c);
		if (  file_s != "." && file_s != ".." ) 
		{
			if (  Right(Lower(file_s),4) != ".tga" ) 
			{
				//  all except raw TGA file
				addfiletocollection(folder_s+"\\"+file_s);
			}
		}
	}
	SetDir (  storedir_s );
//endfunction

}
*/    

// 
//  SUBROUTINES AND FUNCTIONS FOR DEBUG ONLY
// 

//  FUNCTIONS

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
//endfunction

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
//endfunction

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
//endfunction

}

void debugstring ( char* tfile_s, char* desc_s )
{
	sprintf ( t.szwork , "%s%s) info:%s" , t.strarr_s[54].Get() , tfile_s , desc_s );
	timestampactivity(0, t.szwork);
//endfunction

}

void debugseevar ( int var )
{
	sprintf ( t.szwork , "%s%i)" , t.strarr_s[55].Get() , var );
	timestampactivity(0, t.szwork);
//endfunction

}

void debugpeek ( char* desc_s )
{
	SetTextToOpaque (  );
	Ink (  Rgb(255,255,255),0 );
	while (  SpaceKey() == 0 ) 
	{
		SetCursor (  100,150 );
		Print (  desc_s );
		Sync (  );
	}
	while ( SpaceKey()==1 ) Sync();
	SetTextToTransparent( );
//endfunction

}


//Progress Report Debug Function


void debugviewactivate ( int mode )
{

	if (  g.gdebugreportmodestate == 1 ) 
	{
		if (  mode == 1 ) 
		{
			if (  BitmapExist(1) == 0  )  CreateBitmap (  1,GetDisplayWidth(),150 );
		}
		else
		{
			if (  BitmapExist(1) == 1  )  DeleteBitmapEx (  1 );
		}
		SetCurrentBitmap (  0 );
	}

//endfunction

}

void debugviewprogressmax ( int progressmax )
{
	int gprogresscounter = 0;
	Dim (  t.mshot,progressmax  );
	g.lastmshotmem=0;
	g.gprogressmax=progressmax;
	gprogresscounter=0;
//endfunction

}

void debugviewtext ( int progress, char* gamedebugviewtext_s )
{
	int thisisaonetimeglobalbeforethisbuildends = 0;
	int gamedebugviewlastmem = 0;
	int gamedebugviewtime = 0;
	int gamedebugrefresh = 0;
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
			//  Hockeykid - 011110 - engine is done using the load report array, lets get rid of it!
//    `undim loadreport$(100000) ; loadreportarraydimmed=0

		}
	}

	//  progress bar based on progress/gprogressmax (some white and black)
	if (  progress != -1 ) 
	{
		//  Memory counters
		g.mshoti=progress;
		//  FPGC - 111209 - more info to indicate potential reason for a sudden crash (too much memory?)
//   `gamedebugviewtext$="["+Right(Str(1000+mshoti),3)+"] "+gamedebugviewtext$

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
							else
							{
											//  Default Bar Progress
											if (  g.gdebugreportmodestate == 1 ) 
											{
																progressx=54 ; progressy=392 ; progressh=18;
																progressw=GetDisplayWidth()-100.0001;
																if (  GetDisplayWidth() == 800  )  progressw = 693;
																t1=progressw*progress_f ; t2=progressx+t1;
																ty=(GetDisplayHeight()-150)-(GetDisplayHeight()/10);
																if (  GetDisplayHeight() == 600  )  ty = progressy;
																if (  t2>progressx ) { Ink (  Rgb(255,255,255),0  ) ; Box (  progressx,ty,t2,ty+(progressh*tscrhighdiff_f) ); }
																if (  t2<progressx+progressw ) { Ink (  Rgb(0,0,0),0  ) ; Box (  t2,ty,progressx+progressw,ty+(progressh*tscrhighdiff_f) ); }
											}
							}
		}
		else
		{
//       ``if guiloaderhud > -1

//       `` rem GUI-X9 (knxrb)

//       `` if SpriteExist(65534)

//       ``  SizeSprite (  65534,GetDisplayWidth(),GetDisplayHeight() )

//       ``  PasteSprite (  65534,0,0 )

//       `` endif


//       ``endif

		}
		if (  g.gdebugreportmodestate == 0 ) { SyncMask (  0x1  ) ; Sync (  ); } // S4real fix reflection problem with water.
	}
//  `if gdebugreportmodestate == 0 then Sync (  )

 //if ( g.gdebugreportmodestate == 0  )  Sync ( );


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
				//CloseFileMap (  3 );

				//  terminate test game mid-build
				SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
				SetFileMapString (  2, 1256, "-r" );
				SetFileMapDWORD (  2, 994, 2 );
				SetFileMapDWORD (  2, 924, 1 );
				SetEventAndWait (  2 );
				//CloseFileMap (  2 );
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
				//CloseFileMap (  1 );
			}
			//CloseFileMap (  2 );
		}
	}
	//  Build Executable Game Mode
	if (  g.gcompilestandaloneexe == 1 ) 
	{
		//  check if build cancelled
		tokay=0;
		OpenFileMap (  2, "FPSEXCHANGE" );
		SetEventAndWait (  2 );
		if (  GetFileMapDWORD( 2, 994 )  ==  1  )  tokay = 1;
		//CloseFileMap (  2 );
		if (  tokay == 1 ) 
		{
			//  terminate build early
			OpenFileMap (  1, "FPSBUILDGAME" );
			SetFileMapDWORD (  1, 108, 1 );
			SetFileMapDWORD (  1, 112, 0 );
			//  close dialog (cannot reload data into it when RELOAD MAPEDITOR)
			SetFileMapDWORD (  1, 24, 1 );
			SetEventAndWait (  1 );
			//CloseFileMap (  1 );
			//  call map editor back
			OpenFileMap (  2, "FPSEXCHANGE" );
			SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
			SetFileMapString (  2, 1256, "-r" );
			SetFileMapDWORD (  2, 994, 0 );
			SetFileMapDWORD (  2, 924, 1 );
			SetEventAndWait (  2 );
			//CloseFileMap (  2 );
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
			//CloseFileMap (  1 );
		}
	}

	//  debug view Text (  )
	if (  g.gdebugreportmodestate == 1 && g.gmapeditmode == 0 ) 
	{
		thetime=Timer()-gamedebugviewtime;
		thetime_s=Right(Str(1000000+thetime),6);
		themem=gamedebugviewmem-GetMemoryAvailable(0);
		themem_s=Right(Str(10000000+themem),7);
		sprintf ( t.szwork , " [ %s ] (" , themem_s.Get() );
		themem_s=t.szwork;
		if (  themem-gamedebugviewlastmem > 0  )  themem_s = themem_s+"+";
		themem_s=themem_s+Str(themem-gamedebugviewlastmem)+")  :  ";
		gamedebugview_s=thetime_s+themem_s+gamedebugviewtext_s+Chr(13)+Chr(10)+gamedebugview_s;
		gamedebugviewlastmem=themem;

		//  gamemain_writetexttodebugview;
		if (  g.gdebugreportmodestate == 1 ) 
		{
			if (  BitmapExist(1) == 1 ) 
			{
				SetCurrentBitmap (  1 );
				CLS (  Rgb(0,48,0) );
				Ink (  Rgb(255,255,255),0 );
				Print (  gamedebugview_s.Get() );
				Ink (  Rgb(0,255,0),0 );

				//  gamemain_updatedebugviewstats
				for ( stat = 1 ; stat<=  9; stat++ )
				{
					if (  stat == 1  )  stat_s = t.strarr_s[60];
					if (  stat == 2 ) {stat_s = t.strarr_s[61] ; stat2 = g.segidmaster;}
					if (  stat == 3 ) {stat_s = t.strarr_s[62] ; stat2 = g.entidmaster;}
					if (  stat == 4 ) { stat_s = t.strarr_s[63] ; stat2 = g.entityelementlist; }
					if (  stat == 5 ) { stat_s = t.strarr_s[64] ; stat2 = g.aiindexmaster; }
					if (  stat == 6 ) { stat_s = t.strarr_s[65]  ; stat2 = g.texturebankmax; }
					if (  stat == 7 ) { stat_s = t.strarr_s[66]  ; stat2 = g.imagebankmax;}
					if (  stat == 8 ) { stat_s = t.strarr_s[67]  ; stat2 = g.effectbankmax;}
					if (  stat == 9 ) { stat_s = t.strarr_s[68]  ; stat2 = g.waypointmax;}
					if (  stat>1 ) 
					{
						stat2_s=Str(stat2) ; stat3_s="" ; for ( st = 1 ; st <=  stat2 ; st+= 2 ) stat3_s=stat3_s+"|" ;
						stat1_s=stat_s ; stat2_s=stat2_s+Spaces(3-Len(stat2_s.Get()))+stat3_s;
					}
					else
					{
						stat1_s=stat_s ; stat2_s="";
					}
					Text (  GetDisplayWidth()-(GetDisplayWidth()/4),(stat-1)*14,stat1_s.Get() );
					Text (  GetDisplayWidth()-(GetDisplayWidth()/8),(stat-1)*14,stat2_s.Get() );
				}

				SetCurrentBitmap (  0 );
			}
			gamedebugrefresh=1;
		}

		debugviewupdate(1);
		if (  g.gdebugreportstepthroughstate == 1 ) 
		{
			//  step through debugging
			WaitForKey();
		}
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

//endfunction

}

void debugviewupdate ( int doisync )
{
	int gamedebugrefresh = 0;

	if (  g.gdebugreportmodestate == 1 && g.gmapeditmode == 0 ) 
	{
		if (  gamedebugrefresh == 1 ) 
		{
			gamedebugrefresh=0;
			if (  BitmapExist(1) == 1 ) 
			{
				SetCurrentBitmap (  1 );
				GrabImage (  g.editorimagesoffset+1,0,0,GetDisplayWidth(),150,1 );
			}
		}
		SetCurrentBitmap (  0 );
		if (  doisync == 0 && ImageExist(g.editorimagesoffset+1) == 1  )  PasteImage (  g.editorimagesoffset+1,0,GetDisplayHeight()-150 );
		if (  doisync == 1 && ImageExist(g.editorimagesoffset+1) == 1  )  PasteImage (  g.editorimagesoffset+1,0,GetDisplayHeight()-150 );
		if (  doisync == 1  )  Sync (  );
	}

//endfunction

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

//endfunction

}

// 
//  MESH AND TEXTURE HANDLING
// 

/*
int loadinternalmesh ( char* tfile_s )
{
	int meshbankmax = 0;
	int meshid = 0;

//  Default return
meshid=0;

//  Scan for existing mesh
if (  meshbankmax>0 ) 
{
	int meshbankmax = 0;
	int meshid = 0;
	int m = 0;
	for ( m = 1 ; m<=  meshbankmax; m++ )
	{
		if (  cstr(tfile_s) == t.meshbank_s[m].Get() ) { meshid = g.meshbankoffset+m ; break; }
	}
}
else
{
	t.m=meshbankmax+1;
}

//  Did not find, load it
if (  t.m>meshbankmax ) 
{
	++meshbankmax;
	Dim (  t.meshbank_s,meshbankmax  );
	if (  FileExist(tfile_s) == 1 ) 
	{
		meshid=g.meshbankoffset+meshbankmax;
		LoadMesh (  tfile_s,meshid );
		t.meshbank_s[meshbankmax]=tfile_s;
	}
}

//endfunction meshid
	return meshid;
}
*/

/*      
cstr findmaterialtexturesinmodelfile ( char* file_s, char* texpath_s )
{

	//  To determine if a model file requires
	//  texture files, we scan the file for a
	//  match to the Text (  .TGA or .JPG (and use texfile$) )
	returntexfile_s="";
	if (  FileExist(file_s) == 1 ) 
	{
	t.filesize=FileSize(file_s);
	t.mbi=255;
	OpenToRead (  11,file_s );
	MakeMemblockFromFile (  t.mbi,11 );
	CloseFile (  11 );
	for ( t.b = 0 ; t.b<=  t.filesize-1; t.b++ )
	{
		if (  ReadMemblockByte(t.mbi,t.b+0) == Asc(".") ) 
		{

			tfoundpiccy=0;
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("T") || ReadMemblockByte(t.mbi,t.b+1) == Asc("t") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("G") || ReadMemblockByte(t.mbi,t.b+2) == Asc("g") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("A") || ReadMemblockByte(t.mbi,t.b+3) == Asc("a") ) 
					{
						tfoundpiccy=1;
					}
				}
			}
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("J") || ReadMemblockByte(t.mbi,t.b+1) == Asc("j") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("P") || ReadMemblockByte(t.mbi,t.b+2) == Asc("p") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("G") || ReadMemblockByte(t.mbi,t.b+3) == Asc("g") ) 
					{
						tfoundpiccy=1;
					}
				}
			}

			if (  tfoundpiccy == 1 ) 
			{
						//  track back
						for ( t.c = t.b ; t.c>=  t.b-255 step -1; t.c+= -1 )
						{
							if (  ReadMemblockByte(t.mbi,t.c) >= Asc(" ") && ReadMemblockByte(t.mbi,t.c) <= Asc("z") && ReadMemblockByte(t.mbi,t.c) != 34 ) 
							{
								//  part of filename
							}
							else
							{
								//  no more filename
								break;
							}
						}
						texfile_s="";
						for ( t.d = t.c+1 ; t.d<=  t.b+3; t.d++ )
						{
							texfile_s=texfile_s+Chr(ReadMemblockByte(t.mbi,t.d));
						}
						if (  returntexfile_s == ""  )  returntexfile_s = texpath_s+texfile_s;
						inc t.b,4;
			}

		}
	}
	DeleteMemblock (  t.mbi );
	}

//endfunction returntexfile$
	return returntexfile_s
;
}
*/    


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


//FOR MAP EDITOR


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

//  localized description of selection
//  V109 BETA7 - 30408 - prevents editor overwriting when click item and change category (from 1768)
// `localdesc$=GetFileMapString( 1, 1768 )

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

void loadscreenpromptassets ( void )
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
						sprintf ( t.szwork , "languagebank\\%s\\artwork\\watermark\\gameguru-watermark-%ix%i.jpg", g.language_s.Get(), treswidth, tresheight );
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
					sprintf ( t.szwork , "gameguru-watermark-%ix%i.jpg" , tclosestreswidth , tclosestresheight );
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
			if (  t.game.gameisexe == 1 ) 
			{
				if ( g.iStandaloneIsReloading==0 )
				{
					// show splash initially
					tfile_s = respart_s;
					sprintf ( t.szwork, "languagebank\\%s\\artwork\\watermark\\%s", g.language_s.Get(), tfile_s.Get() );
					SetMipmapNum(1); //PE: mipmaps not needed.
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
					sprintf ( t.szwork , "languagebank\\%s\\artwork\\testgamelayout-noweapons.png", g.language_s.Get() );
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
				LoadImage (  t.szwork , g.testgamesplashimage );
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
			if ( g.globals.riftmode > 0 || g.gvrmode > 0 ) 
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
	steam_refresh ( );

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

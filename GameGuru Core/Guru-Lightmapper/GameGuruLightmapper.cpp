//
// GameGuru Lightmapper Engine
//

// Engine includes
#include "GameGuruLightmapper.h"
#include <stdio.h>
#include <string.h>
#include "gameguru.h"
#include "CGfxC.h"

// Externals
extern LPSTR gRefCommandLineString;

void GuruMain ( void )
{
	// these were the globals previously defined in types
	common_init_globals();
	char_init();

	// prepare all default values and check SETUP.INI for changes
	FPSC_SetDefaults();
	FPSC_LoadSETUPINI();

	// set display mode for lightmapper visual
	timestampactivity(0,"set display mode");
	g.gvsync = 0;
	t.bkwidth=GetDesktopWidth() ; t.bkheight=GetDesktopHeight();
	SetDisplayModeMODBACKBUFFER (  GetDesktopWidth(),GetDesktopHeight(),GetDisplayDepth(),g.gvsync,0,0,t.bkwidth,t.bkheight );
	g.gdisplaywidth=GetDesktopWidth();
	g.gdisplayheight=GetDesktopHeight();

	// runs as soon as EXE is launched (when set to 10)
	int autotriggerdefault = 10;//10
	int autotrigger = autotriggerdefault;
	int terrainlightmapped = 0;

	// get lightmapping mode right away
	g.lmlightmapnowmode = 1;
	if ( autotriggerdefault > 0 ) 
	{
		g.lmlightmapnowmode = atoi(Right(gRefCommandLineString,Len(gRefCommandLineString)-1));
	}

	// Init
	SetWindowTitle("");
	SetWindowSettings(0,0,0);
	ShowWindow();
	SyncOn(); SyncRate(0); SetAutoCamOff(); DisableEscapeKey();
	SetSprite ( 1, 0, 0 ); BackdropOff();

	// Full engine integgration path
	SetDir ( "Files" );

	// Prepare basic core shaders
	SETUPLoadAllCoreShadersFIRST(0);
	SETUPLoadAllCoreShadersREST(0,g.gpbroverride);

	// Activate logging
	g.gproducelogfiles = 1;
	g.trueappname_s = "Guru-Lightmapper";
	g.timestampactivitymax = 0;
	g.timestampactivityflagged = 0;
	g.timestampactivityindex = 0;
	g.timestampactivitymemthen = 0;
	g.timestampactivityvideomemthen = 0;
	Dim ( t.timestampactivity_s, 1000 );
	g.timebasepercycle_f = 0;
	g.timebasepercyclestamp = Timer();
	g.timestampactivitymemthen = SMEMAvailable(1);

	// Bitmap Font Image Start
	initbitmapfont ( "fontbank\\FPSCR-Font-24px", 1 );
	shrinkbitmapfont ( "fontbank\\FPSCR-Font-12px", 2, 1 );

	// User prompt
	t.tonscreenprompt_s = cstr("Loading Assets To Lightmap"); lm_flashprompt();

	// Terrain Init
	terrain_initstyles();
	terrain_make();

	// Load Terrain 
	t.tonscreenprompt_s = cstr("Loading Terrain To Lightmap"); lm_flashprompt();
	t.tfile_s = "levelbank\\testmap\\m.dat";
	if ( FileExist ( t.tfile_s.Get() ) == 1 ) 
	{
		terrain_createactualterrain ( );
		terrain_load();
	}

	// Ensure real textures used, not white texture!
	g.gdividetexturesize = 1;

	// LM init called at start
	lm_init();

	// Globals for proto
	///gotparticle as decalparticletype
	Dim ( t.waypoint, 100 );
	Dim ( t.charanimstates, 100 );
	///charanimstate as charanimstatetype
	Dim ( t.smoothanim, 100 );
	///Dim ( t.browseropen_s, 100 );?notfound
	Dim ( t.strarr_s, 100 );
	Dim ( t.debrisshapeindexused, 100 );
	///Dim ( t.mod, 100 );?notfound
	Dim ( t.csi_crouchidle, 5 );
	Dim ( t.csi_crouchgetup, 5 );
	Dim ( t.csi_crouchimpactfore, 5 );
	Dim ( t.csi_crouchimpactback, 5 );
	Dim ( t.csi_crouchimpactleft, 5 );
	Dim ( t.csi_crouchimpactright, 5 );

	// Proto assets
	LoadImage ( "effectbank\\Reloaded\\media\\blank_O.DDS", t.terrain.imagestartindex );
	LoadImage ( "effectbank\\Reloaded\\media\\blank_O.DDS", g.postprocessimageoffset+5 );

	// Load entities from current test level
	g.animmax = 700;
	g.footfallmax = 200;
	g.entidmastermax = 1000;
	Dim ( t.entitybank_s, 1000 );
	Dim2 ( t.entityphysicsbox, 1000, MAX_ENTITY_PHYSICS_BOXES );
	Dim2 ( t.entitybodypart, 1000, 100 );
	Dim2 ( t.entityanim, 1000, g.animmax );
	Dim2 ( t.entityfootfall, 1000, g.footfallmax );
	Dim ( t.entityprofileheader, 1000 );
	Dim ( t.entityprofile, 1000 );
	Dim2 ( t.entitydecal_s, 1000, 100 );
	Dim2 ( t.entitydecal, 1000, 100 );
	Dim2 ( t.entityblood, 1000, BLOODMAX );
	Dim ( t.strarr_s, 700 );

	// Load entity instances
	g.entityelementlist = 10;
	Dim ( t.entityelement, g.entityelementlist );
	Dim ( t.storedentityelement, g.entityelementlist );
	Dim2 ( t.entityphysicsbox, 1000, MAX_ENTITY_PHYSICS_BOXES );
	for ( int e = 1; e <= g.entityelementlist; e++ ) t.entityelement[e].bankindex = 0;
	t.levelmapptah_s = cstr("levelbank\\testmap\\");
	t.lightmapper.onlyloadstaticentitiesduringlightmapper = 1;
	entity_loadbank();
	entity_loadelementsdata();
	for ( t.tupdatee = 1; t.tupdatee <= g.entityelementlist; t.tupdatee++ )
	{
		t.tonscreenprompt_s = cstr("Loading Entity Instance ")+Str(t.tupdatee)+"/"+Str(g.entityelementlist); lm_flashprompt();
		entity_updateentityobj();
	}
	lighting_refresh();
	if ( autotriggerdefault == 10 )
	{
		// do not need OLD lightmap files if baking a fresh new set
	}
	else
	{
		t.lightmapper.onlyloadstaticentitiesduringlightmapper = 2;
		lm_loadscene();
	}
	t.lightmapper.onlyloadstaticentitiesduringlightmapper = 0;

	// Helps effect shaders
	common_vectorsinit();

	// Main
	Cls();
	BackdropOn();
	PositionCamera ( 25600, 600+7500, 25600 );
	PointCamera ( 25600, 600, 25600 );
	SetCameraRange ( 10, 50000 );
	int Null = MouseMoveX() + MouseMoveY();
	while ( EscapeKey() == 0 )
	{
		// Prompt and camera control
		if ( autotriggerdefault == 0 )
		{
			pastebitmapfont ( (cstr("FPS:")+cstr(GetDisplayFPS())).Get(), 20, GetDisplayHeight()-65, 1, 255 );
			float speed = 0.0f;
			if ( ShiftKey() == 1 ) speed = 0.2*(60.0/GetDisplayFPS()); else speed = 50.0*(60.0/GetDisplayFPS());
			float angy = CameraAngleY();
			float angx = CameraAngleX();
			if ( UpKey() == 1 ) MoveCamera ( speed );
			if ( DownKey() == 1 ) MoveCamera ( -speed );
			XRotateCamera ( 0 );
			if ( LeftKey() == 1 ) 
			{
				YRotateCamera ( angy-90 ); 
				MoveCamera ( speed );
				YRotateCamera ( angy );
			}
			if ( RightKey() == 1 ) 
			{
				YRotateCamera ( angy+90 );
				MoveCamera ( speed );
				YRotateCamera ( angy );
			}
			YRotateCamera ( angy + MouseMoveX()/3.0 );
			XRotateCamera ( angx + MouseMoveY()/3.0 );
		}

		// Fill shader with some defaults for proto
		int effectid = g.staticlightmapeffectoffset;
		if ( effectid > 0 )
		{
			if ( GetEffectExist ( effectid ) == 1 )
			{
				SetEffectConstantF ( effectid, "ShadowStrength", 1.0 );
				SetVector4 ( g.terrainvectorindex,0,10000,0,0 );
				SetEffectConstantV ( effectid,"HudFogDist", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex,1,1,1,0 );
				SetEffectConstantV ( effectid,"HudFogColor", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex, 0.15f, 0.15f, 0.15f, 0 );
				SetEffectConstantV ( effectid,"AmbiColorOverride", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex,1,1,1,0 );
				SetEffectConstantV ( effectid,"AmbiColor", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex,1,1,1,0 );
				SetEffectConstantV ( effectid,"SurfColor", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex,100,100,100,0.0 );
				SetEffectConstantV ( effectid,"LightSource", g.terrainvectorindex );
				SetVector4 ( g.terrainvectorindex, 500000, 1, 0, 0 ); // 500000 Y high before alpha fading on entity
				SetEffectConstantV ( effectid,"EntityEffectControl", g.terrainvectorindex );
				SetEffectConstantF ( effectid,"GlobalSpecular", 0.25f );
				SetEffectConstantF ( effectid,"GlobalSurfaceIntensity", 0.5f );
			}
		}

		// Process scene and create lightmapping
		if ( terrainlightmapped == 0 )
		{
			if ( autotrigger > 1 ) autotrigger = autotrigger - 1;
			if ( SpaceKey() == 1 || autotrigger == 1 )
			{
				autotrigger = 0;
				// called when test game triggers a lightmap process
				lm_process();
				terrainlightmapped = 1;
				if ( autotriggerdefault > 0 )
				{
					// end process
					timestampactivity ( 0, "LIGHTMAPPER: End process and return" );
					// end it here (wrong, we want the exe to leave gracefully!)
					ExitProcess ( 0 );
				}
			}
		}

		// Handle any effect code
		if ( g.effectbankmax > 0 )
		{
			for ( int t = 1; t <= g.effectbankmax; t++ )
			{
				int effectid = g.effectbankoffset + t;
				if ( GetEffectExist ( effectid ) == 1 )
				{
					SetEffectTechnique ( effectid,"Highest" );
					SetEffectConstantF ( effectid,"ShadowStrength",0 );
					SetVector4 ( g.terrainvectorindex,0,50000,0,0 );
					SetEffectConstantV ( effectid,"HudFogDist", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,0,0,0,0 );
					SetEffectConstantV ( effectid,"HudFogColor", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,1,1,1,0 );
					SetEffectConstantV ( effectid,"AmbiColorOverride", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,0.75,0.75,0.75,0 );
					SetEffectConstantV ( effectid,"AmbiColor", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,1,1,1,0 );
					SetEffectConstantV ( effectid,"SurfColor", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,30000,10000,50000,0 );
					SetEffectConstantV ( effectid,"LightSource", g.terrainvectorindex );
					SetVector4 ( g.terrainvectorindex,500000, 1, 0, 0 ); // 500000 Y high before alpha fading on entity
					SetEffectConstantV ( effectid,"EntityEffectControl", g.terrainvectorindex );
				}
			}
		}

		// Render terrain 
		if ( ControlKey() == 1 ) terrainlightmapped = 1;
		if ( terrainlightmapped == 0 )
		{
			t.terrain.gameplaycamera = 0;
			BT_SetCurrentCamera ( t.terrain.gameplaycamera );
			BT_UpdateTerrainCull ( t.terrain.TerrainID );
			BT_UpdateTerrainLOD ( t.terrain.TerrainID );
			BT_RenderTerrain ( t.terrain.TerrainID );
		}

		// Update scene
		Sync();
	}

	// Finish
	// end it here (wrong, we want the exe to leave gracefully!)
	ExitProcess ( 0 );
}

void LoadFBX ( LPSTR szFilename, int iID )
{
	// not used in lightmapper
}

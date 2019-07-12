//----------------------------------------------------
//--- GAMEGURU - M-GridEdit
//----------------------------------------------------

#include "gameguru.h"
#include "M-WelcomeSystem.h"
#include "GGVR.h"

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

//  GOTO LABEL (jump from common_init)
void mapeditorexecutable ( void )
{
	//  Debug report status (must activate with producelogfiles=1)
	t.tsplashstatusprogress_s="MAIN EDITOR INIT";
	timestampactivity(0,t.tsplashstatusprogress_s.Get());
	version_splashtext_statusupdate ( );

	//  Means we are in the editor (1) or in standalone game (0)
	timestampactivity(0,"ide input mode");
	g.globals.ideinputmode = 1;

	// No VR or RIFTMODE in Editor Mode
	g.globals.riftmode = 0;
	g.vrglobals.GGVREnabled = 0; 
	g.vrglobals.GGVRUsingVRSystem = 1; 
	if ( g.gvrmode == 2 ) g.vrglobals.GGVREnabled = 1; // OpenVR (Steam)
	if ( g.gvrmode == 3 ) g.vrglobals.GGVREnabled = 2; // Windows Mixed Reality (Microsoft)
	char pVRSystemString[1024];
	sprintf ( pVRSystemString, "choose VR system with mode %d", g.vrglobals.GGVREnabled );
	timestampactivity(0,pVRSystemString);
	int iErrorCode = GGVR_ChooseVRSystem ( g.vrglobals.GGVREnabled, g.gproducelogfiles, cstr(g.fpscrootdir_s+"\\GGWMR.dll").Get() );
	if ( iErrorCode > 0 )
	{
		// if VR headset is not present, switch VR off to speed up non-VR rendering (especially for debug)
		char pErrorStr[1024];
		sprintf ( pErrorStr, "Error Choosing VR System : Code %d", iErrorCode );
		timestampactivity(0,pErrorStr);
		timestampactivity(0,"switching VR off, headset not detected");
		g.vrglobals.GGVREnabled = 0;
	}

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

	// So entirely replace fixed function rendering, use this shader effect
	g.guishadereffectindex = loadinternaleffect("effectbank\\reloaded\\gui_basic.fx");
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
		version_splashtext ( );
	}

	// Set editor fonts and Text (  style and size )
	timestampactivity(0,"Prepare editor fonts");
	SetTextFont (  "Verdana"  ); SetTextToBold (  );
	Ink (  Rgb(255,255,225),0  ); SetTextSize (  26 );

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

	//  only show front dialogs if not resuming from previous session
	int iCountDownToShowQuickStartDialog = 0;
	if ( g.grestoreeditorsettings == 0 ) 
	{
		// Welcome quick start page
		g.quickstartmenumode = 0;
		if ( g.iFreeVersionModeActive == 1 )
		{
			editor_showquickstart ( 0 );
		}
		else
		{
			if ( g.gshowonstartup == 1 || g.iTriggerSoftwareToQuit != 0 ) 
				editor_showquickstart ( 0 );
			else
				welcome_free();
		}
	}
	else
	{
		// always need to close down loading splash
		welcome_free();
	}

	//  Main loop
	timestampactivity(0,"Guru Map Editor Loop Starts");

	// main loop
	while ( 1 )
	{
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

		//  Emergency debugging string
		if (  g.leedebugvalue_s != "" ) 
		{
			SetCursor (  0,0 );
			t.strwork = ""; t.strwork = t.strwork + "leedebugvalue_s="+g.leedebugvalue_s;
			Print ( t.strwork.Get()  );
		}

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
		input_getcontrols ( );
		input_calculatelocalcursor ( );

		//  Importer, Character Creator or Main Editor
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
			if (  t.characterkit.loaded !=  0 ) 
			{
				characterkit_loop ( );
				characterkit_draw ( );
			}
			else
			{
				// give editor a chance to init, then switch to EBE for quick coding!
				// Editor Controls
				terrain_terraintexturesystempainterentry ( );
				editor_constructionselection ( );
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
				characterkit_updateAllCharacterCreatorEntitiesInMap ( );

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
					OpenFileMap (  3, "FPSEXCHANGE" );
					for ( t.idechecks = 1 ; t.idechecks<=  3; t.idechecks++ )
					{
						if (  t.idechecks == 1 ) { t.virtualfileindex = 758  ; t.tvaluetocheck = 1; }
						if (  t.idechecks == 2 ) { t.virtualfileindex = 762  ; t.tvaluetocheck = 1; }
						if (  t.idechecks == 3 ) { t.virtualfileindex = 762  ; t.tvaluetocheck = 2; }
						t.tokay=GetFileMapDWORD( 3, t.virtualfileindex );
						if (  t.tokay == t.tvaluetocheck ) 
						{
							SetEventAndWait (  3 );
							//CloseFileMap (  3 );
							if (  t.idechecks == 1 ) 
							{
								//  Save Standalone
								gridedit_intercept_savefirst ( );
								OpenFileMap (  3, "FPSEXCHANGE" );
								if (  t.editorcanceltask == 0 ) 
								{
									popup_text(t.strarr_s[82].Get());
									gridedit_load_map ( ); // 190417 - ensures levelbank contents SAME as level 1 FPM!
									mapfile_savestandalone ( );
									SetFileMapDWORD (  3, t.virtualfileindex, 3 );
									popup_text_close();
								}
								else
								{
									SetFileMapDWORD (  3, t.virtualfileindex, 0 );
								}
							}
							if (  t.idechecks == 2 ) 
							{
								// (dave) Skip terrain rendering - it causes a crash in debug
								g_bSkipTerrainRender = true;

								//  Import Model
								gridedit_import_ask ( );
								SetFileMapDWORD (  3, t.virtualfileindex, 0 );
							}
							if (  t.idechecks == 3 ) 
							{
								//  Character Creator
								if (  t.characterkit.loaded == 0  )  t.characterkit.loaded = 1;
								SetFileMapDWORD (  3, t.virtualfileindex, 0 );
							}
							SetEventAndWait (  3 );
						}
					}

					// Handle auto trigger stock level loader
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
				}

				// Draw memory remaining

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
				if ( gbWelcomeSystemActive == false )
				{
					SyncRate ( 0 ); SyncMask ( 1 ); Sync ( ); SleepNow ( 5 );
				}
			}
			else
			{
				//  check for PAINT message
				OpenFileMap (  3, "FPSEXCHANGE" );
				SetEventAndWait (  3 );
				if (  GetFileMapDWORD( 3, 60 ) == 1 ) 
				{
					SetFileMapDWORD (  3,60,0  ); t.syncthreetimes=3;
					SetEventAndWait (  3 );
				}
				SyncRate (  0 );
				if (  t.syncthreetimes>0 ) {  --t.syncthreetimes; Sync ( ); }
				SleepNow ( 10 );
			}

			//  Detect if resolution changed (windows)
			editor_detect_invalid_screen ( );
		}
	}

	//  End map editor program
	common_justbeforeend();
	ExitProcess ( 0 );
}

void editor_detect_invalid_screen ( void )
{
	if (  GetDisplayInvalid() != 0 || MatrixExist(g.m4_projection) == 0 ) 
	{
		if (  1 ) 
		{
			// message Box (  - resolution has been changed - must restart - save changes? )
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
	// image to use
	int iEditorHelpImage = 1;
	int iDivideBy = 1;
	#ifdef VRQUEST
	 switch ( iHelpType )
	 {
		case 1 : iEditorHelpImage = 1; break;
		case 2 : iEditorHelpImage = 27; iDivideBy = 2; break;
		case 3 : iEditorHelpImage = 28; iDivideBy = 2; break;
	 }
	#endif
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
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
	while (  t.inputsys.kscancode == 0 && EscapeKey() == 0 ) 
	{
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
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
		if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
		if (  GetFileMapDWORD( 1, 20 ) != 0  )  break;
		if (  GetFileMapDWORD( 1, 516 )>0  )  break;
		t.terrain.gameplaycamera=0;
		terrain_shadowupdate ( );
		terrain_update ( );
		PasteSprite (  123,(GetChildWindowWidth(0)-t.imgx_f)/2,(GetChildWindowHeight(0)-t.imgy_f)/2 );
		Sync (  );
	}
	//  once we are in the help page, can use filemap to detect key (so F1 on/off issue does not occur)
	do
	{
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
		FastSync (  );
	} while ( !(  t.inputsys.kscancode == 0 ) );
}

void editor_showparentalcontrolpage ( void )
{
	// allow parental control to be activated and deactivated
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	do
	{
		t.inputsys.mclick=MouseClick();
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
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
		if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
		if (  GetFileMapDWORD( 1, 516 )>0  )  break;
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
	while ( !(  t.inputsys.kscancode == 0 ) );

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
		MessageBox ( GetForegroundWindow(), "In order for the restricted content mode chosen to take effect, you must exit GameGuru and restart", "GameGuru Restart", MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST );
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
		welcome_show(WELCOME_SERIALCODE);
	}

	// can stay here forever if quit triggered
	if ( g.iTriggerSoftwareToQuit != 0 ) 
	{
		welcome_show(WELCOME_EXITAPP);
	}
	else
	{
		if ( g.iFreeVersionModeActive == 1 )
		{
			welcome_show(WELCOME_FREEINTROAPP);
		}
	}

	// if first time run
	if ( g.gfirsttimerun == 1 ) welcome_show(WELCOME_WHATYOUGET);

	// if welcome not deactivated
	if ( g.gshowonstartup != 0 || iForceMainOpen == 1 )
	{
		// start welcome page
		if ( g.vrqcontrolmode == 0 )
			welcome_show(WELCOME_MAIN);
		else
			welcome_show(WELCOME_MAINVR);
	}

	// close welcome system
	welcome_free();

	/*
	// video and game title strings
	LPSTR pVideoAndGameTitles[3][10] = { {	"Quick Start", 
											"Quick Start",
											"Quick Start",
											"Quick Start",
											"Getting Started",
											"Getting Started",
											"Getting Started",
											"Getting Started",
											"Getting Started",
											"Getting Started" }, 

										{	"What Is GameGuru?", 
											"The Demo Games",
											"How To Create A Game?",
											"The Finished Game",
											"The Editor Basics Part 1",
											"The Editor Basics Part 2",
											"The Widget Tool",
											"Making A Quick Game",
											"Test Level Mode",
											"3D Edit Mode" }, 

										 {	"The Big Escape (FPS)", 
											"Cartoon Antics (Kids)",
											"Gem World (Cell Shaded Game)",
											"Heirs Revenge (Adventure)",
											"Morning Mountain Stroll (FPS)",
											"The Asylum (Horror)",
											"Warehouse (Zombie Survival)",
											"Bridge Battle (Multiplayer)",
											"Camp Oasis (Multiplayer)",
											"Devils Hill (Multiplayer)" } };

	// 050416 - wipe out violent games from the menus
	if ( g.quickparentalcontrolmode == 2 )
	{
		pVideoAndGameTitles[2][0] = "";
		pVideoAndGameTitles[2][4] = "";
		pVideoAndGameTitles[2][5] = "";
	}

	// can be Video menu or Game menu (g.quickstartmenumode)
	if ( 1 )
	{
		editor_freezeanimations();
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetEventAndWait (  1 );
		do
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
			FastSync (  );
		} while ( !(  t.inputsys.kscancode == 0 && t.inputsys.mclick == 0 ) );

		// prepare dialog backdrop
		t.imgx_f=ImageWidth(g.editorimagesoffset+8);
		t.imgy_f=ImageHeight(g.editorimagesoffset+8);
		Sprite (  123,-10000,-10000,g.editorimagesoffset+8 );
		SizeSprite (  123,t.imgx_f,t.imgy_f );

		// 081015 - prepare thumbnails
		for ( int iItemIndex=1; iItemIndex<=10; iItemIndex++ )
		{
			Sprite ( 123+iItemIndex, -10000, -10000, g.editorimagesoffset+30+iItemIndex );
			SizeSprite ( 123+iItemIndex, 160, 98 );
			Sprite ( 143+iItemIndex, -10000, -10000, g.editorimagesoffset+30 );
			SizeSprite ( 143+iItemIndex, 160, 98 );
			Sprite ( 153+iItemIndex, -10000, -10000, g.editorimagesoffset+50+iItemIndex );
			SizeSprite ( 153+iItemIndex, 160, 98 );
		}

		// stay here until leave menu
		int iAutoTriggerVideo = 0;
		if ( g.videoMenuPlayed[0]==0 ) 
		{
			iAutoTriggerVideo = 1;
		}
		t.tclicked=0 ; t.tclosequick=0;
		t.lastmousex=MouseX() ; t.lastmousey=MouseY();
		while (  t.inputsys.kscancode == 0 && EscapeKey() == 0 && t.tclosequick == 0 ) 
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
			if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
			if (  GetFileMapDWORD( 1, 516 )>0  )  break;
			if (  GetFileMapDWORD( 1, 400 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 404 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 408 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 434 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 758 ) != 0 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 762 ) != 0 ) { t.interactive.active = 0  ; break; }
			t.terrain.gameplaycamera=0;
			terrain_shadowupdate ( );
			terrain_update ( );

			// choose backdrop
			if ( g.quickstartmenumode==0 )
				Sprite (  123,-10000,-10000,g.editorimagesoffset+8 );
			else
				Sprite (  123,-10000,-10000,g.editorimagesoffset+41 );

			// paste backdrop
			t.tcenterx=(GetChildWindowWidth(0)-t.imgx_f)/2;
			t.tcentery=(GetChildWindowHeight(0)-t.imgy_f)/2;
			PasteSprite ( 123, t.tcenterx, t.tcentery );
			t.inputsys.xmouse=((GetFileMapDWORD( 1, 0 )+0.0)/800.0)*GetChildWindowWidth(0);
			t.inputsys.ymouse=((GetFileMapDWORD( 1, 4 )+0.0)/600.0)*GetChildWindowHeight(0);

			// 081015 - draw thumb videos
			int iHighlightedIndex = 0;
			int iScrMouseX = t.inputsys.xmouse;
			int iScrMouseY = t.inputsys.ymouse;
			int iItemIndex = 1;
			int iColumnsNum = 5;
			int iColumnsXPos = 0;
			if ( g.quickstartmenumode==1 ) { iColumnsNum = 3; iColumnsXPos = 160+14; }
			for ( int yy=0; yy<2; yy++ )
			{
				for ( int xx=0; xx<iColumnsNum; xx++ )
				{
					int iCornerX = t.tcenterx + 32 + iColumnsXPos + (xx*(160+14));
					int iCornerY = t.tcentery + 106 + (yy*(98+80));
					if ( iScrMouseX>=iCornerX && iScrMouseX<iCornerX+160+14 )
					{
						if ( iScrMouseY>=iCornerY && iScrMouseY<iCornerY+98 )
						{
							iHighlightedIndex = iItemIndex;
							iCornerX-=5;
							iCornerY-=5;
						}
					}
					LPSTR pThumbText = "";
					if ( g.quickstartmenumode==0 )
					{
						if ( g.videoMenuPlayed[iItemIndex-1] > 0 )
						{
							// watched
							SetSpriteAlpha ( 123+iItemIndex, 255*0.6 );
							PasteSprite ( 123+iItemIndex, iCornerX, iCornerY );
							PasteSprite ( 143+iItemIndex, iCornerX, iCornerY );
						}
						else
						{
							// not watched yet
							SetSpriteAlpha ( 123+iItemIndex, 255 );
							PasteSprite ( 123+iItemIndex, iCornerX, iCornerY );
						}
						pThumbText = pVideoAndGameTitles[0][iItemIndex-1];
						pastebitmapfontcenter ( pThumbText, iCornerX + (160/2), iCornerY+80+20, 2, 255 );
						pThumbText = pVideoAndGameTitles[1][iItemIndex-1];
						pastebitmapfontcenter ( pThumbText, iCornerX + (160/2), iCornerY+80+20+20, 2, 255 );
					}
					else
					{
						// draw games to try
						pThumbText = pVideoAndGameTitles[2][iItemIndex-1];
						if ( strlen ( pThumbText ) > 0 )
						{
							SetSpriteAlpha ( 153+iItemIndex, 255 );
							PasteSprite ( 153+iItemIndex, iCornerX, iCornerY );
							pastebitmapfontcenter ( pThumbText, iCornerX + (160/2), iCornerY+80+20, 2, 255 );
						}
						else
						{
							if ( iHighlightedIndex == iItemIndex ) iHighlightedIndex = 0;
						}
					}
					iItemIndex++;
				}
			}

			// show the name of the video
			LPSTR pHighlightedVideoTitle = "";
			if ( iHighlightedIndex > 0 )
			{
				if ( g.quickstartmenumode==0 )
					pHighlightedVideoTitle = pVideoAndGameTitles[1][iHighlightedIndex-1];
				else
					pHighlightedVideoTitle = pVideoAndGameTitles[2][iHighlightedIndex-1];
			}
			int iTextCenterX = t.tcenterx + (ImageWidth(g.editorimagesoffset+8)/2);
			int iTextCenterY = t.tcentery + (ImageHeight(g.editorimagesoffset+8)*0.77);
			pastebitmapfontcenter ( pHighlightedVideoTitle, iTextCenterX, iTextCenterY, 1, 255);

			// must be in IDE focus to ensure stuff not clicked or run in background
			DWORD dwForegroundFocusForIDE = GetFileMapDWORD( 1, 596 );
			if ( dwForegroundFocusForIDE == 10 )
			{
				// 161115 - if click outside of dialog, also close!
				if ( t.inputsys.mclick == 1 )
				{
					if ( iScrMouseX < t.tcenterx || iScrMouseX > t.tcenterx+920 )
					{
						if ( iScrMouseY < t.tcentery || iScrMouseY > t.tcentery+590 )
						{
							t.tclosequick=1;
							t.tclicked=1;
						}
					}
				}

				// if click, play the animation
				if ( (t.inputsys.mclick == 1 && iHighlightedIndex > 0) || iAutoTriggerVideo != 0 ) 
				{
					// release mouse now
					while ( MouseClick()!=0 ) { Sync(); }

					// video menu or game menu
					if ( g.quickstartmenumode==0 )
					{
						// auto trigger is good to hit the user with something cool at the very start
						if ( iAutoTriggerVideo !=0 )
						{
							iHighlightedIndex = iAutoTriggerVideo;
							iAutoTriggerVideo = 0;
						}

						// go full screen
						//SetFileMapDWORD (  1, 970, 2 );
						//SetEventAndWait (  1 );

						// play this animation now
						#ifndef _DEBUG
						cStr pAnimfilename = cStr("");
						pAnimfilename = cStr("languagebank\\")+g.language_s+"\\artwork\\videos\\"+cStr(iHighlightedIndex)+".wmv";
						LoadAnimation ( pAnimfilename.Get(), 1 );
						PlayAnimation ( 1, 0, 0, GetChildWindowWidth(0), GetChildWindowHeight(0) );
						DWORD dwAnimStartedTime = timeGetTime();
						int iWatchedMarkerInSeconds = GetAnimationLength(1) * 0.8f;
						bool bExitPlayback = false;
						while ( AnimationPlaying(1) && bExitPlayback==false )
						{
							if ( (timeGetTime()-dwAnimStartedTime)/1000 > (DWORD)iWatchedMarkerInSeconds || iHighlightedIndex == 1 )
							{
								// watched this video for more than 80% (except first one)
								// mark as watched
								g.videoMenuPlayed[iHighlightedIndex-1] = 1;
							}
							if ( (timeGetTime()-dwAnimStartedTime)/1000 > 2 )
							{
								// can click out after X seconds
								if ( MouseClick()!=0 ) bExitPlayback = true;
							}
							Sync();
						}
						while ( MouseClick()!=0 ) { Sync(); }
						DeleteAnimation ( 1 );
						#endif

						// restore editor within IDE
						//SetFileMapDWORD (  1, 970, 1 );
						//SetEventAndWait (  1 );
					}
					else
					{
						// load a game level in
						t.tlevelautoload_s="";
						switch ( iHighlightedIndex ) 
						{
							case 1 : t.tlevelautoload_s = "The Big Escape.fpm" ; break ;
							case 2 : t.tlevelautoload_s = "Cartoon Antics.fpm" ; break ;
							case 3 : t.tlevelautoload_s = "Gem World.fpm" ; break ;
							case 4 : t.tlevelautoload_s = "The Heirs Revenge.fpm" ; break ;
							case 5 : t.tlevelautoload_s = "Morning Mountain Stroll.fpm" ; break ;
							case 6 : t.tlevelautoload_s = "The Asylum.fpm" ; break ;
							case 7 : t.tlevelautoload_s = "The Warehouse.fpm" ; break ;
							case 8 : t.tlevelautoload_s = "Bridge Battle (MP).fpm" ; break ;
							case 9 : t.tlevelautoload_s = "Camp Oasis (MP).fpm" ; break ;
							case 10 : t.tlevelautoload_s = "Devils Hill (MP).fpm" ; break ;
						}
						t.tlevelautoload_s=g.fpscrootdir_s+"\\Files\\mapbank\\"+t.tlevelautoload_s;

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
									g.showtestlevelclickprompt = timeGetTime() + 5000;
									t.tclosequick=1;
									t.tclicked=1;
								}
							}
						}
					}
				}
			}

			t.tquickhighlight=0;
			if (  t.inputsys.xmouse>t.tcenterx+36 && t.inputsys.xmouse<t.tcenterx+68 && t.inputsys.ymouse>t.tcentery+519 && t.inputsys.ymouse<t.tcentery+553 ) 
			{
				//  toggle show on startup
				t.tquickhighlight=1;
			}
			if (  t.inputsys.xmouse>t.tcenterx+785 && t.inputsys.xmouse<t.tcenterx+890 && t.inputsys.ymouse>t.tcentery+506 && t.inputsys.ymouse<t.tcentery+561 ) 
			{
				//  close
				t.tquickhighlight=3;
			}
			if (  t.inputsys.xmouse>t.tcenterx+860 && t.inputsys.xmouse<t.tcenterx+888 && t.inputsys.ymouse>t.tcentery+39 && t.inputsys.ymouse<t.tcentery+67 ) 
			{
				//  exit cross
				t.tquickhighlight=4;
			}
			if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+558 && t.inputsys.xmouse<t.tcenterx+779 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
			{
				//  play game menu
				t.tquickhighlight=5;
			}
			if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+350 && t.inputsys.xmouse<t.tcenterx+548 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
			{
				//  twitch videos
				t.tquickhighlight=6;
			}
			if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+223 && t.inputsys.xmouse<t.tcenterx+340 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
			{
				// play PDF
				t.tquickhighlight=7;
			}
			if ( g.quickstartmenumode==1 && (t.inputsys.xmouse>t.tcenterx+583 && t.inputsys.xmouse<t.tcenterx+759 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
			{
				//  play video menu
				t.tquickhighlight=5;
			}
			if (  t.inputsys.mclick == 1 ) 
			{
				if (  t.tclicked == 0 ) 
				{
					if (  t.tquickhighlight == 1 ) 
					{
						//  toggle show on startup
						g.gshowonstartup=1-g.gshowonstartup ; t.tclicked=1;
					}
					if ( t.tquickhighlight == 3 || t.tquickhighlight == 4 ) 
					{
						//  close
						t.tclosequick=1;
						t.tclicked=1;
					}
					if (  t.tquickhighlight == 5 ) 
					{
						// jump to game/video menu
						g.quickstartmenumode = 1 - g.quickstartmenumode;

						// wait for click to be released
						do { t.inputsys.mclick=MouseClick(); FastSyncInputOnly(); } while ( !( t.inputsys.mclick == 0 ) );
					}
					if (  t.tquickhighlight == 6 ) 
					{
						// jump to twitch videos
						ExecuteFile ( "https://www.game-guru.com/live-streams","","",0 );
						t.tclicked=1;
					}
					if (  t.tquickhighlight == 7 ) 
					{
						// jump to PDF file
						// Now hosting it online
						//ExecuteFile ( "languagebank\\english\\artwork\\GameGuru - Getting Started Guide.pdf","","",0 );
						ExecuteFile ( "https://www.game-guru.com/downloads/pdfs/GameGuru%20-%20Getting%20Started%20Guide.pdf", "" , "", 0 );
						t.tclicked=1;
					}
				}
			}
			else
			{
				t.tclicked=0;
			}
			if ( g.gshowonstartup == 1 ) 
			{
				PasteImage (  g.editorimagesoffset+9,t.tcenterx+30,t.tcentery+522,1 );
			}
			if ( t.tquickhighlight == 3 ) 
			{
				PasteImage (  g.interactiveimageoffset+15,t.tcenterx+789,t.tcentery+510 );
			}
			if ( t.tquickhighlight == 4 ) 
			{
				PasteImage (  g.editorimagesoffset+44,t.tcenterx+860,t.tcentery+39 );
			}
			if ( t.tquickhighlight == 5 ) 
			{
				if ( g.quickstartmenumode==0 )
					PasteImage (  g.editorimagesoffset+42,t.tcenterx+558,t.tcentery+510 );
				else
					PasteImage (  g.editorimagesoffset+43,t.tcenterx+583,t.tcentery+510 );
			}
			if ( t.tquickhighlight == 6 ) 
			{
				if ( g.quickstartmenumode==0 )
				{
					PasteImage (  g.editorimagesoffset+47,t.tcenterx+350,t.tcentery+510 );
				}
			}
			if ( t.tquickhighlight == 7 ) 
			{
				if ( g.quickstartmenumode==0 )
				{
					PasteImage (  g.editorimagesoffset+48,t.tcenterx+223,t.tcentery+510 );
				}
			}
			Sync (  );
		}
		do
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
		} while ( !(  t.inputsys.kscancode == 0 && t.inputsys.mclick == 0 ) );

		// 141015 - always write file (to store videos played)
		t.tfile_s=g.fpscrootdir_s+"\\showonstartup.ini";
		DeleteAFile (  t.tfile_s.Get() );
		if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
		OpenToWrite (  1,t.tfile_s.Get() );
		WriteString (  1, cstr(g.gshowonstartup).Get() );
		WriteString (  1, "10");
		for ( int n=0; n<10; n++ )
		{
			WriteString (  1, cstr(g.videoMenuPlayed[n]).Get() );
		}
		CloseFile (  1 );
		editor_unfreezeanimations();
	}
	*/

	//  reset before leave
	t.inputsys.kscancode=0;
	t.inputsys.mclick=0;
	t.inputsys.xmouse=0;
	t.inputsys.ymouse=0;
}

/*
void editor_showreviewrequest ( void )
{
	if ( g.reviewRequestReminder == 1 ) 
	{
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetEventAndWait (  1 );
		do
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
			FastSync (  );
		} while ( !(  t.inputsys.kscancode == 0 && t.inputsys.mclick == 0 ) );
		t.imgx_f=ImageWidth(g.editorimagesoffset+11);
		t.imgy_f=ImageHeight(g.editorimagesoffset+11);
		Sprite ( 123, -10000, -10000, g.editorimagesoffset+11 );
		SizeSprite (  123,t.imgx_f,t.imgy_f );
		t.tclicked=0 ; t.tclosequick=0;
		t.lastmousex=MouseX() ; t.lastmousey=MouseY();
		while (  t.inputsys.kscancode == 0 && EscapeKey() == 0 && t.tclosequick == 0 ) 
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
			if (  GetFileMapDWORD( 1, 908 ) == 1  )  break;
			if (  GetFileMapDWORD( 1, 516 )>0  )  break;
			if (  GetFileMapDWORD( 1, 400 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 404 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 408 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 434 ) == 1 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 758 ) != 0 ) { t.interactive.active = 0  ; break; }
			if (  GetFileMapDWORD( 1, 762 ) != 0 ) { t.interactive.active = 0  ; break; }
			t.terrain.gameplaycamera=0;
			terrain_shadowupdate ( );
			terrain_update ( );
			t.tcenterx=(GetChildWindowWidth(0)-t.imgx_f)/2;
			t.tcentery=(GetChildWindowHeight(0)-t.imgy_f)/2;
			PasteSprite ( 123,t.tcenterx,t.tcentery );
			t.inputsys.xmouse=((GetFileMapDWORD( 1, 0 )+0.0)/800.0)*GetChildWindowWidth(0);
			t.inputsys.ymouse=((GetFileMapDWORD( 1, 4 )+0.0)/600.0)*GetChildWindowHeight(0);
			t.tquickhighlight=0;
			if (  t.inputsys.xmouse>t.tcenterx+29 && t.inputsys.xmouse<t.tcenterx+56 && t.inputsys.ymouse>t.tcentery+521 && t.inputsys.ymouse<t.tcentery+549 ) 
			{
				// toggle tick
				t.tquickhighlight=1;
			}
			if (  t.inputsys.xmouse>t.tcenterx+356 && t.inputsys.xmouse<t.tcenterx+565 && t.inputsys.ymouse>t.tcentery+322 && t.inputsys.ymouse<t.tcentery+370 ) 
			{
				//  jump to link
				t.tquickhighlight=2;
			}
			if (  t.inputsys.xmouse>t.tcenterx+785 && t.inputsys.xmouse<t.tcenterx+890 && t.inputsys.ymouse>t.tcentery+506 && t.inputsys.ymouse<t.tcentery+561 ) 
			{
				//  close
				t.tquickhighlight=3;
			}
			if (  t.inputsys.mclick == 1 ) 
			{
				if (  t.tclicked == 0 ) 
				{
					if (  t.tquickhighlight == 1 ) 
					{
						//  toggle show on startup
						g.reviewRequestReminder=1-g.reviewRequestReminder; 
						t.tclicked=1;
					}
					if (  t.tquickhighlight == 2 ) 
					{
						//  jump to link
						ExecuteFile ( "http://store.steampowered.com/reviews/","","",0 );
						//g.reviewRequestReminder = 0; // by order of Rick who wants to hassle users some more..
						t.tclicked=1;
					}
					if (  t.tquickhighlight == 3 ) 
					{
						//  close
						t.tclosequick=1;
						t.tclicked=1;
					}
				}
			}
			else
			{
				t.tclicked=0;
			}
			if ( g.reviewRequestReminder == 0 ) 
			{
				PasteImage ( g.editorimagesoffset+9, t.tcenterx+30, t.tcentery+522, 1 );
			}
			if ( t.tquickhighlight == 2 ) 
			{
				PasteImage ( g.editorimagesoffset+46, t.tcenterx+356, t.tcentery+323, 1 );
			}
			if ( t.tquickhighlight == 3 ) 
			{
				PasteImage (  g.interactiveimageoffset+15,t.tcenterx+789,t.tcentery+510 );
			}
			Sync (  );
		}
		do
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
		} while ( !(  t.inputsys.kscancode == 0 && t.inputsys.mclick == 0 ) );
	}
	//  reset before leave
	t.inputsys.kscancode=0;
	t.inputsys.mclick=0;
	t.inputsys.xmouse=0;
	t.inputsys.ymouse=0;
	return;
}

void editor_showreviewrequest_check ( void )
{
	if ( g.reviewRequestMinuteCounter > 6000 )
	{
		// attempt a review request
		editor_showreviewrequest();

		// and also ensure not again for another X hours
		g.reviewRequestMinuteCounter = 0;
	}
}
*/

void editor_previewmapormultiplayer ( int iUseVRTest )
{
	//  store if project modified
	t.storeprojectmodified=g.projectmodified;
	int tstoreprojectmodifiedstatic = g.projectmodifiedstatic; 

	//  flag that we clicked TEST GAME
	t.interactive.testgameused=1;

	//  Before launch test game, check if enough contiguous
	checkmemoryforgracefulexit();

	//  First call will toggle keyboard/mouse back to BACKGROUND (to capture all direct data)
	SetWindowModeOn (  );

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
					strcat ( pEffectFilename, "apbr_anim.fx"); 
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
		terrain_fastveg_applyshader();
	}
	
	//  set-up test game screen prompt assets
	if ( t.game.runasmultiplayer == 1 ) 
	{
		loadscreenpromptassets(2);
		#ifdef PHOTONMP
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

	// Re-acquire indices now the lists have changed
	// takes visuals.sky$ visuals.terrain$ visuals.vegetation$
	visuals_updateskyterrainvegindex ( );

	// Ensure game visuals settings used
	t.gamevisuals.skyindex=t.visuals.skyindex;
	t.gamevisuals.sky_s=t.visuals.sky_s;
	t.gamevisuals.terrainindex=t.visuals.terrainindex;
	t.gamevisuals.terrain_s=t.visuals.terrain_s;
	t.gamevisuals.vegetationindex=t.visuals.vegetationindex;
	t.gamevisuals.vegetation_s=t.visuals.vegetation_s;
	t.visuals=t.gamevisuals;
	t.visuals.refreshshaders=1;
	t.visuals.refreshvegtexture=1;

	// Hide camera while prepare test map
	t.storecx_f=CameraPositionX();
	t.storecy_f=CameraPositionY();
	t.storecz_f=CameraPositionZ();

	// default start position is edit-camera XZ
	/* remove this so as not to influence start marker positioning
	t.terrain.playerx_f=CameraPositionX(0);
	t.terrain.playerz_f=CameraPositionZ(0);
	if ( t.terrain.TerrainID>0 ) 
	{
		t.terrain.playery_f=BT_GetGroundHeight(t.terrain.TerrainID,t.terrain.playerx_f,t.terrain.playerz_f)+150.0;
	}
	else
	{
		t.terrain.playery_f=1000.0+150.0;
	}
	t.terrain.playerax_f=0.0;
	t.terrain.playeray_f=0.0;
	t.terrain.playeraz_f=0.0;
	*/

	// store all editor entity positions and rotations
	t.storedentityelementlist=g.entityelementlist;
	t.storedentityviewcurrentobj=g.entityviewcurrentobj;
	Dim (  t.storedentityelement,g.entityelementlist );
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.storedentityelement[t.e]=t.entityelement[t.e];
	}

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

	// in VR, if controller powered down, need to jog it back to life
	if ( g.vrglobals.GGVREnabled == 2 )
	{
		GGVR_ReconnectWithHolographicSpaceControllers();
	}

	//
	// launch game root with IDE 'test at cursor position' settings
	//
	t.game.gameisexe=0;
	t.game.set.resolution=0;
	t.game.set.initialsplashscreen=0;
	t.game.set.ismapeditormode=0;
	game_masterroot ( iUseVRTest );
	t.game.set.ismapeditormode=1;

	// Restore entities (remove light map objects for return to IDE editor)
	lm_restoreall ( );

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

	// ensure all locked entity transparency shown again
	for ( t.tte = 1 ; t.tte<=  g.entityelementlist; t.tte++ )
	{
		if ( t.entityelement[t.tte].editorlock == 1 ) 
		{
			t.tobj=t.entityelement[t.tte].obj;
			if ( t.tobj>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					t.entityelement[t.tte].isclone=0;
					entity_converttoclonetransparent ( );
				}
			}
		}
		if ( t.entityelement[t.tte].underground == 1  )  t.entityelement[t.tte].beenmoved = 1;
	}

	// signal that we have finished Test Level, restore mapeditor windows
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetFileMapDWORD (  1, 970, 1 );
	SetEventAndWait (  1 );

	// wait until all mouse activity over and escape key released
	while ( MouseClick() != 0 ) {}
	while ( ScanCode() != 0 ) {}

	// Restore camera
	editor_restoreeditcamera ( );
	t.updatezoom=1;

	// Restore ambience for editor when done
	SetTextFont (  "Verdana"  ); SetTextToBold (  );
	Ink (  Rgb(255,255,225),0  ); SetTextSize (  26 );

	// restore object visibilities
	editor_refresheditmarkers ( );

	// remember game states for next time
	visuals_save ( );
	t.gamevisuals=t.visuals;

	// restore shader constants with editor visuals (and bring back some settings we want to retain)
	t.visuals=t.editorvisuals;
	t.visuals.skyindex=t.gamevisuals.skyindex;
	t.visuals.sky_s=t.gamevisuals.sky_s;
	t.visuals.terrainindex=t.gamevisuals.terrainindex;
	t.visuals.terrain_s=t.gamevisuals.terrain_s;
	t.visuals.vegetationindex=t.gamevisuals.vegetationindex;
	t.visuals.vegetation_s=t.gamevisuals.vegetation_s;

	// and refresh assets based on restore
	t.visuals.refreshshaders=1;
	visuals_loop ( );
	visuals_shaderlevels_update ( );

	// use infinilights to show dynamic lighting in editor
	lighting_init ( );

	// Second call will toggle keyboard/mouse back to FOREGROUND
	SetWindowModeOn ( );

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
	common_refreshDisplaySize ( );

	// restore if project modified
	t.tignoreinvalidateobstacles=1;
	g.projectmodified = t.storeprojectmodified ; if ( g.projectmodified == 1 ) gridedit_changemodifiedflag ( );
	g.projectmodifiedstatic = tstoreprojectmodifiedstatic; 
	t.tignoreinvalidateobstacles=0;

	// Something is clipping objects when returning to editor
	editor_loadcfg();
	editor_refreshcamerarange();
}

void editor_multiplayermode ( void )
{
	// check we are not in the importer or character creator
	editor_checkIfInSubApp ( );

	//  Record last edited project
	t.storeprojectfilename_s=g.projectfilename_s;

	//  Set multiplayer flags here
	t.game.runasmultiplayer=1;
	editor_previewmapormultiplayer ( 1 );

	//  As multiplayer can load OTHER things, restore level to state before we clicked MM button
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

return;

}

void editor_previewmap ( int iUseVRTest )
{
	//  check if we are in the importer or character creator, if we are, don't test ma
	editor_checkIfInSubApp ( );
	//  Set single player test game flags here
	t.game.runasmultiplayer=0;
	editor_previewmapormultiplayer ( iUseVRTest );
}

void input_getfilemapcontrols ( void )
{
	//  Update triggers and issue actions through filemapping system
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );

	// If signal recieved of file map change
	if ( 1 ) 
	{
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
			t.inputsys.mclick=GetFileMapDWORD( 1, 20 );
			if (  GetFileMapDWORD( 1, 28 ) == 1  )  t.inputsys.mclick = 2;
			if (  MouseClick() == 4  )  t.inputsys.mclick = 4;
			if (  t.interactive.insidepanel == 1  )  t.inputsys.mclick = 0;
			t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
			//SetCursor ( 50, 50 );
			//Print ( t.inputsys.kscancode );
			//if ( t.inputsys.kscancode == 69 )
			//{
			//	int lee=42;
			//}
		}

		//  show the KeyState (  if know what to press )
		if ( g.globals.fulldebugview == 1 ) debug_fulldebugview ( );

		//  extra mappings
		input_extramappings ( );

		//  Control keys direct from keyboard
		t.inputsys.keyreturn=GetFileMapDWORD( 1, 108 );
		t.inputsys.keyshift=GetFileMapDWORD( 1, 112 );
		t.inputsys.keyup=GetFileMapDWORD( 1, 120 );
		t.inputsys.keydown=GetFileMapDWORD( 1, 124 );
		t.inputsys.keyleft=GetFileMapDWORD( 1, 128 );
		t.inputsys.keyright=GetFileMapDWORD( 1, 132 );
		t.inputsys.keycontrol=GetFileMapDWORD( 1, 116 );
		t.inputsys.keyalt=GetFileMapDWORD( 1, 136 );
		if (  t.inputsys.kscancode == 32  )  t.inputsys.keyspace = 1; else t.inputsys.keyspace = 0;

		// will release keypress flag if no key AND no SHIFT!!
		if (t.inputsys.kscancode == 0 && t.inputsys.keyshift == 0) t.inputsys.keypressallowshift = 0;

		//  W,A,S,D in editor for scrolling about (easier for user)
		if (  t.inputsys.kscancode == 87  )  t.inputsys.keyup = 1;
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
				t.inputsys.mclick=2;
				t.inputsys.keyleft=0;
				t.inputsys.keyright=0;
				t.inputsys.keyup=0;
				t.inputsys.keydown=0;
			}
		}

		//  special trigger when click LIBRARY TAB, force into respective mode
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

		//  recent file list
		t.trecentfilechoice=GetFileMapDWORD( 1, 442 );
		if (  t.trecentfilechoice>0 ) 
		{
			//  retain choice for action at end of subroutine
			t.trecentfilechoice_s=GetFileMapString( 1, 1000 );
			SetFileMapDWORD (  1, 442, 0 );
		}

		//  termination trigger
		if (  GetFileMapDWORD( 1, 908 ) == 1 ) 
		{
			// show outtro message if free version mode
			if ( g.iFreeVersionModeActive == 1 )
			{
				t.inputsys.ignoreeditorintermination = 1;
				welcome_init(1);
				welcome_init(0);
				welcome_show(WELCOME_FREEINTROAPP);
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

		//  EDIT MENU
		if (  GetFileMapDWORD( 1, 446 ) == 1 ) {  t.inputsys.doundo = 1  ; SetFileMapDWORD (  1, 446, 0 ); }
		if (  GetFileMapDWORD( 1, 450 ) == 1 ) { t.inputsys.doredo = 1  ; SetFileMapDWORD (  1, 450, 0 ); }
		if (  GetFileMapDWORD( 1, 454 ) == 1 ) { t.inputsys.tselcontrol = 1  ; t.inputsys.tselcut = 1 ; t.inputsys.tselcopy = 1 ; SetFileMapDWORD (  1, 454, 0 ); }
		if (  GetFileMapDWORD( 1, 458 ) == 1 ) { t.inputsys.tselcontrol = 1  ; t.inputsys.tselcopy = 1 ; SetFileMapDWORD (  1, 458, 0 ); }

		//  Get toolbar triggers
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
					case 3 :
						editor_previewmap ( 1 );
					break;
				}
			}
			if ( t.toolbarset == 21 ) 
			{
				// HELP MENU Actions
				#if VRQUEST
				switch ( t.toolbarindex ) 
				{
					case 1 : editor_showhelppage ( 1 );  break;
					case 2 : editor_showhelppage ( 2 );  break;
					case 3 : editor_showhelppage ( 3 );  break;
				}
				#else
				 switch (  t.toolbarindex ) 
				 {
					case 1 :
						editor_showhelppage ( );
					break;
					case 2 :
						if (  t.interactive.active == 0  )  t.interactive.active = 2;
					break;
				 }
				#endif
			}
		}

		//  Clear toolbar index
		SetFileMapDWORD (  1, 200, 0 );
		SetFileMapDWORD (  1, 204, 0 );

		//  Clear deltas
		SetFileMapDWORD (  1, 8, 0 );
		SetFileMapDWORD (  1, 12, 0 );
		SetFileMapDWORD (  1, 16, 0 );

		//  Deactivate mouse if leave 3d area
		if (  t.inputsys.xmouse == -1 && t.inputsys.ymouse == -1 ) 
		{
			//editor_restoreentityhighlightobj ( );
			t.inputsys.xmouse=500000;
			t.inputsys.ymouse=0;
			t.inputsys.xmousemove=0;
			t.inputsys.ymousemove=0;
			t.inputsys.activemouse=0;
			t.inputsys.mclick=0;
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

		//  Convert FILE MAP COMM VALUES to DX INPUT CODES
		t.t_s="" ; t.tt=0;
		switch (  t.inputsys.kscancode ) 
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

		//  select items from editing to see values
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

		//  Record final translated key values
		t.inputsys.k_s=t.t_s ; t.inputsys.kscancode=t.tt;

		//  Determine if Library Selection Made
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
	}

	//  Ensure status bar is constantly updated
	++t.interfacestatusbarupdate;
	if (  t.interfacestatusbarupdate>30 ) 
	{
		//  cursor position
		if (  g.gridlayershowsingle == 1 ) 
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

	//  Update status bar out of action subroutines
	gridedit_updatestatusbar ( );

	//  Action after filemap activity
	if (  t.trecentfilechoice>0 ) 
	{
		//  save first
		gridedit_intercept_savefirst ( );
		if (  t.editorcanceltask == 0 ) 
		{
			//  go ahead, load direct (skip the open dialog)
			g.projectfilename_s=t.trecentfilechoice_s;
			gridedit_load_map ( );
		}
		t.trecentfilechoice=0;
	}
}

void editor_handlepguppgdn ( void )
{
	// changes and returns 't.tupdownstepvalue_f'
	if ( t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209 )
	{
		float fEntityStepSize = 5.0f;
		if ( t.gridentity > 0 && t.gridentityobj > 0 )
		{
			if ( ObjectExist(t.gridentityobj)==1 ) 
				fEntityStepSize = ObjectSizeY(t.gridentityobj,1);
		}
		else
		{
			if ( t.widget.activeObject>0 ) 
			{
				if ( ObjectExist(t.widget.activeObject)==1 ) 
					fEntityStepSize = ObjectSizeY(t.widget.activeObject,1);
			}
		}
		if (  t.gridentitygridlock >= 1  )
			t.tupdownstepvalue_f = 0.0;
		else
			t.tupdownstepvalue_f = 1.0;
		if ( t.gridentitygridlock>0 ) 
		{
			if (t.inputsys.keypressallowshift == 0 && (t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209))
			{
				if ( t.gridentitygridlock == 1  )  t.tupdownstepvalue_f = fEntityStepSize;
				if ( t.gridentitygridlock == 2  )  t.tupdownstepvalue_f = 100.0;
				t.inputsys.keypressallowshift = 1;
			}
			else
			{
				t.tupdownstepvalue_f=0;
			}
		}
		if (  t.inputsys.kscancode == 201 )  t.gridentityposy_f += t.tupdownstepvalue_f; 
		if (  t.inputsys.kscancode == 209 )  t.gridentityposy_f -= t.tupdownstepvalue_f; 
	}
}

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
		input_getfilemapcontrols ( );
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
	if (  t.inputsys.kscancode == 0  )  t.inputsys.keypress = 0;

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
			if (  t.inputsys.k_s == "b" && t.inputsys.keypress == 0 ) 
			{
				t.inputsys.keypress=1 ; t.gridentitygridlock=t.gridentitygridlock+1;
				if (  t.gridentitygridlock>2  )  t.gridentitygridlock = 0;
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

		//  Editing of Map
		if (  t.inputsys.k_s == ","  )  t.inputsys.dozoomin = 1;
		if (  t.inputsys.k_s == "."  )  t.inputsys.dozoomout = 1;

		//  TAB Key causes layer edit view control
		if (  t.inputsys.kscancode == 15 && t.inputsys.keypress == 0 ) { t.inputsys.dosinglelayer = 1  ; t.inputsys.keypress = 1; }

		//  F1 for help page
		if ( t.inputsys.kscancode == 59 ) editor_showhelppage ( 1 );
	}
	else
	{
		if (  t.inputsys.k_s == "r"  )  t.inputsys.dorotation = 1;
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
			t.zoomviewcamerarange_f -= (t.inputsys.wheelmousemove/10.0);
		}
		else
		{
			//  Non-Zoomed in View
			if (  t.inputsys.keycontrol == 0 ) 
			{
				if (  t.inputsys.wheelmousemove<0  )  t.inputsys.dozoomout = 1;
				if (  t.inputsys.wheelmousemove>0  )  t.inputsys.dozoomin = 1;
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
			g.waypointeditheight_f=1000.0;
		}
		t.waypointeditstyle=1 ; t.waypointeditstylecolor=0 ; t.waypointeditentity=0;
		t.mx_f=t.cx_f ; t.mz_f=t.cy_f  ; waypoint_createnew ( );
	}
}

void input_calculatelocalcursor ( void )
{
	//  04032015 - 014 - ensure the display updates to window size, so the cursor and picking match up again
	common_refreshDisplaySize ( );

	//  Early warning of resource VIDMEM loss, if matrix gone
	if (  MatrixExist(g.m4_projection) == 0 ) 
	{
		editor_detect_invalid_screen ( );
	}

	//  Local cursor calculation
	t.inputsys.picksystemused=0;
	if (  t.grideditselect != 4 ) 
	{
		//  do not change these values if in zoom mode
		SetCurrentCamera (  0 );
		SetCameraRange (  1,70000 );
		t.screenwidth_f=800.0;
		t.screenheight_f=600.0;
		GetProjectionMatrix (  g.m4_projection );
		GetViewMatrix (  g.m4_view );
		t.blank=InverseMatrix(g.m4_projection,g.m4_projection);
		t.blank=InverseMatrix(g.m4_view,g.m4_view);
		//  work out visible part of full backbuffer (i.e. 1212 of 1360)
		t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
		t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
		//  scale full mouse to fit in visible area
		t.tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/t.tadjustedtoareax_f;
		t.tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/t.tadjustedtoareay_f;
		//  stretch scaled-mouse to projected -1 to +1
		SetVector4 (  g.v4_far,(t.tadjustedtoareax_f*2)-1,-((t.tadjustedtoareay_f*2)-1),0,1 );
		t.tx_f=GetXVector4(g.v4_far);
		t.ty_f=GetYVector4(g.v4_far);
		t.tz_f=GetZVector4(g.v4_far);
		TransformVector4 (  g.v4_far,g.v4_far,g.m4_projection );
		t.tx_f=GetXVector4(g.v4_far);
		t.ty_f=GetYVector4(g.v4_far);
		t.tz_f=GetZVector4(g.v4_far);

		// works in DX9 (D3DXVec4Transform) but not DX11 (KMATRIX)
		//TransformVector4 (  g.v4_far,g.v4_far,g.m4_view );
		//t.tx_f=GetXVector4(g.v4_far);
		//t.ty_f=GetYVector4(g.v4_far);
		//t.tz_f=GetZVector4(g.v4_far);
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
		t.inputsys.localcurrentterrainheight_f=1000.0;
	}

	// when placing waypoints, include entities as 'ground' to check
	t.inputsys.originallocalx_f = t.inputsys.localx_f;
	t.inputsys.originallocaly_f = t.inputsys.localy_f;
	if ( t.gridentitysurfacesnap == 1 || t.onedrag > 0 )
	{
		// only when finding place to place entity
		if ( t.gridentity > 0 || t.onedrag > 0 ) 
		{
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
				fTDX = g.glastpickedx_f - CameraPositionX();
				fTDY = g.glastpickedy_f - CameraPositionY();
				fTDZ = g.glastpickedz_f - CameraPositionZ();
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
	OpenFileMap (  2, "FPSEXCHANGE" );
	SetFileMapDWORD (  2, 850, 1 );
	SetEventAndWait (  2 );
}

void editor_enableafterzoom ( void )
{
	OpenFileMap (  2, "FPSEXCHANGE" );
	SetFileMapDWORD (  2, 850, 0 );
	SetEventAndWait (  2 );
	editor_cutcopyclearstate ( );
}

void editor_init ( void )
{
	//  Load editor images
	SetMipmapNum(1); //PE: mipmaps not needed.
	#if VRQUEST
	 t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\branded\\quick-help.png";
	#else
	 t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\quick-help.png";
	#endif
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
	LoadImage (  "editors\\gfx\\9.png",g.editorimagesoffset+7 );
	LoadImage (  "editors\\gfx\\13.bmp",g.editorimagesoffset+13 );
	LoadImage (  "editors\\gfx\\14.png",g.editorimagesoffset+14 );
	LoadImage (  "editors\\gfx\\18.png",g.editorimagesoffset+18 );
	LoadImage (  "editors\\gfx\\26.bmp",g.editorimagesoffset+26 );
	LoadImage (  "editors\\gfx\\cursor.dds",g.editorimagesoffset+10 );

	//  F9 Edit Mode Graphical Prompts
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-terrain.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+21 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-entity.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+22 );
	t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\f9-help-conkit.png";
	LoadImage ( t.strwork.Get() ,g.editorimagesoffset+23 );

	// new images for editor extra help
	#ifdef VRQUEST
	 t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\branded\\testgamelayout-noweapons.png";
 	 LoadImage (  t.strwork.Get(), g.editorimagesoffset+27 );
	 t.strwork = ""; t.strwork = t.strwork + "languagebank\\"+g.language_s+"\\artwork\\branded\\testgamelayout-vr.png";
 	 LoadImage (  t.strwork.Get(), g.editorimagesoffset+28 );
	#endif

	//  Also loaded by interactive mode when active
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\interactive\\close-highlight.png",g.interactiveimageoffset+15 );

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
	t.borderx_f=1024.0*50.0 ; t.cx_f=t.borderx_f/2.0;
	t.bordery_f=1024.0*50.0 ; t.cy_f=t.bordery_f/2.0;
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
				editor_restoreentityhighlightobj ( );
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
	SetCurrentCamera (  0 );
	PositionCamera (  t.cx_f,600*t.gridzoom_f,t.cy_f );
	PointCamera (  t.cx_f,0,t.cy_f );
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
	t.tstoryzoneincluded=25;
	if ( g.vrqcontrolmode != 0 )
		t.tstoryzoneincluded=23;

	//  Default markers
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
		SetDir (  "ebebank"  );
		UnDim ( t.filelist_s );
		buildfilelist("_builder","");
		SetDir ( pOld );
		int iExtraPFBCount = 0;
		if ( ArrayCount(t.filelist_s)>0 ) 
		{
			for ( t.chkfile = 0; t.chkfile <= ArrayCount(t.filelist_s); t.chkfile++ )
			{
				t.file_s = t.filelist_s[t.chkfile];
				if (  t.file_s != "." && t.file_s != ".." ) 
				{
					if ( cstr(Lower(Right(t.file_s.Get(),4))) == ".pfb" ) 
					{
						// ignore items in default list
						bool bIgnore = false;
						for ( int dl = 1; dl < iFirstFreeSlot; dl++ )
						{
							LPSTR pThisOne = t.ebebank_s[dl].Get();
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
							if ( stricmp ( pNameOnly, t.file_s.Get() )==NULL )
								bIgnore = true;
						}
						if ( bIgnore == false )
						{
							// add to list
							t.ebebank_s[iFirstFreeSlot+iExtraPFBCount] = cstr("ebebank\\_builder\\") + Left(t.file_s.Get(),Len(t.file_s.Get()));

							// next slot
							iExtraPFBCount++;
							if ( iExtraPFBCount>100 ) iExtraPFBCount = 100;
						}
					}
				}
			}
			t.strwork = ""; t.strwork = t.strwork + "total extra PFBs="+Str(iExtraPFBCount);
			timestampactivity(0, t.strwork.Get() );
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
		if ( t.entityprofile[t.locallibraryentidmaster].isebe != 0 )
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
	OpenFileMap (  1, "FPSEXCHANGE" );

	//  Marker Defaults
	g.entidmaster=0;

	//  filllibrary with segments and entities from default prefabs (temp as is above)
	editor_filllibrary ( );
	editor_leftpanelreset ( );
}

void editor_filemapinit ( void )
{
	//  Open for some Defaults for Editor
	OpenFileMap (  1, "FPSEXCHANGE" );

	//  Set default mouse position and visibility
	SetFileMapDWORD (  1, 0, 400 );
	SetFileMapDWORD (  1, 4, 300 );
	SetEventAndWait (  1 );

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
		t.grideditselect = ReadLong ( 1 );
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
				//  first cancel any widget that might be opened
				widget_switchoff ( );
				// remove any entity group rubber band highlighting
				gridedit_clearentityrubberbandlist();
				// the entity ID we are adding
				t.gridentity = t.inputsys.constructselection;
				if ( t.entityprofile[t.gridentity].isebe > 0 )
				{
					// create unique entid and go to entity placement mode
					char pEBEFile[512];
					strcpy ( pEBEFile, t.entitybank_s[t.gridentity].Get());
					t.addentityfile_s = cstr(Left(pEBEFile,strlen(pEBEFile)-4)) + cstr(".fpe");

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
						CopyFile ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
						sDDSSourceFile = cstr(pFinalPathOnly) + sUniqueFilename + cstr("_N.dds");
						sDDSFile = g.mysystem.levelBankTestMap_s + sUniqueFilename + cstr("_N.dds");
						if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
						CopyFile ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
						sDDSSourceFile = cstr(pFinalPathOnly) + sUniqueFilename + cstr("_S.dds");
						sDDSFile = g.mysystem.levelBankTestMap_s + sUniqueFilename + cstr("_S.dds");
						if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
						CopyFile ( sDDSSourceFile.Get(), sDDSFile.Get(), FALSE );
					}
					else
					{
						// EBE not present, which means user protected it (not an editable EBE any more)
						if ( stricmp ( pEBEFile, "..\\ebebank\\_builder\\New Site.fpe" ) != NULL )
						{
							// except New Site of course
							t.entityprofile[t.gridentity].isebe = 0;
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
				t.gridentityrotatex_f=t.entityprofile[t.gridentity].rotx;
				t.gridentityrotatey_f=t.entityprofile[t.gridentity].roty;
				t.gridentityrotatez_f=t.entityprofile[t.gridentity].rotz;
				t.gridentityscalex_f=t.entityprofile[t.gridentity].scale;
				t.gridentityscaley_f=t.entityprofile[t.gridentity].scale;
				t.gridentityscalez_f=t.entityprofile[t.gridentity].scale;
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
						g.waypointeditheight_f=1000.0;
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
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 48, 0 );
		SetEventAndWait (  1 );
		//CloseFileMap (  1 );
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
		t.cameraviewmode=2;
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
	t.ttterrheighthere_f=1000.0;
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
		if (  t.gridentitydroptoground >= 1 && t.gridentitydroptoground <= 2 ) 
		{
			t.thardauto=1  ; editor_findentityground ( );
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
		SetCameraRange ( t.tcamneardistance_f, 70000 );
	}
	else
	{
		//  top down camera distance
		SetCameraRange (  t.tcamneardistance_f,t.tcamrange_f );
	}
}

void editor_mainfunctionality ( void )
{

//  Rotation of entity
if (  t.grideditselect == 5 ) 
{
	//  do not rotate light or trigger entity
	//PE: Allow light rotation rem: t.entityprofile[t.gridentity].ismarker != 2 &&  t.entityprofile[t.gridentity].ismarker != 3
	if (   t.entityprofile[t.gridentity].ismarker != 3 ) 
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
		if (  t.widget.pickedObject != 0 && t.widget.pickedEntityIndex>0 ) 
		{
			//  Rotation control of widget controlled entity
			if (  t.inputsys.domodeterrain == 0 && t.inputsys.domodeentity == 0 ) 
			{
				//  avoid interference from terrain/entity mode change
				float fStoreOrigYAngle = t.entityelement[t.widget.pickedEntityIndex].ry;
				if (  t.inputsys.dorotation == 1 ) 
				{
					t.entityelement[t.widget.pickedEntityIndex].ry=t.entityelement[t.widget.pickedEntityIndex].ry+t.tspeedofrot_f;
				}
				if (  t.inputsys.doentityrotate == 1  )  t.entityelement[t.widget.pickedEntityIndex].rx = t.entityelement[t.widget.pickedEntityIndex].rx-t.tspeedofrot_f;
				if (  t.inputsys.doentityrotate == 2  )  t.entityelement[t.widget.pickedEntityIndex].rx = t.entityelement[t.widget.pickedEntityIndex].rx+t.tspeedofrot_f;
				if (  t.inputsys.doentityrotate == 3  )  t.entityelement[t.widget.pickedEntityIndex].ry = t.entityelement[t.widget.pickedEntityIndex].ry-t.tspeedofrot_f;
				if (  t.inputsys.doentityrotate == 4  )  t.entityelement[t.widget.pickedEntityIndex].ry = t.entityelement[t.widget.pickedEntityIndex].ry+t.tspeedofrot_f;
				if (  t.inputsys.doentityrotate == 5  )  t.entityelement[t.widget.pickedEntityIndex].rz = t.entityelement[t.widget.pickedEntityIndex].rz-t.tspeedofrot_f;
				if (  t.inputsys.doentityrotate == 6  )  t.entityelement[t.widget.pickedEntityIndex].rz = t.entityelement[t.widget.pickedEntityIndex].rz+t.tspeedofrot_f;

				//PE: Update light data for spot.
				if(t.entityelement[t.widget.pickedEntityIndex].eleprof.usespotlighting)
					lighting_refresh();

				if (  t.entityelement[t.widget.pickedEntityIndex].obj>0 ) 
				{
					int iTargetCenterObject = t.entityelement[t.widget.pickedEntityIndex].obj;
					float fOldActiveObjectRY = ObjectAngleY(iTargetCenterObject);
					// 061115 - if rubber band selection, include them in the rotation of Y only
					if (  ObjectExist ( iTargetCenterObject ) == 1 ) 
					{
						if ( g.entityrubberbandlist.size() > 0 )
						{
							// rotate all the grouped entities and move around Y axis of widget as pivot
							float fMovedActiveObjectRY = t.entityelement[t.widget.pickedEntityIndex].ry - fOldActiveObjectRY;
							if ( fMovedActiveObjectRY != 0.0f )
							{
								g.fForceYRotationOfRubberBandFromKeyPress = fMovedActiveObjectRY;
								t.entityelement[t.widget.pickedEntityIndex].ry = fStoreOrigYAngle;
							}
						}
						else
						{
							RotateObject (  t.entityelement[t.widget.pickedEntityIndex].obj,t.entityelement[t.widget.pickedEntityIndex].rx,t.entityelement[t.widget.pickedEntityIndex].ry,t.entityelement[t.widget.pickedEntityIndex].rz );
						}
					}
				}
			}
			//  Find Floor (  control of widget controlled entity or Raise/lower with PGUP and PGDN )
			if (  t.inputsys.keyreturn == 1 || t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209 ) 
			{
				t.tforceentityfindfloor=t.widget.pickedEntityIndex;
				t.tforcepguppgdnkeys=1  ; editor_forceentityfindfloor ( );
			}
		}
		else
		{
			if (  t.inputsys.domodeterrain == 0 && t.inputsys.domodeentity == 0 ) 
			{
				//  avoid interference from terrain/entity mode change
				if (  t.inputsys.dorotation == 1 ) { t.gridentityrotatey_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
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
if ( t.inputsys.doundo == 1 ) editor_undo ( );
if ( t.inputsys.doredo == 1  )editor_redo ( );

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
	if ( t.inputsys.rubberbandmode == 0 && t.ebe.on == 0 )
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
			t.gridzoom_f -= 0.6f;
		}
		else
		{
			t.gridzoom_f -= 0.1f;
		}
	}
	if (  ((t.inputsys.dozoomout == 1 && t.inputsys.keypress == 0) || t.tspecialgridzoomadjustment == 2) && t.gridzoom_f<40.0 ) 
	{
		t.updatezoom=1;
		if (  t.inputsys.keyshift == 1 ) 
		{
			t.gridzoom_f += 0.6f;
		}
		else
		{
			t.gridzoom_f += 0.1f;
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

return;

}

void editor_forceentityfindfloor ( void )
{
	//  receives; tforceentityfindfloor
	t.storegridentityinzoomview=t.gridentityinzoomview ; t.gridentityinzoomview=t.tforceentityfindfloor;
	t.storegridentityposy_f=t.gridentityposy_f ; t.gridentityposy_f=t.entityelement[t.gridentityinzoomview].y;
	int iEntPassMax = 1;
	if ( g.entityrubberbandlist.size() > 0 ) iEntPassMax = g.entityrubberbandlist.size();
	for ( int iEntPass=0; iEntPass<iEntPassMax; iEntPass++ )
	{
		// which entity are we dealing with
		int e = t.gridentityinzoomview;
		if ( g.entityrubberbandlist.size() > 0 ) 
			e = g.entityrubberbandlist[iEntPass].e;

		// if RETURN key pressed
		if ( t.inputsys.keyreturn == 1 ) 
		{
			// store globs in store
			t.storegridentitydroptoground=t.gridentitydroptoground;
			t.storegridentityposx_f=t.gridentityposx_f;
			t.storegridentityposz_f=t.gridentityposz_f;
			t.storegridentityobj=t.gridentityobj;
			t.storegridentityposoffground=t.gridentityposoffground;

			t.gridentitydroptoground=1;
			t.gridentityposx_f=t.entityelement[e].x;
			t.gridentityposz_f=t.entityelement[e].z;
			t.gridentityobj=t.entityelement[e].obj;
			t.thardauto = 1; editor_findentityground ( );
			if (  t.gridentityposoffground == 0 ) 
			{
				if (  t.terrain.TerrainID>0 ) 
				{
					t.gridentityposy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.gridentityposx_f,t.gridentityposz_f);
				}
				else
				{
					t.gridentityposy_f=1000.0;
				}
				if (  t.entityprofile[t.gridentity].ismarker != 0  )  t.gridentityposy_f = t.gridentityposy_f+t.entityprofile[t.gridentity].offy;
				if (  t.entityprofile[t.gridentity].defaultheight != 0  )  t.gridentityposy_f = t.gridentityposy_f+t.entityprofile[t.gridentity].defaultheight;
			}
			t.entityelement[e].x=t.gridentityposx_f;
			t.entityelement[e].z=t.gridentityposz_f;

			// restore globs from store
			t.gridentitydroptoground=t.storegridentitydroptoground;
			t.gridentityposx_f=t.storegridentityposx_f;
			t.gridentityposz_f=t.storegridentityposz_f;
			t.gridentityobj=t.storegridentityobj;
			t.gridentityposoffground=t.storegridentityposoffground;
			t.gridentityusingsoftauto=1;
			t.gridentitysurfacesnap=0;
		}
		if ( t.tforcepguppgdnkeys == 1 ) 
		{
			editor_handlepguppgdn();
		}
		if ( t.entityelement[e].y != t.gridentityposy_f )
		{
			t.entityelement[e].beenmoved = 1;
		}
		t.entityelement[e].y = t.gridentityposy_f;
		if ( t.entityelement[e].obj>0 ) 
		{
			if (  ObjectExist(t.entityelement[e].obj) == 1 ) 
			{
				PositionObject ( t.entityelement[e].obj,t.entityelement[e].x,t.entityelement[t.widget.pickedEntityIndex].y,t.entityelement[e].z );
			}
		}
	}
	t.gridentityposy_f=t.storegridentityposy_f;
	t.gridentityinzoomview=t.storegridentityinzoomview;
}

void editor_viewfunctionality ( void )
{
	//  map view controls
	if (  t.grideditselect == 3 ) 
	{
		if (  t.inputsys.mclick == 1 ) 
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

	//  zoom view controls
	if (  t.grideditselect == 4 ) 
	{

		//  can repos and rotate non-editor-entities
		if (  t.entityelement[t.gridentityinzoomview].editorfixed == 0 ) 
		{

			//  position adjustment
			t.tposadjspeed_f=1.0;
			if (  t.inputsys.keycontrol == 1  )  t.tposadjspeed_f = 0.05f;
			if (  t.inputsys.dozoomviewmovex == 1  )  t.zoomviewtargetx_f -= t.tposadjspeed_f;
			if (  t.inputsys.dozoomviewmovex == 2  )  t.zoomviewtargetx_f += t.tposadjspeed_f;
			if (  t.inputsys.dozoomviewmovey == 1  )  t.zoomviewtargety_f -= t.tposadjspeed_f;
			if (  t.inputsys.dozoomviewmovey == 2  )  t.zoomviewtargety_f += t.tposadjspeed_f;
			if (  t.inputsys.dozoomviewmovez == 1  )  t.zoomviewtargetz_f -= t.tposadjspeed_f;
			if (  t.inputsys.dozoomviewmovez == 2  )  t.zoomviewtargetz_f += t.tposadjspeed_f;

			//  rotation adjustment
			//PE: rotate lights rem: t.entityprofile[t.gridentity].ismarker != 2 &&
			if (  t.entityprofile[t.gridentity].ismarker != 3 ) 
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
				if (  t.inputsys.dorotation == 1 ) {  t.zoomviewtargetry_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if (  t.inputsys.dozoomviewrotatex == 1 ) { t.zoomviewtargetrx_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
				if (  t.inputsys.dozoomviewrotatex == 2 ) { t.zoomviewtargetrx_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 0; }
				if (  t.inputsys.dozoomviewrotatey == 1 ) { t.zoomviewtargetry_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if (  t.inputsys.dozoomviewrotatey == 2 ) { t.zoomviewtargetry_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 1; }
				if (  t.inputsys.dozoomviewrotatez == 1 ) { t.zoomviewtargetrz_f -= t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
				if (  t.inputsys.dozoomviewrotatez == 2 ) { t.zoomviewtargetrz_f += t.tspeedofrot_f  ; t.gridentityrotateaxis = 2; }
				if (  t.inputsys.dozoomviewrotatex >= 98 ) 
				{
					if (  t.inputsys.dozoomviewrotatex == 98 ) 
					{
						if (  t.gridentityrotateaxis == 0  )  t.zoomviewtargetrx_f = 0;
						if (  t.gridentityrotateaxis == 1  )  t.zoomviewtargetry_f = 0;
						if (  t.gridentityrotateaxis == 2  )  t.zoomviewtargetrz_f = 0;
					}
					if (  t.inputsys.dozoomviewrotatex == 99 ) 
					{
						t.zoomviewtargetrx_f=0;
						t.zoomviewtargetry_f=0;
						t.zoomviewtargetrz_f=0;
					}
				}
			}

			//  update gridentity vars for visual
			t.gridentityposx_f=t.zoomviewtargetx_f;
			t.gridentityposy_f=t.zoomviewtargety_f;
			t.gridentityposz_f=t.zoomviewtargetz_f;
			t.gridentityrotatex_f=t.zoomviewtargetrx_f;
			t.gridentityrotatey_f=t.zoomviewtargetry_f;
			t.gridentityrotatez_f=t.zoomviewtargetrz_f;

			//  aply grid if 5x5
			if (  t.gridentitygridlock == 1 ) 
			{
				t.gridentityposx_f=(int(t.gridentityposx_f/5)*5);
				t.gridentityposz_f=(int(t.gridentityposz_f/5)*5);
			}
		}

		//  mouselook mode on/off RMB
		OpenFileMap (  1, "FPSEXCHANGE" );
		if (  t.inputsys.mclick == 2 ) 
		{
			//  center mouse
			SetFileMapDWORD (  1, 48, 1 );
			//  camera position
			t.zoomviewcameraangle_f += t.inputsys.xmousemove/2.0;
			t.zoomviewcameraheight_f -= t.inputsys.ymousemove/1.5;
		}
		else
		{
			SetFileMapDWORD (  1, 48, 0 );
		}
		SetEventAndWait (  1 );
		//CloseFileMap (  1 );

		//  exit zoom view
		if ( t.inputsys.mclick == 1  )  t.tpressedtoleavezoommode = 1;
		if ( t.inputsys.mclick == 0 && t.tpressedtoleavezoommode == 1 )  t.tpressedtoleavezoommode = 2;
		if ( (t.tpressedtoleavezoommode == 2 || t.inputsys.kscancode == 211) || t.editorinterfaceleave == 1 ) 
		{
			//  leave zoomview
			t.inputsys.doautozoomview=1;

			//  reset mouse click (must release LMB before zoom mode ends)
			t.tpressedtoleavezoommode=0;

			//  close any property window
			interface_closepropertywindow ( );
			t.editorinterfaceleave=0;

			//  310315 - Ensure clipping is restored when return
			t.updatezoom=1;

			//  place entity on the map
			if ( t.gridentityinzoomview>0 ) 
			{
				//  DELETE key deletes entity no matter what (for fixed entities too)
				if ( t.gridentity != 0 && t.inputsys.kscancode == 211 ) 
				{
					//  Delete any associated waypoint/trigger zone
					t.waypointindex=t.grideleprof.trigger.waypointzoneindex;
					if (  t.waypointindex>0 ) 
					{
						t.w=t.waypoint[t.waypointindex].start;
						waypoint_delete ( );
					}
					t.grideleprof.trigger.waypointzoneindex=0;

					//  And now delete entity from cursor
					if (  t.gridentityobj == 0 ) 
					{
						DeleteObject (  t.gridentityobj );
						t.gridentityobj=0;
					}
				}
				else
				{
					//  Add entity back into map
					gridedit_addentitytomap ( );
					t.gridentityinzoomview=0;

					//  Hide widget to make clean return to editor
					t.widget.pickedObject=0  ; widget_updatewidgetobject ( );
				}

				//  Reset cursor object settings
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
			t.tbestdist_f=99999 ; t.tbesty_f=0;
			t.tto_f=t.gridentityposy_f-200.0;
			for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
			{
				if (  t.thardauto == 1 ) 
				{
					//  regular
					if (  t.entityelement[t.e].editorlock == 0 ) 
					{
						if (  t.inputsys.keyshift == 0 ) 
						{
							//  if close to building (clipping top of it), do not exceed clip theshold
							t.tfrom_f=t.gridentityposy_f+50000.0;
							if (  t.tfrom_f>t.gridtrueslicey_f  )  t.tfrom_f = t.gridtrueslicey_f;
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
					t.trygridentityposy_f=1000.0;
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

return;

}

void editor_camera ( void )
{

//  Camera Mode
switch (  t.cameraviewmode ) 
{
	case 0:

		//  Control free flight camera viewing angle (mouselook)
		if (  g.gminvert == 1  )  t.ttmousemovey = MouseMoveY()*-1; else t.ttmousemovey = MouseMoveY();
		t.cammousemovex_f=MouseMoveX();
		t.cammousemovey_f=t.ttmousemovey;
		if (  t.inputsys.mclick == 0  )  t.inputsys.mclickreleasestate = 0;
		t.trmb=0;
		if (  t.inputsys.mclick == 2 && t.inputsys.mclickreleasestate == 0  )  t.trmb = 1;
		if (  t.inputsys.mclick == 4 && t.inputsys.mclickreleasestate == 0  )  t.trmb = 2;
		if (  t.trmblock == 0 ) 
		{
			if (  t.cammousemovex_f != 0 || t.cammousemovex_f != 0 || t.inputsys.kscancode != 0  )  t.trmblock = 1;
		}
		else
		{
			if (  t.inputsys.mclick == 0  )  t.trmblock = 0;
		}
		if (  t.trmblock == 0  )  t.trmb = 0;
		if (  g.globals.disablefreeflight == 1  )  t.trmb = 0;
		//if ( (t.inputsys.mclick & 1) == 1 )  t.trmb = 0; // 170616 - if left button down, disable right mouse free flight (for EBE editing)
		if (  t.trmb != 0 ) 
		{
			if (  g.mouseishidden == 0 ) 
			{
				game_hidemouse ( );
				t.editorfreeflight.storemousex=t.inputsys.xmouse;
				t.editorfreeflight.storemousey=t.inputsys.ymouse;
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
				if (  t.trmb == 1 ) 
				{
					//  rotate with RMB
					t.tRotationDivider_f = 5.0;
					t.editorfreeflight.c.angx_f=CameraAngleX()+(t.cammousemovey_f/t.tRotationDivider_f);
					t.editorfreeflight.c.angy_f=CameraAngleY()+(t.cammousemovex_f/t.tRotationDivider_f);
					if (  t.editorfreeflight.c.angx_f>180.0f  )  t.editorfreeflight.c.angx_f = t.editorfreeflight.c.angx_f-360.0f;
					if (  t.editorfreeflight.c.angx_f<-89.999f  )  t.editorfreeflight.c.angx_f = -89.999f;
					if (  t.editorfreeflight.c.angx_f>89.999f  )  t.editorfreeflight.c.angx_f = 89.999f;
				}
			}
		}
		else
		{
			if (  g.mouseishidden == 1 ) 
			{
				t.tideframestartx=70 ; t.tideframestarty=15;
				t.inputsys.xmouse=((t.tideframestartx+t.editorfreeflight.storemousex+0.0)/800.0)*(GetDisplayWidth()+0.0);
				t.inputsys.ymouse=((t.tideframestarty+t.editorfreeflight.storemousey+0.0)/600.0)*(GetDisplayHeight()+0.0);
				game_showmouse ( );
				t.terrain.X_f=999999 ; t.terrain.Y_f=999999;
			}
		}

		//  Handle free flight camea movement
		if (  t.editorfreeflight.mode == 0 ) 
		{
			t.editorfreeflight.c.x_f=t.cx_f;
			t.editorfreeflight.c.y_f=(600.0*t.gridzoom_f);
			t.editorfreeflight.c.z_f=t.cy_f;
			if (  t.inputsys.k_s == "f" && g.globals.disablefreeflight == 0 ) 
			{
				//  top down back to last free flight
				t.editorfreeflight.mode=3;
				if (  t.editorfreeflight.sused == 0 ) 
				{
					t.editorfreeflight.sused=1;
					t.editorfreeflight.s=t.editorfreeflight.c;
					t.tcurrenth_f=BT_GetGroundHeight(t.terrain.TerrainID,t.editorfreeflight.c.x_f,t.editorfreeflight.c.z_f);
					t.editorfreeflight.s.y_f=t.tcurrenth_f+100.0;
					t.editorfreeflight.s.angx_f=20.0;
					t.editorfreeflight.s.angy_f=0;
				}
			}
		}
		if (  t.editorfreeflight.mode == 1 ) 
		{
			if (  t.inputsys.k_s == "g" ) 
			{
				//  free flight to top down
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
			if ( t.ebe.active == 0 )
			{
				if (  t.inputsys.keycontrol == 0 ) 
				{
					if (  t.inputsys.wheelmousemove<0  )  t.plrkeyS = 1;
					if (  t.inputsys.wheelmousemove>0  )  t.plrkeyW = 1;
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
					//t.tffcspeed_f=50.0*g.timeelapsed_f; // 011215 - need more finer default move control in free flight mode
					t.tffcspeed_f=35.0*g.timeelapsed_f;
				}
			}
			if (  t.inputsys.k_s == "]" || t.inputsys.k_s == "\\"  )  t.traise_f = -90;
			if (  t.inputsys.k_s == "["  )  t.traise_f = 90;
			PositionCamera (  t.editorfreeflight.c.x_f,t.editorfreeflight.c.y_f,t.editorfreeflight.c.z_f );
			if (  t.plrkeyW == 1  )  MoveCamera (  t.tffcspeed_f );
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
		PositionCamera (  t.editorfreeflight.c.x_f,t.editorfreeflight.c.y_f,t.editorfreeflight.c.z_f );
		if (  t.editorfreeflight.mode == 0 ) 
		{
			PointCamera (  t.cx_f,0,t.cy_f );
		}
		if (  t.editorfreeflight.mode == 1 ) 
		{
			RotateCamera (  t.editorfreeflight.c.angx_f,t.editorfreeflight.c.angy_f,0 );
		}
		if (  t.editorfreeflight.mode == 2 ) 
		{
			t.editorfreeflight.c.angx_f=CurveAngle(90,CameraAngleX(),10.0);
			t.editorfreeflight.c.angy_f=CurveAngle(0,CameraAngleY(),10.0);
			RotateCamera (  t.editorfreeflight.c.angx_f,t.editorfreeflight.c.angy_f,0 );
		}
		if (  t.editorfreeflight.mode == 3 ) 
		{
			t.editorfreeflight.c.angx_f=CurveAngle(t.editorfreeflight.s.angx_f,CameraAngleX(),10.0);
			t.editorfreeflight.c.angy_f=CurveAngle(t.editorfreeflight.s.angy_f,CameraAngleY(),10.0);
			RotateCamera (  t.editorfreeflight.c.angx_f,t.editorfreeflight.c.angy_f,0 );
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
		t.ttxtwid=getbitmapfontwidth(t.t_s.Get(),2);
		pastebitmapfont(t.t_s.Get(),GetChildWindowWidth(0)-8-t.ttxtwid,4,2,228);

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

	break;
}
return;

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
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetFileMapDWORD (  1, 474, 0 );
	SetFileMapDWORD (  1, 478, 0 );
	SetFileMapDWORD (  1, 482, 0 );
	SetEventAndWait (  1 );
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
		//  redo last stage
		if (  t.entityundo.undoperformed == 1 ) 
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

//  checks if we are currently in a sub app and frees them if we are
}

void editor_checkIfInSubApp ( void )
{
	t.result = 0;
	if ( t.importer.loaded == 1 ) importer_free ( );
	if ( t.characterkit.loaded == 1 ) characterkit_free ( );
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

	//  first layer does NOT detect LARGE entities when 'INSIDE' mode active
	//  second layer then allows LARGE enitities, but could have exited early if first layer found one
	//  third layer does NOT detect MARKERS
	if (  currentlyover <= 0 ) { estart = 1  ; efinish = g.entityelementlist; }
	if (  currentlyover >= 1 ) { estart = currentlyover  ; efinish = currentlyover; }
	for ( tlayers = 0 ; tlayers<=  2; tlayers++ )
	{
		for ( e = estart ; e <= efinish; e++ )
		{
			// 301115 - skip if in marker mode and not a marker
			if ( t.gridentitymarkersmodeonly == 1 && t.entityprofile[t.entityelement[e].bankindex].ismarker==0 )
				continue;

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
									//  work out visible part of full backbuffer (i.e. 1212 of 1360)
									tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
									tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
									//  scale full mouse to fit in visible area
									tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/tadjustedtoareax_f;
									tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/tadjustedtoareay_f;
									//  then provide in a format for the pick-from-screen command
									#ifdef DX11
									tadjustedtoareax_f=tadjustedtoareax_f*(GetDisplayWidth()+0.0);
									tadjustedtoareay_f=tadjustedtoareay_f*(GetDisplayHeight()+0.0);
									#else
									tadjustedtoareax_f=tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
									tadjustedtoareay_f=tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
									#endif
									c=PickScreenObject(tadjustedtoareax_f,tadjustedtoareay_f,o,o);
									if (  c != 0  )  g.gentityundercursorlocked = e;
									tokay=0;
								}
							}
						}
						if (  tokay == 1 ) 
						{
							//  work out visible part of full backbuffer (i.e. 1212 of 1360)
							tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
							tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
							//  scale full mouse to fit in visible area
							tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/tadjustedtoareax_f;
							tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/tadjustedtoareay_f;
							//  then provide in a format for the pick-from-screen command
							#ifdef DX11
							tadjustedtoareax_f=tadjustedtoareax_f*(GetDisplayWidth()+0.0);
							tadjustedtoareay_f=tadjustedtoareay_f*(GetDisplayHeight()+0.0);
							#else
							tadjustedtoareax_f=tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
							tadjustedtoareay_f=tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
							#endif
							c=PickScreenObject(tadjustedtoareax_f,tadjustedtoareay_f,o,o);
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
									if ( bIsMarker==true || t.gridnearcameraclip == -1 || fTryLastPickedY < t.gridnearcameraclip + 20.0f )
									{
										g.glastpickedx_f = CameraPositionX() + GetPickVectorX();
										g.glastpickedy_f = fTryLastPickedY;
										g.glastpickedz_f = CameraPositionZ() + GetPickVectorZ();
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
				//EnableObjectZWrite ( t.entityelement[result].obj );
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
			int tobj = t.entityelement[e].obj;
			if ( tobj > 0 )
			{
				SetAlphaMappingOn ( tobj, 100 );
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
		sRubberBandType item;
		item.e = e;
		g.entityrubberbandlist.push_back ( item );
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

				if ( t.inputsys.mclick == 1 && t.inputsys.rubberbandmode == 0 && t.widget.activeObject == 0 )
				{
					// clear any previous highlights
					if ( t.inputsys.keycontrol ==  0 )
					{
						gridedit_clearentityrubberbandlist();
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

				if ( t.tentitytoselect > 0 && t.entityelement[t.tentitytoselect].obj > 0 ) 
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
					if ( t.widget.activeObject == 0 && t.inputsys.xmouse != 500000 ) 
					{
						if ( t.inputsys.mclick == 1 )
						{
							// start rubber band box
							if ( t.inputsys.rubberbandmode == 0 )
							{
								t.inputsys.rubberbandmode = 1;
								t.inputsys.spacekeynotreleased = 1;
								t.inputsys.rubberbandx = t.inputsys.xmouse;
								t.inputsys.rubberbandy = t.inputsys.ymouse;
							}
						}
					}
				}

				// 201015 - rubber band effect and control
				if ( t.inputsys.rubberbandmode == 1 )
				{
					if ( t.inputsys.xmouse == 500000 )
					{
						// mouse left area, cancel rubber band
						t.inputsys.rubberbandmode = 0;
					}
					else
					{
						// draw and detect
						float fMX = (GetChildWindowWidth(1)+0.0) / 800.0f;
						float fMY = (GetChildWindowHeight(1)+0.0) / 600.0f;
						
						// reverse bound box if inside out
						float fCurrentRubberBandX1 = t.inputsys.rubberbandx;
						float fCurrentRubberBandX2 = t.inputsys.xmouse;
						float fCurrentRubberBandY1 = t.inputsys.rubberbandy;
						float fCurrentRubberBandY2 = t.inputsys.ymouse;
						if ( fCurrentRubberBandX1 > fCurrentRubberBandX2 ) { float fStore = fCurrentRubberBandX1; fCurrentRubberBandX1 = fCurrentRubberBandX2; fCurrentRubberBandX2 = fStore; }
						if ( fCurrentRubberBandY1 > fCurrentRubberBandY2 ) { float fStore = fCurrentRubberBandY1; fCurrentRubberBandY1 = fCurrentRubberBandY2; fCurrentRubberBandY2 = fStore; }

						// detect all entities within box and highlight
						for ( int e = 1; e <= g.entityelementlist; e++ )
						{
							// 060116 - if locked and holding space, unlock it now
							if ( t.inputsys.keyspace != 0 ) t.entityelement[e].editorlock = false;

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
							if ( g.entityrubberbandlist.size() > 0 )
							{
								if ( g.entityrubberbandlist.size()==1 )
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
							Sprite ( 123, -10000, -10000, g.editorimagesoffset+14 );
							float fX1 = t.inputsys.rubberbandx*fMX;
							float fX2 = t.inputsys.xmouse*fMX;
							float fY1 = t.inputsys.rubberbandy*fMY;
							float fY2 = t.inputsys.ymouse*fMY;
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

			//  Entity Edit Mode
			if ( 1 ) 
			{
				//  entity placement update
				if (  t.inputsys.mclick == 2 ) 
				{
					//  070415 - in RMB mode, mouse pos is changed so HIDE gridentity obj as it shifts!
					t.gridentityposx_f=-999000;
					t.gridentityposz_f=-999000;
				}
				else
				{
					t.gridentityposx_f=t.inputsys.localx_f+t.inputsys.dragoffsetx_f;
					t.gridentityposz_f=t.inputsys.localy_f+t.inputsys.dragoffsety_f;
				}

				//  grid system for entities
				if (  t.gridentitygridlock == 1 ) 
				{
					//  small grid lock for better alignments
					t.gridentityposx_f=(int(t.gridentityposx_f/5)*5);
					t.gridentityposz_f=(int(t.gridentityposz_f/5)*5);
					//  special snap-to when edge of entity gets near another
					if (  t.gridentity>0 ) 
					{
						t.tobj=t.gridentityobj;
						if (  t.tobj>0 ) 
						{
							if (  ObjectExist(t.tobj) == 1 ) 
							{
								t.tsrcx_f=t.gridentityposx_f;
								t.tsrcy_f=t.gridentityposy_f;
								t.tsrcz_f=t.gridentityposz_f;
								t.tsrcradius_f=ObjectSize(t.tobj,1);
								t.tfindclosest=-1 ; t.tfindclosestbest_f=999999;
								for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
								{
									t.ttobj=t.entityelement[t.e].obj;
									if (  t.ttobj>0 ) 
									{
										if (  t.entityelement[t.e].bankindex == t.gridentity ) 
										{
											t.tdiffx_f=t.entityelement[t.e].x-t.tsrcx_f;
											t.tdiffy_f=t.entityelement[t.e].y-t.tsrcy_f;
											t.tdiffz_f=t.entityelement[t.e].z-t.tsrcz_f;
											t.tdiff_f=Sqrt(abs(t.tdiffx_f*t.tdiffx_f)+abs(t.tdiffy_f*t.tdiffy_f)+abs(t.tdiffz_f*t.tdiffz_f));
											t.tthisradius_f=ObjectSize(t.ttobj,1);
											if (  t.tdiff_f<t.tsrcradius_f+t.tthisradius_f && t.tdiff_f<t.tfindclosestbest_f ) 
											{
												t.tfindclosestbest_f=t.tdiff_f;
												t.tfindclosest=t.e;
											}
										}
									}
								}
								if (  t.tfindclosest != -1 ) 
								{
									//  go through 6 magnet points of the src entity
									t.tmag1sizex_f=ObjectSizeX(t.tobj,1)/2;
									t.tmag1sizey_f=ObjectSizeY(t.tobj,1)/2;
									t.tmag1sizez_f=ObjectSizeZ(t.tobj,1)/2;
									t.tmag2sizex_f=ObjectSizeX(t.entityelement[t.tfindclosest].obj,1)/2;
									t.tmag2sizey_f=ObjectSizeY(t.entityelement[t.tfindclosest].obj,1)/2;
									t.tmag2sizez_f=ObjectSizeZ(t.entityelement[t.tfindclosest].obj,1)/2;
									if (  ObjectExist(g.entityworkobjectoffset) == 0 ) { MakeObjectCube (  g.entityworkobjectoffset,40  ) ; HideObject (  g.entityworkobjectoffset ); }
									t.tbestmag_f=99999 ; t.tbestmag2id=-1;
									for ( t.magid = 1 ; t.magid<=  6; t.magid++ )
									{
										t.tmagx_f=t.gridentityposx_f;
										t.tmagy_f=t.gridentityposy_f;
										t.tmagz_f=t.gridentityposz_f;
										PositionObject (  g.entityworkobjectoffset,t.tmagx_f,t.tmagy_f,t.tmagz_f );
										RotateObject (  g.entityworkobjectoffset,ObjectAngleX(t.tobj),ObjectAngleY(t.tobj),ObjectAngleZ(t.tobj) );
										if (  t.magid == 1  )  MoveObjectLeft (  g.entityworkobjectoffset,t.tmag1sizex_f );
										if (  t.magid == 2  )  MoveObjectRight (  g.entityworkobjectoffset,t.tmag1sizex_f );
										if (  t.magid == 3  )  MoveObjectUp (  g.entityworkobjectoffset,t.tmag1sizey_f );
										if (  t.magid == 4  )  MoveObjectDown (  g.entityworkobjectoffset,t.tmag1sizey_f );
										if (  t.magid == 5  )  MoveObject (  g.entityworkobjectoffset,t.tmag1sizez_f );
										if (  t.magid == 6  )  MoveObject (  g.entityworkobjectoffset,t.tmag1sizez_f*-1 );
										t.tmagx_f=ObjectPositionX(g.entityworkobjectoffset);
										t.tmagy_f=ObjectPositionY(g.entityworkobjectoffset);
										t.tmagz_f=ObjectPositionZ(g.entityworkobjectoffset);
										t.ttobj=t.entityelement[t.tfindclosest].obj;
										for ( t.mag2id = 1 ; t.mag2id<=  6; t.mag2id++ )
										{
											t.tmag2x_f=t.entityelement[t.tfindclosest].x;
											t.tmag2y_f=t.entityelement[t.tfindclosest].y;
											t.tmag2z_f=t.entityelement[t.tfindclosest].z;
											PositionObject (  g.entityworkobjectoffset,t.tmag2x_f,t.tmag2y_f,t.tmag2z_f );
											RotateObject (  g.entityworkobjectoffset,ObjectAngleX(t.ttobj),ObjectAngleY(t.ttobj),ObjectAngleZ(t.ttobj) );
											if (  t.mag2id == 1  )  MoveObjectLeft (  g.entityworkobjectoffset,t.tmag2sizex_f );
											if (  t.mag2id == 2  )  MoveObjectRight (  g.entityworkobjectoffset,t.tmag2sizex_f );
											if (  t.mag2id == 3  )  MoveObjectUp (  g.entityworkobjectoffset,t.tmag2sizey_f );
											if (  t.mag2id == 4  )  MoveObjectDown (  g.entityworkobjectoffset,t.tmag2sizey_f );
											if (  t.mag2id == 5  )  MoveObject (  g.entityworkobjectoffset,t.tmag2sizez_f );
											if (  t.mag2id == 6  )  MoveObject (  g.entityworkobjectoffset,t.tmag2sizez_f*-1 );
											t.tmag2x_f=ObjectPositionX(g.entityworkobjectoffset);
											t.tmag2y_f=ObjectPositionY(g.entityworkobjectoffset);
											t.tmag2z_f=ObjectPositionZ(g.entityworkobjectoffset);
											//  are magnets close enough together to snap?
											t.tdiffx_f=t.tmag2x_f-t.tmagx_f;
											t.tdiffy_f=t.tmag2y_f-t.tmagy_f;
											t.tdiffz_f=t.tmag2z_f-t.tmagz_f;
											t.tdiff_f=Sqrt(abs(t.tdiffx_f*t.tdiffx_f)+abs(t.tdiffy_f*t.tdiffy_f)+abs(t.tdiffz_f*t.tdiffz_f));
											if (  t.tdiff_f<25.0 ) 
											{
												//  yes, maybe snap to this edge
												if (  t.tdiff_f<t.tbestmag_f ) 
												{
													t.tbestmag_f=t.tdiff_f;
													t.tbestmag2id=t.mag2id;
													t.tbestmag2x_f=t.tmag2x_f+(t.gridentityposx_f-t.tmagx_f);
													t.tbestmag2y_f=t.tmag2y_f+(t.gridentityposy_f-t.tmagy_f);
													t.tbestmag2z_f=t.tmag2z_f+(t.gridentityposz_f-t.tmagz_f);
												}
											}
										}
									}
									if (  t.tbestmag2id != -1 ) 
									{
										t.gridentityposx_f=t.tbestmag2x_f;
										t.gridentityposy_f=t.tbestmag2y_f;
										t.gridentityposz_f=t.tbestmag2z_f;
									}
								}
							}
						}
					}
				}
				if (  t.gridentitygridlock == 2 ) 
				{
					if ( t.entityprofile[t.gridentity].isebe != 0 )
					{
						// align EBE structure to match the 100x100 grid
						t.gridentityposx_f = 0+(int(t.gridentityposx_f/100)*100);
						t.gridentityposz_f = 0+(int(t.gridentityposz_f/100)*100);
					}
					else
					{
						// align a regular entity to respect its center
						t.gridentityposx_f = 50+(int(t.gridentityposx_f/100)*100);
						t.gridentityposz_f = 50+(int(t.gridentityposz_f/100)*100);
					}

					// 130517 - new EBE entity offset to align with 0,0,0 cornered entities from Aslum level and Store (Martin)
					if ( t.entityprofile[t.gridentity].isebe != 0 )
					{
						if ( g.gdefaultebegridoffsetx != 50 )
						{
							t.gridentityposx_f -= (g.gdefaultebegridoffsetx-50);
							t.gridentityposz_f -= (g.gdefaultebegridoffsetz-50);
						}
					}
				}
				if (  t.tforcedynamic == 1 ) 
				{
					t.gridentitystaticmode=0;
				}

				//  Find ground while placing entity on terrain
				bool bApplyEntityOffsets = false;
				if ( t.gridentitysurfacesnap == 1 )
				{
					// new system to locate grid ent pos at point where mouse touches terrain/entity surface
					t.gridentityposy_f = t.inputsys.localcurrentterrainheight_f;
					bApplyEntityOffsets = true;
				}
				else
				{
					if ( t.gridentityposoffground == 0 ) 
					{
						if (  t.terrain.TerrainID>0 ) 
						{
							t.gridentityposy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.gridentityposx_f,t.gridentityposz_f);
						}
						else
						{
							t.gridentityposy_f=1000.0;
						}
						bApplyEntityOffsets = true;
					}
				}
				if ( bApplyEntityOffsets == true )
				{
					//  for markers, apply Y offset
					if (  t.entityprofile[t.gridentity].ismarker != 0 ) 
					{
						t.gridentityposy_f=t.gridentityposy_f+t.entityprofile[t.gridentity].offy;
					}
					//  for entities with default height, apply Y offset
					if (  t.entityprofile[t.gridentity].defaultheight != 0 ) 
					{
						t.gridentityposy_f=t.gridentityposy_f+t.entityprofile[t.gridentity].defaultheight;
					}
				}

				//  move waypoint zone when move trigger entity
				if (  t.grideleprof.trigger.waypointzoneindex>0 ) 
				{
					if (  t.gridentity>0 ) 
					{
						waypoint_movetogrideleprof ( );
					}
					else
					{
						t.grideleprof.trigger.waypointzoneindex=0;
					}
				}

				//  control modification of entity element details
				if (  t.gridentitymodifyelement == 1 ) 
				{
					if ( t.gridedit.entityspraymode == 1 )
					{
						if ( t.gridedit.entitysprayrange>0 ) t.gridedit.entitysprayrange -= 50;
					}
					else
					{
						if ( t.grideleprof.light.range>50 ) t.grideleprof.light.range -= 50;
					}
					t.gridentitymodifyelement=0;
				}
				if (  t.gridentitymodifyelement == 2 ) 
				{
					if ( t.gridedit.entityspraymode == 1 )
					{
						if ( t.gridedit.entitysprayrange<1000 ) t.gridedit.entitysprayrange += 50;
					}
					else
					{
						if ( t.grideleprof.light.range<1000 ) t.grideleprof.light.range += 50;
					}
					t.gridentitymodifyelement=0;
				}

				//  extract entity (RMB) or place entity (LMB)
				if (  (t.widget.duplicatebuttonselected == 2 && t.gridentity == 0) || (t.inputsys.mclick == 1 && t.gridentity != 0) ) 
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
								t.gridentityposy_f=1000.0;
							}
						}

						//  Version Control - stop high resource use
						t.resourceused=2  ; version_resourcewarning ( );

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
								// store original ent ID pased down
								int iStoreGridEntity = t.gridentity;

								// add parent entity
								gridedit_addentitytomap ( );
								int iNewParentEntityIndex = t.e;
								t.entityelement[t.e].iHasParentIndex = t.gridentityhasparent;

								// add children for the parent
								int* piNewEntIndex = new int[g.entityrubberbandlist.size()];
								for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
								{
									int e = g.entityrubberbandlist[i].e;
									if ( e == 0 )
									{
										t.e = iNewParentEntityIndex;
									}
									else
									{
										// duplicate this child and add to map
										t.gridentity = t.entityelement[e].bankindex;
										gridedit_addentitytomap ( );

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
										PositionObject ( t.entityelement[t.e].obj, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z );
										RotateObject ( t.entityelement[t.e].obj, t.entityelement[t.e].rx, t.entityelement[t.e].ry, t.entityelement[t.e].rz );
									}
									piNewEntIndex[i] = t.e;
								}
								// and once all new entities created, link new parents to them
								for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
								{
									t.e = piNewEntIndex[i];
									int e = g.entityrubberbandlist[i].e;
									if ( e != 0 )
									{
										// find source parent of this child, if any
										if ( t.entityelement[e].iHasParentIndex > 0 )
										{
											if ( t.entityelement[e].iHasParentIndex == t.gridentityextractedindex )
											{
												// entity was child of parent entity extacted
												t.entityelement[t.e].iHasParentIndex = iNewParentEntityIndex;
											}
											else
											{
												// entity was child of another entity (a child in here)
												for ( int ii = 0; ii < (int)g.entityrubberbandlist.size(); ii++ )
												{
													int ee = g.entityrubberbandlist[ii].e;
													if ( ee > 0 )
													{
														if ( t.entityelement[e].iHasParentIndex == ee )
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
								SAFE_DELETE ( piNewEntIndex );

								// restore original ent ID
								t.gridentity = iStoreGridEntity;
								t.e = iNewParentEntityIndex;
							}
							else
							{
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
								t.gridentityscalex_f=ObjectScaleX(t.entityelement[t.tentitytoselect].obj);
								t.gridentityscaley_f=ObjectScaleY(t.entityelement[t.tentitytoselect].obj);
								t.gridentityscalez_f=ObjectScaleZ(t.entityelement[t.tentitytoselect].obj);
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
						SetCameraRange (  1,70000 );
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
						PositionMouse (  (t.tideframestartx+t.inputsys.xmouse)*-1,(t.tideframestarty+t.inputsys.ymouse)*-1 );
						editor_refreshcamerarange ( );
					}

					// trigger actual extraction on next cycle
					t.widget.duplicatebuttonselected = 2;
				}
			}

			//  delete selected entity via delete key
			if ( t.onetimeentitypickup == 0 ) 
			{
				if ( t.gridentity != 0 ) 
				{
					if ( t.inputsys.kscancode == 211 ) 
					{
						t.inputsys.mclickreleasestate=1;
						t.gridentitydelete=1;
						t.selstage=1;
					}
				}
				else
				{
					if ( t.inputsys.kscancode == 211 || (t.widget.deletebuttonselected == 1 && t.inputsys.mclick == 0) ) 
					{
						t.widget.deletebuttonselected=0;
						if ( t.widget.pickedEntityIndex > 0 )
						{
							// specifically avoid deleting child entities if highlighting a parent
							if ( g.entityrubberbandlist.size() > 0 )
							{
								// delete all entities in rubber band highlight list
								gridedit_deleteentityrubberbandfrommap ( );
								gridedit_clearentityrubberbandlist();
								t.widget.pickedEntityIndex = 0;
							}
						}
						if (  t.widget.pickedObject>0 ) 
						{
							// delete a single entity selected by widget
							if ( t.widget.pickedEntityIndex > 0 )
							{
								t.tentitytoselect=t.widget.pickedEntityIndex;
								gridedit_deleteentityfrommap ( );
							}
							t.widget.pickedObject=0;
							widget_updatewidgetobject ( );
						}
						t.tentitytoselect=0;
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

			//  Select widget controlled object
			if (  t.inputsys.mclick == 1 && t.gridentity == 0 ) 
			{
				if (  t.tentitytoselect>0 ) 
				{
					if (  t.widget.pickedObject == 0 ) 
					{
						t.widget.pickedEntityIndex=t.tentitytoselect;
						t.entityelement[t.widget.pickedEntityIndex].editorlock=0;
						t.widget.pickedObject=t.entityelement[t.tentitytoselect].obj;
						t.widget.offsetx=g.glastpickedx_f-ObjectPositionX(t.widget.pickedObject);
						t.widget.offsety=g.glastpickedy_f-ObjectPositionY(t.widget.pickedObject);
						t.widget.offsetz=g.glastpickedz_f-ObjectPositionZ(t.widget.pickedObject);

						// 271015 - this may not be required as it is duplicated later on..
						if ( g.entityrubberbandlist.size() > 0 )
							gridedit_moveentityrubberband();
						else
							entity_recordbuffer_move ( );
					}
				}
			}

			//  zoom into entity properties (or EBE EDIT)
			if (  t.widget.propertybuttonselected == 1 ) 
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
						t.gridentityscalex_f=ObjectScaleX(t.entityelement[t.e].obj);
						t.gridentityscaley_f=ObjectScaleY(t.entityelement[t.e].obj);
						t.gridentityscalez_f=ObjectScaleZ(t.entityelement[t.e].obj);
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
						HideObject (  t.editor.objectstartindex+5 );
						t.selstage=1;

						//  disable icons that interfere with zoom mode
						editor_disableforzoom ( );

						//  prepare entity property handler
						interface_openpropertywindow ( );

						//  End widget control of this object
						t.widget.pickedObject=0;
					}
				}
			}

			// update rubberband selection connected to primary cursor entity
			if ( t.gridentity > 0 && g.entityrubberbandlist.size() > 1 && t.fOldGridEntityX > -1.0f )
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
					for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
					{
						int e = g.entityrubberbandlist[i].e;
						int tobj = t.entityelement[e].obj;
						if ( tobj > 0 )
						{
							if ( ObjectExist(tobj) == 1 )
							{
								if ( tobj != t.tobj )
								{
									// do rotation if triggered
									if ( bDoRotateToo == true )
									{
										// a fix for entities that have been inverted
										if ( t.entityelement[e].rx==180 && t.entityelement[e].rz==180 )
										{
											t.entityelement[e].rx = 0;
											t.entityelement[e].ry = t.entityelement[e].ry;
											t.entityelement[e].rz = 0;
											RotateObject ( tobj, t.entityelement[e].rx, t.entityelement[e].ry, t.entityelement[e].rz );
										}

										// rotate the entity
										RotateObject ( tobj, ObjectAngleX(tobj)+fMovedActiveObjectRX, ObjectAngleY(tobj)+fMovedActiveObjectRY, ObjectAngleZ(tobj)+fMovedActiveObjectRZ );
										t.entityelement[e].rx = ObjectAngleX(tobj);
										t.entityelement[e].ry = ObjectAngleY(tobj);
										t.entityelement[e].rz = ObjectAngleZ(tobj);

										// move the entity around a pivot point
										GGVECTOR3 VecPos;
										VecPos.x = g.entityrubberbandlist[i].x;
										VecPos.y = g.entityrubberbandlist[i].y;
										VecPos.z = g.entityrubberbandlist[i].z;
										sObject* pObject = GetObjectData(tobj);
										GGVec3TransformCoord ( &VecPos, &VecPos, &pObject->position.matObjectNoTran );
										t.entityelement[e].x = t.gridentityposx_f + VecPos.x;
										t.entityelement[e].y = t.gridentityposy_f + VecPos.y;
										t.entityelement[e].z = t.gridentityposz_f + VecPos.z;
									}

									// update entity position to new offset position
									t.entityelement[e].x = t.entityelement[e].x + fMovedActiveObjectX;
									t.entityelement[e].y = t.entityelement[e].y + fMovedActiveObjectY;
									t.entityelement[e].z = t.entityelement[e].z + fMovedActiveObjectZ;

									// update latest entity position
									PositionObject ( tobj, t.entityelement[e].x, t.entityelement[e].y, t.entityelement[e].z );
									//PE: Update light data for spot.
									if (t.entityelement[e].eleprof.usespotlighting)
										lighting_refresh();

									// move zones and lights if in group
									//widget_movezonesandlights ( e );
								}
							}
						}
					}
				}
			}
			// record all current offsets from primary cursor entity and rubberband selection
			t.fOldGridEntityX = -1.0f;
			t.fOldGridEntityY = -1.0f;
			t.fOldGridEntityZ = -1.0f;
			t.fOldGridEntityRX = -1.0f;
			t.fOldGridEntityRY = -1.0f;
			t.fOldGridEntityRZ = -1.0f;
			if ( t.gridentity > 0 && t.gridentityobj > 0 && g.entityrubberbandlist.size() > 1 )
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

				// if rubberband selection, delete all in selection
				gridedit_deleteentityrubberbandfrommap();

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

}

return;

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
			}
		}
	}
return;

}

void gridedit_save_test_map ( void )
{
	//  Save map data locally only (not to FPM)
	timestampactivity(0,"SAVETESTMAP: Save map");
	mapfile_savemap ( );

	//  Settings specific to the player
	timestampactivity(0,"SAVETESTMAP: Save t.player config");
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
	t.tfileveg_s=g.mysystem.levelBankTestMap_s+"vegmask.dds"; //"levelbank\\testmap\\vegmask.dds";
	t.tfilewater_s=g.mysystem.levelBankTestMap_s+"watermask.dds"; //"levelbank\\testmap\\watermask.dds";
	terrain_savetextures ( );
	t.tfileveggrass_s=g.mysystem.levelBankTestMap_s+"vegmaskgrass.dat"; //"levelbank\\testmap\\vegmaskgrass.dat";
	timestampactivity(0,"SAVETESTMAP: Save terrain veg");
	terrain_fastveg_savegrass ( );
	timestampactivity(0,"SAVETESTMAP: Save terrain height data");
	t.tfile_s=g.mysystem.levelBankTestMap_s+"m.dat"; //"levelbank\\testmap\\m.dat";
	terrain_save ( );
	
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

void gridedit_new_map ( void )
{
	// ensure tab mode vars reset (no carry from previous session)
	g.tabmode = 0; //TABTAB mode
	g.tabmodeshowfps = 0; //F11 mode
	g.tabmodehidehuds = 0;
	g.mouseishidden = 0;
	t.terrain.terrainpaintermode = 1;

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
	if ( gbWelcomeSystemActive == false ) Sync();

	//  Reset visual settings for new map
	visuals_newlevel ( );

	//  Ensure default terrain and veg graphics
	terrain_changestyle ( );
	g.vegstyleindex=t.visuals.vegetationindex;
	terrain_changevegstyle ( );

	//  Load map data
	editor_hideall3d ( );
	t.statusbar_s=t.strarr_s[366] ; gridedit_updatestatusbar ( );

	//  Clear all settings
	timestampactivity(0,"NEWMAP: _gridedit_clear_settings");
	gridedit_clear_settings ( );

	//  Empty the lightmap folder
	timestampactivity(0,"NEWMAP: _lm_emptylightmapfolder");
	lm_emptylightmapfolder ( );

	// Empty EBEs from testmap folder
	cstr pStoreOld = GetDir(); SetDir ( g.mysystem.levelBankTestMap_s.Get() );
	mapfile_emptyebesfromtestmapfolder(false);
	SetDir ( pStoreOld.Get() );

	// Empty terraintexture files from testmap folder
	SetDir ( g.mysystem.levelBankTestMap_s.Get() );
	if ( FileExist ( "superpalette.ter" ) == 1 ) DeleteFile ( "superpalette.ter" );
	if ( FileExist ( "Texture_D.dds" ) == 1 ) DeleteFile ( "Texture_D.dds" );
	if ( FileExist ( "Texture_N.dds" ) == 1 ) DeleteFile ( "Texture_N.dds" );
	if ( FileExist ( "globalenvmap.dds" ) == 1 ) DeleteFile ( "globalenvmap.dds" );
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
	t.tfile_s=g.mysystem.levelBankTestMap_s+"m.dat"; //"levelbank\\testmap\\m.dat";
	terrain_save ( );
	timestampactivity(0,"NEWMAP: Save terrain data");
	t.tfileveg_s=g.mysystem.levelBankTestMap_s+"vegmask.dds"; //"levelbank\\testmap\\vegmask.dds";
	t.tfilewater_s=g.mysystem.levelBankTestMap_s+"watermask.dds"; //"levelbank\\testmap\\watermask.dds";
	t.tgeneratefreshwatermaskflag=1;
	terrain_generatevegandmaskfromterrain ( );
	timestampactivity(0,"NEWMAP: Save terrain mask data");
	t.tfileveggrass_s=g.mysystem.levelBankTestMap_s+"vegmaskgrass.dat"; //"levelbank\\testmap\\vegmaskgrass.dat";
	terrain_fastveg_buildblankgrass_fornew ( );
	timestampactivity(0,"NEWMAP: Finish t.terrain generation");

	//  Generate heightmap texture for cheap shadows (if required)
	//t.terrain.terraintriggercheapshadowrefresh=2;

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

void gridedit_updatestatusbar ( void )
{
	//  020315 - 012 - display in the status bar if multiplayer lobbies are currently available
	mp_checkIfLobbiesAvailable ( );
	if (  t.statusbar_s+t.steamStatusBar_s != t.laststatusbar_s.Get() ) 
	{
		OpenFileMap (  1,"FPSEXCHANGE" );
		t.strwork = ""; t.strwork = t.strwork + t.statusbar_s+t.steamStatusBar_s;
		SetFileMapString (  1, 4000, t.strwork.Get() );
		SetEventAndWait (  1 );
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
		OpenFileMap (  1,"FPSEXCHANGE" );
		SetFileMapDWORD (  1, 408, 0 );
		SetEventAndWait (  1 );
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
		timestampactivity(0,"Load t.player config");
		mapfile_loadplayerconfig ( );

		//  Load terrain
		popup_text_change(t.strarr_s[610].Get());
		terrain_createactualterrain ( );
		terrain_loaddata ( );

		//  Update remaining map data before editing
		gridedit_updatemapbeforeedit ( );

		//  Load editor configuration
		editor_loadcfg ( );

		//  Load segments/prefab/entities into window
		OpenFileMap (  1,"FPSEXCHANGE" );
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
	OpenFileMap (  1,"FPSEXCHANGE" );
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
}

void gridedit_import_ask ( void )
{
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	do
	{
		t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
	} while ( !(  t.inputsys.kscancode == 0 ) );

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
}

void gridedit_intercept_savefirst ( void )
{
	t.editorcanceltask=0;
	if (  g.projectmodified == 1 ) 
	{
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
		//CloseFileMap (  1 );
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
		OpenFileMap (  1,"FPSEXCHANGE" );
		//t.strwork = ""; t.strwork = t.strwork + g.rootdir_s+"mapbank\\";
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
		//CloseFileMap (  1 );
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
return;

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

	//  refresh 3d view so dialog Box-  not left black Box
	if ( gbWelcomeSystemActive == false )
	{
		for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }
	}

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
	OpenFileMap (  1,"FPSEXCHANGE" );
	//t.strwork = ""; t.strwork = t.strwork + g.rootdir_s+"mapbank\\";
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

	//  refresh 3d view so dialog Box (  (  not left black Box ) )
	for ( t.tsync = 1 ; t.tsync <=  5 ; t.tsync++ ) { Sync ( ); SleepNow ( 10 ); }

	if (  t.returnstring_s != "" ) 
	{
		if (  cstr(Lower(Right(t.returnstring_s.Get(),4))) != ".fpm"  )  t.returnstring_s = t.returnstring_s+".fpm";
		g.projectfilename_s=t.returnstring_s;
		gridedit_save_map ( );
	}

	// we can insert a 'dialog' after a saveas action to see if the user wants to make a Steam Review (After X time)
	//editor_showreviewrequest_check();
}

void gridedit_addentitytomap ( void )
{
	// mark as static if it was
	if ( t.gridentitystaticmode == 1 ) g.projectmodifiedstatic = 1;
	entity_addentitytomap ( );
	entity_recordbuffer_add ( );
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

	// mark as static if it was
	if ( t.entityelement[t.tentitytoselect].staticflag == 1 ) g.projectmodifiedstatic = 1;
	entity_recordbuffer_delete ( );
	entity_deleteentityfrommap ( );

	//  restore tentitytoselect in case switched it
	t.tentitytoselect=t.tstoretentitytoselect;
}

void gridedit_deleteentityrubberbandfrommap ( void )
{
	// will delete all entities in rubber band list, and preserve them into undo buffer
	g.entityrubberbandlistundo.clear();
	for ( int i = 0; i < (int)g.entityrubberbandlist.size(); i++ )
	{
		t.tentitytoselect = g.entityrubberbandlist[i].e;
		if ( t.tentitytoselect > 0 )
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
		entity_recordbuffer_move();
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
		if (  ObjectExist(t.sourceobj) == 1 ) 
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

			//  other entity attributes
			if (  t.entityprofile[t.entid].ismarker != 0 ) 
			{
				//  special setup for marker objects
				SetObjectTransparency ( t.obj, 2 );
				SetObjectCull ( t.obj, 1 );
			}
			else
			{
				if (  t.entityprofile[t.entid].cullmode != 0 ) 
				{
					//  cull mode OFF used for single sided polygon models (Reloaded)
					//  to help with palm tree leaves
					SetObjectCull (  t.obj,0 );
				}
				else
				{
					SetObjectCull (  t.obj,1 );
				}
				//  set transparency mode
				SetObjectTransparency (  t.obj,t.entityprofile[t.entid].transparency );

				// 051115 - only if not using limb visibility for hiding decal arrow
				if ( t.entityprofile[t.entid].addhandlelimb==0 )
				{
					//  set LOD attributes for entities
					entity_calculateentityLODdistances ( t.entid, t.obj, 0 );
				}
			}
			if (  GetNumberOfFrames(t.obj)>0 ) 
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
		t.tescale=t.entityprofile[t.gridentity].scale;
		if (  t.tescale>0  )  ScaleObject (  t.obj,t.tescale,t.tescale,t.tescale );
		SetObjectCollisionOff (  t.obj );
		if ( g.entityrubberbandlist.size() == 1 )
		{
			// only dehighlight if single extract, not if a rubber band / linked entities extraction
			editor_restoreobjhighlightifnotrubberbanded ( t.obj );
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
			ScaleObject (  t.obj,t.tfinalscalex_f,t.tfinalscaley_f,t.tfinalscalez_f );
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

	//  clear OBJ values in entityelements (as all objects are being removed)
	if (  g.entityelementlist>0 ) 
	{
		for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
		{
			t.obj=t.entityelement[t.e].obj;
			if (  t.obj>0 ) 
			{
				if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
			}
			//  clear any character creator objects associated with this entity
			t.ccobjToDelete=t.obj;
			characterkit_deleteEntity ( );

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
	UnDim (  t.entityshadervar );
	UnDim (  t.entitydebug_s );
	g.entityelementmax=100;
	Dim (  t.entityelement,g.entityelementmax  );
	Dim2(  t.entityshadervar,g.entityelementmax, g.globalselectedshadermax  );
	Dim (  t.entitydebug_s,g.entityelementmax  );
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

return;


//Function to help assign textures to plane objects (for editor cursors)


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

// 
//  PROPERTIES
// 

void interface_openpropertywindow ( void )
{
	//  Open proprty window
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
		// special VRQ2 mode also hides concepts of lives, health, blood, violence (substitute health for strength)
		bool bVRQ2ZeroViolenceMode = false;
		if ( g.gvrmode == 3 ) bVRQ2ZeroViolenceMode = true;

		//  Setup usage flags
		t.tsimplecharview=0;
		t.tflaglives=0 ; t.tflaglight=0 ; t.tflagobjective=0 ; t.tflagtdecal=0 ; t.tflagdecalparticle=0 ; t.tflagspawn=0 ; t.tflagifused=0;
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
					}
					if (  t.entityprofile[t.gridentity].ismarker == 4 ) { t.tflagtdecal = 1  ; t.tflagdecalparticle = 1; }
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
							if ( t.entityprofile[t.gridentity].markerindex == 3 ) tflagimage=1;
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
			setpropertystring2(t.group,t.grideleprof.name_s.Get(),t.strarr_s[478].Get(),"Choose a unique name for this character") ; ++t.controlindex;
			setpropertylist2(t.group,t.controlindex,t.grideleprof.aimain_s.Get(),"Behaviour","Select a behaviour for this character",11) ; ++t.controlindex;
			setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),"Voiceover","Select t.a WAV or OGG file this character will use during their behaviour","audiobank\\") ; ++t.controlindex;
			setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),"If Used","Sometimes used to specify the name of an entity to be activated") ; ++t.controlindex;
		}
		else
		{
			//  Name
			t.group=0 ; startgroup(t.strarr_s[412].Get()) ; t.controlindex=0;
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
			}

			// 101016 - Additional General Parameters
			if ( t.tflagchar == 0 && t.tflagvis == 1 ) 
			{
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocluder),"Occluder","Set to YES makes this entity an occluder",0) ; ++t.controlindex;
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isocludee),"Occludee","Set to YES makes this entity an occludee",0) ; ++t.controlindex;
				}
				
				// these will be back when EBE needs doors and windows
				//setpropertystring2(t.group,Str(t.grideleprof.parententityindex),"Parent Index","Selects another entity element to be a parent") ; ++t.controlindex;
				//setpropertystring2(t.group,Str(t.grideleprof.parentlimbindex),"Parent Limb","Specifies the limb index of the parent to connect with") ; ++t.controlindex;
			}

			// 281116 - added Specular Control per entity
			if ( t.tflagvis == 1 ) 
			{
				if ( t.tflagsimpler == 0 )
				{
					setpropertystring2(t.group,Str(t.grideleprof.specularperc),"Specular","Set specular percentage to modulate entity specular effect")  ; ++t.controlindex; 
				}
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
				++t.group ; startgroup(t.strarr_s[415].Get()) ; t.controlindex=0;
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
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.usespotlighting),"Spot Lighting","Set whether emits dynamic spot lighting",0) ; ++t.controlindex;
				}
			}

			//  Is Character
			if (  t.tflagchar == 1 ) 
			{
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
					setpropertystring2(t.group,Str(t.grideleprof.conerange),t.strarr_s[476].Get(),"The range within which the AI may see the player. Zero triggers the characters default range.") ; ++t.controlindex;
					setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[226].Get()) ; ++t.controlindex;
					if ( g.quickparentalcontrolmode != 2 )
					{
						setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isviolent),"Blood Effects","Sets whether blood and screams should be used",0) ; ++t.controlindex;
					}
					if ( t.tflagsimpler == 0 )
					{
						setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.colondeath),"End Collision","Set to NO switches off collision when die",0) ; ++t.controlindex;
					}
				}
				else
				{
					if (  t.tflagweap == 0 && t.tflagammo == 0 ) 
					{
						t.propfield[t.group]=t.controlindex;
						++t.group ; startgroup(t.strarr_s[435].Get()) ; t.controlindex=0;
						setpropertystring2(t.group,t.grideleprof.usekey_s.Get(),t.strarr_s[436].Get(),t.strarr_s[225].Get()) ; ++t.controlindex;
						if ( t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1 )
						{
							// only one level - no winzone chain option
						}
						else
						{
							setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[226].Get()) ; ++t.controlindex;
						}
					}
				}
			}
			if (  t.tflagifused == 1 ) 
			{
				if (  t.tflagusekey == 1 ) 
				{
					setpropertystring2(t.group,t.grideleprof.usekey_s.Get(),t.strarr_s[436].Get(),t.strarr_s[225].Get()) ; ++t.controlindex;
				}
				if ( t.tflagsimpler != 0 & t.entityprofile[t.gridentity].ismarker == 3 && t.entityprofile[t.gridentity].trigger.stylecolor == 1 )
				{
					// only one level - no winzone chain option
				}
				else
				{
					setpropertystring2(t.group,t.grideleprof.ifused_s.Get(),t.strarr_s[437].Get(),t.strarr_s[227].Get()) ; ++t.controlindex;
				}
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
				if (  t.tflaglives == 1 ) { setpropertystring2(t.group,Str(t.grideleprof.lives),t.strarr_s[452].Get(),t.strarr_s[242].Get())  ; ++t.controlindex; }
				if (  t.tflagvis == 1 || t.tflagstats == 1 ) 
				{
					if (  t.tflaglives == 1 ) 
					{
						setpropertystring2(t.group,Str(t.grideleprof.strength),t.strarr_s[453].Get(),t.strarr_s[243].Get()) ; ++t.controlindex;
					}
					else
					{
						if ( t.tflagnotionofhealth == 1 )
						{
							setpropertystring2(t.group,Str(t.grideleprof.strength),t.strarr_s[454].Get(),t.strarr_s[244].Get()) ; ++t.controlindex;
						}
					}
					if (  t.tflagplayersettings == 1 ) 
					{
						if ( g.quickparentalcontrolmode != 2 )
						{
							setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isviolent),"Blood Effects","Sets whether blood and screams should be used",0) ; ++t.controlindex;
						}
						if ( t.tflagnotionofhealth == 1 )
						{
							setpropertystring2(t.group,Str(t.playercontrol.regenrate),"Regeneration Rate","Sets the increase value at which the players health will restore")  ; ++t.controlindex;
							setpropertystring2(t.group,Str(t.playercontrol.regenspeed),"Regeneration Speed","Sets the speed in milliseconds at which the players health will regenerate") ; ++t.controlindex;
							setpropertystring2(t.group,Str(t.playercontrol.regendelay),"Regeneration Delay","Sets the delay in milliseconds after last damage hit before health starts regenerating") ; ++t.controlindex;
						}
					}
					setpropertystring2(t.group,Str(t.grideleprof.speed),t.strarr_s[455].Get(),t.strarr_s[245].Get()) ; ++t.controlindex;
					if (  t.playercontrol.thirdperson.enabled == 1 ) 
					{
						t.tanimspeed_f=t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.animspeed;
					}
					else
					{
						t.tanimspeed_f=t.grideleprof.animspeed;
					}
					setpropertystring2(t.group,Str(t.tanimspeed_f),t.strarr_s[477].Get(),"Sets the default speed of any animation associated with this entity"); ++t.controlindex;
				}
				if (  t.tflaghurtfall == 1 ) { setpropertystring2(t.group,Str(t.grideleprof.hurtfall),t.strarr_s[456].Get(),t.strarr_s[246].Get())  ; ++t.controlindex; }
				if (  t.tflagplayersettings == 1 ) 
				{
					setpropertystring2(t.group,Str(t.playercontrol.jumpmax_f),"Jump Speed","Sets the jump speed of the t.player which controls overall jump height") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.gravity_f),"Gravity","Sets the modified force percentage of the players own gravity") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.fallspeed_f),"Fall Speed","Sets the maximum speed percentage at which the t.player will fall") ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.playercontrol.climbangle_f),"Climb Angle","Sets the maximum angle permitted for the player to ascend a slope") ; ++t.controlindex;
					if (  t.playercontrol.thirdperson.enabled == 0 ) 
					{
						setpropertystring2(t.group,Str(t.playercontrol.wobblespeed_f),"Wobble Speed","Sets the rate of motion applied to the camera when moving") ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.playercontrol.wobbleheight_f*100),"Wobble Height","Sets the degree of motion applied to the camera when moving") ; ++t.controlindex;
						setpropertystring2(t.group,Str(t.playercontrol.footfallpace_f*100),"Footfall Pace","Sets the rate at which the footfall sound is played when moving") ; ++t.controlindex;
					}
					setpropertystring2(t.group,Str(t.playercontrol.accel_f*100),"Acceleration","Sets the acceleration curve used when t.moving from t.a stood position") ; ++t.controlindex;
				}
				if ( t.tflagmobile == 1 ) { setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.isimmobile),t.strarr_s[457].Get(),t.strarr_s[247].Get(),0); ++t.controlindex; }
				if ( t.tflagmobile == 1 ) 
				{ 
					if ( t.tflagsimpler == 0 )
					{
						setpropertystring2(t.group,Str(t.grideleprof.lodmodifier),"LOD Modifier","Modify when the LOD transition takes effect. The default value is 0, increase this to a percentage reduce the LOD effect.") ; ++t.controlindex; 
					}
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
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group,t.controlindex,Str(t.grideleprof.explodable),t.strarr_s[592].Get(),t.strarr_s[593].Get(),0) ; ++t.controlindex;
					setpropertystring2(t.group,Str(t.grideleprof.explodedamage),t.strarr_s[594].Get(),t.strarr_s[595].Get()) ; ++t.controlindex;
				}
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
				if ( t.tflagsimpler == 0 )
				{
					setpropertylist2(t.group, t.controlindex, Str(t.grideleprof.usespotlighting), "Spot Lighting", "Change dynamic light to spot lighting", 0); ++t.controlindex;
				}
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
					//startgroup(t.strarr_s[466].Get());
					startgroup("Media");
				}
				t.controlindex=0;
				if ( g.fpgcgenre == 1 ) 
				{
					if ( g.vrqcontrolmode != 0 )
					{
						if ( t.tflagsound == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[253].Get(),"audiobank\\")  ; ++t.controlindex; }
					}
					else
					{
						if ( t.tflagsound == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[467].Get(),t.strarr_s[253].Get(),"audiobank\\")  ; ++t.controlindex; }
					}
					if ( t.tflagsoundset == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[255].Get(),"audiobank\\voices\\")  ; ++t.controlindex; }
					if ( tflagtext == 1 ) { setpropertystring2(t.group,t.grideleprof.soundset_s.Get(),"Text to Appear","Enter text to appear in-game") ; ++t.controlindex; }
					if ( tflagimage == 1 ) { setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),"Image File","Select image to appear in-game","scriptbank\\images\\imagesinzone\\") ; ++t.controlindex; }
					if ( t.tflagnosecond == 0 ) 
					{
						if ( t.tflagsound == 1 || t.tflagsoundset == 1 )
						{ 
							setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),t.strarr_s[468].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset2_s.Get(),t.strarr_s[480].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset3_s.Get(),t.strarr_s[481].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
							setpropertyfile2(t.group,t.grideleprof.soundset4_s.Get(),t.strarr_s[482].Get(),t.strarr_s[254].Get(),"audiobank\\")  ; ++t.controlindex; 
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
				setpropertyfile2(t.group,t.grideleprof.soundset_s.Get(),t.strarr_s[469].Get(),t.strarr_s[599].Get(),"audiobank\\") ; ++t.controlindex;
				setpropertyfile2(t.group,t.grideleprof.soundset1_s.Get(),"Video Slot",t.strarr_s[601].Get(),"videobank\\") ; ++t.controlindex;
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
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
				if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.chopthis_s = t.chopthis_s+"scriptbank\\";
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
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.tokay = 0;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.tokay = 0;
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
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[478].Get()) ) == 0 )  t.grideleprof.name_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[479].Get()) ) == 0 )  t.grideleprof.name_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[414].Get()) ) == 0 )  t.gridentitystaticmode = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[561].Get()) ) == 0 )  t.grideleprof.aiinit_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[417].Get()) ) == 0 )  t.grideleprof.aimain_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , "behaviour" ) == 0 )  t.grideleprof.aimain_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[418].Get()) ) == 0 )  t.grideleprof.aidestroy_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[433].Get()) ) == 0 )  t.grideleprof.aishoot_s = t.tdataclipped_s;
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[434].Get()) ) == 0 )  t.grideleprof.coneangle = ValF(t.tdata_s.Get());
			if (  strcmp( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[476].Get()) ) == 0 )  t.grideleprof.conerange = ValF(t.tdata_s.Get());
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
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Footfall Pace")  ) == 0 ) t.playercontrol.footfallpace_f = ValF(t.tdata_s.Get())/100.0;
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
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[464].Get()) ) == 0 )  t.grideleprof.basedecal_s = t.tdataclipped_s;

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
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[480].Get()) ) == 0 )  t.grideleprof.soundset2_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[481].Get()) ) == 0 )  t.grideleprof.soundset3_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[482].Get()) ) == 0 )  t.grideleprof.soundset4_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[598].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[600].Get()) ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , "voiceover"  ) == 0 ) t.grideleprof.soundset1_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[462].Get()) ) == 0 )  t.grideleprof.light.range = ValF(t.tdata_s.Get());
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Text to Appear") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Image File") ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower(t.strarr_s[469].Get()) ) == 0 )  t.grideleprof.soundset_s = t.tdataclipped_s;
			if (  strcmp ( Lower(t.tfield_s.Get()) , Lower("Video Slot") ) == 0 )  t.grideleprof.soundset1_s = t.tdataclipped_s;

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
	if (  t.editorinterfaceactive>0 ) 
	{
		//  Close dialog
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 978, 2 );
		SetFileMapDWORD (  1, 462, 0 );
		SetEventAndWait (  1 );
		t.editorinterfaceactive=0;
	}
}

void interface_handlepropertywindow ( void )
{

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

	//CloseFileMap (  2 );

}

return;

}

void interface_live_updates ( void )
{

	//  constantly open access to properties values
	//  so can represent the values prior to using APPLY CHANGES
	if (  Timer()>t.lastliveupdatestimer ) 
	{
		t.lastliveupdatestimer=Timer()+200;
		OpenFileMap (  2, "FPSENTITY" );
		SetEventAndWait (  2 );
		t.iGroup=t.livegroupforthirdperson;
		t.iControl=1 ; t.tfield_s=getpropertyfield(t.iGroup,t.iControl) ; t.tdata_s=getpropertydata(t.iGroup,t.iControl);
		if (  cstr(Lower(t.tfield_s.Get())) == Lower("Camera Distance")  )  t.playercontrol.thirdperson.livecameradistance = ValF(t.tdata_s.Get());
		t.iControl=2 ; t.tfield_s=getpropertyfield(t.iGroup,t.iControl) ; t.tdata_s=getpropertydata(t.iGroup,t.iControl);
		if (  cstr(Lower(t.tfield_s.Get())) == Lower("Camera X Offset")  )  t.playercontrol.thirdperson.livecamerashoulder = ValF(t.tdata_s.Get());
		t.iControl=3 ; t.tfield_s=getpropertyfield(t.iGroup,t.iControl) ; t.tdata_s=getpropertydata(t.iGroup,t.iControl);
		if (  cstr(Lower(t.tfield_s.Get())) == Lower("Camera Y Offset")  )  t.playercontrol.thirdperson.livecameraheight = ValF(t.tdata_s.Get());
		t.iControl=4 ; t.tfield_s=getpropertyfield(t.iGroup,t.iControl) ; t.tdata_s=getpropertydata(t.iGroup,t.iControl);
		if (  cstr(Lower(t.tfield_s.Get())) == Lower("Camera Focus")  )  t.playercontrol.thirdperson.livecamerafocus = ValF(t.tdata_s.Get());
		//CloseFileMap (  2 );
	}

return;

// 
//  Interface Properties Functions
// 

}

void startgroup ( char* s_s )
{
	if (  cstr(s_s) == ""  )  s_s = "";
	SetFileMapDWORD (  3,g.g_filemapoffset,2  ); g.g_filemapoffset += 4;
	SetFileMapDWORD (  3,g.g_filemapoffset,Len(s_s)  ); g.g_filemapoffset += 4;
	SetFileMapString (  3,g.g_filemapoffset,s_s  ); g.g_filemapoffset += ((Len(s_s)+3)/4 )*4;
//endfunction

}

void endgroup ( void )
{
	SetFileMapDWORD (  3,g.g_filemapoffset,0  ); g.g_filemapoffset += 4;
//endfunction

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
//endfunction

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
//endfunction

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

		//  close conmunication with editor
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1,974,2 );
		//CloseFileMap (  1 );

		//  Before we 'BIN OUT', signal IDE that we wish to return to the IDE editor state
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetFileMapDWORD (  1, 970, 1 );
		SetEventAndWait (  1 );
		//CloseFileMap (  1 );

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
		//CloseFileMap (  1 );
		if (  tokay == 1 ) 
		{
			//  no references to 3D objects
			recoverdonotuseany3dreferences=1;
			//  save as now
			gridedit_saveas_map ( );
		}

		//  call a new map editor
		OpenFileMap (  2, "FPSEXCHANGE" );
		SetFileMapString (  2, 1000, "Guru-MapEditor.exe" );
		SetFileMapString (  2, 1256, "-r" );
		SetFileMapDWORD (  2, 994, 0 );
		SetFileMapDWORD (  2, 924, 1 );
		SetEventAndWait (  2 );
		//CloseFileMap (  2 );

		//  Terminate fragmented EXE
		common_justbeforeend();
		ExitProcess ( 0 );

	}
	}

//endfunction

}

int get_cursor_scale_for_obj ( int tObj )
{
	t.tSizeX_f = ObjectSizeX(tObj,1);
	t.tSizeZ_f = ObjectSizeZ(tObj,1);
	t.tscale_f= Sqrt(t.tSizeX_f*t.tSizeX_f + t.tSizeZ_f*t.tSizeZ_f)*3.0;

	return t.tscale_f;
}

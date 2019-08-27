//----------------------------------------------------
//--- GAMEGURU - M-MapFile
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Globals
std::vector<cstr> g_sDefaultAssetFiles;

// 
//  MAP FILE FORMAT
// 

void mapfile_saveproject_fpm ( void )
{
	LPSTR pOldDir = GetDir();

	//  use default or special worklevel stored
	if (  t.goverridefpmdestination_s != "" ) 
	{
		t.ttempprojfilename_s=t.goverridefpmdestination_s;
	}
	else
	{
		t.ttempprojfilename_s=g.projectfilename_s;
	}

	//75593 : Saving FPM g.level file: D:\github\GameGuruRepo\GameGuru\Files\map bank\my-test-map.fpm S:0MB   V: (579,0) 
	if (g.editorsavebak == 1) 
	{
		//PE: Make a backup before overwriting a fpm level.
		char backupname[1024];
		strcpy(backupname, t.ttempprojfilename_s.Get());
		backupname[strlen(backupname) - 1] = 'k';
		backupname[strlen(backupname) - 2] = 'a';
		backupname[strlen(backupname) - 3] = 'b';
		CopyAFile(t.ttempprojfilename_s.Get(), backupname);
	}

	//  log prompts
	timestampactivity(0, cstr(cstr("Saving FPM level file: ")+t.ttempprojfilename_s).Get() );

	//  Switch visuals to gamevisuals as this is what we want to save
	t.editorvisuals=t.visuals ; t.visuals=t.gamevisuals  ; visuals_save ( );

	//  Copy visuals.ini into levelfile folder
	t.tincludevisualsfile=0;
	if (  FileExist( cstr(g.fpscrootdir_s+"\\visuals.ini").Get() ) == 1 ) 
	{
		t.tvisfile_s=g.mysystem.levelBankTestMap_s+"visuals.ini"; //"levelbank\\testmap\\visuals.ini";
		if (  FileExist(t.tvisfile_s.Get()) == 1  )  DeleteAFile (  t.tvisfile_s.Get() );
		CopyAFile (  cstr(g.fpscrootdir_s+"\\visuals.ini").Get(),t.tvisfile_s.Get() );
		t.tincludevisualsfile=1;
	}

	//  And switch back for benefit to editor visuals
	t.visuals=t.editorvisuals; // messes up when click test game again, old: gosub _visuals_save

	//  Delete any old file
	if (  FileExist(t.ttempprojfilename_s.Get()) == 1  )  DeleteAFile (  t.ttempprojfilename_s.Get() );

	//  Copy CFG to testgame area for saving with other files
	t.tttfile_s="cfg.cfg";
	cstr cfgfile_s = g.mysystem.editorsGridedit_s + t.tttfile_s;
	if (  FileExist( cfgfile_s.Get() ) == 1 ) 
	{
		cstr cfginlevelbank_s = g.mysystem.levelBankTestMap_s+t.tttfile_s;
		if ( FileExist( cfginlevelbank_s.Get() ) == 1 ) DeleteAFile ( cfginlevelbank_s.Get() );
		CopyAFile ( cfgfile_s.Get(), cfginlevelbank_s.Get() );
	}

	//  Create a FPM (zipfile)
	CreateFileBlock (  1, t.ttempprojfilename_s.Get() );
	SetFileBlockKey (  1, "mypassword" );
	SetDir ( g.mysystem.levelBankTestMap_s.Get() ); // "levelbank\\testmap\\" );
	AddFileToBlock (  1, "header.dat" );
	AddFileToBlock (  1, "playerconfig.dat" );
	AddFileToBlock (  1, "cfg.cfg" );
	//  entity and waypoint files
	AddFileToBlock (  1, "map.ele" );
	AddFileToBlock (  1, "map.ent" );
	AddFileToBlock (  1, "map.way" );
	//  darkai obstacle data (container zero)
	AddFileToBlock (  1, "map.obs" );
	//  terrain files
	AddFileToBlock (  1, "m.dat" );
	AddFileToBlock (  1, "vegmask.dds" ); //PE: this failed with a runtime error 500 , the file size was 0 ?
	AddFileToBlock (  1, "vegmaskgrass.dat" );
	if ( FileExist ( "superpalette.ter" ) == 1 ) 
		AddFileToBlock ( 1, "superpalette.ter" );

	// add custom content
	cstr sStoreProjAsItGetsChanged = g.projectfilename_s;
	ScanLevelForCustomContent ( t.ttempprojfilename_s.Get() );
	g.projectfilename_s = sStoreProjAsItGetsChanged;

	// putting back optional custom terrain texture
	if ( FileExist ( "Texture_D.dds" ) == 1 ) 
		AddFileToBlock ( 1, "Texture_D.dds" );
	if ( FileExist ( "Texture_D.jpg" ) == 1 ) 
		AddFileToBlock ( 1, "Texture_D.jpg" );

	// VRQUEST Needs 'light' FPMs for transfer
	#ifdef VRQUEST
	// Don't include large files until find a nice to way reduce them considerably (or find a faster way to transfer multiplayer FPM)
	#else
	AddFileToBlock (  1, "watermask.dds" );
	if ( FileExist ( "globalenvmap.dds" ) == 1 ) 
		AddFileToBlock ( 1, "globalenvmap.dds" );
	if ( FileExist ( "Texture_N.dds" ) == 1 ) 
		AddFileToBlock ( 1, "Texture_N.dds" );
	if ( FileExist ( "Texture_N.jpg" ) == 1 ) 
		AddFileToBlock ( 1, "Texture_N.jpg" );
	#endif

	//  lightmap files
	if (  PathExist("lightmaps") == 1 ) 
	{
		AddFileToBlock (  1, "lightmaps\\objectlist.dat" );
		AddFileToBlock (  1, "lightmaps\\objectnummax.dat" );
		t.tnummaxfile_s=t.lightmapper.lmpath_s+"objectnummax.dat";
		if (  FileExist(t.tnummaxfile_s.Get()) == 1 ) 
		{
			OpenToRead (  1,t.tnummaxfile_s.Get() );
			t.temaxinfolder = ReadLong ( 1 );
			CloseFile (  1 );
		}
		else
		{
			t.temaxinfolder=4999;
		}
		for ( t.e = 0 ; t.e<=  (t.temaxinfolder*2)+100; t.e++ )
		{
			t.tname_s=t.lightmapper.lmpath_s+Str(t.e)+".dds";
			if (  FileExist(t.tname_s.Get()) == 1 ) 
			{
				AddFileToBlock (  1, cstr(cstr("lightmaps\\")+Str(t.e)+".dds").Get() );
			}
		}
		t.tfurthestobjnumber=g.lightmappedobjectoffset;
		SetDir (  "lightmaps" );
		ChecklistForFiles (  );
		for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
		{
			t.tfile_s=ChecklistString(t.c);
			if (  t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				if (  cstr(Lower(Right(t.tfile_s.Get(),4))) == ".dbo" ) 
				{
					t.tfile_s=Right(t.tfile_s.Get(),Len(t.tfile_s.Get())-Len("object"));
					t.tfile_s=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
					t.tfilevalue = ValF(t.tfile_s.Get()) ; if (  t.tfilevalue>t.tfurthestobjnumber  )  t.tfurthestobjnumber = t.tfilevalue;
				}
			}
		}
		SetDir (  ".." );
		for ( t.tobj = g.lightmappedobjectoffset; t.tobj <= t.tfurthestobjnumber; t.tobj++ )
		{
			t.tname_s = ""; t.tname_s = t.tname_s + "lightmaps\\object"+Str(t.tobj)+".dbo";
			if (  FileExist(t.tname_s.Get()) == 1  )  AddFileToBlock (  1, cstr(cstr("lightmaps\\object")+Str(t.tobj)+".dbo").Get() );
		}
	}
	//  visual settings
	if (  t.tincludevisualsfile == 1  )  AddFileToBlock (  1, "visuals.ini" );
	//  conkit data
	if (  FileExist("conkit.dat")  )  AddFileToBlock (  1,"conkit.dat" );
	//  ebe files
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		t.tfile_s = ChecklistString(t.c);
		if ( t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			cstr strEnt = cstr(Lower(Right(t.tfile_s.Get(),4)));
			if ( strcmp ( strEnt.Get(), ".ebe" ) == NULL )
			{
				AddFileToBlock ( 1, t.tfile_s.Get() );
				cstr tNameOnly = Left(t.tfile_s.Get(),strlen(t.tfile_s.Get())-4);
				cstr tThisFile = tNameOnly + cstr(".fpe");
				if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				tThisFile = tNameOnly + cstr(".dbo");
				if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				tThisFile = tNameOnly + cstr(".bmp");
				if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				tThisFile = tNameOnly + cstr("_D.dds");
				if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				#ifdef VRQUEST
				 // Don't include large files until find a nice to way reduce them considerably (or find a faster way to transfer multiplayer FPM)
				#else
				 tThisFile = tNameOnly + cstr("_N.dds");
				 if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				 tThisFile = tNameOnly + cstr("_S.dds");
				 if ( FileExist(tThisFile.Get()) ) AddFileToBlock ( 1, tThisFile.Get() );
				#endif
			}
			strEnt = cstr(Lower(Right(t.tfile_s.Get(),6)));
			if ( strcmp ( strEnt.Get(), "_d.dds" ) == NULL )
			{
				AddFileToBlock ( 1, t.tfile_s.Get() );
			}
			#ifdef VRQUEST
			 // Don't include large files until find a nice to way reduce them considerably (or find a faster way to transfer multiplayer FPM)
			#else
			 strEnt = cstr(Lower(Right(t.tfile_s.Get(),6)));
			 if ( strcmp ( strEnt.Get(), "_n.dds" ) == NULL || strcmp ( strEnt.Get(), "_s.dds" ) == NULL )
			 {
				AddFileToBlock ( 1, t.tfile_s.Get() );
			 }
			#endif
		}
	}

	//SetDir (  "..\\.." );
	SetDir ( pOldDir );
	SaveFileBlock ( 1 );

	//  does crazy cool stuff
	t.tsteamsavefilename_s = t.ttempprojfilename_s;
	mp_save_workshop_files_needed ( );

	//  log prompts
	timestampactivity(0,"Saving FPM level file complete");
}

void mapfile_emptyebesfromtestmapfolder ( bool bIgnoreValidTextureFiles )
{
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		t.tfile_s = ChecklistString(t.c);
		if ( t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			// only if not a CUSTOM content piece
			if ( strnicmp ( t.tfile_s.Get(), "CUSTOM_", 7 ) != NULL )
			{
				cstr strEnt = cstr(Lower(Right(t.tfile_s.Get(),4)));
				if ( stricmp ( strEnt.Get(), ".ebe" ) == NULL || stricmp ( strEnt.Get(), ".fpe" ) == NULL )
				{
					DeleteAFile ( t.tfile_s.Get() );
					cstr tNameOnly = Left(t.tfile_s.Get(),strlen(t.tfile_s.Get())-4);
					cstr tThisFile = tNameOnly + cstr(".fpe");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
					tThisFile = tNameOnly + cstr(".dbo");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
					tThisFile = tNameOnly + cstr(".bmp");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
					tThisFile = tNameOnly + cstr("_D.dds");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
					tThisFile = tNameOnly + cstr("_N.dds");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
					tThisFile = tNameOnly + cstr("_S.dds");
					if ( FileExist(tThisFile.Get())==1 ) DeleteAFile ( tThisFile.Get() );
				}
				strEnt = cstr(Lower(Right(t.tfile_s.Get(),6)));
				if ( bIgnoreValidTextureFiles == false )
				{
					if ( strcmp ( strEnt.Get(), "_d.dds" ) == NULL || strcmp ( strEnt.Get(), "_n.dds" ) == NULL || strcmp ( strEnt.Get(), "_s.dds" ) == NULL )
					{
						if ( stricmp ( t.tfile_s.Get(), "Texture_D.dds" ) != NULL && stricmp ( t.tfile_s.Get(), "Texture_N.dds" ) != NULL 
						&&   stricmp ( t.tfile_s.Get(), "Texture_D.jpg" ) != NULL && stricmp ( t.tfile_s.Get(), "Texture_N.jpg" ) != NULL )
						{
							DeleteAFile ( t.tfile_s.Get() );
						}
					}
				}
			}
		}
	}
}

void mapfile_loadproject_fpm ( void )
{
	//  Ensure FPM exists
	t.trerfeshvisualsassets=0;
	timestampactivity(0, cstr(cstr("_mapfile_loadproject_fpm: ")+g.projectfilename_s+" "+GetDir()).Get() );
	if ( FileExist(g.projectfilename_s.Get()) == 1 ) 
	{
		//  Empty the lightmap folder
		timestampactivity(0,"LOADMAP: _lm_emptylightmapfolder");
		lm_emptylightmapfolder ( );

		//  Store and switch folders
		t.tdirst_s=GetDir() ; SetDir ( g.mysystem.levelBankTestMap_s.Get() ); // "levelbank\\testmap\\" );

		//  Delete key testmap file (if any)
		if (  FileExist("header.dat") == 1  )  DeleteAFile (  "header.dat" );
		if (  FileExist("playerconfig.dat") == 1  )  DeleteAFile (  "playerconfig.dat" );
		if (  FileExist("watermask.dds") == 1  )  DeleteAFile (  "watermask.dds" );
		if (  FileExist("visuals.ini") == 1  )  DeleteAFile (  "visuals.ini" );
		if (  FileExist("conkit.dat") == 1  )  DeleteAFile (  "conkit.dat" );
		if (  FileExist("map.obs") == 1  )  DeleteAFile (  "map.obs" );

		//  Delete terrain texture files (if any)
		if ( FileExist("Texture_D.dds") == 1 ) DeleteAFile ( "Texture_D.dds" );
		if ( FileExist("Texture_D.jpg") == 1 ) DeleteAFile ( "Texture_D.jpg" );
		if ( FileExist("Texture_N.dds") == 1 ) DeleteAFile ( "Texture_N.dds" );
		if ( FileExist("Texture_N.jpg") == 1 ) DeleteAFile ( "Texture_N.jpg" );

		//  Delete env map for PBR (if any)
		if ( FileExist("globalenvmap.dds") == 1 ) DeleteAFile ( "globalenvmap.dds" );

		// empty any ebe files
		mapfile_emptyebesfromtestmapfolder(false);

		//  Restore folder to Files (for extraction)
		SetDir ( t.tdirst_s.Get() );

		//  Read FPM into testmap area
		timestampactivity(0,"LOADMAP: read FPM block");
		OpenFileBlock (  g.projectfilename_s.Get(),1,"mypassword" );
		PerformCheckListForFileBlockData (  1 );
		//t.tpath_s=t.rootpath_s+"levelbank\\testmap\\";
		t.tpath_s=g.mysystem.levelBankTestMap_s.Get(); //"levelbank\\testmap\\";
		for ( t.i = 1 ; t.i<=  ChecklistQuantity( ); t.i++ )
		{
			ExtractFileFromBlock (  1, ChecklistString( t.i ), t.tpath_s.Get() );
		}
		CloseFileBlock (  1 );

		//  If file still not present, extraction failed
		//SetDir (  cstr(t.tdirst_s+"\\levelbank\\testmap\\").Get() );
		SetDir ( g.mysystem.levelBankTestMapAbs_s.Get() );
		if (  FileExist("header.dat") == 0 ) 
		{
			//  inform user the FPM could not be loaded (corrupt file)
			t.tloadsuccessfully=0;
		}

		//  If file still not present, extraction failed
		//SetDir (  cstr(t.tdirst_s+"\\levelbank\\testmap\\").Get() );
		SetDir ( g.mysystem.levelBankTestMapAbs_s.Get() );		
		if ( g.memskipwatermask == 0  && FileExist("watermask.dds") == 0 )
		{
			//  Only Reloaded Formats have this texture file, so fail load if not there (Classic FPM)
			t.tloadsuccessfully=2;
		}

		//  load in visuals from loaded file
		timestampactivity(0,"LOADMAP: load in visuals");
		if (  FileExist("visuals.ini") == 1 ) 
		{
			t.tstorefpscrootdir_s=g.fpscrootdir_s;
			g.fpscrootdir_s="" ; visuals_load ( );
			g.fpscrootdir_s=t.tstorefpscrootdir_s;
			t.trerfeshvisualsassets=1;

			//  Ensure visuals settings are copied to gamevisuals (the true destination)
			t.gamevisuals=t.visuals;
			t.editorvisuals=t.visuals;

			//  And ensure editor visuals mimic required settings from loaded data
			visuals_editordefaults ( );
			t.visuals=t.editorvisuals;
			t.visuals.skyindex=t.gamevisuals.skyindex;
			t.visuals.sky_s=t.gamevisuals.sky_s;
			t.visuals.terrainindex=t.gamevisuals.terrainindex;
			t.visuals.terrain_s=t.gamevisuals.terrain_s;
			t.visuals.vegetationindex=t.gamevisuals.vegetationindex;
			t.visuals.vegetation_s=t.gamevisuals.vegetation_s;

			//  Re-acquire indices now the lists have changed
			//  takes visuals.sky$ visuals.terrain$ visuals.vegetation$
			visuals_updateskyterrainvegindex ( );
			t.gamevisuals.skyindex=t.visuals.skyindex;
			t.gamevisuals.terrainindex=t.visuals.terrainindex;
			t.gamevisuals.vegetationindex=t.visuals.vegetationindex;

		}

		//  if MAP OBS exists, we are not generating OBS data this time
		t.aisystem.generateobs=1;
		t.tobsfile_s="map.obs";
		if (  FileExist(t.tobsfile_s.Get()) == 1 ) 
		{
			t.aisystem.generateobs=0;
		}

		//  if CFG file present, copy to editor folder for later use (stores FPG for us)
		if (  t.game.runasmultiplayer == 0 ) 
		{
			//  single player/editor only - not needed when loading multiplayer map
			if (  t.tloadsuccessfully == 1 ) 
			{
				t.tttfile_s="cfg.cfg";
				if (  FileExist(t.tttfile_s.Get()) == 1 ) 
				{
					//if ( FileExist( cstr(t.tdirst_s+"\\editors\\gridedit\\"+t.tttfile_s).Get() ) == 1  )  DeleteAFile (  cstr(t.tdirst_s+"\\editors\\gridedit\\"+t.tttfile_s).Get() );
					//CopyAFile ( t.tttfile_s.Get() ,cstr(t.tdirst_s+"\\editors\\gridedit\\"+t.tttfile_s).Get() );
					cstr cfgfile_s = g.mysystem.editorsGrideditAbs_s + t.tttfile_s;
					if ( FileExist( cfgfile_s.Get() ) == 1  )  DeleteAFile ( cfgfile_s.Get() );
					CopyAFile ( t.tttfile_s.Get(), cfgfile_s.Get() );
				}
			}
		}

		//  Retore and switch folders
		SetDir (  t.tdirst_s.Get() );

		//  if visuals file present, apply it
		timestampactivity(0,"LOADMAP: apply visuals");
		if (  t.trerfeshvisualsassets == 1 ) 
		{
			//  if loading from game (level load), ensure it's the game visuals we use
			if (  t.game.gameisexe == 1 || t.game.runasmultiplayer == 1  )  t.visuals = t.gamevisuals;
			//  and refresh assets based on restore
			t.visuals.refreshshaders=1;
			t.visuals.refreshskysettings=1;
			t.visuals.refreshterraintexture=1;
			t.visuals.refreshvegtexture=1;
			visuals_loop ( );
			visuals_shaderlevels_update ( );
		}
	}
	else
	{
		t.tloadsuccessfully=0;
	}
}

void mapfile_newmap ( void )
{
	//  Defaults
	t.layermax=20 ; t.maxx=500 ; t.maxy=500;
	t.olaylistmax=100;
}

void mapfile_loadmap ( void )
{
	// Load header data (need main mapdata for visdata)
	t.filename_s=t.levelmapptah_s+"header.dat";
	if (  FileExist(t.filename_s.Get()) == 1 ) 
	{
		// Header - version number
		OpenToRead (  1,t.filename_s.Get() );
		t.versionmajor = ReadLong ( 1 );
		t.versionminor = ReadLong ( 1 );
		CloseFile (  1 );
	}

	// 080917 - if old header, delete map.obj as it contains corrupt waypoint data
	if ( t.versionmajor < 1 )
	{
		LPSTR pOldDir = GetDir();
		//LPSTR pObstacleWaypointData = "levelbank\\testmap\\map.obs";
		//if ( FileExist ( pObstacleWaypointData ) == 1 ) DeleteFile ( pObstacleWaypointData );
		cstr obstacleWaypointData_s = g.mysystem.levelBankTestMap_s+"map.obs";
		if ( FileExist ( obstacleWaypointData_s.Get() ) == 1 ) DeleteFile ( obstacleWaypointData_s.Get() );
	}
}

void mapfile_savemap ( void )
{
	// Store old folder
	t.old_s=GetDir();

	// Enter folder
	SetDir ( g.mysystem.levelBankTestMap_s.Get() ); //"levelbank\\testmap\\" );

	// Clear old files out (TEMP)
	if (  FileExist("header.dat") == 1  )  DeleteAFile (  "header.dat" );

	// Create header file
	OpenToWrite (  1,"header.dat" );

	// Version 0.0 = Reloaded
	// Version 1.0 = GameGuru DX11 (new obstacle data save fixes)
	t.versionmajor = 1; WriteLong ( 1, t.versionmajor );
	t.versionminor = 0; WriteLong ( 1, t.versionminor );

	// end of header
	CloseFile (  1 );

	// Restore
	SetDir (  t.old_s.Get() );
}

void mapfile_loadplayerconfig ( void )
{
	//  Load player settings
	t.filename_s=t.levelmapptah_s+"playerconfig.dat";
	if (  FileExist(t.filename_s.Get()) == 1 ) 
	{
		//  Reloaded Header
		OpenToRead (  1,t.filename_s.Get() );

		//  verion header
		t.tmp_s = ReadString ( 1 );
		t.tversion = ReadLong ( 1 );

		//  player settings
		if (  t.tversion >= 201006 ) 
		{
			t.a_f = ReadFloat ( 1 ); t.playercontrol.jumpmax_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.gravity_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.fallspeed_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.climbangle_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.footfallpace_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.wobblespeed_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.wobbleheight_f=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.accel_f=t.a_f;
		}

		//  extra player settings
		if (  t.tversion >= 20100651 ) 
		{
			t.a_f = ReadFloat ( 1 ); t.playercontrol.regenrate=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.regenspeed=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.regendelay=t.a_f;
		}

		//  extra player settings for third person (V1.01.002)
		if (  t.tversion >= 20100652 ) 
		{
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.enabled=t.a;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.startmarkere=t.a;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.charactere=t.a;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.thirdperson.cameradistance=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.thirdperson.cameraheight=t.a_f;
			t.a_f = ReadFloat ( 1 ); t.playercontrol.thirdperson.cameraspeed=t.a_f;
		}
		if (  t.tversion >= 20100653 ) 
		{
			t.a_f = ReadFloat ( 1 ); t.playercontrol.thirdperson.camerafocus=t.a_f;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.cameralocked=t.a;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.camerashoulder=t.a;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.camerafollow=t.a;
			t.a = ReadLong ( 1 ); t.playercontrol.thirdperson.camerareticle=t.a;
		}

		CloseFile (  1 );
	}
}

void mapfile_saveplayerconfig ( void )
{
	//  Store old folder
	t.old_s=GetDir();

	//  Enter folder
	SetDir ( g.mysystem.levelBankTestMap_s.Get() ); // "levelbank\\testmap\\" );

	//  Version for player config has minor value (between betas)
	t.gtweakversion=20100653;

	//  WriteLong (  out )
	if (  FileExist("playerconfig.dat") == 1  )  DeleteAFile (  "playerconfig.dat" );
	OpenToWrite (  1,"playerconfig.dat" );

		//  verion header
		WriteString (  1,"version" );
		WriteLong (  1,t.gtweakversion );

		//  player settings
		WriteFloat (  1,t.playercontrol.jumpmax_f );
		WriteFloat (  1,t.playercontrol.gravity_f );
		WriteFloat (  1,t.playercontrol.fallspeed_f );
		WriteFloat (  1,t.playercontrol.climbangle_f );
		WriteFloat (  1,t.playercontrol.footfallpace_f );
		WriteFloat (  1,t.playercontrol.wobblespeed_f );
		WriteFloat (  1,t.playercontrol.wobbleheight_f );
		WriteFloat (  1,t.playercontrol.accel_f );

		//  extra settings from V1.0065 (20100651)
		WriteFloat (  1,t.playercontrol.regenrate );
		WriteFloat (  1,t.playercontrol.regenspeed );
		WriteFloat (  1,t.playercontrol.regendelay );

		//  extra settings from V1.01.030 (20100652)
		WriteLong (  1,t.playercontrol.thirdperson.enabled );
		WriteLong (  1,t.playercontrol.thirdperson.startmarkere );
		WriteLong (  1,t.playercontrol.thirdperson.charactere );
		WriteFloat (  1,t.playercontrol.thirdperson.cameradistance );
		WriteFloat (  1,t.playercontrol.thirdperson.cameraheight );
		WriteFloat (  1,t.playercontrol.thirdperson.cameraspeed );

		//  20100653 additions
		WriteFloat (  1,t.playercontrol.thirdperson.camerafocus );
		WriteLong (  1,t.playercontrol.thirdperson.cameralocked );
		WriteLong (  1,t.playercontrol.thirdperson.camerashoulder );
		WriteLong (  1,t.playercontrol.thirdperson.camerafollow );
		WriteLong (  1,t.playercontrol.thirdperson.camerareticle );

	CloseFile (  1 );

	//  Restore
	SetDir (  t.old_s.Get() );
}

void mapfile_collectfoldersandfiles ( cstr levelpathfolder )
{
	LPSTR pOldDir = GetDir();

	// Collect ALL files in string array list
	Undim ( t.filecollection_s );
	g.filecollectionmax = 0;
	Dim ( t.filecollection_s, 500 );

	//  Stage 1 - specify all common files
	//addtocollection("editors\\gfx\\guru-forexe.ico");
	addtocollection( cstr(cstr("languagebank\\")+g.language_s+"\\textfiles\\guru-wordcount.ini").Get() );
	addtocollection(cstr(cstr("languagebank\\")+g.language_s+"\\textfiles\\guru-words.txt").Get() );
	addtocollection("audiobank\\misc\\silence.wav");
	addtocollection("audiobank\\misc\\explode.wav");
	addtocollection("audiobank\\misc\\ammo.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_01.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_02.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_03.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_04.wav");
	addtocollection("editors\\gfx\\14.png");
	addtocollection("scriptbank\\gameloop.lua");
	addtocollection("scriptbank\\gameplayercontrol.lua");
	addtocollection("scriptbank\\global.lua");
	addtocollection("scriptbank\\music.lua");
	addfoldertocollection(cstr(cstr("languagebank\\")+g.language_s+"\\artwork\\watermark").Get() );
	addfoldertocollection(cstr(cstr("languagebank\\")+g.language_s+"\\artwork\\watermark\\branded").Get() );
	addfoldertocollection("scriptbank\\ai");
	addfoldertocollection("scriptbank\\images");
	addfoldertocollection("audiobank\\materials");
	addfoldertocollection("audiobank\\music\\theescape");
	addfoldertocollection("audiobank\\voices\\player");
	addfoldertocollection("audiobank\\voices\\characters");
	addfoldertocollection("audiobank\\character\\soldier\\onAggro");
	addfoldertocollection("audiobank\\character\\soldier\\onAlert");
	addfoldertocollection("audiobank\\character\\soldier\\onDeath");
	addfoldertocollection("audiobank\\character\\soldier\\onHurt");
	addfoldertocollection("audiobank\\character\\soldier\\onHurtPlayer");
	addfoldertocollection("audiobank\\character\\soldier\\onIdle");
	addfoldertocollection("audiobank\\character\\soldier\\onInteract");
	addfoldertocollection("databank");
	addfoldertocollection("savegames");
	addfoldertocollection("titlesbank\\default\\");
	addtocollection("titlesbank\\cursorcontrol.lua");
	addtocollection("titlesbank\\resolutions.lua");
	addtocollection("titlesbank\\fillgameslots.lua");
	addtocollection("titlesbank\\gamedata.lua");
	addtocollection("titlesbank\\restoregame.lua");
	addfoldertocollection("effectbank\\reloaded");
	addfoldertocollection("effectbank\\reloaded\\media");
	addfoldertocollection("effectbank\\explosion");
	addfoldertocollection("effectbank\\particles");
	addfoldertocollection("fontbank");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\ammohealth");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\sliders");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\panels");
	addfoldertocollection("gamecore\\decals\\splat");
	addfoldertocollection("gamecore\\decals\\bloodsplat");
	addfoldertocollection("gamecore\\decals\\impact");
	addfoldertocollection("gamecore\\decals\\gunsmoke");
	addfoldertocollection("gamecore\\decals\\smoke1");
	addfoldertocollection("gamecore\\decals\\muzzleflash4");
	addfoldertocollection("gamecore\\decals\\splash_droplets");
	addfoldertocollection("gamecore\\decals\\splash_foam");
	addfoldertocollection("gamecore\\decals\\splash_large");
	addfoldertocollection("gamecore\\decals\\splash_misty");
	addfoldertocollection("gamecore\\decals\\splash_ripple");
	addfoldertocollection("gamecore\\decals\\splash_small");
	addfoldertocollection("gamecore\\projectiletypes");
	addfoldertocollection("gamecore\\vrcontroller");
	addfoldertocollection("gamecore\\projectiletypes\\fantasy\\fireball");
	addfoldertocollection("gamecore\\projectiletypes\\fantasy\\magicbolt");
	addfoldertocollection("gamecore\\projectiletypes\\modern\\handgrenade");
	addfoldertocollection("gamecore\\projectiletypes\\modern\\rpggrenade");

	// add any material decals that are active
	for ( t.m = 0; t.m <= g.gmaterialmax; t.m++ )
	{
		if ( t.material[t.m].usedinlevel == 1 )
		{
			cstr decalFolder_s = cstr("gamecore\\decals\\")+t.material[t.m].decal_s;
			addfoldertocollection(decalFolder_s.Get());
		}
	}

	addfoldertocollection("gamecore\\muzzleflash");
	addfoldertocollection("gamecore\\projectiletypes");
	//addfoldertocollection("gamecore\\brass"); // now done below when detect guns used in level
	addfoldertocollection("gamecore\\hudlayers");

	//  Stage 1B - Style dependent files
	titles_getstyle ( );
	addtocollection("titlesbank\\style.txt");
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1280x720").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1280x800").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1366x768").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1440x900").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1600x900").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1680x1050").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1920x1080").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1920x1200").Get() );

	//  include original FPM
	addtocollection(t.tmasterlevelfile_s.Get());

	//  add character creator folder, if needed
	if (  t.characterkitcontrol.gameHasCharacterCreatorIn  ==  1 ) 
	{
		addfoldertocollection("characterkit\\bodyandhead");
		addfoldertocollection("characterkit\\FacialHair");
		addfoldertocollection("characterkit\\Headattachments");
		addfoldertocollection("characterkit\\Headattachments");
		if (  PathExist("entitybank\\user\\charactercreator")  ==  1 ) 
		{
			addfoldertocollection("entitybank\\user\\charactercreator");
		}
	}

	// Pre-Stage 2 - clear a list which will collect all folders/files to REMOVE from the final standalone file transfer
	// list, courtesy of the special FPP file which controls the final files to be used for standalone creation
	// (eventually to be controlled from a nice UI)
	std::vector<cstr> fppFoldersToRemoveList;
	std::vector<cstr> fppFilesToRemoveList;
	fppFoldersToRemoveList.clear();
	fppFilesToRemoveList.clear();

	// Stage 2 - collect all files (from all levels)
	t.levelindex=0;
	Dim ( t.levellist_s, 100  );
	t.tlevelstoprocess = 1;
	t.tlevelfile_s="";
	g.projectfilename_s=t.tmasterlevelfile_s;
	while ( t.tlevelstoprocess == 1 ) 
	{
	// load in level FPM
	if ( Len(t.tlevelfile_s.Get())>1 ) 
	{
		g.projectfilename_s=t.tlevelfile_s;
		mapfile_loadproject_fpm ( );
		game_loadinentitiesdatainlevel ( );
	}

	// 061018 - check if an FPP file exists for this level file
	cstr pFPPFile = cstr(Left(g.projectfilename_s.Get(),strlen(g.projectfilename_s.Get())-4)) + ".fpp";
	if ( FileExist( pFPPFile.Get() ) == 1 ) 
	{
		// used to specify additional files required for standalone executable
		// handy as a workaround until reported issue resolved
		int iFPPStandaloneExtraFilesMode = 0;
		Dim ( t.data_s, 999 );
		LoadArray ( pFPPFile.Get(), t.data_s );
		for ( t.l = 0 ; t.l <= 999; t.l++ )
		{
			t.line_s = t.data_s[t.l];
			LPSTR pLine = t.line_s.Get();
			if ( Len(pLine) > 0 ) 
			{
				if ( strnicmp ( pLine, "[standalone add files]", 22 ) == NULL )
				{
					// denotes our standalone extra files
					iFPPStandaloneExtraFilesMode = 1;
				}
				else
				{
					if ( strnicmp ( pLine, "[standalone delete files]", 25 ) == NULL )
					{
						// denotes our standalone remove files
						iFPPStandaloneExtraFilesMode = 2;
					}
					else
					{
						// this prevents newer FPP files from getting confused with this original simple method
						if ( iFPPStandaloneExtraFilesMode == 1 )
						{
							// add
							if ( pLine[strlen(pLine)-1] == '\\' )
							{
								// include whole folder
								addfoldertocollection(pLine);
							}
							else
							{
								// include specific file
								addtocollection(pLine);
							}
						}
						if ( iFPPStandaloneExtraFilesMode == 2 )
						{
							// remove
							if ( pLine[strlen(pLine)-1] == '\\' )
							{
								// remove whole folder
								fppFoldersToRemoveList.push_back(cstr(pLine));
							}
							else
							{
								// remove specific file
								fppFilesToRemoveList.push_back(cstr(pLine));
							}
						}
					}
				}
			}
		}
		UnDim(t.data_s);
	}	

	//  chosen sky, terrain and veg
	addtocollection("skybank\\cloudportal.dds");
	addfoldertocollection(cstr(cstr("skybank\\")+t.skybank_s[g.skyindex]).Get() );
	//addfoldertocollection("skybank\\night"); // no need for NIGHT until we have day/night cycles
	addfoldertocollection(cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() );
	addfoldertocollection(cstr(cstr("vegbank\\")+g.vegstyle_s).Get() );

	// choose all entities and associated files
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			// Check for custom images loaded in lua script
			if ( t.entityelement[t.e].eleprof.aimain_s != "" )
			{
				cstr tLuaScript = g.fpscrootdir_s+"\\Files\\scriptbank\\";
				tLuaScript += t.entityelement[t.e].eleprof.aimain_s;
				FILE* tLuaScriptFile = fopen ( tLuaScript.Get() , "r" );
				if ( tLuaScriptFile )
				{
					char tTempLine[2048];
					while ( !feof(tLuaScriptFile) )
					{
						fgets ( tTempLine , 2047 , tLuaScriptFile );
						if ( strstr ( tTempLine , "LoadImages" ) )
						{
							char* pImageFolder = strstr ( tTempLine , "\"" );
							if ( pImageFolder )
							{
								pImageFolder++;
								char* pImageFolderEnd = strstr ( pImageFolder , "\"" );
								if ( pImageFolderEnd )
								{
									*pImageFolderEnd = '\0';
									cstr tFolderToAdd = cstr( cstr("scriptbank\\images\\") + cstr(pImageFolder) );
									addfoldertocollection ( tFolderToAdd.Get() );
								}
							}
						}

						// Handle new load image and sound commands, they can be in nested folders
						if ( strstr ( tTempLine , "LoadImage " ) 
						||	 strstr ( tTempLine , "LoadImage(" )
						||	 strstr ( tTempLine , "LoadGlobalSound(" ) )
						{
							char* pImageFolder = strstr ( tTempLine , "\"" );
							if ( pImageFolder )
							{
								pImageFolder++;
								char* pImageFolderEnd = strstr ( pImageFolder , "\"" );
								if ( pImageFolderEnd )
								{
									*pImageFolderEnd = '\0';
									cstr pFile = cstr(pImageFolder);
									addtocollection ( pFile.Get() );
								}
							}
						}
						if (strstr(tTempLine, "SetSkyTo(" )) {
							char* pSkyFolder = strstr(tTempLine, "\"");
							if (pSkyFolder)
							{
								pSkyFolder++;
								char* pSkyFolderEnd = strstr(pSkyFolder, "\"");
								if (pSkyFolderEnd)
								{
									*pSkyFolderEnd = '\0';
									cstr tFolderToAdd = cstr(cstr("skybank\\") + cstr(pSkyFolder));
									addfoldertocollection(tFolderToAdd.Get());
								}
							}
						}
					}
					fclose ( tLuaScriptFile );
				}
			}

			//  entity profile file
			t.tentityname1_s=cstr("entitybank\\")+t.entitybank_s[t.entid];
			t.tentityname2_s=cstr(Left(t.tentityname1_s.Get(),Len(t.tentityname1_s.Get())-4))+".bin";
			if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tentityname2_s).Get() ) == 1 ) 
			{
				t.tentityname_s=t.tentityname2_s;
			}
			else
			{
				t.tentityname_s=t.tentityname1_s;
			}
			addtocollection(t.tentityname_s.Get());

			//  entity files in folder
			t.tentityfolder_s=t.tentityname_s;
			for ( t.n = Len(t.tentityname_s.Get()) ; t.n >= 1 ; t.n+= -1 )
			{
				if (  cstr(Mid(t.tentityname_s.Get(),t.n)) == "\\" || cstr(Mid(t.tentityname_s.Get(),t.n)) == "/" ) 
				{
					t.tentityfolder_s=Left(t.tentityfolder_s.Get(),t.n);
					break;
				}
			}

			//  model files (main model, final appended model and all other append
			int iModelAppendFileCount = t.entityprofile[t.entid].appendanimmax;
			if ( Len ( t.entityappendanim[t.entid][0].filename.Get() ) > 0 ) iModelAppendFileCount = 0;
			for ( int iModels = -1; iModels <= iModelAppendFileCount; iModels++ )
			{
				LPSTR pModelFile = "";
				if ( iModels == -1 ) 
				{
					pModelFile = t.entityprofile[t.entid].model_s.Get();
				}
				else
				{
					pModelFile = t.entityappendanim[t.entid][iModels].filename.Get();
				}
				t.tlocaltofpe=1;
				for ( t.n = 1 ; t.n <= Len(pModelFile); t.n++ )
				{
					if (  cstr(Mid(pModelFile,t.n)) == "\\" || cstr(Mid(pModelFile,t.n)) == "/" ) 
					{
						t.tlocaltofpe=0 ; break;
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile1_s=t.tentityfolder_s+pModelFile;
				}
				else
				{
					t.tfile1_s=pModelFile;
				}
				t.tfile2_s=cstr(Left(t.tfile1_s.Get(),Len(t.tfile1_s.Get())-2))+".dbo";
				if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tfile2_s).Get() ) == 1 ) 
				{
					t.tfile_s=t.tfile2_s;
				}
				else
				{
					t.tfile_s=t.tfile1_s;
				}
				t.tmodelfile_s=t.tfile_s;
				addtocollection(t.tmodelfile_s.Get());
				// if entity did not specify texture it is multi-texture, so interogate model file
				// do it for every model
				findalltexturesinmodelfile(t.tmodelfile_s.Get(), t.tentityfolder_s.Get(), t.entityprofile[t.entityelement[t.e].bankindex].texpath_s.Get());
			}

			// Export entity FPE BMP file if flagged
			if ( g.gexportassets == 1 ) 
			{
				t.tfile3_s=cstr(Left(t.tentityname_s.Get(),Len(t.tentityname_s.Get())-4))+".bmp";
				if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tfile3_s).Get() ) == 1 ) 
				{
					addtocollection(t.tfile3_s.Get());
				}
			}

			// entity characterpose file (if any)
			t.tfile3_s=cstr(Left(t.tfile1_s.Get(),Len(t.tfile1_s.Get())-2))+".dat";
			if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tfile3_s).Get() ) == 1 ) 
			{
				addtocollection(t.tfile3_s.Get());
			}

			//  texture files
			for ( int iBothTypes = 0; iBothTypes < 2; iBothTypes++ )
			{
				// can be from ELEPROF of entityelement (older maps point to old texture names) or parent ELEPROF original
				cstr pTextureFile = "", pAltTextureFile = "";
				if ( iBothTypes == 0 ) { pTextureFile = t.entityelement[t.e].eleprof.texd_s; pAltTextureFile = t.entityelement[t.e].eleprof.texaltd_s; } 
				if ( iBothTypes == 1 ) { pTextureFile = t.entityprofile[t.entid].texd_s; pAltTextureFile = t.entityprofile[t.entid].texaltd_s; } 

				t.tlocaltofpe=1;
				for ( t.n = 1 ; t.n<=  Len(pTextureFile.Get()); t.n++ )
				{
					if (  cstr(Mid(pTextureFile.Get(),t.n)) == "\\" || cstr(Mid(pTextureFile.Get(),t.n)) == "/" ) 
					{
						t.tlocaltofpe=0 ; break;
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile_s=t.tentityfolder_s+pTextureFile;
				}
				else
				{
					t.tfile_s=pTextureFile;
				}
				addtocollection(t.tfile_s.Get());

				// always allow a DDS texture of same name to be copied over (for test game compatibility)
				for ( int iTwoExtensions = 0; iTwoExtensions <= 1; iTwoExtensions++ )
				{
					if ( iTwoExtensions == 0 ) t.tfileext_s = Right ( t.tfile_s.Get(), 3);
					if ( iTwoExtensions == 1 ) t.tfileext_s = "dds";
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),6)),2)) == "_d" ) 
					{
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_n."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_s."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_i."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_o."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_cube."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
					}
					int iNewPBRTextureMode = 0;
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),10)),6)) == "_color" ) iNewPBRTextureMode = 6+4;
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),11)),7)) == "_albedo" ) iNewPBRTextureMode = 7+4;
					if ( iNewPBRTextureMode > 0 ) 
					{
						cstr pToAdd;
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_color."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_albedo."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_normal."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_specular."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_metalness."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_gloss."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_ao."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_height."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_detail."+t.tfileext_s; addtocollection(pToAdd.Get());
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile_s=t.tentityfolder_s+pAltTextureFile;
				}
				else
				{
					t.tfile_s=pAltTextureFile;
				}
				addtocollection(t.tfile_s.Get());
			}

			// also include textures specified by textureref entries (from importer export)
			cstr tFPEFilePath = g.fpscrootdir_s+"\\Files\\";
			tFPEFilePath += t.tentityname1_s;
			FILE* tFPEFile = fopen ( tFPEFilePath.Get() , "r" );
			if ( tFPEFile )
			{
				char tTempLine[2048];
				while ( !feof(tFPEFile) )
				{
					fgets ( tTempLine , 2047 , tFPEFile );
					if ( strstr ( tTempLine , "textureref" ) )
					{
						char* pToFilename = strstr ( tTempLine , "=" );
						if ( pToFilename )
						{
							while ( *pToFilename == '=' || *pToFilename == 32 ) pToFilename++;
							if ( pToFilename[strlen(pToFilename)-1] == 13 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 10 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 13 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 10 ) pToFilename[strlen(pToFilename)-1] = 0;
							cstr tTextureFile = cstr( t.tentityfolder_s + cstr(pToFilename) );
							addtocollection ( tTextureFile.Get() );
						}
					}
				}
				fclose ( tFPEFile );
			}

			//  shader file
			t.tfile_s=t.entityelement[t.e].eleprof.effect_s ; addtocollection(t.tfile_s.Get());
			//  script files
			t.tfile_s=cstr("scriptbank\\")+t.entityelement[t.e].eleprof.aimain_s ; addtocollection(t.tfile_s.Get());
			//  for the script associated, scan it and include any references to other scripts
			scanscriptfileandaddtocollection(t.tfile_s.Get());
			//  sound files
			t.tfile_s=t.entityelement[t.e].eleprof.soundset_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset1_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset2_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset3_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset4_s ; addtocollection(t.tfile_s.Get());
			//  collectable guns
			cstr pGunPresent = "";
			if ( Len(t.entityprofile[t.entid].isweapon_s.Get()) > 1 ) pGunPresent = t.entityprofile[t.entid].isweapon_s;
			if ( t.entityprofile[t.entid].isammo == 0 )
			{
				// 270618 - only accept HASWEAPON if NOT ammo, so executables are not bloated with ammo that specifies another weapon type
				if ( Len(t.entityelement[t.e].eleprof.hasweapon_s.Get()) > 1 ) pGunPresent = t.entityelement[t.e].eleprof.hasweapon_s;
			}
			if ( Len(pGunPresent.Get()) > 1 )
			{
				t.tfile_s=cstr("gamecore\\guns\\")+pGunPresent; addfoldertocollection(t.tfile_s.Get());
				t.findgun_s = Lower( pGunPresent.Get() ) ; 
				gun_findweaponindexbyname ( );
				if ( t.foundgunid > 0 ) 
				{
					// ammo and brass
					for ( t.x = 0; t.x <= 1; t.x++ )
					{
						// ammo files
						t.tpoolindex=g.firemodes[t.foundgunid][t.x].settings.poolindex;
						if (  t.tpoolindex>0 ) 
						{
							t.tfile_s=cstr("gamecore\\ammo\\")+t.ammopool[t.tpoolindex].name_s;
							if ( PathExist ( t.tfile_s.Get() ) ) addfoldertocollection(t.tfile_s.Get());
						}

						// brass files
						int iBrassIndex = g.firemodes[t.foundgunid][t.x].settings.brass;
						if ( iBrassIndex > 0 ) 
						{
							t.tfile_s = cstr(cstr("gamecore\\brass\\brass")+Str(iBrassIndex));
							if ( PathExist ( t.tfile_s.Get() ) )
								addfoldertocollection(t.tfile_s.Get());
						}
					}

					// and any projectile files associated with it
					cstr pProjectilePresent = t.gun[t.foundgunid].projectile_s;
					if ( Len(pProjectilePresent.Get()) > 1 )
					{
						t.tfile_s=cstr("gamecore\\projectiletypes\\")+pProjectilePresent; 
						addfoldertocollection(t.tfile_s.Get());
					}
				}
			}
			//  zone marker can reference other levels to jump to
			if (  t.entityprofile[t.entid].ismarker == 3 ) 
			{
				t.tlevelfile_s=t.entityelement[t.e].eleprof.ifused_s;
				if (  Len(t.tlevelfile_s.Get())>1 ) 
				{
					t.tlevelfile_s=cstr("mapbank\\")+levelpathfolder+t.tlevelfile_s+".fpm";
					if (  FileExist(cstr(g.fpscrootdir_s+"\\Files\\"+t.tlevelfile_s).Get()) == 1 ) 
					{
						++t.levelmax;
						t.levellist_s[t.levelmax]=t.tlevelfile_s;
						addtocollection(t.tlevelfile_s.Get());
					}
					else
					{
						//  nope, just a regular string entry in the marker field
						t.tlevelfile_s="";
					}
				}
			}
		}
	}

	//  decide if another level needs loading/processing
	if (  t.levelindex<t.levelmax ) 
	{
		t.tlevelfile_s = "";
		t.tlevelstoprocess = 0;
		while ( t.levelindex<t.levelmax && strcmp ( t.tlevelfile_s.Get(), "" )==NULL ) 
		{
			++t.levelindex;
			t.ttrylevelfile_s=t.levellist_s[t.levelindex];
			for ( t.n = 1; t.n <= t.levelindex-1; t.n++ )
			{
				if ( t.ttrylevelfile_s == t.levellist_s[t.n] ) 
				{
					t.ttrylevelfile_s = "";
					break;
				}
			}
			if ( t.ttrylevelfile_s != "" ) 
			{
				t.tlevelfile_s = t.ttrylevelfile_s;
				t.tlevelstoprocess = 1;
			}
		}
	}
	else
	{
		t.tlevelstoprocess = 0;
	}

	//  for each level file
	}

	//  if multi-level, do NOT include the levelbank\testmap temp files
	t.tignorelevelbankfiles=0;
	if (  g.projectfilename_s != t.tmasterlevelfile_s ) 
	{
		timestampactivity(0,"Ignoring levelbank testmap folder for multilevel standalone");
		t.tignorelevelbankfiles=1;
	}
	else
	{
		//addfoldertocollection("levelbank\\testmap"); // 190417 - dont need contents, comes from FPM load!
		//addtocollection("levelbank\\testmap\\header.dat"); // 190719 - this does not exist at time of scan, and created as part of FPM anyhoo
	}

	// 010917 - go through and remove any X files that have DBO counterparts
	SetDir ( cstr(g.fpscrootdir_s+"\\Files\\").Get() );
	t.filesmax = g.filecollectionmax;
	for ( t.fileindex = 1 ; t.fileindex <= t.filesmax; t.fileindex++ )
	{
		t.src_s=t.filecollection_s[t.fileindex];
		if ( FileExist(t.src_s.Get()) == 1 ) 
		{
			char pSrcFile[1024];
			strcpy ( pSrcFile, t.filecollection_s[t.fileindex].Get() );
			if ( strnicmp ( pSrcFile + strlen(pSrcFile) - 4, ".dbo", 4 ) == NULL )
			{
				cstr dboequiv = cstr(Left(pSrcFile,strlen(pSrcFile)-4))+".x";
				if ( FileExist(dboequiv.Get()) == 1 ) 
				{
					// Found DBO, and an X file sitting alongside it, remove the X from consideration
					removefromcollection ( dboequiv.Get() );
				}
			}
		}
	}

	// also remove folders/files marked by FPP file
	if ( fppFoldersToRemoveList.size() > 0 || fppFilesToRemoveList.size() > 0 )
	{
		for ( int n = 0; n < fppFoldersToRemoveList.size(); n++ )
		{
			cstr pRemoveFolder = fppFoldersToRemoveList[n];
			removeanymatchingfromcollection ( pRemoveFolder.Get() );
		}
		for ( int n = 0; n < fppFilesToRemoveList.size(); n++ )
		{
			cstr pRemoveFile = fppFilesToRemoveList[n];
			removeanymatchingfromcollection ( pRemoveFile.Get() );
		}
	}

	// peek at file collection
	cstr tSeeCollectionContents_s = g.fpscrootdir_s + "\\Files\\mapbank\\contents.bak";
	SaveArray ( tSeeCollectionContents_s.Get(), t.filecollection_s );

	// restore dir
	SetDir(pOldDir);

	// collected all files for this level and stored in t.filecollection_s
	return;
}

void mapfile_savestandalone ( void )
{
	//  In Classic, I would run through the load process and collect files as they
	//  where loaded in. In Reloaded, the currently loaded level data is scanned
	//  to arrive at the required files for the Standalone EXE
	t.interactive.savestandaloneused=1;

	// this flag ensures the loadassets splash does not appear when making standalone
	t.levelsforstandalone = 1;

	// give prompts while standalone is saving
	popup_text("Saving Standalone Game : Collecting Files");

	//  check for character creator usage
	characterkit_checkForCharacters ( );

	// 040316 - v1.13b1 - find the nested folder structure of the level (could be in map bank\Easter\level1.fpm)
	t.told_s=GetDir();
	cstr mapbankpath;
	cstr levelpathfolder;
	if ( g.projectfilename_s.Get()[1] != ':' )
	{
		// relative project path
		mapbankpath = cstr("mapbank\\");
		levelpathfolder = Right ( g.projectfilename_s.Get(), strlen(g.projectfilename_s.Get()) - strlen(mapbankpath.Get()) );
	}
	else
	{
		// absolute project path
		//mapbankpath = t.told_s + cstr("\\mapbank\\");
		mapbankpath = g.mysystem.mapbankAbs_s;
		levelpathfolder = Right ( g.projectfilename_s.Get(), strlen(g.projectfilename_s.Get()) - strlen(mapbankpath.Get()) );
	}

	bool bGotNestedPath = false;
	for ( int n = Len(levelpathfolder.Get()) ; n >= 1 ; n+= -1 )
	{
		if ( cstr(Mid(levelpathfolder.Get(),n)) == "\\" || cstr(Mid(levelpathfolder.Get(),n)) == "/" ) 
		{
			levelpathfolder = Left ( levelpathfolder.Get(), n );
			bGotNestedPath = true;
			break;
		}
	}
	if ( bGotNestedPath==false )
	{
		// 240316 - V1.131v1 - if NO nested folder, string must be empty!
		levelpathfolder = "";
	}

	//  Name without EXE
	t.exename_s=g.projectfilename_s;
	if (  cstr(Lower(Right(t.exename_s.Get(),4))) == ".fpm" ) 
	{
		t.exename_s=Left(t.exename_s.Get(),Len(t.exename_s.Get())-4);
	}
	for ( t.n = Len(t.exename_s.Get()) ; t.n >= 1 ; t.n+= -1 )
	{
		if (  cstr(Mid(t.exename_s.Get(),t.n)) == "\\" || cstr(Mid(t.exename_s.Get(),t.n)) == "/" ) 
		{
			t.exename_s=Right(t.exename_s.Get(),Len(t.exename_s.Get())-t.n);
			break;
		}
	}
	if (  Len(t.exename_s.Get())<2  )  t.exename_s = "mylevel";

	//  the level to start off standalone export
	t.tmasterlevelfile_s=cstr("mapbank\\")+levelpathfolder+t.exename_s+".fpm";
	timestampactivity(0,cstr(cstr("Saving standalone from ")+t.tmasterlevelfile_s).Get() );

	//  Create MYDOCS folder if not exist
	if ( PathExist(g.myownrootdir_s.Get()) == 0 ) file_createmydocsfolder ( );

	//  Get absolute My Games folder
	g.exedir_s="?";
	SetDir (  g.myownrootdir_s.Get() );
	t.mygamesfolder_s="My Games";
	if (  PathExist(t.mygamesfolder_s.Get()) == 0  )  MakeDirectory (  t.mygamesfolder_s.Get() );
	if (  PathExist(t.mygamesfolder_s.Get()) == 1 ) 
	{
		SetDir (  t.mygamesfolder_s.Get() );
		g.exedir_s=GetDir();
	}
	SetDir (  t.told_s.Get() );

	//  Path to EXE (for dealing with relative EXE paths later)
	if (  g.exedir_s.Get()[1] == ':' )
	{
		t.exepath_s=g.exedir_s;
	}
	else
	{
		t.exepath_s=g.exedir_s;
	}
	if (  cstr(Right(t.exepath_s.Get(),1)) != "\\"  )  t.exepath_s = t.exepath_s+"\\";

	// Collect all files and folders and store in t.filecollection_s
	mapfile_collectfoldersandfiles ( levelpathfolder );
	/*
	// Collect ALL files in string array list
	Undim ( t.filecollection_s );
	g.filecollectionmax = 0;
	Dim ( t.filecollection_s, 500 );

	//  Stage 1 - specify all common files
	addtocollection("editors\\gfx\\guru-forexe.ico");
	addtocollection( cstr(cstr("languagebank\\")+g.language_s+"\\textfiles\\guru-wordcount.ini").Get() );
	addtocollection(cstr(cstr("languagebank\\")+g.language_s+"\\textfiles\\guru-words.txt").Get() );
	addtocollection("audiobank\\misc\\silence.wav");
	addtocollection("audiobank\\misc\\explode.wav");
	addtocollection("audiobank\\misc\\ammo.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_01.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_02.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_03.wav");
	addtocollection("audiobank\\misc\\Bullet_FlyBy_04.wav");
	addtocollection("editors\\gfx\\14.png");
	addtocollection("scriptbank\\gameloop.lua");
	addtocollection("scriptbank\\gameplayercontrol.lua");
	addtocollection("scriptbank\\global.lua");
	addtocollection("scriptbank\\music.lua");
	addfoldertocollection(cstr(cstr("languagebank\\")+g.language_s+"\\artwork\\watermark").Get() );
	addfoldertocollection(cstr(cstr("languagebank\\")+g.language_s+"\\artwork\\watermark\\branded").Get() );
	addfoldertocollection("scriptbank\\ai");
	addfoldertocollection("scriptbank\\images");
	addfoldertocollection("audiobank\\materials");
	addfoldertocollection("audiobank\\music\\theescape");
	addfoldertocollection("audiobank\\voices\\player");
	addfoldertocollection("audiobank\\voices\\characters");
	addfoldertocollection("audiobank\\character\\soldier\\onAggro");
	addfoldertocollection("audiobank\\character\\soldier\\onAlert");
	addfoldertocollection("audiobank\\character\\soldier\\onDeath");
	addfoldertocollection("audiobank\\character\\soldier\\onHurt");
	addfoldertocollection("audiobank\\character\\soldier\\onHurtPlayer");
	addfoldertocollection("audiobank\\character\\soldier\\onIdle");
	addfoldertocollection("audiobank\\character\\soldier\\onInteract");
	addfoldertocollection("databank");
	addfoldertocollection("savegames");
	addfoldertocollection("titlesbank\\default\\");
	addtocollection("titlesbank\\cursorcontrol.lua");
	addtocollection("titlesbank\\resolutions.lua");
	addtocollection("titlesbank\\fillgameslots.lua");
	addtocollection("titlesbank\\gamedata.lua");
	addtocollection("titlesbank\\restoregame.lua");
	addfoldertocollection("effectbank\\reloaded");
	addfoldertocollection("effectbank\\reloaded\\media");
	addfoldertocollection("effectbank\\explosion");
	addfoldertocollection("effectbank\\particles");
	addfoldertocollection("fontbank");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\ammohealth");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\sliders");
	addfoldertocollection("languagebank\\neutral\\gamecore\\huds\\panels");
	addfoldertocollection("gamecore\\decals\\splat");
	addfoldertocollection("gamecore\\decals\\bloodsplat");
	addfoldertocollection("gamecore\\decals\\impact");
	addfoldertocollection("gamecore\\decals\\gunsmoke");
	addfoldertocollection("gamecore\\decals\\smoke1");
	addfoldertocollection("gamecore\\decals\\muzzleflash4");
	addfoldertocollection("gamecore\\decals\\splash_droplets");
	addfoldertocollection("gamecore\\decals\\splash_foam");
	addfoldertocollection("gamecore\\decals\\splash_large");
	addfoldertocollection("gamecore\\decals\\splash_misty");
	addfoldertocollection("gamecore\\decals\\splash_ripple");
	addfoldertocollection("gamecore\\decals\\splash_small");
	addfoldertocollection("gamecore\\projectiletypes");
	addfoldertocollection("gamecore\\vrcontroller");
	addfoldertocollection("gamecore\\projectiletypes\\fantasy\\fireball");
	addfoldertocollection("gamecore\\projectiletypes\\fantasy\\magicbolt");
	addfoldertocollection("gamecore\\projectiletypes\\modern\\handgrenade");
	addfoldertocollection("gamecore\\projectiletypes\\modern\\rpggrenade");

	// add any material decals that are active
	for ( t.m = 0; t.m <= g.gmaterialmax; t.m++ )
	{
		if ( t.material[t.m].usedinlevel == 1 )
		{
			cstr decalFolder_s = cstr("gamecore\\decals\\")+t.material[t.m].decal_s;
			addfoldertocollection(decalFolder_s.Get());
		}
	}

	addfoldertocollection("gamecore\\muzzleflash");
	addfoldertocollection("gamecore\\projectiletypes");
	//addfoldertocollection("gamecore\\brass"); // now done below when detect guns used in level
	addfoldertocollection("gamecore\\hudlayers");

	//  Stage 1B - Style dependent files
	titles_getstyle ( );
	addtocollection("titlesbank\\style.txt");
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1280x720").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1280x800").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1366x768").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1440x900").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1600x900").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1680x1050").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1920x1080").Get() );
	addfoldertocollection(cstr(cstr("titlesbank\\")+t.ttheme_s+"\\1920x1200").Get() );

	//  include original FPM
	addtocollection(t.tmasterlevelfile_s.Get());

	//  add character creator folder, if needed
	if (  t.characterkitcontrol.gameHasCharacterCreatorIn  ==  1 ) 
	{
		addfoldertocollection("characterkit\\bodyandhead");
		addfoldertocollection("characterkit\\FacialHair");
		addfoldertocollection("characterkit\\Headattachments");
		addfoldertocollection("characterkit\\Headattachments");
		if (  PathExist("entitybank\\user\\charactercreator")  ==  1 ) 
		{
			addfoldertocollection("entitybank\\user\\charactercreator");
		}
	}

	// Pre-Stage 2 - clear a list which will collect all folders/files to REMOVE from the final standalone file transfer
	// list, courtesy of the special FPP file which controls the final files to be used for standalone creation
	// (eventually to be controlled from a nice UI)
	std::vector<cstr> fppFoldersToRemoveList;
	std::vector<cstr> fppFilesToRemoveList;
	fppFoldersToRemoveList.clear();
	fppFilesToRemoveList.clear();

	// Stage 2 - collect all files (from all levels)
	t.levelindex=0;
	Dim ( t.levellist_s, 100  );
	t.tlevelstoprocess = 1;
	t.tlevelfile_s="";
	g.projectfilename_s=t.tmasterlevelfile_s;
	while ( t.tlevelstoprocess == 1 ) 
	{
	// load in level FPM
	if ( Len(t.tlevelfile_s.Get())>1 ) 
	{
		g.projectfilename_s=t.tlevelfile_s;
		mapfile_loadproject_fpm ( );
		game_loadinentitiesdatainlevel ( );
	}

	// 061018 - check if an FPP file exists for this level file
	cstr pFPPFile = cstr(Left(g.projectfilename_s.Get(),strlen(g.projectfilename_s.Get())-4)) + ".fpp";
	if ( FileExist( pFPPFile.Get() ) == 1 ) 
	{
		// used to specify additional files required for standalone executable
		// handy as a workaround until reported issue resolved
		int iFPPStandaloneExtraFilesMode = 0;
		Dim ( t.data_s, 999 );
		LoadArray ( pFPPFile.Get(), t.data_s );
		for ( t.l = 0 ; t.l <= 999; t.l++ )
		{
			t.line_s = t.data_s[t.l];
			LPSTR pLine = t.line_s.Get();
			if ( Len(pLine) > 0 ) 
			{
				if ( strnicmp ( pLine, "[standalone add files]", 22 ) == NULL )
				{
					// denotes our standalone extra files
					iFPPStandaloneExtraFilesMode = 1;
				}
				else
				{
					if ( strnicmp ( pLine, "[standalone delete files]", 25 ) == NULL )
					{
						// denotes our standalone remove files
						iFPPStandaloneExtraFilesMode = 2;
					}
					else
					{
						// this prevents newer FPP files from getting confused with this original simple method
						if ( iFPPStandaloneExtraFilesMode == 1 )
						{
							// add
							if ( pLine[strlen(pLine)-1] == '\\' )
							{
								// include whole folder
								addfoldertocollection(pLine);
							}
							else
							{
								// include specific file
								addtocollection(pLine);
							}
						}
						if ( iFPPStandaloneExtraFilesMode == 2 )
						{
							// remove
							if ( pLine[strlen(pLine)-1] == '\\' )
							{
								// remove whole folder
								fppFoldersToRemoveList.push_back(cstr(pLine));
							}
							else
							{
								// remove specific file
								fppFilesToRemoveList.push_back(cstr(pLine));
							}
						}
					}
				}
			}
		}
		UnDim(t.data_s);
	}	

	//  chosen sky, terrain and veg
	addtocollection("skybank\\cloudportal.dds");
	addfoldertocollection(cstr(cstr("skybank\\")+t.skybank_s[g.skyindex]).Get() );
	//addfoldertocollection("skybank\\night"); // no need for NIGHT until we have day/night cycles
	addfoldertocollection(cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() );
	addfoldertocollection(cstr(cstr("vegbank\\")+g.vegstyle_s).Get() );

	// choose all entities and associated files
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			// Check for custom images loaded in lua script
			if ( t.entityelement[t.e].eleprof.aimain_s != "" )
			{
				cstr tLuaScript = g.fpscrootdir_s+"\\Files\\scriptbank\\";
				tLuaScript += t.entityelement[t.e].eleprof.aimain_s;
				FILE* tLuaScriptFile = fopen ( tLuaScript.Get() , "r" );
				if ( tLuaScriptFile )
				{
					char tTempLine[2048];
					while ( !feof(tLuaScriptFile) )
					{
						fgets ( tTempLine , 2047 , tLuaScriptFile );
						if ( strstr ( tTempLine , "LoadImages" ) )
						{
							char* pImageFolder = strstr ( tTempLine , "\"" );
							if ( pImageFolder )
							{
								pImageFolder++;
								char* pImageFolderEnd = strstr ( pImageFolder , "\"" );
								if ( pImageFolderEnd )
								{
									*pImageFolderEnd = '\0';
									cstr tFolderToAdd = cstr( cstr("scriptbank\\images\\") + cstr(pImageFolder) );
									addfoldertocollection ( tFolderToAdd.Get() );
								}
							}
						}

						// Handle new load image and sound commands, they can be in nested folders
						if ( strstr ( tTempLine , "LoadImage " ) 
						||	 strstr ( tTempLine , "LoadImage(" )
						||	 strstr ( tTempLine , "LoadGlobalSound(" ) )
						{
							char* pImageFolder = strstr ( tTempLine , "\"" );
							if ( pImageFolder )
							{
								pImageFolder++;
								char* pImageFolderEnd = strstr ( pImageFolder , "\"" );
								if ( pImageFolderEnd )
								{
									*pImageFolderEnd = '\0';
									cstr pFile = cstr(pImageFolder);
									addtocollection ( pFile.Get() );
								}
							}
						}
						if (strstr(tTempLine, "SetSkyTo(" )) {
							char* pSkyFolder = strstr(tTempLine, "\"");
							if (pSkyFolder)
							{
								pSkyFolder++;
								char* pSkyFolderEnd = strstr(pSkyFolder, "\"");
								if (pSkyFolderEnd)
								{
									*pSkyFolderEnd = '\0';
									cstr tFolderToAdd = cstr(cstr("skybank\\") + cstr(pSkyFolder));
									addfoldertocollection(tFolderToAdd.Get());
								}
							}
						}
					}
					fclose ( tLuaScriptFile );
				}
			}

			//  entity profile file
			t.tentityname1_s=cstr("entitybank\\")+t.entitybank_s[t.entid];
			t.tentityname2_s=cstr(Left(t.tentityname1_s.Get(),Len(t.tentityname1_s.Get())-4))+".bin";
			if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tentityname2_s).Get() ) == 1 ) 
			{
				t.tentityname_s=t.tentityname2_s;
			}
			else
			{
				t.tentityname_s=t.tentityname1_s;
			}
			addtocollection(t.tentityname_s.Get());

			//  entity files in folder
			t.tentityfolder_s=t.tentityname_s;
			for ( t.n = Len(t.tentityname_s.Get()) ; t.n >= 1 ; t.n+= -1 )
			{
				if (  cstr(Mid(t.tentityname_s.Get(),t.n)) == "\\" || cstr(Mid(t.tentityname_s.Get(),t.n)) == "/" ) 
				{
					t.tentityfolder_s=Left(t.tentityfolder_s.Get(),t.n);
					break;
				}
			}

			//  model files (main model, final appended model and all other append
			int iModelAppendFileCount = t.entityprofile[t.entid].appendanimmax;
			if ( Len ( t.entityappendanim[t.entid][0].filename.Get() ) > 0 ) iModelAppendFileCount = 0;
			for ( int iModels = -1; iModels <= iModelAppendFileCount; iModels++ )
			{
				LPSTR pModelFile = "";
				if ( iModels == -1 ) 
				{
					pModelFile = t.entityprofile[t.entid].model_s.Get();
				}
				else
				{
					pModelFile = t.entityappendanim[t.entid][iModels].filename.Get();
				}
				t.tlocaltofpe=1;
				for ( t.n = 1 ; t.n <= Len(pModelFile); t.n++ )
				{
					if (  cstr(Mid(pModelFile,t.n)) == "\\" || cstr(Mid(pModelFile,t.n)) == "/" ) 
					{
						t.tlocaltofpe=0 ; break;
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile1_s=t.tentityfolder_s+pModelFile;
				}
				else
				{
					t.tfile1_s=pModelFile;
				}
				t.tfile2_s=cstr(Left(t.tfile1_s.Get(),Len(t.tfile1_s.Get())-2))+".dbo";
				if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tfile2_s).Get() ) == 1 ) 
				{
					t.tfile_s=t.tfile2_s;
				}
				else
				{
					t.tfile_s=t.tfile1_s;
				}
				t.tmodelfile_s=t.tfile_s;
				addtocollection(t.tmodelfile_s.Get());
				// if entity did not specify texture it is multi-texture, so interogate model file
				// do it for every model
				findalltexturesinmodelfile(t.tmodelfile_s.Get(), t.tentityfolder_s.Get(), t.entityprofile[t.entityelement[t.e].bankindex].texpath_s.Get());
			}

			// entity characterpose file (if any)
			t.tfile3_s=cstr(Left(t.tfile1_s.Get(),Len(t.tfile1_s.Get())-2))+".dat";
			if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\"+t.tfile3_s).Get() ) == 1 ) 
			{
				addtocollection(t.tfile3_s.Get());
			}

			//  texture files
			for ( int iBothTypes = 0; iBothTypes < 2; iBothTypes++ )
			{
				// can be from ELEPROF of entityelement (older maps point to old texture names) or parent ELEPROF original
				cstr pTextureFile = "", pAltTextureFile = "";
				if ( iBothTypes == 0 ) { pTextureFile = t.entityelement[t.e].eleprof.texd_s; pAltTextureFile = t.entityelement[t.e].eleprof.texaltd_s; } 
				if ( iBothTypes == 1 ) { pTextureFile = t.entityprofile[t.entid].texd_s; pAltTextureFile = t.entityprofile[t.entid].texaltd_s; } 

				t.tlocaltofpe=1;
				for ( t.n = 1 ; t.n<=  Len(pTextureFile.Get()); t.n++ )
				{
					if (  cstr(Mid(pTextureFile.Get(),t.n)) == "\\" || cstr(Mid(pTextureFile.Get(),t.n)) == "/" ) 
					{
						t.tlocaltofpe=0 ; break;
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile_s=t.tentityfolder_s+pTextureFile;
				}
				else
				{
					t.tfile_s=pTextureFile;
				}
				addtocollection(t.tfile_s.Get());

				// always allow a DDS texture of same name to be copied over (for test game compatibility)
				for ( int iTwoExtensions = 0; iTwoExtensions <= 1; iTwoExtensions++ )
				{
					if ( iTwoExtensions == 0 ) t.tfileext_s = Right ( t.tfile_s.Get(), 3);
					if ( iTwoExtensions == 1 ) t.tfileext_s = "dds";
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),6)),2)) == "_d" ) 
					{
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_n."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_s."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_i."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_o."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
						t.tfile_s=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-6))+"_cube."+t.tfileext_s ; addtocollection(t.tfile_s.Get());
					}
					int iNewPBRTextureMode = 0;
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),10)),6)) == "_color" ) iNewPBRTextureMode = 6+4;
					if ( cstr(Left(Lower(Right(t.tfile_s.Get(),11)),7)) == "_albedo" ) iNewPBRTextureMode = 7+4;
					if ( iNewPBRTextureMode > 0 ) 
					{
						cstr pToAdd;
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_color."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_albedo."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_normal."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_specular."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_metalness."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_gloss."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_ao."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_height."+t.tfileext_s; addtocollection(pToAdd.Get());
						pToAdd=cstr(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-iNewPBRTextureMode))+"_detail."+t.tfileext_s; addtocollection(pToAdd.Get());
					}
				}
				if (  t.tlocaltofpe == 1 ) 
				{
					t.tfile_s=t.tentityfolder_s+pAltTextureFile;
				}
				else
				{
					t.tfile_s=pAltTextureFile;
				}
				addtocollection(t.tfile_s.Get());
			}

			// also include textures specified by textureref entries (from importer export)
			cstr tFPEFilePath = g.fpscrootdir_s+"\\Files\\";
			tFPEFilePath += t.tentityname1_s;
			FILE* tFPEFile = fopen ( tFPEFilePath.Get() , "r" );
			if ( tFPEFile )
			{
				char tTempLine[2048];
				while ( !feof(tFPEFile) )
				{
					fgets ( tTempLine , 2047 , tFPEFile );
					if ( strstr ( tTempLine , "textureref" ) )
					{
						char* pToFilename = strstr ( tTempLine , "=" );
						if ( pToFilename )
						{
							while ( *pToFilename == '=' || *pToFilename == 32 ) pToFilename++;
							if ( pToFilename[strlen(pToFilename)-1] == 13 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 10 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 13 ) pToFilename[strlen(pToFilename)-1] = 0;
							if ( pToFilename[strlen(pToFilename)-1] == 10 ) pToFilename[strlen(pToFilename)-1] = 0;
							cstr tTextureFile = cstr( t.tentityfolder_s + cstr(pToFilename) );
							addtocollection ( tTextureFile.Get() );
						}
					}
				}
				fclose ( tFPEFile );
			}

			//  shader file
			t.tfile_s=t.entityelement[t.e].eleprof.effect_s ; addtocollection(t.tfile_s.Get());
			//  script files
			t.tfile_s=cstr("scriptbank\\")+t.entityelement[t.e].eleprof.aimain_s ; addtocollection(t.tfile_s.Get());
			//  for the script associated, scan it and include any references to other scripts
			scanscriptfileandaddtocollection(t.tfile_s.Get());
			//  sound files
			t.tfile_s=t.entityelement[t.e].eleprof.soundset_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset1_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset2_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset3_s ; addtocollection(t.tfile_s.Get());
			t.tfile_s=t.entityelement[t.e].eleprof.soundset4_s ; addtocollection(t.tfile_s.Get());
			//  collectable guns
			cstr pGunPresent = "";
			if ( Len(t.entityprofile[t.entid].isweapon_s.Get()) > 1 ) pGunPresent = t.entityprofile[t.entid].isweapon_s;
			if ( t.entityprofile[t.entid].isammo == 0 )
			{
				// 270618 - only accept HASWEAPON if NOT ammo, so executables are not bloated with ammo that specifies another weapon type
				if ( Len(t.entityelement[t.e].eleprof.hasweapon_s.Get()) > 1 ) pGunPresent = t.entityelement[t.e].eleprof.hasweapon_s;
			}
			if ( Len(pGunPresent.Get()) > 1 )
			{
				t.tfile_s=cstr("gamecore\\guns\\")+pGunPresent; addfoldertocollection(t.tfile_s.Get());
				t.findgun_s = Lower( pGunPresent.Get() ) ; 
				gun_findweaponindexbyname ( );
				if ( t.foundgunid > 0 ) 
				{
					// ammo and brass
					for ( t.x = 0; t.x <= 1; t.x++ )
					{
						// ammo files
						t.tpoolindex=g.firemodes[t.foundgunid][t.x].settings.poolindex;
						if (  t.tpoolindex>0 ) 
						{
							t.tfile_s=cstr("gamecore\\ammo\\")+t.ammopool[t.tpoolindex].name_s;
							if ( PathExist ( t.tfile_s.Get() ) ) addfoldertocollection(t.tfile_s.Get());
						}

						// brass files
						int iBrassIndex = g.firemodes[t.foundgunid][t.x].settings.brass;
						if ( iBrassIndex > 0 ) 
						{
							t.tfile_s = cstr(cstr("gamecore\\brass\\brass")+Str(iBrassIndex));
							if ( PathExist ( t.tfile_s.Get() ) )
								addfoldertocollection(t.tfile_s.Get());
						}
					}

					// and any projectile files associated with it
					cstr pProjectilePresent = t.gun[t.foundgunid].projectile_s;
					if ( Len(pProjectilePresent.Get()) > 1 )
					{
						t.tfile_s=cstr("gamecore\\projectiletypes\\")+pProjectilePresent; 
						addfoldertocollection(t.tfile_s.Get());
					}
				}
			}
			//  zone marker can reference other levels to jump to
			if (  t.entityprofile[t.entid].ismarker == 3 ) 
			{
				t.tlevelfile_s=t.entityelement[t.e].eleprof.ifused_s;
				if (  Len(t.tlevelfile_s.Get())>1 ) 
				{
					t.tlevelfile_s=cstr("mapbank\\")+levelpathfolder+t.tlevelfile_s+".fpm";
					if (  FileExist(cstr(g.fpscrootdir_s+"\\Files\\"+t.tlevelfile_s).Get()) == 1 ) 
					{
						++t.levelmax;
						t.levellist_s[t.levelmax]=t.tlevelfile_s;
						addtocollection(t.tlevelfile_s.Get());
					}
					else
					{
						//  nope, just a regular string entry in the marker field
						t.tlevelfile_s="";
					}
				}
			}
		}
	}

	//  decide if another level needs loading/processing
	if (  t.levelindex<t.levelmax ) 
	{
		t.tlevelfile_s = "";
		t.tlevelstoprocess = 0;
		while ( t.levelindex<t.levelmax && strcmp ( t.tlevelfile_s.Get(), "" )==NULL ) 
		{
			++t.levelindex;
			t.ttrylevelfile_s=t.levellist_s[t.levelindex];
			for ( t.n = 1; t.n <= t.levelindex-1; t.n++ )
			{
				if ( t.ttrylevelfile_s == t.levellist_s[t.n] ) 
				{
					t.ttrylevelfile_s = "";
					break;
				}
			}
			if ( t.ttrylevelfile_s != "" ) 
			{
				t.tlevelfile_s = t.ttrylevelfile_s;
				t.tlevelstoprocess = 1;
			}
		}
	}
	else
	{
		t.tlevelstoprocess = 0;
	}

	//  for each level file
	}

	//  if multi-level, do NOT include the levelbank\testmap temp files
	t.tignorelevelbankfiles=0;
	if (  g.projectfilename_s != t.tmasterlevelfile_s ) 
	{
		timestampactivity(0,"Ignoring levelbank testmap folder for multilevel standalone");
		t.tignorelevelbankfiles=1;
	}
	else
	{
		//addfoldertocollection("levelbank\\testmap"); // 190417 - dont need contents, comes from FPM load!
		addtocollection("levelbank\\testmap\\header.dat");
	}
	end, moved to mapfile_collectfoldersandfiles
	*/

	// prompt
	popup_text_change("Saving Standalone Game : Creating Paths");

	//  Create game folder
	SetDir (  t.exepath_s.Get() );
	MakeDirectory (  t.exename_s.Get() );
	SetDir (  t.exename_s.Get() );
	MakeDirectory (  "Files" );
	SetDir (  "Files" );

	//  Ensure gamesaves files are removed (if any)
	if (  PathExist("gamesaves") == 1 ) 
	{
		SetDir (  "gamesaves" );
		ChecklistForFiles (  );
		for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
		{
			t.tfile_s=ChecklistString(t.c);
			if (  Len(t.tfile_s.Get())>2 ) 
			{
				if (  FileExist(t.tfile_s.Get()) == 1  )  DeleteAFile (  t.tfile_s.Get() );
			}
		}
		SetDir (  ".." );
	}

	//  Ensure file path exists (by creating folders)
	createallfoldersincollection();

	//  If not copying levelbank files, must still create the folder
	if (  t.tignorelevelbankfiles == 1 ) 
	{
		t.olddir_s=GetDir();
		SetDir (  cstr(t.exepath_s+t.exename_s+"\\Files").Get() );
		if (  PathExist("levelbank") == 0  )  MakeDirectory (  "levelbank" );
		SetDir (  "levelbank" );
		if (  PathExist("testmap") == 0  )  MakeDirectory (  "testmap" );
		SetDir (  "testmap" );
		if (  PathExist("lightmaps") == 0  )  MakeDirectory (  "lightmaps" );
		SetDir (  t.olddir_s.Get() );
	}

	//  If existing standalone there, ensure lightmaps are removed (as they will be unintentionally encrypted)
	t.destpath_s=t.exepath_s+t.exename_s+"\\Files\\levelbank\\testmap\\lightmaps";
	if (  PathExist(t.destpath_s.Get()) == 1 ) 
	{
		t.olddir_s=GetDir();
		SetDir (  t.destpath_s.Get() );
		ChecklistForFiles (  );
		for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
		{
			t.tfile_s=ChecklistString(t.c);
			if (  t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				if (  FileExist(t.tfile_s.Get()) == 1  )  DeleteAFile (  t.tfile_s.Get() );
			}
		}
		SetDir (  t.olddir_s.Get() );
	}

	// prompt
	popup_text_change("Saving Standalone Game : Copying Files");

	// restore dir before proceeding
	SetDir(t.told_s.Get());

	//  CopyAFile (  collection to exe folder )
	t.filesmax = g.filecollectionmax;
	for ( t.fileindex = 1 ; t.fileindex<=  t.filesmax; t.fileindex++ )
	{
		t.src_s=t.filecollection_s[t.fileindex];
		if (  FileExist(t.src_s.Get()) == 1 ) 
		{
			t.dest_s=t.exepath_s+t.exename_s+"\\Files\\"+t.src_s;
			if (  FileExist(t.dest_s.Get()) == 1  )  DeleteAFile (  t.dest_s.Get() );
			CopyAFile (  t.src_s.Get(),t.dest_s.Get() );
		}
	}

	// switch to original root to copy exe files and dependencies
	SetDir ( g.originalrootdir_s.Get() );

	//  Copy game engine and rename it
	t.dest_s=t.exepath_s+t.exename_s+"\\"+t.exename_s+".exe";
	if (  FileExist(t.dest_s.Get()) == 1  )  DeleteAFile (  t.dest_s.Get() );
	CopyAFile ( "Guru-MapEditor.exe", t.dest_s.Get() );

	// Copy critical DLLs
	#ifdef PHOTONMP
	 int iSkipSteamFilesIfPhoton = 5;
	#else
	 int iSkipSteamFilesIfPhoton = 7;
	#endif
	for ( int iCritDLLs = 1; iCritDLLs <= iSkipSteamFilesIfPhoton; iCritDLLs++ )
	{
		LPSTR pCritDLLFilename = "";
		switch ( iCritDLLs )
		{
			case 1 : pCritDLLFilename = "avcodec-57.dll"; break;
			case 2 : pCritDLLFilename = "avformat-57.dll"; break;
			case 3 : pCritDLLFilename = "avutil-55.dll"; break;
			case 4 : pCritDLLFilename = "swresample-2.dll"; break;
			case 5 : pCritDLLFilename = "GGWMR.dll"; break;
			case 6 : pCritDLLFilename = "steam_api.dll"; break;
			case 7 : pCritDLLFilename = "sdkencryptedappticket.dll"; break;
		}
		t.dest_s=t.exepath_s+t.exename_s+"\\"+pCritDLLFilename;
		if ( FileExist(t.dest_s.Get()) == 1 ) DeleteAFile ( t.dest_s.Get() );
		CopyAFile ( pCritDLLFilename, t.dest_s.Get() );
	}

	// Copy steam files (see above)
	#ifdef PHOTONMP
	 // No Steam in Photon build
	#else
	 t.dest_s=t.exepath_s+t.exename_s+"\\steam_appid.txt";
	 if ( FileExist(t.dest_s.Get()) == 1  ) DeleteAFile (  t.dest_s.Get() );
	 if ( FileExist("steam_appid.txt") == 1  ) CopyAFile ( "steam_appid.txt",t.dest_s.Get() );
	#endif

	//  Copy visuals settings file
	t.visuals=t.gamevisuals ; visuals_save ( );
	if (  FileExist("visuals.ini") == 1 ) 
	{
		SetDir (  g.fpscrootdir_s.Get() );
		t.dest_s=t.exepath_s+t.exename_s+"\\visuals.ini";
		if (  FileExist(t.dest_s.Get()) == 1  )  DeleteAFile (  t.dest_s.Get() );
		CopyAFile (  "visuals.ini",t.dest_s.Get() );
	}
	t.visuals=t.editorvisuals  ; visuals_save ( );

	//  Create a setup.ini file here reflecting game
	Dim (  t.setuparr_s,999  );
	t.setupfile_s=t.exepath_s+t.exename_s+"\\setup.ini" ; t.i=0;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[GAMERUN]" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "vsync="+Str(g.gvsync) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "superflatterrain="+Str(t.terrain.superflat) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "smoothcamerakeys="+Str(g.globals.smoothcamerakeys) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "occlusionmode="+Str(g.globals.occlusionmode) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "occlusionsize="+Str(g.globals.occlusionsize) ; ++t.i;
	if ( g.vrqcontrolmode != 0 )
	{
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "hidelowfpswarning=1" ; ++t.i;
	}
	else
	{
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "hidelowfpswarning="+Str(g.globals.hidelowfpswarning) ; ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "hardwareinfomode="+Str(g.ghardwareinfomode) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "fullscreen="+Str(g.gfullscreen) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "aspectratio="+Str(g.gaspectratio) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "dividetexturesize="+Str( g.gdividetexturesize ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "producelogfiles=0"; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "adapterordinal="+Str( g.gadapterordinal ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "adapterd3d11only="+Str( g.gadapterd3d11only ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "hidedistantshadows=0"; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "invmouse="+Str( g.gminvert ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "disablerightmousehold="+Str( g.gdisablerightmousehold ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "profileinstandalone="+Str( 0 ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "allowfragmentation="+Str( t.game.allowfragmentation ) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "pbroverride="+Str( g.gpbroverride ) ; ++t.i;

	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "memskipibr=" + Str(g.memskipibr); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "underwatermode=" + Str(g.underwatermode); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "memskipwatermask=" + Str(g.memskipwatermask); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "lowestnearcamera=" + Str(g.lowestnearcamera); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "standalonefreememorybetweenlevels=" + Str(g.standalonefreememorybetweenlevels); ++t.i;


	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "maxtotalmeshlights=" + Str(g.maxtotalmeshlights); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "maxpixelmeshlights=" + Str(g.maxpixelmeshlights); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "terrainoldlight=" + Str(g.terrainoldlight); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "terrainusevertexlights=" + Str(g.terrainusevertexlights); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "maxterrainlights=" + Str(g.maxterrainlights); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "terrainlightfadedistance=" + Str(g.terrainlightfadedistance); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "showstaticlightinrealtime=" + Str(g.showstaticlightinrealtime); ++t.i;	
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "drawcalloptimizer=" + Str(g.globals.drawcalloptimizer); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "forcenowaterreflection=" + Str(g.globals.forcenowaterreflection); ++t.i;
	
	if ( t.DisableDynamicRes == false )
	{
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "disabledynamicres="+Str( 0 ) ; ++t.i;
	}
	else
	{
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "disabledynamicres="+Str( 1 ) ; ++t.i;
	}

	
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowdistance=" + Str(g.globals.realshadowdistancehigh); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowresolution="+Str(g.globals.realshadowresolution) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascadecount="+Str(g.globals.realshadowcascadecount) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "flashlightshadows=" + Str(g.globals.flashlightshadows); ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "speedshadows=" + Str(g.globals.speedshadows); ++t.i;

	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade0="+Str(g.globals.realshadowcascade[0]) ; ++t.i;
	if (g.globals.realshadowsize[0] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize0=" + Str(g.globals.realshadowsize[0]); ++t.i;	
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade1="+Str(g.globals.realshadowcascade[1]) ; ++t.i;
	if (g.globals.realshadowsize[1] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize1=" + Str(g.globals.realshadowsize[1]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade2="+Str(g.globals.realshadowcascade[2]) ; ++t.i;
	if (g.globals.realshadowsize[2] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize2=" + Str(g.globals.realshadowsize[2]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade3="+Str(g.globals.realshadowcascade[3]) ; ++t.i;
	if (g.globals.realshadowsize[3] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize3=" + Str(g.globals.realshadowsize[3]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade4="+Str(g.globals.realshadowcascade[4]) ; ++t.i;
	if (g.globals.realshadowsize[4] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize4=" + Str(g.globals.realshadowsize[4]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade5="+Str(g.globals.realshadowcascade[5]) ; ++t.i;
	if (g.globals.realshadowsize[5] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize5=" + Str(g.globals.realshadowsize[5]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade6="+Str(g.globals.realshadowcascade[6]) ; ++t.i;
	if (g.globals.realshadowsize[6] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize6=" + Str(g.globals.realshadowsize[6]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowcascade7="+Str(g.globals.realshadowcascade[7]) ; ++t.i;
	if (g.globals.realshadowsize[7] > 0) {
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "realshadowsize7=" + Str(g.globals.realshadowsize[7]); ++t.i;
	}
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "" ; ++t.i;

	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[GAMEMENUOPTIONS]" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicslowterrain="+g.graphicslowterrain_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicslowentity="+g.graphicslowentity_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicslowgrass="+g.graphicslowgrass_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicsmediumterrain="+g.graphicsmediumterrain_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicsmediumentity="+g.graphicsmediumentity_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicsmediumgrass="+g.graphicsmediumgrass_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicshighterrain="+g.graphicshighterrain_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicshighentity="+g.graphicshighentity_s; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "graphicshighgrass="+g.graphicshighgrass_s; ++t.i;

	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[CUSTOMIZATIONS]" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "switchtoalt="+Str(g.ggunaltswapkey1) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "melee key="+Str(g.ggunmeleekey) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "zoomholdbreath="+Str(g.gzoomholdbreath) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyUP="+Str(t.listkey[1]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyDOWN="+Str(t.listkey[2]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyLEFT="+Str(t.listkey[3]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyRIGHT="+Str(t.listkey[4]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyJUMP="+Str(t.listkey[5]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyCROUCH="+Str(t.listkey[6]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyENTER="+Str(t.listkey[7]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyRELOAD="+Str(t.listkey[8]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyPEEKLEFT="+Str(t.listkey[9]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyPEEKRIGHT="+Str(t.listkey[10]) ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "keyRUN="+Str(t.listkey[11]) ; ++t.i;

	// vr extras
	if ( g.vrqcontrolmode != 0 || g.gxbox != 0 )
	{
		// CONTROLLER
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "" ; ++t.i;
		t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[CONTROLLER]" ; ++t.i;
		if ( g.vrqcontrolmode != 0 )
		{
			if ( g.vrqoreducontrolmode == 2 )
			{
				// No controller by default in EDU mode
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xbox=0"; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxcontrollertype=2"; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxinvert=0" ; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxmag=100" ; ++t.i;
			}
			else
			{
				// Also no controller in new VR mode
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xbox=0"; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxcontrollertype=2"; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxinvert=0" ; ++t.i;
				t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxmag=100" ; ++t.i;
			}
		}
		else
		{
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xbox="+Str(g.gxbox); ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxcontrollertype="+Str(g.gxboxcontrollertype); ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxinvert="+Str(g.gxboxinvert) ; ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "xboxmag="+Str(g.gxboxmag) ; ++t.i;
		}
	}
	if ( g.vrqcontrolmode != 0 )
	{
		if ( g.vrqoreducontrolmode == 2 )
		{
			// NO VR IN EDU
		}
		else
		{
			// VR
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "" ; ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[VR]" ; ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "vrmode=3"; ++t.i;
			t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "vrmodemag=100"; ++t.i;
		}
	}

	// Optimization Modes
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "[OPTIMIZESETTINGS]" ; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "drawcalloptimizer=1"; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "speedshadows=1"; ++t.i;
	t.setuparr_s[t.i] = ""; t.setuparr_s[t.i] = t.setuparr_s[t.i] + "forcenowaterreflection=1"; ++t.i;

	if (  FileExist(t.setupfile_s.Get()) == 1  )  DeleteAFile (  t.setupfile_s.Get() );
	SaveArray (  t.setupfile_s.Get(),t.setuparr_s );
	UnDim (  t.setuparr_s );

	//  Also save out the localisation ptr file
	Dim (  t.setuparr_s,2  );
	t.setupfile_s=t.exepath_s+t.exename_s+"\\userdetails.ini";
	t.setuparr_s[0]="[LOCALIZATION]";
	t.setuparr_s[1]=cstr("language=")+g.language_s;
	SaveArray (  t.setupfile_s.Get(),t.setuparr_s );
	UnDim (  t.setuparr_s );

	//  Itinery of all files in standalone
	t.titineryfile_s=t.exepath_s+t.exename_s+"\\contents.txt";
	SaveArray (  t.titineryfile_s.Get(),t.filecollection_s );

	//  cleanup file array
	UnDim (  t.filecollection_s );

	//  encrypt media
	t.dest_s=t.exepath_s+t.exename_s;
	if ( g.gexportassets == 0 ) 
	{
		if ( PathExist( cstr(t.dest_s + "\\Files").Get() ) ) 
		{
			// NOTE; Need to exclude lightmaps from encryptor  set encrypt ignore list "lightmaps"
			EncryptAllFiles ( cstr(t.dest_s + "\\Files").Get() );
		}
	}

	//  if not tignorelevelbankfiles, copy unencrypted files 
	if (  t.tignorelevelbankfiles == 0 ) 
	{
		//  now copy the files we do not want to encrypt
		g.filecollectionmax = 0;
		Dim (  t.filecollection_s,500  );

		//  lightmap DBOs
		SetDir (  cstr(g.fpscrootdir_s+"\\Files\\").Get() );
		t.tfurthestobjnumber=g.lightmappedobjectoffset;
		for ( t.tobj = g.lightmappedobjectoffset; t.tobj<= g.lightmappedobjectoffset+99999 ; t.tobj+= 100 )
		{
			t.tname_s = ""; t.tname_s = t.tname_s + "levelbank\\testmap\\lightmaps\\"+"object"+Str(t.tobj)+".dbo";
			if (  FileExist(t.tname_s.Get()) == 1  )  t.tfurthestobjnumber = t.tobj+100;
		}
		for ( t.tobj = g.lightmappedobjectoffset; t.tobj <= t.tfurthestobjnumber; t.tobj++ )
		{
			t.tname_s = ""; t.tname_s = t.tname_s + "levelbank\\testmap\\lightmaps\\"+"object"+Str(t.tobj)+".dbo";
			if (  FileExist(t.tname_s.Get()) == 1  )  addtocollection(t.tname_s.Get());
		}
		for ( t.tobj = g.lightmappedobjectoffset; t.tobj<= g.lightmappedobjectoffsetfinish; t.tobj++ )
		{
			t.tname_s = ""; t.tname_s = t.tname_s + "levelbank\\testmap\\lightmaps\\"+"object"+Str(t.tobj)+".dbo";
			if (  FileExist(t.tname_s.Get()) == 1  )  addtocollection(t.tname_s.Get());
		}
		t.nmax=500;
		for ( t.n = 0 ; t.n<=  5000 ; t.n+= 100 )
		{
			t.tfile_s=cstr("levelbank\\testmap\\lightmaps\\")+Str(t.n)+".dds";
			if (  FileExist(t.tfile_s.Get()) == 1  )  t.nmax = t.n+100;
		}
		for ( t.n = 0 ; t.n<=  t.nmax; t.n++ )
		{
			t.tfile_s=cstr("levelbank\\testmap\\lightmaps\\")+Str(t.n)+".dds";
			if (  FileExist(t.tfile_s.Get()) == 1  )  addtocollection(t.tfile_s.Get());
		}
		t.tfile_s="levelbank\\testmap\\lightmaps\\objectlist.dat" ; addtocollection(t.tfile_s.Get());
		t.tfile_s="levelbank\\testmap\\lightmaps\\objectnummax.dat" ; addtocollection(t.tfile_s.Get());

		//  Copy the 'unencrypted files' collection to exe folder
		timestampactivity(0, cstr(cstr("filecollectionmax=")+Str(g.filecollectionmax)).Get() );
		if ( PathExist ( cstr(t.exepath_s+t.exename_s+"\\Files\\levelbank\\testmap").Get() ) == 0 )
		{
			SetDir ( cstr(t.exepath_s+t.exename_s+"\\Files\\").Get() );
			if ( PathExist ( "levelbank" ) == 0 ) MakeDirectory ( "levelbank" );
			SetDir ( cstr(t.exepath_s+t.exename_s+"\\Files\\levelbank").Get() );
			if ( PathExist ( "testmap" ) == 0 ) MakeDirectory ( "testmap" );
		}
		SetDir ( cstr(t.exepath_s+t.exename_s+"\\Files\\levelbank\\testmap").Get() );
		if (  PathExist("lightmaps") == 0  )  MakeDirectory (  "lightmaps" );
		SetDir (  cstr(g.fpscrootdir_s+"\\Files\\").Get() );
		for ( t.fileindex = 1 ; t.fileindex<=  g.filecollectionmax; t.fileindex++ )
		{
			t.src_s=t.filecollection_s[t.fileindex];
			if (  FileExist(t.src_s.Get()) == 1 ) 
			{
				t.dest_s=t.exepath_s+t.exename_s+"\\Files\\"+t.src_s;
				if (  FileExist(t.dest_s.Get()) == 1  )  DeleteAFile (  t.dest_s.Get() );
				CopyAFile (  t.src_s.Get(),t.dest_s.Get() );
			}
		}
	}

	//  Restore directory
	SetDir (  t.told_s.Get() );

	//  restore original level FPM files
	timestampactivity(0, cstr(cstr("check '")+g.projectfilename_s+"' vs '"+t.tmasterlevelfile_s+"'").Get() );
	if (  g.projectfilename_s != t.tmasterlevelfile_s ) 
	{
		if (  Len(t.tmasterlevelfile_s.Get())>1 ) 
		{
			g.projectfilename_s=t.tmasterlevelfile_s;
			//  need to load EVERYTHING back in
			gridedit_load_map ( );
		}
	}

	// finish popup system
	popup_text_close();

	// no longer making standalone
	t.levelsforstandalone = 0;
}

void scanscriptfileandaddtocollection ( char* tfile_s )
{
	cstr tscriptname_s =  "";
	cstr tlinethis_s =  "";
	int lookforlen = 0;
	cstr lookfor_s =  "";
	cstr tline_s =  "";
	int l = 0;
	int c = 0;
	int tt = 0;
	Dim (  t.scriptpage_s,10000  );
	if (  FileExist(tfile_s) == 1 ) 
	{
		LoadArray (  tfile_s,t.scriptpage_s );
		lookfor_s=Lower("Include(") ; lookforlen=Len(lookfor_s.Get());
		for ( l = 0 ; l < t.scriptpage_s.size() ; l++ )
		{
			tline_s=Lower(t.scriptpage_s[l].Get());

			for ( c = 0 ; c<=  Len(tline_s.Get())-lookforlen-1; c++ )
			{
				tlinethis_s=Right(tline_s.Get(),Len(tline_s.Get())-c);

				// ignore commented out lines
				if ( cstr( Left( tlinethis_s.Get(), 2 )) == "--" ) break;

				if (  cstr( Left( tlinethis_s.Get(), lookforlen )) == lookfor_s.Get() ) 
				{
					//  found script has included ANOTHER script
					// skip spaces and quotes 
					int i = lookforlen + 1;

					while ( i < Len( tlinethis_s.Get() ) &&
						   ( cstr( Mid( tlinethis_s.Get(), i )) == " " ||
						     cstr( Mid( tlinethis_s.Get(), i )) == "\"" ) ) 
					{
						i++;
					};
			
					// if couldn't find the script name skip this line
					if (i == Len(tlinethis_s.Get())) break;

					tscriptname_s=Right(tline_s.Get(),Len(tline_s.Get())-c-i+1);
				
					for ( tt = Len(tscriptname_s.Get()) ; tt>= 4 ; tt+= -1 )
					{
						if (  cstr(Mid(tscriptname_s.Get(),tt-0)) == "a" && cstr(Mid(tscriptname_s.Get(),tt-1)) == "u" && cstr(Mid(tscriptname_s.Get(),tt-2)) == "l" && cstr(Mid(tscriptname_s.Get(),tt-3)) == "." ) 
						{
							break;
						}
					}
					tscriptname_s=Left(tscriptname_s.Get(),tt);
					addtocollection( cstr(cstr("scriptbank\\")+tscriptname_s).Get() );
				}
			}
		}
	}
	UnDim (  t.scriptpage_s );
}

void addtocollection ( char* file_s )
{
	int tarrsize = 0;
	int tfound = 0;
	int f = 0;
	file_s=Lower(file_s);
	//  Ensure this entry is not already present
	tfound=0;
	for ( f = 1 ; f<=  g.filecollectionmax; f++ )
	{
		if (  t.filecollection_s[f] == cstr(file_s)  )  tfound = 1;
	}
	if (  tfound == 0 ) 
	{
		//  Expand file collection array if nearly full
		++g.filecollectionmax;
		tarrsize=ArrayCount(t.filecollection_s);
		if (  g.filecollectionmax>tarrsize-10 ) 
		{
			Dim (  t.filecollection_s,tarrsize+50  );
		}
		t.filecollection_s[g.filecollectionmax]=file_s;
	}
}

void removefromcollection ( char* file_s )
{
	int tfound = 0;
	file_s=Lower(file_s);
	for ( int f = 1 ; f <= g.filecollectionmax; f++ )
		if ( t.filecollection_s[f] == cstr(file_s)  )  
			tfound = f;
	if ( tfound > 0 ) 
	{
		// remove from consideration
		t.filecollection_s[tfound] = "";
	}
}

void removeanymatchingfromcollection ( char* folderorfile_s )
{
	int tfound = 0;
	folderorfile_s = Lower(folderorfile_s);
	for ( int f = 1; f <= g.filecollectionmax; f++ )
	{
		if ( strnicmp ( t.filecollection_s[f].Get(), folderorfile_s, strlen(folderorfile_s) ) == NULL )
		{
			// remove from consideration
			t.filecollection_s[f] = "";
		}
	}
}

void addfoldertocollection ( char* path_s )
{
	cstr tfile_s =  "";
	cstr told_s =  "";
	int c = 0;
	told_s = GetDir();
	if ( PathExist ( path_s ) )
	{
		SetDir ( path_s );
		ChecklistForFiles (  );
		for ( c = 1 ; c<=  ChecklistQuantity(); c++ )
		{
			if (  ChecklistValueA(c) == 0 ) 
			{
				tfile_s=ChecklistString(c);
				if (  tfile_s != "." && tfile_s != ".." ) 
				{
					addtocollection( cstr(cstr(path_s)+"\\"+tfile_s).Get() );
				}
			}
		}
		SetDir (  told_s.Get() );
	}
	else
	{
		timestampactivity(0, cstr(cstr("Tried adding path that does not exist: ")+path_s).Get() );
	}
}

void addallinfoldertocollection ( cstr subThisFolder_s, cstr subFolder_s )
{
	// into folder
	if ( subThisFolder_s.Len() > 0 ) SetDir ( subThisFolder_s.Get() );

	// first scan and record all files and folders - store folders locally
	ChecklistForFiles();
	int iFoldersCount = ChecklistQuantity();
	cstr* pFolders = new cstr[iFoldersCount+1];
	for ( int c = 1; c <= iFoldersCount; c++ )
	{
		pFolders[c] = "";
		LPSTR pFileFolderName = ChecklistString(c);
		if ( strcmp ( pFileFolderName, "." ) != NULL && strcmp ( pFileFolderName, ".." ) !=NULL )
		{
			if ( ChecklistValueA(c) == 1 )
			{
				pFolders[c] = pFileFolderName;
			}
			else
			{
				cstr relativeFilePath_s = subFolder_s + "\\" + pFileFolderName;
				addtocollection ( relativeFilePath_s.Get() );
			}
		}
	}

	// now use local folder list and investigate each one
	for ( int f = 1; f <= iFoldersCount; f++ )
	{
		cstr pFolderName = pFolders[f];
		if ( pFolderName.Len() > 0 )
		{
			cstr relativeFolderPath_s = pFolderName;
			if ( subFolder_s.Len() > 0 ) relativeFolderPath_s = subFolder_s + "\\" + pFolderName;
			addallinfoldertocollection ( pFolderName, relativeFolderPath_s );
		}
	}

	// back out of folder
	if ( subThisFolder_s.Len() > 0 ) SetDir ( ".." );

	// finally free resources
	delete[] pFolders;
}

void createallfoldersincollection ( void )
{
	LPSTR pOldDir = GetDir();
	t.strwork = ""; t.strwork = t.strwork + "Create full path structure ("+Str(t.filesmax)+") for standalone executable";
	timestampactivity(0, t.strwork.Get() );
	t.filesmax = g.filecollectionmax;
	for ( t.fileindex = 1 ; t.fileindex <= t.filesmax; t.fileindex++ )
	{
		t.olddir_s=GetDir();
		t.src_s=t.filecollection_s[t.fileindex];
		t.srcstring_s=t.src_s;
		while ( Len(t.srcstring_s.Get())>0 ) 
		{
			for ( t.c = 1 ; t.c <= Len(t.srcstring_s.Get()); t.c++ )
			{
				if ( cstr(Mid(t.srcstring_s.Get(),t.c)) == "\\" || cstr(Mid(t.srcstring_s.Get(),t.c)) == "/" ) 
				{
					t.chunk_s=Left(t.srcstring_s.Get(),t.c-1);
					if ( Len(t.chunk_s.Get())>0 ) 
					{
						if ( PathExist(t.chunk_s.Get()) == 0 )  MakeDirectory ( t.chunk_s.Get() );
						if ( PathExist(t.chunk_s.Get()) == 0 ) 
						{
							timestampactivity(0,cstr(cstr("Path:")+t.src_s).Get() );
							timestampactivity(0,cstr(cstr("Unable to create folder:'")+t.chunk_s+"' [error code "+Mid(t.srcstring_s.Get(),t.c)+":"+Str(t.c)+":"+Str(Len(t.srcstring_s.Get()))+"]").Get() );
						}
						if ( PathExist(t.chunk_s.Get()) == 1 ) 
						{
							// sometimes an absolute path can be inserted into path sequence (i.e lee\fred\d;\blob\doug)
							SetDir ( t.chunk_s.Get() );
						}
					}
					t.srcstring_s=Right(t.srcstring_s.Get(),Len(t.srcstring_s.Get())-t.c);
					t.c = 1; // start from beginning as string has been cropped
					break;
				}
			}
			if ( t.c>Len(t.srcstring_s.Get()) ) break;
		}
		SetDir ( t.olddir_s.Get() );
	}
	SetDir ( pOldDir );
}

void findalltexturesinmodelfile ( char* file_s, char* folder_s, char* texpath_s )
{
	cstr returntexfile_s =  "";
	int tfoundpiccy = 0;
	cstr texfile_s =  "";
	int filesize = 0;
	int mbi = 0;
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	//  To determine if a model file requires texture files, we scan the file for a
	//  match to the Text (  .TGA or .JPG (and use texfile$) )
	returntexfile_s="";
	if (  FileExist(file_s) == 1 ) 
	{
		filesize=FileSize(file_s);
		mbi=255;
		OpenToRead (  11,file_s );
		MakeMemblockFromFile (  mbi,11 );
		CloseFile (  11 );
		for ( b = 0 ; b <= filesize-4; b++ )
		{
			if (  ReadMemblockByte(mbi,b+0) == Asc(".") ) 
			{
				tfoundpiccy=0;
				if (  ReadMemblockByte(mbi,b+1) == Asc("T") || ReadMemblockByte(mbi,b+1) == Asc("t") ) 
				{
					if (  ReadMemblockByte(mbi,b+2) == Asc("G") || ReadMemblockByte(mbi,b+2) == Asc("g") ) 
					{
						if (  ReadMemblockByte(mbi,b+3) == Asc("A") || ReadMemblockByte(mbi,b+3) == Asc("a") ) 
						{
							tfoundpiccy=1;
						}
					}
				}
				if (  ReadMemblockByte(mbi,b+1) == Asc("J") || ReadMemblockByte(mbi,b+1) == Asc("j") ) 
				{
					if (  ReadMemblockByte(mbi,b+2) == Asc("P") || ReadMemblockByte(mbi,b+2) == Asc("p") ) 
					{
						if (  ReadMemblockByte(mbi,b+3) == Asc("G") || ReadMemblockByte(mbi,b+3) == Asc("g") ) 
						{
							tfoundpiccy=1;
						}
					}
				}
				if (  ReadMemblockByte(mbi,b+1) == Asc("D") || ReadMemblockByte(mbi,b+1) == Asc("d") ) 
				{
					if (  ReadMemblockByte(mbi,b+2) == Asc("D") || ReadMemblockByte(mbi,b+2) == Asc("d") ) 
					{
						if (  ReadMemblockByte(mbi,b+3) == Asc("S") || ReadMemblockByte(mbi,b+3) == Asc("s") ) 
						{
							tfoundpiccy=1;
						}
					}
				}
				if (  ReadMemblockByte(mbi,b+1) == Asc("B") || ReadMemblockByte(mbi,b+1) == Asc("b") ) 
				{
					if (  ReadMemblockByte(mbi,b+2) == Asc("M") || ReadMemblockByte(mbi,b+2) == Asc("m") ) 
					{
						if (  ReadMemblockByte(mbi,b+3) == Asc("P") || ReadMemblockByte(mbi,b+3) == Asc("p") ) 
						{
							tfoundpiccy=1;
						}
					}
				}
				if (ReadMemblockByte(mbi, b + 1) == Asc("P") || ReadMemblockByte(mbi, b + 1) == Asc("p"))
				{
					if (ReadMemblockByte(mbi, b + 2) == Asc("N") || ReadMemblockByte(mbi, b + 2) == Asc("n"))
					{
						if (ReadMemblockByte(mbi, b + 3) == Asc("G") || ReadMemblockByte(mbi, b + 3) == Asc("g"))
						{
							tfoundpiccy = 1;
						}
					}
				}
				if (  tfoundpiccy == 1 ) 
				{
					//  track back
					for ( c = b ; c >= b-255 ; c+= -1 )
					{
						if (  ReadMemblockByte(mbi,c) >= Asc(" ") && ReadMemblockByte(mbi,c) <= Asc("z") && ReadMemblockByte(mbi,c) != 34 ) 
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
					for ( d = c+1 ; d<=  b+3; d++ )
					{
						texfile_s=texfile_s+Chr(ReadMemblockByte(mbi,d));
					}
					texfile_s=Lower(texfile_s.Get());
					if ( strnicmp ( texfile_s.Get(), "effectbank\\", 11 ) == NULL )
					{
						addtocollection(texfile_s.Get() );
					}
					else
					{
						// detect PBR texture set
						bool bDetectedPBRTextureSetName = false;
						cstr texfilenoext_s=cstr(Left(texfile_s.Get(),Len(texfile_s.Get())-4));
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 6 , "_color", 6 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-6); bDetectedPBRTextureSetName = true; }
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 7 , "_normal", 7 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-7); bDetectedPBRTextureSetName = true; }
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 10 , "_metalness", 10 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-10); bDetectedPBRTextureSetName = true; }
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 10 , "_roughness", 10 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-10); bDetectedPBRTextureSetName = true; }
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 6 , "_gloss", 6 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-6); bDetectedPBRTextureSetName = true; }
						if ( strnicmp ( texfilenoext_s.Get() + strlen(texfilenoext_s.Get()) - 3 , "_ao", 3 ) == NULL ) { texfilenoext_s = Left(texfilenoext_s.Get(),strlen(texfilenoext_s.Get())-3); bDetectedPBRTextureSetName = true; }
						if ( bDetectedPBRTextureSetName == true )
						{
							// add other PBR textures just in case not detected in model data
							cstr texfileColor_s = texfilenoext_s + "_color.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileColor_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileColor_s).Get() );
							cstr texfileNormal_s = texfilenoext_s + "_normal.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileNormal_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileNormal_s).Get() );
							cstr texfileMetalness_s = texfilenoext_s + "_metalness.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileMetalness_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileMetalness_s).Get() );
							cstr texfileGloss_s = texfilenoext_s + "_gloss.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileGloss_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileGloss_s).Get() );
							cstr texfileAO_s = texfilenoext_s + "_ao.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileAO_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileAO_s).Get() );
							cstr texfileIllumination_s = texfilenoext_s + "_illumination.dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfileIllumination_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfileIllumination_s).Get() );
						}
						addtocollection( cstr(cstr(folder_s)+texpath_s+texfile_s).Get() );
						addtocollection( cstr(cstr(folder_s)+texfile_s).Get() );
						if (  cstr(Right(texfile_s.Get(),4)) != ".dds" ) 
						{
							//  also convert to DDS and add those too
							addtocollection( cstr(cstr(folder_s)+texfile_s+".png").Get() );
							texfile_s=cstr(Left(texfile_s.Get(),Len(texfile_s.Get())-4))+".dds";
							addtocollection( cstr(cstr(folder_s)+texpath_s+texfile_s).Get() );
							addtocollection( cstr(cstr(folder_s)+texfile_s).Get() );
						}
					}
					b += 4;
				}
			}
		}
		DeleteMemblock (  mbi );
	}
}

//
// Scan default installation, keep core copy of default files for reference (so know custom content when we see it)
//

void CreateItineraryFile ( void )
{
	// check if we have an itinerary file
	LPSTR pOldDir = GetDir();
	cstr pItineraryFile = "assetitinerary.dat";
	if ( FileExist ( pItineraryFile.Get() ) == 0 )
	{
		// Create itinerary file
		g_sDefaultAssetFiles.clear();
		scanallfolder ( "Files", "" );

		// Save g_sDefaultAssetFiles to pItineraryFile
		SetDir ( pOldDir );
		OpenToWrite ( 1, pItineraryFile.Get() );
		WriteLong ( 1, g_sDefaultAssetFiles.size() );
		for ( int n = 0; n < g_sDefaultAssetFiles.size(); n++ )
		{
			WriteString ( 1, g_sDefaultAssetFiles[n].Get() );
		}
		CloseFile ( 1 );
	}
	else
	{
		// Load itinerary file
		g_sDefaultAssetFiles.clear();
		OpenToRead ( 1, pItineraryFile.Get() );
		int iItineraryCount = ReadLong ( 1 );
		for ( int n = 0; n < iItineraryCount; n++ )
		{
			cstr pFileRef = ReadString ( 1 );
			g_sDefaultAssetFiles.push_back ( pFileRef );
		}
		CloseFile ( 1 );
	}
}

void scanallfolder ( cstr subThisFolder_s, cstr subFolder_s )
{
	// into folder
	if ( subThisFolder_s.Len() > 0 ) SetDir ( subThisFolder_s.Get() );

	// first scan all files and folders - store folders locally
	ChecklistForFiles();
	int iFoldersCount = ChecklistQuantity();
	cstr* pFolders = new cstr[iFoldersCount+1];
	for ( int c = 1; c <= iFoldersCount; c++ )
	{
		pFolders[c] = "";
		LPSTR pFileFolderName = ChecklistString(c);
		if ( strcmp ( pFileFolderName, "." ) != NULL && strcmp ( pFileFolderName, ".." ) !=NULL )
		{
			if ( ChecklistValueA(c) == 1 )
			{
				pFolders[c] = pFileFolderName;
			}
			else
			{
				// found file reference
				cstr relativeFilePath_s = subFolder_s + "\\" + pFileFolderName;

				// clean up string
				LPSTR pOldStr = relativeFilePath_s.Get();
				LPSTR pCleanStr = new char[strlen(pOldStr)+1];
				int nn = 0;
				for ( int n = 0; n < strlen(pOldStr); n++ )
				{
					if ( pOldStr[n] == '\\' && pOldStr[n+1] == '\\' ) n++; // skip duplicate backslashes
					pCleanStr[nn++] = pOldStr[n];
				}
				pCleanStr[nn] = 0; 

				// add to master asset list of known stock assets
				g_sDefaultAssetFiles.push_back ( pCleanStr );
			}
		}
	}

	// now use local folder list and investigate each one
	for ( int f = 1; f <= iFoldersCount; f++ )
	{
		cstr pFolderName = pFolders[f];
		if ( pFolderName.Len() > 0 )
		{
			cstr relativeFolderPath_s = pFolderName;
			if ( subFolder_s.Len() > 0 ) relativeFolderPath_s = subFolder_s + "\\" + pFolderName;
			scanallfolder ( pFolderName, relativeFolderPath_s );
		}
	}

	// back out of folder
	if ( subThisFolder_s.Len() > 0 ) SetDir ( ".." );

	// finally free resources
	delete[] pFolders;
}

bool IsFileAStockAsset ( LPSTR pCheckThisFile )  
{
	for ( int n = 0; n < g_sDefaultAssetFiles.size(); n++ )
	{
		LPSTR pCompare = g_sDefaultAssetFiles[n].Get();
		if ( stricmp ( pCheckThisFile, pCompare ) == NULL )
			return true;
	}
	return false;
}

void ScanLevelForCustomContent ( LPSTR pFPMBeingSaved )
{
	// just before save an FPM, if any custom content used, add it to the FPM 
	// in a special Files folder to be referenced by forced redirection 
	// so it prefers FPM included files over stock assets on the local machine 
	// (which may have been changed, unknown to the user sharing their FPM)
	t.tmasterlevelfile_s=cstr("mapbank\\")+pFPMBeingSaved;
	timestampactivity(0,cstr(cstr("Scanning level for custom content:")+t.tmasterlevelfile_s).Get() );
	mapfile_collectfoldersandfiles ( "" );

	// Go through all files required by level, and create custom files for those that are not in default asset itinerary
	for ( t.fileindex = 1; t.fileindex <= g.filecollectionmax; t.fileindex++ )
	{
		SetDir ( cstr(g.fpscrootdir_s + "\\Files\\").Get() );
		t.src_s = t.filecollection_s[t.fileindex];
		if ( FileExist(t.src_s.Get()) == 0 )
		{
			// special case of X files not existing, but DBOs do (if saving a previously loaded custom content FPM)
			if ( strlen(t.src_s.Get()) > 2 )
			{
				char pReplaceXWithDBO[2048];
				strcpy ( pReplaceXWithDBO, t.src_s.Get() );
				pReplaceXWithDBO[strlen(pReplaceXWithDBO)-2] = 0;
				strcat ( pReplaceXWithDBO, ".dbo" );
				t.src_s = pReplaceXWithDBO;
			}
		}
		if ( FileExist(t.src_s.Get()) == 1 ) 
		{
			// clean up string
			LPSTR pOldStr = t.src_s.Get();
			LPSTR pCleanStr = new char[strlen(t.src_s.Get())+1];
			int nn = 0;
			for ( int n = 0; n < strlen(pOldStr); n++ )
			{
				if ( pOldStr[n] == '\\' && pOldStr[n+1] == '\\' ) n++; // skip duplicate backslashes
				pCleanStr[nn++] = pOldStr[n];
			}
			pCleanStr[nn] = 0; 

			// is this a default file
			if ( IsFileAStockAsset ( pCleanStr ) == false )
			{
				// No - copy file and add to file block of FPM being saved (current calling function)
				LPSTR pOneFiledStr = new char[10+strlen(t.src_s.Get())+1];
				strcpy ( pOneFiledStr, "CUSTOM_" );
				int nnn = 7;
				for ( int n = 0; n < strlen(pOldStr); n++ )
				{
					if ( pOldStr[n] == '\\' && pOldStr[n+1] == '\\' ) n++; // skip duplicate backslashes
					if ( pOldStr[n] == '\\' )
						pOneFiledStr[nnn++] = '_';
					else
						pOneFiledStr[nnn++] = pOldStr[n];
				}
				pOneFiledStr[nnn] = 0; 

				// copt file over and add to file block
				cstr pCustomRefFileSource = g.fpscrootdir_s + "\\Files\\" + pCleanStr;//"\\files\\audiobank\\misc\\item.wav";
				cstr sFileRefOneFileDest = pOneFiledStr;//"CUSTOM_Files_audiobank_misc_item.wav";
				if ( FileExist ( sFileRefOneFileDest.Get() ) ) DeleteFile ( sFileRefOneFileDest.Get() );
				SetDir ( g.mysystem.levelBankTestMap_s.Get() );
				CopyAFile ( pCustomRefFileSource.Get(), sFileRefOneFileDest.Get() );
				AddFileToBlock ( 1, sFileRefOneFileDest.Get() );
			}
		}
	}

	// when done, need to be in levelbank folder for rest of FPM saving
	SetDir ( cstr(g.fpscrootdir_s + "\\Files\\").Get() );
	SetDir ( g.mysystem.levelBankTestMap_s.Get() );
}
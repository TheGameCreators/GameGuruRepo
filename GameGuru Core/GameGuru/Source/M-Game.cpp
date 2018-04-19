//----------------------------------------------------
//--- GAMEGURU - M-Game
//----------------------------------------------------

#include "gameguru.h"
#include "cOccluderThread.h"
#include "CGfxC.h"
#include "DarkLUA.h"

extern bool g_occluderOn;
extern bool	g_occluderf9Mode;
// When in the editor we need an extra vis sort rather then restore prev vis to avoid shadows flickering when adding an entity
bool g_bInEditor = true;

// 
//  Game Module to manage all game flow
// 

// The occluder thread
cOccluderThread*	g_pOccluderThread = NULL;
float				g_fOccluderCamVelX = 0.0f;
float				g_fOccluderCamVelZ = 0.0f;
float				g_fOccluderLastCamX = 0.0f;
float				g_fOccluderLastCamZ = 0.0f;

// externals
//extern bool g_VR920RenderStereoNow;

void game_masterroot ( void )
{
	//  Load all one-off non-graphics assets
	timestampactivity(0,"_game_oneoff_nongraphics");
	game_oneoff_nongraphics ( );

	//  Master loop will run until whole game terminated
	t.game.masterloop=1;
	while ( t.game.masterloop == 1 ) 
	{
		// first hide rendering of 3D while we set up
		SyncMaskOverride ( 0 );

		//  Optionally set resolution for game and setup for dependencies
		timestampactivity(0,"_game_setresolution");
		if (  t.game.set.resolution == 1 ) 
		{
			game_setresolution ( );
			game_postresolutionchange ( );
			t.game.set.resolution=0;
		}

		//  One-off splash screen or animation
		if (  t.game.set.initialsplashscreen == 1 ) 
		{
			t.game.set.initialsplashscreen=0;
		}

		//  Setup level progression settings
		t.game.level=1;
		t.game.levelmax=1;
		t.game.levelloop=1;
		t.game.levelendingcycle=0;
		t.game.lostthegame=0;
		t.game.jumplevel_s="";
		strcpy ( t.game.pAdvanceWarningOfLevelFilename, "" );

		//  specify first level to load (same name as executable)
		if (  t.game.gameisexe == 1 ) 
		{
			t.tapp_s=Appname();
			for ( t.n = Len(t.tapp_s.Get()) ; t.n >= 1 ; t.n+= -1 )
			{
				if (  t.tapp_s.Get()[t.n-1] == '\\' || t.tapp_s.Get()[t.n-1] == '/' ) 
				{
					t.tapp_s=Right(t.tapp_s.Get(),Len(t.tapp_s.Get())-t.n);
					break;
				}
			}
			t.game.jumplevel_s=Left(t.tapp_s.Get(),Len(t.tapp_s.Get())-4);
		}

		//  Title init - If this is just a test game, we only need to set default volumes
		timestampactivity(0,"_titles_init");
		if (  t.game.gameisexe == 1 || t.game.runasmultiplayer == 1 ) 
		{
			titles_init ( );
		}
		else
		{
			t.gamesounds.sounds=100;
			t.gamesounds.music=100;
		}

		//  Do title page
		timestampactivity(0,"_titles_titlepage");
		if (  t.game.gameisexe == 1 && t.game.ignoretitle == 0 ) 
		{
			//titles_titlepage ( );
			terrain_sky_hide();
			titles_loadingpageinit();
			titleslua_init ( );
			titleslua_main ( "title" );
			titleslua_free ( );
			terrain_sky_show();
		}

		//  Standaline Multiplayer HOST/JOIN screen
		if (  t.game.runasmultiplayer == 1 ) 
		{
			//  Show screen, use Steam to get friends CREATE LOBBY, wait, ready to begin
			//  re-use title page system! (see Lee)
			//  DOWNLOAD the level selected by the HOSTER and put it in levelbank
			//  called 'multiplayer_level.zip' (has to be a loop for Steam)
			//  EXTRACT the ZIP into the testmap folder ready for the code below
			//  steam_loop is now called inside titlesbase!!
			g.steamworks.mode = STEAM_MODE_MAIN_MENU;
			timestampactivity(0,"_titles_steampage");
			t.game.cancelmultiplayer=0;
			SetCameraView (  0,0,1,1 );
			titles_steampage ( );
			if (  t.game.cancelmultiplayer == 1 ) 
			{
				//  user selected BACK (cancel multiplayer)
				t.game.levelloop=0;
			}
			else
			{
				//  proceed into level loop where multiplayer spawn markers are detected and ghosts loaded
			}
		}

		// Initialise gun system (transcends per-level initialisations)
		t.game.levelplrstatsetup = 1;
		gun_restart ( );
		gun_resetactivateguns ( );

		//  Level loop will run while level progression is in progress
		while (  t.game.levelloop == 1 ) 
		{
			// also hide rendering of 3D while we set up a new level
			SyncMaskOverride ( 0 );

			//  Loading page
			timestampactivity(0,"_titles_loadingpageupdate");
			if (  t.game.gameisexe == 1 ) 
			{
				//titles_loadingpage ( );
				timestampactivity(0,"LUA script : loading");
				terrain_sky_hide();
				titleslua_init ( );
				titleslua_main ( "loading" );
				terrain_sky_show();
				t.game.levelloadprogress=0  ; titles_loadingpageupdate ( );
			}

			//  Extract level files from FPM
			if (  t.game.runasmultiplayer == 1 ) 
			{
				//  Multiplayer FPM loading
				g.projectfilename_s=g.fpscrootdir_s+"\\Files\\editors\\gridedit\\__multiplayerlevel__.fpm";
				t.trerfeshvisualsassets=1;
				mapfile_loadproject_fpm ( );
				t.game.jumplevel_s="";
			}
			else
			{
				//  Single player
				if (  Len(t.game.jumplevel_s.Get())>1 ) 
				{
					// can override jumplevel with 'advanced warning level filename' when LOAD level from MAIN MENU
					if ( strcmp ( t.game.pAdvanceWarningOfLevelFilename, "" ) != NULL )
					{
						t.game.jumplevel_s = t.game.pAdvanceWarningOfLevelFilename;
						strcpy ( t.game.pAdvanceWarningOfLevelFilename, "" );
					}

					// work out first level from exe name (copied to jumplevel_s)
					g.projectfilename_s = ""; 
					g.projectfilename_s = g.projectfilename_s + "mapbank\\" + t.game.jumplevel_s;
					if ( cstr(Lower(Right(g.projectfilename_s.Get(),4))) != ".fpm" )
						g.projectfilename_s=g.projectfilename_s+".fpm";

					// 050316 - if not there, try all subfolders
					if ( FileExist(cstr(g.fpscrootdir_s+"\\Files\\"+g.projectfilename_s).Get()) == 0 ) 
					{
						// go into mapbank folder
						cstr tthisold_s =  "";
						tthisold_s=GetDir();
						SetDir ( cstr(g.fpscrootdir_s+"\\Files\\mapbank\\").Get() );

						// scan for ALL files/folders
						ChecklistForFiles (  );
						for ( int c = 1 ; c<=  ChecklistQuantity(); c++ )
						{
							if (  ChecklistValueA(c) != 0 ) 
							{
								// only folders
								cstr tfolder_s = ChecklistString(c);
								if ( tfolder_s != "." && tfolder_s != ".." ) 
								{
									// skip . and .. folders
									cstr newlevellocation = "";
									newlevellocation = newlevellocation + "mapbank\\" + tfolder_s + "\\" + t.game.jumplevel_s;
									if ( cstr(Lower(Right(newlevellocation.Get(),4))) != ".fpm" )
										newlevellocation = newlevellocation + ".fpm";

									// does this guessed file location exist
									if ( FileExist(cstr(g.fpscrootdir_s+"\\Files\\"+newlevellocation).Get()) == 1 ) 
									{
										// found the level inside a nested folder
										g.projectfilename_s = newlevellocation; 
										break;
									}
								}
							}
						}
						SetDir ( tthisold_s.Get() );
					}

					// finally load the level in
					mapfile_loadproject_fpm ( );
					t.visuals=t.gamevisuals;
					t.game.jumplevel_s="";
				}
			}

			//  we first load extra guns into gun array EARLY (ahead of entity data load which assigns gunids to isweapon hasweapon)
			gun_tagmpgunstolist ( );

			//  just load the entity data for now (rest in _game_loadinleveldata)
			timestampactivity(0,"_game_loadinentitiesdatainlevel");
			if (  t.game.gameisexe == 1 || t.game.runasmultiplayer == 1 ) 
			{
				//  extra precaution, delete any old entities and LM objects
				if (  t.game.runasmultiplayer == 1 ) 
				{
					entity_delete ( );
					lm_removeold ( );
				}
				game_loadinentitiesdatainlevel ( );
			}

			// Load any extra material sounds associated with new entities (i.e. material(m).usedinlevel=1?)
			// NOTE: Level can collect materials (and material depth) and apply here to quicken material loader (2s)
			material_loadsounds ( );

			// if multiplayer, detect spawn positions and add extra UBER characters
			if ( t.game.runasmultiplayer == 1 ) 
			{
				//  these are the multiplayer start markers
				t.tnumberofstartmarkers = 0;
				g.steamworks.team = 0;
				g.steamworks.coop = 0;
				for ( t.tc = 1 ; t.tc<=  STEAM_MAX_NUMBER_OF_PLAYERS; t.tc++ )
				{
					t.steamworksmultiplayerstart[t.tc].active=0;
				}
				t.plrindex=1;
				t.tfoundAMultiplayerScript = 0;
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					//  reset all updates
					t.entityelement[t.e].mp_updateOn = 0;
					//  for chars like zombies
					t.entityelement[t.e].mp_isLuaChar = 0;
					t.entityelement[t.e].mp_rotateType = 0;
					t.entid=t.entityelement[t.e].bankindex;
					if (  t.entid>0 ) 
					{
						if (  t.entityprofile[t.entid].ismarker == 7 && t.plrindex <= STEAM_MAX_NUMBER_OF_PLAYERS ) 
						{

							//  to ensure mp game script always runs from any distance
							t.entityelement[t.e].eleprof.phyalways = 1;
							if (  t.entityelement[t.e].eleprof.aimain_s  ==  "" ) 
							{
								t.entityelement[t.e].eleprof.aimain_s = "multiplayer_firstto10.lua";
							}
							if (  t.entityelement[t.e].eleprof.teamfield  !=  0 ) 
							{
								g.steamworks.team = 1;
							}
	
							//  only let one marker end up with a script otherwise we end up running the same script 8 times
							if (  t.tfoundAMultiplayerScript  ==  0 ) 
							{
								t.tfoundAMultiplayerScript = 1;
								//  12032015 0XX - Team Multiplayer - check for team mode
								if (  FileOpen(3)  ==  1  )  CloseFile (  3 );
								t.strwork = "" ; t.strwork = t.strwork + "scriptbank\\"+t.entityelement[t.e].eleprof.aimain_s;
								OpenToRead (  3, t.strwork.Get() );
								g.steamworks.friendlyfireoff = 0;
								while (  FileEnd(3)  ==  0 ) 
								{
									t.tScriptLine_s = ReadString (  3 );
									t.tScriptLine_s = Lower(t.tScriptLine_s.Get());
									if (  FindSubString(t.tScriptLine_s.Get(),"setmultiplayergamefriendlyfireoff") > 0 && FindSubString(t.tScriptLine_s.Get(),"--SetMultiplayerGameFriendlyFireOff")  <=  0 && FindSubString(t.tScriptLine_s.Get(),"-- SetMultiplayerGameFriendlyFireOff") <=  0 ) 
									{
										g.steamworks.friendlyfireoff = 1;
									}
								}
								CloseFile (  3 );
							}
							else
							{
								t.entityelement[t.e].eleprof.aimain_s = "";
							}
							++t.plrindex;

						}
						else
						{
							if (  t.entityprofile[t.entid].ischaracter  ==  0 ) 
							{
								if (  FileOpen(3)  ==  1  )  CloseFile (  3 );
								if (  t.entityelement[t.e].eleprof.aimain_s  !=  "" ) 
								{
									if (  FileExist(t.entityelement[t.e].eleprof.aimain_s.Get())  ==  1 ) 
									{
										t.strwork = ""; t.strwork = t.strwork + "scriptbank\\"+t.entityelement[t.e].eleprof.aimain_s;
										OpenToRead (  3, t.strwork.Get() );
										while (  FileEnd(3)  ==  0 ) 
										{
											t.tScriptLine_s = ReadString (  3 );
											t.tScriptLine_s = Lower(t.tScriptLine_s.Get());
											//  are the using ai?
											if (  FindSubString(t.tScriptLine_s.Get(),"AIEntityGoToPosition") > 0 ) 
											{
												t.entityelement[t.e].mp_isLuaChar = 1;
											}
										}
										CloseFile (  3 );
									}
								}
							}
						}
					}
				}
				//  Build multiplayer start markers
				t.thaveTeamAMarkers = 0;
				t.thaveTeamBMarkers = 0;
				t.tmpstartindex = 1;
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					t.entid=t.entityelement[t.e].bankindex;
					if (  t.entid>0 ) 
					{
						if (  t.entityprofile[t.entid].ismarker == 7 && t.tmpstartindex <= STEAM_MAX_NUMBER_OF_PLAYERS ) 
						{
							//  add start markers for free for all or team a
							if (  t.entityelement[t.e].eleprof.teamfield < 2 ) 
							{
								//  a spawn GetPoint (  for the multiplayer )
								t.steamworksmultiplayerstart[t.tmpstartindex].active=1;
								t.steamworksmultiplayerstart[t.tmpstartindex].x=t.entityelement[t.e].x;
								//  added 10 onto the y otherwise the players fall through the ground
								t.steamworksmultiplayerstart[t.tmpstartindex].y=t.entityelement[t.e].y+50;
								t.steamworksmultiplayerstart[t.tmpstartindex].z=t.entityelement[t.e].z;
								t.steamworksmultiplayerstart[t.tmpstartindex].angle=t.entityelement[t.e].ry;
								t.thaveTeamAMarkers = 1;
								++t.tnumberofstartmarkers;
								++t.tmpstartindex;
							}
						}
					}
				}
				//  add team b markers if in team mode
				if (  g.steamworks.team  ==  1 ) 
				{
					for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
					{
						t.entid=t.entityelement[t.e].bankindex;
						if (  t.entid>0 ) 
						{
							if (  t.entityprofile[t.entid].ismarker == 7 && t.tmpstartindex <= STEAM_MAX_NUMBER_OF_PLAYERS ) 
							{
								//  add start markers for team b
								if (  t.entityelement[t.e].eleprof.teamfield  ==  2 ) 
								{
									//  a spawn GetPoint (  for the multiplayer )
									t.steamworksmultiplayerstart[t.tmpstartindex].active=1;
									t.steamworksmultiplayerstart[t.tmpstartindex].x=t.entityelement[t.e].x;
									//  added 10 onto the y otherwise the players fall through the ground
									t.steamworksmultiplayerstart[t.tmpstartindex].y=t.entityelement[t.e].y+50;
									t.steamworksmultiplayerstart[t.tmpstartindex].z=t.entityelement[t.e].z;
									t.steamworksmultiplayerstart[t.tmpstartindex].angle=t.entityelement[t.e].ry;
									t.thaveTeamBMarkers = 1;
									++t.tnumberofstartmarkers;
									++t.tmpstartindex;
								}
							}
						}
					}
				}

				//  check for coop mode
				g.steamworks.coop = 0;
				if (  g.steamworks.team  ==  1 ) 
				{
					if (  (t.thaveTeamAMarkers  ==  1 && t.thaveTeamBMarkers  ==  0) || (t.thaveTeamAMarkers  ==  0 && t.thaveTeamBMarkers  ==  1) || (t.thaveTeamAMarkers  ==  0 && t.thaveTeamBMarkers  ==  0) ) 
					{
						g.steamworks.coop = 1;
						steam_setupCoopTeam ( );
					}
				}

				//  perhaps it is a solo game with a start maker only
				if (  g.steamworks.coop  ==  0 && t.tnumberofstartmarkers  ==  0 ) 
				{
					for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
					{
						t.entid=t.entityelement[t.e].bankindex;
						if (  t.entid>0 ) 
						{
							if (  t.entityprofile[t.entid].ismarker == 1 ) 
							{
								//  a spawn GetPoint (  for the multiplayer )
								t.steamworksmultiplayerstart[1].active=1;
								t.steamworksmultiplayerstart[1].x=t.entityelement[t.e].x;
								//  added 10 onto the y otherwise the players fall through the ground
								t.steamworksmultiplayerstart[1].y=t.entityelement[t.e].y+50;
								t.steamworksmultiplayerstart[1].z=t.entityelement[t.e].z;
								t.steamworksmultiplayerstart[1].angle=t.entityelement[t.e].ry;
								t.entityelement[t.e].eleprof.phyalways = 1;
								//  switch it to multiplayer script
								t.entityelement[t.e].eleprof.aimain_s = "multiplayer_firstto10.lua";
								t.tnumberofstartmarkers = 1;
								g.steamworks.coop = 1;
								g.steamworks.team = 1;
								steam_setupCoopTeam ( );

								//  Check for friendly fire off
								if (  FileOpen(3)  ==  1  )  CloseFile (  3 );
								t.strwork ="" ; t.strwork = t.strwork + "scriptbank\\"+t.entityelement[t.e].eleprof.aimain_s;
								OpenToRead (  3, t.strwork.Get() );
								g.steamworks.friendlyfireoff = 0;
								while (  FileEnd(3)  ==  0 ) 
								{
									t.tScriptLine_s = ReadString (  3 );
									t.tScriptLine_s = Lower(t.tScriptLine_s.Get());
									if (  FindSubString(t.tScriptLine_s.Get(),"setmultiplayergamefriendlyfireoff") > 0 && FindSubString(t.tScriptLine_s.Get(),"--SetMultiplayerGameFriendlyFireOff")  <=  0 && FindSubString(t.tScriptLine_s.Get(),"-- SetMultiplayerGameFriendlyFireOff") <=  0 ) 
									{
										g.steamworks.friendlyfireoff = 1;
									}
								}
								CloseFile (  3 );

							}
						}
					}
				}

				//  if multiplayer and not coop, disable ai characters
				if (  t.game.runasmultiplayer == 1 && g.steamworks.coop  ==  0 ) 
				{
					for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
					{
						t.entid=t.entityelement[t.e].bankindex;
						if (  t.entid>0 ) 
						{
							if (  t.entityprofile[t.entid].ischaracter  ==  1 ) 
							{
								t.entityelement[t.e].destroyme=1;
							}
						}
					}
				}

				//  if multiplayer and coop, setup ai for switching who control them, depending on gameplay circumstances
				if (  t.game.runasmultiplayer == 1 && g.steamworks.coop  ==  1 ) 
				{
					for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
					{
						t.entid=t.entityelement[t.e].bankindex;
						if (  t.entid>0 ) 
						{
							if (  t.entityprofile[t.entid].ischaracter  ==  1 || t.entityelement[t.e].mp_isLuaChar ) 
							{
								t.entityelement[t.e].mp_coopControlledByPlayer = -1;
							}
						}
					}
				}

				//  if no multiplayer markers, put some at the default height
				if (  t.tnumberofstartmarkers  ==  0 ) 
				{
					for ( t.tloop = 1 ; t.tloop<=  STEAM_MAX_NUMBER_OF_PLAYERS; t.tloop++ )
					{
						t.steamworksmultiplayerstart[t.tloop].active=1;
						t.steamworksmultiplayerstart[t.tloop].x=25600;
						//  added 10 onto the y otherwise the players fall through the ground
						t.steamworksmultiplayerstart[t.tloop].y=BT_GetGroundHeight(t.terrain.TerrainID,25600.0,25600.0)+50;
						t.steamworksmultiplayerstart[t.tloop].z=25600;
						t.steamworksmultiplayerstart[t.tloop].angle=0;
					}
				}
				//  if coop and only 1 marker, make some more
				if (  g.steamworks.coop  ==  1 && t.tnumberofstartmarkers  ==  1 ) 
				{
					for ( t.tloop = 2 ; t.tloop<=  STEAM_MAX_NUMBER_OF_PLAYERS; t.tloop++ )
					{
						t.steamworksmultiplayerstart[t.tloop].active=1;
						t.steamworksmultiplayerstart[t.tloop].x=t.steamworksmultiplayerstart[1].x;
						//  added 10 onto the y otherwise the players fall through the ground
						t.steamworksmultiplayerstart[t.tloop].y=t.steamworksmultiplayerstart[1].y;
						t.steamworksmultiplayerstart[t.tloop].z=t.steamworksmultiplayerstart[1].z;
						t.steamworksmultiplayerstart[t.tloop].angle=t.steamworksmultiplayerstart[1].angle;
					}
				}
				//  reserve max multiplayer characters (all weapon animations included)
				Dim (  t.tubindex,2+STEAM_MAX_NUMBER_OF_PLAYERS  );
				t.ent_s=g.rootdir_s+"entitybank\\characters\\Uber Soldier.fpe";
				entity_addtoselection_core ( );
				t.tubindex[0]=t.entid;
				t.entityprofile[t.tubindex[0]].ischaracter=0;
				t.entityprofile[t.tubindex[0]].collisionmode=12;
				// No lua script for player chars
				t.entityprofile[t.tubindex[0]].aimain_s = "";

				if (  g.steamworks.team  ==  1 && g.steamworks.coop  ==  0 ) 
				{
					t.ent_s=g.rootdir_s+"entitybank\\characters\\Uber Soldier Red.fpe";
					entity_addtoselection_core ( );
					t.tubindex[1]=t.entid;
					t.entityprofile[t.tubindex[1]].ischaracter=0;
					t.entityprofile[t.tubindex[1]].collisionmode=12;
					// No lua script for player chars
					t.entityprofile[t.tubindex[1]].aimain_s = "";
					t.tti = 1;
				}

				//  add any character creator player avatars in
				for ( t.tcustomAvatarCount = 0 ; t.tcustomAvatarCount<=  STEAM_MAX_NUMBER_OF_PLAYERS-1; t.tcustomAvatarCount++ )
				{
					//  check if there is a custom avatar
					if (  t.steamworks_playerAvatars_s[t.tcustomAvatarCount]  !=  "" ) 
					{
						//  there is so lets built a temp fpe file from it
						t.ent_s=g.rootdir_s+"entitybank\\user\\charactercreator\\customAvatar_"+Str(t.tcustomAvatarCount)+".fpe";
						t.avatarFile_s = t.ent_s;
						t.avatarString_s = t.steamworks_playerAvatars_s[t.tcustomAvatarCount];
						characterkit_makeMultiplayerCharacterCreatorAvatar ( );
						entity_addtoselection_core ( );
						characterkit_removeMultiplayerCharacterCreatorAvatar ( );
						t.tubindex[t.tcustomAvatarCount+2]=t.entid;
						t.entityprofile[t.tubindex[t.tcustomAvatarCount+2]].ischaracter=0;
						t.entityprofile[t.tubindex[t.tcustomAvatarCount+2]].collisionmode=12;
						// No lua script for player chars
						t.entityprofile[t.tubindex[t.tcustomAvatarCount+2]].aimain_s = "";
					}
				}

				//  store ttiswitch for tti as multiplayer avatars can upset the 0->1 switching!
				t.ttiswitch = 1;
				for ( t.plrindex = 1 ; t.plrindex<=  STEAM_MAX_NUMBER_OF_PLAYERS; t.plrindex++ )
				{
					//  Add the max number of players into the level if there are start markers or not
					if (  g.steamworks.team  ==  1 && g.steamworks.coop  ==  0 ) 
					{
						t.ttiswitch = 1 - t.ttiswitch;
					}
					else
					{
						t.ttiswitch = 0;
					}
					t.tti = t.ttiswitch;
					//  check if the player has their own avatar
					if (  t.steamworks_playerAvatars_s[t.plrindex-1]  !=  "" ) 
					{
						t.tti = t.plrindex-1+2;
					}

					t.ubercharacterindex = t.tubindex[t.tti];
					t.entitymaintype=1 ; t.entitybankindex=t.ubercharacterindex;
					t.gridentityeditorfixed=0;
					t.gridentitystaticmode=0;
					t.gridentityhasparent=0;
					t.tfoundone = 0;
					if (  t.steamworksmultiplayerstart[t.plrindex].active == 1 ) 
					{
						t.tfoundone = 1;
						t.gridentityposx_f=t.steamworksmultiplayerstart[t.plrindex].x;
						t.gridentityposy_f=t.steamworksmultiplayerstart[t.plrindex].y;
						t.gridentityposz_f=t.steamworksmultiplayerstart[t.plrindex].z;
					}
					else
					{
						t.tonetotry = t.plrindex/2;
						if (  t.tonetotry > 0 ) 
						{
							t.tfoundone = 1;
							if (  t.steamworksmultiplayerstart[t.tonetotry].active == 1 ) 
							{
								t.gridentityposx_f=t.steamworksmultiplayerstart[t.tonetotry].x;
								t.gridentityposy_f=t.steamworksmultiplayerstart[t.tonetotry].y;
								t.gridentityposz_f=t.steamworksmultiplayerstart[t.tonetotry].z;
							}
						}
					}
					if (  t.tfoundone  ==  0 ) 
					{
						if (  t.steamworksmultiplayerstart[1].active == 1 ) 
						{
							t.gridentityposx_f=t.steamworksmultiplayerstart[1].x;
							t.gridentityposy_f=t.steamworksmultiplayerstart[1].y;
							t.gridentityposz_f=t.steamworksmultiplayerstart[1].z;
						}
					}
					t.gridentityrotatex_f=0;
					t.gridentityrotatey_f=t.steamworksmultiplayerstart[t.plrindex].angle;
					t.gridentityrotatez_f=0;
					t.gridentityscalex_f=100;
					t.gridentityscaley_f=100;
					t.gridentityscalez_f=100;
					entity_fillgrideleproffromprofile ( );
					entity_addentitytomap_core ( );
					t.steamworksmultiplayerstart[t.plrindex].ghostentityindex=t.e;
					//  Grab the entity number for steam to use
					t.steamworks_playerEntityID[t.plrindex-1] = t.e;
					t.entityprofile[t.ubercharacterindex].ismultiplayercharacter=1;
					t.entityprofile[t.ubercharacterindex].hasweapon_s="";
					t.entityprofile[t.ubercharacterindex].hasweapon=0;
					t.entityprofile[t.ubercharacterindex].aimain_s = "";
				}
				UnDim (  t.tubindex );
			}

			//  in standalone, no IDE feeding test level, so load it in
			timestampactivity(0,"_game_loadinleveldata");
			if (  t.game.gameisexe == 1 || t.game.runasmultiplayer == 1 ) 
			{
				game_loadinleveldata ( );
			}

			//  Prepare this level
			game_preparelevel ( );
			game_preparelevel_forplayer ( );
			game_preparelevel_finally ( );

			//  Load any light map objects if available
			timestampactivity(0,"load lightmapped objects");
			lm_loadscene ( );

			//  Setup variables for main game loop
			timestampactivity(0,"initialise final game variables");
			game_init ( );

			//  Helpful prompt for start of test game
			if (  t.game.gameisexe == 0 && t.game.runasmultiplayer == 0 ) 
			{
				t.visuals.generalpromptstatetimer=Timer()+123;
				t.visuals.generalprompt_s="TAB for settings, F9 for 3D edit, F10 for snapshot, Esc to quit.";
			}
			else
			{
				if (  t.game.runasmultiplayer == 1 ) 
				{
					t.visuals.generalpromptstatetimer=Timer()+1000;
					t.visuals.generalprompt_s="Press RETURN to Chat";
				}
				else
				{
					t.visuals.generalpromptstatetimer=0;
				}
			}

			//  setup spin values, this rotates the player 360 degrees at the start to kill initial
			//  stutter issues, during this time we don't want low FPS warnings
			t.postprocessings.spinfill=0 ; t.postprocessings.spinfillStartAng_f=CameraAngleY(0);

			//  apply any settings
			timestampactivity(0,"immediate title settings applied");
			if (  t.game.gameisexe == 1 || t.game.runasmultiplayer == 1 ) 
			{
				titles_immediateupdatesound ( );
				titles_immediateupdategraphics ( );
			}

			//  for multiplayer, check if there is a jetpack in the level and grab the model to place on players back
			if (  t.game.runasmultiplayer == 1 ) 
			{
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					t.entid=t.entityelement[t.e].bankindex;
					if (  t.entid>0 ) 
					{
						if (  cstr(Lower(Left(t.entityprofileheader[t.entid].desc_s.Get(),8)))  ==   "jet pack"  && ObjectExist(g.steamplayermodelsoffset+120)  ==  0 ) 
						{
							CloneObject (  g.steamplayermodelsoffset+120,t.entityelement[t.e].obj );
							YRotateObject (  g.steamplayermodelsoffset+120,180 );
							FixObjectPivot (  g.steamplayermodelsoffset+120 );
						}
					}
				}
			}

			//  check for character creator characters just before game starts
			characterkit_checkForCharacters ( );

			//  Clear screen of any artifacts
			titles_loadingpagefree();
			CLS (  Rgb(0,0,0) );

			// In EXE running, override cameras with no mask until title/loading done
			SyncMaskOverride ( 0xFFFFFFFF );

			// resort texture list to ignore objects set to be ignored
			DoTextureListSort ( );

			// one final command to improve static physics performance
			physics_finalize ( );

			// Wipe out mouse deltas
			t.tMousemove_f = MouseMoveX() + MouseMoveY() + MouseZ(); t.tMousemove_f  = 0;

			//  Tab mode LOW FPS Warning
			g.tabmode=0 ; g.lowfpstarttimer=Timer();

			//  Game loop will run while single level is in play
			t.huddamage.immunity=1000;
			t.game.gameloop=1;
			g.timeelapsed_f=0;
			t.luaglobal.scriptprompt_s="";
			t.luaglobal.scriptprompttime=0;
			t.luaglobal.scriptprompttextsize=0;
			t.luaglobal.scriptprompt3dtime=0;
			strcpy ( t.luaglobal.scriptprompt3dtext, "" );
			
			//  Game cycle loop
			timestampactivity(0,"main game loop begins");
			while ( t.game.gameloop == 1 ) 
			{
				// detect if standalone is a foreground window
				if ( t.game.gameisexe == 1 )
				{
					HWND hForeWnd = GetForegroundWindow();
					if ( GetWindowHandle() != hForeWnd ) 
						t.plrhasfocus = 0;
					else
						t.plrhasfocus = 1;
				}

				// if controller active, also detect for START button press (same as ESCAPE)
				/*
				char pScan[40];
				strcpy ( pScan, "012345678901234567890123456789012345" );
				if ( g.gxbox > 0 )
				{
					for ( int iA = 0; iA <= 31; iA++ ) pScan[iA] = 48+JoystickFireXL(iA);
					pScan[iA]=0;
				}
				*/
				bool bControllerEscape = false;
				if ( g.gxbox > 0 && JoystickFireXL(9) == 1 ) bControllerEscape = true;

				//  trigger options page or exit test level
				if ( EscapeKey() == 1 || bControllerEscape == true ) 
				{
					t.tremembertimer=Timer();
					game_main_snapshotsoundloopcheckpoint ( );
					game_stopallsounds ( 1 );
					while ( EscapeKey() != 0 ) {}
					darkai_character_freezeall ( );
					physics_pausephysics ( );
					entity_pauseanimations ( );
					if ( t.currentgunobj > 0 ) { if ( ObjectExist(t.currentgunobj)==1 ) { SetObjectSpeed ( t.currentgunobj,0) ; } }
					if (  t.playercontrol.jetobjtouse>0 ) 
					{
						if (  ObjectExist(t.playercontrol.jetobjtouse) == 1  )  SetObjectSpeed (  t.playercontrol.jetobjtouse,0 );
					}
					if (  t.game.gameisexe == 0 && t.game.runasmultiplayer == 0 ) 
					{
						t.game.gameloop=0 ; t.game.levelloop=0 ; t.game.masterloop=0;
						if (  t.conkit.editmodeactive == 1 ) 
						{
							conkitedit_switchoff ( );
						}
					}
					else
					{
						g.titlesettings.updateshadowsaswell=1;
						timestampactivity(0,"entering options page");
						titleslua_init ( );
						titleslua_main ( "gamemenu" );
						timestampactivity(0,"leaving options page");
						g.titlesettings.updateshadowsaswell=0;
					}
					if ( t.currentgunobj>0 ) { if ( ObjectExist(t.currentgunobj) == 1 ) { SetObjectSpeed ( t.currentgunobj,t.currentgunanimspeed_f); } }
					physics_resumephysics ( );
					entity_resumeanimations ( );
					t.aisystem.cumilativepauses=Timer()-t.tremembertimer;
					game_main_snapshotsoundloopresume ( );
					t.strwork = ""; t.strwork = t.strwork + "resuming game loop with flag "+Str(t.game.gameloop);
					timestampactivity(0, t.strwork.Get() );
					// Wipe out mouse deltas
					t.tMousemove_f = MouseMoveX() + MouseMoveY() + MouseZ(); t.tMousemove_f  = 0;
				}

				//  Fade in gamescreen (using post process shader)
				if (  t.postprocessings.fadeinvalue_f<1.0 ) 
				{
					// Hide Lua Sprites
					HideOrShowLUASprites ( true );

					//  Once spinfill is complete we can begin fading in. When complete we can enable low fps warnings again
					if ( t.postprocessings.spinfill < 360 ) 
					{
						t.postprocessings.spinfill += 10;
						if ( t.postprocessings.spinfill >= 360 )
						{
							// 020316 - v1.13b1 - restore to agreed start orientation
							RotateCamera ( 0, t.terrain.playerax_f, t.terrain.playeray_f, t.terrain.playeraz_f );
						}
						else
						{
							// spin around to fill video memory with immediate scenery
							RotateCamera (  0,0,t.postprocessings.spinfillStartAng_f + t.postprocessings.spinfill,0 );
						}

						// Wipe out mouse deltas during spin up
						t.tMousemove_f = MouseMoveX() + MouseMoveY() + MouseZ(); t.tMousemove_f  = 0;
					}
					else
					{
						if ( t.postprocessings.fadeinvalue_f <= 0 )
						{
							// only if in test game mode, standalone already set volume values (in title.lua)
							if ( t.game.gameisexe == 0 )
							{
								// set music and sound global volumes
								audio_volume_init ( );
							}
							// and update internal volume values so music update can use volumes!
							audio_volume_update ( );
						}
						t.postprocessings.fadeinvalue_f=t.postprocessings.fadeinvalue_f+(g.timeelapsed_f*0.1f);
						if (  t.postprocessings.fadeinvalue_f >= 1.0f ) 
						{
							t.postprocessings.fadeinvalue_f=1.0f;
							//g.globals.hidelowfpswarning = 0; // this overrides the SETUP.INI setting
							HideOrShowLUASprites ( false );
							EnableAllSprites(); // the disable is called in DarkLUA by ResetFade() black out command when load game position
						}
					}
					t.postprocessings.fadeinvalueupdate=1;
				}
				if (  t.postprocessings.fadeinvalueupdate == 1 ) 
				{
					t.postprocessings.fadeinvalueupdate=0;
					if (  GetEffectExist(g.postprocesseffectoffset+0) == 1 ) 
					{
						SetVector4 (  g.terrainvectorindex,t.postprocessings.fadeinvalue_f,t.postprocessings.fadeinvalue_f,t.postprocessings.fadeinvalue_f,0 );
						SetEffectConstantV (  g.postprocesseffectoffset+0,"OverallColor",g.terrainvectorindex );
						SetEffectConstantV (  g.postprocesseffectoffset+2,"OverallColor",g.terrainvectorindex );
						SetEffectConstantV (  g.postprocesseffectoffset+4,"OverallColor",g.terrainvectorindex );
					}
					//  210115 - helps refresh scene at very start
					t.visuals.refreshshaders=1;
				}

				//  Immunity when respawn
				if (  t.huddamage.immunity>0 ) 
				{
					t.huddamage.immunity=t.huddamage.immunity-(10*g.timeelapsed_f);
					if (  t.huddamage.immunity<0  )  t.huddamage.immunity = 0;
				}

				//  Run all game subroutines
				game_main_loop ( );

				//  Update screen
				game_sync ( );

			} //  Game cycle loop end

			//  Rest any internal game variables
			game_main_stop ( );

			//  Free any level resources
			game_freelevel ( );

			// must reset LUA here for clean end-game-screens
			// ensure LUA is completely reset before loading new ones in
			// the free call is because game options menu init, but not freed back then
			timestampactivity(0,"resetting LUA system");
			titleslua_free ( );
			LuaReset (  );

			//PE: restore waterline.
			t.terrain.waterliney_f = g.gdefaultwaterheight;

			// 240316 - additional cleanup
			steam_freefadesprite ( );

			// if VR, deactivate at this point
			//g_VR920RenderStereoNow = false;

			//  Advance level to 'next one' or 'win game'
			timestampactivity(0,"end of level stage");
			if (  t.game.gameisexe == 1 )
			{
				timestampactivity(0,"game is standalone exe");
				if (  t.game.quitflag == 0 ) 
				{
					timestampactivity(0,"game has not quit");
					if (  Len(t.game.jumplevel_s.Get())>1 ) 
					{
						//  goes around and loads this level name
						timestampactivity(0,"game is loading non-linear level map:");
						timestampactivity(0,"t.game.jumplevel_s.Get()");
					}
					else
					{
						// win, lose or next level pages
						if (  t.game.lostthegame == 1 ) 
						{
							//titles_gamelostpage ( );
							timestampactivity(0,"LUA script : lose");
							terrain_sky_hide();
							titleslua_init ( );
							titleslua_main ( "lose" );
							titleslua_free ( );
							terrain_sky_show();
							t.game.levelloop=0;
						}
						else
						{
							t.game.level=t.game.level+1;
							if (  t.game.level>t.game.levelmax ) 
							{
								//titles_gamewonpage ( );
								timestampactivity(0,"LUA script : win");
								terrain_sky_hide();
								titleslua_init ( );
								titleslua_main ( "win" );
								titleslua_free ( );
								terrain_sky_show();
								t.game.levelloop=0;
							}
							else
							{
								//titles_completepage ( );
								timestampactivity(0,"LUA script : nextlevel");
								terrain_sky_hide();
								titleslua_init ( );
								titleslua_main ( "nextlevel" );
								terrain_sky_show();
							}
						}
					}
				}
			}
			t.game.quitflag=0;

			//  If was in multiplayer session, no level loop currently
			if (  t.game.runasmultiplayer == 1 ) 
			{
				t.game.levelloop=0;
			}

		//  Level loop end
		}

		//  Free any game resources
		game_freegame ( );

		if (  t.game.runasmultiplayer == 1 ) 
		{
			steam_free_game ( );
			steam_cleanupGame ( );
			if (  g.steamworks.goBackToEditor  ==  1 ) 
			{
				g.steamworks.goBackToEditor = 0;
				t.game.masterloop = 0;
			}
		}

		//  get rid of debris and particles that may be lingering
		explosion_cleanup ( );

		//  if ignored title, exit now
		if (  t.game.ignoretitle == 1 && t.game.runasmultiplayer == 0  )  t.game.masterloop = 0;

		//  Master loop end
		if ( t.game.allowfragmentation == 0 ) break;
	}

	//  End splash if EXE is advertising
	if (  t.game.set.endsplash == 1 ) 
	{
		t.game.set.endsplash=0;
	}
}

void game_setresolution ( void )
{
	//  set game resolution here
	t.multisamplingfactor=0;
	t.multimonitormode=0;
	SetDisplayMode (  GetDesktopWidth(),GetDesktopHeight(),32,g.gvsync,t.multisamplingfactor,t.multimonitormode );
	SyncOn (   ); SyncRate (  0  ); Sync (   ); SetAutoCamOff (  );
	DisableEscapeKey (  );
}

void game_postresolutionchange ( void )
{
}

void game_oneoff_nongraphics ( void )
{
	// Trigger a sound stops initial slow-down?
	if ( SoundExist(g.silentsoundoffset) == 1 ) 
	{
		PlaySound ( g.silentsoundoffset );
		PositionSound ( g.silentsoundoffset,0,0,0 );
	}

	//  Force all weapons into weapon slots (initial default start)
	gun_gatherslotorder ( );
}

void game_loadinentitiesdatainlevel ( void )
{
	//  Load player settings
	timestampactivity(0,"Load player config");
	mapfile_loadplayerconfig ( );

	//  Load entity bank and elements
	t.screenprompt_s="LOADING ENTITIES DATA";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	entity_loadbank ( );
	entity_loadelementsdata ( );
}

void game_loadinleveldata ( void )
{
	//  Load waypoints
	t.screenprompt_s="LOADING WAYPOINTS DATA";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	waypoint_loaddata ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	waypoint_recreateobjs ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Load terrain
	t.screenprompt_s="LOADING TERRAIN DATA";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	terrain_loaddata ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Recreate all entities in level
	t.screenprompt_s="CREATE ENTITY OBJECTS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());

	for ( t.tupdatee = 1 ; t.tupdatee<=  g.entityelementlist; t.tupdatee++ )
	{
		entity_updateentityobj ( );
		if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	}
	t.terrain.terrainpainteroneshot=0;

	//  default start position is edit-camera XZ (Y done in physics init call)
	t.terrain.playerx_f=25000;
	t.terrain.playery_f=0;
	t.terrain.playerz_f=25000;
	t.terrain.playerax_f=0.0;
	t.terrain.playeray_f=0.0;
	t.terrain.playeraz_f=0.0;

	//  hide all markers
	t.screenprompt_s="GAME OBJECT CLEANUP";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.obj=t.entityelement[t.e].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1 ) 
			{
				t.entid=t.entityelement[t.e].bankindex;
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
	waypoint_hideall ( );

return;

}

void game_preparelevel ( void )
{
	//Dave Performance - ensure sound volume is 0
	// 271115 - Dave, you just wiped out all dynamic music volume!
	//g.musicsystem.percentageVolume = 0;
	//t.audioVolume.music = 0;
	//t.audioVolume.sound = 0;
	//t.audioVolume.musicFloat = 0.0;
	//t.audioVolume.soundFloat = 0.0;

	//  Init music system first to make sure nothing is playing during the load sequence
	// 271115 - has to be here as LUA triggers Play Music during its INIT but if MUSIC_INIT was
	// called last it would stop the default music from playing, but setting volume to zero 
	// will achieve the same result of keeping music silent until ready
	music_init ( );
	t.audioVolume.musicFloat = 0;
	t.audioVolume.soundFloat = 0;

	//  Load all assets required to perform level
	timestampactivity(0,"_game_preparelevel:");

	//  Switch on post process if it was switched off (init called later in _finally subroutine)
	postprocess_on ( );

	//  (re)load any player sounds (player style specified in player start marker)
	material_loadplayersounds ( );

	//  init character sound
	character_sound_init ( );

	//  particles
	ravey_particles_init ( );

	//  Allow Steam to refresh (so does not stall)
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  HUD graphics
	t.screenprompt_s="LOADING HUD GRAPHICS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	hud_init ( );

	//  Load sky spec (for any shaders later that require sun-pos)
	t.screenprompt_s="LOADING SKY";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	t.terrainskyspecinitmode=0;
	terrain_skyspec_init ( );

	//  Update terrain height map so GetGroundHeight command will work
	t.screenprompt_s="UPDATED TERRAIN";
	timestampactivity(0,t.screenprompt_s.Get());
	terrain_update ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Load weapon system
	t.screenprompt_s="LOADING NEW WEAPONS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	//gun_restart ( ); // 020516 - moved out of level loop (as it transcends per level assets)
	gun_activategunsfromentities ( );
	gun_setup ( );
	gun_loadonlypresent ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Load in HUD Layer assets
	t.screenprompt_s="LOADING HUD LAYERS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	hud_scanforhudlayers ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Veg Creation
	t.screenprompt_s="PREPARING VEGETATION";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	terrain_fastveg_init ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  setup terrain for in-game
	t.screenprompt_s="LOADING WATER SYSTEM";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	terrain_start_play ( );
	terrain_water_init ( );
	terrain_water_loop ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  setup terrain for in-game
	t.screenprompt_s="LOADING A.I SYSTEM";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	darkai_init ( );
	t.aisystem.containerpathmax=0;
	t.screenprompt_s="PREPARING A.I SYSTEM";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	darkai_preparedata ( );

	//  Reset waypoint for game activity
	t.screenprompt_s="RESETTING WAYPOINTS A.I";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	waypoint_reset ( );

	//  setup entities
	t.screenprompt_s="CREATING ENTITY A.I";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	entity_init ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  create A.I entities for all characters
	t.screenprompt_s="CREATING CHARACTERS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	darkai_setup_characters ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  When all static entities added, complete obstacle map
	if (  g.gskipobstaclecreation == 0 ) 
	{
		t.screenprompt_s="CREATING A.I OBSTACLES";
		if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
		timestampactivity(0,t.screenprompt_s.Get());
	}
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	darkai_completeobstacles ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  setup infinilights
	t.screenprompt_s="PREPARING DYNAMIC LIGHTS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	lighting_init ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  particles/flak/debris required
	decal_activatedecalsfromentities ( );
	material_activatedecals ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	decal_loadonlyactivedecals ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  load required scripts
	lua_init ( );
	lua_scanandloadactivescripts ( );

	//  load entity sounds and video
	entity_loadactivesoundsandvideo ( );
}

void game_preparelevel_forplayer ( void )
{
	//  Player settings
	physics_player_init ( );

	//  player start position must come from level setuo info
	t.terrain.gameplaycamera=0;
	SetCurrentCamera (  t.terrain.gameplaycamera );
	PositionCamera (  t.terrain.gameplaycamera,t.terrain.playerx_f,t.terrain.playery_f,t.terrain.playerz_f );
	PointCamera (  t.terrain.gameplaycamera,0,100,0 );
	RotateCamera (  t.terrain.gameplaycamera,t.terrain.playerax_f,t.terrain.playeray_f,t.terrain.playeraz_f );

	//  should be done in visual update call now!!
	SetCameraRange (  t.terrain.gameplaycamera,5,70000 );
	SetCameraAspect (  t.terrain.gameplaycamera,1.325f );
	SetCameraFOV (  t.terrain.gameplaycamera,75 );
}

//Dave Performance - setup character entities for shader switching
void game_setup_character_shader_entities ( bool bMode )
{
	//store the ID's of entity and character shaders
	t.entityBasicShaderID=loadinternaleffectunique("effectbank\\reloaded\\character_static.fx", 1); //PE: old effect never deleted. why ?
	//t.entityBasicShaderID = loadinternaleffect("effectbank\\reloaded\\character_static.fx"); //PE: Need to test this more. why would it need to be unique ?.
	t.characterBasicShaderID=loadinternaleffect("effectbank\\reloaded\\character_basic.fx");


	//PE: Bug. reset effect clip , so visible.
	t.tnothing = MakeVector4(g.characterkitvector);
	SetVector4(g.characterkitvector, 500000, 1, 0, 0);
	SetEffectConstantV(t.entityBasicShaderID, "EntityEffectControl", g.characterkitvector);
	SetEffectConstantV(t.characterBasicShaderID, "EntityEffectControl", g.characterkitvector);
	t.tnothing = DeleteVector4(g.characterkitvector);


	t.characterBasicEntityList.clear();
	t.characterBasicEntityListIsSetToCharacter.clear();

	// build up a list of entities that use the character shader
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		t.entobj = g.entitybankoffset + t.entid;
		if ( t.entid > 0 )
		{
			// Dont add CPUANIMS=1 characters
			if ( t.entityprofile[t.entid].cpuanims==0 )
			{
				// Dont add cc characters to this

				//PE: need apbr_basic.fx , apbr_anim.fx
				//PE: pbr restored later so...
				if (strcmp(Lower(Right(t.entityprofile[t.entid].effect_s.Get(), 13)), "apbr_basic.fx") == 0 && t.entityprofile[t.entid].ischaracter == 1 && t.entityprofile[t.entid].ischaractercreator == 0)
				{
					t.characterBasicEntityList.push_back(t.e);
					if (t.entityelement[t.e].active == 1)
						t.characterBasicEntityListIsSetToCharacter.push_back(true);
					else
						t.characterBasicEntityListIsSetToCharacter.push_back(false);

					// set the bank object to freeze also for when they switch to instances
					if (bMode)
						SetObjectEffect(g.entitybankoffset + t.entityelement[t.e].bankindex, t.entityBasicShaderID);
					else
						SetObjectEffect(g.entitybankoffset + t.entityelement[t.e].bankindex, t.characterBasicShaderID);
					SetObjectEffect(t.entityelement[t.e].obj, t.characterBasicShaderID);
				}
				if ( strcmp ( Lower(Right(t.entityprofile[t.entid].effect_s.Get(),18)) , "character_basic.fx" ) == 0 && t.entityprofile[t.entid].ischaracter == 1 && t.entityprofile[t.entid].ischaractercreator == 0 )
				{
					t.characterBasicEntityList.push_back(t.e);
					if ( t.entityelement[t.e].active == 1 )
						t.characterBasicEntityListIsSetToCharacter.push_back(true);
					else
						t.characterBasicEntityListIsSetToCharacter.push_back(false);
					// set the bank object to freeze also for when they switch to instances
					if ( bMode )
						SetObjectEffect( g.entitybankoffset+t.entityelement[t.e].bankindex , t.entityBasicShaderID );
					else
						SetObjectEffect( g.entitybankoffset+t.entityelement[t.e].bankindex , t.characterBasicShaderID );
					SetObjectEffect( t.entityelement[t.e].obj , t.characterBasicShaderID );
				}
			}
		}
	}
}

//Dave Performance
void game_check_character_shader_entities ( void )
{
	if ( !t.haveSetupShaderSwitching )
	{
		t.haveSetupShaderSwitching = true;
		game_setup_character_shader_entities(true);
	}

	int tobj;
	float dx;
	float dy;
	float dz;
	float dist;

	if ( g_occluderf9Mode )
	{
		for ( int c = 0 ; c < (int)t.characterBasicEntityList.size() ; c++ )
		{		
			tobj = t.entityelement[t.characterBasicEntityList[c]].obj;
			if ( tobj > 0 )
			{
				if ( ObjectExist ( tobj ) == 1 )
				{

					SetObjectEffect( tobj , t.characterBasicShaderID );
					t.characterBasicEntityListIsSetToCharacter[c] = true;							

				}
			}
		}

		return;
	}

	for ( int c = 0 ; c < (int)t.characterBasicEntityList.size() ; c++ )
	{		
		tobj = t.entityelement[t.characterBasicEntityList[c]].obj;
		if ( tobj > 0 && t.entityelement[t.characterBasicEntityList[c]].active == 1 )
		{
			if ( ObjectExist ( tobj ) == 1 )
			{
				dx = CameraPositionX() - ObjectPositionX( tobj );
				dy = CameraPositionY() - ObjectPositionY( tobj );
				dz = CameraPositionZ() - ObjectPositionZ( tobj );
				dist = sqrtf ( dx*dx + dy*dy + dz*dz );

				if ( t.entityelement[t.characterBasicEntityList[c]].ragdollified )
					dist = 0;

				// 120516 - ensure character shader respects always active
				t.te = t.characterBasicEntityList[c]; entity_getmaxfreezedistance();
				if ( dist < t.maximumnonefreezedistance )
				{
					if ( !t.characterBasicEntityListIsSetToCharacter[c] )
					{
						SetObjectEffect( tobj , t.characterBasicShaderID );
						t.characterBasicEntityListIsSetToCharacter[c] = true;	
					}
				}
				else
				{
					if ( t.characterBasicEntityListIsSetToCharacter[c] )
					{
						SetObjectEffect( tobj , t.entityBasicShaderID );
						t.characterBasicEntityListIsSetToCharacter[c] = false;		
					}
				}
			}
		}
	}
}

extern int howManyMarkers;

void game_preparelevel_finally ( void )
{
	// Tell shadow maps to restore previous vis list rather than sort it
	g_bInEditor = false;
	t.performanceCameraDrawDistance = 0;
	t.haveSetupShaderSwitching = false;

	// This is used to record when we have switched lighting modes so we don't do it constantly
	g.inGameLightingMode = 0;
	if (  t.game.runasmultiplayer == 1 ) 
	{
		steam_load_guns ( );
	}

	//Free up spawns sent to lua
	t.entitiesActivatedForLua.clear();

	// Don't switch off guns!
	g.noPlayerGuns = false;
	g.remembergunid = 0;

	//Enable flash light key
	g.flashLightKeyEnabled = true;

	//  Generate mega texture of terrain paint for VERY LOW shaders
	if (  t.terrain.generatedsupertexture == 0 ) 
	{
		t.screenprompt_s="GENERATING TERRAIN SUPER TEXTURE";
		if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
		timestampactivity(0,t.screenprompt_s.Get());
		terrain_generatesupertexture ( false );
		t.terrain.generatedsupertexture = 1;
	}

	//  Trigger the technique to switch to DISTANT shader is far away
	BT_ForceTerrainTechnique (  0 );

	//  Initiate post process system (or reactivate it)
	t.screenprompt_s="INITIALISING POSTPROCESS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	postprocess_init ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	timestampactivity(0,"postprocessing initialised");

	//  Ensure correct shaders in play
	visuals_shaderlevels_update ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Initialise Construction Kit
	conkit_init ( );

	//  Init physics
	t.screenprompt_s="INITIALISING PHYSICS";
	if (  t.game.gameisexe == 0  )  printscreenprompt(t.screenprompt_s.Get()); else loadingpageprogress(5);
	timestampactivity(0,t.screenprompt_s.Get());
	physics_init ( );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  initialise physics for conkit objects
	//conkit_setupphysics ( );

	//  Activate Occlusion System
	timestampactivity(0,"Activate Occlusion System");
	if (  g.globals.occlusionmode == 1 ) 
	{
		//  Once all assets in place, create occlusion database
		// 110416 - commented out again, turns out when occluder in THREAD is flagged to end, it clears the occluder list
		//CPU3DClear(); // 260316 - dont know why the clear was commented out, it is ESSENTIAL to ensure levels dont mess each other up
		CPU3DSetCameraIndex (  0 );
		//  Occlusion poly list can have a variable size to help performance
		CPU3DSetPolyCount ( t.visuals.occlusionvalue );
		//  Add terrain LOD1s as occluders
		//for ( t.obj = t.terrain.TerrainLODOBJStart ; t.obj<=  t.terrain.TerrainLODOBJFinish; t.obj++ )
		//{
		//	if (  t.obj>0 ) 
		//	{
		//		if ( ObjectExist(t.obj) == 1  )  CPU3DAddOccluder (  t.obj );
		//		if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
		//	}
		//}
		//  Set occludees for all entities in level
		t.toccobj=g.occlusionboxobjectoffset;
		for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
		{
			if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
			t.entid=t.entityelement[t.e].bankindex;
			if (  t.entityprofile[t.entid].ismarker == 0 ) 
			{
				t.obj=t.entityelement[t.e].obj;
				if (  t.obj>0 ) 
				{
					if (  ObjectExist(t.obj) == 1 ) 
					{
						//Dave Performance, if the object is static, make it static for the game
						if ( t.entityelement[t.e].staticflag == 1 )
							SetObjectStatic(t.obj , true );
						else
							SetObjectStatic(t.obj , false );		

						//  reject any objects too small as they won't make good occluders
						if (  t.entityelement[t.e].staticflag == 1 && t.entityprofile[t.entid].notanoccluder == 0 && ((ObjectSizeX(t.obj,1)>MINOCCLUDERSIZE && ObjectSizeY(t.obj,1)>MINOCCLUDERSIZE ) || (ObjectSizeZ(t.obj,1)>MINOCCLUDERSIZE && ObjectSizeY(t.obj,1)>MINOCCLUDERSIZE )) ) 
						{
							//  OCLUDER AND OCLUDEE
							//  OPTIMIZE; we can make this even faster by using a simple math-QUAD in the render part
							//  instead of storing the verts of a 12 polygon box!!
							if ( t.entityelement[t.e].eleprof.isocluder == 1 )
							{
								if (  t.entityprofile[t.entid].physicsobjectcount>0 && t.entityprofile[t.entid].collisionmode == 40 ) 
								{
									t.tocy_f=ObjectSizeY(t.obj)/2.0;
									for ( t.tcount = 0 ; t.tcount<=  t.entityprofile[t.entid].physicsobjectcount-1; t.tcount++ )
									{
										if (  ObjectExist(t.toccobj) == 1  )  DeleteObject (  t.toccobj );
										MakeObjectBox (  t.toccobj, t.entityphysicsbox[t.entid][t.tcount].SizeX * (t.entityprofile[t.entid].scale * 0.01 ), t.entityphysicsbox[t.entid][t.tcount].SizeY * (t.entityprofile[t.entid].scale * 0.01 ), t.entityphysicsbox[t.entid][t.tcount].SizeZ * (t.entityprofile[t.entid].scale * 0.01 ) );
										OffsetLimb (  t.toccobj, 0, t.entityphysicsbox[t.entid][t.tcount].OffX * (t.entityprofile[t.entid].scale * 0.01 ) , t.entityphysicsbox[t.entid][t.tcount].OffY * (t.entityprofile[t.entid].scale * 0.01 ) , t.entityphysicsbox[t.entid][t.tcount].OffZ * (t.entityprofile[t.entid].scale * 0.01 ) );
										RotateLimb (  t.toccobj, 0, t.entityphysicsbox[t.entid][t.tcount].RotX , t.entityphysicsbox[t.entid][t.tcount].RotY , t.entityphysicsbox[t.entid][t.tcount].RotZ );
										MakeMeshFromObject (  g.meshgeneralwork,t.toccobj );
										DeleteObject (  t.toccobj );
										MakeObject (  t.toccobj,g.meshgeneralwork,0 );
										DeleteMesh (  g.meshgeneralwork );
										PositionObject (  t.toccobj,ObjectPositionX(t.obj),ObjectPositionY(t.obj)+t.tocy_f,ObjectPositionZ(t.obj) );
										RotateObject (  t.toccobj,ObjectAngleX(t.obj),ObjectAngleY(t.obj),ObjectAngleZ(t.obj) );
										SetObjectMask (  t.toccobj, 0 );
										SetObjectCollisionProperty (  t.toccobj,1 );
										CPU3DAddOccluder (  t.toccobj );
										++t.toccobj;
									}
								}
								else
								{
									//  polygon occluders TOO EXPENSIVE
									//  NOTE; Suggest a new set of polygons inside each model marked 'occluder'
									//  which when detected are submitted here via the OBJ
									// Dave Performance - adding everything in, even poly stuff
									// Don't add in collisionmode 50-59 (trees) as they make poor occluders
									if ( t.entityprofile[t.entid].collisionmode < 50 || t.entityprofile[t.entid].collisionmode > 59 ) 
										CPU3DAddOccluder (  t.obj );
								}
							}
						}
					}
				}

				// Add as an occludee
				if ( t.entityelement[t.e].eleprof.isocludee == 1 )
				{
					// Also let the occluder know if it is a character or not as characters are shown for longer
					// compared to other objects
					if ( t.entityprofile[t.entid].ischaracter == 1 ) 
					{
						// Also add character creator parts, if this is a cc character
						CPU3DAddOccludee ( t.obj , true );

						/* 100517 - bug fix until figure out why occluder does not restore head bits!
						if ( t.entityprofile[t.entid].ischaractercreator == 1 )
						{
							// Head
							t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
							if (  ObjectExist(t.tccobj)  ==  1 ) CPU3DAddOccludee (  t.tccobj , false ); // 100517 - fix bug true );

							// Beard
							t.tccobjbeard = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset)+1;
							if (  ObjectExist(t.tccobjbeard)  ==  1 ) CPU3DAddOccludee (  t.tccobjbeard , false ); // 100517 - fix bug true );

							// Hat
							t.tccobjhat = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset)+2;
							if (  ObjectExist(t.tccobjhat)  ==  1 ) CPU3DAddOccludee (  t.tccobjhat , false ); // 100517 - fix bug true );
						}
						*/
					}
					else
					{
						CPU3DAddOccludee ( t.obj , false );
					}
				}
			}
			else
				howManyMarkers++;
		}
		while (  t.toccobj<g.occlusionboxobjectoffsetfinish ) 
		{
			if (  ObjectExist(t.toccobj) == 1  )  DeleteObject (  t.toccobj );
			++t.toccobj;
		}

		//  also occlude any weapons carried by characters
		if (  g.entityattachmentindex>0 ) 
		{
			for ( t.obj = g.entityattachmentsoffset+1 ; t.obj<=  g.entityattachmentsoffset+g.entityattachmentindex; t.obj++ )
			{
				if (  t.obj>0 ) 
				{
					if (  ObjectExist(t.obj) == 1 ) 
					{
						CPU3DAddOccludee (  t.obj , false );
					}
				}
			}
		}
	}

	//  Final states of entities and call ALL entity script INIT functions
	timestampactivity(0,"Entity Initiations");
	entity_initafterphysics ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	lua_launchallinitscripts ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Once player start known, fill veg area instantly
	timestampactivity(0,"Fill Veg Areas");
	t.completelyfillvegarea=1;
	terrain_fastveg_loop ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Force a shader update to ensure correct shadows are used at start
	t.visuals.refreshcountdown=5;

	//  Let steam know we have finished loading
	if (  t.game.runasmultiplayer  ==  1 ) 
	{
		g.steamworks.finishedLoadingMap = 1;
	}

	// The start marker may have given the play an initial gun, so lets call physics_player_refreshcount just incase it has
	physics_player_refreshcount();
}

void game_stopallsounds ( int iMode )
{
	// iMode : 0-end of level, 1-game menu sounds
	// stop ALL sounds
	for ( t.s = 1 ; t.s<=  65535; t.s++ )
	{
		if (  SoundExist(t.s) == 1 ) 
		{
			if (  t.s >= g.musicsoundoffset && t.s <= g.musicsoundoffsetend ) 
			{
				// if from game menu, do not stop dynamic music sound progress
				if ( iMode == 0 ) StopSound (  t.s );
			}
			else
			{
				StopSound (  t.s );
			}
		}
	}
}

void game_freelevel ( void )
{
	//  hide any jetpacks, etc
	hud_free ( );

	//  stop ALL sounds
	game_stopallsounds ( 0 );

	//  deactivate occlusion
	// 110416 - cannot just clear CPU3D occluder lists, its INSIDE a THREAD and STILL RUNNING even now!
	//if (  g.globals.occlusionmode == 1 ) 
	//{
	//	// Ensure all universe visibility flags are restored!
	//	// 250316 - commented these back in, again, need to clear occlusion effects
	//	CPU3DClear(); 
	//	CPU3DOcclude();
	//}

	// Delete occluder thread and close event handles (moved from free game which is too late for multi-level games)
	if ( g_pOccluderThread )
	{
		//Let the occluder know its time to finish
		g_occluderOn = false;

		//Dave Performance - let the occluder thread know it is okay to begin
		if ( g_hOccluderBegin ) SetEvent ( g_hOccluderBegin );

		//Dave Performance - wait for occluder to finish first
		if ( g_hOccluderEnd ) WaitForSingleObject ( g_hOccluderEnd, INFINITE );
	}
	
	//  free any character AI related stuff
	darkai_release_characters ( );
	darkai_destroy_all_characterdata ( );

	// remove bits created by LUA scripts
	lua_freeprompt3d();

	//  close down game entities
	entity_free ( );

	//  Delete any infinilights
	lighting_free ( );

	//  AI finish
	darkai_free ( );

	//  free physics
	physics_free ( );

	//  deselect current gun and hide all gun objects
	gun_free ( );
	gun_freeguns ( );
	gun_removempgunsfromlist ( );

	//  remove water and sky effects
	terrain_sky_free ( );
	terrain_water_free ( );

	//  restore terrain from in-game
	terrain_stop_play ( );

	//  free vegetation
	terrain_fastveg_free ( );

	//  free Construction Kit
	conkit_free ( );

	//  free any visual leftovers
	visuals_free ( );

	//  free character sound
	character_sound_free ( );

	//  close script system
	lua_free ( );

	//  free projectiles
	weapon_projectile_free ( );

	//  free particles
	ravey_particles_free ( );

	// finally delete entity element objs (only if standalone)
	if ( t.game.gameisexe == 1 )
	{
		// only for standalone as test game needs entities for editor :)
		entity_delete ( );
	}
}

void game_init ( void )
{
	//  Machine independent speed
	game_timeelapsed_init ( );

	//  Load slider menu resources
	sliders_init ( );

	//  Trigger all visuals to update
	t.visuals.refreshshaders=1;

	//  HideMouse (  and clear deltas )
	game_hidemouse ( );

	//  Last thing before main game loop
	physics_beginsimulation ( );

	//  Just before start, stagger AI processing timers
	darkai_staggerAIprocessing ( );

	//  Reset game checkpoint
	t.playercheckpoint.stored=1;
	t.playercheckpoint.x=CameraPositionX(0);
	t.playercheckpoint.y=CameraPositionY(0);
	t.playercheckpoint.z=CameraPositionZ(0);
	t.playercheckpoint.a=CameraAngleY(0);
	t.playercheckpoint.soundloopcheckpointcountdown=20;

	//  Reset hardware flags with each new level map
	t.hardwareinfoglobals.noterrain=0;
	t.hardwareinfoglobals.nowater=0;
	t.hardwareinfoglobals.noguns=0;
	t.hardwareinfoglobals.nolmos=0;
	t.hardwareinfoglobals.nosky=0;
	t.hardwareinfoglobals.nophysics=0;
	t.hardwareinfoglobals.noai=0;
	t.hardwareinfoglobals.nograss=0;
	t.hardwareinfoglobals.noentities=0;

	//  initialise panel resources
	panel_init ( );

	//  construction kit f9 mode cursor
	t.characterkitcontrol.oldF9CursorEntid = 0;
}

void game_freegame ( void )
{
	// Ensure we are switched back to full res
	t.bHiResMode = true;
	t.bOldHiResMode = true;
	SetCameraHiRes ( t.bHiResMode );

	//Free up spawns sent to lua
	t.entitiesActivatedForLua.clear();

	// Ensure Steam chat sprite has gone
	if (  SpriteExist(g.steamchatpanelsprite)  )  DeleteSprite (  g.steamchatpanelsprite );

	// Restore fonts
	loadallfonts();

	//Switch back to using an additional sort visibility list in the editor to avoid flickering when adding new entities
	g_bInEditor = true;
	//  Free file map access game uses
	physics_player_free ( );

	//  Free slider resources, not needed for title
	sliders_free ( );

	// Free LUA Sprites and Images
	FreeLUASpritesAndImages ();

	//  Free resources not specific to a single level before returning to title page
	postprocess_free ( );

	if (  t.game.runasmultiplayer == 1 ) 
	{
		steam_free_game ( );
	}

	panel_free ( );

	//Dave Performance - switch entities and profile object back to animating
	for ( int c = 0 ; c < (int)t.characterBasicEntityList.size() ; c++ )
	{		
		int tobj = t.entityelement[t.characterBasicEntityList[c]].obj;
		if ( tobj > 0 )
		{
			if ( ObjectExist ( tobj ) == 1 )
			{
				if ( !t.characterBasicEntityListIsSetToCharacter[c] )
				{
					SetObjectEffect( tobj , t.characterBasicShaderID );
					SetObjectEffect( g.entitybankoffset+t.entityelement[t.characterBasicEntityList[c]].bankindex , t.characterBasicShaderID );				
				}
			}
		}
	}

	t.characterBasicEntityList.clear();
	t.characterBasicEntityListIsSetToCharacter.clear();

	// switch all objects back to dynamic before heading back to the editor
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entityprofile[t.entid].ismarker == 0 ) 
		{
			t.obj=t.entityelement[t.e].obj;

			if (  t.obj>0 ) 
			{
				SetObjectStatic(t.obj , false );
			}
		}
	}

	//Dave Performance - restore any ignored objects and get the texture list sortid again to include them
	ClearIgnoredObjects ( );
	DoTextureListSort ( );
}

void game_hidemouse ( void )
{
	if (  g.mouseishidden == 0 ) 
	{
		g.mouseishidden=1;
		if (  t.game.gameisexe == 1 ) 
		{
			t.tgamemousex_f=MouseX();
			t.tgamemousey_f=MouseY();
			HideMouse (  );
		}
		else
		{
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetEventAndWait (  1 );
			SetFileMapDWORD (  1,974,1 );
			SetEventAndWait (  1 );
			t.tgamemousex_f=GetFileMapDWORD( 1, 0 );
			t.tgamemousey_f=GetFileMapDWORD( 1, 4 );
			t.tgamemousex_f=t.tgamemousex_f/800.0;
			t.tgamemousey_f=t.tgamemousey_f/600.0;
			t.tgamemousex_f=t.tgamemousex_f*(GetDisplayWidth()+0.0);
			t.tgamemousey_f=t.tgamemousey_f*(GetDisplayHeight()+0.0);
		}
		t.null=MouseMoveX()+MouseMoveY();
	}
}

void game_showmouse ( void )
{
	if (  g.mouseishidden == 1 ) 
	{
		g.mouseishidden=0;
		if (  t.game.gameisexe == 1 ) 
		{
			ShowMouse (  );
		}
		else
		{
			OpenFileMap (  1, "FPSEXCHANGE" );
			SetEventAndWait (  1 );
			t.tgamemousex_f=t.inputsys.xmouse+0.0;
			t.tgamemousey_f=t.inputsys.ymouse+0.0;
			SetFileMapDWORD (  1,982,t.tgamemousex_f );
			SetFileMapDWORD (  1,986,t.tgamemousey_f );
			SetFileMapDWORD (  1,974,2 );
			SetEventAndWait (  1 );
		}
		t.null=MouseMoveX()+MouseMoveY();
	}
}

void game_timeelapsed_init ( void )
{
	//  Machine indie speed
	t.TimerFrequency_f=PerformanceFrequency();
	t.StartFrameTime=PerformanceTimer();
	t.ElapsedTime_f=0.0;
	t.LastTimeStamp_f=timeGetSecond();
	g.timeelapsed_f=0;
}

void game_timeelapsed ( void )
{
	// Calculate time between cycles
	float fThisTimeCount = timeGetSecond();
	t.ElapsedTime_f = fThisTimeCount - t.LastTimeStamp_f;
	g.timeelapsed_f = t.ElapsedTime_f * 20.0;
	t.LastTimeStamp_f = fThisTimeCount;

	//  Cap to around 25fps so that leaps in movement/speed not to severe!
	if (  g.timeelapsed_f>0.75f  )  g.timeelapsed_f = 0.75f;
	if (  g.timeelapsed_f<0.00833f  )  g.timeelapsed_f = 0.00833f;
}

void game_main_snapshotsoundloopcheckpoint ( void )
{
	//  remember any looping sounds but exclude weapon and rocket sounds
	if (  t.playercontrol.disablemusicreset == 0 ) 
	{
		for ( t.s = g.soundbankoffset ; t.s<=  g.soundbankoffsetfinish; t.s++ )
		{
			if (  t.soundloopcheckpoint[t.s] != 2 ) 
			{
				t.soundloopcheckpoint[t.s]=0;
				if (  SoundExist(t.s) == 1 ) 
				{
					if (  SoundLooping(t.s) == 1 ) 
					{
						t.soundloopcheckpoint[t.s]=1;
					}
				}
			}
		}
	}
return;

}

void game_main_snapshotsoundloopresume ( void )
{
	if (  t.playercontrol.disablemusicreset == 0 ) 
	{
		for ( t.s = g.soundbankoffset ; t.s<=  g.soundbankoffsetfinish; t.s++ )
		{
			if (  t.soundloopcheckpoint[t.s] != 2 ) 
			{
				if (  t.soundloopcheckpoint[t.s] == 1 ) 
				{
					t.soundloopcheckpoint[t.s]=0;
					if (  SoundExist(t.s) == 1 ) 
					{
						LoopSound (  t.s );
					}
				}
			}
		}
	}
return;

}

extern bool g_bEarlyExcludeMode;
int oldSpaceKey = 0;

extern int NumberOfObjects;
extern int NumberOfGroupsShown;
extern int NumberOfObjectsShown;

void game_main_loop ( void )
{	
	//  Timer (  based movement )
	game_timeelapsed ( );

	//  Music processing
	music_loop ( );

	//  Character sound update
	//  110315 - 019 - If spawning in, no sound for the player
	if (  t.game.runasmultiplayer  ==  0 || g.steamworks.noplayermovement  ==  0 ) 
	{
		character_sound_update ( );
	}

	//  Trigger soundloops to be snapshot (when start level and at checkpoints)
	if (  t.playercheckpoint.soundloopcheckpointcountdown>0 ) 
	{
		--t.playercheckpoint.soundloopcheckpointcountdown;
		if (  t.playercheckpoint.soundloopcheckpointcountdown == 0 ) 
		{
			game_main_snapshotsoundloopcheckpoint ( );
		}
	}

	//  Force a shader update to ensure correct shadows are used at start
	if (  t.visuals.refreshcountdown>0 ) 
	{
		--t.visuals.refreshcountdown;
		if (  t.visuals.refreshcountdown == 0 ) 
		{
			visuals_shaderlevels_lighting_update ( );
			visuals_shaderlevels_update ( );
			t.visuals.refreshshaders=1;
		}
	}

	// Testgame or Standalone
	// 250316 - when level ends, suspend all logic (including more calls to JumpTolevel or in-game last minute AI stuff)
	if ( t.game.levelendingcycle == 0 )
	{
		if ( (t.game.gameisexe == 0 || g.gprofileinstandalone == 1) && t.game.runasmultiplayer == 0 ) 
		{
			// Test Game Mode
			// Handle light-mapper key
			if (  g.globals.ideinputmode == 1 ) 
			{
				g.lmlightmapnowmode=0;
				if (  KeyState(g.keymap[59]) == 1  )  g.lmlightmapnowmode = 1;
				if (  KeyState(g.keymap[60]) == 1  )  g.lmlightmapnowmode = 2;
				if (  KeyState(g.keymap[61]) == 1  )  g.lmlightmapnowmode = 3;
				if (  KeyState(g.keymap[62]) == 1  )  g.lmlightmapnowmode = 4;
				if (  g.lmlightmapnowmode>0 ) 
				{
					//  User prompt
					t.strwork = ""; t.strwork = t.strwork + "Select Lightmapping Mode "+Str(g.lmlightmapnowmode);
					timestampactivity(0, t.strwork.Get() ) ; t.twas=Timer();
					t.tdisableLMprogressreading=1;
					for ( t.n = 0 ; t.n<=  1; t.n++ )
					{
						t.tonscreenprompt_s="Saving Level Session";
						if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
						lm_onscreenprompt ( ) ; Sync ( );
					}
					t.tdisableLMprogressreading=0;
					//  save level (temp)
					t.gamevisuals=t.visuals;
					g.gpretestsavemode=1;
					gridedit_save_test_map ( );
					g.gpretestsavemode=0;
					t.visuals=t.gamevisuals;
					//  first WIPE OUT old lightmap files
					lm_emptylightmapfolder ( );
					//  second WIPE OUT old LM objects to ensure FRESH bake
					for ( t.tlmobj = g.lightmappedobjectoffset; t.tlmobj<= g.lightmappedobjectoffsetlast; t.tlmobj++ )
					{
						if (  ObjectExist(t.tlmobj) == 1 ) 
						{
							DeleteObject (  t.tlmobj );
						}
					}
					g.lightmappedobjectoffsetfinish=92000;
					g.lightmappedterrainoffset=-1;
					g.lightmappedterrainoffsetfinish=-1;
					//  launch external lightmapper
					SetDir (  ".." );
					timestampactivity(0,"launch external lightmapper") ; t.twas=Timer();
					t.tdisableLMprogressreading=1;
					for ( t.n = 0 ; t.n<=  1; t.n++ )
					{
						t.tonscreenprompt_s="Lightmapping in Progress";
						if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
						lm_onscreenprompt ( ) ; Sync (  );
					}
					t.tdisableLMprogressreading=0;
					t.strwork = ""; t.strwork = t.strwork + "-"+Str(g.lmlightmapnowmode);
					ExecuteFile (  "Guru-Lightmapper.exe", t.strwork.Get() ,"",1 );
					t.strwork = ""; t.strwork = t.strwork + "returned from t.lightmapper - baked in "+Str((Timer()-t.twas)/1000)+" seconds";
					timestampactivity(0, t.strwork.Get() );
					SetDir (  "Files" );
					//  Wait for all input to cease
					t.tdisableLMprogressreading=1;
					while (  ScanCode() != 0 || MouseClick() != 0 ) 
					{
						for ( t.n = 0 ; t.n<=  1; t.n++ )
						{
							t.tonscreenprompt_s="Returning from lightmapper";
							if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
							lm_onscreenprompt ( ) ; Sync ( ) ;
						}
					}
					//  User prompt
					for ( t.n = 0 ; t.n<=  1; t.n++ )
					{
						t.tonscreenprompt_s="Loading Lightmaps";
						if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
						lm_onscreenprompt ( ) ; Sync ( ) ;
					}
					t.tdisableLMprogressreading=0;
					//  load new lightmap scene
					t.lightmapper.onlyloadstaticentitiesduringlightmapper=1;
					lm_loadscene ( );
					t.lightmapper.onlyloadstaticentitiesduringlightmapper=0;
					if (  t.tlmloadsuccess == 1 ) 
					{
						//  Switch to pre-bake mode
						t.visuals.shaderlevels.lighting=1;
						t.gamevisuals.shaderlevels.lighting=1;
						t.slidersmenuindex=t.slidersmenunames.shaderoptions;
						t.slidersmenuvalue[t.slidersmenuindex][4].value=1;
						t.slidersmenuvaluechoice=t.slidersmenuvalue[t.slidersmenuindex][4].gadgettypevalue;
						t.slidersmenuvalueindex=t.slidersmenuvalue[t.slidersmenuindex][4].value;
						sliders_getnamefromvalue ( );
						t.slidersmenuvalue[t.slidersmenuindex][4].value_s=t.slidervaluename_s;
						visuals_shaderlevels_update ( );
					}
					//  restore water mask which may be affected
					terrain_whitewashwatermask ( );
					//  refresh shaders in any event
					t.visuals.refreshshaders=1;
				}
			}

			//  Tab Mode (only when not mid-fpswarning)
			if (  t.plrkeySHIFT == 0 && t.plrkeySHIFT2 == 0  )  t.tkeystate15 = KeyState(g.keymap[15]); else t.tkeystate15 = 0;
			if (  t.game.runasmultiplayer  ==  1  )  g.tabmode  =  0;
			if (  t.conkit.editmodeactive == 1  )  g.tabmode = 0;
			if (  g.lowfpswarning == 0 || g.lowfpswarning == 3 ) 
			{
				if (  t.tkeystate15 == 0  )  t.tabpress = 0;
				if (  g.globals.riftmode>0 ) 
				{
					if (  t.tkeystate15 == 1 && t.tabpress == 0 ) 
					{
						// 101115 - reset hardware menu if press TAB
						g.tabmodeshowfps = 0;

						if (  g.tabmode == 0 ) 
						{
							game_showmouse ( );
							g.tabmode=2;
						}
						else
						{
							game_hidemouse ( );
							g.tabmode=0;
						}
						t.tabpress=1;
					}
				}
				else
				{
					if (  t.tkeystate15 == 1 && t.tabpress == 0 ) 
					{
						// 101115 - reset hardware menu if press TAB
						g.tabmodeshowfps = 0;

						g.tabmode=g.tabmode+1;
						if (  g.tabmode>2 ) 
						{
							g.tabmode=0;
						}
						if (  g.tabmode<2 ) 
						{
							game_hidemouse ( );
						}
						if (  g.tabmode == 2 ) 
						{
							game_showmouse ( );
						}
						t.tabpress=1;
					}
				}
			}
		}
		else
		{
			//  Standalone Mode
		}

		//  Measure Sync (  to loop start )
		t.game.perf.resttosync += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

		//  Control slider menus (based on tab page)
		sliders_loop ( );

		//  Lighting control
		//  Entity control
		//  Particles control
		//  Flak control

		//  update all projectiles
		weapon_projectile_loop ( );

		//  update all particles and emitters
		ravey_particles_update ( );

		//  Decal control
		decalelement_control ( );

		//  Prompt
		if (  t.sky.currenthour_f<1.0 || t.sky.currenthour_f >= 13.0 ) 
		{
			t.pm=int(t.sky.currenthour_f);
			if (  t.pm == 0  )  t.pm = 12; else t.pm = t.pm-12;
			t.pm_s = ""; t.pm_s = t.pm_s + Str(t.pm)+"PM";
		}
		else
		{
			t.pm_s = "";t.pm_s = t.pm_s + Str(int(t.sky.currenthour_f))+"AM";
		}
		t.promptextra_s = ""; t.promptextra_s=t.promptextra_s + "FPS:"+Str(GetDisplayFPS())+" TIME:"+t.pm_s;
		t.game.perf.misc += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

		//  any terrain actions
		if (  t.hardwareinfoglobals.noterrain == 0 ) 
		{
			terrain_loop ( );
		}

		//  loop physics
		if (  t.hardwareinfoglobals.nophysics == 0 ) 
		{
			// Handle physics
			physics_loop ( );

			// read all slider values for player
			t.slidersmenuindex=t.slidersmenunames.player; sliders_readall ( );

			//  Do weapon attachments AFTER physics moved objects (and if char killed off)
			for ( g.charanimindex = 1 ; g.charanimindex <= g.charanimindexmax; g.charanimindex++ )
			{
				// update gun position in hand of character
				t.e = t.charanimstates[g.charanimindex].e;
				if ( t.e > 0 ) entity_controlattachments ( );

				// detect collection of dropped guns
				t.e = t.charanimstates[g.charanimindex].originale;
				if ( t.e > 0 ) entity_monitorattachments ( );
			}

			//  But do allow third person protagonist attachment control
			if (  t.playercontrol.thirdperson.enabled == 1 ) 
			{
				t.e=t.playercontrol.thirdperson.charactere;
				if (  t.e>0 && t.player[1].health>0 ) 
				{
					entity_controlattachments ( );
				}
			}

			//  Construction Kit control
			conkit_loop ( );
		}
		t.game.perf.physics += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

		// In-Game Mode (moved from above so LUA is AFTER physics)
		if ( t.conkit.editmodeactive == 0 ) 
		{
			// if third person, trick AI by moving camera to protagonist location
			if ( t.playercontrol.thirdperson.enabled == 1 ) 
			{
				t.playercontrol.thirdperson.storecamposx = CameraPositionX(t.terrain.gameplaycamera);
				t.playercontrol.thirdperson.storecamposy = CameraPositionY(t.terrain.gameplaycamera);
				t.playercontrol.thirdperson.storecamposz = CameraPositionZ(t.terrain.gameplaycamera);
				t.tobj = t.aisystem.objectstartindex;
				if ( g.luacameraoverride != 1 && g.luacameraoverride != 3 )
				{
					PositionCamera ( t.terrain.gameplaycamera, ObjectPositionX(t.tobj), ObjectPositionY(t.tobj), ObjectPositionZ(t.tobj) );
				}
			}

			//  All Entity logic
			t.ttempoverallaiperftimerstamp=PerformanceTimer();
			if (  t.hardwareinfoglobals.noai == 0 ) 
			{
				// LUA and Entity Logic
				lua_loop ( );
				t.game.perf.ai1 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
				entity_loop ( );
				entity_loopanim ( );
				t.game.perf.ai2 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

				// Update all AI and Characters and VWeaps
				if (  t.aisystem.processlogic == 1 ) 
				{
					if (  t.visuals.debugvisualsmode<100 ) 
					{
						darkai_loop ( );
						t.game.perf.ai3 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
						darkai_update ( );
						t.game.perf.ai4 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
						if (  t.visuals.debugvisualsmode<98 ) 
						{
							darkai_character_loop ( );
						}
						t.game.perf.ai5 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
						t.game.perf.ai6 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
					}
				}
			}
			t.game.perf.ai += PerformanceTimer()-t.ttempoverallaiperftimerstamp;

			// don't do this when in f9 mode
			if ( !g_occluderf9Mode) game_check_character_shader_entities ( );
		}

		// if third person, restore camera from protag-cam trick
		if ( t.playercontrol.thirdperson.enabled == 1 ) 
		{
			if ( g.luacameraoverride != 1 && g.luacameraoverride != 3 )
			{
				PositionCamera (  t.terrain.gameplaycamera,t.playercontrol.thirdperson.storecamposx,t.playercontrol.thirdperson.storecamposy,t.playercontrol.thirdperson.storecamposz );
			}
		}

		//  Gun control
		if ( t.hardwareinfoglobals.noguns == 0 ) 
		{
			gun_manager ( );
			t.slidersmenuindex=t.slidersmenunames.weapon ; sliders_readall ( );
		}
		t.game.perf.gun += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
	}

	//  Steam call moved here as camera changes need to be BEFORE the shadow update
	if (  t.game.runasmultiplayer == 1 ) 
	{
		steam_gameLoop ( );
	}

	//  if we have character creator stuff in, we setup the characters
	if ( t.characterkitcontrol.gameHasCharacterCreatorIn == 1 ) characterkit_updateCharacters ( );

	// Handle veg engine, terrain shadow, sky and water
	// Dave Performance, calling update on veg and terrain shadow every 4 frames rather than every frame
	static int terrainshadowdelay = 0;

	// If the camera is spun round quick, redraw shadows immediately
	// 281116 - int tMouseMove = MouseMoveX(); - yep, this killed fluid mousemoveX, thanks for that Lee!
	int tMouseMove = t.cammousemovex_f;
	if ( tMouseMove > 20 || tMouseMove < -20 )
	{
		terrainshadowdelay = 4;		
	}

	if (  t.hardwareinfoglobals.noterrain == 0 ) 
	{
		//Dave Performance, calling update on veg and terrain shadow every 4 frames rather than every frame
		//Grass every other frame
		//Gets me 10fps increase on my machine		
		static bool terrainvegdelay = true;
		if ( terrainvegdelay = !terrainvegdelay )
		{
			terrain_fastveg_loop ( );
			t.game.perf.terrain1 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
		}
		else
			t.game.perf.terrain1 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

		// 111115 - had to add t.visuals.shaderlevels.entities==1 so HIGHEST entities allow self shadow characters and detailed entities which flicker when shadow update is delayed!
		// but also need constant updates for third person (as can see delay!)
		if ( ++terrainshadowdelay >= 3 || t.visuals.shaderlevels.entities==1 || t.playercontrol.thirdperson.enabled != 0 )
		{
			terrainshadowdelay = 0;
			terrain_shadowupdate ( );
			t.game.perf.terrain2 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
		}
		else
		{
			t.game.perf.terrain2 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();
		}
	}
	else // fix to stop shadows going wonky when terrain is off
	{
		if ( ++terrainshadowdelay >= 3 )
		{
			terrainshadowdelay = 0;
			terrain_shadowupdate ( );
		}
	}
	if (  t.hardwareinfoglobals.nosky == 0 ) 
	{
		terrain_sky_loop ( );
	}
	if (  t.hardwareinfoglobals.nowater == 0 ) 
	{
		terrain_water_loop ( );
	}
	t.game.perf.terrain3 += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

	//  Game Debug Prompts
	if (  t.aisystem.showprompts == 1 ) 
	{
		pastebitmapfont("DEBUG PROMPTS",8,20,1,255) ; t.i=1;
		t.strwork = ""; t.strwork = t.strwork + "NUMBER OF CHARACTERS:"+Str(g.charanimindexmax);
		pastebitmapfont( t.strwork.Get() ,8,20+(t.i*25),1,255) ; ++t.i;
		for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
		{
			t.tdesc_s = ""; t.tdesc_s = t.tdesc_s + "CHAR:"+Str(g.charanimindex);
			t.tdesc_s=t.tdesc_s+"  AIstate_s="+AIGetEntityState(t.charanimstates[g.charanimindex].obj);
			t.tdesc_s=t.tdesc_s+"  t.moving="+Str(t.charanimcontrols[g.charanimindex].moving);
			t.tdesc_s=t.tdesc_s+"  t.ducking="+Str(t.charanimcontrols[g.charanimindex].ducking);
			pastebitmapfont(t.tdesc_s.Get(),8,20+(t.i*25),1,255) ; ++t.i;
			t.tdesc_s="";
			t.tdesc_s=t.tdesc_s+"  plrvisible="+Str(t.entityelement[t.charanimstates[g.charanimindex].e].plrvisible);
			t.tdesc_s=t.tdesc_s+"  health="+Str(t.entityelement[t.charanimstates[g.charanimindex].e].health);
			t.tdesc_s=t.tdesc_s+"  playcsi="+Str(t.charanimstates[g.charanimindex].playcsi);
			t.tdesc_s=t.tdesc_s+"  t.charseq.mode="+Str(t.charseq[t.charanimstates[g.charanimindex].playcsi].mode);
			pastebitmapfont(t.tdesc_s.Get(),8,20+(t.i*25),1,255) ; ++t.i;
			++t.i;
		}
	}

	// Handle occlusion if active
	if ( g.globals.occlusionmode == 1 ) 
	{
		// detect velocity of XZ motion of player and advance 'virtual camera' ahead of real camera
		// in order to give occluder time to reveal visible objects in advance of getting there
		float plrx = CameraPositionX(0);
		float plrz = CameraPositionZ(0);
		g_fOccluderCamVelX = plrx - g_fOccluderLastCamX;
		g_fOccluderCamVelZ = plrz - g_fOccluderLastCamZ;
		g_fOccluderLastCamX = plrx;
		g_fOccluderLastCamZ = plrz;
		if ( fabs(g_fOccluderCamVelX)>0.01f || fabs(g_fOccluderCamVelZ)>0.01f )
		{
			float fDDMultiplier = 20.0f / sqrt(fabs(g_fOccluderCamVelX*g_fOccluderCamVelX)+fabs(g_fOccluderCamVelZ*g_fOccluderCamVelZ));
			g_fOccluderCamVelX *= fDDMultiplier;
			g_fOccluderCamVelZ *= fDDMultiplier;
			if ( g_fOccluderCamVelX < -20.0f ) g_fOccluderCamVelX = -20.0f;
			if ( g_fOccluderCamVelZ < -20.0f ) g_fOccluderCamVelZ = -20.0f;
			if ( g_fOccluderCamVelX > 20.0f ) g_fOccluderCamVelX = 20.0f;
			if ( g_fOccluderCamVelZ > 20.0f ) g_fOccluderCamVelZ = 20.0f;
		}
		else
		{
			g_fOccluderCamVelX = 0.0f;
			g_fOccluderCamVelZ = 0.0f;
		}
		// show me this
		CPUShiftXZ ( g_fOccluderCamVelX, g_fOccluderCamVelZ );

		CPU3DSetCameraFar ( t.visuals.CameraFAR_f );
		if ( g_pOccluderThread == NULL )
		{
			CPU3DOcclude (  );
			g_hOccluderBegin = CreateEvent ( NULL, FALSE, FALSE, NULL );
			g_hOccluderEnd   = CreateEvent ( NULL, FALSE, FALSE, NULL );
			g_pOccluderThread = new cOccluderThread;
			g_pOccluderThread->Start ( );
		}
		g_occluderOn = true;
	}
	t.game.perf.occlusion += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

	// Final post processing step

	// Render pre-terrain post process cameras (includes lightray rendering)
	postprocess_preterrain ( );

	// Render terrain if flagged
	if ( t.hardwareinfoglobals.noterrain == 0 ) 
	{
		terrain_update ( );
	}

	//  explosions and fire
	draw_debris();
	draw_particles();

	//  handle fade out for level progression
	if (  t.game.levelendingcycle > 0 ) 
	{
		game_end_of_level_check ( );
	}

	//  Post process and visual settings system
	postprocess_apply ( );
	visuals_loop ( );
	lighting_loop ( );
	t.game.perf.postprocessing += PerformanceTimer()-g.gameperftimestamp ; g.gameperftimestamp=PerformanceTimer();

	// Check for player guns switched off
	if ( g.noPlayerGuns )
	{
		physics_no_gun_zoom ( );
		if ( g.autoloadgun != 0 ) { g.autoloadgun=0 ; gun_change ( ); }
	}

	//  Update HUD Layer objects (jetpack)
	hud_updatehudlayerobjects ( );

	//  Call this at end of game loop to ensure character objects sufficiently overridden
	darkai_finalsettingofcharacterobjects ( );
}

extern int howManyOccluders;
extern int howManyOccludersDrawn;
extern int howManyOccludees;
extern int howManyOccludeesHidden;
extern int howManyMarkers;
extern float trackingSize;

void game_dynamicRes()
{

	// If dynamic res is disabled via setup ini, return out
	if ( t.DisableDynamicRes ) return;

	if ( g_pGlob->dwNumberOfPrimCalls > 500 && GetDisplayFPS() < 60 )
		t.bHiResMode = false;
	else if ( g_pGlob->dwNumberOfPrimCalls < 200 && GetDisplayFPS() > 60 )
		t.bHiResMode = true;

	if ( t.visuals.debugvisualsmode == 20 ) t.bHiResMode = false;
	if ( t.visuals.debugvisualsmode == 21 ) t.bHiResMode = true;

	if ( t.bHiResMode != t.bOldHiResMode )
	{
		SetCameraHiRes ( t.bHiResMode );
		t.bOldHiResMode = t.bHiResMode;
	}
}

extern float smallDistanceMulti;

void game_sync ( void )
{
	// Some test stuff, can be removed
	/*SetCursor ( 0 , 0 );
	Print ( t.guncollectedcount );
	Print ( smallDistanceMulti );
	Print ( cstr(cstr("howManyOccluders = ") + cstr(howManyOccluders)).Get() );
	Print ( cstr(cstr("howManyOccludersDrawn = ") + cstr(howManyOccludersDrawn)).Get() );
	Print ( cstr(cstr("howManyOccludees = ") + cstr(howManyOccludees)).Get() );
	Print ( cstr(cstr("howManyOccludeesHidden = ") + cstr(howManyOccludeesHidden)).Get() );
	Print ( cstr(cstr("Entity Count = ") + cstr(g.entityelementlist)).Get() );
	Print ( cstr(cstr("Entity Draw vs hidden = ") + cstr((g.entityelementlist - howManyMarkers ) - howManyOccludeesHidden)).Get() );*/
	//Print ( cstr(cstr("Tracking = ") + cstr(trackingSize)).Get() );
	//  Work out overall time spent per cycle
	t.game.perf.overall += PerformanceTimer()-g.gameperfoveralltimestamp ; g.gameperfoveralltimestamp=PerformanceTimer();

	//  Handle VR main camera render swap

	//  HUD Damage Display
	controlblood();
	controldamagemarker();

	//  Slider menus rendered last
	sliders_draw ( );

	//  Detect if FPS drops (only for single player - never for MP games)
	if (  t.game.runasmultiplayer == 0 && g.globals.hidelowfpswarning == 0 && g.tabmode == 0 && g.ghardwareinfomode == 0 && t.visuals.generalpromptstatetimer == 0 ) 
	{
		if ( t.conkit.cooldown>0 )  
		{
			// cooldown ensures this FPS warning only happens when cooled off
			--t.conkit.cooldown;
			g.lowfpstarttimer = Timer();
		}
		if (  t.conkit.editmodeactive == 0 && t.conkit.cooldown == 0 ) 
		{
			if (  g.lowfpswarning == 0 ) 
			{
				if ( (unsigned long)Timer()>g.lowfpstarttimer+2000 ) 
				{
					if ( GetDisplayFPS()<20 ) 
					{
						g.lowfpswarning=1;
						game_showmouse ( );
					}
				}
			}
			else
			{
				if (  g.lowfpswarning == 2 ) 
				{
					game_hidemouse ( );
					while ( MouseClick() != 0 ) { }
					g.lowfpswarning=3;
				}
			}
		}
	}

	//  Only render main and postprocess camera (not paint camera, reflection or lightray cameras)
	//  for globals.riftmode, left and right eyes are rendered in the _postprocess_preterrain step
//  `if t.conkit.editmodeactive=1

	//tmastersyncmask=%0000+(1<<terrain.paintcameraindex)
//  `else

		t.tmastersyncmask=0;
//  `endif

	SyncMask (  t.tmastersyncmask+(1<<3)+(1) );

	//  Update RealSense if any
	///realsense_loop ( );

	// show the KeyState
	if ( g.globals.fulldebugview == 1 ) debug_fulldebugview ( );

	//  Update screen
	//g.gameperftimestamp=PerformanceTimer();
	Sync (  );

	game_dynamicRes();

	//Dave Performance - let the occluder thread know it is okay to begin
	CPU3DOcclude (  );
	if ( g_hOccluderBegin ) SetEvent ( g_hOccluderBegin );

	t.game.perf.synctime += (PerformanceTimer()-g.gameperftimestamp) ; g.gameperftimestamp=PerformanceTimer();

	//  collect main Sync (  statistics )
	t.mainstatistic1=GetStatistic(1);
	t.mainstatistic5=GetStatistic(5);

	//  detect and slow down action
	if (  t.visuals.debugvisualsmode == 4 ) 
	{
		if (  ReturnKey() == 1 ) 
		{
			t.player[1].health=50000;
			physics_pausephysics ( );
			SleepNow (  200 );
			physics_resumephysics ( );
		}
	}

	//  Screen shot feature (redundant on Steam)
	if (  KeyState(g.keymap[68]) == 0  )  t.game.takingsnapshotpress = 0;
	if (  KeyState(g.keymap[68]) == 1 && t.game.takingsnapshotpress == 0 ) 
	{
		//  Grab current screen and save out under unique filename
		if (  ImageExist(g.savescreenshotimage) == 1  )  DeleteImage (  g.savescreenshotimage );
		GrabImage (  g.savescreenshotimage,0,0,GetDisplayWidth(),GetDisplayHeight(),1 );
		file_createmydocsfolder ( );
		t.tfilenum=1;
		while (  FileExist( cstr(g.myownrootdir_s+"\\snapshot"+Str(t.tfilenum)+".jpg").Get() ) == 1 ) 
		{
			++t.tfilenum;

			// GOCTHA!!
			t.strwork = ""; t.strwork = t.strwork + g.myownrootdir_s + "\\snapshot" + Str(t.tfilenum) + ".jpg";
		}
		t.strwork = ""; t.strwork += g.myownrootdir_s+"\\snapshot"+Str(t.tfilenum)+".jpg";
		if ( ImageExist(g.savescreenshotimage) == 1 )
		{
			SaveImage ( t.strwork.Get(), g.savescreenshotimage, 0 );
			DeleteImage (  g.savescreenshotimage );
		}
		t.game.takingsnapshotpress=1;
	}

	//  Work out performance metrics
	t.slidersmenuindex=t.slidersmenunames.performance  ; sliders_readall ( );
}

void game_main_stop ( void )
{

	//  Rest any ingame variables
	if (  t.conkit.entityeditmode != 0 || t.conkit.editmodeactive == 1 ) 
	{
		conkitedit_switchoff ( );
	}

return;

//  called from a lua script to finish current level
}

void game_jump_to_level_from_lua ( void )
{
	if (  t.game.gameisexe == 1 ) 
	{
		if (  t.game.gameloop == 1 && t.game.levelendingcycle  ==  0 ) 
		{
			t.game.jumplevel_s = t.tleveltojump_s;
			t.game.levelendingcycle = 4000;
		}
	}
	else
	{
		t.s_s = "" ; t.s_s = t.s_s+"Jump To Level : "+t.tleveltojump_s ; lua_prompt ( );
	}
}

void game_finish_level_from_lua ( void )
{
	if (  t.game.gameisexe == 1 ) 
	{
		if (  t.game.gameloop == 1 && t.game.levelendingcycle  ==  0 ) 
		{
			t.game.levelendingcycle = 4000;
		}
	}
	else
	{
		t.s_s="Level Complete Triggered"  ; lua_prompt ( );
	}
}

void game_end_of_level_check ( void )
{
	//  end of level fade out
	t.game.levelendingcycle = t.game.levelendingcycle - (g.timeelapsed_f * 200.0);
	t.huddamage.immunity=1000;
	if (  t.game.levelendingcycle  <=  0 ) 
	{
		t.game.gameloop=0;
		t.game.levelendingcycle = 0;
	}

	//  control fade out of screen
	t.postprocessings.fadeinvalue_f = t.game.levelendingcycle / 4000.0;
	t.postprocessings.fadeinvalueupdate=1;

	//  fade audio
	if (  t.audioVolume.music > t.postprocessings.fadeinvalue_f * 100.0  )  t.audioVolume.music  =  t.postprocessings.fadeinvalue_f * 100.0;
	if (  t.audioVolume.sound > t.postprocessings.fadeinvalue_f * 100.0  )  t.audioVolume.sound  =  t.postprocessings.fadeinvalue_f * 100.0;
	audio_volume_update ( );
}

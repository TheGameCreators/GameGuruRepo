//----------------------------------------------------
//--- GAMEGURU - M-LUA
//----------------------------------------------------

#include "gameguru.h"

// 
//  LUA Module
// 

void lua_init ( void )
{
	//  Clear lua bank
	g.luabankmax=0 ; Dim (  t.luabank_s,g.luabankmax  );

	//  Load the common scripts
	if (  g.luabankmax == 0 ) 
	{
		g.luabankmax=2;
		Dim (  t.luabank_s,g.luabankmax  );
		t.tfile_s="scriptbank\\global.lua";
		t.luabank_s[1]=t.tfile_s ; t.r=LoadLua(t.tfile_s.Get());
		t.strwork = "" ; t.strwork = t.strwork + "Loaded "+t.tfile_s;
		timestampactivity(0, t.strwork.Get() );
		t.tfile_s="scriptbank\\music.lua";
		t.luabank_s[2]=t.tfile_s ; t.r=LoadLua(t.tfile_s.Get());
		t.strwork = "" ; t.strwork = t.strwork + "Loaded "+t.tfile_s;
		timestampactivity(0, t.strwork.Get() );
	}

	//  Ensure entity elements are set to a LUA first run state
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entityelement[t.e].lua.firsttime=0;
	}

	//  Reset image system
	for ( t.t = 0 ; t.t<=  99; t.t++ )
	{
		if (  ImageExist(g.promptimageimageoffset+t.t) == 1  )  DeleteImage (  g.promptimageimageoffset+t.t );
	}
	t.promptimage.alignment=0;
	t.promptimage.x=0;
	t.promptimage.y=0;
	t.promptimage.img=0;
	t.promptimage.show=0;

	//  Each time game is paused, add up so we can 'freeze' the LUA Timer value
	t.aisystem.cumilativepauses=0;

	// 100316 - ensure GameLoopInit is called at start of each game session
	t.playercontrol.gameloopinitflag = 10;

	// flags to reset before LUA activity starts
	g.luaactivatemouse = 0;
	g.luacameraoverride = 0;

	// reset some LUA globals
	g.projectileEventType_explosion = 0;
	g.projectileEventType_name = "";
	g.projectileEventType_x = 0;
	g.projectileEventType_y = 0;
	g.projectileEventType_z = 0;
	g.projectileEventType_radius = 0;
	g.projectileEventType_damage = 0;
	g.projectileEventType_entityhit = 0;
}

void lua_loadscriptin ( void )
{
	//  gets entity ready to run AI system
	if (  t.e>0 ) 
	{
		if (  Len(t.entityelement[t.e].eleprof.aimain_s.Get())>0 ) 
		{
			t.tscriptname_s=t.entityelement[t.e].eleprof.aimain_s;
			if (  strcmp ( Right(t.tscriptname_s.Get(),4) , ".fpi" ) == 0  ) { t.strwork = "" ; t.strwork = t.strwork + Left(t.tscriptname_s.Get(),Len(t.tscriptname_s.Get())-4)+".lua" ; t.tscriptname_s = t.strwork; }
			if ( strcmp ( Lower(Right(t.tscriptname_s.Get(),4)) , ".lua" ) != 0 ) 
			{
				t.tscriptname_s=t.tscriptname_s+".lua";
			}
			t.tfile_s = "" ; t.tfile_s=t.tfile_s+"scriptbank\\"+t.tscriptname_s;
			if (  FileExist(t.tfile_s.Get()) == 1 ) 
			{
				t.tfound=0;
				for ( t.i = 1 ; t.i<=  g.luabankmax; t.i++ )
				{
					if (  t.luabank_s[t.i] == t.tfile_s  )  t.tfound = 1;
				}
				if (  t.tfound == 0 ) 
				{
					t.r=LoadLua(t.tfile_s.Get());
					if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
					if (  t.r == 0 ) 
					{
						t.strwork = "" ; t.strwork = t.strwork + "Loaded "+t.tfile_s;
						timestampactivity(0, t.strwork.Get() );
						++g.luabankmax;
						Dim (  t.luabank_s,g.luabankmax  );
						t.luabank_s[g.luabankmax]=t.tfile_s;
						t.tfound=1;
					}
					else
					{
						t.strwork = ""; t.strwork = t.strwork + "Failed to LoadLua ( " + t.tfile_s + " ):"+Str(t.r);
						timestampactivity(0, t.strwork.Get() );
						//       `1; Error occurred running the script
						//       `2; Syntax error
						//       `3; Required memory could not be allocated
						//       `4; Error with error reporting mechanism. (Don't ask!)
						//       `5; Error reading or opening script file (right filename?)
					}
				}
				else
				{
					//  already loaded previously
				}
				if (  t.tfound == 1 ) 
				{
					t.entityelement[t.e].eleprof.aimainname_s=Left(t.tscriptname_s.Get(),Len(t.tscriptname_s.Get())-4);
					t.entityelement[t.e].eleprof.aimain=1;
					t.entityelement[t.e].eleprof.aipreexit=-1;
					t.entityelement[t.e].lua.plrinzone=-1;
					t.entityelement[t.e].lua.entityinzone=0;
					t.entityelement[t.e].lua.flagschanged=1;
					t.entityelement[t.e].lua.dynamicavoidance=0;
					t.entityelement[t.e].lua.dynamicavoidancestuckclock = 0.0f;
				}
			}
		}
	}
}

void lua_scanandloadactivescripts ( void )
{
	//  Scan all active entities and load in used scripts
	//  no nesting as cannot resolve folder nests with global naming convention
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entityelement[t.e].eleprof.aimain=0;
		t.entityelement[t.e].eleprof.aipreexit=-1;
		if (  t.entityelement[t.e].bankindex>0 && t.entityelement[t.e].staticflag == 0 ) 
		{
			//  AI MAIN SCRIPT
			lua_loadscriptin ( );
		}
	}
}

void lua_free ( void )
{
	//  Reset entire LUA environment
	LuaReset (  );

	//  Clear lua bank
	for ( t.i = 1 ; t.i <= g.luabankmax ; t.i++ ) t.luabank_s[t.i]="" ; 
	g.luabankmax=0;
}

void lua_initscript ( void )
{
	// call the INIT functions of all entities
	if ( t.entityelement[t.e].active != 0 || t.entityelement[t.e].eleprof.spawnatstart == 0 )
	{
		if (  t.entityelement[t.e].eleprof.aimain == 1 ) 
		{
			// 151016 - need to ensure g_Entity globals are in place BEFORE INIT, so call update function
			t.tfrm=0; t.tobj = t.entityelement[t.e].obj;
			lua_ensureentityglobalarrayisinitialised();

			// first try initialising with a name string
			t.strwork = ""; t.strwork = t.strwork + Lower(t.entityelement[t.e].eleprof.aimainname_s.Get())+"_init_name";
			LuaSetFunction ( t.strwork.Get() ,2,0 );
			t.tentityname_s = t.entityelement[t.e].eleprof.name_s;
			LuaPushInt (  t.e  ); LuaPushString (  t.tentityname_s.Get()  ); LuaCallSilent (  );
			//  then try initialising without the name parameter
			t.strwork = ""; t.strwork = t.strwork + Lower(t.entityelement[t.e].eleprof.aimainname_s.Get())+"_init";
			LuaSetFunction ( t.strwork.Get() ,1,0 );
			LuaPushInt (  t.e  ); LuaCallSilent (  );
		}
	}
}

void lua_launchallinitscripts ( void )
{
	// call the INIT function of the GLOBAL GAMELOOP INIT
	if ( t.playercontrol.gameloopinitflag == 10 )
	{
		// calls Init once per game
		LuaSetFunction ( "GameLoopInit", 4, 0 );
		LuaPushInt ( t.playercontrol.startstrength ); 
		LuaPushInt ( t.playercontrol.regenrate ); 
		LuaPushInt ( t.playercontrol.regenspeed ); 
		LuaPushInt ( t.playercontrol.regendelay ); 
		LuaCall();

		// sets a decrement of 9 cycles before calling GameLoop function
		t.playercontrol.gameloopinitflag = 9;
	}

	//  launch scripts attached to entities
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if (  t.entityelement[t.e].bankindex>0 ) 
		{
			lua_initscript ( );
		}
	}

	// launch music script
	LuaSetFunction ( "music_init", 0, 0 );
	LuaCallSilent ( );
}

void lua_loop_begin ( void )
{
	// Spawn 1 item from the queue
	entity_lua_spawnifusedfromqueue();
	entity_lua_activateifusedfromqueue();

	// Write LUA globals
	LuaSetInt (  "g_GameStateChange", t.luaglobal.gamestatechange );
	if ( ObjectExist(t.aisystem.objectstartindex)==1 )
	{
		LuaSetFloat (  "g_PlayerPosX",ObjectPositionX(t.aisystem.objectstartindex) );
		LuaSetFloat (  "g_PlayerPosY",ObjectPositionY(t.aisystem.objectstartindex) );
		LuaSetFloat (  "g_PlayerPosZ",ObjectPositionZ(t.aisystem.objectstartindex) );
	}
	LuaSetFloat ( "g_PlayerAngX", wrapangleoffset(CameraAngleX(0)) );
	LuaSetFloat ( "g_PlayerAngY", wrapangleoffset(CameraAngleY(0)) );
	LuaSetFloat ( "g_PlayerAngZ", wrapangleoffset(CameraAngleZ(0)) );
	LuaSetInt (  "g_PlayerObjNo", t.aisystem.objectstartindex );
	LuaSetInt (  "g_PlayerHealth", t.player[t.plrid].health );
	LuaSetInt (  "g_PlayerLives", t.player[t.plrid].lives );
	LuaSetFloat (  "g_PlayerFlashlight", t.playerlight.flashlightcontrol_f );
	LuaSetInt (  "g_PlayerGunCount", t.guncollectedcount );
	LuaSetInt("g_PlayerGunID", t.gunid);
	if ( t.gunid > 0 )
		LuaSetString("g_PlayerGunName", t.gun[t.gunid].name_s.Get());
	else
		LuaSetString("g_PlayerGunName", "");
	LuaSetInt (  "g_PlayerGunMode", t.gunmode );
	int iGunIsFiring = 0;
	if ( t.gunmode >= 101 && t.gunmode <= 120 ) iGunIsFiring = 1;
	if ( t.gunmode >= 1020 && t.gunmode <= 1023 ) iGunIsFiring = 2;
	LuaSetInt (  "g_PlayerGunFired", iGunIsFiring );
	LuaSetInt (  "g_PlayerGunAmmoCount", t.slidersmenuvalue[t.slidersmenunames.weapon][1].value );
	LuaSetInt (  "g_PlayerGunClipCount", t.slidersmenuvalue[t.slidersmenunames.weapon][2].value );
	LuaSetInt (  "g_PlayerGunZoomed", t.gunzoommode );
	LuaSetInt (  "g_Time", Timer()-t.aisystem.cumilativepauses );
	LuaSetFloat (  "g_TimeElapsed", g.timeelapsed_f );
	LuaSetInt (  "g_PlayerThirdPerson", t.playercontrol.thirdperson.enabled );
	LuaSetInt (  "g_PlayerController", g.gxbox );
	int iPlayerFOVPerc = (((t.visuals.CameraFOV_f*t.visuals.CameraASPECT_f)-20.0)/180.0)*100.0;
	LuaSetInt (  "g_PlayerFOV", iPlayerFOVPerc );
	LuaSetInt (  "g_PlayerLastHitTime", t.playercontrol.regentime );
	LuaSetInt (  "g_PlayerDeadTime", t.playercontrol.deadtime );

	//  Quick detection of E key
	if (  t.aisystem.processplayerlogic == 1 ) 
	{
		if (  t.player[t.plrid].health>0  )  t.tKeyPressE = KeyState(18); else t.tKeyPressE = 0;
	}
	else
	{
		t.tKeyPressE=KeyState(g.keymap[18]);
	}
	if ( g.gxbox == 1 ) 
	{
		if ( JoystickFireD() == 1 )  
			t.tKeyPressE = 1;
	}
	if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		if ( GGVR_RightController_Trigger() > 0.9f )
			t.tKeyPressE = 1;
	}
	LuaSetInt ( "g_KeyPressE",t.tKeyPressE );
	LuaSetInt ( "g_KeyPressQ",KeyState(g.keymap[16]) );

	// 241115 - other common keys which might require SIMULTANEOUS detection (bot control)
	LuaSetInt ( "g_KeyPressW", KeyState(g.keymap[17]) );
	LuaSetInt ( "g_KeyPressA", KeyState(g.keymap[30]) );
	LuaSetInt ( "g_KeyPressS", KeyState(g.keymap[31]) );
	LuaSetInt ( "g_KeyPressD", KeyState(g.keymap[32]) );
	LuaSetInt ( "g_KeyPressR", KeyState(g.keymap[19]) );
	LuaSetInt ( "g_KeyPressF", KeyState(g.keymap[33]) );
	LuaSetInt ( "g_KeyPressC", KeyState(g.keymap[46]) );
	//LuaSetInt ( "g_KeyPressJ", !!done in player control code!! );
	LuaSetInt ( "g_KeyPressSPACE", KeyState(g.keymap[57]) );

	// shift key for running/etc
	int tKeyPressShift = 0;
	if ( KeyState(g.keymap[42]) ) tKeyPressShift = 1;
	if ( KeyState(g.keymap[54]) ) tKeyPressShift = 1;
	if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		if ( GGVR_RightController_Grip() == 1 )
			tKeyPressShift = 1;
	}
	LuaSetInt ( "g_KeyPressSHIFT", tKeyPressShift );

	if ( g.luaactivatemouse == 1 )
	{
		g.LUAMouseX += MouseMoveX();
		g.LUAMouseY += MouseMoveY();
		if ( g.LUAMouseX < 0.0f ) g.LUAMouseX = 0.0f;
		if ( g.LUAMouseY < 0.0f ) g.LUAMouseY = 0.0f;
		if ( g.LUAMouseX >= GetDisplayWidth() ) g.LUAMouseX = GetDisplayWidth()-1;
		if ( g.LUAMouseY >= GetDisplayHeight() ) g.LUAMouseY = GetDisplayHeight()-1;
		float fFinalPercX = ( g.LUAMouseX / GetDisplayWidth() ) * 100.0f;
		float fFinalPercY = ( g.LUAMouseY / GetDisplayHeight() ) * 100.0f;
		LuaSetFloat ( "g_MouseX", fFinalPercX );
		LuaSetFloat ( "g_MouseY", fFinalPercY );
		LuaSetInt ( "g_MouseWheel", MouseMoveZ() );

		// 310316 - need to keep real mouse fixed (or it clicks things in other monitors)
		HWND hForeWnd = GetForegroundWindow();
		HWND hThisWnd = g_pGlob->hWnd;
		if ( hThisWnd == hForeWnd ) SetCursorPos(320,240);
	}
	else
	{
		//230216 - to help scripting, relay absolute values if not in mouse active mode
		LuaSetFloat ( "g_MouseX", -1.0f );
		LuaSetFloat ( "g_MouseY", -1.0f );
		//LuaSetInt ( "g_MouseWheel", 0 );
		LuaSetInt ( "g_MouseWheel", MouseZ() );
	}
	LuaSetInt ( "g_MouseClick", MouseClick() );

	// continuing settinf of LUA globals
	LuaSetInt ( "g_EntityElementMax", g.entityelementlist );	

	LuaSetInt("g_PlayerUnderwaterMode", g.underwatermode); // PE: underwater mode active.

	// 020316 - call the global loop once per cycle (for things like loading game states)
	if ( t.playercontrol.gameloopinitflag > 0 ) t.playercontrol.gameloopinitflag--;
	if ( t.playercontrol.gameloopinitflag == 0 )
	{
		// NOTE: Not entirely happy with a call which effectively 'loads from buffer' all the time
		t.tnothing = LuaExecute( cstr ( cstr("GlobalLoop(") + cstr(t.game.gameloop) + cstr(")") ).Get() );
	}

	// 170215 - use regular way to assign globals, not above execute approach
	t.tscan = ScanCode();
	LuaSetInt("g_Scancode", t.tscan);
	if (  t.tscan  ==  0 || t.tscan  ==  54 || t.tscan  ==  29 || t.tscan  ==  56 || t.tscan  ==  184 || t.tscan  ==  157 || t.tscan  ==  58 || t.tscan  ==  15 ) 
	{
		t.strwork = "";
	}
	else
	{
		t.tchar_s = Inkey();
		if (  t.tchar_s  ==  Chr(34)  )  t.tchar_s  =  cstr("\\") + Chr(34);
		if (  t.tchar_s  ==  "\\"  )  t.tchar_s  =  "\\\\";
		if (  Asc(t.tchar_s.Get()) < 32 || Asc(t.tchar_s.Get()) > 126  )  t.tchar_s  =  "";
		t.strwork = t.tchar_s;
	}
	LuaSetString("g_InKey", t.strwork.Get());
	LuaSetString("g_LevelFilename", g.projectfilename_s.Get()+strlen("mapbank\\"));

	// pass in values from projectileexplosionevents
	LuaSetInt("g_projectileevent_explosion", g.projectileEventType_explosion);
	LuaSetString("g_projectileevent_name", g.projectileEventType_name.Get());
	LuaSetInt("g_projectileevent_x", g.projectileEventType_x);
	LuaSetInt("g_projectileevent_y", g.projectileEventType_y);
	LuaSetInt("g_projectileevent_z", g.projectileEventType_z);
	LuaSetInt("g_projectileevent_radius", g.projectileEventType_radius);
	LuaSetInt("g_projectileevent_damage", g.projectileEventType_damage);
	LuaSetInt("g_projectileevent_entityhit", g.projectileEventType_entityhit);
}

void lua_quitting ( void )
{
	lua_loop_begin();
	LuaSetFunction ( "GameLoopQuit", 0, 0 );
	LuaCall();
	lua_loop_finish();
}

void lua_updateweaponstats ( void )
{
	for ( int iMode = 1; iMode <= 5 ; iMode++ )
	{
		int iIndexMax = 10;
		if ( iMode >= 3 ) iIndexMax = 20;
		if ( iMode == 5 ) iIndexMax = 100;
		for ( int iIndex = 0; iIndex <= iIndexMax ; iIndex++ )
		{
			int iValue = 0;
			if ( iMode==1 ) iValue = t.weaponslot [ iIndex ].got;
			if ( iMode==2 ) iValue = t.weaponslot [ iIndex ].pref;
			if ( iMode==3 ) iValue = t.weaponammo [ iIndex ];
			if ( iMode==4 ) iValue = t.weaponclipammo [ iIndex ];
			if ( iMode==5 ) iValue = t.ammopool [ iIndex ].ammo;
			LuaSetFunction (  "UpdateWeaponStatsItem", 3, 0 ); LuaPushInt ( iMode ); LuaPushInt ( iIndex ); LuaPushInt ( iValue ); LuaCall (  );
		}
	}
}

void lua_ensureentityglobalarrayisinitialised ( void )
{
	if ( t.entityelement[t.e].lua.firsttime == 0 ) 
	{
		// only occurs once, unless new spawn/etc
		t.entityelement[t.e].lua.firsttime = 1;

		// 300316 - no need for entity details for static scenery entities in LUA
		if ( t.entityelement[t.e].staticflag == 0 ) 
		{
			LuaSetFunction (  "UpdateEntity",22,0 );
			LuaPushInt (  t.e );
			LuaPushInt (  t.tobj );
			LuaPushFloat (  t.entityelement[t.e].x );
			LuaPushFloat (  t.entityelement[t.e].y );
			LuaPushFloat (  t.entityelement[t.e].z );
			LuaPushFloat (  t.entityelement[t.e].rx );
			LuaPushFloat (  t.entityelement[t.e].ry );
			LuaPushFloat (  t.entityelement[t.e].rz );
			LuaPushInt (  t.entityelement[t.e].active );
			LuaPushInt (  t.entityelement[t.e].activated );
			LuaPushInt (  t.entityelement[t.e].collected );
			LuaPushInt (  t.entityelement[t.e].lua.haskey );
			LuaPushInt (  t.entityelement[t.e].lua.plrinzone );
			LuaPushInt (  t.entityelement[t.e].lua.entityinzone );
			LuaPushInt (  t.entityelement[t.e].plrvisible );
			LuaPushInt (  t.entityelement[t.e].lua.animating );
			LuaPushInt (  t.entityelement[t.e].health );
			LuaPushInt (  t.tfrm );
			LuaPushFloat (  t.entityelement[t.e].plrdist );
			LuaPushInt (  t.entityelement[t.e].lua.dynamicavoidance );

			// 201115 - pass in any hit limb name
			LPSTR pLimbByName = "";
			if ( t.entityelement[t.e].detectedlimbhit >=0 )
			{
				int iObjectNumber = g.entitybankoffset + t.entityelement[t.e].bankindex;
				if ( ObjectExist(iObjectNumber) == 1 )
					if ( LimbExist ( iObjectNumber, t.entityelement[t.e].detectedlimbhit ) == 1 )
						pLimbByName = LimbName ( iObjectNumber, t.entityelement[t.e].detectedlimbhit );
			}

			// push remaining and call
			LuaPushString ( pLimbByName );
			LuaPushInt ( t.entityelement[t.e].detectedlimbhit );
			LuaCall (  );
			t.entityelement[t.e].lua.dynamicavoidance=0;
			t.entityelement[t.e].lua.dynamicavoidancestuckclock = 0.0f;
			t.entityelement[t.e].lua.firsttime = 2;

			// 120417 - set last to current for initial values used by script (delta=0)
			t.entityelement[t.e].lastx = t.entityelement[t.e].x;
			t.entityelement[t.e].lasty = t.entityelement[t.e].y;
			t.entityelement[t.e].lastz = t.entityelement[t.e].z;
			t.entityelement[t.e].animspeedmod = 1.0f;
		}
	}
}

void lua_loop_allentities ( void )
{
	//  Go through all entities with active LUA scripts
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		int thisentid = t.entityelement[t.e].bankindex;
		if ( thisentid>0 && (t.entityelement[t.e].active != 0 || t.entityelement[t.e].lua.flagschanged == 2 || t.entityelement[t.e].eleprof.phyalways != 0 || t.entityelement[t.e].eleprof.spawnatstart==0) ) 
		{
			//  Update entity coordinates with real object coordinates
			t.tfrm=0 ; t.tobj=t.entityelement[t.e].obj;
			if (  t.tobj>0 ) 
			{
				if (  ObjectExist(t.tobj) == 1 ) 
				{
					t.tfrm=GetFrame(t.tobj);
					t.tentid=t.entityelement[t.e].bankindex;
					if (  t.entityprofile[t.tentid].collisionmode != 21 && t.entityprofile[t.tentid].ischaracter != 1 ) 
					{
						//  except entity driven physics objects
						t.entityelement[t.e].x=ObjectPositionX(t.tobj);
						t.entityelement[t.e].y=ObjectPositionY(t.tobj);
						t.entityelement[t.e].z=ObjectPositionZ(t.tobj);
					}
				}
			}

			// 210516 - larger levels (Apocalypse) can produce strange behavior which
			/* 11:55PM - worked enough to recreate table, but 'other' related corruptions caused problems during re-create step!
			// wipes out g_entity[e] tables seemingly at random (at fixed times during the game)
			// suspect LUA internal nuances or memory corruption outside of LUA, so the solution
			// it to detect when such a table is not valid and repair it
			if ( t.entityelement[t.e].lua.firsttime == 2 ) 
			{
				// determine if our g_entity array still exists
				if ( LuaValidateEntityTable ( t.e ) == 0 )
				{
					// recreate table
					t.entityelement[t.e].lua.firsttime = 0;
				}
			}
			*/

			// Initial population of LUA data
			lua_ensureentityglobalarrayisinitialised();

			//  only process logic within plr freeze range
			t.te = t.e; entity_getmaxfreezedistance ( );
			if ( t.entityelement[t.e].plrdist<t.maximumnonefreezedistance || t.entityelement[t.e].lua.flagschanged==2 ) 
			{
				//  If entity is waypoint zone, determine if player inside or outside
				t.waypointindex=t.entityelement[t.e].eleprof.trigger.waypointzoneindex;
				if (  t.waypointindex>0 ) 
				{
					t.tpointx_f=CameraPositionX(0);
					t.tpointz_f=CameraPositionZ(0);
					if (  t.waypoint[t.waypointindex].active == 1 ) 
					{
						if (  t.waypoint[t.waypointindex].style == 2 ) 
						{
							t.tokay = 0; waypoint_ispointinzone ( );
							if (  t.entityelement[t.e].lua.plrinzone != t.tokay ) 
							{
								t.entityelement[t.e].lua.plrinzone=t.tokay;
								t.entityelement[t.e].lua.flagschanged=1;
							}
						}
					}
				}

				//  Detect if USE KEY field entity has been collected
				if (  t.entityelement[t.e].lua.haskey == 0 ) 
				{
					//  check if demilited key
					t.masterkeyname_s=Lower(t.entityelement[t.e].eleprof.usekey_s.Get());
					if (  Len(t.masterkeyname_s.Get())>0 ) 
					{
						t.tmultikey=0;
						for ( t.n = 1 ; t.n<=  Len(t.masterkeyname_s.Get()); t.n++ )
						{
							if (  cstr(Mid(t.masterkeyname_s.Get(),t.n)) == ";" )
							{
								t.tmultikey=1;
							}
						}
						//  Is USEKEY Collected?
						t.tokay=0;
						if (  t.tmultikey == 0 ) 
						{
							//  (SINGLE)
							for ( t.te = 1 ; t.te<=  g.entityelementlist; t.te++ )
							{
								if (  t.entityelement[t.te].collected == 1 ) 
								{
									if (  cstr(Lower(t.entityelement[t.te].eleprof.name_s.Get())) == t.masterkeyname_s ) 
									{
										t.tokay=1 ; break;
									}
								}
							}
						}
						else
						{
							//  (MULTIPLE)
							t.tokay=1;
							t.n=1;
							while (  t.n <= Len(t.masterkeyname_s.Get()) ) 
							{
								t.keyname_s="";
								while (  t.n <= Len(t.masterkeyname_s.Get()) ) 
								{
									if (  cstr(Mid(t.masterkeyname_s.Get(),t.n)) == ";"  )  break;
									t.keyname_s=t.keyname_s+Mid(t.masterkeyname_s.Get(),t.n);
									++t.n;
								}
								//  look for this key
								t.ttokay=0;
								for ( t.te = 1 ; t.te <= g.entityelementlist; t.te++ )
								{
									if (  t.entityelement[t.te].collected == 1 ) 
									{
										if (  cstr(Lower(t.entityelement[t.te].eleprof.name_s.Get())) == t.keyname_s ) 
										{
											t.ttokay=1 ; break;
										}
									}
								}
								//  any key not found means overall master key not valid
								if (  t.ttokay == 0  )  t.tokay = 0;
								++t.n;
							}
						}
						t.entityelement[t.e].lua.haskey=t.tokay;
						t.entityelement[t.e].lua.flagschanged=1;
					}
					else
					{
						//  when door/gate entity does not specify USE KEY, set to -1 to script knows
						//  no key/entity is required here (for additional script behaviours)
						t.entityelement[t.e].lua.haskey=-1;
						t.entityelement[t.e].lua.flagschanged=1;
					}
				}

				//  Detect when entity object animation over
				if (  t.entityelement[t.e].lua.animating == 1 ) 
				{
					t.obj=t.entityelement[t.e].obj;
					if (  t.obj>0 ) 
					{
						if (  ObjectExist(t.obj) == 1 ) 
						{
							if (  GetPlaying(t.obj) == 0 && t.smoothanim[t.obj].transition == 0 ) 
							{
								t.entityelement[t.e].lua.animating=0;
								LuaSetFunction (  "UpdateEntityAnimatingFlag",2,0 );
								LuaPushInt (  t.e );
								LuaPushInt (  t.entityelement[t.e].lua.animating );
								LuaCall (  );
							}
						}
					}
				}

				// 0403016 - can call this one last time to refresh LUA global arrays
				bool bSkipLUAScriptEntityRefreshOnly = false;
				if ( t.entityelement[t.e].lua.flagschanged == 2 )
					bSkipLUAScriptEntityRefreshOnly = true;

				//  Update each cycle as entity position, health and GetFrame (  change constantly )
				if ( t.entityelement[t.e].plrdist<t.maximumnonefreezedistance/4 || t.entityprofile[thisentid].ischaracter == 1 || t.entityelement[t.e].eleprof.phyalways != 0 ) 
				{
					//  first quarter of freeze range get full updates - also characters and those with alwaysactive flags
					if ( t.entityelement[t.e].plrdist<t.maximumnonefreezedistance || t.entityelement[t.e].eleprof.phyalways != 0 ) 
						t.entityelement[t.e].lua.flagschanged=1;
				}
				else
				{
					//  rest gets updates every now and again based on distance
					if (  t.entityelement[t.e].plrdist<t.maximumnonefreezedistance/2.0f ) 
					{
						if (  Rnd(25) == 1  )  t.entityelement[t.e].lua.flagschanged = 1;
					}
					else
					{
						if (  t.entityelement[t.e].plrdist<t.maximumnonefreezedistance/1.25f ) 
						{
							if (  Rnd(50) == 1  )  t.entityelement[t.e].lua.flagschanged = 1;
						}
					}
				}

				// if game state in progress, do not run any entity logic
				if ( t.luaglobal.gamestatechange == 0 )
				{
					// this ensures the game loads in _G[x] states BEFORE we start the game scripts
					// to avoid issues such as the start splash appearing when loading mid-way in level from main menu
					// Called when entity states change
					if ( t.entityelement[t.e].lua.flagschanged == 1 ) // || (t.game.runasmultiplayer  ==  1 && g.mp.endplay  ==  1) ) the MP constant call would be slow!
					{
						//  do not refresh activated and animating as these are set INSIDE LUA!!
						// 190516 - ensure we can only call UpdateEntityRT if we previously called UpdateEntity!!
						if ( t.entityelement[t.e].staticflag == 0 && t.entityelement[t.e].lua.firsttime == 2 ) // 300316 - no need for entity details for static scenery entities in LUA
						{
							LuaSetFunction (  "UpdateEntityRT",21,0 );
							LuaPushInt (  t.e );
							LuaPushInt (  t.tobj );
							if ( g.mp.endplay == 0 ) // can now run own script in multiplayer || t.game.runasmultiplayer == 0
							{
								// if character, update entity coordinates from visible object
								int tentid = t.entityelement[t.e].bankindex;
								if ( tentid > 0 )
								{
									if ( t.entityprofile[tentid].ischaracter == 1 )
									{
										//PE: HierToo this one is ruin the wizard, give it a try, so another solution is needed :)
										//PositionObject(t.entityelement[t.e].obj, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z);
										//t.entityelement[t.e].x = ObjectPositionX ( t.entityelement[t.e].obj );
										t.entityelement[t.e].y = ObjectPositionY ( t.entityelement[t.e].obj );
										//t.entityelement[t.e].z = ObjectPositionZ ( t.entityelement[t.e].obj );
									}
								}
								LuaPushFloat ( t.entityelement[t.e].x );
								LuaPushFloat ( t.entityelement[t.e].y );
								LuaPushFloat ( t.entityelement[t.e].z );
							}
							else
							{
								LuaPushFloat ( 100000 );
								LuaPushFloat ( 100000 );
								LuaPushFloat ( 100000 );
							}
							LuaPushFloat (  t.entityelement[t.e].rx );
							LuaPushFloat (  t.entityelement[t.e].ry );
							LuaPushFloat (  t.entityelement[t.e].rz );
							LuaPushInt (  t.entityelement[t.e].active );
							LuaPushInt (  t.entityelement[t.e].activated );
							LuaPushInt (  t.entityelement[t.e].collected );
							LuaPushInt (  t.entityelement[t.e].lua.haskey );
							LuaPushInt (  t.entityelement[t.e].lua.plrinzone );
							LuaPushInt (  t.entityelement[t.e].lua.entityinzone );
							LuaPushInt (  t.entityelement[t.e].plrvisible );
							LuaPushInt ( t.entityelement[t.e].health );
							LuaPushInt (  t.tfrm );
							LuaPushFloat (  t.entityelement[t.e].plrdist );
							LuaPushInt (  t.entityelement[t.e].lua.dynamicavoidance );

							// 201115 - pass in any hit limb name
							LPSTR pLimbByName = "";
							if ( t.entityelement[t.e].detectedlimbhit >=0 )
							{
								int iObjectNumber = g.entitybankoffset + t.entityelement[t.e].bankindex;
								if ( ObjectExist(iObjectNumber) == 1 )
									if ( LimbExist ( iObjectNumber, t.entityelement[t.e].detectedlimbhit ) == 1 )
										pLimbByName = LimbName ( iObjectNumber, t.entityelement[t.e].detectedlimbhit );
							}
							LuaPushString ( pLimbByName );
							LuaPushInt ( t.entityelement[t.e].detectedlimbhit );
							LuaCall (  );
							t.entityelement[t.e].lua.flagschanged=0;
							if ( t.entityelement[t.e].lua.dynamicavoidance == 1 )
							{
								t.entityelement[t.e].lua.dynamicavoidance=0;
								t.entityelement[t.e].lua.dynamicavoidancestuckclock = 0.0f;
							}
							if ( t.entityelement[t.e].lua.dynamicavoidance == 2 )
							{
								t.entityelement[t.e].lua.dynamicavoidance = 0;
							}

							// 120417 - record entity XYZ after script call (ensures delta from everything)
							// moved below..
							//t.entityelement[t.e].lastx = t.entityelement[t.e].x;
							//t.entityelement[t.e].lasty = t.entityelement[t.e].y;
							//t.entityelement[t.e].lastz = t.entityelement[t.e].z;
						}
					}

					//  Call each cycle
					if (  t.entityelement[t.e].eleprof.aimain == 1 && bSkipLUAScriptEntityRefreshOnly==false ) 
					{
						if (  Len(t.entityelement[t.e].eleprof.aimainname_s.Get())>1 ) 
						{
							if ( 1 ) // can run LUA in multiplayer now t.game.runasmultiplayer == 0 || g.mp.gameAlreadySpawnedBefore  !=  0 ) 
							{
								// can call LUA main function
								t.tcall = 1;

								// for multiplayer coop, only call the main function if we are the ones in control of the ai
								/* now no scenario where call is skipped due to multiplayer
								if (  t.game.runasmultiplayer == 1 && g.mp.coop  ==  1 ) 
								{
									t.entid=t.entityelement[t.e].bankindex;
									if (  t.entid>0 ) 
									{
										if ( (t.entityprofile[t.entid].ischaracter  ==  1 || t.entityelement[t.e].mp_isLuaChar  ==  1) && t.entityprofile[t.entid].ismultiplayercharacter  ==  0 ) 
										{
											if (  t.entityelement[t.e].mp_coopControlledByPlayer  !=  g.mp.me  )  t.tcall  =  0;
										}
									}
									//  only run marker scripts when in endplay mode (multiplayer only)
									if (  t.game.runasmultiplayer  ==  1 && g.mp.endplay  ==  1 ) 
									{
										if (  t.entityprofile[t.entid].ismarker  ==  0  )  t.tcall  =  0;
									}
								}
								*/
								if ( t.entityelement[t.e].eleprof.aimainname_s.Lower() == "default" ) t.tcall = 0;		
								if ( t.tcall == 1 ) 
								{
									if ( t.entityelement[t.e].eleprof.aipreexit >= 1 )
									{
										if ( t.entityelement[t.e].eleprof.aipreexit == 1 )
										{
											t.entityelement[t.e].eleprof.aipreexit = 3;
											t.strwork = cstr(cstr(Lower(t.entityelement[t.e].eleprof.aimainname_s.Get()))+"_preexit");
											LuaSetFunction ( t.strwork.Get(), 1, 0 );
											LuaPushInt ( t.e ); LuaCall ( );
											if ( t.entityelement[t.e].eleprof.aipreexit == 2 )
											{
												t.v = 0.0f;
												entity_lua_setentityhealth();
											}
										}
									}
									else
									{
										t.strwork = cstr(cstr(Lower(t.entityelement[t.e].eleprof.aimainname_s.Get()))+"_main");
										LuaSetFunction ( t.strwork.Get() ,1,0 );
										LuaPushInt (  t.e  ); LuaCall (  );
									}
								}
							}
						}
					}
				}
			}

			// 090517 - should not depend on scripts being refreshed
			t.entityelement[t.e].lastx = t.entityelement[t.e].x;
			t.entityelement[t.e].lasty = t.entityelement[t.e].y;
			t.entityelement[t.e].lastz = t.entityelement[t.e].z;
		}
	}
}

void lua_loop_finish ( void )
{
	//  Detect any messges back from LUA engine (actions)
	while ( LuaNext() ) 
	{
		t.luaaction_s=LuaMessageDesc();
		if ( strcmp ( t.luaaction_s.Get() , "prompt" ) == 0 ) {  t.s_s=LuaMessageString(); lua_prompt(); }
		else if ( strcmp ( t.luaaction_s.Get() , "promptimage" ) == 0 ) { t.v=LuaMessageInt(); lua_promptimage() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptduration" ) == 0 ) {  t.v=LuaMessageIndex() ; t.s_s=LuaMessageString() ; lua_promptduration() ;}
		else if ( strcmp ( t.luaaction_s.Get() , "prompttextsize" ) == 0 ) {  t.v=LuaMessageInt() ; lua_prompttextsize() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptlocal" ) == 0 ) {  t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; lua_promptlocal() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptlocalforvrmode" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_promptlocalforvrmode() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptlocalforvr" ) == 0 ) {  t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; lua_promptlocalforvr() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfognearest" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setfognearest() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfogdistance" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setfogdistance() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfogred" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setfogred() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfoggreen" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setfoggreen() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfogblue" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setfogblue() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setfogintensity" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setfogintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setambienceintensity" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setambienceintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setambiencered" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setambiencered() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setambiencegreen" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setambiencegreen() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setambienceblue" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setambienceblue() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsurfaceintensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setsurfaceintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsurfacered" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setsurfacered() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsurfacegreen" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setsurfacegreen() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsurfaceblue" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setsurfaceblue() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsurfacesunfactor" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setsurfacesunfactor() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setglobalspecular" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setglobalspecular() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setbrightness" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setbrightness() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setconstrast" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setconstrast() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "setpostbloom" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostbloom() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostvignetteradius" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostvignetteradius() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostvignetteintensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostvignetteintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostmotiondistance" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostmotiondistance() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostmotionintensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostmotionintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostdepthoffielddistance" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setpostdepthoffielddistance() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostdepthoffieldintensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostdepthoffieldintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostlightraylength" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostlightraylength() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostlightrayquality" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostlightrayquality() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostlightraydecay" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostlightraydecay() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostsaoradius" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostsaoradius() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostsaointensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostsaointensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpostlensflareintensity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setpostlensflareintensity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setoptionreflection" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setoptionreflection() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setoptionshadows" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setoptionshadows() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setoptionlightrays" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setoptionlightrays() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setoptionvegetation" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setoptionvegetation() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setoptionocclusion" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setoptionocclusion() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcameradistance" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setcameradistance() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcamerafov" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setcamerafov() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcamerazoompercentage" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setcamerazoompercentage() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcameraweaponfov" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setcameraweaponfov() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setterrainlodnear" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setterrainlodnear() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setterrainlodmid" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setterrainlodmid() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setterrainlodfar" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setterrainlodfar() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setterrainsize" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setterrainsize() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setvegetationquantity" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setvegetationquantity() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setvegetationwidth" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setvegetationwidth() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setvegetationheight" ) == 0 ) {  t.v_f=LuaMessageFloat() ; lua_setvegetationheight() ; }

		if ( strcmp ( t.luaaction_s.Get() , "jumptolevel" ) == 0 ) {  t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; lua_jumptolevel() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "finishlevel" ) == 0 ) {  lua_finishlevel() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "hideterrain" ) == 0 ) {  t.v=LuaMessageInt() ; lua_hideterrain() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "showterrain" ) == 0 ) {  t.v=LuaMessageInt() ; lua_showterrain() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "hidewater" ) == 0 ) { t.v=LuaMessageInt() ; lua_hidewater() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "showwater" ) == 0 ) { t.v=LuaMessageInt() ; lua_showwater() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "hidehuds" ) == 0 ) {  t.v=LuaMessageInt() ; lua_hidehuds() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "showhuds" ) == 0 ) {  t.v=LuaMessageInt() ; lua_showhuds() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "freezeai" ) == 0 ) {  t.v=LuaMessageInt() ; lua_freezeai() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "unfreezeai" ) == 0 ) {  t.v=LuaMessageInt() ; lua_unfreezeai() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "freezeplayer" ) == 0 ) { t.v=LuaMessageInt() ; lua_freezeplayer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "unfreezeplayer" ) == 0 ) {  t.v=LuaMessageInt() ; lua_unfreezeplayer() ; } 
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositionx" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositionx(); }
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositiony" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositiony(); }
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositionz" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositionz(); }
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositionax" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositionax(); }
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositionay" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositionay(); }
		else if ( strcmp ( t.luaaction_s.Get() , "setfreezepositionaz" ) == 0 ) { t.v_f = LuaMessageFloat(); lua_setfreezepositionaz(); }
		else if ( strcmp ( t.luaaction_s.Get() , "transporttofreezeposition" ) == 0 ) { t.v=LuaMessageInt() ; lua_transporttofreezeposition() ; }
		
		else if ( strcmp ( t.luaaction_s.Get() , "activatemouse" ) == 0 ) { t.v=LuaMessageInt() ; lua_activatemouse() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "deactivatemouse" ) == 0 ) {  t.v=LuaMessageInt() ; lua_deactivatemouse() ; } 

		else if ( strcmp ( t.luaaction_s.Get() , "setplayerhealth" ) == 0 ) {  t.v= LuaMessageFloat() ; lua_setplayerhealth() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setplayerlives" ) == 0 ) {  t.v= LuaMessageFloat() ; lua_setplayerlives() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "removeplayerweapons" ) == 0 ) { t.v=LuaMessageInt() ; lua_removeplayerweapons() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "disablemusicreset" ) == 0 ) { t.v=LuaMessageInt() ; lua_disablemusicreset() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicload" ) == 0 ) { t.m=LuaMessageIndex(); t.s_s=LuaMessageString(); lua_musicload() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicsetinterval" ) == 0 ) { t.m=LuaMessageIndex() ; t.v=LuaMessageInt() ; lua_musicsetinterval() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicsetlength" ) == 0 ) { t.m=LuaMessageIndex() ; t.v=LuaMessageInt() ; lua_musicsetlength() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicstop" ) == 0 ) { lua_musicstop() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicplayinstant" ) == 0 ) { t.m=LuaMessageInt() ; lua_musicplayinstant() ; lua_musicplayinstant() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicplayfade" ) == 0 ) { t.m=LuaMessageInt() ; lua_musicplayfade() ; lua_musicplayfade() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicplaycue" ) == 0 ) { t.m=LuaMessageInt() ; lua_musicplaycue() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicplaytime" ) == 0 ) { t.m=LuaMessageIndex() ; t.v=LuaMessageInt() ; lua_musicplaytime() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicplaytimecue" ) == 0 ) { t.m=LuaMessageIndex() ; t.v=LuaMessageInt() ; lua_musicplaytimecue() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicsetvolume" ) == 0 ) { t.v=LuaMessageInt() ; lua_musicsetvolume() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicsetdefault" ) == 0 ) { t.m=LuaMessageInt() ; lua_musicsetdefault() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "musicsetfadetime" ) == 0 ) { t.v=LuaMessageInt() ; lua_musicsetfadetime() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "startparticleemitter" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; lua_startparticleemitter() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "stopparticleemitter" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; lua_stopparticleemitter() ; }

		if ( strcmp ( t.luaaction_s.Get() , "starttimer" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_starttimer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "destroy" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_destroy() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "collisionon" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_collisionon() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "collisionoff" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_collisionoff() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "getentityplrvisible" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_getentityplrvisible() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "getentityinzone" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_getentityinzone() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "hide" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_hide() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "show" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_show() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "spawn" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_spawn() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setactivated" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setactivated() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setactivatedformp" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setactivatedformp() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetlimbhit" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_resetlimbhit() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "activateifused" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_activateifused() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "spawnifused" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_spawnifused() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "transporttoifused" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_transporttoifused() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "refreshentity" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_refreshentity() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "moveup" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_moveup() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "moveforward" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_moveforward() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "movebackward" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_movebackward() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "sethoverfactor" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_sethoverfactor() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "setpositionx" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setpositionx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpositiony" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setpositiony() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setpositionz" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setpositionz() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetpositionx" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetpositionx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetpositiony" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetpositiony() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetpositionz" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetpositionz() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "setrotationx" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setrotationx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setrotationy" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setrotationy() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setrotationz" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setrotationz() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetrotationx" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetrotationx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetrotationy" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetrotationy() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "resetrotationz" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_resetrotationz() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "modulatespeed" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_modulatespeed() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatex" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatex() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatey" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatey() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatez" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatez() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setlimbindex" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setlimbindex() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatelimbx" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatelimbx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatelimby" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatelimby() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatelimbz" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_rotatelimbz() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setentityhealth" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setentityhealth() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setentityhealthsilent" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setentityhealthsilent() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setforcex" ) == 0 ) { t.v=LuaMessageFloat() ; entity_lua_setforcex() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setforcey" ) == 0 ) { t.v=LuaMessageFloat() ; entity_lua_setforcey() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setforcez" ) == 0 ) { t.v=LuaMessageFloat() ; entity_lua_setforcez() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setforcelimb" ) == 0 ) { t.e=LuaMessageIndex(); t.v=LuaMessageInt() ; entity_lua_setforcelimb() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "ragdollforce" ) == 0 ) { t.e=LuaMessageIndex(); t.v=LuaMessageInt() ; entity_lua_ragdollforce() ; }

		if ( strcmp ( t.luaaction_s.Get() , "scale" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_scale() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setanimation" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_setanimation() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setanimationframes" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setanimationframes() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playanimation" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_playanimation() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "loopanimation" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_loopanimation() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "stopanimation" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_stopanimation() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "movewithanimation" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_movewithanimation(); }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolmanual" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolmanual() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrollimbo" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrollimbo() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolunarmed" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolunarmed() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolarmed" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolarmed() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolfidget" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolfidget() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcharactertowalkrun" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setcharactertowalkrun() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setlockcharacter" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setlockcharacter() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcharactertostrafe" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setcharactertostrafe() ; }
			else if ( strcmp ( t.luaaction_s.Get() , "setcharactervisiondelay" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setcharactervisiondelay() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolducked" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolducked() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "charactercontrolstand" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_charactercontrolstand() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "lookatplayer" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_lookatplayer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatetoplayer" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_rotatetoplayer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatetoplayerwithoffset" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageFloat() ; entity_lua_rotatetoplayerwithoffset() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "rotatetocamera" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_rotatetocamera() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setanimationframe" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setanimationframe() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setanimationspeed" ) == 0 ) { t.e=LuaMessageIndex() ; t.v_f=LuaMessageFloat() ; entity_lua_setanimationspeed() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "collected" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_collected() ; }
		else if (strcmp(t.luaaction_s.Get(), "addplayerweapon") == 0) { t.e = LuaMessageInt(); entity_lua_addplayerweapon(); }
		else if (strcmp(t.luaaction_s.Get(), "changeplayerweapon") == 0) { t.s_s = LuaMessageString(); entity_lua_changeplayerweapon(); }
		else if (strcmp(t.luaaction_s.Get(), "changeplayerweaponid") == 0) { t.v = LuaMessageInt(); entity_lua_changeplayerweaponid(); }
		else if ( strcmp ( t.luaaction_s.Get() , "replaceplayerweapon" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_replaceplayerweapon() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "addplayerammo" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_addplayerammo() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "addplayerhealth" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_addplayerhealth() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "addplayerjetpack" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_addplayerjetpack() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setplayerpower" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setplayerpower() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "addplayerpower" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_addplayerpower() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "checkpoint" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_checkpoint() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playsoundifsilent" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playsoundifsilent() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playnon3dsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playnon3Dsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "loopnon3dsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_loopnon3Dsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_setsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "loopsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_loopsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "stopsound" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_stopsound() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsoundspeed" ) == 0 ) { t.v=LuaMessageInt() ; entity_lua_setsoundspeed() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setsoundvolume" ) == 0 ) { t.v=LuaMessageInt() ; entity_lua_setsoundvolume() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playvideo" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playvideonoskip(0,0) ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playvideonoskip" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playvideonoskip(0,1) ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptvideo" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playvideonoskip(1,0) ; }
		else if ( strcmp ( t.luaaction_s.Get() , "promptvideonoskip" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_playvideonoskip(1,1) ; }
		else if ( strcmp ( t.luaaction_s.Get() , "stopvideo" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_stopvideo() ; }
		else if ( strcmp ( t.luaaction_s.Get(), "fireweaponinstant") == 0) { t.e = LuaMessageInt(); entity_lua_fireweapon(true); }
		else if ( strcmp ( t.luaaction_s.Get() , "fireweapon" ) == 0 ) { t.e=LuaMessageInt() ; entity_lua_fireweapon() ; }

		//C++ - had to split here, too many else if for the compiler!
		if ( strcmp ( t.luaaction_s.Get() , "hurtplayer" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_hurtplayer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "drownplayer" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_drownplayer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "switchscript" ) == 0 ) { t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; entity_lua_switchscript() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcharactersoundset" ) == 0 ) { t.e=LuaMessageInt() ; character_soundset() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setcharactersound" ) == 0 ) { t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; character_sound_load() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "playcharactersound" ) == 0 ) { t.e=LuaMessageIndex() ; t.s_s=LuaMessageString() ; character_sound_play() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setnogravity" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_set_gravity() ; }
 
		else if ( strcmp ( t.luaaction_s.Get() , "setlightvisible" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; entity_lua_set_light_visible() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "loadimages" ) == 0 ) { t.v=LuaMessageIndex() ; t.s_s=LuaMessageString() ; lua_loadimages() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setimagepositionx" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setimagepositionx() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setimagepositiony" ) == 0 ) { t.v_f=LuaMessageFloat() ; lua_setimagepositiony() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "showimage" ) == 0 ) { t.v=LuaMessageInt() ; lua_showimage() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "hideimage" ) == 0 ) { t.v=LuaMessageInt() ; lua_hideimage() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setimagealignment" ) == 0 ) { t.v=LuaMessageInt() ; lua_setimagealignment() ; }
 
		else if ( strcmp ( t.luaaction_s.Get() , "textx" ) == 0 ) { t.luaText.x  =  LuaMessageFloat() ; t.tluaTextCenterX  =  0 ; }
		else if ( strcmp ( t.luaaction_s.Get() , "texty" ) == 0 ) { t.luaText.y  =  LuaMessageFloat() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "textsize" ) == 0 ) { t.luaText.size  =  LuaMessageInt() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "textcenterx" ) == 0 ) { t.tluaTextCenterX  =  1 ; }
		else if ( strcmp ( t.luaaction_s.Get() , "textred" ) == 0 ) { g.mp.steamColorRed  =  LuaMessageInt() ; g.mp.steamDoColorText  =  1 ; }
		else if ( strcmp ( t.luaaction_s.Get() , "textgreen" ) == 0 ) { g.mp.steamColorGreen  =  LuaMessageInt() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "textblue" ) == 0 ) { g.mp.steamColorBlue  =  LuaMessageInt() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "texttxt" ) == 0 ) { t.luaText.txt = LuaMessageString() ; lua_text() ; }

		else if ( strcmp ( t.luaaction_s.Get() , "nameplatesoff" ) == 0 ) { g.mp.nameplatesOff = 1 ; }
		else if ( strcmp ( t.luaaction_s.Get() , "nameplateson" ) == 0 ) { g.mp.nameplatesOff = 0 ; }

		else if ( strcmp ( t.luaaction_s.Get() , "panelx" ) == 0 ) { t.luaPanel.x  =  LuaMessageFloat() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "panely" ) == 0 ) { t.luaPanel.y  =  LuaMessageFloat() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "panelx2" ) == 0 ) { t.luaPanel.x2  =  LuaMessageFloat() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "panely2" ) == 0 ) { t.luaPanel.y2 = LuaMessageFloat(); lua_panel() ; }
 

		else if ( strcmp ( t.luaaction_s.Get() , "mpgamemode" ) == 0 ) { t.v=LuaMessageInt() ; mp_serverSetLuaGameMode() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setservertimer" ) == 0 ) { t.v=LuaMessageInt() ; mp_setServerTimer() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "serverrespawnall" ) == 0 ) { mp_serverRespawnAll() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "serverendplay" ) == 0 ) { mp_serverEndPlay() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setserverkillstowin" ) == 0 ) { mp_setServerKillsToWin() ; }
 
		else if ( strcmp ( t.luaaction_s.Get() , "mp_aimovetox" ) == 0 ) { t.e  =  LuaMessageIndex() ; t.tSteamX_f  =  LuaMessageFloat() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "mp_aimovetoz" ) == 0 ) { t.e=LuaMessageIndex() ; t.tSteamZ_f=LuaMessageFloat() ; mp_COOP_aiMoveTo() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setskyto") == 0 ) { t.s_s = LuaMessageString(); lua_set_sky(); }

		// 020216 - TITLE/MENU/PAGE LUA COMMANDS
		if ( strcmp ( t.luaaction_s.Get() , "startgame" ) == 0 ) { lua_startgame(); }
		else if ( strcmp ( t.luaaction_s.Get() , "loadgame" ) == 0 ) { lua_loadgame(); }
		else if ( strcmp ( t.luaaction_s.Get() , "savegame" ) == 0 ) { lua_savegame(); }
		else if ( strcmp ( t.luaaction_s.Get() , "quitgame" ) == 0 ) { lua_quitgame(); }
		else if ( strcmp ( t.luaaction_s.Get() , "leavegame" ) == 0 ) { lua_leavegame(); }
		else if ( strcmp ( t.luaaction_s.Get() , "resumegame" ) == 0 ) { lua_resumegame(); }		
		else if ( strcmp ( t.luaaction_s.Get() , "switchpage" ) == 0 ) { t.s_s=LuaMessageString(); lua_switchpage(); }
		else if ( strcmp ( t.luaaction_s.Get() , "switchpageback" ) == 0 ) { lua_switchpageback(); }
		else if ( strcmp ( t.luaaction_s.Get() , "levelfilenametoload" ) == 0 ) { t.s_s=LuaMessageString(); lua_levelfilenametoload(); }
		else if ( strcmp ( t.luaaction_s.Get() , "triggerfadein" ) == 0 ) { lua_triggerfadein(); }	

		else if ( strcmp ( t.luaaction_s.Get() , "setgamequality" ) == 0 ) { t.v=LuaMessageInt() ; lua_setgamequality() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setplayerfov" ) == 0 ) { t.v=LuaMessageInt() ; lua_setplayerfov() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setgamesoundvolume" ) == 0 ) { t.v=LuaMessageInt() ; lua_setgamesoundvolume() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setgamemusicvolume" ) == 0 ) { t.v=LuaMessageInt() ; lua_setgamemusicvolume() ; }
		else if ( strcmp ( t.luaaction_s.Get() , "setloadingresource" ) == 0 ) { t.e=LuaMessageIndex() ; t.v=LuaMessageInt() ; lua_setloadingresource() ; }
		
	}

	// update engine global at end of all LUA activity this cycle
	g.projectileEventType_explosion = LuaGetInt("g_projectileevent_explosion");
}

void lua_loop ( void )
{
	lua_loop_begin();
	lua_loop_allentities();
	lua_loop_finish();
}

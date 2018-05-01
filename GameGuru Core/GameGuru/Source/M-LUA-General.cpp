//----------------------------------------------------
//--- GAMEGURU - M-LUA-General
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Externals
extern UINT g_StereoEyeToggle;

// 
//  LUA General Commands
// 

void lua_prompt ( void )
{
	if ( g.gvrmode > 0 )
	{
		// use VR prompt instead
		lua_prompt3d ( t.s_s.Get(), Timer() );
	}
	else
	{
		t.luaglobal.scriptprompttime=Timer();
		t.luaglobal.scriptprompt_s=t.s_s;
	}
}

void lua_promptduration ( void )
{
	if ( g.gvrmode > 0 )
	{
		// use VR prompt instead
		lua_prompt3d ( t.s_s.Get(), Timer()+t.v );
	}
	else
	{
		t.luaglobal.scriptprompttime=Timer()+t.v;
		t.luaglobal.scriptprompt_s=t.s_s;
	}
}

void lua_prompttextsize ( void )
{
	t.luaglobal.scriptprompttextsize=t.v;
}

void lua_promptlocalcore ( int iTrueLocalOrForVR )
{
	if ( g.gvrmode > 0 )
	{
		// use VR prompt instead
		lua_prompt3d ( t.s_s.Get(), Timer()+1000 );
		float fObjCtrX = GetObjectCollisionCenterX(t.entityelement[t.e].obj);
		float fObjCtrZ = GetObjectCollisionCenterZ(t.entityelement[t.e].obj);
		float fObjHeight = ObjectSizeY(t.entityelement[t.e].obj);
		float fObjAngle = ObjectAngleY(t.entityelement[t.e].obj);
		if ( iTrueLocalOrForVR > 0 )
		{
			// if PromptLocalForVR mode 1 used, always face player
			float fDX = t.entityelement[t.e].x - CameraPositionX(0);
			float fDZ = t.entityelement[t.e].z - CameraPositionZ(0);
			float fDA = atan2deg ( fDX, fDZ );
			fObjAngle = fDA;

			// if PromptLocalForVR mode 2 used, also elevate text toi above entity (for characters)
			if ( iTrueLocalOrForVR == 2 )
			{
				fObjHeight = fObjHeight * 1.8f;
			}
		}
		lua_positionprompt3d ( t.entityelement[t.e].x+fObjCtrX, t.entityelement[t.e].y+(fObjHeight/2.0f), t.entityelement[t.e].z+fObjCtrZ, fObjAngle );
	}
	else
	{
		if ( iTrueLocalOrForVR == 1 )
		{
			t.luaglobal.scriptprompttime=Timer()+t.v;
			t.luaglobal.scriptprompt_s=t.s_s;
		}
		else
		{
			t.entityelement[t.e].overprompt_s=t.s_s;
			t.entityelement[t.e].overprompttimer=Timer()+1000;
		}
	}
}

void lua_promptlocal ( void )
{
	lua_promptlocalcore ( 0 );
}

void lua_promptlocalforvrmode ( void )
{
	t.promptlocalforvrmode = t.v_f;
}

void lua_promptlocalforvr ( void )
{
	lua_promptlocalcore ( t.promptlocalforvrmode );
}

void lua_text ( void )
{
	if (  t.tluarealcoords  ==  0 ) 
	{
		if (  t.luaText.x  ==  -1 ) 
		{
			t.luaText.x = (GetDisplayWidth()-getbitmapfontwidth(t.luaText.txt.Get(),t.luaText.size))/2;
		}
		else
		{
			if (  t.tluaTextCenterX  ==  0 ) 
			{
				t.luaText.x = (t.luaText.x * GetDisplayWidth()) / 100.0;
			}
			else
			{
				t.luaText.x = ((t.luaText.x * GetDisplayWidth()) / 100.0) - (getbitmapfontwidth(t.luaText.txt.Get(),t.luaText.size)/2);
			}
		}
		t.tluaTextCenterX = 0;
		t.luaText.y = (t.luaText.y * GetDisplayHeight()) / 100.0;
		if (  t.luaText.size  ==  1  )  t.luaText.y  =  t.luaText.y - 18;
		if (  t.luaText.size  ==  2  )  t.luaText.y  =  t.luaText.y - 10;
		if (  t.luaText.size  ==  3  )  t.luaText.y  =  t.luaText.y - 21;
	}
	t.tluarealcoords = 0;

	if (  t.steamDoDropShadow  ==  1 ) 
	{
		t.steamDoDropShadow = 0;
		pastebitmapfontcolor(t.luaText.txt.Get(),t.luaText.x+2,t.luaText.y+2,t.luaText.size,255,0,0,0);
	}
	if (  g.steamworks.steamDoColorText  ==  1 ) 
	{
		g.steamworks.steamDoColorText = 0;
		pastebitmapfontcolor(t.luaText.txt.Get(),t.luaText.x,t.luaText.y,t.luaText.size,t.tluatextalpha,g.steamworks.steamColorRed,g.steamworks.steamColorGreen,g.steamworks.steamColorBlue);
	}
	else
	{
		pastebitmapfont(t.luaText.txt.Get(),t.luaText.x,t.luaText.y,t.luaText.size,255);
	}
	t.tluatextalpha = 255;
}

///

void lua_prompt3d ( LPSTR pTextToRender, DWORD dwPrompt3DTime )
{
	// set prompt 3D - only regenerate if text changes
	t.luaglobal.scriptprompt3dtime = dwPrompt3DTime;
	if ( strcmp ( pTextToRender, t.luaglobal.scriptprompt3dtext ) != NULL && g_StereoEyeToggle == 0 )
	{
		strcpy ( t.luaglobal.scriptprompt3dtext, pTextToRender );
		t.luaglobal.scriptprompt3dX = 0.0f;
		t.luaglobal.scriptprompt3dY = 0.0f;
		t.luaglobal.scriptprompt3dZ = 0.0f;
		t.luaglobal.scriptprompt3dAY = 0.0f;

		// create 3d object for text
		if ( ObjectExist(g.prompt3dobjectoffset)==0 )
		{
			MakeObjectPlane ( g.prompt3dobjectoffset, 512/5.0f, 32.0f/5.0f );
			PositionObject ( g.prompt3dobjectoffset, -100000, -100000, -100000 );
			SetObjectEffect ( g.prompt3dobjectoffset, g.guishadereffectindex );
			DisableObjectZDepth ( g.prompt3dobjectoffset );
			DisableObjectZRead ( g.prompt3dobjectoffset );
			SetObjectMask ( g.prompt3dobjectoffset, 1 );
			SetSphereRadius ( g.prompt3dobjectoffset, 0 );
		}

		// render text to texture
		if ( BitmapExist(2) == 0 ) CreateBitmap ( 2, 1024, 1024 );
		SetCurrentBitmap ( 2 );
		CLS ( Rgb(64,64,64) );
		if ( g.gvrmode > 0 )
			pastebitmapfontcenter ( pTextToRender, 512, 0, 4, 255);
		else
			pastebitmapfontcenter ( pTextToRender, 256, 0, 4, 255);
		GrabImage ( g.prompt3dimageoffset, 0, 0, 512, 64, 3 );
		SetCurrentBitmap ( 0 );

		// apply to object
		TextureObject ( g.prompt3dobjectoffset, 0, g.prompt3dimageoffset );
	}
}

void lua_positionprompt3d ( float fX, float fY, float fZ, float fAngleY )
{
	t.luaglobal.scriptprompt3dX = fX;
	t.luaglobal.scriptprompt3dY = fY;
	t.luaglobal.scriptprompt3dZ = fZ;
	t.luaglobal.scriptprompt3dAY = fAngleY;
	lua_updateprompt3d();
}

void lua_updateprompt3d ( void )
{
	// position 3d prompt and face camera
	float fX = t.luaglobal.scriptprompt3dX;
	float fY = t.luaglobal.scriptprompt3dY;
	float fZ = t.luaglobal.scriptprompt3dZ;
	float fA = t.luaglobal.scriptprompt3dAY;
	if ( fX == 0.0f && fY == 0.0f && fZ == 0.0f )
	{
		// no coordinates so show in front of user
		MoveCameraDown ( 0, 18.0f );
		MoveCamera ( 0, 50.0f );
		fX = CameraPositionX(0);
		fY = CameraPositionY(0);
		fZ = CameraPositionZ(0);
		fA = CameraAngleY(0);
		MoveCamera ( 0, -50.0f );
		MoveCameraDown ( 0, -18.0f );
	}
	if ( ObjectExist( g.prompt3dobjectoffset ) == 1 )
	{
		PositionObject ( g.prompt3dobjectoffset, fX, fY, fZ );
		PointObject ( g.prompt3dobjectoffset, ObjectPositionX(t.aisystem.objectstartindex), ObjectPositionY(t.aisystem.objectstartindex)+35.0f, ObjectPositionZ(t.aisystem.objectstartindex) );
		MoveObject ( g.prompt3dobjectoffset, 15.0f );
		RotateObject ( g.prompt3dobjectoffset, 0, fA+180.0f, 0 );
		ShowObject ( g.prompt3dobjectoffset );
	}
}

void lua_hideprompt3d ( void )
{
	if ( ObjectExist( g.prompt3dobjectoffset ) == 1 ) HideObject ( g.prompt3dobjectoffset );
	strcpy ( t.luaglobal.scriptprompt3dtext, "" );
	t.luaglobal.scriptprompt3dtime = 0;
}

void lua_freeprompt3d ( void )
{
	if ( ObjectExist( g.prompt3dobjectoffset ) == 1 )
	{
		DeleteObject ( g.prompt3dobjectoffset );
	}
}

///

void lua_setfognearest ( void )
{
	t.visuals.FogNearest_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setfogdistance ( void )
{
	t.visuals.FogDistance_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setfogred ( void )
{
	t.visuals.FogR_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setfoggreen ( void )
{
	t.visuals.FogG_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setfogblue ( void )
{
	t.visuals.FogB_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setfogintensity ( void )
{
	t.visuals.FogA_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setambienceintensity ( void )
{
	t.visuals.AmbienceIntensity_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setambiencered ( void )
{
	t.visuals.AmbienceRed_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setambiencegreen ( void )
{
	t.visuals.AmbienceGeen_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setambienceblue ( void )
{
	t.visuals.AmbienceBlue_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setsurfaceintensity ( void )
{
	t.visuals.SurfaceIntensity_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setsurfacered ( void )
{
	t.visuals.SurfaceRed_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setsurfacegreen ( void )
{
	t.visuals.SurfaceGreen_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setsurfaceblue ( void )
{
	t.visuals.SurfaceBlue_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setsurfacesunfactor ( void )
{
	t.visuals.SurfaceSunFactor_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setglobalspecular ( void )
{
	t.visuals.Specular_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setbrightness ( void )
{
	t.visuals.PostBrightness_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setconstrast ( void )
{
	t.visuals.PostContrast_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostbloom ( void )
{
	t.visuals.bloommode=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostvignetteradius ( void )
{
	t.visuals.VignetteRadius_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostvignetteintensity ( void )
{
	t.visuals.VignetteIntensity_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostmotiondistance ( void )
{
	t.visuals.MotionDistance_f=t.v_f;
	visuals_CheckSetGlobalDepthSkipSystem();
	visuals_justshaderupdate ( );
}

void lua_setpostmotionintensity ( void )
{
	t.visuals.MotionIntensity_f=t.v_f;
	visuals_CheckSetGlobalDepthSkipSystem();
	visuals_justshaderupdate ( );
}

void lua_setpostdepthoffielddistance ( void )
{
	t.visuals.DepthOfFieldDistance_f=t.v_f;
	visuals_CheckSetGlobalDepthSkipSystem();
	visuals_justshaderupdate ( );
}

void lua_setpostdepthoffieldintensity ( void )
{
	t.visuals.DepthOfFieldIntensity_f=t.v_f;
	visuals_CheckSetGlobalDepthSkipSystem();
	visuals_justshaderupdate ( );
}

void lua_setpostlightraylength ( void )
{
	t.visuals.LightrayLength_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostlightrayquality ( void )
{
	t.visuals.LightrayQuality_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostlightraydecay ( void )
{
	t.visuals.LightrayDecay_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostsaoradius ( void )
{
	t.visuals.SAORadius_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostsaointensity ( void )
{
	t.visuals.SAOIntensity_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setpostlensflareintensity ( void )
{
	t.visuals.LensFlare_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setoptionreflection ( void )
{
	t.visuals.reflectionmode=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setoptionshadows ( void )
{
	t.visuals.shadowmode=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setoptionlightrays ( void )
{
	t.visuals.lightraymode=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setoptionvegetation ( void )
{
	t.visuals.vegetationmode=t.v_f;
	t.visuals.refreshvegetation = 1;
	visuals_justshaderupdate ( );
}

void lua_setoptionocclusion ( void )
{
	t.visuals.occlusionvalue=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setcameradistance ( void )
{
	t.visuals.CameraFAR_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setcamerafov ( void )
{
	t.visuals.CameraFOV_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setcamerazoompercentage ( void )
{
	t.visuals.CameraFOVZoomed_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setcameraweaponfov ( void )
{
	t.visuals.WeaponFOV_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setterrainlodnear ( void )
{
	t.visuals.TerrainLOD1_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setterrainlodmid ( void )
{
	t.visuals.TerrainLOD2_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setterrainlodfar ( void )
{
	t.visuals.TerrainLOD3_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setterrainsize ( void )
{
	t.visuals.TerrainSize_f=t.v_f;
	visuals_justshaderupdate ( );
}

void lua_setvegetationquantity ( void )
{
	t.visuals.VegQuantity_f=t.v_f;
	t.visuals.refreshvegetation = 1;
	visuals_justshaderupdate ( );
}

void lua_setvegetationwidth ( void )
{
	t.visuals.VegWidth_f=t.v_f;
	t.visuals.refreshvegetation = 1;
	visuals_justshaderupdate ( );
}

void lua_setvegetationheight ( void )
{
	t.visuals.VegHeight_f=t.v_f;
	t.visuals.refreshvegetation = 1;
	visuals_justshaderupdate ( );
}

///

void lua_jumptolevel ( void )
{
	t.tleveltojump_s="";
	if (  Len(t.entityelement[t.e].eleprof.ifused_s.Get())>1 ) 
	{
		t.tleveltojump_s=Lower(t.entityelement[t.e].eleprof.ifused_s.Get());
		game_jump_to_level_from_lua ( );
	}
	else
	{
		if (  Len(t.s_s.Get())>1 ) 
		{
			t.tleveltojump_s=t.s_s;
			game_jump_to_level_from_lua ( );
		}
	}
	if (  t.tleveltojump_s == "" ) 
	{
		game_finish_level_from_lua ( );
	}
}

void lua_finishlevel ( void )
{
	game_finish_level_from_lua ( );
}

void lua_hideterrain ( void )
{
	t.hardwareinfoglobals.noterrain=1;
}

void lua_showterrain ( void )
{
	t.hardwareinfoglobals.noterrain=0;
}

void lua_hidewater ( void )
{
	t.hardwareinfoglobals.nowater=1;
	if (  ObjectExist(t.terrain.objectstartindex+5) == 1  )  HideObject (  t.terrain.objectstartindex+5 );
	ODESetDynamicCharacterController (  t.aisystem.objectstartindex,-10000,0,0,0,0,0,0 );
}

void lua_showwater ( void )
{
	t.hardwareinfoglobals.nowater=0;
	if (  ObjectExist(t.terrain.objectstartindex+5) == 1  )  ShowObject (  t.terrain.objectstartindex+5 );
	ODESetDynamicCharacterController (  t.aisystem.objectstartindex,480,0,0,0,0,0,0 );
return;

}

void lua_hidehuds ( void )
{
	g.tabmodehidehuds=1;
}

void lua_showhuds ( void )
{
	g.tabmodehidehuds=0;
}

void lua_freezeai ( void )
{
	t.aisystem.processlogic=0;
}

void lua_unfreezeai ( void )
{
	t.aisystem.processlogic=1;
}

void lua_freezeplayer ( void )
{
	// skip player logic (freezing it)
	t.aisystem.processplayerlogic=0;

	// Store current position to put ensure the player ends up back here after
	t.freezeplayerposonly = 0;
	if ( ObjectExist(t.aisystem.objectstartindex)==1 )
	{
		t.freezeplayerx = ObjectPositionX(t.aisystem.objectstartindex);
		t.freezeplayery = ObjectPositionY(t.aisystem.objectstartindex);
		t.freezeplayerz = ObjectPositionZ(t.aisystem.objectstartindex);
	}
	else
	{
		t.freezeplayerx = CameraPositionX(0);
		t.freezeplayery = CameraPositionY(0);
		t.freezeplayerz = CameraPositionZ(0);
	}
	t.freezeplayerax = CameraAngleX(0);
	t.freezeplayeray = CameraAngleY(0);
	t.freezeplayeraz = CameraAngleZ(0);
	t.freezeplayerspeed = 0;

	// freeze any animation of third person when player frozen
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		int iPlayerObjID = t.entityelement[t.playercontrol.thirdperson.charactere].obj;
		if ( iPlayerObjID > 0 )
		{
			if ( ObjectExist(iPlayerObjID)==1 )
			{
				sObject* pObject = g_ObjectList [ iPlayerObjID ];
				t.freezeplayerspeed = pObject->fAnimSpeed*100.00001f;
				SetObjectSpeed ( iPlayerObjID, 0 );
			}
		}
	}
}

void lua_unfreezeplayer ( void )
{
	// allow regular player control
	t.aisystem.processplayerlogic=1;

	//Place player back at the position they were at when lua_freezeplayer was called
	t.terrain.playerx_f = t.freezeplayerx;
	t.terrain.playery_f = t.freezeplayery;
	t.terrain.playerz_f = t.freezeplayerz;
	t.terrain.playerax_f = t.freezeplayerax;
	t.terrain.playeray_f = t.freezeplayeray;
	t.terrain.playeraz_f = t.freezeplayeraz;

	// Restart character to ensure they don't move
	physics_disableplayer ( );
	physics_setupplayer ( );

	// Deal with third person if needed
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		// ensure character protagonist is respawned
		t.e=t.playercontrol.thirdperson.charactere;
		entity_lua_spawn_core ( );
		physics_player_thirdpersonreset ( );
	}

	// resume any animation speed of third person when player unfrozen
	if (  t.playercontrol.thirdperson.enabled == 1 ) 
	{
		int iPlayerObjID = t.entityelement[t.playercontrol.thirdperson.charactere].obj;
		if ( iPlayerObjID > 0 )
		{
			if ( ObjectExist(iPlayerObjID)==1 )
			{
				SetObjectSpeed ( iPlayerObjID, t.freezeplayerspeed );
			}
		}
	}
}

void lua_setfreezepositionx ( void ) { t.freezeplayerx = t.v_f; }
void lua_setfreezepositiony ( void ) { t.freezeplayery = t.v_f; }
void lua_setfreezepositionz ( void ) { t.freezeplayerz = t.v_f; }
void lua_setfreezepositionax ( void ) { t.freezeplayerax = t.v_f; }
void lua_setfreezepositionay ( void ) { t.freezeplayeray = t.v_f; }
void lua_setfreezepositionaz ( void ) { t.freezeplayeraz = t.v_f; }
void lua_transporttofreezeposition ( void )
{
	// t.v = 1 = position only, leave camera and player rotation unaffected
	t.freezeplayerposonly = t.v;
	physics_disableplayer ( );
	t.terrain.playerx_f = t.freezeplayerx;
	t.terrain.playery_f = t.freezeplayery;
	t.terrain.playerz_f = t.freezeplayerz;
	if ( t.freezeplayerposonly==0 )
	{
		t.terrain.playerax_f = t.freezeplayerax;
		t.terrain.playeray_f = t.freezeplayeray;
		t.terrain.playeraz_f = t.freezeplayeraz;
	}
	physics_setupplayer ( );
	if ( t.freezeplayerposonly==0 )
	{
		t.camangx_f = t.freezeplayerax;
		t.camangy_f = t.freezeplayeray;
		t.playercontrol.finalcameraangley_f = t.camangy_f;
	}
	t.freezeplayerposonly = 0;
}

void lua_activatemouse ( void )
{
	g.luaactivatemouse = 1;
}

void lua_deactivatemouse ( void )
{
	g.luaactivatemouse = 0;
}

void lua_setplayerhealth ( void )
{
	t.player[t.plrid].health=t.v;
}

void lua_setplayerlives ( void )
{
	t.player[t.plrid].lives=t.v;
}

void lua_removeplayerweapons ( void )
{
	//  put away weapon if held
	g.autoloadgun=0;
	//  clear all weapons and ammo
	for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
	{
		t.weaponslot[t.ws].got=0;
		t.weaponammo[t.ws]=0;
		t.tgunid=t.weaponslot[t.ws].pref;
		if (  t.tgunid>0 ) 
		{
			t.tpool=g.firemodes[t.tgunid][0].settings.poolindex;
			t.altpool=g.firemodes[t.tgunid][1].settings.poolindex;
			if (  t.tpool == 0  )  t.weaponclipammo[t.ws] = 0; else t.ammopool[t.tpool].ammo = 0;
			if (  t.altpool == 0  )  t.weaponclipammo[t.ws+10] = 0; else t.ammopool[t.altpool].ammo = 0;
		}
	}
	//  some common resets (as often called when want to reset player)
	t.playercontrol.camerashake_f=0;
	t.playercontrol.flinchx_f=0;
	t.playercontrol.flinchy_f=0;
	t.playercontrol.flinchz_f=0;
	t.playercontrol.pushforce_f=0;
return;

// 
//  LUA Music Commands
// 

}

void lua_disablemusicreset ( void )
{
	t.playercontrol.disablemusicreset=t.v;
}

void lua_musicload ( void )
{
	//  load a music track. Takes m and s$
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	t.tFilePath_s = cstr(GetDir()) + cstr("\\") + t.s_s;
	if (  FileExist(t.tFilePath_s.Get())  ==  0  )  return;

	//  delete current sound if it exists
	t.tSndID = g.musicsoundoffset + t.m - 1;
	if (  SoundExist(t.tSndID)  ==  1 ) 
	{
		StopSound (  t.tSndID );
		DeleteSound (  t.tSndID );
	}

	//  setup new sound
	LoadSound (  t.tFilePath_s.Get(),t.tSndID );
	music_resettrack(t.m);
	t.musictrack[t.m].interval = 0;
	t.musictrack[t.m].length = 0;
}

void lua_musicsetinterval ( void )
{

	//  set the interval v for music m
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	t.musictrack[t.m].interval = t.v;
return;

}

void lua_musicsetlength ( void )
{

	//  set the length v for music m
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	t.musictrack[t.m].length = t.v;
return;

}

void lua_musicstop ( void )
{

	//  stops any music currently playing
	music_stopall(g.musicsystem.fadetime);

return;

}

void lua_musicplayinstant ( void )
{
	//  stop any current music and play m, fading in over period musicsystem.fadetime
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  SoundExist(g.musicsoundoffset + t.m - 1)  ==  0  )  return;
	if (  t.musictrack[t.m].state  ==  1  )  return;
	music_stopall(0);
	music_play(t.m,g.musicsystem.fadetime);
}

void lua_musicplayfade ( void )
{

	//  fade out any playing music and fade in m over period musicsystem.fadetime
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  SoundExist(g.musicsoundoffset + t.m - 1)  ==  0  )  return;
	if (  t.musictrack[t.m].state  ==  1  )  return;
	music_stopall(g.musicsystem.fadetime*3);
	music_play(t.m,g.musicsystem.fadetime);

return;

}

void lua_musicplaycue ( void )
{

	//  find the currently playing track and tell it to stop and play m at it's next interval
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  SoundExist(g.musicsoundoffset + t.m - 1)  ==  0  )  return;
	if (  t.musictrack[t.m].state  ==  1  )  return;
	for ( t.tSearch = 1 ; t.tSearch<=  MUSICSYSTEM_MAXTRACKS; t.tSearch++ )
	{
		if (  t.musictrack[t.tSearch].state  ==  1 ) 
		{
			t.musictrack[t.tSearch].cuetrack = t.m;
			t.musictrack[t.tSearch].cuefade = g.musicsystem.fadetime;
		}
	}

return;

}

void lua_musicplaytime ( void )
{

	//  plays the track m for v amount of time, before returning to the default track
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  SoundExist(g.musicsoundoffset + t.m - 1)  ==  0  )  return;
	t.musictrack[t.m].playtimestamp = Timer();
	t.musictrack[t.m].playtime = t.v;
	t.musictrack[t.m].playtimefade = g.musicsystem.fadetime;

	//  if already playing, updating the timing data was enough
	if (  t.musictrack[t.m].state  ==  1  )  return;
	lua_musicplayfade ( );
return;

}

void lua_musicplaytimecue ( void )
{

	//  plays the track m for v amount of time, before returning to the default track, using the timing interval system
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	if (  SoundExist(g.musicsoundoffset + t.m - 1)  ==  0  )  return;
	t.musictrack[t.m].playtimestamp = Timer();
	t.musictrack[t.m].playtime = t.v;
	t.musictrack[t.m].playtimefade = g.musicsystem.fadetime;

	//  if not playing, set it to play
	if ( t.musictrack[t.m].state != 1 ) lua_musicplaycue ( );

	//  our track is now cued to play, but we need to cue the default track to come in later
	t.musictrack[t.m].cuetrack = g.musicsystem.default;
	if (  g.musicsystem.default > 0 ) 
	{
		t.musictrack[t.m].cuefade = g.musicsystem.fadetime;
	}

return;

}

void lua_musicsetvolume ( void )
{

	//  sets the global music volume to v and tells the playing track to fade to v in time musicsystem.fadetime
	if (  t.v < 0  )  t.v  =  0;
	if (  t.v > 100  )  t.v  =  100;
	if (  t.v  ==  g.musicsystem.percentageVolume  )  return;
	g.musicsystem.percentageVolume = t.v;
	music_fadeAllMusicToNewVolume ( );

return;

}

void lua_musicsetdefault ( void )
{

	//  sets the default music track m to return to after another track is played for a finite amount of time
	if (  t.m < 1 || t.m > MUSICSYSTEM_MAXTRACKS  )  return;
	g.musicsystem.default = t.m;

return;

}

void lua_musicsetfadetime ( void )
{

	//  sets the global fadetime for all subsequent operations that use a fade time
	if (  t.v < 0  )  t.v  =  0;
	g.musicsystem.fadetime = t.v;

return;

// 
//  Particle commands
// 

}

void lua_startparticleemitter ( void )
{
	//  takes e
	if (  t.entityelement[t.e].particleemitterid == 0 ) 
	{
		ravey_particles_get_free_emitter ( );
		if (  t.tResult>0 ) 
		{
			t.entityelement[t.e].particleemitterid=t.tResult;
			g.tEmitter.id = t.tResult;
			g.tEmitter.emitterLife = 0;
			g.tEmitter.parentObject = t.entityelement[t.e].obj;
			g.tEmitter.parentLimb = 0;
			g.tEmitter.isAnObjectEmitter = 0;
			g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
			g.tEmitter.isAnimated = 1;
			g.tEmitter.animationSpeed = 1/64.0;
			g.tEmitter.isLooping = 1;
			g.tEmitter.frameCount = 64;
			g.tEmitter.startFrame = 0;
			g.tEmitter.endFrame = 63;
			g.tEmitter.startsOffRandomAngle = 1;
			g.tEmitter.offsetMinX = -2;
			g.tEmitter.offsetMinY = -2;
			g.tEmitter.offsetMinZ = -2;
			g.tEmitter.offsetMaxX = 2;
			g.tEmitter.offsetMaxY = 2;
			g.tEmitter.offsetMaxZ = 2;
			g.tEmitter.scaleStartMin = 15;
			g.tEmitter.scaleStartMax = 20;
			g.tEmitter.scaleEndMin = 90;
			g.tEmitter.scaleEndMax = 100;
			g.tEmitter.movementSpeedMinX = -0.1f;
			g.tEmitter.movementSpeedMinY = -0.1f;
			g.tEmitter.movementSpeedMinZ = -0.1f;
			g.tEmitter.movementSpeedMaxX = 0.1f;
			g.tEmitter.movementSpeedMaxY = 0.1f;
			g.tEmitter.movementSpeedMaxZ = 0.1f;
			g.tEmitter.rotateSpeedMinZ = -0.1f;
			g.tEmitter.rotateSpeedMaxZ = 0.1f;
			g.tEmitter.startGravity = 0;
			g.tEmitter.endGravity = 0;
			g.tEmitter.lifeMin = 1000;
			g.tEmitter.lifeMax = 2000;
			g.tEmitter.alphaStartMin = 40;
			g.tEmitter.alphaStartMax = 75;
			g.tEmitter.alphaEndMin = 0;
			g.tEmitter.alphaEndMax = 0;
			g.tEmitter.frequency = 25;
			ravey_particles_add_emitter ( );
		}
	}
return;

}

void lua_stopparticleemitter ( void )
{
	//  takes e
	if (  t.entityelement[t.e].particleemitterid>0 ) 
	{
		t.tRaveyParticlesEmitterID=t.entityelement[t.e].particleemitterid;
		ravey_particles_delete_emitter ( );
		t.entityelement[t.e].particleemitterid=0;
	}
return;

// 
//  Prompts and Images
// 

}

void lua_loadimages ( void )
{
	//  takes v,s$
	if (  PathExist( cstr(cstr("scriptbank\\images\\")+t.s_s).Get() ) == 1 ) 
	{
		t.tolddir_s=GetDir();
		SetDir (  cstr(cstr("scriptbank\\images\\")+t.s_s).Get() );
		for ( t.t = 0 ; t.t<=  99; t.t++ )
		{
			t.tfile_s=cstr(Right(Str(1000+t.t),3))+".png";
			if (  FileExist(t.tfile_s.Get()) == 1 ) 
			{
				LoadImage (  t.tfile_s.Get(),g.promptimageimageoffset+t.v+t.t );
			}
			else
			{
				break;
			}
		}
		SetDir (  t.tolddir_s.Get() );
	}
return;

}

void lua_setimagepositionx ( void )
{
	t.promptimage.x=t.v_f;
return;

}

void lua_setimagepositiony ( void )
{
	t.promptimage.y=t.v_f;
return;

}

void lua_setimagealignment ( void )
{
	t.promptimage.alignment=t.v;
return;

}

void lua_showimage ( void )
{
	t.promptimage.img=g.promptimageimageoffset+t.v;
	t.promptimage.show=1;
}

void lua_hideimage ( void )
{
	t.promptimage.show=0;
}

//
// MENU PAGE COMMANDS
//

void lua_startgame ( void )
{
	// START GAME
	t.game.levelloop=1;
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, "" );
}
void lua_loadgame ( void )
{
	// TODO
}
void lua_savegame ( void )
{
	// TODO
}
void lua_quitgame ( void )
{
	//  EXIT TO WINDOWS
	t.game.levelloop=0;
	t.game.masterloop=0;
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, "" );
}
void lua_leavegame ( void )
{
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, "" );
	steam_quitGame ( );
}
void lua_resumegame ( void )
{
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, "" );
}
void lua_switchpage ( void )
{
	// SWITCH TO NEW LUA PAGE
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, t.s_s.Get() );
}
void lua_switchpageback ( void )
{
	t.game.titleloop=0;
	strcpy ( t.game.pSwitchToPage, "-1" );
}
void lua_levelfilenametoload ( void )
{
	strcpy ( t.game.pAdvanceWarningOfLevelFilename, t.s_s.Get() );
}
void lua_triggerfadein ( void )
{
	t.postprocessings.fadeinvalue_f = 0;
}

void lua_setgamequality ( void )
{
	g.titlesettings.graphicsettingslevel = t.v;
	titles_immediateupdategraphics ( );
}
void lua_setplayerfov ( void )
{
	t.visuals.CameraFOV_f=(20+((t.v+0.0)/100.0)*180.0)/t.visuals.CameraASPECT_f;
	t.visuals.refreshshaders=1;
	visuals_loop ( );
}
void lua_setgamesoundvolume ( void )
{
	t.gamesounds.sounds  = t.v;
	titles_immediateupdatesound();
}
void lua_setgamemusicvolume ( void )
{
	t.gamesounds.music  = t.v;
	titles_immediateupdatesound();
}
void lua_setloadingresource ( void )
{
	t.loadingresource[t.e] = t.v;
}

void lua_set_sky(void)
{
	int index = -1;
	for (int i = 0; i < ArrayCount(t.skybank_s); i++) 
	{
		if (t.skybank_s[i].Lower()==t.s_s.Lower()) 
		{
			index = i;
			break;
		}
	}
	if (index != -1) 
	{
		t.visuals.skyindex = index;
		t.visuals.refreshskysettings = 1;
		t.visuals.refreshshaders = 1;
	}
}
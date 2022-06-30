//----------------------------------------------------
//--- GAMEGURU - M-Sky
//----------------------------------------------------
#include "stdafx.h"
#include "gameguru.h"


void sky_init ( void )
{
	// Sky directory
	SetDir ( "skybank" );
	timestampactivity(0, "entering skybank");

	// Assemble list of all skies
	g.skymax=0;
	ChecklistForFiles ( );
	timestampactivity(0, "checking skybank files");

	//PE: We need subfolder support in VRQuest and Classic.
	for (t.c = 1; t.c <= ChecklistQuantity(); t.c++)
	{
		if (ChecklistValueA(t.c) == 1)
		{
			t.file_s = ChecklistString(t.c);
			if (cstr(Left(t.file_s.Get(), 1)) != ".")
			{
				timestampactivity(0, t.file_s.Get());
				++g.skymax;
				Dim(t.skybank_s, g.skymax);
				t.skybank_s[g.skymax] = Lower(t.file_s.Get());
			}
		}
	}

	// Include one sub-folder in for artist skyboxes
	for ( t.s = 1 ; t.s <= g.skymax; t.s++ )
	{
		t.ttry_s = t.skybank_s[t.s]+"\\skyspec.txt";
		if ( FileExist(t.ttry_s.Get()) == 0 ) 
		{
			// set entry to remove
			t.removethisone=t.s;
			// into the folder
			SetDir ( t.skybank_s[t.s].Get() );
			// now populate with any artist skies
			ChecklistForFiles();
			for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
			{
				if ( ChecklistValueA(t.c) == 1 ) 
				{
					t.file_s = ChecklistString(t.c);
					if ( cstr(Left(t.file_s.Get(),1)) != "." ) 
					{
						++g.skymax;
						Dim ( t.skybank_s,g.skymax );
						t.skybank_s[g.skymax]=t.skybank_s[t.s]+"\\"+Lower(t.file_s.Get());
					}
				}
			}
			// out of folder
			SetDir ( ".." );

			// remove this empty folder entry
			for ( t.ss = t.removethisone ; t.ss <= g.skymax-1; t.ss++ )
			{
				t.skybank_s[t.ss]=t.skybank_s[t.ss+1];
			}
			--g.skymax;
		}
	}

	// Restore Dir
	SetDir ( ".." );
	timestampactivity(0, "finished sky bank scan");

	// Default sky settings
	t.sky.currenthour_f=12.0;
	t.sky.daynightprogress=0;
}

void sky_skyspec_init ( void )
{
	// Get lighting data from skyspec
	if ( g.skyindex>0 ) 
	{
		// Load sky spec for light position and color
		t.terrain.skyshader_s="";
		t.terrain.skyscrollshader_s="";
		t.skyname_s=t.skybank_s[g.skyindex];
		t.terrain.sunskyscrollspeedx_f=0.0;
		t.terrain.sunskyscrollspeedz_f=0.0;
		timestampactivity(0,cstr(cstr("Loading skyspec.txt:")+t.skyname_s).Get() );
		OpenToRead ( 1, cstr(cstr("skybank\\")+t.skyname_s+"\\skyspec.txt").Get() );
		Dim ( t.value_f, 3 ); t.valuei=0;
		do
		{
			t.line_s = ReadString(1);
			t.line_s = Lower(t.line_s.Get());

			// common values
			t.try_s = "suncolor";
			if (cstr(Left(t.line_s.Get(), Len(t.try_s.Get()))) == t.try_s)
			{
				terrain_parsed_getvalues();
				t.terrain.suncolorr_f = t.value_f[0];
				t.terrain.suncolorg_f = t.value_f[1];
				t.terrain.suncolorb_f = t.value_f[2];
				t.terrain.suncolorr_f = (t.terrain.suncolorr_f*0.5) + (255 * 0.5);
				t.terrain.suncolorg_f = (t.terrain.suncolorg_f*0.5) + (255 * 0.5);
				t.terrain.suncolorb_f = (t.terrain.suncolorb_f*0.5) + (255 * 0.5);
			}
			t.try_s = "sunstrength";
			if (cstr(Left(t.line_s.Get(), Len(t.try_s.Get()))) == t.try_s)
			{
				terrain_parsed_getvalues();
				t.terrain.sunstrength_f = t.value_f[0];
			}

			t.try_s="sundirection";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s ) 
			{
				terrain_parsed_getvalues ( );
				t.terrain.sundirectionx_f=t.value_f[0]+25;
				t.terrain.sundirectiony_f=t.value_f[1];
				t.terrain.sundirectionz_f=t.value_f[2];
				t.tav_f=abs(t.terrain.sundirectionx_f)+abs(t.terrain.sundirectiony_f)+abs(t.terrain.sundirectionz_f);
				t.terrain.sundirectionx_f=t.terrain.sundirectionx_f/t.tav_f;
				t.terrain.sundirectiony_f=t.terrain.sundirectiony_f/t.tav_f;
				t.terrain.sundirectionz_f=t.terrain.sundirectionz_f/t.tav_f;
				t.terrain.skysundirectionx_f = t.terrain.sundirectionx_f;
				t.terrain.skysundirectiony_f = t.terrain.sundirectiony_f;
				t.terrain.skysundirectionz_f = t.terrain.sundirectionz_f;
			}
			t.try_s="sunrotation";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunrotation_f=t.value_f[0];
			}
			t.try_s="skyscrollx";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunskyscrollspeedx_f=t.value_f[0]/50000.0;
			}
			t.try_s="skyscrollz";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunskyscrollspeedz_f=t.value_f[0]/50000.0;
			}
			t.try_s="skyshader";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getstring ( );
				t.terrain.skyshader_s = t.rest_s;
			}
			t.try_s="skyscrollshader";
			if ( cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getstring ( );
				t.terrain.skyscrollshader_s = t.rest_s;
			}

		} while ( !( t.line_s == "" ) );
		CloseFile ( 1 );
	}

	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

	// If not from editor, also load and create sky assets
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
	//Allow loading of skybox. in editor.
	#else
	if ( t.game.set.ismapeditormode == 0 ) 
	#endif
	{
		// Use sky shader
		cstr useSkyShader_s = "effectbank\\reloaded\\sky_basic.fx";
		cstr useSkyScrollShader_s = "effectbank\\reloaded\\skyscroll_basic.fx";
		if ( t.terrain.skyshader_s != "" ) useSkyShader_s = cstr ( cstr("effectbank\\reloaded\\") + t.terrain.skyshader_s );
		if ( t.terrain.skyscrollshader_s != "" ) useSkyScrollShader_s = cstr ( cstr("effectbank\\reloaded\\") + t.terrain.skyscrollshader_s );

		// In case shader changes, delete any old effects
		if ( GetEffectExist(t.terrain.effectstartindex+4) == 1 ) 
		{
			EraseEffectParameterIndex ( t.terrain.effectstartindex+4, "HudFogDist" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+4, "HudFogColor" );
			DeleteEffect ( t.terrain.effectstartindex+4 );
		}
		if ( GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
		{
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "SkyScrollValues" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "HudFogDist" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "HudFogColor" );
			DeleteEffect ( t.terrain.effectstartindex+9 );
		}

		// Load sky shaders
		if ( GetEffectExist(t.terrain.effectstartindex+4) == 0 ) 
		{
			timestampactivity(0,cstr(cstr("Loading sky shaders:")+useSkyShader_s).Get() );
			LoadEffect ( useSkyShader_s.Get(), t.terrain.effectstartindex+4, 0 );
			t.effectparam.sky.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+4,"HudFogDist");
			t.effectparam.sky.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+4,"HudFogColor");
		}
		if ( GetEffectExist(t.terrain.effectstartindex+9) == 0 ) 
		{
			timestampactivity(0,cstr(cstr("Loading sky shaders:")+useSkyScrollShader_s).Get() );
			LoadEffect ( useSkyScrollShader_s.Get(), t.terrain.effectstartindex+9, 0 );
			t.effectparam.skyscroll.SkyScrollValues=GetEffectParameterIndex(t.terrain.effectstartindex+9,"SkyScrollValues");
			t.effectparam.skyscroll.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+9,"HudFogDist");
			t.effectparam.skyscroll.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+9,"HudFogColor");
		}
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

		// Initialise main and secondary sky
		SetMipmapNum ( 1 );
		timestampactivity(0,cstr(cstr("Initialise sky textures")).Get());
		for ( t.tskyi = 0 ; t.tskyi <= t.terrainskyspecinitmode; t.tskyi++ )
		{
			// sky object
			if ( t.tskyi == 0 ) { t.skyobj = t.terrain.objectstartindex+4  ; t.skyname_s = t.skybank_s[g.skyindex]; }
			if ( t.tskyi == 1 ) { t.skyobj = t.terrain.objectstartindex+8  ; t.skyname_s = "night"; }

			// Load sky spec for light position and color
			if ( t.tskyi == 0 ) 
			{
				// when sky changes, need to change sun feed into gun shaders
				gun_updategunshaders ( );
			}

			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			// skyname only
			t.skynameonly_s=t.skyname_s;
			for ( t.n = Len(t.skyname_s.Get()) ; t.n >= 1 ; t.n+= -1 )
			{
				if ( cstr(Mid(t.skyname_s.Get(),t.n)) == "\\" || cstr(Mid(t.skyname_s.Get(),t.n)) == "/" ) 
				{
					t.skynameonly_s=Right(t.skyname_s.Get(),Len(t.skyname_s.Get())-t.n);
					break;
				}
			}

			// Sky object
			if ( ObjectExist(t.skyobj) == 1 ) DeleteObject ( t.skyobj );
			cstr pFileToLoad = cstr(cstr("skybank\\")+t.skyname_s+"\\"+t.skynameonly_s+".DBO");
			timestampactivity(0,cstr(cstr("Load sky geometry:")+pFileToLoad).Get() );
			if (FileExist(pFileToLoad.Get()) == 0)
			{
				// load X file and save as DBO
				pFileToLoad = cstr(cstr("skybank\\") + t.skyname_s + "\\" + t.skynameonly_s + ".X");
				LoadObject ( pFileToLoad.Get(), t.skyobj );
				if (ObjectExist(t.skyobj) == 1)
				{
					cstr pFileToSave = cstr(cstr("skybank\\") + t.skyname_s + "\\" + t.skynameonly_s + ".dbo");
					SaveObject(pFileToSave.Get(), t.skyobj);
				}
			}
			else
			{
				// load DBO directly (preferred)
				LoadObject ( pFileToLoad.Get(), t.skyobj );
			}

			// Common settings for sky object
			ScaleObject ( t.skyobj,750,750,750 );
			SetObjectTextureMode ( t.skyobj,2,0 );
			SetObjectMask ( t.skyobj,2+1 ); // DX11 actually hidden when lightray renders
			YRotateObject ( t.skyobj,t.terrain.sunrotation_f );
			SetObjectEffect ( t.skyobj,t.terrain.effectstartindex+4 );

			// Fade default
			if ( t.tskyi == 0 ) SetAlphaMappingOn ( t.skyobj,100 );
			if ( t.tskyi == 1 ) SetAlphaMappingOn ( t.skyobj,0 );

			// Transparency mode 7 draws it before all others
			SetObjectTransparency ( t.skyobj,7 );
			DisableObjectZDepth ( t.skyobj );
			DisableObjectZWrite ( t.skyobj );
			DisableObjectZRead ( t.skyobj );
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
		}

		// For MAIN sky, get full name and name only strings (again)
		t.skyname_s=t.skybank_s[g.skyindex];
		t.skynameonly_s=t.skyname_s;
		for ( t.n = Len(t.skyname_s.Get()) ; t.n >=  1 ; t.n+= -1 )
		{
			if ( cstr(Mid(t.skyname_s.Get(),t.n)) == "\\" || cstr(Mid(t.skyname_s.Get(),t.n)) == "/" ) 
			{
				t.skynameonly_s=Right(t.skyname_s.Get(),Len(t.skyname_s.Get())-t.n);
				break;
			}
		}

		// Detect for and load skyscroll plane
		t.skyscroll_s=cstr("skybank\\")+t.skyname_s+"\\"+t.skynameonly_s+"_CLOUDS.dds";
		timestampactivity(0,cstr(cstr("Loading t.skyscroll_s:")+t.skyscroll_s).Get() );
		if ( FileExist(t.skyscroll_s.Get()) == 1 ) 
		{
			if ( ObjectExist(t.terrain.objectstartindex+9) == 1 ) DeleteObject ( t.terrain.objectstartindex+9 );
			if ( ObjectExist(t.terrain.objectstartindex+9) == 0 ) 
			{
				// Create sky scoll object
				MakeObjectPlane ( t.terrain.objectstartindex+9,90000,90000 );
				SetObjectTextureMode ( t.terrain.objectstartindex+9,2,0 );
				SetObjectTransparency ( t.terrain.objectstartindex+9,1 );
				SetObjectCollisionOff ( t.terrain.objectstartindex+9 );
				XRotateObject ( t.terrain.objectstartindex+9,270 );
				SetObjectLight ( t.terrain.objectstartindex+9,0 );
				SetObjectCull ( t.terrain.objectstartindex+9,0 );
				SetObjectFog ( t.terrain.objectstartindex+9,0 );

				// Now create a second UV data layer for static second texture (alphaportal)
				CloneMeshToNewFormat ( t.terrain.objectstartindex+9,512+2 );
				t.q_f=1.0;
				LockVertexDataForLimb ( t.terrain.objectstartindex+9,0 );
				SetVertexDataUV ( 0,1,0,0 );
				SetVertexDataUV ( 0,1,t.q_f,0 );
				SetVertexDataUV ( 1,1,0,0 );
				SetVertexDataUV ( 2,1,t.q_f,t.q_f );
				SetVertexDataUV ( 3,1,0,0 );
				SetVertexDataUV ( 4,1,0,t.q_f );
				SetVertexDataUV ( 5,1,t.q_f,t.q_f );
				UnlockVertexData ( );

				// Apply skyscroll shader (uses second UV data for alpha channel from cloud texture)
				SetObjectEffect ( t.terrain.objectstartindex+9,t.terrain.effectstartindex+9 );
			}
			else
			{
				// Show if hidden
				ShowObject ( t.terrain.objectstartindex+9 );
			}
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			// Always load in specified texture as sky can change
			LoadImage ( t.skyscroll_s.Get(),t.terrain.imagestartindex+9,0,g.gdividetexturesize );
			TextureObject ( t.terrain.objectstartindex+9,0,t.terrain.imagestartindex+9 );

			// Load sky cloud portal
			LoadImage ( "skybank\\cloudportal.dds",t.terrain.imagestartindex+10,0,g.gdividetexturesize );
			TextureObject ( t.terrain.objectstartindex+9,1,t.terrain.imagestartindex+10 );
		}
		else
		{
			// Hide if not used and exists
			if ( ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
			{
				HideObject ( t.terrain.objectstartindex+9 );
			}
		}
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
		SetMipmapNum ( -1 );
	}

	// when change sky, due to sun direction, must update shader constants too
	t.visuals.refreshshaders=1;
}

void sky_hide ( void )
{
	if ( ObjectExist(t.terrain.objectstartindex+4) == 1 ) 
	{
		t.tskyobj1v=GetVisible(t.terrain.objectstartindex+4);
		HideObject ( t.terrain.objectstartindex+4 );
	}
	if ( ObjectExist(t.terrain.objectstartindex+8) == 1 ) 
	{
		t.tskyobj2v=GetVisible(t.terrain.objectstartindex+8);
		HideObject ( t.terrain.objectstartindex+8 );
	}
	if ( ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
	{
		t.tskyobj3v=GetVisible(t.terrain.objectstartindex+9);
		HideObject ( t.terrain.objectstartindex+9 );
	}
}

void sky_show ( void )
{
	if ( ObjectExist(t.terrain.objectstartindex+4) == 1 && t.tskyobj1v == 1 ) ShowObject ( t.terrain.objectstartindex+4 );
	if ( ObjectExist(t.terrain.objectstartindex+8) == 1 && t.tskyobj2v == 1 ) ShowObject ( t.terrain.objectstartindex+8 );
	if ( ObjectExist(t.terrain.objectstartindex+9) == 1 && t.tskyobj3v == 1 ) ShowObject ( t.terrain.objectstartindex+9 );
}

void sky_free ( void )
{
	// free sky objects
	if ( ObjectExist(t.terrain.objectstartindex+4) == 1 ) DeleteObject ( t.terrain.objectstartindex+4 );
	if ( ObjectExist(t.terrain.objectstartindex+8) == 1 ) DeleteObject ( t.terrain.objectstartindex+8 );
	if ( ObjectExist(t.terrain.objectstartindex+9) == 1 ) DeleteObject ( t.terrain.objectstartindex+9 );
}

void sky_loop ( void )
{
	// day
	t.sky.alpha1_f=1.0;
	t.sky.alpha2_f=0;

	// Update skybox position
	if ( t.hardwareinfoglobals.nosky == 0 ) 
	{
		if ( ObjectExist(t.terrain.objectstartindex+4) == 1 ) 
		{
			if ( t.sky.alpha1_f>0 ) 
			{
				PositionObject ( t.terrain.objectstartindex+4,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				SetAlphaMappingOn ( t.terrain.objectstartindex+4,100.0*t.sky.alpha1_f );
				ShowObject ( t.terrain.objectstartindex+4 );
			}
			else
			{
				HideObject ( t.terrain.objectstartindex+4 );
			}
		}
		if ( ObjectExist(t.terrain.objectstartindex+8) == 1 ) 
		{
			if ( t.sky.alpha2_f>0 ) 
			{
				PositionObject ( t.terrain.objectstartindex+8,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				SetAlphaMappingOn ( t.terrain.objectstartindex+8,100.0*t.sky.alpha2_f );
				ShowObject ( t.terrain.objectstartindex+8 );
			}
			else
			{
				HideObject ( t.terrain.objectstartindex+8 );
			}
		}
		if ( ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
		{
			PositionObject ( t.terrain.objectstartindex+9,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera)+7000,CameraPositionZ(t.terrain.gameplaycamera) );
			if ( GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
			{
				t.terrain.sunskyscrollx_f=t.terrain.sunskyscrollx_f+(t.terrain.sunskyscrollspeedx_f*g.timeelapsed_f);
				t.terrain.sunskyscrollz_f=t.terrain.sunskyscrollz_f+(t.terrain.sunskyscrollspeedz_f*g.timeelapsed_f);
				SetVector4 ( g.terrainvectorindex,t.terrain.sunskyscrollx_f,t.terrain.sunskyscrollz_f,0,0 );
				SetEffectConstantVEx ( t.terrain.effectstartindex+9,t.effectparam.skyscroll.SkyScrollValues,g.terrainvectorindex );
			}
		}
	}

	// update sky fog
	if ( GetEffectExist(t.terrain.effectstartindex+4) ) 
	{
		SetVector4 ( g.terrainvectorindex,t.visuals.FogNearest_f,t.visuals.FogDistance_f,0,0 );
		SetEffectConstantVEx ( t.terrain.effectstartindex+4,t.effectparam.sky.HudFogDist,g.terrainvectorindex );
		SetVector4 ( g.terrainvectorindex,t.visuals.FogR_f/255.0,t.visuals.FogG_f/255.0,t.visuals.FogB_f/255.0,t.visuals.FogA_f/255.0 );
		SetEffectConstantVEx ( t.terrain.effectstartindex+4,t.effectparam.sky.HudFogColor,g.terrainvectorindex );
	}
	if ( GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
	{
		SetVector4 ( g.terrainvectorindex,t.visuals.FogNearest_f,t.visuals.FogDistance_f,0,0 );
		SetEffectConstantVEx ( t.terrain.effectstartindex+9,t.effectparam.skyscroll.HudFogDist,g.terrainvectorindex );
		SetVector4 ( g.terrainvectorindex,t.visuals.FogR_f/255.0,t.visuals.FogG_f/255.0,t.visuals.FogB_f/255.0,t.visuals.FogA_f/255.0 );
		SetEffectConstantVEx ( t.terrain.effectstartindex+9,t.effectparam.skyscroll.HudFogColor,g.terrainvectorindex );
	}
}

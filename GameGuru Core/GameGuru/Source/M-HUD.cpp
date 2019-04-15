//----------------------------------------------------
//--- GAMEGURU - M-HUD
//----------------------------------------------------

#include "gameguru.h"

// externs store for jetpack shader effect indexes
int g_jetpackboneshadereffectindex = 0;

// 
//  HUD Subroutines (blood splats, directional damage, etc)
// 

void hud_init ( void )
{
	//  Reset screen bloods and markers
	for ( t.find = 1 ; t.find <= 40 ; t.find++ ) t.screenblood[t.find].used=0; 
	for ( t.find = 1 ; t.find<=  40 ; t.find++ ) t.damagemarker[t.find].used=0 ;
}

void hud_scanforhudlayers ( void )
{

	//  Clear hud layers list
	for ( t.hli = 0 ; t.hli<=  g.hudlayerlistmax; t.hli++ )
	{
		t.hudlayerlist[t.hli].name_s="";
		t.hudlayerlist[t.hli].obj=0;
	}

	//  Scan entities to see if any jetpacks used
	t.hudlayersbankoffsetindex=g.hudlayersbankoffset+1;
	t.hudlayersimageoffsetindex=g.hudlayersimageoffset+1;
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		t.tname_s=Lower(t.entityprofile[t.entid].ishudlayer_s.Get());
		if (  Len(t.tname_s.Get())>1 ) 
		{

			//  first see if we already have the jetpack
			t.tfound=0;
			for ( t.hli = 1 ; t.hli<=  g.hudlayerlistmax; t.hli++ )
			{
				if (  t.hudlayerlist[t.hli].name_s == t.tname_s ) 
				{
					t.tfound=t.hli ; break;
				}
			}
			if (  t.tfound == 0 ) 
			{
				//  find free one or expand
				for ( t.hli = 1 ; t.hli<=  g.hudlayerlistmax; t.hli++ )
				{
					if (  t.hudlayerlist[t.hli].name_s == "" ) 
					{
						t.tfound=t.hli ; break;
					}
				}
				if (  t.tfound == 0 ) 
				{
					//  expand
					t.thudlayerlistmaxold=g.hudlayerlistmax ; g.hudlayerlistmax=g.hudlayerlistmax*2;
					Dim (  t.hudlayerlist,g.hudlayerlistmax );
					for ( t.hli = t.thudlayerlistmaxold+1 ; t.hli<=  g.hudlayerlistmax; t.hli++ )
					{
						t.hudlayerlist[t.hli].name_s="";
						t.hudlayerlist[t.hli].obj=0;
					}
					t.tfound=t.thudlayerlistmaxold+1;
				}
				// add hudlayer item
				if ( GetEffectExist(g.jetpackeffectoffset) == 0 ) 
				{
					t.playercontrol.jetpackeffect=g.jetpackeffectoffset;
					//character_basic.fx
					LoadEffect ( "effectbank\\reloaded\\weapon_bone.fx", t.playercontrol.jetpackeffect, 0 );
					SetEffectTechnique ( t.playercontrol.jetpackeffect, "Highest" );

					//PE: HUD is getting clipped.
					t.tnothing = MakeVector4(g.characterkitvector);
					SetVector4(g.characterkitvector, 500000, 1, 0, 0);
					SetEffectConstantV(t.playercontrol.jetpackeffect, "EntityEffectControl", g.characterkitvector);
					t.tnothing = DeleteVector4(g.characterkitvector);

					if ( g_jetpackboneshadereffectindex == 0 ) g_jetpackboneshadereffectindex = t.playercontrol.jetpackeffect;
				}
				if ( ObjectExist(t.hudlayersbankoffsetindex) == 0 ) 
				{
					t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+".x";
					if (  FileExist( t.strwork.Get() ) == 1 ) 
					{
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+".x";
						LoadObject ( t.strwork.Get() ,t.hudlayersbankoffsetindex );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_D.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+0 );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_N.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+1 );

						//PE: _S look wrong using PBR.
						//t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_S.dds";
						//PE: Use material 0.
						t.strwork = "effectbank\\reloaded\\media\\materials\\0_Metalness.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+2 );
						int gloosstex = loadinternaltextureex("effectbank\\reloaded\\media\\materials\\0_Gloss.dds", 1, t.tfullorhalfdivide);

						//t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_cube.dds";
						//LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+3 );
						t.strwork = ""; t.strwork = t.strwork + "effectbank\\reloaded\\media\\white_D.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+3 );
						if (g.memskipibr == 0) 
						{
							t.entityprofiletexibrid = t.terrain.imagestartindex + 32;
							TextureObject(t.hudlayersbankoffsetindex, 8, t.entityprofiletexibrid);
						}
						TextureObject ( t.hudlayersbankoffsetindex, 7, 0 );
						TextureObject ( t.hudlayersbankoffsetindex, 0, t.hudlayersimageoffsetindex+0 );
						TextureObject ( t.hudlayersbankoffsetindex, 1, t.hudlayersimageoffsetindex+3 );
						TextureObject ( t.hudlayersbankoffsetindex, 2, t.hudlayersimageoffsetindex+1 );
						TextureObject ( t.hudlayersbankoffsetindex, 3, t.hudlayersimageoffsetindex+2 ); // Metal
						TextureObject ( t.hudlayersbankoffsetindex, 4, gloosstex); // Gloss
						TextureObject ( t.hudlayersbankoffsetindex, 5, t.hudlayersimageoffsetindex+3 );
						int iPBRCubeImg = t.terrain.imagestartindex+31;
						TextureObject ( t.hudlayersbankoffsetindex, 6, iPBRCubeImg );
						/*
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+".x";
						LoadObject ( t.strwork.Get() ,t.hudlayersbankoffsetindex );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_D.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+0 );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_N.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+1 );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_S.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+2 );
						t.strwork = ""; t.strwork = t.strwork + "gamecore\\hudlayers\\"+t.entityprofile[t.entid].ishudlayer_s+"_cube.dds";
						LoadImage ( t.strwork.Get() ,t.hudlayersimageoffsetindex+3 );
						TextureObject (  t.hudlayersbankoffsetindex,0,t.hudlayersimageoffsetindex+0 );
						TextureObject (  t.hudlayersbankoffsetindex,1,t.hudlayersimageoffsetindex+1 );
						TextureObject (  t.hudlayersbankoffsetindex,2,t.hudlayersimageoffsetindex+2 );
						TextureObject (  t.hudlayersbankoffsetindex,3,t.hudlayersimageoffsetindex+3 );
						TextureObject (  t.hudlayersbankoffsetindex,4,t.terrain.imagestartindex+2 );
						TextureObject (  t.hudlayersbankoffsetindex,5,g.postprocessimageoffset+5 );
						*/
						SetObjectEffectCore (  t.hudlayersbankoffsetindex,t.playercontrol.jetpackeffect,0 );
						YRotateObject (  t.hudlayersbankoffsetindex,180 );
						FixObjectPivot (  t.hudlayersbankoffsetindex );
						SetObjectMask (  t.hudlayersbankoffsetindex, 1 );
						//PE: Should be set so camera near dont clip hud.
						DisableObjectZDepth(t.hudlayersbankoffsetindex);

						//if (  g.globals.riftmode == 0 ) 
						//{
						//	DisableObjectZDepth (  t.hudlayersbankoffsetindex );
						//}
						//else
						//{
						//	//  interferes with RIFTMODE
						//}
						//  any any rogue marker limbs (until artist can consolidate and remove)
						PerformCheckListForLimbs (  t.hudlayersbankoffsetindex );
						for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
						{
							if (  cstr(Lower(ChecklistString(t.c))) == "plane002" ) 
							{
								HideLimb (  t.hudlayersbankoffsetindex,t.c-1 );
							}
						}
						HideObject (  t.hudlayersbankoffsetindex );
						t.hudlayerlist[t.tfound].hidden=0;
					}
					else
					{
						MakeObjectCube (  t.hudlayersbankoffsetindex,0 );
						HideObject (  t.hudlayersbankoffsetindex );
						t.hudlayerlist[t.tfound].hidden=1;
					}
					t.hudlayerlist[t.tfound].name_s=t.tname_s;
					t.hudlayerlist[t.tfound].obj=t.hudlayersbankoffsetindex;
					t.hudlayerlist[t.tfound].img=t.hudlayersimageoffsetindex;
					++t.hudlayersbankoffsetindex;
					t.hudlayersimageoffsetindex += 4;
				}
			}
			t.entityprofile[t.entid].ishudlayer=t.tfound;
		}
	}

	//  reset HUD layer flags
	t.playercontrol.jetobjtouse=g.hudlayersbankoffset+1;
	t.playercontrol.jetpackhidden=0;
	t.playercontrol.jetpackcollected=0;
	t.playercontrol.jetpackmode=0;
}

void hud_free ( void )
{
	// remove hudlayer shader
	if ( t.playercontrol.jetpackeffect>0 ) 
	{
		if ( GetEffectExist(t.playercontrol.jetpackeffect) == 1 ) DeleteEffect ( t.playercontrol.jetpackeffect );
		if ( g_jetpackboneshadereffectindex == t.playercontrol.jetpackeffect ) g_jetpackboneshadereffectindex = 0;
		t.playercontrol.jetpackeffect=0;
	}

	//  hide any HUD layer objects
	for ( t.hli = 1 ; t.hli<=  g.hudlayerlistmax; t.hli++ )
	{
		t.tobj=t.hudlayerlist[t.hli].obj;
		t.timg=t.hudlayerlist[t.hli].img;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1  )  DeleteObject (  t.tobj );
			if (  ImageExist(t.timg+0) == 1  )  DeleteImage (  t.timg+0 );
			if (  ImageExist(t.timg+1) == 1  )  DeleteImage (  t.timg+1 );
			if (  ImageExist(t.timg+2) == 1  )  DeleteImage (  t.timg+2 );
		}
		t.hudlayerlist[t.hli].obj=0;
		t.hudlayerlist[t.hli].name_s="";
	}
}

void hud_updatehudlayerobjects ( void )
{
	// position and rotate jetpack
	if ( t.playercontrol.jetobjtouse>0 ) 
	{
		if ( ObjectExist(t.playercontrol.jetobjtouse) == 1 ) 
		{
			if ( t.playercontrol.jetpackhidden == 0 ) 
			{
				if ( t.playercontrol.jetpackmode == 2 ) 
				{
					t.tsmooth_f=1;
					t.tydest_f=CameraPositionY(0)+(t.tadjustbasedonwobbley_f*0.25);
					PositionObject (  t.playercontrol.jetobjtouse,CameraPositionX(0),CurveValue(t.tydest_f,ObjectPositionY(t.playercontrol.jetobjtouse),t.tsmooth_f),CameraPositionZ(0) );
					SetObjectToCameraOrientation (  t.playercontrol.jetobjtouse );
					PitchObjectUp (  t.playercontrol.jetobjtouse,CameraAngleX(0)*0.2 );
					ShowObject (  t.playercontrol.jetobjtouse );
					if (  t.playercontrol.jetpackthrust_f>0 ) 
					{
						//  thrusting
						if (  t.playercontrol.jetpackanimation == 0 ) 
						{
							PlayObject (  t.playercontrol.jetobjtouse,174,181 );
							t.playercontrol.jetpackanimation=1;
						}
						else
						{
							if (  GetPlaying(t.playercontrol.jetobjtouse) == 0 ) 
							{
								StopObject (  t.playercontrol.jetobjtouse );
								SetObjectFrame (  t.playercontrol.jetobjtouse,181 );
							}
						}
						SetObjectSpeed (  t.playercontrol.jetobjtouse,400.0*g.timeelapsed_f );
					}
					else
					{
						//  no thrust
						if (  t.playercontrol.jetpackanimation == 1 ) 
						{
							PlayObject (  t.playercontrol.jetobjtouse,184,191 );
							t.playercontrol.jetpackanimation=0;
						}
						else
						{
							if (  GetFrame(t.playercontrol.jetobjtouse) >= 191 ) 
							{
								if (  GetPlaying(t.playercontrol.jetobjtouse) == 0 ) 
								{
									LoopObject (  t.playercontrol.jetobjtouse,30,100 );
								}
							}
						}
						SetObjectSpeed (  t.playercontrol.jetobjtouse,100.0*g.timeelapsed_f );
					}
					if (  t.playercontrol.jetpackhidden == 0 ) 
					{
						t.trotatez_f=t.playercontrol.jetpackfuel_f/1.9;
						if (  t.trotatez_f>280  )  t.trotatez_f = 280.0;
						RotateLimb (  t.playercontrol.jetobjtouse,3,0,0,42+t.trotatez_f );
					}
				}
				else
				{
					HideObject (  t.playercontrol.jetobjtouse );
					if (  GetPlaying(t.playercontrol.jetobjtouse) == 1 ) 
					{
						StopObject (  t.playercontrol.jetobjtouse );
					}
				}
			}
		}
	}
}

void blood_damage_init ( void )
{
	//  set up with place holders for images
	t.huddamage.bloodtimestart=500;
	t.huddamage.bloodtimeend=1500;
	t.huddamage.bloodon=1;
	t.huddamage.damageindicatoron=1;
	t.huddamage.damagetimestart=1;
	t.huddamage.damagetimeend=0;
	t.huddamage.fadespeed=1500;
	t.huddamage.damagetrackshooter=1;

	//  blood splats
	if (  t.huddamage.bloodtotal == 0 ) 
	{
//   `huddamage.maxbloodsplats=66

//   `for mb=0 to huddamage.maxbloodsplats ; rem maybe 'extendedblood' mode later on (if community requests this)

		t.huddamage.maxbloodsplats=7;
		for ( t.mb = 0 ; t.mb<=  t.huddamage.maxbloodsplats; t.mb++ )
		{
			t.a_s = ""; t.a_s=t.a_s+"databank\\bloodsplash"+Str(t.mb+1)+".png";
			if (  FileExist(t.a_s.Get()) == 1 ) 
			{
				LoadImage (  t.a_s.Get(),t.huddamage.bloodstart+t.mb,0,g.gdividetexturesize );
				Sprite (  t.huddamage.bloodstart+t.mb,-1000,-1000,t.huddamage.bloodstart+t.mb );
				++t.huddamage.bloodtotal;

				//  keep alive with steam server
				if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			}
		}
	}

	//  directional indicator
	t.a_s="databank\\indicator.png";
	if (  ImageExist(t.huddamage.indicator) == 0  )  LoadImage (  t.a_s.Get(),t.huddamage.indicator );
	Sprite (  t.huddamage.indicator,-100000,-100000,t.huddamage.indicator );
	OffsetSprite (  t.huddamage.indicator,ImageWidth(t.huddamage.indicator)/2,ImageHeight(t.huddamage.indicator)/2 );

	//  keep alive with steam server
	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

return;


//BloodDamage&Direction Functions


}

void placeblood ( int damage, int x, int y, int z, int howfar )
{
	int find = 0;

	//  chance there will be no blood or early exit if off
	if (  t.huddamage.bloodon<1 || Rnd(100)>50  )  return;

	//  no blood damage for third person
	if (  t.playercontrol.thirdperson.enabled == 1  )  return;

	//  find space
	for ( find = 1 ; find<=  40; find++ )
	{
		if (  t.screenblood[find].used == 0 ) 
		{

			//  set active
			t.screenblood[find].used=1;

			//  choose image
			t.screenblood[find].image=t.huddamage.bloodstart+Rnd(t.huddamage.maxbloodsplats);

			//  position and scale
			t.screenblood[find].x=Rnd(GetDisplayWidth())-256;
			t.screenblood[find].y=Rnd(GetDisplayHeight())-256;
			t.screenblood[find].scale=80+damage+(Rnd(40));
			ScaleSprite (  t.screenblood[find].image,t.screenblood[find].scale );

			//  set life
			t.screenblood[find].life=Timer() + t.huddamage.bloodtimestart;
			if (  t.huddamage.bloodtimeend>t.huddamage.bloodtimestart ) 
			{
				t.screenblood[find].life += Rnd(t.huddamage.bloodtimeend-t.huddamage.bloodtimestart);
			}
			t.screenblood[find].fadeout = t.huddamage.fadespeed;
			return;
		}
	}
//endfunction

}

int controlblood ( void )
{
	int tTimerCache = 0;
	float findtime_f = 0;
	int howmany = 0;
	int tAlpha = 0;
	int cb = 0;
	tTimerCache = Timer();
	howmany=0;
	for ( cb = 1 ; cb<=  40; cb++ )
	{

		//  is blood used?
		if (  t.screenblood[cb].used != 0 ) 
		{

			//  track number in use
			++howmany;

			//  fade out blood
			findtime_f=tTimerCache-t.screenblood[cb].life;
			if (  findtime_f < 0 ) 
			{
				tAlpha = 255;
			}
			else
			{
				tAlpha = 255 - (findtime_f/t.screenblood[cb].fadeout)*255;
				if (  tAlpha  <=  0  )  t.screenblood[cb].used = 0;
			}
			if (  tAlpha > 0 ) 
			{
				SetSpriteAlpha (  t.screenblood[cb].image,tAlpha );
				PasteSprite (  t.screenblood[cb].image,t.screenblood[cb].x,t.screenblood[cb].y );
			}
		}
	}
//endfunction howmany
	return howmany
;
}

void resetblood ( void )
{
	int cb = 0;
	for ( cb = 1 ; cb<=  40; cb++ )
	{
		t.screenblood[cb].fadeout=0;
		t.screenblood[cb].fadetime=Timer();
		t.screenblood[cb].used=0;
	}
//endfunction

}

void new_damage_marker ( int entity, int x, int z, int y, int tempdamage )
{
	int attackeralreadyhasmarker = 0;
	int nodouble = 0;
	float tcamx_f = 0;
	float tcamy_f = 0;
	float tcamz_f = 0;
	int howfar = 0;
	float tdx_f = 0;
	float tdy_f = 0;
	float tdz_f = 0;
	int find = 0;
	//  no blood damage for third person
	if (  t.playercontrol.thirdperson.enabled == 1  )  return;
	//  only allow an attack 1 damage marker per burst
	attackeralreadyhasmarker=0;
	for ( nodouble = 1 ; nodouble<=  40; nodouble++ )
	{
		if (  t.damagemarker[nodouble].entity == entity ) 
		{
			attackeralreadyhasmarker=1;
			nodouble=41;
		}
	}
	//  early exit if damage off or attacker already has marker.
	if (  t.huddamage.damageindicatoron<1 || attackeralreadyhasmarker != 0  )  return;
	//  otherwise give it them a new maker
	for ( find = 1 ; find<=  40; find++ )
	{
		if (  t.damagemarker[find].used == 0 ) 
		{
			//  if it's free set up..
			t.damagemarker[find].x=GetDisplayWidth()/2;
			t.damagemarker[find].y=GetDisplayHeight()/2;
			t.damagemarker[find].life=t.huddamage.damagetimestart;
			if (  t.huddamage.damagetimeend>t.huddamage.damagetimestart ) 
			{
				t.damagemarker[find].life += Rnd(t.huddamage.damagetimeend-t.huddamage.damagetimestart);
			}
			t.damagemarker[find].fadeout=255;
			t.damagemarker[find].fadelife=t.huddamage.fadespeed/255;
			//  ensure fade isn't too slow based on user settings.
			if (  t.damagemarker[find].fadelife>t.huddamage.fadespeed  )  t.damagemarker[find].fadelife = t.huddamage.fadespeed;
			t.damagemarker[find].fadetime=Timer();
			t.damagemarker[find].used=1;
			t.damagemarker[find].image=t.huddamage.indicator;
			t.damagemarker[find].time=Timer();
			t.damagemarker[find].scale=100;
			t.damagemarker[find].entity=entity;
			//  store damage start location
			t.damagemarker[find].lx=x;
			t.damagemarker[find].ly=y;
			t.damagemarker[find].lz=z;
			//  find distance and angle
			tcamx_f=CameraPositionX(0);
			tcamy_f=CameraPositionY(0);
			tcamz_f=CameraPositionZ(0);
			//  if not player -1 (e.g, not from falling,etc.).
			if (  entity != -1 ) 
			{
				tdx_f=x-tcamx_f;
				tdy_f=y+(ObjectSize(t.entityelement[entity].obj)/2)-tcamy_f;
				tdz_f=z-tcamz_f;
				howfar=Sqrt(abs(tdx_f*tdx_f)+abs(tdy_f*tdy_f)+abs(tdz_f*tdz_f));
				if (  howfar>300  )  howfar = 300;
				t.damagemarker[find].range=howfar;
				//  find the angle (later)
				//     `fundx#=entityelement(entity).x-CameraPositionX()
				//     `fundy#=CameraPositionZ()-entityelement(entity).z
				//     `fundx#=entityelement(entity).x-CameraPositionX()
				//     `fundy#=entityelement(entity).z-CameraPositionZ()
				//     `funangle#=atan2deg(fundx#,fundy#)
				//     `rrr#=funangle#-WrapValue(CameraAngleY())
				float fundx = t.entityelement[entity].x - CameraPositionX();
				float fundz = t.entityelement[entity].z - CameraPositionZ();
				float funangle = atan2deg ( fundx, fundz );
				float rrr = funangle - WrapValue(CameraAngleY());
				t.damagemarker[find].angle = WrapValue ( rrr );
			}
			else
			{
				//  set on top of player
				t.damagemarker[find].range=30;
				t.damagemarker[find].angle=600;
			}
			find=41;
		}
	}
	placeblood(tempdamage,x,y,z,howfar);
//endfunction

}

int controldamagemarker ( void )
{
	float funangle_f = 0;
	int findtime = 0;
	float tangle_f = 0;
	float fundx_f = 0;
	float fundy_f = 0;
	int howmany = 0;
	float tcamx_f = 0;
	float tcamy_f = 0;
	float tcamz_f = 0;
	int entity = 0;
	int howfar = 0;
	int talpha = 0;
	int trange = 0;
	float usex_f = 0;
	float usey_f = 0;
	float usez_f = 0;
	float rrr_f = 0;
	float tdx_f = 0;
	float tdy_f = 0;
	float tdz_f = 0;
	int dm = 0;
	howmany=0;
	for ( dm = 1 ; dm<=  40; dm++ )
	{
		//  in use?
		if (  t.damagemarker[dm].used != 0 ) 
		{
			//  if player ignore tracking
			if (  t.damagemarker[dm].angle == 600 ) 
			{
				t.damagemarker[dm].used=0;
			}
			else
			{
				//  otherwise do the business
				++howmany;
				ScaleSprite (  t.damagemarker[dm].image,t.damagemarker[dm].scale );
				findtime=Timer()-t.damagemarker[dm].time;
				if (  Timer()-t.damagemarker[dm].fadetime>t.damagemarker[dm].fadelife ) 
				{
					t.damagemarker[dm].fadeout -= t.damagemarker[dm].fadelife;
					t.damagemarker[dm].fadetime=Timer();
				}
				talpha=t.damagemarker[dm].fadeout;
				if (  talpha<0  )  talpha = 0;
				if (  talpha>255  )  talpha = 255;
				SetSpriteAlpha (  t.damagemarker[dm].image,talpha );
				//  find new angle
				tcamx_f=CameraPositionX(0);
				tcamy_f=CameraPositionY(0);
				tcamz_f=CameraPositionZ(0);
				entity=t.damagemarker[dm].entity;
				if (  entity != -1 ) 
				{
					usex_f=0.0;
					usey_f=0.0;
					usez_f=0.0;
					if (  t.huddamage.damagetrackshooter == 0 ) 
					{
						usex_f=t.damagemarker[dm].lx;
						usey_f=t.damagemarker[dm].ly;
						usez_f=t.damagemarker[dm].lz;
					}
					else
					{
						usex_f=ObjectPositionX(t.entityelement[entity].obj);
						usey_f=ObjectPositionY(t.entityelement[entity].obj);
						usez_f=ObjectPositionZ(t.entityelement[entity].obj);
					}
					tdx_f=usex_f-tcamx_f;
					tdy_f=usey_f+(ObjectSize(t.entityelement[entity].obj)/2)-tcamy_f;
					tdz_f=usez_f-tcamz_f;
					howfar=Sqrt(abs(tdx_f*tdx_f)+abs(tdy_f*tdy_f)+abs(tdz_f*tdz_f));
					if (  howfar>300  )  howfar = 300;
					t.damagemarker[dm].range=howfar;
					//  find the angle.
					fundx_f=usex_f-CameraPositionX(0);
					fundy_f=usez_f-CameraPositionZ(0);
					funangle_f=atan2deg(fundx_f,fundy_f);
					rrr_f=funangle_f-WrapValue(CameraAngleY(0));
					t.damagemarker[dm].angle=WrapValue(rrr_f);
				}
				else
				{
					t.damagemarker[dm].range=30;
					t.damagemarker[dm].angle=600;
				}
				//  free up if invisible
				if (  talpha <= 0 ) 
				{
					t.damagemarker[dm].used=0;
					t.damagemarker[dm].entity=0;
				}
				else
				{
					//  otherwise redraw
					t.tangle_f=t.damagemarker[dm].angle;
					trange=250;
					RotateSprite (  t.damagemarker[dm].image,t.tangle_f );
					PasteSprite (  t.damagemarker[dm].image,t.damagemarker[dm].x+(Sin(t.tangle_f)*trange),t.damagemarker[dm].y-(Cos(t.tangle_f)*trange) );
				}
			}
		}
	}
	//  return number of arrows
//endfunction howmany
	return howmany
;
}

void resetdamagemarker ( void )
{
	int dm = 0;
	for ( dm = 1 ; dm<=  40; dm++ )
	{
		t.damagemarker[dm].used=0;
		t.damagemarker[dm].fadeout=0;
		t.damagemarker[dm].fadetime=Timer();
		t.damagemarker[dm].used=0;
		t.damagemarker[dm].entity=0;
	}
//endfunction

}

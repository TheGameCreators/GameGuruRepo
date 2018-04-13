//----------------------------------------------------
//--- GAMEGURU - M-Gun
//----------------------------------------------------

#include "gameguru.h"

// 
//  GUN CODE (common)
// 

void gun_loaddata ( void )
{
	//  default settings
	t.gun[t.gunid].settings.minpolytrim=0;
	t.gun[t.gunid].projectile_s="";
	t.gun[t.gunid].projectileframe=0;
	t.gun[t.gunid].vweaptex_s="";

	//  two basic firemodes (i.e semi+full auto or shotgun+grenade)
	for ( t.i = 0 ; t.i <= 1; t.i++ )
	{
		//  Default Zoom Walk & Turn
		g.firemodes[t.gunid][t.i].settings.zoomwalkspeed=0.5f;
		g.firemodes[t.gunid][t.i].settings.zoomturnspeed=0.2f;
		g.firemodes[t.gunid][t.i].settings.plrmovespeedmod=1.0f;
		g.firemodes[t.gunid][t.i].settings.plremptyspeedmod=1.0f;
		g.firemodes[t.gunid][t.i].settings.plrturnspeedmod=1.0f;
		g.firemodes[t.gunid][t.i].settings.plrjumpspeedmod=1.0f;
		g.firemodes[t.gunid][t.i].settings.plrreloadspeedmod=1.0f;

		//  Default Run Accuracy (No diff)
		g.firemodes[t.gunid][t.i].settings.runaccuracy = -1;
		g.firemodes[t.gunid][t.i].settings.runanimdelay = 250;

		//  Default Sound Strength
		g.firemodes[t.gunid][t.i].settings.soundstrength=100;

		//  Default equipment settings
		g.firemodes[t.gunid][t.i].settings.equipment=0;
		g.firemodes[t.gunid][t.i].settings.equipmententityelementindex=0;
		g.firemodes[t.gunid][t.i].settings.lockcamera=0;

		//  Reset HUD object rotation
		g.firemodes[t.gunid][t.i].settings.rotx_f=0;
		g.firemodes[t.gunid][t.i].settings.roty_f=0;
		g.firemodes[t.gunid][t.i].settings.rotz_f=0;

		//  resets
		g.firemodes[t.gunid][t.i].settings.reloadspeed=1.0;
		g.firemodes[t.gunid][t.i].settings.decalforward=0;
		g.firemodes[t.gunid][t.i].settings.dofdistance=0;
		g.firemodes[t.gunid][t.i].settings.dofintensity=0;
		g.firemodes[t.gunid][t.i].settings.zoomhidecrosshair=0;

		// by default gunspec defaults there is no spot light for gun
		g.firemodes[t.gunid][t.i].settings.usespotlighting=0;

		// ensure material not ignored by default
		g.firemodes[t.gunid][t.i].settings.ignorematerial = 0;
	}

	//  Load GUNSPEC details
	Dim (  t.data_s,500  );
	t.filename_s = "" ; t.filename_s=t.filename_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\gunspec.txt";
	if (  FileExist(t.filename_s.Get()) == 0  ) { t.filename_s = "" ; t.filename_s=t.filename_s+ "gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\spec.txt"; }
	if (  FileExist(t.filename_s.Get()) == 1 ) 
	{
		LoadArray (  t.filename_s.Get() ,t.data_s );
		for ( t.l = 0 ; t.l<=  499; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if ( t.line_s.Get()[0] != ';' )
				{

					//  take fieldname and value
					for ( t.c = 0 ; t.c <  Len(t.line_s.Get()); t.c++ )
					{
						if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1  ; break; }
					}
					t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
					t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));

					//  take value 1 and 2 from value
					for ( t.c = 0 ; t.c <  Len(t.value_s.Get()); t.c++ )
					{
						if (  t.value_s.Get()[t.c] == ',' ) { t.mid = t.c+1 ; break; }
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1))) ;
					t.value2=ValF(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));

					//  Gun basic settings
					if (  t.field_s == "statuspanelcode"  )  t.gun[t.gunid].statuspanelcode = t.value1;
					if (  t.field_s == "weaponisammo"  )  t.gun[t.gunid].settings.weaponisammo = t.value1;
					if (  t.field_s == "minpolytrim"  )  t.gun[t.gunid].settings.minpolytrim = t.value1;

					//  Gun basic visuals
					if (  t.field_s == "textured"  )  t.gun[t.gunid].texd_s = t.value_s;
					if (  t.field_s == "effect"  )  t.gun[t.gunid].effect_s = t.value_s;
					if (  t.field_s == "transparency"  )  t.gun[t.gunid].transparency = t.value1;
					if (  t.field_s == "vweaptex"  )  t.gun[t.gunid].vweaptex_s = t.value_s;

					//  weapontype ; 0-grenade, 1-pistol, 2-rocket, 3-shotgun, 4-uzi, 5-assault, 51-melee(noammo)
					if (  t.field_s == "weapontype"  )  t.gun[t.gunid].weapontype = t.value1;

					if (  t.field_s == "projectile"  )  t.gun[t.gunid].projectile_s = t.value_s;
					if (  t.field_s == "projectileframe"  )  t.gun[t.gunid].projectileframe = t.value1;

					if (  t.field_s == "keyframe speed" ) t.gun[t.gunid].keyframespeed_f = (float)t.value1 / 100.0f;

					//  Load in details for two firemodes
					for ( t.i = 0 ; t.i<=  2; t.i++ )
					{
						//  Firemode subscript
						if (  t.i == 0  )  t.alt_s = "";
						if (  t.i == 1  )  t.alt_s = "alt ";
						if (  t.i == 2  )  t.alt_s = "alt";
						if (  t.i != 2  )  t.x = t.i; else t.x = 1;

						// 140616 - Whether fire mode affects projectile spot lighting
						if ( t.field_s == t.alt_s+"usespotlighting"  ) g.firemodes[t.gunid][t.x].settings.usespotlighting = t.value1;

						//  Classic details
						if (  t.field_s == t.alt_s+"muzzlesize"  )  g.firemodes[t.gunid][t.x].settings.muzzlesize_f = t.value1;
						if (  t.field_s == t.alt_s+"muzzlecolorr"  )  g.firemodes[t.gunid][t.x].settings.muzzlecolorr = t.value1;
						if (  t.field_s == t.alt_s+"muzzlecolorg"  )  g.firemodes[t.gunid][t.x].settings.muzzlecolorg = t.value1;
						if (  t.field_s == t.alt_s+"muzzlecolorb"  )  g.firemodes[t.gunid][t.x].settings.muzzlecolorb = t.value1;
						if (  t.field_s == t.alt_s+"smoke"  )  g.firemodes[t.gunid][t.x].settings.smoke = t.value1;
						if (  t.field_s == t.alt_s+"smokespeed"  )  g.firemodes[t.gunid][t.x].settings.smokespeed = t.value1;
						if (  t.field_s == t.alt_s+"smokedecal"  )  g.firemodes[t.gunid][t.x].settings.smokedecal_s = t.value_s;
						if (  t.field_s == t.alt_s+"flak"  )  g.firemodes[t.gunid][t.x].settings.flakname_s = t.value_s;
						if (  t.field_s == t.alt_s+"flakrearmframe"  )  g.firemodes[t.gunid][t.x].settings.flakrearmframe = t.value1;
						if (  t.field_s == t.alt_s+"reloadqty"  )  g.firemodes[t.gunid][t.x].settings.reloadqty = t.value1;
						if (  t.field_s == t.alt_s+"reloadspeed"  )  g.firemodes[t.gunid][t.x].settings.reloadspeed = (t.value1+0.0)/100.0;
						if (  t.field_s == t.alt_s+"iterate"  )  g.firemodes[t.gunid][t.x].settings.iterate = t.value1;
						if (  t.field_s == t.alt_s+"range"  )  g.firemodes[t.gunid][t.x].settings.range = t.value1;
						if (t.field_s == t.alt_s + "accuracy")  g.firemodes[t.gunid][t.x].settings.accuracy = t.value1;
						if (  t.field_s == t.alt_s+"damage"  )  g.firemodes[t.gunid][t.x].settings.damage = t.value1;
						if (  t.field_s == t.alt_s+"dropoff"  )  g.firemodes[t.gunid][t.x].settings.dropoff = t.value1;
						if (  t.field_s == t.alt_s+"muzzleflash"  )  g.firemodes[t.gunid][t.x].settings.muzzleflash = t.value1;
						if (  t.field_s == t.alt_s+"firerate"  )  g.firemodes[t.gunid][t.x].settings.firerate = t.value1;
						if (t.field_s == t.alt_s + "detectcoloff")  g.firemodes[t.gunid][t.x].settings.detectcoloff = t.value1;
						if (t.field_s == t.alt_s + "ignorematerial")  g.firemodes[t.gunid][t.x].settings.ignorematerial = t.value1;
						if (  t.field_s == t.alt_s+"zoommode"  )  g.firemodes[t.gunid][t.x].settings.zoommode = t.value1;
						if (  t.field_s == t.alt_s+"zoomaccuracy"  )  g.firemodes[t.gunid][t.x].settings.zoomaccuracy = t.value1;
						if (  t.field_s == t.alt_s+"zoomaccuracybreathhold"  )  g.firemodes[t.gunid][t.x].settings.zoomaccuracybreathhold = t.value1;
						if (  t.field_s == t.alt_s+"zoomaccuracybreath"  )  g.firemodes[t.gunid][t.x].settings.zoomaccuracybreath = t.value1;
						if (  t.field_s == t.alt_s+"zoomaccuracyheld"  )  g.firemodes[t.gunid][t.x].settings.zoomaccuracyheld = t.value1;
						if (  t.field_s == t.alt_s+"zoomscope"  )  g.firemodes[t.gunid][t.x].zoomscope_s = t.value_s;
						if (  t.field_s == t.alt_s+"second"  )  g.firemodes[t.gunid][t.x].settings.seconduse = t.value1;
						if (  t.field_s == t.alt_s+"damagetype"  )  g.firemodes[t.gunid][t.x].settings.damagetype = t.value1;
						if (  t.field_s == t.alt_s+"scorchtype"  )  g.firemodes[t.gunid][t.x].settings.scorchtype = t.value1;
						if (  t.field_s == t.alt_s+"melee damage"  )  g.firemodes[t.gunid][t.x].settings.meleedamage = t.value1;
						if (  t.field_s == t.alt_s+"melee range"  )  g.firemodes[t.gunid][t.x].settings.meleerange = t.value1;
						if (  t.field_s == t.alt_s+"dofdistance"  )  g.firemodes[t.gunid][t.x].settings.dofdistance = t.value1;
						if (  t.field_s == t.alt_s+"dofintensity"  )  g.firemodes[t.gunid][t.x].settings.dofintensity = t.value1;
						if (  t.field_s == t.alt_s+"decal"  )  g.firemodes[t.gunid][t.x].decal_s = t.value_s;
						if (  t.field_s == t.alt_s+"decalforward"  )  g.firemodes[t.gunid][t.x].settings.decalforward = t.value1;
						if (  t.field_s == t.alt_s+"shotgun"  )  g.firemodes[t.gunid][t.x].settings.shotgun = t.value1;
						if (  t.field_s == t.alt_s+"disablerunandshoot"  )  g.firemodes[t.gunid][t.x].settings.disablerunandshoot = t.value1;
						if (  t.field_s == t.alt_s+"force"  )  g.firemodes[t.gunid][t.x].settings.force = t.value1;
						if (  t.field_s == t.alt_s+"melee force"  )  g.firemodes[t.gunid][t.x].settings.meleeforce = t.value1;
						if (  t.field_s == t.alt_s+"npcignorereload"  )  g.firemodes[t.gunid][t.x].settings.npcignorereload = t.value1;
						if (  t.field_s == t.alt_s+"zoomhidecrosshair"  )  g.firemodes[t.gunid][t.x].settings.zoomhidecrosshair = t.value1;

						//  Brass details
						if (  t.field_s == t.alt_s+"brass"  )  g.firemodes[t.gunid][t.x].settings.brass = t.value1;
						if (  t.field_s == t.alt_s+"brasslife"  )  g.firemodes[t.gunid][t.x].settings.brasslife = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassangle"  )  g.firemodes[t.gunid][t.x].settings.brassangle = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassanglerand"  )  g.firemodes[t.gunid][t.x].settings.brassanglerand = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassspeed"  )  g.firemodes[t.gunid][t.x].settings.brassspeed = (t.value1+0.0)/100.0;
						if (  t.field_s == t.alt_s+"brassspeedrand"  )  g.firemodes[t.gunid][t.x].settings.brassspeedrand = (t.value1+0.0)/100.0;
						if (  t.field_s == t.alt_s+"brassupward"  )  g.firemodes[t.gunid][t.x].settings.brassupward = (t.value1+0.0)/100.0;
						if (  t.field_s == t.alt_s+"brassupwardrand"  )  g.firemodes[t.gunid][t.x].settings.brassupwardrand = (t.value1+0.0)/100.0;
						if (  t.field_s == t.alt_s+"brassrotx"  )  g.firemodes[t.gunid][t.x].settings.brassrotx = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassrotxrand"  )  g.firemodes[t.gunid][t.x].settings.brassrotxrand = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassroty"  )  g.firemodes[t.gunid][t.x].settings.brassroty = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassrotyrand"  )  g.firemodes[t.gunid][t.x].settings.brassrotyrand = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassrotz"  )  g.firemodes[t.gunid][t.x].settings.brassrotz = (t.value1+0.0);
						if (  t.field_s == t.alt_s+"brassrotzrand"  )  g.firemodes[t.gunid][t.x].settings.brassrotzrand = (t.value1+0.0);

						//  Advanced Features
						if (  t.field_s == t.alt_s+"gravitygun"  )  g.firemodes[t.gunid][t.x].settings.gravitygun = t.value1;
						if (  t.field_s == t.alt_s+"magnet"  )  g.firemodes[t.gunid][t.x].settings.magnet = t.value1;
						if (  t.field_s == t.alt_s+"magnetstrength"  )  g.firemodes[t.gunid][t.x].settings.magnetstrength = t.value1;
						if (  t.field_s == t.alt_s+"gravitystrength"  )  g.firemodes[t.gunid][t.x].settings.gravitystrength = t.value1;
						if (  t.field_s == t.alt_s+"emptyreloadonly"  )  g.firemodes[t.gunid][t.x].settings.emptyreloadonly = t.value1;
						if (  t.field_s == t.alt_s+"overheatafter"  )  g.firemodes[t.gunid][t.x].settings.overheatafter = t.value1;
						if (  t.field_s == t.alt_s+"cooldown"  )  g.firemodes[t.gunid][t.x].settings.cooldown = t.value1;
						if (  t.field_s == t.alt_s+"jamchance"  )  g.firemodes[t.gunid][t.x].settings.jamchance = t.value1;
						if (  t.field_s == t.alt_s+"jamtimer"  )  g.firemodes[t.gunid][t.x].settings.mintimer = t.value1;
						if (  t.field_s == t.alt_s+"nosubmergedfire"  )  g.firemodes[t.gunid][t.x].settings.nosubmergedfire = t.value1;
						if (  t.field_s == t.alt_s+"burst"  )  g.firemodes[t.gunid][t.x].settings.burst = t.value1;
						if (  t.field_s == t.alt_s+"disablemovespeedmod"  )  g.firemodes[t.gunid][t.x].settings.movespeedmod = t.value1;
						if (  t.field_s == t.alt_s+"chamberedround"  )  g.firemodes[t.gunid][t.x].settings.chamberedround = t.value1;
						if (  t.field_s == t.alt_s+"noautoreload"  )  g.firemodes[t.gunid][t.x].settings.noautoreload = t.value1;
						if (  t.field_s == t.alt_s+"nofullreload"  )  g.firemodes[t.gunid][t.x].settings.nofullreload = t.value1;
						if (  t.field_s == t.alt_s+"runx"  )  g.firemodes[t.gunid][t.x].settings.runx_f = t.value1;
						if (  t.field_s == t.alt_s+"runy"  )  g.firemodes[t.gunid][t.x].settings.runy_f = t.value1;
						if (  t.field_s == t.alt_s+"runacc"  )  g.firemodes[t.gunid][t.x].settings.runaccuracy = t.value1;
						if (  t.field_s == t.alt_s+"runanimdelay"  )  g.firemodes[t.gunid][t.x].settings.runanimdelay = t.value1;
						if (  t.field_s == t.alt_s+"noscorch"  )  g.firemodes[t.gunid][t.x].settings.noscorch = t.value1;
						if (  t.field_s == t.alt_s+"melee noscorch"  )  g.firemodes[t.gunid][t.x].settings.meleenoscorch = t.value1;
						if (  t.field_s == t.alt_s+"simplezoom"  )  g.firemodes[t.gunid][t.x].settings.simplezoom = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomx"  )  g.firemodes[t.gunid][t.x].settings.simplezoomx_f = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomy"  )  g.firemodes[t.gunid][t.x].settings.simplezoomy_f = t.value1;
						if (  t.field_s == t.alt_s+"simplezoommod"  )  g.firemodes[t.gunid][t.x].settings.simplezoommod = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomacc"  )  g.firemodes[t.gunid][t.x].settings.simplezoomacc = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomspeed"  )  g.firemodes[t.gunid][t.x].settings.simplezoomspeed = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomflash"  )  g.firemodes[t.gunid][t.x].settings.simplezoomflash = t.value1;
						if (  t.field_s == t.alt_s+"simplezoomanim"  )  g.firemodes[t.gunid][t.x].settings.simplezoomanim = t.value1;
						if (  t.field_s == t.alt_s+"gunlagspeed"  )  g.firemodes[t.gunid][t.x].settings.gunlagSpeed = t.value1;
						if (  t.field_s == t.alt_s+"gunlagxmax"  )  g.firemodes[t.gunid][t.x].settings.gunlagXmax = t.value1;
						if (  t.field_s == t.alt_s+"gunlagymax"  )  g.firemodes[t.gunid][t.x].settings.gunlagYmax = t.value1;
						if (  t.field_s == t.alt_s+"zoomgunlagspeed"  )  g.firemodes[t.gunid][t.x].settings.zoomgunlagSpeed = t.value1;
						if (  t.field_s == t.alt_s+"zoomgunlagxmax"  )  g.firemodes[t.gunid][t.x].settings.zoomgunlagXmax = t.value1;
						if (  t.field_s == t.alt_s+"zoomgunlagymax"  )  g.firemodes[t.gunid][t.x].settings.zoomgunlagYmax = t.value1;
						if (  t.field_s == t.alt_s+"zoomwalkspeed"  )  g.firemodes[t.gunid][t.x].settings.zoomwalkspeed = t.value1;
						if (  t.field_s == t.alt_s+"zoomturnspeed"  )  g.firemodes[t.gunid][t.x].settings.zoomturnspeed = t.value1;
						if (  t.field_s == t.alt_s+"plrmovespeedmod"  )  g.firemodes[t.gunid][t.x].settings.plrmovespeedmod = t.value1;
						if (  t.field_s == t.alt_s+"plremptyspeedmod"  )  g.firemodes[t.gunid][t.x].settings.plremptyspeedmod = t.value1;
						if (  t.field_s == t.alt_s+"plrturnspeedmod"  )  g.firemodes[t.gunid][t.x].settings.plrturnspeedmod = t.value1;
						if (  t.field_s == t.alt_s+"plrjumpspeedmod"  )  g.firemodes[t.gunid][t.x].settings.plrjumpspeedmod = t.value1;
						if (  t.field_s == t.alt_s+"plrreloadspeedmod"  )  g.firemodes[t.gunid][t.x].settings.plrreloadspeedmod = t.value1;

						if (  t.field_s == t.alt_s+"soundstrength"  )  g.firemodes[t.gunid][t.x].settings.soundstrength = t.value1;
						if (  t.field_s == t.alt_s+"recoily"  )  g.firemodes[t.gunid][t.x].settings.recoily_f = t.value1;
						if (  t.field_s == t.alt_s+"recoilx"  )  g.firemodes[t.gunid][t.x].settings.recoilx_f = t.value1;
						if (  t.field_s == t.alt_s+"recoilyreturn"  )  g.firemodes[t.gunid][t.x].settings.recoilycorrect_f = t.value1;
						if (  t.field_s == t.alt_s+"recoilxreturn"  )  g.firemodes[t.gunid][t.x].settings.recoilxcorrect_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomrecoily"  )  g.firemodes[t.gunid][t.x].settings.zoomrecoily_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomrecoilx"  )  g.firemodes[t.gunid][t.x].settings.zoomrecoilx_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomrecoilyreturn"  )  g.firemodes[t.gunid][t.x].settings.zoomrecoilycorrect_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomrecoilxreturn"  )  g.firemodes[t.gunid][t.x].settings.zoomrecoilxcorrect_f = t.value1;
						if (  t.field_s == t.alt_s+"forcezoomout"  )  g.firemodes[t.gunid][t.x].settings.forcezoomout = t.value1;
						if (  t.field_s == t.alt_s+"ammomax"  )  g.firemodes[t.gunid][t.x].settings.ammomax = t.value1;
						if (  t.field_s == t.alt_s+"equipment"  )  g.firemodes[t.gunid][t.x].settings.equipment = t.value1;
						if (  t.field_s == t.alt_s+"lockcamera"  )  g.firemodes[t.gunid][t.x].settings.lockcamera = t.value1;

						//  Gun model animation keyframe setting
						if (  t.field_s == t.alt_s+"keyframe ratio" ) 
						{
							if (  t.i == 0  )  t.keyframeratio = t.value1; else t.altkeyframeratio = t.value1;
						}

						//  Classic animations
						if (  t.field_s == t.alt_s+"select" ) { g.firemodes[t.gunid][t.x].action.show.s = t.value1  ; g.firemodes[t.gunid][t.x].action.show.e = t.value2; }
						if (  t.field_s == t.alt_s+"idle" ) { g.firemodes[t.gunid][t.x].action.idle.s = t.value1  ; g.firemodes[t.gunid][t.x].action.idle.e = t.value2; }
						if (  t.field_s == t.alt_s+"move" ) { g.firemodes[t.gunid][t.x].action.move.s = t.value1  ; g.firemodes[t.gunid][t.x].action.move.e = t.value2; }
						if (  t.field_s == t.alt_s+"run" ) { g.firemodes[t.gunid][t.x].action.run.s = t.value1  ; g.firemodes[t.gunid][t.x].action.run.e = t.value2; }
						if (  t.field_s == t.alt_s+"fire" ) { g.firemodes[t.gunid][t.x].action.start.s = t.value1  ; g.firemodes[t.gunid][t.x].action.start.e = t.value1 ; g.firemodes[t.gunid][t.x].action.finish.s = t.value1 ; g.firemodes[t.gunid][t.x].action.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"use" ) { g.firemodes[t.gunid][t.x].action.start.s = t.value1  ; g.firemodes[t.gunid][t.x].action.start.e = t.value1 ; g.firemodes[t.gunid][t.x].action.finish.s = t.value1 ; g.firemodes[t.gunid][t.x].action.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"start fire" ) { g.firemodes[t.gunid][t.x].action.start.s = t.value1  ; g.firemodes[t.gunid][t.x].action.start.e = t.value2; }
						if (  t.field_s == t.alt_s+"automatic fire" ) { g.firemodes[t.gunid][t.x].action.automatic.s = t.value1  ; g.firemodes[t.gunid][t.x].action.automatic.e = t.value2; }
						if (  t.field_s == t.alt_s+"end fire" ) { g.firemodes[t.gunid][t.x].action.finish.s = t.value1  ; g.firemodes[t.gunid][t.x].action.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"last start fire" ) { g.firemodes[t.gunid][t.x].action.laststart.s = t.value1  ; g.firemodes[t.gunid][t.x].action.laststart.e = t.value2; }
						if (  t.field_s == t.alt_s+"last end fire" ) { g.firemodes[t.gunid][t.x].action.lastfinish.s = t.value1  ; g.firemodes[t.gunid][t.x].action.lastfinish.e = t.value2; }
						if (  t.field_s == t.alt_s+"reload" ) 
						{
							g.firemodes[t.gunid][t.x].action.startreload.s=t.value1 ; g.firemodes[t.gunid][t.x].action.startreload.e=t.value2;
							g.firemodes[t.gunid][t.x].action.reloadloop.s=t.value2 ; g.firemodes[t.gunid][t.x].action.reloadloop.e=t.value2;
							g.firemodes[t.gunid][t.x].action.endreload.s=t.value2 ; g.firemodes[t.gunid][t.x].action.endreload.e=t.value2;
						}
						if (  t.field_s == t.alt_s+"start reload" ) { g.firemodes[t.gunid][t.x].action.startreload.s = t.value1  ; g.firemodes[t.gunid][t.x].action.startreload.e = t.value2; }
						if (  t.field_s == t.alt_s+"reload loop" ) { g.firemodes[t.gunid][t.x].action.reloadloop.s = t.value1  ; g.firemodes[t.gunid][t.x].action.reloadloop.e = t.value2; }
						if (  t.field_s == t.alt_s+"end reload" ) { g.firemodes[t.gunid][t.x].action.endreload.s = t.value1  ; g.firemodes[t.gunid][t.x].action.endreload.e = t.value2; }
						if (  t.field_s == t.alt_s+"cock" ) { g.firemodes[t.gunid][t.x].action.cock.s = t.value1  ; g.firemodes[t.gunid][t.x].action.cock.e = t.value2; }
						if (  t.field_s == t.alt_s+"putaway" ) { g.firemodes[t.gunid][t.x].action.hide.s = t.value1  ; g.firemodes[t.gunid][t.x].action.hide.e = t.value2; }
						if (  t.field_s == t.alt_s+"start fire 2" ) { g.firemodes[t.gunid][t.x].action.start2.s = t.value1  ; g.firemodes[t.gunid][t.x].action.start2.e = t.value2; }
						if (  t.field_s == t.alt_s+"end fire 2" ) { g.firemodes[t.gunid][t.x].action.finish2.s = t.value1  ; g.firemodes[t.gunid][t.x].action.finish2.e = t.value2; }
						if (  t.field_s == t.alt_s+"start fire 3" ) { g.firemodes[t.gunid][t.x].action.start3.s = t.value1  ; g.firemodes[t.gunid][t.x].action.start3.e = t.value2; }
						if (  t.field_s == t.alt_s+"end fire 3" ) { g.firemodes[t.gunid][t.x].action.finish3.s = t.value1  ; g.firemodes[t.gunid][t.x].action.finish3.e = t.value2; }

						//  Advanced animations
						if (  t.field_s == t.alt_s+"fix jam" ) { g.firemodes[t.gunid][t.x].action2.clearjam.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.clearjam.e = t.value2; }
						if (  t.field_s == t.alt_s+"hand push" ) { g.firemodes[t.gunid][t.x].action2.handpush.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.handpush.e = t.value2; }
						if (  t.field_s == t.alt_s+"hand dead" ) { g.firemodes[t.gunid][t.x].action2.handdead.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.handdead.e = t.value2; }
						if (  t.field_s == t.alt_s+"jammed" ) { g.firemodes[t.gunid][t.x].action2.jammed.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.jammed.e = t.value2; }
						if (  t.field_s == t.alt_s+"change firemode" ) { g.firemodes[t.gunid][t.x].action2.swaptoalt.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.swaptoalt.e = t.value2; }
						if (  t.field_s == t.alt_s+"hand button" ) { g.firemodes[t.gunid][t.x].action2.handbutton.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.handbutton.e = t.value2; }
						if (  t.field_s == t.alt_s+"hand take" ) { g.firemodes[t.gunid][t.x].action2.handtake.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.handtake.e = t.value2; }
						if (  t.field_s == t.alt_s+"pull up" ) { g.firemodes[t.gunid][t.x].action2.pullup.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.pullup.e = t.value2; }
						if (  t.field_s == t.alt_s+"pull down" ) { g.firemodes[t.gunid][t.x].action2.pulldown.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.pulldown.e = t.value2; }
						if (  t.field_s == t.alt_s+"pull left" ) { g.firemodes[t.gunid][t.x].action2.pullleft.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.pullleft.e = t.value2; }
						if (  t.field_s == t.alt_s+"pull right" ) { g.firemodes[t.gunid][t.x].action2.pullright.s = t.value1  ; g.firemodes[t.gunid][t.x].action2.pullright.e = t.value2; }

						//  Empty animations
						if (  t.field_s == t.alt_s+"useempty"  )  g.firemodes[t.gunid][t.x].settings.hasempty = t.value1;
						if (  t.field_s == t.alt_s+"empty shotgun"  )  g.firemodes[t.gunid][t.x].settings.emptyshotgun = t.value1;
						if (  t.field_s == t.alt_s+"empty putaway" ) { g.firemodes[t.gunid][t.x].emptyaction.hide.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.hide.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty select" ) { g.firemodes[t.gunid][t.x].emptyaction.show.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.show.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty idle" ) { g.firemodes[t.gunid][t.x].emptyaction.idle.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.idle.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty move" ) { g.firemodes[t.gunid][t.x].emptyaction.move.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.move.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty run" ) { g.firemodes[t.gunid][t.x].emptyaction.run.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.run.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty start reload" ) { g.firemodes[t.gunid][t.x].emptyaction.startreload.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.startreload.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty reload loop" ) { g.firemodes[t.gunid][t.x].emptyaction.reloadloop.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.reloadloop.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty end reload" ) { g.firemodes[t.gunid][t.x].emptyaction.endreload.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.endreload.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty cock" ) { g.firemodes[t.gunid][t.x].emptyaction.cock.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.cock.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty reload" ) 
						{
							g.firemodes[t.gunid][t.x].emptyaction.startreload.s=t.value1 ; g.firemodes[t.gunid][t.x].emptyaction.startreload.e=t.value2;
							g.firemodes[t.gunid][t.x].emptyaction.reloadloop.s=t.value2 ; g.firemodes[t.gunid][t.x].emptyaction.reloadloop.e=t.value2;
							g.firemodes[t.gunid][t.x].emptyaction.endreload.s=t.value2 ; g.firemodes[t.gunid][t.x].emptyaction.endreload.e=t.value2;
						}
						if (  t.field_s == t.alt_s+"dryfire" ) { g.firemodes[t.gunid][t.x].emptyaction.dryfire.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.dryfire.e = t.value2; }

						//  Melee Animations
						if (  t.field_s == t.alt_s+"melee start" ) { g.firemodes[t.gunid][t.x].meleeaction.start.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.start.e = t.value2; }
						if (  t.field_s == t.alt_s+"melee start 2" ) { g.firemodes[t.gunid][t.x].meleeaction.start2.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.start2.e = t.value2; }
						if (  t.field_s == t.alt_s+"melee start 3" ) { g.firemodes[t.gunid][t.x].meleeaction.start3.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.start3.e = t.value2; }
						if (  t.field_s == t.alt_s+"melee end" ) { g.firemodes[t.gunid][t.x].meleeaction.finish.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"melee end 2" ) { g.firemodes[t.gunid][t.x].meleeaction.finish2.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.finish2.e = t.value2; }
						if (  t.field_s == t.alt_s+"melee end 3" ) { g.firemodes[t.gunid][t.x].meleeaction.finish3.s = t.value1  ; g.firemodes[t.gunid][t.x].meleeaction.finish3.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee start" ) { g.firemodes[t.gunid][t.x].emptyaction.start.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.start.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee start 2" ) { g.firemodes[t.gunid][t.x].emptyaction.start2.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.start2.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee start 3" ) { g.firemodes[t.gunid][t.x].emptyaction.start3.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.start3.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee end" ) { g.firemodes[t.gunid][t.x].emptyaction.finish.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee end 2" ) { g.firemodes[t.gunid][t.x].emptyaction.finish2.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.finish2.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty melee end 3" ) { g.firemodes[t.gunid][t.x].emptyaction.finish3.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyaction.finish3.e = t.value2; }
						if (  t.field_s == "block" ) { g.firemodes[t.gunid][t.x].action.block.s = t.value1 ; g.firemodes[t.gunid][t.x].action.block.e = t.value2; }

						//  Zoom Animations
						if (  t.field_s == t.alt_s+"empty zoomto" ) { g.firemodes[t.gunid][t.x].emptyzoomactionshow.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyzoomactionshow.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty zoom idle" ) { g.firemodes[t.gunid][t.x].emptyzoomactionidle.s = t.value1 ; g.firemodes[t.gunid][t.x].emptyzoomactionidle.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty zoom move" ) { g.firemodes[t.gunid][t.x].emptyzoomactionmove.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyzoomactionmove.e = t.value2; }
						if (  t.field_s == t.alt_s+"empty zoomfrom" ) { g.firemodes[t.gunid][t.x].emptyzoomactionhide.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyzoomactionhide.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom dryfire" ) { g.firemodes[t.gunid][t.x].emptyzoomactiondryfire.s = t.value1  ; g.firemodes[t.gunid][t.x].emptyzoomactiondryfire.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoomto" ) { g.firemodes[t.gunid][t.x].zoomaction.show.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.show.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoomfrom" ) { g.firemodes[t.gunid][t.x].zoomaction.hide.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.hide.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom start fire" ) { g.firemodes[t.gunid][t.x].zoomaction.start.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.start.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom automatic fire" ) { g.firemodes[t.gunid][t.x].zoomaction.automatic.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.automatic.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom end fire" ) { g.firemodes[t.gunid][t.x].zoomaction.finish.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.finish.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom last start fire" ) { g.firemodes[t.gunid][t.x].zoomaction.laststart.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.laststart.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom last end fire" ) { g.firemodes[t.gunid][t.x].zoomaction.lastfinish.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.lastfinish.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom idle" ) { g.firemodes[t.gunid][t.x].zoomaction.idle.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.idle.e = t.value2; }
						if (  t.field_s == t.alt_s+"zoom move" ) { g.firemodes[t.gunid][t.x].zoomaction.move.s = t.value1  ; g.firemodes[t.gunid][t.x].zoomaction.move.e = t.value2; }

						//  Gun repeat-fire sound
						if (  t.field_s == t.alt_s+"fireloop"  )  g.firemodes[t.gunid][t.x].sound.fireloopend = t.value1;

						//  Gun and muzzle alignment
						if (  t.field_s == t.alt_s+"horiz"  )  g.firemodes[t.gunid][t.x].horiz_f = t.value1;
						if (  t.field_s == t.alt_s+"vert"  )  g.firemodes[t.gunid][t.x].vert_f = t.value1;
						if (  t.field_s == t.alt_s+"forward"  )  g.firemodes[t.gunid][t.x].forward_f = t.value1;
						if (  t.field_s == t.alt_s+"alignx"  )  g.firemodes[t.gunid][t.x].settings.muzzlex_f = t.value1;
						if (  t.field_s == t.alt_s+"aligny"  )  g.firemodes[t.gunid][t.x].settings.muzzley_f = t.value1;
						if (  t.field_s == t.alt_s+"alignz"  )  g.firemodes[t.gunid][t.x].settings.muzzlez_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomalignx"  )  g.firemodes[t.gunid][t.x].settings.zoommuzzlex_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomaligny"  )  g.firemodes[t.gunid][t.x].settings.zoommuzzley_f = t.value1;
						if (  t.field_s == t.alt_s+"zoomalignz"  )  g.firemodes[t.gunid][t.x].settings.zoommuzzlez_f = t.value1;
						if (  t.field_s == t.alt_s+"rotx"  )  g.firemodes[t.gunid][t.x].settings.rotx_f = t.value1;
						if (  t.field_s == t.alt_s+"roty"  )  g.firemodes[t.gunid][t.x].settings.roty_f = t.value1;
						if (  t.field_s == t.alt_s+"rotz"  )  g.firemodes[t.gunid][t.x].settings.rotz_f = t.value1;

						//  Gun particle effect
						if (  t.field_s == t.alt_s+"particledecal"  )  g.firemodes[t.gunid][t.x].particle.decal_s = t.value_s;
						if (  t.field_s == t.alt_s+"particlespeed"  )  g.firemodes[t.gunid][t.x].particle.velocity = t.value1;

						//  Advanced ammo pooling system
						if (  t.field_s == t.alt_s+"poolammo" ) 
						{
							if (  t.value_s != "" ) 
							{
								t.poolindex=0 ; t.emptyindex=0;
								for ( t.y = 1 ; t.y<=  100; t.y++ )
								{
									if (  t.ammopool[t.y].name_s == t.value_s ) { t.poolindex = t.y  ; break; }
									if (  t.ammopool[t.y].used == 0 && t.emptyindex == 0  )  t.emptyindex = t.y;
								}
								if (  t.poolindex == 0  )  t.poolindex = t.emptyindex;
								if (  t.poolindex>0 ) 
								{
									g.firemodes[t.gunid][t.x].settings.poolindex=t.poolindex;
									t.ammopool[t.poolindex].used=1 ; t.ammopool[t.poolindex].name_s=t.value_s;
								}
							}
						}

					}

					//  Alternate Fire settings
					if (  t.field_s == "alternateisflak"  )  t.gun[t.gunid].settings.alternateisflak = t.value1;
					if (  t.field_s == "alternateisray"  )  t.gun[t.gunid].settings.alternateisray = t.value1;
					if (  t.field_s == "altto" || t.field_s == "alt to" ) { t.gun[t.gunid].altaction.to.s = t.value1  ; t.gun[t.gunid].altaction.to.e = t.value2; }
					if (  t.field_s == "altfrom" || t.field_s == "alt from" ) { t.gun[t.gunid].altaction.from.s = t.value1  ; t.gun[t.gunid].altaction.from.e = t.value2; }
					if (  t.field_s == "empty altto" || t.field_s == "empty alt to" ) { t.gun[t.gunid].emptyaltactionto.s = t.value1  ; t.gun[t.gunid].emptyaltactionto.e = t.value2; }
					if (  t.field_s == "empty altfrom" || t.field_s == "empty alt from" ) { t.gun[t.gunid].emptyaltactionfrom.s = t.value1  ; t.gun[t.gunid].emptyaltactionfrom.e = t.value2; }
					if (  t.field_s == "addtospare"  )  t.gun[t.gunid].settings.addtospare = t.value1;
					if (  t.field_s == "shareloadedammo"  )  t.gun[t.gunid].settings.modessharemags = t.value1;

					//  Bullet control
					if (  t.field_s == "bullethidemod"  )  t.gun[t.gunid].settings.bulletmod = t.value1;
					if (  t.field_s == "bullethidereset"  )  t.gun[t.gunid].settings.bulletreset = t.value1;
					if (  t.field_s == "bulletlimbtotal"  )  t.gun[t.gunid].settings.bulletlimbsmax = t.value1;
					if (  t.field_s == "bullethidestart" ) 
					{
						t.gun[t.gunid].settings.bulletamount=t.value1;
						t.gun[t.gunid].settings.currentbullet=t.value1+1;
					}

					//  Gun sound bank
					for ( t.p = 1 ; t.p<=  14; t.p++ )
					{
						t.tryfield_s = "" ; t.tryfield_s=t.tryfield_s+"sound"+Str(t.p);
						if (  t.field_s == t.tryfield_s  )  t.gunsound[t.gunid][t.p].name_s = t.value_s;
					}
					for ( t.p = 1 ; t.p<=  4; t.p++ )
					{
						t.tryfield_s = ""; t.tryfield_s=t.tryfield_s+"altsound"+Str(t.p) ; t.tryfield2_s = "" ; t.tryfield2_s=t.tryfield2_s+"alt sound"+Str(t.p);
						if (  t.p != 2 ) 
						{
							if (  t.field_s == t.tryfield_s || t.field_s == t.tryfield2_s  )  t.gunsound[t.gunid][t.p].altname_s = t.value_s;
						}
					}

					//  Gun sound trigger frames
					if (  t.field_s == "soundframes"  )  t.gun[t.gunid].sound.soundframes = t.value1;
					if (  t.gun[t.gunid].sound.soundframes>0 ) 
					{
						for ( t.p = 0 ; t.p<=  t.gun[t.gunid].sound.soundframes; t.p++ )
						{
							t.tryfield_s = ""; t.tryfield_s=t.tryfield_s+"sframe"+Str(t.p);
							if (  t.field_s == t.tryfield_s ) 
							{
								t.gunsounditem[t.gunid][t.p].keyframe=(t.value1*t.keyframeratio);
								t.gunsounditem[t.gunid][t.p].playsound=t.value2;
							}
						}
					}
				}
			}
		}
		UnDim (  t.data_s );
	}

	// Correct any legacy fall-out
	if ( cstr(Lower(t.gun[t.gunid].texd_s.Get())) == "gun_d2.dds"  )  t.gun[t.gunid].texd_s = "gun_d.dds";

	// 130418 - also replace any old TGA references
	char pTexFileName[1024];
	strcpy ( pTexFileName, t.gun[t.gunid].texd_s.Get() );
	if ( stricmp ( pTexFileName+strlen(pTexFileName)-4, ".tga") == NULL )
	{
		pTexFileName[strlen(pTexFileName)-4] = 0;
		strcat ( pTexFileName, ".png" );
		t.gun[t.gunid].texd_s = pTexFileName;
	}

	//  Go through gun settings and populate with defaults
	for ( t.i = 0 ; t.i<=  1; t.i++ )
	{

		//  If no run, replace with regular move action
		if (  g.firemodes[t.gunid][t.i].action.run.e == 0  )  g.firemodes[t.gunid][t.i].action.run = g.firemodes[t.gunid][t.i].action.move;
		if (  g.firemodes[t.gunid][t.i].zoomaction.run.e == 0  )  g.firemodes[t.gunid][t.i].zoomaction.run = g.firemodes[t.gunid][t.i].zoomaction.move;

		//  If no COCK animation, fill with end of reload data
		if (  g.firemodes[t.gunid][t.i].action.cock.e == 0 ) 
		{
			g.firemodes[t.gunid][t.i].action.cock.s=g.firemodes[t.gunid][t.i].action.endreload.e;
			g.firemodes[t.gunid][t.i].action.cock.e=g.firemodes[t.gunid][t.i].action.endreload.e;
		}

		//  Fill empty animations if no animation found
		if (  g.firemodes[t.gunid][t.i].emptyaction.hide.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.hide = g.firemodes[t.gunid][t.i].action.hide;
		if (  g.firemodes[t.gunid][t.i].emptyaction.show.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.show = g.firemodes[t.gunid][t.i].action.show;
		if (  g.firemodes[t.gunid][t.i].emptyaction.idle.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.idle = g.firemodes[t.gunid][t.i].action.idle;
		if (  g.firemodes[t.gunid][t.i].emptyaction.move.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.move = g.firemodes[t.gunid][t.i].action.move;
		if (  g.firemodes[t.gunid][t.i].emptyaction.cock.e == 0 && g.firemodes[t.gunid][t.i].emptyaction.endreload.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.cock = g.firemodes[t.gunid][t.i].action.cock;
		if (  g.firemodes[t.gunid][t.i].emptyaction.startreload.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.startreload = g.firemodes[t.gunid][t.i].action.startreload;
		if (  g.firemodes[t.gunid][t.i].emptyaction.reloadloop.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.reloadloop = g.firemodes[t.gunid][t.i].action.reloadloop;
		if (  g.firemodes[t.gunid][t.i].emptyaction.endreload.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.endreload = g.firemodes[t.gunid][t.i].action.endreload;
		if (  g.firemodes[t.gunid][t.i].emptyaction.run.e == 0  )  g.firemodes[t.gunid][t.i].emptyaction.run = g.firemodes[t.gunid][t.i].emptyaction.move;

		//  If no COCK animation, fill with end of reload data
		if (  g.firemodes[t.gunid][t.i].emptyaction.cock.e == 0 ) 
		{
			g.firemodes[t.gunid][t.i].emptyaction.cock.s=g.firemodes[t.gunid][t.i].emptyaction.endreload.e;
			g.firemodes[t.gunid][t.i].emptyaction.cock.e=g.firemodes[t.gunid][t.i].emptyaction.endreload.e;
		}

		//  If no muzzle colour, go with default
		if (  g.firemodes[t.gunid][t.i].settings.muzzlecolorr == 0 && g.firemodes[t.gunid][t.i].settings.muzzlecolorg == 0 && g.firemodes[t.gunid][t.i].settings.muzzlecolorb == 0 ) 
		{
			g.firemodes[t.gunid][t.i].settings.muzzlecolorr=255;
			g.firemodes[t.gunid][t.i].settings.muzzlecolorg=255;
			g.firemodes[t.gunid][t.i].settings.muzzlecolorb=0;
		}

		//  Set default smoke speed
		if (  g.firemodes[t.gunid][t.i].settings.smokespeed == 0 ) 
		{
			g.firemodes[t.gunid][t.i].settings.smokespeed=25;
		}

		//  Set default rate of fire
		if (  g.firemodes[t.gunid][t.i].settings.firerate == 0 ) 
		{
			g.firemodes[t.gunid][t.i].settings.firerate=12;
		}

		//  Set default force
		if (  g.firemodes[t.gunid][t.i].settings.force == 0 ) 
		{
			g.firemodes[t.gunid][t.i].settings.force=g.firemodes[t.gunid][t.i].settings.damage;
		}
		if (  g.firemodes[t.gunid][t.i].settings.meleeforce == 0 ) 
		{
			g.firemodes[t.gunid][t.i].settings.meleeforce=g.firemodes[t.gunid][t.i].settings.damage;
		}

		//  Default smoke decal
		if (  g.firemodes[t.gunid][t.i].settings.smokedecal_s == "" ) 
		{
			g.firemodes[t.gunid][t.i].settings.smokedecal_s="smoke1";
		}

		//  Find the decal specified
		g.firemodes[t.gunid][t.i].decalid=0;
		if (  g.firemodes[t.gunid][t.i].decal_s != "" ) 
		{
			t.decal_s=g.firemodes[t.gunid][t.i].decal_s ; decal_find ( );
			if (  t.decalid<0 ) 
			{
				t.decalid=0;
			}
			else
			{
				t.decal[t.decalid].active=1;
				g.firemodes[t.gunid][t.i].decalid=t.decalid;
			}
		}
	}
}

void gun_scaninall_dataonly ( void )
{
	for ( t.gunid = 1 ; t.gunid<=g.gunmax; t.gunid++ )
	{
		t.gun_s=t.gun[t.gunid].name_s; 
		gun_loaddata ( );
	}
}

void gun_scaninall_ref ( void )
{
	//  Scan entire guns folder
	SetDir (  "gamecore"  ); t.gunid=1;
	UnDim ( t.filelist_s );
	buildfilelist(g.fpgchuds_s.Get(),"");
	SetDir (  ".." );
	if (  ArrayCount(t.filelist_s)>0 ) 
	{
		for ( t.chkfile = 0 ; t.chkfile<=  ArrayCount(t.filelist_s); t.chkfile++ )
		{
			t.file_s=t.filelist_s[t.chkfile];
			if (  t.file_s != "." && t.file_s != ".." ) 
			{
				if (  cstr(Lower(Right(t.file_s.Get(),11))) == "gunspec.txt" ) 
				{
					t.gun[t.gunid].name_s=Left(t.file_s.Get(),Len(t.file_s.Get())-12);
					t.strwork = ""; t.strwork = t.strwork + "gun "+Str(t.gunid)+":"+t.file_s;
					timestampactivity(0, t.strwork.Get() );
					++t.gunid;
					if (  t.gunid>g.maxgunsinengine  )  t.gunid = g.maxgunsinengine;
				}
			}
		}
		g.gunmax=t.gunid-1;
		t.strwork = ""; t.strwork = t.strwork + "total guns="+Str(g.gunmax);
		timestampactivity(0, t.strwork.Get() );
	}

	//  Now sort the gun list into alphabetical order (MP needs gunid identical on each PC)
	gun_sortintoorder ( );
}

void gun_sortintoorder ( void )
{
	//  Now sort the gun list into alphabetical order
	for ( t.tgid1 = 1 ; t.tgid1<=  g.gunmax; t.tgid1++ )
	{
		for ( t.tgid2 = 1 ; t.tgid2<=  g.gunmax; t.tgid2++ )
		{
			if (  t.tgid1 != t.tgid2 ) 
			{
				t.tname1_s=Lower(t.gun[t.tgid1].name_s.Get());
				t.tname2_s=Lower(t.gun[t.tgid2].name_s.Get());
				//C++ISSUE
				if (  strlen( t.tname1_s.Get() ) > strlen( t.tname2_s.Get() ) ) 
				{
					//  smallest at top
					t.gun[t.tgid1].name_s=t.tname2_s;
					t.gun[t.tgid2].name_s=t.tname1_s;
				}
			}
		}
	}
}

void gun_findweaponindexbyname_core ( void )
{
	t.foundgunid=0;
	if (  t.findgun_s != "" ) 
	{
		for ( t.tid = 1 ; t.tid<=g.gunmax; t.tid++ )
		{
			//timestampactivity(0, cstr(cstr("FindWeaponMatch: ")+t.findgun_s+cstr(" vs ")+t.gun[t.tid].name_s).Get());
			if (  cstr(Lower(t.findgun_s.Get())) == cstr(Lower(t.gun[t.tid].name_s.Get())) ) 
			{
				t.foundgunid=t.tid;
				break;
			}
		}
	}
}

void gun_scaninall_findnewlyaddedgun ( void )
{
	//  if find gun that is not in list, if find, flag is 'found'
	t.storegunid=t.gunid;

	//  gather files
	SetDir (  "gamecore" );
	UnDim ( t.filelist_s );
	buildfilelist(g.fpgchuds_s.Get(),"");
	SetDir (  ".." );

	//  go through file list of latest guns
	if (  ArrayCount(t.filelist_s)>0 ) 
	{
		for ( t.chkfile = 0 ; t.chkfile<=  ArrayCount(t.filelist_s); t.chkfile++ )
		{
			t.file_s=t.filelist_s[t.chkfile];
			if (  t.file_s != "." && t.file_s != ".." ) 
			{
				t.findgun_s="";
				if ( cstr( Lower(Right(t.file_s.Get(),11))) == "gunspec.txt" ) 
				{
					t.findgun_s=Left(t.file_s.Get(),Len(t.file_s.Get())-12);
				}
				if (  t.findgun_s != "" ) 
				{
					gun_findweaponindexbyname_core ( );
					if (  t.foundgunid == 0 ) 
					{
						++g.gunmax;
						if (  g.gunmax>g.maxgunsinengine  )  g.gunmax = g.maxgunsinengine;
						t.gun[g.gunmax].name_s=t.findgun_s;
						t.gunid=g.gunmax ; t.gun_s=t.findgun_s ; gun_loaddata ( );
					}
				}
			}
		}
	}
	t.gunid=t.storegunid;
}

void gun_findweaponindexbyname ( void )
{
	//  try to find gun
	t.storefindgun_s=t.findgun_s; 
	gun_findweaponindexbyname_core ( );

	//  if not found, try rescanning (as new guns can be added by store), and try a second time
	if (  t.foundgunid == 0 ) 
	{
		gun_scaninall_findnewlyaddedgun ( );
		t.findgun_s=t.storefindgun_s; 
		gun_findweaponindexbyname_core ( );
	}
}

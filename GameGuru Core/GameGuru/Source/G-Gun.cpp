//----------------------------------------------------
//--- GAMEGURU - G-Gun
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"


// global store for weapon shader effect indexes
int g_weaponbasicshadereffectindex = 0;
int g_weaponboneshadereffectindex = 0;

// 
//  GUN CORE
// 

void gun_restart ( void )
{
	//  No gun to start with
	g.weaponammoindex=0;
	g.autoloadgun=-1 ; t.gunid=0;
	t.triggerweapononeifexists=1;
	for ( t.t = 1 ; t.t<=  9; t.t++ )
	{
		t.weaponslot[t.t].pref=0;
		t.weaponslot[t.t].got=0;
	}
	for ( t.t = 1 ; t.t<=  20; t.t++ )
	{
		t.weaponammo[t.t]=0;
		t.weaponclipammo[t.t]=0;
	}
	for ( t.i = 1 ; t.i<=  100; t.i++ )
	{
		t.ammopool[t.i].ammo=0;
	}

	//  set maximum slots allowed (for games that allow only a few weapons to be carried)
	//  LEE, find out if these are set elsewhere and remove (and move this code to coirrect place)
	g.maxslots=10;
	g.autoswap=1;
}

void gun_resetactivateguns ( void )
{
	// reset gun activations before levels start
	for ( t.tgunid = 1 ; t.tgunid<=  g.gunmax; t.tgunid++ )
	{
		t.gun[t.tgunid].activeingame=0;
	}

	// some temp variables in the main gun structure
	for ( t.tgunid = 1 ; t.tgunid<=  g.gunmax; t.tgunid++ )
	{
		t.gun[t.tgunid].settings.canaddtospare=0;
		t.gun[t.tgunid].settings.ismelee=0;
	}
}

void gun_activategunsfromentities ( void )
{
	// Only flag those guns present in level
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			if ( t.entityprofile[t.entid].ismarker == 1 ) 
			{
				// Entity is Start Marker
				t.tgunid=t.entityelement[t.e].eleprof.hasweapon;
				if ( t.tgunid>0 ) 
				{
					t.gun[t.tgunid].activeingame=1;
				}
			}
			else
			{
				// Entity Is A Gun
				t.tgunid=t.entityprofile[t.entid].isweapon;
				if ( t.tgunid>0 ) t.gun[t.tgunid].activeingame = 1;

				// Entity is carrying a gun
				t.tgunid = t.entityprofile[t.entid].hasweapon;
				if ( t.tgunid>0 ) 
				{
					t.gun[t.tgunid].activeingame=1;
				}
			}
		}
	}
}

void gun_decaldetails ( void )
{
	for ( t.i = 1 ; t.i<=  g.gunmax; t.i++ )
	{
		for ( t.y = 0 ; t.y<=  1; t.y++ )
		{
			if (  g.firemodes[t.i][t.y].particle.decal_s != "" ) 
			{
				t.decal_s=g.firemodes[t.i][t.y].particle.decal_s;
				decal_find ( );
				if (  t.decalid != -1 ) 
				{
					g.firemodes[t.i][t.y].particle.id=t.decalid;
				}
				else
				{
					g.firemodes[t.i][t.y].particle.decal_s="";
				}
			}
		}
	}
}

void gun_loadonlypresent ( void )
{
	//  Load all guns that have been activated
	for ( t.gunid = 1 ; t.gunid<=  g.gunmax; t.gunid++ )
	{
		if (  t.gun[t.gunid].activeingame == 1 ) 
		{
			t.gun_s=t.gun[t.gunid].name_s ; gun_load ( );
		}
	}

	//  And now fill in player weapon details
	for ( t.tww = 1 ; t.tww<=  9; t.tww++ )
	{
		t.gunid=t.weaponslot[t.tww].pref;
		if (  t.gunid>0 ) 
		{
			if (  t.gunid <= ArrayCount(t.gun) ) 
			{
				if (  t.gun[t.gunid].activeingame == 1 ) 
				{
					t.weaponhud[t.tww]=t.gun[t.gunid].hudimage;
				}
			}
		}
	}

	//  Ensure gun vars are reset
	t.gunid=0;
}

void gun_resetgunsettings ( void )
{
	int ws = 0;
	//  Reset weapons (1=restart)
	if (  t.tcopyorrestart == 0 ) 
	{
		//  copy
		Dim (  t.copyweaponslot,10 );
		//  AirMod - Next 2 Lines modified for Alt Fire
		Dim (  t.copyweaponammo,20 );
		Dim (  t.copyweaponclipammo,20 );
		Dim (  t.copyweaponhud,10 );
		for ( ws = 1 ; ws<= 20; ws++ )
		{
			//  AirMod - Line (  Modified for Alt Fire )
			if (  ws < 11  )  t.copyweaponslot[ws] = t.weaponslot[ws];
			t.copyweaponammo[ws]=t.weaponammo[ws];
			t.copyweaponclipammo[ws]=t.weaponclipammo[ws];
			//  AirMod - Line (  Modified for Alt Fire )
			if (  ws < 11  )  t.copyweaponhud[ws] = t.weaponhud[ws];
		}
	}
	else
	{
		//  restore
		//  AirMod - Line (  Modified for alt Fire )
		for ( ws = 1 ; ws<=  20; ws++ )
		{
			//  AirMod - Line (  Modified for Alt Fire )
			if (  ws < 11  )  t.weaponslot[ws] = t.copyweaponslot[ws];
			t.weaponammo[ws]=t.copyweaponammo[ws];
			t.weaponclipammo[ws]=t.copyweaponclipammo[ws];
			//  AirMod - Line (  Modified for Alt Fire )
			if (  ws < 11  )  t.weaponhud[ws] = t.copyweaponhud[ws];
		}
	}
}

void gun_manager ( void )
{
	// exit early if gun system disabled
	if ( ObjectExist(g.hudbankoffset+5) == 0  ) 
		return;

	// new freeze mode (PLRDISABLE) which stops player attacking
	t.gunclick=t.player[1].state.firingmode;
	if ( t.gunclick == 0 && t.gunmode >= 7 ) t.gunandmelee.pressedtrigger = 0;
	if ( g.mefrozen>0 && g.mefrozentype == 2  )  t.gunclick = 0;
	if ( t.gunclick == 1 && g.firemodes[t.gunid][g.firemode].settings.disablerunandshoot == 1 && t.playercontrol.isrunning == 1 && t.player[1].state.moving == 1  )  t.gunclick = 0;
	if ( t.gunclick == 1 && (g.lowfpswarning == 1 || g.lowfpswarning == 2)  )  t.gunclick = 0;

	// Melee control (for TPP) 
	bool bGunshotOverridden = false;
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		// sometimes gunid can be zero? (start marker/fantasy ranger=zero)
		t.gunid = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.hasweapon;
		bool bWeaponIsMeleeBased = false;
		if (t.gunid > 0)
			if (t.gun[t.gunid].statuspanelcode == 7 || t.gun[t.gunid].statuspanelcode == 8 || t.gun[t.gunid].statuspanelcode == 10) // bow, axe, sword
				bWeaponIsMeleeBased = true;
		if (t.gunid == 0 || bWeaponIsMeleeBased == true)
		{
			// use gunclick to detect one off press for melee
			if (t.gunclick != 1)	t.gunmustreleasefirst = 0;
			if (t.gunclick != 0)
			{
				if (t.gunclick == 1 && t.gunmustreleasefirst == 0)
				{
					// if third person, detect melee override of LMB shooting
					int iE = t.playercontrol.thirdperson.charactere;
					int iWeaponIndex = t.entityelement[iE].eleprof.hasweapon;
					if (iWeaponIndex == 0)
					{
						// but only trigger 'punch' melee if specified in FPE
						int entid = t.entityelement[iE].bankindex;
						t.q = t.entityprofile[entid].startofaianim;
						if (t.entityanim[entid][t.q + t.csi_stoodpunch[1]].start > 0)
						{
							t.charanimcontrols[t.playercontrol.thirdperson.characterindex].meleeing = 1;
						}
						bGunshotOverridden = true;
					}
					else
					{
						if (bWeaponIsMeleeBased == true)
						{
							int entid = t.entityelement[iE].bankindex;
							t.q = t.entityprofile[entid].startofaianim;
							if (t.entityanim[entid][t.q + t.csi_stoodpunch[1]].start > 0)
							{
								t.charanimcontrols[t.playercontrol.thirdperson.characterindex].meleeing = 1;
							}
							bGunshotOverridden = true;
						}
					}
				}
			}
		}
	}

	// Gun controls
	if ( t.gunmode < 100 && bGunshotOverridden == false ) 
	{
		//  Gun Firing
		if (  t.gunclick != 1  )
			t.gunmustreleasefirst = 0;
		if (  t.gunclick != 0 ) 
		{
			bool bGunshotOverridden = false;
			if ( t.gunclick == 1 && t.gunmustreleasefirst == 0 && t.gunandmelee.pressedtrigger == 0 )
			{
				// if gun ready, trigger a shot (only if t.gunandmelee.pressedtrigger is zero so dry fire anim does not freeze)
				if ( t.gunmode >= 5 && t.gunmode <= 26 ) 
				{
					t.gunmodelast = t.gunmode;
					t.gunmode = 101;
					if ( t.game.runasmultiplayer == 1 ) mp_shoot ( );
				}
			}
			t.gunfull=1;
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] < g.firemodes[t.gunid][g.firemode].settings.reloadqty+1 && g.firemodes[t.gunid][g.firemode].settings.chamberedround > 0 )  t.gunfull  =  0;
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] < g.firemodes[t.gunid][g.firemode].settings.reloadqty && g.firemodes[t.gunid][g.firemode].settings.chamberedround == 0 )  t.gunfull  =  0;
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset]>0 && g.firemodes[t.gunid][g.firemode].settings.emptyreloadonly  ==  1 )  t.gunfull = 1;
			if ( t.gunfull == 1 ) g.plrreloading = 0;
			if ( t.gunmode < 30 ) 
			{
				if ( t.gunclick == 2 && (t.gunfull == 0 || g.firemodes[t.gunid][g.firemode].settings.nofullreload == 0)  )  
				{
					float ammoleft = t.weaponammo[g.weaponammoindex + g.ammooffset];
					float ammoleft2 = g.firemodes[t.gunid][g.firemode].settings.chamberedround;
					t.tpool = g.firemodes[t.gunid][g.firemode].settings.poolindex;
					if (t.tpool == 0)
						ammoleft2 = t.weaponclipammo[g.weaponammoindex + g.ammooffset];
					else 
						ammoleft2 = t.ammopool[t.tpool].ammo;
					bool bValid = true;

					if( g.firemodes[t.gunid][g.firemode].settings.minreloadqty > 0 && ammoleft2 == 0 )
					{
						//PE: Prevent reload if lower then minreloadqty.
						if (g.firemodes[t.gunid][g.firemode].settings.jammed != 1)
						{
							if (ammoleft + ammoleft2 < g.firemodes[t.gunid][g.firemode].settings.minreloadqty)
								bValid = false;
						}
					}

					// 110718 - ensure cannot reload while running or transitioning (will reload as soon as transition ends)
					if (bValid && t.playercontrol.usingrun == -1 && (t.gunmode<27 || t.gunmode>28) )
					{
						t.gunmode = 121; //Reload
					}
				}
			}
		}
		else
		{
			//  Gun Movement
			if (  t.gunmode<21 || t.gunmode>39 ) 
			{
				//  AirMod - Enable MOVE animation in crouch
				if (  t.player[1].state.moving != 0  )  t.gunmode = 21;
			}
		}
	}

	//  gun Blocking
	t.block = t.player[1].state.blockingaction;
	if (  t.block  ==  1 && t.gunmode<100 ) 
	{
		t.gunmode=1001;
	}
	else
	{
		if (  t.block  !=  2  )  t.player[1].state.blockingaction  =  0;
	}

	// trigger melee attack
	if ( t.gun[t.gunid].settings.ismelee>0 && t.gunmode<100 ) 
	{
		if (  g.firemodes[t.gunid][g.firemode].meleeaction.start.s>0 ) 
		{
			//if (  t.gunzoommode == 10  )  t.gunzoommode = 11;
			if ( t.gunzoommode >=8 ) t.gunzoommode = 11; // catches all states of a zoomed in state
			if ( t.gun[t.gunid].settings.ismelee == 2 ) t.gunmode = 1020;
			if ( g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 && g.firemodes[t.gunid][g.firemode].settings.simplezoomanim != 0 && t.gunzoommode != 0 ) 
			{
				t.gunmode=2003;
			}
			else
			{
				t.gun[t.gunid].settings.ismelee=2;
			}
		}
		else
		{
			t.gun[t.gunid].settings.ismelee=0;
		}
	}

	//  update HUD object with flash img
	if (  g.firemodes[t.gunid][g.firemode].settings.flashimg != t.gunandmelee.hudbankcrosshairtexture ) 
	{
		TextureObject (  g.hudbankoffset+5,0,g.firemodes[t.gunid][g.firemode].settings.flashimg );
		t.gunandmelee.hudbankcrosshairtexture=g.firemodes[t.gunid][g.firemode].settings.flashimg;
	}

	//  gun selection
	if (  t.gunmode<31 || t.gunmode>35 ) 
	{
		if (  t.player[1].command.newweapon>0 ) 
		{
			t.sel=t.player[1].command.newweapon;
			t.player[1].command.newweapon=0;
			if (  g.weaponammoindex>0 ) 
			{
				//  only if 'different weapon'
				if (  t.weaponslot[g.weaponammoindex].pref != t.sel ) 
				{
					t.gunmode=31 ; t.gunselectionafterhide=t.sel;
					t.gunandmelee.tmouseheld=0;
				}
				else
				{
					//  Alternate Fire
					if (  g.ggunaltswapkey1 == -1 ) 
					{
						if (  (t.gun[t.gunid].settings.alternateisflak  ==  1 || t.gun[t.gunid].settings.alternateisray  ==  1) && t.gunmode  <=  100 ) 
						{
							if (  t.gun[t.gunid].settings.alternate  ==  1 ) { t.gunmode = 2009  ; t.gun[t.gunid].settings.alternate  =  0 ; } else { t.gunmode = 2007 ; t.gun[t.gunid].settings.alternate  =  1; }
						}
					}
				}
			}
			else
			{
				t.gunmode=131 ; g.autoloadgun=t.sel;
				if (  g.autoloadgun != t.gunid  )  t.gunandmelee.tmouseheld = 0;
			}
		}
	}

	//  Change weapon
	gun_change ( );

	//  Need to update hud object for gun here (and again after Sync ( ) )
	gun_update_hud ( );

	// Gun control
	if ( t.gunid > 0 ) 
	{
		if ( ObjectExist(t.currentgunobj) == 1 ) 
		{
			// handle gun and soundcontrol
			if ( g.firemode != t.gun[t.gunid].settings.alternate )
			{
				t.tfireanim = 0;
				t.tmeleeanim = 0;
			}
			g.firemode=t.gun[t.gunid].settings.alternate;
			change_brass_firemode(); //PE: check if we need to change to another brass set.
			g.ammooffset=g.firemode*10;
			if (  t.gun[t.gunid].settings.modessharemags == 1  )  g.ammooffset = 0;
			if ( t.player[t.plrid].health>0 ) gun_control ( );
			gun_shoot ( );
			if (  t.playercontrol.thirdperson.enabled == 0 ) 
			{
				if ( t.gun[t.gunid].settings.flashlimb>=0 ) gun_flash ( );
				if ( t.gun[t.gunid].settings.brasslimb>=0 ) gun_brass ( );
				if ( t.gun[t.gunid].settings.smokelimb>=0 ) gun_smoke ( );
			}
			gun_soundcontrol ( );

			//  handle replacing of projectile for FLAK weapons and hiding it when no ammo left
			t.flakid=g.firemodes[t.gunid][g.firemode].settings.flakindex;
			if (  t.flakid>0 ) 
			{
				if (  g.firemodes[t.gunid][g.firemode].settings.flaklimb != -1 ) 
				{
					t.tshowammobeingloaded=0;
					if (  t.gun[t.gunid].projectileframe == 0 ) 
					{
						if (  GetFrame(t.currentgunobj) >= g.firemodes[t.gunid][g.firemode].action.startreload.s && GetFrame(t.currentgunobj) <= g.firemodes[t.gunid][g.firemode].action.startreload.e  )  t.tshowammobeingloaded = 1;
						if (  GetFrame(t.currentgunobj) >= g.firemodes[t.gunid][g.firemode].action.reloadloop.s && GetFrame(t.currentgunobj) <= g.firemodes[t.gunid][g.firemode].action.reloadloop.e  )  t.tshowammobeingloaded = 1;
						if (  GetFrame(t.currentgunobj) >= g.firemodes[t.gunid][g.firemode].action.endreload.s && GetFrame(t.currentgunobj) <= g.firemodes[t.gunid][g.firemode].action.endreload.e  )  t.tshowammobeingloaded = 1;
					}
					else
					{
						if (  GetFrame(t.currentgunobj) >= g.firemodes[t.gunid][g.firemode].action.start.s && GetFrame(t.currentgunobj) <= t.gun[t.gunid].projectileframe  )  t.tshowammobeingloaded = 1;
					}
					if (  t.weaponammo[g.weaponammoindex] == 0 && t.tshowammobeingloaded == 0 ) 
					{
						HideLimb (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].settings.flaklimb );
					}
					else
					{
						//  hide IMMEDIATELY or DELAYED
						t.thideprojectileinhudmodel=0;
						if (  t.gun[t.gunid].projectileframe == 0 ) 
						{
							//  RPG - always hide rocket as it leaves launcher IMMEDIATELY
							if (  GetFrame(t.currentgunobj) >= g.firemodes[t.gunid][g.firemode].action.start.s && GetFrame(t.currentgunobj) <= g.firemodes[t.gunid][g.firemode].settings.flakrearmframe ) 
							{
								t.thideprojectileinhudmodel=1;
							}
						}
						else
						{
							//  HAND GRENADE - hide grenade after the throw
							if (  GetFrame(t.currentgunobj) >= t.gun[t.gunid].projectileframe && GetFrame(t.currentgunobj) <= g.firemodes[t.gunid][g.firemode].settings.flakrearmframe ) 
							{
								t.thideprojectileinhudmodel=1;
							}
						}
						if (  t.thideprojectileinhudmodel == 1 ) 
						{
							HideLimb (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].settings.flaklimb );
						}
						else
						{
							ShowLimb (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].settings.flaklimb );
						}
					}
				}
			}
		}
	}

	// TPP Melee Control
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		if ( t.charanimcontrols[t.playercontrol.thirdperson.characterindex].meleeing != 0 )
		{
			// player meleeing, detect strike frames and inflict damage on any entity in front of character
			int iE = t.playercontrol.thirdperson.charactere;
			int iObj = t.entityelement[iE].obj;
			if ( iObj > 0 )
			{
				if ( ObjectExist ( iObj ) == 1 )
				{
					float fCurrentFrame = GetFrame(iObj);
					float fCurrentAngle = WrapValue ( ObjectAngleY ( iObj ) );
					int iEntID = t.entityelement[iE].bankindex;
					t.q = t.entityprofile[iEntID].startofaianim;
					float fStrikeRadius = t.entityprofile[iEntID].meleerange;
					float fStrikeStart = t.entityanim[iEntID][t.q+t.csi_stoodpunch[1]].start;
					float fStrikeFinish = t.entityanim[iEntID][t.q+t.csi_stoodpunch[1]].finish;
					float fStrikeThird = (fStrikeFinish-fStrikeStart)/3.0f;
					fStrikeStart += fStrikeThird;
					fStrikeFinish -= fStrikeThird;
					if ( t.entityprofile[iEntID].meleestrikest > 0 )
					{
						fStrikeStart = t.entityprofile[iEntID].meleestrikest;
						fStrikeFinish = t.entityprofile[iEntID].meleestrikefn;
					}
					if ( fCurrentFrame < fStrikeStart ) t.playercontrol.thirdperson.meleestruck = 0;
					if ( fCurrentFrame >= fStrikeStart && fCurrentFrame <= fStrikeFinish && t.playercontrol.thirdperson.meleestruck == 0 )
					{
						// is character facing an entity and in range
						float fCurrentX = ObjectPositionX ( iObj );
						float fCurrentY = ObjectPositionY ( iObj );
						float fCurrentZ = ObjectPositionZ ( iObj );
						for ( int iEE = 1; iEE <= g.entityelementlist; iEE++ )
						{
							if ( iEE != iE )
							{
								float fDX = t.entityelement[iEE].x - fCurrentX;
								float fDZ = t.entityelement[iEE].z - fCurrentZ;
								float fDD = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
								int entid = t.entityelement[iEE].bankindex;
								if ( fDD < fStrikeRadius && t.entityelement[iEE].staticflag == 0 && t.entityelement[iEE].health > 0 && t.entityprofile[entid].ismarker == 0 )
								{
									if ( fabs ( t.entityelement[iEE].y - fCurrentY ) < 50.0f )
									{
										float fDA = WrapValue ( Atan2 ( fDX, fDZ ) );
										float fCompareA = WrapValue(fabs(fCurrentAngle) - fabs(fDA));
										if ( fCompareA > 180 ) fCompareA = fCompareA - 360;
										if ( fabs(fCompareA) < t.entityprofile[iEntID].meleehitangle )
										{
											// apply some damage
											t.tttriggerdecalimpact = 0;
											t.tdamagesource = 1;
											int iDamageRange = t.entityprofile[iEntID].meleedamagefn - t.entityprofile[iEntID].meleedamagest;
											t.tdamage = t.entityprofile[iEntID].meleedamagest + (rand()%iDamageRange);
											entity_hitentity ( iEE, t.entityelement[iEE].obj );
											entity_triggerdecalatimpact ( t.entityelement[iEE].x, t.entityelement[iEE].y+50.0f, t.entityelement[iEE].z );
											t.playercontrol.thirdperson.meleestruck = 1;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// independent handling (no matter which guns is selected)
	gun_brass_indi ( );
}

void gun_change ( void )
{
	//  at start of level, this is set to one
	if (  t.triggerweapononeifexists>0 ) 
	{
		t.triggerweapononeifexists=0;
		t.tgunid=t.weaponslot[1].got;
		if (  t.tgunid>0 ) 
		{
			if (  Len(t.gun[t.tgunid].name_s.Get()) > 1 ) 
			{
				g.autoloadgun=t.tgunid;
			}
		}
	}
	else
	{
		if (  t.playercontrol.thirdperson.enabled == 1 ) 
		{
			// check if third person has weapon but not yet assigned
			if ( t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.hasweapon != 0 && t.gunid == 0 )
			{
				// allow g.autoloadgun through..
			}
			else
			{
				// third person character cannot equip new weapons for the moment
				return;
			}
		}
	}

	if ( g.autoloadgun != -1 && g.autoloadgun <= ArrayCount(t.gun) ) 
	{
		//  if in jet pack mode, switch it off
		if (  t.playercontrol.jetpackhidden == 0 ) 
		{
			if (  g.autoloadgun>0 ) 
			{
				if (  t.playercontrol.jetpackmode>0 ) 
				{
					t.playercontrol.jetpackmode=3;
				}
			}
		}

		//  Free the old gun
		gun_free ( );

		//  Gun selection
		t.gunid=g.autoloadgun;
		t.gun_s=t.gun[t.gunid].name_s;
		g.autoloadgun=-1;

		// reset any random fire choice (so must choose from availale ones for new gun)
		t.tfireanim = 0;

		//  If gun selection valid, load it
		if (  t.gun_s != "" ) 
		{
			g.firemode=0;
			change_brass_firemode(); //PE: check if we need to change to another brass set.
			gun_selectandorload ( );
		}

		//  cause gun lighting to reset
		//t.currentguncolr=-1;  //cyb - not used

		//  Show gun as active
		t.currentgunobj=t.gun[t.gunid].obj;
		if (  t.currentgunobj>0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,g.firemodes[t.gunid][0].action.show.s );
			ShowObject (  t.currentgunobj );
		}
		else
		{
			t.gunid=0;
		}

		//  Default gun action is to SHOW and reveal gun (then goes to gunmode=5 idle)
		t.gunmode=131 ; t.keyboardpress=0;

		//  locate slot for ammo usage
		g.weaponammoindex=0;
		if (  t.gunid>0 ) 
		{
			for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
			{
				if (  t.weaponslot[t.ws].got == t.gunid ) 
				{
					g.weaponammoindex=t.ws ; break;
				}
			}
		}

		//  show all ammo to begin with for new weapon
		if (  t.gunid>0 ) 
		{
			if (  t.gun[t.gunid].obj>0 ) 
			{
				if (  ObjectExist(t.gun[t.gunid].obj) == 1 ) 
				{
					for ( t.p = t.gun[t.gunid].settings.bulletlimbstart ; t.p<=  t.gun[t.gunid].settings.bulletlimbend; t.p++ )
					{
						if (  t.p <= ArrayCount(t.bulletlimbs) ) 
						{
							t.limbnumber=t.bulletlimbs[t.p];
							ScaleLimb (  t.gun[t.gunid].obj,t.limbnumber,100,100,100 );
						}
					}
				}
			}
		}

		gun_updatebulletvisibility();

		// if in VR mode, hide any arms (that can be detected)
		// and eventually allow arms/hands to be specified in special new VR fields in gunspec
		if (t.gunid > 0)
		{
			bool bNormalOrVRMode = false;
			if (bNormalOrVRMode == true)
			{
				sObject* pGunObj = GetObjectData(t.currentgunobj);
				for (int i = 0; i < pGunObj->iFrameCount; i++)
				{
					sFrame* pFrame = pGunObj->ppFrameList[i];
					if (pFrame->pMesh)
					{
						if (strstr(pFrame->szName, "arms") != NULL)
						{
							// found arms, hide this limb
							HideLimb(t.currentgunobj, pFrame->iID);
						}
					}
				}
			}
		}
	}
}

void gun_update_hud ( void )
{
	//  HUD marker update
	if (  ObjectExist(g.hudbankoffset+2) == 1 ) 
	{
		t.tsimwoddle_f=0;
		if (  t.currentgunobj>0 ) 
		{
			if (  ObjectExist(t.currentgunobj) == 1 ) 
			{
				if (  t.playercontrol.movement != 0 ) 
				{
					if (  GetNumberOfFrames(t.currentgunobj) == 0 ) 
					{
						t.gfakewoddle_f=WrapValue(t.gfakewoddle_f+4);
						t.tsimwoddle_f=CurveValue(Cos(t.gfakewoddle_f)*2,t.tsimwoddle_f,15);
					}
				}
				else
				{
					t.tsimwoddle_f=CurveValue(0,t.tsimwoddle_f,15);
				}
			}
		}
		if (  g.globals.riftmode == 0 ) 
		{
			bool bNormalOrVRMode = false;
			if (bNormalOrVRMode == false)
			{
				t.gunax_f = CameraAngleX(); t.gunay_f = CameraAngleY();
				RotateObject(g.hudbankoffset + 2, t.gunax_f, t.gunay_f, 0);
				PositionObject(g.hudbankoffset + 2, CameraPositionX(), CameraPositionY() + t.tsimwoddle_f, CameraPositionZ());
			}
			else
			{
			}
		}
	}

	//  and update visibility
	gun_update_hud_visibility ( );
}

void gun_update_hud_visibility ( void )
{
	if (  ObjectExist(g.hudbankoffset+2) == 1 ) 
	{
		if (  t.currentgunobj>0 ) 
		{
			if (  ObjectExist(t.currentgunobj) == 1 ) 
			{
				if (  (t.player[1].health>0 || t.playercontrol.startstrength == 0) && t.playercontrol.thirdperson.enabled == 0 ) 
				{
					ShowObject (  t.currentgunobj );
				}
				else
				{
					HideObject (  t.currentgunobj );
				}
			}
		}
	}
}

void gun_update_overlay ( void )
{
}

void gun_picksndvariant ( void )
{
	//  takes gunid,tgunsoundindex, returns sndid
	t.trr=1+Rnd(3) ; t.tttokay=0;
	if (  t.trr == 1 && t.gunsound[t.gunid][t.tgunsoundindex].soundid1>0  )  t.tttokay = 1;
	if (  t.trr == 2 && t.gunsound[t.gunid][t.tgunsoundindex].soundid2>0  )  t.tttokay = 1;
	if (  t.trr == 3 && t.gunsound[t.gunid][t.tgunsoundindex].soundid3>0  )  t.tttokay = 1;
	if (  t.trr == 4 && t.gunsound[t.gunid][t.tgunsoundindex].soundid4>0  )  t.tttokay = 1;
	if (  t.tttokay == 1 ) 
	{
		if (  t.trr == 1  )  t.sndid = t.gunsound[t.gunid][t.tgunsoundindex].soundid1;
		if (  t.trr == 2  )  t.sndid = t.gunsound[t.gunid][t.tgunsoundindex].soundid2;
		if (  t.trr == 3  )  t.sndid = t.gunsound[t.gunid][t.tgunsoundindex].soundid3;
		if (  t.trr == 4  )  t.sndid = t.gunsound[t.gunid][t.tgunsoundindex].soundid4;
	}
	else
	{
		t.sndid=t.gunsound[t.gunid][t.tgunsoundindex].soundid1;
	}
}

bool gun_getstartandfinish ( bool bIgnoreGunMode )
{
	//  get start and finish fire animation
	t.gstart = g.firemodes[t.gunid][g.firemode].action.start;
	t.gfinish = g.firemodes[t.gunid][g.firemode].action.finish;

	//  if last bullet though, use alternative if available
	if ( g.firemodes[t.gunid][g.firemode].action.laststart.s>0 ) 
	{
		if ( t.gunmode <= 104 || t.gunmode>106 ) //PE: Last can now happen in 104
		{
			// ensure start/finish not change WHILST performing last fire animation
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] <= 1 )
			{
				t.gstart = g.firemodes[t.gunid][g.firemode].action.laststart;
				t.gfinish = g.firemodes[t.gunid][g.firemode].action.lastfinish;
				return false;
			}
		}
	}

	// normal
	return true;
}

bool gun_getzoomstartandfinish ( void )
{
	t.gstart = g.firemodes[t.gunid][g.firemode].zoomaction.start;
	t.gfinish = g.firemodes[t.gunid][g.firemode].zoomaction.finish;
	if ( g.firemodes[t.gunid][g.firemode].zoomaction.laststart.s>0 ) 
	{
		// if last bullet though, use alternative if available
		if ( t.gunmode<=104 || t.gunmode>106 )
		{
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] <= 1 )
			{
				t.gstart = g.firemodes[t.gunid][g.firemode].zoomaction.laststart;
				t.gfinish = g.firemodes[t.gunid][g.firemode].zoomaction.lastfinish;
				return false;
			}
		}
	}
	return true;
}

void gun_control ( void )
{
	//  trigger gun to show and play custom anim (custstart,custend)
	if (  t.gunmode == 9998 ) 
	{
		ShowObject (  t.currentgunobj );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,g.custstart,g.custend );
		t.gunmode=9999;
	}
	if (  t.gunmode == 9999 ) 
	{
		t.currentgunanimspeed_f = g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed ( t.currentgunobj, t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= g.custend  )  t.gunmode = 5;
	}

	//  Gun Lag
	if (  t.gunzoommode != 0 ) 
	{
		t.gunlagspeed_f=g.firemodes[t.gunid][g.firemode].settings.zoomgunlagSpeed;
		t.gunlagxmax_f=g.firemodes[t.gunid][g.firemode].settings.zoomgunlagXmax;
		t.gunlagymax_f=g.firemodes[t.gunid][g.firemode].settings.zoomgunlagYmax;
	}
	else
	{
		t.gunlagspeed_f=g.firemodes[t.gunid][g.firemode].settings.gunlagSpeed;
		t.gunlagxmax_f=g.firemodes[t.gunid][g.firemode].settings.gunlagXmax;
		t.gunlagymax_f=g.firemodes[t.gunid][g.firemode].settings.gunlagYmax;
	}
	g.gunlagX_f = CurveValue(g.gunlagX_f-(t.cammovex_f)*0.01,g.gunlagX_f,t.gunlagspeed_f);
	g.gunlagY_f = CurveValue(g.gunlagY_f+(t.cammovey_f)*0.01,g.gunlagY_f,t.gunlagspeed_f);
	if (  g.gunlagX_f < - t.gunlagxmax_f  )  g.gunlagX_f  =  -t.gunlagxmax_f;
	if (  g.gunlagY_f < - t.gunlagymax_f  )  g.gunlagY_f  =  -t.gunlagymax_f;
	if (  g.gunlagX_f >  t.gunlagxmax_f  )  g.gunlagX_f  =  t.gunlagxmax_f;
	if (  g.gunlagY_f >  t.gunlagymax_f  )  g.gunlagY_f  =  t.gunlagymax_f;
	g.gunlagX_f = CurveValue(0,g.gunlagX_f,t.gunlagspeed_f*1.2);
	g.gunlagY_f = CurveValue(0,g.gunlagY_f,t.gunlagspeed_f*1.2);

	//  Gun Offset X and Y
	if (  t.gunzoommode != 0 && t.gunzoommode<11 && g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 ) 
	{
		g.gunOffsetX_f = CurveValue(g.firemodes[t.gunid][g.firemode].settings.simplezoomx_f,g.gunOffsetX_f,g.firemodes[t.gunid][g.firemode].settings.simplezoomspeed);
		g.gunOffsetY_f = CurveValue(g.firemodes[t.gunid][g.firemode].settings.simplezoomy_f,g.gunOffsetY_f,g.firemodes[t.gunid][g.firemode].settings.simplezoomspeed);
	}
	else
	{
		t.tx_f=0 ; t.ty_f=0;
		if (  (t.plrkeySHIFT) == 1 && t.playercontrol.movement != 0 ) { t.tx_f  =  g.firemodes[t.gunid][g.firemode].settings.runx_f  ; t.ty_f  =  g.firemodes[t.gunid][g.firemode].settings.runy_f; }
		g.gunOffsetX_f = CurveValue(t.tx_f,g.gunOffsetX_f,g.firemodes[t.gunid][g.firemode].settings.simplezoomspeed);
		g.gunOffsetY_f = CurveValue(t.ty_f,g.gunOffsetY_f,g.firemodes[t.gunid][g.firemode].settings.simplezoomspeed);
	}

	//  use player wobble to affect weapon bounce
	t.tadjustbasedonwobbley_f=((Cos(t.playercontrol.wobble_f)*t.playercontrol.wobbleheight_f)/10.0)-0.275;

	//  gun position offset and rotation
	if (  t.plrzoomin_f != 0.0 ) 
	{
		//  place gun when in zoom mode
		if (  g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 ) 
		{
			if (  g.firemodes[t.gunid][g.firemode].settings.simplezoommod  ==  0 ) 
			{
				t.tzplacement_f=g.firemodes[t.gunid][g.firemode].forward_f-(t.plrzoomin_f*5);
				PositionObject (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].horiz_f+g.gunlagX_f+g.gunOffsetX_f,t.tadjustbasedonwobbley_f+g.firemodes[t.gunid][g.firemode].vert_f+g.gunlagY_f+g.gunOffsetY_f,t.tzplacement_f );
			}
			else
			{
				PositionObject (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].horiz_f+g.gunlagX_f+g.gunOffsetX_f,t.tadjustbasedonwobbley_f+g.firemodes[t.gunid][g.firemode].vert_f+g.gunlagY_f+g.gunOffsetY_f,(g.firemodes[t.gunid][g.firemode].forward_f-(t.plrzoomin_f/g.firemodes[t.gunid][g.firemode].settings.simplezoommod)) );
			}
		}
		else
		{
			PositionObject (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].horiz_f,t.tadjustbasedonwobbley_f+g.firemodes[t.gunid][g.firemode].vert_f,(g.firemodes[t.gunid][g.firemode].forward_f-(t.plrzoomin_f*10.0)) );
		}
		//  when in zoom mode, sway camera based on accuracy
		t.plrzoomaccuracydest_f=g.firemodes[t.gunid][g.firemode].settings.zoomaccuracy/10000.0;
		t.plrzoomaccuracyangleh_f=WrapValue(t.plrzoomaccuracyangleh_f+(Rnd(20)/15.0));
		t.plrzoomaccuracyanglev_f=WrapValue(t.plrzoomaccuracyanglev_f+(Rnd(20)/15.0));
		if (  t.plrzoomaccuracybreath >= 0 ) 
		{
			if (  KeyState(g.keymap[g.gzoomholdbreath]) == 1 && g.firemodes[t.gunid][g.firemode].settings.zoomaccuracybreathhold == 1 ) 
			{
				if (  t.plrzoomaccuracybreath == 0 ) 
				{
					t.plrzoomaccuracybreath=Timer()+g.firemodes[t.gunid][g.firemode].settings.zoomaccuracybreath;
					if (  SoundExist(t.playercontrol.soundstartindex+31) == 1 ) 
					{
						PlaySound (  t.playercontrol.soundstartindex+31 );
					}
				}
				else
				{
					if (  Timer()<t.plrzoomaccuracybreath ) 
					{
						t.plrzoomaccuracydest_f=t.plrzoomaccuracydest_f*(g.firemodes[t.gunid][g.firemode].settings.zoomaccuracyheld/100.0);
					}
					else
					{
						t.plrzoomaccuracybreath=(Timer()+2000)*-1;
						if (  SoundExist(t.playercontrol.soundstartindex+33) == 1 ) 
						{
							PlaySound (  t.playercontrol.soundstartindex+33 );
						}
					}
				}
			}
			else
			{
				if (  t.plrzoomaccuracybreath>0 ) 
				{
					t.plrzoomaccuracybreath=(Timer()+200)*-1;
					if (  SoundExist(t.playercontrol.soundstartindex+31) == 1 ) 
					{
						if (  SoundPlaying(t.playercontrol.soundstartindex+31) == 0 ) 
						{
							if (  SoundExist(t.playercontrol.soundstartindex+32) == 1 ) 
							{
								PlaySound (  t.playercontrol.soundstartindex+32 );
							}
						}
					}
				}
			}
		}
		else
		{
			if (  Timer()<abs(t.plrzoomaccuracybreath) ) 
			{
				t.plrzoomaccuracydest_f=t.plrzoomaccuracydest_f*2.0;
			}
			else
			{
				if (  KeyState(g.keymap[g.gzoomholdbreath]) == 0 ) 
				{
					t.plrzoomaccuracybreath=0;
				}
			}
		}
		t.plrzoomaccuracy_f=CurveValue(t.plrzoomaccuracydest_f,t.plrzoomaccuracy_f,20.0);
		t.tswayx_f=Cos(t.plrzoomaccuracyangleh_f)*t.plrzoomaccuracy_f*g.timeelapsed_f*5;
		t.tswayy_f=Sin(t.plrzoomaccuracyanglev_f)*t.plrzoomaccuracy_f*g.timeelapsed_f*5;
		RotateCamera (  CameraAngleX()+t.tswayx_f,CameraAngleY()+t.tswayy_f,CameraAngleZ() );
	}
	else
	{
		//  place gun when not in zoom
		t.tforwardoffsettohideshoulder_f=-5;
		PositionObject (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].horiz_f+g.gunlagX_f+g.gunOffsetX_f,t.tadjustbasedonwobbley_f+g.firemodes[t.gunid][g.firemode].vert_f+g.gunlagY_f+g.gunOffsetY_f,g.firemodes[t.gunid][g.firemode].forward_f+t.tforwardoffsettohideshoulder_f );
		t.plrzoomaccuracybreath=0;
		t.plrzoomaccuracy_f=0;
	}

	//  Rotate gun for natural effect
	if (  t.plrkeySLOWMOTION == 1 ) 
	{
		t.wax_f=CameraAngleX();
		t.way_f=CameraAngleY();
		t.waz_f=CameraAngleZ();
	}
	else
	{
		t.wax_f=CameraAngleX()-t.lastcamax_f;
		t.way_f=CameraAngleY()-t.lastcamay_f;
		t.waz_f=CameraAngleZ()-t.lastcamaz_f;
		t.lastcamax_f=CameraAngleX();
		t.lastcamay_f=CameraAngleY();
		t.lastcamaz_f=CameraAngleZ();
		float fGunLagSpeed = 20.0f-(t.gunlagspeed_f/5.0f);
		if ( fGunLagSpeed < 1.0f ) fGunLagSpeed = 1.0f;
		if ( fGunLagSpeed > 20.0f ) fGunLagSpeed = 20.0f;
		t.wox_f=t.wox_f+(t.wax_f/fGunLagSpeed);
		t.woy_f=t.woy_f+(t.way_f/fGunLagSpeed);
		t.woz_f=t.woz_f+(t.waz_f/fGunLagSpeed);
		t.wox_f=t.wox_f*0.9;
		t.woy_f=t.woy_f*0.9;
		t.woz_f=t.woz_f*0.9;
		t.sway_f=30.0 ; t.swayn_f=t.sway_f*-1;
		if (  t.woz_f<t.swayn_f  )  t.woz_f = t.swayn_f;
		if (  t.woz_f>t.sway_f  )  t.woz_f = t.sway_f;
		//if (  t.wox_f<t.swayn_f  )  t.wox_f = t.swayn_f;
		//if (  t.wox_f>t.sway_f  )  t.wox_f = t.sway_f;
		//if (  t.woy_f<t.swayn_f  )  t.woy_f = t.swayn_f;
		//if (  t.woy_f>t.sway_f  )  t.woy_f = t.sway_f;
		float fSwayX = t.gunlagxmax_f;
		float fSwayXN = -t.gunlagxmax_f;
		float fSwayY = t.gunlagymax_f;
		float fSwayYN = -t.gunlagymax_f;
		if ( t.wox_f < fSwayXN ) t.wox_f = fSwayXN;
		if ( t.wox_f > fSwayX ) t.wox_f = fSwayX;
		if ( t.woy_f < fSwayYN ) t.woy_f = fSwayYN;
		if ( t.woy_f > fSwayY ) t.woy_f = fSwayY;
	}
	RotateObject ( t.currentgunobj, t.wox_f, 180-t.woy_f, t.woz_f );

	//  hide the object if weapon-ammo and no qty left
	//  OR a grenade with no ammo and not throwing at the time
	t.tokay=0;
	// `tpool=firemode(gunid,firemode).settings.poolindex

	// `if tpool == 0 then ammo == weaponclipammo(weaponammoindex) else ammo == ammopool(tpool).ammo

	if (  t.gun[t.gunid].settings.weaponisammo == 1 && t.weaponammo[g.weaponammoindex] == 0  )  t.tokay = 1;
	if (  t.gun[t.gunid].projectileframe != 0 && t.weaponammo[g.weaponammoindex] == 0 && t.gunmode<100  )  t.tokay = 1;
	if (  t.tokay == 1 ) 
	{
		HideObject (  t.currentgunobj );
	}
	else
	{
	//  `show object currentgunobj

		gun_update_hud_visibility ( );
	}

	//  generic speed of gun animations
	t.tidleormoving=0;
	for ( t.i = 0 ; t.i<=  1; t.i++ )
	{
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].action.idle.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].action.idle.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].action.move.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].action.move.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].action.run.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].action.run.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].zoomaction.idle.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].zoomaction.idle.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].zoomaction.move.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].zoomaction.move.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].emptyaction.idle.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].emptyaction.idle.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].emptyaction.move.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].emptyaction.move.e  )  t.tidleormoving = 1;
		if (  GetFrame(t.currentgunobj)>g.firemodes[t.gunid][t.i].emptyaction.run.s && GetFrame(t.currentgunobj)<g.firemodes[t.gunid][t.i].emptyaction.run.e  )  t.tidleormoving = 1;
	}

	// 060217 - choose the fast and slow speeds of the gun model
	float fGunAnimSpeedFast = 130.0f * t.gun[t.gunid].keyframespeed_f;
	float fGunAnimSpeedSlow = 5.0f * t.gun[t.gunid].keyframespeed_f;

	// Jump will slow down gun animation
	if (  t.playercontrol.jumpmode == 1 && t.tidleormoving == 1 ) 
	{
		//  slow right down if jumping
		if (  t.genericgunanimspeed_f == fGunAnimSpeedFast ) 
		{
			t.currentgunanimspeed_f = g.timeelapsed_f * t.genericgunanimspeed_f;
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		t.genericgunanimspeed_f = fGunAnimSpeedSlow;
	}
	else
	{
		//  regular speed for all weapon animations
		if (  t.genericgunanimspeed_f == fGunAnimSpeedSlow ) 
		{
			t.currentgunanimspeed_f = g.timeelapsed_f * t.genericgunanimspeed_f;
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		t.genericgunanimspeed_f = fGunAnimSpeedFast;
	}

	//  Zoom To/From Animations (show/hide)
	t.tzoomactionshow=g.firemodes[t.gunid][g.firemode].zoomaction.show;
	t.tzoomactionhide=g.firemodes[t.gunid][g.firemode].zoomaction.hide;
	t.tzoomactionidle=g.firemodes[t.gunid][g.firemode].zoomaction.idle;
	t.tzoomactionmove=g.firemodes[t.gunid][g.firemode].zoomaction.move;
	t.taltactionto=t.gun[t.gunid].altaction.to;
	t.taltactionfrom=t.gun[t.gunid].altaction.from;
	if (  t.weaponammo[g.weaponammoindex+g.ammooffset] == 0 ) 
	{
		//  080415 - when have no bullets, modify some anim sets
		if (  g.firemodes[t.gunid][g.firemode].emptyzoomactionshow.s>0 ) 
		{
			t.tzoomactionshow=g.firemodes[t.gunid][g.firemode].emptyzoomactionshow;
		}
		if (  g.firemodes[t.gunid][g.firemode].emptyzoomactionhide.s>0 ) 
		{
			t.tzoomactionhide=g.firemodes[t.gunid][g.firemode].emptyzoomactionhide;
		}
		if (  g.firemodes[t.gunid][g.firemode].emptyzoomactionidle.s>0 ) 
		{
			t.tzoomactionidle=g.firemodes[t.gunid][g.firemode].emptyzoomactionidle;
		}
		if (  g.firemodes[t.gunid][g.firemode].emptyzoomactionmove.s>0 ) 
		{
			t.tzoomactionmove=g.firemodes[t.gunid][g.firemode].emptyzoomactionmove;
		}
		if (  t.gun[t.gunid].emptyaltactionto.s>0 ) 
		{
			t.taltactionto=t.gun[t.gunid].emptyaltactionto;
		}
		if (  t.gun[t.gunid].emptyaltactionfrom.s>0 ) 
		{
			t.taltactionfrom=t.gun[t.gunid].emptyaltactionfrom;
		}
	}
	if (  t.gunmode  >=  2001 && t.gunmode  <=  2010 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
	}
	if (  t.gunmode  ==  2001 ) 
	{
		SetObjectInterpolation (  t.currentgunobj,100 );
		t.gunmode = 2002;
		PlayObject (  t.currentgunobj,t.tzoomactionshow.s,t.tzoomactionshow.e );
	}
	if (  t.gunmode  ==  2002 ) 
	{
		if (  GetFrame(t.currentgunobj) >= t.tzoomactionshow.e  )  t.gunmode = 5;
	}
	if (  t.gunmode  ==  2003 ) 
	{
		SetObjectInterpolation (  t.currentgunobj,100 );
		t.gunmode = 2004;
		PlayObject (  t.currentgunobj,t.tzoomactionhide.s,t.tzoomactionhide.e );
	}
	if (  t.gunmode  ==  2004 ) 
	{
		if (  GetFrame(t.currentgunobj) >= t.tzoomactionhide.e && t.gunzoommode == 0  )  t.gunmode = 5;
	}
	if (  t.gunmode  ==  2005 ) 
	{
		SetObjectInterpolation (  t.currentgunobj,100 );
		t.gunmode = 2006;
		PlayObject (  t.currentgunobj,t.tzoomactionhide.s,t.tzoomactionhide.e );
	}
	if (  t.gunmode  ==  2006 ) 
	{
		if (  GetFrame(t.currentgunobj) >= t.tzoomactionhide.e && t.gunzoommode == 0  )  t.gunmode = 121;
	}

	// handle switch weapon firemode
	if ( t.gunmode == 2007 ) 
	{
		if ( t.gunzoommode >=8 ) 
		{
			t.gunzoommode = 11;
			if ( t.tzoomactionhide.s > 0 )
			{
				t.gunmode = 2027;
				SetObjectInterpolation ( t.currentgunobj,100 );
				PlayObject ( t.currentgunobj, t.tzoomactionhide.s, t.tzoomactionhide.e );
			}
			else
				t.gunmode = 2017;
		}
		else
			t.gunmode = 2017;

		// 110718 - if about to perform firemode switch, and running
		if ( t.gunmode == 2017 && t.playercontrol.usingrun == 1 )
		{
			// intercept with anim to transition back to move first
			SetObjectInterpolation ( t.currentgunobj, 100 );
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runfrom;
			PlayObject ( t.currentgunobj, t.gruntofrom.s, t.gruntofrom.e );
			t.gunmodewaitforframe = t.gruntofrom.e;
			t.gunmode = 2037;
		}
	}
	if ( t.gunmode == 2008 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.taltactionto.e ) 
		{
			t.tfireanim = 0;
			t.tmeleeanim = 0;
			t.gun[t.gunid].settings.alternate = 1;
			g.firemode=t.gun[t.gunid].settings.alternate;
			change_brass_firemode(); //PE: check if we need to change to another brass set.
			//if (g.firemodes[t.gunid][g.firemode].settings.reloadalt > 0)
			//{
			//	//PE: Reload gun.
			//	g.ammooffset = g.firemode * 10;
			//	if (t.gun[t.gunid].settings.modessharemags == 1)  g.ammooffset = 0;
			//	gun_actualreloadcode();
			//	t.gun[t.gunid].settings.ismelee = 0;
			//}

			if ( t.playercontrol.usingrun == 1 )
			{
				t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runto;
				t.gunmode = 27;
			}
			else
				t.gunmode = 5;
		}
	}
	if ( t.gunmode == 2009 ) 
	{
		if ( t.gunzoommode >=8 ) 
		{
			t.gunzoommode = 11;
			if ( t.tzoomactionhide.s > 0 )
			{
				t.gunmode = 2029;
				SetObjectInterpolation (  t.currentgunobj,100 );
				PlayObject ( t.currentgunobj, t.tzoomactionhide.s, t.tzoomactionhide.e );
			}
			else
				t.gunmode = 2019;
		}
		else
			t.gunmode = 2019;

		// 110718 - if about to perform firemode switch, and running
		if ( t.gunmode == 2019 && t.playercontrol.usingrun == 1 )
		{
			// intercept with anim to transition back to move first
			SetObjectInterpolation ( t.currentgunobj, 100 );
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runfrom;
			PlayObject ( t.currentgunobj, t.gruntofrom.s, t.gruntofrom.e );
			t.gunmodewaitforframe = t.gruntofrom.e;
			t.gunmode = 2039;
		}
	}
	if ( t.gunmode == 2010 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.taltactionfrom.e  )  
		{
			t.tfireanim = 0;
			t.tmeleeanim = 0;
			t.gun[t.gunid].settings.alternate = 0;
			g.firemode=t.gun[t.gunid].settings.alternate;
			change_brass_firemode(); //PE: check if we need to change to another brass set.
			//if (g.firemodes[t.gunid][g.firemode].settings.reloadalt > 0)
			//{
			//	//PE: Reload gun.
			//	g.ammooffset = g.firemode * 10;
			//	if (t.gun[t.gunid].settings.modessharemags == 1)  g.ammooffset = 0;
			//	gun_actualreloadcode();
			//	t.gun[t.gunid].settings.ismelee = 0;
			//}

			if ( t.playercontrol.usingrun == 1 )
			{
				t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runto;
				t.gunmode = 27;
			}
			else
				t.gunmode = 5;
		}
	}
	if ( t.gunmode == 2017 )
	{
		t.gunmode = 2008;
		if (g.firemodes[t.gunid][g.firemode].settings.reloadalt > 0)
		{
			int oldgfiremode = g.firemode;
			int oldalternate = t.gun[t.gunid].settings.alternate;
			int oldgammooffset = g.ammooffset;
			t.gun[t.gunid].settings.alternate = 1;
			g.firemode = t.gun[t.gunid].settings.alternate;

			//PE: Reload gun.
			g.ammooffset = g.firemode * 10;
			if (t.gun[t.gunid].settings.modessharemags == 1)  g.ammooffset = 0;
			gun_actualreloadcode();
			gun_updatebulletvisibility();
			t.gun[t.gunid].settings.alternate = oldalternate;
			g.firemode = oldgfiremode;
			g.ammooffset = oldgammooffset;
		}

		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.taltactionto.s,t.taltactionto.e );
		TextureObject (  g.hudbankoffset+5,0,g.firemodes[t.gunid][1].settings.flashimg );
	}
	if ( t.gunmode == 2019 )
	{
		t.gunmode = 2010;
		if (g.firemodes[t.gunid][g.firemode].settings.reloadalt > 0)
		{
			int oldgfiremode = g.firemode;
			int oldalternate = t.gun[t.gunid].settings.alternate;
			int oldgammooffset = g.ammooffset;
			t.gun[t.gunid].settings.alternate = 0;
			g.firemode = t.gun[t.gunid].settings.alternate;

			//PE: Reload gun.
			g.ammooffset = g.firemode * 10;
			if (t.gun[t.gunid].settings.modessharemags == 1)  g.ammooffset = 0;
			gun_actualreloadcode();
			gun_updatebulletvisibility();
			t.gun[t.gunid].settings.alternate = oldalternate;
			g.firemode = oldgfiremode;
			g.ammooffset = oldgammooffset;
		}

		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.taltactionfrom.s,t.taltactionfrom.e );
		TextureObject (  g.hudbankoffset+5,0,g.firemodes[t.gunid][0].settings.flashimg );
	}
	if ( t.gunmode == 2027 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.tzoomactionhide.e ) t.gunmode = 2017;
	}
	if ( t.gunmode == 2029 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.tzoomactionhide.e )  t.gunmode = 2019;
	}
	if ( t.gunmode == 2037 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.gunmodewaitforframe ) t.gunmode = 2017;
	}
	if ( t.gunmode == 2039 ) 
	{
		if ( GetFrame(t.currentgunobj) >= t.gunmodewaitforframe )  t.gunmode = 2019;
	}
	
	// player must be at top speed before transitioning to run animation
	// when fire weapon, t.playercontrol.isrunningtime is updated with timer so does not trigger immediate run after firing
	bool bReallyRunning = false;
	bool bTrueFiring = false;
	if ( t.player[t.plrid].state.firingmode == 1 && t.weaponammo[g.weaponammoindex+g.ammooffset] > 0 ) bTrueFiring = true;
	if ( t.playercontrol.isrunning == 1 && bTrueFiring == false && t.gun[t.gunid].settings.ismelee == 0 ) 
	{
		// also ensure player is not reloading, meleeing, firing but is moving
		if ( t.playercontrol.movement != 0 && (t.gunmode < 121 || t.gunmode > 126) && (t.gunmode < 700 || t.gunmode > 707) )
		{
			if ( t.playercontrol.isrunningtime == 0 ) 
				t.playercontrol.isrunningtime = Timer();
			else
				if ( Timer() > t.playercontrol.isrunningtime + g.firemodes[t.gunid][g.firemode].settings.runanimdelay )
					bReallyRunning = true;
		}
	}
	else
		t.playercontrol.isrunningtime = 0;

	if (  g.firemodes[t.gunid][g.firemode].settings.hasempty == 1 && g.firemodes[t.gunid][g.firemode].settings.isempty == 1 ) 
	{
		if (  t.gunzoommode == 10 && g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 ) 
		{
			if (  g.firemodes[t.gunid][g.firemode].settings.simplezoomanim  !=  0 ) 
			{
				//  Zoom out once we finish off the last of our shooting gunmode
				if (  t.gunmode<101 || t.gunmode>107 ) { t.gunzoommode = 11  ; t.gunmode  =  2003; }
				gun_getzoomstartandfinish ( );
				t.gautomatic=g.firemodes[t.gunid][g.firemode].zoomaction.automatic;
			}
			else
			{
				t.gunzoommode=11;
				gun_getstartandfinish ( false );
				t.gautomatic=g.firemodes[t.gunid][g.firemode].action.automatic;
			}
		}
		else
		{
			gun_getstartandfinish ( false );
			t.gautomatic=g.firemodes[t.gunid][g.firemode].action.automatic;
		}
		t.gshow=g.firemodes[t.gunid][g.firemode].emptyaction.show;
		t.gidle=g.firemodes[t.gunid][g.firemode].emptyaction.idle;
		if ( bReallyRunning==false && t.playercontrol.usingrun != -1 ) 
		{
			t.playercontrol.usingrun=-1;
			if (  t.gunmode >= 21 && t.gunmode <= 28  )  
			{
				bool bInRunningFrames = false;
				float fThisFrame = GetFrame(t.currentgunobj);
				if ( fThisFrame >= g.firemodes[t.gunid][g.firemode].emptyaction.run.s && fThisFrame <= g.firemodes[t.gunid][g.firemode].emptyaction.run.e ) bInRunningFrames = true;
				if ( g.firemodes[t.gunid][g.firemode].emptyaction.runfrom.s > 0 )// && bInRunningFrames == true )
					t.gunmode = 27; //21; // use run to move animation
				else
					t.gunmode = 21;
			}
		}
		if ( bReallyRunning == true && t.playercontrol.usingrun != 1 ) 
		{
			t.playercontrol.usingrun=1;
			if ( t.gunmode >= 21 && t.gunmode <= 28 )  
			{
				if ( g.firemodes[t.gunid][g.firemode].emptyaction.runto.s > 0 )
					t.gunmode = 27;//21; // use move to run animation
				else
					t.gunmode = 21;
			}
		}
		if ( t.playercontrol.usingrun == -1 ) 
		{
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].emptyaction.runfrom;
			t.gmove = g.firemodes[t.gunid][g.firemode].emptyaction.move;
		}
		if ( t.playercontrol.usingrun == 1 ) 
		{
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].emptyaction.runto;
			t.gmove = g.firemodes[t.gunid][g.firemode].emptyaction.run;
		}
		t.gstartreload=g.firemodes[t.gunid][g.firemode].emptyaction.startreload;
		t.greloadloop=g.firemodes[t.gunid][g.firemode].emptyaction.reloadloop;
		t.gendreload=g.firemodes[t.gunid][g.firemode].emptyaction.endreload;
		t.gcock=g.firemodes[t.gunid][g.firemode].emptyaction.cock;
		t.ghide=g.firemodes[t.gunid][g.firemode].emptyaction.hide;
	}
	else
	{
		t.gshow=g.firemodes[t.gunid][g.firemode].action.show;
		t.gidle=g.firemodes[t.gunid][g.firemode].action.idle;
		if ( bReallyRunning == false && t.playercontrol.usingrun != -1 ) 
		{
			t.playercontrol.usingrun=-1;
			if ( t.gunmode >= 21 && t.gunmode <= 28 )  
			{
				bool bInRunningFrames = false;
				float fThisFrame = GetFrame(t.currentgunobj);
				if ( fThisFrame >= g.firemodes[t.gunid][g.firemode].action.run.s && fThisFrame <= g.firemodes[t.gunid][g.firemode].action.run.e ) bInRunningFrames = true;
				if ( g.firemodes[t.gunid][g.firemode].action.runfrom.s > 0 )//&& bInRunningFrames == true )
					t.gunmode = 27; //21; // use move to run animation
				else
					t.gunmode = 21;
			}
		}
		if ( bReallyRunning == true && t.playercontrol.usingrun != 1 ) 
		{
			t.playercontrol.usingrun=1;
			if ( t.gunmode >= 21 && t.gunmode <= 28 )  
			{
				if ( g.firemodes[t.gunid][g.firemode].action.runto.s > 0 )
					t.gunmode = 27;//21; // use move to run animation
				else
					t.gunmode = 21;
			}
		}
		if ( t.playercontrol.usingrun == -1 ) 
		{
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runfrom;
			t.gmove = g.firemodes[t.gunid][g.firemode].action.move;
		}
		if ( t.playercontrol.usingrun == 1 ) 
		{
			t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runto;
			t.gmove = g.firemodes[t.gunid][g.firemode].action.run;
		}
		t.gautomatic=g.firemodes[t.gunid][g.firemode].action.automatic;
		t.gstartreload=g.firemodes[t.gunid][g.firemode].action.startreload;
		t.greloadloop=g.firemodes[t.gunid][g.firemode].action.reloadloop;
		t.gendreload=g.firemodes[t.gunid][g.firemode].action.endreload;
		t.gcock=g.firemodes[t.gunid][g.firemode].action.cock;
		t.ghide=g.firemodes[t.gunid][g.firemode].action.hide;
		if ( t.tfireanim == 0 ) 
		{
			if ( gun_getstartandfinish ( false ) == true )
			{
				// new true random selection of fire
				t.tempani = 1 + Rnd(2);
				if ( t.tempani == 2 && g.firemodes[t.gunid][g.firemode].action.start2.s == 0 ) t.tempani = 1;
				if ( t.tempani == 3 && g.firemodes[t.gunid][g.firemode].action.start3.s == 0 ) t.tempani = 1;
				if ( t.tempani == t.templastani ) 
				{
					t.tempani = t.templastani + 1;
					if ( t.tempani > 3 ) t.tempani = 1;
				}
				if ( t.tempani == 2 && g.firemodes[t.gunid][g.firemode].action.start2.s == 0 ) t.tempani = 1;
				if ( t.tempani == 3 && g.firemodes[t.gunid][g.firemode].action.start3.s == 0 ) t.tempani = 1;
				t.templastani = t.tempani;
				if ( t.tempani == 2 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].action.start2;
					t.gfinish=g.firemodes[t.gunid][g.firemode].action.finish2;
				}
				if ( t.tempani == 3 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].action.start3;
					t.gfinish=g.firemodes[t.gunid][g.firemode].action.finish3;
				}
				t.tfireanim = t.tempani;
			}
		}
		else
		{
			//if ( gun_getstartandfinish ( false ) == true )
			//{
			if (  t.tfireanim == 1 ) 
			{
				gun_getstartandfinish ( false );
			}
			if (  t.tfireanim == 2 ) 
			{
				t.gstart=g.firemodes[t.gunid][g.firemode].action.start2;
				t.gfinish=g.firemodes[t.gunid][g.firemode].action.finish2;
			}
			if (  t.tfireanim == 3 ) 
			{
				t.gstart=g.firemodes[t.gunid][g.firemode].action.start3;
				t.gfinish=g.firemodes[t.gunid][g.firemode].action.finish3;
			}
			//}
		}
	}

	if (  t.gunzoommode != 0 && g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 && g.firemodes[t.gunid][g.firemode].settings.simplezoomanim != 0 ) 
	{
		t.gidle=t.tzoomactionidle;
		t.gmove=t.tzoomactionmove;
		//gun_getzoomstartandfinish ( );
		if ( t.playercontrol.usingrun == -1 ) t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runfrom;
		if ( t.playercontrol.usingrun == 1 ) t.gruntofrom = g.firemodes[t.gunid][g.firemode].action.runto;
		t.gautomatic=g.firemodes[t.gunid][g.firemode].zoomaction.automatic;
		t.gstartreload=g.firemodes[t.gunid][g.firemode].zoomaction.startreload;
		t.greloadloop=g.firemodes[t.gunid][g.firemode].zoomaction.reloadloop;
		t.gendreload=g.firemodes[t.gunid][g.firemode].zoomaction.endreload;
		t.gcock=g.firemodes[t.gunid][g.firemode].zoomaction.cock;
		t.gshow=t.tzoomactionshow;
		t.ghide=t.tzoomactionhide;

		//gun_getzoomstartandfinish ( );
		if ( t.tfireanim == 0 ) 
		{
			if ( gun_getzoomstartandfinish ( ) == true )
			{
				// new true random selection of fire
				t.tempani = 1 + Rnd(2);
				if ( t.tempani == 2 && g.firemodes[t.gunid][g.firemode].zoomaction.start2.s == 0 ) t.tempani = 1;
				if ( t.tempani == 3 && g.firemodes[t.gunid][g.firemode].zoomaction.start3.s == 0 ) t.tempani = 1;
				if ( t.tempani == t.templastani ) 
				{
					t.tempani = t.templastani + 1;
					if ( t.tempani > 3 ) t.tempani = 1;
				}
				if ( t.tempani == 2 && g.firemodes[t.gunid][g.firemode].zoomaction.start2.s == 0 ) t.tempani = 1;
				if ( t.tempani == 3 && g.firemodes[t.gunid][g.firemode].zoomaction.start3.s == 0 ) t.tempani = 1;
				t.templastani = t.tempani;
				if ( t.tempani == 2 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].zoomaction.start2;
					t.gfinish=g.firemodes[t.gunid][g.firemode].zoomaction.finish2;
				}
				if ( t.tempani == 3 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].zoomaction.start3;
					t.gfinish=g.firemodes[t.gunid][g.firemode].zoomaction.finish3;
				}
				t.tfireanim = t.tempani;
			}
		}
		else
		{
			if (  t.tfireanim == 1 ) 
			{
				gun_getzoomstartandfinish ( );
			}
			if (  t.tfireanim == 2 ) 
			{
				t.gstart=g.firemodes[t.gunid][g.firemode].zoomaction.start2;
				t.gfinish=g.firemodes[t.gunid][g.firemode].zoomaction.finish2;
			}
			if (  t.tfireanim == 3 ) 
			{
				t.gstart=g.firemodes[t.gunid][g.firemode].zoomaction.start3;
				t.gfinish=g.firemodes[t.gunid][g.firemode].zoomaction.finish3;
			}
		}
	}

	if (  t.gun[t.gunid].settings.ismelee == 2 ) 
	{
		if (  g.firemodes[t.gunid][g.firemode].settings.isempty == 0 || g.firemodes[t.gunid][g.firemode].emptyaction.start.s == 0 ) 
		{
			if (  t.tmeleeanim == 0 ) 
			{
				// new true random selection of melee
				t.tempmeani = 1 + Rnd(2);
				if ( t.tempmeani == 2 && g.firemodes[t.gunid][g.firemode].meleeaction.start2.s == 0 ) t.tempmeani = 1;
				if ( t.tempmeani == 3 && g.firemodes[t.gunid][g.firemode].meleeaction.start3.s == 0 ) t.tempmeani = 1;
				if ( t.tempmeani == t.tlastmeleeanim ) 
				{
					t.tempmeani = t.tlastmeleeanim + 1;
					if ( t.tempmeani > 3 ) t.tempmeani = 1;
				}
				if ( t.tempmeani == 2 && g.firemodes[t.gunid][g.firemode].meleeaction.start2.s == 0 ) t.tempmeani = 1;
				if ( t.tempmeani == 3 && g.firemodes[t.gunid][g.firemode].meleeaction.start3.s == 0 ) t.tempmeani = 1;
				t.tlastmeleeanim = t.tempmeani;
				if ( t.tempmeani == 1 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish;
				}
				if ( t.tempmeani == 2 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start2;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish2;
				}
				if ( t.tempmeani == 3 )
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start3;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish3;
				}
			}
			else
			{
				if (  t.tmeleeanim == 1 ) 
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish;
				}
				if (  t.tmeleeanim == 2 ) 
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start2;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish2;
				}
				if (  t.tmeleeanim == 3 ) 
				{
					t.gstart=g.firemodes[t.gunid][g.firemode].meleeaction.start3;
					t.gfinish=g.firemodes[t.gunid][g.firemode].meleeaction.finish3;
				}
			}
		}
		else
		{
			if (  g.firemodes[t.gunid][g.firemode].settings.isempty>0 ) 
			{
				if (  t.tmeleeanim == 0 ) 
				{
					// new true random selection of melee
					t.tempmeani = 1 + Rnd(2);
					if ( t.tempmeani == 2 && g.firemodes[t.gunid][g.firemode].emptyaction.start2.s == 0 ) t.tempmeani = 1;
					if ( t.tempmeani == 3 && g.firemodes[t.gunid][g.firemode].emptyaction.start3.s == 0 ) t.tempmeani = 1;
					if ( t.tempmeani == t.tlastmeleeanim ) 
					{
						t.tempmeani = t.tlastmeleeanim + 1;
						if ( t.tempmeani > 3 ) t.tempmeani = 1;
					}
					if ( t.tempmeani == 2 && g.firemodes[t.gunid][g.firemode].emptyaction.start2.s == 0 ) t.tempmeani = 1;
					if ( t.tempmeani == 3 && g.firemodes[t.gunid][g.firemode].emptyaction.start3.s == 0 ) t.tempmeani = 1;
					t.tlastmeleeanim = t.tempmeani;
					if ( t.tempmeani == 1 )
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish;
					}
					if ( t.tempmeani == 2 )
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start2;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish2;
					}
					if ( t.tempmeani == 3 )
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start3;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish3;
					}
				}
				else
				{
					if (  t.tmeleeanim == 1 ) 
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish;
					}
					if (  t.tmeleeanim == 2 ) 
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start2;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish2;
					}
					if (  t.tmeleeanim == 3 ) 
					{
						t.gstart=g.firemodes[t.gunid][g.firemode].emptyaction.start3;
						t.gfinish=g.firemodes[t.gunid][g.firemode].emptyaction.finish3;
					}
				}
			}
		}
		if (  t.tempmeani != 0 ) { t.tmeleeanim = t.tempmeani  ; t.tempmeani = 0; }
	}

	//  Burst and block control
	t.gblock = g.firemodes[t.gunid][g.firemode].action.block;
	if (  t.gunburst <= 0 ) 
	{
		t.gunburst=g.firemodes[t.gunid][g.firemode].settings.burst;
	}

	// 280618 - active/idle constant loopsound feature for weapons
	if ( 1 )
	{
		int iWeaponLoopSound = g.firemodes[t.gunid][g.firemode].sound.loopsound;
		if ( t.weaponammo[g.weaponammoindex+g.ammooffset] == 0 ) iWeaponLoopSound = g.firemodes[t.gunid][g.firemode].sound.emptyloopsound;
		t.sndid = 0;		
		if ( iWeaponLoopSound > 0 ) 
		{
			// only start activeidle once retrieve finished and in idle/move/run
			if ( t.gunmode >= 5 && t.gunmode < 31 )
				t.sndid = t.gunsound[t.gunid][iWeaponLoopSound].soundid1;
			else
				t.sndid = t.gunactiveidlesoundloopindex;
		}
		else
		{
			// only stop activeidle when finish switching alternate modes
			if ( t.gunmode >= 2007 && t.gunmode <= 2010 )
				t.sndid = t.gunactiveidlesoundloopindex;
			else
				t.sndid = 0;
		}
		if ( t.sndid != t.gunactiveidlesoundloopindex )
		{
			if ( t.sndid > 0 )
			{
				if ( SoundExist ( t.sndid ) ==1 )
					LoopSound ( t.sndid );
			}
			else
			{
				if ( t.gunactiveidlesoundloopindex > 0 )
					if ( SoundExist ( t.gunactiveidlesoundloopindex ) == 1 )
						StopSound ( t.gunactiveidlesoundloopindex );
			}
			t.gunactiveidlesoundloopindex = t.sndid;
		}
	}

	//  gun idle control ((4*0.75)=3.0)
	if ( t.gunmode == 5 ) 
	{
		t.gunmode=6;
		t.guninterp=4;
		StopObject (  t.currentgunobj );
		SetObjectInterpolation (  t.currentgunobj,25 );
		SetObjectFrame (  t.currentgunobj,t.gidle.s+3.0 );
	}
	if (  t.gunmode == 6 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.gidle.s+3.0 );
			t.gunmode=7;
		}
	}
	if (  t.gunmode == 7 ) 
	{
		t.gunmode=8;
		PlayObject (  t.currentgunobj,t.gidle.s+3.0,t.gidle.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
	}
	if (  t.gunmode == 8 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gidle.e  )  t.gunmode = 9;
	}
	if (  t.gunmode == 9 ) 
	{
		t.gunmode=10;
		t.guninterp=4;
		StopObject (  t.currentgunobj );
		SetObjectInterpolation (  t.currentgunobj,25 );
		SetObjectFrame (  t.currentgunobj,t.gidle.s+3.0 );
	}
	if (  t.gunmode == 10 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.gidle.s+3.0 );
			t.gunmode=7;
		}
	}

	//  gun movment control ((4*0.75)=3.0
	if (  t.gunmode == 21 ) 
	{
		t.gunmode=22;
		StopObject (  t.currentgunobj );
		SetObjectInterpolation ( t.currentgunobj, 25 );
		t.guninterp=4;
		SetObjectFrame (  t.currentgunobj,t.gmove.s+3.0 );
	}
	if (  t.gunmode == 22 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.gmove.s+3.0 );
			t.gunmode=23;
		}
	}
	if (  t.gunmode >= 23 && t.gunmode <= 26 ) 
	{
		if (  t.playercontrol.movement == 0  )  t.gunmode = 5;
	}
	if (  t.gunmode == 23 ) 
	{
		t.gunmode=24;
		PlayObject (  t.currentgunobj,t.gmove.s+3.0,t.gmove.e );
		if (  g.firemodes[t.gunid][g.firemode].settings.movespeedmod  ==  0 ) 
		{
			t.currentgunanimspeed_f=g.timeelapsed_f*(t.playercontrol.basespeed_f*t.genericgunanimspeed_f);
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		else
		{
			t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
	}
	if (  t.gunmode == 24 ) 
	{
		if (  g.firemodes[t.gunid][g.firemode].settings.movespeedmod  ==  0 ) 
		{
			t.currentgunanimspeed_f=g.timeelapsed_f*(t.playercontrol.basespeed_f*t.genericgunanimspeed_f);
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		else
		{
			t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		if (  GetFrame(t.currentgunobj) >= t.gmove.e  )  t.gunmode = 25;
	}
	if (  t.gunmode == 25 ) 
	{
		t.gunmode=26;
		t.guninterp=4;
		StopObject (  t.currentgunobj );
		SetObjectInterpolation (  t.currentgunobj,25 );
		SetObjectFrame (  t.currentgunobj,t.gmove.s+3.0 );
	}
	if (  t.gunmode == 26 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.gmove.s+3.0 );
			t.gunmode=23;
		}
	}

	// 270618 - move to run animation sequence
	if ( t.gunmode == 27 ) 
	{
		t.gunmode = 28;
		SetObjectInterpolation ( t.currentgunobj, 100 );
		SetObjectFrame ( t.currentgunobj, t.gruntofrom.s );
		PlayObject ( t.currentgunobj, t.gruntofrom.s, t.gruntofrom.e );
		t.currentgunanimspeed_f = g.timeelapsed_f*(t.playercontrol.basespeed_f*t.genericgunanimspeed_f);
		SetObjectSpeed ( t.currentgunobj, t.currentgunanimspeed_f );
		t.gunmodewaitforframe = t.gruntofrom.e;
	}
	if ( t.gunmode == 28 ) 
	{
		// monitor for when move to run transition finished
		if ( GetFrame(t.currentgunobj) >= t.gunmodewaitforframe ) t.gunmode = 21;
	}
	
	// gun put away and hide control
	if (  t.gunmode == 31 && g.noholster == 1 ) 
	{
		// Clear fired count if holstering
		t.gunmode=32;
		t.guninterp=4;
		StopObject (  t.currentgunobj );
		SetObjectInterpolation (  t.currentgunobj,100 );
		SetObjectFrame (  t.currentgunobj,t.ghide.s );
		if (  t.gun[t.gunid].settings.alternate == 0  )  t.sndid = t.gunsound[t.gunid][4].soundid1 ; else t.sndid = t.gunsound[t.gunid][4].altsoundid;
		if (  t.sndid>0 ) 
		{
			if (  SoundExist(t.sndid) == 1 ) 
			{
				if (  SoundPlaying(t.sndid) == 0 ) 
				{
					playinternalsound(t.sndid);
				}
			}
		}
	}
	if (  t.gunmode == 32 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.ghide.s );
			t.currentgunanimspeed_f=g.timeelapsed_f*(t.genericgunanimspeed_f*2.5);
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
			t.gunmode=33;
		}
	}
	if (  t.gunmode == 33 ) 
	{
		t.gunmode=34;
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.ghide.s,t.ghide.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*(t.genericgunanimspeed_f*2.5);
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
	}
	if (  t.gunmode == 34 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*(t.genericgunanimspeed_f*2.5);
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.ghide.e  )  t.gunmode = 35;
	}
	if (  t.gunmode == 35 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*(t.genericgunanimspeed_f*2.5);
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		g.autoloadgun=t.gunselectionafterhide;
		t.gunmode=5;
		if (  t.runwhenaway == 1 ) 
		{
			t.runwhenaway=2;
		}
	}

	//  gun blocking control
	if (  t.gunmode == 1001 ) 
	{
		t.gunmode=1002;
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.gblock.s,t.gblock.e );
	}

	if (  t.gunmode == 1002 ) 
	{
		if (  GetFrame(t.currentgunobj) >= t.gblock.e  )  t.gunmode = 1003;
	}
	if (  t.gunmode == 1003 ) 
	{
		t.gunmode=5;
		t.player[1].state.blockingaction=2;
	}

	//  melee gun modes
	if (  t.gunmode == 1020 ) 
	{
		t.gunmode=1021;
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.gstart.s,t.gstart.e );
	}
	if (  t.gunmode == 1021 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gstart.e  )  t.gunmode = 1022;
	}
	if (  t.gunmode == 1022 ) 
	{
		t.gunmode=1023;
		PlayObject (  t.currentgunobj,t.gfinish.s,t.gfinish.e  ); t.gunshoot=1;
	}
	if (  t.gunmode == 1023 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gfinish.e ) {  t.gun[t.gunid].settings.ismelee = 0  ; t.gunmode = 5 ; t.tmeleeanim = 0; }
	}

	//  Player presses mouse button but has no ammo (avoid model animation freezing)
	if (  t.gunshootnoammo == 1 ) 
	{
		if (  t.weaponammo[g.weaponammoindex+g.ammooffset]>0 ) 
		{
			// do nothing in this case
		}
		else
		{
			t.gunandmelee.tmouseheld=1;
			if ( t.gunclick != 1 ) 
			{
				// 270618 - ensure cannot do dry fire sound when running
				if ( t.playercontrol.usingrun != 1 && t.gunandmelee.pressedtrigger == 0 )
				{
					// dry fire
					t.gunandmelee.pressedtrigger = 1;
					if ( t.gun[t.gunid].settings.alternate == 0  )  t.sndid = t.gunsound[t.gunid][3].soundid1; else t.sndid = t.gunsound[t.gunid][3].altsoundid;
					if ( t.sndid>0 ) 
					{
						if ( SoundExist(t.sndid) == 1 ) 
						{
							if ( SoundPlaying(t.sndid) == 0 ) 
							{
								if ( g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
								{
									playinternalsound(t.sndid);
								}
							}
						}
					}
				}
			}
		}
		t.gunshootnoammo=0;
	}

	//  Player presses reload button but has no ammo (avoid model animation freezing)
	if (  t.gunreloadnoammo == 1 ) 
	{
		g.plrreloading=0;
		t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
		if (  t.tpool == 0  )  t.ammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.ammo = t.ammopool[t.tpool].ammo;
		if (  t.ammo == 0 || t.gun[t.gunid].settings.weaponisammo == 1 ) 
		{
			if (  t.gun[t.gunid].settings.weaponisammo == 0 ) 
			{
				// dry fire fake replaced with unique noammo sound
				t.sndid = t.playercontrol.soundstartindex+19;
				if ( t.sndid>0 ) 
				{
					if ( SoundExist(t.sndid) == 1 ) 
					{
						if ( SoundPlaying(t.sndid) == 0 ) 
						{
							playinternalsound(t.sndid);
						}
					}
				}
			}
		}
		t.gunreloadnoammo=0;
	}

	// gun firing control (or activate with equipment)
	t.tgunactivateequipment=0;
	if ( t.gunmode == 101 ) 
	{
		if ( g.firemodes[t.gunid][g.firemode].settings.reloadqty == 0 ) t.weaponammo[g.weaponammoindex+g.ammooffset] = 99999;
		if ( t.weaponammo[g.weaponammoindex+g.ammooffset]>0 ) 
		{
			if ( g.firemodes[t.gunid][g.firemode].settings.flaklimb != -1 ) 
			{
				// Dave guntimercount = 0 causes an additional nade to be thrown at the start, so we ensure it isnt 0 here
				g.guntimercount = 6;

				//  weapons with flak attachments fire immediately
				if ( t.gun[t.gunid].projectileframe == 0 ) 
				{
					// unless its a delayed flak like a hand grenade
					t.gunflash=1 ; t.gunshoot=1 ; g.guntimercount=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
				}
			}
			t.gunmode=102;
			SetObjectInterpolation (  t.currentgunobj,100 );

			PlayObject (  t.currentgunobj,t.gstart.s,t.gstart.e );
			t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;

			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		}
		else
		{
			if (  t.gunandmelee.pressedtrigger == 0 ) 
			{
				// ensure cannot click again until dryfire finished and mouse released
				t.gunandmelee.pressedtrigger=1;

				// dryfire animation (only if was not running)
				if ( t.gunmodelast < 21 || t.gunmodelast > 26 || (t.gunmodelast >= 21 && t.gunmodelast <= 26 && t.playercontrol.usingrun == -1) )
				{
					if (  t.gunzoommode == 0 ) 
					{
						t.gdryfire=g.firemodes[t.gunid][g.firemode].emptyaction.dryfire;
					}
					else
					{
						t.gdryfire=g.firemodes[t.gunid][g.firemode].emptyzoomactiondryfire;
					}
					if (  t.gdryfire.s>0 ) 
					{
						// play dryfire animation 
						StopObject ( t.currentgunobj );
						SetObjectInterpolation ( t.currentgunobj, 100 );
						SetObjectFrame ( t.currentgunobj, t.gdryfire.s );
						PlayObject ( t.currentgunobj, t.gdryfire.s, t.gdryfire.e );
						t.gunmode=109;

						// dryfire sound
						if ( t.gun[t.gunid].settings.alternate == 0 ) t.sndid = t.gunsound[t.gunid][3].soundid1; else t.sndid = t.gunsound[t.gunid][3].altsoundid;
						if ( t.sndid>0 ) 
						{
							if ( SoundExist(t.sndid) == 1 ) 
							{
								if ( SoundPlaying(t.sndid) == 0 ) 
								{
									if ( g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
									{
										playinternalsound(t.sndid);
									}
								}
							}
						}
					}
					else
					{
						t.gunmode=107;
					}
				}
				else
				{
					// return gunmode state right back to when before '101' fire was triggered (seamless animation with no dryfire interuption)
					t.gunmode = t.gunmodelast;
				}
			}
			else
			{
				t.gunmode=107;
			}
		}
	}
	if ( t.gunmode == 102 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		//PE: Wizard (fire) is waiting here for the animation to change ?
		//https://github.com/TheGameCreators/GameGuruRepo/issues/672
		//not really a bug as "start fire = 44,45" in \gamecore\guns\fantasy\Staff\gunspec.txt will fix it.
		
		//PE: Got stuck here no animations. GetFrame(t.currentgunobj) < t.gstart.e ?
		int iFrame = GetFrame(t.currentgunobj);
		bool bForceNextFrame = false;
		if (!GetPlaying(t.currentgunobj))
		{
			//Not playing a animation something wrong ?
			bForceNextFrame = true;
			if (iFrame < t.gstart.e)
			{
				//PE: Stuck. for debug only. , got it and it worked, resumed :)
				bForceNextFrame = true;
			}
		}

		if( bForceNextFrame || GetFrame(t.currentgunobj) >= t.gstart.e  )
		{
			// moved triggering of brass, smoke and flash to END of start of firing
			if (  t.gun[t.gunid].settings.brasslimb != -1 ) 
			{  
				// eject brass immediately
				// use regular or zoomed brass delay value (milliseconds)
				int iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.brassdelay;
				if ( g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 && t.gunzoommode != 0 ) 
					iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.zoombrassdelay;
				if ( iBrassDelay > 0 )
				{
					if ( t.gunbrasstrigger == 0 )
						t.gunbrasstrigger = timeGetTime() + iBrassDelay;
				}
				else
				{
					t.gunbrass = 1;
				}

				if (t.gun[t.gunid].settings.secondbrassactive)
				{
					int iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.secondbrassdelay;
					if (g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 && t.gunzoommode != 0)
						iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.secondzoombrassdelay;
					if (iBrassDelay > 0)
					{
						if (t.secondgunbrasstrigger == 0)
							t.secondgunbrasstrigger = timeGetTime() + iBrassDelay;
					}
					else
					{
						t.secondgunbrass = 1;
					}
				}

				g.gunbrasscount = g.firemodes[t.gunid][g.firemode].settings.firerate/2; 
			}
			t.gunmode = 103;
		}
		if (  t.gun[t.gunid].projectileframe>0 ) 
		{
			//  Dave, guntimercount = 0 causes an additional nade to be thrown at the start, so we ensure it isnt 0 here
			g.guntimercount = 6;
			//  if a delayed flak, check when frame triggers it
			if (  GetFrame(t.currentgunobj) >= t.gun[t.gunid].projectileframe ) 
			{
				t.gunflash=1 ; t.gunshoot=1 ; g.guntimercount=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
			}
		}
	}
	if (  t.gunmode == 103 ) 
	{
		t.gunmode=104;
		if (  g.firemodes[t.gunid][g.firemode].settings.equipment == 1 ) 
		{
			if (  GetNumberOfFrames(t.currentgunobj) == 0  )  t.tgunactivateequipment = 1;
		}
		else
		{
			if (  t.gun[t.gunid].projectileframe == 0 ) 
			{
				if (  g.firemodes[t.gunid][g.firemode].settings.flaklimb == -1 ) 
				{
					t.gunflash=1 ; t.gunshoot=1 ; g.guntimercount=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
				}
			}
		}
		if ( g.firemodes[t.gunid][g.firemode].settings.doesnotuseammo == 0 )
		{
			t.weaponammo[g.weaponammoindex+g.ammooffset]=t.weaponammo[g.weaponammoindex+g.ammooffset]-1; 
		}
		--t.gunburst;
		if ( t.gun[t.gunid].settings.smokelimb != -1 ) {  t.gunsmoke = 1 ; g.gunsmokecount = g.firemodes[t.gunid][g.firemode].settings.firerate/2; }
		if ( g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
		{
			// trigger sound
			if ( t.gun[t.gunid].settings.alternate == 0 ) 
			{
				t.tgunsoundindex=1  ; gun_picksndvariant ( );
			}
			else
			{
				t.sndid=t.gunsound[t.gunid][1].altsoundid;
			}
			if (  t.sndid>0 ) 
			{
				if (  SoundExist(t.sndid) == 1 ) 
				{
					if (  t.gautomatic.s>0 && t.gun[t.gunid].settings.alternate == 0 || t.gautomatic.s>0 && t.gun[t.gunid].settings.alternate == 1 && t.gun[t.gunid].settings.alternateisray == 1 ) 
					{
						LoopObject (  t.currentgunobj,t.gautomatic.s,t.gautomatic.e );
						t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
						SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
						if (  t.gunmodeloopsnd>0 ) 
						{
							if (  SoundExist(t.gunmodeloopsnd) == 1  )  StopSound (  t.gunmodeloopsnd );
						}
						t.fireloopend = g.firemodes[t.gunid][g.firemode].sound.fireloopend;
						if ( t.fireloopend >= 0 )
						{
							// fireloop for automatic weapons
							PlaySoundOffset ( t.sndid, t.fireloopend  ); 
							LoopSound ( t.sndid, 0, t.fireloopend );
							t.gunmodeloopsnd=t.sndid ; t.gunmodeloopstarted=Timer();
						}
						else
						{
							// when fireloop is negative, we use 'single instance' shots
							// and use negative value as MS time between instance plays
							PlaySound ( t.sndid );
							t.gunmodeloopsnd=0; t.gunmodeloopstarted=Timer();
						}
						t.tvolume_f = 95.0;
						t.tvolume_f = t.tvolume_f * t.audioVolume.soundFloat;
						SetSoundVolume ( t.sndid, t.tvolume_f );
					}
				}
			}
		}
	}
	if ( t.gunmode == 104 ) 
	{
		// create decal particles
		if (  g.firemodes[t.gunid][g.firemode].particle.decal_s != "" ) 
		{
			if (  g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 && t.gunzoommode  !=  0 && g.firemodes[t.gunid][g.firemode].settings.simplezoomflash  ==  1 ) 
			{
				t.x1_f=ObjectPositionX(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.zoommuzzlex_f;
				t.y1_f=ObjectPositionY(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.zoommuzzley_f;
				t.z1_f=ObjectPositionZ(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.zoommuzzlez_f;
			}
			else
			{
				t.x1_f=ObjectPositionX(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.muzzlex_f;
				t.y1_f=ObjectPositionY(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.muzzley_f;
				t.z1_f=ObjectPositionZ(t.currentgunobj)+g.firemodes[t.gunid][g.firemode].settings.muzzlez_f;
			}
			t.decalid=g.firemodes[t.gunid][g.firemode].particle.id;
			t.decalorient=2;
			g.decalx=t.x1_f;
			g.decaly=t.y1_f;
			g.decalz=t.z1_f;
			t.decalscalemodx=100 ; t.decalscalemody=t.decalscalemodx;
			t.originatore = 0; decalelement_create ( );
		}
		if ( t.weaponammo[g.weaponammoindex+g.ammooffset] > 0 )
		{
			// using old or delayed brass ejection system
			g.gunbrasscount -= g.timeelapsed_f;
			g.gunsmokecount -= g.timeelapsed_f;
			g.guntimercount -= g.timeelapsed_f;
			bool bBrassEjected = false;
			if ( g.gunbrasscount <= 0 && t.gun[t.gunid].settings.brasslimb != -1 ) 
			{ 
				// use regular or zoomed brass delay value (milliseconds)
				int iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.brassdelay;
				if ( g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 && t.gunzoommode != 0 ) 
					iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.zoombrassdelay;

				bBrassEjected = true; 
				if ( iBrassDelay > 0 )
				{
					// see below for delayed brass ejection from this trigger
					if ( t.gunbrasstrigger == 0 )
					{
						t.gunbrasstrigger = timeGetTime() + iBrassDelay;
					}
				}
				else
				{
					t.gunbrass = 1; 
				}


				if (t.gun[t.gunid].settings.secondbrassactive)
				{
					int iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.secondbrassdelay;
					if (g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 && t.gunzoommode != 0)
						iBrassDelay = g.firemodes[t.gunid][g.firemode].settings.secondzoombrassdelay;

					bBrassEjected = true;
					if (iBrassDelay > 0)
					{
						// see below for delayed brass ejection from this trigger
						if (t.secondgunbrasstrigger == 0)
						{
							t.secondgunbrasstrigger = timeGetTime() + iBrassDelay;
						}
					}
					else
					{
						t.secondgunbrass = 1;
					}
				}

				g.gunbrasscount = g.firemodes[t.gunid][g.firemode].settings.firerate/2; 
			}
			if (  g.gunsmokecount <= 0 && t.gun[t.gunid].settings.smokelimb != -1 ) { t.gunsmoke = 1  ; g.gunsmokecount = g.firemodes[t.gunid][g.firemode].settings.firerate/2; }
			if (  g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
			{
				if (  t.gunflash == 0  )  t.gunflash = 1;
				if (  g.guntimercount <= 0 ) 
				{
					t.gunshoot=1 ; g.guntimercount=g.firemodes[t.gunid][g.firemode].settings.firerate/2; 
					if ( g.firemodes[t.gunid][g.firemode].settings.doesnotuseammo == 0 )
					{
						t.weaponammo[g.weaponammoindex+g.ammooffset]=t.weaponammo[g.weaponammoindex+g.ammooffset]-1;
					}
					--t.gunburst;
				}
			}
			if (  t.gautomatic.s == 0 || t.gun[t.gunid].settings.alternate == 1 && t.gun[t.gunid].settings.alternateisflak == 1  )  t.gunmode = 105;
			if (  t.gunclick != 1 && g.firemodes[t.gunid][g.firemode].settings.burst<1  )  t.gunmode = 105;
			if (  t.gunburst<1 && g.firemodes[t.gunid][g.firemode].settings.burst>0  )  t.gunmode = 105;
			if (  g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
			{
				if (  t.gun[t.gunid].settings.alternate == 0 ) 
				{
					t.tgunsoundindex=1 ; gun_picksndvariant ( );
				}
				else
				{
					t.sndid=t.gunsound[t.gunid][1].altsoundid;
				}
				t.fireloopend = g.firemodes[t.gunid][g.firemode].sound.fireloopend;
				if ( t.fireloopend >= 0 )
				{
					// regular fireloop handles loop timing
					if (  t.gunmodeloopsnd>0  )  t.gunmodeloopstarted = Timer();
				}
				else
				{
					// negative fireloop causes single instance plays
					if ( bBrassEjected == true )
					{
						if (t.sndid > 0 && SoundExist(t.sndid) == 1)
						{
							PlaySound(t.sndid);
							t.tvolume_f = 95.0;
							t.tvolume_f = t.tvolume_f * t.audioVolume.soundFloat;
							SetSoundVolume(t.sndid, t.tvolume_f);
						}
					}
				}
				if (t.sndid > 0 && SoundExist(t.sndid) == 1)
					posinternal3dsound(t.sndid,CameraPositionX(),CameraPositionY(),CameraPositionZ());
			}

			// 270618 - intercept automatic loop shoot if 

			//PE: In autofire we should wait until ammo 0 , as we have not actually made the shot yet.
			//PE: When ammo 0 it will then trigger the "end fire" animation at the same time as the last shot.
			//if (t.weaponammo[g.weaponammoindex + g.ammooffset] == 1)
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] == 0 )
			{
				bool bUseLastStartAnim = false;
				if ( t.gunzoommode != 0 )
				{
					if ( g.firemodes[t.gunid][g.firemode].zoomaction.laststart.s > 0 )
					{
						gun_getzoomstartandfinish ( );
						bUseLastStartAnim = true;
					}
				}
				else
				{
					if ( g.firemodes[t.gunid][g.firemode].action.laststart.s > 0 )
					{
						gun_getstartandfinish ( true );
						bUseLastStartAnim = true;
					}
				}
				if ( bUseLastStartAnim == true )
				{
					t.gunmode=105;
					t.gunburst=0;
				}
			}
		}
		else
		{
			//  reset variable so burst won't attempt to finish upon reloading
			//  should also stop alt mode from retaining primary burst mode settings
			t.gunmode=105;
			t.gunburst=0;
		}
	}
	if ( t.gunbrasstrigger > 0 )
	{
		if ( timeGetTime() > t.gunbrasstrigger )
		{
			t.gunbrasstrigger = 0;
			t.gunbrass = 1; 
		}
	}
	if (t.gun[t.gunid].settings.secondbrassactive)
	{
		if (t.secondgunbrasstrigger > 0)
		{
			if (timeGetTime() > t.secondgunbrasstrigger)
			{
				t.secondgunbrasstrigger = 0;
				t.secondgunbrass = 1;
			}
		}
	}

	if (  t.gunmode == 105 ) 
	{
		if ( t.gautomatic.s>0 && g.firemodes[t.gunid][g.firemode].settings.burst<1 ) 
		{
			// automatic weapons cannot resume firing right away
		}
		else
		{
			t.gunmustreleasefirst=1;
		}
		if ( t.gfinish.s>0 ) 
		{
			t.gunmode=106;
			t.gunmodewaitforframe=t.gfinish.e;
			PlayObject (  t.currentgunobj,t.gfinish.s,t.gfinish.e );
			t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
			SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
			if ( g.firemodes[t.gunid][g.firemode].settings.equipment == 0 ) 
			{
				t.fireloopend=g.firemodes[t.gunid][g.firemode].sound.fireloopend;
				if ( t.fireloopend >= 0 )
				{
					if (  t.gun[t.gunid].settings.alternate == 0 ) 
					{
						t.tgunsoundindex=1 ; gun_picksndvariant ( );
					}
					else
					{
						t.sndid=t.gunsound[t.gunid][1].altsoundid;
					}
					if (  t.sndid>0 ) 
					{
						if (  SoundExist(t.sndid) == 1 ) 
						{
							PositionSound ( t.sndid,CameraPositionX()/10.0,CameraPositionY()/3.0,CameraPositionZ()/10.0 );
							PlaySoundOffset ( t.sndid,t.fireloopend );
						}
					}
					posinternal3dsound(t.sndid,CameraPositionX(),CameraPositionY(),CameraPositionZ());
				}
			}
		}
		else
		{
			t.gunmode=107;
		}
	}
	if (  t.gunmode == 106 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if ( GetFrame(t.currentgunobj) >= t.gunmodewaitforframe )  t.gunmode = 107;
		//  if a delayed flak, check when frame triggers it; Only one grenade, not SIX!!!
		if (  t.gun[t.gunid].projectileframe>0 ) 
		{
			if (  GetFrame(t.currentgunobj) >= t.gun[t.gunid].projectileframe ) 
			{
				t.gunflash=1 ; t.gunshoot=1 ; g.guntimercount=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
				t.gunmode=108;
			}
		}
		//  detect sound slot 1 triggers, as these are USE actions and should ACTIVATE the equipment
		if (  g.firemodes[t.gunid][g.firemode].settings.equipment == 1 && GetNumberOfFrames(t.currentgunobj)>0 ) 
		{
			if (  t.gun[t.gunid].sound.soundframes>0 ) 
			{
				for ( t.p = 0 ; t.p<=  t.gun[t.gunid].sound.soundframes; t.p++ )
				{
					if (t.p < 100) // ensure cannot access sounditem items out of bounds!
					{
						if (t.gunsounditem[t.gunid][t.p].playsound == 1)
						{
							t.sndid = t.gunsound[t.gunid][t.gunsounditem[t.gunid][t.p].playsound].soundid1;
							if (t.sndid > 0)
							{
								if (int(t.gunsounditem[t.gunid][t.p].keyframe) == int(GetFrame(t.currentgunobj)))
								{
									t.tgunactivateequipment = 1;
								}
							}
						}
					}
				}
			}
		}
	}

	//  if equipment being used and LOCKCAMERA flag set, freeze out player
	if (  g.firemodes[t.gunid][g.firemode].settings.equipment != 0 && g.firemodes[t.gunid][g.firemode].settings.lockcamera == 1 ) 
	{
		if (  t.gunmode >= 101 && t.gunmode<107 ) 
		{
			g.mefrozentype=2 ; g.mefrozen=Timer()+100;
		}
	}

	if ( t.gunmode == 107 ) 
	{
		// reset to normal
		if ( 1 ) 
		{
			// restore to idle

			//PE: After autofire we are here with one bullet left, and we must not play "alt last start fire" on next 101 run.
			//PE: Just played "alt end fire"

			t.gunmode=5;
			t.tfireanim=0;

			// ensure run anim does not kick in right away, leave for Xms until 
			t.playercontrol.isrunningtime = Timer();

			// auto-reload if no bullets
			t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
			if ( t.tpool == 0 ) t.ammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.ammo = t.ammopool[t.tpool].ammo;
			if ( t.weaponammo[g.weaponammoindex+g.ammooffset] == 0 ) 
			{
				if ( t.ammo>0 ) 
				{
					//  AirMod - No Auto Reload Feature
					if (  g.firemodes[t.gunid][g.firemode].settings.noautoreload == 0 ) 
					{
						t.gunmode=121;
					}
				}
			}

			// if equipment, reset freeze if any
			if ( g.firemodes[t.gunid][g.firemode].settings.equipment != 0 && g.firemodes[t.gunid][g.firemode].settings.lockcamera == 1 ) 
			{
				g.mefrozentype=0;
			}
		}
	}
	if (  t.gunmode == 108 ) 
	{
		//  continue running animation after a projectile delayed gunshoot
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gfinish.e  )  t.gunmode = 107;
	}
	if ( t.gunmode == 109 ) 
	{
		// dryfire animation control
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed ( t.currentgunobj,t.currentgunanimspeed_f );
		if ( GetFrame(t.currentgunobj) >= t.gdryfire.e ) 
		{
			t.gunmode=107;
		}
	}

	//  if gunloop sound continues beyond gunfire loop, end it!
	if (  t.gunmodeloopsnd>0 ) 
	{
		if (  Timer()>t.gunmodeloopstarted+300 ) 
		{
			if (  t.gunmode<101 || t.gunmode>107 ) 
			{
				if (  SoundExist(t.gunmodeloopsnd) == 1 ) 
				{
					if (  SoundPlaying(t.gunmodeloopsnd) == 1 ) 
					{
						StopSound (  t.gunmodeloopsnd );
						t.gunmodeloopsnd=0 ; t.gunmodeloopstarted=0;
					}
				}
			}
		}
	}
	//  and after 1 second, terminate LoopSound (  if not refreshed Sin ( ce (looping uzi issue) ) )
	if (  t.gunmodeloopsnd>0 ) 
	{
		if (  Timer()>t.gunmodeloopstarted+1000 ) 
		{
			if (  SoundExist(t.gunmodeloopsnd) == 1 ) 
			{
				if (  SoundPlaying(t.gunmodeloopsnd) == 1 ) 
				{
					StopSound (  t.gunmodeloopsnd );
					t.gunmodeloopsnd=0 ; t.gunmodeloopstarted=0;
				}
			}
		}
	}

	//  gun reload and cock control
	if (  t.gunmode == 121 ) 
	{
		//  240315 - if in zoom mode, get out if flagged
		if (  g.firemodes[t.gunid][g.firemode].settings.forcezoomout == 1 ) 
		{
			if (  t.gunzoommode == 9 || t.gunzoommode == 10  )  t.gunzoommode = 11;
		}
		//  Chambered round
		g.plrreloading=0;
		t.gunchamber = 0;
		if (  g.firemodes[t.gunid][g.firemode].settings.chamberedround>0  )  t.gunchamber  =  1;
		if (  t.weaponammo[g.weaponammoindex+g.ammooffset]  ==  0  )  t.gunchamber  =  0;
		//  Disable Simple Zoom on Reload
		if (  g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 && t.gunzoommode  !=  0 ) 
		{
			if (  t.gunzoommode == 10  )  t.gunzoommode = 11;
			if (  g.firemodes[t.gunid][g.firemode].settings.simplezoomanim  !=  0 ) {  t.gunmode  =  2005  ; gunmode121_cancel(); return; }
		}
		if (  g.firemodes[t.gunid][g.firemode].settings.forcezoomout == 1 && t.gunzoommode != 0 ) 
		{
			if (  t.gunzoommode == 10  )  t.gunzoommode = 11;
		}
		t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
		if (  t.tpool == 0  )  t.ammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.ammo = t.ammopool[t.tpool].ammo;
		if (  t.ammo == 0 || t.gun[t.gunid].settings.weaponisammo == 1 ) 
		{
			t.gunmode=5;
		}
		else
		{
			t.gunmode=122;
			t.guninterp=4;
			StopObject (  t.currentgunobj );
			SetObjectInterpolation (  t.currentgunobj,25 );
			SetObjectFrame (  t.currentgunobj,t.gstartreload.s );
		}
	}
	gunmode121_cancel();
}

void gunmode121_cancel ( void )
{
	if (  t.gunmode == 122 ) 
	{
		--t.guninterp;
		if (  t.guninterp <= 0 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,t.gstartreload.s );
			if (  g.firemodes[t.gunid][g.firemode].settings.shotgun == 1 && g.firemodes[t.gunid][g.firemode].settings.isempty == 0 || g.firemodes[t.gunid][g.firemode].settings.isempty == 1 && g.firemodes[t.gunid][g.firemode].settings.emptyshotgun == 1 ) 
			{
				t.gunmode=700;
			}
			else
			{
				t.gunmode=123;
			}
		}
	}
	
	//  AIRSLIDE SHOTGUN CODE BEGIN
	if (  t.gunmode == 700 ) 
	{
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.gstartreload.s,t.gstartreload.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		t.gunmode=701;
	}
	if (  t.gunmode == 701 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gstartreload.e ) 
		{
			t.gunmode=703;
		}
	}
	if (  t.gunmode == 703 ) 
	{
		PlayObject (  t.currentgunobj,t.greloadloop.s,t.greloadloop.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		t.gunmode=7031;
	}
	if (  t.gunmode == 7031 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.greloadloop.e ) 
		{
			t.gunmode=702;
		}
	}
	if (  t.gunmode == 702 ) 
	{
		//  actual reload
		g.plrreloading=0;
		t.tneedfromclip=g.firemodes[t.gunid][g.firemode].settings.reloadqty+t.gunchamber-t.weaponammo[g.weaponammoindex+g.ammooffset];
		if (  t.tneedfromclip > 1  )  t.tneedfromclip  =  1;
		t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
		if (  t.tpool == 0  )  t.ammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.ammo = t.ammopool[t.tpool].ammo;
		if (  t.tneedfromclip>t.ammo  )  t.tneedfromclip = t.ammo;
		if (  t.tneedfromclip>0 ) 
		{
			t.weaponammo[g.weaponammoindex+g.ammooffset]=t.weaponammo[g.weaponammoindex+g.ammooffset]+t.tneedfromclip;
			if (  t.tpool == 0  )  t.weaponclipammo[g.weaponammoindex+g.ammooffset] = t.weaponclipammo[g.weaponammoindex+g.ammooffset]-t.tneedfromclip; else t.ammopool[t.tpool].ammo = t.ammopool[t.tpool].ammo-t.tneedfromclip;
			t.gunmode=703;
			if (  g.firemodes[t.gunid][g.firemode].settings.reloadqty+t.gunchamber-t.weaponammo[g.weaponammoindex+g.ammooffset]<1  )  t.gunmode = 7041;
			if (  t.player[t.plrid].state.firingmode == 1  )  t.gunmode = 7041;
			//  end reload if no ammo in clip left
			if (  t.tpool == 0 ) 
			{
				if (  t.weaponclipammo[g.weaponammoindex+g.ammooffset] <= 0 ) 
				{
					t.weaponclipammo[g.weaponammoindex+g.ammooffset]=0 ; t.gunmode=5 ; g.plrreloading=0;
				}
			}
			else
			{
				if (  t.ammopool[t.tpool].ammo <= 0 ) 
				{
					t.ammopool[t.tpool].ammo=0 ; t.gunmode=5 ; g.plrreloading=0;
				}
			}
		}
		else
		{
			t.gunmode=7041;
		}
		if (  t.gunsound[t.gunid][2].soundid1>0 ) 
		{
			if (  SoundExist(t.gunsound[t.gunid][2].soundid1) == 1 ) 
			{
				PositionSound (  t.gunsound[t.gunid][2].soundid1,CameraPositionX()/10.0,CameraPositionY()/3.0,CameraPositionZ()/10.0 );
			}
		}
		gun_updatebulletvisibility ( );
	}
	if (  t.gunmode == 7041 ) 
	{
		if (  t.gunchamber > 0 ) 
		{
			t.gunmode=706;
		}
		else
		{
			t.gunmode=704;
		}
	}
	if (  t.gunmode == 704 ) 
	{
		PlayObject (  t.currentgunobj,t.gendreload.s,t.gcock.e );
		t.gunmode=705;
	}
	if (  t.gunmode == 705 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gcock.e ) { t.gunmode = 5  ; g.plrreloading = 0; }
	}
	if (  t.gunmode == 706 ) 
	{
		PlayObject (  t.currentgunobj,t.gendreload.s,t.gendreload.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		t.gunmode=707;
	}
	if (  t.gunmode == 707 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gendreload.e ) { t.gunmode = 5 ; t.gun[t.gunid].settings.ismelee = 0; }
	}
	//  AIRSLIDE SHOTGUN CODE END
	
	if (  t.gunmode == 123 ) 
	{
		t.gunmode=124;
		//  start reload animation
		SetObjectInterpolation (  t.currentgunobj,100 );
		PlayObject (  t.currentgunobj,t.gstartreload.s,t.gcock.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*g.firemodes[t.gunid][g.firemode].settings.reloadspeed*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  t.gunsound[t.gunid][2].soundid1>0 ) 
		{
			if (  SoundExist(t.gunsound[t.gunid][2].soundid1) == 1 ) 
			{
				PositionSound (  t.gunsound[t.gunid][2].soundid1,CameraPositionX()/10.0,CameraPositionY()/3.0,CameraPositionZ()/10.0 );
			}
		}
	}
	if (  t.gunmode == 124 ) 
	{
		//  reload anim with possibility of bullet reset
		t.currentgunanimspeed_f=g.timeelapsed_f*g.firemodes[t.gunid][g.firemode].settings.reloadspeed*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gun[t.gunid].settings.bulletreset ) 
		{
			//  anticipate new weapon ammo quantity to set bullet visibility
			t.tstoreoldammo=t.weaponammo[g.weaponammoindex+g.ammooffset];
			t.tstoreoldammoclip=t.weaponclipammo[g.weaponammoindex+g.ammooffset];
			t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
			if (  t.tpool>0  )  t.tstoreoldammopool = t.ammopool[t.tpool].ammo;
			gun_actualreloadcode ( );
			gun_updatebulletvisibility ( );
			t.weaponammo[g.weaponammoindex+g.ammooffset]=t.tstoreoldammo;
			t.weaponclipammo[g.weaponammoindex+g.ammooffset]=t.tstoreoldammoclip;
			if (  t.tpool>0  )  t.ammopool[t.tpool].ammo = t.tstoreoldammopool;
			t.gunmode=125;
		}
		if (  GetFrame(t.currentgunobj) >= t.gcock.e  )  t.gunmode = 126;
	}
	if (  t.gunmode == 125 ) 
	{
		//  reload anim with no possibility of bullet reset
		t.currentgunanimspeed_f=g.timeelapsed_f*g.firemodes[t.gunid][g.firemode].settings.reloadspeed*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gcock.e  )  t.gunmode = 126;
	}
	if (  t.gunmode == 126 ) 
	{
		//  actual reload
		g.plrreloading=0;
		gun_actualreloadcode ( );
		t.gun[t.gunid].settings.ismelee=0;
		t.gunmode=5;
	}

	//  gun reveal
	if (  t.gunmode == 131 ) 
	{
		SetObjectInterpolation (  t.currentgunobj,100 );
		SetObjectFrame (  t.currentgunobj,t.gshow.s );
		PlayObject (  t.currentgunobj,t.gshow.s,t.gshow.e );
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  t.gunsound[t.gunid][2].soundid1>0 ) 
		{
			if (  SoundExist(t.gunsound[t.gunid][2].soundid1) == 1 ) 
			{
				PositionSound (  t.gunsound[t.gunid][2].soundid1,CameraPositionX()/10.0,CameraPositionY()/3.0,CameraPositionZ()/10.0 );
			}
		}
		t.gunmode=132;
	}
	if (  t.gunmode == 132 ) 
	{
		t.currentgunanimspeed_f=g.timeelapsed_f*t.genericgunanimspeed_f;
		SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
		if (  GetFrame(t.currentgunobj) >= t.gshow.e  )
			t.gunmode = 5;
		if (  GetFrame(t.currentgunobj)<t.gshow.s  )
			t.gunmode = 5;
	}
}

void gun_actualreloadcode ( void )
{
	//  called from several places (above)
	t.tneedfromclip=g.firemodes[t.gunid][g.firemode].settings.reloadqty-t.weaponammo[g.weaponammoindex+g.ammooffset];
	if (  t.gunchamber  ==  1  )  t.tneedfromclip  =  t.tneedfromclip + 1;
	t.tpool=g.firemodes[t.gunid][g.firemode].settings.poolindex;
	if (  t.tpool == 0  )  t.ammo = t.weaponclipammo[g.weaponammoindex+g.ammooffset]; else t.ammo = t.ammopool[t.tpool].ammo;
	if (  t.tneedfromclip>t.ammo  )  t.tneedfromclip = t.ammo;
	if (  t.tneedfromclip>0 ) 
	{
		t.weaponammo[g.weaponammoindex+g.ammooffset]=t.weaponammo[g.weaponammoindex+g.ammooffset]+t.tneedfromclip;
		if (  t.tpool == 0  )  t.weaponclipammo[g.weaponammoindex+g.ammooffset] = t.weaponclipammo[g.weaponammoindex+g.ammooffset]-t.tneedfromclip; else t.ammopool[t.tpool].ammo = t.ammopool[t.tpool].ammo-t.tneedfromclip;
	}
}

void gun_flash ( void )
{
	//  hide muzzle if (  still visible (quick shot  )  zoom bug)
	if (  t.plrzoomin_f != 0.0 && g.firemodes[t.gunid][g.firemode].settings.simplezoom == 0 ) 
	{
		if (  GetVisible(g.hudbankoffset+5) == 1  )  HideObject (  g.hudbankoffset+5 );
		if (  GetVisible(g.hudbankoffset+32) == 1  )  HideObject (  g.hudbankoffset+32 );
		t.gunflash=0;
	}

	//  Allow MuzzleFlash to work in Simple Zoom
	GGVECTOR3 vecOffset;
	if (  g.firemodes[t.gunid][g.firemode].settings.simplezoom  !=  0 && t.gunzoommode  !=  0 && g.firemodes[t.gunid][g.firemode].settings.simplezoomflash  ==  1 ) 
	{
		vecOffset = GGVECTOR3(g.firemodes[t.gunid][g.firemode].settings.zoommuzzlex_f+g.gunlagX_f+g.gunOffsetX_f, g.firemodes[t.gunid][g.firemode].settings.zoommuzzley_f+g.gunlagY_f+g.gunOffsetY_f, g.firemodes[t.gunid][g.firemode].settings.zoommuzzlez_f);
	}
	else
	{
		vecOffset = GGVECTOR3(g.firemodes[t.gunid][g.firemode].settings.muzzlex_f + g.gunlagX_f + g.gunOffsetX_f, g.firemodes[t.gunid][g.firemode].settings.muzzley_f + g.gunlagY_f, g.firemodes[t.gunid][g.firemode].settings.muzzlez_f);
	}
	PositionObject(g.hudbankoffset + 5, vecOffset.x, vecOffset.y, vecOffset.z);
	RotateObject (  g.hudbankoffset+5,0,0,Rnd(360) );
	if ( t.gunflash == 1 ) 
	{
		//  fire flash init
		t.gunflash=2;
		g.gunflashcount=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
		ShowObject (  g.hudbankoffset+5 );
		if (  t.gun[t.gunid].settings.flashlimb2 != -1 ) 
		{
			RotateObject (  g.hudbankoffset+32,0,0,Rnd(360) );
			ShowObject (  g.hudbankoffset+32 );
		}

		//  light flash init
		if ( g.firemodes[t.gunid][g.firemode].settings.usespotlighting != 0 ) 
		{
			RotateCamera ( CameraAngleX(), CameraAngleY()-45.0f, CameraAngleZ() );
			MoveCamera ( 10.0 );
			t.tx_f = CameraPositionX();
			t.ty_f = CameraPositionY();
			t.tz_f = CameraPositionZ();
			MoveCamera ( -10.0 );
			RotateCamera ( CameraAngleX(), CameraAngleY()+45.0f, CameraAngleZ() );
			t.tcolr = g.firemodes[t.gunid][g.firemode].settings.muzzlecolorr/5;// /2; 100718 - tone it down a touch
			t.tcolg = g.firemodes[t.gunid][g.firemode].settings.muzzlecolorg/5;// /2;
			t.tcolb = g.firemodes[t.gunid][g.firemode].settings.muzzlecolorb/5;// /2;
			lighting_spotflash ( );
		}
	}

	if ( t.gunflash == 2 ) 
	{
		// Timer (  based deduction )
		t.firerate=g.firemodes[t.gunid][g.firemode].settings.firerate/2;
		if ( g.gunflashcount <= (t.firerate)-(g.timeelapsed_f*2) ) 
		{
			// hide early
			HideObject (  g.hudbankoffset+5 );
			HideObject (  g.hudbankoffset+32 );
		}
		g.gunflashcount -= g.timeelapsed_f;
		if ( g.gunflashcount <= 0 ) 
		{
			t.gunflash=3;
		}
	}

	if ( t.gunflash == 3 ) 
	{
		// final hide
		t.gunflash=0;
		g.gunflashcount=0;
		HideObject (  g.hudbankoffset+5 );
		HideObject (  g.hudbankoffset+32 );
	}
}

void gun_brass ( void )
{
	//  If gun has no brass, skip this creation moment
	if (  g.firemodes[t.gunid][g.firemode].settings.brass == 0  )  return;

	//  Twin gun second brass feature
	t.gunbrass2 = 0; if ( t.gunbrass == 1 && t.gun[t.gunid].settings.flashlimb2 != -1 ) t.gunbrass2 = 1;
	
	// count available brass, and if two are not available, free the oldest
	if ( t.gunbrass > 0 ||  t.gunbrass2 > 0 )
	{
		int iCountBrass = 0;
		int iCountBrassVisible = 0;
		for ( t.o = 6 ; t.o <= 20; t.o++ )
		{
			iCountBrass++;
			t.obj=g.hudbankoffset+t.o;
			if ( GetVisible(t.obj) == 1 ) 
				iCountBrassVisible++;
		}
		int iBrassNeeded = iCountBrassVisible - (iCountBrass-2);
		if ( iBrassNeeded > 0 )
		{
			// need to free up oldest (i.e lowest) brass
			while ( iBrassNeeded > 0 )
			{
				int iLowestObj = 0;
				float fLowestY = 999999.0f;
				for ( t.o = 6 ; t.o <= 20; t.o++ )
				{
					t.obj = g.hudbankoffset+t.o;
					float fThisY = ObjectPositionY(t.obj);
					if ( fThisY < fLowestY )
					{
						iLowestObj = t.obj;
						fLowestY = fThisY;
					}
				}
				if ( iLowestObj > 0 ) 
				{
					ODEDestroyObject ( iLowestObj );
					HideObject ( iLowestObj );
				}
				iBrassNeeded--;
			}
		}
	}

	if (t.secondgunbrass == 1)
	{
		int iCountBrass = 0;
		int iCountBrassVisible = 0;
		for (t.o = 35; t.o <= 49; t.o++)
		{
			iCountBrass++;
			t.obj = g.hudbankoffset + t.o;
			if (GetVisible(t.obj) == 1)
				iCountBrassVisible++;
		}
		int iBrassNeeded = iCountBrassVisible - (iCountBrass - 2);
		if (iBrassNeeded > 0)
		{
			// need to free up oldest (i.e lowest) brass
			while (iBrassNeeded > 0)
			{
				int iLowestObj = 0;
				float fLowestY = 999999.0f;
				for (t.o = 35; t.o <= 49; t.o++)
				{
					t.obj = g.hudbankoffset + t.o;
					float fThisY = ObjectPositionY(t.obj);
					if (fThisY < fLowestY)
					{
						iLowestObj = t.obj;
						fLowestY = fThisY;
					}
				}
				if (iLowestObj > 0)
				{
					ODEDestroyObject(iLowestObj);
					HideObject(iLowestObj);
				}
				iBrassNeeded--;
			}
		}
	}

	//  find free shell and expell
	for ( t.o = 6 ; t.o<=  20; t.o++ )
	{
		t.obj=g.hudbankoffset+t.o;
		if (  (GetVisible(t.obj) == 0 || t.o == 20) && t.gunbrass == 1 ) 
		{
			t.lx_f=LimbPositionX(t.currentgunobj,t.gun[t.gunid].settings.brasslimb)+1.0-(Rnd(20)/10.0);
			t.ly_f=LimbPositionY(t.currentgunobj,t.gun[t.gunid].settings.brasslimb);
			t.lz_f=LimbPositionZ(t.currentgunobj,t.gun[t.gunid].settings.brasslimb)+1.0-(Rnd(20)/10.0);
			PositionObject (  t.obj,t.lx_f,t.ly_f,t.lz_f );
			RotateObject (  t.obj,0,CameraAngleY(0),0 );
			//ScaleObject (  t.obj, 500, 500, 500 );
			t.brassfallcount_f[t.o]=g.firemodes[t.gunid][g.firemode].settings.brasslife;
			ShowObject (  t.obj );
			//ODECreateDynamicBox (  t.obj );
			ODECreateDynamicBox(t.obj, -1, 12); //PE: Make sure not to move player.
			t.tbrassang_f=g.firemodes[t.gunid][g.firemode].settings.brassangle+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassanglerand);
			t.tbrassspeed_f=g.firemodes[t.gunid][g.firemode].settings.brassspeed+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassspeedrand);
			t.tvelx_f=NewXValue(0,CameraAngleY(0)+t.tbrassang_f,t.tbrassspeed_f);
			t.tvelz_f=NewZValue(0,CameraAngleY(0)+t.tbrassang_f,t.tbrassspeed_f);
			t.tbrassupward_f=g.firemodes[t.gunid][g.firemode].settings.brassupward+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassupwardrand);
			// 310518 - brass is WAY too fast for physics sim, so tone down
			t.tvelx_f /= 10.0f;
			t.tvelz_f /= 10.0f;
			t.tbrassupward_f /= 10.0f;
			ODEAddBodyForce (  t.obj,t.tvelx_f,t.tbrassupward_f,t.tvelz_f,0,0,0 );
			t.tbrassrotx_f=g.firemodes[t.gunid][g.firemode].settings.brassrotx+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassrotxrand);
			t.tbrassroty_f=g.firemodes[t.gunid][g.firemode].settings.brassroty+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassrotyrand);
			t.tbrassrotz_f=g.firemodes[t.gunid][g.firemode].settings.brassrotz+Rnd(g.firemodes[t.gunid][g.firemode].settings.brassrotzrand);
			ODESetAngularVelocity (  t.obj,t.tbrassrotx_f,t.tbrassroty_f,t.tbrassrotz_f );
			t.gunbrass=0;
		}

		if ((GetVisible(t.obj) == 0 || t.o == 20) && t.gunbrass == 0 && t.gunbrass2 == 1)
		{
			t.lx_f = LimbPositionX(t.currentgunobj, t.gun[t.gunid].settings.brasslimb2) + 1.0 - (Rnd(20) / 10.0);
			t.ly_f = LimbPositionY(t.currentgunobj, t.gun[t.gunid].settings.brasslimb2);
			t.lz_f = LimbPositionZ(t.currentgunobj, t.gun[t.gunid].settings.brasslimb2) + 1.0 - (Rnd(20) / 10.0);
			PositionObject(t.obj, t.lx_f, t.ly_f, t.lz_f);
			RotateObject(t.obj, 0, CameraAngleY(), 0);
			t.brassfallcount_f[t.o] = 25.0;
			ShowObject(t.obj);
			//ODECreateDynamicBox(t.obj);
			ODECreateDynamicBox(t.obj, -1, 12); //PE: Make sure not to move player.
			//  apply forces (above) here
			t.gunbrass2 = 0;
		}

	}
	if (t.secondgunbrass == 1)
	{
		for (t.o = 35; t.o <= 49; t.o++)
		{
			t.obj = g.hudbankoffset + t.o;
			if ((GetVisible(t.obj) == 0 || t.o == 49) && t.secondgunbrass == 1)
			{
				int brasslimb = t.gun[t.gunid].settings.brasslimb;
				if (t.gun[t.gunid].settings.brasslimb2 != -1)
					brasslimb = t.gun[t.gunid].settings.brasslimb2;

				t.lx_f = LimbPositionX(t.currentgunobj, brasslimb) + 1.0 - (Rnd(20) / 10.0);
				t.ly_f = LimbPositionY(t.currentgunobj, brasslimb);
				t.lz_f = LimbPositionZ(t.currentgunobj, brasslimb) + 1.0 - (Rnd(20) / 10.0);

				PositionObject(t.obj, t.lx_f, t.ly_f, t.lz_f);
				RotateObject(t.obj, 0, CameraAngleY(0), 0);
				//ScaleObject (  t.obj, 500, 500, 500 );
				t.brassfallcount_f[t.o] = g.firemodes[t.gunid][g.firemode].settings.secondbrasslife;
				ShowObject(t.obj);
				//ODECreateDynamicBox (  t.obj );
				ODECreateDynamicBox(t.obj, -1, 12); //PE: Make sure not to move player.
				t.tbrassang_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassangle + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassanglerand);
				t.tbrassspeed_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassspeed + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassspeedrand);
				t.tvelx_f = NewXValue(0, CameraAngleY(0) + t.tbrassang_f, t.tbrassspeed_f);
				t.tvelz_f = NewZValue(0, CameraAngleY(0) + t.tbrassang_f, t.tbrassspeed_f);
				t.tbrassupward_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassupward + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassupwardrand);
				// 310518 - brass is WAY too fast for physics sim, so tone down
				t.tvelx_f /= 10.0f;
				t.tvelz_f /= 10.0f;
				t.tbrassupward_f /= 10.0f;
				ODEAddBodyForce(t.obj, t.tvelx_f, t.tbrassupward_f, t.tvelz_f, 0, 0, 0);
				t.tbrassrotx_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassrotx + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassrotxrand);
				t.tbrassroty_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassroty + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassrotyrand);
				t.tbrassrotz_f = g.firemodes[t.gunid][g.firemode].settings.secondbrassrotz + Rnd(g.firemodes[t.gunid][g.firemode].settings.secondbrassrotzrand);
				ODESetAngularVelocity(t.obj, t.tbrassrotx_f, t.tbrassroty_f, t.tbrassrotz_f);
				t.secondgunbrass = 0;
			}
		}
	}
}

void gun_brass_indi ( void )
{
	//  new system uses physics, and life fade to remove brass
	for ( t.o = 6 ; t.o<=  20; t.o++ )
	{
		t.obj=g.hudbankoffset+t.o;
		if (  GetVisible(t.obj) == 1 ) 
		{
			t.brassfallcount_f[t.o]=t.brassfallcount_f[t.o]-g.timeelapsed_f;
			if (  t.brassfallcount_f[t.o]<0 ) 
			{
				ODEDestroyObject (  t.obj );
				HideObject (  t.obj );
			}
		}
	}
	if (t.gun[t.gunid].settings.secondbrassactive)
	{
		for (t.o = 35; t.o <= 49; t.o++)
		{
			t.obj = g.hudbankoffset + t.o;
			if (GetVisible(t.obj) == 1)
			{
				t.brassfallcount_f[t.o] = t.brassfallcount_f[t.o] - g.timeelapsed_f;
				if (t.brassfallcount_f[t.o] < 0)
				{
					ODEDestroyObject(t.obj);
					HideObject(t.obj);
				}
			}
		}
	}
}

void gun_smoke ( void )
{
	//  FPSCV104RC5-twingun
	t.gunsmoke2 = 0 ; if (  t.gunsmoke == 1 && t.gun[t.gunid].settings.flashlimb2 != -1  )  t.gunsmoke2 = 1;

	//  find free smoke and puff
	for ( t.o = 21 ; t.o<=  30; t.o++ )
	{
		t.obj=g.hudbankoffset+t.o;
		if (  GetVisible(t.obj) == 0 && t.gunsmoke == 1 ) 
		{
			t.ttsmokelimb=t.gun[t.gunid].settings.smokelimb;
			if (  t.ttsmokelimb <= 0  )  t.ttsmokelimb = t.gun[t.gunid].settings.brasslimb;
			t.lx_f=LimbPositionX(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			t.ly_f=LimbPositionY(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			t.lz_f=LimbPositionZ(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			RotateObject (  t.obj,0,0,Rnd(360)  ); FixObjectPivot (  t.obj );
			PositionObject (  t.obj,t.lx_f,t.ly_f,t.lz_f );
			ShowObject (  t.obj );
			t.smokeframe_f[t.o]=0.0;
			t.gunsmoke=0 ; t.smokeframe=0;
		}
		if (  GetVisible(t.obj) == 0 && t.gunsmoke == 0 && t.gunsmoke2 == 1 ) 
		{
			t.ttsmokelimb=t.gun[t.gunid].settings.smokelimb2;
			if (  t.ttsmokelimb <= 0  )  t.ttsmokelimb = t.gun[t.gunid].settings.brasslimb;
			t.lx_f=LimbPositionX(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			t.ly_f=LimbPositionY(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			t.lz_f=LimbPositionZ(t.currentgunobj,t.ttsmokelimb)+1.0-(Rnd(20)/10.0);
			RotateObject (  t.obj,0,0,Rnd(360)  ); FixObjectPivot (  t.obj );
			PositionObject (  t.obj,t.lx_f,t.ly_f,t.lz_f );
			ShowObject (  t.obj );
			t.smokeframe_f[t.o]=0.0;
			t.gunsmoke2=0 ; t.smokeframe=0;
		}
		if (  GetVisible(t.obj) == 1 ) 
		{
			PointObject ( t.obj, CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			t.smokerisespeed_f=g.firemodes[t.gunid][g.firemode].settings.smokespeed/100.0;
			PositionObject ( t.obj, ObjectPositionX(t.obj),ObjectPositionY(t.obj)+t.smokerisespeed_f,ObjectPositionZ(t.obj) );
			float fSmokeSize = g.firemodes[t.gunid][g.firemode].settings.smokesize;
			ScaleObject ( t.obj, fSmokeSize, fSmokeSize, fSmokeSize );

			t.smokeframe_f[t.o]=t.smokeframe_f[t.o]+(2.0*g.timeelapsed_f) ; t.smokeframe=t.smokeframe_f[t.o];
			if ( GetInScreen(t.obj) == 1 && t.smokeframe <= 15 ) 
			{
				t.ty=t.smokeframe/4;
				t.tx=t.smokeframe-(t.ty*4);
				t.q_f=1.0/4.0 ; t.tx_f=t.tx*t.q_f ; t.ty_f=t.ty*t.q_f;
				ScaleObjectTexture (  t.obj,t.tx_f,t.ty_f );
			}
			else
			{
				HideObject (  t.obj );
			}
		}
	}
}

void gun_updatebulletvisibility ( void )
{
	if (  g.firemode == 0 ) 
	{
		if (  t.gun[t.gunid].settings.bulletmod == 1 ) 
		{
			if (  t.weaponammo[g.weaponammoindex]<t.gun[t.gunid].settings.bulletamount ) 
			{
				for ( t.p = t.gun[t.gunid].settings.bulletlimbstart ; t.p<=  t.gun[t.gunid].settings.bulletlimbend; t.p++ )
				{
					if (  t.p <= ArrayCount(t.bulletlimbs) ) 
					{
						t.limbnumber=t.bulletlimbs[t.p];
						if (  t.limbnumber<t.bulletlimbs[t.gun[t.gunid].settings.bulletlimbstart+t.weaponammo[g.weaponammoindex]] ) 
						{
							ScaleLimb (  t.gun[t.gunid].obj,t.limbnumber,100,100,100 );
						}
						else
						{
							ScaleLimb (  t.gun[t.gunid].obj,t.limbnumber,1,1,1 );
						}
					}
				}
			}
			else
			{
				for ( t.p = t.gun[t.gunid].settings.bulletlimbstart ; t.p<=  t.gun[t.gunid].settings.bulletlimbend; t.p++ )
				{
					if (  t.p <= ArrayCount(t.bulletlimbs) ) 
					{
						t.limbnumber=t.bulletlimbs[t.p];
						ScaleLimb (  t.gun[t.gunid].obj,t.limbnumber,100,100,100 );
					}
				}
			}
		}
	}
}

void gun_shoot ( void )
{
	//  When fire Line (  active )
	if ( t.gunshoot == 1 ) 
	{
		//  170315 - 020 - stop invincible if you shoot
		g.mp.invincibleTimer = 0;

		//  if third person, trigger shot flag
		if ( t.playercontrol.thirdperson.enabled == 1 ) 
		{
			int iWeaponIndex = t.entityelement[t.playercontrol.thirdperson.charactere].eleprof.hasweapon;
			if ( iWeaponIndex > 0 )
			{
				// this triggers the TPP to rotate to face shot direction
				t.playercontrol.thirdperson.shotfired = 1;
			}
		}

		//  Recoil normal and zoom variants
		if (  t.gunzoommode != 0 ) 
		{
			t.xprect_f=g.firemodes[t.gunid][g.firemode].settings.zoomrecoilxcorrect_f/100;
			t.yprect_f=g.firemodes[t.gunid][g.firemode].settings.zoomrecoilycorrect_f/100;
		}
		else
		{
			t.xprect_f=g.firemodes[t.gunid][g.firemode].settings.recoilxcorrect_f/100;
			t.yprect_f=g.firemodes[t.gunid][g.firemode].settings.recoilycorrect_f/100;
		}
		t.xprect_f -= 0.25f;
		t.yprect_f -= 0.25f;

		//  Zoom vs Normal recoil motion
		if ( t.gunzoommode != 0 ) 
		{
			g.gunRecoilY_f = g.gunRecoilY_f + (g.firemodes[t.gunid][g.firemode].settings.zoomrecoily_f/5);
			t.gunRecoilCorrectY_f = t.gunRecoilCorrectY_f + ((g.firemodes[t.gunid][g.firemode].settings.zoomrecoily_f/5)*(t.yprect_f));
			if (  Rnd(1)  ==  1 ) 
			{
				g.gunRecoilX_f = g.gunRecoilX_f + (g.firemodes[t.gunid][g.firemode].settings.zoomrecoilx_f/5) ; g.gunRecoilCorrectX_f = g.gunRecoilCorrectX_f + ((g.firemodes[t.gunid][g.firemode].settings.zoomrecoilx_f/5)*(t.xprect_f));
			}
			else
			{
				g.gunRecoilX_f = g.gunRecoilX_f - (g.firemodes[t.gunid][g.firemode].settings.zoomrecoilx_f/5) ; g.gunRecoilCorrectX_f = g.gunRecoilCorrectX_f - ((g.firemodes[t.gunid][g.firemode].settings.zoomrecoilx_f/5)*(t.xprect_f));
			}
		}
		else
		{
			g.gunRecoilY_f = g.gunRecoilY_f + (g.firemodes[t.gunid][g.firemode].settings.recoily_f/5);
			t.gunRecoilCorrectY_f = t.gunRecoilCorrectY_f + ((g.firemodes[t.gunid][g.firemode].settings.recoily_f/5)*(t.yprect_f));
			if (  Rnd(1)  ==  1 ) 
			{
				g.gunRecoilX_f = g.gunRecoilX_f + (g.firemodes[t.gunid][g.firemode].settings.recoilx_f/5) ; g.gunRecoilCorrectX_f = g.gunRecoilCorrectX_f + ((g.firemodes[t.gunid][g.firemode].settings.recoilx_f/5)*(t.xprect_f));
			}
			else
			{
				g.gunRecoilX_f = g.gunRecoilX_f - (g.firemodes[t.gunid][g.firemode].settings.recoilx_f/5) ; g.gunRecoilCorrectX_f = g.gunRecoilCorrectX_f - ((g.firemodes[t.gunid][g.firemode].settings.recoilx_f/5)*(t.xprect_f));
			}
		}

		//  Line (  Modified for Alternate Fire )
		t.tokay=0;
		if ( g.firemodes[t.gunid][g.firemode].settings.flakindex == 0  )  t.tokay = 1;
		if ( t.gun[t.gunid].settings.alternateisflak == 1 && t.gun[t.gunid].settings.alternate == 0  )  t.tokay = 1;
		if ( t.gun[t.gunid].settings.alternateisray == 1 && t.gun[t.gunid].settings.alternate == 1  )  t.tokay = 1;
		if ( t.gun[t.gunid].settings.ismelee == 2 )  t.tokay = 1;
		if ( t.tokay == 1 ) 
		{
			//  BULLET
			//  gun data controls iterations and accuracy
			t.trayiter=1+g.firemodes[t.gunid][g.firemode].settings.iterate;
			//  Make gun more accurate so bullets don't fly to mars - Additionally use simple zoom accuracy if selected
			if (  t.gunzoommode != 0 && g.firemodes[t.gunid][g.firemode].settings.simplezoom != 0 ) 
			{
				t.trayaccuracy_f=g.firemodes[t.gunid][g.firemode].settings.simplezoomacc;
			}
			else
			{
				//  Temporary sniper rifle accuracy until weapon system overhauled. zoomaccuracy not used and can't figure out how
				//  all the zoommode shananigans work, so unable to use simple zoom. RMB (iron sight) for now
				if (  strcmp ( Lower(t.gun[t.gunid].name_s.Get())  ,  "modern\\sniperm700" ) == 0 && (MouseClick() & 2)  ==  0 ) 
				{
					t.trayaccuracy_f=g.firemodes[t.gunid][g.firemode].settings.accuracy;
					if (  t.trayaccuracy_f>15  )  t.trayaccuracy_f = 15;
				}
				else
				{
					if (  (t.plrkeySHIFT) == 1 && t.playercontrol.movement != 0 && g.firemodes[t.gunid][g.firemode].settings.runaccuracy != -1 ) 
					{
						t.trayaccuracy_f=g.firemodes[t.gunid][g.firemode].settings.runaccuracy;
					}
					else
					{
						t.trayaccuracy_f=g.firemodes[t.gunid][g.firemode].settings.accuracy;
					}
				}
			}

			//  Bullet Limbs code
			gun_updatebulletvisibility ( );

			// set iteration for ray shots
			if ( t.gun[t.gunid].settings.ismelee == 2 )
			{
				// melee attacks only ever have one iteration
				t.gunshootspread=1;
			}
			else
			{
				if (  t.trayiter>1 ) 
				{
					t.gunshootspread=t.trayiter;
				}
				else
				{
					t.gunshootspread=1;
				}
			}

			// store camera position and angle so all rays originate from same location
			t.gunshootspreadposx = CameraPositionX(0);
			t.gunshootspreadposy = CameraPositionY(0);
			t.gunshootspreadposz = CameraPositionZ(0);
			t.gunshootspreadanglex = CameraAngleX(0);
			t.gunshootspreadangley = CameraAngleY(0);
			t.gunshootspreadanglez = CameraAngleZ(0);

			//  shot over
			t.gunshoot=0;
		}
		else
		{
			// flak is the projectile from the players weapon
			bool bNormalOrVRMode = false;
			t.flakid=g.firemodes[t.gunid][g.firemode].settings.flakindex;
			if (  t.flakid>0 ) 
			{
				//  find starting GetPoint (  for projectile )
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					//  third person flak (fireball)
					t.flakangle_f=CameraAngleY(t.terrain.gameplaycamera);
					t.flakpitch_f=0;
					t.flakx_f=ObjectPositionX(t.aisystem.objectstartindex);
					t.flaky_f=ObjectPositionY(t.aisystem.objectstartindex);
					t.flakz_f=ObjectPositionZ(t.aisystem.objectstartindex);
					t.tattobj=t.entityelement[t.playercontrol.thirdperson.charactere].attachmentobj;
					if (  t.tattobj>0 ) 
					{
						if (  ObjectExist(t.tattobj) == 1 ) 
						{
							t.tattlimb=t.entityelement[t.playercontrol.thirdperson.charactere].attachmentobjfirespotlimb;
							t.flakx_f=LimbPositionX(t.tattobj,t.tattlimb);
							t.flaky_f=LimbPositionY(t.tattobj,t.tattlimb);
							t.flakz_f=LimbPositionZ(t.tattobj,t.tattlimb);
						}
					}
				}
				else
				{
					// first person flak
					t.flakangle_f=CameraAngleY();
					t.flakpitch_f=CameraAngleX();

					// can intercept calculated ray with real ray from VR controller (if available)

					t.flakx_f=CameraPositionX()+NewXValue(0,t.flakangle_f+45,40);
					t.flaky_f=CameraPositionY();
					t.flakz_f=CameraPositionZ()+NewZValue(0,t.flakangle_f+45,40);
					if (  t.gun[t.gunid].settings.flashlimb != -1 ) 
					{
						if (  LimbExist(t.currentgunobj,t.gun[t.gunid].settings.flashlimb) == 1 ) 
						{
							t.flakx_f=LimbPositionX(t.currentgunobj,t.gun[t.gunid].settings.flashlimb);
							t.flaky_f=LimbPositionY(t.currentgunobj,t.gun[t.gunid].settings.flashlimb);
							t.flakz_f=LimbPositionZ(t.currentgunobj,t.gun[t.gunid].settings.flashlimb);
						}
					}
				}
				//  create and launch projectile
				t.tProjectileType_s=t.gun[t.gunid].projectile_s  ; weapon_getprojectileid ( );
				if (  t.tProjectileType>0 ) 
				{
					t.tSourceEntity=0 ; t.tTracerFlag=0;
					t.tStartX_f=t.flakx_f ; t.tStartY_f=t.flaky_f ; t.tStartZ_f=t.flakz_f;
					t.tAngX_f=t.flakpitch_f ; t.tAngY_f=t.flakangle_f ; t.tAngZ_f=0;
					weapon_projectile_make ( bNormalOrVRMode );
					//  hide projectile which is part of HUD
					if (  g.firemodes[t.gunid][g.firemode].settings.flaklimb != -1 ) 
					{
						HideLimb (  t.currentgunobj,g.firemodes[t.gunid][g.firemode].settings.flaklimb );
					}
				}
			}

			//  shot over
			t.gunshoot=0;
		}

		// 200918 - trigger ai sound so enemies can pick up the shot
		if ( 1 )
		{
			t.tradius_f=2000;
			AICreateSound ( CameraPositionX(), CameraPositionY(), CameraPositionZ(),t.tradius_f,t.tradius_f,-1 );
			g.aidetectnearbymode = 1;
			g.aidetectnearbycount = 60*4;
			g.aidetectnearbymodeX_f = CameraPositionX();
			g.aidetectnearbymodeZ_f = CameraPositionZ();
		}
	}

	//  And can iterate more gunshoot rays if required
	//  instead of many interations in one call, the iterations are
	//  spread to one per cycle to reduce a 'freeze' effect
	if ( t.gunshootspread>0 ) 
	{
		// trigger another ray in iteration sequence (using stored camera values at time of shot)
		gun_shoot_oneray ( );
		--t.gunshootspread;
	}
}

void gun_shoot_oneray ( void )
{
	// get weapon/melee range
	t.range_f = g.firemodes[t.gunid][g.firemode].settings.range ; if (  t.range_f == 0  )  t.range_f = 3000;

	// 011215 - use firemode zero for melee range and damage (some store weapons only specify primary!)
	if ( t.gun[t.gunid].settings.ismelee == 2 ) t.range_f = g.firemodes[t.gunid][0].settings.meleerange;

	// work out weapon inaccuracies
	t.tca_f=Rnd(360000.0)/1000.0;
	t.tcx_f=Cos(t.tca_f) ; t.tcy_f=Sin(t.tca_f);
	t.tcm_f=Rnd(t.trayaccuracy_f*1000.0)/100000.0;
	if ( t.gunshootspread>1 && t.tcm_f<2.0 ) t.tcm_f = 2.0;
	t.tcx_f=t.tcx_f*t.tcm_f ; t.tcy_f=t.tcy_f*t.tcm_f;

	// project gun-Line for shot
	if ( t.playercontrol.thirdperson.enabled == 1 )
	{
		// if camera lock, always facing forward when firing
		float fRangeOfRay = 7000.0f;
		if ( t.playercontrol.thirdperson.cameralocked == 1 )
		{
			int iCharE = t.playercontrol.thirdperson.charactere;
			float fPlrAngle = ObjectAngleY(t.entityelement[iCharE].obj);
			t.x1_f = ObjectPositionX(t.entityelement[iCharE].obj);
			t.y1_f = ObjectPositionY(t.entityelement[iCharE].obj) + 50.0f;
			t.z1_f = ObjectPositionZ(t.entityelement[iCharE].obj);
			t.x2_f = NewXValue(t.x1_f, fPlrAngle, 100.0f);
			t.y2_f = t.y1_f;
			t.z2_f = NewZValue(t.z1_f, fPlrAngle, 100.0f);

			// reduce range and scatter distortion
			fRangeOfRay = 500.0f;
			t.tcx_f *= (500.0f/7000.0f);
			t.tcy_f *= (500.0f/7000.0f);
		}
		else
		{
			// special TPP shooting (from camera for accurate impact coordinate)
			t.x1_f = t.gunshootspreadposx;
			t.y1_f = t.gunshootspreadposy;
			t.z1_f = t.gunshootspreadposz;

			// work out cross-hair position on screen to distant 3D coordinate
			int iDisplayWidth = GetDisplayWidth();
			int iDisplayHeight = GetDisplayHeight();
			PickScreen2D23D(iDisplayWidth * 0.5f, iDisplayHeight * 0.25f, 7000.0f);
			t.x2_f = t.gunshootspreadposx + GetPickVectorX();
			t.y2_f = t.gunshootspreadposy + GetPickVectorY();
			t.z2_f = t.gunshootspreadposz + GetPickVectorZ();
		}

		// adjust destination vector with bullet inaccuacies
		PositionObject ( g.hudbankoffset+3, t.x1_f, t.y1_f, t.z1_f );
		PointObject ( g.hudbankoffset+3, t.x2_f, t.y2_f, t.z2_f );
		RotateObject ( g.hudbankoffset+3, ObjectAngleX(g.hudbankoffset+3)+t.tcy_f,ObjectAngleY(g.hudbankoffset+3)+t.tcx_f,ObjectAngleZ(g.hudbankoffset+3) );
		MoveObject ( g.hudbankoffset+3, fRangeOfRay );
	}
	else
	{
		// regular FPS shooting from camera center
		t.x1_f=t.gunshootspreadposx;
		t.y1_f=t.gunshootspreadposy;
		t.z1_f=t.gunshootspreadposz;
		PositionObject (  g.hudbankoffset+3,t.x1_f,t.y1_f,t.z1_f );
		t.tca_f=Rnd(360000.0)/1000.0;
		t.tcx_f=Cos(t.tca_f) ; t.tcy_f=Sin(t.tca_f);
		t.tcm_f=Rnd(t.trayaccuracy_f*1000.0)/100000.0;
		if (  t.gunshootspread>1 && t.tcm_f<2.0  )  t.tcm_f = 2.0;
		t.tcx_f=t.tcx_f*t.tcm_f ; t.tcy_f=t.tcy_f*t.tcm_f;
		RotateObject (  g.hudbankoffset+3, t.gunshootspreadanglex+t.tcy_f,t.gunshootspreadangley+t.tcx_f,t.gunshootspreadanglez );
		MoveObject (  g.hudbankoffset+3,t.range_f );
	}

	// final destination of bullet at temp obj (hub+3)
	t.tdropoff_f=((g.firemodes[t.gunid][g.firemode].settings.dropoff+0.0)*12.0*8.0)/100.0;
	t.tdropoff_f=(((8.0/100.0)*t.range_f)/100.0)*t.tdropoff_f;
	PositionObject ( g.hudbankoffset+3,ObjectPositionX(g.hudbankoffset+3),ObjectPositionY(g.hudbankoffset+3)-t.tdropoff_f,ObjectPositionZ(g.hudbankoffset+3) );
	DisableObjectZDepth ( g.hudbankoffset+3 );
	t.x2_f=ObjectPositionX ( g.hudbankoffset+3 );
	t.y2_f=ObjectPositionY ( g.hudbankoffset+3 );
	t.z2_f=ObjectPositionZ ( g.hudbankoffset+3 );

	// can intercept calculated ray with real ray from VR controller (if available)
	bool bNormalOrVRMode = false;

	// reset bullethit vars
	t.bullethit=0 ; t.bullethitstatic=0;
	t.tbullethitmaterial=0 ; t.tbullethitflesh=0;
	t.bulletraytype=g.firemodes[t.gunid][g.firemode].settings.damagetype;
	t.gunrange_f=t.range_f;

	// raycast to entity
	entity_hasbulletrayhit ( );
}

void gun_soundcontrol ( void )
{
	// PlaySound ( frames when GetFrame ( matches ) )
	if (  t.gun[t.gunid].sound.soundframes>0 ) 
	{
		for ( t.p = 0 ; t.p <= t.gun[t.gunid].sound.soundframes; t.p++ )
		{
			if ( t.p < 100) // ensure cannot access sounditem items out of bounds!
			{
				t.sndid = t.gunsound[t.gunid][t.gunsounditem[t.gunid][t.p].playsound].soundid1;
				if ((t.gun[t.gunid].action.automatic.s > 0 && t.gun[t.gunid].settings.alternate == 0) && t.p == 0 || t.gun[t.gunid].altaction.automatic.s > 0 && t.gun[t.gunid].settings.alternate == 1 && t.gun[t.gunid].settings.alternateisray == 1 && t.p == 0)  t.sndid = 0;
				if (t.sndid > 0)
				{
					if (int(t.gunsounditem[t.gunid][t.p].keyframe) == int(GetFrame(t.currentgunobj)))
					{
						if (SoundExist(t.sndid) == 1)
						{
							if (t.gunsounditem[t.gunid][t.p].lastplay == 0)
							{
								t.gunsounditem[t.gunid][t.p].lastplay = 1;
								posinternal3dsound(t.sndid, CameraPositionX(), CameraPositionY(), CameraPositionZ());
								if (SoundExist(t.sndid) == 1)
								{
									PlaySound(t.sndid);
								}
							}
						}
					}
					else
					{
						t.gunsounditem[t.gunid][t.p].lastplay = 0;
					}
				}
			}
		}
	}
}

void gun_create_hud ( void )
{
	// Only create if not already exist
	if ( ObjectExist(g.hudbankoffset+2) == 0 ) 
	{
		// Setup HUD Center Marker
		MakeObjectBox (  g.hudbankoffset+2,30,100,30 );
		SetObjectCollisionOff (  g.hudbankoffset+2 );
		SetObjectMask(g.hudbankoffset + 2, 1);
		HideObject (  g.hudbankoffset+2 );

		// Setup HUD Gun-Line (  Marker (shows impact coord) )
		MakeObjectCube (  g.hudbankoffset+3,5 );
		SetObjectCollisionOff (  g.hudbankoffset+3 );
		SetObjectMask(g.hudbankoffset + 3, 1);
		HideObject (  g.hudbankoffset+3 );


		// Muzzle Flash(es)
		for ( t.t = 0; t.t <= 1; t.t++ )
		{
			if (  t.t == 0  )  t.tobj = g.hudbankoffset+5;
			if (  t.t == 1  )  t.tobj = g.hudbankoffset+32;
			MakeObjectPlane (  t.tobj,25,25 );
			SetObjectCollisionOff ( t.tobj );
			SetObjectTransparency ( t.tobj,1 );
			DisableObjectZDepth ( t.tobj );
			DisableObjectZRead (  t.tobj );
			SetObjectAmbient (  t.tobj,0 );
			SetObjectLight (  t.tobj,0 );
			SetObjectFOV (  t.tobj,37 );
			SetObjectMask(t.tobj, 1); //PE: Dont interfere with shadow camera.
			HideObject (  t.tobj );
			SetObjectEffect ( t.tobj, g.decaleffectoffset );

		}

		// Brass
		for ( t.o = 6 ; t.o <= 20; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			MakeObjectCube ( t.obj,10 );
			SetObjectCollisionOff ( t.obj );
			DisableObjectZDepthEx ( t.obj, 1 );
			SetObjectMask(t.tobj, 1); //PE: Dont interfere with shadow camera.
			HideObject ( t.obj );


		}

		// Second Brass
		for (t.o = 35; t.o <= 49; t.o++)
		{
			t.obj = g.hudbankoffset + t.o;
			MakeObjectCube(t.obj, 10);
			SetObjectCollisionOff(t.obj);
			DisableObjectZDepthEx(t.obj, 1);
			SetObjectMask(t.tobj, 1); //PE: Dont interfere with shadow camera.
			HideObject(t.obj);
		}

		// Smoke
		for ( t.o = 21 ; t.o<=  30; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			MakeObjectPlane (  t.obj,50,50 );
			SetObjectCollisionOff (  t.obj );
			SetObjectTransparency ( t.obj, 6 );
			DisableObjectZDepth ( t.obj );
			DisableObjectZWrite (  t.obj );
			DisableObjectZRead (  t.obj );
			SetObjectAmbient (  t.obj,0 );
			SetObjectLight (  t.obj,0 );
			SetObjectFOV (  t.obj,37 );
			SetObjectMask(t.obj, 1); //PE: Dont interfere with shadow camera.
			HideObject (  t.obj );
			//SetObjectEffect ( t.tobj, g.decaleffectoffset );
			//PE: another one t.tobj , change effect on wrong model.
			SetObjectEffect(t.obj, g.decaleffectoffset);


		}


	}
}

void gun_setup ( void )
{
	//  Create common resources for gun
	gun_create_hud ( );
}

void gun_gatherslotorder ( void )
{

t.tslotmax=0;
g.gunslotmax=0;
Dim (  t.data_s,100  );
t.filename_s="..\\";
t.filename_s += g.setupfilename_s;
LoadArray (  t.filename_s.Get(),t.data_s );
for ( t.l = 0 ; t.l<=  99; t.l++ )
{
	t.line_s=t.data_s[t.l];
	if (  Len(t.line_s.Get())>0 ) 
	{
		if (  strcmp ( Left(t.line_s.Get(),1) , ";" ) != 0 ) 
		{

			//  take fieldname and value
			for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
			{
				if ( t.line_s.Get()[t.c] == '=' )  { t.mid = t.c+1  ; break; }
			}
			t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
			t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));

			//  gather gun type from slot
			for ( t.tww = 1 ; t.tww<=  9; t.tww++ )
			{
				t.tryfield_s="slot";
				t.tryfield_s += Str(t.tww);
				if (  t.field_s == t.tryfield_s ) 
				{
					//  find gun id from name
					t.findgun_s=t.value_s;
					gun_findweaponindexbyname ( );
					t.weaponslot[t.tww].pref=t.foundgunid;
					if (  t.foundgunid>0  )  g.gunslotmax = t.tww;
				}
			}

		}
	}
}
UnDim (  t.data_s );

return;

}

void gun_selectandorload ( void )
{

	//  Load gun if not selected
	if (  t.gun[t.gunid].obj == 0 ) 
	{
		gun_load ( );
	}

	//  Associate gun with player
	t.currentgunobj=t.gun[t.gunid].obj;


	//  Setup gun with muzzle flash image
	if (  t.gun[t.gunid].settings.flashlimb != -1 ) 
	{
		TextureObject ( g.hudbankoffset+5, 0, g.firemodes[t.gunid][0].settings.flashimg );
		GlueObjectToLimb ( g.hudbankoffset+5, g.hudbankoffset+2, 0 );
		PositionObject (  g.hudbankoffset+5,g.firemodes[t.gunid][0].settings.muzzlex_f,g.firemodes[t.gunid][0].settings.muzzley_f,g.firemodes[t.gunid][0].settings.muzzlez_f );
		t.size_f = g.firemodes[t.gunid][0].settings.muzzlesize_f ; if (  t.size_f == 0.0  )  t.size_f = 100.0;
		ScaleObject (  g.hudbankoffset+5,t.size_f,t.size_f,t.size_f );
	}
	if (  t.gun[t.gunid].settings.flashlimb2 != -1 ) 
	{
		TextureObject (  g.hudbankoffset+32,0,g.firemodes[t.gunid][0].settings.flashimg );
		GlueObjectToLimb (  g.hudbankoffset+32,g.hudbankoffset+2,0 );
		PositionObject (  g.hudbankoffset+32,g.firemodes[t.gunid][0].settings.muzzlex_f*-1,g.firemodes[t.gunid][0].settings.muzzley_f,g.firemodes[t.gunid][0].settings.muzzlez_f );
		t.size_f = g.firemodes[t.gunid][0].settings.muzzlesize_f ; if (  t.size_f == 0.0  )  t.size_f = 100.0;
		ScaleObject (  g.hudbankoffset+32,t.size_f,t.size_f,t.size_f );
	}
	else
	{
		ScaleObject (  g.hudbankoffset+32,0,0,0 );
	}

	//  Setup gun with smoke images
	if (  t.gun[t.gunid].settings.smokelimb != -1 ) 
	{
		for ( t.o = 21 ; t.o<=  30; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			TextureObject (  t.obj,g.firemodes[t.gunid][g.firemode].settings.smokeimg );
			HideObject (  t.obj );
			//  apply decal shader
			SetObjectEffect ( t.obj, g.decaleffectoffset );
			SetObjectCull ( t.obj, 0 );
			SetObjectTransparency ( t.obj, 6 );
			DisableObjectZWrite ( t.obj );
			SetObjectMask ( t.obj, 1 );
			//  prep UV for shader anim
			t.q_f=1.0/4.0;
			LockVertexDataForLimb (  t.obj,0 );
			SetVertexDataUV (  0,t.q_f,0 );
			SetVertexDataUV (  1,0,0 );
			SetVertexDataUV (  2,t.q_f,t.q_f );
			SetVertexDataUV (  3,0,0 );
			SetVertexDataUV (  4,0,t.q_f );
			SetVertexDataUV (  5,t.q_f,t.q_f );
			UnlockVertexData (  );
		}
	}


	//  Setup gun with brass models
	if ( t.gun[t.gunid].settings.brasslimb != -1 ) 
	{
		for ( t.o = 6 ; t.o<=  20; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			if ( ObjectExist(t.obj) == 1 )  
			{
				ODEDestroyObject ( t.obj );
				DeleteObject ( t.obj );
			}
			if (  g.firemodes[t.gunid][g.firemode].settings.brassobjmaster == 0 ) 
			{
				MakeObjectCube (  t.obj,0 );
			}
			else
			{
				InstanceObject (  t.obj,g.firemodes[t.gunid][g.firemode].settings.brassobjmaster );
			}
			SetObjectCollisionOff (  t.obj );
			SetObjectFOV (  t.obj,37 );
			HideObject (  t.obj );
		}
		for (t.o = 35; t.o <= 49; t.o++)
		{
			t.obj = g.hudbankoffset + t.o;
			if (ObjectExist(t.obj) == 1)
			{
				ODEDestroyObject(t.obj);
				DeleteObject(t.obj);
			}
			if (g.firemodes[t.gunid][g.firemode].settings.secondbrassobjmaster == 0)
			{
				MakeObjectCube(t.obj, 0);
			}
			else
			{
				InstanceObject(t.obj, g.firemodes[t.gunid][g.firemode].settings.secondbrassobjmaster);
			}
			SetObjectCollisionOff(t.obj);
			SetObjectFOV(t.obj, 37);
			HideObject(t.obj);
		}
	}
	change_brass_firemode(); //PE: Make sure we use the same firemode settings. so can change later when needed.
}

void change_brass_firemode(void)
{
	static int current_firemode = -1;
	if (g.firemode != current_firemode)
	{
		current_firemode = g.firemode;
		if (t.gun[t.gunid].settings.brasslimb != -1)
		{
			for (t.o = 6; t.o <= 20; t.o++)
			{
				t.obj = g.hudbankoffset + t.o;
				if (ObjectExist(t.obj) == 1)
				{
					ODEDestroyObject(t.obj);
					DeleteObject(t.obj);
				}
				int iUseFireMode = g.firemode;
				//PE: Default to mode 0 , if "alt brass" is not setup.
				if (g.firemode == 1 && g.firemodes[t.gunid][g.firemode].settings.brassobjmaster == 0)
					iUseFireMode = 0;

				if (g.firemodes[t.gunid][iUseFireMode].settings.brassobjmaster == 0)
				{
					MakeObjectCube(t.obj, 0);
				}
				else
				{
					InstanceObject(t.obj, g.firemodes[t.gunid][iUseFireMode].settings.brassobjmaster);
				}
				SetObjectCollisionOff(t.obj);
				SetObjectFOV(t.obj, 37);
				HideObject(t.obj);
			}

			for (t.o = 35; t.o <= 49; t.o++)
			{
				t.obj = g.hudbankoffset + t.o;
				if (ObjectExist(t.obj) == 1)
				{
					ODEDestroyObject(t.obj);
					DeleteObject(t.obj);
				}
				int iUseFireMode = g.firemode;
				//PE: Default to mode 0 , if "alt brass" is not setup.
				if (g.firemode == 1 && g.firemodes[t.gunid][g.firemode].settings.secondbrassobjmaster == 0)
					iUseFireMode = 0;

				if (g.firemodes[t.gunid][iUseFireMode].settings.secondbrassobjmaster == 0)
				{
					MakeObjectCube(t.obj, 0);
				}
				else
				{
					InstanceObject(t.obj, g.firemodes[t.gunid][iUseFireMode].settings.secondbrassobjmaster);
				}
				SetObjectCollisionOff(t.obj);
				SetObjectFOV(t.obj, 37);
				HideObject(t.obj);
			}

		}
	}
}


void gun_load ( void )
{
	//  do not load gun if already present
	if (  t.gun[t.gunid].obj>0 ) 
	{
		if (  ObjectExist(t.gun[t.gunid].obj) == 1 ) 
		{
			//  no need to reload
			return;
		}
		else
		{
			//  gun data present, object missing
			t.gun[t.gunid].obj=0;
		}
	}

	//  Load gun data
	gun_loaddata ( );

	//  Load gun models
	t.currentgunfile_s="gamecore\\";
	t.currentgunfile_s += g.fpgchuds_s+"\\"+t.gun_s+"\\HUD.dbo";
	t.currentgunobj = loadgun(t.gunid,t.currentgunfile_s.Get());
	if ( t.currentgunobj == 0 )
	{
		// could not find DBO, so try X (as may be in editor/test game)
		t.currentgunfile_s="gamecore\\";
		t.currentgunfile_s += g.fpgchuds_s+"\\"+t.gun_s+"\\HUD.x";
		t.currentgunobj=loadgun(t.gunid,t.currentgunfile_s.Get());
	}
	if (  t.currentgunobj  ==  0  )  return;
	if (  ObjectExist(t.currentgunobj)  ==  0  )  return;
	t.gun[t.gunid].obj=t.currentgunobj;
	sprintf ( t.szwork , "Load Gun:%s Obj:%i" , t.currentgunfile_s.Get() , t.currentgunobj );
	timestampactivity(0, t.szwork );

	//  Bullet feed can be made invisible as bullets used up
	if (  t.gun[t.gunid].settings.bulletmod == 1 ) 
	{
		t.gun[t.gunid].settings.bulletlimbstart=g.bulletlimbsmax+1;
		g.bulletlimbsmax += t.gun[t.gunid].settings.bulletlimbsmax;
		t.gun[t.gunid].settings.bulletlimbend=g.bulletlimbsmax;
		Dim (  t.bulletlimbs,g.bulletlimbsmax  );
		for ( t.p = 0 ; t.p <= t.gun[t.gunid].settings.bulletlimbsmax; t.p++ )
		{
			if (t.gun[t.gunid].settings.bulletlimbstart + t.p <= g.bulletlimbsmax)  t.bulletlimbs[t.gun[t.gunid].settings.bulletlimbstart + t.p] = -1;
		}
	}

	//  Perform scan to determine hotspot markers
	t.flashlimb=-1 ; t.brasslimb=-1 ; t.smokelimb=-1 ; t.handlimb=-1;
	t.flashlimb2=-1 ; t.brasslimb2=-1 ; t.smokelimb2=-1;
	t.flaklimb1=-1;
	PerformCheckListForLimbs (  t.currentgunobj );
	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		t.name_s=Upper(ChecklistString(t.c));
		if (  t.name_s == "FIRESPOT"  )  t.flashlimb = t.c-1;
		if (  t.name_s == "X3DS_FIRESPOT"  )  t.flashlimb = t.c-1;
		if (  t.name_s == "FIRESPOT02"  )  t.flashlimb2 = t.c-1;
		if (  t.name_s == "BRASS"  )  t.brasslimb = t.c-1;
		if (  t.name_s == "X3DS_BRASS"  )  t.brasslimb = t.c-1;
		if (  t.name_s == "BRASS02"  )  t.brasslimb2 = t.c-1;
		if (  t.name_s == "_SMOKE"  )  t.smokelimb = t.c-1;
		if (  t.name_s == "SMOKE"  )  t.smokelimb = t.c-1;
		if (  t.name_s == "X3DS_SMOKE"  )  t.smokelimb = t.c-1;
		if (  t.name_s == "SMOKE02"  )  t.smokelimb2 = t.c-1;
		if (  t.name_s == "HAND"  )  t.handlimb = t.c-1;
		if (  t.name_s == "X3DS_HAND"  )  t.handlimb = t.c-1;
		if (  t.name_s == "ROCKET"  )  t.flaklimb1 = t.c-1;
		if (  t.name_s == "GUN_ROOT_BONE"  )  t.handlimb = t.c-1;
		if (  t.name_s == "GRENADE_COMBINED"  )  t.flaklimb1 = t.c-1;
		if (  t.gun[t.gunid].settings.bulletmod == 1 ) 
		{
			for ( t.p = 0 ; t.p<=  t.gun[t.gunid].settings.bulletlimbsmax; t.p++ )
			{
				cstr tempstring = "BULLET";
				tempstring += Str(t.p);
				if (t.name_s == tempstring && t.gun[t.gunid].settings.bulletlimbstart + t.p <= g.bulletlimbsmax)  t.bulletlimbs[t.gun[t.gunid].settings.bulletlimbstart + t.p] = t.c - 1;
			}
		}
	}
	g.firemodes[t.gunid][0].settings.flaklimb=t.flaklimb1;
	g.firemodes[t.gunid][1].settings.flaklimb=t.flaklimb1;

	//  Use neighbor limb if certain other limbs not in model
	if (  t.smokelimb == -1 ) 
	{
		if (  t.flashlimb != -1  )  t.smokelimb = t.flashlimb;
	}

	//  brass limb not specified, use smoke hole
	if (  t.brasslimb == -1 && t.smokelimb >= 0  )  t.brasslimb = t.smokelimb;

	//  Store limbs in limb-data
	t.gun[t.gunid].settings.flashlimb=t.flashlimb;
	t.gun[t.gunid].settings.brasslimb=t.brasslimb;
	t.gun[t.gunid].settings.handlimb=t.handlimb;
	t.gun[t.gunid].settings.smokelimb=t.smokelimb;
	t.gun[t.gunid].settings.flashlimb2=t.flashlimb2;
	t.gun[t.gunid].settings.brasslimb2=t.brasslimb2;
	t.gun[t.gunid].settings.smokelimb2=t.smokelimb2;

	//  Also revert to GUN_D.DDS if no texture specified BUT there are no
	//  valid textures built into the HUD model
	t.tfoundvalidinternaltexture=0;
	sObject* pObject = GetObjectData ( t.currentgunobj );
	if ( pObject )
	{
		for ( int iMeshIndex = 0; iMeshIndex < pObject->iMeshCount; iMeshIndex++ )
		{
			sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
			if ( pMesh )
			{
				if ( pMesh->dwTextureCount > 0 )
				{
					if ( pMesh->pTextures[0].iImageID != 0 )
					{
						// at least one texture was successfully loaded by the gun model (so prefer them)
						t.tfoundvalidinternaltexture = 1;
					}
				}
			}
		}
	}
	/* old one checked for existence of files, but image formats can differ
	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		t.tlmbtex_s=LimbTextureName(t.currentgunobj,t.c-1);
		if (  Len(t.tlmbtex_s.Get())>1 ) 
		{
			cStr tfinallmbtex_s = "";
			tfinallmbtex_s = tfinallmbtex_s + "gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.tlmbtex_s;
			sprintf ( t.szwork , "Checking gun texture: %s" , tfinallmbtex_s.Get() );
			timestampactivity(0, t.szwork );
			if (  FileExist(tfinallmbtex_s.Get()) == 1 ) 
			{
				t.tfoundvalidinternaltexture=1;
			}
		}
	}
	*/
	if ( t.tfoundvalidinternaltexture == 0 ) 
	{
		t.gun[t.gunid].texd_s = "gun_D.dds";
	}

	//  Determine number of frames per keyframe
	if (  t.keyframeratio>0 ) 
	{
		t.ratio_f=t.keyframeratio;
	}
	else
	{
		t.ratio_f=1;
	}
	//  Determine number of alt frames per keyframe
	if (  t.altkeyframeratio>0 ) 
	{
		t.altratio_f=t.altkeyframeratio;
	}
	else
	{
		t.altratio_f=1;
	}

	//  Adjust animation data based on actual number of keyframes
	for ( t.i = 0 ; t.i<=  1; t.i++ )
	{
		if (  t.i == 1  )  t.ratio_f = t.altratio_f;
		g.firemodes[t.gunid][t.i].action.show.s = g.firemodes[t.gunid][t.i].action.show.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.show.e = g.firemodes[t.gunid][t.i].action.show.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.idle.s = g.firemodes[t.gunid][t.i].action.idle.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.idle.e = g.firemodes[t.gunid][t.i].action.idle.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.move.s = g.firemodes[t.gunid][t.i].action.move.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.move.e = g.firemodes[t.gunid][t.i].action.move.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.run.s = g.firemodes[t.gunid][t.i].action.run.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.run.e = g.firemodes[t.gunid][t.i].action.run.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.flattentochest.s = g.firemodes[t.gunid][t.i].action.flattentochest.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.flattentochest.e = g.firemodes[t.gunid][t.i].action.flattentochest.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.start.s = g.firemodes[t.gunid][t.i].action.start.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.start.e = g.firemodes[t.gunid][t.i].action.start.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.automatic.s = g.firemodes[t.gunid][t.i].action.automatic.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.automatic.e = g.firemodes[t.gunid][t.i].action.automatic.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.finish.s = g.firemodes[t.gunid][t.i].action.finish.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.finish.e = g.firemodes[t.gunid][t.i].action.finish.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.laststart.s = g.firemodes[t.gunid][t.i].action.laststart.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.laststart.e = g.firemodes[t.gunid][t.i].action.laststart.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.lastfinish.s = g.firemodes[t.gunid][t.i].action.lastfinish.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.lastfinish.e = g.firemodes[t.gunid][t.i].action.lastfinish.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.startreload.s = g.firemodes[t.gunid][t.i].action.startreload.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.startreload.e = g.firemodes[t.gunid][t.i].action.startreload.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.reloadloop.s = g.firemodes[t.gunid][t.i].action.reloadloop.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.reloadloop.e = g.firemodes[t.gunid][t.i].action.reloadloop.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.endreload.s = g.firemodes[t.gunid][t.i].action.endreload.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.endreload.e = g.firemodes[t.gunid][t.i].action.endreload.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.cock.s = g.firemodes[t.gunid][t.i].action.cock.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.cock.e = g.firemodes[t.gunid][t.i].action.cock.e * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.hide.s = g.firemodes[t.gunid][t.i].action.hide.s * t.ratio_f;
		g.firemodes[t.gunid][t.i].action.hide.e = g.firemodes[t.gunid][t.i].action.hide.e * t.ratio_f;
	}

	// If no effect, use global weapon shader (new weapons)
	if ( Len(t.gun[t.gunid].effect_s.Get())<3 ) 
	{
		// standard shader
		t.gun[t.gunid].effect_s = "effectbank\\reloaded\\weapon_basic.fx";

		// 300718 - also OLD weapons did not specify shader, so boost diffuse as they are DNS textures in PBR shader
		// but some other weapons are PBR but don't specify shader, so tone down this auto correction!
		t.gun[t.gunid].boostintensity = 0.1f;
	}

	// If weapon used old entity shader, use new weapon one
	if ( t.gun[t.gunid].effect_s == "effectbank\\reloaded\\entity_basic.fx" ) t.gun[t.gunid].effect_s = "effectbank\\reloaded\\weapon_basic.fx";

	// Load Effect ( (default is weapon_basic shader) )
	if ( t.gun[t.gunid].effect_s != "" ) 
	{
		t.tfile_s=t.gun[t.gunid].effect_s;
		t.teffectid=loadinternaleffect(t.tfile_s.Get());
		if ( stricmp ( t.gun[t.gunid].effect_s.Get(), "effectbank\\reloaded\\weapon_bone.fx" ) == NULL ) 
		{
			// store weapon shader effect IDs for NoZDepth renderer which uses 'CutOutDepth' technique to chizzel out gun
			// from the depth buffer without needing to reset the depth surface (messing up SAO and other depth effects)
			if ( g_weaponboneshadereffectindex == 0 ) g_weaponboneshadereffectindex = t.teffectid;
		}
		if (  t.teffectid == 0 ) 
		{
			//  revert to standard weapon shader if custom specified not exist
			t.gun[t.gunid].effect_s="effectbank\\reloaded\\weapon_basic.fx";
			t.tfile_s=t.gun[t.gunid].effect_s;
			t.teffectid=loadinternaleffect(t.tfile_s.Get());
		}
		if ( stricmp ( t.gun[t.gunid].effect_s.Get(), "effectbank\\reloaded\\weapon_basic.fx" ) == NULL ) 
		{
			// store weapon shader effect IDs for NoZDepth renderer which uses 'CutOutDepth' technique to chizzel out gun
			// from the depth buffer without needing to reset the depth surface (messing up SAO and other depth effects)
			if ( g_weaponbasicshadereffectindex == 0 ) g_weaponbasicshadereffectindex = t.teffectid;
		}
	}
	else
	{
		t.teffectid=0;
	}

	//  Load in special non-bone effect to accompany standard bone shader
	t.teffectid2=0;
	if ( t.gun[t.gunid].effect_s == "effectbank\\reloaded\\weapon_basic.fx" ) 
	{
		t.tfile_s="effectbank\\reloaded\\weapon_bone.fx";
		t.teffectid2=t.teffectid;
		t.teffectid=loadinternaleffect(t.tfile_s.Get());

		// store weapon shader effect IDs for NoZDepth renderer which uses 'CutOutDepth' technique to chizzel out gun
		// from the depth buffer without needing to reset the depth surface (messing up SAO and other depth effects)
		if ( g_weaponboneshadereffectindex == 0 ) g_weaponboneshadereffectindex = t.teffectid;
	}

	// 301117 - ensure weapns with bones and nonbones are handled
	if ( t.gun[t.gunid].effect_s == "effectbank\\reloaded\\weapon_bone.fx" ) 
	{
		t.teffectid2 = loadinternaleffect("effectbank\\reloaded\\weapon_basic.fx");
		if ( g_weaponbasicshadereffectindex == 0 ) g_weaponbasicshadereffectindex = t.teffectid2;
	}

	//  Reset gun textures
	t.gun[t.gunid].texdid=0;
	t.gun[t.gunid].texnid=0;
	t.gun[t.gunid].texmid=0;
	t.gun[t.gunid].texgid=0;
	t.gun[t.gunid].texaoid=0;
	t.gun[t.gunid].texiid=0;
	t.gun[t.gunid].texhid=0;

	//  First Textures are PLATES
	if (  t.teffectid>0 ) 
	{
		//  if texd$ specified, use that as the single texture to use
		cstr timgGloss_s = "";
		cstr timgAO_s = "";
		cstr timgHeight_s = "";
		t.tguntextureoverride=1;
		t.tguntexture_s=t.gun[t.gunid].texd_s;
		if ( Len(t.tguntexture_s.Get())<2 ) t.tguntextureoverride = 0;
		t.tguntexture_s=Left(t.tguntexture_s.Get(),Len(t.tguntexture_s.Get())-Len("_D.dds"));
		timestampactivity(0, "Loading gun textures" );
		if ( t.tguntextureoverride == 1 ) 
		{
			cstr pGunPath = ""; 
			pGunPath = pGunPath+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\";
			if (  g.gdividetexturesize == 0 ) 
			{
				t.imgD_s="effectbank\\reloaded\\media\\white_D.dds";
			}
			else
			{
				t.imgD_s = pGunPath + t.tguntexture_s+"_D.dds";
				if ( FileExist(t.imgD_s.Get()) == 0 ) t.imgD_s = pGunPath + t.tguntexture_s+"_color.dds";
				if ( FileExist(t.imgD_s.Get()) == 0 ) t.imgD_s = "effectbank\\reloaded\\media\\white_D.dds";
			}
			timestampactivity(0, cstr(cstr("Color=")+t.imgD_s).Get() );
			t.imgN_s = pGunPath + t.tguntexture_s+"_N.dds";
			if ( FileExist(t.imgN_s.Get()) == 0 ) t.imgN_s = pGunPath + t.tguntexture_s+"_normal.dds";
			if ( FileExist(t.imgN_s.Get()) == 0 ) t.imgN_s = "effectbank\\reloaded\\media\\blank_N.dds";
			timestampactivity(0, cstr(cstr("Normal=")+t.imgN_s).Get() );
			t.imgS_s = pGunPath + t.tguntexture_s+"_metalness.dds";
			if ( FileExist(t.imgS_s.Get()) == 0 ) t.imgS_s = "effectbank\\reloaded\\media\\blank_black.dds";
			timestampactivity(0, cstr(cstr("Metalness=")+t.imgS_s).Get() );
			t.imgI_s = pGunPath + t.tguntexture_s+"_I.dds";
			if ( FileExist(t.imgI_s.Get()) == 0 ) t.imgI_s = pGunPath + t.tguntexture_s+"_illumination.dds";
			if ( FileExist(t.imgI_s.Get()) == 0 ) t.imgI_s = "effectbank\\reloaded\\media\\blank_I.dds";
			timestampactivity(0, cstr(cstr("Illumination=")+t.imgI_s).Get() );
			timgGloss_s = pGunPath + t.tguntexture_s+"_gloss.dds";
			if ( FileExist(timgGloss_s.Get()) == 0 ) timgGloss_s = pGunPath + t.tguntexture_s+"_S.dds";;
			if ( FileExist(timgGloss_s.Get()) == 0 ) timgGloss_s = "effectbank\\reloaded\\media\\white_D.dds";
			timestampactivity(0, cstr(cstr("Gloss=")+timgGloss_s).Get() );
			timgAO_s = pGunPath + t.tguntexture_s+"_ao.dds";
			if ( FileExist(timgAO_s.Get()) == 0 ) timgAO_s = "effectbank\\reloaded\\media\\white_D.dds";
			timestampactivity(0, cstr(cstr("AO=")+timgAO_s).Get() );
			timgHeight_s = pGunPath + t.tguntexture_s+"_height.dds";
			if ( FileExist(timgHeight_s.Get()) == 0 ) timgHeight_s = "effectbank\\reloaded\\media\\blank_black.dds";
			timestampactivity(0, cstr(cstr("Height=")+timgHeight_s).Get() );
		}
		else
		{
			t.imgD_s="effectbank\\reloaded\\media\\white_D.dds";
			t.imgN_s="effectbank\\reloaded\\media\\blank_N.dds";
			t.imgS_s="effectbank\\reloaded\\media\\blank_black.dds";
			t.imgI_s="effectbank\\reloaded\\media\\blank_I.dds";
			timgGloss_s="effectbank\\reloaded\\media\\white_D.dds";
			timgAO_s="effectbank\\reloaded\\media\\white_D.dds";
			timgHeight_s="effectbank\\reloaded\\media\\blank_black.dds";
		}
		if ( t.gun[t.gunid].transparency > 2 ) 
		{
			t.imgDid=loadinternaltextureex(t.imgD_s.Get(),0,1);
			t.imgNid=loadinternaltextureex(t.imgN_s.Get(),0,1);
			t.imgSid=loadinternaltextureex(t.imgS_s.Get(),0,1);
			t.imgIid=loadinternaltextureex(t.imgI_s.Get(),0,1);
		}
		else
		{
			t.imgDid=loadinternaltextureex(t.imgD_s.Get(),5,0);
			t.imgNid=loadinternaltexture(t.imgN_s.Get());
			t.imgSid=loadinternaltexture(t.imgS_s.Get());
			t.imgIid=loadinternaltexture(t.imgI_s.Get());
		}
		int imgGlossid=loadinternaltexture(timgGloss_s.Get());
		int imgAOid=loadinternaltexture(timgAO_s.Get());
		int imgHeightid=loadinternaltexture(timgHeight_s.Get());

		// determine if need to texture ALL of model, or just the cube maps
		if ( t.tfoundvalidinternaltexture == 0 )
		{
			// apply textures to whole gun model
			if (g.memskipibr == 0) 
			{
				t.entityprofiletexibrid = t.terrain.imagestartindex + 32;
				TextureObject(t.currentgunobj, 8, t.entityprofiletexibrid);
			}
			TextureObject ( t.currentgunobj, 7, t.imgIid );
			if ( t.tguntextureoverride == 1 ) TextureObject ( t.currentgunobj, 0, t.imgDid );
			TextureObject ( t.currentgunobj, 1, imgAOid );
			TextureObject ( t.currentgunobj, 2, t.imgNid );
			TextureObject ( t.currentgunobj, 3, t.imgSid );
			TextureObject ( t.currentgunobj, 4, imgGlossid );
			TextureObject ( t.currentgunobj, 5, imgHeightid );
			int iPBRCubeImg = t.terrain.imagestartindex+31;
			TextureObject ( t.currentgunobj, 6, iPBRCubeImg );
			t.gun[t.gunid].texdid=t.imgDid;
			t.gun[t.gunid].texnid=t.imgNid;
			t.gun[t.gunid].texmid=t.imgSid;
			t.gun[t.gunid].texiid=t.imgIid;
			t.gun[t.gunid].texgid=imgGlossid;
			t.gun[t.gunid].texaoid=imgAOid;
			t.gun[t.gunid].texhid=imgHeightid;
		}
		else
		{
			// only apply cube map to model with pre-existing textures loaded
			int iPBRCubeImg = t.terrain.imagestartindex+31;
			TextureObject ( t.currentgunobj, 6, iPBRCubeImg );

			// some legacy weapons have multi-texture diffuse references, but need other textures populating
			// for new PBR shader
			if ( pObject )
			{
				for ( int iFrameIndex = 0; iFrameIndex < pObject->iFrameCount; iFrameIndex++ )
				{
					sFrame* pFrame = pObject->ppFrameList[iFrameIndex];
					if ( pFrame ) 
					{
						sMesh* pMesh = pFrame->pMesh;
						if ( pMesh )
						{
							if ( pMesh->dwTextureCount > 0 )
							{
								for ( int tt = pMesh->dwTextureCount-1; tt > 0; tt-- )
								{
									if ( pMesh->pTextures[tt].iImageID == 0 )
									{
										if ( tt == 8 )
										{
											if (g.memskipibr == 0) 
											{
												t.entityprofiletexibrid = t.terrain.imagestartindex + 32;
												TextureLimbStage ( t.currentgunobj, iFrameIndex, 8, t.entityprofiletexibrid );
											}
										}
										if ( tt == 7 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 7, t.imgIid );
										if ( tt == 5 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 5, imgHeightid );
										if ( tt == 4 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 4, imgGlossid );
										if ( tt == 3 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 3, t.imgSid );
										if ( tt == 2 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 2, t.imgNid );
										if ( tt == 1 ) TextureLimbStage ( t.currentgunobj, iFrameIndex, 1, imgAOid );
									}
								}
							}
						}
					}
				}
			}
		}

		//  Apply effect to object (special extra parameter to specify both BONE and NON-BONE effect types)
		t.gun[t.gunid].effectidused=t.teffectid;
		SetObjectEffectCore ( t.currentgunobj,t.teffectid,t.teffectid2,0 );

		//  Set position of sun for weapon shader
		SetVector4 ( g.weaponvectorindex,t.terrain.sundirectionx_f,t.terrain.sundirectiony_f,t.terrain.sundirectionz_f,0.0 );
		SetEffectConstantV ( t.teffectid,"LightSource",g.weaponvectorindex );
		SetVector4 ( g.weaponvectorindex,1,1,1,1.0 );
		SetEffectConstantV ( t.teffectid,"SurfColor",g.weaponvectorindex );
	}


	// STANDARD and ALT modes
	image_setlegacyimageloading(true);
	for ( t.i = 0 ; t.i <= 1; t.i++ )
	{
		// load in scope if any
		if ( g.firemodes[t.gunid][t.i].zoomscope_s != "" ) 
		{
			t.img_s = "";
			t.img_s=t.img_s +"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+g.firemodes[t.gunid][t.i].zoomscope_s;
			g.firemodes[t.gunid][t.i].zoomscope=loadinternaltextureex(t.img_s.Get(),5,0);
		}
		else
		{
			//  V109 BETA8 - try to load common scope files in case not specified in gunspec
			t.tzoomscope_s="scope_d2.dds";
			t.img_s = "";
			t.img_s=t.img_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.tzoomscope_s;
			g.firemodes[t.gunid][t.i].zoomscope=loadinternaltextureex(t.img_s.Get(),5,0);
			if (  g.firemodes[t.gunid][t.i].zoomscope == 0 ) 
			{
				t.tzoomscope_s="scope.dds";
				t.img_s = "";
				t.img_s=t.img_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.tzoomscope_s;
				g.firemodes[t.gunid][t.i].zoomscope=loadinternaltextureex(t.img_s.Get(),5,0);
				if (  g.firemodes[t.gunid][t.i].zoomscope == 0 ) 
				{
					t.tzoomscope_s="scope1.dds";
					t.img_s = "";
					t.img_s=t.img_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.tzoomscope_s;
					g.firemodes[t.gunid][t.i].zoomscope=loadinternaltextureex(t.img_s.Get(),5,0);
				}
			}

			//  load in ammo and icon images for status panel (if exist)
			SetMipmapNum(1); //PE: mipmaps not needed.
			if (  t.i == 0  )  t.talt_s = ""; else t.talt_s = "alt";
			t.timg_s = "";
			t.timg_s=t.timg_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.talt_s+"icon.png";
			g.firemodes[t.gunid][t.i].iconimg=loadinternaltextureex(t.timg_s.Get(),0,1);
			t.timg_s = "";
			t.timg_s=t.timg_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.talt_s+"ammo.png";
			g.firemodes[t.gunid][t.i].ammoimg=loadinternaltextureex(t.timg_s.Get(),0,1);
			SetMipmapNum(-1);

		}

		// rotate gun and fix pivot if GUNSPEC changes it
		if (  g.firemodes[t.gunid][t.i].settings.rotx_f != 0 || g.firemodes[t.gunid][t.i].settings.roty_f != 0 || g.firemodes[t.gunid][t.i].settings.rotz_f != 0 ) 
		{
			SetObjectRotationZYX (  t.currentgunobj );
			RotateObject (  t.currentgunobj,g.firemodes[t.gunid][t.i].settings.rotx_f,g.firemodes[t.gunid][t.i].settings.roty_f,g.firemodes[t.gunid][t.i].settings.rotz_f );
			FixObjectPivot (  t.currentgunobj );
		}
	}
	image_setlegacyimageloading(false);

	//  Glue gun to HUD-Gun-Marker
	GlueObjectToLimb (  t.currentgunobj,g.hudbankoffset+2,0 );

	//  Setup gun for correct visuals (special transparency for after-shadow setting)
	if (  t.gun[t.gunid].transparency>2 ) 
	{
		SetObjectTransparency (  t.currentgunobj,t.gun[t.gunid].transparency );
	}
	else
	{
		SetObjectTransparency (  t.currentgunobj,2 );
	}
	if (  g.globals.riftmode == 0 ) 
	{
		DisableObjectZDepth ( t.currentgunobj );
	}
	else
	{
		//  interferes with RIFTMODE
	}

	//  Setup gun for animation
	t.currentgunanimspeed_f=g.timeelapsed_f*(t.genericgunanimspeed_f*0.75);
	SetObjectSpeed (  t.currentgunobj,t.currentgunanimspeed_f );
	LoopObject (  t.currentgunobj );

	// Set art flags for weapon object (can use 32 bit flags here eventually)
	DWORD dwArtFlags = 0;
	if ( t.gun[t.gunid].invertnormal == 1 ) dwArtFlags = 1;
	if ( t.gun[t.gunid].preservetangents == 1 ) dwArtFlags |= 1<<1;
	SetObjectArtFlags ( t.currentgunobj, dwArtFlags, t.gun[t.gunid].boostintensity );

	//  Setup gun with muzzle flash image
	for ( t.i = 0 ; t.i <= 1; t.i++ )
	{
		//  Mussle flash
		t.num = g.firemodes[t.gunid][t.i].settings.muzzleflash ; if (  t.num == 0  )  t.num = 1;
		t.size_f = g.firemodes[t.gunid][t.i].settings.muzzlesize_f ; if (  t.size_f == 0.0  )  t.size_f = 100.0;
		t.muzzleflash_s="gamecore\\muzzleflash\\flash";
		t.muzzleflash_s+=Str(t.num);
		t.muzzleflash_s+=".dds";
		t.imgid=loadmuzzle(t.muzzleflash_s.Get());
		g.firemodes[t.gunid][t.i].settings.flashimg=t.imgid;

		//  Setup gun with brass models
		t.num = g.firemodes[t.gunid][t.i].settings.brass ; if (  t.num == 0  )  t.num = 1;
		t.brass_s = "";
		t.brass_s=t.brass_s+"gamecore\\brass\\brass";
		t.brass_s += Str(t.num);
		t.brass_s += "\\brass";
		t.brass_s += Str(t.num);
		t.brass_s += ".x";
		t.brassobj=loadbrass(t.brass_s.Get());
		if (  t.brassobj == 0 ) 
		{
			//  specifying a brass value that does not exist crashes engine
			t.num=1 ; g.firemodes[t.gunid][t.i].settings.brass=0;
			t.brass_s = "";
			t.brass_s=t.brass_s+"gamecore\\brass\\brass"+Str(t.num)+"\\brass"+Str(t.num)+".x";
			t.brassobj=loadbrass(t.brass_s.Get());
		}
		g.firemodes[t.gunid][t.i].settings.brassobjmaster=t.brassobj;

		//  Setup gun with second brass models
		t.num = g.firemodes[t.gunid][t.i].settings.secondbrass; if (t.num == 0)  t.num = 1;
		t.brass_s = "";
		t.brass_s = t.brass_s + "gamecore\\brass\\brass";
		t.brass_s += Str(t.num);
		t.brass_s += "\\brass";
		t.brass_s += Str(t.num);
		t.brass_s += ".x";
		t.brassobj = loadbrass(t.brass_s.Get());
		if (t.brassobj == 0)
		{
			//  specifying a brass value that does not exist crashes engine
			t.num = 1; g.firemodes[t.gunid][t.i].settings.secondbrass = 0;
			t.brass_s = "";
			t.brass_s = t.brass_s + "gamecore\\brass\\brass" + Str(t.num) + "\\brass" + Str(t.num) + ".x";
			t.brassobj = loadbrass(t.brass_s.Get());
		}
		g.firemodes[t.gunid][t.i].settings.secondbrassobjmaster = t.brassobj;

		//  Setup gun with smoke images
		t.num=g.firemodes[t.gunid][t.i].settings.smoke;
		if (  Len(g.firemodes[t.gunid][t.i].settings.smokedecal_s.Get())>0 ) 
		{
			t.smoke_s = "";
			t.smoke_s=t.smoke_s+"gamecore\\decals\\"+g.firemodes[t.gunid][t.i].settings.smokedecal_s+"\\decal.dds";
			t.imgid=loadsmoke(t.smoke_s.Get());
		}
		else
		{
			if (  t.num == 0  )  t.num = 1;
			if (  t.num == 1 ) 
			{
				t.smoke_s="gamecore\\decals\\gunsmoke\\decal.dds";
			}
			else
			{
				t.smoke_s="gamecore\\decals\\smoke";
				t.smoke_s+=Str(t.num);
				t.smoke_s+="\\decal.dds";
			}
			t.imgid=loadsmoke(t.smoke_s.Get());
		}
		g.firemodes[t.gunid][t.i].settings.smokeimg=t.imgid;
	}

	//  Setup gun with crosshair
	t.crosshair_s = "";
	t.crosshair_s=t.crosshair_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\crosshair.dds";
	image_setlegacyimageloading(true);
	SetMipmapNum(1); //PE: mipmaps not needed.
	t.crosshairimage=loadinternalimagecompressquality(t.crosshair_s.Get(),5,1);
	SetMipmapNum(-1);
	image_setlegacyimageloading(false);
	t.gun[t.gunid].settings.crosshairimg=t.crosshairimage;
	t.gun[t.gunid].secondobj=0;

	//  Load gun sounds and companions
	for ( t.p = 0 ; t.p < 25; t.p++ ) // <= 15
	{
		t.tgname_s = t.gunsound[t.gunid][t.p].name_s;
		if ( t.tgname_s != "" ) 
		{
			// main sound for player
			t.tvariationsavailable=0;
			t.snd_s = "";
			t.snd_s=t.snd_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.tgname_s;
			if (  cstr(Lower(Right(t.tgname_s.Get(),5))) == "1.wav" ) 
			{
				sprintf ( t.szwork , "%s1.wav" , Left(t.snd_s.Get(),Len(t.snd_s.Get())-5) );
				t.gunsound[t.gunid][t.p].soundid1=loadinternalsound( t.szwork );
				sprintf ( t.szwork , "%s2.wav" , Left(t.snd_s.Get(),Len(t.snd_s.Get())-5) );
				t.gunsound[t.gunid][t.p].soundid2=loadinternalsound( t.szwork );
				sprintf ( t.szwork , "%s3.wav" , Left(t.snd_s.Get(),Len(t.snd_s.Get())-5) );
				t.gunsound[t.gunid][t.p].soundid3=loadinternalsound( t.szwork );
				sprintf ( t.szwork , "%s4.wav" , Left(t.snd_s.Get(),Len(t.snd_s.Get())-5) );
				t.gunsound[t.gunid][t.p].soundid4=loadinternalsound( t.szwork );
				t.soundloopcheckpoint[t.gunsound[t.gunid][t.p].soundid1]=2;
				t.soundloopcheckpoint[t.gunsound[t.gunid][t.p].soundid2]=2;
				t.soundloopcheckpoint[t.gunsound[t.gunid][t.p].soundid3]=2;
				t.soundloopcheckpoint[t.gunsound[t.gunid][t.p].soundid4]=2;
				t.tvariationsavailable=1;
			}
			else
			{
				t.gunsound[t.gunid][t.p].soundid1=loadinternalsound(t.snd_s.Get());
				t.gunsound[t.gunid][t.p].soundid2=0;
				t.gunsound[t.gunid][t.p].soundid3=0;
				t.gunsound[t.gunid][t.p].soundid4=0;
				t.soundloopcheckpoint[t.gunsound[t.gunid][t.p].soundid1]=2;
			}
			// extra sound value checks in many places
			if (  t.gunsound[t.gunid][t.p].soundid1>0 ) 
			{
				if (  SoundExist(t.gunsound[t.gunid][t.p].soundid1) == 0 ) 
				{
					t.gunsound[t.gunid][t.p].soundid1=0;
				}
			}
			// companion sounds for other weapon sound uses
			if (  t.gunsound[t.gunid][t.p].soundid1>0 ) 
			{
				if (  t.p <= 3 ) 
				{
					if (  t.tvariationsavailable == 1  ) { t.snd_s = Left(t.snd_s.Get(),Len(t.snd_s.Get())-5); t.snd_s += "2.wav"; }
					t.gunsoundcompanion[t.gunid][t.p][0].soundid=loadinternalsoundcore(t.snd_s.Get(),1);
					if (  t.tvariationsavailable == 1  ) { t.snd_s = Left(t.snd_s.Get(),Len(t.snd_s.Get())-5); t.snd_s += "3.wav"; }
					t.gunsoundcompanion[t.gunid][t.p][1].soundid=loadinternalsoundcorecloneflag(t.snd_s.Get(),1,t.gunsoundcompanion[t.gunid][t.p][0].soundid);
					if (  t.tvariationsavailable == 1  ) { t.snd_s = Left(t.snd_s.Get(),Len(t.snd_s.Get())-5); t.snd_s += "4.wav"; }
					t.gunsoundcompanion[t.gunid][t.p][2].soundid=loadinternalsoundcorecloneflag(t.snd_s.Get(),1,t.gunsoundcompanion[t.gunid][t.p][0].soundid);
				}
			}
			else
			{
				if (  t.p <= 3 ) 
				{
					t.gunsoundcompanion[t.gunid][t.p][0].soundid=0;
					t.gunsoundcompanion[t.gunid][t.p][1].soundid=0;
					t.gunsoundcompanion[t.gunid][t.p][2].soundid=0;
				}
			}
		}
	}

	//  Load gun altsounds
	for ( t.p = 0 ; t.p < 25; t.p++ )
	{
		if (  t.p != 2 ) 
		{
			t.snd_s = "";
			t.snd_s=t.snd_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\"+t.gunsound[t.gunid][t.p].altname_s;
			t.gunsound[t.gunid][t.p].altsoundid=loadinternalsound(t.snd_s.Get());
			if (  t.gunsound[t.gunid][t.p].altsoundid>0 ) 
			{
				if (  SoundExist(t.gunsound[t.gunid][t.p].altsoundid) == 0 ) 
				{
					t.gunsound[t.gunid][t.p].altsoundid=0;
				}
			}
		}
	}

	//  Load HUD image (ammo and weapon selected image)
	t.img_s = "";
	t.img_s=t.img_s+"gamecore\\"+g.fpgchuds_s+"\\"+t.gun_s+"\\hud_icon.dds";
	image_setlegacyimageloading(true);
	SetMipmapNum(1); //PE: mipmaps not needed.
	t.gun[t.gunid].hudimage=loadinternalimagecompressquality(t.img_s.Get(),5,1);
	SetMipmapNum(-1);
	image_setlegacyimageloading(false);

	//  Find and store projectile index for later use
	t.tProjectileType_s=t.gun[t.gunid].projectile_s  ; weapon_getprojectileid ( );
	for ( t.i = 0 ; t.i<=  1; t.i++ )
	{
		g.firemodes[t.gunid][t.i].settings.flakindex=t.tProjectileType;
	}

	//  reset this as only used when gun 'selected'
	t.currentgunobj=0;
}

void gun_updategunshaders ( void )
{
	// update all gun shaders
	for ( t.tgunid = 1 ; t.tgunid<=  g.gunmax; t.tgunid++ )
	{
		t.teffectid=t.gun[t.tgunid].effectidused;
		if (  t.teffectid>0 ) 
		{
			if (  GetEffectExist(t.teffectid) == 1 ) 
			{
				SetVector4 (  g.weaponvectorindex,t.terrain.sundirectionx_f,t.terrain.sundirectiony_f,t.terrain.sundirectionz_f,0.0 );
				SetEffectConstantV (  t.teffectid,"LightSource",g.weaponvectorindex );
				SetVector4 (  g.weaponvectorindex,1,1,1,1.0 );
				SetEffectConstantV (  t.teffectid,"SurfColor",g.weaponvectorindex );
			}
		}
	}
}

void gun_freeafterlevel ( void )
{
	// remember gun when leave the level
	t.lastgunid = t.gunid;

	// 020516 - only if not standalone in game
	g.autoloadgun = 0; gun_change ( );
}

void gun_freeguns ( void )
{

	//  deselect gun in hand
	gun_freeafterlevel ( );

	//  hide and clean guns (going back to level)
	for ( t.gunid = 1 ; t.gunid<=  g.gunmax; t.gunid++ )
	{
		t.tobj=t.gun[t.gunid].obj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1  )  HideObject (  t.tobj );
		}
	}

return;

}

void gun_free ( void )
{
	//  Hide gun from HUD
	if (  t.currentgunobj>0 ) 
	{
		if (  ObjectExist(t.currentgunobj) == 1 ) 
		{
			SetObjectInterpolation (  t.currentgunobj,100 );
			SetObjectFrame (  t.currentgunobj,g.firemodes[t.gunid][0].action.show.s );
			HideObject (  t.currentgunobj );
		}
	}

	//  Stop any gun sounds if free suddenly
	if (  t.gunid>0 ) 
	{
		for ( t.p = 0 ; t.p <  25; t.p++ )
		{
			if (  t.p != 4 ) 
			{
				//  080415 - except put away which we do not want to cut off
				if (  t.gunsound[t.gunid][t.p].soundid1>0  )  StopSound (  t.gunsound[t.gunid][t.p].soundid1 );
				if (  t.gunsound[t.gunid][t.p].soundid2>0  )  StopSound (  t.gunsound[t.gunid][t.p].soundid2 );
				if (  t.gunsound[t.gunid][t.p].soundid3>0  )  StopSound (  t.gunsound[t.gunid][t.p].soundid3 );
				if (  t.gunsound[t.gunid][t.p].soundid4>0  )  StopSound (  t.gunsound[t.gunid][t.p].soundid4 );
			}
		}
		for ( t.p = 1 ; t.p<=  3; t.p++ )
		{
			if (  t.gunsoundcompanion[t.gunid][t.p][0].soundid>0  )  StopSound (  t.gunsoundcompanion[t.gunid][t.p][0].soundid );
			if (  t.gunsoundcompanion[t.gunid][t.p][1].soundid>0  )  StopSound (  t.gunsoundcompanion[t.gunid][t.p][1].soundid );
			if (  t.gunsoundcompanion[t.gunid][t.p][2].soundid>0  )  StopSound (  t.gunsoundcompanion[t.gunid][t.p][2].soundid );
		}
	}

	//  Disassociate gun with player
	t.currentgunobj=0;

	//  Hide support objects for gun
	if (  t.gun[t.gunid].settings.flashlimb != -1 ) 
	{
		t.obj=g.hudbankoffset+5;
		if (  ObjectExist(t.obj) == 1  )  HideObject (  t.obj );
	}
	if (  t.gun[t.gunid].settings.brasslimb != -1 ) 
	{
		for ( t.o = 6 ; t.o<=  20; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			if ( ObjectExist(t.obj) == 1 )  
			{
				ODEDestroyObject ( t.obj );
				HideObject ( t.obj );
			}
		}
		for (t.o = 35; t.o <= 49; t.o++)
		{
			t.obj = g.hudbankoffset + t.o;
			if (ObjectExist(t.obj) == 1)
			{
				ODEDestroyObject(t.obj);
				HideObject(t.obj);
			}
		}
	}
	if (  t.gun[t.gunid].settings.smokelimb != -1 ) 
	{
		for ( t.o = 21 ; t.o<=  30; t.o++ )
		{
			t.obj=g.hudbankoffset+t.o;
			if (  ObjectExist(t.obj) == 1  )  HideObject (  t.obj );
		}
	}

	// stop and clear any idle sound loop
	if ( t.gunactiveidlesoundloopindex != 0 )
	{
		if ( t.gunactiveidlesoundloopindex > 0 && SoundExist ( t.gunactiveidlesoundloopindex ) == 1 ) StopSound ( t.gunactiveidlesoundloopindex );
		t.gunactiveidlesoundloopindex = 0;
	}

	//  Clear basic gun vars
	t.gunflash=0 ; t.gunsmoke=0 ; t.gunbrass=0 ; t.gunshoot=0 ; t.gunmode=5;
	t.gunbrasstrigger = 0;
	t.secondgunbrasstrigger = 0;

}

void gun_releaseresources ( void )
{

	//  delete gun resources
	timestampactivity(0,"_gun_releaseresources");
	for ( t.gunid = 1 ; t.gunid<=  g.gunmax; t.gunid++ )
	{
		t.tobj=t.gun[t.gunid].obj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				DeleteObject (  t.tobj );
			}
			t.gun[t.gunid].obj=0;
		}
		//  remove images fully
		removeinternaltexture(t.gun[t.gunid].texdid);
		removeinternaltexture(t.gun[t.gunid].texnid);
		removeinternaltexture(t.gun[t.gunid].texmid);
		removeinternaltexture(t.gun[t.gunid].texiid);
		removeinternaltexture(t.gun[t.gunid].texgid);
		removeinternaltexture(t.gun[t.gunid].texaoid);
		removeinternaltexture(t.gun[t.gunid].texhid);
	}

	//  reset gunbank
	for ( t.g = 0; t.g <= g.gunbankmaxlimit; t.g++ )
	{
		t.gunbank_s[t.g]="";
	}
	g.gunbankmax=0;
}

void gun_tagmpgunstolist ( void )
{
	if (  t.game.runasmultiplayer == 1 ) 
	{
		timestampactivity(0,"tagging MP guns to end of t.gun bank list");
		t.olddir_s=GetDir();
		t.tpath_s = SteamGetWorkshopItemPath();
		if (  PathExist(t.tpath_s.Get()) == 1 ) 
		{
			SetDir (  t.tpath_s.Get() );
			//  looking for i.e. gamecore_guns_futuristic_futuretekshotgun_gunspec.txt
			ChecklistForFiles (  );
			for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
			{
				t.tfile_s=Lower(ChecklistString(t.c));
				if (  t.tfile_s != "." && t.tpath_s != ".." ) 
				{
					if (  cstr(Right(t.tfile_s.Get(),11)) == "gunspec.txt" ) 
					{
						t.tfullfile_s = t.tfile_s;
						//  include this gun
						t.tgunname_s=Right(t.tfile_s.Get(),Len(t.tfile_s.Get())-14);
						t.tgunname_s=Left(t.tgunname_s.Get(),Len(t.tgunname_s.Get())-12);
						for ( t.n = 1 ; t.n<=  Len(t.tgunname_s.Get()); t.n++ )
						{
							if (  cstr(Mid(t.tgunname_s.Get(),t.n)) == "_" ) 
							{
								t.tgunfolder_s=Left(t.tgunname_s.Get(),t.n-1);
								t.tgunname_s=Right(t.tgunname_s.Get(),Len(t.tgunname_s.Get())-Len(t.tgunfolder_s.Get())-1);
								t.n=Len(t.tgunname_s.Get()) ; break;
							}
						}
						//  add temporarily to gun list for this game session
						t.tfile_s=t.tgunfolder_s+"\\"+t.tgunname_s;
						t.treplace_s = t.tfile_s;
						t.tfile_s = "";
						//  07032015 - 016 - filenames that have _ in this as a character get swapped to @ so we dont think they are folders
						for ( t.n = 1 ; t.n<=  Len(t.treplace_s.Get()); t.n++ )
						{
							if (  t.treplace_s.Get()[t.n-1]  ==  '@'  )  t.tfile_s  =  t.tfile_s + "_"; else t.tfile_s  =  t.tfile_s + Mid(t.treplace_s.Get(),t.n);
						}
						++g.gunmax ; if (  g.gunmax>g.maxgunsinengine  )  g.gunmax = g.maxgunsinengine;
						t.gun[g.gunmax].name_s=t.tfile_s;
						t.gun[g.gunmax].extraformp=1;
						sprintf ( t.szwork , "mp t.temp t.gun %s:%s" , Str(g.gunmax) , t.tfile_s.Get() );
						timestampactivity(0, t.szwork );

						//  09032014 - 018 - adding decals into mp
						if (  FileOpen(3)  )  CloseFile (  3 );
						t.tfoundflash = 0;
						OpenToRead (  3,t.tfullfile_s.Get() );
						t.tfoundflash = 0;
						while (  FileEnd(3)  ==  0 && t.tfoundflash  ==  0 ) 
						{
							t.tisthisdecl_s = ReadString ( 3 );
							if (  cstr(Left(t.tisthisdecl_s.Get(),5) ) ==  "decal" ) 
							{
								t.tlocationofequals = FindLastChar(t.tisthisdecl_s.Get(),"=");
								if (  t.tlocationofequals > 1 ) 
								{
									if ( cstr( Mid(t.tisthisdecl_s.Get(),t.tlocationofequals+1) )  ==  " " ) 
									{
										t.tdecal_s = Right(t.tisthisdecl_s.Get(),Len(t.tisthisdecl_s.Get())-(t.tlocationofequals+1));
									}
									else
									{
										t.tdecal_s = Right(t.tisthisdecl_s.Get(),Len(t.tisthisdecl_s.Get())-(t.tlocationofequals));
									}
									t.tfext_s = "";
									t.tfounddecaltex = 0;
									sprintf ( t.szwork , "gamecore\\decals\\%s\\decal.png" , t.tdecal_s.Get() );
									if (  FileExist( t.szwork )  ==  1  )  t.tfext_s  =  ".png";
									sprintf ( t.szwork , "gamecore\\decals\\%s\\decal.dds" , t.tdecal_s.Get() );
									if (  FileExist( t.szwork )  ==  1  )  t.tfext_s  =  ".dds";
									if (  t.tfext_s  !=  "" ) 
									{
										t.tfounddecaltex = 1;
									}
									t.tfext_s = "";
									t.tfounddecalspec = 0;
									sprintf ( t.szwork , "gamecore\\decals\\%s\\decalspec.txt" , t.tdecal_s.Get() );
									if (  FileExist( t.szwork )  ==  1  )  t.tfext_s  =  ".txt";
									if (  t.tfext_s  !=  "" ) 
									{
										t.tfounddecalspec = 1;
									}
									if (  t.tfounddecalspec  ==  1 && t.tfounddecaltex  ==  1 ) 
									{

										t.newdecal_s=t.tdecal_s;
										for ( t.tdecalid = 1 ; t.tdecalid<=  g.decalmax; t.tdecalid++ )
										{
											if (  t.decal[t.tdecalid].name_s == t.newdecal_s  )  break;
										}
										if (  t.tdecalid>g.decalmax ) 
										{
											if (  t.decalid>g.decalmax ) 
											{
												g.decalmax=t.decalid;
												Dim (  t.decal,g.decalmax  );
											}
											t.decal[t.decalid].name_s=t.newdecal_s;
											t.decal[t.decalid].active=1;
											t.decal_s = t.newdecal_s;
											decal_load ( );
											++t.decalid;
											g.decalmax=t.decalid-1;
										}

									}
								}
							}
						}
						CloseFile (  3 );


					}
				}
			}

			//  09032014 - 018 - adding decals into mp

		}
		SetDir (  t.olddir_s.Get() );
	}
return;

}

void gun_removempgunsfromlist ( void )
{

	//  remove any guns added for MP session
	if (  t.game.runasmultiplayer == 1 ) 
	{
		t.twasgunmax=g.gunmax;
		while (  t.gun[g.gunmax].extraformp == 1 && g.gunmax >= 1 ) 
		{
			t.gun[g.gunmax].extraformp=0;
			--g.gunmax;
		}
		sprintf ( t.szwork , "removing MP guns, reducing t.gun list from %i to %i" , t.twasgunmax , g.gunmax );
		timestampactivity(0, t.szwork );
	}
	return;
}

void gun_playerdead ( void )
{
	t.gunzoommode=0;
	t.gunzoommag_f=0;
	t.gunshoot=0;
	g.firemode=0;
	change_brass_firemode(); //PE: check if we need to change to another brass set.
	t.gunmode=131;
	if (  t.gunmodeloopsnd>0 ) 
	{
		if (  SoundExist(t.gunmodeloopsnd) == 1 ) 
		{
			StopSound (  t.gunmodeloopsnd );
		}
		t.gunmodeloopsnd=0;
	}
	if (  t.currentgunobj>0 ) 
	{
		if (  ObjectExist(t.currentgunobj) == 1 ) 
		{
			StopObject (  t.currentgunobj );
		}
	}
	return;
}

int loadgun ( int gunid, char* tfile_s )
{
	int index = 0;
	cstr tcc_s =  "";
	int tflag = 0;
	int tc = 0;
	int tt;
	index=0;
	if ( g.gunbankmax>0 ) 
	{
		for ( tt = 1; tt <= g.gunbankmax; tt++ )
		{
			if ( strcmp ( tfile_s , t.gunbank_s[tt].Get() ) == 0 ) { index = g.gunbankoffset+tt  ; break; }
		}
	}
	else
	{
		tt=g.gunbankmax+1;
	}
	if ( tt>g.gunbankmax ) 
	{
		++g.gunbankmax;

		if ( FileExist(tfile_s) == 1 ) 
		{
			index=g.gunbankoffset+g.gunbankmax;
			t.gunbank_s[g.gunbankmax]=tfile_s;

			//  07032014 - 016 - the object can sometimes exist already after workshop guns have been used, so we deleted it if it does
			if (  ObjectExist(index)  )  DeleteObject (  index );

			char pChopFile[512];
			strcpy ( pChopFile, tfile_s );
			if (strnicmp(pChopFile + strlen(pChopFile) - 2, ".x", 2) == NULL)
			{
				pChopFile[strlen(pChopFile) - 2] = 0;
				cstr pFileToLoad = cstr(pChopFile) + cstr(".DBO");
				if (FileExist(pFileToLoad.Get()) == 1)
					LoadObject(pFileToLoad.Get(), index);
				else
					LoadObject(tfile_s, index);
			}
			else
				LoadObject(tfile_s, index);

			//  hide any limbs in the weapon which should NEVER be rendered (and cause D3DX shader error)
			PerformCheckListForLimbs (  index );
			if (  t.gun[gunid].settings.minpolytrim>0 ) 
			{
				for ( tc = 1 ; tc<=  ChecklistQuantity(); tc++ )
				{
					tcc_s=Lower(ChecklistString(tc));
					tflag=0;
					if (  tcc_s == "firespot"  )  tflag = 1;
					if (  tcc_s == "firespot02"  )  tflag = 1;
					if (  tcc_s == "x3ds_firespot"  )  tflag = 1;
					if (  tcc_s == "brass"  )  tflag = 1;
					if (  tcc_s == "brass02"  )  tflag = 1;
					if (  tcc_s == "x3ds_brass"  )  tflag = 1;
					if (  tcc_s == "_smoke"  )  tflag = 1;
					if (  tcc_s == "smoke"  )  tflag = 1;
					if (  tcc_s == "smoke2"  )  tflag = 1;
					if (  tcc_s == "x3ds_smoke"  )  tflag = 1;
					if (  tcc_s == "hand"  )  tflag = 1;
					if (  tcc_s == "x3ds_hand"  )  tflag = 1;
					if (  tcc_s == "camera01"  )  tflag = 1;
					//  lee - 091014 - some weapons have leftovers!! (sniper)
					if (  tcc_s == "lens001"  )  tflag = 1;
					if (  tcc_s == "omni001"  )  tflag = 1;
					if (  tcc_s == "omni002"  )  tflag = 1;
					//  lee - 071014 - also hide any limbs that are boxes (leftovers from gun marker work)
					if (  GetLimbPolygonCount(index,tc-1) <= t.gun[gunid].settings.minpolytrim  )  tflag = 1;
					if (  tflag == 1 ) 
					{
						HideLimb (  index,tc-1 );
					}
				}
			}

			//  prepare weapon object
			SetObjectCollisionOff (  index );
			SetObjectInterpolation (  index,100 );
			SetObjectFrame (  index,g.firemodes[gunid][0].action.show.s );
			HideObject (  index );


		}
	}
	return index;
}

int createsecondgun ( void )
{
	int index = 0;
	++g.gunbankmax;
	t.gunbank_s[g.gunbankmax]="second";
	index=g.gunbankoffset+g.gunbankmax;
	return index;
}

int loadbrass ( char* tfile_s )
{
	int tbrassDimg = 0;
	int tbrassNimg = 0;
	int tbrassSimg = 0;
	cstr tdbofile_s =  "";
	cstr ttexdiff_s =  "";
	int teffectid = 0;
	int index = 0;
	int tt = 0;
	index=0;
	if (  g.brassbankmax>0 ) 
	{
		for ( tt = 1 ; tt<=  g.brassbankmax; tt++ )
		{
			if ( strcmp ( tfile_s , t.brassbank_s[tt].Get() ) == 0 ) {  index = g.brassbankoffset+tt ; break; }
		}
	}
	else
	{
		tt=g.brassbankmax+1;
	}
	if (  tt>g.brassbankmax ) 
	{
		//  get texture file from X file
		if (  cstr(Lower(Right(tfile_s,2))) == ".x" ) 
		{
			ttexdiff_s=Left(tfile_s,Len(tfile_s)-2);
		}
		else
		{
			ttexdiff_s=Left(tfile_s,Len(tfile_s)-4);
		}
		// replaced X file load with optional DBO convert/load for HUD.X
		deleteOutOfDateDBO(tfile_s);
		if ( cstr(Lower(Right(tfile_s,2))) == ".x"  )  
		{ 
			tdbofile_s = Left(tfile_s,Len(tfile_s)-2); tdbofile_s += ".dbo";
		} 
		else 
		{
			tdbofile_s = "";
		}
		if ( FileExist(tfile_s) == 1 || FileExist(tdbofile_s.Get()) == 1 ) 
		{
			++g.brassbankmax;
			index=g.brassbankoffset+g.brassbankmax;
			t.brassbank_s[g.brassbankmax]=tfile_s;
			if ( FileExist(tdbofile_s.Get()) == 1 ) 
			{
				strcpy ( tfile_s, tdbofile_s.Get() );
				tdbofile_s="";
			}
			else
			{
				// allowed to save DBO (once only)
			}
			LoadObject ( tfile_s, index );
			if ( Len(tdbofile_s.Get())>1 ) 
			{
				if ( FileExist(tdbofile_s.Get()) == 0 ) 
				{
					// unnecessary now as LoadObject auto creates DBO file!
					SaveObject ( tdbofile_s.Get(),index );
				}
				if (  FileExist(tdbofile_s.Get()) == 1 ) 
				{
					DeleteObject (  index );
					LoadObject (  tdbofile_s.Get(),index );
					strcpy ( tfile_s , tdbofile_s.Get() );
				}
			}

			// Determine if PBR or non-PBR
			bool bHavePBRTextures = false;
			sprintf ( t.szwork, "%s_color.dds", ttexdiff_s.Get() ); if ( FileExist (t.szwork) ) bHavePBRTextures = true;
			sprintf ( t.szwork, "%s_color.png", ttexdiff_s.Get() ); if ( FileExist (t.szwork) ) bHavePBRTextures = true;
			if ( g.gpbroverride == 1 && bHavePBRTextures == true )
			{
				// PBR texturing
				sprintf ( t.szwork , "%s_color.png" , ttexdiff_s.Get() );
				int tbrassCOLORimg = loadinternalimage(t.szwork);
				sprintf ( t.szwork , "%s_normal.png" , ttexdiff_s.Get() );
				int tbrassNORMALimg = loadinternalimage(t.szwork);
				sprintf ( t.szwork , "%s_metalness.png" , ttexdiff_s.Get() );
				int tbrassMETALNESSimg = loadinternalimage(t.szwork);
				if ( tbrassMETALNESSimg == 0 ) tbrassMETALNESSimg = loadinternalimage("effectbank\\reloaded\\media\\white_D.dds");
				sprintf ( t.szwork , "%s_gloss.png" , ttexdiff_s.Get() );
				int tbrassGLOSSimg = loadinternalimage(t.szwork);
				if ( tbrassGLOSSimg == 0 ) tbrassGLOSSimg = loadinternalimage("effectbank\\reloaded\\media\\white_D.dds");
				sprintf ( t.szwork , "%s_ao.png" , ttexdiff_s.Get() );
				int tbrassAOimg = loadinternalimage(t.szwork);
				if ( tbrassAOimg == 0 ) tbrassAOimg = loadinternalimage("effectbank\\reloaded\\media\\white_D.dds");
				sprintf ( t.szwork , "%s_illumination.png" , ttexdiff_s.Get() );
				int tbrassILLUMimg = loadinternalimage(t.szwork);
				if ( tbrassILLUMimg == 0 ) tbrassILLUMimg = loadinternalimage("effectbank\\reloaded\\media\\blank_black.dds");
				int tbrassHEIGHTimg = loadinternalimage("effectbank\\reloaded\\media\\blank_black.dds");
				// and texture the object
				if (g.memskipibr == 0) 
				{
					int iPBRIBRImg = t.terrain.imagestartindex + 32;
					TextureObject(index, 8, iPBRIBRImg);
				}
				TextureObject ( index, 7, tbrassILLUMimg );
				TextureObject ( index, 0, tbrassCOLORimg );
				TextureObject ( index, 1, tbrassAOimg );
				TextureObject ( index, 2, tbrassNORMALimg );
				TextureObject ( index, 3, tbrassMETALNESSimg );
				TextureObject ( index, 4, tbrassGLOSSimg );
				TextureObject ( index, 5, tbrassHEIGHTimg );
				int iPBRCubeImg = t.terrain.imagestartindex+31;
				TextureObject ( index, 6, iPBRCubeImg );
				teffectid=loadinternaleffect("effectbank\\reloaded\\apbr_illum.fx");
			}
			else
			{
				// DNS texturing (non-PBR)
				sprintf ( t.szwork , "%s_D.dds" , ttexdiff_s.Get() );
				tbrassDimg=loadinternalimage(t.szwork);
				sprintf ( t.szwork , "%s_N.dds" , ttexdiff_s.Get() );
				tbrassNimg=loadinternalimage(t.szwork);
				sprintf ( t.szwork , "%s_S.dds" , ttexdiff_s.Get() );
				tbrassSimg=loadinternalimage(t.szwork);
				// and texture the object
				TextureObject (  index,0,tbrassDimg );
				TextureObject (  index,1,loadinternalimagecompressquality("effectbank\\reloaded\\media\\blank_O.dds",1,0) );
				TextureObject (  index,2,tbrassNimg );
				TextureObject (  index,3,tbrassSimg );
				TextureObject (  index,4,t.terrain.imagestartindex );
				TextureObject (  index,5,g.postprocessimageoffset+5 );
				TextureObject (  index,6,loadinternalimagecompressquality("effectbank\\reloaded\\media\\blank_I.dds",1,0) );
				teffectid=loadinternaleffect("effectbank\\reloaded\\entity_basic.fx");
			}
			SetObjectEffect (  index,teffectid );
			RotateObject (  index,0,180,0  ); FixObjectPivot (  index );
			SetObjectCollisionOff (  index );
			SetObjectMask (  index, 1 );
			HideObject (  index );
		}
	}
	return index;
}

int loadmuzzle ( char* tfile_s )
{
	int index = 0;
	int tt = 0;
	index=0;
	if (  g.muzzlebankmax>0 ) 
	{
		for ( tt = 1 ; tt<=  g.muzzlebankmax; tt++ )
		{
			if (  strcmp ( tfile_s , t.muzzlebank_s[tt].Get() ) == 0 ) {  index = g.muzzlebankoffset+tt  ; break; }
		}
	}
	else
	{
		tt=g.muzzlebankmax+1;
	}
	if (  tt>g.muzzlebankmax ) 
	{
		++g.muzzlebankmax;
		t.muzzlebank_s[g.muzzlebankmax]=tfile_s;
		index=g.muzzlebankoffset+g.muzzlebankmax;
		loadinternalimageexcompress(tfile_s,index,5);
	}
	return index;
}

int loadsmoke ( char* tfile_s )
{
	int index = 0;
	int tt = 0;
	index=0;
	if (  g.smokebankmax>0 ) 
	{
		for ( tt = 1 ; tt<=  g.smokebankmax; tt++ )
		{
			if ( strcmp ( tfile_s , t.smokebank_s[tt].Get() ) == 0 ) { index = g.smokebankoffset+tt ; break; }
		}
	}
	else
	{
		tt=g.smokebankmax+1;
	}
	if (  tt>g.smokebankmax ) 
	{
		++g.smokebankmax;
		t.smokebank_s[g.smokebankmax]=tfile_s;
		index=g.smokebankoffset+g.smokebankmax;
		loadinternalimageexcompress(tfile_s,index,5);
	}
	return index;
}


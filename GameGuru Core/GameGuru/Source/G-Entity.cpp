//----------------------------------------------------
//--- GAMEGURU - G-Entity
//----------------------------------------------------

#include "gameguru.h"

// 
//  ENTITY GAME CODE
// 

void entity_init ( void )
{
	//  pre-create element data (load from eleprof)
	timestampactivity(0,"Configure entity instances for use");
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		entity_configueelementforuse ( );
	}

	//  activate all entities and perform any pre-test game setup
	timestampactivity(0,"Configure entity attachments and AI obstacles");
	g.entityattachmentindex=0;
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entid>0 ) 
		{
			//  Activate entity
			t.entityelement[t.e].active=1;
			t.tobj=t.entityelement[t.e].obj;
			if (  t.tobj>0 ) 
			{
				//  if object exists
				if (  ObjectExist(t.tobj) == 1 ) 
				{
					//  Create attachents for entity
					entity_createattachment ( );
					//  Reset AI Obstacle Center references (used later by physics placement)
					t.entityelement[t.e].abscolx_f=-1;
					t.entityelement[t.e].abscolz_f=-1;
					t.entityelement[t.e].abscolradius_f=-1;
					//  Create AI obstacles for all static entities
					if (  t.entityprofile[t.entid].ismarker == 0 ) 
					{
						bool bSceneStatic = false;
						if ( t.entityelement[t.e].staticflag == 1 ) bSceneStatic = true;
						// leelee, sometimes want to make dynamic immobile entities AI obstacles!! (exploding crate in Escape level)
						// if ( t.entityelement[t.e].staticflag == 0 && t.entityelement[t.e].eleprof.isimmobile == 1 ) bSceneStatic = true;
						if ( bSceneStatic == true && t.entityprofile[t.entid].collisionmode != 11 && t.entityprofile[t.entid].collisionmode != 12 ) 
						{
							t.tfullheight=1;
							t.tcontainerid=0;
							if (  t.entityprofile[t.entid].collisionmode >= 50 && t.entityprofile[t.entid].collisionmode<60 ) 
							{
								t.ttreemode=t.entityprofile[t.entid].collisionmode-50;
								//  dont need to setup ai for multiplayer since it doesnt use any ai - unless coop mode!
								if (  t.game.runasmultiplayer == 0 || ( g.steamworks.coop  ==  1 && t.entityprofile[t.entid].ismultiplayercharacter  ==  0 ) ) 
								{
									darkai_setup_tree ( );
								}
							}
							else
							{
								//  dont need to setup ai for multiplayer since it doesnt use any ai
								if (  t.game.runasmultiplayer == 0 || ( g.steamworks.coop  ==  1 && t.entityprofile[t.entid].ismultiplayercharacter  ==  0 ) ) 
								{
									 if ( t.entityprofile[t.entid].collisionmode != 11 && t.entityprofile[t.entid].collisionmode != 12 ) darkai_setup_entity ( );
								}
							}
						}
					}
					//  ensure all transparent static objects are removed from 'intersect all' consideration
					t.tokay=0;
					if (  t.entityelement[t.e].staticflag == 1 ) 
					{
						if (  t.entityprofile[t.entid].canseethrough == 1 ) 
						{
							t.tokay=1;
						}
					}
					if (  t.entityprofile[t.entid].ischaracter == 0 ) 
					{
						if (  t.entityprofile[t.entid].collisionmode == 11  )  t.tokay = 1;
					}
					if (  t.tokay == 1 ) 
					{
						SetObjectCollisionProperty (  t.entityelement[t.e].obj,1 );
					}
					//  ensure all transparency modes set for each entity
					if (  t.entityprofile[t.entid].ismarker == 0 ) 
					{
						int iNeverFive = t.entityelement[t.e].eleprof.transparency;
						if ( iNeverFive == 5 ) iNeverFive = 6;
						SetObjectTransparency (  t.entityelement[t.e].obj, iNeverFive );
					}
					//  update gun/flak settings from latest entity properties
					t.tgunid_s=t.entityprofile[t.entid].isweapon_s;
					entity_getgunidandflakid ( );
					if (  t.tgunid>0 ) 
					{
						for ( int firemode = 0; firemode < 2; firemode++ )
						{
							g.firemodes[t.tgunid][firemode].settings.damage=t.entityelement[t.e].eleprof.damage;
							g.firemodes[t.tgunid][firemode].settings.accuracy=t.entityelement[t.e].eleprof.accuracy;
							g.firemodes[t.tgunid][firemode].settings.reloadqty=t.entityelement[t.e].eleprof.reloadqty;
							g.firemodes[t.tgunid][firemode].settings.iterate=t.entityelement[t.e].eleprof.fireiterations;
							g.firemodes[t.tgunid][firemode].settings.range=t.entityelement[t.e].eleprof.range;
							g.firemodes[t.tgunid][firemode].settings.dropoff=t.entityelement[t.e].eleprof.dropoff;
							g.firemodes[t.tgunid][firemode].settings.usespotlighting=t.entityelement[t.e].eleprof.usespotlighting;
							//  FLAK TODO
						}
					}
				}
			}
		}
	}
}

void entity_bringnewentitiestolife ( void )
{
	//  F9 additions mark new entities with active=2
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid = t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			if ( t.entityprofile[t.entid].isebe == 0 )
			{
				if ( t.entityelement[t.e].active == 2 ) 
				{
					t.tobj = t.entityelement[t.e].obj;
					if ( t.tobj>0 ) 
					{
						//  if object exists
						if ( ObjectExist(t.tobj) == 1 ) 
						{
							// Only redo script for characters (as they were wiped out)
							if ( t.entityprofile[t.entid].ischaracter==1 )
							{
								//  Launch the entity AI
								lua_loadscriptin ( );
								//  Launch init script
								lua_initscript ( );
								//  configure new entity for action
								entity_configueelementforuse ( );
								//  Create attachents for entity
								entity_createattachment ( );
							}
						}
					}
				}
			}
		}
	}

	// 161115 - now restore ALL entities back to actve=1 (now new entities determined)
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			if ( t.entityprofile[t.entid].isebe == 0 )
			{
				if ( t.entityelement[t.e].active == 2 || t.entityelement[t.e].active == 3 || t.entityelement[t.e].active == 4 ) 
				{
					// show attachment
					if (  t.entityelement[t.e].attachmentobj>0 ) 
					{
						if (  ObjectExist(t.entityelement[t.e].attachmentobj) == 1 ) 
						{
							ShowObject (  t.entityelement[t.e].attachmentobj );
						}
					}

					// 171115 - only [3 with character] requires characters to be restored to clones
					if (  t.entityelement[t.e].active == 3 ) 
					{
						t.tte = t.e; 
						entity_converttoclone ( );
					}

					// restore active flag for in-game use
					t.entityelement[t.e].active = 1;
				}
			}
		}
	}
}

void entity_initafterphysics ( void )
{
	//  Handle spawn entities
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entid>0 ) 
		{
			if (  t.entityelement[t.e].eleprof.spawnatstart == 0 ) 
			{
				t.entityelement[t.e].active=0;
				entity_lua_hide ( );
				entity_lua_collisionoff ( );
			}
			else
			{
				entity_lua_collisionon ( );
			}
		}
	}
}

void entity_refreshelementforuse ( void )
{
	// reset entity flags when entity is new (renewed, i.e. load game)

	// internal entity speed modulator defaults at 1.0
	t.entityelement[t.e].speedmodulator_f=1.0;

	// reset explosion state
	t.entityelement[t.e].explodefusetime=0;

	// reset ragdoll state
	t.entityelement[t.e].ragdollified=0;
}

void entity_configueelementforuse ( void )
{
	//  Spawn values
	t.entityelement[t.e].spawn.atstart=t.entityelement[t.e].eleprof.spawnatstart;
	t.entityelement[t.e].spawn.max=t.entityelement[t.e].eleprof.spawnmax;
	t.entityelement[t.e].spawn.delay=t.entityelement[t.e].eleprof.spawndelay;
	t.entityelement[t.e].spawn.qty=t.entityelement[t.e].eleprof.spawnqty;
	t.entityelement[t.e].spawn.upto=t.entityelement[t.e].eleprof.spawnupto;
	t.entityelement[t.e].spawn.afterdelay=t.entityelement[t.e].eleprof.spawnafterdelay;
	t.entityelement[t.e].spawn.whendead=t.entityelement[t.e].eleprof.spawnwhendead;
	t.entityelement[t.e].spawn.delayrandom=t.entityelement[t.e].eleprof.spawndelayrandom;
	t.entityelement[t.e].spawn.qtyrandom=t.entityelement[t.e].eleprof.spawnqtyrandom;
	t.entityelement[t.e].spawn.vel=t.entityelement[t.e].eleprof.spawnvel;
	t.entityelement[t.e].spawn.velrandom=t.entityelement[t.e].eleprof.spawnvelrandom;
	t.entityelement[t.e].spawn.angle=t.entityelement[t.e].eleprof.spawnangle;
	t.entityelement[t.e].spawn.anglerandom=t.entityelement[t.e].eleprof.spawnanglerandom;
	t.entityelement[t.e].spawn.life=t.entityelement[t.e].eleprof.spawnlife;
	if (  t.entityelement[t.e].spawn.atstart == 0 && t.entityelement[t.e].spawn.max == 0 ) 
	{
		t.entityelement[t.e].spawn.max=1;
		if (  t.entityelement[t.e].spawn.afterdelay == 0 && t.entityelement[t.e].spawn.whendead == 0 ) 
		{
			t.entityelement[t.e].spawn.afterdelay=1;
		}
		if (  t.entityelement[t.e].spawn.qty == 0  )  t.entityelement[t.e].spawn.qty = 1;
		if (  t.entityelement[t.e].spawn.upto == 0  )  t.entityelement[t.e].spawn.upto = 1;
	}

	//  Configure health from strength
	if (  t.entityelement[t.e].eleprof.strength>0 ) 
	{
		t.entityelement[t.e].health=t.entityelement[t.e].eleprof.strength;
	}
	else
	{
		t.entityelement[t.e].health=1;
	}

	//  Resolve default weapon gun ids
	if (  t.entityelement[t.e].eleprof.hasweapon_s != "" ) 
	{
		t.findgun_s = Lower( t.entityelement[t.e].eleprof.hasweapon_s.Get() ) ; 
		gun_findweaponindexbyname ( );
		t.entityelement[t.e].eleprof.hasweapon=t.foundgunid;
		if (  t.foundgunid>0 && t.entityprofile[t.entid].isammo == 0  )  t.gun[t.foundgunid].activeingame = 1;
	}
	else
	{
		t.entityelement[t.e].eleprof.hasweapon=0;
	}

	// Reset general flags when entity is newified
	entity_refreshelementforuse();
}

void entity_freeragdoll ( void )
{
	if (  t.entityelement[t.tte].ragdollified == 1 ) 
	{
		t.tphyobj=t.entityelement[t.tte].obj ; ragdoll_destroy ( );
		t.entityelement[t.tte].ragdollified=0;
	}
}

void entity_free ( void )
{
	// close down game entities
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		t.obj=t.entityelement[t.e].obj;
		if (  t.entid>0 && t.obj>0 ) 
		{
			if ( ObjectExist(t.obj) == 1 ) 
			{
				// ensure all ragdolls/dynamic entities are restored
				if ( t.entityprofile[t.entid].ismarker == 0 && t.entityprofile[t.entid].isebe == 0 ) 
				{
					t.tte = t.e ; entity_converttoinstance ( );
					PositionObject (  t.obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
					RotateObject (  t.obj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
					t.tentid=t.entid ; t.tte=t.e ; t.tobj=t.obj ; entity_resettodefaultanimation ( );
					ShowObject (  t.obj );
				}
			}
			entity_freeattachment ( );
			t.entityelement[t.e].active=0;
		}
		// release clonse sounds before leave
		if ( t.entityelement[t.e].soundset>0  )  deleteinternalsound(t.entityelement[t.e].soundset);
		if ( t.entityelement[t.e].soundset<0  )  DeleteAnimation (  abs(t.entityelement[t.e].soundset) );
		t.entityelement[t.e].soundset=0;
		if ( t.entityelement[t.e].soundset1>0  )  deleteinternalsound(t.entityelement[t.e].soundset1);
		if ( t.entityelement[t.e].soundset1<0  )  DeleteAnimation (  abs(t.entityelement[t.e].soundset1) );
		t.entityelement[t.e].soundset1=0;
		if ( t.entityelement[t.e].soundset2>0  )  deleteinternalsound(t.entityelement[t.e].soundset2);
		t.entityelement[t.e].soundset2=0;
		if ( t.entityelement[t.e].soundset3>0  )  deleteinternalsound(t.entityelement[t.e].soundset3);
		t.entityelement[t.e].soundset3=0;
		if ( t.entityelement[t.e].soundset4>0  )  deleteinternalsound(t.entityelement[t.e].soundset4);
		t.entityelement[t.e].soundset4=0;
	}
}

void entity_delete ( void )
{
	//  delete all entities
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if (  t.e <= ArrayCount(t.entityelement) ) 
		{
			// find and delete entity element obj
			t.obj=t.entityelement[t.e].obj;
			if (  t.obj>0 ) 
			{
				if (  ObjectExist(t.obj) == 1 ) 
				{
					DeleteObject (  t.obj );
				}
			}

			//  delete character creator object
			t.ccobjToDelete = t.obj;
			characterkit_deleteEntity ( );
			t.entityelement[t.e].bankindex=0;
			t.entityelement[t.e].obj=0;
		}
	}
}

void entity_pauseanimations ( void )
{
	Dim (  t.storeanimspeeds,g.entityelementlist  );
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.obj=t.entityelement[t.e].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1 ) 
			{
				t.storeanimspeeds[t.e]=GetSpeed(t.obj);
				SetObjectSpeed (  t.obj,0 );
			}
		}
	}
}

void entity_resumeanimations ( void )
{
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.obj=t.entityelement[t.e].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1 ) 
			{
				SetObjectSpeed (  t.obj,t.storeanimspeeds[t.e] );
			}
		}
	}
	UnDim (  t.storeanimspeeds );
}

void entity_loop ( void )
{
	//  Handle all entities in level
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		// 011016 - scenes with LARGE number of static entities hitting perf hard
		if ( t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.phyalways == 0 ) continue;
		// NOTE: Determine essential tasks static needs (i.e. plrdist??)

		// only handle DYNAMIC entities
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			//  Entity object
			t.tobj=t.entityelement[t.e].obj;

			//  Entity Prompt Local
			if ( t.entityelement[t.e].overprompttimer>0 ) 
			{
				if ( ObjectExist(t.tobj) == 1 ) 
				{
					if (  Timer()>(int)t.entityelement[t.e].overprompttimer ) 
					{
						t.entityelement[t.e].overprompttimer=0;
					}
					else
					{
						if (  GetInScreen(t.tobj) == 1 ) 
						{
							t.t_s=t.entityelement[t.e].overprompt_s ; t.twidth=getbitmapfontwidth(t.t_s.Get(),1)/2;
							pastebitmapfont(t.t_s.Get(),GetScreenX(t.tobj)-t.twidth,GetScreenY(t.tobj),1,255);
						}
					}
				}
			}

			// if ragdoll and has force, apply it repeatedly
			if ( t.tobj>0 ) 
			{
				if ( t.entityelement[t.e].ragdollified == 1 && t.entityelement[t.e].ragdollifiedforcevalue_f>1.0 ) 
				{
					BPhys_RagDollApplyForce (  t.tobj,t.entityelement[t.e].ragdollifiedforcelimb,0,0,0,t.entityelement[t.e].ragdollifiedforcex_f,t.entityelement[t.e].ragdollifiedforcey_f,t.entityelement[t.e].ragdollifiedforcez_f,t.entityelement[t.e].ragdollifiedforcevalue_f );
					t.entityelement[t.e].ragdollifiedforcevalue_f=t.entityelement[t.e].ragdollifiedforcevalue_f*0.75;
					if ( t.entityelement[t.e].ragdollifiedforcevalue_f <= 1.0 ) 
					{
						t.entityelement[t.e].ragdollifiedforcevalue_f=0;
					}
				}
			}

			//  obtain distance from camera/player
			entity_controlrecalcdist ( );
			if ( abs(t.entityelement[t.e].plrdist-t.dist_f)>10 ) 
			{
				t.entityelement[t.e].lua.flagschanged=1;
			}
			t.entityelement[t.e].plrdist=t.dist_f;

			// control immunity for entities
			if ( t.entityelement[t.e].briefimmunity > 0 )
			{
				t.entityelement[t.e].briefimmunity--;
			}

			// in all active states, must repell player to avoid penetration
			if ( t.tobj > 0 ) 
			{
				if ( t.entityprofile[t.entid].ischaracter == 1 || t.entityprofile[t.entid].collisionmode == 21 ) 
				{
					if ( t.entityelement[t.e].health>0 && t.entityelement[t.e].usingphysicsnow == 1 ) 
					{
						bool bThirdPersonPlayer = false;
						if ( t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.charactere == t.e ) bThirdPersonPlayer = true;
						if ( bThirdPersonPlayer == false )
						{
							t.tplrproxx_f=ObjectPositionX(t.aisystem.objectstartindex)-ObjectPositionX(t.tobj);
							t.tplrproyy_f=ObjectPositionY(t.aisystem.objectstartindex)-ObjectPositionY(t.tobj);
							t.tplrproxz_f=ObjectPositionZ(t.aisystem.objectstartindex)-ObjectPositionZ(t.tobj);
							t.tplrproxd_f=Sqrt(abs(t.tplrproxx_f*t.tplrproxx_f)+abs(t.tplrproyy_f*t.tplrproyy_f)+abs(t.tplrproxz_f*t.tplrproxz_f));
							t.tplrproxa_f=atan2deg(t.tplrproxx_f,t.tplrproxz_f);
							if (  t.tplrproxd_f<t.entityprofile[t.entid].fatness ) 
							{
								t.playercontrol.pushforce_f=1.0;
								t.playercontrol.pushangle_f=t.tplrproxa_f;
							}
						}
					}
				}
			}

			// Handle when entity limb flinch hurt system
			if ( t.entityelement[t.e].limbhurt>0 && t.entityelement[t.e].health>0 ) 
			{
				//  known limbs
				t.headlimbofcharacter=t.entityprofile[t.entityelement[t.e].bankindex].headlimb;
				t.spine2limbofcharacter=t.entityprofile[t.entityelement[t.e].bankindex].spine2;
				//  determine which segment the limb belongs
				t.tsegmenttoflinch=0;
				if (  t.entityelement[t.e].limbhurt == t.headlimbofcharacter  )  t.tsegmenttoflinch = 1;
				//  degrade flinch value until finished
				t.tsmoothspeed_f=3.0/g.timeelapsed_f;
				t.entityelement[t.e].limbhurta_f=CurveValue(0,t.entityelement[t.e].limbhurta_f,t.tsmoothspeed_f);
				if (  abs(t.entityelement[t.e].limbhurta_f)<1.0 ) 
				{
					t.entityelement[t.e].limbhurta_f=0;
					t.entityelement[t.e].limbhurt=0;
				}
				//  modify character limbs based on segment hurt
				if (  t.tobj>0 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						if (  t.tsegmenttoflinch == 0 ) 
						{
							if (  t.spine2limbofcharacter>0 ) 
							{
								if (  LimbExist(t.tobj,t.spine2limbofcharacter) == 1 ) 
								{
									RotateLimb (  t.tobj,t.spine2limbofcharacter,t.entityelement[t.e].limbhurta_f/3.0,t.entityelement[t.e].limbhurta_f*-1,0 );
								}
							}
						}
						if (  t.tsegmenttoflinch == 1 ) 
						{
							if (  t.headlimbofcharacter>0 ) 
							{
								if (  LimbExist(t.tobj,t.headlimbofcharacter) == 1 ) 
								{
									RotateLimb (  t.tobj,t.headlimbofcharacter,t.entityelement[t.e].limbhurta_f,LimbAngleY(t.tobj,t.headlimbofcharacter),LimbAngleZ(t.tobj,t.headlimbofcharacter) );
								}
							}
						}
					}
				}
			}

			// if entity using non-3d sound, needs to update based on camera position
			// (can also be used for moving entities that LoopSound ( later) )
			if ( t.entityelement[t.e].soundisnonthreedee == 1 ) 
			{
				t.entityelement[t.e].soundisnonthreedee=0;
				if ( t.entityelement[t.e].soundset>0 ) 
				{
					PositionSound (  t.entityelement[t.e].soundset,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
					if ( SoundPlaying(t.entityelement[t.e].soundset) == 1 ) 
					{
						t.entityelement[t.e].soundisnonthreedee=1;
					}
				}
				if ( t.entityelement[t.e].soundset1>0 ) 
				{
					PositionSound (  t.entityelement[t.e].soundset1,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
					if (  SoundPlaying(t.entityelement[t.e].soundset1) == 1 ) 
					{
						t.entityelement[t.e].soundisnonthreedee=1;
					}
				}
				if (  t.entityelement[t.e].soundset2>0 ) 
				{
					PositionSound (  t.entityelement[t.e].soundset2,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
					if (  SoundPlaying(t.entityelement[t.e].soundset2) == 1 ) 
					{
						t.entityelement[t.e].soundisnonthreedee=1;
					}
				}
				if (  t.entityelement[t.e].soundset3>0 ) 
				{
					PositionSound (  t.entityelement[t.e].soundset3,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
					if (  SoundPlaying(t.entityelement[t.e].soundset3) == 1 ) 
					{
						t.entityelement[t.e].soundisnonthreedee=1;
					}
				}
				if (  t.entityelement[t.e].soundset4>0 ) 
				{
					PositionSound (  t.entityelement[t.e].soundset4,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
					if (  SoundPlaying(t.entityelement[t.e].soundset4) == 1 ) 
					{
						t.entityelement[t.e].soundisnonthreedee=1;
					}
				}
			}

			// character creator object
			if ( t.entityprofile[t.entid].ischaractercreator == 1 ) 
			{
				t.tccobj = g.charactercreatorrmodelsoffset+((t.e*3)-t.characterkitcontrol.offset);
				if ( ObjectExist(t.tccobj) == 1 ) 
				{
					// only glue head if enemy is visible
					t.tconstantlygluehead=0;
					if ( t.tobj>0 ) {  if ( GetVisible(t.tobj)==1 ) { t.tconstantlygluehead=1; } } 
					if (  t.game.runasmultiplayer == 1 ) 
					{
						//  deal with multiplayer issues
						//  if (  its me,  )  only show me when im dead
						if (  t.characterkitcontrol.showmyhead == 1 && t.e == t.steamworks_playerEntityID[g.steamworks.me] ) 
						{
							t.tconstantlygluehead=1;
						}
						//  if other players are dead and transitioning to a new spawn postion
						for ( t.ttemploop = 0 ; t.ttemploop<=  STEAM_MAX_NUMBER_OF_PLAYERS; t.ttemploop++ )
						{
							if (  t.ttemploop != g.steamworks.me ) 
							{
								if (  t.e == t.steamworks_playerEntityID[t.ttemploop] && t.steamworks_forcePosition[t.ttemploop]>0 && SteamGetPlayerAlive(t.ttemploop) == 1 ) 
								{
									t.tconstantlygluehead=0;
								}
							}
						}
					}
					//  if head is flagged to by glued, attach to body now
					if (  t.tconstantlygluehead == 1 ) 
					{
						//  NOTE; re-searching for head limb is a performance hit
						t.tSourcebip01_head=getlimbbyname(t.entityelement[t.e].obj, "Bip01_Head");
						if (  t.tSourcebip01_head>0 ) 
						{
							//Dave - fix to heads being backwards for characters when switched off (3000 units away)
							float tdx = CameraPositionX(0) - ObjectPositionX(t.entityelement[t.e].obj);
							float tdy = CameraPositionY(0) - ObjectPositionY(t.entityelement[t.e].obj);
							float tdz = CameraPositionZ(0) - ObjectPositionZ(t.entityelement[t.e].obj);
							float tdist = sqrt ( tdx*tdx + tdy*tdy + tdz*tdz );
							t.te = t.e; entity_getmaxfreezedistance ( );
							if ( tdist > t.maximumnonefreezedistance )
							{
								YRotateObject (  t.tccobj, ObjectAngleY(t.entityelement[t.e].obj)-180 );
							}
							else
							{
								YRotateObject (  t.tccobj, 0 );								
							}							
							GlueObjectToLimbEx (  t.tccobj,t.entityelement[t.e].obj,t.tSourcebip01_head,2 );
						}
					}
					else
					{
						//  else unglue and hide the head
						UnGlueObject (  t.tccobj );
						PositionObject (  t.tccobj,100000,100000,100000 );
					}
				}
			}

			// flag to destroy entity dead (can be set from LUA command or explosion trigger)
			if ( t.entityelement[t.e].destroyme == 1 ) 
			{
				// remove entity from game play
				t.entityelement[t.e].destroyme = 0;
				t.entityelement[t.e].active = 0;
				t.entityelement[t.e].health = 0;
				t.entityelement[t.e].lua.flagschanged = 2;
				if ( t.game.runasmultiplayer == 1 ) 
				{
					steam_addDestroyedObject ( );
				}
				t.obj=t.entityelement[t.e].obj;
				if (  t.obj>0 ) 
				{
					if (  ObjectExist(t.obj) == 1 ) 
					{
						HideObject (  t.obj );
					}
				}

				//  attempt to remove collision object
				entity_lua_collisionoff ( );

				//  possible remove character
				entity_lua_findcharanimstate ( );
				if (  t.tcharanimindex != -1 ) 
				{
					//  deactivate DarkA.I for this dead entity
					darkai_killai ( );

					//  Convert object back to instance and hide it
					darkai_character_remove ( );
					t.charanimstates[t.tcharanimindex] = t.charanimstate;
				}
				else
				{
					//  can still have non-character ragdoll (zombie), so remove ragdoll if so
					t.tphyobj=t.obj ; ragdoll_destroy ( );
				}
			}
		}
	}

	//  handle explosion triggers in separate loop as they call
	//  other subroutines that use E and other entity calls (i.e. physics_explodesphere)
	for ( t.ee = 1 ; t.ee <= g.entityelementlist; t.ee++ )
	{
		t.eentid=t.entityelement[t.ee].bankindex;
		if ( t.eentid>0 ) 
		{
			//  flag to explode entity after a fused amount of time
			if ( t.entityelement[t.ee].explodefusetime>0 ) 
			{
				if ( Timer()>t.entityelement[t.ee].explodefusetime ) 
				{
					t.entityelement[t.ee].explodefusetime = -1;
					// explode from beneath
					t.tdamage=t.entityelement[t.ee].eleprof.explodedamage;
					t.tdamageforce=0;
					t.brayx1_f=ObjectPositionX(t.entityelement[t.ee].obj)+GetObjectCollisionCenterX(t.entityelement[t.ee].obj);
					t.brayy1_f=(ObjectPositionY(t.entityelement[t.ee].obj)+GetObjectCollisionCenterY(t.entityelement[t.ee].obj))-100;
					t.brayz1_f=ObjectPositionZ(t.entityelement[t.ee].obj)+GetObjectCollisionCenterZ(t.entityelement[t.ee].obj);
					t.brayx2_f=t.brayx1_f;
					t.brayy2_f=(t.brayy1_f+100);
					t.brayz2_f=t.brayz1_f;
					t.tdamagesource=0;
					t.ttte = t.ee ; entity_applydamage() ; t.ee=t.ttte;
					// create a huge bang
					t.entityelement[t.ee].destroyme=1;
					t.tProjectileResult = WEAPON_PROJECTILERESULT_EXPLODE;
					t.tx_f=t.entityelement[t.ee].x ; t.ty_f=t.entityelement[t.ee].y ; t.tz_f=t.entityelement[t.ee].z;
					t.tDamage_f = t.entityelement[t.ee].eleprof.explodedamage; 
					t.tradius_f = 300;
					t.tSourceEntity = t.ee;
					// provide the explosion sound (as it cannot come from projectile)
					t.tSoundID=0;
					for ( t.tscanexp = 0 ; t.tscanexp<=  4; t.tscanexp++ )
					{
						if ( SoundExist(g.explodesoundoffset+t.tscanexp) == 1 ) 
						{
							if ( SoundPlaying(g.explodesoundoffset+t.tscanexp) == 0 ) 
							{
								t.tSoundID=g.explodesoundoffset+t.tscanexp;
							}
						}
					}
					weapon_projectileresult_make ( );
				}
			}
		}
	}
}

void entity_loopanim ( void )
{
	// In game or editor, must control entity animation speed (machine indie)#
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		// 011016 - scenes with LARGE number of static entities hitting perf hard
		if ( t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.phyalways == 0 ) continue;
		// NOTE: Determine essential tasks static needs (i.e. plrdist??)

		// only handle DYNAMIC entities 
		t.entid=t.entityelement[t.e].bankindex;
		if ( t.entid>0 ) 
		{
			t.tparentobj=g.entitybankoffset+t.entid;
			if ( t.entityprofile[t.entid].ischaracter == 0 ) 
			{
				// but not for characters which have their own speed control
				t.tobj=t.entityelement[t.e].obj;
				if ( t.tobj>0 ) 
				{
					if ( ObjectExist(t.tobj) == 1 && ObjectExist(t.tparentobj) == 1 ) 
					{
						if ( GetNumberOfFrames(t.tparentobj)>0 ) 
						{
							// 120417 - now modulate anim speed with script controlled modulation
							float fFinalAnimSpeed = t.entityelement[t.e].eleprof.animspeed * t.entityelement[t.e].animspeedmod;

							// Detect if entity instance speed different from base default, and if so, need CLONE!
							if ( t.entityelement[t.e].isclone == 0 ) 
							{
								if ( fFinalAnimSpeed != t.entityprofile[t.entid].animspeed ) 
								{
									// Entity must be unique to allow different speed from parent
									t.tte = t.e ; entity_converttoclone ( );
								}
							}
							if ( t.entityelement[t.e].isclone == 1 ) 
							{
								// Control animation speed of cloned object
								t.tanimspeed_f = fFinalAnimSpeed * t.entityelement[t.e].speedmodulator_f;
								SetObjectSpeed ( t.tobj,g.timeelapsed_f*t.tanimspeed_f );
							}
							else
							{
								//  Control animation speed of parent object associated with instance
								t.tanimspeed_f = t.entityprofile[t.entid].animspeed;
								if (  ObjectExist(t.tparentobj) == 1  ) SetObjectSpeed ( t.tparentobj,g.timeelapsed_f*t.tanimspeed_f );
							}
							//  if animation in progress (handle any transitioning)
							if ( t.entityelement[t.e].lua.animating == 1 ) 
							{
								smoothanimupdate(t.tobj);
							}
						}
					}
				}
			}

			// also handle entity footfall sounds from loopanim
			int iFootFallMax = t.entityprofile[t.entid].footfallmax;
			if ( iFootFallMax > 0 )
			{
				t.tobj = t.entityelement[t.e].obj;
				if ( t.tobj>0 ) 
				{
					for ( int iFootFallIndex = 0; iFootFallIndex < iFootFallMax; iFootFallIndex++ )
					{
						float fCurrentFrame = GetFrame(t.tobj);
						float fDistanceFromFrame = fabs(fCurrentFrame-t.entityfootfall[t.entid][iFootFallIndex].keyframe);
						if ( fCurrentFrame >= t.entityfootfall[t.entid][iFootFallIndex].keyframe && fDistanceFromFrame < 5.0f && iFootFallIndex != t.entityelement[t.e].lastfootfallframeindex )
						{
							// ensure this footfall frame not triggered again until another one gets triggered
							t.entityelement[t.e].lastfootfallframeindex = iFootFallIndex;

							// choose footfall sound for character
							int iFootFallSoundType = t.entityfootfall[t.entid][iFootFallIndex].soundtype;
							int iFootFallType = -1;
							if ( 1 ) // character gravity active
							{
								// above or below water line
								if ( t.entityelement[t.e].y > t.terrain.waterliney_f+36 || t.hardwareinfoglobals.nowater != 0 ) 
								{
									//  takes physcs hit material from environment
									if ( iFootFallSoundType == 0 ) 
									{
										// this is gathered from capsule contact with non-mesh physics geometry
										// that has had its attribute value set (inside m_uerPointer)
										iFootFallType = ODEGetBodyAttribValue ( t.tobj );
									}
									else
									{
										iFootFallType = 0;
									}
								}
								else
								{
									if ( t.entityelement[t.e].y > t.terrain.waterliney_f-33 ) 
									{
										// Footfall water wading sound
										iFootFallType = 17;

										/* characters to create ripples
										//  And also trigger ripples in water
										t.playercontrol.ripplecount_f=t.playercontrol.ripplecount_f+g.timeelapsed_f;
										if (  t.playercontrol.movement != 0  )  t.playercontrol.ripplecount_f = t.playercontrol.ripplecount_f+(g.timeelapsed_f*3);
										if (  t.playercontrol.ripplecount_f>20.0 ) 
										{
											t.playercontrol.ripplecount_f=0.0;
											g.decalx=ObjectPositionX(t.aisystem.objectstartindex);
											g.decaly=t.terrain.waterliney_f+1.0;
											g.decalz=ObjectPositionZ(t.aisystem.objectstartindex);
											decal_triggerwaterripple ( );
										}
										*/
									}
									else
									{
										// underwater sound for character
										iFootFallType = 18;
									}
								}
							}

							//  Manage trigger of footfall sound effects
							if ( iFootFallType != -1 ) 
							{
								if ( 0 ) // no footfalls if not moving or jumping
								{
									// no footfalls here
								}
								else
								{
									// play footfall sound effect at character position
									sound_footfallsound ( iFootFallType, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z, &t.entityelement[t.e].lastfootfallsound );
								}
							}
						}
					}
				}
			}
		}
	}
}

void entity_controlrecalcdist ( void )
{
	//  Distance between player camera and entity
	t.tobj=t.entityelement[t.e].obj;
	if (  t.tobj>0 && (t.entityelement[t.e].active != 0 || t.entityelement[t.e].eleprof.phyalways != 0) ) 
	{
		if (  ObjectExist(t.tobj)  ==  1 ) 
		{
			t.distx_f=CameraPositionX(0)-ObjectPositionX(t.tobj);
			t.disty_f=CameraPositionY(0)-ObjectPositionY(t.tobj);
			t.distz_f=CameraPositionZ(0)-ObjectPositionZ(t.tobj);
			t.dist_f=Sqrt(abs(t.distx_f*t.distx_f)+abs(t.disty_f*t.disty_f)+abs(t.distz_f*t.distz_f));
			t.diffangle_f=atan2deg(t.distx_f,t.distz_f);
			if (  t.diffangle_f<0  )  t.diffangle_f = t.diffangle_f+360;
		}
		else
		{
			t.dist_f=9999999;
		}
	}
	else
	{
		t.dist_f=9999999;
	}
return;

}

void entity_getmaxfreezedistance ( void )
{
	if (  t.entityelement[t.te].eleprof.phyalways != 0 ) 
	{
		//  always active characters NEVER freeze at distance
		t.maximumnonefreezedistance=MAXNEVERFREEZEDISTANCE;
	}
	else
	{
		//  distance at which logic is frozen
		t.maximumnonefreezedistance=MAXFREEZEDISTANCE;
	}
	//  AI that is attacking another player counts as on
	if (  t.game.runasmultiplayer  ==  1 ) 
	{
		if (  t.entityelement[t.te].mp_updateOn  ==  1  )  t.maximumnonefreezedistance = MAXNEVERFREEZEDISTANCE;
	}
}

void entity_updatepos ( void )
{
	// takes te - but not tv# as already dealth with when moved entity X Y Z
	t.tobj=t.entityelement[t.te].obj;

	//  move entity using physics
	if ( t.entityelement[t.te].usingphysicsnow == 1 ) 
	{
		//  control physics object (entity-driven)
		t.tvx_f=t.entityelement[t.te].x-ObjectPositionX(t.tobj);
		t.tvz_f=t.entityelement[t.te].z-ObjectPositionZ(t.tobj);
		t.tvd_f=Sqrt(abs(t.tvx_f*t.tvx_f)+abs(t.tvz_f*t.tvz_f));

		//  ensure it is capped for max physics object movement
		if ( t.tvd_f > 0.0f )
		{
			t.tvx_f=t.tvx_f/t.tvd_f;
			t.tvz_f=t.tvz_f/t.tvd_f;
		}
		if ( t.tvd_f>25.0  )  t.tvd_f = 25.0;
		t.tvx_f=t.tvx_f*t.tvd_f;
		t.tvz_f=t.tvz_f*t.tvd_f;
		t.tvx_f=t.tvx_f*15.0;
		t.tvz_f=t.tvz_f*15.0;

		// entity speed modifier
		t.tentspeed_f=(t.entityelement[t.te].eleprof.speed+0.0)/100.0;
		t.tentinvspeed_f=100.0/(t.entityelement[t.te].eleprof.speed+0.0);

		// special method for characters to climb 'stairs/ramps'
		if ( t.entityelement[t.te].climbgravity > 0.0f )
		{
			// find ground Y of XZ position, then apply physics velocity to get object to that location
			bool bClimbNeeded = true;
			float fStepUp = 20.0f;
			float fStepDown = 5.0f;
			// advance 'step scan' a little so can anticipate a staircase (stuff to climb)
			float fAdvanceX = NewXValue ( 0, t.entityelement[t.te].ry, 12.0f );
			float fAdvanceZ = NewZValue ( 0, t.entityelement[t.te].ry, 12.0f );
			int iCollisionMode = (1<<(0)) | (1<<(1)); //COL_TERRAIN | COL_OBJECT;
			if ( ODERayTerrainEx ( t.entityelement[t.te].x + fAdvanceX, t.entityelement[t.te].y + fStepUp, t.entityelement[t.te].z + fAdvanceZ, t.entityelement[t.te].x + fAdvanceX, t.entityelement[t.te].y - fStepDown, t.entityelement[t.te].z + fAdvanceZ, iCollisionMode ) == 1 )
			{
				t.entityelement[t.te].y = ODEGetRayCollisionY();
			}
			else
			{
				// detected no floor in step down test, revert to gravity eventually!
				bClimbNeeded = false;
			}
			float fRiseY = t.entityelement[t.te].y - ObjectPositionY(t.tobj);

			// rise AI to climb stairs/ramp
			if ( fRiseY > 1.5f ) 
			{
				// need substantial upward rise to keep climbing to avoid setting stuck on impassable obstructions!
				t.entityelement[t.te].climbgravity = 1.0f;
			}
			else
			{
				// if not climbing, and up against resistence, stop climb mode (eventually)
				bClimbNeeded = false;
			}
			if ( fRiseY > 0.001f )
			{
				// climbing phyics
				fRiseY=fRiseY*25.0f;
				ODESetLinearVelocity ( t.tobj, t.tvx_f, fRiseY, t.tvz_f );
			}
			if ( bClimbNeeded == false )
			{
				// allows for subtle flat spots during climb, but deactivates if no upward movement after a while
				//t.entityelement[t.te].climbgravity -= 0.05f; //100417 - many enemies and this takes AGES to get to zero, so AI keeps reseting with no physics movement (just slow grav countdown and lots of XZ math movement!)
				t.entityelement[t.te].climbgravity -= g.timeelapsed_f * 2.0f;
				if ( t.entityelement[t.te].climbgravity < 0.0f ) 
					t.entityelement[t.te].climbgravity = 0.0f;
			}
		}
		if ( t.entityelement[t.te].climbgravity == 0.0f )
		{
			// accelerate physics to reach entity X Y Z quickly
			int entid = t.entityelement[t.te].bankindex;
			//if ( t.tvd_f>24.0*t.tentspeed_f ) // 080317 - very fast AI movement gets capped so never exceeds speed modifier result
			if ( t.tvd_f*t.tentspeed_f > 24.0*t.tentspeed_f ) 
			{
				// no velocity while we adjst dest to current stood location (possible vibrate fix)
				ODESetLinearVelocity ( t.tobj,0,0,0 );

				// ensure entity does not get too far from physics object
				t.entityelement[t.te].x=ObjectPositionX(t.tobj);
				t.entityelement[t.te].z=ObjectPositionZ(t.tobj);

				// if AI bot, must reset its position to last valid pos (where phyiscs object is)
				if ( g.charanimindex > 0 )
				{
					// otherwise it separates from its object terribly FAR!
					AISetEntityPosition ( t.tobj,t.entityelement[t.te].x,t.entityelement[t.te].y,t.entityelement[t.te].z );
				}

				// if character finds themselves stopped by something, they can try a climb state
				if ( t.entityprofile[entid].ischaracter == 1 )
				{
					t.entityelement[t.te].climbgravity = 1.0f;
				}

				// in all cases, register dynamic obstruction with LUA var
				if ( t.entityelement[t.te].lua.dynamicavoidance == 0 )
				{
					t.entityelement[t.te].lua.dynamicavoidancestuckclock += 0.5f;
					if ( t.entityelement[t.te].lua.dynamicavoidancestuckclock > 1.0f )
					{
						t.entityelement[t.te].lua.dynamicavoidance = 1;
					}
				}
				t.entityelement[t.te].lua.flagschanged=1;
			}
			else
			{
				// apply normal velocity for entity movement (but gravity force for characters)
				if ( t.entityprofile[entid].ischaracter == 1 )
				{
					// 100317 - special hover mode tracks surface below entity using raycast on physics
					float fHoverFactor = t.entityprofile[entid].hoverfactor;
					if ( fHoverFactor > 0.0f )
					{
						int iCollisionMode = (1<<(0)) | (1<<(1)); //COL_TERRAIN | COL_OBJECT;
						if ( ODERayTerrainEx ( t.entityelement[t.te].x, t.entityelement[t.te].y + 75.0f, t.entityelement[t.te].z, t.entityelement[t.te].x, t.entityelement[t.te].y - 75.0f, t.entityelement[t.te].z, iCollisionMode ) == 1 )
						{
							t.entityelement[t.te].y = ODEGetRayCollisionY();
							float tvy = (t.entityelement[t.te].y + fHoverFactor) - ObjectPositionY(t.tobj);
							ODESetLinearVelocity ( t.tobj, t.tvx_f, tvy*2.0f, t.tvz_f );
						}
						else
						{
							ODESetLinearVelocityXZWithGravity ( t.tobj, t.tvx_f, t.tvz_f, t.tvgravity_f/5.0f );
						}
					}
					else
					{
						ODESetLinearVelocityXZWithGravity ( t.tobj, t.tvx_f, t.tvz_f, t.tvgravity_f );
					}
				}
				else
				{
					ODESetLinearVelocity ( t.tobj,t.tvx_f,t.tvgravity_f*5.0,t.tvz_f );
				}
			}
		}
	}
	else
	{
		if (  t.entityelement[t.te].nogravity == 0 ) 
		{
			if (  t.terrain.TerrainID>0 ) 
			{
				// work out Floor (  position )
				t.tterrainfloorposy_f=BT_GetGroundHeight(t.terrain.TerrainID,ObjectPositionX(t.tobj),ObjectPositionZ(t.tobj));
			}
			else
			{
				t.tterrainfloorposy_f=1000.0;
			}
			if (  t.entityelement[t.te].y<t.tterrainfloorposy_f ) 
			{
				t.entityelement[t.te].y=t.tterrainfloorposy_f;
			}
		}
		PositionObject (  t.tobj,t.entityelement[t.te].x,t.entityelement[t.te].y,t.entityelement[t.te].z );
	}
}

void entity_determinedamagemultiplier ( void )
{
	t.tdamagemultiplier_f=1.0f;
	/* 190316 - removed head damage hard coding, now in scripts
	//  Check if limb is in any headlimb range (takes ttentid,bulletraylimbhit,tobj)
	if (  t.bulletraylimbhit>0 ) 
	{
		if (  t.ttentid>0 ) 
		{
			if (  t.bulletraylimbhit >= t.entityprofile[t.ttentid].headframestart && t.bulletraylimbhit <= t.entityprofile[t.ttentid].headframefinish ) 
			{
				if (  t.tobj>0 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						if (  t.entityelement[t.ttte].health>0 ) 
						{
							//  work out if object has limbs
							PerformCheckListForLimbs (  t.tobj );
							t.tlimbmax=ChecklistQuantity()-1;
							//  small visual indicator the headshot worked!
							//  but not if a character creator entity
							if (  t.entityprofile[t.ttentid].ischaractercreator  ==  0 ) 
							{
								for ( t.tlmb = t.entityprofile[t.ttentid].headframestart ; t.tlmb<=  t.entityprofile[t.ttentid].headframefinish; t.tlmb++ )
								{
									if (  t.tobj <= t.tlimbmax ) 
									{
										RotateLimb (  t.tobj,t.tlmb,Rnd(80)-40,Rnd(80)-40,0 );
									}
								}
							}
						}
					}
				}
				t.tdamagemultiplier_f=4.0f;
			}
		}
	}
	return;
	*/
}

void entity_determinegunforce ( void )
{
	//  bulletraytype (1-pierce, 2-shotgun pellets)
	t.bulletraytype=g.firemodes[t.gunid][g.firemode].settings.damagetype;
	t.tforce_f=g.firemodes[t.gunid][g.firemode].settings.force*2.0;
	if (  t.gun[t.gunid].settings.ismelee == 2 ) 
	{
		//  100415 - added separate force for melee attacks
		t.tforce_f=g.firemodes[t.gunid][g.firemode].settings.meleeforce*2.0;
	}
	else
	{
		//  regular bullet type force modifiers
		if (  t.bulletraytype == 2  )  t.tforce_f = t.tforce_f*2;
		if (  t.bulletraytype == 1 ) 
		{
			if (  t.tforce_f>40  )  t.tforce_f = 40;
		}
	}
	t.tforce_f=t.tforce_f*20.0;
return;

}

void entity_find_charanimindex_fromttte ( void )
{
	t.tcharanimindex=0;
	if (  t.ttte>0 ) 
	{
		for ( t.ttcharanimindex = 1 ; t.ttcharanimindex<=  g.charanimindexmax; t.ttcharanimindex++ )
		{
			if (  t.charanimstates[t.ttcharanimindex].e == t.ttte ) 
			{
				t.tcharanimindex=t.ttcharanimindex ; break;
			}
		}
	}
return;

}

void entity_applydamage ( void )
{
	if ( t.entityelement[t.ttte].obj <= 0 ) return;
	if ( ObjectExist ( t.entityelement[t.ttte].obj ) == 0 ) return;

	//  if entity being damaged is protagonist
	if (  t.tskiplayerautoreject == 0 ) 
	{
		if (  t.playercontrol.thirdperson.enabled == 1 ) 
		{
			if (  t.ttte == t.playercontrol.thirdperson.charactere ) 
			{
				//  this is the player that was hit, is player damage instead
				t.testore=t.te ; t.te=t.ttte;
				physics_player_takedamage ( );
				t.te=t.testore;
				return;
			}
		}
	}

	//  early exit
	if (  t.entityelement[t.ttte].active == 0  )  return;

	//  no damage if immobile and not a character (collectables)
	if (  t.tallowanykindofdamage == 0 ) 
	{
		// 010216 - if 'isimmobile' was set in FPE, it cannot be reduced to zero health
		//if (  t.entityprofile[t.ttentid].isimmobile == 1 && t.entityprofile[t.ttentid].ischaracter == 0  )  return;
		t.ttentid=t.entityelement[t.ttte].bankindex;
		if (  t.entityprofile[t.ttentid].isimmobile == 1 && t.entityprofile[t.ttentid].ischaracter == 0  )
		{
			// code at the end will cap any reduction to 1, this keeps COLLECTABLES alive but allows scripts to detect damage on ISIMMOBILE entities
		}
	}

	//  magnify damage if player has superpowers
	if (  t.tdamagesource == 2 ) 
	{
		if (  t.player[t.plrid].powers.level != 100 ) 
		{
			t.tpowerratio_f=(t.player[t.plrid].powers.level+0.0)/100.0;
			t.tdamage=(t.tdamage+0.0)*t.tpowerratio_f;
			t.tdamageforce=(t.tdamageforce+0.0)*t.tpowerratio_f;
		}
	}

	//  if damaging ai in mp, you will take the aggro of the enemy
	if (  t.game.runasmultiplayer == 1 && g.steamworks.coop  ==  1 && g.steamworks.ignoreDamageToEntity  ==  0 ) 
	{
		t.ttentid=t.entityelement[t.ttte].bankindex;
		if (  t.ttentid>0 ) 
		{
			if (  (t.entityprofile[t.ttentid].ischaracter  ==  1 || t.entityelement[t.ttte].mp_isLuaChar  ==  1) && t.entityelement[t.ttte].mp_coopControlledByPlayer  !=  g.steamworks.me && t.entityprofile[t.entid].ismultiplayercharacter  ==  0 ) 
			{
				if (  t.entityelement[t.ttte].mp_coopControlledByPlayer  ==  -1 ) 
				{
					t.tsteamplayeralive = 0;
				}
				else
				{
					t.tsteamplayeralive = SteamGetPlayerAlive(t.entityelement[t.ttte].mp_coopControlledByPlayer);
				}
				if (  Timer() - t.entityelement[t.ttte].mp_coopLastTimeSwitchedTarget > 5000 || t.tsteamplayeralive  ==  0 ) 
				{
					t.entityelement[t.ttte].mp_coopControlledByPlayer = g.steamworks.me;
					t.entityelement[t.ttte].mp_updateOn = 1;
					SteamSendLua (  Steam_LUA_TakenAggro,t.ttte,g.steamworks.me );
					t.entityelement[t.ttte].mp_coopLastTimeSwitchedTarget = Timer()+5000;
				}
			}
		}
	}

	//  takes ttte, tdamage, tdamageforce, tdamagesource
	//  takes brayx1# to brayz2#
	//  tdamagesource = 0;self 1;bullet 2;explosion
	t.tobj=t.entityelement[t.ttte].obj;
	//  work out force
	if (  t.tdamageforce>0 ) 
	{
		//  force given
		t.tforce_f=t.tdamageforce;
		//  stop force going overboard in multiplayer or if character
		t.tokay=0;
		if (  t.entityprofile[t.entityelement[t.ttte].bankindex].ismultiplayercharacter == 1  )  t.tokay = 1;
		if (  t.game.runasmultiplayer == 1 && g.steamworks.coop == 1 && t.entityprofile[t.entityelement[t.ttte].bankindex].ismultiplayercharacter == 1  )  t.tokay = 1;
		if (  t.tokay == 1 ) 
		{
			if (  t.tforce_f>150.0  )  t.tforce_f = 150.0;
		}
	}
	else
	{
		//  no force (possibly provided by ODERayTerrainEx (  HIT elsewhere such as bulletray subroutine) )
		t.tforce_f=0.0f;
	}
	//  find character or non-character
	entity_find_charanimindex_fromttte ( );
	t.ttentid=t.entityelement[t.ttte].bankindex;
	if (  t.tcharanimindex == 0 && t.tforce_f>0 && t.entityprofile[t.ttentid].ragdoll == 0 ) 
	{
		//  NON-CHARACTER ENTITY (used for explosion forces)
		t.tdx_f=(t.brayx2_f-t.brayx1_f)*(t.tforce_f/90.0);
		t.tdy_f=(t.brayy2_f-t.brayy1_f)*(t.tforce_f/90.0);
		t.tdz_f=(t.brayz2_f-t.brayz1_f)*(t.tforce_f/90.0);
		ODEAddBodyForce (  t.tobj,t.tdx_f,t.tdy_f,t.tdz_f,0,0,0 );
		ODESetAngularVelocity (  t.tobj,Rnd(600)-300,Rnd(200)-100,Rnd(600)-300 );
	}

	//  work out damage and see if entity gets destroyed
	entity_determinedamagemultiplier ( );
	t.tdamage=t.tdamage*t.tdamagemultiplier_f;

	//  apply damage locally if not multiplayer
	if (  t.game.runasmultiplayer == 0 ) 
	{
		t.entityelement[t.ttte].health=t.entityelement[t.ttte].health-t.tdamage;
	}
	else
	{
		//  Multiplayer, first checks if it is a player, if it is, we send the damage to them to apply
		//  If not, we appply it and inform everyone else
		if (  t.entityprofile[t.entityelement[t.ttte].bankindex].ismultiplayercharacter == 1 ) 
		{
			for ( int tpindex = 0 ; tpindex<=  STEAM_MAX_NUMBER_OF_PLAYERS-1; tpindex++ )
			{
				if (  t.steamworks_playerEntityID[tpindex]  ==  t.ttte && tpindex  !=  g.steamworks.me && SteamGetPlayerAlive(tpindex)  ==  1 ) 
				{
					t.tSteamForce_f = t.tforce_f;
					if (  t.tSteamForce_f  ==  150  )  t.tSteamForce_f  =  300;
					if (  g.steamworks.ignoreDamageToEntity  ==  0 ) 
					{
						t.tsteamlastdamagesentcounter = t.tsteamlastdamagesentcounter + 1;
						//  13032015 0XX - Team Multiplayer
						if (  g.steamworks.team  ==  0 || g.steamworks.friendlyfireoff  ==  0 || t.steamworks_team[tpindex]  !=  t.steamworks_team[g.steamworks.me] ) 
						{
							// Ignore setentityhealth lua message if it is a player
							//if ( t.tallowanykindofdamage == 0 )
							//{
								SteamApplyPlayerDamage (  tpindex,t.tdamage, t.brayx2_f-t.brayx1_f, t.brayy2_f-t.brayy1_f, t.brayz2_f-t.brayz1_f, t.tSteamForce_f, t.bulletraylimbhit );
							//}
						}
					}
				}
			}
		}
		else
		{
			//  it is not a player, so we can apply damage to it
			if (  t.entityelement[t.ttte].health > 0 ) 
			{
				t.entityelement[t.ttte].health=t.entityelement[t.ttte].health-t.tdamage;
				if (  g.steamworks.ignoreDamageToEntity  ==  0 ) 
				{
					if (  t.entityelement[t.ttte].health  <= 0 ) 
					{
						//  for coop, we count ai kills and not player kills
						if (  g.steamworks.coop  ==  1 ) 
						{
							t.tttentid=t.entityelement[t.ttte].bankindex;
							if (  t.tttentid > 0 ) 
							{
								if (  t.entityprofile[t.tttentid].ischaracter  ==  1 || t.entityelement[t.ttte].mp_isLuaChar  ==  1 ) 
								{
									steam_IKilledAnAI ( );
								}
							}
						}
						++t.tempsteamdestroycount;
						steam_destroyentity ( );
					}
				}
			}
		}
	}
	t.entityelement[t.ttte].lua.flagschanged=1;

	// 010216 - special case for ISIMMOBILE FPE that are not characters, they cannot get to zero health (COLLECTABLES)
	if (  t.tallowanykindofdamage == 0 ) 
	{
		t.ttentid=t.entityelement[t.ttte].bankindex;
		if (  t.entityprofile[t.ttentid].isimmobile == 1 && t.entityprofile[t.ttentid].ischaracter == 0  )
		{
			if ( t.entityelement[t.ttte].health <= 0 )
				t.entityelement[t.ttte].health = 1;
		}
	}

	//  when health drops to zero
	if ( t.entityelement[t.ttte].health <= 0 ) 
	{
		//  if explodble, have a delayed reaction
		if (  t.entityelement[t.ttte].eleprof.explodable != 0 ) 
		{
			if (  t.entityelement[t.ttte].explodefusetime  ==  0 ) 
			{
				if (  t.tdamagesource == 2 ) 
				{
					//  explosion is time delayed
					t.entityelement[t.ttte].explodefusetime=Timer()+250;
				}
				else
				{
					//  explosion is instant
					t.entityelement[t.ttte].explodefusetime=Timer();
				}
			}
		}
		else
		{
			//  reset health to zero
			t.entityelement[t.ttte].health=0;
		}

		// 010616 - May be a third person character, no ragdoll means find death animation and play
		int iThirdPersonCharacter = 0;
		if ( t.playercontrol.thirdperson.enabled == 1 ) 
			if ( t.playercontrol.thirdperson.charactere == t.ttte )
				iThirdPersonCharacter = t.playercontrol.thirdperson.characterindex;

		//  if character
		t.tapplyragdollforce=0;
		int iCharacterIndexToUse = t.tcharanimindex;
		if ( iThirdPersonCharacter > 0 ) iCharacterIndexToUse = iThirdPersonCharacter;
		if ( iCharacterIndexToUse > 0 ) 
		{
			//  CHARACTER
			if ( iThirdPersonCharacter==0 ) 
			{
				int iStoreCharIndex = t.tcharanimindex;
				t.tcharanimindex = iCharacterIndexToUse;
				darkai_killai ( );
				t.tcharanimindex = iStoreCharIndex;
			}

			//  if dead, trigger impact death
			t.tdx_f=ObjectPositionX(t.tobj)-t.twhox_f;
			t.tdz_f=ObjectPositionZ(t.tobj)-t.twhoz_f;
			t.tda_f=atan2deg(t.tdx_f,t.tdz_f);
			t.relativeangle_f=WrapValue(ObjectAngleY(t.tobj)-t.tda_f);
			t.impacting=5;
			if (  t.relativeangle_f >= 315-22 || t.relativeangle_f<45+22 ) 
			{
				t.impacting=1;
			}
			else
			{
				if (  t.relativeangle_f >= 45+22 && t.relativeangle_f<135-22  )  t.impacting = 3;
				if (  t.relativeangle_f >= 135-22 && t.relativeangle_f<225+22  )  t.impacting = 2;
				if (  t.relativeangle_f >= 225+22 && t.relativeangle_f<315-22  )  t.impacting = 4;
			}

			//  cannot use state engine - use instant animation for this
			if ( t.charanimstates[iCharacterIndexToUse].playcsi > 0 && t.charanimstates[iCharacterIndexToUse].playcsi >= t.csi_crouchidle[t.charanimstates[iCharacterIndexToUse].weapstyle] && t.charanimstates[iCharacterIndexToUse].playcsi <= t.csi_crouchgetup[t.charanimstates[iCharacterIndexToUse].weapstyle] ) 
			{
				//  die crouched
				if (  t.impacting == 1  )  t.charanimstates[iCharacterIndexToUse].playcsi = t.csi_crouchimpactfore[t.charanimstates[iCharacterIndexToUse].weapstyle];
				if (  t.impacting == 2  )  t.charanimstates[iCharacterIndexToUse].playcsi = t.csi_crouchimpactback[t.charanimstates[iCharacterIndexToUse].weapstyle];
				if (  t.impacting == 3  )  t.charanimstates[iCharacterIndexToUse].playcsi = t.csi_crouchimpactleft[t.charanimstates[iCharacterIndexToUse].weapstyle];
				if (  t.impacting == 4  )  t.charanimstates[iCharacterIndexToUse].playcsi = t.csi_crouchimpactright[t.charanimstates[iCharacterIndexToUse].weapstyle];
			}
			else
			{
				//  die stood
				if (  t.impacting == 1  )  t.charanimstates[iCharacterIndexToUse].playcsi = g.csi_unarmedimpactfore;
				if (  t.impacting == 2  )  t.charanimstates[iCharacterIndexToUse].playcsi = g.csi_unarmedimpactback;
				if (  t.impacting == 3  )  t.charanimstates[iCharacterIndexToUse].playcsi = g.csi_unarmedimpactleft;
				if (  t.impacting == 4  )  t.charanimstates[iCharacterIndexToUse].playcsi = g.csi_unarmedimpactright;
			}
			if (  t.impacting == 5  )  t.charanimstates[iCharacterIndexToUse].playcsi = g.csi_unarmeddeath;
			t.smoothanim[t.tobj].transition=0;

			// only for regular characters
			if ( iThirdPersonCharacter==0 )
			{
				t.entityelement[t.charanimstates[iCharacterIndexToUse].e].health=0;

				//  setting main to 0 so the main lua won't be called for this object
				t.entityelement[t.ttte].eleprof.aimain = 0;

				//  Prepare character for eventual fade out
				t.charanimstates[iCharacterIndexToUse].timetofadeout=Timer()+AICORPSETIME;
				t.charanimstates[iCharacterIndexToUse].fadeoutvalue_f=1.0;
			}

			//  Convert to clone so can operate independent of parent object
			t.tte = t.ttte ; entity_converttoclone ( );

			//  Ragdoll for characters is now optional
			if ( t.entityprofile[t.ttentid].ragdoll == 1 ) 
			{
				//  create ragdoll and stop any further manipulation of the object
				ragdoll_setcollisionmask ( t.entityelement[t.ttte].eleprof.colondeath );
				t.tphye=t.ttte ; t.tphyobj=t.entityelement[t.ttte].obj  ; ragdoll_create ( );
				t.tapplyragdollforce=1;
			}

			//  and make attachment object a physics object
			t.tattobj=t.entityelement[t.ttte].attachmentobj;
			if ( t.tattobj>0  )  ODECreateDynamicBox (  t.tattobj,-1,1 );

			// only for regular characters
			if ( iThirdPersonCharacter==0 )
			{
				//  Ensure character control ceases at this (instantly for ragdoll / anim death delays this assignment)
				if ( t.entityprofile[t.ttentid].ragdoll == 1 )
				{
					t.charanimstates[iCharacterIndexToUse].e=0;
				}
			}
		}
		else
		{
			//  NON-CHARACTER, but can still have ragdoll flagged (like Zombies)
			if (  t.entityprofile[t.ttentid].ragdoll == 1 ) 
			{
				//  can only ragdoll clones not instances
				t.tte=t.ttte ; entity_converttoclone ( );

				//  create ragdoll and stop any further manipulation of the object
				ragdoll_setcollisionmask ( t.entityelement[t.ttte].eleprof.colondeath );
				t.tphye=t.ttte ; t. tphyobj=t.entityelement[t.ttte].obj  ; ragdoll_create ( );
				t.tapplyragdollforce=1;

				//  and make attachment object a physics object
				t.tattobj=t.entityelement[t.ttte].attachmentobj;
				if (  t.tattobj>0  )  ODECreateDynamicBox (  t.tattobj,-1,1 );

			}
		}

		//  multiplayer undocumented stuff
		if (  t.game.runasmultiplayer  ==  1 ) 
		{
			if (  g.steamworks.ignoreDamageToEntity  ==  1 ) 
			{
				if (  t.tapplyragdollforce == 1 ) 
				{
					t.tapplyragdollforce = 0;
					t.entityelement[t.ttte].ragdollified=1;
				}
			}
		}

		//  and apply bullet directional force (tforce#=from gun settings)
		if (  t.tapplyragdollforce == 1 ) 
		{
			t.entityelement[t.ttte].ragdollified=1;
			t.entityelement[t.ttte].ragdollifiedforcex_f=(t.brayx2_f-t.brayx1_f)*0.8;
			t.entityelement[t.ttte].ragdollifiedforcey_f=(t.brayy2_f-t.brayy1_f)*1.2;
			t.entityelement[t.ttte].ragdollifiedforcez_f=(t.brayz2_f-t.brayz1_f)*0.8;
			if (  t.game.runasmultiplayer  ==  0 ) 
			{
				t.entityelement[t.ttte].ragdollifiedforcevalue_f=t.tforce_f*8000.0;
			}
			else
			{
				t.tsteamcoopforcemulti_f = 8000.0;
				if (  g.steamworks.coop  ==  1 ) 
				{
					if (  t.entityprofile[t.entityelement[t.ttte].bankindex].ismultiplayercharacter == 1 ) 
					{
						if (  t.tforce_f > 300.0  )  t.tforce_f  =  300.0;
						t.tsteamcoopforcemulti_f = 2000.0;
					}
				}
				t.entityelement[t.ttte].ragdollifiedforcevalue_f=t.tforce_f*t.tsteamcoopforcemulti_f;
			}
			t.entityelement[t.ttte].ragdollifiedforcelimb=t.bulletraylimbhit;
			t.bulletraylimbhit=-1;
		}

	}
}

void entity_gettruecamera ( void )
{
	//  True camera position
	if (  t.playercontrol.thirdperson.enabled == 1 ) 
	{
		t.tcamerapositionx_f=t.playercontrol.thirdperson.storecamposx;
		t.tcamerapositiony_f=t.playercontrol.thirdperson.storecamposy;
		t.tcamerapositionz_f=t.playercontrol.thirdperson.storecamposz;
	}
	else
	{
		t.tcamerapositionx_f=CameraPositionX(t.terrain.gameplaycamera);
		t.tcamerapositiony_f=CameraPositionY(t.terrain.gameplaycamera);
		t.tcamerapositionz_f=CameraPositionZ(t.terrain.gameplaycamera);
	}
}

void entity_hasbulletrayhit(void)
{
	// bulletray is x1#,y1#,z1#,x2#,y2#,z2#,bulletrayhit,gunrange#
	t.brayx1_f = t.x1_f; t.brayy1_f = t.y1_f; t.brayz1_f = t.z1_f;
	t.brayx2_f = t.x2_f; t.brayy2_f = t.y2_f; t.brayz2_f = t.z2_f;
	t.bulletrayhit = 0; t.bulletraylimbhit = -1; t.tttriggerdecalimpact = 0;
	t.tfoundentityindexhit = -1;
	
	// first cast a ray at any terrain
	if (ODERayTerrain(t.brayx1_f, t.brayy1_f, t.brayz1_f, t.brayx2_f, t.brayy2_f, t.brayz2_f) == 1)
	{
		//  and shorten the ray if we hit terra firma!
		t.brayx2_f = ODEGetRayCollisionX();
		t.brayy2_f = ODEGetRayCollisionY();
		t.brayz2_f = ODEGetRayCollisionZ();
		//  trigger dust flume at coords
		t.tttriggerdecalimpact = 10;
	}
	
	// create a ray and check for object hit (first intersectall command simply fills a secondary range of objects)
	t.ttt = IntersectAll(g.lightmappedobjectoffset, g.lightmappedobjectoffsetfinish, 0, 0, 0, 0, 0, 0, -123);
	// Character creator can override the limb hit, to make the cc head report the head limb of the main character
	t.ccLimbHitOverride = false;
	// check if character creator characters are in the game
	if (t.characterkitcontrol.gameHasCharacterCreatorIn == 1)
	{
		// 010715 - will even detect GLUED OBJECTS thanks to new code in IntersectAll (  )
		t.ttt = IntersectAll(g.charactercreatorrmodelsoffset + ((t.characterkitcontrol.minEntity * 3) - t.characterkitcontrol.offset), g.charactercreatorrmodelsoffset + ((t.characterkitcontrol.maxEntity * 3) - t.characterkitcontrol.offset), 0, 0, 0, 0, 0, 0, -124);
	}
	if (g.firemodes[t.gunid][0].settings.detectcoloff == 1)
	{
		// 111215 - '-125' is ignore collision property where entity was CollisionOff(e)
		t.ttt = IntersectAll(0, 0, 0, 0, 0, 0, 0, 0, -125); 
	}
	// if TPP, can ignore entity used as player
	int iIgnoreOneEntityObj = 0;
	if ( t.playercontrol.thirdperson.enabled == 1 )
	{
		// 220217 - cannot shoot self with weapon!
		iIgnoreOneEntityObj = t.entityelement[t.playercontrol.thirdperson.charactere].obj;
	}
	t.thitvalue=IntersectAll(g.entityviewstartobj,g.entityviewendobj,t.brayx1_f,t.brayy1_f,t.brayz1_f,t.brayx2_f,t.brayy2_f,t.brayz2_f,iIgnoreOneEntityObj);
	if (  t.thitvalue>0 ) 
	{
		//  check if it was a character creator object hit and find the main body object
		if (  t.characterkitcontrol.gameHasCharacterCreatorIn  ==  1 ) 
		{
			if (  t.thitvalue  >=  g.charactercreatorrmodelsoffset+((t.characterkitcontrol.minEntity*3)-t.characterkitcontrol.offset) && t.thitvalue  <=  g.charactercreatorrmodelsoffset+((t.characterkitcontrol.maxEntity*3)-t.characterkitcontrol.offset) ) 
			{
				t.toldthitvalue=t.thitvalue;
				t.tTheE = ((t.thitvalue - g.charactercreatorrmodelsoffset) + t.characterkitcontrol.offset) / 3;
				t.thitvalue = t.entityelement[t.tTheE].obj;
				//Character creator can override the limb hit, to make the cc head report the head limb of the main character
				t.ccLimbHitOverride = true;
				t.ccLimbHitOverrideLimb  = getlimbbyname(t.entityelement[t.tTheE].obj, "Bip01_Head");
			}
		}
		//  record object number we hit
		t.tsteamLastHit=t.thitvalue;
		t.bulletrayhit=t.thitvalue;
		//  first check if object uses 'physics collision' over 'geometry collision'
		if ( t.bulletrayhit>0 ) 
		{
			if ( ObjectExist(t.bulletrayhit) == 1 ) 
			{
				for ( t.tte = 1 ; t.tte<=  g.entityelementlist; t.tte++ )
				{
					if ( t.entityelement[t.tte].obj == t.bulletrayhit ) 
					{
						t.tfoundentityindexhit=t.tte ; break;
					}
				}
			}
		}
		t.tcollisionwithphysics=0;
		if (  t.tfoundentityindexhit != -1 ) 
		{
			t.tentid=t.entityelement[t.tfoundentityindexhit].bankindex;
			if (  t.entityprofile[t.tentid].collisionoverride == 1 || t.entityprofile[t.tentid].collisionmode == 11 ) 
			{
				if (  t.entityprofile[t.tentid].collisionmode == 11 ) 
				{
					t.tcollisionwithphysics=2;
				}
				else
				{
					if (  ODERayTerrainEx(t.brayx1_f,t.brayy1_f,t.brayz1_f,t.brayx2_f,t.brayy2_f,t.brayz2_f,2) == 1 ) 
					{
						t.tcollisionwithphysics=1;
					}
					else
					{
						t.tcollisionwithphysics=2;
					}
				}
			}
		}
		if (  t.tcollisionwithphysics>0 ) 
		{
			if (  t.tcollisionwithphysics == 1 ) 
			{
				//  found hit woth physics shapes instead of geometry
				t.brayx2_f=ODEGetRayCollisionX();
				t.brayy2_f=ODEGetRayCollisionY();
				t.brayz2_f=ODEGetRayCollisionZ();
				t.bulletraylimbhit=0;
				t.tmaterialvalue=t.entityprofile[t.tentid].materialindex;
			}
			else
			{
				//  hit geometry but missed physics shape, no collision (foliage banana tree)
				t.bulletrayhit=-1;
				t.bulletraylimbhit=-1;
			}
		}
		else
		{
			//  shorten ray to reflect hit coordinate
			t.brayx2_f=ChecklistFValueA(6);
			t.brayy2_f=ChecklistFValueB(6);
			t.brayz2_f=ChecklistFValueC(6);
			//  get limb we hit (for flinch effect when we hit enemy limb)
			t.tlimbhit=ChecklistValueB(1);

			//Check if we hit character creator head and adjust limbhit to the head of the character
			if ( t.ccLimbHitOverride )
			{
				t.tlimbhit = t.ccLimbHitOverrideLimb;
				t.ccLimbHitOverride = false;
			}

			if ( t.tfoundentityindexhit != -1 ) 
			{
				t.entityelement[t.tfoundentityindexhit].detectedlimbhit = 0;
				t.entityelement[t.tfoundentityindexhit].lua.flagschanged = 1;
			}
			if (  t.tlimbhit>0 ) 
			{
				if (  ObjectExist(t.bulletrayhit) == 1 ) 
				{
					if (  LimbExist(t.bulletrayhit,t.tlimbhit) == 1 ) 
					{
						// record which limb we hit
						t.bulletraylimbhit = t.tlimbhit;

						// 201115 - also record limb hit within entity (so LUA can do stuff with the info)
						if ( t.tfoundentityindexhit != -1 ) 
						{
							t.entityelement[t.tfoundentityindexhit].detectedlimbhit = t.tlimbhit;
							t.entityelement[t.tfoundentityindexhit].lua.flagschanged = 1;
						}
					}
				}
			}
			//  return material index and use to trigger decal
			t.tmaterialvalue=ChecklistValueA(9);
		}
		if (  t.tmaterialvalue>0  )  t.tttriggerdecalimpact = 10+t.tmaterialvalue;
	}

	//  calculate increment along ray
	t.tbix_f=t.brayx2_f-t.brayx1_f;
	t.tbiy_f=t.brayy2_f-t.brayy1_f;
	t.tbiz_f=t.brayz2_f-t.brayz1_f;
	t.trange_f=Sqrt(abs(t.tbix_f*t.tbix_f)+abs(t.tbiy_f*t.tbiy_f)+abs(t.tbiz_f*t.tbiz_f));
	t.tbix_f=t.tbix_f/t.trange_f;
	t.tbiy_f=t.tbiy_f/t.trange_f;
	t.tbiz_f=t.tbiz_f/t.trange_f;

	//  if bullet ray passed waterlevel, create a splash at intersection
	if (  t.hardwareinfoglobals.nowater == 0 ) 
	{
		if (  t.decalglobal.splashdecalrippleid != 0 && t.terrain.TerrainID>0 && ((t.brayy1_f>t.terrain.waterliney_f && t.brayy2_f<t.terrain.waterliney_f) || (t.brayy1_f<t.terrain.waterliney_f && t.brayy2_f>t.terrain.waterliney_f)) ) 
		{
			//  calculate coordate where ray hit water plane
			t.tperc_f=(t.brayy1_f-t.terrain.waterliney_f)/abs(t.tbiy_f);
			t.tbx_f=t.brayx1_f+(t.tbix_f*t.tperc_f);
			t.tby_f=t.brayy1_f+(t.tbiy_f*t.tperc_f);
			t.tbz_f=t.brayz1_f+(t.tbiz_f*t.tperc_f);
			//  check if this coord ABOVE terrain Floor (  )
			t.tgroundheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tbx_f,t.tbz_f);
			if (  t.tby_f>t.tgroundheight_f ) 
			{
				//  trigger water splash at coords
				g.decalx=t.tbx_f ; g.decaly=t.tby_f+0.5 ; g.decalz=t.tbz_f; t.tInScale_f = 1;
				decal_triggerwatersplash ( );
				//  play splash sound
				t.tmatindex=17 ; t.tsoundtrigger=t.material[t.tmatindex].impactid;
				t.tspd_f=(t.material[t.tmatindex].freq*1.5)+Rnd(t.material[t.tmatindex].freq)*0.5;
				t.tsx_f=g.decalx ; t.tsy_f=g.decaly ; t.tsz_f=g.decalz;
				t.tvol_f = 6 ;  material_triggersound ( );
				t.tsoundtrigger=0;
			}
		}
	}

	//  check if we hit a character
	if (  t.bulletrayhit>0 ) 
	{
		if (  ObjectExist(t.bulletrayhit) == 1 ) 
		{
			//  Find which entity this is
			t.bulletrayhite=-1 ; t.bulletrayhitentid=-1;
			t.tobj=t.bulletrayhit;
			if (  t.tfoundentityindexhit != -1 ) 
			{
				t.tte=t.tfoundentityindexhit ; t.bulletrayhite=t.tte;
				t.bulletrayhitentid=t.entityelement[t.tte].bankindex;
			}
			else
			{
				if (  t.tobj>0 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						for ( t.tte = 1 ; t.tte<=  g.entityelementlist; t.tte++ )
						{
							if (  t.entityelement[t.tte].obj == t.tobj ) 
							{
								t.bulletrayhite=t.tte ; t.bulletrayhitentid=t.entityelement[t.tte].bankindex ; break;
							}
						}
					}
				}
			}

			//  Check if this object is a character
			if (  t.bulletrayhitentid != -1 ) 
			{
				t.px_f=t.x1_f ; t.py_f=t.y1_f ; t.pz_f=t.z1_f;
				entity_determinegunforce ( );
				darkai_ischaracterhit ( );
				if (  t.darkaifirerayhitcharacter == 0 ) 
				{
					//  also make sure it's not a beast (ragdoll)
					t.tokay = 1 ; if (  t.entityprofile[t.bulletrayhitentid].ragdoll == 1  )  t.tokay = 0;
					if (  t.tokay == 1 ) 
					{
						//  create a ray of force along bullet tragectory (to disturb non-character objects)
						if (  ODERayForce(t.brayx1_f,t.brayy1_f,t.brayz1_f,t.brayx2_f,t.brayy2_f,t.brayz2_f,t.tforce_f*0.25) == 1 ) 
						{
							//  and knock dynamic physics objects if force is applied to them
						}
					}
				}
				if (  t.darkaifirerayhitcharacter == 1 ) 
				{
					//  trigger limb flinch system (limbhurt and limbhurta#)
					if (  t.bulletraylimbhit != -1 && t.bulletrayhite != -1 ) 
					{
						t.tte=t.bulletrayhite;
						if (  t.entityelement[t.tte].limbhurt <= 0 ) 
						{
							t.entityelement[t.tte].limbhurt=t.bulletraylimbhit;
							//  determine if entity facing away from plr
							t.tdx_f=ObjectPositionX(t.tobj)-CameraPositionX(0);
							t.tdz_f=ObjectPositionZ(t.tobj)-CameraPositionZ(0);
							t.tangley_f=atan2deg(t.tdx_f,t.tdz_f);
							t.tdiffhurtangle_f=t.tangley_f-ObjectAngleY(t.tobj);
							if (  t.tdiffhurtangle_f<-180  )  t.tdiffhurtangle_f = t.tdiffhurtangle_f+360;
							if (  t.tdiffhurtangle_f>180  )  t.tdiffhurtangle_f = t.tdiffhurtangle_f-360;
							if (  abs(t.tdiffhurtangle_f)<90.0 ) 
							{
								//  bend forward
								t.entityelement[t.tte].limbhurta_f=8+Rnd(8);
							}
							else
							{
								//  bend back
								t.entityelement[t.tte].limbhurta_f=(8+Rnd(8))*-1;
							}
						}
					}
					//  cause blood splat (if violent)
					if (  t.bulletrayhite != -1 ) 
					{
						if (  t.entityelement[t.bulletrayhite].eleprof.isviolent != 0 && g.quickparentalcontrolmode != 2) 
						{
							// 100317 - only if material index not specified (or materialindex=0) (16=flesh)
							if ( t.tttriggerdecalimpact <= 10 || t.tttriggerdecalimpact == 16)
							{
								t.tttriggerdecalimpact=2;
							}
						}
					}
					t.bulletrayhit=0;
				}
			}
			else
			{
				//  did not hit entity, could be we hit a lightmapped static object
				t.bulletrayhite=0;
			}

			//  determine which entity we hit (if not character which is already handled)
			//  for things such as Zombies and other entities in the level
			if ( t.bulletrayhit > 0 && t.bulletrayhite != -1 ) 
			{
				// apply some damage
				t.tdamagesource = 1;
				t.tdamage = g.firemodes[t.gunid][g.firemode].settings.damage;
				if ( t.gun[t.gunid].settings.ismelee == 2 ) t.tdamage = g.firemodes[t.gunid][0].settings.meleedamage;
				entity_hitentity ( t.bulletrayhite, t.bulletrayhit );
			}
		}
	}

	// trigger decal at impact coordinate
	entity_triggerdecalatimpact ( t.brayx2_f, t.brayy2_f, t.brayz2_f );
}

void entity_hitentity ( int e, int obj )
{
	int iStoreE = t.ttte; t.ttte = e;
	if ( t.entityelement[t.ttte].health > 0 ) 
	{
		//  turn into clone in case we need to animate it (could be in distance)
		if ( GetNumberOfFrames(obj) > 0 ) 
		{
			entity_converttoclone ( );
		}

		// determine and set damage force
		entity_determinegunforce ( );
		t.ttentid=t.entityelement[t.ttte].bankindex;
		if (  t.entityprofile[t.ttentid].ischaracter == 1 || t.entityprofile[t.ttentid].ragdoll == 1 ) 
		{
			t.tdamageforce=t.tforce_f;
		}
		else
		{
			t.tdamageforce=0;
		}

		// apply the damage
		entity_applydamage ( );

		// check if we hit an organic or custom character (animal, zombie, etc)
		t.ttentid = t.entityelement[t.ttte].bankindex;

		// cause blood splat
		if ( t.entityprofile[t.ttentid].materialindex == 6 || ( t.entityprofile[t.ttentid].materialindex == 0 && t.entityprofile[t.ttentid].ischaracter == 1 ) ) 
		{
			if ( t.entityelement[t.ttte].eleprof.isviolent != 0 && g.quickparentalcontrolmode != 2 )
			{
				t.tttriggerdecalimpact=2;
			}
		}

		//  cause blood splat on steam multiplayer char
		if ( t.game.runasmultiplayer == 1 ) 
		{
			if ( t.entityprofile[t.ttentid].ismultiplayercharacter == 1 ) 
			{
				if ( t.entityelement[t.ttte].eleprof.isviolent != 0 && g.quickparentalcontrolmode != 2 )
				{
					t.tttriggerdecalimpact=2;
				}
			}
		}
	}
	t.ttte = iStoreE;
}

void entity_triggerdecalatimpact ( float fX, float fY, float fZ )
{
	// trigger decal at impact coordinate
	// 111215 - and only if ignorematerial flag not set by GUNSPEC (for interactive HUD 'weapons')
	if ( t.tttriggerdecalimpact>0 && (t.gunid==0 || g.firemodes[t.gunid][0].settings.ignorematerial == 0) )
	{
		//  trigger decal animation at coords
		g.decalx=fX; g.decaly=fY+0.5 ; g.decalz=fZ;
		if ( t.tttriggerdecalimpact >= 10 ) decal_triggermaterialdecal ( );
		if ( t.tttriggerdecalimpact == 1 ) decal_triggerimpact ( );
		if ( t.tttriggerdecalimpact == 2 ) 
		{
			if ( t.playercontrol.startviolent != 0 && g.quickparentalcontrolmode != 2 ) 
			{
				for ( t.iter = 1 ; t.iter <= 3+Rnd(1); t.iter++ )
				{
					decal_triggerbloodsplat ( );
				}
			}
		}

		//  trigger ai sound so enemies can pick up the shot
		if ( t.gunid > 0 )
		{
			t.tradius_f=2000;
			AICreateSound (  t.brayx2_f,t.brayy2_f+1.0,t.brayz2_f,t.tradius_f,t.tradius_f,-1 );
			g.aidetectnearbymode=1;
			g.aidetectnearbycount=60*4;
			g.aidetectnearbymodeX_f = t.brayx2_f;
			g.aidetectnearbymodeZ_f = t.brayz2_f;
		}

		//  play material impact sound
		t.tmatindex = 0 ; if (  t.tttriggerdecalimpact >= 10  )  t.tmatindex = t.tttriggerdecalimpact-10;
		t.tsoundtrigger=t.material[t.tmatindex].impactid;
		t.tspd_f=t.material[t.tmatindex].freq;
		t.tsx_f=g.decalx ; t.tsy_f=g.decaly ; t.tsz_f=g.decalz;
		t.tvol_f = 100.0f ; material_triggersound ( );
		t.tsoundtrigger=0;
	}
}

void entity_createattachment ( void )
{
	//  Single player character must HOLD the weapon before attaching it
	t.tischaracterholdingweapon=0;
	if (  t.entityprofile[t.entid].ischaracter == 1 && t.entityelement[t.e].eleprof.hasweapon>0 ) 
	{
		t.tischaracterholdingweapon=1;
	}

	//  Load all VWEAPS for each entity that wants weapon attachments
	t.entid=t.entityelement[t.e].bankindex;
	if (  (t.tischaracterholdingweapon == 1 || t.entityprofile[t.entid].ismultiplayercharacter == 1) && t.entityelement[t.e].obj>0 ) 
	{

		//  Make attachment if warranted
		if (  ObjectExist(t.entityelement[t.e].obj) == 1 && t.entityelement[t.e].attachmentobj == 0 ) 
		{
			if (  t.entityprofile[t.entid].firespotlimb>-1 ) 
			{

				//  all vweaps (that are active)
				for ( t.tgindex = 1 ; t.tgindex<=  g.gunmax; t.tgindex++ )
				{
					if (  t.gun[t.tgindex].activeingame == 1 ) 
					{
						t.tweaponname_s=t.gun[t.tgindex].name_s;
						if (  t.tweaponname_s != "" ) 
						{

							//  entity has this gun in their hands
							t.tthasweapon_s=Lower(t.entityprofile[t.entid].hasweapon_s.Get());
							// Dave - added .Lower() as Uzi was being compared to uzi
							if (  t.tthasweapon_s == t.tweaponname_s.Lower() ) 
							{

								//  go and load this gun (attached to calling entity instance)
								++g.entityattachmentindex;
								t.ttobj=g.entityattachmentsoffset+g.entityattachmentindex;
								if (  ObjectExist(t.ttobj) == 1  )  DeleteObject (  t.ttobj );

								//  replaced X file load with optional DBO convert/load
								t.tfile_s="gamecore\\guns\\";
								t.tfile_s += t.tweaponname_s+"\\vweap.x";
								deleteOutOfDateDBO(t.tfile_s.Get());
								if (  cstr(Lower(Right(t.tfile_s.Get(),2))) == ".x"  )  {t.tdbofile_s = Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-2); t.tdbofile_s += ".dbo"; } else t.tdbofile_s = "";
								if (  FileExist(t.tfile_s.Get()) == 1 || FileExist(t.tdbofile_s.Get()) == 1 ) 
								{
									if (  FileExist(t.tdbofile_s.Get()) == 1 ) 
									{
										t.tfile_s=t.tdbofile_s;
										t.tdbofile_s="";
									}
									LoadObject (  t.tfile_s.Get(),t.ttobj );
									SetObjectFilter (  t.ttobj,2 );
									if ( Len(t.tdbofile_s.Get()) > 1 ) 
									{
										if ( FileExist( t.tdbofile_s.Get()) == 0 ) 
										{
											// unnecessary now as LoadObject auto creates DBO file!
											SaveObject ( t.tdbofile_s.Get(), t.ttobj );
										}
										if (  FileExist(t.tdbofile_s.Get()) == 1 ) 
										{
											DeleteObject (  t.ttobj );
											LoadObject (  t.tdbofile_s.Get(),t.ttobj );
											SetObjectFilter (  t.ttobj,2 );
											t.tfile_s=t.tdbofile_s;
										}
									}
								}
								else
								{
									MakeObjectTriangle (  t.ttobj,0,0,0,0,0,0,0,0,0 );
								}
								t.entityelement[t.e].attachmentobj=t.ttobj;

								//  Apply object settings
								SetObjectTransparency (  t.ttobj,1 );
								SetObjectCollisionOff (  t.ttobj );
								SetObjectMask (  t.ttobj, 1 );
								EnableObjectZDepth(t.ttobj); // PE:

								//  VWEAP is NOT part of collision universe (prevents rocket hitting launcher)
								SetObjectCollisionProperty (  t.ttobj,1 );

								// 131115 - fixes issue of some models not being able to detect with intersectall
								SetObjectDefAnim ( t.ttobj, t.entityprofile[t.entid].ignoredefanim );

								//  VWEAP can choose own texture
								t.tvweaptex_s=t.gun[t.tgindex].vweaptex_s;
								if (  Len(t.tvweaptex_s.Get())<2  )  t.tvweaptex_s = "gun";

								//  apply texture to vweap
								if (  g.gdividetexturesize == 0 ) 
								{
									t.texuseid=loadinternaltexture("effectbank\\reloaded\\media\\white_D.dds");
								}
								else
								{
									sprintf ( t.szwork , "gamecore\\guns\\%s\\%s_D.dds" , t.tweaponname_s.Get() , t.tvweaptex_s.Get() );
									t.texuseid=loadinternaltexture(t.szwork);
									if (t.texuseid == 0) {
										sprintf(t.szwork, "gamecore\\guns\\%s\\%s_color.dds", t.tweaponname_s.Get(), t.tvweaptex_s.Get());
										t.texuseid = loadinternaltexture(t.szwork);
									}
								}
								TextureObject (  t.ttobj,0,t.texuseid );
								TextureObject (  t.ttobj,1,loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds") );
								sprintf ( t.szwork , "gamecore\\guns\\%s\\%s_N.dds" ,t.tweaponname_s.Get() , t.tvweaptex_s.Get() );
								t.texuseid=loadinternaltexture(t.szwork);
								if (t.texuseid == 0) {
									sprintf(t.szwork, "gamecore\\guns\\%s\\%s_normal.dds", t.tweaponname_s.Get(), t.tvweaptex_s.Get());
									t.texuseid = loadinternaltexture(t.szwork);
								}
								TextureObject (  t.ttobj,2,t.texuseid );
								sprintf ( t.szwork ,  "gamecore\\guns\\%s\\%s_S.dds" ,t.tweaponname_s.Get() , t.tvweaptex_s.Get()  );
								t.texuseid=loadinternaltexture(t.szwork);
								if (t.texuseid == 0) {
									t.texuseid = loadinternaltexture("effectbank\\reloaded\\media\\white_D.dds");
								}			

								TextureObject (  t.ttobj,3,t.texuseid );
								TextureObject (  t.ttobj,4,t.terrain.imagestartindex );
								TextureObject (  t.ttobj,5,g.postprocessimageoffset+5 );
								TextureObject (  t.ttobj,6,loadinternaltexture("effectbank\\reloaded\\media\\blank_I.dds") );

								//  Apply entity shader to vweap model
								t.teffectid=loadinternaleffect("effectbank\\reloaded\\entity_basic.fx");
								SetObjectEffect (  t.ttobj,t.teffectid );

								//  Find firespot for this vweap
								t.entityelement[t.e].attachmentobjfirespotlimb=0;
								PerformCheckListForLimbs (  t.ttobj );
								for ( t.tc = 1 ; t.tc<=  ChecklistQuantity(); t.tc++ )
								{
									if (  cstr(Lower(ChecklistString(t.tc))) == "firespot" ) 
									{
										t.entityelement[t.e].attachmentobjfirespotlimb=t.tc-1;
										t.tc=ChecklistQuantity()+1;
									}
								}

								//  no need to continue looking thrugh guns
								t.tgindex=g.gunmax ; break;

							}

						}
					}
				}

			}
		}

	}
}

void entity_freeattachment ( void )
{
	if (  t.entityelement[t.e].attachmentobj>0 ) 
	{
		if (  ObjectExist(t.entityelement[t.e].attachmentobj) == 1 ) 
		{
			HideObject (  t.entityelement[t.e].attachmentobj );
		}
	}
}

void entity_controlattachments ( void )
{
	// ensure attachments are updated and visible
	t.tcharacterobj=t.entityelement[t.e].obj;
	t.tobj=t.entityelement[t.e].attachmentobj;
	if ( t.tobj>0 && t.tcharacterobj>0 ) 
	{
		//  Added check for visibility, as attachment will be invisible once it's been picked up
		if ( ObjectExist(t.tobj) == 1 && ObjectExist(t.tcharacterobj) == 1 && GetVisible(t.tobj) == 1 ) 
		{
			//  manual position of gun attachment
			if ( t.entityelement[t.e].beenkilled == 0 && (t.entityelement[t.e].active != 0 || t.entityelement[t.e].eleprof.phyalways != 0) ) 
			{
				t.limbpx_f=ObjectPositionX(t.entityelement[t.e].obj);
				t.limbpy_f=ObjectPositionY(t.entityelement[t.e].obj);
				t.limbpz_f=ObjectPositionZ(t.entityelement[t.e].obj);
				if (  t.entityelement[t.e].obj>0 ) 
				{
					if (  ObjectExist(t.entityelement[t.e].obj) == 1 ) 
					{
						t.tentid=t.entityelement[t.e].bankindex;
						if (  t.entityprofile[t.tentid].firespotlimb >= 0 ) 
						{
							if (  LimbExist(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb) == 1 ) 
							{
								// position of entity hand where gun rests
								// expensive process, so only do when close enough to see one-frame lag
								// be aware that this ADVANCES THE ANIM FRAME ONE STEP - INTENDED??!
								CalculateObjectBoundsEx (  t.entityelement[t.e].obj,1 );
								t.limbpx_f=LimbPositionX(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								t.limbpy_f=LimbPositionY(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								t.limbpz_f=LimbPositionZ(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								t.limbax_f=LimbDirectionX(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								t.limbay_f=LimbDirectionY(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								t.limbaz_f=LimbDirectionZ(t.entityelement[t.e].obj,t.entityprofile[t.tentid].firespotlimb);
								RotateObject (  t.tobj,t.limbax_f,t.limbay_f,t.limbaz_f );
								ScaleObject (  t.tobj,85,85,85 );
							}
						}
					}
				}
				PositionObject (  t.tobj,t.limbpx_f,t.limbpy_f,t.limbpz_f );
				ShowObject (  t.tobj );
				EnableObjectZDepth(t.obj); // PE:
			}
		}
	}
}

void entity_monitorattachments ( void )
{
	//  handle when player picks up ammo from dead enemies. Assumes ammo pool 1 is the default pool we want to update
	if (  t.playercontrol.thirdperson.enabled == 0 ) 
	{
		t.tobj=t.entityelement[t.e].attachmentobj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1 && GetVisible(t.tobj) == 1 ) 
			{
				if (  t.entityelement[t.e].health <= 0 && t.entityelement[t.e].eleprof.cantakeweapon != 0 ) 
				{
					t.txDist_f = CameraPositionX(0) - ObjectPositionX(t.tobj);
					t.tyDist_f = CameraPositionY(0) - ObjectPositionY(t.tobj);
					t.tzDist_f = CameraPositionZ(0) - ObjectPositionZ(t.tobj);
					t.ttD_f = t.txDist_f * t.txDist_f + t.tyDist_f * t.tyDist_f + t.tzDist_f * t.tzDist_f;
					if (  t.ttD_f < 100 * 100 ) 
					{
						//  collect whole weapon, not just ammo
						t.weaponindex=t.entityelement[t.e].eleprof.hasweapon;
						if (  t.weaponindex>0 ) 
						{
							//  if weapon not yet part of players inventory, prompt to press E
							t.gotweapon=0;
							for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
							{
								if (  t.weaponslot[t.ws].got == t.weaponindex  )  t.gotweapon = t.ws;
							}
							int iPressedE = KeyState(g.keymap[18]);
							if ( g.gxbox == 1 ) 
							{
								if ( JoystickFireD() == 1 )  
									iPressedE = 1;
							}
							if (  t.gotweapon == 0 && iPressedE == 0 ) 
							{
								//  prompt user to press E
								t.luaglobal.scriptprompttime=Timer()+200;
								if ( g.gxbox == 0 )
									t.luaglobal.scriptprompt_s="Press E to pick up weapon";
								else
									t.luaglobal.scriptprompt_s="Press Y Button to pick up weapon";
							}
							else
							{
								//  got weapon already, so collect ammo
								t.tqty=t.entityelement[t.e].eleprof.quantity;
								physics_player_addweapon ( );
								playinternalsoundinterupt(t.playercontrol.soundstartindex+16);
								//  eventually this should be done in script
								t.luaglobal.scriptprompttime=Timer()+3000;
								t.luaglobal.scriptprompt_s="Collected enemy ammo";
								//  weapon collected so remove from physics and visible world
								ODEDestroyObject (  t.tobj );
								HideObject (  t.tobj );
							}
						}
					}
				}
			}
		}
	}
}

void entity_converttoclone ( void )
{
	// takes tte
	if ( t.entityelement[t.tte].isclone == 0 ) 
	{
		t.tttentid = t.entityelement[t.tte].bankindex;
		if ( t.entityprofile[t.tttentid].isebe == 0 )
		{
			t.tobj=t.entityelement[t.tte].obj;
			if (  t.tobj>0 ) 
			{
				if (  ObjectExist(t.tobj) == 1 ) 
				{
					t.tstorevis=GetVisible(t.tobj);
					DeleteObject ( t.tobj );
					t.ttsourceobj=g.entitybankoffset+t.tttentid;
					if (  t.tentityconverttoclonenotshared == 1 ) 
					{
						//  need a freshly created when third person character needs new shader applied
						CloneObject (  t.tobj,t.ttsourceobj,0 );
						SetObjectEffect (  t.tobj,0 );
					}
					else
					{
						CloneObject (  t.tobj,t.ttsourceobj,1 );
					}
					//  restore any radius settings the original object might have had
					SetSphereRadius (  t.tobj,-1 );
					t.entityelement[t.tte].isclone=1;
					t.tentid=t.entityelement[t.tte].bankindex;
					entity_prepareobj ( );
					entity_positionandscale ( );
					if (  t.tstorevis == 0  )  HideObject (  t.tobj );
					if ( t.entityprofile[t.tttentid].addhandlelimb == 0 )
					{
						// 301115 - override parent LOD distance with LODModifier
						entity_calculateentityLODdistances ( t.tttentid, t.tobj, t.entityelement[t.tte].eleprof.lodmodifier );
					}
				}
			}
		}
	}
}

void entity_converttoclonetransparent ( void )
{
	// used in IDE editor to show locked entities
	entity_converttoclone ( );
	SetAlphaMappingOn ( t.tobj, 110 ); // special semi-transparent mode with draw first rendering
	//DisableObjectZWrite ( t.tobj );
}

bool entity_isuniquespecularoruv ( int ee )
{
	bool bUnique = false; 
	if ( ee != -1  ) 
	{
		// specular can be overridden per entity
		if ( t.entityelement[ee].eleprof.specularperc != 100.0f ) 
			bUnique = true;

		// UV data scroll/scale can be overridden per entity
		int eentid = t.entityelement[ee].bankindex;
		if ( t.entityprofile[eentid].uvscrollu != 0.0f 
		||   t.entityprofile[eentid].uvscrollv != 0.0f 
		||   t.entityprofile[eentid].uvscaleu != 1.0f 
		||   t.entityprofile[eentid].uvscalev != 1.0f )
			bUnique = true;
	}
	return bUnique;
}

void entity_converttoinstance ( void )
{
	// takes tte
	// 101115 - can only be an instance of CPUANIMS=0 as only pure clones should CPU animate
	if ( t.entityelement[t.tte].isclone == 1 ) 
	{
		t.tobj=t.entityelement[t.tte].obj;
		if ( t.tobj>0 ) 
		{
			// 101216 - if entity is given unique specular, must be a clone to take effect
			// 020217 - quit early if cannot make this an instance
			bool bUniqueSpecularOrUV = entity_isuniquespecularoruv ( t.tte );
			if ( ObjectExist(t.tobj) == 1 && bUniqueSpecularOrUV == false ) 
			{
				//  first remove any ragdoll
				entity_freeragdoll ( );

				//  then delete clone and recreate as instance
				t.tstorevis=GetVisible(t.tobj);
				DeleteObject (  t.tobj );
				t.ttsourceobj=g.entitybankoffset+t.entityelement[t.tte].bankindex;
				if ( t.entityprofile[t.entityelement[t.tte].bankindex].cpuanims==0 )
					InstanceObject (  t.tobj,t.ttsourceobj );
				else
					CloneObject ( t.tobj, t.ttsourceobj, 1 );

				//  restore any radius settings the original object might have had
				SetSphereRadius (  t.tobj,-1 );
				t.entityelement[t.tte].isclone=0;
				entity_prepareobj ( );
				entity_positionandscale ( );
				if (  t.tstorevis == 0  )  HideObject (  t.tobj );
				characterkit_convertCloneInstance ( );
				if ( t.entityprofile[t.tentid].addhandlelimb == 0 )
				{
					// 301115 - override parent LOD distance with LODModifier
					entity_calculateentityLODdistances ( t.tentid, t.tobj, t.entityelement[t.tte].eleprof.lodmodifier );
				}
			}
		}
	}
}

void entity_createobj ( void )
{
	//  takes OBJ, TUPDATEE, TENDIT
	t.sourceobj=g.entitybankoffset+t.tentid;
	if (  ObjectExist(t.sourceobj) == 1 ) 
	{
		if (  t.tupdatee != -1  )  t.entityelement[t.tupdatee].profileobj = t.sourceobj;
		EnableObjectZWrite (  t.sourceobj );
		// 101216 - if entity is given unique specular, must be a clone to take effect
		//bool bUniqueSpecular = false; 
		//if (  t.tupdatee != -1  ) 
		//	if ( t.entityelement[t.tupdatee].eleprof.specularperc != 100.0f ) 
		//		bUniqueSpecular = true;
		//  Create new object
		bool bUniqueSpecular = entity_isuniquespecularoruv ( t.tupdatee );
		if ( t.entityprofile[t.tentid].ismarker != 0 || t.entityprofile[t.tentid].cpuanims != 0 || t.entityprofile[t.gridentity].isebe != 0 || bUniqueSpecular == true ) 
		{
			CloneObject (  t.obj,t.sourceobj,1 );
			if (  t.tupdatee != -1  )  t.entityelement[t.tupdatee].isclone = 1;
		}
		else
		{
			InstanceObject (  t.obj,t.sourceobj );
			if (  t.tupdatee != -1  )  t.entityelement[t.tupdatee].isclone = 0;
		}
		//  restore any radius settings the original object might have had
		SetSphereRadius (  t.obj,-1 );

		//  ensure new object ONLY interacts with main camera and shadow camera
		//  (until postprocess masks kick in)
		if (  t.entityprofile[t.tentid].ismarker != 0 ) 
		{
			SetObjectMask (  t.obj,1 );
		}
		else
		{
			SetObjectMask (  t.obj,1+(1<<31) );
		}

		//  initially prep any objects with animation
		if (  GetNumberOfFrames(t.obj)>0 ) 
		{
			SetObjectFrame (  t.obj,0  ); LoopObject (  t.obj  ); StopObject (  t.obj );
		}

		//  allow first animation
		if (  t.entityprofile[t.tentid].animmax>0 && t.entityprofile[t.tentid].playanimineditor>0 ) 
		{
			// animation chosen
			t.q=t.entityprofile[t.tentid].playanimineditor-1;

			// play through "parent object" (ONE OFF)
			LoopObject ( t.sourceobj, t.entityanim[t.tentid][t.q].start,t.entityanim[t.tentid][t.q].finish );

			// 060217 - and clone object if so
			if ( GetNumberOfFrames(t.obj) > 0 ) 
			{
				LoopObject ( t.obj, t.entityanim[t.tentid][t.q].start, t.entityanim[t.tentid][t.q].finish );
			}
		}

		//  SetObject (  properties )
		t.tobj=t.obj ; t.tte=t.tupdatee ; entity_prepareobj ( );

		//  check if a character creator entity
		if (  t.entityprofile[t.tentid].ischaractercreator  ==  1 ) 
		{
			t.tccSetTentid = 1;
			characterkit_addEntityToMap ( );
		}

		if (  t.tupdatee != -1  ) 
		{
			if ( t.entityprofile[t.tentid].addhandlelimb == 0 )
			{
				// 301115 - override parent LOD distance with LODModifier
				entity_calculateentityLODdistances ( t.tentid, t.tobj, t.entityelement[t.tupdatee].eleprof.lodmodifier );
			}
		}
	}
	else
	{
		//  debug sphere when object not found
		MakeObjectCube (  t.obj,25 );
		SetObjectCollisionOff (  t.obj );
		SetAlphaMappingOn (  t.obj,100 );
	}
}

void entity_prepareobj ( void )
{
	//  takes tte, tobj and tentid
	//  called after entity object clone or instance created (during _entity_createobj and also _entity_converttoclone)
	if (  ObjectExist(t.tobj) == 1 ) 
	{
		//  ensure new object ONLY interacts with main camera and shadow camera
		//  (until postprocess masks kick in)
		if (  t.entityprofile[t.tentid].ismarker != 0 ) 
		{
			SetObjectMask (  t.tobj,1 );
		}
		else
		{
			SetObjectMask (  t.tobj,1+(1<<31) );
		}

		// specific object mask settings
		if ( t.tte > 0 ) visuals_updatespecificobjectmasks ( t.tte, t.tobj );

		//  object properties
		if ( t.entityprofile[t.tentid].ismarker != 0 ) 
		{
			//  special setup for marker objects
			if ( t.entityprofile[t.tentid].ismarker != 2 )
			{
				// 160616 - but only if not light markers which are now real 3D models for better grabbing
				SetObjectTransparency ( t.tobj,2 );
				SetObjectCull ( t.tobj,1 );
				DisableObjectZDepth ( t.tobj );
				DisableObjectZWrite ( t.tobj );
				DisableObjectZRead ( t.tobj );
			}
		}
		else
		{
			if (  t.entityprofile[t.tentid].cullmode != 0 ) 
			{
				SetObjectCull (  t.tobj,0 );
			}
			else
			{
				SetObjectCull (  t.tobj,1 );
			}
		}

		//  object animation
		entity_resettodefaultanimation ( );

		//  object rotation and scale
		if (  t.entityprofile[t.tentid].fixnewy != 0 ) 
		{
			RotateObject (  t.tobj,0,t.entityprofile[t.tentid].fixnewy,0 );
			FixObjectPivot (  t.tobj );
		}
		if (  t.entityprofile[t.tentid].scale != 0  )  ScaleObject (  t.tobj,t.entityprofile[t.tentid].scale,t.entityprofile[t.tentid].scale,t.entityprofile[t.tentid].scale );

		// 091115 - after scaling, ensure LOD is a reflection of overall object size (so LARGE buildings not instantly go to LOD2)
		if (  t.entityprofile[t.tentid].ismarker == 0 ) 
		{
			// 051115 - only if not using limb visibility for hiding decal arrow
			if ( t.entityprofile[t.tentid].addhandlelimb==0 )
			{
				//  set LOD levels for object
				entity_calculateentityLODdistances ( t.tentid, t.tobj, 0 );
			}
		}

		//  no collision and full alpha multiplier
		SetObjectCollisionOff ( t.tobj );
		SetAlphaMappingOn ( t.tobj,100 );

		//  set transparency mode (after 'set alpha mapping on' as it messes with transparency flag)
		if (  t.entityprofile[t.tentid].ismarker == 0 ) 
		{
			SetObjectTransparency ( t.tobj,t.entityprofile[t.tentid].transparency );
			if (  t.entityprofile[t.tentid].transparency >= 2 ) 
			{
				//  set Z write for transparency mode 2 and above
				//Dave - commented this out because otherwise grass is drawn over things like gates
				//DisableObjectZWrite (  t.tobj );
			}
		}

		// 140616 - new mode which can turn entity into HUD-weapon style render order
		if ( t.entityprofile[t.tentid].zdepth == 0 ) 
		{
			DisableObjectZDepthEx ( t.tobj, 1 );
		}

		// 281116 - set specular (new internal property that ties to 'SpecularOverride' shader constant)
		// BUT we will use TTE so can get the per entity ELEPROF state when creating object!
		// also calls this in lightmapper just after cloned parent obj 9see Lightmaping.cpp)
		if ( t.entityprofile[t.tentid].ismarker == 0 && t.tte > 0 )
		{
			SetObjectSpecularPower ( t.tobj, t.entityelement[t.tte].eleprof.specularperc / 100.0f );
		}

		// apply the scrolls cale uv data values for the shader use later on
		if ( t.entityprofile[t.tentid].uvscrollu != 0.0f 
		||   t.entityprofile[t.tentid].uvscrollv != 0.0f 
		||   t.entityprofile[t.tentid].uvscaleu != 1.0f 
		||   t.entityprofile[t.tentid].uvscalev != 1.0f )
		{
			SetObjectScrollScaleUV ( t.tobj, t.entityprofile[t.tentid].uvscrollu, t.entityprofile[t.tentid].uvscrollv, t.entityprofile[t.tentid].uvscaleu, t.entityprofile[t.tentid].uvscalev );
		}
	}
}

void entity_calculateentityLODdistances ( int tentid, int tobj, int iModifier )
{
	float fLODModifier = (100+iModifier)/100.0f;
	if ( t.entityprofile[tentid].lod1distance==0 )
	{
		// default LOD distances a product of scale of object
		float fRelativeScale = ObjectSize ( tobj, 1 ) / 100.0f;
		SetObjectLOD ( tobj, 1, 400.0f * fRelativeScale * fLODModifier );
		SetObjectLOD ( tobj, 2, 800.0f * fRelativeScale * fLODModifier );
	}
	else
	{
		// otherwise its specified by the FPE
		SetObjectLOD ( tobj, 1, (float)t.entityprofile[tentid].lod1distance * fLODModifier );
		SetObjectLOD ( tobj, 2, (float)t.entityprofile[tentid].lod2distance * fLODModifier );
	}
}

void entity_setupcharobjsettings ( void )
{
	//  unique extra setup for character objects, takes charanimstate, obj
	if (  t.obj>0 ) 
	{
		if (  ObjectExist(t.obj) == 1 ) 
		{
			PositionObject (  t.obj,t.entityelement[t.charanimstate.e].x,t.entityelement[t.charanimstate.e].y,t.entityelement[t.charanimstate.e].z );
			RotateObject (  t.obj,t.entityelement[t.charanimstate.e].rx,t.entityelement[t.charanimstate.e].ry,t.entityelement[t.charanimstate.e].rz );
			ScaleObject (  t.obj,100+t.entityelement[t.charanimstate.e].scalex,100+t.entityelement[t.charanimstate.e].scaley,100+t.entityelement[t.charanimstate.e].scalez );
		}
	}
return;

}

void entity_resettodefaultanimation ( void )
{
	//  takes tee, tobj, tentid
	if (  t.tte != -1 ) 
	{
		if (  t.entityelement[t.tte].isclone == 1 ) 
		{
			//  CLONE
			if (  GetNumberOfFrames(t.tobj)>0 ) 
			{
				SetObjectFrame (  t.tobj,0  ); LoopObject (  t.tobj  ); StopObject (  t.tobj );
			}
			if (  t.entityelement[t.tte].staticflag == 1 ) 
			{
				//  do not animate if marked as static
			}
			else
			{
				if (  t.entityprofile[t.tentid].animmax>0 && t.entityprofile[t.tentid].playanimineditor>0 ) 
				{
					t.q=t.entityprofile[t.tentid].playanimineditor-1;
					LoopObject (  t.tobj,t.entityanim[t.tentid][t.q].start,t.entityanim[t.tentid][t.q].finish );
				}
			}
		}
		else
		{
			//  INSTANCE - no self-animation
		}
	}
return;

}

void entity_positionandscale ( void )
{
	//  takes tobj,tte,tentid
	PositionObject (  t.tobj,t.entityelement[t.tte].x,t.entityelement[t.tte].y,t.entityelement[t.tte].z );
	RotateObject (  t.tobj,t.entityelement[t.tte].rx,t.entityelement[t.tte].ry,t.entityelement[t.tte].rz );
//  `tescale=entityprofile(tentid).scale

//  `if tescale>0

	//ScaleObject (  tobj,tescale+entityelement(tte).scalex,tescale+entityelement(tte).scaley,tescale+entityelement(tte).scalez )
//  `else

	//ScaleObject (  tobj,100+entityelement(tte).scalex,100+entityelement(tte).scaley,100+entityelement(tte).scalez )
//  `endif

	ScaleObject (  t.tobj,100+t.entityelement[t.tte].scalex,100+t.entityelement[t.tte].scaley,100+t.entityelement[t.tte].scalez );
	ShowObject (  t.tobj );
	//  ensure all transparent static objects are removed from 'intersect all' consideration
	t.tokay=0;
	if (  t.entityelement[t.tte].staticflag == 1 ) 
	{
		if (  t.entityprofile[t.entityelement[t.tte].bankindex].canseethrough == 1 ) 
		{
			t.tokay=1;
		}
	}
	if (  t.entityprofile[t.entityelement[t.tte].bankindex].ischaracter == 0 ) 
	{
		if (  t.entityprofile[t.entityelement[t.tte].bankindex].collisionmode == 11  )  t.tokay = 1;
	}
	if (  t.tokay == 1 ) 
	{
		SetObjectCollisionProperty (  t.tobj,1 );
	}
return;

}

void entity_updateentityobj ( void )
{
	//  special mode which intercepts non-static entities and replaces with blanks
	if ( t.lightmapper.onlyloadstaticentitiesduringlightmapper == 1 ) 
	{
		//  eliminate entities that NEVER get lightmapped
		if (  t.entityelement[t.tupdatee].staticflag == 0 ) 
		{
			t.entityelement[t.tupdatee].bankindex=0;
			t.entityelement[t.tupdatee].obj=0;
		}
	}

	// Create/replace/remove olay object to reflect (olayindex,ti)
	t.tentid=t.entityelement[t.tupdatee].bankindex;
	if (  t.tentid != 0 ) 
	{
		t.obj=t.entityelement[t.tupdatee].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
			t.entityelement[t.tupdatee].obj=0;
		}
		if (  t.entityelement[t.tupdatee].obj == 0 ) 
		{
			//  find free object
			t.obj=g.entityviewcurrentobj;
			if (  ObjectExist(t.obj) == 1 ) 
			{
				while ( ObjectExist(t.obj)==1 ) ++t.obj;
				g.entityviewcurrentobj=t.obj;
			}
			if (  g.entityviewcurrentobj>g.entityviewendobj ) 
			{
				g.entityviewendobj=g.entityviewcurrentobj;
			}
			g.editorresourcecounterpacer=1;
			//  create object
			entity_createobj ( );
			t.tobj=t.obj ; t.tte=t.tupdatee ; entity_positionandscale ( );
			t.entityelement[t.tupdatee].obj=t.obj;

			//  also punch terrain to flatten area where entity is placed
			if (  g.gridlayershowsingle == 0 ) 
			{
				//  only when not in INSIDE edit mode
				if (  t.gridedit.autoflatten == 1 && t.entityprofile[t.tentid].ismarker == 0 ) 
				{
					t.terrain.terrainpainteroneshot=1;
					t.terrain.X_f=ObjectPositionX(t.obj);
					t.terrain.Y_f=ObjectPositionZ(t.obj);
					t.terrain.shapeA_f=ObjectAngleY(t.obj);
					t.terrain.shapeWidth_f=ObjectSizeX(t.obj,1);
					t.terrain.shapeLong_f=ObjectSizeZ(t.obj,1);
					t.terrain.shapeHeight_f=ObjectPositionY(t.obj);
				}
			}

		}
	}
	if (  t.tentid == 0 ) 
	{
		t.obj=t.entityelement[t.tupdatee].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1  )  DeleteObject (  t.obj );
		}
		t.entityelement[t.tupdatee].obj=0;
	}
}

//----------------------------------------------------
//--- GAMEGURU - M-LUA-Entity
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Externs
extern void entity_refreshelementforuse ( void );

// 
//  LUA Entity Commands
// 

// General

void entity_lua_starttimer ( void )
{
	t.entityelement[t.e].etimer=Timer();
	t.entityelement[t.e].lua.flagschanged=1;
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_StartTimer,t.e,t.v );
	}
}

void entity_lua_findcharanimstate ( void )
{
	//  NOTE; This can be done as a pre-process and stored in the entityelement array
	//  NOTE; for instant retrieval (should save thousands of cycles per loop)
	//  takes e, returns tcharanimindex
	t.tcharanimindex=-1;
	for ( t.ttcharanimindex = 1 ; t.ttcharanimindex<=  g.charanimindexmax; t.ttcharanimindex++ )
	{
		if (  t.charanimstates[t.ttcharanimindex].e == t.e ) { t.tcharanimindex = t.ttcharanimindex  ; break; }
	}
	if (  t.tcharanimindex != -1  )  t.charanimstate = t.charanimstates[t.tcharanimindex];
}

void entity_lua_destroy ( void )
{
	// remove entity from level
	if ( t.entityelement[t.e].active != 0 )
	{
		t.entityelement[t.e].destroyme = 1;
		if (  t.game.runasmultiplayer ==  1 ) 
		{
			SteamDestroy (  t.e );
		}
	}
}

void entity_lua_collisionon ( void )
{
	if ( t.e > g.entityelementlist  )  return;
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if ( ObjectExist(t.obj) == 1 ) 
		{
			t.entid=t.entityelement[t.e].bankindex;
			if ( t.entityelement[t.e].usingphysicsnow == 0 ) 
			{
				// create physics for entity if required
				physics_prepareentityforphysics ( );

				//  if a character, special physics setup
				t.ttte=t.e ; entity_find_charanimindex_fromttte ( );
				if (  t.tcharanimindex>0 ) 
				{
					t.tphyobj=t.entityelement[t.e].obj;
					physics_setupcharacter ( );
				}

				// 240217 - and activate physics for all, not just characters
				t.entityelement[t.e].usingphysicsnow=1;
			}
			if ( t.entityelement[t.e].doorobsactive == 0 )
			{
				// if the LUA script name starts with "DOOR", the AI system will treat it as a blocking obstacle
				// so allowing path finder to ignore gaps that would have normally been used should the door be open
				if ( strnicmp ( t.entityprofile[t.entid].aimain_s.Get(), "door", 4) == NULL ) 
				{
					t.tobj=t.obj ; darkai_adddoor ( );
					t.entityelement[t.e].doorobsactive = 1;
				}
			}
			//  ensure all transparent objects are removed from 'intersect all' consideration
			if (  t.entityprofile[t.entid].canseethrough == 1 || (t.entityprofile[t.entid].collisionmode == 11 && t.entityprofile[t.entid].ischaracter == 0) ) 
			{
				SetObjectCollisionProperty (  t.obj,1 );
			}
			else
			{
				SetObjectCollisionProperty (  t.obj,0 );
			}
		}
	}
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_CollisionOn,t.e,t.v );
	}
}

void entity_lua_collisionoff ( void )
{
	t.tobj=t.entityelement[t.e].obj;
	if (  t.tobj>0 ) 
	{
		if (  ObjectExist(t.tobj) == 1 ) 
		{
			t.entid=t.entityelement[t.e].bankindex;
			if (  t.entityelement[t.e].usingphysicsnow != 0 ) 
			{
				t.tphyobj=t.tobj ; physics_disableobject ( );
				t.entityelement[t.e].usingphysicsnow=0;
			}
			if ( t.entityelement[t.e].doorobsactive == 1 )
			{
				if ( strnicmp ( t.entityprofile[t.entid].aimain_s.Get(), "door", 4) == NULL ) 
				{
					darkai_removedoor ( );
					t.entityelement[t.e].doorobsactive = 0;
				}
			}
			SetObjectCollisionProperty (  t.tobj,1 );
		}
	}
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_CollisionOff,t.e,t.v );
	}
}

void entity_lua_getentityplrvisible ( void )
{
	t.tobj=t.entityelement[t.e].obj;
	if (  t.tobj>0 ) 
	{
		if (  ObjectExist(t.tobj) == 1 ) 
		{
			entity_gettrueplayerpos( );
			t.tx1_f=t.tcamerapositionx_f;
			t.ty1_f=t.tcamerapositiony_f;
			t.tz1_f=t.tcamerapositionz_f;
			t.tx2_f=ObjectPositionX(t.tobj);
			t.ty2_f=ObjectPositionY(t.tobj)+20;//070918 - extra for getting to eye position
			t.tz2_f=ObjectPositionZ(t.tobj);
			t.tsrcobj=g.entitybankoffset+t.entityelement[t.e].bankindex;
			if (  ObjectExist(t.tsrcobj) == 1 ) 
			{
				t.ty2_f=t.ty2_f+ObjectSizeY(t.tsrcobj,1)*0.5;
			}

			if (g.lightmappedobjectoffset >= g.lightmappedobjectoffsetfinish)
				t.ttt = IntersectAll(85000, 85000 + g.merged_new_objects - 1, 0, 0, 0, 0, 0, 0, -123);
			else
				t.ttt=IntersectAll(g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,t.tx1_f,t.ty1_f,t.tz1_f,0,0,0,-123);


			//PE: door.lua , we are hitting t.entityelement[t.playercontrol.thirdperson.charactere].obj
			//PE: Disable t.entityelement[t.playercontrol.thirdperson.charactere].obj from check.
			//https://github.com/TheGameCreators/GameGuruRepo/issues/619

			bool reneableThirdperson = false;
			sObject* pObjectThirdperson = NULL;
			if (t.playercontrol.thirdperson.enabled == 1) {
				//Hide TPP or we will hit it.
				int thirdpersonObj = t.entityelement[t.playercontrol.thirdperson.charactere].obj;
				if (thirdpersonObj > 0) {
					pObjectThirdperson = g_ObjectList[thirdpersonObj];
					if (pObjectThirdperson->bVisible) {
						pObjectThirdperson->bVisible = false;
						reneableThirdperson = true;
					}
				}
			}

			if ( IntersectAll(g.entityviewstartobj,g.entityviewendobj,t.tx1_f,t.ty1_f,t.tz1_f,t.tx2_f,t.ty2_f,t.tz2_f,t.tobj) > 0 )
			{
 					t.entityelement[t.e].plrvisible=0;
			}
			else
			{
				t.entityelement[t.e].plrvisible=1;
			}

			if (reneableThirdperson && pObjectThirdperson) {
				pObjectThirdperson->bVisible = true;
			}

			t.entityelement[t.e].lua.flagschanged=1;
		}
	}
}

void entity_lua_getentityinzone ( void )
{
	// If entity is zone, determine if ANY OTHER entity is inside it
	t.waypointindex=t.entityelement[t.e].eleprof.trigger.waypointzoneindex;
	if (  t.waypointindex>0 ) 
	{
		if ( t.waypoint[t.waypointindex].active == 1 ) 
		{
			if ( t.waypoint[t.waypointindex].style == 2 ) 
			{
				for ( int othere = 1; othere <= g.entityelementlist; othere++ )
				{
					if ( othere != t.e )
					{
						t.tpointx_f = t.entityelement[othere].x;
						t.tpointz_f = t.entityelement[othere].z;
						t.tokay = 0; waypoint_ispointinzone ( );
						if ( t.tokay != 0 )
						{
							t.entityelement[t.e].lua.entityinzone = othere;
							t.entityelement[t.e].lua.flagschanged = 1;
						}
					}
				}
			}
		}
	}
}

void entity_lua_hide ( void )
{
	t.tobj=t.entityelement[t.e].obj;
	if (  t.tobj>0 ) 
	{
		if (  ObjectExist(t.tobj) == 1 ) 
		{
			HideObject (  t.tobj );
		}
	}
	t.tattobj=t.entityelement[t.e].attachmentobj;
	if (  t.tattobj>0 ) 
	{
		if (  ObjectExist(t.tattobj) == 1 ) 
		{
			HideObject (  t.tattobj );
		}
	}
}

void entity_lua_show ( void )
{
	if ( t.entityelement[t.e].active != 0 )
	{
		t.tobj=t.entityelement[t.e].obj;
		if (  t.tobj>0 ) 
		{
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				ShowObject (  t.tobj );
			}
		}
		t.tattobj=t.entityelement[t.e].attachmentobj;
		if (  t.tattobj>0 ) 
		{
			if (  ObjectExist(t.tattobj) == 1 ) 
			{
				ShowObject (  t.tattobj );
			}
		}
	}
}

void entity_lua_spawn_core ( void )
{
	//  resurrect dead entity
	t.tte = t.e ; entity_freeragdoll ( );
	t.ttentid = t.entityelement[t.e].bankindex;

	//  restore attached object
	t.tattobj=t.entityelement[t.e].attachmentobj;
	if (  t.tattobj>0 ) { ODEDestroyObject (  t.tattobj  ) ; ShowObject (  t.tattobj ); }

	//  restore entity and it's AI
	t.entityelement[t.e].active=1;
	t.entityelement[t.e].health=t.entityprofile[t.ttentid].strength;
	if ( Len(t.entityelement[t.e].eleprof.aimainname_s.Get())>1 ) 
	{
		t.entityelement[t.e].eleprof.aimain=1;
		t.entityelement[t.e].eleprof.aipreexit=-1;
	}

	//  restore object of entity
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if (  ObjectExist(t.obj) == 1 ) 
		{
			t.entid=t.ttentid;
			if (  t.entityprofile[t.entid].ismarker == 0 ) 
			{
				t.tte=t.e ; entity_converttoinstance ( );
				PositionObject (  t.obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
				RotateObject (  t.obj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
				t.tentid=t.entid ; t.tte=t.e ; t.tobj=t.obj  ; entity_resettodefaultanimation ( );
				ShowObject (  t.obj );
			}
		}
	}

	// 051115 - only if not using limb visibility for hiding decal arrow
	if ( t.entityprofile[t.ttentid].addhandlelimb==0 )
	{
		//  restore LOD levels for object (ragdoll removed LOD feature)
		entity_calculateentityLODdistances ( t.ttentid, t.obj, t.entityelement[t.e].eleprof.lodmodifier );
	}

	//  restore if character
	if (  t.entityprofile[t.ttentid].ischaracter == 1 ) 
	{
		for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
		{
			if ( t.charanimstates[g.charanimindex].originale == t.e ) 
			{
				// first destroy any part-leftovers of character
				t.charanimstate = t.charanimstates[g.charanimindex];
				t.entityelement[t.e].ragdollified = 1; // entity reload might have wiped ragdoll state
				darkai_character_remove_charpart ( );
				t.charanimstates[g.charanimindex] = t.charanimstate;

				//  create new AI for this entity
				t.charanimstates[g.charanimindex].e = t.e;
				PositionObject ( t.charanimstates[g.charanimindex].obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
				darkai_setupcharacter ( );

				// reapply shader in case object was refreshed
				SetObjectEffect( g.entitybankoffset+t.entityelement[t.e].bankindex , t.entityprofile[t.entid].usingeffect);

				// finally initialise AI (see below)
				lua_initscript();

				// give resurrected characters some immunity to start with
				t.entityelement[t.e].briefimmunity = 100;

				//PE: Physics was lost after several spawn, and you cant shoot char.
				//PE: Explode state dont reset.
				//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/429
				//PE: Make sure physics is setup again, and if char exploded is reset.
				entity_lua_collisionon();
				t.entityelement[t.e].collected = 0;
				t.entityelement[t.e].explodefusetime = 0;

				//PE: 21-06-2019 for Andy to test , just enable z when spawn.
				if (t.entityelement[t.e].obj > 0) {
					EnableObjectZWrite(t.entityelement[t.e].obj);
					EnableObjectZDepth(t.entityelement[t.e].obj);
				}
				// found character, no need to continue
				break;
			}
		}
	}
	else
	{
		// 120916 - ensure collision restored if not character (exploding barrels could be walked through)
		// 120916 - seems collisionON sets SetObjectCollisionProperty to 0 (needed for exploding barrel)
		//if ( t.entityelement[t.e].eleprof.explodable  ==  0 ) 
		//{
		entity_lua_collisionon ( );
		//}
		//else
		//{
		//	t.tphyobj = t.entityelement[t.e].obj;
		//	t.entid = t.ttentid;
		//	physics_setupobject ( );
		//}
		t.entityelement[t.e].collected = 0;
		t.entityelement[t.e].explodefusetime = 0;
	}
	
	//  trigger LUA to update properties for this entity
	t.entityelement[t.e].lua.flagschanged=1;
}

void entity_lua_spawn ( void )
{
	t.tokay=1;
	t.ttentid=t.entityelement[t.e].bankindex;
	if ( t.game.runasmultiplayer == 1 && g.mp.coop == 0 && ( t.entityprofile[t.ttentid].ischaracter == 1 || t.entityelement[t.e].mp_isLuaChar  ==  1) ) t.tokay = 0;
	if ( t.tokay == 1 ) 
	{
		if ( t.entityelement[t.e].eleprof.spawnatstart == 0 ) 
		{
			//  hidden in level, revealed by spawning
			t.entityelement[t.e].eleprof.spawnatstart = 2; // 300316 - new state means been spawned this game (use .health<=0 to know if spawned and died)
			t.entityelement[t.e].active=1;
			t.entityelement[t.e].eleprof.phyalways=0;
			entity_lua_show ( );
			entity_lua_collisionon ( );

			// 080517 - now triggers INIT AGAIN when spawning
			lua_initscript();

			//PE: (2) 21-06-2019 for Andy to test,just enable z when spawn.
			if (t.entityelement[t.e].obj > 0) {
				EnableObjectZWrite(t.entityelement[t.e].obj);
				EnableObjectZDepth(t.entityelement[t.e].obj);
			}

		}
		else
		{
			// initially visible, died, and need respawning
			if ( t.entityelement[t.e].health <= 0 ) 
			{
				//Because zones can fire off constantly, lets ensure we arent adding the same thing to the queue lots of times
				if ( t.entitiesToSpawnQueue.size() > 0 )
				{
					if ( t.entitiesToSpawnQueue[t.entitiesToSpawnQueue.size() -1] == t.e ) return;
				}
				t.entitiesToSpawnQueue.push_back ( t.e );
			}
		}
	}
}

void entity_lua_setactivated ( void )
{
	t.entityelement[t.e].activated=t.v;
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_SetActivated,t.e,t.v );
	}
}

void entity_lua_resetlimbhit ( void )
{
	t.entityelement[t.e].detectedlimbhit = t.v;
}

void entity_lua_activateifused ( void )
{
	t.tstore=t.e;
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_ActivateIfUsed,t.e,t.v );
	}
	t.tifused_s=Lower(t.entityelement[t.e].eleprof.ifused_s.Get());
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if ( cstr(Lower(t.entityelement[t.e].eleprof.name_s.Get())) == t.tifused_s ) 
		{
			//  set activate flag
			t.entityelement[t.e].activated=1;
			t.entityelement[t.e].lua.flagschanged=1;

			//  also spawn if target entity not yet spawned
			if ( t.entityelement[t.e].eleprof.spawnatstart == 0 ) 
			{
				t.entitiesToActivateQueue.push_back ( t.e );	
			}
		}
	}
	t.e=t.tstore;
}

int lastActiveTime = 0;

//Activate from the queue
void entity_lua_activateifusedfromqueue ( void )
{
	if ( t.entitiesToActivateQueue.size() == 0 ) return;

	if ( Timer() - lastActiveTime < 70 ) return;

	lastActiveTime = Timer();

	t.tstore=t.e;
	t.e = t.entitiesToActivateQueue.back();
	t.entitiesToActivateQueue.pop_back();
	t.entitiesActivatedForLua.push_back ( t.e );
	t.delayOneFrameForActivatedLua = 1;
	t.tstore=t.e;
	entity_lua_spawn();
	t.e=t.tstore;
}

void entity_lua_spawnifused ( void )
{
	t.tstore=t.e;
	t.tifused_s=Lower(t.entityelement[t.e].eleprof.ifused_s.Get());
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if (  cstr(Lower(t.entityelement[t.e].eleprof.name_s.Get())) == t.tifused_s ) 
		{
			t.entitiesToSpawnQueue.push_back ( t.e );
		}
	}
	t.e=t.tstore;
}

int lastSpawnedTime = 0;
void entity_lua_spawnifusedfromqueue ( void )
{
	if ( t.entitiesToSpawnQueue.size() == 0 ) return;
	if ( Timer() - lastSpawnedTime < 70 ) return;

	lastSpawnedTime = Timer();
	t.tstore=t.e;
	t.e = t.entitiesToSpawnQueue.back();
	t.entitiesToSpawnQueue.pop_back();
	entity_lua_spawn_core();
	t.e=t.tstore;
}

void entity_lua_transporttoifused ( void )
{
	//  takes e
	t.tstore=t.e;
	t.transporttoe=-1;
	t.tifused_s=Lower(t.entityelement[t.e].eleprof.ifused_s.Get());
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if (  cstr(Lower(t.entityelement[t.e].eleprof.name_s.Get())) == t.tifused_s ) 
		{
			//  transport player to this location
			t.transporttoe=t.e;
		}
	}
	t.e=t.tstore;
	if (  t.transporttoe != -1 ) 
	{
		t.freezeplayerposonly = 0;
		physics_disableplayer ( );
		t.terrain.playerx_f=t.entityelement[t.transporttoe].x;
		t.terrain.playery_f=t.entityelement[t.transporttoe].y+30;
		t.terrain.playerz_f=t.entityelement[t.transporttoe].z;
		t.terrain.playerax_f=0;
		t.terrain.playeray_f=t.entityelement[t.transporttoe].ry;
		t.terrain.playeraz_f=0;
		physics_setupplayer ( );
	}
}

void entity_lua_refreshentity ( void )
{
	// all entity data updated directly, now 
	// ensure visible entity matches data again (for reloading game)
	t.obj = t.entityelement[t.e].obj;
	if ( t.obj > 0 )
	{
		if ( ObjectExist ( t.obj ) == 1 )
		{
			// determine if should 1=destroy, 2=create, 3=createbutnotspawnedyet or 0=leave
			int iRefreshMode = 0;
			if ( t.entityelement[t.e].active == 0 )
			{
				// do not destroy if entity yet to spawn
				if ( t.entityelement[t.e].eleprof.spawnatstart == 0 )
				{
					// not spawned yet, so needs creating for future spawn
					iRefreshMode = 3;
				}
				else
				{
					if ( t.entityelement[t.e].eleprof.spawnatstart == 2 && t.entityelement[t.e].health <= 0 )
					{
						// has been spawned, and killed, should be destroyed
						iRefreshMode = 1;
					}
					else
					{
						// entity has been destroyed
						iRefreshMode = 1;
					}
				}
			}
			else
			{
				// entity needs creating
				iRefreshMode = 2;
			}

			// activate flag switchies entities on and off
			if ( iRefreshMode == 1 )
			{
				// hide in any event (also done in script)
				entity_lua_hide();

				// deactivate collision if any
				entity_lua_collisionoff();

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
			if ( iRefreshMode == 2 || iRefreshMode == 3 )
			{
				// common resets when entity is renewed
				entity_refreshelementforuse();

				// Posiiton and rotate object prior to final physics position, along with other spawning qualities
				t.ttentid = t.entityelement[t.e].bankindex;
				int iHaveCurrentHealth = t.entityelement[t.e].health;
				entity_lua_spawn_core();
				t.entityelement[t.e].health = iHaveCurrentHealth;

				// delete old physics so collision on command can renew it
				ODEDestroyObject ( t.obj );
				t.entityelement[t.e].usingphysicsnow = 0;

				// activate collision if any
				entity_lua_collisionon();

				// this entity needs to be in 'unspawned state'
				if ( iRefreshMode == 3 )
				{
					// set to spawn later in game
					t.entityelement[t.e].eleprof.spawnatstart = 0;
					t.entityelement[t.e].active=0;
					entity_lua_hide ( );
					entity_lua_collisionoff ( );
				}
			}
		}	
	}
}

void entity_lua_collected ( void )
{
	t.entityelement[t.e].collected = t.v;
	t.entityelement[t.e].lua.flagschanged = 1;
}

void entity_lua_checkpoint ( void )
{
	//  player inside zone
	waypoint_hide ( );
	//  record player position
	t.playercheckpoint.stored=1;
	t.playercheckpoint.x=CameraPositionX(0);
	t.playercheckpoint.y=CameraPositionY(0);
	t.playercheckpoint.z=CameraPositionZ(0);
	t.playercheckpoint.a=t.entityelement[t.e].ry;
	//  record all soundloops at the time
	t.playercheckpoint.soundloopcheckpointcountdown=5;
return;

}

void entity_lua_playsound ( void )
{
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		playinternal3dsound(t.tsnd,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z);
	}
	t.luaglobal.lastsoundnumber=t.tsnd;
	if ( t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_PlaySound,t.e,t.v );
	}
}

void entity_lua_playsoundifsilent ( void )
{
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		if ( SoundExist(t.tsnd) == 1 ) 
		{
			if ( SoundPlaying(t.tsnd) == 0 ) 
			{
				playinternal3dsound(t.tsnd,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z);
				t.luaglobal.lastsoundnumber=t.tsnd;
			}
		}
	}
}

void entity_lua_playnon3Dsound_core ( int iLoopMode )
{
	//  since sounds that call this will normally be 3D sounds, positioning the sound is still required. This will mean
	//  the sound playback will not be "non 3d", especially as the player moves. When process entities, if this sound
	//  is flagged as 'non 3D', we update the sound position if playing (in entity_loop). But only when the
	//  soundisnonthreedee flag is set so we don't perform unnecessary calls to SoundPlaying(x) and position
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		if ( SoundExist(t.tsnd) == 1 ) 
		{
			PositionSound (  t.tsnd,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
			SetSoundVolume (  t.tsnd,soundtruevolume(100.0) );
			if (iLoopMode == 0) {
				PlaySound(t.tsnd);
			}
			else {
				if (SoundLooping(t.tsnd) == 0) {
					LoopSound(t.tsnd);
				}
			}
			t.entityelement[t.e].soundisnonthreedee=1;
		}
	}
	t.luaglobal.lastsoundnumber=t.tsnd;
}

void entity_lua_playnon3Dsound ( void )
{
	entity_lua_playnon3Dsound_core ( 0 );
}

void entity_lua_loopnon3Dsound ( void )
{
	entity_lua_playnon3Dsound_core ( 1 );
}

void entity_lua_loopsound ( void )
{
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		if ( SoundLooping(t.tsnd) == 0 ) 
		{
			loopinternal3dsound(t.tsnd,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z);
		}
		else
		{
			// keep calling LoopSound to update entity position during game
			posinternal3dsound(t.tsnd,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z);
		}
	}
	t.luaglobal.lastsoundnumber=t.tsnd;
}

// New command to allow the setting of the active sound before other commands
void entity_lua_setsound ( void )
{
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		t.luaglobal.lastsoundnumber=t.tsnd;
	}	
}

void entity_lua_stopsound ( void )
{
	if ( t.v == 0 ) t.tsnd=t.entityelement[t.e].soundset;
	if ( t.v == 1 ) t.tsnd=t.entityelement[t.e].soundset1;
	if ( t.v == 2 ) t.tsnd=t.entityelement[t.e].soundset2;
	if ( t.v == 3 ) t.tsnd=t.entityelement[t.e].soundset3;
	if ( t.v == 4 ) t.tsnd=t.entityelement[t.e].soundset4;
	if ( t.tsnd>0 ) 
	{
		if ( SoundExist(t.tsnd) == 1 ) 
		{
			StopSound (  t.tsnd );
		}
	}
}

void entity_lua_setsoundspeed ( void )
{
	t.tsnd=t.luaglobal.lastsoundnumber;
	if ( t.tsnd>0 ) 
	{
		if ( SoundExist(t.tsnd) == 1 ) 
		{
			SetSoundSpeed (  t.tsnd,t.v );
		}
	}
}

void entity_lua_setsoundvolume ( void )
{
	t.tsnd=t.luaglobal.lastsoundnumber;
	if ( t.tsnd>0 ) 
	{
		if ( SoundExist(t.tsnd) == 1 ) 
		{
			t.tvolume_f = t.v;
			if (  t.tvolume_f<0  )  t.tvolume_f = 0;
			if (  t.tvolume_f>100  )  t.tvolume_f = 100;
			t.tvolume_f = t.tvolume_f * t.audioVolume.soundFloat;
			SetSoundVolume (  t.tsnd,t.tvolume_f );
		}
	}
}

void entity_lua_playvideonoskip ( int iNoSkipFlag )
{
	if ( t.v > 1 ) return; 
	if ( t.v == 0 ) 
	{
		t.tvideoid=t.entityelement[t.e].soundset;
	}
	else
	{
		t.tvideoid=t.entityelement[t.e].soundset1;
	}
	if ( t.tvideoid<0 ) 
	{
		t.tvideoid=abs(t.tvideoid);
		if ( t.luaglobal.lastvideonumber>0 ) 
		{
			if ( AnimationExist(t.luaglobal.lastvideonumber) == 1 ) 
			{
				StopAnimation (  t.luaglobal.lastvideonumber );
			}
		}
		if ( AnimationExist(t.tvideoid) == 1 ) 
		{
			PlayAnimation (  t.tvideoid );
			PlaceAnimation (  t.tvideoid,0,0,GetDisplayWidth(),GetDisplayHeight() );
			t.ttrackmouse=0;
			while (  AnimationPlaying(t.tvideoid) == 1 ) 
			{
				if ( iNoSkipFlag == 0 )
				{
					t.inputsys.mclick = GetFileMapDWORD( 1, 20 );
					if ( t.inputsys.mclick == 0 ) t.inputsys.mclick = MouseClick();
					if (  t.inputsys.mclick == 0 && t.ttrackmouse == 0  )  t.ttrackmouse = 1;
					if (  t.inputsys.mclick != 0 && t.ttrackmouse == 1  )  t.ttrackmouse = 2;
					if (  t.inputsys.mclick == 0 && t.ttrackmouse == 2  )  break;
				}
				Sync (  );
			}
			PlaceAnimation (  t.tvideoid,-1,-1,0,0 );
			StopAnimation (  t.tvideoid );
			t.luaglobal.lastvideonumber=t.tvideoid;
			//  clear mouse deltas when return to game
			t.tclear=MouseMoveX();
			t.tclear=MouseMoveY();
			t.tclear=GetFileMapDWORD( 1, 0 );
			t.tclear=GetFileMapDWORD( 1, 4 );
		}
	}

	// ensure video trigger does not cause low FPS message
	g.lowfpstarttimer = Timer();
}

void entity_lua_stopvideo ( void )
{
	if ( t.v > 1 ) return; 
	if ( t.v == 0 ) 
	{
		t.tvideoid=t.entityelement[t.e].soundset;
	}
	else
	{
		t.tvideoid=t.entityelement[t.e].soundset1;
	}
	if ( t.tvideoid<0 ) 
	{
		t.tvideoid=abs(t.tvideoid);
		if ( AnimationExist(t.tvideoid) == 1 ) 
		{
			StopAnimation (  t.tvideoid );
			PlaceAnimation (  t.tvideoid,-1,-1,0,0 );
		}
	}
	t.luaglobal.lastvideonumber=0;
}

void entity_lua_moveupdate ( void )
{
	//  takes v# as indicator to entity speed
	t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
}

void entity_lua_rotateupdate ( void )
{
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
		{
			//  control physics object (entity-driven)
			t.tavy_f=t.entityelement[t.e].ry-ObjectAngleY(t.obj);
			t.tavy_f=t.tavy_f*10.0;
			ODESetAngularVelocity ( t.obj, 0, t.tavy_f, 0 );
		}
		RotateObject (  t.obj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
	}
}

void entity_lua_scaleupdate ( void )
{
	t.obj=t.entityelement[t.e].obj;
	ScaleObject (  t.obj,t.entityelement[t.e].eleprof.scale,t.entityelement[t.e].eleprof.scale,t.entityelement[t.e].eleprof.scale );
}

void entity_lua_moveup ( void )
{
	t.ttv_f=(t.v_f/19.775)*g.timeelapsed_f;
	t.entityelement[t.e].nogravity=1;
	t.entityelement[t.e].y=t.entityelement[t.e].y+t.ttv_f;
	entity_lua_moveupdate ( );
}

void entity_lua_sethoverfactor ( void )
{
	t.entityelement[t.e].hoverfactoroverride = t.v_f;
	entity_lua_moveupdate ( );
}

void entity_lua_moveforward_core_nooverlap ( int te, float* pNX, float* pNZ )
{
	float fPlrDistOfMovingEntity = t.entityelement[te].plrdist;
	for ( int tcharanimindex = 1; tcharanimindex <= g.charanimindexmax; tcharanimindex++ )
	{
		int ee = t.charanimstates[tcharanimindex].e;
		if ( te != ee )
		{
			if ( t.entityelement[ee].health > 0 )
			{
				float fDX = t.entityelement[ee].x - *pNX;
				float fDZ = t.entityelement[ee].z - *pNZ;
				float fDD = sqrt ( fabs(fDX*fDX) + fabs(fDZ*fDZ) );
				if ( fDD > 0.0f && fDD < 30.0f )
				{
					fDX /= fDD;
					fDZ /= fDD;
					fDX *= 30.1f;
					fDZ *= 30.1f;
					*pNX = t.entityelement[ee].x - fDX;
					*pNZ = t.entityelement[ee].z - fDZ;
					if ( fPlrDistOfMovingEntity > t.entityelement[ee].plrdist )
					{
						// only trigger avoidance if entity moving if more distant one touching 
						t.entityelement[te].lua.dynamicavoidance = 2;
					}
					break;
				}
			}
		}
	}
}

void entity_lua_moveforward_core ( float fActualMoveUnitsOverride )
{
	// resets
	float fLastX = 0;
	float fLastZ = 0;
	float fBeforeX = 0;
	float fBeforeZ = 0;

	// can move by timer MC-indie or actual move units (from animation spine v object)
	if ( fActualMoveUnitsOverride != 0.0f )
		t.ttv_f = fActualMoveUnitsOverride;
	else
		t.ttv_f = (t.v_f/19.775) * g.timeelapsed_f;

	bool bCancelAvoidFlag = false;
	int entid = t.entityelement[t.e].bankindex;
	if ( t.entityprofile[entid].ischaracter == 1 )
	{
		// need a movement system that will NEVER enter another character radius
		fLastX = t.entityelement[t.e].x;
		fLastZ = t.entityelement[t.e].z;
		t.entityelement[t.e].x = NewXValue(t.entityelement[t.e].x,t.entityelement[t.e].ry,t.ttv_f);
		t.entityelement[t.e].z = NewZValue(t.entityelement[t.e].z,t.entityelement[t.e].ry,t.ttv_f);
		fBeforeX = t.entityelement[t.e].x;
		fBeforeZ = t.entityelement[t.e].z;
		entity_lua_moveforward_core_nooverlap ( t.e, &t.entityelement[t.e].x, &t.entityelement[t.e].z );
		if ( t.entityelement[t.e].x != fBeforeX || t.entityelement[t.e].z != fBeforeZ ) bCancelAvoidFlag = true;

		// 080317 - additional checks for AI inside zones push back to edge of zone, or fully reset position if LEFT
		int iEntityInContainerID = AIGetEntityContainer ( t.entityelement[t.e].obj );
		if ( iEntityInContainerID > 0 )
		{
			// seems the ispointinzone also fails when AI bot coming back down stairs from a floor zone!
			// darkai_staywithzone ( t.entityelement[t.e].obj, fLastX, fLastZ, &t.entityelement[t.e].x, &t.entityelement[t.e].z ); can fail, so use one below BETTER!
			//if ( waypoint_ispointinzoneex ( iEntityInContainerID, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z, 1 ) == 0 )
			//{
			//	t.entityelement[t.e].x = fLastX;
			//	t.entityelement[t.e].z = fLastZ;
			//}
		}
	}
	else
	{
		t.entityelement[t.e].x=NewXValue(t.entityelement[t.e].x,t.entityelement[t.e].ry,t.ttv_f);
		t.entityelement[t.e].z=NewZValue(t.entityelement[t.e].z,t.entityelement[t.e].ry,t.ttv_f);
	}

	// process movement of visual object (with physics) and calc avoidance state
	entity_lua_moveupdate ( );

	// if collided and moved with another character, do not use hard avoid, allow chars to smoothly resolve positions
	if ( bCancelAvoidFlag == true )
	{
		// NOTE: would this collapse the system if two enemies rubbed side by side against same dynamic obstacle?!?
		t.entityelement[t.e].lua.dynamicavoidance = 0;
	}
}

void entity_lua_moveforward ( void )
{
	entity_lua_moveforward_core ( 0.0f );
}

void entity_lua_movebackward ( void )
{
	t.ttv_f=(t.v_f/19.775)*g.timeelapsed_f;
	t.entityelement[t.e].x=NewXValue(t.entityelement[t.e].x,t.entityelement[t.e].ry,t.ttv_f*-1.0);
	t.entityelement[t.e].z=NewZValue(t.entityelement[t.e].z,t.entityelement[t.e].ry,t.ttv_f*-1.0);
	entity_lua_moveupdate ( );
}

void entity_lua_setpositionx ( void )
{
	t.entityelement[t.e].x=t.v_f;
	t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ;entity_updatepos ( );
}

void entity_lua_setpositiony ( void )
{
	t.entityelement[t.e].y=t.v_f;
	t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
}

void entity_lua_setpositionz ( void )
{
	t.entityelement[t.e].z=t.v_f;
	t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
}

void entity_lua_resetpositionx ( void )
{
	t.entityelement[t.e].x=t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		t.entityelement[t.e].x = t.v_f;
		ODESetBodyPosition (  t.tobj,t.v_f, ObjectPositionY(t.tobj), ObjectPositionZ(t.tobj) );
		PositionObject (  t.tobj, t.v_f, ObjectPositionY(t.tobj), ObjectPositionZ(t.tobj) );
	}
	else
	{
		t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
	}
	entity_lua_findcharanimstate ( );
	if ( t.tcharanimindex != -1 ) 
	{
		//t.i = t.charanimstates[g.charanimindex].obj;
		t.i = t.charanimstates[t.tcharanimindex].obj;
		AISetEntityPosition ( t.i, t.v_f, ObjectPositionY(t.tobj), ObjectPositionZ(t.tobj) );
	}
}

void entity_lua_resetpositiony ( void )
{
	t.entityelement[t.e].y=t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		t.entityelement[t.e].y = t.v_f;
		ODESetBodyPosition (  t.tobj,ObjectPositionX(t.tobj), t.v_f, ObjectPositionZ(t.tobj) );
		PositionObject (  t.tobj, ObjectPositionX(t.tobj), t.v_f, ObjectPositionZ(t.tobj) );
	}
	else
	{
		t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
	}
	entity_lua_findcharanimstate ( );
	if ( t.tcharanimindex != -1 ) 
	{
		//t.i = t.charanimstates[g.charanimindex].obj;
		t.i = t.charanimstates[t.tcharanimindex].obj;
		AISetEntityPosition ( t.i, ObjectPositionX(t.tobj), t.v_f, ObjectPositionZ(t.tobj) );
	}
}

void entity_lua_resetpositionz ( void )
{
	t.entityelement[t.e].z=t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		t.entityelement[t.e].z = t.v_f;
		ODESetBodyPosition (  t.tobj,ObjectPositionX(t.tobj), ObjectPositionY(t.tobj), t.v_f );
		PositionObject (  t.tobj, ObjectPositionX(t.tobj), ObjectPositionY(t.tobj), t.v_f );
	}
	else
	{
		t.te=t.e ; t.tv_f=t.v_f ; g.charanimindex=0  ; entity_updatepos ( );
	}
	entity_lua_findcharanimstate ( );
	if ( t.tcharanimindex != -1 ) 
	{
		//t.i = t.charanimstates[g.charanimindex].obj;
		t.i = t.charanimstates[t.tcharanimindex].obj;
		AISetEntityPosition ( t.i, ObjectPositionX(t.tobj), ObjectPositionY(t.tobj), t.v_f );
	}
}

void entity_lua_setrotationx ( void )
{
	t.entityelement[t.e].rx=t.v_f;
	entity_lua_rotateupdate ( );
}

void entity_lua_setrotationy ( void )
{
	t.entityelement[t.e].ry=t.v_f;
	entity_lua_rotateupdate ( );
}

void entity_lua_setrotationz ( void )
{
	t.entityelement[t.e].rz=t.v_f;
	entity_lua_rotateupdate ( );
}

void entity_lua_resetrotationx ( void )
{
	t.entityelement[t.e].rx = t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		ODESetBodyAngle ( t.tobj,t.v_f, ObjectAngleY(t.tobj), ObjectAngleZ(t.tobj) );
		RotateObject ( t.tobj, t.v_f, ObjectAngleY(t.tobj), ObjectAngleZ(t.tobj) );
	}
	else
	{
		entity_lua_rotateupdate ( );
	}
}

void entity_lua_resetrotationy ( void )
{
	t.entityelement[t.e].ry=t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		ODESetBodyAngle ( t.tobj, ObjectAngleX(t.tobj), t.v_f, ObjectAngleZ(t.tobj) );
		RotateObject ( t.tobj, ObjectAngleX(t.tobj), t.v_f, ObjectAngleZ(t.tobj) );
	}
	else
	{
		entity_lua_rotateupdate ( );
	}
}

void entity_lua_resetrotationz ( void )
{
	t.entityelement[t.e].rz=t.v_f;
	if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
	{
		t.tobj=t.entityelement[t.e].obj;
		ODESetBodyAngle ( t.tobj, ObjectAngleX(t.tobj), ObjectAngleY(t.tobj), t.v_f );
		RotateObject ( t.tobj, ObjectAngleX(t.tobj), ObjectAngleY(t.tobj), t.v_f );
	}
	else
	{
		entity_lua_rotateupdate ( );
	}
}

void entity_lua_modulatespeed ( void )
{
	t.entityelement[t.e].speedmodulator_f=t.v_f;
}

void entity_lua_rotatex ( void )
{
	t.v_f=(t.v_f/19.775)*g.timeelapsed_f;
	t.entityelement[t.e].rx=t.entityelement[t.e].rx+t.v_f;
	entity_lua_rotateupdate ( );
}
void entity_lua_rotatey ( void )
{
	t.v_f=(t.v_f/19.775)*g.timeelapsed_f;
	t.entityelement[t.e].ry=t.entityelement[t.e].ry+t.v_f;
	entity_lua_rotateupdate ( );
}
void entity_lua_rotatez ( void )
{
	t.v_f=(t.v_f/19.775)*g.timeelapsed_f;
	t.entityelement[t.e].rz=t.entityelement[t.e].rz+t.v_f;
	entity_lua_rotateupdate ( );
}

void entity_lua_setlimbindex ( void )
{
	int iObj = t.entityelement[t.e].obj;
	if ( iObj > 0 )
	{
		if ( ObjectExist(iObj)==1 )
		{
			if ( t.v < GetObjectData(iObj)->iFrameCount )
			{
				t.lualimbindex = t.v;
			}
		}
	}
}

void entity_lua_rotatelimbx ( void )
{
	int iObj = t.entityelement[t.e].obj;
	if ( iObj > 0 )
	{
		if ( ObjectExist(iObj)==1 )
		{
			if ( t.lualimbindex >= 0 && t.lualimbindex < GetObjectData(iObj)->iFrameCount )
			{
				RotateLimb ( iObj, t.lualimbindex, t.v_f, LimbAngleY(iObj, t.lualimbindex), LimbAngleZ(iObj, t.lualimbindex) );
			}
		}
	}
}
void entity_lua_rotatelimby ( void )
{
	int iObj = t.entityelement[t.e].obj;
	if ( iObj > 0 )
	{
		if ( ObjectExist(iObj)==1 )
		{
			if ( t.lualimbindex >= 0 && t.lualimbindex < GetObjectData(iObj)->iFrameCount )
			{
				RotateLimb ( iObj, t.lualimbindex, LimbAngleX(iObj, t.lualimbindex), t.v_f, LimbAngleZ(iObj, t.lualimbindex) );
			}
		}
	}
}
void entity_lua_rotatelimbz ( void )
{
	int iObj = t.entityelement[t.e].obj;
	if ( iObj > 0 )
	{
		if ( ObjectExist(iObj)==1 )
		{
			if ( t.lualimbindex >= 0 && t.lualimbindex < GetObjectData(iObj)->iFrameCount )
			{
				RotateLimb ( iObj, t.lualimbindex, LimbAngleX(iObj, t.lualimbindex), LimbAngleY(iObj, t.lualimbindex), t.v_f );
			}
		}
	}
}

void entity_lua_scale ( void )
{
	t.entityelement[t.e].eleprof.scale=t.v_f;
	entity_lua_scaleupdate ( );
}

void entity_lua_setanimation ( void )
{
	t.luaglobal.setanim=t.e;
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_SetAnimation,t.e,0 );
	}
}

void entity_lua_setanimationframes ( void )
{
	t.luaglobal.setanim=-1;
	t.luaglobal.setanimstart=t.e;
	t.luaglobal.setanimfinish=t.v;
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_SetAnimationFrames,t.e,t.v );
	}
}

void entity_lua_playorloopanimation ( void )
{
	//  ensure entity is cloned to allow animation
	t.tte = t.e; entity_converttoclone ( );

	//  then do animation
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if (  ObjectExist(t.obj) == 1 ) 
		{
			t.entid=t.entityelement[t.e].bankindex ; t.q=t.luaglobal.setanim;
			if (  t.q == -1 ) 
			{
				//  play specific frames
				t.ttstart=t.luaglobal.setanimstart ; t.ttfinish=t.luaglobal.setanimfinish;
			}
			else
			{
				//  play from FPE animsets
				t.ttstart=t.entityanim[t.entid][t.q].start ; t.ttfinish=t.entityanim[t.entid][t.q].finish;
			}

			//  trigger transition to desired animation
			t.playflag=1-t.luaglobal.loopmode ; t.smoothanim[t.obj].st=-1;
			smoothanimtriggerrev(t.obj,t.ttstart,t.ttfinish,10.0,0,t.playflag);
			t.entityelement[t.e].lua.animating=1;
		}
	}
	if (  t.game.runasmultiplayer  ==  1 ) 
	{
		if (  t.luaglobal.setanim  ==  -1  )  t.luaglobal.setanim  =  0;
	}
}

void entity_lua_playanimation ( void )
{
	t.luaglobal.loopmode=0;
	entity_lua_playorloopanimation ( );
	if (  t.game.runasmultiplayer  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_PlayAnimation,t.e,t.v );
	}
}

void entity_lua_loopanimation ( void )
{
	t.luaglobal.loopmode=1;
	entity_lua_playorloopanimation ( );
}

void entity_lua_stopanimation ( void )
{
	t.obj=t.entityelement[t.e].obj;
	if (  t.obj>0 ) 
	{
		if (  ObjectExist(t.obj) == 1 ) 
		{
			StopObject (  t.obj );
		}
	}
}

void entity_lua_movewithanimation ( void )
{
	// is called continually for cycles where the animation should move the character
	entity_lua_findcharanimstate ( );
	if ( t.tcharanimindex != -1 ) 
	{
		int iID = t.entityelement[t.e].obj;
		if ( iID > 0 )
		{
			sObject* pObject = g_ObjectList [ iID ];
			if ( pObject )
			{
				if ( t.v == 1 )
				{
					// triggers system to provide deltas, and we use them to move this character kinetically
					pObject->bSpineTrackerMoving = true;

					// travel the exact distance walked by animation
					float fMoveForwardDelta = pObject->fSpineCenterTravelDeltaX;
					pObject->fSpineCenterTravelDeltaX = 0.0f;

					// only allow forward direction as anim loop can cause a spine position revert to base!
					if ( fMoveForwardDelta < 0.0f ) fMoveForwardDelta = 0.0f;

					// when neg, use actual, not MC-indie time based move units
					entity_lua_moveforward_core ( fMoveForwardDelta );

					/* replaced with improved move command
					// move entity and call function to control physics/object 
					float fLastX = t.entityelement[t.e].x;
					float fLastZ = t.entityelement[t.e].z;
					t.entityelement[t.e].x = NewXValue(t.entityelement[t.e].x,t.entityelement[t.e].ry, fMoveForwardDelta);
					t.entityelement[t.e].z = NewZValue(t.entityelement[t.e].z,t.entityelement[t.e].ry, fMoveForwardDelta);

					// before complete new coordinate position, ensure it stays within the bounds of a container (if any)
					darkai_staywithzone ( t.entityelement[t.e].obj, fLastX, fLastZ, &t.entityelement[t.e].x, &t.entityelement[t.e].z );

					// update visible object with entity coordinate
					entity_lua_moveupdate ( );
					*/
				}
				else
				{
					// keep character still, ignore spine vs base tracking
					pObject->bSpineTrackerMoving = false;
				}
			}
		}
	}
}

void entity_lua_setanimationframe ( void )
{
	// only for animating entities, which are visible (to prevent ALL animatable objects to clone when reload a saved game position)
	int iID = t.entityelement[t.e].obj;
	if ( iID > 0 )
	{
		sObject* pObject = g_ObjectList [ iID ];
		if ( pObject )
		{
			if ( (int)pObject->fAnimTotalFrames > 0 || t.v_f > 0.0f )
			{
				if ( pObject->bVisible == 1 && pObject->bUniverseVisible == 1 )
				{
					t.tte = t.e; entity_converttoclone ( );
				}
			}
		}
		else
			return;
	}
	else
		return;

	// 300117 - moved above as INSTANCES (to be converted to clones never get past confirmobject which disallows instances)
	// 220316 - ensure entity is cloned to allow animation
	//if ( !ConfirmObject ( iID ) )
	//	return;
	// only for animating entities, which are visible (to prevent ALL animatable objects to clone when reload a saved game position)
	//sObject* pObject = g_ObjectList [ iID ];
	//if ( (int)pObject->fAnimTotalFrames > 0 )
	//{
	//	if ( pObject->bVisible == 1 && pObject->bUniverseVisible == 1 )
	//	{
	//		t.tte = t.e; entity_converttoclone ( );
	//	}
	//}

	// force a frame in the entity object
	SetObjectFrameEx ( t.entityelement[t.e].obj, t.v_f );
}

void entity_lua_setanimationspeed ( void )
{
	//  animspeed is modulated by timeelapsed in entity_loopanim
	t.entityelement[t.e].eleprof.animspeed = t.v_f * 100.0f; // 101115 - character scripts all refer to 1.0 as a speed of 100

	// and characters need to know this change immediately
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
		t.charanimstates[t.tcharanimindex].animationspeed_f=(65.0/100.0)*t.entityelement[t.e].eleprof.animspeed;
}

void entity_lua_setentityhealth_core ( int iSilent )
{
	// if new health is zero, apply damage to entity directly
	if ( t.v <= 0 && iSilent == 0 ) 
	{
		//  set an entities health
		if ( t.entityelement[t.e].briefimmunity == 0 )
		{
			t.ttte = t.e;
			t.tdamage = t.entityelement[t.e].health;
			//PE: https://github.com/TheGameCreators/GameGuruRepo/commit/1863aaa879b05925670badf777a43825b1069702
			//PE: 150619 - this also produce this: https://github.com/TheGameCreators/GameGuruRepo/issues/466
			//PE: Changes so SetEntityHealth(e,-1) is needed to produce headshot effect.
			if (t.v == -1) {
				t.tdamageforce = 700.0f; // 210918 - so headshots have better ragdoll reaction
				t.brayy1_f=t.entityelement[t.e].y-500.0;
				t.v = 0;
			}
			else {
				t.tdamageforce = 0.0f; //PE: 150619
				t.brayy1_f = t.entityelement[t.e].y - 20.0; //PE:
			}
			t.tdamagesource = 0;
			t.brayx1_f=t.entityelement[t.e].x;
			t.brayx2_f=t.entityelement[t.e].x;
			t.brayy2_f=t.entityelement[t.e].y;
			t.brayz1_f=t.entityelement[t.e].z;
			t.brayz2_f=t.entityelement[t.e].z;
			t.tallowanykindofdamage=1;
			entity_applydamage ( );
			t.tallowanykindofdamage=0;
		}
	}
	else
	{
		t.entityelement[t.e].health=t.v;
	}
}
void entity_lua_setentityhealth ( )
{
	entity_lua_setentityhealth_core ( 0 );
}
void entity_lua_setentityhealthsilent ( )
{
	entity_lua_setentityhealth_core ( 1 );
}

void entity_lua_setforcex ( void )
{
	t.brayx2_f = t.v; t.brayx1_f = 0.0f;
}
void entity_lua_setforcey ( void )
{
	t.brayy2_f = t.v; t.brayy1_f = 0.0f;
}
void entity_lua_setforcez ( void )
{
	t.brayz2_f = t.v; t.brayz1_f = 0.0f;
}
void entity_lua_setforcelimb ( void )
{
	t.entityelement[t.e].ragdollifiedforcelimb = t.v;
}
void entity_lua_ragdollforce ( void )
{
	//  set an entities ragdoll force value (t.e to t.v)
	t.entityelement[t.e].ragdollifiedforcex_f = (t.brayx2_f-t.brayx1_f)*0.8;
	t.entityelement[t.e].ragdollifiedforcey_f = (t.brayy2_f-t.brayy1_f)*1.2;
	t.entityelement[t.e].ragdollifiedforcez_f = (t.brayz2_f-t.brayz1_f)*0.8;
	t.entityelement[t.e].ragdollifiedforcevalue_f = t.v * 8000.0;
}

void entity_lua_charactercontrolmanual ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  check we are not already in that state before sending out lua, so we dont flood
		if (  t.charanimstates[t.tcharanimindex].playcsi != g.csi_limbo ) 
			if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 && g.mp.endplay  ==  0 ) 
				mp_sendlua (  MP_LUA_CharacterControlManual,t.e,t.v );

		//  disable all character control influence 
		t.charanimstate.playcsi=g.csi_limbo;
		t.charanimstate.limbomanualmode = 1; // (AND ENTER NEW AI FULL MANUAL MODE)
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
	if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
		mp_sendlua (  MP_LUA_CharacterControlManual,t.e,t.v );
}

void entity_lua_charactercontrollimbo ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  check we are not already in that state before sending out lua, so we dont flood
		if (  t.charanimstates[t.tcharanimindex].playcsi  !=  g.csi_limbo ) 
		{
			if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 && g.mp.endplay  ==  0 ) 
			{
				mp_sendlua (  MP_LUA_CharacterControlLimbo,t.e,t.v );
			}
		}
		//  disable all character control influence
		t.charanimstate.playcsi=g.csi_limbo;
		t.charanimstate.limbomanualmode = 0; // REGULAR LIMBO MODE WITH SOME CONTROL OVER AI OBJECT
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
	if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_CharacterControlLimbo,t.e,t.v );
	}
}

void entity_lua_charactercontrolunarmed ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  check we are not already in that state before sending out lua, so we dont flood
		if ( t.charanimstates[t.tcharanimindex].playcsi != g.csi_unarmed ) 
		{
			if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 && g.mp.endplay  ==  0 ) 
			{
				mp_sendlua (  MP_LUA_CharacterControlUnarmed,t.e,t.v );
			}
		}
		//  restful no weapon out
		if ( t.charanimstates[t.tcharanimindex].playcsi == g.csi_limbo  )  t.charanimstates[t.tcharanimindex].playcsi = g.csi_unarmed;
		t.charanimcontrols[t.tcharanimindex].alerted=0;
	}
}

void entity_lua_charactercontrolarmed ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		// 200316 - replaced UNARMED state with STOOD (as this is the armed CSI states!)
		// check we are not already in that state before sending out lua, so we dont flood
		if ( t.charanimstates[t.tcharanimindex].playcsi != t.csi_stood[t.charanimstates[t.tcharanimindex].weapstyle] ) 
		{
			if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 && g.mp.endplay  ==  0 ) 
			{
				mp_sendlua (  MP_LUA_CharacterControlArmed,t.e,t.v );
			}
		}
		// get weapon out (oh er)
		if (  t.charanimstates[t.tcharanimindex].playcsi == g.csi_limbo ) 
		{
			t.charanimstates[t.tcharanimindex].playcsi = t.csi_stood[t.charanimstates[t.tcharanimindex].weapstyle];
			if ( t.charanimstates[t.tcharanimindex].playcsi == 0 )
			{
				// 110416 - no weapon on this character, defer to unarmed state (legacy behaviour)
				t.charanimstates[t.tcharanimindex].playcsi = g.csi_unarmed;
			}
		}
		t.charanimcontrols[t.tcharanimindex].alerted=1;
	}
}

void entity_lua_charactercontrolfidget ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  cause character to fidget - do we use charactercontrol array for everything?!?
		//  want to avoid 'unnecessary' state machines hard-coded into engine
		if (  t.charanimstates[t.tcharanimindex].playcsi == g.csi_limbo  )  t.charanimstates[t.tcharanimindex].playcsi = g.csi_unarmed;
	}
}

void entity_lua_charactercontrolducked ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  ducked mode
		if (  t.charanimstates[t.tcharanimindex].playcsi == g.csi_limbo  )  t.charanimstates[t.tcharanimindex].playcsi = g.csi_unarmed;
		t.charanimcontrols[t.tcharanimindex].ducking = 1;
	}
return;

}

void entity_lua_charactercontrolstand ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		//  stand up mode
		if (  t.charanimstates[t.tcharanimindex].playcsi == g.csi_limbo ) 
		{
			t.charanimstates[t.tcharanimindex].playcsi=g.csi_unarmed;
		}
		t.charanimcontrols[t.tcharanimindex].ducking = 2;
	}
}

void entity_lua_setcharactertowalkrun ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		t.charanimstate.runmode=t.v;
		t.charanimstate.strafemode=0;
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
	if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		mp_sendlua (  MP_LUA_setcharactertowalkrun,t.e,t.v );
	}
}

void entity_lua_setlockcharacter ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		t.charanimstate.runmode=0;
		t.charanimstate.strafemode=0;
		t.charanimstate.freezeallmovement=t.v;
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
}

void entity_lua_setcharactertostrafe ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		t.charanimstate.runmode=0;
		t.charanimstate.strafemode=1+t.v;
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
return;

}

void entity_lua_setcharactervisiondelay ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{
		t.charanimstate.visiondelay=t.v;
		t.charanimstates[t.tcharanimindex]=t.charanimstate;
	}
return;

}

void entity_lua_lookatplayer ( void )
{
	entity_lua_findcharanimstate ( );
	if (  t.tcharanimindex != -1 ) 
	{

		//  Simply look in direction of player
		t.tdx_f=CameraPositionX(0)-t.entityelement[t.e].x;
		t.tdz_f=CameraPositionZ(0)-t.entityelement[t.e].z;
		AISetEntityAngleY (  t.charanimstate.obj,atan2deg(t.tdx_f,t.tdz_f) );

		//  If angle beyond 'look angle range', perform full rotation
		t.tangley_f=AIGetEntityAngleY(t.charanimstate.obj) ;
		t.headangley_f=t.tangley_f-ObjectAngleY(t.charanimstate.obj) ;
		if (  t.headangley_f<-180  )  t.headangley_f = t.headangley_f+360;
		if (  t.headangley_f>180  )  t.headangley_f = t.headangley_f-360;
		if (  t.headangley_f<-75 || t.headangley_f>75 ) 
		{
			t.charanimstate.currentangle_f=t.tangley_f;
			t.charanimstate.updatemoveangle=1;
			AISetEntityAngleY (  t.charanimstate.obj,t.charanimstate.currentangle_f );
			t.charanimstates[t.tcharanimindex] = t.charanimstate;
		}

	}
	if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		//  only send this max once a second
		if (  Timer() - t.entityelement[t.e].mp_rotateTimer > 1000 ) 
		{
			mp_sendlua (  MP_LUA_LookAtPlayer,t.e,g.mp.me );
			t.entityelement[t.e].mp_rotateTimer = Timer();
		}
	}
}

void entity_lua_rotatetoanglecore ( float fDestAngle, float fAngleOffset )
{
	entity_lua_findcharanimstate ( );
	if ( t.v == 100 ) 
	{
		t.tsmooth_f=1.0;
	}
	else
	{
		t.tsmooth_f=(100.0/(t.v+0.0))/g.timeelapsed_f;
	}
	if (  t.tcharanimindex == -1 ) 
	{
		//  regular entity
		t.tnewangley_f=CurveAngle(fDestAngle,t.entityelement[t.e].ry,t.tsmooth_f);
		t.entityelement[t.e].ry=t.tnewangley_f;
		entity_lua_rotateupdate ( );
	}
	else
	{
		// need to factor in entity speed for AI characters
		//t.tsmooth_f *= (AIGetEntitySpeed(t.charanimstate.obj)/100.0f);
		t.tsmooth_f /= (t.entityelement[t.charanimstate.e].eleprof.speed/100.0f);

		//  character subsystem
		t.tnewangley_f=CurveAngle(fDestAngle,t.charanimstate.currentangle_f,t.tsmooth_f);
		t.charanimstate.currentangle_f=t.tnewangley_f;
		t.charanimstate.updatemoveangle=1;
		t.charanimstates[t.tcharanimindex] = t.charanimstate;
		t.entityelement[t.e].ry=t.tnewangley_f;

		//this overwrites angle assigned by pathfinder (causing circular paths at high speed with smoothing on)
		//AISetEntityAngleY (  t.charanimstate.obj,t.charanimstate.currentangle_f );
		AISetEntityAngleY ( t.charanimstate.obj, fDestAngle );

		// 240217 - and update visually
		entity_lua_rotateupdate ( );
	}
	if (  t.game.runasmultiplayer  ==  1 && g.mp.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		if (  Timer() - t.entityelement[t.e].mp_rotateTimer > 1000 ) 
		{
			mp_sendlua (  MP_LUA_RotateToPlayer,t.e,t.tnewangley_f );
			t.entityelement[t.e].mp_rotateTimer = Timer();
		}
	}
}

void entity_lua_rotatetocore ( float fAngleOffset )
{
	t.tdx_f=t.tcamerapositionx_f-t.entityelement[t.e].x;
	t.tdz_f=t.tcamerapositionz_f-t.entityelement[t.e].z;
	float fDestAngle = atan2deg(t.tdx_f,t.tdz_f);
	entity_lua_rotatetoanglecore ( fDestAngle, fAngleOffset );
}

void entity_lua_rotatetoplayer ( void )
{
	//  third person moves camera to player position
	t.tcamerapositionx_f=CameraPositionX(t.terrain.gameplaycamera);
	t.tcamerapositiony_f=CameraPositionY(t.terrain.gameplaycamera);
	t.tcamerapositionz_f=CameraPositionZ(t.terrain.gameplaycamera);
	entity_lua_rotatetocore ( 0.0f );
}

void entity_lua_rotatetocamera ( void )
{
	//  for when we want the REAL camera (for decals)
	entity_gettruecamera ( );
	entity_lua_rotatetocore ( 0.0f );
}

void entity_lua_rotatetoplayerwithoffset ( void )
{
	t.tcamerapositionx_f=CameraPositionX(t.terrain.gameplaycamera);
	t.tcamerapositiony_f=CameraPositionY(t.terrain.gameplaycamera);
	t.tcamerapositionz_f=CameraPositionZ(t.terrain.gameplaycamera);
	entity_lua_rotatetocore ( t.v );
}

void entity_lua_set_gravity ( void )
{
	t.entityelement[t.e].nogravity=t.v;
	//  commented out for now due to not working anyway
	//BPhys_SetNoGravity entityelement(e).obj,v;
}

void entity_lua_fireweapon ( bool instant)
{
	//  uses rateoffire
	t.tnotokay=t.entityelement[t.e].eleprof.rateoffire;
	
	if (instant) {
		//PE: Instant fire weapon for lua control.
		t.tnotokay = 0;
	}
	else {
		if (t.tnotokay > 100)
		{
			t.tnotokay = Rnd((t.tnotokay - 100) / 5);
			if (t.tnotokay <= 1)  t.tnotokay = 0;
		}
		else
		{
			t.tnotokay = 0;
		}
	}
	if ((DWORD)(Timer()) < t.playercontrol.ressurectionceasefire)  t.tnotokay = 1;
	if (t.entityelement[t.e].limbhurt != 0)  t.tnotokay = 1;

	if (  t.tnotokay == 0 ) 
	{
		entity_lua_findcharanimstate ( );
		if (  t.tcharanimindex != -1 ) 
		{
			if (instant) {
				//PE: for instant fire so it can be controlled from lua.
				t.charanimstate.firerateaccumilator = 0;
			}
			if ( t.charanimstate.limbomanualmode == 1 )
			{
				// AI manual mode just shoots if instructed
				darkai_shootplayer ( );
			}
			else
			{
				//  if character has weapon
				if (  t.entityelement[t.e].eleprof.hasweapon>0 ) 
				{
					if ( t.charanimstate.ammoinclip == 0 ) 
					{
						//  if weapon empty, reload
						if (  t.charanimcontrols[t.tcharanimindex].spotactioning == 0 ) 
						{
							t.charanimcontrols[t.tcharanimindex].spotactioning=1;
						}
					}
					else
					{
						//  if still have ammo in weapon (pass in tcharanimindex)
						darkai_shootplayer ( );
					}
				}
			}
			t.charanimstates[t.tcharanimindex] = t.charanimstate;
		}
	}
}

void entity_lua_hurtplayer ( void )
{
	t.tdamage=t.v ; t.te=t.e ; t.tDrownDamageFlag=0;
	physics_player_takedamage ( );
}

void entity_lua_drownplayer ( void )
{
	t.tdamage=t.v ; t.te=t.e ; t.tDrownDamageFlag=1;
	physics_player_takedamage ( );
	t.tDrownDamageFlag=0;
}

void entity_lua_switchscript ( void )
{
	t.entityelement[t.e].eleprof.aimainname_s = t.s_s;
	t.strwork = cstr(cstr(Lower(t.entityelement[t.e].eleprof.aimainname_s.Get()))+"_init");
	LuaSetFunction (  t.strwork.Get(),1,0 );
	LuaPushInt (  t.e  ); LuaCallSilent (  );
}

void entity_lua_addplayerweapon(void)
{
	//  collect this weapon
	t.tentid = t.entityelement[t.e].bankindex;
	t.weaponindex = t.entityprofile[t.tentid].isweapon;
	t.tqty = t.entityelement[t.e].eleprof.quantity;
	physics_player_addweapon();
}

void entity_lua_changeplayerweapon(void)
{
	// force this weapon NAME to be selected
	t.findgun_s = t.s_s;
	gun_findweaponindexbyname();
	g.autoloadgun = t.foundgunid; 
	gun_change();
}

void entity_lua_changeplayerweaponid(void)
{
	// if no gun, this will load in gun if it is not already in memory (normally all pre-loaded but standalonelevelreload mode can load a level with missing guns)
	// so this will be called from the GameLoopLoadStats global function for all weapons currently held by player at that point in game when start the fresh level
	int iWeaponID = t.v;

	// force this weapon ID to be selected
	g.autoloadgun = iWeaponID;
	gun_change();
}

void entity_lua_replaceplayerweapon ( void )
{
	//  replace this weapon with one currently held
	//  remove weapon from slot
	t.tswapslot=0;
	if (  t.gunid>0 ) 
	{
		//  find swap slot for old weapon (gunid)
		for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
		{
			if (  t.weaponslot[t.ws].pref == t.gunid  )  t.tswapslot = t.ws;
		}
		//  remove old weapon
		t.weaponindex=t.gunid ; physics_player_removeweapon ( );
	}
	//  what is new weapon
	t.tentid=t.entityelement[t.e].bankindex;
	t.weaponindex=t.entityprofile[t.tentid].isweapon;
	//  assign preference for new weapon
	if (  t.tswapslot>0 ) 
	{
		t.weaponslot[t.tswapslot].pref = t.weaponindex;
		t.weaponammo[t.tswapslot] = 0; // reset so new weapon can work out its new ammo
	}
	//  now collect weapon (will find freed up slot from above)
	t.tqty=t.entityelement[t.e].eleprof.quantity;
	physics_player_addweapon ( );
}

void entity_lua_addplayerammo ( void )
{
	//  Collect Ammo for player
	t.tentid=t.entityelement[t.e].bankindex;
	t.tqty=t.entityelement[t.e].eleprof.quantity;
	t.tgunid=t.entityprofile[t.tentid].hasweapon;
	t.tfiremode=0;
	t.tpool=g.firemodes[t.tgunid][t.tfiremode].settings.poolindex;
	if (  t.tpool == 0 ) 
	{
		//  Lee, are we using AMMO POOL only from now on (ammo for a single gun still relevant?)
//   `tammo=weaponclipammo(weaponammoindex+ammooffset)

	}
	else
	{
		//  increase ammo pool by ammo quantity
		t.ammopool[t.tpool].ammo=t.ammopool[t.tpool].ammo+t.tqty;
	}
return;

}

void entity_lua_addplayerhealth ( void )
{
	//  collect health
	t.tqty=t.entityelement[t.e].eleprof.quantity;
	t.player[t.plrid].health=t.player[t.plrid].health+t.tqty;
return;

}

void entity_lua_setplayerpower ( void )
{
	//  increase power of player (levelup/magic)
	t.player[t.plrid].powers.level=t.v;
return;

}

void entity_lua_addplayerpower ( void )
{
	//  increase power of player (levelup/magic)
	t.player[t.plrid].powers.level=t.player[t.plrid].powers.level+t.v;
return;

}

void entity_lua_addplayerjetpack ( void )
{
	// collect jet pack
	t.playercontrol.jetpackcollected = 1;
	if (  t.playercontrol.jetpackmode == 0  )  t.playercontrol.jetpackmode = 1;
	t.playercontrol.jetpackfuel_f=t.playercontrol.jetpackfuel_f+t.v;
	if (  t.playercontrol.jetobjtouse>0 ) 
	{
		if (  ObjectExist(t.playercontrol.jetobjtouse) == 1  )  HideObject (  t.playercontrol.jetobjtouse );
	}
	t.thudlayeritemindex=t.entityprofile[t.entityelement[t.e].bankindex].ishudlayer;
	t.playercontrol.jetobjtouse=t.hudlayerlist[t.thudlayeritemindex].obj;
	t.playercontrol.jetpackhidden=t.hudlayerlist[t.thudlayeritemindex].hidden;
	if (  t.playercontrol.thirdperson.enabled == 1 ) 
	{
		t.playercontrol.jetpackhidden=1;
	}
	if (  t.playercontrol.jetobjtouse>0 ) 
	{
		if (  ObjectExist(t.playercontrol.jetobjtouse) == 1  )  ShowObject (  t.playercontrol.jetobjtouse );
	}
}

void entity_lua_set_light_visible ( void )
{
	//  receives e and v
	t.entityelement[t.e].eleprof.light.islit=t.v;
	for ( t.l = 1 ; t.l<=  g.infinilightmax; t.l++ )
	{
		if (  t.infinilight[t.l].used == 1 ) 
		{
			if (  t.infinilight[t.l].e == t.e ) 
			{
				t.infinilight[t.l].islit=t.v;
			}
		}
	}
return;
}

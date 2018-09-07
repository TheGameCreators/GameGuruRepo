//----------------------------------------------------
//--- GAMEGURU - M-DAI
//----------------------------------------------------

#include "gameguru.h"

// prototypes to link with entity avoidance
void entity_lua_moveforward_core_nooverlap ( int te, float* pNX, float* pNZ );

// 
//  Dark Dave's A.I System
// 

void darkai_init ( void )
{
	//  Init AI
	AIStart( g.guishadereffectindex ); 
	AISetRadius ( t.aisystem.obstacleradius  ); 
	t.aisystem.on=1;

	//  Generate OBS by default
	t.aisystem.generateobs=1;

	//  Used in LUA script to disable components in-play
	t.aisystem.processlogic=1;
	t.aisystem.processplayerlogic=1;

	//  Initial A.I System defaults
	t.aisystem.defaulteyeheight_f=65.0;
	t.aisystem.defaulteyehalfheight_f=t.aisystem.defaulteyeheight_f/2.0;
	t.aisystem.currenteyeheight_f=t.aisystem.defaulteyeheight_f;

	//  Generate character debug object (capsule with cone nose)
	if (  GetMeshExist(t.aisystem.debugentitymesh) == 0 ) 
	{
		MakeObjectCone (  t.aisystem.debugentityworkobj,33 );
		MakeObjectSphere (  t.aisystem.debugentityworkobj2,33  ); MakeMeshFromObject (  t.aisystem.debugentitymesh,t.aisystem.debugentityworkobj2  ); DeleteObject (  t.aisystem.debugentityworkobj2 );
		MakeObjectCylinder (  t.aisystem.debugentityworkobj2,65  ); MakeMeshFromObject (  t.aisystem.debugentitymesh2,t.aisystem.debugentityworkobj2  ); DeleteObject (  t.aisystem.debugentityworkobj2 );
		OffsetLimb (  t.aisystem.debugentityworkobj,0,0,10,-t.aisystem.defaulteyeheight_f  ); ScaleLimb (  t.aisystem.debugentityworkobj,0,50,50,50 );
		AddLimb (  t.aisystem.debugentityworkobj,1,t.aisystem.debugentitymesh  ); OffsetLimb (  t.aisystem.debugentityworkobj,1,0,0,-t.aisystem.defaulteyeheight_f  ); ScaleLimb (  t.aisystem.debugentityworkobj,1,50,50,50 );
		AddLimb (  t.aisystem.debugentityworkobj,2,t.aisystem.debugentitymesh2  ); OffsetLimb (  t.aisystem.debugentityworkobj,2,0,0,t.aisystem.defaulteyeheight_f/-2.0  ); ScaleLimb (  t.aisystem.debugentityworkobj,2,15,100,15  ); RotateLimb (  t.aisystem.debugentityworkobj,2,-90,0,0 );
		XRotateObject (  t.aisystem.debugentityworkobj,90  ); FixObjectPivot (  t.aisystem.debugentityworkobj );
		MakeMeshFromObject (  t.aisystem.debugentitymesh,t.aisystem.debugentityworkobj );
		DeleteObject (  t.aisystem.debugentityworkobj );
	}

	//  Associate 'DarkA.I Player' with unique player object
	if (  ObjectExist(t.aisystem.objectstartindex) == 1  )  DeleteObject (  t.aisystem.objectstartindex );
	MakeObject (  t.aisystem.objectstartindex,t.aisystem.debugentitymesh,0 );
	if (  t.aisystem.usingphysicsforai == 1 ) 
	{
		OffsetLimb (  t.aisystem.objectstartindex,0,0,ObjectSizeY(t.aisystem.objectstartindex)/-2,0 );
	}
	HideObject (  t.aisystem.objectstartindex );
	PositionObject (  t.aisystem.objectstartindex,(512*50),1000,(512*50) );
	AIAddPlayer (  t.aisystem.objectstartindex );
	AISetPlayerContainer (  0 );

	//  new DarkA.I player height system where 'defaulteyehalfheight#' is difference between stood and ducked height
	AISetPlayerHeight (  t.aisystem.defaulteyehalfheight_f );

	// 061115 - reset smoothanim array so no carryover of transitions to new test game
	darkai_resetsmoothanims();
}

void darkai_resetsmoothanims ( void )
{
	for ( int n = 0; n < t.tmaxobjectnumber; n++ )
	{
		t.smoothanim[n].fn = 0;
		t.smoothanim[n].playflag = 0;
		t.smoothanim[n].playstarted = 0;
		t.smoothanim[n].rev = 0;
		t.smoothanim[n].st = 0;
		t.smoothanim[n].transition = 0;
	}
}

void darkai_free ( void )
{
	//  Ensure any debug mode is zero to zero when leave test game
	if (  t.visuals.debugvisualsmode>0 ) 
	{
		t.visuals.debugvisualsmode=0;
	}

	//  Debug view of all obstacles on the ground Floor (  )
	if (  t.aisystem.usingdebugobjects == 1 ) 
	{
		t.aisystem.usingdebugobjects=0;
		AIDebugHideObstacleBounds ( -1 );
		AIDebugHidePaths (  );
		AIDebugHideViewArcs (  );
		//AIDebugHideSounds (  ); // 150416 - froze gun and character animations
	}

	//  free A.I resources (and 1 to stop thread running)
	AIReset ( 1 ); 
	t.aisystem.on=0 ; t.aisystem.generateobs=1;
}

void darkai_preparedata ( void )
{
	//  Create obstacles, paths and finalise this data (done after all data loaded and ready to use)
	t.aisystem.obs=0;

	//  If MAP.OBS file exists, load instead of generate
	t.tobsfile_s="levelbank\\testmap\\map.obs";
	if (  FileExist(t.tobsfile_s.Get()) == 1 ) 
	{
		// NOTE: gridedit_changemodifiedflag deletes this file if static entity altered
	
		// load from file (standalone or last level state with no changes)
		timestampactivity(0,"_darkai_loadobstacles");
		darkai_loadobstacles ( );

		// does not generate new OBS from static entities but completes after that process
		t.aisystem.generateobs=0;
	}

	//  Terrain Obstacles (requires terrain height data loaded)
	t.terrain.terrainregionupdate=0;
	terrain_refreshterrainmatrix ( );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	if ( t.aisystem.generateobs == 1 ) 
	{
		timestampactivity(0,"darkai_obstacles_terrain");
		if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	}

	//  Setup all AI paths and AI containers(navmeshes) via waypoints data
	DWORD dwAIPathCount = 1; // 20316 - path indices must be sequential
	DWORD dwContainerObstacleIndex = 100000;
	timestampactivity(0,"darkai_paths");
	for ( t.twaypointindex = 1 ; t.twaypointindex<=  g.waypointmax; t.twaypointindex++ )
	{
		// if waypoint, create path in AI system
		if (  t.waypoint[t.twaypointindex].style == 1 ) 
		{
			if (  t.waypoint[t.twaypointindex].count>0 ) 
			{
				AIMakePath ( dwAIPathCount );
				for ( t.w = t.waypoint[t.twaypointindex].start ; t.w<=  t.waypoint[t.twaypointindex].finish; t.w++ )
				{
					AIPathAddPoint ( dwAIPathCount,t.waypointcoord[t.w].x,t.waypointcoord[t.w].y,t.waypointcoord[t.w].z,0 );
				}
				dwAIPathCount++;
			}
		}

		// if navmeshzone, create enclosed obstacle (inverted obstacle will only allow paths WITHIN it)
		if ( t.waypoint[t.twaypointindex].style == 3 ) 
		{
			if ( t.waypoint[t.twaypointindex].count>0 ) 
			{
				// by creating plots backwards, we create a containment rather than an obstacle
				DWORD dwAIContainerIDs = t.twaypointindex;
				AIAddContainer ( dwAIContainerIDs );
				int iContainerID = dwAIContainerIDs, iFullHeight = 1, iViewBlocker = 0;
				AIStartNewObstacle ( dwContainerObstacleIndex );
				for ( t.w = t.waypoint[t.twaypointindex].finish-1; t.w >= t.waypoint[t.twaypointindex].start; t.w-- )
				{
					AIAddObstacleVertex ( t.waypointcoord[t.w].x,t.waypointcoord[t.w].z );
				}
				AIEndNewObstacle ( iContainerID, iFullHeight, iViewBlocker );
				dwContainerObstacleIndex++;
			}
		}
	}
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Setup manual cover positions from cover marker entities
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		if (  t.entityelement[t.e].bankindex>0 ) 
		{
			if (  t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 9 ) 
			{
				t.ttx_f=t.entityelement[t.e].x;
				t.tty_f=t.entityelement[t.e].y;
				t.ttz_f=t.entityelement[t.e].z;
				t.tta_f=t.entityelement[t.e].ry;
				LPSTR pIfUsedString = t.entityelement[t.e].eleprof.ifused_s.Get();
				AIAddCoverPoint ( t.ttx_f, t.tty_f, t.ttz_f, t.tta_f, pIfUsedString );
			}
		}
	}
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
}

void darkai_completeobstacles ( void )
{
	// OLD - still need to complete obstacles, even if loaded data in
	// 060917 - don't regenerate waypoints, etc - data has been loaded and is ready
	if ( t.aisystem.generateobs == 1 ) 
	{
		if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
		AICompleteObstacles (  );
	}

	//  when ALL obstacles done, can calculate paths
	if (  t.aisystem.generateobs == 1 ) 
	{
		//  after creating obstacles, save into file
		if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
		darkai_saveobstacles ( );
	}

	// Add additional container connections to scene
	timestampactivity(0,"darkai_zoneconnections");
	for ( int thiswaypointindex = 1 ; thiswaypointindex <= g.waypointmax; thiswaypointindex++ )
	{
		// if waypoint, create path in AI system
		if ( t.waypoint[thiswaypointindex].style == 1 ) 
		{
			if ( t.waypoint[thiswaypointindex].count>0 ) 
			{
				// create connecting paths between containers
				int iLastContainerStartIndex = -1;
				float fLastPointX = 0, fLastPointZ = 0;
				for ( int tw = t.waypoint[thiswaypointindex].start; tw <= t.waypoint[thiswaypointindex].finish; tw++ )
				{
					// which container is it in
					int iContainerStartIndex = 0;
					for ( t.waypointindex = 1; t.waypointindex <= g.waypointmax; t.waypointindex++ )
					{
						if ( waypoint_ispointinzoneex ( t.waypointindex, t.waypointcoord[tw].x, t.waypointcoord[tw].y, t.waypointcoord[tw].z, 1 ) == 1 ) 
						{
							iContainerStartIndex = t.waypointindex;
							break;
						}
					}

					// going to this point (see below)
					t.tpointx_f = t.waypointcoord[tw].x;
					t.tpointz_f = t.waypointcoord[tw].z;

					// create connection between containers
					if ( iLastContainerStartIndex != -1 )
					{
						// if points cross container barrier
						if ( iContainerStartIndex != iLastContainerStartIndex )
						{
							// create a connection between the points
							AIConnectContainers ( iLastContainerStartIndex, fLastPointX, fLastPointZ, iContainerStartIndex, t.tpointx_f, t.tpointz_f );
						}
					}

					// record for next cycle
					iLastContainerStartIndex = iContainerStartIndex;
					fLastPointX = t.tpointx_f;
					fLastPointZ = t.tpointz_f;
				}
			}
		}
	}
}

void darkai_invalidateobstacles ( void )
{
	// Remove container zero obstacles file
	if ( t.tignoreinvalidateobstacles == 0 ) 
	{
		t.tobsfile_s = g.fpscrootdir_s+"\\Files\\levelbank\\testmap\\map.obs";
		if ( FileExist(t.tobsfile_s.Get()) == 1 ) DeleteAFile ( t.tobsfile_s.Get() );
		t.aisystem.generateobs=1;
	}
}

void darkai_saveobstacles ( void )
{
	//  Save container zero obstacles
	t.tobsfile_s="levelbank\\testmap\\map.obs";
	if (  FileExist(t.tobsfile_s.Get()) == 1  )  DeleteAFile (  t.tobsfile_s.Get() );
	//C++ISSUE - hope this works :D
	AISaveObstacleData (  0, (unsigned long) t.tobsfile_s.Get() );
	t.aisystem.generateobs=0;
}

void darkai_loadobstacles ( void )
{
	//  Load container zero obstacles
	t.tobsfile_s="levelbank\\testmap\\map.obs";
	if (  FileExist(t.tobsfile_s.Get()) == 1 ) 
	{
		AILoadObstacleData (  0, (unsigned long) t.tobsfile_s.Get() );
	}
}

int darkai_finddoorcontainer ( int iObj )
{
	int iContainerID = 0;
	for ( int thiswaypointindex = 1 ; thiswaypointindex <= g.waypointmax; thiswaypointindex++ )
	{
		// this is now done inside waypoint_ispointinzoneex
		//if ( t.waypoint[thiswaypointindex].style == 3 ) 
		//{
		// this is now done inside waypoint_ispointinzoneex
		//if ( t.waypoint[thiswaypointindex].count>0 ) 
		//{
		if ( waypoint_ispointinzoneex ( thiswaypointindex, ObjectPositionX(iObj), ObjectPositionY(iObj), ObjectPositionZ(iObj), 1 ) == 1 )
		{
			iContainerID = thiswaypointindex;
			break;
		}
		//}
		//}
	}
	return iContainerID;
}

void darkai_adddoor ( void )
{
	// gate dimensions for now
	if ( t.tobj>0 ) 
	{
		if ( ObjectExist(t.tobj) == 1 ) 
		{
			// maximum bounds of this object at any Y rotation creates a dynamic obstacle for DarkAI
			t.tsx_f=ObjectSizeX(t.tobj,1)/2;
			t.tsz_f=ObjectSizeZ(t.tobj,1)/2;
			if ( t.tsx_f<t.tsz_f ) t.tsx_f = t.tsz_f;
			if ( t.tsz_f<t.tsx_f ) t.tsz_f = t.tsx_f;
			t.tx1_f=ObjectPositionX(t.tobj)-t.tsx_f;
			t.tz1_f=ObjectPositionZ(t.tobj)-t.tsz_f;
			t.tx2_f=ObjectPositionX(t.tobj)+t.tsx_f;
			t.tz2_f=ObjectPositionZ(t.tobj)+t.tsz_f;

			// use collision center to plot accurate bounds of door
			t.tx1_f = ObjectPositionX ( t.tobj ) + GetObjectCollisionCenterX ( t.tobj );
			t.tz1_f = ObjectPositionZ ( t.tobj ) + GetObjectCollisionCenterZ ( t.tobj );
			t.tx2_f = t.tx1_f + t.tsx_f;
			t.tz2_f = t.tz1_f + t.tsz_f;
			t.tx1_f = t.tx1_f - t.tsx_f;
			t.tz1_f = t.tz1_f - t.tsz_f;

			// work out which zone (container) this door resides on(in)
			int iContainerID = darkai_finddoorcontainer ( t.tobj );

			// finally add the blocking door element
			AIAddDoor ( t.tobj, iContainerID, t.tx1_f,t.tz1_f,t.tx2_f,t.tz2_f );
		}
	}
}

void darkai_removedoor ( void )
{
	if ( t.tobj>0 ) 
	{
		if ( ObjectExist(t.tobj) == 1 ) 
		{
			int iContainerID = darkai_finddoorcontainer ( t.tobj );
			AIRemoveDoor ( t.tobj, iContainerID );
		}
	}
}

void darkai_createinternaldebugvisuals ( void )
{
	if (  t.aisystem.usingphysicsforai == 1 ) 
	{
		if (  t.terrain.TerrainID>0 ) 
		{
			t.debuglayerheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.terrain.playerx_f,t.terrain.playerz_f)+15;
		}
		else
		{
			t.debuglayerheight_f=1000+15;
		}
	}
	else
	{
		t.debuglayerheight_f=15;
	}
	AIDebugShowObstacleBounds ( -1, t.debuglayerheight_f );
	AIDebugShowPaths ( t.debuglayerheight_f );
	AIDebugShowViewArcs ( t.debuglayerheight_f );
	//AIDebugShowSounds ( t.debuglayerheight_f ); // 150416 - froze gun and character animations
	//AIDebugShowAvoidanceAngles ( t.debuglayerheight_f );
}

void darkai_destroyinternaldebugvisuals ( void )
{
	AIDebugHideObstacleBounds ( -1 );
	AIDebugHidePaths (  );
	AIDebugHideViewArcs (  );
	//AIDebugHideSounds (  ); // 150416 - froze gun and character animations
}

void darkai_updatedebugobjects ( void )
{
	//  Debug view of all obstacles on the ground Floor (  )
	if (  t.aisystem.on == 1 ) 
	{
		if (  t.visuals.debugvisualsmode >= 10 ) 
		{
			if (  t.aisystem.usingdebugobjects == 0 ) 
			{
				t.aisystem.usingdebugobjects=1;
				//  show A.I debug renders
				darkai_createinternaldebugvisuals ( );
			}
		}
		else
		{
			if (  t.aisystem.usingdebugobjects == 1 ) 
			{
				t.aisystem.usingdebugobjects=0;
				//  hide A.I debug renders
				darkai_destroyinternaldebugvisuals ( );
			}
		}

	}
}

void darkai_obstacles_terrain ( void )
{
	//  can skip obstacle generation
	if (  g.gskipobstaclecreation == 1  )  return;
	if (  g.gskipterrainobstaclecreation == 1  )  return;
	
	//  scan heightmap to create obstacles around water bodies
	obs_fillterraindot ( );

	//  go through loop until all terrain features added to OBS list
	g.obsindex=0;
	t.mooreneighborhood.mode=0;
	while (  t.mooreneighborhood.mode != 99 ) 
	{
		ode_doterraindotwork ( );
	}
	//  create obstacle polygons from data
	if (  g.obsindex>0 ) 
	{
		//  reverse all OBS coords
		Dim (  t.finalobs,g.obsindex  );
		for ( t.oo = 1 ; t.oo<=  g.obsindex; t.oo++ )
		{
			t.finalobs[1+(g.obsindex-t.oo)]=t.obs[t.oo];
		}
		//  create obstacle polygons
		AIStartNewObstacle (  t.aisystem.terrainobsnum  ); t.tobjstarted=1;
		t.tcx_f=0 ; t.tcz_f=0 ; t.tcc=0;
		for ( t.o = 1 ; t.o<=  g.obsindex; t.o++ )
		{
			t.x=t.finalobs[t.o].x ; t.z=t.finalobs[t.o].z;
			if (  t.x == -1 && t.o == 1 ) 
			{
				//  ignore terminator at first index
			}
			else
			{
				if ( t.x == -1 ) break ; else { t.tcx_f=t.tcx_f+t.x ; t.tcz_f=t.tcz_f+t.z ; ++t.tcc ; }
			}
		}
		t.tcx_f=t.tcx_f/t.tcc ; t.tcz_f=t.tcz_f/t.tcc;
		for ( t.o = 1 ; t.o<=  g.obsindex; t.o++ )
		{
			t.x=t.finalobs[t.o].x ; t.z=t.finalobs[t.o].z;
			if (  t.x == -1 && t.o == 1 ) 
			{
				//  ignore terminator at first index
			}
			else
			{
				if (  t.x == -1 ) 
				{
					if (  t.tobjstarted == 1 ) 
					{
						AIEndNewObstacle (  0,0,0  ); t.tobjstarted=0;
						if (  t.o<g.obsindex ) 
						{
							AIStartNewObstacle (  t.aisystem.terrainobsnum  ); t.tobjstarted=1;
						}
					}
					t.tcx_f=0 ; t.tcz_f=0 ; t.tcc=0;
					for ( t.oo = t.o+1 ; t.oo<=  g.obsindex; t.oo++ )
					{
						t.x=t.finalobs[t.oo].x ; t.z=t.finalobs[t.oo].z;
						if ( t.x == -1 ) break; else { t.tcx_f=t.tcx_f+t.x ; t.tcz_f=t.tcz_f+t.z ; ++t.tcc; }
					}
					t.tcx_f=t.tcx_f/t.tcc ; t.tcz_f=t.tcz_f/t.tcc;
				}
				else
				{
					//  expand coordinate based on center (above) so A.I goes around obstacle much wider
					t.tx_f=t.x-t.tcx_f ; t.tz_f=t.z-t.tcz_f ; t.td_f=Sqrt(abs(t.tx_f*t.tx_f)+abs(t.tz_f*t.tz_f));
					t.tx_f=t.tx_f/t.td_f ; t.tz_f=t.tz_f/t.td_f;
					//      `x=x+(tx#*2.0) ; z=z+(tz#*2.0)
					t.x=t.x+(t.tx_f*3.5) ; t.z=t.z+(t.tz_f*3.5);
					AIAddObstacleVertex (  (t.x*50)+25,(t.z*50)+25 );
				}
			}
		}
		UnDim (  t.finalobs );
		if (  t.tobjstarted == 1 ) 
		{
			AIEndNewObstacle (  0,0,0 );
		}
	}

	//  free fillterraindot
	obs_freeterraindot ( );
}

void darkai_obstacles_terrain_refresh ( void )
{
	//  can delete obstacle and regenerate it
	if (  t.aisystem.generateobs == 1 ) 
	{
		if ( t.aisystem.usingdebugobjects == 1 ) darkai_destroyinternaldebugvisuals ( );
		AIRemoveObstacle (  t.aisystem.terrainobsnum );
		AICompleteObstacles (  );
		darkai_obstacles_terrain ( );
		AICompleteObstacles (  );
		if ( t.aisystem.usingdebugobjects == 1 ) darkai_createinternaldebugvisuals ( );
	}
}

void darkai_setup_characters ( void )
{
	//  Create A.I entities for all characters
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entid>0 ) 
		{
			if (  t.entityprofile[t.entid].ischaracter == 1 && t.entityelement[t.e].ragdollified == 0 ) 
			{
				t.tobj=t.entityelement[t.e].obj;
				if (  t.tobj>0 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						// 161115 - but only if not already part of the char anim list
						bool bFound = false;
						for ( int n = 1; n <= g.charanimindexmax; n++ )
						{
							if ( t.charanimstates[n].e == t.e )
							{
								bFound = true;
							}
						}
						if ( bFound==false )
						{
							//  Set up object one as character
							++g.charanimindexmax;
							g.charanimindex=g.charanimindexmax;
							Dim (  t.charanimcontrols,g.charanimindexmax  );
							Dim (  t.charanimstates,g.charanimindexmax );
							Dim2(  t.charactergunpose,g.charanimindexmax, 36 );
							t.charanimstates[g.charanimindex].obj=t.tobj;
							t.charanimstates[g.charanimindex].e=t.e;
							t.charanimstates[g.charanimindex].originale=t.e;
							darkai_setupcharacter ( );

							//  Load in any 'waste twist' data
							/* 301015 - monumentally not worked - instead use new UBER frames for future waste pivot
							t.charanimstates[g.charanimindex].usingcharacterposedat=0;
							t.ent_s=t.entitybank_s[t.entid] ; t.entpath_s=getpath(t.ent_s.Get());
							t.charactergunposefile_s=t.entdir_s+t.entpath_s+t.entityprofile[t.entid].model_s;
							t.charactergunposefile_s=Left(t.charactergunposefile_s.Get(),Len(t.charactergunposefile_s.Get())-2);
							t.charactergunposefile_s+=".dat";
							if (  FileExist(t.charactergunposefile_s.Get()) == 1 ) 
							{
								OpenToRead (  1,t.charactergunposefile_s.Get() );
								for ( t.i = 0 ; t.i<=  36; t.i++ )
								{
									t.a_f = ReadFloat (  1 ); t.charactergunpose[g.charanimindex][t.i].x=t.a_f;
									t.a_f = ReadFloat (  1 ); t.charactergunpose[g.charanimindex][t.i].y=t.a_f;
									t.a_f = ReadFloat (  1  ); t.charactergunpose[g.charanimindex][t.i].z=t.a_f;
								}
								CloseFile (  1 );
								t.charanimstates[g.charanimindex].usingcharacterposedat=1;
							}
							else
							{
							*/
							for ( t.i = 0 ; t.i<=  36; t.i++ )
							{
								t.charactergunpose[g.charanimindex][t.i].x=0;
								t.charactergunpose[g.charanimindex][t.i].y=0;
								t.charactergunpose[g.charanimindex][t.i].z=0;
							}
						}
					}
				}
			}
		}
	}
}

void darkai_destroy_all_characterdata ( void )
{
	//  ensure ALL character data is wiped
	// 161115 - only ADDS characters now (what about deleted characters during F9?)
	//g.charanimindexmax=0;
	//UnDim (  t.charanimcontrols );
	//UnDim (  t.charanimstates );
	//UnDim (  t.charactergunpose );
}

void darkai_release_characters ( void )
{
	//  Reset char max count
	g.charanimindexmax=0;
}

void darkai_setup_tree ( void )
{
	//  set up entity (ttreemode/tobj/tcontainerid/entid) - half height for all trees (so AI can see plr when stood behind tree)
	//  ttreemode ; 0=center on obj using slice 1/32th, 1=bottom root, 2-9=16th's from center downwards
	t.tworldstepy_f=(ObjectSizeY(t.tobj)/16.0);
	if (  t.ttreemode == 0  )  t.tworldy_f = (t.tworldstepy_f*0.25);
	if (  t.ttreemode == 1  )  t.tworldy_f = (t.tworldstepy_f*-0.5);
	if (  t.ttreemode>1  )  t.tworldy_f = (t.tworldstepy_f*8)-((t.ttreemode-2)*t.tworldstepy_f);
	//  0=create SINGLE Box (  that encompases this slice )
	t.tsimplebox=0;
	//  can skip obstacle generation
	if (  g.gskipobstaclecreation == 0 ) 
	{
		//  adding them in as full height so extra cover points arent made for every tree
		AIAddObstacleFromLevel (  t.tobj,t.tcontainerid,1,ObjectPositionY(t.tobj)+t.tworldy_f,t.tsimplebox,((t.entityprofile[t.entid].collisionscaling+0.0)/100.0) );
		//  this is eventually passed on the physics creation (perfect cylinder for tree trunk)
		t.entityelement[t.e].abscolx_f=AILastObstacleCenterX();
		t.entityelement[t.e].abscolz_f=AILastObstacleCenterZ();
		t.entityelement[t.e].abscolradius_f=AILastObstacleRadius();
	}
	else
	{
		t.entityelement[t.e].abscolx_f=ObjectPositionX(t.tobj);
		t.entityelement[t.e].abscolz_f=ObjectPositionZ(t.tobj);
		t.entityelement[t.e].abscolradius_f=ObjectSize(t.tobj,1);
	}
}

void darkai_setup_entity ( void )
{
	// set up entity (tobj/tfullheight/tcontainerid)
	// check if entity is more than plr-height above ground,
	// if so, cannot create obstacle on ground!
	t.tonground=1;
	if ( t.terrain.TerrainID>0 ) 
	{
		t.tusecurrentgroundheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.entityelement[t.e].x,t.entityelement[t.e].z);
	}
	else
	{
		t.tusecurrentgroundheight_f=1000.0;
	}
	if ( t.entityelement[t.e].y>t.tusecurrentgroundheight_f+75.0 ) 
	{
		t.tonground=0;
	}
	//  receives e,entid
	t.tobstype=0 ; t.tfullheight=1;
	if ( t.tonground == 1 ) 
	{
		if ( t.entityprofile[t.entid].forcesimpleobstacle == 0 ) 
		{
			if ( t.entityprofile[t.entid].collisionmode == 11 ) 
			{
				//  no obs
			}
			else
			{
				if (  t.entityprofile[t.entid].collisionmode == 40 ) 
				{
					t.tobstype=3;
				}
				else
				{
					if (  ObjectSizeX(t.tobj,1)>120 || ObjectSizeZ(t.tobj,1)>120 ) 
					{
						t.tobstype=2;
					}
					else
					{
						t.tobstype=1;
					}
				}
			}
		}
		else
		{
			t.tobstype=t.entityprofile[t.entid].forcesimpleobstacle;
		}
	}
	if (  t.tobstype>0 && g.gskipobstaclecreation == 0 ) 
	{
		if (  t.tobstype == 3 ) 
		{
			if (  t.aisystem.generateobs == 1 ) 
			{
				float forceobstacleslicemin = t.entityprofile[t.entid].forceobstaclepolysize;
				float forceobstaclesliceheight = t.entityprofile[t.entid].forceobstaclesliceheight;
				float forceobstaclesliceminsize = t.entityprofile[t.entid].forceobstaclesliceminsize;
				// 051718 - adjust if the entity is sunk into the floor (terrain height would slice higher)
				float fTerrainWouldSliceHigher = t.tusecurrentgroundheight_f - t.entityelement[t.e].y;
				if ( fTerrainWouldSliceHigher > 0 ) forceobstaclesliceheight += fTerrainWouldSliceHigher;
				darkai_addobstoallneededcontainers ( 3, t.tobj, t.tfullheight, forceobstacleslicemin, forceobstaclesliceheight, forceobstaclesliceminsize );
			}
			AIAddAlternateVisibilityObject (  t.tobj,0 );
		}
		if (  t.tobstype == 2 ) 
		{
			if (  t.aisystem.generateobs == 1 ) 
			{
				//AIAddStaticObstacle (  t.tobj,t.tfullheight,t.tcontainerid );
				darkai_addobstoallneededcontainers ( 2, t.tobj, t.tfullheight, 0.0f, 0.0f, 5.0f );
			}
			AIAddAlternateVisibilityObject (  t.tobj,0 );
		}
		if (  t.tobstype == 1 ) 
		{
			if (  ObjectExist(g.darkaiobsboxobject)  ==  1  ) DeleteObject (  g.darkaiobsboxobject ) ;
			MakeObjectBox (  g.darkaiobsboxobject , ObjectSizeX(t.tobj,1) ,ObjectSizeY(t.tobj,1),ObjectSizeZ(t.tobj,1) );
			OffsetLimb (   g.darkaiobsboxobject, 0, GetObjectCollisionCenterX(t.tobj) + LimbOffsetX(t.tobj,0),0,GetObjectCollisionCenterZ(t.tobj) + LimbOffsetZ(t.tobj,0) );
			PositionObject (  g.darkaiobsboxobject,ObjectPositionX(t.tobj),ObjectPositionY(t.tobj),ObjectPositionZ(t.tobj) );
			RotateObject (  g.darkaiobsboxobject , ObjectAngleX(t.tobj),ObjectAngleY(t.tobj),ObjectAngleZ(t.tobj) );
			if (  ObjectSizeY(t.tobj,1)  <=  50  )  t.tfullheight  =  0;
			if (  t.aisystem.generateobs == 1 ) 
			{
				//AIAddStaticObstacle (  g.darkaiobsboxobject,t.tfullheight,t.tcontainerid );
				darkai_addobstoallneededcontainers ( 1, g.darkaiobsboxobject, t.tfullheight, 0.0f, 0.0f, 5.0f );
			}
			AIAddAlternateVisibilityObject (  g.darkaiobsboxobject,0 );
			DeleteObject (  g.darkaiobsboxobject );
		}
	}
}

void darkai_addobstoallneededcontainers ( int iType, int iObj, int iFullHeight, float fMinHeight, float fSliceHeight, float fSliceMinSize )
{
	// go through all zones (zone zero is container zero)
	for ( t.twaypointindex = 0; t.twaypointindex <= g.waypointmax; t.twaypointindex++ )
	{
		if ( t.twaypointindex == 0 || t.waypoint[t.twaypointindex].style == 3 ) 
		{
			if ( t.twaypointindex == 0 || t.waypoint[t.twaypointindex].count > 0 ) 
			{
				// work out center of whole zone and radius to outer most node
				bool bIncludeInContainer = false;
				if ( t.twaypointindex == 0 )
				{
					// container zero zone is everything on the terrain floor
					bIncludeInContainer = true;
				}
				else
				{
					// sub-zones have boundaries
					int iAvCount = 0;
					float fAvX = 0.0f;
					float fAvZ = 0.0f;
					for ( t.w = t.waypoint[t.twaypointindex].finish-1; t.w >= t.waypoint[t.twaypointindex].start; t.w-- )
					{
						fAvX += t.waypointcoord[t.w].x;
						fAvZ += t.waypointcoord[t.w].z;
						iAvCount++;
					}
					if ( iAvCount > 0 )
					{
						// this is the center, now thr radius
						fAvX /= iAvCount;
						fAvZ /= iAvCount;
						float fZoneRadius = 0.0;
						for ( t.w = t.waypoint[t.twaypointindex].finish-1; t.w >= t.waypoint[t.twaypointindex].start; t.w-- )
						{
							float fDX = t.waypointcoord[t.w].x - fAvX;
							float fDZ = t.waypointcoord[t.w].z - fAvZ;
							float fDD = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
							if ( fDD > fZoneRadius ) fZoneRadius = fDD;
							iAvCount++;
						}

						// get distance from this entity and zone center
						float fDX = ObjectPositionX ( iObj ) - fAvX;
						float fDZ = ObjectPositionZ ( iObj ) - fAvZ;
						float fDistanceBetweenZoneAndEnt = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
						float fEntRadius = GetObjectCollisionRadius ( iObj );
						if ( fDistanceBetweenZoneAndEnt < fZoneRadius+fEntRadius )
							bIncludeInContainer = true;
					}
				}
				if ( bIncludeInContainer == true )
				{
					// yes, point is inside this zone
					int iContainerID = t.twaypointindex;
					switch ( iType )
					{
						case 1 : AIAddStaticObstacle ( iObj, iFullHeight, iContainerID ); break;
						case 2 : AIAddStaticObstacle ( iObj, iFullHeight, iContainerID ); break;
						case 3 : AIAddObstacleFromLevel ( iObj, iContainerID, iFullHeight, ObjectPositionY(iObj)+fSliceHeight, fSliceMinSize, fMinHeight, 0 ); break;
					}
				}
			}
		}
	}
}

void darkai_staggerAIprocessing ( void )
{
	//  ensures AI processing spread evenly across vision delay
	t.tstep_f=2000.0/g.charanimindexmax;
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		t.charanimstates[g.charanimindex].visiondelaylastime=Timer()+(g.charanimindex*t.tstep_f);
	}
}

void darkai_setupcharacter ( void )
{
	//  Entity profile index
	t.ttentid=t.entityelement[t.charanimstates[g.charanimindex].e].bankindex;

	//  Setup character defaults
	t.charanimstates[g.charanimindex].playcsi=g.csi_limbo;
	t.charanimstates[g.charanimindex].limbomanualmode=0;
	t.charanimstates[g.charanimindex].alerted=0;
	t.charanimstates[g.charanimindex].realheadangley_f=0.0;
	t.charanimstates[g.charanimindex].animationspeed_f=(65.0/100.0)*t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.animspeed;
	if (  t.charanimstates[g.charanimindex].animationspeed_f <= 0 ) 
	{
		//  300615 - character animation speed should never be zero (legacy maps)
		t.charanimstates[g.charanimindex].animationspeed_f=65.0;
	}
	t.charanimstates[g.charanimindex].outofrange=1;
	t.charanimstates[g.charanimindex].currentangle_f=t.entityelement[t.charanimstates[g.charanimindex].e].ry;
	t.charanimstates[g.charanimindex].moveangle_f=t.charanimstates[g.charanimindex].currentangle_f;
	t.charanimstates[g.charanimindex].visiondelay=2000;
	t.charanimstates[g.charanimindex].visiondelaylastime=0;

	//  By default, characters have default PISTOL weapon style OR new WEAPSTYLE value
	t.tgunid=t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.hasweapon;
	if (  t.entityprofile[t.ttentid].usesweapstyleanims == 0 ) 
	{
		if (  t.tgunid>0 ) 
		{
			t.charanimstates[g.charanimindex].weapstyle=1;
		}
		else
		{
			t.charanimstates[g.charanimindex].weapstyle=0;
		}
	}
	else
	{
		//  1-pistol, 2-rocket, 3-shotgun, 4-uzi, 5-assault
		t.charanimstates[g.charanimindex].weapstyle=t.gun[t.tgunid].weapontype;
	}

	//  populate character with weapon details
	t.tgunid=t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.hasweapon;
	t.charanimstates[g.charanimindex].ammoinclipmax=g.firemodes[t.tgunid][0].settings.reloadqty;
	t.charanimstates[g.charanimindex].ammoinclip=t.charanimstates[g.charanimindex].ammoinclipmax;
	if (  t.charanimstates[g.charanimindex].ammoinclip>0 ) 
	{
		//  allows characters to reload at different times
		t.charanimstates[g.charanimindex].ammoinclip=1+Rnd(t.charanimstates[g.charanimindex].ammoinclip-1);
	}

	//  Set collision property
	SetObjectCollisionProperty (  t.charanimstates[g.charanimindex].obj,0 );

	//  if this character is controlled by third person control, override
	if (  t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.charactere == t.charanimstates[g.charanimindex].e ) 
	{
		//  THIRD PERSON CONTROL
		t.playercontrol.thirdperson.characterindex=g.charanimindex;
		int iWeaponOrDefault = t.charanimstates[g.charanimindex].weapstyle;
		if ( iWeaponOrDefault < 1 ) iWeaponOrDefault = 1; // for zombies and barbarians who have no weapons
		t.charanimstates[g.charanimindex].weapstyle = iWeaponOrDefault; // so related code can find csi anim frames for TPP
		t.charanimstates[g.charanimindex].playcsi=t.csi_stood[iWeaponOrDefault];
		t.charanimstates[g.charanimindex].e=0;
		t.charanimstates[g.charanimindex].rocketstyle=0;
		physics_player_thirdpersonreset ( );
	}
	else
	{
		//  REGULAR AI
		//  determine if character holds 'gun' or 'rocket' style weapon
		if (  t.charanimstates[g.charanimindex].weapstyle <= 1 ) 
		{
			//  only if older legacy character (newer Uber characters use weapstyle=2)
			t.charanimstates[g.charanimindex].rocketstyle=0;
			if (  t.tgunid>0 ) 
			{
				if (  g.firemodes[t.tgunid][0].settings.flakindex>0 ) 
				{
					t.charanimstates[g.charanimindex].rocketstyle=1;
				}
			}
		}

		//  Hard-Code ENEMY or NEUTRAL in entity profile (can be changed via LUA scripting)
		t.charanimstates[g.charanimindex].aiobjectexists=1;
		t.i=t.charanimstates[g.charanimindex].obj;
		AIAddNeutral ( t.i, 0 );

		// Default A.I entity settings
		t.tx_f = ObjectPositionX(t.i);
		t.ty_f = ObjectPositionY(t.i);
		t.tz_f = ObjectPositionZ(t.i);
		AISetEntityPosition (  t.i,t.tx_f,t.ty_f,t.tz_f );
		t.tconeangle=t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.coneangle;
		//if (  t.tconeangle == 0  )  t.tconeangle = 90;
		if (  t.tconeangle == 0  )  t.tconeangle = 179; // change default so characters more responsive (can see behind them)
		AISetEntityFireArc (  t.i,t.tconeangle );
		AISetEntityViewArc (  t.i,t.tconeangle,t.tconeangle*2 );
		t.tconerange=t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.conerange;
		if (  t.tconerange == 0  )  t.tconerange = 800*2;
		AISetEntityViewRange (  t.i,t.tconerange );
		AISetEntityHearingRange (  t.i,100 );
		AISetEntityHeight (  t.i,t.aisystem.defaulteyeheight_f );
		AISetEntityCanHear (  t.i,1 );
		AISetEntityAvoidDistance (  t.i,0 );
		AISetEntityCanHideBehindCorner (  t.i,0 );
		AISetEntityCanDuck (  t.i,0 );
		AISetEntityCanAttack (  t.i,0 );
		AISetEntityCanStrafe (  t.i,0 );
		AISetEntityCanSearch (  t.i,0 );
		AISetEntityCanRoam (  t.i,0 );

		// set propertional to entity move speed (100=750)
		float fTurnSpeed = t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.speed * 7.5f;
		AISetEntityTurnSpeed ( t.i, fTurnSpeed );

		// set the starting container for this character (based on feet proximity to any navmeshzone in level)
		int iContainerStartIndex = 0;
		for ( t.waypointindex = 1; t.waypointindex <= g.waypointmax; t.waypointindex++ )
		{
			if ( waypoint_ispointinzoneex ( t.waypointindex, t.tx_f, t.ty_f, t.tz_f, 1 ) == 1 ) 
			{
				iContainerStartIndex = t.waypointindex;
				break;
			}
		}
		AISetEntityContainer ( t.i, iContainerStartIndex );

		// Command A.I entity to manually stop at start location
		AISetEntityControl ( t.i, 0 );
		AIEntityStopNoMoveAddition ( t.i );
		AISetEntityAngleY ( t.i,t.charanimstates[g.charanimindex].currentangle_f );

		// set whether AI bot is ALWAYS active (for distant enemies on paths, etc)
		if ( t.entityelement[t.charanimstates[g.charanimindex].e].eleprof.phyalways == 0 )
			AISetEntityAlwaysActive ( t.i, false );
		else
			AISetEntityAlwaysActive ( t.i, true );

		// by default, avoidance plotting is instant avoidance
		// NOTE: Real-Time (mode 1) entities will avoid each other and check for other nearby entities every update, 
		// constantly changing positions can result in an entity jumping about as its movement direction changes every update. 
		// This was the only mode in the original release version and is default.
		// PROBLEM is that they PUSH AI out of zones and INTO obstacles, so this needs to be tied to path generation
		// if MODE 1 is to be used again!
		AISetEntityAvoidMode ( t.i, 99 );
	}
}

void darkai_staywithzone ( int iAIObj, float fLastX, float fLastZ, float* pX, float* pZ )
{
	// when move entity under AI control, ensure ALWAYS stay within containing zone if any
	AIStayWithinContainer ( iAIObj, fLastX, fLastZ, pX, pZ );
}

void darkai_makesound ( void )
{
	//  tsx#,tsz#,tradius#
	AICreateSound (  t.tsx_f , t.tsz_f , (int)t.tradius_f/10 ,t.tradius_f , -1 );
}

void darkai_makeexplosionsound ( void )
{
	//  tsx#,tsz#
	AICreateSound (  t.tsx_f,t.tsz_f,(int)200,200.0f,-1 );
}

void darkai_shootplayer ( void )
{
	//  takes tcharanimindex
	//  recalc PLRVISIBLE to ensure the enemy can truly STILL see PLAYER (VisionDelay can make this value erroneous)
	darkai_calcplrvisible ( );
	//  takes charanimstate, if A.I not currently firing
	//  if want to shoot, can override firesound in use (otherwise can wait 7 seconds while sound fades)
	t.tpermitanoverride=0;
	if (  t.charanimstate.firesoundindex>0 && Timer()>(int)t.charanimstate.firesoundstarted+50  )  t.tpermitanoverride = 1;
	if (  (t.charanimstate.firesoundindex == 0 || t.tpermitanoverride == 1) && t.entityelement[t.charanimstate.e].plrvisible == 1 ) 
	{
		//  handle player being shot at
		t.te=t.charanimstate.e;
		if (  t.te>0 ) 
		{
			t.tentid=t.entityelement[t.te].bankindex;
			t.tgunid=t.entityprofile[t.tentid].hasweapon;
			t.tcannotfirenow=0;
			if (  t.charanimstate.playcsi >= t.csi_crouchmovefore[t.charanimstate.weapstyle] && t.charanimstate.playcsi <= t.csi_crouchmoverun[t.charanimstate.weapstyle] ) 
			{
				//  cannot fire if crouch moving
				t.tcannotfirenow=1;
			}
			t.tattachedobj=t.entityelement[t.te].attachmentobj;
			if (  t.tattachedobj>0 ) 
			{
				//  cannot fire if weapon not pointing at player
				t.tattachmentobjfirespotlimb=t.entityelement[t.te].attachmentobjfirespotlimb;
				if (  t.tgunid>0 && t.tattachmentobjfirespotlimb>0 ) 
				{
					t.tx_f=LimbPositionX(t.tattachedobj,t.tattachmentobjfirespotlimb)-ObjectPositionX(t.aisystem.objectstartindex);
					t.ty_f=LimbPositionY(t.tattachedobj,t.tattachmentobjfirespotlimb)-ObjectPositionY(t.aisystem.objectstartindex);
					t.tz_f=LimbPositionZ(t.tattachedobj,t.tattachmentobjfirespotlimb)-ObjectPositionZ(t.aisystem.objectstartindex);
					t.tdist_f=Sqrt(abs(t.tx_f*t.tx_f)+abs(t.ty_f*t.ty_f)+abs(t.tz_f*t.tz_f));
					if (  ObjectExist(g.projectorsphereobjectoffset) == 0 ) 
					{
						MakeObjectSphere (  g.projectorsphereobjectoffset,10 );
						HideObject (  g.projectorsphereobjectoffset );
					}
					PositionObject (  g.projectorsphereobjectoffset,LimbPositionX(t.tattachedobj,t.tattachmentobjfirespotlimb),LimbPositionY(t.tattachedobj,t.tattachmentobjfirespotlimb),LimbPositionZ(t.tattachedobj,t.tattachmentobjfirespotlimb) );
					RotateObject (  g.projectorsphereobjectoffset,LimbDirectionX(t.tattachedobj,t.tattachmentobjfirespotlimb),LimbDirectionY(t.tattachedobj,t.tattachmentobjfirespotlimb),LimbDirectionZ(t.tattachedobj,t.tattachmentobjfirespotlimb) );
					MoveObject (  g.projectorsphereobjectoffset,t.tdist_f*-1 );
					t.tx_f=ObjectPositionX(g.projectorsphereobjectoffset)-ObjectPositionX(t.aisystem.objectstartindex);
					t.ty_f=ObjectPositionY(g.projectorsphereobjectoffset)-ObjectPositionY(t.aisystem.objectstartindex);
					t.tz_f=ObjectPositionZ(g.projectorsphereobjectoffset)-ObjectPositionZ(t.aisystem.objectstartindex);
					t.tdist2_f=Sqrt(abs(t.tx_f*t.tx_f)+abs(t.ty_f*t.ty_f)+abs(t.tz_f*t.tz_f));
					t.tactualdistance_f=t.tdist2_f;
					t.tdist2_f=t.tdist2_f/t.tdist_f;
					t.tdist2_f=int(t.tdist2_f*100);
					if (  t.playercontrol.thirdperson.enabled == 1 ) 
					{
						//  can be less accurate and still hit in third person (no camera scrutiny)
						if (  t.tdist2_f>100  )  t.tcannotfirenow = 1;
					}
					else
					{
						if (  t.tdist2_f>50 && t.tactualdistance_f>100.0  )  t.tcannotfirenow = 1;
					}
				}
			}
			if (  t.tgunid>0 && t.tcannotfirenow == 0 ) 
			{
				//  frequenty of fire
				t.ttratecalc_f=(1.0/(1.0+g.firemodes[t.tgunid][0].settings.firerate))*g.timeelapsed_f*2.0;
				t.charanimstate.firerateaccumilator=t.charanimstate.firerateaccumilator-t.ttratecalc_f;
				if (  t.charanimstate.firerateaccumilator<0.0 ) 
				{
					t.charanimstate.firerateaccumilator=0.5+(Rnd(100)/100.0);
					t.ttrr=Rnd(1);
					for ( t.tt = t.ttrr+0 ; t.tt<=  t.ttrr+1; t.tt++ )
					{
						t.ttsnd=t.gunsoundcompanion[t.tgunid][1][t.tt].soundid;
						if (  t.ttsnd>0 ) 
						{
							if (  SoundExist(t.ttsnd) == 1 ) 
							{
								if (  SoundPlaying(t.ttsnd) == 0 || t.tt == t.ttrr+1 ) 
								{
									t.toldsndid=t.charanimstate.firesoundindex;
									if (  t.toldsndid>0 ) 
									{
										if (  SoundExist(t.toldsndid) == 1 ) 
										{
											StopSound (  t.toldsndid );
										}
									}
									t.charanimstate.firesoundindex=t.ttsnd ; t.tt=3;
									t.tfireloopend=g.firemodes[t.tgunid][0].sound.fireloopend;
									t.charanimstate.firesoundstarted=Timer();
									if (  t.tfireloopend>0 && t.game.runasmultiplayer == 0 ) 
									{
										//  sound loops (need to cap it off)
										t.charanimstate.firesoundexpiry=Timer()+200+Rnd(200);
									}
									else
									{
										//  can let sound fade out slowly naturally
										t.charanimstate.firesoundexpiry=Timer()+5000;
									}
								}
							}
						}
					}
					if (  t.charanimstate.firesoundindex>0 ) 
					{
						//  shoot effects
						t.tgunid=t.entityelement[t.te].eleprof.hasweapon;
						t.tattachedobj=t.entityelement[t.te].attachmentobj;
						darkai_shooteffect ( );
					}
				}
			}
		}
	}
}

void darkai_shooteffect ( void )
{
	//  needs tgunid, example; tgunid=entityprofile(tentid).hasweapon
	//  needs tattachedobj, example; tattachedobj=entityelement(te).attachmentobj
	//  needs te (entityelement index), example; te = e
	//  charanimstate.firesoundindex needs to be set, examle; charanimstate.firesoundindex=ttsnd

	//  Because of coop mode, the ai may not always be shooting at the camera
	if (  t.game.runasmultiplayer == 0 || g.steamworks.coop == 0 ) 
	{
		t.tplayerx_f = ObjectPositionX(t.aisystem.objectstartindex);
		t.tplayery_f = ObjectPositionY(t.aisystem.objectstartindex);
		t.tplayerz_f = ObjectPositionZ(t.aisystem.objectstartindex);
	}
	else
	{
		t.ttentid=t.entityelement[t.te].bankindex;
		//  If they are not a player, they must be ai, so grab their gunid and attacheobj
		if (  t.entityprofile[t.ttentid].ismultiplayercharacter  ==  0 ) 
		{
			t.tgunid=t.entityprofile[t.ttentid].hasweapon;
			t.tattachedobj=t.entityelement[t.te].attachmentobj;
		}
		if (  t.entityelement[t.te].mp_coopControlledByPlayer  ==  g.steamworks.me || t.entityelement[t.te].mp_coopControlledByPlayer  ==  -1 || t.entityprofile[t.ttentid].ismultiplayercharacter  ==  1 ) 
		{
			t.tplayerx_f = CameraPositionX(0);
			t.tplayery_f = CameraPositionY(0);
			t.tplayerz_f = CameraPositionZ(0);
		}
		else
		{
			t.tsteamplayer = t.entityelement[t.te].mp_coopControlledByPlayer;
			t.tplayerx_f = ObjectPositionX(t.entityelement[t.steamworks_playerEntityID[t.tsteamplayer]].obj);
			t.tplayery_f = ObjectPositionY(t.entityelement[t.steamworks_playerEntityID[t.tsteamplayer]].obj);
			t.tplayerz_f = ObjectPositionZ(t.entityelement[t.steamworks_playerEntityID[t.tsteamplayer]].obj);
		}
	}

	//  emit spot flash
	if (  t.tattachedobj>0 ) 
	{
		//  best coordinate is firespot on attached gun
		t.tokay=0;
		t.tattachmentobjfirespotlimb=t.entityelement[t.te].attachmentobjfirespotlimb;
		if (  t.tgunid>0 && t.tattachmentobjfirespotlimb>0 ) 
		{
			t.tx_f=LimbPositionX(t.tattachedobj,t.tattachmentobjfirespotlimb);
			t.ty_f=LimbPositionY(t.tattachedobj,t.tattachmentobjfirespotlimb);
			t.tz_f=LimbPositionZ(t.tattachedobj,t.tattachmentobjfirespotlimb);
			t.tokay=1;
		}
		if (  t.tokay == 0 ) 
		{
			//  actual gun position is better source coordinate
			t.tx_f=ObjectPositionX(t.tattachedobj);
			t.ty_f=ObjectPositionY(t.tattachedobj);
			t.tz_f=ObjectPositionZ(t.tattachedobj);
		}
	}
	else
	{
		//  fallback is entity center
		t.tobj=t.entityelement[t.te].obj;
		t.tx_f=ObjectPositionX(t.tobj);
		t.ty_f=ObjectPositionY(t.tobj)+50.0;
		t.tz_f=ObjectPositionZ(t.tobj);
	}
	t.tcolr=g.firemodes[t.entityelement[t.te].eleprof.hasweapon][0].settings.muzzlecolorr/5;// /2; 100718 - tone it down a touch
	t.tcolg=g.firemodes[t.entityelement[t.te].eleprof.hasweapon][0].settings.muzzlecolorg/5;// /2;
	t.tcolb=g.firemodes[t.entityelement[t.te].eleprof.hasweapon][0].settings.muzzlecolorb/5;// /2;
	lighting_spotflash_forenemies ( );

	//  initiate decal
	t.decalid=g.firemodes[t.tgunid][0].decalid;
	g.decalx=t.tx_f ; g.decaly=t.ty_f ; g.decalz=t.tz_f;
	t.decalscalemodx=0 ; t.decalorient=11;
	t.originatore=-1;
	t.originatorobj=t.tattachedobj;
	t.decalforward=g.firemodes[t.tgunid][0].settings.decalforward;
	if (  g.firemodes[t.tgunid][0].action.automatic.s>0 ) 
	{
		//  special instruction for decal to loop X times
		t.decalburstloop=4;
	}
	else
	{
		t.decalburstloop=0;
	}
	decalelement_create ( );
	t.decalburstloop=0;

	//  emit sound
	t.tsx_f=t.entityelement[t.te].x ; t.tsz_f=t.entityelement[t.te].z ; t.tradius_f=200;
	darkai_makesound ( );
	t.ttsnd=t.charanimstate.firesoundindex;
	if (  t.ttsnd>0 ) 
	{
		if (  SoundExist(t.ttsnd) == 1 ) 
		{
			t.tfireloopend=g.firemodes[t.tgunid][0].sound.fireloopend;
			if (  t.tfireloopend>0 && t.game.runasmultiplayer == 0 ) 
			{
				PlaySoundOffset (  t.ttsnd,t.tfireloopend  ); LoopSound (  t.ttsnd,0,t.tfireloopend );
			}
			else
			{
				PlaySound (  t.ttsnd );
			}
			PositionSound (  t.ttsnd,t.entityelement[t.te].x,t.entityelement[t.te].y,t.entityelement[t.te].z );
			t.tvolume_f=soundtruevolume(95.0);
			SetSoundVolume (  t.ttsnd,t.tvolume_f );
			SetSoundSpeed (  t.ttsnd,43000+Rnd(2000) );
		}
	}

	//  is bullet or flak
	t.tflakid=g.firemodes[t.tgunid][0].settings.flakindex;
	if (  t.tflakid == 0 ) 
	{
		//  BULLET
		//  determine if bullet hit based on distance (ttdistanceaccuracy# lower is better)
		t.ttdx_f=t.tplayerx_f-t.tx_f;
		t.ttdy_f=t.tplayery_f-t.ty_f;
		t.ttdz_f=t.tplayerz_f-t.tz_f;
		t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
		t.ttdistanceaccuracy_f=t.ttdd_f/800.0;
		if (  t.ttentid>0  )  t.tisnotmpchar  =  t.entityprofile[t.ttentid].ismultiplayercharacter; else t.tisnotmpchar  =  0;
		if (  t.game.runasmultiplayer == 0 || ( g.steamworks.coop  ==  1 && t.tisnotmpchar  ==  0 ) ) 
		{
			if (  t.aisystem.playerducking == 1 ) 
			{
				if (  t.playercontrol.movement == 0 ) 
				{
					t.tchancetohit_f=4.0;
				}
				else
				{
					t.tchancetohit_f=12.0;
				}
			}
			else
			{
				if (  t.playercontrol.movement == 0 ) 
				{
					t.tchancetohit_f=2.0;
				}
				else
				{
					t.tchancetohit_f=6.0;
				}
			}
			if (  t.ttdistanceaccuracy_f<0.3 || Rnd(t.tchancetohit_f*t.ttdistanceaccuracy_f) == 0 ) 
			{
				//  amount of damage to player
				if (  t.game.runasmultiplayer  ==  0 || ( g.steamworks.coop  ==  1 && t.entityelement[t.te].mp_coopControlledByPlayer  ==  g.steamworks.me ) ) 
				{
					t.tdamage=g.firemodes[t.tgunid][0].settings.damage;
					physics_player_takedamage ( );
				}
			}
			else
			{
				//  play bullet whiz sound because the AI missed
				t.tSndID = t.playercontrol.soundstartindex+25 + Rnd(3);
				t.tSndX_f = t.tplayerx_f - 100 + Rnd(200);
				t.tSndY_f = t.tplayery_f - 100 + Rnd(200);
				t.tSndZ_f = t.tplayerz_f - 100 + Rnd(200);
				PositionSound (  t.tSndID,t.tSndX_f,t.tSndY_f,t.tSndZ_f );
				SetSoundVolume (  t.tSndID,soundtruevolume(100) );
				SetSoundSpeed (  t.tSndID,36000 + Rnd(10000) );
				PlaySound (  t.tSndID );
			}
		}
	}
	else
	{
		//  FLAK (projectile)
		//  find starting GetPoint (  for projectile )
		t.tobj=t.entityelement[t.te].attachmentobj;
		if (  t.tobj>0 ) 
		{
			t.flakx_f=LimbPositionX(t.tobj,t.entityelement[t.te].attachmentobjfirespotlimb);
			t.flaky_f=LimbPositionY(t.tobj,t.entityelement[t.te].attachmentobjfirespotlimb);
			t.flakz_f=LimbPositionZ(t.tobj,t.entityelement[t.te].attachmentobjfirespotlimb);
			t.tdx_f=t.tplayerx_f-t.flakx_f;
			t.tdy_f=t.tplayery_f-t.flaky_f;
			t.tdz_f=t.tplayerz_f-t.flakz_f;
			t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
			t.flakangle_f=atan2deg(t.tdx_f,t.tdz_f)+(Rnd(4)-2);
			t.flakpitch_f=((t.tdy_f/t.tdd_f)*-35.0)+Rnd(4)-2;

			t.tsteamismpchar = 0;
			t.ttzentid=t.entityelement[t.te].bankindex;
			if (  t.ttzentid>0  )  t.tsteamismpchar  =  t.entityprofile[t.ttzentid].ismultiplayercharacter;

			//  create and launch projectile
			if (  t.game.runasmultiplayer == 0 || t.tsteamismpchar  ==  0 ) 
			{
				//t.tProjectileType=1; characters can shoot ANY projectile type now
				int iStoreGunID = t.gunid;
				t.gunid = t.tgunid;
				t.tProjectileType_s=t.gun[t.gunid].projectile_s; weapon_getprojectileid ( );
				t.tSourceEntity=t.te ; t.tTracerFlag=0;
				t.tStartX_f=t.flakx_f ; t.tStartY_f=t.flaky_f ; t.tStartZ_f=t.flakz_f;
				t.tAngX_f=t.flakpitch_f ; t.tAngY_f=t.flakangle_f ; t.tAngZ_f=0;
				weapon_projectile_make ( );
				t.gunid = iStoreGunID;
			}
		}
		t.tolde = t.e;
		t.e = t.te;
		entity_lua_findcharanimstate ( );
		t.e=t.tolde;
		if (  t.game.runasmultiplayer == 0 || t.tsteamismpchar  ==  0 ) 
		{
			// if not AI manual mode
			if ( t.charanimstate.limbomanualmode != 1 )
			{
				// flak can trigger recoil anim in character if not moving
				if ( t.tcharanimindex  !=  -1 ) 
				{
					if ( t.charanimcontrols[t.tcharanimindex].moving == 0 ) 
					{
						// use INSTANT method too!
						t.charanimcontrols[t.tcharanimindex].spotactioning=4;
						t.smoothanim[t.charanimstate.obj].transition=0;
						if (  t.charanimstate.playcsi >= t.csi_crouchidle[t.charanimstate.weapstyle] && t.charanimstate.playcsi <= t.csi_crouchgetuprocket[t.charanimstate.weapstyle] ) 
						{
							t.charanimstate.playcsi=t.csi_crouchfirerocket[t.charanimstate.weapstyle];
						}
						else
						{
							t.charanimstate.playcsi=t.csi_stoodfirerocket[t.charanimstate.weapstyle];
						}
					}
				}
			}
		}
		//  deduct one unit of ammo (only if npc oes NOT ignore need to reload)
		if (  g.firemodes[t.tgunid][0].settings.npcignorereload == 0 ) 
		{
			t.charanimstate.ammoinclip=t.charanimstate.ammoinclip-1;
			if (  t.charanimstate.ammoinclip<0  )  t.charanimstate.ammoinclip = 0;
		}
	}

	if (  t.game.runasmultiplayer  ==  1 && g.steamworks.coop  ==  1 && t.tLuaDontSendLua  ==  0 ) 
	{
		SteamSendLua (  Steam_LUA_FireWeaponEffectOnly,t.te,0 );
	}
}

void darkai_killai ( void )
{
	if (  t.charanimstates[t.tcharanimindex].aiobjectexists == 1 ) 
	{
		//  Attempt to call the _exit function for the characters script
		if (  t.entityelement[t.charanimstates[t.tcharanimindex].e].eleprof.aimain == 1 ) 
		{
			t.strwork = Lower(t.entityelement[t.charanimstates[t.tcharanimindex].e].eleprof.aimainname_s.Get());
			t.strwork += "_exit";
			LuaSetFunction ( t.strwork.Get() ,1,0 );
			LuaPushInt (  t.charanimstates[t.tcharanimindex].e  ); LuaCallSilent (  );
		}

		// remove AI from AI system
		AIKillEntity ( t.charanimstates[t.tcharanimindex].obj );

		// free this AI from the game loop
		t.charanimstates[t.tcharanimindex].aiobjectexists=0;
		if ( t.entityelement[t.charanimstates[t.tcharanimindex].e].usingphysicsnow != 0 ) 
		{
			t.tphyobj=t.charanimstates[t.tcharanimindex].obj ; physics_disableobject ( );
			t.entityelement[t.charanimstates[t.tcharanimindex].e].usingphysicsnow=0;
		}
		SetObjectCollisionProperty ( t.charanimstates[t.tcharanimindex].obj,1 );
	}

	//  reset any limbs of character
	if (  t.entityelement[t.charanimstates[t.tcharanimindex].e].health>0 ) 
	{
		t.headlimbofcharacter=t.entityprofile[t.entityelement[t.charanimstates[t.tcharanimindex].e].bankindex].headlimb;
		if (  t.headlimbofcharacter>0 ) 
		{
			if (  LimbExist(t.charanimstates[t.tcharanimindex].obj,t.headlimbofcharacter) == 1 ) 
			{
				RotateLimb (  t.charanimstates[t.tcharanimindex].obj,t.headlimbofcharacter,0,0,0 );
			}
		}
		t.spinelimbofcharacter=t.entityprofile[t.entityelement[t.charanimstates[t.tcharanimindex].e].bankindex].spine;
		if (  t.spinelimbofcharacter>0 ) 
		{
			if (  LimbExist(t.charanimstates[t.tcharanimindex].obj,t.spinelimbofcharacter) == 1 ) 
			{
				RotateLimb (  t.charanimstates[t.tcharanimindex].obj,t.spinelimbofcharacter,0,0,0 );
			}
		}
	}

	//  reset any looping/sounds
	t.ttsnd=t.charanimstates[t.tcharanimindex].firesoundindex;
	t.charanimstates[t.tcharanimindex].firesoundindex=0;
	if (  t.ttsnd>0 ) 
	{
		if (  SoundExist(t.ttsnd) == 1 ) 
		{
			StopSound (  t.ttsnd );
		}
	}
}

void darkai_shootcharacter ( void )
{
	//  create sound for A.I when shot made
	t.tsx_f=t.twhox_f ; t.tsz_f=t.twhoz_f ; t.tradius_f=200 ;darkai_makesound ( );
	//  receives charanimindex tobj tdamage twhox# twhoy# twhoz#
	if (  t.entityelement[t.charanimstates[g.charanimindex].e].health>0 ) 
	{
		//  handle shooting of character
		t.ttte=t.charanimstates[g.charanimindex].e;
		t.tdamage=g.firemodes[t.gunid][g.firemode].settings.damage ; t.tdamageforce=t.tforce_f;
		// 100415 - melee attack can override damage
		// 011215 - specify damage using fire mode zero default
		if (  t.gun[t.gunid].settings.ismelee == 2  )  
			t.tdamage = g.firemodes[t.gunid][0].settings.meleedamage;

		t.tdamagesource=1;
		entity_applydamage ( );
	}
}

void darkai_calcplrvisible ( void )
{
	// if the ai is controlled by another player, we can just set as visible here
	if ( t.game.runasmultiplayer  ==  1 ) 
	{
		if ( t.entityelement[t.charanimstate.e].mp_coopControlledByPlayer != g.steamworks.me ) 
		{
			t.entityelement[t.charanimstate.e].plrvisible=0;
			t.entityelement[t.charanimstate.e].lua.flagschanged=1;
			return;
		}
	}

	// takes tcharanimindex
	// work out if entity A.I can see (stored until recalculated) (called from _darkai_loop and _darkai_shootplayer)
	t.entityelement[t.charanimstate.e].plrvisible=0;
	t.entityelement[t.charanimstate.e].lua.flagschanged=1;
	if ( t.player[t.plrid].health > 0 ) 
	{
		// work out distance between player and entity
		t.ttdx_f=ObjectPositionX(t.aisystem.objectstartindex)-ObjectPositionX(t.charanimstate.obj);
		t.ttdz_f=ObjectPositionZ(t.aisystem.objectstartindex)-ObjectPositionZ(t.charanimstate.obj);
		t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdz_f*t.ttdz_f));
		if (  t.ttdd_f<1500.0 ) 
		{
			//  player within 1500 units, otherwise skip further vis checking
			t.ttda_f=atan2deg(t.ttdx_f,t.ttdz_f);
			t.ttdiff_f=WrapValue(t.ttda_f)-WrapValue(ObjectAngleY(t.charanimstate.obj));
			if ( t.ttdiff_f<-180 ) t.ttdiff_f = t.ttdiff_f+360;
			if ( t.ttdiff_f>180 ) t.ttdiff_f = t.ttdiff_f-360;
			t.tconeangle=t.entityelement[t.charanimstate.e].eleprof.coneangle;
			if ( t.tconeangle == 0  ) t.tconeangle = 179; // new default
			if ( abs(t.ttdiff_f) <= t.tconeangle ) 
			{
				// and player is within hemisphere of entity look angle
				t.tgetentcanseevalue=AIGetEntityCanSee(t.charanimstate.obj,ObjectPositionX(t.aisystem.objectstartindex),ObjectPositionY(t.aisystem.objectstartindex),ObjectPositionZ(t.aisystem.objectstartindex),1);
				if ( t.tgetentcanseevalue>0 )
				{
					// player can be seen within inner arc
					t.ttokay=1;
					t.tthavegunobject=0;
					t.tgunobj=t.entityelement[t.charanimstate.e].attachmentobj;
					if ( t.tgunobj>0 ) 
					{
						if ( ObjectExist(t.tgunobj) == 1 ) 
						{
							t.brayx1_f=ObjectPositionX(t.tgunobj);
							t.brayy1_f=ObjectPositionY(t.tgunobj);
							t.brayz1_f=ObjectPositionZ(t.tgunobj);
							t.tthavegunobject=1;
						}
					}
					if ( t.tthavegunobject == 0 ) 
					{
						t.brayx1_f=ObjectPositionX(t.charanimstate.obj);
						t.brayy1_f=ObjectPositionY(t.charanimstate.obj)+20; // 070918 - raised a bit closer to character eyes 
						t.brayz1_f=ObjectPositionZ(t.charanimstate.obj);
						t.tsrcobj=g.entitybankoffset+t.entityelement[t.charanimstate.e].bankindex;
						if ( ObjectExist(t.tsrcobj) == 1 ) t.brayy1_f = t.brayy1_f + (ObjectSizeY(t.tsrcobj,1)*0.5f);
					}

					// 090417 - improve accuracy of enemy plr detection (was putting player at waste level)
					entity_gettruecamera ( );
					t.brayx2_f = t.tcamerapositionx_f;
					t.brayy2_f = t.tcamerapositiony_f;
					t.brayz2_f = t.tcamerapositionz_f;

					// first ensure not going through physics terrain
					if ( ODERayTerrain(t.brayx1_f,t.brayy1_f,t.brayz1_f,t.brayx2_f,t.brayy2_f,t.brayz2_f) == 1 ) 
					{
						t.ttokay=0;
					}
					else
					{
						//  140514 - actually move ray BACK a little in case enemy right up against something!
						t.ttdx_f=t.brayx2_f-t.brayx1_f;
						t.ttdy_f=t.brayy2_f-t.brayy1_f;
						t.ttdz_f=t.brayz2_f-t.brayz1_f;
						t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
						t.ttdx_f=t.ttdx_f/t.ttdd_f;
						t.ttdy_f=t.ttdy_f/t.ttdd_f;
						t.ttdz_f=t.ttdz_f/t.ttdd_f;
						t.brayx1_f=t.brayx1_f-(t.ttdx_f*10.0);
						t.brayy1_f=t.brayy1_f-(t.ttdy_f*10.0);
						t.brayz1_f=t.brayz1_f-(t.ttdz_f*10.0);
						//  if third person, target is an OBJ (so need to cut back dest coordinate so as not to intersect it)
						if (  t.playercontrol.thirdperson.enabled == 1 ) 
						{
							t.brayx2_f=t.brayx2_f-(t.ttdx_f*30.0);
							t.brayy2_f=t.brayy2_f-(t.ttdy_f*30.0);
							t.brayz2_f=t.brayz2_f-(t.ttdz_f*30.0);
						}
						//  (first intersectall command simply fills a secondary range of objects)
						if (  g.gnumberofraycastsallowedincycle>0 ) 
						{
							t.tttokay = 0 ; if (  t.ttdd_f<300  )  t.tttokay = 1;
							if (  g.gnumberofraycastslastoneused != t.tcharanimindex || g.gnumberofraycastsallowedincycle <= 2 || t.tttokay == 1 ) 
							{
								if (  t.tttokay == 0 && g.gnumberofraycastsallowedincycle>0  )  --g.gnumberofraycastsallowedincycle;
								g.gnumberofraycastslastoneused=t.tcharanimindex;
							}
						}
						if (  g.gnumberofraycastslastoneused == t.tcharanimindex ) 
						{
							t.ttt=IntersectAll(g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,t.brayx1_f,t.brayy1_f,t.brayz1_f,0,0,0,-123);
							t.tintersectvalue=IntersectAll(g.entityviewstartobj,g.entityviewendobj,t.brayx1_f,t.brayy1_f,t.brayz1_f,t.brayx2_f,t.brayy2_f,t.brayz2_f,t.charanimstate.obj);//220618 yuk >0;
							if (  t.tintersectvalue>0 ) 
							{
								t.ttokay=0;
							}
						}
						else
						{
							//  allows engine to limit expensive ray casts to a few per cycle
							t.ttokay=0;
						}
					}
					if (  t.ttokay == 1 ) 
					{
						t.entityelement[t.charanimstate.e].plrvisible=1;
						t.entityelement[t.charanimstate.e].lua.flagschanged=1;
					}
				}
			}
		}
	}	
}

void darkai_loop ( void )
{
	//  Handle Player AIStates
	AISetPlayerDucking (  t.aisystem.playerducking );
	AISetPlayerContainer (  t.aisystem.playercontainerid );

	//  allow two raycasts per cycle (and one of them needs to be unique)
	g.gnumberofraycastsallowedincycle=5;

	//  tic down nearby count
	if (  g.aidetectnearbycount > 0  )  --g.aidetectnearbycount;

	//  all characters in game
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		//  This character
		t.charanimstate = t.charanimstates[g.charanimindex];

		//  Entity Element Index for this A.I character
		t.i=t.charanimstate.obj ; t.te=t.charanimstate.e;

		//  ensure can stop looping sound ANY time
		t.ttsnd=t.charanimstate.firesoundindex;
		if (  t.ttsnd>0 ) 
		{
			if (  SoundExist(t.ttsnd) == 1 ) 
			{
				if (  Timer()>(int)t.charanimstate.firesoundexpiry ) 
				{
					StopSound (  t.ttsnd );
				}
				if (  SoundPlaying(t.ttsnd) == 0 ) 
				{
					t.charanimstate.firesoundindex=0;
				}
			}
		}

		//  is entiy active here?
		if (  t.entityelement[t.te].active == 1 ) 
		{

		//  Controls distance at which all characters freeze back to instances
		entity_getmaxfreezedistance ( );

		//  Allows character that would normally be out of range to come alive for a while
		//  Is used when someone is shot by a sniper and those nearby react
		if (  g.aidetectnearbymode == 1 ) 
		{
			if (  g.aidetectnearbycount>0 ) 
			{
				t.dx_f=g.aidetectnearbymodeX_f-t.entityelement[t.charanimstate.e].x;
				t.dz_f=g.aidetectnearbymodeZ_f-t.entityelement[t.charanimstate.e].z;
				t.tdist_f=Sqrt(abs(t.dx_f*t.dx_f)+abs(t.dz_f*t.dz_f));
				if (  t.tdist_f < 300.0 ) 
				{
					t.entityelement[t.charanimstate.e].plrdist = t.maximumnonefreezedistance / 2.0;
				}
			}
			else
			{
				g.aidetectnearbymode = 0;
			}
		}

		//  Ensure characters can be placed back in range immediately if needed
		if (  t.charanimstate.outofrange == 1 && (t.entityelement[t.charanimstate.e].plrdist<t.maximumnonefreezedistance || t.entityelement[t.charanimstate.e].health <= 0) ) 
		{
			//  Back in range
			t.charanimstate.outofrange=0;

			//  Activate object
			if (  AIEntityExist(t.i) == 1  )  AISetEntityActive (  t.i,1 );

			//  Delete old instance obj to use animatable CloneObject (  )
			t.obj=t.charanimstate.obj;
			t.tentid=t.entityelement[t.charanimstate.e].bankindex;
			//Dave Performance - convert to clone is good, so left that
			t.tte=t.charanimstate.e ; entity_converttoclone ( );
			entity_setupcharobjsettings ( );

		}

		//  For valid A.I entities
		if (  AIEntityExist(t.i) == 1 && t.entityelement[t.charanimstate.e].health>0 ) 
		{
			//  If in range for activity
			if (  t.entityelement[t.charanimstate.e].plrdist<t.maximumnonefreezedistance ) 
			{
				//  Only check for ray visibility when triggered (expensive task)
				if (  Timer()>(int)t.charanimstate.visiondelaylastime+t.charanimstate.visiondelay ) 
				{
					//  switches to next char index in sycn with visiondelay
					t.charanimstate.visiondelaylastime=Timer();
					t.tcharanimindex=g.charanimindex;
					darkai_calcplrvisible ( );
				}
			}
			else
			{
				//  Freeze All A.I for character out of range
				if (  t.charanimstate.outofrange == 0 ) 
				{

					//  Character is out of range
					t.charanimstate.outofrange=1;

					//  reset for when resume
					t.charanimstate.playcsi=g.csi_unarmed;
					t.charanimstate.alerted=0;

					//  capture latest position for later resume
					t.obj=t.charanimstate.obj;
					t.entityelement[t.charanimstate.e].x=ObjectPositionX(t.obj);
					t.entityelement[t.charanimstate.e].y=ObjectPositionY(t.obj);
					t.entityelement[t.charanimstate.e].z=ObjectPositionZ(t.obj);
					t.entityelement[t.charanimstate.e].ry=ObjectAngleY(t.obj);

					// Delete old obj and use cheaper InstanceObject (  )
					t.tentid=t.entityelement[t.charanimstate.e].bankindex;
					// converting to instance makes them animate again!
					t.tte=t.charanimstate.e ; entity_converttoinstance ( );
					entity_setupcharobjsettings ( );

					//  animate character object parent in standard idle post
					t.tttsourceobj=g.entitybankoffset+t.entityelement[t.charanimstate.e].bankindex;
					if (  ObjectExist(t.tttsourceobj) == 1 ) 
					{
						if (  t.entityprofile[t.tentid].animmax>0 && t.entityprofile[t.tentid].playanimineditor>0 ) 
						{
							t.q=t.entityprofile[t.tentid].playanimineditor-1;
							LoopObject (  t.tttsourceobj,t.entityanim[t.tentid][t.q].start,t.entityanim[t.tentid][t.q].finish );
							t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.charanimstate.animationspeed_f*2.5*g.timeelapsed_f;
							SetObjectSpeed (  t.tttsourceobj,t.tfinalspeed_f );
						}
						//  for intense CPU animators, stop anim when in distance
						if (  t.entityprofile[t.tentid].cpuanims != 0  )  StopObject (  t.tttsourceobj );
					}

					//  Deactivate out of range character
					if (  AIEntityExist(t.i) == 1  )  AISetEntityActive (  t.i,0 );

				}

			//  end of 'if in range for activity'
			}

		}
		else
		{

			//  A.I not part of simulation

		}

		//  Active else branch
		}
		else
		{

		//  stop LoopSound (  if underlying AI deactives )
		t.ttsnd=t.charanimstate.firesoundindex;
		t.charanimstate.firesoundindex=0;
		if (  t.ttsnd>0 ) 
		{
			if (  SoundExist(t.ttsnd) == 1 ) 
			{
				StopSound (  t.ttsnd );
			}
		}

		//  Active branch
		}

		//  Handle character removal
		if (  t.entityelement[t.charanimstate.e].health <= 0 && t.charanimstate.timetofadeout>0 ) 
		{
			if (  Timer()>t.charanimstate.timetofadeout ) 
			{
				t.txDist_f = ObjectPositionX(t.charanimstate.obj) - CameraPositionX(0);
				t.tzDist_f = ObjectPositionZ(t.charanimstate.obj) - CameraPositionZ(0);
				if (  t.txDist_f * t.txDist_f + t.tzDist_f * t.tzDist_f > 500000 ) 
				{
					if (  GetInScreen(t.charanimstate.obj)  ==  0 ) 
					{
						// disable ability to remove character from system if ALWAYS ACTIVE has been set
						// (allows characters to respawn)
						if ( t.entityelement[t.charanimstate.e].eleprof.phyalways == 0 )
						{
							darkai_character_remove ( );
						}
					}
				}
			}
		}

		//  Store any changes
		t.charanimstates[g.charanimindex] = t.charanimstate;
	}
}

void darkai_update ( void )
{
	//  Update A.I system
	if (  t.visuals.debugvisualsmode<99 ) 
	{
		AIUpdate (  );
	}
}

void darkai_character_remove_charpart ( void )
{
	//  if ragdoll, remove this now
	t.tte=t.charanimstate.originale;
	entity_freeragdoll ( );

	//  switch back to instance
	t.charanimstate.fadeoutvalue_f=0.0;
	t.charanimstate.timetofadeout=0;
}

void darkai_character_remove ( void )
{
	darkai_character_remove_charpart ( );
	entity_converttoinstance ( );
	//  and COMPLETELY remove it from map
	t.entityelement[t.tte].x=-100000;
	t.entityelement[t.tte].y=-100000;
	t.entityelement[t.tte].z=-100000;
	if (  ObjectExist(t.charanimstate.obj) == 1 ) 
	{
		//  HideObject (  and move away )
		HideObject (  t.charanimstate.obj );
		PositionObject (  t.charanimstate.obj,t.entityelement[t.tte].x,t.entityelement[t.tte].y,t.entityelement[t.tte].z );
	}
	// 120416 - hide any attachment to entity
	if ( t.entityelement[t.tte].eleprof.cantakeweapon == 0 )
	{
		// if not meant to be collected after char disappears
		t.tattobj=t.entityelement[t.tte].attachmentobj;
		if ( t.tattobj>0  )  
		{
			if ( ObjectExist( t.tattobj ) == 1 )
			{
				ODEDestroyObject ( t.tattobj );
				HideObject ( t.tattobj );
			}
		}
	}
}

void darkai_character_loop ( void )
{
	// Multiplayer game seems to want to modify original freeze distance (added by dave)
	if (  t.game.runasmultiplayer != 0 ) 
	{
		t.toriginalmaximumnonefreezedistance = t.maximumnonefreezedistance;
	}

	//  Handles inerconnection between DarkA.I and Visual Object
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		//  This character
		t.charanimstate = t.charanimstates[g.charanimindex];

		//  Entity Element Index for this A.I character
		t.te=t.charanimstate.e;
		if (  t.entityelement[t.te].active == 1 && t.charanimstate.obj>0 ) 
		{
		if (  ObjectExist(t.charanimstate.obj) == 1 ) 
		{
		//  if the char is controlled by someone else, we increase the range to max
		if (  t.game.runasmultiplayer  ==  1 && g.steamworks.coop  ==  1 ) 
		{
			if (  t.entityelement[t.te].mp_updateOn  ==  1 && t.entityelement[t.te].mp_coopControlledByPlayer  !=  g.steamworks.me && t.entityelement[t.te].mp_coopControlledByPlayer  !=  -1 ) 
			{
				t.maximumnonefreezedistance = 999990;
			}
			else
			{
				t.maximumnonefreezedistance = t.toriginalmaximumnonefreezedistance;
			}
		}

		//  The 'Alive' behavior of the character
		if (  t.entityelement[t.charanimstate.e].health>0 && t.entityelement[t.charanimstate.e].plrdist<t.maximumnonefreezedistance ) 
		{
			// if in LIMBO MANUAL MODE, skip all AI interference from legacy system
			if ( t.charanimstate.playcsi == g.csi_limbo && t.charanimstate.limbomanualmode == 1 )
			{
				// AI decoupled allowing AI MANUAL MODE to work unmolested (and then data read directly by LUA)
				// but still feed machine indie speed into AI system
				//t.taispeed_f = 220.0 * t.entityelement[t.charanimstate.e].speedmodulator_f;
				//t.taispeed_f = 220.0 * 2.0f * t.entityelement[t.charanimstate.e].speedmodulator_f; // 270217 - all chars seemed slow compared to old so doubled it for NEW AI MANUAL MODE
				t.taispeed_f = 220.0 * t.entityelement[t.charanimstate.e].speedmodulator_f; // 070317 - now zombies sliding, restore this and correct anything else from FPEs
				
				t.taispeedtimecap_f = g.timeelapsed_f;
				if ( t.taispeedtimecap_f<0.23f ) t.taispeedtimecap_f = 0.23f;
				t.taispeed_f = t.taispeed_f * (t.entityelement[t.charanimstate.e].eleprof.speed/100.0);
				AISetEntitySpeed ( t.charanimstate.obj, t.taispeed_f*t.taispeedtimecap_f );
			}
			else
			{
				//  get raw A.I entity position and angle
				t.tairealposx_f=AIGetEntityX(t.charanimstate.obj);
				t.tairealposz_f=AIGetEntityZ(t.charanimstate.obj);
				t.tangley_f=AIGetEntityAngleY(t.charanimstate.obj);

				// proximity check to ensure they do not enter other character entities
				entity_lua_moveforward_core_nooverlap ( t.charanimstate.e, &t.tairealposx_f, &t.tairealposz_f );
				AISetEntityPosition ( t.charanimstate.obj, t.tairealposx_f, AIGetEntityY(t.charanimstate.obj), t.tairealposz_f );

				//  get terrain ground height for A.I entity
				if (  t.terrain.TerrainID>0 ) 
				{
					t.tusecurrentgroundheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tairealposx_f,t.tairealposz_f);
				}
				else
				{
					t.tusecurrentgroundheight_f=1000.0;
				}

				//  work out distance and angle between A.I entity and visible character object
				t.dx_f=t.tairealposx_f-ObjectPositionX(t.charanimstate.obj);
				t.dz_f=t.tairealposz_f-ObjectPositionZ(t.charanimstate.obj);
				t.dd_f=Sqrt(abs(t.dx_f*t.dx_f)+abs(t.dz_f*t.dz_f));
				t.da_f=atan2deg(t.dx_f,t.dz_f);

				//  expert system to prevent small movements when in run-mode
				if (  t.charanimstate.runmode == 1 && t.dd_f<30.0  )  t.dd_f = 0;

				//  All warm-up code removed by Rick (too over-complicated?)
				t.charanimstate.warmupwalk_f=1.0;

				//  move character using charanimcontrols flags if A.I entity somewhere else
				t.charanimstate.distancetotarget_f=t.dd_f;
				if (  t.charanimstate.playcsi != g.csi_limbo ) 
				{
					//  if (  you are less than XX units from destination AND running,  ) 
					t.tdx_f=AIGetEntityDestinationX(t.charanimstate.obj)-ObjectPositionX(t.charanimstate.obj);
					t.tdz_f=AIGetEntityDestinationZ(t.charanimstate.obj)-ObjectPositionZ(t.charanimstate.obj);
					t.tdistancebetweenentityandfinaldestination_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
					t.tdothestop=0;
					if (  t.tdistancebetweenentityandfinaldestination_f <= 25.0 && t.charanimstate.runmode == 0  )  t.tdothestop = 1;
					if (  t.tdistancebetweenentityandfinaldestination_f <= 75.0 && t.charanimstate.runmode == 1  )  t.tdothestop = 1;

					//  don't stop them in coop mode, as the destinations are smaller increments (stopping is handled in the steam module)
					if (  t.game.runasmultiplayer  ==  1 && g.steamworks.coop  ==  1 ) 
					{
						if (  t.entityelement[t.te].mp_updateOn  ==  1 && t.entityelement[t.te].mp_coopControlledByPlayer  !=  g.steamworks.me ) 
						{
							t.tdothestop = 0;
							t.tminrundestsize_f = 0.0;
						}
						else
						{
							t.tminrundestsize_f = 75.0;
						}
					}
					else
					{
						t.tminrundestsize_f = 75.0;
					}

					if (  t.tdothestop == 1 ) 
					{
						//  change destination to current position (so subsystem will do the stopping)
						//  and also means the waypoint system can work again!
						AISetEntityPosition (  t.charanimstate.obj,ObjectPositionX(t.charanimstate.obj),ObjectPositionY(t.charanimstate.obj),ObjectPositionZ(t.charanimstate.obj) );
						AIEntityStop (  t.charanimstate.obj );

						// 050116 - ensure we stop any subsequent movement actions (stops char moving off when active from unfreeze event)
						t.dd_f = 0.0f;
					}

					//  can only move when not in limbo
					if (  t.dd_f >= 10.0 ) 
					{
						//  moving (1-walk,2-back,3-left,4-right,5-run,11-left,12-right)
						t.tneedtostrafe=t.charanimstate.strafemode;
						if (  t.tneedtostrafe>0 ) 
						{
							if (  t.tneedtostrafe == 1  )  t.charanimcontrols[g.charanimindex].moving = 11;
							if (  t.tneedtostrafe == 2  )  t.charanimcontrols[g.charanimindex].moving = 12;
						}
						else
						{
							//  if frozen, set moving to 0
							if (  t.charanimstate.freezeallmovement  ==  0 ) 
							{
								t.tneedtorun=t.charanimstate.runmode;
								if (  t.tneedtorun == 1 && t.tdistancebetweenentityandfinaldestination_f>t.tminrundestsize_f ) 
								{
									t.charanimcontrols[g.charanimindex].moving=5;
								}
								else
								{
									t.charanimcontrols[g.charanimindex].moving=1;
								}
								//  while moving, this is the object Y angle we use
								t.charanimstate.currentangle_f=t.da_f;
							}
							else
							{
								t.charanimcontrols[g.charanimindex].moving=0;
							}
						}
					}
					else
					{
						//  not moving
						t.charanimcontrols[g.charanimindex].moving=0;
					}

					// 060116 - refresh entity angle Y from current character Y angle (for if enter limbo)
					t.entityelement[t.charanimstate.e].ry = t.charanimstate.currentangle_f;
				}
				else
				{
					//  when in limbo state, angle controlled my entityelement RY
					if ( t.charanimstate.limbomanualmode == 1 )
					{
						// should never get here, see condition further up
					}
					else
					{
						// default limbo
						t.charanimstate.currentangle_f=t.entityelement[t.charanimstate.e].ry;
						AISetEntityAngleY (  t.charanimstate.obj,t.charanimstate.currentangle_f );
					}
				}

				//  work out direct Angle between OBJECT character and player
				if ( t.game.runasmultiplayer == 0 || g.steamworks.coop == 0 ) 
				{
					t.tdx_f=CameraPositionX(0)-ObjectPositionX(t.charanimstate.obj);
					t.tdz_f=CameraPositionZ(0)-ObjectPositionZ(t.charanimstate.obj);
				}
				else
				{
					if (  t.entityelement[t.charanimstate.e].mp_coopControlledByPlayer  ==  g.steamworks.me || t.entityelement[t.te].mp_coopControlledByPlayer  ==  -1 ) 
					{
						t.tdx_f=CameraPositionX(0)-ObjectPositionX(t.charanimstate.obj);
						t.tdz_f=CameraPositionZ(0)-ObjectPositionZ(t.charanimstate.obj);
					}
					else
					{
						t.tsteamotherplayer = t.entityelement[t.steamworks_playerEntityID[t.entityelement[t.te].mp_coopControlledByPlayer]].obj;
						t.tdx_f=ObjectPositionX(t.tsteamotherplayer)-ObjectPositionX(t.charanimstate.obj);
						t.tdz_f=ObjectPositionZ(t.tsteamotherplayer)-ObjectPositionZ(t.charanimstate.obj);
					}
				}
				t.tdirectangley_f=atan2deg(t.tdx_f,t.tdz_f);

				//  All characters aim off to side, so correct now
				t.tdirectangley_f=t.tdirectangley_f-10.0;

				//  work out movement angle, and if stood still, only rotate if significant turn
				if (  t.charanimcontrols[g.charanimindex].moving == 0 ) 
				{
					// if stood, and no waste twist data, override rotation to use head direction (gun facing)
					if (  t.charanimstate.usingcharacterposedat == 0 ) 
					{
						// 050116 - this would force stood character to rotate to face player (irrespective of engagement)
						//t.charanimstate.moveangle_f = t.tdirectangley_f;
						// 110416 - completely broke RotateToPlayer functionality (forced rotation only happens because script commanded it!)
						t.charanimstate.moveangle_f = t.charanimstate.currentangle_f;
					}

					//  work out if 'turn on spot' should animate
					t.tdiff_f=abs(WrapValue(t.charanimstate.moveangle_f)-WrapValue(t.charanimstate.currentangle_f));
					if (  t.tdiff_f>180  )  t.tdiff_f = 360-t.tdiff_f;
					if (  t.tdiff_f>20 || (t.tdiff_f>1.0 && t.charanimstate.updatemoveangle == 1) ) 
					{
						if (  t.tdiff_f>20 ) 
						{
							t.charanimcontrols[g.charanimindex].moving=13;
							if (  t.charanimstate.usingcharacterposedat == 1 ) 
							{
								t.charanimstate.moveangle_f=t.charanimstate.currentangle_f;
							}
							t.charanimstate.updatemoveangle=0;
						}
					}
				}
				else
				{
					t.charanimstate.moveangle_f=t.charanimstate.currentangle_f;
				}

				//  ensure curve does not exceed intertia max
				t.tsmoothspeed_f=2.0/g.timeelapsed_f;
				t.tnowaty_f=WrapValue(ObjectAngleY(t.charanimstate.obj));
				t.twanttobey_f=CurveAngle(t.charanimstate.moveangle_f,ObjectAngleY(t.charanimstate.obj),t.tsmoothspeed_f);
				t.tdiffy_f=t.twanttobey_f-t.tnowaty_f;
				if (  t.tdiffy_f<-180  )  t.tdiffy_f = t.tdiffy_f+360.0;
				if (  t.tdiffy_f>180  )  t.tdiffy_f = t.tdiffy_f-360.0;
				if (  abs(t.tdiffy_f)>11.0 ) 
				{
					if (  t.tdiffy_f>0 ) 
					{
						t.tdiffy_f=11.0;
					}
					else
					{
						t.tdiffy_f=-11.0;
					}
					t.twanttobey_f=t.tnowaty_f+t.tdiffy_f;
				}

				//  curve angle of visible character object to move angle
				YRotateObject (  t.charanimstate.obj,t.twanttobey_f );
				//  calculate movement speed of A.I entity
				if (  t.dd_f>40.0 ) 
				{
					t.taispeed_f=0.0;
				}
				else
				{
					if (  t.charanimstate.runmode == 1 ) 
					{
						t.taispeed_f=t.charseq[t.charanimstates[g.charanimindex].playcsi].speed_f*300.0;
						if (  t.taispeed_f<300.0  )  t.taispeed_f = 300.0;
					}
					else
					{
						t.taispeed_f=t.charseq[t.charanimstates[g.charanimindex].playcsi].speed_f*220.0;
						if (  t.taispeed_f<220.0  )  t.taispeed_f = 220.0;
					}
				}
				t.taispeed_f=t.taispeed_f*t.entityelement[t.charanimstate.e].speedmodulator_f;

				//  speed up the character a bit if its not controlled by us
				t.taispeedtimecap_f=g.timeelapsed_f;
				if (  t.taispeedtimecap_f<0.23f  )  t.taispeedtimecap_f = 0.23f;
				t.taispeed_f=t.taispeed_f*(t.entityelement[t.charanimstate.e].eleprof.speed/100.0);
				AISetEntitySpeed (  t.charanimstate.obj,t.taispeed_f*t.taispeedtimecap_f );

				//  control directly of via physics (capsule)
				t.te=t.charanimstate.e ; t.tv_f=1.0f ; entity_updatepos ( );

				//  Angle between AI BOT character and player
				t.taiangley_f=AIGetEntityAngleY(t.charanimstate.obj);
				t.taiangley_f=t.taiangley_f-ObjectAngleY(t.charanimstate.obj);
				if (  t.taiangley_f<-180  )  t.taiangley_f = t.taiangley_f+360;
				if (  t.taiangley_f>180  )  t.taiangley_f = t.taiangley_f-360;

				//  work out relative head angle between angle we want and current model angle
				t.headangley_f=t.tdirectangley_f-ObjectAngleY(t.charanimstate.obj);
				if (  t.headangley_f<-180  )  t.headangley_f = t.headangley_f+360;
				if (  t.headangley_f>180  )  t.headangley_f = t.headangley_f-360;

				//  Choose more accurate object character to player angle if close enough
				if (  abs(t.taiangley_f-t.headangley_f)>45  )  t.headangley_f = t.taiangley_f;
				if (  t.headangley_f<-90 || t.headangley_f>90  )  t.tfacingawayfromplr = 1; else t.tfacingawayfromplr = 0;
				if (  t.headangley_f<-75  )  t.headangley_f = -75;
				if (  t.headangley_f>75  )  t.headangley_f = 75;

				//  Work out if we can use obj-frame override to twist wastes, 'animation frame based' or 'limb rotate based'
				t.ttentid=t.entityelement[t.charanimstate.e].bankindex;
				t.tcsiindex=t.csi_stoodandturnANIM[t.charanimstate.weapstyle];
				t.q=t.entityprofile[t.ttentid].startofaianim+t.tcsiindex;
				if (  t.q >= 0 && t.q<g.animmax ) 
				{
					t.qstart_f=t.entityanim[t.ttentid][t.q].start ; t.qfinish_f=t.entityanim[t.ttentid][t.q].finish ; t.qmiddle_f=t.qstart_f+10.0;
				}
				else
				{
					t.qstart_f=0.0;
				}

				//  Apply twist by obj frame or limb rotation
				t.headlimbofcharacter=t.entityprofile[t.entityelement[t.charanimstate.e].bankindex].headlimb;
				t.spinelimbofcharacter=t.entityprofile[t.entityelement[t.charanimstate.e].bankindex].spine;
				t.tsmoothspeed_f=2.0/g.timeelapsed_f;
				if (  t.charanimcontrols[g.charanimindex].alerted == 0 || t.tfacingawayfromplr == 1 ) 
				{
					//  Adjustment to ensure character looks 'directly' at player (no waste twist at all)
					t.ttwistheadtofacedirectly_f=-11;
					t.charanimstate.realheadangley_f=CurveAngle(t.headangley_f+t.ttwistheadtofacedirectly_f,t.charanimstate.realheadangley_f,t.tsmoothspeed_f);
					if (  t.entityelement[t.charanimstate.e].health>0 ) 
					{
						if (  t.headlimbofcharacter>0 ) 
						{
							if (  LimbExist(t.charanimstate.obj,t.headlimbofcharacter) == 1 ) 
							{
								RotateLimb (  t.charanimstate.obj,t.headlimbofcharacter,0,0,0 );
							}
						}
						if (  t.qstart_f == 0 ) 
						{
							//  Ensure spine is reset when not in alert mode
							if (  t.spinelimbofcharacter>0 ) 
							{
								if (  LimbExist(t.charanimstate.obj,t.spinelimbofcharacter) == 1 ) 
								{
									RotateLimb (  t.charanimstate.obj,t.spinelimbofcharacter,0,0,0 );
								}
							}
						}
					}
				}
				else
				{
					//  Character Spine Angle
					t.charanimstate.realheadangley_f=t.headangley_f;
					t.tdiff_f=WrapValue(t.charanimstate.realheadangley_f)-WrapValue(t.charanimstate.lastwasteangley_f);
					if (  t.tdiff_f<-180  )  t.tdiff_f = t.tdiff_f+360.0;
					if (  t.tdiff_f>180  )  t.tdiff_f = t.tdiff_f-360.0;
					if (  abs(t.tdiff_f)>5 ) 
					{
						if (  t.tdiff_f<0  )  t.tdiff_f = -5; else t.tdiff_f = 5;
					}
					t.tcharanimstaterealheadangley_f=t.charanimstate.lastwasteangley_f+t.tdiff_f;
					t.charanimstate.lastwasteangley_f=t.tcharanimstaterealheadangley_f;
					if (  t.entityelement[t.charanimstate.e].eleprof.hasweapon == 0 ) 
					{
						//  to GetPoint (  head only at player )
						if (  t.entityelement[t.charanimstate.e].health>0 ) 
						{
							if (  t.headlimbofcharacter>0 ) 
							{
								if (  LimbExist(t.charanimstate.obj,t.headlimbofcharacter) == 1 ) 
								{
									//  as idle head on model is adjusted off center, have to compensate until new models arrive..
									RotateLimb (  t.charanimstate.obj,t.headlimbofcharacter,0,t.tcharanimstaterealheadangley_f-15.0,0 );
								}
							}
						}
					}
					else
					{
						//  to GetPoint (  gun at player only if have weapon )
						//  Dave added fix, testing for character creator, otherwise the enemy goes into a weird tpose
						//  Lee added fix, and ONLY if using UBER style DAT file for rotation post data
						if (  t.entityelement[t.charanimstate.e].health>0 && t.entityprofile[t.ttentid].ischaractercreator == 0 && t.charanimstate.usingcharacterposedat == 1 ) 
						{
							if (  t.qstart_f>0 ) 
							{
								//  USE ANIMATION TO ROTATE WASTE
								//  and only if enemy can see player and using stood pose
								t.q1=t.entityprofile[t.ttentid].startofaianim+t.csi_stoodnormalANIM[t.charanimstate.weapstyle];
								t.q2=t.entityprofile[t.ttentid].startofaianim+t.csi_stoodnormalANIM[t.charanimstate.weapstyle];
								if (  t.entityelement[t.charanimstate.e].plrvisible == 1 && GetFrame(t.charanimstate.obj) >= t.entityanim[t.ttentid][t.q1].start && GetFrame(t.charanimstate.obj) <= t.entityanim[t.ttentid][t.q2].finish ) 
								{
									//  animation frame based
									if (  t.tcharanimstaterealheadangley_f<0 ) 
									{
										//  turning to the left
										t.charactergunposeindex_f=t.qstart_f+(abs(t.tcharanimstaterealheadangley_f/12.0));
										if (  t.charactergunposeindex_f>t.qmiddle_f-0.1  )  t.charactergunposeindex_f = t.qmiddle_f-0.1;
									}
									else
									{
										//  turning to the right
										t.charactergunposeindex_f=t.qmiddle_f+((t.tcharanimstaterealheadangley_f/12.0));
										if (  t.charactergunposeindex_f>t.qfinish_f  )  t.charactergunposeindex_f = t.qfinish_f;
									}
									//  set frame override
									t.charanimstate.smoothoverridedest_f=1.0;
									t.charanimstate.smoothoverridedestframe_f=t.charactergunposeindex_f;
									t.charanimstate.smoothoverrideqstart_f=t.qstart_f;
									t.charanimstate.smoothoverrideqmiddle_f=t.qmiddle_f;
									t.charanimstate.smoothoverrideqfinish_f=t.qfinish_f;
								}
							}
							else
							{
								//  USE LIMB TO ROTATE WASTE
								if (  t.spinelimbofcharacter>0 ) 
								{
									if (  LimbExist(t.charanimstate.obj,t.spinelimbofcharacter) == 1 ) 
									{
										//  new waste rotator
										t.charactergunposeindex_f=18.0-(t.tcharanimstaterealheadangley_f/5.0);
										t.charactergunposeindex=int(t.charactergunposeindex_f);
										t.charactergunposefraction_f=t.charactergunposeindex_f-t.charactergunposeindex;
										if (  t.charactergunposeindex<0  )  t.charactergunposeindex = 0;
										if (  t.charactergunposeindex>35  )  t.charactergunposeindex = 35;
										t.lax_f=t.charactergunpose[g.charanimindex][t.charactergunposeindex].x;
										t.lay_f=t.charactergunpose[g.charanimindex][t.charactergunposeindex].y;
										t.laz_f=t.charactergunpose[g.charanimindex][t.charactergunposeindex].z;
										t.lax2_f=(t.charactergunpose[g.charanimindex][t.charactergunposeindex+1].x-t.lax_f)*t.charactergunposefraction_f;
										t.lay2_f=(t.charactergunpose[g.charanimindex][t.charactergunposeindex+1].y-t.lay_f)*t.charactergunposefraction_f;
										t.laz2_f=(t.charactergunpose[g.charanimindex][t.charactergunposeindex+1].z-t.laz_f)*t.charactergunposefraction_f;
										RotateLimb (  t.charanimstate.obj,t.spinelimbofcharacter,t.lax_f+t.lax2_f,t.lay_f+t.lay2_f,t.laz_f+t.laz2_f );
									}
								}
							}
							//  Ensure head lines with gun when shooting!
							if (  t.headlimbofcharacter>0 ) 
							{
								if (  LimbExist(t.charanimstate.obj,t.headlimbofcharacter) == 1 ) 
								{
									RotateLimb (  t.charanimstate.obj,t.headlimbofcharacter,0,0,0 );
								}
							}
						}
					}
				}
			}
		}

		// character Animation Speed
		if ( t.charanimstate.animationspeed_f >=0.0f )
		{
			// only use reverse polarity if animationspeed is NOT negative
			if (  GetSpeed(t.charanimstate.obj)<0  )  t.polarity = -1; else t.polarity = 1;
		}
		else
		{
			// 271115 - can set animatiospeed to negative, no need for polarity
			t.polarity = 1; 
		}
		t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.charanimstate.animationspeed_f*t.polarity*2.5*g.timeelapsed_f;
		SetObjectSpeed (  t.charanimstate.obj,t.tfinalspeed_f );

		//  call character animation system
		if (  t.playercontrol.thirdperson.enabled == 0 ) 
		{
			if (  t.entityelement[t.charanimstate.e].plrdist<t.maximumnonefreezedistance || t.entityelement[t.charanimstate.e].health <= 0 ) 
			{
				char_loop ( );
			}
		}
		else
		{
			if (  g.charanimindex != t.playercontrol.thirdperson.characterindex ) 
			{
				if (  t.entityelement[t.charanimstate.e].plrdist<t.maximumnonefreezedistance || t.entityelement[t.charanimstate.e].health <= 0 ) 
				{
					char_loop ( );
				}
			}
		}

		//  Active branch
		}
		}

		//  Store any changes
		t.charanimstates[g.charanimindex] = t.charanimstate;

	}

return;

}

void darkai_finalsettingofcharacterobjects ( void )
{
	//  Ensure the LAST thing is to override frames to create smooth remder
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		t.charanimstate = t.charanimstates[g.charanimindex];
		t.te=t.charanimstate.e;
		//  Handle any override smoothing (waste twist over existing animation)
		if (  t.entityelement[t.te].active == 1 && t.charanimstate.obj>0 ) 
		{
			if (  ObjectExist(t.charanimstate.obj) == 1 ) 
			{
				if (  t.charanimstate.smoothoverridedest_f>0.0 ) 
				{
					//  if not being refreshed, move frame back to center
					t.qstart_f=t.charanimstate.smoothoverrideqstart_f;
					t.qmiddle_f=t.charanimstate.smoothoverrideqmiddle_f;
					t.qfinish_f=t.charanimstate.smoothoverrideqfinish_f;
					if (  t.charanimstate.smoothoverridedest_f<0.5 ) 
					{
						if (  t.charanimstate.smoothoverridedestframe_f>t.qstart_f && t.charanimstate.smoothoverridedestframe_f <= t.qmiddle_f-0.1 ) 
						{
							t.charanimstate.smoothoverridedestframe_f=t.charanimstate.smoothoverridedestframe_f-0.1;
							if (  t.charanimstate.smoothoverridedestframe_f<t.qstart_f ) 
							{
								t.charanimstate.smoothoverridedestframe_f=t.qstart_f;
							}
						}
						if (  t.charanimstate.smoothoverridedestframe_f>t.qmiddle_f && t.charanimstate.smoothoverridedestframe_f <= t.qfinish_f ) 
						{
							t.charanimstate.smoothoverridedestframe_f=t.charanimstate.smoothoverridedestframe_f-0.1;
							if (  t.charanimstate.smoothoverridedestframe_f<t.qmiddle_f ) 
							{
								t.charanimstate.smoothoverridedestframe_f=t.qmiddle_f;
							}
						}
					}
					//  reduce persistence of this override over time
					t.charanimstates[g.charanimindex].smoothoverridedest_f=t.charanimstates[g.charanimindex].smoothoverridedest_f-g.timeelapsed_f;
					if (  t.charanimstates[g.charanimindex].smoothoverridedest_f<0.001 ) 
					{
						//  switch off override frames
						for ( t.f = 10 ; t.f<=  26; t.f++ )
						{
							SetObjectFrameEx (  t.charanimstate.obj,t.f,-1.0,1 );
						}
						t.charanimstates[g.charanimindex].smoothoverridedest_f=0;
					}
					else
					{
						//  use override frames
						for ( t.f = 10 ; t.f<=  26; t.f++ )
						{
							SetObjectFrameEx (  t.charanimstate.obj,t.f,t.charanimstate.smoothoverridedestframe_f,1 );
						}
					}
				}
			}
		}
	}

return;

}

void darkai_character_freezeall ( void )
{

	//  Used when entering menu/edit modes and characters need to be still
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		t.charanimstate = t.charanimstates[g.charanimindex];
		if (  t.charanimstate.obj>0 ) 
		{
			if (  ObjectExist(t.charanimstate.obj) == 1 ) 
			{
				SetObjectSpeed (  t.charanimstate.obj,0.0 );
			}
		}
	}

return;

}

void darkai_assignanimtofield ( void )
{
	//  291014 - AI system animation sets
	if (  t.tstartofaianim >= 0 ) 
	{
		t.tdidweuseweapstyleanim=0;
		for ( t.tcsiindex = 0 ; t.tcsiindex<=  g.csi_csimax; t.tcsiindex++ )
		{
			t.tryfield_s="";
			if (  t.tcsiindex == g.csi_relaxedANIM0  )  t.tryfield_s = "csi_relaxed1";
			if (  t.tcsiindex == g.csi_relaxedANIM1  )  t.tryfield_s = "csi_relaxed2";
			if (  t.tcsiindex == g.csi_relaxedmoveforeANIM  )  t.tryfield_s = "csi_relaxedmovefore";
			if (  t.tcsiindex == g.csi_cautiousANIM  )  t.tryfield_s = "csi_cautious";
			if (  t.tcsiindex == g.csi_cautiousmoveforeANIM  )  t.tryfield_s = "csi_cautiousmovefore";
			if (  t.tcsiindex == g.csi_unarmedANIM0  )  t.tryfield_s = "csi_unarmed1";
			if (  t.tcsiindex == g.csi_unarmedANIM1  )  t.tryfield_s = "csi_unarmed2";
			if (  t.tcsiindex == g.csi_unarmedconversationANIM  )  t.tryfield_s = "csi_unarmedconversation";
			if (  t.tcsiindex == g.csi_unarmedexplainANIM  )  t.tryfield_s = "csi_unarmedexplain";
			if (  t.tcsiindex == g.csi_unarmedpointforeANIM  )  t.tryfield_s = "csi_unarmedpointfore";
			if (  t.tcsiindex == g.csi_unarmedpointbackANIM  )  t.tryfield_s = "csi_unarmedpointback";
			if (  t.tcsiindex == g.csi_unarmedpointleftANIM  )  t.tryfield_s = "csi_unarmedpointleft";
			if (  t.tcsiindex == g.csi_unarmedpointrightANIM  )  t.tryfield_s = "csi_unarmedpointright";
			if (  t.tcsiindex == g.csi_unarmedmoveforeANIM  )  t.tryfield_s = "csi_unarmedmovefore";
			if (  t.tcsiindex == g.csi_unarmedmoverunANIM  )  t.tryfield_s = "csi_unarmedmoverun";
			if (  t.tcsiindex == g.csi_unarmedstairascendANIM  )  t.tryfield_s = "csi_unarmedstairascend";
			if (  t.tcsiindex == g.csi_unarmedstairdecendANIM  )  t.tryfield_s = "csi_unarmedstairdecend";
			if (  t.tcsiindex == g.csi_unarmedladderascendANIM0  )  t.tryfield_s = "csi_unarmedladderascend1";
			if (  t.tcsiindex == g.csi_unarmedladderascendANIM1  )  t.tryfield_s = "csi_unarmedladderascend2";
			if (  t.tcsiindex == g.csi_unarmedladderascendANIM2  )  t.tryfield_s = "csi_unarmedladderascend3";
			if (  t.tcsiindex == g.csi_unarmedladderdecendANIM0  )  t.tryfield_s = "csi_unarmedladderdecend1";
			if (  t.tcsiindex == g.csi_unarmedladderdecendANIM1  )  t.tryfield_s = "csi_unarmedladderdecend2";
			if (  t.tcsiindex == g.csi_unarmeddeathANIM  )  t.tryfield_s = "csi_unarmeddeath";
			if (  t.tcsiindex == g.csi_unarmedimpactforeANIM  )  t.tryfield_s = "csi_unarmedimpactfore";
			if (  t.tcsiindex == g.csi_unarmedimpactbackANIM  )  
			{
				t.tryfield_s = "csi_unarmedimpactback";
			}
			if (  t.tcsiindex == g.csi_unarmedimpactleftANIM  )  t.tryfield_s = "csi_unarmedimpactleft";
			if (  t.tcsiindex == g.csi_unarmedimpactrightANIM  )  t.tryfield_s = "csi_unarmedimpactright";
			if (  t.tcsiindex == g.csi_inchairANIM  )  t.tryfield_s = "csi_inchair";
			if (  t.tcsiindex == g.csi_inchairsitANIM  )  t.tryfield_s = "csi_inchairsit";
			if (  t.tcsiindex == g.csi_inchairgetupANIM  )  t.tryfield_s = "csi_inchairgetup";
			if (  t.tcsiindex == g.csi_swimANIM  )  t.tryfield_s = "csi_swim";
			if (  t.tcsiindex == g.csi_swimmoveforeANIM  )  t.tryfield_s = "csi_swimmovefore";
			t.strwork = "";
			bool bIsVaultAnim = false;
			for ( t.tweapsty = 1 ; t.tweapsty<=  5; t.tweapsty++ )
			{
				if (  t.tweapsty == 1  )  t.tweapsty_s = "";
				if (  t.tweapsty == 2  )  t.tweapsty_s = "_rocket";
				if (  t.tweapsty == 3  )  t.tweapsty_s = "_shotgun";
				if (  t.tweapsty == 4  )  t.tweapsty_s = "_uzi";
				if (  t.tweapsty == 5  )  t.tweapsty_s = "_assault";
				if (  t.tcsiindex == t.csi_stoodnormalANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodnormal";
				if (  t.tcsiindex == t.csi_stoodrocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodrocket";
				if (  t.tcsiindex == t.csi_stoodfidgetANIM0[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodfidget1";
				if (  t.tcsiindex == t.csi_stoodfidgetANIM1[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodfidget2";
				if (  t.tcsiindex == t.csi_stoodfidgetANIM2[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodfidget3";
				if (  t.tcsiindex == t.csi_stoodfidgetANIM3[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodfidget4";
				if (  t.tcsiindex == t.csi_stoodstartledANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodstartled";
				if (  t.tcsiindex == t.csi_stoodpunchANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodpunch";
				if (  t.tcsiindex == t.csi_stoodkickANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodkick";
				if (  t.tcsiindex == t.csi_stoodmoveforeANIM[t.tweapsty]  )  
					t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmovefore";
				if (  t.tcsiindex == t.csi_stoodmovebackANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoveback";
				if (  t.tcsiindex == t.csi_stoodmoveleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoveleft";
				if (  t.tcsiindex == t.csi_stoodmoverightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoveright";
				if (  t.tcsiindex == t.csi_stoodstepleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodstepleft";
				if (  t.tcsiindex == t.csi_stoodsteprightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodstepright";
				if (  t.tcsiindex == t.csi_stoodstrafeleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodstrafeleft";
				if (  t.tcsiindex == t.csi_stoodstraferightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodstraferight";
				if (  t.tcsiindex == t.csi_stoodvaultANIM[t.tweapsty]  )  { bIsVaultAnim = true; t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodvault"; }
				if (  t.tcsiindex == t.csi_stoodmoverunANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoverun";
				if (  t.tcsiindex == t.csi_stoodmoverunleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoverunleft";
				if (  t.tcsiindex == t.csi_stoodmoverunrightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodmoverunright";
				if (  t.tcsiindex == t.csi_stoodreloadANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodreload";
				if (  t.tcsiindex == t.csi_stoodreloadrocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodreloadrocket";
				if (  t.tcsiindex == t.csi_stoodwaveANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodwave";
				if (  t.tcsiindex == t.csi_stoodtossANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodtoss";
				if (  t.tcsiindex == t.csi_stoodfirerocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodfirerocket";
				if (  t.tcsiindex == t.csi_stoodincoverleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverleft";
				if (  t.tcsiindex == t.csi_stoodincoverpeekleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverpeekleft";
				if (  t.tcsiindex == t.csi_stoodincoverthrowleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverthrowleft";
				if (  t.tcsiindex == t.csi_stoodincoverrightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverright";
				if (  t.tcsiindex == t.csi_stoodincoverpeekrightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverpeekright";
				if (  t.tcsiindex == t.csi_stoodincoverthrowrightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodincoverthrowright";
				if (  t.tcsiindex == t.csi_stoodandturnANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_stoodandturn";
				if (  t.tcsiindex == t.csi_crouchidlenormalANIM0[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchidlenormal1";
				if (  t.tcsiindex == t.csi_crouchidlenormalANIM1[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchidlenormal2";
				if (  t.tcsiindex == t.csi_crouchidlerocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchidlerocket";
				if (  t.tcsiindex == t.csi_crouchdownANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchdown";
				if (  t.tcsiindex == t.csi_crouchdownrocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchdownrocket";
				if (  t.tcsiindex == t.csi_crouchrolldownANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchrolldown";
				if (  t.tcsiindex == t.csi_crouchrollupANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchrollup";
				if (  t.tcsiindex == t.csi_crouchmoveforeANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchmovefore";
				if (  t.tcsiindex == t.csi_crouchmovebackANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchmoveback";
				if (  t.tcsiindex == t.csi_crouchmoveleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchmoveleft";
				if (  t.tcsiindex == t.csi_crouchmoverightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchmoveright";
				if (  t.tcsiindex == t.csi_crouchmoverunANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchmoverun";
				if (  t.tcsiindex == t.csi_crouchreloadANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchreload";
				if (  t.tcsiindex == t.csi_crouchreloadrocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchreloadrocket";
				if (  t.tcsiindex == t.csi_crouchwaveANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchwave";
				if (  t.tcsiindex == t.csi_crouchtossANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchtoss";
				if (  t.tcsiindex == t.csi_crouchfirerocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchfirerocket";
				if (  t.tcsiindex == t.csi_crouchimpactforeANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchimpactfore";
				if (  t.tcsiindex == t.csi_crouchimpactbackANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchimpactback";
				if (  t.tcsiindex == t.csi_crouchimpactleftANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchimpactleft";
				if (  t.tcsiindex == t.csi_crouchimpactrightANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchimpactright";
				if (  t.tcsiindex == t.csi_crouchgetupANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchgetup";
				if (  t.tcsiindex == t.csi_crouchgetuprocketANIM[t.tweapsty]  )  t.tryfield_s = t.strwork + "csi"+t.tweapsty_s+"_crouchgetuprocket";
			}
			if (  t.field_s == t.tryfield_s && t.field_s != "" ) 
			{
				t.q=t.tstartofaianim+t.tcsiindex;
				if (  t.q <= g.animmax ) 
				{
					if (  t.entityanim[t.entid][t.q].start == 0 ) 
					{
						// ensure VAULT animation trims start and end
						if ( bIsVaultAnim == true && t.entityprofile[t.entid].jumpvaulttrim == 1 && t.value1 > 0 )
						{
							// trim ends of animation (vault needs to be quicker for jump usage)
							t.value1 += 10;
							t.value2 -= 10;
						}

						//  only use the first occurance (FPE entries first, Default Values second)
						t.entityanim[t.entid][t.q].start=t.value1 ; t.entityanim[t.entid][t.q].finish=t.value2;
						if (  t.q>t.tnewanimmax  )  t.tnewanimmax = t.q;
						//  if using extra anims from weapstyle, then flag this next gen entity for later
						if (  t.tcsiindex >= t.csi_stoodnormalANIM[2]  ) 
							t.entityprofile[t.entid].usesweapstyleanims = 1;
					}
				}
			}
		}
	}
}

void char_init ( void )
{
	//  Create array to hold transition information for per-object
	t.tmaxobjectnumber=90000; // allows 20,000 entities in each level
	Dim (  t.smoothanim,t.tmaxobjectnumber );

	//  Create arrays for object animation engine used only for characters
	t.tmaxcharacterstateengineentities=10;
	Dim (  t.charanimcontrols,t.tmaxcharacterstateengineentities );
	Dim (  t.charanimstates,t.tmaxcharacterstateengineentities );

	//  Create state engine database for character animations
	char_createseqdata ( );

return;

}

void char_createseqdata ( void )
{

	//  Create character anim sequence database
	t.tcharseqmax=750;
	Dim (  t.charseq,t.tcharseqmax  );
	g.csi_choosealertstate = -1;

	//  Initial state is LIMBO (corpse characters)
	t.csi=0;
	g.csi_limbo = t.csi;
	t.charseq[t.csi].mode=0 ; t.charseq[t.csi].trigger=0 ; ++t.csi;

	g.csi_relaxed = t.csi;
	t.charseq[t.csi].mode=91 ; t.charseq[t.csi].a_f=2 ; ++t.csi;
	g.csi_relaxedANIM0 = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=3 ; t.charseq[t.csi].a_f=900 ; t.charseq[t.csi].b_f=999 ; t.charseq[t.csi].c_f=10 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_relaxed ; ++t.csi;
	g.csi_relaxedANIM1 = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=3 ; t.charseq[t.csi].a_f=1000 ; t.charseq[t.csi].b_f=1282 ; t.charseq[t.csi].c_f=10 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_relaxed ; ++t.csi;
	g.csi_relaxedmovefore = t.csi;
	g.csi_relaxedmoveforeANIM = t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=3 ; t.charseq[t.csi].a_f=1290 ; t.charseq[t.csi].b_f=1320 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;

	g.csi_cautious = t.csi;
	g.csi_cautiousANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=9 ; t.charseq[t.csi].a_f=900 ; t.charseq[t.csi].b_f=999 ; t.charseq[t.csi].c_f=10 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_cautious ; ++t.csi;
	g.csi_cautiousmovefore = t.csi;
	g.csi_cautiousmoveforeANIM = t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].trigger=9 ; t.charseq[t.csi].a_f=1325 ; t.charseq[t.csi].b_f=1419 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_cautiousmovefore ; ++t.csi;

	g.csi_unarmed = t.csi;
	t.charseq[t.csi].mode=91 ; t.charseq[t.csi].a_f=2 ; ++t.csi;
	g.csi_unarmedANIM0 = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=3000 ; t.charseq[t.csi].b_f=3100 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedANIM1 = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=3430 ; t.charseq[t.csi].b_f=3697 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedconversation = t.csi;
	g.csi_unarmedconversationANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=3110 ; t.charseq[t.csi].b_f=3420 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedexplain = t.csi;
	g.csi_unarmedexplainANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=4260 ; t.charseq[t.csi].b_f=4464 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedpointfore = t.csi;
	g.csi_unarmedpointforeANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=4470 ; t.charseq[t.csi].b_f=4535 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedpointback = t.csi;
	g.csi_unarmedpointbackANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=4680 ; t.charseq[t.csi].b_f=4745 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedpointleft = t.csi;
	g.csi_unarmedpointleftANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=4610 ; t.charseq[t.csi].b_f=4675 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedpointright = t.csi;
	g.csi_unarmedpointrightANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=4540 ; t.charseq[t.csi].b_f=4605 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;
	g.csi_unarmedmovefore = t.csi;
	g.csi_unarmedmoveforeANIM = t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=3870 ; t.charseq[t.csi].b_f=3900 ; t.charseq[t.csi].c_f=20 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmedmovefore ; ++t.csi;
	g.csi_unarmedmoverun = t.csi;
	g.csi_unarmedmoverunANIM = t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=4 ; t.charseq[t.csi].a_f=3905 ; t.charseq[t.csi].b_f=3925 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=3.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmedmoverun ; ++t.csi;
	g.csi_unarmedstairascend = t.csi;
	g.csi_unarmedstairascendANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=5600 ; t.charseq[t.csi].b_f=5768 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=3.0 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=110.0 ; t.charseq[t.csi].angle_f=0 ; t.charseq[t.csi].a_f=100.0 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_choosealertstate ; ++t.csi;
	g.csi_unarmedstairdecend = t.csi;
	g.csi_unarmedstairdecendANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=5800 ; t.charseq[t.csi].b_f=5965 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=3.0 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=110.0 ; t.charseq[t.csi].angle_f=0 ; t.charseq[t.csi].a_f=-100.0 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_choosealertstate ; ++t.csi;
	g.csi_unarmedladderascend = t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=15.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	g.csi_unarmedladderascendANIM0 = t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].trigger=8 ; t.charseq[t.csi].loopback=4148 ; t.charseq[t.csi].a_f=4110 ; t.charseq[t.csi].b_f=4148 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0f ; t.charseq[t.csi].vertspeed_f=0.45f ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	g.csi_unarmedladderascendANIM1 = t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=8 ; t.charseq[t.csi].a_f=4148 ; t.charseq[t.csi].b_f=4225 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].vertspeed_f=0.5 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=53 ; t.charseq[t.csi].advancecap_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	g.csi_unarmedladderascendANIM2 = t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].trigger=8 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=4225 ; t.charseq[t.csi].b_f=4255 ; t.charseq[t.csi].c_f=4 ; t.charseq[t.csi].angle_f=0.0f ; t.charseq[t.csi].speed_f=0.9f ; t.charseq[t.csi].vertspeed_f=0.9f ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=10.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=100 ; t.charseq[t.csi].b_f=101 ; t.charseq[t.csi].c_f=4 ; t.charseq[t.csi].angle_f=0.0f ; t.charseq[t.csi].speed_f=0.9f ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_choosealertstate ; ++t.csi;
	g.csi_unarmedladderdecend = t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=-10.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=53 ; t.charseq[t.csi].advancecap_f=-10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	g.csi_unarmedladderdecendANIM0 = t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].trigger=8 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=4255 ; t.charseq[t.csi].b_f=4225 ; t.charseq[t.csi].c_f=4 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.9f ; t.charseq[t.csi].vertspeed_f=0.9f ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	g.csi_unarmedladderdecendANIM1 = t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=8 ; t.charseq[t.csi].a_f=4225 ; t.charseq[t.csi].b_f=4148 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].vertspeed_f=0.5 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=-15.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=100 ; t.charseq[t.csi].b_f=101 ; t.charseq[t.csi].c_f=4 ; t.charseq[t.csi].angle_f=0.0f ; t.charseq[t.csi].speed_f=0.9f ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_choosealertstate ; ++t.csi;
	g.csi_unarmeddeath = t.csi;
	g.csi_unarmeddeathANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=0 ; t.charseq[t.csi].a_f=4800 ; t.charseq[t.csi].b_f=4958 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	g.csi_unarmedimpactfore = t.csi;
	g.csi_unarmedimpactforeANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=4971 ; t.charseq[t.csi].b_f=5021 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	g.csi_unarmedimpactback = t.csi;
	g.csi_unarmedimpactbackANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5031 ; t.charseq[t.csi].b_f=5090 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	g.csi_unarmedimpactleft = t.csi;
	g.csi_unarmedimpactleftANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5171 ; t.charseq[t.csi].b_f=5229 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	g.csi_unarmedimpactright = t.csi;
	g.csi_unarmedimpactrightANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5101 ; t.charseq[t.csi].b_f=5160 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;

	g.csi_inchair = t.csi;
	g.csi_inchairANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=5 ; t.charseq[t.csi].a_f=3744 ; t.charseq[t.csi].b_f=3828 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_inchair ; ++t.csi;
	g.csi_inchairsit = t.csi;
	g.csi_inchairsitANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=5 ; t.charseq[t.csi].a_f=3710 ; t.charseq[t.csi].b_f=3744 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_inchair ; ++t.csi;
	g.csi_inchairgetup = t.csi;
	g.csi_inchairgetupANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=5 ; t.charseq[t.csi].a_f=3828 ; t.charseq[t.csi].b_f=3862 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_unarmed ; ++t.csi;

	g.csi_swim = t.csi;
	g.csi_swimANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=6 ; t.charseq[t.csi].a_f=3930 ; t.charseq[t.csi].b_f=4015 ; t.charseq[t.csi].c_f=10 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_swim ; ++t.csi;
	g.csi_swimmovefore = t.csi;
	g.csi_swimmoveforeANIM = t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=6 ; t.charseq[t.csi].a_f=4030 ; t.charseq[t.csi].b_f=4072 ; t.charseq[t.csi].c_f=10 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_swim ; ++t.csi;

	//  Create CSI instructions for all five weapon styles
	t.weapstylemax=5;
	for ( t.weapstyle = 1 ; t.weapstyle<=  t.weapstylemax; t.weapstyle++ )
	{
	Dim (  t.csi_stood,t.weapstylemax   ); t.csi_stood[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=61 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	Dim (  t.csi_stoodnormal,t.weapstylemax   ); t.csi_stoodnormal[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodnormalANIM,t.weapstylemax   ); t.csi_stoodnormalANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=100 ; t.charseq[t.csi].b_f=205 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodrocket,t.weapstylemax   ); t.csi_stoodrocket[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodrocketANIM,t.weapstylemax   ); t.csi_stoodrocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=6133 ; t.charseq[t.csi].b_f=6206 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodfidget,t.weapstylemax   ); t.csi_stoodfidget[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=91 ; t.charseq[t.csi].a_f=4 ; ++t.csi;
	Dim (  t.csi_stoodfidgetANIM0,t.weapstylemax   ); t.csi_stoodfidgetANIM0[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=100 ; t.charseq[t.csi].b_f=205 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodfidgetANIM1,t.weapstylemax   ); t.csi_stoodfidgetANIM1[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=210 ; t.charseq[t.csi].b_f=318 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodfidgetANIM2,t.weapstylemax   ); t.csi_stoodfidgetANIM2[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=325 ; t.charseq[t.csi].b_f=431 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodfidgetANIM3,t.weapstylemax   ); t.csi_stoodfidgetANIM3[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=440 ; t.charseq[t.csi].b_f=511 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodstartled,t.weapstylemax   ); t.csi_stoodstartled[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=91 ; t.charseq[t.csi].a_f=2 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodstartledANIM,t.weapstylemax   ); t.csi_stoodstartledANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=1425 ; t.charseq[t.csi].b_f=1465 ; t.charseq[t.csi].c_f=3 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodpunch,t.weapstylemax   ); t.csi_stoodpunch[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodpunchANIM,t.weapstylemax   ); t.csi_stoodpunchANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=2340 ; t.charseq[t.csi].b_f=2382 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodkick,t.weapstylemax   ); t.csi_stoodkick[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodkickANIM,t.weapstylemax   ); t.csi_stoodkickANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=5511 ; t.charseq[t.csi].b_f=5553 ; t.charseq[t.csi].c_f=4 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=54 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodmovefore,t.weapstylemax   ); t.csi_stoodmovefore[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoveforeANIM,t.weapstylemax   ); t.csi_stoodmoveforeANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=685 ; t.charseq[t.csi].b_f=707 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.5 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodmovefore[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodmoveback,t.weapstylemax   ); t.csi_stoodmoveback[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmovebackANIM,t.weapstylemax   ); t.csi_stoodmovebackANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=710 ; t.charseq[t.csi].b_f=735 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	Dim (  t.csi_stoodmoveleft,t.weapstylemax   ); t.csi_stoodmoveleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoveleftANIM,t.weapstylemax   ); t.csi_stoodmoveleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=740 ; t.charseq[t.csi].b_f=762 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	Dim (  t.csi_stoodmoveright,t.weapstylemax   ); t.csi_stoodmoveright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoverightANIM,t.weapstylemax   ); t.csi_stoodmoverightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=765 ; t.charseq[t.csi].b_f=789 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	Dim (  t.csi_stoodstepleft,t.weapstylemax   ); t.csi_stoodstepleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodstepleftANIM,t.weapstylemax   ); t.csi_stoodstepleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=610 ; t.charseq[t.csi].b_f=640 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.5 ; ++t.csi;
	Dim (  t.csi_stoodstepright,t.weapstylemax   ); t.csi_stoodstepright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodsteprightANIM,t.weapstylemax   ); t.csi_stoodsteprightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=645 ; t.charseq[t.csi].b_f=676 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.5 ; ++t.csi;
	Dim (  t.csi_stoodstrafeleft,t.weapstylemax   ); t.csi_stoodstrafeleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodstrafeleftANIM,t.weapstylemax   ); t.csi_stoodstrafeleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=855 ; t.charseq[t.csi].b_f=871 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=-90.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	Dim (  t.csi_stoodstraferight,t.weapstylemax   ); t.csi_stoodstraferight[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodstraferightANIM,t.weapstylemax   ); t.csi_stoodstraferightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=875 ; t.charseq[t.csi].b_f=892 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=90.0 ; t.charseq[t.csi].speed_f=1.0 ; ++t.csi;
	Dim (  t.csi_stoodvault,t.weapstylemax   ); t.csi_stoodvault[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodvaultANIM,t.weapstylemax   ); t.csi_stoodvaultANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].trigger=99 ; t.charseq[t.csi].a_f=0 ; t.charseq[t.csi].b_f=0 ; t.charseq[t.csi].c_f=1.0 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=2.0 ; ++t.csi; // 220217 - these now need to come from FPE
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=3.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodmoverun,t.weapstylemax   ); t.csi_stoodmoverun[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoverunANIM,t.weapstylemax   ); t.csi_stoodmoverunANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=795 ; t.charseq[t.csi].b_f=811 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=3.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodmoverunleft,t.weapstylemax   ); t.csi_stoodmoverunleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoverunleftANIM,t.weapstylemax   ); t.csi_stoodmoverunleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=815 ; t.charseq[t.csi].b_f=830 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=2.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodmoverunright,t.weapstylemax   ); t.csi_stoodmoverunright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodmoverunrightANIM,t.weapstylemax   ); t.csi_stoodmoverunrightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=1 ; t.charseq[t.csi].a_f=835 ; t.charseq[t.csi].b_f=850 ; t.charseq[t.csi].c_f=2 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=2.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodreload,t.weapstylemax   ); t.csi_stoodreload[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodreloadANIM,t.weapstylemax   ); t.csi_stoodreloadANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=515 ; t.charseq[t.csi].b_f=605 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=51 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodreloadrocket,t.weapstylemax   ); t.csi_stoodreloadrocket[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodreloadrocketANIM,t.weapstylemax   ); t.csi_stoodreloadrocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6233 ; t.charseq[t.csi].b_f=6315 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=51 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodwave,t.weapstylemax   ); t.csi_stoodwave[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodwaveANIM,t.weapstylemax   ); t.csi_stoodwaveANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=1470 ; t.charseq[t.csi].b_f=1520 ; t.charseq[t.csi].c_f=3 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodtoss,t.weapstylemax   ); t.csi_stoodtoss[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodtossANIM,t.weapstylemax   ); t.csi_stoodtossANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=2390 ; t.charseq[t.csi].b_f=2444 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodfirerocket,t.weapstylemax   ); t.csi_stoodfirerocket[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodfirerocketANIM,t.weapstylemax   ); t.csi_stoodfirerocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6207 ; t.charseq[t.csi].b_f=6232 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverleft,t.weapstylemax   ); t.csi_stoodincoverleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverleftANIM,t.weapstylemax   ); t.csi_stoodincoverleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=2 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=1580 ; t.charseq[t.csi].a_f=1580 ; t.charseq[t.csi].b_f=1580 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverleft[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverpeekleft,t.weapstylemax   ); t.csi_stoodincoverpeekleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverpeekleftANIM,t.weapstylemax   ); t.csi_stoodincoverpeekleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=1581 ; t.charseq[t.csi].a_f=1581 ; t.charseq[t.csi].b_f=1623 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverleft[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverthrowleft,t.weapstylemax   ); t.csi_stoodincoverthrowleft[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverthrowleftANIM,t.weapstylemax   ); t.csi_stoodincoverthrowleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=2680 ; t.charseq[t.csi].a_f=2680 ; t.charseq[t.csi].b_f=2778 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverleft[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverright,t.weapstylemax   ); t.csi_stoodincoverright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverrightANIM,t.weapstylemax   ); t.csi_stoodincoverrightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=2 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=1525 ; t.charseq[t.csi].a_f=1525 ; t.charseq[t.csi].b_f=1525 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverright[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverpeekright,t.weapstylemax   ); t.csi_stoodincoverpeekright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverpeekrightANIM,t.weapstylemax   ); t.csi_stoodincoverpeekrightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=1526 ; t.charseq[t.csi].a_f=1526 ; t.charseq[t.csi].b_f=1573 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverright[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodincoverthrowright,t.weapstylemax   ); t.csi_stoodincoverthrowright[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodincoverthrowrightANIM,t.weapstylemax   ); t.csi_stoodincoverthrowrightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=2570 ; t.charseq[t.csi].a_f=2570 ; t.charseq[t.csi].b_f=2668 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stoodincoverright[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_stoodandturn,t.weapstylemax   ); t.csi_stoodandturn[t.weapstyle]=t.csi;
	Dim (  t.csi_stoodandturnANIM,t.weapstylemax   ); t.csi_stoodandturnANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=7 ; t.charseq[t.csi].loopback=6051 ; t.charseq[t.csi].a_f=6070 ; t.charseq[t.csi].b_f=2668 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchidle,t.weapstylemax   ); t.csi_crouchidle[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=61 ; t.charseq[t.csi].trigger=2 ; ++t.csi;
	Dim (  t.csi_crouchidlenormal,t.weapstylemax   ); t.csi_crouchidlenormal[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=91 ; t.charseq[t.csi].a_f=2 ; ++t.csi;
	Dim (  t.csi_crouchidlenormalANIM0,t.weapstylemax   ); t.csi_crouchidlenormalANIM0[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=2 ; t.charseq[t.csi].a_f=1670 ; t.charseq[t.csi].b_f=1819 ; t.charseq[t.csi].c_f=5.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchidlenormalANIM1,t.weapstylemax   ); t.csi_crouchidlenormalANIM1[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=2 ; t.charseq[t.csi].a_f=1825 ; t.charseq[t.csi].b_f=1914 ; t.charseq[t.csi].c_f=5.0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchidlerocket,t.weapstylemax   ); t.csi_crouchidlerocket[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchidlerocketANIM,t.weapstylemax   ); t.csi_crouchidlerocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].trigger=2 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6472 ; t.charseq[t.csi].b_f=6545 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchdown,t.weapstylemax   ); t.csi_crouchdown[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchdownANIM,t.weapstylemax   ); t.csi_crouchdownANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=1630 ; t.charseq[t.csi].b_f=1646 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchdownrocket,t.weapstylemax   ); t.csi_crouchdownrocket[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchdownrocketANIM,t.weapstylemax   ); t.csi_crouchdownrocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6316 ; t.charseq[t.csi].b_f=6356 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchrolldown,t.weapstylemax   ); t.csi_crouchrolldown[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=-7.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	Dim (  t.csi_crouchrolldownANIM,t.weapstylemax   ); t.csi_crouchrolldownANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=1670 ; t.charseq[t.csi].a_f=2160 ; t.charseq[t.csi].b_f=2216 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.25 ; t.charseq[t.csi].speedinmiddle_f=1.25 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=4.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchrollup,t.weapstylemax   ); t.csi_crouchrollup[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=-7.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	Dim (  t.csi_crouchrollupANIM,t.weapstylemax   ); t.csi_crouchrollupANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=2225 ; t.charseq[t.csi].b_f=2281 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.25 ; t.charseq[t.csi].speedinmiddle_f=1.25 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=3.0 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchmovefore,t.weapstylemax   ); t.csi_crouchmovefore[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=52 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	Dim (  t.csi_crouchmoveforeANIM,t.weapstylemax   ); t.csi_crouchmoveforeANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=2075 ; t.charseq[t.csi].a_f=2075 ; t.charseq[t.csi].b_f=2102 ; t.charseq[t.csi].c_f=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=29.5 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	t.charseq[t.csi].mode=52 ; t.charseq[t.csi].trigger=0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchmoveback,t.weapstylemax   ); t.csi_crouchmoveback[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=52 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	t.charseq[t.csi].mode=3 ; t.charseq[t.csi].speed_f=-29.5 ; t.charseq[t.csi].angle_f=0 ; ++t.csi;
	Dim (  t.csi_crouchmovebackANIM,t.weapstylemax   ); t.csi_crouchmovebackANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=2102 ; t.charseq[t.csi].b_f=2131 ; t.charseq[t.csi].c_f=1 ; ++t.csi;
	t.charseq[t.csi].mode=52 ; t.charseq[t.csi].trigger=0 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchmoveleft,t.weapstylemax   ); t.csi_crouchmoveleft[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchmoveleftANIM,t.weapstylemax   ); t.csi_crouchmoveleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=2015 ; t.charseq[t.csi].b_f=2043 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.5 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchmoveright,t.weapstylemax   ); t.csi_crouchmoveright[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchmoverightANIM,t.weapstylemax   ); t.csi_crouchmoverightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=4 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=2043 ; t.charseq[t.csi].b_f=2072 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=0.5 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchmoverun,t.weapstylemax   ); t.csi_crouchmoverun[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchmoverunANIM,t.weapstylemax   ); t.csi_crouchmoverunANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=5 ; t.charseq[t.csi].trigger=2 ; t.charseq[t.csi].a_f=2135 ; t.charseq[t.csi].b_f=2153 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].angle_f=0.0 ; t.charseq[t.csi].speed_f=1.5 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchreload,t.weapstylemax   ); t.csi_crouchreload[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchreloadANIM,t.weapstylemax   ); t.csi_crouchreloadANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=1920 ; t.charseq[t.csi].b_f=2010 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=51 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchreloadrocket,t.weapstylemax   ); t.csi_crouchreloadrocket[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchreloadrocketANIM,t.weapstylemax   ); t.csi_crouchreloadrocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6380 ; t.charseq[t.csi].b_f=6471 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=51 ; t.charseq[t.csi].trigger=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchwave,t.weapstylemax   ); t.csi_crouchwave[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchwaveANIM,t.weapstylemax   ); t.csi_crouchwaveANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=2460 ; t.charseq[t.csi].b_f=2510 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchtoss,t.weapstylemax   ); t.csi_crouchtoss[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchtossANIM,t.weapstylemax   ); t.csi_crouchtossANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=2520 ; t.charseq[t.csi].b_f=2555 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchfirerocket,t.weapstylemax   ); t.csi_crouchfirerocket[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchfirerocketANIM,t.weapstylemax   ); t.csi_crouchfirerocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=6357 ; t.charseq[t.csi].b_f=6379 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_crouchidle[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchimpactfore,t.weapstylemax   ); t.csi_crouchimpactfore[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchimpactforeANIM,t.weapstylemax   ); t.csi_crouchimpactforeANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5240 ; t.charseq[t.csi].b_f=5277 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	Dim (  t.csi_crouchimpactback,t.weapstylemax   ); t.csi_crouchimpactback[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchimpactbackANIM,t.weapstylemax   ); t.csi_crouchimpactbackANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5290 ; t.charseq[t.csi].b_f=5339 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	Dim (  t.csi_crouchimpactleft,t.weapstylemax   ); t.csi_crouchimpactleft[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchimpactleftANIM,t.weapstylemax   ); t.csi_crouchimpactleftANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5409 ; t.charseq[t.csi].b_f=5466 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	Dim (  t.csi_crouchimpactright,t.weapstylemax   ); t.csi_crouchimpactright[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchimpactrightANIM,t.weapstylemax   ); t.csi_crouchimpactrightANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=0 ; t.charseq[t.csi].a_f=5350 ; t.charseq[t.csi].b_f=5395 ; t.charseq[t.csi].c_f=1 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=g.csi_limbo ; ++t.csi;
	Dim (  t.csi_crouchgetup,t.weapstylemax   ); t.csi_crouchgetup[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchgetupANIM,t.weapstylemax   ); t.csi_crouchgetupANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=1646 ; t.charseq[t.csi].b_f=1663 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	Dim (  t.csi_crouchgetuprocket,t.weapstylemax   ); t.csi_crouchgetuprocket[t.weapstyle]=t.csi;
	Dim (  t.csi_crouchgetuprocketANIM,t.weapstylemax   ); t.csi_crouchgetuprocketANIM[t.weapstyle]=t.csi;
	t.charseq[t.csi].mode=1 ; t.charseq[t.csi].loopback=100 ; t.charseq[t.csi].a_f=6573 ; t.charseq[t.csi].b_f=6607 ; t.charseq[t.csi].c_f=10 ; t.charseq[t.csi].freeze=1 ; ++t.csi;
	t.charseq[t.csi].mode=92 ; t.charseq[t.csi].a_f=t.csi_stood[t.weapstyle] ; ++t.csi;
	}
	g.csi_csimax = t.csi-1;
	if (  g.csi_csimax>t.tcharseqmax-10 ) 
	{
		ExitPrompt (  "getting close to g.csi_csimax max",""  ); 
		ExitProcess ( 0 );
	}

return;

}

void char_getcharseqcsifromplaycsi ( void )
{
	//  returns charseqcsia# and charseqcsib#
	t.q=t.entityprofile[t.ttentid].startofaianim;
	t.charseqcsia_f=t.entityanim[t.ttentid][t.q+t.charanimstate.playcsi].start;
	t.charseqcsib_f=t.entityanim[t.ttentid][t.q+t.charanimstate.playcsi].finish;
	if (  (t.charseqcsia_f == 100 && t.charseqcsib_f == 101) || t.charseqcsia_f == -1 || t.charseqcsib_f == -1 ) 
	{
		t.charseqcsia_f=t.entityanim[t.ttentid][t.q+t.csi_stoodnormalANIM[t.charanimstate.weapstyle]].start;
		t.charseqcsib_f=t.entityanim[t.ttentid][t.q+t.csi_stoodnormalANIM[t.charanimstate.weapstyle]].start+1;
	}
	t.charseqcsiloopback=t.charseq[t.charanimstate.playcsi].loopback;
	if (  t.charseqcsiloopback>0 ) 
	{
		if (  t.charseqcsiloopback == 100  )  t.charseqcsiloopback = t.entityanim[t.ttentid][t.q+t.csi_stoodnormalANIM[t.charanimstate.weapstyle]].start;
		if (  t.charseqcsiloopback == t.charseqcsia_f  )  t.charseqcsiloopback = t.charseqcsia_f;
		if (  t.charseqcsiloopback == t.charseqcsib_f  )  t.charseqcsiloopback = t.charseqcsib_f;
		t.tCrouchCSIANIMIndex=t.entityanim[t.ttentid][t.q+t.csi_crouchidlenormalANIM0[t.charanimstate.weapstyle]].start;
		if (  t.charseqcsiloopback == t.tCrouchCSIANIMIndex  )  t.charseqcsiloopback = t.tCrouchCSIANIMIndex;
	}
}

void char_loop ( void )
{
	// passes in charanimstate
	if ( 1 ) 
	{
		//  Animation sequence state engine
		t.ttentid=t.entityelement[t.charanimstate.e].bankindex;
		if ( t.charanimstate.playcsi >= 0 && t.entityprofile[t.ttentid].animstyle == 0 ) 
		{
			//  control flags
			t.moving=t.charanimcontrols[g.charanimindex].moving;
			t.ducking=t.charanimcontrols[g.charanimindex].ducking;
			t.spotactioning=t.charanimcontrols[g.charanimindex].spotactioning;
			t.leaping=t.charanimcontrols[g.charanimindex].leaping;
			t.impacting=t.charanimcontrols[g.charanimindex].impacting;
			t.alerted=t.charanimcontrols[g.charanimindex].alerted;
			t.meleeing=t.charanimcontrols[g.charanimindex].meleeing;
			t.covering=t.charanimcontrols[g.charanimindex].covering;

			//  actions
			do
			{

			t.nomorerepeats=1;

			if (  t.charseq[t.charanimstate.playcsi].mode == 0 ) 
			{
				//  limbo state
			}

			if (  t.charseq[t.charanimstate.playcsi].mode == 1 ) 
			{
				//  single and static
				char_getcharseqcsifromplaycsi ( );
				smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsia_f,t.charseqcsib_f,t.charseq[t.charanimstate.playcsi].c_f,0,1);
				if (  t.smoothanim[t.charanimstate.obj].transition == 0 && t.smoothanim[t.charanimstate.obj].playstarted == 1 && GetPlaying(t.charanimstate.obj) == 0 ) 
				{
					t.smoothanim[t.charanimstate.obj].playstarted=0;
					if (  t.charseqcsiloopback>0 ) 
					{
						SetObjectFrame (  t.charanimstate.obj,t.charseqcsiloopback );
						t.nomorerepeats=0;
					}
					if (  t.charanimcontrols[g.charanimindex].spotactioning != 0 ) 
					{
						//  reset spotactioning flag if get to end of this sequence (could have been a spot action)
						t.charanimcontrols[g.charanimindex].spotactioning=0;
					}
					++t.charanimstate.playcsi;
				}
				else
				{
					if (  t.smoothanim[t.charanimstate.obj].transition == 0 ) 
					{
					if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
					{
						//  STANDING MODE
						if (  t.alerted == 0 && t.charanimstate.alerted == 1 ) 
						{
							t.charanimstate.playcsi=g.csi_unarmed;
							t.charanimstate.alerted=0;
						}
						else
						{
							if (  t.impacting>0 ) 
							{
								if (  t.impacting == 5  )  t.charanimstate.playcsi = g.csi_unarmeddeath;
							}
							else
							{
								if (  t.ducking == 1 || t.ducking == 3 ) 
								{
									if (  t.ducking == 1 ) 
									{
										if (  t.charanimstate.rocketstyle == 1 ) 
										{
											t.charanimstate.playcsi=t.csi_crouchdownrocket[t.charanimstate.weapstyle];
										}
										else
										{
											t.charanimstate.playcsi=t.csi_crouchdown[t.charanimstate.weapstyle];
										}
									}
									if (  t.ducking == 3 ) 
									{
										t.charanimstate.playcsi=t.csi_crouchrolldown[t.charanimstate.weapstyle];
									}
								}
								else
								{
									if (  t.moving == 1  )  t.charanimstate.playcsi = t.csi_stoodmovefore[t.charanimstate.weapstyle];
									if (  t.moving == 2  )  t.charanimstate.playcsi = t.csi_stoodmoveback[t.charanimstate.weapstyle];
									if (  t.moving == 3  )  t.charanimstate.playcsi = t.csi_stoodmoveleft[t.charanimstate.weapstyle];
									if (  t.moving == 4  )  t.charanimstate.playcsi = t.csi_stoodmoveright[t.charanimstate.weapstyle];
									if (  t.moving == 5  )  t.charanimstate.playcsi = t.csi_stoodmoverun[t.charanimstate.weapstyle];
									if (  t.moving == 7  )  t.charanimstate.playcsi = t.csi_stoodmoverunleft[t.charanimstate.weapstyle];
									if (  t.moving == 8  )  t.charanimstate.playcsi = t.csi_stoodmoverunright[t.charanimstate.weapstyle];
									if (  t.moving == 9  )  t.charanimstate.playcsi = t.csi_stoodstepleft[t.charanimstate.weapstyle];
									if (  t.moving == 10  )  t.charanimstate.playcsi = t.csi_stoodstepright[t.charanimstate.weapstyle];
									if (  t.moving == 11  )  t.charanimstate.playcsi = t.csi_stoodstrafeleft[t.charanimstate.weapstyle];
									if (  t.moving == 12  )  t.charanimstate.playcsi = t.csi_stoodstraferight[t.charanimstate.weapstyle];
									if (  t.moving != 0  )  t.charanimstate.lastmoving = t.moving;
									if (  t.covering == 1  )  t.charanimstate.playcsi = t.csi_stoodincoverleft[t.charanimstate.weapstyle];
									if (  t.covering == 2  )  t.charanimstate.playcsi = t.csi_stoodincoverright[t.charanimstate.weapstyle];
									if (  t.covering == 3  )  t.charanimstate.playcsi = t.csi_stoodincoverpeekleft[t.charanimstate.weapstyle];
									if (  t.covering == 4  )  t.charanimstate.playcsi = t.csi_stoodincoverpeekright[t.charanimstate.weapstyle];
									if (  t.covering != 0  )  t.charanimstate.lastcovering = t.covering;
									if (  t.leaping == 1  )  t.charanimstate.playcsi = t.csi_stoodvault[t.charanimstate.weapstyle];
									if (  t.spotactioning == 1 ) 
									{
										if (  t.charanimstate.rocketstyle == 1 ) 
										{
											t.charanimstate.playcsi=t.csi_stoodreloadrocket[t.charanimstate.weapstyle];
										}
										else
										{
											t.charanimstate.playcsi=t.csi_stoodreload[t.charanimstate.weapstyle];
										}
									}
									if (  t.spotactioning == 2  )  t.charanimstate.playcsi = t.csi_stoodwave[t.charanimstate.weapstyle];
									if (  t.spotactioning == 3  )  t.charanimstate.playcsi = t.csi_stoodtoss[t.charanimstate.weapstyle];
									if (  t.spotactioning == 4 ) 
									{
										if (  t.charanimstate.rocketstyle == 1 ) 
										{
											t.charanimstate.playcsi=t.csi_stoodfirerocket[t.charanimstate.weapstyle];
										}
										else
										{
											//  some guns might have INSANE recoil, can be triggered here (must be STOOD though)
										}
									}
									if (  t.spotactioning == 10  )  t.charanimstate.playcsi = g.csi_unarmedstairascend;
									if (  t.spotactioning == 11  )  t.charanimstate.playcsi = g.csi_unarmedstairdecend;
									if (  t.spotactioning == 12  )  t.charanimstate.playcsi = g.csi_unarmedladderascend;
									if (  t.spotactioning == 13  )  t.charanimstate.playcsi = g.csi_unarmedladderdecend;
									if (  t.meleeing == 1 ) { t.charanimstate.playcsi = t.csi_stoodpunch[t.charanimstate.weapstyle]  ; t.charanimstate.punchorkickimpact = 0; }
									if (  t.meleeing == 2 ) { t.charanimstate.playcsi = t.csi_stoodkick[t.charanimstate.weapstyle]  ; t.charanimstate.punchorkickimpact = 0; }
								}
							}
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 2 ) 
					{
						//  CROUCH MODE
						if (  t.spotactioning == 1 || t.spotactioning == 2 || t.spotactioning == 3 ) 
						{
							if (  t.spotactioning == 1 ) 
							{
								if (  t.charanimstate.rocketstyle == 1 ) 
								{
									t.charanimstate.playcsi=t.csi_crouchreloadrocket[t.charanimstate.weapstyle];
								}
								else
								{
									t.charanimstate.playcsi=t.csi_crouchreload[t.charanimstate.weapstyle];
								}
							}
							if (  t.spotactioning == 2  )  t.charanimstate.playcsi = t.csi_crouchwave[t.charanimstate.weapstyle];
							if (  t.spotactioning == 3  )  t.charanimstate.playcsi = t.csi_crouchtoss[t.charanimstate.weapstyle];
							if (  t.spotactioning == 4 ) 
							{
								if (  t.charanimstate.rocketstyle == 1 ) 
								{
									t.charanimstate.playcsi=t.csi_crouchfirerocket[t.charanimstate.weapstyle];
								}
								else
								{
									//  some guns might have INSANE recoil, can be triggered here (must be STOOD though)
								}
							}
						}
						else
						{
							if (  t.ducking == 2 ) 
							{
								if (  t.charanimstate.rocketstyle == 1 ) 
								{
									t.charanimstate.playcsi=t.csi_crouchgetuprocket[t.charanimstate.weapstyle];
								}
								else
								{
									t.charanimstate.playcsi=t.csi_crouchgetup[t.charanimstate.weapstyle];
								}
							}
							else
							{
								if (  t.impacting>0 ) 
								{
									if (  t.impacting == 5  )  t.charanimstate.playcsi = t.csi_crouchimpactback[t.charanimstate.weapstyle];
								}
								else
								{
									if (  t.moving == 1  )  t.charanimstate.playcsi = t.csi_crouchmovefore[t.charanimstate.weapstyle];
									if (  t.moving == 2  )  t.charanimstate.playcsi = t.csi_crouchmoveback[t.charanimstate.weapstyle];
									if (  t.moving == 3  )  t.charanimstate.playcsi = t.csi_crouchmoveleft[t.charanimstate.weapstyle];
									if (  t.moving == 4  )  t.charanimstate.playcsi = t.csi_crouchmoveright[t.charanimstate.weapstyle];
									if (  t.moving == 5  )  t.charanimstate.playcsi = t.csi_crouchmoverun[t.charanimstate.weapstyle];
									if (  t.moving == 6  )  t.charanimstate.playcsi = t.csi_crouchrollup[t.charanimstate.weapstyle];
								}
							}
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 3 ) 
					{
						//  RELAXED MODE
						if (  t.moving == 1  )  t.charanimstate.playcsi = g.csi_relaxedmovefore;
						if (  t.alerted == 1 && t.charanimstate.alerted == 0 ) 
						{
							t.charanimstate.playcsi=t.csi_stoodstartled[t.charanimstate.weapstyle];
							t.charanimstate.alerted=1;
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 4 ) 
					{
						//  UNARMED MODE
						if (  t.alerted == 1 && t.charanimstate.alerted == 0 ) 
						{
							if (  t.entityelement[t.charanimstate.e].eleprof.hasweapon>0 ) 
							{
								t.charanimstate.playcsi=t.csi_stood[t.charanimstate.weapstyle];
							}
							else
							{
								//  remain unarmed (could have a 'starled anim' here)
							}
							t.charanimstate.alerted=1;
						}
						else
						{
							if (  t.moving == 1  )  t.charanimstate.playcsi = g.csi_unarmedmovefore;
							if (  t.moving == 5  )  t.charanimstate.playcsi = g.csi_unarmedmoverun;
							if (  t.moving != 0  )  t.charanimstate.lastmoving = t.moving;
							if (  t.ducking == 1  )  t.charanimstate.playcsi = t.csi_crouchdown[t.charanimstate.weapstyle];
							if (  t.spotactioning == 4  )  t.charanimstate.playcsi = g.csi_unarmedconversation;
							if (  t.spotactioning == 5  )  t.charanimstate.playcsi = g.csi_unarmedexplain;
							if (  t.spotactioning == 6  )  t.charanimstate.playcsi = g.csi_unarmedpointfore;
							if (  t.spotactioning == 7  )  t.charanimstate.playcsi = g.csi_unarmedpointback;
							if (  t.spotactioning == 8  )  t.charanimstate.playcsi = g.csi_unarmedpointleft;
							if (  t.spotactioning == 9  )  t.charanimstate.playcsi = g.csi_unarmedpointright;
							if (  t.spotactioning == 10  )  t.charanimstate.playcsi = g.csi_unarmedstairascend;
							if (  t.spotactioning == 11  )  t.charanimstate.playcsi = g.csi_unarmedstairdecend;
							if (  t.spotactioning == 12  )  t.charanimstate.playcsi = g.csi_unarmedladderascend;
							if (  t.spotactioning == 13  )  t.charanimstate.playcsi = g.csi_unarmedladderdecend;
							if (  t.impacting == 5  )  t.charanimstate.playcsi = g.csi_unarmeddeath;
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 5 ) 
					{
						//  CHAIR MODE
						if (  t.ducking == 2  )  t.charanimstate.playcsi = g.csi_inchairgetup;
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 6 ) 
					{
						//  SWIM MODE
						if (  t.moving == 1  )  t.charanimstate.playcsi = g.csi_swimmovefore;
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 7 ) 
					{
						//  STANDING IN COVER MODE
					}
				}
				}
			}

			if (  t.charseq[t.charanimstate.playcsi].mode == 2 ) 
			{
				//  loop and static
				char_getcharseqcsifromplaycsi ( );
				smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsia_f,t.charseqcsib_f,t.charseq[t.charanimstate.playcsi].c_f,0,0);
				if (  t.smoothanim[t.charanimstate.obj].transition == 0 && GetFrame(t.charanimstate.obj) >= t.charseqcsia_f && GetFrame(t.charanimstate.obj) <= t.charseqcsib_f ) 
				{
					if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
					{
						//  STANDING MODE
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 2 ) 
					{
						//  CROUCH MODE
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 3 ) 
					{
						//  RELAXED MODE
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 7 ) 
					{
						//  STANDING IN COVER MODE
						if (  t.moving != 0  )  t.charanimstate.playcsi = t.csi_stood[t.charanimstate.weapstyle];
						if (  t.covering == 1  )  t.charanimstate.playcsi = t.csi_stoodincoverleft[t.charanimstate.weapstyle];
						if (  t.covering == 2  )  t.charanimstate.playcsi = t.csi_stoodincoverright[t.charanimstate.weapstyle];
						if (  t.covering == 3  )  t.charanimstate.playcsi = t.csi_stoodincoverpeekleft[t.charanimstate.weapstyle];
						if (  t.covering == 4  )  t.charanimstate.playcsi = t.csi_stoodincoverpeekright[t.charanimstate.weapstyle];
						if (  t.covering != 0 && t.covering != t.charanimstate.lastcovering ) 
						{
							t.charanimstate.lastcovering=t.covering;
						}
						if (  t.spotactioning == 3 && (t.charanimstate.lastcovering == 1 || t.charanimstate.lastcovering == 3)  )  t.charanimstate.playcsi = t.csi_stoodincoverthrowleft[t.charanimstate.weapstyle];
						if (  t.spotactioning == 3 && (t.charanimstate.lastcovering == 2 || t.charanimstate.lastcovering == 4)  )  t.charanimstate.playcsi = t.csi_stoodincoverthrowright[t.charanimstate.weapstyle];
					}
				}
			}

			if (  t.charseq[t.charanimstate.playcsi].mode == 3 ) 
			{
				//  shift position
				t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.charseq[t.charanimstate.playcsi].speed_f;
				t.tfinalspeed_f=t.tfinalspeed_f*(t.entityelement[t.charanimstate.e].eleprof.speed/100.0);
				t.tnewx_f=NewXValue(t.entityelement[t.charanimstate.e].x,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f*-1);
				t.tnewz_f=NewZValue(t.entityelement[t.charanimstate.e].z,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f);
				t.tnewy_f=t.entityelement[t.charanimstate.e].y+t.charseq[t.charanimstate.playcsi].a_f;
				t.entityelement[t.charanimstate.e].x=t.tnewx_f;
				t.entityelement[t.charanimstate.e].y=t.tnewy_f;
				t.entityelement[t.charanimstate.e].z=t.tnewz_f;
				t.nomorerepeats=0;
				++t.charanimstate.playcsi;
			}

			if (  t.charseq[t.charanimstate.playcsi].mode == 4 ) 
			{
				//  single and slide moving
				t.tplayended=0;
				char_getcharseqcsifromplaycsi ( );
				if (  t.charseqcsib_f>t.charseqcsia_f ) 
				{
					smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsia_f,t.charseqcsib_f,t.charseq[t.charanimstate.playcsi].c_f,0,1);
					if (  t.smoothanim[t.charanimstate.obj].transition == 0 && t.smoothanim[t.charanimstate.obj].playstarted == 1 && GetPlaying(t.charanimstate.obj) == 0  )  t.tplayended = 1;
				}
				else
				{
					smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsib_f,t.charseqcsia_f,t.charseq[t.charanimstate.playcsi].c_f,1,1);
					if (  t.smoothanim[t.charanimstate.obj].transition == 0 && t.smoothanim[t.charanimstate.obj].playstarted == 1 && GetFrame(t.charanimstate.obj)<t.charseq[t.charanimstate.playcsi].b_f  )  t.tplayended = 1;
				}
				if (  t.tplayended == 1 ) 
				{
					t.smoothanim[t.charanimstate.obj].playstarted=0;
					if (  t.charseqcsiloopback>0 ) 
					{
						SetObjectFrame (  t.charanimstate.obj,t.charseqcsiloopback );
						t.nomorerepeats=0;
					}
					t.charanimstate.advancedcapactive=0;
					++t.charanimstate.playcsi;
				}
				else
				{
					//  special case - detect if PUNCH / KICK hit player
					//  only do damage if not third person or this obj not the protagonist
					t.tokay = 0 ; if (  t.playercontrol.thirdperson.enabled == 0 || (t.playercontrol.thirdperson.enabled == 1 && t.charanimstate.e != t.playercontrol.thirdperson.charactere)  )  t.tokay = 1;
					if (  t.tokay == 1 && (t.charanimstate.playcsi == t.csi_stoodpunch[t.charanimstate.weapstyle] || t.charanimstate.playcsi == t.csi_stoodkick[t.charanimstate.weapstyle]) ) 
					{
						char_getcharseqcsifromplaycsi ( );
						t.tmidframe_f=t.charseqcsia_f+((t.charseqcsib_f-t.charseqcsia_f)/2);
						if (  GetFrame(t.charanimstate.obj)>t.tmidframe_f ) 
						{
							if (  t.charanimstate.punchorkickimpact == 0 ) 
							{
								t.charanimstate.punchorkickimpact=1;
								//  but only if STILL in range
								if (  t.tplrproxd_f<80.0 ) 
								{
									//  emit sound as player gets thumped!
									t.tpunchsnd=t.playercontrol.soundstartindex+8+Rnd(2);
									if (  SoundPlaying(t.tpunchsnd) == 0 ) 
									{
										PlaySound (  t.tpunchsnd );
									}
									//  inflict damage to player
									t.te=t.charanimstate.e;
									t.tdamage=t.entityelement[t.te].eleprof.strength/10;
									physics_player_takedamage ( );
									//  also apply some force to charater controller
									t.tfacingangle_f=WrapValue(t.tplrproxa_f)-WrapValue(ObjectAngleY(t.charanimstate.obj));
									if (  t.tfacingangle_f>180  )  t.tfacingangle_f = t.tfacingangle_f-360.0;
									if (  t.tfacingangle_f<-180  )  t.tfacingangle_f = t.tfacingangle_f+360.0;
									t.tfacingdegree_f=Sin(t.tfacingangle_f);
									if (  t.charanimstate.playcsi == t.csi_stoodkick[t.charanimstate.weapstyle] ) 
									{
										t.playercontrol.pushforce_f=2.0;
										if (  t.tfacingdegree_f >= 0.0 ) 
										{
											t.playercontrol.flinchx_f=1.0-t.tfacingdegree_f;
										}
									}
									else
									{
										t.playercontrol.pushforce_f=1.5;
										t.playercontrol.flinchy_f=-1.0*t.tfacingdegree_f;
										t.playercontrol.flinchz_f=5.0;
									}
									t.playercontrol.pushangle_f=ObjectAngleY(t.charanimstate.obj);
								}
							}
						}
					}
					//  variable speed based on middle setting
					char_getcharseqcsifromplaycsi ( );
					t.tanimframearc_f=((GetFrame(t.charanimstate.obj)-t.charseqcsia_f)/(t.charseqcsib_f-t.charseqcsia_f))*180.0;
					//  advance cap
					t.tspeed_f=t.charseq[t.charanimstate.playcsi].speed_f+(Sin(t.tanimframearc_f)*t.charseq[t.charanimstate.playcsi].speedinmiddle_f);
					t.tspeed_f=t.tspeed_f*2.5*g.timeelapsed_f;
					if (  t.charanimstate.advancedcapactive == 1 ) 
					{
						if (  abs(t.charanimstate.advanced_f)>0 ) 
						{
							if (  t.charanimstate.advanced_f>0 ) 
							{
								t.revvalue=1;
								t.charanimstate.advanced_f=t.charanimstate.advanced_f-t.tspeed_f;
								if (  t.charanimstate.advanced_f<0 ) 
								{
									t.tspeed_f=t.tspeed_f+t.charanimstate.advanced_f;
									t.charanimstate.advanced_f=0.0;
								}
							}
							else
							{
								t.revvalue=-1;
								t.charanimstate.advanced_f=t.charanimstate.advanced_f+t.tspeed_f;
								if (  t.charanimstate.advanced_f>0 ) 
								{
									t.tspeed_f=t.tspeed_f-t.charanimstate.advanced_f;
									t.charanimstate.advanced_f=0.0;
								}
							}
							t.tspeed_f=t.tspeed_f*t.revvalue;
						}
						else
						{
							t.tspeed_f=0.0;
						}
					}
					t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.tspeed_f;
					if (  t.charseq[t.charanimstate.playcsi].alignpos == 1 ) 
					{
						//  quickly align visual object with A.I entity object as need to be spot on
						t.tnewx_f=AIGetEntityX(t.charanimstate.obj);
						t.tnewz_f=AIGetEntityZ(t.charanimstate.obj);
						if (  t.terrain.TerrainID>0 ) 
						{
							t.tnewy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tnewx_f,t.tnewz_f);
						}
						else
						{
							t.tnewy_f=1000.0;
						}
					}
					else
					{
						//  slide moving
						t.tfinalspeed_f=t.tfinalspeed_f*(t.entityelement[t.charanimstate.e].eleprof.speed/100.0);
						t.tnewx_f=NewXValue(t.entityelement[t.charanimstate.e].x,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f*-1);
						t.tnewy_f=t.entityelement[t.charanimstate.e].y;
						t.tnewz_f=NewZValue(t.entityelement[t.charanimstate.e].z,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f);
						if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
						{
							//  STANDING MODE
							if (  t.moving != t.charanimstate.lastmoving ) 
							{
								t.charanimstate.advancedcapactive=0;
								++t.charanimstate.playcsi;
							}
						}
						if (  t.charseq[t.charanimstate.playcsi].trigger == 8 ) 
						{
							//  CLIMBING LADDER MODE
							if (  GetFrame(t.charanimstate.obj) >= 4126 && GetFrame(t.charanimstate.obj)<4248 ) 
							{
								t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.charseq[t.charanimstate.playcsi].vertspeed_f;
								if (  t.charanimstate.destinationdirection == 1 ) 
								{
									t.tnewy_f=t.tnewy_f+t.tfinalspeed_f;
									if (  t.tnewy_f>t.charanimstate.destinationheight_f  )  t.tnewy_f = t.charanimstate.destinationheight_f;
								}
								else
								{
									t.tnewy_f=t.tnewy_f-t.tfinalspeed_f;
									if (  t.tnewy_f<t.charanimstate.destinationheight_f  )  t.tnewy_f = t.charanimstate.destinationheight_f;
								}
							}
						}
						if (  t.charseq[t.charanimstate.playcsi].trigger == 99 ) 
						{
							//  can intercept a jump if moving and end of jump hits Floor (  )
							if (  t.playercontrol.jumpmode != 1 && t.playercontrol.movement != 0 ) 
							{
								float fStartFrame, fHoldFrame, fResumeFrame, fFinishFrame;
								int entid = t.entityelement[t.charanimstate.originale].bankindex;
								physics_getcorrectjumpframes ( entid, &fStartFrame, &fHoldFrame, &fResumeFrame, &fFinishFrame );
								if (  GetFrame (t.charanimstate.obj) > fResumeFrame-1.0f ) 
								{
									//  third person in end of contact with ground
									t.charanimstate.playcsi=t.csi_stood[t.charanimstate.weapstyle];
								}
							}
						}
					}
					t.entityelement[t.charanimstate.e].x=t.tnewx_f;
					t.entityelement[t.charanimstate.e].y=t.tnewy_f;
					t.entityelement[t.charanimstate.e].z=t.tnewz_f;
				}
			}

			if (  t.charseq[t.charanimstate.playcsi].mode == 5 ) 
			{
				//  loop and slide moving
				char_getcharseqcsifromplaycsi ( );
				if (  t.charseqcsib_f>t.charseqcsia_f ) 
				{
					smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsia_f,t.charseqcsib_f,t.charseq[t.charanimstate.playcsi].c_f,0,0);
				}
				else
				{
					smoothanimtriggerrev(t.charanimstate.obj,t.charseqcsib_f,t.charseqcsia_f,t.charseq[t.charanimstate.playcsi].c_f,1,0);
				}
				if (  t.smoothanim[t.charanimstate.obj].transition == 0 && GetFrame(t.charanimstate.obj) >= t.charseqcsia_f && GetFrame(t.charanimstate.obj) <= t.charseqcsib_f ) 
				{
					t.charanimstate.advancedcapactive=0;
					if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
					{
						//  STANDING MODE
						if (  t.alerted == 0 && t.charanimstate.alerted == 1 ) 
						{
							t.charanimstate.playcsi=g.csi_unarmed;
							t.charanimstate.alerted=0;
						}
						else
						{
							if ( t.leaping == 1 || t.meleeing == 1 ) 
							{
								// 230217 - added punch melee command even when moving/running
								if ( t.meleeing == 1 ) t.charanimstate.playcsi=t.csi_stoodpunch[t.charanimstate.weapstyle];
								if ( t.leaping == 1 ) t.charanimstate.playcsi=t.csi_stoodvault[t.charanimstate.weapstyle];
							}
							else
							{
								if (  t.moving != t.charanimstate.lastmoving ) 
								{
									if (  t.moving == 1 || t.moving == 5 || t.moving == 7 || t.moving == 8 ) 
									{
										//  break instant into run animation
										if (  t.moving == 1  )  t.charanimstate.playcsi = t.csi_stoodmovefore[t.charanimstate.weapstyle];
										if (  t.moving == 5  )  t.charanimstate.playcsi = t.csi_stoodmoverun[t.charanimstate.weapstyle];
										if (  t.moving == 7  )  t.charanimstate.playcsi = t.csi_stoodmoverunleft[t.charanimstate.weapstyle];
										if (  t.moving == 8  )  t.charanimstate.playcsi = t.csi_stoodmoverunright[t.charanimstate.weapstyle];
										if (  t.moving != 0  )  t.charanimstate.lastmoving = t.moving;
									}
									else
									{
										t.charanimstate.playcsi=t.csi_stood[t.charanimstate.weapstyle];
									}
								}
							}
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 2 ) 
					{
						//  CROUCH MODE
						if (  t.moving != 5  )  t.charanimstate.playcsi = t.csi_crouchidle[t.charanimstate.weapstyle];
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 3 ) 
					{
						//  RELAXED MODE
						if (  t.alerted == 1 && t.charanimstate.alerted == 0 ) 
						{
							t.charanimstate.playcsi=t.csi_stood[t.charanimstate.weapstyle];
							t.charanimstate.alerted=1;
						}
						else
						{
							if (  t.moving != 1  )  t.charanimstate.playcsi = g.csi_relaxed;
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 4 ) 
					{
						//  UNARMED MODE
						if ( t.alerted == 1 && t.charanimstate.alerted == 0 ) 
						{
							if ( t.charanimstate.weapstyle==0 )
								t.charanimstate.playcsi = g.csi_unarmed;
							else
								t.charanimstate.playcsi = t.csi_stood[t.charanimstate.weapstyle];
							t.charanimstate.alerted = 1;
						}
						else
						{
							if (  t.moving != t.charanimstate.lastmoving  )  
								t.charanimstate.playcsi = g.csi_unarmed;
						}
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 5 ) 
					{
						//  CHAIR MODE
					}
					if (  t.charseq[t.charanimstate.playcsi].trigger == 6 ) 
					{
						//  SWIM MODE
						if (  t.moving != t.charanimstate.lastmoving  )  t.charanimstate.playcsi = g.csi_swim;
					}
				}
				//  avoid jerky starts in motion by smoothing off speed
				t.tspeed_f=t.charseq[t.charanimstate.playcsi].speed_f*t.charanimstate.warmupwalk_f;
				t.tspeed_f=t.tspeed_f*2.5*g.timeelapsed_f;
				//  advance cap
				if (  t.charanimstate.advancedcapactive == 1 ) 
				{
					if (  abs(t.charanimstate.advanced_f)>0 ) 
					{
						if (  t.charanimstate.advanced_f>0 ) 
						{
							t.revvalue=1;
							t.charanimstate.advanced_f=t.charanimstate.advanced_f-t.tspeed_f;
							if (  t.charanimstate.advanced_f<0 ) 
							{
								t.tspeed_f=t.tspeed_f+t.charanimstate.advanced_f;
								t.charanimstate.advanced_f=0.0;
							}
						}
						else
						{
							t.revvalue=-1;
							t.charanimstate.advanced_f=t.charanimstate.advanced_f+t.tspeed_f;
							if (  t.charanimstate.advanced_f>0 ) 
							{
								t.tspeed_f=t.tspeed_f-t.charanimstate.advanced_f;
								t.charanimstate.advanced_f=0.0;
							}
						}
						t.tspeed_f=t.tspeed_f*t.revvalue;
					}
					else
					{
						t.tspeed_f=0.0;
					}
				}
				t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.tspeed_f;
				t.tfinalspeed_f=t.tfinalspeed_f*(t.entityelement[t.charanimstate.e].eleprof.speed/100.0);
				if (  t.charseq[t.charanimstate.playcsi].alignpos == 1 ) 
				{
					//  quickly align visual object with A.I entity object as need to be spot on
					t.tnewx_f=AIGetEntityX(t.charanimstate.obj);
					t.tnewz_f=AIGetEntityZ(t.charanimstate.obj);
					if (  t.terrain.TerrainID>0 ) 
					{
						t.tnewy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tnewx_f,t.tnewz_f);
					}
					else
					{
						t.tnewy_f=1000.0;
					}
				}
				else
				{
					//  slide moving
					t.tnewx_f=NewXValue(t.entityelement[t.charanimstate.e].x,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f*-1);
					t.tnewy_f=t.entityelement[t.charanimstate.e].y;
					t.tnewz_f=NewZValue(t.entityelement[t.charanimstate.e].z,t.charseq[t.charanimstate.playcsi].angle_f-t.charanimstate.moveangle_f,t.tfinalspeed_f);
					if (  t.charseq[t.charanimstate.playcsi].trigger == 8 ) 
					{
						//  CLIMBING LADDER MODE
						t.tfinalspeed_f=t.entityelement[t.charanimstate.e].speedmodulator_f*t.charseq[t.charanimstate.playcsi].vertspeed_f;
						if (  t.charanimstate.destinationdirection == 1 ) 
						{
							t.tnewy_f=t.tnewy_f+t.tfinalspeed_f;
							if (  t.tnewy_f>t.charanimstate.destinationheight_f-44.0 ) 
							{
								t.charanimstate.advancedcapactive=0;
								++t.charanimstate.playcsi;
							}
						}
						else
						{
							t.tnewy_f=t.tnewy_f-t.tfinalspeed_f;
							if (  t.tnewy_f<t.charanimstate.destinationheight_f ) 
							{
								t.charanimstate.advancedcapactive=0;
								++t.charanimstate.playcsi;
							}
						}
					}
				}
				t.entityelement[t.charanimstate.e].x=t.tnewx_f;
				t.entityelement[t.charanimstate.e].y=t.tnewy_f;
				t.entityelement[t.charanimstate.e].z=t.tnewz_f;
			}

			//  special in-game change actions
			if (  t.charseq[t.charanimstate.playcsi].mode == 51 ) 
			{
				if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
				{
					//  RELOAD WEAPON / ROCKET
					t.charanimstate.ammoinclip=t.charanimstate.ammoinclipmax;
				}
				//  all in-game changes resets spot action
				t.charanimcontrols[g.charanimindex].spotactioning=0;
				//  and moves on to next in sequence
				++t.charanimstate.playcsi;
			}
			if (  t.charseq[t.charanimstate.playcsi].mode == 52 ) 
			{
				if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
				{
					t.charanimstate.freezeanyrotation=1;
				}
				else
				{
					t.charanimstate.freezeanyrotation=0;
				}
				++t.charanimstate.playcsi;
			}
			if (  t.charseq[t.charanimstate.playcsi].mode == 53 ) 
			{
				t.charanimstate.advanced_f=t.charseq[t.charanimstate.playcsi].advancecap_f;
				t.charanimstate.advancedcapactive=1;
				++t.charanimstate.playcsi;
				t.nomorerepeats=0;
			}
			if (  t.charseq[t.charanimstate.playcsi].mode == 54 ) 
			{
				//  switch spot action flag back to zero (signal that a seuqnce has finished (stairs/ladders))
				t.charanimcontrols[g.charanimindex].spotactioning=0;
				t.charanimcontrols[g.charanimindex].meleeing=0;
				++t.charanimstate.playcsi;
				t.nomorerepeats=0;
			}

			//  choice based on factors
			if (  t.charseq[t.charanimstate.playcsi].mode == 61 ) 
			{
				if (  t.charseq[t.charanimstate.playcsi].trigger == 1 ) 
				{
					if (  t.charanimstate.rocketstyle == 1 ) 
					{
						t.charanimstate.playcsi=t.csi_stoodrocket[t.charanimstate.weapstyle];
					}
					else
					{
						t.charanimstate.playcsi=t.csi_stoodnormal[t.charanimstate.weapstyle];
					}
				}
				if (  t.charseq[t.charanimstate.playcsi].trigger == 2 ) 
				{
					if (  t.charanimstate.rocketstyle == 1 ) 
					{
						t.charanimstate.playcsi=t.csi_crouchidlerocket[t.charanimstate.weapstyle];
					}
					else
					{
						t.charanimstate.playcsi=t.csi_crouchidlenormal[t.charanimstate.weapstyle];
					}
				}
			}

			//  leap conditions
			if (  t.charseq[t.charanimstate.playcsi].mode == 91 ) 
			{
				t.trndvalue=Rnd(t.charseq[t.charanimstate.playcsi].a_f);
				if (  t.trndvalue<1  )  t.trndvalue = 1;
				if (  t.trndvalue>t.charseq[t.charanimstate.playcsi].a_f  )  t.trndvalue = t.charseq[t.charanimstate.playcsi].a_f;
				if (  t.trndvalue>1 && Rnd(2) != 1  )  t.trndvalue = 1;
				if (  t.charanimstate.obj>0 ) 
				{
					if (  t.charanimstate.cantrulyfire == 1  )  t.trndvalue = 1;
				}
				//  each event has a '92' to return to random selection ergo the gaps
				if (  t.trndvalue == 1 ) 
				{
					t.trndvalue=1;
				}
				else
				{
					if (  t.trndvalue == 2 ) 
					{
						t.trndvalue=3;
					}
					else
					{
						if (  t.trndvalue == 3 ) 
						{
							t.trndvalue=5;
						}
						else
						{
							if (  t.trndvalue == 4  )  t.trndvalue = 7;
						}
					}
				}
				t.charanimstate.playcsi=t.charanimstate.playcsi+t.trndvalue;
				t.nomorerepeats=0;
			}
			if (  t.charseq[t.charanimstate.playcsi].mode == 92 ) 
			{
				if (  t.charseq[t.charanimstate.playcsi].a_f == g.csi_choosealertstate ) 
				{
					if (  t.charanimstate.alerted == 0 ) 
					{
						t.charanimstate.playcsi=g.csi_unarmed;
					}
					else
					{
						t.charanimstate.playcsi=t.csi_stood[t.charanimstate.weapstyle];
					}
				}
				else
				{
					t.charanimstate.playcsi=t.charseq[t.charanimstate.playcsi].a_f;
				}
				t.smoothanim[t.charanimstate.obj].st=-1;
				t.nomorerepeats=0;
			}
			if (  t.charanimstate.playcsi>g.csi_csimax ) 
			{
				//  end sequence
				t.nomorerepeats=1;
				t.charanimstate.playcsi=-1;
			}
			} while ( !(  t.nomorerepeats == 1 ) );

		}

		//  Update anim sysstem for smoothing
		smoothanimupdate(t.charanimstate.obj);

	}

return;

}

void darkai_ischaracterhit ( void )
{
	//  takes; px#,py#,pz#,tobj
	t.darkaifirerayhitcharacter=0;
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		if (  t.entityelement[t.charanimstates[g.charanimindex].e].health>0 ) 
		{
			if (  t.tobj == t.charanimstates[g.charanimindex].obj ) 
			{
				//  if melee attack on character, half force for better organic response
				if (  t.gun[t.gunid].settings.ismelee == 2  )  t.tforce_f = t.tforce_f/2.0;
				//  damage this character
				t.twhox_f=t.px_f ; t.twhoy_f=t.py_f ; t.twhoz_f=t.pz_f;
				darkai_shootcharacter ( );
				t.darkaifirerayhitcharacter=1;
			}
		}
	}
}

void smoothanimtriggerrev ( int obj, float  st, float  fn, int  speedoftransition, int  rev, int  playflag )
{
	//  transition to the start of the loop frame
	if (  t.smoothanim[obj].st != st ) 
	{
		StopObject (  obj );
		SetObjectInterpolation (  obj,100.0/speedoftransition );
		if (  rev == 1 ) 
		{
			SetObjectFrame (  obj,fn );
		}
		else
		{
			SetObjectFrame (  obj,st );
		}
		t.smoothanim[obj].st=st;
		t.smoothanim[obj].fn=fn;
		t.smoothanim[obj].rev=rev;
		t.smoothanim[obj].transition=speedoftransition;
		t.smoothanim[obj].playflag=playflag;
		t.smoothanim[obj].playstarted=0;
	}
//endfunction

}

void smoothanimtrigger ( int obj, float  st, float  fn, int  speedoftransition )
{
	smoothanimtriggerrev(obj, st, fn, speedoftransition,0,0);
//endfunction

}

void smoothanimupdate ( int obj )
{
	//  when reach loop frame, switch to loop
//  `set cursor 0,100

//  `if obj>0

	//if ObjectExist(obj)=1
	// Print (  "obj=";obj;"  frame=";object frame(obj);"  trans=";smoothanim(obj).transition )
	//endif
//  `endif

	if (  t.smoothanim[obj].transition>0 ) 
	{
		t.smoothanim[obj].transition=t.smoothanim[obj].transition-1;
		if (  t.smoothanim[obj].transition == 0 ) 
		{
			SetObjectInterpolation (  obj,100.0 );
			if (  t.smoothanim[obj].playflag == 1 ) 
			{
				if (  t.smoothanim[obj].playstarted == 0 ) 
				{
					PlayObject (  obj,t.smoothanim[obj].st,t.smoothanim[obj].fn );
					if (  t.smoothanim[obj].rev == 0 ) 
					{
						SetObjectSpeed (  obj,abs(GetSpeed(obj)) );
					}
					else
					{
						SetObjectSpeed (  obj,abs(GetSpeed(obj))*-1 );
						SetObjectFrame (  obj,t.smoothanim[obj].fn );
					}
					t.smoothanim[obj].playstarted=1;
				}
			}
			else
			{
				LoopObject (  obj,t.smoothanim[obj].st,t.smoothanim[obj].fn );
				if (  t.smoothanim[obj].rev == 0 ) 
				{
					SetObjectSpeed (  obj,abs(GetSpeed(obj)) );
				}
				else
				{
					SetObjectSpeed (  obj,abs(GetSpeed(obj))*-1 );
				}
			}
		}
	}
//endfunction

}

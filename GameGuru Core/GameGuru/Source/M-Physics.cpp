//----------------------------------------------------
//--- GAMEGURU - M-Physics
//----------------------------------------------------

#include "gameguru.h"

// 
//  Physics Subroutines and Functions
// 

// 
//  PHYSICS CODE
// 

void physics_inittweakables ( void )
{
	//  Editable in Player Start Marker
	t.playercontrol.regenrate=0;
	t.playercontrol.regenspeed=100;
	t.playercontrol.regendelay=3000;
	t.playercontrol.regentime=0;
	t.playercontrol.jumpmax_f=215.0;
	t.playercontrol.gravity_f=900.0;
	t.playercontrol.fallspeed_f=5000.0;
	t.playercontrol.climbangle_f=70.0;
	t.playercontrol.footfallpace_f=3.0;
	t.playercontrol.wobblespeed_f=460.0;
	t.playercontrol.wobbleheight_f=1.5;
	t.playercontrol.accel_f=25.0;

	//  third person defaults
	t.playercontrol.thirdperson.enabled=0;
	t.playercontrol.thirdperson.charactere=0;
	t.playercontrol.thirdperson.startmarkere=0;
	t.playercontrol.thirdperson.cameralocked=0;
	t.playercontrol.thirdperson.cameradistance=200.0;
	t.playercontrol.thirdperson.cameraheight=100.0;
	t.playercontrol.thirdperson.camerafocus=5.0;
	t.playercontrol.thirdperson.cameraspeed=50.0;
	t.playercontrol.thirdperson.camerashoulder=6.0;
	t.playercontrol.thirdperson.camerafollow=1;
	t.playercontrol.thirdperson.camerareticle=1;
}

void physics_init ( void )
{
	//  Player Control
	t.playercontrol.wobble_f=0.0;
	t.playercontrol.floory_f=0.0;
	t.playercontrol.topspeed_f=0.75;
	t.playercontrol.footfalltype=0;
	t.playercontrol.footfallcount=0;
	if (  t.playercontrol.regenrate>0 ) 
	{
		t.playercontrol.regentime=Timer();
		t.playercontrol.regentick=Timer();
	}
	else
	{
		t.playercontrol.regentime=0;
		t.playercontrol.regentick=0;
	}

	//  Reset jetpack
	t.playercontrol.jetpackmode=0;
	t.playercontrol.jetpackthrust_f=0.0;
	t.playercontrol.jetpackfuel_f=0;

	//  Player misc settings
	t.playercontrol.disablemusicreset=0;

	//  Reset gun collected count
	t.guncollectedcount=0;

	//  Init physics system
	ODEStart (   ); g.gphysicssessionactive=1;

	//  Set starting water Line (  )
	terrain_updatewaterphysics ( );

	//  Create terrain collision
	t.tgenerateterraindirtyregiononly=0;
	timestampactivity(0,"_physics_createterraincollision");
	physics_createterraincollision ( );
	t.tgenerateterraindirtyregiononly=0;

	//  Player Controller Object
	if (  ObjectExist(t.aisystem.objectstartindex) == 0 ) 
	{
		//  normally created by AI which precedes Physics initialisations
		MakeObjectCube (  t.aisystem.objectstartindex,10 );
	}
	HideObject (  t.aisystem.objectstartindex );

	// moved player physics setup closer to main loop
	//t.freezeplayerposonly = 0;
	//physics_setupplayer ( );

	//  set default player gravity
	t.playercontrol.gravityactive=1;
	t.playercontrol.gravityactivepress=0;
	t.playercontrol.lockatheight=0;
	t.playercontrol.lockatheightpress=0;

	//  Setup entity physics
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		physics_prepareentityforphysics ( );
	}

	//  Introduce all characters and entities to the physics universe
	timestampactivity(0,"Introduce all characters");
	for ( g.charanimindex = 1 ; g.charanimindex<=  g.charanimindexmax; g.charanimindex++ )
	{
		// get physics object for this character
		t.tphyobj=t.charanimstates[g.charanimindex].obj;

		// 111115 - but exclude any third person character
		if ( t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.characterindex == g.charanimindex ) 
			t.tphyobj = 0;

		// if object still requires Y adjustment check
		if (  t.tphyobj>0 ) 
		{
			// get entity index associated with character
			t.e=t.charanimstates[g.charanimindex].e;
			physics_setupcharacter ( );
			t.entityelement[t.e].usingphysicsnow=1;
		}
	}

	// Ensure the LUA mouse is always reset
	lua_deactivatemouse();

	// player physics setup closer to main loop
	t.freezeplayerposonly = 0;
	physics_setupplayer ( );
}

void physics_finalize ( void )
{
	ODEFinalizeWorld();
}

void physics_createterraincollision ( void )
{
	// takes tgenerateterraindirtyregiononly (0-full/1-only refresh terrain.dirtyxy)
	// Which terrain collision style
	t.tphysicsterrainobjstart=t.terrain.objectstartindex+1000;
	if ( t.terrain.TerrainID>0 ) 
	{
		//  new physics command which creates the perfect 12+12 collision geometry
		timestampactivity(0,"get terrian sectors");
		t.TerrainID=t.terrain.TerrainID;
		t.terrain.TerrainLODOBJStart=t.tphysicsterrainobjstart;
		for ( t.i = 0 ; t.i <= (int)BT_GetSectorCount(t.TerrainID,0)-1; t.i++ )
		{
			if (  BT_GetSectorExcluded(t.TerrainID,0,t.i) == 0 ) 
			{
				t.sopx_f=BT_GetSectorPositionX(t.TerrainID,0,t.i);
				t.sopy_f=BT_GetSectorPositionY(t.TerrainID,0,t.i);
				t.sopz_f=BT_GetSectorPositionZ(t.TerrainID,0,t.i);
				t.tokay=0;
				if (  t.tgenerateterraindirtyregiononly == 0  )  t.tokay = 1;
				if (  t.tgenerateterraindirtyregiononly == 1 ) 
				{
					//  only re-create if within field of dirty area
					t.tsopradx=abs(t.terrain.dirtyx2-t.terrain.dirtyx1)*2;
					t.tsopradz=abs(t.terrain.dirtyz2-t.terrain.dirtyz1)*2;
					if (  t.sopx_f >= ((t.terrain.dirtyx1-t.tsopradx)*50)-1500 && t.sopx_f <= ((t.terrain.dirtyx2+t.tsopradx)*50)+1500 ) 
					{
						if (  t.sopz_f >= ((t.terrain.dirtyz1-t.tsopradz)*50)-1500 && t.sopz_f <= ((t.terrain.dirtyz2+t.tsopradz)*50)+1500 ) 
						{
							t.tokay=1;
						}
					}
				}
				if (  t.tokay == 1 ) 
				{
					if (  ObjectExist(t.tphysicsterrainobjstart+t.i) == 1  )  DeleteObject (  t.tphysicsterrainobjstart+t.i );
					BT_MakeSectorObject (  t.TerrainID,0,t.i,t.tphysicsterrainobjstart+t.i );
					PositionObject (  t.tphysicsterrainobjstart+t.i,t.sopx_f,t.sopy_f,t.sopz_f );
					SetObjectArbitaryValue (  t.tphysicsterrainobjstart+t.i,0 );
					BT_MakeSectorObject (  t.TerrainID,0,t.i,0 );
					ExcludeOn (  t.tphysicsterrainobjstart+t.i );
					SetObjectMask (  t.tphysicsterrainobjstart+t.i,0 );
					CloneMeshToNewFormat (  t.tphysicsterrainobjstart+t.i,0x002 );
					if (  t.tgenerateterraindirtyregiononly == 1  )  ODEDestroyObject (  t.tphysicsterrainobjstart+t.i );
					t.tterrainsegmentuntouched=1;
					for ( t.tscanzz = -1600 ; t.tscanzz <= 1600 ; t.tscanzz+= 100 )
					{
						for ( t.tscanxx = -1600 ; t.tscanxx <= 1600 ; t.tscanxx+= 100 )
						{
							t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,t.sopx_f+t.tscanxx,t.sopz_f+t.tscanzz);
							if (  t.h_f != 600.0 && t.h_f != -600 ) 
							{
								t.tterrainsegmentuntouched=0;
							}
						}
					}
					if (  t.tterrainsegmentuntouched == 1 ) 
					{
						//  if NO terrain unulations, can use quicker Box (  shape )
						DeleteObject (  t.tphysicsterrainobjstart+t.i );
						MakeObjectPlane (  t.tphysicsterrainobjstart+t.i, 3200, 3200 );
						PositionObject (  t.tphysicsterrainobjstart+t.i,t.sopx_f,600.0,t.sopz_f );
						XRotateObject ( t.tphysicsterrainobjstart+t.i, 90 );
						SetObjectArbitaryValue (  t.tphysicsterrainobjstart+t.i,0 );
						ExcludeOn (  t.tphysicsterrainobjstart+t.i );
						SetObjectMask (  t.tphysicsterrainobjstart+t.i,0 );
						CloneMeshToNewFormat (  t.tphysicsterrainobjstart+t.i,0x002 );
						ODECreateStaticTerrainMesh (  t.tphysicsterrainobjstart+t.i );
					}
					else
					{
						ODECreateStaticTerrainMesh (  t.tphysicsterrainobjstart+t.i );
					}
					DeleteObject (  t.tphysicsterrainobjstart+t.i );
				}
			}
		}
		t.terrain.TerrainLODOBJFinish=(t.tphysicsterrainobjstart+t.i)-1;
		timestampactivity(0,"get terrian sectors complete");

		/*       THIS TECHNIQUE EATS 338MB OF SYSTEM
		//  Setup terrain collision - trimesh
		t.TerrainID=t.terrain.TerrainID;
		t.LODLevel=0;
		t.terrain.TerrainLODOBJStart=t.tphysicsterrainobjstart;
		timestampactivity(0,"Setup t.terrain collision");
		for ( t.i = 0 ; t.i<=  BT_GetSectorCount(t.TerrainID,t.LODLevel)-1; t.i++ )
		{
			if (  BT_GetSectorExcluded(t.TerrainID,t.LODLevel,t.i) == 0 ) 
			{
				t.sopx_f=BT_GetSectorPositionX(t.TerrainID,t.LODLevel,t.i);
				t.sopy_f=BT_GetSectorPositionY(t.TerrainID,t.LODLevel,t.i);
				t.sopz_f=BT_GetSectorPositionZ(t.TerrainID,t.LODLevel,t.i);
				t.tokay=0;
				if (  t.tgenerateterraindirtyregiononly == 0  )  t.tokay = 1;
				if (  t.tgenerateterraindirtyregiononly == 1 ) 
				{
					//  only re-create if within field of dirty area
					t.tsopradx=abs(t.terrain.dirtyx2-t.terrain.dirtyx1)*2;
					t.tsopradz=abs(t.terrain.dirtyz2-t.terrain.dirtyz1)*2;
					if (  t.sopx_f >= (t.terrain.dirtyx1-t.tsopradx)*50 && t.sopx_f <= (t.terrain.dirtyx2+t.tsopradx)*50 ) 
					{
						if (  t.sopz_f >= (t.terrain.dirtyz1-t.tsopradz)*50 && t.sopz_f <= (t.terrain.dirtyz2+t.tsopradz)*50 ) 
						{
							t.tokay=1;
						}
					}
				}
				if (  t.tgenerateterraindirtyregiononly == 1 ) 
				{
					//  if F9 live editing, need the original terrain LOD objects
					if (  ObjectExist(t.tphysicsterrainobjstart+t.i) == 0  )  t.tokay = 1;
				}
				if (  t.tokay == 1 ) 
				{
					if (  ObjectExist(t.tphysicsterrainobjstart+t.i) == 1  )  DeleteObject (  t.tphysicsterrainobjstart+t.i );
					BT_MakeSectorObject (  t.TerrainID,t.LODLevel,t.i,t.tphysicsterrainobjstart+t.i );
					PositionObject (  t.tphysicsterrainobjstart+t.i,t.sopx_f,t.sopy_f,t.sopz_f );
					//  FUTURE ; grass/path/etc - per polygon material ids?
					SetObjectArbitaryValue (  t.tphysicsterrainobjstart+t.i,0 );
					//  to ensure terrain changes do not write into non existent object
					BT_MakeSectorObject (  t.TerrainID,t.LODLevel,t.i,0 );
					//  not needed for now, might need to retain briefly for occluder submission (pre-step)
					ExcludeOn (  t.tphysicsterrainobjstart+t.i );
					SetObjectMask (  t.tphysicsterrainobjstart+t.i,0 );
					CloneMeshToNewFormat (  t.tphysicsterrainobjstart+t.i,0x002 );
					ShowObject (  t.tphysicsterrainobjstart+t.i );
				}
			}
		}
		t.terrain.TerrainLODOBJFinish=(t.tphysicsterrainobjstart+t.i)-1;

		//  If refreshing an existing terrain physics system
		timestampactivity(0,"Create terrainphysicsmasterlist");
		if (  t.tgenerateterraindirtyregiononly == 0 ) 
		{
			//  prepare array to record which consolidated ojects relate to original objects
			tmasterindexmax=t.terrain.TerrainLODOBJFinish-t.terrain.TerrainLODOBJStart;
			Dim (  terrainphysicsmasterlist(tmasterindexmax) );
			Dim (  terrainphysicsmasterlistroot(tmasterindexmax) );
			Dim (  terrainphysicsmasterlistlimb(tmasterindexmax) );
			for ( tmasterindex = 0 ; tmasterindex<=  tmasterindexmax; tmasterindex++ )
			{
				terrainphysicsmasterlist(tmasterindex)=0;
				terrainphysicsmasterlistroot(tmasterindex)=0;
				terrainphysicsmasterlistlimb(tmasterindex)=0;
			next;
		}
		else
		{
			//  find and delete the old consolidated objects
			for ( tphyterrobj = t.terrain.TerrainLODOBJStart ; tphyterrobj<=  t.terrain.TerrainLODOBJFinish; tphyterrobj++ )
			{
				if (  ObjectExist(tphyterrobj) == 1 ) 
				{
					if (  GetVisible(tphyterrobj) == 1 ) 
					{
						//  this object has been recreated
						tmasterindex=tphyterrobj-t.terrain.TerrainLODOBJStart;
						tconsolidatedobj=terrainphysicsmasterlist(tmasterindex);
						if (  tconsolidatedobj>0 ) 
						{
							//  delete the batched up consolidated representitive of above obj
							ODEDestroyObject (  (tconsolidatedobj+1) );
							//  and now erase reference by reversing master ref (only need to delete it once)
							for ( tscanobj = t.terrain.TerrainLODOBJStart ; tscanobj<=  t.terrain.TerrainLODOBJFinish; tscanobj++ )
							{
								tmasterindex=tscanobj-t.terrain.TerrainLODOBJStart;
								if (  terrainphysicsmasterlist(tmasterindex) == tconsolidatedobj ) 
								{
									terrainphysicsmasterlist(tmasterindex)=tconsolidatedobj*-1;
								}
							}
						}
					}
				}
			}
		}

		//  Now we have a LOT of terrain meshes for physics, we can batch them to produce
		//  fewer physics objects to increase the physics performance
		timestampactivity(0,"Consolidate t.terrain meshes");
		tphynewmasterobjindex=t.terrain.TerrainLODOBJFinish;
		for ( tphyterrobj = t.terrain.TerrainLODOBJStart ; tphyterrobj<=  t.terrain.TerrainLODOBJFinish; tphyterrobj++ )
		{
			if (  ObjectExist(tphyterrobj) == 1 ) 
			{
				if (  GetVisible(tphyterrobj) == 1 ) 
				{
					if (  t.tgenerateterraindirtyregiononly == 0 ) 
					{
						//  new consolidated batch
						tphyterrobjmaster=tphyterrobj;
						tphyterrobjmasterx_f=ObjectPositionX(tphyterrobjmaster);
						tphyterrobjmastery_f=ObjectPositionY(tphyterrobjmaster);
						tphyterrobjmasterz_f=ObjectPositionZ(tphyterrobjmaster);
						inc tphynewmasterobjindex;
						if (  ObjectExist(tphynewmasterobjindex) == 1  )  DeleteObject (  tphynewmasterobjindex );
						CloneObject (  tphynewmasterobjindex,tphyterrobjmaster );
						for ( tgroup = 1 ; tgroup<=  15; tgroup++ )
						{
							tbestdd_f=9999999 ; tbestphyterrobj=0;
							for ( tphyterrobj2 = t.terrain.TerrainLODOBJStart ; tphyterrobj2<=  t.terrain.TerrainLODOBJFinish; tphyterrobj2++ )
							{
								if (  tphyterrobj2 != tphyterrobjmaster && ObjectExist(tphyterrobj2) == 1 ) 
								{
									if (  GetVisible(tphyterrobj2) == 1 ) 
									{
										t.tdx_f=ObjectPositionX(tphyterrobjmaster)-ObjectPositionX(tphyterrobj2);
										t.tdz_f=ObjectPositionZ(tphyterrobjmaster)-ObjectPositionZ(tphyterrobj2);
										t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
										if (  t.tdd_f<tbestdd_f ) 
										{
											tbestdd_f=t.tdd_f ; tbestphyterrobj=tphyterrobj2;
										}
									}
								}
							}
							if (  tbestphyterrobj>0 ) 
							{
								t.tdiffx_f=ObjectPositionX(tbestphyterrobj)-ObjectPositionX(tphyterrobjmaster);
								t.tdiffy_f=ObjectPositionY(tbestphyterrobj)-ObjectPositionY(tphyterrobjmaster);
								t.tdiffz_f=ObjectPositionZ(tbestphyterrobj)-ObjectPositionZ(tphyterrobjmaster);
								PositionObject (  tbestphyterrobj,0,0,0 );
								if (  GetMeshExist(g.meshgeneralwork) == 1  )  DeleteMesh (  g.meshgeneralwork );
								MakeMeshFromObject (  g.meshgeneralwork,tbestphyterrobj );
								HideObject (  tbestphyterrobj );
//         `delete object tbestphyterrobj

								if (  t.tgenerateterraindirtyregiononly == 0 ) 
								{
									tmasterindex=tbestphyterrobj-t.terrain.TerrainLODOBJStart;
									terrainphysicsmasterlist(tmasterindex)=tphynewmasterobjindex;
									terrainphysicsmasterlistroot(tmasterindex)=tphyterrobjmaster;
									terrainphysicsmasterlistlimb(tmasterindex)=tgroup;
								}
								AddLimb (  tphynewmasterobjindex,tgroup,g.meshgeneralwork );
								OffsetLimb (  tphynewmasterobjindex,tgroup,t.tdiffx_f,t.tdiffy_f,t.tdiffz_f );
							}
						}
						HideObject (  tphyterrobjmaster );
//       `delete object tphyterrobjmaster

						if (  t.tgenerateterraindirtyregiononly == 0 ) 
						{
							tmasterindex=tphyterrobjmaster-t.terrain.TerrainLODOBJStart;
							terrainphysicsmasterlist(tmasterindex)=tphynewmasterobjindex;
							terrainphysicsmasterlistroot(tmasterindex)=tphyterrobjmaster;
							terrainphysicsmasterlistlimb(tmasterindex)=0;
						}
					}
					else
					{
						//  recreate an existing consolidated batch (original collision obj changed)
						tmasterindex=tphyterrobj-t.terrain.TerrainLODOBJStart;
						tphyterrobjmaster=terrainphysicsmasterlistroot(tmasterindex);
						tphyterrobjmasterx_f=ObjectPositionX(tphyterrobjmaster);
						tphyterrobjmastery_f=ObjectPositionY(tphyterrobjmaster);
						tphyterrobjmasterz_f=ObjectPositionZ(tphyterrobjmaster);
						//  add all that belong to old consolidation to new master object
						for ( tscanobj = t.terrain.TerrainLODOBJStart ; tscanobj<=  t.terrain.TerrainLODOBJFinish; tscanobj++ )
						{
							tmasterindex=tscanobj-t.terrain.TerrainLODOBJStart;
							if (  terrainphysicsmasterlistroot(tmasterindex) == tphyterrobjmaster ) 
							{
								if (  ObjectExist(tscanobj) == 1 ) 
								{
									if (  GetVisible(tscanobj) == 1 ) 
									{
										PositionObject (  tscanobj,0,0,0 );
										if (  GetMeshExist(g.meshgeneralwork) == 1  )  DeleteMesh (  g.meshgeneralwork );
										MakeMeshFromObject (  g.meshgeneralwork,tscanobj );
										tphynewmasterobjindex=terrainphysicsmasterlist(tmasterindex);
										if (  tphynewmasterobjindex<0  )  tphynewmasterobjindex = tphynewmasterobjindex*-1;
										terrainphysicsmasterlist(tmasterindex)=tphynewmasterobjindex;
										tgroup=terrainphysicsmasterlistlimb(tmasterindex);
										change mesh tphynewmasterobjindex,tgroup,g.meshgeneralwork;
										HideObject (  tscanobj );
									}
								}
							}
						}
					}
					ExcludeOn (  tphynewmasterobjindex );
					SetObjectMask (  tphynewmasterobjindex,0 );
					if (  GetMeshExist(g.meshgeneralwork) == 1  )  DeleteMesh (  g.meshgeneralwork );
					MakeMeshFromObject (  g.meshgeneralwork,tphynewmasterobjindex );
					//  glue all meshes together and submit to physics engine
					inc tphynewmasterobjindex;
					if (  ObjectExist(tphynewmasterobjindex) == 1  )  DeleteObject (  tphynewmasterobjindex );
					MakeObject (  tphynewmasterobjindex,g.meshgeneralwork,0 );
					PositionObject (  tphynewmasterobjindex,tphyterrobjmasterx_f,tphyterrobjmastery_f,tphyterrobjmasterz_f );
					ODECreateStaticTerrainMesh (  tphynewmasterobjindex );
					DeleteObject (  tphynewmasterobjindex );
				}
			}
		}
		t.terrain.TerrainLODOBJStart=0;
		t.terrain.TerrainLODOBJFinish=-1;
		*/    

	}
	else
	{
		//  super flat terrain (one quad)
		if (  ObjectExist(t.tphysicsterrainobjstart) == 0 ) 
		{
			MakeObjectBox (  t.tphysicsterrainobjstart,512*100,100,512*100 );
			HideObject (  t.tphysicsterrainobjstart );
			PositionObject (  t.tphysicsterrainobjstart,256*100,1000-50,256*100 );
			SetObjectArbitaryValue (  t.tphysicsterrainobjstart,0 );
		}
		ODECreateStaticBox (  t.tphysicsterrainobjstart );
	}

return;

}

void physics_prepareentityforphysics ( void )
{
	//  takes E and ENTID
	t.tphyobj=t.entityelement[t.e].obj;
	if (  t.entid>0 && t.tphyobj>0 ) 
	{
		t.tnophysics=0;
		if (  t.entityprofile[t.entid].ismarker != 0  )  t.tnophysics = 1;
		if (  t.entityprofile[t.entid].collisionmode == 11  )  t.tnophysics = 1;
		if (  t.entityprofile[t.entid].collisionmode == 12  )  t.tnophysics = 1;
		if (  t.entityelement[t.e].eleprof.physics == 0  )  t.tnophysics = 1;
		if (  t.entityelement[t.e].eleprof.physics == 2  )  t.tnophysics = 1;
		if (  t.entityprofile[t.entid].isammo == 1  )  t.tnophysics = 1;
		if (  Len(t.entityprofile[t.entid].isweapon_s.Get())>1  )  t.tnophysics = 1;
		if (  t.tnophysics == 1 ) 
		{
			//  no physics
		}
		else
		{
			if (  t.entityprofile[t.entid].isebe != 0 ) 
			{
				// EBE structure from cubes
				physics_setupebestructure ( );
				t.entityelement[t.e].usingphysicsnow=1;
			}
			else
			{
				if (  t.entityprofile[t.entid].ischaracter == 1 ) 
				{
					//  physics objects belong to Ghost AI Objects (set outside of this function, i.e. LUA-Entity.cpp)
				}
				else
				{
					//  static or dynamic
					t.tstatic=t.entityelement[t.e].staticflag;
					if (  t.entityelement[t.e].eleprof.isimmobile == 1  )  t.tstatic = 1;
					//  physics object is faux-character capsule (zombies, custom characters)
					if (  t.entityprofile[t.entid].collisionmode == 21 ) 
					{
						//  create capsule (to be controlled as entity-driven, i.e. MoveForward)
						physics_setupcharacter ( );
						t.entityelement[t.e].usingphysicsnow=1;
					}
					else
					{
						//  create solid entities
						if (t.entityprofile[t.entid].collisionmode == 1)
						{
							t.tshape = 2;
						}
						else if (t.entityprofile[t.entid].collisionmode == 9)
						{
							t.tshape = 9;
						}
						else if (t.entityprofile[t.entid].collisionmode == 2)
						{
							t.tshape = 6;
						}
						else if (t.entityprofile[t.entid].collisionmode == 3)
						{
							t.tshape = 7;
						}
						else
						{
							if (  t.entityprofile[t.entid].collisionmode >= 1000 ) 
							{
								t.tshape=t.entityprofile[t.entid].collisionmode;
							}
							else
							{
								if (  t.entityprofile[t.entid].collisionmode >= 50 && t.entityprofile[t.entid].collisionmode<60 ) 
								{
									t.tshape=3;
									t.tstatic=1;
								}
								else
								{
									t.tshape=1;
								}
							}
						}
						//  check if it has a list of physics objects from the importer, collisionmode 40
						if (  t.entityprofile[t.entid].physicsobjectcount > 0 && t.entityprofile[t.entid].collisionmode  ==  40 ) 
						{
							t.tshape = 4;
						}
						t.tweight=t.entityelement[t.e].eleprof.phyweight;
						t.tfriction=t.entityelement[t.e].eleprof.phyfriction;
						t.tcollisionscaling=t.entityprofile[t.entid].collisionscaling;
						physics_setupobject ( );
						t.entityelement[t.e].usingphysicsnow=1;
					}
				}
			}
		}
	}
}

void physics_setupplayernoreset ( void )
{
	//  create character controller for player
	PositionObject (  t.aisystem.objectstartindex,t.terrain.playerx_f,t.terrain.playery_f,t.terrain.playerz_f );
	if ( t.freezeplayerposonly==0 ) RotateObject (  t.aisystem.objectstartindex,t.terrain.playerax_f,t.terrain.playeray_f,t.terrain.playeraz_f );
	SetObjectArbitaryValue (  t.aisystem.objectstartindex,6 );
	ODECreateDynamicCharacterController (  t.aisystem.objectstartindex,t.playercontrol.gravity_f,t.playercontrol.fallspeed_f,t.playercontrol.climbangle_f );
}

void physics_setupplayer ( void )
{
	physics_setupplayernoreset ( );
	if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
	{
		if ( t.freezeplayerposonly==0 ) RotateCamera (  0,t.terrain.playerax_f,t.terrain.playeray_f,t.terrain.playeraz_f );
	}
}

void physics_disableplayer ( void )
{
	ODEDestroyObject (  t.aisystem.objectstartindex );
}

void physics_setupcharacter ( void )
{
	// create physics for this character/faux-character object
	SetObjectArbitaryValue (  t.tphyobj,6 );
	if ( t.entityelement[t.e].eleprof.isimmobile == 0 ) 
	{
		// 190718 - remove t.terrain.adjaboveground_f from enemy terrain relative positioning
		// ensure CHARACTER do not spawn UNDER the terrain
		if (  t.terrain.TerrainID>0 ) 
		{
			//t.tgroundheight_f=BT_GetGroundHeight(t.terrain.TerrainID,ObjectPositionX(t.tphyobj),ObjectPositionZ(t.tphyobj))+t.terrain.adjaboveground_f;
			t.tgroundheight_f=BT_GetGroundHeight(t.terrain.TerrainID,ObjectPositionX(t.tphyobj),ObjectPositionZ(t.tphyobj));
		}
		else
		{
			//t.tgroundheight_f=1000.0+t.terrain.adjaboveground_f;
			t.tgroundheight_f=1000.0;
		}
		t.tgroundheight_f=t.tgroundheight_f+2.5;
		// 291116 - account for object vecCenter (so characters with Y=0=Floor are not unjustly raised)
		// NOW DONE EARLIER SO NO NEED TO ADJUST FOR COL CENTER TWICE (see calling function)
		//float fAccountForVecCenter = GetObjectCollisionCenterY ( t.tphyobj );
		if ( ObjectPositionY(t.tphyobj) <= t.tgroundheight_f ) 
		{
			PositionObject ( t.tphyobj,ObjectPositionX(t.tphyobj), t.tgroundheight_f, ObjectPositionZ(t.tphyobj) );
		}
		else
		{
			PositionObject ( t.tphyobj,ObjectPositionX(t.tphyobj), ObjectPositionY(t.tphyobj), ObjectPositionZ(t.tphyobj) );
		}
		t.tfinalscale_f = g.gcharactercapsulescale_f * ((t.entityprofile[t.entityelement[t.e].bankindex].scale+0.0f)/100.0f);
		float fWeight = t.entityelement[t.e].eleprof.phyweight;
		float fFriction = t.entityelement[t.e].eleprof.phyfriction;
		ODECreateDynamicCapsule ( t.tphyobj, t.tfinalscale_f, 0.0, fWeight, fFriction, -1 );
	}
	else
	{
		// 290515 - fixes scifi DLC characters floating (ISIMMOBILE=1) PositionObject ( t.tphyobj,ObjectPositionX(t.tphyobj),ObjectPositionY(t.tphyobj)+(ObjectSizeY(t.tphyobj,1)/2),ObjectPositionZ(t.tphyobj) );
		PositionObject ( t.tphyobj,ObjectPositionX(t.tphyobj),ObjectPositionY(t.tphyobj),ObjectPositionZ(t.tphyobj) );
		ODECreateStaticCapsule ( t.tphyobj );
	}
}

void physics_setupebestructure ( void )
{
	//  create EBE physics for this object. Takes tphyobj and entid and e
	if ( t.tphyobj>0 ) 
	{
		if ( ObjectExist(t.tphyobj) == 1 ) 
		{
			ebe_physics_setupebestructure ( t.tphyobj, t.e );
		}
	}
}

void physics_setupobject ( void )
{
	// create physics for this object. Takes tphyobj and entid and e
	if ( t.tphyobj>0 ) 
	{
		if ( ObjectExist(t.tphyobj) == 1 ) 
		{
			SetObjectArbitaryValue (  t.tphyobj,t.entityprofile[t.entid].materialindex );
			if ( t.tstatic == 1 || t.game.runasmultiplayer == 1 ) 
			{
				// if static, need to ensure FIXNEWY pivot is respected
				if ( t.tstatic == 1 ) 
				{
					t.tstaticfixnewystore_f=ObjectAngleY(t.tphyobj);
					RotateObject (  t.tphyobj,ObjectAngleX(t.tphyobj),ObjectAngleY(t.tphyobj)+t.entityprofile[t.entid].fixnewy,ObjectAngleZ(t.tphyobj) );
				}
				//  create the physics now
				if ( t.tshape >= 1000 && t.tshape<2000 ) 
				{
					ODECreateStaticBox (  t.tphyobj,t.tshape-1000 );
				}
				else if ( t.tshape >= 2000 && t.tshape<3000 ) 
				{
					ODECreateStaticTriangleMesh (  t.tphyobj,t.tshape-2000 );
				}
				else if ( t.tshape == 1 ) 
				{
					ODECreateStaticBox ( t.tphyobj );
				}
				else if (t.tshape == 6)
				{
					ODECreateStaticSphere ( t.tphyobj );
				}
				else if (t.tshape == 7)
				{
					ODECreateStaticCylinder ( t.tphyobj );
				}
				else if ( t.tshape == 2 || t.tshape == 9 ) 
				{
					if (  t.tshape == 2 ) 
					{
						if (  t.tcollisionscaling != 100 ) 
						{
							ODECreateStaticTriangleMesh (  t.tphyobj,-1,t.tcollisionscaling );
						}
						else
						{
							ODECreateStaticTriangleMesh (  t.tphyobj );
						}
					}
					else
					{
						ODECreateStaticTriangleMesh (  t.tphyobj,-1,t.tcollisionscaling,1 );
					}
				}
				else if ( t.tshape == 3 ) 
				{
					physics_setuptreecylinder ( );
				}
				// tshape 4 is a list of physics objects from the importer
				else if ( t.tshape == 4 ) 
				{
					physics_setupimportershapes ( );
				}
				// if static, restore object before leaving
				if ( t.tstatic == 1 ) 
				{
					RotateObject (  t.tphyobj,ObjectAngleX(t.tphyobj),t.tstaticfixnewystore_f,ObjectAngleZ(t.tphyobj) );
				}
			}
			else
			{
				// objects will fall through Floor (  if they are perfectly sitting on it )
				PositionObject ( t.tphyobj, ObjectPositionX(t.tphyobj), ObjectPositionY(t.tphyobj)+0.1, ObjectPositionZ(t.tphyobj) );
				if ( t.tshape == 6 )     
				{
					// Sphere
					ODECreateDynamicSphere(t.tphyobj, t.tweight, t.tfriction, 0.01f);
				}
				else if ( t.tshape == 7 ) 
				{
					// Cylinder
					ODECreateDynamicCylinder(t.tphyobj, t.tweight, t.tfriction, 0.01f);
				}
				else                     
				{
					// box
					ODECreateDynamicBox(t.tphyobj, -1, 0, t.tweight, t.tfriction, -1);
				}
			}
		}
	}
}

int physics_findfreegamerealtimeobj ( void )
{
	int iFreeObj = g.gamerealtimeobjoffset;
	while ( ObjectExist ( iFreeObj ) == 1 && iFreeObj < g.gamerealtimeobjoffsetmax )
	{
		iFreeObj++;
	}
	if ( iFreeObj == g.gamerealtimeobjoffsetmax )
	{
		if ( ObjectExist ( iFreeObj ) == 1 ) DeleteObject ( iFreeObj );
	}
	return iFreeObj;
}

void physics_freeallgamerealtimeobjs ( void )
{
	for ( int iObj = g.gamerealtimeobjoffset; iObj <= g.gamerealtimeobjoffsetmax; iObj++ )
		if ( ObjectExist ( iObj ) == 1 ) 
			DeleteObject ( iObj );
}

void physics_setupimportershapes ( void )
{
	// flag to control if debug collision boxes should be left
	bool bLeaveDebugCollisionBoxes = false;
	if ( g.globals.showdebugcollisonboxes == 1 ) bLeaveDebugCollisionBoxes = true;

	// get collision boxes data from entity to make importer collision shapes
	if ( t.entid > MAX_ENTITY_PHYSICS_BOXES*2  ) 
	{
		Dim2 ( t.entityphysicsbox , t.entid , MAX_ENTITY_PHYSICS_BOXES   );
	}
	ODEStartStaticObject (  t.tphyobj );
	float fMoveToObjectWorldX = ObjectPositionX ( t.tphyobj );
	float fMoveToObjectWorldY = ObjectPositionY ( t.tphyobj );
	float fMoveToObjectWorldZ = ObjectPositionZ ( t.tphyobj );
	for ( t.tcount = 0 ; t.tcount <= t.entityprofile[t.entid].physicsobjectcount-1; t.tcount++ )
	{
		if (  ObjectExist(g.tempimporterlistobject)  )  DeleteObject (  g.tempimporterlistobject );
		int iObjectToUse = g.tempimporterlistobject;
		if ( bLeaveDebugCollisionBoxes == true ) iObjectToUse = physics_findfreegamerealtimeobj();
		t.tescale=t.entityprofile[t.entid].scale;
		if (  t.tescale>0 ) 
		{
			t.tnewscalex_f=t.tescale+t.entityelement[t.e].scalex;
			t.tnewscaley_f=t.tescale+t.entityelement[t.e].scaley;
			t.tnewscalez_f=t.tescale+t.entityelement[t.e].scalez;
		}
		else
		{
			t.tnewscalex_f=100+t.entityelement[t.e].scalex;
			t.tnewscaley_f=100+t.entityelement[t.e].scaley;
			t.tnewscalez_f=100+t.entityelement[t.e].scalez;
		}
		t.tnewsizex_f=(t.tnewscalex_f/100.0)*(t.entityphysicsbox[t.entid][t.tcount].SizeX);
		t.tnewsizey_f=(t.tnewscaley_f/100.0)*(t.entityphysicsbox[t.entid][t.tcount].SizeY);
		t.tnewsizez_f=(t.tnewscalez_f/100.0)*(t.entityphysicsbox[t.entid][t.tcount].SizeZ);
		MakeObjectBox ( iObjectToUse, t.tnewsizex_f, t.tnewsizey_f, t.tnewsizez_f );
		float tNewOffX = (t.tnewscalex_f/100.0) * t.entityphysicsbox[t.entid][t.tcount].OffX;
		float tNewOffY = (t.tnewscaley_f/100.0) * t.entityphysicsbox[t.entid][t.tcount].OffY;
		float tNewOffZ = (t.tnewscalez_f/100.0) * t.entityphysicsbox[t.entid][t.tcount].OffZ;
		t.tocy_f=ObjectSizeY(t.tphyobj,1)/2.0;
		PositionObject ( iObjectToUse, tNewOffX, t.tocy_f + tNewOffY, tNewOffZ );
		RotateObject ( iObjectToUse, t.entityphysicsbox[t.entid][t.tcount].RotX , t.entityphysicsbox[t.entid][t.tcount].RotY , t.entityphysicsbox[t.entid][t.tcount].RotZ );
		//SetObjectArbitaryValue ( iObjectToUse,t.entityprofile[t.entid].materialindex );
		ODEAddStaticObjectBox ( t.tphyobj, iObjectToUse, t.entityprofile[t.entid].materialindex );
		if ( bLeaveDebugCollisionBoxes == true ) 
		{
			FixObjectPivot ( iObjectToUse );
			RotateObject ( iObjectToUse, t.entityelement[t.e].rx, t.entityelement[t.e].ry, t.entityelement[t.e].rz );
			GGVECTOR3 vecOffset = GGVECTOR3 ( tNewOffX, t.tocy_f + tNewOffY, tNewOffZ );
			sObject* pObjectPtr = GetObjectData ( t.tphyobj );
			GGVec3TransformCoord ( &vecOffset, &vecOffset, &pObjectPtr->position.matRotation );
			PositionObject ( iObjectToUse, fMoveToObjectWorldX+vecOffset.x, fMoveToObjectWorldY+vecOffset.y, fMoveToObjectWorldZ+vecOffset.z );
		}
	}
	if ( ObjectExist(g.tempimporterlistobject)  )  DeleteObject (  g.tempimporterlistobject );
	ODEEndStaticObject (  t.tphyobj, 0 );
}

void physics_setuptreecylinder ( void )
{
	//  takes; tphyobj and entid and sets up a tree cylinder
	if (  t.tphyobj < 1  )  return;
	if (  ObjectExist(t.tphyobj)  ==  0  )  return;
	if (  t.entid < 1  )  return;

	//  Tree height (adjusted for scale)
	t.tSizeY_f = ObjectSizeY(t.tphyobj,1);

	//  if have ABS position from AI OBSTACLE calc, use that instead
	if (  t.entityelement[t.e].abscolx_f != -1 ) 
	{
		t.tFinalX_f = t.entityelement[t.e].abscolx_f;
		t.tFinalZ_f = t.entityelement[t.e].abscolz_f;
	}
	else
	{
		t.tFinalX_f = ObjectPositionX(t.tphyobj);
		t.tFinalZ_f = ObjectPositionZ(t.tphyobj);
	}
	t.tFinalY_f = ObjectPositionY(t.tphyobj) + (t.tSizeY_f/2.0);

	//  if have ABS radius from AI OBSTACLE calc, use that instead
	if (  t.entityelement[t.e].abscolradius_f != -1 ) 
	{
		t.tSizeX_f = t.entityelement[t.e].abscolradius_f;
		t.tSizeZ_f = t.entityelement[t.e].abscolradius_f;
	}
	else
	{
		t.tSizeX_f = 20;
		t.tSizeZ_f = 20;
	}

	//  increase size by 25%
	t.tSizeX_f=t.tSizeX_f*1.25;
	t.tSizeZ_f=t.tSizeZ_f*1.25;

	//  now create our physics object
	SetObjectArbitaryValue (  t.tphyobj,6 );
	ODECreateStaticCylinder (  t.tphyobj,t.tFinalX_f,t.tFinalY_f,t.tFinalZ_f,t.tSizeX_f,t.tSizeY_f,t.tSizeZ_f,0,0,0 );
}

void physics_disableobject ( void )
{
	ODEDestroyObject (  t.tphyobj );
}

void physics_beginsimulation ( void )
{
	//t.ptimer=PerformanceTimer() ; t.pfreq=PerformanceFrequency()/1000 ; t.ptimer=t.ptimer/t.pfreq;
	//t.machineindependentphysicsupdate=t.ptimer;
	t.machineindependentphysicsupdate = timeGetSecond();
}

void physics_pausephysics ( void )
{
	//t.pfreq1=PerformanceFrequency()/1000;
	//t.ptimer1=PerformanceTimer() ; t.ptimer1=t.ptimer1/t.pfreq1;
	t.ptimer1 = timeGetSecond();
}

void physics_resumephysics ( void )
{
	//t.ptimer2=PerformanceTimer() ; t.ptimer2=t.ptimer2/t.pfreq1;
	t.ptimer2 = timeGetSecond();
	t.machineindependentphysicsupdate=t.machineindependentphysicsupdate+(t.ptimer2-t.ptimer1);
}

void physics_loop ( void )
{
	//  Player control
	physics_player ( );

	//  Update physics system
	t.tphysicsadvance_f = timeGetSecond() - t.machineindependentphysicsupdate;
	if (  t.tphysicsadvance_f >= (1.0/120.0) ) 
	{
		//  only process physics once we reach the minimum substep constant
		if ( t.tphysicsadvance_f>0.05f ) t.tphysicsadvance_f = 0.05f;
		t.machineindependentphysicsupdate = timeGetSecond();
		ODEUpdate ( t.tphysicsadvance_f );
	}
}

void physics_free ( void )
{
	// remove any game realtime objects (used for debugging collision boxes, possible LUA spawned 3D objects, etc)
	physics_freeallgamerealtimeobjs();

	//  free terrain physics object
	if (  t.terrain.superflat == 0 ) 
	{
		for ( t.tobj = t.terrain.TerrainLODOBJStart ; t.tobj<=  t.terrain.TerrainLODOBJFinish; t.tobj++ )
		{
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				ODEDestroyObject (  t.tobj );
			}
		}
	}
	else
	{
		if (  ObjectExist(t.tphysicsterrainobjstart) == 1 ) 
		{
			ODEDestroyObject (  t.tphysicsterrainobjstart );
			DeleteObject (  t.tphysicsterrainobjstart );
		}
	}

	//  detatch entity from physics
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.obj=t.entityelement[t.e].obj;
		if (  t.obj>0 ) 
		{
			if (  ObjectExist(t.obj) == 1 ) 
			{
				if (  t.entityelement[t.e].usingphysicsnow == 1 ) 
				{
					t.tphyobj=t.obj  ; physics_disableobject ( );
					t.entityelement[t.e].usingphysicsnow=0;
				}
			}
		}
	}

	//  Clean-up physics system
	ODEEnd (   ); g.gphysicssessionactive=0;
}

void physics_explodesphere ( void )
{
	//  takes texplodex#,texplodey#,texplodez#,texploderadius#
	t.tstrengthofexplosion_f = t.tDamage_f;// 500.0; 200316 - now comes from explosion damage value (from barrel entity property or projectile)

	//  detect if player within radius and apply damage
	t.tdx_f=ObjectPositionX(t.aisystem.objectstartindex)-t.texplodex_f;
	t.tdy_f=ObjectPositionY(t.aisystem.objectstartindex)-t.texplodey_f;
	t.tdz_f=ObjectPositionZ(t.aisystem.objectstartindex)-t.texplodez_f;
	t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdy_f*t.tdy_f)+abs(t.tdz_f*t.tdz_f));
	if (  t.tdd_f<t.texploderadius_f ) 
	{
		//  apply camera shake for nearby explosion
		t.playercontrol.camerashake_f=(((t.texploderadius_f*2)-(t.tdd_f/(t.texploderadius_f*2)))*t.tstrengthofexplosion_f)/150.0/20.0;
		if (  t.playercontrol.camerashake_f > 25.0f  )  t.playercontrol.camerashake_f = 25.0f;
	}
	if (  t.tdd_f<t.texploderadius_f ) 
	{
		//  apply damage
		t.te=-1;
		//t.tdamage = ((t.texploderadius_f-(t.tdd_f/t.texploderadius_f))*t.tstrengthofexplosion_f)/150.0; 
		t.tdamage = (1.0f-(t.tdd_f/t.texploderadius_f)) * t.tstrengthofexplosion_f; // 200316 - correct fall off from explosion radius
		if ( t.tdamage > t.tstrengthofexplosion_f ) t.tdamage = t.tstrengthofexplosion_f;
		if (  t.game.runasmultiplayer  ==  1 ) 
		{
			t.tsteamwasnetworkdamage = 0;
			if (  t.entityelement[t.texplodesourceEntity].mp_networkkill  ==  1 ) 
			{
				//  13032015 0XX - Team Multiplayer
				if (  g.mp.team  ==  0 || g.mp.friendlyfireoff  ==  0 || t.mp_team[t.entityelement[t.texplodesourceEntity].mp_killedby]  !=  t.mp_team[g.mp.me] ) 
				{
					t.tsteamwasnetworkdamage = 1;
				}
			}
		}
		if (  t.game.runasmultiplayer  ==  1 ) 
		{
			//  13032015 0XX - Team Multiplayer
			//  Can't kill yourself if friendly fire is off
			if (  t.tsteamwasnetworkdamage  ==  1 || g.mp.friendlyfireoff  ==  0 || g.mp.damageWasFromAI  ==  1 ) 
			{
				physics_player_takedamage ( );
			}
		}
		else
		{
			physics_player_takedamage ( );
		}
		if (  t.game.runasmultiplayer  ==  1  )  t.tsteamwasnetworkdamage  =  0;

		//  apply force to push player
		t.playercontrol.pushangle_f = atan2deg(t.tdx_f,t.tdz_f);
		t.playercontrol.pushforce_f = (1.0f-(t.tdd_f/t.texploderadius_f))*10.0;
	}
	//  if the explosion was caused by another player, we let them handle it rather than us
	if (  t.game.runasmultiplayer  ==  1 ) 
	{
		if (  t.entityelement[t.texplodesourceEntity].mp_networkkill  ==  1 && g.mp.damageWasFromAI  ==  0 ) 
		{
			return;
		}
	}
	//  create a sphere of force at this location
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  g.mp.damageWasFromAI  ==  0 ) 
		{
			if (  t.texplodesourceEntity > 0 && t.e == t.texplodesourceEntity  )  t.entid = 0;
		}
		if (  t.entid > 0 && t.entityelement[t.e].obj > 0 ) 
		{
			// 220618 - use center of object, not coordinate of entity XYZ
			float fCenterOfEntityX = ObjectPositionX(t.entityelement[t.e].obj) + GetObjectCollisionCenterX(t.entityelement[t.e].obj);
			float fCenterOfEntityY = ObjectPositionY(t.entityelement[t.e].obj) + GetObjectCollisionCenterY(t.entityelement[t.e].obj);
			float fCenterOfEntityZ = ObjectPositionZ(t.entityelement[t.e].obj) + GetObjectCollisionCenterZ(t.entityelement[t.e].obj);
			t.tdx_f = fCenterOfEntityX - t.texplodex_f;
			t.tdy_f = fCenterOfEntityY - t.texplodey_f;
			t.tdz_f = fCenterOfEntityZ - t.texplodez_f;
			t.tdd_f = Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdy_f*t.tdy_f)+abs(t.tdz_f*t.tdz_f));

			if ( t.tdd_f<t.texploderadius_f ) 
			{
				// 220618 - before apply actual entity damage/effect, ensure a line of sight exists (could be behind wall/door)
				float fRayDestFromExplosionX = fCenterOfEntityX - t.texplodex_f;
				float fRayDestFromExplosionY = fCenterOfEntityY - t.texplodey_f;
				float fRayDestFromExplosionZ = fCenterOfEntityZ - t.texplodez_f;
				fRayDestFromExplosionX /= t.tdd_f;
				fRayDestFromExplosionY /= t.tdd_f;
				fRayDestFromExplosionZ /= t.tdd_f;
				fRayDestFromExplosionX *= t.texploderadius_f;
				fRayDestFromExplosionY *= t.texploderadius_f;
				fRayDestFromExplosionZ *= t.texploderadius_f;
				fRayDestFromExplosionX += t.texplodex_f;
				fRayDestFromExplosionY += t.texplodey_f;
				fRayDestFromExplosionZ += t.texplodez_f;

				t.ttt = IntersectAll( g.lightmappedobjectoffset, g.lightmappedobjectoffsetfinish, t.brayx1_f, t.brayy1_f, t.brayz1_f, 0, 0, 0, -123 );

				t.tintersectvalue = IntersectAll( g.entityviewstartobj, g.entityviewendobj, 
					                              t.texplodex_f, t.texplodey_f, t.texplodez_f, 
					                              fRayDestFromExplosionX, fRayDestFromExplosionY, fRayDestFromExplosionZ, 
					                              t.entityelement[ t.e ].obj );
				if ( t.tintersectvalue == 0 || t.tintersectvalue == t.entityelement[ t.texplodesourceEntity ].obj )
				{
					t.tdamage = ( t.texploderadius_f - t.tdd_f ) * t.tstrengthofexplosion_f;
					t.tdamageforce = ( t.texploderadius_f - t.tdd_f ) * t.tstrengthofexplosion_f;
					t.brayx1_f = t.texplodex_f;
					t.brayy1_f = t.texplodey_f;
					t.brayz1_f = t.texplodez_f;
					t.brayx2_f = fCenterOfEntityX;
					t.brayy2_f = fCenterOfEntityY;
					t.brayz2_f = fCenterOfEntityZ;
					t.braydx_f = t.brayx2_f-t.brayx1_f;
					t.braydz_f = t.brayz2_f-t.brayz1_f;
					t.braydist_f = Sqrt(abs(t.braydx_f*t.braydx_f)+abs(t.braydz_f*t.braydz_f));
					if ( t.braydist_f < 75 ) t.brayy2_f = t.texplodey_f + 100.0;
					if ( t.tdamageforce > 150 ) t.tdamageforce = 150;
					t.tdamagesource = 2;
					t.ttte = t.e ; entity_applydamage( ); t.e = t.ttte;
					//  inform darkAI of the explosion
					t.tsx_f = t.entityelement[t.e].x; 
					t.tsz_f = t.entityelement[t.e].z;
					darkai_makeexplosionsound ( );
				}
			}
		}
	}

	//  Reset flag for ai damage
	g.mp.damageWasFromAI = 0;
}

void physics_player_init ( void )
{
	// One Player In Single Player Game
	t.plrid=1;
	t.tnostartmarker=1;

	//  Initialise player settings
	if ( t.game.levelplrstatsetup == 1 )
	{
		// starting stats
		t.playercontrol.startlives=0;
		t.playercontrol.startstrength=100;
		t.playercontrol.startviolent=1;
		t.playercontrol.starthasweapon=0;
		t.playercontrol.starthasweaponqty=0;
		t.playercontrol.speedratio_f=1.0;
		t.playercontrol.hurtfall=100;
		t.playercontrol.canrun=1;
		t.player[t.plrid].lives=t.playercontrol.startlives;
		t.player[t.plrid].health=t.playercontrol.startstrength;
		t.player[t.plrid].powers.level=100;

		//  act on start marker
		t.playercontrol.hurtfall=0;
		t.playercontrol.speedratio_f=1.0;
	}
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entityprofile[t.entid].ismarker == 1 ) 
		{
			//  Player Start Marker Settings
			t.terrain.playerx_f=t.entityelement[t.e].x;
			t.terrain.playery_f=t.entityelement[t.e].y;
			t.terrain.playerz_f=t.entityelement[t.e].z;
			t.terrain.playerax_f=0;
			t.terrain.playeray_f=t.entityelement[t.e].ry;
			t.terrain.playeraz_f=0;
			t.playercontrol.finalcameraangley_f=t.terrain.playeray_f;

			//  Player Global Settings for this level
			if ( t.game.levelplrstatsetup == 1 )
			{
				t.playercontrol.startlives=t.entityelement[t.e].eleprof.lives;
				t.playercontrol.startstrength=t.entityelement[t.e].eleprof.strength;
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					t.tprotagoniste=t.playercontrol.thirdperson.charactere;
					t.playercontrol.starthasweapon=t.entityelement[t.tprotagoniste].eleprof.hasweapon;
				}
				else
				{
					t.playercontrol.starthasweapon=t.entityelement[t.e].eleprof.hasweapon;
				}
				t.playercontrol.starthasweaponqty=t.entityelement[t.e].eleprof.quantity;
				t.playercontrol.startviolent=t.entityelement[t.e].eleprof.isviolent;
				t.playercontrol.speedratio_f=t.entityelement[t.e].eleprof.speed/100.0;
				t.playercontrol.hurtfall=t.entityelement[t.e].eleprof.hurtfall;

				// 050416 - if in parental mode, ensure no weapon at start
				if ( g.quickparentalcontrolmode == 2 )
				{
					// only ban modern day weapons, not fireball
					int iPlrGunID = t.weaponindex=t.playercontrol.starthasweapon;
					if ( iPlrGunID > 0 )
					{
						if ( strnicmp ( t.gun[iPlrGunID].name_s.Get(), "modernday", 9 ) == NULL )
						{
							t.playercontrol.starthasweapon = 0;
							t.playercontrol.starthasweaponqty = 0;
							t.playercontrol.startviolent = 0;
						}
					}
				}

				//  Populate lives and health with default player
				t.player[t.plrid].lives=t.playercontrol.startlives;
				t.player[t.plrid].health=t.playercontrol.startstrength;

				//  Start Marker present
				t.tnostartmarker=0;
			}
			else
			{
				// level 2 and above do not control start stats or weaponry
			}
		}
	}

	//  If no player start marker, reset player physics tweakables
	if ( t.game.levelplrstatsetup == 1 )
	{
		if ( t.tnostartmarker == 1 ) physics_inittweakables ( );
	}

	//  Player start height (marker or no)
	if (  t.terrain.TerrainID>0 ) 
	{
		t.tbestterrainplayery_f=BT_GetGroundHeight(t.terrain.TerrainID,t.terrain.playerx_f,t.terrain.playerz_f)+t.terrain.adjaboveground_f;
	}
	else
	{
		t.tbestterrainplayery_f=1000.0+t.terrain.adjaboveground_f;
	}

	//  also ensure ABOVE water Line (  )
	if (  t.tbestterrainplayery_f<t.terrain.waterliney_f+20+t.terrain.adjaboveground_f ) 
	{
		t.tbestterrainplayery_f=t.terrain.waterliney_f+20+t.terrain.adjaboveground_f;
	}
	if (  t.terrain.playery_f == 0 ) 
	{
		t.terrain.playery_f=t.tbestterrainplayery_f;
	}
	else
	{
		t.terrain.playery_f=t.terrain.playery_f+t.terrain.adjaboveground_f;
		if (  t.terrain.playery_f<t.tbestterrainplayery_f  )  t.terrain.playery_f = t.tbestterrainplayery_f;
	}

	// Select weapon if start marker specifies it
	//for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
	//{
	//	t.weaponslot[t.ws].got=0;
	//	t.weaponslot[t.ws].pref=0;
	//}
	if ( t.game.levelplrstatsetup == 1 )
	{
		if (  t.playercontrol.starthasweapon>0 ) 
		{
			t.weaponindex=t.playercontrol.starthasweapon;
			t.tqty=t.playercontrol.starthasweaponqty;
			physics_player_addweapon ( );
		}
	}
	else
	{
		// if have weapon from previous level session, activate it
		if ( t.lastgunid > 0 ) 
		{
			g.autoloadgun = t.lastgunid;
			t.lastgunid = 0;
		}
	}

	// and only use this flag once per game
	t.game.levelplrstatsetup = 0;

	//  OpenFileMap (  for IDE access )
	if (  t.plrfilemapaccess == 0 && t.game.gameisexe == 0 ) 
	{
		OpenFileMap (  11, "FPSEXCHANGE" );
		SetEventAndWait (  11 );
		t.plrfilemapaccess=1;
	}
}

void physics_player_free ( void )
{

	//  //CloseFileMap (  at end of game )
	if (  t.plrfilemapaccess == 1 ) 
	{
		//CloseFileMap (  11 );
		t.plrfilemapaccess=0;
	}

return;

}

//Dave Performance
int physics_player_listener_delay = 0;
void physics_player ( void )
{
	if ( t.game.runasmultiplayer ==  0 || g.mp.noplayermovement == 0 ) 
	{
		if ( t.aisystem.processplayerlogic == 1 ) 
		{
			physics_player_gatherkeycontrols ( );
			physics_player_control ( );
			gun_update_hud ( );
			if ( ++physics_player_listener_delay > 3 )
			{
				physics_player_listener_delay = 0;
				physics_player_listener ( );
			}
		}
		else
		{
			// prevent player physics movement
			ODEControlDynamicCharacterController ( t.aisystem.objectstartindex, 0, 0, 0, 0, t.aisystem.playerducking, 0, 0, 0 );
		}
		physics_player_handledeath ( );
	}
}

void physics_player_gatherkeycontrols ( void )
{
	// Camera in control of player one
	t.plrid=1;

	// Key configuration
	t.plrkeyW=17;
	t.plrkeyA=30;
	t.plrkeyS=31;
	t.plrkeyD=32;
	t.plrkeyQ=16;
	t.plrkeyE=18;
	t.plrkeyF=33;
	t.plrkeyC=46;
	t.plrkeyZ=44;
	t.plrkeyR=19;
	t.plrkeySPACE=57;
	t.plrkeyRETURN=28;
	t.plrkeySHIFT=42;
	t.plrkeySHIFT2=54;
	t.plrkeyF12=88;
	t.plrkeyJ=36;

	// from SETUP.INI config keys
	if ( t.listkey[1]>0  )  t.plrkeyW = t.listkey[1];
	if ( t.listkey[2]>0  )  t.plrkeyS = t.listkey[2];
	if ( t.listkey[3]>0  )  t.plrkeyA = t.listkey[3];
	if ( t.listkey[4]>0  )  t.plrkeyD = t.listkey[4];
	if ( t.listkey[5]>0  )  t.plrkeySPACE = t.listkey[5];
	if ( t.listkey[6]>0  )  t.plrkeyC = t.listkey[6];
	if ( t.listkey[7]>0  )  t.plrkeyRETURN = t.listkey[7];
	if ( t.listkey[8]>0  )  t.plrkeyR = t.listkey[8];
	if ( t.listkey[9]>0  )  t.plrkeyQ = t.listkey[9];
	if ( t.listkey[10]>0  )  t.plrkeyE = t.listkey[10];
	if ( t.listkey[11]>0  )  t.plrkeySHIFT = t.listkey[11];

	// Read keys from config, and use in player control actions
	if ( g.walkonkeys == 1 ) 
	{
		if ( KeyState(g.keymap[t.plrkeyW]) ==1 ) { t.plrkeyW=1 ; t.plrkeySLOWMOTION=0 ;} else t.plrkeyW=0;
		if ( KeyState(g.keymap[t.plrkeyA]) ==1 ) { t.plrkeyA=1 ; t.plrkeySLOWMOTION=0 ;} else t.plrkeyA=0;
		if ( KeyState(g.keymap[t.plrkeyS]) ==1 ) { t.plrkeyS=1 ; t.plrkeySLOWMOTION=0 ;} else t.plrkeyS=0;
		if ( KeyState(g.keymap[t.plrkeyD]) ==1 ) { t.plrkeyD=1 ; t.plrkeySLOWMOTION=0 ;} else t.plrkeyD=0;
	}
	if ( g.arrowkeyson == 1 ) 
	{
		t.tplrkeySLOWMOTIONold=t.plrkeySLOWMOTION;
		if ( UpKey() == 1 ) { t.plrkeyW = 1  ; t.plrkeySLOWMOTION = 1; }
		if ( LeftKey() == 1 ) { t.plrkeyA = 1  ; t.plrkeySLOWMOTION = 1; }
		if ( DownKey() == 1 ) { t.plrkeyS = 1  ; t.plrkeySLOWMOTION = 1; }
		if ( RightKey() == 1 ) { t.plrkeyD = 1  ; t.plrkeySLOWMOTION = 1; }
		if ( t.tplrkeySLOWMOTIONold != t.plrkeySLOWMOTION ) 
		{
			t.null=MouseMoveX() ; t.null=MouseMoveY();
			t.cammousemovex_f=0 ; t.cammousemovey_f=0;
			t.tFinalCamX_f=ObjectPositionX(t.aisystem.objectstartindex);
			t.tFinalCamY_f=ObjectPositionY(t.aisystem.objectstartindex);
			t.tFinalCamZ_f=ObjectPositionZ(t.aisystem.objectstartindex);
		}
	}
	if ( KeyState(g.keymap[t.plrkeySHIFT]) == 1 && g.runkeys == 1 && t.jumpaction == 0  )  t.plrkeySHIFT = 1; else t.plrkeySHIFT = 0;
	if ( KeyState(g.keymap[t.plrkeySHIFT2]) == 1 && g.runkeys == 1 && t.jumpaction == 0  )  t.plrkeySHIFT2 = 1; else t.plrkeySHIFT2 = 0;
	if ( t.conkit.editmodeactive != 0 ) 
	{
		// FPS 3D Editing Mode - keys elsewhere
	}
	else
	{
		// FPS Gaming Mode
		if ( KeyState(g.keymap[t.plrkeySPACE]) == 1 && g.jumponkey == 1  )  t.plrkeySPACE = 1; else t.plrkeySPACE = 0;
		if ( KeyState(g.keymap[t.plrkeyQ]) == 1 && g.peekonkeys == 1  )  t.plrkeyQ = 1; else t.plrkeyQ = 0;
		if ( KeyState(g.keymap[t.plrkeyE]) == 1 && g.peekonkeys == 1  )  t.plrkeyE = 1; else t.plrkeyE = 0;
		if ( KeyState(g.keymap[t.plrkeyF]) == 1  )  t.plrkeyF = 1; else t.plrkeyF = 0;
		if ( KeyState(g.keymap[t.plrkeyC]) == 1 && g.crouchonkey == 1  )  t.plrkeyC = 1; else t.plrkeyC = 0;
		if ( ControlKey() == 1  )  t.plrkeyC = 1;
		if ( KeyState(g.keymap[t.plrkeyZ]) == 1  )  t.plrkeyZ = 1; else t.plrkeyZ = 0;
		if ( KeyState(g.keymap[t.plrkeyR]) == 1  )  t.plrkeyR = 1; else t.plrkeyR = 0;
		if ( KeyState(g.keymap[t.plrkeyRETURN]) == 1  )  t.plrkeyRETURN = 1; else t.plrkeyRETURN = 0;
		if ( KeyState(g.keymap[t.plrkeyJ]) == 1  )  t.plrkeyJ = 1; else t.plrkeyJ = 0;
	}

	// XBOX/Controller Keys
	if ( g.gxbox == 1 ) 
	{
		if ( JoystickFireC() == 1 ) 
		{
			t.plrkeyR = 1;
		}
		if ( JoystickFireD() == 1 )  // also duplicated in LUA.cpp
		{
			t.plrkeyE = 1;
		}
	}
	if ( g.gxbox == 1 ) 
	{
		if ( g.walkonkeys == 1 ) 
		{
			if ( JoystickY()<-850  )  t.plrkeyW = 1;
			if ( JoystickY()>850  )  t.plrkeyS = 1;
			if ( JoystickX()<-850  )  t.plrkeyA = 1;
			if ( JoystickX()>850  )  t.plrkeyD = 1;
		}
		if ( JoystickFireA() == 1 && g.jumponkey == 1   )  t.plrkeySPACE = 1;
		if ( g.gxboxcontrollertype == 0 ) 
		{
			// XBOX360 Controller
			if ( JoystickFireXL(8) == 1 && g.crouchonkey == 1  )  t.plrkeyC = 1;
			if ( JoystickFireXL(9) == 1  )  t.plrkeyZ = 1;
		}
		if ( g.gxboxcontrollertype == 1 ) 
		{
			// Dual Action
			if ( JoystickFireXL(10) == 1 && g.crouchonkey == 1  )  t.plrkeyC = 1;
			if ( JoystickFireXL(11) == 1  )  t.plrkeyZ = 1;
			if ( JoystickFireXL(4) == 1 && g.runkeys == 1  )  t.plrkeySHIFT = 1;
			if ( JoystickFireXL(6) == 1 && g.runkeys == 1  )  t.plrkeySHIFT = 1;
		}
		if ( g.gxboxcontrollertype == 2 ) 
		{
			// Dual Action F310
			if ( JoystickFireXL(10) == 1 && g.crouchonkey == 1  )  t.plrkeyC = 1;
			if ( JoystickFireXL(11) == 1  )  t.plrkeyZ = 1;
			if ( JoystickFireXL(4) == 1 && g.runkeys == 1  )  t.plrkeySHIFT = 1;
			if ( JoystickFireXL(6) == 1 && g.runkeys == 1  )  t.plrkeySHIFT = 1;
		}
	}

	// Automated actions (script control)
	switch ( g.playeraction ) 
	{
		case 1 : t.player[1].state.firingmode = 1; break ;
		case 2 : t.gunzoommode = 1 ; break ;
		case 3 : t.player[1].state.firingmode = 2 ; break ;
		case 4 : g.forcecrouch = 1 ; break ;
		case 5 : t.plrkeySPACE = 1 ; break ;
		case 6 : t.plrkeyE = 1 ; break ;
		case 7 : t.plrkeyQ = 1 ; break ;
		case 8 : t.plrkeyRETURN = 1 ; break ;
		case 9 : t.tmouseclick = 1 ; break ;
		case 10 : t.tmouseclick = 2 ; break ;
		case 11 : 
		{
			// ensure weapon unjams affect both modes if sharing ammo
			g.firemodes[t.gunid][g.firemode].settings.jammed = 1; 
			if ( t.gun[t.gunid].settings.modessharemags == 1 ) 
			{
				g.firemodes[t.gunid][0].settings.jammed = 1;
				g.firemodes[t.gunid][1].settings.jammed = 1;
			}
		}
		break;
	}

	// Third person disables crouch/zoom/RMB
	t.playercontrol.camrightmousemode=0;
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		g.forcecrouch=0 ; t.plrkeyC=0;
		t.gunzoommode = 0 ; if ( t.tmouseclick == 2 ) { t.tmouseclick = 0  ; t.playercontrol.camrightmousemode = 1; }
	}

	//  Free weapon jam if reload used (possible relocate these to gun module
	if ( t.player[1].state.firingmode == 2 ) //&& t.gunzoommode == 0 ) 
	{
		// unjam or reload animation to unjam weapon
		g.plrreloading=1;
		g.firemodes[t.gunid][g.firemode].settings.shotsfired=0;
		// play free jam animation if it exists
		if ( g.firemodes[t.gunid][g.firemode].action2.clearjam.s != 0 && g.firemodes[t.gunid][g.firemode].settings.jammed == 1 ) 
		{
			// come out of zoom if in it
			if ( t.gunzoommode >=8 ) t.gunzoommode = 11; // catches all states of a zoomed in state

			// play anim to fix jam
			g.plrreloading=2;
			g.custstart=g.firemodes[t.gunid][g.firemode].action2.clearjam.s;
			g.custend=g.firemodes[t.gunid][g.firemode].action2.clearjam.e;
			t.gunmode=9998;
		}
		g.firemodes[t.gunid][g.firemode].settings.shotsfired=0;

		// ensure weapon unjams affect both modes if sharing ammo
		g.firemodes[t.gunid][g.firemode].settings.jammed = 0;
		if ( t.gun[t.gunid].settings.modessharemags == 1 ) 
		{
			g.firemodes[t.gunid][0].settings.jammed = 0;
			g.firemodes[t.gunid][1].settings.jammed = 0;
		}
	}

	// Forced key controls (script control)
	if ( g.forcemove>0  )  t.plrkeyW = 1;
	if ( g.forcemove<0  )  t.plrkeyS = 1;
	if ( g.forcecrouch == 1 && g.playeraction != 4  )  g.forcecrouch = 0;
	if ( g.playeraction != 4  )  g.playeraction = 0;

	// interrogate IDE to see if we have input focus
	if ( t.game.gameisexe == 0 ) 
	{
		t.plrhasfocus=1;
		if ( t.plrfilemapaccess == 1 ) 
		{
			t.plrhasfocus=GetFileMapDWORD( 11, 148 );
		}
	}

	// If player no health (dead), cannot control anything
	if ( t.player[t.plrid].health <= 0 || t.plrhasfocus == 0 ) 
	{
		t.plrkeyW=0;
		t.plrkeyA=0;
		t.plrkeyS=0;
		t.plrkeyD=0;
		t.plrkeyQ=0;
		t.plrkeyE=0;
		t.plrkeyF=0;
		t.plrkeyC=0;
		t.plrkeyZ=0;
		t.plrkeyR=0;
		t.plrkeySPACE=0;
		t.plrkeyRETURN=0;
		t.plrkeySHIFT=0;
		t.plrkeySHIFT2=0;
		t.plrkeyJ=0;
	}
}

void physics_no_gun_zoom ( void )
{
	//t.realfovdegree_f=t.visuals.CameraFOVZoomed_f;//*g.airmod_camfov_f;
	g.realfov_f=t.visuals.CameraFOV_f;//+t.realfovdegree_f;
	if ( g.realfov_f < 15 ) g.realfov_f = 15;
	SetCameraFOV ( g.realfov_f );
	SetCameraFOV ( 2, g.realfov_f );
}

void physics_getcorrectjumpframes ( int entid, float* fStartFrame, float* fHoldFrame, float* fResumeFrame, float* fFinishFrame )
{
	// use frames stored in VAULT animation
	t.q = t.entityprofile[entid].startofaianim + t.csi_stoodvault[1];
	*fStartFrame = t.entityanim[entid][t.q].start;
	*fFinishFrame = t.entityanim[entid][t.q].finish;

	// jump hold animation frames overridden in FPE
	if ( t.entityprofile[entid].jumphold > 0 )
		*fHoldFrame = t.entityprofile[entid].jumphold;
	else
		*fHoldFrame = t.entityanim[entid][t.q].finish - 10;

	// jump resume frame to indicate when can resume movement
	if ( t.entityprofile[entid].jumpresume > 0 )
		*fResumeFrame = t.entityprofile[entid].jumpresume;
	else
		*fResumeFrame = t.entityanim[entid][t.q].finish;
}

void physics_player_control_F9 ( void )
{
	// completely skip use of mousemovexy so LUA mouse system can use it for its own pointer
	if ( g.luaactivatemouse != 1 )
	{
		// No player control if dead, but use up mousemoves to prevent sudden move on respawn or if in multiplayer and respawning
		if ( t.player[t.plrid].health == 0 || t.plrhasfocus == 0 || (t.game.runasmultiplayer == 1 && g.mp.respawnLeft != 0) ) 
		{
			t.tMousemove_f = MouseMoveX() + MouseMoveY();
		}

		// Tab Mode 2, low FPS screen and construction kit menus require mouse.
		t.tConkitMenusOpen = t.conkit.make.paint.menuOpen + t.conkit.make.held.menuOpen + t.conkit.make.feature.menuOpen;
		if ( g.tabmode<2 && g.lowfpswarning != 1 && t.tConkitMenusOpen  ==  0 ) 
		{
			// Manipulate camera angle using mouselook
			if ( g.gminvert == 1 )  t.ttmousemovey = MouseMoveY()*-1; else t.ttmousemovey = MouseMoveY();
			if ( t.plrkeySLOWMOTION == 1 && g.globals.smoothcamerakeys == 1 ) 
			{
				t.cammousemovex_f=t.cammousemovex_f+(MouseMoveX()/10.0);
				t.cammousemovey_f=t.cammousemovey_f+(t.ttmousemovey/10.0);
				t.cammousemovex_f=t.cammousemovex_f*0.94;
				t.cammousemovey_f=t.cammousemovey_f*0.94;
			}
			else
			{
				t.cammousemovex_f=MouseMoveX();
				t.cammousemovey_f=t.ttmousemovey;
			}
			if ( t.plrhasfocus == 1 ) 
			{
				PositionMouse ( GetDesktopWidth()/2,GetDesktopHeight()/2 );
			}
		}
		else
		{
			t.null=MouseMoveX() ; t.null=MouseMoveY();
			t.cammousemovex_f=0 ; t.cammousemovey_f=0;
		}
		if ( g.gxbox == 1 ) 
		{
			t.tjoyrotx_f=((JoystickTwistX()+0.0)-32768.0)/32768.0;
			t.tjoyroty_f=((JoystickTwistY()+0.0)-32768.0)/32768.0;
			if ( t.tjoyrotx_f>-0.2 && t.tjoyrotx_f<0.2  )  t.tjoyrotx_f = 0;
			if ( t.tjoyroty_f>-0.2 && t.tjoyroty_f<0.2  )  t.tjoyroty_f = 0;
			t.cammousemovex_f=t.tjoyrotx_f*6.0;
			t.cammousemovey_f=t.tjoyroty_f*6.0;
		}

		// Modifly rotation speed by zoom amount
		t.tturnspeedmodifier_f=1.0;
		if ( t.gunid>0 ) 
		{
			t.tturnspeedmodifier_f=g.firemodes[t.gunid][g.firemode].settings.plrturnspeedmod;
			if ( t.plrzoomin_f>1 ) 
			{
				// use gunspec settings to control relative speed of zoom mouselook
				t.tturnspeedmodifier_f=t.tturnspeedmodifier_f*g.firemodes[t.gunid][g.firemode].settings.zoomturnspeed;
			}
		}
		t.tRotationDivider_f=8.0/t.tturnspeedmodifier_f;
		t.camangx_f=CameraAngleX(t.terrain.gameplaycamera)+(t.cammousemovey_f/t.tRotationDivider_f);
		t.camangy_f=t.playercontrol.finalcameraangley_f+(t.cammousemovex_f/t.tRotationDivider_f);
	}

	// Cap look up/down angle so cannot wrap around
	if ( t.camangx_f<-80  )  t.camangx_f = -80;
	if ( t.camangx_f>85  )  t.camangx_f = 85;

	// Rotate player camera
	t.tsuspendplayercontrols=0;
	if ( t.player[t.plrid].health>0 && t.tsuspendplayercontrols == 0 ) 
	{
		// when camea rotation overridden, prevent rotation
		if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
		{
			RotateCamera (  t.terrain.gameplaycamera,t.camangx_f,t.camangy_f,0 );
			t.playercontrol.cx_f=CameraAngleX(t.terrain.gameplaycamera);
			t.playercontrol.cy_f=CameraAngleY(t.terrain.gameplaycamera);
			t.playercontrol.cz_f=CameraAngleZ(t.terrain.gameplaycamera);
		}

		// when camera overridden, do not allow movement
		if ( g.luacameraoverride != 1 && g.luacameraoverride != 3 )
		{
			if ( t.conkit.editmodeactive != 0 ) 
			{
				if ( t.plrkeySHIFT == 1 || t.plrkeySHIFT2 == 1 ) 
				{
					t.playercontrol.basespeed_f=4.0;
				}
				else
				{
					if ( t.inputsys.keycontrol == 1 ) 
					{
						t.playercontrol.basespeed_f=0.25;
					}
					else
					{
						t.playercontrol.basespeed_f=1.5;
					}
				}
				t.playercontrol.isrunning=0;
			}
			else
			{
				if ( t.aisystem.playerducking == 1 ) 
				{
					t.playercontrol.basespeed_f=0.5;
					t.playercontrol.isrunning=0;
				}
				else
				{
					if ( t.playercontrol.canrun == 1 && (t.plrkeySHIFT == 1 || t.plrkeySHIFT2 == 1) ) 
					{
						// sprinting speed, modified by directions.
						t.playercontrol.basespeed_f = 2.0;
						if ( t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.camerafollow == 1 ) 
						{
							// WASD run speed
						}
						else
						{
							if ( t.plrkeyS == 1 ) 
							{
								t.playercontrol.basespeed_f = 1.0;
							}
							else
							{
								if ( t.plrkeyW  ==  1 ) 
								{
									if ( t.plrkeyA == 1 || t.plrkeyD == 1 ) 
									{
										t.playercontrol.basespeed_f = 1.75;
									}
								}
								else
								{
									if ( t.plrkeyA == 1 || t.plrkeyD == 1 ) 
									{
										t.playercontrol.basespeed_f = 1.5;
									}
								}
							}
						}
						t.playercontrol.isrunning=1;
					}
					else
					{
						t.playercontrol.basespeed_f=1.0;
						t.playercontrol.isrunning=0;
					}
				}
			}
			t.playercontrol.maxspeed_f=t.playercontrol.topspeed_f*t.playercontrol.basespeed_f;
			t.playercontrol.movement=0;
			if ( t.plrkeyA == 1 ) 
			{
				t.playercontrol.movement=1;
				t.playercontrol.movey_f=t.playercontrol.cy_f-90;
				if ( t.plrkeyW == 1 ) t.playercontrol.movey_f=t.playercontrol.cy_f-45;
				if ( t.plrkeyS == 1 ) t.playercontrol.movey_f=t.playercontrol.cy_f-45-90 ;
			}
			else
			{
				if ( t.plrkeyD == 1 ) 
				{
					t.playercontrol.movement=1;
					t.playercontrol.movey_f=t.playercontrol.cy_f+90;
					if ( t.plrkeyW == 1 ) t.playercontrol.movey_f=t.playercontrol.cy_f+45;
					if ( t.plrkeyS == 1 ) t.playercontrol.movey_f=t.playercontrol.cy_f+45+90;
				}
				else
				{
					if ( t.plrkeyW == 1 ) { t.playercontrol.movement=1 ; t.playercontrol.movey_f=t.playercontrol.cy_f; }
					if ( t.plrkeyS == 1 ) { t.playercontrol.movement=1 ; t.playercontrol.movey_f=t.playercontrol.cy_f+180; }
				}
			}
		}
		else
		{
			// camera overridden players cannot move themselves
			t.playercontrol.movement=0;
		}
	}
	else
	{
		//  dead players cannot move themselves
		t.playercontrol.movement=0;
	}
	t.playercontrol.lastmovement=t.playercontrol.movement;

	//  Control speed and walk wobble
	if ( t.playercontrol.movement>0 ) 
	{
		t.tokay=0;
		if ( t.playercontrol.gravityactive == 1 ) 
		{
			if ( t.playercontrol.plrhitfloormaterial != 0  )  t.tokay = 1;
			if ( t.playercontrol.underwater == 1 && t.hardwareinfoglobals.nowater == 0  )  t.tokay = 1;
			if ( t.playercontrol.jumpmode == 1 && t.playercontrol.jumpmodecanaffectvelocitycountdown_f>0  )  t.tokay = 1;
		}
		else
		{
			t.tokay=1;
		}
		if ( t.tokay == 1 ) 
		{
			// increase plr speed using accel ratio
			t.playercontrol.speed_f=t.playercontrol.speed_f+(t.playercontrol.accel_f*g.timeelapsed_f*0.05);
			if ( t.playercontrol.speed_f>t.playercontrol.maxspeed_f*t.playercontrol.speedratio_f ) 
			{
				t.playercontrol.speed_f=t.playercontrol.maxspeed_f*t.playercontrol.speedratio_f;
			}
		}
		else
		{
			// with no Floor (  under player, reduce any speed for downhill and fall behavior )
			t.tdeductspeed_f=(t.playercontrol.maxspeed_f*t.playercontrol.speedratio_f)*g.timeelapsed_f*0.01;
			t.playercontrol.speed_f=t.playercontrol.speed_f-t.tdeductspeed_f;
			if ( t.playercontrol.speed_f<0.0  )  t.playercontrol.speed_f = 0.0;
		}
		if ( t.playercontrol.gravityactive == 1 && t.playercontrol.jumpmode != 1 ) 
		{
			// on ground
			t.playercontrol.wobble_f=WrapValue(t.playercontrol.wobble_f+(t.playercontrol.wobblespeed_f*t.ElapsedTime_f*t.playercontrol.basespeed_f*t.playercontrol.speedratio_f));
		}
		else
		{
			// in air
			t.playercontrol.wobble_f=CurveValue(0,t.playercontrol.wobble_f,3.0);
		}
	}
	else
	{
		t.tdeductspeed_f=(t.playercontrol.maxspeed_f*t.playercontrol.speedratio_f)*g.timeelapsed_f*0.5;
		t.playercontrol.speed_f=t.playercontrol.speed_f-t.tdeductspeed_f;
		if (  t.playercontrol.speed_f<0.0  )  t.playercontrol.speed_f = 0.0;
	}

	// Pass movement state to player array
	t.player[t.plrid].state.moving=0;
	if ( t.playercontrol.movement == 1 ) 
	{
		t.player[t.plrid].state.moving=1;
		if ( t.aisystem.playerducking != 0 ) 
		{
			t.player[t.plrid].state.moving=2;
		}
	}

	// Prevent player leaving terrain area
	if ( ObjectPositionX(t.aisystem.objectstartindex)<100 ) { t.playercontrol.pushangle_f = 90.0  ; t.playercontrol.pushforce_f = 1.0; }
	if ( ObjectPositionX(t.aisystem.objectstartindex)>51100 ) { t.playercontrol.pushangle_f = 270.0  ; t.playercontrol.pushforce_f = 1.0; }
	if ( ObjectPositionZ(t.aisystem.objectstartindex)<100 ) { t.playercontrol.pushangle_f = 0.0  ; t.playercontrol.pushforce_f = 1.0; }
	if ( ObjectPositionZ(t.aisystem.objectstartindex)>51100 ) { t.playercontrol.pushangle_f = 180.0  ; t.playercontrol.pushforce_f = 1.0; }

	// Reduce any player push force over time
	if ( t.playercontrol.pushforce_f>0 ) 
	{
		t.playercontrol.pushforce_f=t.playercontrol.pushforce_f-(g.timeelapsed_f/2.0);
		if ( t.playercontrol.pushforce_f<0 ) 
		{
			t.playercontrol.pushforce_f=0.0;
		}
	}

	// Get current player ground height stood on
	if ( t.terrain.TerrainID>0 ) 
	{
		t.tplayerterrainheight_f=BT_GetGroundHeight(t.terrain.TerrainID,ObjectPositionX(t.aisystem.objectstartindex),ObjectPositionZ(t.aisystem.objectstartindex));
	}
	else
	{
		t.tplayerterrainheight_f=1000.0;
	}

	// Control player using Physics character controller
	t.tgotonewheight=0;
	t.tjetpackverticalmove_f=0.0;
	if ( t.playercontrol.gravityactive == 0 ) 
	{
		// in air
		if ( t.playercontrol.lockatheight == 1 ) 
		{
			t.tjetpackverticalmove_f=0;
		}
		else
		{
			t.tjetpackverticalmove_f=WrapValue(CameraAngleX(t.terrain.gameplaycamera));
			if ( t.plrkeyS == 1  )  t.tjetpackverticalmove_f = t.tjetpackverticalmove_f-360.0;
			if ( t.plrkeyA == 1  )  t.tjetpackverticalmove_f = 0;
			if ( t.plrkeyD == 1  )  t.tjetpackverticalmove_f = 0;
		}

		// extra height control with mouse wheel
		if ( t.playercontrol.controlheight>0 || t.playercontrol.controlheightcooldown>0 ) 
		{
			t.playercontrol.storemovey=t.playercontrol.movey_f;
			if ( t.playercontrol.controlheight == 3 ) 
			{
				if ( t.terrain.TerrainID>0 ) 
				{
					t.tgotonewheight=1000.0+t.tplayerterrainheight_f;
					t.playercontrol.controlheight=0;
				}
			}
			else
			{
				if ( t.playercontrol.controlheightcooldown > 0.0f )
				{
					if ( t.playercontrol.controlheightcooldownmode == 1 ) 
					{
						t.playercontrol.movey_f=t.playercontrol.movey_f+180;
						t.tjetpackverticalmove_f=-270;
					}
					else
						t.tjetpackverticalmove_f=90;
				}
				else
				{
					if ( t.playercontrol.controlheight == 1 ) 
					{
						// move up
						t.playercontrol.movey_f=t.playercontrol.movey_f+180;
						t.tjetpackverticalmove_f=-270;
					}
					else
					{
						// move down
						t.tjetpackverticalmove_f=90;
					}
					t.playercontrol.controlheightcooldownmode = t.playercontrol.controlheight;
				}
			}
			if ( t.playercontrol.controlheightcooldown > 0 )
			{
				t.playercontrol.controlheightcooldown = t.playercontrol.controlheightcooldown - 1.0f;
				t.playercontrol.speed_f=0;
			}
			else
				t.playercontrol.speed_f=t.playercontrol.maxspeed_f*t.playercontrol.speedratio_f;
		}
	}
	else
	{
		// regular gravity
		if ( t.playercontrol.plrhitfloormaterial == 0 && t.playercontrol.jumpmode != 1 ) 
		{
			t.tjetpackverticalmove_f=25.0;
		}
	}

	// handle slope climb prevention
	t.steepstart_f=30;
	t.steepslopeweight_f=1.0;
	if ( t.tgotonewheight == 0 ) 
	{
		// Player direction and speed
		t.tfinalplrmovey_f=t.playercontrol.movey_f;
		t.tfinalplrspeed_f=t.playercontrol.speed_f*2;
		// If not in air due to jetpack thurst
		if (  t.playercontrol.jetpackthrust_f == 0 ) 
		{
			// Player speed affected by gun speed modifier and zoom mode modifier
			if ( t.gunid>0 ) 
			{
				if ( t.game.runasmultiplayer == 0 && g.globals.enableplrspeedmods == 1 ) 
				{
					// only for single player action - MP play is too muddy!
					if ( g.firemodes[t.gunid][g.firemode].settings.isempty == 1 ) 
					{
						t.tfinalplrspeed_f=t.tfinalplrspeed_f*g.firemodes[t.gunid][g.firemode].settings.plremptyspeedmod;
					}
					else
					{
						t.tfinalplrspeed_f=t.tfinalplrspeed_f*g.firemodes[t.gunid][g.firemode].settings.plrmovespeedmod;
					}
					if ( t.plrzoomin_f>1 ) 
					{
						t.tfinalplrspeed_f=t.tfinalplrspeed_f*g.firemodes[t.gunid][g.firemode].settings.zoomwalkspeed;
					}
					if ( (t.gunmode >= 121 && t.gunmode <= 126) || (t.gunmode>= 700 && t.gunmode <= 707) || (t.gunmode >= 7000 && t.gunmode<= 7099) ) 
					{
						t.tfinalplrspeed_f=t.tfinalplrspeed_f*g.firemodes[t.gunid][g.firemode].settings.plrreloadspeedmod;
					}
				}
			}
			// Determine slope angle of plr direction
			if ( t.terrain.TerrainID>0 ) 
			{
				t.tplrx_f=ObjectPositionX(t.aisystem.objectstartindex);
				t.tplry_f=ObjectPositionY(t.aisystem.objectstartindex);
				t.tplrz_f=ObjectPositionZ(t.aisystem.objectstartindex);
				t.tplrgroundy_f=t.tplry_f-34.0;
				t.tplrx2_f=NewXValue(t.tplrx_f,t.tfinalplrmovey_f,20.0);
				t.tplrz2_f=NewZValue(t.tplrz_f,t.tfinalplrmovey_f,20.0);
				t.tplrgroundy2_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tplrx2_f,t.tplrz2_f);
				t.tplrgroundy2_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tplrx2_f,t.tplrz2_f);
				if ( t.tplrgroundy2_f>t.tplrgroundy_f ) 
				{
					t.tsteepvalue_f=t.tplrgroundy2_f-t.tplrgroundy_f;
					if ( t.tsteepvalue_f>t.steepstart_f ) 
					{
						// terrain steeper than 45 degrees starts to slow down move speed
						t.tfinalplrspeed_f=t.tfinalplrspeed_f-((t.tsteepvalue_f-t.steepstart_f)/t.steepslopeweight_f);
						if ( t.tfinalplrspeed_f<0  )  t.tfinalplrspeed_f = 0;
					}
				}
			}
		}

		// Move player using Bullet character controller
		if ( g.globals.riftmode>0  )  t.tfinalplrmovey_f = t.tfinalplrmovey_f+0;
		if ( t.playercontrol.isrunning == 1  )  t.tjetpackverticalmove_f = 0;
		ODEControlDynamicCharacterController ( t.aisystem.objectstartindex,t.tfinalplrmovey_f,t.tjetpackverticalmove_f,t.tfinalplrspeed_f,t.playerjumpnow_f,t.aisystem.playerducking,t.playercontrol.pushangle_f,t.playercontrol.pushforce_f,t.playercontrol.jetpackthrust_f );
		if ( t.playercontrol.gravityactive == 0 ) 
		{
			if ( t.playercontrol.controlheight>0 ) 
			{
				t.playercontrol.movey_f=t.playercontrol.storemovey;
			}
		}
	}
	else
	{
		// shift player to new height position ('M' key)
		physics_disableplayer ( );
		t.terrain.playerx_f=ObjectPositionX(t.aisystem.objectstartindex);
		t.terrain.playery_f=t.tgotonewheight;
		t.terrain.playerz_f=ObjectPositionZ(t.aisystem.objectstartindex);
		t.terrain.playerax_f=0;
		t.terrain.playeray_f=CameraAngleY(0);
		t.terrain.playeraz_f=0;
		t.playercontrol.finalcameraangley_f=t.terrain.playeray_f;
		physics_setupplayer ( );
		ODESetWorldGravity (  0,0,0 );
		if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
		{
			RotateCamera (  t.terrain.gameplaycamera,90,CameraAngleY(t.terrain.gameplaycamera),CameraAngleZ(t.terrain.gameplaycamera) );
		}
	}

	// Place camera at player object position (stood or ducking)
	if ( t.aisystem.playerducking != 0 ) 
	{
		t.tsubtleeyeadjustment_f=10.0;
	}
	else
	{
		t.tsubtleeyeadjustment_f=30.0;
	}
	t.tadjustbasedonwobbley_f=(Cos(t.playercontrol.wobble_f)*t.playercontrol.wobbleheight_f);
	if ( t.plrkeySLOWMOTION == 1 && g.globals.smoothcamerakeys == 1 ) 
	{
		t.tFinalCamX_f=CurveValue(ObjectPositionX(t.aisystem.objectstartindex),t.tFinalCamX_f,10.0);
		t.tFinalCamY_f=CurveValue(t.tsubtleeyeadjustment_f+ObjectPositionY(t.aisystem.objectstartindex)-t.tadjustbasedonwobbley_f,t.tFinalCamY_f,10.0);
		t.tFinalCamZ_f=CurveValue(ObjectPositionZ(t.aisystem.objectstartindex),t.tFinalCamZ_f,10.0);
	}
	else
	{
		t.tleanangle_f=CameraAngleY(t.terrain.gameplaycamera)+t.playercontrol.leanoverangle_f;
		t.tleanoverx_f=NewXValue(0,t.tleanangle_f,t.playercontrol.leanover_f);
		t.tleanoverz_f=NewZValue(0,t.tleanangle_f,t.playercontrol.leanover_f);
		t.tFinalCamX_f = ObjectPositionX(t.aisystem.objectstartindex)+t.tleanoverx_f;
		t.tFinalCamY_f = t.tsubtleeyeadjustment_f+ObjectPositionY(t.aisystem.objectstartindex)-t.tadjustbasedonwobbley_f;
		t.tFinalCamZ_f = ObjectPositionZ(t.aisystem.objectstartindex)+t.tleanoverz_f;
	}

	// record present camera angles
	t.playercontrol.finalcameraanglex_f = CameraAngleX(t.terrain.gameplaycamera);
	t.playercontrol.finalcameraangley_f = CameraAngleY(t.terrain.gameplaycamera);
	t.playercontrol.finalcameraanglez_f = CameraAngleZ(t.terrain.gameplaycamera);

	// update camera position
	if ( g.luacameraoverride != 1 && g.luacameraoverride != 3 )
	{
		PositionCamera (  t.terrain.gameplaycamera,t.tFinalCamX_f+t.tshakex_f,t.tFinalCamY_f+t.tshakey_f,t.tFinalCamZ_f+t.tshakez_f );
	}

	// Apply flinch (for when hit hit)
	if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
	{
		t.playercontrol.flinchx_f=CurveValue(0,t.playercontrol.flinchx_f,20);
		t.playercontrol.flinchy_f=CurveValue(0,t.playercontrol.flinchy_f,20);
		t.playercontrol.flinchz_f=CurveValue(0,t.playercontrol.flinchz_f,20);
		t.playercontrol.flinchcurrentx_f=CurveValue(t.playercontrol.flinchx_f,t.playercontrol.flinchcurrentx_f,10.0);
		t.playercontrol.flinchcurrenty_f=CurveValue(t.playercontrol.flinchy_f,t.playercontrol.flinchcurrenty_f,10.0);
		t.playercontrol.flinchcurrentz_f=CurveValue(t.playercontrol.flinchz_f,t.playercontrol.flinchcurrentz_f,10.0);
		RotateCamera (  t.terrain.gameplaycamera,CameraAngleX(t.terrain.gameplaycamera)+t.playercontrol.flinchcurrentx_f,CameraAngleY(t.terrain.gameplaycamera)+t.playercontrol.flinchcurrenty_f,CameraAngleZ(t.terrain.gameplaycamera)+t.playercontrol.flinchcurrentz_f+(t.playercontrol.leanover_f/-10.0) );
	}
}

void physics_player_control_LUA ( void )
{
	// when LUA global ready, call LUA player control function
	if ( t.playercontrol.gameloopinitflag == 0 )
	{
		// F9 Edit Mode Controls internal
		if ( t.conkit.editmodeactive != 0 )
		{
			// F9 movement control (for some reason LuaCall does not run PlayerControl in global.lua)??!
			physics_player_control_F9();
		}
		else
		{
			// Feed in-game mappable keys
			LuaSetInt ( "g_PlrKeyW", t.plrkeyW );
			LuaSetInt ( "g_PlrKeyA", t.plrkeyA );
			LuaSetInt ( "g_PlrKeyS", t.plrkeyS );
			LuaSetInt ( "g_PlrKeyD", t.plrkeyD );
			LuaSetInt ( "g_PlrKeyQ", t.plrkeyQ );
			LuaSetInt ( "g_PlrKeyE", t.plrkeyE );
			LuaSetInt ( "g_PlrKeyF", t.plrkeyF );
			LuaSetInt ( "g_PlrKeyC", t.plrkeyC );
			LuaSetInt ( "g_PlrKeyZ", t.plrkeyZ );
			LuaSetInt ( "g_PlrKeyR", t.plrkeyR );
			LuaSetInt ( "g_PlrKeySPACE", t.plrkeySPACE );
			LuaSetInt ( "g_PlrKeyRETURN", t.plrkeyRETURN );
			LuaSetInt ( "g_PlrKeySHIFT", t.plrkeySHIFT );
			LuaSetInt ( "g_PlrKeySHIFT2", t.plrkeySHIFT2 );
			LuaSetInt ( "g_PlrKeyJ", t.plrkeyJ );

			// Call externaliszed script
			LuaSetFunction ( "PlayerControl", 0, 0 );
			LuaCall();
		}

		// System to detect when player (for whatever reason) drops BELOW the terrain
		if ( ObjectPositionY(t.aisystem.objectstartindex)<t.tplayerterrainheight_f-30.0 ) 
		{
			// player should NEVER be here - drastically recreate player
			t.terrain.playerx_f=ObjectPositionX(t.aisystem.objectstartindex);
			t.terrain.playery_f=t.tplayerterrainheight_f+20.0+t.terrain.adjaboveground_f;
			t.terrain.playerz_f=ObjectPositionZ(t.aisystem.objectstartindex);
			t.terrain.playerax_f=0;
			t.terrain.playeray_f=CameraAngleY(0);
			t.terrain.playeraz_f=0;
			t.playercontrol.finalcameraangley_f=t.terrain.playeray_f;
			physics_disableplayer ( );
			physics_setupplayernoreset ( );
		}
	}
}

void physics_player_control ( void )
{
	// No player controls when in editing mode
	if ( t.plrhasfocus == 0 ) return;

	// Gather input data
	t.k_s=Lower(Inkey() );

	// Invincibe Mode - God Mode
	if ( g.ggodmodestate == 1 && t.k_s == "i" ) t.player[1].health = 99999;

	// Get MouseClick (except when in TAB TAB mode)
	// Mode 1 = ignore A, C, D buttons of controller
	if ( g.tabmode < 2 ) 
		t.tmouseclick = control_mouseclick_mode(1);
	else
		t.tmouseclick=0;

	// Set input data for LUA call
	LuaSetInt ( "g_KeyPressJ", t.plrkeyJ );
	LuaSetInt ( "g_MouseClickControl", t.tmouseclick );
	physics_player_control_LUA();

	// Apply colour to shader
	SetVector4 ( g.terrainvectorindex, t.playercontrol.redDeathFog_f, 0, 0, 0 );
	t.tColorVector = g.terrainvectorindex;
	postprocess_setscreencolor ( );
}

void physics_player_handledeath ( void )
{
	// handle player death
	if (  t.game.runasmultiplayer == 0 ) 
	{
		//  Handle player death - only for single player
		if (  t.playercontrol.deadtime>0 ) 
		{
			//  control sequence
			if (  t.playercontrol.thirdperson.enabled == 0 ) 
			{
				if (  CameraAngleZ(0)<45 ) 
				{
					if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
					{
						ZRotateCamera (  0,CameraAngleZ(0)+5.0 );
					}
				}
			}
			//  when death pause over
			if (  t.aisystem.processplayerlogic == 1 ) 
			{
				if (  Timer()>t.playercontrol.deadtime ) 
				{
					if (  t.playercontrol.startlives>0 && t.player[t.plrid].lives == 0 && t.game.gameisexe == 1 ) 
					{
						//  280415 - GAME OVER flag!
						t.game.gameloop=0 ; t.game.lostthegame=1;
					}
					else
					{
						//  move player to start
						physics_player_gotolastcheckpoint ( );
					}
				}
				else
				{
					//  screen fade now handled in _physics_player_handlehealth
				}
			}
		}
	}
}

void physics_player_reset_underwaterstate ( void )
{
	visuals_underwater_off ( );
	t.playercontrol.inwaterstate = 0;
	t.playercontrol.drowntimestamp = 0;
}

void physics_player_listener ( void )
{
	ScaleListener (  5.0  ); RotateListener (  0,CameraAngleY(),0 );
	PositionListener ( CameraPositionX(), CameraPositionY(), CameraPositionZ() );
}

void physics_player_takedamage ( void )
{
	//  Receives; tdamage, te, tDrownDamageFlag
	//  Uses tDrownDamageFlag to avoid blood splats and other non drowning damage effects.
	//  This is set to 0 after takedamage is called, so doesn't need to be unset elsewhere
	//  before calling this sub

	//  player cannot be damaged when immune!
	if (  t.huddamage.immunity>0  )  return;

	//  quite early if in F9 editing mode
	if (  t.conkit.editmodeactive != 0  )  return;

	//  Apply player health damage
	if (  (t.tdamage>0 && t.player[t.plrid].health>0 && t.player[t.plrid].health<99999) || t.tdamage>65000 ) 
	{
		//  Flag player damage in health regen code
		if (  t.playercontrol.regentime>0  )  t.playercontrol.regentime = Timer();
		//  Deduct health from player
		if (  t.playercontrol.startstrength>0  )  t.player[t.plrid].health = t.player[t.plrid].health-t.tdamage;
		//  if NOT drowning, do usual damage stuff
		if (  t.tDrownDamageFlag  ==  0 ) 
		{
			//  Instruct HUD about player damage
			if (  t.playercontrol.startviolent != 0 && g.quickparentalcontrolmode != 2 )
			{
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					//  third person character produces blood
					if (  t.te != -1 ) 
					{
						t.ttobj1=t.entityelement[t.playercontrol.thirdperson.charactere].obj;
						t.decalx1=ObjectPositionX(t.ttobj1);
						t.decaly1=ObjectPositionY(t.ttobj1)+(ObjectSizeY(t.ttobj1)/2);
						t.decalz1=ObjectPositionZ(t.ttobj1);
						t.ttobj2=t.entityelement[t.te].obj;
						t.decalx2=ObjectPositionX(t.ttobj2);
						t.decaly2=ObjectPositionY(t.ttobj2)+(ObjectSizeY(t.ttobj2)/2);
						t.decalz2=ObjectPositionZ(t.ttobj2);
						t.ttdx_f=t.decalx1-t.decalx2;
						t.ttdy_f=t.decaly1-t.decaly2;
						t.ttdz_f=t.decalz1-t.decalz2;
						t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
						t.ttdx_f=t.ttdx_f/t.ttdd_f;
						t.ttdy_f=t.ttdy_f/t.ttdd_f;
						t.ttdz_f=t.ttdz_f/t.ttdd_f;
						g.decalx=t.decalx1-(t.ttdx_f*5.0);
						g.decaly=t.decaly1-(t.ttdy_f*5.0);
						g.decalz=t.decalz1-(t.ttdz_f*5.0);
						g.decalx=(g.decalx-10)+Rnd(20);
						g.decaly=(g.decaly-20)+Rnd(40);
						g.decalz=(g.decalz-10)+Rnd(20);
						for ( t.iter = 1 ; t.iter<=  1+Rnd(1); t.iter++ )
						{
							decal_triggerbloodsplat ( );
						}
					}
				}
				else
				{
					if (  t.te != -1 ) 
					{
						//  only if entity caused damage
						new_damage_marker(t.te,ObjectPositionX(t.entityelement[t.te].obj),ObjectPositionY(t.entityelement[t.te].obj),ObjectPositionZ(t.entityelement[t.te].obj),t.tdamage);
					}
					else
					{
						//  hurt from non-entity source
						for ( t.iter = 0 ; t.iter<=  9; t.iter++ )
						{
							placeblood(50,0,0,0,0);
						}
					}
				}
			}
			//  Trigger player grunt noise
			if (  t.playercontrol.startviolent != 0 && g.quickparentalcontrolmode != 2 ) 
			{
				t.tgruntfrequency=0;
				if ( 1 ) // 100316 - v1.121b3 - t.player[t.plrid].health<t.playercontrol.startstrength*0.25 ) 
				{
					//  one in three chance of grunting and been a while..
					if ( (DWORD)(Timer()+3000) > t.playercontrol.timesincelastgrunt ) 
					{
						if (  Rnd(4) == 1  )  t.tgruntfrequency = 1;
					}
				}
				else
				{
					//  one in ten chance of grunting
					if (  Rnd(11) == 1  )  t.tgruntfrequency = 1;
				}
				if (  t.tgruntfrequency == 1 ) 
				{
					//  only every one in three or if been a while since we grunted
					t.playercontrol.timesincelastgrunt=Timer();
					t.tplrhurt=2+Rnd(2) ; t.tsnd=t.playercontrol.soundstartindex+t.tplrhurt;
					playinternalsound(t.tsnd);
				}
			}
		}
		//  Check if player health at zero
		if (  t.player[t.plrid].health <= 0 ) 
		{
			if (  t.game.runasmultiplayer  ==  1 ) 
			{
				if (  t.tsteamwasnetworkdamage  ==  1 ) 
				{
					if (  t.entityelement[t.texplodesourceEntity].mp_networkkill  ==  1 ) 
					{
						//  inform of network kill
						mp_networkkill ( );
					}
				}
			}
			//  player looses a life
			t.player[t.plrid].health=0;
			if (  t.playercontrol.startlives>0 ) 
			{
				//  only reduce lives if using lives
				if (  t.game.runasmultiplayer == 0 ) 
				{
					t.player[t.plrid].lives=t.player[t.plrid].lives-1;
					if (  t.player[t.plrid].lives <= 0 ) 
					{
						t.player[t.plrid].lives=0;
					}
				}
			}
			if (  t.playercontrol.startviolent != 0 && g.quickparentalcontrolmode != 2 )
			{
				if (  t.tDrownDamageFlag == 0 ) 
				{
					//  player grunts in deadness if this isn't death by drowning
					playinternalsound(t.playercontrol.soundstartindex+1);
				}
			}
			// if camera was overriden, take it back
			g.luacameraoverride = 0;
			// restore player zoom
			t.plrzoominchange=1 ; t.plrzoomin_f=0.0;
			gun_playerdead ( );
			// start death sequence for player
			t.playercontrol.deadtime=Timer()+2000;
			// make sure all music is stopped
			if (  t.playercontrol.disablemusicreset == 0 ) 
			{
				music_resetall ( );
			}
			//  if third person, also create ragdoll of protagonist
			if (  t.playercontrol.thirdperson.enabled == 1 ) 
			{
				t.ttte=t.playercontrol.thirdperson.charactere;
				t.tdamageforce=0;
				t.entityelement[t.ttte].health=1 ; t.tdamage=1;
				t.entityelement[t.ttte].ry=ObjectAngleY(t.entityelement[t.ttte].obj);
				t.tskiplayerautoreject=1;
				entity_applydamage ( );
				t.tskiplayerautoreject=0;
			}
		}
	}
}

void physics_player_gotolastcheckpoint ( void )
{
	//  move player to last checkpoint (or start marker if no checkpoint)
	t.terrain.playerx_f=t.playercheckpoint.x;
	t.terrain.playery_f=t.playercheckpoint.y;
	t.terrain.playerz_f=t.playercheckpoint.z;
	t.terrain.playerax_f=0;
	t.terrain.playeray_f=t.playercheckpoint.a;
	t.terrain.playeraz_f=0;
	t.playercontrol.finalcameraangley_f=t.terrain.playeray_f;
	physics_resetplayer_core ( );
}

void physics_resetplayer_core ( void )
{
	//  Cannot restart under water!
	if (  t.hardwareinfoglobals.nowater == 0 ) 
	{
		if (  t.terrain.playery_f<t.terrain.waterliney_f ) 
		{
			t.terrain.playery_f=t.terrain.waterliney_f+t.terrain.adjaboveground_f;
		}
	}

	//  if the player was previous underwater, set them above water and switch off underwater effects
	physics_player_reset_underwaterstate ( );

	//  disable and setup player
	physics_disableplayer ( );
	physics_setupplayer ( );

	//  restore health
	t.player[t.plrid].health=t.playercontrol.startstrength;

	//  ressurection cease fire allows player to escape shooters when respawn
	t.playercontrol.ressurectionceasefire=Timer()+3000;

	//  reset vegetation
	t.completelyfillvegarea=1;
	terrain_fastveg_loop ( );

	//  fade in game screen again
	t.postprocessings.fadeinvalue_f=0.0;

	//  player is immune for a while
	t.huddamage.immunity=1000;

	//  reset death state
	t.playercontrol.deadtime=0;
	if ( g.luacameraoverride != 2 && g.luacameraoverride != 3 )
	{
		ZRotateCamera (  0,0 );
	}

	//  red screen effect finish
	t.playercontrol.redDeathFog_f = 0;
	SetVector4 (  g.terrainvectorindex,0,0,0,0 );
	t.tColorVector = g.terrainvectorindex ; postprocess_setscreencolor ( );

	//  Stop any blood HUD
	resetblood();
	resetdamagemarker();

	//  Deal with sounds if not disabled via script
	if (  t.playercontrol.disablemusicreset == 0 ) 
	{
		//  Stop any incidental music
		game_stopallsounds ( 0 );
		//  Stop any looping projectile sounds
		weapon_projectile_reset ( );
		//  play default music
		music_playdefault ( );
		//  Restore any sounds from last checkpoint/start marker
		for ( t.s = g.soundbankoffset ; t.s<=  g.soundbankoffsetfinish; t.s++ )
		{
			if (  t.soundloopcheckpoint[t.s] == 1 ) 
			{
				if (  SoundExist(t.s) == 1 ) 
				{
					LoopSound (  t.s );
				}
			}
		}
	}

	//  ensure all markers and waypoints remain hidden
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entityprofile[t.entid].ismarker != 0 ) 
		{
			t.obj=t.entityelement[t.e].obj;
			if (  t.obj>0 ) 
			{
				if (  ObjectExist(t.obj) == 1 ) 
				{
					HideObject (  t.obj );
				}
			}
		}
	}
	waypoint_hideall ( );

	// if third person, resurrect ragdoll protagonist
	if ( t.playercontrol.thirdperson.enabled == 1 ) 
	{
		// ensure character protagonist is respawned
		t.e=t.playercontrol.thirdperson.charactere;
		entity_lua_spawn_core ( );
		physics_player_thirdpersonreset ( );
		//PE: Something is disable Z depth buffer when 3rd die ?, just reenable.
		//PE: https://github.com/TheGameCreators/GameGuruRepo/issues/330
		EnableObjectZWrite(t.entityelement[t.e].obj);
		EnableObjectZDepth(t.entityelement[t.e].obj);
	}
}

void physics_player_thirdpersonreset ( void )
{
	//  convert to clone so can animate it (ragdoll makes it an instance)
	t.tentityconverttoclonenotshared=1;
	t.tte=t.e ; entity_converttoclone ( );
	t.tentityconverttoclonenotshared=0;
	t.charanimstate.e=t.e ; t.obj=t.entityelement[t.e].obj;
	entity_setupcharobjsettings ( );
	//  and reset any transitional states in animation system (avoid freezing it)
	t.smoothanim[t.charanimstate.obj].transition=0;
	t.smoothanim[t.charanimstate.obj].playstarted=0;
	//  camera has smoothing in third person, so reset this
	if ( g.luacameraoverride != 1 && g.luacameraoverride != 3 )
	{
		PositionCamera (  t.terrain.gameplaycamera,t.terrain.playerx_f,t.terrain.playery_f,t.terrain.playerz_f );
	}

	// ensure depth is not written so no motion blur
	// apply special character shader so can uniquely change shader constants
	// without affecting other NPCs and trees, etc
	if ( t.obj>0 ) 
	{
		if ( ObjectExist(t.obj) == 1 ) 
		{
			int tttentid = t.entityelement[t.e].bankindex;
			int ttsourceobj = g.entitybankoffset + tttentid;
			if ( ttsourceobj > 0 )
			{
				if ( ObjectExist ( ttsourceobj ) == 1 )
				{
					if ( GetNumberOfFrames ( ttsourceobj ) > 0 )
					{
						// third person is animating
						SetObjectEffect ( t.obj, g.thirdpersoncharactereffect );
						SetEffectConstantF (  g.thirdpersoncharactereffect,"DepthWriteMode",0.0f );			
					}
					else
					{
						// third person is non-animating
						SetObjectEffect ( t.obj, g.thirdpersonentityeffect );
						SetEffectConstantF ( g.thirdpersonentityeffect, "DepthWriteMode", 0.0f );
					}
				}
			}
		}
	}
	//  and also treat the vweap attachment too
	t.tattachmentobj=t.entityelement[t.e].attachmentobj;
	if (  t.tattachmentobj>0 ) 
	{
		if (  ObjectExist(t.tattachmentobj) == 1 ) 
		{
			SetObjectEffect (  t.tattachmentobj,g.thirdpersonentityeffect );
			SetEffectConstantF (  g.thirdpersonentityeffect,"DepthWriteMode",0.0f );
		}
	}
}

void physics_player_addweapon ( void )
{
	//  takes weaponindex
	t.tweaponisnew=0;
	//  check all weapon slots
	t.gotweapon=0;
	for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
	{
		if (  t.weaponslot[t.ws].got == t.weaponindex  )  t.gotweapon = t.ws;
	}
	if (  t.gotweapon == 0 ) 
	{
		//  check if we have a slot preference
		t.tweaponisnew=1;
		t.gotweaponpref=0;
		for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
		{
			if (  t.weaponslot[t.ws].pref == t.weaponindex  )  t.gotweaponpref = t.ws;
		}
		//  add weapon
		if (  t.gotweaponpref == 0 ) 
		{
			//  find free slot
			for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
			{
				if (  t.weaponslot[t.ws].pref == 0  )  break;
			}
			if (  g.forcedslot != 0 ) { t.ws = g.forcedslot  ; t.gotweaponpref = t.ws ; g.forcedslot = 0; }
			//  count weapons for maximum slots. If exceeded, prevent pick up.
			t.weaps=0;
			for ( t.count = 1 ; t.count <= 10 ; t.count++ )
			{
				if (  t.weaponslot[t.count].pref != 0  )  ++t.weaps;
			}
			if (  t.weaps >= g.maxslots  )  t.ws = 100;
			if (  t.ws <= 10 ) 
			{
				//  add weapon into free slot and create pref for it
				t.weaponslot[t.ws].pref=t.weaponindex;
				t.weaponhud[t.ws]=t.gun[t.weaponindex].hudimage;
				//  mark weapon with 'possible' entity that held this weapon (for equipment activation)
				g.firemodes[t.weaponindex][0].settings.equipmententityelementindex=t.autoentityusedtoholdweapon;
			}
			else
			{
				//  no room for weapon in available slots
				t.ws=0;
			}
		}
		else
		{
			t.ws=t.gotweaponpref;
		}

		//  switch to collected weapon
		if (  g.autoswap == 1 && t.ws>0 ) 
		{
			//  insert as slot weapon
			t.weaponslot[t.ws].got=t.weaponindex;
			t.weaponslot[t.ws].invpos=t.weaponinvposition;
			g.autoloadgun=t.weaponindex;
			t.weaponkeyselection=t.ws;
			t.gotweapon=t.ws;
		}

		//  place details of weapon in slot
		if (  t.ws>0 ) 
		{
			//  insert as slot weapon
			t.weaponslot[t.ws].got=t.weaponindex;
			t.weaponslot[t.ws].invpos=t.weaponinvposition;
			t.gotweapon=t.ws;
			if (  t.gunid == 0 ) 
			{
				//  if no gun held, auto select collected
				g.autoloadgun=t.weaponindex;
				t.weaponkeyselection=t.ws;
			}
		}
	}

	//  weapons start with some ammo
	if (  t.gotweapon>0 ) 
	{
		t.tgunid=t.weaponslot[t.gotweapon].pref;
		if (  t.gun[t.tgunid].settings.weaponisammo == 0 ) 
		{
			if (  t.tqty == 0  )  t.tqty = 1;
			if (  t.weaponammo[t.gotweapon] == 0 && t.tweaponisnew == 1 ) 
			{
				//  provide some alternative ammo (weaponammo+10)
				if (  t.gun[t.tgunid].settings.modessharemags == 0 ) 
				{
					//  080415 - only if not sharing ammo
					t.taltqty=t.tqty;
					if (  t.taltqty>g.firemodes[t.tgunid][1].settings.reloadqty ) 
					{
						t.altpool=g.firemodes[t.tgunid][1].settings.poolindex;
						t.weaponammo[t.gotweapon+10]=g.firemodes[t.tgunid][1].settings.reloadqty;
						if (  t.altpool>0  )  t.ammopool[t.altpool].ammo = t.ammopool[t.altpool].ammo+(t.taltqty-g.firemodes[t.tgunid][1].settings.reloadqty); else t.weaponclipammo[t.gotweapon+10] = t.taltqty-g.firemodes[t.tgunid][1].settings.reloadqty;
					}
					else
					{
						if (  t.gun[t.tgunid].settings.addtospare == 0 ) 
						{
							t.weaponammo[t.gotweapon+10]=t.taltaty;
						}
						else
						{
							if (  t.gun[t.tgunid].settings.canaddtospare == 1 ) 
							{
								t.altpool=g.firemodes[t.tgunid][1].settings.poolindex;
								if (  t.altpool == 0  )  t.weaponclipammo[t.gotweapon+10] = t.weaponclipammo[t.gotweapon+10]+t.taltqty; else t.ammopool[t.altpool].ammo = t.ammopool[t.altpool].ammo+t.taltqty;
							}
							if (  t.gun[t.tgunid].settings.canaddtospare == 0  )  t.weaponammo[t.gotweapon+10] = t.taltqty;
						}
					}
				}
				//  provide some primary ammo
				if (  t.tqty>g.firemodes[t.tgunid][0].settings.reloadqty ) 
				{
					//  gun has MAX slots of ammo, cannot exceed this!
					t.tpool=g.firemodes[t.tgunid][0].settings.poolindex;
					t.weaponammo[t.gotweapon]=g.firemodes[t.tgunid][0].settings.reloadqty;
					if (  t.tpool>0 ) 
					{
						t.ammopool[t.tpool].ammo=t.ammopool[t.tpool].ammo+(t.tqty-g.firemodes[t.tgunid][0].settings.reloadqty);
					}
					else
					{
						t.weaponclipammo[t.gotweapon]=t.tqty-g.firemodes[t.tgunid][0].settings.reloadqty;
					}
				}
				else
				{
					if (  t.gun[t.tgunid].settings.addtospare == 0 ) 
					{
						t.weaponammo[t.gotweapon]=t.tqty;
					}
					else
					{
						//  new gunspec addition "addtospare" this will allow it so picking up ammo
						//  with an empty weapon won't add the ammo directly into the clip
						if (  t.gun[t.tgunid].settings.canaddtospare == 1 ) 
						{
							t.tpool=g.firemodes[t.tgunid][0].settings.poolindex;
							if (  t.tpool == 0 ) 
							{
								t.weaponclipammo[t.gotweapon]=t.weaponclipammo[t.gotweapon]+t.tqty;
							}
							else
							{
								t.ammopool[t.tpool].ammo=t.ammopool[t.tpool].ammo+t.tqty;
							}
						}
						if (  t.gun[t.tgunid].settings.canaddtospare == 0 ) 
						{
							t.gun[t.tgunid].settings.canaddtospare=1;
							t.weaponammo[t.gotweapon]=t.tqty;
						}
					}
				}
			}
			else
			{
				t.tpool=g.firemodes[t.tgunid][0].settings.poolindex;
				t.altpool=g.firemodes[t.tgunid][1].settings.poolindex;
				if (  t.tpool == 0  )  t.weaponclipammo[t.gotweapon] = t.weaponclipammo[t.gotweapon]+t.tqty; else t.ammopool[t.tpool].ammo = t.ammopool[t.tpool].ammo+t.tqty;
				if (  t.altpool == 0  )  t.weaponclipammo[t.gotweapon+10] = t.weaponclipammo[t.gotweapon+10]+t.taltqty; else t.ammopool[t.altpool].ammo = t.ammopool[t.altpool].ammo+t.taltqty;
			}
		}
	}

	//  refresh gun count
	physics_player_refreshcount ( );

	//  if collected weapon, and is empty, trigger reload if gun anim able
	if (  t.gotweapon>0 ) 
	{
		t.tgunid=t.weaponslot[t.gotweapon].pref;
		if (  t.weaponammo[t.gotweapon] == 0 ) 
		{
			if (  t.gunmode >= 5 && t.gunmode<31 ) 
			{
				t.gunmode=121;
			}
		}
	}

return;

}

void physics_player_removeweapon ( void )
{

	//  check all weapon slots
	for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
	{
		if (  t.weaponslot[t.ws].got == t.weaponindex  )  break;
	}
	if (  t.ws <= 10 ) 
	{
		//  Ensure gun is removed (if applicable)
		if (  t.gunid>0 && t.weaponslot[t.ws].got == t.gunid ) 
		{
			g.autoloadgun=0;
		}
		//  drop weapon from slot
		t.weaponslot[t.ws].got=0;
		t.weaponslot[t.ws].invpos=0;
	}

	//  refresh gun count
	physics_player_refreshcount ( );

return;

}

void physics_player_refreshcount ( void )
{

	//  refresh gun count
	t.guncollectedcount=0;
	for ( t.ws = 1 ; t.ws<=  10; t.ws++ )
	{
		if (  t.weaponslot[t.ws].got>0  )  ++t.guncollectedcount;
	}

return;
}

//----------------------------------------------------
//--- GAMEGURU - M-Weapon
//----------------------------------------------------

#include "gameguru.h"


//WEAPON SYSTEM Prototype


void weapon_getfreeobject ( void )
{
	//  sets tObjID to the next available object ID, or 0 if none available
	for ( t.obj = g.weaponsobjectoffset; t.obj <= g.weaponsobjectoffset + 499; t.obj++ )
	{
		if (  ObjectExist(t.obj)  ==  0 ) 
		{
			t.tObjID = t.obj ; return;
		}
	}
	t.tObjID = 0;
}

void weapon_getfreeimage ( void )
{
	//  sets tImgID to the next available image ID, or 0 if none available
	for ( t.timg = g.weaponsimageoffset ; t.timg <=  g.weaponsimageoffset + 299; t.timg++ )
	{
		if (  ImageExist(t.timg)  ==  0 ) 
		{
			t.tImgID = t.timg ; return;
		}
	}
	t.tImgID = 0;
}

void weapon_getfreesound ( void )
{
	//  sets tSndID to the next available sound ID, or 0 if none available
	for ( t.tsnd = g.projectilesoundoffset ; t.tsnd <=  g.projectilesoundoffset + 299; t.tsnd++ )
	{
		if (  SoundExist(t.tsnd)  ==  0 ) 
		{
			t.tSndID = t.tsnd ; return;
		}
	}
	t.tSndID = 0;
}

void weapon_loadobject ( void )
{
	//  loads in an object tFileName$, returns tObjID (0 if failed, or DBP ID of object)
	weapon_getfreeobject ( );
	if (  t.tObjID  ==  0  )  return;
	char pChopFile[512];
	strcpy ( pChopFile, t.tFileName_s.Get() );
	pChopFile[strlen(pChopFile)-2]=0;
	cstr pFileToLoad = cstr(pChopFile) + cstr(".DBO");
	if ( FileExist ( pFileToLoad.Get() ) == 0 ) 
	{
		pFileToLoad = t.tFileName_s;
		LoadObject ( pFileToLoad.Get(), t.tObjID );
	}
	else
	{
		LoadObject ( pFileToLoad.Get(), t.tObjID );
		EnsureObjectDBOIsFVF ( t.tObjID, pFileToLoad.Get(), 274 );
	}
	SetObjectTransparency ( t.tObjID, 6 );
	SetObjectEffect ( t.tObjID, g.guishadereffectindex );
	if (  ObjectExist(t.tObjID)  ==  0 ) 
	{
		t.tObjID = 0; return;
	}
}

void weapon_loadtexture ( void )
{
	//  loads in a texture tFileName$, return tImgID (0 if failed, or DBP ID of image)
	if (  FileExist(t.tFileName_s.Get())  ==  0 ) 
	{
		t.tImgID = 0; return;
	}
	weapon_getfreeimage ( );
	if (  t.tImgID  ==  0  )  return;
	LoadImage (  t.tFileName_s.Get(),t.tImgID );
	if (  ImageExist(t.tImgID)  ==  0 ) 
	{
		t.tImgID = 0; return;
	}
}

void weapon_loadtextfile ( void )
{
	//  loads in a Text ( file ready to be used with _weapon_readfield. Must UnDim (  the fileData$ array after use ) )
	//  tTextFile$, returns tResult 0 (not found) or 1 loaded
	if (  FileExist(t.tTextFile_s.Get()) == 0 ) 
	{
		t.tResult = 0 ; return;
	}
	Dim (  t.fileData_s,300  );
	LoadArray (  t.tTextFile_s.Get(),t.fileData_s );
	t.tResult = 1;
}

void weapon_readfield ( void )
{
	//  reads the file loaded by _weapon_loadtextfile and reads the field specified by tInField$
	//  takes tInField$, populates tResult, value$, value1#, value2#
	t.tInField_s = Lower(t.tInField_s.Get());
	for ( t.l = 0 ; t.l<=  299; t.l++ )
	{
		t.line_s=t.fileData_s[t.l];
		if (  Len(t.line_s.Get())>0 ) 
		{
			if (  t.line_s.Get()[0] != ';' )
			{

				//  take fieldname and value
				t.mid = 0;
				for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
				{
					if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1  ; break; }
				}

				//  make sure we found the = character before continuing
				if (  t.mid > 0 ) 
				{
					t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
					if (  t.field_s  ==  t.tInField_s ) 
					{
						t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));

						//  take value 1 and 2 from value$
						for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
						{
							if (  t.value_s.Get()[t.c] == ',' ) { t.mid = t.c+1  ; break; }
						}
						t.value1_f=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
						t.value2_f=ValF(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));
						t.tResult = 1 ; return;
					}
				}
			}
		}
	}
	t.value_s = "";
	t.value1_f = 0;
	t.value2_f = 0;
	t.tResult = 0;
}

void weapon_init ( void )
{
}

void weapon_free ( void )
{
}

void weapon_loop ( void )
{
}

void weapon_load ( void )
{
}

void weapon_setstate ( void )
{
}

void weapon_addanimation ( void )
{
	t.WeaponAnimation[t.tWeapon][t.tAnim].startFrame = t.tStartFrame;
	t.WeaponAnimation[t.tWeapon][t.tAnim].endFrame = t.tEndFrame;
	t.WeaponAnimation[t.tWeapon][t.tAnim].speed = t.tSpeed;
	t.WeaponAnimation[t.tWeapon][t.tAnim].loopFlag = t.tLoopFlag;
}

void weapon_processanimation ( void )
{
}

// projectile code

void weapon_projectile_init ( void )
{
	//  load and setup our tracer particle. All projectiles instance this particle and can display it if they are a tracer round
	t.tFileName_s = "gamecore\\projectiletypes\\TracerParticle.x"; weapon_loadobject ( );
	if (  t.tObjID > 0 ) 
	{
		g.weaponSystem.objTracer = t.tObjID;
		ExcludeOn (  t.tObjID );
	}

	// scan all projectiles in 'projectiletypes' folder
	SetDir ( "gamecore" );
	UnDim ( t.filelist_s );
	buildfilelist("projectiletypes","");
	SetDir ( ".." );
	if ( ArrayCount(t.filelist_s) > 0 ) 
	{
		for ( t.chkfile = 0 ; t.chkfile <= ArrayCount(t.filelist_s); t.chkfile++ )
		{
			t.file_s = t.filelist_s[t.chkfile];
			if ( t.file_s != "." && t.file_s != ".." ) 
			{
				if ( cstr(Lower(Right(t.file_s.Get(),4))) == ".txt" ) 
				{
					// get folder name only
					char pPathOnly[1024];
					strcpy ( pPathOnly, t.file_s.Get() );
					for ( int n = strlen(pPathOnly); n > 0; n-- )
					{
						if ( pPathOnly[n] == '\\' || pPathOnly[n] == '/' )
						{
							pPathOnly[n] = 0;
							break;
						}
					}
					t.tProjectileName_s = pPathOnly;
					weapon_projectile_load ( );
				}
			}
		}
	}
}

void weapon_getprojectileid ( void )
{
	t.tProjectileType = 0;
	for ( t.tProj = 1 ; t.tProj <= g.weaponSystem.numProjectiles; t.tProj++ )
	{
		t.tProjType = t.WeaponProjectile[t.tProj].baseType;
		if ( cstr(Lower(t.WeaponProjectileBase[t.tProjType].name_s.Get())) == t.tProjectileType_s ) 
		{
			t.tProjectileType=t.tProjType;
		}
	}
}

void NaughtNamedWeaponError ( char* error_s, int time )
{
	DrawToFront (  );
	Text (  10,10,error_s );
	Sync (  );
	SleepNow (  time );
}

void weapon_projectile_free ( void )
{
	//  delete all projectiles (hide and reset them)
	for ( t.tProj = 1 ; t.tProj <= g.weaponSystem.numProjectiles; t.tProj++ )
	{
		if (  t.WeaponProjectile[t.tProj].activeFlag  ==  1 ) 
		{
			weapon_projectile_destroy ( );
		}
		t.WeaponProjectile[t.tProj].sourceEntity=0;
	}
}

void weapon_projectile_loop ( void )
{
	//  processes all active projectiles
	t.tTimer = Timer();
	for ( t.tProj = 1 ; t.tProj <= g.weaponSystem.numProjectiles; t.tProj++ )
	{
		if (  t.WeaponProjectile[t.tProj].activeFlag  ==  1 ) 
		{
			t.tNewProj = t.tProj;

			//  get projectile base index
			t.tProjType = t.WeaponProjectile[t.tProj].baseType;
			t.tobj = t.WeaponProjectile[t.tProj].obj;

			//  save old position
			t.tXOldPos_f = t.WeaponProjectile[t.tProj].xPos_f;
			t.tYOldPos_f = t.WeaponProjectile[t.tProj].yPos_f;
			t.tZOldPos_f = t.WeaponProjectile[t.tProj].zPos_f;
			t.WeaponProjectile[t.tProj].xOldPos_f = t.tXOldPos_f;
			t.WeaponProjectile[t.tProj].yOldPos_f = t.tYOldPos_f;
			t.WeaponProjectile[t.tProj].zOldPos_f = t.tZOldPos_f;

			// projectile movement if not real physics
			if ( t.WeaponProjectileBase[t.tProjType].usingRealPhysics == 0 ) 
			{
				//  perform turning if projectile can turn (use tracer object for calculation simplicity)
				t.tXTurnSpeed_f = t.WeaponProjectile[t.tProj].xTurnSpeed_f;
				t.tYTurnSpeed_f = t.WeaponProjectile[t.tProj].yTurnSpeed_f;
				t.tZTurnSpeed_f = t.WeaponProjectile[t.tProj].zTurnSpeed_f;
				if (  t.tXTurnSpeed_f  !=  0 || t.tYTurnSpeed_f  !=  0 || t.tZTurnSpeed_f  !=  0 ) 
				{
					//  add to local rotations by speed
					t.WeaponProjectile[t.tProj].xTurn_f = t.WeaponProjectile[t.tProj].xTurn_f + t.tXTurnSpeed_f * t.ElapsedTime_f;
					t.WeaponProjectile[t.tProj].yTurn_f = t.WeaponProjectile[t.tProj].yTurn_f + t.tYTurnSpeed_f * t.ElapsedTime_f;
					t.WeaponProjectile[t.tProj].zTurn_f = t.WeaponProjectile[t.tProj].zTurn_f + t.tZTurnSpeed_f * t.ElapsedTime_f;
					//  rotate our tracer object
					t.tTracerObj = t.WeaponProjectile[t.tProj].tracerObj;
					PositionObject (  t.tTracerObj,t.tXOldPos_f,t.tYOldPos_f,t.tZOldPos_f );
					RotateObject (  t.tTracerObj,t.WeaponProjectile[t.tProj].xAng_f,t.WeaponProjectile[t.tProj].yAng_f,t.WeaponProjectile[t.tProj].zAng_f );
					RollObjectRight (  t.tTracerObj,t.WeaponProjectile[t.tProj].zTurn_f );
					TurnObjectRight (  t.tTracerObj,t.WeaponProjectile[t.tProj].yTurn_f );
					PitchObjectUp (  t.tTracerObj,t.WeaponProjectile[t.tProj].xTurn_f );
					//  move our tracer object forward so we can work out the new XYZ speed values
					t.tXSpd_f = t.WeaponProjectile[t.tProj].xSpeed_f;
					t.tYSpd_f = t.WeaponProjectile[t.tProj].ySpeed_f;
					t.tZSpd_f = t.WeaponProjectile[t.tProj].zSpeed_f;
					t.tTotalSpeed_f = Sqrt(t.tXSpd_f*t.tXSpd_f + t.tYSpd_f*t.tYSpd_f + t.tZSpd_f*t.tZSpd_f);
					MoveObject (  t.tTracerObj,t.tTotalSpeed_f );
					t.WeaponProjectile[t.tProj].xSpeed_f = ObjectPositionX(t.tTracerObj) - t.tXOldPos_f;
					t.WeaponProjectile[t.tProj].ySpeed_f = ObjectPositionY(t.tTracerObj) - t.tYOldPos_f;
					t.WeaponProjectile[t.tProj].zSpeed_f = ObjectPositionZ(t.tTracerObj) - t.tZOldPos_f;
					//  can scale projectile based on power of player
					if (  t.playercontrol.thirdperson.enabled == 1 ) 
					{
						t.tscaleme_f=t.player[1].powers.level;
						ScaleObject (  t.tobj,t.tscaleme_f,t.tscaleme_f,t.tscaleme_f );
					}
				}

				//  work out the new position using our speed and elapsed time
				t.tXNewPos_f = t.tXOldPos_f + t.WeaponProjectile[t.tProj].xSpeed_f * t.ElapsedTime_f;
				t.tYNewPos_f = t.tYOldPos_f + t.WeaponProjectile[t.tProj].ySpeed_f * t.ElapsedTime_f;
				t.tZNewPos_f = t.tZOldPos_f + t.WeaponProjectile[t.tProj].zSpeed_f * t.ElapsedTime_f;
				t.WeaponProjectile[t.tProj].xPos_f = t.tXNewPos_f;
				t.WeaponProjectile[t.tProj].yPos_f = t.tYNewPos_f;
				t.WeaponProjectile[t.tProj].zPos_f = t.tZNewPos_f;
			}
			else
			{
				//  get latest position from actual physics object
				t.tXNewPos_f = ObjectPositionX(t.tobj);
				t.tYNewPos_f = ObjectPositionY(t.tobj);
				t.tZNewPos_f = ObjectPositionZ(t.tobj);
				t.WeaponProjectile[t.tProj].xPos_f = t.tXNewPos_f;
				t.WeaponProjectile[t.tProj].yPos_f = t.tYNewPos_f;
				t.WeaponProjectile[t.tProj].zPos_f = t.tZNewPos_f;
			}

			//  create dynamic light effect from all projectile (uses last one)
			if ( t.WeaponProjectile[t.tProj].sourceEntity == 0 ) 
			{
				if ( t.tTimer > t.WeaponProjectile[t.tProj].createStamp + 100 ) 
				{
					if ( t.WeaponProjectile[t.tProj].usespotlighting != 0 ) 
					{
						//  only start dynamic light a split second after launch (so protagonist rear does not light up)
						t.tx_f=t.tXNewPos_f ; t.ty_f=t.tYNewPos_f ; t.tz_f=t.tZNewPos_f ; t.tmodeindex=t.tProj;
						lighting_spotflashtracking ( );
					}
				}
			}

			//  has this projectile exceeded its life?
			t.tthisprojectileexploded=0;
			if (  t.tTimer > t.WeaponProjectile[t.tProj].createStamp + t.WeaponProjectileBase[t.tProjType].life ) 
			{
				//  run out of fuel
				if ( t.WeaponProjectileBase[t.tProjType].projectileEventType > 0 ) 
					t.tProjectileName_s = t.WeaponProjectileBase[t.tProjType].name_s;
				else
					t.tProjectileName_s = "";
				t.tProjectileResult = WEAPON_PROJECTILERESULT_EXPLODE;
				if ( t.tProjectileResult == 2 && t.WeaponProjectileBase[t.tProjType].explosionType == 99 ) 
				{
					t.tProjectileResult = WEAPON_PROJECTILERESULT_CUSTOM;
					t.tProjectileResultExplosionImageID = t.WeaponProjectileBase[t.tProjType].explosionImageID;
					t.tProjectileResultLightFlag = t.WeaponProjectileBase[t.tProjType].explosionLightFlag;
					t.tProjectileResultSmokeImageID = t.WeaponProjectileBase[t.tProjType].explosionSmokeImageID;
					t.tProjectileResultSparksCount = t.WeaponProjectileBase[t.tProjType].explosionSparksCount;
					t.tProjectileResultSize = t.WeaponProjectileBase[t.tProjType].explosionSize;
					t.tProjectileResultSmokeSize = t.WeaponProjectileBase[t.tProjType].explosionSmokeSize;
					t.tProjectileResultSparksSize = t.WeaponProjectileBase[t.tProjType].explosionSparksSize;
				}
				t.tx_f = t.tXNewPos_f; t.ty_f = t.tYNewPos_f; t.tz_f = t.tZNewPos_f;
				t.tDamage_f = t.WeaponProjectileBase[t.tProjType].damage_f;
				t.tradius_f = t.WeaponProjectileBase[t.tProjType].damageRadius_f;
				t.tSoundID = t.WeaponProjectile[t.tProj].soundDeath;
				t.tSourceEntity = t.WeaponProjectile[t.tProj].sourceEntity;
				t.tHitObj = 0;
				weapon_projectileresult_make ( );
				weapon_projectile_destroy ( );
				t.tthisprojectileexploded=1;

				if (  t.game.runasmultiplayer == 1 ) 
				{
					t.tSteamBulletOn = 0;
					if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
					{
						mp_update_projectile ( );
					}
				}
			}
			else
			{
				//  detect if projectile hits the player camera
				t.tdx_f=CameraPositionX(0)-t.tXNewPos_f;
				t.tdy_f=CameraPositionY(0)-t.tYNewPos_f;
				t.tdz_f=CameraPositionZ(0)-t.tZNewPos_f;
				t.tdd_f=t.tdx_f*t.tdx_f + t.tdy_f*t.tdy_f + t.tdz_f*t.tdz_f;
				if (  t.tdd_f<50.0*50.0 && t.playercontrol.thirdperson.enabled == 0 ) 
				{
					//  if close enough to player, and not fired by player, explode!
					if (  t.WeaponProjectile[t.tNewProj].sourceEntity>0 ) 
					{
						t.tHitX_f = t.tXOldPos_f;
						t.tHitY_f = t.tYOldPos_f;
						t.tHitZ_f = t.tZOldPos_f;
						if ( t.WeaponProjectileBase[t.tProjType].projectileEventType > 0 ) 
							t.tProjectileName_s = t.WeaponProjectileBase[t.tProjType].name_s;
						else
							t.tProjectileName_s = "";
						t.tProjectileResult = WEAPON_PROJECTILERESULT_EXPLODE;
						if ( t.tProjectileResult == 2 && t.WeaponProjectileBase[t.tProjType].explosionType == 99 ) 
						{
							t.tProjectileResult = WEAPON_PROJECTILERESULT_CUSTOM;
							t.tProjectileResultExplosionImageID = t.WeaponProjectileBase[t.tProjType].explosionImageID;
							t.tProjectileResultLightFlag = t.WeaponProjectileBase[t.tProjType].explosionLightFlag;
							t.tProjectileResultSmokeImageID = t.WeaponProjectileBase[t.tProjType].explosionSmokeImageID;
							t.tProjectileResultSparksCount = t.WeaponProjectileBase[t.tProjType].explosionSparksCount;
							t.tProjectileResultSize = t.WeaponProjectileBase[t.tProjType].explosionSize;
							t.tProjectileResultSmokeSize = t.WeaponProjectileBase[t.tProjType].explosionSmokeSize;
							t.tProjectileResultSparksSize = t.WeaponProjectileBase[t.tProjType].explosionSparksSize;
						}
						t.tx_f = t.tHitX_f; t.ty_f = t.tHitY_f; t.tz_f = t.tHitZ_f;
						t.tDamage_f = t.WeaponProjectileBase[t.tProjType].damage_f;
						t.tradius_f = t.WeaponProjectileBase[t.tProjType].damageRadius_f;
						t.tSoundID = t.WeaponProjectile[t.tProj].soundDeath;
						t.tSourceEntity = t.WeaponProjectile[t.tProj].sourceEntity;
						t.tHitObj = 0;
						weapon_projectileresult_make ( );
						weapon_projectile_destroy ( );
						t.tthisprojectileexploded=1;

						if (  t.game.runasmultiplayer == 1 ) 
						{
							t.tSteamBulletOn = 0;
							if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
							{
								mp_update_projectile ( );
							}
						}
					}
				}
				else
				{
					//  has this projectile hit the ground?
					if (  ODERayTerrain(t.tXOldPos_f,t.tYOldPos_f,t.tZOldPos_f,t.tXNewPos_f,t.tYNewPos_f,t.tZNewPos_f) == 1 ) 
					{
						//  hit the Floor (  )
						if (  t.WeaponProjectileBase[t.tProjType].resultBounce>0 ) 
						{
							t.tHitX_f=ODEGetRayCollisionX();
							t.tHitY_f=ODEGetRayCollisionY();
							t.tHitZ_f=ODEGetRayCollisionZ();
							if ( t.WeaponProjectileBase[t.tProjType].projectileEventType > 0 ) 
								t.tProjectileName_s = t.WeaponProjectileBase[t.tProjType].name_s;
							else
								t.tProjectileName_s = "";
							t.tProjectileResult = t.WeaponProjectileBase[t.tProjType].resultBounce;
							if ( t.tProjectileResult == 2 && t.WeaponProjectileBase[t.tProjType].explosionType == 99 ) 
							{
								t.tProjectileResult = WEAPON_PROJECTILERESULT_CUSTOM;
								t.tProjectileResultExplosionImageID = t.WeaponProjectileBase[t.tProjType].explosionImageID;
								t.tProjectileResultLightFlag = t.WeaponProjectileBase[t.tProjType].explosionLightFlag;
								t.tProjectileResultSmokeImageID = t.WeaponProjectileBase[t.tProjType].explosionSmokeImageID;
								t.tProjectileResultSparksCount = t.WeaponProjectileBase[t.tProjType].explosionSparksCount;
								t.tProjectileResultSize = t.WeaponProjectileBase[t.tProjType].explosionSize;
								t.tProjectileResultSmokeSize = t.WeaponProjectileBase[t.tProjType].explosionSmokeSize;
								t.tProjectileResultSparksSize = t.WeaponProjectileBase[t.tProjType].explosionSparksSize;
							}
							t.tx_f = t.tHitX_f; t.ty_f = t.tHitY_f; t.tz_f = t.tHitZ_f;
							t.tDamage_f = t.WeaponProjectileBase[t.tProjType].damage_f;
							t.tradius_f = t.WeaponProjectileBase[t.tProjType].damageRadius_f;
							t.tSourceEntity = t.WeaponProjectile[t.tProj].sourceEntity;
							t.tHitObj = 0;
							weapon_projectileresult_make ( );
							weapon_projectile_destroy ( );
							t.tthisprojectileexploded=1;

							if (  t.game.runasmultiplayer == 1 ) 
							{
								t.tSteamBulletOn = 0;
								if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
								{
									mp_update_projectile ( );
								}
							}
						}
					}
					else
					{
						//  has this projectile gone underwater?
						if (  t.tYNewPos_f < t.terrain.waterliney_f ) 
						{
							//  yes it has
							t.tXPos_f = t.tXNewPos_f; t.tZPos_f = t.tZNewPos_f ; t.tsize_f = 1.5;
							weapon_projectile_destroy ( );
							//  trigger splash
							g.decalx = t.tXNewPos_f ; g.decaly = t.terrain.waterliney_f + 1; g.decalz = t.tZNewPos_f; t.tInScale_f = 3;
							decal_triggerwatersplash ( );
							//  trigger sound
							t.tmatindex=17 ; t.tsoundtrigger=t.material[t.tmatindex].impactid;
							t.tspd_f=(t.material[t.tmatindex].freq*1.0)+Rnd(t.material[t.tmatindex].freq)*0.5;
							t.tsx_f=t.tXNewPos_f ; t.tsy_f=t.terrain.waterliney_f ; t.tsz_f=t.tZNewPos_f;
							t.tvol_f = 100.0f ; material_triggersound ( 0 );
							t.tthisprojectileexploded=1;

							if (  t.game.runasmultiplayer == 1 ) 
							{
								t.tSteamBulletOn = 0;
								if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
								{
									mp_update_projectile ( );
								}
							}
						}
						else
						{
							//  check for hit with entities
							t.ttdx_f=t.tXNewPos_f-t.tXOldPos_f;
							t.ttdy_f=t.tYNewPos_f-t.tYOldPos_f;
							t.ttdz_f=t.tZNewPos_f-t.tZOldPos_f;
							t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdy_f*t.ttdy_f)+abs(t.ttdz_f*t.ttdz_f));
							t.ttdx_f=t.ttdx_f/t.ttdd_f;
							t.ttdy_f=t.ttdy_f/t.ttdd_f;
							t.ttdz_f=t.ttdz_f/t.ttdd_f;
							t.tXOldPos_f=t.tXOldPos_f-(t.ttdx_f*60.0);
							t.tYOldPos_f=t.tYOldPos_f-(t.ttdy_f*60.0);
							t.tZOldPos_f=t.tZOldPos_f-(t.ttdz_f*60.0);
							//  ignore the entity object which fires it
							t.tIgnoreObject = 0;
							if (  t.WeaponProjectile[t.tProj].sourceEntity>0  )  t.tIgnoreObject  =  t.entityelement[t.WeaponProjectile[t.tProj].sourceEntity].obj;
							t.tEndEntity = g.entityviewstartobj+g.entityelementlist;
							//Dave - don't need to do raycasts for the nade hitting anything as it is controlled by physics
							if ( t.WeaponProjectileBase[t.tProjType].resultBounce>0 ) 
							{
								if (g.lightmappedobjectoffset >= g.lightmappedobjectoffsetfinish)
									t.ttt = IntersectAll(85000, 85000 + g.merged_new_objects - 1, 0, 0, 0, 0, 0, 0, -123);
								else
									t.ttt=IntersectAll(g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,0,0,0,0,0,0,-123);
								t.tHitObj=IntersectAll(g.entityviewstartobj,g.entityviewendobj,t.tXOldPos_f,t.tYOldPos_f,t.tZOldPos_f,t.tXNewPos_f,t.tYNewPos_f,t.tZNewPos_f,t.tIgnoreObject);
							}
							else
								t.tHitObj = 0;

							// 291015 - in addition, use a distance check to explode projectiles when they are close to enemies
							int iIgnoreThirdPersonEntityCharacter = -1;
							if (  t.playercontrol.thirdperson.enabled == 1 ) 
								iIgnoreThirdPersonEntityCharacter = t.playercontrol.thirdperson.charactere;

							bool bDirectHitOnObjectItself = false;
							for ( t.ttte = 1; t.ttte <= g.entityelementlist; t.ttte++ )
							{
								t.tentid=t.entityelement[t.ttte].bankindex;
								if (  t.entityprofile[t.tentid].ischaracter == 1 ) 
								{
									if (  t.tentid > 0 && t.entityelement[t.ttte].health > 0 && t.ttte != iIgnoreThirdPersonEntityCharacter ) 
									{
										int tobj =  t.entityelement[t.ttte].obj;
										if ( tobj != t.tIgnoreObject )
										{
											float fCharacterMidrift = GetObjectCollisionCenterY(tobj);
											t.tdx_f = t.entityelement[t.ttte].x - t.tXNewPos_f;
											t.tdy_f = (t.entityelement[t.ttte].y + fCharacterMidrift) - t.tYNewPos_f;
											t.tdz_f = t.entityelement[t.ttte].z - t.tZNewPos_f;
											float fDist = sqrt( (t.tdx_f*t.tdx_f)+(t.tdy_f*t.tdy_f)+(t.tdz_f*t.tdz_f) );
											if ( fDist < 40.0f )
											{
												fDist = sqrt( (t.tdx_f*t.tdx_f)+(t.tdz_f*t.tdz_f) );
												if ( fDist < 30.0f )
												{
													// projectile close enough to enemy to cause a hit
													t.tHitObj = t.entityelement[t.ttte].obj;
													bDirectHitOnObjectItself = true;
												}
											}
										}
									}
								}
							}


							//  if third person, ignore the protagonist (but only if this was PLAYERS PROJECTILE)
							if (  t.playercontrol.thirdperson.enabled == 1 ) 
							{
								t.tpersone=t.playercontrol.thirdperson.charactere;
								if (  t.tpersone>0 ) 
								{
									t.tpersonobj=t.entityelement[t.tpersone].obj;
									if (  ObjectExist(t.tpersonobj) == 1 && t.tpersonobj == t.tHitObj ) 
									{
										if (  t.WeaponProjectile[t.tNewProj].sourceEntity == 0 ) 
										{
											t.tHitObj=0;
										}
									}
								}
							}

							//  if confirmed, bang!
							if (  t.tHitObj>0 ) 
							{

								//  only explode if flagged to do so on a hit
								t.tsteamLastHit = t.tHitObj;
								if (  t.WeaponProjectileBase[t.tProjType].resultBounce>0 ) 
								{
									// 091115 - direct it or an intersect coordinate
									if ( bDirectHitOnObjectItself==true )
									{
										// make hit coordinate just in front of enemy towards player camera
										t.tHitX_f = ObjectPositionX(t.tHitObj);
										t.tHitY_f = ObjectPositionY(t.tHitObj);
										t.tHitZ_f = ObjectPositionZ(t.tHitObj);
										float fDX = CameraPositionX() - t.tHitX_f;
										float fDZ = CameraPositionZ() - t.tHitZ_f;
										float fDist = sqrt( (fDX*fDX) + (fDZ*fDZ) );
										fDX /= fDist;
										fDZ /= fDist;
										t.tHitX_f += (fDX*10.0f);
										t.tHitZ_f += (fDZ*10.0f);
									}
									else
									{
										t.tHitX_f=ChecklistFValueA(6);
										t.tHitY_f=ChecklistFValueB(6);
										t.tHitZ_f=ChecklistFValueC(6);
									}
									if ( t.WeaponProjectileBase[t.tProjType].projectileEventType > 0 ) 
										t.tProjectileName_s = t.WeaponProjectileBase[t.tProjType].name_s;
									else
										t.tProjectileName_s = "";
									t.tProjectileResult = t.WeaponProjectileBase[t.tProjType].resultBounce;
									if ( t.tProjectileResult == 2 && t.WeaponProjectileBase[t.tProjType].explosionType == 99 ) 
									{
										t.tProjectileResult = WEAPON_PROJECTILERESULT_CUSTOM;
										t.tProjectileResultExplosionImageID = t.WeaponProjectileBase[t.tProjType].explosionImageID;
										t.tProjectileResultLightFlag = t.WeaponProjectileBase[t.tProjType].explosionLightFlag;
										t.tProjectileResultSmokeImageID = t.WeaponProjectileBase[t.tProjType].explosionSmokeImageID;
										t.tProjectileResultSparksCount = t.WeaponProjectileBase[t.tProjType].explosionSparksCount;
										t.tProjectileResultSize = t.WeaponProjectileBase[t.tProjType].explosionSize;
										t.tProjectileResultSmokeSize = t.WeaponProjectileBase[t.tProjType].explosionSmokeSize;
										t.tProjectileResultSparksSize = t.WeaponProjectileBase[t.tProjType].explosionSparksSize;
									}
									t.tx_f = t.tHitX_f; t.ty_f = t.tHitY_f; t.tz_f = t.tHitZ_f;
									t.tDamage_f = t.WeaponProjectileBase[t.tProjType].damage_f;
									t.tradius_f = t.WeaponProjectileBase[t.tProjType].damageRadius_f;
									t.tSoundID = t.WeaponProjectile[t.tProj].soundDeath;
									t.tSourceEntity = t.WeaponProjectile[t.tProj].sourceEntity;
									weapon_projectileresult_make ( );
									weapon_projectile_destroy ( );
									t.tthisprojectileexploded=1;
									if (  t.game.runasmultiplayer == 1 ) 
									{
										t.tSteamBulletOn = 0;
										if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
										{
											mp_update_projectile ( );
										}
									}
								}
							}
						}
					}
				}
			}

			//  if projectile has not been exploded/removed
			if (  t.tthisprojectileexploded == 0 ) 
			{
				//  update physical object if one exists (and projectile movement if not real physics)
				if (  t.WeaponProjectileBase[t.tProjType].usingRealPhysics == 0 ) 
				{
					if (  t.tobj > 0 ) 
					{
						PositionObject (  t.tobj,t.WeaponProjectile[t.tProj].xPos_f,t.WeaponProjectile[t.tProj].yPos_f,t.WeaponProjectile[t.tProj].zPos_f );
						RotateObject (  t.tobj,t.WeaponProjectile[t.tProj].xAng_f,t.WeaponProjectile[t.tProj].yAng_f,t.WeaponProjectile[t.tProj].zAng_f );
					}
				}

				//  tracer logic
				if (  t.WeaponProjectile[t.tProj].tracerFlag  ==  1 ) 
				{
					ShowObject (  t.WeaponProjectile[t.tProj].tracerObj );
				}

				//  do we need to reposition the 3D sound?
				t.tSndID = t.WeaponProjectile[t.tProj].sound;
				if (  t.tSndID > 0 ) 
				{
					if (  t.WeaponProjectileBase[t.tProjType].soundLoopFlag  ==  1 ) 
					{
						//  position the sound
						PositionSound (  t.tSndID,t.tXNewPos_f,t.tYNewPos_f,t.tZNewPos_f );
						//  calculate and set doppler pitch
						if (  t.WeaponProjectileBase[t.tProjType].soundDopplerFlag  ==  1 ) 
						{
							t.tOldDist_f = t.WeaponProjectile[t.tProj].soundDistFromPlayer_f;
							weapon_projectile_setDistFromPlayer ( );
							t.tDistDiff_f = t.tOldDist_f - t.WeaponProjectile[t.tProj].soundDistFromPlayer_f;
							t.tSoundMultiplier_f = 1 + (t.tDistDiff_f/t.ElapsedTime_f)*0.00015;
							if (  t.tSoundMultiplier_f < 0.5  )  t.tSoundMultiplier_f  =  0.5;
							if (  t.tSoundMultiplier_f > 2  )  t.tSoundMultiplier_f  =  2;
							t.tsoundfreq=t.WeaponProjectileBase[t.tProjType].soundDopplerBaseSpeed * t.tSoundMultiplier_f;
							if (  t.tsoundfreq<10000  )  t.tsoundfreq = 10000;
							if (  t.tsoundfreq>60000  )  t.tsoundfreq = 60000;
							SetSoundSpeed (  t.tSndID,t.tsoundfreq );
						}
					}
				}

				if (  t.game.runasmultiplayer == 1 ) 
				{
					if (  t.WeaponProjectile[t.tNewProj].sourceEntity  ==  0 ) 
					{
						t.WeaponProjectile[t.tProj].xAng_f = ObjectAngleX(t.tobj);
						t.WeaponProjectile[t.tProj].yAng_f = ObjectAngleY(t.tobj);
						t.WeaponProjectile[t.tProj].zAng_f = ObjectAngleZ(t.tobj);
						t.tSteamBulletOn = 1;
						mp_update_projectile ( );
					}
				}
			}
		}
	}
}

void weapon_projectile_load ( void )
{
	//  add a projectile type to the system. Takes; tProjectileName$
	//  returns tResult, which is 0 (failed) or the index in WeaponProjectileBase() array

	// three projectiles are considered defaults, so can pass in a short hand version 
	cstr tProjectileFolder_s = t.tProjectileName_s + "\\";
	if ( strcmp ( Lower(t.tProjectileName_s.Get()), "modern\\handgrenade" ) == NULL ) t.tProjectileName_s = "handgrenade";
	if ( strcmp ( Lower(t.tProjectileName_s.Get()), "modern\\rpggrenade" ) == NULL ) t.tProjectileName_s = "rpggrenade";
	if ( strcmp ( Lower(t.tProjectileName_s.Get()), "fantasy\\fireball" ) == NULL ) t.tProjectileName_s = "fireball";

	// get name only
	cstr tProjectileNameOnly_s = t.tProjectileName_s;
	LPSTR pProjectileFolderName = t.tProjectileName_s.Get();
	for ( int n = strlen(pProjectileFolderName)-1; n > 0; n-- )
	{
		if ( pProjectileFolderName[n] == '\\' )
		{
			tProjectileNameOnly_s = cstr(pProjectileFolderName+n+1);
			break;
		}
	}

	//  find a free slot to load this definition in, or quit if it already exists and return the ID
	t.tNewProjBase = 0;
	for ( t.tP = 1 ; t.tP <= g.weaponSystem.numProjectileBases; t.tP++ )
	{
		if (  t.WeaponProjectileBase[t.tP].name_s == t.tProjectileName_s ) 
		{
			t.tResult = t.tP ; return;
		}
		else
		{
			if (  t.WeaponProjectileBase[t.tP].activeFlag  ==  0 ) 
			{
				t.tNewProjBase = t.tP;
				break;
			}
		}
	}

	//  if tNewProjBase is still zero, we didn't have enough slots
	if ( t.tNewProjBase == 0 ) 
	{
		t.tResult = 0; 
		return;
	}

	//  load in our Text (  file )
	t.tTextFile_s=cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + tProjectileNameOnly_s + ".txt";
	weapon_loadtextfile ( );
	if (  t.tResult  ==  0 ) 
	{
		t.tResult = 0; return;
	}

	//  read Text (  file and setup our projectile values
	t.WeaponProjectileBase[t.tNewProjBase].name_s = t.tProjectileName_s;
	t.tInField_s = "mode" ; weapon_readfield() ; t.WeaponProjectileBase[t.tNewProjBase].mode=t.value1_f;
	t.tInField_s = "scaleMinX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMinX_f = t.value1_f;
	t.tInField_s = "scaleMaxX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMaxX_f = t.value1_f ;
	t.tInField_s = "scaleMinY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMinY_f = t.value1_f ;
	t.tInField_s = "scaleMaxY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMaxY_f = t.value1_f ;
	t.tInField_s = "scaleMinZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMinZ_f = t.value1_f ;
	t.tInField_s = "scaleMaxZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].baseObjScaleMaxZ_f = t.value1_f ;
	t.tInField_s = "avoidPlayerPenetration" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].avoidPlayerPenetration_f = t.value1_f ;
	t.tInField_s = "attachToWeaponLimb" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].attachToWeaponLimb = t.value1_f ;
	t.tInField_s = "accuracyX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].xAccuracy_f = t.value1_f ;
	t.tInField_s = "accuracyY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].yAccuracy_f = t.value1_f ;
	t.tInField_s = "life" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].life = t.value1_f ;
	t.tInField_s = "resultEndOfLife" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultEndOfLife = t.value1_f ;
	t.tInField_s = "damage" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].damage_f = t.value1_f ;
	t.tInField_s = "damageRandom" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].damageRandom_f = t.value1_f ;
	t.tInField_s = "range" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].range_f = t.value1_f ;
	t.tInField_s = "fullDamageRange" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].fullDamageRange_f = t.value1_f ;
	t.tInField_s = "damageRadius" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].damageRadius_f = t.value1_f ;
	t.tInField_s = "resultIfDamaged" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultIfDamaged = t.value1_f ;
	t.tInField_s = "resultIfDamagedDelay" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultIfDamagedDelay = t.value1_f ;
	t.tInField_s = "resultOnInterval" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultOnInterval = t.value1_f ;
	t.tInField_s = "resultOnIntervalTime" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultOnIntervalTime = t.value1_f ;
	t.tInField_s = "resultBounce" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].resultBounce = t.value1_f ;
	t.tInField_s = "thrustTime" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].thrustTime = t.value1_f ;
	t.tInField_s = "thrustDelay" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].thrustDelay = t.value1_f ;
	t.tInField_s = "speedMin" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedMin_f = t.value1_f ;
	t.tInField_s = "speedMax" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedMax_f = t.value1_f ;
	t.tInField_s = "speedAngMinX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMinX_f = t.value1_f ;
	t.tInField_s = "speedAngMaxX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMaxX_f = t.value1_f ;
	t.tInField_s = "speedAngMinY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMinY_f = t.value1_f ;
	t.tInField_s = "speedAngMaxY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMaxY_f = t.value1_f ;
	t.tInField_s = "speedAngMinZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMinZ_f = t.value1_f ;
	t.tInField_s = "speedAngMaxZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedAngMaxZ_f = t.value1_f ;
	t.tInField_s = "speedTurnMinX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMinX_f = t.value1_f ;
	t.tInField_s = "speedTurnMaxX" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMaxX_f = t.value1_f ;
	t.tInField_s = "speedTurnMinY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMinY_f = t.value1_f ;
	t.tInField_s = "speedTurnMaxY" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMaxY_f = t.value1_f ;
	t.tInField_s = "speedTurnMinZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMinZ_f = t.value1_f ;
	t.tInField_s = "speedTurnMaxZ" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].speedTurnMaxZ_f = t.value1_f ;
	t.tInField_s = "usingRealPhysics" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].usingRealPhysics = t.value1_f ;
	t.tInField_s = "gravityModifier" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].gravityModifier_f = t.value1_f ;
	t.tInField_s = "airFrictionModifier" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].airFrictionModifier_f = t.value1_f ;
	t.tInField_s = "groundFrictionModifier" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].groundFrictionModifier_f = t.value1_f ;
	t.tInField_s = "thrustModifier" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].thrustModifier_f = t.value1_f ;
	t.tInField_s = "bounceFlag" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].bounceFlag = t.value1_f ;
	t.tInField_s = "bounceModifier" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].bounceModifier_f = t.value1_f ;
	t.tInField_s = "bounceResult" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].bounceResult = t.value1_f ;
	t.tInField_s = "soundLoopFlag" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].soundLoopFlag = t.value1_f ;
	t.tInField_s = "soundDopplerFlag" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].soundDopplerFlag = t.value1_f ;
	t.tInField_s = "soundDopplerBaseSpeed" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].soundDopplerBaseSpeed = t.value1_f ;
	t.tInField_s = "soundInterval" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].soundInterval = t.value1_f ;
	t.tInField_s = "overridespotlighting" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].overridespotlighting = t.value1_f ;
	t.tInField_s = "noZWrite" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].noZWrite = t.value1_f ;
	
	t.tInField_s = "particleType" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].particleType = t.value1_f;
	t.tInField_s = "particleName" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].particleName = t.value_s;
	t.tInField_s = "explosionType" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionType = t.value1_f;
	t.tInField_s = "explosionName" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionName = t.value_s;
	t.tInField_s = "explosionLightFlag" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionLightFlag = t.value1_f;
	t.tInField_s = "explosionSmokeName" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeName = t.value_s;
	t.tInField_s = "explosionSparksCount" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionSparksCount = t.value1_f;
	t.tInField_s = "explosionSize" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionSize = t.value1_f;
	t.tInField_s = "explosionSmokeSize" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeSize = t.value1_f;
	t.tInField_s = "explosionSparksSize" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].explosionSparksSize = t.value1_f;
	t.tInField_s = "projectileEventType" ; weapon_readfield(); t.WeaponProjectileBase[t.tNewProjBase].projectileEventType = t.value1_f;

	t.tInField_s = "object" ; weapon_readfield ( );
	t.tFileName_s=cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s;

	weapon_loadobject ( );
	if (  t.tObjID  ==  0 ) 
	{
		t.tResult = 0; return;
	}
	t.WeaponProjectileBase[t.tNewProjBase].baseObj = t.tObjID;

	// load particle trail image ID here when particle loaded
	t.WeaponProjectileBase[t.tNewProjBase].particleImageID = 0;
	if ( t.WeaponProjectileBase[t.tNewProjBase].particleType == 99 )
	{
		t.tFileName_s = cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.WeaponProjectileBase[t.tNewProjBase].particleName;
		t.WeaponProjectileBase[t.tNewProjBase].particleImageID = loadinternalimage ( t.tFileName_s.Get() );
		if ( t.WeaponProjectileBase[t.tNewProjBase].particleImageID == 0 )
		{
			timestampactivity ( 0, "Could not find particle image file specified by particleName" );
			t.WeaponProjectileBase[t.tNewProjBase].particleType = 0;
		}
	}

	// load explosion and smoke image ID here when particle loaded
	t.WeaponProjectileBase[t.tNewProjBase].explosionImageID = 0;
	t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeImageID = -1;
	if ( t.WeaponProjectileBase[t.tNewProjBase].explosionType == 99 )
	{
		// explosion
		t.tFileName_s = cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.WeaponProjectileBase[t.tNewProjBase].explosionName;
		t.WeaponProjectileBase[t.tNewProjBase].explosionImageID = loadinternalimage ( t.tFileName_s.Get() );

		// optional smoke
		t.tFileName_s = cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeName;
		t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeImageID = loadinternalimage ( t.tFileName_s.Get() );
		if ( t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeImageID == 0 ) 
		{
			// use default smoke if non specified
			t.WeaponProjectileBase[t.tNewProjBase].explosionSmokeImageID = -1;
		}
	}

	// load diffuse texture (if any)
	t.tInField_s = "textureD" ; weapon_readfield ( );
	if ( strlen ( t.value_s.Get() ) > 4 )
	{
		t.tFileName_s=cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s;
		weapon_loadtexture ( );
		if (  t.tImgID  ==  0 ) 
		{
			t.tResult = 0; return;
		}
		t.WeaponProjectileBase[t.tNewProjBase].textureD = t.tImgID;

		//  load normal texture
		t.tInField_s = "textureN" ; weapon_readfield ( );
		t.tFileName_s=cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s;
		weapon_loadtexture ( );
		if (  t.tImgID  ==  0 ) 
		{
			t.tResult = 0; return;
		}
		t.WeaponProjectileBase[t.tNewProjBase].textureN = t.tImgID;

		//  load specular texture
		t.tInField_s = "textureS" ; weapon_readfield ( );
		t.tFileName_s=cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s;
		weapon_loadtexture ( );
		if (  t.tImgID  ==  0 ) 
		{
			t.tResult = 0; return;
		}
		t.WeaponProjectileBase[t.tNewProjBase].textureS = t.tImgID;
		TextureObject ( t.tObjID, 0, t.WeaponProjectileBase[t.tNewProjBase].textureD );
	}
	else
	{
		// no TEXTURED field assumes it to be a PBR with built-in texture references (for PBR set)
		// just need to add the detail map and cube map
		int texlid = loadinternaltextureex("effectbank\\reloaded\\media\\detail_default.dds", 1, t.tfullorhalfdivide);
		TextureObject ( t.tObjID, 7, texlid );
		TextureObject ( t.tObjID, 6, t.terrain.imagestartindex+31 );
	}
	ExcludeOn (  t.tObjID );

	// load shader (if specified)
	t.teffectid=0;
	t.tInField_s = "effect" ; weapon_readfield ( );
	if ( Len(t.value_s.Get())<=2 ) 
	{
		SetObjectEffect ( t.tObjID, g.decaleffectoffset );
	}
	else
	{
		t.tFileName_s=cstr("effectbank\\reloaded\\") + t.value_s;
		t.teffectid=loadinternaleffect(t.tFileName_s.Get());
		SetObjectEffect (  t.tObjID,t.teffectid );
	}
	t.WeaponProjectileBase[t.tNewProjBase].effectid = t.teffectid;

	// apply no Zwrite if projectile flagged it
	if ( t.WeaponProjectileBase[t.tNewProjBase].noZWrite == 1 )
	{
		DisableObjectZWrite ( t.tObjID );
	}
	SetObjectTransparency ( t.tObjID, 6 );

	// load sounds
	t.tInField_s = "sound" ; weapon_readfield(); t.sound_s = t.value_s;
	if (  t.value_s  !=  "" ) 
	{
		t.tFileName_s = cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s ; weapon_loadsound ( );
		t.WeaponProjectileBase[t.tNewProjBase].sound = t.tSndID;
	}
	t.tInField_s = "soundDeath" ; weapon_readfield(); t.sound_s = t.value_s;
	if (  t.value_s  !=  "" ) 
	{
		t.tFileName_s = cstr("gamecore\\projectileTypes\\") + tProjectileFolder_s + t.value_s ; weapon_loadsound ( );
		t.WeaponProjectileBase[t.tNewProjBase].soundDeath = t.tSndID;
	}

	//  create a number of projectiles now for the pool
	t.tInField_s = "cacheNumber" ; weapon_readfield( ); t.tcount = t.value1_f;
	for ( t.tP = 1 ; t.tP <= t.tcount; t.tP++ )
	{
		weapon_projectile_setup ( );
	}

	UnDim (  t.fileData_s );

	t.WeaponProjectileBase[t.tNewProjBase].activeFlag = 1;
	t.tResult = t.tNewProjBase;
}

void weapon_projectile_setup ( void )
{
	//  sets up an inactive projectile ready for use
	//  takes tNewProjBase, returns tResult (0 failed, or ID of projectileBase)
	//  find an empty slot to setup this projectile in
	for ( t.tNew = 1 ; t.tNew<=  g.weaponSystem.numProjectiles; t.tNew++ )
	{
		if (  t.WeaponProjectile[t.tNew].baseType  ==  0 ) 
		{
			t.WeaponProjectile[t.tNew].baseType = t.tNewProjBase;

			//  if this projectile type has an object, instance it now
			if (  t.WeaponProjectileBase[t.tNewProjBase].baseObj > 0 ) 
			{
				weapon_getfreeobject ( );
				if (  t.tObjID  ==  0 ) 
				{
					t.tResult = 0; return;
				}
				t.WeaponProjectile[t.tNew].obj = t.tObjID;
				CloneObject (  t.tObjID,t.WeaponProjectileBase[t.tNewProjBase].baseObj );
				SetObjectTransparency (  t.tObjID,6 );
				SetObjectMask (  t.tObjID,1 );
				HideObject (  t.tObjID );
			}

			//  make the tracer object
			weapon_getfreeobject ( );
			if (  t.tObjID  ==  0  )  break;
			CloneObject (  t.tObjID, g.weaponSystem.objTracer );
			SetObjectMask (  t.tObjID,1 );
			t.WeaponProjectile[t.tNew].tracerObj = t.tObjID;
			HideObject (  t.tObjID );

			//  make the sound effect
			if (  t.WeaponProjectileBase[t.tNewProjBase].sound > 0 ) 
			{
				weapon_getfreesound ( );
				if (  t.tSndID > 0 ) 
				{
					CloneSound (  t.tSndID,t.WeaponProjectileBase[t.tNewProjBase].sound );
					t.WeaponProjectile[t.tNew].sound = t.tSndID;
				}
			}

			//  make the onDeath sound effect
			if (  t.WeaponProjectileBase[t.tNewProjBase].soundDeath > 0 ) 
			{
				weapon_getfreesound ( );
				if (  t.tSndID > 0 ) 
				{
					CloneSound (  t.tSndID,t.WeaponProjectileBase[t.tNewProjBase].soundDeath );
					t.WeaponProjectile[t.tNew].soundDeath = t.tSndID;
				}
			}

			t.tResult = t.tNew; return;

		}

	}
	t.tResult = 0;
}

void weapon_projectile_make ( void )
{
	//  creates a projectile in the game. Takes;
	//    tProjectileType which is an index of array WeaponProjectileBase()
	//    tSourceEntity, tTracerFlag
	//    tStartX#, tStartY#, tStartZ#, tAngX#, tAngY#, tAngZ#
	//    gunid,firemode

	//  if third person, scan for character targets and adjust X angle (removed targetting for now)
	t.tlocktargetonobject=0;
	if (  t.playercontrol.thirdperson.enabled == 1 && t.playercontrol.thirdperson.cameralocked == 1 ) 
	{
		t.tttbesta_f=500.0;
		for ( t.ttte = 1 ; t.ttte<=  g.entityelementlist; t.ttte++ )
		{
			if ( t.ttte != t.playercontrol.thirdperson.charactere )
			{
				t.tentid=t.entityelement[t.ttte].bankindex;
				if (  t.entityprofile[t.tentid].ischaracter == 1 ) 
				{
					if (  t.tentid>0 && t.entityelement[t.ttte].health>0 ) 
					{
						t.tdx_f=t.entityelement[t.ttte].x-t.tStartX_f;
						t.tdy_f=t.entityelement[t.ttte].y-t.tStartY_f;
						t.tdz_f=t.entityelement[t.ttte].z-t.tStartZ_f;
						t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdy_f*t.tdy_f)+abs(t.tdz_f*t.tdz_f));
						if (  t.tdd_f<1000.0 ) 
						{
							t.tda1_f=WrapValue(atan2deg(t.tdx_f,t.tdz_f));
							t.tda2_f=WrapValue(CameraAngleY(t.terrain.gameplaycamera));
							t.tda_f=t.tda1_f-t.tda2_f;
							if (  t.tda_f<-180  )  t.tda_f = t.tda_f+360;
							if (  t.tda_f>180  )  t.tda_f = t.tda_f-360;
							t.tda_f=abs(t.tda_f);
							if (  t.tda_f <= 10.0 ) 
							{
								if (  t.tda_f<t.tttbesta_f ) 
								{
									t.tttbesta_f=t.tda_f ; t.tlocktargetonobject=t.entityelement[t.ttte].obj;
								}
							}
						}
					}
				}
			}
		}
	}

	//  find a free projectile
	t.tNewProj = 0;
	for ( t.tProj = 1 ; t.tProj<=  g.weaponSystem.numProjectiles; t.tProj++ )
	{
		if (  t.WeaponProjectile[t.tProj].activeFlag  ==  0 ) 
		{
			if (  t.WeaponProjectile[t.tProj].baseType  ==  t.tProjectileType ) 
			{
				t.tNewProj = t.tProj;
				break;
			}
		}
	}
	if (  t.tNewProj  ==  0 ) 
	{
		t.tResult = 0; return;
	}

	t.WeaponProjectile[t.tNewProj].activeFlag = 1;
	t.tTimer = Timer();
	t.WeaponProjectile[t.tNewProj].createStamp = t.tTimer;
	t.WeaponProjectile[t.tNewProj].resultIntervalStamp = t.tTimer;
	t.WeaponProjectile[t.tNewProj].soundIntervalStamp = t.tTimer;
	t.WeaponProjectile[t.tNewProj].sourceEntity = t.tSourceEntity;

	//  Settings from weapon properties
	t.WeaponProjectile[t.tNewProj].usespotlighting = g.firemodes[t.gunid][g.firemode].settings.usespotlighting;
	if ( t.WeaponProjectileBase[t.tProjectileType].overridespotlighting != 0 ) 
	{
		t.WeaponProjectile[t.tNewProj].usespotlighting = t.WeaponProjectileBase[t.tProjectileType].overridespotlighting;
	}

	//  adjustment to start position to avoid penetrating
	t.tobj = t.WeaponProjectile[t.tNewProj].tracerObj;
	t.tweaponlimb=t.WeaponProjectileBase[t.tProjectileType].attachToWeaponLimb;
	if (  t.tweaponlimb >= 0 ) 
	{
		//  use start position described by weapon limb
		if (  LimbExist(t.currentgunobj,t.tweaponlimb) == 1 ) 
		{
			t.tStartX_f = LimbPositionX(t.currentgunobj,t.tweaponlimb);
			t.tStartY_f = LimbPositionY(t.currentgunobj,t.tweaponlimb);
			t.tStartZ_f = LimbPositionZ(t.currentgunobj,t.tweaponlimb);
			t.tAngX_f = LimbAngleX(t.currentgunobj,t.tweaponlimb) ;
			t.tAngY_f = LimbAngleY(t.currentgunobj,t.tweaponlimb) ;
			t.tAngZ_f = LimbAngleZ(t.currentgunobj,t.tweaponlimb) ;
		}
	}

	//  adjustment to start position to avoid penetrating
	PositionObject (  t.tobj,t.tStartX_f,t.tStartY_f,t.tStartZ_f );

	//  auto targetting removed in favour of camera based targetting (if camera in locked mode)
	if (  t.playercontrol.thirdperson.enabled == 1 && t.tSourceEntity == 0 ) 
	{
		if (  t.playercontrol.thirdperson.cameralocked == 1 ) 
		{
			if (  t.tlocktargetonobject>0 ) 
			{
				//  camera fixed, targetting help
				t.tcenterobjy_f=ObjectPositionY(t.tlocktargetonobject)+(35.0);
				PointObject (  t.tobj,ObjectPositionX(t.tlocktargetonobject),t.tcenterobjy_f,ObjectPositionZ(t.tlocktargetonobject) );
				t.tAngX_f=ObjectAngleX(t.tobj) ;
				t.tAngZ_f=ObjectAngleZ(t.tobj) ;
			}
			else
			{
				//  camera fixed, no target
				t.tAngX_f=0;
				t.tAngY_f=CameraAngleY(t.terrain.gameplaycamera) ;
				t.tAngZ_f=0;
			}
		}
		else
		{
			//  nonfixed camera, use users camera view
			t.tAngX_f=CameraAngleX(t.terrain.gameplaycamera) ;
			t.tAngY_f=CameraAngleY(t.terrain.gameplaycamera) ;
			t.tAngZ_f=CameraAngleZ(t.terrain.gameplaycamera) ;
			t.tAngX_f=t.tAngX_f-13.0;
		}
	}
	RotateObject (  t.tobj, t.tAngX_f , t.tAngY_f , t.tAngZ_f );
	MoveObject (  t.tobj,t.WeaponProjectileBase[t.tProjectileType].avoidPlayerPenetration_f );
	t.tStartX_f = ObjectPositionX(t.tobj);
	t.tStartY_f = ObjectPositionY(t.tobj);
	t.tStartZ_f = ObjectPositionZ(t.tobj);

	t.WeaponProjectile[t.tNewProj].startXPos_f = t.tStartX_f;
	t.WeaponProjectile[t.tNewProj].startYPos_f = t.tStartY_f;
	t.WeaponProjectile[t.tNewProj].startZPos_f = t.tStartZ_f;
	t.WeaponProjectile[t.tNewProj].xPos_f = t.tStartX_f;
	t.WeaponProjectile[t.tNewProj].yPos_f = t.tStartY_f;
	t.WeaponProjectile[t.tNewProj].zPos_f = t.tStartZ_f;

	//  use the tracer object (which exists for all projectiles) to calculate speeds and angles
	t.WeaponProjectile[t.tNewProj].tracerFlag = t.tTracerFlag;
	t.tobj = t.WeaponProjectile[t.tNewProj].tracerObj;
	PositionObject (  t.tobj,t.tStartX_f,t.tStartY_f,t.tStartZ_f );
	t.tAngRandom = t.WeaponProjectileBase[t.tProjectileType].xAccuracy_f * 100;
	t.xAngRandom_f = (Rnd(t.tAngRandom)*2 - t.tAngRandom)/100.0 ;
	t.tAngRandom = t.WeaponProjectileBase[t.tProjectileType].yAccuracy_f * 100;
	t.yAngRandom_f = (Rnd(t.tAngRandom)*2 - t.tAngRandom)/100.0 ;
	RotateObject (  t.tobj,t.tAngX_f + t.xAngRandom_f ,t.tAngY_f + t.yAngRandom_f,t.tAngZ_f );
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedMax_f - t.WeaponProjectileBase[t.tProjectileType].speedMin_f)*100;
	t.tspeed_f = t.WeaponProjectileBase[t.tProjectileType].speedMin_f + Rnd(t.tSpeedRange)/100.0;
	MoveObject (  t.tobj,t.tspeed_f );
	t.WeaponProjectile[t.tNewProj].xSpeed_f = ObjectPositionX(t.tobj) - t.tStartX_f;
	t.WeaponProjectile[t.tNewProj].ySpeed_f = ObjectPositionY(t.tobj) - t.tStartY_f;
	t.WeaponProjectile[t.tNewProj].zSpeed_f = ObjectPositionZ(t.tobj) - t.tStartZ_f;
	t.WeaponProjectile[t.tNewProj].xAng_f = ObjectAngleX(t.tobj);
	t.WeaponProjectile[t.tNewProj].yAng_f = ObjectAngleY(t.tobj);
	t.WeaponProjectile[t.tNewProj].zAng_f = ObjectAngleZ(t.tobj);
	t.WeaponProjectile[t.tNewProj].acceleration_f = t.tspeed_f * t.WeaponProjectileBase[t.tProjectileType].thrustModifier_f;

	//  calculate random rotation speeds
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedAngMaxX_f - t.WeaponProjectileBase[t.tProjectileType].speedAngMinX_f)*100;
	t.WeaponProjectile[t.tNewProj].xAngSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedAngMinX_f + Rnd(t.tSpeedRange)/100.0;
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedAngMaxY_f - t.WeaponProjectileBase[t.tProjectileType].speedAngMinY_f)*100;
	t.WeaponProjectile[t.tNewProj].yAngSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedAngMinY_f + Rnd(t.tSpeedRange)/100.0;
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedAngMaxZ_f - t.WeaponProjectileBase[t.tProjectileType].speedAngMinZ_f)*100;
	t.WeaponProjectile[t.tNewProj].zAngSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedAngMinZ_f + Rnd(t.tSpeedRange)/100.0;

	//  calculate random turn speeds
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedTurnMaxX_f - t.WeaponProjectileBase[t.tProjectileType].speedTurnMinX_f)*100;
	t.WeaponProjectile[t.tNewProj].xTurnSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedTurnMinX_f + Rnd(t.tSpeedRange)/100.0;
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedTurnMaxY_f - t.WeaponProjectileBase[t.tProjectileType].speedTurnMinY_f)*100;
	t.WeaponProjectile[t.tNewProj].yTurnSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedTurnMinY_f + Rnd(t.tSpeedRange)/100.0;
	t.tSpeedRange = (t.WeaponProjectileBase[t.tProjectileType].speedTurnMaxZ_f - t.WeaponProjectileBase[t.tProjectileType].speedTurnMinZ_f)*100;
	t.WeaponProjectile[t.tNewProj].zTurnSpeed_f = t.WeaponProjectileBase[t.tProjectileType].speedTurnMinZ_f + Rnd(t.tSpeedRange)/100.0;
	t.WeaponProjectile[t.tNewProj].xTurn_f = 0;
	t.WeaponProjectile[t.tNewProj].yTurn_f = 0;
	t.WeaponProjectile[t.tNewProj].zTurn_f = 0;

	//  if this projectile has an object, position it now
	t.tobj = t.WeaponProjectile[t.tNewProj].obj;
	if (  t.tobj > 0 ) 
	{
		ShowObject (  t.tobj );
		PositionObject (  t.tobj,t.tStartX_f,t.tStartY_f,t.tStartZ_f );
		RotateObject (  t.tobj,t.WeaponProjectile[t.tNewProj].xAng_f,t.WeaponProjectile[t.tNewProj].yAng_f,t.WeaponProjectile[t.tNewProj].zAng_f );
	}

	//  if this projectile uses real physics, grant this now
	if (  t.tobj > 0 ) 
	{
		if (  t.WeaponProjectileBase[t.tProjectileType].usingRealPhysics>0 ) 
		{
			t.tweight=-1 ; t.tfriction=-1;
			YRotateCamera (  0,CameraAngleY(0)-40 );
			SetObjectToCameraOrientation (  t.tobj );
			YRotateCamera (  0,CameraAngleY(0)+40 );
			MoveObject (  t.tobj,10.0 );
			t.WeaponProjectile[t.tNewProj].xSpeed_f=ObjectPositionX(t.tobj)-CameraPositionX(0);
			t.WeaponProjectile[t.tNewProj].ySpeed_f=ObjectPositionY(t.tobj)-CameraPositionY(0);
			t.WeaponProjectile[t.tNewProj].zSpeed_f=ObjectPositionZ(t.tobj)-CameraPositionZ(0);
			MoveObject (  t.tobj,-10.0 );
			ODECreateDynamicBox (  t.tobj,-1,0,t.tweight,t.tfriction,-1 );
			t.tinertiax_f=t.WeaponProjectile[t.tNewProj].xSpeed_f*t.WeaponProjectile[t.tNewProj].acceleration_f;
			t.tinertiay_f=t.WeaponProjectile[t.tNewProj].ySpeed_f*t.WeaponProjectile[t.tNewProj].acceleration_f;
			t.tinertiaz_f=t.WeaponProjectile[t.tNewProj].zSpeed_f*t.WeaponProjectile[t.tNewProj].acceleration_f;
			t.tidiv_f=0.2f;
			ODESetLinearVelocity (  t.tobj,t.tinertiax_f*t.tidiv_f,t.tinertiay_f*t.tidiv_f,t.tinertiaz_f*t.tidiv_f );
			t.tinertiax_f=t.WeaponProjectile[t.tNewProj].xTurnSpeed_f;
			t.tinertiay_f=t.WeaponProjectile[t.tNewProj].yTurnSpeed_f;
			t.tinertiaz_f=t.WeaponProjectile[t.tNewProj].zTurnSpeed_f;
			ODESetAngularVelocity (  t.tobj,t.tinertiax_f,t.tinertiay_f,t.tinertiaz_f );
		}
	}

	//  if this projectile has a sound that loops, start it now
	if (  t.tStartY_f<CameraPositionY(0)+10000 ) 
	{
		t.tSndID = t.WeaponProjectile[t.tNewProj].sound;
		if (  t.tSndID > 0 ) 
		{
			if (  t.WeaponProjectileBase[t.tProjectileType].soundLoopFlag  ==  1 ) 
			{
				PositionSound (  t.tSndID,t.tStartX_f,t.tStartY_f,t.tStartZ_f );
				t.tsoundfreq=t.WeaponProjectileBase[t.tProjectileType].soundDopplerBaseSpeed;
				if (  t.tsoundfreq<10000  )  t.tsoundfreq = 10000;
				if (  t.tsoundfreq>60000  )  t.tsoundfreq = 60000;
				SetSoundSpeed (  t.tSndID,t.tsoundfreq );
				LoopSound (  t.tSndID );
				if (  t.WeaponProjectileBase[t.tProjectileType].soundDopplerFlag  ==  1 ) 
				{
					t.tProj = t.tNewProj ; weapon_projectile_setDistFromPlayer ( );
				}
			}
		}
	}

	//  setup particle emitters for this projectile
	//  but only if entities not set to LOWEST as particle trails are expensive!
	t.tokay = 1 ; if (  t.visuals.shaderlevels.entities == 3  )  t.tokay = 0;
	if (  t.WeaponProjectileBase[t.tProjectileType].particleType>0 && t.tokay == 1 ) 
	{
		ravey_particles_get_free_emitter ( );
		t.WeaponProjectile[t.tNewProj].tempEmitter = t.tResult;
		if (  t.tResult>0 ) 
		{
			weapon_add_projectile_particles ( );
		}
	}

	//  returns tResult (0 not made, 1 successful)
	//  returns tProjectile (index of WeaponProjectile() of projectile made)
	t.tResult = 1 ; t.tProjectile = t.tNewProj;
}

void weapon_add_projectile_particles ( void )
{
	//  takes tResult as the id of the partiels and t.tobj as the parent object
	g.tEmitter.id = t.tResult;
	g.tEmitter.emitterLife = 0;
	g.tEmitter.parentObject = t.tobj;
	g.tEmitter.parentLimb = 0;
	g.tEmitter.isAnObjectEmitter = 0;

	// load stock or custom particle trail image
	g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
	switch ( t.WeaponProjectileBase[t.tProjectileType].particleType )
	{
		// Stock particle images
		case 1 : g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset; break;
		case 2 : g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_FLARE + g.particlesimageoffset; break;
		case 3 : g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_FLAME + g.particlesimageoffset; break;

		// Custom particle image
		case 99 : 
			g.tEmitter.imageNumber = t.WeaponProjectileBase[t.tProjectileType].particleImageID; 
			if ( g.tEmitter.imageNumber == 0 )
			{
				g.tEmitter.imageNumber = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
				t.WeaponProjectileBase[t.tProjectileType].particleType = 0;
			}
			break;
	}
	g.tEmitter.isAnimated = 1;
	g.tEmitter.animationSpeed = 1/2.0;
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
	g.tEmitter.scaleStartMin = 8;
	g.tEmitter.scaleStartMax = 12;
	g.tEmitter.scaleEndMin = 50;
	g.tEmitter.scaleEndMax = 60;
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
	g.tEmitter.frequency = 50;
	ravey_particles_add_emitter ( );
}

void weapon_projectile_reset ( void )
{
	//  for now simply any looping sounds
	for ( t.tProj = 1 ; t.tProj<=  g.weaponSystem.numProjectiles; t.tProj++ )
	{
		if (  t.WeaponProjectile[t.tProj].sound>0 ) 
		{
			if (  SoundExist(t.WeaponProjectile[t.tProj].sound) == 1 ) 
			{
				StopSound (  t.WeaponProjectile[t.tProj].sound );
			}
		}
	}
}

void weapon_mp_projectile_reset ( void )
{
	for ( t.tProj = 1 ; t.tProj<=  g.weaponSystem.numProjectiles; t.tProj++ )
	{
		if (  t.WeaponProjectile[t.tProj].activeFlag  ==  1 ) 
		{
			weapon_projectile_destroy ( );
		}
	}
}

void weapon_projectile_setDistFromPlayer ( void )
{
	//  sets the distance of projectile from player. Takes tProj
	t.txDist_f = CameraPositionX(0) - t.WeaponProjectile[t.tProj].xPos_f;
	t.tyDist_f = CameraPositionY(0) - t.WeaponProjectile[t.tProj].yPos_f;
	t.tzDist_f = CameraPositionZ(0) - t.WeaponProjectile[t.tProj].zPos_f;
	t.WeaponProjectile[t.tProj].soundDistFromPlayer_f = Sqrt(t.txDist_f*t.txDist_f + t.tyDist_f*t.tyDist_f + t.tzDist_f*t.tzDist_f);
}

void weapon_projectile_destroy ( void )
{
	//  resets a projectile back to it's hidden default state. Takes tProj
	t.WeaponProjectile[t.tProj].activeFlag = 0;
	if (  t.WeaponProjectile[t.tProj].obj > 0  )  HideObject (  t.WeaponProjectile[t.tProj].obj );

	//  remove physics object if used
	t.tProjectileType = t.WeaponProjectile[t.tProj].baseType;
	if (  t.WeaponProjectileBase[t.tProjectileType].usingRealPhysics>0 ) 
	{
		ODEDestroyObject (  t.WeaponProjectile[t.tProj].obj );
	}

	//  stop any looping sound
	if (  t.WeaponProjectile[t.tProj].sound > 0 ) 
	{
		StopSound (  t.WeaponProjectile[t.tProj].sound );
	}

	//  temp killing of hardcoded particle emitter
	if (  t.WeaponProjectile[t.tProj].tempEmitter > 0 ) 
	{
		t.tRaveyParticlesEmitterID = t.WeaponProjectile[t.tProj].tempEmitter;
		ravey_particles_delete_emitter ( );
	}
}

void weapon_projectileresult_make ( void )
{
	// creates a projectile result, which is an explosion, or decal, or many other possibilities. Takes;
	// t.tProjectileName_s 
	// tProjectileResult which is value of constant WEAPON_PROJECTILERESULT_*
	// t.tProjectileResultExplosionImageID passed in if a custom projectile explosion used
	// t.tProjectileResultLightFlag 
	// t.tProjectileResultSmokeImageID 
	// t.tProjectileResultSparksCount
	// t.tProjectileResultSize
	// t.tProjectileResultSmokeSize
	// t.tProjectileResultSparksSize
	// tHitObj, tX#, tY#, tZ#, tAngX#, tAngY#, tAngZ#
	// t.tDamage_f, tRadius#, tAICharacter etc.
	// tSoundID, tSourceEntity
	t.tResult = 0;
	switch ( t.tProjectileResult ) 
	{
		case WEAPON_PROJECTILERESULT_DAMAGE:
		{
			// 080618 - copied from below - trigger direct damage if hit an entity
			if ( t.tHitObj>0 ) 
			{
				for ( t.ttte = 1 ; t.ttte <= g.entityelementlist; t.ttte++ )
				{
					t.entid=t.entityelement[t.ttte].bankindex;
					if (  t.entid>0 ) 
					{
						if (  t.entityelement[t.ttte].obj == t.tHitObj ) 
						{
							t.tdamage=t.tDamage_f;
							t.tdamageforce=t.tDamage_f;
							t.tdamagesource=2;
							t.brayx1_f=CameraPositionX(t.terrain.gameplaycamera);
							t.brayy1_f=CameraPositionY(t.terrain.gameplaycamera);
							t.brayz1_f=CameraPositionZ(t.terrain.gameplaycamera);
							t.brayx2_f=t.tx_f ; t.brayy2_f=t.ty_f ; t.brayz2_f=t.tz_f;
							entity_applydamage ( );
							break;
						}
					}
				}
			}
		}
		break;

		case WEAPON_PROJECTILERESULT_EXPLODE:
		case WEAPON_PROJECTILERESULT_CUSTOM:
		{
			// trigger stock explosion
			t.tXPos_f = t.tx_f ; t.tYPos_f = t.ty_f ; t.tZPos_f = t.tz_f;
			if ( t.tProjectileResult == WEAPON_PROJECTILERESULT_CUSTOM )
			{
				// custom explosion image
				int iLightFlag = t.tProjectileResultLightFlag;
				int iSmokeImageID = t.tProjectileResultSmokeImageID;
				int iSparksCount = t.tProjectileResultSparksCount;
				float fSize = (float)t.tProjectileResultSize / 100.0f;
				float fSmokeSize = (float)t.tProjectileResultSmokeSize / 100.0f;
				float fSparksSize = (float)t.tProjectileResultSparksSize / 100.0f;
				explosion_custom(t.tProjectileResultExplosionImageID, iLightFlag, iSmokeImageID, iSparksCount, fSize, fSmokeSize, fSparksSize, t.tXPos_f, t.tYPos_f, t.tZPos_f);
			}
			else
			{
				// stock explosion
				if ( t.tradius_f == 0 ) 
				{
					// fireball if radius is zero
					explosion_fireball(t.tXPos_f,t.tYPos_f,t.tZPos_f);
				}
				else
				{
					// rocket explosion if it has a radius of damage
					explosion_rocket(t.tXPos_f,t.tYPos_f,t.tZPos_f);
				}
			}

			// play explosion sound
			if ( t.tSoundID > 0 ) 
			{
				PositionSound ( t.tSoundID,t.tx_f,t.ty_f,t.tz_f );
				SetSoundSpeed ( t.tSoundID,38000+Rnd(8000) );
				PlaySound ( t.tSoundID );
			}

			// record the explosion for any LUA script detection
			if ( strlen ( t.tProjectileName_s.Get() ) > 0 )
			{
				g.projectileEventType_explosion = 1;
				g.projectileEventType_name = t.tProjectileName_s;
				g.projectileEventType_x = t.tXPos_f;
				g.projectileEventType_y = t.tYPos_f;
				g.projectileEventType_z = t.tZPos_f;
				g.projectileEventType_radius = t.tradius_f;
				g.projectileEventType_damage = t.tDamage_f;
				g.projectileEventType_entityhit = 0;
			}

			// trigger direct damage if hit an entity
			if ( t.tHitObj>0 ) 
			{
				for ( t.ttte = 1 ; t.ttte <= g.entityelementlist; t.ttte++ )
				{
					t.entid=t.entityelement[t.ttte].bankindex;
					if ( t.entid>0 ) 
					{
						if ( t.entityelement[t.ttte].obj == t.tHitObj ) 
						{
							t.tdamage=t.tDamage_f;
							t.tdamageforce=t.tDamage_f;
							t.tdamagesource=2;
							t.brayx1_f=CameraPositionX(t.terrain.gameplaycamera);
							t.brayy1_f=CameraPositionY(t.terrain.gameplaycamera);
							t.brayz1_f=CameraPositionZ(t.terrain.gameplaycamera);
							t.brayx2_f=t.tx_f ; t.brayy2_f=t.ty_f ; t.brayz2_f=t.tz_f;
							entity_applydamage ( );
							if ( strlen ( t.tProjectileName_s.Get() ) > 0 )
							{
								g.projectileEventType_entityhit = t.ttte;
							}
							break;
						}
					}
				}
			}

			// trigger physics sphere explosion at this GetPoint (  (does damage and physics force) )
			t.texplodex_f=t.tx_f;
			t.texplodey_f=t.ty_f;
			t.texplodez_f=t.tz_f;
			t.texploderadius_f=t.tradius_f;
			t.texplodesourceEntity=t.tSourceEntity;
			if ( t.game.runasmultiplayer == 1 && g.mp.coop == 1 ) 
			{
				// Check to see if it was the AI that caused the explosion in coop multiplayer
				if ( t.tSourceEntity != 0 )  g.mp.damageWasFromAI  =  1;
			}
			physics_explodesphere ( );
			t.tResult = 1;
		}
		break;
	}
}

void weapon_loadsound ( void )
{
	//  loads in the sound tFileName$ and returns tSndID = 0 (failed) or the DBP sound number
	if (  FileExist(t.tFileName_s.Get())  ==  0 ) 
	{
		t.tSndID = 0; return;
	}
	weapon_getfreesound ( );
	if (  t.tSndID  ==  0  )  return;
	Load3DSound (  t.tFileName_s.Get(),t.tSndID );
	if (  SoundExist(t.tSndID)  ==  0 ) 
	{
		t.tSndID = 0; return;
	}
}

void weapon_LUA_addWeapon ( void )
{
}

void weapon_LUA_addProjectileBase ( void )
{
}

void weapon_LUA_makeProjectile ( void )
{
}

void weapon_LUA_firePlayerWeapon ( void )
{
}

void weapon_LUA_reloadPlayerWeapon ( void )
{
}

void weapon_LUA_changePlayerWeapon ( void )
{
}

void weapon_LUA_changePlayerWeaponMode ( void )
{
}

void weapon_LUA_setWeaponROFsingle ( void )
{
}

void weapon_LUA_setWeaponROFauto ( void )
{
}

void weapon_LUA_setWeaponAmmo ( void )
{
}

void weapon_LUA_setWeaponDamageModifier ( void )
{
}

void weapon_LUA_setWeaponRangeModifier ( void )
{
}

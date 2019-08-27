//----------------------------------------------------
//--- GAMEGURU - M-Particles
//----------------------------------------------------

#include "gameguru.h"

float fwindVectX, fwindVectZ = 0.0f;

void ravey_particles_set_wind_vector(float fwindX, float fwindZ)
{
	fwindVectX = fwindX;
	fwindVectZ = fwindZ;
}


void ravey_particles_init ( void )
{
	//  pre create max particles
	for ( t.c = 0 ; t.c <  RAVEY_PARTICLES_MAX; t.c++ )
	{
		//  create particle object
		t.tParticleObj = g.raveyparticlesobjectoffset+t.c;
		if (  ObjectExist(t.tParticleObj) == 1  )  DeleteObject (  t.tParticleObj );

		// currently all 'particles' are a simple 100x100 Plane
		// TBD - Add ability to specify other shapes to use as a particle?  Would we gain much from that?
		MakeObjectPlane (  t.tParticleObj,100,100 );
		
		FixObjectPivot (  t.tParticleObj );
		SetObjectTransparency (  t.tParticleObj, 6 );
		SetObjectCollisionOff (  t.tParticleObj );
		DisableObjectZWrite(t.tParticleObj);
		SetObjectTextureMode (  t.tParticleObj, 0, 0 );
		SetObjectLight (  t.tParticleObj, 0 );
		SetObjectMask (  t.tParticleObj, 1 );
		HideObject (  t.tParticleObj );
		TextureObject (  t.tParticleObj, RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset );
		// Do we need to be able to change the 'effect' from Lua?
		SetObjectEffect (  t.tParticleObj, g.decaleffectoffset );
		SetObjectCull ( t.tParticleObj, 0 );

		//  lock UV here and fill with 8x8 UV grid ref
		t.uvwidth_f=1.0/8.0;
		t.uvheight_f=1.0/8.0;
		LockVertexDataForLimbCore (  t.tParticleObj, 0, 1 );
		SetVertexDataUV (  0, t.uvwidth_f, 0 );
		SetVertexDataUV (  1, 0, 0 );
		SetVertexDataUV (  2, t.uvwidth_f, t.uvheight_f );
		SetVertexDataUV (  3, 0, 0 );
		SetVertexDataUV (  4, 0, t.uvheight_f );
		SetVertexDataUV (  5, t.uvwidth_f, t.uvheight_f );
		UnlockVertexData (  );
	}

	//  reset emitters
	for ( t.c = 0 ; t.c < RAVEY_PARTICLE_EMITTERS_MAX; t.c++ )
	{
		t.ravey_particle_emitters[t.c].inUse = 0;
	}

	//  reset particles
	for ( t.c = 0 ; t.c <  RAVEY_PARTICLES_MAX; t.c++ )
	{
		t.ravey_particles[t.c].inUse = 0;
	}

	g.ravey_particles_old_time = 0;

	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
}

void ravey_particles_load_images ( void )
{
	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

	// load default images (used by rpg etc )
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_FLARE + g.particlesimageoffset;
	if (  ImageExist( t.tImgID )  ==  0  )  LoadImage (  "effectbank\\particles\\flare.dds", t.tImgID );

	//  Used for rocket smoke trails
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
	if (  ImageExist( t.tImgID )  ==  0  )  LoadImage (  "effectbank\\particles\\64smoke2.dds", t.tImgID );

	//  Used for fireball projectile
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_FLAME + g.particlesimageoffset;
	if (  ImageExist( t.tImgID )  ==  0  )  LoadImage (  "effectbank\\particles\\flame.dds", t.tImgID );
}

int ravey_particles_load_image( LPSTR szFilename, int iID )
{
	if (iID == 0)
	{
		for ( int i = 0; i < RAVEY_PARTICLES_IMAGETYPE_MAX; i++ )
		{
			t.tImgID = i + g.particlesimageoffset;
			if ( ImageExist( t.tImgID ) == 0)
			{
				iID = i;
				break;
			}
		}
	}
	else if ( iID >= RAVEY_PARTICLES_IMAGETYPE_MAX  ) return -1;

	t.tImgID = iID + g.particlesimageoffset;

	LoadImage( szFilename, t.tImgID );

	return iID;
}

void ravey_particles_set_frames( int iID, int iAnimSpeed, int iStartFrame, int iEndFrame )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].animationSpeed = (float) iAnimSpeed * 0.06f;
		t.ravey_particle_emitters[ iID ].startFrame = iStartFrame;
		t.ravey_particle_emitters[ iID ].endFrame   = iEndFrame;
		if ( iAnimSpeed > 0 )
		{
			t.ravey_particle_emitters[ iID ].isLooping = 1;
		}
	}
}

void ravey_particles_set_no_wind( int iID )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].noWind = TRUE;
	}
}

void ravey_particles_set_speed( int iID, float fMinX, float fMinY, float fMinZ, float fMaxX, float fMaxY, float fMaxZ )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].movementSpeedMinX = fMinX;
		t.ravey_particle_emitters[ iID ].movementSpeedMaxX = fMaxX;
		t.ravey_particle_emitters[ iID ].movementSpeedMinY = fMinY;
		t.ravey_particle_emitters[ iID ].movementSpeedMaxY = fMaxY;
		t.ravey_particle_emitters[ iID ].movementSpeedMinZ = fMinZ;
		t.ravey_particle_emitters[ iID ].movementSpeedMaxZ = fMaxZ;
	}
}

void ravey_particles_set_gravity( int iID, float fStartG, float fEndG )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].startGravity = fStartG;
		t.ravey_particle_emitters[ iID ].endGravity = fEndG;
	}
}
void ravey_particles_set_offset( int iID, float fMinX, float fMinY, float fMinZ, float fMaxX, float fMaxY, float fMaxZ )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1)
	{
		t.ravey_particle_emitters[ iID ].offsetMinX = fMinX;
		t.ravey_particle_emitters[ iID ].offsetMaxX = fMaxX;
		t.ravey_particle_emitters[ iID ].offsetMinY = fMinY;
		t.ravey_particle_emitters[ iID ].offsetMaxY = fMaxY;
		t.ravey_particle_emitters[ iID ].offsetMinZ = fMinZ;
		t.ravey_particle_emitters[ iID ].offsetMaxZ = fMaxZ;
	}
}

void ravey_particles_set_angle( int iID, float fAngX, float fAngY, float fAngZ )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].xAng = fAngX;
		t.ravey_particle_emitters[ iID ].yAng = fAngY;
		t.ravey_particle_emitters[ iID ].zAng = fAngZ;
		t.ravey_particle_emitters[ iID ].useAngle = TRUE;
	}
}

void ravey_particles_set_rotate( int iID, float fMinXRot, float fMaxXRot, float fMinYRot, float fMaxYRot, float fMinZRot, float fMaxZRot, int bStartsOffRandom )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1)
	{
		t.ravey_particle_emitters[ iID ].rotateSpeedMinX = fMinXRot;
		t.ravey_particle_emitters[ iID ].rotateSpeedMaxX = fMaxXRot;
		t.ravey_particle_emitters[ iID ].rotateSpeedMinY = fMinYRot;
		t.ravey_particle_emitters[ iID ].rotateSpeedMaxY = fMaxYRot;
		t.ravey_particle_emitters[ iID ].rotateSpeedMinZ = fMinZRot;
		t.ravey_particle_emitters[ iID ].rotateSpeedMaxZ = fMaxZRot;

		t.ravey_particle_emitters[ iID ].startsOffRandomAngle = bStartsOffRandom;
	}
}

void ravey_particles_set_scale( int iID, float fStartMin, float fStartMax, float fEndMin, float fEndMax )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1)
	{
		t.ravey_particle_emitters[ iID ].scaleStartMin = fStartMin;
		t.ravey_particle_emitters[ iID ].scaleStartMax = fStartMax;
		t.ravey_particle_emitters[ iID ].scaleEndMin   = fEndMin;
		t.ravey_particle_emitters[ iID ].scaleEndMax   = fEndMax;
	}
}

void ravey_particles_set_alpha( int iID, float fStartMin, float fStartMax, float fEndMin, float fEndMax )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1)
	{
		t.ravey_particle_emitters[ iID ].alphaStartMin = fStartMin;
		t.ravey_particle_emitters[ iID ].alphaStartMax = fStartMax; 
		t.ravey_particle_emitters[ iID ].alphaEndMin   = fEndMin;
		t.ravey_particle_emitters[ iID ].alphaEndMax   = fEndMax;
	}
}

void ravey_particles_set_life( int iID, int iSpawnRate, float iLifeMin, float iLifeMax, int iMaxParticles, int iOnDeathAction, int iMaxPerFrame )
{
	if ( t.ravey_particle_emitters[ iID ].inUse == 1 )
	{
		t.ravey_particle_emitters[ iID ].frequency = iSpawnRate;
		t.ravey_particle_emitters[ iID ].lifeMin = iLifeMin;
		t.ravey_particle_emitters[ iID ].lifeMax = iLifeMax;
		t.ravey_particle_emitters[ iID ].maxParticles = iMaxParticles;
		t.ravey_particle_emitters[ iID ].onDeathAction = iOnDeathAction;
		t.ravey_particle_emitters[ iID ].maxPerFrame = iMaxPerFrame;
	} 
}

void ravey_particles_update ( void )
{
	
	if ( g.ravey_particles_old_time > 0 ) 
	{
		g.ravey_particles_time_passed = Timer() - g.ravey_particles_old_time;
	}

	if ( ravey_particles_update_particles() )
	{
		g.ravey_particles_old_time = Timer();
	}
	
	ravey_particles_update_emitters();
}

inline float doCalc(float A, float B)
{
	return A + abs(A - B) *  rand() / RAND_MAX;
}

void generateParticle( travey_particle_emitter* this_emitter )
{
	travey_particle* this_particle = &t.ravey_particles[ t.tfound ];

	//  add new particle
	this_particle->inUse = 1;
	this_particle->emitterID = t.c;

	this_emitter->numParticles++;

	//  Is there a parent object? if so use its position, if not use emitters xyz
	if ( this_emitter->parentObject > 0 && ( this_emitter->onDeathAction != 1 || this_emitter->firstParticle ) )
	{
		if ( this_emitter->parentLimb == 0 )
		{
			this_particle->x = ObjectPositionX( this_emitter->parentObject ) +
				doCalc( this_emitter->offsetMinX, this_emitter->offsetMaxX );
			this_particle->y = ObjectPositionY( this_emitter->parentObject ) +
				doCalc( this_emitter->offsetMinY, this_emitter->offsetMaxY );
			this_particle->z = ObjectPositionZ( this_emitter->parentObject ) +
				doCalc( this_emitter->offsetMinZ, this_emitter->offsetMaxZ );
		}
		else
		{
			this_particle->x = LimbPositionX( this_emitter->parentObject, this_emitter->parentLimb ) +
				doCalc( this_emitter->offsetMinX, this_emitter->offsetMaxX );
			this_particle->y = LimbPositionY( this_emitter->parentObject, this_emitter->parentLimb ) +
				doCalc( this_emitter->offsetMinY, this_emitter->offsetMaxY );
			this_particle->z = LimbPositionZ( this_emitter->parentObject, this_emitter->parentLimb ) +
				doCalc( this_emitter->offsetMinZ, this_emitter->offsetMaxZ );
		}
	}
	else
	{
		this_particle->x = this_emitter->xPos +
			doCalc( this_emitter->offsetMinX, this_emitter->offsetMaxX );
		this_particle->y = this_emitter->yPos +
			doCalc( this_emitter->offsetMinY, this_emitter->offsetMaxY );
		this_particle->z = this_emitter->zPos +
			doCalc( this_emitter->offsetMinZ, this_emitter->offsetMaxZ );
	}

	this_emitter->firstParticle = FALSE;
	
	// movement speed
	this_particle->movementSpeedX = doCalc( this_emitter->movementSpeedMinX, this_emitter->movementSpeedMaxX );
	this_particle->movementSpeedY = doCalc( this_emitter->movementSpeedMinY, this_emitter->movementSpeedMaxY );
	this_particle->movementSpeedZ = doCalc( this_emitter->movementSpeedMinZ, this_emitter->movementSpeedMaxZ );
	// rotation speed curently onl z-rotation allowed
	//t.rotX_f = abs(this_emitter->rotateSpeedMinX - this_emitter->rotateSpeedMaxX) * rand()/RAND_MAX;
	//this_particle->rotateSpeedX = this_emitter->rotateSpeedMinX + t.rotX_f;
	//t.rotY_f = abs(this_emitter->rotateSpeedMinY - this_emitter->rotateSpeedMaxY) * rand()/RAND_MAX;
	//this_particle->rotateSpeedY = this_emitter->rotateSpeedMinY + t.rotY_f;
	this_particle->rotateSpeedZ = doCalc( this_emitter->rotateSpeedMinZ, this_emitter->rotateSpeedMaxZ );
	this_particle->rotz = 0;

	// scale values
	this_particle->scaleStart = doCalc( this_emitter->scaleStartMin, this_emitter->scaleStartMax );
	this_particle->scaleEnd = doCalc( this_emitter->scaleEndMin, this_emitter->scaleEndMax );
	// life expectancy
	this_particle->life = doCalc( this_emitter->lifeMin, this_emitter->lifeMax );
	this_particle->timePassed = 0;
	// alpha settings
	this_particle->alpha = doCalc( this_emitter->alphaStartMin, this_emitter->alphaStartMax );
	this_particle->alphaStart = this_particle->alpha;
	this_particle->alphaEnd = doCalc( this_emitter->alphaEndMin, this_emitter->alphaEndMax );

	t.tobj = t.tfound + g.raveyparticlesobjectoffset;

	PositionObject( t.tobj, this_particle->x, this_particle->y, this_particle->z );

	//PE: TextureObject will clear animation list and trigger a new to be generated , so just texture them directly.
	//TextureObject( t.tobj, 0, this_emitter->imageNumber ); //PE: This makes everything slow ?
	sObject* pObject = g_ObjectList[t.tobj];
	if (pObject) {
		for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++)
			SetBaseTextureStage(pObject->ppMeshList[iMesh], 0, this_emitter->imageNumber);
	}


	SetAlphaMappingOn( t.tobj, t.ravey_particles[t.c].alpha );
	ScaleObject( t.tobj, this_particle->scaleStart, this_particle->scaleStart, this_particle->scaleStart );
	
	//  animation
	this_particle->frameDivide = this_emitter->frameDivide;
	this_particle->frameMulti = this_emitter->frameMulti;
	if ( this_emitter->animationSpeed == 0.0 &&
		 this_emitter->frameCount > 0 )
	{
		this_particle->startFrame = Rnd( this_emitter->frameCount );
		this_particle->endFrame = this_particle->startFrame;
		this_particle->isAnimated = 1;  // sort of lying but we do this to trigger the frame calculation later
	}
	else
	{
		this_particle->startFrame = this_emitter->startFrame;
		this_particle->endFrame = this_emitter->endFrame;
	}
	this_particle->frame = (float)this_particle->startFrame;
	this_particle->previousFrame = this_particle->startFrame;
	this_particle->isAnimated = this_emitter->isAnimated;
	this_particle->isLooping = this_emitter->isLooping;
	this_particle->animationSpeed = this_emitter->animationSpeed;
	this_particle->startGravity = this_emitter->startGravity;
	this_particle->endGravity = this_emitter->endGravity;

	if (this_emitter->startsOffRandomAngle == 0)
	{
		this_particle->rotz = 0;
	}
	else
	{
		this_particle->rotz = (float)Rnd( 360 );
	}
	if (this_emitter->useAngle)
	{
		this_particle->xa = this_emitter->xAng;
		this_particle->ya = this_emitter->yAng;
		this_particle->za = this_emitter->zAng;
		RotateObject( t.tobj, this_particle->xa, this_particle->ya, this_particle->za );
	}
	else
	{
		ZRotateObject( t.tobj, this_particle->rotz );
		PointObject( t.tobj, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
	}
	if (this_particle->isAnimated == 1)
	{
		// 200918 - lock UV here and fill with UV grid ref (copied from init as particles can specify different grid sizes)
		t.uvwidth_f = 1.0 / this_particle->frameDivide;
		t.uvheight_f = 1.0 / this_particle->frameDivide;
		LockVertexDataForLimbCore( t.tobj, 0, 1 );
		SetVertexDataUV( 0, t.uvwidth_f, 0 );
		SetVertexDataUV( 1, 0, 0 );
		SetVertexDataUV( 2, t.uvwidth_f, t.uvheight_f );
		SetVertexDataUV( 3, 0, 0 );
		SetVertexDataUV( 4, 0, t.uvheight_f );
		SetVertexDataUV( 5, t.uvwidth_f, t.uvheight_f );
		UnlockVertexData();

		float fa = Floor( this_particle->frame / this_particle->frameDivide );

		t.line_f = fa * this_particle->frameDivide;

		t.left_f = Floor( this_particle->frame - t.line_f ) * this_particle->frameMulti;


		t.top_f = fa * this_particle->frameMulti;

		ScaleObjectTexture( t.tobj, t.left_f, t.top_f );
	}
	
	ShowObject( t.tobj );
	//  end of new particle	
}

void ravey_particles_generate_particle( int iID, float fPosX, float fPosY, float fPosZ )
{
	if ( t.ravey_particle_emitters[iID].inUse == 1 )
	{
		t.c = iID;
		travey_particle_emitter* this_emitter = &t.ravey_particle_emitters[ t.c ];

		if ( this_emitter->numParticles >= this_emitter->maxParticles ) return;

		// now find a spare particle to use
		t.tfound = -1;
		for ( t.f = 0; t.f < RAVEY_PARTICLES_MAX; t.f++ )
		{
			if ( t.ravey_particles[t.f].inUse == 0 )
			{
				t.tfound = t.f;
				break;
			}
		}

		if ( t.tfound != -1 )
		{
			// if emitter not attached to object and position specified use it as
			// origin point for spawning
			if ( this_emitter->parentObject == 0 )
			{
				if ( fPosX + fPosY + fPosZ > 0.0 )
				{
					this_emitter->xPos = fPosX;
					this_emitter->yPos = fPosY;
					this_emitter->zPos = fPosZ;
				}
			}
			generateParticle( this_emitter );
		}
	}
}


void ravey_particles_update_emitters( void )
{
	for ( t.c = 0; t.c < RAVEY_PARTICLE_EMITTERS_MAX; t.c++ )
	{
		travey_particle_emitter* this_emitter = &t.ravey_particle_emitters[ t.c ];

		if ( this_emitter->inUse == 1 )
		{
			//  time to spawn some particles?
			this_emitter->timePassed += g.ravey_particles_time_passed;
			if ( this_emitter->timePassed > this_emitter->frequency &&
				 this_emitter->numParticles < this_emitter->maxParticles )
			{
				//  move emitter by speed if it doesn't have a parent object
				if ( this_emitter->parentObject == 0 )
				{
					this_emitter->xPos = this_emitter->xPos + this_emitter->xSpeed * this_emitter->timePassed;
					this_emitter->yPos = this_emitter->yPos + this_emitter->ySpeed * this_emitter->timePassed;
					this_emitter->zPos = this_emitter->zPos + this_emitter->zSpeed * this_emitter->timePassed;
				}
				// use frequency of -1 to 'turn off' emitter 
				if ( this_emitter->frequency == -1 ) return;

				if ( this_emitter->frequency < 1 )  this_emitter->frequency = 1;
				t.tAmountToMake = (int)( this_emitter->timePassed / (float)this_emitter->frequency );
				if ( t.tAmountToMake > RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER )  
					t.tAmountToMake = RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER;
				this_emitter->timePassed = 0;
				if ( t.tAmountToMake > this_emitter->maxPerFrame )  t.tAmountToMake = this_emitter->maxPerFrame;

				for ( t.tNewParticleCount = 0; t.tNewParticleCount < t.tAmountToMake; t.tNewParticleCount++ )
				{
					t.tfound = -1;
					for ( t.f = 0; t.f < RAVEY_PARTICLES_MAX; t.f++ )
					{
						if ( t.ravey_particles[t.f].inUse == 0 )
						{
							t.tfound = t.f;
							break;
						}
					}

					if ( t.tfound == -1 ) break;

					generateParticle( this_emitter );

					if ( this_emitter->numParticles >= this_emitter->maxParticles ) break;
				}
			}

			//  life
			if ( this_emitter->hasLife == 1 )
			{
				this_emitter->emitterLifePassed += g.ravey_particles_time_passed;
				if ( this_emitter->emitterLifePassed > this_emitter->emitterLife )
				{
					this_emitter->inUse = 0;
				}
			}
		}
	}
}

bool ravey_particles_update_particles( void )
{
	for ( t.c = 0; t.c < RAVEY_PARTICLES_MAX; t.c++ )
	{
		travey_particle* this_particle = &t.ravey_particles[ t.c ];

		if ( this_particle->inUse )
		{
			travey_particle_emitter* this_emitter = &t.ravey_particle_emitters[ this_particle->emitterID ];

			t.tobj = t.c + g.raveyparticlesobjectoffset;
			this_particle->timePassed += g.ravey_particles_time_passed;
			if ( this_particle->timePassed > this_particle->life )
			{
				this_particle->inUse = false;
				this_emitter->numParticles--;
				HideObject( t.tobj );

				if ( this_emitter->onDeathAction == 1 )
				{
					int numPts = Rnd( 3 );
					if ( numPts > 0 )
					{
						this_emitter->xPos = this_particle->x;
						this_emitter->yPos = this_particle->y;
						this_emitter->zPos = this_particle->z;
						this_emitter->maxParticles += ( numPts - 1 );
						return FALSE;
					}
					else
					{
						if ( this_emitter->maxParticles > 1 ) this_emitter->maxParticles--;
					}
				}
			}
			else
			{
				t.perc_f = ( this_particle->timePassed / this_particle->life );
				if ( t.perc_f < 0.0 )  t.perc_f = 0.0;
				if ( t.perc_f > 1.0 )  t.perc_f = 1.0;

				//  Alpha
				t.tAmount_f = this_particle->alphaStart + ( t.perc_f * ( this_particle->alphaEnd - this_particle->alphaStart ) );
				if ( t.tAmount_f < 0.0 )  t.tAmount_f = 0.0;
				if ( t.tAmount_f > 100.0 )  t.tAmount_f = 100.0;
				SetAlphaMappingOn( t.tobj, t.tAmount_f );

				//  Scale
				t.tAmount_f = this_particle->scaleStart + ( t.perc_f * ( this_particle->scaleEnd - this_particle->scaleStart ) );
				if ( t.tAmount_f < 0.0 )  t.tAmount_f = 0.0;
				//  can scale particles based on power of player
				if ( t.playercontrol.thirdperson.enabled == 1 )
				{
					t.tscaleme_f = t.tAmount_f*( t.player[ 1 ].powers.level / 100.0f );
					ScaleObject( t.tobj, t.tscaleme_f, t.tscaleme_f, t.tscaleme_f );
				}
				ScaleObject( t.tobj, t.tAmount_f, t.tAmount_f, t.tAmount_f );

				t.gravity_f = this_particle->startGravity + ( t.perc_f * ( this_particle->endGravity - this_particle->startGravity ) );

				if ( t.gravity_f < 0.0 ) t.gravity_f = 0.0;

				//  animation
				if ( this_particle->isAnimated == 1 )
				{
					this_particle->frame = this_particle->frame + this_particle->animationSpeed * g.ravey_particles_time_passed;
					if ( this_particle->frame < (float)this_particle->startFrame)  
						this_particle->frame = (float)this_particle->startFrame;
					if ( this_particle->frame > (float)this_particle->endFrame  )
					{
						if ( this_particle->isLooping == 1 )
						{
							this_particle->frame = (float)this_particle->startFrame;
						}
						else
						{
							this_particle->frame = (float)this_particle->endFrame;
						}
					}

					if ( int( this_particle->frame ) != this_particle->previousFrame )
					{
						float fa = Floor( this_particle->frame / this_particle->frameDivide );

						t.line_f = fa * this_particle->frameDivide;

						t.left_f = Floor( this_particle->frame - t.line_f ) * this_particle->frameMulti;

						t.top_f = fa * this_particle->frameMulti;

						ScaleObjectTexture( t.tobj, t.left_f, t.top_f );

						this_particle->previousFrame = int( this_particle->frame );
					}
				}

				//  Position and rotation
				// TBD - have position 'mode' setting, so 'particles' can be positioned relative to terrain for example.
				this_particle->x += this_particle->movementSpeedX * g.ravey_particles_time_passed;
				if ( !this_emitter->noWind ) this_particle->x += fwindVectX * g.ravey_particles_time_passed;
				this_particle->y += ( ( this_particle->movementSpeedY * g.ravey_particles_time_passed ) - t.gravity_f );
				this_particle->z += this_particle->movementSpeedZ * g.ravey_particles_time_passed;
				if ( !this_emitter->noWind ) this_particle->z += fwindVectZ * g.ravey_particles_time_passed;

				PositionObject( t.tobj, this_particle->x, this_particle->y, this_particle->z );

				if ( this_emitter->useAngle )
				{
					RotateObject( t.tobj, this_particle->xa, this_particle->ya, this_particle->za );
				}
				else
				{
					this_particle->rotz = this_particle->rotz + this_particle->rotateSpeedZ * g.ravey_particles_time_passed;
					while ( this_particle->rotz > 360 ) { this_particle->rotz = this_particle->rotz - 360; }
					while ( this_particle->rotz < 0 )   { this_particle->rotz = this_particle->rotz + 360; }
					//RollObjectRight ( t.tobj, this_particle->rotz );
					ZRotateObject( t.tobj, this_particle->rotz );
					PointObject( t.tobj, CameraPositionX(), CameraPositionY(), CameraPositionZ() );
				}
			}
		}
	}
	return TRUE;
}

void ravey_particles_free ( void )
{
	for ( t.c = 0 ; t.c <  RAVEY_PARTICLES_MAX; t.c++ )
	{
		t.tParticleObj = g.raveyparticlesobjectoffset+t.c;
		if (  ObjectExist(t.tParticleObj) == 1  )  DeleteObject (  t.tParticleObj );
	}
}

void ravey_particles_add_emitter( void )
{
	if (g.tEmitter.id >= RAVEY_PARTICLE_EMITTERS_MAX || g.tEmitter.id < 0)  return;

	travey_particle_emitter* this_emitter = &t.ravey_particle_emitters[g.tEmitter.id];

	this_emitter->id = g.tEmitter.id;
	this_emitter->inUse = 1;
	this_emitter->numParticles = 0;
	this_emitter->maxParticles = 100;
	this_emitter->firstParticle = TRUE;

	this_emitter->emitterLife = g.tEmitter.emitterLife;
	if (this_emitter->emitterLife > 0)
	{
		this_emitter->hasLife = 1;
		this_emitter->emitterLifePassed = 0;
	}
	else
	{
		this_emitter->hasLife = 0;
	}

	this_emitter->timePassed = 0;

	this_emitter->parentObject = g.tEmitter.parentObject;
	this_emitter->parentLimb = g.tEmitter.parentLimb;

	this_emitter->xPos = g.tEmitter.xPos;
	this_emitter->yPos = g.tEmitter.yPos;
	this_emitter->zPos = g.tEmitter.zPos;

	this_emitter->useAngle = false;

	this_emitter->isAnObjectEmitter = g.tEmitter.isAnObjectEmitter;
	this_emitter->imageNumber = g.tEmitter.imageNumber;

	this_emitter->isAnimated = g.tEmitter.isAnimated;
	this_emitter->animationSpeed = g.tEmitter.animationSpeed * 0.06f;
	this_emitter->frameCount = g.tEmitter.frameCount;
	this_emitter->startFrame = g.tEmitter.startFrame;
	this_emitter->endFrame = g.tEmitter.endFrame;
	this_emitter->frameDivide = 8;

	if ( this_emitter->frameCount == 49 )  this_emitter->frameDivide = 7;
	if ( this_emitter->frameCount == 36 )  this_emitter->frameDivide = 6;
	if ( this_emitter->frameCount == 25 )  this_emitter->frameDivide = 5;
	if ( this_emitter->frameCount == 16 )  this_emitter->frameDivide = 4;
	if ( this_emitter->frameCount == 9  )  this_emitter->frameDivide = 3;
	if ( this_emitter->frameCount == 4  )  this_emitter->frameDivide = 2;

	this_emitter->frameMulti = 1.0f / this_emitter->frameDivide;

	this_emitter->isLooping = g.tEmitter.isLooping;
	this_emitter->startsOffRandomAngle = g.tEmitter.startsOffRandomAngle;

	this_emitter->startGravity = g.tEmitter.startGravity;
	this_emitter->endGravity = g.tEmitter.endGravity;

	this_emitter->offsetMinX = g.tEmitter.offsetMinX;
	this_emitter->offsetMinY = g.tEmitter.offsetMinY;
	this_emitter->offsetMinZ = g.tEmitter.offsetMinZ;

	this_emitter->offsetMaxX = g.tEmitter.offsetMaxX;
	this_emitter->offsetMaxY = g.tEmitter.offsetMaxY;
	this_emitter->offsetMaxZ = g.tEmitter.offsetMaxZ;

	this_emitter->scaleStartMin = g.tEmitter.scaleStartMin;
	this_emitter->scaleStartMax = g.tEmitter.scaleStartMax;

	this_emitter->scaleEndMin = g.tEmitter.scaleEndMin;
	this_emitter->scaleEndMax = g.tEmitter.scaleEndMax;

	this_emitter->movementSpeedMinX = g.tEmitter.movementSpeedMinX * 0.06f;
	this_emitter->movementSpeedMinY = g.tEmitter.movementSpeedMinY * 0.06f;
	this_emitter->movementSpeedMinZ = g.tEmitter.movementSpeedMinZ * 0.06f;

	this_emitter->movementSpeedMaxX = g.tEmitter.movementSpeedMaxX * 0.06f;
	this_emitter->movementSpeedMaxY = g.tEmitter.movementSpeedMaxY * 0.06f;
	this_emitter->movementSpeedMaxZ = g.tEmitter.movementSpeedMaxZ * 0.06f;

	this_emitter->rotateSpeedMinX = g.tEmitter.rotateSpeedMinX * 0.06f;
	this_emitter->rotateSpeedMinY = g.tEmitter.rotateSpeedMinY * 0.06f;
	this_emitter->rotateSpeedMinZ = g.tEmitter.rotateSpeedMinZ * 0.06f;

	this_emitter->rotateSpeedMaxX = g.tEmitter.rotateSpeedMaxX * 0.06f;
	this_emitter->rotateSpeedMaxY = g.tEmitter.rotateSpeedMaxY * 0.06f;
	this_emitter->rotateSpeedMaxZ = g.tEmitter.rotateSpeedMaxZ * 0.06f;

	this_emitter->lifeMin = g.tEmitter.lifeMin;
	this_emitter->lifeMax = g.tEmitter.lifeMax;

	this_emitter->alphaStartMin = g.tEmitter.alphaStartMin;
	this_emitter->alphaStartMax = g.tEmitter.alphaStartMax;

	this_emitter->alphaEndMin = g.tEmitter.alphaEndMin;
	this_emitter->alphaEndMax = g.tEmitter.alphaEndMax;

	this_emitter->frequency = g.tEmitter.frequency;
}

void ravey_particles_delete_emitter ( void )
{
	t.ravey_particle_emitters[t.tRaveyParticlesEmitterID].inUse = 0;
}

void ravey_particles_delete_all_emitters ( void )
{
	for ( t.c = 0 ; t.c <  RAVEY_PARTICLE_EMITTERS_MAX; t.c++ )
	{
		t.ravey_particle_emitters[t.c].inUse = 0;
	}
}

void ravey_particles_get_free_emitter ( void )
{
	//  returns tResult = 0 (none free) or ID of free emitter
	for ( t.tEmit = 1 ; t.tEmit <  RAVEY_PARTICLE_EMITTERS_MAX; t.tEmit++ )
	{
		if (  t.ravey_particle_emitters[t.tEmit].inUse  ==  0 ) 
		{
			t.tResult = t.tEmit ; return;
		}
	}
	t.tResult = 0;
}

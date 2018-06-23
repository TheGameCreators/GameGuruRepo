//----------------------------------------------------
//--- GAMEGURU - M-Particles
//----------------------------------------------------

#include "gameguru.h"

void ravey_particles_init ( void )
{
	//  pre create max particles
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLES_MAX-1; t.c++ )
	{
		//  create particle object
		t.tParticleObj = g.raveyparticlesobjectoffset+t.c;
		if (  ObjectExist(t.tParticleObj) == 1  )  DeleteObject (  t.tParticleObj );
		MakeObjectPlane (  t.tParticleObj,100,100 );
		YRotateObject (  t.tParticleObj, 180 );
		FixObjectPivot (  t.tParticleObj );
		SetObjectTransparency (  t.tParticleObj,6 );
		SetObjectCollisionOff (  t.tParticleObj );
		DisableObjectZWrite (  t.tParticleObj );
		SetObjectTextureMode (  t.tParticleObj,0,0 );
		SetObjectLight (  t.tParticleObj,0 );
		SetObjectCull (  t.tParticleObj,1 );
		SetObjectMask (  t.tParticleObj,1 );
		HideObject (  t.tParticleObj );
		TextureObject (  t.tParticleObj,RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset );
		SetObjectEffect (  t.tParticleObj, g.decaleffectoffset );
		SetObjectCull ( t.tParticleObj, 0 );
		DisableObjectZWrite ( t.tParticleObj );
		SetObjectMask ( t.tParticleObj, 1 );

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
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLE_EMITTERS_MAX-1; t.c++ )
	{
		t.ravey_particle_emitters[t.c].inUse = 0;
	}

	//  reset particles
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLES_MAX-1; t.c++ )
	{
		t.ravey_particles[t.c].inUse = 0;
	}

	g.ravey_particles_old_time = 0;

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
}

void ravey_particles_load_images ( void )
{
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	//  Stock Particles (1400-1599 - most are custom loaded)
	//  Used for flare trails
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_FLARE + g.particlesimageoffset;
	if (  ImageExist(t.tImgID)  ==  0  )  LoadImage (  "effectbank\\particles\\flare.dds",t.tImgID );
	//  Used for rocket smoke trails
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_LIGHTSMOKE + g.particlesimageoffset;
	if (  ImageExist(t.tImgID)  ==  0  )  LoadImage (  "effectbank\\particles\\64smoke2.dds",t.tImgID );
	//  Used for fireball projectile
	t.tImgID = RAVEY_PARTICLES_IMAGETYPE_FLAME + g.particlesimageoffset;
	if (  ImageExist(t.tImgID)  ==  0  )  LoadImage (  "effectbank\\particles\\flame.dds",t.tImgID );
}

void ravey_particles_update ( void )
{

	if (  g.ravey_particles_old_time > 0 ) 
	{
		g.ravey_particles_time_passed = Timer() - g.ravey_particles_old_time;
	}
	g.ravey_particles_old_time = Timer();

	ravey_particles_update_particles ( );
	ravey_particles_update_emitters ( );
}

void ravey_particles_update_emitters ( void )
{
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLE_EMITTERS_MAX-1; t.c++ )
	{
		if (  t.ravey_particle_emitters[t.c].inUse ) 
		{
			//  move emitter by speed if it doesn't have a parent object
			if (  t.ravey_particle_emitters[t.c].parentObject  ==  0 ) 
			{
				t.ravey_particle_emitters[t.c].xPos = t.ravey_particle_emitters[t.c].xPos + t.ravey_particle_emitters[t.c].xSpeed * t.ravey_particle_emitters[t.c].timePassed;
				t.ravey_particle_emitters[t.c].yPos = t.ravey_particle_emitters[t.c].yPos + t.ravey_particle_emitters[t.c].ySpeed * t.ravey_particle_emitters[t.c].timePassed;
				t.ravey_particle_emitters[t.c].zPos = t.ravey_particle_emitters[t.c].zPos + t.ravey_particle_emitters[t.c].zSpeed * t.ravey_particle_emitters[t.c].timePassed;
			}

			//  time to spawn some particles?
			t.ravey_particle_emitters[t.c].timePassed += g.ravey_particles_time_passed;
			if (  t.ravey_particle_emitters[t.c].timePassed > t.ravey_particle_emitters[t.c].frequency ) 
			{
				if (  t.ravey_particle_emitters[t.c].frequency < 1  )  t.ravey_particle_emitters[t.c].frequency  =  1;
				t.tAmountToMake = t.ravey_particle_emitters[t.c].timePassed/t.ravey_particle_emitters[t.c].frequency;
				if (  t.tAmountToMake > RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER  )  t.tAmountToMake  =  RAVEY_PARTICLES_MAX_SPAWNED_AT_ONCE_BY_AN_EMITTER;
				t.ravey_particle_emitters[t.c].timePassed = 0;

				for ( t.tNewParticleCount = 1 ; t.tNewParticleCount<=  t.tAmountToMake; t.tNewParticleCount++ )
				{
					t.tfound = -1;
					for ( t.f = 0 ; t.f<=  RAVEY_PARTICLES_MAX-1; t.f++ )
					{
						if (  t.ravey_particles[t.f].inUse  ==  0 ) 
						{
							t.tfound = t.f;
							t.f = RAVEY_PARTICLES_MAX;
						}
					}
					//  add new particle
					if (  t.tfound  !=  -1 ) 
					{
						t.ravey_particles[t.tfound].inUse = 1;
						t.distx_f = (t.ravey_particle_emitters[t.c].offsetMinX - t.ravey_particle_emitters[t.c].offsetMaxX);
						t.distx_f = Sqrt(abs(t.distx_f*t.distx_f)) * 1000.0;
						//  Is there a parent object? if so use its position, if not use emitters xyz
						if (  t.ravey_particle_emitters[t.c].parentObject > 0 ) 
						{
							t.ravey_particles[t.tfound].x = LimbPositionX(t.ravey_particle_emitters[t.c].parentObject, t.ravey_particle_emitters[t.c].parentLimb) + t.ravey_particle_emitters[t.c].offsetMinX + Rnd(t.distx_f) / 1000.0;
							t.disty_f = (t.ravey_particle_emitters[t.c].offsetMinY - t.ravey_particle_emitters[t.c].offsetMaxY);
							t.disty_f = Sqrt(abs(t.disty_f*t.disty_f)) * 1000.0;
							t.ravey_particles[t.tfound].y = LimbPositionY(t.ravey_particle_emitters[t.c].parentObject, t.ravey_particle_emitters[t.c].parentLimb) + t.ravey_particle_emitters[t.c].offsetMinY + Rnd(t.disty_f) / 1000.0;
							t.distz_f = (t.ravey_particle_emitters[t.c].offsetMinZ - t.ravey_particle_emitters[t.c].offsetMaxZ);
							t.distz_f = Sqrt(abs(t.distz_f*t.distz_f) ) * 1000.0;
							t.ravey_particles[t.tfound].z = LimbPositionZ(t.ravey_particle_emitters[t.c].parentObject, t.ravey_particle_emitters[t.c].parentLimb) + t.ravey_particle_emitters[t.c].offsetMinZ + Rnd(t.distz_f) / 1000.0;
						}
						else
						{
							t.ravey_particles[t.tfound].x = t.ravey_particle_emitters[t.c].xPos + t.ravey_particle_emitters[t.c].offsetMinX + Rnd(t.distx_f) / 1000.0;
							t.disty_f = (t.ravey_particle_emitters[t.c].offsetMinY - t.ravey_particle_emitters[t.c].offsetMaxY);
							t.disty_f = Sqrt(abs(t.disty_f*t.disty_f)) * 1000.0;
							t.ravey_particles[t.tfound].y = t.ravey_particle_emitters[t.c].yPos + t.ravey_particle_emitters[t.c].offsetMinY + Rnd(t.disty_f) / 1000.0;
							t.distz_f = (t.ravey_particle_emitters[t.c].offsetMinZ - t.ravey_particle_emitters[t.c].offsetMaxZ);
							t.distz_f = Sqrt(abs(t.distz_f*t.distz_f) ) * 1000.0;
							t.ravey_particles[t.tfound].z = t.ravey_particle_emitters[t.c].zPos + t.ravey_particle_emitters[t.c].offsetMinZ + Rnd(t.distz_f) / 1000.0;
						}

						t.distx_f = (t.ravey_particle_emitters[t.c].movementSpeedMinX - t.ravey_particle_emitters[t.c].movementSpeedMaxX);
						t.distx_f = Sqrt(abs(t.distx_f*t.distx_f)  )  * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedX = t.ravey_particle_emitters[t.c].movementSpeedMinX  + Rnd(t.distx_f) / 1000.0;
						t.disty_f = (t.ravey_particle_emitters[t.c].movementSpeedMinY - t.ravey_particle_emitters[t.c].movementSpeedMaxY);
						t.disty_f = Sqrt(abs(t.disty_f*t.disty_f)) * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedY = t.ravey_particle_emitters[t.c].movementSpeedMinY  + Rnd(t.disty_f) / 1000.0;
						t.distz_f = (t.ravey_particle_emitters[t.c].movementSpeedMinZ - t.ravey_particle_emitters[t.c].movementSpeedMaxZ);
						t.distz_f = Sqrt(abs(t.distz_f*t.distz_f)) * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedZ = t.ravey_particle_emitters[t.c].movementSpeedMinZ  + Rnd(t.distz_f) / 1000.0;

						t.rotX_f = (t.ravey_particle_emitters[t.c].rotateSpeedMinX - t.ravey_particle_emitters[t.c].rotateSpeedMaxX);
						t.rotX_f = Sqrt(abs(t.rotX_f*t.rotX_f)  )  * 1000.0;
						t.ravey_particles[t.tfound].rotateSpeedX = t.ravey_particle_emitters[t.c].rotateSpeedMinX  + Rnd(t.rotX_f) / 1000.0;
						t.rotY_f = (t.ravey_particle_emitters[t.c].rotateSpeedMinY - t.ravey_particle_emitters[t.c].rotateSpeedMaxY);
						t.rotY_f = Sqrt(abs(t.rotY_f*t.rotY_f)) * 1000.0;
						t.ravey_particles[t.tfound].rotateSpeedY = t.ravey_particle_emitters[t.c].rotateSpeedMinY  + Rnd(t.rotY_f) / 1000.0;
						t.rotZ_f = (t.ravey_particle_emitters[t.c].rotateSpeedMinZ - t.ravey_particle_emitters[t.c].rotateSpeedMaxZ);
						t.rotZ_f = Sqrt(abs(t.rotZ_f*t.rotZ_f)) * 1000.0;
						t.ravey_particles[t.tfound].rotateSpeedZ = t.ravey_particle_emitters[t.c].rotateSpeedMinZ  + Rnd(t.rotZ_f) / 1000.0;
						t.ravey_particles[t.tfound].rotz = 0;

						t.alphaSize_f = (t.ravey_particle_emitters[t.c].alphaStartMin - t.ravey_particle_emitters[t.c].alphaStartMax);
						t.alphaSize_f = Sqrt(abs(t.alphaSize_f*t.alphaSize_f));
						t.ravey_particles[t.tfound].alpha = t.ravey_particle_emitters[t.c].alphaStartMin + Rnd(t.alphaSize_f);

						t.scaleSize_f = (t.ravey_particle_emitters[t.c].scaleStartMin - t.ravey_particle_emitters[t.c].scaleStartMax);
						t.scaleSize_f = Sqrt(abs(t.scaleSize_f*t.scaleSize_f)) * 1000.0;
						t.ravey_particles[t.tfound].scaleStart = t.ravey_particle_emitters[t.c].scaleStartMin + Rnd(t.scaleSize_f) / 1000.0;
						t.scaleSize_f = (t.ravey_particle_emitters[t.c].scaleEndMin - t.ravey_particle_emitters[t.c].scaleEndMax);
						t.scaleSize_f = Sqrt(abs(t.scaleSize_f*t.scaleSize_f)) * 1000.0;
						t.ravey_particles[t.tfound].scaleEnd = t.ravey_particle_emitters[t.c].scaleEndMin + Rnd(t.scaleSize_f) / 1000.0;

						t.moveSize_f = (t.ravey_particle_emitters[t.c].movementSpeedMinX - t.ravey_particle_emitters[t.c].movementSpeedMaxX);
						t.moveSize_f = Sqrt(abs(t.moveSize_f*t.moveSize_f)) * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedX = t.ravey_particle_emitters[t.c].movementSpeedMinX + (Rnd(t.moveSize_f) / 1000.0);
						t.moveSize_f = (t.ravey_particle_emitters[t.c].movementSpeedMinY - t.ravey_particle_emitters[t.c].movementSpeedMaxY);
						t.moveSize_f = Sqrt(abs(t.moveSize_f*t.moveSize_f)) * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedY = t.ravey_particle_emitters[t.c].movementSpeedMinY + (Rnd(t.moveSize_f) / 1000.0);
						t.moveSize_f = (t.ravey_particle_emitters[t.c].movementSpeedMinZ - t.ravey_particle_emitters[t.c].movementSpeedMaxZ);
						t.moveSize_f = Sqrt(abs(t.moveSize_f*t.moveSize_f)) * 1000.0;
						t.ravey_particles[t.tfound].movementSpeedZ = t.ravey_particle_emitters[t.c].movementSpeedMinZ + (Rnd(t.moveSize_f) / 1000.0);

						t.lifeSize_f = (t.ravey_particle_emitters[t.c].lifeMin - t.ravey_particle_emitters[t.c].lifeMax);
						t.lifeSize_f = Sqrt(abs(t.lifeSize_f*t.lifeSize_f));
						t.ravey_particles[t.tfound].life = t.ravey_particle_emitters[t.c].lifeMin + Rnd(t.lifeSize_f);
						t.ravey_particles[t.tfound].timePassed = 0;

						t.ravey_particles[t.tfound].alphaStart = t.ravey_particles[t.tfound].alpha;
						t.alphaSize_f = (t.ravey_particle_emitters[t.c].alphaEndMin - t.ravey_particle_emitters[t.c].alphaEndMax);
						t.alphaSize_f = Sqrt(abs(t.alphaSize_f*t.alphaSize_f));
						t.ravey_particles[t.tfound].alphaEnd = t.ravey_particle_emitters[t.c].alphaEndMin + Rnd(t.alphaSize_f);

						t.tobj = t.tfound+g.raveyparticlesobjectoffset;
						PositionObject (  t.tobj,t.ravey_particles[t.tfound].x,t.ravey_particles[t.tfound].y,t.ravey_particles[t.tfound].z );
						TextureObject (  t.tobj,0,t.ravey_particle_emitters[t.c].imageNumber );
						SetAlphaMappingOn (  t.tobj, t.ravey_particles[t.c].alpha );
						ScaleObject (  t.tobj, t.ravey_particles[t.tfound].scaleStart,t.ravey_particles[t.tfound].scaleStart,t.ravey_particles[t.tfound].scaleStart );
						PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
						//  defeat Z clash by moving particle to camera in
						++t.ravey_particle_emitters[t.c].zclashfixcounter;
						if (  t.ravey_particle_emitters[t.c].zclashfixcounter>10  )  t.ravey_particle_emitters[t.c].zclashfixcounter = 0;
						MoveObject (  t.tobj,t.ravey_particle_emitters[t.c].zclashfixcounter*0.1 );

						//  animation
						t.ravey_particles[t.tfound].frameDivide = t.ravey_particle_emitters[t.c].frameDivide;
						t.ravey_particles[t.tfound].frameMulti = t.ravey_particle_emitters[t.c].frameMulti;
						t.ravey_particles[t.tfound].startFrame = t.ravey_particle_emitters[t.c].startFrame;
						t.ravey_particles[t.tfound].endFrame = t.ravey_particle_emitters[t.c].endFrame;

						t.ravey_particles[t.tfound].frame = t.ravey_particles[t.tfound].startFrame;
						t.ravey_particles[t.tfound].previousFrame = t.ravey_particles[t.tfound].startFrame;
						t.ravey_particles[t.tfound].isAnimated = t.ravey_particle_emitters[t.c].isAnimated;
						t.ravey_particles[t.tfound].isLooping = t.ravey_particle_emitters[t.c].isLooping;
						t.ravey_particles[t.tfound].animationSpeed = t.ravey_particle_emitters[t.c].animationSpeed;

						t.ravey_particles[t.tfound].startGravity = t.ravey_particle_emitters[t.c].startGravity;
						t.ravey_particles[t.tfound].endGravity = t.ravey_particle_emitters[t.c].endGravity;

						if (  t.ravey_particle_emitters[t.c].startsOffRandomAngle  ==  0 ) 
						{
							ZRotateObject (  t.tobj,0 );
						}
						else
						{
							ZRotateObject (  t.tobj,Rnd(360) );
						}

						if (  t.ravey_particles[t.tfound].isAnimated  ==  1 ) 
						{
							t.line_f = Floor(t.ravey_particles[t.c].frame/t.ravey_particles[t.c].frameDivide) ;
							t.line_f = t.line_f * t.ravey_particles[t.c].frameDivide;
							t.left_f = (Floor(t.ravey_particles[t.c].frame)-t.line_f) * t.ravey_particles[t.c].frameMulti ;
							t.right_f = t.left_f + t.ravey_particles[t.c].frameMulti;
							t.top_f = Floor(t.ravey_particles[t.c].frame/t.ravey_particles[t.c].frameDivide) * t.ravey_particles[t.c].frameMulti;
							t.bottom_f = t.top_f + t.ravey_particles[t.c].frameMulti;
							ScaleObjectTexture (  t.tobj,t.left_f,t.top_f );
						}

						ShowObject (  t.tobj );
					}
					//  end of new particle
					}
			}

			//  life
			if (  t.ravey_particle_emitters[t.c].hasLife  ==  1 ) 
			{
				t.ravey_particle_emitters[t.c].emitterLifePassed += g.ravey_particles_time_passed;
				if (  t.ravey_particle_emitters[t.c].emitterLifePassed > t.ravey_particle_emitters[t.c].emitterLife ) 
				{
					t.ravey_particle_emitters[t.c].inUse = 0;
				}
			}

		}
	}
}

void ravey_particles_update_particles ( void )
{
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLES_MAX-1; t.c++ )
	{
		if (  t.ravey_particles[t.c].inUse ) 
		{
			t.tobj = t.c+g.raveyparticlesobjectoffset;
			t.ravey_particles[t.c].timePassed += g.ravey_particles_time_passed;
			if (  t.ravey_particles[t.c].timePassed > t.ravey_particles[t.c].life ) 
			{
				t.ravey_particles[t.c].inUse = false;
				HideObject (  t.tobj );
			}
			else
			{
				t.perc_f = (t.ravey_particles[t.c].timePassed / t.ravey_particles[t.c].life);
				if (  t.perc_f < 0.0  )  t.perc_f  =  0.0;
				if (  t.perc_f > 1.0  )  t.perc_f  =  1.0;

				//  Alpha
				t.tAmount_f = t.ravey_particles[t.c].alphaStart + (t.perc_f * (t.ravey_particles[t.c].alphaEnd-t.ravey_particles[t.c].alphaStart));
				if (  t.tAmount_f < 0.0  )  t.tAmount_f  =  0.0;
				if (  t.tAmount_f > 100.0  )  t.tAmount_f  =  100.0;
				SetAlphaMappingOn (  t.tobj,t.tAmount_f );

				//  Scale
				t.tAmount_f = t.ravey_particles[t.c].scaleStart + (t.perc_f * (t.ravey_particles[t.c].scaleEnd-t.ravey_particles[t.c].scaleStart));
				if (  t.tAmount_f < 0.0  )  t.tAmount_f  =  0.0;
				//  can scale particles based on power of player
				if (  t.playercontrol.thirdperson.enabled == 1 ) 
				{
					t.tscaleme_f=t.tAmount_f*(t.player[1].powers.level/100.0);
					ScaleObject (  t.tobj,t.tscaleme_f,t.tscaleme_f,t.tscaleme_f );
				}
				ScaleObject (  t.tobj,t.tAmount_f,t.tAmount_f,t.tAmount_f );

				t.tAmount_f = t.ravey_particles[t.c].startGravity + (t.perc_f * (t.ravey_particles[t.c].endGravity-t.ravey_particles[t.c].startGravity));
				t.gravity_f = t.tAmount_f;
				if (  t.gravity_f < 0.0  )  t.gravity_f  =  0.0;

				//  animation
				if (  t.ravey_particles[t.c].isAnimated  ==  1 ) 
				{
					t.ravey_particles[t.c].frame = t.ravey_particles[t.c].frame +  t.ravey_particles[t.c].animationSpeed * g.ravey_particles_time_passed;
					if (  t.ravey_particles[t.c].frame < t.ravey_particles[t.c].startFrame  )  t.ravey_particles[t.c].frame  =  t.ravey_particles[t.c].startFrame;
					if (  t.ravey_particles[t.c].frame > t.ravey_particles[t.c].endFrame ) 
					{
						if (  t.ravey_particles[t.c].isLooping  ==  1 ) 
						{
							t.ravey_particles[t.c].frame = t.ravey_particles[t.c].startFrame;
						}
						else
						{
							t.ravey_particles[t.c].frame = t.ravey_particles[t.c].endFrame;
						}
					}

					if (  int(t.ravey_particles[t.c].frame)  !=   t.ravey_particles[t.c].previousFrame ) 
					{
						t.line_f = Floor(t.ravey_particles[t.c].frame/t.ravey_particles[t.c].frameDivide) ;
						t.line_f = t.line_f * t.ravey_particles[t.c].frameDivide;
						t.left_f = (Floor(t.ravey_particles[t.c].frame)-t.line_f) * t.ravey_particles[t.c].frameMulti ;
						t.right_f = t.left_f + t.ravey_particles[t.c].frameMulti;
						t.top_f = Floor(t.ravey_particles[t.c].frame/t.ravey_particles[t.c].frameDivide) * t.ravey_particles[t.c].frameMulti;
						t.bottom_f = t.top_f + t.ravey_particles[t.c].frameMulti;
						ScaleObjectTexture (  t.tobj,t.left_f,t.top_f );
						t.ravey_particles[t.c].previousFrame = int(t.ravey_particles[t.c].frame);
					}

				}

				//  Position and rotation
				PositionObject (  t.tobj,ObjectPositionX(t.tobj) + (t.ravey_particles[t.c].movementSpeedX * g.ravey_particles_time_passed), -t.gravity_f + ObjectPositionY(t.tobj) + (t.ravey_particles[t.c].movementSpeedY * g.ravey_particles_time_passed),ObjectPositionZ(t.tobj) + (t.ravey_particles[t.c].movementSpeedZ * g.ravey_particles_time_passed) );
				t.rot_f = ObjectAngleZ(t.tobj);
				PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
				t.ravey_particles[t.c].rotz = t.ravey_particles[t.c].rotz + t.ravey_particles[t.c].rotateSpeedZ * g.ravey_particles_time_passed;
				while ( t.rotz > 360.0 ) { t.rotz = t.rotz - 360.0; }
				while ( t.rotz < 0.0 ) { t.rotz = t.rotz + 360.0; }
				RollObjectRight (  t.tobj,t.ravey_particles[t.c].rotz );
			}
		}
	}
}

void ravey_particles_free ( void )
{
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLES_MAX-1; t.c++ )
	{
		t.tParticleObj = g.raveyparticlesobjectoffset+t.c;
		if (  ObjectExist(t.tParticleObj) == 1  )  DeleteObject (  t.tParticleObj );
	}
}

void ravey_particles_add_emitter ( void )
{
	if (  g.tEmitter.id > RAVEY_PARTICLE_EMITTERS_MAX || g.tEmitter.id < 0  )  return;

	t.ravey_particle_emitters[g.tEmitter.id].id = g.tEmitter.id;
	t.ravey_particle_emitters[g.tEmitter.id].inUse = 1;

	t.ravey_particle_emitters[g.tEmitter.id].emitterLife = g.tEmitter.emitterLife;
	if (  t.ravey_particle_emitters[g.tEmitter.id].emitterLife > 0 ) 
	{
		t.ravey_particle_emitters[g.tEmitter.id].hasLife = 1;
		t.ravey_particle_emitters[g.tEmitter.id].emitterLifePassed = 0;
	}
	else
	{
		t.ravey_particle_emitters[g.tEmitter.id].hasLife = 0;
	}

	t.ravey_particle_emitters[g.tEmitter.id].timePassed = 0;

	t.ravey_particle_emitters[g.tEmitter.id].parentObject = g.tEmitter.parentObject;
	t.ravey_particle_emitters[g.tEmitter.id].parentLimb = g.tEmitter.parentLimb;

	t.ravey_particle_emitters[g.tEmitter.id].xPos = g.tEmitter.xPos;
	t.ravey_particle_emitters[g.tEmitter.id].yPos = g.tEmitter.yPos;
	t.ravey_particle_emitters[g.tEmitter.id].zPos = g.tEmitter.zPos;

	t.ravey_particle_emitters[g.tEmitter.id].isAnObjectEmitter = g.tEmitter.isAnObjectEmitter;
	t.ravey_particle_emitters[g.tEmitter.id].imageNumber = g.tEmitter.imageNumber;

	t.ravey_particle_emitters[g.tEmitter.id].isAnimated = g.tEmitter.isAnimated;
	t.ravey_particle_emitters[g.tEmitter.id].animationSpeed = g.tEmitter.animationSpeed * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].frameCount = g.tEmitter.frameCount;
	t.ravey_particle_emitters[g.tEmitter.id].startFrame = g.tEmitter.startFrame;
	t.ravey_particle_emitters[g.tEmitter.id].startGravity = g.tEmitter.startGravity;
	t.ravey_particle_emitters[g.tEmitter.id].endGravity = g.tEmitter.endGravity;
	t.ravey_particle_emitters[g.tEmitter.id].endFrame = g.tEmitter.endFrame;
	t.ravey_particle_emitters[g.tEmitter.id].frameDivide = 8;
	if (  t.ravey_particle_emitters[g.tEmitter.id].frameCount  ==  32  )  t.ravey_particle_emitters[g.tEmitter.id].frameDivide  =  4;
	if (  t.ravey_particle_emitters[g.tEmitter.id].frameCount  ==  16  )  t.ravey_particle_emitters[g.tEmitter.id].frameDivide  =  2;
	t.ravey_particle_emitters[g.tEmitter.id].frameMulti = 1.0 / t.ravey_particle_emitters[g.tEmitter.id].frameDivide;

	t.ravey_particle_emitters[g.tEmitter.id].isLooping = g.tEmitter.isLooping;
	t.ravey_particle_emitters[g.tEmitter.id].startsOffRandomAngle = g.tEmitter.startsOffRandomAngle;

	t.ravey_particle_emitters[g.tEmitter.id].offsetMinX = g.tEmitter.offsetMinX;
	t.ravey_particle_emitters[g.tEmitter.id].offsetMinY = g.tEmitter.offsetMinY;
	t.ravey_particle_emitters[g.tEmitter.id].offsetMinZ = g.tEmitter.offsetMinZ;

	t.ravey_particle_emitters[g.tEmitter.id].offsetMaxX = g.tEmitter.offsetMaxX;
	t.ravey_particle_emitters[g.tEmitter.id].offsetMaxY = g.tEmitter.offsetMaxY;
	t.ravey_particle_emitters[g.tEmitter.id].offsetMaxZ = g.tEmitter.offsetMaxZ;

	t.ravey_particle_emitters[g.tEmitter.id].scaleStartMin = g.tEmitter.scaleStartMin;
	t.ravey_particle_emitters[g.tEmitter.id].scaleStartMax = g.tEmitter.scaleStartMax;

	t.ravey_particle_emitters[g.tEmitter.id].scaleEndMin = g.tEmitter.scaleEndMin;
	t.ravey_particle_emitters[g.tEmitter.id].scaleEndMax = g.tEmitter.scaleEndMax;

	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMinX = g.tEmitter.movementSpeedMinX * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMinY = g.tEmitter.movementSpeedMinY * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMinZ = g.tEmitter.movementSpeedMinZ * 0.06;

	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMaxX = g.tEmitter.movementSpeedMaxX * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMaxY = g.tEmitter.movementSpeedMaxY * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].movementSpeedMaxZ = g.tEmitter.movementSpeedMaxZ * 0.06;

	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMinX = g.tEmitter.rotateSpeedMinX * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMinY = g.tEmitter.rotateSpeedMinY * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMinZ = g.tEmitter.rotateSpeedMinZ * 0.06;

	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMaxX = g.tEmitter.rotateSpeedMaxX * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMaxY = g.tEmitter.rotateSpeedMaxY * 0.06;
	t.ravey_particle_emitters[g.tEmitter.id].rotateSpeedMaxZ = g.tEmitter.rotateSpeedMaxZ * 0.06;

	t.ravey_particle_emitters[g.tEmitter.id].lifeMin = g.tEmitter.lifeMin;
	t.ravey_particle_emitters[g.tEmitter.id].lifeMax = g.tEmitter.lifeMax;

	t.ravey_particle_emitters[g.tEmitter.id].alphaStartMin = g.tEmitter.alphaStartMin;
	t.ravey_particle_emitters[g.tEmitter.id].alphaStartMax = g.tEmitter.alphaStartMax;

	t.ravey_particle_emitters[g.tEmitter.id].alphaEndMin = g.tEmitter.alphaEndMin;
	t.ravey_particle_emitters[g.tEmitter.id].alphaEndMax = g.tEmitter.alphaEndMax;

	t.ravey_particle_emitters[g.tEmitter.id].frequency = g.tEmitter.frequency;
}

void ravey_particles_delete_emitter ( void )
{
	t.ravey_particle_emitters[t.tRaveyParticlesEmitterID].inUse = 0;
}

void ravey_particles_delete_all_emitters ( void )
{
	for ( t.c = 0 ; t.c<=  RAVEY_PARTICLE_EMITTERS_MAX-1; t.c++ )
	{
		t.ravey_particle_emitters[t.c].inUse = 0;
	}
}

void ravey_particles_get_free_emitter ( void )
{
	//  returns tResult = 0 (none free) or ID of free emitter
	for ( t.tEmit = 1 ; t.tEmit<=  RAVEY_PARTICLE_EMITTERS_MAX - 1; t.tEmit++ )
	{
		if (  t.ravey_particle_emitters[t.tEmit].inUse  ==  0 ) 
		{
			t.tResult = t.tEmit ; return;
		}
	}
	t.tResult = 0;
}

//----------------------------------------------------
//--- GAMEGURU - M-Explosion&Fire
//----------------------------------------------------

#include "gameguru.h"

// 
//  Explosions and fire
// 

void explosion_init ( void )
{
	g.rubbletext=15+g.explosionsandfireimagesoffset;
	g.cretetext=16+g.explosionsandfireimagesoffset;
	g.metaltext=17+g.explosionsandfireimagesoffset;
	g.largeexplosion=10+g.explosionsandfireimagesoffset;
	g.sparks=12+g.explosionsandfireimagesoffset;
	g.largeexplosion2=13+g.explosionsandfireimagesoffset;
	g.smokedecal2=14+g.explosionsandfireimagesoffset;
	g.rollingsmoke=20+g.explosionsandfireimagesoffset;
	g.grenadeexplosion=21+g.explosionsandfireimagesoffset;

	g.rubbleobj=1+g.explosionsandfireobjectoffset;
	g.creteobj=2+g.explosionsandfireobjectoffset;
	g.metalobj=3+g.explosionsandfireobjectoffset;

	//  Maxemit=10 emitters, with totalpart of 260 for each emitter.
	//Dave Performance
	g.totalpart=260;
	//Dave Performance
	g.maxemit=5;
	//  Max Debris
	g.debrismax=5;
	//  place holder object pointers
	g.explosionparticleobjectstart=20+g.explosionsandfireobjectoffset;
	g.explosiondebrisobjectstart=2700+g.explosionsandfireobjectoffset;

	//  Explosion art
	LoadImage (  "effectbank\\explosion\\animatedspark.dds",g.sparks,1 );
	LoadImage (  "effectbank\\explosion\\explosion2.dds",g.largeexplosion, 1 );
	LoadImage (  "effectbank\\explosion\\fireball.dds",g.largeexplosion2, 1 );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	LoadImage (  "effectbank\\explosion\\rollingsmoke.dds",g.rollingsmoke,1 );
	LoadImage (  "effectbank\\explosion\\explosion3.dds",g.grenadeexplosion,1 );
	LoadImage (  "effectbank\\explosion\\darksmoke.dds",g.smokedecal2,1 );
	//  Temp rubble
	LoadImage (  "effectbank\\explosion\\rubble.dds",g.rubbletext,1 );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	LoadObject (  "effectbank\\explosion\\rubble.dbo",g.rubbleobj );
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	TextureObject (  g.rubbleobj,g.rubbletext );
	ScaleObject (  g.rubbleobj,100,100,100 );
	HideObject (  g.rubbleobj );
	//  Temp rubble
	LoadImage (  "effectbank\\explosion\\concretechunk.dds",g.cretetext,1 );
	LoadObject (  "effectbank\\explosion\\concretechunk.dbo",g.creteobj );

	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );

	TextureObject (  g.creteobj,g.cretetext );
	HideObject (  g.creteobj );
	//  Temp rubble
	LoadImage (  "effectbank\\explosion\\metalchunk.dds",g.metaltext,1 );
	LoadObject (  "effectbank\\explosion\\metalchunk.dbo",g.metalobj );
	TextureObject (  g.metalobj,g.metaltext );
	HideObject (  g.metalobj );
	//  debris data
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	Dim (  t.debris,g.debrismax  );
	Dim2(  t.particle,g.maxemit, g.totalpart);
	make_particles();
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
	make_debris();
	if ( t.game.runasmultiplayer == 1 ) steam_refresh ( );
}

void explosion_cleanup ( void )
{
	//  remove debris
	for ( t.draw = 1 ; t.draw<=  g.debrismax; t.draw++ )
	{
		if (  t.draw <= ArrayCount(t.debris) ) 
		{
			if (  t.debris[t.draw].used == 1 && ObjectExist(t.debris[t.draw].obj) == 1 ) 
			{
				t.debris[t.draw].used=0;
				if (  t.debris[t.draw].physicscreated == 1 ) 
				{
					ODEDestroyObject (  t.debris[t.draw].obj );
					t.debris[t.draw].physicscreated=0;
				}
				HideObject (  t.debris[t.draw].obj );
			}
		}
	}

	//  remove particles
	for ( t.emitter = 1 ; t.emitter<=  g.maxemit; t.emitter++ )
	{
		if (  t.emitter <= ArrayCount(t.particle) ) 
		{
			for ( t.draw = 1 ; t.draw<=  g.totalpart; t.draw++ )
			{
				//  check if particle used
				if (  t.particle[t.emitter][t.draw].used != 0 ) 
				{
					//  kill particle
					t.particle[t.emitter][t.draw].used = 0;
					if (  t.particle[t.emitter][t.draw].physicscreated == 1 ) 
					{
						ODEDestroyObject (  t.particle[t.emitter][t.draw].obj );
						t.particle[t.emitter][t.draw].physicscreated=0;
					}
					HideObject (  t.particle[t.emitter][t.draw].obj );
				}
			}
		}
	}

return;

}

void explosion_free ( void )
{
}

void draw_debris ( void )
{
	int tx = 0;
	int ty = 0;
	int tz = 0;
	int draw = 0;
	//  Draw debris and update, run through debris
	for ( draw = 1 ; draw <= g.debrismax; draw++ )
	{
		if (  t.debris[draw].used == 1 && ObjectExist(t.debris[draw].obj) == 1 ) 
		{
			//  if active
			PositionObject (  t.debris[draw].obj,t.debris[draw].x,t.debris[draw].y,t.debris[draw].z );
			ShowObject (  t.debris[draw].obj );
			//  set to spinning
			tx=WrapValue(ObjectAngleX(t.debris[draw].obj)+5);
			ty=WrapValue(ObjectAngleY(t.debris[draw].obj)+5);
			tz=WrapValue(ObjectAngleZ(t.debris[draw].obj)+5);
			RotateObject (  t.debris[draw].obj,tx,ty,tz );
			//  kill if life expired
			if (  Timer()-t.debris[draw].lifetime>t.debris[draw].life ) 
			{
				t.debris[draw].used=0;
				if (  t.debris[draw].physicscreated == 1 ) 
				{
					ODEDestroyObject (  t.debris[draw].obj );
					t.debris[draw].physicscreated=0;
				}
				HideObject (  t.debris[draw].obj );
			}
		}
	}
}

void draw_particles ( void )
{
	int tonceforemitter = 0;
	int hideframe = 0;
	int endframe = 0;
	int emitter = 0;
	int doit = 0;
	int size = 0;
	int tobj = 0;
	int zz = 0;
	int draw = 0;
	for ( emitter = 1 ; emitter<=  g.maxemit; emitter++ )
	{
		tonceforemitter=0;
		for ( draw = 1 ; draw<=  g.totalpart; draw++ )
		{

			//  check if particle used
			if (  t.particle[emitter][draw].used != 0 ) 
			{
			//  check for delayed particles
			doit=1;
			if (  t.particle[emitter][draw].activein != 0 ) 
			{
				zz=Timer()-t.particle[emitter][draw].activetime;
				if (  zz<t.particle[emitter][draw].activein  )  doit = 0;
			}

			//  if active but dead, ensure particle is killed
			if (  doit == 1 ) 
			{

				//  size is the number of frames in an animation, current 4x4 and 8x8 supported.
				endframe=0;
				if (  t.particle[emitter][draw].size == 4 ) 
				{
					endframe=16;
				}
				else
				{
					endframe=64;
				}
				size=t.particle[emitter][draw].size;

				//  if explosion show quick flash, in full version should be replaced with FPSC spotflash.
				if (  t.particle[emitter][draw].etype == 2 || t.particle[emitter][draw].etype == 7 || t.particle[emitter][draw].etype == 6 ) 
				{
					//  Show explosion light
				}

				//  control alpha and apply new value if it's changed
				t.particle[emitter][draw].oldalpha=t.particle[emitter][draw].alpha;
				if (  t.particle[emitter][draw].fadein != 0 ) 
				{
					if (  t.particle[emitter][draw].fadedir == 0 ) 
					{
						t.particle[emitter][draw].alpha=t.particle[emitter][draw].alpha+t.particle[emitter][draw].fadein;
						if (  t.particle[emitter][draw].alpha>t.particle[emitter][draw].fademax ) 
						{
							t.particle[emitter][draw].alpha=t.particle[emitter][draw].fademax;
							if (  t.particle[emitter][draw].fadeout>0  )  t.particle[emitter][draw].fadedir = 1;
						}
						if (  t.particle[emitter][draw].alpha<0 && t.particle[emitter][draw].fadein != 0  )  t.particle[emitter][draw].alpha = 0;
					}
					if (  t.particle[emitter][draw].fadedir == 1 ) 
					{
						t.particle[emitter][draw].alpha=t.particle[emitter][draw].alpha-t.particle[emitter][draw].fadeout;
						if (  t.particle[emitter][draw].alpha<0 ) 
						{
							t.particle[emitter][draw].alpha=0;
							t.particle[emitter][draw].fadedir=2;
						}
					}
				}
				if (  t.particle[emitter][draw].oldalpha != t.particle[emitter][draw].alpha ) 
				{
					SetAlphaMappingOn (  t.particle[emitter][draw].obj,t.particle[emitter][draw].alpha );
				}
				//  if particle is a fire, generate sparks every 2.5 seconds
				if (  t.particle[emitter][draw].etype == 5 || t.particle[emitter][draw].etype == 12 || t.particle[emitter][draw].etype == 13 ) 
				{
					if (  Timer()-t.particle[emitter][draw].time>2500 ) 
					{
						Create_Emitter(t.particle[emitter][draw].x,t.particle[emitter][draw].y,t.particle[emitter][draw].z,14,Rnd(2)+1,g.sparkdecal,0,0,0,0,0,0);
						t.particle[emitter][draw].time=Timer();
						if (  t.particle[emitter][draw].etype == 5 && t.particle[emitter][draw].nextframe>35 ) 
						{
							make_large_fire(t.particle[emitter][draw].x,t.particle[emitter][draw].y-20,t.particle[emitter][draw].z);
						}
					}
				}
				//  move particles using at pre designed speed allowing for a little chaos. Fires have no movement
				if (  t.particle[emitter][draw].etype != 5 && t.particle[emitter][draw].etype != 11 && t.particle[emitter][draw].etype != 12 && t.particle[emitter][draw].etype != 13 ) 
				{
					t.particle[emitter][draw].x=t.particle[emitter][draw].x+t.particle[emitter][draw].vx+Rnd(t.particle[emitter][draw].vxvar_f);
					t.particle[emitter][draw].y=t.particle[emitter][draw].y+t.particle[emitter][draw].vy+Rnd(t.particle[emitter][draw].vyvar_f);
					t.particle[emitter][draw].z=t.particle[emitter][draw].z+t.particle[emitter][draw].vz+Rnd(t.particle[emitter][draw].vzvar_f);
				}
				//  update
				PositionObject (  t.particle[emitter][draw].obj,t.particle[emitter][draw].x,t.particle[emitter][draw].y,t.particle[emitter][draw].z );
				//  rem point at camera
				tobj=t.particle[emitter][draw].obj;
				PointObject (  tobj,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
				//C++ISSUE - zbias doesnt seem to work for this, so set it to -1 for now
				//EnableObjectZBias (  tobj,0.0f,-1.0f );
				// this causes explosion to render in front of things when it should be behind/amongst (like grass)
				// MoveObject ( tobj , 200 );
				hideframe=0;
				if (  t.particle[emitter][draw].etype == 2 && t.particle[emitter][draw].nextframe == 1  )  hideframe = 1;
				if (  hideframe == 0  )  ShowObject (  t.particle[emitter][draw].obj );
				//  update animation frame if time.
				if (  Timer()-t.particle[emitter][draw].lastanitime>t.particle[emitter][draw].anispeed ) 
				{
					t.particle[emitter][draw].lastanitime=Timer();
					Set_Object_Frame_Update(t.particle[emitter][draw].obj,t.particle[emitter][draw].nextframe,size,size);
					++t.particle[emitter][draw].nextframe;
					//  reset if animation at last frame, kill if not looped
					if (  t.particle[emitter][draw].nextframe >= endframe ) 
					{
						if (  t.particle[emitter][draw].playforloops == 0 || (t.particle[emitter][draw].alpha <= 0 && t.particle[emitter][draw].fadein<0) ) 
						{
							t.particle[emitter][draw].used=0;
							t.particle[emitter][draw].nextframe=0;
						}
						else
						{
							--t.particle[emitter][draw].playforloops;
							t.particle[emitter][draw].nextframe=1;
							if (  t.particle[emitter][draw].playforloops<3 && (t.particle[emitter][draw].etype == 5 || t.particle[emitter][draw].etype == 12 || t.particle[emitter][draw].etype == 13 || t.particle[emitter][draw].etype == 14) || (t.particle[emitter][draw].playforloops == 0 && t.particle[emitter][draw].etype == 1) ) 
							{
								//  control alpha fading if on way out. Small fire fades faster
								if (  t.particle[emitter][draw].etype != 14 ) 
								{
									t.particle[emitter][draw].fadein=- 0.3f;
								}
								else
								{
									t.particle[emitter][draw].fadein=-0.5f;
								}
							}
						}
					}
				}

				//  if particle has a life counter, kill when exceeds it
				if (  t.particle[emitter][draw].life != 0 ) 
				{
					if (  Timer()>t.particle[emitter][draw].life ) 
					{
						t.particle[emitter][draw].used=0;
					}
				}
				if (  t.particle[emitter][draw].alpha <= 0 && t.particle[emitter][draw].fadein != 0 ) 
				{
					t.particle[emitter][draw].used=0;
				}

				//  hide/destroy if no longer used
				if (  t.particle[emitter][draw].used == 0 ) 
				{
					//  kill particle
					if (  t.particle[emitter][draw].physicscreated == 1 ) 
					{
						ODEDestroyObject (  t.particle[emitter][draw].obj );
						t.particle[emitter][draw].physicscreated=0;
					}
					HideObject (  t.particle[emitter][draw].obj );
				}

			}

			//  particle used endif
			}

		}
	}
}

//  create emitters using presets.
int Create_Emitter ( int x, int y, int z, int etype, int part, int textureid, int delay, int scale, int damage, int sound, int volume, int loopsound )
{
	float variation_f = 0;
	int forceangle = 0;
	float funangle_f = 0;
	float tangle_f = 0;
	float tforce_f = 0;
	int emitter = 0;
	float fundx_f = 0;
	float fundy_f = 0;
	int num = 0;
	int use = 0;

	//  find free emitter, if none free exit
	emitter=find_free_emitter();
	if (  emitter == 0  )  return 0;

	switch (  etype ) 
	{
		//  make small smoke
		case 1:
			for ( num = 1 ; num<=  part; num++ )
			{
				//  smoke starts at particle 1
				use=find_free_particle(emitter,1,10);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*10);
								t.particle[emitter][use].y=y+(num*(Rnd(32)+2));
								t.particle[emitter][use].z=z+(Cos(variation_f)*10);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=1;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=60;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );

								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,0 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.02f;
								t.particle[emitter][use].vy=1+(Rnd(0.2f)+(num*0.07f));
								t.particle[emitter][use].vz=0.001f;
								t.particle[emitter][use].vxvar_f=0.04f;
								t.particle[emitter][use].vyvar_f=0.05f+(num*0.02f);
								t.particle[emitter][use].vzvar_f=0.002f;
								t.particle[emitter][use].alpha=0;
								t.particle[emitter][use].fadein=0.6f;
								t.particle[emitter][use].fademax=255;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].activeframe=Rnd(62)+1;
							}
						}
				}
			}
		break;
		//  small explosion starts at particle 1
		case 2:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_particle(emitter,1,9);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x;
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=2;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								//  life in millisecs
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=55;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.001f;
								t.particle[emitter][use].vy=0.001f+(num*0.002f);
								t.particle[emitter][use].vz=-0.002f;
								t.particle[emitter][use].vxvar_f=0.002f;
								t.particle[emitter][use].vyvar_f=0.003f;
								t.particle[emitter][use].vzvar_f=-0.01f;
							}
						}
				}
			}
		break;
		//  3= Debris//uses objects over particles - object related and use ODE
		case 3:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_debris();
				if (  use != 0 ) 
				{
					variation_f=Rnd(360);
					t.debris[use].obj=use+g.explosiondebrisobjectstart;
					t.debris[use].damage=damage;
					t.debris[use].x=x+(Sin(variation_f)*6);
					t.debris[use].y=y+Rnd(5);
					t.debris[use].z=z+(Cos(variation_f)*6);
					t.debris[use].delay=delay;
					t.debris[use].used=1;
					t.debris[use].life=4800;
					t.debris[use].lifetime=Timer();
					tforce_f=5.0;
					SetAlphaMappingOn (  t.debris[use].obj,100 );
					if (  t.debris[use].physicscreated == 1  )  ODEDestroyObject (  t.debris[use].obj );
					t.debris[use].physicscreated=1;
					PositionObject (  t.debris[use].obj,t.debris[use].x,t.debris[use].y,t.debris[use].z );
					ODECreateDynamicBox (  t.debris[use].obj,-1,11 );
					ODESetLinearVelocity (  t.debris[use].obj,(-75+Rnd(148))*tforce_f,(155+Rnd(35))*tforce_f,(-74+Rnd(128))*tforce_f );
					ODESetAngularVelocity (  t.debris[use].obj,Rnd(15),Rnd(15),Rnd(15) );
					ODESetBodyMass ( t.debris[use].obj, 200 );
				}
			}
		break;
		//  4 = sparks from explosion, uses physics
		case 4:
			for ( num = 1 ; num<=  part; num++ )
			{
				//  smaller particles start at 41
				use=find_free_particle(emitter,41,110);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*8);
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z+(Cos(variation_f)*8);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].size=4;
								t.particle[emitter][use].etype=7;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=4;
								t.particle[emitter][use].life=Timer()+1500;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=35;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].nextframe=1;
								forceangle=WrapValue(-90+Rnd(180));
								tforce_f=2.5f;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,30 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6);
								PositionObject (  t.particle[emitter][use].obj,t.particle[emitter][use].x,t.particle[emitter][use].y,t.particle[emitter][use].z );
								if (  t.particle[emitter][use].physicscreated == 1  )  ODEDestroyObject (  t.particle[emitter][use].obj );
								t.particle[emitter][use].physicscreated=1;
								ODECreateDynamicBox (  t.particle[emitter][use].obj,-1,11 );
								ODESetLinearVelocity (  t.particle[emitter][use].obj,(Rnd(25))*tforce_f,(30+Rnd(205))*tforce_f,(-Rnd(25)+Rnd(25))*tforce_f );
								ODESetAngularVelocity (  t.particle[emitter][use].obj,0,forceangle,0 );
								ODEAddBodyForce (  t.particle[emitter][use].obj, 0, 0.05f, 0,0,-50,0 );
							}
						}
				}
			}
		break;
		//  fire - large
		case 5:
			for ( num = 1 ; num<=  part; num++ )
			{
			//  flame starts at 111
			use=find_free_particle(emitter,111,116);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=2+Rnd(2);
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x;
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=5;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=20;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );

								t.particle[emitter][use].nextframe=Rnd(8)+1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,0 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vy=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vxvar_f=0;
								t.particle[emitter][use].vyvar_f=0;
								t.particle[emitter][use].vzvar_f=0;
								t.particle[emitter][use].fadein=0.4f;
								t.particle[emitter][use].fademax=255;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].alpha=0;
								t.particle[emitter][use].time=Timer()+Rnd(1000);
							}
						}
				}
			}
		break;
		//  large explosion starts at particle 131
		case 6:
			for ( num = 1 ; num<=  part; num++ )
			{
				use=find_free_particle(emitter,131,160);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
					if (  t.particle[emitter][use].obj>0 ) 
					{
						if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
						{
							variation_f=Rnd(360);
							t.particle[emitter][use].playforloops=0;
							t.particle[emitter][use].actionperformed=0;
							t.particle[emitter][use].damage=damage;
							t.particle[emitter][use].x=x;
							t.particle[emitter][use].y=y+(ObjectSize(t.particle[emitter][use].obj)/4);
							t.particle[emitter][use].z=z;
							t.particle[emitter][use].used=1;
							t.particle[emitter][use].etype=6;
							t.particle[emitter][use].size=8;
							t.particle[emitter][use].rotate=0;
							t.particle[emitter][use].activein=delay;
							t.particle[emitter][use].activetime=Timer();
							t.particle[emitter][use].life=0;
							t.particle[emitter][use].lasttime=Timer();
							t.particle[emitter][use].lastanitime=Timer();
							t.particle[emitter][use].anispeed=25;
							t.particle[emitter][use].fadein=10.0f;
							t.particle[emitter][use].fademax=100;
							t.particle[emitter][use].fadedir=0;
							t.particle[emitter][use].fadeout=1.0f;
							TextureObject (  t.particle[emitter][use].obj,0,textureid );
							t.particle[emitter][use].nextframe=1;
							SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
							SetObjectTransparency (  t.particle[emitter][use].obj,6 );
							t.particle[emitter][use].vx=-0.001f;
							t.particle[emitter][use].vy=0.001f+(num*0.002f);
							t.particle[emitter][use].vz=-0.002f;
							t.particle[emitter][use].vxvar_f=0.002f;
							t.particle[emitter][use].vyvar_f=0.003f;
							t.particle[emitter][use].vzvar_f=-0.01f;
						}
					}
				}
			}
		break;
		case 7:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_particle(emitter,131,160);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x;
								t.particle[emitter][use].y=y+(ObjectSizeY(t.particle[emitter][use].obj)/4);
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=7;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=35;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.001f;
								t.particle[emitter][use].vy=0.001f+(num*0.002f);
								t.particle[emitter][use].vz=-0.002f;
								t.particle[emitter][use].vxvar_f=0.002f;
								t.particle[emitter][use].vyvar_f=0.003f;
								t.particle[emitter][use].vzvar_f=-0.01f;
							}
						}
				}
			}
		break;
		//  large smoke starts at particle 131
		case 8:
			for ( num = 1 ; num<=  part; num++ )
			{
				//  large smoke starts at particle 131
				use=find_free_particle(emitter,131,160);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
					if (  t.particle[emitter][use].obj>0 ) 
					{
						if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
						{
							variation_f=Rnd(360);
							t.particle[emitter][use].playforloops=0;
							t.particle[emitter][use].actionperformed=0;
							t.particle[emitter][use].damage=damage;
							t.particle[emitter][use].x=x+(Sin(variation_f)*1);
							t.particle[emitter][use].y=y+(num*(Rnd(32)+2));
							t.particle[emitter][use].z=z+(Cos(variation_f)*1);
							t.particle[emitter][use].used=1;
							t.particle[emitter][use].etype=8;
							t.particle[emitter][use].size=8;
							t.particle[emitter][use].activein=delay;
							t.particle[emitter][use].activetime=Timer();
							t.particle[emitter][use].rotate=0;
							t.particle[emitter][use].life=0;
							t.particle[emitter][use].lasttime=Timer();
							t.particle[emitter][use].lastanitime=Timer();
							t.particle[emitter][use].anispeed=55;
							t.particle[emitter][use].fadein=1.0;
							t.particle[emitter][use].fademax=100;
							t.particle[emitter][use].fadedir=0;
							t.particle[emitter][use].fadeout=1.0;
							TextureObject (  t.particle[emitter][use].obj,0,textureid );
							t.particle[emitter][use].nextframe=1;
							SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
							SetObjectTransparency (  t.particle[emitter][use].obj,6 );
							t.particle[emitter][use].vx=-0.02f;
							t.particle[emitter][use].vy=3+Rnd(2)+(num*0.03f);
							t.particle[emitter][use].vz=0.001f;
							t.particle[emitter][use].vxvar_f=0.04f;
							t.particle[emitter][use].vyvar_f=0.5f+(num*0.02f);
							t.particle[emitter][use].vzvar_f=0.002f;
						}
					}
				}
			}
		break;
		//  large rolling smoke starts at particle 251
		case 9:
			for ( num = 1 ; num<=  part; num++ )
			{
				use=find_free_particle(emitter,251,260);
				if (  use != 0 ) 
				{
					if (  t.particle[emitter][use].obj>0 ) 
					{
						if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
						{
							variation_f=Rnd(360);
							t.particle[emitter][use].playforloops=0;
							t.particle[emitter][use].actionperformed=0;
							t.particle[emitter][use].damage=damage;
							t.particle[emitter][use].x=x+(Sin(variation_f)*1);
							t.particle[emitter][use].y=y+(num*(Rnd(32)+2));
							t.particle[emitter][use].z=z+(Cos(variation_f)*1);
							t.particle[emitter][use].used=1;
							t.particle[emitter][use].etype=1;
							t.particle[emitter][use].size=8;
							t.particle[emitter][use].activein=delay;
							t.particle[emitter][use].activetime=Timer();
							t.particle[emitter][use].rotate=0;
							t.particle[emitter][use].life=0;
							t.particle[emitter][use].lasttime=Timer();
							t.particle[emitter][use].lastanitime=Timer();
							t.particle[emitter][use].anispeed=45;
							t.particle[emitter][use].fadein=10.0f;
							t.particle[emitter][use].fademax=100;
							t.particle[emitter][use].fadedir=0;
							t.particle[emitter][use].fadeout=0.6f;
							TextureObject (  t.particle[emitter][use].obj,textureid );
							t.particle[emitter][use].nextframe=0;
							SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
							SetObjectTransparency (  t.particle[emitter][use].obj,6);//2 );
							//  always face camera
							fundx_f=t.particle[emitter][use].x-CameraPositionX();
							fundy_f=CameraPositionZ()-t.particle[emitter][use].z;
							funangle_f=atan2deg(fundy_f,fundx_f);
							tangle_f=180-funangle_f;
							t.particle[emitter][use].vx=Cos(tangle_f)*(Rnd(2));
							t.particle[emitter][use].vy=Rnd(0.2f)+(num*.03);
							t.particle[emitter][use].vz=Sin(tangle_f)*(Rnd(2));
							t.particle[emitter][use].vxvar_f=0;
							t.particle[emitter][use].vyvar_f=(num*0.02f);
							t.particle[emitter][use].vzvar_f=0;
						}
					}
				}
			}
		break;
		//  small smoke starts at particle 1
		case 10:
			for ( num = 1 ; num<=  part; num++ )
			{
				use=find_free_particle(emitter,1,10);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*1);
								t.particle[emitter][use].y=y+(num*(Rnd(32)+2));
								t.particle[emitter][use].z=z+(Cos(variation_f)*1);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=10;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=55;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.02f;
								t.particle[emitter][use].vy=0.8f+Rnd(0.2f)+(num*0.03f);
								t.particle[emitter][use].vz=0.001f;
								t.particle[emitter][use].vxvar_f=0.04f;
								t.particle[emitter][use].vyvar_f=0.5f+(num*0.02f);
								t.particle[emitter][use].vzvar_f=0.002f;
							}
						}
				}
			}
		break;
		//  small flame starts at 111
		case 11:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_particle(emitter,111,130);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x;
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=11;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=55;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,0 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vy=0;
								t.particle[emitter][use].vx=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vxvar_f=0;
								t.particle[emitter][use].vyvar_f=0;
								t.particle[emitter][use].vzvar_f=0;
								t.particle[emitter][use].fadein=0.3f;
								t.particle[emitter][use].fademax=180;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].alpha=0;
							}
						}
				}
			}
		break;
		//  flame meduim starts at 117
		case 12:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_particle(emitter,117,122);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=2+Rnd(2);
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*15);
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z+(Cos(variation_f)*15);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=12;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=20;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );

								t.particle[emitter][use].nextframe=Rnd(8)+1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,0 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vy=0;
								t.particle[emitter][use].vx=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vxvar_f=0;
								t.particle[emitter][use].vyvar_f=0;
								t.particle[emitter][use].vzvar_f=0;
								t.particle[emitter][use].fadein=0.3f;
								t.particle[emitter][use].fademax=255;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].alpha=0;
								t.particle[emitter][use].time=Timer()+Rnd(1000);
							}
						}
				}
			}
		break;
		//  large flame starts at 123
		case 13:
			for ( num = 1 ; num<=  part; num++ )
			{
			use=find_free_particle(emitter,123,130);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=2+Rnd(2);
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*7);
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z+(Cos(variation_f)*7);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=13;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=20;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=Rnd(8)+1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,0 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vy=0;
								t.particle[emitter][use].vx=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vxvar_f=0;
								t.particle[emitter][use].vyvar_f=0;
								t.particle[emitter][use].vzvar_f=0;
								t.particle[emitter][use].fadein=0.3f;
								t.particle[emitter][use].fademax=255;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].alpha=0;
								t.particle[emitter][use].time=Timer()+Rnd(1000);
							}
						}
				}
			}
		break;
		//  14 = slow fire sparks
		case 14:
			for ( num = 1 ; num<=  part; num++ )
			{
			//  very small particles start at 81
			use=find_free_particle(emitter,81,110);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*8);
								t.particle[emitter][use].y=y+Rnd(25);
								t.particle[emitter][use].z=z+(Cos(variation_f)*8);
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].size=4;
								t.particle[emitter][use].etype=14;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=4;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=Rnd(100)+45;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,75 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6);
								PositionObject (  t.particle[emitter][use].obj,t.particle[emitter][use].x,t.particle[emitter][use].y,t.particle[emitter][use].z );
								t.particle[emitter][use].vy=Rnd(1)+0.5f;
								t.particle[emitter][use].vx=0;
								t.particle[emitter][use].vz=0;
								t.particle[emitter][use].vxvar_f=0;
								t.particle[emitter][use].vyvar_f=Rnd(0.8f);
								t.particle[emitter][use].vzvar_f=0;
								t.particle[emitter][use].fadein=0.3f;
								t.particle[emitter][use].fademax=200;
								t.particle[emitter][use].fadedir=0;
								t.particle[emitter][use].fadeout=0;
								t.particle[emitter][use].alpha=0;
								t.particle[emitter][use].time=Timer()+Rnd(1000);
							}
						}
				}
			}
		break;
		case 15:
			for ( num = 1 ; num<=  part; num++ )
			{
			//  large smoke starts at particle 11
			use=find_free_particle(emitter,11,40);
				if (  use != 0 ) 
				{
					if (  t.particle[emitter][use].obj>0 ) 
					{
						if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
						{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x+(Sin(variation_f)*1);
								t.particle[emitter][use].y=y+(num*(Rnd(32)+2));
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=1;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=35;
								TextureObject (  t.particle[emitter][use].obj,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,90 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.02f;
								t.particle[emitter][use].vy=0.8f+Rnd(0.2f)+(num*0.03f);
								t.particle[emitter][use].vz=0.001f;
								t.particle[emitter][use].vxvar_f=0.04f;
								t.particle[emitter][use].vyvar_f=0.5f+(num*0.02f);
								t.particle[emitter][use].vzvar_f=0.002f;
							}
						}
				}
			}
		break;
//   `16=explosion normal - grenade

		case 16:
			for ( num = 1 ; num<=  part; num++ )
			{
			//  large explosion starts at particle 1
			use=find_free_particle(emitter,1,9);
				if (  use != 0 ) 
				{
					reset_current_particle(emitter,use);
						if (  t.particle[emitter][use].obj>0 ) 
						{
							if (  ObjectExist(t.particle[emitter][use].obj) == 1 ) 
							{
								variation_f=Rnd(360);
								t.particle[emitter][use].playforloops=0;
								t.particle[emitter][use].actionperformed=0;
								t.particle[emitter][use].damage=damage;
								t.particle[emitter][use].x=x;
								t.particle[emitter][use].y=y;
								t.particle[emitter][use].z=z;
								t.particle[emitter][use].used=1;
								t.particle[emitter][use].etype=16;
								t.particle[emitter][use].size=8;
								t.particle[emitter][use].activein=delay;
								t.particle[emitter][use].activetime=Timer();
								t.particle[emitter][use].rotate=0;
								t.particle[emitter][use].life=0;
								t.particle[emitter][use].lasttime=Timer();
								t.particle[emitter][use].lastanitime=Timer();
								t.particle[emitter][use].anispeed=25;
								TextureObject (  t.particle[emitter][use].obj,0,textureid );
								t.particle[emitter][use].nextframe=1;
								SetAlphaMappingOn (  t.particle[emitter][use].obj,100 );
								SetObjectTransparency (  t.particle[emitter][use].obj,6 );
								t.particle[emitter][use].vx=-0.001f;
								t.particle[emitter][use].vy=0.001f+(num*0.002f);
								t.particle[emitter][use].vz=-0.002f;
								t.particle[emitter][use].vxvar_f=0.002f;
								t.particle[emitter][use].vyvar_f=0.003f;
								t.particle[emitter][use].vzvar_f=-0.01f;
							}
						}
				}
			}
		break;
	}
	return emitter;
}

void Set_Object_Frame ( int tempobj, int currentframe, int height_f, int width_f )
{
	float xmove_f = 0;
	float ymove_f = 0;
	float U_f = 0;
	float V_f = 0;
	//  write out base UVs for Sprite (  anim (updated inside shader later with 'scale object Text ( ure' below) ) )
	U_f=0.0;
	V_f=0.0;
	xmove_f=1.0f/height_f;
	ymove_f=1.0f/width_f;
	LockVertexDataForLimb (  tempobj , 0 );
	SetVertexDataUV (  0,U_f+xmove_f,V_f );
	SetVertexDataUV (  1,U_f,V_f );
	SetVertexDataUV (  2,U_f+xmove_f,V_f+ymove_f );
	SetVertexDataUV (  3,U_f,V_f );
	SetVertexDataUV (  4,U_f,V_f+ymove_f );
	SetVertexDataUV (  5,U_f+xmove_f,V_f+ymove_f );
	UnlockVertexData (  );
	//  reset effect UV scaling
	ScaleObjectTexture (  tempobj,0,0 );
}

void Set_Object_Frame_Update ( int tempobj, int currentframe, int height_f, int width_f )
{
	float xmove_f = 0;
	float ymove_f = 0;
	int across = 0;
	float U_f = 0;
	float V_f = 0;
	xmove_f=1.0/height_f;
	ymove_f=1.0/width_f;
	if ( currentframe != 0 )
	{
		across=int(currentframe/height_f);
		V_f=ymove_f*across;
		U_f=currentframe*xmove_f;
	}
	else
	{
		U_f=0;
		V_f=0;
	}
	//  replace vertex data write with special feed into shader (values write to shader constant UVScaling)
	ScaleObjectTexture (  tempobj,U_f,V_f );
//endfunction

}

void make_debris ( void )
{
	int nextobject = 0;
	int rand = 0;
	int make = 0;
	//  make and intilise debris objects
	for ( make = 1 ; make<=  g.debrismax; make++ )
	{
		nextobject=g.explosiondebrisobjectstart+make;
		rand=g.rubbleobj+Rnd(2);
		CloneObject (  nextobject,rand );
		PositionObject (  nextobject,0,0,0 );
		ScaleObject (  nextobject,Rnd(20)+15,Rnd(20)+15,Rnd(20)+15 );
		t.debris[make].obj=nextobject;
		t.debris[make].used=0;
		HideObject (  nextobject );
	}
}

void make_particles ( void )
{
	int nextobject = 0;
	int emitter = 0;
	int tscale = 0;
	int scale = 0;
	int make = 0;
	//  make and intilise particle objects, different types for different tasks.
	nextobject=g.explosionparticleobjectstart;
		for ( emitter = 1 ; emitter<=  g.maxemit; emitter++ )
		{
				for ( make = 1 ; make<=  g.totalpart; make++ )
				{
					++nextobject;
					scale=180+Rnd(80);
						//  Explosion particles
						if (  make <= 10 ) 
						{
							MakeObjectPlane (  nextobject,200,200 );
							//MakeObjectCube ( nextobject , 200 );
						}
						//  Major smoke
						if (  make>10 && make <= 40 ) 
						{
							MakeObjectPlane (  nextobject,scale,scale );
							//MakeObjectCube ( nextobject , scale );
						}
						if (  make>40 && make <= 80 ) 
						{
							MakeObjectPlane (  nextobject,2+Rnd(2),2+Rnd(2) );
							//MakeObjectCube ( nextobject , 2+Rnd(2) );
						}
						if (  make>80 && make <= 110 ) 
						{
							tscale=1+Rnd(1);
							MakeObjectPlane (  nextobject,tscale,tscale );
							//MakeObjectCube ( nextobject , tscale );
						}
						if (  make>110 && make <= 116 ) 
						{
							MakeObjectPlane (  nextobject,90,70 );
							//MakeObjectCube ( nextobject , 80 );
						}
						if (  make>116 && make <= 122 ) 
						{
						MakeObjectPlane (  nextobject,60,60 );
						//MakeObjectCube ( nextobject , 60 );
							}
						if (  make>122 && make <= 130 ) 
						{
							MakeObjectPlane (  nextobject,30,30 );
							//MakeObjectCube ( nextobject , 30 );
						}
						//  big bang
						if (  make>130 && make <= 160 ) 
						{
							MakeObjectPlane (  nextobject,400,400 );
							//MakeObjectCube ( nextobject , 400 );
						}
						if (  make>160 && make <= 250 ) 
						{
							MakeObjectPlane (  nextobject,10,10 );
							//MakeObjectCube ( nextobject , 10 );
						}
						if (  make>250 && make <= 261 ) 
						{
							MakeObjectPlane (  nextobject,400,200 );
							//MakeObjectCube ( nextobject , 300 );
						}
						TextureObject ( nextobject,g.smokedecal );
						SetObjectTransparency ( nextobject, 6 );

						// apply decal shader
						SetObjectEffect ( nextobject, g.decaleffectoffset );
						SetObjectCull ( nextobject, 0 );
						DisableObjectZWrite ( nextobject );
						SetObjectMask ( nextobject, 1 );

						Set_Object_Frame(nextobject,0,8,8);
						HideObject (  nextobject );
						t.particle[emitter][make].obj=nextobject;
				}
		}
//endfunction

}

int find_free_debris ( void )
{
	int gotone = 0;
	int find = 0;
//  Find unused debris object
	gotone=0;
	for ( find = 1 ; find<=  30; find++ )
	{
		if (  t.debris[find].used == 0 ) 
		{
			t.debris[find].used=1;
			gotone=find;
			find=31;
		}
	}
//endfunction gotone
	return gotone
;
}

int find_free_particle ( int emitter, int start, int endpart )
{
	int gotone = 0;
	int find = 0;
//  Find unsed particle emitter
	gotone=0;
	for ( find = start ; find<=  endpart; find++ )
	{
		if (  t.particle[emitter][find].used == 0 ) 
		{
			gotone=find;
			find=endpart+1;
		}
	}
//endfunction gotone
	return gotone;
}

int find_free_emitter ( void )
{
	int gotone = 0;
	int find = 0;
	gotone=0;	
	for ( find = 1 ; find<=  g.maxemit; find++ )
	{
		if (  t.particle[find][1].used == 0 ) 
		{
		gotone=find;
		find=g.maxemit+1;
		}
	}
//endfunction gotone
	return gotone
;
}

void make_large_fire ( int x, int y, int z )
{
	int firesprite = 0;
	int firetotal = 0;
	int tx = 0;
	int tz = 0;
	int fire = 0;
	//  make large fire, 3 stages
	firesprite=0;
	firetotal=1;
	for ( fire = 1 ; fire<=  firetotal; fire++ )
	{
		tx=x;
		tz=z;
		Create_Emitter(tx+8,y,tz,13,1,g.afterburndecal+firesprite,0,0,0,5,100,1);
		Create_Emitter(tx,y+12,tz,12,1,g.afterburndecal+firesprite,500,0,20,0,0,0);
		Create_Emitter(tx,y+20,tz,5,1,g.afterburndecal+firesprite,1000,0,20,5,100,1);
		Create_Emitter(x+8,y+20,z,12,2+Rnd(1),g.afterburndecal+firesprite,1000,0,0,5,100,1);
	}
	//  create smoke randomly
	if (  Rnd(100)>50 ) 
	{
		Create_Emitter(x,y+35,z,1,1,g.smokedecal2,700,2,0,0,0,0);
	}
//endfunction

}

void make_medium_fire ( int x, int y, int z )
{
	int firesprite = 0;
	//  make medium fire, 2 stages
	firesprite=0;
	Create_Emitter(x+8,y,z,13,1,g.afterburndecal+firesprite,0,0,20,5,100,1);
	Create_Emitter(x,y+12,z,12,2+Rnd(1),g.afterburndecal+firesprite,400,0,20,5,100,1);
	if (  Rnd(100)>50 ) 
	{
		Create_Emitter(x,y+35,z,1,1,g.smokedecal2,1000,2,0,0,0,0);
	}
//endfunction

}

void make_small_fire ( int x, int y, int z )
{
	int firesprite = 0;
	//  make small fire
	firesprite=0;
	Create_Emitter(x+8,y,z,13,2+Rnd(1),g.afterburndecal+firesprite,0,0,0,5,100,1);
	if (  Rnd(100)>50  )  Create_Emitter(x,y+35,z,1,1,g.smokedecal2,700,0,0,0,0,0);
//endfunction

}

void explosion_rocket ( int x, int y, int z )
{
	int f = 0;
	//  explosion
	Create_Emitter(x-Rnd(2),y+50,z,6,2,g.largeexplosion,0,0,20,2,100,0);
	//  sparks
	for ( f = 0 ; f <= 4; f++ )
	{
		Create_Emitter(x,y+30,z,4,16,g.sparks,70+(f*500),1,0,0,0,0);
	}
	//  rubble - removed for now - not consistent with material exploding!
//  `Create_Emitter(x,y,z,3,Rnd(3)+5,5,700,2,0,0,0,0)

//endfunction

}

void explosion_grenade ( int x, int y, int z )
{
	int f = 0;
	//  grenade explosion
	Create_Emitter(x-Rnd(2),y+40,z,16,1,g.grenadeexplosion,80,0,20,2,100,0);
	//  grenade explosion second smoke
	Create_Emitter(x,y+60,z,15,1,g.smokedecal2,60,1200,20,0,100,0);
	//  sparks
	for ( f = 0 ; f <= 4; f++ )
	{
		Create_Emitter(x,y,z,4,5,g.sparks,70+(f*500),1,0,0,0,0);
	}
//endfunction

}

void explosion_fireball ( int x, int y, int z )
{
	int temitterindex = 0;
	int tmodeindex = 0;
	/*float tx_f = 0;
	float ty_f = 0;
	float tz_f = 0;*/
	int f = 0;
	//  grenade explosion
	temitterindex=Create_Emitter(x-Rnd(2),y,z,16,1,g.grenadeexplosion,80,0,20,2,100,0);
	//  start dynamic light blast
	t.playerlight.mode=0 ; t.tx_f=x ; t.ty_f=y ; t.tz_f=z ; tmodeindex=temitterindex ; lighting_spotflashexplosion ( );
	//  grenade explosion second smoke
	Create_Emitter(x,y+10,z,15,1,g.smokedecal2,60,1200,20,0,100,0);
	//  sparks
	for ( f = 0 ; f <= 7; f++ )
	{
		Create_Emitter(x,y,z,4,5,g.sparks,70+(f*500),1,0,0,0,0);
	}
//endfunction

}

void reset_current_particle ( int emitter, int use )
{
	int damage = 0;
	t.particle[emitter][use].actionperformed=0;
	t.particle[emitter][use].damage=damage;
	t.particle[emitter][use].x=-100000;
	t.particle[emitter][use].y=-100000;
	t.particle[emitter][use].z=-100000;
	t.particle[emitter][use].used=0;
	t.particle[emitter][use].etype=0;
	t.particle[emitter][use].size=0;
	t.particle[emitter][use].activein=0;
	t.particle[emitter][use].activetime=Timer();
	t.particle[emitter][use].rotate=0;
	t.particle[emitter][use].life=0;
	t.particle[emitter][use].lasttime=Timer();
	t.particle[emitter][use].lastanitime=Timer();
	t.particle[emitter][use].anispeed=0;
	t.particle[emitter][use].nextframe=0;
	t.particle[emitter][use].vx=0;
	t.particle[emitter][use].vy=0;
	t.particle[emitter][use].vz=0;
	t.particle[emitter][use].vxvar_f=0;
	t.particle[emitter][use].vyvar_f=0;
	t.particle[emitter][use].vzvar_f=0;
	t.particle[emitter][use].alpha=0;
	t.particle[emitter][use].fadein=0;
	t.particle[emitter][use].fademax=0;
	t.particle[emitter][use].fadedir=0;
	t.particle[emitter][use].fadeout=0;
	t.particle[emitter][use].activeframe=0;
	t.particle[emitter][use].oldalpha=0;
//endfunction

}


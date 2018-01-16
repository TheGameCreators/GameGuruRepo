//----------------------------------------------------
//--- GAMEGURU - M-Decal
//----------------------------------------------------

#include "gameguru.h"

// 
//  Decal Module
// 

void decal_init ( void )
{

//  Load in all decal
decal_scaninallref ( );

//  Precreate elements as each is unique (UV writing)
for ( t.f = 1 ; t.f<=  g.decalelementmax; t.f++ )
{
	t.tobj=g.decalelementoffset+t.f ; t.decalelement[t.f].obj=t.tobj;
	t.decalelement[t.f].uvgridsize=0;
	if (  ObjectExist(t.tobj) == 1  )  DeleteObject (  t.tobj );
	MakeObjectPlane (  t.tobj,100,100 );
	SetObjectTransparency (  t.tobj, 6 );
	SetObjectCollisionOff (  t.tobj );
	DisableObjectZWrite (  t.tobj );
	SetObjectTextureMode (  t.tobj,2,0 );
	SetObjectLight (  t.tobj,0 );
	SetObjectCull (  t.tobj,0 );
	HideObject (  t.tobj );
}

//  ensure fixed decals available
t.decalglobal.splashdecalrippleid=0;
t.decalglobal.splashdecalsmallid=0;
t.decalglobal.splashdecallargeid=0;
t.decalglobal.splashdecalmistyid=0;
t.decalglobal.splashdecaldropletsid=0;
t.decalglobal.splashdecalfoamid=0;
t.decalglobal.dustflumeid=0;
t.decalglobal.impactid=0;
t.decalglobal.bloodsplatid=0;
for ( t.tdscan = 1 ; t.tdscan<=  9; t.tdscan++ )
{
	if (  t.tdscan == 1  )  t.decal_s = "splash_ripple";
	if (  t.tdscan == 2  )  t.decal_s = "splash_small";
	if (  t.tdscan == 3  )  t.decal_s = "splash_large";
	if (  t.tdscan == 4  )  t.decal_s = "splash_misty";
	if (  t.tdscan == 5  )  t.decal_s = "splash_droplets";
	if (  t.tdscan == 6  )  t.decal_s = "splash_foam";
	if (  t.tdscan == 7  )  t.decal_s = "dustflume";
	if (  t.tdscan == 8  )  t.decal_s = "impact";
	if (  t.tdscan == 9  )  t.decal_s = "bloodsplat";
	decal_find ( );
	if (  t.decalid>0 ) 
	{
		if (  t.tdscan == 1  )  t.decalglobal.splashdecalrippleid = t.decalid;
		if (  t.tdscan == 2  )  t.decalglobal.splashdecalsmallid = t.decalid;
		if (  t.tdscan == 3  )  t.decalglobal.splashdecallargeid = t.decalid;
		if (  t.tdscan == 4  )  t.decalglobal.splashdecalmistyid = t.decalid;
		if (  t.tdscan == 5  )  t.decalglobal.splashdecaldropletsid = t.decalid;
		if (  t.tdscan == 6  )  t.decalglobal.splashdecalfoamid = t.decalid;
		if (  t.tdscan == 7  )  t.decalglobal.dustflumeid = t.decalid;
		if (  t.tdscan == 8  )  t.decalglobal.impactid = t.decalid;
		if (  t.tdscan == 9  )  t.decalglobal.bloodsplatid = t.decalid;
	}
}

return;

}

void decal_loaddata ( void )
{

//  establish some defaults
t.decal[t.decalid].across                  = 4;
t.decal[t.decalid].down                    = 4;
t.decal[t.decalid].variants                = 1;
t.decal[t.decalid].playspeed_f              = 1.0;
t.decal[t.decalid].particle.offsety        = 0;
t.decal[t.decalid].particle.scale          = 100;
t.decal[t.decalid].particle.randomstartx   = 0;
t.decal[t.decalid].particle.randomstarty   = 0;
t.decal[t.decalid].particle.randomstartz   = 0;
t.decal[t.decalid].particle.linearmotionx  = 0;
t.decal[t.decalid].particle.linearmotiony  = 0;
t.decal[t.decalid].particle.linearmotionz  = 0;
t.decal[t.decalid].particle.lineargravity  = 0;
t.decal[t.decalid].particle.randommotionx  = 0;
t.decal[t.decalid].particle.randommotiony  = 0;
t.decal[t.decalid].particle.randommotionz  = 0;
t.decal[t.decalid].particle.mirrormode     = 0;
t.decal[t.decalid].particle.camerazshift   = 500;
t.decal[t.decalid].particle.scaleonlyx     = 100;
t.decal[t.decalid].particle.lifeincrement  = 100;
t.decal[t.decalid].particle.alphaintensity = 100;
t.decal[t.decalid].particle.animated       = 0;

//  load decal description
Dim (  t.data_s,100  );
t.filename_s = "";
t.filename_s=t.filename_s+"gamecore\\decals\\"+t.decal_s+"\\decalspec.txt";
LoadArray (  t.filename_s.Get() , t.data_s );
for ( t.l = 0 ; t.l<=  99; t.l++ )
{
	t.line_s=t.data_s[t.l];
	if (  Len(t.line_s.Get())>0 ) 
	{
		if ( strcmp ( Left(t.line_s.Get(),1) , ";" ) != 0 ) 
		{

			//  take fieldname and value
			for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
			{
				if (  t.line_s.Get()[t.c] == '=' ) { t.mid = t.c+1  ; break; }
			}
			t.field_s=Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1)));
			t.value_s=removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid));

			//  take value 1 and 2 from value
			for ( t.c = 0 ; t.c <  Len(t.value_s.Get()); t.c++ )
			{
				if (  t.value_s.Get()[t.c] == ',' ) {  t.mid = t.c+1  ; break; }
			}
			t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
			t.value2=ValF(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));

			//  decal
			if (  t.field_s == "across"  )  t.decal[t.decalid].across = t.value1;
			if (  t.field_s == "down"  )  t.decal[t.decalid].down = t.value1;
			if (  t.field_s == "variants"  )  t.decal[t.decalid].variants = t.value1;
			if (  t.field_s == "playspeed"  )  t.decal[t.decalid].playspeed_f = (t.value1+0.0)/100.0;

			//  additional fields for decal based particle function
			if (  t.field_s == "offsety"  )  t.decal[t.decalid].particle.offsety = t.value1;
			if (  t.field_s == "scale"  )  t.decal[t.decalid].particle.scale = t.value1;
			if (  t.field_s == "randomstartx"  )  t.decal[t.decalid].particle.randomstartx = t.value1;
			if (  t.field_s == "randomstarty"  )  t.decal[t.decalid].particle.randomstarty = t.value1;
			if (  t.field_s == "randomstartz"  )  t.decal[t.decalid].particle.randomstartz = t.value1;
			if (  t.field_s == "linearmotionx"  )  t.decal[t.decalid].particle.linearmotionx = t.value1;
			if (  t.field_s == "linearmotiony"  )  t.decal[t.decalid].particle.linearmotiony = t.value1;
			if (  t.field_s == "linearmotionz"  )  t.decal[t.decalid].particle.linearmotionz = t.value1;
			if (  t.field_s == "lineargravity"  )  t.decal[t.decalid].particle.lineargravity = t.value1;
			if (  t.field_s == "randommotionx"  )  t.decal[t.decalid].particle.randommotionx = t.value1;
			if (  t.field_s == "randommotiony"  )  t.decal[t.decalid].particle.randommotiony = t.value1;
			if (  t.field_s == "randommotionz"  )  t.decal[t.decalid].particle.randommotionz = t.value1;
			if (  t.field_s == "mirrormode"  )  t.decal[t.decalid].particle.mirrormode = t.value1;
			if (  t.field_s == "camerazshift"  )  t.decal[t.decalid].particle.camerazshift = t.value1;
			if (  t.field_s == "scaleonlyx"  )  t.decal[t.decalid].particle.scaleonlyx = t.value1;
			if (  t.field_s == "lifeincrement"  )  t.decal[t.decalid].particle.lifeincrement = t.value1;
			if (  t.field_s == "alphaintensity"  )  t.decal[t.decalid].particle.alphaintensity = t.value1;
			if (  t.field_s == "animated"  )  t.decal[t.decalid].particle.animated = t.value1;

		}
	}
}
UnDim (  t.data_s );

//  default scale is 100
if (  t.decal[t.decalid].particle.scale == 0  )  t.decal[t.decalid].particle.scale = 100;

//  Some pre-game calculations
t.decal[t.decalid].framemax=t.decal[t.decalid].across*t.decal[t.decalid].down;

return;

}

void decal_load ( void )
{

//  Load decal data
decal_loaddata ( );

//  Load decal image and store name in bank
t.strwork = "" ; t.strwork = t.strwork+"gamecore\\decals\\"+t.decal_s+"\\decal.dds";
loaddecal( t.strwork.Get() ,t.decalid);

return;

}

void decal_scaninallref ( void )
{

//  Scan entire decals folder
SetDir (  "gamecore"  ); t.decalid=1;
UnDim (t.filelist_s);
buildfilelist("decals","");
SetDir (  ".." );
if (  ArrayCount(t.filelist_s)>0 ) 
{
	for ( t.chkfile = 0 ; t.chkfile<=  ArrayCount(t.filelist_s); t.chkfile++ )
	{
		t.file_s=t.filelist_s[t.chkfile];
		if (  t.file_s != "." && t.file_s != ".." ) 
		{
			t.tryfile_s=Lower(Right(t.file_s.Get(),13));
			if (  t.tryfile_s == "decalspec.txt" ) 
			{
				t.newdecal_s=Left(t.file_s.Get(),Len(t.file_s.Get())-14);
				for ( t.tdecalid = 1 ; t.tdecalid<=  g.decalmax; t.tdecalid++ )
				{
					if (  t.decal[t.tdecalid].name_s == t.newdecal_s  )  break;
				}
				if (  t.tdecalid>g.decalmax ) 
				{
					if (  t.decalid>g.decalmax ) 
					{
						g.decalmax=t.decalid;
						Dim (  t.decal,g.decalmax );
					}
					t.decal[t.decalid].name_s=t.newdecal_s;
					++t.decalid;
				}
			}
		}
	}
}
g.decalmax=t.decalid-1;

return;

}

void decal_loadonlyactivedecals ( void )
{

	//  Load all decals that have been activated
	for ( t.decalid = 1 ; t.decalid<=  g.decalmax; t.decalid++ )
	{
		if (  t.decal[t.decalid].active == 1 ) 
		{
			t.decal_s=t.decal[t.decalid].name_s;
			decal_load ( );
		}
	}

return;

}

void decal_getparticlefile ( void )
{

	//  if not scanned decal folder, do this now (before we can retrieve particle data)
	if ( t.decal[1].name_s == "" ) decal_scaninallref ( );

	//  takes particlefile$, fill gotparticle with data
	for ( t.decalid = 1 ; t.decalid<=  g.decalmax; t.decalid++ )
	{
		if (  cstr(Lower(t.decal[t.decalid].name_s.Get())) == Lower(t.particlefile_s.Get()) ) 
		{
			if (  t.decal[t.decalid].across == 0 ) 
			{
				t.decal[t.decalid].active=1;
				decal_loaddata ( );
			}
			g.gotparticle=t.decal[t.decalid].particle;
			break;
		}
	}
}

void decalelement_create ( void )
{
	//  Decal Orient Modes
	//  0 - perfectly upright facing camera (X=Z=0)
	//  1 - rotated to decalorient x y z#
	//  2 - flattened on Floor (90,0,0)
	//  3 - decal facing camera
	//  4 - decal facing camera (advanced 5.0 toward camera)
	//  5 - decal facing camera (advanced 15.0 toward camera)
	//  7 - decal based particle moves in 3D space (animated image)
	//  8 - decal based particle moves in 3D space (single image) (rotates Z~360)
	//  11 - single image blast then fade out over short time (muzzle from characters) (rotates Z~360)
	//  12 - same as [3] but fades alpha throughout frame life
	//  13 - same as [8] but replaces decal texture with blood splat from HUD

	//  early out if decal over 1000 units away from camera
	t.tdxx=g.decalx-CameraPositionX(t.terrain.gameplaycamera);
	t.tdyy=g.decaly-CameraPositionY(t.terrain.gameplaycamera);
	t.tdzz=g.decalz-CameraPositionZ(t.terrain.gameplaycamera);
	t.tddd=Sqrt(abs(t.tdxx*t.tdxx)+abs(t.tdyy*t.tdyy)+abs(t.tdzz*t.tdzz));

	//  limit decal usage to X distance
	if (  g.decalrange == 0  )  g.decalrange = 1000;
	if (  t.tddd>g.decalrange  )  return;

	//  find free decal element
	for ( t.d = 1 ; t.d<=  g.decalelementmax; t.d++ )
	{
		if (  t.decalelement[t.d].active == 0  )  break;
	}
	if (  t.d<g.decalelementmax ) 
	{
		//  activate new decal element
		t.currentdecald=t.d;
		t.decalelement[t.d].decalid=t.decalid;
		t.decalelement[t.d].active=1;
		t.decalelement[t.d].xpos=g.decalx;
		t.decalelement[t.d].ypos=g.decaly;
		t.decalelement[t.d].zpos=g.decalz;
		t.decalelement[t.d].burstloop=t.decalburstloop;
		t.decalelement[t.d].decalforward=t.decalforward;
		if (  t.decal[t.decalid].variants>1 ) 
		{
			t.tvariantsection=t.decal[t.decalid].framemax/t.decal[t.decalid].variants;
			t.tvarchoice=Rnd(t.decal[t.decalid].variants-1);
			t.decalelement[t.d].frame=t.tvarchoice*t.tvariantsection;
			t.decalelement[t.d].framefinish=((t.tvarchoice+1)*t.tvariantsection);
		}
		else
		{
			t.decalelement[t.d].frame=0;
			t.decalelement[t.d].framefinish=t.decal[t.decalid].framemax;
		}
		t.decalelement[t.d].orient=t.decalorient;
		t.decalelement[t.d].originator=t.originatore;
		t.decalelement[t.d].originatorobj=t.originatorobj;
		t.decalelement[t.d].particle=t.decal[t.decalid].particle;
		//  scale affects
		if (  t.decalscalemodx != 0 ) 
		{
			t.decalelement[t.d].particle.randomstartx=(t.decalelement[t.d].particle.randomstartx/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.randomstarty=(t.decalelement[t.d].particle.randomstarty/100.0)*t.decalscalemody;
			t.decalelement[t.d].particle.randomstartz=(t.decalelement[t.d].particle.randomstartz/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.linearmotionx=(t.decalelement[t.d].particle.linearmotionx/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.linearmotiony=(t.decalelement[t.d].particle.linearmotiony/100.0)*t.decalscalemody;
			t.decalelement[t.d].particle.linearmotionz=(t.decalelement[t.d].particle.linearmotionz/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.lineargravity=(t.decalelement[t.d].particle.lineargravity/100.0)*t.decalscalemody;
			t.decalelement[t.d].particle.randommotionx=(t.decalelement[t.d].particle.randommotionx/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.randommotiony=(t.decalelement[t.d].particle.randommotiony/100.0)*t.decalscalemody;
			t.decalelement[t.d].particle.randommotionz=(t.decalelement[t.d].particle.randommotionz/100.0)*t.decalscalemodx;
			t.decalelement[t.d].particle.camerazshift=(t.decalelement[t.d].particle.camerazshift/100.0)*t.decalscalemodx;
		}
		t.decalelement[t.d].fadestarttime=Timer();
		if (  t.decalelement[t.d].orient == 8 || t.decalelement[t.d].orient == 13 ) 
		{
			t.decalelement[t.d].fadespan=750*(100.0/t.decalelement[t.d].particle.lifeincrement);
		}
		else
		{
			if (  t.decalelement[t.d].orient == 11 ) 
			{
				t.decalelement[t.d].fadespan=50;
			}
			else
			{
				t.decalelement[t.d].fadespan=200;
			}
		}
		entity_assignentityparticletodecalelement ( );
		if (  t.decalelement[t.d].orient == 7 || t.decalelement[t.d].orient == 8 || t.decalelement[t.d].orient == 13 ) 
		{
			//  decal based particle adjust the Y pos, mirror and IX
			t.decalelement[t.d].xpos=t.decalelement[t.d].xpos-((Rnd(t.decalelement[t.d].particle.randomstartx)-(t.decalelement[t.d].particle.randomstartx/2))/100.0);
			t.decalelement[t.d].ypos=t.decalelement[t.d].ypos-((Rnd(t.decalelement[t.d].particle.randomstarty)-(t.decalelement[t.d].particle.randomstarty/2))/100.0);
			t.decalelement[t.d].zpos=t.decalelement[t.d].zpos-((Rnd(t.decalelement[t.d].particle.randomstartz)-(t.decalelement[t.d].particle.randomstartz/2))/100.0);
			t.decalelement[t.d].particleix=((t.decalelement[t.d].particle.randommotionx/2)-Rnd(t.decalelement[t.d].particle.randommotionx))/100.0;
			t.decalelement[t.d].particleiy=((t.decalelement[t.d].particle.randommotiony/2)-Rnd(t.decalelement[t.d].particle.randommotiony))/100.0;
			t.decalelement[t.d].particleiz=((t.decalelement[t.d].particle.randommotionz/2)-Rnd(t.decalelement[t.d].particle.randommotionz))/100.0;
			if (  t.decal[t.decalid].particle.mirrormode == 0  )  t.decalelement[t.d].particlemirror = 0;
			if (  t.decal[t.decalid].particle.mirrormode == 1  )  t.decalelement[t.d].particlemirror = 1;
			if (  t.decal[t.decalid].particle.mirrormode == 2  )  t.decalelement[t.d].particlemirror = Rnd(1);
		}
		//  prepare decal object
		t.tobj=t.decalelement[t.d].obj;
		if (  t.decalscalemodx == 0 ) 
		{
			t.decalelement[t.d].particle.offsety=t.decal[t.decalid].particle.offsety;
		}
		else
		{
			t.decalelement[t.d].particle.offsety=(t.decal[t.decalid].particle.offsety/100.0)*(t.decalscalemody+0.0);
		}
		PositionObject (  t.tobj,t.decalelement[t.d].xpos,t.decalelement[t.d].ypos+t.decalelement[t.d].particle.offsety,t.decalelement[t.d].zpos );
		//  can use root limb to rotate around Z (muzzle flash, etc)
		RotateLimb (  t.tobj,0,0,0,0 );
		//  face camera or leave as is
		if (  t.decalelement[t.d].orient == 0 ) 
		{
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			XRotateObject (  t.tobj,0  ); ZRotateObject (  t.tobj,0 );
		}
		if (  t.decalelement[t.d].orient == 1 ) 
		{
			RotateObject (  t.tobj,t.decalorientx_f,t.decalorienty_f,t.decalorientz_f );
		}
		if (  t.decalelement[t.d].orient == 2 ) 
		{
			RotateObject (  t.tobj,90,0,0 );
		}
		if (  t.decalelement[t.d].orient == 3 ) 
		{
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
		}
		if (  t.decalelement[t.d].orient == 4 ) 
		{
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			MoveObject (  t.tobj,5.0 );
		}
		if (  t.decalelement[t.d].orient == 5 ) 
		{
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			MoveObject (  t.tobj,15.0 );
		}
		if (  t.decalelement[t.d].orient == 7 || t.decalelement[t.d].orient == 8 || t.decalelement[t.d].orient == 13 ) 
		{
			//  spawn a decal based particle fragment (for new flame and fire) (once)
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			MoveObject (  t.tobj,t.decalelement[t.d].particle.camerazshift/100.0 );
			t.decalelement[t.d].xpos=ObjectPositionX(t.tobj);
			t.decalelement[t.d].ypos=ObjectPositionY(t.tobj);
			t.decalelement[t.d].zpos=ObjectPositionZ(t.tobj);
			if (  t.decalelement[t.d].orient == 8 ) 
			{
				//  single image rotate Z~360
				RotateLimb (  t.tobj,0,0,0,Rnd(360) );
			}
		}
		if (  t.decalelement[t.d].orient == 11 ) 
		{
			//  231213 - single frame muzzle image with fade
			RotateLimb (  t.tobj,0,0,0,Rnd(360) );
			PointObject (  t.tobj,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			MoveObject (  t.tobj,5.0+(t.decalforward/10.0) );
		}
		EnableObjectZBias (  t.tobj,0.0,0.0 );
		//  texture for this decal type
		if (  t.decalelement[t.d].orient == 13 ) 
		{
			TextureObject (  t.tobj,t.huddamage.bloodstart+Rnd(t.huddamage.maxbloodsplats) );
		}
		else
		{
			TextureObject (  t.tobj,t.decal[t.decalid].imageid );
		}
		//  scale the decal
		if (  t.decalscalemodx == 0 ) 
		{
			t.decalelement[t.d].scalemodx=t.decalelement[t.d].particle.scale*(t.decalelement[t.d].particle.scaleonlyx/100.0);
			t.decalelement[t.d].scalemody=t.decalelement[t.d].particle.scale;
		}
		else
		{
			t.decalelement[t.d].scalemodx=t.decalscalemodx*(t.decalelement[t.d].particle.scale/100.0)*(t.decalelement[t.d].particle.scaleonlyx/100.0);
			t.decalelement[t.d].scalemody=t.decalscalemody*(t.decalelement[t.d].particle.scale/100.0);
		}
		ScaleObject (  t.tobj,t.decalelement[t.d].scalemodx,t.decalelement[t.d].scalemody,100 );
		SetAlphaMappingOn (  t.tobj,0 );
		//  UV data prep
		if (  t.decal[t.decalid].across>1 ) 
		{
			t.qx_f=1.0/(t.decal[t.decalid].across+0.0);
			t.qy_f=1.0/(t.decal[t.decalid].across+0.0);
		}
		else
		{
			t.qx_f=1.0;
			t.qy_f=1.0;
		}
		if (  t.decalelement[t.d].orient == 7 && t.decalelement[t.d].particlemirror == 1 ) 
		{
			//  decal based particle can mirror the image
			t.qx_f=t.qx_f*-1;
		}
		SetObjectEffect ( t.tobj, g.decaleffectoffset );
		SetObjectCull ( t.tobj, 0 );
		DisableObjectZWrite ( t.tobj );
		SetObjectMask ( t.tobj, 1 );

		LockVertexDataForLimb (  t.tobj,0 );
		SetVertexDataUV (  0,t.qx_f,0 );
		SetVertexDataUV (  1,0,0 );
		SetVertexDataUV (  2,t.qx_f,t.qy_f );
		SetVertexDataUV (  3,0,0 );
		SetVertexDataUV (  4,0,t.qy_f );
		SetVertexDataUV (  5,t.qx_f,t.qy_f );
		UnlockVertexData (  );

		//  show decal object
		ShowObject (  t.tobj );
	}
	else
	{
		t.tobj=0;
	}
}

void decalelement_continue ( void )
{
	//  decal is looping, merely need to continue the current decal
	t.d=t.currentdecald;
	if (  t.d>0 && t.decalelement[t.d].originator == t.originatore ) 
	{
		//  simply reset decal
		t.decalelement[t.d].frame=0;
	}
	else
	{
		//  different, so recreate
		t.decalforward=t.decalelement[t.d].decalforward;
		decalelement_create ( );
	}

return;

}

void decalelement_control ( void )
{

	//  True camera position (returns tcamerapositionx#,tcamerapositiony#,tcamerapositionz#)
	entity_gettruecamera ( );

	//  Control all decal activity
	for ( t.f = 1 ; t.f<=  g.decalelementmax; t.f++ )
	{
		if (  t.decalelement[t.f].active == 1 ) 
		{
			//  decal speed machine independent
			t.decaltimeelapsed_f=g.timeelapsed_f;
			//  update decal object
			t.tobj=t.decalelement[t.f].obj ; t.tdetonate=0;
			//  if decal not in visible screen, destroy for performance - off no code
			if (  t.tdetonate == 0 ) 
			{
				//  possible movement
				if (  t.decalelement[t.f].orient == 7 || t.decalelement[t.f].orient == 8 || t.decalelement[t.f].orient == 13 ) 
				{
					//  decal based particle moves in 3D space
					t.decalelement[t.f].xpos=t.decalelement[t.f].xpos+(((t.decalelement[t.f].particleix/100.0)*100.0)*t.decaltimeelapsed_f);
					t.decalelement[t.f].ypos=t.decalelement[t.f].ypos+(((t.decalelement[t.f].particleiy/100.0)*100.0)*t.decaltimeelapsed_f);
					t.decalelement[t.f].zpos=t.decalelement[t.f].zpos+(((t.decalelement[t.f].particleiz/100.0)*100.0)*t.decaltimeelapsed_f);
					t.decalelement[t.f].xpos=t.decalelement[t.f].xpos+((t.decalelement[t.f].particle.linearmotionx/100.0)*t.decaltimeelapsed_f);
					t.decalelement[t.f].ypos=t.decalelement[t.f].ypos+((t.decalelement[t.f].particle.linearmotiony/100.0)*t.decaltimeelapsed_f);
					t.decalelement[t.f].zpos=t.decalelement[t.f].zpos+((t.decalelement[t.f].particle.linearmotionz/100.0)*t.decaltimeelapsed_f);
					if (  t.decalelement[t.f].particle.lineargravity != 0 ) 
					{
						t.decalelement[t.f].particle.linearmotiony=t.decalelement[t.f].particle.linearmotiony-((t.decalelement[t.f].particle.lineargravity/100.0)*t.decaltimeelapsed_f*10.0);
					}
				}
				if (  t.decalelement[t.f].orient == 13 ) 
				{
					//  in addition, for blood, move towards player for spray effect (see below)
				}
				//  animation
				t.decalid=t.decalelement[t.f].decalid;
				//  next frame
				t.decalelement[t.f].framedelay=t.decalelement[t.f].framedelay+(t.decaltimeelapsed_f*2*t.decal[t.decalid].playspeed_f);
				if (  t.decalelement[t.f].framedelay >= 1 ) 
				{
					t.tmultiple=t.decalelement[t.f].framedelay;
					t.decalelement[t.f].framedelay=0;
					t.decalelement[t.f].frame=t.decalelement[t.f].frame+((t.decalelement[t.f].particle.lifeincrement/100.0)*t.tmultiple);
				}
				if (  t.decalelement[t.f].orient == 8 || t.decalelement[t.f].orient == 11 || t.decalelement[t.f].orient == 13 ) 
				{
					//  Sin ( gle image fader Timer (  based life ) )
					t.decalelement[t.f].frame=0;
					if ( (int)((Timer()-t.decalelement[t.f].fadestarttime)) > t.decalelement[t.f].fadespan ) 
					{
						t.tdetonate=1;
					}
				}
				else
				{
					//  regular frame end detection
					if (  t.decalelement[t.f].frame>t.decalelement[t.f].framefinish-1 ) 
					{
						//  no more animation
						t.decalelement[t.f].frame=t.decalelement[t.f].framefinish-0.1;
						t.tdetonate=1;
					}
				}
				//  write UV for correct anim frame
				if (  GetVisible(t.tobj) == 1 && GetInScreen(t.tobj) == 1 && t.tdetonate == 0 || t.decalelement[t.f].decalid == t.decalglobal.splashdecalrippleid && t.tdetonate == 0 && ObjectExist(t.tobj) ) 
				{
					if (  t.decalelement[t.f].decalid == t.decalglobal.splashdecalrippleid && ObjectExist(t.tobj)  )  SetAlphaMappingOn (  t.tobj,8 );
					//  rotate to face camera if flagged
					if (  t.decalelement[t.f].orient == 0 ) 
					{
						PointObject (  t.tobj,t.tcamerapositionx_f,t.tcamerapositiony_f,t.tcamerapositionz_f );
						XRotateObject (  t.tobj,0  ); ZRotateObject (  t.tobj,0 );
					}
					if (  t.decalelement[t.f].orient == 2 ) 
					{
						SetObjectCull (  t.tobj,1 );
					}
					else
					{
						SetObjectCull (  t.tobj,0 );
					}
					if (  t.decalelement[t.f].orient == 3 || t.decalelement[t.f].orient == 11 || t.decalelement[t.f].orient == 12 ) 
					{
						PointObject (  t.tobj,t.tcamerapositionx_f,t.tcamerapositiony_f,t.tcamerapositionz_f );
					}
					if (  t.decalelement[t.f].orient == 7 || t.decalelement[t.f].orient == 8 || t.decalelement[t.f].orient == 13 ) 
					{
						//  decal based particle faces camera always and moves
						t.ty=t.decalelement[t.f].particle.offsety;
						PositionObject (  t.tobj,t.decalelement[t.f].xpos,t.decalelement[t.f].ypos+t.ty,t.decalelement[t.f].zpos );
						PointObject (  t.tobj,t.tcamerapositionx_f,t.tcamerapositiony_f,t.tcamerapositionz_f );
						if (  t.decalelement[t.f].orient == 13 ) 
						{
							//  spray towards player too
							MoveObject (  t.tobj,2.0*t.decaltimeelapsed_f );
							t.decalelement[t.f].xpos=ObjectPositionX(t.tobj);
							t.decalelement[t.f].ypos=ObjectPositionY(t.tobj);
							t.decalelement[t.f].zpos=ObjectPositionZ(t.tobj);
						}
					}
					if (  t.decalelement[t.f].orient == 0 || t.decalelement[t.f].orient == 1 || t.decalelement[t.f].orient == 3 || t.decalelement[t.f].orient == 4 || t.decalelement[t.f].orient == 5 ) 
					{
						SetAlphaMappingOn (  t.tobj,100.0 );
					}
					if (  t.decalelement[t.f].orient == 2 ) 
					{
						//  fade in and out over life of decal animation
						t.pt_f=t.decalelement[t.f].framefinish;
						t.p_f=(t.decalelement[t.f].framefinish-t.decalelement[t.f].frame)/t.pt_f;
						if (  t.p_f >= 0.5 ) 
						{
							SetAlphaMappingOn (  t.tobj,100.0-((t.p_f-0.5)*200.0) );
						}
						else
						{
							SetAlphaMappingOn (  t.tobj,t.p_f*200.0 );
						}
					}
					if (  t.decalelement[t.f].orient == 7 ) 
					{
						//  fade over life of decal animation
						t.pt_f=t.decalelement[t.f].framefinish;
						t.p_f=(t.decalelement[t.f].framefinish-t.decalelement[t.f].frame)/t.pt_f;
						SetAlphaMappingOn (  t.tobj,Sin(t.p_f*180)*t.decalelement[t.f].particle.alphaintensity );
					}
					if (  t.decalelement[t.f].orient == 8 || t.decalelement[t.f].orient == 11 || t.decalelement[t.f].orient == 13 ) 
					{
						//  single image fader
						t.tfadeperc_f=((Timer()-t.decalelement[t.f].fadestarttime)+0.0)/(t.decalelement[t.f].fadespan+0.0);
						if (  t.tfadeperc_f<0.0  )  t.tfadeperc_f = 0.0;
						if (  t.tfadeperc_f>1.0  )  t.tfadeperc_f = 1.0;
						t.tfinalalphavalue_f=t.decalelement[t.f].particle.alphaintensity/100.0;
						if (  t.decalelement[t.f].burstloop>0  )  t.tfinalalphavalue_f = t.tfinalalphavalue_f*0.7;
						SetAlphaMappingOn (  t.tobj,(100.0-(t.tfadeperc_f*100.0))*t.tfinalalphavalue_f );
					}
					if (  t.decalelement[t.f].orient == 12 ) 
					{
						//  multi-image fader (fades towards end)
						t.pt_f=t.decalelement[t.f].framefinish;
						t.p_f=(t.decalelement[t.f].framefinish-t.decalelement[t.f].frame)/t.pt_f;
						SetAlphaMappingOn (  t.tobj,Sin(t.p_f*180)*100 );
					}
					//  decal animation setting
					if (  t.decalelement[t.f].orient == 7 && t.decalelement[t.f].particle.animated != 1 && t.decalelement[t.f].decalid != t.decalglobal.splashdecalrippleid ) 
					{
						//  decal based particle only uses first animation frame
						t.tframe=0;
					}
					else
					{
						t.tframe=t.decalelement[t.f].frame;
					}
					//  animation UVs
					if (  t.decal[t.decalid].across>1 ) 
					{
						t.ty=t.tframe/t.decal[t.decalid].across ; t.tx=t.tframe-(t.ty*t.decal[t.decalid].across);
						t.qx_f=1.0/(t.decal[t.decalid].across+0.0) ; t.tx_f=t.tx*t.qx_f;
						t.qy_f=1.0/(t.decal[t.decalid].across+0.0) ; t.ty_f=t.ty*t.qy_f;
					}
					else
					{
						t.qx_f=1.0 ; t.tx_f=0;
						t.qy_f=1.0 ; t.ty_f=0;
					}
					if (  t.decalelement[t.f].orient == 7 && t.decalelement[t.f].particlemirror == 1 ) 
					{
						//  decal based particle can mirror the image
						t.tx_f=(t.tx*t.qx_f)+t.qx_f;
					}
					ScaleObjectTexture (  t.tobj,t.tx_f,t.ty_f );
				}
			}
			//  detonate trigger
			if (  t.tdetonate == 1 ) 
			{
				HideObject (  t.decalelement[t.f].obj );
				t.decalelement[t.f].originator=0;
				t.decalelement[t.f].active=0;
				if (  t.decalelement[t.f].burstloop>0 ) 
				{
					//  re-create decal if burstloop active
					t.decalelement[t.f].burstloop=t.decalelement[t.f].burstloop-1;
					if (  t.decalelement[t.f].burstloop>0 ) 
					{
						t.decalid=t.decalelement[t.f].decalid;
						t.toriginatorobj=t.decalelement[t.f].originatorobj;
						if (  t.toriginatorobj>0 ) 
						{
							//  check the object exists
							if (  ObjectExist(t.toriginatorobj)  ==  1 ) 
							{
								g.decalx=ObjectPositionX(t.toriginatorobj);
								g.decaly=ObjectPositionY(t.toriginatorobj);
								g.decalz=ObjectPositionZ(t.toriginatorobj);
							}
							else
							{
								g.decalx=t.decalelement[t.f].xpos;
								g.decaly=t.decalelement[t.f].ypos;
								g.decalz=t.decalelement[t.f].zpos;
							}
						}
						else
						{
							g.decalx=t.decalelement[t.f].xpos;
							g.decaly=t.decalelement[t.f].ypos;
							g.decalz=t.decalelement[t.f].zpos;
						}
						t.decalscalemodx=0;
						t.decalorient=t.decalelement[t.f].orient;
						t.originatore=-1;
						t.decalburstloop=t.decalelement[t.f].burstloop;
						t.decalforward=t.decalelement[t.f].decalforward;
						decalelement_create ( );
						t.decalburstloop=0;
					}
				}
			}
		}
	}
}

void decal_triggermaterialdebris ( void )
{

	//  takes tsoundmaterial,tsx#,tsy#,tsz#
	if (  t.tsoundmaterial>0 ) 
	{
		t.decalid=t.material[t.tsoundmaterial-1].decalid;
		if (  t.tsy_f<g.waterheight_f+10  )  t.decalid = 0;
		if (  t.decalid>0 ) 
		{
			//  create the decal for this event
			t.decalscalemodx=0;
			t.decalorient=4 ; g.decalx=t.tsx_f ; g.decaly=t.tsy_f ; g.decalz=t.tsz_f;
			t.decalforward=0;
			t.originatore=-1 ; decalelement_create ( );
		}
		t.tsoundmaterial=0;
	}
}

void decal_find ( void )
{
	//  Load all decals
	for ( t.decalid = 1 ; t.decalid<=  g.decalmax; t.decalid++ )
	{
		if (  t.decal[t.decalid].name_s == t.decal_s  )  break;
	}
	if (  t.decalid>g.decalmax  )  t.decalid = -1;
}

void decal_activatedecalsfromentities ( void )
{
	//  Only flag those present in level
	for ( t.tdecalid = 1 ; t.tdecalid<=  g.decalmax; t.tdecalid++ )
	{
		t.tokay=0;
		if (  t.tdecalid == t.decalglobal.splashdecalrippleid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.splashdecalsmallid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.splashdecallargeid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.splashdecalmistyid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.splashdecaldropletsid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.splashdecalfoamid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.dustflumeid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.impactid  )  t.tokay = 1;
		if (  t.tdecalid == t.decalglobal.bloodsplatid  )  t.tokay = 1;
		if (  t.tokay == 1 ) 
		{
			t.decal[t.tdecalid].active=1;
		}
		else
		{
			t.decal[t.tdecalid].active=0;
		}
	}
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.entid=t.entityelement[t.e].bankindex;
		if (  t.entid>0 ) 
		{
			//  per-entity decal list
			if (  t.entityprofile[t.entid].decalmax>0 ) 
			{
				for ( t.tq = 0 ; t.tq<=  t.entityprofile[t.entid].decalmax-1; t.tq++ )
				{
					t.tdecalid=t.entitydecal[t.entid][t.tq];
					t.decal[t.tdecalid].active=1;
				}
			}
			//  decals through associated weapon
			t.tfoundgunid=t.entityprofile[t.entid].hasweapon;
			if (  t.tfoundgunid>0 ) 
			{
				for ( t.talt = 0 ; t.talt<=  1; t.talt++ )
				{
					t.tdecalid=g.firemodes[t.tfoundgunid][t.talt].decalid;
					t.decal[t.tdecalid].active=1;
				}
			}
		}
	}

return;

}

void decal_triggerwatersplash ( void )
{

	//  location of water splash passed in decalx y z
	//  SK Uses tInScale# to modify size.
	t.originatore=-1;
	t.decalforward=0;
	t.decalscalemodx=(40+Rnd(20))*t.tInScale_f ; t.decalscalemody=(40+Rnd(20))*t.tInScale_f;
	t.decalid=t.decalglobal.splashdecalrippleid ; t.decalorient=2 ; decalelement_create ( );
	if (  Rnd(1) == 0 ) 
	{
		t.decalid=t.decalglobal.splashdecalsmallid ; t.decalorient=0 ; decalelement_create ( );
	}
	else
	{
		t.decalid=t.decalglobal.splashdecallargeid ; t.decalorient=0 ; decalelement_create ( );
	}
	t.decalid=t.decalglobal.splashdecalmistyid ; t.decalorient=12; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecaldropletsid ; t.decalorient=8 ; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecaldropletsid ; t.decalorient=8 ; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecaldropletsid ; t.decalorient=8 ; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecaldropletsid ; t.decalorient=8  ; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecaldropletsid ; t.decalorient=8 ; decalelement_create ( );
	t.decalid=t.decalglobal.splashdecalfoamid ; t.decalorient=2 ; decalelement_create ( );
}

void decal_triggerwaterripple ( void )
{
	//  location of water ripple passed in decalx y z
	t.originatore=-1 ; t.decalscalemodx=200 ; t.decalscalemody=200;
	t.decalforward=0;
	t.decalid=t.decalglobal.splashdecalrippleid ; t.decalorient=2 ; decalelement_create ( );
}

void decal_triggerimpact ( void )
{
	//  location of dust effect passed in decalx y z
	t.originatore=-1;
	t.decalscalemodx=20+Rnd(10) ; t.decalscalemody=20;
	t.decalforward=0;
	t.decalid=t.decalglobal.impactid ; t.decalorient=5  ; decalelement_create ( );
}

void decal_triggerbloodsplat ( void )
{
	//  location of effect passed in decalx y z
	t.originatore=-1;
	t.decalscalemodx=40+Rnd(20) ; t.decalscalemody=40+Rnd(20);
	t.decalforward=0;
	t.decalid=t.decalglobal.bloodsplatid ; t.decalorient=13 ; decalelement_create ( );
}

void decal_triggermaterialdecal ( void )
{
	//  location of material decal effect passed in decalx y z
	t.originatore=-1;
	t.decalscalemodx=20+Rnd(10) ; t.decalscalemody=20;
	t.decalforward=0;
	t.decalid=t.material[t.tttriggerdecalimpact-10].decalid;
	t.decalorient=5 ; decalelement_create ( );
}

int loaddecal ( char* tfile_s, int decalid )
{
	int timg = 0;
	timg=loadinternalimagecompress(tfile_s,5);
	t.decal[decalid].imageid=timg;
//endfunction timg
	return timg
;
}

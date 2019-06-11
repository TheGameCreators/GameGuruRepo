//----------------------------------------------------
//--- GAMEGURU - M-Postprocess
//----------------------------------------------------

#include "gameguru.h"

// 
//  POST PROCESSING (0-main cam,1-reserved,2-reflect cam,3-finalrender cam)
//  4-sunlight ray camera
// 

// Some convenient globals for VR controller shader and textures
int g_iCShaderID = 0;
int g_iCTextureID0 = 0;
int g_iCTextureID1 = 0;
int g_iCTextureID2 = 0;
int g_iCTextureID3 = 0;
int g_iCTextureID4 = 0;
int g_iCTextureID5 = 0;
int g_iCTextureID6 = 0;

void postprocess_init ( void )
{
	//  postprocesseffectoffset;
	//  0-bloom
	//  1-scatter
	//  2-nobloom

	//  full screen shaders
	g.gpostprocessing=1;

	//  if flagged, use post processing
	timestampactivity(0,"postprocessing code started");
	if (  g.gpostprocessing == 0 ) 
	{
		//  ensure camera zero set correctly
		SetCameraView (  0,0,0,GetDisplayWidth(),GetDisplayHeight() );
	}
	else
	{
		//  Initialise or Activate existing
		if ( t.gpostprocessmode == 0 ) 
		{
			// kind of post process (gpostprocessing)
			// 1 - bloom
			timestampactivity(0,"postprocessing check");
			if ( g.gpostprocessing == 1 ) 
			{
				//  new camera 3 holds post-process screen quad
				g.gfinalrendercameraid=3;
				CreateCamera ( g.gfinalrendercameraid );
				SetCameraView ( g.gfinalrendercameraid,0,0,GetDisplayWidth(),GetDisplayHeight() );
				if (  GetEffectExist(g.postprocesseffectoffset) == 1  )  DeleteEffect (  g.postprocesseffectoffset );
				t.tshaderchoice_s="effectbank\\reloaded\\post-bloom.fx";
				LoadEffect (  t.tshaderchoice_s.Get(),g.postprocesseffectoffset,0 );
				if (  GetEffectExist(g.postprocesseffectoffset+2) == 1  )  DeleteEffect (  g.postprocesseffectoffset+2 );
				t.tshaderchoice_s="effectbank\\reloaded\\post-none.fx";
				LoadEffect (  t.tshaderchoice_s.Get(),g.postprocesseffectoffset+2,0 );
				//  create and prepare resources for full screen final render
				MakeObjectPlane ( g.postprocessobjectoffset+0, 2, 2 );
				SetObjectEffect (  g.postprocessobjectoffset+0,g.postprocesseffectoffset );
				g.postprocessobjectoffset0laststate = g.postprocesseffectoffset;
				SetObjectMask (  g.postprocessobjectoffset+0,(1 << g.gfinalrendercameraid) );
				SetCameraToImage ( 0, g.postprocessimageoffset+0, GetDisplayWidth(), GetDisplayHeight(), 2 );
				TextureObject ( g.postprocessobjectoffset+0,0,g.postprocessimageoffset+0 );
				if( g.underwatermode == 1 ) {
					//PE:Underwater normal texture for wave.
					TextureObject(g.postprocessobjectoffset + 0, 1, t.terrain.imagestartindex + 7);
				}

				//  special code to instruct this post process shader to generate depth texture
				//  from the main camera zero and pass into the shader as 'DepthMapTex' texture slot
				SetVector4 (  g.terrainvectorindex,0,0,0,0 );
				SetEffectConstantV (  g.postprocesseffectoffset+0,"[hook-depth-data]",g.terrainvectorindex );

				//  post process mode on
				g.gpostprocessingnotransparency=0;
				t.gpostprocessmode=1;

				//  if Virtual Reality RIFT MODE active
				if (  g.globals.riftmode>0 ) 
				{
					//  Camera SIX is second eye camera (right eye) [left eye is camera zero]
					t.glefteyecameraid=6 ; CreateCamera (  t.glefteyecameraid );
					SetCameraToImage (  t.glefteyecameraid,g.postprocessimageoffset+3,GetDisplayWidth(),GetDisplayHeight(),2 );
					t.grighteyecameraid=7 ; CreateCamera (  t.grighteyecameraid );
					SetCameraToImage (  t.grighteyecameraid,g.postprocessimageoffset+4,GetDisplayWidth(),GetDisplayHeight(),2 );
					//  special post process shader for RIFT stereoscopics
					if (  GetEffectExist(g.postprocesseffectoffset+3) == 1  )  DeleteEffect (  g.postprocesseffectoffset+3 );
					t.tshaderchoice_s="effectbank\\reloaded\\post-rift.fx";
					LoadEffect (  t.tshaderchoice_s.Get(),g.postprocesseffectoffset+3,0 );
					//  create and prepare resources for full screen final render
					MakeObjectPlane (  g.postprocessobjectoffset+3,2,2 );
					SetObjectEffect (  g.postprocessobjectoffset+3,g.postprocesseffectoffset+3 );
					g.postprocessobjectoffset0laststate = g.postprocesseffectoffset+3;
					SetObjectMask (  g.postprocessobjectoffset+3,(1 << g.gfinalrendercameraid) );
					TextureObject (  g.postprocessobjectoffset+3,0,g.postprocessimageoffset+0 );
					//  how adjust the main post process quad and shader to create stereo screens
					SetObjectEffect (  g.postprocessobjectoffset+0,g.postprocesseffectoffset+3 );
					g.postprocessobjectoffset0laststate = g.postprocesseffectoffset+3;
					TextureObject (  g.postprocessobjectoffset+0,0,g.postprocessimageoffset+3 );
					TextureObject (  g.postprocessobjectoffset+3,0,g.postprocessimageoffset+4 );
					PositionObject (  g.postprocessobjectoffset+0,-0.5,0,0 );
					PositionObject (  g.postprocessobjectoffset+3,0.5,0,0 );

					//  Adjust left and right eye camera projections for Rift viewing
					if (  g.globals.riftmoderesult == 0 ) 
					{
						//  rift update projection matrix glefteyecameraid, grighteyecameraid, visuals.CameraNEAR#,visuals.CameraFAR#
					}
					else
					{
						// 311017 - solve Z clash issues by adjusting near depth based on far depth
						float fFinalNearDepth = 2.0f + t.visuals.CameraNEAR_f + ((t.visuals.CameraFAR_f/70000.0f)*8.0f);
						SetCameraRange ( t.glefteyecameraid, fFinalNearDepth, t.visuals.CameraFAR_f );
						SetCameraRange ( t.grighteyecameraid, fFinalNearDepth, t.visuals.CameraFAR_f );
					}
				}

				// VR Support - create VR cameras
				char pErrorStr[1024];
				sprintf ( pErrorStr, "check if VR required with codes %d and %d", g.vrglobals.GGVREnabled, g.vrglobals.GGVRUsingVRSystem );
				timestampactivity(0,pErrorStr);
				if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
				{
					// Set camera IDs and initialise GGVR
					t.glefteyecameraid = 6;
					t.grighteyecameraid = 7;
					g.vrglobals.GGVRInitialized = 0;
					sprintf ( pErrorStr, "initialise VR System Mode %d", g.vrglobals.GGVREnabled );
					timestampactivity(0,pErrorStr);
					if ( g_iCShaderID == 0 ) g_iCShaderID = g.controllerpbreffect;
					if ( g_iCTextureID0 == 0 ) g_iCTextureID0 = loadinternaltextureex("gamecore\\vrcontroller\\vrcontroller_color.png", 1, t.tfullorhalfdivide);
					if ( g_iCTextureID1 == 0 ) g_iCTextureID1 = loadinternaltextureex("effectbank\\reloaded\\media\\blank_O.dds", 1, t.tfullorhalfdivide);
					if ( g_iCTextureID2 == 0 ) g_iCTextureID2 = loadinternaltextureex("gamecore\\vrcontroller\\vrcontroller_normal.png", 1, t.tfullorhalfdivide);
					if ( g_iCTextureID3 == 0 ) g_iCTextureID3 = loadinternaltextureex("gamecore\\vrcontroller\\vrcontroller_metalness.png", 1, t.tfullorhalfdivide);
					if ( g_iCTextureID4 == 0 ) g_iCTextureID4 = loadinternaltextureex("gamecore\\vrcontroller\\vrcontroller_gloss.png", 1, t.tfullorhalfdivide);
					if ( g_iCTextureID5 == 0 ) g_iCTextureID5 = g.postprocessimageoffset+5;
					if ( g_iCTextureID6 == 0 ) g_iCTextureID6 = t.terrain.imagestartindex+31;//loadinternaltextureex("effectbank\\reloaded\\media\\blank_I.dds", 1, t.tfullorhalfdivide);
					sprintf ( pErrorStr, "controller asset %d %d %d %d %d %d %d %d", g_iCShaderID, g_iCTextureID0, g_iCTextureID1, g_iCTextureID2, g_iCTextureID3, g_iCTextureID4, g_iCTextureID5, g_iCTextureID6 );
					timestampactivity(0,pErrorStr);
					int iErrorCode = GGVR_Init ( g.rootdir_s.Get(), g.postprocessimageoffset + 4, g.postprocessimageoffset + 3, t.grighteyecameraid, t.glefteyecameraid, 10000, 10001, 10002, 10003, 10004, 10005, 10099, g.guishadereffectindex, g.editorimagesoffset+14, g_iCShaderID, g_iCTextureID0, g_iCTextureID1, g_iCTextureID2, g_iCTextureID3, g_iCTextureID4, g_iCTextureID5, g_iCTextureID6);
					if ( iErrorCode > 0 )
					{
						sprintf ( pErrorStr, "Error starting VR : Code %d", iErrorCode );
						timestampactivity(0,pErrorStr);
					}
				}

				// and new SAO shader which is slower but nicer looking with SAO effect in place
				if (  GetEffectExist(g.postprocesseffectoffset+4) == 1  )  DeleteEffect ( g.postprocesseffectoffset+4 );
				t.tshaderchoice_s="effectbank\\reloaded\\post-sao.fx";
				LoadEffect ( t.tshaderchoice_s.Get(), g.postprocesseffectoffset+4,0 );
				SetVector4 ( g.terrainvectorindex,0,0,0,0 );
				SetEffectConstantV ( g.postprocesseffectoffset+4,"[hook-depth-data]", g.terrainvectorindex );
			}

			//  Light ray shade setup
			t.glightraycameraid=4;
			if (  t.glightraycameraid>0 ) 
			{
				//  load lightray shader and required texture
				t.lightrayshader_s="effectbank\\reloaded\\Scatter.fx";
				LoadEffect (  t.lightrayshader_s.Get(),g.postprocesseffectoffset+1,0 );
				LoadImage (  "effectbank\\reloaded\\media\\Sun.dds",g.postprocessimageoffset+2,0,g.gdividetexturesize );
				//  lightray camera
				CreateCamera (  t.glightraycameraid );
				SetCameraFOV (  t.glightraycameraid,75 );
				BackdropOn (  t.glightraycameraid );
				BackdropColor (  t.glightraycameraid,Rgb(0,0,0) );
				SetCameraAspect (  t.glightraycameraid,1.325f );
				//  scatter shader quad
				MakeObjectPlane (  g.postprocessobjectoffset+1,2,2,1 );
				SetObjectEffect (  g.postprocessobjectoffset+1,g.postprocesseffectoffset+1 );
				SetSphereRadius ( g.postprocessobjectoffset+1, 0 );
				g.postprocessobjectoffset0laststate = g.postprocesseffectoffset+1;
				//SetCameraToImage (  t.glightraycameraid,g.postprocessimageoffset+1,GetDisplayWidth()/2,GetDisplayHeight()/2 );
				SetCameraToImage (  t.glightraycameraid,g.postprocessimageoffset+1,GetDisplayWidth()/4,GetDisplayHeight()/4 );
				TextureObject (  g.postprocessobjectoffset+1,0,g.postprocessimageoffset+1 );
				SetObjectMask (  g.postprocessobjectoffset+1,(pow(2,0)) );
				///GhostObjectOn (  g.postprocessobjectoffset+1,0 );
				SetObjectTransparency ( g.postprocessobjectoffset+1,1 );
				DisableObjectZWrite ( g.postprocessobjectoffset+1 );
				DisableObjectZDepthEx ( g.postprocessobjectoffset+1, 0 );
				//  sun plane object
				MakeObjectPlane (  g.postprocessobjectoffset+2,1500*2,1500*2 );
				TextureObject (  g.postprocessobjectoffset+2,g.postprocessimageoffset+2 );
				SetObjectTransparency (  g.postprocessobjectoffset+2,1 );
				SetObjectLight (  g.postprocessobjectoffset+2,0 );
				SetObjectMask (  g.postprocessobjectoffset+2,pow(2,t.glightraycameraid) );
				SetObjectEffect ( g.postprocessobjectoffset+2, g.guishadereffectindex );
			}

			//  Assign all parameter handles
			if (  GetEffectExist(g.postprocesseffectoffset+1) == 1 ) 
			{
				t.effectparam.postprocess.LightDir=GetEffectParameterIndex(g.postprocesseffectoffset+1,"LightDir");
				t.effectparam.postprocess.AlphaAngle=GetEffectParameterIndex(g.postprocesseffectoffset+1,"AlphaAngle");
				t.effectparam.postprocess.LightRayLength=GetEffectParameterIndex(g.postprocesseffectoffset+1,"LightRayLength");
				t.effectparam.postprocess.LightRayQuality=GetEffectParameterIndex(g.postprocesseffectoffset+1,"LightRayQuality");
				t.effectparam.postprocess.LightRayDecay=GetEffectParameterIndex(g.postprocesseffectoffset+1,"LightRayDecay");
			}
			if (  GetEffectExist(g.postprocesseffectoffset+0) == 1 ) 
			{
				t.effectparam.postprocess.ScreenColor0=GetEffectParameterIndex(g.postprocesseffectoffset+0,"ScreenColor");
			}
			if (  GetEffectExist(g.postprocesseffectoffset+2) == 1 ) 
			{
				t.effectparam.postprocess.ScreenColor2=GetEffectParameterIndex(g.postprocesseffectoffset+2,"ScreenColor");
			}
			if (  GetEffectExist(g.postprocesseffectoffset+4) == 1 ) 
			{
				t.effectparam.postprocess.ScreenColor4=GetEffectParameterIndex(g.postprocesseffectoffset+4,"ScreenColor");
			}
		}
		else
		{
			//  Activate existing post process effect
			postprocess_on ( );
		}

		//  reset post process shader defaults
		postprocess_reset_fade();

		//  generate terrain heightmap image to feed into this shader EVERY RUN
		/* completely remove old dynamic cheap shadow
		if (  t.gdynamicterrainshadowcameraid>0 ) 
		{
			t.terrain.terrainregionupdate=0;
			terrain_refreshterrainmatrix ( );
			terrain_createheightmapfromheightdata ( );
			postprocess_applycheapshadow ( );
		}
		*/
	}

	//  restore cam id when done
	SetCurrentCamera (  0 );
}

void postprocess_reset_fade ( void )
{
	//  reset post process shader defaults
	if (  t.gpostprocessmode>0 ) 
	{
		if (  GetEffectExist(g.postprocesseffectoffset+0) == 1 ) 
		{
			SetVector4 (  g.terrainvectorindex,0,0,0,0 );
			SetEffectConstantV (  g.postprocesseffectoffset+0,"ScreenColor",g.terrainvectorindex );
			SetEffectConstantV (  g.postprocesseffectoffset+2,"ScreenColor",g.terrainvectorindex );
			SetEffectConstantV (  g.postprocesseffectoffset+4,"ScreenColor",g.terrainvectorindex );
			t.postprocessings.fadeinvalue_f=0;
			SetVector4 (  g.terrainvectorindex,t.postprocessings.fadeinvalue_f,t.postprocessings.fadeinvalue_f,t.postprocessings.fadeinvalue_f,0 );
			SetEffectConstantV (  g.postprocesseffectoffset+0,"OverallColor",g.terrainvectorindex );
			SetEffectConstantV (  g.postprocesseffectoffset+2,"OverallColor",g.terrainvectorindex );
			SetEffectConstantV (  g.postprocesseffectoffset+4,"OverallColor",g.terrainvectorindex );
		}
	}
}

void postprocess_general_init ( void )
{
	//  called at start of prepare level sequence
	if (  GetEffectExist(g.decaleffectoffset) == 0 ) 
	{
		LoadEffect ( "effectbank\\reloaded\\decal_basic.fx", g.decaleffectoffset, 0 );
	}
}

void postprocess_forcheapshadows ( void )
{
	// Called ahead of IDE editor usage (cheap shadows)
	/* completely remove old dynamic cheap shadow
	if ( 1 ) 
	{
		//  Dynamic terrain shadow camera shader setup
		t.gdynamicterrainshadowcameraid=5;
		if (  t.gdynamicterrainshadowcameraid>0 ) 
		{
			//  ensure this camera DOES NOT change the visibility mask (optimization)
			AddVisibilityListMask (  t.gdynamicterrainshadowcameraid );
			//  load 'dynamic terrain shadow camera' shader and required texture
			t.dynterrshadowshader_s="effectbank\\reloaded\\dynamicterrainshadow_basic.fx";
			if (  GetEffectExist(g.postprocesseffectoffset+5) == 0 ) 
			{
				//  once only
				LoadEffect (  t.dynterrshadowshader_s.Get(),g.postprocesseffectoffset+5,0 );
				LoadImage (  "effectbank\\reloaded\\media\\heightmap.dds",g.postprocessimageoffset+6 );
				//  create camera
				CreateCamera (  t.gdynamicterrainshadowcameraid );
				SetCameraRange (  t.gdynamicterrainshadowcameraid,10,50000 );
				BackdropOn (  t.gdynamicterrainshadowcameraid );
				BackdropColor (  t.gdynamicterrainshadowcameraid,Rgb(0,0,0) );
				SetCameraAspect (  t.gdynamicterrainshadowcameraid,1.0 );
				SetCameraFOV (  t.gdynamicterrainshadowcameraid,90 );
				PositionCamera (  t.gdynamicterrainshadowcameraid,25600,25600,25600 );
				PointCamera (  t.gdynamicterrainshadowcameraid,25600,0,25600 );
				//  create and prepare resources for full 'image' quad render
				MakeObjectPlane (  g.postprocessobjectoffset+5,51200,51200 );
				XRotateObject (  g.postprocessobjectoffset+5,90 );
				PositionObject (  g.postprocessobjectoffset+5,25600,0,25600 );
				SetObjectEffect (  g.postprocessobjectoffset+5,g.postprocesseffectoffset+5 );
				SetObjectMask (  g.postprocessobjectoffset+5,(1 << t.gdynamicterrainshadowcameraid) );
				SetSphereRadius (  g.postprocessobjectoffset+5,0 );
				SetCameraToImage (  t.gdynamicterrainshadowcameraid, g.postprocessimageoffset+5, 4096, 4096 );
			}
			//  initially clear this 'writable' camera image
			t.gdynamicterrainshadowcameragenerate=5;
			t.gdynamicterrainshadowcameratimer=0;
		}
	}
	*/
}

void postprocess_applycheapshadow ( void )
{
	/* completely remove old dynamic cheap shadow
	//  Select technique based on superflat terrain mode
	if (  t.terrain.TerrainID == 0 ) 
	{
		SetEffectTechnique (  g.postprocesseffectoffset+5,"SuperFlat" );
	}
	else
	{
		SetEffectTechnique (  g.postprocesseffectoffset+5,"Terrain" );
	}

	//  provide latest heightmap image to dynamic terrain shadow shader plane
	if (  ObjectExist(g.postprocessobjectoffset+5) == 1 ) 
	{
		TextureObject (  g.postprocessobjectoffset+5,0,g.postprocessimageoffset+6 );
	}

	//  ensure terrain gets cheap shadow
	if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		TextureObject (  t.terrain.terrainobjectindex,1,g.postprocessimageoffset+5 );
	}

	// ensure ALL entities receive the subsequent 'dynterrsha' image
	for ( t.e = 1 ; t.e <= g.entityelementlist; t.e++ )
	{
		t.entid = t.entityelement[t.e].bankindex;
		if ( t.entid > 0 ) 
		{
			if ( t.entityprofile[t.entid].ismarker == 0 ) 
			{
				// Stage 4 (dynamicshadowcheapmap) only applies to effect profile zero (non-PBR)
				if ( t.entityprofile[t.entid].usingeffect > 0 && t.entityprofile[t.entid].effectprofile == 0 ) 
				{
					t.sourceobj = g.entitybankoffset+t.entid;
					if ( ObjectExist(t.sourceobj) == 1 ) 
					{
						TextureObject ( t.sourceobj,4,g.postprocessimageoffset+5 );
					}
					if ( t.entityelement[t.e].obj>0 ) 
					{
						if ( ObjectExist(t.entityelement[t.e].obj) == 1 ) 
						{
							TextureObject ( t.entityelement[t.e].obj,4,g.postprocessimageoffset+5 );
						}
					}
				}
			}
		}
	}

	//  ensure gun receives the dynamic terrain shadow texture
	for ( t.tgunid = 1 ; t.tgunid<=  g.gunmax; t.tgunid++ )
	{
		t.tcurrentgunobj=t.gun[t.tgunid].obj;
		if (  t.tcurrentgunobj>0 ) 
		{
			if (  ObjectExist(t.tcurrentgunobj) == 1 ) 
			{
				TextureObject (  t.tcurrentgunobj,5,g.postprocessimageoffset+5 );
			}
		}
	}
	*/
}

void postprocess_free ( void )
{
	// only free if enagaged
	if ( t.gpostprocessmode > 0 )
	{
		// free GGVR if used
		if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
		{
			GGVR_Shutdown();
		}

		//  free post processing objects
		if (  ObjectExist(g.postprocessobjectoffset+0) == 1  )  DeleteObject (  g.postprocessobjectoffset+0 );
		if (  ObjectExist(g.postprocessobjectoffset+1) == 1  )  DeleteObject (  g.postprocessobjectoffset+1 );
		if (  ObjectExist(g.postprocessobjectoffset+2) == 1  )  DeleteObject (  g.postprocessobjectoffset+2 );
		if (  ObjectExist(g.postprocessobjectoffset+3) == 1  )  DeleteObject (  g.postprocessobjectoffset+3 );
		if (  ImageExist(g.postprocessimageoffset+2) == 1  )  DeleteImage (  g.postprocessimageoffset+2 );
		if (  ImageExist(g.postprocessimageoffset+3) == 1  )  DeleteImage (  g.postprocessimageoffset+3 );

		//  free any resources created by post process technique
		if (  GetEffectExist(g.postprocesseffectoffset+0) == 1  )  DeleteEffect (  g.postprocesseffectoffset+0 );
		if (  GetEffectExist(g.postprocesseffectoffset+2) == 1  )  DeleteEffect (  g.postprocesseffectoffset+2 );
		if (  GetEffectExist(g.postprocesseffectoffset+3) == 1  )  DeleteEffect (  g.postprocesseffectoffset+3 );
		if (  GetEffectExist(g.postprocesseffectoffset+4) == 1  )  DeleteEffect (  g.postprocesseffectoffset+4 );
		if (  g.gfinalrendercameraid>0 ) 
		{
			if (  CameraExist(g.gfinalrendercameraid) == 1 ) 
			{
				DestroyCamera (  g.gfinalrendercameraid );
			}
			g.gfinalrendercameraid=0;
		}

		//  free lightray shader
		if (  GetEffectExist(g.postprocesseffectoffset+1) == 1  )  DeleteEffect (  g.postprocesseffectoffset+1 );
		if (  t.glightraycameraid>0 ) 
		{
			if (  CameraExist(t.glightraycameraid) == 1 ) 
			{
				DestroyCamera (  t.glightraycameraid );
			}
			t.glightraycameraid=0;
		}

		//  Total reset
		g.gpostprocessingnotransparency=0;
		t.gpostprocessmode=0;

		//  Restore main camera
		SetCurrentCamera (  0 );
		SetCameraView (  0,0,0,GetDesktopWidth(),GetDesktopHeight() );
		SetCameraToImage (  0,-1,0,0,0 );
	}
}

void postprocess_off ( void )
{
	if ( t.gpostprocessmode>0 ) 
	{
		if (  ObjectExist(g.postprocessobjectoffset+0) == 1  )  HideObject (  g.postprocessobjectoffset+0 );
		if (  ObjectExist(g.postprocessobjectoffset+1) == 1  )  HideObject (  g.postprocessobjectoffset+1 );
		if (  ObjectExist(g.postprocessobjectoffset+2) == 1  )  HideObject (  g.postprocessobjectoffset+2 );
		SetCameraToImage (  0,-1,0,0,0 );
		if (  g.gfinalrendercameraid>0 ) 
		{
			SetCameraView (  g.gfinalrendercameraid,0,0,1,1 );
		}
		if (  t.glightraycameraid>0 ) 
		{
			SetCameraView (  t.glightraycameraid,0,0,1,1 );
		}
	}
}

void postprocess_on ( void )
{
	if ( t.gpostprocessmode>0 ) 
	{
		if ( ObjectExist(g.postprocessobjectoffset+0) == 1  )  ShowObject (  g.postprocessobjectoffset+0 );
		if ( ObjectExist(g.postprocessobjectoffset+1) == 1  )  ShowObject (  g.postprocessobjectoffset+1 );
		if ( ObjectExist(g.postprocessobjectoffset+2) == 1  )  ShowObject (  g.postprocessobjectoffset+2 );
		SetCameraToImage (  0,g.postprocessimageoffset+0,GetDisplayWidth(),GetDisplayHeight(),2 );
		if ( g.gfinalrendercameraid>0 ) 
		{
			SetCameraView (  g.gfinalrendercameraid,0,0,GetDisplayWidth(),GetDisplayHeight() );
		}
		if ( t.glightraycameraid>0 ) 
		{
			SetCameraView (  t.glightraycameraid,0,0,GetDisplayWidth(),GetDisplayHeight() );
		}
	}
}

void postprocess_preterrain ( void )
{
	//  Render pre-terrain post process cameras for 'glightraycameraid' and 'rift mode' (below)
	if (  g.gpostprocessing>0 && t.glightraycameraid>0 && t.visuals.lightraymode>0 ) 
	{
		//  switch to black textures technique (fast)
		if (  t.terrain.vegetationshaderindex>0 ) 
		{
			if (  GetEffectExist(t.terrain.vegetationshaderindex) == 1  )  SetEffectTechnique (  t.terrain.vegetationshaderindex,"blacktextured" );
		}
		if (  GetEffectExist(t.terrain.terrainshaderindex) == 1  )  SetEffectTechnique (  t.terrain.terrainshaderindex,"blacktextured" );
		for ( t.t = -5 ; t.t<=  g.effectbankmax; t.t++ )
		{
			if (  t.t == -5  ) t.teffectid = g.controllerpbreffect;
			if ( t.t == -4  )  t.teffectid = g.lightmappbreffect;
			if ( t.t == -3  )  t.teffectid = g.thirdpersonentityeffect;
			if ( t.t == -2  )  t.teffectid = g.thirdpersoncharactereffect;
			if ( t.t == -1  )  t.teffectid = g.staticlightmapeffectoffset;
			if ( t.t == 0  )  t.teffectid = g.staticshadowlightmapeffectoffset;
			if ( t.t>0  )  t.teffectid = g.effectbankoffset+t.t;
			if ( GetEffectExist(t.teffectid) == 1 ) 
			{
				SetEffectTechnique ( t.teffectid, "blacktextured" );
			}
		}
		// remove sky from lightray
		if (  ObjectExist(t.terrain.objectstartindex+4) == 1 ) 
		{
			t.tskyobj1v=GetVisible(t.terrain.objectstartindex+4);
			HideObject (  t.terrain.objectstartindex+4 );
		}
		if (  ObjectExist(t.terrain.objectstartindex+8) == 1 ) 
		{
			t.tskyobj2v=GetVisible(t.terrain.objectstartindex+8);
			HideObject (  t.terrain.objectstartindex+8 );
		}
		if (  ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
		{
			t.tskyobj3v=GetVisible(t.terrain.objectstartindex+9);
			HideObject (  t.terrain.objectstartindex+9 );
		}

		// 051016 - moved here as still need to render lightray even if no terrain
		if ( t.hardwareinfoglobals.noterrain == 0 ) 
		{
			//  render light ray camera terrain
			if (  t.terrain.TerrainID>0 ) 
			{
				BT_SetCurrentCamera ( t.glightraycameraid );
				BT_UpdateTerrainLOD ( t.terrain.TerrainID );
				BT_UpdateTerrainCull ( t.terrain.TerrainID );
				BT_RenderTerrain ( t.terrain.TerrainID );
			}
		}

		//  Update whole camera view inc. masked in objects
		SyncMask ( 1 << t.glightraycameraid );
		FastSync ( );

		// show sky box again
		if ( t.hardwareinfoglobals.nosky == 0 ) 
		{
			if ( ObjectExist(t.terrain.objectstartindex+4) == 1 && t.tskyobj1v == 1 ) ShowObject ( t.terrain.objectstartindex+4 );
			if ( ObjectExist(t.terrain.objectstartindex+8) == 1 && t.tskyobj2v == 1 ) ShowObject ( t.terrain.objectstartindex+8 );
			if ( ObjectExist(t.terrain.objectstartindex+9) == 1 && t.tskyobj3v == 1 ) ShowObject ( t.terrain.objectstartindex+9 );
		}
		//  restore to original technique
		if ( t.terrain.vegetationshaderindex>0 ) 
		{
			if (  GetEffectExist(t.terrain.vegetationshaderindex) == 1 ) 
			{
				visuals_shaderlevels_vegetation_update ( );
			}
		}
		if ( GetEffectExist(t.terrain.terrainshaderindex) == 1 ) 
		{
			visuals_shaderlevels_terrain_update ( );
		}
		visuals_shaderlevels_entities_update ( );

		// restore camera Sync (  )
		SyncMask ( 0xfffffff9 );
		BT_SetCurrentCamera (  0 );
	}

	//  render left and right eye camera terrains
	if (  g.globals.riftmode>0 ) 
	{
		//  render terrains now
		if ( t.hardwareinfoglobals.noterrain == 0 ) 
		{
			if (  t.terrain.TerrainID>0 ) 
			{
				for ( t.leftright = 0 ; t.leftright<=  1; t.leftright++ )
				{
					//  left and right camera in turn
					if (  t.leftright == 0  )  t.tcamindex = t.glefteyecameraid;
					if (  t.leftright == 1  )  t.tcamindex = t.grighteyecameraid;

					//  adjust sky objects to center on this camera
					if (  ObjectExist(t.terrain.objectstartindex+4) == 1  )  PositionObject (  t.terrain.objectstartindex+4,CameraPositionX(t.tcamindex),CameraPositionY(t.tcamindex),CameraPositionZ(t.tcamindex) );
					if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  PositionObject (  t.terrain.objectstartindex+8,CameraPositionX(t.tcamindex),CameraPositionY(t.tcamindex),CameraPositionZ(t.tcamindex) );
					if (  ObjectExist(t.terrain.objectstartindex+9) == 1  )  PositionObject (  t.terrain.objectstartindex+9,CameraPositionX(t.tcamindex),CameraPositionY(t.tcamindex)+7000,CameraPositionZ(t.tcamindex) );

					//  update gun for this camera render
					if (  ObjectExist(g.hudbankoffset+2) == 1 ) 
					{
						//  need to adjust
						PositionObject (  g.hudbankoffset+2,t.oldoldcamx_f,(t.oldoldcamy_f+t.tsimwoddle_f)-10.0,t.oldoldcamz_f );
						t.gunax_f=t.oldoldcamax_f ; t.gunay_f=t.oldoldcamay_f;
						RotateObject (  g.hudbankoffset+2,t.gunax_f,t.gunay_f,0 );
						MoveObject (  g.hudbankoffset+2,-5.0 );
						//  aligh cross hair too
						PositionObject (  g.hudbankoffset+31,t.oldoldcamx_f,t.oldoldcamy_f,t.oldoldcamz_f );
						RotateObject (  g.hudbankoffset+31,t.oldoldcamax_f,t.oldoldcamay_f,0 );
						MoveObject (  g.hudbankoffset+31,400 );
					}

					//  render terrain for this camera
					BT_SetCurrentCamera (  t.tcamindex );
					BT_UpdateTerrainCull (  t.terrain.TerrainID );
					BT_UpdateTerrainLOD (  t.terrain.TerrainID );
					BT_RenderTerrain (  t.terrain.TerrainID );
					SyncMask (  (1<<t.tcamindex) );
					FastSync (  );
				}
				if (  ObjectExist(t.terrain.objectstartindex+4) == 1  )  PositionObject (  t.terrain.objectstartindex+4,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  PositionObject (  t.terrain.objectstartindex+8,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				if (  ObjectExist(t.terrain.objectstartindex+9) == 1  )  PositionObject (  t.terrain.objectstartindex+9,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera)+7000,CameraPositionZ(t.terrain.gameplaycamera) );
				SetCurrentCamera (  0 );
			}

			//  I should use LAST camera pos/angle as the stereo appears in a post process render
			t.oldoldcamx_f=CameraPositionX(0);
			t.oldoldcamy_f=CameraPositionY(0);
			t.oldoldcamz_f=CameraPositionZ(0);
			t.oldoldcamax_f=CameraAngleX(0);
			t.oldoldcamay_f=CameraAngleY(0);

			//  restore camera Sync (  )
			SyncMask (  0xfffffff9 );
			BT_SetCurrentCamera (  0 );
		}
	}

	// VR Support - render VR cameras
	if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		// position VR player at location of main camera
		GGVR_SetPlayerPosition(t.tFinalCamX_f, t.tFinalCamY_f, t.tFinalCamZ_f);
		GGVR_SetPlayerAngleY(t.playercontrol.finalcameraangley_f);

		// update seated/standing flag
		g.vrglobals.GGVRStandingMode = GGVR_GetTrackingSpace();

		// update HMD position and controller feedback
		bool bPlayerDucking = false;
		if ( t.aisystem.playerducking != 0 ) bPlayerDucking = true;
		GGVR_UpdatePlayer(bPlayerDucking,t.terrain.TerrainID,g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,g.entityviewstartobj,g.entityviewendobj);

		// handle teleport
		float fTelePortDestX = 0.0f;
		float fTelePortDestY = 0.0f;
		float fTelePortDestZ = 0.0f;
		float fTelePortDestAngleY = 0.0f;
		if ( GGVR_HandlePlayerTeleport ( &fTelePortDestX, &fTelePortDestY, &fTelePortDestZ, &fTelePortDestAngleY ) == true )
		{
			physics_disableplayer ( );
			t.terrain.playerx_f=fTelePortDestX;
			t.terrain.playery_f=fTelePortDestY+30;
			t.terrain.playerz_f=fTelePortDestZ;
			t.terrain.playerax_f=0;
			t.terrain.playeray_f=CameraAngleY(0);
			t.terrain.playeraz_f=0;
			physics_setupplayer ( );
		}

		//  render terrains now
		if (t.hardwareinfoglobals.noterrain == 0)
		{
			if (t.terrain.TerrainID > 0)
			{
				// for WMR style VR
				int iDebugMode = 0;
				if ( g.gproducelogfiles > 0 ) 
				{
					static bool bDoThisPreSubmitDebugLineOnce = false;
					if ( bDoThisPreSubmitDebugLineOnce == false )
					{
						timestampactivity(0,"Calling GGVR_PreSubmit VR");
						bDoThisPreSubmitDebugLineOnce = true;
					}
					iDebugMode = 1;
				}
				int iErrorCode = GGVR_PreSubmit(iDebugMode);
				if ( iErrorCode > 0 )
				{
					char pErrorStr[1024];
					sprintf ( pErrorStr, "Error running VR : Code %d", iErrorCode );
					timestampactivity(0,pErrorStr);
				}

				// render left and right eyes
				for (t.leftright = 0; t.leftright <= 1; t.leftright++)
				{
					//  left and right camera in turn
					if (t.leftright == 0)  t.tcamindex = t.glefteyecameraid;
					if (t.leftright == 1)  t.tcamindex = t.grighteyecameraid;

					//  adjust sky objects to center on this camera
					if (ObjectExist(t.terrain.objectstartindex + 4) == 1)  PositionObject(t.terrain.objectstartindex + 4, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex), CameraPositionZ(t.tcamindex));
					if (ObjectExist(t.terrain.objectstartindex + 8) == 1)  PositionObject(t.terrain.objectstartindex + 8, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex), CameraPositionZ(t.tcamindex));
					if (ObjectExist(t.terrain.objectstartindex + 9) == 1)  PositionObject(t.terrain.objectstartindex + 9, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex) + 7000, CameraPositionZ(t.tcamindex));

					//  render terrain for this camera
					BT_SetCurrentCamera(t.tcamindex);
					BT_UpdateTerrainCull(t.terrain.TerrainID);
					BT_UpdateTerrainLOD(t.terrain.TerrainID);
					BT_RenderTerrain(t.terrain.TerrainID);

					// The camera perspective matrix needs to be overridden directly inside the camera data structure
					tagCameraData* pCameraPtr = (tagCameraData*)GetCameraInternalData ( t.tcamindex );
					if ( pCameraPtr ) 
					{
						// get the projection matrix for each eye
						GGMATRIX Projection;
						if (t.leftright == 0)
							Projection = GGVR_GetLeftEyeProjectionMatrix();
						else
							Projection = GGVR_GetRightEyeProjectionMatrix();

						// transpose so compatible with the way our shadow model 5.0 shaders work
						GGMATRIX newWorkingProj = Projection;
						newWorkingProj.m[0][2] = -Projection.m[0][2];
						newWorkingProj.m[1][2] = -Projection.m[1][2];
						newWorkingProj.m[2][2] = -Projection.m[2][2];
						newWorkingProj.m[3][2] = -Projection.m[3][2];
						GGMatrixTranspose ( &newWorkingProj, &newWorkingProj );

						// finally override the camera projection matrix now
						pCameraPtr->matProjection = newWorkingProj;
					}

					// and now render
					SyncMask((1 << t.tcamindex));
					FastSync();
				}

				// for OpenVR style VR
				GGVR_Submit();

				// restore skies
				if (ObjectExist(t.terrain.objectstartindex + 4) == 1)  PositionObject(t.terrain.objectstartindex + 4, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera), CameraPositionZ(t.terrain.gameplaycamera));
				if (ObjectExist(t.terrain.objectstartindex + 8) == 1)  PositionObject(t.terrain.objectstartindex + 8, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera), CameraPositionZ(t.terrain.gameplaycamera));
				if (ObjectExist(t.terrain.objectstartindex + 9) == 1)  PositionObject(t.terrain.objectstartindex + 9, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera) + 7000, CameraPositionZ(t.terrain.gameplaycamera));

				// restore camera
				SetCurrentCamera(0);
			}

			//  I should use LAST camera pos/angle as the stereo appears in a post process render
			t.oldoldcamx_f = CameraPositionX(0);
			t.oldoldcamy_f = CameraPositionY(0);
			t.oldoldcamz_f = CameraPositionZ(0);
			t.oldoldcamax_f = CameraAngleX(0);
			t.oldoldcamay_f = CameraAngleY(0);

			//  restore camera Sync (  )
			SyncMask(0xfffffff9);
			BT_SetCurrentCamera(0);
		}
	}
}

void postprocess_apply ( void )
{
	if (  t.gpostprocessmode>0 ) 
	{
		// prepare the render for 'glightraycameraid'
		if ( t.glightraycameraid>0 ) 
		{
			if ( t.visuals.lightraymode == 0 ) 
			{
				// Hide light ray objects when mode not used
				HideObject ( g.postprocessobjectoffset+1 );
				HideObject ( g.postprocessobjectoffset+2 );
			}
			else
			{
				// Show light ray objects when mode used
				ShowObject ( g.postprocessobjectoffset+1 );
				ShowObject ( g.postprocessobjectoffset+2 );

				// light ray sun object's rotation
				g.rotvar_f=38.0;

				// update light ray sun object
				PositionObject (  g.postprocessobjectoffset+2,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
				RotateObject (  g.postprocessobjectoffset+2,-20,0+g.rotvar_f,0 );
				MoveObject (  g.postprocessobjectoffset+2,2000 );

				// update light ray scatter object
				PositionObject (  g.postprocessobjectoffset+1,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
				RotateObject (  g.postprocessobjectoffset+1,CameraAngleX(0),CameraAngleY(0),CameraAngleZ(0) );
				MoveObject (  g.postprocessobjectoffset+1,2 );

				// update light ray camera
				PositionCamera ( t.glightraycameraid,CameraPositionX(0),CameraPositionY(0),CameraPositionZ(0) );
				RotateCamera ( t.glightraycameraid,CameraAngleX(0),CameraAngleY(0),CameraAngleZ(0) );

				//  update scatter shader light direction
				t.ldirDX_f=CameraPositionX(0)-ObjectPositionX(g.postprocessobjectoffset+2);
				t.ldirDY_f=CameraPositionY(0)-ObjectPositionY(g.postprocessobjectoffset+2);
				t.ldirDZ_f=CameraPositionZ(0)-ObjectPositionZ(g.postprocessobjectoffset+2);
				t.ldirDD_f=Sqrt(abs(t.ldirDX_f*t.ldirDX_f)+abs(t.ldirDY_f*t.ldirDY_f)+abs(t.ldirDZ_f*t.ldirDZ_f));
				t.ldirDA_f=WrapValue(atan2deg(t.ldirDX_f,t.ldirDZ_f))-WrapValue(CameraAngleY(0));
				t.ldirDX_f=t.ldirDX_f/t.ldirDD_f;
				t.ldirDY_f=t.ldirDY_f/t.ldirDD_f;
				t.ldirDZ_f=t.ldirDZ_f/t.ldirDD_f;
				SetVector4 (  g.postprocesseffectoffset+1,t.ldirDX_f,t.ldirDY_f,t.ldirDZ_f,0 );
				SetEffectConstantVEx (  g.postprocesseffectoffset+1,t.effectparam.postprocess.LightDir,g.postprocesseffectoffset+1 );

				//  light ray AlphaAngle
				if (  t.ldirDA_f<-180  )  t.ldirDA_f = t.ldirDA_f+360.0;
				if (  t.ldirDA_f>180  )  t.ldirDA_f = t.ldirDA_f-360.0;
				t.ldirDAlpha_f=1.0-((180.0-abs(t.ldirDA_f))/45.0);
				if (  t.ldirDAlpha_f<0  )  t.ldirDAlpha_f = 0;
				if (  t.ldirDAlpha_f>1  )  t.ldirDAlpha_f = 1;
				t.ldirDAlpha_f=t.ldirDAlpha_f*t.sky.alpha1_f;
				SetEffectConstantFEx (  g.postprocesseffectoffset+1,t.effectparam.postprocess.AlphaAngle,t.ldirDAlpha_f );
				
				// pass in settings
				SetEffectConstantFEx ( g.postprocesseffectoffset+1, t.effectparam.postprocess.LightRayLength, t.visuals.LightrayLength_f );
				SetEffectConstantFEx ( g.postprocesseffectoffset+1, t.effectparam.postprocess.LightRayDecay, t.visuals.LightrayDecay_f );
			}
		}
	}
}

void postprocess_setscreencolor ( void )
{
	//  takes tColorVector
	SetEffectConstantVEx (  g.postprocesseffectoffset+0,t.effectparam.postprocess.ScreenColor0,t.tColorVector );
	SetEffectConstantVEx (  g.postprocesseffectoffset+2,t.effectparam.postprocess.ScreenColor2,t.tColorVector );
	SetEffectConstantVEx (  g.postprocesseffectoffset+4,t.effectparam.postprocess.ScreenColor4,t.tColorVector );
}

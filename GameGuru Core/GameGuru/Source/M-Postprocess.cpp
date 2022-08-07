//----------------------------------------------------
//--- GAMEGURU - M-Postprocess
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

#ifndef PRODUCTCLASSIC
#include "openxr.h"
#endif

// 
//  POST PROCESSING (0-main cam,1-reserved,2-reflect cam,3-finalrender cam)
//  4-sunlight ray camera
// 

#ifdef VRTECH
// Some convenient globals for VR controller shader and textures
int g_iCShaderID = 0;
int g_iCTextureID0 = 0;
int g_iCTextureID1 = 0;
int g_iCTextureID2 = 0;
int g_iCTextureID3 = 0;
int g_iCTextureID4 = 0;
int g_iCTextureID5 = 0;
int g_iCTextureID6 = 0;
#endif

void postprocess_init ( void )
{
	// full screen shaders
	g.gpostprocessing = 1;

	//  postprocesseffectoffset;
	//  0-bloom
	//  1-scatter
	//  2-nobloom

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

				//LUT post processing
				if (ImageExist(t.terrain.imagestartindex + 33) == 1)
				{
					TextureObject(g.postprocessobjectoffset + 0, 2, t.terrain.imagestartindex + 33);
				}
				
				//HBAO post processing
				if (ImageExist(t.terrain.imagestartindex + 38) == 1)
				{
					TextureObject(g.postprocessobjectoffset + 0, 3, t.terrain.imagestartindex + 38);
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
						float fFinalNearDepth = 2.0f + t.visuals.CameraNEAR_f + ((t.visuals.CameraFAR_f/300000.0f)*8.0f);
						SetCameraRange ( t.glefteyecameraid, fFinalNearDepth, t.visuals.CameraFAR_f );
						SetCameraRange ( t.grighteyecameraid, fFinalNearDepth, t.visuals.CameraFAR_f );
					}
				}

				#ifdef VRTECH
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
					int oculusTex0 = loadinternaltextureex("gamecore\\vrcontroller\\oculus\\controller_bc.png", 1, t.tfullorhalfdivide);
					sprintf ( pErrorStr, "controller asset %d %d %d %d %d %d %d %d", g_iCShaderID, g_iCTextureID0, g_iCTextureID1, g_iCTextureID2, g_iCTextureID3, g_iCTextureID4, g_iCTextureID5, g_iCTextureID6 );
					timestampactivity(0,pErrorStr);
					int iErrorCode = GGVR_Init ( g.rootdir_s.Get(), g.postprocessimageoffset + 4, g.postprocessimageoffset + 3, t.grighteyecameraid, t.glefteyecameraid, 10000, 10001, 10002, 10003, 10004, 10005, 10099, g.guishadereffectindex, g.editorimagesoffset+14, g_iCShaderID, g_iCTextureID0, g_iCTextureID1, g_iCTextureID2, g_iCTextureID3, g_iCTextureID4, g_iCTextureID5, g_iCTextureID6, oculusTex0);
					if ( iErrorCode > 0 )
					{
						sprintf ( pErrorStr, "Error starting VR : Code %d", iErrorCode );
						timestampactivity(0,pErrorStr);

						t.visuals.generalpromptstatetimer = Timer()+1000;
						t.visuals.generalprompt_s = "No OpenXR runtime found";
					}
					GGVR_SetGenericOffsetAngX( g.gvroffsetangx );
					GGVR_SetWMROffsetAngX( g.gvrwmroffsetangx );
				}
				#endif

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
	}
	// restore cam id when done
	SetCurrentCamera (  0 );
}

void postprocess_reset_fade ( void )
{
	// reset post process shader defaults
	if ( t.gpostprocessmode>0 ) 
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
	// called at start of prepare level sequence
	if (  GetEffectExist(g.decaleffectoffset) == 0 ) 
	{
		LoadEffect ( "effectbank\\reloaded\\decal_basic.fx", g.decaleffectoffset, 0 );
	}
}

void postprocess_free ( void )
{
	// only free if enagaged
	if ( t.gpostprocessmode > 0 )
	{
		#ifdef VRTECH
		// free GGVR if used
		if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
		{
			GGVR_Shutdown();
		}
		#endif

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

		//  Restore main camera
		SetCurrentCamera (  0 );
		SetCameraView (  0,0,0,GetDesktopWidth(),GetDesktopHeight() );
		SetCameraToImage (  0,-1,0,0,0 );

		// and reset flag
		t.gpostprocessmode=0;
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
		for ( t.t = -6 ; t.t<=  g.effectbankmax; t.t++ )
		{
			if ( t.t == -6  )  t.teffectid = g.lightmappbreffectillum;
			#ifdef VRTECH
			if ( t.t == -5  ) t.teffectid = g.controllerpbreffect;
			#else
			if ( t.t == -5  ) continue;
			#endif
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

	#ifndef PRODUCTCLASSIC
	#ifdef VRTECH
	// VR Support - render VR cameras
	if ( g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		if ( !GGVR_IsRuntimeFound() ) GGVR_ReInit();
		
		if ( !GGVR_IsRuntimeFound() )
		{
			t.visuals.generalpromptstatetimer = Timer()+1000;
			t.visuals.generalprompt_s = "32-bit OpenXR runtime not found";
		}
		else
		{
			// position VR player at location of main camera
			GGVR_SetPlayerPosition(t.tFinalCamX_f, t.tFinalCamY_f, t.tFinalCamZ_f);

			// this sets the origin based on the current camera zero (ARG!)
			// should only set based on player angle (minus HMD influence) as HMD added later at right time for smooth headset viewing!
			GGVR_SetPlayerAngleY(t.camangy_f);

			// update seated/standing flag
			g.vrglobals.GGVRStandingMode = GGVR_GetTrackingSpace();

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

			// will return -1 to skip rendering this frame, not an error
			int iErrorCode = GGVR_PreSubmit(iDebugMode);
			if ( iErrorCode > 0 )
			{
				char pErrorStr[1024];
				sprintf ( pErrorStr, "Error running VR : Code %d", iErrorCode );
				timestampactivity(0,pErrorStr);
			}
			else
			{
				// determine if headset missing
				if ( iErrorCode == -3 )
				{
					t.visuals.generalpromptstatetimer = Timer()+1000;
					t.visuals.generalprompt_s = "VR headset not found";
				}
				else if ( iErrorCode == -2 )
				{
					t.visuals.generalpromptstatetimer = Timer()+1000;
					t.visuals.generalprompt_s = "VR headset is currently inactive";
				}
			}
			
			// OpenXR may want us to skip rendering this frame, so only render if error code is 0
			if ( iErrorCode == 0 )
			{
				// handle teleport
				float fTelePortDestX = 0.0f;
				float fTelePortDestY = 0.0f;
				float fTelePortDestZ = 0.0f;
				float fTelePortDestAngleY = 0.0f;
				bool VRteleport = GGVR_HandlePlayerTeleport ( &fTelePortDestX, &fTelePortDestY, &fTelePortDestZ, &fTelePortDestAngleY );
				if ( VRteleport )
				{
					physics_disableplayer ( );
					t.terrain.playerx_f=fTelePortDestX;
					t.terrain.playery_f=fTelePortDestY+30;
					t.terrain.playerz_f=fTelePortDestZ;
					physics_setupplayer ( );
				}

				float leftStickY = OpenXRGetLeftStickY();
				float rightStickY = OpenXRGetRightStickY();
				if ( abs(rightStickY) > abs(leftStickY) ) leftStickY = rightStickY;
				int blinkTurn = 1;
				if ( abs(leftStickY) > 0.4 ) blinkTurn = 0;

				if ( blinkTurn )
				{
					int VRturn = GGVR_GetTurnDirection();
					if ( VRturn != 0 )
					{
						t.terrain.playerax_f=0;
						t.camangy_f += (VRturn * 30.0f);
						t.terrain.playeray_f = t.camangy_f;
						//t.terrain.playeray_f = t.camangy_f;// CameraAngleY(0);
						t.playercontrol.finalcameraangley_f=t.terrain.playeray_f;
						t.terrain.playeraz_f=0;
					}
				}
				else
				{			
					// continuous turning
					float leftStickX = OpenXRGetLeftStickX();
					float rightStickX = OpenXRGetRightStickX();
					if ( abs(rightStickX) > abs(leftStickX) ) leftStickX = rightStickX;
					int sign = 1;
					if ( leftStickX < 0 ) 
					{
						sign = -1;
						leftStickX = -leftStickX;
					}
					leftStickX = (leftStickX - 0.1f) * 1.11111f;
					leftStickX = leftStickX * leftStickX;
					leftStickX *= sign;
					t.camangy_f += leftStickX;
					t.terrain.playeray_f = t.camangy_f;
					t.playercontrol.finalcameraangley_f = t.terrain.playeray_f;
				}

				/*
				// continuous movement with thumbsticks, doesn't seem to work very well
				float leftMoveX = OpenXRGetLeftStickX();
				float leftMoveY = OpenXRGetLeftStickY();
				float rightMoveX = OpenXRGetRightStickX();
				float rightMoveY = OpenXRGetRightStickY();
				if ( abs(rightMoveX) > abs(leftMoveX) ) leftMoveX = rightMoveX;
				if ( abs(rightMoveY) > abs(leftMoveY) ) leftMoveY = rightMoveY;

				float angY = GGVR_GetHMDAngleY();
				float sinAngY = sin(angY * PI / 180.0f );
				float cosAngY = cos(angY * PI / 180.0f );

				physics_disableplayer ( );
				t.terrain.playerz_f += cosAngY*leftMoveY - sinAngY*leftMoveY;
				t.terrain.playerx_f += cosAngY*leftMoveY + sinAngY*leftMoveY;
				physics_setupplayer ( );
				*/

				// update HMD position and controller feedback
				bool bPlayerDucking = false;
				if ( t.aisystem.playerducking != 0 ) bPlayerDucking = true;
				int iBatchStart = g.batchobjectoffset;
				int iBatchEnd = g.batchobjectoffset + g.merged_new_objects + 1;
				GGVR_UpdatePlayer(bPlayerDucking,t.terrain.TerrainID,g.lightmappedobjectoffset,g.lightmappedobjectoffsetfinish,g.entityviewstartobj,g.entityviewendobj,iBatchStart,iBatchEnd);
						
				// render left and right eyes
				for (t.leftright = 0; t.leftright <= 1; t.leftright++)
				{
					//  left and right camera in turn
					if (t.leftright == 0)  
					{
						t.tcamindex = t.glefteyecameraid;
						GGVR_StartRender( 0 );
					}
					if (t.leftright == 1)  
					{
						t.tcamindex = t.grighteyecameraid;
						GGVR_StartRender( 1 );
					}

					//  adjust sky objects to center on this camera
					if (ObjectExist(t.terrain.objectstartindex + 4) == 1)  PositionObject(t.terrain.objectstartindex + 4, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex), CameraPositionZ(t.tcamindex));
					if (ObjectExist(t.terrain.objectstartindex + 8) == 1)  PositionObject(t.terrain.objectstartindex + 8, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex), CameraPositionZ(t.tcamindex));
					if (ObjectExist(t.terrain.objectstartindex + 9) == 1)  PositionObject(t.terrain.objectstartindex + 9, CameraPositionX(t.tcamindex), CameraPositionY(t.tcamindex) + 7000, CameraPositionZ(t.tcamindex));

					// render terrain for this camera
					BT_SetCurrentCamera(t.tcamindex);
					BT_UpdateTerrainCull(t.terrain.TerrainID);
					BT_UpdateTerrainLOD(t.terrain.TerrainID);
					if ( t.hardwareinfoglobals.noterrain == 0 && t.terrain.TerrainID > 0 )
						BT_RenderTerrain(t.terrain.TerrainID);
					else
						BT_NoRenderTerrain(t.terrain.TerrainID);

					// and now render
					SyncMask((1 << t.tcamindex));
					FastSync();

					GGVR_EndRender();
				}

				// restore skies
				if (ObjectExist(t.terrain.objectstartindex + 4) == 1)  PositionObject(t.terrain.objectstartindex + 4, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera), CameraPositionZ(t.terrain.gameplaycamera));
				if (ObjectExist(t.terrain.objectstartindex + 8) == 1)  PositionObject(t.terrain.objectstartindex + 8, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera), CameraPositionZ(t.terrain.gameplaycamera));
				if (ObjectExist(t.terrain.objectstartindex + 9) == 1)  PositionObject(t.terrain.objectstartindex + 9, CameraPositionX(t.terrain.gameplaycamera), CameraPositionY(t.terrain.gameplaycamera) + 7000, CameraPositionZ(t.terrain.gameplaycamera));

				// restore camera
				SetCurrentCamera(0);
			}

			// finialize VR frame, must be called even if we don't render
			if ( iErrorCode == -1 || iErrorCode == 0 ) GGVR_Submit();

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
	#endif
	#endif
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
				#ifdef VRTECH
				g.rotvar_f=38.0 + 180.0f;
				#else
				g.rotvar_f=38.0;
				#endif

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
				//SetEffectConstantVEx (  g.postprocesseffectoffset+1,t.effectparam.postprocess.LightDir,g.postprocesseffectoffset+1 );
				SetEffectConstantV(g.postprocesseffectoffset + 1, "LightDir", g.postprocesseffectoffset + 1);//cyb

				//  light ray AlphaAngle
				if (  t.ldirDA_f<-180  )  t.ldirDA_f = t.ldirDA_f+360.0;
				if (  t.ldirDA_f>180  )  t.ldirDA_f = t.ldirDA_f-360.0;
				t.ldirDAlpha_f=1.0-((180.0-abs(t.ldirDA_f))/45.0);
				if (  t.ldirDAlpha_f<0  )  t.ldirDAlpha_f = 0;
				if (  t.ldirDAlpha_f>1  )  t.ldirDAlpha_f = 1;
				t.ldirDAlpha_f=t.ldirDAlpha_f*t.sky.alpha1_f;
				//SetEffectConstantFEx (  g.postprocesseffectoffset+1,t.effectparam.postprocess.AlphaAngle,t.ldirDAlpha_f ); //cyb
				SetEffectConstantF(g.postprocesseffectoffset + 1, "AlphaAngle", t.ldirDAlpha_f);
								
				// pass in settings
				//SetEffectConstantFEx ( g.postprocesseffectoffset+1, t.effectparam.postprocess.LightRayLength, t.visuals.LightrayLength_f ); //cyb
				//SetEffectConstantFEx ( g.postprocesseffectoffset+1, t.effectparam.postprocess.LightRayDecay, t.visuals.LightrayDecay_f ); //cyb
				SetEffectConstantF(g.postprocesseffectoffset + 1, "LightRayLength", t.visuals.LightrayLength_f);
				SetEffectConstantF(g.postprocesseffectoffset + 1, "LightRayDecay", t.visuals.LightrayDecay_f);
			}
		}
	}
}

void postprocess_setscreencolor ( void )
{
	//SetEffectConstantVEx (  g.postprocesseffectoffset+0,t.effectparam.postprocess.ScreenColor0,t.tColorVector );
	//SetEffectConstantVEx (  g.postprocesseffectoffset+2,t.effectparam.postprocess.ScreenColor2,t.tColorVector );
	//SetEffectConstantVEx (  g.postprocesseffectoffset+4,t.effectparam.postprocess.ScreenColor4,t.tColorVector );

	//cyb
	SetEffectConstantV(g.postprocesseffectoffset + 0, "ScreenColor", t.tColorVector);
	SetEffectConstantV(g.postprocesseffectoffset + 2, "ScreenColor", t.tColorVector);
	SetEffectConstantV(g.postprocesseffectoffset + 4, "ScreenColor", t.tColorVector);
}

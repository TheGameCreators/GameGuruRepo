//----------------------------------------------------
//--- GAMEGURU - M-Lightmapping
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  Lightmapping Module
// 

int GetIsValidShader ( LPSTR pEffectFilename )
{
	int tvalidshadereffect = 0;
	if ( strcmp ( Lower(pEffectFilename), "effectbank\\reloaded\\entity_basic.fx" ) == 0 
	||   strcmp ( Lower(pEffectFilename), "effectbank\\reloaded\\apbr_basic.fx" ) == 0 )  
	{
		tvalidshadereffect = 1;
	}
	return tvalidshadereffect;
}

void lm_init ( void )
{
	//  set lightmap path
	t.lightmapper.lmpath_s = g.mysystem.levelBankTestMapAbs_s+"lightmaps\\";
	t.lightmapper.lmobjectfile_s=t.lightmapper.lmpath_s+"objectlist.dat";

	//  Load in lightmap shader effect for static geometry
	if (  GetEffectExist(g.staticlightmapeffectoffset) == 0 ) 
	{
		LoadEffect (  "effectbank\\reloaded\\static_basic.fx",g.staticlightmapeffectoffset,0 );
		filleffectparamarray(g.staticlightmapeffectoffset);
	}
	if (  GetEffectExist(g.staticshadowlightmapeffectoffset) == 0 ) 
	{
		LoadEffect (  "effectbank\\reloaded\\shadow_basic.fx",g.staticshadowlightmapeffectoffset,0 );
		filleffectparamarray(g.staticshadowlightmapeffectoffset);
	}
}

void lm_removeold ( void )
{
	// Until system can detect ALL changes (lights, subtle changes), just delete EVERYTHING
	g.glmsceneentitymax=g.entityelementlist;
	Dim (  t.lmsceneobj,g.glmsceneentitymax );
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (t.e < t.entityelement.size()) 
		{
			t.tobjstart=t.lmsceneobj[t.e].startobj;
			if (t.tobjstart > 0)
			{
				for (t.tobj = t.tobjstart; t.tobj <= t.lmsceneobj[t.e].finishobj; t.tobj++)
				{
					if (ObjectExist(t.tobj) == 1)  DeleteObject(t.tobj);
				}
				t.lmsceneobj[t.e].startobj = 0;
				t.lmsceneobj[t.e].finishobj = 0;
				t.lmsceneobj[t.e].lmvalid = 0;
			}
		}
	}

	//  And to make double sure, wipe out ALL lightmap region objects
	for ( t.tobj = g.lightmappedobjectoffset; t.tobj<= g.lightmappedobjectoffsetlast; t.tobj++ )
	{
		if (  ObjectExist(t.tobj) == 1  )  DeleteObject (  t.tobj );
	}
}

void lm_deletelmobjectsfirst ( void )
{
	//  Always erase all terrain objects (detect changes eventually)
	//  as they are recreated (max 256 based on 16x16 slices)
	t.tlmobj2=g.lightmappedobjectoffset;
	while (  t.tlmobj2<g.lightmappedobjectoffset+256 ) 
	{
		if (  ObjectExist(t.tlmobj2) == 1 ) 
		{
			DeleteObject (  t.tlmobj2 );
		}
		++t.tlmobj2;
	}
}

void lm_createglassterrainobjects ( void )
{
	//  Always erase all terrain objects (detect changes eventually)
	//  as they are recreated (max 256 based on 16x16 slices)
	lm_deletelmobjectsfirst();

	//  Update terrain LOD so it's ALL highest LOD
	if (  t.terrain.TerrainID>0 ) 
	{
		BT_SetTerrainLODDistance (  t.terrain.TerrainID,1,9999999 );
		BT_SetTerrainLODDistance (  t.terrain.TerrainID,2,9999999 );
		BT_UpdateTerrainLOD (  t.terrain.TerrainID );
		BT_RenderTerrain (  t.terrain.TerrainID );
		BT_Intern_Render();
	}

	//  create LOD0 meshes from terrain, for use in lightmapping process
	t.LODLevel=0 ; t.tlmobj2=g.lightmappedobjectoffset ; g.lightmappedterrainoffset=t.tlmobj2;
	if (  t.terrain.TerrainID>0 ) 
	{
		//  terrain
		t.lmterrainshadrad=(3200/2)+400;
		for ( t.i = 0 ; t.i<= (int)BT_GetSectorCount(t.terrain.TerrainID,t.LODLevel)-1; t.i++ )
		{
			if (  BT_GetSectorExcluded(t.terrain.TerrainID,t.LODLevel,t.i) == 0 ) 
			{
				t.sopx_f=BT_GetSectorPositionX(t.terrain.TerrainID,t.LODLevel,t.i);
				t.sopy_f=BT_GetSectorPositionY(t.terrain.TerrainID,t.LODLevel,t.i);
				t.sopz_f=BT_GetSectorPositionZ(t.terrain.TerrainID,t.LODLevel,t.i);
				t.nearenoughtoentitytoadd=0;
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					t.tobj=t.entityelement[t.e].obj;
					if (  t.tobj>0 ) 
					{
						t.tentid=t.entityelement[t.e].bankindex;
						if (  t.entityprofile[t.tentid].ismarker == 0 ) 
						{
							if (  ObjectExist(t.tobj) == 1 ) 
							{
								t.tradwithheight=t.lmterrainshadrad+(ObjectSizeY(t.tobj,1)*2);
								if (  t.entityelement[t.e].x >= t.sopx_f-t.tradwithheight && t.entityelement[t.e].x<t.sopx_f+t.tradwithheight ) 
								{
									if (  t.entityelement[t.e].z >= t.sopz_f-t.tradwithheight && t.entityelement[t.e].z<t.sopz_f+t.tradwithheight ) 
									{
										t.nearenoughtoentitytoadd=1;
									}
								}
							}
						}
					}
				}
				if (  t.nearenoughtoentitytoadd == 1 ) 
				{
					//  create LOD0 object from this terrain segment
					if (  ObjectExist(t.tlmobj2) == 1  )  DeleteObject (  t.tlmobj2 );
					BT_MakeSectorObject (  t.terrain.TerrainID,t.LODLevel,t.i,t.tlmobj2 );
					MakeMeshFromObject (  t.tlmobj2,t.tlmobj2 );
					DeleteObject (  t.tlmobj2 );
					BT_MakeSectorObject (  t.terrain.TerrainID,t.LODLevel,t.i,0 );
					MakeObject (  t.tlmobj2,t.tlmobj2,0 );
					CloneMeshToNewFormat (  t.tlmobj2,0x002+0x100,1 );
					PositionObject (  t.tlmobj2,t.sopx_f,t.sopy_f,t.sopz_f );
					SetObjectMask (  t.tlmobj2, 1 );
					SetObjectCollisionProperty (  t.tlmobj2,1 );
					HideObject (  t.tlmobj2 );
				}
			}
			++t.tlmobj2;
		}
	}
	else
	{
		//  create large flat plane to catch shadow
		t.lmterrainshadrad=(5120/2)+400;
		for ( t.z = 0 ; t.z<=  9; t.z++ )
		{
			for ( t.x = 0 ; t.x<=  9; t.x++ )
			{
				if (  ObjectExist(t.tlmobj2) == 1  )  DeleteObject (  t.tlmobj2 );
				MakeObjectPlane (  t.tlmobj2,5120,5120 );
				OffsetLimb (  t.tlmobj2,0,0,1001,0 );
				RotateLimb (  t.tlmobj2,0,90,0,0 );
				MakeMeshFromObject (  t.tlmobj2,t.tlmobj2 );
				DeleteObject (  t.tlmobj2 );
				MakeObject (  t.tlmobj2,t.tlmobj2,0 );
				CloneMeshToNewFormat (  t.tlmobj2,0x002+0x200,1 );
				PositionObject (  t.tlmobj2,2560+(5120*t.x),0,2560+(5120*t.z) );
				SetObjectMask (  t.tlmobj2, 1 );
				SetObjectCollisionProperty (  t.tlmobj2,1 );
				HideObject (  t.tlmobj2 );
				++t.tlmobj2;
			}
		}
	}
	g.lightmappedterrainoffsetfinish=t.tlmobj2-1;
}

void lm_flashprompt ( void )
{
	t.tdisableLMprogressreading=1;
	for ( t.s = 0 ; t.s<=  1; t.s++ )
	{
		CLS (  Rgb(102,102,153) );
		lm_onscreenprompt ( );
		Sync (  );
	}
	t.tdisableLMprogressreading=0;
return;

}

void lm_onscreenprompt ( void )
{
	for ( t.tdouble = 0 ; t.tdouble<=  1; t.tdouble++ )
	{
		if (  t.tdouble == 0 ) 
		{
			if (  t.overlordmaxiterations>0 ) 
			{
				t.tslices_f=100.0/(t.overlordmaxiterations+0.0);
				t.tslicestart_f=t.tslices_f*t.overlordindex;
			}
			else
			{
				t.tslices_f=0;
				t.tslicestart_f=0;
			}
		}
		else
		{
			t.tslices_f=(100.0/(2+t.markslicesindex+t.tterrainobjectsbatches))/100.0;
			if (  t.tcurrententityindex <= 0 ) 
			{
				t.trevvalue=t.tterrainobjectsbatches-abs(t.tcurrententityindex);
				t.tslicestart_f=(100.0*t.tslices_f)*(t.trevvalue);
			}
			else
			{
				t.tslicestart_f=(100.0*t.tslices_f)*(1+t.tterrainobjectsbatches+t.markslicesindex);
			}
		}
		t.tcx=GetDisplayWidth()/2 ; t.tcy=(GetDisplayHeight()/2)-40+(t.tdouble*60);
		t.tsx=GetDisplayWidth()/4 ; t.tsy=24;
		/*
		Ink (  Rgb(0,0,0),0 );
		Line (  (t.tcx-t.tsx)+0,(t.tcy-t.tsy)+0,(t.tcx+t.tsx)-0,(t.tcy-t.tsy)+0 );
		Line (  (t.tcx-t.tsx)+0,(t.tcy+t.tsy)-0,(t.tcx+t.tsx)-0,(t.tcy+t.tsy)-0 );
		Line (  (t.tcx-t.tsx)+0,(t.tcy-t.tsy)+0,(t.tcx-t.tsx)+0,(t.tcy+t.tsy)-0 );
		Line (  (t.tcx+t.tsx)-0,(t.tcy-t.tsy)+0,(t.tcx+t.tsx)-0,(t.tcy+t.tsy)-0 );
		Ink (  Rgb(255,255,255),0 );
		Line (  (t.tcx-t.tsx)+1,(t.tcy-t.tsy)+1,(t.tcx+t.tsx)-1,(t.tcy-t.tsy)+1 );
		Line (  (t.tcx-t.tsx)+1,(t.tcy+t.tsy)-1,(t.tcx+t.tsx)-1,(t.tcy+t.tsy)-1 );
		Line (  (t.tcx-t.tsx)+1,(t.tcy-t.tsy)+1,(t.tcx-t.tsx)+1,(t.tcy+t.tsy)-1 );
		Line (  (t.tcx+t.tsx)-1,(t.tcy-t.tsy)+1,(t.tcx+t.tsx)-1,(t.tcy+t.tsy)-1 );
		*/
		if ( ImageExist ( g.editorimagesoffset+14 ) == 0 ) LoadImage (  "editors\\gfx\\14.png",g.editorimagesoffset+14 );
		float fMX = 1.0f;//(GetChildWindowWidth(1)+0.0) / 800.0f;
		float fMY = 1.0f;//(GetChildWindowHeight(1)+0.0) / 600.0f;
		Sprite ( 123, -10000, -10000, g.editorimagesoffset+14 );
		float fX1 = ((t.tcx-t.tsx)+1)*fMX;
		float fX2 = ((t.tcx+t.tsx)-1)*fMX;
		float fY1 = ((t.tcy-t.tsy)+1)*fMY;
		float fY2 = ((t.tcy+t.tsy)-1)*fMY;
		if ( fX2 < fX1 ) { float fSt = fX1; fX1 = fX2; fX2 = fSt; }
		if ( fY2 < fY1 ) { float fSt = fY1; fY1 = fY2; fY2 = fSt; }
		SizeSprite ( 123, fX2-fX1, 2 );
		PasteSprite ( 123, fX1, fY1 );
		SizeSprite ( 123, fX2-fX1, 2 );
		PasteSprite ( 123, fX1, fY2-2 );
		SizeSprite ( 123, 2, fY2-fY1 );
		PasteSprite ( 123, fX1, fY1 );
		SizeSprite ( 123, 2, fY2-fY1 );
		PasteSprite ( 123, fX2-2, fY1 );
		if ( ImageExist ( g.editorimagesoffset+15 ) == 0 ) LoadImage (  "editors\\gfx\\15.png",g.editorimagesoffset+15 );

		//t.talphacol as DWORD;
		t.talphacol=(128<<24)+(255<<16)+(255<<8)+255 ; Ink (  t.talphacol,0 );
		t.tdiffx_f=((t.tcx+t.tsx)-2)-((t.tcx-t.tsx)+2);
		if (  t.tdouble == 0 ) 
		{
			t.tdiffx_f=(t.tdiffx_f/100)*(t.tslicestart_f);
		}
		else
		{
			if (  t.tdisableLMprogressreading == 1 ) 
			{
				t.tdiffx_f=0;
			}
			else
			{
				if (  t.tdisableLMprogressreading == 2 ) 
				{
					t.tdiffx_f=t.rememberlasttdiffx_f;
				}
				else
				{
					t.tdiffx_f=(t.tdiffx_f/100)*(t.tslicestart_f+(LMGetPercent()*t.tslices_f));
					t.rememberlasttdiffx_f=t.tdiffx_f;
				}
			}
		}
		//Box (  (t.tcx-t.tsx)+2,(t.tcy-t.tsy)+2,(t.tcx-t.tsx)+2+(t.tdiffx_f),(t.tcy+t.tsy)-2 );
		if ( ImageExist ( g.editorimagesoffset+15 ) == 0 ) LoadImage (  "editors\\gfx\\15.png",g.editorimagesoffset+15 );
		Sprite ( 123, -10000, -10000, g.editorimagesoffset+15 );
		int iWidth = ((t.tcx-t.tsx)+2+(t.tdiffx_f))-((t.tcx-t.tsx)+2); if ( iWidth < 0 ) iWidth = 0;
		int iHeight = ((t.tcy+t.tsy)-2)-((t.tcy-t.tsy)+2); if ( iHeight < 0 ) iHeight = 0;
		SizeSprite ( 123, iWidth, iHeight );
		PasteSprite ( 123, (t.tcx-t.tsx)+2, (t.tcy-t.tsy)+2 );
		if (  t.tdouble == 0 ) 
		{
			if (  t.overlordx>0 || t.overlordz>0 ) 
			{
				t.theaderprompt_s = ""; t.theaderprompt_s=t.theaderprompt_s+"Lightmapping Grid "+Str(t.overlordx/t.overlordradius)+" x "+Str(t.overlordz/t.overlordradius);
			}
			else
			{
				if (  g.lmlightmapnowmode == 0  )  t.t_s = "";
				if (  g.lmlightmapnowmode == 1  )  t.t_s = "Quick";
				if (  g.lmlightmapnowmode == 2  )  t.t_s = "Basic";
				if (  g.lmlightmapnowmode == 3  )  t.t_s = "Lighting";
				if (  g.lmlightmapnowmode == 4  )  t.t_s = "Occlusion";
				t.theaderprompt_s=t.t_s+" Lightmapper (F1=Quick F2=Basic F3=Lights F4=Occlude)";
			}
		}
		else
		{
			t.theaderprompt_s=t.tonscreenprompt_s;
		}
		t.tbacktrack=getbitmapfontwidth(t.theaderprompt_s.Get(),1)/2;
		pastebitmapfont(t.theaderprompt_s.Get(),(GetDisplayWidth()/2)-t.tbacktrack,t.tcy-15,1,255);
	}
}

void lm_process ( void )
{
	//  Start the lightmap Timer (  )
	t.starttimer=Timer();

	//  remove any LM objects which no longer match entityelement data
	lm_removeold ( );

	//  Prepare terrain objects for scene
	timestampactivity(0,"LIGHTMAPPER: Create Glass Terrain Objects");
	lm_createglassterrainobjects ( );

	//  User prompt
	t.tdisableLMprogressreading=1;
	for ( t.n = 0 ; t.n<=  1; t.n++ )
	{
		t.tonscreenprompt_s="Preparing to Lightmap";
		if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
		lm_onscreenprompt() ; Sync ( );
	}
	t.tdisableLMprogressreading=0;

	//  use terrain objects to chart progress through overlord loop
	t.overlordmaxiterations=0;
	t.overlordradius=1024;
	for ( t.overlordz = 0 ; t.overlordz <= 51200 ; t.overlordz+= t.overlordradius )
	{
	for ( t.overlordx = 0 ; t.overlordx <= 51200 ; t.overlordx+= t.overlordradius )
	{
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			if (  ObjectExist(t.tlmobj2) == 1 )
			{
				bool bDoThisTerrainPiece = false;
				if ( g.iLightmappingDeactivateDirectionalLight == 1 && g.iLightmappingAllTerrainLighting == 1 ) 
					bDoThisTerrainPiece = true; // 250917 - all terrain needs shadow terrain if no directional light!
				if (  ObjectPositionX(t.tlmobj2) >= t.overlordx && ObjectPositionX(t.tlmobj2)<t.overlordx+t.overlordradius ) 
					if (  ObjectPositionZ(t.tlmobj2) >= t.overlordz && ObjectPositionZ(t.tlmobj2)<t.overlordz+t.overlordradius ) 
						bDoThisTerrainPiece = true;
				if ( bDoThisTerrainPiece == true )
				{
					++t.overlordmaxiterations;
				}
			}
		}
	}
	}
	t.overlordindex=0;

	//  Overruling sliced up terrain and LM objects, we lightmap by quadrant
	//  to ensure system memory is conserved into managable grids of work
	t.lightmappedobjnumbersofar=g.lightmappedterrainoffsetfinish+1;
	t.storelasttexturefilenumber=0;
	Dim (  t.overlordprocessed,g.glmsceneentitymax  );
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (t.e < t.entityelement.size())
		{
			t.overlordprocessed[t.e] = 0;
			t.lmsceneobj[t.e].includerotandscale = 0;
			t.lmsceneobj[t.e].reverseframes = 0;
		}
	}

	//  Start BIG OVERLORD LOOP
	Dim (  t.objsaveexclude,g.lightmappedobjectoffsetlast-g.lightmappedobjectoffset  );
	for ( t.overlordz = 0 ; t.overlordz<=  51200 ; t.overlordz+= t.overlordradius )
	{
	for ( t.overlordx = 0 ; t.overlordx<=  51200 ; t.overlordx+= t.overlordradius )
	{

	//  Work out terrain iterations (as can eat huge amounts of system memory)
	t.tterrainobjectscount=0;
	for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
	{
		if ( ObjectExist(t.tlmobj2) == 1 )
		{
			HideObject (  t.tlmobj2 );
			bool bDoThisTerrainPiece = false;
			if ( g.iLightmappingDeactivateDirectionalLight == 1 && g.iLightmappingAllTerrainLighting == 1) 
				bDoThisTerrainPiece = true; // 250917 - all terrain needs shadow terrain if no directional light!
			if (  ObjectPositionX(t.tlmobj2) >= t.overlordx && ObjectPositionX(t.tlmobj2)<t.overlordx+t.overlordradius ) 
				if (  ObjectPositionZ(t.tlmobj2) >= t.overlordz && ObjectPositionZ(t.tlmobj2)<t.overlordz+t.overlordradius ) 
					bDoThisTerrainPiece = true;

			if ( bDoThisTerrainPiece == true )
			{
				ShowObject (  t.tlmobj2 );
				++t.tterrainobjectscount;
			}
		}
	}
	if (  t.tterrainobjectscount>0  )  ++t.overlordindex;
	t.tcurrententityindex=0;

	//  mark entities we can process in this grid section
	t.tcountoverlordmarks=0;
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		t.tentid=t.entityelement[t.e].bankindex;
		t.tvalidshadereffect = GetIsValidShader ( t.entityprofile[t.tentid].effect_s.Get() );
		t.tokay=0;
		if (  t.entityelement[t.e].obj>0 && t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.entityprofile[t.tentid].ismarker == 0 && t.tvalidshadereffect == 1  )  t.tokay = 1;
		if (  t.tokay == 1 ) 
		{
			if (  t.overlordprocessed[t.e] == 0 ) 
			{
				t.tdx_f=t.entityelement[t.e].x-(t.overlordx+(t.overlordradius/2));
				t.tdz_f=t.entityelement[t.e].z-(t.overlordz+(t.overlordradius/2));
				t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
				if (  t.tdd_f<((t.overlordradius+0.0)*1.1) ) 
				{
					t.overlordprocessed[t.e]=1;
					++t.tcountoverlordmarks;
				}
			}
		}
	}

	//  Convert all objects to be lightmapped to standard static FVF
	if (  t.tcountoverlordmarks>0 || t.tterrainobjectscount>0 ) 
	{
		t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Overlord Grid Work. "+Str(t.tcountoverlordmarks)+" Objects and "+Str(t.tterrainobjectscount)+" Terrain at "+Str(t.overlordx/t.overlordradius)+" t.x "+Str(t.overlordz/t.overlordradius);
		timestampactivity(0, t.strwork.Get() );
		t.tlmobj=t.lightmappedobjnumbersofar;
		t.tlmobjstartedfrom=t.tlmobj;
		for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
		{
			t.tentid=t.entityelement[t.e].bankindex;
			t.tobj=t.entityelement[t.e].obj;
			if (  t.tobj>0 && t.overlordprocessed[t.e] == 1 ) 
			{
				t.ttsourceobj=g.entitybankoffset+t.tentid;
				t.tvalidshadereffect = GetIsValidShader ( t.entityprofile[t.tentid].effect_s.Get() );
				if ( t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.entityprofile[t.tentid].ismarker == 0 && t.tvalidshadereffect == 1 ) 
				{
					//  hide instance
					HideObject ( t.tobj );
					if (  t.lmsceneobj[t.e].lmvalid == 0 ) 
					{
						//  create one or more LM objects
						t.lmsceneobj[t.e].startobj=t.tlmobj;
						t.lmsceneobj[t.e].reverseframes=t.entityprofile[t.tentid].reverseframes;

						//  create new object ready for lightmapping
						t.tmultimatcount=GetMultiMaterialCount(t.ttsourceobj);
						if (  t.tmultimatcount == 0 ) 
						{
							t.mastertlmobj=t.tlmobj;
						}
						else
						{
							t.mastertlmobj=t.tlmobj+t.tmultimatcount+1;
						}

						// if original object not instance, can use it for the clone (keeps specular and things)
						int iSrcObjectToUse = t.ttsourceobj;
						sObject* pSrcObject = GetObjectData ( t.tobj );
						if ( pSrcObject )
						{
							if (  t.tmultimatcount == 0 ) 
							{
								if ( pSrcObject->pInstanceOfObject == NULL )
								{
									iSrcObjectToUse = t.tobj;
								}
							}
						}
						if (  ObjectExist(t.mastertlmobj) == 1  )  DeleteObject (  t.mastertlmobj );
						CloneObject ( t.mastertlmobj, iSrcObjectToUse, 0 );

						// set lightmapper obj range
						if ( t.tlmobj > g.lightmappedobjectoffsetfinish )  g.lightmappedobjectoffsetfinish = t.tlmobj;
						if ( t.tlmobj >= t.lightmappedobjnumbersofar )  t.lightmappedobjnumbersofar = t.tlmobj+1;

						// perhaps should clone from loaded/prepared ent obj not parent obj (specular?)
						// also be aware when we merge this spec call is repeated!
						if ( t.entityprofile[t.tentid].ismarker == 0 && t.e > 0 )
						{
							SetObjectSpecularPower ( t.mastertlmobj, t.entityelement[t.e].eleprof.specularperc / 100.0f );
						}
						if ( t.entityprofile[t.tentid].uvscrollu != 0.0f 
						||   t.entityprofile[t.tentid].uvscrollv != 0.0f 
						||   t.entityprofile[t.tentid].uvscaleu != 1.0f 
						||   t.entityprofile[t.tentid].uvscalev != 1.0f )
						{
							SetObjectScrollScaleUV ( t.mastertlmobj, t.entityprofile[t.tentid].uvscrollu, t.entityprofile[t.tentid].uvscrollv, t.entityprofile[t.tentid].uvscaleu, t.entityprofile[t.tentid].uvscalev );
						}

						//  strip out LOD1 and LOD2 meshes
						t.tmultimatflag=GetMultiMaterialCount(t.mastertlmobj);
						PerformCheckListForLimbs (  t.mastertlmobj );
						t.tbestlod=-1;
						LPSTR pSimplygonBlenderStyle = NULL;
						for ( t.c = ChecklistQuantity() ; t.c >= 1 ; t.c+= -1 )
						{
							t.tname_s=Lower(ChecklistString(t.c));
							if (  t.tname_s == "lod_0" && (t.tbestlod == -1 || t.tbestlod>0)  )  t.tbestlod = 0;
							if (  t.tname_s == "lod_1" && (t.tbestlod == -1 || t.tbestlod>1)  )  t.tbestlod = 1;
							if (  t.tname_s == "lod_2" && (t.tbestlod == -1)  )  t.tbestlod = 2;

							// additional LODs to choose from (Simplygon->Blender)
							if ( strlen(t.tname_s.Get()) > 5 )
							{
								LPSTR pLODPart = t.tname_s.Get() + strlen(t.tname_s.Get()) - 5;
								if ( stricmp ( pLODPart, "_LOD1" )==NULL && (t.tbestlod == -1 || t.tbestlod>1) ) { t.tbestlod = 1;  pSimplygonBlenderStyle = Lower(ChecklistString(t.c)); }
								if ( stricmp ( pLODPart, "_LOD2" )==NULL && (t.tbestlod == -1) ) { t.tbestlod = 2; pSimplygonBlenderStyle = Lower(ChecklistString(t.c)); }
							}
						}
						if ( pSimplygonBlenderStyle!=NULL )
						{
							// uses _LOD1 and _LOD2 markers but _LOD0 not specified so find first mesh that is not LOD1/2
							// to be used as the BEST LOD mesh (the highest polygon one)
							char pTestWith[256];
							strcpy ( pTestWith, pSimplygonBlenderStyle );
							pTestWith[strlen(pSimplygonBlenderStyle)-5] = 0;
							for ( t.c = ChecklistQuantity() ; t.c >= 1 ; t.c+= -1 )
							{
								t.tname_s=Lower(ChecklistString(t.c));
								if ( stricmp ( t.tname_s.Get(), pTestWith )==NULL )
								{
									t.tbestlod = 0;
								}
							}
						}
						for ( t.c = ChecklistQuantity() ; t.c >= 1 ; t.c+= -1 )
						{
							t.tname_s=Lower(ChecklistString(t.c));
							t.tokay=0;
							if (  t.tbestlod >= 0 ) 
							{
								if (  t.tbestlod == 0 && (t.tname_s == "lod_1" || t.tname_s == "lod_2")  )  t.tokay = 1;
								if (  t.tbestlod == 1 && (t.tname_s == "lod_2")  )  t.tokay = 1;
								// additional LODs to choose from (Simplygon->Blender)
								if ( strlen(t.tname_s.Get()) > 5 )
								{
									LPSTR pLODPart = t.tname_s.Get() + strlen(t.tname_s.Get()) - 5;
									if ( t.tbestlod == 0 && (stricmp ( pLODPart, "_LOD1" )==NULL || stricmp ( pLODPart, "_LOD2" )==NULL)  ) t.tokay = 1;
									if ( t.tbestlod == 1 && (stricmp ( pLODPart, "_LOD2" )==NULL) ) t.tokay = 1;
								}
							}
							if (  t.tokay == 1 ) 
							{
								RemoveLimb (  t.mastertlmobj,t.c-1 );
							}
							else
							{
								//  seems X file load to can leave garbage (artist leftover) in matCombined
								//  so restore this to identity matrices (fixes wrong scaling of trees in foliage pack)
								if (  t.entityprofile[t.tentid].resetlimbmatrix == 1 ) 
								{
									OffsetLimb (  t.mastertlmobj,t.c-1,0,0,0,0 );
								}
							}
						}
						if (  t.entityprofile[t.tentid].fixnewy != 0 ) 
						{
							RotateObject (  t.mastertlmobj,0,t.entityprofile[t.tentid].fixnewy,0 );
							FixObjectPivot (  t.mastertlmobj );
						}
						SetObjectEffect (  t.mastertlmobj,0 );
						CloneMeshToNewFormat (  t.mastertlmobj,530,1 );

						//  is this entity a multimaterial model, create more LM objects (and reduce parent too at end)
						if (  t.tmultimatflag == 0 ) 
						{
							//  110115 - only for non multimaterial models
							t.storetlmobj=t.tlmobj ; t.tlmobj=t.mastertlmobj;
							lm_preplmobj () ; t.tlmobj=t.storetlmobj;
							//  leave to move to next one
							SetObjectWireframe (  t.tlmobj,1 );
							++t.tlmobj;
						}
						else
						{
							//  now need to create small LM objects from master
							PerformCheckListForLimbs (  t.mastertlmobj );
							t.tmultimatcount=ChecklistQuantity();
							for ( t.mpass = 1 ; t.mpass<=  t.tmultimatcount-1; t.mpass++ )
							{
								if (  GetMeshExist(g.meshlightmapwork) == 1  )  DeleteMesh (  g.meshlightmapwork );
								MakeMeshFromLimb (  g.meshlightmapwork,t.mastertlmobj,t.mpass );
								if (  GetMeshExist(g.meshlightmapwork) == 1 ) 
								{
									if (  ObjectExist(t.tlmobj) == 1  )  DeleteObject (  t.tlmobj );
									if (  t.tlmobj>g.lightmappedobjectoffsetfinish  )  g.lightmappedobjectoffsetfinish = t.tlmobj;
									if (  t.tlmobj >= t.lightmappedobjnumbersofar  )  t.lightmappedobjnumbersofar = t.tlmobj+1;
									MakeObject (  t.tlmobj,g.meshlightmapwork,-1 );
									SetObjectEffect (  t.tlmobj,0 );
									lm_preplmobj ( );
									++t.tlmobj;
								}
							}
							if (  GetMeshExist(g.meshlightmapwork) == 1  )  DeleteMesh (  g.meshlightmapwork );
							MakeMeshFromLimb (  g.meshlightmapwork,t.mastertlmobj,0 );
							if (  GetMeshExist(g.meshlightmapwork) == 1 ) 
							{
								if (  ObjectExist(t.tlmobj) == 1  )  DeleteObject (  t.tlmobj );
								if (  t.tlmobj>g.lightmappedobjectoffsetfinish  )  g.lightmappedobjectoffsetfinish = t.tlmobj;
								if (  t.tlmobj >= t.lightmappedobjnumbersofar  )  t.lightmappedobjnumbersofar = t.tlmobj+1;
								MakeObject (  t.tlmobj,g.meshlightmapwork,-1 );
								SetObjectEffect (  t.tlmobj,0 );
								lm_preplmobj ( );
								SetObjectWireframe (  t.tlmobj,1 );
								++t.tlmobj;
							}
							DeleteObject (  t.mastertlmobj );
						}
						t.lmsceneobj[t.e].finishobj=t.tlmobj-1;
						//  trigger this one to be lightmapped (next step)
						t.lmsceneobj[t.e].lmvalid=-2;
					}
				}
			}
		}

		//  Now we have formatted lightmap objects ready, let's glue the ones
		//  with same textures together, and in close proximity, thus reducing
		//  the individual draw calls when we come to render them
		t.tconsolidatelocallightmapobjectspolylimit=3000;
		timestampactivity(0,cstr(cstr("LIGHTMAPPER: Consolidate static objects (")+cstr(t.tconsolidatelocallightmapobjectspolylimit)+cstr(" poly limit)")).Get());
		t.tcountconsolidations=0;
		t.tconsolidatelocallightmapobjects = 1;
		if (  t.tconsolidatelocallightmapobjects == 1 ) 
		{
			t.tlobjmax=t.lightmappedobjnumbersofar-t.tlmobjstartedfrom;
			if (  t.tlobjmax<1  )  t.tlobjmax = 1;
			t.tlobjmax=t.tlobjmax*2;
			Dim (  t.gtlobjused,t.tlobjmax );
			for ( t.tlmobjindex = 1 ; t.tlmobjindex <= t.tlobjmax ; t.tlmobjindex++ ) t.gtlobjused[t.tlmobjindex]=0 ; 
			for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
			{
				bool bEBENeedsToRetainMeshesForMaterialArbValue = false;
				if ( t.entityprofile[t.entityelement[t.e].bankindex].isebe != 0 ) bEBENeedsToRetainMeshesForMaterialArbValue = true;
				if ( bEBENeedsToRetainMeshesForMaterialArbValue == true && t.lmsceneobj[t.e].lmvalid == -2 && t.lmsceneobj[t.e].startobj>0 && t.overlordprocessed[t.e] == 1 ) 
				{
					t.tlmobj = t.lmsceneobj[t.e].startobj;
					if (  ObjectExist(t.tlmobj) == 1 ) 
					{
						// untouched LM object that's an EBE needs its first limb removing
						if ( GetMeshExist(g.meshlightmapwork) == 1 ) DeleteMesh ( g.meshlightmapwork );
						if ( ObjectExist(g.tempobjectoffset) == 1 ) DeleteObject ( g.tempobjectoffset );
						MakeObjectCube ( g.tempobjectoffset, 0 );
						CloneMeshToNewFormat ( g.tempobjectoffset, 530, 1 );
						MakeMeshFromLimb ( g.meshlightmapwork, g.tempobjectoffset, 0 );
						ChangeMesh ( t.lmsceneobj[t.e].startobj, 0, g.meshlightmapwork );

						// not consolidated, so preserve rotation
						t.lmsceneobj[t.e].includerotandscale=1;
						lm_preplmobj ( );
					}
				}
				if ( bEBENeedsToRetainMeshesForMaterialArbValue == false && t.lmsceneobj[t.e].lmvalid == -2 && t.lmsceneobj[t.e].startobj>0 && t.overlordprocessed[t.e] == 1 ) 
				{
					// untouched LM object, use this as the base to add similar local objects to get particulars of this LM objects
					for ( t.tlmobj = t.lmsceneobj[t.e].startobj; t.tlmobj <= t.lmsceneobj[t.e].finishobj; t.tlmobj++ )
					{
						if (  ObjectExist(t.tlmobj) == 1 ) 
						{
							if (  1 ) 
							{
								++t.tcountconsolidations;
								t.tmasterx_f=ObjectPositionX(t.tlmobj);
								t.tmastery_f=ObjectPositionY(t.tlmobj);
								t.tmasterz_f=ObjectPositionZ(t.tlmobj);
								//  detect if object contains GLASS, if so, do NOT consolidate
								t.tskipconsolidation=0;
								PerformCheckListForLimbs (  t.tlmobj );
								for ( t.tlmi = 0 ; t.tlmi<=  ChecklistQuantity()-1; t.tlmi++ )
								{
									t.tname_s=Lower(ChecklistString(1+t.tlmi));
									if (  t.tname_s == "glass" ) 
									{
										t.tskipconsolidation=1;
									}
								}
								//  240315 - also skip consolidation if cumilative polygons will overload shader mesh maker (max 65535 vertices)
								if (  GetObjectPolygonCount(t.tlmobj)>21840 ) 
								{
									t.tskipconsolidation=2;
								}
								//  remove any rotation from the master object and move to root limb
								if (  GetObjectPolygonCount(t.tlmobj) == 0 || t.tskipconsolidation != 0 ) 
								{
									//  parent object has NO polygons, so skip this entirely!
								}
								else
								{
									//  if have polys, MUST convert each object
									t.tcreatedmasterrecepticle=0;
									//  mark as used (also master object to glue stuff to)
									t.tlmobjindex=t.tlmobj-t.tlmobjstartedfrom;
									t.gtlobjused[t.tlmobjindex]=-1;
									//  object exists, glue stuff to it
									if (  GetMeshExist(g.meshlightmapwork) == 1  )  DeleteMesh (  g.meshlightmapwork );
									MakeMeshFromObject (  g.meshlightmapwork,t.tlmobj );
									if (  GetMeshExist(g.meshlightmapwork) == 1 ) 
									{
										//  go through all other 'untouched' objects
										t.tlastpolycount=0;
										for ( t.ee = 1; t.ee <= g.glmsceneentitymax; t.ee++ ) // 040116 - changed t.e to to 1 to ensure maximum opportunity to batch anything in range
										{
											// 040116 - added  t.e != t.ee to ensure first mesh not added twice!
											t.ttlmobjstart=t.lmsceneobj[t.ee].startobj;
											bool bEBENeedsToRetainMeshesForMaterialArbValue = false;
											if ( t.entityprofile[t.entityelement[t.ee].bankindex].isebe != 0 ) bEBENeedsToRetainMeshesForMaterialArbValue = true;
											if ( t.e != t.ee && t.lmsceneobj[t.ee].lmvalid == -2 && t.ttlmobjstart>0 && t.overlordprocessed[t.ee] == 1 && bEBENeedsToRetainMeshesForMaterialArbValue == false ) 
											{
												//  this object is available for gluing
												t.ttlmobjfinish=t.lmsceneobj[t.ee].finishobj;
												for ( t.ttlmobj = t.ttlmobjstart ; t.ttlmobj<=  t.ttlmobjfinish; t.ttlmobj++ )
												{
													if (  ObjectExist(t.ttlmobj) == 1 ) 
													{
														t.ttlmobjindex=t.ttlmobj-t.tlmobjstartedfrom;
														if (  t.gtlobjused[t.ttlmobjindex] == 0 ) 
														{
															// 040115 - find first limb which HAS a mesh (thus a texture) - a two frame mesh with the first being an empty root will FAIL this test!
															int iLimbThatHasMeshTexture = 0;
															sObject* pThisObj = GetObjectData(t.tlmobj);
															if ( pThisObj )
															{
																for ( int iF=0; iF<pThisObj->iFrameCount; iF++ )
																{
																	if ( pThisObj->ppFrameList[iF]->pMesh )
																	{
																		iLimbThatHasMeshTexture = iF;
																		break;
																	}
																}
															}
															int iOtherLimbThatHasMeshTexture = 0;
															sObject* pOtherObj = GetObjectData(t.ttlmobj);
															if ( pOtherObj )
															{
																for ( int iF=0; iF<pOtherObj->iFrameCount; iF++ )
																{
																	if ( pOtherObj->ppFrameList[iF]->pMesh )
																	{
																		iOtherLimbThatHasMeshTexture = iF;
																		break;
																	}
																}
															}
															if (  GetLimbTexturePtr(t.tlmobj,iLimbThatHasMeshTexture) == GetLimbTexturePtr(t.ttlmobj,iOtherLimbThatHasMeshTexture) && GetLimbTexturePtr(t.tlmobj,iLimbThatHasMeshTexture)>0 ) 
															{
																t.tdx_f=t.entityelement[t.e].x-t.entityelement[t.ee].x;
																t.tdz_f=t.entityelement[t.e].z-t.entityelement[t.ee].z;
																t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
																if (  t.tdd_f<1000 ) 
																{
																	//  this object close enough and of same texture index, add this object to master object
																	t.tnomorespace=0;
																	if (  GetObjectVertexCount(t.tlmobj)+GetObjectVertexCount(t.ttlmobj) >= 65530  )  t.tnomorespace = 1;
																	if (  t.tnomorespace == 0 && t.tlastpolycount+GetObjectPolygonCount(t.ttlmobj) <= t.tconsolidatelocallightmapobjectspolylimit ) 
																	{
																		//  only when KNOW we are doing this, create the new master mesh for consolidation
																		if (  t.tcreatedmasterrecepticle == 0 ) 
																		{
																			//  start a new master mesh object
																			DeleteObject (  t.tlmobj );
																			t.tlmobjdest=t.tlmobj;
																			MakeObject (  t.tlmobjdest,g.meshlightmapwork,-1 );																			
																			PositionObject (  t.tlmobjdest,t.tmasterx_f,t.tmastery_f,t.tmasterz_f );
																			t.tcreatedmasterrecepticle=1;
																		}
																		//  and within polygon count max limit
																		if (  GetMeshExist(g.meshlightmapwork) == 1  )  DeleteMesh (  g.meshlightmapwork );
																		MakeMeshFromObject (  g.meshlightmapwork,t.ttlmobj );
																		//  add this mesh to the master object
																		PerformCheckListForLimbs (  t.tlmobjdest );
																		AddLimb (  t.tlmobjdest,ChecklistQuantity(),g.meshlightmapwork );
																		t.tox_f=ObjectPositionX(t.ttlmobj)-t.tmasterx_f;
																		t.toy_f=ObjectPositionY(t.ttlmobj)-t.tmastery_f;
																		t.toz_f=ObjectPositionZ(t.ttlmobj)-t.tmasterz_f;
																		OffsetLimb (  t.tlmobjdest,ChecklistQuantity(),t.tox_f,t.toy_f,t.toz_f );
																		//  now create a super-mesh which includes both master and new object
																		DeleteMesh (  g.meshlightmapwork );
																		MakeMeshFromObject (  g.meshlightmapwork,t.tlmobjdest );
																		//  and create a new master object which combines both
																		DeleteObject (  t.tlmobjdest );
																		MakeObject (  t.tlmobjdest,g.meshlightmapwork,-1 );
																		PositionObject (  t.tlmobjdest,t.tmasterx_f,t.tmastery_f,t.tmasterz_f );
																		SetObjectMask (  t.tlmobjdest, 1 );
																		if (  t.entityelement[t.e].staticflag == 1 ) 
																		{
																			if (  t.entityprofile[t.entityelement[t.e].bankindex].canseethrough == 1 ) 
																			{
																				SetObjectCollisionProperty (  t.tlmobjdest,1 );
																			}
																		}
																		if (  t.entityprofile[t.entityelement[t.e].bankindex].ischaracter == 0 ) 
																		{
																			if (  t.entityprofile[t.entityelement[t.e].bankindex].collisionmode == 11 ) 
																			{
																				SetObjectCollisionProperty (  t.tlmobjdest,1 );
																			}
																		}
																		//  delete the doner object and work mesh
																		if (  GetMeshExist(g.meshlightmapwork) == 1  )  DeleteMesh (  g.meshlightmapwork );
																		CloneObject (  t.tlmobjdest,t.tlmobj,101 );
																		DeleteObject (  t.ttlmobj );
																		//  ensure deleted doner object info saved (so real ent can be hidden/deleted later)
																		//  record size of this object polygons
																		t.tlastpolycount=GetObjectPolygonCount(t.tlmobj);
																		// when merge objects together, still need to retain specular power
																		if ( t.e > 0 )
																		{
																			if ( t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 0 ) 
																			{
																				SetObjectSpecularPower ( t.tlmobj, t.entityelement[t.e].eleprof.specularperc / 100.0f );
																			}
																		}
																		//  add THIS object to the master
																		t.gtlobjused[t.ttlmobjindex]=t.ee;
																		//  also ensure this object does NOT get saved
																		t.objsaveexclude[t.ttlmobj-g.lightmappedobjectoffset]=1;
																	}
																	else
																	{
																		//  polygon count reached, skip rest for this batch
																		t.ttlmobj=t.ttlmobjfinish+1 ; t.ee=g.glmsceneentitymax+1;
																	}
																}
															}
														}
													}
												}
											}
										}
									}
									if (  t.tcreatedmasterrecepticle == 0 ) 
									{
										//  must take LM and bake in rotation and scale, even if not batched
										//  but only if not a multimesh otherwise two meshes get glued and one tex used
										//  and only if not made up of meshes with DIFFERENT textures
										t.tfirsttexname_s="" ; t.tonedifferent=0;
										PerformCheckListForLimbs (  t.tlmobj );
										for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
										{
											LPSTR sTmp = LimbTextureName(t.tlmobj, t.c - 1);
											t.ttexname_s = sTmp;
											if (sTmp) delete[] sTmp;
											if (  t.tfirsttexname_s == "" ) 
											{
												if (  t.ttexname_s != ""  )  t.tfirsttexname_s = t.ttexname_s;
											}
											else
											{
												if (  t.ttexname_s != t.tfirsttexname_s  )  t.tonedifferent = 1;
											}
										}
										if (  t.tonedifferent == 0 ) 
										{
											//  can consolidate into one mesh and one texture
											DeleteObject (  t.tlmobj );
											MakeObject (  t.tlmobj,g.meshlightmapwork,-1 );
											PositionObject (  t.tlmobj,t.tmasterx_f,t.tmastery_f,t.tmasterz_f );
											// when merge objects together, still need to retain specular power
											// can be improved by making spec part of makemeshfromX
											if ( t.e > 0 )
											{
												if ( t.entityprofile[t.entityelement[t.e].bankindex].ismarker == 0 ) 
												{
													SetObjectSpecularPower ( t.tlmobj, t.entityelement[t.e].eleprof.specularperc / 100.0f );
												}
											}
										}
										else
										{
											//  if not touched at all, ensure properties restored
											t.lmsceneobj[t.e].includerotandscale=1;
											lm_preplmobj ( );
										}
									}
								}
								if (  t.tskipconsolidation == 2 ) 
								{
									if (  t.lmsceneobj[t.e].includerotandscale == 0 ) 
									{
										t.lmsceneobj[t.e].includerotandscale=1;
										lm_preplmobj ( );
									}
								}
							}
						}
					}
				}
			}
			//  only when ALL objects consolidated, mark entities we should hide (as they have been consolidated)
			for ( t.ttlmobjindex = 1 ; t.ttlmobjindex<=  t.tlobjmax; t.ttlmobjindex++ )
			{
				t.ee=t.gtlobjused[t.ttlmobjindex];
				if (  t.ee>0 ) 
				{
					t.lmsceneobj[t.ee].startobj=abs(t.lmsceneobj[t.ee].startobj)*-1;
				}
			}
			UnDim (  t.gtlobjused );
		}
		t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Completed Consolidation with "+Str(t.tcountconsolidations)+" new objects";
		timestampactivity(0, t.strwork.Get() );

	}

	//  Go through a series of grouped entities
	t.markslicesindex=0 ; t.tcurrententitygroup=1;
	while (  t.tcurrententityindex <= g.glmsceneentitymax && (t.tcountoverlordmarks>0 || t.tterrainobjectscount>0) ) 
	{

	//  Group title for this batch
	t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Group "+Str(t.tcurrententitygroup)+" at entity t.index "+Str(t.tcurrententityindex);
	timestampactivity(0, t.strwork.Get() );

	//  Prepare all entity lightmapped objects for scene (objs start at end of terrain object set)
	if (  t.tcurrententityindex <= 0 ) 
	{
		//  signifies doing terrain objects first
		t.strwork = "" ; t.strwork = t.strwork + "LIGHTMAPPER: Prepare Terrain For Pre-Baking ("+Str(t.tterrainobjectscount)+" segments)";
		timestampactivity(0, t.strwork.Get() );
	}
	else
	{
		//  signifies doing all objects in slices
		t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Prepare Objects For Pre-Baking ("+Str(t.tcurrententityindex)+" to "+Str(g.glmsceneentitymax)+")";
		timestampactivity(0, t.strwork.Get() );
		t.tnextentityindex=-1;
		t.tobjectstobelitcount=0;
		for ( t.e = t.tcurrententityindex ; t.e<=  g.glmsceneentitymax; t.e++ )
		{
			t.tobj = t.entityelement[t.e].obj;
			if ( t.tobj>0 && t.overlordprocessed[t.e] == 1 ) 
			{
				t.tentid=t.entityelement[t.e].bankindex;
				t.tvalidshadereffect = GetIsValidShader ( t.entityprofile[t.tentid].effect_s.Get() );
				if (  t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.tvalidshadereffect == 1 ) 
				{
					//  hide instance
					if (  t.lmsceneobj[t.e].lmvalid == -2 ) 
					{
						//  ready for next step in process
						t.lmsceneobj[t.e].lmvalid=2;
						++t.tobjectstobelitcount;

						//  copy entityelement info to this LM obj
						t.lmsceneobj[t.e].bankindex=t.entityelement[t.e].bankindex;
						t.lmsceneobj[t.e].x=t.entityelement[t.e].x;
						t.lmsceneobj[t.e].y=t.entityelement[t.e].y;
						t.lmsceneobj[t.e].z=t.entityelement[t.e].z;
						t.lmsceneobj[t.e].rx=t.entityelement[t.e].rx;
						t.lmsceneobj[t.e].ry=t.entityelement[t.e].ry;
						t.lmsceneobj[t.e].rz=t.entityelement[t.e].rz;
						t.lmsceneobj[t.e].sx=t.entityelement[t.e].scalex;
						t.lmsceneobj[t.e].sy=t.entityelement[t.e].scaley;
						t.lmsceneobj[t.e].sz=t.entityelement[t.e].scalez;
					}
				}
			}
		}
		if (  t.tnextentityindex == -1 ) 
		{
			t.strwork = "" ; t.strwork = t.strwork + "LIGHTMAPPER: Working on all "+Str(t.tobjectstobelitcount)+" objects";
			timestampactivity(0, t.strwork.Get() );
		}
		else
		{
			t.strwork = "" ; t.strwork = t.strwork + "LIGHTMAPPER: Slicing work at "+Str(t.tnextentityindex-1);
			timestampactivity(0, t.strwork.Get() );
		}
		if (  t.tnextentityindex == -1  )  t.tnextentityindex = g.glmsceneentitymax+1;
	}

	//  First scan to see if any ACTUAL lightmapping would take place (collision objects needed)
	timestampactivity(0,"LIGHTMAPPER: Scan for relevant collision objects");
	t.colobjworkcount=0;
	Dim (  t.colobjworke,g.entityelementlist  );
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		t.tobj=t.entityelement[t.e].obj;
		if (  t.tobj>0 ) 
		{
			t.tentid=t.entityelement[t.e].bankindex;
			t.tvalidshadereffect = GetIsValidShader ( t.entityprofile[t.tentid].effect_s.Get() );
			if (  t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.entityprofile[t.tentid].ismarker == 0 && t.tvalidshadereffect == 1 ) 
			{
				//  only choose those near enough to cast a possible shadow on current target
				t.tokay=0;
				if (  t.tcurrententityindex <= 0 ) 
				{
					//  shadowing terrain
					if (  t.tterrainobjectscount>0 ) 
					{
						t.tradwithheight=t.lmterrainshadrad+(ObjectSizeY(t.tobj,1)*2);
						for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
						{
							if (  ObjectExist(t.tlmobj2) == 1 ) 
							{
								if (  ObjectPositionX(t.tlmobj2) >= t.overlordx && ObjectPositionX(t.tlmobj2)<t.overlordx+t.overlordradius ) 
								{
									if (  ObjectPositionZ(t.tlmobj2) >= t.overlordz && ObjectPositionZ(t.tlmobj2)<t.overlordz+t.overlordradius ) 
									{
										if (  t.entityelement[t.e].x >= ObjectPositionX(t.tlmobj2)-t.tradwithheight && t.entityelement[t.e].x<ObjectPositionX(t.tlmobj2)+t.tradwithheight ) 
										{
											if (  t.entityelement[t.e].z >= ObjectPositionZ(t.tlmobj2)-t.tradwithheight && t.entityelement[t.e].z<ObjectPositionZ(t.tlmobj2)+t.tradwithheight ) 
											{
												t.tokay=1;
											}
										}
									}
								}
							}
						}
					}
				}
				else
				{
					//  shadowing LM objects
					t.tdx_f=t.entityelement[t.e].x-(t.overlordx+(t.overlordradius/2));
					t.tdz_f=t.entityelement[t.e].z-(t.overlordz+(t.overlordradius/2));
					t.tdd_f=Sqrt(abs(t.tdx_f*t.tdx_f)+abs(t.tdz_f*t.tdz_f));
					if (  t.tdd_f<((t.overlordradius+0.0)*1.5) ) 
					{
						t.tokay=1;
					}
				}
				if (  t.tokay == 1 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						++t.colobjworkcount ; t.colobjworke[t.colobjworkcount]=t.e;
					}
				}
			}
		}
	}

	//  If nothing to lightmap, or nothing casting a shadow, can skip (unless terrain which needs light)
	if (  t.tcurrententityindex <= 0 || (t.colobjworkcount>0 && (t.tcurrententityindex>0 && t.tobjectstobelitcount>0)) ) 
	{

	//  Prepare lightmapping stage
	t.quality_f=g.fLightmappingQuality;
	t.blurlevel_f=g.fLightmappingBlurLevel;
	if (  t.tcurrententityindex <= 0 ) 
	{
		t.texturesize=g.iLightmappingSizeTerrain;
	}
	else
	{
		t.texturesize=g.iLightmappingSizeEntity;
	}
	t.lightingthreads=-1;
	if (  g.lmlightmapnowmode == 1 || g.lmlightmapnowmode == 2 || g.lmlightmapnowmode == 3 ) 
	{
		//  quick lightmapping
		t.ambientocclusion=0;
		t.ambientocclusiondistance_f=0.0;
	}
	if (  g.lmlightmapnowmode == 4 ) 
	{
		//  full lightmapping with ambient occlusion
		t.ambientocclusion=10;
		t.ambientocclusiondistance_f=5.0;
	}
	LMStart (  );
	if (  PathExist(t.lightmapper.lmpath_s.Get()) == 0  )  MakeDirectory (  t.lightmapper.lmpath_s.Get() );
	LMSetMode (  0 );
	LMSetLightMapFileFormat (  1 );
	LMSetLightMapFolder (  t.lightmapper.lmpath_s.Get() );
	LMSetLightMapStartNumber (  t.storelasttexturefilenumber );
	//  include all static lights from scene
	if (  g.lmlightmapnowmode == 2 || g.lmlightmapnowmode == 3 || g.lmlightmapnowmode == 4 ) 
	{
		for ( t.l = 1 ; t.l<=  g.infinilightmax; t.l++ )
		{
			if (  t.infinilight[t.l].used == 1 && t.infinilight[t.l].type == 1 ) 
			{
				t.tr_f=t.infinilight[t.l].colrgb.r/255.0;
				t.tg_f=t.infinilight[t.l].colrgb.g/255.0;
				t.tb_f=t.infinilight[t.l].colrgb.b/255.0;
				t.tr_f=t.tr_f-0.0 ; t.tr_f=t.tr_f*2;
				t.tg_f=t.tg_f-0.0 ; t.tg_f=t.tg_f*2;
				t.tb_f=t.tb_f-0.0 ; t.tb_f=t.tb_f*2;
				t.trad_f=t.infinilight[t.l].range;
				t.tatten_f=16.0/(t.trad_f*t.trad_f);
				LMAddCustomPointLight (  t.infinilight[t.l].x,t.infinilight[t.l].y,t.infinilight[t.l].z,t.trad_f,t.trad_f,t.tatten_f,t.tr_f,t.tg_f,t.tb_f );
			}
		}
	}
	if ( g.iLightmappingDeactivateDirectionalLight == 0 )
	{
		LMAddDirectionalLight ( -0.25f, -0.5f, -0.25f, 1.05f, 1.05f, 1.05f );
	}
	//LMSetAmbientLight ( 0.25f, 0.25f, 0.25f );
	LMSetAmbientLight ( g.fLightmappingAmbientR, g.fLightmappingAmbientG, g.fLightmappingAmbientB );
	if ( t.ambientocclusion>0 ) 
	{
		LMSetAmbientOcclusionOn (  t.ambientocclusion,t.ambientocclusiondistance_f,1 );
	}

	//  Go through all entities and create collision data
	//  use actual entity instances to generate collision data (strips to LOD0 inside command)
	t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Create "+Str(t.colobjworkcount)+" Collision Objects";
	timestampactivity(0, t.strwork.Get() );
	for ( t.colobjworkindex = 1 ; t.colobjworkindex<=  t.colobjworkcount; t.colobjworkindex++ )
	{
		t.e=t.colobjworke[t.colobjworkindex];
		t.tobj=t.entityelement[t.e].obj;
		t.tentid=t.entityelement[t.e].bankindex;
		if (  t.entityprofile[t.tentid].castshadow != -1 ) 
		{
			if ( t.entityprofile[t.tentid].isebe != 0 )
			{
				// hide spikey sphere marker so it does not cast shadow
				if ( GetMeshExist(g.meshlightmapwork) == 1 ) DeleteMesh ( g.meshlightmapwork );
				if ( ObjectExist(g.tempobjectoffset) == 1 ) DeleteObject ( g.tempobjectoffset );
				MakeObjectCube ( g.tempobjectoffset, 0 );
				CloneMeshToNewFormat ( g.tempobjectoffset, 530, 1 );
				MakeMeshFromLimb ( g.meshlightmapwork, g.tempobjectoffset, 0 );
				int iObjectToWorkOn = t.tobj;
				sObject* pObject = GetObjectData ( t.tobj );
				if ( pObject )
					if ( pObject->pInstanceOfObject )
						iObjectToWorkOn = pObject->pInstanceOfObject->dwObjectNumber;
				ChangeMesh ( iObjectToWorkOn, 0, g.meshlightmapwork );
			}
			if (  t.entityprofile[t.tentid].transparency>0 ) 
			{
				if (  g.lmlightmapnowmode == 2 || g.lmlightmapnowmode == 3 || g.lmlightmapnowmode == 4 ) 
				{
					LMAddTransparentCollisionObject (  t.tobj,1 );
				}
				else
				{
					LMAddCollisionObject (  t.tobj );
				}
			}
			else
			{
				LMAddCollisionObject (  t.tobj );
			}
		}
	}
	UnDim (  t.colobjworke );
	if (  t.tcurrententityindex>0 ) 
	{
		//  add terrain as colliders for objects (so ambient occlusion on objects can happen)
		if ( g.iLightmappingExcludeTerrain == 0 )
		{
			timestampactivity(0,"LIGHTMAPPER: Create Terrain Ray Collision Data");
			if (  g.lightmappedterrainoffset != -1 ) 
			{
				for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
				{
					if (  ObjectExist(t.tlmobj2) == 1 ) 
					{
						if (  GetVisible(t.tlmobj2) == 1 ) 
						{
							LMAddCollisionObject (  t.tlmobj2 );
						}
					}
				}
			}
		}
	}
	LMBuildCollisionData (  );

	// 150416 - prompt to know we are building the lightmapper objects
	t.tonscreenprompt_s="Building list of objects to lightmap";
	if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
	lm_onscreenprompt() ; Sync ( );

	//  Add objects to be lightmapped
	if (  t.tcurrententityindex <= 0 ) 
	{
		//  Add terrain objects for lightmapping
		if ( g.iLightmappingExcludeTerrain == 0 )
		{
			for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
			{
				if (  ObjectExist(t.tlmobj2) == 1 ) 
				{
					if (  GetVisible(t.tlmobj2) == 1 ) 
					{
						LMAddShadedLightMapObject (  t.tlmobj2,0,-1 );
					}
				}
			}
		}
	}
	else
	{
		//  Add entity objects for lightmapping
		timestampactivity(0,"LIGHTMAPPER: Add Objects To Be Lightmapped");
		for ( t.e = t.tcurrententityindex ; t.e<=  t.tnextentityindex-1; t.e++ )
		{
			if (  t.overlordprocessed[t.e] == 1 ) 
			{
				if (  t.lmsceneobj[t.e].lmvalid == 2 ) 
				{
					t.tobjstart=abs(t.lmsceneobj[t.e].startobj);
					for ( t.tlmobj = t.tobjstart ; t.tlmobj<=  t.lmsceneobj[t.e].finishobj; t.tlmobj++ )
					{
						if (  ObjectExist(t.tlmobj) == 1 ) 
						{
							// 011215 - solve facet issue by rewelding normals together (anything less than 50 degrees is blended for curved surfaces)
							// this did not work, and might even be messing them up!
							//if ( g.fLightmappingSmoothAngle > 0 )
							//	SetObjectSmoothing ( t.tlmobj, g.fLightmappingSmoothAngle );

							// then add to be lightmapped
							LMAddShadedLightMapObject ( t.tlmobj, 1, 0 ); // 010217 - flag to force no flat shading
						}
					}
				}
			}
		}
	}

	//  Calculate all lightmaps
	timestampactivity(0,"LIGHTMAPPER: Calculate Lightmaps");
	if (  t.lightingthreads == 0 ) 
	{
		//  single threading now uses time sliced commands
		LMBuildLightMapsStart (  t.texturesize,t.quality_f,t.blurlevel_f );
		while (  LMBuildLightMapsCycle() == 0 ) 
		{
			SleepNow (  50 );
			t.tonscreenprompt_s = ""; t.tonscreenprompt_s=t.tonscreenprompt_s+LMGetStatus()+" (hold ESCAPE to cancel)";
			if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
			lm_onscreenprompt ( );
			lm_focuscameraonoverlordxz ( );
			Sync (  );
		}
	}
	else
	{
		LMBuildLightMapsThread (  t.texturesize,t.quality_f,t.blurlevel_f,t.lightingthreads ) ;
		while (  LMGetComplete() == 0 ) 
		{
			SleepNow (  50 );
			t.tonscreenprompt_s = "" ; t.tonscreenprompt_s=t.tonscreenprompt_s+LMGetStatus()+" (hold ESCAPE to cancel)";
			if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
			lm_onscreenprompt ( );
			lm_focuscameraonoverlordxz ( );
			Sync (  );
		}
	}

	//  If ESCAPE key press detected, must cancel lightmapping step
	t.tskiplightmapcompletion=0;
	if (  EscapeKey() == 1 ) 
	{
		t.tdisableLMprogressreading=1;
		while (  EscapeKey() == 1 ) 
		{
			t.tonscreenprompt_s="Lightmapping Aborted";
			if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
			lm_onscreenprompt( ) ; Sync ( );
		}
		lm_emptylightmapandttsfilesfolder ( );
		t.tdisableLMprogressreading=0;
		t.tskiplightmapcompletion=1;
	}

	//  Finalise and save lightmaps
	if (  t.tskiplightmapcompletion == 1 ) 
	{
		timestampactivity(0,"LIGHTMAPPER: Lightmapping aborted.");
		LMReset (  );
		lm_deleteall ( );
		return;
	}
	else
	{
		timestampactivity(0,"LIGHTMAPPER: Finalise And Save Lightmap Textures");
		t.tdisableLMprogressreading=2;
		for ( t.n = 0 ; t.n<=  1; t.n++ )
		{
			if (  t.tcurrententityindex <= 0 ) 
			{
				t.tonscreenprompt_s="Saving Terrain Lightmaps";
			}
			else
			{
				t.tonscreenprompt_s="Saving Entity Lightmaps";
			}
			if ( t.hardwareinfoglobals.noterrain == 0 ) terrain_update ( );
			lm_onscreenprompt() ; Sync ( );
		}
		t.tdisableLMprogressreading=0;
		LMCompleteLightMaps ( );
		timestampactivity(0,"LIGHTMAPPER: Lightmap bake completed");
		t.storelasttexturefilenumber=LMGetLightMapLastNumber();
	}

	//  Reset lightmapper system
	timestampactivity(0,"LIGHTMAPPER: Reset Lightmapper");
	LMReset ( );

	//  Skipped lm process
	}

	//  Finished light map, now save it out
	if (  t.tcurrententityindex <= 0 ) 
	{
		//  done terrain objects (only when tcurrententityindex>0), THEN do the entity objects
		++t.tcurrententitygroup;
		t.tnextentityindex=t.tcurrententityindex+1;
	}
	else
	{
		for ( t.e = t.tcurrententityindex ; t.e<=  t.tnextentityindex-1; t.e++ )
		{
			if (  t.lmsceneobj[t.e].lmvalid == 2 ) 
			{
				t.lmsceneobj[t.e].needsaving=1;
				t.lmsceneobj[t.e].lmvalid=3;
			}
		}
	}

	//  Loop to group lightmapping into a series of memory saving batches
	t.tcurrententityindex=t.tnextentityindex;
	}
	if (  t.tcountoverlordmarks>0 || t.tterrainobjectscount>0 ) 
	{
		timestampactivity(0,"LIGHTMAPPER:");
		t.tterrainobjectscount=0;
		t.tcountoverlordmarks=0;
	}

	//  Finally mark ones we baked as used, and restore the 'collision radius ones'
	for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
	{
		if (  t.overlordprocessed[t.e] == 1  )  t.overlordprocessed[t.e] = 3;
		if (  t.overlordprocessed[t.e] == 2  )  t.overlordprocessed[t.e] = 0;
		if (  t.overlordprocessed[t.e] == 4  )  t.overlordprocessed[t.e] = 3;
	}

	//  We step through each grid in the entire world (keeps workload groups small)
	}
	}
	timestampactivity(0,"LIGHTMAPPER: Finished overlord loop");
	UnDim (  t.overlordprocessed );

	//  only if processed anything for this grid
	if (  1 ) 
	{
		//  Raise terrain light map objects
		t.liftshadowstositontopofterrain_f = 0.1f;
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			if (  ObjectExist(t.tlmobj2) == 1 ) 
			{
				PositionObject (  t.tlmobj2,ObjectPositionX(t.tlmobj2),t.liftshadowstositontopofterrain_f,ObjectPositionZ(t.tlmobj2) );
				SetObjectTransparency (  t.tlmobj2, 6 );
				lm_zbias ( );
			}
		}

		//  Prepare shader for light map objects
		timestampactivity(0,"LIGHTMAPPER: Apply Static Shaders");
		lm_handleshaders ( );

		//  Save lightmapped scene objects
		timestampactivity(0,"LIGHTMAPPER: Save Lightmap Files");
		lm_savescene ( );

		//  Finally show results of lightmapping (hide static entities)
		timestampactivity(0,"LIGHTMAPPER: Show Lightmap Results");
		lm_showall ( );

	}

	//  free usage array when done
	UnDim (  t.objsaveexclude );

	//  End lightmapper Timer (  )
	t.endtimer = Timer();
}

void lm_zbias ( void )
{
	//  ZBIAS to avoid shadows sinking
	EnableObjectZBias ( t.tlmobj2, -0.6f, -0.0000005f ); // DX11 values set in DX11 creation of renderstate!!

	// force bShadowBlend so can use SRC=DEST_COLOR blend and vice versa
	sObject* pObject = GetObjectData ( t.tlmobj2 );
	for ( int iMeshIndex = 0; iMeshIndex < pObject->iMeshCount; iMeshIndex++ )
	{
		sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
		pMesh->bShadowBlend = true;
	}
}

void lm_focuscameraonoverlordxz ( void )
{
	//  slowly slide to the next grid we are working on
	t.tfocuscamx_f=t.overlordx+(t.overlordradius/2);
	t.tfocuscamz_f=t.overlordz+(t.overlordradius/2);
	PositionCamera (  0,CurveValue(t.tfocuscamx_f,CameraPositionX(0),10.0),CameraPositionY(0),CurveValue(t.tfocuscamz_f,CameraPositionZ(0),10.0) );
}

void lm_deleteall ( void )
{

	//  restore instance entities
	lm_restoreall ( );

	//  remove any LM objects
	for ( t.tlmobj = g.lightmappedobjectoffset; t.tlmobj<= g.lightmappedobjectoffsetfinish; t.tlmobj++ )
	{
		if (  ObjectExist(t.tlmobj) == 1 ) 
		{
			DeleteObject (  t.tlmobj );
		}
	}
	g.lightmappedobjectoffsetfinish=g.lightmappedobjectoffset-1;

	//  remove terrain objects
	if (  g.lightmappedterrainoffset != -1 ) 
	{
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			if (  ObjectExist(t.tlmobj2) == 1 ) 
			{
				DeleteObject (  t.tlmobj2 );
			}
		}
		g.lightmappedterrainoffset=-1;
		g.lightmappedterrainoffsetfinish=-1;
	}

	//  clear lmsceneobj list
	if (  ArrayCount(t.lmsceneobj) >= g.glmsceneentitymax ) 
	{
		for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
		{
			if (t.e < t.entityelement.size())
			{
				t.lmsceneobj[t.e].startobj = 0;
				t.lmsceneobj[t.e].finishobj = 0;
				t.lmsceneobj[t.e].lmvalid = 0;
			}
		}
	}
	g.glmsceneentitymax=0;
}

void lm_restoreall ( void )
{

	//  called by test game when editing and in-game through slider panel PRE-BAKEvsREALTIME

	//  hide LM objects and terrain
	lm_hide_lmos ( );

	//  restore static entities
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (  t.e <= ArrayCount(t.entityelement) ) 
		{
			t.tobj=t.entityelement[t.e].obj;
			if (  t.tobj>0 ) 
			{
				t.entid=t.entityelement[t.e].bankindex ; t.ttsourceobj=g.entitybankoffset+t.entid;
				t.tvalidshadereffect = GetIsValidShader ( t.entityprofile[t.entid].effect_s.Get() );
				if (  t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.entityprofile[t.entid].ismarker == 0 && t.tvalidshadereffect == 1 ) 
				{
					if (  ObjectExist(t.tobj) == 1 ) 
					{
						ShowObject (  t.tobj );
					}
				}
			}
		}
	}

return;

}

void lm_show_lmos ( void )
{

	//  show LM objects
	for ( t.tlmobj = g.lightmappedobjectoffset; t.tlmobj<= g.lightmappedobjectoffsetfinish; t.tlmobj++ )
	{
		if (  ObjectExist(t.tlmobj) == 1 ) 
		{
			ShowObject (  t.tlmobj );
		}
	}

	//  show terrain objects
	if (  g.lightmappedterrainoffset != -1 ) 
	{
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			if (  ObjectExist(t.tlmobj2) == 1 ) 
			{
				ShowObject (  t.tlmobj2 );
			}
		}
	}

return;

}

void lm_hide_lmos ( void )
{

	//  hide LM objects
	for ( t.tlmobj = g.lightmappedobjectoffset; t.tlmobj<= g.lightmappedobjectoffsetfinish; t.tlmobj++ )
	{
		if (  ObjectExist(t.tlmobj) == 1 ) 
		{
			HideObject (  t.tlmobj );
			//Dave Performance
			//SetIgnoreObject (  t.tlmobj , true );
		}
	}

	//  hide terrain objects
	if (  g.lightmappedterrainoffset != -1 ) 
	{
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			if (  ObjectExist(t.tlmobj2) == 1 ) 
			{
				HideObject (  t.tlmobj2 );
				//Dave Performance
				//SetIgnoreObject (  t.tlmobj2 , true );
			}
		}
	}

return;

}

void lm_showall ( void )
{

	//  called by slider panel PREBAKEvsREALTIME

	//  show LM objects and terrain
	lm_show_lmos ( );

	//  hide static entities (only if an LM object)
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (  t.e <= ArrayCount(t.lmsceneobj) ) 
		{
			if (t.e < t.entityelement.size())
			{
				t.tlmobj = t.lmsceneobj[t.e].startobj;
				if ((t.tlmobj > 0 && t.lmsceneobj[t.e].lmvalid == 3) || (t.tlmobj < 0))
				{
					t.tobj = t.entityelement[t.e].obj;
					if (t.tobj > 0)
					{
						if (ObjectExist(t.tobj) == 1)
						{
							t.tentid = t.entityelement[t.e].bankindex; t.ttsourceobj = g.entitybankoffset + t.tentid;
							t.tvalidshadereffect = GetIsValidShader(t.entityprofile[t.tentid].effect_s.Get());
							if (t.entityelement[t.e].staticflag == 1 && t.entityelement[t.e].eleprof.spawnatstart == 1 && t.entityprofile[t.tentid].ismarker == 0 && t.tvalidshadereffect == 1)
							{
								SetIgnoreObject(t.tobj, true);
							}
						}
					}
				}
			}
		}
	}
}

void lm_savescene ( void )
{
	// Save lightmapped scene objects
	t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Save Lightmap Files: "+Str(g.glmsceneentitymax)+" Objects";
	timestampactivity(0, t.strwork.Get() );
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (t.e < t.entityelement.size())
		{
			if (t.lmsceneobj[t.e].needsaving == 1)
			{
				t.tobjstart = abs(t.lmsceneobj[t.e].startobj);
				for (t.tobj = t.tobjstart; t.tobj <= t.lmsceneobj[t.e].finishobj; t.tobj++)
				{
					//  only objects that have not been excluded in consolidator
					if (t.objsaveexclude[t.tobj - g.lightmappedobjectoffset] == 0)
					{
						//  save the object
						t.tfile_s = t.lightmapper.lmpath_s + "object" + Str(t.tobj) + ".dbo";
						if (FileExist(t.tfile_s.Get()) == 1) DeleteAFile(t.tfile_s.Get());
						if (ObjectExist(t.tobj) == 1)
						{
							SetObjectWireframe(t.tobj, 0);
							SaveObject(t.tfile_s.Get(), t.tobj);
							//  discover which LM texture file used
							if (t.tobj == t.lmsceneobj[t.e].startobj)
							{
								t.tlmtextureindex = findlightmaptexturefilenameindex(t.tfile_s.Get());
								t.lmsceneobj[t.e].lmindex = t.tlmtextureindex;
							}
						}
					}
				}
				//  finished saving
				t.lmsceneobj[t.e].needsaving=0;
			}
		}
	}

	//  save all terrain objects
	t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Save Lightmap Files: "+Str(g.lightmappedterrainoffsetfinish-g.lightmappedterrainoffset)+" Terrain Objects";
	timestampactivity(0, t.strwork.Get() );
	for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
	{
		if (  ObjectExist(t.tlmobj2) == 1 ) 
		{
			t.tfile_s=t.lightmapper.lmpath_s+"object"+Str(t.tlmobj2)+".dbo";
			if (  FileExist(t.tfile_s.Get()) == 1  )  DeleteAFile (  t.tfile_s.Get() );
			SaveObject (  t.tfile_s.Get(),t.tlmobj2 );
		}
	}

	//  save new lm object list
	t.strwork = "" ; t.strwork = t.strwork + "LIGHTMAPPER: Save Lightmap Files: Saving "+t.lightmapper.lmobjectfile_s;
	timestampactivity(0, t.strwork.Get() );
	t.lmsceneobjversion=1004;
	if (  FileExist(t.lightmapper.lmobjectfile_s.Get()) == 1  )  DeleteAFile (  t.lightmapper.lmobjectfile_s.Get() );
	OpenToWrite (  1,t.lightmapper.lmobjectfile_s.Get() );
	WriteLong (  1,t.lmsceneobjversion );
	WriteLong (  1,g.glmsceneentitymax );
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		WriteLong (  1,t.lmsceneobj[t.e].startobj );
		WriteLong (  1,t.lmsceneobj[t.e].finishobj );
		WriteLong (  1,t.lmsceneobj[t.e].bankindex );
		WriteFloat (  1,t.lmsceneobj[t.e].x );
		WriteFloat (  1,t.lmsceneobj[t.e].y );
		WriteFloat (  1,t.lmsceneobj[t.e].z );
		WriteFloat (  1,t.lmsceneobj[t.e].rx );
		WriteFloat (  1,t.lmsceneobj[t.e].ry );
		WriteFloat (  1,t.lmsceneobj[t.e].rz );
		WriteFloat (  1,t.lmsceneobj[t.e].sx );
		WriteFloat (  1,t.lmsceneobj[t.e].sy );
		WriteFloat (  1,t.lmsceneobj[t.e].sz );
		WriteLong (  1,t.lmsceneobj[t.e].lmindex );
		WriteLong (  1,t.lmsceneobj[t.e].includerotandscale );
		if (  t.lmsceneobjversion >= 1004 ) 
		{
			WriteLong (  1,t.lmsceneobj[t.e].reverseframes );
		}
	}
	WriteLong (  1,g.lightmappedterrainoffset );
	WriteLong (  1,g.lightmappedterrainoffsetfinish );
	for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
	{
		if (  ObjectExist(t.tlmobj2) == 1 ) 
		{
			WriteFloat (  1,ObjectPositionX(t.tlmobj2) );
			WriteFloat (  1,ObjectPositionZ(t.tlmobj2) );
		}
		else
		{
			WriteFloat (  1,-1 );
			WriteFloat (  1,-1 );
		}
	}
	CloseFile (  1 );

	//  NOTE; Maybe delete any objects that no longer exist in list?

	//  write out max num file for next time
	t.tnummaxfile_s=t.lightmapper.lmpath_s+"objectnummax.dat";
	t.strwork = ""; t.strwork = t.strwork + "LIGHTMAPPER: Save Lightmap Files: Save new "+t.tnummaxfile_s;
	timestampactivity(0, t.strwork.Get() );
	if (  FileExist(t.tnummaxfile_s.Get()) == 1  )  DeleteAFile (  t.tnummaxfile_s.Get() );
	OpenToWrite (  1,t.tnummaxfile_s.Get() );
	WriteLong (  1,g.glmsceneentitymax );
	CloseFile (  1 );
}

void lm_emptylightmapandttsfilesfolder ( void )
{
	// lightmap - only if lightmaps folder exists
	if ( PathExist(t.lightmapper.lmpath_s.Get()) == 1 ) 
	{
		//  Delete EVERYTHING
		t.tolddir_s=GetDir();
		SetDir ( t.lightmapper.lmpath_s.Get() );
		ChecklistForFiles (  );
		t.strwork = ""; t.strwork = t.strwork + "Deleting "+Str(ChecklistQuantity())+" lightmap files";
		timestampactivity(0, t.strwork.Get() );
		for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
		{
			t.tfile_s=ChecklistString(t.c);
			if (  t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				DeleteAFile ( t.tfile_s.Get() );
			}
		}
		timestampactivity(0,"Deleting complete.");
		SetDir ( t.tolddir_s.Get() );
	}

	#ifdef VRTECH
	// ttsfiles - only if ttsfiles folder exists
	cstr ttsFilesPath_s = g.mysystem.levelBankTestMapAbs_s + "ttsfiles";
	if ( PathExist(ttsFilesPath_s.Get()) == 1 ) 
	{
		t.tolddir_s = GetDir();
		SetDir(ttsFilesPath_s.Get());
		ChecklistForFiles (  );
		t.strwork = ""; t.strwork = t.strwork + "Deleting "+Str(ChecklistQuantity())+" ttsfiles files";
		timestampactivity(0, t.strwork.Get() );
		for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
		{
			t.tfile_s = ChecklistString(t.c);
			if (  t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				DeleteAFile ( t.tfile_s.Get() );
			}
		}
		timestampactivity(0,"Deleting complete.");
		SetDir ( t.tolddir_s.Get() );
	}
	#endif
}

extern int lightmappedterrainoffset;
extern int lightmappedobjectoffset;

void lm_loadscene ( void )
{
	// set reset globals used by occluder
	lightmappedterrainoffset = MAXINT32;
	lightmappedobjectoffset = MAXINT32;
	t.tlmloadsuccess=0;

	// Don't use lightmaps for multiplayer, unless you are the host
	if ( FileExist(t.lightmapper.lmobjectfile_s.Get()) == 0 || (t.game.runasmultiplayer  ==  1 && g.mp.isGameHost  ==  0)  )  return;

	// Remove any old lightmapping scene
	timestampactivity(0,"delete old LMOs");
	lm_deleteall ( );

	// Load lmsceneobj list
	t.tlightmapfilesexist=0;
	g.lightmappedterrainoffsetfinish=g.lightmappedterrainoffset;
	timestampactivity(0,"load LMO list");
	if (  FileExist(t.lightmapper.lmobjectfile_s.Get()) == 1 ) 
	{
		t.tlightmapfilesexist=1;
		OpenToRead (  1,t.lightmapper.lmobjectfile_s.Get() );
		t.tlmsceneobjversion = ReadLong ( 1 );
		if (  t.tlmsceneobjversion >= 1002 ) 
		{
			g.glmsceneentitymax = ReadLong ( 1 );
			Dim (  t.lmsceneobj,g.glmsceneentitymax );
			for ( t.e = 0 ; t.e <= g.glmsceneentitymax ; t.e++ ) {  t.lmsceneobj[t.e].startobj=0 ; t.lmsceneobj[t.e].finishobj=0 ; t.lmsceneobj[t.e].lmvalid=0;  }
			for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
			{
				t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].startobj=t.a;
				t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].finishobj=t.a;
				t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].bankindex=t.a;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].x=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].y=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].z=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].rx=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].ry=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].rz=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].sx=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].sy=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lmsceneobj[t.e].sz=t.a_f;
				t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].lmindex=t.a;
				if (  t.tlmsceneobjversion >= 1003 ) 
				{
					t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].includerotandscale=t.a;
				}
				if (  t.tlmsceneobjversion >= 1004 ) 
				{
					t.a = ReadLong ( 1 ); t.lmsceneobj[t.e].reverseframes=t.a;
				}
				t.lmsceneobj[t.e].lmvalid=3;
				t.lmsceneobj[t.e].needsaving=0;
			}
			g.lightmappedterrainoffset = ReadLong ( 1 );
			g.lightmappedterrainoffsetfinish = ReadLong ( 1 );
			Dim2 ( t.lightmappedterrain,g.lightmappedterrainoffsetfinish-g.lightmappedterrainoffset, 1 );
			for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
			{
				t.a_f = ReadFloat ( 1 ); t.lightmappedterrain[t.tlmobj2-g.lightmappedterrainoffset][0]=t.a_f;
				t.a_f = ReadFloat ( 1 ); t.lightmappedterrain[t.tlmobj2-g.lightmappedterrainoffset][1]=t.a_f;
			}
		}
		CloseFile (  1 );
	}

	// Now mark ANY LM objects as not exist that do not tally with entityelement data
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if ( t.e <= ArrayCount(t.entityelement) ) 
		{
			t.tokay=0;
			if ( t.entityelement[t.e].obj == 0  )  t.tokay = 1;
			if ( t.entityelement[t.e].bankindex != t.lmsceneobj[t.e].bankindex  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].x != (int)t.lmsceneobj[t.e].x  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].y != (int)t.lmsceneobj[t.e].y  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].z != (int)t.lmsceneobj[t.e].z  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].rx != (int)t.lmsceneobj[t.e].rx  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].ry != (int)t.lmsceneobj[t.e].ry  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].rz != (int)t.lmsceneobj[t.e].rz  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].scalex != (int)t.lmsceneobj[t.e].sx  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].scaley != (int)t.lmsceneobj[t.e].sy  )  t.tokay = 1;
			if ( (int)t.entityelement[t.e].scalez != (int)t.lmsceneobj[t.e].sz  )  t.tokay = 1;
			if ( t.tokay == 1 ) 
			{
				t.lmsceneobj[t.e].lmvalid=0;
				t.lmsceneobj[t.e].startobj=0;
				t.lmsceneobj[t.e].finishobj=0;
				if (  t.lmsceneobj[t.e].lmindex>0 ) 
				{
					for ( t.te = 1 ; t.te<=  g.glmsceneentitymax; t.te++ )
					{
						if (  t.te != t.e ) 
						{
							if (  t.lmsceneobj[t.e].lmindex == t.lmsceneobj[t.te].lmindex ) 
							{
								t.lmsceneobj[t.te].lmvalid=0;
							}
						}
					}
				}
			}
		}
		else
		{
			t.lmsceneobj[t.e].lmvalid=0;
		}
	}

	// record current directory
	t.tolddir_s=GetDir();

	// Free hold on any previously stored lightmaps, so the ones
	// loaded next are the newest ones (to be re-used by other LMOs)
	timestampactivity(0,"clear LMO internal textures");
	ClearAnyLightMapInternalTextures (  );

	// Activate auto generation of mipmaps for ALL LM objects
	SetImageAutoMipMap (  1 );

	// Load lightmapped scene objects (and only those not out of date)
	timestampactivity(0,"begin loading LMO files");
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (t.e < t.entityelement.size())
		{
			t.tlmobj = t.lmsceneobj[t.e].startobj;
			t.tentid = t.entityelement[t.e].bankindex;
			if ((t.tlmobj > 0 && t.lmsceneobj[t.e].lmvalid == 3) || (t.tlmobj < 0))
			{
				// hide instance entity
				if (t.e <= ArrayCount(t.entityelement))
				{
					if (t.game.gameisexe == 1)
					{
						// Standalone Game
						t.toldobj = t.entityelement[t.e].obj; if (t.toldobj > 0) { if (ObjectExist(t.toldobj) == 1) { DeleteObject(t.toldobj); } }
						t.entityelement[t.e].obj = 0;
					}
					else
					{
						// Editor
						t.toldobj = t.entityelement[t.e].obj; if (t.toldobj > 0) { if (ObjectExist(t.toldobj) == 1) { SetIgnoreObject(t.toldobj, true); } }
					}
				}

				// load LM object
				t.tobjstart = abs(t.lmsceneobj[t.e].startobj);
				for (t.tlmobj = t.tobjstart; t.tlmobj <= t.lmsceneobj[t.e].finishobj; t.tlmobj++)
				{
					t.tfile_s = t.lightmapper.lmpath_s + "object" + Str(t.tlmobj) + ".dbo";
					if (FileExist(t.tfile_s.Get()) == 1 && t.e <= ArrayCount(t.entityelement))
					{
						if (t.tlmobj > g.lightmappedobjectoffsetfinish)  g.lightmappedobjectoffsetfinish = t.tlmobj;
						if (ObjectExist(t.tlmobj) == 1)  DeleteObject(t.tlmobj);
						t.tentid = t.entityelement[t.e].bankindex;
						t.tdir_s = t.entdir_s + getpath(t.entitybank_s[t.tentid].Get());
						if (PathExist(t.tdir_s.Get()))
						{
							SetDir(t.tdir_s.Get());
						}
						if (t.lightmapper.onlyloadstaticentitiesduringlightmapper > 0 && Rnd(10) == 1)
						{
							t.tdisableLMprogressreading = 1;
							for (t.n = 0; t.n <= 1; t.n++)
							{
								if (t.lightmapper.onlyloadstaticentitiesduringlightmapper == 2)  CLS(Rgb(102, 102, 153));
								t.strwork = ""; t.strwork = t.strwork + "Loading Lightmap Object " + Str(t.e);
								t.tonscreenprompt_s = t.strwork.Get();
								if (t.hardwareinfoglobals.noterrain == 0) terrain_update();
								lm_onscreenprompt(); Sync();
							}
							t.tdisableLMprogressreading = 0;
						}
						LoadObject(t.tfile_s.Get(), t.tlmobj);
						if (t.lmsceneobj[t.e].reverseframes == 1)
						{
							ReverseObjectFrames(t.tlmobj);
						}
						SetDir(t.tolddir_s.Get());

						//Check if we need illum.
						bool useillum = false;
						sObject* pObject = g_ObjectList[t.tlmobj];
						sMesh* pMesh;
						for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++) {
							pMesh = pObject->ppMeshList[iMesh];
							if (pMesh) {
								DWORD dwTextureCount = pMesh->dwTextureCount;
								for (DWORD t = 0; t < dwTextureCount; t++) {
									sTexture* pTexture = &(pMesh->pTextures[t]);
									if (pTexture) {

										if (pestrcasestr(&pTexture->pName[0], "_illumination.") || pestrcasestr(&pTexture->pName[0], "_emissive.") || pestrcasestr(&pTexture->pName[0], "_i.dds")) {
											//timestampactivity(0, "LM use illum");
											useillum = true;

											//debug
											//char dtmp[2048];
											//sprintf(dtmp, "LM ILLUM m: %d , t: %d , name: %s", (int)iMesh, (int)t, pTexture->pName);
											//timestampactivity(0, dtmp);

											break;
										}
									}
								}
							}
							if (useillum)
								break;
						}


						// if white out, replace diffuse with white texture
						if (g.gdividetexturesize == 0)
						{
							t.ttexuseid = loadinternaltextureex("effectbank\\reloaded\\media\\white_D.dds", 1, 0);
							TextureObject(t.tlmobj, 0, t.ttexuseid);
						}
						else
						{
							// determine if in PBR mode, and apply lightmap PBR shader
							// load PBR lightmap shader if not exists
							if (GetEffectExist(g.lightmappbreffect) == 0)
							{
								LPSTR pLightmapPBREffect = "effectbank\\reloaded\\apbr_lightmapped.fx";
								//LPSTR pLightmapPBREffect = "effectbank\\reloaded\\static_basic.fx";
								LoadEffect(pLightmapPBREffect, g.lightmappbreffect, 0);
							}
							if (GetEffectExist(g.lightmappbreffectillum) == 0)
							{
								LPSTR pLightmapPBREffect = "effectbank\\reloaded\\apbr_lightmapped_illum.fx";
								LoadEffect(pLightmapPBREffect, g.lightmappbreffectillum, 0);
							}

							// apply lightmap PBR shader to lightmapped object
							if (useillum)
							{
								SetObjectEffect(t.tlmobj, g.lightmappbreffectillum);
							}
							else
							{
								SetObjectEffect(t.tlmobj, g.lightmappbreffect);
							}
						}

						// apply textures from doner entity parent only if not been consolidated
						if (t.lmsceneobj[t.e].startobj > 0)
						{
							// textures common to PBR and DNS
							if (t.entityprofile[t.tentid].texdid > 0) TextureObject(t.tlmobj, 0, t.entityprofile[t.tentid].texdid);
							if (t.entityprofile[t.tentid].texnid > 0) TextureObject(t.tlmobj, 2, t.entityprofile[t.tentid].texnid);
							if (t.entityprofile[t.tentid].texsid > 0) TextureObject(t.tlmobj, 3, t.entityprofile[t.tentid].texsid);

							// which shader style (PBR or DNS)
							if (g.gpbroverride == 1)
							{
								// PBR textures
								if (g.memskipibr == 0) TextureObject(t.tlmobj, 8, t.entityprofiletexibrid);
								TextureObject(t.tlmobj, 7, t.entityprofile[t.tentid].texlid);
								TextureObject(t.tlmobj, 4, t.entityprofile[t.tentid].texgid);
								TextureObject(t.tlmobj, 5, t.entityprofile[t.tentid].texhid);
							}
							else
							{
								// non-PBR
								TextureObject(t.tlmobj, 4, t.terrain.imagestartindex);
								TextureObject(t.tlmobj, 5, g.postprocessimageoffset + 5);
							}

							// Apply all textures to REMAINING entity parent object (V C I)
							// TextureObject ( t.entobj, 6, t.entityprofile[t.entid].texiid );
							if (t.entityprofile[t.tentid].texiid > 0)
							{
								TextureObject(t.tlmobj, 6, t.entityprofile[t.tentid].texiid);
							}
						}

						// batching incorporates rotation and scale (not position as we want to camera cull them)
						SetObjectMask(t.tlmobj, 1);
						if (t.e <= ArrayCount(t.entityelement))
						{
							PositionObject(t.tlmobj, t.entityelement[t.e].x, t.entityelement[t.e].y, t.entityelement[t.e].z);
							if (t.lmsceneobj[t.e].includerotandscale == 0)
							{
								RotateObject(t.tlmobj, 0, 0, 0);
							}
							else
							{
								RotateObject(t.tlmobj, t.entityelement[t.e].rx, t.entityelement[t.e].ry, t.entityelement[t.e].rz);
								ScaleObject(t.tlmobj, 100 + t.entityelement[t.e].scalex, 100 + t.entityelement[t.e].scaley, 100 + t.entityelement[t.e].scalez);
							}
							// ensure batched object shares collision property of parent entity
							if (t.entityelement[t.e].staticflag == 1)
							{
								t.tentid = t.entityelement[t.e].bankindex;
								if (t.entityprofile[t.tentid].canseethrough == 1)
								{
									SetObjectCollisionProperty(t.tlmobj, 1);
								}
							}
							if (t.entityprofile[t.tentid].ischaracter == 0)
							{
								if (t.entityprofile[t.tentid].collisionmode == 11)
								{
									SetObjectCollisionProperty(t.tlmobj, 1);
								}
							}
						}
						// assign cull and transparency modes from parent settings
						t.entid = 0; if (t.e <= ArrayCount(t.entityelement))  t.entid = t.entityelement[t.e].bankindex;
						if (t.entid > 0)
						{
							// apply certain settings lost in the conversion
							#ifdef VRTECH
							if (t.entityprofile[t.entid].cullmode >= 0)
							{
								if (t.entityprofile[t.entid].cullmode != 0)
								{
									SetObjectCull(t.tlmobj, 0);
								}
								else
								{
									SetObjectCull(t.tlmobj, 1);
								}
							}
							if (t.entityelement[t.e].eleprof.transparency >= 0)
							{
								SetObjectTransparency(t.tlmobj, t.entityelement[t.e].eleprof.transparency);
							}
							#else
							SetObjectTransparency(t.tlmobj, t.entityelement[t.e].eleprof.transparency);
							if (t.entityprofile[t.entid].cullmode != 0)
							{
								SetObjectCull(t.tlmobj, 0);
							}
							else
							{
								SetObjectCull(t.tlmobj, 1);
							}
							#endif
						}
					}
				}
			}
		}
	}

	// Load in terrain shadow objects
	timestampactivity(0,"loading LMO terrain files");
	t.liftshadowstositontopofterrain_f = 0.1f;
	if ( g.lightmappedterrainoffsetfinish>g.lightmappedterrainoffset ) 
	{
		for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
		{
			t.tfile_s=t.lightmapper.lmpath_s+"object"+Str(t.tlmobj2)+".dbo";
			if ( ObjectExist(t.tlmobj2) == 1  )  DeleteObject (  t.tlmobj2 );
			if ( FileExist(t.tfile_s.Get()) == 1 ) 
			{
				if ( g.iLightmappingExcludeTerrain == 0 )
				{
					LoadObject (  t.tfile_s.Get(),t.tlmobj2 );
					SetObjectMask (  t.tlmobj2, 1 );
					SetObjectCollisionProperty (  t.tlmobj2,1 );
					// NOTE; Can optimize for shader here by making these objects world position based!
					PositionObject (  t.tlmobj2,t.lightmappedterrain[t.tlmobj2-g.lightmappedterrainoffset][0],t.liftshadowstositontopofterrain_f,t.lightmappedterrain[t.tlmobj2-g.lightmappedterrainoffset][1] );
					SetObjectTransparency (  t.tlmobj2, 6 );
					DisableObjectZWrite ( t.tlmobj2 );
					lm_zbias ( );
				}
				else
				{
					// dummy object eventually hidden from draw
					MakeObjectPlane ( t.tlmobj2, 0, 0 );
					SetObjectMask ( t.tlmobj2, 0 );
					HideObject ( t.tlmobj2 );
				}
			}
		}
	}
	UnDim (  t.lightmappedterrain );

	// finished generating mipmaps
	SetImageAutoMipMap (  0 );

	// If there has been any LM loading, use PRE-BAKE technique
	if ( t.tlightmapfilesexist == 1 ) 
	{
		t.visuals.shaderlevels.lighting=1;
		t.visuals.refreshshaders=1;
	}

	// confirm the load was successful
	t.tlmloadsuccess=1;

	// add lightmapped objects as occludees
	for ( t.tlmobj = g.lightmappedobjectoffset; t.tlmobj<= g.lightmappedobjectoffsetfinish; t.tlmobj++ )
	{
		if ( ObjectExist(t.tlmobj) == 1 ) 
		{
			CPU3DAddOccludee ( t.tlmobj , false );
			if ( ( ObjectSizeX(t.tlmobj,1)>MINOCCLUDERSIZE && ObjectSizeY(t.tlmobj,1)>MINOCCLUDERSIZE ) || ( ObjectSizeZ(t.tlmobj,1)>MINOCCLUDERSIZE && ObjectSizeY(t.tlmobj,1)>MINOCCLUDERSIZE ) ) 
				CPU3DAddOccluder (  t.tlmobj );
		}
	}

	// add terrain lightmap objects as occludees
	if ( g.iLightmappingExcludeTerrain == 0 )
	{
		if ( g.lightmappedterrainoffset != -1 ) 
		{
			for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
			{
				if ( ObjectExist(t.tlmobj2) == 1 ) 
				{
					CPU3DAddOccludee ( t.tlmobj2 , false );
					if ( (ObjectSizeX(t.tlmobj2,1)>MINOCCLUDERSIZE && ObjectSizeY(t.tlmobj2,1)>MINOCCLUDERSIZE) || (ObjectSizeZ(t.tlmobj2,1)>MINOCCLUDERSIZE && ObjectSizeY(t.tlmobj2,1)>MINOCCLUDERSIZE) ) 
						CPU3DAddOccluder (  t.tlmobj2 );
				}
			}
		}
	}

	// set globals used by the occluder
	lightmappedterrainoffset = g.lightmappedterrainoffset;
	lightmappedobjectoffset = g.lightmappedobjectoffset;
}

void lm_preplmobj ( void )
{
	//  takes tlmobj,e,tentid
	if (  t.e <= ArrayCount(t.entityelement) ) 
	{
		PositionObject (  t.tlmobj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
		RotateObject (  t.tlmobj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
		ScaleObject (  t.tlmobj,100+t.entityelement[t.e].scalex,100+t.entityelement[t.e].scaley,100+t.entityelement[t.e].scalez );
	}
	SetObjectMask (  t.tlmobj, 1 );
}

void lm_handleshaders ( void )
{
	//  Apply effect to new lightmapped objects
	for ( t.e = 1 ; t.e<=  g.glmsceneentitymax; t.e++ )
	{
		if (  t.lmsceneobj[t.e].lmvalid == 3 ) 
		{
			// 081215 - also allow minus values (indicating special case LM object)
			if (  t.lmsceneobj[t.e].startobj != 0 ) // > 0 ) 
			{
				t.entid = 0 ; if (  t.e <= ArrayCount(t.entityelement)  )  t.entid = t.entityelement[t.e].bankindex;
				for ( t.tlmobj = abs(t.lmsceneobj[t.e].startobj); t.tlmobj<= t.lmsceneobj[t.e].finishobj; t.tlmobj++ )
				{
					if (  ObjectExist(t.tlmobj) == 1 ) 
					{
						// 100718 - important no shaders are applied to LM entity objects (done when loading in game engine)
						SetObjectEffect ( t.tlmobj, 0 );
						SetObjectWireframe ( t.tlmobj,0 );
						if (t.entid > 0)
						{
							// apply certain settings lost in the conversion
							#ifdef VRTECH
							if (t.entityprofile[t.entid].cullmode >= 0)
							{
								if (t.entityprofile[t.entid].cullmode != 0)
								{
									SetObjectCull(t.tlmobj, 0);
								}
								else
								{
									SetObjectCull(t.tlmobj, 1);
								}
							}
							if (t.entityelement[t.e].eleprof.transparency >= 0)
							{
								SetObjectTransparency(t.tlmobj, t.entityelement[t.e].eleprof.transparency);
							}
							#else
							SetObjectTransparency (  t.tlmobj,t.entityelement[t.e].eleprof.transparency );
							if (  t.entityprofile[t.entid].cullmode != 0 ) 
							{
								SetObjectCull (  t.tlmobj,0 );
							}
							else
							{
								SetObjectCull (  t.tlmobj,1 );
							}
							#endif
						}
					}
				}
			}
			t.lmsceneobj[t.e].lmvalid=1;
		}
	}

	//  apply super simple shadow shader
	for ( t.tlmobj2 = g.lightmappedterrainoffset ; t.tlmobj2<=  g.lightmappedterrainoffsetfinish; t.tlmobj2++ )
	{
		if (  ObjectExist(t.tlmobj2) == 1 ) 
		{
			SetObjectEffect (  t.tlmobj2,g.staticshadowlightmapeffectoffset );
			ShowObject (  t.tlmobj2 );
		}
	}
}

int findlightmaptexturefilenameindex ( char* file_s )
{
	int treturnlightmapindex = 0;
	int tfoundlightmappath = 0;
	int filesize = 0;
	int mbi = 0;
	cstr n_s =  "";
	int b = 0;
	int c = 0;
	treturnlightmapindex=-1;
	if (  FileExist(file_s) == 1 ) 
	{
	filesize=FileSize(file_s);
	mbi=255;
	OpenToRead (  11,file_s );
	MakeMemblockFromFile (  mbi,11 );
	CloseFile (  11 );
	for ( b = 0 ; b<=  filesize-1; b++ )
	{
		tfoundlightmappath=0;
		if (  ReadMemblockByte(mbi,b+0) == Asc("l") ) 
		{
			if (  ReadMemblockByte(mbi,b+1) == Asc("i") ) 
			{
				if (  ReadMemblockByte(mbi,b+2) == Asc("g") ) 
				{
					if (  ReadMemblockByte(mbi,b+3) == Asc("h") ) 
					{
						if (  ReadMemblockByte(mbi,b+4) == Asc("t") ) 
						{
							if (  ReadMemblockByte(mbi,b+5) == Asc("m") ) 
							{
								if (  ReadMemblockByte(mbi,b+6) == Asc("a") ) 
								{
									if (  ReadMemblockByte(mbi,b+7) == Asc("p") ) 
									{
										if (  ReadMemblockByte(mbi,b+8) == Asc("s") ) 
										{
											if (  ReadMemblockByte(mbi,b+9) == Asc("\\") ) 
											{
												tfoundlightmappath=1;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			if (  tfoundlightmappath == 1 ) 
			{
				n_s="";
				for ( c = b+10 ; c<=  b+18; c++ )
				{
					if (  ReadMemblockByte(mbi,c) == Asc(".") ) 
					{
						break;
					}
					else
					{
						n_s=n_s+Chr(ReadMemblockByte(mbi,c));
					}
				}
				//  found the numeric of the lightmap texture
				treturnlightmapindex=ValF(n_s.Get());
				b=filesize ; break;
			}
		}
	}
	DeleteMemblock (  mbi );
	}
//endfunction treturnlightmapindex
	return treturnlightmapindex;
}

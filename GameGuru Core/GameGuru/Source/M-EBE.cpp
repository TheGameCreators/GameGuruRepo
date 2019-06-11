//----------------------------------------------------
//--- GAMEGURU - M-EBE
//----------------------------------------------------

#include "gameguru.h"

// EBE Globals (8MB)
#define CUBEAREASIZE 200
#define CUBEMAXMESH 8000
unsigned char pCubes[CUBEAREASIZE][CUBEAREASIZE][CUBEAREASIZE];
unsigned char pTemp[CUBEAREASIZE][CUBEAREASIZE][CUBEAREASIZE];
DWORD pVertCountStore[CUBEMAXMESH];
bool pbTriggerDrawBufferCreation[CUBEMAXMESH];
short pMasterGridMeshRef[20][20][20][2];
struct sMyColBox 
{
	int x1;
	int x2;
	int y1;
	int y2;
	int z1;
	int z2;
	int iMaterialIndex;
	unsigned char cCubeTexIndex;
};
#define CUBECOLBOXMAX 2000
sMyColBox pMyColBox[CUBECOLBOXMAX];
#define EBETEXPANELSPRMAX 6

// EBE Local Structure
struct evebuildtexprofiletype
{
	int iWidth;
	int iHeight;
	int iMaterialRef[64];
	cStr sTextureFile[64];
};
struct evebuildpatterntype
{
	int iWidth;
	int iHeight;
	int iDepth;
	int iPreserveMode;
	int iWidthOffset;
	int iDepthOffset;
	cStr pPRow[20][20];
};
struct ebebuildtype
{
	int iToolObj;
	int iBuildObj;
	int iCasterObj;
	int iCurrentGridLayer;
	int iLocalKeyPressed;
	int iCursorGridPosX;
	int iCursorGridPosZ;
	int iCursorRotation;
	int iCurrentTexture;
	evebuildpatterntype OriginalPattern;
	evebuildpatterntype Pattern;
	evebuildtexprofiletype TXP;
	int iTexturePanelSprite[6];
	int iTexturePanelImg[6];
	int iTexturePanelX;
	int iTexturePanelY;
	int iTexturePanelWidth;
	int iTexturePanelHeight;
	int iTexturePanelHighSprite;
	int iTexturePanelHighImg;
	int iEBEHelpSpr;
	int iEBEHelpImg;
	int iEBETexHelpSpr;
	int iEBETexHelpImg;
	int iMatSpr[16];
	int iMatImg[19];
	bool bCustomiseTexture;
	int iTexPlateImage;
	int iRefreshBuild;
	int iDeterminedAxisDir;
	int iCursorGridPosLastGoodX;
	int iCursorGridPosLastGoodZ;
};
ebebuildtype ebebuild;

// Undo/Redo Buffers
DWORD g_dwUndoBufferCount = 0;
DWORD* g_pUndoBufferPtr = NULL;
DWORD g_dwRedoBufferCount = 0;
DWORD* g_pRedoBufferPtr = NULL;

void ebe_init ( int BuildObj, int iEntID )
{
	// Basic inits
	ebebuild.bCustomiseTexture = false;
	ebebuild.iRefreshBuild = 0;

	// Create resources for EBE editing
	pastebitmapfontcenterwithboxout("PREPARING BUILDING EDITOR SYSTEM",GetChildWindowWidth()/2,40,1,255); Sync();

	// Create a simple floor
	unsigned char cTexIndex = 0;

	// Create texture selection panel
	ebebuild.iTexPlateImage = 0;
	ebebuild.iTexturePanelX = GetChildWindowWidth()-210;
	ebebuild.iTexturePanelY = GetChildWindowHeight()-210;
	ebebuild.iTexturePanelWidth = 200;
	ebebuild.iTexturePanelHeight = 200;
	for ( int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++ )
	{
		LPSTR pTexImg = "";
		int iX = ebebuild.iTexturePanelX;
		int iY = ebebuild.iTexturePanelY;
		int iWidth = ebebuild.iTexturePanelWidth;
		int iHeight = ebebuild.iTexturePanelHeight;
		if ( iTex==0 ) { pTexImg = "TexHUD-F.png"; iX -= 10; iY -= 10; iWidth += 20; iHeight += 20; }
		if ( iTex==1 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY -= 10; iWidth += 20; iHeight = 1; }
		if ( iTex==2 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY += 209; iWidth += 20; iHeight = 1; }
		if ( iTex==3 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY -= 10; iWidth = 1; iHeight += 20; }
		if ( iTex==4 ) { pTexImg = "TexHUD-L.png"; iX += 209; iY -= 10; iWidth = 1; iHeight += 20; }
		if ( iTex==5 ) { pTexImg = "textures_D.dds"; }
		ebebuild.iTexturePanelSprite[iTex] = g.ebeinterfacesprite + 31 + iTex;
		ebebuild.iTexturePanelImg[iTex] = loadinternalimage(cstr(cstr("ebebank\\default\\")+cstr(pTexImg)).Get());
		Sprite ( ebebuild.iTexturePanelSprite[iTex], iX, iY, ebebuild.iTexturePanelImg[iTex] );
		SizeSprite ( ebebuild.iTexturePanelSprite[iTex], iWidth, iHeight );
		if ( iTex==5 ) ebebuild.iTexPlateImage = ebebuild.iTexturePanelImg[iTex];
	}

	// Texture highlighter
	ebebuild.iTexturePanelHighSprite = g.ebeinterfacesprite + 0;
	ebebuild.iTexturePanelHighImg = loadinternalimage("ebebank\\default\\TextureHighlighter.dds");
	Sprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelX, ebebuild.iTexturePanelY, ebebuild.iTexturePanelHighImg );
	SizeSprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelWidth/4, ebebuild.iTexturePanelHeight/4 );

	// Help Dialog Shortcut Keys
	ebebuild.iEBEHelpSpr = g.ebeinterfacesprite + 1;
	ebebuild.iEBEHelpImg = loadinternalimage("languagebank\\english\\artwork\\ebe-help.png");
	Sprite ( ebebuild.iEBEHelpSpr, ebebuild.iTexturePanelX - ImageWidth(ebebuild.iEBEHelpImg) - 10, ebebuild.iTexturePanelY + 210 - ImageHeight(ebebuild.iEBEHelpImg), ebebuild.iEBEHelpImg );

	// Help Dialog Shortcut Keys
	ebebuild.iEBETexHelpSpr = g.ebeinterfacesprite + 2;
	ebebuild.iEBETexHelpImg = loadinternalimage("languagebank\\english\\artwork\\ebe-texturehelp.png");
	Sprite ( ebebuild.iEBETexHelpSpr, ebebuild.iTexturePanelX - 10, ebebuild.iTexturePanelY - 10 - ImageHeight(ebebuild.iEBETexHelpImg), ebebuild.iEBETexHelpImg );

	// Load TXP default file
	ebe_loadtxp(cstr(cstr("ebebank\\default\\")+cstr("textures_profile.txp")).Get());

	// Material index overlays and sprites to place over texture highlighter selection
	for ( int n = 0; n <= 18; n++ )
	{
		cstr sMatIconFile = "";
		if ( n == 0 ) sMatIconFile = "ebe-material-0-gen.png";
		if ( n == 1 ) sMatIconFile = "ebe-material-1-sto.png";
		if ( n == 2 ) sMatIconFile = "ebe-material-2-met.png";
		if ( n == 3 ) sMatIconFile = "ebe-material-3-woo.png";
		if ( n == 4 ) sMatIconFile = "ebe-material-4.png";
		if ( n == 5 ) sMatIconFile = "ebe-material-5.png";
		if ( n == 6 ) sMatIconFile = "ebe-material-6.png";
		if ( n == 7 ) sMatIconFile = "ebe-material-7.png";
		if ( n == 8 ) sMatIconFile = "ebe-material-8.png";
		if ( n == 9 ) sMatIconFile = "ebe-material-9.png";
		if ( n == 10 ) sMatIconFile = "ebe-material-10.png";
		if ( n == 11 ) sMatIconFile = "ebe-material-11.png";
		if ( n == 12 ) sMatIconFile = "ebe-material-12.png";
		if ( n == 13 ) sMatIconFile = "ebe-material-13.png";
		if ( n == 14 ) sMatIconFile = "ebe-material-14.png";
		if ( n == 15 ) sMatIconFile = "ebe-material-15.png";
		if ( n == 16 ) sMatIconFile = "ebe-material-16.png";
		if ( n == 17 ) sMatIconFile = "ebe-material-17.png";
		if ( n == 18 ) sMatIconFile = "ebe-material-18.png";
		ebebuild.iMatImg[n] = loadinternalimage(cstr(cstr("languagebank\\english\\artwork\\")+sMatIconFile).Get());
	}
	int n = 0;
	for ( int y = 0; y < 4; y++ )
	{
		for ( int x = 0; x < 4; x++ )
		{
			ebebuild.iMatSpr[n] = g.ebeinterfacesprite + 11 + n;
			Sprite ( ebebuild.iMatSpr[n], ebebuild.iTexturePanelX + 36 + (x*50), ebebuild.iTexturePanelY + 36 + (y*50), ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[n]] );
			SizeSprite ( ebebuild.iMatSpr[n], 13, 13 );
			n++;
		}
	}

	// Create Building Site Tool Object
	int iObj = g.ebeobjectbankoffset + 0;
	MakeObjectPlane ( iObj, 100, 100 );
	MakeMeshFromObject ( g.meshgeneralwork, iObj );
	DeleteObject ( iObj );
	MakeObjectCube ( iObj, 5.0f );
	MakeMeshFromObject ( g.meshebe1, iObj );
	DeleteObject ( iObj );
	MakeObjectBox ( iObj, 1000, 0.1f, 1000 );
	MakeMeshFromObject ( g.meshebe, iObj );

	// Grid Limbs (0-base grid, 1-floating grid for current edit layer)
	int iGridImg = loadinternaltexture("ebebank\\default\\GridBox.dds");
	int iFloatImg = loadinternaltexture("ebebank\\default\\FloatBox.dds");
	int iCursorImg = loadinternaltexture("ebebank\\default\\CursorBox.dds");	
	int iLimbIndex = 0;
	OffsetLimb ( iObj, iLimbIndex, 500, 0.05, 500 );
	iLimbIndex = 1;
	AddLimb ( iObj, iLimbIndex, g.meshebe );
	OffsetLimb ( iObj, iLimbIndex, 500, 0.05, 500 );

	// now scale present meshes so grid creates a mirror effect of UVs for good textured grid
	ScaleObjectTexture ( iObj, 200, 200 );

	// Cursor Limb (2-can be used to show current mouse cursor within grid)
	iLimbIndex = 2;
	AddLimb ( iObj, iLimbIndex, g.meshebe1 );
	OffsetLimb ( iObj, iLimbIndex, 2.5f, 2.5f, 2.5f );

	// Replace default texture with grid textures
	TextureLimbStage ( iObj, 0, 0, iGridImg );
	TextureLimbStage ( iObj, 1, 0, iFloatImg );
	TextureLimbStage ( iObj, 2, 0, iCursorImg );
	SetObjectTransparency ( iObj, 6 );
	DisableObjectZWrite ( iObj );

	// Apply entity shader
	int iToolEffectIndex = loadinternaleffect("effectbank\\reloaded\\ebe_basic.fx");
	SetObjectEffect ( iObj, iToolEffectIndex );
	SetObjectMask ( iObj, 0x1 );

	// Create invisible obj to cast for object selection detection
	ebebuild.iCasterObj = g.ebeobjectbankoffset + 1;
	MakeObjectCube ( ebebuild.iCasterObj, 1 );
	SetObjectMask ( ebebuild.iCasterObj, 0x1 );
	HideObject ( ebebuild.iCasterObj );

	// New template has marker in limb 2, regular EBE structure has it in zero
	int iLimbWithMarker = 0;
	if ( stricmp ( Right ( t.entitybank_s[iEntID].Get(), 21), "_builder\\New Site.fpe" ) == NULL )
		iLimbWithMarker = 2;

	// Create ebe marker mesh from first 
	MakeMeshFromLimb ( g.meshebemarker, BuildObj, iLimbWithMarker );

	// mark EBE has intialised
	t.ebe.active = 1;
}

void ebe_init_newbuild ( int iBuildObj, int entid )
{
	// ensure EBE system is initialised
	if ( t.ebe.active == 0 ) ebe_init(iBuildObj,entid);

	// load TXP profile from entity data
	// and update visuals for the material ref icons
	if ( t.entityprofile[entid].ebe.dwMatRefCount > 0 )
	{
		int ncount = t.entityprofile[entid].ebe.dwMatRefCount;
		for ( int n = 0; n < ncount; n++ )
		{
			ebebuild.TXP.iMaterialRef[n] = t.entityprofile[entid].ebe.iMatRef[n];
			Sprite ( ebebuild.iMatSpr[n], SpriteX(ebebuild.iMatSpr[n]), SpriteY(ebebuild.iMatSpr[n]), ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[n]] );
		}
	}

	// detect if texture ref profile differs from current one
	bool bTexturePlateDifferent = false;
	if ( t.entityprofile[entid].ebe.dwTexRefCount > 0 )
	{
		int ncount = t.entityprofile[entid].ebe.dwTexRefCount;
		for ( int n = 0; n < ncount; n++ )
		{
			if ( stricmp ( ebebuild.TXP.sTextureFile[n].Get(), t.entityprofile[entid].ebe.pTexRef[n] ) != NULL )
			{
				bTexturePlateDifferent = true;
			}
		}
	}
	if ( bTexturePlateDifferent == true )
	{
		// update editor texture plate to match incoming EBE structure
		// by loading it direct from the EBE special long named textures and also the TXP file
		int ncount = t.entityprofile[entid].ebe.dwTexRefCount;
		for ( int n = 0; n < ncount; n++ )
			ebebuild.TXP.sTextureFile[n] = t.entityprofile[entid].ebe.pTexRef[n];

		// Also save snapshot of latest textures_DNS
		cstr sUniqueFilename = ebe_constructlongTXPname("_D.dds");
		cstr sLongFilename = cstr("ebebank\\default\\") + sUniqueFilename;
		cstr tRawPathAndFile = cstr(Left(sLongFilename.Get(),strlen(sLongFilename.Get())-6));
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		if ( FileExist(sDDSFile.Get()) == 0 ) 
		{
			// cache in ebebank\default deleted, so copy from
			cstr tSourceRaw = g.mysystem.levelBankTestMap_s + sUniqueFilename; //cstr("levelbank\\testmap\\") + sUniqueFilename;
			tSourceRaw = cstr(Left(tSourceRaw.Get(),strlen(tSourceRaw.Get())-6));
			cstr tDDSSource = tSourceRaw + "_D.dds";
			if ( FileExist(tDDSSource.Get()) == 0 ) 
			{
				// not in ebebank or in levelbank, try original saved ebe entity (though a real entity should never get here - only their entitybank copies)
				tSourceRaw = cstr("entitybank\\")+cstr(t.entitybank_s[entid]) + "\\" + sUniqueFilename;
				tDDSSource = tSourceRaw + "_D.dds";
			}
			sDDSFile = tRawPathAndFile + cstr("_D.dds");
			CopyFile ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			tDDSSource = tSourceRaw + "_N.dds";
			sDDSFile = tRawPathAndFile + cstr("_N.dds");
			CopyFile ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			tDDSSource = tSourceRaw + "_S.dds";
			sDDSFile = tRawPathAndFile + cstr("_S.dds");
			CopyFile ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			sDDSFile = tRawPathAndFile + cstr("_D.dds");
		}
		if ( FileExist(sDDSFile.Get()) == 1 ) 
		{
			cstr tDDSFilename = "ebebank\\default\\textures_D.dds";
			if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteAFile ( tDDSFilename.Get() );
			CopyFile ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
			sDDSFile = tRawPathAndFile + cstr("_N.dds");
			tDDSFilename = "ebebank\\default\\textures_N.dds";
			if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteAFile ( tDDSFilename.Get() );
			CopyFile ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
			sDDSFile = tRawPathAndFile + cstr("_S.dds");
			tDDSFilename = "ebebank\\default\\textures_S.dds";
			if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteAFile ( tDDSFilename.Get() );
			CopyFile ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
		}

		// and the TXP file too (260317 - generate, as copy may not exist)
		//sLongFilename = cstr("ebebank\\default\\") + ebe_constructlongTXPname(".txp");
		//cstr sTextureTXPFile = "ebebank\\default\\textures_profile.txp";
		//if ( FileExist(sTextureTXPFile.Get()) == 1 ) DeleteAFile ( sTextureTXPFile.Get() );
		//CopyFile ( sLongFilename.Get(), sTextureTXPFile.Get(), FALSE );
		ebe_savetxp(cstr(cstr("ebebank\\default\\")+cstr("textures_profile.txp")).Get());

		// and update EBE editor texture plate image for selector
		LoadImage ( "ebebank\\default\\textures_D.dds", ebebuild.iTexPlateImage );
		Sprite ( ebebuild.iTexturePanelSprite[5], SpriteX(ebebuild.iTexturePanelSprite[5]), SpriteY(ebebuild.iTexturePanelSprite[5]), ebebuild.iTexPlateImage );
	}

	// shift grid object away so don't see last incarnation of it
	if ( ebebuild.iToolObj > 0 )
		if ( ObjectExist ( ebebuild.iToolObj ) == 1 )
			PositionObject ( ebebuild.iToolObj, -999999, -999999, -999999 );

	// Wipe cube building site
	memset ( pCubes, 0, sizeof(pCubes) );

	// Create Building Site Object from original entity element OBJ mesh
	float fRX = ObjectAngleX(iBuildObj);
	float fRY = ObjectAngleY(iBuildObj);
	float fRZ = ObjectAngleZ(iBuildObj);
	ebebuild.iBuildObj = iBuildObj;
	DeleteObject ( iBuildObj );

	// Start with the marker mesh
	MakeObject ( iBuildObj, g.meshebemarker, 0 );
	RotateObject ( iBuildObj, fRX, fRY, fRZ );
	SetSphereRadius ( iBuildObj, 0 );

	// Assign slot indexes for 3D grid volume
	int iSlotIndex = 1;
	for ( int y = 0; y <= CUBEAREASIZE-10; y+=10 )
	{
		for ( int z = 0; z <= CUBEAREASIZE-10; z+=10 )
		{
			for ( int x = 0; x <= CUBEAREASIZE-10; x+=10 )
			{
				pMasterGridMeshRef[x/10][y/10][z/10][0] = iSlotIndex;
				pMasterGridMeshRef[x/10][y/10][z/10][1] = 0;
				pVertCountStore[iSlotIndex] = 0;
				iSlotIndex++;
			}
		}
	}

	// Apply texture plate
	int iTexD = loadinternaltexture("ebebank\\default\\textures_D.dds");
	int iTexN = loadinternaltexture("ebebank\\default\\textures_N.dds");
	int iTexS = loadinternaltexture("ebebank\\default\\textures_S.dds");
	//t.entityprofile[entid].texdid = iTexD;
	//t.entityprofile[entid].texnid = iTexN;
	//t.entityprofile[entid].texsid = iTexS;
	//int iTexD = t.entityprofile[entid].texdid;
	//int iTexN = t.entityprofile[entid].texnid;
	//int iTexS = t.entityprofile[entid].texsid;
	//if ( iTexD > 0 && ImageExist ( iTexD ) == 1 ) DeleteImage ( iTexD );
	//if ( iTexN > 0 && ImageExist ( iTexN ) == 1 ) DeleteImage ( iTexN );
	//if ( iTexS > 0 && ImageExist ( iTexS ) == 1 ) DeleteImage ( iTexS );
	LoadImage ( "ebebank\\default\\textures_D.dds", iTexD, 0, g.gdividetexturesize);
	LoadImage ( "ebebank\\default\\textures_N.dds", iTexN, 0, g.gdividetexturesize);
	LoadImage ( "ebebank\\default\\textures_S.dds", iTexS, 0, g.gdividetexturesize);
	TextureObject ( iBuildObj, 0, iTexD );
	TextureObject ( iBuildObj, 1, loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds") );
	TextureObject ( iBuildObj, 2, iTexN );
	TextureObject ( iBuildObj, 3, iTexS );
	TextureObject ( iBuildObj, 4, t.terrain.imagestartindex );
	TextureObject ( iBuildObj, 5, g.postprocessimageoffset+5 );
	TextureObject ( iBuildObj, 6, loadinternaltexture("effectbank\\reloaded\\media\\blank_I.dds") );
	SetObjectTransparency ( iBuildObj, 0 );

	// unpack data into cube data
	DWORD dwRLESize = t.entityprofile[entid].ebe.dwRLESize;
	if ( dwRLESize > 0 )
	{
		DWORD* pRLEData = t.entityprofile[entid].ebe.pRLEData;
		ebe_unpacksite ( dwRLESize, pRLEData );
	}

	// Refresh mesh with cube construction
	ebe_refreshmesh ( iBuildObj, 0, 0, 0, CUBEAREASIZE-10, CUBEAREASIZE-11, CUBEAREASIZE-10 );

	// aply shader effect (now done inside refresh)
	int iEBEEffectIndex = loadinternaleffect("effectbank\\reloaded\\ebe_basic.fx");
	SetLimbEffect ( iBuildObj, 0, iEBEEffectIndex );

	// Only render EBE to main camera and shadow camera
	SetObjectMask ( iBuildObj, 1+(1<<31) );

	// In case new 'shader' associated with new entity, refresh please
	visuals_justshaderupdate ( );

	// the EBE construction build object
	sObject* pObject = g_ObjectList [ iBuildObj ];

	// Ensures col not calculated in main sync render loop
	UpdateColCenter ( pObject );

	// EBE construction object cannot have collision active (so raycast skips its half-baked meshes)
	SetColOff ( pObject );
}

void ebe_updateparent ( int entityelementindex )
{
	// early exits
	if ( entityelementindex == 0 ) return;

	// before can delete a parent, must delete all instance objects to it
	int iEntityBankID = t.entityelement[entityelementindex].bankindex;
	t.sourceobj = g.entitybankoffset + iEntityBankID;
	if ( ObjectExist ( t.sourceobj ) == 1 )
	{
		// go through all entity elements to look for instances of parent
		for ( int te = 1 ; te <= g.entityelementlist; te++ )
		{
			if ( te != entityelementindex )
			{
				int entid = t.entityelement[te].bankindex;
				if ( entid == iEntityBankID )
				{
					int iObj = t.entityelement[te].obj;
					if ( iObj > 0 ) 
					{
						DeleteObject ( iObj );
					}
				}
			}
		}
		// now delete old parent
		DeleteObject ( t.sourceobj );
	}

	// create fresh entity profile parent object from latest optimized 
	int iEntityElementObj = t.entityelement[entityelementindex].obj;

	// Clone new parent from above working object
	CloneObject ( t.sourceobj, iEntityElementObj );
	PositionObject ( t.sourceobj, -999999, -999999, -999999 );

	// finally create new instances from revised parent
	for ( int te = 1 ; te <= g.entityelementlist; te++ )
	{
		if ( te != entityelementindex )
		{
			int entid = t.entityelement[te].bankindex;
			if ( entid == iEntityBankID )
			{
				int iObj = t.entityelement[te].obj;
				if ( iObj > 0 ) 
				{
					// must create clone state and instance object initially  
					t.tte = te;
					t.entityelement[t.tte].isclone = 1;
					InstanceObject ( iObj, t.sourceobj );

					// then this function can recreate the instance properly with positions and settings
					entity_converttoinstance();
				}
			}
		}
	}
}

void ebe_freecubedata ( int entitybankindex )
{
	SAFE_DELETE ( t.entityprofile[entitybankindex].ebe.pRLEData );
	t.entityprofile[entitybankindex].ebe.dwRLESize = 0; 
	SAFE_DELETE ( t.entityprofile[entitybankindex].ebe.iMatRef );
	t.entityprofile[entitybankindex].ebe.dwMatRefCount = 0; 
	SAFE_DELETE ( t.entityprofile[entitybankindex].ebe.pTexRef );
	t.entityprofile[entitybankindex].ebe.dwTexRefCount = 0; 
}

void ebe_makeseamless ( int iRow, int iCol, float* fU1, float* fU1R, float* fV1, float* fV1R, float* fW1, float* fZ1, float* fW1R, float* fZ1R, float* fU2, float* fU2R, float* fV2, float* fW2, float* fZ2, float* fW2R, float* fZ2R )
{
	// shrink UV coords to fit into 1022x1022 tiling (seamless trick)
	*fU1 -= (0.25f) * iCol;
	*fV1 -= (0.25f) * iRow;
	*fU2 -= (0.25f) * iCol;
	*fV2 -= (0.25f) * iRow;
	*fU1R -= (0.25f) * iCol;
	*fU2R -= (0.25f) * iCol;
	*fZ1R -= (0.25f) * iRow;
	*fZ2R -= (0.25f) * iRow;
	*fZ1 -= (0.25f) * iRow;
	*fZ2 -= (0.25f) * iRow;
	*fW1 -= (0.25f) * iCol;
	*fW2 -= (0.25f) * iCol;
	*fW1R -= (0.25f) * iCol;
	*fW2R -= (0.25f) * iCol;
	float fShrinkDest = 1022.0f;
	*fU1 = (*fU1/1024.0f) * fShrinkDest;
	*fV1 = (*fV1/1024.0f) * fShrinkDest;
	*fU2 = (*fU2/1024.0f) * fShrinkDest;
	*fV2 = (*fV2/1024.0f) * fShrinkDest;
	*fU1R = (*fU1R/1024.0f) * fShrinkDest;
	*fU2R = (*fU2R/1024.0f) * fShrinkDest;
	*fZ1R = (*fZ1R/1024.0f) * fShrinkDest;
	*fZ2R = (*fZ2R/1024.0f) * fShrinkDest;
	*fZ1 = (*fZ1/1024.0f) * fShrinkDest;
	*fZ2 = (*fZ2/1024.0f) * fShrinkDest;
	*fW1 = (*fW1/1024.0f) * fShrinkDest;
	*fW2 = (*fW2/1024.0f) * fShrinkDest;
	*fW1R = (*fW1R/1024.0f) * fShrinkDest;
	*fW2R = (*fW2R/1024.0f) * fShrinkDest;
	float fPixelBorder = 1.0f;
	*fU1 += (0.25f/1024.0f) * fPixelBorder;
	*fV1 += (0.25f/1024.0f) * fPixelBorder;
	*fU2 += (0.25f/1024.0f) * fPixelBorder;
	*fV2 += (0.25f/1024.0f) * fPixelBorder;
	*fU1R += (0.25f/1024.0f) * fPixelBorder;
	*fU2R += (0.25f/1024.0f) * fPixelBorder;
	*fZ1R += (0.25f/1024.0f) * fPixelBorder;
	*fZ2R += (0.25f/1024.0f) * fPixelBorder;
	*fZ1 += (0.25f/1024.0f) * fPixelBorder;
	*fZ2 += (0.25f/1024.0f) * fPixelBorder;
	*fW1 += (0.25f/1024.0f) * fPixelBorder;
	*fW2 += (0.25f/1024.0f) * fPixelBorder;
	*fW1R += (0.25f/1024.0f) * fPixelBorder;
	*fW2R += (0.25f/1024.0f) * fPixelBorder;
	*fU1 += (0.25f) * iCol;
	*fV1 += (0.25f) * iRow;
	*fU2 += (0.25f) * iCol;
	*fV2 += (0.25f) * iRow;
	*fU1R += (0.25f) * iCol;
	*fU2R += (0.25f) * iCol;
	*fZ1R += (0.25f) * iRow;
	*fZ2R += (0.25f) * iRow;
	*fZ1 += (0.25f) * iRow;
	*fZ2 += (0.25f) * iRow;
	*fW1 += (0.25f) * iCol;
	*fW2 += (0.25f) * iCol;
	*fW1R += (0.25f) * iCol;
	*fW2R += (0.25f) * iCol;
}

void ebe_refreshmesh ( int iBuildObj, int x1, int y1, int z1, int x2, int y2, int z2 )
{
	// Refresh mesh with cube construction
	sObject* pObject = GetObjectData ( iBuildObj );
	if ( pObject )
	{
		// may need this to map effect to newly created limb
		int iEBEEffectIndex = loadinternaleffect("effectbank\\reloaded\\ebe_basic.fx");

		// count verts in all grid locations
		// fixed 50x50x50 cube sliced into 5 units (10x10x10)
		for ( int y = y1; y <= y2; y+=10 )
		{
			for ( int z = z1; z <= z2; z+=10 )
			{
				for ( int x = x1; x <= x2; x+=10 )
				{
					// count verts only for this slot (mesh/limb collection)
					DWORD dwVertPos = 0;
					DWORD dwIndicePos = 0;

					// this step will create a single mesh for the 10x10x10 cube collection at this mastergrid ref (x,y,z)
					for ( int yb = 0; yb < 10; yb++ )
					{
						for ( int xb = 0; xb < 10; xb++ )
						{
							for ( int zb = 0; zb < 10; zb++ )
							{
								// should we create a cube here
								int iAbsX = x+xb;
								int iAbsY = y+yb;
								int iAbsZ = z+zb;
								unsigned char cCubeCode = pCubes[iAbsX][iAbsY][iAbsZ];
								if ( cCubeCode > 0)
								{
									// next cube
									dwVertPos+=24;
									dwIndicePos+=36;
								}
							}
						}
					}

					// store verts for this slot and increment slot index
					int iSlotIndex = pMasterGridMeshRef[x/10][y/10][z/10][0];
					pVertCountStore[iSlotIndex] = dwVertPos;
				}
			}
		}

		// clear trigger flags
		for ( int iLimbIndex = 0; iLimbIndex < CUBEMAXMESH; iLimbIndex++ )
			pbTriggerDrawBufferCreation[iLimbIndex] = false;

		// create meshes from above verts collected
		DWORD dwColor = GGCOLOR(1,1,1,1);
		float fU = 1.0f/4.0f;
		float fV = 1.0f/4.0f;
		for ( int y = y1; y <= y2; y+=10 )
		{
			for ( int z = z1; z <= z2; z+=10 )
			{
				for ( int x = x1; x <= x2; x+=10 )
				{
					// record slot and limb index at the slot location
					int iMasterGridRefX = x / 10;
					int iMasterGridRefY = y / 10;
					int iMasterGridRefZ = z / 10;
					int iSlotIndex = pMasterGridMeshRef[iMasterGridRefX][iMasterGridRefY][iMasterGridRefZ][0];
					int iLimbIndex = pMasterGridMeshRef[iMasterGridRefX][iMasterGridRefY][iMasterGridRefZ][1];

					// if need a new mesh, create it here
					if ( iLimbIndex == 0 && pVertCountStore[iSlotIndex] > 0 )
					{
						iLimbIndex = pObject->iFrameCount;
						AddLimb ( iBuildObj, iLimbIndex, g.meshebe1 );
						HideLimb ( iBuildObj, iLimbIndex );
						pObject->ppFrameList[iLimbIndex]->pMesh->dwTextureCount = 1;
						pObject->ppFrameList[iLimbIndex]->pMesh->pTextures = new sTexture[1];
						sMesh* pDestMesh = pObject->ppFrameList[iLimbIndex]->pMesh;
						CloneInternalTextures ( pDestMesh, pObject->ppMeshList[0] );
						if ( pDestMesh->pVertexShaderEffect == NULL )
						{
							sEffectItem* pEffectItem = m_EffectList [ iEBEEffectIndex ];
							SetSpecialEffect ( pDestMesh, pEffectItem->pEffectObj );
							pDestMesh->bVertexShaderEffectRefOnly = true;
						}
						pMasterGridMeshRef[iMasterGridRefX][iMasterGridRefY][iMasterGridRefZ][1] = iLimbIndex;
					}

					// set up mesh for exact number of cubes
					sMesh* pMesh = pObject->ppFrameList[iLimbIndex]->pMesh;
					DWORD dwVertPos = pVertCountStore[iSlotIndex];
					DWORD dwIndicePos = (dwVertPos/2)*3;
					SetupMeshFVFData ( pMesh, pMesh->dwFVF, dwVertPos, dwIndicePos );
					pMesh->iPrimitiveType = GGPT_TRIANGLELIST;
					pMesh->iDrawVertexCount = dwVertPos;
					pMesh->iDrawPrimitives  = dwIndicePos / 3;
					pMesh->bVBRefreshRequired = true;
					pMesh->bMeshHasBeenReplaced = true;
					g_vRefreshMeshList.push_back ( pMesh );
					ShowLimb ( iBuildObj, iLimbIndex );
					if ( pMesh->pDrawBuffer == NULL )
					{
						// add object mesh to buffers
						pbTriggerDrawBufferCreation[iLimbIndex] = true;
					}

					// this step will create a single mesh for the 10x10x10 cube collection
					dwVertPos = 0;
					dwIndicePos = 0;
					for ( int yb = 0; yb < 10; yb++ )
					{
						for ( int xb = 0; xb < 10; xb++ )
						{
							for ( int zb = 0; zb < 10; zb++ )
							{
								// should we create a cube here
								int iAbsX = x+xb;
								int iAbsY = y+yb;
								int iAbsZ = z+zb;
								unsigned char cCubeCode = pCubes[iAbsX][iAbsY][iAbsZ];
								if ( cCubeCode > 0)
								{
									// UV for texture choice
									unsigned char cCubeTexIndex = (cCubeCode & (15<<4)) >> 4;
									int iRow = cCubeTexIndex / 4;
									int iCol = cCubeTexIndex - (iRow*4);
									float fBitU = fU / 20.0f;
									float fBitV = fV / 20.0f;
									float fCoverageU = fBitU * (float)(iAbsX%20);
									float fCoverageUR = fBitU * (float)(19-(iAbsX%20));
									float fCoverageV = fBitV * (float)(19-(iAbsY%20));
									float fCoverageVR = fBitV * (float)(iAbsY%20);
									float fCoverageW = fBitU * (float)(iAbsZ%20);
									float fCoverageWR = fBitU * (float)(19-(iAbsZ%20));
									float fU1 = (fU * iCol)+fCoverageU;
									float fU1R = (fU * iCol)+fCoverageUR;
									float fV1 = (fV * iRow)+fCoverageV;
									float fV1R = (fV * iRow)+fCoverageVR;
									float fW1 = (fU * iCol)+fCoverageW;
									float fZ1 = (fU * iRow)+fCoverageW;
									float fW1R = (fU * iCol)+fCoverageWR;
									float fZ1R = (fU * iRow)+fCoverageWR;
									float fU2 = fU1 + fBitU;
									float fU2R = fU1R + fBitU;
									float fV2 = fV1 + fBitV;
									float fV2R = fV1R + fBitV;
									float fW2 = fW1 + fBitU;
									float fZ2 = fZ1 + fBitU;
									float fW2R = fW1R + fBitU;
									float fZ2R = fZ1R + fBitU;

									// process UV for seamless texturing
									ebe_makeseamless ( iRow, iCol, &fU1, &fU1R, &fV1, &fV1R, &fW1, &fZ1, &fW1R, &fZ1R, &fU2, &fU2R, &fV2, &fW2, &fZ2, &fW2R, &fZ2R );

									// create cube verts
									float fWidth1 = (iAbsX*5.0f)+0.0f;
									float fWidth2 = (iAbsX*5.0f)+5.0f;
									float fHeight1 = (iAbsY*5.0f)+0.0f;
									float fHeight2 = (iAbsY*5.0f)+5.0f;
									float fDepth1 = (iAbsZ*5.0f)+0.0f;
									float fDepth2 = (iAbsZ*5.0f)+5.0f;
									SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData, dwVertPos+0, fWidth1, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU1, fV1 );	// front
									SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData, dwVertPos+1, fWidth2, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU2, fV1 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+2, fWidth2, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU2, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+3, fWidth1, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU1, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+4, fWidth1, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU2R, fV1 );	// back
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+5, fWidth1, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU2R, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+6, fWidth2, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU1R, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+7, fWidth2, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU1R, fV1 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+8, fWidth1, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, fU1, fZ1R );	// top
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+9, fWidth2, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, fU2, fZ1R );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+10, fWidth2, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, fU2, fZ2R );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+11, fWidth1, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, fU1, fZ2R );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+12, fWidth1, fHeight1, fDepth2,  0.0f, -1.0f,  0.0f, dwColor, fU1, fZ2 );	// bottom
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+13, fWidth1, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, fU1, fZ1 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+14, fWidth2, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, fU2, fZ1 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+15, fWidth2, fHeight1, fDepth2,	 0.0f, -1.0f,  0.0f, dwColor, fU2, fZ2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+16, fWidth2, fHeight2, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, fW1, fV1 );	// right
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+17, fWidth2, fHeight2, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, fW2, fV1 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+18, fWidth2, fHeight1, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, fW2, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+19, fWidth2, fHeight1, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, fW1, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+20, fWidth1, fHeight2, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, fW2R, fV1 );	// left
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+21, fWidth1, fHeight1, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, fW2R, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+22, fWidth1, fHeight1, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, fW1R, fV2 );
									SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+23, fWidth1, fHeight2, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, fW1R, fV1 );

									// and now fill in the index list
									pMesh->pIndices [ dwIndicePos+0 ] = dwVertPos+0;		pMesh->pIndices [ dwIndicePos+1 ] = dwVertPos+1;		pMesh->pIndices [ dwIndicePos+2 ] = dwVertPos+2;
									pMesh->pIndices [ dwIndicePos+3 ] = dwVertPos+2;		pMesh->pIndices [ dwIndicePos+4 ] = dwVertPos+3;		pMesh->pIndices [ dwIndicePos+5 ] = dwVertPos+0;
									pMesh->pIndices [ dwIndicePos+6 ] = dwVertPos+4;		pMesh->pIndices [ dwIndicePos+7 ] = dwVertPos+5;		pMesh->pIndices [ dwIndicePos+8 ] = dwVertPos+6;
									pMesh->pIndices [ dwIndicePos+9 ] = dwVertPos+6;		pMesh->pIndices [ dwIndicePos+10 ] = dwVertPos+7;		pMesh->pIndices [ dwIndicePos+11 ] = dwVertPos+4;
									pMesh->pIndices [ dwIndicePos+12 ] = dwVertPos+8;		pMesh->pIndices [ dwIndicePos+13 ] = dwVertPos+9;		pMesh->pIndices [ dwIndicePos+14 ] = dwVertPos+10;
									pMesh->pIndices [ dwIndicePos+15 ] = dwVertPos+10;		pMesh->pIndices [ dwIndicePos+16 ] = dwVertPos+11;		pMesh->pIndices [ dwIndicePos+17 ] = dwVertPos+8;
									pMesh->pIndices [ dwIndicePos+18 ] = dwVertPos+12;		pMesh->pIndices [ dwIndicePos+19 ] = dwVertPos+13;		pMesh->pIndices [ dwIndicePos+20 ] = dwVertPos+14;
									pMesh->pIndices [ dwIndicePos+21 ] = dwVertPos+14;		pMesh->pIndices [ dwIndicePos+22 ] = dwVertPos+15;		pMesh->pIndices [ dwIndicePos+23 ] = dwVertPos+12;
									pMesh->pIndices [ dwIndicePos+24 ] = dwVertPos+16;		pMesh->pIndices [ dwIndicePos+25 ] = dwVertPos+17;		pMesh->pIndices [ dwIndicePos+26 ] = dwVertPos+18;
									pMesh->pIndices [ dwIndicePos+27 ] = dwVertPos+18;		pMesh->pIndices [ dwIndicePos+28 ] = dwVertPos+19;		pMesh->pIndices [ dwIndicePos+29 ] = dwVertPos+16;
									pMesh->pIndices [ dwIndicePos+30 ] = dwVertPos+20;		pMesh->pIndices [ dwIndicePos+31 ] = dwVertPos+21;		pMesh->pIndices [ dwIndicePos+32 ] = dwVertPos+22;
									pMesh->pIndices [ dwIndicePos+33 ] = dwVertPos+22;		pMesh->pIndices [ dwIndicePos+34 ] = dwVertPos+23;		pMesh->pIndices [ dwIndicePos+35 ] = dwVertPos+20;

									// next cube
									dwVertPos+=24;
									dwIndicePos+=36;
								}
							}
						}
					}
				}
			}
		}

		// ensure we do not trigger slow bounds recalc
		pObject->bUpdateOverallBounds = false;

		// if any triggers flagged, create drawbuffer if not exist
		for ( int iLimbIndex = 1; iLimbIndex < CUBEMAXMESH; iLimbIndex++ )
		{
			if ( pbTriggerDrawBufferCreation[iLimbIndex] == true )
			{
				sMesh* pMesh = pObject->ppFrameList[iLimbIndex]->pMesh;
				if ( pMesh->pDrawBuffer == NULL )
				{
					// add object mesh to buffers (unique VB/IB buffer for this new mesh)
					m_ObjectManager.AddObjectMeshToBuffers ( pMesh, true );
				}
			}
		}
	}
}

void ebe_loadpattern ( LPSTR pEBEFilename )
{
	// default pattern is single cube
	ebebuild.Pattern.iWidth = 1;
	ebebuild.Pattern.iHeight = 1;
	ebebuild.Pattern.iDepth = 1;
	ebebuild.Pattern.iPreserveMode = 0;
	for ( int iY = 0; iY < 20; iY++ )
	{
		for ( int iZ = 0; iZ < 20; iZ++ )
		{
			ebebuild.Pattern.pPRow[iY][iZ] = cStr("0");
		}
	}
	ebebuild.Pattern.pPRow[0][0] = cStr("1");

	// if EBE file exists, replace above pattern
	if ( FileExist(pEBEFilename) == 1 ) 
	{
		Dim ( t.data_s, 2000 );
		LoadArray ( pEBEFilename, t.data_s );
		for ( t.l = 0; t.l <= 1999; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if (  t.line_s.Get()[0] != ';' ) 
				{
					//  take fieldname and value
					for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
					{
						if ( t.line_s.Get()[t.c] == '=' ) 
						{ 
							t.mid = t.c+1  ; break;
						}
					}
					t.field_s=cstr(Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1))));
					t.value_s=cstr(removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid)));
					for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
					{
						if (  t.value_s.Get()[t.c] == ',' ) 
						{ 
							t.mid = t.c+1 ; break; 
						}
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value1_f=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value2_s=cstr(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));
					if ( Len(t.value2_s.Get())>0  ) t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;

					// extract field data from EBE file
					t.tryfield_s="width";
					if ( t.field_s == t.tryfield_s  ) 
					{
						ebebuild.Pattern.iWidth = t.value1;
						if ( ebebuild.Pattern.iWidth > 20 ) ebebuild.Pattern.iWidth = 20;
					}
					t.tryfield_s="height";
					if ( t.field_s == t.tryfield_s  )
					{
						ebebuild.Pattern.iHeight = t.value1;
						if ( ebebuild.Pattern.iHeight > 20 ) ebebuild.Pattern.iHeight = 20;
					}
					t.tryfield_s="depth";
					if ( t.field_s == t.tryfield_s  ) 
					{
						ebebuild.Pattern.iDepth = t.value1;
						if ( ebebuild.Pattern.iDepth > 20 ) ebebuild.Pattern.iDepth = 20;
					}
					t.tryfield_s="preservemode";
					if ( t.field_s == t.tryfield_s  ) 
					{
						ebebuild.Pattern.iPreserveMode = t.value1;
					}

					// pattern strings
					for ( int iY = 0; iY < ebebuild.Pattern.iHeight; iY++ )
					{
						for ( int iZ = 0; iZ < ebebuild.Pattern.iDepth; iZ++ )
						{
							t.tryfield_s = cStr("prow") + cStr(iY) + cStr("x") + cStr(iZ);
							if ( t.field_s == t.tryfield_s  ) ebebuild.Pattern.pPRow[iY][iZ] = Lower(t.value_s.Get());
						}
					}
				}
			}
		}
		UnDim (  t.data_s );
	}
	ebebuild.Pattern.iWidthOffset = 0;
	ebebuild.Pattern.iDepthOffset = 0;

	// copy loaded pattern to original store
	ebebuild.OriginalPattern = ebebuild.Pattern;

	// if go direct to use pattern, ensure current rotation taken into account
	ebe_updatepatternwithrotation();
}

void ebe_updatepatternwithrotation ( void )
{
	// uses ebebuild.iCursorRotation
	if ( ebebuild.iCursorRotation == 0 )
	{
		ebebuild.Pattern = ebebuild.OriginalPattern;
	}
	else
	{
		// wipe pattern to recreate
		ebebuild.Pattern.iWidth = 0;
		ebebuild.Pattern.iHeight = 0;
		ebebuild.Pattern.iDepth = 0;
		ebebuild.Pattern.iWidthOffset = 0;
		ebebuild.Pattern.iDepthOffset = 0;
		for ( int iY = 0; iY < 20; iY++ )
		{
			for ( int iZ = 0; iZ < 20; iZ++ )
			{
				ebebuild.Pattern.pPRow[iY][iZ] = cStr("0");
			}
		}
		if ( ebebuild.iCursorRotation == 1 )
		{
			// 90 degree CW for XZ
			ebebuild.Pattern.iWidth = ebebuild.OriginalPattern.iDepth;
			ebebuild.Pattern.iHeight = ebebuild.OriginalPattern.iHeight;
			ebebuild.Pattern.iDepth = ebebuild.OriginalPattern.iWidth;
			for ( int iY = 0; iY < ebebuild.Pattern.iHeight; iY++ )
			{
				int iX = 0;
				for ( int iZ = ebebuild.Pattern.iDepth-1; iZ >= 0 ; iZ-- )
				{
					cStr pFillX = cStr("");
					for ( int iOZ = 0; iOZ < ebebuild.OriginalPattern.iDepth; iOZ++ )
					{
						LPSTR pNeedFullZeros = ebebuild.OriginalPattern.pPRow[iY][iOZ].Get();
						if ( strcmp ( pNeedFullZeros, "0" ) == NULL ) pNeedFullZeros = "00000000000000000000";
						pFillX = pFillX + Mid(pNeedFullZeros,1+iX);
					}
					ebebuild.Pattern.pPRow[iY][iZ] = pFillX;
					iX++;
				}
			}
		}
		if ( ebebuild.iCursorRotation == 2 )
		{
			// 180 degree CW for XZ
			ebebuild.Pattern.iWidth = ebebuild.OriginalPattern.iWidth;
			ebebuild.Pattern.iHeight = ebebuild.OriginalPattern.iHeight;
			ebebuild.Pattern.iDepth = ebebuild.OriginalPattern.iDepth;
			ebebuild.Pattern.iDepthOffset = ebebuild.OriginalPattern.iWidth-ebebuild.OriginalPattern.iDepth;
			for ( int iY = 0; iY < ebebuild.Pattern.iHeight; iY++ )
			{
				int iOZ = 0;
				for ( int iZ = ebebuild.OriginalPattern.iDepth-1; iZ >= 0; iZ-- )
				{
					cStr pFillX = cStr("");
					for ( int iOX = ebebuild.OriginalPattern.iWidth-1; iOX >= 0; iOX-- )
					{
						pFillX = pFillX + Mid(ebebuild.OriginalPattern.pPRow[iY][iOZ].Get(),1+iOX);
					}
					ebebuild.Pattern.pPRow[iY][iZ] = pFillX;
					iOZ++;
				}
			}
		}
		if ( ebebuild.iCursorRotation == 3 )
		{
			// 270 degree CW for XZ
			ebebuild.Pattern.iWidth = ebebuild.OriginalPattern.iDepth;
			ebebuild.Pattern.iHeight = ebebuild.OriginalPattern.iHeight;
			ebebuild.Pattern.iDepth = ebebuild.OriginalPattern.iWidth;
			ebebuild.Pattern.iWidthOffset = ebebuild.OriginalPattern.iWidth-ebebuild.OriginalPattern.iDepth;
			for ( int iY = 0; iY < ebebuild.Pattern.iHeight; iY++ )
			{
				int iOX = 0;
				for ( int iZ = 0; iZ < ebebuild.Pattern.iDepth; iZ++ )
				{
					cStr pFillX = cStr("");
					for ( int iOZ = ebebuild.OriginalPattern.iDepth-1; iOZ >=0; iOZ-- )
					{
						pFillX = pFillX + Mid(ebebuild.OriginalPattern.pPRow[iY][iOZ].Get(),1+iOX);
					}
					ebebuild.Pattern.pPRow[iY][iZ] = pFillX;
					iOX++;
				}
			}
		}
	}
}

void ebe_settexturehighlight ( void )
{
	int iRow = ebebuild.iCurrentTexture / 4;
	int iCol = ebebuild.iCurrentTexture - (iRow*4);
	float fChoiceWidth = ebebuild.iTexturePanelWidth/4;
	float fChoiceHeight = ebebuild.iTexturePanelHeight/4;
	Sprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelX+(iCol*fChoiceWidth), ebebuild.iTexturePanelY+(iRow*fChoiceHeight), ebebuild.iTexturePanelHighImg );
}

void ebe_loop ( void )
{
	// if no tools object, cannot proceed
	if ( ebebuild.iToolObj == 0 ) return;
	if ( ObjectExist ( ebebuild.iToolObj ) == 0 ) return;
	if ( GetObjectData ( ebebuild.iToolObj )->bVisible == false ) return;

	// if UNDO or somehow delete this building obj, revert entity mode
	if ( ObjectExist ( ebebuild.iBuildObj ) == 0 )
	{
		ebe_hide();
		t.inputsys.domodeentity = 1;
		return;
	}

	// go through model slowly to refresh meshes (and avoid a vertbuffer lock)
	if ( ebebuild.iRefreshBuild > 0 )
	{
		ebebuild.iRefreshBuild--;
		int iLayerTotarget = 19 - ebebuild.iRefreshBuild;
		int iYBottom = (iLayerTotarget*10);
		int iYTop = (iLayerTotarget*10)+10;
		if ( iYTop > CUBEAREASIZE-11 ) iYTop = CUBEAREASIZE-11;
		ebe_refreshmesh ( ebebuild.iBuildObj, 0, iYBottom, 0, CUBEAREASIZE-10, iYTop, CUBEAREASIZE-10 );
		return;
	}

	// keep orientation with the main build object
	RotateObject ( ebebuild.iToolObj, ObjectAngleX(ebebuild.iBuildObj), ObjectAngleY(ebebuild.iBuildObj), ObjectAngleZ(ebebuild.iBuildObj) );

	// Keep tool object with EBE entity object (build object)
	int iAtX = t.entityelement[t.ebe.entityelementindex].x;
	int iAtY = t.entityelement[t.ebe.entityelementindex].y;
	int iAtZ = t.entityelement[t.ebe.entityelementindex].z;
	PositionObject ( ebebuild.iToolObj, iAtX, iAtY+(CameraPositionY(0)/5000.0f), iAtZ );
	PositionObject ( ebebuild.iBuildObj, iAtX, iAtY, iAtZ );

	// Only when release mouse continue
	if ( t.ebe.bReleaseMouseFirst == true && t.inputsys.mclick != 0 ) return;
	t.ebe.bReleaseMouseFirst = false;

	// One press key logic
	if ( t.inputsys.kscancode == 0 ) ebebuild.iLocalKeyPressed = 0;

	// Reason this is above action for selecting customise is to allow texture highlight to show as selected
	if ( ebebuild.bCustomiseTexture == true && t.inputsys.mclick == 0 )
	{
		ebebuild.bCustomiseTexture = false;
		int iEntityProfileIndex = t.entityelement[t.ebe.entityelementindex].bankindex;
		if ( ebe_loadcustomtexture ( iEntityProfileIndex, ebebuild.iCurrentTexture ) == 1 )
		{
			// successfully pasted new texture into plate
		}
	}

	// use - and + keys to scroll through material index for current texture selected
	if ( ebebuild.iLocalKeyPressed == 0 )
	{
		if ( t.inputsys.k_s == "-" ) { ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture]-=1; ebebuild.iLocalKeyPressed = 1; }
		if ( t.inputsys.k_s == "=" ) { ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture]+=1; ebebuild.iLocalKeyPressed = 1; }
		if ( ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] < 0 ) ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] = 0;
		if ( ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] > 18 ) ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] = 18;
		SetSpriteImage(ebebuild.iMatSpr[ebebuild.iCurrentTexture],ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture]]);
	}

	// Select texture if in Texture Panel or Customise one
	int iRealSprMouseX = (GetChildWindowWidth()/800.0f) * t.inputsys.xmouse;
	int iRealSprMouseY = (GetChildWindowHeight()/600.0f) * t.inputsys.ymouse;
	if ( t.inputsys.mclick > 0 )
	{
		if ( iRealSprMouseX > ebebuild.iTexturePanelX && iRealSprMouseX < ebebuild.iTexturePanelX + ebebuild.iTexturePanelWidth )
		{
			if ( iRealSprMouseY > ebebuild.iTexturePanelY && iRealSprMouseY < ebebuild.iTexturePanelY + ebebuild.iTexturePanelHeight )
			{
				// while tile
				float fWhichCol = (float)(iRealSprMouseX - ebebuild.iTexturePanelX) / (float)ebebuild.iTexturePanelWidth;
				float fWhichRow = (float)(iRealSprMouseY - ebebuild.iTexturePanelY) / (float)ebebuild.iTexturePanelHeight;
				int iWhichTextureOver = (((int)(fWhichRow*4))*4) + (int)(fWhichCol*4);

				// select texture choice
				ebebuild.iCurrentTexture = iWhichTextureOver;
				ebe_settexturehighlight();

				// and if it was right mouse, customise this texture too
				if ( t.inputsys.mclick == 2 )
				{
					// replace texture within texture atlas
					ebebuild.bCustomiseTexture = true;
				}

				// ensure we do not write into builder if selecting texture
				t.inputsys.mclick = 0;
			}
		}
	}

	// Control EBE edit grid layer
	float fCurrentGridLayerAbsHeight = ebebuild.iCurrentGridLayer*5.0f;
	float fMouseWheel = t.inputsys.wheelmousemove;
	if ( t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209 || (fMouseWheel != 0 && t.inputsys.keycontrol == 0 ) )
	{
		if ( ebebuild.iLocalKeyPressed == 0 )
		{
			// work out snap grid size
			int iGridSize = ebebuild.Pattern.iWidth;
			if ( ebebuild.Pattern.iHeight > iGridSize ) iGridSize = ebebuild.Pattern.iHeight;
			if ( ebebuild.Pattern.iDepth > iGridSize ) iGridSize = ebebuild.Pattern.iDepth;

			// special case for ROW shape (one high, and one dimension longer on X or Z)
			if ( ebebuild.Pattern.iHeight == 1 && ebebuild.Pattern.iWidth != ebebuild.Pattern.iDepth ) iGridSize = 1;

			// move current grid up or down
			float fYShift = 0.0f;
			ebebuild.iLocalKeyPressed = 1;
			if ( (t.inputsys.kscancode == 201 || fMouseWheel > 0.0f) && ebebuild.iCurrentGridLayer < 200-iGridSize ) { ebebuild.iCurrentGridLayer += iGridSize; fYShift = iGridSize*5.0f; }
			if ( (t.inputsys.kscancode == 209 || fMouseWheel < 0.0f) ) 
			{ 
				ebebuild.iCurrentGridLayer -= iGridSize; fYShift = iGridSize*-5.0f; 
				if ( ebebuild.iCurrentGridLayer < 0 ) ebebuild.iCurrentGridLayer = 0;
			}
			fCurrentGridLayerAbsHeight = ebebuild.iCurrentGridLayer*5.0f;
			OffsetLimb ( ebebuild.iToolObj, 1, 500, fCurrentGridLayerAbsHeight + 0.05f, 500 );

			// also move camera for easier more intuitive editing 
			if ( fMouseWheel == 0.0f )
			{
				// only with mouse wheel for convenience
				t.editorfreeflight.c.y_f += fYShift;
				PositionCamera ( 0, t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f );
			}
		}
	}

	// Rotate cursor and pattern
	if ( t.inputsys.k_s == "r" )
	{
		if ( ebebuild.iLocalKeyPressed == 0 )
		{
			ebebuild.iLocalKeyPressed = 1;
			ebebuild.iCursorRotation += 1;
			if ( ebebuild.iCursorRotation > 3 )
			{
				ebebuild.iCursorRotation = 0;
			}
			ebe_updatepatternwithrotation();
		}
	}

	// Position cursor on current grid layer
	bool bOffGrid = false;
	SetCurrentCamera ( 0 );
	SetCameraRange ( 1,70000 );
	t.screenwidth_f = 800.0;
	t.screenheight_f = 600.0;
	GetProjectionMatrix ( g.m4_projection );
	GetViewMatrix ( g.m4_view );
	t.blank=InverseMatrix(g.m4_projection,g.m4_projection);
	t.blank=InverseMatrix(g.m4_view,g.m4_view);
	t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
	t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
	t.tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/t.tadjustedtoareax_f;
	t.tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/t.tadjustedtoareay_f;
	SetVector4 ( g.v4_far, (t.tadjustedtoareax_f*2)-1, -((t.tadjustedtoareay_f*2)-1), 0, 1 );
	TransformVector4 ( g.v4_far,g.v4_far,g.m4_projection );

	// works in DX9 (D3DXVec4Transform) but not DX11 (KMATRIX)
	//TransformVector4 (  g.v4_far,g.v4_far,g.m4_view );
	//t.tx_f=GetXVector4(g.v4_far);
	//t.ty_f=GetYVector4(g.v4_far);
	//t.tz_f=GetZVector4(g.v4_far);
	t.tx_f=GetXVector4(g.v4_far);
	t.ty_f=GetYVector4(g.v4_far);
	t.tz_f=GetZVector4(g.v4_far);
	SetVector3 ( g.v3_far, t.tx_f, t.ty_f, t.tz_f );
	TransformVectorCoordinates3 ( g.v3_far, g.v3_far, g.m4_view );
	t.tx_f=GetXVector3(g.v3_far);
	t.ty_f=GetYVector3(g.v3_far);
	t.tz_f=GetZVector3(g.v3_far);

	t.fx_f=CameraPositionX(0);
	t.fy_f=CameraPositionY(0);
	t.fz_f=CameraPositionZ(0);
	t.tx_f=t.tx_f-t.fx_f;
	t.ty_f=t.ty_f-t.fy_f;
	t.tz_f=t.tz_f-t.fz_f;
	t.tt_f=abs(t.tx_f)+abs(t.ty_f)+abs(t.tz_f);
	t.tx_f=t.tx_f/t.tt_f;
	t.ty_f=t.ty_f/t.tt_f;
	t.tz_f=t.tz_f/t.tt_f;
	if ( t.ty_f < 0 ) 
	{
		// face down onto grid
		float fGridAbsHeight = LimbPositionY ( ebebuild.iToolObj, 1 );
		float fDistToGridLayer = CameraPositionY(0) - fGridAbsHeight;
		float fSteps = fDistToGridLayer / fabs(t.ty_f);
		t.tx_f *= fSteps;
		t.tz_f *= fSteps;
		t.tx_f -= ObjectPositionX ( ebebuild.iToolObj );
		t.tz_f -= ObjectPositionZ ( ebebuild.iToolObj );
		t.tx_f += CameraPositionX(0);
		t.tz_f += CameraPositionZ(0);
		GGVECTOR3 vec3 = GGVECTOR3(t.tx_f,0,t.tz_f);
		GGMATRIX matRot;
		GGMatrixRotationY ( &matRot, GGToRadian(-ObjectAngleY(ebebuild.iToolObj)) );
		GGVec3TransformCoord ( &vec3, &vec3, &matRot );
		t.tx_f = vec3.x;
		t.tz_f = vec3.z;
		t.tx_f = (int)(t.tx_f/5.0f);
		t.tz_f = (int)(t.tz_f/5.0f);
		if ( t.tx_f < 0 ) { t.tx_f = 0; bOffGrid = true; }
		if ( t.tz_f < 0 ) { t.tz_f = 0; bOffGrid = true; }
		if ( t.tx_f > 199 ) { t.tx_f = 199; bOffGrid = true; }
		if ( t.tz_f > 199 ) { t.tz_f = 199; bOffGrid = true; }
		int iGridSnapX = ebebuild.Pattern.iWidth;
		int iGridSnapZ = ebebuild.Pattern.iDepth;
		if ( iGridSnapX > iGridSnapZ ) iGridSnapZ = iGridSnapX;
		if ( iGridSnapZ > iGridSnapX ) iGridSnapX = iGridSnapZ;
		t.tx_f = (int)(t.tx_f/iGridSnapX);
		t.tz_f = (int)(t.tz_f/iGridSnapZ);
		t.tx_f = (t.tx_f*iGridSnapX) + (ebebuild.Pattern.iWidth/2);
		t.tz_f = (t.tz_f*iGridSnapZ) + (ebebuild.Pattern.iDepth/2);
		t.tx_f += ebebuild.Pattern.iWidthOffset;
		t.tz_f += ebebuild.Pattern.iDepthOffset;
		ebebuild.iCursorGridPosX = t.tx_f;
		ebebuild.iCursorGridPosZ = t.tz_f;
		t.tx_f *= 5.0f;
		t.tz_f *= 5.0f;
		int iHalfX = ebebuild.Pattern.iWidth / 2;
		int iHalfZ = ebebuild.Pattern.iDepth / 2;
		float fFractionX = ebebuild.Pattern.iWidth - (iHalfX*2);
		float fFractionZ = ebebuild.Pattern.iDepth - (iHalfZ*2);
		float fCubeSizeX = fFractionX * 2.5f;
		float fCubeSizeY = ebebuild.Pattern.iHeight * 2.5f;
		float fCubeSizeZ = fFractionZ * 2.5f;
		OffsetLimb ( ebebuild.iToolObj, 2, t.tx_f + fCubeSizeX, fCurrentGridLayerAbsHeight + fCubeSizeY, t.tz_f + fCubeSizeZ );
	}
	else
	{
		// looking UP at grid - nono
		bOffGrid = true;
	}

	// Hide cursor if not in grid
	if ( bOffGrid==false )
	{
		ShowLimb ( ebebuild.iToolObj, 2 );
		ScaleLimb ( ebebuild.iToolObj, 2, ebebuild.Pattern.iWidth*100.0f, ebebuild.Pattern.iHeight*100.0f, ebebuild.Pattern.iDepth*100.0f );
		if ( ebebuild.Pattern.iWidth == ebebuild.Pattern.iDepth )
			RotateLimb ( ebebuild.iToolObj, 2, 0, ebebuild.iCursorRotation * 90.0f, 0 );
		else
			RotateLimb ( ebebuild.iToolObj, 2, 0, 0.0f, 0 );
	}
	else
	{
		HideLimb ( ebebuild.iToolObj, 2 );
	}

	// Before start to mark cube data, snapshot for undo buffer
	if ( t.ebe.iWrittenCubeData == 0 && t.inputsys.mclick == 1 )
	{
		ebe_snapshottobuffer ();
		t.ebe.iWrittenCubeData = 1;
	}
	else
	{
		// and wait for user to click and write 'something'
		if ( t.inputsys.mclick == 0 && t.ebe.iWrittenCubeData > 1 )
		{
			// only then when release mouse, do a new snapshot
			t.ebe.iWrittenCubeData = 0;
		}
	}

	// If use axis-line-system, can correct cursor position
	if ( t.inputsys.keycontrol != 0 )
	{
		if ( ebebuild.iDeterminedAxisDir == 0 )
		{
			if ( ebebuild.iCursorGridPosLastGoodX != ebebuild.iCursorGridPosX ) ebebuild.iDeterminedAxisDir = 1;
			if ( ebebuild.iCursorGridPosLastGoodZ != ebebuild.iCursorGridPosZ ) ebebuild.iDeterminedAxisDir = 2;
			ebebuild.iCursorGridPosLastGoodX = ebebuild.iCursorGridPosX;
			ebebuild.iCursorGridPosLastGoodZ = ebebuild.iCursorGridPosZ;
		}
		else
		{
			if ( ebebuild.iDeterminedAxisDir == 1 ) ebebuild.iCursorGridPosZ = ebebuild.iCursorGridPosLastGoodZ;
			if ( ebebuild.iDeterminedAxisDir == 2 ) ebebuild.iCursorGridPosX = ebebuild.iCursorGridPosLastGoodX;
		}
	}
	else
	{
		ebebuild.iDeterminedAxisDir = 0;
		ebebuild.iCursorGridPosLastGoodX = ebebuild.iCursorGridPosX;
		ebebuild.iCursorGridPosLastGoodZ = ebebuild.iCursorGridPosZ;
	}

	// Detect if we add/delete/texture inside the grid
	if ( bOffGrid==false )
	{
		bool bReverseOperation = false;
		if ( t.inputsys.keyshift == 1 ) bReverseOperation = true;
		if ( t.inputsys.mclick == 1 )
		{
			// holding down left mouse button - add/delete/texture
			bool bRecordChangeToCubeData = false;
			int iRecordX1=999999, iRecordY1=999999, iRecordZ1=999999;
			int iRecordX2=-999999, iRecordY2=-999999, iRecordZ2=-999999;
			for ( int iPatternX = 0; iPatternX < ebebuild.Pattern.iWidth; iPatternX++ )
			{
				for ( int iPatternY = 0; iPatternY < ebebuild.Pattern.iHeight; iPatternY++ )
				{
					for ( int iPatternZ = 0; iPatternZ < ebebuild.Pattern.iDepth; iPatternZ++ )
					{
						// get cube tyle and texture assignment
						cStr pPattern = ebebuild.Pattern.pPRow[iPatternY][iPatternZ];
						//cStr pTexture = ebebuild.Pattern.pTRow[iPatternY][iPatternZ]; // future idea!
						char* p;
						unsigned char cCubeType = strtol( Mid(pPattern.Get(),1+iPatternX), &p, 16 );
						unsigned char cTexIndex = 0;
						if ( cCubeType > 0 ) cTexIndex = ebebuild.iCurrentTexture;

						// intended new block type
						unsigned char cBitCube = (cTexIndex<<4) + cCubeType;

						// work out exact place in construction grid
						int iThisX = ebebuild.iCursorGridPosX + iPatternX;
						int iThisY = ebebuild.iCurrentGridLayer + iPatternY;
						int iThisZ = ebebuild.iCursorGridPosZ + iPatternZ;

						// cursor holds shape at center, so backward offset
						iThisX = iThisX - ebebuild.Pattern.iWidth / 2;
						iThisZ = iThisZ - ebebuild.Pattern.iDepth / 2;

						// only if within field of construction area
						if ( iThisX < CUBEAREASIZE && iThisY < CUBEAREASIZE && iThisZ < CUBEAREASIZE )
						{
							// add or delete pattern
							bool bPreserveCube = false;
							if ( bReverseOperation == true ) 
							{
								// delete mode
								// only wipe out if same texture
								unsigned char cTexIndexA = (cBitCube & (15<<4)) >> 4;
								unsigned char cTexIndexB = (pCubes[iThisX][iThisY][iThisZ] & (15<<4)) >> 4;
								if ( cTexIndexA == cTexIndexB )
									cBitCube = 0;
								else
									cBitCube = pCubes[iThisX][iThisY][iThisZ];

								// if in delete mode, cancel a delete if the preserve mode protects it
								if ( cBitCube == 0 && ebebuild.Pattern.iPreserveMode == 1 )
								{
									// check if 'wall for example' is next to a cube at Z+1/X+1 which is filled
									int iThisRotation = ebebuild.iCursorRotation;
									if ( iThisRotation == 0 )
										if ( iThisZ+1 < CUBEAREASIZE )
											if ( pCubes[iThisX][iThisY][iThisZ+1] != 0 )
												bPreserveCube = true;
									if ( iThisRotation == 1 )
										if ( iThisX+1 < CUBEAREASIZE )
											if ( pCubes[iThisX+1][iThisY][iThisZ] != 0 )
												bPreserveCube = true;
									if ( iThisRotation == 2 )
										if ( iThisZ-1 >= 0 )
											if ( pCubes[iThisX][iThisY][iThisZ-1] != 0 )
												bPreserveCube = true;
									if ( iThisRotation == 3 )
										if ( iThisX-1 >= 0 )
											if ( pCubes[iThisX-1][iThisY][iThisZ] != 0 )
												bPreserveCube = true;
								}
							}
							else
							{
								// add mode
								// if in add mode, cancel an addition if the preserve mode protects it
								// and the target is already filled in (an existing cube)
								if ( pCubes[iThisX][iThisY][iThisZ] != 0 )
								{
									if ( ebebuild.Pattern.iPreserveMode == 1 )
									{
										// check if 'wall for example' is next to a cube at Z+1/X+1 which is filled
										int iThisRotation = ebebuild.iCursorRotation;
										if ( iThisRotation == 0 )
											if ( iThisZ+1 < CUBEAREASIZE )
												if ( pCubes[iThisX][iThisY][iThisZ+1] != 0 )
													bPreserveCube = true;
										if ( iThisRotation == 1 )
											if ( iThisX+1 < CUBEAREASIZE )
												if ( pCubes[iThisX+1][iThisY][iThisZ] != 0 )
													bPreserveCube = true;
										if ( iThisRotation == 2 )
											if ( iThisZ-1 >= 0 )
												if ( pCubes[iThisX][iThisY][iThisZ-1] != 0 )
													bPreserveCube = true;
										if ( iThisRotation == 3 )
											if ( iThisX-1 >= 0 )
												if ( pCubes[iThisX-1][iThisY][iThisZ] != 0 )
													bPreserveCube = true;
									}
									if ( ebebuild.Pattern.iPreserveMode == 2 )
									{
										// check if 'stairs for example' should not add to anything already present
										if ( pCubes[iThisX][iThisY][iThisZ] != 0 )
											bPreserveCube = true;
									}
								}
							}

							// replace blank with current cube already in there
							if ( bPreserveCube == true )
							{
								cBitCube = pCubes[iThisX][iThisY][iThisZ];
							}

							// proceed to replace cube if different in any way
							if ( pCubes[iThisX][iThisY][iThisZ] != cBitCube )
							{
								pCubes[iThisX][iThisY][iThisZ] = cBitCube;
								if ( iThisX < iRecordX1 ) iRecordX1 = iThisX;
								if ( iThisY < iRecordY1 ) iRecordY1 = iThisY;
								if ( iThisZ < iRecordZ1 ) iRecordZ1 = iThisZ;
								if ( iThisX > iRecordX2 ) iRecordX2 = iThisX;
								if ( iThisY > iRecordY2 ) iRecordY2 = iThisY;
								if ( iThisZ > iRecordZ2 ) iRecordZ2 = iThisZ;
								bRecordChangeToCubeData = true;
							}

							// mark that we have written something to the cube data, just just clicked
							if ( t.ebe.iWrittenCubeData == 1 ) t.ebe.iWrittenCubeData = 2;
						}
					}
				}
			}

			// only update mesh(es) if cube data actually changed
			if ( bRecordChangeToCubeData == true )
			{
				// update within range of pattern stamp
				iRecordX1 = ((int)iRecordX1/10) * 10;
				iRecordY1 = ((int)iRecordY1/10) * 10;
				iRecordZ1 = ((int)iRecordZ1/10) * 10;
				iRecordX2 = ((int)iRecordX2/10) * 10;
				iRecordY2 = ((int)iRecordY2/10) * 10;
				iRecordZ2 = ((int)iRecordZ2/10) * 10;
				ebe_refreshmesh ( ebebuild.iBuildObj, iRecordX1, iRecordY1, iRecordZ1, iRecordX2, iRecordY2, iRecordZ2 );
			}
		}
	}
	else
	{
		// if off grid, and left click another EBE entity, switch to that one
		if ( t.inputsys.mclick == 1 )
		{
			int iFoundE = findentitycursorobj(-1);
			if ( iFoundE > 0 && iFoundE != t.ebe.entityelementindex )
			{
				if ( t.entityprofile[t.entityelement[iFoundE].bankindex].isebe != 0 )
				{
					// change to new site
					ebe_newsite ( iFoundE );
					return;
				}
			}
		}
	}
}

void ebe_snapshottobuffer ( void )
{
	// pack cube data into undo buffer
	ebe_packsite ( &g_dwUndoBufferCount, &g_pUndoBufferPtr );

	// wipe redo as this snapshot is latest
	SAFE_DELETE ( g_pRedoBufferPtr );
	g_dwRedoBufferCount = 0;
}

void ebe_undo ( void )
{
	// ensure we are building a structure
	int iBuildObj = ebebuild.iBuildObj;
	if ( iBuildObj == 0 ) return;

	// extract undo buffer into cube data
	if ( g_pUndoBufferPtr != NULL )
	{
		// first snapshot cube data to redo buffer
		SAFE_DELETE ( g_pRedoBufferPtr );
		ebe_packsite ( &g_dwRedoBufferCount, &g_pRedoBufferPtr );

		// now unpack undo buffer to cube data
		ebe_unpacksite ( g_dwUndoBufferCount, g_pUndoBufferPtr );
		SAFE_DELETE ( g_pUndoBufferPtr );
		g_dwUndoBufferCount = 0;

		// refresh cube data to model
		ebebuild.iRefreshBuild = 20;
	}
}

void ebe_redo ( void )
{
	// ensure we are building a structure
	int iBuildObj = ebebuild.iBuildObj;
	if ( iBuildObj == 0 ) return;

	// revert back to last cube data, the undo was a mistake
	if ( g_pRedoBufferPtr != NULL )
	{
		// first pack cube data to undo buffer (if want to alternative UNDO/REDO)
		SAFE_DELETE ( g_pUndoBufferPtr );
		ebe_packsite ( &g_dwUndoBufferCount, &g_pUndoBufferPtr );

		// now put back the cube data from the redo buffer
		ebe_unpacksite ( g_dwRedoBufferCount, g_pRedoBufferPtr );
		SAFE_DELETE ( g_pRedoBufferPtr );
		g_dwRedoBufferCount = 0;

		// refresh cube data to model
		ebebuild.iRefreshBuild = 20;
	}
}

void ebe_physics_setupebestructure ( int tphyobj, int entityelementindex )
{
	// unpack data into cube data
	int entid = t.entityelement[entityelementindex].bankindex;
	DWORD dwRLESize = t.entityprofile[entid].ebe.dwRLESize;
	if ( dwRLESize > 0 )
	{
		DWORD* pRLEData = t.entityprofile[entid].ebe.pRLEData;
		ebe_unpacksite ( dwRLESize, pRLEData );
	}

	// Create material ref table
	DWORD dwMatRefCount = t.entityprofile[entid].ebe.dwMatRefCount;
	if ( dwMatRefCount < 16 ) dwMatRefCount = 16;
	int* pMatRefTable = new int[dwMatRefCount];
	memset ( pMatRefTable, 0, sizeof ( pMatRefTable ) );
	if ( t.entityprofile[entid].ebe.dwMatRefCount > 0 )
		for ( int n = 0; n < dwMatRefCount; n++ )
			pMatRefTable[n] = t.entityprofile[entid].ebe.iMatRef[n];

	// 3D flood fill to make box list
	for ( int iThisX = 0; iThisX < CUBEAREASIZE; iThisX++ )
	{
		for ( int iThisY = 0; iThisY < CUBEAREASIZE; iThisY++ )
		{
			for ( int iThisZ = 0; iThisZ < CUBEAREASIZE; iThisZ++ )
			{
				pTemp[iThisX][iThisY][iThisZ] = pCubes[iThisX][iThisY][iThisZ];
			}
		}
	}
	int iMyColBoxIndex = 0;
	for ( int iThisX = 0; iThisX < CUBEAREASIZE; iThisX++ )
	{
		for ( int iThisY = 0; iThisY < CUBEAREASIZE; iThisY++ )
		{
			for ( int iThisZ = 0; iThisZ < CUBEAREASIZE; iThisZ++ )
			{
				unsigned char cBitCube = pTemp[iThisX][iThisY][iThisZ];
				if ( cBitCube != 0 )
				{
					// only wipe out if same texture
					unsigned char cTexIndexA = (cBitCube & (15<<4)) >> 4;
					int iMatRefIndexA = pMatRefTable[cTexIndexA];
					pMyColBox[iMyColBoxIndex].iMaterialIndex = iMatRefIndexA;
					pMyColBox[iMyColBoxIndex].x1 = iThisX;
					pMyColBox[iMyColBoxIndex].y1 = iThisY;
					pMyColBox[iMyColBoxIndex].z1 = iThisZ;
					pMyColBox[iMyColBoxIndex].x2 = iThisX;
					pMyColBox[iMyColBoxIndex].y2 = iThisY;
					pMyColBox[iMyColBoxIndex].z2 = iThisZ;
					int iStopped = 0;
					int iXOkay = 1;
					int iYOkay = 1;
					int iZOkay = 1;
					while ( iXOkay == 1 || iYOkay == 1 || iZOkay == 1 )
					{
						for ( int iTryPass = 0; iTryPass < 3; iTryPass++ )
						{
							bool bDoAxis = false;
							if ( iTryPass == 0 && iXOkay == 1 ) bDoAxis = true;
							if ( iTryPass == 1 && iYOkay == 1 ) bDoAxis = true;
							if ( iTryPass == 2 && iZOkay == 1 ) bDoAxis = true;
							if ( bDoAxis == true )
							{
								if ( iTryPass==0 ) 
								{
									pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x2 + 1;
									if ( pMyColBox[iMyColBoxIndex].x2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].x2 = 199;
										bDoAxis = false;
										iXOkay = 0;
									}
								}
								if ( iTryPass==1 ) 
								{
									pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y2 + 1;
									if ( pMyColBox[iMyColBoxIndex].y2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].y2 = 199;
										bDoAxis = false;
										iYOkay = 0;
									}
								}
								if ( iTryPass==2 ) 
								{
									pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z2 + 1;
									if ( pMyColBox[iMyColBoxIndex].z2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].z2 = 199;
										bDoAxis = false;
										iZOkay = 0;
									}
								}
								if ( bDoAxis == true )
								{
									int iAllSolid = 1;
									for ( int iTX = pMyColBox[iMyColBoxIndex].x1; iTX <= pMyColBox[iMyColBoxIndex].x2; iTX++ )
									{
										for ( int iTY = pMyColBox[iMyColBoxIndex].y1; iTY <= pMyColBox[iMyColBoxIndex].y2; iTY++ )
										{
											for ( int iTZ = pMyColBox[iMyColBoxIndex].z1; iTZ <= pMyColBox[iMyColBoxIndex].z2; iTZ++ )
											{
												unsigned char cTexIndexB = (pTemp[iTX][iTY][iTZ] & (15<<4)) >> 4;
												int iMatRefIndexB = pMatRefTable[cTexIndexB];
												if ( pTemp[iTX][iTY][iTZ] == 0 || iMatRefIndexA != iMatRefIndexB )
												{
													// end this axis
													iAllSolid = 0;
													iTX = pMyColBox[iMyColBoxIndex].x2;
													iTY = pMyColBox[iMyColBoxIndex].y2;
													iTZ = pMyColBox[iMyColBoxIndex].z2;
													break;
												}
											}
										}
									}
									if ( iAllSolid == 0 )
									{
										// failed, step back and flag axis as no more
										if ( iTryPass==0 ) 
										{
											pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x2 - 1;
											if ( pMyColBox[iMyColBoxIndex].x2 < pMyColBox[iMyColBoxIndex].x1 )
											{
												pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x1;
											}
											iXOkay = 0;
										}
										if ( iTryPass==1 ) 
										{
											pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y2 - 1;
											if ( pMyColBox[iMyColBoxIndex].y2 < pMyColBox[iMyColBoxIndex].y1 )
											{
												pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y1;
											}
											iYOkay = 0;
										}
										if ( iTryPass==2 ) 
										{
											pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z2 - 1;
											if ( pMyColBox[iMyColBoxIndex].z2 < pMyColBox[iMyColBoxIndex].z1 )
											{
												pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z1;
											}
											iZOkay = 0;
										}
									}
								}
							}
						}
					}
					// now erase all within this box area
					for ( int iDX = pMyColBox[iMyColBoxIndex].x1; iDX <= pMyColBox[iMyColBoxIndex].x2; iDX++ )
					{
						for ( int iDY = pMyColBox[iMyColBoxIndex].y1; iDY <= pMyColBox[iMyColBoxIndex].y2; iDY++ )
						{
							for ( int iDZ = pMyColBox[iMyColBoxIndex].z1; iDZ <= pMyColBox[iMyColBoxIndex].z2; iDZ++ )
							{
								pTemp[iDX][iDY][iDZ] = 0;
							}
						}
					}
					// move to next box list index
					iMyColBoxIndex++; if ( iMyColBoxIndex >= CUBECOLBOXMAX ) iMyColBoxIndex = CUBECOLBOXMAX-1;
				}
			}
		}
	}

	// Create optimized collision boxes from above 3d flood fill temp array
	ODEStartStaticObject ( tphyobj );
	for ( int iBoxIndex = 0; iBoxIndex < iMyColBoxIndex; iBoxIndex++ )
	{
		int iX1 = pMyColBox[iBoxIndex].x1;
		int iY1 = pMyColBox[iBoxIndex].y1;
		int iZ1 = pMyColBox[iBoxIndex].z1;
		int iW = 1+(pMyColBox[iBoxIndex].x2 - pMyColBox[iBoxIndex].x1);
		int iH = 1+(pMyColBox[iBoxIndex].y2 - pMyColBox[iBoxIndex].y1);
		int iD = 1+(pMyColBox[iBoxIndex].z2 - pMyColBox[iBoxIndex].z1);
		if ( ObjectExist(g.tempimporterlistobject) ) DeleteObject ( g.tempimporterlistobject );
		int iObjectToUse = g.tempimporterlistobject;
		int iSizeX = iW*5;
		int iSizeY = iH*5;
		int iSizeZ = iD*5;
		MakeObjectBox ( iObjectToUse, iSizeX, iSizeY, iSizeZ );
		PositionObject ( iObjectToUse, (iX1*5)+(iSizeX/2), (iY1*5)+(iSizeY/2), (iZ1*5)+(iSizeZ/2) );
		RotateObject ( iObjectToUse, 0, 0, 0 );
		ODEAddStaticObjectBox ( tphyobj, iObjectToUse, pMyColBox[iBoxIndex].iMaterialIndex );
	}
	if ( ObjectExist(g.tempimporterlistobject) ) DeleteObject ( g.tempimporterlistobject );
	ODEEndStaticObject ( tphyobj, 0 );

	// free resources
	SAFE_DELETE ( pMatRefTable );
}

void ebe_optimize_e ( void )
{
	// early out
	if ( t.ebe.entityelementindex == 0 ) return;

	// recreate entity using optimized polygons
	int e = t.ebe.entityelementindex;
	int iObj = t.entityelement[e].obj;

	// optimize the entity element object
	ebe_optimize_object ( iObj, t.entityelement[e].bankindex );

	// ensure parent is updated (for things like extracting, lightmapping, etc)
	ebe_updateparent ( e );

	// Set collision of working object active so can be ray cast detected
	sObject* pObject = g_ObjectList [ iObj ];
	SetColOn ( pObject );
}

void ebe_optimize_object ( int iObj, int iEntID )
{
	// Create material ref table
	DWORD dwMatRefCount = t.entityprofile[iEntID].ebe.dwMatRefCount;
	if ( dwMatRefCount < 16 ) dwMatRefCount = 16;
	int* pMatRefTable = new int[dwMatRefCount];
	memset ( pMatRefTable, 0, sizeof ( pMatRefTable ) );
	if ( t.entityprofile[iEntID].ebe.dwMatRefCount > 0 )
		for ( int n = 0; n < dwMatRefCount; n++ )
			pMatRefTable[n] = t.entityprofile[iEntID].ebe.iMatRef[n];

	// copy cube data to temp array
	for ( int iThisX = 0; iThisX < CUBEAREASIZE; iThisX++ )
		for ( int iThisY = 0; iThisY < CUBEAREASIZE; iThisY++ )
			for ( int iThisZ = 0; iThisZ < CUBEAREASIZE; iThisZ++ )
				pTemp[iThisX][iThisY][iThisZ] = pCubes[iThisX][iThisY][iThisZ];

	// traverse cube data, find boxes of same texture
	int iMyColBoxIndex = 0;
	for ( int iThisX = 0; iThisX < CUBEAREASIZE; iThisX++ )
	{
		for ( int iThisY = 0; iThisY < CUBEAREASIZE; iThisY++ )
		{
			for ( int iThisZ = 0; iThisZ < CUBEAREASIZE; iThisZ++ )
			{
				unsigned char cBitCube = pTemp[iThisX][iThisY][iThisZ];
				if ( cBitCube != 0 )
				{
					pMyColBox[iMyColBoxIndex].x1 = iThisX;
					pMyColBox[iMyColBoxIndex].y1 = iThisY;
					pMyColBox[iMyColBoxIndex].z1 = iThisZ;
					pMyColBox[iMyColBoxIndex].x2 = iThisX;
					pMyColBox[iMyColBoxIndex].y2 = iThisY;
					pMyColBox[iMyColBoxIndex].z2 = iThisZ;
					unsigned char cTexIndex = (cBitCube & (15<<4)) >> 4;
					pMyColBox[iMyColBoxIndex].cCubeTexIndex = cTexIndex;
					int iMatRefIndexA = pMatRefTable[cTexIndex];
					pMyColBox[iMyColBoxIndex].iMaterialIndex = iMatRefIndexA;
					int iRegionX = iThisX / 20; // must split boxes into the 20x20x20 regions to preserve UV wrap (as using atlas)
					int iRegionY = iThisY / 20;
					int iRegionZ = iThisZ / 20;
					int iStopped = 0;
					int iXOkay = 1;
					int iYOkay = 1;
					int iZOkay = 1;
					while ( iXOkay == 1 || iYOkay == 1 || iZOkay == 1 )
					{
						for ( int iTryPass = 0; iTryPass < 3; iTryPass++ )
						{
							bool bDoAxis = false;
							if ( iTryPass == 0 && iXOkay == 1 ) bDoAxis = true;
							if ( iTryPass == 1 && iYOkay == 1 ) bDoAxis = true;
							if ( iTryPass == 2 && iZOkay == 1 ) bDoAxis = true;
							if ( bDoAxis == true )
							{
								if ( iTryPass==0 ) 
								{
									pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x2 + 1;
									if ( pMyColBox[iMyColBoxIndex].x2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].x2 = 199;
										bDoAxis = false;
										iXOkay = 0;
									}
									if ( pMyColBox[iMyColBoxIndex].x2 / 20 != iRegionX )
									{
										pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x2 - 1;
										bDoAxis = false;
										iXOkay = 0;
									}
								}
								if ( iTryPass==1 ) 
								{
									pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y2 + 1;
									if ( pMyColBox[iMyColBoxIndex].y2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].y2 = 199;
										bDoAxis = false;
										iYOkay = 0;
									}
									if ( pMyColBox[iMyColBoxIndex].y2 / 20 != iRegionY )
									{
										pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y2 - 1;
										bDoAxis = false;
										iYOkay = 0;
									}
								}
								if ( iTryPass==2 ) 
								{
									pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z2 + 1;
									if ( pMyColBox[iMyColBoxIndex].z2 >= 200 )
									{
										pMyColBox[iMyColBoxIndex].z2 = 199;
										bDoAxis = false;
										iZOkay = 0;
									}
									if ( pMyColBox[iMyColBoxIndex].z2 / 20 != iRegionZ )
									{
										pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z2 - 1;
										bDoAxis = false;
										iZOkay = 0;
									}
								}
								if ( bDoAxis == true )
								{
									int iAllSolid = 1;
									for ( int iTX = pMyColBox[iMyColBoxIndex].x1; iTX <= pMyColBox[iMyColBoxIndex].x2; iTX++ )
									{
										for ( int iTY = pMyColBox[iMyColBoxIndex].y1; iTY <= pMyColBox[iMyColBoxIndex].y2; iTY++ )
										{
											for ( int iTZ = pMyColBox[iMyColBoxIndex].z1; iTZ <= pMyColBox[iMyColBoxIndex].z2; iTZ++ )
											{
												bool bGapOrDifferentTexture = false;
												unsigned char cBitCube = pTemp[iTX][iTY][iTZ];
												unsigned char cThisTexIndex = (cBitCube & (15<<4)) >> 4;
												if ( cBitCube == 0 ) bGapOrDifferentTexture = true;
												if ( cTexIndex != cThisTexIndex ) bGapOrDifferentTexture = true; // separate by texture to get boxes of one tex type each
												int iMatRefIndexB = pMatRefTable[cThisTexIndex];
												if ( iMatRefIndexA != iMatRefIndexB ) bGapOrDifferentTexture = true; // also separate by material to get boxes of one material type each
												if ( bGapOrDifferentTexture == true )
												{
													// end this axis
													iAllSolid = 0;
													iTX = pMyColBox[iMyColBoxIndex].x2;
													iTY = pMyColBox[iMyColBoxIndex].y2;
													iTZ = pMyColBox[iMyColBoxIndex].z2;
													break;
												}
											}
										}
									}
									if ( iAllSolid == 0 )
									{
										// failed, step back and flag axis as no more
										if ( iTryPass==0 ) 
										{
											pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x2 - 1;
											if ( pMyColBox[iMyColBoxIndex].x2 < pMyColBox[iMyColBoxIndex].x1 )
											{
												pMyColBox[iMyColBoxIndex].x2 = pMyColBox[iMyColBoxIndex].x1;
											}
											iXOkay = 0;
										}
										if ( iTryPass==1 ) 
										{
											pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y2 - 1;
											if ( pMyColBox[iMyColBoxIndex].y2 < pMyColBox[iMyColBoxIndex].y1 )
											{
												pMyColBox[iMyColBoxIndex].y2 = pMyColBox[iMyColBoxIndex].y1;
											}
											iYOkay = 0;
										}
										if ( iTryPass==2 ) 
										{
											pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z2 - 1;
											if ( pMyColBox[iMyColBoxIndex].z2 < pMyColBox[iMyColBoxIndex].z1 )
											{
												pMyColBox[iMyColBoxIndex].z2 = pMyColBox[iMyColBoxIndex].z1;
											}
											iZOkay = 0;
										}
									}
								}
							}
						}
					}
					// now erase all within this box area
					for ( int iDX = pMyColBox[iMyColBoxIndex].x1; iDX <= pMyColBox[iMyColBoxIndex].x2; iDX++ )
					{
						for ( int iDY = pMyColBox[iMyColBoxIndex].y1; iDY <= pMyColBox[iMyColBoxIndex].y2; iDY++ )
						{
							for ( int iDZ = pMyColBox[iMyColBoxIndex].z1; iDZ <= pMyColBox[iMyColBoxIndex].z2; iDZ++ )
							{
								pTemp[iDX][iDY][iDZ] = 0;
							}
						}
					}
					// move to next box list index
					iMyColBoxIndex++; if ( iMyColBoxIndex >= CUBECOLBOXMAX ) iMyColBoxIndex = CUBECOLBOXMAX-1;
				}
			}
		}
	}

	// Create optimized poly boxes
	float fX = ObjectPositionX(iObj);
	float fY = ObjectPositionY(iObj);
	float fZ = ObjectPositionZ(iObj);
	float fRX = ObjectAngleX(iObj);
	float fRY = ObjectAngleY(iObj);
	float fRZ = ObjectAngleZ(iObj);
	DeleteObject ( iObj );
	MakeObject ( iObj, g.meshebemarker, 0 );
	PositionObject ( iObj, fX, fY, fZ );
	RotateObject ( iObj, fRX, fRY, fRZ );

	// create meshes (one per material currently materials range from 0 to 18)
	int iMeshIndex = 0;
	for ( int iMatRefIndex = 0; iMatRefIndex <= 18; iMatRefIndex++ )
	{
		// this material in box collection?
		int iCountMaterialForThisMesh = 0;
		for ( int iBoxIndex = 0; iBoxIndex < iMyColBoxIndex; iBoxIndex++ )
			if ( pMyColBox[iBoxIndex].iMaterialIndex == iMatRefIndex )
				iCountMaterialForThisMesh++;
		if ( iCountMaterialForThisMesh == 0 )
			continue;

		// create this mesh (we found a match material)
		iMeshIndex++;
		AddLimb ( iObj, iMeshIndex, g.meshebemarker );
		sObject* pObject = GetObjectData ( iObj );
		sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
		pMesh->bVBRefreshRequired = true;
		pMesh->bMeshHasBeenReplaced = true;
		g_vRefreshMeshList.push_back ( pMesh );
		DWORD dwVertPos = iCountMaterialForThisMesh * 24;
		DWORD dwIndicePos = (dwVertPos/2)*3;
		SetupMeshFVFData ( pMesh, pMesh->dwFVF, dwVertPos, dwIndicePos );
		pMesh->iPrimitiveType = GGPT_TRIANGLELIST;
		pMesh->iDrawVertexCount = dwVertPos;
		pMesh->iDrawPrimitives  = dwIndicePos / 3;

		// assign material to mesh
		pMesh->Collision.dwArbitaryValue = iMatRefIndex;

		// populate with poly boxes
		DWORD dwColor = GGCOLOR(1,1,1,1);
		float fU = 1.0f/4.0f;
		float fV = 1.0f/4.0f;
		dwVertPos = 0;
		dwIndicePos = 0;
		for ( int iBoxIndex = 0; iBoxIndex < iMyColBoxIndex; iBoxIndex++ )
		{
			// process box if correct material
			if ( pMyColBox[iBoxIndex].iMaterialIndex != iMatRefIndex )
				continue;

			// work out size and position of unified box
			unsigned char cCubeTexIndex = pMyColBox[iBoxIndex].cCubeTexIndex;
			int iX1 = pMyColBox[iBoxIndex].x1;
			int iY1 = pMyColBox[iBoxIndex].y1;
			int iZ1 = pMyColBox[iBoxIndex].z1;
			int iW = 1+(pMyColBox[iBoxIndex].x2 - pMyColBox[iBoxIndex].x1);
			int iH = 1+(pMyColBox[iBoxIndex].y2 - pMyColBox[iBoxIndex].y1);
			int iD = 1+(pMyColBox[iBoxIndex].z2 - pMyColBox[iBoxIndex].z1);
			int iAbsX = iX1;
			int iAbsY = iY1;
			int iAbsZ = iZ1;
			int iAbs2X = iX1+(iW-1);
			int iAbs2Y = iY1+(iH-1);
			int iAbs2Z = iZ1+(iD-1);
			if ( ObjectExist(g.tempimporterlistobject) ) DeleteObject ( g.tempimporterlistobject );
			int iObjectToUse = g.tempimporterlistobject;
			int iSizeX = iW*5;
			int iSizeY = iH*5;
			int iSizeZ = iD*5;

			// UV for texture choice
			int iRow = cCubeTexIndex / 4;
			int iCol = cCubeTexIndex - (iRow*4);
			float fBitU = fU / 20.0f;
			float fBitV = fV / 20.0f;
			float fCoverageU = fBitU * (float)(iAbsX%20);
			float fCoverageUR = fBitU * (float)(19-(iAbs2X%20));
			float fCoverageV = fBitV * (float)(19-(iAbs2Y%20));
			float fCoverageVR = fBitV * (float)(iAbsY%20);
			float fCoverageW = fBitU * (float)(iAbsZ%20);
			float fCoverageWR = fBitU * (float)(19-(iAbs2Z%20));
			float fU1 = (fU * iCol)+fCoverageU;
			float fU1R = (fU * iCol)+fCoverageUR;
			float fV1 = (fV * iRow)+fCoverageV;
			float fV1R = (fV * iRow)+fCoverageVR;
			float fW1 = (fU * iCol)+fCoverageW;
			float fZ1 = (fU * iRow)+fCoverageW;
			float fW1R = (fU * iCol)+fCoverageWR;
			float fZ1R = (fU * iRow)+fCoverageWR;
			float fU2 = fU1 + (fBitU*iW);
			float fU2R = fU1R + (fBitU*iW);
			float fV2 = fV1 + (fBitV*iH);
			float fW2 = fW1 + (fBitU*iD);
			float fZ2 = fZ1 + (fBitU*iD);
			float fW2R = fW1R + (fBitU*iD);
			float fZ2R = fZ1R + (fBitU*iD);

			// process UV for seamless texturing
			ebe_makeseamless ( iRow, iCol, &fU1, &fU1R, &fV1, &fV1R, &fW1, &fZ1, &fW1R, &fZ1R, &fU2, &fU2R, &fV2, &fW2, &fZ2, &fW2R, &fZ2R );

			// create one poly box
			float fWidth1 = (iX1*5);
			float fWidth2 = (iX1*5)+iSizeX;
			float fHeight1 = (iY1*5);
			float fHeight2 = (iY1*5)+iSizeY;
			float fDepth1 = (iZ1*5);
			float fDepth2 = (iZ1*5)+iSizeZ;
			SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData, dwVertPos+0, fWidth1, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU1, fV1 );	// front
			SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData, dwVertPos+1, fWidth2, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU2, fV1 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+2, fWidth2, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU2, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+3, fWidth1, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, fU1, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+4, fWidth1, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU2R, fV1 );	// back
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+5, fWidth1, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU2R, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+6, fWidth2, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU1R, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+7, fWidth2, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, fU1R, fV1 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+8, fWidth1, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, fU1, fZ1R );	// top
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+9, fWidth2, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, fU2, fZ1R );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+10, fWidth2, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, fU2, fZ2R );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+11, fWidth1, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, fU1, fZ2R );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+12, fWidth1, fHeight1, fDepth2,  0.0f, -1.0f,  0.0f, dwColor, fU1, fZ2 );	// bottom
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+13, fWidth1, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, fU1, fZ1 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+14, fWidth2, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, fU2, fZ1 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+15, fWidth2, fHeight1, fDepth2,	 0.0f, -1.0f,  0.0f, dwColor, fU2, fZ2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+16, fWidth2, fHeight2, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, fW1, fV1 );	// right
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+17, fWidth2, fHeight2, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, fW2, fV1 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+18, fWidth2, fHeight1, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, fW2, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+19, fWidth2, fHeight1, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, fW1, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+20, fWidth1, fHeight2, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, fW2R, fV1 );	// left
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+21, fWidth1, fHeight1, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, fW2R, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+22, fWidth1, fHeight1, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, fW1R, fV2 );
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, dwVertPos+23, fWidth1, fHeight2, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, fW1R, fV1 );

			// and now fill in the index list
			pMesh->pIndices [ dwIndicePos+0 ] = dwVertPos+0;		pMesh->pIndices [ dwIndicePos+1 ] = dwVertPos+1;		pMesh->pIndices [ dwIndicePos+2 ] = dwVertPos+2;
			pMesh->pIndices [ dwIndicePos+3 ] = dwVertPos+2;		pMesh->pIndices [ dwIndicePos+4 ] = dwVertPos+3;		pMesh->pIndices [ dwIndicePos+5 ] = dwVertPos+0;
			pMesh->pIndices [ dwIndicePos+6 ] = dwVertPos+4;		pMesh->pIndices [ dwIndicePos+7 ] = dwVertPos+5;		pMesh->pIndices [ dwIndicePos+8 ] = dwVertPos+6;
			pMesh->pIndices [ dwIndicePos+9 ] = dwVertPos+6;		pMesh->pIndices [ dwIndicePos+10 ] = dwVertPos+7;		pMesh->pIndices [ dwIndicePos+11 ] = dwVertPos+4;
			pMesh->pIndices [ dwIndicePos+12 ] = dwVertPos+8;		pMesh->pIndices [ dwIndicePos+13 ] = dwVertPos+9;		pMesh->pIndices [ dwIndicePos+14 ] = dwVertPos+10;
			pMesh->pIndices [ dwIndicePos+15 ] = dwVertPos+10;		pMesh->pIndices [ dwIndicePos+16 ] = dwVertPos+11;		pMesh->pIndices [ dwIndicePos+17 ] = dwVertPos+8;
			pMesh->pIndices [ dwIndicePos+18 ] = dwVertPos+12;		pMesh->pIndices [ dwIndicePos+19 ] = dwVertPos+13;		pMesh->pIndices [ dwIndicePos+20 ] = dwVertPos+14;
			pMesh->pIndices [ dwIndicePos+21 ] = dwVertPos+14;		pMesh->pIndices [ dwIndicePos+22 ] = dwVertPos+15;		pMesh->pIndices [ dwIndicePos+23 ] = dwVertPos+12;
			pMesh->pIndices [ dwIndicePos+24 ] = dwVertPos+16;		pMesh->pIndices [ dwIndicePos+25 ] = dwVertPos+17;		pMesh->pIndices [ dwIndicePos+26 ] = dwVertPos+18;
			pMesh->pIndices [ dwIndicePos+27 ] = dwVertPos+18;		pMesh->pIndices [ dwIndicePos+28 ] = dwVertPos+19;		pMesh->pIndices [ dwIndicePos+29 ] = dwVertPos+16;
			pMesh->pIndices [ dwIndicePos+30 ] = dwVertPos+20;		pMesh->pIndices [ dwIndicePos+31 ] = dwVertPos+21;		pMesh->pIndices [ dwIndicePos+32 ] = dwVertPos+22;
			pMesh->pIndices [ dwIndicePos+33 ] = dwVertPos+22;		pMesh->pIndices [ dwIndicePos+34 ] = dwVertPos+23;		pMesh->pIndices [ dwIndicePos+35 ] = dwVertPos+20;

			// next cube
			dwVertPos+=24;
			dwIndicePos+=36;
		}

		// add the worked mesh to buffers (unique VB/IB buffer for this new mesh)
		m_ObjectManager.AddObjectMeshToBuffers ( pMesh, true );
	}
	// assign material to new entity (may need to create some way for multiple material indexes inside single mesh?)
	//SetObjectArbitaryValue ( iObj, t.entityprofile[iEntID].materialindex ); // done per mesh now (above)

	// work out bounds/radius of entity
	CalculateObjectBounds ( iObj );

	// Apply texture plate
	//int iTexD = t.entityprofile[iEntID].texdid;
	//int iTexN = t.entityprofile[iEntID].texnid;
	//int iTexS = t.entityprofile[iEntID].texsid;
	int iTexD = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.dds")).Get());
	int iTexN = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_N.dds")).Get());
	int iTexS = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_S.dds")).Get());
	t.entityprofile[iEntID].texdid = iTexD;
	t.entityprofile[iEntID].texnid = iTexN;
	t.entityprofile[iEntID].texsid = iTexS;
	//if ( iTexD > 0 && ImageExist ( iTexD ) == 1 ) DeleteImage ( iTexD );
	//if ( iTexN > 0 && ImageExist ( iTexN ) == 1 ) DeleteImage ( iTexN );
	//if ( iTexS > 0 && ImageExist ( iTexS ) == 1 ) DeleteImage ( iTexS );
	//LoadImage ( cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.dds")).Get(), iTexD );
	//LoadImage ( cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_N.dds")).Get(), iTexN );
	//LoadImage ( cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_S.dds")).Get(), iTexS );
	TextureObject ( iObj, 0, iTexD );
	TextureObject ( iObj, 1, loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds") );
	TextureObject ( iObj, 2, iTexN );
	TextureObject ( iObj, 3, iTexS );
	TextureObject ( iObj, 4, t.terrain.imagestartindex );
	TextureObject ( iObj, 5, g.postprocessimageoffset+5 );
	TextureObject ( iObj, 6, loadinternaltexture("effectbank\\reloaded\\media\\blank_I.dds") );
	SetObjectTransparency ( iObj, 0 );

	// now apply regular entity shader (so has proper shading and can be lightmapped)
	int iEffectIndex = loadinternaleffect("effectbank\\reloaded\\entity_basic.fx");
	SetObjectEffect ( iObj, iEffectIndex );
	SetObjectMask ( iObj, 0x1+(1<<31) );

	// free resources
	SAFE_DELETE ( pMatRefTable );
}

void ebe_reset ( void )
{
	if ( t.ebe.on == 0 )
	{
		// early out if not ready for this (i.e. no init performed)
		if ( t.ebe.entityelementindex == 0 ) return;

		// Set the tool object we will be using
		ebebuild.iToolObj = g.ebeobjectbankoffset + 0;

		// reset defaults when enter build mode
		if ( t.ebe.entityelementindex != t.ebe.lastentityelementindex )
		{
			// only if we switch to a new EBE site (so can edit/leave/edit and keep settings)
			t.ebe.lastentityelementindex = t.ebe.entityelementindex;
			ebebuild.iCurrentGridLayer = 0;
			ebebuild.iCursorRotation = 0;
			ebebuild.iCurrentTexture = 0;

			// reset pattern selection to [3] FLOOR SHAPE (most common and visual to start with)
			ebe_loadpattern ( t.ebebank_s[3].Get() );
		}

		// update edit grid to correct grid height
		float fCurrentGridLayerAbsHeight = ebebuild.iCurrentGridLayer*5.0f;
		OffsetLimb ( ebebuild.iToolObj, 1, 500, fCurrentGridLayerAbsHeight + 0.05f, 500 );

		// update cursor to start when new construction reset
		ebe_settexturehighlight();

		// show if previously hidden
		ebe_show ();

		// begin tool work
		t.ebe.on = 1;
	}
}

void ebe_hardreset ( void )
{
	// full reset of editor state
	t.ebe.lastentityelementindex = -1;
	ebebuild.iCurrentGridLayer = 0;
	ebebuild.iCursorRotation = 0;
	ebebuild.iCurrentTexture = 0;

	// force reset when new site created/loaded
	t.ebe.on = 0;
}

void ebe_hide ( void )
{
	if ( t.ebe.on == 1 )
	{
		ebe_finishsite();
		if ( ebebuild.iTexturePanelSprite[0] > 0 && ebebuild.iToolObj > 0 )
		{
			for ( int n = 0; n < 16; n++ ) if ( SpriteExist ( ebebuild.iMatSpr[n] ) == 1 ) HideSprite ( ebebuild.iMatSpr[n] );
			if ( SpriteExist ( ebebuild.iEBETexHelpSpr ) == 1 ) HideSprite ( ebebuild.iEBETexHelpSpr );
			if ( SpriteExist ( ebebuild.iEBEHelpSpr ) == 1 ) HideSprite ( ebebuild.iEBEHelpSpr );
			if ( SpriteExist ( ebebuild.iTexturePanelHighSprite ) == 1 ) HideSprite ( ebebuild.iTexturePanelHighSprite );
			for ( int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++ )
			{
				if ( SpriteExist ( ebebuild.iTexturePanelSprite[iTex] ) == 1 ) HideSprite ( ebebuild.iTexturePanelSprite[iTex] );
			}
			if ( ObjectExist ( ebebuild.iToolObj ) == 1 ) HideObject ( ebebuild.iToolObj );
		}
		t.ebe.on = 0;
	}

	// always clear undo buffers
	SAFE_DELETE ( g_pUndoBufferPtr );
	g_dwUndoBufferCount = 0;
	SAFE_DELETE ( g_pRedoBufferPtr );
	g_dwRedoBufferCount = 0;
}

void ebe_show ( void )
{
	if ( ebebuild.iTexturePanelSprite[0] > 0 && ebebuild.iToolObj > 0 )
	{
		for ( int n = 0; n < 16; n++ ) if ( SpriteExist ( ebebuild.iMatSpr[n] ) == 1 ) ShowSprite ( ebebuild.iMatSpr[n] );
		if ( SpriteExist ( ebebuild.iEBETexHelpSpr ) == 1 ) ShowSprite ( ebebuild.iEBETexHelpSpr );
		if ( SpriteExist ( ebebuild.iEBEHelpSpr ) == 1 ) ShowSprite ( ebebuild.iEBEHelpSpr );
		if ( SpriteExist ( ebebuild.iTexturePanelHighSprite ) == 1 ) ShowSprite ( ebebuild.iTexturePanelHighSprite );
		for ( int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++ )
		{
			if ( SpriteExist ( ebebuild.iTexturePanelSprite[iTex] ) == 1 ) ShowSprite ( ebebuild.iTexturePanelSprite[iTex] );
		}
		if ( ObjectExist ( ebebuild.iToolObj ) == 1 ) ShowObject ( ebebuild.iToolObj );
	}
}

void ebe_newsite ( int iEntityIndex )
{
	// save any existing site
	if ( t.ebe.entityelementindex > 0 ) ebe_finishsite();

	// and assign newly added entity as site
	t.ebe.entityelementindex = iEntityIndex;

	// create new build object when triggered (t.ebe.entityelementindex)
	int iBuildObj = t.entityelement[t.ebe.entityelementindex].obj;
	int entid = t.entityelement[t.ebe.entityelementindex].bankindex;
	ebe_init_newbuild ( iBuildObj, entid );

	// reset for tool work
	ebe_reset();
}

int ebe_save ( int iEntityIndex )
{
	// local vars
	int iHaveCreatedNewItems = 0;

	// Needed locals
	LPSTR pOldDir = GetDir();
	HWND hThisWnd = GetForegroundWindow();

	// Check if EBE
	int iEntID = 0;
	bool bIsThisAnEBE = false;
	if ( iEntityIndex > 0 ) 
	{
		iEntID = t.entityelement[iEntityIndex].bankindex;
		if ( iEntID > 0 ) 
			if ( t.entityprofile[iEntID].isebe != 0 )
				bIsThisAnEBE = true;
	}
	if ( bIsThisAnEBE == false )
		return 0;

	// EBE Save Folder
	t.strwork = g.fpscrootdir_s + "\\Files\\entitybank\\user\\ebestructures";
	if ( PathExist( t.strwork.Get() ) == 0 ) 
	{
		MessageBox ( hThisWnd, "Cannot find 'entitybank\\user\\ebestructures' folder", "Error", MB_OK | MB_TOPMOST );
		return 0;
	}

	//  Ask for save filename
	cStr tSaveFile = "";
	cStr tSaveMessage = "Save EBE Structure";
	tSaveFile = openFileBox("EBE Structure (.ebe)|*.ebe|All Files|*.*|", t.strwork.Get(), tSaveMessage.Get(), ".ebe", IMPORTERSAVEFILE);
	if ( tSaveFile == "Error" )
	{
		SetDir(pOldDir);
		return 0;
	}

	// Truncate file from absolute save filename
	LPSTR pFileNameOnly = NULL;
	LPSTR pThisSaveFile = tSaveFile.Get();
	for ( int n = strlen(tSaveFile.Get())-1; n > 0; n-- )
	{
		if ( pThisSaveFile[n] == '\\' || pThisSaveFile[n] == '/' )
		{
			pFileNameOnly = pThisSaveFile + n + 1;
			break;
		}
	}

	// Check if already exists, if so, ask if should be overwritten
	if ( FileExist ( tSaveFile.Get() ) == 1 )
	{
		// Already Exists
		char pDisplayErrorMsg[512];
		if ( strlen ( pFileNameOnly ) > 482 ) pFileNameOnly = pThisSaveFile + strlen(pThisSaveFile) - 480;
		strcpy ( pDisplayErrorMsg, pFileNameOnly );
		strcat ( pDisplayErrorMsg, " already exists! Overwrite?" );
		if ( MessageBox ( hThisWnd, pDisplayErrorMsg, "File Already Exists", MB_YESNO | MB_TOPMOST ) != IDYES )
		{
			// Abort save here
			SetDir(pOldDir);
			return 0;
		}
	}
	else
	{
		// record this is a new EBE file
		iHaveCreatedNewItems = 1;

		// assign new name to entityID
		t.entityprofileheader[iEntID].desc_s = Left(pFileNameOnly,strlen(pFileNameOnly)-4);
	}

	// restore current folder
	SetDir(pOldDir);

	// Use filename to save EBE Entity to location
	ebe_save_ebefile ( tSaveFile, iEntID );

	// return 1 if have created new EBE file (will trigger icon to be added to IDE)
	return iHaveCreatedNewItems;
}

void ebe_load_ebefile ( cStr pLoadFile, int iEntID )
{
	// clear previous data, and reset for new data
	SAFE_DELETE ( t.entityprofile[iEntID].ebe.pRLEData );
	t.entityprofile[iEntID].ebe.dwRLESize = 0;
	SAFE_DELETE ( t.entityprofile[iEntID].ebe.iMatRef );
	t.entityprofile[iEntID].ebe.dwMatRefCount = 0;
	for ( int n = 0; n < t.entityprofile[iEntID].ebe.dwTexRefCount; n++ )
		SAFE_DELETE ( t.entityprofile[iEntID].ebe.pTexRef[n] );
	SAFE_DELETE ( t.entityprofile[iEntID].ebe.pTexRef );
	t.entityprofile[iEntID].ebe.dwTexRefCount = 0;

	// Use filename to save EBE Entity to location
	if ( FileExist ( pLoadFile.Get() ) == 1 )
	{
		OpenToRead ( 1, pLoadFile.Get() );
		int iVersionNumber = ReadLong ( 1 );
		if ( iVersionNumber >= 101 )
		{
			// Store X, Y, Z dimenions of grid volume
			int iCubeAreaSizeX = ReadLong ( 1 );// CUBEAREASIZE )
			int iCubeAreaSizeY = ReadLong ( 1 );// CUBEAREASIZE )
			int iCubeAreaSizeZ = ReadLong ( 1 );// CUBEAREASIZE )

			// Save cube raw data
			t.entityprofile[iEntID].ebe.dwRLESize = ReadLong ( 1 );
			t.entityprofile[iEntID].ebe.pRLEData = new DWORD[t.entityprofile[iEntID].ebe.dwRLESize];
			for ( DWORD dwPos = 0; dwPos < t.entityprofile[iEntID].ebe.dwRLESize; dwPos++ )
			{
				int iDataItem = ReadLong ( 1 );
				t.entityprofile[iEntID].ebe.pRLEData[dwPos] = *(DWORD*)&iDataItem;
			}
		}
		if ( iVersionNumber >= 102 )
		{
			// Load material references
			t.entityprofile[iEntID].ebe.dwMatRefCount = ReadLong ( 1 );
			SAFE_DELETE ( t.entityprofile[iEntID].ebe.iMatRef );
			t.entityprofile[iEntID].ebe.iMatRef = new int[t.entityprofile[iEntID].ebe.dwMatRefCount];
			for ( DWORD dwI = 0; dwI < t.entityprofile[iEntID].ebe.dwMatRefCount; dwI++ )
			{
				t.entityprofile[iEntID].ebe.iMatRef[dwI] = ReadLong ( 1 );
			}
		}
		if ( iVersionNumber >= 103 )
		{
			// Load texture references
			for ( DWORD dwI = 0; dwI < t.entityprofile[iEntID].ebe.dwTexRefCount; dwI++ ) 
				SAFE_DELETE ( t.entityprofile[iEntID].ebe.pTexRef[dwI] );
			SAFE_DELETE ( t.entityprofile[iEntID].ebe.pTexRef );
			t.entityprofile[iEntID].ebe.dwTexRefCount = ReadLong ( 1 );
			t.entityprofile[iEntID].ebe.pTexRef = new LPSTR[t.entityprofile[iEntID].ebe.dwTexRefCount];
			for ( DWORD dwI = 0; dwI < t.entityprofile[iEntID].ebe.dwTexRefCount; dwI++ )
			{
				LPSTR pString = ReadString ( 1 );
				t.entityprofile[iEntID].ebe.pTexRef[dwI] = new char[256];
				strcpy ( t.entityprofile[iEntID].ebe.pTexRef[dwI], pString );
			}
		}
		CloseFile ( 1 );
	}
}

void ebe_save_ebefile ( cStr tSaveFile, int iEntID )
{
	// Chop current directory from save file to shorten filenames (for long EBE texture name)
	LPSTR pOldDir = GetDir();
	if ( strnicmp ( tSaveFile.Get(), pOldDir, strlen(pOldDir) ) == NULL )
		tSaveFile = Right ( tSaveFile.Get(), strlen(tSaveFile.Get()) - (strlen(pOldDir)+1) );
	
	// Use filename (tSaveFile, i.e. ebe1.ebe) to save EBE Entity to location
	if ( FileExist(tSaveFile.Get()) == 1 ) DeleteAFile ( tSaveFile.Get() );
	OpenToWrite ( 1, tSaveFile.Get() );
	int iVersionNumber = 103; 
	WriteLong ( 1, iVersionNumber );
	if ( iVersionNumber >= 101 )
	{
		// Store X, Y, Z dimenions of grid volume
		WriteLong ( 1, CUBEAREASIZE );
		WriteLong ( 1, CUBEAREASIZE );
		WriteLong ( 1, CUBEAREASIZE );

		// Save cube raw data
		WriteLong ( 1, t.entityprofile[iEntID].ebe.dwRLESize );
		for ( DWORD dwPos = 0; dwPos < t.entityprofile[iEntID].ebe.dwRLESize; dwPos++ )
		{
			DWORD dwDataItem = t.entityprofile[iEntID].ebe.pRLEData[dwPos];
			WriteLong ( 1, *(int*)&dwDataItem );
		}
	}
	if ( iVersionNumber >= 102 )
	{
		// Store material references
		WriteLong ( 1, t.entityprofile[iEntID].ebe.dwMatRefCount );
		for ( DWORD dwI = 0; dwI < t.entityprofile[iEntID].ebe.dwMatRefCount; dwI++ )
		{
			if ( t.entityprofile[iEntID].ebe.iMatRef != NULL )
			{
				WriteLong ( 1, t.entityprofile[iEntID].ebe.iMatRef[dwI] );
			}
			else
			{
				WriteLong ( 1, 0 );
			}
		}
	}
	if ( iVersionNumber >= 103 )
	{
		// Store texture references
		WriteLong ( 1, t.entityprofile[iEntID].ebe.dwTexRefCount );
		for ( DWORD dwI = 0; dwI < t.entityprofile[iEntID].ebe.dwTexRefCount; dwI++ )
		{
			if ( t.entityprofile[iEntID].ebe.pTexRef[dwI] != NULL )
			{
				WriteString ( 1, t.entityprofile[iEntID].ebe.pTexRef[dwI] );
			}
			else
			{
				WriteString ( 1, "" );
			}
		}
	}
	CloseFile ( 1 );

	// Determine name part
	cStr tNameOnly;
	LPSTR pFileNameOnly = NULL;
	LPSTR pThisSaveFile = tSaveFile.Get();
	for ( int n = strlen(pThisSaveFile)-1; n > 0; n-- )
	{
		if ( pThisSaveFile[n] == '\\' || pThisSaveFile[n] == '/' )
		{
			pFileNameOnly = pThisSaveFile + n + 1;
			break;
		}
	}
	tNameOnly = Left ( pFileNameOnly, strlen(pFileNameOnly) - 4 );
	cstr tRawPathAndFile = cstr ( Left ( tSaveFile.Get(), strlen(tSaveFile.Get()) - 4 ) );
	cstr tRawPath = cstr ( Left ( tSaveFile.Get(), strlen(tSaveFile.Get()) - 4 - strlen(tNameOnly.Get()) ) );

	// Now create a real entity from it (for quickest library and level loading)
	// FPE
	Dim ( t.setuparr_s, 30 );
	t.setuparr_s[ 0]=";EBE Entity";
	t.setuparr_s[ 1]="";
	t.setuparr_s[ 2]=";Header";
	t.setuparr_s[ 3]=cstr("desc          = ") + tNameOnly;
	t.setuparr_s[ 4]="";
	t.setuparr_s[ 5]=";AI";
	t.setuparr_s[ 6]="aimain	      = default.lua";
	t.setuparr_s[ 7]="";
	t.setuparr_s[ 8]=";Orientation";
	t.setuparr_s[ 9]=cstr("model         = ") + tNameOnly + ".dbo";
	t.setuparr_s[10]=cstr("textured      = ") + t.entityprofile[iEntID].texd_s;
	t.setuparr_s[11]="effect        = effectbank\\reloaded\\entity_basic.fx";
	t.setuparr_s[12]="transparency  = 0";
	t.setuparr_s[13]="scale         = 100";
	t.setuparr_s[14]="defaultstatic = 1";
	t.setuparr_s[15]="collisionmode = 1";
	t.setuparr_s[16]="forcesimpleobstacle = 3";
	t.setuparr_s[17]=cstr("forceobstaclepolysize = ") + cstr(t.entityprofile[iEntID].forceobstaclepolysize);
	t.setuparr_s[18]="";
	t.setuparr_s[19]=";EBE Builder Extras";
	t.setuparr_s[20]="isebe         = 1";
	cstr pFPEFile = tRawPathAndFile + cstr(".fpe");
	if ( FileExist(pFPEFile.Get()) == 1 ) DeleteAFile ( pFPEFile.Get() );
	SaveArray ( pFPEFile.Get(), t.setuparr_s );
	UnDim ( t.setuparr_s );

	// BMP
	cstr tBMPFilename = "ebebank\\_builder\\EBE.bmp";
	if ( FileExist(tBMPFilename.Get()) == 1 ) 
	{
		cstr sBMPFile = tRawPathAndFile + cstr(".bmp");
		if ( FileExist(sBMPFile.Get()) == 1 ) DeleteAFile ( sBMPFile.Get() );
		CopyFile ( tBMPFilename.Get(), sBMPFile.Get(), FALSE );
	}

	// DBO
	int iSourceObj = g.entitybankoffset + iEntID;
	cstr tDBOFile = tRawPathAndFile + cstr(".dbo");
	if ( FileExist(tDBOFile.Get()) == 1 ) DeleteAFile ( tDBOFile.Get() );
	SaveObject ( tDBOFile.Get(), iSourceObj );

	// DDS
	cstr tDDSSourceRaw = cstr("ebebank\\default\\") + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
	cstr tDDSSourceFilename = tDDSSourceRaw + "_D.dds";
	tRawPathAndFile = tRawPath + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
	if ( FileExist(tDDSSourceFilename.Get()) == 0 ) 
	{
		if ( strnicmp ( tRawPath.Get(), "levelbank", 9 ) != NULL )
		{
			//tDDSSourceRaw = cstr("levelbank\\testmap\\") + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
			tDDSSourceRaw = g.mysystem.levelBankTestMap_s + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
			tDDSSourceFilename = tDDSSourceRaw + "_D.dds";
		}
	}
	if ( FileExist(tDDSSourceFilename.Get()) == 1 ) 
	{
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
		tDDSSourceFilename = tDDSSourceRaw + "_N.dds";
		sDDSFile = tRawPathAndFile + cstr("_N.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
		tDDSSourceFilename = tDDSSourceRaw + "_S.dds";
		sDDSFile = tRawPathAndFile + cstr("_S.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
	}
}

int ebe_loadcustomtexture ( int iEntityProfileIndex, int iWhichTextureOver )
{
	// Needed locals
	LPSTR pOldDir = GetDir();
	HWND hThisWnd = GetForegroundWindow();

	// must have texture plate image before we can customise it
	if ( ebebuild.iTexPlateImage == 0 )
		return 0;

	// EBE Load Folder
	t.strwork = g.fpscrootdir_s + "\\Files\\ebebank\\texturesource";
	if ( PathExist( t.strwork.Get() ) == 0 ) 
	{
		MessageBox ( hThisWnd, "Cannot find 'ebebank\texturesource' folder", "Error", MB_OK | MB_TOPMOST );
		return 0;
	}

	// final destination of Texture Plate subsets
	cstr sSavePathFile = g.fpscrootdir_s + "\\Files\\ebebank\\default\\textures";

	//  Ask for save filename
	cStr tLoadFile = "";
	cStr tLoadMessage = "Replace with custom texture";
	tLoadFile = openFileBox("Diffuse File (_D.dds)|*.dds|Texture File (.dds)|*.dds|All Files|*.*|", t.strwork.Get(), tLoadMessage.Get(), ".dds", IMPORTERSAVEFILE);
	if ( tLoadFile == "Error" )
	{
		SetDir(pOldDir);
		return 0;
	}

	// Use large prompt
	t.statusbar_s = "Generating Building Editor Texture Plate"; 
	popup_text(t.statusbar_s.Get());

	// preferred format
	GGFORMAT d3dFormat;
	GGFORMAT compressedFormat;
	#ifdef DX11
	#else
	D3DSURFACE_DESC backbufferdesc;
	g_pGlob->pHoldBackBufferPtr->GetDesc ( &backbufferdesc );
	d3dFormat = backbufferdesc.Format;
	compressedFormat = D3DFMT_DXT1;
	#endif

	// work out if _D.dds or not
	int iTexSetsPossible = 1;
	LPSTR pLoadFilename = tLoadFile.Get();
	cstr pFileExt = Right ( pLoadFilename, 6 );
	if ( stricmp ( pFileExt.Get(), "_d.dds" ) == NULL )
		iTexSetsPossible = 3;

	// go through texture subsets (D, N, S )
	for ( int iTexSet = 0; iTexSet < iTexSetsPossible; iTexSet++ )
	{
		// work out texture filename
		cstr pFilename = pLoadFilename;
		cstr pFileExt = cstr("_D.dds");
		if ( iTexSetsPossible == 3 )
		{
			pFilename = Left ( pLoadFilename, strlen(pLoadFilename)-6 );
			if ( iTexSet == 0 ) pFileExt = cstr("_D.dds");
			if ( iTexSet == 1 ) pFileExt = cstr("_N.dds");
			if ( iTexSet == 2 ) pFileExt = cstr("_S.dds");
			pFilename = pFilename + pFileExt;
		}

		// check if the texture exists, else use backup blank
		if ( FileExist ( pFilename.Get() ) == 0 )
		{
			// file not exist, if N or S, substitute with blank
			if ( iTexSet == 1 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_N.dds";
			if ( iTexSet == 2 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_black.dds";
		}

		// check if texture to load exists
		#ifdef DX11
		// use terrain custom texture maker for inserting texture in textureplate
		cstr pPlateFilename = cstr(sSavePathFile) + pFileExt;
		terrain_createnewterraintexture ( pPlateFilename.Get(), iWhichTextureOver, pFilename.Get(), 1, 1 );

		// reload image with new file (apply auto-mipmapping when load)
		if ( iTexSet == 0 ) 
		{
			// texture used by texture plate
			LoadImage ( pPlateFilename.Get(), ebebuild.iTexPlateImage,0,g.gdividetexturesize);
			TextureObject ( ebebuild.iBuildObj, 0, ebebuild.iTexPlateImage );
			TextureObject ( ebebuild.iBuildObj, 1, ebebuild.iTexPlateImage );
		}
		#else
		GGIMAGE_INFO finfo;
		LPDIRECT3DSURFACE9 pLoadedTexSurface = NULL;
		HRESULT hRes = D3DXGetImageInfoFromFile( pFilename.Get(), &finfo );
		if ( hRes == S_OK )
		{
			// file exists, use the provided _N or _S texture file
		}
		else
		{
			// file not exist, if N or S, substitute with blank
			if ( iTexSet == 1 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_N.dds";
			if ( iTexSet == 2 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_black.dds";
			hRes = D3DXGetImageInfoFromFile( pFilename.Get(), &finfo );
		}

		// create and load the texture selected
		if ( hRes == S_OK )
		{
			hRes = m_pD3D->CreateRenderTarget( finfo.Width, finfo.Height, d3dFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &pLoadedTexSurface, NULL);
			hRes = D3DXLoadSurfaceFromFile( pLoadedTexSurface, NULL, NULL, pFilename.Get(), NULL, D3DX_FILTER_POINT, 0, &finfo );

			// create and load the texture plate surface
			LPDIRECT3DTEXTURE9 pTextureDDS;
			LPDIRECT3DSURFACE9 pPlateSurface = NULL;
			cstr pPlateFilename = cstr(sSavePathFile) + pFileExt;
			hRes = D3DXGetImageInfoFromFile( pPlateFilename.Get(), &finfo );
			m_pD3D->CreateTexture ( finfo.Width, finfo.Height, 1, 0, d3dFormat, D3DPOOL_MANAGED, &pTextureDDS, NULL );
			if ( pTextureDDS )
			{
				// copy texture to DDS compressed texture
				pTextureDDS->GetSurfaceLevel ( 0, &pPlateSurface );
				if ( pPlateSurface )
				{
					hRes = D3DXLoadSurfaceFromFile( pPlateSurface, NULL, NULL, pPlateFilename.Get(), NULL, D3DX_FILTER_POINT, 0, &finfo );
				}
			}

			// get surface of current texture plate
			if ( pLoadedTexSurface && pPlateSurface ) 
			{
				// work out exact offset to slot position
				int iRow = iWhichTextureOver / 4;
				int iCol = iWhichTextureOver - (iRow*4);
				int iTexSlotOffsetX = iCol * 1024;
				int iTexSlotOffsetY = iRow * 1024;
				RECT rcPlate = RECT();
				rcPlate.left = iTexSlotOffsetX; rcPlate.top = iTexSlotOffsetY; rcPlate.right = iTexSlotOffsetX+1024; rcPlate.bottom = iTexSlotOffsetY+1024;

				// paste to fill 1024x1024 initially (to get at corners)
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);

				// paste surface so smaller 1022x1022 texture can seamlessly wrap
				int iX = 1, iY = 0;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
				iX = 1, iY = 2;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
				iX = 0, iY = 1;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
				iX = 2, iY = 1;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
	
				// then paste into surface at 1022x1022 (so can have seamless textures within atlas)
				rcPlate.left = iTexSlotOffsetX+1; rcPlate.top = iTexSlotOffsetY+1;
				rcPlate.right = iTexSlotOffsetX+1023; rcPlate.bottom = iTexSlotOffsetY+1023;
				hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);

				// and finally copy back to LoadTexture
				SAFE_RELEASE ( pLoadedTexSurface );
				hRes = m_pD3D->CreateRenderTarget( finfo.Width, finfo.Height, d3dFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &pLoadedTexSurface, NULL);
				hRes = D3DXLoadSurfaceFromSurface(pLoadedTexSurface, NULL, NULL, pPlateSurface, NULL, NULL, D3DX_DEFAULT, 0);

				// now create the compressed surface for the save
				SAFE_RELEASE ( pPlateSurface );
				SAFE_RELEASE ( pTextureDDS );
				D3DFORMAT d3dChoice = compressedFormat; if ( iTexSet == 1 ) d3dChoice = d3dFormat;
				m_pD3D->CreateTexture ( finfo.Width, finfo.Height, 0, D3DUSAGE_AUTOGENMIPMAP, d3dChoice, D3DPOOL_MANAGED, &pTextureDDS, NULL );
				if ( pTextureDDS )
				{
					// copy texture to DDS compressed texture
					pTextureDDS->GetSurfaceLevel ( 0, &pPlateSurface );
					if ( pPlateSurface )
					{
						hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, NULL, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
					}
					pTextureDDS->GenerateMipSubLevels();
				}

				// save new texture surface out
				D3DXIMAGE_FILEFORMAT DestFormat = D3DXIFF_DDS;
				cstr pSaveLocation = cstr(sSavePathFile) + pFileExt;
				hRes = D3DXSaveSurfaceToFile( pSaveLocation.Get(), DestFormat, pPlateSurface, NULL, NULL );
				if ( FAILED ( hRes ) )
				{
					char pStrClue[512];
					wsprintf ( pStrClue, "Failed to save new custom texture plate: %s", pFilename.Get() );
					RunTimeError(RUNTIMEERROR_IMAGEERROR,pStrClue);
					SAFE_RELEASE(pLoadedTexSurface);
					SAFE_RELEASE(pPlateSurface);
					return 0;
				}

				// and the PNG for debugging
				if ( 0 )
				{
					DestFormat = D3DXIFF_PNG;
					cstr pFilePNGExt;
					if ( iTexSet == 0 ) pFilePNGExt = cstr("_D.png");
					if ( iTexSet == 1 ) pFilePNGExt = cstr("_N.png");
					if ( iTexSet == 2 ) pFilePNGExt = cstr("_S.png");
					pSaveLocation = cstr(sSavePathFile) + pFilePNGExt;
					hRes = D3DXSaveSurfaceToFile( pSaveLocation.Get(), DestFormat, pPlateSurface, NULL, NULL );
				}
			}

			// free temp surface captures
			SAFE_RELEASE(pPlateSurface);
			SAFE_RELEASE(pTextureDDS);
			SAFE_RELEASE(pLoadedTexSurface);

			// reload image with new file (apply auto-mipmapping when load)
			cstr pRefreshTexturesFileName = cstr(sSavePathFile) + pFileExt;
			if ( iTexSet == 0 ) 
			{
				// texture used by texture plate
				LoadImage ( pRefreshTexturesFileName.Get(), ebebuild.iTexPlateImage );
				TextureObject ( ebebuild.iBuildObj, 0, ebebuild.iTexPlateImage );
				TextureObject ( ebebuild.iBuildObj, 1, ebebuild.iTexPlateImage );

				// diffuse texture (260317 - this crashed when two EBE entities of same texture in scene)
				//int iTexIndex = t.entityprofile[iEntityProfileIndex].texdid;
				//if ( iTexIndex > 0 ) LoadImage ( pRefreshTexturesFileName.Get(), iTexIndex );
			}
			else
			{
				/*
				// normal and specular (260317 - this crashed when two EBE entities of same texture in scene)
				int iTexIndex = t.entityprofile[iEntityProfileIndex].texnid;
				if ( iTexSet == 2 ) iTexIndex = t.entityprofile[iEntityProfileIndex].texsid;
				if ( iTexIndex > 0 )
				{
					LoadImage ( pRefreshTexturesFileName.Get(), iTexIndex );
					if ( iTexSet == 1 ) TextureObject ( ebebuild.iBuildObj, 2, iTexIndex );
					if ( iTexSet == 2 ) TextureObject ( ebebuild.iBuildObj, 3, iTexIndex );
				}
				*/
			}
			
			//strangely changing diffuse and normal (maybe old code) - moved correct line further up
			//TextureObject ( ebebuild.iBuildObj, 1, ebebuild.iTexPlateImage );
			//TextureObject ( ebebuild.iBuildObj, 2, ebebuild.iTexPlateImage );
		}
		#endif
	}

	// write chosen texture to texture reference
	char pNameOnly[256];
	char pFilenameAndPath[256];
	strcpy ( pFilenameAndPath, tLoadFile.Get() );
	for ( int n = strlen(pFilenameAndPath); n > 0; n-- )
	{
		if ( pFilenameAndPath[n] == '\\' || pFilenameAndPath[n] == '/' )
		{
			strcpy ( pNameOnly, pFilenameAndPath + n + 1 );
			break;
		}
	}
	ebebuild.TXP.sTextureFile[iWhichTextureOver] = pNameOnly;

	// restore current folder
	SetDir(pOldDir);

	// Clear status Text
	t.statusbar_s = ""; popup_text_close();

	// success
	return 1;
}

void ebe_finishsite ( void )
{
	// ensure TXP profile is saved
	ebe_savetxp(cstr(cstr("ebebank\\default\\")+cstr("textures_profile.txp")).Get());

	// save data created into entity that holds it (t.ebe.entityelementindex)
	if ( t.ebe.entityelementindex > 0 )
	{
		// latest cube data back into entity element
		int entid = t.entityelement[t.ebe.entityelementindex].bankindex;
		if ( entid > 0 )
		{
			// write ebe structure to the entity profile
			DWORD dwRLESize = t.entityprofile[entid].ebe.dwRLESize;
			ebe_packsite ( &t.entityprofile[entid].ebe.dwRLESize, &t.entityprofile[entid].ebe.pRLEData );

			// write TXP profile to the entity profile
			t.entityprofile[entid].ebe.dwMatRefCount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
			SAFE_DELETE ( t.entityprofile[entid].ebe.iMatRef );
			t.entityprofile[entid].ebe.iMatRef = new int[t.entityprofile[entid].ebe.dwMatRefCount];
			for ( int n = 0; n < t.entityprofile[entid].ebe.dwMatRefCount; n++ )
				t.entityprofile[entid].ebe.iMatRef[n] = ebebuild.TXP.iMaterialRef[n];

			// write texture references to the entity profile
			for ( int n = 0; n < t.entityprofile[entid].ebe.dwTexRefCount; n++ )
				SAFE_DELETE ( t.entityprofile[entid].ebe.pTexRef[n] );
			SAFE_DELETE ( t.entityprofile[entid].ebe.pTexRef );
			t.entityprofile[entid].ebe.dwTexRefCount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
			t.entityprofile[entid].ebe.pTexRef = new LPSTR[t.entityprofile[entid].ebe.dwTexRefCount];
			for ( int n = 0; n < t.entityprofile[entid].ebe.dwTexRefCount; n++ )
			{
				t.entityprofile[entid].ebe.pTexRef[n] = new char[256];
				strcpy ( t.entityprofile[entid].ebe.pTexRef[n], ebebuild.TXP.sTextureFile[n].Get() );
			}

			// write new texture path and name for this entity
			t.entityprofile[entid].texpath_s = "ebebank\\default\\";
			t.entityprofile[entid].texd_s = ebe_constructlongTXPname("_D.dds");
			cstr tthistexdir_s = t.entityprofile[entid].texpath_s + t.entityprofile[entid].texd_s;
			if ( t.entityprofile[entid].transparency == 0 ) 
				t.entityprofile[entid].texdid = loadinternaltextureex(tthistexdir_s.Get(),1,0);
			else
				t.entityprofile[entid].texdid = loadinternaltextureex(tthistexdir_s.Get(),5,0);
			tthistexdir_s = t.entityprofile[entid].texpath_s + ebe_constructlongTXPname("_N.dds");
			t.entityprofile[entid].texnid = loadinternaltextureex(tthistexdir_s.Get(),5,0);
			tthistexdir_s = t.entityprofile[entid].texpath_s + ebe_constructlongTXPname("_S.dds");
			t.entityprofile[entid].texsid = loadinternaltextureex(tthistexdir_s.Get(),1,0);

			// recreate entity using optimized polygons
			ebe_optimize_e();
		}

		// finished with entity
		t.ebe.entityelementindex = 0;
	}
}

void ebe_packsite ( DWORD* pdwRLEPos, DWORD** ppRLEData )
{
	// delete any previous site data
	if ( *ppRLEData != NULL ) { delete *ppRLEData; }

	// use RLE to scan cube site and pack into small memory footprint
	*pdwRLEPos = 0;
	*ppRLEData = NULL;
	for ( int iPass = 0; iPass < 2; iPass++ )
	{
		DWORD dwCount = 0;
		unsigned char lastItem = 0;
		if ( iPass == 1 ) { *ppRLEData = new DWORD[*pdwRLEPos]; }
		*pdwRLEPos = 0;
		for ( int x = 0; x < CUBEAREASIZE; x++ )
		{
			for ( int y = 0; y < CUBEAREASIZE; y++ )
			{
				for ( int z = 0; z < CUBEAREASIZE; z++ )
				{
					if ( pCubes[x][y][z] != lastItem )
					{
						if ( iPass == 1 ) { (*ppRLEData)[*pdwRLEPos] = dwCount; }
						(*pdwRLEPos)++;
						if ( iPass == 1 ) { (*ppRLEData)[*pdwRLEPos] = lastItem; }
						(*pdwRLEPos)++;
						dwCount = 0;
					}
					lastItem = pCubes[x][y][z];
					dwCount++;
				}
			}
		}
	}
}

void ebe_unpacksite ( DWORD dwRLESize, DWORD* pRLEData )
{
	int x = 0, y = 0, z = 0;
	memset ( pCubes, 0, sizeof(pCubes) );
	for ( DWORD dwPos = 0; dwPos < dwRLESize; dwPos+=2 )
	{
		DWORD dwCount = pRLEData[dwPos+0];
		unsigned char bItem = pRLEData[dwPos+1];
		for ( int n = 0; n < dwCount; n++ )
		{
			pCubes[x][y][z] = bItem;
			z++;
			if ( z >= CUBEAREASIZE )
			{
				z = 0;
				y++;
				if ( y >= CUBEAREASIZE )
				{
					y = 0;
					x++;
					if ( x >= CUBEAREASIZE )
					{
						// should reach end of RLE sequence here
						x = 0; // resetting X to prevent overflow!
					}
				}
			}
		}
	}
}

void ebe_loadtxp ( LPSTR pTXPFilename )
{
	// default pattern is single cube
	ebebuild.TXP.iWidth = 4;
	ebebuild.TXP.iHeight = 4;
	for ( int n = 0; n < 64; n++ )
	{
		ebebuild.TXP.sTextureFile[n] = "concretelight_d.dds";
		ebebuild.TXP.iMaterialRef[n] = 0;
	}
	ebebuild.TXP.sTextureFile[0] = "concretelight_d.dds"; ebebuild.TXP.iMaterialRef[0] = 1;
	ebebuild.TXP.sTextureFile[1] = "rock_d.dds"; ebebuild.TXP.iMaterialRef[1] = 1;
	ebebuild.TXP.sTextureFile[2] = "rusty_d.dds"; ebebuild.TXP.iMaterialRef[2] = 2;
	ebebuild.TXP.sTextureFile[3] = "planks_d.dds"; ebebuild.TXP.iMaterialRef[3] = 3;

	// if TXP file exists, replace above pattern
	if ( FileExist(pTXPFilename) == 1 ) 
	{
		Dim ( t.data_s, 2000 );
		LoadArray ( pTXPFilename, t.data_s );
		for ( t.l = 0; t.l <= 1999; t.l++ )
		{
			t.line_s=t.data_s[t.l];
			if (  Len(t.line_s.Get())>0 ) 
			{
				if (  t.line_s.Get()[0] != ';' ) 
				{
					//  take fieldname and value
					for ( t.c = 0 ; t.c < Len(t.line_s.Get()); t.c++ )
					{
						if ( t.line_s.Get()[t.c] == '=' ) 
						{ 
							t.mid = t.c+1  ; break;
						}
					}
					t.field_s=cstr(Lower(removeedgespaces(Left(t.line_s.Get(),t.mid-1))));
					t.value_s=cstr(removeedgespaces(Right(t.line_s.Get(),Len(t.line_s.Get())-t.mid)));
					for ( t.c = 0 ; t.c < Len(t.value_s.Get()); t.c++ )
					{
						if (  t.value_s.Get()[t.c] == ',' ) 
						{ 
							t.mid = t.c+1 ; break; 
						}
					}
					t.value1=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value1_f=ValF(removeedgespaces(Left(t.value_s.Get(),t.mid-1)));
					t.value2_s=cstr(removeedgespaces(Right(t.value_s.Get(),Len(t.value_s.Get())-t.mid)));
					if ( Len(t.value2_s.Get())>0  ) t.value2 = ValF(t.value2_s.Get()); else t.value2 = -1;

					// extract field data from file
					t.tryfield_s="width"; if ( t.field_s == t.tryfield_s  )  ebebuild.TXP.iWidth = t.value1;
					t.tryfield_s="height"; if ( t.field_s == t.tryfield_s  ) ebebuild.TXP.iHeight = t.value1;

					// filename strings and material references
					int ncount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
					if ( ncount > 0 )
					{
						for ( int n = 0; n < ncount; n++ )
						{
							// Texture filename
							cstr sNum = cstr(100+n);
							t.tryfield_s = cStr("t") + cStr(Right(sNum.Get(),strlen(sNum.Get())-1));
							if ( t.field_s == t.tryfield_s  ) ebebuild.TXP.sTextureFile[n] = Lower(t.value_s.Get());

							// Material Index
							t.tryfield_s = cStr("m") + cStr(Right(sNum.Get(),strlen(sNum.Get())-1));
							if ( t.field_s == t.tryfield_s  ) ebebuild.TXP.iMaterialRef[n] = t.value1;
						}
					}
				}
			}
		}
		UnDim (  t.data_s );
	}
}

cstr ebe_constructlongTXPname ( LPSTR pExt )
{
	cstr sLongFilename = "";
	/* created files TOO LARGE!
	int ncount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
	if ( ncount > 0 )
	{
		for ( int n = 0; n < ncount; n++ )
		{
			cstr sChop = Left(ebebuild.TXP.sTextureFile[n].Get(),strlen(ebebuild.TXP.sTextureFile[n].Get())-4);
			sChop = Left(sChop.Get(),15);
			sLongFilename = sLongFilename + sChop;
		}
	}
	sLongFilename = sLongFilename + pExt;
	*/
	int ncount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
	if ( ncount > 0 )
	{
		LONGLONG lValue = 0;
		for ( int n = 0; n < ncount; n++ )
		{
			cstr sEncode = Upper(Left(ebebuild.TXP.sTextureFile[n].Get(),strlen(ebebuild.TXP.sTextureFile[n].Get())-4));
			char pEncodeString[512];
			strcpy ( pEncodeString, sEncode.Get() );
			LONGLONG lScale = 1;
			for ( int n = 0; n < strlen(pEncodeString); n++ )
			{
				int charnum = pEncodeString[n] - ' ';
				lValue = lValue + (charnum*lScale);
				lScale = lScale * 100;
			}
		}
		char num[1024];
		sprintf ( num, "%llu", lValue );
		sLongFilename = sLongFilename + num;
	}
	sLongFilename = sLongFilename + pExt;
	return sLongFilename;
}

void ebe_savetxp ( LPSTR pTXPFilename )
{
	// Calc info
	int ncount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;

	// save two files, the editors TXP and the one that represents this arrangement uniquely
	for ( int iFilePass = 0; iFilePass < 2; iFilePass++ )
	{
		char pFileToSave[1024];
		strcpy ( pFileToSave, pTXPFilename );
		if ( iFilePass == 1 ) 
		{
			cstr sLongTXPFilename = cstr("ebebank\\default\\") + ebe_constructlongTXPname(".txp");
			strcpy ( pFileToSave, sLongTXPFilename.Get() ); 
		}

		// Save TXP file out
		int iLine = 0;
		Dim ( t.setuparr_s, 100 );
		t.setuparr_s[iLine]=";Texture Plate Dimensions"; iLine++;
		t.setuparr_s[iLine]=cstr("Width = ") + ebebuild.TXP.iWidth; iLine++;
		t.setuparr_s[iLine]=cstr("iHeight = ") + ebebuild.TXP.iHeight; iLine++;
		t.setuparr_s[iLine]=""; iLine++;
		t.setuparr_s[iLine]=";Texture Plate Files"; iLine++;
		for ( int n = 0; n < ncount; n++ ) 
		{
			cstr sNum = cstr(100+n);
			t.setuparr_s[iLine]=cstr("T") + cstr(Right(sNum.Get(),strlen(sNum.Get())-1)) + cstr(" = ") + ebebuild.TXP.sTextureFile[n]; 
			iLine++; 
		}
		t.setuparr_s[iLine]=""; iLine++;
		t.setuparr_s[iLine]=";Texture Plate Materials"; iLine++;
		for ( int n = 0; n < ncount; n++ ) 
		{
			cstr sNum = cstr(100+n);
			t.setuparr_s[iLine]=cstr("M") + cstr(Right(sNum.Get(),strlen(sNum.Get())-1)) + cstr(" = ") + ebebuild.TXP.iMaterialRef[n]; 
			iLine++; 
		}
		cstr pTXPFile = cstr(pFileToSave);
		if ( FileExist(pTXPFile.Get()) == 1 ) DeleteAFile ( pTXPFile.Get() );
		SaveArray ( pTXPFile.Get(), t.setuparr_s );
	}
	UnDim ( t.setuparr_s );

	// Also save snapshot of latest textures_DNS
	cstr sLongFilename = cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.dds");
	cstr tRawPathAndFile = cstr(Left(sLongFilename.Get(),strlen(sLongFilename.Get())-6));
	cstr tDDSFilename = "ebebank\\default\\textures_D.dds";
	if ( FileExist(tDDSFilename.Get()) == 1 ) 
	{
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
		tDDSFilename = "ebebank\\default\\textures_N.dds";
		sDDSFile = tRawPathAndFile + cstr("_N.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
		tDDSFilename = "ebebank\\default\\textures_S.dds";
		sDDSFile = tRawPathAndFile + cstr("_S.dds");
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFile ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
	}
}

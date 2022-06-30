//----------------------------------------------------
//--- GAMEGURU - M-EBE
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

#ifdef ENABLEIMGUI
//PE: GameGuru IMGUI.
#include "..\Imgui\imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "..\Imgui\imgui_internal.h"
#include "..\Imgui\imgui_impl_win32.h"
#include "..\Imgui\imgui_gg_dx11.h"
#include <algorithm>
#include <string>
#include <time.h>
#endif

// Enums (duplicated, need to clean this up)
enum MaterialComponentTEXTURESLOT
{
	BASECOLORMAP,
	NORMALMAP,
	SURFACEMAP,
	EMISSIVEMAP,
	DISPLACEMENTMAP,
	OCCLUSIONMAP,
	TRANSMISSIONMAP,
	SHEENCOLORMAP,
	SHEENROUGHNESSMAP,
	CLEARCOATMAP,
	CLEARCOATROUGHNESSMAP,
	CLEARCOATNORMALMAP,
	TEXTURESLOT_COUNT
};

#ifdef VRTECH
bool dbo2xConvert(LPSTR pFilefrom, LPSTR pFileto);
#endif

void set_inputsys_mclick(int value);

// EBE Globals (8MB)
#define CUBEAREASIZE 200
#define CUBEMAXMESH 8000
unsigned char pCubes[CUBEAREASIZE][CUBEAREASIZE][CUBEAREASIZE];
unsigned char pTemp[CUBEAREASIZE][CUBEAREASIZE][CUBEAREASIZE];
DWORD pVertCountStore[CUBEMAXMESH];
bool pbTriggerDrawBufferCreation[CUBEMAXMESH];
short pMasterGridMeshRef[20][20][20][2];

#ifdef VRTECH
char ActiveEBEFilename[260] = { "\0" };
#endif

#if defined(ENABLEIMGUI)
extern bool bBuilder_Properties_Window;
int texture_set_selection = 0;
char structure_name[MAX_PATH];
extern float fPropertiesColoumWidth;
extern int grideleprof_uniqui_id;
extern bool bForceKey;
extern  cstr csForceKey;
extern int iForceScancode;
int delay_execute = 0;
int skib_frames_execute = 0;
char BuilderPath[MAX_PATH] = "entitybank\\user\\ebestructures";
extern bool bTriggerMessage;
extern char cTriggerMessage[MAX_PATH];
extern preferences pref;
#endif


#ifdef VRTECH
bool bDisableAllSprites = true;
int iPaintMode = 1;
#else
bool bDisableAllSprites = false;
#endif

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

//PE: Mesh - Vertex per material has been secured to not allow more then 63135 per mesh, so we can increase MAX Cubes.
#define CUBECOLBOXMAX 5000
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
	pastebitmapfontcenterwithboxout("PREPARING STRUCTURE EDITOR SYSTEM",GetChildWindowWidth()/2,40,1,255); Sync();

	// also create destination folder
	char pEBEWriteFolder[MAX_PATH];
	strcpy_s(pEBEWriteFolder, g.fpscrootdir_s.Get());
	strcat_s(pEBEWriteFolder, MAX_PATH, "\\Files\\ebebank\\default\\"); 
	GG_CreatePath(pEBEWriteFolder);

	// Create a simple floor
	unsigned char cTexIndex = 0;

	// Create texture selection panel
	#ifdef VRTECH
	#else
	ebebuild.iTexPlateImage = 0;
	#endif
	ebebuild.iTexturePanelX = GetChildWindowWidth()-210;
	ebebuild.iTexturePanelY = GetChildWindowHeight()-210;
	ebebuild.iTexturePanelWidth = 200;
	ebebuild.iTexturePanelHeight = 200;
	image_setlegacyimageloading(true);
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
		#ifdef VRTECH
		if ( iTex==5 ) { pTexImg = "textures_D.jpg"; }
		#else
		if ( iTex==5 ) { pTexImg = "textures_D.dds"; }
		#endif
		ebebuild.iTexturePanelSprite[iTex] = g.ebeinterfacesprite + 31 + iTex;
		ebebuild.iTexturePanelImg[iTex] = loadinternalimage(cstr(cstr("ebebank\\default\\")+cstr(pTexImg)).Get());
		if(!bDisableAllSprites)
			Sprite ( ebebuild.iTexturePanelSprite[iTex], iX, iY, ebebuild.iTexturePanelImg[iTex] );
		if (!bDisableAllSprites) SizeSprite ( ebebuild.iTexturePanelSprite[iTex], iWidth, iHeight );
		if ( iTex==5 ) ebebuild.iTexPlateImage = ebebuild.iTexturePanelImg[iTex];
	}
	image_setlegacyimageloading(false);

	// Texture highlighter
	ebebuild.iTexturePanelHighSprite = g.ebeinterfacesprite + 0;
	ebebuild.iTexturePanelHighImg = loadinternalimage("ebebank\\default\\TextureHighlighter.dds");
	if (!bDisableAllSprites) Sprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelX, ebebuild.iTexturePanelY, ebebuild.iTexturePanelHighImg );
	if (!bDisableAllSprites) SizeSprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelWidth/4, ebebuild.iTexturePanelHeight/4 );

	// Help Dialog Shortcut Keys
	ebebuild.iEBEHelpSpr = g.ebeinterfacesprite + 1;
	ebebuild.iEBEHelpImg = loadinternalimage("languagebank\\english\\artwork\\ebe-help.png");
	if (!bDisableAllSprites) Sprite ( ebebuild.iEBEHelpSpr, ebebuild.iTexturePanelX - ImageWidth(ebebuild.iEBEHelpImg) - 10, ebebuild.iTexturePanelY + 210 - ImageHeight(ebebuild.iEBEHelpImg), ebebuild.iEBEHelpImg );

	// Help Dialog Shortcut Keys
	ebebuild.iEBETexHelpSpr = g.ebeinterfacesprite + 2;
    ebebuild.iEBETexHelpImg = loadinternalimage("languagebank\\english\\artwork\\ebe-texturehelp.png");
	if (!bDisableAllSprites) Sprite ( ebebuild.iEBETexHelpSpr, ebebuild.iTexturePanelX - 10, ebebuild.iTexturePanelY - 10 - ImageHeight(ebebuild.iEBETexHelpImg), ebebuild.iEBETexHelpImg );

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
			if (!bDisableAllSprites) Sprite ( ebebuild.iMatSpr[n], ebebuild.iTexturePanelX + 36 + (x*50), ebebuild.iTexturePanelY + 36 + (y*50), ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[n]] );
			if (!bDisableAllSprites) SizeSprite ( ebebuild.iMatSpr[n], 13, 13 );
			n++;
		}
	}

	// Create Building Site Tool Object
	int iToolObj = g.ebeobjectbankoffset + 0;
	if (ObjectExist(iToolObj) == 1) DeleteObject(iToolObj);
	MakeObjectPlane ( iToolObj, 100, 100 );
	if (GetMeshExist(g.meshgeneralwork) == 1) DeleteMesh(g.meshgeneralwork);
	MakeMeshFromObject ( g.meshgeneralwork, iToolObj );
	DeleteObject ( iToolObj );
	MakeObjectCube ( iToolObj, 5.0f );
	if (GetMeshExist(g.meshebe1) == 1) DeleteMesh(g.meshebe1);
	MakeMeshFromObject ( g.meshebe1, iToolObj );
	DeleteObject ( iToolObj );
	MakeObjectBox ( iToolObj, 1000, 0.1f, 1000 );
	if (GetMeshExist(g.meshebe) == 1) DeleteMesh(g.meshebe);
	MakeMeshFromObject ( g.meshebe, iToolObj );

	// Grid Limbs (0-base grid, 1-floating grid for current edit layer)
	image_setlegacyimageloading(true);
	int iGridImg = loadinternaltexture("ebebank\\default\\GridBox.dds");
	int iFloatImg = loadinternaltexture("ebebank\\default\\FloatBox.dds");
	int iCursorImg = loadinternaltexture("ebebank\\default\\CursorBox.dds");	
	image_setlegacyimageloading(false);
	int iLimbIndex = 0;
	OffsetLimb ( iToolObj, iLimbIndex, 500, 0.05, 500 );
	iLimbIndex = 1;
	AddLimb ( iToolObj, iLimbIndex, g.meshebe );
	OffsetLimb ( iToolObj, iLimbIndex, 500, 0.05, 500 );

	// now scale present meshes so grid creates a mirror effect of UVs for good textured grid
	ScaleObjectTexture ( iToolObj, 200, 200 );

	// Cursor Limb (2-can be used to show current mouse cursor within grid)
	iLimbIndex = 2;
	AddLimb ( iToolObj, iLimbIndex, g.meshebe1 );
	OffsetLimb ( iToolObj, iLimbIndex, 2.5f, 2.5f, 2.5f );

	// Replace default texture with grid textures
	TextureLimbStage ( iToolObj, 0, 0, iGridImg );
	TextureLimbStage ( iToolObj, 1, 0, iFloatImg );
	TextureLimbStage ( iToolObj, 2, 0, iCursorImg );
	SetObjectTransparency ( iToolObj, 6 );
	DisableObjectZWrite ( iToolObj );

	// Apply entity shader
	int iToolEffectIndex = loadinternaleffect("effectbank\\reloaded\\ebe_basic.fx");
	SetObjectEffect ( iToolObj, iToolEffectIndex );
	SetObjectMask ( iToolObj, 0x1 );

	// Create invisible obj to cast for object selection detection
	ebebuild.iCasterObj = g.ebeobjectbankoffset + 1;
	if (ObjectExist(ebebuild.iCasterObj) == 1) DeleteObject(ebebuild.iCasterObj);
	MakeObjectCube ( ebebuild.iCasterObj, 1 );
	SetObjectMask ( ebebuild.iCasterObj, 0x1 );
	HideObject ( ebebuild.iCasterObj );
	
	// New template has marker in limb 2, regular EBE structure has it in zero
	int iLimbWithMarker = 0;
	if ( stricmp ( Right ( t.entitybank_s[iEntID].Get(), 21), "_builder\\New Site.fpe" ) == NULL )
		iLimbWithMarker = 2;

	// Create ebe marker mesh from first 
	if (GetMeshExist(g.meshebemarker) == 1) DeleteMesh(g.meshebemarker);
	MakeMeshFromLimb ( g.meshebemarker, BuildObj, iLimbWithMarker );

	#if defined(ENABLEIMGUI)
	image_setlegacyimageloading(true);
	if (!ImageExist(EBE_CONTROL1))
		LoadImage("editors\\uiv3\\ebe-control1.png", EBE_CONTROL1);
	if (!ImageExist(EBE_CONTROL2))
		LoadImage("editors\\uiv3\\ebe-control2.png", EBE_CONTROL2);
	if (!ImageExist(EBE_CONTROL3))
		LoadImage("editors\\uiv3\\ebe-control3.png", EBE_CONTROL3);
	if (!ImageExist(EBE_CONTROL4))
		LoadImage("editors\\uiv3\\ebe-control4.png", EBE_CONTROL4);
	if (!ImageExist(EBE_CONTROL5))
		LoadImage("editors\\uiv3\\ebe-control5.png", EBE_CONTROL5);
	if (!ImageExist(EBE_CONTROL6))
		LoadImage("editors\\uiv3\\ebe-control6.png", EBE_CONTROL6);
	if (!ImageExist(EBE_THUMB))
		LoadImage("editors\\uiv3\\ebe-thumb.png", EBE_THUMB);
	image_setlegacyimageloading(false);
	#endif


	// mark EBE has intialised
	t.ebe.active = 1;
}

#ifdef VRTECH
void ebe_free(void)
{
	// delete all resources created in above _init
	for ( int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++ )
	{
		if (ebebuild.iTexturePanelImg[iTex] > 0)
		{
			if (ImageExist(ebebuild.iTexturePanelImg[iTex]) == 1)
			{
				removeinternalimage(ebebuild.iTexturePanelImg[iTex]);
			}
			ebebuild.iTexturePanelImg[iTex] = 0;
		}
		if (SpriteExist(ebebuild.iTexturePanelSprite[iTex]) == 1)
		{
			DeleteSprite(ebebuild.iTexturePanelSprite[iTex]);
		}
		ebebuild.iTexturePanelSprite[iTex] = 0;
	}
	ebebuild.iTexPlateImage = 0;

	if (ebebuild.iTexturePanelHighSprite > 0) if (SpriteExist(ebebuild.iTexturePanelHighSprite) == 1) DeleteSprite(ebebuild.iTexturePanelHighSprite);
	if (ebebuild.iTexturePanelHighImg > 0) if (ImageExist(ebebuild.iTexturePanelHighImg) == 1) removeinternalimage(ebebuild.iTexturePanelHighImg);
	ebebuild.iTexturePanelHighSprite = 0;
	ebebuild.iTexturePanelHighImg = 0;

	if (ebebuild.iEBEHelpSpr > 0) if (SpriteExist(ebebuild.iEBEHelpSpr) == 1) DeleteSprite(ebebuild.iEBEHelpSpr);
	if (ebebuild.iEBEHelpImg > 0) if (ImageExist(ebebuild.iEBEHelpImg) == 1) removeinternalimage(ebebuild.iEBEHelpImg);
	ebebuild.iEBEHelpSpr = 0;
	ebebuild.iEBEHelpImg = 0;

	if (ebebuild.iEBETexHelpSpr > 0) if (SpriteExist(ebebuild.iEBETexHelpSpr) == 1) DeleteSprite(ebebuild.iEBETexHelpSpr);
	if (ebebuild.iEBETexHelpImg > 0) if (ImageExist(ebebuild.iEBETexHelpImg) == 1) removeinternalimage(ebebuild.iEBETexHelpImg);
	ebebuild.iEBETexHelpSpr = 0;
	ebebuild.iEBETexHelpImg = 0;

	for ( int n = 0; n <= 18; n++ )
	{
		if ( ebebuild.iMatImg[n]> 0) if (ImageExist(ebebuild.iMatImg[n]) == 1) removeinternalimage(ebebuild.iMatImg[n]);
		ebebuild.iMatImg[n] = 0;
	}
	int n = 0;
	for ( int y = 0; y < 4; y++ )
	{
		for ( int x = 0; x < 4; x++ )
		{
			if (ebebuild.iMatSpr[n] > 0) if (SpriteExist(ebebuild.iMatSpr[n]) == 1) DeleteSprite(ebebuild.iMatSpr[n]);
			ebebuild.iMatSpr[n] = 0;
			n++;
		}
	}

	t.ebe.active = 0;
}
#endif

void ebe_init_newbuild ( int iBuildObj, int entid )
{
	// ensure EBE system is initialised
	if (t.ebe.active == 0)
	{
		ebe_init(iBuildObj, entid);
	}

	// load TXP profile from entity data
	// and update visuals for the material ref icons
	if ( t.entityprofile[entid].ebe.dwMatRefCount > 0 )
	{
		int ncount = t.entityprofile[entid].ebe.dwMatRefCount;
		for ( int n = 0; n < ncount; n++ )
		{
			ebebuild.TXP.iMaterialRef[n] = t.entityprofile[entid].ebe.iMatRef[n];
			if (!bDisableAllSprites) Sprite ( ebebuild.iMatSpr[n], SpriteX(ebebuild.iMatSpr[n]), SpriteY(ebebuild.iMatSpr[n]), ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[n]] );
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
		// update editor textures to match incoming EBE structure
		// by loading it direct from the EBE special long named textures and also the TXP file
		int ncount = t.entityprofile[entid].ebe.dwTexRefCount;
		for ( int n = 0; n < ncount; n++ )
			ebebuild.TXP.sTextureFile[n] = t.entityprofile[entid].ebe.pTexRef[n];

		// Also save snapshot of latest textures_DNS
		#ifdef VRTECH
		cstr sUniqueFilename = ebe_constructlongTXPname("_D.jpg");
		#else
		cstr sUniqueFilename = ebe_constructlongTXPname("_D.dds");
		#endif
		cstr sLongFilename = cstr("ebebank\\default\\") + sUniqueFilename;
		cstr tRawPathAndFile = cstr(Left(sLongFilename.Get(),strlen(sLongFilename.Get())-6));
		#ifdef VRTECH
		cstr sDDSFile = tRawPathAndFile + cstr("_D.jpg");
		#else
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		#endif
		if ( FileExist(sDDSFile.Get()) == 0 ) 
		{
			// cache in ebebank\default deleted, so copy from
			cstr tSourceRaw = g.mysystem.levelBankTestMap_s + sUniqueFilename; //cstr("levelbank\\testmap\\") + sUniqueFilename;
			tSourceRaw = cstr(Left(tSourceRaw.Get(),strlen(tSourceRaw.Get())-6));
			#ifdef VRTECH
			cstr tDDSSource = tSourceRaw + "_D.jpg";
			#else
			cstr tDDSSource = tSourceRaw + "_D.dds";
			#endif
			if ( FileExist(tDDSSource.Get()) == 0 ) 
			{
				// not in ebebank or in levelbank, try original saved ebe entity (though a real entity should never get here - only their entitybank copies)
				tSourceRaw = cstr("entitybank\\")+cstr(t.entitybank_s[entid]) + "\\" + sUniqueFilename;
				#ifdef VRTECH
				tDDSSource = tSourceRaw + "_D.jpg";
				#else
				tDDSSource = tSourceRaw + "_D.dds";
				#endif
			}
			#ifdef VRTECH
			sDDSFile = tRawPathAndFile + cstr("_D.jpg");
			#else
			sDDSFile = tRawPathAndFile + cstr("_D.dds");
			#endif
			CopyFileA ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			#ifdef VRTECH
			#else
			 tDDSSource = tSourceRaw + "_N.dds";
			 sDDSFile = tRawPathAndFile + cstr("_N.dds");
			 CopyFile ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			 tDDSSource = tSourceRaw + "_S.dds";
			 sDDSFile = tRawPathAndFile + cstr("_S.dds");
			 CopyFile ( tDDSSource.Get(), sDDSFile.Get(), FALSE );
			#endif
			#ifdef VRTECH
			sDDSFile = tRawPathAndFile + cstr("_D.jpg");
			#else
			sDDSFile = tRawPathAndFile + cstr("_D.dds");
			#endif
		}
		if ( FileExist(sDDSFile.Get()) == 1 ) 
		{
			#ifdef VRTECH
			cstr tDDSFilename = "ebebank\\default\\textures_D.jpg";
			#else
			cstr tDDSFilename = "ebebank\\default\\textures_D.dds";
			#endif
			if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteFileA ( tDDSFilename.Get() );
			CopyFileA ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
			#ifdef VRTECH
			#else
			 sDDSFile = tRawPathAndFile + cstr("_N.dds");
			 tDDSFilename = "ebebank\\default\\textures_N.dds";
			 if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteAFile ( tDDSFilename.Get() );
			 CopyFile ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
			 sDDSFile = tRawPathAndFile + cstr("_S.dds");
			 tDDSFilename = "ebebank\\default\\textures_S.dds";
			 if ( FileExist(tDDSFilename.Get()) == 1 ) DeleteAFile ( tDDSFilename.Get() );
			 CopyFile ( sDDSFile.Get(), tDDSFilename.Get(), FALSE );
			#endif
		}

		// and the TXP file too (260317 - generate, as copy may not exist)
		ebe_savetxp(cstr(cstr("ebebank\\default\\")+cstr("textures_profile.txp")).Get());

		// and update EBE editor textures image for selector
		#ifdef VRTECH
		LoadImage ( "ebebank\\default\\textures_D.jpg", ebebuild.iTexPlateImage );
		#else
		LoadImage ( "ebebank\\default\\textures_D.dds", ebebuild.iTexPlateImage );
		#endif
		if (!bDisableAllSprites) Sprite ( ebebuild.iTexturePanelSprite[5], SpriteX(ebebuild.iTexturePanelSprite[5]), SpriteY(ebebuild.iTexturePanelSprite[5]), ebebuild.iTexPlateImage );
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

	// Apply textures
	#ifdef VRTECH
 	 int iTexD = loadinternaltexture("ebebank\\default\\textures_D.jpg");
	 int iTexN = loadinternaltexture("effectbank\\reloaded\\media\\blank_N.dds");
	 int iTexS = loadinternaltexture("effectbank\\reloaded\\media\\blank_medium_S.DDS");
	#else
 	 int iTexD = loadinternaltexture("ebebank\\default\\textures_D.dds");
	 int iTexN = loadinternaltexture("ebebank\\default\\textures_N.dds");
	 int iTexS = loadinternaltexture("ebebank\\default\\textures_S.dds");
	#endif
	#ifdef VRTECH
	 LoadImage ( "ebebank\\default\\textures_D.jpg", iTexD );
	 LoadImage ( "effectbank\\reloaded\\media\\blank_N.dds", iTexN );
	 LoadImage ( "effectbank\\reloaded\\media\\blank_medium_S.DDS", iTexS );
	#else
	 LoadImage ( "ebebank\\default\\textures_D.dds", iTexD, 0, g.gdividetexturesize);
	 LoadImage ( "ebebank\\default\\textures_N.dds", iTexN, 0, g.gdividetexturesize);
	 LoadImage ( "ebebank\\default\\textures_S.dds", iTexS, 0, g.gdividetexturesize);
	#endif
	TextureObject ( iBuildObj, 0, iTexD );
	TextureObject ( iBuildObj, 1, loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds") );
	TextureObject ( iBuildObj, 2, iTexN );
	TextureObject ( iBuildObj, 3, iTexS );
	#ifdef VRTECH
	TextureObject ( iBuildObj, 4, loadinternaltexture("effectbank\\reloaded\\media\\materials\\0_Gloss.dds") );
	#else
	TextureObject ( iBuildObj, 4, t.terrain.imagestartindex );
	#endif
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



	#ifdef VRTECH
	//Place camera.
	int iAtX = t.entityelement[t.ebe.entityelementindex].x;
	int iAtY = t.entityelement[t.ebe.entityelementindex].y;
	int iAtZ = t.entityelement[t.ebe.entityelementindex].z;
	t.editorfreeflight.mode = 1;
	t.editorfreeflight.c.x_f = iAtX;
	t.editorfreeflight.c.y_f = iAtY + 400.0f;
	t.editorfreeflight.c.z_f = iAtZ;
	t.editorfreeflight.c.angx_f = 0.0f;
	t.editorfreeflight.c.angy_f = 50.0f;
	PositionCamera(t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
	RotateCamera(t.editorfreeflight.c.angx_f, t.editorfreeflight.c.angy_f, 0);
	MoveCamera(-350.0f);
	t.editorfreeflight.c.x_f = CameraPositionX();
	t.editorfreeflight.c.y_f = CameraPositionY();
	t.editorfreeflight.c.z_f = CameraPositionZ();
	t.editorfreeflight.c.angx_f = 30;
	#endif
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
	//PE: Get a crash here pRLEData == NULL
	if(t.entityprofile[entitybankindex].ebe.pRLEData)
		SAFE_DELETE ( t.entityprofile[entitybankindex].ebe.pRLEData );
	t.entityprofile[entitybankindex].ebe.dwRLESize = 0; 
	if(t.entityprofile[entitybankindex].ebe.iMatRef)
		SAFE_DELETE ( t.entityprofile[entitybankindex].ebe.iMatRef );
	t.entityprofile[entitybankindex].ebe.dwMatRefCount = 0; 
	if(t.entityprofile[entitybankindex].ebe.pTexRef)
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
	#ifdef VRTECH
	float fPixelBorder = 1.0f;
	float fShrinkDest = 1022.0f;
	#else

	//PE: We need 1022 on both versions now. ( https://github.com/TheGameCreators/GameGuruRepo/issues/782 )
	float fPixelBorder = 1.0f;
	float fShrinkDest = 1022.0f;
	
	//Old classic.
	// move UV coordinates inside texture slot so not touching with borders of other texture slots
	//float fPixelBorder = 32.0f;
	//float fShrinkDest = 1024.0f - (fPixelBorder*2);

	#endif
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
					SetupMeshFVFData ( pMesh, pMesh->dwFVF, dwVertPos, dwIndicePos, false );
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
		#ifdef VRTECH
		strcpy(ActiveEBEFilename, pEBEFilename);
		#endif
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
	if (!bDisableAllSprites) Sprite ( ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelX+(iCol*fChoiceWidth), ebebuild.iTexturePanelY+(iRow*fChoiceHeight), ebebuild.iTexturePanelHighImg );
}

#if defined(ENABLEIMGUI)
void imgui_ebe_loop(void)
{
	// if no tools object, cannot proceed
	if (ebebuild.iToolObj == 0) return;
	if (ObjectExist(ebebuild.iToolObj) == 0) return;
	if (GetObjectData(ebebuild.iToolObj)->bVisible == false) return;

	switch (delay_execute) 
	{
		case 1: //Change texture in plate.
		{
			delay_execute = 0;
			ebebuild.bCustomiseTexture = false;
			int iEntityProfileIndex = t.entityelement[t.ebe.entityelementindex].bankindex;
			if (ebe_loadcustomtexture(iEntityProfileIndex, ebebuild.iCurrentTexture) == 1)
			{
				// successfully pasted new texture into plate
			}
			//Reset click state.
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.MouseDown[1] = 0;
			io.MouseDownDuration[1] = 0;
			io.MouseDown[0] = 0;
			io.MouseDownDuration[0] = 0;
			skib_frames_execute = 5;

			break;
		}
		default:
			break;
	}
	if (skib_frames_execute > 0)
		skib_frames_execute--;

	ImVec4 drawCol_Down = ImColor(180, 180, 160, 255);
	ImVec4 drawCol_back = ImColor(255, 255, 255, 128);
	ImVec4 drawCol_normal = ImColor(255, 255, 255, 255);
	ImVec4 drawCol_hover = ImColor(180, 180, 180, 230);

	fPropertiesColoumWidth = 80;
	if (bBuilder_Properties_Window) 
	{
		float media_icon_size = 40.0f;
		float plate_width = (media_icon_size + 6.0) * 4.0f;
		ImVec4 tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_PlotHistogram];
		if (pref.current_style == 3)
			tool_selected_col = ImGui::GetStyle().Colors[ImGuiCol_Button];
		grideleprof_uniqui_id = 15000;
		extern int iGenralWindowsFlags;
		ImGui::Begin("Structure Properties##BuilderPropertiesWindow", &bBuilder_Properties_Window, iGenralWindowsFlags);

		float w = ImGui::GetWindowContentRegionWidth();

		if (ImGui::StyleCollapsingHeader("Name", ImGuiTreeNodeFlags_DefaultOpen)) {

			//Display icon.
			int thumb_size = 48;
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (thumb_size*0.5), 0.0f));
			ImGui::ImgBtn(EBE_THUMB, ImVec2(thumb_size, thumb_size), drawCol_back, drawCol_normal, drawCol_normal, drawCol_normal, -1, 0, 0, 0, true);
			
			ImGui::Indent(10);
			
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
			ImGui::Text("Name");
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));
			ImGui::SetCursorPos(ImVec2(fPropertiesColoumWidth, ImGui::GetCursorPosY()));
			ImGui::PushItemWidth(-10);
			ImGui::InputText("##structure_nameInput", &structure_name[0], MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGui::PopItemWidth();
			if (ImGui::IsItemHovered() ) ImGui::SetTooltip("Enter the name of this structure");
			ImGui::Indent(-10);

		}

		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (ImGui::StyleCollapsingHeader("Parts", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("##BuilderChildPanel", ImVec2(0, ImGui::GetFontSize()*3.0 + 12.0));

			extern char ActiveEBEFilename[260];
			int preview_count = 0;
			float contentarea = ImGui::GetWindowSize().x * ImGui::GetWindowSize().y;
			int media_icon_size_tools = 64;
			int iColumnsWidth = 110;
			bool bNoText = false;
			if (contentarea > 90000) {
				media_icon_size_tools = 64;
				iColumnsWidth = 110;
			}
			else if (contentarea > 80000) {
				media_icon_size_tools = 48;
				iColumnsWidth = 110 - 16;
			}
			else if (contentarea > 40000) {
				media_icon_size_tools = 32;
				iColumnsWidth = 110 - 16 - 16;
			}
			else if (contentarea > 22000) {
				media_icon_size_tools = 20;
				iColumnsWidth = 110 - 16 - 16;
				bNoText = true;
			}
			else {
				media_icon_size_tools = 20;
				iColumnsWidth = 110 - 16 - 16 - 16 - 8 ;
				bNoText = true;
			}

			bool bDisplayText = true;
			ImGui::SetWindowFontScale(SMALLFONTSIZE);
			float fWinWidth = ImGui::GetWindowSize().x - 10.0; // Flicker - ImGui::GetCurrentWindow()->ScrollbarSizes.x;
			if (iColumnsWidth >= fWinWidth && fWinWidth > media_icon_size_tools) {
				iColumnsWidth = fWinWidth;
				ImGui::SetWindowFontScale(SMALLESTFONTSIZE);
			}
			if (fWinWidth <= media_icon_size_tools + 10) {
				iColumnsWidth = media_icon_size_tools;
				ImGui::SetWindowFontScale(SMALLESTFONTSIZE);
			}
			if (fWinWidth <= 42) {
				iColumnsWidth = media_icon_size_tools + 16;
				bDisplayText = false;
			}

			int iColumns = (int)(ImGui::GetWindowSize().x / (iColumnsWidth));
			if (iColumns <= 1)
				iColumns = 1;

			ImGui::Columns(iColumns, "mycolumns4entities", false);  //false no border

			float fFramePadding = (iColumnsWidth - media_icon_size_tools)*0.5;
			float fCenterX = iColumnsWidth * 0.5;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(fFramePadding, 2.0f));

			for (int il = 0; il < t.ebebankmax; il++) 
			{
				int icon = TOOL_BUILDER;
				char tmp[MAX_PATH];
				strcpy(tmp, t.ebebank_s[il].Get());
				int pos = strlen(tmp);
				while (pos > 0 && tmp[pos] != '\\') pos--;
				if (pos > 0) 
				{
					cstr Text;
					strcpy(&tmp[0], &tmp[pos + 1]);
					if (pestrcasestr(tmp, "new site.")) { icon = EBE_NEW; Text = "Add New Site"; }
					else if (pestrcasestr(tmp, "cube.")) { icon = EBE_CUBE; Text = "Cube"; }
					else if (pestrcasestr(tmp, "floor.")) { icon = EBE_FLOOR; Text = "Floor"; }
					else if (pestrcasestr(tmp, "wall.")) { icon = EBE_WALL; Text = "Wall"; }
					else if (pestrcasestr(tmp, "column.")) { icon = EBE_COLUMN; Text = "Column"; }
					else if (pestrcasestr(tmp, "row.")) { icon = EBE_ROW; Text = "Row"; }
					else if (pestrcasestr(tmp, "stairs.")) { icon = EBE_STAIRS; Text = "Stairs"; }
					else if (pestrcasestr(tmp, "block.")) { icon = EBE_BLOCK; Text = "Block"; }
					else 
					{
						icon = TOOL_BUILDER;
						int pos2 = 0;
						while (tmp[pos2] != '.' && pos2 < strlen(tmp)) pos2++;
						if (pos2 > 0)
							tmp[pos2] = 0;
						Text = tmp;
					}

					if (strlen(ActiveEBEFilename) > 0 && pestrcasestr(t.ebebank_s[il].Get(), ActiveEBEFilename)) {
						ImVec2 padding = { 2.0, 2.0 };
						ImGuiWindow* window = ImGui::GetCurrentWindow();
						const ImRect image_bb((window->DC.CursorPos - padding) + ImVec2(fFramePadding, 2.0f), window->DC.CursorPos + padding + ImVec2(fFramePadding, 2.0f) + ImVec2(media_icon_size_tools, media_icon_size_tools));
						window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
					}

					if (ImGui::ImgBtn(icon, ImVec2(media_icon_size_tools, media_icon_size_tools), drawCol_back, drawCol_normal, drawCol_hover, drawCol_Down, -1, 0, 0, 0, true))
					{
						extern bool bImporter_Window;
						extern bool bWaypoint_Window;
						extern bool bWaypointDrawmode;
						extern bool g_bCharacterCreatorPlusActivated;
						void CheckTooltipObjectDelete(void);
						void CloseDownEditorProperties(void);

						if (bWaypointDrawmode || bWaypoint_Window) { bWaypointDrawmode = false; bWaypoint_Window = false; }
						if (bImporter_Window) { importer_quit(); bImporter_Window = false; }
						if (g_bCharacterCreatorPlusActivated) g_bCharacterCreatorPlusActivated = false;

						CheckTooltipObjectDelete();
						CloseDownEditorProperties();

						// leelee - seems we are creating a new entity, then not using it when a non newsite button is selected (with the old code)?
						if (icon != EBE_NEW) 
						{
							// select a new pattern
							LPSTR pPBFEBEFile = t.ebebank_s[il].Get();
							ebe_loadpattern(pPBFEBEFile);
							t.inputsys.constructselection = 0;
						}
						else
						{
							// trigger creation of new site
							t.addentityfile_s = t.ebebank_s[il].Get();
							if (t.addentityfile_s != "")
							{
								entity_adduniqueentity(false);
								t.tasset = t.entid;
								if (t.talreadyloaded == 0)
								{
									editor_filllibrary();
								}
							}
							t.inputsys.constructselection = t.tasset;
							t.gridentity = t.entid;
							t.inputsys.constructselection = t.entid;
							t.inputsys.domodeentity = 1;
							t.grideditselect = 5;
							editor_refresheditmarkers();

							// NewSite, make sure we are in entity mode.
							bForceKey = true;
							csForceKey = "e";
						}
						/*
						t.addentityfile_s = t.ebebank_s[il].Get();
						if (t.addentityfile_s != "")
						{
							entity_adduniqueentity(false);
							t.tasset = t.entid;
							if (t.talreadyloaded == 0)
							{
								editor_filllibrary();
							}
						}
						t.inputsys.constructselection = t.tasset;

						t.gridentity = t.entid;
						t.inputsys.constructselection = t.entid;
						t.inputsys.domodeentity = 1;
						t.grideditselect = 5;
						editor_refresheditmarkers();

						if (icon != EBE_NEW) {
							LPSTR pPBFEBEFile = t.ebebank_s[il].Get();
							ebe_loadpattern(pPBFEBEFile);
							t.inputsys.constructselection = 0;
						}
						else {
							//NewSite, make sure we are in entity mode.
							bForceKey = true;
							csForceKey = "e";
						}
						*/
					}
					if (ImGui::IsItemHovered() && Text != "") ImGui::SetTooltip("%s", Text.Get());

					if (!bNoText) {
						int iTextWidth = ImGui::CalcTextSize(Text.Get()).x;
						if (iTextWidth < iColumnsWidth)
							ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (fCenterX - (iTextWidth*0.5)), ImGui::GetCursorPosY()));
						ImGui::TextWrapped(Text.Get());
					}
					ImGui::NextColumn();
				}
			}

			ImGui::PopStyleVar();
			ImGui::SetWindowFontScale(1.00);
			ImGui::Columns(1);
			ImGui::EndChild();
		}

		if (ImGui::StyleCollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int control_image_size = 42;
			float control_width = (control_image_size + 6.0) * 3.0f;

			int indent = (w*0.5) - (control_width*0.5);
			if (indent < 10)
				indent = 10;
			ImGui::Indent(indent);

			if (iPaintMode == 1)
			{
				ImVec2 padding = { 4.0, 4.0 };
				const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(control_image_size, control_image_size));
				window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			}
			if (ImGui::ImgBtn(EBE_CONTROL1, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//Paint mode.
				iPaintMode = 1;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Paint Mode");

			ImGui::SameLine();

			if (iPaintMode != 1)
			{
				ImVec2 padding = { 4.0, 4.0 };
				const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(control_image_size, control_image_size));
				window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
			}
			if (ImGui::ImgBtn(EBE_CONTROL2, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//Remove mode.
				iPaintMode = 0;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete Mode");
			ImGui::SameLine();

			if (ImGui::ImgBtn(EBE_CONTROL3, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//rotate
				bForceKey = true;
				csForceKey = "r";
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate ('R')");

			
			if (ImGui::ImgBtn(EBE_CONTROL4, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//page up
				iForceScancode = 201;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Page Up");
			ImGui::SameLine();
			if (ImGui::ImgBtn(EBE_CONTROL5, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//page down
				iForceScancode = 209;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Page Down");
			ImGui::SameLine();
			if (ImGui::ImgBtn(EBE_CONTROL6, ImVec2(control_image_size, control_image_size), ImVec4(0.0, 0.0, 0.0, 0.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, 0, 0, 0, false)) {
				//exit
				bForceKey = true;
				csForceKey = "e";
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Exit Structure Editor ('E')");
			
			ImGui::Indent(-indent);

		}
		if (ImGui::StyleCollapsingHeader("Texture Set", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (plate_width*0.5), 0.0f));
			int indent = (w*0.5) - (plate_width*0.5);
			if (indent < 10)
				indent = 10;
			ImGui::Indent(indent);
			
			ImGui::PushItemWidth(plate_width);

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, 4.0f));
			const char* items[] = { "Default" };
			if (ImGui::Combo("##TextureSetSelection", &texture_set_selection, items, IM_ARRAYSIZE(items))) 
			{
				//texture_set_selection
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select Texture Palette");

			ImGui::PopItemWidth();
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.0f, 4.0f));
			int n = 0;
			int iTextureIndexBeingHoveredOver = -1;
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					if (ebebuild.iTexPlateImage > 0) 
					{
						ImVec2 padding = { 2.0, 2.0 };
						ImVec4 bg_col = { 0.0, 0.0, 0.0, 1.0 };
						const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(media_icon_size, media_icon_size));
						window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, ImGui::GetColorU32(bg_col), 0.0f, 15);

						if (ebebuild.iCurrentTexture == n) 
						{
							ImVec2 padding = { 4.0, 4.0 };
							const ImRect image_bb((window->DC.CursorPos - padding), window->DC.CursorPos + padding + ImVec2(media_icon_size, media_icon_size));
							window->DrawList->AddRect(image_bb.Min, image_bb.Max, ImGui::GetColorU32(tool_selected_col), 0.0f, 15, 2.0f);
						}

						ImGui::PushID(ebebuild.iTexPlateImage + n + 200000);
						if (ImGui::ImgBtn(ebebuild.iTexPlateImage, ImVec2(media_icon_size, media_icon_size), ImVec4(0.0, 0.0, 0.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, n + 1, 4, 4, false)) 
						{
							ebebuild.iCurrentTexture = n;
						}
						if (skib_frames_execute == 0 && ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) 
						{
							ebebuild.iCurrentTexture = n;
							delay_execute = 1; //@Lee remove this line if you dont want to support changing textures.
						}
						if ( ImGui::IsItemHovered() )
						{
							iTextureIndexBeingHoveredOver = n;
						}

						ImGui::PopID();
						if (x != 3)
							ImGui::SameLine();

						if (ImGui::IsItemHovered()) 
						{
							ImVec2 cursor_pos = ImGui::GetIO().MousePos;
							ImVec2 tooltip_offset(10.0f, ImGui::GetFontSize()*1.5);
							ImVec2 tooltip_position = cursor_pos;
							if (tooltip_position.x + 210 > GetDesktopWidth())
								tooltip_position.x -= 210;
							tooltip_position.x += tooltip_offset.x;
							tooltip_position.y += tooltip_offset.y;
							ImGui::SetNextWindowPos(tooltip_position);
							ImGui::SetNextWindowContentWidth(204.0f);
							ImGui::BeginTooltip();
							float icon_ratio;
							int icon_size = 204;
							ImGui::ImgBtn(ebebuild.iTexPlateImage, ImVec2(icon_size, icon_size), ImVec4(0.0, 0.0, 0.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(0.8, 0.8, 0.8, 0.8), ImVec4(0.8, 0.8, 0.8, 0.8), 0, n + 1, 4, 4, false);
							int iTexSlot = iTextureIndexBeingHoveredOver;
							int iMaterialIndex = ebebuild.TXP.iMaterialRef[iTexSlot];
							LPSTR pMaterialType = "Other";
							switch (iMaterialIndex)
							{
								case 0: pMaterialType = "Generic"; break;
								case 1: pMaterialType = "Stone"; break;
								case 2: pMaterialType = "Metal"; break;
								case 3: pMaterialType = "Wood"; break;
							}
							char pShowMaterialText[256];
							sprintf(pShowMaterialText, "Material %d: %s", 1+iMaterialIndex, pMaterialType);
							ImGui::TextCenter(pShowMaterialText);
							ImGui::EndTooltip();
						}
					}
					n++;
				}
			}
			ImGui::Indent(-indent);

			// detect if user wants to change material
			if ( iTextureIndexBeingHoveredOver !=-1 )
			{
				ImGuiIO& io = ImGui::GetIO(); (void)io;
				if ( io.KeysDown[49] > 0 ) ebebuild.TXP.iMaterialRef[iTextureIndexBeingHoveredOver] = 0;
				if ( io.KeysDown[50] > 0 ) ebebuild.TXP.iMaterialRef[iTextureIndexBeingHoveredOver] = 1;
				if ( io.KeysDown[51] > 0 ) ebebuild.TXP.iMaterialRef[iTextureIndexBeingHoveredOver] = 2;
				if ( io.KeysDown[52] > 0 ) ebebuild.TXP.iMaterialRef[iTextureIndexBeingHoveredOver] = 3;
			}

			// keyboard help to change material
			ImGui::TextCenter("Hover over texture to view");
			ImGui::TextCenter("Press keys 1-4 to choose material");
			ImGui::TextCenter("1=Generic  2=Stone  3=Metal  4=Wood");
			ImGui::Text("");
		}


		if (ImGui::StyleCollapsingHeader("Save Structure", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float col_start = 80.0f;
			float save_gadget_size = ImGui::GetFontSize()*12.0;

			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 3));
			ImGui::Text("Path");
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 3));

			ImGui::SetCursorPos(ImVec2(col_start, ImGui::GetCursorPosY()));

			float path_gadget_size = ImGui::GetFontSize()*2.0;

			ImGui::PushItemWidth(-10 - path_gadget_size);

			ImGui::InputText("##InputPathBuilder", &BuilderPath[0], 250);
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::PushItemWidth(path_gadget_size);
			if (ImGui::StyleButton("...##Builderpath"))
			{
				//PE: filedialogs change dir so.
				cStr tOldDir = GetDir();
				char * cFileSelected;
				cstr fulldir = tOldDir + "\\entitybank\\user\\ebestructures";
				char pRealDestFolder[MAX_PATH];
				strcpy(pRealDestFolder, fulldir.Get());
				GG_GetRealPath(pRealDestFolder,1);
				strcat(pRealDestFolder, "\\"); // does not seem to place me in the current ebestructures folder?
				fulldir = pRealDestFolder;
				cFileSelected = (char *)noc_file_dialog_open(NOC_FILE_DIALOG_DIR, "All\0*.*\0", fulldir.Get(), NULL);
				SetDir(tOldDir.Get());
				if (cFileSelected && strlen(cFileSelected) > 0) 
				{
					strcpy(BuilderPath, cFileSelected);
				}
			}
			ImGui::PopItemWidth();

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (save_gadget_size*0.5), 0.0f));

			if (ImGui::StyleButton("Save Structure As Entity", ImVec2(save_gadget_size, 0)))
			{
				//Save structure
				if (strlen(structure_name) > 0)
				{
					if (strlen(BuilderPath) > 0)
					{
						//Save
						cStr tSaveFile = BuilderPath;
						if(BuilderPath[strlen(BuilderPath)-1] != '\\' || BuilderPath[strlen(BuilderPath) - 1] != '/')
							tSaveFile += "\\";
						tSaveFile += structure_name;
						if(!pestrcasestr(tSaveFile.Get(),".ebe"))
							tSaveFile += ".ebe";

						//Resolve path.
						char resolved[MAX_PATH];
						int retval = GetFullPathNameA(tSaveFile.Get(), MAX_PATH, resolved, NULL);
						if (retval > 0) {
							tSaveFile = resolved;
						}


						int iBuilderId = t.ebe.entityelementindex;
						ebe_hide();
						//ebe_finishsite();


						strcpy(cTriggerMessage, "Structure Saved");
						bTriggerMessage = true;

						bool bSaveIt = true;
						// Check if already exists, if so, ask if should be overwritten
						if (FileExist(tSaveFile.Get()) == 1)
						{
							// Already Exists
							char pDisplayErrorMsg[512];
							strcpy(pDisplayErrorMsg, structure_name);
							strcat(pDisplayErrorMsg, " already exists! Overwrite?");
							HWND hThisWnd = GetForegroundWindow();
							if (MessageBoxA(hThisWnd, pDisplayErrorMsg, "File Already Exists", MB_YESNO | MB_TOPMOST) != IDYES)
							{
								strcpy(cTriggerMessage, "Structure was NOT Saved.");
								bSaveIt = false;
							}
						}

						if (bSaveIt) {
							ebe_save_ebefile(tSaveFile, iBuilderId);
						}

						ebe_newsite(iBuilderId);

					}
					else
					{
						strcpy(cTriggerMessage, "Please select a path where you like the structure saved.");
						bTriggerMessage = true;
					}
				}
				else
				{
					strcpy(cTriggerMessage, "You must give your structure a name before you can save it.");
					bTriggerMessage = true;
				}
			}
		}

		if (!pref.bHideTutorials)
		{
			if (ImGui::StyleCollapsingHeader("Tutorial (this feature is incomplete)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Indent(10);
				void SmallTutorialVideo(char *tutorial, char* combo_items[] = NULL, int combo_entries = 0, int iVideoSection = 0);
				cstr cShowTutorial = "03 - Add character and set a path";
				char* tutorial_combo_items[] = { "01 - Getting started", "02 - Creating terrain", "03 - Add character and set a path" };
				SmallTutorialVideo(cShowTutorial.Get(), tutorial_combo_items, ARRAYSIZE(tutorial_combo_items), SECTION_STRUCTURE_EDITOR);
				float but_gadget_size = ImGui::GetFontSize()*12.0;
				float w = ImGui::GetWindowContentRegionWidth() - 10.0;
				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2((w*0.5) - (but_gadget_size*0.5), 0.0f));
				#ifdef INCLUDESTEPBYSTEP
				if (ImGui::StyleButton("View Step by Step Tutorial", ImVec2(but_gadget_size, 0)))
				{
					// pre-select tutorial 03
					extern bool bHelpVideo_Window;
					extern bool bHelp_Window;
					extern char cForceTutorialName[1024];
					bHelp_Window = true;
					bHelpVideo_Window = true;
					extern bool bSetTutorialSectionLeft;
					bSetTutorialSectionLeft = false;
					strcpy(cForceTutorialName, cShowTutorial.Get());
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Start Step by Step Tutorial");
				#endif

				ImGui::Indent(-10);
			}
		}

		void CheckMinimumDockSpaceSize(float minsize);
		CheckMinimumDockSpaceSize(250.0f);

		if (ImGui::GetCurrentWindow()->ScrollbarSizes.x > 0) {
			//Hitting exactly at the botton could cause flicker, so add some additional lines when scrollbar on.
			ImGui::Text("");
			ImGui::Text("");
		}

		ImGui::End();
	}
}
#endif

void ebe_loop(void)
{
	// if no tools object, cannot proceed
	if (ebebuild.iToolObj == 0) return;
	if (ObjectExist(ebebuild.iToolObj) == 0) return;
	if (GetObjectData(ebebuild.iToolObj)->bVisible == false) return;

	//Dynamic place tools
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
	ebebuild.iTexturePanelX = GetChildWindowWidth() - 210;
	ebebuild.iTexturePanelY = GetChildWindowHeight() - 200;
	ebebuild.iTexturePanelWidth = 200;
	ebebuild.iTexturePanelHeight = 200;
	for (int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++)
	{
		LPSTR pTexImg = "";
		int iX = ebebuild.iTexturePanelX;
		int iY = ebebuild.iTexturePanelY;
		int iWidth = ebebuild.iTexturePanelWidth;
		int iHeight = ebebuild.iTexturePanelHeight;
		if (iTex == 0) { iX -= 10; iY -= 10; iWidth += 20; iHeight += 20; }
		if (iTex == 1) { iX -= 10; iY -= 10; iWidth += 20; iHeight = 1; }
		if (iTex == 2) { iX -= 10; iY += 209; iWidth += 20; iHeight = 1; }
		if (iTex == 3) { iX -= 10; iY -= 10; iWidth = 1; iHeight += 20; }
		if (iTex == 4) { iX += 209; iY -= 10; iWidth = 1; iHeight += 20; }
		if (!bDisableAllSprites) Sprite(ebebuild.iTexturePanelSprite[iTex], iX, iY, ebebuild.iTexturePanelImg[iTex]);
	}
	if (!bDisableAllSprites) Sprite(ebebuild.iTexturePanelHighSprite, ebebuild.iTexturePanelX, ebebuild.iTexturePanelY, ebebuild.iTexturePanelHighImg);
	if (!bDisableAllSprites) Sprite(ebebuild.iEBEHelpSpr, ebebuild.iTexturePanelX - ImageWidth(ebebuild.iEBEHelpImg) - 10, ebebuild.iTexturePanelY + 210 - ImageHeight(ebebuild.iEBEHelpImg), ebebuild.iEBEHelpImg);
	if (!bDisableAllSprites) Sprite(ebebuild.iEBETexHelpSpr, ebebuild.iTexturePanelX - 10, ebebuild.iTexturePanelY - 10 - ImageHeight(ebebuild.iEBETexHelpImg), ebebuild.iEBETexHelpImg);
	int n = 0;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (!bDisableAllSprites) Sprite(ebebuild.iMatSpr[n], ebebuild.iTexturePanelX + 36 + (x * 50), ebebuild.iTexturePanelY + 36 + (y * 50), ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[n]]);
			n++;
		}
	}
	ebe_settexturehighlight();
	#endif

	// if UNDO or somehow delete this building obj, revert entity mode
	if (ObjectExist(ebebuild.iBuildObj) == 0)
	{
		ebe_hide();
		t.inputsys.domodeentity = 1;
		return;
	}

	// go through model slowly to refresh meshes (and avoid a vertbuffer lock)
	if (ebebuild.iRefreshBuild > 0)
	{
		ebebuild.iRefreshBuild--;
		int iLayerTotarget = 19 - ebebuild.iRefreshBuild;
		int iYBottom = (iLayerTotarget * 10);
		int iYTop = (iLayerTotarget * 10) + 10;
		if (iYTop > CUBEAREASIZE - 11) iYTop = CUBEAREASIZE - 11;
		ebe_refreshmesh(ebebuild.iBuildObj, 0, iYBottom, 0, CUBEAREASIZE - 10, iYTop, CUBEAREASIZE - 10);
		return;
	}

	// keep orientation with the main build object
	RotateObject(ebebuild.iToolObj, ObjectAngleX(ebebuild.iBuildObj), ObjectAngleY(ebebuild.iBuildObj), ObjectAngleZ(ebebuild.iBuildObj));

	// Keep tool object with EBE entity object (build object)
	int iAtX = t.entityelement[t.ebe.entityelementindex].x;
	int iAtY = t.entityelement[t.ebe.entityelementindex].y;
	int iAtZ = t.entityelement[t.ebe.entityelementindex].z;
	PositionObject(ebebuild.iToolObj, iAtX, iAtY + (CameraPositionY(0) / 1000.0f), iAtZ);
	PositionObject(ebebuild.iBuildObj, iAtX, iAtY, iAtZ);

	// Only when release mouse continue
	if (t.ebe.bReleaseMouseFirst == true && t.inputsys.mclick != 0) return;
	t.ebe.bReleaseMouseFirst = false;

	// One press key logic
	if (t.inputsys.kscancode == 0) ebebuild.iLocalKeyPressed = 0;

	// Reason this is above action for selecting customise is to allow texture highlight to show as selected
	if (ebebuild.bCustomiseTexture == true && t.inputsys.mclick == 0)
	{
		ebebuild.bCustomiseTexture = false;
		int iEntityProfileIndex = t.entityelement[t.ebe.entityelementindex].bankindex;
		if (ebe_loadcustomtexture(iEntityProfileIndex, ebebuild.iCurrentTexture) == 1)
		{
			// successfully pasted new texture into plate
		}
	}

	// use - and + keys to scroll through material index for current texture selected
	if (ebebuild.iLocalKeyPressed == 0)
	{
		if (t.inputsys.k_s == "-") { ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] -= 1; ebebuild.iLocalKeyPressed = 1; }
		if (t.inputsys.k_s == "=") { ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] += 1; ebebuild.iLocalKeyPressed = 1; }
		if (ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] < 0) ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] = 0;
		if (ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] > 18) ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture] = 18;
		if (!bDisableAllSprites) SetSpriteImage(ebebuild.iMatSpr[ebebuild.iCurrentTexture], ebebuild.iMatImg[ebebuild.TXP.iMaterialRef[ebebuild.iCurrentTexture]]);
	}

	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE)
	//This has been moved to imgui.
	#else
	// Select texture if in Texture Panel or Customise one
	int iRealSprMouseX = (GetChildWindowWidth() / 800.0f) * t.inputsys.xmouse;
	int iRealSprMouseY = (GetChildWindowHeight() / 600.0f) * t.inputsys.ymouse;
	if (t.inputsys.mclick > 0)
	{
		if (iRealSprMouseX > ebebuild.iTexturePanelX && iRealSprMouseX < ebebuild.iTexturePanelX + ebebuild.iTexturePanelWidth)
		{
			if (iRealSprMouseY > ebebuild.iTexturePanelY && iRealSprMouseY < ebebuild.iTexturePanelY + ebebuild.iTexturePanelHeight)
			{
				// while tile
				float fWhichCol = (float)(iRealSprMouseX - ebebuild.iTexturePanelX) / (float)ebebuild.iTexturePanelWidth;
				float fWhichRow = (float)(iRealSprMouseY - ebebuild.iTexturePanelY) / (float)ebebuild.iTexturePanelHeight;
				int iWhichTextureOver = (((int)(fWhichRow * 4)) * 4) + (int)(fWhichCol * 4);

				// select texture choice
				ebebuild.iCurrentTexture = iWhichTextureOver;
				ebe_settexturehighlight();

				// and if it was right mouse, customise this texture too
				if (t.inputsys.mclick == 2)
				{
					// replace texture within texture atlas
					ebebuild.bCustomiseTexture = true;
				}

				// ensure we do not write into builder if selecting texture
				set_inputsys_mclick(0);//t.inputsys.mclick = 0;
			}
		}
	}
	#endif

	// Control EBE edit grid layer
	float fCurrentGridLayerAbsHeight = ebebuild.iCurrentGridLayer*5.0f;
	float fMouseWheel = t.inputsys.wheelmousemove;
	if (t.inputsys.kscancode == 201 || t.inputsys.kscancode == 209 || (fMouseWheel != 0 && t.inputsys.keycontrol == 0))
	{
		if (ebebuild.iLocalKeyPressed == 0)
		{
			// work out snap grid size
			int iGridSize = ebebuild.Pattern.iWidth;
			if (ebebuild.Pattern.iHeight > iGridSize) iGridSize = ebebuild.Pattern.iHeight;
			if (ebebuild.Pattern.iDepth > iGridSize) iGridSize = ebebuild.Pattern.iDepth;

			// special case for ROW shape (one high, and one dimension longer on X or Z)
			if (ebebuild.Pattern.iHeight == 1 && ebebuild.Pattern.iWidth != ebebuild.Pattern.iDepth) iGridSize = 1;

			// move current grid up or down
			float fYShift = 0.0f;
			ebebuild.iLocalKeyPressed = 1;
			if ((t.inputsys.kscancode == 201 || fMouseWheel > 0.0f) && ebebuild.iCurrentGridLayer < 200 - iGridSize) { ebebuild.iCurrentGridLayer += iGridSize; fYShift = iGridSize * 5.0f; }
			if ((t.inputsys.kscancode == 209 || fMouseWheel < 0.0f))
			{
				ebebuild.iCurrentGridLayer -= iGridSize; fYShift = iGridSize * -5.0f;
				if (ebebuild.iCurrentGridLayer < 0) ebebuild.iCurrentGridLayer = 0;
			}
			fCurrentGridLayerAbsHeight = ebebuild.iCurrentGridLayer*5.0f;
			OffsetLimb(ebebuild.iToolObj, 1, 500, fCurrentGridLayerAbsHeight + 0.05f, 500);

			// also move camera for easier more intuitive editing 
			if (fMouseWheel == 0.0f)
			{
				// only with mouse wheel for convenience
				t.editorfreeflight.c.y_f += fYShift;
				PositionCamera(0, t.editorfreeflight.c.x_f, t.editorfreeflight.c.y_f, t.editorfreeflight.c.z_f);
			}
		}
	}

	// Rotate cursor and pattern
	if (t.inputsys.k_s == "r")
	{
		if (ebebuild.iLocalKeyPressed == 0)
		{
			ebebuild.iLocalKeyPressed = 1;
			ebebuild.iCursorRotation += 1;
			if (ebebuild.iCursorRotation > 3)
			{
				ebebuild.iCursorRotation = 0;
			}
			ebe_updatepatternwithrotation();
		}
	}

	// Position cursor on current grid layer
	bool bOffGrid = false;
	float fGridAbsHeight = LimbPositionY(ebebuild.iToolObj, 1);
	SetCurrentCamera(0);
	SetCameraRange(1, 70000);
	t.screenwidth_f = 800.0;
	t.screenheight_f = 600.0;
	GetProjectionMatrix(g.m4_projection);
	GetViewMatrix(g.m4_view);
	t.blank = InverseMatrix(g.m4_projection, g.m4_projection);
	t.blank = InverseMatrix(g.m4_view, g.m4_view);
	#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
	//PE: Do not use (800.0x600.0) Just convert , we need any resolution to work.
	t.tadjustedtoareax_f = ((float)t.inputsys.xmouse / (float)GetDisplayWidth()) / ((float)GetDisplayWidth() / (float)GetChildWindowWidth(-1));
	t.tadjustedtoareay_f = ((float)t.inputsys.ymouse / (float)GetDisplayHeight()) / ((float)GetDisplayHeight() / (float)GetChildWindowHeight(-1));
	#else
	t.tadjustedtoareax_f = (GetDisplayWidth() + 0.0) / (GetChildWindowWidth() + 0.0);
	t.tadjustedtoareay_f = (GetDisplayHeight() + 0.0) / (GetChildWindowHeight() + 0.0);
	t.tadjustedtoareax_f = ((t.inputsys.xmouse + 0.0) / 800.0) / t.tadjustedtoareax_f;
	t.tadjustedtoareay_f = ((t.inputsys.ymouse + 0.0) / 600.0) / t.tadjustedtoareay_f;
	#endif
	SetVector4(g.v4_far, (t.tadjustedtoareax_f * 2) - 1, -((t.tadjustedtoareay_f * 2) - 1), 0, 1);
	TransformVector4(g.v4_far, g.v4_far, g.m4_projection);
	// works in DX9 (D3DXVec4Transform) but not DX11 (KMATRIX)
	t.tx_f = GetXVector4(g.v4_far);
	t.ty_f = GetYVector4(g.v4_far);
	t.tz_f = GetZVector4(g.v4_far);
	SetVector3(g.v3_far, t.tx_f, t.ty_f, t.tz_f);
	TransformVectorCoordinates3(g.v3_far, g.v3_far, g.m4_view);
	t.tx_f = GetXVector3(g.v3_far);
	t.ty_f = GetYVector3(g.v3_far);
	t.tz_f = GetZVector3(g.v3_far);
	t.fx_f = CameraPositionX(0);
	t.fy_f = CameraPositionY(0);
	t.fz_f = CameraPositionZ(0);
	t.tx_f = t.tx_f - t.fx_f;
	t.ty_f = t.ty_f - t.fy_f;
	t.tz_f = t.tz_f - t.fz_f;
	t.tt_f = abs(t.tx_f) + abs(t.ty_f) + abs(t.tz_f);
	t.tx_f = t.tx_f / t.tt_f;
	t.ty_f = t.ty_f / t.tt_f;
	t.tz_f = t.tz_f / t.tt_f;
	if (t.ty_f < 0)
	{
		// face down onto grid
		float fDistToGridLayer = CameraPositionY(0) - fGridAbsHeight;
		float fSteps = fDistToGridLayer / fabs(t.ty_f);
		t.tx_f *= fSteps;
		t.tz_f *= fSteps;
		t.tx_f -= ObjectPositionX ( ebebuild.iToolObj );
		t.tz_f -= ObjectPositionZ ( ebebuild.iToolObj );
		t.tx_f += CameraPositionX(0);
		t.tz_f += CameraPositionZ(0);
	}
	else
	{
		bOffGrid = true;
	}
	if ( bOffGrid == false )
	{
		GGVECTOR3 vec3 = GGVECTOR3(t.tx_f, 0, t.tz_f);
		GGMATRIX matRot;
		GGMatrixRotationY(&matRot, GGToRadian(-ObjectAngleY(ebebuild.iToolObj)));
		GGVec3TransformCoord(&vec3, &vec3, &matRot);
		t.tx_f = vec3.x;
		t.tz_f = vec3.z;
		t.tx_f = (int)(t.tx_f / 5.0f);
		t.tz_f = (int)(t.tz_f / 5.0f);
		if (t.tx_f < 0) { t.tx_f = 0; bOffGrid = true; }
		if (t.tz_f < 0) { t.tz_f = 0; bOffGrid = true; }
		if (t.tx_f > 199) { t.tx_f = 199; bOffGrid = true; }
		if (t.tz_f > 199) { t.tz_f = 199; bOffGrid = true; }
		int iGridSnapX = ebebuild.Pattern.iWidth;
		int iGridSnapZ = ebebuild.Pattern.iDepth;
		if (iGridSnapX > iGridSnapZ) iGridSnapZ = iGridSnapX;
		if (iGridSnapZ > iGridSnapX) iGridSnapX = iGridSnapZ;
		t.tx_f = (int)(t.tx_f / iGridSnapX);
		t.tz_f = (int)(t.tz_f / iGridSnapZ);
		t.tx_f = (t.tx_f*iGridSnapX) + (ebebuild.Pattern.iWidth / 2);
		t.tz_f = (t.tz_f*iGridSnapZ) + (ebebuild.Pattern.iDepth / 2);
		t.tx_f += ebebuild.Pattern.iWidthOffset;
		t.tz_f += ebebuild.Pattern.iDepthOffset;
		ebebuild.iCursorGridPosX = t.tx_f;
		ebebuild.iCursorGridPosZ = t.tz_f;
		t.tx_f *= 5.0f;
		t.tz_f *= 5.0f;
		int iHalfX = ebebuild.Pattern.iWidth / 2;
		int iHalfZ = ebebuild.Pattern.iDepth / 2;
		float fFractionX = ebebuild.Pattern.iWidth - (iHalfX * 2);
		float fFractionZ = ebebuild.Pattern.iDepth - (iHalfZ * 2);
		float fCubeSizeX = fFractionX * 2.5f;
		float fCubeSizeY = ebebuild.Pattern.iHeight * 2.5f;
		float fCubeSizeZ = fFractionZ * 2.5f;
		OffsetLimb(ebebuild.iToolObj, 2, t.tx_f + fCubeSizeX, fCurrentGridLayerAbsHeight + fCubeSizeY, t.tz_f + fCubeSizeZ);
	}

	// Hide cursor if not in grid
	if (bOffGrid == false)
	{
		ShowLimb(ebebuild.iToolObj, 2);
		ScaleLimb(ebebuild.iToolObj, 2, ebebuild.Pattern.iWidth*100.0f, ebebuild.Pattern.iHeight*100.0f, ebebuild.Pattern.iDepth*100.0f);
		if (ebebuild.Pattern.iWidth == ebebuild.Pattern.iDepth)
			RotateLimb(ebebuild.iToolObj, 2, 0, ebebuild.iCursorRotation * 90.0f, 0);
		else
			RotateLimb(ebebuild.iToolObj, 2, 0, 0.0f, 0);
	}
	else
	{
		HideLimb(ebebuild.iToolObj, 2);
	}

	// Before start to mark cube data, snapshot for undo buffer
	if (t.ebe.iWrittenCubeData == 0 && t.inputsys.mclick == 1)
	{
		ebe_snapshottobuffer();
		t.ebe.iWrittenCubeData = 1;
	}
	else
	{
		// and wait for user to click and write 'something'
		if (t.inputsys.mclick == 0 && t.ebe.iWrittenCubeData > 1)
		{
			// only then when release mouse, do a new snapshot
			t.ebe.iWrittenCubeData = 0;
		}
	}

	// If use axis-line-system, can correct cursor position
	if (t.inputsys.keycontrol != 0)
	{
		if (ebebuild.iDeterminedAxisDir == 0)
		{
			if (ebebuild.iCursorGridPosLastGoodX != ebebuild.iCursorGridPosX) ebebuild.iDeterminedAxisDir = 1;
			if (ebebuild.iCursorGridPosLastGoodZ != ebebuild.iCursorGridPosZ) ebebuild.iDeterminedAxisDir = 2;
			ebebuild.iCursorGridPosLastGoodX = ebebuild.iCursorGridPosX;
			ebebuild.iCursorGridPosLastGoodZ = ebebuild.iCursorGridPosZ;
		}
		else
		{
			if (ebebuild.iDeterminedAxisDir == 1) ebebuild.iCursorGridPosZ = ebebuild.iCursorGridPosLastGoodZ;
			if (ebebuild.iDeterminedAxisDir == 2) ebebuild.iCursorGridPosX = ebebuild.iCursorGridPosLastGoodX;
		}
	}
	else
	{
		ebebuild.iDeterminedAxisDir = 0;
		ebebuild.iCursorGridPosLastGoodX = ebebuild.iCursorGridPosX;
		ebebuild.iCursorGridPosLastGoodZ = ebebuild.iCursorGridPosZ;
	}

	// Detect if we add/delete/texture inside the grid
	if (bOffGrid == false)
	{
		bool bReverseOperation = false;
		if (t.inputsys.keyshift == 1) bReverseOperation = true;
		if (t.inputsys.mclick == 1)
		{
			// holding down left mouse button - add/delete/texture
			bool bRecordChangeToCubeData = false;
			int iRecordX1 = 999999, iRecordY1 = 999999, iRecordZ1 = 999999;
			int iRecordX2 = -999999, iRecordY2 = -999999, iRecordZ2 = -999999;
			for (int iPatternX = 0; iPatternX < ebebuild.Pattern.iWidth; iPatternX++)
			{
				for (int iPatternY = 0; iPatternY < ebebuild.Pattern.iHeight; iPatternY++)
				{
					for (int iPatternZ = 0; iPatternZ < ebebuild.Pattern.iDepth; iPatternZ++)
					{
						// get cube tyle and texture assignment
						cStr pPattern = ebebuild.Pattern.pPRow[iPatternY][iPatternZ];
						char* p;
						unsigned char cCubeType = strtol(Mid(pPattern.Get(), 1 + iPatternX), &p, 16);
						unsigned char cTexIndex = 0;
						if (cCubeType > 0) cTexIndex = ebebuild.iCurrentTexture;

						// intended new block type
						unsigned char cBitCube = (cTexIndex << 4) + cCubeType;

						// work out exact place in construction grid
						int iThisX = ebebuild.iCursorGridPosX + iPatternX;
						int iThisY = ebebuild.iCurrentGridLayer + iPatternY;
						int iThisZ = ebebuild.iCursorGridPosZ + iPatternZ;

						// cursor holds shape at center, so backward offset
						iThisX = iThisX - ebebuild.Pattern.iWidth / 2;
						iThisZ = iThisZ - ebebuild.Pattern.iDepth / 2;

						// only if within field of construction area
						if (iThisX < CUBEAREASIZE && iThisY < CUBEAREASIZE && iThisZ < CUBEAREASIZE)
						{
							// add or delete pattern
							bool bPreserveCube = false;
							#ifdef VRTECH
							if (bReverseOperation == true || iPaintMode != 1)
							#else
							if (bReverseOperation == true)
							#endif
							{
								// delete mode
								// only wipe out if same texture
								unsigned char cTexIndexA = (cBitCube & (15 << 4)) >> 4;
								unsigned char cTexIndexB = (pCubes[iThisX][iThisY][iThisZ] & (15 << 4)) >> 4;
								if (cTexIndexA == cTexIndexB)
									cBitCube = 0;
								else
									cBitCube = pCubes[iThisX][iThisY][iThisZ];

								// if in delete mode, cancel a delete if the preserve mode protects it
								if (cBitCube == 0 && ebebuild.Pattern.iPreserveMode == 1)
								{
									// check if 'wall for example' is next to a cube at Z+1/X+1 which is filled
									int iThisRotation = ebebuild.iCursorRotation;
									if (iThisRotation == 0)
										if (iThisZ + 1 < CUBEAREASIZE)
											if (pCubes[iThisX][iThisY][iThisZ + 1] != 0)
												bPreserveCube = true;
									if (iThisRotation == 1)
										if (iThisX + 1 < CUBEAREASIZE)
											if (pCubes[iThisX + 1][iThisY][iThisZ] != 0)
												bPreserveCube = true;
									if (iThisRotation == 2)
										if (iThisZ - 1 >= 0)
											if (pCubes[iThisX][iThisY][iThisZ - 1] != 0)
												bPreserveCube = true;
									if (iThisRotation == 3)
										if (iThisX - 1 >= 0)
											if (pCubes[iThisX - 1][iThisY][iThisZ] != 0)
												bPreserveCube = true;
								}
							}
							else
							{
								// add mode
								// if in add mode, cancel an addition if the preserve mode protects it
								// and the target is already filled in (an existing cube)
								if (pCubes[iThisX][iThisY][iThisZ] != 0)
								{
									if (ebebuild.Pattern.iPreserveMode == 1)
									{
										// check if 'wall for example' is next to a cube at Z+1/X+1 which is filled
										int iThisRotation = ebebuild.iCursorRotation;
										if (iThisRotation == 0)
											if (iThisZ + 1 < CUBEAREASIZE)
												if (pCubes[iThisX][iThisY][iThisZ + 1] != 0)
													bPreserveCube = true;
										if (iThisRotation == 1)
											if (iThisX + 1 < CUBEAREASIZE)
												if (pCubes[iThisX + 1][iThisY][iThisZ] != 0)
													bPreserveCube = true;
										if (iThisRotation == 2)
											if (iThisZ - 1 >= 0)
												if (pCubes[iThisX][iThisY][iThisZ - 1] != 0)
													bPreserveCube = true;
										if (iThisRotation == 3)
											if (iThisX - 1 >= 0)
												if (pCubes[iThisX - 1][iThisY][iThisZ] != 0)
													bPreserveCube = true;
									}
									if (ebebuild.Pattern.iPreserveMode == 2)
									{
										// check if 'stairs for example' should not add to anything already present
										if (pCubes[iThisX][iThisY][iThisZ] != 0)
											bPreserveCube = true;
									}
								}
							}

							// replace blank with current cube already in there
							if (bPreserveCube == true)
							{
								cBitCube = pCubes[iThisX][iThisY][iThisZ];
							}

							// proceed to replace cube if different in any way
							if (pCubes[iThisX][iThisY][iThisZ] != cBitCube)
							{
								pCubes[iThisX][iThisY][iThisZ] = cBitCube;
								if (iThisX < iRecordX1) iRecordX1 = iThisX;
								if (iThisY < iRecordY1) iRecordY1 = iThisY;
								if (iThisZ < iRecordZ1) iRecordZ1 = iThisZ;
								if (iThisX > iRecordX2) iRecordX2 = iThisX;
								if (iThisY > iRecordY2) iRecordY2 = iThisY;
								if (iThisZ > iRecordZ2) iRecordZ2 = iThisZ;
								bRecordChangeToCubeData = true;
							}

							// mark that we have written something to the cube data, just just clicked
							if (t.ebe.iWrittenCubeData == 1) t.ebe.iWrittenCubeData = 2;
						}
					}
				}
			}

			// only update mesh(es) if cube data actually changed
			if (bRecordChangeToCubeData == true)
			{
				// update within range of pattern stamp
				iRecordX1 = ((int)iRecordX1 / 10) * 10;
				iRecordY1 = ((int)iRecordY1 / 10) * 10;
				iRecordZ1 = ((int)iRecordZ1 / 10) * 10;
				iRecordX2 = ((int)iRecordX2 / 10) * 10;
				iRecordY2 = ((int)iRecordY2 / 10) * 10;
				iRecordZ2 = ((int)iRecordZ2 / 10) * 10;
				ebe_refreshmesh(ebebuild.iBuildObj, iRecordX1, iRecordY1, iRecordZ1, iRecordX2, iRecordY2, iRecordZ2);
			}
		}
	}
	else
	{
		// if off grid, and left click another EBE entity, switch to that one
		if (t.inputsys.mclick == 1)
		{
			int iFoundE = findentitycursorobj(-1);
			if (iFoundE > 0 && iFoundE != t.ebe.entityelementindex)
			{
				if (t.entityprofile[t.entityelement[iFoundE].bankindex].isebe != 0)
				{
					// change to new site
					ebe_newsite(iFoundE);
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

	// the object to optimize
	sObject* pObject = g_ObjectList[iObj];

	//Update master.
	int iEntityBankID = t.entityelement[e].bankindex;
	t.sourceobj = g.entitybankoffset + iEntityBankID;

	// ensure parent is updated (for things like extracting, lightmapping, etc)
	ebe_updateparent ( e );

	// Set collision of working object active so can be ray cast detected
	pObject = g_ObjectList[iObj];
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
		int secure_vertex_data = 62400; //PE: must be divideable by 24
		if (dwVertPos > secure_vertex_data)
			dwVertPos = secure_vertex_data; //PE: Max vertex. per material.
		DWORD dwIndicePos = (dwVertPos/2)*3;

		SetupMeshFVFData ( pMesh, pMesh->dwFVF, dwVertPos, dwIndicePos, false );
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
			if (dwVertPos >= secure_vertex_data) //PE: Make sure we never crash.
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

	// Apply textures
	#ifdef VRTECH
	 int iTexD = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.jpg")).Get());
	 int iTexN = loadinternaltexture("effectbank\\reloaded\\media\\blank_N.dds");
	 int iTexS = loadinternaltexture("effectbank\\reloaded\\media\\blank_medium_S.DDS");
	#else
	 int iTexD = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.dds")).Get());
	 int iTexN = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_N.dds")).Get());
	 int iTexS = loadinternaltexture(cstr(cstr("ebebank\\default\\") + ebe_constructlongTXPname("_S.dds")).Get());
	#endif
	t.entityprofile[iEntID].texdid = iTexD;
	t.entityprofile[iEntID].texnid = iTexN;
	t.entityprofile[iEntID].texsid = iTexS;
	#ifdef VRTECH
	TextureObject ( iObj, 0, iTexD );
	TextureObject ( iObj, 1, loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds") );
	TextureObject ( iObj, 2, iTexN );
	TextureObject ( iObj, 3, iTexS );
	TextureObject ( iObj, 4, loadinternaltexture("effectbank\\reloaded\\media\\materials\\0_Gloss.dds") );
	TextureObject ( iObj, 5, g.postprocessimageoffset+5 );
	TextureObject ( iObj, 6, loadinternaltexture("effectbank\\reloaded\\media\\blank_I.dds") );
	TextureObject ( iObj, 7, loadinternaltexture("effectbank\\reloaded\\media\\detail_default.dds") );
	#else
	//PE: for apbr_basic.fx
	TextureObject(iObj, 0, iTexD); //1=albedo
	TextureObject(iObj, 1, loadinternaltexture("effectbank\\reloaded\\media\\blank_O.dds")); //1=ao
	TextureObject(iObj, 2, iTexN); //2=normal
	TextureObject(iObj, 3, iTexS); //3=metalness spec is fine here.
	TextureObject(iObj, 4, loadinternaltexture("effectbank\\reloaded\\media\\white_D.dds")); //4=Gloss 
	TextureObject(iObj, 5, loadinternaltexture("effectbank\\reloaded\\media\\blank_black.dds")); //5=height
	TextureObject(iObj, 6, t.terrain.imagestartindex + 31); //6=env.
	TextureObject(iObj, 7, loadinternaltexture("effectbank\\reloaded\\media\\detail_default.dds")); //7 = illum/detail ...
	#endif
	SetObjectTransparency ( iObj, 0 );
	
	// now apply regular entity shader
	int iEffectIndex = loadinternaleffect("effectbank\\reloaded\\apbr_basic.fx");
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
		#ifdef ENABLEIMGUI
		#ifdef USELEFTPANELSTRUCTUREEDITOR
		ImGui::SetWindowFocus("Structure Editor##LeftPanel");
		#endif
		bBuilder_Properties_Window = true;
		#endif
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
			if (!bDisableAllSprites) 
			{
				for (int n = 0; n < 16; n++) if (SpriteExist(ebebuild.iMatSpr[n]) == 1) HideSprite(ebebuild.iMatSpr[n]);
				if (SpriteExist(ebebuild.iEBETexHelpSpr) == 1) HideSprite(ebebuild.iEBETexHelpSpr);
				if (SpriteExist(ebebuild.iEBEHelpSpr) == 1) HideSprite(ebebuild.iEBEHelpSpr);
				if (SpriteExist(ebebuild.iTexturePanelHighSprite) == 1) HideSprite(ebebuild.iTexturePanelHighSprite);
				for (int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++)
				{
					if (SpriteExist(ebebuild.iTexturePanelSprite[iTex]) == 1) HideSprite(ebebuild.iTexturePanelSprite[iTex]);
				}
			}
			if ( ObjectExist ( ebebuild.iToolObj ) == 1 ) HideObject ( ebebuild.iToolObj );
		}
		t.ebe.on = 0;
		#ifdef ENABLEIMGUI
		ImGui::SetWindowFocus(TABENTITYNAME);
		bBuilder_Properties_Window = false;
		#endif
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
		if (!bDisableAllSprites) {
			for (int n = 0; n < 16; n++) if (SpriteExist(ebebuild.iMatSpr[n]) == 1) ShowSprite(ebebuild.iMatSpr[n]);
			if (SpriteExist(ebebuild.iEBETexHelpSpr) == 1) ShowSprite(ebebuild.iEBETexHelpSpr);
			if (SpriteExist(ebebuild.iEBEHelpSpr) == 1) ShowSprite(ebebuild.iEBEHelpSpr);
			if (SpriteExist(ebebuild.iTexturePanelHighSprite) == 1) ShowSprite(ebebuild.iTexturePanelHighSprite);
			for (int iTex = 0; iTex < EBETEXPANELSPRMAX; iTex++)
			{
				if (SpriteExist(ebebuild.iTexturePanelSprite[iTex]) == 1) ShowSprite(ebebuild.iTexturePanelSprite[iTex]);
			}
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
		MessageBoxA ( hThisWnd, "Cannot find 'entitybank\\user\\ebestructures' folder", "Error", MB_OK | MB_TOPMOST );
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
		if ( MessageBoxA ( hThisWnd, pDisplayErrorMsg, "File Already Exists", MB_YESNO | MB_TOPMOST ) != IDYES )
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
	#ifdef VRTECH
	t.setuparr_s[9] = cstr("model         = ") + tNameOnly + ".x";
	t.setuparr_s[10]=cstr("textured      = ") + t.entityprofile[iEntID].texd_s.Get();
	t.setuparr_s[11]="effect        = effectbank\\reloaded\\entity_basic.fx";
	#else
	t.setuparr_s[ 9]=cstr("model         = ") + tNameOnly + ".dbo";
	t.setuparr_s[10]=cstr("textured      = ") + t.entityprofile[iEntID].texd_s;
	t.setuparr_s[11]="effect        = effectbank\\reloaded\\apbr_basic.fx"; // entity_basic.fx";
	#endif
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
		CopyFileA ( tBMPFilename.Get(), sBMPFile.Get(), FALSE );
	}

	// DBO
	int iSourceObj = g.entitybankoffset + iEntID;
	cstr tDBOFile = tRawPathAndFile + cstr(".dbo");
	if ( FileExist(tDBOFile.Get()) == 1 ) DeleteAFile ( tDBOFile.Get() );
	SaveObject ( tDBOFile.Get(), iSourceObj );

	// and X
	#ifdef VRTECH
	cstr tXFile = tRawPathAndFile + cstr(".x");
	if ( FileExist(tXFile.Get()) == 1 ) DeleteAFile ( tXFile.Get() );
	dbo2xConvert(tDBOFile.Get(), tXFile.Get());
	// and finally delete DBO now we have an X
	if ( FileExist(tDBOFile.Get()) == 1 ) DeleteAFile ( tDBOFile.Get() );
	#endif

	// DDS/JPG
	cstr tDDSSourceRaw = cstr("ebebank\\default\\") + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
	#ifdef VRTECH
	cstr tDDSSourceFilename = tDDSSourceRaw + "_D.jpg";
	#else
	cstr tDDSSourceFilename = tDDSSourceRaw + "_D.dds";
	#endif
	tRawPathAndFile = tRawPath + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
	if ( FileExist(tDDSSourceFilename.Get()) == 0 ) 
	{
		if ( strnicmp ( tRawPath.Get(), "levelbank", 9 ) != NULL )
		{
			tDDSSourceRaw = g.mysystem.levelBankTestMap_s + Left(t.entityprofile[iEntID].texd_s.Get(),strlen(t.entityprofile[iEntID].texd_s.Get())-6);
			#ifdef VRTECH
			tDDSSourceFilename = tDDSSourceRaw + "_D.jpg";
			#else
			tDDSSourceFilename = tDDSSourceRaw + "_D.dds";
			#endif
		}
	}
	if ( FileExist(tDDSSourceFilename.Get()) == 1 ) 
	{
		#ifdef VRTECH
		cstr sDDSFile = tRawPathAndFile + cstr("_D.jpg");
		#else
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		#endif
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFileA ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
		#ifdef VRTECH
		#else
		 tDDSSourceFilename = tDDSSourceRaw + "_N.dds";
		 sDDSFile = tRawPathAndFile + cstr("_N.dds");
		 if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		 CopyFile ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
		 tDDSSourceFilename = tDDSSourceRaw + "_S.dds";
		 sDDSFile = tRawPathAndFile + cstr("_S.dds");
		 if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		 CopyFile ( tDDSSourceFilename.Get(), sDDSFile.Get(), FALSE );
		#endif
	}
}

int ebe_loadcustomtexture ( int iEntityProfileIndex, int iWhichTextureOver )
{
	// Needed locals
	LPSTR pOldDir = GetDir();
	HWND hThisWnd = GetForegroundWindow();

	// must have textures image before we can customise it
	if ( ebebuild.iTexPlateImage == 0 )
		return 0;

	// EBE Load Folder
	t.strwork = g.fpscrootdir_s + "\\Files\\ebebank\\texturesource";
	if ( PathExist( t.strwork.Get() ) == 0 ) 
	{
		MessageBoxA ( hThisWnd, "Cannot find textures folder' folder", "Error", MB_OK | MB_TOPMOST );
		return 0;
	}

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
	t.statusbar_s = "Generating Building Editor Textures"; 
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
	#ifdef VRTECH
	#else
	if ( stricmp ( pFileExt.Get(), "_d.dds" ) == NULL )
		iTexSetsPossible = 3;
	#endif

	// go through texture subsets (D, N, S )
	for ( int iTexSet = 0; iTexSet < iTexSetsPossible; iTexSet++ )
	{
		// work out texture filename
		cstr pFilename = pLoadFilename;
		#ifdef VRTECH
		cstr pFileExt = cstr("_D.jpg");
		#else
		cstr pFileExt = cstr("_D.dds");
		#endif
		if ( iTexSetsPossible == 3 )
		{
			pFilename = Left ( pLoadFilename, strlen(pLoadFilename)-6 );
			#ifdef VRTECH
			if ( iTexSet == 0 ) pFileExt = cstr("_D.jpg");
			if ( iTexSet == 1 ) pFileExt = cstr("_N.jpg");
			if ( iTexSet == 2 ) pFileExt = cstr("_S.jpg");
			#else
			if ( iTexSet == 0 ) pFileExt = cstr("_D.dds");
			if ( iTexSet == 1 ) pFileExt = cstr("_N.dds");
			if ( iTexSet == 2 ) pFileExt = cstr("_S.dds");
			#endif
			pFilename = pFilename + pFileExt;
		}

		// check if the texture exists, else use backup blank
		if ( FileExist ( pFilename.Get() ) == 0 )
		{
			// file not exist, if N or S, substitute with blank
			if ( iTexSet == 1 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_N.dds";
			if ( iTexSet == 2 ) pFilename = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_black.dds";
		}

		// final destination of Textures subsets
		cstr sSavePathFileNonAbs = "ebebank\\default\\textures";
		cstr sSavePathFile = g.fpscrootdir_s + "\\Files\\" + sSavePathFileNonAbs;

		// check if texture to load exists
		#ifdef DX11
		// use terrain custom texture maker for inserting texture in textureplate
		cstr pPlateFilename = cstr(sSavePathFile) + pFileExt;
		terrain_createnewterraintexture ( pPlateFilename.Get(), iWhichTextureOver, pFilename.Get(), 1, 1 );

		// reload image with new file (apply auto-mipmapping when load)
		if ( iTexSet == 0 ) 
		{
			// texture used by textures
			#ifdef VRTECH
			LoadImage ( pPlateFilename.Get(), ebebuild.iTexPlateImage );
			#else
			LoadImage ( pPlateFilename.Get(), ebebuild.iTexPlateImage,0,g.gdividetexturesize);
			#endif
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
				//if(0)
				//{
				//	DestFormat = D3DXIFF_PNG;
				//	cstr pFilePNGExt;
				//	if ( iTexSet == 0 ) pFilePNGExt = cstr("_D.png");
				//	if ( iTexSet == 1 ) pFilePNGExt = cstr("_N.png");
				//	if ( iTexSet == 2 ) pFilePNGExt = cstr("_S.png");
				//	pSaveLocation = cstr(sSavePathFile) + pFilePNGExt;
				//	hRes = D3DXSaveSurfaceToFile( pSaveLocation.Get(), DestFormat, pPlateSurface, NULL, NULL );
				//}
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
			#ifdef VRTECH
			if ( t.entityprofile[entid].texdid > 0 ) removeinternaltexture(t.entityprofile[entid].texdid);
			#endif
			t.entityprofile[entid].texpath_s = "ebebank\\default\\";
			#ifdef VRTECH
			t.entityprofile[entid].texd_s = ebe_constructlongTXPname("_D.jpg");
			#else
			t.entityprofile[entid].texd_s = ebe_constructlongTXPname("_D.dds");
			#endif
			image_setlegacyimageloading(true);
			cstr tthistexdir_s = t.entityprofile[entid].texpath_s + t.entityprofile[entid].texd_s;
			if ( t.entityprofile[entid].transparency == 0 ) 
				t.entityprofile[entid].texdid = loadinternaltextureex(tthistexdir_s.Get(),1,0);
			else
				t.entityprofile[entid].texdid = loadinternaltextureex(tthistexdir_s.Get(),5,0);
			image_setlegacyimageloading(false);
			#ifdef VRTECH
			 t.entityprofile[entid].texnid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_N.dds",5,0);
			 t.entityprofile[entid].texsid = loadinternaltextureex("effectbank\\reloaded\\media\\blank_medium_S.DDS",1,0);
			#else
			 tthistexdir_s = t.entityprofile[entid].texpath_s + ebe_constructlongTXPname("_N.dds");
			 t.entityprofile[entid].texnid = loadinternaltextureex(tthistexdir_s.Get(),5,0);
			 tthistexdir_s = t.entityprofile[entid].texpath_s + ebe_constructlongTXPname("_S.dds");
			 t.entityprofile[entid].texsid = loadinternaltextureex(tthistexdir_s.Get(),1,0);
			#endif

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
					if (ebebuild.TXP.iWidth > 0 && ebebuild.TXP.iHeight > 0)
					{
						int ncount = ebebuild.TXP.iWidth * ebebuild.TXP.iHeight;
						if (ncount > 0)
						{
							for (int n = 0; n < ncount; n++)
							{
								// Texture filename
								cstr sNum = cstr(100 + n);
								t.tryfield_s = cStr("t") + cStr(Right(sNum.Get(), strlen(sNum.Get()) - 1));
								if (t.field_s == t.tryfield_s) ebebuild.TXP.sTextureFile[n] = Lower(t.value_s.Get());

								// Material Index
								t.tryfield_s = cStr("m") + cStr(Right(sNum.Get(), strlen(sNum.Get()) - 1));
								if (t.field_s == t.tryfield_s) ebebuild.TXP.iMaterialRef[n] = t.value1;
							}
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
		t.setuparr_s[iLine]=cstr("Height = ") + ebebuild.TXP.iHeight; iLine++;
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
	#ifdef VRTECH
	cstr sLongFilename = cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.jpg");
	#else
	cstr sLongFilename = cstr("ebebank\\default\\") + ebe_constructlongTXPname("_D.dds");
	#endif
	cstr tRawPathAndFile = cstr(Left(sLongFilename.Get(),strlen(sLongFilename.Get())-6));
	#ifdef VRTECH
	cstr tDDSFilename = "ebebank\\default\\textures_D.jpg";
	#else
	cstr tDDSFilename = "ebebank\\default\\textures_D.dds";
	#endif
	if ( FileExist(tDDSFilename.Get()) == 1 ) 
	{
		#ifdef VRTECH
		cstr sDDSFile = tRawPathAndFile + cstr("_D.jpg");
		#else
		cstr sDDSFile = tRawPathAndFile + cstr("_D.dds");
		#endif
		if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		CopyFileA ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
		#ifdef VRTECH
		#else
		 tDDSFilename = "ebebank\\default\\textures_N.dds";
		 sDDSFile = tRawPathAndFile + cstr("_N.dds");
		 if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		 CopyFile ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
		 tDDSFilename = "ebebank\\default\\textures_S.dds";
		 sDDSFile = tRawPathAndFile + cstr("_S.dds");
		 if ( FileExist(sDDSFile.Get()) == 1 ) DeleteAFile ( sDDSFile.Get() );
		 CopyFile ( tDDSFilename.Get(), sDDSFile.Get(), FALSE );
		#endif
	}
}

#ifdef VRTECH
void ebe_restoreebedefaulttextures(void)
{
	char pRealDestPath[MAX_PATH];
	char pRealDestPathAndFile[MAX_PATH];
	sprintf(pRealDestPath, "ebebank\\default\\");

	// ensure when start new level/etc, the old EBE textures are reset!
	sprintf(pRealDestPathAndFile, "%stextures_profile.txp", pRealDestPath);
	CopyFileA("ebebank\\default\\original_profile.txp", pRealDestPathAndFile,FALSE);
	CopyFileA("ebebank\\default\\original_D.jpg", "ebebank\\default\\textures_D.jpg",FALSE);

	// also free if EBE already active (to erase old plate texture)
	if (t.ebe.active == 1) ebe_free();
}
#endif

int ebe_createnewstructuretexture ( LPSTR pDestTerrainTextureFile, int iWhichTextureOver, LPSTR pTexFileToLoad, int iSeamlessMode, int iCompressIt )
{
	return ImageCreateTexturePlate( pDestTerrainTextureFile, iWhichTextureOver, pTexFileToLoad, iSeamlessMode, iCompressIt);
}

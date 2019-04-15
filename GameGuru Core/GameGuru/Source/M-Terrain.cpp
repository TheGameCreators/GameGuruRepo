//----------------------------------------------------
//--- GAMEGURU - M-Terrain
//----------------------------------------------------
#include "gameguru.h"
#include "..\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Objects\ShadowMapping\cShadowMaps.h"
#include "DirectXTex.h"
using namespace DirectX;

// shadow mapping
extern CascadedShadowsManager g_CascadedShadow;

// Terrain Build Globals
#define TERRAINTEXPANELSPRMAX 6

// Terrain Build Local Structure
struct terrainbuildtype
{
	int initialised;
	int active;
	bool bReleaseMouseFirst;
	bool bUsingTerrainTextureSystemPaintSelector;
	bool bCustomiseTexture;
	cstr terrainstyle;
	int iCurrentTexture;
	int iTexturePanelSprite[6];
	int iTexturePanelImg[6];
	int iTexturePanelX;
	int iTexturePanelY;
	int iTexturePanelWidth;
	int iTexturePanelHeight;
	int iTexturePanelHighSprite;
	int iTexturePanelHighImg;
	int iHelpSpr;
	int iHelpImg;
	int iTexHelpSpr;
	int iTexHelpImg;
	int iTexPlateImage;
};
terrainbuildtype terrainbuild;

void terrain_initstyles ( void )
{
	// Init terrain work bitmap (so save level does not crash due to memory creation)
	if ( BitmapExist(g.terrainworkbitmapindex) == 0 ) 
	{
		// bitmap to perform pixel work
		CreateBitmap (  g.terrainworkbitmapindex,MAXTEXTURESIZE,MAXTEXTURESIZE );
		SetCurrentBitmap (  0 );

		// create array used later for creating terrain super texture
		//Dim3( t.pot, 5, 3, 3  ); // 280317 - 16 textures per terrain
		Dim3( t.pot, 16, 3, 3 );
	}

	//  Set occlusion style (hardware assisted)
	SetOcclusionMode (  1 );

	//  Choose terrain style
	g.terrainstylemax=0;
	g.terrainstyle_s="common";
	SetDir (  "terrainbank" );
	ChecklistForFiles (  );

	for ( t.c = 1 ; t.c <= ChecklistQuantity(); t.c++ )
	{
		if ( t.c == 1 )
		{
			// first one is CUSTOM to select custom terrain texture from terrain panel
			++g.terrainstylemax;
			Dim (  t.terrainstylebank_s,g.terrainstylemax  );
			t.terrainstylebank_s[g.terrainstylemax]="CUSTOM";
		}
		t.tfile_s=ChecklistString(t.c);
		if (  ChecklistValueA(t.c) == 1 ) 
		{
			if (  t.tfile_s.Get()[0] != '.' ) 
			{
				++g.terrainstylemax;
				Dim (  t.terrainstylebank_s,g.terrainstylemax  );
				t.terrainstylebank_s[g.terrainstylemax]=Lower(t.tfile_s.Get());
			}
		}
	}
	SetDir (  ".." );

	//  Collect vegetation styles
	g.vegstylemax=0;
	SetDir (  "vegbank" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=ChecklistString(t.c);
		if (  ChecklistValueA(t.c) == 1 ) 
		{
			if (  t.tfile_s.Get()[0] != '.' ) 
			{
				++g.vegstylemax;
				Dim (  t.vegstylebank_s,g.vegstylemax );
				t.vegstylebank_s[g.vegstylemax]=Lower(t.tfile_s.Get());
			}
		}
	}
	SetDir (  ".." );

	//  small lookup for memblock painting circles
	Dim ( t.curve_f,100 );
	for ( t.r = 0 ; t.r <= 180; t.r++ )
	{
		t.trx_f = Cos(t.r-90)*100.0;
		t.trz_f = Sin(t.r-90)*100.0;
		t.curve_f [ int((100+t.trz_f)/2) ] = t.trx_f/100.0;
	}

	//  Choose default terrin and veg styles (also called when new level)
	terrain_initstyles_reset ( );
}

void terrain_initstyles_reset ( void )
{
	//  style file chooses default style to use
	t.tfile_s="terrainbank\\style.txt";
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		OpenToRead (  1,t.tfile_s.Get() );
		g.terrainstyle_s = ReadString ( 1 );
		CloseFile (  1 );
	}
	if (  PathExist( cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() ) == 0 ) 
	{
		g.terrainstyle_s=t.terrainstylebank_s[1];
	}
	//  find terrainstyle index
	for ( g.terrainstyleindex = 1 ; g.terrainstyleindex<=  g.terrainstylemax; g.terrainstyleindex++ )
	{
		if (  cstr(Lower(g.terrainstyle_s.Get())) == t.terrainstylebank_s[g.terrainstyleindex] ) 
		{
			break;
		}
	}
	// 080517 - if exceed array (i.e not found) reset to last slot
	if ( g.terrainstyleindex > g.terrainstylemax )
	{
		g.terrainstyleindex = g.terrainstylemax;
		g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
	}

	//  Choose default veg style
	g.vegstyle_s="lushy";
	if (  PathExist( cstr(cstr("vegbank\\")+g.vegstyle_s).Get() ) == 0 ) 
	{
		g.vegstyle_s=t.vegstylebank_s[1];
	}
	//  find vegstyle index
	for ( g.vegstyleindex = 1 ; g.vegstyleindex<=  g.vegstylemax; g.vegstyleindex++ )
	{
		if (  cstr(Lower(g.vegstyle_s.Get())) == t.vegstylebank_s[g.vegstyleindex] ) 
		{
			break;
		}
	}
	// 080517 - if exceed array (i.e not found) reset to last slot
	if ( g.vegstyleindex > g.vegstylemax )
	{
		g.vegstyleindex = g.vegstylemax;
		g.vegstyle_s = t.vegstylebank_s[g.vegstyleindex];
	}
}

void terrain_setupedit ( void )
{
	// Water plane (use proper reflect/refract later)
	//LoadImage (  "effectbank\\reloaded\\media\\water.png",t.terrain.imagestartindex+0,0,g.gdividetexturesize );
	LoadImage (  "effectbank\\reloaded\\media\\water.dds",t.terrain.imagestartindex+0,0,g.gdividetexturesize );
	MakeObjectPlane (  t.terrain.objectstartindex+2,200000,200000 );
	XRotateObject (  t.terrain.objectstartindex+2,90 );
	SetObjectLight (  t.terrain.objectstartindex+2,0 );
	SetObjectCull (  t.terrain.objectstartindex+2,0 );
	TextureObject (  t.terrain.objectstartindex+2,t.terrain.imagestartindex+0 );
	ScaleObjectTexture (  t.terrain.objectstartindex+2,1000,1000 );
	SetAlphaMappingOn (  t.terrain.objectstartindex+2,50 );

	// Water Effect Shader earlier
	if ( GetEffectExist ( t.terrain.effectstartindex+1 ) == 0 )
	{
		LoadEffect ( "effectbank\\reloaded\\water_basic.fx",t.terrain.effectstartindex+1,0 );
		t.effectparam.water.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+1,"HudFogDist");
		t.effectparam.water.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+1,"HudFogColor");
	}
	SetObjectEffect ( t.terrain.objectstartindex+2, t.terrain.effectstartindex+1 );
	SetEffectTechnique ( t.terrain.effectstartindex+1, "Editor" );

	// Terrain Edit Settings
	t.terrain.zoom_f=0.4f;
	t.terrain.camx_f=(1024*50)/2;
	t.terrain.camz_f=(1024*50)/2;
	t.terrain.terrainpaintermode=1;
	t.terrain.lastterrainpaintermode=1;
	t.terrain.terrainlevel_f=750.0f;
	t.terrain.RADIUS_f=250.0f;
	SetCameraRange (  100,55000 );

	// Reset undo buffer
	t.terrainundo.bufferfilled=0;
	t.terrainundo.mode=0;

	// Initial quick test player position
	t.terrain.playerx_f=25000;
	t.terrain.playerz_f=25000;
	t.terrain.playerax_f=0;
	t.terrain.playeray_f=0;
	t.terrain.playeraz_f=0;
}

cstr terrain_getterrainfolder ( void )
{
	g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
	cstr sTerrainTextureLocation = cstr(cstr("terrainbank\\")+g.terrainstyle_s);
	if ( g.terrainstyleindex == 1 ) sTerrainTextureLocation = g.mysystem.levelBankTestMap_s.Get(); //"levelbank\\testmap";
	return sTerrainTextureLocation;
}

void terrain_paintselector_init ( void )
{
	// quit early if in F9 editing mode
	if ( t.conkit.editmodeactive != 0 )  
		return;

	// Create texture selection panel
	terrainbuild.iTexPlateImage = 0;
	terrainbuild.iTexturePanelX = GetChildWindowWidth()-210;
	terrainbuild.iTexturePanelY = GetChildWindowHeight()-210;
	terrainbuild.iTexturePanelWidth = 200;
	terrainbuild.iTexturePanelHeight = 200;
	for ( int iTex = 0; iTex < TERRAINTEXPANELSPRMAX; iTex++ )
	{
		LPSTR pTexImg = "";
		int iX = terrainbuild.iTexturePanelX;
		int iY = terrainbuild.iTexturePanelY;
		int iWidth = terrainbuild.iTexturePanelWidth;
		int iHeight = terrainbuild.iTexturePanelHeight;
		if ( iTex==0 ) { pTexImg = "TexHUD-F.png"; iX -= 10; iY -= 10; iWidth += 20; iHeight += 20; }
		if ( iTex==1 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY -= 10; iWidth += 20; iHeight = 1; }
		if ( iTex==2 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY += 209; iWidth += 20; iHeight = 1; }
		if ( iTex==3 ) { pTexImg = "TexHUD-L.png"; iX -= 10; iY -= 10; iWidth = 1; iHeight += 20; }
		if ( iTex==4 ) { pTexImg = "TexHUD-L.png"; iX += 209; iY -= 10; iWidth = 1; iHeight += 20; }
		if ( iTex==5 ) 
		{ 
			// terrain texture plate
			cstr sTerrainTextureLocation = terrain_getterrainfolder();
			terrainbuild.iTexturePanelImg[iTex] = loadinternalimage(cstr(sTerrainTextureLocation+"\\Texture_D.dds").Get());
			terrainbuild.terrainstyle = g.terrainstyle_s;
		}
		else
		{
			// UI graphics
			terrainbuild.iTexturePanelImg[iTex] = loadinternalimage(cstr(cstr("terrainbuild\\default\\")+cstr(pTexImg)).Get());
		}
		terrainbuild.iTexturePanelSprite[iTex] = g.terrainpainterinterfacesprite + 31 + iTex;
		Sprite ( terrainbuild.iTexturePanelSprite[iTex], iX, iY, terrainbuild.iTexturePanelImg[iTex] );
		SizeSprite ( terrainbuild.iTexturePanelSprite[iTex], iWidth, iHeight );
		if ( iTex==5 ) 
		{
			terrainbuild.iTexPlateImage = terrainbuild.iTexturePanelImg[iTex];
			SetSprite ( terrainbuild.iTexturePanelSprite[iTex], 0, 0 );
		}
	}

	// Texture highlighter
	terrainbuild.iTexturePanelHighSprite = g.terrainpainterinterfacesprite + 0;
	terrainbuild.iTexturePanelHighImg = loadinternalimage("terrainbuild\\default\\TextureHighlighter.dds");
	Sprite ( terrainbuild.iTexturePanelHighSprite, terrainbuild.iTexturePanelX, terrainbuild.iTexturePanelY, terrainbuild.iTexturePanelHighImg );
	SizeSprite ( terrainbuild.iTexturePanelHighSprite, terrainbuild.iTexturePanelWidth/4, terrainbuild.iTexturePanelHeight/4 );

	// Help Dialog Shortcut Keys
	terrainbuild.iHelpSpr = g.terrainpainterinterfacesprite + 1;
	terrainbuild.iHelpImg = loadinternalimage("languagebank\\english\\artwork\\terrainbuild-help.png");
	Sprite ( terrainbuild.iHelpSpr, terrainbuild.iTexturePanelX - ImageWidth(terrainbuild.iHelpImg) - 10, terrainbuild.iTexturePanelY + 210 - ImageHeight(terrainbuild.iHelpImg), terrainbuild.iHelpImg );

	// Help Dialog Shortcut Keys
	terrainbuild.iTexHelpSpr = g.terrainpainterinterfacesprite + 2;
	terrainbuild.iTexHelpImg = loadinternalimage("languagebank\\english\\artwork\\terrainbuild-texturehelp.png");
	Sprite ( terrainbuild.iTexHelpSpr, terrainbuild.iTexturePanelX - 10, terrainbuild.iTexturePanelY - 10 - ImageHeight(terrainbuild.iTexHelpImg), terrainbuild.iTexHelpImg );

	// terrain paint selector inited
	terrainbuild.initialised = 1;
	terrainbuild.active = 1;
}

void terrain_paintselector_hide ( void )
{
	if ( terrainbuild.active == 1 )
	{
		// hide any UI elements
		if ( terrainbuild.iTexturePanelSprite[0] > 0 )
		{
			if ( SpriteExist ( terrainbuild.iTexHelpSpr ) == 1 ) HideSprite ( terrainbuild.iTexHelpSpr );
			if ( SpriteExist ( terrainbuild.iHelpSpr ) == 1 ) HideSprite ( terrainbuild.iHelpSpr );
			if ( SpriteExist ( terrainbuild.iTexturePanelHighSprite ) == 1 ) HideSprite ( terrainbuild.iTexturePanelHighSprite );
			for ( int iTex = 0; iTex < TERRAINTEXPANELSPRMAX; iTex++ )
			{
				if ( SpriteExist ( terrainbuild.iTexturePanelSprite[iTex] ) == 1 ) HideSprite ( terrainbuild.iTexturePanelSprite[iTex] );
			}
		}
		terrainbuild.active = 0;
	}
}

void terrain_resetfornewlevel ( void )
{
	terrainbuild.terrainstyle = "";
}

void terrain_paintselector_show ( void )
{
	// quit early if in F9 editing mode
	if ( t.conkit.editmodeactive != 0 )  
		return;

	if ( terrainbuild.active == 0 )
	{
		// show UI elements
		if ( terrainbuild.iTexturePanelSprite[0] > 0 )
		{
			if ( SpriteExist ( terrainbuild.iTexHelpSpr ) == 1 ) ShowSprite ( terrainbuild.iTexHelpSpr );
			if ( SpriteExist ( terrainbuild.iHelpSpr ) == 1 ) ShowSprite ( terrainbuild.iHelpSpr );
			if ( SpriteExist ( terrainbuild.iTexturePanelHighSprite ) == 1 ) ShowSprite ( terrainbuild.iTexturePanelHighSprite );
			for ( int iTex = 0; iTex < TERRAINTEXPANELSPRMAX; iTex++ )
			{
				if ( SpriteExist ( terrainbuild.iTexturePanelSprite[iTex] ) == 1 ) ShowSprite ( terrainbuild.iTexturePanelSprite[iTex] );
			}
		}
		terrainbuild.active = 1;

		// if terrain style changed, load correct terrain texture into UI
		cstr sTerrainTextureLocation = terrain_getterrainfolder();
		if ( strcmp ( terrainbuild.terrainstyle.Get(), g.terrainstyle_s.Get() ) != NULL )
		{
			terrainbuild.terrainstyle = g.terrainstyle_s;
			LoadImage ( cstr(sTerrainTextureLocation+"\\Texture_D.dds").Get(), terrainbuild.iTexPlateImage );
			int iX = terrainbuild.iTexturePanelX;
			int iY = terrainbuild.iTexturePanelY;
			Sprite ( terrainbuild.iTexturePanelSprite[5], iX, iY, terrainbuild.iTexturePanelImg[5] );
			SetSprite ( terrainbuild.iTexturePanelSprite[5], 0, 0 );
		}
	}
}

void terrainbuild_settexturehighlight ( void )
{
	if ( terrainbuild.iTexturePanelHighSprite > 0 )
	{
		int iRow = terrainbuild.iCurrentTexture / 4;
		int iCol = terrainbuild.iCurrentTexture - (iRow*4);
		float fChoiceWidth = terrainbuild.iTexturePanelWidth/4;
		float fChoiceHeight = terrainbuild.iTexturePanelHeight/4;
		Sprite ( terrainbuild.iTexturePanelHighSprite, terrainbuild.iTexturePanelX+(iCol*fChoiceWidth), terrainbuild.iTexturePanelY+(iRow*fChoiceHeight), terrainbuild.iTexturePanelHighImg );
	}
}

void terrain_paintselector_control ( void )
{
	// manage terrain texture paint selector
	// ensure system is initialised
	if ( terrainbuild.initialised == 0 ) 
	{
		if ( t.conkit.editmodeactive == 0 ) 
		{
			// don't show if this gets triggered via F9 mode
			pastebitmapfontcenter("PREPARING TEXTURE PAINTER",GetChildWindowWidth()/2,40,1,255) ; Sync (  );
		}
		terrain_paintselector_init();
	}
	terrain_paintselector_show();

	// Only when release mouse continue
	if ( terrainbuild.bReleaseMouseFirst == true && t.inputsys.mclick != 0 ) return;
	terrainbuild.bReleaseMouseFirst = false;

	// Reason this is above action for selecting customise is to allow texture highlight to show as selected
	if ( terrainbuild.bCustomiseTexture == true && t.inputsys.mclick == 0 )
	{
		// load from terrainbank or custom from levelbank\testmap
		g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
		char pThisTerrainTexturePath[512];
		strcpy ( pThisTerrainTexturePath, cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() );
		if ( g.terrainstyleindex == 1 ) strcpy ( pThisTerrainTexturePath, g.mysystem.levelBankTestMap_s.Get() ); //"levelbank\\testmap" );
		if ( PathExist( pThisTerrainTexturePath ) == 1 ) 
		{
			// check if new terrain texture system file available, and create if not
			char pOldTerrainTextureFile[512];
			strcpy ( pOldTerrainTextureFile, cstr(cstr(pThisTerrainTexturePath)+cstr("\\Texture_D.dds")).Get() );
			if ( FileExist ( pOldTerrainTextureFile ) == 1 )
			{
				// switch to using CUSTOM 
				if ( g.terrainstyleindex != 1 )
				{
					t.visuals.terrainindex = 1;
					t.visuals.terrain_s = "CUSTOM";
					g.terrainstyleindex = t.visuals.terrainindex;
					g.terrainstyle_s = t.visuals.terrain_s;
					char pNewLocationForFile[512];
					//strcpy ( pNewLocationForFile, "levelbank\\testmap\\Texture_D.dds" );
					strcpy ( pNewLocationForFile, cstr(g.mysystem.levelBankTestMap_s+"Texture_D.dds").Get() );
					if ( FileExist ( pNewLocationForFile ) == 1 ) DeleteFile ( pNewLocationForFile );
					CopyFile ( pOldTerrainTextureFile, pNewLocationForFile, FALSE );
					strcpy ( pOldTerrainTextureFile, cstr(cstr(pThisTerrainTexturePath)+cstr("\\Texture_N.dds")).Get() );
					//strcpy ( pNewLocationForFile, "levelbank\\testmap\\Texture_N.dds" );
					strcpy ( pNewLocationForFile, cstr(g.mysystem.levelBankTestMap_s+"Texture_N.dds").Get() );
					if ( FileExist ( pNewLocationForFile ) == 1 ) DeleteFile ( pNewLocationForFile );
					CopyFile ( pOldTerrainTextureFile, pNewLocationForFile, FALSE );
					//strcpy ( pOldTerrainTextureFile, "levelbank\\testmap\\Texture_D.dds" );
					strcpy ( pOldTerrainTextureFile, cstr(g.mysystem.levelBankTestMap_s+"Texture_D.dds").Get() );
				}

				// change this texture slot with a new one
				bool bMustClosePopup = false;
				terrainbuild.bCustomiseTexture = false;
				if ( terrain_loadcustomtexture ( pOldTerrainTextureFile, terrainbuild.iCurrentTexture ) == 1 )
				{
					// successfully changed the texture
					LoadImage ( pOldTerrainTextureFile, terrainbuild.iTexPlateImage );
					int iX = terrainbuild.iTexturePanelX;
					int iY = terrainbuild.iTexturePanelY;
					Sprite ( terrainbuild.iTexturePanelSprite[5], iX, iY, terrainbuild.iTexturePanelImg[5] );

					// also re-texture current terrain with change too
					terrain_loadlatesttexture ( );

					// generate super texture palette from above NEW texture
					terrain_deletesupertexturepalette();

					// must close popup
					bMustClosePopup = true;
				}

				// close popup if used
				if ( bMustClosePopup == true )
				{
					// Clear status Text
					t.statusbar_s = ""; popup_text_close();
				}
			}
		}
	}

	// Select texture if in Texture Panel or Customise one
	int iRealSprMouseX = (GetChildWindowWidth()/800.0f) * t.inputsys.xmouse;
	int iRealSprMouseY = (GetChildWindowHeight()/600.0f) * t.inputsys.ymouse;
	if ( t.inputsys.mclick > 0 )
	{
		if ( iRealSprMouseX > terrainbuild.iTexturePanelX && iRealSprMouseX < terrainbuild.iTexturePanelX + terrainbuild.iTexturePanelWidth )
		{
			if ( iRealSprMouseY > terrainbuild.iTexturePanelY && iRealSprMouseY < terrainbuild.iTexturePanelY + terrainbuild.iTexturePanelHeight )
			{
				// while tile
				float fWhichCol = (float)(iRealSprMouseX - terrainbuild.iTexturePanelX) / (float)terrainbuild.iTexturePanelWidth;
				float fWhichRow = (float)(iRealSprMouseY - terrainbuild.iTexturePanelY) / (float)terrainbuild.iTexturePanelHeight;
				int iWhichTextureOver = (((int)(fWhichRow*4))*4) + (int)(fWhichCol*4);

				// select texture choice
				terrainbuild.iCurrentTexture = iWhichTextureOver;
				terrainbuild_settexturehighlight();

				// and if it was right mouse, customise this texture too
				if ( t.inputsys.mclick == 2 )
				{
					// replace texture within texture atlas
					terrainbuild.bCustomiseTexture = true;
				}

				// ensure we do not write into texture painter if selecting texture
				t.inputsys.mclick = 0;
				t.mc = 0;
			}
		}
	}
}

int terrain_loadcustomtexture ( LPSTR pDestPathAndFile, int iTextureSlot )
{
	// Needed locals
	LPSTR pOldDir = GetDir();
	HWND hThisWnd = GetForegroundWindow();

	// terrain build Load Folder
	t.strwork = g.fpscrootdir_s + "\\Files\\terrainbuild\\texturesource";
	if ( PathExist( t.strwork.Get() ) == 0 ) 
	{
		MessageBox ( hThisWnd, "Cannot find 'terrainbuild\texturesource' folder", "Error", MB_OK | MB_TOPMOST );
		return 0;
	}
	SetDir ( t.strwork.Get() );

	//  Ask for save filename
	cStr tLoadFile = "";
	cStr tLoadMessage = "Replace with custom texture";
	//tLoadFile = openFileBox("Diffuse File (_D.dds)|*.dds|Texture File (.dds)|*.dds|All Files|*.*|", t.strwork.Get(), tLoadMessage.Get(), ".dds", IMPORTERSAVEFILE);
	tLoadFile = openFileBox("Diffuse File (.png)|*.png|Texture File (.dds)|*.dds|All Files|*.*|", t.strwork.Get(), tLoadMessage.Get(), ".png", IMPORTERSAVEFILE);
	if ( tLoadFile == "Error" )
	{
		SetDir(pOldDir);
		return 0;
	}

	// Use large prompt
	t.statusbar_s = "Generating New Terrain Texture"; 
	popup_text(t.statusbar_s.Get());

	// strip file from path
	char pPathOnly[512];
	strcpy ( pPathOnly, pDestPathAndFile );
	for ( int n = strlen(pDestPathAndFile); n > 0 ; n-- )
	{
		if ( pDestPathAndFile[n] == '\\' || pDestPathAndFile[n] == '/' )
		{
			pPathOnly[n+1] = 0;
			break;
		}
	}

	// final destination of Texture Plate subsets (from current terrain)
	cstr sSavePathFile = g.fpscrootdir_s + "\\Files\\" + pPathOnly;
	SetDir(sSavePathFile.Get());

	// replace texture in plate with provided custom one
	terrain_createnewterraintexture ( "texture_D.dds", iTextureSlot, tLoadFile.Get(), 0, 1 );

	// if a normal map exists for it, use that too
	cstr tLoadNormalFile = tLoadFile;
	tLoadNormalFile = cstr(Left(tLoadNormalFile.Get(),strlen(tLoadNormalFile.Get())-6)) + "_N.dds";
	if ( FileExist ( tLoadNormalFile.Get() ) == 0 )
	{
		tLoadNormalFile = g.fpscrootdir_s + "\\Files\\effectbank\\reloaded\\media\\blank_N.dds";
	}
	terrain_createnewterraintexture ( "texture_N.dds", iTextureSlot, tLoadNormalFile.Get(), 0, 0 );

	// restore current folder
	SetDir(pOldDir);

	// success
	return 1;
}

int terrain_createnewterraintexture ( LPSTR pDestTerrainTextureFile, int iWhichTextureOver, LPSTR pTexFileToLoad, int iSeamlessMode, int iCompressIt )
{
	#ifdef DX11
	// check if texture to load exists
	GGIMAGE_INFO finfo;
	HRESULT hr = D3DX11GetImageInfoFromFile( pTexFileToLoad, NULL, &finfo, NULL );

	// filenames to WCHAR
	wchar_t wFilenameInsert[512];
	wchar_t wFilenamePlate[512];
	cstr pPlateFilename = cstr(pDestTerrainTextureFile);
	MultiByteToWideChar(CP_ACP, 0, pTexFileToLoad, -1, wFilenameInsert, sizeof(wFilenameInsert));
	MultiByteToWideChar(CP_ACP, 0, pPlateFilename.Get(), -1, wFilenamePlate, sizeof(wFilenamePlate));

	// is insert a DDS or other
	bool bInsertTextureIsDDS = false;
	if ( strnicmp ( pTexFileToLoad + strlen(pTexFileToLoad) - 4, ".dds", 4 ) == NULL )
		bInsertTextureIsDDS = true;

	// create and load the texture selected
	ScratchImage imageTextureToInsert;
	ScratchImage imageTexturePlate;
	ScratchImage convertedTextureToInsert;
	ScratchImage convertedTexturePlate;
	LPGGTEXTURE pLoadedTexSurface1024 = NULL;
	LPGGTEXTURE pLoadedTexSurface512 = NULL;
	LPGGTEXTURE pLoadedTexSurface256512 = NULL;
	LPGGTEXTURE pLoadedTexSurface512256 = NULL;
	LPGGTEXTURE pPlateSurface = NULL;
	if ( hr == S_OK )
	{
		// create/load texture to be inserted
		TexMetadata insertdata;
		if ( bInsertTextureIsDDS == true )
		{
			hr = GetMetadataFromDDSFile( wFilenameInsert, DDS_FLAGS_NONE, insertdata );			
			hr = LoadFromDDSFile( wFilenameInsert, DDS_FLAGS_NONE, &insertdata, imageTextureToInsert );
		}
		else
		{
			hr = GetMetadataFromWICFile( wFilenameInsert, DDS_FLAGS_NONE, insertdata );			
			hr = LoadFromWICFile( wFilenameInsert, DDS_FLAGS_NONE, &insertdata, imageTextureToInsert );
		}
		if ( SUCCEEDED(hr) )
		{
			// create/load texture of plate
			TexMetadata platedata;
			hr = GetMetadataFromDDSFile( wFilenamePlate, DDS_FLAGS_NONE, platedata );			
			if ( hr != S_OK )
			{
				// if plate file not exist, create one and provide dimensions
				finfo.Width = 4096;
				finfo.Height = 4096;

				// create the plate from fresh
				//imageTexturePlate.Initialize2D ( DXGI_FORMAT_BC5_UNORM, finfo.Width, finfo.Height, 1, 1, 0 ); 
				imageTexturePlate.Initialize2D ( DXGI_FORMAT_BC5_UNORM, finfo.Width, finfo.Height, 1, 13, 0 ); 
				platedata = imageTexturePlate.GetMetadata();
			}
			else
			{
				// existing plate exists, load it in
				hr = LoadFromDDSFile( wFilenamePlate, DDS_FLAGS_NONE, &platedata, imageTexturePlate );
				if ( FAILED(hr) ) platedata.width = 0;
			}
			if ( platedata.width > 0 )
			{
				// ensure we convert compressed textures to uncompressed ones
				ScratchImage* pWrkImage = &imageTextureToInsert;
				if ( imageTextureToInsert.GetMetadata().format >= DXGI_FORMAT_BC1_TYPELESS && imageTextureToInsert.GetMetadata().format <= DXGI_FORMAT_BC5_SNORM )
				{
					hr = Decompress( imageTextureToInsert.GetImages(), imageTextureToInsert.GetImageCount(), imageTextureToInsert.GetMetadata(),
						DXGI_FORMAT_B8G8R8A8_UNORM, convertedTextureToInsert );
					pWrkImage = &convertedTextureToInsert;
				}

				// resize
				ScratchImage InsertImage1024;
				ScratchImage InsertImage512;
				ScratchImage InsertImage256512;
				ScratchImage InsertImage512256;
				hr = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), 1024, 1024, TEX_FILTER_SEPARATE_ALPHA, InsertImage1024 );
				CreateTexture(m_pD3D, InsertImage1024.GetImages(), InsertImage1024.GetImageCount(), InsertImage1024.GetMetadata(), &pLoadedTexSurface1024 );
				if ( iSeamlessMode == 0 )
				{
					// 512x512 at center
					hr = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), 512, 512, TEX_FILTER_SEPARATE_ALPHA, InsertImage512 );
					CreateTexture(m_pD3D, InsertImage512.GetImages(), InsertImage512.GetImageCount(), InsertImage512.GetMetadata(), &pLoadedTexSurface512 );
					hr = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), 256, 512, TEX_FILTER_SEPARATE_ALPHA, InsertImage256512 );
					CreateTexture(m_pD3D, InsertImage256512.GetImages(), InsertImage256512.GetImageCount(), InsertImage256512.GetMetadata(), &pLoadedTexSurface256512 );
					hr = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), 512, 256, TEX_FILTER_SEPARATE_ALPHA, InsertImage512256 );
					CreateTexture(m_pD3D, InsertImage512256.GetImages(), InsertImage512256.GetImageCount(), InsertImage512256.GetMetadata(), &pLoadedTexSurface512256 );
				}
				else
				{
					// 1022x1022 at center
					hr = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), 1022, 1022, TEX_FILTER_SEPARATE_ALPHA, InsertImage512 );
					CreateTexture(m_pD3D, InsertImage512.GetImages(), InsertImage512.GetImageCount(), InsertImage512.GetMetadata(), &pLoadedTexSurface512 );
				}

				// texture plate always compressed
				hr = Decompress( imageTexturePlate.GetImages(), imageTexturePlate.GetImageCount(), imageTexturePlate.GetMetadata(),
					DXGI_FORMAT_B8G8R8A8_UNORM, convertedTexturePlate );
				if ( convertedTexturePlate.GetImageCount() == 0 )
				{
					CreateTexture(m_pD3D, imageTexturePlate.GetImages(), imageTexturePlate.GetImageCount(), platedata, &pPlateSurface );
				}
				else
				{
					platedata.format = convertedTexturePlate.GetImages()->format;
					CreateTexture(m_pD3D, convertedTexturePlate.GetImages(), convertedTexturePlate.GetImageCount(), platedata, &pPlateSurface );
				}
			}
		}

		// paste loaded texture into plate (60 in center of atlas texture area)
		if ( pLoadedTexSurface1024 && pPlateSurface ) 
		{
			// work out exact offset to slot position
			int iRow = iWhichTextureOver / 4;
			int iCol = iWhichTextureOver - (iRow*4);
			int iTexSlotOffsetX = iCol * 1024;
			int iTexSlotOffsetY = iRow * 1024;
			RECT rcPlate = RECT();

			// paste to fill 1024x1024 initially (to get at corners)
			rcPlate.left = iTexSlotOffsetX; rcPlate.top = iTexSlotOffsetY; rcPlate.right = iTexSlotOffsetX+1024; rcPlate.bottom = iTexSlotOffsetY+1024;
			m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface1024, 0, NULL );
			if ( iSeamlessMode == 0 )
			{
				// paste squashed 256x512 borders to help seamlessness
				// left
				rcPlate.left = iTexSlotOffsetX+0; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+256; rcPlate.bottom = iTexSlotOffsetY+256+512;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface256512, 0, NULL );
				// right
				rcPlate.left = iTexSlotOffsetX+256+512; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+1024; rcPlate.bottom = iTexSlotOffsetY+256+512;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface256512, 0, NULL );
				// top
				rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+0; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+256;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512256, 0, NULL );
				// bottom
				rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+256+512; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+1024;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512256, 0, NULL );
				// paste insert so smaller 512x512 terrain texture atlas can be seamless
				rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+256+512;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
			}
			else
			{
				// paste 1022x1022 on borders for seamlessness
				int iX = 1, iY = 0;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
				iX = 1, iY = 2;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
				iX = 0, iY = 1;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
				iX = 2, iY = 1;
				rcPlate.left = iTexSlotOffsetX+iX; rcPlate.top = iTexSlotOffsetY+iY; rcPlate.right = iTexSlotOffsetX+iX+1022; rcPlate.bottom = iTexSlotOffsetY+iY+1022;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
				// then paste into surface at 1022x1022 (so can have seamless textures within atlas)
				rcPlate.left = iTexSlotOffsetX+1; rcPlate.top = iTexSlotOffsetY+1;
				m_pImmediateContext->CopySubresourceRegion ( pPlateSurface, 0, (UINT)rcPlate.left, (UINT)rcPlate.top, 0, pLoadedTexSurface512, 0, NULL );
			}

			// replace imageTexturePlate with contents of pPlateSurface
			hr = CaptureTexture( m_pD3D, m_pImmediateContext, pPlateSurface, imageTexturePlate );
			if ( SUCCEEDED(hr) )
			{
				// first create a full mipmap set of images (as only first mipmap layer was affected above)
				ScratchImage mipChain;
				hr = GenerateMipMaps( imageTexturePlate.GetImages(), imageTexturePlate.GetImageCount(), imageTexturePlate.GetMetadata(), TEX_FILTER_SEPARATE_ALPHA, 0, mipChain );

				if ( iCompressIt == 0 )
				{
					// save new UNCOMPRESSED texture surface out
					const Image* img = mipChain.GetImages();
					hr = SaveToDDSFile( img, mipChain.GetImageCount(), mipChain.GetMetadata(), DDS_FLAGS_NONE, wFilenamePlate );
				}
				else
				{
					// compress to a DXT5 (BC3) texture
					//hr = Compress( imageTexturePlate.GetImages(), imageTexturePlate.GetImageCount(), imageTexturePlate.GetMetadata(), 
					//	DXGI_FORMAT_BC3_UNORM, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, convertedTexturePlate );
					hr = Compress( mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), 
						DXGI_FORMAT_BC3_UNORM, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, convertedTexturePlate );

					// save new texture surface out
					const Image* img = convertedTexturePlate.GetImages();
					hr = SaveToDDSFile( img, convertedTexturePlate.GetImageCount(), convertedTexturePlate.GetMetadata(), DDS_FLAGS_NONE, wFilenamePlate );
				}
			}
		}

		// free temp surface captures
		SAFE_RELEASE(pPlateSurface);
		SAFE_RELEASE(pLoadedTexSurface1024);
		SAFE_RELEASE(pLoadedTexSurface512);
		SAFE_RELEASE(pLoadedTexSurface256512);
		SAFE_RELEASE(pLoadedTexSurface512256);
	}
	#else
	// preferred format
	D3DSURFACE_DESC backbufferdesc;
	g_pGlob->pHoldBackBufferPtr->GetDesc ( &backbufferdesc );
	D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;//backbufferdesc.Format;
	//D3DFORMAT compressedFormat = D3DFMT_DXT1;

	// check if texture to load exists
	GGIMAGE_INFO finfo;
	LPDIRECT3DSURFACE9 pLoadedTexSurface = NULL;
	HRESULT hRes = D3DXGetImageInfoFromFile( pTexFileToLoad, &finfo );

	// create and load the texture selected
	if ( hRes == S_OK )
	{
		// load texture to be inserted
		hRes = m_pD3D->CreateRenderTarget( finfo.Width, finfo.Height, d3dFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &pLoadedTexSurface, NULL);
		hRes = D3DXLoadSurfaceFromFile( pLoadedTexSurface, NULL, NULL, pTexFileToLoad, NULL, D3DX_FILTER_POINT, 0, &finfo );

		// create/load the destination texture plate surface
		LPDIRECT3DTEXTURE9 pTextureDDS;
		LPDIRECT3DSURFACE9 pPlateSurface = NULL;
		cstr pPlateFilename = cstr(pDestTerrainTextureFile);
		hRes = D3DXGetImageInfoFromFile( pPlateFilename.Get(), &finfo );
		if ( hRes != S_OK )
		{
			// if plate file not exist, provide dimensions
			finfo.Width = 4096;
			finfo.Height = 4096;
		}
		m_pD3D->CreateTexture ( finfo.Width, finfo.Height, 1, 0, d3dFormat, D3DPOOL_MANAGED, &pTextureDDS, NULL );
		if ( pTextureDDS )
		{
			pTextureDDS->GetSurfaceLevel ( 0, &pPlateSurface );
			if ( pPlateSurface )
			{
				hRes = D3DXLoadSurfaceFromFile( pPlateSurface, NULL, NULL, pPlateFilename.Get(), NULL, D3DX_FILTER_POINT, 0, &finfo );
			}
		}

		// paste loaded texture into plate (60 in center of atlas texture area)
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

			// paste squashed 256x512 borders to help seamlessness
			// left
			rcPlate.left = iTexSlotOffsetX+0; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+256; rcPlate.bottom = iTexSlotOffsetY+256+512;
			hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
			// right
			rcPlate.left = iTexSlotOffsetX+256+512; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+1024; rcPlate.bottom = iTexSlotOffsetY+256+512;
			hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
			// top
			rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+0; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+256;
			hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
			// bottom
			rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+256+512; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+1024;
			hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);

			// paste insert so smaller 512x512 terrain texture atlas can be seamless
			rcPlate.left = iTexSlotOffsetX+256; rcPlate.top = iTexSlotOffsetY+256; rcPlate.right = iTexSlotOffsetX+256+512; rcPlate.bottom = iTexSlotOffsetY+256+512;
			hRes = D3DXLoadSurfaceFromSurface(pPlateSurface, NULL, &rcPlate, pLoadedTexSurface, NULL, NULL, D3DX_DEFAULT, 0);
	
			// and finally copy back to LoadTexture
			SAFE_RELEASE ( pLoadedTexSurface );
			hRes = m_pD3D->CreateRenderTarget( finfo.Width, finfo.Height, d3dFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &pLoadedTexSurface, NULL);
			hRes = D3DXLoadSurfaceFromSurface(pLoadedTexSurface, NULL, NULL, pPlateSurface, NULL, NULL, D3DX_DEFAULT, 0);

			// now create the compressed surface for the save
			SAFE_RELEASE ( pPlateSurface );
			SAFE_RELEASE ( pTextureDDS );
			D3DFORMAT d3dChoice = d3dFormat; // no compression, messes up mipmap and filtering
			if ( stricmp ( (pDestTerrainTextureFile + strlen(pDestTerrainTextureFile)) - 6, "_D.dds" ) == NULL ) d3dChoice = D3DFMT_DXT5;
			//m_pD3D->CreateTexture ( finfo.Width, finfo.Height, 0, D3DUSAGE_AUTOGENMIPMAP, d3dChoice, D3DPOOL_MANAGED, &pTextureDDS, NULL );
			m_pD3D->CreateTexture ( finfo.Width, finfo.Height, 1, 0, d3dChoice, D3DPOOL_MANAGED, &pTextureDDS, NULL );
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
			hRes = D3DXSaveSurfaceToFile( pPlateFilename.Get(), DestFormat, pPlateSurface, NULL, NULL );
			if ( FAILED ( hRes ) )
			{
				char pStrClue[512];
				wsprintf ( pStrClue, "Failed to save new custom texture plate: %s", pDestTerrainTextureFile );
				RunTimeError(RUNTIMEERROR_IMAGEERROR,pStrClue);
				SAFE_RELEASE(pLoadedTexSurface);
				SAFE_RELEASE(pPlateSurface);
				return 0;
			}

			// and the PNG for debugging
			if ( 0 )
			{
				DestFormat = D3DXIFF_PNG;
				cstr pDebugFilePNG = cstr(Left(pPlateFilename.Get(),strlen(pPlateFilename.Get())-4))+cstr(".png");
				hRes = D3DXSaveSurfaceToFile( pDebugFilePNG.Get(), DestFormat, pPlateSurface, NULL, NULL );
			}
		}

		// free temp surface captures
		SAFE_RELEASE(pPlateSurface);
		SAFE_RELEASE(pTextureDDS);
		SAFE_RELEASE(pLoadedTexSurface);
	}
	#endif

	// success
	return 1;
}

void terrain_loadlatesttexture ( void )
{
	// determine location of terrain texture
	char pLocationOfTerrainTexture[512];
	strcpy ( pLocationOfTerrainTexture, cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() );
	if ( g.terrainstyleindex == 1 ) strcpy ( pLocationOfTerrainTexture, g.mysystem.levelBankTestMap_s.Get() ); //"levelbank\\testmap" );

	// load the terrain texture into the terrain object
	SetImageAutoMipMap ( 1 );
	if ( g.gdividetexturesize == 0 ) 
	{
		t.tthistexdir_s="effectbank\\reloaded\\media\\white_D.dds";
		LoadImage ( t.tthistexdir_s.Get(),t.terrain.imagestartindex+13,0,g.gdividetexturesize );
	}
	else
	{
		// new terrain texture technique
		LoadImage ( cstr(cstr(pLocationOfTerrainTexture)+"\\Texture_D.dds").Get(),t.terrain.imagestartindex+13,0,g.gdividetexturesize );
	}
	LoadImage ( cstr(cstr(pLocationOfTerrainTexture)+"\\Texture_N.dds").Get(),t.terrain.imagestartindex+21,0,g.gdividetexturesize );
	TextureObject ( t.terrain.terrainobjectindex,2,t.terrain.imagestartindex+13 );
	// stage 3 : rem circle texture for highlighter
	TextureObject ( t.terrain.terrainobjectindex,4,t.terrain.imagestartindex+21 );
	SetImageAutoMipMap ( 0 );
}

void terrain_changestyle ( void )
{
	// replace terrain textures with those specified by terrainstyleindex
	SetImageAutoMipMap ( 1 );
	g.terrainstyle_s=t.terrainstylebank_s[g.terrainstyleindex];
	if ( ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		// if terrain folder exists
		if ( PathExist( cstr(cstr("terrainbank\\")+g.terrainstyle_s).Get() ) == 1 || g.terrainstyleindex == 1) 
		{
			// check if new terrain texture system file available, and create if not
			if ( g.terrainstyleindex > 1 ) 
			{
				char pNewTerrainTextureFile[512];
				strcpy ( pNewTerrainTextureFile, cstr(cstr("terrainbank\\")+g.terrainstyle_s+"\\Texture_D.dds").Get() );
				if ( FileExist ( pNewTerrainTextureFile ) == 0 )
				{
					// create diffuse and normal textures that combine old textures into one atlas of 4x4 textures (4096x4096)
					LPSTR pOldDir = GetDir();
					SetDir ( cstr(cstr("terrainbank\\")+g.terrainstyle_s+"\\").Get() );
					strcpy ( pNewTerrainTextureFile, "Texture_D.dds" );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 0, "Path_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 1, "Path_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 2, "Path_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 3, "Default_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 4, "Default_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 5, "Default_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 6, "Sedimentary_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 7, "Sedimentary_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 8, "Sedimentary_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 9, "Sedimentary_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 10, "Mossy_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 11, "Mossy_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 12, "Mossy_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 13, "Rocky_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 14, "Rocky_D.dds", 0, 1 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 15, "Rocky_D.dds", 0, 1 );
				
					// and now the normals file
					strcpy ( pNewTerrainTextureFile, "Texture_N.dds" );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 0, "Path_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 1, "Path_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 2, "Path_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 3, "Default_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 4, "Default_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 5, "Default_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 6, "Sedimentary_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 7, "Sedimentary_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 8, "Sedimentary_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 9, "Sedimentary_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 10, "Mossy_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 11, "Mossy_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 12, "Mossy_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 13, "Rocky_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 14, "Rocky_N.dds", 0, 0 );
					terrain_createnewterraintexture ( pNewTerrainTextureFile, 15, "Rocky_N.dds", 0, 0 );

					// restore directory after terrain texture file creation
					SetDir ( pOldDir );
				}
			}

			// load terrain textures
			terrain_loadlatesttexture ( );

			// generate super texture from above existing texture
			terrain_generatesupertexture ( false );
		}
	}
	SetImageAutoMipMap (  0 );
}

void terrain_changevegstyle ( void )
{
	//  replace vegetaion mesh and texture
	g.vegstyle_s=t.vegstylebank_s[g.vegstyleindex];
	if (  PathExist( cstr(cstr("vegbank\\")+g.vegstyle_s).Get()) == 1 ) 
	{
		terrain_fastveg_setgrassimage ( );
	}
}

void terrain_getpaintmode ( void )
{
	if (  t.conkit.entityeditmode == 1 ) 
	{
		if ( t.gridentitymarkersmodeonly == 0 )
			t.mode_s="Entity Mode";
		else
			t.mode_s="Marker Mode";
	}
	else
	{
		if (  t.terrain.terrainpaintermode >= 1 && t.terrain.terrainpaintermode <= 5 ) 
		{
			t.mode_s="Sculpt Terrain - ";
			if (  t.terrain.terrainpaintermode == 1  )  t.mode_s = t.mode_s+"Shape";
			if (  t.terrain.terrainpaintermode == 2  )  t.mode_s = t.mode_s+"Level";
			if (  t.terrain.terrainpaintermode == 3  )  t.mode_s = t.mode_s+"Stored Level";
			if (  t.terrain.terrainpaintermode == 4  )  t.mode_s = t.mode_s+"Blend";
			if (  t.terrain.terrainpaintermode == 5  )  t.mode_s = t.mode_s+"Ramp";
		}
		else
		{
			t.mode_s="Paint Terrain - ";
			if (  t.terrain.terrainpaintermode == 6  )  t.mode_s = t.mode_s+"Texture";
			if (  t.terrain.terrainpaintermode == 7  )  t.mode_s = t.mode_s+"Texture";
			if (  t.terrain.terrainpaintermode == 8  )  t.mode_s = t.mode_s+"Texture";
			if (  t.terrain.terrainpaintermode == 9  )  t.mode_s = t.mode_s+"Texture";
			if (  t.terrain.terrainpaintermode == 10  )  t.mode_s = t.mode_s+"Grass";
		}
	}
}

void terrain_loop ( void )
{
	//  F9 Editing Mode can change terrain heights, so need to
	//  adjust AI obstacle map to compensate
	if (  t.terrain.triggerobstaclerefresh>0 ) 
	{
		--t.terrain.triggerobstaclerefresh;
		if (  t.terrain.triggerobstaclerefresh == 0 ) 
		{
			t.terrain.terrainregionupdate=0;
			terrain_refreshterrainmatrix ( );
			darkai_obstacles_terrain_refresh ( );
		}
	}
}

void terrain_terraintexturesystempainterentry ( void )
{
	// track if terrain paint mode used or not (so can switch panel off when done)
	terrainbuild.bUsingTerrainTextureSystemPaintSelector = false;
}

void terrain_detectendofterraintexturesystempainter ( void )
{
	// if no terrain texture system painter in use, hide panel
	if ( terrainbuild.bUsingTerrainTextureSystemPaintSelector == false ) 
		terrain_paintselector_hide();
}

void terrain_editcontrol ( void )
{
	// terrain edit speed
	if (  t.inputsys.keycontrol == 1 ) 
		t.terrain.ts_f=t.ts_f/5.0;
	else
		t.terrain.ts_f=t.ts_f/2.5;

	// locate terrain coordinate
	t.tupdateterraincursor=0;
	if ( t.terrain.terrainpaintermode == 1 ) 
	{
		//  aise/lower should not shift cursor position as land changes
		if ( t.inputsys.mclick == 0 ) 
		{
			t.tupdateterraincursor=1;
		}
		else
		{
			if ( t.terrain.lastxmouse == t.inputsys.xmouse && t.terrain.lastymouse == t.inputsys.ymouse ) 
			{
				// do not move terrain cursor to create single hill/dip
			}
			else
			{
				t.tupdateterraincursor=1;
			}
		}
	}
	else
	{
		//  other modes can move cursor in all states
		t.tupdateterraincursor=1;
	}
	t.terrain.lastxmouse=t.inputsys.xmouse;
	t.terrain.lastymouse=t.inputsys.ymouse;
	if (  t.tupdateterraincursor == 1 ) 
	{
		t.terrain.X_f=t.inputsys.localx_f;
		t.terrain.Y_f=t.inputsys.localy_f;
	}

	//  Height at camera coordinate
	if ( t.terrain.TerrainID>0 ) 
		t.terrain.camheightatcoord_f=BT_GetGroundHeight(t.terrain.TerrainID,t.terrain.X_f,t.terrain.Y_f);
	else
		t.terrain.camheightatcoord_f=1000.0;

	// Only control keys while not editing entities
	if ( t.conkit.entityeditmode == 0 ) 
	{
		// Control painter objects
		if ( t.inputsys.k_s == "1"  )  t.terrain.terrainpaintermode = 1;
		if ( t.inputsys.k_s == "2"  )  t.terrain.terrainpaintermode = 2;
		if ( t.inputsys.k_s == "3"  )  t.terrain.terrainpaintermode = 3;
		if ( t.inputsys.k_s == "4"  )  t.terrain.terrainpaintermode = 4;
		if ( t.inputsys.k_s == "5"  )  t.terrain.terrainpaintermode = 5;
		if ( t.inputsys.k_s == "6"  )  t.terrain.terrainpaintermode = 6;
		if ( t.inputsys.k_s == "7"  )  t.terrain.terrainpaintermode = 7;
		if ( t.inputsys.k_s == "8"  )  t.terrain.terrainpaintermode = 8;
		if ( t.inputsys.k_s == "9"  )  t.terrain.terrainpaintermode = 9;
		if ( t.inputsys.k_s == "0"  )  t.terrain.terrainpaintermode = 10;
		t.tmin = 50; // 220216 - new standard size for both modes
		if ( t.inputsys.k_s == "-" && t.terrain.RADIUS_f>t.tmin  )  t.terrain.RADIUS_f = t.terrain.RADIUS_f-(25*t.terrain.ts_f);
		if ( t.inputsys.k_s == "=" && t.terrain.RADIUS_f<g.fTerrainBrushSizeMax  )  t.terrain.RADIUS_f = t.terrain.RADIUS_f+(25*t.terrain.ts_f);

		// move any entities lifted/dropped due to terrain sculpting (see code below)
		if ( t.terrain.TerrainID > 0 ) 
		{
			if ( t.terrain.terrainpaintermode >= 1 && t.terrain.terrainpaintermode <= 5 ) 
			{
				// this can also undo all InstanceStamp ( constructs )
				t.trevealallinstancestampentities=0;

				// raise any entities subject to this terrain radius
				for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
				{
					t.entid=t.entityelement[t.e].bankindex;
					if (  t.entityelement[t.e].floorposy > 0 && t.entityelement[t.e].editorlock == false ) 
					{
						t.obj=t.entityelement[t.e].obj;
						if (  g.gridlayershowsingle == 1 ) 
						{
							//  do not select if TAB slice mode active and entity too big (buildings, walls, etc)
							if (  t.obj>0 ) 
							{
								if (  ObjectSizeX(t.obj)>95 && ObjectSizeY(t.obj)>95 && ObjectSizeZ(t.obj)>95 ) 
								{
									t.obj=0;
								}
							}
						}
						if (  t.obj>0 ) 
						{
							if (  ObjectExist(t.obj) == 1 ) 
							{
								t.tadjy_f=BT_GetGroundHeight(t.terrain.TerrainID,t.entityelement[t.e].x,t.entityelement[t.e].z)-t.entityelement[t.e].floorposy;
								if (  t.tadjy_f != 0 ) 
								{
									t.entityelement[t.e].y=t.entityelement[t.e].y+t.tadjy_f;
									if (  t.conkit.editmodeactive == 1 ) 
									{
										//  when in FPS 3D Edit Mode - when physics is ACTIVE - must reset entity in new position
										t.tphyobj=t.entityelement[t.e].obj;
										physics_disableobject ( );
										PositionObject (  t.tphyobj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
										RotateObject (  t.tphyobj,t.entityelement[t.e].rx,t.entityelement[t.e].ry,t.entityelement[t.e].rz );
										physics_prepareentityforphysics ( );
										//  also record this change PERMINANTLY for return to editor
										if (  ArrayCount(t.storedentityelement)>0 ) 
										{
											t.storedentityelement[t.e].y=t.storedentityelement[t.e].y+t.tadjy_f;
										}
									}
									else
									{
										PositionObject (  t.obj,t.entityelement[t.e].x,t.entityelement[t.e].y,t.entityelement[t.e].z );
									}
								}
							}
						}
						t.entityelement[t.e].floorposy=0;
					}
				}
				if ( t.trevealallinstancestampentities == 1 ) 
				{
					// we have edited the entities in the game itself, so remove
					// InstanceStamp (  system for manual entity edit mode )
					for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
					{
						t.entid=t.entityelement[t.e].bankindex;
						t.obj=t.entityelement[t.e].obj;
						if (  t.obj>0 ) 
						{
							if (  ObjectExist(t.obj) == 1 ) 
							{
								if (  t.entityprofile[t.entid].ismarker == 0 ) 
								{
									ShowObject (  t.obj );
								}
							}
						}
					}
				}
			}
		}

		// Any click means we modified the project
		t.mc = t.inputsys.mclick ; if (  t.mc == 2 || t.mc == 4  )  t.mc = 0;
		if ( t.mc!=0 )
		{
			g.projectmodified=1;
			gridedit_changemodifiedflag ( ); 
			// not affected g.projectmodifiedstatic
		}

		// record before area modified
		if ( t.mc != 0 ) 
		{ 
			// 301115 - wipe but can create new entity action below (for restoring entities displayed by terrain)
			if ( t.terrain.lastmc == 0 ) t.entityundo.action=0;
			terrain_recordbuffer ( );
		}

		// paint heights
		if ( t.terrain.TerrainID>0 ) 
		{
			if ( t.terrain.terrainpaintermode >= 1 && t.terrain.terrainpaintermode <= 5 ) 
			{
				//  Sculpt
				t.tmaketerraindirty=0;
				if (  t.mc != 0 ) 
				{
					if (  t.terrain.lastmc == 0  )  t.mconce = 1; else t.mconce = 0;
					if (  t.mc == 1 && t.inputsys.keyshift == 0 ) 
					{
						t.terrain.AMOUNT_f=50*t.terrain.zoom_f;
					}
					if (  t.mc == 1 && t.inputsys.keyshift == 1 ) 
					{
						t.terrain.AMOUNT_f=-50*t.terrain.zoom_f;
					}
					if (  t.terrain.X_f>1 && t.terrain.X_f<(1024*50.0)-1 ) 
					{
						if (  t.terrain.Y_f>1 && t.terrain.Y_f<(1024*50.0)-1 ) 
						{
							//  record old entity Y positions relative to terrain Floor (  )
							if ( t.mconce == 1 )
							{
								g.entityrubberbandlistundo.clear();
								for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
								{
									t.obj=t.entityelement[t.e].obj;
									if (  t.obj>0 ) 
									{
										t.ttdx_f=t.entityelement[t.e].x-t.terrain.X_f;
										t.ttdz_f=t.entityelement[t.e].z-t.terrain.Y_f;
										t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdz_f*t.ttdz_f));
										if (  t.ttdd_f <= t.terrain.RADIUS_f ) 
										{
											t.tentitytoselect = t.e;
											entity_recordbuffer_move();
											g.entityrubberbandlistundo.push_back ( t.entityundo );
										}
									}
								}
								if ( g.entityrubberbandlistundo.size() > 0 )
								{
									// special code to point this undo event to the rubberbandlist undo buffer
									t.entityundo.entityindex = -123;
									t.entityundo.bankindex = -123;
								}
							}
							for ( t.e = 1 ; t.e<=  g.entityelementlist; t.e++ )
							{
								t.obj=t.entityelement[t.e].obj;
								if (  t.obj>0 ) 
								{
									t.ttdx_f=t.entityelement[t.e].x-t.terrain.X_f;
									t.ttdz_f=t.entityelement[t.e].z-t.terrain.Y_f;
									t.ttdd_f=Sqrt(abs(t.ttdx_f*t.ttdx_f)+abs(t.ttdz_f*t.ttdz_f));
									if (  t.ttdd_f <= t.terrain.RADIUS_f ) 
									{
										t.entityelement[t.e].floorposy=BT_GetGroundHeight(t.terrain.TerrainID,t.entityelement[t.e].x,t.entityelement[t.e].z);
									}
								}
							}
							//  modify height map
							if (  t.terrain.terrainpaintermode == 1 ) 
							{
								BT_RaiseTerrain (  t.terrain.TerrainID,t.terrain.X_f,t.terrain.Y_f,t.terrain.RADIUS_f,t.terrain.AMOUNT_f*t.terrain.ts_f );
								t.tmaketerraindirty=1;
							}
							if (  t.terrain.terrainpaintermode == 2 ) 
							{
								if (  t.mconce == 1 ) 
								{
									t.terrain.terrainlevel_f=BT_GetGroundHeight(t.terrain.TerrainID,t.terrain.X_f,t.terrain.Y_f);
								}
								t.terrain.terrainlevelt_f=t.terrain.terrainlevel_f;
								if (  t.mc == 1 && t.inputsys.keyshift == 1 ) 
								{
									t.terrain.AMOUNT_f=50*t.terrain.zoom_f;
									t.terrain.terrainlevelt_f=t.terrain.terrainlevelt_f+Rnd(50)-25;
								}
								BT_RaiseTerrain (  t.terrain.TerrainID,t.terrain.X_f,t.terrain.Y_f,t.terrain.RADIUS_f,t.terrain.AMOUNT_f*t.terrain.ts_f,t.terrain.terrainlevelt_f );
								t.tmaketerraindirty=1;
							}
							if (  t.terrain.terrainpaintermode == 3 ) 
							{
								t.terrain.AMOUNT_f=50*t.terrain.zoom_f;
								BT_RaiseTerrain (  t.terrain.TerrainID,t.terrain.X_f,t.terrain.Y_f,t.terrain.RADIUS_f,t.terrain.AMOUNT_f*t.terrain.ts_f,t.terrain.terrainlevel_f );
								t.tmaketerraindirty=1;
							}
							if (  t.terrain.terrainpaintermode == 4 ) 
							{
								//  blend terrain heights
								t.trx=t.terrain.X_f/50.0;
								t.ttry=t.terrain.Y_f/50.0;
								t.trrad=t.terrain.RADIUS_f/50.0;
								//int newSize = (((t.trrad+1)*2)*((t.trrad+1)*2));
								//Dim (  t.storeheights_f, newSize );
								t.tstartx=t.trx-t.trrad ; t.tfinishx=t.trx+t.trrad;
								//t.tstarty=t.t-t.trrad ; t.tfinishy=t.ttry+t.trrad;
								t.tstarty=t.ttry-t.trrad ; t.tfinishy=t.ttry+t.trrad; // 310317 - typo creates crash!
								if ( t.trrad < 4 ) t.trrad = 4;
								int newSize = (t.tfinishx-t.tstartx)+((t.tfinishy-t.tstarty)*(t.trrad*2));
								Dim (  t.storeheights_f, newSize );

								for ( t.trady = t.tstarty ; t.trady<=  t.tfinishy; t.trady++ )
								{
									for ( t.tradx = t.tstartx ; t.tradx<=  t.tfinishx; t.tradx++ )
									{
										t.tindex=(t.tradx-t.tstartx)+(((t.trady-t.tstarty)*t.trrad*2));
										t.storeheights_f[t.tindex]=BT_GetGroundHeight(t.terrain.TerrainID,t.tradx*50,t.trady*50,1);
									}
								}
								for ( t.trady = t.tstarty+1 ; t.trady<=  t.tfinishy-1; t.trady++ )
								{
									for ( t.tradx = t.tstartx+1 ; t.tradx<=  t.tfinishx-1; t.tradx++ )
									{
										t.tdistx=t.tradx-t.trx ; t.tdisty=t.trady-t.ttry;
										t.tdist=Sqrt(abs(t.tdistx*t.tdistx)+abs(t.tdisty*t.tdisty));
										if (  t.tdist <= t.trrad ) 
										{
											t.tweightrad=t.trrad-1 ; t.tweight_f=((t.tweightrad-t.tdist)+0.0)/(t.tweightrad+0.0);
											if (  t.tweight_f<0  )  t.tweight_f = 0.0;
											if (  t.tweight_f>1.0  )  t.tweight_f = 1.0;
											t.tcurrentheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.tradx*50,t.trady*50,1);
											t.tindex=(t.tradx-t.tstartx)+(((t.trady-t.tstarty)*t.trrad*2));
											t.tcount=0 ; t.theight_f=0;
											for ( t.ttty = t.trady-1 ; t.ttty<=  t.trady+1; t.ttty++ )
											{
												for ( t.tttx = t.tradx-1 ; t.tttx<=  t.tradx+1; t.tttx++ )
												{
													if (  t.ttty >= 0 && t.ttty <= 1023 ) 
													{
														if (  t.tttx >= 0 && t.tttx <= 1023 ) 
														{
															t.ttindex=(t.tttx-t.tstartx)+(((t.ttty-t.tstarty)*t.trrad*2));
															if (  t.ttindex >= 0 && t.ttindex <= ArrayCount(t.storeheights_f) ) 
															{
																t.theight_f=t.theight_f+t.storeheights_f[t.ttindex];
																++t.tcount;
															}
														}
													}
												}
											}
											t.theight_f=t.theight_f/t.tcount;
											t.tblendfactor_f=(0.2*t.terrain.ts_f)*t.tweight_f;
											t.theight_f=(t.theight_f*t.tblendfactor_f)+(t.tcurrentheight_f*(1.0-t.tblendfactor_f));
											BT_SetPointHeight (  t.terrain.TerrainID,t.tradx,t.trady,t.theight_f );
										}
									}
								}
								t.tmaketerraindirty=1;
							}
							if (  t.terrain.terrainpaintermode == 5 ) 
							{
								//  form ramp
								if (  t.terrain.rubberbandrampflag == 0 ) 
								{
									t.terrain.rubberbandrampflag=1;
									t.terrain.rubberbandrampx=t.terrain.X_f;
									t.terrain.rubberbandrampy=t.terrain.Y_f;
									t.terrain.rubberbandrampradius=t.terrain.RADIUS_f;
									//  finished later when MC is released
								}
							}
						}
					}
					//  flag that we have changed the terrain height (for F9 mode to generate new water boundaries or not)
					t.terrain.heightsmodified=1;
					//  also flag to delete any grass within area of modification
					if (  t.conkit.editmodeactive == 1 ) 
					{
						if (  t.terrain.terrainregionupdate == 1 ) 
						{
							t.terrain.terrainregionupdate=0;
							t.terrain.grassregionupdate=0;
							terrain_fastveg_clearregion ( );
						}
					}
				}
				else
				{
					//  released left mouse button
					if (  t.terrain.terrainquickupdate == 1 ) 
					{
						if (  t.game.set.shaderrequirecheapshadow == 1 ) 
						{
							//  and update heightmap texture
							terrain_quickupdateheightmapfromheightdata ( );
						}
					}
					if (  t.terrain.rubberbandrampflag == 1 ) 
					{
						//  only if mouse not LEFT the edit area
						t.terrain.rubberbandrampflag=0;
						if (  t.inputsys.xmouse != 500000 ) 
						{
							//  create ramp from last recorded position to here
							t.tfromx_f=t.terrain.rubberbandrampx/50.0;
							t.tfromy_f=t.terrain.rubberbandrampy/50.0;
							t.ttox_f=t.terrain.X_f/50.0;
							t.ttoy_f=t.terrain.Y_f/50.0;
							t.tdiffx_f=t.tfromx_f-t.ttox_f;
							t.tdiffy_f=t.tfromy_f-t.ttoy_f;
							t.tangle_f=atan2deg(t.tdiffx_f,t.tdiffy_f);
							t.tdistance_f=Sqrt(abs(t.tdiffx_f*t.tdiffx_f)+abs(t.tdiffy_f*t.tdiffy_f))*50.0;
							t.trx1=t.tfromx_f;
							t.try1=t.tfromy_f;
							t.trrad1=t.terrain.rubberbandrampradius/50.0;
							t.theight1_f=BT_GetGroundHeight(t.terrain.TerrainID,t.trx1*50,t.try1*50,1);
							t.trx2=t.ttox_f;
							t.try2=t.ttoy_f;
							t.trrad2=t.terrain.RADIUS_f/50.0;
							t.theight2_f=BT_GetGroundHeight(t.terrain.TerrainID,t.trx2*50,t.try2*50,1);
							t.theightstep_f=(t.theight2_f-t.theight1_f);
							t.tslices_f=t.tdistance_f/25.0;
							t.tstepbit_f=t.theightstep_f/t.tslices_f;
							t.trbitx_f=(t.trx2-t.trx1+0.0)/t.tslices_f;
							t.trbity_f=(t.try2-t.try1+0.0)/t.tslices_f;
							for ( t.t = 0 ; t.t<=  (t.tslices_f-1); t.t++ )
							{
								t.trx3_f=(t.trx1+0.0)+(t.trbitx_f*t.t);
								t.try3_f=(t.try1+0.0)+(t.trbity_f*t.t);
								// Ensure a min size, so even the smallest radius does something
								t.ttrrad1 = t.trrad1-2 ; if (  t.ttrrad1<5  )  t.ttrrad1 = 5;
								t.ttrrad2 = t.trrad2-2 ; if (  t.ttrrad2<5  )  t.ttrrad2 = 5;
								t.ttr=t.ttrrad1+(((t.ttrrad2-t.ttrrad1+0.0)/t.tslices_f)*t.t);
								t.tedgettr=t.ttr;
								for ( t.tt = t.tedgettr*-1 ; t.tt<=  t.tedgettr; t.tt++ )
								{
									t.trxx=NewXValue(t.trx3_f,t.tangle_f+90,t.tt) ;
									t.tryy=NewZValue(t.try3_f, t.tangle_f+90,t.tt);
									t.tdestheight_f=(t.tstepbit_f*t.t);
									if (  t.tt >= t.tedgettr-2 || t.tt <= (t.tedgettr*-1)+2 ) 
									{
										//  set the ramp edge
										if (  t.theight1_f>t.theight2_f ) 
										{
											t.tdiff_f=(t.theight1_f-t.theight2_f)+t.tdestheight_f;
											t.tlowesth_f=t.theight2_f;
										}
										else
										{
											t.tdiff_f=(t.theight2_f-t.theight1_f)-t.tdestheight_f;
											t.tlowesth_f=t.theight1_f;
										}
										/* 061115 - removed feathering for now (low to high corrupts)
										if (  t.tt == t.tedgettr || t.tt == (t.tedgettr*-1) ) 
										{
											t.tfeather_f=0.2;
										}
										else
										{
											if (  t.tt == t.tedgettr-1 || t.tt == (t.tedgettr*-1)+1 ) 
											{
												t.tfeather_f=0.5;
											}
											else
											{
												t.tfeather_f=0.9;
											}
										}
										*/
										t.tfinalheight_f=t.tlowesth_f; //+(t.tdiff_f*t.tfeather_f);
										if (  t.tfinalheight_f<BT_GetGroundHeight(t.terrain.TerrainID,t.trxx*50,t.tryy*50,1) ) 
										{
											//  skip height set if current Floor (  HIGHER than the new height trying to set )
										}
										else
										{
											BT_SetPointHeight (  t.terrain.TerrainID,t.trxx,t.tryy,t.tfinalheight_f );
										}
									}
									else
									{
										//  set the solid ramp height
										BT_SetPointHeight (  t.terrain.TerrainID,t.trxx,t.tryy,t.theight1_f+t.tdestheight_f );
									}
								}
							}
							t.tmaketerraindirty=1;
						}
					}
				}
			}
			if (  t.tmaketerraindirty == 1 ) 
			{
				// mark dirty region for water mask (and other things eventually)
				t.x=t.terrain.X_f/50.0 ; t.z=t.terrain.Y_f/50.0 ; t.r=1+(t.terrain.RADIUS_f/50.0);
				if ( t.terrain.terrainregionupdate == 0 ) 
				{
					t.terrain.terrainregionupdate=1;
					t.terrain.terrainregionx1=t.x;
					t.terrain.terrainregionx2=t.x;
					t.terrain.terrainregionz1=t.z;
					t.terrain.terrainregionz2=t.z;
				}
				if ( t.x-t.r<t.terrain.terrainregionx1  )  t.terrain.terrainregionx1 = t.x-t.r;
				if ( t.x+t.r>t.terrain.terrainregionx2  )  t.terrain.terrainregionx2 = t.x+t.r;
				if ( t.z-t.r<t.terrain.terrainregionz1  )  t.terrain.terrainregionz1 = t.z-t.r;
				if ( t.z+t.r>t.terrain.terrainregionz2  )  t.terrain.terrainregionz2 = t.z+t.r;
				if ( t.terrain.terrainregionx1<0  )  t.terrain.terrainregionx1 = 0;
				if ( t.terrain.terrainregionx2>1024  )  t.terrain.terrainregionx2 = 1024;
				if ( t.terrain.terrainregionz1<0  )  t.terrain.terrainregionz1 = 0;
				if ( t.terrain.terrainregionz2>1024  )  t.terrain.terrainregionz2 = 1024;
				// mark quick refresh region (used to update heightmap texture for cheap shadow)
				if ( t.terrain.terrainquickupdate == 0 ) 
				{
					t.terrain.terrainquickupdate=1;
					t.terrain.terrainquickx1=t.x;
					t.terrain.terrainquickx2=t.x;
					t.terrain.terrainquickz1=t.z;
					t.terrain.terrainquickz2=t.z;
				}
				if ( t.x-t.r<t.terrain.terrainquickx1  )  t.terrain.terrainquickx1 = t.x-t.r;
				if ( t.x+t.r>t.terrain.terrainquickx2  )  t.terrain.terrainquickx2 = t.x+t.r;
				if ( t.z-t.r<t.terrain.terrainquickz1  )  t.terrain.terrainquickz1 = t.z-t.r;
				if ( t.z+t.r>t.terrain.terrainquickz2  )  t.terrain.terrainquickz2 = t.z+t.r;
				if ( t.terrain.terrainquickx1<0  )  t.terrain.terrainquickx1 = 0;
				if ( t.terrain.terrainquickx2>1024  )  t.terrain.terrainquickx2 = 1024;
				if ( t.terrain.terrainquickz1<0  )  t.terrain.terrainquickz1 = 0;
				if ( t.terrain.terrainquickz2>1024  )  t.terrain.terrainquickz2 = 1024;
				//  set terrain to dirty
				t.terrain.dirtyterrain=1;
				t.terrain.dirtyx1=((t.terrain.X_f-t.terrain.RADIUS_f)/50.0)-1;
				t.terrain.dirtyx2=((t.terrain.X_f+t.terrain.RADIUS_f)/50.0)+1;
				t.terrain.dirtyz1=((t.terrain.Y_f-t.terrain.RADIUS_f)/50.0)-1;
				t.terrain.dirtyz2=((t.terrain.Y_f+t.terrain.RADIUS_f)/50.0)+1;
				if (  t.terrain.dirtyx1<0  )  t.terrain.dirtyx1 = 0;
				if (  t.terrain.dirtyz1<0  )  t.terrain.dirtyz1 = 0;
				if (  t.terrain.dirtyx2>1023  )  t.terrain.dirtyx2 = 1023;
				if (  t.terrain.dirtyz2>1023  )  t.terrain.dirtyz2 = 1023;
				// now we need to raise the grass to this new terrain height
				t.terrain.grassupdateafterterrain = 1;
				t.terrain.grassregionx1 = t.terrain.dirtyx1;
				t.terrain.grassregionx2 = t.terrain.dirtyx2;
				t.terrain.grassregionz1 = t.terrain.dirtyz1;
				t.terrain.grassregionz2 = t.terrain.dirtyz2;
			}
		}
		t.terrain.lastmc=t.mc;

		// Paint textures (6 through 10 was the old paint modes, used now to switch into paint system mode)
		if ( t.terrain.terrainpaintermode >= 6 && t.terrain.terrainpaintermode <= 10 ) 
		{
			// new terrain texture system paint selector
			terrainbuild.bUsingTerrainTextureSystemPaintSelector = true;
			terrain_paintselector_control ( );

			// detect if paint mode changes (reflect for now in currenttexture index)
			if ( t.terrain.terrainpaintermode != t.terrain.lastterrainpaintermode )
			{
				t.terrain.lastterrainpaintermode = t.terrain.terrainpaintermode;
				if ( t.terrain.terrainpaintermode == 6 ) terrainbuild.iCurrentTexture = 0;
				if ( t.terrain.terrainpaintermode == 7 ) terrainbuild.iCurrentTexture = 8;
				if ( t.terrain.terrainpaintermode == 8 ) terrainbuild.iCurrentTexture = 12;
				if ( t.terrain.terrainpaintermode == 9 ) terrainbuild.iCurrentTexture = 15;
				terrainbuild_settexturehighlight ( );
			}

			// handle painting of the terrain itself
			if ( t.mc == 1 && (t.terrain.X_f != t.terrain.lastpaintx_f || t.terrain.Y_f != t.terrain.lastpaintz_f) ) 
			{
				//  paint color
				t.terrain.lastpaintx_f=t.terrain.X_f;
				t.terrain.lastpaintz_f=t.terrain.Y_f;
				if ( t.terrain.terrainpaintermode != 10 ) 
				{
					if ( t.inputsys.keyshift == 1 )  
						t.texselect = Rgb(0,0,64);
					else
						t.texselect = Rgb(0,0,terrainbuild.iCurrentTexture*17);

					t.texselectgrass = 0;
					//if ( t.terrain.terrainpaintermode == 6 )  t.texselect  =  Rgb(0,0,0);
					//if ( t.terrain.terrainpaintermode == 7 )  t.texselect  =  Rgb(0,0,128);
					//if ( t.terrain.terrainpaintermode == 8 )  t.texselect  =  Rgb(0,0,192);
					//if ( t.terrain.terrainpaintermode == 9 )  t.texselect  =  Rgb(0,0,255);
				}
				if ( t.terrain.terrainpaintermode == 10 ) 
				{
					//  painting grass
					t.x=t.terrain.X_f/50.0 ; t.z=t.terrain.Y_f/50.0 ; t.r=1+(t.terrain.RADIUS_f/50.0);
					if (  t.terrain.grassregionupdate == 0 ) 
					{
						t.terrain.grassregionupdate=1;
						t.terrain.grassregionx1=t.x;
						t.terrain.grassregionx2=t.x;
						t.terrain.grassregionz1=t.z;
						t.terrain.grassregionz2=t.z;
					}
					if (  t.x-t.r<t.terrain.grassregionx1  )  t.terrain.grassregionx1 = t.x-t.r;
					if (  t.x+t.r>t.terrain.grassregionx2  )  t.terrain.grassregionx2 = t.x+t.r;
					if (  t.z-t.r<t.terrain.grassregionz1  )  t.terrain.grassregionz1 = t.z-t.r;
					if (  t.z+t.r>t.terrain.grassregionz2  )  t.terrain.grassregionz2 = t.z+t.r;
					if (  t.terrain.grassregionx1<0  )  t.terrain.grassregionx1 = 0;
					if (  t.terrain.grassregionx2>1024  )  t.terrain.grassregionx2 = 1024;
					if (  t.terrain.grassregionz1<0  )  t.terrain.grassregionz1 = 0;
					if (  t.terrain.grassregionz2>1024  )  t.terrain.grassregionz2 = 1024;

					// grass value stored in RED component
					t.texselectgrass = 1;
					if ( t.inputsys.keyshift == 1 )  
						t.texselect = Rgb(0,0,0);
					else
						t.texselect = Rgb(255,0,0);
				}

				// finally paint to veghshadow texture
				terrain_paintterrain ( );

				// invalidate super texture as terrain paint changed
				t.terrain.generatedsupertexture = 0;
			}
		}

		// allows second sculpt/paint to erase first undo buffer
		if ( t.mc == 0  )  t.terrainundo.mode = 0;
	}
}

void ConvertVegMemblock ( int grassmemblock )
{
	// bitmap image now stored in RGBA format, so need to convert so memblock keeps using old format
	t.tPindex = 4+4+4;
	for ( t.tP = 0 ; t.tP<=  MAXTEXTURESIZE*MAXTEXTURESIZE - 1; t.tP++ )
	{
		BYTE pNewRed = ReadMemblockByte ( grassmemblock, t.tPindex+0 );
		BYTE pNewBlue = ReadMemblockByte ( grassmemblock, t.tPindex+2 );
		WriteMemblockByte ( grassmemblock,t.tPindex+0,pNewBlue );
		WriteMemblockByte ( grassmemblock,t.tPindex+2,pNewRed );
		t.tPindex += 4;
	}
}

void terrain_recordbuffer ( void )
{
	if (  1 ) 
	{
		if (  t.terrainundo.mode == 0 ) 
		{
			t.terrainundo.tux1=(t.terrain.X_f/50.0) ; t.terrainundo.tux2=t.terrainundo.tux1;
			t.terrainundo.tuz1=(t.terrain.Y_f/50.0) ; t.terrainundo.tuz2=t.terrainundo.tuz1;
			for ( t.tuz = 0 ; t.tuz<=  1024; t.tuz++ )
			{
				for ( t.tux = 0 ; t.tux<=  1024; t.tux++ )
				{
					t.terrainundobuffer[t.tux][t.tuz]=-1;
				}
			}
			//  snapshot present terrain paint and store in 124
			if (  MemblockExist(124) == 0  ) CreateMemblockFromImage (  124,t.terrain.imagestartindex+2 );
			if (  MemblockExist(123) == 1  )  CopyMemblock (  123,124,0,0,GetMemblockSize(123) );
			t.terrainundo.mode=1;
		}
		else
		{
			t.turadius=t.terrain.RADIUS_f/50.0;
			t.tux1 = (t.terrain.X_f/50.0)-1-t.turadius ; if (  t.tux1<0  )  t.tux1 = 0;
			t.tux2 = (t.terrain.X_f/50.0)+1+t.turadius ; if (  t.tux2>1024  )  t.tux2 = 1024;
			t.tuz1 = (t.terrain.Y_f/50.0)-1-t.turadius ; if (  t.tuz1<0  )  t.tuz1 = 0;
			t.tuz2 = (t.terrain.Y_f/50.0)+1+t.turadius ; if (  t.tuz2>1024  )  t.tuz2 = 1024;
			if (  t.terrainundo.tux1>t.tux1  )  t.terrainundo.tux1 = t.tux1;
			if (  t.terrainundo.tux2<t.tux2  )  t.terrainundo.tux2 = t.tux2;
			if (  t.terrainundo.tuz1>t.tuz1  )  t.terrainundo.tuz1 = t.tuz1;
			if (  t.terrainundo.tuz2<t.tuz2  )  t.terrainundo.tuz2 = t.tuz2;
			for ( t.tuz = t.tuz1 ; t.tuz<=  t.tuz2; t.tuz++ )
			{
				for ( t.tux = t.tux1 ; t.tux<=  t.tux2; t.tux++ )
				{
					if (  t.terrainundobuffer[t.tux][t.tuz] == -1 ) 
					{
						t.tx_f=t.tux*50 ; t.tz_f=t.tuz*50;
						t.terrainundobuffer[t.tux][t.tuz]=BT_GetGroundHeight(t.terrain.TerrainID,t.tx_f,t.tz_f,1);
					}
				}
			}
			t.terrainundo.bufferfilled=1;

			// 161115 - need this flag to ensure undo can happen again
			t.entityundo.undoperformed = 0;

		}
	}
}

void terrain_undo ( void )
{
	if (  t.entityundo.undoperformed == 0 ) 
	{
		if (  t.terrainundo.bufferfilled == 1 ) 
		{
			//  record heights for redo buffer
			for ( t.tuz = 0 ; t.tuz<=  1024; t.tuz++ )
			{
				for ( t.tux = 0 ; t.tux<=  1024; t.tux++ )
				{
					t.terrainredobuffer[t.tux][t.tuz]=-1;
				}
			}
			//  replace with old terrain heights
			for ( t.tuz = t.terrainundo.tuz1 ; t.tuz<=  t.terrainundo.tuz2; t.tuz++ )
			{
				for ( t.tux = t.terrainundo.tux1 ; t.tux<=  t.terrainundo.tux2; t.tux++ )
				{
					if (  t.terrainundobuffer[t.tux][t.tuz] != -1 ) 
					{
						if (  t.terrainredobuffer[t.tux][t.tuz] == -1 ) 
						{
							t.tx_f=t.tux*50 ; t.tz_f=t.tuz*50;
							t.terrainredobuffer[t.tux][t.tuz]=BT_GetGroundHeight(t.terrain.TerrainID,t.tx_f,t.tz_f,1);
						}
						BT_SetPointHeight (  t.terrain.TerrainID,t.tux,t.tuz,t.terrainundobuffer[t.tux][t.tuz] );
					}
				}
			}

			// replace new terrain paint with old one
			if (  MemblockExist(123) == 1 && MemblockExist(124) == 1 ) 
			{
				// first remember the original image (for the redo)
				CreateMemblockFromImage ( 125, t.terrain.imagestartindex+2 );

				// create restored paint texture from undo memblock
				CopyMemblock ( 124,123,0,0,GetMemblockSize(124) );
				CreateImageFromMemblock ( t.terrain.imagestartindex+2,123 );
				TextureObject ( t.terrain.terrainobjectindex, 0, t.terrain.imagestartindex+2 );
			}
			t.terrainundo.bufferfilled=1;
		}
	}
}

void terrain_redo ( void )
{
	//  310315 - can only redo if something in buffer (cannot redo if NO terrain activity yet)
	if (  t.entityundo.undoperformed == 1 && t.terrainundo.bufferfilled == 1 ) 
	{
		//  reverse terrain buffers
		for ( t.tuz = 0 ; t.tuz<=  1024; t.tuz++ )
		{
			for ( t.tux = 0 ; t.tux<=  1024; t.tux++ )
			{
				t.terrainundobuffer[t.tux][t.tuz]=t.terrainredobuffer[t.tux][t.tuz];
				t.terrainredobuffer[t.tux][t.tuz]=-1;
			}
		}
		//  replace with old terrain heights
		for ( t.tuz = 0 ; t.tuz<=  1024; t.tuz++ )
		{
			for ( t.tux = 0 ; t.tux<=  1024; t.tux++ )
			{
				if (  t.terrainundobuffer[t.tux][t.tuz] != -1 ) 
				{
					if (  t.terrainredobuffer[t.tux][t.tuz] == -1 ) 
					{
						t.tx_f=t.tux*50 ; t.tz_f=t.tuz*50;
						t.terrainredobuffer[t.tux][t.tuz]=BT_GetGroundHeight(t.terrain.TerrainID,t.tx_f,t.tz_f,1);
					}
					BT_SetPointHeight (  t.terrain.TerrainID,t.tux,t.tuz,t.terrainundobuffer[t.tux][t.tuz] );
				}
			}
		}

		//  put back the original image before the undo happened
		if (  MemblockExist(123) == 1 && MemblockExist(124) == 1 && MemblockExist(125) == 1 ) 
		{
			//  before redo image, record present image for possible later undo
			CreateMemblockFromImage ( 124, t.terrain.imagestartindex+2 );

			//  create image from redo buffer memblock (123 on this occasion)
			CopyMemblock (  125,123,0,0,GetMemblockSize(125) );
			CreateImageFromMemblock (  t.terrain.imagestartindex+2,123 );
			TextureObject (  t.terrain.terrainobjectindex,0,t.terrain.imagestartindex+2 );
		}

		//  undo buffer ready for undoing the redo
		for ( t.tuz = 0 ; t.tuz<=  1024; t.tuz++ )
		{
			for ( t.tux = 0 ; t.tux<=  1024; t.tux++ )
			{
				t.terrainundobuffer[t.tux][t.tuz]=t.terrainredobuffer[t.tux][t.tuz];
			}
		}
		t.terrainundo.bufferfilled=1;
	}
}

void terrain_editcontrol_auxiliary ( void )
{
	//  some terrain controls are triggered by entity placement
	if (  t.terrain.TerrainID>0 ) 
	{
		//  Sculpt Terrain Shapes
		if (  t.terrain.terrainpainteroneshot == 1 ) 
		{
			//  reset terrain region
			terrain_clearterraindirtyregion ( );
			t.terrain.terrainregionx1=-1;
			//  magnify (by a percent)
			t.terrain.shapeWidth_f=t.terrain.shapeWidth_f*1.1;
			t.terrain.shapeLong_f=t.terrain.shapeLong_f*1.25;
			//  record old terrain for the undo
			t.tstrad_f=t.terrain.RADIUS_f;
			t.terrain.RADIUS_f=t.terrain.shapeWidth_f;
			if (  t.terrain.shapeLong_f>t.terrain.RADIUS_f  )  t.terrain.RADIUS_f = t.terrain.shapeLong_f;
			t.terrainundo.mode=0;
			t.terrain.RADIUS_f=t.terrain.RADIUS_f*1.25;
			terrain_recordbuffer ( );
			terrain_recordbuffer ( );
			t.terrain.RADIUS_f=t.tstrad_f;
			//  set ground height to rotated rectangular platform (building placement)
			t.ttsideshiftstart_f=((t.terrain.shapeWidth_f/-2));
			t.ttsideshiftfinish_f=((t.terrain.shapeWidth_f/2)+50);
			t.ttsideshiftstartmargin_f=t.ttsideshiftstart_f*1.75;
			t.ttsideshiftfinishmargin_f=t.ttsideshiftfinish_f*1.75;
			for ( t.ttsideshift_f = t.ttsideshiftstartmargin_f ; t.ttsideshift_f<=  t.ttsideshiftfinishmargin_f ; t.ttsideshift_f+= 10 )
			{
				t.terrain.shapestartX_f=NewXValue(t.terrain.X_f,t.terrain.shapeA_f+90,t.ttsideshift_f);
				t.terrain.shapestartZ_f=NewZValue(t.terrain.Y_f,t.terrain.shapeA_f+90,t.ttsideshift_f);
				t.terrain.shapelineX1_f=NewXValue(t.terrain.shapestartX_f,t.terrain.shapeA_f+180,(t.terrain.shapeLong_f/2));
				t.terrain.shapelineZ1_f=NewZValue(t.terrain.shapestartZ_f,t.terrain.shapeA_f+180,(t.terrain.shapeLong_f/2));
				t.terrain.shapelineX2_f=NewXValue(t.terrain.shapestartX_f,t.terrain.shapeA_f+0,(t.terrain.shapeLong_f/2)+50);
				t.terrain.shapelineZ2_f=NewZValue(t.terrain.shapestartZ_f,t.terrain.shapeA_f+0,(t.terrain.shapeLong_f/2)+50);
				t.terrain.shapeStep_f=t.terrain.shapeLong_f;
				t.ttsubsteps=10;
				t.terrain.shapeincX_f=((t.terrain.shapelineX2_f-t.terrain.shapelineX1_f))/(t.terrain.shapeStep_f/t.ttsubsteps);
				t.terrain.shapeincZ_f=((t.terrain.shapelineZ2_f-t.terrain.shapelineZ1_f))/(t.terrain.shapeStep_f/t.ttsubsteps);
				t.terrain.stpX_f=t.terrain.shapelineX1_f;
				t.terrain.stpZ_f=t.terrain.shapelineZ1_f;
				t.terrain.stpX_f=t.terrain.stpX_f-(t.terrain.shapeincX_f*(t.terrain.shapeStep_f/t.ttsubsteps/2));
				t.terrain.stpZ_f=t.terrain.stpZ_f-(t.terrain.shapeincZ_f*(t.terrain.shapeStep_f/t.ttsubsteps/2));
				for ( t.ttstp = 1 ; t.ttstp <= int(t.terrain.shapeStep_f*2) ; t.ttstp+= t.ttsubsteps )
				{

					//  work out percentage of new terrain height influence
					t.tnewheightinfluence_f=0.0;
					if (  t.ttsideshift_f>t.ttsideshiftstart_f && t.ttsideshift_f<t.ttsideshiftfinish_f ) 
					{
						t.tnewheightinfluence_f=1.0;
					}
					else
					{
						t.tsize_f=abs(t.ttsideshiftstartmargin_f-t.ttsideshiftstart_f);
						if (  t.ttsideshift_f <= t.ttsideshiftstart_f ) 
						{
							t.tnewheightinfluence_f=(abs(t.ttsideshiftstartmargin_f)-abs(t.ttsideshift_f))/t.tsize_f;
						}
						else
						{
							if (  t.ttsideshift_f >= t.ttsideshiftfinish_f ) 
							{
								t.tnewheightinfluence_f=(abs(t.ttsideshiftfinishmargin_f)-abs(t.ttsideshift_f))/t.tsize_f;
							}
						}
					}
					t.tnewheightinfluence2_f=0.0;
					if (  t.ttstp>int(t.terrain.shapeStep_f*0.5) && t.ttstp<int(t.terrain.shapeStep_f*1.5) ) 
					{
						t.tnewheightinfluence2_f=1.0;
					}
					else
					{
						t.tsize_f=t.terrain.shapeStep_f*0.5;
						if (  t.ttstp <= int(t.terrain.shapeStep_f*0.5) ) 
						{
							t.tnewheightinfluence2_f=(t.ttstp+0.0)/t.tsize_f;
						}
						else
						{
							if (  t.ttstp >= int(t.terrain.shapeStep_f*1.5) ) 
							{
								t.tnewheightinfluence2_f=((abs(t.terrain.shapeStep_f*2)-abs(t.ttstp))+0.0)/t.tsize_f;
							}
						}
					}
					t.tnewheightinfluence_f=(t.tnewheightinfluence_f+t.tnewheightinfluence2_f)/2.0;
					if (  t.tnewheightinfluence_f<0.0  )  t.tnewheightinfluence_f = 0.0;
					if (  t.tnewheightinfluence_f>1.0  )  t.tnewheightinfluence_f = 1.0;

					//  get old height
					t.x=t.terrain.stpX_f/50.0;
					t.z=t.terrain.stpZ_f/50.0;
					t.toldheight_f=BT_GetGroundHeight(t.terrain.TerrainID,t.x*50.0,t.z*50.0);
					t.tfinalheight_f=(t.toldheight_f*(1.0-t.tnewheightinfluence_f))+(t.terrain.shapeHeight_f*t.tnewheightinfluence_f);

					//  apply new height
					BT_SetPointHeight (  t.terrain.TerrainID,t.x,t.z,t.tfinalheight_f );

					//  advance step
					t.terrain.stpX_f=t.terrain.stpX_f+t.terrain.shapeincX_f;
					t.terrain.stpZ_f=t.terrain.stpZ_f+t.terrain.shapeincZ_f;

					//  record region dirtied
					if (  t.terrain.terrainregionx1 == -1 ) 
					{
						t.terrain.terrainregionx1=t.x;
						t.terrain.terrainregionx2=t.x;
						t.terrain.terrainregionz1=t.z;
						t.terrain.terrainregionz2=t.z;
					}
					if (  t.x<t.terrain.terrainregionx1  )  t.terrain.terrainregionx1 = t.x;
					if (  t.x>t.terrain.terrainregionx2  )  t.terrain.terrainregionx2 = t.x;
					if (  t.z<t.terrain.terrainregionz1  )  t.terrain.terrainregionz1 = t.z;
					if (  t.z>t.terrain.terrainregionz2  )  t.terrain.terrainregionz2 = t.z;

				}
			}

			//  ensure terrain update region is updated
			if (  t.terrain.terrainregionx1<0  )  t.terrain.terrainregionx1 = 0;
			if (  t.terrain.terrainregionx2>1024  )  t.terrain.terrainregionx2 = 1024;
			if (  t.terrain.terrainregionz1<0  )  t.terrain.terrainregionz1 = 0;
			if (  t.terrain.terrainregionz2>1024  )  t.terrain.terrainregionz2 = 1024;

			//  only once
			t.terrain.X_f=-1000000 ; t.terrain.Y_f=-1000000;
			t.terrain.terrainpainteroneshot=2;

		}
		else
		{

			//  After terrain sculpt, move any
			if (  t.terrain.terrainpainteroneshot == 2 ) 
			{
				//  also rescue any entities that have been sent underground as a result
				for ( t.teee = 1 ; t.teee<=  g.entityelementlist; t.teee++ )
				{
					if (  t.entityelement[t.teee].x >= t.terrain.terrainregionx1*50 && t.entityelement[t.teee].x <= t.terrain.terrainregionx2*50 ) 
					{
						if (  t.entityelement[t.teee].z >= t.terrain.terrainregionz1*50 && t.entityelement[t.teee].z <= t.terrain.terrainregionz2*50 ) 
						{
							t.ttobj=t.entityelement[t.teee].obj;
							if (  t.ttobj>0 ) 
							{
								if (  ObjectExist(t.ttobj) == 1 ) 
								{
									if (  GetVisible(t.ttobj) == 1 ) 
									{
										t.entityelement[t.teee].y=BT_GetGroundHeight(t.terrain.TerrainID,t.entityelement[t.teee].x,t.entityelement[t.teee].z);
										PositionObject (  t.ttobj,t.entityelement[t.teee].x,t.entityelement[t.teee].y,t.entityelement[t.teee].z );
									}
								}
							}
						}
					}
				}
				//  now done
				t.terrain.terrainpainteroneshot=0;
			}
		}
	}
}

void terrain_paintterrain ( void )
{
	// receives terrain.X# Y# RADIUS# texselect texpainttype
	// create memblock for image manipulation
	if ( MemblockExist(123) == 0 )  
	{
		CreateMemblockFromImage (  123,t.terrain.imagestartindex+2 );
	}
	if ( MemblockExist(123) == 1 ) 
	{
		// memblock header
		t.twid=ReadMemblockDWord(123,0);
		t.thig=ReadMemblockDWord(123,4);
		t.tdep=ReadMemblockDWord(123,8);

		// calculate coordinates to draw to
		t.tcntx=int(t.terrain.X_f/25.0)+1;
		t.tcntz=int(t.terrain.Y_f/25.0)+1;
		t.trad_f=t.terrain.RADIUS_f/35.0;
		t.trad=int(t.trad_f)-1;
		t.tzs = t.tcntz-t.trad; if ( t.tzs < 0 ) t.tzs = 0;
		t.tzf = t.tcntz+t.trad; if ( t.tzf > 2047 ) t.tzf = 2047;
		for ( t.tz = t.tzs; t.tz <= t.tzf; t.tz++ )
		{
			t.tti = (((t.tz-t.tzs)+0.0)/((t.tzf+0.0)-(t.tzs+0.0)))*100;
			if ( t.tti < 0 ) t.tti = 0;
			if ( t.tti > 100 ) t.tti = 100;
			t.txs = t.tcntx-(t.curve_f[t.tti]*t.trad); if ( t.txs < 0 ) t.txs = 0;
			t.txf = t.tcntx+(t.curve_f[t.tti]*t.trad); if ( t.txf > 2047 ) t.txf = 2047;
			for ( t.tx = t.txs; t.tx <= t.txf; t.tx++ )
			{
				t.mpos=12+(((t.tz*2048)+t.tx)*4);
				t.mrgb=ReadMemblockDWord(123,t.mpos);
				if ( t.texselectgrass == 1 )
				{
					// paint grass - do not affect floor
					t.tnewr=(RgbR(t.mrgb)*0.85f)+(RgbR(t.texselect)*0.15f);
					t.tnewg=RgbG(t.mrgb);
					t.tnewb=RgbB(t.mrgb);
				}
				else
				{
					// paint terrain floor - leave grass alone
					if ( RgbB(t.texselect) == 0 )
					{
						// slot 1 (path texture slot) also paints into overlay channel
						t.tnewr=RgbR(t.mrgb);
						t.tnewg=(RgbG(t.mrgb)*0.85f)+(255*0.15f);
						t.tnewb=RgbB(t.mrgb);
					}
					else
					{
						// regular transitions layer for 16 textures
						t.tnewr=RgbR(t.mrgb);
						t.tnewg=(RgbG(t.mrgb)*0.85f)+(RgbG(t.texselect)*0.15f);
						t.tnewb=(RgbB(t.mrgb)*0.85f)+(RgbB(t.texselect)*0.15f);
					}
				}
				WriteMemblockDWord ( 123, t.mpos, Rgb(t.tnewr,t.tnewg,t.tnewb) );
			}
		}
		CreateImageFromMemblock (  t.terrain.imagestartindex+2,123 );
		TextureObject (  t.terrain.terrainobjectindex,0,t.terrain.imagestartindex+2 );
	}
}

void terrain_cursor ( void )
{
	if (  t.terrain.terrainshaderindex>0 ) 
	{
		t.thalf_f=1024.0*25.0;
		float fHighlightX = t.terrain.X_f + 25;
		float fHighlightY = t.terrain.Y_f + 25;
		SetVector4 (  g.terrainvectorindex,(fHighlightX-t.thalf_f)/102.4,(fHighlightY-t.thalf_f)/102.4,t.terrain.RADIUS_f,0 );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightCursor",g.terrainvectorindex );
		//if (  t.terrain.terrainpaintermode >= 6 && t.terrain.terrainpaintermode <= 10 ) 
		if ( t.terrain.terrainpaintermode == 10 ) 
		{
			//  only show grass in GRASS paint mode
			SetVector4 (  g.terrainvectorindex,1,1,1,1 );
		}
		else
		{
			SetVector4 (  g.terrainvectorindex,1,0,1,1 );
		}
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightParams",g.terrainvectorindex );
	}
}

void terrain_cursor_nograsscolor ( void )
{
	if (  t.terrain.terrainshaderindex>0 ) 
	{
		t.thalf_f=1024.0*25.0;
		SetVector4 (  g.terrainvectorindex,(t.terrain.X_f-t.thalf_f)/102.4,(t.terrain.Y_f-t.thalf_f)/102.4,t.terrain.RADIUS_f,0 );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightCursor",g.terrainvectorindex );
		SetVector4 (  g.terrainvectorindex,1,0,t.terrain.entityeditmodecursorR,t.terrain.entityeditmodecursorG );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightParams",g.terrainvectorindex );
	}
}

void terrain_cursor_off ( void )
{
	if (  t.terrain.terrainshaderindex>0 ) 
	{
		SetVector4 (  g.terrainvectorindex,0,0,0,0 );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightCursor",g.terrainvectorindex );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightParams",g.terrainvectorindex );
	}
}

void terrain_renderonly ( void )
{
	BT_RenderTerrain (  t.terrain.TerrainID );
}

void terrain_update ( void )
{
	if ( t.terrain.TerrainID > 0 ) 
	{
		BT_SetCurrentCamera (  t.terrain.gameplaycamera );
		BT_UpdateTerrainCull (  t.terrain.TerrainID );
		BT_UpdateTerrainLOD (  t.terrain.TerrainID );
		if ( g.globals.riftmode>0 ) 
		{
			//  rendered in _postprocess_preterrain
		}
		else
		{
			//  main terrain render for normal mode
			terrain_renderonly ( );
		}
	}
}

void terrain_clearterraindirtyregion ( void )
{
	t.terrain.terrainregionupdate=0;
	t.terrain.terrainregionx1=0;
	t.terrain.terrainregionx2=0;
	t.terrain.terrainregionz1=0;
	t.terrain.terrainregionz2=0;
}

void terrain_cleargrassdirtyregion ( void )
{
	t.terrain.grassregionupdate=0;
	t.terrain.grassregionx1=0;
	t.terrain.grassregionx2=0;
	t.terrain.grassregionz1=0;
	t.terrain.grassregionz2=0;
}

void terrain_cleardirtyregion ( void )
{
	terrain_clearterraindirtyregion ( );
	terrain_cleargrassdirtyregion ( );
}

void terrain_waterineditor ( void )
{
	PositionObject (  t.terrain.objectstartindex+2,ObjectPositionX(t.terrain.objectstartindex+2),t.terrain.waterliney_f-t.terrain.waterlineyadjustforclip_f,ObjectPositionZ(t.terrain.objectstartindex+2) );
}

void terrain_assignnewshader ( void )
{
	// Choose the terrain shader to use
	//if ( t.terrain.iTerrainPBRMode == 1 )
	//if ( g.gpbroverride == 1 )
	//	t.terrain.terrainshaderindex = t.terrain.effectstartindex+5;
	//else
	t.terrain.terrainshaderindex = t.terrain.effectstartindex+0;

	// Terrain shader constants
	memset ( &t.effectparam.terrain, 0, sizeof(t.effectparam.terrain) );
	if ( GetEffectExist ( t.terrain.terrainshaderindex ) == 1 )
	{
		t.effectparam.terrain.g_lights_data=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_data");
		t.effectparam.terrain.g_lights_pos0=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_pos0");
		t.effectparam.terrain.g_lights_atten0=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_atten0");
		t.effectparam.terrain.g_lights_diffuse0=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_diffuse0");
		t.effectparam.terrain.g_lights_pos1=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_pos1");
		t.effectparam.terrain.g_lights_atten1=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_atten1");
		t.effectparam.terrain.g_lights_diffuse1=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_diffuse1");
		t.effectparam.terrain.g_lights_pos2=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_pos2");
		t.effectparam.terrain.g_lights_atten2=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_atten2");
		t.effectparam.terrain.g_lights_diffuse2=GetEffectParameterIndex(t.terrain.terrainshaderindex,"g_lights_diffuse2");
		t.effectparam.terrain.SpotFlashPos=GetEffectParameterIndex(t.terrain.terrainshaderindex,"SpotFlashPos");
		t.effectparam.terrain.SpotFlashColor=GetEffectParameterIndex(t.terrain.terrainshaderindex,"SpotFlashColor");

		// wipe any previous param storage
		ResetEffect ( t.terrain.terrainshaderindex );

		// and ensure this terrain becoms responsible for shadow map generation
		ChangeShadowMappingPrimary ( t.terrain.terrainshaderindex );
	}
}

void terrain_applyshader ( void )
{
	// Apply effect and textures to terrain object
	if ( ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		// Choose the terrain shader to use
		terrain_assignnewshader();

		// get handles to this new shader to place shadow ptrs
		LPGGEFFECT pEffectPtr = NULL;
		cSpecialEffect* pEffectObject = m_EffectList [ t.terrain.terrainshaderindex ]->pEffectObj;
		if ( pEffectObject )
			if ( pEffectObject->m_pEffect )
				pEffectPtr = pEffectObject->m_pEffect;
		if ( pEffectPtr )
		{
			g_CascadedShadow.m_depthHandle[0] = pEffectPtr->GetVariableByName( "DepthMapTX1" );
			g_CascadedShadow.m_depthHandle[1] = pEffectPtr->GetVariableByName( "DepthMapTX2" );
			g_CascadedShadow.m_depthHandle[2] = pEffectPtr->GetVariableByName( "DepthMapTX3" );
			g_CascadedShadow.m_depthHandle[3] = pEffectPtr->GetVariableByName( "DepthMapTX4" );
			g_CascadedShadow.m_depthHandle[4] = pEffectPtr->GetVariableByName( "DepthMapTX5" );
			g_CascadedShadow.m_depthHandle[5] = pEffectPtr->GetVariableByName( "DepthMapTX6" );
			g_CascadedShadow.m_depthHandle[6] = pEffectPtr->GetVariableByName( "DepthMapTX7" );
			g_CascadedShadow.m_depthHandle[7] = pEffectPtr->GetVariableByName( "DepthMapTX8" );
		}

		// Apply terrain shader
		SetObjectEffect ( t.terrain.terrainobjectindex, t.terrain.terrainshaderindex );

		// Apply textures for shader
		if ( g.gpbroverride == 0 )//t.terrain.iTerrainPBRMode == 0 )
		{
			// non-PBR
			TextureObject ( t.terrain.terrainobjectindex,0,t.terrain.imagestartindex+2 );
			TextureObject ( t.terrain.terrainobjectindex,1,g.postprocessimageoffset+5 );
			TextureObject ( t.terrain.terrainobjectindex,2,t.terrain.imagestartindex+13 );
			TextureObject ( t.terrain.terrainobjectindex,3,t.terrain.imagestartindex+17 );
			TextureObject ( t.terrain.terrainobjectindex,4,t.terrain.imagestartindex+21 );
		}
		else
		{
			// Get PBR support textures
			if ( ImageExist ( t.terrain.imagestartindex+14 ) == 0 ) LoadImage ( "effectbank\\reloaded\\media\\AGED.png",t.terrain.imagestartindex+14 );
			if ( ImageExist ( t.terrain.imagestartindex+15 ) == 0 ) LoadImage ( "effectbank\\reloaded\\media\\blank_black.dds",t.terrain.imagestartindex+15 );

			// PBR
			TextureObject ( t.terrain.terrainobjectindex, 0, t.terrain.imagestartindex+2 ); //VEGMAP RBG = Distant terrain texture,  A=circle for editor
			TextureObject ( t.terrain.terrainobjectindex, 1, t.terrain.imagestartindex+14 );//AGED map (ao, gloss, height, detail)
			TextureObject ( t.terrain.terrainobjectindex, 2, t.terrain.imagestartindex+13 );//Diffuse
			TextureObject ( t.terrain.terrainobjectindex, 3, t.terrain.imagestartindex+17 );//Highlighter
			TextureObject ( t.terrain.terrainobjectindex, 4, t.terrain.imagestartindex+21 );//Normal
			TextureObject ( t.terrain.terrainobjectindex, 5, t.terrain.imagestartindex+15 );//SpecularMap
			TextureObject ( t.terrain.terrainobjectindex, 6, t.terrain.imagestartindex+31 );//EnvironmentMap
			if (g.memskipibr == 0) TextureObject ( t.terrain.terrainobjectindex, 8, t.terrain.imagestartindex+32 );//GlossCurveMap
		}
	}
}

void terrain_createactualterrain ( void )
{
	// Load PBR env map (real CUBE moved to later when terrain loads in, for now use default)
	cstr texEnvMap = "effectbank\\reloaded\\media\\CUBE.dds";
	LoadImage ( texEnvMap.Get(), t.terrain.imagestartindex+31, 2 );

	//  Uses terrain.imagestartindex+3 for heightmap image
	if (  t.terrain.TerrainID != 0 ) 
	{
		BT_DeleteTerrain (  t.terrain.TerrainID );
		t.terrain.TerrainID=0;
	}
	if (  t.terrain.superflat == 0 ) 
	{
		t.terrain.TerrainID=BT_MakeTerrain();
		if (  FileExist(t.theightfile_s.Get()) == 1 ) 
		{
			SetMipmapNum(1); //PE: mipmaps not needed.
			LoadImage (  t.theightfile_s.Get(),t.terrain.imagestartindex+3 );
			SetMipmapNum(-1);
		}
		else
		{
			SetMipmapNum(1); //PE: mipmaps not needed.
			LoadImage (  "effectbank\\reloaded\\media\\heightmap.dds",t.terrain.imagestartindex+3 );
			SetMipmapNum(-1);
		}
		BT_SetTerrainHeightmap (  t.terrain.TerrainID,t.terrain.imagestartindex+3 );
		BT_SetTerrainScale (  t.terrain.TerrainID,50 );
		BT_SetTerrainYScale (  t.terrain.TerrainID,8 );
		BT_SetTerrainSplit (  t.terrain.TerrainID,32 ); // 220317 - for new terrain texture resolution - 16 );
		BT_SetTerrainQuadRotation (  t.terrain.TerrainID,1 );
		BT_SetTerrainSmoothing (  t.terrain.TerrainID,0 );
		BT_SetTerrainDetailTile (  t.terrain.TerrainID,2 );
		BT_SetTerrainLOD (  t.terrain.TerrainID,3 );
		BT_SetTerrainLODDistance (  t.terrain.TerrainID,1,8000 );
		BT_SetTerrainLODDistance (  t.terrain.TerrainID,2,15000 );
		BT_BuildTerrain (  t.terrain.TerrainID,t.terrain.terrainobjectindex,1 );
		t.terrain.TerrainSize_f=BT_GetTerrainSize(t.terrain.TerrainID);
		SetTerrainRenderLevel (  100 );
	}
	else
	{
		//  Super Flat Terrain (for non landscape levels)
		if (  ObjectExist(t.terrain.terrainobjectindex) == 1  )  DeleteObject (  t.terrain.terrainobjectindex );
		if (  t.terrain.superflat == 2 ) 
		{
			//  no terrain object at all
		}
		else
		{
			MakeObjectPlane (  t.terrain.terrainobjectindex,512*100,512*100 );
			RotateObject (  t.terrain.terrainobjectindex,90,180,0 );
			PositionObject (  t.terrain.terrainobjectindex,256*100,1000,256*100 );
			SetObjectCull (  t.terrain.terrainobjectindex,0 );
			ScaleObject (  t.terrain.terrainobjectindex,100,100,-100 );
		}
		t.terrain.TerrainID=0;
	}

	//  mask set in visuals_loop (nope, now here so shadow not rendering this object)
	if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		SetObjectMask (  t.terrain.terrainobjectindex, 1 );
	}

	//  Apply effect and textures to terrain object
	terrain_applyshader();
}

void terrain_make ( void )
{
	// Terrain system
	t.terrain.terrainobjectindex=t.terrain.objectstartindex+3;
	if ( ObjectExist(t.terrain.terrainobjectindex) == 0 ) 
	{
		// Load terrain shaders (non-PBR and PBR) (applied later in terrain_applyshader)
		//if ( GetEffectExist(t.terrain.effectstartindex+0) == 0 ) LoadEffect ( "effectbank\\reloaded\\terrain_basic.fx", t.terrain.effectstartindex+0, 0 );
		//if ( GetEffectExist(t.terrain.effectstartindex+5) == 0 ) LoadEffect ( "effectbank\\reloaded\\apbr_terrain.fx", t.terrain.effectstartindex+5, 0 );
		if ( GetEffectExist(t.terrain.effectstartindex+0) == 0 )
		{
			LPSTR pEffectToUse = "effectbank\\reloaded\\terrain_basic.fx";
			if ( g.gpbroverride == 1 ) pEffectToUse = "effectbank\\reloaded\\apbr_terrain.fx";
			LoadEffect ( pEffectToUse, t.terrain.effectstartindex+0, 0 );
			timestampactivity(0, cstr(cstr("Terrain Shader:")+pEffectToUse).Get() );
		}
		terrain_assignnewshader();

		// IBR curve loopup map as globals
		if (g.memskipibr == 0) 
		{
			cstr texIBRMap = "effectbank\\reloaded\\media\\IBR.png";
			LoadImage(texIBRMap.Get(), t.terrain.imagestartindex + 32);
		}

		// Prepare shader as a shadow mapping primary effect (updated also in terrain_applyshader function if iTerrainPBRMode changes)
		SetEffectShadowMappingMode ( 255 );
		SetEffectToShadowMappingEx ( t.terrain.terrainshaderindex, g.shadowdebugobjectoffset, g.guidepthshadereffectindex, g.globals.hidedistantshadows, g.globals.realshadowresolution, g.globals.realshadowcascadecount, g.globals.realshadowcascade[0], g.globals.realshadowcascade[1], g.globals.realshadowcascade[2], g.globals.realshadowcascade[3], g.globals.realshadowcascade[4], g.globals.realshadowcascade[5], g.globals.realshadowcascade[6], g.globals.realshadowcascade[7] );

		//  Load all terrain textures
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
		if (  t.terrain.superflat == 2 ) 
		{
			//  No terrain to texture
		}
		else
		{
			//if (  FileExist("levelbank\\testmap\\vegmask.dds") == 1 ) 
			if (  FileExist(cstr(g.mysystem.levelBankTestMap_s+"vegmask.dds").Get()) == 1 ) 
			{
				if (  ImageExist(t.terrain.imagestartindex+2) == 0 ) 
				{
					SetMipmapNum(1);
					//LoadImage (  "levelbank\\testmap\\vegmask.dds",t.terrain.imagestartindex+2,10,0 );
					LoadImage ( cstr(g.mysystem.levelBankTestMap_s+"vegmask.dds").Get(),t.terrain.imagestartindex+2,10,0 );
					if ( ImageExist ( t.terrain.imagestartindex+2 ) == 0 )
					{
						terrain_generatevegandmask_grab ( );
					}
					SetMipmapNum(-1);
					if (  MemblockExist(123) == 1  )  DeleteMemblock (  123 );
					CreateMemblockFromImage ( 123, t.terrain.imagestartindex+2 );
				}
			}
			else
			{
				terrain_generatevegandmask_grab ( );
			}

			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			if (  g.gdividetexturesize == 0 ) 
			{
				t.tthistexdir_s="effectbank\\reloaded\\media\\white_D.dds";
				LoadImage (  t.tthistexdir_s.Get(),t.terrain.imagestartindex+13,0,g.gdividetexturesize );
				LoadImage (  t.tthistexdir_s.Get(),t.terrain.imagestartindex+21,0,g.gdividetexturesize );
			}
			else
			{
				LoadImage ( cstr(cstr("terrainbank\\")+g.terrainstyle_s+"\\Texture_D.dds").Get(),t.terrain.imagestartindex+13,0,g.gdividetexturesize );
				LoadImage ( cstr(cstr("terrainbank\\")+g.terrainstyle_s+"\\Texture_N.dds").Get(),t.terrain.imagestartindex+21,0,g.gdividetexturesize );
			}
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			// This texture acts as highlight graphic and also store for mega texture (distant terrain texture composite)
			SetImageAutoMipMap (  0 ); // PE: SetImageAutoMipMap Dont work anymore.
			SetMipmapNum(1); //PE: mipmaps not needed.
			LoadImage (  "effectbank\\reloaded\\media\\circle.dds",t.terrain.imagestartindex+17,10,0 );
			SetMipmapNum(-1);
			SetImageAutoMipMap (  1 );
		}
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

		//  Water handling vars
		t.terrain.WaterCamY_f=0.0;
	}

	//  By default, always select NEAR technique
	BT_ForceTerrainTechnique (  1 );

	// 100417 - when terrain created, reset water line for new terrain
	t.terrain.waterliney_f = g.gdefaultwaterheight;
}

void terrain_load ( void )
{
	if ( FileExist(t.tfile_s.Get()) == 1 && t.terrain.TerrainID > 0 ) 
	{
		OpenToRead ( 1, t.tfile_s.Get() );
		if ( MemblockExist(1) == 0 ) 
		{
			ReadMemblock ( 1, 1 );
			t.mi = 0;
			for ( t.z = 0; t.z <= 1023; t.z++ )
			{
				for ( t.x = 0 ; t.x <= 1023; t.x++ )
				{
					t.h_f = ReadMemblockFloat(1,t.mi);
					t.terrainmatrix[t.x][t.z] = t.h_f;
					t.mi += 4;
					BT_SetPointHeight ( t.terrain.TerrainID, t.x, t.z, t.h_f );
				}
			}
			DeleteMemblock (  1 );

			// force a shore on all terrain imported
			t.x = 0; for ( t.z = 0 ; t.z <= 1023 ; t.z++ ) BT_SetPointHeight ( t.terrain.TerrainID, t.x, t.z, 0.0  );
			t.x = 1023; for ( t.z = 0 ; t.z<= 1023 ; t.z++ ) BT_SetPointHeight ( t.terrain.TerrainID, t.x, t.z, 0.0  );
			t.z = 0 ; for ( t.x = 0 ; t.x <= 1023 ; t.x++ ) BT_SetPointHeight ( t.terrain.TerrainID, t.x, t.z, 0.0  );
			t.z = 1023; for ( t.x = 0 ; t.x <= 1023 ; t.x++ ) BT_SetPointHeight ( t.terrain.TerrainID, t.x, t.z, 0.0  );
		}
		else
		{
			ExitPrompt (  "Memblock 1 already exists!","Terrain"  ); ExitProcess ( 0 );
		}
		CloseFile (  1 );
	}
}

void terrain_save ( void )
{
	if (  t.terrain.TerrainID>0 ) 
	{
		if ( FileExist(t.tfile_s.Get()) == 1 ) DeleteAFile ( t.tfile_s.Get() );
		if ( MemblockExist(1) == 0 ) 
		{
			MakeMemblock ( 1,1024*1024*4 );
			OpenToWrite ( 1, t.tfile_s.Get() );
			t.mi=0;
			for ( t.z = 0 ; t.z<=  1023; t.z++ )
			{
				for ( t.x = 0 ; t.x<=  1023; t.x++ )
				{
					t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,t.x*50.0,t.z*50.0,1);
					if (  t.h_f<0.0  )  t.h_f = 0.0;
					WriteMemblockFloat (  1,t.mi,t.h_f );
					t.mi += 4;
				}
			}
			WriteMemblock ( 1,1 );
			CloseFile ( 1 );
			DeleteMemblock ( 1 );
		}
		else
		{
			ExitPrompt (  "Memblock 1 already exists!","Terrain"  ); ExitProcess ( 0 );
		}
	}
}

void terrain_savetextures ( void )
{
	//  save veg map texture
	if (  FileExist(t.tfileveg_s.Get()) == 1  )  DeleteAFile (  t.tfileveg_s.Get() );
	if (  ImageExist(t.terrain.imagestartindex+2) == 1 ) 
	{
		SaveImage (  t.tfileveg_s.Get(),t.terrain.imagestartindex+2 );
	}
	SetCurrentBitmap (  0 );

	//  save water mask texture
	if (  t.terrain.terrainregionx1 != t.terrain.terrainregionx2 || FileExist(t.tfilewater_s.Get()) == 0 ) 
	{
		t.terrain.terrainregionupdate=2;
		terrain_refreshterrainmatrix ( );
		terrain_updatewatermask ( );
	}
}

void terrain_generatevegandmask_grab ( void )
{
	SetCurrentBitmap ( g.terrainworkbitmapindex );
	CLS ( 0, 0, 64 );
	GrabImage ( t.terrain.imagestartindex+2,0,0,MAXTEXTURESIZE,MAXTEXTURESIZE );
	SetCurrentBitmap ( 0 );
}

void terrain_generatevegandmaskfromterrain ( void )
{
	//  newly generated terrain needs updated veg map and water mask
	if (  FileExist(t.tfileveg_s.Get()) == 1  )  DeleteAFile (  t.tfileveg_s.Get() );
	terrain_generatevegandmask_grab ( );
	timestampactivity(0, cstr(cstr("Deleting old memblock:")+Str(MemblockExist(123))).Get() );
	if (  MemblockExist(123) == 1  )  DeleteMemblock (  123 );
	CreateMemblockFromImage ( 123,t.terrain.imagestartindex+2 );

	//  update water mask
	t.terrain.terrainregionupdate=2;
	terrain_refreshterrainmatrix ( );
	if (  t.tgeneratefreshwatermaskflag == 1 ) 
	{
		terrain_updatewatermask_new ( );
		t.tgeneratefreshwatermaskflag=0;
	}
	else
	{
		terrain_updatewatermask ( );
	}

	// trigger vegmap image to be pasted to paint camera
	if (  ObjectExist(t.terrain.terrainobjectindex)  ==  1 ) 
	{
		TextureObject ( t.terrain.terrainobjectindex, 0, t.terrain.imagestartindex+2 );
	}
}

void terrain_generateblanktextures ( void )
{
	//  blank veg
	if (  FileExist(t.tfileveg_s.Get()) == 1  )  DeleteAFile (  t.tfileveg_s.Get() );
	SetCurrentBitmap (  g.terrainworkbitmapindex );
	CLS ( 0, 0, 64 );
	GrabImage (  t.terrain.imagestartindex+2,0,0,MAXTEXTURESIZE,MAXTEXTURESIZE );
	SaveImage (  t.tfileveg_s.Get(),t.terrain.imagestartindex+2 );
	if (  MemblockExist(123) == 1  )  DeleteMemblock (  123 );
	CreateMemblockFromImage (  123,t.terrain.imagestartindex+2 );

	//  blank water mask
	if (  FileExist(t.tfilewater_s.Get()) == 1  )  DeleteAFile (  t.tfilewater_s.Get() );
	CLS (  Rgb(0,0,0) );
	if (g.memskipwatermask == 1) {
		GrabImage(t.terrain.imagestartindex + 4, 0, 0, 1, 1);
	}
	else {
		GrabImage(t.terrain.imagestartindex + 4, 0, 0, MAXTEXTURESIZE, MAXTEXTURESIZE);
	}
	SaveImage (  t.tfilewater_s.Get(),t.terrain.imagestartindex+4 );
	SetCurrentBitmap (  0 );
}

void terrain_loaddata ( void )
{
	// load new terrain into engine
	if ( t.terrain.superflat != 2 )
	{
		t.filename_s=t.levelmapptah_s+"m.dat";
		if ( FileExist(t.filename_s.Get()) == 1 ) 
		{
			// Load terrain height data
			t.tfile_s=t.filename_s ; terrain_load ( );

			// Load veg shadow map
			if ( FileExist( cstr(t.levelmapptah_s+"vegmask.dds").Get() ) == 1 ) 
			{
				if (  ImageExist(t.terrain.imagestartindex+2) == 1  )  DeleteImage (  t.terrain.imagestartindex+2 );
				SetMipmapNum(1);
				LoadImage (  cstr(t.levelmapptah_s+"vegmask.dds").Get(),t.terrain.imagestartindex+2,10,0 );
				SetMipmapNum(-1);
				if (  MemblockExist(123) == 1  )  DeleteMemblock (  123 );
				CreateMemblockFromImage (  123,t.terrain.imagestartindex+2 );
				TextureObject (  t.terrain.terrainobjectindex,0,t.terrain.imagestartindex+2 );
			}

			// Load water mask
			if (g.memskipwatermask == 0 && FileExist( cstr(t.levelmapptah_s+"watermask.dds").Get() ) == 1 )
			{
				if ( ImageExist(t.terrain.imagestartindex+4) == 1  )  DeleteImage (  t.terrain.imagestartindex+4 );
				SetMipmapNum(1);
				LoadImage (  cstr(t.levelmapptah_s+"watermask.dds").Get(),t.terrain.imagestartindex+4,10,0,1 );
				SetMipmapNum(-1);
			}
			if (ImageExist(t.terrain.imagestartindex + 4) == 0)
			{
				GrabImage(t.terrain.imagestartindex + 4, 0, 0, 1, 1);
			}

			// Load veg grass memblock
			if ( FileExist( cstr(t.levelmapptah_s+"vegmaskgrass.dat").Get() ) == 1 ) 
			{
				t.tfileveggrass_s=t.levelmapptah_s+"vegmaskgrass.dat";
				terrain_fastveg_loadgrass ( );
			}

			// Load any custom terrain texture if present
			if ( FileExist( cstr(t.levelmapptah_s+"Texture_D.dds").Get() ) == 1 ) 
			{
				// custom texture in FPM overrides one specified in visual
				g.terrainstyleindex = 1;
			}
			else
			{
				// find terrainstyle index specified in visual
				g.terrainstyle_s = t.visuals.terrain_s;
				for ( g.terrainstyleindex = 1 ; g.terrainstyleindex <= g.terrainstylemax; g.terrainstyleindex++ )
					if ( cstr(Lower(g.terrainstyle_s.Get())) == t.terrainstylebank_s[g.terrainstyleindex] ) 
						break;
				// 080517 - if exceed array (i.e not found) reset to last slot
				if ( g.terrainstyleindex > g.terrainstylemax ) 
				{
					g.terrainstyleindex = g.terrainstylemax;
					g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
				}
			}
			terrain_loadlatesttexture ( );

			// 251017 - moved from actualterrain creation (too soon before)
			// Load PBR env map - if no local CUBE file, load the global cube file if a PBR
			cstr texEnvMap = g.mysystem.levelBankTestMap_s+"globalenvmap.dds"; //"levelbank\\testmap\\globalenvmap.dds";
			if ( FileExist ( texEnvMap.Get() ) == 0 ) texEnvMap = "effectbank\\reloaded\\media\\CUBE.dds";
			LoadImage ( texEnvMap.Get(), t.terrain.imagestartindex+31, 2 );
			TextureObject ( t.terrain.terrainobjectindex, 6, t.terrain.imagestartindex+31 );

			// generate super texture from above existing texture
			terrain_generatesupertexture ( false );
		}
	}
}

void terrain_delete ( void )
{
	if (  t.terrain.TerrainID>0 ) 
	{
		BT_DeleteTerrain (  t.terrain.TerrainID );
		t.terrain.TerrainID=0;
	}
	if (  t.terrain.terrainshaderindex>0 ) 
	{
		if (  GetEffectExist(t.terrain.terrainshaderindex) == 1  )  DeleteEffect (  t.terrain.terrainshaderindex );
	}
}

void terrain_shadowupdate ( void )
{
	//  Shadow Mapping ; Activate cascade shadow mapping for terrain
	if (  t.visuals.shadowmode>0 ) 
	{
		if ( 1 ) 
		{
			//  place shadowlight
			t.x_f=0 ; t.y_f=1000 ; t.z_f=0;
			PositionLight (  0,t.x_f+5000,t.y_f+10000,t.z_f+5000 );
			PointLight (  0,t.x_f,t.y_f,t.z_f );

			//  hide any jetpack
			//  NOTE; This should be MASK based, not hidden!
			if (  t.playercontrol.jetobjtouse>0 ) 
			{
				if (  ObjectExist(t.playercontrol.jetobjtouse) == 1 ) 
				{
					t.tjet1v=GetVisible(t.playercontrol.jetobjtouse);
					HideObject (  t.playercontrol.jetobjtouse );
				}
			}

			//  hide skybox before rendering cascade
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

			//  hide editor water
			if (  ObjectExist(t.terrain.objectstartindex+2) == 1 ) 
			{
				t.twaterobj=GetVisible(t.terrain.objectstartindex+2);
				HideObject (  t.terrain.objectstartindex+2 );
			}

			//  hide superflat terrain
			if (  t.terrain.TerrainID == 0 ) 
			{
				if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
				{
					HideObject (  t.terrain.terrainobjectindex );
				}
			}

			//  hide all vegetation
			if (  t.hardwareinfoglobals.nograss == 0 ) 
			{
				HideVegetationGrid (  );
			}

			//  shadows for map editor and game
			if ( 1 ) 
			{
				//  prepare scene shaders to render into the cascade shadow textures
				for ( t.t = -4 ; t.t<=  g.effectbankmax; t.t++ )
				{
					if ( t.t == -4  )  t.effectid = g.lightmappbreffect;
					if ( t.t == -3  )  t.effectid = g.thirdpersonentityeffect;
					if ( t.t == -2  )  t.effectid = g.thirdpersoncharactereffect;
					if ( t.t == -1  )  t.effectid = g.staticlightmapeffectoffset;
					if ( t.t == 0  )  t.effectid = g.staticshadowlightmapeffectoffset;
					if ( t.t>0  )  t.effectid = g.effectbankoffset+t.t;
					if ( t.effectid>0 ) 
					{
						if ( GetEffectExist(t.effectid) == 1 )  SetEffectTechnique ( t.effectid,"DepthMap" );
						//PE: This produce non transparent shadows on PBR objects in mode g.gpbroverride == 0
						//if ( g.gpbroverride == 0 )
						//{
						//	if ( t.visuals.shaderlevels.terrain >= 3 && t.visuals.shaderlevels.lighting != 1 ) 
						//	{
						//		// if this special technique does not exist in shaders, no new technique is used
						//		if ( GetEffectExist(t.effectid) == 1 ) SetEffectTechnique ( t.effectid,"DepthMapNoAnim" );
						//	}
						//}
					}
				}

				//  set static entity shader to depthmap technique also
				if (  GetEffectExist(g.staticlightmapeffectoffset) == 1  )  SetEffectTechnique (  g.staticlightmapeffectoffset,"DepthMap" );

				//  control how many cascade shadows to render based on shader levels for terrain shader
				//t.tonlyusingcheapestcascade=0;
				t.game.set.shaderrequirecheapshadow=0;
				SetEffectShadowMappingMode ( 0 );

				//PE: If we can set m_fCascadeFrustumsEyeSpaceDepths here (0) if not used,
				//PE: then all shaders can use GetShadow highest/medium and we can control what cascades is available here.
				//PE: current: old terrain medium/low = cascade 7 , veg high = cascade 3 , nonpbrentity medium = none , pbr entity medium = all.

				if ( 1 ) // g.gpbroverride == 1
				{
					// PBR default mode is to render all shadows in editor mode

					//PE: Set frustum percent so we can fake a medium.

					g_CascadedShadow.m_iCascadePartitionsZeroToOne[0] = g.globals.realshadowcascade[0];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[1] = g.globals.realshadowcascade[1];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[2] = g.globals.realshadowcascade[2];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[3] = g.globals.realshadowcascade[3];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[4] = g.globals.realshadowcascade[4];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[5] = g.globals.realshadowcascade[5];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[6] = g.globals.realshadowcascade[6];
					g_CascadedShadow.m_iCascadePartitionsZeroToOne[7] = g.globals.realshadowcascade[7];
					g.globals.realshadowdistance = g.globals.realshadowdistancehigh;
					SetShadowTexelSize(g.globals.realshadowresolution);
					SetEffectShadowMappingMode ( 255 );

					//PE: PBR both terrain and entity must be set to low/medium before lowering cascades.
					//PE: Editor always use highest.
					if (t.game.set.ismapeditormode != 1 && t.visuals.shaderlevels.terrain >= 4 && t.visuals.shaderlevels.entities >= 3) 
					{
						//PE: Lowest disable shadows.
						SetEffectShadowMappingMode(0);
					}
					else if ( (g.globals.editorusemediumshadows == 1 && t.game.set.ismapeditormode == 1 ) || (t.game.set.ismapeditormode != 1 && t.visuals.shaderlevels.terrain >= 2 && t.visuals.shaderlevels.entities >= 2 ) ) 
					{
						for (int icl = 0; icl < g.globals.realshadowcascadecount; icl++)
							g_CascadedShadow.m_iCascadePartitionsZeroToOne[icl] = 0;

						SetEffectShadowMappingMode( (1<< g.globals.realshadowcascadecount-1) + (1<< (g.globals.realshadowcascadecount-2)) );
						g.globals.realshadowdistance = g.globals.realshadowdistancehigh*0.65; //PE: Lower distance by 35%.
						SetShadowTexelSize(2048); // Needed when we use so low a resolution.
						g_CascadedShadow.m_iCascadePartitionsZeroToOne[g.globals.realshadowcascadecount - 2] = 24;
						g_CascadedShadow.m_iCascadePartitionsZeroToOne[g.globals.realshadowcascadecount - 1] = 100;
					}
				}
				else
				{
					if (  t.visuals.shaderlevels.lighting == 1 && t.game.set.ismapeditormode == 0 ) 
					{
						//  PREBAKE requires second cascade for near-shadows (dynamic objects in test/game)
						if (  t.visuals.shaderlevels.entities == 1 ) 
						{
							SetEffectShadowMappingMode ( 255 ); //%11111111 //15 ); //%1111
						}
						else
						{
							//  uses cascade 2 and 4 (near and far)
							SetEffectShadowMappingMode ( 10 ); //%1010
						}
					}
					else
					{
						if (  t.visuals.shaderlevels.terrain <= 2 || t.visuals.shaderlevels.entities == 1 ) 
						{
							//  for HIGH/HIGHEST shaders that require all FOUR shadow cascades
							SetEffectShadowMappingMode ( 255 ); // %11111111 15 ); //%1111
						}
						else
						{
							//  for LOW/LOWEST only DISTANT CASCADE is used
							//SetEffectShadowMappingMode ( 8 ); //%00001000 //%1000
							SetEffectShadowMappingMode ( 128 ); //%10000000 (changed terrain_basic to use cascade 7) //120418 - but keep cascade 4 for PBR shaders
							/* complete remove old dynamic cheap shadow trick
							if (  t.game.set.ismapeditormode == 1 ) 
							{
								//  if in LOWEST MAP, constant update
								t.tonlyusingcheapestcascade=0;
							}
							else
							{
								//  if in LOWEST TEST/GAME, intermittent update
								t.tonlyusingcheapestcascade=1;
							}
							*/
						}
					}
					/* complete remove old dynamic cheap shadow trick
					if ( t.visuals.shaderlevels.terrain>2 ) 
					{
						// for LOWEST, only render lowest cascade every X second(s)
						t.game.set.shaderrequirecheapshadow=1;
						if ( t.gdynamicterrainshadowcameragenerate == 0 ) 
						{
							if (  t.game.set.ismapeditormode == 1 ) 
							{
								//  when in editor - full speed shadow updates
								t.gdynamicterrainshadowcameratrigger=1;
							}
							else
							{
								//  when in game - real-time spread of shadow updates
								if (  Timer()>t.gdynamicterrainshadowcameratimer ) 
								{
									t.gdynamicterrainshadowcameratimer=Timer()+20;
									t.gdynamicterrainshadowcameratrigger=1;
								}
							}
						}
					}
					*/
				}

				//  render primary cascade (terrainshaderindex)
				if (  t.terrain.terrainshaderindex>0 ) 
				{
					/* completely removed old dynamic cheap shadows
					//  expensive to render shadow objects from high orbit, so only do for new view
					//  or if something moved this cycle catch it with Timer (  based refresh )
					g.cheapshadowhistorypacer_f=g.cheapshadowhistorypacer_f+g.timeelapsed_f;
					if (  g.cheapshadowhistorypacer_f>1.0 ) 
					{
						g.cheapshadowhistorypacer_f=0.0;
						t.tonlyusingcheapestcascade=0;
					}
					if (  t.tonlyusingcheapestcascade == 1 ) 
					{
						//  perform an update to the furthest shadow cascade (only for areas not done)
						if (  ArrayCount(t.cheapshadowhistory) <= 0 ) 
						{
							Dim2(  t.cheapshadowhistory,512, 512 );
						}
						t.tcx=CameraPositionX(0)/100.0;
						if (  t.tcx<0  )  t.tcx = 0;
						if (  t.tcx>512  )  t.tcx = 512;
						t.tcz=CameraPositionZ(0)/100.0;
						if (  t.tcz<0  )  t.tcz = 0;
						if (  t.tcz>512  )  t.tcz = 512;
						t.tca=WrapValue(CameraAngleY(0))/90.0;
						if (  t.tca<0  )  t.tc_f = 0;
						if (  t.tca>3  )  t.tc_f = 3;
						t.tbitwant=1<<t.tca;
						t.tbitshave=t.cheapshadowhistory[t.tcx][t.tcz];
						if (  (t.tbitshave && t.tbitwant) == 0 ) 
						{
							//  primary shader renders actual cascade shadow RTs (only if not encountered b4)
							RenderEffectShadowMapping (  t.terrain.terrainshaderindex );
							t.cheapshadowhistory[t.tcx][t.tcz]=t.tbitshave || t.tbitwant;
						}
					}
					else
					*/
					//{
					// primary shader renders actual cascade shadow RTs
					RenderEffectShadowMapping ( t.terrain.terrainshaderindex );
					//}
				}

				//  restore all scene shaders after render to cascade shadow textures
				visuals_shaderlevels_entities_update ( );

				// give shaders that use shadows the latest-realtime values
				if ( t.terrain.vegetationshaderindex>0 ) 
				{
					// veg - secondary shader simply conveys required shadow constants to shader
					if ( GetEffectExist(t.terrain.vegetationshaderindex) == 1 ) 
					{
						RenderEffectShadowMapping ( t.terrain.vegetationshaderindex );
					}
				}
				if (  t.gunid>0 && t.gun[t.gunid].effectidused>0 ) 
				{
					//  gun - secondary shader simply conveys required shadow constants to shader
					RenderEffectShadowMapping (  t.gun[t.gunid].effectidused );
				}
				for ( t.t = -4 ; t.t<=  g.effectbankmax; t.t++ )
				{
					if (  t.t == -4  )  t.effectid = g.lightmappbreffect;
					if (  t.t == -3  )  t.effectid = g.thirdpersonentityeffect;
					if (  t.t == -2  )  t.effectid = g.thirdpersoncharactereffect;
					if (  t.t == -1  )  t.effectid = g.staticlightmapeffectoffset;
					if (  t.t == 0  )  t.effectid = g.staticshadowlightmapeffectoffset;
					if (  t.t>0  )  t.effectid = g.effectbankoffset+t.t;
					if (  GetEffectExist(t.effectid) == 1 ) 
					{
						//  entities - secondary shader simply conveys required shadow constants to shader
						RenderEffectShadowMapping (  t.effectid );
					}
				}

				/* completely remove old dynamic cheap shadow
				//  if dynamic shadows required with pre-bake, send shadow info to glass terrain shader
				//  calculate and send shadow constants to dynamic terrain shadow shader
				if (  t.gdynamicterrainshadowcameraid>0 ) 
				{
					//  cheapshadow - secondary shader simply conveys requ ired shadow constants to shader
					RenderEffectShadowMapping (  g.postprocesseffectoffset+5 );
				}
				*/
			}

			//  show all vegetation, but only if in game (we don't show veg in grid edit mode)
			if (  t.hardwareinfoglobals.nograss == 0 ) 
			{
				if (  t.game.gameloop  !=  0  )  ShowVegetationGrid (  );
			}

			//  show water in editor
			if (  ObjectExist(t.terrain.objectstartindex+2) == 1 && t.twaterobj == 1  )  ShowObject (  t.terrain.objectstartindex+2 );

			//  show superflat terrain
			if (  t.terrain.TerrainID == 0 ) 
			{
				if (  ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
				{
					ShowObject (  t.terrain.terrainobjectindex );
				}
			}

			//  show sky Box (  after rendering shadow cascades )
			if (  t.hardwareinfoglobals.nosky == 0 ) 
			{
				if (  ObjectExist(t.terrain.objectstartindex+4) == 1 && t.tskyobj1v == 1  )  ShowObject (  t.terrain.objectstartindex+4 );
				if (  ObjectExist(t.terrain.objectstartindex+8) == 1 && t.tskyobj2v == 1  )  ShowObject (  t.terrain.objectstartindex+8 );
				if (  ObjectExist(t.terrain.objectstartindex+9) == 1 && t.tskyobj3v == 1  )  ShowObject (  t.terrain.objectstartindex+9 );
			}

			//  show any jetpack
			if (  t.playercontrol.jetobjtouse>0 ) 
			{
				if (  ObjectExist(t.playercontrol.jetobjtouse) == 1 && t.tskyobj1v == 1  )  ShowObject (  t.playercontrol.jetobjtouse );
			}

			// shadow debug when required
			if ( t.visuals.debugvisualsmode == 1 ) 
			{
				// show debug shadow map (first cascade)
				if ( g.shadowdebugobjectoffset+0 > 0 )
				{
					if ( ObjectExist ( g.shadowdebugobjectoffset+0 ) == 1 )
					{
						if ( ReturnKey() == 1 ) 
							ShowObject ( g.shadowdebugobjectoffset+0 );
						else
							HideObject ( g.shadowdebugobjectoffset+0 );
					}
				}
			}
		}
	}

	//  occlusion debug when required (put near shadow debug so can find easier)
	if ( t.visuals.debugvisualsmode == 2 ) 
	{
		if (  ReturnKey() == 1 ) 
		{
			CPU3DShow (  1 );
		}
		else
		{
			CPU3DShow (  0 );
		}
	}

	//  view of dynamic terrain shadow texture
	if ( t.visuals.debugvisualsmode == 3 ) 
	{
		if (  ImageExist(g.postprocessimageoffset+5) == 1 ) 
		{
			if (  ReturnKey() == 1 ) 
			{
				t.timg=g.postprocessimageoffset+5;
				Sprite (  g.postprocessimageoffset+5,-10000,-10000,t.timg );
				PasteSprite (  g.postprocessimageoffset+5,(GetDisplayWidth()-ImageWidth(t.timg))/2,(GetDisplayHeight()-ImageHeight(t.timg))/2 );
			}
			else
			{
				if (  SpriteExist(g.postprocessimageoffset+5) == 1  )  DeleteSprite (  g.postprocessimageoffset+5 );
			}
		}
	}

	/* completely removed old dynamic cheap shadow
	// Render dynamic terrain shadow camera
	if ( t.gdynamicterrainshadowcameraid>0 ) 
	{
		//  only if quad for rendering cheap shadow is visible (prebake can hide it)
		if (  ObjectExist(g.postprocessobjectoffset+5) == 1 ) 
		{
			//  Clear the dynamic terrain shadow camera
			if (  t.gdynamicterrainshadowcameragenerate>0 ) 
			{
				--t.gdynamicterrainshadowcameragenerate;
				if (  t.gdynamicterrainshadowcameragenerate == 1 ) 
				{
					SyncMask (  1<<t.gdynamicterrainshadowcameraid );
					BackdropOn (  t.gdynamicterrainshadowcameraid );
					BackdropColor (  t.gdynamicterrainshadowcameraid,Rgb(0,0,0) );
					FastSync (  );
					if (  t.game.set.ismapeditormode == 1 ) 
					{
						SyncMask (  1 );
					}
					else
					{
						SyncMask (  0xfffffff9 );
					}
					BackdropOff (  t.gdynamicterrainshadowcameraid );
				}
			}
			else
			{
				//  instead, we do a single snapshot when triggered
				if (  t.gdynamicterrainshadowcameratrigger == 1 ) 
				{
					t.gdynamicterrainshadowcameratrigger=0;
					SyncMask (  1<<t.gdynamicterrainshadowcameraid );
					FastSync (  );
					if (  t.game.set.ismapeditormode == 1 ) 
					{
						SyncMask (  1 );
					}
					else
					{
						SyncMask (  0xfffffff9 );
					}
				}
			}
		}
	}
	*/

	/* completely removed old dynamic cheap shadow, so don't need this refresh
	//  Generate heightmap texture for cheap shadows (triggered by loading new level)
	if (  t.terrain.terraintriggercheapshadowrefresh>0 ) 
	{
		t.terrain.terraintriggercheapshadowrefresh=t.terrain.terraintriggercheapshadowrefresh-1;
		if (  t.terrain.terraintriggercheapshadowrefresh == 0 ) 
		{
			t.terrain.terrainquickupdate=1;
			t.terrain.terrainquickx1=0;
			t.terrain.terrainquickx2=1024;
			t.terrain.terrainquickz1=0;
			t.terrain.terrainquickz2=1024;
			terrain_quickupdateheightmapfromheightdata ( );
		}
	}
	*/
}

void terrain_updaterealheights ( void )
{
	if (  t.terrain.TerrainID>0 ) 
	{
		//  sculp terrain from existing height data
		for ( t.z = 1 ; t.z<=  1023; t.z++ )
		{
			for ( t.x = 1 ; t.x<=  1023; t.x++ )
			{
				t.h_f=t.terrainmatrix[t.x][t.z];
				if (  t.h_f<0  )  t.h_f = 0;
				BT_SetPointHeight (  t.terrain.TerrainID,t.x,t.z,t.h_f );
			}
		}

		//  cap edges
		for ( t.z = 1 ; t.z<=  1024; t.z++ )
		{
			BT_SetPointHeight (  t.terrain.TerrainID,0,t.z,0 );
			BT_SetPointHeight (  t.terrain.TerrainID,1024,t.z,0 );
		}
		for ( t.x = 1 ; t.x<=  1024; t.x++ )
		{
			BT_SetPointHeight (  t.terrain.TerrainID,t.x,0,0 );
			BT_SetPointHeight (  t.terrain.TerrainID,t.x,1024,0 );
		}

		// after amending terrain, update height map
		BT_SetCurrentCamera (  0 );
		BT_UpdateTerrainCull (  t.terrain.TerrainID );
		BT_UpdateTerrainLOD (  t.terrain.TerrainID );
		BT_RenderTerrain (  t.terrain.TerrainID );

		//  update terrain internals and empty queue (was SyncOn (  260115) )
		BT_Intern_Render( );
	}
}

void terrain_randomiseorflattenterrain ( void )
{
	// 100417 - dont use water line for terrain generation
	float fTerrainDefaultHeight = g.gdefaultterrainheight;
	if (  t.terrainflattenmode == 1 ) 
	{
		//  flat
		for ( t.z = 0 ; t.z<=  1023; t.z++ )
		{
			for ( t.x = 0 ; t.x<=  1023; t.x++ )
			{
				t.h_f = fTerrainDefaultHeight;
				t.terrainmatrix[t.x][t.z]=t.h_f;
			}
		}
	}
	else
	{
		// first reset
		for ( t.z = 0 ; t.z<=  1023; t.z++ )
			for ( t.x = 0 ; t.x<=  1023; t.x++ )
				t.terrainmatrix[t.x][t.z] = 100;

		//  random - generate seed heights
		t.terrain.terrain_seed = Timer();
		t.terrain.terrain_range = 5 + Rnd(4);
		generate_terrain(t.terrain.terrain_seed,t.terrain.terrain_range,terrain_chunk_size);

		//  ensure terrain is above the waterline (and high enough to avoid Z clip at max editor zoomout)
		for ( t.z = 1 ; t.z<=  1023; t.z++ )
		{
			for ( t.x = 1 ; t.x<=  1023; t.x++ )
			{
				t.h_f=t.terrainmatrix[t.x][t.z]*2.5f;
				if (  t.h_f<0  )  t.h_f = 0;
				t.terrainmatrix[t.x][t.z] = fTerrainDefaultHeight + t.h_f;
			}
		}
	}

	//  sculp terrain from existing height data
	terrain_updaterealheights ( );
}

void terrain_flattenterrain ( void )
{
	t.terrainflattenmode=1;
	terrain_randomiseorflattenterrain ( );
}

void terrain_randomiseterrain ( void )
{
	t.terrainflattenmode=0;
	terrain_randomiseorflattenterrain ( );
}

void terrain_refreshterrainmatrix ( void )
{
	if (  t.terrain.TerrainID>0 ) 
	{
		if (  t.terrain.terrainregionupdate == 1 ) 
		{
			t.xs1=t.terrain.terrainregionx1;
			t.xs2=t.terrain.terrainregionx2;
			t.zs1=t.terrain.terrainregionz1;
			t.zs2=t.terrain.terrainregionz2;
		}
		else
		{
			t.xs1=0;
			t.xs2=1024;
			t.zs1=0;
			t.zs2=1024;
		}
		for ( t.z = t.zs1 ; t.z<=  t.zs2; t.z++ )
		{
			for ( t.x = t.xs1 ; t.x<=  t.xs2; t.x++ )
			{
				t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,t.x*50.0,t.z*50.0,1);
				t.terrainmatrix[t.x][t.z]=t.h_f;
			}
		}
	}
}

void terrain_skipifnowaterexposed ( void )
{
	// returns tokay=1 if we should skip
	t.xs1=2; t.xs2=1024-2; t.zs1=2; t.zs2=1024-2;
	for ( t.z = t.zs1 ; t.z <= t.zs2; t.z++ )
	{
		for ( t.x = t.xs1 ; t.x <= t.xs2; t.x++ )
		{
			if ( t.terrainmatrix[t.x][t.z] < t.terrain.waterliney_f ) 
			{
				// below water - we cannot skip
				t.tokay=0; 
				return;
			}
		}
	}

	//  no water found, we can skip
	t.tokay=1;
}

void terrain_updatewatermask ( void )
{
	//  takes tfilewater$
	//  if VIDMEM reset, must leave silently

	if (g.memskipwatermask == 1)  return; //PE: if watermask not used.

	if (  BitmapExist(g.terrainworkbitmapindex) == 0  )  return;
	//  if no water exposed, we can skip this
	terrain_skipifnowaterexposed() ; if (  t.tokay == 1  )  return;
	//  terrain.terrainregionupdate ; 0-full update, 1-local region update, 2-local but save new file
	if (  ImageExist(t.terrain.imagestartindex+4) == 0 ) 
	{
		if (  FileExist(t.tfilewater_s.Get()) == 1 ) 
		{
			LoadImage (  t.tfilewater_s.Get(),t.terrain.imagestartindex+4,10,0 );
		}
	}
	if (  t.game.gameisexe == 0 ) 
	{
		//  only create new water mask and save if not standalone
		if (  t.terrain.terrainregionupdate>0 ) 
		{
			t.xs1=t.terrain.terrainregionx1*MAXTEXTURESIZEMULTIPLIER;
			t.xs2=(t.terrain.terrainregionx2*MAXTEXTURESIZEMULTIPLIER)-1;
			t.zs1=t.terrain.terrainregionz1*MAXTEXTURESIZEMULTIPLIER;
			t.zs2=(t.terrain.terrainregionz2*MAXTEXTURESIZEMULTIPLIER)-1;
		}
		else
		{
			t.xs1=0;
			t.xs2=MAXTEXTURESIZE-1;
			t.zs1=0;
			t.zs2=MAXTEXTURESIZE-1;
		}
		SetCurrentBitmap (  g.terrainworkbitmapindex );
		if (  ImageExist(t.terrain.imagestartindex+4) == 1 ) 
		{
			PasteImage (  t.terrain.imagestartindex+4,0,0 );
		}
		LockPixels (  );
		for ( t.z = t.zs1 ; t.z<=  t.zs2; t.z++ )
		{
			for ( t.x = t.xs1 ; t.x<=  t.xs2; t.x++ )
			{
				t.nx_f=t.x;
				t.nz_f=t.z;
				t.nx=t.nx_f/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nz=t.nz_f/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nxs=t.nx;
				t.nzs=t.nz;
				t.nxe = t.nx+1 ; if (  t.nxe>1023  )  t.nxe = 1023;
				t.nze = t.nz+1 ; if (  t.nze>1023  )  t.nze = 1023;
				t.nxb_f=(t.nx_f-(t.nx*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nzb_f=(t.nz_f-(t.nz*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.h1_f=t.terrainmatrix[t.nxs][t.nzs];
				t.h2_f=t.terrainmatrix[t.nxe][t.nzs];
				t.h3_f=t.terrainmatrix[t.nxs][t.nze];
				t.h4_f=t.terrainmatrix[t.nxe][t.nze];
				t.ha_f=t.h1_f+((t.h2_f-t.h1_f)*t.nxb_f);
				t.hb_f=t.h3_f+((t.h4_f-t.h3_f)*t.nxb_f);
				t.h_f=t.ha_f+((t.hb_f-t.ha_f)*t.nzb_f);
				if (  t.h_f<0  )  t.h_f = 0;
				if (  t.h_f >= t.terrain.waterliney_f ) 
				{
					//  above water
					t.a=0;
				}
				else
				{
					//  below/in water
					t.a=(t.terrain.waterliney_f-t.h_f)*10.0;
					if (  t.a>255  )  t.a = 255;
				}
				//  ripple effect
				t.nxs = t.nx-1 ; if (  t.nxs<0  )  t.nxs = 0;
				t.nzs = t.nz-1 ; if (  t.nzs<0  )  t.nzs = 0;
				t.nxe = t.nx+1 ; if (  t.nxe>1023  )  t.nxe = 1023;
				t.nze = t.nz+1 ; if (  t.nze>1023  )  t.nze = 1023;
				t.nxb_f=(t.nx_f-(t.nx*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nzb_f=(t.nz_f-(t.nz*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.h1_f=t.terrainmatrix[t.nxs][t.nzs];
				t.h2_f=t.terrainmatrix[t.nxe][t.nzs];
				t.h3_f=t.terrainmatrix[t.nxs][t.nze];
				t.h4_f=t.terrainmatrix[t.nxe][t.nze];
				t.b=t.a;
				if (  t.h1_f >= t.terrain.waterliney_f || t.h2_f>= t.terrain.waterliney_f || t.h3_f >= t.terrain.waterliney_f || t.h4_f>= t.terrain.waterliney_f ) 
				{
					t.b=0;
				}
				Dot (  t.x,t.z,Rgb(t.a,t.b,t.a) );
				++t.tdotcount;
			}
		}
		UnlockPixels (  );
		if (  ImageExist(t.terrain.imagestartindex+4)  ==  1  )  DeleteImage (  t.terrain.imagestartindex+4 );
		GrabImage (  t.terrain.imagestartindex+4,0,0,MAXTEXTURESIZE,MAXTEXTURESIZE );
		if (  ImageExist(t.terrain.imagestartindex+4)  ==  1 ) 
		{
			if (  t.terrain.terrainregionupdate == 0 || t.terrain.terrainregionupdate == 2 || FileExist(t.tfilewater_s.Get()) == 0 ) 
			{
				if (  FileExist(t.tfilewater_s.Get()) == 1  )  DeleteAFile (  t.tfilewater_s.Get() );
				SaveImage (  t.tfilewater_s.Get(),t.terrain.imagestartindex+4,5 );
			}
		}
		SetCurrentBitmap (  0 );
	}
}

void terrain_updatewatermask_new ( void )
{
//	if (g.memskipwatermask == 1)  return; //PE: if watermask not used.

	SetCurrentBitmap ( g.terrainworkbitmapindex );
	CLS ( 0 );
	GrabImage ( t.terrain.imagestartindex+4, 0, 0, 1, 1 );
	if ( FileExist(t.tfilewater_s.Get()) == 1  )  DeleteAFile (  t.tfilewater_s.Get() );
	SaveImage ( t.tfilewater_s.Get(),t.terrain.imagestartindex+4 );
	SetCurrentBitmap (  0 );
}

void terrain_whitewashwatermask ( void )
{
	SetCurrentBitmap ( g.terrainworkbitmapindex );
	LockPixels ( );
	Dot ( 0,0,Rgb(255,255,255) );
	UnlockPixels ( );
	GrabImage ( t.terrain.imagestartindex+4,0,0,1,1 );
	SetCurrentBitmap ( 0 );
	if ( ObjectExist(t.terrain.objectstartindex+5) == 1 ) 
	{
		TextureObject ( t.terrain.objectstartindex+5,3,t.terrain.imagestartindex+4 );
	}
}

void terrain_createheightmapfromheightdata ( void )
{
	// take current terrain heights and make a height map image
	SetCurrentBitmap ( g.terrainworkbitmapindex );
	LockPixels ( );
	for ( t.z = 0 ; t.z<=  1024-1; t.z++ )
	{
		for ( t.x = 0 ; t.x<=  1024-1; t.x++ )
		{
			t.h_f=0.0+(t.terrainmatrix[t.x][t.z]*100.0);
			if ( t.h_f<0  )  t.h_f = 0.0;
			t.tcolr=t.h_f/65536.0;
			t.tcolg=(t.h_f-(t.tcolr*65536.0))/256.0;
			t.tcolb=t.h_f-(t.tcolr*65536.0)-(t.tcolg*256.0);
			Dot ( t.x,t.z,Rgb(t.tcolr,t.tcolg,t.tcolb) );
		}
	}
	UnlockPixels ( );

	//  re-used this for dynamic terrain shadow texture generation (need terrain heights for shader)
	if ( ImageExist(g.postprocessimageoffset+6) == 1 ) DeleteImage ( g.postprocessimageoffset+6 );
	GrabImage ( g.postprocessimageoffset+6,0,0,1024,1024 );
	SetCurrentBitmap ( 0 );
}

void terrain_quickupdateheightmapfromheightdata ( void )
{
	//  work out if we update ALL or just a region
	t.xs1=0 ; t.xs2=1024 ; t.zs1=0 ; t.zs2=1024;
	if (  t.terrain.terrainquickupdate == 1 ) 
	{
		t.terrain.terrainquickupdate=0;
		t.xs1=t.terrain.terrainquickx1;
		t.xs2=t.terrain.terrainquickx2;
		t.zs1=t.terrain.terrainquickz1;
		t.zs2=t.terrain.terrainquickz2;
	}
	//  get latest heights from terrain
	if (  t.terrain.TerrainID>0 ) 
	{
		for ( t.z = t.zs1 ; t.z<=  t.zs2; t.z++ )
		{
			for ( t.x = t.xs1 ; t.x<=  t.xs2; t.x++ )
			{
				t.h_f=BT_GetGroundHeight(t.terrain.TerrainID,t.x*50.0,t.z*50.0,1);
				t.terrainmatrix[t.x][t.z]=t.h_f;
			}
		}
	}
	else
	{
		for ( t.z = t.zs1 ; t.z<=  t.zs2; t.z++ )
		{
			for ( t.x = t.xs1 ; t.x<=  t.xs2; t.x++ )
			{
				t.h_f=1000.0 ; t.terrainmatrix[t.x][t.z]=t.h_f;
			}
		}
	}
	//  update heightmap texture
	SetCurrentBitmap (  g.terrainworkbitmapindex );
	if (  ImageExist(g.postprocessimageoffset+6) == 1  )  PasteImage (  g.postprocessimageoffset+6,0,0 );
	LockPixels (  );
	for ( t.z = t.zs1 ; t.z<=  t.zs2-1; t.z++ )
	{
		for ( t.x = t.xs1 ; t.x<=  t.xs2-1; t.x++ )
		{
			t.h_f=0.0+(t.terrainmatrix[t.x][t.z]*100.0);
			if (  t.h_f<0  )  t.h_f = 0.0;
			t.tcolr=t.h_f/65536.0;
			t.tcolg=(t.h_f-(t.tcolr*65536.0))/256.0;
			t.tcolb=t.h_f-(t.tcolr*65536.0)-(t.tcolg*256.0);
			Dot (  t.x,t.z,Rgb(t.tcolr,t.tcolg,t.tcolb) );
		}
	}
	UnlockPixels (  );
	if (  ImageExist(g.postprocessimageoffset+6) == 1  )  DeleteImage (  g.postprocessimageoffset+6 );
	GrabImage (  g.postprocessimageoffset+6,0,0,1024,1024 );
	SetCurrentBitmap (  0 );
	//  update all that use this texture
	postprocess_applycheapshadow ( );
}

void terrain_generatetextureselect ( void )
{
	SetCurrentBitmap (  g.terrainworkbitmapindex );
	LockPixels (  );
	for ( t.z = 0 ; t.z<=  MAXTEXTURESIZE; t.z++ )
	{
		for ( t.x = 0 ; t.x<=  MAXTEXTURESIZE; t.x++ )
		{
			t.h_f=t.terrainmatrix[t.x/MAXTEXTURESIZEMULTIPLIER][t.z/MAXTEXTURESIZEMULTIPLIER];
			if (  t.h_f<0  )  t.h_f = 0;
			t.texselect=Rgb(0,0,0);
			t.grasscoverage=Rgb(0,0,0);
			if (  t.h_f >= t.terrain.waterliney_f-100.0 && t.h_f <= t.terrain.waterliney_f+50.0 ) 
			{
				//  shore
				t.by_f=(t.h_f-(t.terrain.waterliney_f-100.0))/150.0;
				t.texselect=Rgb(0,0,196.0-(t.by_f*128.0));
			}
			else
			{
				if (  t.h_f<t.terrain.waterliney_f-50.0 ) 
				{
					//  underwater
					t.texselect=Rgb(0,0,192);
				}
				else
				{
					if (  t.h_f>t.terrain.waterliney_f+50.0 ) 
					{
						//  inland
						t.texselect=Rgb(0,0,64);
						t.thresh_f=750.0;
						if (  t.h_f>t.terrain.waterliney_f+t.thresh_f ) 
						{
							//  too high for grass
							t.by_f=(t.h_f-(t.terrain.waterliney_f+t.thresh_f))/500.0;
							t.cmax = 64.0+(t.by_f*64.0) ; if (  t.cmax>128+32  )  t.cmax = 128+32;
							t.texselect=Rgb(0,0,t.cmax);
							t.by_f=(t.h_f-(t.terrain.waterliney_f+t.thresh_f))/200.0;
							t.cmax=255-(t.by_f*255.0);
							if (  t.cmax<0  )  t.cmax = 0;
							if (  t.cmax>255  )  t.cmax = 255;
							t.grasscoverage=Rgb(t.cmax,0,0);
						}
						else
						{
							//  grass in sweet spot
							t.by_f=(t.h_f-(t.terrain.waterliney_f+100.0))/200.0;
							t.cmax=(t.by_f*255.0);
							if (  t.cmax<0  )  t.cmax = 0;
							if (  t.cmax>255  )  t.cmax = 255;
							t.grasscoverage=Rgb(t.cmax,0,0);
						}
					}
				}
			}
			Dot (  t.x,t.z,t.texselect+t.grasscoverage );
		}
	}
	UnlockPixels (  );
	GrabImage (  t.terrain.imagestartindex+2,0,0,MAXTEXTURESIZE,MAXTEXTURESIZE );
	if (  MemblockExist(123) == 1  )  DeleteMemblock (  123 );
	CreateMemblockFromImage (  123,t.terrain.imagestartindex+2 );
	TextureObject (  t.terrain.terrainobjectindex,0,t.terrain.imagestartindex+2 );
	SetCurrentBitmap (  0 );
}

void terrain_deletesupertexturepalette ( void )
{
	if ( g.terrainstyleindex == 1 )
	{
		// custom from levelbank\testmap
		t.tfile_s = g.mysystem.levelBankTestMap_s+"superpalette.ter"; //cstr("levelbank\\testmap\\superpalette.ter");
	}
	else
	{
		// regular from terrainbank
		g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
		t.tfile_s = cstr("terrainbank\\")+g.terrainstyle_s+"\\superpalette.ter";
	}
	if ( FileExist(t.tfile_s.Get()) == 1 ) DeleteFile ( t.tfile_s.Get() );
}

void terrain_generatesupertexture ( bool bForceRecalcOfPalette )
{
	// In superflat mode 2, we don't have a terrain
	if ( t.terrain.superflat == 2 )  return;

	// determine location for terrain texture palette
	if ( g.terrainstyleindex == 1 )
	{
		// custom from levelbank\testmap
		t.tfile_s = g.mysystem.levelBankTestMap_s+"superpalette.ter"; //cstr("levelbank\\testmap\\superpalette.ter");
	}
	else
	{
		// regular from terrainbank
		g.terrainstyle_s = t.terrainstylebank_s[g.terrainstyleindex];
		t.tfile_s = cstr("terrainbank\\")+g.terrainstyle_s+"\\superpalette.ter";
	}

	// First obtain paint palette from terrainbank
	if ( FileExist(t.tfile_s.Get()) == 1 ) 
	{
		// eraese older versions of file
		OpenToRead ( 7, t.tfile_s.Get() );
		t.tversion = ReadLong ( 7 );
		CloseFile ( 7 );
		if ( t.tversion == 100 ) 
			DeleteFile ( t.tfile_s.Get() );
	}
	if ( FileExist(t.tfile_s.Get()) == 1 && bForceRecalcOfPalette == false ) 
	{
		// Load pre-calculated data into arrays
		OpenToRead (  7,t.tfile_s.Get() );
		t.tversion = ReadLong ( 7 );
		if ( t.tversion == 100 )
		{
			// version 100
			for ( t.p = 1 ; t.p <= 5; t.p++ )
			{
				for ( t.z = 0 ; t.z <= 3; t.z++ )
				{
					for ( t.x = 0 ; t.x <= 3; t.x++ )
					{
						t.a = ReadLong ( 7 ); 
						t.pot[t.p][t.x][t.z]=t.a;
					}
				}
			}
		}
		if ( t.tversion == 103 )
		{
			// version 103 - March 2017
			for ( t.p = 1 ; t.p <= 16; t.p++ )
			{
				for ( t.z = 0 ; t.z <= 3; t.z++ )
				{
					for ( t.x = 0 ; t.x <= 3; t.x++ )
					{
						t.a = ReadLong ( 7 ); 
						t.pot[t.p][t.x][t.z] = t.a;
					}
				}
			}
		}
		CloseFile (  7 );
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
	}
	else
	{
		// Delete old file first
		if ( FileExist(t.tfile_s.Get()) == 1 ) DeleteFile ( t.tfile_s.Get() );

		// Generate pallete data for super terrain and save as file
		SetCurrentBitmap ( g.terrainworkbitmapindex );
		for ( t.p = 1 ; t.p <= 16; t.p++ )
		{
			t.timg=t.terrain.imagestartindex+13;
			int pasterow = (t.p-1)/4;
			int pastecol = (t.p-1)-(pasterow*4);
			int pasteatx = pastecol*1024;
			int pasteaty = pasterow*1024;
			if ( GetImageExistEx ( t.timg ) ) PasteImage ( t.timg, -pasteatx, -pasteaty );
			LockPixels ( );
			t.timgw=1024/4;
			t.timgh=1024/4;
			for ( t.z = 0 ; t.z<=  3; t.z++ )
			{
				for ( t.x = 0 ; t.x<=  3; t.x++ )
				{
					//  find average of this quadrant of the paint texture
					t.tr_f=0 ; t.tg_f=0 ; t.tb_f=0 ; t.tt_f=0;
					for ( t.avz = t.z*t.timgh ; t.avz<=  ((t.z+1)*t.timgh)-1; t.avz++ )
					{
						for ( t.avx = t.x*t.timgw ; t.avx<=  ((t.x+1)*t.timgw)-1; t.avx++ )
						{
							t.trgba=GetPoint(t.avx,t.avz);
							t.tr_f=t.tr_f+RgbR(t.trgba);
							t.tg_f=t.tg_f+RgbG(t.trgba);
							t.tb_f=t.tb_f+RgbB(t.trgba);
							t.tt_f=t.tt_f+1;
						}
						if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
					}
					t.tr=t.tr_f/t.tt_f;
					t.tg=t.tg_f/t.tt_f;
					t.tb=t.tb_f/t.tt_f;
					if (  t.tr>255  )  t.tr = 255;
					if (  t.tg>255  )  t.tg = 255;
					if (  t.tb>255  )  t.tb = 255;
					t.pot[t.p][t.x][t.z]=(t.tr<<16)+(t.tg<<8)+(t.tb);
				}
				if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
			}
			UnlockPixels (  );
		}
		SetCurrentBitmap (  0 );

		//  Save pre-calculated data
		OpenToWrite ( 7, t.tfile_s.Get() );
		int iVersionNumber = 103;
		WriteLong ( 7, iVersionNumber );
		for ( t.p = 1 ; t.p <= 16; t.p++ )
		{
			for ( t.z = 0 ; t.z <= 3; t.z++ )
			{
				for ( t.x = 0 ; t.x <= 3; t.x++ )
				{
					t.a = t.pot[t.p][t.x][t.z]; 
					WriteLong ( 7, t.a );
				}
			}
		}
		CloseFile (  7 );
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
	}

	// New command to create the mega texture entirely inside the Image Module (special mode)
	if ( MemblockExist(9) == 1  ) DeleteMemblock ( 9 );
	MakeMemblock ( 9, (17*16)*4 );
	for ( t.p = 1 ; t.p <= 16; t.p++ )
	{
		for ( t.z = 0 ; t.z <= 3; t.z++ )
		{
			for ( t.x = 0 ; t.x <= 3; t.x++ )
			{
				WriteMemblockDWord ( 9, ((t.p*16)+(t.z*4)+t.x)*4,t.pot[t.p][t.x][t.z] );
			}
		}
	}
	TransferImage ( t.terrain.imagestartindex+17,t.terrain.imagestartindex+2,1,9 );

	//  Get the 'approximate Floor ( colour (from default_D) top-left corner pixel (for hemisphere lighting) )
	t.floorcolor=t.pot[3][0][0];
	t.terrain.floorcolorr_f=RgbR(t.floorcolor);
	t.terrain.floorcolorg_f=RgbG(t.floorcolor);
	t.terrain.floorcolorb_f=RgbB(t.floorcolor);
	t.terrain.floorcolorr_f=(t.terrain.floorcolorr_f*0.25)+(255*0.75);
	t.terrain.floorcolorg_f=(t.terrain.floorcolorg_f*0.25)+(255*0.75);
	t.terrain.floorcolorb_f=(t.terrain.floorcolorb_f*0.25)+(255*0.75);

	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

	TextureObject (  t.terrain.terrainobjectindex,3,t.terrain.imagestartindex+17 );
}

void terrain_generateshadows ( void )
{
	/*      
	//  NOTE; need to fix from 4096 to MAXTEXTURESIZE
	//  shadow light setup
	t.terrain.lightrange_f=2500;
	t.terrain.ldirx_f=-0.25*(-1*t.terrain.lightrange_f);
	t.terrain.ldiry_f=-0.5*(-1*t.terrain.lightrange_f);
	t.terrain.ldirz_f=-0.25*(-1*t.terrain.lightrange_f);
	t.terrain.ldirnx_f=t.terrain.ldirx_f/t.terrain.lightrange_f;
	t.terrain.ldirny_f=t.terrain.ldiry_f/t.terrain.lightrange_f;
	t.terrain.ldirnz_f=t.terrain.ldirz_f/t.terrain.lightrange_f;
	//  paint shadows to map
	if (  BitmapExist(g.terrainworkbitmapindex) == 1  )  DeleteBitmapEx (  g.terrainworkbitmapindex );
	if (  BitmapExist(g.terrainworkbitmapindex) == 0 ) 
	{
		CreateBitmap (  g.terrainworkbitmapindex,MAXTEXTURESIZE,MAXTEXTURESIZE );
		PasteImage (  t.terrain.imagestartindex+2,0,0 );
		LockPixels (  );
		//  slope
		lightslope_f=abs(t.terrain.ldiry_f)/1024.0;
		//  paint shadow pixels
		for ( t.z = (MAXTEXTURESIZE/2)-200 ; t.z<=  (MAXTEXTURESIZE/2)+600; t.z++ )
		{
			for ( t.x = (MAXTEXTURESIZE/2)-200 ; t.x<=  (MAXTEXTURESIZE/2)+600; t.x++ )
			{
				t.x_f=(t.x-(MAXTEXTURESIZE/2))*12.5;
				t.z_f=(t.z-(MAXTEXTURESIZE/2))*12.5;
				//  interpolate correct height
				t.nx_f=t.x;
				t.nz_f=t.z;
				t.nx=t.nx_f/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nz=t.nz_f/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nxb_f=(t.nx_f-(t.nx*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.nzb_f=(t.nz_f-(t.nz*MAXTEXTURESIZEMULTIPLIER))/(0.0+MAXTEXTURESIZEMULTIPLIER);
				t.h1_f=t.terrainmatrix[t.nx][t.nz];
				t.h2_f=t.terrainmatrix[t.nx+1][t.nz];
				t.h3_f=t.terrainmatrix[t.nx][t.nz+1];
				t.h4_f=t.terrainmatrix[t.nx+1][t.nz+1];
				//  can get normal from four height corners, and produce normal for Dot (  against light direction )
				t.h_f=99999.9;
				if (  t.h1_f<t.h_f  )  t.h_f = t.h1_f;
				if (  t.h2_f<t.h_f  )  t.h_f = t.h2_f;
				if (  t.h3_f<t.h_f  )  t.h_f = t.h3_f;
				if (  t.h4_f<t.h_f  )  t.h_f = t.h4_f;
				hn1_f=t.h1_f-t.h_f;
				hn2_f=t.h2_f-t.h_f;
				hn3_f=t.h3_f-t.h_f;
				hn4_f=t.h4_f-t.h_f;
				terraintilesize_f=12.5*MAXTEXTURESIZEMULTIPLIER;
				thisnormalx_f=(((hn1_f-hn2_f)/terraintilesize_f)+((hn3_f-hn4_f)/terraintilesize_f))/2.0;
				thisnormalz_f=(((hn1_f-hn3_f)/terraintilesize_f)+((hn2_f-hn4_f)/terraintilesize_f))/2.0;
				thisnormaly_f=1.0-thisnormalx_f-thisnormalz_f;
				thisdot_f=(thisnormalx_f*t.terrain.ldirnx_f)+(thisnormaly_f*t.terrain.ldirny_f)+(thisnormalz_f*t.terrain.ldirnz_f);
				//  trace from terrain GetPoint (  towards sun )
				//  stepping up the ray slope, and if
				//  any terrain height is HIGHER, then
				//  the sun has been blocked by terrain
				t.ha_f=t.h1_f+((t.h2_f-t.h1_f)*t.nxb_f);
				t.hb_f=t.h3_f+((t.h4_f-t.h3_f)*t.nxb_f);
				t.h_f=t.ha_f+((t.hb_f-t.ha_f)*t.nzb_f);
				t.d=0;
				dleadup=0;
				t.nx_f=t.x;
				ny_f=t.h_f;
				t.nz_f=t.z;
				for ( t.n = 0 ; t.n<=  MAXTEXTURESIZE-1; t.n++ )
				{
					t.nx_f=t.nx_f+(t.terrain.ldirnx_f*5.0);
					ny_f=ny_f+(t.terrain.ldirny_f*5.0);
					t.nz_f=t.nz_f+(t.terrain.ldirnz_f*5.0);
					t.nx=t.nx_f/4;
					t.nz=t.nz_f/4;
					if (  t.nx>0 && t.nx<1024 && t.nz>0 && t.nz<1024 ) 
					{
						//  interpolate between the four corner heights
						t.nxb_f=(t.nx_f-(t.nx*4))/4.0;
						t.nzb_f=(t.nz_f-(t.nz*4))/4.0;
						t.h1_f=t.terrainmatrix[t.nx][t.nz];
						t.h2_f=t.terrainmatrix[t.nx+1][t.nz];
						t.h3_f=t.terrainmatrix[t.nx][t.nz+1];
						t.h4_f=t.terrainmatrix[t.nx+1][t.nz+1];
						t.ha_f=t.h1_f+((t.h2_f-t.h1_f)*t.nxb_f);
						t.hb_f=t.h3_f+((t.h4_f-t.h3_f)*t.nxb_f);
						t.h_f=t.ha_f+((t.hb_f-t.ha_f)*t.nzb_f);
						if (  t.h_f>ny_f ) 
						{
							//  terrain blocked sun ray = shadow!
							dleadupsize=dleadup;
							inc t.d ; if (  t.d >= 16  )  break;
						}
						inc dleadup;
						if (  ny_f>5500.0 ) 
						{
							//  ray cleared highest peek
							break;
						}
					}
					else
					{
						//  ray left map
						break;
					}
				}
				t.col=GetPoint(t.x,t.z);
				colr=RgbR(t.col);
				colg=RgbG(t.col);
				colb=RgbB(t.col);
				colg=colg+abs(thisdot_f)*64;
				colgbase=colg;
				if (  t.d == 0 ) 
				{
					//  can skip, no change, in full light
				}
				else
				{
					castshadowstr=(t.d*12);
					castshadowstr=castshadowstr-(dleadupsize);
					colg=colg+castshadowstr;
				}
				if (  colg<colgbase  )  colg = colgbase;
				if (  colg>255  )  colg = 255;
				Dot (  t.x,t.z,Rgb(colr,colg,colb) );
			}
		}
		//  blue step to smooth in strange step artefacts?!?
		if (  0 ) 
		{
		for ( t.z = 2048-200 ; t.z<=  2048+200; t.z++ )
		{
			for ( t.x = 2048-200 ; t.x<=  2048+200; t.x++ )
			{
				t.col=GetPoint(t.x,t.z);
				colr=RgbR(t.col);
				colg=RgbG(t.col);
				colb=RgbB(t.col);
				if (  colg>0 ) 
				{
					//  back
					colgstore=colg/2;
					t.col=GetPoint(t.x-1,t.z);
					colr=RgbR(t.col);
					colg=RgbG(t.col)+colgstore;
					colb=RgbB(t.col);
					if (  colg>255  )  colg = 255;
					Dot (  t.x-1,t.z,Rgb(colr,colg,colb) );
					//  up
					t.col=GetPoint(t.x,t.z-1);
					colr=RgbR(t.col);
					colg=RgbG(t.col)+colgstore;
					colb=RgbB(t.col);
					if (  colg>255  )  colg = 255;
					Dot (  t.x,t.z-1,Rgb(colr,colg,colb) );
					//  further blur
					colgstore=colgstore/2;
					t.col=GetPoint(t.x-1,t.z-1);
					colr=RgbR(t.col);
					colg=RgbG(t.col)+colgstore;
					colb=RgbB(t.col);
					if (  colg>255  )  colg = 255;
					Dot (  t.x-1,t.z-1,Rgb(colr,colg,colb) );
					t.col=GetPoint(t.x-2,t.z);
					colr=RgbR(t.col);
					colg=RgbG(t.col)+colgstore;
					colb=RgbB(t.col);
					if (  colg>255  )  colg = 255;
					Dot (  t.x-2,t.z,Rgb(colr,colg,colb) );
					t.col=GetPoint(t.x,t.z-2);
					colr=RgbR(t.col);
					colg=RgbG(t.col)+colgstore;
					colb=RgbB(t.col);
					if (  colg>255  )  colg = 255;
					Dot (  t.x,t.z-2,Rgb(colr,colg,colb) );
				}
			}
		}
		}
		UnlockPixels (  );
		GrabImage (  t.terrain.imagestartindex+2,0,0,MAXTEXTURESIZE,MAXTEXTURESIZE );
		TextureObject (  t.terrain.objectstartindex+1,t.terrain.imagestartindex+2 );
		ShowObject (  t.terrain.objectstartindex+1 );
		DeleteBitmapEx (  g.terrainworkbitmapindex );
	}
	else
	{
			"Bitmap "+Str(g.terrainworkbitmapindex)+" already exists!","Terrain" ; end;
	}
	*/ 
}

void generate_terrain ( int seed, int scale, int mchunk_size )
{
	// restored to original method
	// diamond-square algorithm - thanks to Lewis999 from the DBP 20 liner challenge forum!
	Randomize(seed);
	int s = scale;
	// generates the main point on the line z=0 for the first matrix 
	for ( int x = 2^s; x <= mchunk_size; x += 2^s )
	{
		t.terrainmatrix[x][0] = t.terrainmatrix[x-2^s][0] + Rnd((2^s)*10)-((2^s)*10)/2;
	}
	//generates the main point on theline y=0 for the first matrix 
	for ( int y = 2^s; y <= mchunk_size; y += 2^s )
	{
		t.terrainmatrix[0][y] = t.terrainmatrix[0][y-2^s] + Rnd((2^s)*10)-((2^s)*10)/2;
	}
	//generates the rest of the main points for the first matrix
	for ( int y = 2^s; y <= mchunk_size; y += 2^s )
	{
		for ( int x = 2^s; x <= mchunk_size; x += 2^s )
		{
			t.terrainmatrix[x][y] = (t.terrainmatrix[x-2^s][y] + t.terrainmatrix[x][y-2^s])/2 + Rnd((2^s)*10)-((2^s)*10)/2;
		}
	}
	//calculates the rest of the points based on the main points
	for ( int o = 1; o <=s; o++ )
	{
		int p = s-o+1;
		int pow = powf(2,p); // replaces 2^p
		for ( int y = 0; y <=mchunk_size; y += pow )
		{
			for ( int x = pow/2; x <= mchunk_size; x += pow )
			{
				t.terrainmatrix[x][y] = (t.terrainmatrix[x-(pow/2)][y] + t.terrainmatrix[x+(pow/2)][y])/2 + Rnd(pow*2)-pow;
			}
		}
		for ( int y = pow/2; y <= mchunk_size; y += pow )
		{
			for ( int x = 0; x <= mchunk_size; x += pow )
			{
				t.terrainmatrix[x][y] = (t.terrainmatrix[x][y-(pow/2)] + t.terrainmatrix[x][y+(pow/2)])/2 + Rnd(pow*2)-pow;
			}
		}
		for ( int y = pow/2; y <= mchunk_size; y += pow )
		{
			for ( int x = pow/2; x <= mchunk_size; x += pow )
			{
				t.terrainmatrix[x][y] = (t.terrainmatrix[x][y-(pow/2)] + t.terrainmatrix[x][y+(pow/2)] + t.terrainmatrix[x-(pow/2)][y] + t.terrainmatrix[x+(pow/2)][y])/4 + Rnd(pow*2)-pow;
			}
		}
	}
}

void generate_terrain_dave ( int seed, int scale, int mchunk_size )
{
	// flatten out initially
	for ( int y = 0 ; y < 1024 ; y++ )
	{
		for ( int x = 0 ; x < 1024 ; x++ )
		{
			t.terrainmatrix[x][y] = 0;
		}
	}
	DiamondSquare ( 0,0,1024,1024,400,128 );
	return;
}

void DiamondSquare(unsigned x1, unsigned y1, unsigned x2, unsigned y2, float range, unsigned level) 
{
    if (level < 1) return;

    // diamonds
    for (unsigned i = x1 + level; i < x2; i += level)
	{
        for (unsigned j = y1 + level; j < y2; j += level) 
		{
            float a = t.terrainmatrix[i - level][j - level];
            float b = t.terrainmatrix[i][j - level];
            float c = t.terrainmatrix[i - level][j];
            float d = t.terrainmatrix[i][j];
            float e = t.terrainmatrix[i - level / 2][j - level / 2] = (a + b + c + d) / 4 + ((float)Rnd(100) / 100.0f) * range;
        }
	}

    // squares
    for (unsigned i = x1 + 2 * level; i < x2; i += level)
	{
        for (unsigned j = y1 + 2 * level; j < y2; j += level) 
		{
            float a = t.terrainmatrix[i - level][j - level];
            float b = t.terrainmatrix[i][j - level];
            float c = t.terrainmatrix[i - level][j];
            float d = t.terrainmatrix[i][j];
            float e = t.terrainmatrix[i - level / 2][j - level / 2];

            float f = t.terrainmatrix[i - level][j - level / 2] = (a + c + e + t.terrainmatrix[i - 3 * level / 2][j - level / 2]) / 4 + ((float)Rnd(100) / 100.0f) * range;
            float g = t.terrainmatrix[i - level / 2][j - level] = (a + b + e + t.terrainmatrix[i - level / 2][j - 3 * level / 2]) / 4 + ((float)Rnd(100) / 100.0f) * range;
        }
	}

    DiamondSquare(x1, y1, x2, y2, range / 2, level / 2);
}

// 
//  TERRAIN IN-GAME
// 

void terrain_start_play ( void )
{
	//  Turn highlighting off
	if ( t.terrain.terrainshaderindex>0 ) 
	{
		SetVector4 (  g.terrainvectorindex,-999999,-999999,0,0 );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightCursor",g.terrainvectorindex );
		SetVector4 (  g.terrainvectorindex,0,0,0,0 );
		SetEffectConstantV (  t.terrain.terrainshaderindex,"HighlightParams",g.terrainvectorindex );
	}

	// switch off normals in terrain (allow baked shadows to takeover)
	if ( ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		SetObjectLight (  t.terrain.terrainobjectindex,0 );
	}

	//  fog effect to create sense of distance
	t.tFogNear_f=t.visuals.FogNearest_f ; t.tFogFar_f=t.visuals.FogDistance_f;
	t.tFogR_f=t.visuals.FogR_f ; t.tFogG_f=t.visuals.FogG_f ; t.tFogB_f=t.visuals.FogB_f ; ; t.tFogA_f=t.visuals.FogA_f;
	terrain_setfog ( );
}

void terrain_stop_play ( void )
{
	t.tFogR_f = 0; t.tFogG_f = 0; t.tFogB_f = 0; t.tFogA_f = 0; t.tFogNear_f = 500000; t.tFogFar_f = 15000000;
	terrain_setfog ( );

	// switch terrain normals back on for real-time editing
	if ( ObjectExist(t.terrain.terrainobjectindex) == 1 ) 
	{
		SetObjectLight (  t.terrain.terrainobjectindex,1 );
		SetObjectMask (  t.terrain.terrainobjectindex, 1 );
	}

	// Restore camera range for editing
	SetCameraRange (  100,55000 );
}

void terrain_setfog ( void )
{
	// takes; tFogR#,tFogG#,tFogB#,tFogNear#,tFogFar#
	if ( t.terrain.terrainshaderindex>0 ) 
	{
		SetVector4 ( g.terrainvectorindex,0,0,0,0 );
		SetEffectConstantV ( t.terrain.terrainshaderindex,"FogColor",g.terrainvectorindex );
		SetVector4 ( g.terrainvectorindex,t.tFogR_f/255.0,t.tFogG_f/255.0,t.tFogB_f/255.0,t.tFogA_f/255.0 );
		SetEffectConstantV ( t.terrain.terrainshaderindex,"HudFogColor",g.terrainvectorindex );
		SetVector4 ( g.terrainvectorindex,t.tFogNear_f,t.tFogFar_f,0,0 );
		SetEffectConstantV ( t.terrain.terrainshaderindex,"HudFogDist",g.terrainvectorindex );
	}
	if ( t.terrain.vegetationshaderindex>0 ) 
	{
		SetVector4 ( g.vegetationvectorindex,0,0,0,0 );
		SetEffectConstantV ( t.terrain.vegetationshaderindex,"FogColor",g.vegetationvectorindex );
		SetVector4 ( g.vegetationvectorindex,t.tFogR_f/255.0,t.tFogG_f/255.0,t.tFogB_f/255.0,t.tFogA_f/255.0 );
		SetEffectConstantV ( t.terrain.vegetationshaderindex,"HudFogColor",g.vegetationvectorindex );
		SetVector4 ( g.vegetationvectorindex,t.tFogNear_f,t.tFogFar_f,0,0 );
		SetEffectConstantV ( t.terrain.vegetationshaderindex,"HudFogDist",g.vegetationvectorindex );
	}
}

void terrain_parsed_getstring ( void )
{
	for ( t.n = 1 ; t.n<=  Len(t.line_s.Get()); t.n++ )
	{
		if ( cstr(Mid(t.line_s.Get(),t.n)) == "=" ) 
		{
			if ( cstr(Mid(t.line_s.Get(),t.n+1)) == " " ) 
				t.rest_s=Right(t.line_s.Get(),(Len(t.line_s.Get())-t.n)-1);
			else
				t.rest_s=Right(t.line_s.Get(),Len(t.line_s.Get())-t.n);

			t.n=Len(t.line_s.Get());
		}
	}
}

void terrain_parsed_getvalues ( void )
{
	for ( t.n = 1 ; t.n<=  Len(t.line_s.Get()); t.n++ )
	{
		if ( cstr(Mid(t.line_s.Get(),t.n)) == "=" ) 
		{
			t.rest_s=Right(t.line_s.Get(),Len(t.line_s.Get())-t.n);
			t.n=Len(t.line_s.Get());
		}
	}
	t.valuei=0;
	for ( t.n = 1 ; t.n<=  Len(t.rest_s.Get()); t.n++ )
	{
		if (  cstr(Mid(t.rest_s.Get(),t.n)) == "," || t.n == Len(t.rest_s.Get()) ) 
		{
			if (  t.n == Len(t.rest_s.Get()) ) 
			{
				t.value_s=Left(t.rest_s.Get(),t.n);
			}
			else
			{
				t.value_s=Left(t.rest_s.Get(),t.n-1);
			}
			t.value_f[t.valuei]=ValF(t.value_s.Get()) ; ++t.valuei;
			t.rest_s=Right(t.rest_s.Get(),Len(t.rest_s.Get())-t.n);
			t.n=0;
		}
	}
}

void sky_init ( void )
{
	//  Dir
	SetDir ( "skybank" );
	timestampactivity(0, "entering skybank");

	//  Assemble list of all skies
	g.skymax=0;
	ChecklistForFiles (  );
	timestampactivity(0, "checking skybank files");
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		if (  ChecklistValueA(t.c) == 1 ) 
		{
			t.file_s=ChecklistString(t.c);
			timestampactivity(0, t.file_s.Get());
			if (  cstr(Left(t.file_s.Get(),1)) != "." )
			{
				++g.skymax;
				Dim (  t.skybank_s,g.skymax );
				t.skybank_s[g.skymax]=Lower(t.file_s.Get());
			}
		}
	}

	//  Include one sub-folder in for artist skyboxes
	for ( t.s = 1 ; t.s<=  g.skymax; t.s++ )
	{
		t.ttry_s=t.skybank_s[t.s]+"\\skyspec.txt";
		if (  FileExist(t.ttry_s.Get()) == 0 ) 
		{
			//  set entry to remove
			t.removethisone=t.s;
			//  into the folder
			SetDir (  t.skybank_s[t.s].Get() );
			//  now populate with any artist skies
			ChecklistForFiles (  );
			for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
			{
				if (  ChecklistValueA(t.c) == 1 ) 
				{
					t.file_s=ChecklistString(t.c);
					if (  cstr(Left(t.file_s.Get(),1)) != "." ) 
					{
						++g.skymax;
						Dim (  t.skybank_s,g.skymax );
						t.skybank_s[g.skymax]=t.skybank_s[t.s]+"\\"+Lower(t.file_s.Get());
					}
				}
			}
			//  out of folder
			SetDir (  ".." );
			//  remove this empty folder entry
			for ( t.ss = t.removethisone ; t.ss<=  g.skymax-1; t.ss++ )
			{
				t.skybank_s[t.ss]=t.skybank_s[t.ss+1];
			}
			--g.skymax;
		}
	}

	//  Restore Dir
	SetDir (  ".." );
	timestampactivity(0, "finished sky bank scan");

	//  Default sky settings
	t.sky.currenthour_f=12.0;
	t.sky.daynightprogress=0;
}

void terrain_skyspec_init ( void )
{
	//  Get lighting data from skyspec
	if ( g.skyindex>0 ) 
	{
		//  Load sky spec for light position and color
		t.terrain.skyshader_s="";
		t.terrain.skyscrollshader_s="";
		t.skyname_s=t.skybank_s[g.skyindex];
		t.terrain.sunskyscrollspeedx_f=0.0;
		t.terrain.sunskyscrollspeedz_f=0.0;
		timestampactivity(0,cstr(cstr("Loading skyspec.txt:")+t.skyname_s).Get() );
		OpenToRead (  1, cstr(cstr("skybank\\")+t.skyname_s+"\\skyspec.txt").Get() );
		Dim ( t.value_f, 3 ); t.valuei=0;
		do
		{
			t.line_s = ReadString ( 1 );
			t.line_s = Lower(t.line_s.Get());
			t.try_s="sundirection";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s ) 
			{
				terrain_parsed_getvalues ( );
				t.terrain.sundirectionx_f=t.value_f[0]+25;
				t.terrain.sundirectiony_f=t.value_f[1];
				t.terrain.sundirectionz_f=t.value_f[2];
				t.tav_f=abs(t.terrain.sundirectionx_f)+abs(t.terrain.sundirectiony_f)+abs(t.terrain.sundirectionz_f);
				t.terrain.sundirectionx_f=t.terrain.sundirectionx_f/t.tav_f;
				t.terrain.sundirectiony_f=t.terrain.sundirectiony_f/t.tav_f;
				t.terrain.sundirectionz_f=t.terrain.sundirectionz_f/t.tav_f;
			}
			t.try_s="sunstrength";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s ) 
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunstrength_f=t.value_f[0];
			}
			t.try_s="suncolor";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s ) 
			{
				terrain_parsed_getvalues ( );
				t.terrain.suncolorr_f=t.value_f[0];
				t.terrain.suncolorg_f=t.value_f[1];
				t.terrain.suncolorb_f=t.value_f[2];
				t.terrain.suncolorr_f=(t.terrain.suncolorr_f*0.5)+(255*0.5);
				t.terrain.suncolorg_f=(t.terrain.suncolorg_f*0.5)+(255*0.5);
				t.terrain.suncolorb_f=(t.terrain.suncolorb_f*0.5)+(255*0.5);
			}
			t.try_s="sunrotation";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunrotation_f=t.value_f[0];
			}
			t.try_s="skyscrollx";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunskyscrollspeedx_f=t.value_f[0]/50000.0;
			}
			t.try_s="skyscrollz";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getvalues ( );
				t.terrain.sunskyscrollspeedz_f=t.value_f[0]/50000.0;
			}
			t.try_s="skyshader";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getstring ( );
				t.terrain.skyshader_s = t.rest_s;
			}
			t.try_s="skyscrollshader";
			if (  cstr(Left(t.line_s.Get(),Len(t.try_s.Get()))) == t.try_s )
			{
				terrain_parsed_getstring ( );
				t.terrain.skyscrollshader_s = t.rest_s;
			}
		} while ( !(  t.line_s == "" ) );
		CloseFile (  1 );
	}

	if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

	//  If not from editor, also load and create sky assets
	if (  t.game.set.ismapeditormode == 0 ) 
	{
		// Use sky shader
		cstr useSkyShader_s = "effectbank\\reloaded\\sky_basic.fx";
		cstr useSkyScrollShader_s = "effectbank\\reloaded\\skyscroll_basic.fx";
		if ( t.terrain.skyshader_s != "" ) useSkyShader_s = cstr ( cstr("effectbank\\reloaded\\") + t.terrain.skyshader_s );
		if ( t.terrain.skyscrollshader_s != "" ) useSkyScrollShader_s = cstr ( cstr("effectbank\\reloaded\\") + t.terrain.skyscrollshader_s );

		// In case shader changes, delete any old effects
		if (  GetEffectExist(t.terrain.effectstartindex+4) == 1 ) 
		{
			EraseEffectParameterIndex ( t.terrain.effectstartindex+4, "HudFogDist" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+4, "HudFogColor" );
			DeleteEffect ( t.terrain.effectstartindex+4 );
		}
		if (  GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
		{
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "SkyScrollValues" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "HudFogDist" );
			EraseEffectParameterIndex ( t.terrain.effectstartindex+9, "HudFogColor" );
			DeleteEffect ( t.terrain.effectstartindex+9 );
		}

		// Load sky shaders
		if ( GetEffectExist(t.terrain.effectstartindex+4) == 0 ) 
		{
			timestampactivity(0,cstr(cstr("Loading sky shaders:")+useSkyShader_s).Get() );
			LoadEffect ( useSkyShader_s.Get(), t.terrain.effectstartindex+4, 0 );
			t.effectparam.sky.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+4,"HudFogDist");
			t.effectparam.sky.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+4,"HudFogColor");
		}
		if ( GetEffectExist(t.terrain.effectstartindex+9) == 0 ) 
		{
			timestampactivity(0,cstr(cstr("Loading sky shaders:")+useSkyScrollShader_s).Get() );
			LoadEffect ( useSkyScrollShader_s.Get(), t.terrain.effectstartindex+9, 0 );
			t.effectparam.skyscroll.SkyScrollValues=GetEffectParameterIndex(t.terrain.effectstartindex+9,"SkyScrollValues");
			t.effectparam.skyscroll.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+9,"HudFogDist");
			t.effectparam.skyscroll.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+9,"HudFogColor");
		}
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

		//  Initialise main and secondary sky
		SetMipmapNum ( 1 );
		timestampactivity(0,cstr(cstr("Initialise sky textures")).Get());
		for ( t.tskyi = 0 ; t.tskyi<=  t.terrainskyspecinitmode; t.tskyi++ )
		{
			//  sky object
			if (  t.tskyi == 0 ) { t.skyobj = t.terrain.objectstartindex+4  ; t.skyname_s = t.skybank_s[g.skyindex]; }
			if (  t.tskyi == 1 ) { t.skyobj = t.terrain.objectstartindex+8  ; t.skyname_s = "night"; }

			//  Load sky spec for light position and color
			if (  t.tskyi == 0 ) 
			{
				//  when sky changes, need to change sun feed into gun shaders
				gun_updategunshaders ( );
			}

			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			//  skyname only
			t.skynameonly_s=t.skyname_s;
			for ( t.n = Len(t.skyname_s.Get()) ; t.n >= 1 ; t.n+= -1 )
			{
				if (  cstr(Mid(t.skyname_s.Get(),t.n)) == "\\" || cstr(Mid(t.skyname_s.Get(),t.n)) == "/" ) 
				{
					t.skynameonly_s=Right(t.skyname_s.Get(),Len(t.skyname_s.Get())-t.n);
					break;
				}
			}

			//  Sky object (NOTE; ideally need to render sky Box (  FIRST (avoid making cam range higher!) )
			if (  ObjectExist(t.skyobj) == 1  )  DeleteObject (  t.skyobj );
			cstr pFileToLoad = cstr(cstr("skybank\\")+t.skyname_s+"\\"+t.skynameonly_s+".DBO");
			if ( FileExist ( pFileToLoad.Get() ) == 0 ) pFileToLoad = cstr(cstr("skybank\\")+t.skyname_s+"\\"+t.skynameonly_s+".X");
			timestampactivity(0,cstr(cstr("Load sky geometry:")+pFileToLoad).Get() );
			LoadObject ( pFileToLoad.Get(), t.skyobj );
			ScaleObject (  t.skyobj,750,750,750 );
			SetObjectTextureMode (  t.skyobj,2,0 );
			SetObjectMask (  t.skyobj,2+1 ); // DX11 actually hidden when lightray renders
			YRotateObject (  t.skyobj,t.terrain.sunrotation_f );
			SetObjectEffect (  t.skyobj,t.terrain.effectstartindex+4 );

			//  Fade default
			if (  t.tskyi == 0  )  SetAlphaMappingOn (  t.skyobj,100 );
			if (  t.tskyi == 1  )  SetAlphaMappingOn (  t.skyobj,0 );

			//  Transparency mode 7 draws it before all others
			SetObjectTransparency (  t.skyobj,7 );
			DisableObjectZDepth (  t.skyobj );
			DisableObjectZWrite (  t.skyobj );
			DisableObjectZRead (  t.skyobj );

			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
		}

		//  For MAIN sky, get full name and name only strings (again)
		t.skyname_s=t.skybank_s[g.skyindex];
		t.skynameonly_s=t.skyname_s;
		for ( t.n = Len(t.skyname_s.Get()) ; t.n >=  1 ; t.n+= -1 )
		{
			if (  cstr(Mid(t.skyname_s.Get(),t.n)) == "\\" || cstr(Mid(t.skyname_s.Get(),t.n)) == "/" ) 
			{
				t.skynameonly_s=Right(t.skyname_s.Get(),Len(t.skyname_s.Get())-t.n);
				break;
			}
		}

		//  Detect for and load skyscroll plane
		t.skyscroll_s=cstr("skybank\\")+t.skyname_s+"\\"+t.skynameonly_s+"_CLOUDS.dds";
		timestampactivity(0,cstr(cstr("Loading t.skyscroll_s:")+t.skyscroll_s).Get() );
		if (  FileExist(t.skyscroll_s.Get()) == 1 ) 
		{
			if (  ObjectExist(t.terrain.objectstartindex+9) == 1 ) DeleteObject ( t.terrain.objectstartindex+9 );
			if (  ObjectExist(t.terrain.objectstartindex+9) == 0 ) 
			{
				//  Create sky scoll object
				MakeObjectPlane (  t.terrain.objectstartindex+9,90000,90000 );
				SetObjectTextureMode (  t.terrain.objectstartindex+9,2,0 );
				SetObjectTransparency (  t.terrain.objectstartindex+9,1 );
				SetObjectCollisionOff (  t.terrain.objectstartindex+9 );
				XRotateObject (  t.terrain.objectstartindex+9,270 );
				SetObjectLight (  t.terrain.objectstartindex+9,0 );
				SetObjectCull (  t.terrain.objectstartindex+9,0 );
				SetObjectFog (  t.terrain.objectstartindex+9,0 );

				//  Now create a second UV data layer for static second texture (alphaportal)
				CloneMeshToNewFormat (  t.terrain.objectstartindex+9,512+2 );
				t.q_f=1.0;
				LockVertexDataForLimb (  t.terrain.objectstartindex+9,0 );
				SetVertexDataUV (  0,1,0,0 );
				SetVertexDataUV (  0,1,t.q_f,0 );
				SetVertexDataUV (  1,1,0,0 );
				SetVertexDataUV (  2,1,t.q_f,t.q_f );
				SetVertexDataUV (  3,1,0,0 );
				SetVertexDataUV (  4,1,0,t.q_f );
				SetVertexDataUV (  5,1,t.q_f,t.q_f );
				UnlockVertexData (  );

				//  Apply skyscroll shader (uses second UV data for alpha channel from cloud texture)
				SetObjectEffect (  t.terrain.objectstartindex+9,t.terrain.effectstartindex+9 );
			}
			else
			{
				//  Show if hidden
				ShowObject (  t.terrain.objectstartindex+9 );
			}
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );

			//  Always load in specified texture as sky can change
			LoadImage ( t.skyscroll_s.Get(),t.terrain.imagestartindex+9,0,g.gdividetexturesize );
			TextureObject (  t.terrain.objectstartindex+9,0,t.terrain.imagestartindex+9 );

			//  Load sky cloud portal
			LoadImage (  "skybank\\cloudportal.dds",t.terrain.imagestartindex+10,0,g.gdividetexturesize );
			TextureObject (  t.terrain.objectstartindex+9,1,t.terrain.imagestartindex+10 );
		}
		else
		{
			//  Hide if not used and exists
			if (  ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
			{
				HideObject (  t.terrain.objectstartindex+9 );
			}
		}
		if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
		SetMipmapNum ( -1 );
	}

	//  when change sky, due to sun direction, must update shader constants too
	t.visuals.refreshshaders=1;
}

void terrain_sky_hide ( void )
{
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
}

void terrain_sky_show ( void )
{
	if (  ObjectExist(t.terrain.objectstartindex+4) == 1 && t.tskyobj1v == 1  )  ShowObject (  t.terrain.objectstartindex+4 );
	if (  ObjectExist(t.terrain.objectstartindex+8) == 1 && t.tskyobj2v == 1  )  ShowObject (  t.terrain.objectstartindex+8 );
	if (  ObjectExist(t.terrain.objectstartindex+9) == 1 && t.tskyobj3v == 1  )  ShowObject (  t.terrain.objectstartindex+9 );
}

void terrain_sky_free ( void )
{
	//  free sky objects
	if (  ObjectExist(t.terrain.objectstartindex+4) == 1  )  DeleteObject (  t.terrain.objectstartindex+4 );
	if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  DeleteObject (  t.terrain.objectstartindex+8 );
	if (  ObjectExist(t.terrain.objectstartindex+9) == 1  )  DeleteObject (  t.terrain.objectstartindex+9 );
}

void terrain_sky_loop ( void )
{

	//  Control sky fades (day/night/switch) - removed from first version
//  `if ScanCode() == 0 then sky.changingsky == 0
	
//  `if sky.changingsky=0

	//if KeyState(65)=1
	// //  go to evening and resum day/night cycle
	// sky.currenthour#=22.0
	// sky.daynightprogress=1
	// sky.changingsky=1
	//endif
//  `endif

//  `if sky.daynightprogress=1

	//sky.currenthour#=sky.currenthour#+0.005
	//if sky.currenthour# >= 24.0 then sky.currenthour# == sky.currenthour#-24.0
//  `endif

//  `if sky.currenthour#>18.0

	////  starts to get night
	//sky.alpha2#=(sky.currenthour#-18.0)/6.0
	//sky.alpha1#=1.0-sky.alpha2#
//  `else

	//if sky.currenthour#>2.0 and sky.currenthour#<=8.0
	// //  starts to get day
	// sky.alpha1#=(sky.currenthour#-2.0)/6.0
	// sky.alpha2#=1.0-sky.alpha1#
	//else
	// if sky.currenthour#<=2.0
	//  //  night
	//  sky.alpha1#=0
	//  sky.alpha2#=1.0
	// else
	//  //  day
	//  sky.alpha1#=1.0
	//  sky.alpha2#=0
	// endif
	//endif
//  `endif


	//  day
	t.sky.alpha1_f=1.0;
	t.sky.alpha2_f=0;

	//  Update skybox position
	if (  t.hardwareinfoglobals.nosky == 0 ) 
	{
		if (  ObjectExist(t.terrain.objectstartindex+4) == 1 ) 
		{
			if (  t.sky.alpha1_f>0 ) 
			{
				PositionObject (  t.terrain.objectstartindex+4,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				SetAlphaMappingOn (  t.terrain.objectstartindex+4,100.0*t.sky.alpha1_f );
				ShowObject (  t.terrain.objectstartindex+4 );
			}
			else
			{
				HideObject (  t.terrain.objectstartindex+4 );
			}
		}
		if (  ObjectExist(t.terrain.objectstartindex+8) == 1 ) 
		{
			if (  t.sky.alpha2_f>0 ) 
			{
				PositionObject (  t.terrain.objectstartindex+8,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera),CameraPositionZ(t.terrain.gameplaycamera) );
				SetAlphaMappingOn (  t.terrain.objectstartindex+8,100.0*t.sky.alpha2_f );
				ShowObject (  t.terrain.objectstartindex+8 );
			}
			else
			{
				HideObject (  t.terrain.objectstartindex+8 );
			}
		}
		if (  ObjectExist(t.terrain.objectstartindex+9) == 1 ) 
		{
			PositionObject (  t.terrain.objectstartindex+9,CameraPositionX(t.terrain.gameplaycamera),CameraPositionY(t.terrain.gameplaycamera)+7000,CameraPositionZ(t.terrain.gameplaycamera) );
			if (  GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
			{
				t.terrain.sunskyscrollx_f=t.terrain.sunskyscrollx_f+(t.terrain.sunskyscrollspeedx_f*g.timeelapsed_f);
				t.terrain.sunskyscrollz_f=t.terrain.sunskyscrollz_f+(t.terrain.sunskyscrollspeedz_f*g.timeelapsed_f);
				SetVector4 (  g.terrainvectorindex,t.terrain.sunskyscrollx_f,t.terrain.sunskyscrollz_f,0,0 );
				SetEffectConstantVEx (  t.terrain.effectstartindex+9,t.effectparam.skyscroll.SkyScrollValues,g.terrainvectorindex );
			}
		}
	}

	//  update sky fog
	if (  GetEffectExist(t.terrain.effectstartindex+4) ) 
	{
		SetVector4 (  g.terrainvectorindex,t.visuals.FogNearest_f,t.visuals.FogDistance_f,0,0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+4,t.effectparam.sky.HudFogDist,g.terrainvectorindex );
		SetVector4 (  g.terrainvectorindex,t.visuals.FogR_f/255.0,t.visuals.FogG_f/255.0,t.visuals.FogB_f/255.0,t.visuals.FogA_f/255.0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+4,t.effectparam.sky.HudFogColor,g.terrainvectorindex );
	}
	if (  GetEffectExist(t.terrain.effectstartindex+9) == 1 ) 
	{
		SetVector4 (  g.terrainvectorindex,t.visuals.FogNearest_f,t.visuals.FogDistance_f,0,0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+9,t.effectparam.skyscroll.HudFogDist,g.terrainvectorindex );
		SetVector4 (  g.terrainvectorindex,t.visuals.FogR_f/255.0,t.visuals.FogG_f/255.0,t.visuals.FogB_f/255.0,t.visuals.FogA_f/255.0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+9,t.effectparam.skyscroll.HudFogColor,g.terrainvectorindex );
	}

return;

}

void terrain_water_init ( void )
{
	//  Setup Reflection Camera (range set in visuals)
	CreateCamera (  2  ); BackdropOff (  2 );
	t.terrain.reflsizer=g.greflectionrendersize;
	SetCameraToImage (  2,t.terrain.imagestartindex+6,t.terrain.reflsizer,t.terrain.reflsizer );

	//  Re-make water mask if required
	t.tfilewater_s=g.mysystem.levelBankTestMap_s+"watermask.dds"; //"levelbank\\testmap\\watermask.dds";
	t.terrain.terrainregionupdate=0;
	terrain_refreshterrainmatrix ( );
	terrain_updatewatermask ( );
	terrain_clearterraindirtyregion ( );

	//  Make Water plain
	LoadImage (  "effectbank\\reloaded\\media\\waves2.dds",t.terrain.imagestartindex+7,0,0);//g.gdividetexturesize );
	
	if ( ImageExist(t.terrain.imagestartindex+4) == 0 ) 
	{
		//PE: Just create a 1x1 image for shader , if OLDWATER is used.
		if (g.memskipwatermask == 1) {
			//  blank water mask
			SetCurrentBitmap(g.terrainworkbitmapindex);
			if (FileExist(t.tfilewater_s.Get()) == 1)  DeleteAFile(t.tfilewater_s.Get());
			CLS(Rgb(0, 0, 0));
			GrabImage(t.terrain.imagestartindex + 4, 0, 0, 1, 1);
			//SaveImage(t.tfilewater_s.Get(), t.terrain.imagestartindex + 4);
			SetCurrentBitmap(0);
		}
		else {
			SetMipmapNum(1);
			//LoadImage("levelbank\\testmap\\watermask.dds", t.terrain.imagestartindex + 4, 10, 0);
			LoadImage(cstr(g.mysystem.levelBankTestMap_s+"watermask.dds").Get(), t.terrain.imagestartindex + 4, 10, 0);
			SetMipmapNum(-1);
		}
	}
	MakeObjectPlane (  t.terrain.objectstartindex+5,1024*50,1024*50 );
	PositionObject (  t.terrain.objectstartindex+5,1024*25,t.terrain.waterliney_f,1024*25 );
	TextureObject (  t.terrain.objectstartindex+5,0,t.terrain.imagestartindex+7 );
	TextureObject (  t.terrain.objectstartindex+5,1,t.terrain.imagestartindex+5 );
	TextureObject (  t.terrain.objectstartindex+5,2,t.terrain.imagestartindex+6 );
	TextureObject (  t.terrain.objectstartindex+5,3,t.terrain.imagestartindex+4 );
	// now done for editor water earlier
	SetEffectTechnique ( t.terrain.effectstartindex+1, "UseReflection" );
	//if ( GetEffectExist ( t.terrain.effectstartindex+1 ) == 0 )
	//{
	//	LoadEffect (  "effectbank\\reloaded\\water_basic.fx",t.terrain.effectstartindex+1,0 );
	//	t.effectparam.water.HudFogDist=GetEffectParameterIndex(t.terrain.effectstartindex+1,"HudFogDist");
	//	t.effectparam.water.HudFogColor=GetEffectParameterIndex(t.terrain.effectstartindex+1,"HudFogColor");
	//}
	SetObjectEffect (  t.terrain.objectstartindex+5,t.terrain.effectstartindex+1 );
	XRotateObject (  t.terrain.objectstartindex+5,90 );
	SetObjectTransparency (  t.terrain.objectstartindex+5, 5 ); // 021215 - the only object that should have this flag (WATER PLANE)
	SetObjectOcclusion (  t.terrain.objectstartindex+5,0,0,0,0,0 );

	//  set fog settings
	t.tFogNear_f=t.visuals.FogNearest_f ; t.tFogFar_f=t.visuals.FogDistance_f;
	t.tFogR_f=t.visuals.FogR_f ; t.tFogG_f=t.visuals.FogG_f ; t.tFogB_f=t.visuals.FogB_f ; ; t.tFogA_f=t.visuals.FogA_f;
	terrain_water_setfog ( );

	//  hide editor water object
	if (  ObjectExist(t.terrain.objectstartindex+2) == 1 ) 
	{
		HideObject (  t.terrain.objectstartindex+2 );
	}
}

void terrain_water_free ( void )
{
	//  free quick test water and sky effects
	if (  ObjectExist(t.terrain.objectstartindex+5) == 1  )  DeleteObject (  t.terrain.objectstartindex+5 );

	// DX11 cannot delete effect - sort this later with Shader Editor
	SetEffectTechnique ( t.terrain.effectstartindex+1, "Editor" );
	//if (  GetEffectExist(t.terrain.effectstartindex+1) == 1  )  DeleteEffect (  t.terrain.effectstartindex+1 );

	if (  CameraExist(2) == 1  )  DestroyCamera (  2 );
	if (  ImageExist(t.terrain.imagestartindex+6) == 1  )  DeleteImage (  t.terrain.imagestartindex+6 );
	if (  ImageExist(t.terrain.imagestartindex+7) == 1  )  DeleteImage (  t.terrain.imagestartindex+7 );

	//  show editor water plane
	if (  ObjectExist(t.terrain.objectstartindex+2) == 1 ) 
	{
		if ( t.game.gameisexe == 1 )
			HideObject (  t.terrain.objectstartindex+2 );
		else
			ShowObject (  t.terrain.objectstartindex+2 );
	}
}

void terrain_updatewatermechanism ( void )
{
	//  water visiblity
	if ( t.hardwareinfoglobals.nowater == 0 ) lua_showwater ( );
	if ( t.hardwareinfoglobals.nowater != 0 ) { t.twf=t.hardwareinfoglobals.nowater ; lua_hidewater() ; t.hardwareinfoglobals.nowater=t.twf; }
	//  water physics
//terrain_updatewaterphysics ( );
return;

}

void terrain_updatewaterphysics ( void )
{
	// water physics
	if ( g.gphysicssessionactive == 1 ) 
	{
		if ( t.hardwareinfoglobals.nowater == 0 && t.visuals.reflectionmode>0 ) 
		{
			ODESetWaterLine ( t.terrain.waterliney_f - 20.0f );// 480.0 );
		}
		else
		{
			ODESetWaterLine ( -10000.0 );
		}
	}
}

void terrain_water_setfog ( void )
{
	//  takes; tFogR#,tFogG#,tFogB#,tFogNear#,tFogFar#
	if ( GetEffectExist(t.terrain.effectstartindex+1) ) 
	{
		SetVector4 (  g.terrainvectorindex,t.tFogR_f/255.0,t.tFogG_f/255.0,t.tFogB_f/255.0,t.tFogA_f/255.0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+1,t.effectparam.water.HudFogColor,g.terrainvectorindex );
		SetVector4 (  g.terrainvectorindex,t.tFogNear_f,t.tFogFar_f,0,0 );
		SetEffectConstantVEx (  t.terrain.effectstartindex+1,t.effectparam.water.HudFogDist,g.terrainvectorindex );
	}
}

void terrain_water_loop ( void )
{
	//  Adjust reflective processing based on actual number of water pixels in final scene
	t.visuals.reflectionmodepixelsrendered=0;
	if (  t.visuals.reflectionmode>0 ) 
	{
		if (  ObjectExist(t.terrain.objectstartindex+5) == 1 ) 
		{
			// DX11 had the query removed so cannot count pixels rendered (would also fix 'one cycle' no reflection issue)
			t.visuals.reflectionmodepixelsrendered=1;//GetObjectOcclusionValue(t.terrain.objectstartindex+5);
			if (  t.visuals.reflectionmodepixelsrendered>0 ) 
			{
				if (  t.visuals.reflectionmodemodified == 0 ) 
				{
					t.visuals.reflectionmodemodified=t.visuals.reflectionmode;
					visuals_updateobjectmasks ( );
				}
			}
			else
			{
				if (  t.visuals.reflectionmodemodified>0 ) 
				{
					t.visuals.reflectionmodemodified=0;
					visuals_updateobjectmasks ( );
				}
			}
		}
	}

	//  Update Water plain
	if (  ObjectExist(t.terrain.objectstartindex+5) == 1 && t.visuals.reflectionmodepixelsrendered>0 ) 
	{
		PositionObject (  t.terrain.objectstartindex+5,ObjectPositionX(t.terrain.objectstartindex+5),t.terrain.waterliney_f,ObjectPositionZ(t.terrain.objectstartindex+5) );
		t.terrain.WaterCamY_f=CameraPositionY()-t.terrain.waterliney_f;
		//  Refraction camera (looks bad with stuff floating in it)
		HideObject (  t.terrain.objectstartindex+5 );
		//  Reflection camera
		if (  t.visuals.reflectionmode>0 ) 
		{
			//  special render technique for terrain reflection
			SetEffectTechnique ( t.terrain.effectstartindex+1, "UseReflection" );

			//  set sky position to be relative to reflection render
			PositionObject (  t.terrain.objectstartindex+4,CameraPositionX(),t.terrain.waterliney_f-t.terrain.WaterCamY_f,CameraPositionZ() );
			if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  PositionObject (  t.terrain.objectstartindex+8,CameraPositionX(),t.terrain.waterliney_f-t.terrain.WaterCamY_f,CameraPositionZ() );
			PositionCamera (  2,CameraPositionX(),t.terrain.waterliney_f-t.terrain.WaterCamY_f,CameraPositionZ() );
			RotateCamera (  2,-CameraAngleX(),CameraAngleY(),CameraAngleZ() );
			//  only render terrain/objects if mode allows
			if (  t.visuals.reflectionmode>25 ) 
			{
				//  full reflection mode renders terrain
				if (  t.terrain.WaterCamY_f>0 ) 
				{
					SetCameraClip (  2,1,0,t.terrain.waterliney_f-0.0,0,0,1,0 );
				}
				else
				{
					SetCameraClip (  2,1,0,t.terrain.waterliney_f+50.0,0,0,-1,0 );
				}
				//  if terrain exists, render it
				if (  t.terrain.TerrainID>0 ) 
				{
					BT_SetCurrentCamera (  2 );
					BT_SetTerrainLODDistance (  t.terrain.TerrainID,1,700.0 );
					BT_SetTerrainLODDistance (  t.terrain.TerrainID,2,701.0 );
					BT_UpdateTerrainLOD (  t.terrain.TerrainID );
					BT_UpdateTerrainCull (  t.terrain.TerrainID );
					BT_RenderTerrain (  t.terrain.TerrainID );
					BT_SetCurrentCamera (  0 );
					BT_SetTerrainLODDistance (  t.terrain.TerrainID,1,1401.0+t.visuals.TerrainLOD1_f );
					BT_SetTerrainLODDistance (  t.terrain.TerrainID,2,1401.0+t.visuals.TerrainLOD2_f );
				}
			}
			else
			{
				//  no terrain in reflection render
			}
			if (t.visuals.reflectionmode == 1) {
				//PE: Special mode that only clear the reflection image mainly for underwater.
				SetCameraClip(2, 1, 0, t.terrain.waterliney_f - 100000.0, 0, 0, -1, 0);
			}
			SyncMask (  1<<2 );

			//  simpler terrain for reflection render
			if (  GetEffectExist(t.terrain.terrainshaderindex) == 1  )  SetEffectTechnique (  t.terrain.terrainshaderindex,"ReflectedOnly" );
			FastSync (  );
			//  restore terrain shader technique
			if ( GetEffectExist(t.terrain.terrainshaderindex) == 1  ) visuals_shaderlevels_terrain_update ( );
			//  restore sky position to main camera
			PositionObject (  t.terrain.objectstartindex+4,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
			if (  ObjectExist(t.terrain.objectstartindex+8) == 1  )  PositionObject (  t.terrain.objectstartindex+8,CameraPositionX(),CameraPositionY(),CameraPositionZ() );
		}
		else
		{
			SetEffectTechnique ( t.terrain.effectstartindex+1, "NoReflection" );
		}
	}

	//  Restore regular rendering
	SyncMask (  0xfffffff9 );

	//  Show water again for main rendering
	if (  t.hardwareinfoglobals.nowater == 0 ) 
	{
		if (  ObjectExist(t.terrain.objectstartindex+5) == 1 ) 
		{
			ShowObject (  t.terrain.objectstartindex+5 );
		}
	}
}

// 
//  VEGEATATION SYSTEM
// 

void terrain_fastveg_assignnewshader ( void )
{
	// Choose the vegetation shader to use
	//if ( g.gpbroverride == 1 ) // t.terrain.iTerrainPBRMode == 1 )
	//	t.terrain.vegetationshaderindex = t.terrain.effectstartindex+6;
	//else
	//	t.terrain.vegetationshaderindex = t.terrain.effectstartindex+2;
	t.terrain.vegetationshaderindex = t.terrain.effectstartindex+2;

	// Veg shader constants
	memset ( &t.effectparam.vegetation, 0, sizeof(t.effectparam.vegetation) );
	if ( GetEffectExist ( t.terrain.vegetationshaderindex ) == 1 )
	{
		t.effectparam.vegetation.g_lights_data=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_data");
		t.effectparam.vegetation.g_lights_pos0=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_pos0");
		t.effectparam.vegetation.g_lights_atten0=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_atten0");
		t.effectparam.vegetation.g_lights_diffuse0=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_diffuse0");
		t.effectparam.vegetation.g_lights_pos1=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_pos1");
		t.effectparam.vegetation.g_lights_atten1=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_atten1");
		t.effectparam.vegetation.g_lights_diffuse1=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_diffuse1");
		t.effectparam.vegetation.g_lights_pos2=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_pos2");
		t.effectparam.vegetation.g_lights_atten2=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_atten2");
		t.effectparam.vegetation.g_lights_diffuse2=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"g_lights_diffuse2");
		t.effectparam.vegetation.SpotFlashPos=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"SpotFlashPos");
		t.effectparam.vegetation.SpotFlashColor=GetEffectParameterIndex(t.terrain.vegetationshaderindex,"SpotFlashColor");

		// wipe any previous param storage
		ResetEffect ( t.terrain.vegetationshaderindex );
	}
}

void terrain_fastveg_applyshader ( void )
{
	// Choose the vegetation shader to use
	terrain_fastveg_assignnewshader();

	// Apply veg shader to all veg objects
	UpdateGrassTexture ( g.gpbroverride );//t.terrain.iTerrainPBRMode );
	UpdateGrassShader ( t.terrain.vegetationshaderindex );
}

void terrain_fastveg_init ( void )
{
	//  init our resource values
	t.tGrassObj=t.terrain.objectstartindex+6;
	t.tGrassImg=t.terrain.imagestartindex+8;
	t.tObjectGridStart = t.terrain.objectstartindex + 6201;
	t.terrain.vegetationshaderindex=t.terrain.effectstartindex+2;
	t.tVegShadowTex = t.terrain.imagestartindex+2;
	t.tTempMesh = t.terrain.objectstartindex+7;

	// load shaders for non-PBR and PBR vegetation
	if ( GetEffectExist ( t.terrain.effectstartindex+2 ) == 0 )
	{
		//LoadEffect (  "effectbank\\reloaded\\apbr_veg.fx", t.terrain.effectstartindex+6, 0 );
		if ( g.gpbroverride == 1 )
			LoadEffect (  "effectbank\\reloaded\\apbr_veg.fx", t.terrain.effectstartindex+2, 0 );
		else
			LoadEffect (  "effectbank\\reloaded\\vegetation_basic.fx", t.terrain.effectstartindex+2, 0 );
	}

	// PBR Support textures
	int iPBRAGEDImg = t.terrain.imagestartindex+14;
	int iPBRSpecImg = t.terrain.imagestartindex+15;
	int iPBRCubeImg = t.terrain.imagestartindex+31;
	int iPBRCurveImg = t.terrain.imagestartindex+32;
	if ( ImageExist ( t.terrain.imagestartindex+14 ) == 0 ) LoadImage ( "effectbank\\reloaded\\media\\AGED.png",t.terrain.imagestartindex+14 );
	if ( ImageExist ( t.terrain.imagestartindex+15 ) == 0 ) LoadImage ( "effectbank\\reloaded\\media\\blank_black.dds",t.terrain.imagestartindex+15 );
	SetPBRResourceValues ( iPBRAGEDImg, iPBRSpecImg, iPBRCubeImg, iPBRCurveImg );

	//  Pass the veg system the following initialisation values first. These are just resource numbers and numerical settings;
	//  1. Grass object/mesh ID for base grass model
	//  2. Start object for grid of grass models (ensure grid size doesnt exist available object numbers when x2)
	//  3. Grass texture
	//  4. Shadow texture
	//  5. A temporary mesh used for building grass objects
	//  6. Shader ID
	//  7. Grass existance memblock
	//  8. Memblock width/height
	//  9. Camera mask for veg objects
	//C++ ISSUE %001 replaced with 1 - should be fine since %001 in binary is 1 in dec
	SetResourceValues (  t.tGrassObj,t.tObjectGridStart,t.tGrassImg,t.tVegShadowTex,t.tTempMesh,t.terrain.vegetationshaderindex,t.terrain.grassmemblock,MAXTEXTURESIZE, 1 );

	//  now load our grass mesh
	if (  ObjectExist(t.tGrassObj)  ==  1  )  DeleteObject (  t.tGrassObj );
	if (  GetMeshExist(t.tGrassObj)  ==  1  )  DeleteMesh (  t.tGrassObj );

	cstr pFileToLoad = cstr(cstr("vegbank\\")+g.vegstyle_s+"\\veg.DBO");
	if ( FileExist ( pFileToLoad.Get() ) == 0 ) pFileToLoad = cstr(cstr("vegbank\\")+g.vegstyle_s+"\\veg.X");
	LoadObject ( pFileToLoad.Get(),t.tGrassObj );

	ScaleObject (  t.tGrassObj,200,200,200 );
	MakeMeshFromObject (  t.tGrassObj,t.tGrassObj );
	DeleteObject (  t.tGrassObj );

	//  load our grass piece image
	terrain_fastveg_setgrassimage ( );

	// apply correct shader
	terrain_fastveg_applyshader ();

	//  if the user has updated the grass bitmap in the editor and the init function has been called, we are testing a level
	//  and a new grass memblock file needs to be made from the bitmap. Otherwise there should be one to load
	t.tfileveggrass_s=g.mysystem.levelBankTestMap_s+"vegmaskgrass.dat"; //"levelbank\\testmap\\vegmaskgrass.dat";
	if (  t.terrain.grassregionx1 != t.terrain.grassregionx2 || t.terrain.grassregionupdate == 2 || FileExist(t.tfileveggrass_s.Get()) == 0 ) 
	{
		terrain_fastveg_updategrassfrombitmap ( );
	}
	else
	{
		terrain_fastveg_loadgrass ( );
	}

	//  calculate how big our grid of vegetation is
	terrain_fastveg_setgrassgridandfade ( );

	//  now create and setup all of our vegetation objects. This will delete any veg that already exists. Passing;
	//  1. Number of grass items per veg object
	//  2. Width of grass
	//  3. Height of grass
	//  4. Size of each veg area
	//  5. Dimension of the veg grid
	if (  t.terrain.superflat == 1 ) 
	{
		t.tTerrainID = 0;
	}
	else
	{
		t.tTerrainID = t.terrain.TerrainID;
	}
	int iTrimUsingGrassMemblock = 0;
	if ( t.game.gameisexe == 1 ) iTrimUsingGrassMemblock = t.terrain.grassmemblock;
	MakeVegetationGrid ( 4.0f*t.visuals.VegQuantity_f,t.visuals.VegWidth_f,t.visuals.VegHeight_f,terrain_veg_areawidth,t.terrain.vegetationgridsize,t.tTerrainID, iTrimUsingGrassMemblock );
}

void terrain_fastveg_setgrassimage ( void )
{
	SetImageAutoMipMap (  1 );
	t.tGrassImg=t.terrain.imagestartindex+8;
	if ( ImageExist(t.tGrassImg)  ==  1  )  DeleteImage (  t.tGrassImg );
	if ( g.gdividetexturesize == 0 ) 
	{
		LoadImage ( "effectbank\\reloaded\\media\\white_D.dds",t.tGrassImg );
	}
	else
	{
		LoadImage ( cstr(cstr("vegbank\\")+g.vegstyle_s+"\\grass.dds").Get(),t.tGrassImg );
	}
	UpdateGrassTexture ( g.gpbroverride );//t.terrain.iTerrainPBRMode );
	SetImageAutoMipMap ( 0 );
}

void terrain_fastveg_setgrassgridandfade ( void )
{
	t.terrain.vegetationgridsize = t.visuals.vegetationmode * 0.3;
	t.tGrassFadeDistance = terrain_veg_areawidth * (t.terrain.vegetationgridsize/2 - 1);
}

void terrain_fastveg_loop ( void )
{
	//  early exit if no veg used
	if (  t.visuals.vegetationmode == 0  )  return;

	//  if in superflat mode we call a version of update which is much faster
	if (  t.terrain.superflat == 1 ) 
	{
			UpdateSuperFlat (  CameraPositionX(0),CameraPositionZ(0),TERRAIN_SUPERFLAT_HEIGHT,g.postprocessimageoffset+5 );
	}
	else
	{

		//  do we need to update after an F9 terrain raise?
		if (  t.terrain.grassupdateafterterrain  ==  1 ) 
		{
			t.tRegionX1 = t.terrain.grassregionx1 * 50;
			t.tRegionX2 = t.terrain.grassregionx2 * 50;
			t.tRegionZ1 = t.terrain.grassregionz1 * 50;
			t.tRegionZ2 = t.terrain.grassregionz2 * 50;

			//  only update when MouseClick (  isn't pressed or cursor moves, to reduce chug slowdown )
			if (  MouseClick()  ==  0 || t.tRegionX1  !=  t.terrain.lastgrassupdatex1 || t.tRegionX2  !=  t.terrain.lastgrassupdatex2 || t.tRegionZ1  !=  t.terrain.lastgrassupdatez1 || t.tRegionZ2  !=  t.terrain.lastgrassupdatez2 ) 
			{
				UpdateVegZoneBlitzTerrain (  t.tRegionX1,t.tRegionZ1,t.tRegionX2,t.tRegionZ2,t.terrain.TerrainID );
				t.terrain.lastgrassupdatex1 = t.tRegionX1;
				t.terrain.lastgrassupdatex2 = t.tRegionX2;
				t.terrain.lastgrassupdatez1 = t.tRegionZ1;
				t.terrain.lastgrassupdatez2 = t.tRegionZ2;
				t.terrain.grassupdateafterterrain = 0;
			}
		}

		//  main terrain update
		if (  t.hardwareinfoglobals.nograss == 0 ) 
		{
			UpdateBlitzTerrain (  CameraPositionX(0),CameraPositionZ(0),t.terrain.TerrainID,g.postprocessimageoffset+5 );
		}

	}
}

void terrain_fastveg_clearregion ( void )
{
	//  clear the grass memblock in the described terrain region
	t.tvegareax1=((t.terrain.terrainregionx1+0.0)/5)-1;
	t.tvegareax2=((t.terrain.terrainregionx2+0.0)/5)+1;
	t.tvegareaz1=((t.terrain.terrainregionz1+0.0)/5)-1;
	t.tvegareaz2=((t.terrain.terrainregionz2+0.0)/5)+1;
	if (  t.tvegareax1<0  )  t.tvegareax1 = 0;
	if (  t.tvegareax2>MAXTEXTURESIZE-1  )  t.tvegareax2 = MAXTEXTURESIZE - 1;
	if (  t.tvegareaz1<0  )  t.tvegareaz1 = 0;
	if (  t.tvegareaz2>MAXTEXTURESIZE-1  )  t.tvegareaz2 = MAXTEXTURESIZE - 1;
	for ( t.tvz = t.tvegareaz1 ; t.tvz<=  t.tvegareaz2; t.tvz++ )
	{
		t.tZStep = t.tvz*MAXTEXTURESIZE;
		for ( t.tvx = t.tvegareax1 ; t.tvx<=  t.tvegareax2; t.tvx++ )
		{
			WriteMemblockByte (  t.terrain.grassmemblock,(4+4+4+((t.tvx+t.tZStep)*4))+2,0 );
		}
	}
}

void terrain_fastveg_updatedirtyregionfast ( void )
{
	//  scan vegmap image and extract grass values into veggrass memblock
	SetCurrentBitmap (  g.terrainworkbitmapindex );
//  `paste image terrain.imagestartindex+1,0,0

	PasteImage (  t.terrain.imagestartindex+2,0,0 );
	LockPixels (  );
	//  *2 as the veggrass aray is 2048 vs 1024 terraingrid coords
	t.tValue = t.terrain.grassregionz1*2 ; terrain_fastveg_clamptomemblockres() ; t.tBack = t.tValue;
	t.tValue = t.terrain.grassregionz2*2 ; terrain_fastveg_clamptomemblockres() ; t.tFront = t.tValue;
	t.tValue = t.terrain.grassregionx1*2 ; terrain_fastveg_clamptomemblockres() ; t.tLeft = t.tValue;
	t.tValue = t.terrain.grassregionx2*2 ; terrain_fastveg_clamptomemblockres() ; t.tRight = t.tValue;
	for ( t.tvz = t.tBack ; t.tvz<=  t.tFront; t.tvz++ )
	{
		t.tZStep = t.tvz*MAXTEXTURESIZE;
		for ( t.tvx = t.tLeft ; t.tvx<=  t.tRight; t.tvx++ )
		{
			WriteMemblockByte (  t.terrain.grassmemblock,(4+4+4+((t.tvx+t.tZStep)*4))+2,RgbR(GetPoint(t.tvx,t.tvz)) );
		}
	}
	UnlockPixels (  );
	SetCurrentBitmap (  0 );
	t.tRegionX1 = t.terrain.grassregionx1 * 50;
	t.tRegionX2 = t.terrain.grassregionx2 * 50;
	t.tRegionZ1 = t.terrain.grassregionz1 * 50;
	t.tRegionZ2 = t.terrain.grassregionz2 * 50;
	if (  t.tRegionX1  !=  t.terrain.lastgrassupdatex1 || t.tRegionX2  !=  t.terrain.lastgrassupdatex2 || t.tRegionZ1  !=  t.terrain.lastgrassupdatez1 || t.tRegionZ2  !=  t.terrain.lastgrassupdatez2 ) 
	{
		if (  t.terrain.superflat  ==  1 ) 
		{
			UpdateVegZoneSuperFlat (  t.tRegionX1,t.tRegionZ1,t.tRegionX2,t.tRegionZ2,TERRAIN_SUPERFLAT_HEIGHT );
		}
		else
		{
			UpdateVegZoneBlitzTerrain (  t.tRegionX1,t.tRegionZ1,t.tRegionX2,t.tRegionZ2,t.terrain.TerrainID );
		}
		t.terrain.lastgrassupdatex1 = t.tRegionX1;
		t.terrain.lastgrassupdatex2 = t.tRegionX2;
		t.terrain.lastgrassupdatez1 = t.tRegionZ1;
		t.terrain.lastgrassupdatez2 = t.tRegionZ2;
	}
}

void terrain_fastveg_clamptomemblockres ( void )
{
	//  clamps input value tValue to the memblock resolution
	if (  t.tValue < 0  )  t.tValue  =  0;
	if (  t.tValue  >=  MAXTEXTURESIZE  )  t.tValue  =  MAXTEXTURESIZE - 1;
}

void terrain_fastveg_updategrassfrombitmap ( void )
{
	// if no image, must have had a VIDMEM reset, leave quietly
	if ( ImageExist(t.terrain.imagestartindex+2) == 0  )  return;

	// uses raw image data inside VEG module
	if ( MemblockExist(t.terrain.grassmemblock) == 1  )  DeleteMemblock (  t.terrain.grassmemblock );
	CreateMemblockFromImage ( t.terrain.grassmemblock, t.terrain.imagestartindex+2 );
	
	//this was only required because images are loading in RGBA, when they should be BGRA (as it was with DX9!!)
	//ConvertVegMemblock ( t.terrain.grassmemblock );

	// slopes and water can't exist in superflat mode, so only delete invalid grass in terrain mode
	if ( t.terrain.superflat == 0  )  DeleteInvalidGrass (  t.terrain.TerrainID,t.terrain.waterliney_f,1.0 );
}

void terrain_fastveg_loadgrass ( void )
{
	//  load grass data memblock
	if (  FileExist(t.tfileveggrass_s.Get()) == 1 ) 
	{
		OpenToRead (  3,t.tfileveggrass_s.Get() );
		if (  MemblockExist(t.terrain.grassmemblock)  )  DeleteMemblock (  t.terrain.grassmemblock );
		ReadMemblock (  3,t.terrain.grassmemblock );
		CloseFile (  3 );

		//  151214 - if old grass memblock, reconstruct as new raw image grass memblock
		if (  ReadMemblockDWord(t.terrain.grassmemblock,8) != 32 ) 
		{
			DeleteMemblock (  t.terrain.grassmemblock );
			MakeMemblock (  t.terrain.grassmemblock,4+4+4+((MAXTEXTURESIZE*MAXTEXTURESIZE)*4) );
			WriteMemblockDWord (  t.terrain.grassmemblock,0,MAXTEXTURESIZE );
			WriteMemblockDWord (  t.terrain.grassmemblock,4,MAXTEXTURESIZE );
			WriteMemblockDWord (  t.terrain.grassmemblock,8,32 );
		}

		//  This helper call removes all grass entries in the grass memblock where the grass exists on a steep slope or under water.
		//  It can be called at any time when slope grass needs to be removed, but SetResourceValues (  must have been called earlier. )
		//  It is quite slow because it cycles through several million memblock entries and makes ground height calcs for each!
		//  Params; terrain ID, waterheight, max height difference over 1 unit
		if (  t.terrain.superflat == 0  )  DeleteInvalidGrass (  t.terrain.TerrainID,t.terrain.waterliney_f,1.0 );
	}
	else
	{
		terrain_fastveg_buildblankgrass ( );
	}
}

void terrain_fastveg_savegrass ( void )
{
	//  regenerate the memblock from the vegmask bitmap for consistency
	t.terrain.grassregionupdate=0;
	terrain_fastveg_updategrassfrombitmap ( );

	//  save grass memblock to disk
	if (  FileExist(t.tfileveggrass_s.Get()) == 1  )  DeleteAFile (  t.tfileveggrass_s.Get() );
	OpenToWrite (  3,t.tfileveggrass_s.Get() );
	WriteMemblock (  3,t.terrain.grassmemblock );
	CloseFile (  3 );
}

void terrain_fastveg_buildblankgrass ( void )
{
	//  make a blank grass data memblock, or clear the one that already exists
	//  151214 - wrong format but VEH module deals with nonraw-image memblocks as ZERO
	if (  MemblockExist(t.terrain.grassmemblock)  ==  0 ) 
	{
		MakeMemblock (  t.terrain.grassmemblock,4+4+4+((MAXTEXTURESIZE*MAXTEXTURESIZE)*4) );
	}
	WriteMemblockDWord (  t.terrain.grassmemblock,0,MAXTEXTURESIZE );
	WriteMemblockDWord (  t.terrain.grassmemblock,4,MAXTEXTURESIZE );
	WriteMemblockDWord (  t.terrain.grassmemblock,8,32 );
	t.tPindex=4+4+4;
	for ( t.tP = 0 ; t.tP<=  MAXTEXTURESIZE*MAXTEXTURESIZE - 1; t.tP++ )
	{
		WriteMemblockByte (  t.terrain.grassmemblock,t.tPindex+2,0 );
		t.tPindex += 4;
	}
	terrain_fastveg_savegrass ( );
}

void terrain_fastveg_buildblankgrass_fornew ( void )
{

	//  Create new memblock for grass
	if (  MemblockExist(t.terrain.grassmemblock) == 1  )  DeleteMemblock (  t.terrain.grassmemblock );
	if (  MemblockExist(t.terrain.grassmemblock) == 0 ) 
	{
		MakeMemblock (  t.terrain.grassmemblock,4+4+4+((MAXTEXTURESIZE*MAXTEXTURESIZE)*4) );
		WriteMemblockDWord (  t.terrain.grassmemblock,0,MAXTEXTURESIZE );
		WriteMemblockDWord (  t.terrain.grassmemblock,4,MAXTEXTURESIZE );
		WriteMemblockDWord (  t.terrain.grassmemblock,8,32 );
	}

	//  save grass memblock to disk
	//  151214 - wrong format but VEH module deals with nonraw-image memblocks as ZERO
	if (  FileExist(t.tfileveggrass_s.Get()) == 1  )  DeleteAFile (  t.tfileveggrass_s.Get() );
	OpenToWrite (  3,t.tfileveggrass_s.Get() );
		WriteMemblock (  3,t.terrain.grassmemblock );
	CloseFile (  3 );

return;

}

void terrain_fastveg_free ( void )
{
	//  We used to DeleteVegetationGrid (  here to clear away veg. Now to save time during testing we HideVegetationGrid (  instead. ) )
	HideVegetationGrid (  );
}

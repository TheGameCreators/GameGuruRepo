//----------------------------------------------------
//--- GAMEGURU - M-Importer
//----------------------------------------------------

// Includes
#include "gameguru.h"

// Globals
bool g_bLoadedFBXModel = false;
int g_iFirstTimeFBXImport = 0;
int g_iPreferPBR = 0;
int g_iPreferPBRLateShaderChange = 0;
bool g_bCameraInSkyForImporter = false;
char g_pShowFilenameHoveringOver[1024];
DWORD g_dwMapImageListIndexToLimbCount = 0;
int* g_piMapImageListIndexToLimbIndex = NULL;

// Prototypes
void LoadFBX ( LPSTR pFilename, int iObjectNumber );

void importer_init ( void )
{
	// hide entities and waypoints
	importer_fade_out ( );
	for ( t.te = 1 ; t.te <= g.entityelementlist; t.te++ )
	{
		t.tobj=t.entityelement[t.te].obj;
		if ( t.tobj>0 ) 
		{
			if ( ObjectExist(t.tobj) == 1 ) HideObject ( t.tobj );
			int iCCObjHead = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+0;
			int iCCObjBeard = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+1;
			int iCCObjHat = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+2;
			if ( ObjectExist(iCCObjHead) == 1 ) HideObject ( iCCObjHead );
			if ( ObjectExist(iCCObjBeard) == 1 ) HideObject ( iCCObjBeard );
			if ( ObjectExist(iCCObjHat) == 1 ) HideObject ( iCCObjHat );
		}
	}
	waypoint_hideall ( );

	// need to see all editor entity objects wiped from screen
	Sync(); Sync();

	// init importer vars
	t.timportermouseoff = 0;
	t.tImporterOriginalScreenWidth = GetDisplayWidth();
	t.tImporterOriginalScreenHeight = GetDisplayHeight();

	//  Multiplier to convert mouse coords to importer coords
	t.tadjustedtoimporterxbase_f=GetChildWindowWidth()/800.0;
	t.tadjustedtoimporterybase_f=GetChildWindowHeight()/600.0;
	t.importer.scaleMulti = 1.0;

	Dim2(  t.snapPosX_f,50, 9  );
	Dim2(  t.snapPosY_f,50, 9  );
	Dim2(  t.snapPosZ_f,50, 9  );

	t.importer.cancel = 0;
	t.importer.cancelCount = 0;
	t.importer.oldMouseClick = t.inputsys.mclick;
	t.importer.reload = 0;
	t.importer.changeSizeKeyDown = 0;
	t.importer.showCollisionOnly = 0;
	t.importer.oldShowCollision = 0;

	//  grab initial folder
	t.importer.startDir = GetDir();

	importer_checkForShaderFiles ( );
	importer_checkForScriptFiles ( );

	t.importer.importerActive = 1;
	t.importer.showScaleChange = 0;
	t.importer.collisionObjectMode = 0;
	t.importer.buttonPressedCount = 0;

	for ( int tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		t.importerTextures[tCount].imageID = 0;
		t.importerTextures[tCount].spriteID = 0;
		t.importerTextures[tCount].spriteID2 = 0;
		t.importerTextures[tCount].fileName = "";
		t.importerTextures[tCount].iExpandedThisSlot = 0;	
		t.importerTextures[tCount].iOptionalStage = 0;
		t.importerTextures[tCount].iAssociatedBaseImage = 0;
	}

	t.importer.helpShow = 0;

	//  load resources unique to importer
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\paneltop.png",g.importermenuimageoffset+1,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\panelmiddle.png",g.importermenuimageoffset+2,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\panelbottom.png",g.importermenuimageoffset+3,1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\tab.png",g.importermenuimageoffset+4,0 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\tabselected.png",g.importermenuimageoffset+5,0 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\blankTex.png",g.importermenuimageoffset+6,0 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\texturePanel.png",g.importermenuimageoffset+7,0 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\help.png",g.importermenuimageoffset+8,0 );
	//offset 9 is used with help Text (  )
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\multiTex.png",g.importermenuimageoffset+10,0 );

	//  Help Icon
	t.tSpriteID = 200;

	//  Find free Sprite (  slot )
	while (  SpriteExist(t.tSpriteID) ) 
	{
		++t.tSpriteID;
	}
	t.importer.helpSprite = t.tSpriteID;
	if (  t.importer.scaleMulti > 1.0 ) 
	{
		Sprite (  t.importer.helpSprite , (GetChildWindowWidth()/2) - 170 + 128 , 0, g.importermenuimageoffset+8 );
	}
	else
	{
		Sprite (  t.importer.helpSprite , (GetChildWindowWidth()/2) - 170 + 128 , (GetChildWindowHeight()/2) - 300 - 4, g.importermenuimageoffset+8 );
	}
	SizeSprite (  t.importer.helpSprite , 32 , 32 );

	t.importer.helpSprite2 = t.tSpriteID+1;
	Sprite (  t.importer.helpSprite2 , (GetChildWindowWidth()/2) - 303 , (GetChildWindowHeight()/2) - 213, g.importermenuimageoffset+4 );
	SizeSprite (  t.importer.helpSprite2 , 605 , 435 );
	SetSpriteDiffuse (  t.importer.helpSprite2 , 0,0,0 );
	SetSpriteAlpha (  t.importer.helpSprite2, 200 );

	t.importer.helpSprite3 = t.tSpriteID+2;
	Sprite (  t.importer.helpSprite3 , (GetChildWindowWidth()/2) - 303 , (GetChildWindowHeight()/2) - 213, g.importermenuimageoffset+4 );
	SizeSprite (  t.importer.helpSprite3 , 605 , 40 );
	SetSpriteDiffuse (  t.importer.helpSprite3 , 0,0,0 );
	SetSpriteAlpha (  t.importer.helpSprite3, 150 );

	t.importer.helpSprite4 = t.tSpriteID+3;

	HideSprite (  t.importer.helpSprite2 );
	HideSprite (  t.importer.helpSprite3 );

	t.importer.oldTabMode = g.tabmode;
	g.tabmode = IMPORTERTABPAGE1;

	t.importer.viewMessage = "Front";

	// Check if panels already exist
	if ( t.importer.panelsExist ==  0 ) 
	{
		t.importer.panelsExist = 1;
		if (  t.importer.scaleMulti  <= 0  )  t.importer.scaleMulti  =  1.0;
		g.slidersmenumax = 30; // 020718 - start out of way of TAB TAB panels
		t.importer.properties1Index = g.slidersmenumax;
		t.slidersmenu[g.slidersmenumax].tabpage=IMPORTERTABPAGE1;
		t.slidersmenu[g.slidersmenumax].title_s="Properties";
		t.slidersmenu[g.slidersmenumax].thighlight=-1;
		t.slidersmenu[g.slidersmenumax].titlemargin=63;
		t.slidersmenu[g.slidersmenumax].leftmargin=25;
		t.slidersmenu[g.slidersmenumax].itemcount=12;
		t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
		t.slidersmenu[g.slidersmenumax].ttop= (GetChildWindowHeight() / 2 ) - 281 -3;
		t.slidersmenu[g.slidersmenumax].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenuvalue[g.slidersmenumax][1].name_s="Scale";
		t.slidersmenuvalue[g.slidersmenumax][1].value=100;
		t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=0;
		t.slidersmenuvalue[g.slidersmenumax][1].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMin = 19;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMax = 500;
		t.slidersmenuvalue[g.slidersmenumax][2].name_s="Shader";
		t.slidersmenuvalue[g.slidersmenumax][2].value=2;
		t.slidersmenuvalue[g.slidersmenumax][2].value_s=t.importerShaderFiles[2];
		t.slidersmenuvalue[g.slidersmenumax][2].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][2].gadgettypevalue=101;
		t.slidersmenuvalue[g.slidersmenumax][3].name_s="Y Rotation";
		t.slidersmenuvalue[g.slidersmenumax][3].value=0;
		t.slidersmenuvalue[g.slidersmenumax][3].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][3].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][3].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][4].name_s="Collision Mode";
		t.slidersmenuvalue[g.slidersmenumax][4].value=1;
		t.slidersmenuvalue[g.slidersmenumax][4].value_s="Box";
		t.slidersmenuvalue[g.slidersmenumax][4].gadgettypevalue=110;
		t.slidersmenuvalue[g.slidersmenumax][4].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][5].name_s="Default Static";
		t.slidersmenuvalue[g.slidersmenumax][5].value=1;
		t.slidersmenuvalue[g.slidersmenumax][5].value_s="Yes";
		t.slidersmenuvalue[g.slidersmenumax][5].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][5].gadgettypevalue=103;
		t.slidersmenuvalue[g.slidersmenumax][6].name_s="Strength";
		t.slidersmenuvalue[g.slidersmenumax][6].value=25;
		t.slidersmenuvalue[g.slidersmenumax][6].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][6].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][6].valueMax = 1000;
		t.slidersmenuvalue[g.slidersmenumax][7].name_s="Is A Character";
		t.slidersmenuvalue[g.slidersmenumax][7].value=1;
		t.slidersmenuvalue[g.slidersmenumax][7].value_s="No";
		t.slidersmenuvalue[g.slidersmenumax][7].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][7].gadgettypevalue=117;
		t.slidersmenuvalue[g.slidersmenumax][8].name_s="Cull Mode";
		t.slidersmenuvalue[g.slidersmenumax][8].value=2;
		t.slidersmenuvalue[g.slidersmenumax][8].value_s="No";
		t.slidersmenuvalue[g.slidersmenumax][8].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][8].gadgettypevalue=103;
		t.slidersmenuvalue[g.slidersmenumax][9].name_s="Transparency";
		t.slidersmenuvalue[g.slidersmenumax][9].value=1;
		t.slidersmenuvalue[g.slidersmenumax][9].value_s="None";
		t.slidersmenuvalue[g.slidersmenumax][9].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][9].gadgettypevalue=111;
		t.slidersmenuvalue[g.slidersmenumax][10].name_s="Material";
		t.slidersmenuvalue[g.slidersmenumax][10].value=1;
		t.slidersmenuvalue[g.slidersmenumax][10].value_s="Generic";
		t.slidersmenuvalue[g.slidersmenumax][10].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][10].gadgettypevalue=112;
		t.slidersmenuvalue[g.slidersmenumax][11].name_s="Script";
		t.slidersmenuvalue[g.slidersmenumax][11].value=1;
		t.slidersmenuvalue[g.slidersmenumax][11].value_s=t.importerScriptFiles[1];
		t.slidersmenuvalue[g.slidersmenumax][11].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][11].gadgettypevalue=114;
		t.slidersmenuvalue[g.slidersmenumax][12].name_s="Height Guide";
		t.slidersmenuvalue[g.slidersmenumax][12].value=1;
		t.slidersmenuvalue[g.slidersmenumax][12].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][12].gadgettype=99;
		//t.slidersmenuvalue[g.slidersmenumax][13].name_s="Geo Twizzle";
		//t.slidersmenuvalue[g.slidersmenumax][13].value=1+g_iFBXGeometryToggleMode;
		//t.slidersmenuvalue[g.slidersmenumax][13].gadgettype=1;
		//t.slidersmenuvalue[g.slidersmenumax][13].gadgettypevalue=115;
		//t.slidersmenuvalue[g.slidersmenumax][14].name_s="Center Mesh";
		//t.slidersmenuvalue[g.slidersmenumax][14].value=1+g_iFBXGeometryCenterMesh;
		//t.slidersmenuvalue[g.slidersmenumax][14].gadgettype=1;
		//t.slidersmenuvalue[g.slidersmenumax][14].gadgettypevalue=116;
		for ( int n = 0; n < 13; n++ )
		{
			// prevent scale and rotate sliders being disabled for clickchange
			t.slidersmenuvalue[g.slidersmenumax][n].expanddetect = 0;
		}

		++g.slidersmenumax;
		t.importer.properties2Index = g.slidersmenumax;
		t.slidersmenu[g.slidersmenumax].tabpage=IMPORTERTABPAGE2;
		t.slidersmenu[g.slidersmenumax].title_s="Collision";
		t.slidersmenu[g.slidersmenumax].thighlight=-1;
		t.slidersmenu[g.slidersmenumax].titlemargin=63;
		t.slidersmenu[g.slidersmenumax].leftmargin=25;
		t.slidersmenu[g.slidersmenumax].itemcount=12;
		t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
		t.slidersmenu[g.slidersmenumax].ttop= (GetChildWindowHeight() / 2 ) - 281-3;
		t.slidersmenu[g.slidersmenumax].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenuvalue[g.slidersmenumax][1].name_s="View";
		t.slidersmenuvalue[g.slidersmenumax][1].value=1;
		t.slidersmenuvalue[g.slidersmenumax][1].value_s="Front";
		t.slidersmenuvalue[g.slidersmenumax][1].gadgettype=1;
		t.slidersmenuvalue[g.slidersmenumax][1].gadgettypevalue=109;

		t.slidersmenuvalue[g.slidersmenumax][2].name_s="New";
		t.slidersmenuvalue[g.slidersmenumax][2].value=1;
		t.slidersmenuvalue[g.slidersmenumax][2].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][2].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][3].name_s="Duplicate";
		t.slidersmenuvalue[g.slidersmenumax][3].value=1;
		t.slidersmenuvalue[g.slidersmenumax][3].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][3].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][4].name_s="Delete";
		t.slidersmenuvalue[g.slidersmenumax][4].value=1;
		t.slidersmenuvalue[g.slidersmenumax][4].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][4].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][5].name_s="Next";
		t.slidersmenuvalue[g.slidersmenumax][5].value=1;
		t.slidersmenuvalue[g.slidersmenumax][5].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][5].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][6].name_s="Previous";
		t.slidersmenuvalue[g.slidersmenumax][6].value=1;
		t.slidersmenuvalue[g.slidersmenumax][6].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][6].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][7].name_s="Rotate X";
		t.slidersmenuvalue[g.slidersmenumax][7].value=0;
		t.slidersmenuvalue[g.slidersmenumax][7].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][7].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][7].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][7].readmodeindex=0;

		t.slidersmenuvalue[g.slidersmenumax][8].name_s="Rotate Y";
		t.slidersmenuvalue[g.slidersmenumax][8].value=0;
		t.slidersmenuvalue[g.slidersmenumax][8].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][8].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][8].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][8].readmodeindex=0;

		t.slidersmenuvalue[g.slidersmenumax][9].name_s="Rotate Z";
		t.slidersmenuvalue[g.slidersmenumax][9].value=0;
		t.slidersmenuvalue[g.slidersmenumax][9].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][9].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][9].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][9].readmodeindex=0;

		t.slidersmenuvalue[g.slidersmenumax][10].name_s="";
		t.slidersmenuvalue[g.slidersmenumax][10].value=1;
		t.slidersmenuvalue[g.slidersmenumax][10].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][10].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][11].name_s="";
		t.slidersmenuvalue[g.slidersmenumax][11].value=1;
		t.slidersmenuvalue[g.slidersmenumax][11].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][11].gadgettype=99;

		t.slidersmenuvalue[g.slidersmenumax][12].name_s="";
		t.slidersmenuvalue[g.slidersmenumax][12].value=1;
		t.slidersmenuvalue[g.slidersmenumax][12].value_s="";
		t.slidersmenuvalue[g.slidersmenumax][12].gadgettype=99;

		++g.slidersmenumax;
		t.importer.properties3Index = g.slidersmenumax;
		t.slidersmenu[g.slidersmenumax].tabpage=IMPORTERTABPAGE3;
		t.slidersmenu[g.slidersmenumax].title_s="Thumbnail";
		t.slidersmenu[g.slidersmenumax].thighlight=-1;
		t.slidersmenu[g.slidersmenumax].titlemargin=63;
		t.slidersmenu[g.slidersmenumax].leftmargin=25;
		t.slidersmenu[g.slidersmenumax].itemcount=12;
		t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
		t.slidersmenu[g.slidersmenumax].ttop= (GetChildWindowHeight() / 2 ) - 281-3;
		t.slidersmenu[g.slidersmenumax].tleft= (GetChildWindowWidth() / 2 ) + 330;

		t.slidersmenuvalue[g.slidersmenumax][1].name_s="Rotate X";
		t.slidersmenuvalue[g.slidersmenumax][1].value=0;
		t.slidersmenuvalue[g.slidersmenumax][1].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex=0;

		t.slidersmenuvalue[g.slidersmenumax][2].name_s="Rotate Y";
		t.slidersmenuvalue[g.slidersmenumax][2].value=0;
		t.slidersmenuvalue[g.slidersmenumax][2].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][2].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][2].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex=0;

		t.slidersmenuvalue[g.slidersmenumax][3].name_s="Rotate Z";
		t.slidersmenuvalue[g.slidersmenumax][3].value=0;
		t.slidersmenuvalue[g.slidersmenumax][3].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][3].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][3].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][3].readmodeindex=0;

		for ( t.temp = 4 ; t.temp<=  12; t.temp++ )
		{
			t.slidersmenuvalue[g.slidersmenumax][t.temp].name_s="";
			t.slidersmenuvalue[g.slidersmenumax][t.temp].value=Rnd(100);
			t.slidersmenuvalue[g.slidersmenumax][t.temp].gadgettype=99;
		}

		++g.slidersmenumax;
		t.importer.properties4Index = g.slidersmenumax;
		t.slidersmenu[g.slidersmenumax].tabpage=IMPORTERTABPAGE4;
		t.slidersmenu[g.slidersmenumax].title_s="SETTINGS 4";
		t.slidersmenu[g.slidersmenumax].thighlight=-1;
		t.slidersmenu[g.slidersmenumax].titlemargin=63;
		t.slidersmenu[g.slidersmenumax].leftmargin=25;
		t.slidersmenu[g.slidersmenumax].itemcount=11;
		t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
		t.slidersmenu[g.slidersmenumax].ttop= (GetChildWindowHeight() / 2 ) - 281;
		t.slidersmenu[g.slidersmenumax].tleft= (GetChildWindowWidth() / 2 ) + 330;
		for ( t.temp = 1 ; t.temp<=  11; t.temp++ )
		{
			t.slidersmenuvalue[g.slidersmenumax][t.temp].name_s = "" ; t.slidersmenuvalue[g.slidersmenumax][t.temp].name_s=t.slidersmenuvalue[g.slidersmenumax][t.temp].name_s+"Stuff " + Str(t.temp);
			t.slidersmenuvalue[g.slidersmenumax][t.temp].value=Rnd(100);
			t.slidersmenuvalue[g.slidersmenumax][t.temp].readmodeindex=t.temp;
		}

		t.importerTabs[1].x = (GetChildWindowWidth() / 2) + 65-128 + 250;
		t.importerTabs[1].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[1].label = "Properties";
		t.importerTabs[1].selected = 1;
		t.importerTabs[1].tabpage = IMPORTERTABPAGE1;

		t.importerTabs[2].x = t.importerTabs[1].x + 128;
		t.importerTabs[2].y = t.importerTabs[1].y;
		t.importerTabs[2].label = "Collision";
		t.importerTabs[2].selected = 0;
		t.importerTabs[2].tabpage = IMPORTERTABPAGE2;

		t.importerTabs[3].x = t.importerTabs[2].x + 128;
		t.importerTabs[3].y = t.importerTabs[1].y;
		t.importerTabs[3].label = "Thumbnail";
		t.importerTabs[3].selected = 0;
		t.importerTabs[3].tabpage = IMPORTERTABPAGE3;

		t.importerTabs[4].x = t.importerTabs[3].x + 256;
		t.importerTabs[4].y = t.importerTabs[1].y;
		t.importerTabs[4].label = "...";
		t.importerTabs[4].selected = 0;
		t.importerTabs[4].tabpage = IMPORTERTABPAGE4;

		//  Importer button
		t.importerTabs[5].x = (GetChildWindowWidth() / 2) - 320;
		t.importerTabs[5].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[5].label = "Save Entity";
		t.importerTabs[5].selected = 0;
		t.importerTabs[5].tabpage = -1;

		//  New button
		t.importerTabs[6].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[6].y = t.importerTabs[1].y + 125;
		t.importerTabs[6].label = "Add New";
		t.importerTabs[6].selected = 0;
		t.importerTabs[6].tabpage = -1;

		//  Dupe button
		t.importerTabs[12].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[12].y = t.importerTabs[1].y + 125 + 38;
		t.importerTabs[12].label = "Duplicate";
		t.importerTabs[12].selected = 0;
		t.importerTabs[12].tabpage = -1;

		//  Delete button
		t.importerTabs[7].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[7].y = t.importerTabs[1].y + 125 + (38*2);
		t.importerTabs[7].label = "Delete";
		t.importerTabs[7].selected = 0;
		t.importerTabs[7].tabpage = -1;

		//  Next button
		t.importerTabs[8].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[8].y = t.importerTabs[1].y + 125 + (38*3);
		t.importerTabs[8].label = "Next";
		t.importerTabs[8].selected = 0;
		t.importerTabs[8].tabpage = -1;

		//  Previous button
		t.importerTabs[9].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[9].y = t.importerTabs[1].y + 125 + (38*4);
		t.importerTabs[9].label = "Previous";
		t.importerTabs[9].selected = 0;
		t.importerTabs[9].tabpage = -1;

		//  Show Guide button
		t.importerTabs[10].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[10].y = t.importerTabs[1].y + 125 + (38*10) - 5;
		t.importerTabs[10].label = "Turn Guide Off";
		t.importerTabs[10].selected = 1;
		t.importerTabs[10].tabpage = -1;

		//  Importer button
		t.importerTabs[11].x = (GetChildWindowWidth() / 2) - 320 +128;
		t.importerTabs[11].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[11].label = "Cancel";
		t.importerTabs[11].selected = 0;
		t.importerTabs[11].tabpage = -1;

		t.importer.slidersmenumax=g.slidersmenumax;
	}
	else
	{
		g.slidersmenumax=t.importer.slidersmenumax;
	}

	// get actual value string names
	t.slidersmenuvaluechoice=115;
	t.slidersmenuvalueindex=t.slidersmenuvalue[t.importer.properties1Index][13].value;
	sliders_getnamefromvalue ( );
	t.slidersmenuvalue[t.importer.properties1Index][13].value_s=t.slidervaluename_s;
	t.slidersmenuvaluechoice=116;
	t.slidersmenuvalueindex=t.slidersmenuvalue[t.importer.properties1Index][14].value;
	sliders_getnamefromvalue ( );
	t.slidersmenuvalue[t.importer.properties1Index][14].value_s=t.slidervaluename_s;

	if (  t.importer.scaleMulti > 1.0 ) 
	{
		t.importerTabs[1].y = 0;
		t.importerTabs[2].y = 0;
		t.importerTabs[3].y = 0;
		t.importerTabs[4].y = 0;
		t.importerTabs[5].y = 0;
		t.importerTabs[11].y = 0;
		t.slidersmenu[t.importer.properties1Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties2Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties3Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties4Index].tleft= GetChildWindowWidth() - 255;

		//  New button
		t.importerTabs[6].x = GetChildWindowWidth() - 159;
		t.importerTabs[7].x = GetChildWindowWidth() - 159;
		t.importerTabs[8].x = GetChildWindowWidth() - 159;
		t.importerTabs[9].x = GetChildWindowWidth() - 159;
		t.importerTabs[10].x = GetChildWindowWidth() - 159;

		t.importerTabs[1].label = "Properties";
		t.importerTabs[1].selected = 1;
		t.importerTabs[1].tabpage = IMPORTERTABPAGE1;

		t.importerTabs[2].label = "Collision";
		t.importerTabs[2].selected = 0;
		t.importerTabs[2].tabpage = IMPORTERTABPAGE2;

		t.importerTabs[3].label = "Thumbnail";
		t.importerTabs[3].selected = 0;
		t.importerTabs[3].tabpage = IMPORTERTABPAGE3;

		t.importerTabs[4].label = "...";
		t.importerTabs[4].selected = 0;
		t.importerTabs[4].tabpage = IMPORTERTABPAGE4;

		//  Importer button
		t.importerTabs[5].label = "Save Entity";
		t.importerTabs[5].selected = 0;
		t.importerTabs[5].tabpage = -1;

		//  New button
		t.importerTabs[6].label = "Add New";
		t.importerTabs[6].selected = 0;
		t.importerTabs[6].tabpage = -1;

		//  New button
		t.importerTabs[12].label = "Duplicate";
		t.importerTabs[12].selected = 0;
		t.importerTabs[12].tabpage = -1;

		//  Delete button
		t.importerTabs[7].label = "Delete";
		t.importerTabs[7].selected = 0;
		t.importerTabs[7].tabpage = -1;

		//  Next button
		t.importerTabs[8].label = "Next";
		t.importerTabs[8].selected = 0;
		t.importerTabs[8].tabpage = -1;

		//  Previous button
		t.importerTabs[9].label = "Previous";
		t.importerTabs[9].selected = 0;
		t.importerTabs[9].tabpage = -1;

		//  Show Guide button
		t.importerTabs[10].label = "Turn Guide Off";
		t.importerTabs[10].selected = 1;
		t.importerTabs[10].tabpage = -1;

		//  Importer button
		t.importerTabs[11].label = "Cancel";
		t.importerTabs[11].selected = 0;
		t.importerTabs[11].tabpage = -1;

	}
	else
	{
		t.slidersmenu[t.importer.properties1Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties2Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties3Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties4Index].tleft= (GetChildWindowWidth() / 2 ) + 330;

		t.importerTabs[1].x = (GetChildWindowWidth() / 2) + 65-128 + 250;
		t.importerTabs[1].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[1].label = "Properties";
		t.importerTabs[1].selected = 1;
		t.importerTabs[1].tabpage = IMPORTERTABPAGE1;

		t.importerTabs[2].x = t.importerTabs[1].x + 128;
		t.importerTabs[2].y = t.importerTabs[1].y;
		t.importerTabs[2].label = "Collision";
		t.importerTabs[2].selected = 0;
		t.importerTabs[2].tabpage = IMPORTERTABPAGE2;

		t.importerTabs[3].x = t.importerTabs[2].x + 128;
		t.importerTabs[3].y = t.importerTabs[1].y;
		t.importerTabs[3].label = "Thumbnail";
		t.importerTabs[3].selected = 0;
		t.importerTabs[3].tabpage = IMPORTERTABPAGE3;

		t.importerTabs[4].x = t.importerTabs[3].x + 256;
		t.importerTabs[4].y = t.importerTabs[1].y;
		t.importerTabs[4].label = "...";
		t.importerTabs[4].selected = 0;
		t.importerTabs[4].tabpage = IMPORTERTABPAGE4;

		//  Importer button
		t.importerTabs[5].x = (GetChildWindowWidth() / 2) - 320;
		t.importerTabs[5].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[5].label = "Save Entity";
		t.importerTabs[5].selected = 0;
		t.importerTabs[5].tabpage = -1;

		//  New button
		t.importerTabs[6].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[6].y = t.importerTabs[1].y + 125;
		t.importerTabs[6].label = "Add New";
		t.importerTabs[6].selected = 0;
		t.importerTabs[6].tabpage = -1;

		//  New button
		t.importerTabs[12].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[12].y = t.importerTabs[1].y + 125 + 38;
		t.importerTabs[12].label = "Duplicate";
		t.importerTabs[12].selected = 0;
		t.importerTabs[12].tabpage = -1;

		//  Delete button
		t.importerTabs[7].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[7].y = t.importerTabs[1].y + 125 + (38*2);
		t.importerTabs[7].label = "Delete";
		t.importerTabs[7].selected = 0;
		t.importerTabs[7].tabpage = -1;

		//  Next button
		t.importerTabs[8].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[8].y = t.importerTabs[1].y + 125 + (38*3);
		t.importerTabs[8].label = "Next";
		t.importerTabs[8].selected = 0;
		t.importerTabs[8].tabpage = -1;

		//  Previous button
		t.importerTabs[9].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[9].y = t.importerTabs[1].y + 125 + (38*4);
		t.importerTabs[9].label = "Previous";
		t.importerTabs[9].selected = 0;
		t.importerTabs[9].tabpage = -1;

		//  Show Guide button
		t.importerTabs[10].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[10].y = t.importerTabs[1].y + 125 + (38*10) - 5;
		t.importerTabs[10].label = "Turn Guide Off";
		t.importerTabs[10].selected = 1;
		t.importerTabs[10].tabpage = -1;

		//  Importer button
		t.importerTabs[11].x = (GetChildWindowWidth() / 2) - 320 +128;
		t.importerTabs[11].y = (GetChildWindowHeight() / 2) - 304;
		t.importerTabs[11].label = "Cancel";
		t.importerTabs[11].selected = 0;
		t.importerTabs[11].tabpage = -1;
	}
	t.timporterpickdepth_f = 1250;

	// reposition camera in sky so no terrain depth clipping can occur
	//PE: bug , fix object clipping invisible import objects.
	if ( g_bCameraInSkyForImporter == false )
	{
		PositionCamera ( 0, CameraPositionX(0), CameraPositionY(0)+100000, CameraPositionZ(0) );
		SetCameraRange ( 0, 1.0f, 70000.0f );
		g_bCameraInSkyForImporter = true;
	}
}

void importer_free ( void )
{
	// restore camera from sky
	if ( g_bCameraInSkyForImporter = true )
	{
		PositionCamera ( 0, CameraPositionX(0), CameraPositionY(0)-100000, CameraPositionZ(0) );
		if ( t.editorfreeflight.mode == 1 ) 
			SetCameraRange ( 0, t.tcamneardistance_f, 70000 );
		else
			SetCameraRange ( 0, t.tcamneardistance_f, t.tcamrange_f );
		g_bCameraInSkyForImporter = false;
	}
	// show entities and waypoints
	importer_show_mouse ( );
	for ( t.te = 1 ; t.te <= g.entityelementlist; t.te++ )
	{
		t.tobj=t.entityelement[t.te].obj;
		if ( t.tobj>0 ) 
		{
			if ( ObjectExist(t.tobj) == 1 ) ShowObject ( t.tobj );
			int iCCObjHead = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+0;
			int iCCObjBeard = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+1;
			int iCCObjHat = g.charactercreatorrmodelsoffset+((t.te*3)-t.characterkitcontrol.offset)+2;
			if ( ObjectExist(iCCObjHead) == 1 ) ShowObject ( iCCObjHead );
			if ( ObjectExist(iCCObjBeard) == 1 ) ShowObject ( iCCObjBeard );
			if ( ObjectExist(iCCObjHat) == 1 ) ShowObject ( iCCObjHat );
		}
	}
	waypoint_showall ( );

	//  Free any resources
	for ( t.i = 1 ; t.i<=  15; t.i++ )
	{
		if (  ImageExist(g.importermenuimageoffset+t.i) == 1  )  DeleteImage (  g.importermenuimageoffset+t.i );
	}

	//  DeleteObjects (  used )
	if (  ObjectExist(t.importer.objectnumber) ) DeleteObject ( t.importer.objectnumber );
	if (  ObjectExist(t.importer.objectnumberpreeffectcopy) ) DeleteObject ( t.importer.objectnumberpreeffectcopy );
	if (  ObjectExist(t.importer.dummyCharacterObjectNumber)  )  DeleteObject (  t.importer.dummyCharacterObjectNumber );

	//  Delete any textures and sprites used
	int tCount = 1;
	for ( tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		if (  t.importerTextures[tCount].spriteID  !=  0 ) 
		{
			if ( SpriteExist ( t.importerTextures[tCount].spriteID ) ) DeleteSprite (  t.importerTextures[tCount].spriteID );
		}
		if (  t.importerTextures[tCount].spriteID2  !=  0 ) 
		{
			if ( SpriteExist ( t.importerTextures[tCount].spriteID2 ) ) DeleteSprite (  t.importerTextures[tCount].spriteID2 );
		}
		t.importerTextures[tCount].spriteID = 0;
		t.importerTextures[tCount].spriteID2 = 0;
		if (  t.importerTextures[tCount].imageID  !=  0 ) 
		{
			if (  ImageExist(t.importerTextures[tCount].imageID)  )  DeleteImage (  t.importerTextures[tCount].imageID );
		}
		t.importerTextures[tCount].imageID = 0;
		t.importerTextures[tCount].originalName = "";
		t.importerTextures[tCount].fileName = "";
		t.importerTextures[tCount].iExpandedThisSlot = 0;
		t.importerTextures[tCount].iOptionalStage = 0;
		t.importerTextures[tCount].iAssociatedBaseImage = 0;
	}

	if (  SpriteExist (t.importer.helpSprite) )   DeleteSprite (  t.importer.helpSprite );
	if (  SpriteExist (t.importer.helpSprite2)  )  DeleteSprite (  t.importer.helpSprite2 );
	if (  SpriteExist (t.importer.helpSprite3)  )  DeleteSprite (  t.importer.helpSprite3 );
	if (  SpriteExist (t.importer.helpSprite4)  )  DeleteSprite (  t.importer.helpSprite4 );

	//  Delete grid objects
	for ( tCount = 1 ; tCount<=  9; tCount++ )
	{
		if (  t.importerGridObject[tCount] > 0 ) 
		{
			if (  ObjectExist(t.importerGridObject[tCount])  ==  1  )  DeleteObject (  t.importerGridObject[tCount] );
		}
	}

	//  Free Collision boxes
	for ( tCount = 0 ; tCount<=  t.importer.collisionShapeCount; tCount++ )
	{
		if (  t.importerCollision[tCount].object > 0 ) 
		{
			if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
			{
				DeleteObject (  t.importerCollision[tCount].object );
				DeleteObject (  t.importerCollision[tCount].object2 );
			}
		}
	}

	//  DeleteObject (  selected markers )
	for ( tCount = 1 ; tCount<=  9; tCount++ )
	{
		if (  t.selectedObjectMarkers[tCount] > 0 ) 
		{
			if (  ObjectExist(t.selectedObjectMarkers[tCount])  ==  1  )  DeleteObject (  t.selectedObjectMarkers[tCount] );
		}
	}

	t.importer.loaded = 0;
	t.importerTabs[11].selected = 0;

	//  Return tab mode back to original state
	g.tabmode = t.importer.oldTabMode;

	//  Set back to initial dir
	SetDir (  t.importer.startDir.Get() );

	UnDim (  t.tUnknown );
	UnDim (  t.snapPosX_f );
	UnDim (  t.snapPosY_f );
	UnDim (  t.snapPosZ_f );
}

cstr importer_getfilenameonly ( LPSTR pFileAndPossiblePath )
{
	cstr pFileNameOnly = pFileAndPossiblePath;
	for ( int n = strlen(pFileAndPossiblePath); n > 0; n-- )
	{
		if ( pFileAndPossiblePath[n] == '\\' || pFileAndPossiblePath[n] == '/' )
		{
			pFileNameOnly = cstr(pFileAndPossiblePath+n+1);
			break;
		}
	}
	return pFileNameOnly;
}

int importer_findtextureinlist ( LPSTR pFindFilename )
{
	for ( int iImageListIndex = 1; iImageListIndex < IMPORTERTEXTURESMAX; iImageListIndex++ )
	{
		if ( t.importerTextures[iImageListIndex].imageID > 0 )
		{
			cstr pCompareWith = t.importerTextures[iImageListIndex].fileName;
			if ( strnicmp ( pCompareWith.Get(), pFindFilename, strlen(pCompareWith.Get()) ) == NULL )
			{
				return iImageListIndex;
			}
		}
	}
	return 0;
}

int importer_findtextureindexinlist ( LPSTR pFindFilename )
{
	int iImageListindex = importer_findtextureinlist ( pFindFilename );
	if ( iImageListindex > 0 )
		return t.importerTextures[iImageListindex].imageID;
	else
		return 0;
}

int importer_addtexturefiletolist ( cstr fileName, cstr sourceName, int* tCount )
{
	// Check if we already have this texture
	t.tfound = 0;
	for ( t.tCount2 = 1 ; t.tCount2 <= IMPORTERTEXTURESMAX; t.tCount2++ )
	{
		if (  t.importerTextures[t.tCount2].fileName  == fileName ) 
		{
			t.tfound = 1;
			break;
		}
	}

	// If we don't have the texture, add it to list
	if ( t.tfound == 0 ) 
	{
		// find free slot
		t.tfound = 0;
		for ( int tCount3 = 1 ; tCount3 <= IMPORTERTEXTURESMAX; tCount3++ )
		{
			if ( strlen ( t.importerTextures[tCount3].fileName.Get() ) == 0 ) 
			{
				t.tfound = tCount3;
				break;
			}
		}

		// did we find a free texture slot?
		if ( t.tfound > 0 ) 
		{
			//  Add to importer texture list
			if ( t.tfound < IMPORTERTEXTURESMAX )
			{
				if ( t.tfound <= (*tCount) )
				{
					t.importerTextures[t.tfound].fileName = fileName;
					t.importerTextures[t.tfound].originalName = sourceName;
				}
				else
				{
					++(*tCount);
					t.importerTextures[(*tCount)].fileName = fileName;
					t.importerTextures[(*tCount)].originalName = sourceName;
				}
			}
		}
	}
	return t.tfound;
}

void importer_addtoimagelistandloadifexist ( LPSTR pImgFilename, int iOptionalStage, int iOptionalBaseImageSlotIndex )
{
	int tCount = t.tcounttextures;
	if ( FileExist ( pImgFilename ) )
	{
		// assign image to new slot in image list
		int iInsertedAtSlot = importer_addtexturefiletolist ( pImgFilename, pImgFilename, &tCount );

		// assign any special texture 'stage' value
		t.importerTextures[iInsertedAtSlot].iOptionalStage = iOptionalStage;
		t.importerTextures[iInsertedAtSlot].iAssociatedBaseImage = iOptionalBaseImageSlotIndex;

		// load image in
		t.tImageID = g.importermenuimageoffset+15;
		while ( ImageExist(t.tImageID) == 1 ) ++t.tImageID;
		LoadImage ( t.importerTextures[iInsertedAtSlot].fileName.Get(), t.tImageID );
		t.importerTextures[iInsertedAtSlot].imageID = t.tImageID;
	}
	t.tcounttextures = tCount;
}

void importer_findimagetypesfromlist ( cstr fileName, int iBaseImageSlotIndex, int* piImgColor, int* piImgNormal, int* piImgSpecular, int* piImgGloss, int* piImgAO, int* piImgHeight )
{
	// get base filename extension (deduct image format ext)
	LPSTR pExt = NULL;
	for ( int iImgFormat = 0; iImgFormat < 4; iImgFormat++ )
	{
		if ( iImgFormat == 0 ) pExt = ".png";
		if ( iImgFormat == 1 ) pExt = ".dds";
		if ( iImgFormat == 2 ) pExt = ".tga";
		if ( iImgFormat == 3 ) pExt = ".jpg";
		if ( strnicmp ( fileName.Get()+strlen(fileName.Get())-strlen(pExt), pExt, strlen(pExt) ) == NULL ) 
			break;
	}
	if ( pExt == NULL ) return;

	// get base filename extension (deduct color specifier)
	LPSTR pImgType = NULL;
	cstr pBaseNoFileExt = cstr(Left(fileName.Get(),Len(fileName.Get())-Len(pExt)));
	for ( int iImgType = 0; iImgType < 5; iImgType++ )
	{
		// find kind of 'color' image type
		if ( iImgType == 0 ) pImgType = "_diffuse";
		if ( iImgType == 1 ) pImgType = "_color";
		if ( iImgType == 2 ) pImgType = "_d";
		if ( iImgType == 3 ) pImgType = "_albedo";
		if ( strnicmp ( pBaseNoFileExt.Get()+strlen(pBaseNoFileExt.Get())-strlen(pImgType), pImgType, strlen(pImgType) ) == NULL ) 
		{
			// get base filename (minus image type)
			cstr pBaseFile = cstr(Left(pBaseNoFileExt.Get(),Len(pBaseNoFileExt.Get())-Len(pImgType)));

			// locate color image from image list
			cstr pColorFile = pBaseFile + cstr(pImgType) + cstr(pExt);
			*piImgColor = importer_findtextureindexinlist ( pColorFile.Get() );

			// attempt to locate other textures associated with color image
			cstr pNormalFile = pBaseFile + cstr("_normal") + cstr(pExt);
			importer_addtoimagelistandloadifexist ( pNormalFile.Get(), 2, iBaseImageSlotIndex );
			*piImgNormal = importer_findtextureindexinlist ( pNormalFile.Get() );

			cstr pSpecularFile = pBaseFile + cstr("_specular") + cstr(pExt);
			importer_addtoimagelistandloadifexist ( pSpecularFile.Get(), 3, iBaseImageSlotIndex );
			*piImgSpecular = importer_findtextureindexinlist ( pSpecularFile.Get() );
			if ( *piImgSpecular == 0 )
			{
				cstr pMetalnessFile = pBaseFile + cstr("_metalness") + cstr(pExt);
				importer_addtoimagelistandloadifexist ( pMetalnessFile.Get(), 3, iBaseImageSlotIndex );
				*piImgSpecular = importer_findtextureindexinlist ( pMetalnessFile.Get() );
			}

			cstr pGlossFile = pBaseFile + cstr("_gloss") + cstr(pExt);
			importer_addtoimagelistandloadifexist ( pGlossFile.Get(), 4, iBaseImageSlotIndex );
			*piImgGloss = importer_findtextureindexinlist ( pGlossFile.Get() );

			cstr pAOFile = pBaseFile + cstr("_ao") + cstr(pExt);
			importer_addtoimagelistandloadifexist ( pAOFile.Get(), 1, iBaseImageSlotIndex );
			*piImgAO = importer_findtextureindexinlist ( pAOFile.Get() );
			if ( *piImgAO == 0 ) 
			{
				pAOFile = g.rootdir_s + cstr("effectbank\\reloaded\\media\\blank_O.dds");
				importer_addtoimagelistandloadifexist ( pAOFile.Get(), 1, iBaseImageSlotIndex );
				*piImgAO = importer_findtextureindexinlist ( pAOFile.Get() );
			}

			cstr pHeightFile = pBaseFile + cstr("_height") + cstr(pExt);
			importer_addtoimagelistandloadifexist ( pHeightFile.Get(), 5, iBaseImageSlotIndex );
			*piImgHeight = importer_findtextureindexinlist ( pHeightFile.Get() );
		}
	}
}

void importer_findandremoveentry ( cstr sFileToRemove )
{
	for ( int tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		if ( strnicmp ( t.importerTextures[tCount].fileName.Get(), sFileToRemove.Get(), strlen(sFileToRemove.Get())-4 ) == NULL ) 
		{
			if ( ImageExist ( t.importerTextures[tCount].imageID ) == 1 ) DeleteImage ( t.importerTextures[tCount].imageID );
			t.importerTextures[tCount].imageID = 0;
			t.importerTextures[tCount].fileName = "";
		}
	}
}

void importer_removeentryandassociatesof ( int tCount )
{
	// the file to delete (and all its associates)
	LPSTR pRemoveTextureFile = t.importerTextures[tCount].fileName.Get();

	// get base filename extension (deduct image format ext)
	LPSTR pExt = NULL;
	for ( int iImgFormat = 0; iImgFormat < 4; iImgFormat++ )
	{
		if ( iImgFormat == 0 ) pExt = ".png";
		if ( iImgFormat == 1 ) pExt = ".dds";
		if ( iImgFormat == 2 ) pExt = ".tga";
		if ( iImgFormat == 3 ) pExt = ".jpg";
		if ( strnicmp ( pRemoveTextureFile+strlen(pRemoveTextureFile)-strlen(pExt), pExt, strlen(pExt) ) == NULL ) 
			break;
	}
	if ( pExt == NULL ) return;

	// get base filename extension (deduct color specifier)
	LPSTR pImgType = NULL;
	cstr pBaseNoFileExt = cstr(Left(pRemoveTextureFile,Len(pRemoveTextureFile)-Len(pExt)));
	for ( int iImgType = 0; iImgType < 5; iImgType++ )
	{
		// find kind of 'color' image type
		if ( iImgType == 0 ) pImgType = "_diffuse";
		if ( iImgType == 1 ) pImgType = "_color";
		if ( iImgType == 2 ) pImgType = "_d";
		if ( iImgType == 3 ) pImgType = "_albedo";
		if ( strnicmp ( pBaseNoFileExt.Get()+strlen(pBaseNoFileExt.Get())-strlen(pImgType), pImgType, strlen(pImgType) ) == NULL ) 
		{
			// get base filename (minus image type)
			cstr pBaseFile = cstr(Left(pBaseNoFileExt.Get(),Len(pBaseNoFileExt.Get())-Len(pImgType)));

			// locate color image from image list
			importer_findandremoveentry( pBaseFile + cstr("_normal") + cstr(pExt) );
			importer_findandremoveentry( pBaseFile + cstr("_specular") + cstr(pExt) );
			importer_findandremoveentry( pBaseFile + cstr("_metalness") + cstr(pExt) );
			importer_findandremoveentry( pBaseFile + cstr("_gloss") + cstr(pExt) );
			importer_findandremoveentry( pBaseFile + cstr("_ao") + cstr(pExt) );
			importer_findandremoveentry( pBaseFile + cstr("_height") + cstr(pExt) );
		}
	}
}

void importer_applyimagelisttextures ( bool bCubeMapOnly, int iOptionalOnlyUpdateImageListIndex, bool bExpandOutPBRTextureSet )
{
	// either update cube map only (for model that already has its texture stages intact)
	// work out object texture stages (based on shader chosen)
	int iColorStage = 0;
	int iAOStage = 1;
	int iNormalStage = 2;
	int iSpecularStage = 3;
	int iGlossStage = 4;
	int iHeightStage = -1;
	int iEnvStage = 6;
	int iImageIndexForCUBE = 72543;
	cstr pShaderCUBE = "effectbank\\reloaded\\media\\CUBE.dds";
	LoadImage ( pShaderCUBE.Get(), iImageIndexForCUBE, 2 );
	PerformCheckListForLimbs ( t.importer.objectnumber );
	int iTextureCount = 0;
	for ( int tCount = 0 ; tCount <= ChecklistQuantity()-1; tCount++ )
	{
		cstr pLimbTextureName = importer_getfilenameonly ( LimbTextureName ( t.importer.objectnumber, tCount ) );
		if ( strlen ( pLimbTextureName.Get() ) > 0 )
		{
			// new FPE field to specify limbs that are hair (i.e. no zwrite, no culling)
			cstr pBaseFile = Left ( pLimbTextureName.Get(), strlen(pLimbTextureName.Get())-4 );
			if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 6, "_color", 6 ) == NULL )
			{
				pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-6 );
				if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 5, "_hair", 5 ) == NULL )
				{
					// switch off culling (leave zwrite as distant hair rendered over nearer hair)
					SetLimbCull ( t.importer.objectnumber, tCount, false );
				}
			}

			// apply cube map regardless
			TextureLimbStage ( t.importer.objectnumber, tCount, iEnvStage, iImageIndexForCUBE );

			// also apply default textures where texture slot does not have image
			sObject* pObject = GetObjectData ( t.importer.objectnumber );
			if ( pObject )
			{
				if ( tCount <= pObject->iFrameCount )
				{
					sFrame* pFrame = pObject->ppFrameList[tCount];
					if ( pFrame->pMesh )
					{
						sMesh* pMesh = pFrame->pMesh;
						int iTextureMax = pMesh->dwTextureCount;
						if ( iTextureMax > 4 ) iTextureMax = 4;
						for ( int iTexture = 1; iTexture <= iTextureMax; iTexture++ )
						{
							if ( pMesh->pTextures[iTexture].iImageID == 0 )
							{
								int iDefaultImage = 0;
								if ( iTexture == iAOStage ) iDefaultImage = loadinternaltextureex("effectbank\\reloaded\\media\\blank_O.dds",1,t.tfullorhalfdivide);
								if ( iTexture == iNormalStage ) iDefaultImage = loadinternaltextureex("effectbank\\reloaded\\media\\blank_N.dds",1,t.tfullorhalfdivide);
								if ( iTexture == iSpecularStage ) iDefaultImage = loadinternaltextureex("effectbank\\reloaded\\media\\blank_black.dds",1,t.tfullorhalfdivide);
								if ( iTexture == iGlossStage ) iDefaultImage = loadinternaltextureex("effectbank\\reloaded\\media\\white_D.dds",1,t.tfullorhalfdivide);
								TextureLimbStage ( t.importer.objectnumber, tCount, iTexture, iDefaultImage );
							}
						}
					}
				}
			}

			// count textures specified in model
			iTextureCount++;
		}
	}

	// should map new texture choices to the original image slots
	if ( g_piMapImageListIndexToLimbIndex == NULL )
	{
		bool bAbsolutelyNoTexturesReferencedAnywhere = true;
		g_dwMapImageListIndexToLimbCount = ChecklistQuantity(); 
		g_piMapImageListIndexToLimbIndex = new int[g_dwMapImageListIndexToLimbCount];
		for ( int tLimbIndex = 0 ; tLimbIndex <= ChecklistQuantity()-1; tLimbIndex++ )
		{
			int iFindImageSlotIndex = -1;
			cstr pLimbTextureName = importer_getfilenameonly ( LimbTextureName ( t.importer.objectnumber, tLimbIndex ) );
			LPSTR pSearch = pLimbTextureName.Get();
			if ( strlen ( pSearch ) > 0 )
			{
				iFindImageSlotIndex = 1; // 220618 - default to slot one if no specific match can be made (so model CAN be textured)
				bAbsolutelyNoTexturesReferencedAnywhere = false;
				for ( int tCount = 1; tCount <= t.tcounttextures; tCount++ )
				{
					LPSTR pThisImageItem = t.importerTextures[tCount].fileName.Get();
					if ( strnicmp ( pThisImageItem + strlen(pThisImageItem) - strlen(pSearch), pSearch, strlen(pSearch)-4 ) == NULL )
					{
						iFindImageSlotIndex = tCount;
					}
				}
			}
			g_piMapImageListIndexToLimbIndex [ tLimbIndex ] = iFindImageSlotIndex;
		}
		if ( bAbsolutelyNoTexturesReferencedAnywhere == true )
		{
			// okay, so now we know that no meshes are particular about their texture, 
			// we will use slot one for all textures on this model
			int iFindImageSlotIndex = 1;
			sObject* pObject = GetObjectData ( t.importer.objectnumber );
			if ( pObject )
			{
				for ( int tLimbIndex = 0 ; tLimbIndex <= ChecklistQuantity()-1; tLimbIndex++ )
				{
					sFrame* pFrame = pObject->ppFrameList[tLimbIndex];
					if ( pFrame )
					{
						if ( pFrame->pMesh )
						{
							g_piMapImageListIndexToLimbIndex [ tLimbIndex ] = iFindImageSlotIndex;
						}
					}
				}
			}
		}
	}

	// or full retexture model from imagelist
	if ( bCubeMapOnly == false )
	{
		/*
		// work out if object is single or multi-texture
		int iTextureCount = 0;
		char pStoreTextureNames[50][512];
		memset ( pStoreTextureNames, 0, sizeof(pStoreTextureNames) );
		PerformCheckListForLimbs ( t.importer.objectnumber );
		for ( int tCount = 0 ; tCount <= ChecklistQuantity()-1; tCount++ )
		{
			cstr pLimbTextureName = importer_getfilenameonly ( LimbTextureName ( t.importer.objectnumber, tCount ) );
			if ( strlen ( pLimbTextureName.Get() ) > 0 )
			{
				// only add/count if unique
				bool bTexNameUnique = true;
				for ( int iScan = 0; iScan < iTextureCount; iScan++ )
				{
					if ( stricmp ( pStoreTextureNames[iScan], pLimbTextureName.Get() ) == NULL )
					{
						bTexNameUnique = false; 
						break;
					}
				}

				// add to list of texture names found in object limbs
				if ( bTexNameUnique == true )
				{
					if ( iTextureCount < 50 )
					{
						strcpy ( pStoreTextureNames[iTextureCount], pLimbTextureName.Get() );
						iTextureCount++;
					}
				}
			}
		}
		*/

		/*
		// single or multi texture
		if ( iTextureCount <= 1 )
		{
			// SINGLE - assign texture to model from slot one 
			TextureObject (  t.importer.objectnumber, t.importerTextures[1].imageID );
			for ( int tCount = 0 ; tCount <= ChecklistQuantity()-1; tCount++ )
			{
				TextureLimb ( t.importer.objectnumber, tCount, t.importerTextures[1].imageID );
				int iImgColor=0, iImgNormal=0, iImgSpecular=0, iImgGloss=0, iImgAO=0, iImgHeight=0;
				importer_findimagetypesfromlist ( t.importerTextures[1].fileName, &iImgColor, &iImgNormal, &iImgSpecular, &iImgGloss, &iImgAO, &iImgHeight );
				if ( iImgColor > 0 ) TextureLimbStage ( t.importer.objectnumber, tCount, iColorStage, iImgColor );
				if ( iImgNormal > 0 ) TextureLimbStage ( t.importer.objectnumber, tCount, iNormalStage, iImgNormal );
				if ( iImgSpecular > 0 ) TextureLimbStage ( t.importer.objectnumber, tCount, iSpecularStage, iImgSpecular );
				if ( iImgGloss > 0 ) TextureLimbStage ( t.importer.objectnumber, tCount, iGlossStage, iImgGloss );
				if ( iImgAO > 0 && iAOStage != - 1 ) TextureLimbStage ( t.importer.objectnumber, tCount, iAOStage, iImgAO );
				if ( iImgHeight > 0 && iHeightStage != - 1 ) TextureLimbStage ( t.importer.objectnumber, tCount, iHeightStage, iImgHeight );
				TextureLimbStage ( t.importer.objectnumber, tCount, iEnvStage, iImageIndexForCUBE );
			}
		}
		else
		{
		*/

		// texture stage specific non-base (normal, ao, etc)
		int iOptionalStage = 0;
		if ( iOptionalOnlyUpdateImageListIndex > 0 ) iOptionalStage = t.importerTextures[iOptionalOnlyUpdateImageListIndex].iOptionalStage;
		if ( iOptionalStage > 0 )
		{
			int iAssociatedBaseImage = t.importerTextures[iOptionalOnlyUpdateImageListIndex].iAssociatedBaseImage;
			for ( int tLimbIndex = 0 ; tLimbIndex <= ChecklistQuantity()-1; tLimbIndex++ )
			{
				int iImageListIndex = g_piMapImageListIndexToLimbIndex [ tLimbIndex ];
				if ( iImageListIndex > 0 && t.importerTextures[iImageListIndex].iAssociatedBaseImage > 0 ) iImageListIndex = t.importerTextures[iImageListIndex].iAssociatedBaseImage;
				if ( iImageListIndex > 0 && iAssociatedBaseImage == iImageListIndex )
				{
					if ( iOptionalStage == 2 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iNormalStage, t.importerTextures[iOptionalOnlyUpdateImageListIndex].imageID );
					if ( iOptionalStage == 3 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iSpecularStage, t.importerTextures[iOptionalOnlyUpdateImageListIndex].imageID );
					if ( iOptionalStage == 4 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iGlossStage, t.importerTextures[iOptionalOnlyUpdateImageListIndex].imageID );
					if ( iOptionalStage == 1 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iAOStage, t.importerTextures[iOptionalOnlyUpdateImageListIndex].imageID );
				}
			}
		}
		else
		{
			// texture all model or just one limb
			for ( int tLimbIndex = 0 ; tLimbIndex <= ChecklistQuantity()-1; tLimbIndex++ )
			{
				int iImageListIndex = g_piMapImageListIndexToLimbIndex [ tLimbIndex ];
				if ( iImageListIndex > 0 && (iOptionalOnlyUpdateImageListIndex == -1 || iOptionalOnlyUpdateImageListIndex == iImageListIndex ))
				{
					cstr pLimbTextureName = importer_getfilenameonly ( t.importerTextures[iImageListIndex].fileName.Get() );
					if ( strlen ( pLimbTextureName.Get() ) > 0 )
					{
						if ( t.importerTextures[iImageListIndex].imageID > 0 )
						{
							// diffuse/albedo
							TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iColorStage, t.importerTextures[iImageListIndex].imageID );

							// find and load any associated PBR textures for this color texture choice
							if ( bExpandOutPBRTextureSet == true )
							{
								// only find every texture on initial texture load, not when replacing specific texture slots
								int iImgColor=0, iImgNormal=0, iImgSpecular=0, iImgGloss=0, iImgAO=0, iImgHeight=0;
								importer_findimagetypesfromlist ( t.importerTextures[iImageListIndex].fileName, iImageListIndex, &iImgColor, &iImgNormal, &iImgSpecular, &iImgGloss, &iImgAO, &iImgHeight );
								if ( iImgColor > 0 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iColorStage, iImgColor );
								if ( iImgNormal > 0 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iNormalStage, iImgNormal );
								if ( iImgSpecular > 0 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iSpecularStage, iImgSpecular );
								if ( iImgGloss > 0 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iGlossStage, iImgGloss );
								if ( iAOStage != - 1 && iImgAO > 0 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iAOStage, iImgAO );
								if ( iImgHeight > 0 && iHeightStage != - 1 ) TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iHeightStage, iImgHeight );
							}

							/*
							// work out base filename by removing known albedo extensions
							cstr pBaseFile = Left ( pLimbTextureName.Get(), strlen(pLimbTextureName.Get())-4 );
							if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 2, "_d", 2 ) == NULL )
							{
								pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-2 );
							}
							else if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 6, "_color", 6 ) == NULL )
							{
								pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-6 );
							}
							else if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 8, "_diffuse", 8 ) == NULL )
							{
								pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-8 );
							}
							if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 5, "_hair", 5 ) == NULL )
							{
								// switch off culling (leave zwrite as distant hair rendered over nearer hair)
								SetLimbCull ( t.importer.objectnumber, tLimbIndex, false );
							}

							// detect normal
							cstr pNormalFile = pBaseFile + cstr("normal.png");
							int iImageIndexForNormal = importer_findtextureinlist ( pNormalFile.Get() );
							TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iNormalStage, t.importerTextures[iImageIndexForNormal].imageID );

							// detect specular or metalness
							cstr pSpecularFile = pBaseFile + cstr("specular.png");
							int iImageIndexForSpecular = importer_findtextureinlist ( pSpecularFile.Get() );
							if ( iImageIndexForSpecular == 0 )
							{
								pSpecularFile = pBaseFile + cstr("metalness.png");
								iImageIndexForSpecular = importer_findtextureinlist ( pSpecularFile.Get() );
								TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iSpecularStage, t.importerTextures[iImageIndexForSpecular].imageID );
							}
							TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iSpecularStage, t.importerTextures[iImageIndexForSpecular].imageID );

							// delect gloss
							cstr pGlossFile = pBaseFile + cstr("gloss.png");
							int iImageIndexForGloss = importer_findtextureinlist ( pGlossFile.Get() );
							TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iGlossStage, t.importerTextures[iImageIndexForGloss].imageID );
							*/

							// apply environment cube map
							TextureLimbStage ( t.importer.objectnumber, tLimbIndex, iEnvStage, iImageIndexForCUBE );
						}
					}
				}
			}
			//}
			/*
			for ( int tCount = 0 ; tCount <= ChecklistQuantity()-1; tCount++ )
			{
				cstr pLimbTextureName = importer_getfilenameonly ( LimbTextureName ( t.importer.objectnumber, tCount ) );
				if ( strlen ( pLimbTextureName.Get() ) > 0 )
				{
					for ( int iImageListIndex = 0; iImageListIndex < IMPORTERTEXTURESMAX; iImageListIndex++ )
					{
						if ( t.importerTextures[iImageListIndex].imageID > 0 )
						{
							cstr pCompareWith = importer_getfilenameonly ( t.importerTextures[iImageListIndex].fileName.Get() );
							if ( strnicmp ( pCompareWith.Get(), pLimbTextureName.Get(), strlen(pCompareWith.Get()) ) == NULL )
							{
								// diffuse/albedo
								TextureLimbStage ( t.importer.objectnumber, tCount, iColorStage, t.importerTextures[iImageListIndex].imageID );

								// work out base filename by removing known albedo extensions
								cstr pBaseFile = Left ( pLimbTextureName.Get(), strlen(pLimbTextureName.Get())-4 );
								if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 2, "_d", 2 ) == NULL )
								{
									pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-2 );
								}
								else if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 6, "_color", 6 ) == NULL )
								{
									pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-6 );
								}
								else if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 8, "_diffuse", 8 ) == NULL )
								{
									pBaseFile = Left ( pBaseFile.Get(), strlen(pBaseFile.Get())-8 );
								}
								if ( strnicmp ( pBaseFile.Get() + strlen(pBaseFile.Get()) - 5, "_hair", 5 ) == NULL )
								{
									// switch off culling (leave zwrite as distant hair rendered over nearer hair)
									SetLimbCull ( t.importer.objectnumber, tCount, false );
								}

								// detect normal
								cstr pNormalFile = pBaseFile + cstr("normal.png");
								int iImageIndexForNormal = importer_findtextureinlist ( pNormalFile.Get() );
								TextureLimbStage ( t.importer.objectnumber, tCount, iNormalStage, t.importerTextures[iImageIndexForNormal].imageID );

								// detect specular or metalness
								cstr pSpecularFile = pBaseFile + cstr("specular.png");
								int iImageIndexForSpecular = importer_findtextureinlist ( pSpecularFile.Get() );
								if ( iImageIndexForSpecular == 0 )
								{
									pSpecularFile = pBaseFile + cstr("metalness.png");
									iImageIndexForSpecular = importer_findtextureinlist ( pSpecularFile.Get() );
									TextureLimbStage ( t.importer.objectnumber, tCount, iSpecularStage, t.importerTextures[iImageIndexForSpecular].imageID );
								}
								TextureLimbStage ( t.importer.objectnumber, tCount, iSpecularStage, t.importerTextures[iImageIndexForSpecular].imageID );

								// delect gloss
								cstr pGlossFile = pBaseFile + cstr("gloss.png");
								int iImageIndexForGloss = importer_findtextureinlist ( pGlossFile.Get() );
								TextureLimbStage ( t.importer.objectnumber, tCount, iGlossStage, t.importerTextures[iImageIndexForGloss].imageID );

								// detect CUBE
								TextureLimbStage ( t.importer.objectnumber, tCount, iEnvStage, iImageIndexForCUBE );
							}
						}
					}
				}
			}
			*/
		}
	}
}

void importer_assignsprite ( int tCount )
{
	t.tSpriteID = 50;
	while (  SpriteExist(t.tSpriteID) == 1 ) ++t.tSpriteID;
	t.importerTextures[tCount].spriteID = t.tSpriteID+1;
	t.importerTextures[tCount].spriteID2 = t.tSpriteID;
	Sprite ( t.importerTextures[tCount].spriteID, -10000, -10000, g.importermenuimageoffset+6 );
	Sprite ( t.importerTextures[tCount].spriteID2, -10000, -10000, g.importermenuimageoffset+6 );
}

void importer_recreate_texturesprites ( void )
{
	// Create sprite to represent texture 
	t.tSpriteID = 50;
	for ( int tCount = 1; tCount <= t.tcounttextures; tCount++ )
	{
		if ( SpriteExist ( t.tSpriteID+0 ) == 1 ) DeleteSprite ( t.tSpriteID+0 );
		if ( SpriteExist ( t.tSpriteID+1 ) == 1 ) DeleteSprite ( t.tSpriteID+1 );
		importer_assignsprite ( tCount );
		t.tSpriteID+=2;
	}
	for ( int tCount = t.tcounttextures+1; tCount < IMPORTERTEXTURESMAX; tCount++ )
	{
		int iSprite = t.importerTextures[tCount].spriteID;
		int iSprite2 = t.importerTextures[tCount].spriteID2;
		if ( iSprite > 0 && SpriteExist ( iSprite ) == 1 ) DeleteSprite ( iSprite );
		if ( iSprite2 > 0 && SpriteExist ( iSprite2 ) == 1 ) DeleteSprite ( iSprite2 );
	}
}

int giRememberLastEffectIndexInImporter = -1;

void importer_changeshader ( LPSTR pNewShaderFilename )
{
	// when shader changed in importer dialog, change shader of imported model
	if ( t.importer.objectnumber > 0 )
	{
		if ( ObjectExist ( t.importer.objectnumber ) == 1 )
		{
			char pRelativeEffectPath[512];
			strcpy ( pRelativeEffectPath, "effectbank\\reloaded\\" );
			strcat ( pRelativeEffectPath, pNewShaderFilename );
			if ( giRememberLastEffectIndexInImporter > 0 ) deleteinternaleffect ( giRememberLastEffectIndexInImporter );
			int iEffectID = loadinternaleffectunique ( pRelativeEffectPath, 1 ); //PE: old effect never deleted. ?
			DeleteObject ( t.importer.objectnumber );
			CloneObject ( t.importer.objectnumber, t.importer.objectnumberpreeffectcopy );
			//ReverseObjectFrames ( t.importer.objectnumber ); // hair rendered last
			importer_applyimagelisttextures ( true, -1, false );
			importer_recreate_texturesprites();
			LockObjectOn ( t.importer.objectnumber );

			//DisableObjectZRead ( t.importer.objectnumber ); // messes up import preview!
			//DisableObjectZDepth ( t.importer.objectnumber );
			SetObjectEffect ( t.importer.objectnumber, iEffectID ); 
			//SetEffectTechnique ( iEffectID, "LowestWithCutOutDepth" ); // messes up import preview!
			//SetObjectTransparency ( t.importer.objectnumber, 6 );

			GlueObjectToLimbEx ( t.importer.objectnumber, t.importerGridObject[8], 0 , 1 );
			giRememberLastEffectIndexInImporter = iEffectID;
			//PE: Bug. reset effect clip , so visible.
			t.tnothing = MakeVector4(g.characterkitvector);
			SetVector4(g.characterkitvector, 500000, 1, 0, 0);
			SetEffectConstantV(iEffectID, "EntityEffectControl", g.characterkitvector);
			t.tnothing = DeleteVector4(g.characterkitvector);
		}
	}
}

void importer_loadmodel ( void )
{
	// show loading prompt (FBX conversion can take a while)
	popup_text("importing and converting model");

	// init
	if ( t.importer.importerActive == 0 ) importer_init ( );
	importer_sort_names ( );
	for ( t.t = 0 ; t.t<=  50; t.t++ )
	{
		t.snapPosX_f[t.t][1] = -99999;
	}

	// check if importing an FBX for first time
	bool bHasFBXExtension = false;
	if ( strnicmp ( t.importer.objectFilename.Get() + strlen(t.importer.objectFilename.Get()) - 4, ".fbx", 4 ) == NULL ) bHasFBXExtension = true;
	if ( bHasFBXExtension == true && g_iFirstTimeFBXImport == 0 )
	{
		// set-up importer for FBX friendly import settings
		//g_iFBXGeometryToggleMode = 1;
		//g_iFBXGeometryCenterMesh = 1;
		g_iPreferPBR = 1;
		g_iFirstTimeFBXImport = 1;
	}

	//  load model specified by timporterfile$
	g_bLoadedFBXModel = false;
	t.importer.objectnumber=g.importermenuobjectoffset+1;
	t.importer.objectnumberpreeffectcopy=g.importermenuobjectoffset+3;
	t.strwork = ""; t.strwork = t.strwork + t.importer.objectFileOriginalPath + t.importer.objectFilename;
	if ( FileExist ( t.strwork.Get() ) ) 
	{
		if ( strnicmp ( t.strwork.Get() + strlen(t.strwork.Get()) - 4, ".fbx", 4 )==NULL )
		{
			LoadFBX ( t.strwork.Get(), t.importer.objectnumber );
			//SetObjectRenderMatrixMode ( t.importer.objectnumber, 1 );
			g_bLoadedFBXModel = true;
		}
		else
		{
			LoadObject ( t.strwork.Get() ,t.importer.objectnumber );
			//SetObjectRenderMatrixMode ( t.importer.objectnumber, 0 );
		}
	}
	else
	{
		t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\"+t.importer.objectFilename;
		if (  FileExist( t.strwork.Get() ) ) 
		{
			if ( strnicmp ( t.strwork.Get() + strlen(t.strwork.Get()) - 4, ".fbx", 4 )==NULL )
			{
				LoadFBX ( t.strwork.Get(), t.importer.objectnumber );
				//SetObjectRenderMatrixMode ( t.importer.objectnumber, 1 );
				g_bLoadedFBXModel = true;
			}
			else
			{
				LoadObject ( t.strwork.Get() ,t.importer.objectnumber );
				//SetObjectRenderMatrixMode ( t.importer.objectnumber, 0 );
			}
		}
		else
		{
			MakeObjectCube (  t.importer.objectnumber,100 );
		}
	}

	// 111118 - if import fails for any reason, ensure no crash
	if ( ObjectExist ( t.importer.objectnumber ) == 0 )
	{
		MakeObjectCube (  t.importer.objectnumber,100 );
	}

	// Position imported object at center
	LockObjectOn (  t.importer.objectnumber );
	PositionObject (  t.importer.objectnumber , 0 , 0 , 0 );

	//  Dummy character object
	t.importer.dummyCharacterObjectNumber=g.importermenuobjectoffset+2;
	if (  ObjectExist(t.importer.dummyCharacterObjectNumber)  )  DeleteObject (  t.importer.dummyCharacterObjectNumber );
	LoadObject (  "entitybank\\characters\\Uber Soldier.X",t.importer.dummyCharacterObjectNumber );
	TextureObject (  t.importer.dummyCharacterObjectNumber,0 );
	DisableObjectZDepth (  t.importer.dummyCharacterObjectNumber );
	LockObjectOn (  t.importer.dummyCharacterObjectNumber );
	ColorObject (  t.importer.dummyCharacterObjectNumber , Rgb(0,0,0) );
	SetObjectLight (  t.importer.dummyCharacterObjectNumber ,1 );
	SetObjectAmbience (  t.importer.dummyCharacterObjectNumber,0 );
	SetAlphaMappingOn (  t.importer.dummyCharacterObjectNumber , 30 );
	SetObjectEmissive (  t.importer.dummyCharacterObjectNumber, Rgb(255,0,0) );
	SetObjectEffect ( t.importer.dummyCharacterObjectNumber, g.guishadereffectindex );

	//  Set the pivot to the bottom of the object
	t.tTop_f = -10000;
	t.tBottom_f = 10000;
	PerformCheckListForLimbs (  t.importer.objectnumber );

	//  Find out the max and min Y vertex positions so we can work out where the pivot is now
	for ( t.tLimbCount = 1 ; t.tLimbCount<=  ChecklistQuantity()-1; t.tLimbCount++ )
	{
		LockVertexDataForLimb (  t.importer.objectnumber, t.tLimbCount );
		t.tVmax=GetVertexDataVertexCount();
		for ( t.tVertexCount = 0 ; t.tVertexCount<=  t.tVmax-1; t.tVertexCount++ )
		{
			t.tVertexY_f = GetVertexDataPositionY(t.tVertexCount);
			if (  t.tVertexY_f > t.tTop_f  )  t.tTop_f  =  t.tVertexY_f;
			if (  t.tVertexY_f < t.tBottom_f  )  t.tBottom_f  =  t.tVertexY_f;
		}
		UnlockVertexData (  );
	}

	//  MoveObject (  pivot to the bottom of the model for use in FPSC )
	t.importer.objectLeftOffset = 0;
	t.importer.objectFrontOffset = 0;
	t.importer.objectBottomOffset = 0;
	t.importer.objectTopOffset = 0;
	t.importer.objectCenterOffset = 0;

	//  calculate the radius that would encase the whole object (new mode 2 takes OriginalMatrix scaling into account)
	t.tObjectSize_f = ObjectSizeY(t.importer.objectnumber,2);
	if (  ObjectSizeX (t.importer.objectnumber,2) > t.tObjectSize_f  ) t.tObjectSize_f = ObjectSizeX (t.importer.objectnumber,2);
	if (  ObjectSizeZ (t.importer.objectnumber,2) > t.tObjectSize_f  ) t.tObjectSize_f = ObjectSizeZ (t.importer.objectnumber,2);
	t.importer.originalObjectSize = t.tObjectSize_f;

	//  Check the size and adjust scale to fit in view
	t.tScale = ( 300.00 / t.tObjectSize_f ) * 100.0;
	t.tScale = t.tScale * t.importer.scaleMulti;
	t.importer.objectScaleForEditing = t.tScale;
	t.importer.objectCenterOffset = -(ObjectSizeY (t.importer.objectnumber,0) / 2.0) * (t.tScale / 100.0) ;
	ScaleObject (  t.importer.objectnumber , t.tScale , t.tScale , t.tScale );
	t.tObjectSizeX_f = (ObjectSizeX(t.importer.objectnumber,0) * (t.tScale / 100.0)) * 0.95;
	t.tObjectSizeY_f = (ObjectSizeY(t.importer.objectnumber,0) * (t.tScale / 100.0)) * 0.95;
	t.tObjectSizeZ_f = (ObjectSizeZ(t.importer.objectnumber,0) * (t.tScale / 100.0)) * 0.95;
	PositionObject ( t.importer.objectnumber, 0, 0, 0 );

	t.tBoxOffsetLeft_f = 0;
	t.tBoxOffsetTop_f = 0;
	t.tBoxOffsetFront_f = 0;
	ScaleObject (  t.importer.dummyCharacterObjectNumber , t.tScale , t.tScale , t.tScale * 0.2 );
	PositionObject ( t.importer.dummyCharacterObjectNumber, 0, 0, 0 );

	//  Create the grid below the character to help show the camera rotation
	int tCount = 1;
	for ( tCount = 1 ; tCount<= 6; tCount++ )
	{
		t.importerGridObject[tCount] = findFreeObject();
		MakeObjectPlane (  t.importerGridObject[tCount], 0 , 400 * t.importer.scaleMulti );
		PositionObject ( t.importerGridObject[tCount], 0, 0, 0 );
		SetObjectWireframe (  t.importerGridObject[tCount] , 1 );
		ColorObject (  t.importerGridObject[tCount],Rgb(0,0,0) );
		SetObjectAmbience (  t.importerGridObject[tCount],0 );
		SetObjectEmissive (  t.importerGridObject[tCount], Rgb(0,0,0) );
		SetObjectLight (  t.importerGridObject[tCount] , 1 );
		XRotateObject (  t.importerGridObject[tCount],180 );
		LockObjectOn (  t.importerGridObject[tCount] );
		SetObjectEffect ( t.importerGridObject[tCount], g.guishadereffectindex );
	}

	//  Front marker
	t.importerGridObject[7] = findFreeObject();
	MakeObjectPlane (  t.importerGridObject[7], 10 , 20 * t.importer.scaleMulti );
	PositionObject ( t.importerGridObject[7], 0, 0, 0 );
	ColorObject (  t.importerGridObject[7],Rgb(0,0,0) );
	SetObjectLight (  t.importerGridObject[7] , 1 );
	SetObjectAmbience (  t.importerGridObject[7],0 );
	SetObjectEmissive (  t.importerGridObject[7], Rgb(0,0,0) );
	MoveObject (  t.importerGridObject[7], 200 );
	XRotateObject (  t.importerGridObject[7], -90 );
	LockObjectOn (  t.importerGridObject[7] );
	SetObjectEffect ( t.importerGridObject[7], g.guishadereffectindex );

	//  Position the grid
	MoveObjectLeft (  t.importerGridObject[1],200 * t.importer.scaleMulti );
	XRotateObject (  t.importerGridObject[1] , -90 );
	MoveObjectRight (  t.importerGridObject[2],200 * t.importer.scaleMulti );
	XRotateObject (  t.importerGridObject[2] , -90 );
	MoveObject (  t.importerGridObject[3],200 * t.importer.scaleMulti );
	YRotateObject (  t.importerGridObject[3],90 );
	XRotateObject (  t.importerGridObject[3] , -90 );
	MoveObject (  t.importerGridObject[4],-200 * t.importer.scaleMulti );
	YRotateObject (  t.importerGridObject[4],90 );
	XRotateObject (  t.importerGridObject[4] , -90 );
	YRotateObject (  t.importerGridObject[5],90 );
	XRotateObject (  t.importerGridObject[5] , -90 );
	XRotateObject (  t.importerGridObject[6] , -90 );

	t.importerGridObject[8] = findFreeObject();
	MakeObjectCube (  t.importerGridObject[8] , 1 );
	HideObject (  t.importerGridObject[8] );
	SetObjectLight (  t.importerGridObject[8],1 );
	LockObjectOn (  t.importerGridObject[8] );
	SetObjectEffect ( t.importerGridObject[8], g.guishadereffectindex );
	for ( tCount = 1 ; tCount<=  7; tCount++ )
	{
		FixObjectPivot (  t.importerGridObject[tCount] );
		GlueObjectToLimbEx (  t.importerGridObject[tCount], t.importerGridObject[8], 0 , 1 );
	}
	GlueObjectToLimbEx (  t.importer.dummyCharacterObjectNumber, t.importerGridObject[8], 0 , 1 );

	//  SetObject (  rotation mode and initial angle (angle may change if a fpe is associated with the model) )
	t.importer.objectRotateMode = 0;
	t.importer.objectAngleY = 0.0;

	//  create pivot for collision boxes
	t.importerGridObject[9] = findFreeObject();
	MakeObjectCube (  t.importerGridObject[9] , 1 );
	SetObjectLight (  t.importerGridObject[9],1 );
	ColorObject (  t.importerGridObject[9],Rgb(0,0,0) );
	HideObject (  t.importerGridObject[9] );
	LockObjectOn (  t.importerGridObject[9] );
	for ( tCount = 1 ; tCount<=  9; tCount++ )
	{
		SetObjectEmissive ( t.importerGridObject[tCount], Rgb(255,255,0) );
		SetObjectEffect ( t.importerGridObject[tCount], g.guishadereffectindex );
	}

	//  Check for FPE file, load and apply if one is found
	importer_load_fpe ( );

	// Autoset other FPE settings (if FBX import for first time)
	if ( g_iPreferPBR > 0 )
	{
		t.importer.objectFPE.effect = "effectbank\\reloaded\\apbr_basic.fx";
	}

	// update UI panel of any changes so far
	importer_apply_fpe();

	// ensure no value from emissive effect
	SetObjectEmissive ( t.importer.objectnumber, 0 );

	// reset image to texture limb mapping array
	g_dwMapImageListIndexToLimbCount = 0;
	SAFE_DELETE ( g_piMapImageListIndexToLimbIndex );

	// if FBX import, add appropriate texture files to importerlist
	if ( bHasFBXExtension == true )
	{
		// Clean importer Textures List
		int tCount = 0;
		for ( tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
		{
			t.importerTextures[tCount].imageID = 0;
			t.importerTextures[tCount].fileName = "";
			t.importerTextures[tCount].iExpandedThisSlot = 0;		
			t.importerTextures[tCount].iOptionalStage = 0;
			t.importerTextures[tCount].iAssociatedBaseImage = 0;
		}

		// first check if importer has textures from FBX conversion
		cstr sOldDir = GetDir();
		SetDir(cstr(g.rootdir_s+"importer\\temp").Get());
		bool bUseTexturesFromConversion = false;
		sObject* pObject = GetObjectData ( t.importer.objectnumber );
		if ( pObject )
		{
			tCount = 0;
			for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iMesh];
				int iStoreImageSlotIndexForBase = -1;
				for ( int iTextureStage = 0; iTextureStage < pMesh->dwTextureCount; iTextureStage++ )
				{
					int iSlotUsed = -1;
					cstr pTextureFile = pMesh->pTextures[iTextureStage].pName;
					if ( FileExist ( pTextureFile.Get() ) == 0 )
					{
						pTextureFile = Left ( pTextureFile.Get(), strlen(pTextureFile.Get())-4 );
						pTextureFile = pTextureFile + ".dds";
					}
					if ( FileExist ( pTextureFile.Get() ) == 1 )
					{
						cstr pAbsoluteFile = g.rootdir_s + "importer\\temp\\" + pTextureFile;
						iSlotUsed = importer_addtexturefiletolist ( pAbsoluteFile.Get(), pAbsoluteFile.Get(), &tCount );
						bUseTexturesFromConversion = true;
					}
					else
					{
						// FBX conversion had no textures produced, so add texture filename directly to importer image list
						// which we can then use later to find associate texture files
						if ( strlen ( pMesh->pTextures[iTextureStage].pName ) > 0 )
						{
							cstr pAbsoluteFile = t.importer.objectFileOriginalPath + pMesh->pTextures[iTextureStage].pName;
							iSlotUsed = importer_addtexturefiletolist ( pAbsoluteFile.Get(), pAbsoluteFile.Get(), &tCount );
						}
					}
					if ( iSlotUsed > -1 )
					{
						if ( iTextureStage == 0 ) 
						{
							// base texture (color)
							iStoreImageSlotIndexForBase = iSlotUsed;
						}
						else
						{
							// associate other stages to above base stage
							t.importerTextures[iSlotUsed].iOptionalStage = iTextureStage;
							t.importerTextures[iSlotUsed].iAssociatedBaseImage = iStoreImageSlotIndexForBase;
						}
					}
				}
			}
			SetDir(sOldDir.Get());
		}
		if ( bUseTexturesFromConversion == true )
		{
			// if model did not have its own textures, texture with external ones next to FBX model
			importer_load_textures_finish ( tCount, true ); // add cube mapping texture stage only
		}
		else
		{
			// if model did not have its own textures, texture with external ones next to FBX model
			importer_load_textures_finish ( tCount, false ); // texture object from import image texture list
		}
	}
	else
	{
		//  Load textures the old fashioned way
		importer_load_textures ( );
	}

	// prior to setting effect (changing it) keep a copy of original import (but after setting textures)
	if ( ObjectExist( t.importer.objectnumberpreeffectcopy ) == 1 ) DeleteObject ( t.importer.objectnumberpreeffectcopy );
	CloneObject ( t.importer.objectnumberpreeffectcopy, t.importer.objectnumber );

	// Once shader wiping tetxure applied, apply shader to imported object (changed later if prefer PBR shader - see below)
	int iEffectID = loadinternaleffect("effectbank\\reloaded\\entity_basic.fx");
	SetObjectEffect ( t.importer.objectnumber, iEffectID );

	//PE: make sure it is changed. fix for: https://github.com/TheGameCreators/GameGuruRepo/issues/123
	importer_changeshader("entity_basic.fx");

	//PE: Bug. make sure we dont get clipped, model was only half visible.
	//reuse g.characterkitvector = 46
	t.tnothing = MakeVector4(g.characterkitvector);
	SetVector4(g.characterkitvector, 500000, 1, 0, 0);
	SetEffectConstantV(iEffectID, "EntityEffectControl", g.characterkitvector);
	t.tnothing = DeleteVector4(g.characterkitvector);

	// attach to gimble so can manipulate imported object
	GlueObjectToLimbEx (  t.importer.objectnumber, t.importerGridObject[8], 0 , 1 );
	PositionObject (  t.importerGridObject[8], 0 , 0 , IMPORTERZPOSITION );

	// if no physics shapes attached to mode, make a default one covering the whole object
	if (  t.importer.collisionShapeCount  ==  0 ) 
	{
		importer_add_collision_box ( );
	}

	//  set up collision object selection boxes
	for ( tCount = 1 ; tCount<=  9; tCount++ )
	{
		t.selectedObjectMarkers[tCount] = findFreeObject();
		MakeObjectCube (  t.selectedObjectMarkers[tCount], 10 );
		SetObjectLight (  t.selectedObjectMarkers[tCount], 1 );
		SetObjectAmbience (  t.selectedObjectMarkers[tCount],0 );
		if (  tCount  !=  9 ) 
		{
			SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(255,255,0) );
		}
		else
		{
			SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(0,255,255) );
		}
		DisableObjectZDepth (  t.selectedObjectMarkers[tCount] );
		LockObjectOn (  t.selectedObjectMarkers[tCount] );
		SetObjectWireframe (  t.selectedObjectMarkers[tCount],1 );
		HideObject (  t.selectedObjectMarkers[tCount] );
		SetObjectEmissive ( t.selectedObjectMarkers[tCount], Rgb(255,255,0) );
		SetObjectEffect ( t.selectedObjectMarkers[tCount], g.guishadereffectindex );
	}

	//  Default selected collision object
	t.importer.selectedCollisionObject = 0;
	t.slidersmenuvalue[t.importer.properties2Index][6].value = ObjectAngleX(t.importerCollision[0].object2);
	t.slidersmenuvalue[t.importer.properties2Index][7].value = ObjectAngleY(t.importerCollision[0].object2);
	t.slidersmenuvalue[t.importer.properties2Index][8].value = ObjectAngleZ(t.importerCollision[0].object2);

	t.importer.scaleMulti = 2.0;
	importer_screenSwitch ( );

	//  Start importer loop
	t.importer.loaded=1;
	importer_fade_in ( );

	// reset camera vars for importer viewer
	float fDefaultCameraHeight = ObjectSizeY(t.importer.objectnumber,1) / 2.0;
	t.importer.cameraheight = fDefaultCameraHeight;
	t.importer.originalcameraheight = fDefaultCameraHeight;
	t.importer.lastcameraheightforshift = fDefaultCameraHeight;
	t.importer.camerazoom = 1.0f;

	// if prefer PBR, effect needs to be switched now
	g_iPreferPBRLateShaderChange = 0;
	if ( g_iPreferPBR > 0 )
	{
		// apply shader to preferred PBR render view
		g_iPreferPBRLateShaderChange = 1;
	}

	// remove editor prompt
	popup_text_close();
}

void importer_RestoreCollisionShiftHeight ( void )
{
	float fShiftForCollisionObjects = t.importer.originalcameraheight - t.importer.lastcameraheightforshift;
	t.importer.lastcameraheightforshift = t.importer.originalcameraheight;
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  t.importerCollision[tCount].object > 0 ) 
		{
			if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
			{
				float fX = ObjectPositionX(t.importerCollision[tCount].object);
				float fY = ObjectPositionY(t.importerCollision[tCount].object) - fShiftForCollisionObjects;
				float fZ = ObjectPositionZ(t.importerCollision[tCount].object);
				PositionObject ( t.importerCollision[tCount].object, fX, fY, fZ );
				PositionObject ( t.importerCollision[tCount].object2, fX, fY, fZ );
			}
		}
	}
}

void importer_loop ( void )
{
	// this gets triggered once importer underway
	if ( g_iPreferPBRLateShaderChange > 0 )
	{
		g_iPreferPBRLateShaderChange--;
		if ( g_iPreferPBRLateShaderChange == 0 ) 
		{
			importer_changeshader ( t.importer.objectFPE.effect.Get() ); Sync();
			importer_changeshader ( t.importer.objectFPE.effect.Get() ); Sync();
		}
	}

	importer_update_scale ( );
	t.inputsys.xmousemove = t.inputsys.xmousemove * 2;
	t.inputsys.ymousemove = t.inputsys.ymousemove * 2;

	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
	
	//  Multiplier to convert mouse coords to importer coords
	t.tadjustedtoimporterxbase_f=GetChildWindowWidth()/800.0;
	t.tadjustedtoimporterybase_f=GetChildWindowHeight()/600.0;
	t.importer.MouseX = t.inputsys.xmouse*t.tadjustedtoimporterxbase_f;
	t.importer.MouseY = t.inputsys.ymouse*t.tadjustedtoimporterybase_f;

	if (  t.timportermouseoff  ==  0 ) 
	{
		t.toldMouseBeforeEditX = t.importer.MouseX + ( GetDesktopWidth() - GetChildWindowWidth() );
		t.toldMouseBeforeEditY = t.importer.MouseY + ( GetDesktopHeight() - GetChildWindowHeight() );
	}

	//  Control gadgets in panel
	if (  t.importer.loaded == 1 ) 
	{
		SetCursor (  0 , 100 );
		int iCameraAdjustMode = 0;
		if ( t.importerTabs[1].selected == 1 ) iCameraAdjustMode = 2;
		if ( t.importerTabs[2].selected == 1 ) iCameraAdjustMode = 1;
		if ( iCameraAdjustMode > 0 )
		{
			float fFinalScale, fFinalScale2;
			if ( t.inputsys.keyup != 0 ) t.importer.cameraheight += 10.0f;
			if ( t.inputsys.keydown != 0 ) t.importer.cameraheight -= 10.0f;
			if ( iCameraAdjustMode == 2 )
			{
				if ( t.inputsys.wheelmousemove > 0 ) t.importer.camerazoom += 0.05f;
				if ( t.inputsys.wheelmousemove < 0 ) t.importer.camerazoom -= 0.05f;
				if ( t.importer.camerazoom < 0.05f ) t.importer.camerazoom = 0.05f;
				fFinalScale = t.importer.objectScaleForEditing * t.importer.camerazoom;
				fFinalScale2 = 100.0f * t.importer.camerazoom;
			}
			else
			{
				fFinalScale = t.importer.objectScaleForEditing;
				fFinalScale2 = 100.0f;
			}
			PositionObject ( t.importer.objectnumber, 0, -t.importer.cameraheight, 0 );
			ScaleObject ( t.importer.objectnumber, fFinalScale, fFinalScale, fFinalScale );
			PositionObject ( t.importer.dummyCharacterObjectNumber, 0, -t.importer.cameraheight, 0 );
			ScaleObject ( t.importer.dummyCharacterObjectNumber, fFinalScale, fFinalScale, fFinalScale * 0.2f );
			for ( int tCount = 1 ; tCount<= 6; tCount++ )
			{
				PositionObject ( t.importerGridObject[tCount], 0, -t.importer.cameraheight, 0 );
				ScaleObject ( t.importerGridObject[tCount], fFinalScale2, fFinalScale2, fFinalScale2 );
			}
			float fShiftForCollisionObjects = t.importer.cameraheight - t.importer.lastcameraheightforshift;
			t.importer.lastcameraheightforshift = t.importer.cameraheight;
			for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
			{
				if (  t.importerCollision[tCount].object > 0 ) 
				{
					if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
					{
						float fX = ObjectPositionX(t.importerCollision[tCount].object);
						float fY = ObjectPositionY(t.importerCollision[tCount].object) - fShiftForCollisionObjects;
						float fZ = ObjectPositionZ(t.importerCollision[tCount].object);
						PositionObject ( t.importerCollision[tCount].object, fX, fY, fZ );
						PositionObject ( t.importerCollision[tCount].object2, fX, fY, fZ );
					}
				}
			}
		}
		else
		{
			// viewing controls only for main panel
			float fFinalScale = t.importer.objectScaleForEditing;
			float fDefaultCameraHeight = -(ObjectSizeY(t.importer.objectnumber,0)/2.0);
			PositionObject ( t.importer.objectnumber, 0, fDefaultCameraHeight, 0 );
			ScaleObject ( t.importer.objectnumber, fFinalScale, fFinalScale, fFinalScale );
			PositionObject ( t.importer.dummyCharacterObjectNumber, 0, fDefaultCameraHeight, 0 );
			ScaleObject ( t.importer.dummyCharacterObjectNumber, fFinalScale, fFinalScale, fFinalScale * 0.2f );
			fFinalScale = 100.0f;
			for ( int tCount = 1 ; tCount<= 6; tCount++ )
			{
				PositionObject ( t.importerGridObject[tCount], 0, fDefaultCameraHeight, 0 );
				ScaleObject ( t.importerGridObject[tCount], fFinalScale, fFinalScale, fFinalScale );
			}
			importer_RestoreCollisionShiftHeight();
		}

		importer_help ( );

		//  This is to get round a gimble lock issue
		RotateObject (  t.importerGridObject[9],0,0,0 );

		if (  t.importerTabs[5].selected  ==  1 ) 
		{
			t.importerTabs[5].selected = 0;
			t.timportersaveon = 1;
			importer_save_entity ( );
		}

		if (  t.importer.helpShow  ==  0  )  t.importer.message  =  "Click and drag to rotate";

		//  Update or hide selection markers
		importer_update_selection_markers ( );

		//  Tab One Mode - settings
		if (  g.tabmode  ==  IMPORTERTABPAGE1 ) 
		{
			SetAlphaMappingOn (  t.importer.objectnumber , 100 );
			SetObjectTransparency (  t.importer.objectnumber, 255 );

			//  Front view
			YRotateObject (   t.importerGridObject[8] , 0 );
			XRotateObject (   t.importerGridObject[8] , 0 );
			YRotateObject (   t.importerGridObject[9] , 0 );
			XRotateObject (   t.importerGridObject[9] , 0 );

			// only if not over the properties panel
			if ( t.importer.MouseX > t.slidersmenu[t.importer.properties1Index].tleft 
			&&	 t.importer.MouseY > t.slidersmenu[t.importer.properties1Index].ttop )	
			{
				// mouse within properties panel, do not rotate object on click
			}
			else
			{
				//  Rotation control
				switch (  t.importer.objectRotateMode ) 
				{
					case 0:

						if (  t.inputsys.mclick  ==  1 && t.importer.oldMouseClick  ==  0 ) 
						{
							#ifdef DX11
							float fMX = (GetDisplayWidth()+0.0) / 800.0f;
							float fMY = (GetDisplayHeight()+0.0) / 600.0f;
							t.tadjustedtoareax_f = t.inputsys.xmouse*fMX;
							t.tadjustedtoareay_f = t.inputsys.ymouse*fMY;
							#else
							t.tadjustedtoareax_f = t.importer.MouseX;
							t.tadjustedtoareay_f = t.importer.MouseY;
							#endif
							//if (  PickScreenObjectEx ( t.importer.MouseX , t.importer.MouseY , t.importer.objectnumber , t.importer.objectnumber , 1 )  !=  0 )
							if ( PickScreenObjectEx ( t.tadjustedtoareax_f, t.tadjustedtoareay_f, t.importer.objectnumber, t.importer.objectnumber, 1 )  !=  0 )
							{
								t.importer.objectRotateMode = 1;
								t.tnothing = t.inputsys.xmousemove;
								importer_hide_mouse ( );
							}
						}

					break;
					case 1:

						if (  t.inputsys.mclick  ==  0 ) 
						{
							t.importer.objectRotateMode = 0;
							importer_show_mouse ( );
						}
						else
						{
							t.importer.message = "How down shift to lock to 45' Steps";
							if (  t.inputsys.keyshift  ==  0 ) 
							{
								t.importer.objectAngleY -= t.inputsys.xmousemove;
							}
							else
							{
								t.importer.message = "Locked to 45' steps";
								t.importer.mouseMoveSnap -= t.inputsys.xmousemove;
								if (  t.importer.mouseMoveSnap < -30 )  //was -100
								{
									t.importer.mouseMoveSnap = 0;
									t.importer.objectAngleY -= 45;
									if (  t.importer.objectAngleY > 0 && t.importer.objectAngleY < 45  )  t.importer.objectAngleY  =  0;
									if (  t.importer.objectAngleY > 45 && t.importer.objectAngleY < 90  )  t.importer.objectAngleY  =  45;
									if (  t.importer.objectAngleY > 90 && t.importer.objectAngleY < 135  )  t.importer.objectAngleY  =  90;
									if (  t.importer.objectAngleY > 135 && t.importer.objectAngleY < 225  )  t.importer.objectAngleY  =  135;
									if (  t.importer.objectAngleY > 225 && t.importer.objectAngleY < 315  )  t.importer.objectAngleY  =  225;
									if (  t.importer.objectAngleY > 315 && t.importer.objectAngleY  )  t.importer.objectAngleY  =  315;
								}
								if (  t.importer.mouseMoveSnap > 30 ) // was 100
								{
									t.importer.mouseMoveSnap = 0;
									t.importer.objectAngleY += 45;
									if (  t.importer.objectAngleY > 0 && t.importer.objectAngleY < 45  )  t.importer.objectAngleY  =  45;
									if (  t.importer.objectAngleY > 45 && t.importer.objectAngleY < 90  )  t.importer.objectAngleY  =  90;
									if (  t.importer.objectAngleY > 90 && t.importer.objectAngleY < 135  )  t.importer.objectAngleY  =  135;
									if (  t.importer.objectAngleY > 135 && t.importer.objectAngleY < 225  )  t.importer.objectAngleY  =  225;
									if (  t.importer.objectAngleY > 225 && t.importer.objectAngleY < 315  )  t.importer.objectAngleY  =  315;
									if (  t.importer.objectAngleY > 315 && t.importer.objectAngleY  )  t.importer.objectAngleY  =  0;
								}
							}

							//  Ensure angle is 0-360
							while (  t.importer.objectAngleY > 360 ) 
							{
								t.importer.objectAngleY -= 360;
							}
							while (  t.importer.objectAngleY < 0 ) 
							{
								t.importer.objectAngleY += 360;
							}

							//  Update slider based on object angle
							t.slidersmenuvalue[t.importer.properties1Index][3].value = t.importer.objectAngleY;
						}
					break;
				}
			}

			//  update angle base on slider
			t.importer.objectAngleY = t.slidersmenuvalue[t.importer.properties1Index][3].value;
			YRotateObject (  t.importer.objectnumber,t.importer.objectAngleY );
		}

		//  Tab Two Mode - collision
		if (  g.tabmode  ==  IMPORTERTABPAGE2 ) 
		{
			t.importer.message = "Collision";
			SetAlphaMappingOn (  t.importer.objectnumber , 40 );

			//Views Shortcut keys
			if (  t.inputsys.kscancode  ==  37 ) 
			{
				t.slidersmenuvalue[t.importer.properties2Index][1].value = 2;
				t.slidersmenuvalue[t.importer.properties2Index][1].value_s = "Left";
			}
			if (  t.inputsys.kscancode  ==  39 ) 
			{
				t.slidersmenuvalue[t.importer.properties2Index][1].value = 3;
				t.slidersmenuvalue[t.importer.properties2Index][1].value_s = "Right";
			}
			if (  t.inputsys.kscancode  ==  38 ) 
			{
				t.slidersmenuvalue[t.importer.properties2Index][1].value = 4;
				t.slidersmenuvalue[t.importer.properties2Index][1].value_s = "Top";
			}
			if (  t.inputsys.kscancode  ==  40 ) 
			{
				t.slidersmenuvalue[t.importer.properties2Index][1].value = 1;
				t.slidersmenuvalue[t.importer.properties2Index][1].value_s = "Front";
			}

			// Update Views
			if (  t.slidersmenuvalue[t.importer.properties2Index][1].value  ==  2 ) 
			{
				YRotateObject (   t.importerGridObject[8] , -90 );
				XRotateObject (   t.importerGridObject[8] , 0 );
				YRotateObject (  t.importerGridObject[9] , -90 );
				XRotateObject (   t.importerGridObject[9] , 0 );
				t.importer.viewMessage = "Left";
			}
			if (  t.slidersmenuvalue[t.importer.properties2Index][1].value  ==  3 ) 
			{
				YRotateObject (   t.importerGridObject[8] , 90 );
				XRotateObject (   t.importerGridObject[8] , 0 );
				YRotateObject (  t.importerGridObject[9] , 90 );
				XRotateObject (   t.importerGridObject[9] , 0 );
				t.importer.viewMessage = "Right";
			}
			if (  t.slidersmenuvalue[t.importer.properties2Index][1].value  ==  4 ) 
			{
				YRotateObject (   t.importerGridObject[8] , 0 );
				XRotateObject (   t.importerGridObject[8] , -90 );
				YRotateObject (  t.importerGridObject[9] , 0 );
				XRotateObject (   t.importerGridObject[9] , -90 );
				t.importer.viewMessage = "Top";
			}
			if (  t.slidersmenuvalue[t.importer.properties2Index][1].value  ==  1 ) 
			{
				YRotateObject (   t.importerGridObject[8] , 0 );
				XRotateObject (   t.importerGridObject[8] , 0 );
				YRotateObject (  t.importerGridObject[9] , 0 );
				XRotateObject (   t.importerGridObject[9] , 0 );
				t.importer.viewMessage = "Front";
			}

			//  Update collision pivot, using turn object (Sin ( ce we are rotating this x and y to Line (  up with the grid, then need to to rotate it with the child object ) )
			//  This gets round a gimble lock issue that occurs from using euler, resseting the rotation at the begnining of the loop sorts things out
			YRotateObject (  t.importer.objectnumber,0 );
			importer_check_for_physics_changes ( );
			importer_update_selection_markers ( );
		}

		if (  g.tabmode  ==  IMPORTERTABPAGE3 ) 
		{
			SetAlphaMappingOn (  t.importer.objectnumber , 100 );
			SetObjectTransparency (  t.importer.objectnumber, 255 );

			//  Front view
			YRotateObject (   t.importerGridObject[8] , 0 );
			XRotateObject (   t.importerGridObject[8] , 0 );
			YRotateObject (   t.importerGridObject[9] , 0 );
			XRotateObject (   t.importerGridObject[9] , 0 );

			//  Rotation control
			switch (  t.importer.objectRotateMode ) 
			{
				case 0:

					if (  t.inputsys.mclick  ==  1 && t.importer.oldMouseClick  ==  0 ) 
					{
						if (  PickScreenObjectEx ( t.importer.MouseX , t.importer.MouseY , t.importer.objectnumber , t.importer.objectnumber , 1 )  !=  0 ) 
						{
							t.importer.objectRotateMode = 1;
							t.tnothing = t.inputsys.xmousemove;
							importer_hide_mouse ( );
						}
					}

				break;
				case 1:

					if (  t.inputsys.mclick  ==  0 ) 
					{
						t.importer.objectRotateMode = 0;
						importer_show_mouse ( );
					}
					else
					{
						t.importer.message = "How down shift to lock to 45' Steps";
						if (  t.inputsys.keyshift  ==  0 ) 
						{
							t.importer.objectAngleY2 -= t.inputsys.xmousemove;
						}
						else
						{
							t.importer.message = "Locked to 45' steps";
							t.importer.mouseMoveSnap -= t.inputsys.xmousemove;
							if (  t.importer.mouseMoveSnap < -100 ) 
							{

								t.importer.mouseMoveSnap = 0;
								t.importer.objectAngleY2 -= 45;

								if (  t.importer.objectAngleY2 > 0 && t.importer.objectAngleY2 < 45  )  t.importer.objectAngleY2  =  0;
								if (  t.importer.objectAngleY2 > 45 && t.importer.objectAngleY2 < 90  )  t.importer.objectAngleY2  =  45;
								if (  t.importer.objectAngleY2 > 90 && t.importer.objectAngleY2 < 135  )  t.importer.objectAngleY2  =  90;
								if (  t.importer.objectAngleY2 > 135 && t.importer.objectAngleY2 < 225  )  t.importer.objectAngleY2  =  135;
								if (  t.importer.objectAngleY2 > 225 && t.importer.objectAngleY2 < 315  )  t.importer.objectAngleY2  =  225;
								if (  t.importer.objectAngleY2 > 315 && t.importer.objectAngleY2  )  t.importer.objectAngleY2  =  315;

							}
							if (  t.importer.mouseMoveSnap > 100 ) 
							{

								t.importer.mouseMoveSnap = 0;
								t.importer.objectAngleY2 += 45;

								if (  t.importer.objectAngleY2 > 0 && t.importer.objectAngleY2 < 45  )  t.importer.objectAngleY2  =  45;
								if (  t.importer.objectAngleY2 > 45 && t.importer.objectAngleY2 < 90  )  t.importer.objectAngleY2  =  90;
								if (  t.importer.objectAngleY2 > 90 && t.importer.objectAngleY2 < 135  )  t.importer.objectAngleY2  =  135;
								if (  t.importer.objectAngleY2 > 135 && t.importer.objectAngleY2 < 225  )  t.importer.objectAngleY2  =  225;
								if (  t.importer.objectAngleY2 > 225 && t.importer.objectAngleY2 < 315  )  t.importer.objectAngleY2  =  315;
								if (  t.importer.objectAngleY2 > 315 && t.importer.objectAngleY2  )  t.importer.objectAngleY2  =  0;
							}
						}

						//  Ensure angle is 0-360
						while (  t.importer.objectAngleY2 > 360 ) 
						{
							t.importer.objectAngleY2 -= 360;
						}
						while (  t.importer.objectAngleY2 < 0 ) 
						{
							t.importer.objectAngleY2 += 360;
						}

						//  Update slider based on object angle
						t.slidersmenuvalue[t.importer.properties3Index][2].value = t.importer.objectAngleY2;
					}

				break;

			}

			//  update angle base on slider
			t.importer.objectAngleY2 = t.slidersmenuvalue[t.importer.properties3Index][2].value;
			RotateObject (  t.importer.objectnumber,t.slidersmenuvalue[t.importer.properties3Index][1].value,t.slidersmenuvalue[t.importer.properties3Index][2].value,t.slidersmenuvalue[t.importer.properties3Index][3].value );
		}

		if (  t.importer.showCollisionOnly  ==  1 ) 
		{
			t.importer.message = "Showing Collision Only";
			importer_ShowCollisionOnly ( );
		}

		//  handle scale
		importer_handleScale ( );

		//  update tabs
		importer_tabs_update ( );

		//  update textures
		importer_update_textures ( );

		//  handle exit (260416 - added trigger reload of model for FBX core flags)
		if (  ControlKey() == 1 || t.importer.cancel == 1 )//|| g_iTriggerReloadOfImportModel == 1 ) 
		{
			--t.importer.cancelCount;
			if ( t.importer.cancelCount <= 0 )//|| g_iTriggerReloadOfImportModel == 1 ) 
			{
				importer_quit ( );
				//if ( g_iTriggerReloadOfImportModel == 1 ) 
				//{
				//	g_iTriggerReloadOfImportModel = 0;
				//	importer_loadmodel ( );
				//}
			}
		}
	}

	// escape importer
	if (  t.inputsys.kscancode  ==  27 && t.importer.oldShowCollision  ==  0 ) 
	{
		t.importer.showCollisionOnly = 1 - t.importer.showCollisionOnly;
		t.importer.oldShowCollision = 1;
		if ( t.importer.showCollisionOnly == 0 ) importer_ShowCollisionOnlyOff ( );
	}
	if (  t.inputsys.kscancode  !=  27  )  t.importer.oldShowCollision  =  0;
	t.importer.oldMouseClick = t.inputsys.mclick;

	//  110315 - 019 - fix for mouse dissapearing
	if (  t.timportersaveon  ==  1 ) 
	{
		importer_show_mouse ( );
	}
}

void importer_update_selection_markers ( void )
{
	//  Show / hide collision depending which tab is selected
	if (  t.importerTabs[2].selected  ==  0 ) 
	{
		for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
		{
			if (  t.importerCollision[tCount].object > 0 ) 
			{
				if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
				{
					HideObject (  t.importerCollision[tCount].object );
					HideObject (  t.importerCollision[tCount].object2 );
				}
			}
		}
		for ( t.tCount2 = 1 ; t.tCount2<=  9; t.tCount2++ )
		{
			if (  t.selectedObjectMarkers[t.tCount2] > 0 ) 
			{
				if (  ObjectExist(t.selectedObjectMarkers[t.tCount2])  ==  1  )  HideObject (  t.selectedObjectMarkers[t.tCount2] );
			}
		}
	}
	else
	{
		if (  t.importer.collisionShapeCount > 0 ) 
		{
			if (  t.importerCollision[t.importer.selectedCollisionObject].object > 0 && t.importerCollision[t.importer.selectedCollisionObject].object2 > 0 ) 
			{
				if (  ObjectExist(t.importerCollision[t.importer.selectedCollisionObject].object)  ==  1 && ObjectExist(t.importerCollision[t.importer.selectedCollisionObject].object2)  ==  1 ) 
				{
					t.importerCollision[t.importer.selectedCollisionObject].rotx = t.slidersmenuvalue[t.importer.properties2Index][7].value;
					t.importerCollision[t.importer.selectedCollisionObject].roty = t.slidersmenuvalue[t.importer.properties2Index][8].value;
					t.importerCollision[t.importer.selectedCollisionObject].rotz = t.slidersmenuvalue[t.importer.properties2Index][9].value;
					RotateObject (  t.importerCollision[t.importer.selectedCollisionObject].object, t.importerCollision[t.importer.selectedCollisionObject].rotx, t.importerCollision[t.importer.selectedCollisionObject].roty, t.importerCollision[t.importer.selectedCollisionObject].rotz );
					RotateObject (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.importerCollision[t.importer.selectedCollisionObject].rotx, t.importerCollision[t.importer.selectedCollisionObject].roty, t.importerCollision[t.importer.selectedCollisionObject].rotz );
				}
			}
		}
		if (  t.importer.collisionShapeCount  ==  0 ) 
		{
			for ( t.tCount2 = 1 ; t.tCount2<=  9; t.tCount2++ )
			{
				if (  t.selectedObjectMarkers[t.tCount2] > 0 ) 
				{
					if (  ObjectExist(t.selectedObjectMarkers[t.tCount2])  ==  1  )  HideObject (  t.selectedObjectMarkers[t.tCount2] );
				}
			}
			return;
		}
		for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
		{
			if (  t.importerCollision[tCount].object > 0 ) 
			{
				if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
				{
					ShowObject (  t.importerCollision[tCount].object2 );
					if (  t.importer.selectedCollisionObject  ==  tCount ) 
					{
						for ( t.tCount2 = 1 ; t.tCount2<=  9; t.tCount2++ )
						{
							ShowObject (  t.selectedObjectMarkers[t.tCount2] );
							RotateObject (  t.selectedObjectMarkers[t.tCount2],0,0,0 );
						}

						//  position pivot at 0,0,0 so we can glue things to it
						PositionObject ( t.importerGridObject[9], 0, 0, 0 );

						//  top front left
						UnGlueObject (  t.selectedObjectMarkers[1] );
						PositionObject (  t.selectedObjectMarkers[1], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[1], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) );
						MoveObjectUp (  t.selectedObjectMarkers[1], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[1], -LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectLeft (  t.selectedObjectMarkers[1], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[1], t.importerGridObject[9],0 );

						//  top front right
						UnGlueObject (  t.selectedObjectMarkers[2] );
						PositionObject (  t.selectedObjectMarkers[2], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[2], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) );
						MoveObjectUp (  t.selectedObjectMarkers[2], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[2], -LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectRight (  t.selectedObjectMarkers[2], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[2], t.importerGridObject[9],0 );

						//  bottom front left
						UnGlueObject (  t.selectedObjectMarkers[3] );
						PositionObject (  t.selectedObjectMarkers[3], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[3], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) ) ;
						MoveObjectDown (  t.selectedObjectMarkers[3], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[3], -LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectLeft (  t.selectedObjectMarkers[3], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[3], t.importerGridObject[9],0 );

						//  bottom front right
						UnGlueObject (  t.selectedObjectMarkers[4] );
						PositionObject (  t.selectedObjectMarkers[4], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) )  ;
						RotateObject (  t.selectedObjectMarkers[4], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) ) ;
						MoveObjectDown (  t.selectedObjectMarkers[4], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[4], -LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectRight (  t.selectedObjectMarkers[4], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[4], t.importerGridObject[9],0 );

						//  top back left
						UnGlueObject (  t.selectedObjectMarkers[5] );
						PositionObject (  t.selectedObjectMarkers[5], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[5], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) );
						MoveObjectUp (  t.selectedObjectMarkers[5], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[5], LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectLeft (  t.selectedObjectMarkers[5], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[5], t.importerGridObject[9],0 );

						//  top back right
						UnGlueObject (  t.selectedObjectMarkers[6] );
						PositionObject (  t.selectedObjectMarkers[6], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[6], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) ) ;
						MoveObjectUp (  t.selectedObjectMarkers[6], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[6], LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectRight (  t.selectedObjectMarkers[6], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[6], t.importerGridObject[9],0 );

						//  bottom back left
						UnGlueObject (  t.selectedObjectMarkers[7] );
						PositionObject (  t.selectedObjectMarkers[7], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[7], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) );
						MoveObjectDown (  t.selectedObjectMarkers[7], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[7], LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectLeft (  t.selectedObjectMarkers[7], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[7], t.importerGridObject[9],0 );

						//  bottom back right
						UnGlueObject (  t.selectedObjectMarkers[8] );
						PositionObject (  t.selectedObjectMarkers[8], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[8], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) );
						MoveObjectDown (  t.selectedObjectMarkers[8], LimbScaleY(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObject (  t.selectedObjectMarkers[8], LimbScaleZ(t.importerCollision[tCount].object2,0) / 2.0 );
						MoveObjectRight (  t.selectedObjectMarkers[8], LimbScaleX(t.importerCollision[tCount].object2,0) / 2.0 );
						GlueObjectToLimb (  t.selectedObjectMarkers[8], t.importerGridObject[9],0 );

						//  center
						UnGlueObject (  t.selectedObjectMarkers[9] );
						PositionObject (  t.selectedObjectMarkers[9], ObjectPositionX (t.importerCollision[tCount].object2), ObjectPositionY (t.importerCollision[tCount].object2), ObjectPositionZ (t.importerCollision[tCount].object2) ) ;
						RotateObject (  t.selectedObjectMarkers[9], ObjectAngleX (t.importerCollision[tCount].object2),ObjectAngleY(t.importerCollision[tCount].object2),ObjectAngleZ(t.importerCollision[tCount].object2) ) ;
						GlueObjectToLimb (  t.selectedObjectMarkers[9], t.importerGridObject[9],0 );

						//  put pivot back into the scene
						PositionObject (   t.importerGridObject[9],0,0,IMPORTERZPOSITION );

					}
				}
			}
		}
	}

	for ( int tCount = 1 ; tCount<=  9; tCount++ )
	{
		t.snapPosX_f[t.importer.selectedCollisionObject][tCount] = ObjectPositionX(t.selectedObjectMarkers[tCount]);
		t.snapPosY_f[t.importer.selectedCollisionObject][tCount] = ObjectPositionY(t.selectedObjectMarkers[tCount]);
		t.snapPosZ_f[t.importer.selectedCollisionObject][tCount] = ObjectPositionZ(t.selectedObjectMarkers[tCount]);
	}
}

void importer_ShowCollisionOnly ( void )
{
	HideObject (  t.importer.objectnumber );
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  t.importerCollision[tCount].object > 0 ) 
		{
			if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
			{
				ShowObject (  t.importerCollision[tCount].object );
				SetObjectLight (  t.importerCollision[tCount].object, 1 );
				ColorObject (  t.importerCollision[tCount].object , Rgb(255,255,100) );
				GhostObjectOff (  t.importerCollision[tCount].object );
			}
		}
	}
	RotateObject (  t.importerGridObject[9], ObjectAngleX(t.importer.objectnumber), ObjectAngleY(t.importer.objectnumber), ObjectAngleZ(t.importer.objectnumber) );
}

void importer_ShowCollisionOnlyOff ( void )
{
	ShowObject (  t.importer.objectnumber );
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  t.importerCollision[tCount].object > 0 ) 
		{
			if (  ObjectExist(t.importerCollision[tCount].object)  ==  1 ) 
			{
				GhostObjectOn (  t.importerCollision[tCount].object );
				HideObject (  t.importerCollision[tCount].object );
			}
		}
	}
}

void importer_snapLeft ( void )
{
	t.tx_f = ObjectPositionX(t.tSnapObject);
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  tCount  !=  t.importer.selectedCollisionObject ) 
		{
			for ( t.tCount2 = 1 ; t.tCount2<=  8; t.tCount2++ )
			{
				t.tdiff_f = t.tx_f - t.snapPosX_f[tCount][t.tCount2];
				if (  abs(t.tdiff_f) < 5 ) 
				{
					MoveObject (  t.tSnapObject, t.tdiff_f );
					return;
				}
			}
		}
	}
}

void importer_snapUp ( void )
{
	t.ty_f = ObjectPositionY(t.tSnapObject);
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  tCount  !=  t.importer.selectedCollisionObject ) 
		{
			for ( t.tCount2 = 1 ; t.tCount2<=  8; t.tCount2++ )
			{
				t.tdiff_f = t.ty_f - t.snapPosY_f[tCount][t.tCount2];
				if (  abs(t.tdiff_f) < 5 ) 
				{
					MoveObjectUp (  t.tSnapObject, t.tdiff_f );
					return;
				}
			}
		}
	}
}

void importer_snapforward ( void )
{
	t.tz_f = ObjectPositionZ(t.tSnapObject);
	for ( int tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
		if (  tCount  !=  t.importer.selectedCollisionObject ) 
		{
			for ( t.tCount2 = 1 ; t.tCount2<=  8; t.tCount2++ )
			{
				t.tdiff_f = t.tz_f - t.snapPosZ_f[tCount][t.tCount2];
				if (  abs(t.tdiff_f) < 5 ) 
				{
					MoveObject (  t.tSnapObject, t.tdiff_f );
					return;
				}
			}
		}
	}
}

void importer_check_for_physics_changes ( void )
{
	if (  t.importer.collisionShapeCount  ==  0  )  return;
	int tCount = 0;
	for ( tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
	{
			if (  t.snapPosX_f[tCount][1]  ==  -99999 ) 
			{
				t.importer.selectedCollisionObject = tCount;
				t.slidersmenuvalue[t.importer.properties2Index][7].value = t.importerCollision[t.importer.selectedCollisionObject].rotx;
				t.slidersmenuvalue[t.importer.properties2Index][8].value = t.importerCollision[t.importer.selectedCollisionObject].roty;
				t.slidersmenuvalue[t.importer.properties2Index][9].value = t.importerCollision[t.importer.selectedCollisionObject].rotz;
				importer_update_selection_markers ( );
			}
	}

	// coordinate system for importer grabbing collision box corners
	//float fMX = (GetDisplayWidth()+0.0) / 800.0f; 230618 - not working for collision box control
	//float fMY = (GetDisplayHeight()+0.0) / 600.0f;
	//t.tadjustedtoareax_f = t.inputsys.xmouse*fMX;
	//t.tadjustedtoareay_f = t.inputsys.ymouse*fMY;
	t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
	t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
	t.tadjustedtoareax_f=((t.inputsys.xmouse+0.0)/800.0)/t.tadjustedtoareax_f;
	t.tadjustedtoareay_f=((t.inputsys.ymouse+0.0)/600.0)/t.tadjustedtoareay_f;
	t.tadjustedtoareax_f=t.tadjustedtoareax_f*(GetDisplayWidth()+0.0);
	t.tadjustedtoareay_f=t.tadjustedtoareay_f*(GetDisplayHeight()+0.0);

	//  No marker selected yet
	if (  t.importer.collisionObjectMode  ==  0 ) 
	{
		if (  t.inputsys.mclick  ==  1 ) 
		{
			//  Have we clicked on a marker?
			t.importer.selectedMarker = 0;
			for ( tCount = 1 ; tCount<=  9; tCount++ )
			{
				t.picked = PickScreenObjectEx(t.tadjustedtoareax_f, t.tadjustedtoareay_f, t.selectedObjectMarkers[tCount], t.selectedObjectMarkers[tCount],1);
				ScaleObject (  t.selectedObjectMarkers[tCount],100,100,100 );
				SetObjectAmbience (  t.selectedObjectMarkers[tCount],0 );
				if (  tCount  !=  9 ) 
				{
					SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(255,255,0) );
				}
				else
				{
					SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(0,255,255) );
				}
				if (  t.picked  !=  0 ) 
				{
					//  Enlarge and recolor the selected corner marker
					t.importer.selectedMarker = tCount;
					t.tScaleX_f = t.inputsys.xmousemove ; t.tScaleY_f = t.inputsys.ymousemove;
					ScaleObject (  t.selectedObjectMarkers[tCount],100,100,100 );
					SetObjectAmbience (  t.selectedObjectMarkers[tCount],0 );
					if (  tCount  !=  9 ) 
					{
						SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(255,100,0) );
					}
					else
					{
						SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(0,100,255) );
					}

					float xpick=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
					float ypick=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
					//  scale full mouse to fit in visible area
					xpick=((t.inputsys.xmouse+0.0)/800.0)/t.widgetinputsysxmouse_f;
					ypick=((t.inputsys.ymouse+0.0)/600.0)/t.widgetinputsysymouse_f;
					//  then provide in a format for the pick-from-screen command
					xpick=xpick*(GetChildWindowWidth()+0.0);
					ypick=ypick*(GetChildWindowHeight()+0.0);
					PickScreen2D23D (  xpick , ypick , IMPORTERZPOSITION );
					t.timporterpickdepth_f =  LimbPositionZ(t.selectedObjectMarkers[t.importer.selectedMarker],0);
					t.timporterpickdepth_f = t.timporterpickdepth_f * t.tadjustedtoimporterxbase_f;
					PickScreen2D23D (  xpick, ypick, t.timporterpickdepth_f );
					t.timporteroldmousex = GetPickVectorX();
					t.timporteroldmousey = GetPickVectorZ();
				}
			}

			//  If we have not clicked on a marker, then perhaps another collision box?
			if (  t.importer.selectedMarker  ==  0 ) 
			{
				for ( tCount = 0 ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
				{
					if (  PickScreenObjectEx(t.tadjustedtoareax_f, t.tadjustedtoareay_f, t.importerCollision[tCount].object2, t.importerCollision[tCount].object2,1) ) 
					{
						t.importer.selectedCollisionObject = tCount;
						t.slidersmenuvalue[t.importer.properties2Index][7].value = ObjectAngleX(t.importerCollision[tCount].object2);
						t.slidersmenuvalue[t.importer.properties2Index][8].value = ObjectAngleY(t.importerCollision[tCount].object2);
						t.slidersmenuvalue[t.importer.properties2Index][9].value = ObjectAngleZ(t.importerCollision[tCount].object2);
						break;
					}
				}
			}
			else
			{
				t.importer.collisionObjectMode = 1;
			}
		}
	}
	else
	{
		//  Marker is selected, lets deal with it
		if (  t.inputsys.mclick  ==  1 ) 
		{
			for ( tCount = 1 ; tCount<=  9; tCount++ )
			{
				if (  t.importer.selectedMarker  !=  tCount  )  
				{
					ScaleObject (  t.selectedObjectMarkers[tCount],50,50,50 );
				}
			}

			importer_hide_mouse ( );

			t.tXOnly = 1;
			t.tYOnly = 1;
			t.tZOnly = 1;
			t.importer.message = "Hold X,C or V to lock an axis";
			if (  t.inputsys.kscancode  ==  88 || t.inputsys.kscancode  ==  67 || t.inputsys.kscancode  ==  86 ) 
			{
				t.tXOnly = 0 ; t.tYOnly = 0 ; t.tZOnly = 0;
			}
			if (  t.inputsys.kscancode  ==  88  )  t.tXOnly  =  1;
			if (  t.inputsys.kscancode  ==  67  )  t.tYOnly  =  1;
			if (  t.inputsys.kscancode  ==  86  )  t.tZOnly  =  1;

			t.tScaleX_f = 0.0;
			t.tScaleY_f = 0.0;
			t.tScaleZ_f = 0.0;
			t.tMultiX_f = 1.0;
			t.tMultiY_f = 1.0;
			t.tMultiZ_f = 1.0;

			t.tr1_f = t.slidersmenuvalue[t.importer.properties2Index][7].value;
			t.tr2_f = t.slidersmenuvalue[t.importer.properties2Index][8].value;
			t.tr3_f = t.slidersmenuvalue[t.importer.properties2Index][9].value;

			RotateObject (  t.selectedObjectMarkers[1], t.tr1x_f+360,t.tr2_f+360,t.tr3_f+360 );

			if (  t.importer.selectedMarker  ==  1 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[1];

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[1]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[1]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[1]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[2]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[3]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[5]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[1],90 );
					MoveObject (  t.selectedObjectMarkers[1], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[1], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapLeft ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[1],90 );
					MoveObject (  t.selectedObjectMarkers[1], -t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[1],-90 );
					MoveObject (  t.selectedObjectMarkers[1], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[1],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[1],180 );
					MoveObject (  t.selectedObjectMarkers[1], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[1], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[1],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[1],180 );
					MoveObject (  t.selectedObjectMarkers[1], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[1], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[1],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[1]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[1]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[1]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.oldz2_f > t.oldz1_f ) 
				{
					t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
				}
				else
				{
					t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
				}
			}

			if (  t.importer.selectedMarker  ==  2 ) 
			{
				t.tSnapObject = t.selectedObjectMarkers[2];
				t.tMultiX_f = -1;
				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[2]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[2]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[2]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[1]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[4]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[6]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[2],90 );
					MoveObject (  t.selectedObjectMarkers[2], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[2], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[2],180 );
						importer_snapLeft ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[2],90 );
					MoveObject (  t.selectedObjectMarkers[2], t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[2],-90 );
					MoveObject (  t.selectedObjectMarkers[2], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[2],180 );
						TurnObjectLeft (  t.selectedObjectMarkers[2],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[2],180 );
					MoveObject (  t.selectedObjectMarkers[2], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[2], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[2],180 );
					MoveObject (  t.selectedObjectMarkers[2], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[2], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[2]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[2]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[2]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.importer.viewMessage  !=  "Top" ) 
				{
					if (  t.oldz2_f > t.oldz1_f ) 
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
				}
				else
				{
					if (  t.oldz2_f > t.oldz1_f ) 
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
				}
			}

			if (  t.importer.selectedMarker  ==  3 ) 
			{
				t.tSnapObject = t.selectedObjectMarkers[3];

				t.tMultiY_f = -1.0;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[3]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[3]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[3]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[4]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[1]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[8]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[3],90 );
					MoveObject (  t.selectedObjectMarkers[3], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[3], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapLeft ( );
						RollObjectRight (  t.selectedObjectMarkers[3],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[3],90 );
					MoveObject (  t.selectedObjectMarkers[3], -t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[3],-90 );
					MoveObject (  t.selectedObjectMarkers[3], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[3],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[3],180 );
					MoveObject (  t.selectedObjectMarkers[3], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[3], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[3],180 );
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[3],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[3],180 );
					MoveObject (  t.selectedObjectMarkers[3], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[3], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectRight (  t.selectedObjectMarkers[3],180 );
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[3],180 );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[3]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[3]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[3]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.oldz2_f > t.oldz1_f ) 
				{
					t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
				}
				else
				{
					t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
				}

			}

			if (  t.importer.selectedMarker  ==  4 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[4];

				t.tMultiX_f = -1;
				t.tMultiY_f = -1;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[4]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[4]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[4]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[3]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[2]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[5]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[4],90 );
					MoveObject (  t.selectedObjectMarkers[4], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[4], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[4],180 );
						importer_snapLeft ( );
						RollObjectRight (  t.selectedObjectMarkers[4],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[4],90 );
					MoveObject (  t.selectedObjectMarkers[4], t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[4],-90 );
					MoveObject (  t.selectedObjectMarkers[4], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[4],-90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[4],180 );
					MoveObject (  t.selectedObjectMarkers[4], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[4], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[4],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[4],180 );
					MoveObject (  t.selectedObjectMarkers[4], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[4], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[4],180 );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[4]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[4]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[4]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.importer.viewMessage  !=  "Top" ) 
				{
					if (  t.oldz2_f > t.oldz1_f ) 
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
				}
				else
				{
					if (  t.oldz2_f > t.oldz1_f ) 
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
				}
			}

			if (  t.importer.selectedMarker  ==  5 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[5];

				t.tMultiZ_f = -1;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[5]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[5]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[5]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[6]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[7]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[1]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[5],90 );
					MoveObject (  t.selectedObjectMarkers[5], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[5], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapLeft ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[5],90 );
					MoveObject (  t.selectedObjectMarkers[5], -t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[5],-90 );
					MoveObject (  t.selectedObjectMarkers[5], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[5],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[5],180 );
					MoveObject (  t.selectedObjectMarkers[5], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[5], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[5],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[5],180 );
					MoveObject (  t.selectedObjectMarkers[5], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[5], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[5],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[5]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[5]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[5]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.oldz2_f < t.oldz1_f ) 
				{
					t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
				}
				else
				{
					t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
				}

			}


			if (  t.importer.selectedMarker  ==  6 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[6];

				t.tMultiX_f = -1;
				t.tMultiZ_f = -1;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[6]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[6]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[6]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[5]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[8]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[2]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[6],90 );
					MoveObject (  t.selectedObjectMarkers[6], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[6], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[6],180 );
						importer_snapLeft ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[6],90 );
					MoveObject (  t.selectedObjectMarkers[6], t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[6],-90 );
					MoveObject (  t.selectedObjectMarkers[6], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[6],180 );
						TurnObjectLeft (  t.selectedObjectMarkers[6],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[6],180 );
					MoveObject (  t.selectedObjectMarkers[6], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[6], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[6],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[6],180 );
					MoveObject (  t.selectedObjectMarkers[6], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[6], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[6],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[6]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[6]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[6]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.importer.viewMessage  !=  "Top" ) 
				{
					if (  t.oldz2_f > t.oldz1_f ) 
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
				}
				else
				{
					if (  t.oldz2_f < t.oldz1_f ) 
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
				}

			}

			if (  t.importer.selectedMarker  ==  7 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[7];

				t.tMultiZ_f = -1;
				t.tMultiY_f = -1;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[7]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[7]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[7]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[4]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[2]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[1]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[7],90 );
					MoveObject (  t.selectedObjectMarkers[7], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[7], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[7],180 );
						importer_snapLeft ( );
						RollObjectRight (  t.selectedObjectMarkers[7],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[7],90 );
					MoveObject (  t.selectedObjectMarkers[7], t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[7],-90 );
					MoveObject (  t.selectedObjectMarkers[7], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[7],180+90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[7],180 );
					MoveObject (  t.selectedObjectMarkers[7], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[7], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[7],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[7],180 );
					MoveObject (  t.selectedObjectMarkers[7], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[7], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[7],180 );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[7]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[7]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[7]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f < t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.importer.viewMessage  !=  "Top" ) 
				{
					if (  t.oldz2_f < t.oldz1_f ) 
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
				}
				else
				{
					if (  t.oldz2_f < t.oldz1_f ) 
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
				}
			}

			if (  t.importer.selectedMarker  ==  8 ) 
			{

				t.tSnapObject = t.selectedObjectMarkers[8];

				t.tMultiX_f = -1;
				t.tMultiZ_f = -1;
				t.tMultiY_f = -1;

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[8]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[8]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[8]);
				t.oldx2_f = ObjectPositionX(t.selectedObjectMarkers[7]);
				t.oldy2_f = ObjectPositionY(t.selectedObjectMarkers[6]);
				t.oldz2_f = ObjectPositionZ(t.selectedObjectMarkers[1]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[8],90 );
					MoveObject (  t.selectedObjectMarkers[8], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[8], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[8],180 );
						importer_snapLeft ( );
						RollObjectRight (  t.selectedObjectMarkers[8],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[8],90 );
					MoveObject (  t.selectedObjectMarkers[8], t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[8],-90 );
					MoveObject (  t.selectedObjectMarkers[8], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[8],-90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					TurnObjectLeft (  t.selectedObjectMarkers[8],180 );
					MoveObject (  t.selectedObjectMarkers[8], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[8], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[8],180 );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					TurnObjectRight (  t.selectedObjectMarkers[8],180 );
					MoveObject (  t.selectedObjectMarkers[8], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[8], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						RollObjectRight (  t.selectedObjectMarkers[8],180 );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[8]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[8]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[8]);

				t.dist1x_f = GetDistance( t.oldx1_f,0,0,t.oldx2_f,0,0);
				t.dist2x_f = GetDistance( t.newx1_f,0,0,t.oldx2_f,0,0);

				//  X plane
				if (  t.oldx2_f > t.oldx1_f ) 
				{
					t.tScaleX_f = -(t.dist2x_f - t.dist1x_f);
				}
				else
				{
					t.tScaleX_f = (t.dist2x_f - t.dist1x_f);
				}

				//  Y plane
				t.dist1y_f = GetDistance( t.oldy1_f,0,0,t.oldy2_f,0,0);
				t.dist2y_f = GetDistance( t.newy1_f,0,0,t.oldy2_f,0,0);

				if (  t.oldy2_f > t.oldy1_f ) 
				{
					t.tScaleY_f = -(t.dist2y_f - t.dist1y_f);
				}
				else
				{
					t.tScaleY_f = (t.dist2y_f - t.dist1y_f);
				}

				//  Z plane
				t.dist1z_f = GetDistance( t.oldz1_f,0,0,t.oldz2_f,0,0);
				t.dist2z_f = GetDistance( t.newz1_f,0,0,t.oldz2_f,0,0);

				if (  t.importer.viewMessage  !=  "Top" ) 
				{
					if (  t.oldz2_f < t.oldz1_f ) 
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
				}
				else
				{
					if (  t.oldz2_f < t.oldz1_f ) 
					{
						t.tScaleZ_f = -(t.dist2z_f - t.dist1z_f);
					}
					else
					{
						t.tScaleZ_f = (t.dist2z_f - t.dist1z_f);
					}
				}
			}

			if (  t.importer.selectedMarker  ==  9 ) 
			{
				t.tSnapObject = t.selectedObjectMarkers[1];

				t.oldx1_f = ObjectPositionX(t.selectedObjectMarkers[9]);
				t.oldy1_f = ObjectPositionY(t.selectedObjectMarkers[9]);
				t.oldz1_f = ObjectPositionZ(t.selectedObjectMarkers[9]);

				if (  t.importer.viewMessage  ==  "Front" ) 
				{
					RotateObject (  t.selectedObjectMarkers[9],0,0,0 );
					TurnObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleY ( t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					PitchObjectUp (   t.selectedObjectMarkers[9],-ObjectAngleX (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					RollObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleZ (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					TurnObjectLeft (  t.selectedObjectMarkers[9],90 );
					MoveObject (  t.selectedObjectMarkers[9], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[9], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapLeft ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Top" ) 
				{
					RotateObject (  t.selectedObjectMarkers[9],-ObjectAngleX (t.importerCollision[t.importer.selectedCollisionObject].object), ObjectAngleY ( t.importerCollision[t.importer.selectedCollisionObject].object),-ObjectAngleZ (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					RotateObject (  t.selectedObjectMarkers[9],0,0,0 );
					TurnObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleY ( t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					PitchObjectUp (   t.selectedObjectMarkers[9],-ObjectAngleX (t.importerCollision[t.importer.selectedCollisionObject].object) );
					RollObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleZ (t.importerCollision[t.importer.selectedCollisionObject].object) );
					TurnObjectLeft (  t.selectedObjectMarkers[9],90 );
					MoveObject (  t.selectedObjectMarkers[9], -t.inputsys.xmousemove );
					TurnObjectLeft (  t.selectedObjectMarkers[9],-90 );
					MoveObject (  t.selectedObjectMarkers[9], -t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						importer_snapforward ( );
						TurnObjectLeft (  t.selectedObjectMarkers[9],90 );
						importer_snapLeft ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Left" ) 
				{
					RotateObject (  t.selectedObjectMarkers[9],0,0,0 );
					TurnObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleY ( t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					PitchObjectUp (   t.selectedObjectMarkers[9],-ObjectAngleX (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					RollObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleZ (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					TurnObjectLeft (  t.selectedObjectMarkers[9],180 );
					MoveObject (  t.selectedObjectMarkers[9], t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[9], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[9],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}
				if (  t.importer.viewMessage  ==  "Right" ) 
				{
					RotateObject (  t.selectedObjectMarkers[9],0,0,0 );
					TurnObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleY ( t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					PitchObjectUp (   t.selectedObjectMarkers[9],-ObjectAngleX (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					RollObjectLeft (  t.selectedObjectMarkers[9],ObjectAngleZ (t.importerCollision[t.importer.selectedCollisionObject].object) ) ;
					TurnObjectRight (  t.selectedObjectMarkers[9],180 );
					MoveObject (  t.selectedObjectMarkers[9], -t.inputsys.xmousemove );
					MoveObjectUp (  t.selectedObjectMarkers[9], t.inputsys.ymousemove );
					if (  t.inputsys.keyshift ) 
					{
						TurnObjectLeft (  t.selectedObjectMarkers[9],180 );
						importer_snapforward ( );
						importer_snapUp ( );
					}
				}

				t.newx1_f = ObjectPositionX(t.selectedObjectMarkers[9]);
				t.newy1_f = ObjectPositionY(t.selectedObjectMarkers[9]);
				t.newz1_f = ObjectPositionZ(t.selectedObjectMarkers[9]);

				t.tmoveX_f = t.newx1_f - t.oldx1_f;
				t.tmoveY_f = t.newy1_f - t.oldy1_f;
				t.tmoveZ_f = t.newz1_f - t.oldz1_f;
	
				if (  t.tXOnly  ==  0  )  t.tmoveX_f  =  0;
				if (  t.tYOnly  ==  0  )  t.tmoveY_f  =  0;
				if (  t.tZOnly  ==  0  )  t.tmoveZ_f  =  0;
	
				MoveObjectRight (  t.importerCollision[t.importer.selectedCollisionObject].object, t.tmoveX_f );
				MoveObjectDown (  t.importerCollision[t.importer.selectedCollisionObject].object, t.tmoveY_f );
				MoveObject (  t.importerCollision[t.importer.selectedCollisionObject].object, t.tmoveZ_f );
				ShowObject (  t.importerCollision[t.importer.selectedCollisionObject].object );
				GhostObjectOn (   t.importerCollision[t.importer.selectedCollisionObject].object,2 );
				SetObjectLight (  t.importerCollision[t.importer.selectedCollisionObject].object,1 );
				ColorObject (  t.importerCollision[t.importer.selectedCollisionObject].object,Rgb(0,0,0) );
				SetObjectAmbience (  t.importerCollision[t.importer.selectedCollisionObject].object,0 );
				SetObjectEmissive (  t.importerCollision[t.importer.selectedCollisionObject].object, Rgb(100,100,0) );

				MoveObjectRight (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.tmoveX_f );
				MoveObjectDown (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.tmoveY_f );
				MoveObject (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.tmoveZ_f );

				return;
			}

			//  If any axis are not locked allow movement
			if (  t.tXOnly  ==  0  )  t.tScaleX_f  =  0;
			if (  t.tYOnly  ==  0  )  t.tScaleY_f  =  0;
			if (  t.tZOnly  ==  0  )  t.tScaleZ_f  =  0;

			//  stop the boxes becomes too small or inverted
			if (  LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleX_f < 5 ) 
			{
				t.tScaleX_f = -(5.0-LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object2,0));
			}
			if (  LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleY_f < 5 ) 
			{
				t.tScaleY_f = -(5.0-LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object2,0));
			}
			if (  LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object2,0) + t.tScaleZ_f < 5 ) 
			{
				t.tScaleZ_f = (5.0-LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object2,0));
			}

			ScaleLimb (  t.importerCollision[t.importer.selectedCollisionObject].object , 0, LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleX_f , LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleY_f, LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object2,0) + t.tScaleZ_f );
			t.importerCollision[t.importer.selectedCollisionObject].sizex = LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object,0) - t.tScaleX_f;
			t.importerCollision[t.importer.selectedCollisionObject].sizey = LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object,0) - t.tScaleY_f;
			t.importerCollision[t.importer.selectedCollisionObject].sizez = LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object,0) + t.tScaleZ_f;
			MoveObjectRight (  t.importerCollision[t.importer.selectedCollisionObject].object, t.tScaleX_f * t.tMultiX_f / 2.0 );
			MoveObjectDown (  t.importerCollision[t.importer.selectedCollisionObject].object, t.tScaleY_f * t.tMultiY_f / 2.0 );
			MoveObject (  t.importerCollision[t.importer.selectedCollisionObject].object, -t.tScaleZ_f * t.tMultiZ_f / 2.0 );
			ShowObject (  t.importerCollision[t.importer.selectedCollisionObject].object );

			GhostObjectOn (   t.importerCollision[t.importer.selectedCollisionObject].object,2 );

			SetObjectLight (  t.importerCollision[t.importer.selectedCollisionObject].object,1 );
			ColorObject (  t.importerCollision[t.importer.selectedCollisionObject].object,Rgb(0,0,0) );
			SetObjectAmbience (  t.importerCollision[t.importer.selectedCollisionObject].object,0 );
			SetObjectEmissive (  t.importerCollision[t.importer.selectedCollisionObject].object, Rgb(100,100,0) );

			ScaleLimb (  t.importerCollision[t.importer.selectedCollisionObject].object2 , 0, LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleX_f , LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object2,0) - t.tScaleY_f, LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object2,0) + t.tScaleZ_f );
			MoveObjectRight (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.tScaleX_f * t.tMultiX_f / 2.0 );
			MoveObjectDown (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.tScaleY_f * t.tMultiY_f / 2.0 );
			MoveObject (  t.importerCollision[t.importer.selectedCollisionObject].object2, -t.tScaleZ_f * t.tMultiZ_f / 2.0 );

		}
		else
		{

			//  Mouse released, come out of editing
			importer_show_mouse ( );
			t.timporterpickdepth_f = 1250;
			HideObject (  t.importerCollision[t.importer.selectedCollisionObject].object );
			for ( tCount = 1 ; tCount<=  9; tCount++ )
			{
				t.picked = PickScreenObjectEx(t.importer.MouseX, t.importer.MouseY, t.selectedObjectMarkers[tCount], t.selectedObjectMarkers[tCount],1);
				ScaleObject (  t.selectedObjectMarkers[tCount],100,100,100 );
				SetObjectAmbience (  t.selectedObjectMarkers[tCount],0 );
				if (  tCount  !=  9 ) 
				{
					SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(255,255,0) );
				}
				else
				{
					SetObjectEmissive (  t.selectedObjectMarkers[tCount], Rgb(0,255,255) );
				}

			}
			t.importer.selectedMarker = 0;
			t.importer.collisionObjectMode = 0;
		}
	}
}

void importer_update_textures ( void )
{
	// count how many images are specified in image list
	int iImageCount = 0;
	for ( int tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
		if ( strlen ( t.importerTextures[tCount].fileName.Get() ) > 0 )
			iImageCount++;

	// show all textures associated with this model import (so user can change textures used)
	strcpy ( g_pShowFilenameHoveringOver, "" );
	for ( int tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		// skip image slots that have no filename
		if ( strlen ( t.importerTextures[tCount].fileName.Get() ) == 0 )
			continue;

		// determine what texture is shown in UI (if any)
		int iTexSlotImage = t.importerTextures[tCount].imageID;

		// work out texture panel position
		t.tOffsetX = 10;
		if (  tCount > 5  )  t.tOffsetX  =  10+128;
		if (  tCount > 10  )  t.tOffsetX  =  10+(128*2);
		if (  tCount > 15  )  t.tOffsetX  =  10+(128*3);
		if (  tCount > 20  )  t.tOffsetX  =  10+(128*4);
		//if (  t.importer.scaleMulti  !=  1.0  )  t.tOffsetX  =  0;
		t.tOffsetY = tCount;
		if (  tCount > 5  )  t.tOffsetY  =  tCount-5;
		if (  tCount > 10  )  t.tOffsetY  =  tCount-10;
		if (  tCount > 15  )  t.tOffsetY  =  tCount-15;
		if (  tCount > 20  )  t.tOffsetY  =  tCount-20;
		int iVertical = (t.tOffsetY-1);
		t.tOffsetY = 10 + (iVertical * 128);

		if (  t.importer.scaleMulti  !=  1.0 ) 
			Sprite (  t.importerTextures[tCount].spriteID2 , t.tOffsetX, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19+20 , g.importermenuimageoffset+7 );
		else
			Sprite (  t.importerTextures[tCount].spriteID2 , (GetChildWindowWidth()/2) - 430 - t.tOffsetX -19 -20, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19+20 , g.importermenuimageoffset+7 );
		SizeSprite (  t.importerTextures[tCount].spriteID2 , 128 , 128 );

		if ( iTexSlotImage > 0 )
		{
			if (  t.importer.scaleMulti  !=  1.0 ) 
				Sprite (  t.importerTextures[tCount].spriteID , t.tOffsetX + 20 , (GetChildWindowHeight()/2) - 400 + t.tOffsetY+20 , iTexSlotImage );
			else
				Sprite (  t.importerTextures[tCount].spriteID , (GetChildWindowWidth()/2) - 430 - t.tOffsetX-20 , (GetChildWindowHeight()/2) - 400 + t.tOffsetY+20 , iTexSlotImage );
			SizeSprite (  t.importerTextures[tCount].spriteID , 90 , 90 );
			SetSpritePriority (  t.importerTextures[tCount].spriteID, 1 );
		}
		else
		{
			Sprite (  t.importerTextures[tCount].spriteID , -10000, -10000, g.importermenuimageoffset+7 );
		}

		if (  t.importer.MouseX >= SpriteX (t.importerTextures[tCount].spriteID2)-5 && t.importer.MouseY >= (GetChildWindowHeight()/2) - 400 + (iVertical * 128) + 5 )
		{
			if (  t.importer.MouseX <= SpriteX (t.importerTextures[tCount].spriteID2)+128-5 && t.importer.MouseY <= 90+(GetChildWindowHeight()/2) - 400 + (iVertical * 128) + 30 ) 
			{
				strcpy ( g_pShowFilenameHoveringOver, t.importerTextures[tCount].fileName.Get() );
				if (  t.inputsys.mclick  ==  0 ) 
				{
					if (  t.importer.scaleMulti  !=  1.0 ) 
						Sprite ( t.importerTextures[tCount].spriteID2, t.tOffsetX, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19 + 20 , g.importermenuimageoffset+7 );
					else
						Sprite ( t.importerTextures[tCount].spriteID2, (GetChildWindowWidth()/2) - 430 - t.tOffsetX -19 - 20, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19 + 20 , g.importermenuimageoffset+7 );
					SizeSprite ( t.importerTextures[tCount].spriteID2 , 128 , 128 );
					SizeSprite ( t.importerTextures[tCount].spriteID , 106 , 106 );
					if (  t.importer.scaleMulti  ==  1.0 ) 
					{
						Sprite (  t.importerTextures[tCount].spriteID , (GetChildWindowWidth()/2)-430-8-20 , (GetChildWindowHeight()/2) - 400 + tCount * 128 - 8+20 , iTexSlotImage );
					}
					else
					{
						if ( iTexSlotImage > 0 )
						{
							Sprite (  t.importerTextures[tCount].spriteID , t.tOffsetX + 10 , (GetChildWindowHeight()/2) - 400 + (iVertical * 128) - 8+20 , iTexSlotImage );
						}
					}
					SetSpritePriority (  t.importerTextures[tCount].spriteID, 1 );
				}
				else
				{
					if (  t.importer.oldMouseClick  ==  0 ) 
					{
						t.tFileName_s = openFileBox("PNG|*.png|DDS|*.dds|JPEG|*.jpg|BMP|*.bmp|All Files|*.*|", "", "Open Texture", ".dds", IMPORTEROPENFILE);
						if (  t.tFileName_s  ==  "Error"  )  return;
						if (  FileExist ( t.tFileName_s.Get() )  ==  1 )  
						{
							// prompt as this may take some seconds
							LPSTR pDelayPrompt = "Loading chosen texture and associated files";
							for ( int iSyncPass=0; iSyncPass<2; iSyncPass++ )
							{
								pastebitmapfont(pDelayPrompt,(GetChildWindowWidth()/2) - (getbitmapfontwidth (pDelayPrompt,1)/2),860,1,255);
								Sync();
							}

							// find free image
							t.tImageID = t.importerTextures[tCount].imageID;
							if ( t.tImageID == 0 )
							{
								t.tImageID = g.importermenuimageoffset+15;
								while ( ImageExist(t.tImageID) == 1 ) ++t.tImageID;
							}

							// can expand out a color texture once (to add normal/gloss/etc)
							bool bExpandOutPBRTextureSet = false;

							// replace image details
							if ( ImageExist ( t.tImageID ) == 1 ) DeleteImage ( t.tImageID );
							LoadImage ( t.tFileName_s.Get(), t.tImageID );
							if ( ImageExist ( t.tImageID ) == 1 ) 
							{
								// remove any previous references to associated files for the old filename
								if ( t.importerTextures[tCount].iExpandedThisSlot == 0 )
								{
									// but only if its a base color texutre
									char pIsItTexColor[2048];
									strcpy ( pIsItTexColor, t.importerTextures[tCount].fileName.Get() );
									if ( strlen ( pIsItTexColor ) > 1+8+4 )
									{
										pIsItTexColor[strlen(pIsItTexColor)-4]=0;
										if ( strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 2, "_d", 2 ) == NULL
										|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 6, "_color", 6 ) == NULL
										|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 8, "_diffuse", 8 ) == NULL
										|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 7, "_albedo", 7 ) == NULL
										|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 8, "blankTex", 8 ) == NULL )
										{
											// for both!
											strcpy ( pIsItTexColor, t.tFileName_s.Get() );
											if ( strlen ( pIsItTexColor ) > 1+8+4 )
											{
												pIsItTexColor[strlen(pIsItTexColor)-4]=0;
												if ( strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 2, "_d", 2 ) == NULL
												|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 6, "_color", 6 ) == NULL
												|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 8, "_diffuse", 8 ) == NULL
												|| strnicmp ( pIsItTexColor + strlen(pIsItTexColor) - 7, "_albedo", 7 ) == NULL )
												{
													importer_removeentryandassociatesof ( tCount );
													t.importerTextures[tCount].iExpandedThisSlot = 1;
													bExpandOutPBRTextureSet = true;
												}
											}
										}
									}
								}

								// update image list data
								t.importerTextures[tCount].fileName = t.tFileName_s;
								t.importerTextures[tCount].imageID = t.tImageID;
							}

							// ensure single texture is specified in FPE
							if ( iImageCount == 1 ) 
							{
								t.importer.objectFPE.textured = t.tFileName_s;
							}

							// reapply texture to model
							importer_applyimagelisttextures(false, tCount, bExpandOutPBRTextureSet);
							importer_recreate_texturesprites();
						}
					}
				}
			}
		}
	}
	SetDir (  t.importer.startDir.Get() );

	/* old pre-180617
	// we only use one texture slot (for single texture or shows multi-tex image FOR NOW)
	// for ( int tCount = 1 ; tCount<=  10; tCount++ )
	int tCount = 1;

	// determine what texture is shown in UI
	int iTexSlotImage = t.importerTextures[tCount].imageID;
	if ( iTexSlotImage == 0  ) return;

	// is this a multi textures
	if ( t.importerTextures[2].imageID != 0 )
		iTexSlotImage = g.importermenuimageoffset+10;

	// is image valid
	if ( ImageExist (iTexSlotImage) == 0 )  
		return;

	t.tOffsetX = 0;
	if (  tCount > 5  )  t.tOffsetX  =  128;
	if (  t.importer.scaleMulti  !=  1.0  )  t.tOffsetX  =  0;
	t.tOffsetY = tCount;
	if (  tCount > 5  )  t.tOffsetY  =  tCount-5;
	t.tOffsetY = t.tOffsetY * 128;

	if (  t.importer.scaleMulti  !=  1.0 ) 
	{
		Sprite (  t.importerTextures[tCount].spriteID2 , 0, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19+20 , g.importermenuimageoffset+7 );
	}
	else
	{
		Sprite (  t.importerTextures[tCount].spriteID2 , (GetChildWindowWidth()/2) - 430 - t.tOffsetX -19 -20, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19+20 , g.importermenuimageoffset+7 );
	}
	SizeSprite (  t.importerTextures[tCount].spriteID2 , 128 , 128 );

	if (  t.importer.scaleMulti  !=  1.0 ) 
	{
		Sprite (  t.importerTextures[tCount].spriteID , 20 , (GetChildWindowHeight()/2) - 400 + t.tOffsetY+20 , iTexSlotImage );
	}
	else
	{
		Sprite (  t.importerTextures[tCount].spriteID , (GetChildWindowWidth()/2) - 430 - t.tOffsetX-20 , (GetChildWindowHeight()/2) - 400 + t.tOffsetY+20 , iTexSlotImage );
	}
	SizeSprite (  t.importerTextures[tCount].spriteID , 90 , 90 );
	SetSpritePriority (  t.importerTextures[tCount].spriteID, 1 );

	if (  t.importer.MouseX  >=  SpriteX (t.importerTextures[tCount].spriteID2) -20 && t.importer.MouseY >=  (GetChildWindowHeight()/2) - 400 + tCount * 128+20 )
	{
		if (  t.importer.MouseX  <=  SpriteX (t.importerTextures[tCount].spriteID2) +128+20 && t.importer.MouseY <=  90+(GetChildWindowHeight()/2) - 400 + tCount * 128+20 ) 
		{
			if (  t.inputsys.mclick  ==  0 ) 
			{
				if (  t.importer.scaleMulti  !=  1.0 ) 
				{
					Sprite (  t.importerTextures[tCount].spriteID2 , 0 , (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19 + 20 , g.importermenuimageoffset+7 );
				}
				else
				{
					Sprite (  t.importerTextures[tCount].spriteID2 , (GetChildWindowWidth()/2) - 430 - t.tOffsetX -19 - 20, (GetChildWindowHeight()/2) - 400 + t.tOffsetY-19 + 20 , g.importermenuimageoffset+7 );
				}
				SizeSprite (  t.importerTextures[tCount].spriteID2 , 128 , 128 );
				SizeSprite (  t.importerTextures[tCount].spriteID , 106 , 106 );
				if (  t.importer.scaleMulti  ==  1.0 ) 
				{
					Sprite (  t.importerTextures[tCount].spriteID , (GetChildWindowWidth()/2) - 430 -8-20 , (GetChildWindowHeight()/2) - 400 + tCount * 128 - 8+20 , iTexSlotImage );
				}
				else
				{
					Sprite (  t.importerTextures[tCount].spriteID , 10 , (GetChildWindowHeight()/2) - 400 + tCount * 128 - 8+20 , iTexSlotImage );
				}
				SetSpritePriority (  t.importerTextures[tCount].spriteID, 1 );
			}
			else
			{
				if (  t.importer.oldMouseClick  ==  0 ) 
				{
					t.tFileName_s = openFileBox("PNG|*.png|DDS|*.dds|JPEG|*.jpg|BMP|*.bmp|All Files|*.*|", "", "Open Texture", ".dds", IMPORTEROPENFILE);
					if (  t.tFileName_s  ==  "Error"  )  return;
					if (  FileExist ( t.tFileName_s.Get() )  ==  1 )  
					{
						// prompt as this may take some seconds
						LPSTR pDelayPrompt = "Loading chosen texture and associated files";
						for ( int iSyncPass=0; iSyncPass<2; iSyncPass++ )
						{
							pastebitmapfont(pDelayPrompt,(GetChildWindowWidth()/2) - (getbitmapfontwidth (pDelayPrompt,1)/2),860,1,255);
							Sync();
						}

						// clear image list of old images
						for ( tCount = 2 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
						{
							t.importerTextures[tCount].fileName = "";
							t.importerTextures[tCount].imageID = 0;
						}

						// start again at image slot one
						tCount = 1;

						// first reset sprite so new image (same ID) can take hold
						Sprite ( t.importerTextures[tCount].spriteID, -99999, -99999, g.importermenuimageoffset+10 );

						// replace image details
						if ( ImageExist ( t.importerTextures[tCount].imageID ) ==1 ) DeleteImage ( t.importerTextures[tCount].imageID );
						LoadImage ( t.tFileName_s.Get(), t.importerTextures[tCount].imageID );
						t.importerTextures[tCount].fileName = t.tFileName_s;

						// ensure single texture is specified in FPE
						t.importer.objectFPE.textured = t.tFileName_s;

						// reapply texture to model
						importer_applyimagelisttextures(true);
					}
				}
			}
		}
	}
	SetDir (  t.importer.startDir.Get() );
	*/
}

void importer_load_textures_finish ( int tCount, bool bCubeMapOnly )
{
	// final image load count, load textures and sort txture button sprite
	t.tcounttextures = tCount;

	// Load textures
	for ( tCount = 1; tCount <= t.tcounttextures; tCount++ )
	{
		t.tImageID = g.importermenuimageoffset+15;
		while ( ImageExist(t.tImageID) == 1 ) ++t.tImageID;
		LoadImage ( t.importerTextures[tCount].fileName.Get(), t.tImageID );
		if ( ImageExist ( t.tImageID ) == 1 )
			t.importerTextures[tCount].imageID = t.tImageID;
		else
			t.importerTextures[tCount].imageID = 0;
	}
	
	// Apply textures to model
	importer_applyimagelisttextures ( bCubeMapOnly, -1, true );
	importer_recreate_texturesprites();
}

void importer_load_textures ( void )
{
	// Clean importer Textures List
	int tCount = 0;
	for ( tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		t.importerTextures[tCount].imageID = 0;
		t.importerTextures[tCount].fileName = "";
		t.importerTextures[tCount].iExpandedThisSlot = 0;		
		t.importerTextures[tCount].iOptionalStage = 0;
		t.importerTextures[tCount].iAssociatedBaseImage = 0;
	}

	// Byte scan of file - good for TXT or FBX based model files
	t.filesize = FileSize(t.timporterfile_s.Get());
	t.mbi=255;
	OpenToRead (  11,t.timporterfile_s.Get() );
	MakeMemblockFromFile (  t.mbi,11 );
	CloseFile (  11 );
	tCount=0;
	t.leavetime=Timer();
	for ( t.b = 0 ; t.b<=  t.filesize-5; t.b++ )
	{
		//  JPG, PNG, DDS, BMP, TGA
		t.tokay=0;
		if (  ReadMemblockByte(t.mbi,t.b+0) == Asc(".") ) 
		{
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("j") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("p") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("g") ) 
					{
						t.tokay=1;
					}
				}
			}
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("p") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("n") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("g") ) 
					{
						t.tokay=1;
					}
				}
			}
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("d") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("d") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("s") ) 
					{
						t.tokay=1;
					}
				}
			}
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("t") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("g") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("a") ) 
					{
						t.tokay=1;
					}
				}
			}
			if (  ReadMemblockByte(t.mbi,t.b+1) == Asc("b") ) 
			{
				if (  ReadMemblockByte(t.mbi,t.b+2) == Asc("m") ) 
				{
					if (  ReadMemblockByte(t.mbi,t.b+3) == Asc("p") ) 
					{
						t.tokay=1;
					}
				}
			}
		}
		if ( t.tokay == 1 ) 
		{
			// determine name of external texture file
			t.c=t.b;
			while (  t.c>0 ) 
			{
				--t.c ; if (  t.c <= 0 || ReadMemblockByte(t.mbi,t.c)<32 || ReadMemblockByte(t.mbi,t.c)>Asc("z")  )  break;
			}
			++t.c;
			t.tthisfile_s="";
			while (  t.c <= t.b+3 ) 
			{
				t.tthisfile_s=t.tthisfile_s+Chr(ReadMemblockByte(t.mbi,t.c)) ; ++t.c;
			}
			t.b=t.b+3;

			//  found texture specified in imported model
			t.tSourceName_s = t.tthisfile_s;

			//  check if texture file exists alonside model file
			char pFindFile[512];
			t.tFileName_s = t.importer.objectFileOriginalPath + t.tSourceName_s;
			strcpy ( pFindFile, t.tFileName_s.Get() );
			if ( FileExist(pFindFile)  ==  0 ) 
			{
				// if not, try just the filename itself (no path)
				int iFoundFilename = -1;
				for ( int n = strlen(pFindFile)-1; n > 0; n-- )
				{
					if ( pFindFile[n] == '\\' || pFindFile[n] == '/' )
					{
						iFoundFilename = n;
						break;
					}
				}
				if ( iFoundFilename != -1 )
				{
					char pFilenameOnly[512];
					strcpy ( pFilenameOnly, pFindFile+iFoundFilename+1 );
					strcpy ( pFindFile, t.importer.objectFileOriginalPath.Get() );
					strcat ( pFindFile, pFilenameOnly );
				}
			}
			t.tFileName_s = pFindFile;

			//  ensure file is not a path or contains invalid characters
			if ( strcmp ( Right(t.tFileName_s.Get(),1) , "/" ) == 0 || strcmp ( Right(t.tFileName_s.Get(),1) , "\\" ) == 0  )  t.tFileName_s = "";
			for ( t.fc = 1 ; t.fc<=  Len(t.tFileName_s.Get()); t.fc++ )
			{
				if (  Asc(Mid(t.tFileName_s.Get(),t.fc)) < 32 || Asc(Mid(t.tFileName_s.Get(),t.fc)) > 126 ) 
				{
					t.tFileName_s = "";
					break;
				}
			}

			// Add found texture to the importer texture list (if exists)
			if ( FileExist(t.tFileName_s.Get()) == 1 ) 
			{
				importer_addtexturefiletolist ( t.tFileName_s, t.tSourceName_s, &tCount );
			}
		}
	}
	DeleteMemblock ( t.mbi );

	// If nothing in file byte scan, lets see if the FPE had a texture
	t.tFoundInFPE = 0;
	if ( tCount == 0 ) 
	{
		// get texture from FPE file
		if (  t.importer.objectFPE.textured  !=  "" ) 
		{
			t.tSourceName_s = t.importer.objectFPE.textured;
			if (  FileExist ( t.tSourceName_s.Get() ) == 1 )  
			{
				t.tFoundInFPE = 1;
				t.tFileName_s = t.tSourceName_s;
			}
			if (  t.tFoundInFPE  ==  0 ) 
			{
				t.tFileName_s = t.importer.objectFileOriginalPath + t.tSourceName_s;
				if (  FileExist ( t.tFileName_s.Get() ) == 1 )  
				{
					t.tFoundInFPE = 1;
				}
			}
		}

		// see if the FPE texture has a relative path
		if ( t.tFoundInFPE == 0 ) 
		{
			t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\"+t.importer.objectFPE.textured;
			if (  FileExist( t.strwork.Get() ) == 1 ) 
			{
				t.tSourceName_s=""; t.tSourceName_s = t.tSourceName_s+g.fpscrootdir_s + "\\Files\\"+t.importer.objectFPE.textured;
				t.tFoundInFPE = 1;
				t.tFileName_s = ""; t.tFileName_s = t.tFileName_s + g.fpscrootdir_s + "\\Files\\"+t.importer.objectFPE.textured;
			}
		}

		// if FPE did have valid texture, add to list
		if ( t.tFoundInFPE == 1 ) 
		{
			importer_addtexturefiletolist ( t.tFileName_s, t.tSourceName_s, &tCount );
		}
	}

	// if above scans/fpe found no texture file(s), go through limb texture names (in case we're dealing with binary X file)
	if ( tCount == 0 ) 
	{
		SetObjectEffect (  t.importer.objectnumber,0 );
		//CloneMeshToNewFormat (  t.importer.objectnumber,530,1 ); // 220618 - for some reason importer adding second UV layer!
		PerformCheckListForLimbs (  t.importer.objectnumber );
		for ( t.tCount9 = 1 ; t.tCount9 <= ChecklistQuantity()-1; t.tCount9++ )
		{
			t.tlimbname_s = "";
			if (  LimbExist(t.importer.objectnumber,t.tCount9)  )  t.tlimbname_s = LimbTextureName(t.importer.objectnumber,t.tCount9);
			if (  t.tlimbname_s != "" ) 
			{
				t.tSourceName_s=t.tlimbname_s;
				t.tFileName_s=t.importer.objectFileOriginalPath+t.tSourceName_s;
				if (  FileExist(t.tFileName_s.Get()) == 0 ) 
				{
					t.tSourceName_s = ""; t.tSourceName_s=t.tSourceName_s+Left(t.tlimbname_s.Get(),Len(t.tlimbname_s.Get())-4)+".dds";
					t.tFileName_s=t.importer.objectFileOriginalPath+t.tSourceName_s;
				}
				if (  FileExist(t.tFileName_s.Get()) == 1 ) 
				{
					importer_addtexturefiletolist ( t.tFileName_s, t.tSourceName_s, &tCount );
				}
			}
		}
	}

	// if still no images, revert to default blank texture placeholder
	if ( tCount == 0 ) 
	{
		// if no actual texture file or FPE texture valid, use blank texture
		t.tSourceName_s = "languagebank\\neutral\\gamecore\\huds\\importer\\blankTex.png";
		importer_addtexturefiletolist ( t.tSourceName_s, t.tSourceName_s, &tCount );
	}

	// final image load count, load textures and sort txture button sprite
	importer_load_textures_finish ( tCount, false );
}

void importer_load_fpe ( void )
{
	//  reset physics shape count
	t.importer.isProtected = 0;
	t.importer.collisionShapeCount = 0;

	//  Split the filename into tokens to grab the path, object name and create fpe name
	Dim (  t.tArray,300 );
	Dim (  t.tUnknown,300 );

	t.importer.unknownFPELineCount = 0;

	//  Reset FPE data to default before attempting to load
	t.importer.objectFPE.desc = Left(t.importer.objectFilenameFPE.Get(),Len(t.importer.objectFilenameFPE.Get())-4);

	//  ;visualinfo
	t.importer.objectFPE.textured = "";
	if (  LimbExist(t.importer.objectnumber, 1) == 1  )  t.importer.objectFPE.textured  =  LimbTextureName(t.importer.objectnumber, 1);
	t.importer.objectFPE.effect = "effectbank\\reloaded\\entity_basic.fx";
	t.importer.objectFPE.castshadow = "0";
	t.importer.objectFPE.transparency = "0";
	//  ;orientation
	t.importer.objectFPE.model = t.importer.objectFilename;
	t.importer.objectFPE.offx = "0";
	t.importer.objectFPE.offy = "0";
	t.importer.objectFPE.offz = "0";
	t.importer.objectFPE.rotx = "0";
	t.importer.objectFPE.roty = "0";
	t.importer.objectFPE.rotz = "0";
	t.importer.objectFPE.scale = "100";
	t.importer.objectFPE.collisionmode = "0";
	t.importer.objectFPE.defaultstatic = "1";
	t.importer.objectFPE.materialindex = "0";
	t.importer.objectFPE.matrixmode = "0";
	t.importer.objectFPE.cullmode = "0";
	//  ;identity details
	t.importer.objectFPE.ischaracter = "0";
	t.importer.objectFPE.hasweapon = "";
	t.importer.objectFPE.isobjective = "0";
	t.importer.objectFPE.cantakeweapon = "0";
	//  ;statistics
	t.importer.objectFPE.strength = "25";
	t.importer.objectFPE.explodable= "0";
	t.importer.objectFPE.debrisshape = "0";
	//  ;ai
	t.importer.objectFPE.aimain = "default.lua";
	//  ;spawn
	t.importer.objectFPE.spawnmax = "0";
	t.importer.objectFPE.spawndelay = "0";
	t.importer.objectFPE.spawnqty = "0";
	//  ;anim
	t.importer.objectFPE.animmax = "0";
	t.tTotalFrames_s = "0";
	t.importer.objectFPE.anim0 = ""; t.importer.objectFPE.anim0 = t.importer.objectFPE.anim0+"0," + t.tTotalFrames_s;
	t.importer.objectFPE.playanimineditor = "0";
	t.importer.objectFPE.ignorecsirefs = "1";

	//  Check if an FPE exists, if so load it in
	if (  FileOpen(1) ) CloseFile (1);
	t.strwork = ""; t.strwork = t.strwork + t.importer.objectFileOriginalPath + t.importer.objectFilenameFPE;
	if (  FileExist ( t.strwork.Get() ) )  
	{
		t.strwork = ""; t.strwork = t.strwork +t.importer.objectFileOriginalPath + t.importer.objectFilenameFPE;
		OpenToRead (  1 , t.strwork.Get() );
		while (  FileEnd(1)  ==  0 ) 
		{
			t.tstring_s = ReadString (  1 );
			t.tempLine_s = t.tstring_s;

			t.tArrayMarker = 0;
			t.tToken_s=FirstToken(t.tstring_s.Get()," ");
			if (  t.tToken_s  !=  "" ) 
			{
				t.tArray[t.tArrayMarker] = t.tToken_s;
				++t.tArrayMarker;
			}
			do
			{
				t.tToken_s=NextToken(" ");
				if (  t.tToken_s  !=  "" ) 
				{
					t.tArray[t.tArrayMarker] = t.tToken_s;
					++t.tArrayMarker;
				}
			} while ( !(  t.tToken_s == "" ) );
			t.tStippedString_s = "";
			int tCount = 0;
			for ( tCount = 0 ; tCount<=  t.tArrayMarker-1; tCount++ )
			{
				if (  tCount < 3 ) 
				{
					t.tStippedString_s = t.tStippedString_s + t.tArray[tCount];
				}
				else
				{
					t.tStippedString_s = t.tStippedString_s + " " + t.tArray[tCount];
				}
			}
			if (  t.tStippedString_s  != "" && strcmp ( Left(t.tStippedString_s.Get(),1) , ";" ) != 0 ) 
			{
				t.tToken_s=FirstToken(t.tStippedString_s.Get(),"=");
				t.tToken2_s=NextToken("=");

				//  Get rid of any tabs that exist and replace with nothing (some files have tabs in sometimes)
				t.tstring_s = t.tToken_s ; t.tToken_s = "";
				for ( tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
				{
					if (  cstr(Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  t.tToken_s  =  t.tToken_s + Mid(t.tstring_s.Get(),tCount);
				}

				t.tstring_s = t.tToken2_s ; t.tToken2_s = "";
				for ( tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
				{
					if ( cstr( Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  t.tToken2_s  =  t.tToken2_s + Mid(t.tstring_s.Get(),tCount);
				}

				//  Header
				if ( t.tToken_s == "protected" ) { t.importer.isProtected  =  ValF(t.tToken2_s.Get()) ; }
				else if ( t.tToken_s == "desc" ) { t.importer.objectFPE.desc  =  t.tToken2_s ; }
				//  Visual Info
				else if ( t.tToken_s == "textured" ) { t.importer.objectFPE.textured  =  t.tToken2_s ; }
				else if ( t.tToken_s == "effect" ) { t.importer.objectFPE.effect  =  t.tToken2_s ; }
				else if ( t.tToken_s == "castshadow" ) { t.importer.objectFPE.castshadow  =  t.tToken2_s ; }
				else if ( t.tToken_s == "transparency" ) { t.importer.objectFPE.transparency  =  t.tToken2_s ; } 
				//  Orientation
				else if ( t.tToken_s == "model" ) { t.importer.objectFPE.model  =  t.tToken2_s ; }
				else if ( t.tToken_s == "offx" ) { t.importer.objectFPE.offx  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "offy" ) { t.importer.objectFPE.offy  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "offz" ) { t.importer.objectFPE.offz  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "rotx" ) { t.importer.objectFPE.rotx  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "roty" ) { t.importer.objectFPE.roty  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "rotz" ) { t.importer.objectFPE.rotz  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "scale" ) { t.importer.objectFPE.scale  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "collisionmode" ) { t.importer.objectFPE.collisionmode  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "defaultstatic" ) { t.importer.objectFPE.defaultstatic  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "materialindex" ) { t.importer.objectFPE.materialindex  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "matrixmode" ) { t.importer.objectFPE.matrixmode  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "cullmode" ) { t.importer.objectFPE.cullmode  =  t.tToken2_s ; } 
					
				//  Identity details
				else if ( t.tToken_s == "ischaracter" ) { t.importer.objectFPE.ischaracter  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "hasweapon" ) { t.importer.objectFPE.hasweapon  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "isobjective" ) { t.importer.objectFPE.isobjective  =  t.tToken2_s ; } 

				//  Statistics
				else if ( t.tToken_s == "strength" ) { t.importer.objectFPE.strength  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "explodable" ) { t.importer.objectFPE.explodable  =  t.tToken2_s ; } 
				else if ( t.tToken_s == "debrisshape" ) { t.importer.objectFPE.debrisshape  =  t.tToken2_s ; }
				//  AI
				else if ( t.tToken_s == "aimain" ) { importer_check_script_token_exists() ; t.importer.objectFPE.aimain  =  t.tToken2_s ; }
				//  Spawn
				else if ( t.tToken_s == "spawnmax" ) { t.importer.objectFPE.spawnmax  =  t.tToken2_s ; }
				else if ( t.tToken_s == "spawndelay" ) { t.importer.objectFPE.spawndelay  =  t.tToken2_s ; }
				else if ( t.tToken_s == "spawnqty" ) { t.importer.objectFPE.spawnqty  =  t.tToken2_s ; }
				//  Physics
				else if ( t.tToken_s == "physicscount" ) {
					t.tPhyscount = ValF(t.tToken2_s.Get());
					t.importer.collisionShapeCount = 0;
					while (  t.importer.collisionShapeCount < t.tPhyscount ) 
					{
						t.tstring_s = ReadString (  1 );

						t.tArrayMarker = 0;
						t.ttToken_s=FirstToken(t.tstring_s.Get()," ");
						if (  t.ttToken_s  !=  "" ) 
						{
							t.tArray[t.tArrayMarker] = t.ttToken_s;
							++t.tArrayMarker;
						}
						do
						{
							t.ttToken_s=NextToken(" ");
							if (  t.ttToken_s  !=  "" ) 
							{
								t.tArray[t.tArrayMarker] = t.ttToken_s;
								++t.tArrayMarker;
							}
						} while ( !(  t.ttToken_s == "" ) );
						t.tStippedString_s = "";
						for ( int tCount = 0 ; tCount<=  t.tArrayMarker-1; tCount++ )
						{
							if (  tCount < 3 ) 
							{
								t.tStippedString_s = t.tStippedString_s + t.tArray[tCount];
							}
							else
							{
								t.tStippedString_s = t.tStippedString_s + " " + t.tArray[tCount];
							}
						}
						if (  t.tStippedString_s  !=  "" && t.tStippedString_s.Get()[0]  !=  ';' ) 
						{
							t.ttToken_s=FirstToken(t.tStippedString_s.Get(),"=");
							t.ttToken2_s=NextToken("=");

							//  Get rid of any tabs that exist and replace with nothing (some files have tabs in sometimes)
							t.tstring_s = t.ttToken_s ; t.ttToken_s = "";
							for ( int tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
							{
								if (  cstr(Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  t.ttToken_s  =  t.ttToken_s + Mid(t.tstring_s.Get(),tCount);
							}

							t.tstring_s = t.ttToken2_s ; t.ttToken2_s = "";
							for ( int tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
							{
								if (  cstr(Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  t.ttToken2_s  =  t.ttToken2_s + Mid(t.tstring_s.Get(),tCount);
							}

							//  get rid of the quotation marks
							t.tStrip_s = t.ttToken2_s;
							t.tStrip_s = Left(t.tStrip_s.Get(), Len(t.tStrip_s.Get())-1);
							t.tStrip_s = Right(t.tStrip_s.Get(), Len(t.tStrip_s.Get())-1);

							t.tArrayMarker = 0;
							t.ttToken_s=FirstToken(t.tStrip_s.Get(),",");
							if (  t.ttToken_s  !=  "" ) 
							{
								t.tArray[t.tArrayMarker] = t.ttToken_s;
								++t.tArrayMarker;
							}
							do
							{
								t.ttToken_s=NextToken(",");
								if (  t.ttToken_s  !=  "" ) 
								{
									t.tArray[t.tArrayMarker] = t.ttToken_s;
									++t.tArrayMarker;
								}
							} while ( !(  t.ttToken_s == "" ) );

							//  Format; shapetype, sizex, sizey, sizez, offx, offy, offz, rotx, roty, rotz
							t.tPShapeType = ValF(t.tArray[0].Get());
							t.tPSizeX_f = ValF(t.tArray[1].Get());
							t.tPSizeY_f = ValF(t.tArray[2].Get());
							t.tPSizeZ_f = ValF(t.tArray[3].Get());
							t.tPOffX_f = ValF(t.tArray[4].Get());
							t.tPOffY_f = ValF(t.tArray[5].Get());
							t.tPOffZ_f = ValF(t.tArray[6].Get());
							t.tPRotX_f = ValF(t.tArray[7].Get());
							t.tPRotY_f = ValF(t.tArray[8].Get());
							t.tPRotZ_f = ValF(t.tArray[9].Get());
							importer_add_collision_box_loaded ( );
						}
					}
				}
				else
				{
					//  Store any unknown lines
					t.tUnknown[t.importer.unknownFPELineCount] = t.tempLine_s;
					++t.importer.unknownFPELineCount;
				}
			}
		}

	}

	CloseFile (  1 );

	//  Cleanup
	UnDim (  t.tArray );

	importer_apply_fpe ( );
}

void importer_apply_fpe ( void )
{
	//  Scale
	t.slidersmenuvalue[t.importer.properties1Index][1].value = ValF(t.importer.objectFPE.scale.Get());
	t.importer.oldScale = t.slidersmenuvalue[t.importer.properties1Index][1].value;

	//  angle
	t.importer.objectAngleY = ValF(t.importer.objectFPE.roty.Get());
	t.slidersmenuvalue[t.importer.properties1Index][3].value = t.importer.objectAngleY;
	YRotateObject (  t.importer.objectnumber,t.importer.objectAngleY );

	//  Collision Mode
	if (  t.importer.objectFPE.collisionmode  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "Box";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=1;
	}
	if (  t.importer.objectFPE.collisionmode  ==  "1" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "Polygon";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=2;
	}
	if (  t.importer.objectFPE.collisionmode  ==  "99" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "No Collision";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=3;
	}
	if (  t.importer.objectFPE.collisionmode  ==  "1001" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "Limb One Box";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=4;
	}
	if (  t.importer.objectFPE.collisionmode  ==  "2001" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "Limb One Poly";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=5;
	}
	if (  t.importer.objectFPE.collisionmode  ==  "40" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][4].value_s = "Collision Boxes";
		t.slidersmenuvalue[t.importer.properties1Index][4].value=6;
	}

	//  Default Static
	if (  t.importer.objectFPE.defaultstatic  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][5].value_s = "No";
		t.slidersmenuvalue[t.importer.properties1Index][5].value=2;
	}
	else
	{
		t.slidersmenuvalue[t.importer.properties1Index][5].value_s = "Yes";
		t.slidersmenuvalue[t.importer.properties1Index][5].value=1;
	}

	//  Strength
	t.slidersmenuvalue[t.importer.properties1Index][6].value = ValF(t.importer.objectFPE.strength.Get());

	//  Cull Mode
	if (  t.importer.objectFPE.cullmode  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][8].value_s = "No";
		t.slidersmenuvalue[t.importer.properties1Index][8].value=2;
	}
	else
	{
		t.slidersmenuvalue[t.importer.properties1Index][8].value_s = "Yes";
		t.slidersmenuvalue[t.importer.properties1Index][8].value=1;
	}

	//  Transparency
	if (  t.importer.objectFPE.transparency  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][9].value_s = "None";
		t.slidersmenuvalue[t.importer.properties1Index][9].value=1;
	}
	if (  t.importer.objectFPE.transparency  ==  "1" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][9].value_s = "Standard";
		t.slidersmenuvalue[t.importer.properties1Index][9].value=2;
	}
	if (  t.importer.objectFPE.transparency  ==  "2" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][9].value_s = "Render last";
		t.slidersmenuvalue[t.importer.properties1Index][9].value=3;
	}
	//  Material Inxed
	if (  t.importer.objectFPE.materialindex  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][10].value_s = "Generic";
		t.slidersmenuvalue[t.importer.properties1Index][10].value=1;
	}
	if (  t.importer.objectFPE.materialindex  ==  "1" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][10].value_s = "Stone";
		t.slidersmenuvalue[t.importer.properties1Index][10].value=2;
	}
	if (  t.importer.objectFPE.materialindex  ==  "2" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][10].value_s = "Metal";
		t.slidersmenuvalue[t.importer.properties1Index][10].value=3;
	}
	if (  t.importer.objectFPE.materialindex  ==  "3" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][10].value_s = "Wood";
		t.slidersmenuvalue[t.importer.properties1Index][10].value=4;
	}
	//  Is Character
	if (  t.importer.objectFPE.ischaracter  ==  "0" ) 
	{
		t.slidersmenuvalue[t.importer.properties1Index][7].value_s = "No";
		t.slidersmenuvalue[t.importer.properties1Index][7].value=1;
	}
	else
	{
		t.slidersmenuvalue[t.importer.properties1Index][7].value_s = "Yes";
		t.slidersmenuvalue[t.importer.properties1Index][7].value=2;
	}

	//  Shader effect
	t.importer.objectFPE.effect = Right(t.importer.objectFPE.effect.Get(), Len(t.importer.objectFPE.effect.Get())-20);

	for ( t.tc = 1 ; t.tc<=  t.importer.shaderFileCount; t.tc++ )
	{
		if (  t.importerShaderFiles[t.tc]  ==  t.importer.objectFPE.effect ) 
		{
			t.slidersmenuvalue[t.importer.properties1Index][2].value=t.tc;
			t.slidersmenuvalue[t.importer.properties1Index][2].value_s=t.importerShaderFiles[t.tc];
			break;
		}
	}
	//  AI MAIN
	for ( t.tc = 1 ; t.tc<=  t.importer.scriptFileCount; t.tc++ )
	{
		if (  t.importerScriptFiles[t.tc]  ==  t.importer.objectFPE.aimain ) 
		{
			t.slidersmenuvalue[t.importer.properties1Index][11].value=t.tc;
			t.slidersmenuvalue[t.importer.properties1Index][11].value_s=t.importerScriptFiles[t.tc];
			break;
		}
	}

	// other UI panels that need updating externally
	//t.slidersmenuvaluechoice=115;
	//t.slidersmenuvalue[t.importer.properties1Index][13].value=1+g_iFBXGeometryToggleMode;
	//t.slidersmenuvalueindex=t.slidersmenuvalue[t.importer.properties1Index][13].value;
	//sliders_getnamefromvalue ( );
	//t.slidersmenuvalue[t.importer.properties1Index][13].value_s=t.slidervaluename_s;
	//t.slidersmenuvaluechoice=116;
	//t.slidersmenuvalue[t.importer.properties1Index][14].value=1+g_iFBXGeometryCenterMesh;
	//t.slidersmenuvalueindex=t.slidersmenuvalue[t.importer.properties1Index][14].value;
	//sliders_getnamefromvalue ( );
	//t.slidersmenuvalue[t.importer.properties1Index][14].value_s=t.slidervaluename_s;
}

void importer_save_fpe ( void )
{
	//  update FPE data from importer sliders
	//  Angle
	t.importer.objectFPE.roty = Str(t.importer.objectAngleY);
	//  Scale
	t.importer.objectFPE.scale = Str(t.slidersmenuvalue[t.importer.properties1Index][1].value);
	//  Collision Mode
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 1  )  t.importer.objectFPE.collisionmode  =  "0";
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 2  )  t.importer.objectFPE.collisionmode  =  "1";
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 3  )  t.importer.objectFPE.collisionmode  =  "99";
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 4  )  t.importer.objectFPE.collisionmode  =  "1001";
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 5  )  t.importer.objectFPE.collisionmode  =  "2001";
	if (  t.slidersmenuvalue[t.importer.properties1Index][4].value == 6  )  t.importer.objectFPE.collisionmode  =  "40";
	//  Default Static
	if ( t.slidersmenuvalue[t.importer.properties1Index][5].value == 2 ) 
		t.importer.objectFPE.defaultstatic = "0";
	else
		t.importer.objectFPE.defaultstatic = "1";
	//  Cull Mode
	if ( t.slidersmenuvalue[t.importer.properties1Index][8].value == 2 ) 
		t.importer.objectFPE.cullmode = "0";
	else
		t.importer.objectFPE.cullmode = "1";
	//  Transparency
	t.importer.objectFPE.transparency = Str(t.slidersmenuvalue[t.importer.properties1Index][9].value-1);
	//  MaterialIndex
	t.importer.objectFPE.materialindex = Str(t.slidersmenuvalue[t.importer.properties1Index][10].value-1);
	//  Strength
	t.importer.objectFPE.strength = Str(t.slidersmenuvalue[t.importer.properties1Index][6].value);
	//  Explodable
	t.importer.objectFPE.explodable = "0";
	//  castshadow
	t.importer.objectFPE.castshadow = "0";
	//  ischaracter
	if ( t.slidersmenuvalue[t.importer.properties1Index][7].value <= 1 ) 
	{
		t.importer.objectFPE.ischaracter = "0";
	}
	else
	{
		// leave character as is, or convert to Uber Animation and rename Skeleton (done after this function call)
		t.importer.objectFPE.ischaracter = "1";
	}
	//  isobjective
	t.importer.objectFPE.isobjective = "0";//Str(t.slidersmenuvalue[t.importer.properties1Index][8].value);//bug
	//  cantakeweapon
	t.importer.objectFPE.cantakeweapon = "0";
	//  update shader selection
	t.importer.objectFPE.effect = ""; t.importer.objectFPE.effect = t.importer.objectFPE.effect+"effectbank\\reloaded\\" + t.slidersmenuvalue[t.importer.properties1Index][2].value_s;
	if ( strnicmp ( t.slidersmenuvalue[t.importer.properties1Index][2].value_s.Get(), "apbr_", 5 ) == NULL ) 
		g_iPreferPBR = 1;
	else
		g_iPreferPBR = 0;

	//  aimain
	t.importer.objectFPE.aimain = t.slidersmenuvalue[t.importer.properties1Index][11].value_s;

	// ensure collision objects are back to reset position (ignoring camera height adjustment)
	importer_RestoreCollisionShiftHeight();

	t.chosenFileNameFPE_s = t.importer.tFPESaveName;
	if (  FileOpen (1)  )  CloseFile (1);
	if (  FileExist (t.chosenFileNameFPE_s.Get())  )  DeleteAFile (  t.chosenFileNameFPE_s.Get() ) ;
	OpenToWrite (  1, t.chosenFileNameFPE_s.Get() );

	t.tString = ";Saved by GameGuru Object Importer v1.000" ;WriteString (  1 , t.tString.Get() );
	t.tString = ";header" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("desc") + "= " + t.importer.objectFPE.desc ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";visualinfo" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("textured") + "= " + t.importer.objectFPE.textured ;WriteString (  1 , t.tString.Get() );

	// a new system to record texture references in FPE so can save standalone better
	for ( int tCount = 1 ; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		if ( strlen ( t.importerTextures[tCount].fileName.Get() ) > 0 ) 
		{
			char pFileOnly[2048];
			strcpy ( pFileOnly, t.importerTextures[tCount].fileName.Get() );
			for ( int n = strlen(pFileOnly)-1; n > 0; n-- )
			{
				if ( pFileOnly[n] == '\\' || pFileOnly[n] == '/' )
				{
					strcpy ( pFileOnly, pFileOnly+n+1 );
					break;
				}
			}
			t.tString = "" ; t.tString = t.tString + importerPadString(cstr(cstr("textureref")+cstr(tCount)).Get()) + " = " + cstr(pFileOnly);
			WriteString (  1 , t.tString.Get() );
		}
	}

	t.tString = "" ; t.tString = t.tString + importerPadString("effect") + "= " + t.importer.objectFPE.effect ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("castshadow") + "= " + t.importer.objectFPE.castshadow ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("transparency") + "= " + t.importer.objectFPE.transparency ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = "" ; t.tString = t.tString + ";orientation" ; WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("model") + "= " + t.importer.objectFPE.model ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("offx") + "= " + t.importer.objectFPE.offx ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("offy") + "= " + t.importer.objectFPE.offy ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("offz") + "= " + t.importer.objectFPE.offz ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("rotx") + "= " + t.importer.objectFPE.rotx ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("roty") + "= " + t.importer.objectFPE.roty ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("rotz") + "= " + t.importer.objectFPE.rotz ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("scale") + "= " + t.importer.objectFPE.scale ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("collisionmode") + "= " + t.importer.objectFPE.collisionmode ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("defaultstatic") + "= " + t.importer.objectFPE.defaultstatic ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("materialindex") + "= " + t.importer.objectFPE.materialindex ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("matrixmode") + "= " + t.importer.objectFPE.matrixmode ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("cullmode") + "= " + t.importer.objectFPE.cullmode ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";physics shapes" ;WriteString (  1 , t.tString.Get() );
	if ( t.importer.objectFPE.collisionmode != "40" )
	{
		// no physics box collision
		t.tString = ""; t.tString = t.tString + importerPadString("physicscount") + "= 0"; WriteString ( 1, t.tString.Get() );
	}
	else
	{
		// physics box collision used, so export details
		t.tString = ""; t.tString = t.tString + importerPadString("physicscount") + "= " + Str(t.importer.collisionShapeCount) ; WriteString (  1, t.tString.Get() );
		t.tExportScale_f = 100.0 / t.importer.objectScaleForEditing;
		for ( t.tPCount = 0 ; t.tPCount<=  t.importer.collisionShapeCount - 1; t.tPCount++ )
		{
			t.strwork = ""; t.strwork = t.strwork + "physics" + Str(t.tPCount);
			t.tString = ""; t.tString = t.tString+importerPadString( t.strwork.Get() ) + "= " + Chr(34);
			t.tString = t.tString + "0,";
			t.tString = t.tString + Str(t.importerCollision[t.tPCount].sizex * t.tExportScale_f) + ",";
			t.tString = t.tString + Str(t.importerCollision[t.tPCount].sizey * t.tExportScale_f) + ",";
			t.tString = t.tString + Str(t.importerCollision[t.tPCount].sizez * t.tExportScale_f) + ",";
			t.tString = t.tString + Str(ObjectPositionX(t.importerCollision[t.tPCount].object2) * t.tExportScale_f) + ",";
			t.tString = t.tString + Str((ObjectPositionY(t.importerCollision[t.tPCount].object2)* t.tExportScale_f) ) + ",";
			t.tString = t.tString + Str(ObjectPositionZ(t.importerCollision[t.tPCount].object2) * t.tExportScale_f) + ",";
			t.tString = t.tString + Str(ObjectAngleX(t.importerCollision[t.tPCount].object2)) + ",";
			t.tString = t.tString + Str(ObjectAngleY(t.importerCollision[t.tPCount].object2)) + ",";
			t.tString = t.tString + Str(ObjectAngleZ(t.importerCollision[t.tPCount].object2));
			t.tString = t.tString + Chr(34);
			WriteString (  1, t.tString.Get() );
		}
	}
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";identity details" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("ischaracter") + "= " + t.importer.objectFPE.ischaracter ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("hasweapon") + "= " + t.importer.objectFPE.hasweapon ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("isobjective") + "= " + t.importer.objectFPE.isobjective ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("cantakeweapon") + "= " + t.importer.objectFPE.cantakeweapon ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";statistics" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("strength") + "= " + t.importer.objectFPE.strength ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("explodable") + "= " + t.importer.objectFPE.explodable ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("debrisshape") + "= " + t.importer.objectFPE.debrisshape ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";ai" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("aimain") + "= " + t.importer.objectFPE.aimain ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";spawn" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("spawnmax") + "= " + t.importer.objectFPE.spawnmax ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("spawndelay") + "= " + t.importer.objectFPE.spawndelay ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("spawnqty") + "= " + t.importer.objectFPE.spawnqty ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ;WriteString (  1 , t.tString.Get() );

	t.tString = ";anim" ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("animmax") + "= " + t.importer.objectFPE.animmax ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("anim0") + "= " + t.importer.objectFPE.anim0 ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("playanimineditor") + "= " + t.importer.objectFPE.playanimineditor ;WriteString (  1 , t.tString.Get() );
	t.tString = "" ; t.tString = t.tString + importerPadString("ignorecsirefs") + "= " + t.importer.objectFPE.ignorecsirefs ;WriteString (  1 , t.tString.Get() );

	//  Save any lines that were not understand when the initial FPE was loaded in
	//  add a blank Line (  (  if there are extra Lines to add ) )
	if (  t.importer.unknownFPELineCount > 0 ) 
	{

		t.tString = "" ;WriteString (  1 , t.tString.Get() );
		t.tString = ";settings not understood by the exporter" ;WriteString (  1 , t.tString.Get() );

		for ( int tCount = 0 ; tCount<=  t.importer.unknownFPELineCount-1; tCount++ )
		{
				if (  t.tUnknown[tCount].Get()[0] !=  ';' && Len(t.tUnknown[tCount].Get() ) > 2 ) 
				{
					WriteString (  1 , t.tUnknown[tCount].Get() );
				}
		}

	}

	CloseFile (  1 );
}

void importer_handleScale ( void )
{
	//  Show or hide guide model
	if (  t.importerTabs[10].selected  ==  1 && g.tabmode  ==  IMPORTERTABPAGE1 ) 
	{
		ShowObject (  t.importer.dummyCharacterObjectNumber );
	}
	else
	{
		HideObject (  t.importer.dummyCharacterObjectNumber );
	}
	if (  t.importer.oldScale  !=  t.slidersmenuvalue[t.importer.properties1Index][1].value || t.importer.showScaleChange > 0 ) 
	{
		t.importer.message = ""; t.importer.message = t.importer.message + "Scale:" + Str(t.slidersmenuvalue[t.importer.properties1Index][1].value) + "%. Object displayed in relation to t.a typical character";
		t.importer.oldScale = t.slidersmenuvalue[t.importer.properties1Index][1].value;
		if (  t.importer.showScaleChange  <=  0  )  t.importer.showScaleChange  =  1;
		if (  t.inputsys.mclick  ==  0  )  --t.importer.showScaleChange;
	}
	t.tscale_f = t.importer.objectScaleForEditing;
	t.tScaleMultiplier_f = t.slidersmenuvalue[t.importer.properties1Index][1].value / 100.0;
	t.tscale_f = t.tscale_f / t.tScaleMultiplier_f;
	t.tscale_f = t.tscale_f * t.importer.camerazoom;
	ScaleObject (  t.importer.dummyCharacterObjectNumber , t.tscale_f , t.tscale_f , t.tscale_f * 0.2 );
}

void importer_draw ( void )
{
	//  draw to screen if active
	if ( t.importer.loaded == 1 && ImageExist(g.importermenuimageoffset+1) ) 
	{
		if ( t.importer.scaleMulti == 1.0 ) 
		{
			t.timg=g.importermenuimageoffset;
			t.tscrx=(GetChildWindowWidth()-ImageWidth(t.timg+1))/2;
			t.tscry=(GetChildWindowHeight()/2) - 300;
			PasteImage (  t.timg+1,t.tscrx-4,t.tscry,1 );
			t.ttopy=t.tscry+ImageHeight(t.timg+1);
			for ( t.tlong = 0 ; t.tlong<=  4; t.tlong++ )
			{
				PasteImage (  t.timg+2,t.tscrx-2,t.ttopy,1 );
				t.ttopy += ImageHeight(t.timg+2);
			}
			PasteImage (  t.timg+3,t.tscrx,t.ttopy,1 );
			pastebitmapfont(t.importer.message.Get(),(GetChildWindowWidth()/2) - (getbitmapfontwidth (t.importer.message.Get(),1)/2),t.tscry+525,1,255);
			pastebitmapfont(t.importer.viewMessage.Get(),(GetChildWindowWidth()/2) - 295,t.tscry+90,1,255);
		}
		else
		{
			pastebitmapfont(t.importer.message.Get(),(GetChildWindowWidth()/2) - (getbitmapfontwidth (t.importer.message.Get(),1)/2),40,1,255);
			t.strwork = ""; t.strwork = t.strwork + "View: " + t.importer.viewMessage;
			pastebitmapfont( t.strwork.Get() ,10,5,1,255);
		}

		// extra help information
		if ( t.importerTabs[1].selected == 1 )
		{
			LPSTR pExtraHelp = "Use W and S to raise camera view, and mouse wheel to zoom in and out";
			if ( strlen ( g_pShowFilenameHoveringOver ) > 0 ) pExtraHelp = g_pShowFilenameHoveringOver;
			pastebitmapfont(pExtraHelp,(GetChildWindowWidth()/2) - (getbitmapfontwidth (pExtraHelp,1)/2),860,1,255);
		}

		// Draw the sliders
		sliders_readall ( );

		// Slider management
		sliders_loop ( );

		//  Draw sliders last
		sliders_draw ( );
	}
}

void importer_quit ( void )
{
	importer_fade_out ( );
	//  load model specified by timporterfile$
	if (  ObjectExist(t.importer.objectnumber) == 1 ) 
	{
		DeleteObject (  t.importer.objectnumber );
	}

	//  dummy character model
	if (  ObjectExist(t.importer.dummyCharacterObjectNumber)  )  DeleteObject (  t.importer.dummyCharacterObjectNumber );

	importer_free ( );
	t.importer.importerActive = 0;
}

void ConvertWorldToRelative ( sFrame* pFrame, GGMATRIX* pStoreNewPoseFrames, GGMATRIX* pTraverseMatrix )
{
	if ( pFrame )
	{
		// take the world bone from pStoreNewPoseFrames for this frame ID and create relative matrix from it
		int iFrameIndex = pFrame->iID;
		GGMATRIX matWorldBone = pStoreNewPoseFrames[iFrameIndex];

		// go through hierarchy, create matCombined as you go, transforming worldbones into relative bones
		float fDet = 0;
		GGMATRIX matInverseOfTraverse = *pTraverseMatrix;
		GGMatrixInverse ( &matInverseOfTraverse, &fDet, pTraverseMatrix );
		GGMATRIX matWorldBoneInBaseSpace;
		GGMatrixMultiply ( &matWorldBoneInBaseSpace, &matWorldBone, &matInverseOfTraverse );
		GGMATRIX matRelativeBone = matWorldBoneInBaseSpace;

		/* discovered we need to progress matCombined as real world to work differences with stored bones
		// world1 starting accumilated reference point
		GGMATRIX matW1 = *pTraverseMatrix;//matWorldBone; // parent world matrix
		// relative bone
		GGMATRIX matR; // the relative rotation and translation (as a bone would work)
		// world2 result
		GGMATRIX matW2 = matWorldBone;//*pTraverseMatrix; // child world matrix (further down hierarchy)
		// now work out the original relative bone which got W1 to W2
		// work out difference between rotations of W1 and W2
		GGMATRIX matRot2 = matW2;
		matRot2._41 = 0;
		matRot2._42 = 0;
		matRot2._43 = 0;
		GGMATRIX matRot1 = matW1;
		matRot1._41 = 0;
		matRot1._42 = 0;
		matRot1._43 = 0;
		float fDet = 0;
		GGMATRIX matRotInverse;
		GGMatrixInverse ( &matRotInverse, &fDet, &matRot1 );
		GGMATRIX matRot;
		GGMatrixMultiply ( &matRot, &matRotInverse, &matRot2 );
		// transform W1/W2 into traversing matricx base to get correct translation (relative to base, not world up base)
		GGMATRIX matW1Trans;
		GGMATRIX matW2Trans;
		GGMatrixMultiply ( &matW1Trans, &matW1, &matRot1 );
		GGMatrixMultiply ( &matW2Trans, &matW2, &matRot1 );
		// work out world offset difference 
		GGMATRIX matPos;
		GGMatrixIdentity ( &matPos );
		matPos._41 = matW2Trans._41 - matW1Trans._41;
		matPos._42 = matW2Trans._42 - matW1Trans._42;
		matPos._43 = matW2Trans._43 - matW1Trans._43;
		// again, rotation difference was calculated from world positions, transform to traverse base so its relative!
		GGMATRIX matRotAlignedToTraversingBase = matRot;
		GGMatrixMultiply ( &matRotAlignedToTraversingBase, &matRot, &matRot1 );
		// combine for final R matrix
		GGMATRIX matRelativeBone;
		matRelativeBone = matRotAlignedToTraversingBase;
		matRelativeBone._41 = matPos._41;
		matRelativeBone._42 = matPos._42;
		matRelativeBone._43 = matPos._43;
		// test that the new relative matrix works on W1 to get W2 again!
		GGMatrixMultiply ( &matW2, &matRelativeBone, &matW1 );
		*/

		// finally 'restore' the relative bone!
		pFrame->matTransformed = matRelativeBone;

		// use relative bone to calculate world bones as we go
		GGMatrixMultiply ( &pFrame->matCombined, &pFrame->matTransformed, pTraverseMatrix );

		// convert child frames
		ConvertWorldToRelative ( pFrame->pChild, pStoreNewPoseFrames, &pFrame->matCombined );

		// convert sibling frames
		ConvertWorldToRelative ( pFrame->pSibling, pStoreNewPoseFrames, pTraverseMatrix );	
	}
}

void importer_save_entity ( void )
{
	//  Check if user folder exists, if not create it
	t.strwork = ""; t.strwork = t.strwork + t.importer.startDir + "\\entitybank\\user";
	if (  PathExist( t.strwork.Get() )  ==  0 ) 
	{
		MakeDirectory (  t.strwork.Get() );
	}

	//  Ask for save filename
	t.tSaveFile_s = "";
	t.timporterprotected = 1;
	t.timportermessage_s = "Save Entity";
	while (  t.timporterprotected  ==  1 ) 
	{
		if (  t.importer.fpeIsMainFile  ==  0 ) 
		{
			t.strwork = ""; t.strwork = t.strwork +t.importer.startDir + "\\entitybank\\user";
			t.tSaveFile_s = openFileBox("Model (.dbo)|*.dbo|All Files|*.*|", t.strwork.Get() , t.timportermessage_s.Get(), ".dbo", IMPORTERSAVEFILE);
		}
		else
		{
			t.strwork = ""; t.strwork = t.strwork +t.importer.startDir + "\\entitybank\\user";
			t.tSaveFile_s = openFileBox("GG Entity (.fpe)|*.fpe|All Files|*.*|", t.strwork.Get(), t.timportermessage_s.Get(), ".fpe", IMPORTERSAVEFILE);
		}
		if (  t.tSaveFile_s  ==  "Error" ) 
		{
			t.timportersaveon = 0;
			return;
		}
		t.timporterprotected = importer_check_if_protected(t.tSaveFile_s.Get());
		if (  t.timporterprotected  ==  1 ) 
		{
			t.timportermessage_s = "You cannot overwrite protected media, please choose an alternative name";
		}
	}

	//  Ensure it has the dbo extension
	Dim (  t.tArray,300  );
	t.tArrayMarker = 0;
	t.tstring_s=t.tSaveFile_s;
	t.tToken_s=FirstToken(t.tstring_s.Get(),".");
	if (  t.tToken_s  !=  "" ) 
	{
		t.tArray[t.tArrayMarker] = t.tToken_s;
		++t.tArrayMarker;
	}
	do
	{
		t.tToken_s=NextToken(".");
		if (  t.tToken_s  !=  "" ) 
		{
			t.tArray[t.tArrayMarker] = t.tToken_s;
			++t.tArrayMarker;
		}
	} while ( !(  t.tToken_s == "" ) );
	t.tStippedFileName_s = "";
	int tCount = 0;
	for ( tCount = 0 ; tCount<=  t.tArrayMarker-2; tCount++ )
	{
		t.tStippedFileName_s = t.tStippedFileName_s + t.tArray[tCount];
	}
	UnDim (  t.tArray );
	t.tStippedFileName_s = t.tSaveFile_s;
	if (  strcmp ( Mid(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-1)  ,  "." ) == 0 ) 
	{
		t.tStippedFileName_s = Left(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-2);
	}
	else
	{
		t.tStippedFileName_s = Left(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-4);
	}

	//  Grab the folder path
	Dim (  t.tArray,300  );
	t.tArrayMarker = 0;
	t.tstring_s=t.tStippedFileName_s;
	t.tToken_s=FirstToken(t.tstring_s.Get(),"\\");
	if (  t.tToken_s  !=  "" ) 
	{
		t.tArray[t.tArrayMarker] = t.tToken_s;
		++t.tArrayMarker;
	}
	do
	{
		t.tToken_s=NextToken("\\");
		if (  t.tToken_s  !=  "" ) 
		{
			t.tArray[t.tArrayMarker] = t.tToken_s;
			++t.tArrayMarker;
		}
	} while ( !(  t.tToken_s == "" ) );
	t.tStippedFileName_s = "";
	for ( tCount = 0 ; tCount<=  t.tArrayMarker-2; tCount++ )
	{
		t.tStippedFileName_s = t.tStippedFileName_s + t.tArray[tCount] + "\\";
	}
	t.tSavePath_s = t.tStippedFileName_s;

	//  Store file names
	if (  t.importer.fpeIsMainFile  ==  0 ) 
	{
		t.tSaveFile_s = t.tArray[t.tArrayMarker-1] + ".dbo";
		t.tSaveThumb_s = t.tArray[t.tArrayMarker-1] + ".BMP";
		t.importer.tFPESaveName = t.tArray[t.tArrayMarker-1] + ".fpe";
		t.importer.objectFPE.desc = t.tArray[t.tArrayMarker-1];
		t.importer.objectFPE.model = t.tSaveFile_s;
	}
	else
	{
		t.tSaveFile_s = t.importer.objectFilename;
		t.tSaveThumb_s = t.tArray[t.tArrayMarker-1] + ".BMP";
		t.importer.tFPESaveName = t.tArray[t.tArrayMarker-1] + ".fpe";
		t.importer.objectFPE.model = t.tSaveFile_s;
	}

	// Just before save object, ensure all texture references don't include root path
	sObject* pObject = GetObjectData ( t.importer.objectnumber );
	if ( pObject )
	{
		for ( int iMeshIndex = 0; iMeshIndex < pObject->iMeshCount; iMeshIndex++ )
		{
			sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
			if ( pMesh )
			{
				for ( int iTextureStage = 0; iTextureStage < pMesh->dwTextureCount; iTextureStage++ )
				{
					LPSTR pTexName = pMesh->pTextures[iTextureStage].pName;
					if ( strlen ( pTexName ) > 2 )
					{
						if ( pTexName[1] == ':' )
						{
							for ( int n = strlen(pTexName)-1; n > 0; n-- )
							{
								if ( pTexName[n] == '\\' || pTexName[n] == '/' )
								{
									strcpy ( pTexName, pTexName+n+1 );
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	// if selected 'Use Uber Anims', then rename skeleton and apply uber animations automatically
	if ( t.slidersmenuvalue[t.importer.properties1Index][7].value == 3 ) 
	{
		sObject* pObject = GetObjectData(t.importer.objectnumber);
		if ( pObject )
		{
			// load in correct Y pose and overwrite imported modes transform matrices
			GGMATRIX* pStoreNewPoseFrames = NULL;
			int objectnumberforframedatacopy = findFreeObject();
			cstr pAbsPathToUberAnimFile = g.fpscrootdir_s + "\\Files\\entitybank\\Characters\\2CHAINFINGER-YPOSE.dbo";//Uber Soldier.X";//appendanims.x";
			LoadObject ( pAbsPathToUberAnimFile.Get(), objectnumberforframedatacopy );
			if ( ObjectExist ( objectnumberforframedatacopy ) == 1 )
			{
				sObject* pObjectWithYPoseData = GetObjectData ( objectnumberforframedatacopy );
				if ( pObjectWithYPoseData )
				{
					// go through and find differences between poses
					for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
					{
						sFrame* pFrame = pObject->ppFrameList[iFrame];
						if ( pFrame )
						{
							LPSTR pFrameName = pFrame->szName;
							if ( pFrameName )
							{
								if ( strlen(pFrameName) > 0 )
								{
									// for this imported model frame, find the equivilant frame in the appendanim model (with the Y pose hidden in the skinning transform)
									for ( int iFindFrame = 0; iFindFrame < pObjectWithYPoseData->iFrameCount; iFindFrame++ )
									{
										sFrame* pFindFrame = pObjectWithYPoseData->ppFrameList[iFindFrame];
										if ( pFindFrame )
										{
											LPSTR pFindFrameName = pFindFrame->szName;
											if ( pFindFrameName )
											{
												if ( strlen(pFindFrameName) > 0 )
												{
													if ( stricmp ( pFrameName, pFindFrameName ) == NULL )
													{
														// just grab the local relative transform from the Y pose model
														pFrame->matOriginal = pFindFrame->matOriginal;
														pFrame->matTransformed = pFindFrame->matOriginal;

														// done with this findframe
														break;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				DeleteObject ( objectnumberforframedatacopy );
			}

			// go through all frames of imported model
			for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
			{
				sFrame* pFrame = pObject->ppFrameList[iFrame];
				if ( pFrame )
				{
					LPSTR pOldFrameName = pFrame->szName;
					if ( pOldFrameName )
					{
						if ( strlen(pOldFrameName) > 0 )
						{
							// look for common bone names, and transform to GG standard skeleton name
							const char* pNewName = "";
							if ( stricmp ( pOldFrameName, "mixamorig_Hips" ) == NULL ) pNewName = "Bip01_Pelvis";
							if ( stricmp ( pOldFrameName, "mixamorig_Spine" ) == NULL ) pNewName = "Bip01_Spine";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftUpLeg" ) == NULL ) pNewName = "Bip01_L_Thigh";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftLeg" ) == NULL ) pNewName = "Bip01_L_Calf";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftFoot" ) == NULL) pNewName = "Bip01_L_Foot";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftToeBase" ) == NULL ) pNewName = "Bip01_L_Toe0";
							if ( stricmp ( pOldFrameName, "mixamorig_RightUpLeg" ) == NULL ) pNewName = "Bip01_R_Thigh";
							if ( stricmp ( pOldFrameName, "mixamorig_RightLeg" ) == NULL ) pNewName = "Bip01_R_Calf";
							if ( stricmp ( pOldFrameName, "mixamorig_RightFoot" ) == NULL ) pNewName = "Bip01_R_Foot";
							if ( stricmp ( pOldFrameName, "mixamorig_RightToeBase" ) == NULL ) pNewName = "Bip01_R_Toe0";
							if ( stricmp ( pOldFrameName, "mixamorig_Spine1" ) == NULL ) pNewName = "Bip01_Spine1";
							if ( stricmp ( pOldFrameName, "mixamorig_Spine2" )== NULL ) pNewName = "Bip01_Spine2";
							if ( stricmp ( pOldFrameName, "mixamorig_Neck" ) == NULL ) pNewName = "Bip01_Neck";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftShoulder" ) == NULL ) pNewName = "Bip01_L_Clavicle";
							if ( stricmp ( pOldFrameName, "mixamorig_RightShoulder" ) == NULL ) pNewName = "Bip01_R_Clavicle";
							if ( stricmp ( pOldFrameName, "mixamorig_Head" ) == NULL ) pNewName = "Bip01_Head";
							if ( stricmp ( pOldFrameName, "mixamorig_HeadTop_End" ) == NULL ) pNewName = "Bip01_HeadTop";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftArm" ) == NULL ) pNewName = "Bip01_L_UpperArm";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftForeArm" ) == NULL ) pNewName = "Bip01_L_Forearm";
							if ( stricmp ( pOldFrameName, "mixamorig_RightArm" ) == NULL ) pNewName = "Bip01_R_UpperArm";
							if ( stricmp ( pOldFrameName, "mixamorig_RightForeArm" ) == NULL ) pNewName = "Bip01_R_Forearm";
							if ( stricmp ( pOldFrameName, "mixamorig_LeftHand" ) == NULL ) pNewName = "Bip01_L_Hand";
							if ( stricmp ( pOldFrameName, "mixamorig_RightHand" ) == NULL ) pNewName = "Bip01_R_Hand";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandThumb1" ) > 0 ) pNewName = "Bip01_L_Finger0";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandThumb2" ) > 0 ) pNewName = "Bip01_L_Finger01";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandThumb3" ) > 0 ) pNewName = "Bip01_L_Finger02";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandThumb4" ) > 0 ) pNewName = "Bip01_L_Finger03";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandIndex1" ) > 0 ) pNewName = "Bip01_L_Finger1";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandIndex2" ) > 0 ) pNewName = "Bip01_L_Finger11";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandIndex3" ) > 0 ) pNewName = "Bip01_L_Finger12";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandIndex4" ) > 0 ) pNewName = "Bip01_L_Finger13";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandMiddle1" ) > 0 ) pNewName = "Bip01_L_Finger2";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandMiddle2" ) > 0 ) pNewName = "Bip01_L_Finger21";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandMiddle3" ) > 0 ) pNewName = "Bip01_L_Finger22";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandMiddle4" ) > 0 ) pNewName = "Bip01_L_Finger23";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandRing1" ) > 0 ) pNewName = "Bip01_L_Finger3";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandRing2" ) > 0 ) pNewName = "Bip01_L_Finger31";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandRing3" ) > 0 ) pNewName = "Bip01_L_Finger32";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandRing4" ) > 0 ) pNewName = "Bip01_L_Finger33";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandPinky1" ) > 0 ) pNewName = "Bip01_L_Finger4";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandPinky2" ) > 0 ) pNewName = "Bip01_L_Finger41";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandPinky3" ) > 0 ) pNewName = "Bip01_L_Finger42";
							if ( strstr ( pOldFrameName, "mixamorig_LeftHandPinky4" ) > 0 ) pNewName = "Bip01_L_Finger43";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandThumb1" ) > 0 ) pNewName = "Bip01_R_Finger0";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandThumb2" ) > 0 ) pNewName = "Bip01_R_Finger01";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandThumb3" ) > 0 ) pNewName = "Bip01_R_Finger02";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandThumb4" ) > 0 ) pNewName = "Bip01_R_Finger03";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandIndex1" ) > 0 ) pNewName = "Bip01_R_Finger1";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandIndex2" ) > 0 ) pNewName = "Bip01_R_Finger11";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandIndex3" ) > 0 ) pNewName = "Bip01_R_Finger12";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandIndex4" ) > 0 ) pNewName = "Bip01_R_Finger13";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandMiddle1" ) > 0 ) pNewName = "Bip01_R_Finger2";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandMiddle2" ) > 0 ) pNewName = "Bip01_R_Finger21";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandMiddle3" ) > 0 ) pNewName = "Bip01_R_Finger22";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandMiddle4" ) > 0 ) pNewName = "Bip01_R_Finger23";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandRing1" ) > 0 ) pNewName = "Bip01_R_Finger3";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandRing2" ) > 0 ) pNewName = "Bip01_R_Finger31";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandRing3" ) > 0 ) pNewName = "Bip01_R_Finger32";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandRing4" ) > 0 ) pNewName = "Bip01_R_Finger33";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandPinky1" ) > 0 ) pNewName = "Bip01_R_Finger4";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandPinky2" ) > 0 ) pNewName = "Bip01_R_Finger41";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandPinky3" ) > 0 ) pNewName = "Bip01_R_Finger42";
							if ( strstr ( pOldFrameName, "mixamorig_RightHandPinky4" ) > 0 ) pNewName = "Bip01_R_Finger43";
							if ( strlen(pNewName) == 0 )
							{
								/* should not associate by happenstance, it meses up anim skin deformation if two frames are referenced under name name!
								if ( strstr ( pOldFrameName, "Hips" ) > 0 ) pNewName = "Bip01_Pelvis";
								if ( strstr ( pOldFrameName, "Spine" ) > 0 ) pNewName = "Bip01_Spine";
								if ( strstr ( pOldFrameName, "LeftUpLeg" ) > 0 ) pNewName = "Bip01_L_Thigh";
								if ( strstr ( pOldFrameName, "LeftLeg" ) > 0 ) pNewName = "Bip01_L_Calf";
								if ( strstr ( pOldFrameName, "LeftFoot" ) > 0 ) pNewName = "Bip01_L_Foot";
								if ( strstr ( pOldFrameName, "LeftToeBase" ) > 0 ) pNewName = "Bip01_L_Toe0";
								if ( strstr ( pOldFrameName, "RightUpLeg" ) > 0 ) pNewName = "Bip01_R_Thigh";
								if ( strstr ( pOldFrameName, "RightLeg" ) > 0 ) pNewName = "Bip01_R_Calf";
								if ( strstr ( pOldFrameName, "RightFoot" ) > 0 ) pNewName = "Bip01_R_Foot";
								if ( strstr ( pOldFrameName, "RightToeBase" ) > 0 ) pNewName = "Bip01_R_Toe0";
								if ( strstr ( pOldFrameName, "Spine1" ) > 0 ) pNewName = "Bip01_Spine1";
								if ( strstr ( pOldFrameName, "Spine2" ) > 0 ) pNewName = "Bip01_Spine2";
								if ( strstr ( pOldFrameName, "Neck" ) > 0 ) pNewName = "Bip01_Neck";
								if ( strstr ( pOldFrameName, "LeftShoulder" ) > 0 ) pNewName = "Bip01_L_Clavicle";
								if ( strstr ( pOldFrameName, "RightShoulder" ) > 0 ) pNewName = "Bip01_R_Clavicle";
								if ( strstr ( pOldFrameName, "Head" ) > 0 ) pNewName = "Bip01_Head";
								if ( strstr ( pOldFrameName, "LeftArm" ) > 0 ) pNewName = "Bip01_L_UpperArm";
								if ( strstr ( pOldFrameName, "LeftForeArm" ) > 0 ) pNewName = "Bip01_L_Forearm";
								if ( strstr ( pOldFrameName, "LeftHand" ) > 0 ) pNewName = "Bip01_L_Hand";
								if ( strstr ( pOldFrameName, "RightArm" ) > 0 ) pNewName = "Bip01_R_UpperArm";
								if ( strstr ( pOldFrameName, "RightForeArm" ) > 0 ) pNewName = "Bip01_R_Forearm";
								if ( strstr ( pOldFrameName, "RightHand" ) > 0 ) pNewName = "Bip01_R_Hand";
								if ( strstr ( pOldFrameName, "LeftHandThumb1" ) > 0 ) pNewName = "Bip01_L_Finger0";
								if ( strstr ( pOldFrameName, "LeftHandThumb2" ) > 0 ) pNewName = "Bip01_L_Finger01";
								if ( strstr ( pOldFrameName, "LeftHandThumb3" ) > 0 ) pNewName = "Bip01_L_Finger02";
								if ( strstr ( pOldFrameName, "LeftHandThumb4" ) > 0 ) pNewName = "Bip01_L_Finger03";
								if ( strstr ( pOldFrameName, "LeftHandIndex1" ) > 0 ) pNewName = "Bip01_L_Finger1";
								if ( strstr ( pOldFrameName, "LeftHandIndex2" ) > 0 ) pNewName = "Bip01_L_Finger11";
								if ( strstr ( pOldFrameName, "LeftHandIndex3" ) > 0 ) pNewName = "Bip01_L_Finger12";
								if ( strstr ( pOldFrameName, "LeftHandIndex4" ) > 0 ) pNewName = "Bip01_L_Finger13";
								if ( strstr ( pOldFrameName, "LeftHandMiddle1" ) > 0 ) pNewName = "Bip01_L_Finger2";
								if ( strstr ( pOldFrameName, "LeftHandMiddle2" ) > 0 ) pNewName = "Bip01_L_Finger21";
								if ( strstr ( pOldFrameName, "LeftHandMiddle3" ) > 0 ) pNewName = "Bip01_L_Finger22";
								if ( strstr ( pOldFrameName, "LeftHandMiddle4" ) > 0 ) pNewName = "Bip01_L_Finger23";
								if ( strstr ( pOldFrameName, "LeftHandRing1" ) > 0 ) pNewName = "Bip01_L_Finger3";
								if ( strstr ( pOldFrameName, "LeftHandRing2" ) > 0 ) pNewName = "Bip01_L_Finger31";
								if ( strstr ( pOldFrameName, "LeftHandRing3" ) > 0 ) pNewName = "Bip01_L_Finger32";
								if ( strstr ( pOldFrameName, "LeftHandRing4" ) > 0 ) pNewName = "Bip01_L_Finger33";
								if ( strstr ( pOldFrameName, "LeftHandPinky1" ) > 0 ) pNewName = "Bip01_L_Finger4";
								if ( strstr ( pOldFrameName, "LeftHandPinky2" ) > 0 ) pNewName = "Bip01_L_Finger41";
								if ( strstr ( pOldFrameName, "LeftHandPinky3" ) > 0 ) pNewName = "Bip01_L_Finger42";
								if ( strstr ( pOldFrameName, "LeftHandPinky4" ) > 0 ) pNewName = "Bip01_L_Finger43";
								if ( strstr ( pOldFrameName, "RightHandThumb1" ) > 0 ) pNewName = "Bip01_R_Finger0";
								if ( strstr ( pOldFrameName, "RightHandThumb2" ) > 0 ) pNewName = "Bip01_R_Finger01";
								if ( strstr ( pOldFrameName, "RightHandThumb3" ) > 0 ) pNewName = "Bip01_R_Finger02";
								if ( strstr ( pOldFrameName, "RightHandThumb4" ) > 0 ) pNewName = "Bip01_R_Finger03";
								if ( strstr ( pOldFrameName, "RightHandIndex1" ) > 0 ) pNewName = "Bip01_R_Finger1";
								if ( strstr ( pOldFrameName, "RightHandIndex2" ) > 0 ) pNewName = "Bip01_R_Finger11";
								if ( strstr ( pOldFrameName, "RightHandIndex3" ) > 0 ) pNewName = "Bip01_R_Finger12";
								if ( strstr ( pOldFrameName, "RightHandIndex4" ) > 0 ) pNewName = "Bip01_R_Finger13";
								if ( strstr ( pOldFrameName, "RightHandMiddle1" ) > 0 ) pNewName = "Bip01_R_Finger2";
								if ( strstr ( pOldFrameName, "RightHandMiddle2" ) > 0 ) pNewName = "Bip01_R_Finger21";
								if ( strstr ( pOldFrameName, "RightHandMiddle3" ) > 0 ) pNewName = "Bip01_R_Finger22";
								if ( strstr ( pOldFrameName, "RightHandMiddle4" ) > 0 ) pNewName = "Bip01_R_Finger23";
								if ( strstr ( pOldFrameName, "RightHandRing1" ) > 0 ) pNewName = "Bip01_R_Finger3";
								if ( strstr ( pOldFrameName, "RightHandRing2" ) > 0 ) pNewName = "Bip01_R_Finger31";
								if ( strstr ( pOldFrameName, "RightHandRing3" ) > 0 ) pNewName = "Bip01_R_Finger32";
								if ( strstr ( pOldFrameName, "RightHandRing4" ) > 0 ) pNewName = "Bip01_R_Finger33";
								if ( strstr ( pOldFrameName, "RightHandPinky1" ) > 0 ) pNewName = "Bip01_R_Finger4";
								if ( strstr ( pOldFrameName, "RightHandPinky2" ) > 0 ) pNewName = "Bip01_R_Finger41";
								if ( strstr ( pOldFrameName, "RightHandPinky3" ) > 0 ) pNewName = "Bip01_R_Finger42";
								if ( strstr ( pOldFrameName, "RightHandPinky4" ) > 0 ) pNewName = "Bip01_R_Finger43";
								*/
							}

							// replace all instances of this name (if found)
							if ( strlen(pNewName) > 0 ) 
							{
								// rename animation reference name too
								if ( pObject->pAnimationSet )
								{
									sAnimation* pCurrent = pObject->pAnimationSet->pAnimation;
									while(pCurrent)
									{
										if ( pCurrent->szName )
										{
											if ( stricmp ( pCurrent->szName, pFrame->szName ) == NULL )
											{
												strcpy ( pCurrent->szName, pNewName );
												break;
											}
										}
										pCurrent = pCurrent->pNext;
									}
								}

								// also rename bone names within each mesh
								for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
								{
									sMesh* pMesh = pObject->ppMeshList[iMesh];
									if ( pMesh )
									{
										if ( pMesh->pBones )
										{
											for ( int iBone = 0; iBone < pMesh->dwBoneCount; iBone++ )
											{
												if ( pMesh->pBones [ iBone ].szName )
												{
													if ( stricmp ( pMesh->pBones [ iBone ].szName, pFrame->szName ) == NULL )
													{
														strcpy ( pMesh->pBones [ iBone ].szName, pNewName );
														break;
													}
												}								
											}
										}
									}
								}

								// finally rename frame
								strcpy ( pFrame->szName, pNewName );
							}
							//else
							//	MessageBox ( NULL, "not found for", pOldFrameName, MB_OK );
						}
					}
				}
			}

			// now insert Bip01 into the frame hierarchy between Sceene Root and Pelvis
			for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
			{
				sFrame* pFindPelvisFrame = pObject->ppFrameList[iFrame];
				if ( pFindPelvisFrame )
				{
					LPSTR pFrameName = pFindPelvisFrame->szName;
					if ( pFrameName )
					{
						if ( strlen(pFrameName) > 0 )
						{
							if ( stricmp ( pFrameName, "Bip01_Pelvis" ) == NULL )
							{
								// create new Bip01 frame
								sFrame* pPelvisParent_SceneRoot = pFindPelvisFrame->pParent;
								sFrame* pBip01Frame = new sFrame();

								// insert it between scene root and pelvis
								strcpy ( pBip01Frame->szName, "Bip01" );
								pPelvisParent_SceneRoot->pChild = pBip01Frame;
								pBip01Frame->pParent = pPelvisParent_SceneRoot;
								pFindPelvisFrame->pParent = pBip01Frame;
								pBip01Frame->pChild = pFindPelvisFrame;

								// also create a new Animation object for this new Bip01 frame (so appended anim data can go somewhere (below))
								if ( pObject->pAnimationSet )
								{
									sAnimation* pLastAnim = pObject->pAnimationSet->pAnimation;
									if ( pLastAnim )
									{
										while ( pLastAnim->pNext ) pLastAnim = pLastAnim->pNext;
									}
									if ( pLastAnim )
									{
										sAnimation* pNewAnim = new sAnimation();
										pNewAnim->bLinear = 1;
										pNewAnim->pFrame = pBip01Frame;
										strcpy ( pNewAnim->szName, "Bip01" );
										pLastAnim->pNext = pNewAnim;
									}
								}

								// we are done
								break;
							}
						}
					}
				}
			}

			// now we need to change the model geometry from a T bone to a Y pose (using the relative local transforms we generated above)
			GGMATRIX matrix;
			if ( 1 )
			{
				GGMatrixIdentity ( &matrix );
				UpdateFrame ( pObject->pFrame, &matrix );
				for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
				{
					// wipe out original vertex data
					sMesh* pMesh = pObject->ppMeshList[iMesh];
					if ( pMesh->pOriginalVertexData ) SAFE_DELETE ( pMesh->pOriginalVertexData );

					// animate mesh on CPU
					AnimateBoneMeshBONE ( pObject, NULL, pMesh );

					// now record new vertex mesh shape
					if ( pMesh->pOriginalVertexData ) SAFE_DELETE ( pMesh->pOriginalVertexData );
					DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
					pMesh->pOriginalVertexData = (BYTE*)new char [ dwTotalVertSize ];
					memcpy ( pMesh->pOriginalVertexData, pMesh->pVertexData, dwTotalVertSize );
				}
			}

			// now replace frame transform matrices in imported model (copied from appendanims.x)
			// which zeros the rotation and puts model in Y shape pose (not T) ready for stock 
			// animation data (but retains skinning information for the unique mesh geometry of the model)
			objectnumberforframedatacopy = findFreeObject();
			pAbsPathToUberAnimFile = g.fpscrootdir_s + "\\Files\\entitybank\\Characters\\appendanims.x";
			LoadObject ( pAbsPathToUberAnimFile.Get(), objectnumberforframedatacopy );
			if ( ObjectExist ( objectnumberforframedatacopy ) == 1 )
			{
				sObject* pObjectWithGoodFrameData = GetObjectData ( objectnumberforframedatacopy );
				if ( pObjectWithGoodFrameData )
				{
					// go through all frames of imported object, match up name and copy frame transform matrix over from 'framegood' model
					for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
					{
						sFrame* pFrame = pObject->ppFrameList[iFrame];
						if ( pFrame )
						{
							LPSTR pFrameName = pFrame->szName;
							if ( pFrameName )
							{
								if ( strlen(pFrameName) > 0 )
								{
									// check to find this name in framegood model
									for ( int iFrameGood = 0; iFrameGood < pObjectWithGoodFrameData->iFrameCount; iFrameGood++ )
									{
										sFrame* pFrameGood = pObjectWithGoodFrameData->ppFrameList[iFrameGood];
										if ( pFrameGood )
										{
											LPSTR pFrameGoodName = pFrameGood->szName;
											if ( pFrameGoodName )
											{
												if ( strlen(pFrameGoodName) > 0 )
												{
													// check if we have a match
													if ( stricmp ( pFrameName, pFrameGoodName ) == NULL )
													{
														// now copy the frame data to the import model
														pFrame->matOriginal = pFrameGood->matOriginal;
														pFrame->matTransformed = pFrameGood->matTransformed;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				DeleteObject ( objectnumberforframedatacopy );
			}

			// overwrite bone[].translate matrix with difference between new pose (Y) and old pose (T) (for AnimateBoneMeshBONE)
			/*
			if ( pStoreNewPoseFrames )
			{
				for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
				{
					sMesh* pMesh = pObject->ppMeshList[iMesh];
					for ( DWORD dwBoneMatrixIndex = 0; dwBoneMatrixIndex < pMesh->dwBoneCount; dwBoneMatrixIndex++ )
					{
						// find frame related to this bone
						LPSTR pBoneName = pMesh->pBones [ dwBoneMatrixIndex ].szName;
						if ( pBoneName )
						{
							if ( strlen ( pBoneName ) > 0 )
							{
								for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
								{
									sFrame* pFrame = pObject->ppFrameList[iFrame];
									LPSTR pFrameName = pFrame->szName;
									if ( pFrameName )
									{
										if ( strlen ( pFrameName ) > 0 )
										{
											if ( stricmp ( pBoneName, pFrameName ) == NULL )
											{
												pFrame->matTransformed = pStoreNewPoseFrames[iFrame];
												break;			
											}
										}
									}
								}
							}
						}
					}
				}
				delete pStoreNewPoseFrames;
			}
			*/

			/* moved up to just after new transforms created
			// now we need to change the model geometry from a T bone to a Y pose
			GGMATRIX matrix;
			if ( 1 )
			{
				GGMatrixIdentity ( &matrix );
				UpdateFrame ( pObject->pFrame, &matrix );
				for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
				{
					// wipe out original vertex data
					sMesh* pMesh = pObject->ppMeshList[iMesh];
					if ( pMesh->pOriginalVertexData ) SAFE_DELETE ( pMesh->pOriginalVertexData );

					// animate mesh on CPU
					AnimateBoneMeshBONE ( pObject, NULL, pMesh );

					// now record new vertex mesh shape
					if ( pMesh->pOriginalVertexData ) SAFE_DELETE ( pMesh->pOriginalVertexData );
					DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
					pMesh->pOriginalVertexData = (BYTE*)new char [ dwTotalVertSize ];
					memcpy ( pMesh->pOriginalVertexData, pMesh->pVertexData, dwTotalVertSize );
				}
			}
			*/

			// and restore transformed matrix from originals
			for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
			{
				sFrame* pFrame = pObject->ppFrameList[iFrame];
				if ( pFrame )
				{
					pFrame->matTransformed = pFrame->matOriginal;
				}
			}

			// delete all keys so append can be fresh (and specifically remove matrix keys which WILL mess up overall animation)
			if ( pObject->pAnimationSet )
			{
				sAnimationSet* pAnimSet = pObject->pAnimationSet;
				while ( pAnimSet != NULL )
				{
					sAnimation* pAnim = pAnimSet->pAnimation;
					while ( pAnim != NULL )
					{
						// scans all animation data and creates the interpolation vectors between all keyframes (vital)
						if ( pAnim )
						{
							SAFE_DELETE(pAnim->pPositionKeys);
							pAnim->dwNumPositionKeys=0;
							SAFE_DELETE(pAnim->pRotateKeys);
							pAnim->dwNumRotateKeys=0;
							SAFE_DELETE(pAnim->pScaleKeys);
							pAnim->dwNumScaleKeys=0;
							SAFE_DELETE(pAnim->pMatrixKeys);
							pAnim->dwNumMatrixKeys=0;
						}
						pAnim = pAnim->pNext;
					}
					pAnimSet = pAnimSet->pNext;
				}
			}

			// append uber animations to character
			AppendAnimationFromFile ( pObject, pAbsPathToUberAnimFile.Get(), 0 );

			// now update model to first frame animation pose (to calculate combined matrix from pose)
			GGMatrixIdentity ( &matrix );
			UpdateAllFrameData ( pObject, 0.0f );
			UpdateFrame ( pObject->pFrame, &matrix );

			// and then calculate the inverse of those matCombined transforms to apply to the bone matTransforms
			// so bone matrix cancels out first frame pose leaving matCombined animations to shape shader verts
			for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iMesh];
				if ( pMesh )
				{
					sBone* pBones = pMesh->pBones;
					if ( pBones )
					{
						for ( int iBone = 0; iBone < pMesh->dwBoneCount; iBone++ )
						{
							float fDet = 0.0f;
							GGMatrixInverse ( &pBones[iBone].matTranslation, &fDet, pMesh->pFrameMatrices [ iBone ] );
						}
					}
				}
			}
		}
	}

	// Save Object
	if ( strcmp ( Lower(Right(t.tSaveFile_s.Get(),4)) , ".dbo" ) == 0 ) 
	{
		if ( FileExist (t.tSaveFile_s.Get()) == 1 ) DeleteAFile ( t.tSaveFile_s.Get() ) ;
		ScaleObject (  t.importer.objectnumber,100,100,100 );
		SaveObject (  t.tSaveFile_s.Get(),t.importer.objectnumber );
		ScaleObject (  t.importer.objectnumber,t.importer.objectScaleForEditing,t.importer.objectScaleForEditing,t.importer.objectScaleForEditing );
	}
	else
	{
		if (  t.importer.objectFileOriginalPath+t.importer.objectFilename  !=  t.tSavePath_s+t.importer.objectFilename ) 
		{
			t.strwork = ""; t.strwork = t.strwork +t.importer.objectFileOriginalPath+t.importer.objectFilename;
			if (  FileExist( t.strwork.Get() ) == 1 ) 
			{
				t.strwork = ""; t.strwork = t.strwork +t.tSavePath_s+t.importer.objectFilename;
				if (  FileExist( t.strwork.Get() ) == 0 ) 
				{
					t.strwork = ""; t.strwork = t.strwork +t.importer.objectFileOriginalPath+t.importer.objectFilename;
					cstr string1 = t.tSavePath_s+t.importer.objectFilename;
					CopyAFile ( t.strwork.Get() , string1.Get() );
				}
			}
		}
	}
	UnDim (  t.tArray );

	Dim ( t.tArray2, 220 );
	for ( tCount = 1; tCount <= IMPORTERTEXTURESMAX; tCount++ )
	{
		// skip files that do not exist
		if ( t.importerTextures[tCount].imageID == 0 )  
			continue;

		//  Split the filename into tokens to grab the path
		t.tSourceName_s = t.importerTextures[tCount].fileName;
		t.tArrayMarker = 0;
		t.tstring_s=t.tSourceName_s;
		t.tToken_s=FirstToken(t.tstring_s.Get(),"\\");
		if (  t.tToken_s  !=  "" ) 
		{
			t.tArray2[t.tArrayMarker] = t.tToken_s;
			++t.tArrayMarker;
		}
		do
		{
			t.tToken_s=NextToken("\\");
			if (  t.tToken_s  !=  "" ) 
			{
				t.tArray2[t.tArrayMarker] = t.tToken_s;
				++t.tArrayMarker;
			}
		} while ( !(  t.tToken_s == "" ) );

		//  Now store just the file name
		t.tDestFileName = t.tSavePath_s + t.tArray2[t.tArrayMarker-1];
		if ( tCount == 1 )
		{
			// first texture in list is the 'color/diffuse' one (even if multitexture)
			t.importer.objectFPE.textured = t.tArray2[t.tArrayMarker-1];
		}

		//  Copy and save images to destination
		if (  ImageExist(t.importerTextures[tCount].imageID) == 1 ) 
		{
			cstr test1 = ""; test1 = test1 + g.fpscrootdir_s + "\\Files\\" + t.importerTextures[tCount].fileName;
			if (  FileExist (t.importerTextures[tCount].fileName.Get())  ==  1 && t.tDestFileName  !=  test1 && t.tDestFileName  !=  t.importerTextures[tCount].fileName ) 
			{
				if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
				{
					CopyAFile (  t.importerTextures[tCount].fileName.Get(),t.tDestFileName.Get() );
				}
			}
			else
			{
				t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.importerTextures[tCount].fileName;
				cstr test2 = ""; test2 = test2 + g.fpscrootdir_s + "\\Files\\" + t.importerTextures[tCount].fileName;
				if (  FileExist ( t.strwork.Get() )  ==  1 && t.tDestFileName  !=  test2 ) 
				{
					if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
					{
						t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.importerTextures[tCount].fileName;
						CopyAFile (  t.strwork.Get() ,t.tDestFileName.Get() );
					}
				}
				else
				{
					t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.importerTextures[tCount].fileName;
					if (  t.tDestFileName  !=  t.strwork ) 
					{
						if (  ImageExist (t.importerTextures[tCount].imageID) ) 
						{
							if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
							{
								SaveImage (  t.tDestFileName.Get() , t.importerTextures[tCount].imageID );
							}
						}
					}
				}
			}
		}

		//  Check for Normal and Specular and copy those if they exist
		if (  Len(t.importerTextures[tCount].fileName.Get()) > 6 ) 
		{
			if ( strcmp ( Lower(Right(t.importerTextures[tCount].fileName.Get(),6)) , "_d.dds" ) == 0 || strcmp ( Lower(Right(t.importerTextures[tCount].fileName.Get(),6)) , "_d.png" ) == 0 ) 
			{
				//  Normal map first
				t.tnormalmapfile_s = ""; t.tnormalmapfile_s = t.tnormalmapfile_s + Left(t.importerTextures[tCount].fileName.Get(),Len(t.importerTextures[tCount].fileName.Get())-6) + "_N.dds";
				t.tDestFileName = t.tSavePath_s + t.tArray2[t.tArrayMarker-1];
				cstr TempFileName = t.tDestFileName;
				t.tDestFileName = "" ; t.tDestFileName = t.tDestFileName + Left(TempFileName.Get(),Len(TempFileName.Get())-6) + "_N.dds";
				t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
				if (  FileExist (t.tnormalmapfile_s.Get())  ==  1 && t.tDestFileName  !=  t.strwork && t.tDestFileName  !=  t.tnormalmapfile_s )
				{
					if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
					{
						CopyAFile (  t.tnormalmapfile_s.Get(),t.tDestFileName.Get() );
					}
				}
				else
				{
					t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					if (  FileExist ( t.strwork.Get() )  ==  1 && t.tDestFileName  !=  g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s ) 
					{
						if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
						{
							t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
							CopyAFile ( t.strwork.Get() ,t.tDestFileName.Get() );
						}
					}
				}

				//  Normal map continued
				t.tnormalmapfile_s = ""; t.tnormalmapfile_s = t.tnormalmapfile_s + Left(t.importerTextures[tCount].fileName.Get(),Len(t.importerTextures[tCount].fileName.Get())-6) + "_N.png";
				t.tDestFileName = t.tSavePath_s + t.tArray2[t.tArrayMarker-1];
				TempFileName = t.tDestFileName;
				t.tDestFileName = "" ; t.tDestFileName = t.tDestFileName + Left(TempFileName.Get(),Len(TempFileName.Get())-6) + "_N.png";
				t.strwork = ""; t.strwork = t.strwork + "\\Files\\" + t.tnormalmapfile_s;
				if (  FileExist (t.tnormalmapfile_s.Get())  ==  1 && t.tDestFileName  !=  g.fpscrootdir_s + t.strwork && t.tDestFileName  !=  t.tnormalmapfile_s )  
				{
					if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
					{
						CopyAFile (  t.tnormalmapfile_s.Get() ,t.tDestFileName.Get() );
					}
				}
				else
				{

					cstr work1 = ""; work1 = work1 + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					if (  FileExist ( work1.Get() )  ==  1 && t.tDestFileName  !=  t.strwork )  
					{
						if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
						{
							t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
							CopyAFile (  t.strwork.Get() , t.tDestFileName.Get() );
						}
					}
				}

				//  Specular now
				t.tnormalmapfile_s = "" ; t.tnormalmapfile_s = t.tnormalmapfile_s + Left(t.importerTextures[tCount].fileName.Get(),Len(t.importerTextures[tCount].fileName.Get())-6) + "_S.dds";
				t.tDestFileName = t.tSavePath_s + t.tArray2[t.tArrayMarker-1];
				TempFileName = t.tDestFileName;
				t.tDestFileName = "" ; t.tDestFileName = t.tDestFileName + Left (TempFileName.Get(),Len(TempFileName.Get())-6) + "_S.dds";
				t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
				if (  FileExist (t.tnormalmapfile_s.Get())  ==  1 && t.tDestFileName  !=  t.strwork && t.tDestFileName  !=  t.tnormalmapfile_s ) 
				{
					if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
					{
						CopyAFile (  t.tnormalmapfile_s.Get() , t.tDestFileName.Get() );
					}
				}
				else
				{
					cstr work2 = ""; work2 = work2 + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					if (  FileExist ( work2.Get() )  ==  1 && t.tDestFileName  !=  t.strwork ) 
					{
						if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
						{
							t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
							CopyAFile (  t.strwork.Get() ,t.tDestFileName.Get() );
						}
					}
				}

				t.tnormalmapfile_s = ""; t.tnormalmapfile_s = t.tnormalmapfile_s + Left(t.importerTextures[tCount].fileName.Get(),Len(t.importerTextures[tCount].fileName.Get())-6) + "_S.png";
				t.tDestFileName = t.tSavePath_s + t.tArray2[t.tArrayMarker-1];
				TempFileName = t.tDestFileName;
				t.tDestFileName = ""; t.tDestFileName = t.tDestFileName + Left(TempFileName.Get(),Len(TempFileName.Get())-6) + "_S.png";
				t.strwork = ""; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
				if (  FileExist (t.tnormalmapfile_s.Get())  ==  1 && t.tDestFileName  !=  t.strwork && t.tDestFileName  !=  t.tnormalmapfile_s ) 
				{
					if (  FileExist(t.tDestFileName.Get())  ==  0 ) 
					{
						CopyAFile (  t.tnormalmapfile_s.Get(),t.tDestFileName.Get() );
					}
				}
				else
				{
					cstr work3 = ""; work3 = work3 + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
					if (  FileExist ( work3.Get() )  ==  1 && t.tDestFileName  !=  t.strwork ) 
					{
						if (  FileExist(t.tDestFileName.Get() )  ==  0 ) 
						{
							t.strwork = "" ; t.strwork = t.strwork + g.fpscrootdir_s + "\\Files\\" + t.tnormalmapfile_s;
							CopyAFile (  t.strwork.Get() ,t.tDestFileName.Get() );
						}
					}
				}
			}
		}
	}
	UnDim (  t.tArray2 );

	// If model has MORE than one texture, blank out FPE textured
	if ( t.tcounttextures > 1 ) 
	{
		// but make sure they are 'different' textures, not just DNS/PBR sets
		LPSTR pFile = t.importerTextures[1].fileName.Get();
		LPSTR pExt = "_d.png";
		if ( strnicmp ( pFile+strlen(pFile)-10, "_color.png", 10 ) == NULL ) pExt = "_color.png";
		if ( strnicmp ( pFile+strlen(pFile)-12, "_diffuse.png", 12 ) == NULL ) pExt = "_diffuse.png";
		if ( strnicmp ( pFile+strlen(pFile)-11, "_albedo.png", 11 ) == NULL ) pExt = "_albedo.png";
		if ( strnicmp ( pFile+strlen(pFile)-10, "_color.dds", 10 ) == NULL ) pExt = "_color.dds";
		if ( strnicmp ( pFile+strlen(pFile)-12, "_diffuse.dds", 12 ) == NULL ) pExt = "_diffuse.dds";
		if ( strnicmp ( pFile+strlen(pFile)-11, "_albedo.dds", 11 ) == NULL ) pExt = "_albedo.dds";
		cstr pBaseFilePart = Left(pFile,Len(pFile)-strlen(pExt));
		for ( int tCount = 2; tCount <= 10; tCount++ )
		{
			LPSTR pCompareFile = t.importerTextures[tCount].fileName.Get();
			if ( strlen ( pCompareFile ) > 4 )
			{
				if ( strnicmp ( pCompareFile, pBaseFilePart.Get(), strlen(pBaseFilePart.Get()) ) == NULL )
				{
					// matches base part, now exclude files of known extension
					bool bExclude = false;
					char pRest[1024];
					strcpy ( pRest, pCompareFile + strlen(pBaseFilePart.Get()) );
					pRest[strlen(pRest)-4] = 0;
					if ( stricmp ( pRest, "_normal" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_specular" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_metalness" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_gloss" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_ao" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_height" ) == NULL ) bExclude = true;
					if ( stricmp ( pRest, "_cube" ) == NULL ) bExclude = true;
					if ( bExclude == false )
					{
						// found a match to base texture, but unknown extra part
						t.importer.objectFPE.textured="";
						break;
					}
				}
				else
				{
					// found a different base texture, cannot be a single textured model
					t.importer.objectFPE.textured="";
					break;
				}
			}
		}
	}

	// save FPE file
	importer_save_fpe ( );

	// Save/Generate Thumbnail BMP
	t.strwork = ""; t.strwork = t.strwork + t.importer.objectFileOriginalPath + t.importer.objectFilename;
	cstr pSourceBMP = cstr ( cstr(Left ( t.strwork.Get(), strlen(t.strwork.Get())-4)) + ".bmp" );
	if ( FileExist ( pSourceBMP.Get() ) ) 
	{
		// if BMP already existed in source area, use that one
		if ( FileExist ( t.tSaveThumb_s.Get()) == 1 ) DeleteAFile ( t.tSaveThumb_s.Get() );
		CopyFile ( pSourceBMP.Get(), t.tSaveThumb_s.Get(), FALSE );
	}
	else
	{
		// generate new BMP thumb from scratch
		CreateBitmap ( 32, 64, 64 );
		SetCurrentBitmap ( 32 );
		SetCameraAspect ( 1.0 );

		//  Make a white background Box (  )
		t.twhiteobj = 1;
		t.tfound = 0;

		//  Find a free object number
		while (  t.tfound  ==  0 ) 
		{
			if (  ObjectExist(t.twhiteobj)  ==  1 ) 
			{
				++t.twhiteobj;
			}
			else
			{
				t.tfound = 1;
			}
		}

		MakeObjectBox (  t.twhiteobj,200000,200000,1 );
		SetObjectLight (  t.twhiteobj,0 );
		LockObjectOn (  t.twhiteobj );
		PositionObject (  t.twhiteobj,0,0,1000 );
		SetObjectEffect ( t.twhiteobj, g.guishadereffectindex );
		SetObjectEmissive ( t.twhiteobj, Rgb(255,255,255) );

		t.tOldSelected = t.importerTabs[1].selected;

		HideSprite (  t.importer.helpSprite2 );
		HideSprite (  t.importer.helpSprite3 );
		if (  SpriteExist(t.importer.helpSprite4)  )  DeleteSprite (  t.importer.helpSprite4 );

		t.importerTabs[1].selected = 1;
		importer_update_selection_markers ( );
		RotateObject (  t.importerGridObject[8], 0,0,0 );
		PositionObject (  t.importerGridObject[8],0,0,IMPORTERZPOSITION );
		RotateObject (  t.importerGridObject[8], t.slidersmenuvalue[t.importer.properties3Index][1].value, t.slidersmenuvalue[t.importer.properties3Index][2].value, t.slidersmenuvalue[t.importer.properties3Index][3].value );
		SetAlphaMappingOn (  t.importer.objectnumber , 255 );
		HideObject (  t.importer.dummyCharacterObjectNumber );
		t.importerTabs[1].selected = t.tOldSelected;
		for ( t.tc = 1 ; t.tc<=  7; t.tc++ )
		{
			HideObject (  t.importerGridObject[t.tc] );
		}
		bool b6002 = false; if ( ObjectExist(6002)==1) { b6002 = GetVisible(6002); HideObject(6002); }
		bool b6003 = false; if ( ObjectExist(6003)==1) { b6003 = GetVisible(6003); HideObject(6003); }
		Sync();
		if ( b6002 == true ) ShowObject ( 6002 );
		if ( b6003 == true ) ShowObject ( 6003 );
		RotateObject (  t.importerGridObject[8], 0,0,0 );
		for ( t.tc = 1 ; t.tc<=  7; t.tc++ )
		{
			ShowObject ( t.importerGridObject[t.tc] );
		}
		if ( FileExist (t.tSaveThumb_s.Get())  ==  1  )  DeleteAFile (  t.tSaveThumb_s.Get() ) ;
		GrabImage ( g.importermenuimageoffset+50, 0, 0, 64, 64, 3 );
		t.tSprite = 100;
		while (  SpriteExist (t.tSprite) ) 
		{
			++t.tSprite;
		}
		SaveImage ( t.tSaveThumb_s.Get(), g.importermenuimageoffset+50 );
		DeleteImage ( g.importermenuimageoffset+50 );
		PositionObject ( t.importerGridObject[8],0,0,IMPORTERZPOSITION );
		DeleteObject ( t.twhiteobj );
	}

	t.aspect_f=GetDesktopWidth() ; t.aspect_f=t.aspect_f/GetDesktopHeight();
	SetCameraAspect (  t.aspect_f );
	SetCurrentBitmap (  0 );
	importer_update_selection_markers ( );
	importer_draw ( );
	importer_tabs_draw ( );
	Sync (  );
	t.importer.cancel = 1;
	t.timportersaveon = 0;
}

void importer_tabs_update ( void )
{
	//  unselect buttons after a time
	if (  t.importer.buttonPressedCount > 0 ) 
	{
		--t.importer.buttonPressedCount;
		if (  t.importer.buttonPressedCount  <= 0 ) 
		{
			t.importerTabs[12].selected = 0;
			for ( int tCount = 5 ; tCount<=  9; tCount++ )
			{
				t.importerTabs[tCount].selected = 0;
			}
		}
	}

	if (  t.importer.oldMouseClick  ==  0 && t.inputsys.mclick  ==  1 ) 
	{
		for ( t.tCount5 = 1 ; t.tCount5 <= 12; t.tCount5++ )
		{
			if (  t.importer.MouseX  >=  t.importerTabs[t.tCount5].x && t.importer.MouseX  <=  t.importerTabs[t.tCount5].x + 128 ) 
			{
				if (  t.importer.MouseY  >=  t.importerTabs[t.tCount5].y && t.importer.MouseY  <=  t.importerTabs[t.tCount5].y + 32 ) 
				{
					if (  t.tCount5 < 5 ) 
					{

						for ( t.tCount2 = 1 ; t.tCount2<=  3; t.tCount2++ )
						{
							if (  t.tCount5  <=  3 ) 
							{
								if (  t.tCount2  ==  t.tCount5  )  t.importerTabs[t.tCount2].selected  =  1; else t.importerTabs[t.tCount2].selected  =  0;
							}
						}
						if (  t.tCount5  <= 3 ) 
						{
							g.tabmode = t.importerTabs[t.tCount5].tabpage;
							RotateObject (  t.importer.objectnumber,0,0,0 );
						}

					}
					else
					{

						//  check for save entity pressed
						if (  t.tCount5  ==  5 ) 
						{
							t.importerTabs[5].selected = 1;
						}

						//  cancel
						if (  t.tCount5  ==  11 ) 
						{
							t.importerTabs[11].selected = 1;
							t.importer.cancel = 1;
							t.importer.cancelCount = 10;
						}
						//  tab 1 buttons
						if (  t.tCount5  ==  10 && t.importerTabs[1].selected  ==  1 ) 
						{
							t.importerTabs[t.tCount5].selected = 1 - t.importerTabs[t.tCount5].selected;
						}

						//  tab 2 buttons
						if (  t.tCount5 > 5 && t.importerTabs[2].selected  ==  1 ) 
						{
							t.importerTabs[t.tCount5].selected = 1;
							t.importer.buttonPressedCount = 20;

							//  add new collision Box (  )
							if (  t.tCount5  ==  6 ) 
							{
								importer_add_collision_box ( );
							}

							//  delete collision Box (  )
							if (  t.tCount5  ==  7 ) 
							{
								importer_delete_collision_box ( );
							}

							//  next collision Box (  )
							if (  t.tCount5  ==  8 ) 
							{
								++t.importer.selectedCollisionObject;
								if (  t.importer.selectedCollisionObject  >=  t.importer.collisionShapeCount ) 
								{
									t.importer.selectedCollisionObject = 0;
								}
							}

							//  previous collision Box (  )
							if (  t.tCount5  ==  9 ) 
							{
								--t.importer.selectedCollisionObject;
								if (  t.importer.selectedCollisionObject < 0 ) 
								{
									t.importer.selectedCollisionObject = t.importer.collisionShapeCount-1;
								}
							}

							//  add new collision Box (  )
							if (  t.tCount5  ==  12 ) 
							{
								importer_dupe_collision_box ( );
							}

						}
					}
				}
			}
		}
	}

	if ( t.importerTabs[10].selected == 1 ) 
	{
		t.importerTabs[10].label = "Turn Guide Off";
	}
	else
	{
		t.importerTabs[10].label = "Turn Guide On";
	}
}

void importer_tabs_draw ( void )
{
	// if image exist, importer active
	if ( t.importer.importerActive == 1 )
	{
		for ( int tCount = 1 ; tCount<=  12; tCount++ )
		{
			if (  tCount < 4 || tCount  >=  5 ) 
			{
				//  dont draw the collsion buttons if the tab isnt active
				t.skip = 0;
				if (  tCount > 5 && tCount < 10 && t.importerTabs[2].selected  ==  0  )  t.skip  =  1;
				if (  tCount  ==  12 && t.importerTabs[2].selected  ==  0  )  t.skip  =  1;
				if (  tCount  ==  10 && t.importerTabs[1].selected  ==  0  )  t.skip  =  1;

				// going to draw button
				if (  t.skip  ==  0 ) 
				{
					if (  t.importerTabs[tCount].selected  ==  1 ) 
					{
						PasteImage (  g.importermenuimageoffset+5 , t.importerTabs[tCount].x , t.importerTabs[tCount].y );
					}
					else
					{
						PasteImage (  g.importermenuimageoffset+4 , t.importerTabs[tCount].x , t.importerTabs[tCount].y );
					}
					if (  tCount > 5 && tCount < 11 || tCount  ==  12 ) 
					{
						pastebitmapfont(t.importerTabs[tCount].label.Get(),t.importerTabs[tCount].x + 64 - ((getbitmapfontwidth(t.importerTabs[tCount].label.Get(),2)) / 2) ,t.importerTabs[tCount].y + 9,2,255);
					}
					else
					{
						pastebitmapfont(t.importerTabs[tCount].label.Get(),t.importerTabs[tCount].x + 64 - ((getbitmapfontwidth(t.importerTabs[tCount].label.Get(),1)) / 2) ,t.importerTabs[tCount].y,1,255);
					}
				}
			}
		}
	}
}

void importer_add_collision_box ( void )
{
	t.tScale = t.importer.objectScaleForEditing;
	if (  t.importer.collisionShapeCount < 49 ) 
	{
		t.importerCollision[t.importer.collisionShapeCount].object = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object , 100 , 100 , 100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object, 0, t.tObjectSizeX_f , t.tObjectSizeY_f, t.tObjectSizeZ_f );
		t.importerCollision[t.importer.collisionShapeCount].sizex = t.tObjectSizeX_f;
		t.importerCollision[t.importer.collisionShapeCount].sizey = t.tObjectSizeY_f;
		t.importerCollision[t.importer.collisionShapeCount].sizez = t.tObjectSizeZ_f;
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object , t.tBoxOffsetLeft_f , t.tBoxOffsetTop_f, t.tBoxOffsetFront_f );
		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = 0;
		t.importerCollision[t.importer.collisionShapeCount].roty = 0;
		t.importerCollision[t.importer.collisionShapeCount].rotz = 0;
		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object, g.guishadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object, Rgb(255,255,255) );
		SetAlphaMappingOn ( t.importerCollision[t.importer.collisionShapeCount].object, 10 );

		t.importerCollision[t.importer.collisionShapeCount].object2 = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object2 , 100,100,100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object2, 0, t.tObjectSizeX_f , t.tObjectSizeY_f, t.tObjectSizeZ_f );
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object2 , t.tBoxOffsetLeft_f , t.tBoxOffsetTop_f, t.tBoxOffsetFront_f );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );
		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = 0;
		t.importerCollision[t.importer.collisionShapeCount].roty = 0;
		t.importerCollision[t.importer.collisionShapeCount].rotz = 0;
		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object2,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		SetObjectWireframe (  t.importerCollision[t.importer.collisionShapeCount].object2 , 1 );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object2, g.guiwireframeshadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object2, Rgb(255,255,255) );

		PositionObject (  t.importerGridObject[9], 0 , 0 , IMPORTERZPOSITION );

		t.importer.selectedCollisionObject = t.importer.collisionShapeCount;
		t.slidersmenuvalue[t.importer.properties2Index][7].value = 0;
		t.slidersmenuvalue[t.importer.properties2Index][8].value = 0;
		t.slidersmenuvalue[t.importer.properties2Index][9].value = 0;
		++t.importer.collisionShapeCount;
	}
}

void importer_dupe_collision_box ( void )
{
	t.tScale = t.importer.objectScaleForEditing;
	if (  t.importer.selectedCollisionObject  ==  -1  )  return;
	if (  t.importer.collisionShapeCount < 49 ) 
	{
		t.importerCollision[t.importer.collisionShapeCount].object = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object , 100 , 100 , 100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object, 0, LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object,0) ,  LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object,0),  LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object,0) );
		t.importerCollision[t.importer.collisionShapeCount].sizex = LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object,0);
		t.importerCollision[t.importer.collisionShapeCount].sizey = LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object,0);
		t.importerCollision[t.importer.collisionShapeCount].sizez = LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object,0);
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object , ObjectPositionX(t.importerCollision[t.importer.selectedCollisionObject].object) ,  ObjectPositionY(t.importerCollision[t.importer.selectedCollisionObject].object),  ObjectPositionZ(t.importerCollision[t.importer.selectedCollisionObject].object) );
		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = ObjectAngleX(t.importerCollision[t.importer.selectedCollisionObject].object2);
		t.importerCollision[t.importer.collisionShapeCount].roty = ObjectAngleY(t.importerCollision[t.importer.selectedCollisionObject].object2);
		t.importerCollision[t.importer.collisionShapeCount].rotz = ObjectAngleZ(t.importerCollision[t.importer.selectedCollisionObject].object2);
		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object, g.guishadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object, Rgb(255,255,255) );
		SetAlphaMappingOn ( t.importerCollision[t.importer.collisionShapeCount].object, 10 );

		t.importerCollision[t.importer.collisionShapeCount].object2 = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object2 , 100,100,100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object2, 0, LimbScaleX(t.importerCollision[t.importer.selectedCollisionObject].object,0) ,  LimbScaleY(t.importerCollision[t.importer.selectedCollisionObject].object,0),  LimbScaleZ(t.importerCollision[t.importer.selectedCollisionObject].object,0) );
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object2 , ObjectPositionX(t.importerCollision[t.importer.selectedCollisionObject].object) ,  ObjectPositionY(t.importerCollision[t.importer.selectedCollisionObject].object),  ObjectPositionZ(t.importerCollision[t.importer.selectedCollisionObject].object) );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );

		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = ObjectAngleX(t.importerCollision[t.importer.selectedCollisionObject].object2);
		t.importerCollision[t.importer.collisionShapeCount].roty = ObjectAngleY(t.importerCollision[t.importer.selectedCollisionObject].object2);
		t.importerCollision[t.importer.collisionShapeCount].rotz = ObjectAngleZ(t.importerCollision[t.importer.selectedCollisionObject].object2);
		RotateObject (   t.importerCollision[t.importer.collisionShapeCount].object, ObjectAngleX(t.importerCollision[t.importer.selectedCollisionObject].object2),ObjectAngleY(t.importerCollision[t.importer.selectedCollisionObject].object2),ObjectAngleZ(t.importerCollision[t.importer.selectedCollisionObject].object2) );
		RotateObject (   t.importerCollision[t.importer.collisionShapeCount].object2, ObjectAngleX(t.importerCollision[t.importer.selectedCollisionObject].object2),ObjectAngleY(t.importerCollision[t.importer.selectedCollisionObject].object2),ObjectAngleZ(t.importerCollision[t.importer.selectedCollisionObject].object2) );
		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object2,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		SetObjectWireframe (  t.importerCollision[t.importer.collisionShapeCount].object2 , 1 );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object2, g.guiwireframeshadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object2, Rgb(255,255,255) );

		PositionObject (  t.importerGridObject[9], 0 , 0 , IMPORTERZPOSITION );
		t.importer.selectedCollisionObject = t.importer.collisionShapeCount;
		t.slidersmenuvalue[t.importer.properties2Index][7].value = t.importerCollision[t.importer.collisionShapeCount].rotx;
		t.slidersmenuvalue[t.importer.properties2Index][8].value = t.importerCollision[t.importer.collisionShapeCount].roty;
		t.slidersmenuvalue[t.importer.properties2Index][9].value = t.importerCollision[t.importer.collisionShapeCount].rotz;
		++t.importer.collisionShapeCount;
	}
}

void importer_add_collision_box_loaded ( void )
{
	if (  t.importer.collisionShapeCount < 49 ) 
	{
		t.importerCollision[t.importer.collisionShapeCount].object = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object , 100 , 100 , 100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object, 0, t.tPSizeX_f * (t.tScale / 100.0) , t.tPSizeY_f * (t.tScale / 100.0) , t.tPSizeZ_f * (t.tScale / 100.0) );
		t.importerCollision[t.importer.collisionShapeCount].sizex = t.tPSizeX_f * (t.tScale / 100.0);
		t.importerCollision[t.importer.collisionShapeCount].sizey = t.tPSizeY_f * (t.tScale / 100.0);
		t.importerCollision[t.importer.collisionShapeCount].sizez = t.tPSizeZ_f * (t.tScale / 100.0);
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object, t.tPOffX_f * (t.tScale / 100.0) , (t.tPOffY_f * (t.tScale / 100.0)) , t.tPOffZ_f * (t.tScale / 100.0) );
		RotateObject (  t.importerCollision[t.importer.collisionShapeCount].object, t.tPRotX_f , t.tPRotY_f , t.tPRotZ_f );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object, g.guishadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object, Rgb(255,255,255) );
		SetAlphaMappingOn ( t.importerCollision[t.importer.collisionShapeCount].object, 10 );

		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = t.tPRotX_f;
		t.importerCollision[t.importer.collisionShapeCount].roty = t.tPRotY_f;
		t.importerCollision[t.importer.collisionShapeCount].rotz = t.tPRotZ_f;

		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object );
		t.importerCollision[t.importer.collisionShapeCount].object2 = findFreeObject();
		MakeObjectBox (  t.importerCollision[t.importer.collisionShapeCount].object2 , 100,100,100 );
		ScaleLimb (  t.importerCollision[t.importer.collisionShapeCount].object2, 0, t.tPSizeX_f * (t.tScale / 100.0) , t.tPSizeY_f * (t.tScale / 100.0) , t.tPSizeZ_f * (t.tScale / 100.0) );
		LockObjectOn (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		PositionObject (  t.importerCollision[t.importer.collisionShapeCount].object2, t.tPOffX_f * (t.tScale / 100.0) , (t.tPOffY_f * (t.tScale / 100.0)) , t.tPOffZ_f * (t.tScale / 100.0) );
		RotateObject (  t.importerCollision[t.importer.collisionShapeCount].object2, t.tPRotX_f , t.tPRotY_f , t.tPRotZ_f );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object,0 );
		SetObjectCull (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );
		SetObjectEffect ( t.importerCollision[t.importer.collisionShapeCount].object2, g.guiwireframeshadereffectindex );
		SetObjectEmissive ( t.importerCollision[t.importer.collisionShapeCount].object2, Rgb(255,255,255) );

		PositionObject (  t.importerGridObject[9], 0 , 0 , 0 );
		t.importerCollision[t.importer.collisionShapeCount].rotx = t.tPRotX_f;
		t.importerCollision[t.importer.collisionShapeCount].roty = t.tPRotY_f;
		t.importerCollision[t.importer.collisionShapeCount].rotz = t.tPRotZ_f;
		GlueObjectToLimbEx (  t.importerCollision[t.importer.collisionShapeCount].object2,t.importerGridObject[9], 0 , 1 );
		DisableObjectZDepth (  t.importerCollision[t.importer.collisionShapeCount].object2 );
		SetObjectWireframe (  t.importerCollision[t.importer.collisionShapeCount].object2 , 1 );
		SetObjectLight (  t.importerCollision[t.importer.collisionShapeCount].object2,0 );
		HideObject (  t.importerCollision[t.importer.collisionShapeCount].object2 );

		PositionObject (  t.importerGridObject[9], 0 , 0 , IMPORTERZPOSITION );

		t.importer.selectedCollisionObject = t.importer.collisionShapeCount;
		t.slidersmenuvalue[t.importer.properties2Index][7].value = t.tPRotX_f;
		t.slidersmenuvalue[t.importer.properties2Index][8].value = t.tPRotY_f;
		t.slidersmenuvalue[t.importer.properties2Index][9].value = t.tPRotZ_f;
		++t.importer.collisionShapeCount;
	}
}

void importer_delete_collision_box ( void )
{
	if (  t.importer.collisionShapeCount > 0 ) 
	{
		DeleteObject (  t.importerCollision[t.importer.selectedCollisionObject].object );
		t.importerCollision[t.importer.selectedCollisionObject].object = 0;
		DeleteObject (  t.importerCollision[t.importer.selectedCollisionObject].object2 );
		t.importerCollision[t.importer.selectedCollisionObject].object2 = 0;

		for ( int tCount = t.importer.selectedCollisionObject ; tCount<=  t.importer.collisionShapeCount-1; tCount++ )
		{
			t.importerCollision[tCount].object = t.importerCollision[tCount+1].object;
			t.importerCollision[tCount].object2 = t.importerCollision[tCount+1].object2;
			t.importerCollision[tCount].sizex = t.importerCollision[tCount+1].sizex;
			t.importerCollision[tCount].sizey = t.importerCollision[tCount+1].sizey;
			t.importerCollision[tCount].sizez = t.importerCollision[tCount+1].sizez;
			t.importerCollision[tCount].rotx = t.importerCollision[tCount+1].rotx;
			t.importerCollision[tCount].roty = t.importerCollision[tCount+1].roty;
			t.importerCollision[tCount].rotz = t.importerCollision[tCount+1].rotz;
		}

		--t.importer.collisionShapeCount;
		t.importer.selectedCollisionObject = t.importer.collisionShapeCount-1;
		if (  t.importer.selectedCollisionObject  >=  0 ) 
		{
			t.slidersmenuvalue[t.importer.properties2Index][7].value = t.importerCollision[t.importer.selectedCollisionObject].rotx;
			t.slidersmenuvalue[t.importer.properties2Index][8].value = t.importerCollision[t.importer.selectedCollisionObject].roty;
			t.slidersmenuvalue[t.importer.properties2Index][9].value = t.importerCollision[t.importer.selectedCollisionObject].rotz;
		}
		if (  t.importer.collisionShapeCount > 0 ) 
		{
			if (  t.importerCollision[t.importer.selectedCollisionObject].object > 0 && t.importerCollision[t.importer.selectedCollisionObject].object2 > 0 ) 
			{
				if (  ObjectExist(t.importerCollision[t.importer.selectedCollisionObject].object)  ==  1 && ObjectExist(t.importerCollision[t.importer.selectedCollisionObject].object2)  ==  1 ) 
				{
					RotateObject (  t.importerCollision[t.importer.selectedCollisionObject].object, t.slidersmenuvalue[t.importer.properties2Index][7].value, t.slidersmenuvalue[t.importer.properties2Index][8].value, t.slidersmenuvalue[t.importer.properties2Index][9].value );
					RotateObject (  t.importerCollision[t.importer.selectedCollisionObject].object2, t.slidersmenuvalue[t.importer.properties2Index][7].value, t.slidersmenuvalue[t.importer.properties2Index][8].value, t.slidersmenuvalue[t.importer.properties2Index][9].value );
					t.importerCollision[t.importer.selectedCollisionObject].rotx = t.slidersmenuvalue[t.importer.properties2Index][7].value;
					t.importerCollision[t.importer.selectedCollisionObject].roty = t.slidersmenuvalue[t.importer.properties2Index][8].value;
					t.importerCollision[t.importer.selectedCollisionObject].rotz = t.slidersmenuvalue[t.importer.properties2Index][9].value;
				}
			}
		}
	}
}

void importer_checkForShaderFiles ( void )
{
	t.tOriginalPath_s = t.importer.startDir;
	if (  strcmp ( Right(t.tOriginalPath_s.Get(),1) , "\\" ) != 0  )  t.tOriginalPath_s  =  t.tOriginalPath_s + "\\";
	SetDir (  cstr(t.tOriginalPath_s + "effectbank\\reloaded").Get() );
	t.importer.shaderFileCount = 0;
	FindFirst (  );
	do
	{
		t.ts_s = GetFileName();
		t.ts_s = Lower(t.ts_s.Get());

		//  Remove the shaders that are not for imported entities
		if ( t.ts_s == "apbr_core.fx" ) t.ts_s = "";
		if ( t.ts_s == "apbr_terrain.fx" ) t.ts_s = "";
		if ( t.ts_s == "cascadeshadows.fx" ) t.ts_s = "";
		if ( t.ts_s == "character_editor.fx" ) t.ts_s = "";
		if ( t.ts_s == "character_static.fx" ) t.ts_s = "";
		if ( t.ts_s == "constantbuffers.fx" ) t.ts_s = "";
		if ( t.ts_s == "dynamicterrainshadow_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "ebe_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "entity_core.fx" ) t.ts_s = "";
		if ( t.ts_s == "gui_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "gui_diffuse.fx" ) t.ts_s = "";
		if ( t.ts_s == "gui_showdepth.fx" ) t.ts_s = "";
		if ( t.ts_s == "gui_wireframe.fx" ) t.ts_s = "";
		if ( t.ts_s == "post-bloom.fx" ) t.ts_s = "";
		if ( t.ts_s == "post-core.fx" ) t.ts_s = "";
		if ( t.ts_s == "post-none.fx" ) t.ts_s = "";
		if ( t.ts_s == "post-rift.fx" ) t.ts_s = "";
		if ( t.ts_s == "post-sao.fx" ) t.ts_s = "";
		if ( t.ts_s == "scatter.fx" ) t.ts_s = "";
		if ( t.ts_s == "settings.fx" ) t.ts_s = "";
		if ( t.ts_s == "shadow_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "sky_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "sky_core.fx" ) t.ts_s = "";
		if ( t.ts_s == "sky_scroll.fx" ) t.ts_s = "";
		if ( t.ts_s == "skyscroll_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "sprite_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "static_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "terrain_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "vegetation_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "water_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "weapon_basic.fx" ) t.ts_s = "";
		if ( t.ts_s == "weapon_bone.fx" ) t.ts_s = "";
		if ( strcmp ( Right(t.ts_s.Get(),3) , ".fx" ) == 0 ) 
		{
			if ( t.importer.shaderFileCount < IMPORTERSHADERFILESMAX ) 
			{
				t.importerShaderFiles[t.importer.shaderFileCount+1] = t.ts_s;
				++t.importer.shaderFileCount;
			}
		}
		FindNext (  );
	} while ( !(  GetFileType() == -1 ) );
	SetDir (  t.importer.startDir.Get() );
}

void importer_checkForScriptFiles ( void )
{
	t.tOriginalPath_s = t.importer.startDir;
	if (  cstr(Right(t.tOriginalPath_s.Get(),1))  !=  "\\"  )  t.tOriginalPath_s  =  t.tOriginalPath_s + "\\";
	SetDir (  cstr(t.tOriginalPath_s + "scriptbank").Get() );

	t.importer.scriptFileCount = 0;

	FindFirst (  );
	do
	{

		t.ts_s = GetFileName();
		t.ts_s = Lower(t.ts_s.Get());

		if (  cstr(Right(t.ts_s.Get(),4))  ==  ".lua" ) 
		{
			if (  t.importer.scriptFileCount < IMPORTERSCRIPTFILESMAX ) 
			{
				t.importerScriptFiles[t.importer.scriptFileCount+1] = t.ts_s;
				++t.importer.scriptFileCount;
			}
		}
		FindNext (  );
	} while ( !(  GetFileType() == -1 ) );

	SetDir (  t.importer.startDir.Get() );
}

void importer_help ( void )
{

	if (  t.inputsys.mclick  ==  1 && t.importer.oldMouseClick  ==  0 ) 
	{
		if (  t.importer.MouseX  >=  SpriteX(t.importer.helpSprite) && t.importer.MouseX  <=  SpriteX(t.importer.helpSprite) + 32 && t.importer.MouseY >=  SpriteY(t.importer.helpSprite) && t.importer.MouseY  <=  SpriteY(t.importer.helpSprite) + 32 ) 
		{
			t.importer.helpShow = 1 - t.importer.helpShow;
			t.importer.oldMouseClick = 1;
			if (  t.importer.helpShow  ==  1  )  t.importer.helpFade  =  0;
		}
		else
		{
			t.importer.helpShow = 0;
		}
	}

	if (  t.importer.helpShow  ==  0 ) 
	{
		SetSpriteDiffuse (  t.importer.helpSprite , 255,255,255 );
		if (  SpriteExist(t.importer.helpSprite4) ) 
		{
			SetSpriteAlpha (  t.importer.helpSprite4, t.importer.helpFade );
			if (  t.importer.helpFade > 0  )  t.importer.helpFade  =  t.importer.helpFade - 20;
			if (  t.importer.helpFade  <=  0 ) 
			{
				t.importer.helpFade = 0;
				DeleteSprite (  t.importer.helpSprite4 );
				if (  ImageExist(g.importermenuimageoffset+9)  )  DeleteImage (  g.importermenuimageoffset+9 );
			}
			else
			{
				Sprite (  t.importer.helpSprite4 , (GetChildWindowWidth()/2) - 303 , (GetChildWindowHeight()/2) - 213, g.importermenuimageoffset+9 );
			}
		}
		return;
	}

	t.importer.message ="Left click to hide help";

	SetSpriteDiffuse (  t.importer.helpSprite , 100,100,100 );

	t.tx = 40;
	t.ty = 64;

	if (  ImageExist(g.importermenuimageoffset+9)  ==  0 ) 
	{
		CreateBitmap (  32,605,435 );
		SetCurrentBitmap (  32 );

		CLS (  Rgb(28,34,39) );

		pastebitmapfont("Importer Help",(605/2) - getbitmapfontwidth("Importer Help",1)/2,20,1,255);

		t.tT_s = "Properties Shortcut Keys";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,1,255);

		t.ty += 32;
		t.tT_s = "When the Properties tab is selected, clicking and dragging on the object will allow you to rotate it.";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);
		t.ty += 20;
		t.tT_s  =  "Holding shift while dragging the object will limit the rotation to 45 degree steps" ;
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 40;
		t.tT_s = "Collision Shortcut Keys";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,1,255);

		t.ty += 32;
		t.tT_s = "When the Collsion tab is selected, You can change the view by using the cursor keys:";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 20;
		t.tT_s = "UP: Top view, LEFT: Left view, RIGHT: Right View, DOWN: Front view";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 32;
		t.tT_s = "When resizing t.a collision Box ( , you can limit t.movement to an axis ):";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 20;
		t.tT_s = "X: Limit to X axis, C: Limit to Y axis, V: Limit to Z axis";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 20;
		t.tT_s = "You can limit more than one axis at t.a time by holding multiple keys down";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 20;
		t.tT_s  =  "Holding shift while dragging collision boxes will snap them to other boxes" ; 
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		t.ty += 40;
		t.tT_s = "Thumbnail Shortcut Keys";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,1,255);

		t.ty += 32;
		t.tT_s = "When the Thumbnail tab is selected, clicking and dragging on the object will allow you to rotate it.";
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);
		t.ty += 20;
		t.tT_s  =  "Holding shift while dragging the object will limit the rotation to 45 degree steps" ; 
		pastebitmapfont(t.tT_s.Get(),t.tx,t.ty,2,255);

		GrabImage (  g.importermenuimageoffset+9,0,0,605,435,3 );
		SetCurrentBitmap (  0 );
		DeleteBitmapEx (  32 );
	}
	Sprite (  t.importer.helpSprite4 , (GetChildWindowWidth()/2) - 303 , (GetChildWindowHeight()/2) - 213, g.importermenuimageoffset+9 );
	SetSpritePriority (  t.importer.helpSprite4,2 );
	SetSpriteAlpha (  t.importer.helpSprite4, t.importer.helpFade );
	if (  t.importer.helpFade < 255  )  t.importer.helpFade  =  t.importer.helpFade + 20;
	if (  t.importer.helpFade > 255  )  t.importer.helpFade  =  255;
}

void importer_screenSwitch ( void )
{ 
	if (  t.importer.scaleMulti > 1.0 ) 
	{
		Sprite (  t.importer.helpSprite , (GetChildWindowWidth()/2) - 170 + 128 , 0, g.importermenuimageoffset+8 );
	}
	else
	{
		Sprite (  t.importer.helpSprite , (GetChildWindowWidth()/2) - 170 + 128 , (GetChildWindowHeight()/2) - 300 - 4, g.importermenuimageoffset+8 );
	}

	if (  t.importer.scaleMulti > 1.0 ) 
	{
		t.importerTabs[1].y = 0;
		t.importerTabs[2].y = 0;
		t.importerTabs[3].y = 0;
		t.importerTabs[4].y = 0;
		t.importerTabs[5].y = 0;
		t.importerTabs[11].y = 0;
		t.slidersmenu[t.importer.properties1Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties2Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties3Index].tleft= GetChildWindowWidth() - 255;
		t.slidersmenu[t.importer.properties4Index].tleft= GetChildWindowWidth() - 255;

		//  New button
		t.importerTabs[6].x = GetChildWindowWidth() - 159;
		t.importerTabs[7].x = GetChildWindowWidth() - 159;
		t.importerTabs[8].x = GetChildWindowWidth() - 159;
		t.importerTabs[9].x = GetChildWindowWidth() - 159;
		t.importerTabs[10].x = GetChildWindowWidth() - 159;
		t.importerTabs[12].x = GetChildWindowWidth() - 159;

	}
	else
	{
		t.slidersmenu[t.importer.properties1Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties2Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties3Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
		t.slidersmenu[t.importer.properties4Index].tleft= (GetChildWindowWidth() / 2 ) + 330;
	
		t.importerTabs[1].x = (GetChildWindowWidth() / 2) + 65-128 + 250;
		t.importerTabs[1].y = (GetChildWindowHeight() / 2) - 304;

		t.importerTabs[2].x = t.importerTabs[1].x + 128;
		t.importerTabs[2].y = t.importerTabs[1].y;

		t.importerTabs[3].x = t.importerTabs[2].x + 128;
		t.importerTabs[3].y = t.importerTabs[1].y;

		t.importerTabs[4].x = t.importerTabs[3].x + 256;
		t.importerTabs[4].y = t.importerTabs[1].y;

		//  Importer button
		t.importerTabs[5].x = (GetChildWindowWidth() / 2) - 320;
		t.importerTabs[5].y = (GetChildWindowHeight() / 2) - 304;

		//  New button
		t.importerTabs[6].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[6].y = t.importerTabs[1].y + 125;

		//  New button
		t.importerTabs[12].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[12].y = t.importerTabs[1].y + 125 + 38;

		//  Delete button
		t.importerTabs[7].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[7].y = t.importerTabs[1].y + 125 + (38*2);

		//  Next button
		t.importerTabs[8].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[8].y = t.importerTabs[1].y + 125 + (38*3);

		//  Previous button
		t.importerTabs[9].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[9].y = t.importerTabs[1].y + 125 + (38*4);

		//  Show Guide button
		t.importerTabs[10].x = t.importerTabs[4].x - 28 + 10 - 256 + 1;
		t.importerTabs[10].y = t.importerTabs[1].y + 125 + (38*10) - 5;

		//  Importer button
		t.importerTabs[11].x = (GetChildWindowWidth() / 2) - 320 +128;
		t.importerTabs[11].y = (GetChildWindowHeight() / 2) - 304;
	}
}

void importer_debug ( char* debugText_s )
{
	while (  SpaceKey() ) 
	{
	}
	while (  SpaceKey()  ==  0 ) 
	{
		SetCursor (  0 , 100 );
		t.strwork = "" ; t.strwork = t.strwork + "Debug ): " + debugText_s;
		Print ( t.strwork.Get() );
		Sync (  );
	}
}

void importer_debug_num ( char* title_s, int  debugNumber )
{
	cstr debugText_s =  "";

	while (  SpaceKey() ) 
	{
	}
	if (  title_s  ==  ""  )  title_s  =  "Number";
	debugText_s = ""; debugText_s = debugText_s + title_s + ": " + Str(debugNumber);
	while (  SpaceKey()  ==  0 ) 
	{
		SetCursor (  0 , 100 );
		Print (  debugText_s.Get() );
		Sync (  );
	}
}

// 
//  File dialog functions
// 

int findFreeDll ( void )
{
	int tFoundFree = 0;
	int tCount = 0;

	tFoundFree = 0;

	for ( tCount = 1 ; tCount<=  50; tCount++ )
	{
		if (  DLLExist ( tCount )  ==  0 ) 
		{
			tFoundFree = tCount;
			break;
		}
	}
	return tFoundFree;
}

int findFreeMemblock ( void )
{
	int tFoundFree = 0;
	int tCount = 0;

	tFoundFree = 0;

	for ( tCount = 1 ; tCount<=  50; tCount++ )
	{
		if (  MemblockExist (  tCount )  ==  0 ) 
		{
			tFoundFree = tCount;
			break;
		}
	}
	return tFoundFree;
}

char* openFileBox ( char* filter_, char* initdir_, char* dtitle_, char* defext_, unsigned char open )
{
	DWORD filebufferptr = 0;
	int OPENFILENAME = 0;
	cstr filebuffer =  "";
	int comdlg32 = 0;
	cstr tCode_s =  "";
	int tCount2 = 0;
	cstr code_s =  "";
	DWORD retval = 0;
	int tFound = 0;
	int user32 = 0;
	DWORD flags = 0;
	DWORD lpofn = 0;
	DWORD hwnd = 0;
	int size = 0;
	char filter[512];
	char initdir[512];
	char dtitle[512];
	char defext[512];

	strcpy ( filter , filter_ );
	strcpy ( initdir , initdir_ );
	strcpy ( dtitle , dtitle_ );
	strcpy ( defext , defext_ );

	if ( t.importer.loaded == 1 ) importer_show_mouse ( );
	
	//  Load in required DLL's
	user32 = findFreeDll();
	DLLLoad (  "user32.dll",user32 );
	comdlg32 = findFreeDll();
	DLLLoad (  "comdlg32.dll",comdlg32 );

	//  Get handle ( unique ID ) to the calling ( this ) window
	hwnd = CallDLL(user32,"GetForegroundWindow");

	//  Unload DLL as it is no longer needed
	DLLDelete (  user32 );

	//  Get the Memblock Number
	OPENFILENAME = findFreeMemblock();

	//  Make The Memblock containing the OPENFILENAME structure
	MakeMemblock (  OPENFILENAME,76 );

	//  Get the pointer to the just created Structure
	lpofn = GetMemblockPtr(OPENFILENAME);

	strcat ( filter , "|" );
	strcat ( initdir ,  "|" );
	strcat ( dtitle , "|" );
	strcat ( defext , "|" );
	filebuffer = "" ; filebuffer = filebuffer + "|";
	filebuffer+= Spaces(255);
	filebuffer+= "|";
	filebufferptr = _get_str_ptr(filebuffer.Get());
	flags = 0x00001000 || 0x00000004 || 0x00000002;
	size = 0;

	WriteMemblockDWord (  OPENFILENAME,0,76 );
	WriteMemblockDWord (  OPENFILENAME,4,hwnd );
	WriteMemblockDWord (  OPENFILENAME,12,_get_str_ptr(filter) );
	WriteMemblockDWord (  OPENFILENAME,24,1 );
	WriteMemblockDWord (  OPENFILENAME,28,filebufferptr );
	WriteMemblockDWord (  OPENFILENAME,32,256 );
	WriteMemblockDWord (  OPENFILENAME,44,_get_str_ptr(initdir) );
	WriteMemblockDWord (  OPENFILENAME,48,_get_str_ptr(dtitle) );
	WriteMemblockDWord (  OPENFILENAME,52,flags );
	WriteMemblockDWord (  OPENFILENAME,60,_get_str_ptr(defext) );

	//  Call the Command to open the dialouge
	if ( open )
	{
		retval = CallDLL(comdlg32,"GetOpenFileNameA",lpofn);
	}
	else
	{
		retval = CallDLL(comdlg32,"GetSaveFileNameA",lpofn);
	}
	//  Check if it was sucecfull
	if  ( retval !=  0 )
	{
		code_s = _get_str(filebufferptr,256);
		tCode_s = "";
		tFound = 0;
		for ( tCount2 = Len(code_s.Get()) ; tCount2>=  1 ; tCount2+= -1 )
		{
			if (  code_s.Get()[tCount2-1]  !=  ' '  )  tFound  =  1;
			t.strwork = ""; t.strwork = t.strwork +  Mid(code_s.Get(),tCount2) + tCode_s;;
			if (  tFound  !=  0  )  tCode_s  = t.strwork;
		}
		code_s = tCode_s;
	}
	else
	{
		retval = CallDLL(comdlg32,"CommDlgExtendedError");
		switch ( retval )
		{
			case 0xFFFF :  code_s  =  "The dialog box could not be created. The common dialog box function's call to the DialogBox function failed. For example, this error occurs if the common dialog box call specifies an invalid window handle."; break;
			case 0x0006 : code_s = "The common dialog box function failed to find a specified resource." ; break;
			case 0x0004 : code_s = "The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle." ; break;
			case 0x0002 : code_s = "The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available." ; break;
			case 0x000B : code_s = "The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure." ; break;
			case 0x0008 : code_s = "The common dialog box function failed to lock a specified resource." ; break;
			case 0x0003 : code_s = "The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template." ; break;
			case 0x0007 : code_s = "The common dialog box function failed to load a specified string." ; break;
			case 0x0001 : code_s = "The lStructSize member of the initialization structure for the corresponding common dialog box is invalid." ; break;
			case 0x0005 : code_s = "The common dialog box function failed to load a specified string." ;break;
			case 0x3003 : code_s = "The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the file name specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size, in TCHARs, required to receive the full name." ; break;
			case 0x0009 : code_s = "The common dialog box function was unable to allocate memory for internal structures." ; break;
			case 0x3002 : code_s = "A file name is invalid." ; break;
			case 0x000A : code_s = "The common dialog box function was unable to lock the memory associated with a handle." ; break;
			case 0x3001 : code_s = "An attempt to subclass a list box failed because sufficient memory was not available." ; break;
			default : code_s = "Error" ; break;
		}
	}

	DLLDelete (  comdlg32 );

	DeleteMemblock (  OPENFILENAME );

	strcpy ( t.szreturn , code_s.Get() );
	return t.szreturn;
}

int _get_str_ptr ( char* pstr )
{
	int memnum = 0;
	DWORD memptr = 0;
	int strlen = 0;
	DWORD strptr = 0;
	char cchar = 0;

	memnum = findFreeMemblock();
	strlen = Len(pstr);

	MakeMemblock (  memnum,strlen );

	for ( int i = 1 ; i <= strlen ; i++ )
	{
		if ( pstr[i-1] == '|' )
		{
			cchar = 0;
		}
		else
		{
			cchar = pstr[i-1];
		}

		if ( cchar > 255 ) cchar = 255;
		if ( cchar < 0 ) cchar = 0;

		WriteMemblockByte (  memnum,(i - 1), (byte)cchar );
	}

	memptr = GetMemblockPtr(memnum);
	strptr = MakeMemory(strlen);

	CopyMemory (  strptr,memptr,strlen );

	DeleteMemblock (  memnum );

	return strptr;
}

char* _get_str ( DWORD strptr, int strsize )
{
	int memnum = 0;
	DWORD memptr = 0;
	cstr cchar =  "";
	cstr str = "";

	memnum = findFreeMemblock();
	MakeMemblock (  memnum,strsize );
	memptr = GetMemblockPtr(memnum);

	CopyMemory (  memptr,strptr,strsize );

	for ( int  i = 1 ; i <= strsize; i++ )
	{
		str = str + Chr(ReadMemblockByte(memnum,i - 1));
	}

	DeleteMemblock (  memnum );

	t.returnstring_s = str;

	return t.returnstring_s.Get();

}

char* importerPadString ( char* tString )
{
	t.returnstring_s = tString;

	while (  Len(t.returnstring_s.Get()) < 17 ) 
	{
		t.returnstring_s += " ";
	}

	return t.returnstring_s.Get();
}

int findFreeObject ( void )
{
	int found = 0;

	found = g.importermenuobjectoffset;
	while (  ObjectExist ( found )  ==  1 )
	{
		++found;
	}
	return found;
}

float GetDistance ( float x1, float  y1, float  z1, float  x2, float  y2, float  z2 )
{
	float result_f = 0;
	float tXd_f = 0;
	float tYd_f = 0;
	float yZd_f = 0;

	tXd_f = x2 - x1;
	tYd_f = y2 - y1;
	yZd_f = z2 - z1;

	result_f = Sqrt ( tXd_f * tXd_f + tYd_f * tYd_f + yZd_f * yZd_f );

	return result_f;
}

int importer_check_if_protected ( char* timporterfilecheck_s )
{
	int timporterprotectedcheck = 0;
	cstr tStippedFileName_s =  "";
	cstr tStippedString_s =  "";
	int tArrayMarker = 0;
	cstr tempLine_s =  "";
	cstr tstring_s =  "";
	cstr tToken2_s =  "";
	cstr tToken_s =  "";
	int tCount = 0;

	timporterprotectedcheck = 0;

	//  Ensure it has the dbo extension
	Dim (  t.tArray,400  );
	tArrayMarker = 0;
	tstring_s=timporterfilecheck_s;
	tToken_s=FirstToken(tstring_s.Get(),".");
	if (  tToken_s  !=  "" ) 
	{
		t.tArray[tArrayMarker] = tToken_s;
		++tArrayMarker;
	}
	do
	{
		tToken_s=NextToken(".");
		if (  tToken_s  !=  "" ) 
		{
			t.tArray[tArrayMarker] = tToken_s;
			++tArrayMarker;
		}
	} while ( !(  tToken_s == "" ) );
	tStippedFileName_s = "";
	for ( tCount = 0 ; tCount<=  tArrayMarker-2; tCount++ )
	{
		tStippedFileName_s = tStippedFileName_s + t.tArray[tCount];
	}
	UnDim (  t.tArray );
	Dim (  t.tArray,400  );

	tStippedFileName_s = timporterfilecheck_s;
	if (  cstr(Mid(tStippedFileName_s.Get(),Len(tStippedFileName_s.Get())-1))  ==  "." ) 
	{
			tStippedFileName_s = Left(tStippedFileName_s.Get(),Len(tStippedFileName_s.Get())-2);
	}
	else
	{
		tStippedFileName_s = Left(tStippedFileName_s.Get(),Len(tStippedFileName_s.Get())-4);
	}
	t.strwork = ""; t.strwork =  tStippedFileName_s + ".fpe";
	strcpy ( timporterfilecheck_s , t.strwork.Get() );

	//  Check if an FPE exists, if so load it in
	if (  FileOpen(1)  )  CloseFile (1) ;
	if (  FileExist (timporterfilecheck_s) ) 
	{
		OpenToRead (  1 , timporterfilecheck_s );
		while (  FileEnd(1)  ==  0 ) 
		{
			t.tstring_s = ReadString (  1 );
			tempLine_s = t.tstring_s;

			tArrayMarker = 0;
			tToken_s=FirstToken(t.tstring_s.Get()," ");
			if (  tToken_s  !=  "" ) 
			{
				t.tArray[tArrayMarker] = tToken_s;
				++tArrayMarker;
			}
			do
			{
				tToken_s=NextToken(" ");
				if (  tToken_s  !=  "" ) 
				{
					t.tArray[tArrayMarker] = tToken_s;
					++tArrayMarker;
				}
			} while ( !(  tToken_s == "" ) );
			tStippedString_s = "";
			for ( tCount = 0 ; tCount<=  tArrayMarker-1; tCount++ )
			{
				if (  tCount < 3 ) 
				{
					tStippedString_s = tStippedString_s + t.tArray[tCount];
				}
				else
				{
					tStippedString_s = tStippedString_s + " " + t.tArray[tCount];
				}
			}
			if (  tStippedString_s  !=  "" && tStippedString_s.Get()[0]  !=  ';' )
			{
				tToken_s=FirstToken(tStippedString_s.Get(),"=");
				tToken2_s=NextToken("=");

				//  Get rid of any tabs that exist and replace with nothing (some files have tabs in sometimes)
				t.tstring_s = tToken_s ; tToken_s = "";
				for ( tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
				{
					if ( cstr( Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  tToken_s  =  tToken_s + Mid(t.tstring_s.Get(),tCount);
				}

				t.tstring_s = tToken2_s ; tToken2_s = "";
				for ( tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
				{
					if (  cstr(Mid(t.tstring_s.Get(),tCount))  !=  Chr(9)  )  tToken2_s  =  tToken2_s + Mid(t.tstring_s.Get(),tCount);
				}

				//  find out which token we have and set the importer fpe string
				if ( tToken_s == "protected" ) timporterprotectedcheck  =  ValF(tToken2_s.Get()) ;
			}
		}

	}

	CloseFile (  1 );
	UnDim (  t.tArray );

	if (  timporterprotectedcheck > 1  )  timporterprotectedcheck  =  1;
	if (  timporterprotectedcheck < 0  )  timporterprotectedcheck  =  0;

	return timporterprotectedcheck;
}

void importer_sort_names ( void )
{
	//  Split the filename into tokens to grab the path, object name and create fpe name
	Dim (  t.tArray,400  );
	t.tArrayMarker = 0;
	t.tstring_s=t.timporterfile_s;
	t.tToken_s=FirstToken(t.tstring_s.Get(),"\\");
	if (  t.tToken_s  !=  "" ) 
	{
		t.tArray[t.tArrayMarker] = t.tToken_s;
		++t.tArrayMarker;
	}
	do
	{
		t.tToken_s=NextToken("\\");
		if (  t.tToken_s  !=  "" ) 
		{
			t.tArray[t.tArrayMarker] = t.tToken_s;
			++t.tArrayMarker;
		}
	} while ( !(  t.tToken_s == "" ) );
	t.tStippedFileName_s = "";

	//  Grab path only
	int tCount = 0;
	for ( tCount = 0 ; tCount<=  t.tArrayMarker-2; tCount++ )
	{
		t.tStippedFileName_s = t.tStippedFileName_s + t.tArray[tCount] + "\\";
	}

	//  Store file path
	t.importer.objectFileOriginalPath = t.tStippedFileName_s;
	if (  cstr(Left(t.importer.objectFileOriginalPath.Get(),2))  ==  ".." ) 
	{
		t.tFPSDir_s = t.importer.startDir;
		t.tFPSDir_s = Left(t.tFPSDir_s.Get(),Len(t.tFPSDir_s.Get())-6);
		t.importer.objectFileOriginalPath = t.tFPSDir_s + Right(t.importer.objectFileOriginalPath.Get(),Len(t.importer.objectFileOriginalPath.Get())-2);
	}

	//  Now store just the file names
	t.tStippedFileName_s = t.tArray[t.tArrayMarker-1];
	t.tOriginalName_s = t.tStippedFileName_s;
	t.tArrayMarker = 0;
	t.tstring_s=t.tStippedFileName_s;
	t.tToken_s=FirstToken(t.tstring_s.Get(),".");
	if (  t.tToken_s  !=  "" ) 
	{
		t.tArray[t.tArrayMarker] = t.tToken_s;
		++t.tArrayMarker;
	}
	do
	{
		t.tToken_s=NextToken(".");
		if (  t.tToken_s  !=  "" ) 
		{
			t.tArray[t.tArrayMarker] = t.tToken_s;
			++t.tArrayMarker;
		}
	} while ( !(  t.tToken_s == "" ) );
	t.tStippedFileName_s = "";
	for ( tCount = 0 ; tCount<=  t.tArrayMarker-2; tCount++ )
	{
		t.tStippedFileName_s = t.tStippedFileName_s + t.tArray[tCount];
	}

	t.tStippedFileName_s = t.tOriginalName_s;
	if (  cstr(Mid(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-1))  ==  "." ) 
	{
			t.tStippedFileName_s = Left(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-2);
	}
	else
	{
		t.tStippedFileName_s = Left(t.tStippedFileName_s.Get(),Len(t.tStippedFileName_s.Get())-4);
	}

	//  Store file names
	t.importer.objectFilename = t.tStippedFileName_s + "." + t.tArray[t.tArrayMarker-1];
	t.importer.objectFilenameFPE = t.tStippedFileName_s +  ".fpe";
	t.importer.objectFilenameExtension = cstr(".") + t.tArray[t.tArrayMarker-1];
	UnDim (  t.tArray );
	
	//  check if it is an fpe that has been loaded in, if so we need to change the model name
	if (  cstr(Lower(Right(t.timporterfile_s.Get(),4)))  ==  ".fpe" ) 
	{
		t.importer.fpeIsMainFile = 1;
		importer_find_object_name_from_fpe ( );
	}
	else
	{
		t.importer.fpeIsMainFile = 0;
	}
}

void importer_find_object_name_from_fpe ( void )
{
	t.timporterprotectedcheck = 0;
	Dim (  t.tArray,400  );
	t.timporterfilecheck_s = t.timporterfile_s;

	//  Check if an FPE exists, if so load it in
	if ( FileOpen(1) )  CloseFile (1);
	if (  FileExist (t.timporterfilecheck_s.Get()) ) 
	{
		OpenToRead (  1 , t.timporterfilecheck_s.Get() );
		while (  FileEnd(1)  ==  0 ) 
		{
			t.tstring_s = ReadString (  1 );
			t.tempLine_s = t.tstring_s;

			t.tArrayMarker = 0;
			t.tToken_s=FirstToken(t.tstring_s.Get()," ");
			if (  t.tToken_s  !=  "" ) 
			{
				t.tArray[t.tArrayMarker] = t.tToken_s;
				++t.tArrayMarker;
			}
			do
			{
				t.tToken_s=NextToken(" ");
				if (  t.tToken_s  !=  "" ) 
				{
					t.tArray[t.tArrayMarker] = t.tToken_s;
					++t.tArrayMarker;
				}
			} while ( !(  t.tToken_s == "" ) );
			t.tStippedString_s = "";
			int tCount = 0;
			for ( tCount = 0 ; tCount<=  t.tArrayMarker-1; tCount++ )
			{
				if (  tCount < 3 ) 
				{
					t.tStippedString_s = t.tStippedString_s + t.tArray[tCount];
				}
				else
				{
					t.tStippedString_s = t.tStippedString_s + " " + t.tArray[tCount];
				}
			}
			if (  t.tStippedString_s  !=  "" && t.tStippedString_s.Get()[0]  !=  ';' )
			{
				t.tToken_s=FirstToken(t.tStippedString_s.Get(),"=");
				t.tToken2_s=NextToken("=");

				//  Get rid of any tabs that exist and replace with nothing (some files have tabs in sometimes)
				t.tstring_s = t.tToken_s ; t.tToken_s = "";
				for ( int tCount2 = 1 ; tCount2 <=  Len(t.tstring_s.Get()); tCount2++ )
				{
					if (  cstr(Mid(t.tstring_s.Get(),tCount2))  !=  Chr(9)  )  t.tToken_s  =  t.tToken_s + Mid(t.tstring_s.Get(),tCount2);
				}

				t.tstring_s = t.tToken2_s ; t.tToken2_s = "";
				for ( tCount = 1 ; tCount<=  Len(t.tstring_s.Get()); tCount++ )
				{
					if ( cstr( Mid(t.tstring_s.Get(),tCount) ) !=  Chr(9)  )  t.tToken2_s  =  t.tToken2_s + Mid(t.tstring_s.Get(),tCount);
				}

				if ( t.tToken_s == "model" ) t.importer.objectFilename  =  t.tToken2_s ;
			}
		}

	}

	CloseFile (  1 );
	UnDim (  t.tArray );

return;

}

void importer_hide_mouse ( void )
{
	HideMouse (  );
}

void importer_show_mouse ( void )
{
	ShowMouse (  );
}

void importer_fade_out ( void )
{
	t.tfound = 0;
	t.twhiteobj = 1;
	while (  t.tfound  ==  0 ) 
	{
		if (  ObjectExist(t.twhiteobj)  ==  1 ) 
		{
			++t.twhiteobj;
		}
		else
		{
			t.tfound = 1;
		}
	}

	MakeObjectBox (  t.twhiteobj,200000,200000,1 );
	SetObjectLight (  t.twhiteobj,1 );
	LockObjectOn (  t.twhiteobj );
	PositionObject (  t.twhiteobj,0,0,4000 );
	DisableObjectZDepth (  t.twhiteobj );
	SetAlphaMappingOn (  t.twhiteobj,0 );
	ColorObject (  t.twhiteobj , Rgb(0,0,0) );
	SetObjectAmbience (  t.twhiteobj,0 );
	SetObjectEmissive (  t.twhiteobj, Rgb(40,104,131) );

	for ( t.tc = 0 ; t.tc <=  100 ; t.tc += 10 )
	{
			SetAlphaMappingOn (  t.twhiteobj,t.tc );
			Sync (  );
	}
	DeleteObject (  t.twhiteobj );
}

void importer_fade_in ( void )
{
	t.tfound = 0;
	t.twhiteobj = 1;
	while (  t.tfound  ==  0 ) 
	{
		if (  ObjectExist(t.twhiteobj)  ==  1 ) 
		{
			++t.twhiteobj;
		}
		else
		{
			t.tfound = 1;
		}
	}

	MakeObjectBox (  t.twhiteobj,200000,200000,1 );
	SetObjectLight (  t.twhiteobj,1 );
	LockObjectOn (  t.twhiteobj );
	PositionObject (  t.twhiteobj,0,0,4000 );
	DisableObjectZDepth (  t.twhiteobj );
	SetAlphaMappingOn (  t.twhiteobj,100 );
	ColorObject (  t.twhiteobj , Rgb(0,0,0) );
	SetObjectAmbience (  t.twhiteobj,0 );
	SetObjectEmissive (  t.twhiteobj, Rgb(40,104,131) );

	for ( t.tc = 100 ; t.tc >=  0 ; t.tc+= -10 )
	{
			SetAlphaMappingOn (  t.twhiteobj,t.tc );
			Sync (  );
	}
	DeleteObject (  t.twhiteobj );
}

void importer_check_script_token_exists ( void )
{
	t.tfound = 0;
	for ( t.tloop = 0 ; t.tloop<=  t.importer.scriptFileCount-1; t.tloop++ )
	{
			if (  t.tToken2_s  ==  t.importerScriptFiles[t.tloop]  )  t.tfound  =  1;
	}

	if (  t.tfound  ==  0  )  t.tToken2_s  =  "default.lua";
}

void importer_update_scale ( void )
{
	if (  t.inputsys.mclick  ==  0 ) 
	{
		t.importer.dropDownListNumber = 0;
		t.importer.oldTime = 0;

		if (  t.timporterprevscale  !=  t.slidersmenuvalue[t.importer.properties1Index][1].value ) 
		{
			t.timporterprevscale = t.slidersmenuvalue[t.importer.properties1Index][1].value;
			t.slidersmenuvalue[t.importer.properties1Index][1].value_s=Str(t.slidersmenuvalue[t.importer.properties1Index][1].value);
			t.slidersmenuvalue[t.importer.properties1Index][1].readmodeindex=50;
			t.slidersmenuvalue[t.importer.properties1Index][1].useCustomRange = 1;
			t.slidersmenuvalue[t.importer.properties1Index][1].valueMin = t.slidersmenuvalue[t.importer.properties1Index][1].value-49;
			t.slidersmenuvalue[t.importer.properties1Index][1].valueMax = t.slidersmenuvalue[t.importer.properties1Index][1].value+50;
			if (  t.slidersmenuvalue[t.importer.properties1Index][1].valueMin < 1 ) 
			{
				t.slidersmenuvalue[t.importer.properties1Index][1].valueMin = 1;
				t.slidersmenuvalue[t.importer.properties1Index][1].valueMax = 100;//400; for better lower scale resolution work
			}
		}
	}
}

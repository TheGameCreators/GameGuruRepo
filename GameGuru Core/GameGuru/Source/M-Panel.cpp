//----------------------------------------------------
//--- GAMEGURU - M-Panel
//----------------------------------------------------

// Includes
#include "stdafx.h"
#include "gameguru.h"

// Globals
bool g_bHaveDrawnSome2D = false;
int g_iInit2DDrawingCapture = 0;
float g_fPreferredVRScreenPosX = 0.0f;
float g_fPreferredVRScreenPosY = 0.0f;
float g_fPreferredVRScreenPosZ = 0.0f;
float g_fPreferredVRScreenRotY = 0.0f;

void panel_init ( void )
{
	SetMipmapNum(1); //PE: mipmaps not needed.
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_tl.png",g.panelimageoffset );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_tm.png",g.panelimageoffset+1 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_tr.png",g.panelimageoffset+2 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_ml.png",g.panelimageoffset+3 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_mm.png",g.panelimageoffset+4 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_mr.png",g.panelimageoffset+5 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_bl.png",g.panelimageoffset+6 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_bm.png",g.panelimageoffset+7 );
	LoadImage (  "languagebank\\neutral\\gamecore\\huds\\panels\\panel_br.png",g.panelimageoffset+8 );
	SetMipmapNum(-1);
}

void lua_panel ( void )
{
	t.luaPanel.x = Ceil((t.luaPanel.x * GetDisplayWidth()) / 100.0);
	t.luaPanel.y = Ceil((t.luaPanel.y * GetDisplayHeight()) / 100.0);
	t.luaPanel.x2 = Ceil((t.luaPanel.x2 * GetDisplayWidth()) / 100.0);
	t.luaPanel.y2 = Ceil((t.luaPanel.y2 * GetDisplayHeight()) / 100.0);

	if (  t.luaPanel.x < 0  )  t.luaPanel.x  =  0;
	if (  t.luaPanel.y < 0  )  t.luaPanel.y  =  0;
	if (  t.luaPanel.x2 > GetDisplayWidth()  )  t.luaPanel.x2  =  GetDisplayWidth();
	if (  t.luaPanel.y2 > GetDisplayHeight()  )  t.luaPanel.y2  =  GetDisplayHeight();

	t.tSprite = g.panelimageoffset;
	t.tw = ImageWidth ( g.panelimageoffset );
	t.th = ImageHeight ( g.panelimageoffset );
	if (  (t.luaPanel.x2 - t.luaPanel.x) - (t.tw*2) < 0  )  t.luaPanel.x2  =  t.luaPanel.x + (t.tw*2) + 1;
	if (  (t.luaPanel.y2 - t.luaPanel.y) - (t.th*2) < 0  )  t.luaPanel.y2  =  t.luaPanel.y + (t.th*2) + 1;

	//  top left
	t.timage = g.panelimageoffset;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x,t.luaPanel.y );
	//  top middle
	t.timage = g.panelimageoffset+1;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,(t.luaPanel.x2 - t.luaPanel.x) - (t.tw*2),t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x+t.tw,t.luaPanel.y );
	//  top right
	t.timage = g.panelimageoffset+2;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x2-t.tw,t.luaPanel.y );

	//  middle left
	t.timage = g.panelimageoffset+3;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,(t.luaPanel.y2 - t.luaPanel.y) - (t.th*2) );
	PasteSprite (  t.tSprite,t.luaPanel.x,t.luaPanel.y+t.th );
	//  middle middle
	t.timage = g.panelimageoffset+4;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,(t.luaPanel.x2 - t.luaPanel.x) - (t.tw*2),(t.luaPanel.y2 - t.luaPanel.y) - (t.th*2) );
	PasteSprite (  t.tSprite,t.luaPanel.x+t.tw,t.luaPanel.y+t.th );
	//  middle right
	t.timage = g.panelimageoffset+5;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,(t.luaPanel.y2 - t.luaPanel.y) - (t.th*2) );
	PasteSprite (  t.tSprite,t.luaPanel.x2-t.tw,t.luaPanel.y+t.th );

	t.tSprite = g.panelimageoffset;
	t.tw = ImageWidth ( g.panelimageoffset );
	t.th = ImageHeight ( g.panelimageoffset );
	//  top left
	t.timage = g.panelimageoffset+6;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x,t.luaPanel.y2-t.th );
	//  top middle
	t.timage = g.panelimageoffset+7;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,(t.luaPanel.x2 - t.luaPanel.x) - (t.tw*2),t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x+t.tw,t.luaPanel.y2-t.th );
	//  top right
	t.timage = g.panelimageoffset+8;
	Sprite (  t.tSprite,-10000,-10000,t.timage );
	SizeSprite (  t.tSprite,t.tw,t.th );
	PasteSprite (  t.tSprite,t.luaPanel.x2-t.tw,t.luaPanel.y2-t.th );

	// panel sets the preferred location of the VR screen panel
	if (g_bHaveDrawnSome2D == false && t.luaPanel.e > 0 )
	{
		g_fPreferredVRScreenPosX = t.entityelement[t.luaPanel.e].x;
		g_fPreferredVRScreenPosY = t.entityelement[t.luaPanel.e].y;
		g_fPreferredVRScreenPosZ = t.entityelement[t.luaPanel.e].z;
		g_fPreferredVRScreenRotY = t.entityelement[t.luaPanel.e].ry;
	}

	// flag that some 2D has happened (triggers capture at end of rendering)
	g_bHaveDrawnSome2D = true;
}

void panel_free ( void )
{
	for ( t.i = 0 ; t.i<=  8; t.i++ )
	{
		if (  SpriteExist(g.panelimageoffset+t.i)  )  DeleteSprite (  g.panelimageoffset+t.i );
		if (  ImageExist(g.panelimageoffset+t.i)  )  DeleteImage (  g.panelimageoffset+t.i );
	}
}

//
// Extra code for VR which redirects 2D drawing to a bitmap, for later capture
// as an image so it can be rendered in 3D for VR viewing
//

void panel_First2DDrawing(void)
{
	// if running VR
	#ifdef VRTECH
	if (g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		// called before any 2D drawing is done
		if (g_iInit2DDrawingCapture == 0)
		{
			CreateBitmap(12, GetDisplayWidth(), GetDisplayHeight());
			g_iInit2DDrawingCapture = 1;
		}
		SetCurrentBitmap(12);
		CLS(Rgb(0, 0, 0));

		// if no panel to set VR screen at entity position, use position immediately in front of camera
		g_bHaveDrawnSome2D = false;
		MoveCamera(0, 75);
		g_fPreferredVRScreenPosX = CameraPositionX(0);
		g_fPreferredVRScreenPosY = CameraPositionY(0);
		g_fPreferredVRScreenPosZ = CameraPositionZ(0);
		g_fPreferredVRScreenRotY = CameraAngleY(0)+180.0f;
		MoveCamera(0, -75);
	}
	#endif
}

void panel_Last2DDrawing(void)
{
	// if running VR
	#ifdef VRTECH
	if (g.vrglobals.GGVREnabled > 0 && g.vrglobals.GGVRUsingVRSystem == 1 )
	{
		// called after last of 2D drawing activity completes
		if (g_bHaveDrawnSome2D == true) GrabImage(g.luadrawredirectimageoffset, 0, 0, GetDisplayWidth() - 1, GetDisplayHeight() - 1, 3);
		SetCurrentBitmap(0);
		if (g_bHaveDrawnSome2D == true)
		{
			float fHeightOfVRScreen = GetDisplayHeight() / 10.0f;
			if (g_iInit2DDrawingCapture == 1)
			{
				if (CheckObjectExist(g.luadrawredirectobjectoffset) == 0)
				{
					MakeObjectBox(g.luadrawredirectobjectoffset, GetDisplayWidth() / 10.0f, fHeightOfVRScreen, 0.0f);
					TextureObject(g.luadrawredirectobjectoffset, 0, g.luadrawredirectimageoffset);
					SetObjectEffect(g.luadrawredirectobjectoffset, g.guishadereffectindex);
					SetSphereRadius(g.luadrawredirectobjectoffset, 0);
					SetObjectMask(g.luadrawredirectobjectoffset, (1 << 6) + (1 << 7) + 1);
					g_iInit2DDrawingCapture = 2;
				}
			}
			if (CheckObjectExist(g.luadrawredirectobjectoffset) == 1)
			{
				PositionObject(g.luadrawredirectobjectoffset, g_fPreferredVRScreenPosX, g_fPreferredVRScreenPosY + (fHeightOfVRScreen / 2.0f), g_fPreferredVRScreenPosZ);
				RotateObject(g.luadrawredirectobjectoffset, 0, g_fPreferredVRScreenRotY, 0);
				ShowObject(g.luadrawredirectobjectoffset);
			}
		}
		else
		{
			if (CheckObjectExist(g.luadrawredirectobjectoffset) == 1)
			{
				if (GetVisible(g.luadrawredirectobjectoffset) == 1 )
				{
					HideObject(g.luadrawredirectobjectoffset);
				}
			}
		}
	}
	#endif
}

void panel_Free2DDrawing(void)
{
	#ifdef VRTECH
	if (BitmapExist(12) == 1) DeleteBitmapEx(12);
	if (CheckObjectExist(g.luadrawredirectobjectoffset) == 1) DeleteObject(g.luadrawredirectobjectoffset);
	if (GetImageExistEx(g.luadrawredirectimageoffset) == 1) DeleteImage(g.luadrawredirectimageoffset);
	g_iInit2DDrawingCapture = 0;
	#endif
}
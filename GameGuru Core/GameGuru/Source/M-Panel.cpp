//----------------------------------------------------
//--- GAMEGURU - M-Panel
//----------------------------------------------------

#include "gameguru.h"

//  ***** Included Source File *****
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
return;

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

return;

}

void panel_free ( void )
{

for ( t.i = 0 ; t.i<=  8; t.i++ )
{
	if (  SpriteExist(g.panelimageoffset+t.i)  )  DeleteSprite (  g.panelimageoffset+t.i );
	if (  ImageExist(g.panelimageoffset+t.i)  )  DeleteImage (  g.panelimageoffset+t.i );
}

}

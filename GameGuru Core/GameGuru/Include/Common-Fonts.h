//----------------------------------------------------
//--- GAMEGURU - Common-Fonts
//----------------------------------------------------

#include "cstr.h"

void initbitmapfont ( char* name_s, int index );
void loadallfonts ( void );
void changebitmapfont ( char* name_s, int index ); // new function to replace a font (dave)
void shrinkbitmapfont ( char* name_s, int index, int fromindex );
void pastebitmapfont ( char* text_s, int tx, int ty, int index, int alpha );
void pastebitmapfontcenter ( char* text_s, int tx, int ty, int index, int alpha );
void pastebitmapfontcenterwithboxout ( char* text_s, int tx, int ty, int index, int alpha );
int getbitmapfontwidth ( char* text_s, int index );
int getbitmapfontheight ( int index );
void pastebitmapfontcolor ( char* text_s, int tx, int ty, int index, int alpha, int r, int g, int b );

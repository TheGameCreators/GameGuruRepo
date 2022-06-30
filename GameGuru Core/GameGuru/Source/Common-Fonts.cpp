//----------------------------------------------------
//--- GAMEGURU - Common-Fonts
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

// 
//  BITMAP FONT
// 

//(dave) new function to relace a font
void changebitmapfont ( char* name_s, int index )
{
	if (  ImageExist(g.bitmapfontimagetart+index) == 1 ) 
		DeleteImage (g.bitmapfontimagetart+index);

	initbitmapfont ( name_s , index );
}

void loadallfonts ( void )
{
	timestampactivity(0, "initbitmapfont - startlegacy image loading");
	image_setlegacyimageloading(true);
	timestampactivity(0, "initbitmapfont - deleting old images");
	for ( int c = 1 ; c < 8 ; c++ )
	{
		if (  ImageExist(g.bitmapfontimagetart+c) == 1 ) 
			DeleteImage (g.bitmapfontimagetart+c);
	}
	timestampactivity(0, "initbitmapfont - first font load");
	initbitmapfont("fontbank\\FPSCR-Font-24px",1);
	timestampactivity(0, "initbitmapfont - first font shrink");
	shrinkbitmapfont("fontbank\\FPSCR-Font-12px",2,1);
	timestampactivity(0, "initbitmapfont - remaining font loads");
	initbitmapfont("fontbank\\FPSCR-Font-36px",3);
	initbitmapfont("fontbank\\FPSCR-Font-40px",4);
	initbitmapfont("fontbank\\FPSCR-Font-72px",5);
	initbitmapfont("fontbank\\FPSCR-Font-36px-Multiplayer",6);
	initbitmapfont("fontbank\\FPSCR-Font-25px",7);
	timestampactivity(0, "shrinkbitmapfont - new medium small size text");
	shrinkbitmapfont("fontbank\\FPSCR-Font-40px", 8, 4);
	timestampactivity(0, "initbitmapfont - finish legacy loading");
	image_setlegacyimageloading(false);
}

void initbitmapfont ( char* name_s, int index )
{
	int charindex = 0;
	float lastbh_f = 0;
	cstr char_s =  "";
	cstr line_s =  "";
	float bx1_f = 0;
	float bx2_f = 0;
	float by1_f = 0;
	float by2_f = 0;
	cstr png_s =  "";
	cstr txt_s =  "";
	float bh_f = 0;
	float bw_f = 0;
	cstr x1_s =  "";
	cstr x2_s =  "";
	cstr y1_s =  "";
	cstr y2_s =  "";
	int pos = 0;
	int n = 0;
	png_s=name_s;
	png_s += ".png";
	txt_s=name_s;
	txt_s += "-Subimages.fnt";
	if ( ImageExist(g.bitmapfontimagetart+index) == 0 ) 
	{
		LoadImage (  png_s.Get(),g.bitmapfontimagetart+index );
		OpenToRead (  1,txt_s.Get() );
		while (  FileEnd(1) == 0 ) 
		{
			line_s = ReadString (  1 );
			for ( n=1 ; n <= Len(line_s.Get()) ; n++ ) { if ( line_s.Get()[n-1] == ':' ) { pos=n-1 ; n=Len(line_s.Get()) ; } }
			char_s=Left(line_s.Get(),pos); line_s=Right(line_s.Get(),Len(line_s.Get())-(pos+1));
			for ( n=1 ; n <= Len(line_s.Get()) ; n++ ) { if ( line_s.Get()[n-1] == ':' ) { pos=n-1 ; n=Len(line_s.Get()) ; } }
			x1_s=Left(line_s.Get(),pos)  ; line_s=Right(line_s.Get(),Len(line_s.Get())-(pos+1));
			for ( n=1 ; n <= Len(line_s.Get()) ; n++ ) { if ( line_s.Get()[n-1] == ':' ) { pos=n-1 ; n=Len(line_s.Get()) ; } }
			y1_s=Left(line_s.Get(),pos) ; line_s=Right(line_s.Get(),Len(line_s.Get())-(pos+1));
			pos=Len(line_s.Get());
			for ( n=1 ; n <= Len(line_s.Get()) ; n++ ) { if ( line_s.Get()[n-1] == ':' ) { pos=n-1 ; n=Len(line_s.Get()) ; } }
			x2_s=Left(line_s.Get(),pos) ; line_s=Right(line_s.Get(),Len(line_s.Get())-(pos+1));
			for ( n=1 ; n <= Len(line_s.Get()) ; n++ ) { if ( line_s.Get()[n-1] == ':' ) { pos=n-1 ; n=Len(line_s.Get()) ; } }
			y2_s=line_s;
			charindex=ValF(char_s.Get());
			bx1_f=ValF(x1_s.Get());
			by1_f=ValF(y1_s.Get());
			bw_f=ValF(x2_s.Get());
			bh_f=ValF(y2_s.Get());
			if (  bh_f == 0  )  bh_f = lastbh_f;
			if (  lastbh_f == 0  )  lastbh_f = bh_f;
			bx2_f=bx1_f+bw_f;
			by2_f=by1_f+bh_f;
			t.bitmapfont[index][charindex].x1=bx1_f/(ImageWidth(g.bitmapfontimagetart+index)+0.0);
			t.bitmapfont[index][charindex].y1=by1_f/(ImageHeight(g.bitmapfontimagetart+index)+0.0);
			t.bitmapfont[index][charindex].x2=bx2_f/(ImageWidth(g.bitmapfontimagetart+index)+0.0);
			t.bitmapfont[index][charindex].y2=by2_f/(ImageHeight(g.bitmapfontimagetart+index)+0.0);
			t.bitmapfont[index][charindex].w=bw_f;
			t.bitmapfont[index][charindex].h=bh_f;
		}
		CloseFile (  1 );
	}
}

void shrinkbitmapfont ( char* name_s, int index, int fromindex )
{
	int charindex = 0;
	cstr png_s =  "";
	if (  ImageExist(g.bitmapfontimagetart+index) == 0 ) 
	{
		png_s=name_s;
		png_s+=".png";
		LoadImage (  png_s.Get(),g.bitmapfontimagetart+index );
		for ( charindex = 0 ; charindex<=  255; charindex++ )
		{
			t.bitmapfont[index][charindex].x1=t.bitmapfont[fromindex][charindex].x1;
			t.bitmapfont[index][charindex].y1=t.bitmapfont[fromindex][charindex].y1;
			t.bitmapfont[index][charindex].x2=t.bitmapfont[fromindex][charindex].x2;
			t.bitmapfont[index][charindex].y2=t.bitmapfont[fromindex][charindex].y2;
			t.bitmapfont[index][charindex].w=t.bitmapfont[fromindex][charindex].w/2;
			t.bitmapfont[index][charindex].h=t.bitmapfont[fromindex][charindex].h/2;
		}
	}
//endfunction

}

void pastebitmapfont ( char* text_s, int tx, int ty, int index, int alpha )
{
	int charindex = 0;
	float u1_f = 0;
	float u2_f = 0;
	float v1_f = 0;
	float v2_f = 0;
	int sid = 0;
	int n = 0;
	//  text$
	sid=g.bitmapfontimagetart+index;
	if (  sid  <=  0  )  return;
	if (  ImageExist(sid)  ==  0  )  return;
	float fImageTexOffsetU = 0.5f/ImageWidth(sid);
	float fImageTexOffsetV = 0.5f/ImageHeight(sid);
	Sprite (  sid,-10000,-10000,g.bitmapfontimagetart+index );
	for ( n = 1 ; n <= Len(text_s); n++ )
	{
		charindex=Asc(Mid(text_s,n));
		u1_f=t.bitmapfont[index][charindex].x1;
		v1_f=t.bitmapfont[index][charindex].y1;
		u2_f=t.bitmapfont[index][charindex].x2;
		v2_f=t.bitmapfont[index][charindex].y2;
		SizeSprite ( sid, t.bitmapfont[index][charindex].w,t.bitmapfont[index][charindex].h );
		SetSpriteTextureCoordinates ( sid, 0, u1_f+fImageTexOffsetU, v1_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 1, u2_f+fImageTexOffsetU, v1_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 2, u1_f+fImageTexOffsetU, v2_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 3, u2_f+fImageTexOffsetU, v2_f+fImageTexOffsetV );
		SetSpriteAlpha (  sid,alpha );
		PasteSprite (  sid,tx,ty );
		tx += t.bitmapfont[index][charindex].w;
	}
}


void pastebitmapfontcenter ( char* text_s, int tx, int ty, int index, int alpha )
{
	int charindex = 0;
	int twidth = 0;
	int sid = 0;
	int n = 0;
	sid=g.bitmapfontimagetart+index;
	if (  sid  <=  0  )  return;
	if (  ImageExist(sid)  ==  0  )  return;
	twidth=0;
	for ( n = 1 ; n <= Len(text_s); n++ )
	{
		charindex=Asc(Mid(text_s,n) );
		twidth += t.bitmapfont[index][charindex].w;
	}
	pastebitmapfont(text_s,tx-(twidth/2),ty,index,alpha);
}

void pastebitmapfontcenterwithboxout ( char* text_s, int tx, int ty, int index, int alpha )
{
	// as above but place a grey semi-transparent box behind it (later)
	pastebitmapfontcenter ( text_s, tx, ty, index, alpha );
}

int getbitmapfontwidth ( char* text_s, int index )
{
	int charindex = 0;
	int twidth=0;
	int n;
	for ( n = 1 ; n <= Len(text_s); n++ )
	{
		charindex=Asc(Mid(text_s,n) );
		twidth += t.bitmapfont[index][charindex].w;
	}
//endfunction twidth
	return twidth;
}

int getbitmapfontheight ( int index )
{
	int charindex=Asc("A");
	return t.bitmapfont[index][charindex].h;
}

void pastebitmapfontcolor ( char* text_s, int tx, int ty, int index, int alpha, int r, int gg, int b )
{
	int charindex = 0;
	float u1_f = 0;
	float u2_f = 0;
	float v1_f = 0;
	float v2_f = 0;
	int sid = 0;
	int n;
	//  text$
	sid = g.bitmapfontimagetart+index;
	if (  sid  <=  0  )  return;
	if (  ImageExist(sid)  ==  0  )  return;
	float fImageTexOffsetU = 0.5f/ImageWidth(sid);
	float fImageTexOffsetV = 0.5f/ImageHeight(sid);
	#ifdef VRTECH
	Sprite (  sid,-10000,-10000,g.bitmapfontimagetart+index );
	#else
	Sprite ( sid, -10000, -10000, sid );
	#endif
	for ( n = 1 ; n<=  Len(text_s); n++ )
	{
		charindex=Asc(Mid(text_s,n) );
		u1_f=t.bitmapfont[index][charindex].x1;
		v1_f=t.bitmapfont[index][charindex].y1;
		u2_f=t.bitmapfont[index][charindex].x2;
		v2_f=t.bitmapfont[index][charindex].y2;
		SizeSprite (  sid,t.bitmapfont[index][charindex].w,t.bitmapfont[index][charindex].h );
		SetSpriteTextureCoordinates ( sid, 0, u1_f+fImageTexOffsetU, v1_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 1, u2_f+fImageTexOffsetU, v1_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 2, u1_f+fImageTexOffsetU, v2_f+fImageTexOffsetV );
		SetSpriteTextureCoordinates ( sid, 3, u2_f+fImageTexOffsetU, v2_f+fImageTexOffsetV );

		SetSpriteDiffuse (  sid,r,gg,b );
		SetSpriteAlpha (  sid,alpha );
		PasteSprite (  sid,tx,ty );
		tx += t.bitmapfont[index][charindex].w;
	}
	SetSpriteDiffuse (  sid,255,255,255 );
}

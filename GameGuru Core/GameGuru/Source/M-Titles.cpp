//----------------------------------------------------
//--- GAMEGURU - M-Titles
//----------------------------------------------------

// Includes
#include "stdafx.h"
#include "gameguru.h"
#include "io.h"

// Defines
#ifdef VRTECH
#define ENABLEMPAVATAR
#endif

// External error helper
extern char g_strErrorClue[512];

// 
//  OBS Obstacle Generation Module
// 

void titles_init ( void )
{
	// determine where 'titles' LUA files will be loaded from
	titles_getstyle ( );

	// safe write folder
	if ( PathExist(g.myownrootdir_s.Get()) == 0 ) file_createmydocsfolder ( );

	#ifdef VRTECH
	// load backdrop used in titles
	if ( g.vrqcontrolmode != 0 )
	{
		cstr pPath = cstr("languagebank\\")+g.language_s+cstr("\\artwork\\socialvr.png");
		LoadImage ( pPath.Get(), g.editorimagesoffset+64 );
	}
	#endif

	/* redundant now LUA in control of titles system
	//  determine the resolution we should use
	t.tclosest=9999999;
	t.tclosestreswidth=0 ; t.tclosestresheight=0;
	for ( t.tres = 1 ; t.tres<=  8; t.tres++ )
	{
		if (  t.tres == 1 ) { t.treswidth = 1280  ; t.tresheight = 720; }
		if (  t.tres == 2 ) { t.treswidth = 1280  ; t.tresheight = 800; }
		if (  t.tres == 3 ) { t.treswidth = 1366  ; t.tresheight = 768; }
		if (  t.tres == 4 ) { t.treswidth = 1440  ; t.tresheight = 900; }
		if (  t.tres == 5 ) { t.treswidth = 1600  ; t.tresheight = 900; }
		if (  t.tres == 6 ) { t.treswidth = 1680  ; t.tresheight = 1050; }
		if (  t.tres == 7 ) { t.treswidth = 1920  ; t.tresheight = 1080; }
		if (  t.tres == 8 ) { t.treswidth = 1920  ; t.tresheight = 1200; }
		t.tdiff=abs(GetDisplayWidth()-t.treswidth)+abs(GetDisplayHeight()-t.tresheight);
		if (  t.tdiff<t.tclosest ) 
		{
			t.tclosest=t.tdiff;
			t.tclosestreswidth=t.treswidth;
			t.tclosestresheight=t.tresheight;
		}
	}
	t.resfolder_s = "";t.resfolder_s=t.resfolder_s+Str(t.tclosestreswidth)+"x"+Str(t.tclosestresheight)+"\\";

	//  Separate resolution options for MP backdrop image
	t.tmpclosest=9999999;
	t.tmpclosestreswidth=0 ; t.tmpclosestresheight=0;
	for ( t.tres = 1 ; t.tres<=  13; t.tres++ )
	{
		if (  t.tres == 1 ) { t.treswidth = 640  ; t.tresheight = 480; }
		if (  t.tres == 2 ) { t.treswidth = 1024 ; t.tresheight = 768; }
		if (  t.tres == 3 ) { t.treswidth = 1152 ; t.tresheight = 864; }
		if (  t.tres == 4 ) { t.treswidth = 1280 ; t.tresheight = 720; }
		if (  t.tres == 5 ) { t.treswidth = 1280 ; t.tresheight = 800; }
		if (  t.tres == 6 ) { t.treswidth = 1280 ; t.tresheight = 960; }
		if (  t.tres == 7 ) { t.treswidth = 1366 ; t.tresheight = 768; }
		if (  t.tres == 8 ) { t.treswidth = 1440 ; t.tresheight = 900; }
		if (  t.tres == 9 ) { t.treswidth = 1600 ; t.tresheight = 900; }
		if (  t.tres == 10 ) { t.treswidth = 1600 ; t.tresheight = 1200; }
		if (  t.tres == 11 ) { t.treswidth = 1680 ; t.tresheight = 1050; }
		if (  t.tres == 12 ) { t.treswidth = 1920 ; t.tresheight = 1080; }
		if (  t.tres == 13 ) { t.treswidth = 1920 ; t.tresheight = 1200; }
		t.tdiff=abs(GetDisplayWidth()-t.treswidth)+abs(GetDisplayHeight()-t.tresheight);
		if (  t.tdiff<t.tmpclosest ) 
		{
			t.tmpclosest=t.tdiff;
			t.tmpclosestreswidth=t.treswidth;
			t.tmpclosestresheight=t.tresheight;
		}
	}
	t.tmpres_s = "" ;t.tmpres_s=t.tmpres_s + Str(t.tmpclosestreswidth)+"x"+Str(t.tmpclosestresheight);

	//  Load all titles images
	t.strwork = ""; t.strwork = t.strwork + "titlesbank\\"+t.ttheme_s+"\\"+t.resfolder_s+t.tthemeprefix_s+"title.jpg";
	LoadImage ( t.strwork.Get(), g.titlesimageoffset+0 );
	t.tfile_s=""; t.tfile_s=t.tfile_s+"languagebank\\neutral\\artwork\\MPbackground-"+t.tmpres_s+".png";
	if (  FileExist(t.tfile_s.Get()) == 1  )  LoadImage (  t.tfile_s.Get(),g.titlesimageoffset+10 );
	LoadImage (  "titlesbank\\default\\start.png",g.titlesimageoffset+1 );
	LoadImage (  "titlesbank\\default\\start-hover.png",g.titlesimageoffset+2 );
	LoadImage (  "titlesbank\\default\\quit-game.png",g.titlesimageoffset+3 );
	LoadImage (  "titlesbank\\default\\quit-game-hover.png",g.titlesimageoffset+4 );
	LoadImage (  "titlesbank\\default\\visit-website.png",g.titlesimageoffset+6 );
	LoadImage (  "titlesbank\\default\\visit-website-hover.png",g.titlesimageoffset+7 );
	LoadImage (  "titlesbank\\default\\about.png",g.titlesimageoffset+8 );
	LoadImage (  "titlesbank\\default\\about-hover.png",g.titlesimageoffset+9 );

	t.strwork = ""; t.strwork = t.strwork +"titlesbank\\"+t.ttheme_s+"\\"+t.resfolder_s+t.tthemeprefix_s+"loading.jpg";
	LoadImage (  t.strwork.Get() ,g.titlesimageoffset+14 );
	LoadImage (  "titlesbank\\default\\loading-text.png",g.titlesimageoffset+15 ) ;
	LoadImage (  "titlesbank\\default\\loading-bar-empty.png",g.titlesimageoffset+16 );
	LoadImage (  "titlesbank\\default\\loading-bar-full.png",g.titlesimageoffset+17 );
	LoadImage (  "titlesbank\\default\\loading-dot.png",g.titlesimageoffset+18 ) ;

	LoadImage (  "titlesbank\\default\\game-paused-title.png",g.titlesimageoffset+21 );
	LoadImage (  "titlesbank\\default\\resume-game.png",g.titlesimageoffset+23 );
	LoadImage (  "titlesbank\\default\\resume-game-hover.png",g.titlesimageoffset+24 );
	LoadImage (  "titlesbank\\default\\graphics-settings.png",g.titlesimageoffset+25 );
	LoadImage (  "titlesbank\\default\\graphics-settings-hover.png",g.titlesimageoffset+26 );
	LoadImage (  "titlesbank\\default\\sound-levels.png",g.titlesimageoffset+27 );
	LoadImage (  "titlesbank\\default\\sound-levels-hover.png",g.titlesimageoffset+28 );

	LoadImage (  "titlesbank\\default\\sound-levels-title.png",g.titlesimageoffset+31 );
	LoadImage (  "titlesbank\\default\\sound-effects-label.png",g.titlesimageoffset+32 );
	LoadImage (  "titlesbank\\default\\music-label.png",g.titlesimageoffset+33 );
	LoadImage (  "titlesbank\\default\\slider-bar-empty.png",g.titlesimageoffset+34 );
	LoadImage (  "titlesbank\\default\\slider-bar-full.png",g.titlesimageoffset+35 );
	LoadImage (  "titlesbank\\default\\slider-bar-dot.png",g.titlesimageoffset+36 );
	LoadImage (  "titlesbank\\default\\main-menu.png",g.titlesimageoffset+37 );
	LoadImage (  "titlesbank\\default\\main-menu-hover.png",g.titlesimageoffset+38 );

	t.strwork = ""; t.strwork = t.strwork + "titlesbank\\"+t.ttheme_s+"\\"+t.resfolder_s+t.tthemeprefix_s+"lost.jpg";
	LoadImage (  t.strwork.Get(),g.titlesimageoffset+39 );
	t.strwork = ""; t.strwork = t.strwork + "titlesbank\\"+t.ttheme_s+"\\"+t.resfolder_s+t.tthemeprefix_s+"end.jpg";
	LoadImage ( t.strwork.Get() ,g.titlesimageoffset+40 );
	LoadImage (  "titlesbank\\default\\level-complete-title.png",g.titlesimageoffset+41 );
	LoadImage (  "titlesbank\\default\\game-won-title.png",g.titlesimageoffset+42 );
	LoadImage (  "titlesbank\\default\\back.png",g.titlesimageoffset+43 );
	LoadImage (  "titlesbank\\default\\back-hover.png",g.titlesimageoffset+44 );
	LoadImage (  "titlesbank\\default\\continue.png",g.titlesimageoffset+45 );
	LoadImage (  "titlesbank\\default\\continue-hover.png",g.titlesimageoffset+46 );
	LoadImage (  "titlesbank\\default\\fov.png",g.titlesimageoffset+47 );

	LoadImage (  "titlesbank\\default\\graphics-settings.png",g.titlesimageoffset+51 );
	LoadImage (  "titlesbank\\default\\graphics-settings-hover.png",g.titlesimageoffset+52 );
	LoadImage (  "titlesbank\\default\\graphics-settings-title.png",g.titlesimageoffset+53 );
	LoadImage (  "titlesbank\\default\\lowest.png",g.titlesimageoffset+54 );
	LoadImage (  "titlesbank\\default\\lowest-hover.png",g.titlesimageoffset+55 );
	LoadImage (  "titlesbank\\default\\lowest-selected.png",g.titlesimageoffset+56 );
	LoadImage (  "titlesbank\\default\\medium.png",g.titlesimageoffset+57 );
	LoadImage (  "titlesbank\\default\\medium-hover.png",g.titlesimageoffset+58 );
	LoadImage (  "titlesbank\\default\\medium-selected.png",g.titlesimageoffset+59 );
	LoadImage (  "titlesbank\\default\\highest.png",g.titlesimageoffset+60 );
	LoadImage (  "titlesbank\\default\\highest-hover.png",g.titlesimageoffset+61 );
	LoadImage (  "titlesbank\\default\\highest-selected.png",g.titlesimageoffset+62 );

	t.strwork = ""; t.strwork = t.strwork +"titlesbank\\"+t.ttheme_s+"\\"+t.resfolder_s+t.tthemeprefix_s+"about.jpg";
	LoadImage ( t.strwork.Get() ,g.titlesimageoffset+66 );

	//  Load title audio
	if (  SoundExist(g.titlessoundoffset+1) == 1  )  DeleteSound (  g.titlessoundoffset+1 );
	t.strwork = ""; t.strwork = t.strwork + "titlesbank\\"+t.ttheme_s+"\\title.ogg";
	if (  FileExist( t.strwork.Get() ) == 1 ) 
	{
		t.strwork = ""; t.strwork = t.strwork + "titlesbank\\"+t.ttheme_s+"\\title.ogg";
		LoadSound ( t.strwork.Get() , g.titlessoundoffset+1 );
	}

	//  Sound Volume defaults
	t.gamesounds.sounds=100;
	t.gamesounds.music=100;
	t.gamesounds.titlemusicvolume = 0;

	//  Load any settings if present
	titles_load ( );

	//  flags for titles system
	t.game.quitflag=0;
	*/
}

void titles_getstyle ( void )
{
	t.ttheme_s="default" ; t.tthemeprefix_s="";
	t.tfile_s="titlesbank\\style.txt";
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		OpenToRead ( 1 , t.tfile_s.Get() );
		t.ttheme_s = ReadString ( 1 );
		t.tthemeprefix_s = ReadString ( 1 );
		CloseFile (  1 );
	}
}

void titles_save ( void )
{
	/* 080216 superceded with LUA system now
	//  save settings from all title menus
	t.tfile_s=g.myownrootdir_s+"\\"+g.titlessavefile_s;
	t.ttitleversion=101;
	if (  FileExist(t.tfile_s.Get()) == 1  )  DeleteAFile (  t.tfile_s.Get() );
	OpenToWrite (  1,t.tfile_s.Get() );
	WriteLong (  1,t.ttitleversion );
	WriteLong (  1,t.gamesounds.sounds );
	WriteLong (  1,t.gamesounds.music );
	WriteLong (  1,g.titlesettings.graphicsettingslevel );
	CloseFile (  1 );
	*/
}

void titles_load ( void )
{
	/* 080216 superceded with LUA system now
	//  load settings for all title menus
	t.tfile_s=g.myownrootdir_s+"\\"+g.titlessavefile_s;
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		OpenToRead (  1,t.tfile_s.Get() );
		t.ttitleversion = ReadLong ( 1 );
		if (  t.ttitleversion == 101 ) 
		{
			t.a = ReadLong ( 1 ); t.gamesounds.sounds=t.a;
			t.a = ReadLong ( 1 ); t.gamesounds.music=t.a;
			t.a = ReadLong ( 1 ); g.titlesettings.graphicsettingslevel=t.a;
		}
		CloseFile (  1 );
	}
	*/
}

void titles_immediateupdatesound ( void )
{
	//  make audio settings take effect
	t.audioVolume.sound=t.gamesounds.sounds;
	t.audioVolume.music=t.gamesounds.music;
	audio_volume_update ( );
}

int titles_getvaluefromgfxname ( int iCurrentValue, int iTerrEntGra, LPSTR pGfxName )
{
	int iValue = iCurrentValue;
	if ( iTerrEntGra==1 )
	{
		// terrain
		if ( stricmp ( pGfxName, "highest" )==NULL ) iValue = 1;
		if ( stricmp ( pGfxName, "high" )==NULL ) iValue = 2;
		if ( stricmp ( pGfxName, "medium" )==NULL ) iValue = 3;
		if ( stricmp ( pGfxName, "lowest" )==NULL ) iValue = 4;
	}
	if ( iTerrEntGra==2 )
	{
		// entity
		if ( stricmp ( pGfxName, "highest" )==NULL ) iValue = 1;
		if ( stricmp ( pGfxName, "medium" )==NULL ) iValue = 2;
		if ( stricmp ( pGfxName, "lowest" )==NULL ) iValue = 3;
	}
	if ( iTerrEntGra==3 )
	{
		// grass
		if ( stricmp ( pGfxName, "highest" )==NULL ) iValue = 1;
		if ( stricmp ( pGfxName, "high" )==NULL ) iValue = 2;
		if ( stricmp ( pGfxName, "medium" )==NULL ) iValue = 3;
		if ( stricmp ( pGfxName, "lowest" )==NULL ) iValue = 4;
	}
	return iValue;
}

void titles_immediateupdategraphics ( void )
{
	// have default values for HIGH, MEDIUM and LOW
	// but can overrride with [GAMEMENUOPTIONS] settings
	int iTerrainLow = 4, iTerrainMed = 3, iTerrainHigh = 1;
	int iEntityLow = 3, iEntityMed = 2, iEntityHigh = 1;
	int iGrassLow = 3, iGrassMed = 2, iGrassHigh = 1;
	iTerrainLow = titles_getvaluefromgfxname ( iTerrainLow, 1, g.graphicslowterrain_s.Get() );
	iTerrainMed = titles_getvaluefromgfxname ( iTerrainMed, 1, g.graphicsmediumterrain_s.Get() );
	iTerrainHigh = titles_getvaluefromgfxname ( iTerrainHigh, 1, g.graphicshighterrain_s.Get() );
	iEntityLow = titles_getvaluefromgfxname ( iEntityLow, 2, g.graphicslowentity_s.Get() );
	iEntityMed = titles_getvaluefromgfxname ( iEntityMed, 2, g.graphicsmediumentity_s.Get() );
	iEntityHigh = titles_getvaluefromgfxname ( iEntityHigh, 2, g.graphicshighentity_s.Get() );
	iGrassLow = titles_getvaluefromgfxname ( iGrassLow, 3, g.graphicslowgrass_s.Get() );
	iGrassMed = titles_getvaluefromgfxname ( iGrassMed, 3, g.graphicsmediumgrass_s.Get() );
	iGrassHigh = titles_getvaluefromgfxname ( iGrassHigh, 3, g.graphicshighgrass_s.Get() );

	//  make graphic settings take effect
	if (  g.titlesettings.graphicsettingslevel == 1 ) 
	{
		//  LOWEST
		t.visuals.shaderlevels.terrain = iTerrainLow;
		t.visuals.shaderlevels.entities = iEntityLow;
		t.visuals.shaderlevels.vegetation = iGrassLow;
		//  Restore to regular transition multiplier
		t.visuals.DistanceTransitionMultiplier_f=1.0;
	}
	if (  g.titlesettings.graphicsettingslevel == 2 ) 
	{
		//  MEDIUM
		t.visuals.shaderlevels.terrain = iTerrainMed;
		t.visuals.shaderlevels.entities = iEntityMed;
		t.visuals.shaderlevels.vegetation = iGrassMed;
		//  Restore to regular transition multiplier
		t.visuals.DistanceTransitionMultiplier_f=1.0;
	}
	if (  g.titlesettings.graphicsettingslevel == 3 ) 
	{
		//  HIGHEST
		t.visuals.shaderlevels.terrain = iTerrainHigh;
		t.visuals.shaderlevels.entities = iEntityHigh;
		t.visuals.shaderlevels.vegetation = iGrassHigh;
		//  Up the transition values too
		t.visuals.DistanceTransitionMultiplier_f=10.0;
	}
	//  update if TAB mode slides available
	t.slidersmenuindex=t.slidersmenunames.shaderoptions;
	t.slidersmenuvalue[t.slidersmenuindex][1].value=t.visuals.shaderlevels.terrain;
	t.slidersmenuvalue[t.slidersmenuindex][2].value=t.visuals.shaderlevels.entities;
	t.slidersmenuvalue[t.slidersmenuindex][3].value=t.visuals.shaderlevels.vegetation;
	//  update strings in slider data
	for ( t.tn = 1 ; t.tn<=  4; t.tn++ )
	{
		t.slidersmenuindex=t.slidersmenunames.shaderoptions;
		t.slidersmenuvaluechoice=t.slidersmenuvalue[t.slidersmenuindex][t.tn].gadgettypevalue;
		t.slidersmenuvalueindex=t.slidersmenuvalue[t.slidersmenuindex][t.tn].value;
		sliders_getnamefromvalue ( );
		t.slidersmenuvalue[t.slidersmenuindex][t.tn].value_s=t.slidervaluename_s;
	}
	//  call visuals to update shaders
	//  need to update any shadow elements when change technique
	if (  g.titlesettings.updateshadowsaswell == 1 ) 
	{
		//  if set to one, will leave options menu and return
		#ifdef VRTECH
		terrain_shadowupdate ( );
		#else
		if (g.globals.speedshadows > 0)
		{
			//Update all cascades.
			int gss = g.globals.speedshadows;
			g.globals.speedshadows = 0;
			terrain_shadowupdate();
			terrain_update();
			g.globals.speedshadows = gss;
		}
		else
			terrain_shadowupdate();
		#endif
		t.visuals.refreshshaders=1;
		visuals_loop ( );
		visuals_shaderlevels_update ( );
		t.visuals.refreshshaders=1;
		#ifdef VRTECH
		#else
		Sync();
		#endif
	}
}

void titles_titlepage ( void )
{
	t.titlespage=1;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	t.titlesbar[t.titlespage][g.titlesbarmax].fill=0;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+1;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+2;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="START";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-480.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-480.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="-1";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-56.0*t.tva_f)-24;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-56.0*t.tva_f)+24;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+8;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+9;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="ABOUT";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-390.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-390.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+3;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+4;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="EXIT";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-300.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-300.0*t.tva_f)+t.timghig;
	fadetitlesmusic(100);
	titles_base ( );
	fadetitlesmusic(0);
	//  wait until title music finished fading before quitting
	while (  t.gamesounds.titlemusicvolume > 0 ) 
	{
		handletitlesmusic();
	}
}

void titles_aboutpage ( void )
{
	t.titlespage=6;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+43;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+44;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="BACK";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-80.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-80.0*t.tva_f)+t.timghig;
	fadetitlesmusic(100);
	titles_base ( );
	fadetitlesmusic(0);

	//  wait until title music finished fading before quitting
	while (  t.gamesounds.titlemusicvolume > 0 ) 
	{
		handletitlesmusic();
	}
}

void titles_loadingpage ( void )
{
	t.titlespage=2;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=(GetDisplayHeight()/2)-40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+14;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	t.titlesbar[t.titlespage][g.titlesbarmax].fill=0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+15;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(GetDisplayHeight()-80.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(GetDisplayHeight()-80.0*t.tva_f)+t.timghig;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=1;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+16;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].fill=0;
	titles_base ( );
}

void titles_loadingpageinit ( void )
{
	// before loading LUA page created, reset resource array
	t.loadingresource[0] = 0;
}

void titles_loadingpageupdate ( void )
{
	// 090216 - replaced with LUA system
	//t.titlesbar[t.titlespage][g.titlesbarmax].fill=t.game.levelloadprogress;
	//titles_base ( );
	if ( t.loadingresource[0] > 0 )
	{
		int iSpriteIndex = t.loadingresource[1];
		if ( SpriteExist ( iSpriteIndex )==1 )
		{
			int iImageIndex = GetSpriteImage(iSpriteIndex);
			if ( ImageExist ( iImageIndex )==1 )
			{
				int iProgressWidth = (ImageWidth(iImageIndex)/100)*t.game.levelloadprogress;
				SizeSprite ( iSpriteIndex, iProgressWidth, ImageHeight(iImageIndex) );
				for ( int iItemToDraw = t.loadingresource[0] ; iItemToDraw > 0 ; iItemToDraw-- )
				{
					iSpriteIndex = t.loadingresource[iItemToDraw];
					PasteSprite(iSpriteIndex, SpriteX(iSpriteIndex), SpriteY(iSpriteIndex) );
				}
			}
		}
	}

	//  dave added a skip test for syncing to prevent the editor being drawn when switching to mp game start
	if (  Timer() - t.tskipLevelSync > 200  )  Sync (  );
}

void titles_loadingpagefree ( void )
{
	// specified inside LUA script describing loading page
	if ( t.loadingresource[0] > 0 )
	{
		for ( int iItemToDelete = t.loadingresource[0]; iItemToDelete > 0 ; iItemToDelete-- )
		{
			int iSpriteIndex = t.loadingresource[iItemToDelete];
			int iImageIndex = GetSpriteImage(iSpriteIndex);
			DeleteSprite ( iSpriteIndex );
			DeleteImage ( iImageIndex );
		}
		t.loadingresource[0] = 0;
	}
}

void titles_completepage ( void )
{
	t.titlespage=3;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+41;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(80.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(80.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+45;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+46;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CONTINUE";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-80.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-80.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_gamewonpage ( void )
{
	t.titlespage=4;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+40;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	t.titlesbar[t.titlespage][g.titlesbarmax].fill=0;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+45;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+46;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CONTINUE";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-80.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-80.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_gamelostpage ( void )
{
	t.titlespage=5;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+39;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	t.titlesbar[t.titlespage][g.titlesbarmax].fill=0;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+45;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+46;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CONTINUE";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-80.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-80.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steampage ( void )
{
	// grab avatar info so we only have to do it once
	#ifdef VRTECH
	t.tShowAvatarSprite = 0;
	#ifdef ENABLEMPAVATAR
	t.tShowAvatarSprite = 1;
	characterkitplus_loadMyAvatarInfo();
	#endif
	#else
	t.tShowAvatarSprite = 1;
	characterkit_loadMyAvatarInfo ( );
	#endif

	InkEx ( 255, 255, 255 );
	t.titlespage=7;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();

	#ifdef VRTECH
	// Add multiplayer title text
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=2;
	#ifdef PRODUCTV3
	 strcpy ( t.titlesbar[t.titlespage][g.titlesbarmax].text, "Social VR" );
	#else
	 strcpy ( t.titlesbar[t.titlespage][g.titlesbarmax].text, "Multiplayer" );
	#endif
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=GetDisplayWidth()/2;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=GetDisplayHeight()*0.1f;
	#endif

	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="HOST A LEVEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-400.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-400.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	#ifdef PHOTONMP
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="SEARCH FOR GAMES";
	#else
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="SEARCH FOR LOBBIES";
	#endif
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-350.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-350.0*t.tva_f)+t.timghig;

	// BACK
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="BACK";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-300.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-300.0*t.tva_f)+t.timghig;

	#ifdef VRTECH
	// Add TEACHER CODE button (for extra functionalities; such as viewing ALL games across VRQ server)
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="ENTER TEACHER CODE";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-200.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-200.0*t.tva_f)+t.timghig;
	#endif

	// add another button if the user has character creator entities made
	#ifdef ENABLEMPAVATAR
	if (CharacterKitCheckForUserMade() == 1)
	{
		++g.titlesbuttonmax;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].img = 0;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh = 0;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s = "CHOOSE/CHANGE ONLINE AVATAR";
		t.timgwid = 200; t.timghig = 16;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1 = (GetDisplayWidth() / 2) - t.timgwid;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2 = (GetDisplayWidth() / 2) + t.timgwid;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1 = (GetDisplayHeight() - 150.0*t.tva_f) - t.timghig;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2 = (GetDisplayHeight() - 150.0*t.tva_f) + t.timghig;
	}
	#endif
	titles_base ( );
}

void titles_steamCreateLobby ( void )
{
	t.titlespage=8;
	#ifdef VRTECH
	t.titlesname_s="";//"My Lobby";
	#else
	t.titlesname_s="My Lobby";
	#endif
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=1;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="START LEVEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-90.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-90.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	#ifdef PHOTONMP
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="LEAVE LEVEL";
	#else
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="LEAVE LOBBY";
	#endif
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamSearchLobbies ( void )
{
	t.titlespage=9;
	#ifdef VRTECH
	t.titlesname_s="";//Searching for a lobby to join"; prevent overwriting other title
	#else
	t.titlesname_s="Searching for a lobby to join";
	#endif
	
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	#ifdef PHOTONMP
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="JOIN LEVEL";
	#else
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="JOIN LOBBY";
	#endif
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-150.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-150.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CANCEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamInLobbyGuest ( void )
{
	t.titlespage=10;
	#ifdef VRTECH
	t.titlesname_s="";//In Lobby";
	#else
	t.titlesname_s="In Lobby";
	#endif
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	#ifdef PHOTONMP
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="LEAVE LEVEL";
	#else
	 t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="LEAVE LOBBY";
	#endif
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_optionspage ( void )
{
	t.titlesalreadyinoptionsloop=0;
	if (  t.titlespage >= 11 && t.titlespage <= 20 ) 
	{
		t.titlesalreadyinoptionsloop=1;
	}
	t.titlespage=11;
	t.titlesclearmode=0;
	t.titlesname_s="";
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=1;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+21;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(80.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(80.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+23;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+24;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="RESUME LEVEL";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-200.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-200.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+51;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+52;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="GRAPHICS SETTINGS";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-400.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-400.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+27;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+28;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="SOUND LEVELS";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-300.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-300.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	if (  t.game.runasmultiplayer == 1 ) 
	{
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+3;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+4;
	}
	else
	{
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+37;
		t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+38;
	}
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="EXIT";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-500.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-500.0*t.tva_f)+t.timghig;
	if (  t.titlesalreadyinoptionsloop == 0 ) 
	{
		titles_base ( );
	}
}

void titles_graphicssettings ( void )
{
	t.titlespage=12;
	t.titlesname_s="";
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=1;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+53;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(80.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(80.0*t.tva_f)+t.timghig;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+47;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(GetDisplayHeight()-320.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(GetDisplayHeight()-320.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+54;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+55;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="LOWEST";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=2;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmodevalue=1;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-580.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-580.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+57;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+58;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="MEDIUM";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=2;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmodevalue=2;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-500.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-500.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+60;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+61;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="HIGHEST";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=2;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmodevalue=3;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-420.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-420.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+34;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="FOV";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=3;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].value=(((t.visuals.CameraFOV_f*t.visuals.CameraASPECT_f)-20.0)/180.0)*100.0;
	t.timgwid=(ImageWidth(g.titlesimageoffset+34)/2);
	t.timghig=(ImageHeight(g.titlesimageoffset+34)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-260.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-260.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+43;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+44;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="BACK";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-120.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-120.0*t.tva_f)+t.timghig;
}

void titles_soundlevels ( void )
{
	t.titlespage=13;
	t.titlesname_s="";
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=1;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+31;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(80.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(80.0*t.tva_f)+t.timghig;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+32;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(GetDisplayHeight()-550.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(GetDisplayHeight()-550.0*t.tva_f)+t.timghig;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+33;
	t.timgwid=(ImageWidth(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbar[t.titlespage][g.titlesbarmax].img)/2);
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=(GetDisplayHeight()-400.0*t.tva_f)-t.timghig;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=(GetDisplayHeight()-400.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+34;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="IN-GAME SOUNDS";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=1;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].value=t.gamesounds.sounds;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-500.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-500.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+34;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="IN-GAME MUSIC";
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].specialmode=1;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].value=t.gamesounds.music;
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-350.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-350.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=g.titlesimageoffset+43;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=g.titlesimageoffset+44;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="BACK";
	t.timgwid=(ImageWidth(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.timghig=(ImageHeight(t.titlesbutton[t.titlespage][g.titlesbuttonmax].img)/2);
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-200.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-200.0*t.tva_f)+t.timghig;
}

void titles_steamchoosetypeoflevel ( void )
{
	t.titlespage=14;
	t.titlesname_s="";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="PLAY ONE OF YOUR LEVELS";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-400.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-400.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="BACK";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-300.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-300.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamchoosefpmtouse ( void )
{
	t.titlespage=15;
	t.titlesname_s="";//"Choose a level to play";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CHOOSE LEVEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-150.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-150.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CANCEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamdoyouwanttocreateorupdateaworkshopitem ( void )
{
	t.titlespage=16;
	t.titlesname_s="Workshop item";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="YES, A WORKSHOP ITEM";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=((GetDisplayWidth()/4)*3)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=((GetDisplayWidth()/4)*3)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CANCEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/4)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/4)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="VIEW AGREEMENT";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/4*3)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/4*3)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-100.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-100.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamcreatingworkshopitem ( void )
{
	t.titlespage=17;
	t.titlesname_s="Workshop item";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="PLEASE WAIT";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/2)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/2)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamdoyouwanttosubscribetoworkshopitem ( void )
{
	t.titlespage=18;
	t.titlesname_s="Workshop item";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="YES, SUBSCRIBE";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=((GetDisplayWidth()/4)*3)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=((GetDisplayWidth()/4)*3)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="CANCEL";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=(GetDisplayWidth()/4)-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=(GetDisplayWidth()/4)+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_subscribetoworkshopitemwaitingforresult ( void )
{
	t.titlespage=19;
	t.titlesname_s="Workshop item";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="Back";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=((GetDisplayWidth()/2))-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=((GetDisplayWidth()/2))+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_steamdTellingToRestart ( void )
{
	t.titlespage=20;
	t.titlesname_s="Workshop item";
	t.titlesclearmode=1;
	t.titlesnamey=40;
	g.titlesbuttonmax=0;
	g.titlesbarmax=0;
	t.titlesgamerendering=0;
	t.tva_f=(GetDisplayHeight()+0.0)/768.0;
	++g.titlesbarmax;
	t.titlesbar[t.titlespage][g.titlesbarmax].mode=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].img=g.titlesimageoffset+66;
	t.titlesbar[t.titlespage][g.titlesbarmax].x1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].x2=GetDisplayWidth();
	t.titlesbar[t.titlespage][g.titlesbarmax].y1=0;
	t.titlesbar[t.titlespage][g.titlesbarmax].y2=GetDisplayHeight();
	++g.titlesbuttonmax;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].img=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].imghigh=0;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].name_s="Back";
	t.timgwid=200 ; t.timghig=16;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x1=((GetDisplayWidth()/2))-t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].x2=((GetDisplayWidth()/2))+t.timgwid;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y1=(GetDisplayHeight()-50.0*t.tva_f)-t.timghig;
	t.titlesbutton[t.titlespage][g.titlesbuttonmax].y2=(GetDisplayHeight()-50.0*t.tva_f)+t.timghig;
	titles_base ( );
}

void titles_base ( void )
{
	//  need mouse in menus
	if (  g.titlesbuttonmax>0 ) 
	{
		game_showmouse ( );
		++t.thaveShownMouse;
	}

	//  180315 - cleanup any sprites
	for ( t.s = 1 ; t.s<=  50000; t.s++ )
	{
		if (  SpriteExist(t.s) == 1 ) 
		{
			HideSprite (  t.s );
		}
	}

	//  titles loop
	t.game.titleloop=1;
	t.null=MouseMoveX()+MouseMoveY();
	while (  t.game.titleloop == 1 && g.titlesettings.updateshadowsaswell != 2 ) 
	{
		//  Handle titles music fading
		handletitlesmusic();
		bool usingDInput = false;
		//  Input source
		if (  t.game.gameisexe == 1 ) 
		{
			t.mc=MouseClick() ; t.mx=MouseX() ; t.my=MouseY();
		}
		else
		{
			#ifdef FPSEXCHANGE
			 OpenFileMap (  1, "FPSEXCHANGE" );
			 SetEventAndWait (  1 );
			 t.mx=GetFileMapDWORD( 1, 0 );
			 t.my=GetFileMapDWORD( 1, 4 );
			 #ifdef PRODUCTV3
			 // No expansion needed fo VRQ
			 #else
			 //PE: Need to expand mouse xy in classic.
			 t.mx=((t.mx+0.0)/800.0)*(GetDesktopWidth()+0.0);
			 t.my=((t.my+0.0)/600.0)*(GetDesktopHeight()+0.0);
			 #endif
			 t.mc=GetFileMapDWORD( 1, 20 );
			#else
			 t.mx = MouseX();
			 t.my = MouseY();
			 t.mc = MouseClick();
			#endif
		}

		// Display
		if ( t.titlesclearmode == 1 )  
		{
			CLS ( 0 );
			#ifdef VRTECH
			if ( g.vrqcontrolmode != 0 )
			{
				if ( ImageExist ( g.editorimagesoffset+64 ) )
				{
					Sprite ( 123, -100000, -100000, g.editorimagesoffset+64 );
					SizeSprite ( 123, GetDisplayWidth(), GetDisplayHeight() );
					PasteSprite ( 123, 0, 0 );
				}
			}
			#endif
		}
		pastebitmapfontcenter(t.titlesname_s.Get(),GetDisplayWidth()/2,t.titlesnamey,1,255);

		//  Draw bars
		t.p=t.titlespage;
		for ( t.b = 1 ; t.b<=  g.titlesbarmax; t.b++ )
		{
			if ( t.titlesbar[t.p][t.b].mode == 0 ) 
			{
				t.timg=t.titlesbar[t.p][t.b].img;
				if (  t.game.runasmultiplayer == 1 ) 
				{
					if (  t.timg == g.titlesimageoffset+66 && ImageExist(g.titlesimageoffset+10) == 1 ) 
					{
						t.timg=g.titlesimageoffset+10;
					}
				}
				//  020315 - 012 - Ensure the image is valid and exists
				if (  t.timg > 0 ) 
				{
					if (  ImageExist(t.timg) ) 
					{
						Sprite (  123,-100000,-100000,t.timg );
						SizeSprite (  123,t.titlesbar[t.p][t.b].x2-t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y2-t.titlesbar[t.p][t.b].y1 );
						PasteSprite (  123,t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1 );
					}
				}
			}
			if ( t.titlesbar[t.p][t.b].mode == 1 ) 
			{
				if (  t.titlesbar[t.p][t.b].img>0 ) 
				{
					//  empty bar
					Sprite (  123,-100000,-100000,t.titlesbar[t.p][t.b].img );
					SizeSprite (  123,t.titlesbar[t.p][t.b].x2-t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y2-t.titlesbar[t.p][t.b].y1 );
					PasteSprite (  123,t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1 );
					//  fill bar
					t.ttxx=((t.titlesbar[t.p][t.b].x2-t.titlesbar[t.p][t.b].x1)/100.0)*t.titlesbar[t.p][t.b].fill;
					Sprite (  123,-100000,-100000,t.titlesbar[t.p][t.b].img+1 );
					SizeSprite (  123,t.ttxx,ImageHeight(t.titlesbar[t.p][t.b].img+1) );
					PasteSprite (  123,t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1 );
					//  star
					Sprite (  123,-100000,-100000,t.titlesbar[t.p][t.b].img+2 );
					SizeSprite (  123,ImageWidth(t.titlesbar[t.p][t.b].img+2),ImageHeight(t.titlesbar[t.p][t.b].img+2) );
					PasteSprite (  123,t.titlesbar[t.p][t.b].x1+t.ttxx,t.titlesbar[t.p][t.b].y1 );
				}
				else
				{
					InkEx ( 64, 64, 64 );// Rgb(64,64,64),0 );
					LineEx (  t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1,t.titlesbar[t.p][t.b].x2,t.titlesbar[t.p][t.b].y1 );
					LineEx (  t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y2,t.titlesbar[t.p][t.b].x2,t.titlesbar[t.p][t.b].y2 );
					LineEx (  t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1,t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y2 );
					LineEx (  t.titlesbar[t.p][t.b].x2,t.titlesbar[t.p][t.b].y1,t.titlesbar[t.p][t.b].x2,t.titlesbar[t.p][t.b].y2 );
					t.ttxx=((t.titlesbar[t.p][t.b].x2-t.titlesbar[t.p][t.b].x1)/100.0)*t.titlesbar[t.p][t.b].fill;
					InkEx ( 255, 255, 255 );//  Rgb(255,255,255),0 );
					BoxEx (  t.titlesbar[t.p][t.b].x1,t.titlesbar[t.p][t.b].y1,t.titlesbar[t.p][t.b].x1+t.ttxx,t.titlesbar[t.p][t.b].y2 );
				}
			}
			#ifdef VRTECH
			if ( t.titlesbar[t.p][t.b].mode == 2 )
			{
				// write text to screen (no image)
				pastebitmapfontcenter ( t.titlesbar[t.p][t.b].text, t.titlesbar[t.p][t.b].x1, t.titlesbar[t.p][t.b].y1, 5, 255);
			}
			#endif
		}

		//  steam condition
		if (  t.game.runasmultiplayer  ==  0 || g.mp.dontDrawTitles  ==  0 || (t.titlespage  >=  11 && t.titlespage  <=  13) ) 
		{
			//  Scan buttons
			t.ttitlesbuttonhighlight=0;
			for ( t.b = 1 ; t.b<=  g.titlesbuttonmax; t.b++ )
			{
				//  check highlighting/slider control
				if (  t.mx >= t.titlesbutton[t.p][t.b].x1 && t.mx <= t.titlesbutton[t.p][t.b].x2 ) 
				{
					if (  t.my >= t.titlesbutton[t.p][t.b].y1 && t.my <= t.titlesbutton[t.p][t.b].y2 ) 
					{
						t.ttitlesbuttonhighlight=t.b;
						if (  t.mc == 1 ) 
						{
							if (  t.titlesbutton[t.p][t.b].specialmode == 1 ) 
							{
								t.tperc_f=t.mx-t.titlesbutton[t.p][t.b].x1;
								t.titlesbutton[t.p][t.b].value=(t.tperc_f/(0.0+(t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)))*100.0;
								if (  t.p == 13 && t.b == 1  )  t.gamesounds.sounds = t.titlesbutton[t.p][t.b].value;
								if (  t.p == 13 && t.b == 2  )  t.gamesounds.music = t.titlesbutton[t.p][t.b].value;
								titles_immediateupdatesound ( );
							}
							if (  t.titlesbutton[t.p][t.b].specialmode == 2 ) 
							{
								t.tperc_f=t.mx-t.titlesbutton[t.p][t.b].x1;
								t.titlesbutton[t.p][t.b].value=(t.tperc_f/(0.0+(t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)))*100.0;
								if (  t.p == 13 && t.b == 1  )  t.gamesounds.sounds = t.titlesbutton[t.p][t.b].value;
								if (  t.p == 13 && t.b == 2  )  t.gamesounds.music = t.titlesbutton[t.p][t.b].value;
								titles_immediateupdatesound ( );
							}
							if (  t.titlesbutton[t.p][t.b].specialmode == 3 ) 
							{
								t.tperc_f=t.mx-t.titlesbutton[t.p][t.b].x1;
								t.titlesbutton[t.p][t.b].value=(t.tperc_f/(0.0+(t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)))*100.0;
								t.visuals.CameraFOV_f=(20+((t.titlesbutton[t.p][t.b].value+0.0)/100.0)*180.0)/t.visuals.CameraASPECT_f;
								t.visuals.refreshshaders=1;
								visuals_loop ( );
							}
						}
					}
				}
				//  draw
				if (  t.titlesbutton[t.p][t.b].specialmode == 1 || t.titlesbutton[t.p][t.b].specialmode == 3 ) 
				{
					//  slider bars
					t.nstep_f=((t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)/100.0)*t.titlesbutton[t.p][t.b].value;
					if (  t.titlesbutton[t.p][t.b].img>0 ) 
					{
						Sprite (  123,-100000,-100000,t.titlesbutton[t.p][t.b].img+1 );
						SizeSprite (  123,t.nstep_f,t.titlesbutton[t.p][t.b].y2-t.titlesbutton[t.p][t.b].y1 );
						PasteSprite (  123,t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y1 );
					}
					else
					{
						InkEx ( 64, 64, 64 );// Rgb(64,64,64),0 );
						BoxEx (  t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y1,t.titlesbutton[t.p][t.b].x1+t.nstep_f,t.titlesbutton[t.p][t.b].y2 );
						InkEx ( 255, 255, 255 );// Rgb(255,255,255),0 );
					}
				}
				if (  t.titlesbutton[t.p][t.b].img == 0 ) 
				{
					if (  t.titlesbutton[t.p][t.b].name_s != "-1" ) 
					{
						InkEx ( 255, 255, 255 );
						LineEx (  t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y1,t.titlesbutton[t.p][t.b].x2,t.titlesbutton[t.p][t.b].y1 );
						LineEx (  t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y2,t.titlesbutton[t.p][t.b].x2,t.titlesbutton[t.p][t.b].y2 );
						LineEx (  t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y1,t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y2 );
						LineEx (  t.titlesbutton[t.p][t.b].x2,t.titlesbutton[t.p][t.b].y1,t.titlesbutton[t.p][t.b].x2,t.titlesbutton[t.p][t.b].y2 );
					}
				}
				else
				{
					t.timagechosen=0;
					if (  t.titlesbutton[t.p][t.b].specialmode == 2 ) 
					{
						if (  g.titlesettings.graphicsettingslevel == t.titlesbutton[t.p][t.b].specialmodevalue ) 
						{
							Sprite (  123,-100000,-100000,t.titlesbutton[t.p][t.b].img+2 );
							t.timagechosen=1;
						}
					}
					if (  t.timagechosen == 0 ) 
					{
						if (  t.b == t.ttitlesbuttonhighlight && t.titlesbutton[t.p][t.b].imghigh>0 ) 
						{
							Sprite (  123,-100000,-100000,t.titlesbutton[t.p][t.b].imghigh );
						}
						else
						{
							Sprite (  123,-100000,-100000,t.titlesbutton[t.p][t.b].img );
						}
					}
					SizeSprite (  123,t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y2-t.titlesbutton[t.p][t.b].y1 );
					PasteSprite (  123,t.titlesbutton[t.p][t.b].x1,t.titlesbutton[t.p][t.b].y1 );
				}
				if (  t.titlesbutton[t.p][t.b].specialmode == 1 || t.titlesbutton[t.p][t.b].specialmode == 3 ) 
				{
					t.nstep_f=(t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)/10.0;
					for ( t.n = 1 ; t.n<=  10; t.n++ )
					{
						LineEx (  t.titlesbutton[t.p][t.b].x1+(t.n*t.nstep_f),t.titlesbutton[t.p][t.b].y2-10,t.titlesbutton[t.p][t.b].x1+(t.n*t.nstep_f),t.titlesbutton[t.p][t.b].y2 );
					}
				}
				if (  t.titlesbutton[t.p][t.b].img == 0 && t.titlesbutton[t.p][t.b].name_s != "-1" ) 
				{
					pastebitmapfontcenter(t.titlesbutton[t.p][t.b].name_s.Get(),t.titlesbutton[t.p][t.b].x1+((t.titlesbutton[t.p][t.b].x2-t.titlesbutton[t.p][t.b].x1)/2),t.titlesbutton[t.p][t.b].y1+((t.titlesbutton[t.p][t.b].y2-t.titlesbutton[t.p][t.b].y1)/2)-15,1,255);
				}
			}
		}

		// Controls
		t.tescapepress=0;
		if ( t.mc == 1 && t.mcselected == 0  )  t.mcselected = 1;
		if ( t.mc == 0 && t.mcselected == 1  )  t.mcselected = 2;
		if ( EscapeKey() == 1 ) 
		{
			while ( EscapeKey() != 0 ) {}
			t.mcselected=2 ; t.tescapepress=1;
			t.ttitlesbuttonhighlight = 0;
		}
		if ( t.mcselected == 2 ) 
		{
			t.mcselected=0;

			// TITLE PAGE
			t.titlespagetousehere=t.titlespage;
			if (  t.titlespagetousehere == 1 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  PLAY
					t.game.levelloop=1;
					t.game.titleloop=0;
				}
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					//  PLAY
					ExecuteFile ( "http://www.fpscreator.com","","",1 );
				}
				if (  t.ttitlesbuttonhighlight == 3 ) 
				{
					//  ABOUT
					titles_aboutpage ( );
				}
				if (  t.ttitlesbuttonhighlight == 4 ) 
				{
					//  EXIT TO WINDOWS
					t.game.levelloop=0;
					t.game.masterloop=0;
					t.game.titleloop=0;
					t.ttitlesbuttonhighlight=0;
				}
			}
			//  ABOUT PAGE
			if (  t.titlespagetousehere == 6 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 || t.tescapepress == 1 ) 
				{
					titles_titlepage ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
			}
			//  OPTIONS PAGE
			if (  t.titlespagetousehere == 11 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 || t.tescapepress == 1 ) 
				{
					//  RESUME
					t.game.titleloop=0;
					t.titlespage=0;
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					//  GRAPHICS SETTINGS
					titles_graphicssettings ( );
				}
				if (  t.ttitlesbuttonhighlight == 3 ) 
				{
					//  SOUND LEVELS
					titles_soundlevels ( );
				}
				if (  t.ttitlesbuttonhighlight == 4 ) 
				{
					//  QUIT GAME
					mp_quitGame ( );
				}
			}

			//
			// MULTIPLAYER SCREENS
			//

			// MAIN MULTIPLAYER MENU
			if (  t.titlespagetousehere == 7 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					// choose type of level
					#ifdef VRTECH
					#ifdef ENABLEMPAVATAR
					characterkitplus_loadMyAvatarInfo();
					#endif
					// create list
					g.mp.listboxmode = 1;
					mp_searchForFpms ( );
					titles_steamchoosefpmtouse();
					#else
					characterkit_loadMyAvatarInfo ( );
					titles_steamchoosetypeoflevel ( );
					#endif
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					// search for lobbies
					g.mp.listboxmode = 0;
					mp_searchForLobbies ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steamSearchLobbies ( );
				}
				if (  t.ttitlesbuttonhighlight == 3 ) 
				{
					// back to IDE/Title
					mp_backToStart ( );
					mp_quitGame ( );
					t.game.masterloop = 0;
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 4 ) 
				{
					#ifdef VRTECH
					// enter Teacher Code
					bool bCodeValid = false;
					int iGotCode = 0;
					ClearEntryBuffer();
					char pCursor[32];
					strcpy ( pCursor, "|" );
					DWORD dwTimeFlash = Timer() + 500;
					while ( iGotCode == 0 )
					{
						// clear screen
						Sprite ( 123, -100000, -100000, g.editorimagesoffset+64 );
						SizeSprite ( 123, GetDisplayWidth(), GetDisplayHeight() );
						PasteSprite ( 123, 0, 0 );

						// title and instructions for entry system
						pastebitmapfontcenter("TEACHER CODE ENTRY",GetDisplayWidth()/2,50,1,255);
						pastebitmapfontcenter("Type out the 'Teacher Code' provided by your Admin",GetDisplayWidth()/2,GetDisplayHeight()-150,1,255);
						pastebitmapfontcenter("to view all levels across the server",GetDisplayWidth()/2,GetDisplayHeight()-100,1,255);

						// extra instructions
						pastebitmapfontcenter("Enter Code:",GetDisplayWidth()/2,(GetDisplayHeight()/2)-50,1,255);
						pastebitmapfontcenter("( press RETURN to submit code, press ESCAPE to quit entry )",GetDisplayWidth()/2,(GetDisplayHeight()/2)+50,1,255);

						// box for entry
						InkEx ( 255, 255, 255 );
						BoxEx ( (GetDisplayWidth()/2)-100, (GetDisplayHeight()/2)-7, (GetDisplayWidth()/2)+100, (GetDisplayHeight()/2)+33 );
						InkEx ( 8, 8, 8 );
						BoxEx ( (GetDisplayWidth()/2)-98, (GetDisplayHeight()/2)-6, (GetDisplayWidth()/2)+98, (GetDisplayHeight()/2)+32 );
						InkEx ( 255, 255, 255 );

						// entry string
						LPSTR pEntryString = Entry(0);
						cstr pFullDisplay = cstr(pEntryString) + pCursor;
						pastebitmapfontcenter(pFullDisplay.Get(),GetDisplayWidth()/2,(GetDisplayHeight()/2)-3,1,255);
						if ( ReturnKey() == 1 )
						{
							iGotCode = 1;
							LPSTR pValidTeacherCode = "12345";
							if ( strnicmp ( pEntryString, pValidTeacherCode, strlen(pValidTeacherCode) ) == NULL )
								bCodeValid = true;
						}
						if ( EscapeKey() == 1 )
						{
							iGotCode = 2;
							t.mcselected = 0;
						}

						// Flash cursor
						if ( Timer() > dwTimeFlash )
						{
							dwTimeFlash = Timer() + 500;
							if ( strcmp ( pCursor, "|" ) == NULL )
								strcpy ( pCursor, " " );
							else
								strcpy ( pCursor, "|" );
						}

						// Update screen
						Sync();
					}

					// if escaped, ignore all we did
					if ( iGotCode != 2 )
					{
						// write special file to activate teacher mode
						if ( bCodeValid == true )
						{
							// teacher mode on
							if ( FileOpen(1) ) CloseFile ( 1 );
							OpenToWrite ( 1, cstr(g.fpscrootdir_s + "\\teacherviewallmode.dat").Get() );
							WriteString ( 1, Str(1) );
							CloseFile ( 1 );
						}
						else
						{
							// teacher mode off
							cstr TeacherViewAllFile_s = cstr(g.fpscrootdir_s + "\\teacherviewallmode.dat");
							if ( FileExist ( TeacherViewAllFile_s.Get() ) ) DeleteFileA ( TeacherViewAllFile_s.Get() );
						}

						// reset multiplayer system and force a full init (to setup new server state)
						mp_restartMultiplayerSystem();
						mp_backToStart ( );
						t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					}
					#else
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					characterkit_chooseOnlineAvatar ( );
					#endif
				}
				#ifdef ENABLEMPAVATAR
				if (t.ttitlesbuttonhighlight == 5)
				{
					t.tescapepress = 0; t.ttitlesbuttonhighlight = 0;
					characterkitplus_chooseOnlineAvatar();
					mp_restartMultiplayerSystem();
					mp_backToStart();
					t.tescapepress = 0; t.ttitlesbuttonhighlight = 0;
				}
				#endif
			}

			// INSIDE MY OWN LOBBY/ROOM SCREEN - READY TO START THE GAME WITH PRESENT PLAYERS
			if ( t.titlespagetousehere == 8 ) 
			{
				if ( t.ttitlesbuttonhighlight == 1 ) 
				{
					// start the game server
					mp_launchGame ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if ( t.ttitlesbuttonhighlight == 2 ) 
				{
					// exit and drop the lobby
					mp_leaveALobby ( );
					#ifdef PHOTONMP
					 // Photon can have host leave and rejoin, no need to reset everything!
					#else
					 mp_resetSteam ( );
					#endif
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}

			// GAME LIST SCREEN - READY TO SELECT A LOBBY/ROOM TO ENTER
			if ( t.titlespagetousehere == 9 ) 
			{
				if ( t.ttitlesbuttonhighlight == 1 ) 
				{
					mp_joinALobby ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					if ( g.mp.mode == MP_JOINING_LOBBY ) 
					{
						#ifdef PHOTONMP
						 // not yet, but soon!
						#else
						 characterkit_loadMyAvatarInfo ( );
						#endif
						titles_steamInLobbyGuest ( );
					}
				}
				if ( t.ttitlesbuttonhighlight == 2 ) 
				{
					mp_backToStart ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}

			#ifdef VRTECH
			/* 160419 - seems redundant, one one choice, so we go direct to it
			// PLAY ONE OF YOUR LEVELS BUTTON SCREEN
			if (  t.titlespagetousehere == 14 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  create list
					g.mp.listboxmode = 1;
					mp_searchForFpms ( );
					titles_steamchoosefpmtouse ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				} 
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					mp_backToStart ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}
			*/
			#else
			//  STEAM PAGE Choose type of level (host)
			if (  t.titlespagetousehere == 14 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  create list
					g.mp.listboxmode = 1;
					mp_searchForFpms ( );
					titles_steamchoosefpmtouse ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				} 
				//  TAKE THIS Line (  OUT BELOW WHEN YOU PUT THE CODE ABOVE BACK! )
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					mp_backToStart ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}
			#endif

			// SELECTED OWN LEVEL - CREATE LOBBY/ROOM FOR IT
			#ifdef VRTECH
			if ( t.titlespagetousehere == 15 ) 
			{
				if ( t.ttitlesbuttonhighlight == 1 ) 
				{
					int iSizeOfFPMFile = atoi(t.tfpmfilesizelist_s[g.mp.selectedLobby].Get());
					if ( iSizeOfFPMFile < 100 )
					{
						// picked the level, create lobby
						mp_selectedALevel ( );
						#ifdef PHOTONMP
						 // Photon does not support custom content at this time in this way!
						 mp_createLobby ( );
						 titles_steamCreateLobby ( );
						#else
						 if ( g.mp.levelContainsCustomContent ==  0 || g.mp.workshopItemChangedFlag  ==  0 ) 
						 {
							mp_createLobby ( );
							titles_steamCreateLobby ( );
						 }
						 else
						 {
							titles_steamdoyouwanttocreateorupdateaworkshopitem ( );
						 }
						#endif
						t.tescapepress=0;
					}
					t.ttitlesbuttonhighlight=0;
				}
				if ( t.ttitlesbuttonhighlight == 2 ) 
				{
					mp_backToStart ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}
			#else
			//  STEAM PAGE Choose level from list (host)
			if (  t.titlespagetousehere == 15 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  picked the level, create lobby
					mp_selectedALevel ( );
					if (  g.mp.levelContainsCustomContent  ==  0 || g.mp.workshopItemChangedFlag  ==  0 ) 
					{
						mp_createLobby ( );
						titles_steamCreateLobby ( );
					}
					else
					{
						titles_steamdoyouwanttocreateorupdateaworkshopitem ( );
					}
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					mp_backToStart ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
				}
			}
			#endif

			// INSIDE SOMEONE ELSES LOBBY/ROOM SCREEN - WAITING FOR HOST TO START GAME
			if ( t.titlespagetousehere == 16 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					g.mp.buildingWorkshopItemMode = 0;
					mp_buildWorkShopItem ( );
					titles_steamcreatingworkshopitem ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					//  create list
					g.mp.listboxmode = 1;
					mp_searchForFpms ( );
					titles_steamchoosefpmtouse ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				if (  t.ttitlesbuttonhighlight == 3 ) 
				{
					SteamShowAgreement (  );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
			}

			// CAN LEAVE LOBBY/ROOM FROM HERE
			if ( t.titlespagetousehere == 10 ) 
			{
				if ( t.ttitlesbuttonhighlight == 1 ) 
				{
					mp_leaveALobby ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steampage ( );
					return;
				}
			}

			// SUBSCRIBE TO WORKSHOP ITEM (CUSTOM LEVEL) SCREEN
			if (  t.titlespagetousehere == 18 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					mp_SubscribeToWorkShopItem ( );
					titles_subscribetoworkshopitemwaitingforresult ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				//  back to searching for lobbies
				if (  t.ttitlesbuttonhighlight == 2 ) 
				{
					//  search for lobbies
					g.mp.listboxmode = 0;
					mp_searchForLobbies ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steamSearchLobbies ( );
				}
			}

			// WAITING FOR SUBSCRIPTION RESULTS SCREEN
			if (  t.titlespagetousehere == 19 ) 
			{
				//  back to searching for lobbies
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  search for lobbies
					g.mp.listboxmode = 0;
					mp_searchForLobbies ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steamSearchLobbies ( );
				}
			}

			// BACK TO SEARCH FOR LOBBIES
			if (  t.titlespagetousehere == 20 ) 
			{
				//  back to searching for lobbies
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  search for lobbies
					g.mp.listboxmode = 0;
					mp_searchForLobbies ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
					titles_steamSearchLobbies ( );
				}
			}

			//  GRAPHICS SETTINGS
			if (  t.titlespagetousehere == 12 ) 
			{
				if (  t.ttitlesbuttonhighlight == 5 || t.tescapepress == 1 ) 
				{
					//  BACK
					titles_optionspage ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				else
				{
					if ( t.ttitlesbuttonhighlight==1 ) {  g.titlesettings.graphicsettingslevel=1 ; titles_immediateupdategraphics ( ); }
					if ( t.ttitlesbuttonhighlight==2 ) {  g.titlesettings.graphicsettingslevel=2 ;titles_immediateupdategraphics ( ); }
					if ( t.ttitlesbuttonhighlight==3 ) {  g.titlesettings.graphicsettingslevel=3 ;titles_immediateupdategraphics ( ); }
				}
			}
			//  SOUND LEVELS
			if (  t.titlespagetousehere == 13 ) 
			{
				if (  t.ttitlesbuttonhighlight == 3 || t.tescapepress == 1 ) 
				{
					//  BACK
					titles_optionspage ( );
					t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
				}
				else
				{
					if (  t.ttitlesbuttonhighlight == 1 ) 
					{
						//  IN-GAME SOUNDS
					}
					if (  t.ttitlesbuttonhighlight == 2 ) 
					{
						//  IN-GAME MUSIC
					}
				}
			}
			//  LEVEL COMPLETE and GAME WON
			if (  t.titlespagetousehere == 3 || t.titlespagetousehere == 4 || t.titlespagetousehere == 5 ) 
			{
				if (  t.ttitlesbuttonhighlight == 1 ) 
				{
					//  CONTINUE
					t.game.titleloop=0;
				}
			}
		}
		//  this is outside of the loop since it checks for state changes rather than a button press
		if (  t.titlespage == 17 ) 
		{
			mp_buildWorkShopItem ( );
			//  success
			if (  g.mp.buildingWorkshopItemMode  ==  99 ) 
			{
				mp_createLobby ( );
				titles_steamCreateLobby ( );
				t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
			}
			//  fail
			if (  g.mp.buildingWorkshopItemMode  ==  98 ) 
			{
				mp_buildingWorkshopItemFailed ( );
				//  create list again
				g.mp.listboxmode = 1;
				mp_searchForFpms ( );
				titles_steamchoosefpmtouse ( );
				t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
			}
		}
		if (  t.titlespagetousehere >= 11 && t.titlespagetousehere <= 19 && t.tescapepress == 1 ) 
		{
			//  can RESUME from options menu using escape key too
			while ( EscapeKey() != 0 ) {}
			t.game.titleloop=0;
			t.titlespage=0;
			t.tescapepress=0 ; t.ttitlesbuttonhighlight=0;
		}
		if (  g.titlesbuttonmax == 0 ) 
		{
			//  Leave right away (Sync (  done later after any updates on page required) )
			t.game.titleloop=0;
		}
		else
		{
			//  Maintain any game visual
			if (  t.titlesgamerendering == 1 ) 
			{
				//  game updates
				music_loop ( );
				//  game renders
				t.gdynamicterrainshadowcameratrigger=1;
				terrain_update ( );
				t.tmastersyncmask=0;
				SyncMask (  t.tmastersyncmask+(1<<3)+(1) );
			}
			//  Steam called to handle the new HOST/JOIN page
			if (  (t.titlespage >= 7 && t.titlespage <= 10) || (t.titlespage>= 15 && t.titlespage <= 20) ) 
			{
				if (  t.game.runasmultiplayer == 1 ) 
				{
					mp_loop ( );
				}
			}
			//  if in MP game, need to keep connection running
			if ( t.game.runasmultiplayer == 1 ) mp_refresh ( );
			// Update screen
			// dave added a skip test for syncing to prevent the editor being drawn when switching to mp game start
			#ifdef VRTECH
			if (  Timer() - t.tskipLevelSync > 200  )  Sync (  );
			#else
			if (Timer() - t.tskipLevelSync > 2000) 
			{
				terrain_shadowupdate();
				terrain_update();
				Sync();
			}
			#endif
		}

	}
	t.null=MouseMoveX()+MouseMoveY();
	while ( EscapeKey() != 0 ) {}

	//  HideMouse (  when menu finished )
	if (  g.titlesbuttonmax>0 || t.thaveShownMouse > 0 ) 
	{
		game_hidemouse ( );
		--t.thaveShownMouse;
	}

	//  Save any settings when finished
	titles_save ( );
}

void loadingpageprogress ( int delta )
{
	t.game.levelloadprogress=t.game.levelloadprogress+(delta/1.2);
	if (  t.game.levelloadprogress>100  )  t.game.levelloadprogress = 100;
	titles_loadingpageupdate ( );
	handletitlesmusic();
}

void handletitlesmusic ( void )
{
	float tFadeTime_f = 0;
	int tMusicID = 0;
	int tVolDiff = 0;

	//  if music doesn't exist, no GetPoint (  doing anything else )
	tMusicID = g.titlessoundoffset+1;
	if (  SoundExist(tMusicID) == 0  )  return;

	//  process fading (all fades take 1 second)
	tFadeTime_f = (Timer() - t.gamesounds.titlemusicfadestamp)/1000.0;
	if (  tFadeTime_f > 1 ) 
	{
		t.gamesounds.titlemusicvolume = t.gamesounds.titlemusicvolumetarget;
	}
	else
	{
		tVolDiff = t.gamesounds.titlemusicvolumetarget - t.gamesounds.titlemusicvolumestart;
		t.gamesounds.titlemusicvolume = t.gamesounds.titlemusicvolumestart + tVolDiff * tFadeTime_f;
	}

	//  make sure music is playing if we have some volume
	if (  t.gamesounds.titlemusicvolume>0 ) 
	{
		if (  SoundPlaying(tMusicID)  ==  0 ) 
		{
			LoopSound (  tMusicID );
		}
	}
	else
	{
		StopSound (  tMusicID );
	}
	SetSoundVolume (  tMusicID,80 + t.gamesounds.titlemusicvolume * 0.2 );
}

void fadetitlesmusic ( int tFadeVol )
{
	t.gamesounds.titlemusicvolumestart = t.gamesounds.titlemusicvolume;
	t.gamesounds.titlemusicvolumetarget = tFadeVol;
	t.gamesounds.titlemusicfadestamp = Timer();
}

/// LUA powered screens

void titleslua_init ( void )
{
	// only load titlesbank scripts if not already done so
	if ( t.bThemeScriptsLoaded==false )
	{
		// load global script to control them all
		t.tfile_s = ""; t.tfile_s = t.tfile_s + "scriptbank\\global.lua";
		t.r=LoadLua(t.tfile_s.Get());

		// load all scripts located in titlebank theme folder
		LPSTR pOldDir = GetDir();
		SetDir(cstr(cstr("titlesbank\\")+t.ttheme_s).Get());
		struct _finddata_t filedata;
		intptr_t hFile = _findfirst("*.lua", &filedata);
		if(hFile!=-1L)
		{
			t.r = LoadLua(filedata.name);
			while(_findnext(hFile, &filedata)==0)
			{
				if(!(filedata.attrib & _A_SUBDIR))
				{
					t.r = LoadLua(filedata.name);
				}
			}
			_findclose(hFile);
		}
		SetDir(pOldDir);

		// titlesbank scripts loaded
		t.bThemeScriptsLoaded = true;
	}
}

void titleslua_main_inandout ( LPSTR pPageName )
{
	// need to quickly flash through page to handle
	// resources - affects speed of DirectX 3D Sound (?!?)
	game_timeelapsed_init ( );
	char pCurrentPage[256];
	strcpy ( pCurrentPage, pPageName );
	strcpy ( t.game.pSwitchToLastPage, pCurrentPage );	
	char pLUAInit[256];
	strcpy ( pLUAInit, cstr(cstr(pCurrentPage)+"_init").Get() );
	strcpy ( g_strErrorClue, pLUAInit );
	LuaSetFunction ( pLUAInit, 0, 0 ); 
	LuaCall (  );
	char pLUAMain[256];
	strcpy ( pLUAMain, cstr(cstr(pCurrentPage)+"_main").Get() );
	strcpy ( g_strErrorClue, pLUAMain );
	game_timeelapsed();
	lua_loop_begin();
	LuaSetFunction ( pLUAMain, 0, 0 ); LuaCall (  );
	lua_loop_finish();
	sliders_draw ( );
	//Sync();
	Sleep(1);
	char pLUAFree[256];
	strcpy ( pLUAFree, cstr(cstr(pCurrentPage)+"_free").Get() );
	LuaSetFunction ( pLUAFree, 0, 0 ); 
	LuaCall (  );
	if ( strcmp ( t.game.pSwitchToPage, "-1")==NULL )
		strcpy ( pCurrentPage, t.game.pSwitchToLastPage );
	else
		strcpy ( pCurrentPage, t.game.pSwitchToPage );
	t.conkit.cooldown = 100;
}

int g_iTitleMainState = 0;
char g_pTitleCurrentPage[256];
char g_pTitleLUAMain[256];

void titleslua_main_stage1_init(LPSTR pPageName)
{
	// title main init

	// Machine independent speed
	game_timeelapsed_init ( );

	// keep doing pages until we exit with no page
	strcpy ( g_pTitleCurrentPage, pPageName );
	strcpy ( t.game.pSwitchToLastPage, g_pTitleCurrentPage );	
}
void titleslua_main_stage2_preloop(void)
{
	// title preloop
	// call init function to set up resources
	char pLUAInit[256];
	strcpy ( pLUAInit, cstr(cstr(g_pTitleCurrentPage)+"_init").Get() );
	strcpy ( g_strErrorClue, pLUAInit );
	LuaSetFunction ( pLUAInit, 0, 0 ); 
	LuaCall (  );

	// stay in main until page is quit
	strcpy ( g_pTitleLUAMain, cstr(cstr(g_pTitleCurrentPage)+"_main").Get() );
	strcpy ( g_strErrorClue, g_pTitleLUAMain );
	t.game.titleloop = 1;
}
void titleslua_main_stage3_inloop(void)
{
	// title loop
	// Machine independent speed update (makes g_TimeElapsed available)
	game_timeelapsed();

	// run LUA logic
	lua_loop_begin();
	LuaSetFunction ( g_pTitleLUAMain, 0, 0 ); LuaCall (  );
	lua_loop_finish();

	// if in game, extra update refreshes
	if ( t.game.gameloop == 1 ) 
	{
		music_loop ( );
		t.gdynamicterrainshadowcameratrigger=1;
		#ifdef VRTECH
		terrain_update ( );
		t.tmastersyncmask=0;
		SyncMask (  t.tmastersyncmask+(1<<3)+(1) );
		#else
		terrain_shadowupdate();
		terrain_update ( );
		t.tmastersyncmask=0;
		//SyncMask (  t.tmastersyncmask+(1<<3)+(1) );
		//PE: We need to render everything to also see shadows.
		SyncMask(0xfffffff9);
		#endif
	}

	// draw all sprites required
	sliders_draw ( );

	// update screen
	Sync();
}
void titleslua_main_stage4_afterloop(void)
{
	// title afterloop
	// wait for mouse click release
	while ( MouseClick()!=0 ) { Sleep(1); }

	// call to allow local resources to be freed
	char pLUAFree[256];
	strcpy ( pLUAFree, cstr(cstr(g_pTitleCurrentPage)+"_free").Get() );
	LuaSetFunction ( pLUAFree, 0, 0 ); 
	LuaCall (  );
		
	// switch to new page name
	if ( strcmp ( t.game.pSwitchToPage, "-1")==NULL )
		strcpy ( g_pTitleCurrentPage, t.game.pSwitchToLastPage );
	else
		strcpy ( g_pTitleCurrentPage, t.game.pSwitchToPage );

	// ensure IMGUI does not attempt to render
	#ifndef PRODUCTCLASSIC
	extern bool bBlockImGuiUntilNewFrame;
	bBlockImGuiUntilNewFrame = true;
	#endif
}
void titleslua_main_stage5(void)
{
	// currentpage afterloop
	// need to ensure low FPS warning not triggered by game sync absense
	t.conkit.cooldown = 100;
}
bool titleslua_main_loopcode(void)
{
	if (g_iTitleMainState == 0)
	{
		// current page loop
		if (strcmp(g_pTitleCurrentPage, "") != NULL) 
			g_iTitleMainState = 1;
		else
			g_iTitleMainState = 4;
	}
	if (g_iTitleMainState == 1)
	{
		// title pre
		titleslua_main_stage2_preloop();
		g_iTitleMainState = 2;
	}
	if (g_iTitleMainState == 2)
	{
		// title loop
		if (t.game.titleloop == 1) 
			titleslua_main_stage3_inloop();
		else
			g_iTitleMainState = 3;
	}
	if (g_iTitleMainState == 3)
	{
		// title post
		titleslua_main_stage4_afterloop();
		g_iTitleMainState = 0;
	}
	if (g_iTitleMainState == 4)
	{
		// currentpage post
		titleslua_main_stage5();
		g_iTitleMainState = 0;
		return true;
	}
	return false;
}

void titleslua_main(LPSTR pPageName)
{
	// tight internal loop version for legacy
	bool bRunLoop = true;
	titleslua_main_stage1_init(pPageName);
	g_iTitleMainState = 0;
	while (bRunLoop == true)
	{
		if (titleslua_main_loopcode() == true) bRunLoop = false;
	}
	/*
	titleslua_main_stage1(pPageName);
	while ( strcmp ( g_pTitleCurrentPage, "" )!=NULL )
	{
		titleslua_main_stage2();
		while ( t.game.titleloop==1 )
		{
			titleslua_main_stage3();
		}
		titleslua_main_stage4();
	}
	titleslua_main_stage5();
	*/
}

/*
void titleslua_main_old ( LPSTR pPageName )
{
	// Machine independent speed
	game_timeelapsed_init ( );

	// keep doing pages until we exit with no page
	char pCurrentPage[256];
	strcpy ( pCurrentPage, pPageName );
	strcpy ( t.game.pSwitchToLastPage, pCurrentPage );	
	while ( strcmp ( pCurrentPage, "" )!=NULL )
	{
		// call init function to set up resources
		char pLUAInit[256];
		strcpy ( pLUAInit, cstr(cstr(pCurrentPage)+"_init").Get() );
		strcpy ( g_strErrorClue, pLUAInit );
		LuaSetFunction ( pLUAInit, 0, 0 ); 
		LuaCall (  );

		// stay in main until page is quit
		char pLUAMain[256];
		strcpy ( pLUAMain, cstr(cstr(pCurrentPage)+"_main").Get() );
		strcpy ( g_strErrorClue, pLUAMain );
		t.game.titleloop = 1;
		while ( t.game.titleloop==1 )
		{
			// Machine independent speed update (makes g_TimeElapsed available)
			game_timeelapsed();

			// run LUA logic
			lua_loop_begin();
			LuaSetFunction ( pLUAMain, 0, 0 ); LuaCall (  );
			lua_loop_finish();

			// if in game, extra update refreshes
			if ( t.game.gameloop == 1 ) 
			{
				music_loop ( );
				t.gdynamicterrainshadowcameratrigger=1;
				#ifdef VRTECH
				terrain_update ( );
				t.tmastersyncmask=0;
				SyncMask (  t.tmastersyncmask+(1<<3)+(1) );
				#else
				terrain_shadowupdate();
				terrain_update ( );
				t.tmastersyncmask=0;
				//SyncMask (  t.tmastersyncmask+(1<<3)+(1) );
				//PE: We need to render everything to also see shadows.
				SyncMask(0xfffffff9);
				#endif
			}

			// draw all sprites required
			sliders_draw ( );

			// update screen
			Sync();
		}

		// wait for mouse click release
		while ( MouseClick()!=0 ) { Sleep(1); }

		// call to allow local resources to be freed
		char pLUAFree[256];
		strcpy ( pLUAFree, cstr(cstr(pCurrentPage)+"_free").Get() );
		LuaSetFunction ( pLUAFree, 0, 0 ); 
		LuaCall (  );
		
		// switch to new page name
		if ( strcmp ( t.game.pSwitchToPage, "-1")==NULL )
			strcpy ( pCurrentPage, t.game.pSwitchToLastPage );
		else
			strcpy ( pCurrentPage, t.game.pSwitchToPage );
	}

	// need to ensure low FPS warning not triggered by game sync absense
	t.conkit.cooldown = 100;
}
*/

void titleslua_free ( void )
{
	// frees all loaded scripts
	t.bThemeScriptsLoaded = false;
	//LuaReset (  );
}

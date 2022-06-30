//----------------------------------------------------
//--- GAMEGURU - M-Interactive
//----------------------------------------------------

#include "stdafx.h"
#include "gameguru.h"

void set_inputsys_mclick(int value);

// 
//  Interactive Module
// 

void interactive_init ( void )
{
	#ifdef VRTECH
	//  Load interactive resources
	t.tinteractivepath_s="languagebank\\neutral\\gamecore\\huds\\interactive\\";
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"tutorial-background.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+1 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"small-panel.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+2 );

	//  Welcome
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"emc-icon.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+11 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"begin.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+12 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"begin-highlight.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+13 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"close.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+14 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"close-highlight.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+15 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"checkbox.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+16 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"checkbox-hightlight.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+17 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"checkbox-tick.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+18 );

	//  Select
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"grid-panel.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+21 );
	t.strwork = "" ; t.strwork = t.strwork +  t.tinteractivepath_s+"back.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+22 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"back-highlight.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+23 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"next.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+24 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"next-highlight.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+25 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"highlighter.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+26 );

	//  Summary
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"tutorials.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+31 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"tutorials-highlight.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+32 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"finish.png";
	LoadImage (  t.strwork.Get() ,g.interactiveimageoffset+33 );
	t.strwork = "" ; t.strwork = t.strwork + t.tinteractivepath_s+"finish-highlight.png";
	LoadImage ( t.strwork.Get() ,g.interactiveimageoffset+34 );
	#endif

	//  Scan mapbank\tutorial maps and collect grid of levels
	t.tindex=1;
	t.toldd_s=GetDir();
	SetDir (  "mapbank\\tutorialmaps" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			if (  cstr(Right(t.tfile_s.Get(),4)) == ".fpm" ) 
			{
				//  fpm level filename
				if (  t.tindex <= 24 ) 
				{
					//  store fpm name
					t.tutorialmaps_s[t.tindex]=Upper(t.tfile_s.Get());
					//  and load thumbnail
					t.timg_s = "" ; t.timg_s=t.timg_s + Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4)+".png";
					if (  FileExist(t.timg_s.Get()) == 1 ) 
					{
						LoadImage (  t.timg_s.Get(),g.interactiveimageoffset+40+t.tindex );
					}
					//  and advance
					++t.tindex;
				}
			}
		}
	}
	SetDir (  t.toldd_s.Get() );

	// By default this is ON
	t.interactive.showonstartup=1;
	t.interactive.active = 0;

	#ifdef VRTECH
	t.tfile_s=g.fpscrootdir_s+"\\tutorialonstartup.ini";
	if (  FileExist(t.tfile_s.Get()) == 1 ) 
	{
		OpenToRead (  1,t.tfile_s.Get() );
		t.t_s = ReadString (  1 ); t.interactive.showonstartup=ValF(t.t_s.Get());
		CloseFile (  1 );
	}
	if (  t.interactive.showonstartup == 1 ) 
	{
		t.interactive.active=2;
	}
	else
	{
		t.interactive.active=0;
	}
	#endif
}

void interactive_loop ( void )
{

	//  When set to one, disables click in editor
	t.interactive.insidepanel=0;

	//  main panel control
	if (  t.interactive.active == 2 ) 
	{

		//  Trigger interactive tutorial panel
		t.interactive.pageindex=1;
		t.interactive.sceneindex=1;
		interactive_restartalpha ( );
		t.interactive.alphafadecommand=0;
		t.interactive.active=1;
		t.interactive.alphainitial=1;

	}
	if (  t.interactive.active == 1 ) 
	{

		//  Input method
		if (  g.slidersprotoworkmode == 1 ) 
		{
			t.inputsys.kscancode=ScanCode();
			set_inputsys_mclick(MouseClick());// t.inputsys.mclick = MouseClick();
			t.inputsys.xmouse=MouseX();
			t.inputsys.ymouse=MouseY();
		}
		else
		{
			t.inputsys.kscancode=GetFileMapDWORD( 1, 100 );
			set_inputsys_mclick(GetFileMapDWORD(1, 20));// t.inputsys.mclick = GetFileMapDWORD(1, 20);
			t.inputsys.xmouse=((GetFileMapDWORD( 1, 0 )+0.0)/800.0)*GetChildWindowWidth(0);
			t.inputsys.ymouse=((GetFileMapDWORD( 1, 4 )+0.0)/600.0)*GetChildWindowHeight(0);

			//  detect open, save, new, newflat, saveas
			if (  GetFileMapDWORD( 1, 400 ) == 1  )  t.interactive.active = 0;
			if (  GetFileMapDWORD( 1, 404 ) == 1  )  t.interactive.active = 0;
			if (  GetFileMapDWORD( 1, 408 ) != 0  )  t.interactive.active = 0;
			if (  GetFileMapDWORD( 1, 434 ) == 1  )  t.interactive.active = 0;
			if (  GetFileMapDWORD( 1, 758 ) != 0  )  t.interactive.active = 0;
			if (  GetFileMapDWORD( 1, 762 ) != 0  )  t.interactive.active = 0;

		}

		//  draw backdrop panel
		t.tbackdropimage=g.interactiveimageoffset+1 ; t.tbackdropoffsety=0;
		if (  t.interactive.pageindex >= 21 && t.interactive.pageindex<91 ) 
		{
			t.tbackdropimage=g.interactiveimageoffset+2 ; t.tbackdropoffsety=155;
		}
		t.imgx_f=ImageWidth(t.tbackdropimage);
		t.imgy_f=ImageHeight(t.tbackdropimage);
		t.tcenterx=(GetChildWindowWidth(0)-t.imgx_f)/2;
		t.tcentery=(GetChildWindowHeight(0)-t.imgy_f)/2;
		t.tcentery=t.tcentery+t.tbackdropoffsety;
		Sprite (  223,-10000,-10000,t.tbackdropimage );
		SizeSprite (  223,t.imgx_f,t.imgy_f );
		if (  t.interactive.pageindex == 1 && t.interactive.alphainitial == 1 ) 
		{
			SetSpriteAlpha (  223,t.interactive.alphafade );
		}
		else
		{
			SetSpriteAlpha (  223,255 );
		}
		PasteSprite (  223,t.tcenterx,t.tcentery );
		if (  t.inputsys.xmouse>t.tcenterx && t.inputsys.xmouse<t.tcenterx+t.imgx_f && t.inputsys.ymouse>t.tcentery && t.inputsys.ymouse<t.tcentery+t.imgy_f ) 
		{
			t.interactive.insidepanel=1;
		}

		//  draw page specific stuff
		t.interactive.itemhighlightindex=0;
		switch (  t.interactive.pageindex ) 
		{
			case 1 : interactive_welcome() ; break ;
 
			case 11 : interactive_picktemplate() ; break ;
 
			case 21 : interactive_entitywork() ; break ;
 
			case 91 : interactive_summary() ; break ;
 
		}//~   `

	}

	//  alpha control
	if (  t.interactive.alphafadestage == 1 ) 
	{
		t.interactive.alphafade=t.interactive.alphafade+((Timer()-t.interactive.alphalasttime)*2);
		t.interactive.alphalasttime=Timer();
		if (  t.interactive.alphafade >= 255 ) 
		{
			t.interactive.alphafade=255;
			t.interactive.alphafadestage=2;
		}
	}

	//  never carry this mouse click outside interactive panel
	set_inputsys_mclick(0);// t.inputsys.mclick = 0;
}

void interactive_restartalpha ( void )
{

	//  Force a fade in
	t.interactive.alphafadestage=1;
	t.interactive.alphafade=0;
	t.interactive.alphalasttime=Timer();
}

void interactive_welcome ( void )
{

	//  detect button highlighting
	interactivedrawimg(t.tcenterx+431,t.tcentery+137,g.interactiveimageoffset+11,1,0);
	interactivedrawimg(t.tcenterx+385,t.tcentery+398,g.interactiveimageoffset+12,2,1);
	interactivedrawimg(t.tcenterx+789,t.tcentery+510,g.interactiveimageoffset+14,3,1);
	interactivedrawimg(t.tcenterx+037,t.tcentery+522,g.interactiveimageoffset+16,4,1);
	if (  t.interactive.showonstartup == 0  )  PasteImage (  g.interactiveimageoffset+18,t.tcenterx+38,t.tcentery+522,1 );

	//  Text (  )
	pastebitmapfont("INTERACTIVE TUTORIAL",t.tcenterx+669,t.tcentery+42,7,t.interactive.alphafade);
	pastebitmapfont("Don't show t.interactive tutorial again",t.tcenterx+76,t.tcentery+522,7,t.interactive.alphafade);
	pastebitmapfontcenter("Welcome to the Game Guru Interactive Tutorial",t.tcenterx+460,t.tcentery+200,7,t.interactive.alphafade);
	t.t=0 ; t.t_s="To help you get the most out of Game Guru this short interactive tutorial will guide" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);
	t.t=1 ; t.t_s="you through the creation and editing of a simple scene, and walk you through a quick" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);
	t.t=2 ; t.t_s="play test of the level you created." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);

	//  handle button controls
	if (  t.inputsys.mclick == 1 ) 
	{
		if (  t.interactive.clicked == 0 ) 
		{
			if (  t.interactive.itemhighlightindex == 2 ) 
			{
				//  begin tutorial
				t.interactive.pageindex=11;
				t.interactive.sceneselectednow=0;
				interactive_restartalpha ( );
				t.interactive.alphainitial=0;
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 3 ) 
			{
				//  close
				set_inputsys_mclick(1);// t.inputsys.mclick = 1;
				while (  t.inputsys.mclick != 0 ) 
				{
					if (  g.slidersprotoworkmode == 1 ) 
					{
						set_inputsys_mclick(MouseClick());// t.inputsys.mclick = MouseClick();
					}
					else
					{
						set_inputsys_mclick(GetFileMapDWORD(1, 20));// t.inputsys.mclick = GetFileMapDWORD(1, 20);
					}
				}
				t.interactive.active=0;
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 4 ) 
			{
				//  toggle show on startup
				t.interactive.showonstartup=1-t.interactive.showonstartup;
				t.tfile_s=g.fpscrootdir_s+"\\tutorialonstartup.ini";
				if (  FileExist(t.tfile_s.Get()) == 1  )  DeleteAFile (  t.tfile_s.Get() );
				if (  FileOpen(1) == 1  )  CloseFile (  1 );
				OpenToWrite (  1,t.tfile_s.Get() );
				WriteString (  1,Str(t.interactive.showonstartup) );
				CloseFile (  1 );
				t.interactive.clicked=1;
			}
		}
	}
	else
	{
		t.interactive.clicked=0;
	}

return;

}

void interactive_picktemplate ( void )
{
	//  buttons
	interactivedrawimg(t.tcenterx+10,t.tcentery+92,g.interactiveimageoffset+21,1,0);
	interactivedrawimgalpha(t.tcenterx+30,t.tcentery+510,g.interactiveimageoffset+22,2,1,1);
	interactivedrawimgalpha(t.tcenterx+838,t.tcentery+510,g.interactiveimageoffset+24,3,1,1);
	//  draw thumbs
	for ( t.tGridY = 0 ; t.tGridY<=  3; t.tGridY++ )
	{
		for ( t.tGridX = 0 ; t.tGridX<=  5; t.tGridX++ )
		{
			t.tindex=1+(t.tGridX)+(t.tGridY*6);
			interactivedrawonly(t.tcenterx+10+(t.tGridX*150),t.tcentery+92+(t.tGridY*100),g.interactiveimageoffset+40+t.tindex,0);
			pastebitmapfontcenter(t.tutorialmaps_s[t.tindex].Get(),t.tcenterx+10+(t.tGridX*150)+75,t.tcentery+92+(t.tGridY*100)+80,2,t.interactive.alphafade);
		}
	}
	//  highlighters
	t.tGridY=(t.interactive.sceneindex-1)/6;
	t.tGridX=(t.interactive.sceneindex-1-(t.tGridY*6));
	PasteImage (  g.interactiveimageoffset+26,t.tcenterx+10+(t.tGridX*150)+1,t.tcentery+92+(t.tGridY*100)+1,1 );
	if (  t.interactive.itemhighlightindex == 1 ) 
	{
		t.tGridX=(t.inputsys.xmouse-t.tcenterx-10)/150;
		t.tGridY=(t.inputsys.ymouse-t.tcentery-92)/100;
		if (  t.tGridX<0  )  t.tGridX = 0;
		if (  t.tGridX>5  )  t.tGridX = 5;
		if (  t.tGridX<0  )  t.tGridY = 0;
		if (  t.tGridY>3  )  t.tGridY = 3;
		PasteImage (  g.interactiveimageoffset+26,t.tcenterx+10+(t.tGridX*150)+1,t.tcentery+92+(t.tGridY*100)+1,1 );
	}

	//  Text (  )
	pastebitmapfont("INTERACTIVE TUTORIAL",t.tcenterx+669,t.tcentery+42,7,255);
	if (  t.interactive.sceneselectednow == 1 ) 
	{
		t.t_s="Scene Selected - Click the Right Arrow Button to continue";
	}
	else
	{
		t.t_s="Select A Scene For Your Level";
	}
	pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+526,7,t.interactive.alphafade);

	//  handle button controls
	if (  t.inputsys.mclick == 1 ) 
	{
		if (  t.interactive.clicked == 0 ) 
		{
			if (  t.interactive.itemhighlightindex == 1 ) 
			{
				//  select scene
				t.interactive.sceneindex=1+(t.tGridY*6)+t.tGridX;
				t.interactive.sceneselectednow=1;
				t.interactive.clicked=1;
				//  load immediately
				if (  t.interactive.sceneindex >= 1 && t.interactive.sceneindex <= 24 ) 
				{
					t.toldprojectfilename_s=g.projectfilename_s;
					g.projectfilename_s=g.fpscrootdir_s+"\\Files\\mapbank\\tutorialmaps\\"+t.tutorialmaps_s[t.interactive.sceneindex];
					if (  Len(g.projectfilename_s.Get())>2 ) 
					{
						gridedit_load_map ( );
					}
					else
					{
						g.projectfilename_s=t.toldprojectfilename_s;
					}
				}
			}
			if (  t.interactive.itemhighlightindex == 2 ) 
			{
				//  previous
				t.interactive.pageindex=1 ; interactive_restartalpha ( );
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 3 && t.interactive.sceneselectednow == 1 ) 
			{
				//  next
				t.interactive.pageindex=21 ; interactive_restartalpha ( );
				t.interactive.sequencegroup=1;
				t.interactive.sequenceindex=1;
				t.interactive.editorlogicevent=0;
				t.interactive.clicked=1;
			}
		}
	}
	else
	{
		t.interactive.clicked=0;
	}

return;

}

void interactive_entitywork ( void )
{

	//  buttons
	interactivedrawimgalpha(t.tcenterx+30,t.tcentery+200,g.interactiveimageoffset+22,1,1,1);
	interactivedrawimgalpha(t.tcenterx+838,t.tcentery+200,g.interactiveimageoffset+24,2,1,1);

	//  Sequence groups (all stages of interaction)
	t.interactive.sequencegroupmax=3;
	if (  t.interactive.sequencegroup == 1 ) 
	{

		//  Choose and Place Entity
		t.t=0 ; t.t_s="On the left you will see a button for adding new entities and beneath" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=1 ; t.t_s="a list of entities already loaded as part of the scene you selected." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=2 ; t.t_s="Entities are game objects used to populate your level." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=3 ; t.t_s="" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.interactive.sequencemax=2;
		t.t_s = ""; t.t_s=t.t_s+Str(t.interactive.sequenceindex)+" of "+Str(t.interactive.sequencemax)+" : ";
		switch (  t.interactive.sequenceindex ) 
		{
		case 1 : t.t_s = t.t_s+"Choose an Entity from the left side panel" ; break;
		case 2 : t.t_s = t.t_s+"Left click to place the Entity into the scene" ; break ;
		}		//~   pastebitmapfontcenter(t$,tcenterx+460,tcentery+216,7,interactive.alphafade)
		pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+216,7,t.interactive.alphafade);

	}
	if (  t.interactive.sequencegroup == 2 ) 
	{

		//  Edit Entity
		t.t=0 ; t.t_s="You can modify an entity and its properties using the entity widget" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=1 ; t.t_s="which is displayed when you select the entity within the scene." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=2 ; t.t_s="" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=3 ; t.t_s="" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.interactive.sequencemax=5;
		t.t_s = ""; t.t_s=t.t_s+Str(t.interactive.sequenceindex)+" of "+Str(t.interactive.sequencemax)+" : ";
		switch (  t.interactive.sequenceindex ) 
		{
		case 1 : t.t_s = t.t_s+"Left click an entity within the scene" ; break ;
		case 2 : t.t_s = t.t_s+"Use the green square to drag the entity around" ; break ;
		case 3 : t.t_s = t.t_s+"Select PROPERTIES from the t.widget menu" ; break ;
		case 4 : t.t_s = t.t_s+"Edit the settings then click APPLY CHANGES" ; break ;
		case 5 : t.t_s = t.t_s+"Reselect entity && press R key to easily rotate" ; break ;
		}		//~   pastebitmapfontcenter(t$,tcenterx+460,tcentery+216,7,interactive.alphafade)
		pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+216,7,t.interactive.alphafade);

	}
	if (  t.interactive.sequencegroup == 3 ) 
	{

		//  Test The Game
		t.t=0 ; t.t_s="You can test your game level at any time by clicking the TEST GAME button" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=1 ; t.t_s="from the main toolbar. During the test, all A.I will become active and" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=2 ; t.t_s="player controls enabled so you can play the current state of your game." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.t=3 ; t.t_s="You can leave test game at any time by pressing the ESCAPE key." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+35+(t.t*30),7,255);
		t.interactive.sequencemax=2;
		t.t_s = ""; t.t_s=t.t_s+Str(t.interactive.sequenceindex)+" of "+Str(t.interactive.sequencemax)+" : ";
		switch (  t.interactive.sequenceindex ) 
		{
		case 1 : t.t_s = t.t_s+"Click the test game button located in the above toolbar" ; break ;
		case 2 : t.t_s = t.t_s+"Export your game by selecting SAVE STANDALONE in FILE menu" ; break ;
		}		//~   pastebitmapfontcenter(t$,tcenterx+460,tcentery+216,7,interactive.alphafade)
		pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+216,7,t.interactive.alphafade);

	}

	//  In-Editor actions trigger advancement of tutorial
	t.tlogicalnextstep=0;
	if (  1 ) 
	{
		if (  t.interactive.sequencegroup == 1 ) 
		{
			if (  t.interactive.sequenceindex == 1 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.gridentity == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.gridentity>0  )  t.tlogicalnextstep = 1;
			}
			if (  t.interactive.sequenceindex == 2 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.gridentity>0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.gridentity == 0  )  t.tlogicalnextstep = 1;
			}
		}
		if (  t.interactive.sequencegroup == 2 ) 
		{
			if (  t.interactive.sequenceindex == 1 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.widget.pickedObject == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.widget.pickedObject>0  )  t.tlogicalnextstep = 1;
			}
			if (  t.interactive.sequenceindex == 2 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.widget.pickedSection != t.widget.widgetXZObj  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.widget.pickedSection == t.widget.widgetXZObj && t.inputsys.mclick == 1  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 2 && t.inputsys.mclick == 0  )  t.tlogicalnextstep = 1;
			}
			if (  t.interactive.sequenceindex == 3 ) 
			{
				if (  t.interactive.editorlogicevent  ==  0 && t.grideditselect !=  4  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent  ==  1 && t.grideditselect  ==  4  )  t.tlogicalnextstep  =  1;
			}
			if (  t.interactive.sequenceindex == 4 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.interactive.applychangesused == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.interactive.applychangesused == 1 ) { t.tlogicalnextstep = 1  ; t.interactive.applychangesused = 0; }
			}
			if (  t.interactive.sequenceindex == 5 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.inputsys.keypress == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.inputsys.keypress != 0  )  t.tlogicalnextstep = 1;
			}
		}
		if (  t.interactive.sequencegroup == 3 ) 
		{
			if (  t.interactive.sequenceindex == 1 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.interactive.testgameused == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.interactive.testgameused == 1 ) { t.interactive.testgameused = 0  ; t.tlogicalnextstep = 1; }
			}
			if (  t.interactive.sequenceindex == 2 ) 
			{
				if (  t.interactive.editorlogicevent == 0 && t.interactive.savestandaloneused == 0  )  ++t.interactive.editorlogicevent;
				if (  t.interactive.editorlogicevent == 1 && t.interactive.savestandaloneused == 1 ) { t.interactive.savestandaloneused = 0  ; t.tlogicalnextstep = 1; }
			}
		}
	}

	//  handle button controls
	if (  t.inputsys.mclick == 1 || t.tlogicalnextstep == 1 ) 
	{
		if (  t.interactive.clicked == 0 ) 
		{
			if (  t.interactive.itemhighlightindex == 1 ) 
			{
				//  previous
				t.interactive.editorlogicevent=0;
				t.interactive.sequenceindex=t.interactive.sequenceindex-1  ; interactive_restartalpha ( );
				if (  t.interactive.sequenceindex<1 ) 
				{
					t.interactive.sequencegroup=t.interactive.sequencegroup-1;
					if (  t.interactive.sequencegroup<1 ) 
					{
						t.interactive.pageindex=11;
					}
					t.interactive.sequenceindex=t.interactivesequencemaxhistory[t.interactive.sequencegroup];
				}
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 2 || t.tlogicalnextstep == 1 ) 
			{
				//  next
				t.interactive.editorlogicevent=0;
				t.interactive.sequenceindex=t.interactive.sequenceindex+1  ; interactive_restartalpha ( );
				if (  t.interactive.sequenceindex>t.interactive.sequencemax ) 
				{
					t.interactivesequencemaxhistory[t.interactive.sequencegroup]=t.interactive.sequencemax;
					t.interactive.sequenceindex=1;
					t.interactive.sequencegroup=t.interactive.sequencegroup+1;
					if (  t.interactive.sequencegroup>t.interactive.sequencegroupmax ) 
					{
						t.interactive.pageindex=91;
					}
				}
				t.interactive.clicked=1;
			}
		}
	}
	if (  t.inputsys.mclick == 0  )  t.interactive.clicked = 0;

return;

}

void interactive_summary ( void )
{

	//  detect button highlighting
	interactivedrawimg(t.tcenterx+431,t.tcentery+137,g.interactiveimageoffset+11,1,0);
	interactivedrawimg(t.tcenterx+385,t.tcentery+398,g.interactiveimageoffset+31,2,1);
	interactivedrawimgalpha(t.tcenterx+789,t.tcentery+510,g.interactiveimageoffset+33,3,1,1);
	interactivedrawimgalpha(t.tcenterx+30,t.tcentery+510,g.interactiveimageoffset+22,5,1,1);

	//  Text (  )
	pastebitmapfont("INTERACTIVE TUTORIAL",t.tcenterx+669,t.tcentery+42,7,t.interactive.alphafade);
	pastebitmapfontcenter("Thank you for using the Interactive Tutorial",t.tcenterx+460,t.tcentery+200,7,t.interactive.alphafade);
	t.t=0 ; t.t_s="If you need more help you can find additional tutorials on the Game Guru website" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);
	t.t=1 ; t.t_s="by clicking the TUTORIALS button. The site contains videos showing both basic" ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);
	t.t=2 ; t.t_s="and advanced features of Game Guru." ; pastebitmapfontcenter(t.t_s.Get(),t.tcenterx+460,t.tcentery+250+(t.t*30),7,t.interactive.alphafade);

	//  handle button controls
	if (  t.inputsys.mclick == 1 ) 
	{
		if (  t.interactive.clicked == 0 ) 
		{
			if (  t.interactive.itemhighlightindex == 2 ) 
			{
				//  jump to tutorial website
				ExecuteFile ( "https://www.game-guru.com/tutorials","","",0 );
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 3 ) 
			{
				//  finish
				set_inputsys_mclick(1);// t.inputsys.mclick = 1;
				while (  t.inputsys.mclick != 0 ) 
				{
					if (  g.slidersprotoworkmode == 1 ) 
					{
						set_inputsys_mclick(MouseClick());// t.inputsys.mclick = MouseClick();
					}
					else
					{
						set_inputsys_mclick(GetFileMapDWORD(1, 20));// t.inputsys.mclick = GetFileMapDWORD(1, 20);
					}
				}
				t.interactive.active=0;
				t.interactive.clicked=1;
			}
			if (  t.interactive.itemhighlightindex == 5 ) 
			{
				//  previous
				t.interactive.pageindex=21  ; interactive_restartalpha ( );
				t.interactive.sequencegroup=t.interactive.sequencegroupmax;
				t.interactive.sequenceindex=t.interactivesequencemaxhistory[t.interactive.sequencegroup];
				t.interactive.clicked=1;
			}
		}
	}
	else
	{
		t.interactive.clicked=0;
	}

return;


//Functions for interactive tutorial


}

void interactivedrawonly ( int x, int y, int img, int noalpha )
{
	int high = 0;
	int wid = 0;
	wid=ImageWidth(img) ; high=ImageHeight(img);
	Sprite (  223,-10000,-10000,img );
	if (  noalpha == 1 ) 
	{
		SetSpriteAlpha (  223,255 );
	}
	else
	{
		SetSpriteAlpha (  223,t.interactive.alphafade );
	}
	SizeSprite (  223,wid,high  ); PasteSprite (  223,x,y );
//endfunction

}

void interactivedrawimgalpha ( int x, int y, int img, int itemindex, int highlightable, int noalpha )
{
	int wid = 0;
	int high = 0;
	wid=ImageWidth(img) ; high=ImageHeight(img);
	if (  t.inputsys.xmouse>x && t.inputsys.xmouse<x+wid && t.inputsys.ymouse>y && t.inputsys.ymouse<y+high ) 
	{
		t.interactive.itemhighlightindex=itemindex;
		if (  highlightable == 1  )  img = img+1;
	}
	interactivedrawonly(x,y,img,noalpha);
//endfunction

}

void interactivedrawimg ( int x, int y, int img, int itemindex, int highlightable )
{
	interactivedrawimgalpha(x,y,img,itemindex,highlightable,0);
//endfunction

}

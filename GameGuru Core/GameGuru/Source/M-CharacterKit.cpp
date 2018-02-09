//----------------------------------------------------
//--- GAMEGURU - M-CharacterKit
//----------------------------------------------------

#include "gameguru.h"

void characterkit_init ( void )
{
	if (  t.characterkit.selected  !=  99 ) 
	{
		pastebitmapfontcenter("PREPARING CHARACTER CREATOR",GetChildWindowWidth()/2,40,1,255) ; Sync (  );
	}
	else
	{
		pastebitmapfontcenter("PREPARING CHARACTERS",GetChildWindowWidth()/2,40,1,255) ; Sync (  );
	}

	//  hide waypoints
	waypoint_hideall ( );

	t.characterkit.oldMouseClick = 1;
	t.characterkit.loadthumbs = 0;

	t.characterkitcontrol.customHeadMode = 0;

	t.characterkit.imagestart = g.charactercreatorEditorImageoffset;
	if (  ImageExist(g.charactercreatorEditorImageoffset)  ==  1  )  DeleteImage (  g.charactercreatorEditorImageoffset );
	//  hide entities

	//  "hide" all entities in map by moving them out the way
	characterkit_resetcolors ( );

		for ( t.tcce = 1 ; t.tcce<=  g.entityelementlist; t.tcce++ )
		{
			t.tccentid=t.entityelement[t.tcce].bankindex;
			if (  t.tccentid>0 ) 
			{
				t.tccsourceobj = t.entityelement[t.tcce].obj;
				if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
				{
					PositionObject (  t.tccsourceobj,0,0,0 );
				}
			}
		}
		t.ttimeToUpdateAllCharacterCreatorEntitiesInMap = 0;
		characterkit_updateAllCharacterCreatorEntitiesInMap ( );

	//  show loading message
	t.tjustloadedcc=1;

	//  setup weapon options
	Dim (  t.characterkitweaponbank_s,7  );
	t.characterkitweaponbank_s[1] = "colt1911";
	t.characterkitweaponbank_s[2] = "Uzi";
	t.characterkitweaponbank_s[3] = "SniperM700";
	t.characterkitweaponbank_s[4] = "Shotgun";
	t.characterkitweaponbank_s[5] = "RPG";
	t.characterkitweaponbank_s[6] = "Magnum357";
	t.characterkitweaponbank_s[7] = "None";
	t.characterkitweaponmax = 7;

	t.characterkitcontrol.isMale = 1;

	//  Scan for all body and head parts
	t.characterkit.body_s="male_Civilian_1_royals";
	t.characterkit.head_s="male_Civilian_1";
	t.characterkit.facialhair_s="none";
	t.characterkit.hat_s="none";
	t.characterkit.bodyindex=0;
	t.characterkit.headindex=0;
	t.characterkit.facialhairindex=0;
	t.characterkit.hatindex=0;
	t.tolddir_s=GetDir();
	t.characterkitcontrol.originalDir_s = t.tolddir_s;
	SetDir (  "characterkit\\bodyandhead" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
		if (  strcmp ( Right(t.tfile_s.Get(),9) , "_head.cci" ) == 0 ) 
		{
				++g.characterkitheadmax;
				if ( cstr (Lower(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-9))) == cstr (Lower(t.characterkit.head_s.Get()) ) ) 
				{
					t.characterkit.headindex = g.characterkitheadmax;
					t.characterkit.originalHeadIndex = g.characterkitheadmax;
				}
				Dim (  t.characterkitheadbank_s,g.characterkitheadmax );
				t.characterkitheadbank_s[g.characterkitheadmax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-9);
				if (  t.characterkitheadbank_s[g.characterkitheadmax] == t.characterkit.head_s ) 
				{
					t.characterkit.headindex=g.characterkitheadmax;
				}
			}
			else
			{
				if (  cstr(Right(t.tfile_s.Get(),4)) == ".cci" ) 
				{
					++g.characterkitbodymax;
					if (  cstr(Lower(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4)))  ==  cstr(Lower(t.characterkit.body_s.Get()))  )  t.characterkit.bodyindex  =  g.characterkitbodymax;
					Dim (  t.characterkitbodybank_s,g.characterkitbodymax );
					t.characterkitbodybank_s[g.characterkitbodymax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
					if (  t.characterkitbodybank_s[g.characterkitbodymax] == t.characterkit.body_s  )  t.characterkit.bodyindex = g.characterkitbodymax;
				}
			}
		}
	}
	SetDir (  t.tolddir_s.Get() );

	SetDir (  "characterkit\\FacialHair" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			if (  cstr(Right(t.tfile_s.Get(),4)) == ".cci" ) 
			{
				++t.characterkitfacialhairmax;
				if (  cstr(Lower(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4)))  ==  cstr(Lower(t.characterkit.facialhair_s.Get()))  )  t.characterkit.facialhairindex  =  t.characterkitfacialhairmax;
				Dim (  t.characterkitfacialhairbank_s,t.characterkitfacialhairmax  );
				t.characterkitfacialhairbank_s[t.characterkitfacialhairmax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
				if (  t.characterkitfacialhairbank_s[t.characterkitfacialhairmax] == t.characterkit.facialhair_s  )  t.characterkit.facialhairindex = t.characterkitfacialhairmax;
			}
		}
	}
	SetDir (  t.tolddir_s.Get() );

	SetDir (  "characterkit\\Headattachments" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			if (  cstr(Right(t.tfile_s.Get(),4)) == ".cci" ) 
			{
				if (  cstr(Lower(Left(t.tfile_s.Get(),7)))  !=  "1_fmale" && cstr(Lower(Left(t.tfile_s.Get(),7)))  !=  "2_fmale" ) 
				{
					++t.characterkithatmax;
					if (  cstr(Lower(Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4)))  ==  cstr(Lower(t.characterkit.hat_s.Get()))  )  t.characterkit.hatindex  =  t.characterkithatmax;
					Dim (  t.characterkithatbank_s,t.characterkithatmax  );
					t.characterkithatbank_s[t.characterkithatmax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
					if (  t.characterkithatbank_s[t.characterkithatmax] == t.characterkit.hat_s.Get()  )  t.characterkit.hatindex = t.characterkithatmax;
				}
			}
		}
	}

	//  female heads
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			if (  cstr(Right(t.tfile_s.Get(),4)) == ".cci" ) 
			{
				if (  cstr(Lower(Left(t.tfile_s.Get(),7)))  ==  "1_fmale" || cstr(Lower(Left(t.tfile_s.Get(),7)))  ==  "2_fmale" || cstr(Lower(Left(t.tfile_s.Get(),4)))  ==  "none" ) 
				{
					++t.characterkitfemalehatmax;
					Dim (  t.characterkitfemalehatbank_s,t.characterkitfemalehatmax  );
					t.characterkitfemalehatbank_s[t.characterkitfemalehatmax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
				}
			}
		}
	}
	SetDir (  t.tolddir_s.Get() );

	//  Check if panels already exist
	if (  t.characterkit.panelsExist  ==  0 ) 
	{
		++g.slidersmenumax;
		t.characterkit.panelsExist = 1;
		t.characterkit.properties1Index = g.slidersmenumax;
		t.slidersmenu[g.slidersmenumax].tabpage=CHARACTERKITTABPAGE1;
		t.slidersmenu[g.slidersmenumax].title_s=" ";
		t.slidersmenu[g.slidersmenumax].thighlight=-1;
		t.slidersmenu[g.slidersmenumax].titlemargin=30;
		t.slidersmenu[g.slidersmenumax].leftmargin=25;
		t.slidersmenu[g.slidersmenumax].itemcount=12; // set below too
		t.slidersmenu[g.slidersmenumax].panelheight=30+(t.slidersmenu[g.slidersmenumax].itemcount*38);
		t.slidersmenu[g.slidersmenumax].ttop= (GetChildWindowHeight() / 2 ) - 281 -3;
		t.slidersmenu[g.slidersmenumax].tleft= (GetChildWindowWidth() ) - 265;
		t.slidersmenuvalue[g.slidersmenumax][1].name_s = "Character View Angle";
		t.slidersmenuvalue[g.slidersmenumax][1].value = 180;
		t.slidersmenuvalue[g.slidersmenumax][1].readmodeindex = 0;
		t.slidersmenuvalue[g.slidersmenumax][1].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][1].valueMax = 360;
		t.slidersmenuvalue[g.slidersmenumax][2].name_s = "Character View Height";
		t.slidersmenuvalue[g.slidersmenumax][2].value = 5;
		t.slidersmenuvalue[g.slidersmenumax][2].readmodeindex = 0;
		t.slidersmenuvalue[g.slidersmenumax][2].useCustomRange = 1;
		t.slidersmenuvalue[g.slidersmenumax][2].valueMin = 0;
		t.slidersmenuvalue[g.slidersmenumax][2].valueMax = 10;

		t.slidersmenuvalue[g.slidersmenumax][3].name_s = "";
		t.slidersmenuvalue[g.slidersmenumax][3].gadgettype = -1;
		t.slidersmenuvalue[g.slidersmenumax][3].gadgettypevalue = 51;
		t.slidersmenuvalue[g.slidersmenumax][4].name_s = "";
		t.slidersmenuvalue[g.slidersmenumax][4].gadgettype = -1;
		t.slidersmenuvalue[g.slidersmenumax][4].gadgettypevalue = 52;
		t.slidersmenuvalue[g.slidersmenumax][5].name_s = "";
		t.slidersmenuvalue[g.slidersmenumax][5].gadgettype = -1;
		t.slidersmenuvalue[g.slidersmenumax][5].gadgettypevalue = 53;
		t.slidersmenuvalue[g.slidersmenumax][6].name_s = "";
		t.slidersmenuvalue[g.slidersmenumax][6].gadgettype = -1;
		t.slidersmenuvalue[g.slidersmenumax][6].gadgettypevalue = 54;
		for ( t.c = 7 ; t.c<=  11; t.c++ )
		{
			t.slidersmenuvalue[g.slidersmenumax][t.c].name_s = "";
			t.slidersmenuvalue[g.slidersmenumax][t.c].gadgettype = 99;
			t.slidersmenuvalue[g.slidersmenumax][t.c].gadgettypevalue = -1;
		}

		// 050416 - parental control removes WEAPON Choice
		if ( g.quickparentalcontrolmode != 2 )
		{
			t.slidersmenuvalue[g.slidersmenumax][12].name_s = "Weapon";
			t.slidersmenuvalue[g.slidersmenumax][12].value = 1;
			t.slidersmenuvalue[g.slidersmenumax][12].value_s = "Colt1911";
			t.slidersmenuvalue[g.slidersmenumax][12].gadgettype = 1;
			t.slidersmenuvalue[g.slidersmenumax][12].gadgettypevalue = 55;
		}
		else
		{
			t.slidersmenu[g.slidersmenumax].itemcount = 11;
		}

		t.slidersmenuindex=g.slidersmenumax;
		t.characterkit.slidersmenumax=g.slidersmenumax;
	}

	//  Activate character kit
	t.characterkit.loaded=1;
	t.characterkitcontrol.oldtabmode = g.tabmode;
	g.tabmode=CHARACTERKITTABPAGE1;
	t.toldslidersmenumax = g.slidersmenumax;
	g.slidersmenumax=t.characterkit.slidersmenumax;
	for ( t.tn = 3 ; t.tn<=  6; t.tn++ )
	{
		t.slidersmenuvaluechoice=t.slidersmenuvalue[g.slidersmenumax][t.tn].gadgettypevalue;
		if (  t.tn == 3  )  t.slidersmenuvalue[g.slidersmenumax][t.tn].value  =  t.characterkit.bodyindex;
		if (  t.tn == 4  )  t.slidersmenuvalue[g.slidersmenumax][t.tn].value  =  t.characterkit.headindex;
		if (  t.tn == 5  )  t.slidersmenuvalue[g.slidersmenumax][t.tn].value  =  t.characterkit.facialhairindex;
		if (  t.tn == 6  )  t.slidersmenuvalue[g.slidersmenumax][t.tn].value  =  t.characterkit.hatindex;
		t.slidersmenuvalueindex=t.slidersmenuvalue[g.slidersmenumax][t.tn].value;
		sliders_getnamefromvalue ( );
		t.slidersmenuvalue[g.slidersmenumax][t.tn].value_s=t.slidervaluename_s;
	}

	//  Load resources
	characterkit_loadEffects ( );

	if (  ImageExist(t.characterkit.imagestart+0)  ==  1  )  DeleteImage (  t.characterkit.imagestart+0 );
	LoadImage (  "effectbank\\reloaded\\media\\blank_low_S.DDS",t.characterkit.imagestart+0 );
	if (  ImageExist(t.characterkit.imagestart+1)  ==  1  )  DeleteImage (  t.characterkit.imagestart+1 );
	LoadImage (  "effectbank\\reloaded\\media\\blank_O.DDS",t.characterkit.imagestart+1 );
	if (  ImageExist(t.characterkit.imagestart+2)  ==  1  )  DeleteImage (  t.characterkit.imagestart+2 );
	LoadImage (  "effectbank\\reloaded\\media\\blank_I.DDS",t.characterkit.imagestart+2 );

	//  Extra buttons (use importer buttons)
//  `txpos = (GetChildWindowWidth() / 2) + 360

	t.txpos = (GetChildWindowWidth() ) - 230;
	for ( t.t = 0 ; t.t <=  12 ; t.t++ ) t.importerTabs[t.t].label="" ;

	t.importerTabs[1].x = t.txpos;
	t.importerTabs[1].y = (GetChildWindowHeight() / 2) -180;
	t.importerTabs[1].label = "Head";
	t.importerTabs[1].selected = 0;
	t.importerTabs[1].tabpage = -1;

	t.importerTabs[2].x = t.txpos;
	t.importerTabs[2].y = (GetChildWindowHeight() / 2) -180 + 32;
	t.importerTabs[2].label = "Body";
	t.importerTabs[2].selected = 0;
	t.importerTabs[2].tabpage = -1;

	t.importerTabs[3].x = t.txpos;
	t.importerTabs[3].y = (GetChildWindowHeight() / 2) -180 + 64;
	t.importerTabs[3].label = "Head Attachement";
	t.importerTabs[3].selected = 0;
	t.importerTabs[3].tabpage = -1;

	t.importerTabs[4].x = t.txpos;
	t.importerTabs[4].y = (GetChildWindowHeight() / 2) -180 + 96;
	t.importerTabs[4].label = "Facial Hair";
	t.importerTabs[4].selected = 0;
	t.importerTabs[4].tabpage = -1;


	t.importerTabs[7].x = t.txpos;
	t.importerTabs[7].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 32;
	t.importerTabs[7].label = "Skin Tone";
	t.importerTabs[7].selected = 0;
	t.importerTabs[7].tabpage = -1;

	t.importerTabs[8].x = t.txpos;
	t.importerTabs[8].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 64;
	t.importerTabs[8].label = "Upper Clothes Tint";
	t.importerTabs[8].selected = 0;
	t.importerTabs[8].tabpage = -1;
	
	t.importerTabs[9].x = t.txpos;
	t.importerTabs[9].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 96;
	t.importerTabs[9].label = "Facial Hair Tint";
	t.importerTabs[9].selected = 0;
	t.importerTabs[9].tabpage = -1;
	
	t.importerTabs[10].x = t.txpos;
	t.importerTabs[10].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 128;
	t.importerTabs[10].label = "Lower Clothes Tint";
	t.importerTabs[10].selected = 0;
	t.importerTabs[10].tabpage = -1;
	
	t.importerTabs[11].x = t.txpos;
	t.importerTabs[11].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 160;
	t.importerTabs[11].label = "Shoes Tint";
	t.importerTabs[11].selected = 0;
	t.importerTabs[11].tabpage = -1;
	
	t.importerTabs[12].x = t.txpos;
	t.importerTabs[12].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 192;
	t.importerTabs[12].label = "Head Attachement Tint";
	t.importerTabs[12].selected = 0;
	t.importerTabs[12].tabpage = -1;

	t.importerTabs[5].x = t.txpos;
	t.importerTabs[5].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 192 +42;
	t.importerTabs[5].label = "Save Character";
	t.importerTabs[5].selected = 0;
	t.importerTabs[5].tabpage = -1;

	t.importerTabs[6].x = 40;
	t.importerTabs[6].y = 40;
	t.importerTabs[6].label = "Back to Editor";
	t.importerTabs[6].selected = 0;
	t.importerTabs[6].tabpage = -1;

	if (  ImageExist(t.characterkit.imagestart+41) == 0  )  LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\tab_wide.png",t.characterkit.imagestart+41,1 );
	if (  ImageExist(t.characterkit.imagestart+42) == 0  )  LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\tabselected_wide.png",t.characterkit.imagestart+42,1 );

	if (  ImageExist(t.characterkit.imagestart+43) == 0  )  LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\paneltop.png",t.characterkit.imagestart+43,1 );
	if (  ImageExist(t.characterkit.imagestart+44) == 0  )  LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\panelmiddle.png",t.characterkit.imagestart+44,1 );
	if (  ImageExist(t.characterkit.imagestart+45) == 0  )  LoadImage (  "languagebank\\neutral\\gamecore\\huds\\importer\\panelbottom.png",t.characterkit.imagestart+45,1 );

return;

}

void characterkit_loadEffects ( void )
{
	if (  t.characterkit.effectforcharacter == 0 ) 
	{
		t.characterkit.effectforcharacter = g.charactercreatoreffectbankoffset ; LoadEffect (  "effectbank\\reloaded\\character_editor.fx" , g.charactercreatoreffectbankoffset, 0 );
		t.characterkit.effectforcharacterHighlight = g.charactercreatoreffectbankoffset+1 ; LoadEffect (  "effectbank\\reloaded\\character_editor.fx",g.charactercreatoreffectbankoffset+1,0 );
		//t.characterkit.effectforAttachments = g.charactercreatoreffectbankoffset+2 ; LoadEffect (  "effectbank\\reloaded\\entity_basic.fx",g.charactercreatoreffectbankoffset+2,0 );
		//t.characterkit.effectforAttachmentsHighlight = g.charactercreatoreffectbankoffset+3 ; LoadEffect (  "effectbank\\reloaded\\entity_basic.fx",g.charactercreatoreffectbankoffset+3,0 );
		//t.characterkit.effectforBeard = g.charactercreatoreffectbankoffset+4 ; LoadEffect (  "effectbank\\reloaded\\entity_basic.fx",g.charactercreatoreffectbankoffset+4,0 );
		//t.characterkit.effectforBeardHighlight = g.charactercreatoreffectbankoffset+5 ; LoadEffect (  "effectbank\\reloaded\\entity_basic.fx",g.charactercreatoreffectbankoffset+5,0 );
		t.characterkit.effectforAttachments = g.charactercreatoreffectbankoffset+2 ; LoadEffect (  "effectbank\\reloaded\\character_static.fx",g.charactercreatoreffectbankoffset+2,0 );
		t.characterkit.effectforAttachmentsHighlight = g.charactercreatoreffectbankoffset+3 ; LoadEffect (  "effectbank\\reloaded\\character_static.fx",g.charactercreatoreffectbankoffset+3,0 );
		t.characterkit.effectforBeard = g.charactercreatoreffectbankoffset+4 ; LoadEffect (  "effectbank\\reloaded\\character_static.fx",g.charactercreatoreffectbankoffset+4,0 );
		t.characterkit.effectforBeardHighlight = g.charactercreatoreffectbankoffset+5 ; LoadEffect (  "effectbank\\reloaded\\character_static.fx",g.charactercreatoreffectbankoffset+5,0 );
	}
	t.tnothing = MakeVector4(g.characterkitvector);
	SetVector4 (  g.characterkitvector,0.7f,0.7f,0.7f,1.0f );
	SetEffectConstantV (  t.characterkit.effectforcharacterHighlight, "AmbiColor" , g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforAttachmentsHighlight, "AmbiColor" , g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforBeardHighlight, "AmbiColor" , g.characterkitvector );
	SetVector4 (  g.characterkitvector,0.5,0.5,0.5,1.0 );
	SetEffectConstantV (  t.characterkit.effectforcharacter, "AmbiColor" , g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforAttachments, "AmbiColor" , g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforBeard, "AmbiColor" , g.characterkitvector );

	SetVector4 (  g.characterkitvector,500000,0,0,0 );
	SetEffectConstantV (  t.characterkit.effectforAttachments,"EntityEffectControl",g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforAttachmentsHighlight,"EntityEffectControl",g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforBeard,"EntityEffectControl",g.characterkitvector );
	SetEffectConstantV (  t.characterkit.effectforBeardHighlight,"EntityEffectControl",g.characterkitvector );

	t.tnothing = DeleteVector4(g.characterkitvector);
}

void characterkit_resetcolors ( void )
{
	t.tnewred_f = -1;
	t.tnewredshirt_f = -1;
	t.tnewredtrousers_f = -1;
	t.tnewredshoes_f = -1;
	t.tnewredbeard_f = -1;
	t.tnewredhat_f = -1;

	t.characterkit.skinPickOn = 0;
	t.characterkit.haveSkinColor = 0;
	t.characterkit.haveShirtColor = 0;
	t.characterkit.haveBeardColor = 0;
	t.characterkit.haveTrousersColor = 0;
	t.characterkit.haveShoesColor = 0;
	t.characterkit.haveHatColor = 0;

return;

}

void characterkit_free ( void )
{

	characterkit_cleanupCustomHead ( );

	//  show all waypoints
	waypoint_showall ( );

	//  put all entities back where they were
	for ( t.tcce = 1 ; t.tcce<=  g.entityelementlist; t.tcce++ )
	{
		t.tccentid=t.entityelement[t.tcce].bankindex;
		if (  t.tccentid>0 ) 
		{
			t.tccsourceobj = t.entityelement[t.tcce].obj;
			if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
			{
				PositionObject (  t.tccsourceobj, t.entityelement[t.tcce].x,t.entityelement[t.tcce].y,t.entityelement[t.tcce].z );
			}
		}
	}
	t.ttimeToUpdateAllCharacterCreatorEntitiesInMap = 0;
	characterkit_updateAllCharacterCreatorEntitiesInMap ( );

	characterkit_resetcolors ( );
	SetDir (  t.characterkitcontrol.originalDir_s.Get() );
	g.tabmode = t.characterkitcontrol.oldtabmode;
	g.slidersmenumax = t.toldslidersmenumax;

	if (  t.characterkit.effectforcharacter == 1 ) 
	{
		DeleteEffect (  t.characterkit.effectforcharacter );
		DeleteEffect (  t.characterkit.effectforcharacterHighlight );
		DeleteEffect (  t.characterkit.effectforAttachments );
		DeleteEffect (  t.characterkit.effectforAttachmentsHighlight );
		DeleteEffect (  t.characterkit.effectforBeard );
		DeleteEffect (  t.characterkit.effectforBeardHighlight );
		t.characterkit.effectforcharacter=0;
	}

	g.characterkitheadmax=0;
	g.characterkitbodymax=0;
	t.characterkitfacialhairmax=0;
	t.characterkithatmax=0;
	t.characterkitfemalehatmax=0;

	for ( t.tloop = t.characterkit.imagestart ; t.tloop<=  t.characterkit.imagestart + 50; t.tloop++ )
	{
		if (  ImageExist(t.tloop)  ==  1  )  DeleteImage (  t.tloop );
	}

	for ( t.tloop = t.characterkit.objectstart ; t.tloop<=  t.characterkit.objectstart+4; t.tloop++ )
	{
		if (  ObjectExist(t.tloop)  ==  1  )  DeleteObject (  t.tloop );
	}

	t.characterkitcontrol.init = 0;
	t.characterkit.loaded = 0;
	t.characterkitcontrol.customHeadMode = 0;
	
	UnDim (  t.characterkithatbank_s );
	UnDim (  t.characterkitfemalehatbank_s );
	UnDim (  t.characterkitfacialhairbank_s );
	UnDim (  t.characterkitheadbank_s );
	UnDim (  t.characterkitbodybank_s );
	UnDim (  t.characterkitweaponbank_s );

	t.characterkit.bodyindexloaded=-1;
	t.characterkit.headindexloaded=-1;
	t.characterkit.facialhairindexloaded=-1;
	t.characterkit.hatindexloaded=-1;

	//  Update characters already in editor
	t.trecolorAfterGame = 1;
}

void characterkit_loop ( void )
{
	t.tccmessage_s = "";

	if (  t.characterkitcontrol.init  ==  0 ) 
	{
		//  Load resources
		characterkit_init ( );
		t.characterkitcontrol.init = 1;
	}
	if (  t.characterkit.loaded == 1 ) 
	{
		// keep shaders refreshed if exists
		if ( GetEffectExist ( t.characterkit.effectforcharacter ) == 1 )
		{
			SetEffectConstantF ( t.characterkit.effectforcharacter,"ShadowStrength", 0 );
			SetEffectConstantF ( t.characterkit.effectforcharacterHighlight,"ShadowStrength", 0 );
			SetEffectConstantF ( t.characterkit.effectforAttachments,"ShadowStrength", 0 );
			SetEffectConstantF ( t.characterkit.effectforAttachmentsHighlight,"ShadowStrength", 0 );
			SetEffectConstantF ( t.characterkit.effectforBeard,"ShadowStrength", 0 );
			SetEffectConstantF ( t.characterkit.effectforBeardHighlight,"ShadowStrength", 0 );
		}

		//  Get input from IDE
		if (  g.slidersprotoworkmode == 1 ) 
		{
			//  Keyboard
			t.inputsys.kscancode=ScanCode();
			//  Mouse
			t.inputsys.xmouse=MouseX();
			t.inputsys.ymouse=MouseY();
			t.inputsys.mclick=MouseClick();
			t.inputsys.xmousemove=MouseMoveX();
		}
		else
		{
			//  Multiplier to convert mouse coords to importer coords
			t.tadjustedtoccxbase_f=GetChildWindowWidth()/800.0;
			t.tadjustedtoccybase_f=GetChildWindowHeight()/600.0;
			t.tccoldmousex = t.inputsys.xmouse;
			t.tccoldmousey = t.inputsys.ymouse;
			t.inputsys.xmouse = t.inputsys.xmouse*t.tadjustedtoccxbase_f;
			t.inputsys.ymouse = t.inputsys.ymouse*t.tadjustedtoccybase_f;
		}

		//  if using custom head, match arms to head
		if (  t.characterkitcontrol.hasCustomHead  ==  1 && t.characterkitcontrol.customHeadMode  ==  0 ) 
		{
			characterkit_colorMatchBody ( );
		}

		//  Handle extra buttons
		if (  t.inputsys.mclick  ==  0 ) 
		{
			for ( t.tCount5 = 1 ; t.tCount5<=  12; t.tCount5++ )
			{
				if (  t.inputsys.xmouse  >=  t.importerTabs[t.tCount5].x && t.inputsys.xmouse  <=  t.importerTabs[t.tCount5].x + 188 ) 
				{
					if (  t.inputsys.ymouse  >=  t.importerTabs[t.tCount5].y && t.inputsys.ymouse  <=  t.importerTabs[t.tCount5].y + 32 ) 
					{
						t.importerTabs[t.tCount5].selected = 1;
					}
					else
					{
						t.importerTabs[t.tCount5].selected = 0;
					}
				}
				else
				{
					t.importerTabs[t.tCount5].selected = 0;
				}
			}
		}

		//  head
		if (  t.importerTabs[1].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[1].selected = 0;
			//  is making a custom head, this will match the colors of the skin to the photo imported
			if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
			{
				t.tmakeFaceTexture = 1;
			}
			else
			{
				t.characterkit.selected = 2;
				t.characterkit.thumbGadgetOn = 1 ; t.characterkit.oldMouseClick = 1 ; t.characterkit.loadthumbs = 1;
			}
		}

		if (  t.importerTabs[2].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[2].selected = 0;
			if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
			{
				//  If already in sample mode, head back to normal
				if (  t.characterkitcontrol.customHeadMode  ==  10 ) 
				{
					t.characterkitcontrol.customHeadMode = 3;

					//  get rid of the sample sprites
					for ( t.tloop = 0 ; t.tloop<=  2; t.tloop++ )
					{
						if (  t.ccSampleSprite[t.tloop] > 0 ) 
						{
							if (  SpriteExist(t.ccSampleSprite[t.tloop])  )  DeleteSprite (  t.ccSampleSprite[t.tloop] );
						}
					}

				}
				else
				{
					//  switch on sample mode
					t.characterkitcontrol.customHeadMode = 10;
				}
				t.characterkitcontrol.samplerMode = 0;
			}
			else
			{
				t.characterkit.selected = 1;
				t.characterkit.thumbGadgetOn = 1 ; t.characterkit.oldMouseClick = 1 ; t.characterkit.loadthumbs = 1;
			}
		}

		if (  t.importerTabs[3].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[3].selected = 0;
			t.characterkit.selected = 4;
			t.characterkit.thumbGadgetOn = 1 ; t.characterkit.oldMouseClick = 1 ; t.characterkit.loadthumbs = 1;
		}

		if (  t.importerTabs[4].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[4].selected = 0;
			t.characterkit.selected = 3;
			t.characterkit.thumbGadgetOn = 1 ; t.characterkit.oldMouseClick = 1 ; t.characterkit.loadthumbs = 1;
		}

		if (  t.importerTabs[5].selected  ==  1 && t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0 ) 
		{
			if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
			{
				t.characterkitcontrol.customHeadMode = 4;
				t.importerTabs[5].selected = 0;
				t.characterkit.oldMouseClick = 1;
			}
			else
			{
				t.importerTabs[5].selected = 0;
				characterkit_save_entity ( );
			}
		}
		if (  t.importerTabs[6].selected  ==  1 && t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0 ) 
		{
			if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
			{
				t.characterkitcontrol.customHeadMode = 5;
				t.importerTabs[6].selected = 0;
				t.characterkit.oldMouseClick = 1;
			}
			else
			{
				t.importerTabs[6].selected = 0;
				characterkit_free ( );
			}
			return;
		}
		if (  t.importerTabs[7].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[7].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 0;
			t.characterkit.oldMouseClick = 1;
		}
		if (  t.importerTabs[8].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[8].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 1;
			t.characterkit.oldMouseClick = 1;
		}
		if (  t.importerTabs[9].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[9].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 2;
			t.characterkit.oldMouseClick = 1;
		}
		if (  t.importerTabs[10].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[10].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 3;
			t.characterkit.oldMouseClick = 1;
		}
		if (  t.importerTabs[11].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[11].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 4;
			t.characterkit.oldMouseClick = 1;
		}
		if (  t.importerTabs[12].selected  ==  1 && t.inputsys.mclick  ==  1 ) 
		{
			t.importerTabs[12].selected = 0;
			t.characterkit.skinPickOn = 1;
			t.tuseOtherMask = 5;
			t.characterkit.oldMouseClick = 1;
		}

		//  ensure the sliders panel is in the correct place
		characterkit_alignUI ( );

		if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
		{
			characterkit_customHead ( );
			return;
		}

		//  Allow picking with mouse
		characterkit_mousePick ( );
		//  Rotate using mouse
		characterkit_mouseRotate ( );

		//  update thumb gadget
		characterkit_thumbgadget ( );

		//  Control character kit object view
		characterkit_update_object ( );

		//pick skin tone
		if ( t.characterkit.skinPickOn == 1 )characterkit_pickSkinTone ( );

	}

	t.characterkit.oldMouseClick = t.inputsys.mclick;
	t.inputsys.xmouse = t.tccoldmousex;
	t.inputsys.ymouse = t.tccoldmousey;
}

void characterkit_alignUI ( void )
{

	t.slidersmenu[t.characterkit.properties1Index].ttop= (GetChildWindowHeight() / 2 ) - 281 -3;
	t.slidersmenu[t.characterkit.properties1Index].tleft= (GetChildWindowWidth() ) - 265;

	if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
	{
		t.slidersmenuvalue[t.characterkit.properties1Index][1].gadgettype = -1;
		t.slidersmenuvalue[t.characterkit.properties1Index][2].gadgettype = -1;
		t.slidersmenuvalue[t.characterkit.properties1Index][12].gadgettype = -1;

		t.slidersmenuvalue[t.characterkit.properties1Index][1].name_s = "";
		t.slidersmenuvalue[t.characterkit.properties1Index][2].name_s = "";
		t.slidersmenuvalue[t.characterkit.properties1Index][12].name_s = "";

	}
	else
	{
		t.slidersmenuvalue[t.characterkit.properties1Index][1].gadgettype = 0;
		t.slidersmenuvalue[t.characterkit.properties1Index][2].gadgettype = 0;
		t.slidersmenuvalue[t.characterkit.properties1Index][12].gadgettype = 1;

		t.slidersmenuvalue[t.characterkit.properties1Index][1].name_s = "Character View Angle";
		t.slidersmenuvalue[t.characterkit.properties1Index][2].name_s = "Character View Height";
		t.slidersmenuvalue[t.characterkit.properties1Index][12].name_s = "Weapon";

	}

	t.txpos = (GetChildWindowWidth() ) - 230;
	if (  t.characterkitcontrol.customHeadMode  !=  0  )  t.txpos  =  10000;

	t.toffset = 2;

	t.importerTabs[3].x = t.txpos;
	t.importerTabs[3].y = (GetChildWindowHeight() / 2) -180;

	t.importerTabs[1].x = t.txpos;
	t.importerTabs[1].y = (GetChildWindowHeight() / 2) -180 + 32;

	t.importerTabs[4].x = t.txpos;
	t.importerTabs[4].y = (GetChildWindowHeight() / 2) -180 + 64;

	t.importerTabs[2].x = t.txpos;
	t.importerTabs[2].y = (GetChildWindowHeight() / 2) -180 + 96;

	t.importerTabs[7].x = t.txpos;
	t.importerTabs[7].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 32 + t.toffset;

	t.importerTabs[12].x = t.txpos;
	t.importerTabs[12].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 64 + t.toffset;

	t.importerTabs[9].x = t.txpos;
	t.importerTabs[9].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 96 + t.toffset;

	t.importerTabs[8].x = t.txpos;
	t.importerTabs[8].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 128 + t.toffset;

	t.importerTabs[10].x = t.txpos;
	t.importerTabs[10].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 160 + t.toffset;

	t.importerTabs[11].x = t.txpos;
	t.importerTabs[11].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 192 + t.toffset;

	t.importerTabs[5].x = t.txpos;
	t.importerTabs[5].y = (GetChildWindowHeight() / 2) -180 + 96 + 10 + 192 +90;

	t.importerTabs[6].x = 40;
	t.importerTabs[6].y = 40;
}

void characterkit_draw ( void )
{
	if (  t.characterkitcontrol.customHeadMode  !=  0 ) 
	{
		t.txpos = (GetChildWindowWidth() ) - 230;
		for ( t.t = 0 ; t.t <= 12 ; t.t++ ) t.importerTabs[t.t].x = 10000 ; 
		//  normal head editing
		if (  t.characterkitcontrol.customHeadMode  !=  10 ) 
		{
			t.importerTabs[1].label = "Match Face Color";
			t.importerTabs[1].x = t.txpos;
			t.importerTabs[2].label = "Adjust Sampling Spots";
			t.importerTabs[2].x = t.txpos;
			t.importerTabs[5].label = "Make Face";
			t.importerTabs[5].x = t.txpos;
		}
		else
		{
			t.importerTabs[2].label = "Finished Sampling Spots";
			t.importerTabs[2].x = t.txpos;
		//  sampler adjusting
		}
	}
	else
	{
		t.txpos = (GetChildWindowWidth() ) - 230;
		for ( t.t = 0 ; t.t <=  12 ; t.t++ ) t.importerTabs[t.t].x=t.txpos ;
		t.importerTabs[1].label = "Head";
		t.importerTabs[2].label = "Body";
		t.importerTabs[5].label = "Save Character";
		t.importerTabs[6].label = "Back to Editor";
		t.importerTabs[6].x = 40;
		if (  t.characterkitcontrol.hasCustomHead  ==  1  )  t.importerTabs[7].x  =  10000;
	}

	if (  t.tccmessage_s  !=  "*1" ) 
	{
		pastebitmapfont(t.tccmessage_s.Get(),(GetChildWindowWidth()/2) - (getbitmapfontwidth (t.tccmessage_s.Get(),1)/2),GetChildWindowHeight()-70,1,255);
	}
	else
	{
		pastebitmapfont("Left click the section of the object you wish to change",(GetChildWindowWidth()/2) - (getbitmapfontwidth ("Left click the section of the object you wish to change",1)/2),GetChildWindowHeight()-100,1,255);
		pastebitmapfont("or right click to rotate",(GetChildWindowWidth()/2) - (getbitmapfontwidth ("or right click to rotate",1)/2),GetChildWindowHeight()-70,1,255);
	}

	//  draw to screen if active
	if (  t.characterkit.loaded == 1 ) 
	{

		t.inputsys.xmouse = t.tccoldmousex;
		t.inputsys.ymouse = t.tccoldmousey;

		//  Draw the sliders
		t.characterkit.inUse = 1;
		sliders_readall ( );
		sliders_loop ( );
		sliders_draw ( );
		t.characterkit.inUse = 0;

	}

return;

}

void characterkit_draw_buttons ( void )
{
	//  Draw extra buttons
	for ( t.tcount = 1 ; t.tcount<=  12; t.tcount++ )
	{
		if (  t.importerTabs[t.tcount].selected  ==  1 ) 
		{
			PasteImage (  t.characterkit.imagestart+42,t.importerTabs[t.tcount].x,t.importerTabs[t.tcount].y );
		}
		else
		{
			PasteImage (  t.characterkit.imagestart+41,t.importerTabs[t.tcount].x,t.importerTabs[t.tcount].y );
		}
		pastebitmapfont(t.importerTabs[t.tcount].label.Get(),t.importerTabs[t.tcount].x + 94 - ((getbitmapfontwidth(t.importerTabs[t.tcount].label.Get(),2)) / 2) ,t.importerTabs[t.tcount].y+8,2,255);
	}
return;

}

void characterkit_update_object ( void )
{

	if (  ImageExist(t.characterkit.imagestart+25)  ==  0 ) 
	{
		LoadImage (  "languagebank\\neutral\\gamecore\\huds\\characterkit\\skintones.png",t.characterkit.imagestart+25 );
		LoadImage (  "languagebank\\neutral\\gamecore\\huds\\characterkit\\colorwheel.png",t.characterkit.imagestart+26 );
		LoadImage (  "languagebank\\neutral\\gamecore\\huds\\characterkit\\target.png",t.characterkit.imagestart+28 );
	}
	//  Asset loading and prep
	t.trestartanimationloop=0;
	for ( t.assetsequence = 1 ; t.assetsequence<=  4; t.assetsequence++ )
	{
		t.tokay=0;
		if (  t.assetsequence == 1 && t.characterkit.bodyindex != t.characterkit.bodyindexloaded  )  t.tokay = 1;
		//  if it is the head, we force reload on attachments also since they are glued to the head
		if (  t.assetsequence == 2 && t.characterkit.headindex != t.characterkit.headindexloaded ) 
		{
			t.tokay=1;
			if (  cstr(Lower(Left(t.characterkitheadbank_s[t.characterkit.headindex].Get(),5)))  ==  "fmale" || cstr(Lower(Left(t.characterkitheadbank_s[t.characterkit.headindex].Get(),7)))  ==  "1_fmale" ) 
			{
				if (  t.characterkitcontrol.isMale  ==  1 ) 
				{
					t.characterkit.hatindex = 1;
					t.slidersmenuindex=t.characterkit.properties1Index;
					t.slidersmenuvalue[t.slidersmenuindex][6].value = 1;
					t.slidersmenuvalue[t.slidersmenuindex][6].value_s = t.characterkitfemalehatbank_s[1];
				}
				t.characterkitcontrol.isMale = 0;
			}
			else
			{
				if (  t.characterkitcontrol.isMale  ==  0 ) 
				{
					t.characterkit.hatindex = 2;
					t.slidersmenuindex=t.characterkit.properties1Index;
					t.slidersmenuvalue[t.slidersmenuindex][6].value = 2;
					t.slidersmenuvalue[t.slidersmenuindex][6].value_s = t.characterkithatbank_s[2];
				}
				t.characterkitcontrol.isMale = 1;
			}
			t.characterkit.facialhairindexloaded = -1 ; t.characterkit.hatindexloaded = -1;
		}
		if (  t.assetsequence == 3 && t.characterkit.facialhairindex != t.characterkit.facialhairindexloaded  )  t.tokay = 1;
		if (  t.assetsequence == 4 && t.characterkit.hatindex != t.characterkit.hatindexloaded  )  t.tokay = 1;
		if (  t.tokay == 1 ) 
		{
			//  indicate which asset is being replaced by making it wireframe
			t.tobj=t.characterkit.objectstart+t.assetsequence;
			if (  ObjectExist(t.tobj) == 1  )  SetObjectWireframe (  t.tobj,1 );
			//  prompt user before load freeze
			for ( t.s = 0 ; t.s<=  1; t.s++ )
			{
				if (  t.tjustloadedcc  ==  1 ) 
				{
					pastebitmapfontcenter("LOADING CHARACTER CREATOR",GetChildWindowWidth()/2,40,1,255) ; Sync (  );
				}
				else
				{
					pastebitmapfontcenter("LOADING NEW ASSET",GetChildWindowWidth()/2,40,1,255) ; Sync (  );
				}
			}
			t.tjustloadedcc=0;
			//  load the asset
			if (  t.assetsequence == 1 ) { t.characterkit.bodyindexloaded = t.characterkit.bodyindex  ; t.characterkit.body_s = t.characterkitbodybank_s[t.characterkit.bodyindex] ; t.tpath_s = "characterkit\\bodyandhead\\" ; t.tasset_s = t.characterkit.body_s ; t.xextra_s = "" ; t.tassetimg = 11; }
			if (  t.assetsequence == 2 ) 
			{
				t.characterkit.headindexloaded=t.characterkit.headindex ; t.characterkit.head_s=t.characterkitheadbank_s[t.characterkit.headindex] ; t.tpath_s = "characterkit\\bodyandhead\\" ; t.tasset_s=t.characterkit.head_s ; t.xextra_s="_head" ; t.tassetimg=14;
				t.characterkitcontrol.hasCustomHead = 0;
				//  check for projection head
				if (  cstr(Lower(Left(t.characterkit.head_s.Get(),12)))  ==  "1_projection" ) { t.characterkitcontrol.customHeadMode  =  1  ; t.characterkitcontrol.fmaleProjection  =  0 ; t.tforceUpdateOfHead  =  60 ; t.characterkitcontrol.hasCustomHead  =  1; }
				if (  cstr(Lower(Left(t.characterkit.head_s.Get(),18)))  ==  "1_fmale_projection" ) { t.characterkitcontrol.customHeadMode  =  1  ; t.characterkitcontrol.fmaleProjection  =  1 ; t.tforceUpdateOfHead  =  60 ; t.characterkitcontrol.hasCustomHead  =  1; }
			}
				if (  t.assetsequence == 3 ) { t.characterkit.facialhairindexloaded = t.characterkit.facialhairindex  ; t.characterkit.facialhair_s = t.characterkitfacialhairbank_s[t.characterkit.facialhairindex] ; t.tpath_s  =  "characterkit\\facialhair\\" ; t.tasset_s = t.characterkit.facialhair_s ; t.xextra_s = "" ; t.tassetimg = 17; }
			if (  t.assetsequence == 4 ) 
			{
				t.characterkit.hatindexloaded=t.characterkit.hatindex;
				if (  t.characterkitcontrol.isMale  ==  1 ) 
				{
					t.characterkit.hat_s=t.characterkithatbank_s[t.characterkit.hatindex];
				}
				else
				{
					t.characterkit.hat_s=t.characterkitfemalehatbank_s[t.characterkit.hatindex];
				}
				t.tpath_s = "characterkit\\Headattachments\\";
				t.tasset_s=t.characterkit.hat_s;
				t.xextra_s="" ; t.tassetimg=19;
			}
			t.tcciloadname_s = t.tpath_s+t.tasset_s+t.xextra_s+".cci";
			characterkit_loadCCI ( );
			if (  t.tccimesh_s  ==  ""  )  break;
			t.tobj=t.characterkit.objectstart+t.assetsequence;
			if (  FileExist(t.tccimesh_s.Get())  ==  0  )  ExitPrompt (  cstr( cstr("Can't find ")+t.tccimesh_s).Get(), "Character Creator Error" );
			if (  ObjectExist(t.tobj) == 1  )  DeleteObject (  t.tobj );
			/*      
			while (  SpaceKey()  ==  0 ) 
			{
				SetCursor (  0,0 );
				Cls (  );
				Print (  t.tasset_s+t.xextra_s+".X" );
				Sync (  );
			}
			}
			*/    
			LoadObject (  t.tccimesh_s.Get(),t.tobj );
//    `for tcount = 0 to 3

//     `set character creator tones tobj,tcount,-1,0,0,0.5

//    `next tcount

			if (  ImageExist(t.characterkit.imagestart+t.tassetimg+0) == 1  )  DeleteImage (  t.characterkit.imagestart+t.tassetimg+0 );
			if (  ImageExist(t.characterkit.imagestart+t.tassetimg+1) == 1  )  DeleteImage (  t.characterkit.imagestart+t.tassetimg+1 );
			if (  FileExist(t.tccidiffuse_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccidiffuse_s).Get(), "Character Creator Error" );
			LoadImage (  t.tccidiffuse_s.Get(),t.characterkit.imagestart+t.tassetimg+0 );
			if (  FileExist(t.tccinormal_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccinormal_s).Get(), "Character Creator Error" );
			LoadImage (  t.tccinormal_s.Get(),t.characterkit.imagestart+t.tassetimg+1 );
			//  if head glue to body
			if (  t.assetsequence  ==  2 ) 
			{
				if (  ObjectExist(t.characterkit.objectstart+1) ) 
				{
					t.tSourcebip01_head=getlimbbyname(t.characterkit.objectstart+1, "Bip01_Head");
					GlueObjectToLimbEx (  t.tobj,t.characterkit.objectstart+1,t.tSourcebip01_head,2 );
				}
			}
			//  the mask, if head or body
			if (  t.assetsequence < 3 ) 
			{
			//characterkit_makeMask ( );
				if (  FileExist(t.tccimask_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccimask_s).Get(), "Character Creator Error" );
				LoadImage (  t.tccimask_s.Get(),t.characterkit.imagestart+t.tassetimg+2 );
			}
			TextureObject (  t.tobj,0,t.characterkit.imagestart+t.tassetimg+0 );
			TextureObject (  t.tobj,1,t.characterkit.imagestart+1 );
			TextureObject (  t.tobj,2,t.characterkit.imagestart+t.tassetimg+1 );
			TextureObject (  t.tobj,3,t.characterkit.imagestart+0 );
			TextureObject (  t.tobj,4,t.characterkit.imagestart+2 );
			TextureObject (  t.tobj,5,t.characterkit.imagestart+2 );
			TextureObject (  t.tobj,6,t.characterkit.imagestart+2 );
			if (  t.assetsequence < 3 ) 
			{
				TextureObject (  t.tobj,11,t.characterkit.imagestart+t.tassetimg+2 );
				SetObjectEffect (  t.tobj,t.characterkit.effectforcharacter );
				//  head or body
				t.tnewred_f = -1;

				//  body
				if (  t.assetsequence  ==  1 ) 
				{
					t.tnewredshirt_f = -1;
					t.tnewredtrousers_f = -1;
					t.tnewredshoes_f = -1;
					SetCharacterCreatorTones (  t.tobj,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
					if (  ObjectExist ( t.tobj+1)  ==  1  )  SetCharacterCreatorTones (  t.tobj+1,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 ) ;
					SetCharacterCreatorTones (  t.tobj,1,t.tnewredshirt_f,t.tnewgreenshirt_f,t.tnewblueshirt_f,0.5 );
					SetCharacterCreatorTones (  t.tobj,2,t.tnewredtrousers_f,t.tnewgreentrousers_f,t.tnewbluetrousers_f,0.5 );
					SetCharacterCreatorTones (  t.tobj,3,t.tnewredshoes_f,t.tnewgreenshoes_f,t.tnewblueshoes_f,0.5 );
				}
				else
				{
					SetCharacterCreatorTones (  t.tobj,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
					if (  ObjectExist ( t.tobj-1)  ==  1  )  SetCharacterCreatorTones (  t.tobj-1,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 ) ;
				}
			}
			if (  t.assetsequence  ==  4 ) 
			{
				TextureObject (  t.tobj,11,t.characterkit.imagestart+1 );
				SetObjectEffectCore (  t.tobj,t.characterkit.effectforAttachments,1 );
			}
			if (  t.assetsequence  ==  3 ) 
			{
				TextureObject (  t.tobj,11,t.characterkit.imagestart+1 );
				SetObjectEffectCore (  t.tobj,t.characterkit.effectforBeard,1 );
			}
			SetEffectTechnique (  t.characterkit.effectforcharacter,"Highest" );
			SetEffectTechnique (  t.characterkit.effectforBeard,"Highest" );
			SetEffectTechnique (  t.characterkit.effectforAttachments,"Highest" );

			//  Facial hair
			if (  t.assetsequence  ==  3 ) 
			{
				if (  ObjectExist(t.characterkit.objectstart+2) ) 
				{
					t.tBip01_FacialHair=getlimbbyname(t.characterkit.objectstart+2, "Bip01_FacialHair");

					if (  ObjectExist(t.tobj)  ==  1 ) 
					{
//       `position object tobj,LimbPositionX(t.characterkit.objectstart+2,tBip01_FacialHair),LimbPositionY(t.characterkit.objectstart+2,tBip01_FacialHair),LimbPositionZ(t.characterkit.objectstart+2,tBip01_FacialHair)

//       `offset limb t.characterkit.objectstart+2,tBip01_FacialHair,0,0,-1

//       `scale limb t.characterkit.objectstart+2,tBip01_FacialHair,650,650,650

						GlueObjectToLimbEx (  t.tobj,t.characterkit.objectstart+2,t.tBip01_FacialHair,2 );
					}
				}
				SetCharacterCreatorTones (  t.tobj,0,t.tnewredbeard_f,t.tnewgreenbeard_f,t.tnewbluebreard_f,0.5 );
				SetCharacterCreatorTones (  t.tobj,1,-1,0,0,0.0 );
				SetCharacterCreatorTones (  t.tobj,2,-1,0,0,0.0 );
				SetCharacterCreatorTones (  t.tobj,3,-1,0,0,0.0 );
			}
			//  Hat
			if (  t.assetsequence  ==  4 ) 
			{
				t.Bip01_Headgear=getlimbbyname(t.characterkit.objectstart+2, "Bip01_Headgear");
				if (  ObjectExist(t.characterkit.objectstart+2) ) 
				{
					if (  t.Bip01_Headgear  <=  0  )  t.Bip01_Headgear = getlimbbyname(t.characterkit.objectstart+2, "Bip01_FacialHair");
					if (  ObjectExist(t.tobj)  ==  1 ) 
					{
//       `position object tobj,LimbPositionX(t.characterkit.objectstart+2,tBip01_FacialHair),LimbPositionY(t.characterkit.objectstart+2,tBip01_FacialHair),LimbPositionZ(t.characterkit.objectstart+2,tBip01_FacialHair)

//       `offset limb t.characterkit.objectstart+2,Bip01_Headgear,0,0,-1

//       `scale limb t.characterkit.objectstart+2,Bip01_Headgear,630,630,630

						GlueObjectToLimbEx (  t.tobj,t.characterkit.objectstart+2,t.Bip01_Headgear,2 );
					}
				}
			}

			LockObjectOn (  t.tobj );
			SetObjectRotationZYX (  t.tobj );
			SetObjectLOD (  t.tobj,1,1000 );
			SetObjectLOD (  t.tobj,2,1500 );
			t.trestartanimationloop=1;

			t.characterkit_diffuse_s[t.assetsequence] = t.tccidiffuse_s;
			t.characterkit_normal_s[t.assetsequence] = t.tccinormal_s;
			t.characterkit_mask_s[t.assetsequence] = t.tccimask_s;

		}

	}
	if (  t.trestartanimationloop == 1 ) 
	{
		for ( t.assetsequence = 1 ; t.assetsequence<=  1; t.assetsequence++ )
		{
			t.tobj=t.characterkit.objectstart+t.assetsequence;
			if (  ObjectExist(t.tobj) == 1 ) 
			{
				LoopObject (  t.tobj,3000,3100 );
				SetObjectFrame (  t.tobj,3000 );
			}
		}
	}

	//  Monitor when character attrib changes
	t.slidersmenuindex=t.characterkit.properties1Index;
	for ( t.assetsequence = 1 ; t.assetsequence<=  1; t.assetsequence++ )
	{
		t.tobj=t.characterkit.objectstart+t.assetsequence;
		if (  ObjectExist(t.tobj) == 1 ) 
		{
			YRotateObject (  t.tobj,t.slidersmenuvalue[t.slidersmenuindex][1].value-180 );
			XRotateObject (  t.tobj,0-(t.slidersmenuvalue[t.slidersmenuindex][2].value*3) );
			PositionObject (  t.tobj,0,-25-t.slidersmenuvalue[t.slidersmenuindex][2].value,85+(t.slidersmenuvalue[t.slidersmenuindex][2].value*3) );
		}
	}
	t.characterkit.bodyindex=t.slidersmenuvalue[t.slidersmenuindex][3].value;
	t.characterkit.headindex=t.slidersmenuvalue[t.slidersmenuindex][4].value;
	t.characterkit.facialhairindex=t.slidersmenuvalue[t.slidersmenuindex][5].value;
	t.characterkit.hatindex=t.slidersmenuvalue[t.slidersmenuindex][6].value;
}

void characterkit_save_entity ( void )
{

	//  Store old dir
	t.tolddir_s=GetDir();

	//  Check if user folder exists, if not create it
	if (  PathExist( cstr( g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() )  ==  0 ) 
	{
		MakeDirectory (  cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() );
	}
	if (  PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator").Get() )  ==  0 ) 
	{
		MakeDirectory (  cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator").Get() );
	}

	ShowMouse (  );

	//  Save dialog
	t.tSaveFile_s="";
	t.tentityprotected=1;
	t.ttitlemessage_s="Save Entity";
	while (  t.tentityprotected == 1 ) 
	{
		t.tSaveFile_s=openFileBox("FPSC Entity (.fpe)|*.fpe|All Files|*.*|", cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\").Get(), t.ttitlemessage_s.Get(), ".fpe", CHARACTERKITSAVEFILE);
		if (  t.tSaveFile_s == "Error"  )  return;
		t.tentityprotected=0;
	}

	t.tname_s = t.tSaveFile_s;
	if (  cstr(Lower(Right(t.tname_s.Get(),4)))  ==  ".fpe"  )  t.tname_s  =  Left(t.tname_s.Get(),Len(t.tname_s.Get())-4);
	for ( t.tloop = Len(t.tname_s.Get()) ; t.tloop>=  1 ; t.tloop+= -1 )
	{
		if (  cstr(Mid(t.tname_s.Get(),t.tloop))  ==  "\\" || cstr(Mid(t.tname_s.Get(),t.tloop))  ==  "/" ) 
		{
			t.tname_s = Right(t.tname_s.Get(),Len(t.tname_s.Get())- t.tloop);
			break;
		}
	}

//  `tSaveFile$ = fpscrootdir$+"\\Files\\entitybank\\user\\charactercreator\\" + tname$+".fpe"


	t.tcopyfrom_s = g.fpscrootdir_s+"\\Files\\entitybank\\Characters\\Uber Soldier.fpe";
	t.tcopyto_s = t.tSaveFile_s;
	if (  cstr(Lower(Right(t.tcopyto_s.Get(),4)))  !=  ".fpe"  )  t.tcopyto_s  =  t.tcopyto_s + ".fpe";

	if (  FileExist(t.tcopyto_s.Get())  ==  1  )  DeleteAFile (  t.tcopyto_s.Get() );

	if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
	if (  FileOpen(2)  ==  1  )  CloseFile (  2 );

	OpenToRead (  1, t.tcopyfrom_s.Get() );
	OpenToWrite (  2,t.tcopyto_s.Get() );

	t.tcount = 0;
	while (  FileEnd(1)  ==  0 ) 
	{
		t.ts_s = ReadString ( 1 );
		//  adjust offset for male/female
		if (  cstr(Lower(Left(t.ts_s.Get(),4)))  ==  "offy" ) 
		{
			t.ts_s = "offy          = -6";
			if (  cstr(Lower(Left(t.ts_s.Get(),5)))  ==  "fmale" || cstr(Lower(Left(t.ts_s.Get(),7)))  ==  "1_fmale"  )  t.ts_s  =  "offy           =  -4";
		}
		if (  cstr(Lower(Left(t.ts_s.Get(),4)))  ==  "desc"  )  t.ts_s  =  cstr("desc           =  ") + t.tname_s;
		if (  cstr(Lower(Left(t.ts_s.Get(),9)))  ==  "hasweapon" ) 
		{
			t.ts_s = "";
			if ( strlen( t.slidersmenuvalue[t.characterkit.properties1Index][12].value_s.Get()) > 1 )
			{
				t.ts_s = cstr("hasweapon     = modern\\") + t.slidersmenuvalue[t.characterkit.properties1Index][12].value_s;
			}
			if (  t.slidersmenuvalue[t.characterkit.properties1Index][12].value_s  ==  "None"  )  t.ts_s  =  "";
		}

		WriteString (  2,t.ts_s.Get() );
		++t.tcount;
		if (  t.tcount  ==  2 ) 
		{
			WriteString (  2,"" );
			WriteString (  2,";character creator" );
			t.tcc_s = "charactercreator = t.v:2:";
			t.tcc_s = t.tcc_s + t.characterkit.body_s+":"+t.characterkit.head_s+":"+t.characterkit.facialhair_s+":"+t.characterkit.hat_s+":";
			t.tcc_s = t.tcc_s +Str(t.tnewred_f)+":"+Str(t.tnewgreen_f)+":"+Str(t.tnewblue_f)+":";
			t.tcc_s = t.tcc_s +Str(t.tnewredshirt_f)+":"+Str(t.tnewgreenshirt_f)+":"+Str(t.tnewblueshirt_f)+":";
			t.tcc_s = t.tcc_s +Str(t.tnewredbeard_f)+":"+Str(t.tnewgreenbeard_f)+":"+Str(t.tnewbluebreard_f)+":";
			t.tcc_s = t.tcc_s +Str(t.tnewredtrousers_f)+":"+Str(t.tnewgreentrousers_f)+":"+Str(t.tnewbluetrousers_f)+":";
			t.tcc_s = t.tcc_s +Str(t.tnewredshoes_f)+":"+Str(t.tnewgreenshoes_f)+":"+Str(t.tnewblueshoes_f)+":";
			t.tcc_s = t.tcc_s +Str(t.tnewredhat_f)+":"+Str(t.tnewgreenhat_f)+":"+Str(t.tnewbluehat_f);
			WriteString (  2,t.tcc_s.Get() );
			if (  cstr(Lower(Left(t.characterkit.head_s.Get(),5)))  ==  "fmale" || cstr(Lower(Left(t.characterkit.head_s.Get(),7)))  ==  "1_fmale" ) 
			{
				WriteString (  2,";sound" );
				WriteString (  2,"soundset      = Female" );
			}
		}
	}
	
	CloseFile (  1 );
	CloseFile (  2 );
	

	//  Save Thumbnail
	CreateBitmap (  32,64,64 );
//	CreateBitmap ( 32, 1600, 500 );
	SetCurrentBitmap (  32 );
	SetCameraAspect (  1.0 );
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
	//MakeObjectSphere (  t.twhiteobj, 500 );
	MakeObjectBox (  t.twhiteobj,200000,200000,1 );
	SetObjectLight (  t.twhiteobj,0 );
	LockObjectOn (  t.twhiteobj );
	PositionObject (  t.twhiteobj,0,0,1000 );
	SetObjectEffect ( t.twhiteobj, g.guishadereffectindex );
	SetObjectEmissive ( t.twhiteobj, Rgb(255,255,255) );

	for ( t.tloop = 1 ; t.tloop<=  4; t.tloop++ )
	{
		if (  t.tloop  ==  1 ) 
		{
			t.tobj = t.characterkit.objectstart+t.tloop;
			if (  ObjectExist(t.tobj)  ==  1 ) 
			{
				YRotateObject (  t.tobj,0 );
				XRotateObject (  t.tobj,0-(5*3) );
				PositionObject (  t.tobj,0,-25-5,85+(5*3) );
				MoveObject (  t.tobj, -70 );
				XRotateObject (  t.tobj,0-13 );
				MoveObjectUp (  t.tobj,-16 );
				YRotateObject (  t.tobj,-10 );
			}
		}
	}
	bool b6002 = false; if ( ObjectExist(6002)==1) { b6002 = GetVisible(6002); HideObject(6002); }
	bool b6003 = false; if ( ObjectExist(6003)==1) { b6003 = GetVisible(6003); HideObject(6003); }
	Sync (  );
	if ( b6002 == true ) ShowObject ( 6002 );
	if ( b6003 == true ) ShowObject ( 6003 );
	for ( t.tloop = 1 ; t.tloop<=  4; t.tloop++ )
	{
		if (  t.tloop  ==  1 ) 
		{
			t.tobj = t.characterkit.objectstart+t.tloop;
			if (  ObjectExist(t.tobj)  ==  1 ) 
			{
				YRotateObject (  t.tobj,0 );
				XRotateObject (  t.tobj,0-(5*3) );
				PositionObject (  t.tobj,0,-25-5,85+(5*3) );
			}
		}
	}

	t.tSaveThumb_s = cstr(Left(t.tcopyto_s.Get(),Len(t.tcopyto_s.Get())-4)) + ".bmp";
	if (  FileExist ( t.tSaveThumb_s.Get())  ==  1  )  DeleteAFile (  t.tSaveThumb_s.Get() ) ;
	GrabImage (  g.importermenuimageoffset+50,0,0,64,64,3 );
	//GrabImage (  g.importermenuimageoffset+50,0,0,1600,500,3 );
	t.tSprite = 100;
	while ( SpriteExist (t.tSprite) == 1 ) 
	{
		++t.tSprite;
	}
	SaveImage (  t.tSaveThumb_s.Get(), g.importermenuimageoffset+50 );

	///Sprite ( 43125, 50, 50, g.importermenuimageoffset+50 );
	DeleteImage (  g.importermenuimageoffset+50 );

	DeleteObject (  t.twhiteobj );

	//  check for custom head texture
	t.tcustomeHeadSave_s = cstr(Left(t.tcopyto_s.Get(),Len(t.tcopyto_s.Get())-4)) + "_cc.dds";
	if (  FileExist(t.tcustomeHeadSave_s.Get())  ==  1  )  DeleteAFile (  t.tcustomeHeadSave_s.Get() );
	if (  cstr(Lower(Left(t.characterkit.head_s.Get(),12)))  ==  "1_projection" || cstr(Lower(Left(t.characterkit.head_s.Get(),18)))  ==  "1_fmale_projection" ) 
	{
		if (  ImageExist(t.characterkit.imagestart+14)  ==  1 ) 
		{
			SaveImage (  t.tcustomeHeadSave_s.Get(),t.characterkit.imagestart+14 );
		}
	}

	//  restore camera view
	t.aspect_f=GetDesktopWidth() ; t.aspect_f=t.aspect_f/GetDesktopHeight();
	SetCameraAspect (  t.aspect_f );
	SetCurrentBitmap (  0 );
	characterkit_draw ( );
	Sync (  );

	//  Restore old dir
	SetDir (  t.tolddir_s.Get() );
}

void characterkit_makeMultiplayerCharacterCreatorAvatar ( void )
{
	//  delete it if it exists (it shouldn't, but just in case)
	if (  FileExist(t.avatarFile_s.Get())  ==  1  )  DeleteAFile (  t.avatarFile_s.Get() );

	//  Store old dir
	t.tolddir_s=GetDir();

	//  Check if user folder exists, if not create it
	if (  PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() )  ==  0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user").Get() );
	}
	if (  PathExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator").Get() )  ==  0 ) 
	{
		MakeDirectory ( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator").Get() );
	}

	t.tSaveFile_s = t.avatarFile_s;

	t.tname_s = t.tSaveFile_s;
	if (  cstr(Lower(Right(t.tname_s.Get(),4)))  ==  ".fpe"  )  t.tname_s  =  Left(t.tname_s.Get(),Len(t.tname_s.Get())-4);
	for ( t.tloop = Len(t.tname_s.Get()) ; t.tloop >= 1 ; t.tloop+= -1 )
	{
		if (  cstr(Mid(t.tname_s.Get(),t.tloop))  ==  "\\" || cstr(Mid(t.tname_s.Get(),t.tloop))  ==  "/" ) 
		{
			t.tname_s = Right(t.tname_s.Get(),Len(t.tname_s.Get())- t.tloop);
			break;
		}
	}

	t.tcopyfrom_s = g.fpscrootdir_s+"\\Files\\entitybank\\Characters\\Uber Soldier.fpe";
	t.tcopyto_s = t.tSaveFile_s;
	if (  cstr(Lower(Right(t.tcopyto_s.Get(),4)))  !=  ".fpe"  )  t.tcopyto_s  =  t.tcopyto_s + ".fpe";

	if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
	if (  FileOpen(2)  ==  1  )  CloseFile (  2 );

	OpenToRead (  1, t.tcopyfrom_s.Get() );
	OpenToWrite (  2,t.tcopyto_s.Get() );

	t.tcount = 0;
	while (  FileEnd(1)  ==  0 ) 
	{
		t.ts_s = ReadString ( 1 );
		//  adjust offset for male/female
		if (  cstr(Lower(Left(t.ts_s.Get(),4)))  ==  "offy" ) 
		{
			t.ts_s = "offy          = -6";
			if (  cstr(Lower(Left(t.ts_s.Get(),5)))  ==  "fmale"  )  t.ts_s  =  "offy           =  -4";
		}
		if (  cstr(Lower(Left(t.ts_s.Get(),4)))  ==  "desc"  )  t.ts_s  =  cstr("desc           =  ") + t.tname_s;

		WriteString (  2,t.ts_s.Get() );
		++t.tcount;
		if (  t.tcount  ==  2 ) 
		{
			WriteString (  2,"" );
			WriteString (  2,";character creator" );
			WriteString (  2, cstr(cstr("charactercreator = ") + t.avatarString_s).Get() );
		}
	}

	CloseFile (  1 );
	CloseFile (  2 );

	//  Restore old dir
	SetDir (  t.tolddir_s.Get() );

return;

//  remove temp avatar fpe
//  needs avatarFile$
}

void characterkit_removeMultiplayerCharacterCreatorAvatar ( void )
{
	if (  FileExist(t.avatarFile_s.Get())  ==  1  )  DeleteAFile (  t.avatarFile_s.Get() );
return;

}

void characterkit_save_fpe ( void )
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
	if (  t.slidersmenuvalue[t.importer.properties1Index][5].value == 2 ) 
	{
		t.importer.objectFPE.defaultstatic = "0";
	}
	else
	{
		t.importer.objectFPE.defaultstatic = "1";
	}
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
	if (  t.slidersmenuvalue[t.importer.properties1Index][7].value == 2 ) 
	{
		t.importer.objectFPE.ischaracter = "0";
	}
	else
	{
		t.importer.objectFPE.ischaracter = "1";
	}
	//  isobjective
	t.importer.objectFPE.isobjective = Str(t.slidersmenuvalue[t.importer.properties1Index][8].value);
	//  cantakeweapon
	t.importer.objectFPE.cantakeweapon = "0";
	//  update shader selection
	t.importer.objectFPE.effect = cstr(cstr("effectbank\\reloaded\\") + t.slidersmenuvalue[t.importer.properties1Index][2].value_s);
	//  aimain
	t.importer.objectFPE.aimain = t.slidersmenuvalue[t.importer.properties1Index][11].value_s;
	//t.tString as string;
	t.chosenFileNameFPE_s = t.importer.tFPESaveName;
	if (  FileOpen (1)  )  CloseFile (1) ;
	if (  FileExist (  t.chosenFileNameFPE_s.Get() )  )  DeleteAFile (  t.chosenFileNameFPE_s.Get() ) ;
	OpenToWrite (  1, t.chosenFileNameFPE_s.Get() );
		t.tString = ";Saved by GameGuru Object Importer v1.000" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";header" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("desc")) + "= " + t.importer.objectFPE.desc ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";visualinfo" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("textured")) + "= " + t.importer.objectFPE.textured ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("effect")) + "= " + t.importer.objectFPE.effect ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("castshadow")) + "= " + t.importer.objectFPE.castshadow ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("transparency")) + "= " + t.importer.objectFPE.transparency ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";orientation" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("model")) + "= " + t.importer.objectFPE.model ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("offx")) + "= " + t.importer.objectFPE.offx ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("offy")) + "= " + t.importer.objectFPE.offy ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("offz")) + "= " + t.importer.objectFPE.offz ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("rotx")) + "= " + t.importer.objectFPE.rotx ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("roty")) + "= " + t.importer.objectFPE.roty ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("rotz")) + "= " + t.importer.objectFPE.rotz ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("scale")) + "= " + t.importer.objectFPE.scale ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("collisionmode")) + "= " + t.importer.objectFPE.collisionmode ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("defaultstatic")) + "= " + t.importer.objectFPE.defaultstatic ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("materialindex")) + "= " + t.importer.objectFPE.materialindex ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";physics shapes" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("physicscount")) + "= " + Str(t.importer.collisionShapeCount) ; WriteString (  1, t.tString.Get() );
		t.tExportScale_f = 100.0 / t.importer.objectScaleForEditing;
		for ( t.tPCount = 0 ; t.tPCount<=  t.importer.collisionShapeCount - 1; t.tPCount++ )
		{
			t.tString = cstr(CharacterKitPadString( cstr(cstr("physics") + Str(t.tPCount)).Get() ) ) + cstr("= ") + Chr(34);
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
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";identity details" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("ischaracter")) + "= " + t.importer.objectFPE.ischaracter ; WriteString (  1 , t.tString.Get() );
		if ( g.quickparentalcontrolmode == 2 )
		{
			t.tString = cstr(CharacterKitPadString("hasweapon")) + "= "; 
			WriteString (  1 , t.tString.Get() );
		}
		else
		{
			t.tString = cstr(CharacterKitPadString("hasweapon")) + "= " + t.importer.objectFPE.hasweapon ; 
			WriteString (  1 , t.tString.Get() );
		}
		t.tString = cstr(CharacterKitPadString("isobjective")) + "= " + t.importer.objectFPE.isobjective ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("cantakeweapon")) + "= " + t.importer.objectFPE.cantakeweapon ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";statistics" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("strength")) + "= " + t.importer.objectFPE.strength ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("explodable")) + "= " + t.importer.objectFPE.explodable ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("debrisshape")) + "= " + t.importer.objectFPE.debrisshape ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";ai" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("aimain")) + "= " + t.importer.objectFPE.aimain ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";spawn" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("spawnmax")) + "= " + t.importer.objectFPE.spawnmax ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("spawndelay")) + "= " + t.importer.objectFPE.spawndelay ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("spawnqty")) + "= " + t.importer.objectFPE.spawnqty ; WriteString (  1 , t.tString.Get() );
		t.tString = "" ; WriteString (  1 , t.tString.Get() );
		t.tString = ";anim" ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("animmax")) + "= " + t.importer.objectFPE.animmax ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("anim0")) + "= " + t.importer.objectFPE.anim0 ; WriteString (  1 , t.tString.Get() );
		t.tString = cstr(CharacterKitPadString("playanimineditor")) + "= " + t.importer.objectFPE.playanimineditor ; WriteString (  1 , t.tString.Get() );
		//  Save any lines that were not understand when the initial FPE was loaded in
		//  add a blank line if there are extra lines to add
		if (  t.importer.unknownFPELineCount > 0 ) 
		{
			t.tString = "" ; WriteString (  1 , t.tString.Get() );
			t.tString = ";settings not understood by the exporter" ; WriteString (  1 , t.tString.Get() );
		}
	CloseFile (  1 );

return;

}

void characterkit_mousePick ( void )
{
	if (  t.importer.objectRotateMode  !=  0 || t.characterkit.thumbGadgetOn  ==  1 || t.characterkit.skinPickOn  ==  1   )  return;

	//  try attachments first
	t.tpick = 0;
	#ifdef DX11
	float fMX = (GetDisplayWidth()+0.0) / 800.0f;
	float fMY = (GetDisplayHeight()+0.0) / 600.0f;
	t.tadjustedtoareax_f = t.tccoldmousex*fMX;
	t.tadjustedtoareay_f = t.tccoldmousey*fMY;
	#else
	t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
	t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
	t.tadjustedtoareax_f=((t.tccoldmousex+0.0)/800.0)/t.tadjustedtoareax_f;
	t.tadjustedtoareay_f=((t.tccoldmousey+0.0)/600.0)/t.tadjustedtoareay_f;
	t.tadjustedtoareax_f=t.tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
	t.tadjustedtoareay_f=t.tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
	#endif
	if (  t.tpick  ==  0  )  t.tpick  =  PickScreenObjectEx ( t.tadjustedtoareax_f , t.tadjustedtoareay_f , t.characterkit.objectstart , t.characterkit.objectstart+10 , 1 ) ;
	if (  t.tpick  !=  0 ) 
	{
		//  long message, handles with two lines
		t.tccmessage_s = "*1";
		t.characterkit.selected = t.tpick - t.characterkit.objectstart;
		if (  t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0 ) { t.characterkit.thumbGadgetOn  =  1  ; t.characterkit.oldMouseClick  =  1 ; t.characterkit.loadthumbs  =  1; }
		if (  t.tpick  !=  t.characterkit.picked ) 
		{
			if (  t.tpick < t.characterkit.objectstart+3 ) 
			{
				SetObjectEffect (  t.tpick,t.characterkit.effectforcharacterHighlight );
			}
			if (  t.tpick  ==  t.characterkit.objectstart+4 ) 
			{
				SetObjectEffectCore (  t.tpick,t.characterkit.effectforAttachmentsHighlight,1 );
			}
			if (  t.tpick  ==  t.characterkit.objectstart+3 ) 
			{
				SetObjectEffectCore (  t.tpick,t.characterkit.effectforBeardHighlight,1 );
			}
			if (  t.characterkit.picked  !=  0 ) 
			{
				if (  t.characterkit.picked < t.characterkit.objectstart+3 ) 
				{
					SetObjectEffect (  t.characterkit.picked,t.characterkit.effectforcharacter );
				}
				if (  t.characterkit.picked  ==  t.characterkit.objectstart+4 ) 
				{
					SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforAttachments,1 );
				}
				if (  t.characterkit.picked  ==  t.characterkit.objectstart+3 ) 
				{
					SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforBeard,1 );
				}
			}
			t.characterkit.picked = t.tpick;
		}
	}
	else
	{
		if (  t.characterkit.picked  !=  0 ) 
		{
			if (  t.characterkit.picked < t.characterkit.objectstart+3 ) 
			{
				SetObjectEffect (  t.characterkit.picked,t.characterkit.effectforcharacter );
			}
			if (  t.characterkit.picked  ==  t.characterkit.objectstart+4 ) 
			{
				SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforAttachments,1 );
			}
			if (  t.characterkit.picked  ==  t.characterkit.objectstart+3 ) 
			{
				SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforBeard,1 );
			}
			t.characterkit.picked = 0;
		}
	}
}

void characterkit_mouseRotate ( void )
{
	if (  t.characterkit.thumbGadgetOn  ==  1 || t.characterkit.skinPickOn  )  return;
	//  Rotation control
	t.slidersmenuindex=t.characterkit.properties1Index;
	switch (  t.importer.objectRotateMode ) 
	{
		case 0:

			if (  t.inputsys.mclick  ==  2 && t.characterkit.oldMouseClick  ==  0 ) 
			{
				#ifdef DX11
				float fMX = (GetDisplayWidth()+0.0) / 800.0f;
				float fMY = (GetDisplayHeight()+0.0) / 600.0f;
				t.tadjustedtoareax_f = t.tccoldmousex*fMX;
				t.tadjustedtoareay_f = t.tccoldmousey*fMY;
				#else
				t.tadjustedtoareax_f=(GetDisplayWidth()+0.0)/(GetChildWindowWidth()+0.0);
				t.tadjustedtoareay_f=(GetDisplayHeight()+0.0)/(GetChildWindowHeight()+0.0);
				t.tadjustedtoareax_f=((t.tccoldmousex+0.0)/800.0)/t.tadjustedtoareax_f;
				t.tadjustedtoareay_f=((t.tccoldmousey+0.0)/600.0)/t.tadjustedtoareay_f;
				t.tadjustedtoareax_f=t.tadjustedtoareax_f*(GetChildWindowWidth()+0.0);
				t.tadjustedtoareay_f=t.tadjustedtoareay_f*(GetChildWindowHeight()+0.0);
				#endif
				if (  PickScreenObjectEx ( t.tadjustedtoareax_f , t.tadjustedtoareay_f , t.characterkit.objectstart , t.characterkit.objectstart+10 , 1 )  !=  0 )  
				{
					t.importer.objectRotateMode = 1;
					t.tnothing = t.inputsys.xmousemove;
					if (  t.characterkit.picked  !=  0 ) 
					{
						if (  t.tpick < t.characterkit.objectstart+3 ) 
						{
							SetObjectEffect (  t.characterkit.picked,t.characterkit.effectforcharacter );
						}
						if (  t.tpick  ==  t.characterkit.objectstart+4 ) 
						{
							SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforAttachments,1 );
						}
						if (  t.tpick  ==  t.characterkit.objectstart+3 ) 
						{
							SetObjectEffectCore (  t.characterkit.picked,t.characterkit.effectforBeard,1 );
						}
						t.characterkit.picked = 0;
					}
			}
			}

		break;
		case 1:

			if (  t.inputsys.mclick  ==  0 ) 
			{
				t.importer.objectRotateMode = 0;
				//importer_show_mouse ( );
			}
			else
			{

				t.slidersmenuvalue[t.slidersmenuindex][1].value -= t.inputsys.xmousemove;
				//  Ensure angle is 0-360
				while (  t.slidersmenuvalue[t.slidersmenuindex][1].value > 360 ) 
				{
					t.slidersmenuvalue[t.slidersmenuindex][1].value -= 360;
				}
				while (  t.slidersmenuvalue[t.slidersmenuindex][1].value < 0 ) 
				{
					t.slidersmenuvalue[t.slidersmenuindex][1].value += 360;
				}
			}

		break;

	}//~ return
return;

}

void characterkit_thumbgadget ( void )
{
	if (  t.characterkit.thumbGadgetOn  ==  1 ) 
	{
		if (  t.characterkit.picked  !=  0 ) 
		{
			if (  t.characterkit.picked < t.characterkit.objectstart+3 ) 
			{
				SetObjectEffect (  t.tpick,t.characterkit.effectforcharacter );
			}
			if (  t.characterkit.picked  ==  t.characterkit.objectstart+4 ) 
			{
				SetObjectEffectCore (  t.tpick,t.characterkit.effectforAttachments,1 );
			}
			if (  t.characterkit.picked  ==  t.characterkit.objectstart+3 ) 
			{
				SetObjectEffectCore (  t.tpick,t.characterkit.effectforBeard,1 );
			}
			t.characterkit.picked = 0;
		}
		if (  ImageExist(t.characterkit.imagestart+42) ) 
		{
				t.timg=t.characterkit.imagestart+42;
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
				Ink (  Rgb(255,255,255),0 );
				Box (  t.tscrx+50,t.tscry+ImageHeight(t.timg+1), t.tscrx+ImageWidth(t.timg+1)-50, t.tscry+ImageHeight(t.timg+1)+(ImageHeight(t.timg+2)*5) );
				t.tLeft = t.tscrx+50;
				t.tTop = t.tscry+ImageHeight(t.timg+1);
				t.tRight = t.tscrx+ImageWidth(t.timg+1)-85;
				t.tBottom = t.tscry+ImageHeight(t.timg+1)+(ImageHeight(t.timg+2)*5);
		}

		t.ttopx = t.tscrx+60;
		t.ttopy = t.tscry+ImageHeight(t.timg+1)+10;
		t.torigscrx = t.ttopx;
		t.torigscry = t.ttopy;

		//  draw the thumbs
		if (  t.characterkit.selected  ==  1 ) 
		{
			t.tpath_s = "characterkit\\bodyandhead\\";
			t.tmax = g.characterkitbodymax;
			t.tsliderindex = 3;
		}
		if (  t.characterkit.selected  ==  2 ) 
		{
			t.tpath_s = "characterkit\\bodyandhead\\";
			t.tmax = g.characterkitheadmax;
			t.tsliderindex = 4;
		}
		if (  t.characterkit.selected  ==  3 ) 
		{
			t.tpath_s = "characterkit\\FacialHair\\";
			t.tmax = t.characterkitfacialhairmax;
			t.tsliderindex = 5;
		}
		if (  t.characterkit.selected  ==  4 ) 
		{
			t.tpath_s = "characterkit\\Headattachments\\";
			if (  t.characterkitcontrol.isMale  ==  1 ) 
			{
				t.tmax = t.characterkithatmax;
			}
			else
			{
				t.tmax = t.characterkitfemalehatmax;
			}
			t.tsliderindex = 6;
		}
		//  99 is for picking online avatar
		if (  t.characterkit.selected  ==  99 ) 
		{
			t.tpath_s = "entitybank\\user\\charactercreator\\";
			t.tmax = t.characterkitavatarmax;
			t.tsliderindex = 99;
		}

		//  scroll bar if needed
		t.thowmanyrowsmax = 3;
		if (  t.characterkit.selected  ==  99  )  t.thowmanyrowsmax  =  6;
		if (  t.characterkit.howManyRows > t.thowmanyrowsmax ) 
		{
			t.tsub_f=15.0;
			t.tboxleft_f = t.tscrx+64.0+(140.0*4.0);
			t.tboxright_f = t.tscrx+64.0+(140.0*4.0)+23.0;
			t.tboxtop_f = t.tscry+ImageHeight(t.timg+1)+3.0 + t.characterkit.scrollPosition;
			t.tslidertop_f = t.tscry+ImageHeight(t.timg+1)+3.0;
			t.tboxbottom_f = t.tscry+ImageHeight(t.timg+1)-3.0+(ImageHeight(t.timg+1.0)*5.0) - (t.characterkit.howManyRows*t.tsub_f) + t.characterkit.scrollPosition;
			t.tsliderbottom_f = t.tscry+ImageHeight(t.timg+1)-3.0+(ImageHeight(t.timg+1.0)*5.0);
			//  see if we are over the scrollbar
			t.tscrollbarboost = 0;
			if (  t.inputsys.xmouse  >=  t.tboxleft_f && t.inputsys.xmouse  <=  t.tboxright_f ) 
			{
				if (  t.inputsys.ymouse  >=  t.tboxtop_f && t.inputsys.ymouse  <=  t.tboxbottom_f ) 
				{
					t.tscrollbarboost = 50;
					if (  t.inputsys.mclick  ==  1 && t.characterkit.scrollBarOn  ==  0 ) 
					{
						t.characterkit.scrollBarOn = 1;
						t.characterkit.scrollBarOffsetY = t.inputsys.ymouse-t.tboxtop_f;
					}
				}
			}
			//  update the scroll bar
			if (  t.characterkit.scrollBarOn  ==  1 ) 
			{
				t.tscrollbarboost = 50;
				if (  t.inputsys.mclick  ==  0  )  t.characterkit.scrollBarOn  =  0;
				t.tYPos_f = (t.inputsys.ymouse - t.tslidertop_f) - t.characterkit.scrollBarOffsetY;
				//  check bounds of slider
				t.tslidersize_f = t.tscry+ImageHeight(t.timg+1.0)-3.0+(ImageHeight(t.timg+1.0)*5.0) - (t.characterkit.howManyRows*t.tsub_f);
				if (  t.tYPos_f < 0  )  t.tYPos_f  =  0;
				if (  t.tYPos_f > (t.tsliderbottom_f-t.tslidersize_f)  )  t.tYPos_f  =  (t.tsliderbottom_f-t.tslidersize_f);
				t.characterkit.scrollPosition = t.tYPos_f;
			}
			//BoxGradient (  t.tboxleft_f,t.tboxtop_f,t.tboxright_f,t.tboxbottom_f, Rgb(102+t.tscrollbarboost,107+t.tscrollbarboost,112+t.tscrollbarboost),Rgb(102+t.tscrollbarboost,107+t.tscrollbarboost,112+t.tscrollbarboost),Rgb(36+t.tscrollbarboost,43+t.tscrollbarboost,50+t.tscrollbarboost),Rgb(36+t.tscrollbarboost,43+t.tscrollbarboost,50+t.tscrollbarboost) );
			GGBoxGradient (  t.tboxleft_f,t.tboxtop_f,t.tboxright_f,t.tboxbottom_f, 255, 102+t.tscrollbarboost, 107+t.tscrollbarboost, 112+t.tscrollbarboost );//),Rgb(102+t.tscrollbarboost,107+t.tscrollbarboost,112+t.tscrollbarboost),Rgb(36+t.tscrollbarboost,43+t.tscrollbarboost,50+t.tscrollbarboost),Rgb(36+t.tscrollbarboost,43+t.tscrollbarboost,50+t.tscrollbarboost );
		}

		if (  BitmapExist(2)  ==  0  )  CreateBitmap (  2,1024,1024 );
		SetCurrentBitmap (  2 );
		CLS (  Rgb(255,255,255) );
		t.ttopx = 0;
		t.ttopy = 0;

		for ( t.tloop = 1 ; t.tloop<=  t.tmax; t.tloop++ )
		{
			t.timg = t.characterkit.imagestart+60+t.tloop;
			if (  t.characterkit.loadthumbs  ==  1 ) 
			{
				t.tmax_f = t.tmax;
				t.characterkit.howManyRows = Ceil(t.tmax_f / 4.0);
				if (  t.characterkit.selected  ==  99  )  t.characterkit.howManyRows  =  Ceil(t.tmax_f / 8.0);
				if (  ImageExist(t.timg)  ==  1  )  DeleteImage (  t.timg );
				if (  t.characterkit.selected  ==  1  )  t.tbank_s  =  t.characterkitbodybank_s[t.tloop];
				if (  t.characterkit.selected  ==  2  )  t.tbank_s  =  t.characterkitheadbank_s[t.tloop]+"_head";
				if (  t.characterkit.selected  ==  3  )  t.tbank_s  =  t.characterkitfacialhairbank_s[t.tloop];
				if (  t.characterkit.selected  ==  4 ) 
				{
					if (  t.characterkitcontrol.isMale  ==  1 ) 
					{
						t.tbank_s = t.characterkithatbank_s[t.tloop];
					}
					else
					{
						t.tbank_s = t.characterkitfemalehatbank_s[t.tloop];
					}
				}

				if (  t.characterkit.selected  ==  99 ) 
				{
					t.tbank_s = t.characterkitavatarbank_s[t.tloop];
					t.tfiletoload_s = t.tpath_s+t.tbank_s+".bmp";
				}
				else
				{
					t.tfiletoload_s = t.tpath_s+t.tbank_s+"_thumb.png";
				}

				if (  FileExist(t.tfiletoload_s.Get())  ==  1 ) 
				{
					LoadImage (  t.tfiletoload_s.Get(),t.timg );
				}
				else
				{
					LoadImage (  "characterkit\\bodyandhead\\male_Civilian_1_head_thumb.png",t.timg );
				}
			}
			PasteImage (  t.timg, t.ttopx, t.ttopy - (t.characterkit.scrollPosition*6) );
			if (  t.characterkit.selected  !=  99 ) 
			{
				t.ttopx += 140;
				if (  t.ttopx > (140*3) ) 
				{
					t.ttopx = 0;
					t.ttopy += 140;
				}
			}
			else
			{
				t.ttopx += 70;
				if (  t.ttopx > (70*7) ) 
				{
					t.ttopx = 0;
					t.ttopy += 70;
				}
			}
		}
		if (  ImageExist(t.characterkit.imagestart+27)  ==  1  )  DeleteImage (  t.characterkit.imagestart+27 );
		GrabImage (  t.characterkit.imagestart+27,0,0,140*4,3*140,1 );
		SetCurrentBitmap (  0 );
		PasteImage (  t.characterkit.imagestart+27,t.torigscrx,t.torigscry );
		DeleteImage (  t.characterkit.imagestart+27 );

//   `set cursor 0,400

//   `print "howManyRows = " + Str(t.characterkit.howManyRows)


		t.ttopx = t.torigscrx;
		t.ttopy = t.torigscry;
		if (  t.characterkit.howManyRows > t.thowmanyrowsmax  )  t.ttopy -= (t.characterkit.scrollPosition*6);

		if (  t.characterkit.selected  !=  99 ) 
		{
			t.tpickedthumbx_f = Floor(((t.inputsys.xmouse - t.ttopx) / 140.0) + 1.0);
			t.tpickedthumby_f = Floor(((t.inputsys.ymouse - t.ttopy) / 140.0) + 1.0);
		}
		else
		{
			t.tpickedthumbx_f = Floor(((t.inputsys.xmouse - t.ttopx) / 70.0) + 1.0);
			t.tpickedthumby_f = Floor(((t.inputsys.ymouse - t.ttopy) / 70.0) + 1.0);
		}
//   `set cursor 0,0

//   `print tpickedthumbx#

//   `print tpickedthumby#

		if (  t.characterkit.selected  !=  99 ) 
		{
			t.tpickedthumb_f = (4.0 * (t.tpickedthumby_f-1.0)) + t.tpickedthumbx_f;
		}
		else
		{
			t.tpickedthumb_f = (8.0 * (t.tpickedthumby_f-1.0)) + t.tpickedthumbx_f;
		}
		t.tpickedthumb = t.tpickedthumb_f;
//   `print tpickedthumb

		//  highlight the selected one
		if (  t.characterkit.selected  !=  99 ) 
		{
			if (  t.tpickedthumb > 0 && t.tpickedthumb  <=  t.tmax && t.tpickedthumbx_f <=  4 ) 
			{
				t.tlinex_f = ((t.tpickedthumbx_f-1.0)*140)+t.torigscrx;
				t.tliney_f = t.torigscry+((t.tpickedthumby_f-1)*140)-5-(t.characterkit.scrollPosition*6);
				t.tlinemaxy_f = t.tliney_f + 140;
				if (  t.tlinemaxy_f > t.torigscry+(3*140)  )  t.tlinemaxy_f  =  t.torigscry+(3*140);
				if (  t.tliney_f < t.torigscry-5  )  t.tliney_f  =  t.torigscry-5;
				if (  t.tlinex_f  >=  t.torigscrx && t.tliney_f  <=  t.torigscry+(3*140) ) 
				{
					Ink (  Rgb(240,240,255),0 );
					Line (  t.tlinex_f,t.tliney_f,t.tlinex_f+140,t.tliney_f );
					Line (  t.tlinex_f,t.tliney_f,t.tlinex_f,t.tlinemaxy_f );
					Ink (  Rgb(220,220,240),0 );
					Line (  t.tlinex_f,t.tlinemaxy_f,t.tlinex_f+140,t.tlinemaxy_f );
					Line (  t.tlinex_f+140,t.tliney_f,t.tlinex_f+140,t.tlinemaxy_f );
					Ink (  Rgb(255,255,255),0 );
				}
			}
		}
		else
		{
			if (  t.tpickedthumb > 0 && t.tpickedthumb  <=  t.tmax && t.tpickedthumbx_f <=  8 ) 
			{
				t.tlinex_f = ((t.tpickedthumbx_f-1.0)*70)+t.torigscrx;
				t.tliney_f = t.torigscry+((t.tpickedthumby_f-1)*70)-5-(t.characterkit.scrollPosition*6);
				t.tlinemaxy_f = t.tliney_f + 70;
				if (  t.tlinemaxy_f > t.torigscry+(6*70)  )  t.tlinemaxy_f  =  t.torigscry+(6*70);
				if (  t.tliney_f < t.torigscry-5  )  t.tliney_f  =  t.torigscry-5;
				if (  t.tlinex_f  >=  t.torigscrx && t.tliney_f  <=  t.torigscry+(6*70) ) 
				{
					Ink (  Rgb(240,240,255),0 );
					Line (  t.tlinex_f,t.tliney_f,t.tlinex_f+70,t.tliney_f );
					Line (  t.tlinex_f,t.tliney_f,t.tlinex_f,t.tlinemaxy_f );
					Ink (  Rgb(220,220,240),0 );
					Line (  t.tlinex_f,t.tlinemaxy_f,t.tlinex_f+70,t.tlinemaxy_f );
					Line (  t.tlinex_f+70,t.tliney_f,t.tlinex_f+70,t.tlinemaxy_f );
					Ink (  Rgb(255,255,255),0 );
					steam_text(-1,95,2,t.characterkitavatarbank_s[t.tpickedthumb].Get());
				}
			}
	
		}
		if (  t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0 && t.characterkit.scrollBarOn  ==  0 ) 
		{
			t.characterkit.thumbGadgetOn = 0 ; t.characterkit.oldMouseClick = 1;
			//  ensure we arent clicking outside the Box (  )
			if (  t.inputsys.xmouse < t.tLeft || t.inputsys.ymouse < t.tTop || t.inputsys.xmouse > t.tRight || t.inputsys.ymouse > t.tBottom ) 
			{
				t.tpickedthumb = -1;
			}
			if (  t.tpickedthumb > 0 && t.tpickedthumb  <=  t.tmax ) 
			{
				if (  t.tsliderindex  !=  99 ) 
				{

					t.slidersmenuindex=t.characterkit.properties1Index;
					t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value = t.tpickedthumb;
					if (  t.tsliderindex  ==  3  )  t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value_s  =  t.characterkitbodybank_s[t.tpickedthumb];
					if (  t.tsliderindex  ==  4 ) 
					{
						t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value_s = t.characterkitheadbank_s[t.tpickedthumb];
						if (  cstr(Left(t.characterkitheadbank_s[t.tpickedthumb].Get(),12))  ==  "1_projection"  )  t.characterkit.headindexloaded  =  -1;
						if (  cstr(Left(t.characterkitheadbank_s[t.tpickedthumb].Get(),18))  ==  "1_fmale_projection"  )  t.characterkit.headindexloaded  =  -1;
						characterkit_update_object ( );
					}
					if (  t.tsliderindex  ==  5  )  t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value_s  =  t.characterkitfacialhairbank_s[t.tpickedthumb];
					if (  t.tsliderindex  ==  6 ) 
					{
						if (  t.characterkitcontrol.isMale  ==  1 ) 
						{
							t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value_s = t.characterkithatbank_s[t.tpickedthumb];
						}
						else
						{
							t.slidersmenuvalue[t.slidersmenuindex][t.tsliderindex].value_s = t.characterkitfemalehatbank_s[t.tpickedthumb];
						}
					}

				}
				else
				{

					t.tpickedAvatar_s = t.characterkitavatarbank_s[t.tpickedthumb];
				}
			}
		}
		t.characterkit.loadthumbs = 0;
	}
	else
	{
		t.characterkit.scrollPosition = 0;
	}
return;

//  requires tcciloadname$ to be set to the absolutely path to the file
//  requires tpath$ e.g tpath$ = "characterkit\\bodyandhead\\"
//  tccquick = 1 to exit after just getting names
}

void characterkit_loadCCI ( void )
{

	t.tccimesh_s = "";
	t.tccidiffuse_s = "";
	t.tccinormal_s = "";
	t.tccimask_s = "";

	if (  FileExist(t.tcciloadname_s.Get())  ==  0  )  return;
	if (  FileOpen(10)  )  CloseFile (  10 );

	OpenToRead (  10, t.tcciloadname_s.Get() );

		while (  FileEnd(10)  ==  0 ) 
		{
			t.tline_s = ReadString ( 10 );
			t.tcciStat_s = Lower(FirstToken( t.tline_s.Get(), " "));
			t.tcciValue_s = NextToken( " " );
			if (  t.tcciValue_s  ==  "="  )  t.tcciValue_s  =  NextToken( " " );
			if (  t.tcciStat_s  ==  "mesh"  )  t.tccimesh_s  =  t.tpath_s+t.tcciValue_s;
			if (  t.tcciStat_s  ==  "diffuse"  )  t.tccidiffuse_s  =  t.tpath_s+t.tcciValue_s;
			if (  t.tcciStat_s  ==  "normal"  )  t.tccinormal_s  =  t.tpath_s+t.tcciValue_s;
			if (  t.tcciStat_s  ==  "mask"  )  t.tccimask_s  =  t.tpath_s+t.tcciValue_s;
		}

	CloseFile (  10 );

	if (  t.tccquick  ==  1 ) 
	{
		t.tccquick = 0;
		return;
	}

	//  Don't reset tones on changing parts
	/*      
	//  body
	if (  t.assetsequence  ==  1 ) 
	{
		t.tnewred_f = -1;
		t.tnewredshirt_f = -1;
		t.tnewredtrousers_f = -1;
		t.tnewredshoes_f = -1;
	}
	//  head
	if (  t.assetsequence  ==  2 ) 
	{
		t.tnewred_f = -1;
	}
	//  beard
	if (  t.assetsequence  ==  3 ) 
	{
		t.tnewredbeard_f = -1;
	}
	*/    
	//  hat
	if (  t.assetsequence  ==  4 ) 
	{
	}

return;

}

void characterkit_pickSkinTone ( void )
{
	if (  t.tuseOtherMask  ==  0 ) 
	{
		PasteImage (  t.characterkit.imagestart+25,5,(GetChildWindowHeight()/2)-(ImageHeight(t.characterkit.imagestart+25)/2),1 );
	}
	else
	{
		PasteImage (  t.characterkit.imagestart+26,5,(GetChildWindowHeight()/2)-(ImageHeight(t.characterkit.imagestart+26)/2),1 );
	}

	t.tpointx = 5;
	t.tpointy = (GetChildWindowHeight()/2)-(ImageHeight(t.characterkit.imagestart+25)/2);
	//  if out of bounds exit
	if (  t.inputsys.xmouse < t.tpointx+37 || t.inputsys.xmouse > t.tpointx+481 || t.inputsys.ymouse < t.tpointy+64 || t.inputsys.ymouse > t.tpointy+444 ) 
	{
		if (  t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0  )  t.characterkit.skinPickOn  =  0;

		if (  t.tuseOtherMask  ==  0 ) 
		{
			t.tnewred_f = -1;
			t.characterkit.haveSkinColor = 0;
		}
		if (  t.tuseOtherMask  ==  1 ) 
		{
			t.tnewredshirt_f = -1;
			t.characterkit.haveShirtColor = 0;
		}
		if (  t.tuseOtherMask  ==  2 ) 
		{
			t.tnewredbeard_f = -1;
			t.characterkit.haveBeardColor = 0;
		}
		if (  t.tuseOtherMask  ==  3 ) 
		{
			t.tnewredtrousers_f = -1;
			t.characterkit.haveTrousersColor = 0;
		}
		if (  t.tuseOtherMask  ==  4 ) 
		{
			t.tnewredshoes_f = -1;
			t.characterkit.haveShoesColor = 0;
		}
		if (  t.tuseOtherMask  ==  5 ) 
		{
			t.tnewredhat_f = -1;
			t.characterkit.haveHatColor = 0;
		}
		if (  t.tuseOtherMask  ==  0 ) 
		{
			//  body and head skin
			t.tobj=t.characterkit.objectstart+1;
			SetCharacterCreatorTones (  t.tobj,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
			SetCharacterCreatorTones (  t.tobj+1,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
		}
		if (  t.tuseOtherMask  ==  1 ) 
		{
			t.tobj=t.characterkit.objectstart+1;
			SetCharacterCreatorTones (  t.tobj,1,t.tnewredshirt_f,t.tnewgreenshirt_f,t.tnewblueshirt_f,0.5 );
		}
		if (  t.tuseOtherMask  ==  2 ) 
		{
			t.tobj=t.characterkit.objectstart+3;
			SetCharacterCreatorTones (  t.tobj,0,t.tnewredbeard_f,t.tnewgreenbeard_f,t.tnewbluebreard_f,0.5 );
			SetCharacterCreatorTones (  t.tobj,1,-1,0,0,0.0 );
			SetCharacterCreatorTones (  t.tobj,2,-1,0,0,0.0 );
			SetCharacterCreatorTones (  t.tobj,3,-1,0,0,0.0 );
		}
		if (  t.tuseOtherMask  ==  3 ) 
		{
			t.tobj=t.characterkit.objectstart+1;
			SetCharacterCreatorTones (  t.tobj,2,t.tnewredtrousers_f,t.tnewgreentrousers_f,t.tnewbluetrousers_f,0.5 );
		}
		if (  t.tuseOtherMask  ==  4 ) 
		{
			t.tobj=t.characterkit.objectstart+1;
			SetCharacterCreatorTones (  t.tobj,3,t.tnewredshoes_f,t.tnewgreenshoes_f,t.tnewblueshoes_f,0.5 );
		}
		if (  t.tuseOtherMask  ==  5 ) 
		{
			t.tobj=t.characterkit.objectstart+4;
			SetCharacterCreatorTones (  t.tobj,0,t.tnewredhat_f,t.tnewgreenhat_f,t.tnewbluehat_f,0.5 );
			SetCharacterCreatorTones (  t.tobj,1,-1,0,0,0.0 );
			SetCharacterCreatorTones (  t.tobj,2,-1,0,0,0.0 );
			SetCharacterCreatorTones (  t.tobj,3,-1,0,0,0.0 );
		}

		return;
	}
	t.tpixel = GetPoint(t.inputsys.xmouse,t.inputsys.ymouse);
	if (  t.tuseOtherMask  ==  0 ) 
	{
		t.tnewred_f = RgbR(t.tpixel);
		t.tnewgreen_f = RgbG(t.tpixel);
		t.tnewblue_f = RgbB(t.tpixel);
		t.characterkit.haveSkinColor = 1;
	}
	if (  t.tuseOtherMask  ==  1 ) 
	{
		t.tnewredshirt_f = RgbR(t.tpixel);
		t.tnewgreenshirt_f = RgbG(t.tpixel);
		t.tnewblueshirt_f = RgbB(t.tpixel);
		t.characterkit.haveShirtColor = 1;
	}
	if (  t.tuseOtherMask  ==  2 ) 
	{
		t.tnewredbeard_f = RgbR(t.tpixel);
		t.tnewgreenbeard_f = RgbG(t.tpixel);
		t.tnewbluebreard_f = RgbB(t.tpixel);
		t.characterkit.haveBeardColor = 1;
	}
	if (  t.tuseOtherMask  ==  3 ) 
	{
		t.tnewredtrousers_f = RgbR(t.tpixel);
		t.tnewgreentrousers_f = RgbG(t.tpixel);
		t.tnewbluetrousers_f = RgbB(t.tpixel);
		t.characterkit.haveTrousersColor = 1;
	}
	if (  t.tuseOtherMask  ==  4 ) 
	{
		t.tnewredshoes_f = RgbR(t.tpixel);
		t.tnewgreenshoes_f = RgbG(t.tpixel);
		t.tnewblueshoes_f = RgbB(t.tpixel);
		t.characterkit.haveShoesColor = 1;
	}
	if (  t.tuseOtherMask  ==  5 ) 
	{
		t.tnewredhat_f = RgbR(t.tpixel);
		t.tnewgreenhat_f = RgbG(t.tpixel);
		t.tnewbluehat_f = RgbB(t.tpixel);
		t.characterkit.haveHatColor = 1;
	}
	if (  t.tuseOtherMask  ==  0 ) 
	{
		//  body and head skin
		t.tobj=t.characterkit.objectstart+1;
		SetCharacterCreatorTones (  t.tobj,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
		SetCharacterCreatorTones (  t.tobj+1,0,t.tnewred_f,t.tnewgreen_f,t.tnewblue_f,0.5 );
	}
	if (  t.tuseOtherMask  ==  1 ) 
	{
		t.tobj=t.characterkit.objectstart+1;
		SetCharacterCreatorTones (  t.tobj,1,t.tnewredshirt_f,t.tnewgreenshirt_f,t.tnewblueshirt_f,0.5 );
	}
	if (  t.tuseOtherMask  ==  2 ) 
	{
		t.tobj=t.characterkit.objectstart+3;
		SetCharacterCreatorTones (  t.tobj,0,t.tnewredbeard_f,t.tnewgreenbeard_f,t.tnewbluebreard_f,0.5 );
		SetCharacterCreatorTones (  t.tobj,1,-1,0,0,0.0 );
		SetCharacterCreatorTones (  t.tobj,2,-1,0,0,0.0 );
		SetCharacterCreatorTones (  t.tobj,3,-1,0,0,0.0 );
	}
	if (  t.tuseOtherMask  ==  3 ) 
	{
		t.tobj=t.characterkit.objectstart+1;
		SetCharacterCreatorTones (  t.tobj,2,t.tnewredtrousers_f,t.tnewgreentrousers_f,t.tnewbluetrousers_f,0.5 );
	}
	if (  t.tuseOtherMask  ==  4 ) 
	{
		t.tobj=t.characterkit.objectstart+1;
		SetCharacterCreatorTones (  t.tobj,3,t.tnewredshoes_f,t.tnewgreenshoes_f,t.tnewblueshoes_f,0.5 );
	}
	if (  t.tuseOtherMask  ==  5 ) 
	{
		t.tobj=t.characterkit.objectstart+4;
		SetCharacterCreatorTones (  t.tobj,0,t.tnewredhat_f,t.tnewgreenhat_f,t.tnewbluehat_f,0.5 );
		SetCharacterCreatorTones (  t.tobj,1,-1,0,0,0.0 );
		SetCharacterCreatorTones (  t.tobj,2,-1,0,0,0.0 );
		SetCharacterCreatorTones (  t.tobj,3,-1,0,0,0.0 );
	}

	if (  t.inputsys.mclick  ==  1 && t.characterkit.oldMouseClick  ==  0 ) 
	{
		t.characterkit.skinPickOn = 0;
	}
return;

//  needs tccentitytocheck = e
}

void characterkit_getObjectforEntity ( void )
{
	t.tccobj = 0;
	t.ttttentid = t.entityelement[t.tccentitytocheck].bankindex;
	if (  t.ttttentid > 0 ) 
	{
		//  character creator object
		if (  t.entityprofile[t.ttttentid].ischaractercreator  ==  1 ) 
		{
			t.tccobj = g.charactercreatorrmodelsoffset+((t.tccentitytocheck*3)-t.characterkitcontrol.offset);
		}
	}
return;

//  Search through entities in the scene, discover any character creator ones and update them
}

void characterkit_updateAllCharacterCreatorEntitiesInMap ( void )
{
//  `if inputsys.mclick  ==  1 then ttimeToUpdateAllCharacterCreatorEntitiesInMap  ==  0

	//  if we have come out of test game, we need to recolor all chars
	if (  t.trecolorAfterGame  ==  1 ) 
	{
		t.trecolorAfterGame = 0;
		characterkit_updateCharacters ( );
	}
	//  don't need to constantly update, so just when a mouse button has been clicked we do it for the next second
//  `if Timer() - ttimeToUpdateAllCharacterCreatorEntitiesInMap > 200 or MouseClick() <> 0

//   `ttimeToUpdateAllCharacterCreatorEntitiesInMap = Timer()

		for ( t.tcce = 1 ; t.tcce<=  g.entityelementlist; t.tcce++ )
		{
			t.tccentid=t.entityelement[t.tcce].bankindex;
			if (  t.tccentid>0 ) 
			{
				if (  t.entityprofile[t.tccentid].ischaractercreator  ==  1 ) 
				{
					t.tccsourceobj = t.entityelement[t.tcce].obj;
					if (  t.tccsourceobj > 0 ) 
					{
					if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
					{
						//  glue head to body
							t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
						if (  ObjectExist(t.tccobj)  ==  1 ) 
						{

								//  gluing doesnt work, probably down to things being instanced or something
//          `glue object to limb tccobj,tccsourceobj,tSourcebip01_head,2

									UnGlueObject (  t.tccobj );
									PositionObject (  t.tccobj, ObjectPositionX(t.tccsourceobj),ObjectPositionY(t.tccsourceobj),ObjectPositionZ(t.tccsourceobj) );
									YRotateObject (  t.tccobj, ObjectAngleY(t.tccsourceobj)+180 );
									t.tprofileobj=g.entitybankoffset+t.tccentid;
									if (  t.tprofileobj > 0 ) 
									{
										if (  ObjectExist(t.tprofileobj)  ==  1 ) 
										{
											t.tSourcebip01_head=getlimbbyname(t.tprofileobj, "Bip01_Head");
											if (  t.tSourcebip01_head > 0 ) 
											{
												t.tamountup_f = LimbPositionY(t.tprofileobj,t.tSourcebip01_head) - ObjectPositionY(t.tprofileobj);
												t.tamountforward_f = ObjectPositionZ(t.tprofileobj) - LimbPositionZ(t.tprofileobj,t.tSourcebip01_head);
												MoveObjectUp (  t.tccobj,t.tamountup_f );
												MoveObject (  t.tccobj,t.tamountforward_f );
												StopObject (  t.tprofileobj );
											}
										}
									}
//         `endif

							}
						}
					}
				}
			}
		}
//  `endif

return;

//  Search through entities in the scene, discover any character creator ones and update them
//  this is the in game version which is cut down for speed
}

void characterkit_updateAllCharacterCreatorEntitiesInGame ( void )
{
	for ( t.tcce = t.characterkitcontrol.minEntity ; t.tcce<=  t.characterkitcontrol.maxEntity; t.tcce++ )
	{
		if (  t.tcce  !=  t.steamworks_playerEntityID[g.steamworks.me] ) 
		{

			t.tccentid=t.entityelement[t.tcce].bankindex;
			if (  t.tccentid>0 ) 
			{
				if (  t.entityprofile[t.tccentid].ischaractercreator  ==  1 ) 
				{
					t.tccsourceobj = t.entityelement[t.tcce].obj;
					if (  t.tccsourceobj > 0 ) 
					{
					if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
					{
						//  place head to body
							t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
						if (  ObjectExist(t.tccobj)  ==  1 ) 
						{

								//  gluing doesnt work, probably down to things being instanced or something
//          `glue object to limb tccobj,tccsourceobj,tSourcebip01_head,2

									UnGlueObject (  t.tccobj );
									PositionObject (  t.tccobj, ObjectPositionX(t.tccsourceobj),ObjectPositionY(t.tccsourceobj),ObjectPositionZ(t.tccsourceobj) );
									YRotateObject (  t.tccobj, ObjectAngleY(t.tccsourceobj)+180 );
									t.tprofileobj=g.entitybankoffset+t.tccentid;
									if (  t.tprofileobj > 0 ) 
									{
										if (  ObjectExist(t.tprofileobj)  ==  1 ) 
										{
											t.tSourcebip01_head=getlimbbyname(t.tprofileobj, "Bip01_Head");
											if (  t.tSourcebip01_head > 0 ) 
											{
												t.tamountup_f = LimbPositionY(t.tprofileobj,t.tSourcebip01_head) - ObjectPositionY(t.tprofileobj);
												t.tamountforward_f = ObjectPositionZ(t.tprofileobj) - LimbPositionZ(t.tprofileobj,t.tSourcebip01_head);
												MoveObjectUp (  t.tccobj,t.tamountup_f );
												MoveObject (  t.tccobj,t.tamountforward_f );
												StopObject (  t.tprofileobj );
											}
										}
									}
//         `endif

							}
						}
					}
				}
			}

		}
	}
return;

//  Search through entities in the scene, discover any character creator ones and update them
//  this is the in game version which is cut down for speed
}

void characterkit_updateCharacterCreatorEntityInGame ( void )
{
	t.tcce=t.tte;
	if (  t.tcce  !=  t.steamworks_playerEntityID[g.steamworks.me] ) 
	{
		t.tccentid=t.entityelement[t.tcce].bankindex;
		if (  t.tccentid>0 ) 
		{
			if (  t.entityprofile[t.tccentid].ischaractercreator  ==  1 ) 
			{
				t.tccsourceobj = t.entityelement[t.tcce].obj;
				if (  t.tccsourceobj > 0 ) 
				{
					if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
					{
						//  place head to body
						t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
						if (  ObjectExist(t.tccobj)  ==  1 ) 
						{

							//  gluing works with instances no problem
							t.tprofileobj=g.entitybankoffset+t.tccentid;
							if (  t.tprofileobj > 0 ) 
							{
								if (  ObjectExist(t.tprofileobj)  ==  1 ) 
								{
									t.tSourcebip01_head=getlimbbyname(t.tprofileobj, "Bip01_Head");
									if (  t.tSourcebip01_head > 0 ) 
									{
										GlueObjectToLimbEx (  t.tccobj,t.tccsourceobj,t.tSourcebip01_head,2 );
									}
								}
							}

//        `rem gluing doesnt work, probably down to things being instanced or something

							//UnGlueObject (  tccobj )
							//PositionObject (  tccobj, ObjectPositionX(tccsourceobj),ObjectPositionY(tccsourceobj),ObjectPositionZ(tccsourceobj) )
							//YRotateObject (  tccobj, ObjectAngleY(tccsourceobj)+180 )
							//tprofileobj=entitybankoffset+tccentid
							//if tprofileobj > 0
							// if ObjectExist(tprofileobj) = 1
							//  tSourcebip01_head=getlimbbyname(tprofileobj, "Bip01_Head")
							//  if tSourcebip01_head > 0
							//   tamountup# = LimbPositionY(tprofileobj,tSourcebip01_head) - ObjectPositionY(tprofileobj)
							//   tamountforward# = ObjectPositionZ(tprofileobj) - LimbPositionZ(tprofileobj,tSourcebip01_head)
							//   MoveObjectUp (  tccobj,tamountup# )
							//   MoveObject (  tccobj,tamountforward# )
							//   StopObject (  tprofileobj )
							//  endif
							// endif
							//endif

						}
					}
				}
			}
		}
	}
return;

//  for initial spawn in in mp
}

void characterkit_updateAllCharacterCreatorEntitiesInMapFirstSpawn ( void )
{
	characterkit_updateCharacters ( );

	for ( t.tcce = 1 ; t.tcce<=  g.entityelementlist; t.tcce++ )
	{
		t.tccentid=t.entityelement[t.tcce].bankindex;
		if (  t.tccentid>0 ) 
		{
			if (  t.entityprofile[t.tccentid].ischaractercreator  ==  1 ) 
			{
				t.tccsourceobj = t.entityelement[t.tcce].obj;
				if (  t.tccsourceobj > 0 ) 
				{
				if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
				{
					//  glue head to body
						t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
					if (  ObjectExist(t.tccobj)  ==  1 ) 
					{

							PositionObject (  t.tccobj, ObjectPositionX(t.tccsourceobj),ObjectPositionY(t.tccsourceobj),ObjectPositionZ(t.tccsourceobj) );
							if (  t.tccobj  !=  t.entityelement[t.steamworks_playerEntityID[g.steamworks.me]].obj ) 
							{
								YRotateObject (  t.tccobj, ObjectAngleY(t.tccsourceobj)+180 );
							}
							else
							{
								UnGlueObject (  t.tccobj );
								YRotateObject (  t.tccobj, ObjectAngleY(t.tccsourceobj) );
							}
							t.tprofileobj=g.entitybankoffset+t.tccentid;
							if (  t.tprofileobj > 0 ) 
							{
								if (  ObjectExist(t.tprofileobj)  ==  1 ) 
								{
									t.tSourcebip01_head=getlimbbyname(t.tprofileobj, "Bip01_Head");
									if (  t.tSourcebip01_head > 0 ) 
									{
										t.tamountup_f = LimbPositionY(t.tprofileobj,t.tSourcebip01_head) - ObjectPositionY(t.tprofileobj);
										t.tamountforward_f = ObjectPositionZ(t.tprofileobj) - LimbPositionZ(t.tprofileobj,t.tSourcebip01_head);
										MoveObjectUp (  t.tccobj,t.tamountup_f );
										MoveObject (  t.tccobj,t.tamountforward_f );
										StopObject (  t.tprofileobj );
									}
								}
							}
						}
					}
				}
			}
		}
	}
return;

//  needs entid
}

void characterkit_loadEntityProfileObjects ( void )
{
	//  Add Character Creator bits on
	if (  t.characterkitcontrol.count  ==  0  )  t.characterkitcontrol.bankOffset  =  t.entid*3;
	++t.characterkitcontrol.count;

	characterkit_convertString ( );

	//  read in cci
	t.tcciloadname_s = g.fpscrootdir_s+"\\Files\\characterkit\\bodyandhead\\" + t.tbody_s + ".cci";
	t.tpath_s = "characterkit\\bodyandhead\\";
	t.tccquick = 1;
	characterkit_loadCCI ( );

	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset)+6;
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );
	if (  FileExist(t.tccinormal_s.Get())  ==  0  )  ExitPrompt ( cstr(cstr("Can't find ")+t.tccinormal_s).Get(), "Character Creator Error" );
	LoadImage (  t.tccinormal_s.Get(),t.tImgID+2 );
	TextureObject (  t.entobj,2,t.tImgID+2 );
	if (  FileExist(t.tccimask_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccinormal_s).Get(), "Character Creator Error" );
	LoadImage (  t.tccimask_s.Get(),t.tImgID+3 );
	TextureObject (  t.entobj,11,t.tImgID+3 );
	t.tfx=loadinternaleffectunique("effectbank\\reloaded\\character_basic.fx",0);
	SetObjectEffect (  t.entobj,t.tfx );

	SetCharacterCreatorTones (  t.entobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
	SetCharacterCreatorTones (  t.entobj,1,t.topRed_f,t.topGreen_f,t.topBlue_f,0.5 );
	SetCharacterCreatorTones (  t.entobj,2,t.bottomRed_f,t.bottomGreen_f,t.bottomBlue_f,0.5 );
	SetCharacterCreatorTones (  t.entobj,3,t.shoesRed_f,t.shoesGreen_f,t.shoesBlue_f,0.5 );

	//  read in cci head
	t.tcciloadname_s = g.fpscrootdir_s+"\\Files\\characterkit\\bodyandhead\\" + t.thead_s + "_head.cci";
	t.tpath_s = "characterkit\\bodyandhead\\";
	//  just grab basic info
	t.tccquick = 1;
	characterkit_loadCCI ( );
	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset);
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	LoadObject (  t.tccimesh_s.Get(),t.tccobj );
	SetObjectMask (  t.tccobj,1+(1<<31) );
	TextureObject (  t.tccobj,5,g.postprocessimageoffset+5 );
	t.tfx=loadinternaleffectunique("effectbank\\reloaded\\character_basic.fx",0);
	SetObjectLOD ( t.tccobj, 1, 6000 );
	SetObjectLOD ( t.tccobj, 2, 12000 );
	//  *6 because there are 3 objects with 3 textures each, so 9 slots needed
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset);
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );

	if (  t.tHasProjectionHead  ==  1 ) 
	{
		t.tccidiffuse_s = g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\" + t.entityprofileheader[t.entid].desc_s + "_cc.dds";
	}

	if (  FileExist(t.tccidiffuse_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccidiffuse_s).Get(), "Character Creator Error" );
	LoadImage (  t.tccidiffuse_s.Get(),t.tImgID );
	TextureObject (  t.tccobj,0,t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	if (  FileExist(t.tccinormal_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccinormal_s).Get(), "Character Creator Error" );
	LoadImage (  t.tccinormal_s.Get(),t.tImgID+1 );
	TextureObject (  t.tccobj,2,t.tImgID+1 );
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );
	LoadImage (  t.tccimask_s.Get(),t.tImgID+2 );
	TextureObject (  t.tccobj,11,t.tImgID+2 );
	SetObjectEffect (  t.tccobj,t.tfx );
	PositionObject (  t.tccobj,100000,100000,100000 );

	if (  t.tHasProjectionHead  ==  0 ) 
	{
		SetCharacterCreatorTones (  t.tccobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
	}
	else
	{
		SetCharacterCreatorTones (  t.tccobj,0,-1,-1,-1,0 );
	}
	SetCharacterCreatorTones (  t.tccobj,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,3,-1,0,0,0.5 );

	//  read in cci beard
	t.tcciloadname_s = g.fpscrootdir_s+"\\Files\\characterkit\\facialhair\\" + t.tbeard_s + ".cci";
	t.tpath_s = "characterkit\\facialhair\\";
	//  just grab basic info
	t.tccquick = 1;
	characterkit_loadCCI ( );
	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+1;
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	LoadObject (  t.tccimesh_s.Get(),t.tccobj );
	//t.tfx=loadinternaleffectunique("effectbank\\reloaded\\entity_basic.fx",0);
	t.tfx=loadinternaleffectunique("effectbank\\reloaded\\character_static.fx",0);
	//  *6 because there are 3 objects with 3 textures each, so 9 slots needed
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset)+3;
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );
	LoadImage (  t.tccidiffuse_s.Get(),t.tImgID );
	TextureObject (  t.tccobj,0,t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	LoadImage (  t.tccinormal_s.Get(),t.tImgID+1 );
	TextureObject (  t.tccobj,2,t.tImgID+1 );
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );
	if (  ImageExist(t.characterkit.imagestart+1) == 0  )  LoadImage (  "effectbank\\reloaded\\media\\blank_O.DDS",t.characterkit.imagestart+1 );
	TextureObject (  t.tccobj,11,t.characterkit.imagestart+1 );
	SetObjectEffect (  t.tccobj,t.tfx );
	PositionObject (  t.tccobj,100000,100000,100000 );

	SetCharacterCreatorTones (  t.tccobj,0,t.beardRed_f,t.beardGreen_f,t.beardBlue_f,0.5 );
	SetCharacterCreatorTones (  t.tccobj,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,3,-1,0,0,0.5 );

	//  Set radius of zero allows parent to animate even if outside of frustrum view
	if (  GetNumberOfFrames(t.tccobj)>0 ) 
	{
		//  but ONLY for animating objects, do not need to run parent objects if still
		SetSphereRadius (  t.tccobj,0 );
	}

	//  read in cci hat
	t.tcciloadname_s = g.fpscrootdir_s+"\\Files\\characterkit\\Headattachments\\" + t.that_s + ".cci";
	t.tpath_s = "characterkit\\Headattachments\\";
	//  just grab basic info
	t.tccquick = 1;
	characterkit_loadCCI ( );
	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+2;
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	LoadObject (  t.tccimesh_s.Get(),t.tccobj );
	//t.tfx=loadinternaleffectunique("effectbank\\reloaded\\entity_basic.fx",0);
	t.tfx=loadinternaleffectunique("effectbank\\reloaded\\character_static.fx",0);
	//  *6 because there are 3 objects with 2 textures each, so 6 slots needed
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset)+6;
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );
	LoadImage (  t.tccidiffuse_s.Get(),t.tImgID );
	TextureObject (  t.tccobj,0,t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	LoadImage (  t.tccinormal_s.Get(),t.tImgID+1 );
	TextureObject (  t.tccobj,2,t.tImgID+1 );
	if (  ImageExist(t.characterkit.imagestart+1) == 0  )  LoadImage (  "effectbank\\reloaded\\media\\blank_O.DDS",t.characterkit.imagestart+1 );
	TextureObject (  t.tccobj,11,t.characterkit.imagestart+1 );
	SetObjectEffect (  t.tccobj,t.tfx );
	PositionObject (  t.tccobj,100000,100000,100000 );

	SetCharacterCreatorTones (  t.tccobj,0,t.hatRed_f,t.hatGreen_f,t.hatBlue_f,0.5 );
	SetCharacterCreatorTones (  t.tccobj,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,3,-1,0,0,0.5 );

	//  Set radius of zero allows parent to animate even if outside of frustrum view
	if (  GetNumberOfFrames(t.tccobj)>0 ) 
	{
		//  but ONLY for animating objects, do not need to run parent objects if still
		SetSphereRadius (  t.tccobj,0 );
	}
}

void characterkit_convertString ( void )
{

	t.tHasProjectionHead = 0;
	if (  t.tccstringtoconvert_s == "" ) 
	{
		t.tstring_s = t.entityprofile[t.entid].charactercreator_s;
	}
	else
	{
		t.tstring_s = t.tccstringtoconvert_s;
		t.tccstringtoconvert_s = "";
	}
	//  "v"
	//t.tnothing_s = FirstToken(t.tstring_s.Get(),":");
	t.tnothing_s = strtok(t.tstring_s.Get(),":");
	//  "version number"
	t.tcciversion_s = strtok(NULL,":");
	//  body mesh
	t.tbody_s = strtok(NULL,":");
	//  head mesh
	t.thead_s = strtok(NULL,":");
	//  check for projection head
	t.ts_s = Lower(t.thead_s.Get());
	if (  cstr(Left(t.ts_s.Get(),10))  ==  "projection" || cstr(Left(t.ts_s.Get(),16))  ==  "fmale_projection" || cstr(Left(t.ts_s.Get(),12))  ==  "1_projection" || cstr(Left(t.ts_s.Get(),18))  ==  "1_fmale_projection" ) 
	{
		t.tHasProjectionHead = 1;
		//  to remain compatible with old version check here
		if (  t.tcciversion_s  ==  "1" ) 
		{
			t.tcheckfile_s = g.fpscrootdir_s+"\\Files\\characterkit\\bodyandhead\\"+t.thead_s+".cci";
			if (  FileExist(t.tcheckfile_s.Get())  ==  0 ) 
			{
				t.thead_s = cstr("1_")+t.thead_s;
			}
		}
	}
	//  beard mesh
	t.tbeard_s = strtok(NULL,":");
	//  hat mesh
	t.that_s = strtok(NULL,":");
	//  to remain compatible with old version check here
	if (  t.tcciversion_s  ==  "1" ) 
	{
		t.tcheckfile_s = g.fpscrootdir_s+"\\Files\\characterkit\\Headattachments\\"+t.that_s+".cci";
		if (  FileExist(t.tcheckfile_s.Get())  ==  0 ) 
		{
			t.tcheckfile_s = g.fpscrootdir_s+"\\Files\\characterkit\\Headattachments\\1_"+t.that_s+".cci";
			if (  FileExist(t.tcheckfile_s.Get())  ==  0 ) 
			{
				t.tcheckfile_s = g.fpscrootdir_s+"\\Files\\characterkit\\Headattachments\\2_"+t.that_s+".cci";
				if (  FileExist(t.tcheckfile_s.Get())  ==  0 ) 
				{
					t.that_s = "none";
				}
				else
				{
					t.that_s = cstr("2_")+t.that_s;
				}
			}
			else
			{
				t.that_s = cstr("1_")+t.that_s;
			}
		}
	}
	//  skin
	t.skinRed_f = ValF(strtok(NULL,":"));
	t.skinGreen_f =ValF(strtok(NULL,":"));
	t.skinBlue_f = ValF(strtok(NULL,":"));
	//  top
	t.topRed_f = ValF(strtok(NULL,":"));
	t.topGreen_f = ValF(strtok(NULL,":"));
	t.topBlue_f = ValF(strtok(NULL,":"));
	//  beard
	t.beardRed_f = ValF(strtok(NULL,":"));
	t.beardGreen_f = ValF(strtok(NULL,":"));
	t.beardBlue_f = ValF(strtok(NULL,":"));
	//  bottom
	t.bottomRed_f = ValF(strtok(NULL,":"));
	t.bottomGreen_f = ValF(strtok(NULL,":"));
	t.bottomBlue_f = ValF(strtok(NULL,":"));
	//  shoes
	t.shoesRed_f = ValF(strtok(NULL,":"));
	t.shoesGreen_f = ValF(strtok(NULL,":"));
	t.shoesBlue_f = ValF(strtok(NULL,":"));
	if (  ValF(t.tcciversion_s.Get()) > 1 ) 
	{
		//  hat
		t.hatRed_f = ValF(strtok(NULL,":"));
		t.hatGreen_f = ValF(strtok(NULL,":"));
		t.hatBlue_f = ValF(strtok(NULL,":"));
	}
	else
	{
		t.hatRed_f = -1;
		t.hatGreen_f = -1;
		t.hatBlue_f = -1;
	}

return;

//  checks if we have any character creator characters in the game
}

void characterkit_checkForCharacters ( void )
{

	//  unglue any heads from profile objects
	for ( t.entid = 0 ; t.entid<=  100; t.entid++ )
	{
		//  Character creator head, unglue profile object
		t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset);
		if (  ObjectExist(t.tccobj) == 1 ) 
		{
			UnGlueObject (  t.tccobj );
			PositionObject (  t.tccobj,100000,100000,100000 );
		}
		if (  ObjectExist(t.tccobj+1) == 1 ) 
		{
			UnGlueObject (  t.tccobj+1 );
			PositionObject (  t.tccobj+1,100000,100000,100000 );
		}
		if (  ObjectExist(t.tccobj+2) == 1 ) 
		{
			UnGlueObject (  t.tccobj+2 );
			PositionObject (  t.tccobj+2,100000,100000,100000 );
		}
	}

	t.characterkitcontrol.gameHasCharacterCreatorIn = 0;
	t.characterkitcontrol.minEntity = 1;
	t.characterkitcontrol.maxEntity = g.entityelementlist;
	for ( t.tcce = 1 ; t.tcce<=  g.entityelementlist; t.tcce++ )
	{
		t.entid=t.entityelement[t.tcce].bankindex;
		if (  t.entid>0 ) 
		{
			if (  t.entityprofile[t.entid].ischaractercreator  ==  1 ) 
			{
				t.characterkitcontrol.gameHasCharacterCreatorIn = 1;
				t.characterkitcontrol.maxEntity = t.tcce;
				t.trecolorAfterGame = 1;
			}
			else
			{
				if (  t.characterkitcontrol.gameHasCharacterCreatorIn  ==  0  )  t.characterkitcontrol.minEntity  =  t.tcce+1;
			}
		}
	}
}

void characterkit_updateCharacters ( void )
{
		//  check the entities we are checking are within bounds, if not exit out
		if (  t.characterkitcontrol.maxEntity > g.entityelementlist || t.characterkitcontrol.minEntity > g.entityelementlist  )  return;

		for ( t.tcce = t.characterkitcontrol.minEntity ; t.tcce<=  t.characterkitcontrol.maxEntity; t.tcce++ )
		{
			t.entid=t.entityelement[t.tcce].bankindex;
			if (  t.entid>0 ) 
			{
				if (  t.entityprofile[t.entid].ischaractercreator  ==  1 ) 
				{
					t.tccsourceobj = t.entityelement[t.tcce].obj;
					characterkit_convertString ( );
//      `set cursor 0,50

//      `print GetFrame(entityelement(tcce).obj)

					//  body
					if (  t.tccsourceobj > 0 ) 
					{
						if (  ObjectExist(t.tccsourceobj)  ==  1 ) 
						{
							SetCharacterCreatorTones (  t.tccsourceobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
							SetCharacterCreatorTones (  t.tccsourceobj,1,t.topRed_f,t.topGreen_f,t.topBlue_f,0.5 );
							SetCharacterCreatorTones (  t.tccsourceobj,2,t.bottomRed_f,t.bottomGreen_f,t.bottomBlue_f,0.5 );
							SetCharacterCreatorTones (  t.tccsourceobj,3,t.shoesRed_f,t.shoesGreen_f,t.shoesBlue_f,0.5 );
						}
					}
					//  head
					t.tccobj = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset);
					if (  ObjectExist(t.tccobj)  ==  1 ) 
					{
						if (  t.tHasProjectionHead  ==  0 ) 
						{
							SetCharacterCreatorTones (  t.tccobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
						}
						else
						{
							SetCharacterCreatorTones (  t.tccobj,0,-1,-1,-1,0 );
						}
						SetCharacterCreatorTones (  t.tccobj,1,-1,0,0,0.1f );
						SetCharacterCreatorTones (  t.tccobj,2,-1,0,0,0.1f );
						SetCharacterCreatorTones (  t.tccobj,3,-1,0,0,0.1f );
					}
					//  beard
					t.tccobjbeard = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset)+1;
					if (  ObjectExist(t.tccobjbeard)  ==  1 ) 
					{
						SetCharacterCreatorTones (  t.tccobjbeard,0,t.beardRed_f,t.beardGreen_f,t.beardBlue_f,0.5 );
						SetCharacterCreatorTones (  t.tccobjbeard,1,-1,0,0,0.5 );
						SetCharacterCreatorTones (  t.tccobjbeard,2,-1,0,0,0.5 );
						SetCharacterCreatorTones (  t.tccobjbeard,3,-1,0,0,0.5 );
					}
					//  hat
					t.tccobjhat = g.charactercreatorrmodelsoffset+((t.tcce*3)-t.characterkitcontrol.offset)+2;
					if (  ObjectExist(t.tccobjhat)  ==  1 ) 
					{
						SetCharacterCreatorTones (  t.tccobjhat,0,t.hatRed_f,t.hatGreen_f,t.hatBlue_f,0.5 );
						SetCharacterCreatorTones (  t.tccobjhat,1,-1,0,0,0.5 );
						SetCharacterCreatorTones (  t.tccobjhat,2,-1,0,0,0.5 );
						SetCharacterCreatorTones (  t.tccobjhat,3,-1,0,0,0.5 );
					}
				}
			}
		}
return;

//  needs e and entid
}

void characterkit_addEntityToMap ( void )
{
	if (  t.tccSetTentid  ==  1 ) 
	{
		t.toldEntid = t.entid;
		t.entid = t.tentid;
		t.tolde = t.e;
		t.e = t.tte;
		t.tccsourceobj = t.obj;
	}
	else
	{
		t.tccsourceobj = t.entityelement[t.e].obj;
	}

	//  Character creator head, unglue profile object
	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset);
	if (  ObjectExist(t.tccobj) == 1 ) 
	{
		UnGlueObject (  t.tccobj );
		PositionObject (  t.tccobj,100000,100000,100000 );
	}

	if (  t.characterkitcontrol.offset  ==  0  )  t.characterkitcontrol.offset  =  -500;
	characterkit_convertString ( );
	t.tmainobj = t.obj;
	if (  t.tmainobj > 0 ) 
	{
		if (  ObjectExist(t.tmainobj)  ==  1 ) 
		{
			SetCharacterCreatorTones (  t.tmainobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
			SetCharacterCreatorTones (  t.tmainobj,1,t.topRed_f,t.topGreen_f,t.topBlue_f,0.5 );
			SetCharacterCreatorTones (  t.tmainobj,2,t.bottomRed_f,t.bottomGreen_f,t.bottomBlue_f,0.5 );
			SetCharacterCreatorTones (  t.tmainobj,3,t.shoesRed_f,t.shoesGreen_f,t.shoesBlue_f,0.5 );
		}
	}

	//  head
	t.tccobj = g.charactercreatorrmodelsoffset+((t.e*3)-t.characterkitcontrol.offset);
	if (  ObjectExist(g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset))  ==  0 ) 
	{
		t.entityprofile[t.entid].ischaractercreator = 0;
		t.entityprofile[t.entid].charactercreator_s="";
		return;
	}
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	CloneObject (  t.tccobj,g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset),1 );
	PositionObject (  t.tccobj,ObjectPositionX(t.tccsourceobj),ObjectPositionY(t.tccsourceobj) ,ObjectPositionZ(t.tccsourceobj) );
	RotateObject (  t.tccobj,ObjectAngleX(t.tccsourceobj),ObjectAngleY(t.tccsourceobj),ObjectAngleZ(t.tccsourceobj) );
	//Fixed LOD issue with cc head
	SetObjectLOD ( t.tccobj, 1, 6000 );
	SetObjectLOD ( t.tccobj, 2, 12000 );

	//  restore any radius settings the original object might have had
	SetSphereRadius (  t.tccobj,-1 );

	//  ensure new object ONLY interacts with main camera and shadow camera
	//  (until postprocess masks kick in)
	SetObjectMask (  t.tccobj,1+(1<<31) );
	TextureObject (  t.tccobj,5,g.postprocessimageoffset+5 );

	if (  t.tHasProjectionHead  ==  0 ) 
	{
		SetCharacterCreatorTones (  t.tccobj,0,t.skinRed_f,t.skinGreen_f,t.skinBlue_f,0.5 );
	}
	else
	{
		SetCharacterCreatorTones (  t.tccobj,0,-1,-1,-1,0 );
	}
	SetCharacterCreatorTones (  t.tccobj,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobj,3,-1,0,0,0.5 );

	//  beard
	t.tccobjbeard = g.charactercreatorrmodelsoffset+((t.e*3)-t.characterkitcontrol.offset)+1;
	if (  ObjectExist(t.tccobjbeard)  ==  1  )  DeleteObject (  t.tccobjbeard );
	CloneObject (  t.tccobjbeard,g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+1,1 );
	//  glue to head
	t.tBip01_FacialHair=getlimbbyname(t.tccobj, "Bip01_FacialHair");

	GlueObjectToLimbEx (  t.tccobjbeard,t.tccobj,t.tBip01_FacialHair,2 );

	//  restore any radius settings the original object might have had
	SetSphereRadius (  t.tccobjbeard,-1 );

	//  ensure new object ONLY interacts with main camera and shadow camera
	//  (until postprocess masks kick in)
	SetObjectMask (  t.tccobjbeard,1+(1<<31) );

	SetCharacterCreatorTones (  t.tccobjbeard,0,t.beardRed_f,t.beardGreen_f,t.beardBlue_f,0.5 );
	SetCharacterCreatorTones (  t.tccobjbeard,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobjbeard,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobjbeard,3,-1,0,0,0.5 );

	//  hat
	t.tccobjhat = g.charactercreatorrmodelsoffset+((t.e*3)-t.characterkitcontrol.offset)+2;
	if (  ObjectExist(t.tccobjhat)  ==  1  )  DeleteObject (  t.tccobjhat );
	CloneObject (  t.tccobjhat,g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+2,1 );
	//  glue to head
	t.Bip01_Headgear=getlimbbyname(t.tccobj, "Bip01_Headgear");

	GlueObjectToLimbEx (  t.tccobjhat,t.tccobj,t.Bip01_Headgear,2 );

	//  restore any radius settings the original object might have had
	SetSphereRadius (  t.tccobjhat,-1 );

	//  ensure new object ONLY interacts with main camera and shadow camera
	//  (until postprocess masks kick in)
	SetObjectMask (  t.tccobjhat,1+(1<<31) );

	SetCharacterCreatorTones (  t.tccobjhat,0,t.hatRed_f,t.hatGreen_f,t.hatBlue_f,0.5 );
	SetCharacterCreatorTones (  t.tccobjhat,1,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobjhat,2,-1,0,0,0.5 );
	SetCharacterCreatorTones (  t.tccobjhat,3,-1,0,0,0.5 );

	//  clue head to body
	t.tccobj = g.charactercreatorrmodelsoffset+((t.e*3)-t.characterkitcontrol.offset);
	t.tSourcebip01_head=getlimbbyname(t.tccsourceobj, "Bip01_Head");
	t.tDestbip01_head=getlimbbyname(t.tccobj, "Bip01_Head");
	if (  t.tSourcebip01_head > 0 && t.tDestbip01_head > 0 ) 
	{
		GlueObjectToLimbEx (  t.tccobj,t.tccsourceobj,t.tSourcebip01_head,2 );
	}

	if (  t.tccSetTentid  ==  1 ) 
	{
		t.tccSetTentid = 0;
		t.entid = t.toldEntid;
		t.e = t.tolde;
	}
}

void characterkit_deleteBankObject ( void )
{
	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset);
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset);
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );

	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+1;
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset)+3;
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );

	t.tccobj = g.charactercreatorrmodelsoffset+((t.entid*3)-t.characterkitcontrol.bankOffset)+2;
	if (  ObjectExist(t.tccobj)  ==  1  )  DeleteObject (  t.tccobj );
	t.tImgID = g.charactercreatorimageoffset+((t.entid*10)-t.characterkitcontrol.bankOffset)+6;
	if (  ImageExist(t.tImgID)  ==  1  )  DeleteImage (  t.tImgID );
	if (  ImageExist(t.tImgID+1)  ==  1  )  DeleteImage (  t.tImgID+1 );
	if (  ImageExist(t.tImgID+2)  ==  1  )  DeleteImage (  t.tImgID+2 );
	if (  ImageExist(t.tImgID+3)  ==  1  )  DeleteImage (  t.tImgID+3 );

	t.entityprofile[t.entid].ischaractercreator = 0;
	t.entityprofile[t.entid].charactercreator_s="";
}

void characterkit_deleteAllBankObjects ( void )
{
	t.toldEntid = t.entid;
	for ( t.entid = 0 ; t.entid<=  100; t.entid++ )
	{
		characterkit_deleteBankObject ( );
	}
	t.entid = t.toldEntid;
}

void characterkit_deleteEntity ( void )
{
	// 150216 - Ravey - bit of a mess, but 't.ccobjToDelete' actually means 'e'
	// and this used to DELETE character creator head parents as 't.characterkitcontrol.offset' was zero when this
	// was called, creating erasure of the NEW character creator head object parents that had been loaded!!
	// so we reinstate some hard coding (lord knows why -500) to recreate the correct offset for deleting..
	if ( t.characterkitcontrol.offset==0 ) t.characterkitcontrol.offset = -500;
	for ( t.tloop = 0; t.tloop<=2; t.tloop++ )
	{
		t.tccobj = g.charactercreatorrmodelsoffset+((t.ccobjToDelete*3)-t.characterkitcontrol.offset);
		if (  ObjectExist(t.tccobj+t.tloop) == 1  )  DeleteObject (  t.tccobj+t.tloop );
	}
}

void characterkit_makeF9Cursor ( void )
{

	//  if no character creator characters in the level, return
	if (  t.characterkitcontrol.gameHasCharacterCreatorIn  ==  0  )  return;
	
	if (  t.characterkitcontrol.oldF9CursorEntid > 0 ) 
	{
		t.tccobj = g.charactercreatorrmodelsoffset+((t.characterkitcontrol.oldF9CursorEntid*3)-t.characterkitcontrol.bankOffset);
//   `if ObjectExist(tccobj) then UnGlueObject (  tccobj )

		PositionObject (  t.tccobj,-10000,-10000,-10000 );
		t.characterkitcontrol.oldF9CursorEntid = 0;
	}
	//  Character creator head
	if (  t.conkit.edit.entityindex > 0 ) 
	{
		t.tobj = t.conkit.objectstartnumber+1;
		if (  t.tobj > 0 ) 
		{
			if (  ObjectExist(t.tobj) ) 
			{

				t.tentid = t.entityelement[t.conkit.edit.entityindex].bankindex;
				if (  t.entityprofile[t.tentid].ischaractercreator  ==  1 ) 
				{
//     `pausetext("hello")

					t.characterkitcontrol.oldF9CursorEntid = t.tentid;
//      `tSourcebip01_head=getlimbbyname(tobj, "Bip01_Head")

//      `pausetext("checking for head bone")

//      `if tSourcebip01_head > 0

//       `pausetext("found a head bone")

						t.tccobj = g.charactercreatorrmodelsoffset+((t.tentid*3)-t.characterkitcontrol.bankOffset);
						if (  ObjectExist(t.tccobj) == 1 ) 
						{
//        `pausetext("found head model")

							t.tBip01_FacialHair=getlimbbyname(t.tccobj, "Bip01_FacialHair");
							if (  t.tBip01_FacialHair > 0  )  GlueObjectToLimbEx (  g.charactercreatorrmodelsoffset+((t.tentid*3)-t.characterkitcontrol.bankOffset)+1,t.tccobj,t.tBip01_FacialHair,2 );
							t.Bip01_Headgear=getlimbbyname(t.tccobj, "Bip01_Headgear");
							if (  t.Bip01_Headgear > 0  )  GlueObjectToLimbEx (  g.charactercreatorrmodelsoffset+((t.tentid*3)-t.characterkitcontrol.bankOffset)+2,t.tccobj,t.Bip01_Headgear,2 );
//        `glue object to limb tccobj,tobj,0,2

							PositionObject (  t.tccobj,ObjectPositionX(t.tobj),ObjectPositionY(t.tobj),ObjectPositionZ(t.tobj) );
							MoveObjectUp (  t.tccobj,64 );
							MoveObject (  t.tccobj,-1 );
							YRotateObject (  t.tccobj,ObjectAngleY(t.tobj)+180 );
						//pausetext("glued")
						}
//      `else

//       `pausetext("no head bone")

//      `endif

				}

			}
		}

	// pausetext("bye")
	}
return;

//  just for debugging purposes
}

void pausetext ( char* t_s )
{
	int tdelaytime = 0;
	tdelaytime = Timer();
	while (  Timer() - tdelaytime < 2000 ) 
	{
		Cls (  );
		SetCursor (  0,0 );
		Print (  t_s );
		Sync (  );
	}
}

/*      
void characterkit_changeSkinTone ( void )
{
	t.assetsequence = 1;
	if (  t.assetsequence == 1  )  t.tassetimg = 11;
	if (  t.assetsequence == 2  )  t.tassetimg = 14;
	t.timg = t.characterkit.imagestart + t.tassetimg;
	pastebitmapfontcenter("Generating Text ( ure",GetDisplayWidth()/2,(GetDisplayHeight()/2)-50,1,255)  ); Sync (  );
	load bitmap t.characterkit_diffuse_s[t.assetsequence],1;
	SetCurrentBitmap (  1 );
	LockPixels (  );
		for ( t.y = 0 ; t.y<=  2047; t.y++ )
		{
			for ( t.x = 0 ; t.x<=  2047; t.x++ )
			{
				t.tpixel = GetPoint(t.x,t.y);
				tamountfromMask_f = ( characterkit_skinmask(t.assetsequence,t.x,t.y) / 255.0 ) / 2.0;
				tamountfromMaskShirt_f = ( characterkit_shirtmask(t.assetsequence,t.x,t.y) / 255.0 ) / 2.0;
				tamountfromPixel_f = 1.0 - tamountfromMask_f;
				tamountfromPixelShirt_f = 1.0 - tamountfromMaskShirt_f;
				if (  t.characterkit.haveSkinColor  ==  1 ) 
				{
					if (  tamountfromMask_f > 0.0 ) 
					{
						tred  =  (RgbR(t.tpixel) * tamountfromPixel_f) + (tnewred * tamountfromMask_f) ; if (  tred > 255 then tred  ==  255  ) ; if tred < 0 then tred  =  0;
						tgreen  =  (RgbG(t.tpixel) * tamountfromPixel_f) + (tnewgreen * tamountfromMask_f) ; if (  tgreen > 255 then tgreen  ==  255  ) ; if tgreen < 0 then tgreen  =  0;
						tblue  =  (RgbB(t.tpixel) * tamountfromPixel_f) + (tnewblue * tamountfromMask_f) ; if (  tblue > 255 then tblue  ==  255  ) ; if tblue < 0 then tblue  =  0;
						Dot (  t.x,t.y,Rgb(tred,tgreen,tblue) );
					}
				}
				if (  t.characterkit.haveShirtColor  ==  1 ) 
				{
					if (  tamountfromMaskShirt_f > 0.0 ) 
					{
						tred  =  (RgbR(t.tpixel) * tamountfromPixelShirt_f) + (tnewredshirt * tamountfromMaskShirt_f) ; if (  tred > 255 then tred  ==  255  ) ; if tred < 0 then tred  =  0;
						tgreen  =  (RgbG(t.tpixel) * tamountfromPixelShirt_f) + (tnewgreenshirt * tamountfromMaskShirt_f) ; if (  tgreen > 255 then tgreen  ==  255  ) ; if tgreen < 0 then tgreen  =  0;
						tblue  =  (RgbB(t.tpixel) * tamountfromPixelShirt_f) + (tnewblueshirt * tamountfromMaskShirt_f) ; if (  tblue > 255 then tblue  ==  255  ) ; if tblue < 0 then tblue  =  0;
						Dot (  t.x,t.y,Rgb(tred,tgreen,tblue) );
					}
				}
			}
		}
	UnlockPixels (  );
	if (  ImageExist(t.timg)  ==  1  )  DeleteImage (  t.timg );
	GrabImage (  t.timg,0,0,2048,2048 );
	SaveImage (  "tempcci.png",t.timg );
	DeleteImage (  t.timg );
	LoadImage (  "tempcci.png",t.timg );
	DeleteAFile (  "tempcci.png" );
	SetCurrentBitmap (  0 );

	t.tobj=t.characterkit.objectstart+t.assetsequence;
	TextureObject (  t.tobj,0,t.timg );
	DeleteBitmapEx (  1 );
return;

//  takes assetsequence
}

void characterkit_makeMask ( void )
{
	if (  t.assetsequence  ==  1 ) 
	{
		Dim (  characterkit_skinmask(2,2048,2048) );
		Dim (  characterkit_shirtmask(2,2048,2048) );
	}
	load bitmap t.tccimask_s,1;
	SetCurrentBitmap (  1 );
	LockPixels (  );
		for ( t.y = 0 ; t.y<=  2047; t.y++ )
		{
			for ( t.x = 0 ; t.x<=  2047; t.x++ )
			{
				t.tpixel = GetPoint(t.x,t.y);
				characterkit_skinmask(t.assetsequence,t.x,t.y)= RgbR(t.tpixel);
				characterkit_shirtmask(t.assetsequence,t.x,t.y)= RgbG(t.tpixel);
			}
		}
	UnlockPixels (  );
	SetCurrentBitmap (  0 );
	DeleteBitmapEx (  1 );
return;
*/    


//Functions

//}

void characterkit_updateFromWidget ( void )
{
	if (  t.widget.activeObject > 0 ) 
	{
		t.tccentid = t.entityelement[t.widget.pickedEntityIndex].bankindex;
		if (  t.tccentid > 0 ) 
		{
			//  check if this is a character creator entity
//    ``if entityprofile(t.widget.pickedEntityIndex).ischaractercreator  ==  1 `` Naughty!! use CORRECT subscripts ;)

			if (  t.entityprofile[t.tccentid].ischaractercreator == 1 ) 
			{
				t.tccobj = g.charactercreatorrmodelsoffset+((t.widget.pickedEntityIndex*3)-t.characterkitcontrol.offset);
//     `if ObjectExist(tccobj) = 1

				if (  ObjectExist(t.tccobj) == 1 ) 
				{
					PositionObject (  t.tccobj,ObjectPositionX(t.widget.activeObject),ObjectPositionY(t.widget.activeObject),ObjectPositionZ(t.widget.activeObject) );
				}
			}
		}
	}
return;

//  allow choosing of online avatar
}

void characterkit_chooseOnlineAvatar ( void )
{

	//  find spare image
	for ( t.c = 1 ; t.c<=  100000; t.c++ )
	{
		if (  ImageExist(t.c)  ==  0 ) 
		{
			t.tbdimage = t.c;
			break;
		}
	}

	GrabImage (  t.tbdimage,0,0,GetDisplayWidth(), GetDisplayHeight(),3 );

	t.tolddir_s = GetDir();
	t.characterkit.selected = 99;
	characterkit_init ( );
	SetDir (  t.tolddir_s.Get() );

	t.characterkit.thumbGadgetOn = 1;
	t.characterkit.picked = 0;
	t.characterkit.loadthumbs = 1;
	//  mode 99 is for avatar selection
	t.characterkit.selected = 99;
	t.importer.message = "";
	t.importer.viewMessage = "";

	t.tpickerOldMouseClick = MouseClick();

	t.characterkitavatarmax = 0;
	SetDir (  "entitybank\\user\\charactercreator" );
	ChecklistForFiles (  );
	for ( t.c = 1 ; t.c<=  ChecklistQuantity(); t.c++ )
	{
		t.tfile_s=Lower(ChecklistString(t.c));
		if (  t.tfile_s != "." && t.tfile_s != ".." ) 
		{
			if (  cstr(Right(t.tfile_s.Get(),4)) == ".fpe" ) 
			{
				++t.characterkitavatarmax;
				Dim (  t.characterkitavatarbank_s,t.characterkitavatarmax  );
				t.characterkitavatarbank_s[t.characterkitavatarmax]=Left(t.tfile_s.Get(),Len(t.tfile_s.Get())-4);
			}
		}
	}
	SetDir (  t.tolddir_s.Get() );
	t.tpickedAvatar_s = "";

	do
	{

		//  handle input
		OpenFileMap (  1, "FPSEXCHANGE" );
		SetEventAndWait (  1 );
		t.inputsys.xmouse=GetFileMapDWORD( 1, 0 );
		t.inputsys.ymouse=GetFileMapDWORD( 1, 4 );
		//CloseFileMap (  1 );

		t.tadjustedtoccxbase_f=GetChildWindowWidth()/800.0;
		t.tadjustedtoccybase_f=GetChildWindowHeight()/600.0;

		t.tccoldmousex = t.inputsys.xmouse;
		t.tccoldmousey = t.inputsys.ymouse;
		t.inputsys.xmouse = t.inputsys.xmouse*t.tadjustedtoccxbase_f;
		t.inputsys.ymouse = t.inputsys.ymouse*t.tadjustedtoccybase_f;
		t.inputsys.mclick = MouseClick();

		PasteImage (  t.tbdimage,0,0,1 );
		characterkit_thumbgadget ( );
		t.characterkit.oldMouseClick = t.inputsys.mclick;
		Sync (  );
	} while ( !(  t.characterkit.thumbGadgetOn  ==  0 ) );

	//  clean up
	DeleteImage (  t.tbdimage );
	characterkit_free ( );
	UnDim (  t.characterkitavatarbank_s );
	t.characterkit.selected = 0;
	if ( t.tpickedAvatar_s != "" ) characterkit_saveAvatarInfo ( );
	while (  MouseClick()  !=  0 ) 
	{
		Sync (  );
	}
return;

}

void characterkit_saveAvatarInfo ( void )
{
	t.tavatarstring_s = "";
	if (  FileExist( cstr(cstr("entitybank\\user\\charactercreator\\")+t.tpickedAvatar_s+".fpe").Get() )  ==  1 ) 
	{
		if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
		//  grab the character creator string
		OpenToRead (  1, cstr(cstr("entitybank\\user\\charactercreator\\")+t.tpickedAvatar_s+".fpe").Get() );

		while (  FileEnd(1)  ==  0 ) 
		{
			t.tline_s = ReadString ( 1 );
			//  adjust offset for male/female
			t.tcciStat_s = Lower(FirstToken( t.tline_s.Get(), " "));
			if (  t.tcciStat_s  ==  "charactercreator" ) 
			{
				t.tcciValue_s = NextToken( " " );
				if (  t.tcciValue_s  ==  "="  )  t.tcciValue_s  =  NextToken( " " );
				t.tavatarstring_s = t.tcciValue_s;
				break;
			}
		}

		CloseFile (  1 );

		if (  FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() )  ==  1  )  DeleteAFile (  cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		OpenToWrite (  1, cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
		WriteString (  1,t.tavatarstring_s.Get() );
		WriteString (  1,t.tpickedAvatar_s.Get() );
		CloseFile (  1 );
		//  load in the avatar for multiplayer
		characterkit_loadMyAvatarInfo ( );

	}
return;

//  load our avatar info so we can use it and also send to others in multiplayer
}

void characterkit_checkAvatarExists ( void )
{
	if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
	if (  FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() )  ==  1 ) 
	{
		OpenToRead (  1, cstr (g.fpscrootdir_s + "\\multiplayeravatar.dat").Get()  );
		g.steamworks.myAvatar_s = ReadString ( 1 );
		g.steamworks.myAvatarHeadTexture_s = ReadString ( 1 );
		g.steamworks.myAvatarName_s = g.steamworks.myAvatarHeadTexture_s;
		//  store the name of the head texture
		if (  g.steamworks.myAvatarHeadTexture_s  !=  "" ) 
		{
			g.steamworks.myAvatarHeadTexture_s = g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\"+ g.steamworks.myAvatarHeadTexture_s + "_cc.dds";
			if (  FileExist(g.steamworks.myAvatarHeadTexture_s.Get())  ==  0  )
			{
				g.steamworks.myAvatarHeadTexture_s  =  "";
				CloseFile ( 1 );
				DeleteAFile (  cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() );
				return;
			}
		}
		CloseFile (  1 );
	}
}

void characterkit_loadMyAvatarInfo ( void )
{

	//  blank out the data first
	g.steamworks.myAvatar_s = "";
	g.steamworks.myAvatarName_s = "";
	g.steamworks.myAvatarHeadTexture_s = "";
	g.steamworks.haveSentMyAvatar = 0;

	for ( t.c = 0 ; t.c<=  STEAM_MAX_NUMBER_OF_PLAYERS-1; t.c++ )
	{
		t.steamworks_playerAvatars_s[t.c] = "";
		t.steamworks_playerAvatarOwners_s[t.c] = "";
	}

	if (  FileOpen(1)  ==  1  )  CloseFile (  1 );
	if (  FileExist( cstr(g.fpscrootdir_s + "\\multiplayeravatar.dat").Get() )  ==  1 ) 
	{
		OpenToRead (  1, cstr (g.fpscrootdir_s + "\\multiplayeravatar.dat").Get()  );
		g.steamworks.myAvatar_s = ReadString ( 1 );
		g.steamworks.myAvatarHeadTexture_s = ReadString ( 1 );
		g.steamworks.myAvatarName_s = g.steamworks.myAvatarHeadTexture_s;
		//  store the name of the head texture
		if (  g.steamworks.myAvatarHeadTexture_s  !=  "" ) 
		{
			g.steamworks.myAvatarHeadTexture_s = g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\"+ g.steamworks.myAvatarHeadTexture_s + "_cc.dds";
			if (  FileExist(g.steamworks.myAvatarHeadTexture_s.Get())  ==  0  )  g.steamworks.myAvatarHeadTexture_s  =  "";
		}
		CloseFile (  1 );

		if (  t.tShowAvatarSprite  ==  1 ) 
		{
			t.tShowAvatarSprite = 0;
			if (  g.charactercreatorEditorImageoffset > 1 ) 
			{
				if (  ImageExist(g.charactercreatorEditorImageoffset)  ==  1  )  DeleteImage (  g.charactercreatorEditorImageoffset );
				if (  FileExist( cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\"+g.steamworks.myAvatarName_s+".bmp").Get() )  ==  1 ) 
				{
					LoadImage (  cstr(g.fpscrootdir_s+"\\Files\\entitybank\\user\\charactercreator\\"+g.steamworks.myAvatarName_s+".bmp").Get() ,g.charactercreatorEditorImageoffset );
				}
			}
		}

	}

return;

}

void characterkit_customHead ( void )
{

	switch (  t.characterkitcontrol.customHeadMode ) 
	{
		//  choose image
		case 1:

			characterkit_draw ( );

			//  ensure no highlighting is being done
			SetObjectEffect (  t.characterkit.objectstart+1,t.characterkit.effectforcharacter );
			SetObjectEffect (  t.characterkit.objectstart+2,t.characterkit.effectforcharacter );
			SetObjectEffectCore (  t.characterkit.objectstart+3,t.characterkit.effectforBeard,1 );
			SetObjectEffectCore (  t.characterkit.objectstart+4,t.characterkit.effectforAttachments,1 );

			t.tCustomHeadRed_f = -1;

			Sync (  );
			//  set up default sample points
			t.ccSamplePointX[0] = 130;
			t.ccSamplePointY[0] = 75;
			t.ccSamplePointX[1] = 95;
			t.ccSamplePointY[1] = 118;
			t.ccSamplePointX[2] = 158;
			t.ccSamplePointY[2] = 121;
			//  move the sample points down for female
			if (  t.characterkitcontrol.fmaleProjection  ==  1 ) 
			{
				t.ccSamplePointX[1] = t.ccSamplePointX[1] - 8;
				t.ccSamplePointY[1] = t.ccSamplePointY[1] + 15;
				t.ccSamplePointX[2] = t.ccSamplePointX[2] + 8;
				t.ccSamplePointY[2] = t.ccSamplePointY[2] + 15;
			}

			SetCursor (  0,0 );
			t.tccmessage_s = "Choose image to use for your head";


			t.tRedColor_f = 255;
			t.tGreenColor_f = 255;
			t.tBlueColor_f = 255;

			t.tmakeFaceTexture = 0;

			ShowMouse (  );
			t.tFileName_s="";
			t.tentityprotected=1;
			t.ttitlemessage_s="Choose an image to use";
			while (  t.tentityprotected == 1 ) 
			{
				t.tFileName_s = openFileBox("All Files|*.*|PNG|*.png|JPEG|*.jpg|BMP|*.bmp|DDS|*.dds|TGA|*.tga|", "", "Select Head Image", "*.*", IMPORTEROPENFILE);

				if (  t.tFileName_s == "Error" || t.tFileName_s  ==  "" ) 
				{
					SetDir (  t.characterkitcontrol.originalDir_s.Get() );
					t.characterkit.headindex = t.characterkit.originalHeadIndex;
					t.characterkit.headindexloaded = -1;
					t.characterkit.head_s=t.characterkitheadbank_s[t.characterkit.headindex];
					t.characterkitcontrol.hasCustomHead = 0;
					t.characterkit.oldMouseClick = 1;
					t.characterkitcontrol.customHeadMode = 0;
					t.slidersmenuindex=t.characterkit.properties1Index;
					t.slidersmenuvalue[t.slidersmenuindex][4].value = t.characterkit.headindex;
					t.characterkit.thumbGadgetOn = 0;
					t.characterkit.loadthumbs = 0;
					return;
				}

//     `tempstring$ = Lower(Right(tFileName$,3))

//     `if tempstring$<>"jpg" and tempstring$<>"png" and tempstring$<>"bmp" and tempstring$<>"dds" and tempstring$<>"tga"

//     `tentityprotected=1

//     `else

					t.tentityprotected=0;
//     `endif

			}
			t.characterkitcontrol.customHeadMode = 2;

		break;
		//  load in image
		case 2:
			//  ensure it is an image file, if not jump back to grabbing file to avoid crashing out

				t.tfail = 0;
				if (  Len(t.tFileName_s.Get()) > 3 ) 
				{
					t.tempString_s = Lower(Right(t.tFileName_s.Get(),3));
					if ( t.tempString_s != "jpg" && t.tempString_s != "png" && t.tempString_s != "bmp" && t.tempString_s != "dds" && t.tempString_s != "tga"  )  t.tfail  =  1;
				}
				else
				{
					t.tfail = 1;
				}

				if (  t.tfail  ==  1 ) 
				{
					t.characterkitcontrol.customHeadMode = 1;
					return;
				}

			characterKitAdjustDisplayForCustomHead(1);

			t.tcustomHeadMessage_s = "Custom t.Head - Loading Image";

			t.tccidiffuse_s = t.tFileName_s;
			t.tobj=t.characterkit.objectstart+2;
			t.tassetimg=14;
//    `if ImageExist(t.characterkit.imagestart+tassetimg+0) == 1 then DeleteImage (  t.characterkit.imagestart+tassetimg+0 )
			
			if (  FileExist(t.tccidiffuse_s.Get())  ==  0  )  ExitPrompt (  cstr(cstr("Can't find ")+t.tccidiffuse_s).Get(), "Character Creator Error" );
			LoadImage (  t.tccidiffuse_s.Get(),t.characterkit.imagestart+t.tassetimg+40 );
			TextureObject (  t.tobj,0,t.characterkit.imagestart+t.tassetimg+40 );

			t.characterkitcontrol.customHeadMode = 3;
			SetDir (  t.characterkitcontrol.originalDir_s.Get() );

			ResetFindFreeSprite();
			t.theadProjBackSprite = CharacterKitFindFreeSprite();
			t.theadTexSprite = CharacterKitFindFreeSprite();
			t.theadProjGuideSprite = CharacterKitFindFreeSprite();
			t.theadProjFrontSprite2 = CharacterKitFindFreeSprite();
			t.theadProjFrontSprite = CharacterKitFindFreeSprite();
			t.theadmarker1 = CharacterKitFindFreeSprite();
			t.theadmarker2 = CharacterKitFindFreeSprite();
			t.theadmarker3 = CharacterKitFindFreeSprite();
			t.theadmarker4 = CharacterKitFindFreeSprite();
			t.theheadoriginal = CharacterKitFindFreeSprite();
			t.ccSampleSprite[0] = CharacterKitFindFreeSprite();
			t.ccSampleSprite[1] = CharacterKitFindFreeSprite();
			t.ccSampleSprite[2] = CharacterKitFindFreeSprite();

			//t.ttexx as float;
			//t.ttexy as float;
			//t.ttexw as float;
			//t.ttexh as float;
			//t.ttexa as float;
			t.ttexx = GetChildWindowWidth()/2;
			t.ttexy = GetChildWindowHeight()/2;
			t.ttexw = 256;
			t.ttexh = 256;
			t.ttexa = 0;
			t.characterkitcontrol.customHeadMouseMode = 0;
		break;
		case 3:


			t.tccmessage_s = "Move points to align face to the mask, right click and drag to rotate image";
			if (  t.characterkitcontrol.fmaleProjection  ==  1  )  t.tfmale_s  =  "fmale_"; else t.tfmale_s  =  "";
			if (  ImageExist(t.characterkit.imagestart+35)  ==  0 ) 
			{
				LoadImage (  cstr(cstr("characterkit\\bodyandhead\\") + t.tfmale_s + "projection_guide_D.dds").Get() , t.characterkit.imagestart+35 );
			}
			if (  ImageExist(t.characterkit.imagestart+36)  ==  0 ) 
			{
				LoadImage (  cstr(cstr("characterkit\\bodyandhead\\") + t.tfmale_s + "projection_frontplate_D.dds").Get() , t.characterkit.imagestart+36 );
			}
			if (  ImageExist(t.characterkit.imagestart+37)  ==  0 ) 
			{
				LoadImage (  cstr(cstr("characterkit\\bodyandhead\\") + t.tfmale_s + "projection_frontplate_Black_D.dds").Get() , t.characterkit.imagestart+37 );
			}
			if (  ImageExist(t.characterkit.imagestart+38)  ==  0 ) 
			{
				LoadImage (  "characterkit\\bodyandhead\\projection_frontplate_noalpha_D.DDS" , t.characterkit.imagestart+38 );
			}
			if (  ImageExist(t.characterkit.imagestart+39)  ==  0 ) 
			{
				//LoadImage (  "characterkit\\bodyandhead\\circle.png" , t.characterkit.imagestart+39 );
				LoadImage (  "characterkit\\bodyandhead\\circle.dds" , t.characterkit.imagestart+39 );
			}
			if (  ImageExist(t.characterkit.imagestart+40)  ==  0 ) 
			{
				if (  t.tfmale_s  ==  "" ) 
				{
					LoadImage (  "characterkit\\bodyandhead\\male_projection_basehead_D.dds" , t.characterkit.imagestart+40 );
				}
				else
				{
					LoadImage (  "characterkit\\bodyandhead\\fmale_projection_basehead_D.dds" , t.characterkit.imagestart+40 );
				}
			}
//    `if BitmapExist(9) = 0

//     `load bitmap "characterkit\\bodyandhead\\male_projection_basehead_D.dds" , 9

//     `set current bitmap 0

//    `endif


			//  out of bounds checks
			if (  t.ttexw < 32  )  t.ttexw  =  32;
			if (  t.ttexh < 32  )  t.ttexh  =  32;
			if (  t.ttexw > 1024 ) 
			{
				t.ttexx = GetChildWindowWidth()/2;
				t.ttexw = 256;
				t.characterkitcontrol.customHeadMouseMode = 0;
			}
			if (  t.ttexh > 1024 ) 
			{
				t.ttexy = GetChildWindowHeight()/2;
				t.ttexh = 256;
				t.characterkitcontrol.customHeadMouseMode = 0;
			}

			characterkit_DrawProjection ( );

			t.tx1_f = t.ttexx+(Sin(t.ttexa)*t.ttexh/2.0);
			t.ty1_f = t.ttexy-(Cos(t.ttexa)*t.ttexh/2.0);
			t.tx1_f = t.tx1_f+(Sin(t.ttexa-90)*t.ttexw/2.0);
			t.ty1_f = t.ty1_f-(Cos(t.ttexa-90)*t.ttexw/2.0);
			Sprite (  t.theadmarker1,t.tx1_f,t.ty1_f,t.characterkit.imagestart+39 );
			OffsetSprite (  t.theadmarker1,16,16 );
			SetSpritePriority (  t.theadmarker1,3 );

			t.tx2_f = t.tx1_f+(Sin(t.ttexa+90)*t.ttexw*1.0);
			t.ty2_f = t.ty1_f-(Cos(t.ttexa+90)*t.ttexw*1.0);
			Sprite (  t.theadmarker2,t.tx2_f,t.ty2_f,t.characterkit.imagestart+39 );
			OffsetSprite (  t.theadmarker2,16,16 );
			SetSpritePriority (  t.theadmarker2,3 );

			t.tx3_f = t.tx2_f+(Sin(t.ttexa+180)*t.ttexh*1.0);
			t.ty3_f = t.ty2_f-(Cos(t.ttexa+180)*t.ttexh*1.0);
			Sprite (  t.theadmarker3,t.tx3_f,t.ty3_f,t.characterkit.imagestart+39 );
			OffsetSprite (  t.theadmarker3,16,16 );
			SetSpritePriority (  t.theadmarker3,3 );

			t.tx4_f = t.tx3_f+(Sin(t.ttexa+270)*t.ttexw*1.0);
			t.ty4_f = t.ty3_f-(Cos(t.ttexa+270)*t.ttexw*1.0);
			Sprite (  t.theadmarker4,t.tx4_f,t.ty4_f,t.characterkit.imagestart+39 );
			OffsetSprite (  t.theadmarker4,16,16 );
			SetSpritePriority (  t.theadmarker4,3 );

			switch (  t.characterkitcontrol.customHeadMouseMode ) 
			{
				case 0:

					//  not in any editing mode
					t.tx = t.inputsys.xmouse;
					t.ty = t.inputsys.ymouse;
					t.tfoundsomething = 0;

					if (  t.tx  >=  t.tx1_f-16 && t.tx  <=  t.tx1_f+16 && t.ty >=  t.ty1_f-16 && t.ty  <=  t.ty1_f+16 ) 
					{
						SetSpriteDiffuse (  t.theadmarker1,255,255,0 );
						if (  MouseClick()  ==  1 ) 
						{
							t.characterkitcontrol.customHeadMouseMode = 1;
							t.toffx = t.inputsys.xmouse - t.tx1_f;
							t.toffy = t.inputsys.ymouse - t.ty1_f;
						}
					}
					else
					{
						SetSpriteDiffuse (  t.theadmarker1,255,255,255 );
					}
					if (  t.tx  >=  t.tx2_f-16 && t.tx  <=  t.tx2_f+16 && t.ty >=  t.ty2_f-16 && t.ty  <=  t.ty2_f+16 ) 
					{
						SetSpriteDiffuse (  t.theadmarker2,255,255,0 );
						if (  MouseClick()  ==  1 ) 
						{
							t.characterkitcontrol.customHeadMouseMode = 2;
							t.toffx = t.inputsys.xmouse - t.tx2_f;
							t.toffy = t.inputsys.ymouse - t.ty2_f;
						}
					}
					else
					{
						SetSpriteDiffuse (  t.theadmarker2,255,255,255 );
					}
					if (  t.tx  >=  t.tx3_f-16 && t.tx  <=  t.tx3_f+16 && t.ty >=  t.ty3_f-16 && t.ty  <=  t.ty3_f+16 ) 
					{
						SetSpriteDiffuse (  t.theadmarker3,255,255,0 );
						if (  MouseClick()  ==  1 ) 
						{
							t.characterkitcontrol.customHeadMouseMode = 3;
							t.toffx = t.inputsys.xmouse - t.tx3_f;
							t.toffy = t.inputsys.ymouse - t.ty3_f;
						}
					}
					else
					{
						SetSpriteDiffuse (  t.theadmarker3,255,255,255 );
					}
					if (  t.tx  >=  t.tx4_f-16 && t.tx  <=  t.tx4_f+16 && t.ty >=  t.ty4_f-16 && t.ty  <=  t.ty4_f+16 ) 
					{
						SetSpriteDiffuse (  t.theadmarker4,255,255,0 );
						if (  MouseClick()  ==  1 ) 
						{
							t.characterkitcontrol.customHeadMouseMode = 4;
							t.toffx = t.inputsys.xmouse - t.tx4_f;
							t.toffy = t.inputsys.ymouse - t.ty4_f;
						}
					}
					else
					{
						SetSpriteDiffuse (  t.theadmarker4,255,255,255 );
					}

					t.toldttexx = t.ttexx;
					t.toldttexy = t.ttexy;
					t.toldttexw = t.ttexw;
					t.toldttexh = t.ttexh;

					//  rotate
					if (  MouseClick()  ==  2 ) 
					{
						if (  t.inputsys.ymouse > GetChildWindowHeight()/2 ) 
						{
							t.ttexa = t.ttexa - (t.inputsys.xmouse - t.theadoldmx);
						}
						else
						{
							t.ttexa = t.ttexa + (t.inputsys.xmouse - t.theadoldmx);
						}
//       `ttexa = ttexa - (inputsys.ymouse - theadoldmy)

						while ( t.ttexa > 360 ) { t.ttexa = t.ttexa - 360; }
						while ( t.ttexa < 0 ) { t.ttexa = t.ttexa + 360; }
					}

				break;
				//  top left Box (  )
				case 1:
					if (  MouseClick()  ==  0  )  t.characterkitcontrol.customHeadMouseMode  =  0;

					t.tx1_f = t.inputsys.xmouse - t.toffx;
					t.ty1_f = t.inputsys.ymouse - t.toffy;

					t.dx_f = t.tx1_f - t.tx2_f;
					t.dy_f = t.ty1_f - t.ty2_f;
					t.ttexw = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);
					t.dx_f = t.tx1_f - t.tx4_f;
					t.dy_f = t.ty1_f - t.ty4_f;
					t.ttexh = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);

					t.ttexx = t.tx1_f+(Sin(t.ttexa+90)*t.ttexw/2.0);
					t.ttexy = t.ty1_f-(Cos(t.ttexa+90)*t.ttexw/2.0);
					t.ttexx = t.ttexx+(Sin(t.ttexa+180)*t.ttexh/2.0);
					t.ttexy = t.ttexy-(Cos(t.ttexa+180)*t.ttexh/2.0);

				break;
				//  top right Box (  )
				case 2:
					if (  MouseClick()  ==  0  )  t.characterkitcontrol.customHeadMouseMode  =  0;

					t.tx2_f = t.inputsys.xmouse - t.toffx;
					t.ty2_f = t.inputsys.ymouse - t.toffy;

					t.dx_f = t.tx1_f - t.tx2_f;
					t.dy_f = t.ty1_f - t.ty2_f;
					t.ttexw = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);
					t.dx_f = t.tx2_f - t.tx3_f;
					t.dy_f = t.ty2_f - t.ty3_f;
					t.ttexh = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);

					t.ttexx = t.tx2_f+(Sin(t.ttexa-90)*t.ttexw/2.0);
					t.ttexy = t.ty2_f-(Cos(t.ttexa-90)*t.ttexw/2.0);
					t.ttexx = t.ttexx+(Sin(t.ttexa+180)*t.ttexh/2.0);
					t.ttexy = t.ttexy-(Cos(t.ttexa+180)*t.ttexh/2.0);

				break;
				//  bottom right Box (  )
				case 3:
					if (  MouseClick()  ==  0  )  t.characterkitcontrol.customHeadMouseMode  =  0;

					t.tx3_f = t.inputsys.xmouse - t.toffx;
					t.ty3_f = t.inputsys.ymouse - t.toffy;

					t.dx_f = t.tx3_f - t.tx4_f;
					t.dy_f = t.ty3_f - t.ty4_f;
					t.ttexw = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);
					t.dx_f = t.tx2_f - t.tx3_f;
					t.dy_f = t.ty2_f - t.ty3_f;
					t.ttexh = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);

					t.ttexx = t.tx3_f+(Sin(t.ttexa-90)*t.ttexw/2.0);
					t.ttexy = t.ty3_f-(Cos(t.ttexa-90)*t.ttexw/2.0);
					t.ttexx = t.ttexx+(Sin(t.ttexa+0)*t.ttexh/2.0);
					t.ttexy = t.ttexy-(Cos(t.ttexa+0)*t.ttexh/2.0);

				break;
				//  bottom left Box (  )
				case 4:
					if (  MouseClick()  ==  0  )  t.characterkitcontrol.customHeadMouseMode  =  0;

					t.tx4_f = t.inputsys.xmouse - t.toffx;
					t.ty4_f = t.inputsys.ymouse - t.toffy;

					t.dx_f = t.tx3_f - t.tx4_f;
					t.dy_f = t.ty3_f - t.ty4_f;
					t.ttexw = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);
					t.dx_f = t.tx4_f - t.tx1_f;
					t.dy_f = t.ty4_f - t.ty1_f;
					t.ttexh = Sqrt(t.dx_f*t.dx_f + t.dy_f*t.dy_f);

					t.ttexx = t.tx4_f+(Sin(t.ttexa+90)*t.ttexw/2.0);
					t.ttexy = t.ty4_f-(Cos(t.ttexa+90)*t.ttexw/2.0);
					t.ttexx = t.ttexx+(Sin(t.ttexa+0)*t.ttexh/2.0);
					t.ttexy = t.ttexy-(Cos(t.ttexa+0)*t.ttexh/2.0);

				break;
			}//~    `

			t.theadoldmx = t.inputsys.xmouse;
			t.theadoldmy = t.inputsys.ymouse;

		break;
		//  save our head and cleanup
		case 4:
			characterkit_cleanupCustomHead ( );

			t.importerTabs[1].selected = 0;
			t.characterkit.headindexloaded = t.characterkit.headindex;
			characterKitAdjustDisplayForCustomHead(0);
			return;

		break;
		//  cancel custom head
		case 5:
			characterkit_cleanupCustomHead ( );
			t.importerTabs[1].selected = 0;
			t.characterkit.headindexloaded = t.characterkit.headindex;
			characterKitAdjustDisplayForCustomHead(0);
			return;

		break;
		//  user can place color samplers where they want
		case 10:

			t.tccmessage_s = "Move sample spots then click Match Face Colour";
			characterkit_DrawProjection ( );

			t.txo = (GetChildWindowWidth()/2) - 128;
			t.tyo = (GetChildWindowHeight()/2) - 128;

			switch (  t.characterkitcontrol.samplerMode ) 
			{
				case 0:
					t.toverone = -1;

					for ( t.tloop = 0 ; t.tloop<=  2; t.tloop++ )
					{
						if (  t.inputsys.xmouse  >=  t.ccSamplePointX[t.tloop]-16+t.txo && t.inputsys.xmouse  <=  t.ccSamplePointX[t.tloop]+16+t.txo && t.inputsys.ymouse >=  t.ccSamplePointY[t.tloop]-16+t.tyo && t.inputsys.ymouse  <=  t.ccSamplePointY[t.tloop]+16+t.tyo ) 
						{
							t.toverone = t.tloop;
						}
					}

					if (  MouseClick()  ==  1 && t.toverone  !=  -1 ) 
					{
						t.characterkitcontrol.samplerMode = 1;
						t.tsox = (t.inputsys.xmouse - t.txo) - t.ccSamplePointX[t.toverone];
						t.tsoy = (t.inputsys.ymouse - t.tyo) - t.ccSamplePointY[t.toverone];
					}

				break;

				case 1:
					t.ccSamplePointX[t.toverone] = (t.inputsys.xmouse - t.txo) - t.tsox;
					t.ccSamplePointY[t.toverone] = (t.inputsys.ymouse - t.tyo) - t.tsoy;
					if (  MouseClick()  ==  0  )  t.characterkitcontrol.samplerMode  =  0;
				break;
			}//~    `

			//  Draw the samplers
			for ( t.tloop = 0 ; t.tloop<=  2; t.tloop++ )
			{
				Sprite (  t.ccSampleSprite[t.tloop],t.ccSamplePointX[t.tloop]+t.txo,t.ccSamplePointY[t.tloop]+t.tyo,t.characterkit.imagestart+28 );
				SizeSprite (  t.ccSampleSprite[t.tloop],32,32 );
				OffsetSprite (  t.ccSampleSprite[t.tloop],16,16 );
				SetSpritePriority (  t.ccSampleSprite[t.tloop],10 );
				if (  t.toverone  ==  t.tloop ) 
				{
					SetSpriteDiffuse (  t.ccSampleSprite[t.tloop],255,255,255 );
				}
				else
				{
					SetSpriteDiffuse (  t.ccSampleSprite[t.tloop],180,180,180 );
				}
				PasteSprite (  t.ccSampleSprite[t.tloop],t.ccSamplePointX[t.tloop]+t.txo,t.ccSamplePointY[t.tloop]+t.tyo );
			}

		break;
	}//~  `

	//  show message
	pastebitmapfontcenter(t.tcustomHeadMessage_s.Get(),GetChildWindowWidth()/2,40,1,255);

	characterkit_updateCustomHeadTexture ( );
	t.toldheadmouse = MouseClick();
	t.characterkit.oldMouseClick = MouseClick();

return;

}

void characterkit_DrawProjection ( void )
{

	Sprite (  t.theadProjBackSprite,GetChildWindowWidth()/2 - 128, GetChildWindowHeight()/2 - 128,t.characterkit.imagestart+38  ); SizeSprite (  t.theadProjBackSprite, 256,256 );
	Sprite (  t.theadTexSprite,t.ttexx, t.ttexy,t.characterkit.imagestart+t.tassetimg+40  ); SizeSprite (  t.theadTexSprite, t.ttexw,t.ttexh  ); SetSpritePriority (  t.theadTexSprite, 1 );
	OffsetSprite (  t.theadTexSprite,t.ttexw/2.0,t.ttexh/2.0 );
	RotateSprite (  t.theadTexSprite, t.ttexa );
	PasteSprite (  t.theadTexSprite,t.ttexx, t.ttexy );

	Sprite (  t.theheadoriginal,GetChildWindowWidth()/2 - 128, GetChildWindowHeight()/2 - 128,t.characterkit.imagestart+40  ); SizeSprite (  t.theheadoriginal, 256,256 );
//  `set Sprite (  diffuse theheadoriginal,tRedColor#,tgreencolor#,tbluecolor# )

	PasteSprite (  t.theheadoriginal,GetChildWindowWidth()/2 - 128,GetChildWindowHeight()/2 - 128 );

	Sprite (  t.theadProjGuideSprite,GetChildWindowWidth()/2 - 128, GetChildWindowHeight()/2 - 128,t.characterkit.imagestart+35  ); SizeSprite (  t.theadProjGuideSprite, 256,256  ); SetSpritePriority (  t.theadProjGuideSprite, 3 );
	SetSpriteDiffuse (  t.theadProjGuideSprite,255,255,255 );
	Sprite (  t.theadProjFrontSprite,GetChildWindowWidth()/2 - 128, GetChildWindowHeight()/2 - 128,t.characterkit.imagestart+36  ); SizeSprite (  t.theadProjFrontSprite, 256,256  ); SetSpritePriority (  t.theadProjFrontSprite, 3 );
	Sprite (  t.theadProjFrontSprite2,GetChildWindowWidth()/2 - 128, GetChildWindowHeight()/2 - 128,t.characterkit.imagestart+36  ); SizeSprite (  t.theadProjFrontSprite2, 256,256  ); SetSpritePriority (  t.theadProjFrontSprite2 , 3 );

return;

}

void characterkit_updateCustomHeadTexture ( void )
{
	if (  t.characterkitcontrol.customHeadMouseMode  !=  0 || MouseClick()  !=  0 || t.tforceUpdateOfHead > 0 ) 
	{
		t.tforceUpdateOfHead = t.tforceUpdateOfHead-1;
		t.tassetimg=14;
		DeleteImage (  t.characterkit.imagestart+t.tassetimg+0 );
		GrabImage (  t.characterkit.imagestart+t.tassetimg+0,GetChildWindowWidth()/2 - 128,GetChildWindowHeight()/2 - 128,GetChildWindowWidth()/2 + 128,GetChildWindowHeight()/2 + 128,2 );
		t.tobj=t.characterkit.objectstart+2;
		TextureObject (  t.tobj,0,t.characterkit.imagestart+t.tassetimg+0 );
	}
	if (  t.tmakeFaceTexture > 0 ) 
	{
		t.tmakeFaceTexture = 0;
		t.tforceUpdateOfHead = 2;
		characterkit_updateCustomHeadTextureColorMatch ( );
	}
//  `if ImageExist(t.characterkit.imagestart+40) = 1

//   `paste image t.characterkit.imagestart+40,1500,0

//  `endif

return;

}

void characterkit_updateCustomHeadTextureColorMatch ( void )
{
	SetCursor (  0,300 );
	Print (  "mapping colors" );
	SetCurrentBitmap (  0 );
//  `if BitmapExist(10) then DeleteBitmapEx (  10 )
	
//  `create bitmap 10,256,256

//  `set current bitmap 10

	//  remake the current texture
	PasteImage (  t.characterkit.imagestart+t.tassetimg+0,0,0 );
	Sync (  );
	PasteImage (  t.characterkit.imagestart+t.tassetimg+0,0,0 );
	Sync (  );
	PasteImage (  t.characterkit.imagestart+t.tassetimg+0,0,0 );
//  `lock pixels

		//  sample 3 points on the face

		t.tpixel = GetPoint(t.ccSamplePointX[0],t.ccSamplePointY[0]);
		t.tr1_f = RgbR(t.tpixel);
		t.tg1_f = RgbG(t.tpixel);
		t.tb1_f = RgbB(t.tpixel);

		t.tpixel = GetPoint(t.ccSamplePointX[1],t.ccSamplePointY[1]);
		t.tr2_f = RgbR(t.tpixel);
		t.tg2_f = RgbG(t.tpixel);
		t.tb2_f = RgbB(t.tpixel);

		t.tpixel = GetPoint(t.ccSamplePointX[2],t.ccSamplePointY[2]);
		t.tr3_f = RgbR(t.tpixel);
		t.tg3_f = RgbG(t.tpixel);
		t.tb3_f = RgbB(t.tpixel);

		//  get average color
		t.tr1_f = (t.tr1_f+t.tr2_f+t.tr3_f)/3.0;
		t.tg1_f = (t.tg1_f+t.tg2_f+t.tg3_f)/3.0;
		t.tb1_f = (t.tb1_f+t.tb2_f+t.tb3_f)/3.0;

		t.tCustomHeadRed_f = t.tr1_f;
		t.tCustomHeadGreen_f = t.tg1_f;
		t.tCustomHeadBlue_f = t.tb1_f;

		//  sample default texture
		t.tpixel = GetPoint(73,170);
		t.tr2_f = RgbR(t.tpixel);
		t.tg2_f = RgbG(t.tpixel);
		t.tb2_f = RgbB(t.tpixel);

		//  work out the difference to apply
//   `tRedColor# = (tr2#/tr1#)

//   `tGreenColor# = (tg2#/tg1#)

//   `tBlueColor# = (tb2#/tb1#)


		t.tRedColor_f = (t.tr1_f/t.tr2_f);
		t.tGreenColor_f = (t.tg1_f/t.tg2_f);
		t.tBlueColor_f = (t.tb1_f/t.tb2_f);

//   `tRedColor# = (tr1#)

//   `tGreenColor# = (tg1#)

//   `tBlueColor# = (tb1#)

//  `unlock pixels

	SetCurrentBitmap (  0 );
	if (  BitmapExist(10)  )  DeleteBitmapEx (  10 );
//  `create bitmap 10,1024,1024

	if (  ImageExist(t.characterkit.imagestart+40)  ==  1  )  DeleteImage (  t.characterkit.imagestart+40 );
	if (  t.characterkitcontrol.fmaleProjection  ==  0 ) 
	{
		LoadImage (  "characterkit\\bodyandhead\\male_projection_basehead_D.png" , t.characterkit.imagestart+40 );
	}
	else
	{
		LoadImage (  "characterkit\\bodyandhead\\fmale_projection_basehead_D.png" , t.characterkit.imagestart+40 );
	}
	if (  MemblockExist(32)  ==  1  )  DeleteMemblock (  32 );
	CreateMemblockFromImage (  32,t.characterkit.imagestart+40 );

	t.mbSize = GetMemblockSize( 32 );

	for ( t.pos = 12 ; t.pos <=  t.mbSize - 12 ; t.pos+= 4 )
	{
	
		t.tr1_f = ReadMemblockByte( 32, t.pos + 2 );
		t.tg1_f = ReadMemblockByte( 32, t.pos + 1 );
		t.tb1_f = ReadMemblockByte( 32, t.pos + 0 );

		t.tr1_f = (t.tr1_f * (0.5+t.tRedColor_f)) * 0.75;
		t.tg1_f = (t.tg1_f * (0.5+t.tGreenColor_f)) * 0.75;
		t.tb1_f = (t.tb1_f * (0.5+t.tBlueColor_f)) * 0.75;

//   `tr1# = (tr1# * 0.25) +  (tRedColor# *.75)

//   `tg1# = (tg1# * 0.25) +  (tGreenColor# * 0.75)

//   `tb1# = (tb1# * 0.25) + (tBlueColor# * 0.75)



		if (  t.tr1_f > 255.0  )  t.tr1_f  =  255.0;
		if (  t.tg1_f > 255.0  )  t.tg1_f  =  255.0;
		if (  t.tb1_f > 255.0  )  t.tb1_f  =  255.0;
		if (  t.tr1_f < 0.0  )  t.tr1_f  =  0.0;
		if (  t.tg1_f < 0.0  )  t.tg1_f  =  0.0;
		if (  t.tb1_f < 0.0  )  t.tb1_f  =  0.0;
	
		WriteMemblockByte (  32, t.pos + 0, (byte)t.tb1_f );
		WriteMemblockByte (  32, t.pos + 1, (byte)t.tg1_f );
		WriteMemblockByte (  32, t.pos + 2, (byte)t.tr1_f );
	
	}

	DeleteImage (  t.characterkit.imagestart+40 );
	CreateImageFromMemblock (  t.characterkit.imagestart+40,32 );
	DeleteMemblock (  32 );
//  `set bitmap format 32

//  `if BitmapExist(9) = 1

//   `copy bitmap 9,10

//  `else

//   `exit prompt "Bitmap 9 does not exist", "Error"

//  `endif

	/*      
	SetCurrentBitmap (  10 );
	LockPixels (  );

		for ( t.y = 0 ; t.y<=  1024; t.y++ )
		{
			for ( t.x = 0 ; t.x<=  1024; t.x++ )
			{
					t.tpixel = GetPoint(t.x,t.y);
					t.tr1_f = RgbR(t.tpixel);
					t.tg1_f = RgbG(t.tpixel);
					t.tb1_f = RgbB(t.tpixel);

					t.tr1_f = t.tr1_f * t.tRedColor_f;
					t.tg1_f = t.tg1_f * t.tGreenColor_f;
					t.tb1_f = t.tb1_f * t.tBlueColor_f;
					if (  t.tr1_f > 255.0  )  t.tr1_f  =  255.0;
					if (  t.tg1_f > 255.0  )  t.tg1_f  =  255.0;
					if (  t.tb1_f > 255.0  )  t.tb1_f  =  255.0;
					if (  t.tr1_f < 0.0  )  t.tr1_f  =  0.0;
					if (  t.tg1_f < 0.0  )  t.tg1_f  =  0.0;
					if (  t.tb1_f < 0.0  )  t.tb1_f  =  0.0;
					Dot (  t.x,t.y,Rgb(t.tr1_f,t.tg1_f,t.tb1_f) );

			}
		}

	UnlockPixels (  );
	if (  ImageExist(t.characterkit.imagestart+40)  )  DeleteImage (  t.characterkit.imagestart+40 );
	GrabImage (  t.characterkit.imagestart+40,0,0,1024,1024,3 );
	if (  BitmapExist(10)  )  DeleteBitmapEx (  10 );
	*/    
	SetCurrentBitmap (  0 );
	characterkit_colorMatchBody ( );
return;

}

void characterkit_colorMatchBody ( void )
{

	t.tobj=t.characterkit.objectstart+1;
	SetCharacterCreatorTones (  t.tobj,0,t.tCustomHeadRed_f,t.tCustomHeadGreen_f,t.tCustomHeadBlue_f,0.5 );
	t.tnewred_f = t.tCustomHeadRed_f;
	t.tnewgreen_f = t.tCustomHeadGreen_f;
	t.tnewblue_f = t.tCustomHeadBlue_f;
	/*      
	//  check the texture file exists, if not, back out immediately
	if (  t.tccidiffuse_s  ==  ""  )  return;
	if (  FileExist(t.tccidiffuse_s)  ==  0  )  return;

	t.tassetimg=11;
	t.timg = t.characterkit.imagestart+t.tassetimg+0;
	if (  ImageExist(t.timg)  ==  1  )  DeleteImage (  t.timg );
	LoadImage (  t.tccidiffuse_s , t.timg );
	if (  MemblockExist(32)  ==  1  )  DeleteMemblock (  32 );
	CreateMemblockFromImage (  32,t.timg );

	t.mbSize = GetMemblockSize( 32 );

	for ( t.pos = 12 ; t.pos<=  t.mbSize - 12 step 4; t.pos+= 4 )
	{
	
		t.tr1_f = ReadMemblockByte( 32, t.pos + 2 );
		t.tg1_f = ReadMemblockByte( 32, t.pos + 1 );
		t.tb1_f = ReadMemblockByte( 32, t.pos + 0 );

		t.tr1_f = (t.tr1_f * (0.5+t.tRedColor_f)) * 0.75;
		t.tg1_f = (t.tg1_f * (0.5+t.tGreenColor_f)) * 0.75;
		t.tb1_f = (t.tb1_f * (0.5+t.tBlueColor_f)) * 0.75;

//   `tr1# = (tr1# * 0.25) +  (tRedColor# *.75)

//   `tg1# = (tg1# * 0.25) +  (tGreenColor# * 0.75)

//   `tb1# = (tb1# * 0.25) + (tBlueColor# * 0.75)



		if (  t.tr1_f > 255.0  )  t.tr1_f  =  255.0;
		if (  t.tg1_f > 255.0  )  t.tg1_f  =  255.0;
		if (  t.tb1_f > 255.0  )  t.tb1_f  =  255.0;
		if (  t.tr1_f < 0.0  )  t.tr1_f  =  0.0;
		if (  t.tg1_f < 0.0  )  t.tg1_f  =  0.0;
		if (  t.tb1_f < 0.0  )  t.tb1_f  =  0.0;
	
		WriteMemblockByte (  32, t.pos + 0, t.tb1_f );
		WriteMemblockByte (  32, t.pos + 1, t.tg1_f );
		WriteMemblockByte (  32, t.pos + 2, t.tr1_f );
	
	}

	DeleteImage (  t.timg );
	CreateImageFromMemblock (  t.timg );
	DeleteMemblock (  32 );
	//  apply the new texture
	t.tobj=t.characterkit.objectstart+1;
	TextureObject (  t.tobj,0,t.timg );
	*/    

return;

}

void characterkit_cleanupCustomHead ( void )
{

	for ( t.tloop = 35 ; t.tloop<=  40; t.tloop++ )
	{
		if (  ImageExist(t.characterkit.imagestart+t.tloop)  )  DeleteImage (  t.characterkit.imagestart+t.tloop );
	}
	if (  BitmapExist(9)  ==  1  )  DeleteBitmapEx (  9 );

	if (  t.theadProjBackSprite > 0 ) 
	{
		if (  SpriteExist(t.theadProjBackSprite)  )  DeleteSprite (  t.theadProjBackSprite );
	}
	if (  t.theadTexSprite > 0 ) 
	{
		if (  SpriteExist(t.theadTexSprite)  )  DeleteSprite (  t.theadTexSprite );
	}
	if (  t.theadProjGuideSprite > 0 ) 
	{
		if (  SpriteExist(t.theadProjGuideSprite)  )  DeleteSprite (  t.theadProjGuideSprite );
	}
	if (  t.theadProjFrontSprite > 0 ) 
	{
		if (  SpriteExist(t.theadProjFrontSprite)  )  DeleteSprite (  t.theadProjFrontSprite );
	}
	if (  t.theadProjFrontSprite2 > 0 ) 
	{
		if (  SpriteExist(t.theadProjFrontSprite2)  )  DeleteSprite (  t.theadProjFrontSprite2 );
	}
	if (  t.theheadoriginal > 0 ) 
	{
		if (  SpriteExist(t.theheadoriginal)  )  DeleteSprite (  t.theheadoriginal );
	}

	for ( t.tloop = 0 ; t.tloop<=  2; t.tloop++ )
	{
		if (  t.ccSampleSprite[t.tloop] > 0 ) 
		{
			if (  SpriteExist(t.ccSampleSprite[t.tloop])  )  DeleteSprite (  t.ccSampleSprite[t.tloop] );
		}
	}

	if (  t.theadmarker1 > 0 ) 
	{
		if (  SpriteExist(t.theadmarker1)  )  DeleteSprite (  t.theadmarker1 );
		if (  SpriteExist(t.theadmarker2)  )  DeleteSprite (  t.theadmarker2 );
		if (  SpriteExist(t.theadmarker3)  )  DeleteSprite (  t.theadmarker3 );
		if (  SpriteExist(t.theadmarker4)  )  DeleteSprite (  t.theadmarker4 );
	}

	t.characterkitcontrol.customHeadMouseMode = 0;
	t.characterkitcontrol.customHeadMode = 0;

	t.characterkit.oldMouseClick = 1;

return;

}

void characterkit_convertCloneInstance ( void )
{
	//  needs tte for e
	characterkit_updateCharacterCreatorEntityInGame ( );
return;

//  NOTE; globals not in types is BAD BAD BAD!!
g.characterkitfindfreespritestart = 1;

}

void characterKitAdjustDisplayForCustomHead ( int adjust )
{
	int slidersmenuindex = 0;
	int tobj = 0;
	if (  adjust  ==  1 ) 
	{
		//  shift main character over so we can see it

		tobj=t.characterkit.objectstart+1;

		YRotateObject (  tobj,0 );
		t.characterkitcontrol.origPosX_f = ObjectPositionX(tobj);
		t.characterkitcontrol.origPosY_f = ObjectPositionY(tobj);
		t.characterkitcontrol.origPosZ_f = ObjectPositionZ(tobj);
		t.characterkitcontrol.origAngX_f = ObjectAngleX(tobj);
		t.characterkitcontrol.origAngY_f = ObjectAngleY(tobj);
		t.characterkitcontrol.origAngZ_f = ObjectAngleZ(tobj);

		MoveObjectDown (  tobj,10 );
		MoveObjectLeft (  tobj,30 );
		MoveObject (  tobj,-30 );
		TurnObjectLeft (  tobj,40 );

		//  hide gui
		slidersmenuindex=t.characterkit.properties1Index;
		t.slidersmenu[g.slidersmenumax].tleft = 100000;

	}
	else
	{
		//  shift main character over so we can see it
		tobj=t.characterkit.objectstart+1;
		PositionObject (  tobj,t.characterkitcontrol.origPosX_f,t.characterkitcontrol.origPosY_f,t.characterkitcontrol.origPosZ_f );
		RotateObject (  tobj,t.characterkitcontrol.origAngX_f,t.characterkitcontrol.origAngY_f,t.characterkitcontrol.origAngZ_f );

		//  hide gui
		slidersmenuindex=t.characterkit.properties1Index;
		t.slidersmenu[g.slidersmenumax].tleft = (GetChildWindowWidth() ) - 265;

	}
//endfunction

}

void ResetFindFreeSprite ( void )
{
	g.characterkitfindfreespritestart = 1;
//endfunction

}

int CharacterKitFindFreeSprite ( void )
{
	int tfindspriteloop = 0;
	int spr = 0;
	spr = 0;
	if (  g.characterkitfindfreespritestart < 1  )  g.characterkitfindfreespritestart  =  1;

	for ( tfindspriteloop = g.characterkitfindfreespritestart ; tfindspriteloop<=  10000; tfindspriteloop++ )
	{
		if (  SpriteExist(tfindspriteloop)  ==  0 ) 
		{
			spr = tfindspriteloop;
			g.characterkitfindfreespritestart = spr+1;
			break;
		}
	}
//endfunction spr
	return spr
;
}

char* CharacterKitPadString ( char* tString )
{
	while (  Len(tString) < 17 ) 
	{
		strcat (tString , " " );
	}
//endfunction tString
	return tString;
}

void writelog ( char* tstring )
{
	if (  FileExist("f:\alog.txt" )  ==  0 )
	{
		OpenToWrite (  10, "f:\\alog.txt" );
	}
	WriteString (  10,tstring );
//endfunction

}

//  check if the user has made any character creator characters
int CharacterKitCheckForUserMade ( void )
{
	cstr toldfolder_s =  "";
	//cstr t.tfile_s =  "";
	int result = 0;
	result = 0;
	sprintf ( t.szwork , "%s\\Files\\entitybank\\user\\charactercreator" , g.fpscrootdir_s.Get() );
	if (  PathExist( t.szwork )  ==  1 ) 
	{
	//  store original folder
		toldfolder_s = GetDir();
		sprintf ( t.szwork , "%s\\Files\\entitybank\\user\\charactercreator" , g.fpscrootdir_s.Get() );
		SetDir (  t.szwork );
		//  check for character fpe's
		ChecklistForFiles ( );

		for ( int c = 1 ; c<=  ChecklistQuantity(); c++ )
		{
			t.tfile_s=Lower(ChecklistString(c));
			if (  t.tfile_s != "." && t.tfile_s != ".." ) 
			{
				if (  strcmp ( Right(t.tfile_s.Get(),4) , ".fpe" ) == 0 ) 
				{
					result = 1;
					break;
				}
			}
		}

		//  return to original folder
		SetDir (  toldfolder_s.Get() );
	}
//endfunction result
	return result;
}

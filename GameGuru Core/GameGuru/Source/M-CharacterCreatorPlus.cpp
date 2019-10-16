//----------------------------------------------------
//--- GAMEGURU - M-CharacterCreatorPlus
//----------------------------------------------------

// Includes
#include "gameguru.h"
#include "M-CharacterCreatorPlus.h"

// Globals
bool g_bCharacterCreatorPlusActivated = false;
sCharacterCreatorPlus g_CharacterCreatorPlus;

// Functions
void charactercreatorplus_init(void)
{
	// Initialisation prompt
	pastebitmapfontcenter("INIT CHARACTER CREATOR PLUS", GetChildWindowWidth() / 2, 40, 1, 255); Sync();
	g_CharacterCreatorPlus.bInitialised = true;

	// hide other editor stuff
	terrain_paintselector_hide();
	waypoint_hideall ( );

	// scan for all character parts
	cstr olddir_s = GetDir();
	SetDir ( "charactercreatorplus\\parts\\adult male" );
	ChecklistForFiles();
	for ( int c = 1; c <= ChecklistQuantity(); c++ )
	{
		cstr tfile_s = Lower(ChecklistString(c));
		if (tfile_s != "." && tfile_s != ".." )
		{
			if ( strcmp ( Right(tfile_s.Get(),9) , "_head.cci" ) == 0 )
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
		}
	}
	SetDir (olddir_s.Get() );

	// create default character to begin creation
	int iCharObj = g.characterkitobjectoffset + 1;
	LoadObject ( "charactercreatorplus\\parts\\adult male\\adult male body 02.X", iCharObj );
	int iCharTextureWhite = g.charactercreatorEditorImageoffset + 0;
	LoadImage ( "effectbank\\reloaded\\media\\blank_O.DDS", iCharTextureWhite );
	int iCharTexture = g.charactercreatorEditorImageoffset + 1;
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male body 01_color.dds", iCharTexture+0 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male body 01_normal.dds", iCharTexture+2 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male body 01_metal.dds", iCharTexture+3 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male body 01_roughness.dds", iCharTexture+4 );
	TextureObject ( iCharObj, 0, iCharTexture+0 );
	TextureObject ( iCharObj, 1, iCharTextureWhite );
	TextureObject ( iCharObj, 2, iCharTexture+2 );
	TextureObject ( iCharObj, 3, iCharTexture+3 );
	TextureObject ( iCharObj, 4, iCharTexture+4 );
	TextureObject ( iCharObj, 5, iCharTextureWhite );
	SetObjectEffect ( iCharObj, g.thirdpersoncharactereffect );
	PositionObject ( iCharObj, 25600, 600, 25600 );
	SetObjectArtFlags ( iCharObj, 1<<1, 0 );

	// stitch head onto body ;)
	int iCharObjHead = g.characterkitobjectoffset + 2;
	LoadObject ( "charactercreatorplus\\parts\\adult male\\adult male head 02.X", iCharObjHead );
	int iCharHeadTexture = g.charactercreatorEditorImageoffset + 11;
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male head 01_color.dds", iCharHeadTexture+0 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male head 01_normal.dds", iCharHeadTexture+2 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male head 01_metal.dds", iCharHeadTexture+3 );
	LoadImage ( "charactercreatorplus\\parts\\adult male\\adult male head 01_roughness.dds", iCharHeadTexture+4 );
	TextureObject ( iCharObjHead, 0, iCharHeadTexture+0 );
	TextureObject ( iCharObjHead, 1, iCharTextureWhite );
	TextureObject ( iCharObjHead, 2, iCharHeadTexture+2 );
	TextureObject ( iCharObjHead, 3, iCharHeadTexture+3 );
	TextureObject ( iCharObjHead, 4, iCharHeadTexture+4 );
	TextureObject ( iCharObjHead, 5, iCharTextureWhite );
	SetObjectEffect ( iCharObjHead, g.thirdpersoncharactereffect );

	StealMeshesFromObject ( iCharObj, iCharObjHead );
	DeleteObject ( iCharObjHead );

	LoopObject ( iCharObj, 0, 483 );
}

void charactercreatorplus_free(void)
{
	// show editor stuff
	waypoint_showall();

	// deactivate character creator plus
	g_bCharacterCreatorPlusActivated = false;
}

void charactercreatorplus_loop(void)
{
	if ( g_CharacterCreatorPlus.bInitialised == false )
	{
		// set up for character editing
		charactercreatorplus_init();
	}
	else
	{
		// editing character mode

		
	}
}


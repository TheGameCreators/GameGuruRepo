//----------------------------------------------------
//--- GAMEGURU - Welcome System
//----------------------------------------------------

// Includes
#include "M-WelcomeSystem.h"
#include "gameguru.h"
#include "CInputC.h"
#include <time.h>
#include <wininet.h>

// Externals
extern int g_trialStampDaysLeft;

// Globals
int g_welcomesystemclosedown = 0;
struct welcomepagetype
{
	int iTopLeftX;
	int iTopLeftY;
	int iPageWidth;
	int iPageHeight;
	int iCenterX;
	int iCenterY;
	float fPercToPageX;
	float fPercToPageY;
};
welcomepagetype g_welcome;

struct welcomebuttontype
{
	int x1, y1, x2, y2;
	int alpha;
};
welcomebuttontype g_welcomebutton[20];
int g_welcomebuttoncount = 0;
bool gbWelcomeSystemActive = false;
int g_welcomeCycle = 0;
char g_welcomeText[1024];
char g_welcomeImageUrl[1024];
char g_welcomeLinkUrl[1024];

void welcome_loadasset ( cstr welcomePath, LPSTR pImageFilename, int iImageID )
{
	cstr imageFile = welcomePath+pImageFilename; 
	LoadImage ( imageFile.Get(), iImageID );
}

void welcome_waitfornoinput ( void )
{
	OpenFileMap (  1, "FPSEXCHANGE" );
	SetEventAndWait (  1 );
	do
	{
		t.inputsys.kscancode=ScanCode();
		t.inputsys.mclick=MouseClick();
		t.terrain.gameplaycamera=0;
		terrain_shadowupdate ( );
		terrain_update ( );
		PasteImage ( g.editorimagesoffset+12, 0, 0 );
		FastSync (  );
	} while ( !( t.inputsys.kscancode == 0 && t.inputsys.mclick == 0 ) );
}

void welcome_init ( int iLoadingPart )
{
	// iLoadingPart:
	// 1-only load anim backdrop (quick load and show)
	// 2-load rest of core gfx for welcome system
	// 0-load MAIN gfx

	// welcome system is active
	gbWelcomeSystemActive = true;

	// load assets for welcome system (can use g.editorimagesoffset+8 through +60)
	// in addition, cannot have 10, 13, 14, 18, 21, 22, 23, 26 (from editor core)
	cstr welcomePath = cstr("languagebank\\")+g.language_s+cstr("\\artwork\\");

	// load only backdrop for quick refresh when launching
	if ( iLoadingPart == 1 )
	{
		// only interested in anim backdrop for now
		welcome_loadasset ( welcomePath, "welcome\\animated-backdrop.png", g.editorimagesoffset+12 );
		welcome_loadasset ( welcomePath, "welcome-assets\\splash-logo.bmp", g.editorimagesoffset+41 );
	}
	if ( iLoadingPart == 2 )
	{
		// load welcome system assets
		welcome_loadasset ( welcomePath, "welcome\\welcome-page.png", g.editorimagesoffset+8 );

		// what you get
		welcome_loadasset ( welcomePath, "welcome-assets\\product-logo.png", g.editorimagesoffset+9 );
		welcome_loadasset ( welcomePath, "welcome\\animated-backdrop.png", g.editorimagesoffset+12 );

		// load in 3x3 pieces
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-1.png", g.editorimagesoffset+31 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-2.png", g.editorimagesoffset+32 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-3.png", g.editorimagesoffset+33 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-4.png", g.editorimagesoffset+34 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-5.png", g.editorimagesoffset+35 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-6.png", g.editorimagesoffset+36 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-7.png", g.editorimagesoffset+37 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-8.png", g.editorimagesoffset+38 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-line-9.png", g.editorimagesoffset+39 );
	}
	if ( iLoadingPart == 0 )
	{
		// load tick/bits assets
		welcome_loadasset ( welcomePath, "welcome\\welcome-tick.png", g.editorimagesoffset+40 );

		// welcome main page
		welcome_loadasset ( welcomePath, "welcome\\welcome-learn.png", g.editorimagesoffset+56 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-play.png", g.editorimagesoffset+57 );
		welcome_loadasset ( welcomePath, "welcome\\welcome-create.png", g.editorimagesoffset+58 );

		// screenshot thumb for sample level
		welcome_loadasset ( welcomePath, "welcome-assets\\product-example.png", g.editorimagesoffset+59 );

		// image if free weekend used
		// reserved  g.editorimagesoffset+61  test game loaded arrow
		if ( g.iFreeVersionModeActive == 1 )
		{
			// only used for free weekend build version
			welcome_loadasset ( welcomePath, "welcome-assets\\free-weekend.png", g.editorimagesoffset+60 );
			welcome_loadasset ( welcomePath, "welcome-assets\\free-weekend-prompt.png", g.editorimagesoffset+62 );
			welcome_loadasset ( welcomePath, "welcome-assets\\free-weekend-click.png", g.editorimagesoffset+63 );
		}
		if ( g.iFreeVersionModeActive == 2 )
		{
			// only used for free trial version
			welcome_loadasset ( welcomePath, "welcome-assets\\free-trial.png", g.editorimagesoffset+60 );
			welcome_loadasset ( welcomePath, "welcome-assets\\free-trial-prompt.png", g.editorimagesoffset+62 );
			welcome_loadasset ( welcomePath, "welcome-assets\\free-trial-click.png", g.editorimagesoffset+63 );
		}
		
		// announcement system
		welcome_loadasset ( welcomePath, "welcome-assets\\gameguru-news-banner.png", g.editorimagesoffset+64 );
	}

	// calculate page dimensions and useful vars
	memset ( &g_welcome, 0, sizeof(g_welcome) );
	g_welcome.iPageWidth = ImageWidth ( g.editorimagesoffset+8 );
	g_welcome.iPageHeight = ImageHeight ( g.editorimagesoffset+8 );
	g_welcome.iCenterX = GetChildWindowWidth(0)/2;
	g_welcome.iCenterY = GetChildWindowHeight(0)/2;
	g_welcome.iTopLeftX = g_welcome.iCenterX-(g_welcome.iPageWidth/2);
	g_welcome.iTopLeftY = g_welcome.iCenterY-(g_welcome.iPageHeight/2);
	g_welcome.fPercToPageX = g_welcome.iPageWidth / 100.0f;
	g_welcome.fPercToPageY = g_welcome.iPageHeight / 100.0f;

	// master flag to close down welcome system from anywhere within init/free sequence
	g_welcomesystemclosedown = 0;
}

void welcome_free ( void )
{
	// free welcome assets (save memory)
	for ( int iID = 30; iID <= 60; iID++ )
		if ( iID != 12 )
			if ( GetImageExistEx ( g.editorimagesoffset + iID ) == 1 )
				DeleteImage ( g.editorimagesoffset + iID );

	// welcome system not active
	gbWelcomeSystemActive = false;

	// if triggered a load, do it here, just before delete backdrop
	if ( strlen(t.tlevelautoload_s.Get()) > 0 )
	{
		// just render backdrop for the load pause 
		welcome_staticbackdrop();

		//  ask to save first if modified project open
		t.editorcanceltask=0;
		if ( g.projectmodified == 1 ) 
		{
			// If project modified, ask if want to save first
			gridedit_intercept_savefirst ( );
		}
		if ( t.editorcanceltask == 0 ) 
		{
			if ( t.tlevelautoload_s != "" ) 
			{
				if ( cstr(Lower(Right(t.tlevelautoload_s.Get(),4))) == ".fpm" ) 
				{
					// leave dialog for the load
					g.projectfilename_s=t.tlevelautoload_s;
					gridedit_load_map ( );
					g.showtestlevelclickprompt = timeGetTime() + 5000;

					// prevent brand new level from being wiped
					t.bIgnoreFirstCallToNewLevel = true;
				}
			}
		}
	}

	// now delete backdrop
	if ( GetImageExistEx ( g.editorimagesoffset + 12 ) == 1 ) DeleteImage ( g.editorimagesoffset + 12 );
}

void welcome_animbackdrop ( void )
{
	SetSprite ( 123, 0, 1 );
	SetSprite ( 124, 0, 1 );
	Sprite ( 123, -100000, -100000, g.editorimagesoffset+12 );
	Sprite ( 124, -100000, -100000, g.editorimagesoffset+41 );
	for ( int iPercCol = 0; iPercCol <= 255; iPercCol++ )
	{
		Cls();
		SetSpriteAlpha ( 123, iPercCol );
		PasteSprite ( 123, 0, 0 );
		int iLogoFade = (iPercCol*2)-255; if ( iLogoFade < 0 ) iLogoFade = 0;
		SetSpriteAlpha ( 124, iLogoFade );
		int iImgWidth = ImageWidth(g.editorimagesoffset+41);
		int iImgHeight = ImageHeight(g.editorimagesoffset+41);
		iImgWidth = (iImgWidth/255.0f)*iPercCol;
		iImgHeight = (iImgHeight/255.0f)*iPercCol;
		SizeSprite ( 124, iImgWidth, iImgHeight );
		PasteSprite ( 124, (GetChildWindowWidth(0)-iImgWidth)/2, (GetChildWindowHeight(0)-iImgHeight)/2 );
		Sync();
		if ( g.gvsync != 0 ) 
		{
			if ( iPercCol != 255 )
			{
				iPercCol+=3;
				if ( iPercCol >= 254 ) iPercCol = 254;
			}
		}
		else
			Sleep(2);
	}
}

void welcome_staticbackdrop ( void )
{
	//SetSprite ( 123, 0, 1 );
	//Sprite ( 123, -100000, -100000, g.editorimagesoffset+12 );
	//SizeSprite ( 123, GetDesktopWidth(), GetDesktopHeight() );
	for ( int iSyncPass = 0; iSyncPass <= 1; iSyncPass++ )
	{
		PasteImage ( g.editorimagesoffset+12, 0, 0 );
		//Cls();
		//SetSpriteAlpha ( 123, 255 );
		//PasteSprite ( 123, 0, 0 );
		Sync();
	}
}

void welcome_updatebackdrop ( char* pText )
{
	if ( SpriteExist(123)==1 ) Sprite ( 123, -10000, -10000, g.editorimagesoffset+12 );
	if ( SpriteExist(124)==1 ) DeleteSprite(124);
	for ( int s = 0; s < 2; s++ )
	{
		Cls();
		PasteImage ( g.editorimagesoffset+12, 0, 0 );
		Sprite ( 123, -10000, -10000, g.editorimagesoffset+41 );
		int iImgWidth = ImageWidth(g.editorimagesoffset+41);
		int iImgHeight = ImageHeight(g.editorimagesoffset+41);
		SizeSprite ( 123, iImgWidth, iImgHeight );
		PasteSprite ( 123, (GetChildWindowWidth(0)-iImgWidth)/2, (GetChildWindowHeight(0)-iImgHeight)/2 );
		pastebitmapfontcenter ( pText, GetChildWindowWidth(0)/2, ((GetChildWindowHeight(0)-iImgHeight)/2)+(iImgHeight)-48, 4, 255);
		Sync();
	}
}

void welcome_converttopagecoords ( float& fX1, float& fY1 )
{
	fX1 *= g_welcome.fPercToPageX;
	fY1 *= g_welcome.fPercToPageY;
}

void welcome_converttopagecoords ( float& fX1, float& fY1, float& fX2, float& fY2 )
{
	fX1 *= g_welcome.fPercToPageX;
	fY1 *= g_welcome.fPercToPageY;
	fX2 *= g_welcome.fPercToPageX;
	fY2 *= g_welcome.fPercToPageY;
}

void welcome_drawbox_core ( int iButtonID, float fX1, float fY1, float fX2, float fY2, int iVisible )
{
	// use 3x3 pieces to form a variable sized box
	welcome_converttopagecoords ( fX1, fY1, fX2, fY2 );
	int iBoxX1 = (g_welcome.iTopLeftX + fX1)-8;
	int iBoxY1 = (g_welcome.iTopLeftY + fY1)-8;
	int iBoxX2 = (g_welcome.iTopLeftX + fX2);
	int iBoxY2 = (g_welcome.iTopLeftY + fY2);
	if ( iVisible == 1 )
	{
		PasteImage ( g.editorimagesoffset+31, iBoxX1, iBoxY1, 1 );
		for ( int iX = iBoxX1+8; iX < iBoxX2; iX+=8 )
		{
			PasteImage ( g.editorimagesoffset+32, iX, iBoxY1, 1 );
			PasteImage ( g.editorimagesoffset+38, iX, iBoxY2, 1 );
		}
		PasteImage ( g.editorimagesoffset+33, iBoxX2, iBoxY1, 1 );
		for ( int iY = iBoxY1+8; iY < iBoxY2; iY+=8 )
		{
			PasteImage ( g.editorimagesoffset+34, iBoxX1, iY, 1 );
			PasteImage ( g.editorimagesoffset+36, iBoxX2, iY, 1 );
		}
		PasteImage ( g.editorimagesoffset+37, iBoxX1, iBoxY2, 1 );
		PasteImage ( g.editorimagesoffset+39, iBoxX2, iBoxY2, 1 );
	}

	// assign button if one
	if ( iButtonID > 0 )
	{
		if ( iButtonID > g_welcomebuttoncount ) g_welcomebuttoncount = iButtonID;
		g_welcomebutton[iButtonID].x1 = iBoxX1;
		g_welcomebutton[iButtonID].y1 = iBoxY1;
		g_welcomebutton[iButtonID].x2 = iBoxX2;
		g_welcomebutton[iButtonID].y2 = iBoxY2;
		g_welcomebutton[iButtonID].alpha = 192;
	}
}

void welcome_drawbox ( int iButtonID, float fX1, float fY1, float fX2, float fY2 )
{
	welcome_drawbox_core ( iButtonID, fX1, fY1, fX2, fY2, 1 );
}

void welcome_drawbox_hide ( int iButtonID, float fX1, float fY1, float fX2, float fY2 )
{
	welcome_drawbox_core ( iButtonID, fX1, fY1, fX2, fY2, 0 );
}

void welcome_text ( LPSTR pText, int iFontType, float fX, float fY, float fAlpha, bool bMultiline, bool bLeftAligned )
{
	welcome_converttopagecoords ( fX, fY );
	int iNextLine = 0;
	bool bHaveALine = true;
	char pLineData[10240];
	char pThisLine[10240];
	char pWorkLine[10240];
	strcpy ( pLineData, pText );
	while ( bHaveALine==true )
	{
		bHaveALine = false;
		strcpy ( pThisLine, pLineData );
		if ( bMultiline == true )
		{
			for ( int n = 0; n < strlen(pLineData); n++ )
			{
				if ( pLineData[n] == '\n' )
				{
					strcpy ( pThisLine, pLineData );
					pThisLine[n] = 0;
					strcpy ( pWorkLine, pLineData+n+1 );
					strcpy ( pLineData, pWorkLine );
					bHaveALine = true;
					break;
				}
			}
		}
		int iTextRealHeight = getbitmapfontheight(iFontType);
		float fTextWidth = getbitmapfontwidth(pThisLine,iFontType) / g_welcome.fPercToPageX;
		float fTextHeight = iTextRealHeight / g_welcome.fPercToPageY;
		int iTextX = (g_welcome.iTopLeftX + fX);
		int iTextY = (g_welcome.iTopLeftY + fY + 1) - (iTextRealHeight/2.0f);
		if ( bLeftAligned == true )
			pastebitmapfont ( pThisLine, iTextX, iTextY+(iNextLine*iTextRealHeight), iFontType, fAlpha);
		else
			pastebitmapfontcenter ( pThisLine, iTextX, iTextY+(iNextLine*iTextRealHeight), iFontType, fAlpha);
		iNextLine++;
	}
}

void welcome_textinbox ( int iButtonID, LPSTR pText, int iFontType, float fX, float fY, float fAlpha )
{
	float fTextWidth = getbitmapfontwidth(pText,iFontType) / g_welcome.fPercToPageX;
	float fTextHeight = getbitmapfontheight(iFontType) / g_welcome.fPercToPageY;
	float fX1 = fX - fTextWidth;
	float fY1 = fY - (fTextHeight/2);
	float fX2 = fX + fTextWidth;
	float fY2 = fY + (fTextHeight/2);
	if ( fAlpha >= 0 )
		welcome_drawbox ( iButtonID, fX1, fY1, fX2, fY2 );
	else
	{
		welcome_drawbox_hide ( iButtonID, fX1, fY1, fX2, fY2 );
		fAlpha = 0.0f;
	}
	if ( iButtonID == 0 ) fAlpha = 32;
	welcome_text ( pText, iFontType, fX, fY, fAlpha, false, false );
}

void welcome_drawimage ( int iImageID, float fX, float fY, bool bLeftAligned )
{
	welcome_converttopagecoords ( fX, fY );
	int iX = g_welcome.iTopLeftX + fX;
	int iY = g_welcome.iTopLeftY + fY;
	if ( bLeftAligned == false ) iX -= (ImageWidth(iImageID)/2.0f);
	PasteImage ( iImageID, iX, iY );
}

void welcome_drawrotatedimage ( int iImageID, float fX, float fY, float fAngle, int iOffsetX, int iOffsetY, bool bLeftAligned )
{
	welcome_converttopagecoords ( fX, fY );
	int iX = g_welcome.iTopLeftX + fX;
	int iY = g_welcome.iTopLeftY + fY;
	Sprite ( 123, -10000, -10000, iImageID );
	SizeSprite ( 123, ImageWidth(iImageID), ImageHeight(iImageID) );
	OffsetSprite ( 123, iOffsetX, iOffsetY );
	RotateSprite ( 123, fAngle );
	if ( bLeftAligned == false ) iX -= (ImageWidth(iImageID)/2.0f);
	PasteSprite ( 123, iX, iY );
}

// SERIAL CODE PAGE

struct welcomeserialcodetype
{
	int iNextButtonID;
	char pCode[23];
	int iKeyPressed;
	int iAfterError;
};
welcomeserialcodetype g_welcomeserialcode;

void welcome_serialcode_init ( void )
{
	memset ( &g_welcomeserialcode, 0, sizeof(g_welcomeserialcode) );
	g_welcomeserialcode.iNextButtonID = 1;
	strcpy ( g_welcomeserialcode.pCode, "" );
	g_welcomeserialcode.iKeyPressed = 1;
	g_welcomeserialcode.iAfterError = 0;
}

void welcome_serialcode_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_drawrotatedimage ( g.editorimagesoffset+9, 50, 5, 0, 0, 0, false );
	welcome_drawbox ( 0, 10, 23, 90, 81 );
	welcome_text ( "SERIAL CODE ENTRY\n", 1, 50, 28, 192, true, false );
	if ( g_welcomeserialcode.iAfterError > 1 )
	{
		welcome_text ( "Serial code has been entered incorrectly or has expired.", 1, 50, 50, 192, true, false );
		int iOkayID = 3; welcome_textinbox ( iOkayID, "TRY AGAIN", 1, 50, 70, g_welcomebutton[iOkayID].alpha );
	}
	else
	{
		welcome_text ( "Please enter the 22-digit serial code to activate this software.", 1, 50, 43, 192, true, false );
		welcome_drawbox ( 0, 30, 50, 70, 60 );
		welcome_text ( g_welcomeserialcode.pCode, 1, 50, 55, 192, true, false );
		welcome_text ( "You can use CTRL+V to paste contents of clipboard.", 2, 50, 65, 192, true, false );
		iID = g_welcomeserialcode.iNextButtonID; if ( strlen(g_welcomeserialcode.pCode) < 22 ) iID = 0;
		welcome_textinbox ( iID, "CONFIRM", 1, 50, 90, g_welcomebutton[iID].alpha );
		if ( g_welcomeserialcode.iAfterError == 1 )
		{
			int iQuitID = 2; welcome_textinbox ( iQuitID, "QUIT", 1, 85, 90, g_welcomebutton[iQuitID].alpha );
		}
	}

	// serial code entry control
	int iKeyScanCode = ScanCode();
	if ( iKeyScanCode == 0 ) g_welcomeserialcode.iKeyPressed = 0;
	if ( iKeyScanCode > 0 && g_welcomeserialcode.iKeyPressed == 0)
	{
		unsigned char c = InKey();
		//sprintf ( g_welcomeserialcode.pCode, "%d %d", iKeyScanCode, c );
		if ( (c >= 48 && c <= 57) || (c >= 97 && c <= 122) )
		{
			g_welcomeserialcode.iKeyPressed = 1;
			if ( strlen(g_welcomeserialcode.pCode) < 22 )
			{
				strcat ( g_welcomeserialcode.pCode, Chr(c) );
				strupr ( g_welcomeserialcode.pCode );
			}
		}
		if ( iKeyScanCode == 14 )
		{
			if ( strlen(g_welcomeserialcode.pCode) > 0 )
			{
				g_welcomeserialcode.iKeyPressed = 1;
				g_welcomeserialcode.pCode[strlen(g_welcomeserialcode.pCode)-1] = 0;
			}
		}
		if ( iKeyScanCode == 29 && c == 118 )
		{
			g_welcomeserialcode.iKeyPressed = 1;
			LPSTR pBuffer = new char[strlen((LPSTR)GetClipboard(NULL))+1];
			strcpy ( pBuffer, (LPSTR)GetClipboard(NULL) );
			pBuffer[22] = 0;
			strcpy ( g_welcomeserialcode.pCode, pBuffer );
			SAFE_DELETE(pBuffer);
		}
	}

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 && iID > 0 ) 
		{
			// check if serial code valid
			if ( common_isserialcodevalid(g_welcomeserialcode.pCode) == 1 )
			{
				// write correct serial code to VRQ file
				common_writeserialcode(g_welcomeserialcode.pCode);

				// and ensure we can pass to active app
				g.iTriggerSoftwareToQuit = 0;

				// and continue
				t.tclosequick = 1;
			}
			else
			{
				// serial code is invalid or expired
				g_welcomeserialcode.iAfterError = 2;
			}
		}
		if ( iHighlightingButton == 2 ) 
		{
			// signal GameGuru to close down immediately
			g.iTriggerSoftwareToQuit = 1;
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 3 ) 
		{
			// signal GameGuru to close down immediately
			g_welcomeserialcode.iAfterError = 1;
		}
	}
}

// WHAT YOU GET PAGE

struct welcomewhatyougettype
{
	int iNextButtonID;
	char pLines[10][1024];
};
welcomewhatyougettype g_welcomewhatyouget;

void welcome_whatyouget_init ( void )
{
	memset ( &g_welcomewhatyouget, 0, sizeof(g_welcomewhatyouget) );
	g_welcomewhatyouget.iNextButtonID = 1;

	// load text for product
	cstr productIntroPath = cstr("languagebank\\")+g.language_s+cstr("\\artwork\\welcome-assets\\product-intro.txt");
	OpenToRead ( 1, productIntroPath.Get() );
	for ( int iLine = 0; iLine < 10; iLine++ )
	{
		LPSTR pLineData = ReadString ( 1 );
		strcpy ( g_welcomewhatyouget.pLines[iLine], pLineData );
	}
	CloseFile ( 1 );
}

void welcome_whatyouget_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_drawrotatedimage ( g.editorimagesoffset+9, 50, 5, 0, 0, 0, false );
	welcome_drawbox ( 0, 10, 23, 90, 81 );
	for ( int iLine = 0; iLine < 10; iLine++ )
	{
		welcome_text ( g_welcomewhatyouget.pLines[iLine], 1, 50, 28+(iLine*5), 192, true, false );
	}
	iID = g_welcomewhatyouget.iNextButtonID; welcome_textinbox ( iID, "NEXT", 1, 50, 90, g_welcomebutton[iID].alpha );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) t.tclosequick = 1;
	}
}

// CHANGE LOG PAGE

struct welcomechangelogtype
{
	int iNextButtonID;
	char pLog[102400];
};
welcomechangelogtype g_welcomechangelog;

void welcome_changelog_init ( void )
{
	memset ( &g_welcomechangelog, 0, sizeof(g_welcomechangelog) );
	g_welcomechangelog.iNextButtonID = 1;

	// create a change log from anything newly added to GameGuru
	strcpy ( g_welcomechangelog.pLog, "V 2017.09 DX11 BETA 30\n" );
	strcat ( g_welcomechangelog.pLog, "* Tweaked Escape level to stop rocket man from messing up LOD animation\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Tweaked Escape level to stop rocket man from messing\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Tweaked Escape level to stop rocket man from messing\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Tweaked Escape level to stop rocket man from messing\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
	strcat ( g_welcomechangelog.pLog, "* Added generation of global environment map from sky/floor for PBR effect\n" );
}

void welcome_changelog_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "LATEST CHANGE LOG", 5, 50, 10, 255, false, false );
	welcome_drawbox ( 0, 10, 20, 90, 65 );
	welcome_text ( "Take a look at all the changes made since the last time you launched\nGameGuru. Use the mouse wheel to scroll through them.", 1, 50, 72, 192, true, false );
	welcome_text ( g_welcomechangelog.pLog, 2, 13, 25, 192, true, true );
	iID = g_welcomechangelog.iNextButtonID; welcome_textinbox ( iID, "NEXT", 1, 50, 90, g_welcomebutton[iID].alpha );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) t.tclosequick = 1;
	}
}

// MAIN PAGE

struct welcomemaintype
{
	int iTickToHide;
};
welcomemaintype g_welcomemain;

void welcome_main_init ( void )
{
	memset ( &g_welcomemain, 0, sizeof(g_welcomemain) );
	g_welcomemain.iTickToHide = 1;
}

void welcome_main_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "WELCOME TO GAMEGURU", 5, 50, 10, 255, false, false );
	welcome_text ( "Would you like to learn more about GameGuru, load a sample level to play\nor go directly to the editor and start creating a 3D game?", 1, 50, 72, 192, true, false );
	iID = 4; welcome_drawbox ( iID, 10, 90, 11.5f, 92 );
	if ( g.gshowonstartup != 0 ) welcome_drawrotatedimage ( g.editorimagesoffset+40, 10.75f, 88.5f, 0, 0, 0, false );
	welcome_text ( "Tick to skip welcome dialog in future", 1, 13.5f, 91.0f, 255, false, true );
	welcome_drawrotatedimage ( g.editorimagesoffset+56, 22.5f, 22, 0, 0, 0, false );
	welcome_drawrotatedimage ( g.editorimagesoffset+57, 50.0f, 22, 0, 0, 0, false );
	welcome_drawrotatedimage ( g.editorimagesoffset+58, 77.5f, 22, 0, 0, 0, false );
	iID = 1; welcome_textinbox ( iID, "LEARN", 1, 22.5f, 60, g_welcomebutton[iID].alpha );
	iID = 2; welcome_textinbox ( iID, "PLAY", 1, 50, 60, g_welcomebutton[iID].alpha );
	iID = 3; welcome_textinbox ( iID, "CREATE", 1, 77.5f, 60, g_welcomebutton[iID].alpha );

	// control page
	if ( t.inputsys.mclick == 1 && t.inputsys.mclickreleasestate == 0 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			// read the PDF manual
			cstr pPDFPath = g.fpscrootdir_s + "\\Files\\languagebank\\english\\artwork\\GameGuru - Getting Started Guide.pdf";
			ExecuteFile ( pPDFPath.Get(), "" , "", 0 );
			welcome_waitfornoinput();
		}
		if ( iHighlightingButton == 2 ) 
		{
			// jump into play page
			welcome_waitfornoinput();
			welcome_setuppage ( WELCOME_PLAY );
			welcome_runloop ( WELCOME_PLAY );
			welcome_waitfornoinput();
			welcome_setuppage ( WELCOME_MAIN );
			welcome_staticbackdrop();
			iHighlightingButton = 0;
			PasteImage ( g.editorimagesoffset+12, 0, 0 );
		}
		if ( iHighlightingButton == 3 ) 
		{
			// go to editor in create mode
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 4 ) 
		{
			// toggle startup flag
			t.inputsys.mclick = 0;
			t.inputsys.mclickreleasestate = 1;
			g.gshowonstartup = 1 - g.gshowonstartup;

			// save setting
			t.tfile_s = g.fpscrootdir_s+"\\showonstartup.ini";
			DeleteAFile ( t.tfile_s.Get() );
			if ( FileOpen(1) ==  1 ) CloseFile ( 1 );
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, cstr(g.gshowonstartup).Get() );
			WriteString ( 1, "10" );
			for ( int n = 0; n < 10; n++ )
			{
				WriteString ( 1, "0" );
			}
			CloseFile (  1 );
		}
	}
}

// PLAY PAGE

struct welcomeplaytype
{
	int iBackButtonID;
};
welcomeplaytype g_welcomeplay;

void welcome_play_init ( void )
{
	memset ( &g_welcomeplay, 0, sizeof(g_welcomeplay) );
	g_welcomeplay.iBackButtonID = 1;
}

int welcome_play_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "PLAY A SAMPLE LEVEL", 5, 50, 10, 255, false, false );
	//welcome_drawbox ( 0, 50-12.0f, 20, 50+12.0f, 65 );
	//welcome_drawrotatedimage ( g.editorimagesoffset+59, 50, 22, 0, 0, 0, false );
	//welcome_text ( "THE BIG ESCAPE", 2, 50, 50, 255, false, false );
	welcome_drawrotatedimage ( g.editorimagesoffset+59, 50, 20, 0, 0, 0, false );
	welcome_text ( "THE BIG ESCAPE", 2, 50, 18, 255, false, false );
	iID = 2; welcome_textinbox ( iID, "LOAD", 1, 50, 60, g_welcomebutton[iID].alpha );
	welcome_text ( "This sample level has been created to demonstrate the type\nof game levels you can create using the stock assets that come with GameGuru.", 1, 50, 72, 192, true, false );
	iID = g_welcomeplay.iBackButtonID; welcome_textinbox ( iID, "BACK", 1, 50, 90, g_welcomebutton[iID].alpha );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			// back
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 2 )
		{
			// load a game level
			t.tlevelautoload_s="The Big Escape.fpm";
			//t.tlevelautoload_s=g.fpscrootdir_s+"\\Files\\mapbank\\"+t.tlevelautoload_s;
			t.tlevelautoload_s=g.mysystem.mapbankAbs_s+t.tlevelautoload_s;

			// force immediate quit of welcome system
			return 1;
		}
	}

	// normal continue
	return 0;
}

void welcome_exitapp_init ( void )
{
}

void welcome_exitapp_page ( int iHighlightingButton )
{
	// draw page
	welcome_drawbox ( 0, 10, 23, 90, 81 );
	if ( g.iTriggerSoftwareToQuit == 2 )
	{
		welcome_text ( "Steam not running or free weekend is over", 1, 50, 2+(2*5), 192, true, false );
		if ( g_welcomeCycle >=0 )
		{
			g_welcomeCycle++; if ( g_welcomeCycle > 50 ) g_welcomeCycle = 0;
		}
		if ( g_welcomeCycle > 25 )
			welcome_drawimage ( g.editorimagesoffset+60, 50, 30, false );
		else
			welcome_drawimage ( g.editorimagesoffset+63, 50, 30, false );
		welcome_text ( "Use the FILE > Exit function or Close Button to exit app", 1, 50, 15+(15*5), 192, true, false );
		welcome_textinbox ( 1, "GET YOUR VERY OWN VERSION HERE", 3, 50, 66, -1 );
	}
	if ( g.iTriggerSoftwareToQuit == 3 )
	{
		welcome_text ( "All DirectX 11 Shaders Updated", 1, 50, 28+(2*5), 192, true, false );
		welcome_text ( "You may change 'forceloadtestgameshaders' back to zero", 1, 50, 28+(3*5), 192, true, false );
		welcome_text ( "Use the FILE > Exit function or Close Button to exit app", 1, 50, 28+(6*5), 192, true, false );
	}

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 && g_welcomeCycle >= 0 ) 
		{
			// go to Steam's GameGuru Page
			ExecuteFile ( "https://store.steampowered.com/app/266310/GameGuru/","","",0 );
			g_welcomeCycle = -1;
		}
	}
}

void welcome_freeintroapp_init ( void )
{
	g_welcomeCycle = 0;
}

void welcome_freeintroapp_page ( int iHighlightingButton )
{
	// draw page
	welcome_drawimage ( g.editorimagesoffset+62, 50, 30, false );
	welcome_text ( "Steam Free Weekend Version", 5, 50, 15, 192, true, false );
	welcome_textinbox ( 1, "CONTINUE CONTINUE", 3, 50, 69-10, -1 );
	welcome_textinbox ( 2, "GET YOUR OWN VERZ", 3, 50, 69, -1 );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 2 && g_welcomeCycle == 0 ) 
		{
			// go to Steam's GameGuru Page
			ExecuteFile ( "https://store.steampowered.com/app/266310/GameGuru/","","",0 );
			g_welcomeCycle = -1;
			t.tclosequick = 1;
		}
	}
}

void welcome_freetrialexitapp_init ( void )
{
}

void welcome_freetrialexitapp_page ( int iHighlightingButton )
{
	// draw page
	welcome_drawbox ( 0, 10, 23, 90, 81 );
	if ( g.iTriggerSoftwareToQuit == 2 )
	{
		welcome_text ( "Free Trial Has Expired", 1, 50, 2+(2*5), 192, true, false );
		if ( g_welcomeCycle >=0 )
		{
			g_welcomeCycle++; if ( g_welcomeCycle > 50 ) g_welcomeCycle = 0;
		}
		if ( g_welcomeCycle > 25 )
			welcome_drawimage ( g.editorimagesoffset+60, 50, 30, false );
		else
			welcome_drawimage ( g.editorimagesoffset+63, 50, 30, false );
		welcome_text ( "Use the FILE > Exit function or Close Button to exit app", 1, 50, 15+(15*5), 192, true, false );
		welcome_textinbox ( 1, "GET YOUR VERY OWN VERSION HERE", 3, 50, 66, -1 );
	}
	if ( g.iTriggerSoftwareToQuit == 3 )
	{
		welcome_text ( "All DirectX 11 Shaders Updated", 1, 50, 28+(2*5), 192, true, false );
		welcome_text ( "You may change 'forceloadtestgameshaders' back to zero", 1, 50, 28+(3*5), 192, true, false );
		welcome_text ( "Use the FILE > Exit function or Close Button to exit app", 1, 50, 28+(6*5), 192, true, false );
	}

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 && g_welcomeCycle >= 0 ) 
		{
			// go to GameGuru Page
			ExecuteFile ( "http://bit.ly/2M6GfX8","","",0 );
			g_welcomeCycle = -1;
		}
	}
}

void welcome_freetrialintroapp_init ( void )
{
	g_welcomeCycle = 0;
}

void welcome_freetrialintroapp_page ( int iHighlightingButton )
{
	// draw page
	welcome_drawimage ( g.editorimagesoffset+62, 50, 30, false );
	welcome_textinbox ( 1, "CONTINUE CONTINUE", 3, 50, 69-10, -1 );
	welcome_textinbox ( 2, "GET YOUR OWN VERZ", 3, 50, 69, -1 );

	// days left
	char pShowDaysLeft[1024];
	if ( g_trialStampDaysLeft == 1 )
		sprintf ( pShowDaysLeft, "Last Day of Free Trial", g_trialStampDaysLeft );
	else
		sprintf ( pShowDaysLeft, "%d Days Left of Free Trial", g_trialStampDaysLeft );
	welcome_text ( pShowDaysLeft, 5, 50, 15, 192, true, false );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 2 && g_welcomeCycle == 0 ) 
		{
			// go to GameGuru Page
			ExecuteFile ( "http://bit.ly/2M6GfX8","","",0 );
			g_welcomeCycle = -1;
			t.tclosequick = 1;
		}
	}
}

void CleanStringOfEscapeSlashes ( char* pText )
{
	char *str = pText;
	char *str2 = pText;
	while ( *str )
	{
		if ( *str == '"' )
		{
			str++;
		}
		else
		{
			if ( *str == '\\' )
			{
				str++;
				switch( *str )
				{
					case 'n': *str2 = '\n'; break;
					case 'r': *str2 = '\r'; break;
					case '"': *str2 = '"'; break;
					case 'b': *str2 = '\b'; break;
					case 'f': *str2 = '\f'; break;
					case 't': *str2 = '\t'; break;
					case '/': *str2 = '/'; break;
					case '\\': *str2 = '\\'; break;
					default: *str2 = *str;
				}
			}
			else
			{
				*str2 = *str;
			}
			str++;
			str2++;
		}
	}
	*str2 = 0;
}

UINT OpenURLForDataOrFile ( LPSTR pDataReturned, DWORD* pReturnDataSize, LPSTR pUniqueCode, LPSTR pVerb, LPSTR urlWhere, LPSTR pLocalFileForImageOrNews )
{
	UINT iError = 0;
	unsigned int dwDataLength = 0;
	HINTERNET m_hInet = InternetOpen( "InternetConnection", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( m_hInet == NULL )
	{
		iError = GetLastError( );
	}
	else
	{
		unsigned short wHTTPType = INTERNET_DEFAULT_HTTPS_PORT;
		HINTERNET m_hInetConnect = InternetConnect( m_hInet, "www.thegamecreators.com", wHTTPType, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		if ( m_hInetConnect == NULL )
		{
			iError = GetLastError( );
		}
		else
		{
			int m_iTimeout = 2000;
			InternetSetOption( m_hInetConnect, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)&m_iTimeout, sizeof(m_iTimeout) );  
			HINTERNET hHttpRequest = HttpOpenRequest( m_hInetConnect, pVerb, urlWhere, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0 );
			if ( hHttpRequest == NULL )
			{
				iError = GetLastError( );
			}
			else
			{
				HttpAddRequestHeaders( hHttpRequest, "Content-Type: application/x-www-form-urlencoded", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE );
				int bSendResult = 0;
				FILE* fpImageLocalFile = NULL;
				if ( pLocalFileForImageOrNews == NULL )
				{
					// News
					char m_szPostData[1024];
					strcpy ( m_szPostData, "k=vIo3sc2z" );
					#ifdef FREETRIALVERSION
					 strcat ( m_szPostData, "&app=gamegurufree" );
					#else
					 strcat ( m_szPostData, "&app=gameguru" );
					#endif
					strcat ( m_szPostData, "&uid=" );
					strcat ( m_szPostData, pUniqueCode );
					bSendResult = HttpSendRequest( hHttpRequest, NULL, -1, (void*)(m_szPostData), strlen(m_szPostData) );
				}
				else
				{
					// Image URL, open local file for writing
					bSendResult = HttpSendRequest( hHttpRequest, NULL, -1, NULL, NULL );
					fpImageLocalFile = fopen( pLocalFileForImageOrNews , "wb" );
				}
				if ( bSendResult == 0 )
				{
					iError = GetLastError( );
				}
				else
				{
					int m_iStatusCode = 0;
					char m_szContentType[150];
					unsigned int dwBufferSize = sizeof(int);
					unsigned int dwHeaderIndex = 0;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (void*)&m_iStatusCode, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int dwContentLength = 0;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (void*)&dwContentLength, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int ContentTypeLength = 150;
					HttpQueryInfo( hHttpRequest, HTTP_QUERY_CONTENT_TYPE, (void*)m_szContentType, (LPDWORD)&ContentTypeLength, (LPDWORD)&dwHeaderIndex );
					char pBuffer[ 20000 ];
					for(;;)
					{
						unsigned int written = 0;
						if( !InternetReadFile( hHttpRequest, (void*) pBuffer, 2000, (LPDWORD)&written ) )
						{
							// error
						}
						if ( written == 0 ) break;
						if ( fpImageLocalFile )
						{
							// write direct to local image file
							fwrite(pBuffer, 1, written, fpImageLocalFile );
						}
						else
						{
							// comple news for return string
							if ( dwDataLength + written > 10240 ) written = 10240 - dwDataLength;
							memcpy( pDataReturned + dwDataLength, pBuffer, written );
							dwDataLength = dwDataLength + written;
							if ( dwDataLength >= 10240 ) break;
						}
					}
					InternetCloseHandle( hHttpRequest );
				}
				if ( fpImageLocalFile )
				{
					fclose(fpImageLocalFile);
					fpImageLocalFile = NULL;
				}
			}
			InternetCloseHandle( m_hInetConnect );
		}
		InternetCloseHandle( m_hInet );
	}
	if ( iError > 0 )
	{
		char *szError = 0;
		if ( iError > 12000 && iError < 12174 ) 
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle("wininet.dll"), iError, 0, (char*)&szError, 0, 0 );
		else 
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, iError, 0, (char*)&szError, 0, 0 );
		if ( szError )
		{
			LocalFree( szError );
		}
	}

	// complete
	*pReturnDataSize = dwDataLength;
	return iError;
}

bool welcome_announcements_init ( void )
{
	// grab news from server
	bool bNewsIsAvailable = false;
	strcpy ( g_welcomeText, "" );
	strcpy ( g_welcomeImageUrl, "" );
	strcpy ( g_welcomeLinkUrl, "" );

	// generate unique code for AGK install if none available
	char pUniqueCodeFile[ 1024 ];
	strcpy( pUniqueCodeFile, g.fpscrootdir_s.Get() );
	strcat( pUniqueCodeFile, "\\installcode.dat" );
	char pUniqueCode[33];
	memset ( pUniqueCode, 33, 0 );
	FILE *file = fopen(pUniqueCodeFile, "r");
	if ( file == NULL )
	{
		// generate
		time_t mtime;
		mtime = time(0);
		srand(mtime);
		int n = 0;
		for (; n < 32; n++ ) 
		{
			pUniqueCode[n] = 65+(rand()%22);
		}
		pUniqueCode[32] = 0;
		FILE* fp = fopen( pUniqueCodeFile , "w" );
		fwrite(pUniqueCode , 1 , 32 , fp );
		fclose(fp);
	}
	else
	{
		// read
		fread(pUniqueCode, 1, 32, file);
		fclose(file);
	}
	pUniqueCode[32] = 0;

	// are we a special IDE?
	int iSpecialIDEForViewingTestAnnouncements = 0;
	char pSpecialIDETestFile[2048];
	strcpy ( pSpecialIDETestFile, g.fpscrootdir_s.Get() );
	strcat ( pSpecialIDETestFile, "\\SHOWTEST.dat" );
	FILE* showtestfile = fopen(pSpecialIDETestFile, "r");
	if ( showtestfile != NULL )
	{
		MessageBox ( NULL, "Running in IDE Announcement Test Mode", "Latest News", MB_OK );
		iSpecialIDEForViewingTestAnnouncements = 1;
		fclose(showtestfile);
	}				

	// request news from server
	char pImageLocalFile[2048];
	DWORD dwDataReturnedSize = 0;
	char pDataReturned[10240];
	memset ( pDataReturned, 0, sizeof(pDataReturned) );
	UINT iError = OpenURLForDataOrFile ( pDataReturned, &dwDataReturnedSize, pUniqueCode, "POST", "/api/app/announcement", NULL );
	if ( iError <= 0 && *pDataReturned != 0 && strchr(pDataReturned, '{') != 0 )
	{
		// break up response string
		char updated_at[1024];
		memset ( updated_at, 0, sizeof(updated_at) );
		char pNewsText[10240];
		strcpy ( pNewsText, "" );
		char pURLText[10240];
		strcpy ( pURLText, "" );
		char pImageURL[10240];
		strcpy ( pImageURL, "" );
		char pWorkStr[10240];
		strcpy ( pWorkStr, pDataReturned );
		if ( pWorkStr[0]=='{' ) strcpy ( pWorkStr, pWorkStr+1 );
		int n = 10200;
		for (; n>0; n-- ) if ( pWorkStr[n] == '}' ) { pWorkStr[n] = 0; break; }
		char* pChop = strstr ( pWorkStr, "," );
		char pStatusStr[10240];
		strcpy ( pStatusStr, pWorkStr );
		if ( pChop ) pStatusStr[pChop-pWorkStr] = 0;
		char* pStatusValue = strstr ( pStatusStr, ":" ) + 1;
		if ( pChop[0]==',' ) pChop += 1;
		if ( strstr ( pStatusValue, "success" ) != NULL )
		{
			// success
			// news
			pChop = strstr ( pChop, ":" ) + 2;
			strcpy ( pNewsText, pChop );
			char pEndOfChunk[4];
			pEndOfChunk[0]='"';
			pEndOfChunk[1]=',';
			pEndOfChunk[2]='"';
			pEndOfChunk[3]=0;
			char* pNewsTextEnd = strstr ( pNewsText, pEndOfChunk );
			pNewsText[pNewsTextEnd-pNewsText] = 0;
			pChop += strlen(pNewsText);

			// go through news and replace \n with real carriage returns
			int n = 0;
			char* pReplacePos = pNewsText;
			for(;;)
			{
				pReplacePos = strstr ( pReplacePos, "\\r\\n" );
				if ( pReplacePos != NULL )
				{
					pReplacePos[0]=' ';
					pReplacePos[1]=' ';
					pReplacePos[2]=' ';
					pReplacePos[3]='\n';
					pReplacePos+=4;
				}
				else
					break;
			}

			// url
			strcpy ( pURLText, pChop );
			pEndOfChunk[0]='"';
			pEndOfChunk[1]=',';
			pEndOfChunk[2]='"';
			strcpy ( pURLText, strstr ( pURLText, pEndOfChunk ) + 9 );
			char* pURLEnd = strstr ( pURLText, pEndOfChunk );
			pURLText[pURLEnd-pURLText] = 0;
			pChop += strlen(pURLText) + 9;
			CleanStringOfEscapeSlashes ( pURLText );

			// image_url
			pChop = strstr ( pChop, "image_url" );
			pChop += 11; // skips past image_url":
			LPSTR pEndOfImageURL = strstr ( pChop, ",\"test" );
			DWORD dwLength = pEndOfImageURL-pChop;
			memcpy ( pImageURL, pChop, dwLength );
			pImageURL[dwLength] = 0;
			CleanStringOfEscapeSlashes ( pImageURL );

			// test flag
			int iTestAnnouncement = 0;
			pChop = strstr ( pChop, ",\"test\":" );
			pChop += 8; // get past ,"test":
			if ( *pChop == '0' )
			{
				iTestAnnouncement = 0;
			}
			else
			{
				iTestAnnouncement = 1;
			}

			// updated_at
			char pUpdatedAt[10240];
			pEndOfChunk[0]='"';
			pEndOfChunk[1]=':';
			pEndOfChunk[2]='{';
			pChop = strstr ( pChop, pEndOfChunk ) + 2 + 9;
			strcpy ( pUpdatedAt, pChop );
			memcpy ( updated_at, pUpdatedAt, 19 );
			updated_at[19] = 0;

			// show what_notifications dialog if news available
			char install_stamp_at[1024];
			memset ( install_stamp_at, 0, sizeof(install_stamp_at) );

			// Image Handling
			strcpy ( pImageLocalFile, g.fpscrootdir_s.Get() );
			strcat ( pImageLocalFile, "\\languagebank\\english\\artwork\\welcome-assets\\gameguru-news-banner.png" );

			// so we download an image
			char pNoDomainPart[1024];
			strcpy ( pNoDomainPart, "" );
			if ( strcmp ( pImageURL, "null" ) != NULL )
			{
				// get filename only
				strcpy ( pNoDomainPart, pImageURL + strlen("https://www.thegamecreators.com") );
				//strrev ( pNoDomainPart );
				//pNoDomainPart[strlen("https://www.thegamecreators.com//")] = 0;
				//strrev ( pNoDomainPart );

				// get file ext
				char pFileExt[1024];
				strcpy ( pFileExt, pNoDomainPart + strlen(pNoDomainPart) - 4 );

				// Download the image file
				DWORD dwImageReturnedSize = 0;
				char pImageReturned[10240];
				sprintf ( pImageLocalFile, "%s\\localimagefile%s", g.fpscrootdir_s.Get(), pFileExt );				
				UINT iImageError = OpenURLForDataOrFile ( pImageReturned, &dwImageReturnedSize, "", "GET", pNoDomainPart, pImageLocalFile );
				if ( iImageError == 0 )
				{
					// load local image file soon (below)
				}
				else
				{
					// if image not downloaded for some reason, revert to default
					strcpy ( pImageLocalFile, g.fpscrootdir_s.Get() );
					strcat ( pImageLocalFile, "\\languagebank\\english\\artwork\\welcome-assets\\gameguru-news-banner.png" );
				}
			}

			// real announcement or test announcement
			if ( iSpecialIDEForViewingTestAnnouncements == 1 )
			{
				// show the test announcement
				bNewsIsAvailable = true;
				strcpy ( g_welcomeText, pNewsText );
				strcpy ( g_welcomeLinkUrl, pURLText );
				strcpy ( g_welcomeImageUrl, pImageLocalFile );
			}
			if ( iTestAnnouncement == 0 && iSpecialIDEForViewingTestAnnouncements == 0 )
			{
				char pInstallStampFile[ 1024 ];
				strcpy( pInstallStampFile, g.fpscrootdir_s.Get() );
				strcat( pInstallStampFile, "\\installstamp.dat" );
				file = fopen(pInstallStampFile, "r");
				if ( file != NULL )
				{
					fread(install_stamp_at, 1, 19, file);
					install_stamp_at[ 19 ] = 0;
					fclose(file);
				}
				if ( strcmp ( updated_at, install_stamp_at ) != NULL )
				{
					// different updated_at entry, show new news
					bNewsIsAvailable = true;
					strcpy ( g_welcomeText, pNewsText );
					strcpy ( g_welcomeLinkUrl, pURLText );
					strcpy ( g_welcomeImageUrl, pImageLocalFile );

					// update install stamp so we know news has been read
					FILE* fp = fopen( pInstallStampFile , "w" );
					fwrite(updated_at , 1 , 19 , fp );
					fclose(fp);
				}
			}
		}
		else
		{
			// error
			char* pMessageValue = strstr ( pChop, ":" ) + 1;
		}
	}

	// if no news, skip the welcome dialog
	if ( bNewsIsAvailable == true )
	{
		// overwrite default with announcement image
		if ( FileExist ( g_welcomeImageUrl ) == 1 ) LoadImage ( g_welcomeImageUrl, g.editorimagesoffset+64 );
		if ( ImageExist ( g.editorimagesoffset+64 ) == 0 ) 
		{
			// sometimes the image is download corrupt!
			strcpy ( g_welcomeImageUrl, g.fpscrootdir_s.Get() );
			strcat ( g_welcomeImageUrl, "\\languagebank\\english\\artwork\\welcome-assets\\gameguru-news-banner.png" );
			LoadImage ( g_welcomeImageUrl, g.editorimagesoffset+64 );
		}
		g_welcomeCycle = 0; if ( strlen(g_welcomeLinkUrl) > 0 ) g_welcomeCycle = 1;		
	}

	// false will skip the dialog altogether
	return bNewsIsAvailable;
}

void welcome_announcements_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "Latest News", 5, 50, 10, 192, true, false );
	welcome_drawrotatedimage ( g.editorimagesoffset+64, 50, 20, 0, 0, 0, false );

	int iCharCount = 0;
	int iLineIndex = 1;
	char pTextToShow[1024];
	strcpy ( pTextToShow, g_welcomeText );
	char pLine1[1024]; strcpy ( pLine1, "" );
	char pLine2[1024]; strcpy ( pLine2, "" );
	char pLine3[1024]; strcpy ( pLine3, "" );
	char pWorkLine[1024]; strcpy ( pWorkLine, "" );
	for ( int n = 0; n < strlen(pTextToShow); n++ )
	{
		iCharCount++;
		if ( iCharCount > 65 && (pTextToShow[n] == ' ' || pTextToShow[n] == ',' || pTextToShow[n] == '.' || pTextToShow[n] == '"') )
		{
			if ( iLineIndex == 1 ) { strcpy ( pLine1, pTextToShow ); pLine1[n+1] = 0; }
			if ( iLineIndex == 2 ) { strcpy ( pLine2, pTextToShow ); pLine2[n+1] = 0; }
			strcpy ( pWorkLine, pTextToShow );
			strcpy ( pTextToShow, pWorkLine+n+1 ); n = 0;
			iCharCount = 0;
			iLineIndex++;
			if ( iLineIndex == 3 ) break;
		}
	}
	strcpy ( pLine3, pTextToShow );
	welcome_text ( pLine1, 1, 50, 58, 192, true, false );
	welcome_text ( pLine2, 1, 50, 63, 192, true, false );
	welcome_text ( pLine3, 1, 50, 68, 192, true, false );
	if ( g_welcomeCycle == 0 ) 
	{ 
		iID = 1; welcome_textinbox ( iID, "EXIT", 1, 50, 80, g_welcomebutton[iID].alpha );
	}
	else
	{
		iID = 1; welcome_textinbox ( iID, "EXIT", 1, 100-35, 80, g_welcomebutton[iID].alpha );
		iID = 2; welcome_textinbox ( iID, "GOTO LINK", 1, 35, 80, g_welcomebutton[iID].alpha ); 
	}
	//iID = 3; welcome_drawbox ( iID, 10, 90, 11.5f, 92 );
	//if ( g.gshowannouncements == 0 ) welcome_drawrotatedimage ( g.editorimagesoffset+40, 10.75f, 88.5f, 0, 0, 0, false );
	//welcome_text ( "Tick to skip news dialog in future", 1, 13.5f, 91.0f, 255, false, true );

	// control page
	if ( t.inputsys.mclick == 1 && t.inputsys.mclickreleasestate == 0 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			t.tclosequick = 1;
		}
		if ( iHighlightingButton == 2 ) 
		{
			// jump to announcement link if there is one
			ExecuteFile ( g_welcomeLinkUrl,"","",0 );
			t.tclosequick = 1;
		}
		/*
		if ( iHighlightingButton == 3 ) 
		{
			// toggle flag
			t.inputsys.mclick = 0;
			t.inputsys.mclickreleasestate = 1;
			g.gshowannouncements = 1 - g.gshowannouncements;

			// save setting
			t.tfile_s = g.fpscrootdir_s+"\\showannouncements.ini";
			DeleteAFile ( t.tfile_s.Get() );
			if ( FileOpen(1) == 1 ) CloseFile ( 1 );
			OpenToWrite ( 1, t.tfile_s.Get() );
			WriteString ( 1, cstr(g.gshowannouncements).Get() );
			CloseFile ( 1 );
		}
		*/
	}
}

void welcome_savestandalone_init ( void )
{
	g_welcomeCycle = 0;
}

void welcome_savestandalone_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "Save Standalone", 5, 50, 15, 192, true, false );
	if ( g_welcomeCycle == 0 )
	{
		welcome_text ( "Choose where you would like your standalone to be saved", 1, 50, 30, 192, true, false );
		welcome_drawbox ( 0, 10, 37, 74, 43 );
		welcome_text ( g.exedir_s.Get(), 1, 42, 40, 192, true, false );
		iID = 3; welcome_textinbox ( iID, "CHANGE", 1, 85, 40, g_welcomebutton[iID].alpha );
		// save standalone button
		iID = 2; welcome_textinbox ( iID, "SAVE STANDALONE", 1, 50, 65, g_welcomebutton[iID].alpha );
		iID = 1; welcome_textinbox ( iID, "EXIT", 1, 50, 85, g_welcomebutton[iID].alpha );
	}
	else
	{
		welcome_text ( "Standalone Folder", 1, 50, 30, 192, true, false );
		welcome_drawbox ( 0, 10, 37, 90, 43 );
		welcome_text ( g.exedir_s.Get(), 1, 50, 40, 192, true, false );
		// show standalone progress
		welcome_text ( "Exporting your game as a standalone executable and encrypting media files", 1, 50, 60, 192, true, false );
		welcome_drawbox ( 0, 10, 65, 90, 70 );
		int iPercentageComplete = (int)mapfile_savestandalone_getprogress();
		char pProgressString[1024];
		char percentsymbol[2];
		percentsymbol[0] = 37;
		percentsymbol[1] = 0;
		sprintf ( pProgressString, "Progress Complete : %d%s", iPercentageComplete, percentsymbol );
		welcome_text ( pProgressString, 1, 50, 67.5f, 192, true, false );
		if ( iPercentageComplete > 0 && iPercentageComplete < 100 )
		{
			// only offer CANCEL button if in progress
			iID = 1; welcome_textinbox ( iID, "CANCEL", 1, 50, 85, g_welcomebutton[iID].alpha );
		}
	}

	// control standalone saving
	if ( g_welcomeCycle == 2 )
	{
		// start save standalone creation
		mapfile_savestandalone_start();
		g_welcomeCycle = 3;
	}
	if ( g_welcomeCycle == 3 )
	{
		// run standalone creation calls
		if ( mapfile_savestandalone_continue() == 1 )
		{
			// complete standalone creation
			g_welcomeCycle = 4;
		}
	}
	if ( g_welcomeCycle == 4 )
	{
		// complete standalone creation
		mapfile_savestandalone_finish();
		g_welcomeCycle = 0;
		t.tclosequick = 1;
	}
	if ( g_welcomeCycle == 5 )
	{
		// cancel standalone creation
		mapfile_savestandalone_restoreandclose();
		g_welcomeCycle = 0;
		t.tclosequick = 1;
	}
	if ( g_welcomeCycle == 1 ) g_welcomeCycle = 2;

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 ) 
		{
			if ( g_welcomeCycle == 0 )
			{
				// exit
				t.tclosequick = 1;
			}
			else
			{
				// force cancel stage
				if ( g_welcomeCycle == 3 ) g_welcomeCycle = 5;
			}
		}
		if ( iHighlightingButton == 2 ) 
		{
			// save standalone start
			g_welcomeCycle = 1;
		}
		if ( iHighlightingButton == 3 ) 
		{
			// change location of save standalone game
			OpenFileMap ( 1,"FPSEXCHANGE" );
			SetFileMapString ( 1, 1000, g.exedir_s.Get() );
			SetFileMapDWORD ( 1, 424, 2 );
			SetEventAndWait ( 1 );
			while ( GetFileMapDWORD(1, 424) == 2 ) 
			{
				SetEventAndWait ( 1 );
			}
			t.returnstring_s = GetFileMapString(1, 1000);
			if ( strlen(t.returnstring_s.Get()) > 0 )
				g.exedir_s = t.returnstring_s;
		}
	}
}

// WORKER FUNCTIONS

bool welcome_setuppage ( int iPageIndex )
{
	// clear data vars
	memset ( g_welcomebutton, 0, sizeof(g_welcomebutton) );
	g_welcomebuttoncount = 0;

	// clear page specific globals
	if ( iPageIndex == WELCOME_SERIALCODE ) welcome_serialcode_init();
	if ( iPageIndex == WELCOME_WHATYOUGET ) welcome_whatyouget_init();
	if ( iPageIndex == WELCOME_CHANGELOG ) welcome_changelog_init();
	if ( iPageIndex == WELCOME_MAIN ) welcome_main_init();
	if ( iPageIndex == WELCOME_PLAY ) welcome_play_init();
	if ( iPageIndex == WELCOME_EXITAPP ) welcome_exitapp_init();
	if ( iPageIndex == WELCOME_FREEINTROAPP ) welcome_freeintroapp_init();
	if ( iPageIndex == WELCOME_ANNOUNCEMENTS ) return welcome_announcements_init();
	if ( iPageIndex == WELCOME_SAVESTANDALONE ) welcome_savestandalone_init();
	if ( iPageIndex == WELCOME_FREETRIALINTROAPP ) welcome_freetrialintroapp_init();
	if ( iPageIndex == WELCOME_FREETRIALEXITAPP ) welcome_freetrialexitapp_init();

	// normally success
	return true;
}

void welcome_runloop ( int iPageIndex )
{
	// get actions through filemapping system
	OpenFileMap ( 1, "FPSEXCHANGE" );
	SetEventAndWait ( 1 );

	// run loop when in welcome page
	t.tclicked=0 ; t.tclosequick=0;
	t.lastmousex=MouseX() ; t.lastmousey=MouseY();
	t.inputsys.mclickreleasestate = 0;
	bool bStayInsideLoop = true;
	while ( bStayInsideLoop == true ) 
	{
		// exit conditions
		if ( iPageIndex == WELCOME_EXITAPP ||  iPageIndex == WELCOME_FREETRIALEXITAPP )
		{
			// can never leave this loop!
		}
		else
		{
			if ( EscapeKey() == 0 && t.tclosequick == 0 && g_welcomesystemclosedown == 0 )
			{
				// normal running
			}
			else
			{
				// leave loop, which was escaped or triggered
				bStayInsideLoop = false;
			}
		}

		// must be in IDE focus to ensure stuff not clicked or run in background
		DWORD dwForegroundFocusForIDE = GetFileMapDWORD( 1, 596 );
		if ( dwForegroundFocusForIDE == 10 )
		{
			t.inputsys.kscancode=ScanCode();
			t.inputsys.mclick=MouseClick();
			if ( t.inputsys.mclick == 0 ) t.inputsys.mclickreleasestate = 0;
			if ( t.inputsys.ignoreeditorintermination == 0 )
			{
				if ( GetFileMapDWORD( 1, 908 ) == 1 )  break;
			}
			if ( GetFileMapDWORD( 1, 516 ) > 0 )  break;
			if ( GetFileMapDWORD( 1, 400 ) == 1 ) { t.interactive.active = 0  ; break; }
			if ( GetFileMapDWORD( 1, 404 ) == 1 ) { t.interactive.active = 0  ; break; }
			if ( GetFileMapDWORD( 1, 408 ) == 1 ) { t.interactive.active = 0  ; break; }
			if ( GetFileMapDWORD( 1, 434 ) == 1 ) { t.interactive.active = 0  ; break; }
			if ( GetFileMapDWORD( 1, 762 ) != 0 ) { t.interactive.active = 0  ; break; }
			t.terrain.gameplaycamera=0;
			terrain_shadowupdate ( );
			terrain_update ( );

			// paste backdrop
			PasteImage ( g.editorimagesoffset+12, 0, 0 );

			// paste page panel
			PasteImage ( g.editorimagesoffset+8, g_welcome.iTopLeftX, g_welcome.iTopLeftY );

			// get mouse coordinate for control
			t.inputsys.xmouse = ((GetFileMapDWORD( 1, 0 )+0.0)/800.0)*GetChildWindowWidth(0);
			t.inputsys.ymouse = ((GetFileMapDWORD( 1, 4 )+0.0)/600.0)*GetChildWindowHeight(0);

			// highlight hover over a button
			int iHighlightingButton = -1;
			for ( int iButton = 1; iButton <= g_welcomebuttoncount; iButton++ )
			{
				g_welcomebutton[iButton].alpha = 128;
				if ( t.inputsys.xmouse >= g_welcomebutton[iButton].x1 && t.inputsys.xmouse <= g_welcomebutton[iButton].x2 )
				{
					if ( t.inputsys.ymouse >= g_welcomebutton[iButton].y1 && t.inputsys.ymouse <= g_welcomebutton[iButton].y2 )
					{
						iHighlightingButton = iButton;
						g_welcomebutton[iHighlightingButton].alpha = 255;
					}
				}
			}

			// display correct page
			if ( iPageIndex == WELCOME_SERIALCODE ) welcome_serialcode_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_WHATYOUGET ) welcome_whatyouget_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_CHANGELOG ) welcome_changelog_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_MAIN ) welcome_main_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_PLAY ) g_welcomesystemclosedown = welcome_play_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_EXITAPP ) welcome_exitapp_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_FREEINTROAPP ) welcome_freeintroapp_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_ANNOUNCEMENTS ) welcome_announcements_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_SAVESTANDALONE ) welcome_savestandalone_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_FREETRIALINTROAPP ) welcome_freetrialintroapp_page ( iHighlightingButton );
			if ( iPageIndex == WELCOME_FREETRIALEXITAPP ) welcome_freetrialexitapp_page ( iHighlightingButton );

			// update screen
			Sync();
		}
	}
	t.tclosequick = 0;

	// wait until all input ends
	welcome_waitfornoinput();
}

void welcome_show ( int iPageIndex )
{
	if ( welcome_setuppage ( iPageIndex ) == true )
	{
		welcome_runloop ( iPageIndex );
	}
}

/*
// must be in IDE focus to ensure stuff not clicked or run in background
DWORD dwForegroundFocusForIDE = GetFileMapDWORD( 1, 596 );
if ( dwForegroundFocusForIDE == 10 )
{
	// 161115 - if click outside of dialog, also close!
	if ( t.inputsys.mclick == 1 )
	{
		if ( iScrMouseX < t.tcenterx || iScrMouseX > t.tcenterx+920 )
		{
			if ( iScrMouseY < t.tcentery || iScrMouseY > t.tcentery+590 )
			{
				t.tclosequick=1;
				t.tclicked=1;
			}
		}
	}

	// if click, play the animation
	if ( (t.inputsys.mclick == 1 && iHighlightedIndex > 0) || iAutoTriggerVideo != 0 ) 
	{
		// release mouse now
		while ( MouseClick()!=0 ) { Sync(); }

		// video menu or game menu
		if ( g.quickstartmenumode==0 )
		{
			// auto trigger is good to hit the user with something cool at the very start
			if ( iAutoTriggerVideo !=0 )
			{
				iHighlightedIndex = iAutoTriggerVideo;
				iAutoTriggerVideo = 0;
			}

			// go full screen
			//SetFileMapDWORD (  1, 970, 2 );
			//SetEventAndWait (  1 );

			// play this animation now
			#ifndef _DEBUG
			cStr pAnimfilename = cStr("");
			pAnimfilename = cStr("languagebank\\")+g.language_s+"\\artwork\\videos\\"+cStr(iHighlightedIndex)+".wmv";
			LoadAnimation ( pAnimfilename.Get(), 1 );
			PlayAnimation ( 1, 0, 0, GetChildWindowWidth(0), GetChildWindowHeight(0) );
			DWORD dwAnimStartedTime = timeGetTime();
			int iWatchedMarkerInSeconds = GetAnimationLength(1) * 0.8f;
			bool bExitPlayback = false;
			while ( AnimationPlaying(1) && bExitPlayback==false )
			{
				if ( (timeGetTime()-dwAnimStartedTime)/1000 > (DWORD)iWatchedMarkerInSeconds || iHighlightedIndex == 1 )
				{
					// watched this video for more than 80% (except first one)
					// mark as watched
					g.videoMenuPlayed[iHighlightedIndex-1] = 1;
				}
				if ( (timeGetTime()-dwAnimStartedTime)/1000 > 2 )
				{
					// can click out after X seconds
					if ( MouseClick()!=0 ) bExitPlayback = true;
				}
				Sync();
			}
			while ( MouseClick()!=0 ) { Sync(); }
			DeleteAnimation ( 1 );
			#endif

			// restore editor within IDE
			//SetFileMapDWORD (  1, 970, 1 );
			//SetEventAndWait (  1 );
		}
		else
		{
			// load a game level in
			t.tlevelautoload_s="";
			switch ( iHighlightedIndex ) 
			{
				case 1 : t.tlevelautoload_s = "The Big Escape.fpm" ; break ;
				case 2 : t.tlevelautoload_s = "Cartoon Antics.fpm" ; break ;
				case 3 : t.tlevelautoload_s = "Gem World.fpm" ; break ;
				case 4 : t.tlevelautoload_s = "The Heirs Revenge.fpm" ; break ;
				case 5 : t.tlevelautoload_s = "Morning Mountain Stroll.fpm" ; break ;
				case 6 : t.tlevelautoload_s = "The Asylum.fpm" ; break ;
				case 7 : t.tlevelautoload_s = "The Warehouse.fpm" ; break ;
				case 8 : t.tlevelautoload_s = "Bridge Battle (MP).fpm" ; break ;
				case 9 : t.tlevelautoload_s = "Camp Oasis (MP).fpm" ; break ;
				case 10 : t.tlevelautoload_s = "Devils Hill (MP).fpm" ; break ;
			}
			t.tlevelautoload_s=g.fpscrootdir_s+"\\Files\\mapbank\\"+t.tlevelautoload_s;

			//  ask to save first if modified project open
			t.editorcanceltask=0;
			if (  g.projectmodified == 1 ) 
			{
				//  If project modified, ask if want to save first
				gridedit_intercept_savefirst ( );
			}
			if (  t.editorcanceltask == 0 ) 
			{
				if (  t.tlevelautoload_s != "" ) 
				{
					if (  cstr(Lower(Right(t.tlevelautoload_s.Get(),4))) == ".fpm" ) 
					{
						g.projectfilename_s=t.tlevelautoload_s;
						gridedit_load_map ( );
						g.showtestlevelclickprompt = timeGetTime() + 5000;
						t.tclosequick=1;
						t.tclicked=1;
					}
				}
			}
		}
	}
}

t.tquickhighlight=0;
if (  t.inputsys.xmouse>t.tcenterx+36 && t.inputsys.xmouse<t.tcenterx+68 && t.inputsys.ymouse>t.tcentery+519 && t.inputsys.ymouse<t.tcentery+553 ) 
{
	//  toggle show on startup
	t.tquickhighlight=1;
}
if (  t.inputsys.xmouse>t.tcenterx+785 && t.inputsys.xmouse<t.tcenterx+890 && t.inputsys.ymouse>t.tcentery+506 && t.inputsys.ymouse<t.tcentery+561 ) 
{
	//  close
	t.tquickhighlight=3;
}
if (  t.inputsys.xmouse>t.tcenterx+860 && t.inputsys.xmouse<t.tcenterx+888 && t.inputsys.ymouse>t.tcentery+39 && t.inputsys.ymouse<t.tcentery+67 ) 
{
	//  exit cross
	t.tquickhighlight=4;
}
if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+558 && t.inputsys.xmouse<t.tcenterx+779 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
{
	//  play game menu
	t.tquickhighlight=5;
}
if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+350 && t.inputsys.xmouse<t.tcenterx+548 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
{
	//  twitch videos
	t.tquickhighlight=6;
}
if ( g.quickstartmenumode==0 && (t.inputsys.xmouse>t.tcenterx+223 && t.inputsys.xmouse<t.tcenterx+340 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
{
	// play PDF
	t.tquickhighlight=7;
}
if ( g.quickstartmenumode==1 && (t.inputsys.xmouse>t.tcenterx+583 && t.inputsys.xmouse<t.tcenterx+759 && t.inputsys.ymouse>t.tcentery+509 && t.inputsys.ymouse<t.tcentery+558) ) 
{
	//  play video menu
	t.tquickhighlight=5;
}
if (  t.inputsys.mclick == 1 ) 
{
	if (  t.tclicked == 0 ) 
	{
		if (  t.tquickhighlight == 1 ) 
		{
			//  toggle show on startup
			g.gshowonstartup=1-g.gshowonstartup ; t.tclicked=1;
		}
		if ( t.tquickhighlight == 3 || t.tquickhighlight == 4 ) 
		{
			//  close
			t.tclosequick=1;
			t.tclicked=1;
		}
		if (  t.tquickhighlight == 5 ) 
		{
			// jump to game/video menu
			g.quickstartmenumode = 1 - g.quickstartmenumode;

			// wait for click to be released
			do { t.inputsys.mclick=MouseClick(); FastSyncInputOnly(); } while ( !( t.inputsys.mclick == 0 ) );
		}
		if (  t.tquickhighlight == 6 ) 
		{
			// jump to twitch videos
			ExecuteFile ( "https://www.game-guru.com/live-streams","","",0 );
			t.tclicked=1;
		}
		if (  t.tquickhighlight == 7 ) 
		{
			// jump to PDF file
			// Now hosting it online
			//ExecuteFile ( "languagebank\\english\\artwork\\GameGuru - Getting Started Guide.pdf","","",0 );
			ExecuteFile ( "https://www.game-guru.com/downloads/pdfs/GameGuru%20-%20Getting%20Started%20Guide.pdf", "" , "", 0 );
			t.tclicked=1;
		}
	}
}
else
{
	t.tclicked=0;
}
if ( g.gshowonstartup == 1 ) 
{
	PasteImage (  g.editorimagesoffset+9,t.tcenterx+30,t.tcentery+522,1 );
}
if ( t.tquickhighlight == 3 ) 
{
	PasteImage (  g.interactiveimageoffset+15,t.tcenterx+789,t.tcentery+510 );
}
if ( t.tquickhighlight == 4 ) 
{
	PasteImage (  g.editorimagesoffset+44,t.tcenterx+860,t.tcentery+39 );
}
if ( t.tquickhighlight == 5 ) 
{
	if ( g.quickstartmenumode==0 )
		PasteImage (  g.editorimagesoffset+42,t.tcenterx+558,t.tcentery+510 );
	else
		PasteImage (  g.editorimagesoffset+43,t.tcenterx+583,t.tcentery+510 );
}
if ( t.tquickhighlight == 6 ) 
{
	if ( g.quickstartmenumode==0 )
	{
		PasteImage (  g.editorimagesoffset+47,t.tcenterx+350,t.tcentery+510 );
	}
}
if ( t.tquickhighlight == 7 ) 
{
	if ( g.quickstartmenumode==0 )
	{
		PasteImage (  g.editorimagesoffset+48,t.tcenterx+223,t.tcentery+510 );
	}
}
*/


/*
// PERFORMANCE CHECKER PAGE

struct welcomeperformancetype
{
	int iRunPerformanceCheck;
	float fRotateMe;
	int iNextButtonID;
	char pStatus[10240];
};
welcomeperformancetype g_welcomeperformance;

void welcome_performance_init ( void )
{
	memset ( &g_welcomeperformance, 0, sizeof(g_welcomeperformance) );
	g_welcomeperformance.fRotateMe = 0.0f;
	strcpy ( g_welcomeperformance.pStatus, "SYSTEM READY." );
}

void welcome_performance_page ( int iHighlightingButton )
{
	// draw page
	int iID = 0;
	welcome_text ( "PERFORMANCE CHECKER", 5, 50, 10, 255, false, false );
	welcome_drawbox ( 0, 10, 20, 40, 65 );
	welcome_drawbox ( 0, 42, 20, 90, 65 );
	welcome_text ( "Use the performance checker to help you determine the optimum settings\nfor performance and quality on your system.", 1, 50, 72, 192, true, false );
	welcome_text ( g_welcomeperformance.pStatus, 1, 45, 26, 192, true, true );
	welcome_drawimage ( g.editorimagesoffset+50, 15, 25, true );
	welcome_drawrotatedimage ( g.editorimagesoffset+51, 26, 46, g_welcomeperformance.fRotateMe, 16, 128, false );
	iID = 1; welcome_textinbox ( iID, "TEST NOW", 1, 25, 55, g_welcomebutton[iID].alpha );
	iID = g_welcomeperformance.iNextButtonID; welcome_textinbox ( iID, "NEXT", 1, 50, 90, g_welcomebutton[iID].alpha );

	// control page
	if ( t.inputsys.mclick == 1 ) 
	{
		if ( iHighlightingButton == 1 && g_welcomeperformance.iRunPerformanceCheck == 0 )
		{
			g_welcomeperformance.iRunPerformanceCheck = 1;
		}
		if ( iHighlightingButton == 2 ) t.tclosequick = 1;
	}

	// control any mechanics and actions
	if ( g_welcomeperformance.iRunPerformanceCheck > 0 && g_welcomeperformance.iRunPerformanceCheck < 1000 )
	{
		// run performance check counter
		g_welcomeperformance.iRunPerformanceCheck++;
		if ( g_welcomeperformance.iRunPerformanceCheck >= 1000 )
		{
			// finish performance check and activate NEXT button
			g_welcomeperformance.iRunPerformanceCheck = 1000;
			g_welcomeperformance.iNextButtonID = 2;
		}

		// sequence of checks to determine users machine speed
		if ( g_welcomeperformance.iRunPerformanceCheck == 10 )
		{
			strcpy ( g_welcomeperformance.pStatus, "SYSTEM TEST STARTED..\n" );
		}
		if ( g_welcomeperformance.iRunPerformanceCheck == 100 )
		{
			strcat ( g_welcomeperformance.pStatus, "CHECKING CPU..\n" );
		}
		if ( g_welcomeperformance.iRunPerformanceCheck == 200 )
		{
			strcat ( g_welcomeperformance.pStatus, "CHECKING GPU..\n" );
		}
		if ( g_welcomeperformance.iRunPerformanceCheck == 300 )
		{
			strcat ( g_welcomeperformance.pStatus, "CHECKING MEMORY..\n" );
		}
		if ( g_welcomeperformance.iRunPerformanceCheck == 400 )
		{
			strcat ( g_welcomeperformance.pStatus, "COMPLETE.\n\n" );
		}
		if ( g_welcomeperformance.iRunPerformanceCheck == 999 )
		{
			strcat ( g_welcomeperformance.pStatus, "CONFIGURING FOR 'LOWEST' MODE\n" );
		}

		// show performance needle in real-time determination of results
		g_welcomeperformance.fRotateMe -= 0.04f;
	}
}
*/

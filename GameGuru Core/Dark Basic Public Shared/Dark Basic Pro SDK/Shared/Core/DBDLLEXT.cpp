//
// EXT Specific Functions
//

// Common Includes
#include "windows.h"
#include "DBDLLCore.h"

#include "CTextC.h"
#include "CBasic2DC.h"
#include "CGfxC.h"
#include "CBasic2DC.h"

// External Gloobal Data
extern GlobStruct g_Glob;

DARKSDK void EXT_CreateDisplay(DWORD dwDisplayType)
{
	// Create Display Area (last flag is locable bb - performance flag here)
	#ifdef DX11
	//270517 - seems we are creating things TWICE - this default init not needed as we call SetDisplayMode later on in game code
	#else
	if(dwDisplayType==3)
		SetDisplayMode(g_Glob.dwWindowWidth, g_Glob.dwWindowHeight, g_Glob.iScreenDepth, 0, 0, 1);
	else
		SetDisplayMode(g_Glob.dwWindowWidth, g_Glob.dwWindowHeight, 0, 1, 0, 1);
	#endif
}

DARKSDK void EXT_DeleteDisplay(void)
{
	// Free Display Elements
	RestoreDisplayMode();
}

DARKSDK void EXT_SetDefaultDisplayProperties(void)
{
	// Assign Ink Colour

	// Assign Fill Style

	// Assign Font Style
	SetTextFont("Fixedsys", 0);
}

DARKSDK void EXT_ClearPrintArea(void)
{
	// Clear Display
	SETUPClear( 0, 0, 0 );
}

DARKSDK void EXT_ClearSomeText(int LeftmostToClear, int RightmostToClear)
{
	// Work out Text Details
	SIZE Size;
	Size.cx=GetTextWidth("_");
	Size.cy=GetTextHeight("_");

	// Clear Area
	DWORD dwStoreColor = g_Glob.dwForeColor;
	g_Glob.dwForeColor = g_Glob.dwBackColor;
	Box(LeftmostToClear, g_Glob.iCursorY, RightmostToClear, g_Glob.iCursorY+Size.cy);
	g_Glob.dwForeColor = dwStoreColor;
}

DARKSDK void EXT_ShiftDisplayUp(int iShiftUp)
{
	// Shift Area Up The Display
	CopyArea(0, 0, g_dwScreenWidth, g_dwScreenHeight, 0, iShiftUp);
}

DARKSDK void EXT_PrintSomething(LPSTR pStr, bool bIncludeCarriageReturn)
{
	if(pStr)
	{
		// Work out Text Details
		SIZE Size;
		Size.cx=GetTextWidth(pStr);//g_Text_GetWidth(pStr);
		Size.cy=GetTextHeight(pStr);
		if(Size.cy==0) Size.cy=GetTextHeight(" ");

		// Write Text
		Text(g_Glob.iCursorX, g_Glob.iCursorY, pStr);

		// Advance Cursor
		if(bIncludeCarriageReturn)
		{
			g_Glob.iCursorX=0;
			g_Glob.iCursorY+=Size.cy;

			// Scroll if reach bottom
			RECT rc;
			RECT winrc;
			GetClientRect(g_Glob.hWnd, &rc);
			GetWindowRect(g_Glob.hWnd, &winrc);
			int iTopDiff=(winrc.bottom-winrc.top)-rc.bottom;
			rc.top=0;
			rc.left=0;
			rc.right=g_Glob.dwWindowWidth;
			rc.bottom=g_Glob.dwWindowHeight-iTopDiff;
			if(g_Glob.iCursorY>rc.bottom-Size.cy)
			{
				// Shift Distance
				int iDist = g_Glob.iCursorY-(rc.bottom-Size.cy);

				// Shift Contents of display
				EXT_ShiftDisplayUp(iDist);

				// Clear Area
				DWORD dwStoreColor = g_Glob.dwForeColor;
				g_Glob.dwForeColor = g_Glob.dwBackColor;
				Box(0, rc.bottom-Size.cy, g_dwScreenWidth, g_dwScreenHeight);
				g_Glob.dwForeColor = dwStoreColor;

				// Restore Y Position
				g_Glob.iCursorY = (rc.bottom-Size.cy);
			}
		}
		else
		{
			g_Glob.iCursorX+=Size.cx;
		}
	}
}

DARKSDK SIZE EXT_GetTextSize(LPSTR pText)
{
	SIZE s;
	s.cx=GetTextWidth(pText);
	s.cy=GetTextHeight(pText);
	return s;
}

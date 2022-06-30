//
// EXT Specific Functions
//

// Common Includes
#include "windows.h"
#include "DBDLLCore.h"

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
}

DARKSDK void EXT_ClearPrintArea(void)
{
	// Clear Display
	SETUPClear( 0, 0, 0 );
}

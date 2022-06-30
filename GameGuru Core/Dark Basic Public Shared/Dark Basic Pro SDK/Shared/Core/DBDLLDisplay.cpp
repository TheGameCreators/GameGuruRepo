//
// Display Related Functions
//

// Common Includes
#include "windows.h"
#include "DBDLLGDI.h"
#include "DBDLLEXT.h"
#include "DBDLLCore.h"

// Global data to store system colors
DBPRO_GLOBAL bool bColoursCaptured=false;
DBPRO_GLOBAL DWORD aDefaultSystemColors[30];

DARKSDK void CaptureSystemColors ( void )
{
	// capture all system colors
	bColoursCaptured=true;
	for ( int i=0; i<29; i++)
		aDefaultSystemColors[i] = GetSysColor(i); 
}

DARKSDK void RestoreSystemColors ( void )
{
	// element array
	if ( bColoursCaptured==true )
	{
		int aiSysElements[30];
		for ( int i=0; i<29; i++)
			aiSysElements[i]=i;

		// restore all system colors
		SetSysColors(28, aiSysElements, aDefaultSystemColors); 

		// reset flag
		bColoursCaptured=false;
	}
}

DARKSDK void CreateDisplay(DWORD dwDisplayType)
{
	EXT_CreateDisplay(dwDisplayType);
	CaptureSystemColors();
}

DARKSDK void DeleteDisplay(void)
{
	RestoreSystemColors();
	EXT_DeleteDisplay();
}

DARKSDK void SetDefaultDisplayProperties(void)
{
	EXT_SetDefaultDisplayProperties();
}

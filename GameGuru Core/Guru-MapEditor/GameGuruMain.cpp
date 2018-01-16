//
// GameGuru Main Engine
//

// Engine includes
#include "GameGuruMain.h"
#include <stdio.h>
#include <string.h>
#include "gameguru.h"

// FBX Importing
#include <iostream>
#include "FBXExporter\FBXExporter.h"

void GuruMain ( void )
{
	// Launch GameGuru 
	common_init();
}

void LoadFBX ( LPSTR szFilename, int iID )
{
	// import FBX, convert to DBO
	FBXExporter* myExporter = new FBXExporter();
	myExporter->Initialize();
	myExporter->LoadScene( szFilename, "" );
	myExporter->ExportFBX( iID );
}

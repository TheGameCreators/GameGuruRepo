//
// DBDLLExtCalls Function
//

// Common Includes
#include "DBDLLCore.h"
#include "CCameraC.h"
#include "CObjectsC.h"
#include "RenderList.h"

// Global External Data
extern GlobStruct g_Glob;

// mike - 100405
#define DBPRO_GLOBAL 

// Setup Function
DARKSDK bool CreateRenderOrderList(void)
{
	// Create render order list
    AddToRenderList( CAMERAUpdate,				0 );
    AddToRenderList( UpdateOnce,				1000 );
    AddToRenderList( Update,					8000 );
    AddToRenderList( UpdateGhostLayer,			10000 );
    AddToRenderList( UpdateNoZDepth,			13000 );

	// Complete
	return true;
}

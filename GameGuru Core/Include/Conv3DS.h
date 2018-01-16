//
// Conv3DS Functions Header
//

//////////////////////////////////////////////////////////////////////////////////
// INCLUDE COMMON FILES //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#include <windows.h>


//////////////////////////////////////////////////////////////////////////////////////
// DBOFORMAT INCLUDE /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOBlock.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFormat.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFrame.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOMesh.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBORawMesh.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOEffects.h"
#include "..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\DBOFormat\DBOFile.h"
#include "globstruct.h"

//#ifdef DARKSDK_COMPILE
	#undef DARKSDK
	#undef DBPRO_GLOBAL
	#define DARKSDK 
	#define DBPRO_GLOBAL static
//#else
//	#define DARKSDK __declspec ( dllexport )
//	#define DBPRO_GLOBAL 
//	#define DARKSDK_DLL 
//#endif

//////////////////////////////////////////////////////////////////////////////////
// DEFINES ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//#define DARKSDK __declspec ( dllexport )
//#define SAFE_DELETE( p )       { if ( p ) { delete ( p );       ( p ) = NULL; } }
//#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
//#define SAFE_DELETE_ARRAY( p ) { if ( p ) { delete [ ] ( p );   ( p ) = NULL; } }

//////////////////////////////////////////////////////////////////////////////////////
// EXPORTED FUNCTIONS ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
#ifndef DARKSDK_COMPILE
extern "C"
{
	DARKSDK void	Conv3DSPassCoreData	( LPVOID pGlobPtr );
	DARKSDK	bool	Conv3DSConvert			( LPSTR pFilename, DWORD *pBlock, DWORD* pdwSize );
	DARKSDK void	Conv3DSFree			( LPSTR );
}
#else
	void	PassCoreData3DS	( LPVOID pGlobPtr );
	bool	Convert3DS		( LPSTR pFilename, DWORD *pBlock, DWORD* pdwSize );
	void	Free3DS			( LPSTR );
#endif

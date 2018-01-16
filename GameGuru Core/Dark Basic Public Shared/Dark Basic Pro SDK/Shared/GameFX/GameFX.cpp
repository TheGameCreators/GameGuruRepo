//
// GameFX
//

/*

// Includes
#include ".\..\..\..\..\Include\GameFX.h"

//
// CONSTRUCTOR FUNCTIONS
//

void GAMEFXConstructor ( void )
{
}

void GAMEFXDestructor ( void )
{
}

//
// MAIN GAMEFX FUNCTIONS
//

// Static
void CreateNodeTree						( float fX, float fY, float fZ )
{
	B3D_CreateNodeTree ( fX, fY, fZ );
}

void AddNodeTreeObject					( int iID, int iType, int iArbitaryValue, int iCastShadow, int iPortalBlocker )
{
	B3D_AddNodeTreeObject ( iID, iType, iArbitaryValue, iCastShadow, iPortalBlocker );
}

void AddNodeTreeLimb						( int iID, int iLimb, int iType, int iArbitaryValue, int iCastShadow, int iPortalBlocker )
{
	B3D_AddNodeTreeLimb ( iID, iLimb, iType, iArbitaryValue, iCastShadow, iPortalBlocker );
}

void RemoveNodeTreeObject					( int iID )
{
	B3D_RemoveNodeTreeObject ( iID );
}

void DeleteNodeTree						( void )
{
	B3D_DeleteNodeTree();
}

void SetNodeTreeWireframeOn				( void )
{
	B3D_SetNodeTreeWireframeOn();
}

void SetNodeTreeWireframeOff				( void )
{
	B3D_SetNodeTreeWireframeOff();
}

void MakeNodeTreeCollisionBox				( float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2 )
{
	B3D_MakeNodeTreeCollisionBox ( fX1, fY1, fZ1, fX2, fY2, fZ2 );
}

void SetNodeTreeTextureMode				( int iMode )
{
	B3D_SetNodeTreeTextureMode ( iMode );
}

void DisableNodeTreeOcclusion				( void )
{
	B3D_DisableNodeTreeOcclusion();
}

void EnableNodeTreeOcclusion				( void )
{
	B3D_EnableNodeTreeOcclusion();
}

void SaveNodeTreeObjects					( DWORD dwFilename )
{
	B3D_SaveNodeTreeObjects	( (LPSTR)dwFilename );
}

void SetNodeTreeEffectTechnique					( DWORD dwFilename )
{
	B3D_SetNodeTreeEffectTechnique ( (LPSTR)dwFilename );
}

void LoadNodeTreeObjects					( DWORD dwFilename, int iDivideTextureSize )
{
	B3D_LoadNodeTreeObjects	( (LPSTR)dwFilename, iDivideTextureSize );
}

void AttachObjectToNodeTree				( int iID )
{
	B3D_AttachObjectToNodeTree ( iID );
}

void DetachObjectFromNodeTree				( int iID )
{
	B3D_DetachObjectFromNodeTree ( iID );
}

void SetNodeTreePortalsOn					( void )
{
	B3D_SetNodeTreePortalsOn();
}

void SetNodeTreePortalsOff				( void )
{
	B3D_SetNodeTreePortalsOff();
}

void SetNodeTreeCulling ( int iFlag )
{
	B3D_SetNodeTreeCulling(iFlag);
}

void BuildNodeTreePortals					( void )
{
	B3D_BuildNodeTreePortals();
}

void SetNodeTreeScorchTexture				( int iImageID, int iWidth, int iHeight )
{
	B3D_SetNodeTreeScorchTexture	( iImageID, iWidth, iHeight );
}

void AddNodeTreeScorch					( float fSize, int iType )
{
	B3D_AddNodeTreeScorch ( fSize, iType );
}

void AddNodeTreeLight						( int iLightIndex, float fX, float fY, float fZ, float fRange )
{
	B3D_AddNodeTreeLight ( iLightIndex, fX, fY, fZ, fRange );
}

// Static Expressions
DARKSDK int GetStaticHit ( float fOldX1, float fOldY1, float fOldZ1, float fOldX2, float fOldY2, float fOldZ2, float fNX1,   float fNY1,   float fNZ1,   float fNX2,   float fNY2,   float fNZ2   )
{
	return B3D_GetStaticHit ( fOldX1, fOldY1, fOldZ1, fOldX2, fOldY2, fOldZ2, fNX1, fNY1, fNZ1, fNX2, fNY2, fNZ2   );
}

DARKSDK int GetStaticLineOfSight ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz, float fWidth, float fAccuracy )
{
	return B3D_GetStaticLineOfSight ( fSx, fSy, fSz, fDx, fDy, fDz, fWidth, fAccuracy );
}

DARKSDK int GetStaticRayCast ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz )
{
	return B3D_GetStaticRayCast ( fSx, fSy, fSz, fDx, fDy, fDz );
}

DARKSDK int GetStaticVolumeCast ( float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float fSize )
{
	return B3D_GetStaticVolumeCast ( fX, fY, fZ, fNewX, fNewY, fNewZ, fSize );
}

DARKSDK float GetStaticX ( void )
{
	return (float)B3D_GetStaticX ( );
}

DARKSDK float GetStaticY ( void )
{
	return (float)B3D_GetStaticY ( );
}

DARKSDK float GetStaticZ ( void )
{
	return (float)B3D_GetStaticZ ( );
}

DARKSDK int GetStaticFloor ( void )
{
	return B3D_GetStaticFloor ( );
}

DARKSDK int GetStaticColCount ( void )
{
	return B3D_GetStaticColCount ( );
}

DARKSDK int GetStaticColValue ( void )
{
	return B3D_GetStaticColValue ( );
}

DARKSDK float GetStaticLineOfSightX ( void )
{
	return (float)B3D_GetStaticLineOfSightX ( );
}

DARKSDK float GetStaticLineOfSightY ( void )
{
	return (float)B3D_GetStaticLineOfSightY ( );
}

DARKSDK float GetStaticLineOfSightZ ( void )
{
	return (float)B3D_GetStaticLineOfSightZ ( );
}

// CSG Commands (CSG)

void PeformCSGUnion						( int iObjectA, int iObjectB )
{
	B3D_PeformCSGUnion ( iObjectA, iObjectB );
}

void PeformCSGDifference					( int iObjectA, int iObjectB )
{
	B3D_PeformCSGDifference ( iObjectA, iObjectB );
}

void PeformCSGIntersection				( int iObjectA, int iObjectB )
{
	B3D_PeformCSGIntersection ( iObjectA, iObjectB );
}

void PeformCSGClip						( int iObjectA, int iObjectB )
{
	B3D_PeformCSGClip ( iObjectA, iObjectB );
}

void PeformCSGUnionOnVertexData			( int iBrushMeshID )
{
	B3D_PeformCSGUnionOnVertexData ( iBrushMeshID );
}

void PeformCSGDifferenceOnVertexData		( int iBrushMeshID )
{
	B3D_PeformCSGDifferenceOnVertexData ( iBrushMeshID );
}

void PeformCSGIntersectionOnVertexData	( int iBrushMeshID )
{
	B3D_PeformCSGIntersectionOnVertexData ( iBrushMeshID );
}

DARKSDK int  GFObjectBlocking						( int iID, float X1, float Y1, float Z1, float X2, float Y2, float Z2 )
{
	return B3D_ObjectBlocking ( iID, X1, Y1, Z1, X2, Y2, Z2 );
}

void ReduceMesh							( int iMeshID, int iBlockMode, int iNearMode, int iGX, int iGY, int iGZ )
{
	B3D_ReduceMesh ( iMeshID, iBlockMode, iNearMode, iGX, iGY, iGZ );
}

void MakeLODFromMesh						( int iMeshID, int iVertNum, int iNewMeshID )
{
	B3D_MakeLODFromMesh	( iMeshID, iVertNum, iNewMeshID );
}

// Light Maps

void AddObjectToLightMapPool				( int iID )
{
	B3D_AddObjectToLightMapPool ( iID );
}

void AddLimbToLightMapPool				( int iID, int iLimb )
{
	B3D_AddLimbToLightMapPool ( iID, iLimb );
}

void AddStaticObjectsToLightMapPool		( void )
{
	B3D_AddStaticObjectsToLightMapPool();
}

void AddLightMapLight						( float fX, float fY, float fZ, float fRadius, float fRed, float fGreen, float fBlue, float fBrightness, int bCastShadow )
{
	B3D_AddLightMapLight ( fX, fY, fZ, fRadius, fRed, fGreen, fBlue, fBrightness, bCastShadow );
}

void FlushLightMapLights					( void )
{
	B3D_FlushLightMapLights();
}

void CreateLightMaps						( int iLMSize, int iLMQuality, DWORD dwPathForLightMaps )
{
	B3D_CreateLightMaps	( iLMSize, iLMQuality, (LPSTR)dwPathForLightMaps );
}

// Shadows

void SetGlobalShadowsOn					( void )
{
	B3D_SetGlobalShadowsOn();
}

void SetGlobalShadowsOff					( void )
{
	B3D_SetGlobalShadowsOff();
}

void SetShadowPosition ( int iMode, float fX, float fY, float fZ )
{
	B3D_SetShadowPosition (  iMode,  fX,  fY,  fZ );
}

void SetShadowColor ( int iRed, int iGreen, int iBlue, int iAlphaLevel )
{
	B3D_SetShadowColor (  iRed,  iGreen,  iBlue,  iAlphaLevel );
}

void SetShadowShades ( int iShades )
{
	B3D_SetShadowShades (  iShades );
}

// Others

void AddLODToObject ( int iCurrentID, int iLODModelID, int iLODLevel, float fDistanceOfLOD )
{
	B3D_AddLODToObject ( iCurrentID, iLODModelID, iLODLevel, fDistanceOfLOD );
}


// Explosions
void MakeExplosion ( int iExplosionID, float fX, float fY, float fZ )
{
	MessageBox ( NULL, "bang", "bang!", MB_OK );
}

*/

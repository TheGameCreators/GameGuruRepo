//
// GameFX Header
//

// Includes
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
//#include "globstruct.h"

#define MYDLL 
#define DARKSDK 

/*
// Constructor Functions (optional)
MYDLL void GAMEFXConstructor ( void );
MYDLL void GAMEFXDestructor ( void );

// Static
DARKSDK void CreateNodeTree						( float fX, float fY, float fZ );
DARKSDK void AddNodeTreeObject					( int iID, int iType, int iArbitaryValue, int iCastShadow, int iPortalBlocker );
DARKSDK void AddNodeTreeLimb					( int iID, int iLimb, int iType, int iArbitaryValue, int iCastShadow, int iPortalBlocker );
DARKSDK void RemoveNodeTreeObject				( int iID );
DARKSDK void DeleteNodeTree						( void );
DARKSDK void SetNodeTreeWireframeOn				( void );
DARKSDK void SetNodeTreeWireframeOff			( void );
DARKSDK void MakeNodeTreeCollisionBox			( float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2 );
DARKSDK void SetNodeTreeTextureMode				( int iMode );
DARKSDK void DisableNodeTreeOcclusion			( void );
DARKSDK void EnableNodeTreeOcclusion			( void );
DARKSDK void SaveNodeTreeObjects				( DWORD dwFilename );
DARKSDK void SetNodeTreeEffectTechnique			( DWORD dwFilename );
DARKSDK void LoadNodeTreeObjects				( DWORD dwFilename, int iDivideTextureSize );
DARKSDK void AttachObjectToNodeTree				( int iID );
DARKSDK void DetachObjectFromNodeTree			( int iID );
DARKSDK void SetNodeTreePortalsOn				( void );
DARKSDK void SetNodeTreePortalsOff				( void );
DARKSDK void SetNodeTreeCulling					( int iFlag );
DARKSDK void BuildNodeTreePortals				( void );
DARKSDK void SetNodeTreeScorchTexture			( int iImageID, int iWidth, int iHeight );
DARKSDK void AddNodeTreeScorch					( float fSize, int iType );
DARKSDK void AddNodeTreeLight					( int iLightIndex, float fX, float fY, float fZ, float fRange );

// Static Expressions
DARKSDK int GetStaticHit						( float fOldX1, float fOldY1, float fOldZ1, float fOldX2, float fOldY2, float fOldZ2, float fNX1,   float fNY1,   float fNZ1,   float fNX2,   float fNY2,   float fNZ2   );
DARKSDK int GetStaticLineOfSight				( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz, float fWidth, float fAccuracy );
DARKSDK int GetStaticRayCast					( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz );
DARKSDK int GetStaticVolumeCast					( float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float fSize );
DARKSDK float GetStaticX						( void );
DARKSDK float GetStaticY						( void );
DARKSDK float GetStaticZ						( void );
DARKSDK int GetStaticFloor						( void );
DARKSDK int GetStaticColCount					( void );
DARKSDK int GetStaticColValue					( void );
DARKSDK float GetStaticLineOfSightX				( void );
DARKSDK float GetStaticLineOfSightY				( void );
DARKSDK float GetStaticLineOfSightZ				( void );

// CSG Commands (CSG)
DARKSDK void PeformCSGUnion						( int iObjectA, int iObjectB );
DARKSDK void PeformCSGDifference				( int iObjectA, int iObjectB );
DARKSDK void PeformCSGIntersection				( int iObjectA, int iObjectB );
DARKSDK void PeformCSGClip						( int iObjectA, int iObjectB );
DARKSDK void PeformCSGUnionOnVertexData			( int iBrushMeshID );
DARKSDK void PeformCSGDifferenceOnVertexData	( int iBrushMeshID );
DARKSDK void PeformCSGIntersectionOnVertexData	( int iBrushMeshID );
DARKSDK int  ObjectBlocking						( int iID, float X1, float Y1, float Z1, float X2, float Y2, float Z2 );
DARKSDK void ReduceMesh							( int iMeshID, int iBlockMode, int iNearMode, int iGX, int iGY, int iGZ );
DARKSDK void MakeLODFromMesh					( int iMeshID, int iVertNum, int iNewMeshID );

// Light Maps
DARKSDK void AddObjectToLightMapPool			( int iID );
DARKSDK void AddLimbToLightMapPool				( int iID, int iLimb );
DARKSDK void AddStaticObjectsToLightMapPool		( void );
DARKSDK void AddLightMapLight					( float fX, float fY, float fZ, float fRadius, float fRed, float fGreen, float fBlue, float fBrightness, int bCastShadow );
DARKSDK void FlushLightMapLights				( void );
DARKSDK void CreateLightMaps					( int iLMSize, int iLMQuality, DWORD dwPathForLightMaps );

// Shadows
DARKSDK void SetGlobalShadowsOn					( void );
DARKSDK void SetGlobalShadowsOff				( void );
DARKSDK void SetShadowPosition					( int iMode, float fX, float fY, float fZ );
DARKSDK void SetShadowColor						( int iRed, int iGreen, int iBlue, int iAlphaLevel );
DARKSDK void SetShadowShades					( int iShades );

// Others
DARKSDK void AddLODToObject						( int iCurrentID, int iLODModelID, int iLODLevel, float fDistanceOfLOD );

// Explosions
DARKSDK void MakeExplosion						( int iExplosionID, float fX, float fY, float fZ );
*/

//
// LightMapper Header
//

#define DLLEXPORT 
#include "windows.h"
//class sObject;

DLLEXPORT void LMStart ( );
DLLEXPORT void LMAddCollisionObject ( int iObjID );
DLLEXPORT void LMBuildCollisionData( );
DLLEXPORT void LMAddLightMapObject( int iObjID, sObject *pObject, int iBaseStage, int iDynamicLight, int iShaded, int iFlatNormals );
DLLEXPORT void LMBuildLightMaps( int iTexSize, float fQuality, int iBlur, int iNumThreads );
DLLEXPORT void LMSetBlendMode( int iNewMode );
DLLEXPORT void LMAddPointLight( float posX, float posY, float posZ, float radius, float red, float green, float blue );
DLLEXPORT void LMAddDirectionalLight( float dirX, float dirY, float dirZ, float red, float green, float blue );
DLLEXPORT void LMAddSpotLight( float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float ang1, float ang2, float range, float red, float green, float blue );
DLLEXPORT void LMClearLights( );
DLLEXPORT void LMClearCollisionObjects( );
DLLEXPORT void LMClearLightMapObjects( );
DLLEXPORT void LMReset( );
DLLEXPORT void LMSetAmbientLight( float red, float green, float blue );
DLLEXPORT void LMAddCustomPointLight( float posX, float posY, float posZ, float radius, float attenuation, float attenuation2, float red, float green, float blue );
DLLEXPORT void LMBuildLightMapsThread( int iTexSize, float fQuality, int iBlur, int iNumThreads );
DLLEXPORT float LMGetPercent( );
DLLEXPORT LPSTR LMGetStatus( void );
DLLEXPORT int LMGetComplete( );
DLLEXPORT void LMAddLightMapObject( int iObjID, sObject *pObject, int iBaseStage, int iDynamicLight, int iShaded, int iFlatNormals );
DLLEXPORT void LMAddTransparentCollisionObject ( int iObjID, int iType );
DLLEXPORT void LMSetMode( int iMode );
DLLEXPORT void LMSetShadowPower( float fPower );
DLLEXPORT void LMSetAmbientOcclusionOn( int iIterations, float fRayDist, int iPattern );
DLLEXPORT void LMSetAmbientOcclusionOff( );
DLLEXPORT void LMSetLightMapName ( DWORD pInString );
DLLEXPORT void LMSetLightMapFolder ( LPSTR pInString );
DLLEXPORT void LMAddLightMapObject( int iObjID, sObject *pObject, int iBaseStage, int iDynamicLight, int iShaded, int iFlatNormals );
DLLEXPORT void LMTerminateThread( );
DLLEXPORT void LMAddShadedLightMapObject ( int iObjID, int iLightMapStage );
DLLEXPORT void LMAddShadedLightMapObject ( int iObjID, int iLightMapStage, int iFlatShaded );
DLLEXPORT void LMBoostCurvedSurfaceQuality( float fMaxSize, float fBoost );
DLLEXPORT void LMAddShadedLightMapObject ( int iObjID, int iLightMapStage, int iFlatShaded );
DLLEXPORT void LMAddLightMapObject ( int iObjID, int iBaseStage, int iDynamicLight );
DLLEXPORT void LMBuildLightMapsThread( int iTexSize, float fQuality, int iBlur, int iNumThreads );
DLLEXPORT void LMBuildLightMaps( int iTexSize, float fQuality, int iBlur, int iNumThreads );
DLLEXPORT void LMSetLightMapFolder ( DWORD pInString );
DLLEXPORT void LMSetLightMapFileFormat ( int iFormat );
DLLEXPORT void LMCompleteLightMaps( );
DLLEXPORT void LMSetLightMapStartNumber ( int iFileNumber );
DLLEXPORT int LMGetLightMapLastNumber( );
DLLEXPORT void LMBuildLightMapsStart( int iTexSize, float fQuality, int iBlur );
DLLEXPORT int LMBuildLightMapsCycle( void );


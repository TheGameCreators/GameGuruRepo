#ifndef _CPARTICLE_H_
#define _CPARTICLE_H_

#include "directx-macros.h"
#include ".\..\Dark Basic Pro SDK\Shared\data\cdatac.h"
#include ".\..\Dark Basic Pro SDK\Shared\SpecialEffects\cpositionc.h"
#include ".\..\Dark Basic Pro SDK\Shared\SpecialEffects\cparticlemanagerc.h"
#include <math.h>

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>   
#include <windowsx.h>

#undef DARKSDK
#define DARKSDK

#define DB_PRO 1
#if DB_PRO
 #define SDK_BOOL int
 #define SDK_FLOAT float
 #define SDK_LPSTR DWORD
 #define SDK_RETFLOAT float
#endif

DARKSDK void ParticlesConstructor			( );
DARKSDK void ParticlesDestructor		( void );
DARKSDK void ParticlesSetErrorHandler		( LPVOID pErrorHandlerPtr );
DARKSDK void ParticlesPassCoreData			( LPVOID pGlobPtr );
DARKSDK void ParticlesRefreshD3D ( int iMode );
DARKSDK void ParticlesUpdate					( void );

bool UpdateParticlePtr ( int iID );

DARKSDK bool MakeParticles			( int iID, DWORD dwFlush, DWORD dwDiscard, float fRadius, int iImage );
DARKSDK void DeleteParticles		( int iID );
DARKSDK void SetParticlesColor		( int iID, int iRed, int iGreen, int iBlue );
DARKSDK void SetParticlesTime		( int iID, float fTime );

// commands

DARKSDK void CreateParticlesEx				( int iID, int iImageIndex, int maxParticles, float fRadius );
DARKSDK void DeleteParticlesEx				( int iID );

DARKSDK void SetParticlesSecondsPerFrame		( int iID, float fTime );
DARKSDK void SetParticlesNumberOfEmmissions	( int iID, int iNumber );
DARKSDK void SetParticlesVelocity			( int iID, float fVelocity );
DARKSDK void SetParticlesPosition			( int iID, float fX, float fY, float fZ );
DARKSDK void SetParticlesEmitPosition		( int iID, float fX, float fY, float fZ );
DARKSDK void SetParticlesColorEx				( int iID, int iRed, int iGreen, int iBlue );
DARKSDK void SetParticlesRotation			( int iID, float fX, float fY, float fZ );
DARKSDK void SetParticlesGravity				( int iID, float fGravity );
DARKSDK void SetParticlesChaos				( int iID, float fChaos );
DARKSDK void SetParticlesLife				( int iID, int iLifeValue );
DARKSDK void SetParticlesMask				( int iID, int iCameraMask );
DARKSDK void EmitSingleParticle		( int iID, float fX, float fY, float fZ );
DARKSDK void SetParticlesFloor				( int iID, int iFlag );
DARKSDK void GhostParticlesOn				( int iID, int iMode );
DARKSDK void GhostParticlesOff				( int iID );
DARKSDK void GhostParticles					( int iID, int iMode, float fPercentage );

DARKSDK void HideParticles					( int iID );
DARKSDK void ShowParticles					( int iID );

DARKSDK void CreateParticlesSnowEffect		( int iID, int iImageIndex, int maxParticles, float fX, float fY, float fZ, float fWidth, float fHeight, float fDepth );
DARKSDK void CreateParticlesFireEffect		( int iID, int iImageIndex, int maxParticles, float fX, float fY, float fZ, float fWidth, float fHeight, float fDepth );

DARKSDK void SetParticlesPositionVector3		( int iID, int iVector );
DARKSDK void GetParticlesPositionVector3		( int iVector, int iID );
DARKSDK void SetParticlesRotationVector3		( int iID, int iVector );
DARKSDK void GetParticlesRotationVector3		( int iVector, int iID );

// expressions

DARKSDK SDK_BOOL ParticlesExist			( int iID );
DARKSDK SDK_FLOAT ParticlesPositionX	( int iID );
DARKSDK SDK_FLOAT ParticlesPositionY	( int iID );
DARKSDK SDK_FLOAT ParticlesPositionZ	( int iID );

#endif _CPARTICLE_H_
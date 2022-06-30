#ifndef _CPOSITION_H_
#define _CPOSITION_H_

#include "directx-macros.h"

struct tagObjectPos
{
	GGMATRIX 	matObject;			// transformation matrix

	GGVECTOR3		vecLook;			// look vector
	GGVECTOR3		vecUp;				// up vector
	GGVECTOR3		vecRight;			// right vector
	GGVECTOR3		vecPosition;		// position vector
	GGVECTOR3		vecRotate;			// rotate vector
	GGVECTOR3		vecYawPitchRoll;	// yaw, pitch and roll
	GGVECTOR3		vecScale;			// scale vector
	GGVECTOR3		vecLast;
	GGVECTOR3		vecLastPosition;
	float			fAspect;			// aspect ratio
	float			fFOV;				// field of view
	float			fZNear;				// z near
	float			fZFar;				// z far
	bool			bRotate;			// rotation type
	bool			bLock;				// locked to camera
	bool			bHasBeenMovedForResponse; //used for auto-collision-response

	bool			bFreeFlightRotation;

	bool			bApplyPivot;	// used to fix pivot
	GGVECTOR3		vecPivot;		

	GGMATRIX 	matObjectNoTran;	// new - get the final matrix just before translation
	bool			bColCenterUpdated;	// used to track center of actual mesh for collision
	GGVECTOR3		vecColCenter;

	tagObjectPos ( )
	{
		vecLook     = GGVECTOR3 ( 0, 0, 1 );		// look vector
		vecUp       = GGVECTOR3 ( 0, 1, 0 );		// up vector
		vecRight    = GGVECTOR3 ( 1, 0, 0 );		// right vector

		bFreeFlightRotation = false;
		
		memset ( &matObject, 0, sizeof ( GGMATRIX ) );
	}
};

void ParticleInternalUpdate ( int iID );		// update any changes to camera


// Internal Functions
float ParticleGetXPosition ( int iID );
float ParticleGetYPosition ( int iID );
float ParticleGetZPosition ( int iID );
float ParticleGetXRotation ( int iID );
float ParticleGetYRotation ( int iID );
float ParticleGetZRotation ( int iID );


// DBV1 Euler
void ParticleScale     ( int iID, float fX, float fY, float fZ );
void ParticlePosition  ( int iID, float fX, float fY, float fZ );


void ParticleRotate    ( int iID, float fX, float fY, float fZ );
void ParticleXRotate   ( int iID, float fX );
void ParticleYRotate   ( int iID, float fY );
void ParticleZRotate   ( int iID, float fZ );
void ParticlePoint     ( int iID, float fX, float fY, float fZ );

void ParticleMove      ( int iID, float fStep );
void ParticleMoveUp    ( int iID, float fStep );
void ParticleMoveDown  ( int iID, float fStep );
void ParticleMoveLeft  ( int iID, float fStep );
void ParticleMoveRight ( int iID, float fStep );

// DBV1 Freeflight
void ParticleTurnLeft  ( int iID, float fAngle );
void ParticleTurnRight ( int iID, float fAngle );
void ParticlePitchUp   ( int iID, float fAngle );
void ParticlePitchDown ( int iID, float fAngle );
void ParticleRollLeft  ( int iID, float fAngle );
void ParticleRollRight ( int iID, float fAngle );

// DBV1 Expressions
DWORD ParticleGetXPositionEx ( int iID );
DWORD ParticleGetYPositionEx ( int iID );
DWORD ParticleGetZPositionEx ( int iID );
DWORD ParticleGetXRotationEx ( int iID );
DWORD ParticleGetYRotationEx ( int iID );
DWORD ParticleGetZRotationEx ( int iID );


#endif _CPOSITION_H_

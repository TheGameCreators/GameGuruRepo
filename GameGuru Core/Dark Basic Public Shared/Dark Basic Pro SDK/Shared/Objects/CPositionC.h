#ifndef _CPOSITION_H_
#define _CPOSITION_H_

#include "directx-macros.h"
#include "global.h"

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

void InternalUpdate ( int iID );		// update any changes to camera

//#ifndef DARKSDK_COMPILE

	/*
#else
	// Internal Functions
	float GetXPosition ( int iID );
	float GetYPosition ( int iID );
	float GetZPosition ( int iID );
	float GetXRotation ( int iID );
	float GetYRotation ( int iID );
	float GetZRotation ( int iID );

	// DBV1 Euler
	void Scale     ( int iID, float fX, float fY, float fZ );

	void Position  ( int iID, float fX, float fY, float fZ );

	void Rotate    ( int iID, float fX, float fY, float fZ );
	void XRotate   ( int iID, float fX );
	void YRotate   ( int iID, float fY );
	void ZRotate   ( int iID, float fZ );
	void Point     ( int iID, float fX, float fY, float fZ );

	void Move      ( int iID, float fStep );
	void MoveUp    ( int iID, float fStep );
	void MoveDown  ( int iID, float fStep );
	void MoveLeft  ( int iID, float fStep );
	void MoveRight ( int iID, float fStep );

	// DBV1 Freeflight
	void TurnLeft  ( int iID, float fAngle );
	void TurnRight ( int iID, float fAngle );
	void PitchUp   ( int iID, float fAngle );
	void PitchDown ( int iID, float fAngle );
	void RollLeft  ( int iID, float fAngle );
	void RollRight ( int iID, float fAngle );

	// DBV1 Expressions
	DWORD GetXPositionEx ( int iID );
	DWORD GetYPositionEx ( int iID );
	DWORD GetZPositionEx ( int iID );
	DWORD GetXRotationEx ( int iID );
	DWORD GetYRotationEx ( int iID );
	DWORD GetZRotationEx ( int iID );

	///////////////

	// Internal Functions
	float dbtObjectGetXPosition ( int iID );
	float dbtObjectGetYPosition ( int iID );
	float dbtObjectGetZPosition ( int iID );
	float dbtObjectGetXRotation ( int iID );
	float dbtObjectGetYRotation ( int iID );
	float dbtObjectGetZRotation ( int iID );

	// DBV1 Euler
	void dbtObjectScale     ( int iID, float fX, float fY, float fZ );
	void dbtObjectPosition  ( int iID, float fX, float fY, float fZ );

	void dbtObjectRotate    ( int iID, float fX, float fY, float fZ );
	void dbtObjectXRotate   ( int iID, float fX );
	void dbtObjectYRotate   ( int iID, float fY );
	void dbtObjectZRotate   ( int iID, float fZ );
	void dbtObjectPoint     ( int iID, float fX, float fY, float fZ );

	void dbtObjectMove      ( int iID, float fStep );
	void dbtObjectMoveUp    ( int iID, float fStep );
	void dbtObjectMoveDown  ( int iID, float fStep );
	void dbtObjectMoveLeft  ( int iID, float fStep );
	void dbtObjectMoveRight ( int iID, float fStep );

	// DBV1 Freeflight
	void dbtObjectTurnLeft  ( int iID, float fAngle );
	void dbtObjectTurnRight ( int iID, float fAngle );
	void dbtObjectPitchUp   ( int iID, float fAngle );
	void dbtObjectPitchDown ( int iID, float fAngle );
	void dbtObjectRollLeft  ( int iID, float fAngle );
	void dbtObjectRollRight ( int iID, float fAngle );

	// DBV1 Expressions
	DWORD dbtObjectGetXPositionEx ( int iID );
	DWORD dbtObjectGetYPositionEx ( int iID );
	DWORD dbtObjectGetZPositionEx ( int iID );
	DWORD dbtObjectGetXRotationEx ( int iID );
	DWORD dbtObjectGetYRotationEx ( int iID );
	DWORD dbtObjectGetZRotationEx ( int iID );
#endif
	*/

#endif _CPOSITION_H_
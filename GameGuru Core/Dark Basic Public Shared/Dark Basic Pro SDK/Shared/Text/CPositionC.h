#ifndef _CPOSITION_H_
#define _CPOSITION_H_

#include <directx-macros.h>

struct tagObjectPos
{
	GGMATRIX		matObject;			// transformation matrix

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

	GGMATRIX		matObjectNoTran;	// new - get the final matrix just before translation
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

void TextInternalUpdate ( int iID );		// update any changes to camera

// Internal Functions
float TextGetXPosition ( int iID );
float TextGetYPosition ( int iID );
float TextGetZPosition ( int iID );
float TextGetXRotation ( int iID );
float TextGetYRotation ( int iID );
float TextGetZRotation ( int iID );


// DBV1 Euler
void TextScale     ( int iID, float fX, float fY, float fZ );
void TextPosition  ( int iID, float fX, float fY, float fZ );


void TextRotate    ( int iID, float fX, float fY, float fZ );
void TextXRotate   ( int iID, float fX );
void TextYRotate   ( int iID, float fY );
void TextZRotate   ( int iID, float fZ );
void TextPoint     ( int iID, float fX, float fY, float fZ );

void TextMove      ( int iID, float fStep );
void TextMoveUp    ( int iID, float fStep );
void TextMoveDown  ( int iID, float fStep );
void TextMoveLeft  ( int iID, float fStep );
void TextMoveRight ( int iID, float fStep );

// DBV1 Freeflight
void TextTurnLeft  ( int iID, float fAngle );
void TextTurnRight ( int iID, float fAngle );
void TextPitchUp   ( int iID, float fAngle );
void TextPitchDown ( int iID, float fAngle );
void TextRollLeft  ( int iID, float fAngle );
void TextRollRight ( int iID, float fAngle );

// DBV1 Expressions
DWORD TextGetXPositionEx ( int iID );
DWORD TextGetYPositionEx ( int iID );
DWORD TextGetZPositionEx ( int iID );
DWORD TextGetXRotationEx ( int iID );
DWORD TextGetYRotationEx ( int iID );
DWORD TextGetZRotationEx ( int iID );

#endif _CPOSITION_H_

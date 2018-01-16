#include "cpositionc.h"
#include ".\..\error\cerror.h"

tagObjectPos* m_pParticlePos;

extern LPGGDEVICE		m_pD3D;
extern bool UpdateParticlePtr ( int iID );

static void ParticleInternalUpdate ( int iID );

// internal

float ParticleGetXPosition ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return m_pParticlePos->vecPosition.x;
}

float ParticleGetYPosition ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return m_pParticlePos->vecPosition.y;
}

float ParticleGetZPosition ( int iID )
{
	// update internal pointer
	if ( !UpdateParticlePtr ( iID ) )
		return 0;

	return m_pParticlePos->vecPosition.z;
}

float ParticleGetXRotation ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return m_pParticlePos->vecRotate.x;
}

float ParticleGetYRotation ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return m_pParticlePos->vecRotate.y;
}

float ParticleGetZRotation ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return m_pParticlePos->vecRotate.z;
}

static float wrapangleoffset(float da)
{
	int breakout=100;
	while(da<0.0f || da>=360.0f)
	{
		if(da<0.0f) da=da+360.0f;
		if(da>=360.0f) da=da-360.0f;
		breakout--;
		if(breakout==0) break;
	}
	if(breakout==0) da=0.0f;
	return da;
}

static void GetAngleFromPoint(float x1, float y1, float z1, float x2, float y2, float z2, float* ax, float* ay, float* az)
{
	GGVECTOR3 Vector;
	Vector.x = x2-x1;
	Vector.y = y2-y1;
	Vector.z = z2-z1;

	// Find Y and then X axis rotation
	double yangle=atan2(Vector.x, Vector.z);
	if(yangle<0.0) yangle+=D3DXToRadian(360.0);
	if(yangle>=D3DXToRadian(360.0)) yangle-=D3DXToRadian(360.0);

	GGMATRIX yrotate;
	D3DXMatrixRotationY ( &yrotate, (float)-yangle );
	D3DXVec3TransformCoord ( &Vector, &Vector, &yrotate );

	double xangle=-atan2(Vector.y, Vector.z);
	if(xangle<0.0) xangle+=D3DXToRadian(360.0);
	if(xangle>=D3DXToRadian(360.0)) xangle-=D3DXToRadian(360.0);

	*ax = wrapangleoffset(D3DXToDegree((float)xangle));
	*ay = wrapangleoffset(D3DXToDegree((float)yangle));
	*az = 0.0f;
}

// commands

void ParticleScale ( int iID, float fX, float fY, float fZ )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// scale an object by the specified scale values

	// set the new values
	m_pParticlePos->vecScale = GGVECTOR3 ( fX/100.0f, fY/100.0f, fZ/100.0f );

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticlePosition ( int iID, float fX, float fY, float fZ )
{
	// set the position of an object

	// update internal data
	if ( !UpdateParticlePtr ( iID ) )
		return;

	// set the new position
	m_pParticlePos->vecPosition = GGVECTOR3 ( fX, fY, fZ );
	m_pParticlePos->bHasBeenMovedForResponse=true;

	// apply the changes
	ParticleInternalUpdate ( iID );
}


void ParticleRotate ( int iID, float fX, float fY, float fZ )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// rotate an object around all 3 axes

	// store the new rotation values
	m_pParticlePos->vecRotate = GGVECTOR3 ( fX, fY, fZ );
	m_pParticlePos->bFreeFlightRotation = false;

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticleXRotate ( int iID, float fX )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// rotate an object around it's x axis

	// store the new rotation values
	m_pParticlePos->vecRotate = GGVECTOR3 ( fX, m_pParticlePos->vecRotate.y, m_pParticlePos->vecRotate.z );
	m_pParticlePos->bFreeFlightRotation = false;

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticleYRotate ( int iID, float fY )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// rotate an object around it's y axis

	// store the new rotation values
	m_pParticlePos->vecRotate = GGVECTOR3 ( m_pParticlePos->vecRotate.x, fY, m_pParticlePos->vecRotate.z );
	m_pParticlePos->bFreeFlightRotation = false;

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticleZRotate ( int iID, float fZ )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// rotate an object around it's z axis

	// store the new rotation values
	m_pParticlePos->vecRotate = GGVECTOR3 ( m_pParticlePos->vecRotate.x, m_pParticlePos->vecRotate.y, fZ );
	m_pParticlePos->bFreeFlightRotation = false;

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticlePoint ( int iID, float fX, float fY, float fZ )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// rotation from xyz diff
	GetAngleFromPoint ( m_pParticlePos->vecPosition.x, m_pParticlePos->vecPosition.y, m_pParticlePos->vecPosition.z,
						fX, fY, fZ, &m_pParticlePos->vecRotate.x, &m_pParticlePos->vecRotate.y, &m_pParticlePos->vecRotate.z);

	// apply the changes
	ParticleInternalUpdate ( iID );
}

void ParticleSetLook ( int iID, float fX, float fY, float fZ )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecLook.x = fX;
	m_pParticlePos->vecLook.y = fY;
	m_pParticlePos->vecLook.z = fZ;
}

void ParticleMove ( int iID, float fStep )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	// moves an object forward in it's current direction

	// update internal data
	if ( !UpdateParticlePtr ( iID ) )
		return;

	// mike : Always move INTO the Z
	m_pParticlePos->vecPosition += ( fStep * m_pParticlePos->vecLook );
	m_pParticlePos->bHasBeenMovedForResponse=true;

	// apply changes
	ParticleInternalUpdate ( iID );
}

void ParticleMoveLeft ( int iID, float fStep )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecPosition -= fStep * m_pParticlePos->vecRight;
	m_pParticlePos->bHasBeenMovedForResponse=true;

	ParticleInternalUpdate ( iID );
}

void ParticleMoveRight ( int iID, float fStep )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecPosition += fStep * m_pParticlePos->vecRight;
	m_pParticlePos->bHasBeenMovedForResponse=true;

	ParticleInternalUpdate ( iID );
}

void ParticleMoveUp ( int iID, float fStep )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecPosition += ( fStep * m_pParticlePos->vecUp );
	m_pParticlePos->bHasBeenMovedForResponse=true;

	ParticleInternalUpdate ( iID );
}

void ParticleMoveDown ( int iID, float fStep )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecPosition -= ( fStep * m_pParticlePos->vecUp );
	m_pParticlePos->bHasBeenMovedForResponse=true;

	ParticleInternalUpdate ( iID );
}

void ParticleTurnLeft ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.x -= fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticleTurnRight ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.x += fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticlePitchUp ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.y += fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticlePitchDown ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.y -= fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticleRollLeft ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.z -= fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticleRollRight ( int iID, float fAngle )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return;
	}

	m_pParticlePos->vecYawPitchRoll.z += fAngle;
	m_pParticlePos->bFreeFlightRotation = true;

	ParticleInternalUpdate ( iID );
}

void ParticleInternalUpdate ( int iID )
{
	// apply any changes to the object e.g. position

	// update internal pointer
	if ( !UpdateParticlePtr ( iID ) )
		return;

	// variable declarations
	GGMATRIX matTranslation;										// translation ( position )
	GGMATRIX matRotation, matRotateX, matRotateY, matRotateZ;	// rotation
	GGMATRIX matScale;											// scale
	bool		bUpdate = false;									// used to update look, up and right vector

	// apply scaling to the object
	D3DXMatrixScaling ( &matScale, m_pParticlePos->vecScale.x, m_pParticlePos->vecScale.y, m_pParticlePos->vecScale.z );
	
	// apply translation to the object
	D3DXMatrixTranslation ( &matTranslation, m_pParticlePos->vecPosition.x, m_pParticlePos->vecPosition.y, m_pParticlePos->vecPosition.z );

	// setup rotation matrices
	if ( m_pParticlePos->bFreeFlightRotation )
	{
		D3DXMatrixRotationYawPitchRoll ( &matRotation,	D3DXToRadian ( m_pParticlePos->vecYawPitchRoll.x ),
														D3DXToRadian ( m_pParticlePos->vecYawPitchRoll.y ),
														D3DXToRadian ( m_pParticlePos->vecYawPitchRoll.z ) ); 
	}
	else
	{
		D3DXMatrixRotationX ( &matRotateX, D3DXToRadian ( m_pParticlePos->vecRotate.x ) );	// x rotation
		D3DXMatrixRotationY ( &matRotateY, D3DXToRadian ( m_pParticlePos->vecRotate.y ) );	// y rotation
		D3DXMatrixRotationZ ( &matRotateZ, D3DXToRadian ( m_pParticlePos->vecRotate.z ) );	// z rotation

		// build final rotation matrix
		matRotation = matRotateX * matRotateY * matRotateZ;
	}

	// Apply pivot if any
	if ( m_pParticlePos->bApplyPivot )
	{
		D3DXMatrixRotationX ( &matRotateX, D3DXToRadian ( m_pParticlePos->vecPivot.x ) );	// x rotation
		D3DXMatrixRotationY ( &matRotateY, D3DXToRadian ( m_pParticlePos->vecPivot.y ) );	// y rotation
		D3DXMatrixRotationZ ( &matRotateZ, D3DXToRadian ( m_pParticlePos->vecPivot.z ) );	// z rotation

		// build final rotation matrix
		GGMATRIX matPivotRotation = matRotateX * matRotateY * matRotateZ;

		// modify current rotation
		matRotation = matPivotRotation * matRotation;
	}

	// scale first, then rotation, then translation (notran is a quick way of stripping translation for colcenter calc)
	m_pParticlePos->matObjectNoTran = matScale * matRotation;
	m_pParticlePos->matObject = m_pParticlePos->matObjectNoTran * matTranslation;

	
	// now we have to update the look, up and right vector, we do this because
	// when we want to move an object we move it using these vectors so we need
	// to update them to reflect any changes setup by the rotations

	// if any of the rotation values have changed then we must reset the look, up
	// and right vectors to their original states
	// LEEFIX - 171002 - Ensure free flight is seperate from euler
	if( m_pParticlePos->bFreeFlightRotation==false )
	{
		if ( m_pParticlePos->vecLast != m_pParticlePos->vecRotate )
		{
			// save the current rotation vector
			m_pParticlePos->vecLast = m_pParticlePos->vecRotate;

			// regenerate the look, up and right vectors
			m_pParticlePos->vecLook  = GGVECTOR3 ( 0, 0, 1 );		// look vector
			m_pParticlePos->vecUp    = GGVECTOR3 ( 0, 1, 0 );		// up vector
			m_pParticlePos->vecRight = GGVECTOR3 ( 1, 0, 0 );		// right vector
		
			// signal that we need to update the vectors
			bUpdate = true;
		}
	}
	else
	{
		// free flight always applies rotation
		bUpdate = true;
	}

	// only update if there has been a change in rotation
	if ( bUpdate )
	{
		if ( m_pParticlePos->bFreeFlightRotation )
		{
			// free flight modifies lookupright directly (uses current rotation matrix)
			D3DXVec3TransformCoord ( &m_pParticlePos->vecLook, &m_pParticlePos->vecLook,		&matRotation );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecRight,   &m_pParticlePos->vecRight,    &matRotation );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecUp,   &m_pParticlePos->vecUp,			&matRotation );
		}
		else
		{
			// x rotation
			D3DXMatrixRotationAxis ( &matRotation, &m_pParticlePos->vecRight, D3DXToRadian ( m_pParticlePos->vecRotate.x ) );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecLook, &m_pParticlePos->vecLook,  &matRotation );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecUp,   &m_pParticlePos->vecUp,    &matRotation );

			D3DXMatrixRotationAxis ( &matRotation, &GGVECTOR3 ( 0.0f, 1.0f, 0.0f ), D3DXToRadian ( m_pParticlePos->vecRotate.y ) );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecLook, &m_pParticlePos->vecLook, &matRotation );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecRight,   &m_pParticlePos->vecRight,   &matRotation );

			D3DXMatrixRotationAxis ( &matRotation, &m_pParticlePos->vecLook, D3DXToRadian ( m_pParticlePos->vecRotate.z ) );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecRight, &m_pParticlePos->vecRight, &matRotation );
			D3DXVec3TransformCoord ( &m_pParticlePos->vecUp,   &m_pParticlePos->vecUp,   &matRotation );
		}
	}

	// May need to recalc colcenter if used later
	m_pParticlePos->bColCenterUpdated=false;
}

GGVECTOR3 ParticleGetPosVec ( int iID )
{
	return m_pParticlePos->vecPosition;
}

GGVECTOR3 ParticleGetRotVec ( int iID )
{
	return m_pParticlePos->vecRotate;
}

DWORD ParticleGetXPositionEx ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return *(DWORD*)&m_pParticlePos->vecPosition.x;
}

DWORD ParticleGetYPositionEx ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}	
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return *(DWORD*)&m_pParticlePos->vecPosition.y;
}

DWORD ParticleGetZPositionEx ( int iID )
{
	// update internal pointer
	if ( !UpdateParticlePtr ( iID ) )
		return 0;

	return *(DWORD*)&m_pParticlePos->vecPosition.z;
}

DWORD ParticleGetXRotationEx ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return *(DWORD*)&m_pParticlePos->vecRotate.x;
}

DWORD ParticleGetYRotationEx ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return *(DWORD*)&m_pParticlePos->vecRotate.y;
}

DWORD ParticleGetZRotationEx ( int iID )
{
	// update internal data
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNUMBERILLEGAL);
		return 0;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_B3DMODELNOTEXISTS);
		return 0;
	}

	return *(DWORD*)&m_pParticlePos->vecRotate.z;
}

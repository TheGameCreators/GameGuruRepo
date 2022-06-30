#pragma once

#include "Windows.h"
#include "btBulletDynamicsCommon.h"
#include "directx-macros.h"

class BT2DX
{
public:
	BT2DX(void);
	~BT2DX(void);
	static GGVECTOR3 BT2DX_VECTOR3(const btVector3 &v);
	static GGQUATERNION BT2DX_QUATERNION(const btQuaternion &q);
	static GGMATRIX BT2DX_MATRIX(const btTransform &ms);
	static GGMATRIX ConvertBulletMotionState(const btMotionState &ms);
	static btVector3 DX_VECTOR3_2BT( GGVECTOR3 &v);
	static GGMATRIX ConvertBulletTransform( btTransform *bulletTransformMatrix );
	static void XPrepareMatrixFromRULP( GGMATRIX &matOutput, GGVECTOR3 *R, GGVECTOR3 *U, GGVECTOR3 *L, GGVECTOR3 *P );
};


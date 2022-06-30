///#include "StdAfx.h"
#include "BT2DX.h"


BT2DX::BT2DX(void)
{
}


BT2DX::~BT2DX(void)
{
}

GGVECTOR3 BT2DX::BT2DX_VECTOR3(const btVector3 &v)
{
	return GGVECTOR3(v.x(), v.y(), v.z());
}


btVector3 BT2DX::DX_VECTOR3_2BT( GGVECTOR3 &v)
{
	return btVector3(v.x, v.y, v.z);
}

GGQUATERNION BT2DX::BT2DX_QUATERNION(const btQuaternion &q)
{
	return GGQUATERNION(q.x(), q.y(), q.z(), q.w());
}
//Converts a Bullet Transform to a Direct X Matrix
GGMATRIX BT2DX::BT2DX_MATRIX(const btTransform &ms)
{
	btQuaternion q = ms.getRotation();
	btVector3 p = ms.getOrigin();
	GGMATRIX pos, rot, world;
	GGMatrixTranslation(&pos, p.x(), p.y(), p.z());
	GGMatrixRotationQuaternion(&rot, &BT2DX_QUATERNION(q));
	GGMatrixMultiply(&world, &rot, &pos);
	return world;
}
//Takes a Bullet Motion State and returns a Direct X Matrix
GGMATRIX BT2DX::ConvertBulletMotionState(const btMotionState &ms)
{
	btTransform world;
	ms.getWorldTransform(world);
	return BT2DX_MATRIX(world);
}

GGMATRIX BT2DX::ConvertBulletTransform( btTransform *bulletTransformMatrix )
{
   GGMATRIX world;
   btVector3 R = bulletTransformMatrix->getBasis().getColumn(0);
   btVector3 U = bulletTransformMatrix->getBasis().getColumn(1);
   btVector3 L = bulletTransformMatrix->getBasis().getColumn(2);
   btVector3 P = bulletTransformMatrix->getOrigin();

   GGVECTOR3 vR, vU, vL, vP;
   vR.x = R.x();vR.y = R.y();vR.z = R.z();
   vU.x = U.x();vU.y = U.y();vU.z = U.z();
   vL.x = L.x();vL.y = L.y();vL.z = L.z();
   vP.x = P.x();vP.y = P.y();vP.z = P.z();

   XPrepareMatrixFromRULP( world, &vR, &vU, &vL, &vP );
   return world;
}

void BT2DX::XPrepareMatrixFromRULP( GGMATRIX &matOutput, GGVECTOR3 *R, GGVECTOR3 *U, GGVECTOR3 *L, GGVECTOR3 *P )
{
    matOutput._11 = R->x;matOutput._12 = R->y;matOutput._13 = R->z;matOutput._14 = 0.f;
    matOutput._21 = U->x;matOutput._22 = U->y;matOutput._23 = U->z;matOutput._24 = 0.f;
    matOutput._31 = L->x;matOutput._32 = L->y;matOutput._33 = L->z;matOutput._34 = 0.f;
    matOutput._41 = P->x;matOutput._42 = P->y;matOutput._43 = P->z;matOutput._44 = 1.f;
}










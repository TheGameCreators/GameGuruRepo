#include "directx-macros.h"
#ifdef DX11

#include "K3D_Vector3D.h"
#include "K3D_Matrix.h"
#include <math.h>

#define PI   3.141592654f

#define DegreeToRadian( degree ) ((degree) * (PI / 180.0f))
#define RadianToDegree( radian ) ((radian) * (180.0f / PI))

namespace KMaths
{

//--------------------------
// 4D Vector
//--------------------------

Vector4::Vector4( float fx, float fy, float fz, float fw )
{
    x = fx;
    y = fy;
    z = fz;
    w = fw;
}

// assignment operators
 Vector4& Vector4::operator += ( const Vector4& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

 Vector4&
Vector4::operator -= ( const Vector4& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

 Vector4&
Vector4::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
}

 Vector4& Vector4::operator /= ( float f )
{
    float fInv = 1.0f / f;
    x *= fInv;
    y *= fInv;
    z *= fInv;
    w *= fInv;
    return *this;
}


// unary operators
 Vector4
Vector4::operator + () const
{
    return *this;
}

 Vector4
Vector4::operator - () const
{
    return Vector4(-x, -y, -z, -w);
}


// binary operators
 Vector4
Vector4::operator + ( const Vector4& v ) const
{
    return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

 Vector4
Vector4::operator - ( const Vector4& v ) const
{
    return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

 Vector4
Vector4::operator * ( float f ) const
{
    return Vector4(x * f, y * f, z * f, w * f);
}

 Vector4
Vector4::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Vector4(x * fInv, y * fInv, z * fInv, w * fInv);
}


 Vector4
operator * ( float f, const class Vector4& v )
{
    return Vector4(f * v.x, f * v.y, f * v.z, f * v.w);
}

 float Length ( const Vector4 *pV )
{
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
}

 float LengthSq ( const Vector4 *pV )
{
    return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w;
}

 float Dot ( const Vector4 *pV1, const Vector4 *pV2 )
{
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

 Vector4* Cross ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2 )
{
    Vector4 v;

#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->y * pV2->z - pV1->z * pV2->y;
    v.y = pV1->z * pV2->x - pV1->x * pV2->z;
    v.z = pV1->x * pV2->y - pV1->y * pV2->x;
    v.w = 1;

    *pOut = v;
    return pOut;
}

 Vector4* Add ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2 )
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->x + pV2->x;
    v.y = pV1->y + pV2->y;
    v.z = pV1->z + pV2->z;
    v.w = 1;
	*pOut = v;
    return pOut;
}

 Vector4* Subtract ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2 )
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->x - pV2->x;
    v.y = pV1->y - pV2->y;
    v.z = pV1->z - pV2->z;
    v.w = pV1->w - pV2->w;
	*pOut = v;
    return pOut;
}

 Vector4* Minimize ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2 )
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->x < pV2->x ? pV1->x : pV2->x;
    v.y = pV1->y < pV2->y ? pV1->y : pV2->y;
    v.z = pV1->z < pV2->z ? pV1->z : pV2->z;
    v.w = pV1->w < pV2->w ? pV1->w : pV2->w;
	*pOut = v;
    return pOut;
}

 Vector4* Maximize ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2 )
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->x > pV2->x ? pV1->x : pV2->x;
    v.y = pV1->y > pV2->y ? pV1->y : pV2->y;
    v.z = pV1->z > pV2->z ? pV1->z : pV2->z;
    v.w = pV1->w > pV2->w ? pV1->w : pV2->w;
	*pOut = v;
    return pOut;
}

 Vector4* Scale ( Vector4 *pOut, const Vector4 *pV, float s)
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV)
        return NULL;
#endif

    v.x = pV->x * s;
    v.y = pV->y * s;
    v.z = pV->z * s;
    v.w = pV->w * s;
	*pOut = v;
    return pOut;
}

 Vector4* Lerp ( Vector4 *pOut, const Vector4 *pV1, const Vector4 *pV2, float s )
{
	Vector4 v;
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->x + s * (pV2->x - pV1->x);
    v.y = pV1->y + s * (pV2->y - pV1->y);
    v.z = pV1->z + s * (pV2->z - pV1->z);
    v.w = pV1->w + s * (pV2->w - pV1->w);
	*pOut = v;
    return pOut;
}

Vector4* BaryCentric(Vector4 *pout, const Vector4 *pv1, const Vector4 *pv2, const Vector4 *pv3, float f, float g)
{
	Vector4 v;
    v.x = (1.0f-f-g) * (pv1->x) + f * (pv2->x) + g * (pv3->x);
    v.y = (1.0f-f-g) * (pv1->y) + f * (pv2->y) + g * (pv3->y);
    v.z = (1.0f-f-g) * (pv1->z) + f * (pv2->z) + g * (pv3->z);
    v.w = (1.0f-f-g) * (pv1->w) + f * (pv2->w) + g * (pv3->w);
	*pout = v;
    return pout;
}

Vector4* CatmullRom( Vector4 *pout, const Vector4 *pv0, const Vector4 *pv1, const Vector4 *pv2, const Vector4 *pv3, float s)
{
	Vector4 v;
    v.x = 0.5f * (2.0f * pv1->x + (pv2->x - pv0->x) *s + (2.0f *pv0->x - 5.0f * pv1->x + 4.0f * pv2->x - pv3->x) * s * s + (pv3->x -3.0f * pv2->x + 3.0f * pv1->x - pv0->x) * s * s * s);
    v.y = 0.5f * (2.0f * pv1->y + (pv2->y - pv0->y) *s + (2.0f *pv0->y - 5.0f * pv1->y + 4.0f * pv2->y - pv3->y) * s * s + (pv3->y -3.0f * pv2->y + 3.0f * pv1->y - pv0->y) * s * s * s);
    v.z = 0.5f * (2.0f * pv1->z + (pv2->z - pv0->z) *s + (2.0f *pv0->z - 5.0f * pv1->z + 4.0f * pv2->z - pv3->z) * s * s + (pv3->z -3.0f * pv2->z + 3.0f * pv1->z - pv0->z) * s * s * s);
    v.w = 0.5f * (2.0f * pv1->w + (pv2->w - pv0->w) *s + (2.0f *pv0->w - 5.0f * pv1->w + 4.0f * pv2->w - pv3->w) * s * s + (pv3->w -3.0f * pv2->w + 3.0f * pv1->w - pv0->w) * s * s * s);
	*pout = v;
    return pout;
}

Vector4* Hermite(Vector4 *pout, const Vector4 *pv1, const Vector4 *pt1, const Vector4 *pv2, const Vector4 *pt2, float s)
{
	Vector4 v;
    float h1, h2, h3, h4;

    h1 = 2.0f * s * s * s - 3.0f * s * s + 1.0f;
    h2 = s * s * s - 2.0f * s * s + s;
    h3 = -2.0f * s * s * s + 3.0f * s * s;
    h4 = s * s * s - s * s;

    v.x = h1 * (pv1->x) + h2 * (pt1->x) + h3 * (pv2->x) + h4 * (pt2->x);
    v.y = h1 * (pv1->y) + h2 * (pt1->y) + h3 * (pv2->y) + h4 * (pt2->y);
    v.z = h1 * (pv1->z) + h2 * (pt1->z) + h3 * (pv2->z) + h4 * (pt2->z);
    v.w = h1 * (pv1->w) + h2 * (pt1->w) + h3 * (pv2->w) + h4 * (pt2->w);
	*pout = v;
    return pout;
}

Vector4* Normalize(Vector4 *pout, const Vector4 *pv)
{
    float norm;
    norm = Length(pv);
    if ( !norm )
    {
     pout->x = 0.0f;
     pout->y = 0.0f;
     pout->z = 0.0f;
     pout->w = 0.0f;
    }
    else
    {
     pout->x = pv->x / norm;
     pout->y = pv->y / norm;
     pout->z = pv->z / norm;
     pout->w = pv->w / norm;
    }
    return pout;
}

Vector4* Transform(Vector4 *pout, const Vector4 *pv, const KMaths::Matrix *pm)
{
	// 260717 - I think D3DXVec4Transform works differently, lets try it
    //pout->x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0] * pv->w;
    //pout->y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1] * pv->w;
    //pout->z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2] * pv->w;
    //pout->w = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3] * pv->w;
	Vector4 out;
    out.x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0] * pv->w;
    out.y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1] * pv->w;
    out.z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2] * pv->w;
    out.w = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3] * pv->w;
	*pout = out;
    return pout;
}

Vector4* TransformCoord(Vector4 *pout, const Vector4 *pv, const KMaths::Matrix *pm)
{
	float x,y,z,w;

	x = pv->x * pm->_11 + pv->y * pm->_21 + pv->z * pm->_31 + pm->_41;
	y = pv->x * pm->_12 + pv->y * pm->_22 + pv->z * pm->_32 + pm->_42;
	z = pv->x * pm->_13 + pv->y * pm->_23 + pv->z * pm->_33 + pm->_43;
	w = pv->x * pm->_14 + pv->y * pm->_24 + pv->z * pm->_34 + pm->_44;

	pout->x = x / w;
	pout->y = y / w;
	pout->z = z / w;
	
    return pout;
}

Vector4* TransformNormal(Vector4 *pout, const Vector4 *pv, const KMaths::Matrix *pm)
{
	Vector4 v;
    v.x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z;
    v.y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z;
    v.z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z;
    v.w = 1;
	*pout = v;
    return pout;
}

}

#endif
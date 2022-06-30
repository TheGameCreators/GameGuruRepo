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
// 3D Vector
//--------------------------

Vector3::Vector3( const float *pf )
{
#ifdef D3DX_DEBUG
    if(!pf)
        return;
#endif

    x = pf[0];
    y = pf[1];
    z = pf[2];
}

Vector3::Vector3( float fx, float fy, float fz )
{
    x = fx;
    y = fy;
    z = fz;
}


// casting

Vector3::operator float* ()
{
    return (float *) &x;
}


Vector3::operator const float* () const
{
    return (const float *) &x;
}




// assignment operators
 Vector3& Vector3::operator += ( const Vector3& v )
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

 Vector3&
Vector3::operator -= ( const Vector3& v )
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

 Vector3&
Vector3::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

 Vector3& Vector3::operator /= ( float f )
{
    float fInv = 1.0f / f;
    x *= fInv;
    y *= fInv;
    z *= fInv;
    return *this;
}


// unary operators
 Vector3
Vector3::operator + () const
{
    return *this;
}

 Vector3
Vector3::operator - () const
{
    return Vector3(-x, -y, -z);
}


// binary operators
 Vector3
Vector3::operator + ( const Vector3& v ) const
{
    return Vector3(x + v.x, y + v.y, z + v.z);
}

 Vector3
Vector3::operator - ( const Vector3& v ) const
{
    return Vector3(x - v.x, y - v.y, z - v.z);
}

 Vector3
Vector3::operator * ( float f ) const
{
    return Vector3(x * f, y * f, z * f);
}

 Vector3
Vector3::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Vector3(x * fInv, y * fInv, z * fInv);
}


 Vector3
operator * ( float f, const class Vector3& v )
{
    return Vector3(f * v.x, f * v.y, f * v.z);
}


bool Vector3::operator == ( const Vector3& v ) const
{
    return x == v.x && y == v.y && z == v.z;
}

 bool
Vector3::operator != ( const Vector3& v ) const
{
    return x != v.x || y != v.y || z != v.z;
}

 float Length ( const Vector3 *pV )
{
#ifdef D3DX_DEBUG
    if(!pV)
        return 0.0f;
#endif

#ifdef __cplusplus
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
#else
    return (float) sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
#endif
}

  float Length4 ( const Vector4 *pV )
{
#ifdef __cplusplus
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
#else
    return (float) sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
#endif
}

 float LengthSq ( const Vector3 *pV )
{
#ifdef D3DX_DEBUG
    if(!pV)
        return 0.0f;
#endif

    return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}

 float Dot ( const Vector3 *pV1, const Vector3 *pV2 )
{
#ifdef D3DX_DEBUG
    if(!pV1 || !pV2)
        return 0.0f;
#endif

    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

 Vector3* Cross ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2 )
{
    Vector3 v;

#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    v.x = pV1->y * pV2->z - pV1->z * pV2->y;
    v.y = pV1->z * pV2->x - pV1->x * pV2->z;
    v.z = pV1->x * pV2->y - pV1->y * pV2->x;

    *pOut = v;
    return pOut;
}

 Vector3* Add ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2 )
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    pOut->x = pV1->x + pV2->x;
    pOut->y = pV1->y + pV2->y;
    pOut->z = pV1->z + pV2->z;
    return pOut;
}

 Vector3* Subtract ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2 )
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
}

 Vector3* Minimize ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2 )
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
    pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
    pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
    return pOut;
}

 Vector3* Maximize ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2 )
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
    pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
    pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
    return pOut;
}

 Vector3* Scale ( Vector3 *pOut, const Vector3 *pV, float s)
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV)
        return NULL;
#endif

    pOut->x = pV->x * s;
    pOut->y = pV->y * s;
    pOut->z = pV->z * s;
    return pOut;
}

 Vector3* Lerp ( Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2, float s )
{
#ifdef D3DX_DEBUG
    if(!pOut || !pV1 || !pV2)
        return NULL;
#endif

    pOut->x = pV1->x + s * (pV2->x - pV1->x);
    pOut->y = pV1->y + s * (pV2->y - pV1->y);
    pOut->z = pV1->z + s * (pV2->z - pV1->z);
    return pOut;
}










Vector3* BaryCentric(Vector3 *pout, const Vector3 *pv1, const Vector3 *pv2, const Vector3 *pv3, float f, float g)
{
    pout->x = (1.0f-f-g) * (pv1->x) + f * (pv2->x) + g * (pv3->x);
    pout->y = (1.0f-f-g) * (pv1->y) + f * (pv2->y) + g * (pv3->y);
    pout->z = (1.0f-f-g) * (pv1->z) + f * (pv2->z) + g * (pv3->z);
    return pout;
}


Vector3* CatmullRom( Vector3 *pout, const Vector3 *pv0, const Vector3 *pv1, const Vector3 *pv2, const Vector3 *pv3, float s)
{
    pout->x = 0.5f * (2.0f * pv1->x + (pv2->x - pv0->x) *s + (2.0f *pv0->x - 5.0f * pv1->x + 4.0f * pv2->x - pv3->x) * s * s + (pv3->x -3.0f * pv2->x + 3.0f * pv1->x - pv0->x) * s * s * s);
    pout->y = 0.5f * (2.0f * pv1->y + (pv2->y - pv0->y) *s + (2.0f *pv0->y - 5.0f * pv1->y + 4.0f * pv2->y - pv3->y) * s * s + (pv3->y -3.0f * pv2->y + 3.0f * pv1->y - pv0->y) * s * s * s);
    pout->z = 0.5f * (2.0f * pv1->z + (pv2->z - pv0->z) *s + (2.0f *pv0->z - 5.0f * pv1->z + 4.0f * pv2->z - pv3->z) * s * s + (pv3->z -3.0f * pv2->z + 3.0f * pv1->z - pv0->z) * s * s * s);
    return pout;
}

Vector3* Hermite(Vector3 *pout, const Vector3 *pv1, const Vector3 *pt1, const Vector3 *pv2, const Vector3 *pt2, float s)
{
    float h1, h2, h3, h4;

    h1 = 2.0f * s * s * s - 3.0f * s * s + 1.0f;
    h2 = s * s * s - 2.0f * s * s + s;
    h3 = -2.0f * s * s * s + 3.0f * s * s;
    h4 = s * s * s - s * s;

    pout->x = h1 * (pv1->x) + h2 * (pt1->x) + h3 * (pv2->x) + h4 * (pt2->x);
    pout->y = h1 * (pv1->y) + h2 * (pt1->y) + h3 * (pv2->y) + h4 * (pt2->y);
    pout->z = h1 * (pv1->z) + h2 * (pt1->z) + h3 * (pv2->z) + h4 * (pt2->z);
    return pout;
}

Vector3* Normalize(Vector3 *pout, const Vector3 *pv)
{
    float norm;

    norm = Length(pv);
    if ( !norm )
    {
     pout->x = 0.0f;
     pout->y = 0.0f;
     pout->z = 0.0f;
    }
    else
    {
     pout->x = pv->x / norm;
     pout->y = pv->y / norm;
     pout->z = pv->z / norm;
    }
    return pout;
}

/*
Vector4* Transform(Vector4 *pout, const Vector3 *pv, const KMaths::Matrix *pm)
{
    pout->x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0];
    pout->y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1];
    pout->z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2];
    pout->w = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3];
    return pout;
}
*/


Vector3*  TransformCoord(Vector3 *pout, const Vector3 *pv, const KMaths::Matrix *pm)
{
	float x,y,z,w;

	x = pv->x * pm->_11 + pv->y * pm->_21 + pv->z * pm->_31 + pm->_41;
	y = pv->x * pm->_12 + pv->y * pm->_22 + pv->z * pm->_32 + pm->_42;
	z = pv->x * pm->_13 + pv->y * pm->_23 + pv->z * pm->_33 + pm->_43;
	w = pv->x * pm->_14 + pv->y * pm->_24 + pv->z * pm->_34 + pm->_44;

	pout->x = x / w;
	pout->y = y / w;
	pout->z = z / w;
	
	/*
	float norm;

    norm = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] *pv->z + pm->m[3][3];

    if ( norm )
    {
     pout->x = (pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0]) / norm;
     pout->y = (pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1]) / norm;
     pout->z = (pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2]) / norm;
    }
    else
    {
     pout->x = 0.0f;
     pout->y = 0.0f;
     pout->z = 0.0f;
    }
	*/

    return pout;
}

Vector3*  TransformNormal(Vector3 *pout, const Vector3 *pv, const KMaths::Matrix *pm)
{
	float x, y, z;
    x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z;
    y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z;
    z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z;
    pout->x = x;
    pout->y = y;
    pout->z = z;
    return pout;
}

/*
Vector4::Vector4( float fx, float fy, float fz, float fw )
{
    x = fx;
    y = fy;
    z = fz;
    w = fw;
}
float Vec4Length ( const Vector4 *pV )
{
    #ifdef __cplusplus
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
#else
    return (float) sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w);
#endif
}
Vector4* Vec4Transform(Vector4 *pout, const Vector4 *pv, const Matrix *pm)
{
    Vector4 out;

	//pout->x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0] * pv->w;
    //pout->y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1] * pv->w;
    //pout->z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2] * pv->w;
    //pout->w = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3] * pv->w;

    out.x = pm->m[0][0] * pv->x + pm->m[1][0] * pv->y + pm->m[2][0] * pv->z + pm->m[3][0] * pv->w;
    out.y = pm->m[0][1] * pv->x + pm->m[1][1] * pv->y + pm->m[2][1] * pv->z + pm->m[3][1] * pv->w;
    out.z = pm->m[0][2] * pv->x + pm->m[1][2] * pv->y + pm->m[2][2] * pv->z + pm->m[3][2] * pv->w;
    out.w = pm->m[0][3] * pv->x + pm->m[1][3] * pv->y + pm->m[2][3] * pv->z + pm->m[3][3] * pv->w;
    *pout = out;
    return pout;
}
Vector4* Vec4Normalize(Vector4 *pout, const Vector4 *pv)
{
    float norm;
    norm = Vec4Length(pv);
    pout->x = pv->x / norm;
    pout->y = pv->y / norm;
    pout->z = pv->z / norm;
    pout->w = pv->w / norm;
    return pout;
}
*/

Quaternion::Quaternion( const float* pf )
{


    x = pf[0];
    y = pf[1];
    z = pf[2];
    w = pf[3];
}



Quaternion::Quaternion( float fx, float fy, float fz, float fw )
{
    x = fx;
    y = fy;
    z = fz;
    w = fw;
}


// casting

Quaternion::operator float* ()
{
    return (float *) &x;
}


Quaternion::operator const float* () const
{
    return (const float *) &x;
}


// assignment operators
 Quaternion&
Quaternion::operator += ( const Quaternion& q )
{
    x += q.x;
    y += q.y;
    z += q.z;
    w += q.w;
    return *this;
}

 Quaternion&
Quaternion::operator -= ( const Quaternion& q )
{
    x -= q.x;
    y -= q.y;
    z -= q.z;
    w -= q.w;
    return *this;
}

 Quaternion&
Quaternion::operator *= ( const Quaternion& q )
{
    QuaternionMultiply(this, this, &q);
    return *this;
}

 Quaternion&
Quaternion::operator *= ( float f )
{
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
}

 Quaternion&
Quaternion::operator /= ( float f )
{
    float fInv = 1.0f / f;
    x *= fInv;
    y *= fInv;
    z *= fInv;
    w *= fInv;
    return *this;
}


// unary operators
 Quaternion
Quaternion::operator + () const
{
    return *this;
}

 Quaternion
Quaternion::operator - () const
{
    return Quaternion(-x, -y, -z, -w);
}


// binary operators
 Quaternion
Quaternion::operator + ( const Quaternion& q ) const
{
    return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
}

 Quaternion
Quaternion::operator - ( const Quaternion& q ) const
{
    return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
}

 Quaternion
Quaternion::operator * ( const Quaternion& q ) const
{
    Quaternion qT;
    QuaternionMultiply(&qT, this, &q);
    return qT;
}

 Quaternion
Quaternion::operator * ( float f ) const
{
    return Quaternion(x * f, y * f, z * f, w * f);
}

 Quaternion
Quaternion::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Quaternion(x * fInv, y * fInv, z * fInv, w * fInv);
}


 Quaternion
operator * (float f, const Quaternion& q )
{
    return Quaternion(f * q.x, f * q.y, f * q.z, f * q.w);
}


 bool
Quaternion::operator == ( const Quaternion& q ) const
{
    return x == q.x && y == q.y && z == q.z && w == q.w;
}

 bool
Quaternion::operator != ( const Quaternion& q ) const
{
    return x != q.x || y != q.y || z != q.z || w != q.w;
}


 float QuaternionLength
    ( const Quaternion *pQ )
{

#ifdef __cplusplus
    return sqrtf(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w);
#else
    return (float) sqrt(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w);
#endif
}

 float QuaternionLengthSq
    ( const Quaternion *pQ )
{


    return pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w;
}

 float QuaternionDot
    ( const Quaternion *pQ1, const Quaternion *pQ2 )
{


    return pQ1->x * pQ2->x + pQ1->y * pQ2->y + pQ1->z * pQ2->z + pQ1->w * pQ2->w;
}


 Quaternion* QuaternionIdentity
    ( Quaternion *pOut )
{


    pOut->x = pOut->y = pOut->z = 0.0f;
    pOut->w = 1.0f;
    return pOut;
}

 bool QuaternionIsIdentity
    ( const Quaternion *pQ )
{


    return pQ->x == 0.0f && pQ->y == 0.0f && pQ->z == 0.0f && pQ->w == 1.0f;
}


 Quaternion* QuaternionConjugate
    ( Quaternion *pOut, const Quaternion *pQ )
{


    pOut->x = -pQ->x;
    pOut->y = -pQ->y;
    pOut->z = -pQ->z;
    pOut->w =  pQ->w;
    return pOut;
}


 Quaternion* QuaternionMultiply(Quaternion *pout, const Quaternion *pq1, const Quaternion *pq2)
{
    Quaternion out;

    

    out.x = pq2->w * pq1->x + pq2->x * pq1->w + pq2->y * pq1->z - pq2->z * pq1->y;
    out.y = pq2->w * pq1->y - pq2->x * pq1->z + pq2->y * pq1->w + pq2->z * pq1->x;
    out.z = pq2->w * pq1->z + pq2->x * pq1->y - pq2->y * pq1->x + pq2->z * pq1->w;
    out.w = pq2->w * pq1->w - pq2->x * pq1->x - pq2->y * pq1->y - pq2->z * pq1->z;
    *pout = out;
    return pout;
}

 Quaternion*  QuaternionSlerp(Quaternion *out, const Quaternion *q1, const Quaternion *q2, float t)
{
    float dot, temp;

    
    temp = 1.0f - t;
    dot = QuaternionDot(q1, q2);
    if (dot < 0.0f)
    {
        t = -t;
        dot = -dot;
    }

    if (1.0f - dot > 0.001f)
    {
        float theta = acosf(dot);

        temp = sinf(theta * temp) / sinf(theta);
        t = sinf(theta * t) / sinf(theta);
    }

    out->x = temp * q1->x + t * q2->x;
    out->y = temp * q1->y + t * q2->y;
    out->z = temp * q1->z + t * q2->z;
    out->w = temp * q1->w + t * q2->w;

    return out;
}

Quaternion* QuaternionRotationAxis(Quaternion* pOut, const Vector3* pV, float t)
{
	// assumes axis is already normalised
    Quaternion out;
	double s = sin(t/2.0f);
	out.x = pV->x * s;
	out.y = pV->y * s;
	out.z = pV->z * s;
	out.w = cos(t/2.0f);
    *pOut = out;
    return pOut;
}


Plane::Plane( const float* pf )
{


    a = pf[0];
    b = pf[1];
    c = pf[2];
    d = pf[3];
}



Plane::Plane( float fa, float fb, float fc, float fd )
{
    a = fa;
    b = fb;
    c = fc;
    d = fd;
}


// casting

Plane::operator float* ()
{
    return (float *) &a;
}


Plane::operator const float* () const
{
    return (const float *) &a;
}


// assignment operators
 Plane&
Plane::operator *= ( float f )
{
    a *= f;
    b *= f;
    c *= f;
    d *= f;
    return *this;
}

 Plane&
Plane::operator /= ( float f )
{
    float fInv = 1.0f / f;
    a *= fInv;
    b *= fInv;
    c *= fInv;
    d *= fInv;
    return *this;
}


// unary operators
 Plane
Plane::operator + () const
{
    return *this;
}

 Plane
Plane::operator - () const
{
    return Plane(-a, -b, -c, -d);
}


// binary operators
 Plane
Plane::operator * ( float f ) const
{
    return Plane(a * f, b * f, c * f, d * f);
}

 Plane
Plane::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Plane(a * fInv, b * fInv, c * fInv, d * fInv);
}

 Plane
operator * (float f, const Plane& p )
{
    return Plane(f * p.a, f * p.b, f * p.c, f * p.d);
}

 bool
Plane::operator == ( const Plane& p ) const
{
    return a == p.a && b == p.b && c == p.c && d == p.d;
}

 bool
Plane::operator != ( const Plane& p ) const
{
    return a != p.a || b != p.b || c != p.c || d != p.d;
}


 Plane* PlaneNormalize(Plane *out, const Plane *p)
{
    float norm;

    
    norm = sqrtf(p->a * p->a + p->b * p->b + p->c * p->c);
    if (norm)
    {
        out->a = p->a / norm;
        out->b = p->b / norm;
        out->c = p->c / norm;
        out->d = p->d / norm;
    }
    else
    {
        out->a = 0.0f;
        out->b = 0.0f;
        out->c = 0.0f;
        out->d = 0.0f;
    }

    return out;
}

 Plane* PlaneTransform(Plane *pout, const Plane *pplane, const Matrix *pm)
{
    const Plane plane = *pplane;

    pout->a = pm->m[0][0] * plane.a + pm->m[1][0] * plane.b + pm->m[2][0] * plane.c + pm->m[3][0] * plane.d;
    pout->b = pm->m[0][1] * plane.a + pm->m[1][1] * plane.b + pm->m[2][1] * plane.c + pm->m[3][1] * plane.d;
    pout->c = pm->m[0][2] * plane.a + pm->m[1][2] * plane.b + pm->m[2][2] * plane.c + pm->m[3][2] * plane.d;
    pout->d = pm->m[0][3] * plane.a + pm->m[1][3] * plane.b + pm->m[2][3] * plane.c + pm->m[3][3] * plane.d;
    return pout;
}

 Plane* PlaneFromPointNormal(Plane *pout, const Vector3 *pvpoint, const Vector3 *pvnormal)
{
    
    pout->a = pvnormal->x;
    pout->b = pvnormal->y;
    pout->c = pvnormal->z;
    pout->d = -Dot(pvpoint, pvnormal);
    return pout;
}

float PlaneDotCoord ( const Plane *pP, const Vector3 *pV )
{
    return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d;
}

 
Plane* PlaneFromPoints(Plane *pout, const Vector3 *pv1, const Vector3 *pv2, const Vector3 *pv3)
{
    Vector3 edge1, edge2, normal, Nnormal;

    edge1.x = 0.0f; edge1.y = 0.0f; edge1.z = 0.0f;
    edge2.x = 0.0f; edge2.y = 0.0f; edge2.z = 0.0f;
    Subtract(&edge1, pv2, pv1);
    Subtract(&edge2, pv3, pv1);
    Cross(&normal, &edge1, &edge2);
    Normalize(&Nnormal, &normal);
    PlaneFromPointNormal(pout, pv1, &Nnormal);
    return pout;
}


inline void CrossFast(Vector3 *pOut, const Vector3 *pV1, const Vector3 *pV2)
{
	Vector3 v;
	v.x = pV1->y * pV2->z - pV1->z * pV2->y;
	v.y = pV1->z * pV2->x - pV1->x * pV2->z;
	v.z = pV1->x * pV2->y - pV1->y * pV2->x;
	*pOut = v;
}
inline float DotFast(const Vector3 *pV1, const Vector3 *pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

//PE: This is pretty slow. use inline and no return on cross.
bool IntersectTri(const Vector3 *p0, const Vector3 *p1, const Vector3 *p2, const Vector3 *praypos, const Vector3 *praydir, float *pu, float *pv, float *pdist)
{
	// MOLLER TRUMBORE ALGORITHM
	// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-9-ray-triangle-intersection/m-ller-trumbore-algorithm/

	Vector3 edge1 = *p1 - *p0;
	Vector3 edge2 = *p2 - *p0;
	Vector3 pvec;
	CrossFast(&pvec, praydir, &edge2);

	float det = DotFast ( &edge1, &pvec);

	if (det == 0)
		return false;
	
	float invDet = 1 / det;

	Vector3 tvec = *praypos - *p0;
	*pu = DotFast(&tvec, &pvec) * invDet;
	
	if (*pu < 0 || *pu > 1)
		return false;

	Vector3 qvec;
	CrossFast(&qvec, &tvec, &edge1);
	*pv = DotFast(praydir, &qvec) * invDet;
	if (*pv < 0 || *pu + *pv > 1)
		return false;
	*pdist = DotFast(&edge2, &qvec) * invDet;

	return true;
}


float EulerWrapAngle(float Angle)
    {
        float NewAngle = fmod(Angle, 360.0f);
        if (NewAngle < 0.0f)
            NewAngle+=360.0f;
        return NewAngle;
    }

    KMaths::Vector3 GetAngleFromDirectionVector(KMaths::Vector3 Vector)
    {
	    // Find Y and then X axis rotation
	    double yangle=atan2(Vector.x, Vector.z);
	    if(yangle<0.0) yangle+=KMATHSToRadian(360.0);
	    if(yangle>=KMATHSToRadian(360.0)) yangle-=KMATHSToRadian(360.0);

	    KMaths::Matrix yrotate;
	    KMaths::MatrixRotationY ( &yrotate, (float)-yangle );
	    KMaths::TransformCoord ( &Vector, &Vector, &yrotate );

	    double xangle=-atan2(Vector.y, Vector.z);
	    if(xangle<0.0) xangle+=KMATHSToRadian(360.0);
	    if(xangle>=KMATHSToRadian(360.0)) xangle-=KMATHSToRadian(360.0);

        return KMaths::Vector3( EulerWrapAngle(KMATHSToDegree((float)xangle)),
                	        EulerWrapAngle(KMATHSToDegree((float)yangle)),
                    	    0.0f);
    }
}

#endif
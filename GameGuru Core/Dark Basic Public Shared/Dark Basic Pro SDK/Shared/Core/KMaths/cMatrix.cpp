#include "directx-macros.h"
#ifdef DX11

#include "K3D_Matrix.h"
#include <math.h>
#include <string.h>

#define PI   3.141592654f

#define DegreeToRadian( degree ) ((degree) * (PI / 180.0f))
#define RadianToDegree( radian ) ((radian) * (180.0f / PI))

namespace KMaths
{


//--------------------------
// Matrix
//--------------------------

Matrix::Matrix( const float* pf )
{
#ifdef D3DX_DEBUG
    if(!pf)
        return;
#endif

    memcpy(&_11, pf, sizeof(Matrix));
}


Matrix::Matrix( const Matrix& mat )
{
    memcpy(&_11, &mat, sizeof(Matrix));
}




Matrix::Matrix( float f11, float f12, float f13, float f14,
                        float f21, float f22, float f23, float f24,
                        float f31, float f32, float f33, float f34,
                        float f41, float f42, float f43, float f44 )
{
    _11 = f11; _12 = f12; _13 = f13; _14 = f14;
    _21 = f21; _22 = f22; _23 = f23; _24 = f24;
    _31 = f31; _32 = f32; _33 = f33; _34 = f34;
    _41 = f41; _42 = f42; _43 = f43; _44 = f44;
}



// access grants
 float&
Matrix::operator () ( unsigned int iRow, unsigned int iCol )
{
    return m[iRow][iCol];
}

 float
Matrix::operator () ( unsigned int iRow, unsigned int iCol ) const
{
    return m[iRow][iCol];
}


// casting operators

Matrix::operator float* ()
{
    return (float *) &_11;
}


Matrix::operator const float* () const
{
    return (const float *) &_11;
}


// assignment operators
 Matrix&
Matrix::operator *= ( const Matrix& mat )
{
    MatrixMultiply(this, this, &mat);
    return *this;
}

 Matrix&
Matrix::operator += ( const Matrix& mat )
{
    _11 += mat._11; _12 += mat._12; _13 += mat._13; _14 += mat._14;
    _21 += mat._21; _22 += mat._22; _23 += mat._23; _24 += mat._24;
    _31 += mat._31; _32 += mat._32; _33 += mat._33; _34 += mat._34;
    _41 += mat._41; _42 += mat._42; _43 += mat._43; _44 += mat._44;
    return *this;
}

 Matrix&
Matrix::operator -= ( const Matrix& mat )
{
    _11 -= mat._11; _12 -= mat._12; _13 -= mat._13; _14 -= mat._14;
    _21 -= mat._21; _22 -= mat._22; _23 -= mat._23; _24 -= mat._24;
    _31 -= mat._31; _32 -= mat._32; _33 -= mat._33; _34 -= mat._34;
    _41 -= mat._41; _42 -= mat._42; _43 -= mat._43; _44 -= mat._44;
    return *this;
}

 Matrix&
Matrix::operator *= ( float f )
{
    _11 *= f; _12 *= f; _13 *= f; _14 *= f;
    _21 *= f; _22 *= f; _23 *= f; _24 *= f;
    _31 *= f; _32 *= f; _33 *= f; _34 *= f;
    _41 *= f; _42 *= f; _43 *= f; _44 *= f;
    return *this;
}

 Matrix&
Matrix::operator /= ( float f )
{
    float fInv = 1.0f / f;
    _11 *= fInv; _12 *= fInv; _13 *= fInv; _14 *= fInv;
    _21 *= fInv; _22 *= fInv; _23 *= fInv; _24 *= fInv;
    _31 *= fInv; _32 *= fInv; _33 *= fInv; _34 *= fInv;
    _41 *= fInv; _42 *= fInv; _43 *= fInv; _44 *= fInv;
    return *this;
}


// unary operators
 Matrix
Matrix::operator + () const
{
    return *this;
}

 Matrix
Matrix::operator - () const
{
    return Matrix(-_11, -_12, -_13, -_14,
                      -_21, -_22, -_23, -_24,
                      -_31, -_32, -_33, -_34,
                      -_41, -_42, -_43, -_44);
}


// binary operators
 Matrix
Matrix::operator * ( const Matrix& mat ) const
{
    Matrix matT;
    MatrixMultiply(&matT, this, &mat);
    return matT;
}

 Matrix
Matrix::operator + ( const Matrix& mat ) const
{
    return Matrix(_11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
                      _21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
                      _31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
                      _41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44);
}

 Matrix
Matrix::operator - ( const Matrix& mat ) const
{
    return Matrix(_11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
                      _21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
                      _31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
                      _41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44);
}

 Matrix
Matrix::operator * ( float f ) const
{
    return Matrix(_11 * f, _12 * f, _13 * f, _14 * f,
                      _21 * f, _22 * f, _23 * f, _24 * f,
                      _31 * f, _32 * f, _33 * f, _34 * f,
                      _41 * f, _42 * f, _43 * f, _44 * f);
}

 Matrix
Matrix::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Matrix(_11 * fInv, _12 * fInv, _13 * fInv, _14 * fInv,
                      _21 * fInv, _22 * fInv, _23 * fInv, _24 * fInv,
                      _31 * fInv, _32 * fInv, _33 * fInv, _34 * fInv,
                      _41 * fInv, _42 * fInv, _43 * fInv, _44 * fInv);
}


 Matrix
operator * ( float f, const Matrix& mat )
{
    return Matrix(f * mat._11, f * mat._12, f * mat._13, f * mat._14,
                      f * mat._21, f * mat._22, f * mat._23, f * mat._24,
                      f * mat._31, f * mat._32, f * mat._33, f * mat._34,
                      f * mat._41, f * mat._42, f * mat._43, f * mat._44);
}


 bool
Matrix::operator == ( const Matrix& mat ) const
{
    return 0 == memcmp(this, &mat, sizeof(Matrix));
}

 bool
Matrix::operator != ( const Matrix& mat ) const
{
    return 0 != memcmp(this, &mat, sizeof(Matrix));
}



Matrix* MatrixIdentity ( Matrix *pOut )
{
    pOut->m[0][1] = pOut->m[0][2] = pOut->m[0][3] =
    pOut->m[1][0] = pOut->m[1][2] = pOut->m[1][3] =
    pOut->m[2][0] = pOut->m[2][1] = pOut->m[2][3] =
    pOut->m[3][0] = pOut->m[3][1] = pOut->m[3][2] = 0.0f;

    pOut->m[0][0] = pOut->m[1][1] = pOut->m[2][2] = pOut->m[3][3] = 1.0f;
    return pOut;
}

Matrix* MatrixRotationX(Matrix *pout, float angle)
{
    MatrixIdentity(pout);

	pout->m[1][1] = cosf(angle);
    pout->m[2][2] = cosf(angle);
    pout->m[1][2] = sinf(angle);
    pout->m[2][1] = -sinf(angle);
	if ( fabs(pout->m[1][1]) < 0.0001f ) pout->m[1][1] = 0.0f;
	if ( fabs(pout->m[2][2]) < 0.0001f ) pout->m[2][2] = 0.0f;
	if ( fabs(pout->m[1][2]) < 0.0001f ) pout->m[1][2] = 0.0f;
	if ( fabs(pout->m[2][1]) < 0.0001f ) pout->m[2][1] = 0.0f;
    return pout;
}

Matrix* MatrixRotationY(Matrix *pout, float angle)
{
    MatrixIdentity(pout);
    pout->m[0][0] = cosf(angle);
    pout->m[2][2] = cosf(angle);
    pout->m[0][2] = -sinf(angle);
    pout->m[2][0] = sinf(angle);
	if ( fabs(pout->m[0][0]) < 0.0001f ) pout->m[0][0] = 0.0f;
	if ( fabs(pout->m[2][2]) < 0.0001f ) pout->m[2][2] = 0.0f;
	if ( fabs(pout->m[0][2]) < 0.0001f ) pout->m[0][2] = 0.0f;
	if ( fabs(pout->m[2][0]) < 0.0001f ) pout->m[2][0] = 0.0f;
    return pout;
}

Matrix* MatrixRotationZ(Matrix *pout, float angle)
{
    MatrixIdentity(pout);
    pout->m[0][0] = cosf(angle);
    pout->m[1][1] = cosf(angle);
    pout->m[0][1] = sinf(angle);
    pout->m[1][0] = -sinf(angle);
	if ( fabs(pout->m[0][0]) < 0.0001f ) pout->m[0][0] = 0.0f;
	if ( fabs(pout->m[1][1]) < 0.0001f ) pout->m[1][1] = 0.0f;
	if ( fabs(pout->m[0][1]) < 0.0001f ) pout->m[0][1] = 0.0f;
	if ( fabs(pout->m[1][0]) < 0.0001f ) pout->m[1][0] = 0.0f;
    return pout;
}

Matrix* MatrixMultiply(Matrix *pout, const Matrix *pm1, const Matrix *pm2)
{
    Matrix out;
    int i,j;


    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            out.m[i][j] = pm1->m[i][0] * pm2->m[0][j] + pm1->m[i][1] * pm2->m[1][j] + pm1->m[i][2] * pm2->m[2][j] + pm1->m[i][3] * pm2->m[3][j];
        }
    }

    *pout = out;
    return pout;
}

Matrix* MatrixTranslation(Matrix *pout, float x, float y, float z)
{
    
    MatrixIdentity(pout);
    pout->m[3][0] = x;
    pout->m[3][1] = y;
    pout->m[3][2] = z;
    return pout;
}


Matrix* MatrixScaling(Matrix *pout, float sx, float sy, float sz)
{
    

    MatrixIdentity(pout);
    pout->m[0][0] = sx;
    pout->m[1][1] = sy;
    pout->m[2][2] = sz;
    return pout;
}

Matrix* MatrixInverse(Matrix *pout, float *pdeterminant, const Matrix *pm)
{
    float det, t[3], v[16];
    unsigned int i, j;



    t[0] = pm->m[2][2] * pm->m[3][3] - pm->m[2][3] * pm->m[3][2];
    t[1] = pm->m[1][2] * pm->m[3][3] - pm->m[1][3] * pm->m[3][2];
    t[2] = pm->m[1][2] * pm->m[2][3] - pm->m[1][3] * pm->m[2][2];
    v[0] = pm->m[1][1] * t[0] - pm->m[2][1] * t[1] + pm->m[3][1] * t[2];
    v[4] = -pm->m[1][0] * t[0] + pm->m[2][0] * t[1] - pm->m[3][0] * t[2];

    t[0] = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];
    t[1] = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
    t[2] = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
    v[8] = pm->m[3][3] * t[0] - pm->m[2][3] * t[1] + pm->m[1][3] * t[2];
    v[12] = -pm->m[3][2] * t[0] + pm->m[2][2] * t[1] - pm->m[1][2] * t[2];

    det = pm->m[0][0] * v[0] + pm->m[0][1] * v[4] +
        pm->m[0][2] * v[8] + pm->m[0][3] * v[12];
    if (det == 0.0f)
        return NULL;
    if (pdeterminant)
        *pdeterminant = det;

    t[0] = pm->m[2][2] * pm->m[3][3] - pm->m[2][3] * pm->m[3][2];
    t[1] = pm->m[0][2] * pm->m[3][3] - pm->m[0][3] * pm->m[3][2];
    t[2] = pm->m[0][2] * pm->m[2][3] - pm->m[0][3] * pm->m[2][2];
    v[1] = -pm->m[0][1] * t[0] + pm->m[2][1] * t[1] - pm->m[3][1] * t[2];
    v[5] = pm->m[0][0] * t[0] - pm->m[2][0] * t[1] + pm->m[3][0] * t[2];

    t[0] = pm->m[0][0] * pm->m[2][1] - pm->m[2][0] * pm->m[0][1];
    t[1] = pm->m[3][0] * pm->m[0][1] - pm->m[0][0] * pm->m[3][1];
    t[2] = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
    v[9] = -pm->m[3][3] * t[0] - pm->m[2][3] * t[1]- pm->m[0][3] * t[2];
    v[13] = pm->m[3][2] * t[0] + pm->m[2][2] * t[1] + pm->m[0][2] * t[2];

    t[0] = pm->m[1][2] * pm->m[3][3] - pm->m[1][3] * pm->m[3][2];
    t[1] = pm->m[0][2] * pm->m[3][3] - pm->m[0][3] * pm->m[3][2];
    t[2] = pm->m[0][2] * pm->m[1][3] - pm->m[0][3] * pm->m[1][2];
    v[2] = pm->m[0][1] * t[0] - pm->m[1][1] * t[1] + pm->m[3][1] * t[2];
    v[6] = -pm->m[0][0] * t[0] + pm->m[1][0] * t[1] - pm->m[3][0] * t[2];

    t[0] = pm->m[0][0] * pm->m[1][1] - pm->m[1][0] * pm->m[0][1];
    t[1] = pm->m[3][0] * pm->m[0][1] - pm->m[0][0] * pm->m[3][1];
    t[2] = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
    v[10] = pm->m[3][3] * t[0] + pm->m[1][3] * t[1] + pm->m[0][3] * t[2];
    v[14] = -pm->m[3][2] * t[0] - pm->m[1][2] * t[1] - pm->m[0][2] * t[2];

    t[0] = pm->m[1][2] * pm->m[2][3] - pm->m[1][3] * pm->m[2][2];
    t[1] = pm->m[0][2] * pm->m[2][3] - pm->m[0][3] * pm->m[2][2];
    t[2] = pm->m[0][2] * pm->m[1][3] - pm->m[0][3] * pm->m[1][2];
    v[3] = -pm->m[0][1] * t[0] + pm->m[1][1] * t[1] - pm->m[2][1] * t[2];
    v[7] = pm->m[0][0] * t[0] - pm->m[1][0] * t[1] + pm->m[2][0] * t[2];

    v[11] = -pm->m[0][0] * (pm->m[1][1] * pm->m[2][3] - pm->m[1][3] * pm->m[2][1]) +
        pm->m[1][0] * (pm->m[0][1] * pm->m[2][3] - pm->m[0][3] * pm->m[2][1]) -
        pm->m[2][0] * (pm->m[0][1] * pm->m[1][3] - pm->m[0][3] * pm->m[1][1]);

    v[15] = pm->m[0][0] * (pm->m[1][1] * pm->m[2][2] - pm->m[1][2] * pm->m[2][1]) -
        pm->m[1][0] * (pm->m[0][1] * pm->m[2][2] - pm->m[0][2] * pm->m[2][1]) +
        pm->m[2][0] * (pm->m[0][1] * pm->m[1][2] - pm->m[0][2] * pm->m[1][1]);

    det = 1.0f / det;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            pout->m[i][j] = v[4 * i + j] * det;

    return pout;
}

Matrix* MatrixOrthoLH(Matrix *pout, float w, float h, float zn, float zf)
{
    
    MatrixIdentity(pout);
    pout->m[0][0] = 2.0f / w;
    pout->m[1][1] = 2.0f / h;
    pout->m[2][2] = 1.0f / (zf - zn);
    pout->m[3][2] = zn / (zn - zf);
    return pout;
}

Matrix* MatrixOrthoOffCenterLH(Matrix *pout, float l, float r, float b, float t, float zn, float zf)
{
    MatrixIdentity(pout);
    pout->m[0][0] = 2.0f/(r-l);
    pout->m[1][1] = 2.0f/(t-b);
    pout->m[2][2] = 1.0f/(zf-zn);
    pout->m[3][0] = (l+r)/(l-r);
    pout->m[3][1] = (t+b)/(b-t);
    pout->m[3][2] = zn/(zn-zf);
    pout->m[3][3] = 1.0f;
    return pout;
}

Matrix* MatrixOrthoRH(Matrix *pout, float w, float h, float zn, float zf)
{
    
    MatrixIdentity(pout);
    pout->m[0][0] = 2.0f / w;
    pout->m[1][1] = 2.0f / h;
    pout->m[2][2] = 1.0f / (zn - zf);
    pout->m[3][2] = zn / (zn - zf);
    return pout;
}


Matrix*  MatrixReflect(Matrix *pout, const KMaths::Plane *pplane)
{
    KMaths::Plane Nplane;

	KMaths::PlaneNormalize(&Nplane, pplane);
    MatrixIdentity(pout);
    pout->m[0][0] = 1.0f - 2.0f * Nplane.a * Nplane.a;
    pout->m[0][1] = -2.0f * Nplane.a * Nplane.b;
    pout->m[0][2] = -2.0f * Nplane.a * Nplane.c;
    pout->m[1][0] = -2.0f * Nplane.a * Nplane.b;
    pout->m[1][1] = 1.0f - 2.0f * Nplane.b * Nplane.b;
    pout->m[1][2] = -2.0f * Nplane.b * Nplane.c;
    pout->m[2][0] = -2.0f * Nplane.c * Nplane.a;
    pout->m[2][1] = -2.0f * Nplane.c * Nplane.b;
    pout->m[2][2] = 1.0f - 2.0f * Nplane.c * Nplane.c;
    pout->m[3][0] = -2.0f * Nplane.d * Nplane.a;
    pout->m[3][1] = -2.0f * Nplane.d * Nplane.b;
    pout->m[3][2] = -2.0f * Nplane.d * Nplane.c;
    return pout;
}

Matrix*  MatrixTranspose(Matrix *pout, const Matrix *pm)
{
    const Matrix m = *pm;
    int i,j;

    
    for (i=0; i<4; i++)
        for (j=0; j<4; j++) pout->m[i][j] = m.m[j][i];

    return pout;
}


Matrix*  MatrixRotationQuaternion(Matrix *pout, const Quaternion *pq)
{

	MatrixIdentity(pout);
    pout->m[0][0] = 1.0f - 2.0f * (pq->y * pq->y + pq->z * pq->z);
    pout->m[0][1] = 2.0f * (pq->x *pq->y + pq->z * pq->w);
    pout->m[0][2] = 2.0f * (pq->x * pq->z - pq->y * pq->w);
    pout->m[1][0] = 2.0f * (pq->x * pq->y - pq->z * pq->w);
    pout->m[1][1] = 1.0f - 2.0f * (pq->x * pq->x + pq->z * pq->z);
    pout->m[1][2] = 2.0f * (pq->y *pq->z + pq->x *pq->w);
    pout->m[2][0] = 2.0f * (pq->x * pq->z + pq->y * pq->w);
    pout->m[2][1] = 2.0f * (pq->y *pq->z - pq->x *pq->w);
    pout->m[2][2] = 1.0f - 2.0f * (pq->x * pq->x + pq->y * pq->y);
    return pout;
}



Matrix* MatrixLookAtLH(Matrix *out, const KMaths::Vector3 *eye, const KMaths::Vector3 *at, const KMaths::Vector3 *up)
{
    KMaths::Vector3 right, upn, vec;

    
	KMaths::Subtract(&vec, at, eye);
    KMaths::Normalize(&vec, &vec);
    KMaths::Cross(&right, up, &vec);
    KMaths::Cross(&upn, &vec, &right);
    KMaths::Normalize(&right, &right);
    KMaths::Normalize(&upn, &upn);
    out->m[0][0] = right.x;
    out->m[1][0] = right.y;
    out->m[2][0] = right.z;
    out->m[3][0] = -KMaths::Dot(&right, eye);
    out->m[0][1] = upn.x;
    out->m[1][1] = upn.y;
    out->m[2][1] = upn.z;
    out->m[3][1] = -KMaths::Dot(&upn, eye);
    out->m[0][2] = vec.x;
    out->m[1][2] = vec.y;
    out->m[2][2] = vec.z;
    out->m[3][2] = -KMaths::Dot(&vec, eye);
    out->m[0][3] = 0.0f;
    out->m[1][3] = 0.0f;
    out->m[2][3] = 0.0f;
    out->m[3][3] = 1.0f;

    return out;
}

Matrix* MatrixLookAtRH(Matrix *out, const KMaths::Vector3 *eye, const KMaths::Vector3 *at,const KMaths::Vector3 *up)
{
    KMaths::Vector3 right, upn, vec;

    
    KMaths::Subtract(&vec, at, eye);
    KMaths::Normalize(&vec, &vec);
    KMaths::Cross(&right, up, &vec);
    KMaths::Cross(&upn, &vec, &right);
    KMaths::Normalize(&right, &right);
    KMaths::Normalize(&upn, &upn);
    out->m[0][0] = -right.x;
    out->m[1][0] = -right.y;
    out->m[2][0] = -right.z;
    out->m[3][0] = KMaths::Dot(&right, eye);
    out->m[0][1] = upn.x;
    out->m[1][1] = upn.y;
    out->m[2][1] = upn.z;
    out->m[3][1] = -KMaths::Dot(&upn, eye);
    out->m[0][2] = -vec.x;
    out->m[1][2] = -vec.y;
    out->m[2][2] = -vec.z;
    out->m[3][2] = KMaths::Dot(&vec, eye);
    out->m[0][3] = 0.0f;
    out->m[1][3] = 0.0f;
    out->m[2][3] = 0.0f;
    out->m[3][3] = 1.0f;

    return out;
}

Matrix*  MatrixPerspectiveFovLH(Matrix *pout, float fovy, float aspect, float zn, float zf)
{
    
    MatrixIdentity(pout);
    pout->m[0][0] = 1.0f / (aspect * tanf(fovy/2.0f));
    pout->m[1][1] = 1.0f / tanf(fovy/2.0f);
    pout->m[2][2] = zf / (zf - zn);
    pout->m[2][3] = 1.0f;
    pout->m[3][2] = (zf * zn) / (zn - zf);
    pout->m[3][3] = 0.0f;
    return pout;
}

Matrix*  MatrixPerspectiveFovRH(Matrix *pout, float fovy, float aspect, float zn, float zf)
{
    MatrixIdentity(pout);
    pout->m[0][0] = 1.0f / (aspect * tanf(fovy/2.0f));
    pout->m[1][1] = 1.0f / tanf(fovy/2.0f);
    pout->m[2][2] = zf / (zn - zf);
    pout->m[2][3] = -1.0f;
    pout->m[3][2] = (zf * zn) / (zn - zf);
    pout->m[3][3] = 0.0f;
    return pout;
}




Matrix*  MatrixPerspectiveLH(Matrix *pout, float w, float h, float zn, float zf)
{
    MatrixIdentity(pout);
    pout->m[0][0] = 2.0f * zn / w;
    pout->m[1][1] = 2.0f * zn / h;
    pout->m[2][2] = zf / (zf - zn);
    pout->m[3][2] = (zn * zf) / (zn - zf);
    pout->m[2][3] = 1.0f;
    pout->m[3][3] = 0.0f;
    return pout;
}


Matrix*  MatrixPerspectiveRH(Matrix *pout, float w, float h, float zn, float zf)
{
    MatrixIdentity(pout);
    pout->m[0][0] = 2.0f * zn / w;
    pout->m[1][1] = 2.0f * zn / h;
    pout->m[2][2] = zf / (zn - zf);
    pout->m[3][2] = (zn * zf) / (zn - zf);
    pout->m[2][3] = -1.0f;
    pout->m[3][3] = 0.0f;
    return pout;
}


Matrix *  MatrixRotationAxis(Matrix *out, const KMaths::Vector3 *v, float angle)
{
    KMaths::Vector3 nv;
    float sangle, cangle, cdiff;

    

    KMaths::Normalize(&nv, v);
    sangle = sinf(angle);
    cangle = cosf(angle);
    cdiff = 1.0f - cangle;

    out->m[0][0] = cdiff * nv.x * nv.x + cangle;
    out->m[1][0] = cdiff * nv.x * nv.y - sangle * nv.z;
    out->m[2][0] = cdiff * nv.x * nv.z + sangle * nv.y;
    out->m[3][0] = 0.0f;
    out->m[0][1] = cdiff * nv.y * nv.x + sangle * nv.z;
    out->m[1][1] = cdiff * nv.y * nv.y + cangle;
    out->m[2][1] = cdiff * nv.y * nv.z - sangle * nv.x;
    out->m[3][1] = 0.0f;
    out->m[0][2] = cdiff * nv.z * nv.x - sangle * nv.y;
    out->m[1][2] = cdiff * nv.z * nv.y + sangle * nv.x;
    out->m[2][2] = cdiff * nv.z * nv.z + cangle;
    out->m[3][2] = 0.0f;
    out->m[0][3] = 0.0f;
    out->m[1][3] = 0.0f;
    out->m[2][3] = 0.0f;
    out->m[3][3] = 1.0f;

    return out;
}



Matrix *  MatrixRotationYawPitchRoll(Matrix *out, float yaw, float pitch, float roll)
{
    float sroll, croll, spitch, cpitch, syaw, cyaw;


    sroll = sinf(roll);
    croll = cosf(roll);
    spitch = sinf(pitch);
    cpitch = cosf(pitch);
    syaw = sinf(yaw);
    cyaw = cosf(yaw);

    out->m[0][0] = sroll * spitch * syaw + croll * cyaw;
    out->m[0][1] = sroll * cpitch;
    out->m[0][2] = sroll * spitch * cyaw - croll * syaw;
    out->m[0][3] = 0.0f;
    out->m[1][0] = croll * spitch * syaw - sroll * cyaw;
    out->m[1][1] = croll * cpitch;
    out->m[1][2] = croll * spitch * cyaw + sroll * syaw;
    out->m[1][3] = 0.0f;
    out->m[2][0] = cpitch * syaw;
    out->m[2][1] = -spitch;
    out->m[2][2] = cpitch * cyaw;
    out->m[2][3] = 0.0f;
    out->m[3][0] = 0.0f;
    out->m[3][1] = 0.0f;
    out->m[3][2] = 0.0f;
    out->m[3][3] = 1.0f;

    return out;
}

}

#endif

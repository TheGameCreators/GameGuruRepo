
#ifndef _CMATRIX_H_
#define _CMATRIX_H_

//#include <d3dx9.h>
#include "K3D_Vector3D.h"

namespace KMaths
{

struct _Matrix 
{
    union 
	{
        struct 
		{
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;

        };

        float m [ 4 ] [ 4 ];
    };
};

struct Matrix : public _Matrix
{
	public:
		Matrix ( ) { };
		Matrix ( const float * );
		Matrix ( const Matrix& );
		Matrix ( float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44 );

		float& operator ( ) ( unsigned int Row, unsigned int Col );
		float  operator ( ) ( unsigned int Row, unsigned int Col ) const;

		operator float* ( );
		operator const float* ( ) const;

		Matrix& operator *= ( const Matrix& );
		Matrix& operator += ( const Matrix& );
		Matrix& operator -= ( const Matrix& );
		Matrix& operator *= ( float );
		Matrix& operator /= ( float );

		Matrix operator + ( ) const;
		Matrix operator - ( ) const;

		Matrix operator * ( const Matrix& ) const;
		Matrix operator + ( const Matrix& ) const;
		Matrix operator - ( const Matrix& ) const;
		Matrix operator * ( float ) const;
		Matrix operator / ( float ) const;

		friend Matrix operator * ( float, const Matrix& );

		bool operator == ( const Matrix& ) const;
		bool operator != ( const Matrix& ) const;
};

Matrix* MatrixIdentity             ( Matrix* pOut );
Matrix* MatrixRotationX            ( Matrix* pOut, float fAngle );
Matrix* MatrixRotationY            ( Matrix* pOut, float fAngle );
Matrix* MatrixRotationZ            ( Matrix* pOut, float fAngle );
Matrix* MatrixMultiply             ( Matrix* pOut, const Matrix* pMat1, const Matrix* pMat2 );
Matrix* MatrixTranslation          ( Matrix* pOut, float fX, float fY, float fZ );
Matrix* MatrixScaling              ( Matrix* pOut, float fX, float fY, float fZ );
Matrix* MatrixInverse              ( Matrix* pOut, float* pfDeterminant, const Matrix* pMatrix );
Matrix* MatrixOrthoLH              ( Matrix* pOut, float fWidth, float fHeight, float fNearZ, float fFarZ );
Matrix* MatrixOrthoRH              ( Matrix* pOut, float fWidth, float fHeight, float fNearZ, float fFarZ );
Matrix* MatrixOrthoOffCenterLH     ( Matrix* pOut, float l, float r, float b, float t, float zn, float zf );
Matrix* MatrixReflect              ( Matrix* pOut, const KMaths::Plane* pPlane );
Matrix* MatrixTranspose            ( Matrix* pOut, const Matrix* pMatrix );
Matrix* MatrixRotationQuaternion   ( Matrix* pOut, const Quaternion* pQuaternion );
Matrix* MatrixLookAtLH             ( Matrix* pOut, const KMaths::Vector3* pEye, const KMaths::Vector3* pAt, const KMaths::Vector3* pUp );
Matrix* MatrixLookAtRH             ( Matrix* pOut, const KMaths::Vector3* pEye, const KMaths::Vector3* pAt, const KMaths::Vector3* pUp );
Matrix* MatrixPerspectiveFovLH     ( Matrix* pOut, float fFOV, float fAspect, float fNearZ, float fFarZ );
Matrix* MatrixPerspectiveFovRH     ( Matrix* pOut, float fFOV, float fAspect, float fNearZ, float fFarZ );
Matrix* MatrixPerspectiveLH        ( Matrix* pOut, float fWidth, float fHeight, float fNearZ, float fFarZ );
Matrix* MatrixPerspectiveRH        ( Matrix* pOut, float fWidth, float fHeight, float fNearZ, float fFarZ);
Matrix* MatrixRotationAxis         ( Matrix* pOut, const KMaths::Vector3* vecRotation, float fAngle );
Matrix* MatrixRotationYawPitchRoll ( Matrix* pOut, float fYaw, float fPitch, float fRoll );
};

#endif
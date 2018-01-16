#ifndef _CVECTOR3D_H_
#define _CVECTOR3D_H_

#define KMATHS_PI    ( ( float ) 3.141592654f )

#define KMATHSToRadian( degree ) ( ( degree ) * ( KMATHS_PI / 180.0f ) )
#define KMATHSToDegree( radian ) ( ( radian ) * ( 180.0f / KMATHS_PI ) )

namespace KMaths
{
struct Matrix;

class Vector4
{
	public:
		Vector4 ( ) { };
		Vector4 ( float x, float y, float z, float w );

		Vector4& operator += ( const Vector4& );
		Vector4& operator -= ( const Vector4& );
		Vector4& operator *= ( float );
		Vector4& operator /= ( float );

		Vector4 operator + () const;
		Vector4 operator - () const;

		Vector4 operator + ( const Vector4& ) const;
		Vector4 operator - ( const Vector4& ) const;
		Vector4 operator * ( float ) const;
		Vector4 operator / ( float ) const;

	public:
		float x;
		float y;
		float z;
		float w;
};

class Vector3
{
	public:
		Vector3 ( )  { };
		Vector3 ( const float* );
		Vector3 ( float fX, float fY, float fZ );

		operator float* ( );
		operator const float* ( ) const;

		Vector3& operator += ( const Vector3& );
		Vector3& operator -= ( const Vector3& );
		Vector3& operator *= ( float );
		Vector3& operator /= ( float );

		Vector3 operator + () const;
		Vector3 operator - () const;

		Vector3 operator + ( const Vector3& ) const;
		Vector3 operator - ( const Vector3& ) const;
		Vector3 operator * ( float ) const;
		Vector3 operator / ( float ) const;

		friend Vector3 operator * ( float, const class Vector3& );

		bool operator == ( const Vector3& ) const;
		bool operator != ( const Vector3& ) const;

	public:
		float x;
		float y;
		float z;
};

class Vector2
{
	public:
		Vector2 ( const float* );
		Vector2 ( float fX, float fY );

		operator float* ( );
		operator const float* ( ) const;

		Vector2& operator += ( const Vector2& );
		Vector2& operator -= ( const Vector2& );
		Vector2& operator *= ( float );
		Vector2& operator /= ( float );

		Vector2 operator + () const;
		Vector2 operator - () const;

		Vector2 operator + ( const Vector2& ) const;
		Vector2 operator - ( const Vector2& ) const;
		Vector2 operator * ( float ) const;
		Vector2 operator / ( float ) const;

	public:
		float x;
		float y;
};


struct Quaternion
{

public:
    Quaternion ( ) {}
    Quaternion ( const float * );
    Quaternion ( float fX, float fY, float fZ, float fW );

    operator float* ( );
    operator const float* ( ) const;

    Quaternion& operator += ( const Quaternion& );
    Quaternion& operator -= ( const Quaternion& );
    Quaternion& operator *= ( const Quaternion& );
    Quaternion& operator *= ( float );
    Quaternion& operator /= ( float );

    Quaternion  operator + ( ) const;
    Quaternion  operator - ( ) const;

    Quaternion operator + ( const Quaternion& ) const;
    Quaternion operator - ( const Quaternion& ) const;
    Quaternion operator * ( const Quaternion& ) const;
    Quaternion operator * ( float ) const;
    Quaternion operator / ( float ) const;

    friend Quaternion operator* ( float, const Quaternion& );

    bool operator == ( const Quaternion& ) const;
    bool operator != ( const Quaternion& ) const;

    float x, y, z, w;
};


struct Plane
{

public:
    Plane ( ) { }
    Plane ( const float* );
    Plane ( float a, float b, float c, float d );

    operator float* ( );
    operator const float* ( ) const;

    Plane& operator *= ( float );
    Plane& operator /= ( float );

    Plane operator + ( ) const;
    Plane operator - ( ) const;

    Plane operator * ( float ) const;
    Plane operator / ( float ) const;

    friend Plane operator * ( float, const Plane& );

    bool operator == ( const Plane& ) const;
    bool operator != ( const Plane& ) const;

	float a, b, c, d;
};

Vector3*	BaryCentric				( Vector3* pOut, const Vector3* pV1, const Vector3* pV2, const Vector3* pV3, float f, float g );
Vector3*	TransformNormal			( Vector3* pOut, const Vector3* pV, const KMaths::Matrix* pMatrix );
Vector3*	TransformCoord			( Vector3* pOut, const Vector3* pV, const KMaths::Matrix* pMatrix );
Vector4*	Transform				( Vector4* pOut, const Vector3* pV, const KMaths::Matrix* pMatrix );
Vector3*	Normalize				( Vector3* pOut, const Vector3* pV );
Vector3*	Hermite					( Vector3* pOut, const Vector3* pV1, const Vector3* pT1, const Vector3* pV2, const Vector3* pT2, float s );
Vector3*	CatmullRom				( Vector3* pOut, const Vector3* pV0, const Vector3* pV1, const Vector3* pV2, const Vector3* pV3, float s );
Vector3*	BaryCentric				( Vector3* pOut, const Vector3* pV1, const Vector3* pV2, const Vector3* pV3, float f, float g );
Vector3*	Lerp					( Vector3* pOut, const Vector3* pV1, const Vector3* pV2, float s );
Vector3*	Scale					( Vector3* pOut, const Vector3* pV, float fS );
Vector3*	Maximize				( Vector3* pOut, const Vector3* pV1, const Vector3* pV2 );
Vector3*	Minimize				( Vector3* pOut, const Vector3* pV1, const Vector3* pV2 );
Vector3*	Subtract				( Vector3* pOut, const Vector3* pV1, const Vector3* pV2 );
Vector3*	Add						( Vector3* pOut, const Vector3* pV1, const Vector3* pV2 );
Vector3*	Cross					( Vector3* pOut, const Vector3* pV1, const Vector3* pV2 );
float		Dot						( const Vector3* pV1, const Vector3* pV2 );
float		LengthSq				( const Vector3* pV );
float		Length					( const Vector3* pV );
float		Length4					( const Vector4* pV );
///Vector4*	Vec4Transform			( Vector4* pOut, const Vector4* pV, const Matrix* pMatrix );
///Vector4*	Vec4Normalize			( Vector4* pOut, const Vector4* pV );
///float		Vec4Dot					( Vector4* pV1, const Vector4* pV2 );
Vector4*	Transform				( Vector4* pOut, const Vector4* pV, const Matrix* pMatrix );
Vector4*	Normalize				( Vector4* pOut, const Vector4* pV );
float		Dot						( Vector4* pV1, const Vector4* pV2 );

Quaternion* QuaternionMultiply		( Quaternion* pOut, const Quaternion* pQ1, const Quaternion* pQ2);
Quaternion* QuaternionSlerp			( Quaternion* pOut, const Quaternion* pQ1, const Quaternion* pQ2, float t );
Plane*		PlaneNormalize			( Plane* pOut, const Plane* pPlane );
Plane*		PlaneTransform			( Plane* pOut, const Plane* pPlane, const Matrix* pMatrix );
Plane*		PlaneFromPointNormal	( Plane* pOut, const Vector3* pVPoint, const Vector3* pVNormal );
float		PlaneDotCoord			( const Plane* pP, const Vector3* pV );
Plane*		PlaneFromPoints			( Plane* pOut, const Vector3* pV1, const Vector3* pV2, const Vector3* pV3 );
bool		IntersectTri			( const Vector3* p0, const Vector3* p1, const Vector3* p2, const Vector3* pRayPos, const Vector3* pRayDir, float* pu, float* pv, float* pdist );

Quaternion* QuaternionRotationAxis	( Quaternion* pOut, const Vector3* pV, float t );

};

#endif
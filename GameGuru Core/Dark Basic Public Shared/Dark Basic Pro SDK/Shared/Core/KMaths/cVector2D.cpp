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
// 2D Vector
//--------------------------

Vector2::Vector2 ( float x1, float y1 )
{
	// assign values to x and y points

	x = x1;
	y = y1;
}

// assignment operators
 Vector2& Vector2::operator += ( const Vector2& v )
{
    x += v.x;
    y += v.y;
    return *this;
}

 Vector2&
Vector2::operator -= ( const Vector2& v )
{
    x -= v.x;
    y -= v.y;
    return *this;
}

 Vector2&
Vector2::operator *= ( float f )
{
    x *= f;
    y *= f;
    return *this;
}

 Vector2& Vector2::operator /= ( float f )
{
    float fInv = 1.0f / f;
    x *= fInv;
    y *= fInv;
    return *this;
}


// unary operators
 Vector2
Vector2::operator + () const
{
    return *this;
}

 Vector2
Vector2::operator - () const
{
    return Vector2(-x, -y);
}


// binary operators
 Vector2
Vector2::operator + ( const Vector2& v ) const
{
    return Vector2(x + v.x, y + v.y);
}

 Vector2
Vector2::operator - ( const Vector2& v ) const
{
    return Vector2(x - v.x, y - v.y);
}

 Vector2
Vector2::operator * ( float f ) const
{
    return Vector2(x * f, y * f);
}

 Vector2
Vector2::operator / ( float f ) const
{
    float fInv = 1.0f / f;
    return Vector2(x * fInv, y * fInv);
}


 Vector2
operator * ( float f, const class Vector2& v )
{
    return Vector2(f * v.x, f * v.y);
}

 float Length ( const Vector2 *pV )
{
    return sqrtf(pV->x * pV->x + pV->y * pV->y);
}

 float LengthSq ( const Vector2 *pV )
{
    return pV->x * pV->x + pV->y * pV->y;
}

 float Dot ( const Vector2 *pV1, const Vector2 *pV2 )
{
    return pV1->x * pV2->x + pV1->y * pV2->y;
}

/*
Vector2 Vector2::operator + ( Vector2& vector )
{
	// add one vector to another

	return Vector2 ( x + vector.x, y + vector.y );
}

Vector2 Vector2::operator - ( Vector2& vector )
{
	// subtract one vector from another

	return Vector2 ( x - vector.x, y - vector.y );
}

Vector2 Vector2::operator * ( float fScalar )
{
	// multiply a vector by a scalar

	return Vector2 ( x * fScalar, y * fScalar );
}

Vector2 Vector2::operator / ( float fScalar )
{
	return Vector2 ( x / fScalar, y / fScalar );
}

float Vector2::GetLength ( void )
{
	// get the length of a vector

	return ( float ) sqrt ( ( x * x ) + ( y * y ) );
}

float Vector2::GetDistance ( Vector2& vector )
{
	// return distance from two vectors

	// rebuild the current vector and subtract
	// from it the vector passed in
	Vector2 result = Vector2 ( x, y ) - vector;

	// return the length of this vector
	return result.GetLength ( );
}

float Vector2::GetDotProduct ( Vector2& vector )
{
	Vector2 a ( x, y );
	Vector2 b ( vector.x, vector.y );

	return ( a.x * b.x ) + ( a.y * b.y );
}

float Vector2::GetAngleBetween ( Vector2& vector )
{
	// determine angle between two vectors, will return
	// in the range of 0 - 180

	// set up our vectors
	Vector2 a ( x, y );
	Vector2 b ( vector.x, vector.y );

	// get the dot product
	float dot    = ( a.x * b.x ) + ( a.y * b.y );

	// now get cosine of angle and divide this
	// by lengths of our vectors so we normalise the result
	float cosine = dot / ( a.GetLength ( ) * b.GetLength ( ) );

	// determine true angle
	float angle  = RadianToDegree ( acos ( cosine ) );

	// return our angle
	return angle;
}

float Vector2::GetAngleBetweenEx ( Vector2& vector )
{
	Vector2 a ( x, y );
	Vector2 b ( vector.x, vector.y );

	a.Normalize ( );
	b.Normalize ( );

	float dot    = ( a.x * b.x ) + ( a.y * b.y );
	float angle  = RadianToDegree ( acos ( dot ) );

	return angle;
}

float Vector2::GetFullAngleBetween ( Vector2& vector )
{
	// get an angle between 0 - 360 for our vectors

	// declare our current vector, our vector passed
	// in and a vector pointing 90 degrees to the left
	Vector2 a ( x, y );
	Vector2 b ( vector.x, vector.y );
	Vector2 c ( -1, 0 );

	// find angle between a and b
	float angle1 = a.GetAngleBetween ( b );

	// find angle between b and c
	float angle2 = b.GetAngleBetween ( c );

	// final angle
	float angle3 = angle1;

	// if angle between b and c is less than 90 then
	// deduct our angle between a and b from 360, if
	// this is not the case the
	if ( angle2 < 90.0f )
	{
		angle3 = 360 - angle1;
	}
	
	return angle3;
}

void Vector2::Normalize ( void )
{
	// create a unit vector

	Vector2 original ( *this );

	x = x / original.GetLength ( );
	y = y / original.GetLength ( );
}

void Vector2::Inverse ( void )
{
	x *= -1;
	y *= -1;
}

void Vector2::Rotate ( float fDegree ) 
{
	// to rotate around the origin of the object we must
	// translate the object to its origin ( 0, 0 ), perform
	// the rotation and then translate it back to its
	// centre point

	float fRadian = DegreeToRadian ( fDegree );

	float fOldX = x;
	float fOldY = y;

	x = 0;
	y = 0;

	float tx = ( x * cosf ( fRadian ) ) - ( y * sinf ( fRadian ) );
	float ty = ( x * sinf ( fRadian ) ) + ( y * cosf ( fRadian ) );
  
	//x = tx;
	//y = ty;

	x = tx + fOldX;
	y = ty + fOldY;
}

void Vector2::Rotate ( float fDegree, Vector2 point )
{
	// this function will rotate our vector around the specified point

	// convert our degree to radian
	float fRadian = DegreeToRadian ( fDegree );
	float fNewX   = 0.0f;
	float fNewY   = 0.0f;

	// translate our position to our new origin
	x = x - point.x;
	y = y - point.y;

	// work out new x and y positions
	fNewX = ( x * cosf ( fRadian ) ) - ( y * sinf ( fRadian ) );
	fNewY = ( x * sinf ( fRadian ) ) + ( y * cosf ( fRadian ) );
  
	// take our new positions and translate back to
	// the original position to determine the final
	// location for these points
	x = fNewX + point.x;
	y = fNewY + point.y;
}
*/

}

#endif


#ifndef _CVECTOR2D_H_
#define _CVECTOR2D_H_

class cVector2D
{
	// this is a simple vector class for 2D vectors

	public:

		// our member variables
		float x;
		float y;

	public:

		// constructors
		cVector2D ( );
		cVector2D ( float x1, float y1 );

		// operators
		cVector2D& operator = ( cVector2D& vector );
		cVector2D  operator + ( cVector2D& vector );
		cVector2D  operator - ( cVector2D& vector );
		cVector2D  operator * ( float fScalar );
		cVector2D  operator / ( float fScalar );

		// functions
		float GetLength           ( void );
		float GetDistance         ( cVector2D& vector );

		float GetDotProduct       ( cVector2D& vector );
		float GetAngleBetween     ( cVector2D& vector );
		float GetAngleBetweenEx   ( cVector2D& vector );
		float GetFullAngleBetween ( cVector2D& vector );

		void  Normalize   ( void );
		void  Inverse     ( void );

		void Rotate ( float fDegree );
		void Rotate ( float fDegree, cVector2D point );
};

#endif
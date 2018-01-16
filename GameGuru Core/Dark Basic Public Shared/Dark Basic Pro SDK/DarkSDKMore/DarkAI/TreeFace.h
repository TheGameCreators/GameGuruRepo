#ifndef TREEFACE_H
#define TREEFACE_H

#include "Point.h"
#include "Vector.h"

//stripped down face data for collision only, vertex position + face normal + face plane distance d
class TreeFace
{
    public:    
        Point vert1,vert2,vert3;
        Vector normal;
        float d;
        //bool collisionon;
        int iID;
        TreeFace* nextFace;

		TreeFace( ) { };
		virtual ~TreeFace( ) { };

		bool MakeFace( Point *p1, Point *p2, Point *p3, int id );
        
        virtual bool Intersects( const Point* p, const Vector* v, int dir, float *dist ) const;
		bool PointInPoly( const Point* p ) const;
		
    private:
};

#endif

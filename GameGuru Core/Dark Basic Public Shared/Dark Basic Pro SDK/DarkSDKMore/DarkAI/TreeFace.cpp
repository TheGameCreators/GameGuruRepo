#include "TreeFace.h"

bool TreeFace::MakeFace( Point *p1, Point *p2, Point *p3, int id )
{
	iID = id;
	
	vert1 = *p1;
	vert2 = *p2;
	vert3 = *p3;

	Vector v1(p1,p3);
	Vector v2(p1,p2);

	normal = v1.crossProduct(&v2);
	float length = normal.size();

	//zero area polygons will have an undefined normal, which will cause problems later
	//so if any exist remove them
	if (length<0.00001f) return false;
    
	normal.mult(1.0f/length);

	v1 = normal;
	v2.set(p2->x,p2->y,p2->z);
    
	d = -1.0f*(v1.dotProduct(&v2));

	return true;
}

bool TreeFace::Intersects(const Point* p, const Vector* v, int dir, float *dist ) const
{
    //if (!collisionon) return false;

	float dist1,dist2;
    Point intersect;
	                                
	dist1 = normal.x*(p->x+v->x) + normal.y*(p->y+v->y) + normal.z*(p->z+v->z) + d;
    dist2 = normal.x*p->x + normal.y*p->y + normal.z*p->z + d;
    
    //if ( dist2*dist1 > 0 ) return false;
	if ( dist2 < 0 || dist1 > 0 ) return false;
                
    dist1 = -(normal.x*v->x) - (normal.y*v->y) - (normal.z*v->z);
    if ( dist1 < -0.00001 || dist1 > 0.00001 ) 
	{
		dist1=dist2/dist1;
	}
    else 
	{
		//0 if ray is parallel to face
		//dist1=0;
		return false;
	}

	//if there is already a closer intersection, ignore 
	if ( dist && dist1 > *dist && *dist >= 0 ) return false;
    
    intersect.x = p->x + v->x*dist1;
    intersect.y = p->y + v->y*dist1;
    intersect.z = p->z + v->z*dist1;
                
    if ( !PointInPoly( &intersect ) ) return false;

	//set this point as the closest intersection
	if ( dist ) *dist = dist1;

    return true;
}

inline bool TreeFace::PointInPoly( const Point* p) const
{
    //First side
    Vector v3(&vert1,&vert2);
    v3 = v3.crossProduct(&normal);
    
    float ld = v3.x*vert1.x + v3.y*vert1.y + v3.z*vert1.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001f) return false;
    
    //Second side
    v3.set(&vert2,&vert3);
    v3 = v3.crossProduct(&normal);
    
    ld = v3.x*vert2.x + v3.y*vert2.y + v3.z*vert2.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001f) return false;
    
    //Third side
    v3.set(&vert3,&vert1);
    v3 = v3.crossProduct(&normal);
    
    ld = v3.x*vert3.x + v3.y*vert3.y + v3.z*vert3.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001f) return false;
    
    return true;
}
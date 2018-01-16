#ifndef BOX_H
#define BOX_H

#define L 1000000000
#define S -1000000000

#include "Point.h"
#include "Vector.h"

class Box
{

    public:
    
        //Point lastNormal;
        
        Box(Point* min, Point* max);
        Box(float x=L,float y=L,float z=L,float x2=S,float y2=S,float z2=S);
        ~Box() {}
        
        void ResetBox();        
        void Set(Point* min, Point* max);
        void Set(float x,float y,float z,float x2,float y2,float z2);
        
        void CorrectBox();        
        
        bool InBox(Point* p) const;        
        int IntersectBox(const Point* p, const Vector* vi, float* dist) const;

		bool CollidesFace(Point* p1, Point* p2, Point* p3);
                
        float maxbx() const { return maxx; }
        float maxby() const { return maxy; }
        float maxbz() const { return maxz; }
        float minbx() const { return minx; }
        float minby() const { return miny; }
        float minbz() const { return minz; }
        
        void maxbx(float x) { maxx = x; }
        void maxby(float y) { maxy = y; }
        void maxbz(float z) { maxz = z; }
        void minbx(float x) { minx = x; }
        void minby(float y) { miny = y; }
        void minbz(float z) { minz = z; }
    
    private:
    
        float maxx,maxy,maxz,minx,miny,minz;
};



inline Box::Box(Point* min, Point* max)
{
    minx = min->x; miny = min->y; minz = min->z;
    maxx = max->x; maxy = max->y; maxz = max->z;
    CorrectBox();
}

inline Box::Box(float x,float y,float z,float x2,float y2,float z2)
{
    minx = x; miny = y; minz = z;
    maxx = x2; maxy = y2; maxz = z2;
    CorrectBox();
}

inline void Box::ResetBox()
{
    minx = L; miny = L; minz = L;
    maxx = S; maxy = S; maxz = S;
}

inline void Box::Set(Point* min, Point* max)
{
    minx = min->x; miny = min->y; minz = min->z;
    maxx = max->x; maxy = max->y; maxz = max->z;
}            

inline void Box::Set(float x,float y,float z,float x2,float y2,float z2)
{
    minx = x; miny = y; minz = z;
    maxx = x2; maxy = y2; maxz = z2;
}

inline void Box::CorrectBox() 
{ 
    minx=minx-0.0001f; miny=miny-0.0001f; minz=minz-0.0001f; 
    maxx=maxx+0.0001f; maxy=maxy+0.0001f; maxz=maxz+0.0001f;
}

inline bool Box::InBox(Point* p) const { 
    if (p->x<minx || p->x>maxx || p->y<miny || p->y>maxy || p->z<minz || p->z>maxz) return false;         
    return true;
}

inline int Box::IntersectBox(const Point* p, const Vector* vi, float* dist) const
{        
    float tminx,tmaxx,tminy,tmaxy;
    int side=0;           

    if (vi->x>=0) {
        tminx = (minx - p->x) *vi->x;
        tmaxx = (maxx - p->x) *vi->x;
    }
    else {
        tmaxx = (minx - p->x) *vi->x;
        tminx = (maxx - p->x) *vi->x;
    }
    
    if (vi->y>=0) {
        tminy = (miny - p->y) *vi->y;
        tmaxy = (maxy - p->y) *vi->y;
    }
    else {
        tmaxy = (miny - p->y) *vi->y;
        tminy = (maxy - p->y) *vi->y;
    }
               
    if ((tminx > tmaxy) || (tminy > tmaxx)) return 0;
    
    if (tminy>tminx) tminx = tminy;
    if (tmaxy<tmaxx) tmaxx = tmaxy;
    
    if (vi->z>=0) {
        tminy = (minz - p->z) *vi->z;
        tmaxy = (maxz - p->z) *vi->z;
    }
    else {
        tmaxy = (minz - p->z) *vi->z;
        tminy = (maxz - p->z) *vi->z;
    }
    
    if ((tminx > tmaxy) || (tminy > tmaxx)) return 0;
                                      
    if (tminy>tminx) tminx = tminy;
    if (tmaxy<tmaxx) tmaxx = tmaxy;
    
    if (tmaxx<0.0) return 0;
    if (tminx>1.0) return 0;
    
	if ( dist ) *dist = tminx;
    return 1;
}

inline bool Box::CollidesFace(Point* p1, Point* p2, Point* p3)
{
    //check if the polygon is completely on one side of the box
    if (p1->x<minx && p2->x<minx && p3->x<minx) return false;
    if (p1->y<miny && p2->y<miny && p3->y<miny) return false;
    if (p1->z<minz && p2->z<minz && p3->z<minz) return false;
    
    if (p1->x>maxx && p2->x>maxx && p3->x>maxx) return false;
    if (p1->y>maxy && p2->y>maxy && p3->y>maxy) return false;
    if (p1->z>maxz && p2->z>maxz && p3->z>maxz) return false;
    
    //check which vertices of the triangle are inside the box
    int count = 0;
    if (InBox(p1)) count++;
    if (InBox(p2)) count++;
    if (InBox(p3)) count++;
    
    //3 inside = completely inside, collision
    //0 inside = might straddle box, need more checks
    //1 or 2   = must intersect box
    //if (count==3) return false;
    if (count>0) return true;
    
    float dist1;
    //check the polygon edges for intersection with the box
    Vector temp(1/(p2->x-p1->x),1/(p2->y-p1->y),1/(p2->z-p1->z));
    if (IntersectBox(p1,&temp,&dist1) && dist1>=0) return true;
    
    temp.set(1/(p3->x-p2->x),1/(p3->y-p2->y),1/(p3->z-p2->z));
    if (IntersectBox(p2,&temp,&dist1) && dist1>=0) return true;
    
    temp.set(1/(p1->x-p3->x),1/(p1->y-p3->y),1/(p1->z-p3->z));
    if (IntersectBox(p3,&temp,&dist1) && dist1>=0) return true;
    
    //remaining case - triangle completely surrounds box, if so
    //box diagonals would intersect triangle
    
    //only check the box diagonals (4 of them)
    //get poly normal
    temp.set(p1,p2);  
    Vector normal(p1,p3);          
    normal = temp.crossProduct(&normal);
    
    //check for 'point' triangles
    float distv = normal.x*normal.x + normal.y*normal.y + normal.z*normal.z;
    if (distv<0.00001) return false;
    //calculate d
    distv = normal.x*p1->x + normal.y*p1->y + normal.z*p1->z;
    
    //work out the normal for each side of the poly, stored in temp, temp2 and temp3
    temp = temp.crossProduct(&normal);        
    dist1 = temp.x*p1->x + temp.y*p1->y + temp.z*p1->z;
    
    Vector temp2(p2,p3);
    temp2 = temp2.crossProduct(&normal);        
    float dist2 = temp2.x*p2->x + temp2.y*p2->y + temp2.z*p2->z;
    
    Vector temp3(p3,p1);
    temp3 = temp3.crossProduct(&normal);        
    float dist3 = temp3.x*p3->x + temp3.y*p3->y + temp3.z*p3->z;
    
    //do some precalculation
    float b1 = minx*normal.x; float b4 = maxx*normal.x;
    float b2 = miny*normal.y; float b5 = maxy*normal.y;
    float b3 = minz*normal.z; float b6 = maxz*normal.z;
    
    float d1,d2,d3,d4,d5;
    Point intersect;
    
    //diagonal 1
    d1 = b1 + b2 + b3 - distv;
    d2 = b4 + b5 + b6 - distv;
    if (d1*d2<0) {
        //find intersection point and check if inside triangle
        d3 = -d1/((b4-b1) + (b5-b2) + (b6-b3));
        intersect.x = minx + (maxx-minx)*d3;
        intersect.y = miny + (maxy-miny)*d3;
        intersect.z = minz + (maxz-minz)*d3;
        d3 = temp.x*intersect.x + temp.y*intersect.y + temp.z*intersect.z - dist1;
        d4 = temp2.x*intersect.x + temp2.y*intersect.y + temp2.z*intersect.z - dist2;
        d5 = temp3.x*intersect.x + temp3.y*intersect.y + temp3.z*intersect.z - dist3;
        if (d3<-0.00001 && d4<-0.00001 && d5<-0.00001) return true;
    }
    
    //diagonal 2 - re-use previous calculation
    d1 = d1 - b2 + b5;
    d2 = d2 - b5 + b2;
    if (d1*d2<0) {
        d3 = -d1/((b4-b1) + (b2-b5) + (b6-b3));
        intersect.x = minx + (maxx-minx)*d3;
        intersect.y = maxy + (miny-maxy)*d3;
        intersect.z = minz + (maxz-minz)*d3;
        d3 = temp.x*intersect.x + temp.y*intersect.y + temp.z*intersect.z - dist1;
        d4 = temp2.x*intersect.x + temp2.y*intersect.y + temp2.z*intersect.z - dist2;
        d5 = temp3.x*intersect.x + temp3.y*intersect.y + temp3.z*intersect.z - dist3;
        if (d3<-0.00001 && d4<-0.00001 && d5<-0.00001) return true;
    }
    
    //diagonal 3
    d1 = d1 - b3 + b6;
    d2 = d2 - b6 + b3;
    if (d1*d2<0) {
        d3 = -d1/((b4-b1) + (b2-b5) + (b3-b6));
        intersect.x = minx + (maxx-minx)*d3;
        intersect.y = maxy + (miny-maxy)*d3;
        intersect.z = maxz + (minz-maxz)*d3;
        d3 = temp.x*intersect.x + temp.y*intersect.y + temp.z*intersect.z - dist1;
        d4 = temp2.x*intersect.x + temp2.y*intersect.y + temp2.z*intersect.z - dist2;
        d5 = temp3.x*intersect.x + temp3.y*intersect.y + temp3.z*intersect.z - dist3;
        if (d3<-0.00001 && d4<-0.00001 && d5<-0.00001) return true;
    }
    
    //diagonal 4
    d1 = d1 - b5 + b2;
    d2 = d2 - b2 + b5;
    if (d1*d2<0) {
        d3 = -d1/((b4-b1) + (b5-b2) + (b3-b6));
        intersect.x = minx + (maxx-minx)*d3;
        intersect.y = miny + (maxy-miny)*d3;
        intersect.z = maxz + (minz-maxz)*d3;
        d3 = temp.x*intersect.x + temp.y*intersect.y + temp.z*intersect.z - dist1;
        d4 = temp2.x*intersect.x + temp2.y*intersect.y + temp2.z*intersect.z - dist2;
        d5 = temp3.x*intersect.x + temp3.y*intersect.y + temp3.z*intersect.z - dist3;
        if (d3<-0.00001 && d4<-0.00001 && d5<-0.00001) return true;
    }
    
    return false;
}

#endif

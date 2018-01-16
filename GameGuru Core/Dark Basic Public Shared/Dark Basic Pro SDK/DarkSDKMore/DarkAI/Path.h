#ifndef H_PATH
#define H_PATH

#include <vector>
#include <cctype>
#include <math.h>

struct sPoint
{
    float x, y; //y is z in 3D space
	float realy;
	int container;

	bool operator==(sPoint p) const {
		return ( fabs( x - p.x ) < 0.000001 ) && ( fabs( y - p.y ) < 0.000001 );
	}
};

//path simply stores a sequence of points in a vector.
//allows points to be inserted in the middle easily
class Path
{
  private:
     
	std::vector < sPoint > sPoint_list;
    float iSphereRadius;
    
    float fPathLength;

	int iPathObject;
    
  public:  

    static float fDebugObjHeight;
    
    Path ( );
    ~Path ( );
    
    int    CountPoints ( );
    
    sPoint GetPoint ( int iIndex );
	sPoint GetLast ( );
    void AddPoint ( float fX, float fY, float fZ );					//adds a point to the end of the vector
	void AddPoint ( float fX, float fY, float fZ, int container );
    void InsertPoint ( int iIndex, float fX, float fY, int container );	//inserts a point just before the index specified
	void SwapPoints ( int iIndex1, int iIndex2 );
	bool Contains ( float fX, float fY );
    
    void CalculateLength ( );
	void CalculateSqrLength ( );
    float GetLength ( );
    
    //removes all points from the vector
    void Clear ( );
	void RemoveLast ( );
	void RemoveFirst ( );
    
    //draw the path with circles at it's nodes
    void DebugDraw ( float radius );
	void DebugHide ( );
    
};

#endif

#ifndef H_POLYGON
#define H_POLYGON

#include <vector>
#include <cctype>

struct sVertex
{
    float x, y;
};

//holds a vector of vertices of any number
//vertices must be defined in a clockwise order for the current pathfinder to work
class Polygon2D
{
  private:
     
	std::vector < sVertex > sVertex_list;
  
  public: 

    bool bHalfHeight;
	int id;
	bool bTerrain;
	bool bDiveOver;
    
    Polygon2D ( );
    ~Polygon2D ( );
    
    int CountVertices ( );
    void Clear ( );
    
    void AddVertex ( float fX, float fY );
    void AddVertex ( sVertex* pVertex );
    sVertex GetVertex ( int iIndex );
    
    void Draw ( );
    
};

#endif

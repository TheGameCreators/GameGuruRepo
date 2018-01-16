#ifndef TREEFACE_H
#define TREEFACE_H

#include "Box.h"
#include "Lumel.h"
#include "CollisionTreeLightmapper.h"
#include "directx-macros.h"
#include "DBPro Functions.h"
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "LMGlobal.h"
#include "Point.h"
#include "Vector.h"

// global function to delete global texture list 
void TreeFaceFreeUsages(void);

class TreeFaceLightmapper
{
    public:    
        Point vert1,vert2,vert3;
        Vector normal;
        float d;
        //bool collisionon;
        //unsigned faceId;
        TreeFaceLightmapper* nextFace;

		static bool backFaces;

		TreeFaceLightmapper( ) { };
		virtual ~TreeFaceLightmapper( ) { };

		bool MakeFace( Point *p1, Point *p2, Point *p3 );
        
        virtual bool intersects( const Point* p, const Vector* v, Lumel* pColour, float* pShadow ) const;
		bool pointInPoly(const Point* p) const;
		virtual bool IsCurved( ) const { return false; }
        
    private:
};

class TransparentFace : public TreeFaceLightmapper
{
	
public:
	
	class TextureClass
	{
	public:
		IGGTexture *pTexture;
		DWORD dwSysMemTransTexWidth;
		DWORD dwSysMemTransTexHeight;
		DWORD* pSysMemTransTex;
		TextureClass *pNextTexture;

		TextureClass( ) { pTexture = 0; pSysMemTransTex=0; pNextTexture = 0; }
		~TextureClass( ) { if ( pSysMemTransTex ) delete[] pSysMemTransTex; pSysMemTransTex=0; }
	};

	static TextureClass *pTextureList;

	float u1,u2,u3;
	float v1,v2,v3;
	//int iWidth, iHeight;
	int iType;				//black(0) or alpha(1) transparency
	TextureClass* pTextureClassUsed;

	IGGTexture *pTexture;

	TransparentFace( ) { pTexture = 0; };
	~TransparentFace( ) {  }; // pTexture only a ref, release in pTextureList

	bool MakeTransparentFace( Point *p1, Point *p2, Point *p3, int type, float fU1, float fV1, float fU2, float fV2, float fU3, float fV3, sTexture* pSrcTexture );

	bool intersects( const Point* p, const Vector* v, Lumel *pColour, float* pShadow ) const;

	void InterpolateUV( const Point *p, float *pU, float *pV ) const;
	bool IsCurved( ) const { return true; }
	
};

class TreeNodeLightmapper
{
      public:
          
          TreeNodeLightmapper() { left = 0; right = 0; }
          ~TreeNodeLightmapper();
          
          void buildTree(TreeFaceLightmapper* faces,unsigned fnum,int facesPerNode);
          
          bool intersects( const Point* p, const Vector* vec, const Vector* vecI, Lumel* pColour, float* pShadow, TreeFaceLightmapper** ppLastHit ) const;
          //Box* getBounds() { return &bounds; }
             
      public:     
          
          Box bounds;
          TreeNodeLightmapper* left;
          TreeNodeLightmapper* right;
};


inline bool TreeFaceLightmapper::intersects(const Point* p, const Vector* v, Lumel* pColour, float* pShadow) const
{
    //if (!collisionon) return false;

	float dist1,dist2;
    Point intersect;
	//Point *p = &(CollisionTreeLightmapper::p);
	//Vector *v = &(CollisionTreeLightmapper::vec);
                                    
	dist1 = normal.x*(p->x+v->x) + normal.y*(p->y+v->y) + normal.z*(p->z+v->z) + d;
    dist2 = normal.x*p->x + normal.y*p->y + normal.z*p->z + d;
    
    if ( dist2*dist1 > 0 ) return false;
                
    dist1=-(normal.x*v->x) - (normal.y*v->y) - (normal.z*v->z);
    if (dist1<-0.0001 || dist1>0.0001) dist1=dist2/dist1;
    else dist1=0;
    
    intersect.x = p->x + v->x*dist1;
    intersect.y = p->y + v->y*dist1;
    intersect.z = p->z + v->z*dist1;
                
    if (!pointInPoly(&intersect)) return false;
    
	if ( pShadow ) *pShadow = 1.0;

    return true;
}

inline bool TreeFaceLightmapper::pointInPoly( const Point* p) const
{
    //First side
    Vector v3(&vert1,&vert2);
    v3 = v3.crossProduct(&normal);
    
    float ld = v3.x*vert1.x + v3.y*vert1.y + v3.z*vert1.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001) return false;
    
    //Second side
    v3.set(&vert2,&vert3);
    v3 = v3.crossProduct(&normal);
    
    ld = v3.x*vert2.x + v3.y*vert2.y + v3.z*vert2.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001) return false;
    
    //Third side
    v3.set(&vert3,&vert1);
    v3 = v3.crossProduct(&normal);
    
    ld = v3.x*vert3.x + v3.y*vert3.y + v3.z*vert3.z;
    
    ld = v3.x*p->x + v3.y*p->y + v3.z*p->z - ld;
    if (ld<-0.000001) return false;
    
    return true;
}

#endif

#ifndef COLTREE_H
#define COLTREE_H

#include "Point.h"
#include "Vector.h"
#include "CollisionObject.h"

class TreeFace;
class TreeNode;
struct sObject;

class CollisionTree : public CollisionObject
{

    public:

		//static Vector vec, vecI;
		//static Point p;
		CollisionTree();
        ~CollisionTree();

		void Reset( );
        
        void SetFacesPerNode(int num) { if (num<2) facesPerNode = 2; else facesPerNode = num; }        
        void MakeCollisionObject(unsigned vnum, float* vertices);
		void MakeCollisionObject(int fnum, TreeFace *pFaceList);

		void MakeCollisionObject( sObject* pObject );
        
        bool Intersects( const Point* p, const Vector* vec, int dir, float* dist ) const;

		int GetCollisionType() { return 0; }
		              
    private:
        
        int facesPerNode;
        TreeNode* tree;
};

#endif

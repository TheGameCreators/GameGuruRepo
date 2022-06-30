#ifndef TREENODE_H
#define TREENODE_H

#include "TreeFace.h"
#include "Box.h"

class TreeNode
{
      public:
          
          TreeNode() { left = 0; right = 0; }
          ~TreeNode();
          
          void BuildTree(TreeFace* faces,unsigned fnum,int facesPerNode);
          
          bool Intersects( const Point* p, const Vector* vec, const Vector* vecI, int dir, float *dist ) const;
		  //Box* getBounds() { return &bounds; }
   
      private:     
          
          Box bounds;
          TreeNode* left;
          TreeNode* right;
};

#endif
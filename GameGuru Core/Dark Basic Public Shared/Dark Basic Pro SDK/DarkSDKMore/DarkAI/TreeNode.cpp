#include "TreeNode.h"
#include <cstdlib>

TreeNode::~TreeNode()
{
    if (right==0)
    {
        TreeFace* temp;
        
        while(left!=0)
        {
            temp = (TreeFace*)left;
            left = (TreeNode*)(temp->nextFace);
            delete (TreeFace*)temp;
        }
    }
    else
    {
        delete left;
        delete right;
    }
}
     
void TreeNode::BuildTree( TreeFace* faces, unsigned fnum, int facesPerNode )
{
    left=0;
    right=0;
    
    //sprintf_s(str,255,"Creating Node... %d",fnum);
    //MessageBox(NULL, str, "Tree", MB_OK);
    
    //build a bounding box for this node
    float minx = faces->vert1.x;
    float miny = faces->vert1.y;
    float minz = faces->vert1.z;
    float maxx = faces->vert1.x;
    float maxy = faces->vert1.y;
    float maxz = faces->vert1.z;
    
    for(TreeFace* thisFace=faces; thisFace!=0; thisFace = thisFace->nextFace) 
    {
        if (thisFace->vert1.x>maxx) maxx = thisFace->vert1.x;
        if (thisFace->vert1.x<minx) minx = thisFace->vert1.x;
        if (thisFace->vert1.y>maxy) maxy = thisFace->vert1.y;
        if (thisFace->vert1.y<miny) miny = thisFace->vert1.y;
        if (thisFace->vert1.z>maxz) maxz = thisFace->vert1.z;
        if (thisFace->vert1.z<minz) minz = thisFace->vert1.z;
        
        if (thisFace->vert2.x>maxx) maxx = thisFace->vert2.x;
        if (thisFace->vert2.x<minx) minx = thisFace->vert2.x;
        if (thisFace->vert2.y>maxy) maxy = thisFace->vert2.y;
        if (thisFace->vert2.y<miny) miny = thisFace->vert2.y;
        if (thisFace->vert2.z>maxz) maxz = thisFace->vert2.z;
        if (thisFace->vert2.z<minz) minz = thisFace->vert2.z;
        
        if (thisFace->vert3.x>maxx) maxx = thisFace->vert3.x;
        if (thisFace->vert3.x<minx) minx = thisFace->vert3.x;
        if (thisFace->vert3.y>maxy) maxy = thisFace->vert3.y;
        if (thisFace->vert3.y<miny) miny = thisFace->vert3.y;
        if (thisFace->vert3.z>maxz) maxz = thisFace->vert3.z;
        if (thisFace->vert3.z<minz) minz = thisFace->vert3.z;
    }
    
    bounds.Set(minx,miny,minz,maxx,maxy,maxz);
    bounds.CorrectBox();
    
    //sprintf_s(str,255,"Building Tree... %5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f",maxx,maxy,maxz,minx,miny,minz);
    //MessageBox(NULL, str, "Intersect Object", MB_OK);
    
    //if this is a leaf node then place the linked list into this node's pointer
    if ((int)fnum<=facesPerNode)
    {
        left = (TreeNode*)faces;
        return;
    }
    
    //otherwise divide the linked list into two new lists
    float difx = maxx-minx;
    float dify = maxy-miny;
    float difz = maxz-minz;
    
    //x=0, y=1, z=2
    //axis is biggest extent, axis1 is second, axis2 is smallest
    int axis = 0;
    int axis1 = 0;
    int axis2 = 0;
    
    if (difx>dify)
    {
        if (difx<difz) { axis = 2; axis1 = 0; axis2 = 1; }
        else
        {
            axis = 0;
            if (dify>difz) { axis1 = 1; axis2 = 2; }
            else { axis1 = 2; axis2 = 1; }
        } 
    }
    else
    {
        if (dify<difz) { axis = 2; axis1 = 1; axis2 = 0; }
        else
        {
            axis = 1;
            if (difx>difz) { axis1 = 0; axis2 = 2; }
            else { axis1 = 2; axis2 = 0; }
        }
    }
    //axes stored for later when handling unequal splits
    
    float limit = 0;
    TreeFace* leftFaces = 0; //new linked list 1
    TreeFace* rightFaces = 0; //new linked list 2
    TreeFace* lastFace = 0; //points to the last element in the new leftFaces list, helps reconstruct the whole list
    TreeFace* temp = 0;
    unsigned fnum1=0, fnum2=0;
    
    //limit for deciding which side of the split a triangle belongs                                                                            
    switch(axis)
    {
        case 0 : limit=difx/2.0f + minx;break;
        case 1 : limit=dify/2.0f + miny;break;
        case 2 : limit=difz/2.0f + minz;break;
    }
    
    float avg;
    float total = 0;
            
    while (faces>0)
    {
        //get the triangles average distance along this axis and compare with the limit
        switch(axis)
        {
            case 0 : avg = (faces->vert1.x + faces->vert2.x + faces->vert3.x)/3.0f;break;
            case 1 : avg = (faces->vert1.y + faces->vert2.y + faces->vert3.y)/3.0f;break;
            case 2 : avg = (faces->vert1.z + faces->vert2.z + faces->vert3.z)/3.0f;break;
        }
            
        total += avg;
                
        if (avg>=limit)
        {
            if (fnum1==0) lastFace = faces;
            
            //pull the first item from the @faces list and attach to the front of @leftFaces
            temp = faces->nextFace;
            faces->nextFace = leftFaces;
            leftFaces = faces;
            faces = temp;
            
            fnum1++;
        }
        else
        {
            temp = faces->nextFace;
            faces->nextFace = rightFaces;
            rightFaces = faces;
            faces = temp;
            
            fnum2++;
        }
    }
    
    //sprintf_s(str,255,"Attempt 1: %d, %d, %d, %d",fnum,fnum1,fnum2,axis);
    //MessageBox(NULL, str, "Debug", MB_OK);
    
    //check for an uneven split - more than 90% of the triangles on one side
    //if so then try changing axis
    if (abs((int) (fnum1-fnum2))>fnum*0.8)
    {
        fnum1=0;
        fnum2=0;
        limit=0;
        
        //check the second largest axis
        switch(axis1)
        {
            case 0 : limit=difx/2.0f + minx;break;
            case 1 : limit=dify/2.0f + miny;break;
            case 2 : limit=difz/2.0f + minz;break;
        }
        
        //reconstruct the list, if none in @leftFaces, already done in @rightFaces
        //otherwise use @lastFace to append the two together
        if (leftFaces)
        {
            faces = leftFaces;
            lastFace->nextFace = rightFaces;
        }
        else faces = rightFaces;
        
        leftFaces = 0;
        rightFaces = 0;
        
        while (faces>0)
        {
            switch(axis1)
            {
                case 0 : avg = (faces->vert1.x + faces->vert2.x + faces->vert3.x)/3.0f;break;
                case 1 : avg = (faces->vert1.y + faces->vert2.y + faces->vert3.y)/3.0f;break;
                case 2 : avg = (faces->vert1.z + faces->vert2.z + faces->vert3.z)/3.0f;break;
            }
                
            if (avg>=limit)
            {
                if (fnum1==0) lastFace = faces;
                
                temp = faces->nextFace;
                faces->nextFace = leftFaces;
                leftFaces = faces;
                faces = temp;
                
                fnum1++;
            }
            else
            {
                temp = faces->nextFace;
                faces->nextFace = rightFaces;
                rightFaces = faces;
                faces = temp;
                
                fnum2++;
            }
        }
        
        //sprintf_s(str,255,"Attempt 2: %d, %d, %d, %d",fnum,fnum1,fnum2,axis1);
        //MessageBox(NULL, str, "Debug", MB_OK);
    }
    
    //if we still don't get a decent split try the final axis
    if (abs((int) (fnum1-fnum2))>fnum*0.8)
    {
        fnum1=0;
        fnum2=0;
        
        //check the third largest axis
        switch(axis2)
        {
            case 0 : limit=difx/2.0f + minx;break;
            case 1 : limit=dify/2.0f + miny;break;
            case 2 : limit=difz/2.0f + minz;break;
        }
        
        if (leftFaces)
        {
            faces = leftFaces;
            lastFace->nextFace = rightFaces;
        }
        else faces = rightFaces;
        
        leftFaces = 0;
        rightFaces = 0;
        
        while (faces>0)
        {
            switch(axis2)
            {
                case 0 : avg = (faces->vert1.x + faces->vert2.x + faces->vert3.x)/3.0f;break;
                case 1 : avg = (faces->vert1.y + faces->vert2.y + faces->vert3.y)/3.0f;break;
                case 2 : avg = (faces->vert1.z + faces->vert2.z + faces->vert3.z)/3.0f;break;
            }
                
            if (avg>=limit)
            {
                if (fnum1==0) lastFace = faces;
                
                temp = faces->nextFace;
                faces->nextFace = leftFaces;
                leftFaces = faces;
                faces = temp;
                
                fnum1++;
            }
            else
            {
                temp = faces->nextFace;
                faces->nextFace = rightFaces;
                rightFaces = faces;
                faces = temp;
                
                fnum2++;
            }
        }
        
        //sprintf_s(str,255,"Attempt 3: %d, %d, %d, %d",fnum,fnum1,fnum2,axis2);
        //MessageBox(NULL, str, "Debug", MB_OK);
    }
    
    //if we get another uneven split then split based on the average distance
    //along the original axis
    if (abs((int) (fnum1-fnum2))>fnum*0.8)
    {
        fnum1=0;
        fnum2=0;
        limit = total/fnum;
        
        if (leftFaces)
        {
            faces = leftFaces;
            lastFace->nextFace = rightFaces;
        }
        else faces = rightFaces;
        
        leftFaces = 0;
        rightFaces = 0;
        
        while (faces>0)
        {
            switch(axis)
            {
                case 0 : avg = (faces->vert1.x + faces->vert2.x + faces->vert3.x)/3.0f;break;
                case 1 : avg = (faces->vert1.y + faces->vert2.y + faces->vert3.y)/3.0f;break;
                case 2 : avg = (faces->vert1.z + faces->vert2.z + faces->vert3.z)/3.0f;break;
            }
                
            if (avg>=limit)
            {
                if (fnum1==0) lastFace = faces;
                
                temp = faces->nextFace;
                faces->nextFace = leftFaces;
                leftFaces = faces;
                faces = temp;
                
                fnum1++;
            }
            else
            {
                temp = faces->nextFace;
                faces->nextFace = rightFaces;
                rightFaces = faces;
                faces = temp;
                
                fnum2++;
            }
        }
        
        //sprintf_s(str,255,"Attempt 4: %d, %d, %d, %d",fnum,fnum1,fnum2,axis);
        //MessageBox(NULL, str, "Debug", MB_OK);
    }
                
    //If this still doesn't produce a decent split, split them randomly
    if (abs((int) (fnum1-fnum2))>fnum*0.9)
    {
        fnum1=0;
        fnum2=0;
        
        if (leftFaces)
        {
            faces = leftFaces;
            lastFace->nextFace = rightFaces;
        }
        else faces = rightFaces;
        
        leftFaces = 0;
        rightFaces = 0;
        
        int flag = 1;
        
        while (faces>0)
        {
            if (flag)
            {
                temp = faces->nextFace;
                faces->nextFace = leftFaces;
                leftFaces = faces;
                faces = temp;
                
                fnum1++;
                flag = 0;
            }
            else
            {
                temp = faces->nextFace;
                faces->nextFace = rightFaces;
                rightFaces = faces;
                faces = temp;
                
                fnum2++;
                flag = 1;
            }
        }
        
        //sprintf_s(str,255,"Random: %d, %d, %d",fnum,fnum1,fnum2);
        //MessageBox(NULL, str, "Debug", MB_OK);
    }
    
    left = new TreeNode();
    right = new TreeNode();
    
//    sprintf_s(str,255,"Done");
//    MessageBox(NULL, str, "Collision Setup Info", MB_OK); 
                
    left->BuildTree(leftFaces,fnum1,facesPerNode);
    right->BuildTree(rightFaces,fnum2,facesPerNode);
}

bool TreeNode::Intersects( const Point* p, const Vector* vec, const Vector* vecI, int dir, float *dist ) const
{
	float fNewDist = 0;
	if ( bounds.IntersectBox( p, vecI, &fNewDist ) == 0 ) return false;
	if ( dist && fNewDist > *dist && *dist >= 0 ) return false;

	bool bResult = false;
    
    //if not a leaf node then simply pass ray details to the two child nodes
    if (right!=0)
    {
        if ( left->Intersects( p, vec, vecI, dir, dist ) ) 
		{
			//if closest collision dist is not needed, can return on any collision
			if ( dist ) bResult = true;
			else return true;
		}
        if ( right->Intersects( p, vec, vecI, dir, dist ) ) bResult = true;
    }
    else
    {
        //else cycle through all the faces in it
        for(TreeFace* thisFace=(TreeFace*)left; thisFace!=0; thisFace=thisFace->nextFace)
        {
			if ( thisFace->Intersects( p, vec, dir, dist ) ) 
			{
				//if closest collision dist is not needed, can return on any collision
				if ( dist ) bResult = true;
				else return true;
			}
        }
    }
                        
    return bResult;
}
// Dave moved these to the top for £20 from Lee Bamber
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "DBPro Functions.h"

#include <Windows.h>
#include "CollisionTree.h"
#include "TreeFace.h"
#include "TreeNode.h"

//Vector CollisionTree::vec, CollisionTree::vecI;
//Point CollisionTree::p;

CollisionTree::CollisionTree( )
{
	facesPerNode = 2;
	tree = 0;
	iObjID = 0;
}

CollisionTree::~CollisionTree() 
{ 
	delete tree; 
}

void CollisionTree::Reset( )
{
	delete tree;
	tree = 0;
}

//takes vertices as 3 floats to a face (position only)
void CollisionTree::MakeCollisionObject(unsigned vnum, float* vertices)
{
    //build tree 
    
    int j;
    unsigned fnum = vnum/3;
    Point p1,p2,p3;
    TreeFace* faces = 0;
    
    for(int i=0; i<(int)fnum; i++) 
    {
        j=i*9;
        
        TreeFace* pNewFace = new TreeFace();
        
        p1.set(*(vertices+j),*(vertices+j+1),*(vertices+j+2));
        p2.set(*(vertices+j+3),*(vertices+j+4),*(vertices+j+5));
        p3.set(*(vertices+j+6),*(vertices+j+7),*(vertices+j+8));
        
		//if creation fails, delete
		if ( !pNewFace->MakeFace( &p1, &p2, &p3, i ) ) 
		{ 
			delete pNewFace; 
			continue; 
		}
        
		//construct a linked list of faces
		pNewFace->nextFace = faces;
        faces = pNewFace;
    }
    
    delete [] vertices;
    
    if (facesPerNode<2) facesPerNode = 2;
    
	if ( tree ) Reset( );

    tree = new TreeNode(); 
    tree->BuildTree(faces,fnum,facesPerNode);
}

void CollisionTree::MakeCollisionObject( int fnum, TreeFace *pFaceList )
{
	if (facesPerNode<2) facesPerNode = 2;
    
	if ( tree ) Reset( );

    tree = new TreeNode(); 
    tree->BuildTree(pFaceList,fnum,facesPerNode);
}

void CollisionTree::MakeCollisionObject( sObject *pObject )
{
	if ( !pObject ) return;

	//FILE* pDataFile = GG_fopen( "Object Data.txt","w" );

	sObject *pPosObject = pObject;

	if ( pObject->pInstanceOfObject ) pObject = pObject->pInstanceOfObject;
	
	int iNumFrames = pObject->iFrameCount;
	
	TreeFace* pFaceList = 0;
	int iNumFaces = 0;

	for (int iFrame = 0; iFrame < iNumFrames; iFrame++ )
	{
		if ( !pObject->ppFrameList [ iFrame ] ) continue;
		sMesh *pMesh = pObject->ppFrameList [ iFrame ]->pMesh;
		if ( !pMesh ) continue;

		CalculateAbsoluteWorldMatrix ( pPosObject, pObject->ppFrameList [ iFrame ], pObject->ppFrameList [ iFrame ]->pMesh );

		int iPrimitiveType = pMesh->iPrimitiveType;
		if ( iPrimitiveType != 4 ) 
		{
			ConvertLocalMeshToTriList( pMesh );
		}

		DWORD dwFVFSize = pMesh->dwFVFSize;
		int iNumVertices = pMesh->dwVertexCount;
		int iNumIndices = pMesh->dwIndexCount;

		BYTE *pVertexData = pMesh->pVertexData;
		#ifdef X10
		DWORD *pIndices = pMesh->pIndices;
		#else
		WORD *pIndices = pMesh->pIndices;
		#endif

		if ( dwFVFSize < 12 ) continue;
		if ( iNumVertices <= 0 ) continue;
		//if ( iNumIndices > 0 && iPrimitiveType != 4 ) continue;

		int iMax = iNumIndices>0 ? iNumIndices : iNumVertices;
		Point p1,p2,p3;
		int dwIndex;
		GGVECTOR3 vecVert;

		sOffsetMap	pOffsetMap;
		GetFVFOffsetMap ( pMesh, &pOffsetMap );

		//sprintf(errAIStr,"Indices: %d Vertices: %d\n\n",iNumIndices,iNumVertices);
		//fputs(errAIStr,pDataFile);

		for ( int i = 0; i < iMax; i+=3 )
		{
			if ( iNumIndices > 0 ) dwIndex = *(pIndices + i)*dwFVFSize; else dwIndex = i*dwFVFSize;

			vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
			vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
			vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
			
			GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
			p1.x = vecVert.x; p1.y = vecVert.y; p1.z = vecVert.z;

			if ( iNumIndices > 0 ) dwIndex = *(pIndices + i + 1)*dwFVFSize; else dwIndex = (i+1)*dwFVFSize;

			vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
			vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
			vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
		
			GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
			p2.x = vecVert.x; p2.y = vecVert.y; p2.z = vecVert.z;

			if ( iNumIndices > 0 ) dwIndex = *(pIndices + i + 2)*dwFVFSize; else dwIndex = (i+2)*dwFVFSize;

			vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
			vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
			vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
		
			GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
			p3.x = vecVert.x; p3.y = vecVert.y; p3.z = vecVert.z;

			TreeFace* aFace;
			bool bValid;
			
			aFace = new TreeFace( );
			bValid = aFace->MakeFace( &p1, &p2, &p3, iNumFaces );
			
			//zero area polygons will have an undefined normal, which will cause problems later
			//so if any exist remove them
			if (!bValid) { delete aFace; continue; }
	        
			aFace->nextFace = pFaceList;
			pFaceList = aFace;

			iNumFaces++;
		}
	}

	if ( pFaceList ) MakeCollisionObject( iNumFaces, pFaceList );
}

bool CollisionTree::Intersects( const Point* p, const Vector* vec, int dir, float *dist ) const
{    
	Vector vecI;
	vecI.set( 1.0f/vec->x, 1.0f/vec->y, 1.0f/vec->z );

	if ( !tree ) return false;
    return tree->Intersects( p, vec, &vecI, dir, dist );
}
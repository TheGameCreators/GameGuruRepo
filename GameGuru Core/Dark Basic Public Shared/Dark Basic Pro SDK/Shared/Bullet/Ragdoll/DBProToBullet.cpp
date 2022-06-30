
// BulletPhysicsWrapper for DarkBasic Professional
//Stab In The Dark Software 
//Copyright (c) 2002-2014
//http://stabinthedarksoftware.com

///#include "StdAfx.h"
#include "DBProToBullet.h"

//#include "DBPro.hpp"
#include "CObjectsC.h"
#include ".\..\..\Error\CError.h"

// need proper resource handling here!!
int iStartObjectOffset = 1000;
int iStartMeshOffset = 1000;

int FindFreeObject ( void )
{
	int iObj = iStartObjectOffset;
	while ( iObj )
	{
		if ( ObjectExist ( iObj )==NULL )
			return iObj;
		iObj++;
	}
	return -1;
}

int FindFreeMesh ( void )
{
	iStartMeshOffset++;
	return iStartMeshOffset;
}

namespace DBProToBullet
{
	btScalar DBProToBullet::GetObjectDiameter(int objectID)
	{
		btVector3 sizeVec = GetObjectSize(objectID);
		btScalar diameter = sizeVec.getX();
		if(diameter < sizeVec.getY())
			diameter = sizeVec.getY();
		else if (diameter < sizeVec.getZ())
			diameter = sizeVec.getZ();
		return diameter;
	}
	btVector3 DBProToBullet::GetObjectSize(int objectID)
	{
		return btVector3(ObjectSizeX(objectID), ObjectSizeY(objectID), ObjectSizeZ(objectID));
	}

	DBProToBullet::DBProVertexData* GetVertexData(int objectID, btScalar scaleFactor, bool bTransform, bool bReverseVertexOrder, bool bMirrorOnXAxis) 
	{
		DBProToBullet::DBProVertexData* VertexData = new DBProToBullet::DBProVertexData();
		int prevIndexCount = 0;
		btTransform objectTrans = DBProToBullet::GetTransform(objectID);
		btVector3 objectScale = DBProToBullet::GetScale(objectID); 
		int limbCount = ::GetLimbCount(objectID);
		for(int limbNumber = 0; limbNumber < limbCount; limbNumber++)
		{
			LockVertexDataForLimb(objectID,limbNumber);
			int vertexCount = GetVertexDataVertexCount();
			int indexCount = GetVertexDataIndexCount();
			for(int i = 0; i < vertexCount; i++)
			{
				btVector3 vertex(GetVertexDataPositionX(i), GetVertexDataPositionY(i), GetVertexDataPositionZ(i));
				vertex = vertex * objectScale;//handle DBPro scaling
				vertex = vertex / scaleFactor;
				vertex = bTransform ? objectTrans * vertex : vertex;
				vertex = bMirrorOnXAxis ? vertex * btVector3(-1.0,1.0,1.0) : vertex;
				//
				btVector3 normal(GetVertexDataNormalsX(i), GetVertexDataNormalsY(i), GetVertexDataNormalsZ(i));	
				//
				btVector3 uv(GetVertexDataU(i,0), GetVertexDataV(i,0), 0.0);
				//
				VertexData->vertexBuffer.push_back(vertex);
				VertexData->normals.push_back(normal);
				VertexData->uvData.push_back(uv);
			}
			for(int i = 0; i < indexCount; i +=3)
			{
				if(bReverseVertexOrder)
				{
					VertexData->indexBuffer.push_back(GetIndexData(i+2)+prevIndexCount);
					VertexData->indexBuffer.push_back(GetIndexData(i+1)+prevIndexCount);
					VertexData->indexBuffer.push_back(GetIndexData(i)+prevIndexCount);
				}
				else
				{
					VertexData->indexBuffer.push_back(GetIndexData(i)+prevIndexCount);
					VertexData->indexBuffer.push_back(GetIndexData(i+1)+prevIndexCount);
					VertexData->indexBuffer.push_back(GetIndexData(i+2)+prevIndexCount);
				}
			}
			prevIndexCount += indexCount;
			UnlockVertexData();
		}
		return VertexData;
	}

	//void GetVertexData(int objectID, btAlignedObjectArray<btVector3> & outObjVerts,
	//	btAlignedObjectArray<btVector3> &outObjUVData,  btAlignedObjectArray<btVector3> & outObjNormals, 
	//	bool bTransform, bool bMirrorXAxis /*=false*/)
	//{
	//	btTransform objectTrans;
	//	objectTrans = DBProToBullet::GetTransform(objectID);

	//	for(int limb = 0; limb < GetLimbCount(objectID); limb++)
	//	{
	//		LockVertexDataForLimb(objectID,limb);
	//		for(int i = 0; i < GetVertexDataVertexCount(); i++)
	//		{
	//			btVector3 positionVec;
	//			btVector3 normalVec;
	//			btVector3 uvVec;
	//			if(bMirrorXAxis)
	//			{
	//				positionVec = btVector3(GetVertexDataPositionX(i)*-1, GetVertexDataPositionY(i), GetVertexDataPositionZ(i));
	//			}
	//			else
	//			{
	//				positionVec = btVector3(GetVertexDataPositionX(i), GetVertexDataPositionY(i), GetVertexDataPositionZ(i));
	//			}

	//			normalVec = btVector3(GetVertexDataNormalsX(i), GetVertexDataNormalsY(i), GetVertexDataNormalsZ(i));				
	//			uvVec = btVector3(GetVertexDataU(i,0), GetVertexDataV(i,0), 0.0);
	//			if(bTransform == true)
	//			{
	//				positionVec = objectTrans * positionVec;
	//			}
	//			outObjVerts.push_back(positionVec);
	//			outObjNormals.push_back(normalVec);
	//			outObjUVData.push_back(uvVec);
	//		}
	//		UnlockVertexData();
	//	}
	//}

	/*void GetIndexData(int objectID, btAlignedObjectArray<int> & outObjIndices, bool bReverseOrder)
	{
		int prevIndexCount = 0;
		for(int limb = 0; limb < GetLimbCount(objectID); limb++)
		{
			LockVertexDataForLimb(objectID,limb);
			for(int i = 0; i < GetVertexDataIndexCount(); i +=3)
			{
				if(bReverseOrder == true)
				{
					outObjIndices.push_back(GetIndexdata(i+2)+prevIndexCount);
					outObjIndices.push_back(GetIndexdata(i+1)+prevIndexCount);
					outObjIndices.push_back(GetIndexdata(i)+prevIndexCount);
				}
				else
				{
					outObjIndices.push_back(GetIndexdata(i)+prevIndexCount);
					outObjIndices.push_back(GetIndexdata(i+1)+prevIndexCount);
					outObjIndices.push_back(GetIndexdata(i+2)+prevIndexCount);
				}
			}
			prevIndexCount += GetVertexDataIndexCount();
			UnlockVertexData();
		}
	}*/

	btVector3 DBProToBullet::GetScale(int objectID)
	{
		return btVector3(ObjectScaleX(objectID)/100.0, ObjectScaleY(objectID)/100.0, ObjectScaleZ(objectID)/100.0);
	}

	//Note: Transform does not include scaling
	btTransform DBProToBullet::GetTransform(int objectID, btScalar scaleFactor/* = 1.0*/)
	{
		btTransform objTransform;
		objTransform.setIdentity();
		objTransform.setOrigin(btVector3(ObjectPositionX(objectID)/scaleFactor, ObjectPositionY(objectID)/scaleFactor, ObjectPositionZ(objectID)/scaleFactor));
		//Using a btMatrix setEulerYPR must take it ZYX for correct rotations in DBPro.
		btMatrix3x3 btMatrix;
		btMatrix.setEulerYPR(btScalar(btRadians(ObjectAngleZ(objectID))),
										 btScalar(btRadians(ObjectAngleY(objectID))),
										 btScalar(btRadians(ObjectAngleX(objectID))));
		objTransform.setBasis(btMatrix);
		return objTransform;
	}

	btVector3 DBProToBullet::GetVector3(int vectorID)
	{
		return btVector3(btScalar(GetXVector3(vectorID)), btScalar(GetYVector3(vectorID)), btScalar(GetZVector3(vectorID)));
	}


	void DBProToBullet::AssertValidVector(int vectorID, LPCSTR message)
	{
		if(VectorExist(vectorID) == false)
		{
			//ReportError(message,"Bullet Physics Wrapper");
			RunTimeError ( 0, (LPSTR)message );
		}
	}

	void DBProToBullet::AssertValidObject(int objectID, LPCSTR message)
	{
		if(ObjectExist(objectID) == false)
		{
			//ReportError(message,"Bullet Physics Wrapper");
			RunTimeError ( 0, (LPSTR)message );
		}
	}
}



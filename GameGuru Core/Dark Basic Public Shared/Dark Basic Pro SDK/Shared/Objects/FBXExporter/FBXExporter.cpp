#include "FBXExporter.h"
#include "Utilities.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// Include headers to make DBO object
#include "CObjectsC.h"

FBXExporter::FBXExporter()
{
	mFBXManager = nullptr;
	mFBXScene = nullptr;
	mTriangleCount = 0;
	mHasAnimation = true;
	QueryPerformanceFrequency(&mCPUFreq);
}

bool FBXExporter::Initialize()
{
	mFBXManager = FbxManager::Create();
	if (!mFBXManager)
	{
		return false;
	}

	FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(mFBXManager, IOSROOT);
	mFBXManager->SetIOSettings(fbxIOSettings);

	mFBXScene = FbxScene::Create(mFBXManager, "myScene");

	return true;
}

bool FBXExporter::LoadScene(const char* inFileName, const char* inOutputPath)
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	mInputFilePath = inFileName;
	mOutputFilePath = inOutputPath;

	QueryPerformanceCounter(&start);
	FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "myImporter");

	if (!fbxImporter)
	{
		return false;
	}

	if (!fbxImporter->Initialize(inFileName, -1, mFBXManager->GetIOSettings()))
	{
		return false;
	}

	if (!fbxImporter->Import(mFBXScene))
	{
		return false;
	}
	fbxImporter->Destroy();
	QueryPerformanceCounter(&end);
	std::cout << "Loading FBX File: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	return true;
}

void FBXExporter::ExportFBX()
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	
	// Get the clean name of the model
	std::string genericFileName = Utilities::GetFileName(mInputFilePath);
	genericFileName = Utilities::RemoveSuffix(genericFileName);

	QueryPerformanceCounter(&start);
	ProcessSkeletonHierarchy(mFBXScene->GetRootNode());
	if(mSkeleton.mJoints.empty())
	{
		mHasAnimation = false;
	}

	std::cout << "\n\n\n\nExporting Model:" << genericFileName << "\n";
	QueryPerformanceCounter(&end);
	std::cout << "Processing Skeleton Hierarchy: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	QueryPerformanceCounter(&start);
	ProcessGeometry(mFBXScene->GetRootNode());
	QueryPerformanceCounter(&end);
	std::cout << "Processing Geometry: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";

	QueryPerformanceCounter(&start);
	Optimize();
	QueryPerformanceCounter(&end);
	std::cout << "Optimization: " << ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart)) << "s\n";
	//PrintMaterial();
	std::cout << "\n\n";

	// create a DBO from the prepared model data
	int iID = 8987;
	bool bErr = CreateNewObject ( iID, "mesh" );
	if ( bErr == true )
	{
		// no transform of new limb
		D3DXMATRIX matWorld;
		D3DXMatrixIdentity ( &matWorld );

		// already have frame and mesh
		sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;
		if ( pMesh )
		{
			// populate vertex data from FBX model data
			pMesh->dwVertexCount = mVertices.size();
			DWORD dwVertElementCount = 3 + 3 + 2;
			pMesh->dwFVFSize = dwVertElementCount * sizeof(float);
			pMesh->pVertexData = (BYTE*)new float[pMesh->dwVertexCount*dwVertElementCount];
			pMesh->dwFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
			float* pVertPtr = (float*)pMesh->pVertexData;
			for ( int i=0; i<pMesh->dwVertexCount; i++ )
			{
				pVertPtr[0] = mVertices[i].mPosition.x;
				pVertPtr[1] = mVertices[i].mPosition.y;
				pVertPtr[2] = mVertices[i].mPosition.z;
				pVertPtr[3] = mVertices[i].mNormal.x;
				pVertPtr[4] = mVertices[i].mNormal.y;
				pVertPtr[5] = mVertices[i].mNormal.z;
				pVertPtr[6] = mVertices[i].mUV.x;
				pVertPtr[7] = mVertices[i].mUV.y;
				pVertPtr += dwVertElementCount;
			}
			//inStream << "\t\t\t<sw>" << static_cast<float>(mVertices[i].mVertexBlendingInfos[0].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[1].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[2].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[3].mBlendingWeight) << "</sw>" << std::endl;
			//inStream << "\t\t\t<si>" << mVertices[i].mVertexBlendingInfos[0].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[1].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[2].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[3].mBlendingIndex << "</si>" << std::endl;

			// populate index data from FBX model data
			pMesh->dwIndexCount = mTriangleCount * 3;
			pMesh->pIndices = (WORD*)new WORD[pMesh->dwIndexCount];
			WORD* pIndicePtr = pMesh->pIndices;
			for ( int i=0; i<mTriangleCount; i++ )
			{
				pIndicePtr[0] = mTriangles[i].mIndices[0];
				pIndicePtr[1] = mTriangles[i].mIndices[2];
				pIndicePtr[2] = mTriangles[i].mIndices[1];
				pIndicePtr += 3;
			}

			// set other mesh properties
			pMesh->pVertexDec = NULL;
			pMesh->dwBoneCount = 0;
		}

		// setup new object and introduce to buffers
		SetNewObjectFinalProperties ( iID, -1.0f );

		// save DBO object out
		if ( DoesFileExist ( "Door.DBO" )==true ) DeleteFile (  "Door.DBO" );
		SaveObject ( "Door.DBO", iID );
	}

	/*
	std::string outputMeshName = mOutputFilePath + genericFileName + ".itpmesh";
	std::ofstream meshOutput(outputMeshName);
	WriteMeshToStream(meshOutput);
	if(mHasAnimation)
	{
		std::string outputNnimName = mOutputFilePath + genericFileName + ".itpanim";
		std::ofstream animOutput(outputNnimName);
		WriteAnimationToStream(animOutput);
	}
	*/

	CleanupFbxManager();
	std::cout << "\n\nExport Done!\n";
}

void FBXExporter::ProcessGeometry(FbxNode* inNode)
{
	if (inNode->GetNodeAttribute())
	{
		switch (inNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessControlPoints(inNode);
			if(mHasAnimation)
			{
				ProcessJointsAndAnimations(inNode);
			}
			ProcessMesh(inNode);
			AssociateMaterialToMesh(inNode);
			ProcessMaterials(inNode);
			break;
		}
	}

	for (int i = 0; i < inNode->GetChildCount(); ++i)
	{
		ProcessGeometry(inNode->GetChild(i));
	}
}

void FBXExporter::ProcessSkeletonHierarchy(FbxNode* inRootNode)
{

	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = inRootNode->GetChild(childIndex);
		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	}
}

void FBXExporter::ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex)
{
	if(inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint currJoint;
		currJoint.mParentIndex = inParentIndex;
		currJoint.mName = inNode->GetName();
		mSkeleton.mJoints.push_back(currJoint);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton.mJoints.size(), myIndex);
	}
}

void FBXExporter::ProcessControlPoints(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	unsigned int ctrlPointCount = currMesh->GetControlPointsCount();
	for(unsigned int i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* currCtrlPoint = new CtrlPoint();
		XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->mPosition = currPosition;
		mControlPoints[i] = currCtrlPoint;
	}
}

void FBXExporter::ProcessJointsAndAnimations(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount();
	// This geometry transform is something I cannot understand
	// I think it is from MotionBuilder
	// If you are using Maya for your models, 99% this is just an
	// identity matrix
	// But I am taking it into account anyways......
	FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(inNode);

	// A deformer is a FBX thing, which contains some clusters
	// A cluster contains a link, which is basically a joint
	// Normally, there is only one deformer in a mesh
	for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		// There are many types of deformers in Maya,
		// We are using only skins, so we see if this is a skin
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (!currSkin)
		{
			continue;
		}

		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
		{
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			std::string currJointName = currCluster->GetLink()->GetName();
			unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
			FbxAMatrix transformMatrix;						
			FbxAMatrix transformLinkMatrix;					
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			// Update the information in mSkeleton 
			mSkeleton.mJoints[currJointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;
			mSkeleton.mJoints[currJointIndex].mNode = currCluster->GetLink();

			// Associate each joint with the control points it affects
			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i < numOfIndices; ++i)
			{
				BlendingIndexWeightPair currBlendingIndexWeightPair;
				currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
				currBlendingIndexWeightPair.mBlendingWeight = currCluster->GetControlPointWeights()[i];
				mControlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
			}

			// Get animation information
			// Now only supports one take
			FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			mAnimationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
			Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;

			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
			{
				FbxTime currTime;
				currTime.SetFrame(i, FbxTime::eFrames24);
				*currAnim = new Keyframe();
				(*currAnim)->mFrameNum = i;
				FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
				(*currAnim)->mGlobalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				currAnim = &((*currAnim)->mNext);
			}
		}
	}

	// Some of the control points only have less than 4 joints
	// affecting them.
	// For a normal renderer, there are usually 4 joints
	// I am adding more dummy joints if there isn't enough
	BlendingIndexWeightPair currBlendingIndexWeightPair;
	currBlendingIndexWeightPair.mBlendingIndex = 0;
	currBlendingIndexWeightPair.mBlendingWeight = 0;
	for(auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		for(unsigned int i = itr->second->mBlendingInfo.size(); i <= 4; ++i)
		{
			itr->second->mBlendingInfo.push_back(currBlendingIndexWeightPair);
		}
	}
}

unsigned int FBXExporter::FindJointIndexUsingName(const std::string& inJointName)
{
	for(unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		if (mSkeleton.mJoints[i].mName == inJointName)
		{
			return i;
		}
	}

	throw std::exception("Skeleton information in FBX file is corrupted.");
}


void FBXExporter::ProcessMesh(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();

	mTriangleCount = currMesh->GetPolygonCount();
	int vertexCounter = 0;
	mTriangles.reserve(mTriangleCount);

	for (unsigned int i = 0; i < mTriangleCount; ++i)
	{
		XMFLOAT3 normal[3];
		XMFLOAT3 tangent[3];
		XMFLOAT3 binormal[3];
		XMFLOAT2 UV[3][2];
		Triangle currTriangle;
		mTriangles.push_back(currTriangle);

		for (unsigned int j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];


			ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);
			// We only have diffuse texture
			for (int k = 0; k < 1; ++k)
			{
				ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]);
			}


			PNTIWVertex temp;
			temp.mPosition = currCtrlPoint->mPosition;
			temp.mNormal = normal[j];
			temp.mUV = UV[j][0];
			// Copy the blending info from each control point
			for(unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
			{
				VertexBlendingInfo currBlendingInfo;
				currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
				currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight;
				temp.mVertexBlendingInfos.push_back(currBlendingInfo);
			}
			// Sort the blending info so that later we can remove
			// duplicated vertices
			temp.SortBlendingInfoByWeight();

			mVertices.push_back(temp);
			mTriangles.back().mIndices.push_back(vertexCounter);
			++vertexCounter;
		}
	}

	// Now mControlPoints has served its purpose
	// We can free its memory
	for(auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		delete itr->second;
	}
	mControlPoints.clear();
}

void FBXExporter::ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV)
{
	if(inUVLayer >= 2 || inMesh->GetElementUVCount() <= inUVLayer)
	{
		throw std::exception("Invalid UV Layer Number");
	}
	FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(inUVLayer);

	switch(vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch(vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch(vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	if(inMesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");
	}

	FbxGeometryElementNormal* vertexNormal = inMesh->GetElementNormal(0);
	switch(vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch(vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch(vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
{
	if(inMesh->GetElementBinormalCount() < 1)
	{
		throw std::exception("Invalid Binormal Number");
	}

	FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
	switch(vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch(vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch(vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExporter::ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent)
{
	if(inMesh->GetElementTangentCount() < 1)
	{
		throw std::exception("Invalid Tangent Number");
	}

	FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
	switch(vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch(vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch(vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

// This function removes the duplicated vertices and
// adjust the index buffer properly
// This function should take a while, though........
void FBXExporter::Optimize()
{
	// First get a list of unique vertices
	std::vector<PNTIWVertex> uniqueVertices;
	for(unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		for(unsigned int j = 0; j < 3; ++j)
		{
			// If current vertex has not been added to
			// our unique vertex list, then we add it
			if(FindVertex(mVertices[i * 3 + j], uniqueVertices) == -1)
			{
				uniqueVertices.push_back(mVertices[i * 3 + j]);
			}
		}
	}

	// Now we update the index buffer
	for(unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		for(unsigned int j = 0; j < 3; ++j)
		{
			mTriangles[i].mIndices[j] = FindVertex(mVertices[i * 3 + j], uniqueVertices);
		}
	}
	
	mVertices.clear();
	mVertices = uniqueVertices;
	uniqueVertices.clear();

	// Now we sort the triangles by materials to reduce 
	// shader's workload
	std::sort(mTriangles.begin(), mTriangles.end());
}

int FBXExporter::FindVertex(const PNTIWVertex& inTargetVertex, const std::vector<PNTIWVertex>& uniqueVertices)
{
	for(unsigned int i = 0; i < uniqueVertices.size(); ++i)
	{
		if(inTargetVertex == uniqueVertices[i])
		{
			return i;
		}
	}

	return -1;
}

/*
void FBXExporter::ReduceVertices()
{
	CleanupFbxManager();
	std::vector<Vertex::PNTVertex> newVertices;
	for (unsigned int i = 0; i < mVertices.size(); ++i)
	{
		int index = FindVertex(mVertices[i], newVertices);
		if (index == -1)
		{
			mIndexBuffer[i] = newVertices.size();
			newVertices.push_back(mVertices[i]);
		}
		else
		{
			mIndexBuffer[i] = index;
		}
	}

	mVertices = newVertices;
}
*/

/*

int FBXExporter::FindVertex(const Vertex::PNTVertex& inTarget, const std::vector<Vertex::PNTVertex>& inVertices)
{
	int index = -1;
	for (unsigned int i = 0; i < inVertices.size(); ++i)
	{
		if (inTarget == inVertices[i])
		{
			index = i;
		}
	}

	return index;
}
*/

void FBXExporter::AssociateMaterialToMesh(FbxNode* inNode)
{
	FbxLayerElementArrayTemplate<int>* materialIndices;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	FbxMesh* currMesh = inNode->GetMesh();

	if(currMesh->GetElementMaterial())
	{
		materialIndices = &(currMesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = currMesh->GetElementMaterial()->GetMappingMode();

		if(materialIndices)
		{
			switch(materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (materialIndices->GetCount() == mTriangleCount)
				{
					for (unsigned int i = 0; i < mTriangleCount; ++i)
					{
						unsigned int materialIndex = materialIndices->GetAt(i);
						mTriangles[i].mMaterialIndex = materialIndex;
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				unsigned int materialIndex = materialIndices->GetAt(0);
				for (unsigned int i = 0; i < mTriangleCount; ++i)
				{
					mTriangles[i].mMaterialIndex = materialIndex;
				}
			}
			break;

			default:
				throw std::exception("Invalid mapping mode for material\n");
			}
		}
	}
}

void FBXExporter::ProcessMaterials(FbxNode* inNode)
{
	unsigned int materialCount = inNode->GetMaterialCount();

	for(unsigned int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = inNode->GetMaterial(i);
		ProcessMaterialAttribute(surfaceMaterial, i);
		ProcessMaterialTexture(surfaceMaterial, mMaterialLookUp[i]);
	}
}

void FBXExporter::ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, unsigned int inMaterialIndex)
{
	FbxDouble3 double3;
	FbxDouble double1;
	if (inMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		PhongMaterial* currMaterial = new PhongMaterial();

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Specular;
		currMaterial->mSpecular.x = static_cast<float>(double3.mData[0]);
		currMaterial->mSpecular.y = static_cast<float>(double3.mData[1]);
		currMaterial->mSpecular.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Reflection;
		currMaterial->mReflection.x = static_cast<float>(double3.mData[0]);
		currMaterial->mReflection.y = static_cast<float>(double3.mData[1]);
		currMaterial->mReflection.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;

		// Shininess
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Shininess;
		currMaterial->mShininess = double1;

		// Specular Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->SpecularFactor;
		currMaterial->mSpecularPower = double1;


		// Reflection Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->ReflectionFactor;
		currMaterial->mReflectionFactor = double1;

		mMaterialLookUp[inMaterialIndex] = currMaterial;
	}
	else if (inMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		LambertMaterial* currMaterial = new LambertMaterial();

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;

		mMaterialLookUp[inMaterialIndex] = currMaterial;
	}
}

void FBXExporter::ProcessMaterialTexture(FbxSurfaceMaterial* inMaterial, Material* ioMaterial)
{
	unsigned int textureIndex = 0;
	FbxProperty property;

	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = inMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
		if(property.IsValid())
		{
			unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
			for(unsigned int i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if(layeredTexture)
				{
					throw std::exception("Layered Texture is currently unsupported\n");
				}
				else
				{
					FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
					if(texture)
					{
						std::string textureType = property.GetNameAsCStr();
						FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

						if(fileTexture)
						{
							if (textureType == "DiffuseColor")
							{
								ioMaterial->mDiffuseMapName = fileTexture->GetFileName();
							}
							else if (textureType == "SpecularColor")
							{
								ioMaterial->mSpecularMapName = fileTexture->GetFileName();
							}
							else if (textureType == "Bump")
							{
								ioMaterial->mNormalMapName = fileTexture->GetFileName();
							}
						}
					}
				}
			}
		}
	}
}

void FBXExporter::PrintMaterial()
{
	for(auto itr = mMaterialLookUp.begin(); itr != mMaterialLookUp.end(); ++itr)
	{
		itr->second->WriteToStream(std::cout);
		std::cout << "\n\n";
	}
}

void FBXExporter::PrintTriangles()
{
	for(unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		std::cout << "Triangle# " << i + 1 << " Material Index: " << mTriangles[i].mMaterialIndex << "\n";
	}
}

void FBXExporter::CleanupFbxManager()
{
	mFBXScene->Destroy();
	mFBXManager->Destroy();

	mTriangles.clear();

	mVertices.clear();

	mSkeleton.mJoints.clear();

	for(auto itr = mMaterialLookUp.begin(); itr != mMaterialLookUp.end(); ++itr)
	{
		delete itr->second;
	}
	mMaterialLookUp.clear();
}

void FBXExporter::WriteMeshToStream(std::ostream& inStream)
{
	inStream << "<?xml version='1.0' encoding='UTF-8' ?>" << std::endl;
	inStream << "<itpmesh>" << std::endl;
	if(mHasAnimation)
	{
		inStream << "\t<!-- position, normal, skinning weights, skinning indices, texture-->" << std::endl;
		inStream << "\t<format>pnst</format>" << std::endl;
	}
	else
	{
		inStream << "\t<format>pnt</format>" << std::endl;
	}
	inStream << "\t<texture>" << mMaterialLookUp[0]->mDiffuseMapName << "</texture>" << std::endl;
	inStream << "\t<triangles count='" << mTriangleCount << "'>" << std::endl;

	for (unsigned int i = 0; i < mTriangleCount; ++i)
	{
		// We need to change the culling order
		inStream << "\t\t<tri>" << mTriangles[i].mIndices[0] << "," << mTriangles[i].mIndices[2] << "," << mTriangles[i].mIndices[1] << "</tri>" << std::endl;
	}
	inStream << "\t</triangles>" << std::endl;

	
	inStream << "\t<vertices count='" << mVertices.size() << "'>" << std::endl;
	for (unsigned int i = 0; i < mVertices.size(); ++i)
	{
		inStream << "\t\t<vtx>" << std::endl;
		inStream << "\t\t\t<pos>" << mVertices[i].mPosition.x << "," << mVertices[i].mPosition.y << "," << -mVertices[i].mPosition.z << "</pos>" << std::endl;
		inStream << "\t\t\t<norm>" << mVertices[i].mNormal.x << "," << mVertices[i].mNormal.y << "," << -mVertices[i].mNormal.z << "</norm>" << std::endl;
		if(mHasAnimation)
		{
			inStream << "\t\t\t<sw>" << static_cast<float>(mVertices[i].mVertexBlendingInfos[0].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[1].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[2].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[3].mBlendingWeight) << "</sw>" << std::endl;
			inStream << "\t\t\t<si>" << mVertices[i].mVertexBlendingInfos[0].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[1].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[2].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[3].mBlendingIndex << "</si>" << std::endl;
		}
		inStream << "\t\t\t<tex>" << mVertices[i].mUV.x << "," << 1.0f - mVertices[i].mUV.y << "</tex>" << std::endl;
		inStream << "\t\t</vtx>" << std::endl;
	}
	
	inStream << "\t</vertices>" << std::endl;
	inStream << "</itpmesh>" << std::endl;
}

void FBXExporter::WriteAnimationToStream(std::ostream& inStream)
{
	inStream << "<?xml version='1.0' encoding='UTF-8' ?>" << std::endl;
	inStream << "<itpanim>" << std::endl;
	inStream << "\t<skeleton count='" << mSkeleton.mJoints.size() << "'>" << std::endl;
	for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		inStream << "\t\t<joint id='" << i << "' name='" << mSkeleton.mJoints[i].mName << "' parent='" << mSkeleton.mJoints[i].mParentIndex << "'>\n";
		inStream << "\t\t\t";
		FbxVector4 translation = mSkeleton.mJoints[i].mGlobalBindposeInverse.GetT();
		FbxVector4 rotation = mSkeleton.mJoints[i].mGlobalBindposeInverse.GetR();
		translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
		rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
		mSkeleton.mJoints[i].mGlobalBindposeInverse.SetT(translation);
		mSkeleton.mJoints[i].mGlobalBindposeInverse.SetR(rotation);
		FbxMatrix out = mSkeleton.mJoints[i].mGlobalBindposeInverse;

		Utilities::WriteMatrix(inStream, out.Transpose(), true);
		inStream << "\t\t</joint>\n";
	}
	inStream << "\t</skeleton>\n";
	inStream << "\t<animations>\n";
	inStream << "\t\t<animation name='" << mAnimationName << "' length='" << mAnimationLength << "'>\n";
	for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		inStream << "\t\t\t" << "<track id = '" << i << "' name='" << mSkeleton.mJoints[i].mName << "'>\n";
		Keyframe* walker = mSkeleton.mJoints[i].mAnimation;
		while(walker)
		{
			inStream << "\t\t\t\t" << "<frame num='" << walker->mFrameNum - 1 << "'>\n";
			inStream << "\t\t\t\t\t";
			FbxVector4 translation = walker->mGlobalTransform.GetT();
			FbxVector4 rotation = walker->mGlobalTransform.GetR();
			translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
			rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
			walker->mGlobalTransform.SetT(translation);
			walker->mGlobalTransform.SetR(rotation);
			FbxMatrix out = walker->mGlobalTransform;
			Utilities::WriteMatrix(inStream, out.Transpose(), true);
			inStream << "\t\t\t\t" << "</frame>\n";
			walker = walker->mNext;
		}
		inStream << "\t\t\t" << "</track>\n";
	}
	inStream << "\t\t</animation>\n";
	inStream << "</animations>\n";
	inStream << "</itpanim>";
}

#include "FBXExporter.h"
#include "Utilities.h"
#include <fstream>
#include <sstream>
#include <iomanip>

// Include headers to make DBO object
#include "CObjectsC.h"

// External
extern int g_iFBXGeometryToggleMode;
extern int g_iFBXGeometryCenterMesh;

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

// globals to hold frame list during geometry traversal
sFrame** g_pFrameList = NULL;
int g_iFrameIndex = 0;

void FBXExporter::ExportFBX ( int iID )
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	
	// Get the clean name of the model
	std::string genericFileName = Utilities::GetFileName(mInputFilePath);
	genericFileName = Utilities::RemoveSuffix(genericFileName);
	mAnimationLength = 0;

	// create a DBO from the prepared model data
	bool bErr = CreateNewObject ( iID, "fbxmodel" );
	if ( bErr == false )
		return;

	// this is the object we are constructing
	sObject* pObject = g_ObjectList [ iID ];

	// work out FBX skeelton and joints structure
	ProcessSkeletonHierarchy(mFBXScene->GetRootNode());
	if(mSkeleton.mJoints.empty()) mHasAnimation = false;

	// count how many meshes (returns mMeshCount)
	ProcessMeshHierarchy(mFBXScene->GetRootNode());

	// work out max number of frames to create
	DWORD dwFrameCount = mSkeleton.mJoints.size();
	if ( mMeshCount > dwFrameCount ) dwFrameCount = mMeshCount;

	// create frames from skeleton(bone) information (one and the same in FBX)
	sFrame* pRootFrame = pObject->pFrame;
	g_pFrameList = NULL;
	if ( dwFrameCount > 0 )
	{
		g_pFrameList = new sFrame* [ dwFrameCount ];
		for ( int f = 0; f < dwFrameCount; f++ )
		{
			if ( f==0 )
			{
				// root frame
				if ( mSkeleton.mJoints.size() > 0 )
					strcpy ( pRootFrame->szName, const_cast<char *>(mSkeleton.mJoints[f].mName.c_str()) );
				else
					strcpy ( pRootFrame->szName, "root" );

				g_pFrameList [ f ] = pRootFrame;
			}
			else
			{
				// create a chain of frames
				sFrame* pFrame = new sFrame();
				if ( f < mSkeleton.mJoints.size() )
					strcpy ( pFrame->szName, const_cast<char *>(mSkeleton.mJoints[f].mName.c_str()) );
				else
					strcpy ( pRootFrame->szName, "frame" );

				g_pFrameList [ f ] = pFrame;
			}
		}
		// FBX does not like nested child frames (seems the animation is based on global root matrix changes)
		for ( int f = 1; f < dwFrameCount; f++ )
		{
			int iThisParentIndex = 0;
			if ( f < mSkeleton.mJoints.size() ) iThisParentIndex = mSkeleton.mJoints[f].mParentIndex;
			if ( iThisParentIndex >= 0 && 0 ) // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			{
				// associate this frame with parent, and inform parent of this child
				g_pFrameList [ f ]->pParent = g_pFrameList [ iThisParentIndex ];
				if ( g_pFrameList [ iThisParentIndex ]->pChild == NULL )
				{
					// first child assigned to parent
					g_pFrameList [ iThisParentIndex ]->pChild = g_pFrameList [ f ];
				}
				else
				{
					// already has first child, scan syblings and place at end of chain
					sFrame* pLatestChild = g_pFrameList [ iThisParentIndex ]->pChild;
					while ( pLatestChild->pSibling != NULL ) pLatestChild = pLatestChild->pSibling;
					pLatestChild->pSibling = g_pFrameList [ f ];
				}
			}
			else
			{
				// no parent needed, so add to chain of root syblings
				sFrame* pLatestSybling = pRootFrame;
				while ( pLatestSybling->pSibling != NULL ) pLatestSybling = pLatestSybling->pSibling;
				pLatestSybling->pSibling = g_pFrameList [ f ];
			}
		}
	}

	// process all meshes in model
	g_iFrameIndex = 0;
	ProcessGeometry(mFBXScene->GetRootNode(),pObject);

	// create animation set data for keyframe animation
	if ( mAnimationLength > 0 )
	{
		pObject->pAnimationSet = new sAnimationSet();
		strcpy ( pObject->pAnimationSet->szName, const_cast<char *>(mAnimationName.c_str()) );
		pObject->pAnimationSet->pAnimation = NULL;
		pObject->pAnimationSet->pvecBoundCenter = NULL;
		pObject->pAnimationSet->pfBoundRadius = NULL;
		pObject->pAnimationSet->pvecBoundMax = NULL;
		pObject->pAnimationSet->pvecBoundMin = NULL;
		pObject->pAnimationSet->ulLength = mAnimationLength;
		sAnimation* pLastAnim = NULL;
		for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
		{
			sAnimation* pAnim = new sAnimation();
			strcpy ( pAnim->szName, const_cast<char *>(mSkeleton.mJoints[i].mName.c_str()) );
			if ( pLastAnim == NULL )
			{
				// root first anim in chain
				pObject->pAnimationSet->pAnimation = pAnim;
			}
			else
			{
				// add to end of chain
				pLastAnim->pNext = pAnim;
			}
			pLastAnim = pAnim;

			// create all MATRIX key frames for this frame/bone
			pAnim->dwNumMatrixKeys = 0;
			Keyframe* walker = mSkeleton.mJoints[i].mAnimation;
			while(walker) { pAnim->dwNumMatrixKeys++; walker=walker->mNext; }
			pAnim->pMatrixKeys = new sMatrixKey[pAnim->dwNumMatrixKeys];
			pAnim->dwNumMatrixKeys = 0;
			walker = mSkeleton.mJoints[i].mAnimation;
			while(walker) 
			{
				pAnim->pMatrixKeys [ pAnim->dwNumMatrixKeys ].dwTime = walker->mFrameNum;

				//FbxVector4 translation = walker->mGlobalTransform.GetT();
				//FbxVector4 rotation = walker->mGlobalTransform.GetR();
				//translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
				//rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
				//walker->mGlobalTransform.SetT(translation);
				//walker->mGlobalTransform.SetR(rotation);

				FbxMatrix out = walker->mGlobalTransform;
				FbxMatrix transposed = out;//.Transpose();
				GGMATRIX matTransposed;
				for (int x=0; x<4; x++)
				{
					matTransposed(x, 0) = transposed.Get(x, 0);
					matTransposed(x, 1) = transposed.Get(x, 1);
					matTransposed(x, 2) = transposed.Get(x, 2);
					matTransposed(x, 3) = transposed.Get(x, 3);
				}
				pAnim->pMatrixKeys [ pAnim->dwNumMatrixKeys ].matMatrix = matTransposed;
				pAnim->dwNumMatrixKeys++; 
				walker=walker->mNext; 
			}
			if ( pAnim->dwNumMatrixKeys > 1 )
			{
				DWORD dwTime;
				for ( DWORD dwKey = 0; dwKey < pAnim->dwNumMatrixKeys - 1; dwKey++ )
				{
					pAnim->pMatrixKeys [ dwKey ].matInterpolation = pAnim->pMatrixKeys [ dwKey + 1 ].matMatrix - pAnim->pMatrixKeys [ dwKey ].matMatrix;
					dwTime = pAnim->pMatrixKeys [ dwKey + 1 ].dwTime - pAnim->pMatrixKeys [ dwKey ].dwTime;
					if ( !dwTime ) dwTime = 1;
					pAnim->pMatrixKeys [ dwKey ].matInterpolation /= ( float ) dwTime;
				}
			}
		}
	}

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, -1.0f );

	CleanupFbxManager();
	std::cout << "\n\nExport Done!\n";
}

void FBXExporter::ProcessGeometry(FbxNode* inNode,sObject* pObject)
{
	// Get the node’s default TRS properties
	FbxDouble3 myNodeLclTranslation = inNode->LclTranslation.Get();
	FbxDouble3 myNodeLclRotation    = inNode->LclRotation.Get(); // lee, this is being ignored for now!
	FbxDouble3 myNodeLclScaling     = inNode->LclScaling.Get();

	// test attribute to see if a mesh
	if (inNode->GetNodeAttribute())
	{
		FbxNodeAttribute* fbxna = inNode->GetNodeAttribute();
		FbxNodeAttribute::EType attribtype = fbxna->GetAttributeType();
		switch ( attribtype )
		{
		case FbxNodeAttribute::eMesh:

			// inits
			mVertices.clear();
			mTriangles.clear();

			// gather mesh data from this node
			int iWhichFrameIndex = -1;
			ProcessControlPoints(inNode);
			if(mHasAnimation)
			{
				iWhichFrameIndex = ProcessJointsAndAnimations(inNode);
			}
			ProcessMesh(inNode);
			AssociateMaterialToMesh(inNode);
			ProcessMaterials(inNode);

			// create DBO mesh to hold the above data
			sMesh* pMesh = NULL;
			int iUsingFrame = 0;
			if ( g_iFrameIndex == 0 )
			{
				// we must move mesh from root frame to where we need it
				if ( iWhichFrameIndex != -1 ) iUsingFrame = iWhichFrameIndex;
				pMesh = g_pFrameList[0]->pMesh;
				g_pFrameList[0]->pMesh = NULL;
				g_pFrameList[iUsingFrame]->pMesh = pMesh;
			}
			else
			{
				// create mesh, and associate it with 
				iUsingFrame = g_iFrameIndex;
				pMesh = new sMesh();
				if ( iWhichFrameIndex != -1 ) iUsingFrame = iWhichFrameIndex;
				g_pFrameList[iUsingFrame]->pMesh = pMesh;
			}
			g_iFrameIndex++;

			// correct frame associated with this new mesh
			sFrame* pFrame = g_pFrameList[iUsingFrame];

			// assign translation to frame (we need to ensure -Y and Z are twizzled)
			GGMATRIX matTran;
			GGMATRIX matRot;
			GGMATRIX matRotX;
			GGMATRIX matRotY;
			GGMATRIX matRotZ;

			// LEE FIRST STEP IS TO FOLLOW TUTORIAL:
			// https://www.gamedev.net/articles/programming/graphics/how-to-work-with-fbx-sdk-r3582/
			// AS CLOSE AS POSSIBLE, FORGET THE BODGES DONE WHEN THIS WAS FIRST TRIED
			////GGMatrixScaling ( &pFrame->matOriginal, myNodeLclScaling[0], myNodeLclScaling[2], -myNodeLclScaling[1] );
			////GGMatrixTranslation ( &matTran, myNodeLclTranslation[0], myNodeLclTranslation[2], -myNodeLclTranslation[1] );
			GGMatrixScaling ( &pFrame->matOriginal, myNodeLclScaling[0], myNodeLclScaling[1], myNodeLclScaling[2] );
			GGMatrixTranslation ( &matTran, myNodeLclTranslation[0], myNodeLclTranslation[1], -myNodeLclTranslation[2] );
			GGMatrixMultiply ( &pFrame->matOriginal, &pFrame->matOriginal, &matTran );
			////GGMatrixRotationX ( &matRotX, GGToRadian(myNodeLclRotation[0]) );
			////GGMatrixRotationY ( &matRotY, GGToRadian(myNodeLclRotation[1]) );
			////GGMatrixRotationZ ( &matRotZ, GGToRadian(myNodeLclRotation[2]) );
			GGMatrixRotationX ( &matRotX, -GGToRadian(myNodeLclRotation[0]) );
			GGMatrixRotationY ( &matRotY, -GGToRadian(myNodeLclRotation[1]) );
			GGMatrixRotationZ ( &matRotZ, GGToRadian(myNodeLclRotation[2]) );
			GGMatrixMultiply ( &matRot, &matRotX, &matRotY );
			GGMatrixMultiply ( &matRot, &matRot, &matRotZ );

			// populate vertex data from FBX model data
			pMesh->dwVertexCount = mVertices.size();
			DWORD dwVertElementCount = 3 + 3 + 2;
			pMesh->dwFVFSize = dwVertElementCount * sizeof(float);
			pMesh->pVertexData = (BYTE*)new float[pMesh->dwVertexCount*dwVertElementCount];
			pMesh->dwFVF = GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1;
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

			// if too many indices, use vertex only (already organised into triangles)
			if ( mTriangleCount > 21845 )
			{
				// NOTE: relies on larger meshes using triangles (not quad, five sided, etc)
				pMesh->pIndices = NULL;
				pMesh->dwIndexCount = 0;
				pMesh->iDrawVertexCount = pMesh->dwVertexCount;
				pMesh->iDrawPrimitives = pMesh->dwVertexCount / 3;
			}
			else
			{
				// populate index data from FBX model data
				pMesh->dwIndexCount = 0;
				for ( int i=0; i<mTriangleCount; i++ )
				{
					int iTriCount = mTriangles[i].mIndices.size() - 2;
					pMesh->dwIndexCount += (iTriCount*3);
				}
				pMesh->pIndices = (WORD*)new WORD[pMesh->dwIndexCount];
				WORD* pIndicePtr = pMesh->pIndices;
				for ( int i=0; i<mTriangleCount; i++ )
				{
					int iTriCount = mTriangles[i].mIndices.size() - 2;
					for ( int n = 0; n < iTriCount; n++ )
					{
						// for FBX twizzle, reorder for correct culling
						if ( g_iFBXGeometryToggleMode == 1 )
						{
							pIndicePtr[0+(n*3)] = mTriangles[i].mIndices[0];
							pIndicePtr[1+(n*3)] = mTriangles[i].mIndices[2+n];
							pIndicePtr[2+(n*3)] = mTriangles[i].mIndices[1+n];
						}
						else
						{
							pIndicePtr[0+(n*3)] = mTriangles[i].mIndices[0];
							pIndicePtr[1+(n*3)] = mTriangles[i].mIndices[1+n];
							pIndicePtr[2+(n*3)] = mTriangles[i].mIndices[2+n];
						}
					}
					pIndicePtr += (iTriCount*3);
				}

				// primitive count
				pMesh->iDrawVertexCount = pMesh->dwVertexCount;
				pMesh->iDrawPrimitives = pMesh->dwIndexCount / 3;
			}

			// fill bone information (only if this mesh has skin anim data)
			if ( mAnimationLengthThisTime > 0 )
			{
				pMesh->dwBoneCount = mSkeleton.mJoints.size();
				pMesh->pBones = new sBone [ pMesh->dwBoneCount ];
				for ( int b = 0; b < pMesh->dwBoneCount; b++ )
				{
					// gather all influences from skeleton data
					strcpy ( pMesh->pBones [ b ].szName, const_cast<char *>(mSkeleton.mJoints[b].mName.c_str()) );
					for ( int iPass=0; iPass<2; iPass++ )
					{
						DWORD dwNumInfluences = 0;
						for ( int v=0; v<pMesh->iDrawVertexCount; v++ )
						{
							for ( int four=0; four<4; four++ )
							{
								int iBlendBone = mVertices[v].mVertexBlendingInfos[four].mBlendingIndex;
								float fBlendWeight = mVertices[v].mVertexBlendingInfos[four].mBlendingWeight;
								if ( iBlendBone==b && fBlendWeight > 0.0f )
								{
									if ( iPass==1 )
									{
										pMesh->pBones [ b ].pVertices [ dwNumInfluences ] = v;
										pMesh->pBones [ b ].pWeights [ dwNumInfluences ] = fBlendWeight;
									}
									dwNumInfluences++;
								}
							}
						}
						if ( iPass==0 )
						{
							pMesh->pBones [ b ].dwNumInfluences = dwNumInfluences;
							pMesh->pBones [ b ].pVertices = new DWORD [ dwNumInfluences ];
							pMesh->pBones [ b ].pWeights = new float [ dwNumInfluences ];
						}
					}

					// calculate bone matrix from bindpose given by FBX
					//FbxVector4 translation = mSkeleton.mJoints[b].mGlobalBindposeInverse.GetT();
					//FbxVector4 rotation = mSkeleton.mJoints[b].mGlobalBindposeInverse.GetR();
					//translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
					//rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
					//mSkeleton.mJoints[b].mGlobalBindposeInverse.SetT(translation);
					//mSkeleton.mJoints[b].mGlobalBindposeInverse.SetR(rotation);
					
					FbxMatrix out = mSkeleton.mJoints[b].mGlobalBindposeInverse;
					FbxMatrix transposed = out;//.Transpose();
					GGMATRIX matTransposed;
					for (int x=0; x<4; x++)
					{
						matTransposed(x, 0) = transposed.Get(x, 0);
						matTransposed(x, 1) = transposed.Get(x, 1);
						matTransposed(x, 2) = transposed.Get(x, 2);
						matTransposed(x, 3) = transposed.Get(x, 3);
					}
					pMesh->pBones [ b ].matTranslation = matTransposed;
				}
			}

			// fill mesh with knowledge of textures
			char pMaterialTextureFilename[512];
			for ( int iTexPass = 0; iTexPass < 2; iTexPass++ )
			{
				if ( iTexPass == 1 ) 
				{
					if ( pMesh->dwTextureCount > 0 )
					{
						// prepare array to hold textures for mesh
						pMesh->pTextures = new sTexture[pMesh->dwTextureCount];
						memset ( pMesh->pTextures, 0, sizeof(pMesh->pTextures) );
					}
					else
					{
						// no textures associated with this mesh
						pMesh->pTextures = NULL;
					}
				}
				pMesh->dwTextureCount = 0;
				for ( int iTexTypeIndex = 0; iTexTypeIndex < 4; iTexTypeIndex++ )
				{
					strcpy ( pMaterialTextureFilename, "" );
					if ( mMaterialLookUp[0] )
					{
						if ( iTexTypeIndex == 0 ) strcpy ( pMaterialTextureFilename, mMaterialLookUp[0]->mDiffuseMapName.c_str() );
						if ( iTexTypeIndex == 1 ) strcpy ( pMaterialTextureFilename, mMaterialLookUp[0]->mNormalMapName.c_str() );
						if ( iTexTypeIndex == 2 ) strcpy ( pMaterialTextureFilename, mMaterialLookUp[0]->mSpecularMapName.c_str() );
						if ( iTexTypeIndex == 3 ) strcpy ( pMaterialTextureFilename, mMaterialLookUp[0]->mGlossMapName.c_str() );
					}
					if ( strlen ( pMaterialTextureFilename ) > 0 || iTexTypeIndex == 0 )
					{
						if ( iTexPass == 0 )
						{
							pMesh->dwTextureCount++;
						}
						else
						{
							strcpy ( pMesh->pTextures[pMesh->dwTextureCount].pName, pMaterialTextureFilename );
							pMesh->dwTextureCount++;
						}
					}
				}
			}
			break;
		}
	}

	for (int i = 0; i < inNode->GetChildCount(); ++i)
	{
		ProcessGeometry(inNode->GetChild(i),pObject);
	}
}

void FBXExporter::ProcessMeshHierarchy(FbxNode* inRootNode)
{
	mMeshCount = 0;
	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = inRootNode->GetChild(childIndex);
		ProcessMeshHierarchyRecursively(currNode);
	}
}
void FBXExporter::ProcessMeshHierarchyRecursively(FbxNode* inNode)
{
	if(inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		mMeshCount++;
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessMeshHierarchyRecursively(inNode->GetChild(i));
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

	// flip Z and Y values FBX -> GG
	if ( g_iFBXGeometryToggleMode == 1 )
	{
		// swap Y and Z values (GG is Y as up) and X for left-handed coordinate system
		for(unsigned int i = 0; i < ctrlPointCount; ++i)
		{
			float fStoreY = mControlPoints[i]->mPosition.y;
			mControlPoints[i]->mPosition.y = mControlPoints[i]->mPosition.z;
			mControlPoints[i]->mPosition.z = fStoreY;
			mControlPoints[i]->mPosition.x = mControlPoints[i]->mPosition.x;
		}
	}

	// gather bounds of mesh to find center
	// g_iFBXGeometryCenterMesh: 1-center all, 2-center floor
	if ( g_iFBXGeometryCenterMesh >= 1 )
	{
		float fX1 = 999999;
		float fX2 = -999999;
		float fY1 = 999999;
		float fY2 = -999999;
		float fZ1 = 999999;
		float fZ2 = -999999;
		for(unsigned int i = 0; i < ctrlPointCount; ++i)
		{
			float fX = mControlPoints[i]->mPosition.x;
			float fY = mControlPoints[i]->mPosition.y;
			float fZ = mControlPoints[i]->mPosition.z;
			if ( fX > fX2 ) fX2 = fX;
			if ( fX < fX1 ) fX1 = fX;
			if ( fY > fY2 ) fY2 = fY;
			if ( fY < fY1 ) fY1 = fY;
			if ( fZ > fZ2 ) fZ2 = fZ;
			if ( fZ < fZ1 ) fZ1 = fZ;
		}
		float fCenterX = fX1 + ((fX2 - fX1)/2.0f);
		float fCenterY = fY1;// + ((fY2 - fY1)/2.0f); // ensure all imports are placed on virtual floor by default
		if ( g_iFBXGeometryCenterMesh == 2 ) fCenterY = fY1 + ((fY2 - fY1)/2.0f); 
		float fCenterZ = fZ1 + ((fZ2 - fZ1)/2.0f);

		// shift control points so mesh always on floor, centered on the XZ
		for(unsigned int i = 0; i < ctrlPointCount; ++i)
		{
			mControlPoints[i]->mPosition.x -= fCenterX;
			mControlPoints[i]->mPosition.y -= fCenterY;
			mControlPoints[i]->mPosition.z -= fCenterZ;
		}
	}
}

int FBXExporter::ProcessJointsAndAnimations(FbxNode* inNode)
{
	int iAssociateThisMeshWithJointIndex = 0;

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
	mAnimationLengthThisTime = 0;
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
			iAssociateThisMeshWithJointIndex = currJointIndex;

			FbxAMatrix transformMatrix;						
			FbxAMatrix transformLinkMatrix;					
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space

			if ( currJointIndex == 0 )
			{
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
			}
			else
			{
				// LEE, almost got it, just need to figure  out the relationship of the BOX
				// with the current matrix cauclations......
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * geometryTransform;
				//globalBindposeInverseMatrix.SetIdentity();
			}

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
			FbxTime::EMode timemode = mFBXScene->GetGlobalSettings().GetTimeMode();

			mAnimationLengthThisTime = end.GetFrameCount(timemode) - start.GetFrameCount(timemode) + 1;
			if ( mAnimationLengthThisTime > mAnimationLength ) mAnimationLength = mAnimationLengthThisTime;
			Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;

			for (FbxLongLong i = start.GetFrameCount(timemode); i <= end.GetFrameCount(timemode); ++i)
			{
				FbxTime currTime;
				currTime.SetFrame(i, timemode);
				*currAnim = new Keyframe();
				(*currAnim)->mFrameNum = i;
				FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
				(*currAnim)->mGlobalTransform = currentTransformOffset;//.Inverse();
				FbxAMatrix matAnotherOne = currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				(*currAnim)->mGlobalTransform = matAnotherOne;
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

	// we can use this to place mesh in correct joint/limb/frame
	return iAssociateThisMeshWithJointIndex;
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
	MessageBox ( GetForegroundWindow(), "FBX Import", "Skeleton information in FBX file is corrupted", MB_OK | MB_TOPMOST );
	return 0;
}

void FBXExporter::ProcessMesh(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	mTriangleCount = currMesh->GetPolygonCount();
	int vertexCounter = 0;
	mTriangles.reserve(mTriangleCount);

	XMFLOAT3 normal[8];
	XMFLOAT3 tangent[8];
	XMFLOAT3 binormal[8];
	XMFLOAT2 UV[8][2];
	for (unsigned int i = 0; i < mTriangleCount; ++i)
	{
		Triangle currTriangle;
		mTriangles.push_back(currTriangle);

		// mPolygons[pPolygonIndex].mSize can be 3 for triangle faces and 4 for quad faces (or more via FBX)
		int iPolySize = currMesh->mPolygons[i].mSize;
		if ( iPolySize > 8 ) iPolySize = 8; // limit to 8 sided polygons for the moment
		for (unsigned int j = 0; j < iPolySize; ++j)
		{
			// get control point from face
			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];

			// read in the normal
			ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);

			// if twizzle in effect, also twizzle normals (GG is Y as up)
			if ( g_iFBXGeometryToggleMode == 1 )
			{
				float fStoreY = normal[j].y;
				normal[j].y = normal[j].z;
				normal[j].z = fStoreY;
				normal[j].x = normal[j].x;
			}

			// we only have diffuse texture UV data right now
			for (int k = 0; k < 1; ++k)
			{
				ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]);
				UV[j][k].y *= -1; // 130416 - fix so UV mapping matches GG rendering expectation
			}

			// create final vertex template for all assembled data
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
			temp.SortBlendingInfoByWeight();

			// add vertex and index to collection
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
		outUV.x = 0;
		outUV.y = 0;
		return;
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
		}
		break;
	}
}

void FBXExporter::ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal)
{
	if(inMesh->GetElementNormalCount() < 1)
	{
		outNormal.x = 0;
		outNormal.y = 0;
		outNormal.z = 0;
		return;
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
		}
		break;
	}
}

void FBXExporter::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal)
{
	if(inMesh->GetElementBinormalCount() < 1)
	{
		outBinormal.x = 0;
		outBinormal.y = 0;
		outBinormal.z = 0;
		return;
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
		}
		break;
	}
}

void FBXExporter::ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outTangent)
{
	if(inMesh->GetElementTangentCount() < 1)
	{
		outTangent.x = 0;
		outTangent.y = 0;
		outTangent.z = 0;
		return;
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
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
			MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid Reference", MB_OK | MB_TOPMOST );
		}
		break;
	}
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
				MessageBox ( GetForegroundWindow(), "FBX Import", "Invalid mapping mode for material", MB_OK | MB_TOPMOST );
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
					MessageBox ( GetForegroundWindow(), "FBX Import", "Layered Texture is currently unsupported", MB_OK | MB_TOPMOST );
					return;
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
								ioMaterial->mDiffuseMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "Bump" || textureType == "NormalMap")
							{
								ioMaterial->mNormalMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "SpecularColor")
							{
								ioMaterial->mSpecularMapName = fileTexture->GetRelativeFileName();
							}
							else if (textureType == "ShininessExponent")
							{
								ioMaterial->mGlossMapName = fileTexture->GetRelativeFileName();
							}
						}
					}
				}
			}
		}
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

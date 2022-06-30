//
// CCollision Functions Implementation
//

#include "CCollision.h"
#include "CCameraC.h"
#include "CObjectsC.h"

bool						g_bGlobalCollisionActive		= true;
bool						g_bAutoColStarted				= false;
bool						bResponseDataAvailableFromDBV1	= false;
extern GGVECTOR3			gvLatestObjectCollisionResult;
int							m_iCameraUsingForCollision		= 0;
int							g_iCameraCollisionBehaviourMode = 0;
float						fLastCamX, fLastCamY, fLastCamZ;
sObject						CameraObject;
sMegaCollisionFeedback		MegaCollisionFeedback;

// 201115 - temp expandable arrays for new accurate ray cast collision detector
DWORD g_dwMasterTableBoneCount = 0;
GGMATRIX* g_matMasterTableBoneMatrices = NULL;
DWORD g_dwMasterTableVertexCount = 0;
BYTE* g_pMasterTableWeights = NULL;
DWORD g_dwMasterTablePrimitiveCount = 0;
float* g_pMasterTableMeshCopy = NULL;
// Set this flag to true to have raycast check against a box first, Set it false after performing the check
bool RayCollisionDoBoxCheckFirst = false;

extern int tgamesetismapeditormode;

// Collision Checklist 

void CreateCollisionChecklist ( void )
{
	// create a checklist first
	g_pGlob->checklisthasvalues  = true;
	g_pGlob->checklisthasstrings = true;
	g_pGlob->checklistexists = true;
	
	// store the number of items in the checklist
	char szTempString [ 256 ];
	g_pGlob->checklistqty = 9;

	// create space for checklist information
	for ( int c=0; c<g_pGlob->checklistqty; c++ ) GlobExpandChecklist ( c, 256 );
	
	// 1 - frame indexes
	// checklist value A = mesh number
	// checklist value B = related frame number (if A is bone mesh)
	sprintf ( szTempString, "%d,%d", MegaCollisionFeedback.iFrameCollision, MegaCollisionFeedback.iFrameRelatedToBone );
	strcpy ( g_pGlob->checklist [ 0 ].string, szTempString );
	g_pGlob->checklist [ 0 ].valuea = MegaCollisionFeedback.iFrameCollision;
	g_pGlob->checklist [ 0 ].valueb = MegaCollisionFeedback.iFrameRelatedToBone;

	// 2 - vertex indexes
	// checklist value A = V0
	// checklist value B = V1
	// checklist value C = V2
	sprintf ( szTempString, "%d,%d,%d", MegaCollisionFeedback.dwVertex0IndexOfHitPoly, MegaCollisionFeedback.dwVertex1IndexOfHitPoly, MegaCollisionFeedback.dwVertex2IndexOfHitPoly );
	strcpy ( g_pGlob->checklist [ 1 ].string, szTempString );
	g_pGlob->checklist [ 1 ].valuea = MegaCollisionFeedback.dwVertex0IndexOfHitPoly;
	g_pGlob->checklist [ 1 ].valueb = MegaCollisionFeedback.dwVertex1IndexOfHitPoly;
	g_pGlob->checklist [ 1 ].valuec = MegaCollisionFeedback.dwVertex2IndexOfHitPoly;

	// 3 - object-space coordinate of V0
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vec0Hit.x, MegaCollisionFeedback.vec0Hit.y, MegaCollisionFeedback.vec0Hit.z );
	strcpy ( g_pGlob->checklist [ 2 ].string, szTempString );
	g_pGlob->checklist [ 2 ].fvaluea = MegaCollisionFeedback.vec0Hit.x;
	g_pGlob->checklist [ 2 ].fvalueb = MegaCollisionFeedback.vec0Hit.y;
	g_pGlob->checklist [ 2 ].fvaluec = MegaCollisionFeedback.vec0Hit.z;

	// 4 - object-space coordinate of V1
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vec1Hit.x, MegaCollisionFeedback.vec1Hit.y, MegaCollisionFeedback.vec1Hit.z );
	strcpy ( g_pGlob->checklist [ 3 ].string, szTempString );
	g_pGlob->checklist [ 3 ].fvaluea = MegaCollisionFeedback.vec1Hit.x;
	g_pGlob->checklist [ 3 ].fvalueb = MegaCollisionFeedback.vec1Hit.y;
	g_pGlob->checklist [ 3 ].fvaluec = MegaCollisionFeedback.vec1Hit.z;

	// 5 - object-space coordinate of V2
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vec2Hit.x, MegaCollisionFeedback.vec2Hit.y, MegaCollisionFeedback.vec2Hit.z );
	strcpy ( g_pGlob->checklist [ 4 ].string, szTempString );
	g_pGlob->checklist [ 4 ].fvaluea = MegaCollisionFeedback.vec2Hit.x;
	g_pGlob->checklist [ 4 ].fvalueb = MegaCollisionFeedback.vec2Hit.y;
	g_pGlob->checklist [ 4 ].fvaluec = MegaCollisionFeedback.vec2Hit.z;

	// 6 - world space coordinate where the collision struck!
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vecHitPoint.x, MegaCollisionFeedback.vecHitPoint.y, MegaCollisionFeedback.vecHitPoint.z );
	strcpy ( g_pGlob->checklist [ 5 ].string, szTempString );
	g_pGlob->checklist [ 5 ].fvaluea = MegaCollisionFeedback.vecHitPoint.x;
	g_pGlob->checklist [ 5 ].fvalueb = MegaCollisionFeedback.vecHitPoint.y;
	g_pGlob->checklist [ 5 ].fvaluec = MegaCollisionFeedback.vecHitPoint.z;

	// 7 - normal of polygon struck (from vertA)
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vecNormal.x, MegaCollisionFeedback.vecNormal.y, MegaCollisionFeedback.vecNormal.z );
	strcpy ( g_pGlob->checklist [ 6 ].string, szTempString );
	g_pGlob->checklist [ 6 ].fvaluea = MegaCollisionFeedback.vecNormal.x;
	g_pGlob->checklist [ 6 ].fvalueb = MegaCollisionFeedback.vecNormal.y;
	g_pGlob->checklist [ 6 ].fvaluec = MegaCollisionFeedback.vecNormal.z;

	// 8 - reflection normal based on angle of impact
	// checklist value A, B, C = X, Y, Z
	sprintf ( szTempString, "%f,%f,%f", MegaCollisionFeedback.vecReflectedNormal.x, MegaCollisionFeedback.vecReflectedNormal.y, MegaCollisionFeedback.vecReflectedNormal.z );
	strcpy ( g_pGlob->checklist [ 7 ].string, szTempString );
	g_pGlob->checklist [ 7 ].fvaluea = MegaCollisionFeedback.vecReflectedNormal.x;
	g_pGlob->checklist [ 7 ].fvalueb = MegaCollisionFeedback.vecReflectedNormal.y;
	g_pGlob->checklist [ 7 ].fvaluec = MegaCollisionFeedback.vecReflectedNormal.z;

	// 9 - arbitary value
	sprintf ( szTempString, "%d", MegaCollisionFeedback.dwArbitaryValue );
	strcpy ( g_pGlob->checklist [ 8 ].string, szTempString );
	g_pGlob->checklist [ 8 ].valuea = MegaCollisionFeedback.dwArbitaryValue;
	g_pGlob->checklist [ 8 ].valueb = 0;
	g_pGlob->checklist [ 8 ].valuec = 0;
}

// Polygon Math Functions
bool GetRayCollisionEx ( sObject* pObject, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float* fDistance, int iIgnoreAllButLastFrame )
{
	// Actual Object for data, this object for matWorld
	sObject* pActualObject = pObject;
	if ( pActualObject->pInstanceOfObject )
		pActualObject = pActualObject->pInstanceOfObject;

	// Result is distance of intersect (default is zero)
	float fDistToIntersect=0.0f;

	// Some information to return from ray-collision detection
	int iFrameCollision = 0;
	sMesh* pMeshThatHasBeenHitRef = NULL;
	DWORD dwVertex0IndexOfHitPoly = 0;
	DWORD dwVertex1IndexOfHitPoly = 0;
	DWORD dwVertex2IndexOfHitPoly = 0;
	GGVECTOR3 vec0Hit, vec1Hit, vec2Hit;
	GGVECTOR3 vecSurfaceNormal;
	GGVECTOR3 vecHitPoint;
	DWORD dwArbValueDetected = 0;

	// Frames to participate in this intersect test
	int iScanFrameStart = 0;
	if (iIgnoreAllButLastFrame == 1)
	{
		for (int iFrame = 0; iFrame < pActualObject->iFrameCount; iFrame++)
			if (pActualObject->ppFrameList[iFrame]->pMesh)
				iScanFrameStart = iFrame;
	}

	// TRADITIONAL POLYGON TEST AGAINST RAY
	int iUseNewTransformedVertsMode = false;
	//if ( pObject->pAnimationSet ) iUseNewTransformedVertsMode = true;
	if ( pActualObject->pAnimationSet ) iUseNewTransformedVertsMode = true;
	if ( iUseNewTransformedVertsMode==false || pObject->bIgnoreDefAnim==true )
	{
		// Check aligned line against object meshes
		for ( int iFrame=iScanFrameStart; iFrame<pActualObject->iFrameCount; iFrame++ )
		{
			// get frame ptr
			sFrame* pFrame = pActualObject->ppFrameList[iFrame];
			sMesh* pMesh = pFrame->pMesh;
			if ( pMesh )
			{
				// leeadd - 310305 - this is set if the object is not culled
				bool bRejectCulledPolysPointAway=false;

				// mike - 041005 - we can't do this, even polygons facing away must be tested
				//if ( pMesh->bCull==true ) bRejectCulledPolysPointAway=true;

				// leeadd - 010604 - if instance limb visibility hidden, skip now
				if ( pObject->pInstanceMeshVisible )
				{
					// if limb in instance hidden, skip
					if ( pObject->pInstanceMeshVisible [ iFrame ]==false )
						continue;
				}

				// leefix - 280305 - sphere uses tristrip by default, switch it if collision required on it
				ConvertLocalMeshToTriList ( pMesh );

				// leeadd - 280504 - calculate correct absolute world matrix (UpdateRealtimeFrameVectors might have been better)
				CalculateAbsoluteWorldMatrix ( pObject, pFrame, pMesh );

				// 190115 - some models have animations which may offset the final vertex positions, so account for here
				GGMATRIX matWorld = pFrame->matAbsoluteWorld;
				//if ( pObject->pAnimationSet && pObject->bIgnoreDefAnim==false )
				if ( pActualObject->pAnimationSet && pObject->bIgnoreDefAnim==false )
				{
					//if ( pObject->pAnimationSet->pAnimation )
					if ( pActualObject->pAnimationSet->pAnimation )
					{
						matWorld = pFrame->matCombined;
						GGMatrixMultiply ( &matWorld, &matWorld, &pFrame->matAbsoluteWorld );
					}
				}

				// Calculate inverse based on limb world orientation
				GGMATRIX matInv;
				GGMatrixInverse(&matInv, NULL, &matWorld );

				// Calculate actual line vectors based on position of target object
				GGVECTOR3 StartVector = GGVECTOR3( fX, fY, fZ );
				GGVECTOR3 EndVector = GGVECTOR3( fNewX, fNewY, fNewZ );
				GGVECTOR3 OriginalVector = EndVector - StartVector;
				float fOriginalLength = GGVec3Length ( &OriginalVector );
				GGVec3TransformCoord( &StartVector, &StartVector, &matInv );
				GGVec3TransformCoord( &EndVector, &EndVector, &matInv );
				GGVECTOR3 DirVector = EndVector - StartVector;
				float fOrientedLength = GGVec3Length ( &DirVector );
				GGVec3Normalize( &DirVector, &DirVector );
				float fDistanceModifier = fOriginalLength/fOrientedLength;

				// Check for intersect with triangle in mesh
				BYTE* Ptr = pMesh->pVertexData;
				if ( Ptr )
				{
					// vertex data vars
					float* pV = (float*)Ptr;
					DWORD dwSizeInFloats = (DWORD)(pMesh->dwFVFSize/4);

					// index data vars
					unsigned short*	IndexPtr = pMesh->pIndices;
					if ( IndexPtr==NULL )
					{
						// no index - standard 3vert faces
						int iVertexIndex = 0;

						// load polygon information into structures
						for ( int i = 0; i < pMesh->iDrawPrimitives; i++ )
						{
							// triangle indices
							//leefix - 040803 - for vertexdata only, can use DWORD and not WORD!
							DWORD v0 = iVertexIndex+0;
							DWORD v1 = iVertexIndex+1;
							DWORD v2 = iVertexIndex+2;
							iVertexIndex+=3;

							// triangle vertice ptrs
							GGVECTOR3* pVec0 = (GGVECTOR3*)(pV+((v0*dwSizeInFloats)));
							GGVECTOR3* pVec1 = (GGVECTOR3*)(pV+((v1*dwSizeInFloats)));
							GGVECTOR3* pVec2 = (GGVECTOR3*)(pV+((v2*dwSizeInFloats)));

							// leeadd - 310305 - check if ray facing poly, or the poly is culled (on a flag)
							GGVECTOR3 vNormal;
							if ( bRejectCulledPolysPointAway==true )
							{
								// direction of polygon
								GGVec3Cross ( &vNormal, &( *pVec2 - *pVec1 ), &( *pVec1 - *pVec0 ) );
								GGVec3Normalize ( &vNormal, &vNormal );
								float fDotProduct = GGVec3Dot ( &vNormal, &DirVector );
								if ( fDotProduct < 0.0f )
									continue;
							}

							// test ray with triangle
							float fU, fV, fThisDistance;
							BOOL bIntersectionTrue = GGIntersectTri(pVec0,pVec1,pVec2,&StartVector,&DirVector,&fU,&fV,&fThisDistance);
							if ( bIntersectionTrue == TRUE )
							{
								// always use actual distance (not scaled version)
								fThisDistance *= fDistanceModifier;

								// record smallest distance of intersect
								if ( fThisDistance > 0.0f )
								{
									if ( fDistToIntersect == 0.0f || fThisDistance < fDistToIntersect )
									{
										fDistToIntersect = fThisDistance;
										iFrameCollision = iFrame;
										pMeshThatHasBeenHitRef = pMesh;
										dwVertex0IndexOfHitPoly = v0;
										dwVertex1IndexOfHitPoly = v1;
										dwVertex2IndexOfHitPoly = v2;
										vec0Hit = *pVec0;
										vec1Hit = *pVec1;
										vec2Hit = *pVec2;
										dwArbValueDetected = pMesh->Collision.dwArbitaryValue;

										// work out normal for surface direction
										GGVec3Cross ( &vNormal, &( *pVec2 - *pVec1 ), &( *pVec0 - *pVec1 ) );
										GGVec3Normalize ( &vNormal, &vNormal );
										vecSurfaceNormal.x = vNormal.x;
										vecSurfaceNormal.y = vNormal.y;
										vecSurfaceNormal.z = vNormal.z;
									}
								}
							}
						}
					}
					else
					{
						// load polygon information into structures
						for ( int i = 0; i < pMesh->iDrawPrimitives; i++ )
						{
							// triangle indices
							unsigned short v0 = *(IndexPtr+0);
							unsigned short v1 = *(IndexPtr+1);
							unsigned short v2 = *(IndexPtr+2);
							IndexPtr+=3;

							// triangle vertice ptrs
							GGVECTOR3* pVec0 = (GGVECTOR3*)(pV+((v0*dwSizeInFloats)));
							GGVECTOR3* pVec1 = (GGVECTOR3*)(pV+((v1*dwSizeInFloats)));
							GGVECTOR3* pVec2 = (GGVECTOR3*)(pV+((v2*dwSizeInFloats)));

							// leeadd - 310305 - check if ray facing poly, or the poly is culled (on a flag)
							GGVECTOR3 vNormal;
							if ( bRejectCulledPolysPointAway==true )
							{
								// direction of polygon
								GGVec3Cross ( &vNormal, &( *pVec2 - *pVec1 ), &( *pVec1 - *pVec0 ) );
								GGVec3Normalize ( &vNormal, &vNormal );
								float fDotProduct = GGVec3Dot ( &vNormal, &DirVector );
								if ( fDotProduct < 0.0f )
									continue;
							}

							// check tri
							float fU, fV, fThisDistance;
							if ( GGIntersectTri(pVec0,pVec1,pVec2,&StartVector,&DirVector,&fU,&fV,&fThisDistance) == TRUE )
							{
								// always use actual distance (not scaled version)
								fThisDistance *= fDistanceModifier;

								// record smallest distance of intersect
								if ( fThisDistance > 0.0f )
								{
									if ( fDistToIntersect == 0.0f || fThisDistance < fDistToIntersect )
									{
										// store useful info
										fDistToIntersect = fThisDistance;
										iFrameCollision = iFrame;
										pMeshThatHasBeenHitRef = pMesh;
										dwVertex0IndexOfHitPoly = v0;
										dwVertex1IndexOfHitPoly = v1;
										dwVertex2IndexOfHitPoly = v2;
										vec0Hit = *pVec0;
										vec1Hit = *pVec1;
										vec2Hit = *pVec2;
										dwArbValueDetected = pMesh->Collision.dwArbitaryValue;

										// work out normal for surface direction
										GGVec3Cross ( &vNormal, &( *pVec2 - *pVec1 ), &( *pVec0 - *pVec1 ) );
										GGVec3Normalize ( &vNormal, &vNormal );
										vecSurfaceNormal.x = vNormal.x;
										vecSurfaceNormal.y = vNormal.y;
										vecSurfaceNormal.z = vNormal.z;
									}
								}
							}
						}
					}
				}

				// transform any vecNormal by world matrix so its a world normal
				GGVec3TransformNormal( &vecSurfaceNormal, &vecSurfaceNormal, &matWorld );
			}
		}
	}
	else
	{
		// Check aligned line against object meshes
		for ( int iFrame=iScanFrameStart; iFrame<pActualObject->iFrameCount; iFrame++ )
		{
			// get frame ptr
			sFrame* pFrame = pActualObject->ppFrameList[iFrame];
			if ( pObject->pInstanceMeshVisible )
				if ( pObject->pInstanceMeshVisible [ iFrame ]==false )
					continue;

			// get mesh ptr
			sMesh* pMesh = pFrame->pMesh;
			if ( pMesh )
			{
				// first time around, copy vertex data to original-store
				if ( pMesh->pOriginalVertexData==NULL )
				{
					// first time around, copy vertex data to original-store
					DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
					pMesh->pOriginalVertexData = (BYTE*)new char [ dwTotalVertSize ];
					memcpy ( pMesh->pOriginalVertexData, pMesh->pVertexData, dwTotalVertSize );
				}

				// A system to animate the vertices using bone matrices for accurate hit detection
				if ( pMesh->dwBoneCount > g_dwMasterTableBoneCount )
				{
					g_dwMasterTableBoneCount = pMesh->dwBoneCount;
					SAFE_DELETE(g_matMasterTableBoneMatrices);
					g_matMasterTableBoneMatrices = new GGMATRIX [ g_dwMasterTableBoneCount ];
				}
				if ( pMesh->dwVertexCount > g_dwMasterTableVertexCount )
				{
					g_dwMasterTableVertexCount = pMesh->dwVertexCount;
					SAFE_DELETE(g_pMasterTableWeights);
					g_pMasterTableWeights = new BYTE [ pMesh->dwVertexCount ];
				}
				memset ( g_pMasterTableWeights, 0, pMesh->dwVertexCount );

				// create final matrices for transforming verts
				for ( DWORD dwMatrixIndex = 0; dwMatrixIndex < pMesh->dwBoneCount; dwMatrixIndex++ )
				{
					if ( pMesh->pFrameMatrices [ dwMatrixIndex ] )
						GGMatrixMultiply ( &g_matMasterTableBoneMatrices [ dwMatrixIndex ], &pMesh->pBones [ dwMatrixIndex ].matTranslation, pMesh->pFrameMatrices [ dwMatrixIndex ] );
					else
						memcpy ( &g_matMasterTableBoneMatrices [ dwMatrixIndex ], &pMesh->pBones [ dwMatrixIndex ].matTranslation, sizeof(GGMATRIX) );
				}

				// transform verts using all bone influences
				if ( pMesh->iDrawPrimitives > (int)g_dwMasterTablePrimitiveCount )
				{
					g_dwMasterTablePrimitiveCount = pMesh->iDrawPrimitives;
					SAFE_DELETE(g_pMasterTableMeshCopy);
					g_pMasterTableMeshCopy = new float [ g_dwMasterTablePrimitiveCount * 3 * 3 ];
					memset ( g_pMasterTableMeshCopy, 0, sizeof(float)*g_dwMasterTablePrimitiveCount * 3 * 3 );
				}
				for ( int iBone = 0; iBone < ( int ) pMesh->dwBoneCount; iBone++ )
				{
					// go through all influenced bones
					for ( int iLoop = 0; iLoop < ( int ) pMesh->pBones [ iBone ].dwNumInfluences; iLoop++ )
					{
						// get the correct vertex and weight
						int iOffset = pMesh->pBones [ iBone ].pVertices [ iLoop ];
						float fWeight = pMesh->pBones [ iBone ].pWeights [ iLoop ];

						// Vertex Data Ptrs
						float* pDestVertexBase = (float*)(g_pMasterTableMeshCopy + ( 3 * iOffset ));
						float* pVertexBase = (float*)(pMesh->pOriginalVertexData + ( pMesh->dwFVFSize * iOffset ));

						// initially clear temp vertex if changing it
						if ( g_pMasterTableWeights [ iOffset ]==0 ) memset ( pDestVertexBase, 0, 12 );
						g_pMasterTableWeights [ iOffset ] = 1;
			
						//PE: Slow 14%

						// get original vertex position
						GGVECTOR3 vec = GGVECTOR3 ( *(pVertexBase+0), *(pVertexBase+1), *(pVertexBase+2) );

						// multiply the vector and the bone matrix with weight
						GGVECTOR3 newVec;
						GGMATRIX* pMat = &(g_matMasterTableBoneMatrices [ iBone ]);
						newVec.x = vec.x * pMat->_11 + vec.y * pMat->_21 + vec.z * pMat->_31 + pMat->_41;
						newVec.y = vec.x * pMat->_12 + vec.y * pMat->_22 + vec.z * pMat->_32 + pMat->_42;
						newVec.z = vec.x * pMat->_13 + vec.y * pMat->_23 + vec.z * pMat->_33 + pMat->_43;
						newVec = newVec * fWeight;
			
						// accumilate vertex for final result
						*(pDestVertexBase+0) += newVec.x;
						*(pDestVertexBase+1) += newVec.y;
						*(pDestVertexBase+2) += newVec.z;
					}
				}

				// Calculate inverse based on object world orientation (per frame handled by temp bone anim above)
				GGMATRIX matInv;
				GGMATRIX matWorldToUse = pObject->position.matWorld;
				if ( pMesh->dwBoneCount == 0 )
				{
					// 220618 - using old pos/rot/scl anim system (i.e. door_c.x)
					// uses the instanced world but needs to adjust by instance/ormaster anim frame combine matrix
					GGMatrixMultiply(&matWorldToUse,&matWorldToUse,&pFrame->matCombined);
				}
				GGMatrixInverse(&matInv, NULL, &matWorldToUse );

				// Calculate actual line vectors based on position of target object
				GGVECTOR3 StartVector = GGVECTOR3( fX, fY, fZ );
				GGVECTOR3 EndVector = GGVECTOR3( fNewX, fNewY, fNewZ );
				GGVECTOR3 OriginalVector = EndVector - StartVector;
				float fOriginalLength = GGVec3Length ( &OriginalVector );
				GGVec3TransformCoord( &StartVector, &StartVector, &matInv );
				GGVec3TransformCoord( &EndVector, &EndVector, &matInv );
				GGVECTOR3 DirVector = EndVector - StartVector;
				float fOrientedLength = GGVec3Length ( &DirVector );
				GGVec3Normalize( &DirVector, &DirVector );
				float fDistanceModifier = fOriginalLength/fOrientedLength;

				// Check for intersect with triangle in mesh (060117 - if no bones, revert to vertexdata)
				float* pV = g_pMasterTableMeshCopy;
				DWORD dwVertexStride = 3;
				if ( pMesh->dwBoneCount == 0 ) 
				{
					pV = (float*)pMesh->pVertexData;
					dwVertexStride = pMesh->dwFVFSize / 4;
				}
				if ( pV )
				{
					unsigned short*	IndexPtr = pMesh->pIndices;
					if ( IndexPtr != NULL )
					{
						// indexed verts
						for ( int i = 0; i < pMesh->iDrawPrimitives; i++ )
						{
							// get a triangle
							unsigned short v0 = *(IndexPtr+0);
							unsigned short v1 = *(IndexPtr+1);
							unsigned short v2 = *(IndexPtr+2);
							IndexPtr+=3;
							GGVECTOR3* pVec0 = (GGVECTOR3*)(pV+((v0*dwVertexStride)));
							GGVECTOR3* pVec1 = (GGVECTOR3*)(pV+((v1*dwVertexStride)));
							GGVECTOR3* pVec2 = (GGVECTOR3*)(pV+((v2*dwVertexStride)));

							// this triangle, transform by all bone influences
							float fU, fV, fThisDistance;
							if ( GGIntersectTri(pVec0,pVec1,pVec2,&StartVector,&DirVector,&fU,&fV,&fThisDistance) == TRUE )
							{
								// always use actual distance (not scaled version)
								fThisDistance *= fDistanceModifier;

								// record smallest distance of intersect
								if ( fThisDistance > 0.0f )
								{
									if ( fDistToIntersect == 0.0f || fThisDistance < fDistToIntersect )
									{
										// store useful info
										fDistToIntersect = fThisDistance;
										iFrameCollision = iFrame;
										pMeshThatHasBeenHitRef = pMesh;
										dwVertex0IndexOfHitPoly = v0;
										dwVertex1IndexOfHitPoly = v1;
										dwVertex2IndexOfHitPoly = v2;
										vec0Hit = *pVec0;
										vec1Hit = *pVec1;
										vec2Hit = *pVec2;
										dwArbValueDetected = pMesh->Collision.dwArbitaryValue;

										// work out normal for surface direction
										GGVECTOR3 vNormal;
										GGVec3Cross ( &vNormal, &( *pVec2 - *pVec1 ), &( *pVec0 - *pVec1 ) );
										GGVec3Normalize ( &vNormal, &vNormal );
										vecSurfaceNormal = vNormal;
									}
								}
							}
						}
					}
				}

				// transform any vecNormal by world matrix so its a world normal
				GGVec3TransformNormal( &vecSurfaceNormal, &vecSurfaceNormal, &matWorldToUse );
			}
		}
	}

	// copy distance to output param
	*fDistance = fDistToIntersect;
	if ( fDistToIntersect > 0.0f )
	{
		// calculate actual hit point location
		GGVECTOR3 StartVector = GGVECTOR3( fX, fY, fZ );
		GGVECTOR3 EndVector = GGVECTOR3( fNewX, fNewY, fNewZ );
		GGVECTOR3 DirVector = EndVector - StartVector;
		GGVec3Normalize( &DirVector, &DirVector );
		vecHitPoint = StartVector + ( DirVector * fDistToIntersect );

		// for bone models only, use bone data to figure out actual 'BONE' limb
		sFrame* pFrameAssociatedWithBone = NULL;
		if ( pMeshThatHasBeenHitRef->dwBoneCount )
		{
			// go through all bone data and find a bone that uses V0 (enough)
			int iBoneMax = ( int ) pMeshThatHasBeenHitRef->dwBoneCount;
			for ( int iBone = 0; iBone < iBoneMax; iBone++ )
			{
				// go through all influenced bones
				for ( int iLoop = 0; iLoop < ( int ) pMeshThatHasBeenHitRef->pBones [ iBone ].dwNumInfluences; iLoop++ )
				{
					// get the correct vertex index from this bone influence item
					DWORD dwOffset = pMeshThatHasBeenHitRef->pBones [ iBone ].pVertices [ iLoop ];
					if ( dwOffset==dwVertex0IndexOfHitPoly )
					{
						// found the bone responsible for moving this V0!
						pFrameAssociatedWithBone = pMeshThatHasBeenHitRef->pFrameRef [ iBone ];
					
						// exit both loops immediately
						iBone=iBoneMax;
						break;
					}
				}
			}
		}

		// find index of this frame
		int iFrameRelatedToBone = -1;
		if ( pFrameAssociatedWithBone ) iFrameRelatedToBone=pFrameAssociatedWithBone->iID;

		// create a checklist to store all collision feedback
		MegaCollisionFeedback.iFrameCollision = iFrameCollision;
		MegaCollisionFeedback.iFrameRelatedToBone = iFrameRelatedToBone;
		MegaCollisionFeedback.dwVertex0IndexOfHitPoly = dwVertex0IndexOfHitPoly;
		MegaCollisionFeedback.dwVertex1IndexOfHitPoly = dwVertex1IndexOfHitPoly;
		MegaCollisionFeedback.dwVertex2IndexOfHitPoly = dwVertex2IndexOfHitPoly;
		MegaCollisionFeedback.vec0Hit = vec0Hit;
		MegaCollisionFeedback.vec1Hit = vec1Hit;
		MegaCollisionFeedback.vec2Hit = vec2Hit;
		MegaCollisionFeedback.vecHitPoint = vecHitPoint;
		MegaCollisionFeedback.vecNormal = vecSurfaceNormal;//GGVECTOR3(0,0,0);
		MegaCollisionFeedback.vecReflectedNormal = GGVECTOR3(0,0,0);
		MegaCollisionFeedback.dwArbitaryValue = dwArbValueDetected;
		CreateCollisionChecklist();

		// complete collision feedback
		return true;
	}
	else
	{
		// no collision detected
		return false;
	}
}

bool GetRayCollision(sObject* pObject, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float* fDistance)
{
	return GetRayCollisionEx(pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, fDistance, 0);
}

bool GetRayCollisionBoxFirst ( sObject* pObject, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float* fDistance )
{
	// Actual Object for data, this object for matWorld
	sObject* pActualObject = pObject;
	if ( pActualObject->pInstanceOfObject )
		pActualObject = pActualObject->pInstanceOfObject;

	// work out length of ray
	GGVECTOR3 vec3value = GGVECTOR3(fX,fY,fZ) - GGVECTOR3(fNewX,fNewY,fNewZ);
	float fDistanceBetweenPoints = GGVec3Length(&vec3value);

	// do a fast box check before commiting to intensive work
	GGMATRIX matInvWorld;
	float fDet;
	GGMatrixInverse(&matInvWorld,&fDet,&pObject->position.matWorld);
	GGVECTOR3 vecFrom = GGVECTOR3(fX,fY,fZ);
	GGVECTOR3 vecTo = GGVECTOR3(fNewX,fNewY,fNewZ);
	GGVec3TransformCoord(&vecFrom,&vecFrom,&matInvWorld);
	GGVec3TransformCoord(&vecTo,&vecTo,&matInvWorld);
	IntersectRay transformedray;
	transformedray.origin[0] = vecFrom.x;
	transformedray.origin[1] = vecFrom.y;
	transformedray.origin[2] = vecFrom.z;
	transformedray.direction[0] = vecTo.x-vecFrom.x;
	transformedray.direction[1] = vecTo.y-vecFrom.y;
	transformedray.direction[2] = vecTo.z-vecFrom.z;
	transformedray.direction[0] /= fDistanceBetweenPoints;
	transformedray.direction[1] /= fDistanceBetweenPoints;
	transformedray.direction[2] /= fDistanceBetweenPoints;

	// check if ray intersects object bound box (ray vs box) [using object space]
	IntersectBox box;
	box.min[0] = pObject->collision.vecMin.x;
	box.min[1] = pObject->collision.vecMin.y;
	box.min[2] = pObject->collision.vecMin.z;
	box.max[0] = pObject->collision.vecMax.x;
	box.max[1] = pObject->collision.vecMax.y;
	box.max[2] = pObject->collision.vecMax.z;
	int tnear, tfar;

	// if the ray is not inside the box, no need to go any further
	if ( intersectRayAABox2(transformedray, box, tnear, tfar)==false )
		return false;

	// return result
	return GetRayCollision ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, fDistance );
}

// an intersect command which scans ALL objects, and returns closest hit
// includeinvisibibleones, 

// Bound Management

bool UpdateColCenter( sObject* pObject )
{
	// update overall bounds if triggered
	if ( pObject->bUpdateOverallBounds==true )
	{
		// update only once for performance
		pObject->bUpdateOverallBounds=false;

		// calculate latest state of object (animation frame data, etc)
		CalculateObjectWorld ( pObject, NULL );

		// update object bounds
		CalculateAllBounds ( pObject, true );
	}

	// center of bound changes with orientation
	if ( pObject->collision.bColCenterUpdated==false )
	{
		// Update made for this cycle (no need to recalc)
		pObject->collision.bColCenterUpdated=true;

		// Update collision center vector of object
//		leefix - 200203 - use generated center as min/max can be adjusted for box collision..
//		pObject->collision.vecColCenter = pObject->collision.vecMin + ((pObject->collision.vecMax - pObject->collision.vecMin)/2.0f);
		pObject->collision.vecColCenter = pObject->collision.vecCentre;

		// setup the world matrix for the object
		CalcObjectWorld ( pObject );

		// Transform vector by object rotation (exclusing translation factor)
		GGVec3TransformCoord ( &pObject->collision.vecColCenter, &pObject->collision.vecColCenter, &pObject->position.matObjectNoTran );
	}

	// all okay
	return true;
}

float GetObjectCollisionCenterFinalX ( sObject* pObject )
{
	// U75 - 210407 - FPSCV115 - add scaling from object center pObject->position.vecScale
	if ( UpdateColCenter ( pObject ) )
		return pObject->collision.vecColCenter.x/pObject->position.vecScale.x;
	else
		return 0.0f;
}

float GetObjectCollisionCenterFinalY ( sObject* pObject )
{
	// U75 - 210407 - FPSCV115 - add scaling from object center pObject->position.vecScale
	if ( UpdateColCenter ( pObject ) )
		return pObject->collision.vecColCenter.y/pObject->position.vecScale.y;
	else
		return 0.0f;
}

float GetObjectCollisionCenterFinalZ ( sObject* pObject )
{
	// U75 - 210407 - FPSCV115 - add scaling from object center pObject->position.vecScale
	if ( UpdateColCenter ( pObject ) )
		return pObject->collision.vecColCenter.z/pObject->position.vecScale.z;
	else
		return 0.0f;
}

void ColPosition ( sObject* pObject, float fX, float fY, float fZ )
{
	// set the new position
	pObject->collision.bHasBeenMovedForResponse=true;

	float fOffsetX = GetObjectCollisionCenterFinalX ( pObject );
	float fOffsetY = GetObjectCollisionCenterFinalY ( pObject );
	float fOffsetZ = GetObjectCollisionCenterFinalZ ( pObject );
	pObject->position.vecPosition.x = fX - fOffsetX;
	pObject->position.vecPosition.y = fY - fOffsetY;
	pObject->position.vecPosition.z = fZ - fOffsetZ;
}

float GetXColPosition ( sObject* pObject )
{
	float fPos = pObject->position.vecPosition.x;
	float fOffsetX = GetObjectCollisionCenterFinalX(pObject);
	return fPos + fOffsetX;
}

float GetYColPosition ( sObject* pObject )
{
	float fPos = pObject->position.vecPosition.y;
	float fOffsetY = GetObjectCollisionCenterFinalY(pObject);
	return fPos + fOffsetY;
}

float GetZColPosition ( sObject* pObject )
{
	float fPos = pObject->position.vecPosition.z;
	float fOffsetZ = GetObjectCollisionCenterFinalZ(pObject);
	return fPos + fOffsetZ;
}

bool CheckBoxIntersect ( 	GGVECTOR3 vecPos1, GGVECTOR3 vecMin1, GGVECTOR3 vecMax1, 
							GGVECTOR3 vecPos2, GGVECTOR3 vecMin2, GGVECTOR3 vecMax2 )
{
	// check box intersection

	// update vectors to match the position
	vecMin1 += vecPos1;
	vecMax1 += vecPos1;

	vecMin2 += vecPos2;
	vecMax2 += vecPos2;

	// see if they intersect
	if ( vecMax1.x >= vecMin2.x && vecMin1.x <= vecMax2.x &&
		 vecMax1.y >= vecMin2.y && vecMin1.y <= vecMax2.y &&
		 vecMax1.z >= vecMin2.z && vecMin1.z <= vecMax2.z
	   )
			return true;
	
	return false;
}

float CheckSphereIntersect ( GGVECTOR3 vecPos1, float fRadius1, GGVECTOR3 vecPos2, float fRadius2 )
{
	float fXDiff, fYDiff, fZDiff;
	fXDiff = ( float ) fabs ( vecPos2.x - vecPos1.x );
	fYDiff = ( float ) fabs ( vecPos2.y - vecPos1.y );
	fZDiff = ( float ) fabs ( vecPos2.z - vecPos1.z );
	double dDist = sqrt ( ((double)fXDiff*(double)fXDiff) + ((double)fYDiff*(double)fYDiff) + ((double)fZDiff*(double)fZDiff) );
	float fOverlapDistance = (fRadius1+fRadius2) - (float)dDist;
	if ( fOverlapDistance > 0.0f )
	{
		// spheres touched, determine by how much
		return fOverlapDistance;
	}
	else
	{
		// zero overlap - no sphere collision
		return 0.0f;
	}
}

bool PointWithinTriangle ( GGVECTOR3* pNormal, GGVECTOR3* pVector, GGVECTOR3* pVec0, GGVECTOR3* pVec1, GGVECTOR3* pVec2, float fDistanceChk )
{
	// Check if my sphere-line intersects triangle
	float fU, fV, fDistance;
	if ( GGIntersectTri ( pVec0, pVec1, pVec2, pVector, pNormal, &fU, &fV, &fDistance) == TRUE )
		if ( fDistance <= fDistanceChk )
			return true;
		else
			return false;
	else
		return false;
}

bool CheckSphereToMeshIntersect ( sMesh* pMesh, GGVECTOR3* pNormal, GGVECTOR3* pVec, GGVECTOR3* pRad )
{
	// work out which radius to use (the biggest)
	float fRadiusMatchedToPlain = pRad->x;
	if ( fRadiusMatchedToPlain < pRad->y ) fRadiusMatchedToPlain = pRad->y;
	if ( fRadiusMatchedToPlain < pRad->z ) fRadiusMatchedToPlain = pRad->z;

	// Hit state
	bool bHit = false;
	BYTE* Ptr = pMesh->pVertexData;
	if ( Ptr )
	{
		unsigned short*	IndexPtr = pMesh->pIndices;
		if ( IndexPtr )
		{
			// vertex data vars
			float* pV = (float*)Ptr;
			DWORD dwSizeInFloats = (DWORD)(pMesh->dwFVFSize/4);

			// load polygon information into structures
			for ( int i = 0; i < pMesh->iDrawPrimitives; i++ )
			{
				// triangle indices
				unsigned short v0 = *(IndexPtr+0);
				unsigned short v1 = *(IndexPtr+1);
				unsigned short v2 = *(IndexPtr+2);
				IndexPtr+=3;

				// triangle vertice ptrs
				GGVECTOR3* pVec0 = (GGVECTOR3*)(pV+((v0*dwSizeInFloats)));
				GGVECTOR3* pVec1 = (GGVECTOR3*)(pV+((v1*dwSizeInFloats)));
				GGVECTOR3* pVec2 = (GGVECTOR3*)(pV+((v2*dwSizeInFloats)));

				// calculate plane from vertices
				GGPLANE Plane;
				GGPlaneFromPoints ( &Plane, pVec0, pVec1, pVec2 );

				// calculate dot product of plane and vector of collision sphere
				float fDotProduct = GGPlaneDotCoord ( &Plane, pVec );

				// dot product is distance from plane and vector
				if ( fabs(fDotProduct) < fRadiusMatchedToPlain )
				{
					// If vector in triangle of poly, it is touching
					if ( PointWithinTriangle ( pNormal, pVec, pVec0, pVec1, pVec2, fRadiusMatchedToPlain ) )
					{
						bHit=true;
						break;
					}
				}
			}
		}
	}
	return bHit;
}

bool CheckSphereToObjectIntersect ( sObject* pObject, GGVECTOR3* pVecVec, GGVECTOR3* pDistance )
{
	bool bHit = false;
	for ( int iFrame=0; iFrame<pObject->iFrameCount; iFrame++ )
	{
		// get frame ptr
		sFrame* pFrame = pObject->ppFrameList[iFrame];
		if ( pFrame->pMesh )
		{
			// adjust B-vector to A-object-space
			GGMATRIX matInv;
			GGMATRIX matWorld = pFrame->matCombined * pObject->position.matWorld;
			GGMatrixInverse(&matInv, NULL, &matWorld );
			GGVECTOR3 ObjVec = *pVecVec;
			GGVec3TransformCoord( &ObjVec, &ObjVec, &matInv );
			GGVECTOR3 ObjRad = *pDistance;
			GGVec3TransformCoord( &ObjRad, &ObjRad, &matInv );

			// produce normal from offset to object center
			GGVECTOR3 Normal = pObject->collision.vecCentre - ObjVec;
			GGVec3Normalize ( &Normal, &Normal );

			// check sphere against frame mesh
			if ( CheckSphereToMeshIntersect ( pFrame->pMesh, &Normal, &ObjVec, &ObjRad ) )
			{
				bHit=true;
				break;
			}
		}
	}
	return bHit;
}

bool CheckTwoObjectsForBoxOverlap( int iObjectA, sObject* pObjectA, int iObjectB, sObject* pObjectB )
{
	if(DBV1_ReturnModelBoxSystemOverlapResult ( pObjectA, pObjectB ))
		return true;
	else
		return false;
}
int DBPRO_ReturnOverlapResult ( int iObjectA, sObject* pObjectA, int iObjectB, sObject* pObjectB )
{
	// In case of collision, this is who we hit
	int iOverlapResultIndex=iObjectB;

	// Shape of Collision To Use
	DWORD dwCollisionShape;
	if(pObjectA->collision.eCollisionType==COLLISION_POLYGON
	|| pObjectB->collision.eCollisionType==COLLISION_POLYGON)
	{
		// PolyObj is always A, Sphere is always B (leefix - 210703 - swap if A is nonPolyObj)
		if(pObjectA->collision.eCollisionType!=COLLISION_POLYGON
		&& pObjectB->collision.eCollisionType==COLLISION_POLYGON)
		{
			// Swap ptrs and indexes
			sObject* pObjectS = pObjectA;
			pObjectA = pObjectB;
			pObjectB = pObjectS;
			int iObjectS = iObjectA;
			iObjectA = iObjectB;
			iObjectB = iObjectS;
		}

		// Use polycheck if either is poly collision
		dwCollisionShape=COLLISION_POLYGON;
	}
	else
	{
		if(pObjectA->collision.eCollisionType==COLLISION_BOX
		|| pObjectB->collision.eCollisionType==COLLISION_BOX)
		{
			// BOX V BOX if one of the objects uses a BOX Shape
			// The idea is that spheres colliding here use fixed box (sliding along - 'fixed' not rotating boxes)
			dwCollisionShape=COLLISION_BOX;
		}
		else
		{
			// In all other cases, use object setting
			dwCollisionShape=pObjectA->collision.eCollisionType;
		}
	}

	switch ( dwCollisionShape )
	{
		case COLLISION_NONE:
		{
			return 0;
		}
		break;

		case COLLISION_SPHERE:
		{
			// Update collision centers if required
			UpdateColCenter(pObjectA);
			UpdateColCenter(pObjectB);
			GGVECTOR3 vecMidA = pObjectA->collision.vecColCenter;
			GGVECTOR3 vecMidB = pObjectB->collision.vecColCenter;
			vecMidA = pObjectA->position.vecPosition + vecMidA;
			vecMidB = pObjectB->position.vecPosition + vecMidB;

			// finalise radius calc and do intersect check
			float fRadius1 = pObjectA->collision.fRadius;
			float fRadius2 = pObjectB->collision.fRadius;
			fRadius1 *= (pObjectA->position.vecScale.x+pObjectA->position.vecScale.y+pObjectA->position.vecScale.z)/3.0f;
			fRadius2 *= (pObjectB->position.vecScale.x+pObjectB->position.vecScale.y+pObjectB->position.vecScale.z)/3.0f;
			float fOverlapDistance = CheckSphereIntersect ( vecMidA, fRadius1, vecMidB, fRadius2 );
			if ( fOverlapDistance > 0.0f )
			{
				// Spheres are touching, now calculate vector to remove A from B
				GGVECTOR3 vecDirAwayFromA = vecMidB - vecMidA;
				GGVec3Normalize ( &vecDirAwayFromA, &vecDirAwayFromA );
				vecDirAwayFromA *= fOverlapDistance;

				// copy adjustment to slide vector
				bResponseDataAvailableFromDBV1 = true;
				gvLatestObjectCollisionResult = vecDirAwayFromA;

				// positive collision result
				return iOverlapResultIndex;
			}
			else
			{
				// no collision of spheres
				return 0;
			}
		}
		break;

		case COLLISION_BOX:
		{
			// box collision
			bResponseDataAvailableFromDBV1=true;
			if(CheckTwoObjectsForBoxOverlap(iObjectA, pObjectA, iObjectB, pObjectB))
			{
				return iOverlapResultIndex;
			}
			else
				return 0;
		}
		break;

		case COLLISION_POLYGON:
		{
			// box collision first
			if(CheckTwoObjectsForBoxOverlap(iObjectA, pObjectA, iObjectB, pObjectB))
			{
				// PolyObj is always A (and B is always sphere)
				// work out radius vector
				//leefix - 010306 - u60 - sphere radius MUST be calclated with B, not A which is PolyObj!
				//float fDistance = pObjectA->collision.fRadius;
				float fDistance = pObjectB->collision.fRadius;
				GGVECTOR3 vecRadius = GGVECTOR3 ( fDistance, fDistance, fDistance );

				// LEEFIX - 210703 - NODETREE will use better collision, so for now
				// we will use a very accurate and slower star line collision system
				float fReturnDistance=0.0f;
				for ( int s=0; s<5; s++)
				{
					GGVECTOR3 VecFrom = pObjectB->position.vecPosition;
					GGVECTOR3 VecTo = pObjectB->position.vecPosition;
					
					float fDistFromStart = fDistance;
					switch ( s )
					{
						case 0 : VecFrom.y-=fDistance; VecTo.y+=fDistance; fDistFromStart*=2; break;
						case 1 : VecFrom.z-=fDistance; VecTo.z+=fDistance; fDistFromStart*=2; break;
						case 2 : VecFrom.x-=fDistance; VecTo.x+=fDistance; fDistFromStart*=2; break;
						
						case 3 :
						{
							// 0.6f

							float fMultiplier = 0.6f;

							VecFrom.x-=fDistance*fMultiplier;
								 VecFrom.z-=fDistance*fMultiplier;
								 VecFrom.y-=fDistance*fMultiplier;
								 VecTo.x+=fDistance*fMultiplier;
								 VecTo.z+=fDistance*fMultiplier;
								 VecTo.y+=fDistance*fMultiplier;
								 fDistFromStart=(fDistance*fMultiplier)*2; 
						}
						break;
						
						case 4 :
						{
							float fMultiplier = 0.6f;

							VecFrom.x-=fDistance*fMultiplier;
								 VecFrom.z+=fDistance*fMultiplier;
								 VecFrom.y-=fDistance*fMultiplier;
								 VecTo.x+=fDistance*fMultiplier;
								 VecTo.z-=fDistance*fMultiplier;
								 VecTo.y+=fDistance*fMultiplier;
								 fDistFromStart=(fDistance*fMultiplier)*2; 
						}
						 break;
					}
					if(GetRayCollision ( pObjectA, VecFrom.x, VecFrom.y, VecFrom.z, VecTo.x, VecTo.y, VecTo.z, &fReturnDistance ))
					{
						if ( fReturnDistance <= fDistFromStart )
							return iOverlapResultIndex;
					}	
				}
			}
			else
				return 0;
		}
		break;
	}

	// No collision
	return 0;
}

int GetOverlapCollisionResultCore ( int iObjectA, sObject* pObjectA, int iObjectB, sObject* pObjectB )
{
	// result value
	int overlap=0;

	// check object A is valid (B can be zero)
	if ( !pObjectA )
		return 0;

	// lee - 180406 - u6rc10 - added exclusion code
	if(pObjectA->collision.bActive==false || pObjectA->bExcluded==true)
		return 0;
	
	// now perform collision detection on them
	if(pObjectB)
	{
		// only if object B has active collision
		if(pObjectB->collision.bActive && pObjectB->bExcluded==false)
		{
			// use DBV1BOX collision or DBPRO collision
			bResponseDataAvailableFromDBV1 = pObjectA->collision.bUseBoxCollision;
			if( bResponseDataAvailableFromDBV1==true )
				overlap=DBV1_ReturnModelBoxSystemOverlapResult( pObjectA, pObjectB );
			else
				overlap=DBPRO_ReturnOverlapResult( iObjectA, pObjectA, iObjectB, pObjectB );

			// ensure return value is either zero or one (false/true)
			if(overlap>0) overlap=1;
		}
	}
	else
	{
		// Handle cumilative gvLatestObjectCollisionResult
		GGVECTOR3 gvCumilativeObjectCollisionResult;
		gvCumilativeObjectCollisionResult.x=0;
		gvCumilativeObjectCollisionResult.y=0;
		gvCumilativeObjectCollisionResult.z=0;

		// Check collision against ALL other objects
		for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
		{
			// get index from shortlist
			int iCurrentObjectID = g_ObjectListRef [ iShortList ];

			// get a pointer to the object from the sorted draw list
			sObject* pCurrent = g_ObjectList [ iCurrentObjectID ];
			if ( pCurrent == NULL ) continue;

			// lee - 180406 - u6rc10 - if excluded, skip
			if ( pCurrent->bExcluded==true )
				continue;

			// Only objects that have collision on
			if(pObjectA!=pCurrent && pCurrent->collision.bActive)
			{
				// Check against current box
				int thisoverlap = 0;
				if( pObjectA->collision.bUseBoxCollision==true )
				{
					// If boundboxer doing check, treat everything else as bouncboxes
					bResponseDataAvailableFromDBV1=true;
					thisoverlap = DBV1_ReturnModelBoxSystemOverlapResult( pObjectA, pCurrent );
					if ( thisoverlap>0 )
						overlap=iCurrentObjectID;
				}
				else
				{
					// Multiple collision shape checks
					bResponseDataAvailableFromDBV1=false;
					thisoverlap = DBPRO_ReturnOverlapResult( iObjectA, pObjectA, iCurrentObjectID, pCurrent );
					if ( thisoverlap>0 ) overlap=thisoverlap;
				}

				// Ensure greatest result is retained
				if ( thisoverlap != 0 )
				{
					if ( fabs(gvLatestObjectCollisionResult.x) > fabs(gvCumilativeObjectCollisionResult.x) ) gvCumilativeObjectCollisionResult.x = gvLatestObjectCollisionResult.x;
					if ( fabs(gvLatestObjectCollisionResult.y) > fabs(gvCumilativeObjectCollisionResult.y) ) gvCumilativeObjectCollisionResult.y = gvLatestObjectCollisionResult.y;
					if ( fabs(gvLatestObjectCollisionResult.z) > fabs(gvCumilativeObjectCollisionResult.z) ) gvCumilativeObjectCollisionResult.z = gvLatestObjectCollisionResult.z;
				}
			}
		}

		// Transfer retained collision result over
		gvLatestObjectCollisionResult = gvCumilativeObjectCollisionResult;
	}
	return overlap;
}

int GetOverlapCollisionResult ( int iObjectA, int iObjectB )
{
	// Global collision can deactivate all checks
	if ( g_bGlobalCollisionActive==false )
		return 0;

	// variable declarations
	sObject*	pObjectA = NULL;		// object data a
	sObject*	pObjectB = NULL;		// object data b

	// get object data
	pObjectA = g_ObjectList [ iObjectA ];
	pObjectB = g_ObjectList [ iObjectB ];

	// Call with additional ptr info
	return GetOverlapCollisionResultCore ( iObjectA, pObjectA, iObjectB, pObjectB );
}

void AdjustBoundBoxToCollisionSize ( sObject* pObject, float fHalfSize )
{
	pObject->collision.vecMin.x = fHalfSize*-1.0f;
	pObject->collision.vecMin.y = fHalfSize*-1.0f;
	pObject->collision.vecMin.z = fHalfSize*-1.0f;
	pObject->collision.vecMax.x = fHalfSize;
	pObject->collision.vecMax.y = fHalfSize;
	pObject->collision.vecMax.z = fHalfSize;
	pObject->collision.vecCentre.x = pObject->collision.vecMin.x+((pObject->collision.vecMax.x-pObject->collision.vecMin.x)*0.5f);
	pObject->collision.vecCentre.y = pObject->collision.vecMin.y+((pObject->collision.vecMax.y-pObject->collision.vecMin.y)*0.5f);
	pObject->collision.vecCentre.z = pObject->collision.vecMin.z+((pObject->collision.vecMax.z-pObject->collision.vecMin.z)*0.5f);
}

// Collision Commands

void SetColOn ( sObject* pObject )
{
	pObject->collision.bActive = true;
}

void SetColOff ( sObject* pObject )
{
	pObject->collision.bActive = false;
	pObject->collision.iResponseMode = 0;
}

void SetColBox ( sObject* pObject, float iX1, float iY1, float iZ1, float iX2, float iY2, float iZ2, int iRotatedBoxFlag )
{
	// Old style bound box check
	pObject->collision.bUseBoxCollision = true;
	DBV1_MakeObjectCollisionBox( pObject, iX1, iY1, iZ1, iX2, iY2, iZ2, iRotatedBoxFlag );
}

void FreeColBox ( sObject* pObject )
{
	pObject->collision.bUseBoxCollision = false;
}

void SetColToSpheres ( sObject* pObject )
{
	pObject->collision.eCollisionType = COLLISION_SPHERE;
	pObject->collision.bFixedBoxCheck = true; // this idea is that foixed boxes slide even when rotating
}

void SetColToBoxes ( sObject* pObject )
{
	pObject->collision.eCollisionType = COLLISION_BOX;
	pObject->collision.bFixedBoxCheck = false;
}

void SetColToPolygons ( sObject* pObject )
{
	pObject->collision.eCollisionType = COLLISION_POLYGON;
	pObject->collision.bFixedBoxCheck = false;
}

void SetSphereRadius ( sObject* pObject, float fRadius )
{
	// leeadd - 260806 - u63 - if radius is zero, we are asking for the object not to be visually culled
	if ( fRadius<=0.0f )
	{
		// lee - 051214 - special mode to restore radius of cloned objects that lost theirs
		if ( fRadius==-1 )
		{
			// Use preserved largest radius to re-populate object
			fRadius = pObject->collision.fLargestRadius;
			pObject->collision.fRadius = fRadius;
			float fBiggestScale = max (	pObject->position.vecScale.x, max (
										pObject->position.vecScale.y,
										pObject->position.vecScale.z) );
			pObject->collision.fScaledRadius = fRadius * fBiggestScale;
			pObject->collision.fScaledLargestRadius = fRadius * fBiggestScale;
		}
		else
		{
			// Special setting which allows the object to be drawn, no matter if in view
			pObject->collision.fRadius = fRadius;
			pObject->collision.fScaledLargestRadius = fRadius;
		}
	}
	else
	{
		// Set Radius of sphere
		pObject->collision.fRadius = fRadius;
		AdjustBoundBoxToCollisionSize ( pObject, fRadius );
	}
}

void GlobalColOn ( void )
{
	g_bGlobalCollisionActive = true;
}

void GlobalColOff ( void )
{
	g_bGlobalCollisionActive = false;
}

int CheckCol ( int iObjectA, int iObjectB )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectA ) )
		return 0;

	if ( iObjectB==0 )
	{
		// Compare against all objects
	}
	else
	{
		// check the object exists
		if ( !ConfirmObjectInstance ( iObjectB ) )
			return 0;
	}

	// Direct result from check
	return GetOverlapCollisionResult ( iObjectA, iObjectB );
}

int CheckHit ( int iObjectA, int iObjectB )
{
	// check the object exists
	if ( !ConfirmObject ( iObjectA ) )
		return 0;

	sObject* pObjectA = g_ObjectList [ iObjectA ];
	if ( iObjectB==0 )
	{
		// Compare against all objects
	}
	else
	{
		// check the object exists
		if ( !ConfirmObject ( iObjectB ) )
			return 0;
	}

	// Only return 'hit-once' event
	int iHitEvent=0;
	
	// Get overlap of two objects
	int overlap = GetOverlapCollisionResult ( iObjectA, iObjectB );

	// leefix - 060306 - u60 - can now handle multiple object HIT collision per same cycle
//	if(overlap>0 && pObjectA->collision.iHitOverlapStore==0)
//		iHitEvent = overlap;
//	else
//		iHitEvent = 0;
	if ( pObjectA->collision.iHitOverlapStore==0 )
	{
		// no history of collision
		if ( overlap > 0 )
		{
			// Store for next time
			if ( iObjectB > 0 )
				pObjectA->collision.iHitOverlapStore = iObjectB;
			else
				pObjectA->collision.iHitOverlapStore = overlap;

			// regster hit
			iHitEvent = overlap;
		}
	}
	else
	{
		// this ensures only the object being HIT CONSTANTLY, can release the hit-flag to zero
		if ( pObjectA->collision.iHitOverlapStore==iObjectB || iObjectB==0 )
		{
			// had hit X, wait till we are not hitting it no more
			if ( overlap == 0 )
			{
				pObjectA->collision.iHitOverlapStore=0;
			}
		}
	}

	// Direct result from check
	return iHitEvent;
}

int CheckLimbCol ( int iObjectA, int iLimbA, int iObjectB, int iLimbB )
{
	// limb return value
	int iReturnValue=-1;

	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectA ) )
		return 0;

	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectB ) )
		return 0;

	// actual object or instance
	sObject* pObjectA = g_ObjectList [ iObjectA ];
	sObject* pActualObjectA = pObjectA;
	if ( pObjectA->pInstanceOfObject )
		pActualObjectA = pObjectA->pInstanceOfObject;

	sObject* pObjectB = g_ObjectList [ iObjectB ];
	sObject* pActualObjectB = pObjectB;
	if ( pObjectB->pInstanceOfObject )
		pActualObjectB = pObjectB->pInstanceOfObject;

	// Check all limbs against all limbs
	for ( int iFrameA=0; iFrameA<pActualObjectA->iFrameCount; iFrameA++ )
	{
		// get frame ptr
		sFrame* pFrameA = pActualObjectA->ppFrameList[iFrameA];
		UpdateRealtimeFrameVectors ( pActualObjectA, pFrameA );
		sMesh* pMeshA = pFrameA->pMesh;
		if ( pMeshA )
		{
			// if limbA specified, only this frame
			if ( iLimbA==iFrameA || iLimbA==-1 )
			{
				// for each A limb, check all B limbs
				for ( int iFrameB=0; iFrameB<pActualObjectB->iFrameCount; iFrameB++ )
				{
					// get frame ptr
					sFrame* pFrameB = pActualObjectB->ppFrameList[iFrameB];
					UpdateRealtimeFrameVectors ( pActualObjectB, pFrameB );
					sMesh* pMeshB = pFrameB->pMesh;
					if ( pMeshB )
					{
						// prepare world location of A
						CalculateAbsoluteWorldMatrix ( pObjectA, pFrameA, pMeshA );
						GGVECTOR3 box1p;// = pFrameA->vecPosition;
						box1p.x = pFrameA->matAbsoluteWorld._41;
						box1p.y = pFrameA->matAbsoluteWorld._42;
						box1p.z = pFrameA->matAbsoluteWorld._43;
						GGMATRIX box1r = pFrameA->matAbsoluteWorld;
						box1r._41=0.0f; box1r._42=0.0f; box1r._43=0.0f; 

						// prepare world location of B
						CalculateAbsoluteWorldMatrix ( pObjectB, pFrameB, pMeshB );
						GGVECTOR3 box2p;
						box2p.x = pFrameB->matAbsoluteWorld._41;
						box2p.y = pFrameB->matAbsoluteWorld._42;
						box2p.z = pFrameB->matAbsoluteWorld._43;
						GGMATRIX box2r = pFrameB->matAbsoluteWorld;
						box2r._41=0.0f; box2r._42=0.0f; box2r._43=0.0f; 

						// compare A and B limb for collision
						int iColResult = CheckWorldBoxCollision ( box1p, box1r, &pMeshA->Collision, box2p, box2r, &pMeshB->Collision );
						if(iColResult>0)
						{
							// act based on limb search mode
							if ( iLimbB==-1 )
							{
								// return with limb B number
								iReturnValue = iFrameB;
								goto _done;
							}
							else
							{
								// return with true (1)
								if ( iLimbB==iFrameB )
								{
									iReturnValue = 1;
									goto _done;
								}
							}
						}
					}
				}
			}
		}
	}

	// Complete
	_done:
	return iReturnValue;
}

int	CheckLimbHit ( int iObjectA, int iLimbA, int iObjectB, int iLimbB )
{
	// check the object exists
	if ( !ConfirmObject ( iObjectA ) )
		return -1;

	// Only return 'hit-once' event
	int iHitEvent=-1;
	
	// collision detection
	int overlap = CheckLimbCol ( iObjectA, iLimbA, iObjectB, iLimbB );

	sObject* pObjectA = g_ObjectList [ iObjectA ];
	if(overlap!=-1)
	{
		if ( pObjectA->collision.iHitOverlapStore==-1 )
			iHitEvent = overlap;
		else
			if ( pObjectA->collision.iHitOverlapStore!=overlap )
				iHitEvent = overlap;
	}
	else
		iHitEvent = -1;

	// Store for next time
	pObjectA->collision.iHitOverlapStore = overlap;

	// Direct result from check
	return iHitEvent;
}

float CheckIntersectObject ( sObject* pObject, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreAllButLastFrame )
{
	// ensure all meshes of object are tri-lists
	if ( pObject->ppFrameList )
	{
		bool bIfObjChanged = false;
		for ( int iFrame=0; iFrame<pObject->iFrameCount; iFrame++ )
		{
			// get frame ptr
			sFrame* pFrame = pObject->ppFrameList[iFrame];
			if ( pFrame )
			{
				sMesh* pMesh = pFrame->pMesh;
				if ( pMesh )
				{
					// First ensure mesh is a tri-list
					if ( ConvertLocalMeshToTriList ( pMesh ) )
						bIfObjChanged = true;
				}
			}
		}

		// call manager to replace the mesh for continued use of object ( u55 - only if changed )
		if ( bIfObjChanged )
			m_ObjectManager.RenewReplacedMeshes ( pObject );
	}

	// actual intersect test
	float fDistance=0.0f;
	if ( RayCollisionDoBoxCheckFirst == false )
		GetRayCollisionEx ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, &fDistance, iIgnoreAllButLastFrame );
	else
		GetRayCollisionBoxFirst ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, &fDistance );
	
	//PE: 11-06-19 issue: https://github.com/TheGameCreators/GameGuruRepo/issues/502
	//PE: Some animations have bone collision problems so test mesh if this is the case.
	//PE: This is a editor only function so will not have any speed impact in game.

	//PE: We cant do this , as some test games and scripts use this to check animations with collision problems.
	if(1) // tgamesetismapeditormode == 1)
	{
		if (fDistance == 0.0f && !pObject->bIgnoreDefAnim && RayCollisionDoBoxCheckFirst == false)
		{
			sObject* pActualObject = pObject;
			if (pActualObject->pInstanceOfObject) pActualObject = pActualObject->pInstanceOfObject;
			if (pActualObject->pAnimationSet)
			{
				//PE: iUseNewTransformedVertsMode fails for some animated object
				//PE: So in this case also test using mesh and ignore bones.
				pObject->bIgnoreDefAnim = true;
				GetRayCollisionEx(pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, &fDistance, iIgnoreAllButLastFrame);
				pObject->bIgnoreDefAnim = false;
			}
		}
	}
	// return result
	return fDistance;
}

float GetColRadius ( sObject* pObject )
{
	// radius multiplied by average of scale
	return pObject->collision.fScaledRadius;
}

float GetColCenterX ( sObject* pObject )
{
	// get normal pos
	return GetObjectCollisionCenterFinalX(pObject);
}

float GetColCenterY ( sObject* pObject )
{
	// get normal pos
	return GetObjectCollisionCenterFinalY(pObject);
}

float GetColCenterZ ( sObject* pObject )
{
	// get normal pos
	return GetObjectCollisionCenterFinalZ(pObject);
}

float GetColX ( void )
{
	return DBV1_ObjectCollisionXF();
}

float GetColY ( void )
{
	return DBV1_ObjectCollisionYF();
}

float GetColZ ( void )
{
	return DBV1_ObjectCollisionZF();
}

int GetCollidedAgainstFloor ( void )
{
	if ( g_DBPROCollisionResult.bCollidedAgainstFloor==true )
		return 1;
	else
		return 0;
}

//
// Static Collision Functions
//

void SetStaticColBox ( float fX1, float fY1, float fZ1, float fX2, float fY2, float fZ2 )
{
	DBV1_MakeStaticCollisionBox( fX1, fY1, fZ1, fX2, fY2, fZ2 );
}

void FreeStaticColBoxes ( void )
{
	DBV1_DeleteAllStaticCollisionBoxes();
}

int GetStaticLineOfSightEx ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz, float fWidth, float fAccuracy )
{
	return DBV1_StaticLineOfSightHit( fSx, fSy, fSz, fDx, fDy, fDz, fWidth, fAccuracy );
}

float GetStaticLineOfSightExX ( void )
{
	return DBV1_LineOfSightXF();
}

float GetStaticLineOfSightExY ( void )
{
	return DBV1_LineOfSightYF();
}

float GetStaticLineOfSightExZ ( void )
{
	return DBV1_LineOfSightZF();
}

int GetStaticHitEx (	float fOldX1, float fOldY1, float fOldZ1, float fOldX2, float fOldY2, float fOldZ2,
					float fNX1,   float fNY1,   float fNZ1,   float fNX2,   float fNY2,   float fNZ2    )
{
	g_DBPROCollisionResult.bUsed = false;
	return DBV1_StaticCollisionBoxHit(fOldX1,fOldY1,fOldZ1,fOldX2,fOldY2,fOldZ2, fNX1,fNY1,fNZ1,fNX2,fNY2,fNZ2);
}

//
// Static Collision Functions (Plus Extras)
//

float GetStaticColX ( void )
{
	if ( g_DBPROCollisionResult.bUsed )
		return g_DBPROCollisionResult.vecDifference.x;
	else
		return DBV1_StaticCollisionXF();
}

float GetStaticColY ( void )
{
	if ( g_DBPROCollisionResult.bUsed )
		return g_DBPROCollisionResult.vecDifference.y;
	else
		return DBV1_StaticCollisionYF();
}

float GetStaticColZ ( void )
{
	if ( g_DBPROCollisionResult.bUsed )
		return g_DBPROCollisionResult.vecDifference.z;
	else
		return DBV1_StaticCollisionZF();
}

float GetStaticColPosX ( void )
{
	return g_DBPROCollisionResult.vecPos.x;
}

float GetStaticColPosY ( void )
{
	return g_DBPROCollisionResult.vecPos.y;
}

float GetStaticColPosZ ( void )
{
	return g_DBPROCollisionResult.vecPos.z;
}

float GetStaticColNormalX ( void )
{
	return g_DBPROCollisionResult.vecNormal.x;
}

float GetStaticColNormalY ( void )
{
	return g_DBPROCollisionResult.vecNormal.y;
}

float GetStaticColNormalZ ( void )
{
	return g_DBPROCollisionResult.vecNormal.z;
}

float GetStaticColTextureU ( void )
{
	return g_DBPROCollisionResult.fTextureU;
}

float GetStaticColTextureV ( void )
{
	return g_DBPROCollisionResult.fTextureV;
}

int GetStaticColPolysChecked ( void )
{
	return g_DBPROCollisionResult.iPolysChecked;
}

DWORD GetStaticColArbitaryValue ( void )
{
	return g_DBPROCollisionResult.dwArbitaryValue;
}

//
// Cool Auto-Collision Commands
//

void DoAutomaticStart ( void )
{
	// Global collision can deactivate all checks
	if ( g_bGlobalCollisionActive==false )
		return;

	// Only auto-collision objects (use shortlist to speed up process)
	for ( int iShortList = -1; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iCurrentObjectID;
		if ( iShortList==-1 )
			iCurrentObjectID = -1;
		else
			iCurrentObjectID = g_ObjectListRef [ iShortList ];

		// CameraObject or Object pointer
		sObject* pCurrent;
		if ( iCurrentObjectID == -1 )
			pCurrent = &CameraObject;
		else
		{
			pCurrent = g_ObjectList [ iCurrentObjectID ];
			if ( pCurrent == NULL ) continue;
		}

		if(pCurrent->collision.bActive && pCurrent->collision.iResponseMode>0)
		{
			// start as not moved
			pCurrent->collision.bHasBeenMovedForResponse=false;

			// if camera, some extra work
			if( iCurrentObjectID == -1 )
			{
				// Set camera data prior to auto-collision
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( m_iCameraUsingForCollision );
				CameraObject.position.vecPosition = m_Camera_Ptr->vecPosition;
				CameraObject.position.matWorld._41 = m_Camera_Ptr->vecPosition.x;
				CameraObject.position.matWorld._42 = m_Camera_Ptr->vecPosition.y;
				CameraObject.position.matWorld._43 = m_Camera_Ptr->vecPosition.z;
				fLastCamX = CameraObject.position.vecPosition.x;
				fLastCamY = CameraObject.position.vecPosition.y;
				fLastCamZ = CameraObject.position.vecPosition.z;
				pCurrent->position.vecRotate = GGVECTOR3 (0,0,0);
//				CameraObject.position.vecLastRotate = GGVECTOR3 ( 0, m_Camera_Ptr->fYRotate, 0 );
			}
		}

		// record position before cycle
		pCurrent->position.vecLastPosition = pCurrent->position.vecPosition;
		pCurrent->position.vecLastRotate = pCurrent->position.vecRotate;
		pCurrent->position.matLastFreeFlightRotate = pCurrent->position.matFreeFlightRotate;
	}

	// automatic collision started, so it will finish
	g_bAutoColStarted = true;
}

void DoAutomaticEnd ( void )
{
	// Global collision can deactivate all checks
	if ( g_bGlobalCollisionActive==false )
		return;

	// Autocol must have started before it can be finished off
	if ( g_bAutoColStarted==false )
		return;

	// Now officially finished
	g_bAutoColStarted = false;

	// Based on difference, apply responses where required
	for ( int iShortList = -1; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iCurrentObjectID;
		if ( iShortList==-1 )
			iCurrentObjectID = -1;
		else
			iCurrentObjectID = g_ObjectListRef [ iShortList ];

		// CameraObject or Object pointer
		sObject* pCurrentA;
		if ( iCurrentObjectID == -1 )
			pCurrentA = &CameraObject;
		else
		{
			pCurrentA = g_ObjectList [ iCurrentObjectID ];
			if ( pCurrentA == NULL ) continue;
		}

		if(pCurrentA->collision.bActive && pCurrentA->collision.iResponseMode>0)
		{
			// If camera moved since start, signal the move
			if(iCurrentObjectID==-1)
			{
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( m_iCameraUsingForCollision );
				pCurrentA->position.vecPosition = m_Camera_Ptr->vecPosition;
				// leefix - 200704 - u56 - camera collision needs vector in matWorld (for box collision slider)
				pCurrentA->position.matWorld._41 = m_Camera_Ptr->vecPosition.x;
				pCurrentA->position.matWorld._42 = m_Camera_Ptr->vecPosition.y;
				pCurrentA->position.matWorld._43 = m_Camera_Ptr->vecPosition.z;
				pCurrentA->position.vecRotate = GGVECTOR3 ( 0, 0, 0 );
				if(pCurrentA->position.vecPosition.x!=fLastCamX
				|| pCurrentA->position.vecPosition.y!=fLastCamY
				|| pCurrentA->position.vecPosition.z!=fLastCamZ )
				{
					pCurrentA->collision.bHasBeenMovedForResponse = true;
				}
			}

			// if object A 'has' moved this cycle, deal with auto-adjustment (automatic collision)
			if(pCurrentA->collision.bHasBeenMovedForResponse==true)
			{
				for ( int iShortListB = -1; iShortListB < g_iObjectListRefCount; iShortListB++ )
				{
					// get index from shortlist
					int iCurrentObjectB;
					if ( iShortListB==-1 )
						iCurrentObjectB = -1;
					else
						iCurrentObjectB = g_ObjectListRef [ iShortListB ];

					// CameraObject or Object pointer
					sObject* pCurrentB;
					if ( iCurrentObjectB == -1 )
						pCurrentB = &CameraObject;
					else
					{
						pCurrentB = g_ObjectList [ iCurrentObjectB ];
						if ( pCurrentB == NULL ) continue;
					}

					// lee - 060406 - u6rc6 - quick reject if camera moved by moving object (so cannot be pushed around)
					if ( g_iCameraCollisionBehaviourMode==1 )
						if ( iCurrentObjectID==-1 && pCurrentB->position.vecPosition!=pCurrentB->position.vecLastPosition )
							continue;

					// compare all pairs
					if(pCurrentA!=pCurrentB)
					{
						if(pCurrentB->collision.bActive)
						{
							// Compare A with B for collision
							if( GetOverlapCollisionResultCore ( iCurrentObjectID, pCurrentA, iCurrentObjectB, pCurrentB ) > 0 )
							{
								// Apply any sliding data (only if response mode is 1)
								float fAdjX = DBV1_ObjectCollisionXF();
								float fAdjY = DBV1_ObjectCollisionYF();
								float fAdjZ = DBV1_ObjectCollisionZF();
								if(bResponseDataAvailableFromDBV1==true && pCurrentA->collision.iResponseMode==1)
								{
									// Response Mode 0(as 1) : Adjust position for sliding response
									pCurrentA->position.vecPosition.x-=fAdjX;
									pCurrentA->position.vecPosition.y-=fAdjY;
									pCurrentA->position.vecPosition.z-=fAdjZ;
								}
								else
								{
									// Response Mode 1(as 2) : Adjust old position for restoration response
									pCurrentA->position.vecPosition = pCurrentA->position.vecLastPosition;
								}

								// do not allow rotation to succeed if collision (causes sticky objects)
								float fRotationDiff = (float)fabs(pCurrentA->position.vecRotate.x - pCurrentA->position.vecLastRotate.x);
								fRotationDiff += (float)fabs(pCurrentA->position.vecRotate.y - pCurrentA->position.vecLastRotate.y);
								fRotationDiff += (float)fabs(pCurrentA->position.vecRotate.z - pCurrentA->position.vecLastRotate.z);
								if ( fRotationDiff > 0.0f && pCurrentA->collision.eCollisionType!=COLLISION_SPHERE )
								{
									// lee - 240306 - u6b4 - restore to old rotation, easiest  pure solution, allow spheres to rotate too
									pCurrentA->position.vecRotate = pCurrentA->position.vecLastRotate;
									pCurrentA->position.matFreeFlightRotate = pCurrentA->position.matLastFreeFlightRotate;
									RegenerateLookVectors( pCurrentA );

									/* lee - 240306 - u6b4 - the whole concept of pushng back is flawed
									// restore rotation
									pCurrentA->position.vecRotate = pCurrentA->position.vecLastRotate;
									RegenerateLookVectors( pCurrentA );

									// also push out to keep collision system fluid
									GGVECTOR3 vecDirAwayFromA = pCurrentA->position.vecPosition - pCurrentB->position.vecPosition;

									// lee - 240306 - u6b4 - Normalizing inflates if the values are smaller than zero ( glitching )
									GGVec3Normalize ( &vecDirAwayFromA, &vecDirAwayFromA );
									// so scale it with the scaled radius of the object (one Xth of radius slide)
									vecDirAwayFromA *= pCurrentA->collision.fScaledLargestRadius/50.0f;

									// leefix - 150306 - u60b3 - caused objects to stick (as rotating objects would create un-necessary force)
									if ( fAdjX != 0.0f ) pCurrentA->position.vecPosition.x += vecDirAwayFromA.x;
									if ( fAdjZ != 0.0f ) pCurrentA->position.vecPosition.z += vecDirAwayFromA.z;
									*/
								}

								// response data as DBV1 format
								bResponseDataAvailableFromDBV1 = true;

								// if obj=camera, apply to camera too
								if(iCurrentObjectID==-1)
								{
									tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( m_iCameraUsingForCollision );
									m_Camera_Ptr->vecPosition = pCurrentA->position.vecPosition;
									CameraInternalUpdate ( m_iCameraUsingForCollision );
								}
							}
						}
					}
				}
			}
		}
	}
}

void AutoObjectCol ( sObject* pObject, float fRadius, int iResponse )
{
	// Prepare new radius and bound box (if specified)
	if ( fRadius > 0.0f )
	{
		// Prepare new radius and boundbox size
		pObject->collision.fRadius = fRadius;
		AdjustBoundBoxToCollisionSize ( pObject, fRadius );
	}

	// Response Mode
	pObject->collision.iResponseMode = 1+iResponse;
	pObject->collision.bBoundProduceResult = true;

	// Set start object position
	pObject->position.vecLastPosition = pObject->position.vecPosition;
}

void AutoCameraCol ( int iCameraID, float fRadius, int iResponse, int iStandGroundMode )
{
	// update internal data
	if ( iCameraID < 0 || iCameraID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_CAMERANUMBERILLEGAL);
		return;
	}
	if ( GetCameraInternalData ( iCameraID )==NULL )
	{
		RunTimeError(RUNTIMEERROR_CAMERANOTEXIST);
		return;
	}
	else
	{
		return;
	}

	// set global camera collision flag
	g_iCameraCollisionBehaviourMode = iStandGroundMode;

	// Used to hold camera index
	m_iCameraUsingForCollision = iCameraID;

	// Special camera collision object
	memset ( &CameraObject, sizeof(CameraObject), 0);

	// Prepare radius and boundbox size (camera has no native collision type)
	CameraObject.collision.fRadius = fRadius;
	AdjustBoundBoxToCollisionSize ( &CameraObject, fRadius );

	// Elongate camera bound box for human sized area
	CameraObject.collision.vecMin.y *= 3.3f;
	CameraObject.collision.vecMax.y *= 3.3f;

	// Set standard collision defaults
	CameraObject.collision.bActive=true;
	CameraObject.collision.eCollisionType = COLLISION_BOX;
// lee - 150306 - u60b3 - caused camera collision to stick on floor
//	CameraObject.collision.bFixedBoxCheck=false;
	CameraObject.collision.bFixedBoxCheck=true;
	CameraObject.collision.iResponseMode=1+iResponse;
	CameraObject.bVisible=false;

	// Set start position from current camera
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( m_iCameraUsingForCollision );
	CameraObject.position.vecLastPosition = m_Camera_Ptr->vecPosition;
	CameraObject.position.vecPosition = m_Camera_Ptr->vecPosition;
	fLastCamX = m_Camera_Ptr->vecPosition.x;
	fLastCamY = m_Camera_Ptr->vecPosition.y;
	fLastCamZ = m_Camera_Ptr->vecPosition.z;
}

void AutoCameraCol ( int iCameraID, float fRadius, int iResponse )
{
	// see above
	AutoCameraCol ( iCameraID, fRadius, iResponse, 0 );
}
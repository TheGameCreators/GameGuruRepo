#include "DBPro Functions.h"

#include "LMObject.h"
#include "LMPolyGroup.h"
#include "LMPoly.h"
#include "LMTexture.h"
#include "Light.h"
#include "CollisionTreeLightmapper.h"

#include "CObjectsC.h"

LMObject::LMObject( sObject *pParentObject, sFrame *pParentFrame, sMesh *pNewMesh )
{
	New(pParentObject, pParentFrame, pNewMesh);
}

LMObject::~LMObject( )
{
	Free();
}

void LMObject::New ( sObject *pParentObject, sFrame *pParentFrame, sMesh *pNewMesh )
{
	pMesh = pNewMesh;
	pFrame = pParentFrame;
	pObject = pParentObject;

	pNextObject = 0;
	pPolyList = 0;
	pPolyGroupList = 0;
	iNumPolys = 0;
	iBaseStage = 0;
	iDynamicLightMode = 0;
	fQuality = 0;
	bShaded = false;
	bIgnoreNormals = false;

	pLMTexture = 0;
	iID = 0;
	iLimbID = 0;
}

void LMObject::Free ( void )
{
	LMPolyGroup *pTempGroup;
	while ( pPolyGroupList )
	{
		pTempGroup = pPolyGroupList;
		pPolyGroupList = pPolyGroupList->pNextGroup;
		delete pTempGroup;
	}

	// lee - 240914 - if did not add polys to group, delete them from LMObject where they where originally created
	while ( pPolyList )
	{
		LMPoly* pTempPoly = pPolyList;
		pPolyList = pPolyList->pNextPoly;
		delete pTempPoly;
	}
	pPolyList = 0;
}

sObject* LMObject::GetObjectPtr( )
{
	return pObject;
}

sFrame* LMObject::GetFramePtr( )
{
	return pFrame;
}

sMesh* LMObject::GetMeshPtr( )
{
	return pMesh;
}

void LMObject::SetBaseStage( int iNewStage )
{
	if ( iNewStage < 0 || iNewStage > 7 ) iNewStage = -1;
	iBaseStage = iNewStage;
}

void LMObject::SetShaded( int iLightMapStage )
{
	bShaded = true;
	iBaseStage = iLightMapStage;
}

bool LMObject::GetIsShaded( )
{
	return bShaded;
}

void LMObject::SetDynamicLightMode( int iNewMode )
{
	if ( iNewMode < 0 ) iNewMode = 0;
	iDynamicLightMode = iNewMode;
}

LMPolyGroup* LMObject::GetFirstGroup( )
{
	return pPolyGroupList;
}

int LMObject::GetNumPolys( )
{
	return iNumPolys;
}

float LMObject::GetQuality( )
{
	return fQuality;
}

bool LMObject::WillFit( int iTexSizeU, int iTexSizeV )
{
	int iAvailArea = iTexSizeU*iTexSizeV;
	int iTotalArea = 0;

	LMPolyGroup *pGroup = pPolyGroupList;
	int iMaxU=0, iMaxV=0;

	while ( pGroup )
	{
		iTotalArea += pGroup->GetScaledSizeU( ) * pGroup->GetScaledSizeV( );

		if ( pGroup->GetScaledSizeU( ) > iMaxU ) iMaxU = pGroup->GetScaledSizeU( );
		if ( pGroup->GetScaledSizeV( ) > iMaxV ) iMaxV = pGroup->GetScaledSizeV( );

		pGroup = pGroup->pNextGroup;
	}

	bool bWillFit = iTotalArea*1.01 < iAvailArea;
	bWillFit = bWillFit && iMaxU < iTexSizeU && iMaxV < iTexSizeV;

	return bWillFit;
}

float LMObject::RecomendedQaulity( int iTexSizeU, int iTexSizeV )
{
	float fAvailArea = (float) iTexSizeU*iTexSizeV;
	float fTotalArea = 0;

	LMPolyGroup *pGroup = pPolyGroupList;

	while ( pGroup )
	{
		fTotalArea += pGroup->GetSizeU( ) * pGroup->GetSizeV( );

		pGroup = pGroup->pNextGroup;
	}

	//unfinished

	return 1.0;
}

void LMObject::BuildPolyList( bool bFlatShaded )
{
	CalculateAbsoluteWorldMatrix ( pObject, pFrame, pMesh );

	BYTE *pVertexData = pMesh->pVertexData;
	WORD *pIndexData = pMesh->pIndices;
	DWORD dwFVFSize = pMesh->dwFVFSize;
	int iNumVertices = pMesh->dwVertexCount;
	int iNumIndices = pMesh->dwIndexCount;
	
	Point p1,p2,p3;
	GGVECTOR3 vecVert;

	fObjCenX = 0; fObjCenY = 0; fObjCenZ = 0;
	float fPosX = 0;
	float fPosY = 0;
	float fPosZ = 0;
	iNumPolys = 0;
	float *pU1, *pU2, *pU3;
	float *pV1, *pV2, *pV3;
	Vector normV1, normV2, normV3;
	Point diffuv1, diffuv2, diffuv3;

	sOffsetMap cOffsetMap;
	GetFVFOffsetMap( pMesh, &cOffsetMap );

	int iLMStage = 1;
	if ( bShaded ) iLMStage = iBaseStage;
	
	GGMATRIX * pmatTransInvAbsoluteWorld = new GGMATRIX;
	GGMatrixInverse( pmatTransInvAbsoluteWorld, NULL, &pFrame->matAbsoluteWorld );
	GGMatrixTranspose( pmatTransInvAbsoluteWorld, pmatTransInvAbsoluteWorld );

	// iterate through index list or pure vertex list
	int iIterCount = iNumVertices;
	if ( iNumIndices > 0 ) iIterCount = iNumIndices;
	for ( int i = 0; i < iIterCount; i+=3 )
	{
		// choose vertex or index based vertex
		DWORD dwIndex0, dwIndex1, dwIndex2;
		if ( iNumIndices > 0 )
		{
			dwIndex0 = (pIndexData[i+0])*dwFVFSize;
			dwIndex1 = (pIndexData[i+1])*dwFVFSize;
			dwIndex2 = (pIndexData[i+2])*dwFVFSize;
		}
		else
		{
			dwIndex0 = (i+0)*dwFVFSize;
			dwIndex1 = (i+1)*dwFVFSize;
			dwIndex2 = (i+2)*dwFVFSize;
		}

		// vertex 0
		vecVert.x = *( (float*) ( pVertexData + dwIndex0 )     );
		vecVert.y = *( (float*) ( pVertexData + dwIndex0 + 4 ) );
		vecVert.z = *( (float*) ( pVertexData + dwIndex0 + 8 ) );
		if ( cOffsetMap.dwNX>0 )
		{
			normV1.x = *( (float*) ( pVertexData + dwIndex0 + (cOffsetMap.dwNX*4) ) );
			normV1.y = *( (float*) ( pVertexData + dwIndex0 + (cOffsetMap.dwNY*4) ) );
			normV1.z = *( (float*) ( pVertexData + dwIndex0 + (cOffsetMap.dwNZ*4) ) );
		}
		if ( iLMStage==1 )
		{
			diffuv1.x = *(float*) ( pVertexData + dwIndex0 + cOffsetMap.dwTU [ 0 ]*4 );
			diffuv1.y = *(float*) ( pVertexData + dwIndex0 + cOffsetMap.dwTV [ 0 ]*4 );
		}
		pU1 = (float*) ( pVertexData + dwIndex0 + cOffsetMap.dwTU [ iLMStage ]*4 );
		pV1 = (float*) ( pVertexData + dwIndex0 + cOffsetMap.dwTV [ iLMStage ]*4 );
		GGVec3TransformCoord( &vecVert, &vecVert, &pFrame->matAbsoluteWorld );
		p1.x = vecVert.x; p1.y = vecVert.y; p1.z = vecVert.z;
		vecVert.x = normV1.x; vecVert.y = normV1.y; vecVert.z = normV1.z;
		GGVec3TransformNormal( &vecVert, &vecVert, pmatTransInvAbsoluteWorld );
		normV1.x = vecVert.x; normV1.y = vecVert.y; normV1.z = vecVert.z;

		// vertex 1
		vecVert.x = *( (float*) ( pVertexData + dwIndex1 )     );
		vecVert.y = *( (float*) ( pVertexData + dwIndex1 + 4 ) );
		vecVert.z = *( (float*) ( pVertexData + dwIndex1 + 8 ) );
		if ( cOffsetMap.dwNX>0 )
		{
			normV2.x = *( (float*) ( pVertexData + dwIndex1 + (cOffsetMap.dwNX*4) ) );
			normV2.y = *( (float*) ( pVertexData + dwIndex1 + (cOffsetMap.dwNY*4) ) );
			normV2.z = *( (float*) ( pVertexData + dwIndex1 + (cOffsetMap.dwNZ*4) ) );
		}
		if ( iLMStage==1 )
		{
			diffuv2.x = *(float*) ( pVertexData + dwIndex1 + cOffsetMap.dwTU [ 0 ]*4 );
			diffuv2.y = *(float*) ( pVertexData + dwIndex1 + cOffsetMap.dwTV [ 0 ]*4 );
		}
		pU2 = (float*) ( pVertexData + dwIndex1 + cOffsetMap.dwTU [ iLMStage ]*4 );
		pV2 = (float*) ( pVertexData + dwIndex1 + cOffsetMap.dwTV [ iLMStage ]*4 );
		GGVec3TransformCoord( &vecVert, &vecVert, &pFrame->matAbsoluteWorld );
		p2.x = vecVert.x; p2.y = vecVert.y; p2.z = vecVert.z;
		vecVert.x = normV2.x; vecVert.y = normV2.y; vecVert.z = normV2.z;
		GGVec3TransformNormal( &vecVert, &vecVert, pmatTransInvAbsoluteWorld );
		normV2.x = vecVert.x; normV2.y = vecVert.y; normV2.z = vecVert.z;

		// vertex 2
		vecVert.x = *( (float*) ( pVertexData + dwIndex2 )     );
		vecVert.y = *( (float*) ( pVertexData + dwIndex2 + 4 ) );
		vecVert.z = *( (float*) ( pVertexData + dwIndex2 + 8 ) );
		if ( cOffsetMap.dwNX>0 )
		{
			normV3.x = *( (float*) ( pVertexData + dwIndex2 + (cOffsetMap.dwNX*4) ) );
			normV3.y = *( (float*) ( pVertexData + dwIndex2 + (cOffsetMap.dwNY*4) ) );
			normV3.z = *( (float*) ( pVertexData + dwIndex2 + (cOffsetMap.dwNZ*4) ) );
		}
		if ( iLMStage==1 )
		{
			diffuv3.x = *(float*) ( pVertexData + dwIndex2 + cOffsetMap.dwTU [ 0 ]*4 );
			diffuv3.y = *(float*) ( pVertexData + dwIndex2 + cOffsetMap.dwTV [ 0 ]*4 );
		}
		pU3 = (float*) ( pVertexData + dwIndex2 + cOffsetMap.dwTU [ iLMStage ]*4 );
		pV3 = (float*) ( pVertexData + dwIndex2 + cOffsetMap.dwTV [ iLMStage ]*4 );
		GGVec3TransformCoord( &vecVert, &vecVert, &pFrame->matAbsoluteWorld );
		p3.x = vecVert.x; p3.y = vecVert.y; p3.z = vecVert.z;
		vecVert.x = normV3.x; vecVert.y = normV3.y; vecVert.z = normV3.z;
		GGVec3TransformNormal( &vecVert, &vecVert, pmatTransInvAbsoluteWorld );
		normV3.x = vecVert.x; normV3.y = vecVert.y; normV3.z = vecVert.z;

		// work out polygon normal and length of normal
		Vector v1(&p1,&p3);
		Vector v2(&p1,&p2);
		v1 = v1.crossProduct(&v2);
		float length = v1.size();

		// zero area polygons will have an undefined normal, which will cause problems later
		// so if any exist remove them
		//if (length<0.0001f) continue;  // 271114 - screws up poly count!!

		// lee - 021014 - if no normals, calculate them from the polygon direction
		if ( cOffsetMap.dwNX==0 )
		{
			normV1 = v1;
			normV2 = v1;
			normV3 = v1;
		}
		normV1.normalise( );
		normV2.normalise( );
		normV3.normalise( );
		float fDotP1 = normV1.dotProduct( &normV2 );
		float fDotP2 = normV1.dotProduct( &normV3 );
		v1.mult(1.0f/length);
		float fDotP3 = normV1.dotProduct( &v1 );

		// add to master poly list
		LMPoly* pNewPoly = NULL;
		if ( bFlatShaded ) // 020718 - ignoring normals!! || ( fDotP1 > 0.995f && fDotP2 > 0.995f && fDotP3 > 0.995f ) )
		{
			// flat polygon
			pNewPoly = new LMPoly( );
		}
		else
		{
			// curved polygon
			pNewPoly = new LMCurvedPoly( );
			((LMCurvedPoly*)pNewPoly)->SetVertexNormals( &normV1, &normV2, &normV3 );
		}
		if ( pNewPoly )
		{
			pNewPoly->fU1 = *pU1; pNewPoly->fV1 = *pV1;
			pNewPoly->fU2 = *pU2; pNewPoly->fV2 = *pV2;
			pNewPoly->fU3 = *pU3; pNewPoly->fV3 = *pV3;
			pNewPoly->SetVertices( &p1, &p2, &p3 );
			pNewPoly->GetAvgPoint( &fPosX, &fPosY, &fPosZ );
			fObjCenX += fPosX;
			fObjCenY += fPosY;
			fObjCenZ += fPosZ;
			pNewPoly->SetNormal( &v1 );
			pNewPoly->SetDiffuseUV ( &diffuv1, &diffuv2, &diffuv3 );
			v2.set(p2.x,p2.y,p2.z);
			pNewPoly->d = -1.0f*(v1.dotProduct(&v2));
			pNewPoly->Flatten( );
			pNewPoly->colour = 0xff000000 | ((rand() % 255) << 16) | ((rand() % 255) << 8) | ((rand() % 255));
			pNewPoly->pNextPoly = pPolyList;
			pPolyList = pNewPoly;
			iNumPolys++;
		}
	}
	SAFE_DELETE ( pmatTransInvAbsoluteWorld );

	// do not finalise if no polygons added to list
	if ( iNumPolys <= 0 ) return;

	// finalise centre position
	fObjCenX /= iNumPolys;
	fObjCenY /= iNumPolys;
	fObjCenZ /= iNumPolys;

	// work out radius
	fObjRadius = 0;
	float fNewRadius;
	LMPoly *pPoly = pPolyList;
	while ( pPoly )
	{
		fNewRadius = pPoly->GetMaxRadius( fObjCenX, fObjCenY, fObjCenZ );
		if ( fNewRadius > fObjRadius ) fObjRadius = fNewRadius;
		pPoly = pPoly->pNextPoly;
	}
	fObjRadius = sqrt( fObjRadius );
}

void LMObject::GroupPolys( float fNewQuality )
{
	fQuality = fNewQuality;

	LMPoly *pPoly = pPolyList;
	LMPoly *pNextPoly;

	while ( pPoly )
	{
		pNextPoly = pPoly->pNextPoly;

		LMPolyGroup *pNewGroup = new LMPolyGroup( );
		pNewGroup->AddPoly( pPoly );

		int iNumIterations = 0;
		float fUsedSpace = pNewGroup->GetUsedSpace( );
		bool bFound = true;

		while ( fUsedSpace < 0.999f && iNumIterations < 20 && bFound ) 
		{ 
			bFound = false;

			LMPoly *pPoly2 = pNextPoly;
			LMPoly *pLastPoly2 = 0;
		
			LMPoly *pPolyIndex = 0;
			LMPoly *pLastPolyIndex = 0;

			//search for another poly to add to this group
			while ( pPoly2 )
			{
				if ( pNewGroup->Joined( pPoly2 ) )
				{
					float fNewUsedSpace = pNewGroup->GetNewUsedSpace( pPoly2 );

					if ( fNewUsedSpace > fUsedSpace )
					{
						fUsedSpace = fNewUsedSpace;
						pPolyIndex = pPoly2;
						pLastPolyIndex = pLastPoly2;
					}

					bFound = true;
				}
				
				pLastPoly2 = pPoly2;
				pPoly2 = pPoly2->pNextPoly;
			}

			if ( pPolyIndex )
			{
				if ( pPolyIndex == pNextPoly )
				{
					pNextPoly = pPolyIndex->pNextPoly;
				}
				else
				{
					pLastPolyIndex->pNextPoly = pPolyIndex->pNextPoly;
				}

				pNewGroup->AddPoly( pPolyIndex );
			}

			iNumIterations++;
		}
		
		
		pNewGroup->Scale( fQuality );
		pNewGroup->pNextGroup = pPolyGroupList;
		pPolyGroupList = pNewGroup;

		pPoly = pNextPoly;
	}

	pPolyList = 0;
}

void LMObject::SortGroups( )
{
	if ( !pPolyGroupList ) return;
	
	LMPolyGroup *pLastGroup = 0;
	LMPolyGroup *pGroup = pPolyGroupList;

	while ( pGroup )
	{
		int iMaxSize = pGroup->GetScaledSizeU( ) * pGroup->GetScaledSizeV( );

		LMPolyGroup *pLastGroup2 = 0;
		LMPolyGroup *pLastLargestGroup2 = 0;
		LMPolyGroup *pGroup2 = pGroup->pNextGroup;
		LMPolyGroup *pLargestGroup = 0;

		while ( pGroup2 )
		{
			int iNewSize = pGroup2->GetScaledSizeU( ) * pGroup2->GetScaledSizeV( );

			if ( iNewSize > iMaxSize )
			{
				iMaxSize = iNewSize;
				pLargestGroup = pGroup2;
				pLastLargestGroup2 = pLastGroup2;
			}

			pLastGroup2 = pGroup2;
			pGroup2 = pGroup2->pNextGroup;
		}

		if ( pLargestGroup )
		{
			LMPolyGroup *pTemp = pGroup->pNextGroup;
			LMPolyGroup *pTemp2 = pLargestGroup->pNextGroup;

			if ( pTemp == pLargestGroup ) pTemp = pGroup;

			pLargestGroup->pNextGroup = pTemp;
			pGroup->pNextGroup = pTemp2;

			if ( pLastLargestGroup2 ) pLastLargestGroup2->pNextGroup = pGroup;
			
			if ( !pLastGroup )
				pPolyGroupList = pLargestGroup;
			else
				pLastGroup->pNextGroup = pLargestGroup;

			pLastGroup = pLargestGroup;
			pGroup = pLargestGroup->pNextGroup;
		}
		else
		{
			pLastGroup = pGroup;
			pGroup = pGroup->pNextGroup;
		}
	}
}

void LMObject::ReScaleGroups( float fNewQuality )
{
	fQuality = fNewQuality;
	
	LMPolyGroup *pGroup = pPolyGroupList;

	while ( pGroup )
	{
		pGroup->Scale( fQuality );

		pGroup = pGroup->pNextGroup;
	}
}

void LMObject::SetLocalValues( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, HANDLE pSemaphore )
{
	pLocalLightList = pLightList;
	pLocalColTree = pColTree;
	iLocalBlur = iBlur;
	pLocalSemaphore = pSemaphore;
}

void LMObject::WaitForThreads( )
{
	LMPolyGroup *pGroup = pPolyGroupList;
	while ( pGroup )
	{
//		pGroup->Join( );
		pGroup = pGroup->pNextGroup;
	}
}

unsigned int LMObject::Run( )
{
	CalculateLight( pLocalLightList, pLocalColTree, iLocalBlur, bIgnoreNormals, 0 );
	ApplyToTexture( );

	ReleaseSemaphore( pLocalSemaphore, 1, NULL );
	 
	return 0;
}

void LMObject::CalculateLight( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, bool bPassInIgnoreNormals, int iThreadLimit )
{
	LMPolyGroup *pGroup = pPolyGroupList;
	while ( pGroup )
	{
		if ( GetAsyncKeyState(VK_ESCAPE)!=0 ) break;
		pGroup->CalculateLight( pLightList, pColTree, iBlur, bPassInIgnoreNormals );
		pGroup = pGroup->pNextGroup;
	}
}

void LMObject::ApplyToTexture( )
{
	if ( !pLMTexture ) return;
	LMPolyGroup *pGroup = pPolyGroupList;
	while ( pGroup )
	{
		if ( GetAsyncKeyState(VK_ESCAPE)!=0 ) break;
		pGroup->ApplyToTexture( pLMTexture );
		pGroup = pGroup->pNextGroup;
	}
}

void LMObject::CalculateVertexUV( int iTexSizeU, int iTexSizeV )
{
	LMPolyGroup *pPolyGroup = pPolyGroupList;

	while ( pPolyGroup )
	{
		pPolyGroup->CalculateTexUV( iTexSizeU, iTexSizeV );

		pPolyGroup = pPolyGroup->pNextGroup;
	}
}

void LMObject::UpdateObject( int iBlendMode )
{
	if ( !pLMTexture )return;
	if ( !pMesh ) return;

	if ( !pMesh->pTextures )
	{
		MessageBox( NULL, "Invalid Texture List", "Error", 0 );
		return;
	}

	if ( pMesh->dwTextureCount < 2 && !bShaded )
	{
		char str[256];
		sprintf_s(str,255,"Not Enough Texture Stages (%d)", pMesh->dwTextureCount );
		MessageBox( NULL, str, "Error", 0 );
		return;
	}

	int iLMStage = 0;
	if ( !bShaded )
	{
		if ( iBaseStage >= 0 ) pMesh->pTextures [ 1 ]					= pMesh->pTextures [ iBaseStage ];
	}
	else
	{
		iLMStage = iBaseStage;
		if ( pMesh->dwTextureCount < 2 && iLMStage > 0 )
		{
			char str[256];
			sprintf_s(str,255,"Object Only Has ONE Texture Stage, lightmapper needs %d", 1+iLMStage );
			MessageBox( NULL, str, "Error", 0 );
			return;
		}
	}

	// setup lightmap as FIRST stage (diffuse+lightmap)
	pMesh->pTextures [ iLMStage ].dwStage			= iLMStage;
	pMesh->pTextures [ iLMStage ].iImageID			= 0;
	pMesh->pTextures [ iLMStage ].pTexturesRef		= pLMTexture->GetDXTextureRef( );
	strcpy_s ( pMesh->pTextures [ iLMStage ].pName, 255   , pLMTexture->GetFilename( ) );
	
	if ( !bShaded )
	{
		if ( iDynamicLightMode == 0 )
		{
			pMesh->pTextures [ iLMStage ].dwBlendMode		= GGTOP_SELECTARG1;
			pMesh->bLight = false;
		}
		else
		{
			pMesh->pTextures [ iLMStage ].dwBlendMode		= GGTOP_ADD;
			pMesh->bLight = true;
		}
	}

	pMesh->pTextures [ iLMStage ].dwBlendArg1		= GGTA_TEXTURE;
	if ( !bShaded ) pMesh->pTextures [ iLMStage ].dwBlendArg2 = GGTA_DIFFUSE;
	else pMesh->pTextures [ iLMStage ].dwBlendArg2 = GGTA_CURRENT;
	pMesh->pTextures [ iLMStage ].dwAddressU		= GGTADDRESS_CLAMP;
	pMesh->pTextures [ iLMStage ].dwAddressV		= GGTADDRESS_CLAMP;
	pMesh->pTextures [ iLMStage ].dwMagState		= GGTEXF_LINEAR;//2x2 sample
	pMesh->pTextures [ iLMStage ].dwMinState		= GGTEXF_LINEAR;
	pMesh->pTextures [ iLMStage ].dwMipState		= GGTEXF_NONE;
	pMesh->pTextures [ iLMStage ].dwTexCoordMode	= 10 + iLMStage;

	// copy base texture to SECOND stage (current*basetexture)
	if ( !bShaded )
	{
		pMesh->pTextures [ 1 ].dwStage			= 1;
		pMesh->pTextures [ 1 ].dwBlendMode		= iBlendMode;
		pMesh->pTextures [ 1 ].dwBlendArg1		= GGTA_TEXTURE;
		pMesh->pTextures [ 1 ].dwBlendArg2		= GGTA_CURRENT;
		pMesh->pTextures [ 1 ].dwTexCoordMode	= 10;
		pMesh->pTextures [ 0 ].dwTexCoordMode	= 11;
	}
	
	if ( pMesh->dwTextureCount > 2 && !bShaded )
	{
		for ( DWORD i = 2; i < pMesh->dwTextureCount; i++ )
		{
			pMesh->pTextures [ i ].dwBlendMode		= GGTOP_DISABLE;
			pMesh->pTextures [ i ].dwBlendArg2		= GGTA_CURRENT;
			pMesh->pTextures [ i ].dwBlendArg1		= GGTA_TEXTURE;
			pMesh->pTextures [ i ].dwStage			= i;
			pMesh->pTextures [ i ].iImageID			= -1;
			pMesh->pTextures [ i ].pTexturesRef		= 0;
		}
	}
}

void LMObject::CreateTriOnlyAndApplyUVData ( void )
{
	// count all polys
	int iTotalPolyCount = 0;
	LMPolyGroup *pPolyGroup = pPolyGroupList;
	while ( pPolyGroup )
	{
		LMPoly* pPolyPtr = pPolyGroup->pPolyList;
		while ( pPolyPtr )
		{
			iTotalPolyCount++;
			pPolyPtr = pPolyPtr->pNextPoly;
		}
		pPolyGroup = pPolyGroup->pNextGroup;
	}

	// channel to write to
	int iStageForUV = iBaseStage;
	if ( iBaseStage==1 )
	{
		int lee=42;
	}

	// make a tri-only new mesh contents
	ConvertLocalMeshToVertsOnly ( pMesh, false );
	ConvertToFVF( pMesh, pMesh->dwFVF );
	DWORD dwVertexCount = iTotalPolyCount * 3;
	if ( pMesh->dwVertexCount==dwVertexCount )
	{
		// get update new tri-only mesh if poly data matches count
		DWORD dwFVFSize = pMesh->dwFVFSize;
		BYTE *pVertexData = pMesh->pVertexData;
		sOffsetMap cOffsetMap;
		GetFVFOffsetMap( pMesh, &cOffsetMap );

		// get inverse matrices (to restore local object space values in new vert data)
		float fDet;
		GGMATRIX matInvWorld;
		GGMatrixInverse(&matInvWorld,&fDet,&pFrame->matAbsoluteWorld);

		// 160718 - old code seems to include world position in the inverse (MUST mess up normals!)
		// OR IT COULD BE THE WRONG NORMALS ARE BEING PASSED IN - rotate entity 270 degrees, see normals mess up!!
		GGMATRIX matInvTransposedWorld;
		GGMATRIX * pmatTransInvAbsoluteWorld = new GGMATRIX;
		GGMatrixInverse( pmatTransInvAbsoluteWorld, NULL, &pFrame->matAbsoluteWorld );
		GGMatrixTranspose( pmatTransInvAbsoluteWorld, pmatTransInvAbsoluteWorld );
		GGMatrixInverse(&matInvTransposedWorld,&fDet,pmatTransInvAbsoluteWorld);
		//GGMATRIX matInvNoTransWorld = pFrame->matAbsoluteWorld;
		//matInvNoTransWorld._41 = 0;
		//matInvNoTransWorld._42 = 0;
		//matInvNoTransWorld._43 = 0;
		//GGMatrixInverse(&matInvNoTransWorld,&fDet,&matInvNoTransWorld);

		// fill new vertex data block
		GGVECTOR3 vecVert;
		GGVECTOR3 vecNorm;
		DWORD dwVertexIndex = 0;
		LMPolyGroup *pPolyGroup = pPolyGroupList;
		while ( pPolyGroup )
		{
			LMPoly* pPolyPtr = pPolyGroup->pPolyList;
			while ( pPolyPtr )
			{
				// vertex 0
				vecVert.x = pPolyPtr->vert1[0];
				vecVert.y = pPolyPtr->vert1[1];
				vecVert.z = pPolyPtr->vert1[2];
				GGVec3TransformCoord ( &vecVert, &vecVert, &matInvWorld );
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwX*4) ) ) = vecVert.x;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwY*4) ) ) = vecVert.y;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwZ*4) ) ) = vecVert.z;
				if ( cOffsetMap.dwNX > 0 )
				{
					// 020117 - need original normals thanks!
					//if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					//{
					//	vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv1[0];
					//	vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv1[1];
					//	vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv1[2];
					//}
					//else
					// 020718 - reinstated curvedpolys
					if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					{
						vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv1[0];
						vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv1[1];
						vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv1[2];
					}
					else
					{
						vecNorm.x = pPolyPtr->normal[0]; 
						vecNorm.y = pPolyPtr->normal[1];
						vecNorm.z = pPolyPtr->normal[2];
					}
					GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvTransposedWorld );
					//GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvNoTransWorld );
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNX*4) ) ) = vecNorm.x;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNY*4) ) ) = vecNorm.y;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNZ*4) ) ) = vecNorm.z;
				}
				if ( iStageForUV==1 )
				{
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[0]*4) ) ) = pPolyPtr->diffuseuv1[0];
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[0]*4) ) ) = pPolyPtr->diffuseuv1[1];
				}
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[iStageForUV]*4) ) ) = pPolyPtr->fU1;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[iStageForUV]*4) ) ) = pPolyPtr->fV1;
				dwVertexIndex += dwFVFSize;

				// vertex 1
				vecVert.x = pPolyPtr->vert2[0];
				vecVert.y = pPolyPtr->vert2[1];
				vecVert.z = pPolyPtr->vert2[2];
				GGVec3TransformCoord ( &vecVert, &vecVert, &matInvWorld );
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwX*4) ) ) = vecVert.x;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwY*4) ) ) = vecVert.y;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwZ*4) ) ) = vecVert.z;
				if ( cOffsetMap.dwNX > 0 )
				{
					// 020117 - need original normals thanks!
					//if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					//{
					//	vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv2[0];
					//	vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv2[1];
					//	vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv2[2];
					//}
					//else
					// 020718 - reinstated curvedpolys
					if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					{
						vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv2[0];
						vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv2[1];
						vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv2[2];
					}
					else
					{
						vecNorm.x = pPolyPtr->normal[0];
						vecNorm.y = pPolyPtr->normal[1];
						vecNorm.z = pPolyPtr->normal[2];
					}
					GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvTransposedWorld );
					//GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvNoTransWorld );
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNX*4) ) ) = vecNorm.x;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNY*4) ) ) = vecNorm.y;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNZ*4) ) ) = vecNorm.z;
				}
				if ( iStageForUV==1 )
				{
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[0]*4) ) ) = pPolyPtr->diffuseuv2[0];
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[0]*4) ) ) = pPolyPtr->diffuseuv2[1];
				}
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[iStageForUV]*4) ) ) = pPolyPtr->fU2;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[iStageForUV]*4) ) ) = pPolyPtr->fV2;
				dwVertexIndex += dwFVFSize;

				// vertex 2
				vecVert.x = pPolyPtr->vert3[0];
				vecVert.y = pPolyPtr->vert3[1];
				vecVert.z = pPolyPtr->vert3[2];
				GGVec3TransformCoord ( &vecVert, &vecVert, &matInvWorld );
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwX*4) ) ) = vecVert.x;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwY*4) ) ) = vecVert.y;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwZ*4) ) ) = vecVert.z;
				if ( cOffsetMap.dwNX > 0 )
				{
					// 020117 - need original normals thanks!
					//if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					//{
					//	vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv3[0];
					//	vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv3[1];
					//	vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv3[2];
					//}
					//else
					// 020718 - reinstated curvedpolys
					if ( ((LMCurvedPoly*)pPolyPtr)->IsCurved() )
					{
						vecNorm.x = ((LMCurvedPoly*)pPolyPtr)->normalv3[0];
						vecNorm.y = ((LMCurvedPoly*)pPolyPtr)->normalv3[1];
						vecNorm.z = ((LMCurvedPoly*)pPolyPtr)->normalv3[2];
					}
					else
					{
						vecNorm.x = pPolyPtr->normal[0];
						vecNorm.y = pPolyPtr->normal[1];
						vecNorm.z = pPolyPtr->normal[2];
					}
					GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvTransposedWorld );
					//GGVec3TransformNormal ( &vecNorm, &vecNorm, &matInvNoTransWorld );
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNX*4) ) ) = vecNorm.x;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNY*4) ) ) = vecNorm.y;
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwNZ*4) ) ) = vecNorm.z;
				}
				if ( iStageForUV==1 )
				{
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[0]*4) ) ) = pPolyPtr->diffuseuv3[0];
					*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[0]*4) ) ) = pPolyPtr->diffuseuv3[1];
				}
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTU[iStageForUV]*4) ) ) = pPolyPtr->fU3;
				*( (float*) ( pVertexData + dwVertexIndex + (cOffsetMap.dwTV[iStageForUV]*4) ) ) = pPolyPtr->fV3;
				dwVertexIndex += dwFVFSize;

				// next poly
				pPolyPtr = pPolyPtr->pNextPoly;
			}
			pPolyGroup = pPolyGroup->pNextGroup;
		}

		// refresh as new vertex data in mesh
		pMesh->bVBRefreshRequired = true;
		RefreshMeshShortList ( pMesh );
	}
	else
	{
		//MessageBox ( NULL, "Cannot write UV data, baked geometry counts do not match", "Lightmapping Error", MB_OK );
	}
}

//
// CObjectsC Functions Implementation
//

//#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4800)
#include "CObjectsC.h"
#include "CGfxC.h"
#include ".\..\Core\SteamCheckForWorkshop.h"
#include "cVectorC.h"
#include "CMemblocks.h"
#include "cOccluderThread.h"
#include <algorithm>
#include "ShadowMapping\cShadowMaps.h"
#include "CFileC.h"

#ifndef DX11
// Occlusion object global
#include "Occlusion\cOcclusion.h"
extern COcclusion g_Occlusion;
#endif
extern CascadedShadowsManager g_CascadedShadow;

// External Globals
extern bool g_bSwitchLegacyOn;
extern bool g_bFastBoundsCalculation;
extern GGHANDLE g_pMainCameraDepthHandle;
extern LPGGEFFECT g_pMainCameraDepthEffect;
extern bool g_bSkipAnyDedicatedDepthRendering;

// Global Lists
std::vector< GGHANDLE > g_EffectParamHandleList;

#ifndef NOSTEAMORVIDEO
void timestampactivity(int i, char* desc_s); // for debug
#endif

// Global Intersect All Helpers
struct OrderByCamDistance
{
    bool operator()(sObject* pObjectA, sObject* pObjectB)
    {
        if (pObjectA->position.fCamDistance < pObjectB->position.fCamDistance)
            return true;
        if (pObjectA->position.fCamDistance == pObjectB->position.fCamDistance)
            return (pObjectA->dwObjectNumber < pObjectB->dwObjectNumber);
        return false;
    }
};
std::vector< sObject* > g_pIntersectShortList;

// Global to store a second range of objects for IntersectAll special mode
int g_iIntersectAllSecondStart = 0;
int g_iIntersectAllSecondEnd = 0;
// Globals for a third rane of objects
int g_iIntersectAllThirdStart = 0;
int g_iIntersectAllThirdEnd = 0;
bool g_bIgnoreCollisionPropertyOnce = false;
//#define SKIPGRIDUSED
#ifdef SKIPGRIDUSED
float g_fIntersectAllSkipGridX = 0;
float g_fIntersectAllSkipGridZ = 0;
DWORD g_dwSkipGrid[1024][1024];
int g_iSkipGridResult[1024][1024];
#endif

// Global can be changed to improve performance (0-allow CPU, 3-do not allow any CPU anim animation)
int g_iDefaultCPUAnimState = 0;

// For DB_ObjectScreenData for more accurate reporting of 'in screen' setting.
namespace
{
    // u74b7 - Generate the frustum planes from the transformation matrix
    void ExtractFrustumPlanes(GGPLANE p_Planes[6], const GGMATRIX & matCamera)
    {
        // Left clipping plane
        p_Planes[0].a = matCamera._14 + matCamera._11;
        p_Planes[0].b = matCamera._24 + matCamera._21;
        p_Planes[0].c = matCamera._34 + matCamera._31;
        p_Planes[0].d = matCamera._44 + matCamera._41;

        // Right clipping plane
        p_Planes[1].a = matCamera._14 - matCamera._11;
        p_Planes[1].b = matCamera._24 - matCamera._21;
        p_Planes[1].c = matCamera._34 - matCamera._31;
        p_Planes[1].d = matCamera._44 - matCamera._41;
        
        // Top clipping plane
        p_Planes[2].a = matCamera._14 - matCamera._12;
        p_Planes[2].b = matCamera._24 - matCamera._22;
        p_Planes[2].c = matCamera._34 - matCamera._32;
        p_Planes[2].d = matCamera._44 - matCamera._42;
        
        // Bottom clipping plane
        p_Planes[3].a = matCamera._14 + matCamera._12;
        p_Planes[3].b = matCamera._24 + matCamera._22;
        p_Planes[3].c = matCamera._34 + matCamera._32;
        p_Planes[3].d = matCamera._44 + matCamera._42;
        
        // Near clipping plane
        p_Planes[4].a = matCamera._13;
        p_Planes[4].b = matCamera._23;
        p_Planes[4].c = matCamera._33;
        p_Planes[4].d = matCamera._43;
        
        // Far clipping plane
        p_Planes[5].a = matCamera._14 - matCamera._13;
        p_Planes[5].b = matCamera._24 - matCamera._23;
        p_Planes[5].c = matCamera._34 - matCamera._33;
        p_Planes[5].d = matCamera._44 - matCamera._43;
        
        // Normalise the planes
        for (int i = 0; i < 6; ++i)
            GGPlaneNormalize(&p_Planes[i], &p_Planes[i]);
    }

    // Calculate the minimum signed distance from the plane to a point
    inline float DistancePlaneToPoint(const GGPLANE & Plane, const GGVECTOR3 & pt)
    {
        return Plane.a*pt.x + Plane.b*pt.y + Plane.c*pt.z + Plane.d;
    }

    bool ContainsSphere(const GGPLANE p_Planes[6], const GGVECTOR3& vecCentre, const float fRadius)
    {
	    // calculate if sphere is on the correct 'side' of each plane
	    for(int i = 0; i < 6; ++i)
        {
            // If sphere on the wrong side, we're done
            if (DistancePlaneToPoint(p_Planes[i], vecCentre) < -fRadius)
                 return false;
	    }

	    // otherwise we are fully in view
	    return true;
    }
    
    /*
    inline bool ContainsPoint(const GGPLANE p_Plane[6], const GGVECTOR3& vecPoint)
    {
        return ContainsSphere(p_Plane, vecPoint, 0.0);
    }
    */

    float ApplyPivot ( sObject* pObject, int iMode, GGVECTOR3 vecValue, float fValue )
    {
	    if ( pObject->position.bApplyPivot )
	    {
		    GGVec3TransformCoord ( &vecValue, &vecValue, &pObject->position.matPivot );

		    if ( iMode == 0 ) return vecValue.x;
		    if ( iMode == 1 ) return vecValue.y;
		    if ( iMode == 2 ) return vecValue.z;
	    }

	    return fValue;
    }

    SDK_FLOAT GetAxisSizeFromVectorOffset ( int iID, int iActualSize, int iVectorOffset )
    {
        // iActualSize is 0 = unscaled, 1 = scaled, 2 = scaled (including originalmatrix scaling)
        // iVectorOffset is 0 = x, 1 = y, 2 = z

	    // Check the object exists
	    if ( !ConfirmObjectInstance ( iID ) )
		    return 0;

        // Get the object pointer
	    sObject* pObject = g_ObjectList [ iID ];

        // If the object is an instance, grab the scaling from the instance (to be applied later)
        // and move on to the object itself
        float fAdjustScale = 1.0;
	    if ( pObject->pInstanceOfObject )
        {
            fAdjustScale = pObject->position.vecScale[ iVectorOffset ];
            pObject = pObject->pInstanceOfObject;
        }

        // Get the precomputed size of the objects x dimension
        float fValue = (pObject->collision.vecMax[ iVectorOffset ] - pObject->collision.vecMin[ iVectorOffset ]);
    	
	    // Apply pivot if needed
	    fValue = ApplyPivot ( pObject, iVectorOffset, GGVECTOR3 ( pObject->collision.vecMax - pObject->collision.vecMin ), fValue );

	    // Ensure size is reported as positive
	    fValue = fabs ( fValue );

	    // Adjusts to scale now
	    if ( iActualSize==1 || iActualSize==2 )
		{
			// basic scale value from object
            fValue = fValue * pObject->position.vecScale[ iVectorOffset ] * fAdjustScale;
		}
	    if ( iActualSize==2 )
		{
			// additionally, apply original matrix scaling
			GGVECTOR3 vecUnit = GGVECTOR3(1,1,1);
			GGVec3TransformCoord ( &vecUnit, &vecUnit, &pObject->ppFrameList[0]->matOriginal );
            fValue = fValue * vecUnit[ iVectorOffset ];
		}

		// return scale value
	    return fValue;
    }
}

void* GetObjectsInternalData ( int iID )
{
	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return NULL;

	// return a pointer to the data
	return (void*)g_ObjectList [ iID ];
}

// Functions moved from other parts so they can be called by g_xxx_funcptrs

DARKSDK_DLL void ConvertToFVF ( sMesh* pMesh, DWORD dwFVF )
{
	//PE: Only if we are actually going to change FVF.
	if (pMesh->dwFVF != dwFVF)
	{
		// when mesh changes FVF, really need to erase old orig data
		// simply because it will attempt to 'copy' when asked to reset, and it will copy the wrong FVF pattern
		SAFE_DELETE_ARRAY(pMesh->pOriginalVertexData);

		// Use main FVF converter function
		ConvertLocalMeshToFVF(pMesh, dwFVF);
	}
}

DARKSDK_DLL void SmoothNormals ( sMesh* pMesh, float fPercentage )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// faster method assumes vertices are shared, so go through faces, collect normals for each vertex in the face
	// then we can average them at the end
	if ( offsetMap.dwZ>0 && offsetMap.dwNZ>0 )
	{
		// index buffer or raw vertice list
		bool bUsingIndices = true;
		DWORD iCount = pMesh->dwIndexCount;
		if ( iCount == 0 ) { iCount = pMesh->dwVertexCount; bUsingIndices = false; }

		// a normal for each vertex in mesh (we will accumilate normals into these slots)
		int* iNormalCount = new int [pMesh->dwVertexCount];
		GGVECTOR3* fNormals = new GGVECTOR3 [pMesh->dwVertexCount];
		for ( DWORD v=0; v<pMesh->dwVertexCount; v++ ) 
		{
			iNormalCount[v] = 0;
			fNormals[v] = GGVECTOR3(0,0,0);
		}

		// go through all polys, collect normals for each face vert
		for ( DWORD i=0; i<iCount; i+=3 )
		{
			// read face
			DWORD dwFace0, dwFace1, dwFace2;
			if ( bUsingIndices == true )
			{
				dwFace0 = pMesh->pIndices[i+0];
				dwFace1 = pMesh->pIndices[i+1];
				dwFace2 = pMesh->pIndices[i+2];
			}
			else
			{
				dwFace0 = i+0;
				dwFace1 = i+1;
				dwFace2 = i+2;
			}

			// get vertex
			GGVECTOR3 vecVert0 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace0 ) );
			GGVECTOR3 vecVert1 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace1 ) );
			GGVECTOR3 vecVert2 = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwFace2 ) );

			// get normal
			//GGVECTOR3* pvecNorm0 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace0 ) );
			//GGVECTOR3* pvecNorm1 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace1 ) );
			//GGVECTOR3* pvecNorm2 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace2 ) );

			// calculate normal from vertices
			GGVECTOR3 vNormal;
			GGVec3Cross ( &vNormal, &( vecVert2 - vecVert1 ), &( vecVert0 - vecVert1 ) );
			GGVec3Normalize ( &vNormal, &vNormal );

			// apply new normal to geometry for all normals associated with the poly
			//*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace0 ) ) = vNormal;
			//*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace1 ) ) = vNormal;
			//*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * dwFace2 ) ) = vNormal;

			// for now average everything, ignore percentage threshold
			fNormals[dwFace0] += vNormal;
			fNormals[dwFace1] += vNormal;
			fNormals[dwFace2] += vNormal;
			iNormalCount[dwFace0] += 1;
			iNormalCount[dwFace1] += 1;
			iNormalCount[dwFace2] += 1;
		}

		// we now have accumilated normals associated with vertices, now average them and write result into normal vector
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)pMesh->dwVertexCount; iCurrentVertex++ )
		{
			// get normal vector for vertex
			GGVECTOR3* pvecOrigNormal = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) );

			// can only average normals that are used by faces
			if ( iNormalCount[iCurrentVertex] > 0 )
			{
				// average normal
				GGVECTOR3 vecAveragedNormal = fNormals[iCurrentVertex] / iNormalCount[iCurrentVertex];
				*pvecOrigNormal = vecAveragedNormal;
			}
		}

		// free usages
		SAFE_DELETE ( iNormalCount );
		SAFE_DELETE ( fNormals );
	}

	/*
	// assume no more than 32 shared vertex points
	DWORD dwSharedVertexMax=32;
	DWORD dwNumberOfVertices=pMesh->dwVertexCount;
	BYTE* NormalCount = new BYTE [ dwNumberOfVertices ];
	ZeroMemory ( NormalCount, dwNumberOfVertices*sizeof(BYTE) );
	GGVECTOR3* fNormals = new GGVECTOR3 [dwNumberOfVertices*dwSharedVertexMax];
	int* pNormalRef = new int [dwNumberOfVertices*dwSharedVertexMax];

	// check if same
	float fEpsilonRange = 0.01f;

	// make sure we have data in the vertices
	if ( offsetMap.dwZ>0 && offsetMap.dwNZ>0 )
	{
		// This is super slow (8-15seconds on regular 18K character)
		// each vertex checks every other vertex 7000*7000 iterations!
		// go through all of the vertices
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)dwNumberOfVertices; iCurrentVertex++ )
		{
			// see how many other vertices share this space
			GGVECTOR3 vecVert = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );
			//float* pQuickScanVertPtr = (float*) pMesh->pVertexData + offsetMap.dwX;
			for ( int iScanVert = 0; iScanVert < (int)dwNumberOfVertices; iScanVert++ )
			{
				// 150416 -V1.131-b1 too slow this one, need an early out
				//if ( *pQuickScanVertPtr != vecVert.x ) continue;
				//pQuickScanVertPtr += offsetMap.dwSize;

				// get vertex position and scan vertex position
				GGVECTOR3 vecScanVert = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iScanVert ) );

				// determine if vectors near each other
				float fDX = fabs ( vecVert.x - vecScanVert.x );
				float fDY = fabs ( vecVert.y - vecScanVert.y );
				float fDZ = fabs ( vecVert.z - vecScanVert.z );
				float fDD = (fDX+fDY+fDZ)/3.0f;

				// if they match, add normal to table
				if ( iCurrentVertex!=iScanVert && fDD <= fEpsilonRange )
				{
					if ( fDD > 0.0f )
					{
						int iVertsCloseButNotLinedUp = 42;
					}

					// get normal from the scanned vertex
					GGVECTOR3 vecScannedNormal = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iScanVert ) );

					// add normal to table
					BYTE Index = NormalCount [ iCurrentVertex ];
					if ( Index < 32 )
					{
						fNormals [ (iCurrentVertex*dwSharedVertexMax)+Index ] = vecScannedNormal;
						pNormalRef [ (iCurrentVertex*dwSharedVertexMax)+Index ] = iScanVert;
						NormalCount [ iCurrentVertex ] = NormalCount [ iCurrentVertex ] + 1;
					}
				}
			}
		}
	}

	// for each vertex, choose new normal that fits the smoothing rule
	for ( int iCurrentVertex = 0; iCurrentVertex < (int)dwNumberOfVertices; iCurrentVertex++ )
	{
		// get position and normal from vertex
		GGVECTOR3 vecOrigPosition = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );
		GGVECTOR3 vecOrigNormal = *(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) );
				
		// for any vertices that share this space, consider their normals
		DWORD dwOtherNormalsCount = NormalCount [ iCurrentVertex ];
		if ( dwOtherNormalsCount > 0 )
		{
			// clear averaging vector
			DWORD dwAverageCount = 1;
			GGVECTOR3 vecAverageNormal = vecOrigNormal;

			// only include normals whos angle is within rule
			for ( DWORD iIndex=0; iIndex<dwOtherNormalsCount; iIndex++ )
			{
				// get other normal
				GGVECTOR3 vecOtherNormal = fNormals [ (iCurrentVertex*dwSharedVertexMax)+iIndex ];

				// calculate difference
				float fDiffX = (float)fabs(vecOrigNormal.x - vecOtherNormal.x);
				float fDiffY = (float)fabs(vecOrigNormal.y - vecOtherNormal.y);
				float fDiffZ = (float)fabs(vecOrigNormal.z - vecOtherNormal.z);

				// if all are within rule, add to averaging normal
				float fDifference = (fDiffX+fDiffY+fDiffZ) / 3.0f;
				if ( fDifference <= fPercentage )
				{
					vecAverageNormal += vecOtherNormal;
					dwAverageCount++;
				}
			}

			// finalse averaging
			vecAverageNormal /= (float)dwAverageCount;

			// apply new normal to data
			*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) ) = vecAverageNormal;

			// also apply to all those that contributed to this average (as we stamp through this list in one direction only)
			for ( DWORD iIndex=0; iIndex<dwOtherNormalsCount; iIndex++ )
			{
				// set all other normals at same time
				int iOtherVertIndex = pNormalRef [ (iCurrentVertex*dwSharedVertexMax)+iIndex ];
				*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iOtherVertIndex ) ) = vecAverageNormal;

				// and also align position to be exactly this one (to help with true edge detection)
				*(GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iOtherVertIndex ) ) = vecOrigPosition;
			}
		}
	}

	// free usages
	SAFE_DELETE ( NormalCount );
	SAFE_DELETE ( fNormals );
	SAFE_DELETE ( pNormalRef );
	*/

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

//////////////////////////////////////////////////////////////////////////////////
// COMMANDS //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK_DLL void RefreshMeshShortList ( sMesh* pMesh )
{
	if ( pMesh==NULL ) return;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void LoadCore ( char* szFilename, char* szOrgFilename, int iID, int iDBProMode, int iDivideTextureSize )
{
	// ensure the object is okay to use
	ConfirmNewObject ( iID );

	// check memory allocation
	ID_ALLOCATION ( iID );

	// load the object
	if ( !LoadDBO ( szFilename, &g_ObjectList [ iID ] , szOrgFilename ) )
		return;

	// setup new object and introduce to buffers
	if ( !SetNewObjectFinalProperties ( iID, -1.0f ) )
		return;


	// add object id to shortlist
	AddObjectToObjectListRef ( iID );

	// calculate path from filename
	char szPath [ MAX_PATH ];
	if ( _strnicmp ( (char*)szFilename+strlen((char*)szFilename)-4, ".mdl", 4 )==NULL )
	{
		// MDL models store their textures in the temp folder
		DBOCalculateLoaderTempFolder();
		strcpy ( szPath, g_WindowsTempDirectory );
	}
	else
	{
		//PE: This is why textures is found, and are doubble loaded.
		//PE: In standalone path is C:\Users\name\AppData\Local\Temp\\dbpdata\ , We need to use the real path.
		
		// Path is current model location
		strcpy( szPath, "" );
		LPSTR pFile = (LPSTR)szOrgFilename;
		DWORD dwLength = strlen(pFile);
		for ( int n=dwLength; n>0; n-- )
		{
			if ( pFile[n]=='\\' || pFile[n]=='/' )
			{
				strcpy ( szPath, pFile );
				szPath[n+1]=0;
				break;
			}
		}	
	}

	// prepare textures for all meshes (load them)
	sObject* pObject = g_ObjectList [ iID ];

	for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++) 
	{
		LoadInternalTextures(pObject, pObject->ppMeshList[iMesh], szPath, iDBProMode, iDivideTextureSize);
	}

	// 140817 - need this for lightmapper objects loading with shaders applied
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// get mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh->bUseVertexShader )
		{
			// Search if effect already loaded (else create a new)
			CreateNewOrSharedEffect ( pMesh, true );
		}
	}
}

#ifndef NOSTEAMORVIDEO
void timestampactivity(int i, char* desc_s); // for debug
#endif

DARKSDK_DLL void LoadObject(LPSTR szFilename, int iID)
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);

	// store current folder (typically mode dir)
	char pStoreCurrentDir[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, pStoreCurrentDir);

	// determine if loading an encrypted model file
	bool bTempFolderChangeForEncrypt = CheckForWorkshopFile(VirtualFilename);

	// get path of original model file passed in
	char pPathToOriginalFile[_MAX_PATH];
	strcpy(pPathToOriginalFile, "");
	if ( strlen(VirtualFilename) > 0 )
	{
		// get relative path from current
		strcpy(pPathToOriginalFile, VirtualFilename);
		for (DWORD n = strlen(pPathToOriginalFile) - 1; n > 0; n--)
		{
			if (pPathToOriginalFile[n] == '\\' || pPathToOriginalFile[n] == '/' || (unsigned char)(pPathToOriginalFile[n]) < 32)
			{
				pPathToOriginalFile[n] = 0;
				break;
			}
		}
	}

	// Decrypt and use media, re-encrypt
	if ( g_pGlob->Decrypt ) g_pGlob->Decrypt(VirtualFilename);

	// Load media
	LoadCore ( VirtualFilename, szFilename, iID, 0, 0 );

	// Re-encrypt
	if ( g_pGlob->Encrypt ) g_pGlob->Encrypt( VirtualFilename );
}

DARKSDK_DLL void LoadObject ( LPSTR szFilename, int iID, int iDBProMode )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	CheckForWorkshopFile (VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	LoadCore ( VirtualFilename, szFilename, iID, iDBProMode, 0 );
	g_pGlob->Encrypt( VirtualFilename );
}

DARKSDK_DLL void LoadObject ( LPSTR szFilename, int iID, int iDBProMode, int iDivideTextureSize )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	CheckForWorkshopFile (VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	LoadCore ( VirtualFilename, szFilename, iID, iDBProMode, iDivideTextureSize );
	g_pGlob->Encrypt( VirtualFilename );
}

DARKSDK_DLL void EnsureObjectDBOIsFVF ( int iID, LPSTR pFileToLoad, DWORD dwRequiredFVF )
{
	// for new DX11, must ensure object FVF inside DBO is a specific type
	// i.e. the projectile/smoke DBOs where 338, but need to be 274 for decal/gui shaders, etc
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	bool bConvertedOneOrMoreMeshes = false;
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMeshIndex=0; iMeshIndex<pObject->iMeshCount; iMeshIndex++ )
	{
		// make a new mesh from the original mesh, and ensure it's verts only
		sMesh* pMesh = pObject->ppMeshList[iMeshIndex];
		if ( pMesh->dwFVF != dwRequiredFVF )
		{
			ConvertToFVF ( pMesh, dwRequiredFVF );
			bConvertedOneOrMoreMeshes = true;
		}
	}

	// and save new DBO if converted any of the meshes
	if ( bConvertedOneOrMoreMeshes == true )
	{
		if ( DoesFileExist ( pFileToLoad ) ) DeleteFile ( pFileToLoad );
		SaveObject ( pFileToLoad, iID );
	}
}

DARKSDK_DLL void SaveObjectEx ( LPSTR szFilename, int iID, bool bCompactOBJ )
{
	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// check ptr valid
	LPSTR pDBPFilename = szFilename;
	if ( pDBPFilename )
	{
		// U78 - if OBJ extension detected, switch to OBJ exporter
		// OBJ is a static format (no transforms, no animation, not much really)
		if ( strnicmp ( pDBPFilename + strlen(pDBPFilename) - 4, ".obj", 4 )==NULL )
		{
			// vertex indices are global to the file
			DWORD dwStartOfVertexBatch = 1;

			// Get just the name
			char pJustObjName[512];
			strcpy ( pJustObjName, pDBPFilename );
			pJustObjName[strlen(pJustObjName)-4]=0;

			// MTL file
			char pMTLFilename[512];
			strcpy ( pMTLFilename, pJustObjName );
			strcat ( pMTLFilename, ".mtl" );

			// open MTL file for writing
			HANDLE hMTLfile = NULL;
			if ( bCompactOBJ == false ) hMTLfile = GG_CreateFile ( pMTLFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if ( bCompactOBJ == true || hMTLfile != INVALID_HANDLE_VALUE )
			{
				// write OBJ format
				LPSTR pLine = 0;
				DWORD byteswritten=0;
				HANDLE hfile = GG_CreateFile ( pDBPFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				if ( hfile != INVALID_HANDLE_VALUE )
				{
					// header
					pLine = "# OBJ Model File converted by the mighty Game Creators\n";
					WriteFile( hfile, pLine, strlen(pLine), &byteswritten, NULL ); 
					pLine = "# more tools found at www.thegamecreators.com\n";
					WriteFile( hfile, pLine, strlen(pLine), &byteswritten, NULL ); 
					pLine = "\n";
					WriteFile( hfile, pLine, strlen(pLine), &byteswritten, NULL ); 

					// material file name
					char pDynLine[512];
					if (bCompactOBJ == false)
					{
						pLine = "# Material library\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
						sprintf (pDynLine, "mtllib %s\n\n", pMTLFilename);
						WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
					}

					// object name
					if (bCompactOBJ == false)
					{
						pLine = "# Object\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
						sprintf (pDynLine, "o %s\n", pJustObjName);
						WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
						pLine = "\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
					}

					// group name
					if (bCompactOBJ == false)
					{
						pLine = "# Mesh\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
						pLine = "g mesh\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
						pLine = "\n";
						WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
					}

					// vertices
					pLine = "# Vertex list\n";
					WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);

					// dump all vertices from all meshes into OBJ
					sMesh** pVertOnlyMeshes = new sMesh*[pObject->iFrameCount];
					for (int iFrameIndex = 0; iFrameIndex < pObject->iFrameCount; iFrameIndex++)
					{
						sFrame* pFrame = pObject->ppFrameList[iFrameIndex];
						if (pFrame)
						{
							sMesh* pMesh = pFrame->pMesh;
							if (pMesh)
							{
								// make a new mesh from the original mesh, and ensure it's verts only
								pVertOnlyMeshes[iFrameIndex] = new sMesh;
								sMesh* pVertOnlyMesh = pVertOnlyMeshes[iFrameIndex];
								MakeMeshFromOtherMesh (true, pVertOnlyMesh, pMesh, NULL);
								ConvertLocalMeshToVertsOnly (pVertOnlyMesh, false);

								// create matrix to transform XYZ
								GGMATRIX matTemp;
								GGMATRIX matThisFrame;
								GGMatrixIdentity(&matThisFrame);
								// scale and rotation done when first added obj(as mesh) into monster object
								// scale
								//GGMatrixScaling(&matTemp, pFrame->vecScale.x, pFrame->vecScale.y, pFrame->vecScale.z);
								//GGMatrixMultiply(&matThisFrame, &matThisFrame, &matTemp);
								// rotation
								//GGMatrixRotationX(&matTemp, GGToRadian (pFrame->vecRotation.x));
								//GGMatrixMultiply(&matThisFrame, &matThisFrame, &matTemp);
								//GGMatrixRotationY(&matTemp, GGToRadian (pFrame->vecRotation.y));
								//GGMatrixMultiply(&matThisFrame, &matThisFrame, &matTemp);
								//GGMatrixRotationZ(&matTemp, GGToRadian (pFrame->vecRotation.z));
								//GGMatrixMultiply(&matThisFrame, &matThisFrame, &matTemp);
								// translation
								GGMatrixTranslation	(&matTemp, pFrame->vecOffset.x, pFrame->vecOffset.y, pFrame->vecOffset.z);
								GGMatrixMultiply(&matThisFrame, &matThisFrame, &matTemp);

								// for each vertex
								BYTE* pVertData = pVertOnlyMesh->pVertexData;
								for (DWORD dwV = 0; dwV < pVertOnlyMesh->dwVertexCount; dwV++)
								{
									GGVECTOR3 vecXYZ = GGVECTOR3((float)*((float*)pVertData + 0), (float)*((float*)pVertData + 1), (float)*((float*)pVertData + 2));
									GGVec3TransformCoord(&vecXYZ, &vecXYZ, &matThisFrame);
									sprintf (pDynLine, "v %f %f %f\n", vecXYZ.x, vecXYZ.y, vecXYZ.z);
									WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									pVertData += pVertOnlyMesh->dwFVFSize;
								}
								if (bCompactOBJ == false)
								{
									pVertData = pVertOnlyMesh->pVertexData;
									for (DWORD dwV = 0; dwV < pVertOnlyMesh->dwVertexCount; dwV++)
									{
										float fReverseVCoordForOBJ = -(float)*((float*)pVertData + 7);
										sprintf (pDynLine, "vt %f %f\n", (float)*((float*)pVertData + 6), fReverseVCoordForOBJ);
										WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
										pVertData += pVertOnlyMesh->dwFVFSize;
									}
									pVertData = pVertOnlyMesh->pVertexData;
									for (DWORD dwV = 0; dwV < pVertOnlyMesh->dwVertexCount; dwV++)
									{
										sprintf (pDynLine, "vn %f %f %f\n", (float)*((float*)pVertData + 3), (float)*((float*)pVertData + 4), (float)*((float*)pVertData + 5));
										WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
										pVertData += pVertOnlyMesh->dwFVFSize;
									}
								}
							}
						}
					}

					// faces
					pLine = "\n";
					WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
					pLine = "# Face list\n";
					WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
					
					// dump all face index data from all meshes into OBJ
					for (int iFrameIndex = 0; iFrameIndex < pObject->iFrameCount; iFrameIndex++)
					{
						sFrame* pFrame = pObject->ppFrameList[iFrameIndex];
						if (pFrame)
						{
							sMesh* pMesh = pFrame->pMesh;
							if (pMesh)
							{
								// verts only meshes created above
								sMesh* pVertOnlyMesh = pVertOnlyMeshes[iFrameIndex];

								// texture filename
								if (bCompactOBJ == false)
								{
									// no guarentee its in the X file - but we try first
									// else we use the name of the model file
									char pFileOnlyNoExt[512];
									for (int iTry = 0; iTry < 2; iTry++)
									{
										LPSTR pOrigPathAndFile = NULL;
										if (iTry == 0) pOrigPathAndFile = pMesh->pTextures[0].pName;
										if (iTry == 1) pOrigPathAndFile = pDBPFilename;
										strcpy (pFileOnlyNoExt, "");
										if (pOrigPathAndFile)
										{
											// trim off any paths first
											strcpy (pFileOnlyNoExt, pOrigPathAndFile);
											for (int n = strlen(pOrigPathAndFile) - 1; n > 0; n--)
											{
												if (pOrigPathAndFile[n] == '\\' || pOrigPathAndFile[n] == '/')
												{
													strcpy (pFileOnlyNoExt, pOrigPathAndFile + n + 1);
													n = 0; break;
												}
											}

											// now we see if any variations of this filename exists
											if (strlen(pFileOnlyNoExt) > 4)
											{
												pFileOnlyNoExt[strlen(pFileOnlyNoExt) - 4] = 0;
												strcat (pFileOnlyNoExt, ".png");
												HANDLE hExists = GG_CreateFile(pFileOnlyNoExt, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
												if (hExists != INVALID_HANDLE_VALUE)
												{
													// this texture file exists - we have our texture name
													CloseHandle(hExists);
													iTry = 99;
												}
												else
												{
													// the PNG of the named texture does not exist, but sometimes
													// texture names are truncated (chair_a.x) so need to be sliced
													// up to find which part of the end is the actual texture (up to 32 chars)
													char pSlicedVariant[512];
													for (int n = 32; n > 5; n--)
													{
														strcpy (pSlicedVariant, pFileOnlyNoExt + strlen(pFileOnlyNoExt) - n);
														hExists = GG_CreateFile(pSlicedVariant, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
														if (hExists != INVALID_HANDLE_VALUE)
														{
															// this texture file exists - we have our texture name
															strcpy (pFileOnlyNoExt, pSlicedVariant);
															CloseHandle(hExists);
															iTry = 99;
															n = 0;
														}
													}
												}
											}
										}
									}

									// remove spaces from material record
									char pNoSpacesFile[512];
									strcpy (pNoSpacesFile, pFileOnlyNoExt);
									for (DWORD n = 0; n < strlen(pNoSpacesFile); n++)
										if (pNoSpacesFile[n] == ' ') pNoSpacesFile[n] = '_';

									// write material(texture) for this collecion of faces(mesh)
									sprintf (pDynLine, "usemtl %s\n", pNoSpacesFile);
									WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);

									// also write this into the MTL file
									sprintf (pDynLine, "newmtl %s\n", pNoSpacesFile);
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    Ns 20\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    d 1\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    illum 2\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    Kd 1.0 1.0 1.0\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    Ks 0 0 0\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    Ka 0 0 0\n");
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									sprintf (pDynLine, "    map_Kd %s\n\n", pNoSpacesFile);
									WriteFile(hMTLfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);

									// for each face
									for (DWORD dwV = 0; dwV < pVertOnlyMesh->dwVertexCount; dwV += 3)
									{
										int iA = dwV + dwStartOfVertexBatch + 0;
										int iB = dwV + dwStartOfVertexBatch + 1;
										int iC = dwV + dwStartOfVertexBatch + 2;
										sprintf (pDynLine, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", iA, iA, iA, iB, iB, iB, iC, iC, iC);
										WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
									}
								}
								else
								{
									// for each face - simple face index only
									int iLimit = 9999999;
									for (DWORD dwV = 0; dwV < pVertOnlyMesh->dwVertexCount; dwV += 3)
									{
										int iA = dwV + dwStartOfVertexBatch + 0;
										int iB = dwV + dwStartOfVertexBatch + 1;
										int iC = dwV + dwStartOfVertexBatch + 2;
										if (iA < iLimit && iB < iLimit && iC < iLimit)
										{
											sprintf (pDynLine, "f %d %d %d\n", iA, iB, iC);
											WriteFile(hfile, pDynLine, strlen(pDynLine), &byteswritten, NULL);
										}
									}
								}

								// Advance global start marker for vertice indices
								dwStartOfVertexBatch += pVertOnlyMesh->dwVertexCount;

								// free temp mesh
								SAFE_DELETE (pVertOnlyMesh);
							}
						}
					}

					// End of file marker
					pLine = "\n";
					WriteFile(hfile, pLine, strlen(pLine), &byteswritten, NULL);
					pLine = "# End of file\n";
					WriteFile( hfile, pLine, strlen(pLine), &byteswritten, NULL ); 
					
					// finish file
					CloseHandle ( hfile );
				}

				// finish file
				CloseHandle ( hMTLfile );
			}
		}
		else
		{
			// ensure filename uses DBO extension
			if ( strnicmp ( pDBPFilename + strlen(pDBPFilename) - 4, ".dbo", 4 )!=NULL )
			{
				RunTimeError ( RUNTIMEERROR_B3DMUSTUSEDBOEXTENSION );
				return;
			}

			// save the object as DBO
			if ( !SaveDBO ( pDBPFilename, pObject ) )
				return;
		}
	}
}

DARKSDK_DLL void SaveObject (LPSTR szFilename, int iID)
{
	SaveObjectEx (szFilename, iID, false);
}

DARKSDK_DLL void SetDeleteCallBack ( int iID, ON_OBJECT_DELETE_CALLBACK pfn, int userData )
{
	// mike - 050803 - delete object override

	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return;

	if ( g_ObjectList [ iID ]->iDeleteCount == 0 )
	{
		g_ObjectList [ iID ]->iDeleteCount += 25;
		g_ObjectList [ iID ]->pDelete       = new sObject::sDelete [ g_ObjectList [ iID ]->iDeleteCount ];
	}
	
	if ( g_ObjectList [ iID ]->iDeleteID < g_ObjectList [ iID ]->iDeleteCount )
	{
		g_ObjectList [ iID ]->pDelete [ g_ObjectList [ iID ]->iDeleteID ].onDelete = pfn;
		g_ObjectList [ iID ]->pDelete [ g_ObjectList [ iID ]->iDeleteID ].userData = userData;	

		g_ObjectList [ iID ]->iDeleteID++;
	}

	//g_ObjectList [ iID ]->onDelete = pfn;
	//g_ObjectList [ iID ]->userData = userData;
}

DARKSDK_DLL void SetDisableTransform ( int iID, bool bTransform )
{
	// mike - 050803 - can stop DB Pro transforming an object

	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return;

	g_ObjectList [ iID ]->bDisableTransform = bTransform;
}

DARKSDK_DLL void CreateMeshForObject ( int iID, DWORD dwFVF, DWORD dwVertexCount, DWORD dwIndexCount )
{
	// mike - 050803 - create a new mesh for an object

	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return;

	sObject* pObject = g_ObjectList [ iID ];

	if ( pObject->pFrame )
		SAFE_DELETE ( pObject->pFrame );
	
	pObject->pFrame = new sFrame;

	if ( !pObject->pFrame )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}

	pObject->pFrame->pMesh = new sMesh;

	if ( !pObject->pFrame->pMesh )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}
	
	if ( !SetupMeshFVFData ( pObject->pFrame->pMesh, dwFVF, dwVertexCount, dwIndexCount, false ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}

	pObject->pFrame->pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pObject->pFrame->pMesh->iDrawVertexCount = pObject->pFrame->pMesh->dwVertexCount;
	pObject->pFrame->pMesh->iDrawPrimitives  = pObject->pFrame->pMesh->dwIndexCount / 3;

	SetNewObjectFinalProperties ( iID, 100 );

	// setup new object and introduce to buffers
	//SetNewObjectFinalProperties ( iID, (100.0f)/2 );

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void DeleteObject ( int iID )
{
	// mike - 101005 - excluded objects could not previously by delete
	if ( !CheckObjectExist ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];
	bool bObjectusedUniqueNotSharedBuffers = pObject->bUsesItsOwnBuffers;

	// ensure delete calls ondelete code
	for ( int i = 0; i < g_ObjectList [ iID ]->iDeleteID; i++ )
	{
		if ( pObject->pDelete [ i ].onDelete )
		{
			pObject->pDelete [ i ].onDelete ( iID, pObject->pDelete [ i ].userData );
		}
	}

	// delete object
	DeleteObjectSpecial ( iID );

	// leespeed - 140307 - if the object only used unique VBIB buffers, it did not share any VB IB
	// which means there will be nothing to add back in, so we can skip this step
	if ( bObjectusedUniqueNotSharedBuffers==false )
	{
		m_ObjectManager.AddFlaggedObjectsBackToBuffers ();
	}

	// update
	m_ObjectManager.UpdateTextures();
}

DARKSDK_DLL void DeleteObjects ( int iFrom, int iTo )
{
	// some simple checks
	if ( iTo==0 || iFrom==0 ) return;
	if ( iTo<iFrom ) return;

	// delete multiple objects
	for ( int iID=iFrom; iID<=iTo; iID++ )
	{
		// ensure it exists first
		if ( !CheckObjectExist ( iID ) )
			continue;

		// get object ptr
		sObject* pObject = g_ObjectList [ iID ];
		if ( pObject )
		{
			// ensure delete calls ondelete code
			for ( int i = 0; i < g_ObjectList [ iID ]->iDeleteID; i++ )
			{
				if ( pObject->pDelete [ i ].onDelete )
				{
					pObject->pDelete [ i ].onDelete ( iID, pObject->pDelete [ i ].userData );
				}
			}

			// delete object
			DeleteObjectSpecial ( iID );
		}
	}

	// upon buffer removal, some object where flagged for re-creation
	m_ObjectManager.AddFlaggedObjectsBackToBuffers ();

	// update texture list when introduce new object(s)
	m_ObjectManager.UpdateTextures();
}

DARKSDK_DLL void ClearObjectsOfTextureRef ( LPGGTEXTURE pTextureRef )
{
	// go through all objects being managed and remove texture ref
	m_ObjectManager.RemoveTextureRefFromAllObjects ( pTextureRef );
}

DARKSDK_DLL void SetObject ( int iID, SDK_BOOL bWireframe, int iTransparency, SDK_BOOL bCull )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==FALSE		);
		SetTransparency	( pObject->ppMeshList [ iMesh ], bTransparency==TRUE	);
		SetCull			( pObject->ppMeshList [ iMesh ], bCull==TRUE			);
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );
}

DARKSDK_DLL void SetObject ( int iID, SDK_BOOL bWireframe, int iTransparency, SDK_BOOL bCull, int iFilter )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		//SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==TRUE		);
		// mike - 011005 - state must be false
		SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==FALSE		);
		SetTransparency	( pObject->ppMeshList [ iMesh ], bTransparency==TRUE	);
		SetCull			( pObject->ppMeshList [ iMesh ], bCull==TRUE			);
		SetFilter		( pObject->ppMeshList [ iMesh ], iFilter				);
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );
}

DARKSDK_DLL void SetObject ( int iID, SDK_BOOL bWireframe, int iTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==FALSE		);
		SetTransparency	( pObject->ppMeshList [ iMesh ], bTransparency==TRUE	);
		SetCull			( pObject->ppMeshList [ iMesh ], bCull==TRUE			);
		SetFilter		( pObject->ppMeshList [ iMesh ], iFilter				);
		SetLight		( pObject->ppMeshList [ iMesh ], bLight==TRUE			);
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );
}

DARKSDK_DLL void SetObject ( int iID, SDK_BOOL bWireframe, int iTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight, SDK_BOOL bFog )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==FALSE		);
		SetTransparency	( pObject->ppMeshList [ iMesh ], bTransparency==TRUE	);
		SetCull			( pObject->ppMeshList [ iMesh ], bCull==TRUE			);
		SetFilter		( pObject->ppMeshList [ iMesh ], iFilter				);
		SetLight		( pObject->ppMeshList [ iMesh ], bLight==TRUE			);
		SetFog			( pObject->ppMeshList [ iMesh ], bFog==TRUE				);
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );
}

DARKSDK_DLL void SetObject ( int iID, SDK_BOOL bWireframe, int iTransparency, SDK_BOOL bCull, int iFilter, SDK_BOOL bLight, SDK_BOOL bFog, SDK_BOOL bAmbient )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;
	if ( iTransparency==7 ) bTransparency=FALSE;  // U75 - 051209 - Deal with early-rendered objects correctly

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetWireframe	( pObject->ppMeshList [ iMesh ], bWireframe==FALSE		);
		SetTransparency	( pObject->ppMeshList [ iMesh ], bTransparency==TRUE	);
		SetCull			( pObject->ppMeshList [ iMesh ], bCull==TRUE			);
		SetFilter		( pObject->ppMeshList [ iMesh ], iFilter				);
		SetLight		( pObject->ppMeshList [ iMesh ], bLight==TRUE			);
		SetFog			( pObject->ppMeshList [ iMesh ], bFog==TRUE				);
		SetAmbient		( pObject->ppMeshList [ iMesh ], bAmbient==TRUE			);
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );
}

DARKSDK_DLL void SetObjectWireframe ( int iID, SDK_BOOL bWireframe )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetWireframe ( pObject->ppMeshList [ iMesh ], bWireframe==TRUE );
}

DARKSDK_DLL void SetObjectTransparency ( int iID, int iTransparency )
{
	// Transparency Modes
	// 0 - first-phase no alpha
	// 1 - first-phase with alpha masking
	// 2 and 3 - second-phase which overlaps solid geometry
	// 4 - alpha test (only render beyond 0x000000CF alpha values)
	// 5 - water line object (seperates depth sort automatically)
	// 6 - combination of 3 and 4 (second phase render with alpha blend AND alpha test, used for fading LOD leaves)
	// 7 - very early draw phase no alpha
	// 8 - below water line , render before water.

	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// new transparency mode
	SDK_BOOL bTransparency=TRUE;
	if ( iTransparency==0 ) bTransparency=FALSE;
	if ( iTransparency==7 ) bTransparency=FALSE;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetTransparency ( pObject->ppMeshList [ iMesh ], bTransparency==TRUE );
		SetAlphaTest ( pObject->ppMeshList [ iMesh ], 0x0 ); 
		if ( iTransparency==4 || iTransparency==6 || iTransparency == 8 )
		{
			SetAlphaTest ( pObject->ppMeshList [ iMesh ], 0x000000CF );
		}
	}

	// apply transparency as object overlay
	SetObjectTransparency ( pObject, iTransparency );

}

DARKSDK_DLL void SetObjectCull ( int iID, SDK_BOOL bCull )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// lee - 040306 - u6rc5 - solve CW/CCW issue with some model imports
	int iCullMode = bCull;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetCullCWCCW ( pObject->ppMeshList [ iMesh ], iCullMode );


}

DARKSDK_DLL void SetLimbCull ( int iID, int iLimbIndex, SDK_BOOL bCull )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbIndex ) )
		return;

	// solve CW/CCW issue with some model imports
	int iCullMode = bCull;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList[iLimbIndex];
	if ( pFrame ) 
		if ( pFrame->pMesh )
			SetCullCWCCW ( pFrame->pMesh, iCullMode );


}

DARKSDK_DLL void SetObjectFilterStage ( int iID, int iStage, int iFilter )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetFilter ( pObject->ppMeshList [ iMesh ], iStage, iFilter );
}

DARKSDK_DLL void SetObjectFilter ( int iID, int iFilter )
{
	// iFilter
	// D3DTEXF_POINT = 1 (use 0)
	// GGTEXF_LINEAR = 2 (use 1)
	// D3DTEXF_ANISOTROPIC = 3 (use 2)
	// D3DTEXF_PYRAMIDALQUAD = 6 (use 5)
	// D3DTEXF_GAUSSIANQUAD = 7 (use 6)
	// When assigned, MeshCPP (iFilter++)
	SetObjectFilterStage ( iID, 0, iFilter );
}

DARKSDK_DLL void SetObjectLight ( int iID, SDK_BOOL bLight )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetLight ( pObject->ppMeshList [ iMesh ], bLight==TRUE );
}

DARKSDK_DLL void SetObjectFog ( int iID, SDK_BOOL bFog )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetFog ( pObject->ppMeshList [ iMesh ], bFog==TRUE );
}

DARKSDK_DLL void SetObjectAmbient ( int iID, SDK_BOOL bAmbient )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetAmbient ( pObject->ppMeshList [ iMesh ], bAmbient==TRUE );
}

DARKSDK_DLL void SetObjectRenderMatrixMode ( int iID, int iRenderMatrixMode )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting for Render Matrix Mode
	sObject* pObject = g_ObjectList [ iID ];
	if ( iRenderMatrixMode==1 )
		pObject->dwApplyOriginalScaling = 1;
	else
		pObject->dwApplyOriginalScaling = 0;
}

DARKSDK_DLL void SetObjectMask ( int iID, int iMASK )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	pObject->dwCameraMaskBits = (DWORD)iMASK; // u63 - 0-30 camera bits in mask
}

DARKSDK_DLL void AddObjectMask ( int iID, DWORD dwAddMASK )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	pObject->dwCameraMaskBits |= dwAddMASK;
}

/*
DARKSDK_DLL int RgbR ( DWORD iRGB )
{
	return (int)((iRGB & 0x00FF0000) >> 16);
}

DARKSDK_DLL int RgbG ( DWORD iRGB )
{
	return (int)((iRGB & 0x0000FF00) >> 8);
}

DARKSDK_DLL int RgbB ( DWORD iRGB )
{
	return (int)((iRGB & 0x000000FF) );
}
*/

DARKSDK_DLL void ColorObject ( int iID, DWORD dwRGB )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		SetDiffuse ( pObject->ppMeshList [ iMesh ], 1.0f );
		SetBaseColor ( pObject->ppMeshList [ iMesh ], dwRGB );
	}

	// trigger a re-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectDiffuseEx ( int iID, DWORD dwRGB, int iMaterialOrVertexData )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// lee - 240206 - u60 - if object is instance, use custom-data-slot to store diffuse colour
	sObject* pActualObject = pObject->pInstanceOfObject;
	if ( pActualObject )
	{
		// only if have at least one mesh
		if ( pActualObject->iMeshCount>0 )
		{
			// mesh ptr
			sMesh* pMesh = pActualObject->ppMeshList [ 0 ];

			// this object is an instance
			SetObjectStatisticsInteger(iID,0,dwRGB);
			if ( pObject->dwCustomSize==0 )
			{
				// create custom slot
				DWORD dwStatisticsDataSize = 8;
				pObject->dwCustomSize = dwStatisticsDataSize*-1;
				pObject->pCustomData = (LPVOID)new DWORD[dwStatisticsDataSize];
				for ( DWORD i=0; i<dwStatisticsDataSize; i++ )
					*(((DWORD*)pObject->pCustomData)+i) = 0;
			}
			if ( pObject->dwCustomSize>4000000000 )
			{
				// set diffuse colour
				*(((DWORD*)pObject->pCustomData)+0) = dwRGB;
			}
		}
	}
	else
	{
		// apply setting to all meshes
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			// lee - 240206 - u60 - do both to ensure diffuse is written (SetDiffuseEx added)
			if ( iMaterialOrVertexData==1 )
				SetDiffuseEx ( pObject->ppMeshList [ iMesh ], dwRGB );
			else
				SetDiffuseMaterial ( pObject->ppMeshList [ iMesh ], dwRGB );
		}
	}
}

DARKSDK_DLL void SetObjectDiffuse ( int iID, DWORD dwRGB )
{
	// see above
	SetObjectDiffuseEx ( iID, dwRGB, 0 );
}

DARKSDK_DLL void SetObjectAmbience ( int iID, DWORD dwRGB )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetAmbienceMaterial ( pObject->ppMeshList [ iMesh ], dwRGB );
}

DARKSDK_DLL void SetObjectSpecular ( int iID, DWORD dwRGB )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetSpecularMaterial ( pObject->ppMeshList [ iMesh ], dwRGB );
}

DARKSDK_DLL void SetObjectSpecularPower ( int iID, float fPower )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetSpecularPower ( pObject->ppMeshList [ iMesh ], fPower );
}

DARKSDK_DLL void SetObjectScrollScaleUV ( int iID, float fScrU, float fScrV, float fScaU, float fScaV )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh )
		{
			pMesh->fScrollOffsetU = fScrU;
			pMesh->fScrollOffsetV = fScrV;
			pMesh->fScaleOffsetU = fScaU;
			pMesh->fScaleOffsetV = fScaV;
		}
	}
}

DARKSDK_DLL void SetObjectArtFlags ( int iID, DWORD dwArtFlags, float fBoostIntensity, float fParallaxStrength)
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// apply setting to all meshes (or parent if just instance)
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceOfObject ) pObject = pObject->pInstanceOfObject;
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh )
		{
			pMesh->dwArtFlags = dwArtFlags;
			pMesh->fBoostIntensity = fBoostIntensity;
			pMesh->fParallaxStrength = fParallaxStrength;
		}
	}
}

DARKSDK_DLL void SetObjectSpecular ( int iID, DWORD dwRGB, float fPower )
{
	// U73 - 230309 - helper extra function parameter
	SetObjectSpecular ( iID, dwRGB );
	SetObjectSpecularPower ( iID, fPower );
}

DARKSDK_DLL void SetObjectEmissive ( int iID, DWORD dwRGB )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetEmissiveMaterial ( pObject->ppMeshList [ iMesh ], dwRGB );
}

DARKSDK_DLL void SetObjectArbitaryValue ( int iID, DWORD dwArbValue )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		pObject->ppMeshList [ iMesh ]->Collision.dwArbitaryValue = dwArbValue;
}

DARKSDK_DLL void MakeObject ( int iID, int iMeshID, int iImageID )
{
	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// attempt to create a new object
	if ( !CreateNewObject ( iID, "mesh" ) )
		return;

	// no transform of new limb
	GGMATRIX matWorld;
	GGMatrixIdentity ( &matWorld );

	// setup general object data
	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;
	MakeMeshFromOtherMesh ( true, pMesh, g_RawMeshList [ iMeshID ], &matWorld );

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, -1.0f );

	// give the object a texture (optional)
	if ( iImageID==-1 )
	{
		pMesh->dwTextureCount = g_RawMeshList [ iMeshID ]->dwTextureCount; 
		pMesh->pTextures = new sTexture [ pMesh->dwTextureCount ]; 
		CloneInternalTextures ( pMesh, g_RawMeshList [ iMeshID ] );
	}
	else
		TextureObject ( iID, iImageID );
}

// Recursive Support Function (move to DBO when solid)
sFrame* MakeObjectFromLimbRec ( sFrame* pCurrentFrameToCopy, sFrame* pDstParentFrame )
{
	sFrame* pDstFrameRoot = NULL;
	sFrame* pDstFrame = NULL;
	while ( pCurrentFrameToCopy )
	{
		// frame hierarchy being created
		sFrame* pThisDstFrame = new sFrame;

		// first frame is root frame
		if ( pDstFrameRoot==NULL )
		{
			// first frame of list holds parent ref
			pDstFrameRoot = pThisDstFrame;
			pDstFrameRoot->pParent = pDstParentFrame;
		}

		// last dst frame is sybling to this new one
		if ( pDstFrame ) pDstFrame->pSibling = pThisDstFrame;

		// new dst current frame for copy
		pDstFrame = pThisDstFrame;

		// copy src frame data to dst frame data
		memcpy ( pDstFrame, pCurrentFrameToCopy, sizeof(sFrame) );
		pDstFrame->pChild = NULL;
		pDstFrame->pSibling = NULL;
		pDstFrame->pMesh = NULL;

		// go into children of this frame
		pDstFrame->pChild = MakeObjectFromLimbRec ( pCurrentFrameToCopy->pChild, pDstFrame );

		// next sybling
		pCurrentFrameToCopy = pCurrentFrameToCopy->pSibling;
	}

	// return created and filled frame
	return pDstFrameRoot;
}

DARKSDK_DLL void MakeObjectFromLimbEx ( int iNewID, int iSrcID, int iLimbID, int iCopyAllFromLimb )
{
	// check the mesh exists
	if ( !ConfirmObjectAndLimbInstance ( iSrcID, iLimbID ) )
		return;

	// attempt to create a new object
	if ( !CreateNewObject ( iNewID, "limbmesh" ) )
		return;

	// make object from mesh contained within object
	sObject* pNewObject = g_ObjectList [ iNewID ];
	sObject* pSrcObject = g_ObjectList [ iSrcID ];
	sObject* pActualSrcObject = pSrcObject;
	if ( pSrcObject->pInstanceOfObject ) pActualSrcObject = pSrcObject->pInstanceOfObject;
	sFrame* pSrcFrame = pActualSrcObject->ppFrameList [ iLimbID ];

	// leefix - 181105 - reset original before copy
	ResetVertexDataInMesh ( pSrcObject );

	// leefix - 181105 - can copy limbs even if no mesh there (copy hierarchy and
	// lee - 030406 - u6rc5 - added new command to make one-limb vs all-limb-hierarchy legacy U59 compat.
	if ( iCopyAllFromLimb==1 )
	{
		// Delete frame and mesh from new-object (start from scratch)
		SAFE_DELETE ( pNewObject->pFrame->pMesh );
		SAFE_DELETE ( pNewObject->pFrame );

		// trace through pFrame (limb) selected and copy all hierarchy from it
		sFrame* pDstFrameRoot = NULL;
		pDstFrameRoot = MakeObjectFromLimbRec ( pSrcFrame, NULL );

		// copy frame reference to new object
		pNewObject->pFrame = pDstFrameRoot;

		// force obj to make framelist
		CreateFrameAndMeshList ( pNewObject );

		// copy meshes of src object to dst object
		DWORD dwMeshCount = pActualSrcObject->iMeshCount;
		pNewObject->iMeshCount = dwMeshCount;
		if ( dwMeshCount>0 )
		{
			pNewObject->ppMeshList = new sMesh* [ dwMeshCount ];
			for ( int m=0; m<(int)dwMeshCount; m++ )
			{
				sMesh* pSrcMesh = pActualSrcObject->ppMeshList [ m ];
				sMesh* pDestMesh = new sMesh;

				// root matrix
				GGMATRIX matWorld;
				GGMatrixIdentity ( &matWorld );

				// mesh copy
				MakeMeshFromOtherMesh ( true, pDestMesh, pSrcMesh, &matWorld );
				pNewObject->ppMeshList [ m ] = pDestMesh;

				// bone data copy
				DWORD dwBoneCount = pSrcMesh->dwBoneCount;
				if ( dwBoneCount>0 )
				{
					DWORD dwNewBone = 0;
					pDestMesh->pBones = new sBone [ dwBoneCount ];
					for ( DWORD b=0; b<dwBoneCount; b++ )
					{
						bool bNeedThisBone = false;
						for ( DWORD l=0; l<(DWORD)pNewObject->iFrameCount; l++ )
							if ( strcmp ( pSrcMesh->pBones [ b ].szName, pNewObject->ppFrameList [ l ]->szName )==NULL )
								{ bNeedThisBone = true; break; }

						if ( bNeedThisBone==true )
						{
							memcpy ( &pDestMesh->pBones [ dwNewBone ], &pSrcMesh->pBones [ b ], sizeof ( sBone ) );
							DWORD dwNumInfluences = pDestMesh->pBones [ dwNewBone ].dwNumInfluences;
							pDestMesh->pBones [ dwNewBone ].pVertices = new DWORD [ dwNumInfluences ];
							pDestMesh->pBones [ dwNewBone ].pWeights = new float [ dwNumInfluences ];
							memcpy ( pDestMesh->pBones [ dwNewBone ].pVertices, pSrcMesh->pBones [ b ].pVertices, sizeof(DWORD)*dwNumInfluences );
							memcpy ( pDestMesh->pBones [ dwNewBone ].pWeights, pSrcMesh->pBones [ b ].pWeights, sizeof(float)*dwNumInfluences );
							dwNewBone=dwNewBone+1;
						}
					}
					pDestMesh->dwBoneCount = dwNewBone;
				}
			}
		}

		// only one main mesh exists for bone animation, so we assign it here
		if ( pNewObject->pFrame && pNewObject->ppMeshList )
		{
			// lee - 270306 - u6b5 - first look for mesh from original frame, if exist
			int iMeshBest = -1;
			sMesh* pMeshUsedByOldFrame = pSrcFrame->pMesh;
			for ( int iM=0; iM<(int)pSrcObject->iMeshCount; iM++ )
				if ( pSrcObject->ppMeshList [ iM ]==pMeshUsedByOldFrame )
					iMeshBest = iM;

			// lee - 270306 - u6b5 - pre-U6b5 code..
			if ( iMeshBest==-1 )
			{
				// make SURE the main bone mesh stolen is the BIGGEST
				DWORD dwVertCountLargest = 0;
				for ( int iM=0; iM<(int)dwMeshCount; iM++ )
				{
					if ( pNewObject->ppMeshList [iM ]->dwVertexCount > dwVertCountLargest )
					{
						dwVertCountLargest = pNewObject->ppMeshList [iM ]->dwVertexCount;
						iMeshBest = iM;
					}
				}
			}

			// assign chosen mesh to frame now
			pNewObject->pFrame->pMesh = pNewObject->ppMeshList [ iMeshBest ];
		}

		// copy all animation keyframe data for frames we have taken (if any)
		sAnimationSet* pOrigAnimSet = pSrcObject->pAnimationSet;
		if ( pOrigAnimSet )
		{
			sAnimation* pOrigAnim = pOrigAnimSet->pAnimation;
			if ( pOrigAnim )
			{
				pNewObject->pAnimationSet = new sAnimationSet;
				memcpy ( pNewObject->pAnimationSet, pOrigAnimSet, sizeof ( sAnimationSet ) );
				pNewObject->pAnimationSet->pAnimation = NULL;
				pNewObject->pAnimationSet->pvecBoundCenter = NULL;
				pNewObject->pAnimationSet->pfBoundRadius = NULL;
				pNewObject->pAnimationSet->pvecBoundMax = NULL;
				pNewObject->pAnimationSet->pvecBoundMin = NULL;
				sAnimation* pWorkAnim = NULL;
				while ( pOrigAnim != NULL )
				{
					// if animation name same as any frame in new object, copy anim from src obj to new obj
					bool bNeedThisAnim = false;
					for ( DWORD l=0; l<(DWORD)pNewObject->iFrameCount; l++ )
						if ( strcmp ( pOrigAnim->szName, pNewObject->ppFrameList [ l ]->szName )==NULL )
							{ bNeedThisAnim = true; break; }

					// add this one?
					if ( bNeedThisAnim==true )
					{
						// new animation 
						sAnimation* pCurrentAnim = new sAnimation;		

						// link new anim with previous item
						if ( pWorkAnim ) pWorkAnim->pNext = pCurrentAnim;

						// assign first animation 
						if ( pNewObject->pAnimationSet->pAnimation==NULL )
							pNewObject->pAnimationSet->pAnimation = pCurrentAnim;

						// work animation
						pWorkAnim = pCurrentAnim;

						// fill with name
						memcpy ( pWorkAnim->szName, pOrigAnim->szName, sizeof(pWorkAnim->szName) );

						// create animation and copy orig data
						pWorkAnim->dwNumMatrixKeys = pOrigAnim->dwNumMatrixKeys;
						pWorkAnim->pMatrixKeys = new sMatrixKey [ pWorkAnim->dwNumMatrixKeys ];
						memcpy ( pWorkAnim->pMatrixKeys, pOrigAnim->pMatrixKeys, sizeof ( sMatrixKey )*pWorkAnim->dwNumMatrixKeys );
					}

					// move to the next sequence
					pOrigAnim = pOrigAnim->pNext;
				}
			}
		}

		// setup new object and introduce to buffers
		pNewObject->iMeshCount *= -1; // negated within function CreateFrameAndMeshList
		pNewObject->iFrameCount *= -1; // negated within function CreateFrameAndMeshList
		SetNewObjectFinalProperties ( iNewID, -1.0f );

		// give the object a default texture
		TextureObject ( iNewID, 0 );

		// leeadd - 181105 - many meshes are possible
		for ( int m=0; m<(int)dwMeshCount; m++ )
			CloneInternalTextures ( pNewObject->ppMeshList [ m ], pActualSrcObject->ppMeshList [ m ] );
	}
	else
	{
		// src and destination
		sMesh* pSrcMesh = pActualSrcObject->ppMeshList [ 0 ];
		if ( pSrcFrame ) if ( pSrcFrame->pMesh ) pSrcMesh = pSrcFrame->pMesh;
		sFrame* pDestFrame = g_ObjectList [ iNewID ]->pFrame;
		sMesh* pDestMesh = pDestFrame->pMesh;

		// just copy one limb mesh from object
		if ( pSrcMesh==NULL )
		{
			// failed
			RunTimeError ( RUNTIMEERROR_LIMBNOTEXIST );
			return;
		}

		// work out the world transform to apply to the captured mesh
		CalculateObjectWorld ( pSrcObject, NULL );
		GGMATRIX matWorld = pSrcFrame->matCombined * pSrcObject->position.matObjectNoTran;

		// create new mesh from existing mesh
		MakeMeshFromOtherMesh ( true, pDestMesh, pSrcMesh, &matWorld );

		// setup new object and introduce to buffers
		SetNewObjectFinalProperties ( iNewID, -1.0f );

		// give the object a default texture
		TextureObject ( iNewID, 0 );

		// leeadd - 240604 - u54 - copy texture from original limb to new object
		if ( pDestMesh && pSrcMesh )
			CloneInternalTextures ( pDestMesh, pSrcMesh );
	}
}

DARKSDK_DLL void MakeObjectFromLimb ( int iNewID, int iSrcID, int iLimbID )
{
	// see above
	MakeObjectFromLimbEx ( iNewID, iSrcID, iLimbID, 0 );
}

void SetObjectLOD ( int iCurrentID, int iLODLevel, float fDistanceOfLOD )
{
	// if object exists
	if ( !ConfirmObjectInstance ( iCurrentID ) )
		return;

	// identify real object (if instance)
	sObject* pObject = g_ObjectList [ iCurrentID ];
	sObject* pRealObject = pObject;
	if ( pObject->pInstanceOfObject ) pRealObject = pObject->pInstanceOfObject;

	// initially scan object limb names for LOD_0 LOD_1 LOD_2
	int iObjLOD0LimbIndex = -1;
	int iObjLOD1LimbIndex = -1;
	int iObjLOD2LimbIndex = -1;
	PerformCheckListForLimbs(pRealObject->dwObjectNumber);
	for(int c=0; c<g_pGlob->checklistqty; c++)
	{
		// standard LOD markers
		if ( strcmp ( g_pGlob->checklist[c].string, "LOD_0" )==NULL ) iObjLOD0LimbIndex = c;
		if ( strcmp ( g_pGlob->checklist[c].string, "LOD_1" )==NULL ) iObjLOD1LimbIndex = c;
		if ( strcmp ( g_pGlob->checklist[c].string, "LOD_2" )==NULL ) iObjLOD2LimbIndex = c;

		// LOD markers produced by X->ASSIMP->OBJ->SIMPLYGON->FBX->BLENDER->X
		if ( strlen(g_pGlob->checklist[c].string) > 5 )
		{
			LPSTR pLODTextPart = g_pGlob->checklist[c].string + strlen(g_pGlob->checklist[c].string) - 5;
			if ( strcmp ( pLODTextPart, "_LOD1" )==NULL ) iObjLOD1LimbIndex = c;
			if ( strcmp ( pLODTextPart, "_LOD2" )==NULL ) iObjLOD2LimbIndex = c;
		}
	}

	// ensue we fill in missing LOD indices based on availability
	if ( iObjLOD1LimbIndex==-1 )
	{
		iObjLOD1LimbIndex = iObjLOD2LimbIndex;
		if ( iObjLOD1LimbIndex==-1 ) iObjLOD1LimbIndex = iObjLOD0LimbIndex;
	}
	if ( iObjLOD2LimbIndex==-1 )
	{
		iObjLOD2LimbIndex = iObjLOD1LimbIndex;
		if ( iObjLOD2LimbIndex==-1 ) iObjLOD2LimbIndex = iObjLOD0LimbIndex;
	}

	// use this command to assign per-object LOD distances
	if ( iLODLevel >= 1 && iLODLevel <= 2 )
		pObject->fLODDistance [ iLODLevel-1 ] = fDistanceOfLOD;

	// find first mesh (as fallback of LOD_X missing)
	int iFirstFrameWithMesh = 0;
	for (; iFirstFrameWithMesh<pRealObject->iFrameCount; iFirstFrameWithMesh++ )
		if ( pRealObject->ppFrameList[iFirstFrameWithMesh]->pMesh )
			break;

	// extra check to ensure mesh is not already covered by LOD1 or LOD2 (sometimes primary mesh has no LOD0 marker)
	for ( int iUniqueLOD0=0; iUniqueLOD0<pRealObject->iFrameCount; iUniqueLOD0++ )
	{
		if ( pRealObject->ppFrameList[iUniqueLOD0]->pMesh )
		{
			if ( iUniqueLOD0 != iObjLOD1LimbIndex && iUniqueLOD0 != iObjLOD2LimbIndex )
			{
				iFirstFrameWithMesh = iUniqueLOD0;
			}
		}
	}

	// special mode to disregard 'AddLODToObject' style mesh LOD, and use limb visibility style
	pObject->iUsingWhichLOD = -1000;
	pObject->bHadLODNeedCamDistance = true;
	pObject->iLOD0LimbIndex = iObjLOD0LimbIndex;
	pObject->iLOD1LimbIndex = iObjLOD1LimbIndex;
	pObject->iLOD2LimbIndex = iObjLOD2LimbIndex;
	if ( pObject->iLOD0LimbIndex==-1 ) pObject->iLOD0LimbIndex = iFirstFrameWithMesh;
	if ( pObject->iLOD1LimbIndex==-1 ) pObject->iLOD1LimbIndex = iFirstFrameWithMesh;
	if ( pObject->iLOD2LimbIndex==-1 ) pObject->iLOD2LimbIndex = iFirstFrameWithMesh;
}

void AddLODToObject ( int iCurrentID, int iLODModelID, int iLODLevel, float fDistanceOfLOD )
{
	// takes all meshes of lodmodel and adds them to a special lod alternative meshes of specified object
	if ( !ConfirmObject ( iCurrentID ) || !ConfirmObject ( iLODModelID ) )
		return;

	// make object from mesh contained within object
	sObject* pObject = g_ObjectList [ iCurrentID ];
	sObject* pLODObject = g_ObjectList [ iLODModelID ];

	// leeadd - 061208 - U71 - limit lod levels (allow 0,1 and since U71 use 2 for last QUAD/DECAL level)
	if ( iLODLevel<0 || iLODLevel>2 )
		return;

	// leefix - 250106 - so important when copying meshes!
	ResetVertexDataInMesh ( pLODObject );

	// flag as a LOD object
	pObject->iUsingWhichLOD = 0;
	pObject->bHadLODNeedCamDistance = true;
	if ( iLODLevel < 2 )
		pObject->fLODDistance [ iLODLevel ] = fDistanceOfLOD;
	else
		pObject->fLODDistanceQUAD = fDistanceOfLOD;

	// leeadd - 061208 - new alpha fade feature of LOD system
	pObject->iUsingOldLOD = -1;
	pObject->fLODTransition = 0.0f;

	// U74 - 120409 - if quad level used, default for object is at QUAD (furthest first)
	if ( iLODLevel==2 )
	{
		// start off as QUAD level, and adjust as required (copied by instance command too)
		pObject->iUsingOldLOD = 3;
		pObject->iUsingWhichLOD = 3;
	}

	// copy meshes of src object to dst object
	DWORD dwLODMeshCount = pLODObject->iMeshCount;
	if ( dwLODMeshCount>0 )
	{
		// go through all meshes in LOD model
		for ( int iLODFrame = 0; iLODFrame < pLODObject->iFrameCount; iLODFrame++ )
		{
			sFrame* pLODFrame = pLODObject->ppFrameList [ iLODFrame ];
			if ( pLODFrame )
			{
				if ( pLODFrame->pMesh )
				{
					// root frame used to scan obj heirarchy to make bone matrix assignments
					sFrame* pRootFrame = pObject->ppFrameList [ 0 ];

					// find this mesh in main object
					for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
					{
						// get frame ptr
						sFrame* pFrame = pObject->ppFrameList [ iFrame ];
						if ( pFrame )
						{
							// leeadd - U71 - where the mesh is going to go
							sMesh* pDestinationLODMesh = NULL;
							if ( iLODLevel < 2 )
								pDestinationLODMesh = pFrame->pLOD [ iLODLevel ];
							else
								pDestinationLODMesh = pFrame->pLODForQUAD;

							if ( pFrame->pMesh && pDestinationLODMesh==NULL )
							{
								// when found match (or proving a final QUAD model)
								if ( _stricmp ( pLODFrame->szName, pFrame->szName )==NULL || iLODLevel==2 )
								{
									// create mesh within main object, copying lod data across
									sMesh* pSrcMesh = pLODFrame->pMesh;
									sMesh* pDestMesh = new sMesh;

									// root matrix
									GGMATRIX matWorld;
									GGMatrixIdentity ( &matWorld );

									// mesh copy
									MakeMeshFromOtherMesh ( true, pDestMesh, pSrcMesh, &matWorld );
									if ( iLODLevel < 2 )
										pFrame->pLOD [ iLODLevel ] = pDestMesh;
									else
										pFrame->pLODForQUAD = pDestMesh;

									// ensure work from original mesh base
									CollectOriginalVertexData ( pDestMesh );

									// copy over settings from main mesh (transparency, etc) - cannot change after LOD in place!
									CopyMeshSettings ( pDestMesh, pSrcMesh );

									// bone data copy
									DWORD dwBoneCount = pSrcMesh->dwBoneCount;
									if ( dwBoneCount>0 )
									{
										pDestMesh->pBones = new sBone [ dwBoneCount ];
										for ( DWORD b=0; b<dwBoneCount; b++ )
										{
											DWORD dwNewBone = b;
											memcpy ( &pDestMesh->pBones [ dwNewBone ], &pSrcMesh->pBones [ b ], sizeof ( sBone ) );
											DWORD dwNumInfluences = pDestMesh->pBones [ dwNewBone ].dwNumInfluences;
											pDestMesh->pBones [ dwNewBone ].pVertices = new DWORD [ dwNumInfluences ];
											pDestMesh->pBones [ dwNewBone ].pWeights = new float [ dwNumInfluences ];
											memcpy ( pDestMesh->pBones [ dwNewBone ].pVertices, pSrcMesh->pBones [ b ].pVertices, sizeof(DWORD)*dwNumInfluences );
											memcpy ( pDestMesh->pBones [ dwNewBone ].pWeights, pSrcMesh->pBones [ b ].pWeights, sizeof(float)*dwNumInfluences );
										}
										pDestMesh->dwBoneCount = dwBoneCount;
									}

									// now map frames to bones
									InitOneMeshFramesToBones ( pDestMesh );
									MapOneMeshFramesToBones ( pDestMesh, pRootFrame );

									// share textures and shaders
									if ( iLODLevel<2 )
									{
										// copy texture from pFrame->pMesh (not srcmesh) as we want a LOD mesh only for geometry
										pDestMesh->dwTextureCount = pFrame->pMesh->dwTextureCount; 
										pDestMesh->pTextures = new sTexture [ pDestMesh->dwTextureCount ]; 
										CloneInternalTextures ( pDestMesh, pFrame->pMesh );

										// leefix - 041208 - U71 - copy over shader influence too
										CloneShaderEffects ( pDestMesh, pFrame->pMesh );

										// add mesh to buffers
										m_ObjectManager.AddObjectMeshToBuffers ( pFrame->pLOD [ iLODLevel ], false );
									}
									else
									{
										// copy over texture from pSrcMesh
										pDestMesh->dwTextureCount = pSrcMesh->dwTextureCount; 
										pDestMesh->pTextures = new sTexture [ pDestMesh->dwTextureCount ]; 
										CloneInternalTextures ( pDestMesh, pSrcMesh );
										CloneShaderEffects ( pDestMesh, pSrcMesh );
										m_ObjectManager.AddObjectMeshToBuffers ( pFrame->pLODForQUAD, false );
									}

									// 220513 - finished adding this LOD to parent object
									iFrame = pObject->iFrameCount;
								}
							}
						}
					}
				}
			}
		}
	}
}

DARKSDK_DLL void CloneObject ( int iDestinationID, int iSourceID, int iCloneSharedData )
{
	// iCloneSharedData modes:
	// 101 = use existing object in destination and JUST CLONE TEXTURES OVER

	// check if dest object not exists
	if ( iCloneSharedData!=101 )
	{
		// except for when dest already exists via mode 101
		if ( !ConfirmNewObject ( iDestinationID ) ) return;
		ID_ALLOCATION ( iDestinationID );
	}

	// check if src object exists
	if ( !ConfirmObjectInstance ( iSourceID ) )
		return;

	// first restore original vertex data in object 
	sObject* pObject = g_ObjectList [ iSourceID ];

	// mike - 011005 - see if the original source object is an instance
	if ( pObject->pInstanceOfObject )
		pObject = pObject->pInstanceOfObject;

	// clone the object
	if ( iCloneSharedData!=101 )
	{
		// create new object in destination
		ResetVertexDataInMesh ( pObject );

		if ( !CloneDBO ( &g_ObjectList [ iDestinationID ], pObject ) )
			return;

		// Special clone parameter shares an original object not deleted (good for lots of bone anim objects)
		sObject* pNewObject = g_ObjectList [ iDestinationID ];
		if ( iCloneSharedData!=0 )
		{
			// work on cloned object (cut out potential huge anim data)
			// Update dependency details
			// Do this whether or not there is animation data, to ensure that is predictable
			pObject->dwDependencyCount++;
			pNewObject->pObjectDependency = pObject;

			if ( pNewObject->pAnimationSet )
			{
				sAnimation* pOrigAnim = pObject->pAnimationSet->pAnimation;
				sAnimation* pAnim = pNewObject->pAnimationSet->pAnimation;
				if ( pAnim && pOrigAnim )
				{
					while ( pAnim != NULL )
					{
						// Erase Animation Data (matrix data usually huge)
						SAFE_DELETE(pAnim->pMatrixKeys);
						pAnim->dwNumMatrixKeys=0;

						// substitute with original objects anim data
						pAnim->pSharedReadAnim = pOrigAnim;

						// move to the next sequence
						pOrigAnim = pOrigAnim->pNext;
						pAnim = pAnim->pNext;
					}
				}
			}
		}

		// and put back the latest frame of the source object
		pObject->fAnimLastFrame=-1.0f;
		UpdateObjectAnimation ( pObject );

		// setup new object and introduce to buffers
		SetNewObjectFinalProperties ( iDestinationID, -1.0f );

		// lee - 180406 - u6rc10 - clone all collision data
		pNewObject->collision = g_ObjectList [ iSourceID ]->collision;
	}

	// transfer references within meshes that cannot be DBO cloned (post-setup)
	sObject* pNewObject = g_ObjectList [ iDestinationID ];
	DWORD dwMeshCount = pObject->iMeshCount;
	if ( pNewObject->iMeshCount<(int)dwMeshCount ) dwMeshCount = pNewObject->iMeshCount;
	for ( int iMesh = 0; iMesh < (int)dwMeshCount; iMesh++ )
	{
		sMesh* pMesh = NULL; if ( iMesh<pObject->iMeshCount ) pMesh = pObject->ppMeshList [ iMesh ];
		sMesh* pNewMesh = NULL; if ( iMesh<pNewObject->iMeshCount ) pNewMesh = pNewObject->ppMeshList [ iMesh ];
		if ( pMesh && pNewMesh )
		{
			CloneShaderEffects ( pNewMesh, pMesh );
		}
	}

	// 010917 - also clone frame exclusion flag
	DWORD dwFrameCount = pNewObject->iFrameCount;
	for ( int iFrame = 0; iFrame < (int)dwFrameCount; iFrame++ )
	{
		sFrame* pFrame = NULL; if ( iFrame<pObject->iFrameCount ) pFrame = pObject->ppFrameList [ iFrame ];
		sFrame* pNewFrame = NULL; if ( iFrame<pNewObject->iFrameCount ) pNewFrame = pNewObject->ppFrameList [ iFrame ];
		pNewFrame->bExcluded = pFrame->bExcluded;
	}

	// 131115 - transfer flags required by clones
	pNewObject->bIgnoreDefAnim = g_ObjectList [ iSourceID ]->bIgnoreDefAnim;

	// 090217 - clone special spine center system
	pNewObject->bUseSpineCenterSystem = g_ObjectList [ iSourceID ]->bUseSpineCenterSystem;
	pNewObject->dwSpineCenterLimbIndex = g_ObjectList [ iSourceID ]->dwSpineCenterLimbIndex;
	pNewObject->fSpineCenterTravelDeltaX = g_ObjectList [ iSourceID ]->fSpineCenterTravelDeltaX;
	pNewObject->fSpineCenterTravelDeltaZ = g_ObjectList [ iSourceID ]->fSpineCenterTravelDeltaZ;

	// handle clone ref
	if ( iCloneSharedData!=101 )
	{
		// some shader settings take over vertex control (so reset to original data)
		ResetVertexDataInMesh ( pNewObject );

		// add object id to shortlist
		AddObjectToObjectListRef ( iDestinationID );
	}

	// clone textures for all meshes (clone references)
	if ( g_ObjectList [ iDestinationID ]->ppMeshList )
	{
		for ( int iMesh = 0; iMesh < g_ObjectList [ iDestinationID ]->iMeshCount; iMesh++ )
		{
			// get mesh ptr
			sMesh* pMesh = NULL; if ( iMesh<g_ObjectList [ iDestinationID ]->iMeshCount ) pMesh = g_ObjectList [ iDestinationID ]->ppMeshList [ iMesh ];
			sMesh* pOrigMesh = NULL; if ( iMesh<pObject->iMeshCount ) pOrigMesh = pObject->ppMeshList [ iMesh ];
			if ( pMesh && pOrigMesh )
			{
				// lee - 230206 - handle if multimaterial or regular mesh
				//pMesh->bUseMultiMaterial = pOrigMesh->bUseMultiMaterial;
				DWORD dwMultiMatCount = pOrigMesh->dwMultiMaterialCount;
				if ( pMesh->bUseMultiMaterial==true && pOrigMesh->bUseMultiMaterial==true )
				{
					// Currrent texture if any used
					sTexture* pTexture = NULL;

					// if multimaterial not exist (clone texture only)
					if ( pMesh->pMultiMaterial==NULL )
					{
						// create it now
						pMesh->pMultiMaterial = new sMultiMaterial [ dwMultiMatCount ];
						memset ( pMesh->pMultiMaterial, 0, sizeof(sMultiMaterial) * dwMultiMatCount );
					}

					// Define textures for multi material array
					pMesh->dwMultiMaterialCount = dwMultiMatCount;
					for ( DWORD m=0; m<dwMultiMatCount; m++ )
					{
						// get multimat at index
						sMultiMaterial* pMultiMat = &(pMesh->pMultiMaterial [ m ]);
						sMultiMaterial* pOrigMultiMat = &(pOrigMesh->pMultiMaterial [ m ]);

						// copy references over (clone)
						strcpy ( pMultiMat->pName, pOrigMultiMat->pName );
						pMultiMat->dwIndexCount		= pOrigMultiMat->dwIndexCount;
						pMultiMat->dwIndexStart		= pOrigMultiMat->dwIndexStart;
						pMultiMat->mMaterial		= pOrigMultiMat->mMaterial;
						pMultiMat->pTexturesRef		= pOrigMultiMat->pTexturesRef;
						pMultiMat->pTexturesRefN	= pOrigMultiMat->pTexturesRefN;
						pMultiMat->pTexturesRefS	= pOrigMultiMat->pTexturesRefS;
						pMultiMat->pTexturesRefG	= pOrigMultiMat->pTexturesRefG;
						pMultiMat->pTexturesRefM	= pOrigMultiMat->pTexturesRefM;
					}

					// multimaterial still uses texture)
					CloneInternalTextures ( pMesh, pOrigMesh );
				}
				else
				{
					// if original multimesh but dest regular, copy multi to reg (first material wins)
					if ( pMesh->bUseMultiMaterial==false && pOrigMesh->bUseMultiMaterial==true )
					{
						if ( dwMultiMatCount>=1 )
						{
							//SAFE_DELETE(pMesh->pTextures);//crashes, mem leak, fuind out whY!!
							if ( pMesh->dwTextureCount!=2 )
							{
								pMesh->dwTextureCount = 2;
								pMesh->pTextures = new sTexture [ pMesh->dwTextureCount ];
								memset ( pMesh->pTextures, 0, sizeof(sTexture)*pMesh->dwTextureCount );
							}
							pMesh->pTextures [ 0 ].iImageID  = pOrigMesh->pTextures [ 0 ].iImageID;;
							pMesh->pTextures [ 0 ].pTexturesRefView = pOrigMesh->pMultiMaterial [ 0 ].pTexturesRef;
							strcpy ( pMesh->pTextures [ 0 ].pName, pOrigMesh->pMultiMaterial [ 0 ].pName );
							pMesh->pTextures [ 0 ].dwBlendMode = GGTOP_MODULATE;
							pMesh->pTextures [ 0 ].dwBlendArg1 = GGTA_TEXTURE;
							pMesh->pTextures [ 0 ].dwBlendArg2 = GGTA_DIFFUSE;
							pMesh->bUseMultiMaterial = false;
							pMesh->fSpecularOverride = 1.0f;
							pMesh->bUsesMaterial = false;

							// also wipe out shader as it will screw up lightmapper
							SetSpecialEffect ( pMesh, NULL );
						}
					}
					else
					{
						// regular mesh clone texture
						CloneInternalTextures ( pMesh, pOrigMesh );
					}
				}
			}
		}
	}

	if ( iCloneSharedData!=101 )
	{
		// position data of cloned object must match source
		memcpy ( &g_ObjectList [ iDestinationID ]->position, &pObject->position, sizeof ( sPositionData ) );
	}

	// 110416 - copy over matrix mode state (some models use special FBX rendering matrix styles)
	g_ObjectList [ iDestinationID ]->dwApplyOriginalScaling = g_ObjectList [ iSourceID ]->dwApplyOriginalScaling;
}

DARKSDK_DLL void CloneObject ( int iDestinationID, int iSourceID )
{
	CloneObject ( iDestinationID, iSourceID, 0 );
}

DARKSDK_DLL void InstanceObject ( int iDestinationID, int iSourceID )
{
    int iActualSourceID = iSourceID;

	// check if dest object not exists
	if ( !ConfirmNewObject ( iDestinationID ) )
		return;

    // u74b7 - Allow instancing of an instance
    //         Actually instance the original instead.

	// check if src object exists, either as an original or an instance
	if ( !ConfirmObjectInstance ( iSourceID ) )
		return;

    // If src is an instance, use that instances original for mesh data
    // but copy the other details from the instance
    if (g_ObjectList [ iSourceID ]->pInstanceOfObject)
    {
        iActualSourceID = g_ObjectList [ iSourceID ]->pInstanceOfObject->dwObjectNumber;
    }

	// check memory allocation
	ID_ALLOCATION ( iDestinationID );

	// create pure instance of source object
	g_ObjectList [ iDestinationID ] = new sObject;
	g_ObjectList [ iDestinationID ]->pInstanceOfObject = g_ObjectList [ iActualSourceID ];
    g_ObjectList [ iDestinationID ]->pObjectDependency = g_ObjectList [ iActualSourceID ];
    g_ObjectList [ iActualSourceID ]->dwDependencyCount++;

	// U72 - 100109 - flag parent so it knows to animate even if not visible (irreversable)
	g_ObjectList [ iSourceID ]->position.bParentOfInstance = true;

	// lee - 250307 - store object number for reference
	g_ObjectList [ iDestinationID ]->dwObjectNumber = iDestinationID;

	// copy over basic info such as collision (so instances can have instant collision)
	g_ObjectList [ iDestinationID ]->collision = g_ObjectList [ iSourceID ]->collision;

	// 250217 - messed up collision center values, so recalc here for destination
	g_ObjectList [ iDestinationID ]->collision.vecCentre = g_ObjectList [ iDestinationID ]->collision.vecMin + ((g_ObjectList [ iDestinationID ]->collision.vecMax - g_ObjectList [ iDestinationID ]->collision.vecMin)/2.0f); 
	g_ObjectList [ iDestinationID ]->collision.bColCenterUpdated = false;

	// lee - 310306 - u6rc5 - must carry LOD flag info across
	g_ObjectList [ iDestinationID ]->fLODDistance[0] = g_ObjectList [ iSourceID ]->fLODDistance[0];
	g_ObjectList [ iDestinationID ]->fLODDistance[1] = g_ObjectList [ iSourceID ]->fLODDistance[1];
	g_ObjectList [ iDestinationID ]->fLODDistanceQUAD = g_ObjectList [ iSourceID ]->fLODDistanceQUAD;
	g_ObjectList [ iDestinationID ]->bHadLODNeedCamDistance = g_ObjectList [ iSourceID ]->bHadLODNeedCamDistance;

	// U74 - 120409 - copy current state of parent LOD as well (so instancing when as a QUAD, recreate QUAD)
	g_ObjectList [ iDestinationID ]->iUsingOldLOD = g_ObjectList [ iSourceID ]->iUsingOldLOD;
	g_ObjectList [ iDestinationID ]->iUsingWhichLOD = g_ObjectList [ iSourceID ]->iUsingWhichLOD;
	g_ObjectList [ iDestinationID ]->fLODTransition = g_ObjectList [ iSourceID ]->fLODTransition;

	// create limb visibility array
	g_ObjectList [ iDestinationID ]->pInstanceMeshVisible = new bool [ g_ObjectList[iSourceID]->iFrameCount ];
	memset ( g_ObjectList [ iDestinationID ]->pInstanceMeshVisible, 255, g_ObjectList[iSourceID]->iFrameCount * sizeof(bool) );

	// mike - 021005 - retain pivot from source
	g_ObjectList [ iDestinationID ]->position.bApplyPivot = g_ObjectList [ iSourceID ]->position.bApplyPivot;
	g_ObjectList [ iDestinationID ]->position.matPivot = g_ObjectList [ iSourceID ]->position.matPivot;

	// add object id to shortlist
	AddObjectToObjectListRef ( iDestinationID );
}

DARKSDK_DLL void MakeObjectSphere ( int iID, float fRadius, int iRings, int iSegments )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "sphere" ) )
		return;

	// DBV1 size=diameter
	fRadius/=2;

	// setup general object data
	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;
	if ( MakeMeshSphere ( true, pMesh, GGVECTOR3(0,0,0), fRadius, iRings, iSegments, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB(255,255,255,255) )==false )
		return;

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, fRadius );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void MakeObjectSphere ( int iID, float fRadius )
{
	MakeObjectSphere ( iID, fRadius, 12, 12 );
}

DARKSDK_DLL void MakeObjectCube ( int iID, float fSize )
{
	MakeObjectBox ( iID, fSize, fSize, fSize );
}

DARKSDK_DLL GlobStruct* GetGlobalStructure ( void )
{
	return g_pGlob;
}

DARKSDK_DLL void GetPositionData ( int iID, sPositionData** ppPosition )
{
	if ( g_ObjectList [ iID ] )
	{
		CalculateObjectWorld ( g_ObjectList [ iID ], NULL );
		*ppPosition = &g_ObjectList [ iID ]->position;
	}
}

//Dave Performance - enables objects to be marked as static (they are set to false by default)
DARKSDK void SetObjectStatic ( int iID , bool isStatic = true )
{
	if ( g_ObjectList [ iID ] )
	{
		g_ObjectList [ iID ]->bIsStatic = isStatic;
		//Update collision centre, because it won't get updated again
		if ( isStatic )
			UpdateColCenter ( g_ObjectList [ iID ] );
	}
}

DARKSDK void SetObjectAsCharacter ( int iID, bool mode )
{
	if ( g_ObjectList [ iID ] )
	{
		g_ObjectList [ iID ]->bIsCharacter = mode;
	}
}

std::vector <int> g_vIgnoredObjectList;
extern bool g_ForceTextureListUpdate;

//Dave Performance - enables objects to be marked as ignored and not be in the sorting list
void SetIgnoreObject ( int iID , bool mode )
{
	if ( g_ObjectList [ iID ] )
	{
		g_ObjectList [ iID ]->bIgnored = mode;
		if ( mode ) g_vIgnoredObjectList.push_back ( iID );
	}
}

//Dave Performance - sets no objects to be ignored and resets the list
void ClearIgnoredObjects ( void )
{
	for ( int c = 0 ; c < (int)g_vIgnoredObjectList.size() ; c++ )
	{
		if ( g_ObjectList [ g_vIgnoredObjectList[c] ] )
		{
			g_ObjectList [ g_vIgnoredObjectList[c] ]->bIgnored = false;
		}
	}

	g_vIgnoredObjectList.clear();
}

void ShowIgnoredObjects ( void )
{
	for ( int c = 0 ; c < (int)g_vIgnoredObjectList.size() ; c++ )
	{
		if ( g_ObjectList [ g_vIgnoredObjectList[c] ] )
		{
			g_ObjectList [ g_vIgnoredObjectList[c] ]->bIgnored = false;
			g_ObjectList [ g_vIgnoredObjectList[c] ]->bVisible = true;
		}
	}

	DoTextureListSort();
}

void HideIgnoredObjects ( void )
{
	for ( int c = 0 ; c < (int)g_vIgnoredObjectList.size() ; c++ )
	{
		if ( g_ObjectList [ g_vIgnoredObjectList[c] ] )
		{
			g_ObjectList [ g_vIgnoredObjectList[c] ]->bIgnored = true;
			g_ObjectList [ g_vIgnoredObjectList[c] ]->bVisible = false;
		}
	}

	DoTextureListSort();
}

//Dave Performance - force the texture list sorting to happen, ignoring any objects set to be ignored
void DoTextureListSort ( void )
{
	g_ForceTextureListUpdate = true;
	m_ObjectManager.SortTextureList();
	g_ForceTextureListUpdate = false;
}


DARKSDK_DLL void GetEmitterData ( int iID, BYTE** ppVertices, DWORD* pdwVertexCount, int** ppiDrawCount )
{
	if ( g_ObjectList [ iID ] )
	{
		if ( g_ObjectList [ iID ]->pFrame->pMesh )
		{
			sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;

			*ppVertices     = pMesh->pVertexData;
			*pdwVertexCount = pMesh->dwVertexCount;
			*ppiDrawCount   = &pMesh->iDrawPrimitives;
		}
	}
}

DARKSDK_DLL void UpdateEmitter ( int iID )
{
	if ( g_ObjectList [ iID ] )
	{
		if ( g_ObjectList [ iID ]->pFrame->pMesh )
		{
			g_ObjectList [ iID ]->pFrame->pMesh->bVBRefreshRequired = true;
			g_vRefreshMeshList.push_back ( g_ObjectList [ iID ]->pFrame->pMesh );
		}
	}
}

DARKSDK_DLL void MakeEmitter ( int iID, int iSize )
{
	MakeObjectPlane ( iID, iSize, iSize );
}

// New construction commands for multiplayer nameplates
DARKSDK int MakeNewObjectPanel	( int iID , int iNumberOfCharacters )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "nameplate" ) )
		return 0;

	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;

	// create memory
	DWORD dwVertexCount = 6 * iNumberOfCharacters; // store number of vertices
	DWORD dwIndexCount  = 0; // store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_DIFFUSE | GGFVF_TEX1, dwVertexCount, dwIndexCount, false ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return 0;
	}

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = iNumberOfCharacters * 2;

	return 1;
}

DARKSDK void SetObjectPanelQuad	( int iID, int index, float fX, float fY, float fWidth, float fHeight, float fU1, float fV1, float fU2, float fV2, int r , int g , int b )
{
	index *= 6;

	// DB compatability
	fWidth  /= 2.0f;
	fHeight /= 2.0f;
	fX /= 2.0f;
	fY /= 2.0f;

	float pos_pos_x = fX + fWidth;
	float neg_pos_x = fX;
	float pos_pos_y = fY + fHeight;
	float neg_pos_y = fY;

	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;

	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 0,  neg_pos_x,  pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU1, fV1 );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 1,   pos_pos_x,  pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU2, fV1 );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 2,  pos_pos_x, neg_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU2, fV2 );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 3,   pos_pos_x,  neg_pos_y, 0.0f,  0.0f,  0.0f, -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU2, fV2 );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 4,   neg_pos_x, neg_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU1, fV2 );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  index + 5,   neg_pos_x, pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, r, g, b ), fU1, fV1 );

}

DARKSDK void FinishObjectPanel	( int iID, float fWidth, float fHeight )
{
	// DB compatability
	fWidth  /= 2.0f;
	fHeight /= 2.0f;

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, (fWidth+fHeight)/2 );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void MakeObjectPlane ( int iID, float fWidth, float fHeight, int iFlag , bool legacymode)
{

	// attempt to create a new object
	if ( !CreateNewObject ( iID, "plane" ) )
		return;

	// DB compatability
	fWidth  /= 2.0f;
	fHeight /= 2.0f;

	// create box mesh for object
	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;

#ifdef PRODUCTCLASSIC
	MakeMeshPlain(true, pMesh, fWidth, fHeight, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB(255, 255, 255, 255));
#else
	if(legacymode)
		MakeMeshPlain(true, pMesh, fWidth, fHeight, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB(255, 255, 255, 255));
	else
		MakeMeshPlainEx ( true, pMesh, fWidth, fHeight, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB (255,255,255,255) );
#endif

	// if flag is 2, create index buffer for this plane
	if (iFlag == 2)
	{
		//// make temp terrain nicer to look at
		//ScaleTexture(pMesh, 200, 200);

		// for when planes require an index buffer
		if (pMesh->dwIndexCount == 0)
		{
			// create some indices right now
			pMesh->dwIndexCount = pMesh->dwVertexCount;
			pMesh->pIndices = new WORD[pMesh->dwIndexCount];
			for (int i = 0; i < pMesh->dwIndexCount; i++)
				pMesh->pIndices[i] = i;
		}
	}

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, (fWidth+fHeight)/2 );
	
	// this is not called in regular MakeObjectPlane(id,w,h), i.e. when flag==0

#ifdef PRODUCTCLASSIC
	//PE: Strange Classic always rotated this.
	YRotateObject(iID, 180);
#else
	if (iFlag > 0)
	{
		// set object Y=180 for compatibility with correct plain object
		YRotateObject(iID, 180);
	}
#endif

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );

	// special settings
	SetObjectCull ( iID, false );
}

DARKSDK_DLL void MakeObjectPlane ( int iID, float fWidth, float fHeight )
{
	MakeObjectPlane(iID, fWidth, fHeight, 0 , false);
}

DARKSDK_DLL void MakeObjectTriangle ( int iID, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3 )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "triangle" ) )
		return;

	// setup general object data
	sMesh* pMesh         = g_ObjectList [ iID ]->pFrame->pMesh;	// get a pointer to the mesh ( easier to access now )

	// create vertex memory
	DWORD dwVertexCount = 3;									// store number of vertices
	DWORD dwIndexCount  = 0;									// store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, dwVertexCount, dwIndexCount, false ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}
	
	// create vertices for plane
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  0,  x1, y1, z1,  0.0f,  0.0f, -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  1,  x2, y2, z2,  0.0f,  0.0f, -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  2,  x3, y3, z3,  0.0f,  0.0f, -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 1.0f );

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = 1;

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, x2-x1 );

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );

	// special settings
	SetObjectCull ( iID, false );
}

DARKSDK_DLL void MakeObjectBox ( int iID, float fWidth, float fHeight, float fDepth )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "box" ) )
		return;

	// first off divide the size by 2 to keep compatibility with DB
	fWidth  /= 2.0f;
	fHeight /= 2.0f;
	fDepth  /= 2.0f;

	// create box mesh for object
	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;
	MakeMeshBox ( true, pMesh, -fWidth, -fHeight, -fDepth, fWidth, fHeight, fDepth, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB (255,255,255,255) );

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, (fWidth+fHeight)/2 );

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void MakeObjectPyramid ( int iID, float fSize )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "pyramid" ) )
		return;

	// first off divide the size by 2
	fSize  /= 2.0f;

	// create mesh for object
	sMesh* pMesh = g_ObjectList [ iID ]->pFrame->pMesh;
// lee - 150306 - u60b3 - added DIFFUSE so that latest DX can handle the undeclared format of this model (aniso)
//	MakeMeshPyramid ( true, pMesh, fSize, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, GGCOLOR_ARGB (255,255,255,255) );
	MakeMeshPyramid ( true, pMesh, fSize, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_DIFFUSE | GGFVF_TEX1, GGCOLOR_ARGB (255,255,255,255) );

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, fSize );

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void MakeObjectCylinder ( int iID, float fSize )
{
	// attempt to create a new object
	if ( !CreateNewObject ( iID, "cylinder" ) )
		return;

	float fHeight   = fSize;
	float fRadius   = fSize / 2;
	int   iSegments = 30;

	// setup general object data
	sMesh* pMesh         = g_ObjectList [ iID ]->pFrame->pMesh;		// get a pointer to the mesh ( easier to access now )

	// create vrtex memory
	DWORD dwVertexCount = ( iSegments + 1 ) * 2;					// store number of vertices
	DWORD dwIndexCount  = 0;										// store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, dwVertexCount, dwIndexCount, false ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}

	float fDeltaSegAngle = ( 2.0f * GG_PI / iSegments );
	float fSegmentLength = 1.0f / ( float ) iSegments;
	int	  iVertex        = 0;

	// create the sides triangle strip
	for ( int iCurrentSegment = 0; iCurrentSegment <= iSegments; iCurrentSegment++ )
	{
		float x0 = fRadius * sinf ( iCurrentSegment * fDeltaSegAngle );
		float z0 = fRadius * cosf ( iCurrentSegment * fDeltaSegAngle );

		// Calculate normal
		GGVECTOR3 Normal = GGVECTOR3 ( x0, 0.0f, z0 );
		GGVec3Normalize ( &Normal, &Normal ); 

		// set vertex A
		SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  iVertex, x0, 0.0f+(fHeight/2.0f), z0, Normal.x, Normal.y, Normal.z, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 0.0f );

		// increment vertex index
		iVertex++;

		// set vertex B
		SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  iVertex, x0, 0.0f-(fHeight/2.0f), z0, Normal.x, Normal.y, Normal.z, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 1.0f );

		// increment vertex index
		iVertex++;
	}

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLESTRIP;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = pMesh->dwVertexCount - 2;

	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, fSize );

	// box collision for box shapes
	SetColToBoxes ( g_ObjectList [ iID ] );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

DARKSDK_DLL void MakeObjectCone ( int iID, float fSize )
{
	// make a cone

	// attempt to create a new object
	if ( !CreateNewObject ( iID, "cone" ) )
		return;

	float fHeight   = fSize;
	int   iSegments = 11;

	// correct cone size
	fSize/=2.0f;
	
	// setup general object data
	sMesh* pMesh         = g_ObjectList [ iID ]->pFrame->pMesh;		// get a pointer to the mesh ( easier to access now )

	// create vrtex memory
	DWORD dwVertexCount = (iSegments * 2) + 1;						// store number of vertices
	DWORD dwIndexCount  = iSegments * 3;							// store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, dwVertexCount, dwIndexCount, false ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return;
	}

	float fDeltaSegAngle = ( (2.0f * GG_PI) / iSegments );
	float fSegmentLength = 1.0f / ( float ) iSegments;
	float fy0            = ( 90.0f - ( float ) GGToDegree ( atan ( fHeight / fSize ) ) ) / 90.0f;
	int	  iVertex        = 0;
	int	  iIndex         = 0;
	WORD  wVertexIndex   = 0;

	// for each segment, add a triangle to the sides triangle list
	for ( int iCurrentSegment = 0; iCurrentSegment <= iSegments; iCurrentSegment++ )
	{
		float x0 = fSize * sinf ( iCurrentSegment * fDeltaSegAngle );
		float z0 = fSize * cosf ( iCurrentSegment * fDeltaSegAngle );

		// Calculate normal
		GGVECTOR3 Normal = GGVECTOR3 ( x0, fy0, z0 );
		GGVec3Normalize ( &Normal, &Normal ); 

		// not the last segment though
		if ( iCurrentSegment < iSegments )
		{
			// set vertex A
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  iVertex, 0.0f, 0.0f+(fHeight/2.0f), 0.0f, Normal.x, Normal.y, Normal.z, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 0.0f );

			// increment vertex index
			iVertex++;

			// set vertex B
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  iVertex, x0, 0.0f-(fHeight/2.0f), z0, Normal.x, Normal.y, Normal.z, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 1.0f );

			// increment vertex index
			iVertex++;
		}
		else
		{
			// set last vertex
			SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  iVertex, x0, 0.0f - ( fHeight / 2.0f ), z0, Normal.x, Normal.y, Normal.z, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 1.0f );

			// increment vertex index
			iVertex++;
		}

		// not the last segment though
		if ( iCurrentSegment < iSegments )
		{
			// set three indices per segment
			pMesh->pIndices [ iIndex ] = wVertexIndex;
			iIndex++;
			wVertexIndex++;
			
			pMesh->pIndices [ iIndex ] = wVertexIndex;
			iIndex++;

			if ( iCurrentSegment == iSegments-1 )
				wVertexIndex += 1;
			else
				wVertexIndex += 2;
			
			pMesh->pIndices [ iIndex ] = wVertexIndex;
			iIndex++;
			wVertexIndex--;	
		}
	}

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = ( iSegments * 2 ) + 1;
	pMesh->iDrawPrimitives  = iSegments;
	
	// setup new object and introduce to buffers
	SetNewObjectFinalProperties ( iID, fSize );

	// give the object a default texture
	TextureObject ( iID, 0 );
}

// Animation Commands

DARKSDK_DLL void AppendObject ( LPSTR pString, int iID, int iFrame )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pAnimationSet==NULL )
	{
		RunTimeError(RUNTIMEERROR_B3DKEYFRAMENOTEXIST);
		return;
	}

	// Append animation from file to model
	if ( !AppendAnimationFromFile ( pObject, (LPSTR)pString, iFrame ) )
	{
		RunTimeError(RUNTIMEERROR_B3DOBJECTAPPENDFAILED);
		return;
	}
}

DARKSDK_DLL void PlayObject ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimLooping = false;
	pObject->bAnimPlaying = true;
	pObject->fAnimFrame = 0.0f;
}

DARKSDK_DLL void PlayObject ( int iID, int iStart )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimLooping = false;
	pObject->bAnimPlaying = true;
	pObject->fAnimFrame = (float)iStart;
	if ( pObject->fAnimFrame > pObject->fAnimFrameEnd )
		pObject->fAnimFrameEnd=pObject->fAnimTotalFrames;
}

DARKSDK_DLL void PlayObject ( int iID, int iStart, int iEnd )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimLooping = false;
	pObject->bAnimPlaying = true;
	pObject->fAnimFrame = (float)iStart;
	pObject->fAnimFrameEnd = (float)iEnd;
}

DARKSDK_DLL void LoopObject ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimPlaying = true;
	pObject->bAnimLooping = true;
	pObject->fAnimFrame = 0.0f;
}

DARKSDK_DLL void LoopObject ( int iID, int iStart )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimLooping = true;
	pObject->bAnimPlaying = true;
	pObject->fAnimLoopStart = (float)iStart;
	if ( pObject->fAnimFrame < pObject->fAnimLoopStart )
		pObject->fAnimFrame = pObject->fAnimLoopStart;
}

DARKSDK_DLL void LoopObject ( int iID, int iStart, int iEnd )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->bAnimLooping = true;
	pObject->bAnimPlaying = true;
	pObject->fAnimLoopStart = (float)iStart;
	pObject->fAnimFrameEnd = (float)iEnd;
	if ( pObject->fAnimFrame < pObject->fAnimLoopStart )
		pObject->fAnimFrame = pObject->fAnimLoopStart;
	if ( pObject->fAnimFrame > pObject->fAnimFrameEnd )
		pObject->fAnimFrame = pObject->fAnimLoopStart;
}

DARKSDK_DLL void StopObject(int iID)
{
	// check the object exists
	if (!ConfirmObject(iID))
		return;

	// control animation
	sObject* pObject = g_ObjectList[iID];

	pObject->bAnimLooping = false;
	pObject->bAnimPlaying = false;
}

DARKSDK_DLL void SetObjectSpeed ( int iID, int iSpeed )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];

	pObject->fAnimSpeed = (float)iSpeed / 100.0f;
}

DARKSDK_DLL void SetObjectDefAnim ( int iID, int iSkipDefFrameInIntersectAll )
{
	if ( !ConfirmObject ( iID ) ) return;
	sObject* pObject = g_ObjectList [ iID ];
	if ( iSkipDefFrameInIntersectAll==1 )
		pObject->bIgnoreDefAnim = true;
	else
		pObject->bIgnoreDefAnim = false;
}

DARKSDK_DLL void SetObjectFrameEx ( int iID, float fFrame, int iRecalculateBounds )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// control animation
	if ( pObject->fAnimInterp==1.0f || pObject->bAnimPlaying )
	{
		// direct frame set if animating or not manually slerping
		pObject->fAnimFrame = fFrame;
	}
	else
	{
		// use manual slerp for frame interpolation
		pObject->bAnimManualSlerp = true;
		pObject->fAnimSlerpStartFrame = pObject->fAnimFrame;
		pObject->fAnimSlerpEndFrame = fFrame;
		pObject->fAnimSlerpTime = 0.0f;
	}

	// ensure object is updated
	pObject->bAnimUpdateOnce = true;

	// leeadd - 211008 - u71 - whether frame zero shifts overall object bounds
	DWORD dwDoesNotShiftFrameZero = 0;

	// leeadd - 070305 - if recalc, perform update and recalc bounds
	if ( iRecalculateBounds==1 )
	{
		// triggers bounds to be recalculated
		pObject->bUpdateOnlyCurrentFrameBounds = true;
		pObject->bUpdateOverallBounds = true;
	}

	// leefix - 260806 - u63 - replaced else which forced a bounds recalc from the SET FRAME obj,frm command..
	if ( iRecalculateBounds==2 )
	{
		// can reverse the setting
		if ( pObject->bUpdateOnlyCurrentFrameBounds )
		{
			pObject->bUpdateOnlyCurrentFrameBounds = false;
			pObject->bUpdateOverallBounds = true;
		}
	}

	// leeadd - 211008 - u71 - recalc, but flag frame zero so it does not shift overall object bounds
	if ( iRecalculateBounds==3 )
	{
		// triggers bounds to be recalculated
		pObject->bUpdateOnlyCurrentFrameBounds = false;
		pObject->bUpdateOverallBounds = true;
		dwDoesNotShiftFrameZero = 1;
	}

	// flag frame to NOT shift object bounds
	if ( pObject->ppFrameList )
	{
		if ( pObject->ppFrameList [ 0 ] )
		{
			if ( dwDoesNotShiftFrameZero==1 )
				pObject->ppFrameList [ 0 ]->dwStatusBits |= 1; // bit 1
			else
				pObject->ppFrameList [ 0 ]->dwStatusBits &= 1; // bit 1
		}
	}
}

DARKSDK_DLL void SetObjectFrameEx ( int iID, float fFrame )
{
	// by default it does not recalculate bounds as it is slow
	SetObjectFrameEx ( iID, fFrame, 0 );
}

DARKSDK_DLL void SetObjectFrame ( int iID, int iFrame )
{
	SetObjectFrameEx ( iID, (float)iFrame );
}

DARKSDK_DLL void SetObjectFrameEx ( int iID, int iLimbID, float fFrame, int iEnableOverride )
{
	// check the object exists (and limb)
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// can set an animation frame override per limb
	if ( pObject->pfAnimLimbFrame==NULL )
	{
		// must allocate it if not currently created
		pObject->pfAnimLimbFrame = new float [ pObject->iFrameCount ];
		for ( int iN=0; iN<pObject->iFrameCount; iN++ )
			pObject->pfAnimLimbFrame [ iN ] = -1.0f;
	}

	// set the per limb animation frame
	switch ( iEnableOverride )
	{
		case 0 : pObject->pfAnimLimbFrame [ iLimbID ] = -1.0f;	break; // normal
		case 1 : pObject->pfAnimLimbFrame [ iLimbID ] = fFrame;	break; // override with new frame
		case 2 : pObject->pfAnimLimbFrame [ iLimbID ] = -2.0f;	break; // disable all animation (manual angles only)
	}

	// ensure object is updated
	pObject->bAnimUpdateOnce = true;
}

DARKSDK_DLL void SetObjectInterpolation ( int iID, int iJump )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// control animation
	sObject* pObject = g_ObjectList [ iID ];
	pObject->fAnimInterp = (float)iJump/100.0f;

	// leefix - 210303 - to switch off interpolation
	if ( iJump==100 ) pObject->bAnimManualSlerp=false;
}

DARKSDK_DLL void SaveObjectAnimation ( int iID, LPSTR pFilename )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK_DLL void AppendObjectAnimation ( int iID, LPSTR pFilename )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK_DLL void SetObjectAnimationMode ( int iID, int iMode )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

// Visual Commands

DARKSDK_DLL void AddVisibilityListMask ( int iID )
{
	m_ObjectManager.m_dwSkipVisibilityListMask |= (1<<iID);
}

DARKSDK_DLL void HideObject ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// set object to visibility
	sObject* pObject = g_ObjectList [ iID ];
	pObject->bVisible = false;

}

DARKSDK_DLL void ShowObject ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// set object to visibility
	sObject* pObject = g_ObjectList [ iID ];
	pObject->bVisible = true;

}

DARKSDK_DLL void SetObjectRotationXYZ ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// object rotation setting
	sObject* pObject = g_ObjectList [ iID ];
	pObject->position.dwRotationOrder = ROTORDER_XYZ;
}

DARKSDK_DLL void SetObjectRotationZYX ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// object rotation setting
	sObject* pObject = g_ObjectList [ iID ];
	pObject->position.dwRotationOrder = ROTORDER_ZYX;
}

DARKSDK_DLL void SetObjectRotationZXY ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// object rotation setting
	sObject* pObject = g_ObjectList [ iID ];
	pObject->position.dwRotationOrder = ROTORDER_ZXY;
}

/*
DARKSDK_DLL void GhostObjectOn ( int iID, int iFlag )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceOfObject==NULL )
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			SetGhost ( pObject->ppMeshList [ iMesh ], true, iFlag );

	// promote to overlay layer
	pObject->bGhostedObject = true;

	UpdateOverlayFlag ( pObject );
}

DARKSDK_DLL void GhostObjectOn ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// use higher function
	GhostObjectOn ( iID, -1 );
}

DARKSDK_DLL void GhostObjectOff ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetGhost ( pObject->ppMeshList [ iMesh ], false, -1 );

	// promote to overlay layer
	pObject->bGhostedObject = false;
	UpdateOverlayFlag ( pObject );
}
*/

DARKSDK_DLL void FadeObject ( int iID, float fPercentage )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// knock down perc
	fPercentage/=100.0f;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetDiffuse ( pObject->ppMeshList [ iMesh ], fPercentage );
}

void GlueObjectToLimbEx ( int iSource, int iTarget, int iLimbID, int iMode )
{
	// check the object exists
	if ( !ConfirmObject ( iSource ) )
		return;

	//if ( !ConfirmObjectInstance ( iSource ) )
	//	return;

	// leefix - 100304 - check the object exists (and limb)
	if ( !ConfirmObjectAndLimb ( iTarget, iLimbID ) )
		return;

	//if ( !ConfirmObjectAndLimbInstance ( iTarget, iLimbID ) )
	//	return;

	// get object pointers
	sObject* pSourceObject = g_ObjectList [ iSource ];


	// assign target limb to override source position data
	pSourceObject->position.bGlued			= true;
	pSourceObject->position.iGluedToObj		= iTarget;

	// glue mode
	if ( iMode==1 )
	{
		// mode 1 - wipe out frame orient, leaving position only (avoid hierarchy frame problems)
		pSourceObject->position.iGluedToMesh	= iLimbID * -1;
	}
	else
	{
		if ( iMode==2 )
		{
			// mode 2 - wipe out child object position for accurate limb location placement
			pSourceObject->position.iGluedToMesh	= iLimbID;

			// leeadd - 150306 - u60b3 - reset position
			pSourceObject->position.vecPosition = GGVECTOR3 ( 0, 0, 0 );
		}
		else
		{
			// mode 0 - regular glue object to a limb (default behaviour)
			pSourceObject->position.iGluedToMesh	= iLimbID;
		}
	}

}

DARKSDK_DLL void GlueObjectToLimb ( int iSource, int iTarget, int iLimbID )
{
	GlueObjectToLimbEx ( iSource, iTarget, iLimbID, 0 );
}

DARKSDK_DLL void UnGlueObject ( int iSource )
{
	// check the object exists
	if ( !ConfirmObject ( iSource ) )
		return;

	// get object pointer
	sObject* pSourceObject = g_ObjectList [ iSource ];

	// get target object pointer
	int iTarget = pSourceObject->position.iGluedToObj;
	int iLimbID = abs ( pSourceObject->position.iGluedToMesh );
	if ( iTarget > 0 )
	{
		sObject* pTargetObject = g_ObjectList [ iTarget ];
		if ( pTargetObject )
		{
			// set new position of source to target
			if ( iLimbID < pTargetObject->iFrameCount )
			{
				pSourceObject->position.vecPosition.x	= pTargetObject->ppFrameList [ iLimbID ]->matAbsoluteWorld._41;
				pSourceObject->position.vecPosition.y	= pTargetObject->ppFrameList [ iLimbID ]->matAbsoluteWorld._42;
				pSourceObject->position.vecPosition.z	= pTargetObject->ppFrameList [ iLimbID ]->matAbsoluteWorld._43;
			}
			
			// wipe out glue assignment
			pSourceObject->position.bGlued			= false;
			pSourceObject->position.iGluedToObj		= 0;
			pSourceObject->position.iGluedToMesh	= 0;
		}
	}

}

DARKSDK_DLL void UnGlueAllObjects ( void )
{
	// check the object exists
	for ( int iSource = 0; iSource < g_iObjectListCount; iSource++ )
	{
		if ( !ConfirmObject ( iSource ) )
			return;

		// get object pointer
		sObject* pSourceObject = g_ObjectList [ iSource ];

		// get target object pointer
		int iTarget = pSourceObject->position.iGluedToObj;
		if ( iTarget > 0 )
		{
			sObject* pTargetObject = g_ObjectList [ iTarget ];
			if ( pTargetObject )
			{
				// set new position of source to target
				int iFrameIndex = abs ( pSourceObject->position.iGluedToMesh );
				
				CalcObjectWorld ( pSourceObject );
				CalcObjectWorld ( pTargetObject );

				DWORD x = LimbDirectionX ( iSource, 0 );
				DWORD y = LimbDirectionY ( iSource, 0 );
				DWORD z = LimbDirectionZ ( iSource, 0 );

				float fX = *( float* ) &x;
				float fY = *( float* ) &y;
				float fZ = *( float* ) &z;

				RotateObject ( iSource, fX, fY, fZ );

				pSourceObject->position.vecPosition.x = pSourceObject->position.matWorld._41;
				pSourceObject->position.vecPosition.y = pSourceObject->position.matWorld._42;
				pSourceObject->position.vecPosition.z = pSourceObject->position.matWorld._43;

				// wipe out glue assignment
				pSourceObject->position.bGlued			= false;
				pSourceObject->position.iGluedToObj		= 0;
				pSourceObject->position.iGluedToMesh	= 0;
			}
		}
	}
}

DARKSDK_DLL void LockObjectOn ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// promote to overlay layer
	pObject->bLockedObject = true;
	UpdateOverlayFlag ( pObject );
}

DARKSDK_DLL void LockObjectOff ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// promote to overlay layer
	pObject->bLockedObject = false;
	UpdateOverlayFlag ( pObject );
}

DARKSDK_DLL void DisableObjectZDepthEx	( int iID, int iKeepUpdateStage )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// promote to overlay layer
	if ( iKeepUpdateStage == 1 )
	{
		// only want to ensure object in natural update stage does not write to depth
		sObject* pActualObject = pObject; if ( pActualObject->pInstanceOfObject ) pActualObject = pActualObject->pInstanceOfObject;
		for ( int iMeshIndex = 0; iMeshIndex < pActualObject->iMeshCount; iMeshIndex++ )
		{
			sMesh* pMesh = pActualObject->ppMeshList[iMeshIndex];
			if ( pMesh ) pMesh->bZWrite = false;
		}
	}
	else
	{
		// default which pushes render to new update stage
		pObject->bNewZLayerObject = true;
		UpdateOverlayFlag ( pObject );
	}
}

DARKSDK_DLL void DisableObjectZDepth ( int iID )
{
	DisableObjectZDepthEx ( iID, 0 );
}

DARKSDK_DLL void EnableObjectZDepth ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// promote to overlay layer
	pObject->bNewZLayerObject = false;
	UpdateOverlayFlag ( pObject );
}

DARKSDK_DLL void DisableObjectZRead ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetZRead ( pObject->ppMeshList [ iMesh ], false );
}

DARKSDK_DLL void EnableObjectZRead ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetZRead ( pObject->ppMeshList [ iMesh ], true );
}

DARKSDK_DLL void DisableObjectZWriteEx ( int iID, bool bProtectState )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++)
	{
		SetZWrite(pObject->ppMeshList[iMesh], false);
		pObject->ppMeshList[iMesh]->bProtectZWriteState = bProtectState;
	}
}

DARKSDK_DLL void DisableObjectZWrite(int iID)
{
	DisableObjectZWriteEx(iID,false);
}

DARKSDK_DLL void DisableLimbZWrite ( int iID, int iLimbIndex )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbIndex ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList[iLimbIndex];
	if ( pFrame )
		if ( pFrame->pMesh ) 
			SetZWrite ( pFrame->pMesh, false );
}

DARKSDK_DLL void EnableObjectZWrite ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for (int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++)
		SetZWrite(pObject->ppMeshList[iMesh], true);
}

DARKSDK_DLL void DisableObjectZBias ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetZBias ( pObject->ppMeshList [ iMesh ], false, 0.0f, 0.0f );
}

DARKSDK_DLL void EnableObjectZBias ( int iID, float fSlopeScale, float fDepth )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetZBias ( pObject->ppMeshList [ iMesh ], true, fSlopeScale, fDepth );
}

DARKSDK_DLL void ReverseObjectFrames ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject ) 
	{
		int iFrameCount = pObject->iFrameCount;
		sFrame** pTempList = new sFrame*[iFrameCount];
		if ( pTempList )
		{
			for ( int iFrame=0; iFrame<iFrameCount; iFrame++ )
			{
				pTempList[iFrame] = pObject->ppFrameList[iFrame];
			}
			for ( int iFrame=0; iFrame<iFrameCount; iFrame++ )
			{
				pObject->ppFrameList[iFrameCount-1-iFrame] = pTempList[iFrame];
			}
			delete pTempList;
		}
	}
}

DARKSDK_DLL void SetObjectFOV ( int iID, float fFOV )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to object
	sObject* pObject = g_ObjectList [ iID ];
	pObject->fFOV = fFOV;
}

DARKSDK_DLL void FixObjectPivot ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// update to latest rotation
	UpdateObjectRotation ( pObject );

	// rotation to apply
	GGMATRIX matRotation;
	if ( pObject->position.bFreeFlightRotation )
		matRotation = pObject->position.matFreeFlightRotate;
	else
		matRotation = pObject->position.matRotation;

	// copy rotation to pivot and activate
	if ( pObject->position.bApplyPivot==false )
	{
		// first pivot capture
		pObject->position.bApplyPivot = true;
		pObject->position.matPivot = matRotation;
	}
	else
	{
		// compounded pivot capture
		pObject->position.matPivot = matRotation * pObject->position.matPivot;
	}

	// reset rotation
	pObject->position.bFreeFlightRotation = false;
	pObject->position.vecRotate = GGVECTOR3 ( 0.0f, 0.0f, 0.0f );
	GGMatrixIdentity ( &pObject->position.matRotation );

	// regenerate look vectors
	RegenerateLookVectors( pObject );

}

void ResetObjectPivot ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// reset pivot and anim update flags
	pObject->position.bApplyPivot = false;
	pObject->bAnimUpdateOnce = true;

	// also reset user matrix
	for ( int iLimbID=0; iLimbID<pObject->iFrameCount; iLimbID++ )
	{
		GGMatrixIdentity ( &pObject->ppFrameList[iLimbID]->matUserMatrix );
	}

}

void SetToObjectOrientationEx ( int iID, int iWhichID, int iLimbID, int iMode )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// and possible limb
	if ( iLimbID!=-1 )
		if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
			return;

	// check the object exists
	if ( !ConfirmObject ( iWhichID ) )
		return;

	// get object pointers
	sObject* pObject = g_ObjectList [ iID ];
	sObject* pWhichObject = g_ObjectList [ iWhichID ];

	// copy data from 'which-object'
	if ( iLimbID!=-1 )
	{
		// FROM LIMB
		sFrame* pWhichFrame = pWhichObject->ppFrameList [ iLimbID ];
		if ( pWhichFrame )
		{
			pObject->position.bFreeFlightRotation	= true;
			pObject->position.matFreeFlightRotate	= pWhichFrame->matCombined;
		}
	}
	else
	{
		// FROM OBJECT
		pObject->position.bFreeFlightRotation	= pWhichObject->position.bFreeFlightRotation;
		pObject->position.matFreeFlightRotate	= pWhichObject->position.matFreeFlightRotate;
		pObject->position.dwRotationOrder		= pWhichObject->position.dwRotationOrder;
		pObject->position.vecRotate				= pWhichObject->position.vecRotate;
		pObject->position.matRotation			= pWhichObject->position.matRotation;
		pObject->position.matRotateX			= pWhichObject->position.matRotateX;
		pObject->position.matRotateY			= pWhichObject->position.matRotateY;
		pObject->position.matRotateZ			= pWhichObject->position.matRotateZ;
		
		// mike - 011005 - use or leave pivot
		if ( iMode == 1 )
		{
			pObject->position.bApplyPivot		= pWhichObject->position.bApplyPivot;
			pObject->position.matPivot			= pWhichObject->position.matPivot;
		}

		pObject->position.vecLook				= pWhichObject->position.vecLook;
		pObject->position.vecUp					= pWhichObject->position.vecUp;
		pObject->position.vecRight				= pWhichObject->position.vecRight;
	}

}

DARKSDK_DLL void SetObjectToObjectOrientation ( int iID, int iWhichID )
{
	SetToObjectOrientationEx ( iID, iWhichID, -1, 0 );
}

DARKSDK_DLL void SetObjectToObjectOrientation ( int iID, int iWhichID, int iMode )
{
	SetToObjectOrientationEx ( iID, iWhichID, -1, iMode );
}

DARKSDK_DLL void SetObjectToCameraOrientation ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];

	// get camera pointer
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	// copy data from camera zero
	pObject->position.vecLook				= m_Camera_Ptr->vecLook;
	pObject->position.vecUp					= m_Camera_Ptr->vecUp;
	pObject->position.vecRight				= m_Camera_Ptr->vecRight;
	pObject->position.bFreeFlightRotation	= m_Camera_Ptr->bUseFreeFlightRotation;

    // camera and object free flights are inverse of each other
    // pObject->position.matFreeFlightRotate = m_Camera_Ptr->matFreeFlightRotate;
	FLOAT fDeterminant;
	GGMatrixInverse ( &pObject->position.matFreeFlightRotate, &fDeterminant, &m_Camera_Ptr->matFreeFlightRotate );

	pObject->position.vecRotate.x			= m_Camera_Ptr->fXRotate;
	pObject->position.vecRotate.y			= m_Camera_Ptr->fYRotate;
	pObject->position.vecRotate.z			= m_Camera_Ptr->fZRotate;
	if ( m_Camera_Ptr->bRotate )
		pObject->position.dwRotationOrder		= ROTORDER_XYZ;
	else
		pObject->position.dwRotationOrder		= ROTORDER_ZYX;

	// update object with new rotation
}

// Texture commands

DARKSDK_DLL void TextureObjectRef ( int iID, LPGGSHADERRESOURCEVIEW pTextureRef, float fClipU, float fClipV )
{
	// check the object exists
	g_pGlob->dwInternalFunctionCode=12001;
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		SetBaseTextureStageRef ( pMesh, 0, pTextureRef );

		// get the offset map for the FVF
		sOffsetMap offsetMap;
		GetFVFOffsetMap ( pMesh, &offsetMap );

		// change the UV offsets
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * 0 ) ) = fClipU;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * 2 ) ) = fClipU;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * 2 ) ) = fClipV;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * 4 ) ) = fClipV;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * 5 ) ) = fClipU;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * 5 ) ) = fClipV;

		// flag mesh for a VB update
		pMesh->bVBRefreshRequired=true;
		g_vRefreshMeshList.push_back ( pMesh );
	}
}

DARKSDK_DLL void TextureObject ( int iID, int iImage )
{
	// check the object exists
	g_pGlob->dwInternalFunctionCode=12001;
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetBaseTexture ( pObject->ppMeshList [ iMesh ], -1, iImage );
	
	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
	g_pGlob->dwInternalFunctionCode=12002;

}

DARKSDK_DLL void TextureObject ( int iID, int iStage, int iImage )
{
	SetObjectTextureStageEx ( iID, iStage, iImage, 0 );
}

DARKSDK_DLL void SetObjectTextureStageEx ( int iID, int iStage, int iImage, int iDoNotSortTextures )
{
	// check the object exists
	g_pGlob->dwInternalFunctionCode=11001;
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	g_pGlob->dwInternalFunctionCode=11011;
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetBaseTextureStage ( pObject->ppMeshList [ iMesh ], iStage, iImage );
	
	// trigger a ew-new
	g_pGlob->dwInternalFunctionCode=11022;
	m_ObjectManager.RenewReplacedMeshes ( pObject );

	// res-sort textures only if flagged
	if ( iDoNotSortTextures==0 ) m_ObjectManager.UpdateTextures ( );
	g_pGlob->dwInternalFunctionCode=11023;

}

DARKSDK_DLL void ScrollObjectTexture ( int iID, int iStage, float fU, float fV )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		ScrollTexture ( pObject->ppMeshList [ iMesh ], iStage, fU, fV );

	// lee - 130206 - update 'original' data to reflect this UV change
	UpdateVertexDataInMesh ( pObject );
}

DARKSDK_DLL void ScrollObjectTexture ( int iID, float fU, float fV )
{
	// refers to core function above
	ScrollObjectTexture ( iID, 0, fU, fV );
}

DARKSDK_DLL void ScaleObjectTexture ( int iID, int iStage, float fU, float fV )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// if object has effect applied, skip UV vertex write and pass into vars for later use when setting effect constant
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject==NULL ) return;

	cSpecialEffect* pEff = NULL;
	if ( pObject->ppMeshList[0] ) pEff = pObject->ppMeshList[0]->pVertexShaderEffect;
	if ( pEff!=NULL )
	{
		// apply to all meshes
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			if ( pMesh )
			{
				pMesh->fUVScalingU = fU;
				pMesh->fUVScalingV = fV;
			}
		}
	}
	else
	{
		// apply to all meshes
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			ScaleTexture ( pObject->ppMeshList [ iMesh ], iStage, fU, fV );

		// lee - 130206 - update 'original' data to reflect this UV change
		UpdateVertexDataInMesh ( pObject );
	}
}

DARKSDK_DLL void ScaleObjectTexture ( int iID, float fU, float fV )
{
	// refers to core function above
	ScaleObjectTexture ( iID, 0, fU, fV );
}

DARKSDK_DLL void SetObjectUVManually ( int iObjID, int iFrameIndex, float fWidth, float fHeight )
{
	// set UV data manually for now
	float U_f = 0;
	float V_f = 0;
	float USize_f = 1.0f / fWidth;
	float VSize_f = 1.0f / fHeight;
	if ( iFrameIndex != 0 )
	{
		int across = int(iFrameIndex/fWidth);
		V_f = VSize_f * across;
		U_f = iFrameIndex * USize_f;
	}
	LockVertexDataForLimb ( iObjID, 0 );
	SetVertexDataUV(0, U_f, V_f);
	SetVertexDataUV(1, U_f + USize_f, V_f);
	SetVertexDataUV(2, U_f + USize_f, V_f + VSize_f);
	SetVertexDataUV(3, U_f + USize_f, V_f + VSize_f);
	SetVertexDataUV(4, U_f, V_f + VSize_f);
	SetVertexDataUV(5, U_f, V_f);
	UnlockVertexData();

}

DARKSDK_DLL void SetObjectSmoothing ( int iID, float fPercentage )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// limit percentage range
	if ( fPercentage<0.0f ) fPercentage=0.0f;
	if ( fPercentage>100.0f ) fPercentage=100.0f;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SmoothNormals ( pObject->ppMeshList [ iMesh ], fPercentage/100.0f );
}

DARKSDK_DLL void SetObjectNormalsEx ( int iID, int iMode )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		GenerateNormals ( pObject->ppMeshList [ iMesh ], iMode );

	// lee - 130206 - update 'original' data to reflect this UV change
	UpdateVertexDataInMesh ( pObject );
}

DARKSDK_DLL void SetObjectNormals ( int iID )
{
	SetObjectNormalsEx ( iID, 0 );
}

DARKSDK_DLL void SetObjectTextureModeStage ( int iID, int iStage, int iMode, int iMipGeneration )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply setting to all meshes (for one stage only)
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetTextureMode ( pObject->ppMeshList [ iMesh ], iStage, iMode, iMipGeneration );
}

DARKSDK_DLL void SetObjectTextureMode ( int iID, int iMode, int iMipGeneration )
{
	// iMode (AddressU and AddressV)
	// D3DTADDRESS_WRAP = 1
    // D3DTADDRESS_MIRROR = 2
    // GGTADDRESS_CLAMP = 3
    // D3DTADDRESS_BORDER = 4
    // D3DTADDRESS_MIRRORONCE = 5
	// iMipGeneration (MipStage)
	// GGTEXF_NONE = 0
	// D3DTEXF_POINT = 1
	// GGTEXF_LINEAR = 2
	// D3DTEXF_ANISOTROPIC = 3
	// D3DTEXF_PYRAMIDALQUAD = 6
	// D3DTEXF_GAUSSIANQUAD = 7
	SetObjectTextureModeStage ( iID, 0, iMode, iMipGeneration );
}

DARKSDK_DLL void SetObjectLightMap ( int iID, int iImage, int iAddDIffuseToStageZero )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// added for the benefit of FPSXC104RC7 (used for darklight replacement to built-in lightmapper)
	if ( iAddDIffuseToStageZero==0 )
	{
		// regular light mapping blend
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			SetMultiTexture ( pObject->ppMeshList [ iMesh ], 1, GGTOP_MODULATE, 3, iImage );
	}
	else
	{
		// the idea is we want DIFFUSE+lightmap in stage zero, and texture on stage two
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			if ( pMesh->dwTextureCount>=2 )
			{
				// switch them 
				pMesh->pTextures [ 0 ].dwBlendMode		= GGTOP_ADD;
				pMesh->pTextures [ 0 ].dwBlendArg1		= GGTA_TEXTURE;
				pMesh->pTextures [ 0 ].dwBlendArg2		= GGTA_DIFFUSE;
				pMesh->pTextures [ 1 ].dwBlendMode		= GGTOP_MODULATE;
				pMesh->pTextures [ 1 ].dwBlendArg1		= GGTA_TEXTURE;
				pMesh->pTextures [ 1 ].dwBlendArg2		= GGTA_CURRENT;
			}
		}
	}

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectLightMap ( int iID, int iImage )
{
	SetObjectLightMap ( iID, iImage, 0 );
}

DARKSDK_DLL void SetObjectSphereMap ( int iID, int iSphereImage )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetMultiTexture ( pObject->ppMeshList [ iMesh ], 1, GGTOP_MODULATE, 1, iSphereImage );

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectCubeMapStage ( int iID, int iStage, int i1, int i2, int i3, int i4, int i5, int i6 )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// generate cube map 
	LPGGCUBETEXTURE pCubeTexture = NULL;
	pCubeTexture = CreateNewImageCubeMap ( i1, i2, i3, i4, i5, i6 );

	// These six images are sources for a cubemap, so if the images are used as
	// camera render targets, we can use a cube face instead

	SetCubeFace ( i1, pCubeTexture, 0 );
	SetCubeFace ( i2, pCubeTexture, 1 );
	SetCubeFace ( i3, pCubeTexture, 2 );
	SetCubeFace ( i4, pCubeTexture, 3 );
	SetCubeFace ( i5, pCubeTexture, 4 );
	SetCubeFace ( i6, pCubeTexture, 5 );

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];

		// apply cube map reference to mesh
		SetCubeTexture ( pMesh, iStage, pCubeTexture );

		// U75 - 120809 - also, if currently using shader, ensure this dynamic cube map is allowed in manager
		if ( pMesh->pVertexShaderEffect )
		{
			DWORD dwCorrectBitForThisStage = 1 << iStage;
			pMesh->pVertexShaderEffect->m_dwUseDynamicTextureMask = pMesh->pVertexShaderEffect->m_dwUseDynamicTextureMask | dwCorrectBitForThisStage;
		}
	}

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectCubeMap ( int iID, int i1, int i2, int i3, int i4, int i5, int i6 )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// Cube is on stage one
	int iStage = 1;

	// generate cube map 
	LPGGCUBETEXTURE pCubeTexture = NULL;
	pCubeTexture = CreateNewImageCubeMap ( i1, i2, i3, i4, i5, i6 );

	// These six images are sources for a cubemap, so if the images are used as
	// camera render targets, we can use a cube face instead

	SetCubeFace ( i1, pCubeTexture, 0 );
	SetCubeFace ( i2, pCubeTexture, 1 );
	SetCubeFace ( i3, pCubeTexture, 2 );
	SetCubeFace ( i4, pCubeTexture, 3 );
	SetCubeFace ( i5, pCubeTexture, 4 );
	SetCubeFace ( i6, pCubeTexture, 5 );

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];

		// apply cube map reference to mesh
		SetCubeTexture ( pMesh, iStage, pCubeTexture );

		// U75 - 120809 - also, if currently using shader, ensure this dynamic cube map is allowed in manager
		if ( pMesh->pVertexShaderEffect )
		{
			DWORD dwCorrectBitForThisStage = 1 << iStage;
			pMesh->pVertexShaderEffect->m_dwUseDynamicTextureMask = pMesh->pVertexShaderEffect->m_dwUseDynamicTextureMask | dwCorrectBitForThisStage;
		}
	}

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}


DARKSDK_DLL void SetObjectDetailMap ( int iID, int iImage )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetMultiTexture ( pObject->ppMeshList [ iMesh ], 1, GGTOP_ADD, 3, iImage );

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectBlendMap ( int iID, int iLimbID, int iStage, int iImage, int iTexCoordMode, int iMode, int iA, int iB, int iC, int iR )
{
	// BLEND MODE PARAMS
	// iStage 0-7
	// iImage Index
	// iTexCoordMode
	// 0 - Regular UV Stage Match
	// 1 - Sphere Mapping UV Data
	// 2 - Cube Mapping UV Data
	// 3 - Steal UV Data From Stage Zero
	// 10-17 - Take UV Data From Stage.. (10=0,11-1,etc)
	// iBlendMode
	// GGTOP_DISABLE = 1,
	// GGTOP_SELECTARG1 = 2,
	// D3DTOP_SELECTARG2 = 3,
	// GGTOP_MODULATE = 4,
	// GGTOP_MODULATE2X = 5,
	// GGTOP_MODULATE4X = 6,
	// GGTOP_ADD = 7,
	// GGTOP_ADDSIGNED = 8,
	// GGTOP_ADDSIGNED2X = 9,
	// D3DTOP_SUBTRACT = 10,
	// GGTOP_ADDSMOOTH = 11,
	// D3DTOP_BLENDDIFFUSEALPHA = 12,
	// D3DTOP_BLENDTEXTUREALPHA = 13,
	// D3DTOP_BLENDFACTORALPHA = 14,
	// D3DTOP_BLENDTEXTUREALPHAPM = 15,
	// D3DTOP_BLENDCURRENTALPHA = 16,
	// D3DTOP_PREMODULATE = 17,
	// GGTOP_MODULATEALPHA_ADDCOLOR = 18,
	// GGTOP_MODULATECOLOR_ADDALPHA = 19,
	// GGTOP_MODULATEINVALPHA_ADDCOLOR = 20,
	// GGTOP_MODULATEINVCOLOR_ADDALPHA = 21,
	// D3DTOP_BUMPENVMAP = 22,
	// D3DTOP_BUMPENVMAPLUMINANCE = 23,
	// D3DTOP_DOTPRODUCT3 = 24,
	// D3DTOP_MULTIPLYADD = 25,
	// D3DTOP_LERP = 26,
	// iA, iB, iC, iR: D3DTA's : default is GGTA_TEXTURE,GGTA_CURRENT,-1,GGTA_CURRENT
	// iForceArgX
	// [forcably change the COLORARG values]
	// GGTA_DIFFUSE = 0          0x00000000  // select diffuse color (read only)
	// GGTA_CURRENT = 1          0x00000001  // select stage destination register (read/write)
	// GGTA_TEXTURE = 2          0x00000002  // select texture color (read only)
	// D3DTA_TFACTOR = 3          0x00000003  // select D3DRS_TEXTUREFACTOR (read only)
	// D3DTA_SPECULAR = 4         0x00000004  // select specular color (read only)
	// D3DTA_TEMP = 5             0x00000005  // select temporary register color (read/write)
	// D3DTA_CONSTANT = 6         0x00000006  // select texture stage constant
	// D3DTA_COMPLEMENT = 16      0x00000010  // take 1.0 - x (read modifier)
	// D3DTA_ALPHAREPLICATE = 32  0x00000020  // replicate alpha to color components (read modifier)

	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// U74 - 080509 - is limb number ok
	if ( iLimbID!=-1 )
		if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
			return;

	// apply to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	if ( iLimbID==-1 )
	{
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			SetMultiTexture ( pObject->ppMeshList [ iMesh ], iStage, (DWORD)iMode, iTexCoordMode, iImage );

		// U73 - 210309 - apply D3DTA values to all meshes (if applicable)
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			if ( iA!=-1 ) pObject->ppMeshList [ iMesh ]->pTextures [ iStage ].dwBlendArg1 = iA;
			if ( iB!=-1 ) pObject->ppMeshList [ iMesh ]->pTextures [ iStage ].dwBlendArg2 = iB;
			if ( iC!=-1 ) pObject->ppMeshList [ iMesh ]->pTextures [ iStage ].dwBlendArg0 = iC;
			if ( iR!=-1 ) pObject->ppMeshList [ iMesh ]->pTextures [ iStage ].dwBlendArgR = iR;
		}
	}
	else
	{
		// U74 - 080509 - single limb
		sFrame* pFrame = pObject->ppFrameList[iLimbID];
		if ( pFrame )
		{
			if ( pFrame->pMesh )
			{
				SetMultiTexture ( pFrame->pMesh, iStage, (DWORD)iMode, iTexCoordMode, iImage );
				if ( iA!=-1 ) pFrame->pMesh->pTextures [ iStage ].dwBlendArg1 = iA;
				if ( iB!=-1 ) pFrame->pMesh->pTextures [ iStage ].dwBlendArg2 = iB;
				if ( iC!=-1 ) pFrame->pMesh->pTextures [ iStage ].dwBlendArg0 = iC;
				if ( iR!=-1 ) pFrame->pMesh->pTextures [ iStage ].dwBlendArgR = iR;
			}
		}
	}

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void SetObjectBlendMap ( int iID, int iStage, int iImage, int iTexCoordMode, int iMode, int iA, int iB, int iC, int iR )
{
	// U75 - 080509 - default all meshes, specify -1 for mesh param
	SetObjectBlendMap ( iID, -1, iStage, iImage, iTexCoordMode, iMode, iA, iB, iC, iR );
}

DARKSDK_DLL void SetObjectBlendMap ( int iID, int iStage, int iImage, int iTexCoordMode, int iMode )
{
	// default blend mapping command up to U73 - 210309 - replaced with larger blending command for lerping
	SetObjectBlendMap ( iID, iStage, iImage, iTexCoordMode, iMode, -1, -1, -1, -1 );
}

DARKSDK_DLL void SetBlendMap ( int iID, int iImage, int iMode )
{
	SetObjectBlendMap ( iID, 1, iImage, 3, iMode );
}

DARKSDK_DLL void SetTextureMD3 ( int iID, int iH0, int iH1, int iL0, int iL1, int iL2, int iU0 )
{
	// MIKEMIKE : Fits in with MD3 format of what DBO will make of it..[MD3]
}

DARKSDK_DLL int SwitchRenderTargetToDepthTexture ( int iFlag )
{
	return m_ObjectManager.SwitchRenderTargetToDepth(iFlag);
}


DARKSDK_DLL void SetObjectDebugInfo(int iID, DWORD value)
{
	// check the object exists
	if (!ConfirmObjectInstance(iID))
		return;

	// get object ptr
	sObject* pActualObject = g_ObjectList[iID];
	pActualObject->dwReservedR4 = value;
}

// New Texture Functions

DARKSDK_DLL void SetAlphaMappingOn ( int iID, float fPercentage, bool bForceUnTransparency )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object ptr
	sObject* pActualObject = g_ObjectList [ iID ];

	// 060217 - need to remember transparency state when not 100%
	if ( fPercentage != 100.0f )
	{
		if ( pActualObject->dwRememberTransparencyState == 0 )
		{
			if ( pActualObject->bTransparentObject == false ) 
				pActualObject->dwRememberTransparencyState  = 1;
			else
				pActualObject->dwRememberTransparencyState  = 2;
		}
	}

	// new mode to apply color for highlighting 
	if ( fPercentage > 100.0f )
	{
		GGCOLOR dwColorValueOnly = GGCOLOR_ARGB ( 0, 0, 0, 0 );
		if ( fPercentage < 110.0f )
		{
			if ( fPercentage <= 102.0f )
			{
				if ( fPercentage==101.0f )
				{
					dwColorValueOnly = GGCOLOR_ARGB ( 255, 128, 0, 0 );
				}
				else
				{
					dwColorValueOnly = GGCOLOR_ARGB ( 255, 128, 0, 64 );
				}
			}
			else
			{
				if ( fPercentage==103.0f )
				{
					dwColorValueOnly = GGCOLOR_ARGB ( 255, 0, 128, 0 );
				}
					//cyb
					if (fPercentage == 104.0f)
					{
						dwColorValueOnly = GGCOLOR_ARGB(255, 0, 128, 64);
					}
					else
					{
						dwColorValueOnly = GGCOLOR_ARGB(255, 128, 107, 0);  //'gold'
					}
			}
			pActualObject->dwInstanceAlphaOverride = dwColorValueOnly;
			pActualObject->bInstanceAlphaOverride = true;
			pActualObject->bTransparentObject = true;
		}
		if ( fPercentage == 110.0f ) fPercentage = 52.0f;
		pActualObject->fArtificialDistanceOffset = 50000.0f;
	}

	// or set true alpha transparency
	if ( fPercentage <= 100.0f )
	{
		// reset extra color feature if back to 100
		if ( fPercentage == 100.0f )
		{
			pActualObject->dwInstanceAlphaOverride = 0;
			pActualObject->bInstanceAlphaOverride = false;

			// 261115 - cannot set to nontransparent as this may have been an explosion/fading decal and
			// must remain transparent for semi-transparent render ordering!
			if ( bForceUnTransparency == true )
			{
				// flagged when dehighlighting in editor (so non transparents can be rendered BEHIND transparent ones, EBE)
				pActualObject->bTransparentObject = false;
			}
			else
			{
				if ( pActualObject->dwRememberTransparencyState == 1 ) pActualObject->bTransparentObject = false;
				if ( pActualObject->dwRememberTransparencyState == 2 ) pActualObject->bTransparentObject = true;
			}
			pActualObject->dwRememberTransparencyState = 0;

			// 060217 - ensure we restore artificialdistanceoffset
			pActualObject->fArtificialDistanceOffset = 0;
		}

		// apply alpha factor
		if ( pActualObject->pInstanceOfObject )
		{
			// direct alpha factor effect on instance
			if ( fPercentage!=100.0f )
			{
				// some level of alpha, make transparent and set alpha value
				fPercentage/=100.0f;
				DWORD dwAlpha = (DWORD)(fPercentage*255);
				GGCOLOR dwAlphaValueOnly = GGCOLOR_ARGB ( dwAlpha, 0, 0, 0 );
				pActualObject->dwInstanceAlphaOverride = dwAlphaValueOnly;
				pActualObject->bInstanceAlphaOverride = true;
				pActualObject->bTransparentObject = true;
			}
		}
		else
		{
			// apply to all meshes
			for ( int iMesh = 0; iMesh < pActualObject->iMeshCount; iMesh++ )
			{
				SetAlphaOverride ( pActualObject->ppMeshList [ iMesh ], fPercentage );
				SetTransparency ( pActualObject->ppMeshList [ iMesh ], true );
			}
		}

	}
}

DARKSDK_DLL void SetAlphaMappingOn ( int iID, float fPercentage )
{
	SetAlphaMappingOn ( iID, fPercentage, false );
}

/*
DARKSDK_DLL void SetObjectEffectOn ( int iID, SDK_LPSTR pFilename, int iUseDefaultTextures )
{
	// check the object exists or not
	bool bUseDefaultModel = false;
	if ( !CheckObjectExist ( iID ) )
	{
		// create blank object to hold 'default model'
		MakeObjectPyramid ( iID, 1.0f );
		bUseDefaultModel = true;
	}

	// determine if we should use default textures
	bool bUseDefaultTextures = false;
	if ( iUseDefaultTextures==1 )
		bUseDefaultTextures = true;

	// Create external effect obj
	cSpecialEffect* pEffectObj = new cExternalEffect;
	pEffectObj->Load ( (char*)pFilename, bUseDefaultModel, bUseDefaultTextures );

	// reset vertex data before apply special effect
	sObject* pObject = g_ObjectList [ iID ];
	ResetVertexDataInMesh ( pObject );

	// apply to all meshes
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// get mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];

		// apply unique effect
		if ( !SetSpecialEffect ( pMesh, pEffectObj ) )
		{
			// failed to setup effect
			if ( bUseDefaultModel ) DeleteObjectSpecial ( iID );
			RunTimeError ( RUNTIMEERROR_B3DVSHADERCANNOTCREATE );
			return;
		}

		// first mesh is original, rest are references only
		if ( iMesh==0 )
			pMesh->bVertexShaderEffectRefOnly = false;
		else
			pMesh->bVertexShaderEffectRefOnly = true;
	}

	// full or quick update of object
	if ( bUseDefaultModel )
	{
		// setup new object and introduce to buffers
		SetNewObjectFinalProperties ( iID, -1.0f );
	}
	else
	{
		// as shader recreates mesh format, must regenerate buffer instance
		m_ObjectManager.RemoveObjectFromBuffers ( pObject );
		m_ObjectManager.AddObjectToBuffers ( pObject );
	}
}
*/

// FX effects commands

DARKSDK_DLL void LoadEffectEx ( LPSTR pFilename, int iEffectID, int iUseDefaultTextures, int iDoNotGenerateExtraData )
{
	// check the effect exists
	if ( !ConfirmNewEffect ( iEffectID ) )
		return;

	// determine if file even exists
	if ( !DoesFileExist ( (LPSTR)pFilename ) )
	{
		RunTimeError ( RUNTIMEERROR_FILENOTEXIST );
		return;
	}

	// determine if we should use default textures
	bool bUseDefaultTextures = false;
	if ( iUseDefaultTextures==1 )
		bUseDefaultTextures = true;

	// load the effect into array
	m_EffectList [ iEffectID ] = new sEffectItem;
	if ( m_EffectList [ iEffectID ]->pEffectObj )
	{
		// assign generate extra data flag then load the effect
		m_EffectList [ iEffectID ]->pEffectObj->m_dwEffectIndex = iEffectID;
		m_EffectList [ iEffectID ]->pEffectObj->m_bDoNotGenerateExtraData = (DWORD)iDoNotGenerateExtraData;
		m_EffectList [ iEffectID ]->pEffectObj->m_bNeed8BonesPerVertex = false;
		if ( !m_EffectList [ iEffectID ]->pEffectObj->Load ( iEffectID, (char*)pFilename, false, bUseDefaultTextures ) )
		{
			// leefix - 200906 - u63 - if effect failed, still keep it in mem for delete effect clearup (can still checklist for errors on it)
			//SAFE_DELETE ( m_EffectList [ iEffectID ] );
		}
	}
}

DARKSDK_DLL void LoadEffect ( LPSTR pFilename, int iEffectID, int iUseDefaultTextures )
{
	// default call generates extra data such as normals, tangents/binormals, etc
	LoadEffectEx ( pFilename, iEffectID, iUseDefaultTextures, 0 );
}

DARKSDK_DLL void DeleteEffectCore ( int iEffectID, bool bAlsoEraseObjReferences )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// decouple depth render system if used
	sEffectItem* pEffectItem = m_EffectList [ iEffectID ];
	if ( pEffectItem )
	{
		if ( pEffectItem->pEffectObj )
		{
			if ( g_pMainCameraDepthEffect==pEffectItem->pEffectObj->m_pEffect )
			{
				g_pMainCameraDepthEffect = NULL;
				g_pMainCameraDepthHandle = NULL;
			}
		}
	}

	// check all other objects that reference this effect, if they reference - it then remove the link so it wont cause a crash later on
	if ( bAlsoEraseObjReferences == true )
	{
		for ( DWORD dwObject = 0; dwObject < (DWORD)g_iObjectListCount; dwObject++ )
		{
			sObject* pObject = g_ObjectList [ dwObject ];
			if ( pObject )
			{
				for ( DWORD dwMesh = 0; dwMesh < (DWORD)pObject->iMeshCount; dwMesh++ )
				{
					if ( pObject->ppMeshList [ dwMesh ]->pVertexShaderEffect == m_EffectList [ iEffectID ]->pEffectObj )
					{
						strcpy ( pObject->ppMeshList [ dwMesh ]->pEffectName, "" );
						pObject->ppMeshList [ dwMesh ]->pVertexShaderEffect = NULL;
						pObject->ppMeshList[dwMesh]->dl_lights = NULL;
						pObject->ppMeshList[dwMesh]->dl_lightsVS = NULL;
						pObject->ppMeshList[dwMesh]->dl_pos[0] = NULL;
						pObject->ppMeshList[dwMesh]->dl_diffuse[0] = NULL;
						pObject->ppMeshList[dwMesh]->dl_angle[0] = NULL;
					}
				}
			}
		}
	}

	// also remove from SETUP-shortlist
	if ( SETUPFreeShader ( iEffectID ) == true )
	{
		// above deleted effect, so wipe effect ptr here
		m_EffectList [ iEffectID ] = NULL;
	}
	else
	{
		// delete effect from array
		SAFE_DELETE ( m_EffectList [ iEffectID ] );
	}

	// 210917 - also remove from effectparam list
	if ( iEffectID < EFFECT_INDEX_SIZE )
		SAFE_DELETE ( g_CascadedShadow.m_pEffectParam [ iEffectID ] );
}

DARKSDK_DLL void DeleteEffect ( int iEffectID )
{
	DeleteEffectCore ( iEffectID, true );
}

// globals for now
#include "ShadowMapping\cShadowMaps.h"
int							g_PrimaryShadowEffect = 0;
int							g_iDebugObjStart = 0;
int							g_iDebugEffectIndex = 0;
int							g_HideDistantShadows = 1;
int							g_TerrainShadows = 0;
int							g_RealShadowResolution = 1024;
CascadedShadowsManager      g_CascadedShadow;
CascadeConfig               g_CascadeConfig;
bool                        g_bMoveLightTexelSize = TRUE;
CFirstPersonCamera          g_ViewerCamera;          
CFirstPersonCamera          g_LightCamera;         
CFirstPersonCamera*         g_pActiveCamera = &g_ViewerCamera;

DARKSDK_DLL void SetEffectToShadowMappingEx ( int iEffectID, int iDebugObjStart, int iDebugEffectIndex, int iHideDistantShadows, int iTerrainShadows, int iRealShadowResolution, int iRealShadowCascadeCount, int iC0, int iC1, int iC2, int iC3, int iC4, int iC5, int iC6, int iC7 )
{
	// setup effect to support shadow mapping
	g_PrimaryShadowEffect = iEffectID;
	if ( iDebugObjStart > 0 )
	{
		g_iDebugObjStart = iDebugObjStart;
		g_iDebugEffectIndex = iDebugEffectIndex;
	}
	g_HideDistantShadows = iHideDistantShadows;
	g_TerrainShadows = iTerrainShadows;
	g_RealShadowResolution = iRealShadowResolution;
    g_CascadeConfig.m_iBufferSize = g_RealShadowResolution;
    g_CascadeConfig.m_nCascadeLevels = iRealShadowCascadeCount;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[0] = iC0;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[1] = iC1;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[2] = iC2;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[3] = iC3;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[4] = iC4;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[5] = iC5;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[6] = iC6;
    g_CascadedShadow.m_iCascadePartitionsZeroToOne[7] = iC7;
    g_CascadedShadow.m_iCascadePartitionsMax = 100;
    SHADOW_TEXTURE_FORMAT sbt = (SHADOW_TEXTURE_FORMAT)0;
    g_CascadeConfig.m_ShadowBufferFormat = sbt;
    g_CascadedShadow.m_bMoveLightTexelSize = g_bMoveLightTexelSize;
    g_CascadedShadow.m_eSelectedCascadesFit = FIT_TO_SCENE; 
    g_CascadedShadow.m_eSelectedNearFarFit = FIT_NEARFAR_SCENE_AABB;
	g_CascadedShadow.m_fBlurBetweenCascadesAmount = 0.25f;

	// create resources for shadow mapper
    g_CascadedShadow.Init(	&g_ViewerCamera, &g_LightCamera, &g_CascadeConfig );

	// cascade render mask (upto eight cascades)
	g_CascadedShadow.m_dwMask = 0xF;

	// complete
	return;
}

DARKSDK_DLL void ChangeShadowMappingPrimary ( int iEffectID )
{
	// when switch from two terrain shaders (PBR and NONPBR), ensure shadow creaiton tied to active one
	g_PrimaryShadowEffect = iEffectID;
}

DARKSDK_DLL void SetEffectToShadowMapping ( int iEffectID )
{
	SetEffectToShadowMappingEx ( iEffectID, 0, 0, 1, 0, 1024, 4, 2, 8, 16, 75, 100, 100, 100, 100 );
}

DARKSDK_DLL void SetEffectShadowMappingMode ( int iMode )
{
	// Can set the mask for which cascades get rendered
	g_CascadedShadow.m_dwMask = iMode;
}

DARKSDK_DLL void SetShadowTexelSize(int isize)
{
	// Can set the size of the cascade textures use, to calculate the texel size.
	g_CascadeConfig.m_iBufferSize = isize;
}

DARKSDK_DLL void RenderEffectShadowMapping ( int iEffectID )
{
	// renders shadow maps for effect

	// check the effect exists
	if ( iEffectID!=0 )
		if ( !ConfirmEffect ( iEffectID ) )
			return;

	// effect pointer
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// primary shadow map must produce the shadow
	if ( iEffectID==g_PrimaryShadowEffect )
	{
		// assign this effect as primary
		pEffectObject->m_bPrimaryEffectForCascadeShadowMapping = true;

		// set a clear color
		FLOAT ClearColor[4] = { 0.0f, 0.25f, 0.25f, 0.55f };

		// process shadow mapping frame
		g_CascadedShadow.InitFrame( pEffectPtr );

		// set technique for depth rendering
		#ifdef DX11
		GGTECHNIQUEHANDLE hOldTechnique = pEffectObject->m_hCurrentTechnique;
		if ( pEffectPtr )
		{
			pEffectObject->m_hCurrentTechnique = pEffectPtr->GetTechniqueByName ( "DepthMap" );
		}
		#else
		GGHANDLE hOldTechnique = pEffectPtr->GetCurrentTechnique();
		if ( pEffectPtr )
		{
			GGHANDLE hTechnique = pEffectPtr->GetTechniqueByName ( "DepthMap" );
			if ( hTechnique )
				pEffectPtr->SetTechnique(hTechnique);
		}
		#endif

		// render all shadows in cascades (multiple shadow maps based on frustrum slices)
		g_CascadedShadow.RenderShadowsForAllCascades(pEffectPtr);

		// Restore technique after depth renders
		#ifdef DX11
		if ( pEffectPtr && hOldTechnique )
		{
			pEffectObject->m_hCurrentTechnique = hOldTechnique;
		}
		#else
		if ( pEffectPtr && hOldTechnique )
			pEffectPtr->SetTechnique(hOldTechnique);
		#endif
	}
	// set shaodw mapping settings for final render (for all effects that call this command inc. primary)
    g_CascadedShadow.RenderScene( iEffectID, pEffectPtr, NULL, NULL, NULL, false );

	// create debug objects to view shadow maps
	if ( g_iDebugObjStart > 0 )
	{
		if ( ObjectExist ( g_iDebugObjStart+0 ) == 0 )
		{
			int iShadowDebugObj = g_iDebugObjStart+0;
			MakeObjectBox ( iShadowDebugObj, 1024, 1024, 0.1f );
			SetObjectLight ( iShadowDebugObj, 1 );
			LockObjectOn ( iShadowDebugObj );
			SetObjectEffect(iShadowDebugObj, g_iDebugEffectIndex);
			PositionObject ( iShadowDebugObj, 0, 0, 1155 );
			DisableObjectZRead ( iShadowDebugObj );
			HideObject ( iShadowDebugObj );
		}
	}

	// complete
	return;
}

DARKSDK_DLL void SetDefaultCPUAnimState ( int iCPUAnimMode )
{
	g_iDefaultCPUAnimState = iCPUAnimMode;
}

DARKSDK_DLL void SetObjectEffectCore ( int iID, int iEffectID, int iEffectNoBoneID, int iForceCPUAnimationMode )
{
	// iForceCPUAnimationMode:
	// 0-USE DEFAULT SETTING (0=GPU/CPU or 3=strictly GPU only)
	// 1-Force CPU Animation
	// 2-Anim + Hide all meshes with no bone data
	// 3-Force GPU Animation Only (prevents ALL CPU animations for performance)
	if ( iForceCPUAnimationMode == 0 ) iForceCPUAnimationMode = g_iDefaultCPUAnimState;

	// check the object exists
	g_pGlob->dwInternalFunctionCode=10001;
	if ( !ConfirmObject ( iID ) )
		return;

	// check the effect exists
	if ( iEffectID!=0 )
		if ( !ConfirmEffect ( iEffectID ) )
			return;

	// check the effectnobone exists
	if ( iEffectNoBoneID!=0 )
		if ( !ConfirmEffect ( iEffectNoBoneID ) )
			return;

	// get object ptr
	g_pGlob->dwInternalFunctionCode=10002;
	sObject* pObject = g_ObjectList [ iID ];

	// leefix - 040805 - if object ALREADY has effect, must remove it first
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		SetSpecialEffect ( pMesh, NULL );
	}

	// reset vertex data before apply special effect
	ResetVertexDataInMesh ( pObject );

	// remove effect if zero
	if ( iEffectID>0 )
	{
		// apply to specific mesh
		g_pGlob->dwInternalFunctionCode=11000;
		sEffectItem* pEffectItem = m_EffectList [ iEffectID ];

		sEffectItem* pEffectNoBoneItem = NULL;
		if ( iEffectNoBoneID>0 ) pEffectNoBoneItem = m_EffectList [ iEffectNoBoneID ];

		// 131018 - check if any of the meshes require 8 bones per vertex
		// if they do, the whole model needs to use the 8 bone system
		bool bUses8BonePerVertexSystem = false;
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			if ( bUses8BonePerVertexSystem == false )
				if ( CheckIfNeedExtraBonesPerVertices ( pMesh ) == true )
					bUses8BonePerVertexSystem = true;
		}
		pEffectItem->pEffectObj->m_bNeed8BonesPerVertex = bUses8BonePerVertexSystem;
		if ( pEffectNoBoneItem != NULL ) pEffectNoBoneItem->pEffectObj->m_bNeed8BonesPerVertex = bUses8BonePerVertexSystem;

		// apply setting to all meshes
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			// for each mesh
			g_pGlob->dwInternalFunctionCode=11001+iMesh;
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];

			// if nobone effect index submitted, means we want to assign two effect types based on whether mesh has bones
			bool bShaderApplySuccess = false;
			if ( iEffectNoBoneID>0 && pEffectNoBoneItem!=NULL && pMesh->dwBoneCount==0 )
			{
				// apply specific NON-BONE effect passed in as optional parameter
				bShaderApplySuccess = SetSpecialEffect ( pMesh, pEffectNoBoneItem->pEffectObj );
			}
			else
			{
				// apply REGULAR (BONE) shared effect
				bShaderApplySuccess = SetSpecialEffect ( pMesh, pEffectItem->pEffectObj );
			}
			if ( bShaderApplySuccess==true )
			{
				pMesh->bVertexShaderEffectRefOnly = true;
				pMesh->dwForceCPUAnimationMode = (DWORD)iForceCPUAnimationMode;
				if ( pMesh->dwForceCPUAnimationMode == 2 ) pMesh->dwForceCPUAnimationMode = 1;
				if ( iForceCPUAnimationMode==2 && pMesh->dwBoneCount==0 )
				{
					pMesh->bVisible = false;
				}
			}
			else
			{
				// lee - 300914 - maybe replace this with substitute technique?
				pMesh->bVisible = false;
			}
		}
	}
	else
	{
		// reset setting to all meshes
		g_pGlob->dwInternalFunctionCode=12001;
		sObject* pObject = g_ObjectList [ iID ];
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			g_pGlob->dwInternalFunctionCode=12001+iMesh;
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			SetSpecialEffect ( pMesh, NULL );
			pMesh->dwForceCPUAnimationMode = 0;
		}
	}

	// as recreates mesh format, must regenerate buffer instance
	g_pGlob->dwInternalFunctionCode=13001;
	m_ObjectManager.RemoveObjectFromBuffers ( pObject );
	m_ObjectManager.AddObjectToBuffers ( pObject );
	g_pGlob->dwInternalFunctionCode=13002;
}

DARKSDK_DLL void SetObjectEffectCore ( int iID, int iEffectID, int iForceCPUAnimationMode )
{
	SetObjectEffectCore ( iID, iEffectID, 0, iForceCPUAnimationMode );
}

DARKSDK_DLL void SetOcclusionMode ( int iOcclusionMode )
{
	// 0 - none
	// 1 - HOQ - Hardware Occlusion Queries (determine if visible pixels rendered)
	#ifndef DX11
	g_Occlusion.SetOcclusionMode ( iOcclusionMode );
	#endif
}

DARKSDK_DLL void SetObjectOcclusion ( int iID, int iOcclusionShape, int iMeshOrLimbID, int iA, int iIsOccluder, int iDeleteFromOccluder )
{
	#ifdef DX11
	return;
	#else
	// iOcclusionShape & iMeshOrLimbID ignored for now

	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// HOQ Method
	if ( g_Occlusion.GetOcclusionMode()==1 )
	{
		// only if not already an occludabler
		if ( g_Occlusion.d3dQuery[pObject->dwObjectNumber] )
			return;

		// create query object
		#ifdef DX11
		#else
		m_pD3D->CreateQuery( D3DQUERYTYPE_OCCLUSION, &g_Occlusion.d3dQuery[pObject->dwObjectNumber] );
		g_Occlusion.d3dQuery[pObject->dwObjectNumber]->Issue( D3DISSUE_BEGIN );
		g_Occlusion.d3dQuery[pObject->dwObjectNumber]->Issue( D3DISSUE_END );
		#endif
	}

	// complete
	return;
	#endif
}

DARKSDK_DLL int GetObjectOcclusionValue ( int iID )
{
	// return var
	int iReturnValue = 0;
	#ifndef DX11
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	if ( g_Occlusion.GetOcclusionMode()==1 )
	{
		// can get how many of its pixels got rendered
		sObject* pObject = g_ObjectList [ iID ];
		iReturnValue = g_Occlusion.dwQueryValue[pObject->dwObjectNumber];

		// and trigger the next query (does not happen each cycle as expensive for some GPUs)
		if ( g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber]==0 )
			g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 99;
	}
	if ( g_Occlusion.GetOcclusionMode()==2 )
	{
		// 0-shown or 1-occluded
		iReturnValue = 0;
	}
	#endif
	// return value
	return iReturnValue;
}

DARKSDK_DLL void SetObjectEffect ( int iID, int iEffectID )
{
	// call master core function for this
	SetObjectEffectCore ( iID, iEffectID, 0 );
}

DARKSDK_DLL void SetLimbEffect ( int iID, int iLimbID, int iEffectID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// check the effect exists
	if ( iEffectID!=0 )
		if ( !ConfirmEffect ( iEffectID ) )
			return;

	// ensure limb has mesh
	sObject* pObject = g_ObjectList [ iID ];
	sMesh* pMesh = pObject->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// ensure mesh is reset
	ResetVertexDataInMeshPerMesh ( pMesh );

	// apply to specific mesh
	if ( iEffectID>0 )
	{
		sEffectItem* pEffectItem = m_EffectList [ iEffectID ];
		SetSpecialEffect ( pMesh, pEffectItem->pEffectObj );
		pMesh->bVertexShaderEffectRefOnly = true;
	}
	else
		SetSpecialEffect ( pMesh, NULL );

	// as recreates mesh format, must regenerate buffer instance
	m_ObjectManager.RemoveObjectFromBuffers ( pObject );
	m_ObjectManager.AddObjectToBuffers ( pObject );
}

DARKSDK_DLL void PerformChecklistForEffectValues ( int iEffectID )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	#ifdef DX11
	#else
	// Get effect ptr and desc
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObj = NULL;
	if ( m_EffectList [ iEffectID ]->pEffectObj )
	{
		pEffectObj = m_EffectList [ iEffectID ]->pEffectObj;
		if ( pEffectObj->m_pEffect ) pEffectPtr = pEffectObj->m_pEffect;
	}

	// if effect tr valid
	if ( !pEffectPtr )
		return;

	// build checklist from all top-level params
	GGEFFECT_DESC	EffectDesc;
	pEffectPtr->GetDesc( &EffectDesc );

	// Generate Checklist
	DWORD dwMaxStringSizeInEnum=0;
	bool bCreateChecklistNow=false;
	g_pGlob->checklisthasvalues=true;
	g_pGlob->checklisthasstrings=true;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, dwMaxStringSizeInEnum);
		}

		// Look at parameters
		g_pGlob->checklistqty=0;
		for( UINT iParam = 0; iParam < EffectDesc.Parameters; iParam++ )
		{
			// get this parameter handle and description
			D3DXPARAMETER_DESC ParamDesc;
			GGHANDLE hParam = pEffectPtr->GetParameter ( NULL, iParam );
			pEffectPtr->GetParameterDesc( hParam, &ParamDesc );

			// Add to checklist
			DWORD dwSize=0;
			if(ParamDesc.Name) dwSize=strlen(ParamDesc.Name);
			if(dwSize>dwMaxStringSizeInEnum) dwMaxStringSizeInEnum=dwSize;
			if(bCreateChecklistNow)
			{
				// New checklist item
				if(ParamDesc.Name==NULL)
					strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, "<noname>");
				else
					strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, ParamDesc.Name);

				// class and type form var type id (dbpro usage)
				int iVarTypeIdentity = 0;
				if ( ParamDesc.Class==D3DXPC_SCALAR )
				{
					if ( ParamDesc.Type==D3DXPT_BOOL ) iVarTypeIdentity = 1;
					if ( ParamDesc.Type==D3DXPT_INT ) iVarTypeIdentity = 2;
					if ( ParamDesc.Type==D3DXPT_FLOAT ) iVarTypeIdentity = 3;
				}
				if ( ParamDesc.Class==D3DXPC_VECTOR )
				{
					iVarTypeIdentity = 4;
				}
				if ( ParamDesc.Class==D3DXPC_MATRIX_ROWS
				||   ParamDesc.Class==D3DXPC_MATRIX_COLUMNS )
				{
					iVarTypeIdentity = 5;
				}
				g_pGlob->checklist[g_pGlob->checklistqty].valuea = iVarTypeIdentity;

				// whether app-hooked (dbpro providing the value)
				if ( pEffectObj->AssignValueHook ( (char*)ParamDesc.Semantic, NULL)==true )
					g_pGlob->checklist[g_pGlob->checklistqty].valueb = 1;
				else
				{
					// non semantic hook identities
					bool bHookIsValid = false;

					// by annotation
					GGHANDLE hAnnot = pEffectPtr->GetAnnotationByName( hParam, "UIDirectional" );
					if( hAnnot != NULL && pEffectObj->m_LightDirHandle ) bHookIsValid=true;
					hAnnot = pEffectPtr->GetAnnotationByName( hParam, "UIDirectionalInv" );
					if( hAnnot != NULL && pEffectObj->m_LightDirInvHandle ) bHookIsValid=true;
					hAnnot = pEffectPtr->GetAnnotationByName( hParam, "UIPosition" );
					if( hAnnot != NULL && pEffectObj->m_LightPosHandle ) bHookIsValid=true;
					hAnnot = pEffectPtr->GetAnnotationByName( hParam, "UIObject" );
					if( hAnnot != NULL )
					{
						// light type
						LPCSTR pstrLightType = NULL;
						if ( hAnnot != NULL ) pEffectPtr->GetString( hAnnot, &pstrLightType );
						if ( pstrLightType )
						{
							if ( _stricmp((char*)pstrLightType,"directionalight")==NULL && pEffectObj->m_LightDirHandle ) bHookIsValid=true;
							if ( _stricmp((char*)pstrLightType,"pointlight")==NULL && pEffectObj->m_LightPosHandle ) bHookIsValid=true;
							if ( _stricmp((char*)pstrLightType,"spotlight")==NULL && pEffectObj->m_LightPosHandle ) bHookIsValid=true;
						}
					}

					// special cases
					if( _stricmp ( ParamDesc.Name, "XFile" )==NULL ) bHookIsValid=true;
					if ( ParamDesc.Type>=D3DXPT_TEXTURE ) bHookIsValid=true;

					// assign result
					if ( bHookIsValid==true )
						g_pGlob->checklist[g_pGlob->checklistqty].valueb = 1;
					else
						g_pGlob->checklist[g_pGlob->checklistqty].valueb = 0;
				}

				// class
				g_pGlob->checklist[g_pGlob->checklistqty].valuec = ParamDesc.Class;

				// type
				g_pGlob->checklist[g_pGlob->checklistqty].valued = ParamDesc.Type;
			}
			g_pGlob->checklistqty++;
		}
	}

	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
	#endif
}

DARKSDK_DLL void PerformChecklistForEffectErrors ( void )
{
	// Generate Checklist
	DWORD dwMaxStringSizeInEnum=0;
	bool bCreateChecklistNow=false;
	g_pGlob->checklisthasvalues=false;
	g_pGlob->checklisthasstrings=true;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, dwMaxStringSizeInEnum);
		}

		// Look at error buffer (if any)
		if ( g_pEffectErrorMsg ) 
		{
			LPSTR pPtr = g_pEffectErrorMsg;
			LPSTR pPtrEnd = g_pEffectErrorMsg + g_dwEffectErrorMsgSize;
			LPSTR pLastByte = pPtr;
			g_pGlob->checklistqty=0;
			while ( 1 )
			{
				if ( pPtr>=pPtrEnd || ( *(unsigned char*)pPtr==13 || *(unsigned char*)(pPtr+1)==10 ) )
				{
					// determine error line
					DWORD dwSize = (pPtr-pLastByte)+1;
					LPSTR pErrorLine = new char[dwSize];
					memcpy ( pErrorLine, pLastByte, dwSize );
					pErrorLine[dwSize]=0;

					// skip the colons
					int nn=-1; int iCount=2;
					for ( int n=0; n<(int)dwSize; n++)
					{
						if ( pErrorLine[n]==':' )
						{
							if ( iCount>0 )
							{
								nn=n+2;
								iCount--;
							}
							else
								break;
						}
					}
					if ( nn!=-1 )
					{
						_strrev ( pErrorLine );
						pErrorLine[dwSize-nn]=0;
						_strrev ( pErrorLine );
						dwSize = strlen(pErrorLine)+1;
					}

					// Add to checklist at end of line or buffer
					if(dwSize>dwMaxStringSizeInEnum) dwMaxStringSizeInEnum=dwSize;
					if(bCreateChecklistNow)
					{
						// New checklist item
						strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, pErrorLine);
					}
					g_pGlob->checklistqty++;

					// go for next line
					if ( pPtr>=pPtrEnd )
					{
						// exit buffer scan
						break;
					}
					else
					{
						// next line
						pPtr+=2; pLastByte=pPtr;
					}
				}
				else
				{
					// next byte
					pPtr++;
				}
			}
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

DARKSDK_DLL void PerformChecklistForEffectErrors ( int iEffectID )
{
	PerformChecklistForEffectErrors();
}

DARKSDK_DLL void SetEffectTranspose ( int iEffectID, int iTransposeFlag )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// Get effect ptr
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;

	// apply flag
	if ( iTransposeFlag==1 )
		pEffectObject->m_bTranposeToggle = true;
	else
		pEffectObject->m_bTranposeToggle = false;
}

DARKSDK_DLL void ResetEffect ( int iEffectID )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// called when switching shader and need new param values from new shader
	SAFE_DELETE ( g_CascadedShadow.m_pEffectParam[iEffectID] );
}

DARKSDK_DLL void EraseEffectParameterIndex ( int iEffectID, LPSTR pConstantName )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// prevent system from overriding manual change by removing hook
	#ifdef DX11
	#else
	if ( pEffectObject && pEffectPtr )
	{
		GGHANDLE hConstantParamHandle = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );
		if ( !g_EffectParamHandleList.empty() )
		{
			for ( DWORD iIndex = 0; iIndex < g_EffectParamHandleList.size(); ++iIndex )
			{
				GGHANDLE hThisHandle = g_EffectParamHandleList [ iIndex ];
				if ( hThisHandle==hConstantParamHandle )
				{
					g_EffectParamHandleList [ iIndex ] = NULL;
				}
			}
		}
	}
	#endif
}

DARKSDK_DLL DWORD GetEffectParameterIndex ( int iEffectID, LPSTR pConstantName )
{
	// return unique index
	DWORD dwParameterIndex = 0;
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return 0;

	// get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// prevent system from overriding manual change by removing hook
	if ( pEffectObject && pEffectPtr )
	{
		#ifdef DX11
		GGHANDLE hConstantParamHandle = pEffectPtr->GetVariableByName ( (char*)pConstantName );
		#else
		GGHANDLE hConstantParamHandle = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );
		#endif
		if ( !g_EffectParamHandleList.empty() )
		{
			for ( DWORD iIndex = 0; iIndex < g_EffectParamHandleList.size(); ++iIndex )
			{
				GGHANDLE hThisHandle = g_EffectParamHandleList [ iIndex ];
				if ( hThisHandle==hConstantParamHandle )
				{
					// found param already in list, return unique index
					#ifdef DX11
					dwParameterIndex = (DWORD)hThisHandle;
					#else
					dwParameterIndex = iIndex;
					#endif
					hConstantParamHandle=NULL;
					break;
				}
			}
		}
		if ( hConstantParamHandle!=NULL )
		{
			pEffectObject->AssignValueHookCore ( NULL, hConstantParamHandle, 0, true ); //remove handle
			g_EffectParamHandleList.push_back ( hConstantParamHandle );
			#ifdef DX11
			dwParameterIndex = (DWORD)hConstantParamHandle;
			#else
			dwParameterIndex = g_EffectParamHandleList.size() - 1;
			#endif
		}
	}

	// return unique index to parameter
	return dwParameterIndex;
}

DARKSDK_DLL LPGGEFFECT SetEffectConstantCore ( int iEffectID, LPSTR pConstantName, int iOptionalParamIndex )
{

	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return NULL;

	// Get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// special hook constant names for internal shader texture hooks
	if ( pConstantName )
	{
		if ( strnicmp ( (char*)pConstantName, "[hook-depth-data]", strlen((char*)pConstantName) )==NULL )
		{
			// find shader handle, assign to texture in effect
			g_pMainCameraDepthEffect = pEffectPtr;
			if ( g_pMainCameraDepthEffect!=NULL )
			{
				#ifdef DX11
				g_pMainCameraDepthHandle = g_pMainCameraDepthEffect->GetVariableByName( "DepthTex" );
				#else
				g_pMainCameraDepthHandle = g_pMainCameraDepthEffect->GetParameterByName( NULL, "DepthTex" );
				#endif
			}
		}
	}

	// prevent system from overriding manual change by removing hook
	if ( pEffectObject && pEffectPtr )
	{
		if ( pConstantName==NULL )
		{
			// already removed hook for paramindex based constant setting
		}
		else
		{
			#ifdef DX11
			GGHANDLE hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );
			if ( hParam->IsValid() )
			{
				pEffectObject->AssignValueHookCore ( NULL, hParam, 0, true ); //remove handle
			}
			#else
			GGHANDLE hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );
			pEffectObject->AssignValueHookCore ( NULL, hParam, 0, true ); //remove handle
			#endif
		}
	}

	// return effect ptr (if any)
	return pEffectPtr;
}

DARKSDK_DLL LPGGEFFECT SetEffectConstantCore ( int iEffectID, LPSTR pConstantName )
{
	return SetEffectConstantCore ( iEffectID, pConstantName, -1 );
}

DARKSDK_DLL void SetEffectConstantB ( int iEffectID, LPSTR pConstantName, DWORD dwOptionalParamIndex, int iValue )
{
	// get constant ptr
	LPGGEFFECT pEffectPtr = SetEffectConstantCore ( iEffectID, pConstantName );
	#ifdef DX11
	if ( pEffectPtr )
	{
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = (GGHANDLE)dwOptionalParamIndex;
		else
			hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );

		if ( hParam->IsValid() )
			hParam->AsScalar()->SetBool ( iValue );
	}
	#else
	if ( pEffectPtr )
	{
		// apply value to constant
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = g_EffectParamHandleList [ iOptionalParamIndex ];
		else
			hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );

		pEffectPtr->SetBool ( hParam, iValue );
	}
	#endif
}

DARKSDK_DLL void SetEffectConstantB ( int iEffectID, LPSTR pConstantName, int iValue )
{
	SetEffectConstantB ( iEffectID, pConstantName, -1, iValue );
}

DARKSDK_DLL void SetEffectConstantBEx ( int iEffectID, DWORD dwParamIndex, int iValue )
{
	SetEffectConstantB ( iEffectID, NULL, dwParamIndex, iValue );
}

DARKSDK_DLL void SetEffectConstantI ( int iEffectID, LPSTR pConstantName, DWORD dwOptionalParamIndex, int iValue )
{
	// get constant ptr
	LPGGEFFECT pEffectPtr = SetEffectConstantCore ( iEffectID, pConstantName );
	#ifdef DX11
	if ( pEffectPtr )
	{
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = (GGHANDLE)dwOptionalParamIndex;
		else
			hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );

		if ( hParam->IsValid() )
			hParam->AsScalar()->SetInt ( iValue );
	}
	#else
	if ( pEffectPtr )
	{
		// apply value to constant
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = g_EffectParamHandleList [ iOptionalParamIndex ];
		else
			hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );

		pEffectPtr->SetInt ( hParam, iValue );
	}
	#endif
}

DARKSDK_DLL void SetEffectConstantI ( int iEffectID, LPSTR pConstantName, int iValue )
{
	SetEffectConstantI ( iEffectID, pConstantName, -1, iValue );
}

DARKSDK_DLL void SetEffectConstantIEx ( int iEffectID, DWORD dwParamIndex, int iValue )
{
	SetEffectConstantI ( iEffectID, NULL, dwParamIndex, iValue );
}

DARKSDK_DLL void SetEffectConstantF ( int iEffectID, LPSTR pConstantName, DWORD dwOptionalParamIndex, float fValue )
{
	// get constant ptr
	LPGGEFFECT pEffectPtr = SetEffectConstantCore ( iEffectID, pConstantName );
	#ifdef DX11
	if ( pEffectPtr )
	{
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = (GGHANDLE)dwOptionalParamIndex;
		else
			hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );

		if ( hParam->IsValid() )
			hParam->AsScalar()->SetFloat ( fValue );
	}
	#else
	if ( pEffectPtr )
	{
		// apply value to constant
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = g_EffectParamHandleList [ iOptionalParamIndex ];
		else
			hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );

		pEffectPtr->SetFloat ( hParam, fValue );
	}
	#endif
}

DARKSDK_DLL void SetEffectConstantF ( int iEffectID, LPSTR pConstantName, float fValue )
{
	SetEffectConstantF ( iEffectID, pConstantName, -1, fValue );
}

DARKSDK_DLL void SetEffectConstantFEx ( int iEffectID, DWORD dwParamIndex, float fValue )
{
	SetEffectConstantF ( iEffectID, NULL, dwParamIndex, fValue );
}

DARKSDK_DLL void SetEffectConstantV ( int iEffectID, LPSTR pConstantName, DWORD dwOptionalParamIndex, int iVector )
{
	// early out if no valid  param index
	if ( pConstantName==NULL && dwOptionalParamIndex==-1 )
		return;

	// get constant ptr
	LPGGEFFECT pEffectPtr = SetEffectConstantCore ( iEffectID, pConstantName );
	#ifdef DX11
	if ( pEffectPtr )
	{
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = (GGHANDLE)dwOptionalParamIndex;
		else
			hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );

		if ( hParam->IsValid() )
		{
			GGVECTOR4 vecData = GetVector4 ( iVector );
			hParam->AsVector()->SetFloatVector ( (float*)&vecData );
		}
	}
	#else
	if ( pEffectPtr )
	{
		// apply value to constant
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = g_EffectParamHandleList [ iOptionalParamIndex ];
		else
			hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );

		GGVECTOR4 vecData = GetVector4 ( iVector );
		pEffectPtr->SetVector ( hParam, &vecData );
	}
	#endif
}

DARKSDK_DLL void SetEffectConstantV ( int iEffectID, LPSTR pConstantName, int iVector )
{
	SetEffectConstantV ( iEffectID, pConstantName, -1, iVector );
}

DARKSDK_DLL void SetEffectConstantVEx ( int iEffectID, DWORD dwParamIndex, int iValue )
{
	SetEffectConstantV ( iEffectID, NULL, dwParamIndex, iValue );
}

DARKSDK_DLL void SetEffectConstantM ( int iEffectID, LPSTR pConstantName, DWORD dwOptionalParamIndex, int iMatrix )
{
	// early out if no valid  param index
	if ( pConstantName==NULL && dwOptionalParamIndex==-1 )
		return;

	// get constant ptr
	LPGGEFFECT pEffectPtr = SetEffectConstantCore ( iEffectID, pConstantName );
	#ifdef DX11
	if ( pEffectPtr )
	{
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = (GGHANDLE)dwOptionalParamIndex;
		else
			hParam = pEffectPtr->GetVariableByName ( (char*)pConstantName );

		if ( hParam->IsValid() )
		{
			GGMATRIX matData = GetMatrix ( iMatrix );
			hParam->AsMatrix()->SetMatrix ( (float*)&matData );
		}
	}
	#else
	if ( pEffectPtr )
	{
		// apply value to constant
		GGHANDLE hParam;
		if ( pConstantName==NULL )
			hParam = g_EffectParamHandleList [ iOptionalParamIndex ];
		else
			hParam = pEffectPtr->GetParameterByName ( NULL, (char*)pConstantName );

		GGMATRIX matData = GetMatrix ( iMatrix );
		pEffectPtr->SetMatrix ( hParam, &matData );
	}
	#endif
}

DARKSDK_DLL void SetEffectConstantM ( int iEffectID, LPSTR pConstantName, int iMatrix )
{
	SetEffectConstantM ( iEffectID, pConstantName, -1, iMatrix );
}

DARKSDK_DLL void SetEffectConstantMEx ( int iEffectID, DWORD dwParamIndex, int iValue )
{
	SetEffectConstantM ( iEffectID, NULL, dwParamIndex, iValue );
}

DARKSDK_DLL void SetEffectTechnique	( int iEffectID, LPSTR pTechniqueName )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// Get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// Choose technique based on name
	#ifdef DX11
	if ( pEffectPtr )
	{
		if ( pTechniqueName == NULL )
		{
			pEffectObject->m_hCurrentTechnique = pEffectPtr->GetTechniqueByIndex(0);
		}
		else
		{
			ID3DX11EffectTechnique* hTechnique = pEffectPtr->GetTechniqueByName ( (LPSTR)pTechniqueName );
			if ( hTechnique ) 
			{
				// assign the technique for this effect
				if ( hTechnique->IsValid() )
				{
					pEffectObject->m_hCurrentTechnique = hTechnique;
				}

				// find a pass named 'RenderDepthPixelsPass' and flag if found as we can skip this pass if engine does not use depth related stuff like DOF and MOTION BLUR (performance)
				//pEffectObject->m_DepthRenderPassHandle = pEffectPtr->GetPassByName ( hTechnique, "RenderDepthPixelsPass");
			}
		}
	}
	#else
	if ( pEffectPtr )
	{
		GGHANDLE hTechnique = pEffectPtr->GetTechniqueByName ( (LPSTR)pTechniqueName );
		if ( hTechnique ) 
		{
			// assign the technique for this effect
			pEffectPtr->SetTechnique(hTechnique);

			// 091115 - find a pass named 'RenderDepthPixelsPass' and flag if found as we can skip this pass if engine does not use depth related stuff like DOF and MOTION BLUR (performance)
			pEffectObject->m_DepthRenderPassHandle = pEffectPtr->GetPassByName ( hTechnique, "RenderDepthPixelsPass");
		}
	}
	#endif
	return;
}

DARKSDK_DLL void SetEffectTechniqueEx ( int iEffectID, DWORD dwPtr )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// Get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// Choose technique based on name
	#ifdef DX11
	if ( pEffectPtr )
	{
		ID3DX11EffectTechnique* hTechnique = (ID3DX11EffectTechnique*)dwPtr;
		if ( hTechnique ) 
		{
			// assign the technique for this effect
			if ( hTechnique->IsValid() )
			{
				pEffectObject->m_hCurrentTechnique = hTechnique;
			}
		}
	}
	#else
	#endif
	return;
}

DARKSDK_DLL DWORD GetEffectTechniqueEx ( int iEffectID )
{
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return NULL;

	// Get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// Choose technique based on name
	#ifdef DX11
	if ( pEffectPtr )
	{
		return (DWORD)pEffectObject->m_hCurrentTechnique;
	}
	#else
	#endif
	return NULL;
}

DARKSDK_DLL void SetEffectLODTechnique	( int iEffectID, LPSTR pTechniqueName )
{
	/* experimental idea
	// check the effect exists
	if ( !ConfirmEffect ( iEffectID ) )
		return;

	// Get effect ptr
	LPGGEFFECT pEffectPtr = NULL;
	cSpecialEffect* pEffectObject = m_EffectList [ iEffectID ]->pEffectObj;
	if ( pEffectObject )
		if ( pEffectObject->m_pEffect )
			pEffectPtr = pEffectObject->m_pEffect;

	// Choose technique based on name
	if ( pEffectPtr && pTechniqueName )
	{
		if ( strcmp ( (LPSTR)pTechniqueName, "" )!=NULL )
		{
			// use this when object at LOD distance
			GGHANDLE hTechnique = pEffectPtr->GetTechniqueByName ( (LPSTR)pTechniqueName );
			pEffectObject->m_hLODTechnique = hTechnique;
		}
		else
		{
			// do not override effect based on object distance
			pEffectObject->m_hLODTechnique = NULL;
		}
	}
	else
		return;
	*/
}

DARKSDK_DLL void SetGlobalDepthSkipSystem ( bool bSkipDepthRenderings )
{
	g_bSkipAnyDedicatedDepthRendering = bSkipDepthRenderings;
}

DARKSDK_DLL int GetEffectExist ( int iEffectID )
{
	// check the effect exists
	if ( iEffectID < 1 || iEffectID > MAX_EFFECTS )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DEFFECTNUMBERILLEGAL );
		return 0;
	}
	if ( m_EffectList [ iEffectID ] )
		return 1;
	else
		return 0;
}

DARKSDK int GetObjectPolygonCount ( int iObjectNumber )
{
	// total count
	int iPolygonTotal = 0;

	// check the object exists
	if ( !ConfirmObject ( iObjectNumber ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iObjectNumber ];
	if ( pObject )
	{
		if ( pObject->iMeshCount>0 )
		{
			for ( int iM=0; iM<pObject->iMeshCount; iM++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iM];
				if ( pMesh )
				{
					iPolygonTotal = iPolygonTotal + pMesh->iDrawPrimitives;
				}
			}
		}
	}

	// return total
	return iPolygonTotal;
}

int GetObjectVertexCount ( int iObjectNumber )
{
	// total count
	int iVertexCountTotal = 0;

	// check the object exists
	if ( !ConfirmObject ( iObjectNumber ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iObjectNumber ];
	if ( pObject )
	{
		if ( pObject->iMeshCount>0 )
		{
			for ( int iM=0; iM<pObject->iMeshCount; iM++ )
			{
				sMesh* pMesh = pObject->ppMeshList[iM];
				if ( pMesh )
				{
					if ( (int)pMesh->dwVertexCount>iVertexCountTotal )
					{
						iVertexCountTotal = pMesh->dwVertexCount;
					}
				}
			}
		}
	}

	// return total
	return iVertexCountTotal;
}

int GetObjectTotalVertexCount(int iObjectNumber)
{
	// total count
	int iVertexCountTotal = 0;

	// check the object exists
	if (!ConfirmObject(iObjectNumber))
		return 0;

	// return object information
	sObject* pObject = g_ObjectList[iObjectNumber];
	if (pObject)
	{
		if (pObject->iMeshCount > 0)
		{
			for (int iM = 0; iM < pObject->iMeshCount; iM++)
			{
				sMesh* pMesh = pObject->ppMeshList[iM];
				if (pMesh)
				{
					iVertexCountTotal += pMesh->dwVertexCount;
				}
			}
		}
	}

	// return total
	return iVertexCountTotal;
}

// Custom vertex shaders

DARKSDK_DLL void SetVertexShaderOn ( int iID, int iShader )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// validate shader
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}
	if ( m_VertexShaders [ iShader ].pVertexDec==NULL )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERINVALID);
		return;
	}

	// shader to set
	LPGGVERTEXSHADER pVertexShader = m_VertexShaders [ iShader ].pVertexShader;
	LPGGVERTEXLAYOUT pVertexDec = m_VertexShaders [ iShader ].pVertexDec;

	// apply shader to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetCustomShader ( pObject->ppMeshList [ iMesh ], pVertexShader, pVertexDec, 1 );
}

DARKSDK_DLL void SetVertexShaderOff ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply shader off to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetNoShader ( pObject->ppMeshList [ iMesh ] );
}

DARKSDK_DLL void CloneMeshToNewFormat ( int iID, DWORD dwFVF, DWORD dwEraseBones )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// object ref
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject==NULL )
		return;

	// create new mesh list to store ALL new meshes (erase bones means lightmapper process)
	if ( dwEraseBones==1 )
	{
		DWORD dwTotalMaterialCount = 0;
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			if ( pMesh )
				if ( pMesh->bUseMultiMaterial==true )
					dwTotalMaterialCount+=pMesh->dwMultiMaterialCount;
		}

		// convert object if found to use multimaterial heracy
		if ( dwTotalMaterialCount>0 )
		{
			// new mesh list and ref to connect old frame ptrs to new meshes
			sMesh** pTotalMeshListRef = new sMesh*[dwTotalMaterialCount];
			sMesh** pTotalMeshList = new sMesh*[dwTotalMaterialCount];

			// start off new frame list contents
			int iNewFrameCount = pObject->iFrameCount+dwTotalMaterialCount;
			sFrame** pTotalFrameList = new sFrame*[iNewFrameCount];
			memset ( pTotalFrameList, 0, sizeof(sFrame*)*iNewFrameCount );
			for ( int iFrameIndex=0; iFrameIndex<pObject->iFrameCount; iFrameIndex++ )
				pTotalFrameList [ iFrameIndex ] = pObject->ppFrameList [ iFrameIndex ];
			int iFrameCurrentIndex = pObject->iFrameCount;

			DWORD dwMaterialMeshIndex = 0;
			for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			{
				sMesh* pMesh = pObject->ppMeshList [ iMesh ];
				if ( pMesh )
				{
					if ( pMesh->bUseMultiMaterial==true )
					{
						// for each material, create a new mesh
						DWORD dwMaterialCount = pMesh->dwMultiMaterialCount;
						sMultiMaterial* pMultiMaterial = pMesh->pMultiMaterial;
						for ( DWORD dwMaterialIndex=0; dwMaterialIndex<dwMaterialCount; dwMaterialIndex++ )
						{
							sMesh* pNewMesh = new sMesh();
							if ( pNewMesh )
							{
								// duplicate mesh
								GGMATRIX matWorld;
								GGMatrixIdentity ( &matWorld );
								MakeMeshFromOtherMesh ( true, pNewMesh, pMesh, &matWorld );

								// copy in correct texture
								if ( pNewMesh->pTextures==NULL )
								{
									pNewMesh->dwTextureCount = 2;
									pNewMesh->pTextures = new sTexture [ pNewMesh->dwTextureCount ];
									memset ( pNewMesh->pTextures, 0, sizeof(sTexture)*pNewMesh->dwTextureCount );
								}
								pNewMesh->pTextures [ 0 ].iImageID  = pMesh->pTextures [ 0 ].iImageID;
								pNewMesh->pTextures [ 0 ].pTexturesRefView = pMultiMaterial [ dwMaterialIndex ].pTexturesRef;
								strcpy ( pNewMesh->pTextures [ 0 ].pName, pMultiMaterial [ dwMaterialIndex ].pName );
								pNewMesh->pTextures [ 0 ].dwBlendMode = GGTOP_MODULATE;
								pNewMesh->pTextures [ 0 ].dwBlendArg1 = GGTA_TEXTURE;
								pNewMesh->pTextures [ 0 ].dwBlendArg2 = GGTA_DIFFUSE;
								pNewMesh->bUseMultiMaterial = false;
								pNewMesh->fSpecularOverride = 1.0f;
								pNewMesh->bUsesMaterial = false;

								// modify index data so mesh only points to revelant polygons
								DWORD dwPolyCount = pMultiMaterial [ dwMaterialIndex ].dwPolyCount;
								if ( pNewMesh->pIndices != NULL )
								{
									// straight copy of relevant indices for this material 
									pNewMesh->dwIndexCount = dwPolyCount*3;
									pNewMesh->iDrawVertexCount = pMesh->iDrawVertexCount;
									pNewMesh->iDrawPrimitives  = dwPolyCount;
									memcpy ( pNewMesh->pIndices, pMesh->pIndices + pMultiMaterial [ dwMaterialIndex ].dwIndexStart, dwPolyCount*3*sizeof(WORD) );
								}
								else
								{
									// mesh exceeded 16bit index buffer, so need to manually copy the relevant verts for vert only mesh
									pNewMesh->dwIndexCount = 0;
									pNewMesh->iDrawVertexCount = dwPolyCount*3;
									pNewMesh->iDrawPrimitives  = dwPolyCount;
									for ( int i = 0; i < dwPolyCount*3; i+=3 )
									{
										int iV0 = pMultiMaterial [ dwMaterialIndex ].dwIndexStart + i + 0;
										int iV1 = pMultiMaterial [ dwMaterialIndex ].dwIndexStart + i + 1;
										int iV2 = pMultiMaterial [ dwMaterialIndex ].dwIndexStart + i + 2;
										*((GGVECTOR3*)pNewMesh->pVertexData+i+0) = *(GGVECTOR3*)pMesh->pVertexData+iV0;
										*((GGVECTOR3*)pNewMesh->pVertexData+i+1) = *(GGVECTOR3*)pMesh->pVertexData+iV1;
										*((GGVECTOR3*)pNewMesh->pVertexData+i+2) = *(GGVECTOR3*)pMesh->pVertexData+iV2;
									}
								}

								// add to new mesh list
								pTotalMeshList [ dwMaterialMeshIndex ] = pNewMesh;
								pTotalMeshListRef [ dwMaterialMeshIndex ] = pMesh;
								dwMaterialMeshIndex++;
							}
						}
					}
				}
			}

			// and replace old mesh list and references with new
			for ( int iMeshIndex=0; iMeshIndex<pObject->iMeshCount; iMeshIndex++ )
			{
				SAFE_DELETE(pObject->ppMeshList[iMeshIndex]);
			}
			SAFE_DELETE(pObject->ppMeshList);
			pObject->iMeshCount = dwTotalMaterialCount;
			pObject->ppMeshList = pTotalMeshList;

			// and update frame references
			for ( int iFrameIndex=0; iFrameIndex<pObject->iFrameCount; iFrameIndex++ )
			{
				sFrame* pFrame = pObject->ppFrameList[iFrameIndex];
				if ( pFrame )
				{
					sMesh* pMeshToReplace = pFrame->pMesh;
					sMesh* pMeshToReplaceWith = NULL;
					if ( pMeshToReplace )
					{
						for ( int iScanMatIndex=0; iScanMatIndex<(int)dwTotalMaterialCount; iScanMatIndex++ )
						{
							if ( pMeshToReplace==pTotalMeshListRef[iScanMatIndex] )
							{
								// found reference to OLD mesh
								pMeshToReplaceWith = pTotalMeshList[iScanMatIndex];
								pTotalMeshListRef[iScanMatIndex] = NULL;
								break;
							}
						}
					}
					pFrame->pMesh = pMeshToReplaceWith;
					if ( pMeshToReplaceWith )
					{
						// also tag 'additional' meshes onto the end as sybling frames
						sFrame* pThisFrame = pFrame;
						for ( int iScanMatIndex=0; iScanMatIndex<(int)dwTotalMaterialCount; iScanMatIndex++ )
						{
							if ( pTotalMeshListRef[iScanMatIndex]==pMeshToReplace )
							{
								while ( pThisFrame->pSibling ) pThisFrame = pThisFrame->pSibling;
								pThisFrame->pSibling = new sFrame();
								pThisFrame->pSibling->matOriginal = pFrame->matOriginal;
								pThisFrame->pSibling->matTransformed = pFrame->matTransformed;
								pThisFrame->pSibling->matCombined = pFrame->matCombined;
								strcpy ( pThisFrame->pSibling->szName, pFrame->szName );
								pThisFrame->pSibling->pMesh = pTotalMeshList[iScanMatIndex];
								// NOTE: Ensure this does not wipe critical REF needed if more meshes are looking for this hierarchy slot
								pTotalMeshListRef[iScanMatIndex] = NULL;
								pTotalFrameList [ iFrameCurrentIndex ] = pThisFrame->pSibling;
								iFrameCurrentIndex++;
								//break;
							}
						}
					}
				}
			}
			SAFE_DELETE ( pObject->ppFrameList );
			pObject->ppFrameList = pTotalFrameList;
			pObject->iFrameCount = iFrameCurrentIndex;

			// free usages
			SAFE_DELETE(pTotalMeshListRef);
		}
	}

	/*
	// Flexible vertex format bits
	#define D3DFVF_RESERVED0        0x001
	#define D3DFVF_POSITION_MASK    0x00E
	#define GGFVF_XYZ              0x002
	#define GGFVF_XYZRHW           0x004
	#define GGFVF_XYZB1            0x006
	#define GGFVF_XYZB2            0x008
	#define GGFVF_XYZB3            0x00a
	#define GGFVF_XYZB4            0x00c
	#define GGFVF_XYZB5            0x00e

	#define GGFVF_NORMAL           0x010
	#define GGFVF_PSIZE            0x020
	#define GGFVF_DIFFUSE          0x040
	#define D3DFVF_SPECULAR         0x080

	#define GGFVF_TEXCOUNT_MASK    0xf00
	#define GGFVF_TEXCOUNT_SHIFT   8
	#define D3DFVF_TEX0             0x000
	#define GGFVF_TEX1             0x100
	#define GGFVF_TEX2             0x200
	#define GGFVF_TEX3             0x300
	#define D3DFVF_TEX4             0x400
	#define D3DFVF_TEX5             0x500
	#define D3DFVF_TEX6             0x600
	#define D3DFVF_TEX7             0x700
	#define D3DFVF_TEX8             0x800
	#define D3DFVF_LASTBETA_UBYTE4  0x1000

    // Typical
    model         = GGFVF_XYZ + GGFVF_NORMAL + GGFVF_TEX1
					0x002 + 0x010 + 0x100 = 0x152 (274)
    model+diffuse = GGFVF_XYZ + GGFVF_NORMAL + GGFVF_TEX1 + GGFVF_DIFFUSE
					0x002 + 0x010 + 0x100 + 0x040 = 0x152 (338)
    model-normal  = GGFVF_XYZ + GGFVF_DIFFUSE + GGFVF_TEX1
					0x002 + 0x040 + 0x100 = (332)
	*/

	// clone mesh to the specific format
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		ConvertToFVF ( pObject->ppMeshList [ iMesh ], dwFVF );

	// lee - 050914 - also remove any bone animation data as converted object cannot animate without correct FVF skinning
	if ( dwEraseBones==1 )
	{
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		{
			sMesh* pMesh = pObject->ppMeshList [ iMesh ];
			if ( pMesh->dwBoneCount > 0 )
			{
				// erase the bone data
				SAFE_DELETE_ARRAY ( pMesh->pBones );
				pMesh->dwBoneCount = 0;
			}
		}
	}

	// regenerate buffer instance
	m_ObjectManager.RemoveObjectFromBuffers ( pObject );
	m_ObjectManager.AddObjectToBuffers ( pObject );
}

DARKSDK void CloneMeshToNewFormat ( int iID, DWORD dwFVF )
{
	CloneMeshToNewFormat ( iID, dwFVF, 0 );
}

DARKSDK_DLL void SetVertexShaderStreamCount ( int iID, int iCount )
{
	// set the size of the stream buffer

	// check the id is valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// make sure the count is ok
	if ( iCount < 1 )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCOUNTILLEGAL);
		return;
	}

	// store the count
	m_VertexShaders [ iID ].dwDecArrayCount = iCount;
	
	// allocate memory for the begining and end addons
	DWORD dwSize = iCount + 1;
	if ( ! ( m_VertexShaders [ iID ].pDecArray = new GGVERTEXELEMENT [ dwSize ] ) )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTCREATE);
		return;
	}

	// clear declaration data
	memset ( m_VertexShaders [ iID ].pDecArray, 0, sizeof(GGVERTEXELEMENT)*dwSize );

	// add the start and end values
	#ifdef DX11
	#else
	m_VertexShaders [ iID ].pDecArray [ iCount ].Stream = 0xFF;
	m_VertexShaders [ iID ].pDecArray [ iCount ].Offset  = 0;
	m_VertexShaders [ iID ].pDecArray [ iCount ].Type  = D3DDECLTYPE_UNUSED;
	m_VertexShaders [ iID ].pDecArray [ iCount ].Method  = 0;
	m_VertexShaders [ iID ].pDecArray [ iCount ].Usage = 0;
	m_VertexShaders [ iID ].pDecArray [ iCount ].UsageIndex = 0;
	#endif
}

DARKSDK_DLL void SetVertexShaderStream ( int iID, int iPos, int iDataUsage, int iDataType )
{
	// check the id is valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// make sure the dec pos is ok
	if ( iPos < 0 || iPos > (int)m_VertexShaders [ iID ].dwDecArrayCount )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERSTREAMPOSINVALID);
		return;
	}

	// check data usage 0-13
	if ( iDataUsage < 0 || iDataUsage > 13 )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERDATAINVALID);
		return;
	}

	#ifdef DX11
	#else
	DWORD dwOffset=0;
	int iIndex=0;
	while ( iIndex<(iPos-1) )
	{
		DWORD dwSize=0;
		switch ( m_VertexShaders [ iID ].pDecArray [ iIndex ].Type )
		{
			case D3DDECLTYPE_FLOAT1 :
			case D3DDECLTYPE_D3DCOLOR :
				dwSize=1;
				break;

			case GGDECLTYPE_FLOAT2 :
			case D3DDECLTYPE_SHORT2 :
				dwSize=2;
				break;

			case GGDECLTYPE_FLOAT3 :
				dwSize=3;
				break;

			case D3DDECLTYPE_FLOAT4 :
			case D3DDECLTYPE_UBYTE4 :
			case D3DDECLTYPE_SHORT4 :
				dwSize=4;
				break;
		}
		dwOffset+=dwSize*4;
		iIndex++;
	}

	// add the dec
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].Stream = 0;
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].Offset = dwOffset;
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].Type = iDataType;
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].Method = GGDECLMETHOD_DEFAULT;
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].Usage = iDataUsage;
	m_VertexShaders [ iID ].pDecArray [ iPos-1 ].UsageIndex = 0;
	#endif
}

DARKSDK_DLL void CreateVertexShaderFromFile ( int iID, SDK_LPSTR szFile )
{
	// check the id is valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// free any previous shaders
	#ifdef DX11
	#else
	SAFE_RELEASE ( m_VertexShaders [ iID ].pVertexShader );
	SAFE_RELEASE ( m_VertexShaders [ iID ].pVertexDec );

	// compile and create shader
	LPD3DXBUFFER pCode;
	LPD3DXBUFFER pErrorMsg;
	if ( FAILED ( D3DXAssembleShaderFromFile ( (LPSTR)szFile, NULL, NULL, 0, &pCode, &pErrorMsg ) ) )
	{
		//LPSTR pSee = (LPSTR)pErrorMsg->GetBufferPointer();
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTASSEMBLE);
		return;
	}
	if ( FAILED ( m_pD3D->CreateVertexShader ( (DWORD*)pCode->GetBufferPointer ( ), &m_VertexShaders [ iID ].pVertexShader ) ) )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTCREATE);
		return;
	}

	// free shader buffer
	pCode->Release ( );

	// create vertex declaration object
	if ( FAILED ( m_pD3D->CreateVertexDeclaration ( m_VertexShaders [ iID ].pDecArray, &m_VertexShaders [ iID ].pVertexDec ) ) )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTCREATE);
		return;
	}
	#endif
}

DARKSDK_DLL void SetVertexShaderVector ( int iID, DWORD dwRegister, int iVector, DWORD dwConstantCount )
{
	// vertify shader valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}
	
	// set constant
	#ifdef DX11
	#else
	GGVECTOR4 vecData = GetVector4 ( iVector );
	m_pD3D->SetVertexShaderConstantF ( dwRegister, (float*)&vecData, 1 );
	#endif
}

DARKSDK_DLL void SetVertexShaderMatrix ( int iID, DWORD dwRegister, int iMatrix, DWORD dwConstantCount )
{
	// vertify shader valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// set constant
	#ifdef DX11
	#else
	GGMATRIX matData = GetMatrix ( iMatrix );
	m_pD3D->SetVertexShaderConstantF ( dwRegister, (float*)&matData, 4 );
	#endif
}

DARKSDK_DLL void DeleteVertexShader ( int iShader )
{
	// vertify shader valid
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	#ifdef DX11
	#else
	// free vertex dec array
	SAFE_DELETE_ARRAY ( m_VertexShaders [ iShader ].pDecArray );

	// free vertex shader objects
	if ( m_VertexShaders [ iShader ].pVertexShader )
	{
		m_VertexShaders [ iShader ].pVertexShader->Release();
		m_VertexShaders [ iShader ].pVertexShader=NULL;
	}
	if ( m_VertexShaders [ iShader ].pVertexDec )
	{
		m_VertexShaders [ iShader ].pVertexDec->Release();
		m_VertexShaders [ iShader ].pVertexDec=NULL;
	}
	#endif
}

DARKSDK_DLL void SetPixelShaderOn ( int iID, int iShader )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// verify shader
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// apply shader to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetCustomPixelShader ( pObject->ppMeshList [ iMesh ], m_PixelShaders [ iShader ].pPixelShader );
}

DARKSDK_DLL void SetPixelShaderOff ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// apply shader off to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetNoPixelShader ( pObject->ppMeshList [ iMesh ] );
}

DARKSDK_DLL void SetPixelShaderTexture ( int iShaderObject, int iSlot, int iTexture )
{
	// check the object exists
	if ( !ConfirmObject ( iShaderObject ) )
		return;

	// apply shader to all meshes
	sObject* pObject = g_ObjectList [ iShaderObject ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		SetMultiTexture ( pObject->ppMeshList [ iMesh ], iSlot, GGTOP_MODULATE, 0, iTexture );

	// trigger a ew-new and re-sort
	m_ObjectManager.RenewReplacedMeshes ( pObject );
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void CreatePixelShaderFromFile ( int iID, SDK_LPSTR szFile )
{
	// check the id is valid
	if ( iID < 0 || iID > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	#ifdef DX11
	#else
	// free any previous shaders
	SAFE_RELEASE ( m_PixelShaders [ iID ].pPixelShader );

	// compile and create shader
	LPD3DXBUFFER pCode;
	if ( FAILED ( D3DXAssembleShaderFromFile ( (LPSTR)szFile, 0, NULL, 0, &pCode, NULL ) ) )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTASSEMBLE);
		return;
	}
	if ( FAILED ( m_pD3D->CreatePixelShader ( (DWORD*)pCode->GetBufferPointer(), &m_PixelShaders [ iID ].pPixelShader ) ) )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERCANNOTCREATE);
		return;
	}

	// free buffer
	pCode->Release ( );
	#endif
}

DARKSDK_DLL void DeletePixelShader ( int iShader )
{
	// vertify shader valid
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return;
	}

	// delete pixel shader
	#ifdef DX11
	#else
	if ( m_PixelShaders [ iShader ].pPixelShader )
	{
		m_PixelShaders [ iShader ].pPixelShader->Release();
		m_PixelShaders [ iShader ].pPixelShader=NULL;
	}
	#endif
}

// Collision Commands

DARKSDK_DLL void SetObjectCollisionOn ( int iID )
{
}

DARKSDK_DLL void SetObjectCollisionOff ( int iID )
{
}

DARKSDK_DLL void MakeCollisionBox ( int iID, float iX1, float iY1, float iZ1, float iX2, float iY2, float iZ2, int iRotatedBoxFlag )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	SetColBox( pObject, iX1, iY1, iZ1, iX2, iY2, iZ2, iRotatedBoxFlag );
}

DARKSDK_DLL void DeleteCollisionBox ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	FreeColBox ( pObject );
}

DARKSDK_DLL void SetCollisionToSpheres ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	SetColToSpheres ( pObject );
}

DARKSDK_DLL void SetCollisionToBoxes ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	SetColToBoxes ( pObject );
}

DARKSDK_DLL void SetCollisionToPolygons ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	SetColToPolygons ( pObject );
}

DARKSDK_DLL void SetSphereRadius ( int iID, float fRadius )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iID ];
	SetSphereRadius ( pObject, fRadius );
}

DARKSDK_DLL void SetGlobalCollisionOn ( void )
{
	GlobalColOn();
}

DARKSDK_DLL void SetGlobalCollisionOff ( void )
{
	GlobalColOff();
}

DARKSDK_DLL float IntersectObjectCore ( sObject* pObject, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreAllButLastFrame )
{
	// object must have its world data calculated
	CalcObjectWorld ( pObject );

	// do intersect check
	return CheckIntersectObject ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, iIgnoreAllButLastFrame );
}

DARKSDK_DLL float IntersectObject ( int iObjectID, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectID ) )
		return 0;

	// get object pointer
	sObject* pObject = g_ObjectList [ iObjectID ];

	// do intersect check
	float fDistance=IntersectObjectCore ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, 0 );

	// and must be within distance of specified vectors
	float fDistanceBetweenPoints;
	GGVECTOR3 vec3value = GGVECTOR3(fX,fY,fZ) - GGVECTOR3(fNewX,fNewY,fNewZ);
	fDistanceBetweenPoints = GGVec3Length(&vec3value);
	if ( fDistance > fDistanceBetweenPoints ) fDistance=0.0f;
	return fDistance;
}

bool intersectRayAABox2(const IntersectRay &ray, const IntersectBox &box, int& tnear, int& tfar)
{
    GGVECTOR3 T_1, T_2; // vectors to hold the T-values for every direction
    double t_near = -DBL_MAX; // maximums defined in float.h
    double t_far = DBL_MAX;

    for (int i = 0; i < 3; i++)
	{ 
		//we test slabs in every direction
        if (ray.direction[i] == 0)
		{ 
			// ray parallel to planes in this direction
            if ((ray.origin[i] < box.min[i]) || (ray.origin[i] > box.max[i])) 
			{
                return false; // parallel AND outside box : no intersection possible
            }
        } 
		else 
		{ 
			// ray not parallel to planes in this direction
            T_1[i] = (box.min[i] - ray.origin[i]) / ray.direction[i];
            T_2[i] = (box.max[i] - ray.origin[i]) / ray.direction[i];

            if(T_1[i] > T_2[i])
			{ 
				// we want T_1 to hold values for intersection with near plane
                std::swap(T_1,T_2);
            }
            if (T_1[i] > t_near)
			{
                t_near = T_1[i];
            }
            if (T_2[i] < t_far)
			{
                t_far = T_2[i];
            }
            if( (t_near > t_far) || (t_far < 0) )
			{
                return false;
            }
        }
    }
    tnear = t_near; tfar = t_far; // put return values in place
    return true; // if we made it here, there was an intersection - YAY
}

DARKSDK_DLL int IntersectAll_OLD ( int iPrimaryStart, int iPrimaryEnd, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreObjNo )
{
	// special iIgnoreObjNo mode
	if ( iIgnoreObjNo==-123 || iIgnoreObjNo==-124 )
	{
		if ( iIgnoreObjNo==-123 )
		{
			// obtain second range of objects to check
			g_iIntersectAllSecondStart = iPrimaryStart;
			g_iIntersectAllSecondEnd = iPrimaryEnd;
			#ifdef SKIPGRIDUSED
			g_fIntersectAllSkipGridX = fX;
			g_fIntersectAllSkipGridZ = fZ;
			#endif
			return 0;
		}
		else
		{
			// obtain third range of objects to check
			g_iIntersectAllThirdStart = iPrimaryStart;
			g_iIntersectAllThirdEnd = iPrimaryEnd;
			#ifdef SKIPGRIDUSED
			g_fIntersectAllSkipGridX = fX;
			g_fIntersectAllSkipGridZ = fZ;
			#endif
			return 0;
		}
	}
	else
	{
		// detect if the ray is recorded in skipgrid as blocked
		#ifdef SKIPGRIDUSED
		if ( g_fIntersectAllSkipGridX>0 )
		{
			int iSkipGridRefFromX = fX/50.0f;
			int iSkipGridRefFromZ = fZ/50.0f;
			if ( iSkipGridRefFromX < 0 ) iSkipGridRefFromX=0;
			if ( iSkipGridRefFromX > 1023 ) iSkipGridRefFromX=1023;
			if ( iSkipGridRefFromZ < 0 ) iSkipGridRefFromZ=0;
			if ( iSkipGridRefFromZ > 1023 ) iSkipGridRefFromZ=1023;
			WORD wTargetX = fNewX/10.0f;
			WORD wTargetZ = fNewZ/10.0f;
			DWORD dwSkipGridValue = g_dwSkipGrid[iSkipGridRefFromX][iSkipGridRefFromZ];
			if ( dwSkipGridValue > 0 )
			{
				WORD wRefX = (dwSkipGridValue & 0xFFFF0000) >> 16;
				WORD wRefZ = (dwSkipGridValue & 0x0000FFFF);
				if ( wRefX==wTargetX && wRefZ==wTargetZ )
				{
					// this target was found to be blocked from this coordinate
					return g_iSkipGridResult[iSkipGridRefFromX][iSkipGridRefFromZ];
				}
			}
		}
		#endif
	}

	// return value (0=no hit, >0=object number, -1=other geometry)
	int iHitValue = 0;

	// work out length of ray
	GGVECTOR3 vec3value = GGVECTOR3(fX,fY,fZ) - GGVECTOR3(fNewX,fNewY,fNewZ);
	float fDistanceBetweenPoints = GGVec3Length(&vec3value);

	// LEE: Dave, now you commented back in the old code, this entire routine is redundant, nes pa?
	// go through all objects and collect a shortlist of boxes intersected by ray
	// DAVE: Good point :D
	/*g_pIntersectShortList.clear();
	for ( int iPass=0; iPass<3; iPass++ )
	{
		int iStart, iEnd;
		if ( iPass==0 ) { iStart=iPrimaryStart; iEnd=iPrimaryEnd; }
		if ( iPass==1 ) { iStart=g_iIntersectAllSecondStart; iEnd=g_iIntersectAllSecondEnd; }
		if ( iPass==2 ) { iStart=g_iIntersectAllThirdStart; iEnd=g_iIntersectAllThirdEnd; if ( iStart == 0 ) break; }
		for ( int iObjectID = iStart; iObjectID <= iEnd; iObjectID++ )
		{
			// make sure we have a valid object
			sObject* pObject = g_ObjectList [ iObjectID ];
			if ( !pObject ) 
				continue;

			// check if object is excluded
			// check if object in dead state (non collisin detectable)
			if ( pObject->dwObjectNumber==iIgnoreObjNo || pObject->collision.dwCollisionPropertyValue==1 || !pObject->bVisible )
				continue;

			// check if object in same 'region' as ray
			float fDX=0, fDY=0, fDZ=0;
			if ( pObject->position.iGluedToObj>0 )
			{
				// use parent object instead
				sObject* pParentObject = g_ObjectList [ pObject->position.iGluedToObj ];
				if ( pParentObject )
				{
					fDX = pParentObject->position.vecPosition.x - fX;
					fDY = pParentObject->position.vecPosition.y - fY;
					fDZ = pParentObject->position.vecPosition.z - fZ;
				}
			}
			else
			{
				fDX = pObject->position.vecPosition.x - fX;
				fDY = pObject->position.vecPosition.y - fY;
				fDZ = pObject->position.vecPosition.z - fZ;
			}
			float fDist = sqrt(fabs(fDX*fDX)+fabs(fDY*fDY)+fabs(fDZ*fDZ));
			if ( fDist <= ((pObject->collision.fLargestRadius*3)+fDistanceBetweenPoints) )
			{
				// instead of transforming box to object world orientation, transform ray
				// on a per object basis back into object space, for quicker box checking
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
				if ( intersectRayAABox2(transformedray, box, tnear, tfar)==true )
				{
					g_pIntersectShortList.push_back ( pObject );
				}
			}
		}
	}

	// for shortlist of object(boxes) that touch ray, check closest with full polygon check
	float fBestDistance = 999999.9f;
	GlobChecklistStruct pBestHit[10];
	std::sort(g_pIntersectShortList.begin(), g_pIntersectShortList.end(), OrderByCamDistance() );
	int iIntersectShortListMax = g_pIntersectShortList.size ( );*/
	// DAVE 9/12/2014 Commented this out as sometimes the closer object isnt the one that gets hit by the ray first
	// E.G. in the middle of a building which counts as closer but the player who is inside the building would be the one getting hit as he is infront of the wall that will get tested first
	// Since the building counts as being closer, the test is a success and the player is never checked despite the polygon that would be hit of the player being closer
	/*for ( int iShortListIndex=0; iShortListIndex<iIntersectShortListMax; iShortListIndex++ )
	{
		sObject* pObject = g_pIntersectShortList [ iShortListIndex ];
		int iObjectID = pObject->dwObjectNumber;
		float fDistance = IntersectObjectCore ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, 0 );
		if ( fDistance > 0 && fDistance < fBestDistance && fDistance < fDistanceBetweenPoints )
		{
			// object was intersected, return obj number
			fBestDistance = fDistance;
			iHitValue = iObjectID;

			// populate checklist with extra hit info
			// 0 - frame indexes (A = mesh number, B = related frame number (if A is bone mesh))
			// 1 - vertex indexes
			// 2 - object-space coordinate of V0
			// 3 - object-space coordinate of V1
			// 4 - object-space coordinate of V2
			// 5 - world space coordinate where the collision struck!
			// 6 - normal of polygon struck (from vertA)
			// 7 - reflection normal based on angle of impact
			// 8 - arbitary value from mesh collision structure (set using SetObjectArbitary)
			for ( int iI=0; iI<9; iI++ ) pBestHit [ iI ] = g_pGlob->checklist [ iI ];

			// first polygon hit wins (unless it has no camdistance such as consolidated LM objects)
			if ( pObject->position.fCamDistance > 0.0f )
			{
				break;
			}
		}
	}*/

	// Dave - Added these declarations here, since the above routine isn't used anymore
	float fBestDistance = 999999.9f;
	GlobChecklistStruct pBestHit[10];
	
	// DAVE 9122014 This block was commented out, but put back in so it checks every object in the list
	// go through all objects presently in scene
	for ( int iPass=0; iPass<3; iPass++ )
	{
		int iStart, iEnd;
		if ( iPass==0 ) { iStart=iPrimaryStart; iEnd=iPrimaryEnd; }
		if ( iPass==1 ) { iStart=g_iIntersectAllSecondStart; iEnd=g_iIntersectAllSecondEnd; if ( iStart == 0 ) continue; }
		if ( iPass==2 ) { iStart=g_iIntersectAllThirdStart; iEnd=g_iIntersectAllThirdEnd; if ( iStart == 0 ) break; }
		for ( int iObjectID = iStart; iObjectID <= iEnd; iObjectID++ )
		{
			// make sure we have a valid object
			sObject* pObject = g_ObjectList [ iObjectID ];
			if ( pObject )
			{
				// check if object is excluded
				// check if object in dead state (non collisin detectable)
				//Dave added a skip for hidden objects
				if ( pObject->dwObjectNumber==iIgnoreObjNo || pObject->collision.dwCollisionPropertyValue==1 || !pObject->bVisible )
				{
					// ignore this object (usually the caster)
				}
				else
				{
					// check if object in same 'region' as ray
					float fDX=0, fDY=0, fDZ=0;
					if ( pObject->position.iGluedToObj>0 )
					{
						// use parent object instead
						sObject* pParentObject = g_ObjectList [ pObject->position.iGluedToObj ];
						if ( pParentObject )
						{
							fDX = pParentObject->position.vecPosition.x - fX;
							fDY = pParentObject->position.vecPosition.y - fY;
							fDZ = pParentObject->position.vecPosition.z - fZ;
						}
					}
					else
					{
						fDX = pObject->position.vecPosition.x - fX;
						fDY = pObject->position.vecPosition.y - fY;
						fDZ = pObject->position.vecPosition.z - fZ;
					}
					float fDist = sqrt(fabs(fDX*fDX)+fabs(fDY*fDY)+fabs(fDZ*fDZ));
					if ( fDist <= ((pObject->collision.fLargestRadius*3)+fDistanceBetweenPoints) )
					{
						// check if ray intersects object bound box (ray vs box)
						IntersectBox box;
						box.min[0] = pObject->position.vecPosition.x + pObject->collision.vecMin.x;
						box.min[1] = pObject->position.vecPosition.y + pObject->collision.vecMin.y;
						box.min[2] = pObject->position.vecPosition.z + pObject->collision.vecMin.z;
						box.max[0] = pObject->position.vecPosition.x + pObject->collision.vecMax.x;
						box.max[1] = pObject->position.vecPosition.y + pObject->collision.vecMax.y;
						box.max[2] = pObject->position.vecPosition.z + pObject->collision.vecMax.z;
						if ( true )//intersectRayAABox2(ray, box, tnear, tfar)==true )
						{
							// do intersect check
							float fDistance = IntersectObjectCore ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, 0 );
							if ( fDistance > 0 && fDistance < fBestDistance )
							{
								// and must be within distance of specified vectors
								if ( fDistance <= fDistanceBetweenPoints )
								{
									// only if object in detection range
									int iObjectHit = pObject->dwObjectNumber;
									if ( iObjectHit >= iStart && iObjectHit <= iEnd )
									{
										// object was intersected, return obj number
										iHitValue = iObjectHit;

										// populate checklist with extra hit info
										// 0 - frame indexes (A = mesh number, B = related frame number (if A is bone mesh))
										// 1 - vertex indexes
										// 2 - object-space coordinate of V0
										// 3 - object-space coordinate of V1
										// 4 - object-space coordinate of V2
										// 5 - world space coordinate where the collision struck!
										// 6 - normal of polygon struck (from vertA)
										// 7 - reflection normal based on angle of impact
										// 8 - normal vector?
										for ( int iI=0; iI<9; iI++ ) pBestHit [ iI ] = g_pGlob->checklist [ iI ];

										// find closest distance
										fBestDistance = fDistance;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	

	// if a hit was detected
	if ( iHitValue!=0 )
	{
		// copy best hit info back to checklist
		for ( int iI=0; iI<9; iI++ )
		{
			LPSTR pSaveStr = g_pGlob->checklist [ iI ].string;
			g_pGlob->checklist [ iI ] = pBestHit [ iI ];
			g_pGlob->checklist [ iI ].string = pSaveStr;
			if ( pSaveStr ) strcpy ( pSaveStr, "" );
		}
	}

	#ifdef SKIPGRIDUSED
	// also record in skipgrid (for optimized future collisions)
	int iSkipGridRefFromX = fX/50.0f;
	int iSkipGridRefFromZ = fZ/50.0f;
	if ( iSkipGridRefFromX < 0 ) iSkipGridRefFromX=0;
	if ( iSkipGridRefFromX > 1023 ) iSkipGridRefFromX=1023;
	if ( iSkipGridRefFromZ < 0 ) iSkipGridRefFromZ=0;
	if ( iSkipGridRefFromZ > 1023 ) iSkipGridRefFromZ=1023;
	WORD wTargetX = fNewX/10.0f;
	WORD wTargetZ = fNewZ/10.0f;
	DWORD dwSkipGridValue = (wTargetX<<16) + (wTargetZ);
	g_dwSkipGrid[iSkipGridRefFromX][iSkipGridRefFromZ] = dwSkipGridValue;
	g_iSkipGridResult[iSkipGridRefFromX][iSkipGridRefFromZ] = iHitValue;
	#endif

	// incase we dont want the third list next time
	g_iIntersectAllThirdStart = 0;

	// return hit value depending on what was hit
	return iHitValue;
}

//Dave Performance
//Previous intersect all is above, incase of issues
//This version combines the orignal method with the shortlist of boxes checked to provide the best of both versions
DARKSDK_DLL int IntersectAllEx ( int iPrimaryStart, int iPrimaryEnd, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreObjNo, int iStaticOnly)
{
	// special iIgnoreObjNo mode
	if (iIgnoreObjNo == -123 || iIgnoreObjNo == -124 || iIgnoreObjNo == -125)
	{
		g_bIgnoreCollisionPropertyOnce = false;
		if (iIgnoreObjNo == -123)
		{
			// obtain second range of objects to check
			g_iIntersectAllSecondStart = iPrimaryStart;
			g_iIntersectAllSecondEnd = iPrimaryEnd;
#ifdef SKIPGRIDUSED
			g_fIntersectAllSkipGridX = fX;
			g_fIntersectAllSkipGridZ = fZ;
#endif
			return 0;
		}
		if (iIgnoreObjNo == -124)
		{
			// obtain third range of objects to check
			g_iIntersectAllThirdStart = iPrimaryStart;
			g_iIntersectAllThirdEnd = iPrimaryEnd;
#ifdef SKIPGRIDUSED
			g_fIntersectAllSkipGridX = fX;
			g_fIntersectAllSkipGridZ = fZ;
#endif
			return 0;
		}
		if (iIgnoreObjNo == -125)
		{
			// 111215 - flag that this next intersect call will detect objects EVEN IF THEY HAVE A COLLISION PROPERTY OF ONE
			// which allows 'interactive weapon' ray casts to detect objects even if CollisionOff(e) was used
			g_bIgnoreCollisionPropertyOnce = true;
			return 0;
		}
	}
	else
	{
		// detect if the ray is recorded in skipgrid as blocked
		#ifdef SKIPGRIDUSED
		if ( g_fIntersectAllSkipGridX>0 )
		{
			int iSkipGridRefFromX = fX/50.0f;
			int iSkipGridRefFromZ = fZ/50.0f;
			if ( iSkipGridRefFromX < 0 ) iSkipGridRefFromX=0;
			if ( iSkipGridRefFromX > 1023 ) iSkipGridRefFromX=1023;
			if ( iSkipGridRefFromZ < 0 ) iSkipGridRefFromZ=0;
			if ( iSkipGridRefFromZ > 1023 ) iSkipGridRefFromZ=1023;
			WORD wTargetX = fNewX/10.0f;
			WORD wTargetZ = fNewZ/10.0f;
			DWORD dwSkipGridValue = g_dwSkipGrid[iSkipGridRefFromX][iSkipGridRefFromZ];
			if ( dwSkipGridValue > 0 )
			{
				WORD wRefX = (dwSkipGridValue & 0xFFFF0000) >> 16;
				WORD wRefZ = (dwSkipGridValue & 0x0000FFFF);
				if ( wRefX==wTargetX && wRefZ==wTargetZ )
				{
					// this target was found to be blocked from this coordinate
					return g_iSkipGridResult[iSkipGridRefFromX][iSkipGridRefFromZ];
				}
			}
		}
		#endif
	}

	// return value (0=no hit, >0=object number, -1=other geometry)
	int iHitValue = 0;

	// work out length of ray
	GGVECTOR3 vec3value = GGVECTOR3(fX,fY,fZ) - GGVECTOR3(fNewX,fNewY,fNewZ);
	float fDistanceBetweenPoints = GGVec3Length(&vec3value);

	// LEE: Dave, now you commented back in the old code, this entire routine is redundant, nes pa?
	// go through all objects and collect a shortlist of boxes intersected by ray
	// DAVE: Good point :D
	g_pIntersectShortList.clear();
	for ( int iPass=0; iPass<3; iPass++ )
	{
		int iStart, iEnd;
		if ( iPass==0 ) { iStart=iPrimaryStart; iEnd=iPrimaryEnd; }
		if ( iPass==1 ) { iStart=g_iIntersectAllSecondStart; iEnd=g_iIntersectAllSecondEnd; }
		if ( iPass==2 ) { iStart=g_iIntersectAllThirdStart; iEnd=g_iIntersectAllThirdEnd; if ( iStart == 0 ) break; }
		for ( int iObjectID = iStart; iObjectID <= iEnd; iObjectID++ )
		{
			// make sure we have a valid object
			sObject* pObject = g_ObjectList [ iObjectID ];
			if ( !pObject ) 
				continue;

			// check if object is excluded
			// check if object in dead state (non collisin detectable)
			if ( g_bIgnoreCollisionPropertyOnce == true)
			{
				// do not reject based on collision property if this flag set
				if (pObject->dwObjectNumber == iIgnoreObjNo || !pObject->bVisible)
					continue;
			}
			else
			{
				if (pObject->dwObjectNumber == iIgnoreObjNo || pObject->collision.dwCollisionPropertyValue == 1 || !pObject->bVisible)
					continue;
			}

			// check if object in same 'region' as ray
			float fDX=0, fDY=0, fDZ=0;
			if ( pObject->position.iGluedToObj>0 )
			{
				// use parent object instead
				sObject* pParentObject = g_ObjectList [ pObject->position.iGluedToObj ];
				if ( pParentObject )
				{
					fDX = pParentObject->position.vecPosition.x - fX;
					fDY = pParentObject->position.vecPosition.y - fY;
					fDZ = pParentObject->position.vecPosition.z - fZ;
				}
			}
			else
			{
				fDX = pObject->position.vecPosition.x - fX;
				fDY = pObject->position.vecPosition.y - fY;
				fDZ = pObject->position.vecPosition.z - fZ;
			}
			float fDist = sqrt((fDX*fDX)+(fDY*fDY)+(fDZ*fDZ));
			if ( fDist <= ((pObject->collision.fLargestRadius*3)+fDistanceBetweenPoints) )
			{
				// 110919 - ensure any offset applied to frame zero is accounted for (OFFSETX/Y/Z)
				GGMATRIX matWorldWithFrameOffset = pObject->position.matWorld;

				//PE: ppFrameList[0]->matAbsoluteWorld is wrong on some objects.
				//sObject* pActualObj = pObject;

				//PE: @Lee how do i extract the Offset x,y,z and what is it used for ?
				//PE: We cant use matAbsoluteWorld as it has inverse settings -1 (on some objects), and the collision check dont work.
				//if (pObject->pInstanceOfObject) pActualObj = pObject->pInstanceOfObject;
				//if (pActualObj->ppFrameList[0]) {
				//	matWorldWithFrameOffset = pActualObj->ppFrameList[0]->matAbsoluteWorld;
				//}

				// Please validate this will fix, https://github.com/TheGameCreators/GameGuruRepo/issues/724#issuecomment-606155967
				// PE: @Lee , why do we need matWorldWithFrameOffset = pActualObj->ppFrameList[0]->matAbsoluteWorld; ?? ? ??
				// PE: We need the original object position to generate the correct vecFrom
				// LB: Looks good!

				matWorldWithFrameOffset._41 = pObject->position.matWorld._41;
				matWorldWithFrameOffset._42 = pObject->position.matWorld._42;
				matWorldWithFrameOffset._43 = pObject->position.matWorld._43;

				// instead of transforming box to object world orientation, transform ray
				// on a per object basis back into object space, for quicker box checking
				float fDet;
				GGMATRIX matInvWorld;
				GGMatrixInverse(&matInvWorld,&fDet,&matWorldWithFrameOffset);//pObject->position.matWorld);
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

				// get half height size of object bounds to create larger bounbox detection area in the Y
				float fHeightSize = pObject->collision.vecMax.y - pObject->collision.vecMin.y;
				if ( fHeightSize < 0 ) fHeightSize = -fHeightSize;
				fHeightSize *= 0.5f;

				// check if ray intersects object bound box (ray vs box) [using object space]
				IntersectBox box;
				box.min[0] = pObject->collision.vecMin.x;
				// 010318 - seems my code to expand the boundbox does not work if min is 30 and max is 52!
				//box.min[1] = pObject->collision.vecMin.y * 2; // 240817 - object global bounds for some characters can be off, so increase to compensate
				box.min[1] = pObject->collision.vecMin.y-fHeightSize; // 240817 - object global bounds for some characters can be off, so increase to compensate
				box.min[2] = pObject->collision.vecMin.z;
				box.max[0] = pObject->collision.vecMax.x;
				// 010318 - seems my code to expand the boundbox does not work if min is 30 and max is 52!
				//box.max[1] = pObject->collision.vecMax.y * 2; // 240817 - object global bounds for some characters can be off, so increase to compensate
				box.max[1] = pObject->collision.vecMax.y+fHeightSize; // 240817 - object global bounds for some characters can be off, so increase to compensate
				box.max[2] = pObject->collision.vecMax.z;
				int tnear, tfar;
				if ( intersectRayAABox2(transformedray, box, tnear, tfar)==true )
				{
					g_pIntersectShortList.push_back ( pObject );
				}
			}
		}
	}

	// 111215 - and always reset this flag just in case we call a non special mode twice (once only check!!)
	g_bIgnoreCollisionPropertyOnce = false;

	// for shortlist of object(boxes) that touch ray, check closest with full polygon check
	float fBestDistance = 999999.9f;
	GlobChecklistStruct pBestHit[10];
	std::sort(g_pIntersectShortList.begin(), g_pIntersectShortList.end(), OrderByCamDistance() );
	int iIntersectShortListMax = g_pIntersectShortList.size ( );
	// DAVE 9/12/2014 Commented this out as sometimes the closer object isnt the one that gets hit by the ray first
	// E.G. in the middle of a building which counts as closer but the player who is inside the building would be the one getting hit as he is infront of the wall that will get tested first
	// Since the building counts as being closer, the test is a success and the player is never checked despite the polygon that would be hit of the player being closer
	/*for ( int iShortListIndex=0; iShortListIndex<iIntersectShortListMax; iShortListIndex++ )
	{
		sObject* pObject = g_pIntersectShortList [ iShortListIndex ];
		int iObjectID = pObject->dwObjectNumber;
		float fDistance = IntersectObjectCore ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, 0 );
		if ( fDistance > 0 && fDistance < fBestDistance && fDistance < fDistanceBetweenPoints )
		{
			// object was intersected, return obj number
			fBestDistance = fDistance;
			iHitValue = iObjectID;

			// populate checklist with extra hit info
			// 0 - frame indexes (A = mesh number, B = related frame number (if A is bone mesh))
			// 1 - vertex indexes
			// 2 - object-space coordinate of V0
			// 3 - object-space coordinate of V1
			// 4 - object-space coordinate of V2
			// 5 - world space coordinate where the collision struck!
			// 6 - normal of polygon struck (from vertA)
			// 7 - reflection normal based on angle of impact
			// 8 - arbitary value from mesh collision structure (set using SetObjectArbitary)
			for ( int iI=0; iI<9; iI++ ) pBestHit [ iI ] = g_pGlob->checklist [ iI ];

			// first polygon hit wins (unless it has no camdistance such as consolidated LM objects)
			if ( pObject->position.fCamDistance > 0.0f )
			{
				break;
			}
		}
	}*/

	// DAVE 9122014 This block was commented out, but put back in so it checks every object in the list
	// go through all objects presently in scene
	for ( int iShortListIndex=0; iShortListIndex<iIntersectShortListMax; iShortListIndex++ )
	{
		// make sure we have a valid object
		sObject* pObject = g_pIntersectShortList [ iShortListIndex ];
		int iObjectID = pObject->dwObjectNumber;
		if ( pObject )
		{
			// check if object is excluded
			// check if object in dead state (non collisin detectable)
			//Dave added a skip for hidden objects
			if ( true )//intersectRayAABox2(ray, box, tnear, tfar)==true )
			{
				// do intersect check
				float fDistance = IntersectObjectCore ( pObject, fX, fY, fZ, fNewX, fNewY, fNewZ, 0 );
				if ( fDistance > 0 && fDistance < fBestDistance )
				{
					// and must be within distance of specified vectors
					if ( fDistance <= fDistanceBetweenPoints )
					{
						// only if object in detection range
						int iObjectHit = pObject->dwObjectNumber;
						// object was intersected, return obj number
						iHitValue = iObjectHit;

						// populate checklist with extra hit info
						// 0 - frame indexes (A = mesh number, B = related frame number (if A is bone mesh))
						// 1 - vertex indexes
						// 2 - object-space coordinate of V0
						// 3 - object-space coordinate of V1
						// 4 - object-space coordinate of V2
						// 5 - world space coordinate where the collision struck!
						// 6 - normal of polygon struck (from vertA)
						// 7 - reflection normal based on angle of impact
						// 8 - normal vector?
						for ( int iI=0; iI<9; iI++ ) pBestHit [ iI ] = g_pGlob->checklist [ iI ];

						// find closest distance
						fBestDistance = fDistance;
					}
				}
			}

		}
	}
	

	// if a hit was detected
	if ( iHitValue!=0 )
	{
		// copy best hit info back to checklist
		for ( int iI=0; iI<9; iI++ )
		{
			LPSTR pSaveStr = g_pGlob->checklist [ iI ].string;
			g_pGlob->checklist [ iI ] = pBestHit [ iI ];
			g_pGlob->checklist [ iI ].string = pSaveStr;
			if ( pSaveStr ) strcpy ( pSaveStr, "" );
		}
	}

	#ifdef SKIPGRIDUSED
	// also record in skipgrid (for optimized future collisions)
	int iSkipGridRefFromX = fX/50.0f;
	int iSkipGridRefFromZ = fZ/50.0f;
	if ( iSkipGridRefFromX < 0 ) iSkipGridRefFromX=0;
	if ( iSkipGridRefFromX > 1023 ) iSkipGridRefFromX=1023;
	if ( iSkipGridRefFromZ < 0 ) iSkipGridRefFromZ=0;
	if ( iSkipGridRefFromZ > 1023 ) iSkipGridRefFromZ=1023;
	WORD wTargetX = fNewX/10.0f;
	WORD wTargetZ = fNewZ/10.0f;
	DWORD dwSkipGridValue = (wTargetX<<16) + (wTargetZ);
	g_dwSkipGrid[iSkipGridRefFromX][iSkipGridRefFromZ] = dwSkipGridValue;
	g_iSkipGridResult[iSkipGridRefFromX][iSkipGridRefFromZ] = iHitValue;
	#endif

	// incase we dont want the third list next time
	g_iIntersectAllThirdStart = 0;

	// return hit value depending on what was hit
	return iHitValue;
}

DARKSDK_DLL int IntersectAll(int iPrimaryStart, int iPrimaryEnd, float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, int iIgnoreObjNo)
{
	return IntersectAllEx(iPrimaryStart, iPrimaryEnd, fX, fY, fZ, fNewX, fNewY, fNewZ, iIgnoreObjNo, 0);
}

DARKSDK void SetObjectCollisionProperty ( int iObjectID, int iPropertyValue )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectID ) )
		return;

	// assign property value
	sObject* pObject = g_ObjectList [ iObjectID ];
	pObject->collision.dwCollisionPropertyValue = iPropertyValue;
}

// Cool Auto-Collision Commands

DARKSDK_DLL void AutomaticObjectCollision ( int iObjectID, float fRadius, int iResponse )
{
	// check the object exists
	if ( !ConfirmObject ( iObjectID ) )
		return;

	// get object pointer
	sObject* pObject = g_ObjectList [ iObjectID ];
	AutoObjectCol ( pObject, fRadius, iResponse );
}

DARKSDK_DLL void AutomaticCameraCollision ( int iCameraID, float fRadius, int iResponse, int iStandGroundMode )
{
	AutoCameraCol ( iCameraID, fRadius, iResponse, iStandGroundMode );
}

DARKSDK_DLL void AutomaticCameraCollision ( int iCameraID, float fRadius, int iResponse )
{
	AutoCameraCol ( iCameraID, fRadius, iResponse );
}

DARKSDK_DLL void ForceAutomaticEnd ( void )
{
	// leeadd - 080604 - required to find new camera/obj position before sync!
	AutomaticEnd ();
}

DARKSDK_DLL void HideBounds ( int iID )
{
}

DARKSDK_DLL void ShowBoundsEx ( int iID, int iBoxOnly )
{
}

DARKSDK_DLL void ShowBounds ( int iID )
{
}

DARKSDK_DLL void ShowBounds ( int iID, int iLimb )
{
}

//
// LIMB Commands
//

DARKSDK_DLL void PerformCheckListForLimbs ( int iID )
{
	// check the object limb exists
	if ( !ConfirmObject ( iID ) )
		return;

	// Generate Checklist
	DWORD dwMaxStringSizeInEnum=0;
	bool bCreateChecklistNow=false;
	g_pGlob->checklisthasvalues=true;
	g_pGlob->checklisthasstrings=true;

	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
			{
				GlobExpandChecklist(c, dwMaxStringSizeInEnum);
			}
		}

		// Run through total list
		g_pGlob->checklistqty=0;
		DWORD dwLimbMax = g_ObjectList [ iID ]->iFrameCount;
		for(DWORD iLimbID=0; iLimbID<dwLimbMax; iLimbID++)
		{
			// Get limb name
			LPSTR pName = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->szName;

			// Add to checklist
			DWORD dwSize=0;
			if(pName) dwSize=strlen(pName);
			if(dwSize>dwMaxStringSizeInEnum) dwMaxStringSizeInEnum=dwSize;
			if(bCreateChecklistNow)
			{
				// New checklist item
				g_pGlob->checklist[g_pGlob->checklistqty].valuea=iLimbID;
				if(pName==NULL)
					strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, "");
				else
					strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, pName);

				// calculate parent and child id
				sFrame* pCurrent = g_ObjectList [ iID ]->ppFrameList [ iLimbID ];
				sFrame* pParent = pCurrent->pParent;
				sFrame* pSibling = pCurrent->pSibling;
				sFrame* pChild = pCurrent->pChild;
				int childid=-1, siblingid=-1, parentid=-1;
				if ( pParent ) parentid = pParent->iID;
				if ( pSibling ) siblingid = pSibling->iID;
				if ( pChild ) childid = pChild->iID;

				// record id of frame index
				g_pGlob->checklist[g_pGlob->checklistqty].valuea = iLimbID;

				// record id of parent
				g_pGlob->checklist[g_pGlob->checklistqty].valueb = parentid;

				// record if of sibling
				g_pGlob->checklist[g_pGlob->checklistqty].valuec = siblingid;

				// record if of child
				g_pGlob->checklist[g_pGlob->checklistqty].valued = childid;
			}
			g_pGlob->checklistqty++;
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

DARKSDK_DLL void PerformCheckListForOnscreenObjects ( int iMode )
{
	// 301007 - new command
	bool bCreateChecklistNow=false;
	g_pGlob->checklisthasvalues=true;
	g_pGlob->checklisthasstrings=false;
	for(int pass=0; pass<2; pass++)
	{
		// Ensure checklist is large enough
		if(pass==1)
		{
			bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, 256);
		}

		// Run through total list
		g_pGlob->checklistqty=0;
		int iVisibleObjectCount = m_ObjectManager.GetVisibleObjectCount();
		sObject** ppSortedObjectVisibleList = m_ObjectManager.GetSortedObjectVisibleList();
		for ( int iObject = 0; iObject < iVisibleObjectCount; iObject++ )
		{
			// the object to draw
			sObject* pObject = ppSortedObjectVisibleList [ iObject ];
			if ( pObject==NULL )
				continue;

			// Add to checklist
			if(bCreateChecklistNow)
			{
				// record id of frame index
				g_pGlob->checklist[g_pGlob->checklistqty].valuea = (int)pObject->dwObjectNumber;

				// reserved
				g_pGlob->checklist[g_pGlob->checklistqty].valueb = 0;
				g_pGlob->checklist[g_pGlob->checklistqty].valuec = 0;
				g_pGlob->checklist[g_pGlob->checklistqty].valued = 0;
			}
			g_pGlob->checklistqty++;
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

DARKSDK_DLL void HideLimb ( int iID, int iLimbID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return;

	// actual object or instance of object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceMeshVisible )
	{
		// record hide state in instance-mesh-visibility-array
		sObject* pActualObject = pObject->pInstanceOfObject;
		if ( iLimbID>=0 && iLimbID<pActualObject->iFrameCount)
			pObject->pInstanceMeshVisible [ iLimbID ] = false;
	}
	else
	{
		// ensure limb has mesh
		sMesh* pMesh = pObject->ppFrameList [ iLimbID ]->pMesh;
		if ( !pMesh )
			return;

		// apply to specific mesh
		Hide ( pMesh );

	}
}

DARKSDK_DLL void ShowLimb ( int iID, int iLimbID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return;

	// actual object or instance of object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceMeshVisible )
	{
		// record hide state in instance-mesh-visibility-array
		sObject* pActualObject = pObject->pInstanceOfObject;
		if ( iLimbID>=0 && iLimbID<pActualObject->iFrameCount)
			pObject->pInstanceMeshVisible [ iLimbID ] = true;
	}
	else
	{
		// ensure limb has mesh
		sMesh* pMesh = pObject->ppFrameList [ iLimbID ]->pMesh;
		if ( !pMesh )
			return;

		// apply to specific mesh
		Show ( pMesh );

	}
}

DARKSDK_DLL void TextureLimb ( int iID, int iLimbID, int iImageID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	SetBaseTexture ( pMesh, -1, iImageID );
}

DARKSDK_DLL void TextureLimbStage ( int iID, int iLimbID, int iStage, int iImageID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	SetBaseTextureStage ( pMesh, iStage, iImageID );
}

DARKSDK_DLL void ColorLimb ( int iID, int iLimbID, DWORD dwColor )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	SetBaseColor ( pMesh, dwColor );
}

DARKSDK_DLL void ScrollLimbTexture ( int iID, int iLimbID, int iStage, float fU, float fV )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	ScrollTexture ( pMesh, iStage, fU, fV );
}

DARKSDK_DLL void ScrollLimbTexture ( int iID, int iLimbID, float fU, float fV )
{
	// refers to core function above
	ScrollLimbTexture ( iID, iLimbID, 0, fU, fV );
}

DARKSDK_DLL void ScaleLimbTexture ( int iID, int iLimbID, int iStage, float fU, float fV )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	ScaleTexture ( pMesh, iStage, fU, fV );
}

DARKSDK_DLL void ScaleLimbTexture ( int iID, int iLimbID, float fU, float fV )
{
	// refers to core function above
	ScaleLimbTexture ( iID, iLimbID, 0, fU, fV );
}

DARKSDK_DLL void SetLimbSmoothing ( int iID, int iLimbID, float fPercentage )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// limit percentage range
	if ( fPercentage<0.0f ) fPercentage=0.0f;
	if ( fPercentage>100.0f ) fPercentage=100.0f;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	SmoothNormals ( pMesh, fPercentage/100.0f );
}

DARKSDK_DLL void SetLimbNormals ( int iID, int iLimbID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// apply to specific mesh
	GenerateNormals ( pMesh, 0 );
}

DARKSDK_DLL void OffsetLimb ( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundsFlag )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// apply to specific frame
	Offset ( g_ObjectList [ iID ]->ppFrameList [ iLimbID ], fX, fY, fZ );
	g_ObjectList [ iID ]->bAnimUpdateOnce = true;

	// leefix - 230604 - u54 - copy user matrix to combined matrix
	g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matCombined = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matUserMatrix;

	// u55 - 080704 - under flag for best of both worlds
	if ( iBoundsFlag==1 ) CalculateAllBounds ( g_ObjectList [ iID ], false );
	//#endif
}

DARKSDK_DLL void OffsetLimb ( int iID, int iLimbID, float fX, float fY, float fZ )
{
	OffsetLimb ( iID, iLimbID, fX, fY, fZ, 0 );
}

DARKSDK_DLL void RotateLimb ( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundsFlag )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// apply to specific frame
	Rotate ( g_ObjectList [ iID ]->ppFrameList [ iLimbID ], fX, fY, fZ );

	// leefix - 230604 - u54 - copy user matrix to combined matrix
    if ( g_ObjectList [ iID ]->position.bApplyPivot==false )
	{
		// 270614 - used applypivot to prevent matCombined getting wiped out during ragdoll creation
		g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matCombined = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matUserMatrix;
	}
	g_ObjectList [ iID ]->bAnimUpdateOnce = true;

	// u55 - 080704 - under flag for best of both worlds
	if ( iBoundsFlag==1 ) CalculateAllBounds ( g_ObjectList [ iID ], false );
}

DARKSDK_DLL void RotateLimb ( int iID, int iLimbID, float fX, float fY, float fZ )
{
	RotateLimb ( iID, iLimbID, fX, fY, fZ, 0 );
}

DARKSDK_DLL void ScaleLimb ( int iID, int iLimbID, float fX, float fY, float fZ, int iBoundsFlag )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// scale down
	fX /= 100.0f;
	fY /= 100.0f;
	fZ /= 100.0f;

	// apply to specific frame
	Scale ( g_ObjectList [ iID ]->ppFrameList [ iLimbID ], fX, fY, fZ );

	// leefix - 230604 - u54 - copy user matrix to combined matrix
	g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matCombined = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->matUserMatrix;
	g_ObjectList [ iID ]->bAnimUpdateOnce = true;

	// u55 - 080704 - under flag for best of both worlds
	if ( iBoundsFlag==1 ) CalculateAllBounds ( g_ObjectList [ iID ], false );
}

DARKSDK_DLL void ScaleLimb ( int iID, int iLimbID, float fX, float fY, float fZ )
{
	ScaleLimb ( iID, iLimbID, fX, fY, fZ, 0 );
}

DARKSDK_DLL void AddLimb ( int iID, int iLimbID, int iMeshID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// next available frame is
	int iAvailableFrame = pObject->iFrameCount;

	// if frame specified, must be correct
	if ( iLimbID > -1 )
	{
		if ( iLimbID!=iAvailableFrame )
		{
			RunTimeError ( RUNTIMEERROR_LIMBMUSTCHAININSEQUENCE );
		}
	}

	// and the mesh to create frame with
	sMesh* pNewMesh = new sMesh;
	if ( pNewMesh==NULL )
		return;

	// no transform of new limb
	GGMATRIX matWorld;
	GGMatrixIdentity ( &matWorld );

	// make a copy of the mesh
	MakeMeshFromOtherMesh ( true, pNewMesh, g_RawMeshList [ iMeshID ], &matWorld );

	// add new frame to end of 
	if ( !AddNewFrame ( pObject, pNewMesh, "new limb" ) )
	{
		// could not make limb
		RunTimeError ( RUNTIMEERROR_B3DMESHTOOLARGE );
		SAFE_DELETE ( pNewMesh );
	}

	// recreate all mesh and frame lists
	CreateFrameAndMeshList ( pObject );

	// ensure bounds are recalculated
	pObject->bUpdateOverallBounds=true;

	// update mesh(es) of object
	m_ObjectManager.RefreshObjectInBuffer ( pObject );
}

DARKSDK_DLL void RemoveLimb ( int iID, int iLimbID )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// cannot remove root frame
	if ( iLimbID==0 )
	{
		// could not remove limb zero
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return;
	}

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// free mesh resources
	if ( pObject->ppFrameList [ iLimbID ] )
	{
		if ( pObject->ppFrameList [ iLimbID ]->pMesh )
		{
			// frees buffers used to render mesh to backbuffer
			m_ObjectManager.RemoveBuffersUsedByObjectMesh ( pObject->ppFrameList [ iLimbID ]->pMesh );
		}
	}

	// remove frame from object
	if ( !RemoveFrame ( pObject, pObject->ppFrameList [ iLimbID ] ) )
	{
		// could not remove limb
		RunTimeError ( RUNTIMEERROR_LIMBNOTEXIST );
		return;
	}

	// recreate all meshand frame lists
	CreateFrameAndMeshList ( pObject );

	// ensure bounds are recalculated
	pObject->bUpdateOverallBounds=true;

	// update mesh(es) of object
	m_ObjectManager.RefreshObjectInBuffer ( pObject );
}

DARKSDK_DLL void LinkLimbEx ( int iID, int iParentID, int iLimbID, bool bDoNotReconstructMeshFrameList )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iParentID ) )
		return;

	// cannot move root frame
	if ( iLimbID==0 )
	{
		// could not remove limb zero
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return;
	}

	// cannot to ones self
	if ( iLimbID==iParentID )
	{
		// could not remove limb zero
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return;
	}

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// link frame from object
	if ( !LinkFrame ( pObject, pObject->ppFrameList [ iLimbID ], pObject->ppFrameList [ iParentID ] ) )
	{
		// could not link limb
		RunTimeError ( RUNTIMEERROR_LIMBNOTEXIST );
		return;
	}

	// recreate all meshand frame lists
	if (bDoNotReconstructMeshFrameList == false)
	{
		CreateFrameAndMeshList(pObject);
		pObject->bUpdateOverallBounds = true;
	}
}

DARKSDK_DLL void LinkLimb(int iID, int iParentID, int iLimbID)
{
	LinkLimbEx(iID, iParentID, iLimbID, false);
}

DARKSDK_DLL void ChangeLimbName	( int iID, int iLimbID, LPSTR pNewName )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// cannot move root frame
	if ( iLimbID==0 )
	{
		// could not remove limb zero
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return;
	}

	// change limb name
	sObject* pObject = g_ObjectList [ iID ];
	if (pObject)
	{
		strcpy(pObject->ppFrameList[iLimbID]->szName, pNewName);
	}
}

//
// MESH Commands
//

DARKSDK_DLL void LoadMeshCore ( SDK_LPSTR szFilename, int iMeshID )
{
	/*
	// ensure the mesh is okay to use
	ConfirmNewMesh ( iMeshID );

	// check memory allocation
	ID_MESH_ALLOCATION ( iMeshID );

	// load the mesh from an x file
	if ( !LoadRawMesh ( (LPSTR)szFilename, &g_RawMeshList [ iMeshID ] ) )
	{
		// pass mesh filename
		char pExtraErr[512];
		wsprintf ( pExtraErr, "LOAD MESH '%s',%d", szFilename, iMeshID );
		RunTimeError ( RUNTIMEERROR_B3DMESHNOTEXIST, pExtraErr );
		return;
	}
	*/
}

DARKSDK_DLL void DeleteMesh ( int iMeshID )
{
	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// delete the mesh
	if ( !DeleteRawMesh ( g_RawMeshList [ iMeshID ] ) )
		return;

	// free the mesh from the list
	g_RawMeshList [ iMeshID ] = NULL;
}

/*
DARKSDK_DLL void LoadMesh ( LPSTR szFilename, int iMeshID )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	//g_pGlob->UpdateFilenameFromVirtualTable( VirtualFilename);

	CheckForWorkshopFile (VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( VirtualFilename );
	LoadMeshCore ( (SDK_LPSTR)VirtualFilename, iMeshID );
	g_pGlob->Encrypt( VirtualFilename );
}

DARKSDK_DLL void SaveMesh ( LPSTR pFilename, int iMeshID )
{
	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// save mesh as an x file
	if ( !SaveRawMesh ( pFilename, g_RawMeshList [ iMeshID ] ) )
		return;
}
*/

DARKSDK_DLL void ChangeMesh ( int iObjectID, int iLimbID, int iMeshID )
{
	// check the limb exists
	if ( !ConfirmObjectAndLimb ( iObjectID, iLimbID ) )
		return;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iObjectID ];
	sFrame* pFrameOfMeshToReplace = pObject->ppFrameList [ iLimbID ];
	sMesh* pOldMesh = pFrameOfMeshToReplace->pMesh;

	// 310817 - new mesh eater mode
	if ( iMeshID == 0 )
	{
		// keep existing mesh, just reset to zero
		for ( int iVertIndex = 0; iVertIndex < pOldMesh->dwVertexCount; iVertIndex++ )
		{
			*((float*)pOldMesh->pVertexData+(iVertIndex*(pOldMesh->dwFVFSize/4))+0) = 0.0f;
			*((float*)pOldMesh->pVertexData+(iVertIndex*(pOldMesh->dwFVFSize/4))+1) = 0.0f;
			*((float*)pOldMesh->pVertexData+(iVertIndex*(pOldMesh->dwFVFSize/4))+2) = 0.0f;
		}
		pOldMesh->bVBRefreshRequired = true;
	}
	else
	{
		// create new mesh
		sMesh* pNewMesh = new sMesh;
		if ( pNewMesh==NULL )
			return;

		// no transform of new limb
		GGMATRIX matWorld;
		GGMatrixIdentity ( &matWorld );

		// make a copy of the mesh
		MakeMeshFromOtherMesh ( true, pNewMesh, g_RawMeshList [ iMeshID ], &matWorld );

		// lee - 280306 - u6rc2 - if specify limb with no mesh, exit now
		if ( pOldMesh==NULL )
		{
			// no way to detect if mesh in limb, so silent fail this
			RunTimeWarning ( RUNTIMEERROR_B3DLIMBBUTNOMESH );
			SAFE_DELETE ( pNewMesh );
			return;
		}

		// create a texture-set for new mesh
		DWORD dwTextureCount = pOldMesh->dwTextureCount;
		pNewMesh->pTextures = new sTexture [ dwTextureCount ];
		pNewMesh->dwTextureCount = dwTextureCount;

		// lee - 200206 - u60 - extract all material and texture information from old mesh
		CloneInternalTextures ( pNewMesh, pOldMesh );
		CopyMeshSettings ( pNewMesh, pFrameOfMeshToReplace->pMesh );

		// 200603 - remove drawbuffer from mesh to be replaced
		m_ObjectManager.RemoveBuffersUsedByObjectMesh ( pFrameOfMeshToReplace->pMesh );

		// replace mesh
		if ( !ReplaceFrameMesh ( pFrameOfMeshToReplace, pNewMesh ) )
		{
			// failed to change mesh
			SAFE_DELETE ( pNewMesh );
		}

		// recreate all meshand frame lists
		CreateFrameAndMeshList ( pObject );

		// calculate bounding areas of object
		CalculateAllBounds ( pObject, false );
	}

	// update mesh(es) of object
	m_ObjectManager.RenewReplacedMeshes ( pObject );
}

DARKSDK_DLL void ConvertMeshToVertexData ( int iMeshID )
{
	// leeadd - 140405 - check the mesh not exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// do the conversion
	if ( g_RawMeshList ) ConvertLocalMeshToVertsOnly ( g_RawMeshList [ iMeshID ], false ); 
}

DARKSDK_DLL void MakeMeshFromObject ( int iMeshID, int iObjectID, int iIgnoreMode )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iObjectID ) )
		return;

	// check the mesh not exists
	if ( !ConfirmNewMesh ( iMeshID ) )
		return;

    // Get a pointer to the object (or if an instance, that instances object)
    sObject* pObject = g_ObjectList [ iObjectID ];
    if (pObject->pInstanceOfObject)
        pObject = pObject->pInstanceOfObject;

	// create new mesh
	sMesh* pNewMesh = NULL;
	if ( !CreateSingleMeshFromObject ( &pNewMesh, pObject, iIgnoreMode ) )
		return;

	// leeadd - 080405 - convert final mesh to vert only (for ODE trimesh support)
	// lee, interferes with fpsc-mapeditor blueprint, needs index data (and should keep it)
	// so move this to later in the trimesh making, as only ODE needs this!
	// maybe a new ObjectDLL command CONVERT OBJECT TO VERTEXDATA Object Number
	// ConvertLocalMeshToVertsOnly ( pNewMesh ); 
	// leeadd - 141008 - u70 - also make mesh from sphere and cylinder object need this - so put back in this case!
	if ( pNewMesh->iPrimitiveType!=GGPT_TRIANGLELIST )
		ConvertLocalMeshToVertsOnly ( pNewMesh, false ); 

	// check memory allocation
	ID_MESH_ALLOCATION ( iMeshID );

	// if full, delete contents
	if ( g_RawMeshList [ iMeshID ] )
	{
		SAFE_DELETE ( g_RawMeshList [ iMeshID ] );
	}

	// assign new mesh to rawmeshlist
	g_RawMeshList[iMeshID] = pNewMesh;
}

DARKSDK_DLL void MakeMeshFromLimb ( int iMeshID, int iObjectID, int iLimbNumber )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iObjectID, iLimbNumber ) )
		return;

	// check the mesh not exists
	if ( !ConfirmNewMesh ( iMeshID ) )
		return;

    // Get a pointer to the object (or if an instance, that instances object)
    sObject* pObject = g_ObjectList [ iObjectID ];
    if (pObject->pInstanceOfObject)
        pObject = pObject->pInstanceOfObject;

	// early out if no mesh associated with the limb specified
	if ( pObject->ppFrameList )
		if ( pObject->ppFrameList[iLimbNumber] )
			if ( pObject->ppFrameList[iLimbNumber]->pMesh==NULL )
				return;

	// create new mesh
	sMesh* pNewMesh = NULL;
	if ( !CreateSingleMeshFromLimb ( &pNewMesh, pObject, iLimbNumber, 0 ) )
		return;

	// leeadd - 080405 - convert final mesh to vert only (for ODE trimesh support)
	// lee, interferes with fpsc-mapeditor blueprint, needs index data (and should keep it)
	// so move this to later in the trimesh making, as only ODE needs this!
	// maybe a new ObjectDLL command CONVERT OBJECT TO VERTEXDATA Object Number
	// ConvertLocalMeshToVertsOnly ( pNewMesh ); 
	// leeadd - 141008 - u70 - also make mesh from sphere and cylinder object need this - so put back in this case!
	if ( pNewMesh->iPrimitiveType!=GGPT_TRIANGLELIST )
		ConvertLocalMeshToVertsOnly ( pNewMesh, false ); 

	// check memory allocation
	ID_MESH_ALLOCATION ( iMeshID );

	// if full, delete contents
	if ( g_RawMeshList [ iMeshID ] )
	{
		SAFE_DELETE ( g_RawMeshList [ iMeshID ] );
	}

	// assign new mesh to rawmeshlist
	g_RawMeshList[iMeshID] = pNewMesh;
}

DARKSDK_DLL void MakeMeshFromObject ( int iMeshID, int iObjectID )
{
	// ignore nothing
	MakeMeshFromObject ( iMeshID, iObjectID, 0 );
}

DARKSDK_DLL void StealMeshesFromObject ( int iMasterObjectID, int iDonerObjectID )
{
	// this will transfer the meshes from one object to another, and erase references to those meshes
	// so the doner object can be safely deleted (ideal for merging two objects that share frames/anims into one)
	if ( !ConfirmObject ( iMasterObjectID ) && !ConfirmObject ( iDonerObjectID ) )
		return;

	// master object ptr
	sObject* pMasterObject = GetObjectData ( iMasterObjectID );

	// find last sibling to connect to
	sFrame* pFrameLinkage = pMasterObject->pFrame;
	while ( pFrameLinkage->pSibling ) pFrameLinkage = pFrameLinkage->pSibling;

	// go through all doner meshes
	sObject* pDonerObject = GetObjectData ( iDonerObjectID );
	for ( int iDonerFrameIndex = 0; iDonerFrameIndex < pDonerObject->iFrameCount; iDonerFrameIndex++ )
	{
		sFrame* pDonerFrame = pDonerObject->ppFrameList[iDonerFrameIndex];
		if ( pDonerFrame )
		{
			sMesh* pDonerMesh = pDonerFrame->pMesh;
			if ( pDonerMesh )
			{
				// add a new frame to root as sybling
				sFrame* pNewMasterFrame = new sFrame;
				pFrameLinkage->pSibling = pNewMasterFrame;
				strcpy ( pNewMasterFrame->szName, pDonerFrame->szName );
				pNewMasterFrame->pMesh = pDonerMesh;

				// erase reference from parent doner object
				pDonerFrame->pMesh = NULL;

				// and from list
				for ( int iMesh = 0; iMesh < pDonerObject->iMeshCount; iMesh++ )
				{
					sMesh* pMesh = pDonerObject->ppMeshList[iMesh];
					if ( pMesh == pDonerMesh ) pDonerObject->ppMeshList[iMesh] = NULL;
				}
			}
		}
	}

	// create frames and mesh lists from frame hierarchy modified above
	CreateFrameAndMeshList ( pMasterObject );

	// go through new meshes and associate with the frames of the master object
	if ( pMasterObject->ppMeshList )
	{
		InitFramesToBones ( pMasterObject->ppMeshList, pMasterObject->iMeshCount );
		MapFramesToBones ( pMasterObject->ppMeshList, pMasterObject->pFrame, pMasterObject->iMeshCount );
	}
}

DARKSDK_DLL void ReduceMesh ( int iMeshID, int iBlockMode, int iNearMode, int iGX, int iGY, int iGZ )
{
	//	reduce mesh 1,blockmode,nearmode,gx,gy,gz

	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// reduce mesh as instructed
	ReduceMeshPolygons ( g_RawMeshList [ iMeshID ], iBlockMode, iNearMode, iGX, iGY, iGZ );
}

// Mesh/Limb Manipulation Commands

DBPRO_GLOBAL sObject*	g_pCurrentVertexDataObject		= NULL;
DBPRO_GLOBAL sMesh*		g_pCurrentVertexDataMesh		= NULL;
DBPRO_GLOBAL int		g_iCurrentVertexDataUpdateMode	= 0;

DARKSDK_DLL void LockVertexDataForLimbCore ( int iID, int iLimbID, int iReplaceOrUpdate )
{
	// check the object limb exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return;

	// assign mesh of limb for editing
	g_pCurrentVertexDataMesh = pMesh;
	g_pCurrentVertexDataObject = g_ObjectList [ iID ];
	g_iCurrentVertexDataUpdateMode = iReplaceOrUpdate;
}

DARKSDK_DLL void LockVertexDataForLimb ( int iID, int iLimbID )
{
	// lock vertex in basic replace mode
	LockVertexDataForLimbCore ( iID, iLimbID, 1 );
}

DARKSDK_DLL void LockVertexDataForMesh ( int iMeshID )
{
	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// assign mesh of rawmesh for editing
	g_pCurrentVertexDataMesh = g_RawMeshList [ iMeshID ];
	g_pCurrentVertexDataObject = NULL;
	g_iCurrentVertexDataUpdateMode = 0;
}

DARKSDK_DLL void UnlockVertexData ( void )
{
	// mike - 010903 - need to update bounds or object may not be visible
//	CalculateAllBounds ( g_pCurrentVertexDataObject, true ); //slow slow slow

	// MIKE - 040204 - temporary fix for physics
	// LEE - 190204 - allow manual control over '1update' or '0replace' (until find physics issue)
	if ( g_pCurrentVertexDataObject )
	{
		if ( g_iCurrentVertexDataUpdateMode==2 )
		{
			// leeadd - 010306 - u60 - update original copy if flagged
			SAFE_DELETE ( g_pCurrentVertexDataMesh->pOriginalVertexData );
			CollectOriginalVertexData ( g_pCurrentVertexDataMesh );

			// not quite as fast as mode 1, but good for saving final results
			g_pCurrentVertexDataMesh->bVBRefreshRequired = true;
			g_vRefreshMeshList.push_back ( g_pCurrentVertexDataMesh );
		}
		else
		{
			if ( g_iCurrentVertexDataUpdateMode==1 )
			{
				// mesh VB update - same size, just adjusted data - fast
				g_pCurrentVertexDataMesh->bVBRefreshRequired = true;
				g_vRefreshMeshList.push_back ( g_pCurrentVertexDataMesh );
			}
			else
			{
				// complete mesh replace - slow
				g_pCurrentVertexDataObject->bReplaceObjectFromBuffers = true;
				m_ObjectManager.g_bObjectReplacedUpdateBuffers = true;
			}
		}
	}

	// end edit session
	g_pCurrentVertexDataMesh = NULL;
	g_pCurrentVertexDataObject = NULL;
	g_iCurrentVertexDataUpdateMode = 0;
}

DARKSDK_DLL void SetVertexDataPosition ( int iVertex, float fX, float fY, float fZ )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iVertex<0 || iVertex>=(int)g_pCurrentVertexDataMesh->dwVertexCount ) return;

	SetPositionData ( g_pCurrentVertexDataMesh, iVertex, fX, fY, fZ );
}

DARKSDK_DLL void SetVertexDataNormals ( int iVertex, float fNX, float fNY, float fNZ )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iVertex<0 || iVertex>=(int)g_pCurrentVertexDataMesh->dwVertexCount ) return;
	SetNormalsData ( g_pCurrentVertexDataMesh, iVertex, fNX, fNY, fNZ );
}

DARKSDK_DLL void SetVertexDataDiffuse	( int iVertex, DWORD dwDiffuse )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iVertex<0 || iVertex>=(int)g_pCurrentVertexDataMesh->dwVertexCount ) return;
	SetDiffuseData ( g_pCurrentVertexDataMesh, iVertex, dwDiffuse );
}

DARKSDK_DLL void SetIndexData ( int iIndex, int iValue )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iIndex<0 || iIndex>=(int)g_pCurrentVertexDataMesh->dwIndexCount ) return;
	g_pCurrentVertexDataMesh->pIndices [ iIndex ] = iValue;
}

DARKSDK_DLL void SetVertexDataUV ( int iVertex, float fU, float fV )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iVertex<0 || iVertex>=(int)g_pCurrentVertexDataMesh->dwVertexCount ) return;

	SetUVData ( g_pCurrentVertexDataMesh, iVertex, fU, fV );
}

DARKSDK_DLL void SetVertexDataUV ( int iVertex, int iIndex, float fU, float fV )
{
	// write directly to mesh
	if ( !g_pCurrentVertexDataMesh ) return;
	if ( iVertex<0 || iVertex>=(int)g_pCurrentVertexDataMesh->dwVertexCount ) return;

	// only stages 0 to 7
	if ( iIndex >= 8 )
		return;

	// convert mesh if not supporting the stage
	bool bOkay=false;
	for ( int iI=0; iI<=iIndex; iI++ )
	{
		if ( iI==0 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & GGFVF_TEX1 )  bOkay=true;
		if ( iI==1 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & GGFVF_TEX2 )  bOkay=true;
		if ( iI==2 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & GGFVF_TEX3 )  bOkay=true;
		#ifdef DX11
		#else
		if ( iI==3 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & D3DFVF_TEX4 )  bOkay=true;
		if ( iI==4 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & D3DFVF_TEX5 )  bOkay=true;
		if ( iI==5 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & D3DFVF_TEX6 )  bOkay=true;
		if ( iI==6 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & D3DFVF_TEX7 )  bOkay=true;
		if ( iI==7 && iIndex<=iI && g_pCurrentVertexDataMesh->dwFVF & D3DFVF_TEX8 )  bOkay=true;
		#endif
	}
	if ( bOkay==false )
	{
		// convert to correct format
		DWORD dwFVF = g_pCurrentVertexDataMesh->dwFVF;
		if ( iIndex==0 ) dwFVF = dwFVF | GGFVF_TEX1;
		if ( iIndex==1 ) dwFVF = dwFVF | GGFVF_TEX2;
		if ( iIndex==2 ) dwFVF = dwFVF | GGFVF_TEX3;
		#ifdef DX11
		#else
		if ( iIndex==3 ) dwFVF = dwFVF | D3DFVF_TEX4;
		if ( iIndex==4 ) dwFVF = dwFVF | D3DFVF_TEX5;
		if ( iIndex==5 ) dwFVF = dwFVF | D3DFVF_TEX6;
		if ( iIndex==6 ) dwFVF = dwFVF | D3DFVF_TEX7;
		if ( iIndex==7 ) dwFVF = dwFVF | D3DFVF_TEX8;
		#endif
		ConvertLocalMeshToFVF ( g_pCurrentVertexDataMesh, dwFVF );
	}

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( g_pCurrentVertexDataMesh, &offsetMap );

	// make sure we have an offset to write UVs
	if ( offsetMap.dwTU[iIndex]>0 )
	{
		// set single UV vertex component
		*( ( float* ) g_pCurrentVertexDataMesh->pVertexData + offsetMap.dwTU[iIndex] + ( offsetMap.dwSize * iVertex ) ) = fU;
		*( ( float* ) g_pCurrentVertexDataMesh->pVertexData + offsetMap.dwTV[iIndex] + ( offsetMap.dwSize * iVertex ) ) = fV;
	}

	// flag mesh for a VB update
	g_pCurrentVertexDataMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( g_pCurrentVertexDataMesh );
}

DARKSDK_DLL void SetVertexDataSize ( int iVertex, float fSize )
{
	// mike - 160903 - set size for point sprites

	if ( !g_pCurrentVertexDataMesh ) return;

	sOffsetMap offsetMap;
	GetFVFOffsetMap ( g_pCurrentVertexDataMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( g_pCurrentVertexDataMesh->dwFVF & GGFVF_PSIZE )
	{
		if ( iVertex < (int)g_pCurrentVertexDataMesh->dwVertexCount )
			*( ( float* ) g_pCurrentVertexDataMesh->pVertexData + offsetMap.dwPointSize + ( offsetMap.dwSize * iVertex ) ) = fSize;
	}

	// flag mesh for a VB update
	g_pCurrentVertexDataMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( g_pCurrentVertexDataMesh );
}

DARKSDK_DLL void AddMeshToVertexData ( int iMeshID )
{
	// check the mesh exists
	if ( !ConfirmMesh ( iMeshID ) )
		return;

	// write directly to mesh
	if ( g_pCurrentVertexDataMesh )
		if ( !AddMeshToData ( g_pCurrentVertexDataMesh, g_RawMeshList [ iMeshID ] ) )
			return;

	// must renew mesh
	if ( g_pCurrentVertexDataObject )
		m_ObjectManager.RenewReplacedMeshes ( g_pCurrentVertexDataObject );
}

DARKSDK_DLL void DeleteMeshFromVertexData ( int iVertex1, int iVertex2, int iIndex1, int iIndex2 )
{
	// write directly to mesh
	if ( g_pCurrentVertexDataMesh )
		if ( !DeleteMeshFromData ( g_pCurrentVertexDataMesh, iVertex1, iVertex2, iIndex1, iIndex2 ) )
			return;

	// must renew mesh
	if ( g_pCurrentVertexDataObject )
		m_ObjectManager.RenewReplacedMeshes ( g_pCurrentVertexDataObject );
}

DARKSDK_DLL int ObjectBlocking	( int iID, float X1, float Y1, float Z1, float X2, float Y2, float Z2 )
{
	// default is not blocking
	bool bResult = false;

	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return false;

	// check ALL meshes
	sObject* pObject = g_ObjectList [ iID ];
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
		if ( CheckIfMeshBlocking ( pObject->ppMeshList [ iMesh ], X1, Y1, Z1, X2, Y2, Z2 )==true )
			bResult=true;

	// result
	return bResult;
}

DARKSDK_DLL void AddMemblockToObject ( int iMemblock, int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// check memblock DLL exists
	/*if ( !MemblockExist )
		return;*/

	// check memblock X exists
	if ( !MemblockExist ( iMemblock ) )
		return;

	// release any previous custom data
	SAFE_DELETE ( pObject->pCustomData );

	// create custom data from memblock
	DWORD dwMemBlockSize = GetMemblockSize ( iMemblock );
	pObject->dwCustomSize = dwMemBlockSize + 8;
	pObject->pCustomData = new char [ pObject->dwCustomSize ];
	LPSTR pMemData = (LPSTR)GetMemblockPtr ( iMemblock );
	BYTE* pBytePtr = (BYTE*)pObject->pCustomData;
	*((DWORD*)pBytePtr+0 ) = DBOBLOCK_OBJECT_CUSTOMDATA; // custom token
	memcpy ( pBytePtr+4, pMemData, dwMemBlockSize ); // memblock data
	*((DWORD*)(pBytePtr+4+dwMemBlockSize) ) = 0; // terminator
}

DARKSDK_DLL void GetMemblockFromObject ( int iMemblock, int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// ensure we have custom data
	if ( !pObject->pCustomData )
		return;

	// check call ptr to memblock DLL exists
	/*if ( !MemblockExist )
		return;*/

	// 250413 - must be real membloc, not instance stamp custom data
	if ( pObject->dwCustomSize>4000000000 )
		return;

	// get custom data and get the data from it
	DWORD dwMemblockToken = *((DWORD*)(LPSTR)pObject->pCustomData+0 );
	LPSTR pMemblockData = (LPSTR)pObject->pCustomData+4;
	DWORD dwMemBlockSize = pObject->dwCustomSize-8;

	// token must be correct
	if ( dwMemblockToken!=DBOBLOCK_OBJECT_CUSTOMDATA )
		return;

	// check memblock X exists
	if ( MemblockExist ( iMemblock ) )
	{
		// delete it as we are creating a new one
		DeleteMemblock ( iMemblock );
	}	

	// make memblock
	MakeMemblock ( iMemblock, dwMemBlockSize );

	// read data into memblock from custom data in object
	LPSTR pMemData = (LPSTR)GetMemblockPtr ( iMemblock );
	BYTE* pBytePtr = (BYTE*)pObject->pCustomData;
	memcpy ( pMemData, pBytePtr+4, dwMemBlockSize );
}

DARKSDK_DLL void DeleteMemblockFromObject ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// ensure we have custom data
	if ( !pObject->pCustomData )
		return;

	// clear custom data
	SAFE_DELETE ( pObject->pCustomData );
	pObject->dwCustomSize = 0;
}

void SetObjectStatisticsInteger ( int iID, int iIndex, int dwValue )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->dwCustomSize==0 )
	{
		// create custom slot
		DWORD dwStatisticsDataSize = 8;
		pObject->dwCustomSize = dwStatisticsDataSize*-1;
		pObject->pCustomData = (LPVOID)new DWORD[dwStatisticsDataSize];
		for ( DWORD i=0; i<dwStatisticsDataSize; i++ )
			*(((DWORD*)pObject->pCustomData)+i) = 0;
	}
	if ( pObject->dwCustomSize>4000000000 )
	{
		*(((DWORD*)pObject->pCustomData)+iIndex) = dwValue;
	}
}

DARKSDK_DLL int ObjectStatisticsInteger ( int iID, int iIndex )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// ensure we have custom data
	int iResult = 0;
	if ( pObject->dwCustomSize>4000000000 )
	{
		if ( pObject->pCustomData )
		{
			iResult = *(((int*)pObject->pCustomData)+iIndex);
		}
	}
	return iResult;
}


// Mesh/Limb Manipulation Expressions

DARKSDK_DLL int GetVertexDataVertexCount ( void )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	return GetVertexCount ( g_pCurrentVertexDataMesh );
}

DARKSDK_DLL int GetVertexDataIndexCount ( void )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	return GetIndexCount ( g_pCurrentVertexDataMesh );
}

DARKSDK_DLL float GetVertexDataPositionX ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataPositionX ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataPositionY ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataPositionY ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataPositionZ ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataPositionZ ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataNormalsX	( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataNormalsX ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataNormalsY	( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataNormalsY ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataNormalsZ	( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataNormalsZ ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL DWORD GetVertexDataDiffuse ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	return GetDataDiffuse ( g_pCurrentVertexDataMesh, iVertex );
}

DARKSDK_DLL float GetVertexDataU ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataU ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataV ( int iVertex )
{
	// return vertexdata value
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataV ( g_pCurrentVertexDataMesh, iVertex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataU ( int iVertex, int iIndex )
{
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataU ( g_pCurrentVertexDataMesh, iVertex, iIndex );
	return fValue;
}

DARKSDK_DLL float GetVertexDataV ( int iVertex, int iIndex )
{
	if ( !g_pCurrentVertexDataMesh ) return 0;
	float fValue = GetDataV ( g_pCurrentVertexDataMesh, iVertex, iIndex );
	return fValue;
}

DARKSDK_DLL int GetIndexData ( int iIndex )
{
	if ( !g_pCurrentVertexDataMesh ) return 0;
	if ( iIndex<0 || iIndex>=(int)g_pCurrentVertexDataMesh->dwIndexCount ) return 0;
	return g_pCurrentVertexDataMesh->pIndices [ iIndex ];
}

DARKSDK_DLL DWORD GetVertexDataPtr ( void )
{
	return (DWORD)g_pCurrentVertexDataMesh;
}

// Misc Commands

DARKSDK_DLL void SetFastBoundsCalculation ( int iMode )
{
	if ( iMode==1 )
		g_bFastBoundsCalculation = true;
	else
		g_bFastBoundsCalculation = false;
}

DARKSDK_DLL void SetMipmapMode ( int iMode )
{
	// OBSOLETE
}

DARKSDK_DLL void FlushVideoMemory ( void )
{
	// OBSOLETE
}

DARKSDK_DLL void DisableTNL ( void )
{
	// OBSOLETE
}

DARKSDK_DLL void EnableTNL ( void )
{
	// OBSOLETE
}

DARKSDK_DLL void Convert3DStoX ( DWORD pFilename1, DWORD pFilename2 )
{
	// OBSOLETE
}

DARKSDK_DLL int PickScreenObjectEx ( int iX, int iY, int iObjectStart, int iObjectEnd, int iIgnoreCamera, int iIgnoreAllButLastFrame )
{
	// result hit
	int iObjectHit=0;
    bool bPickingLocked = false;

	// leefix - 010306 - u60 - camera X (actually checked in U6 and it is; dwCurrentSetCameraID )
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

    // u74b7 - Save the FOV as may need to reset it later
    float fCurrentFov = m_Camera_Ptr->fFOV;

	// calculate line from camera to diatant point through projected XY
	GGVECTOR3 vecFrom = m_Camera_Ptr->vecPosition;

	// Calculate inverse to take 2D into 3D
	GGMATRIX matInvertedView = m_Camera_Ptr->matView;
	GGMATRIX matInvertedProjection = m_Camera_Ptr->matProjection;
	GGMatrixInverse ( &matInvertedView, NULL, &matInvertedView );
	GGMatrixInverse ( &matInvertedProjection, NULL, &matInvertedProjection );

	// leeadd - 130306 - u60 - great god lee adds camera independence
	if ( iIgnoreCamera == 1 )
	{
        // u74b7 - In this mode, we are looking for locked objects only using the default FOV
        bPickingLocked = true;
        if (fCurrentFov != GGToDegree(3.14159265358979323846f/2.905f) )
        {
    		SetCameraFOV ( GGToDegree(3.14159265358979323846f/2.905f) );
        }

		// null camera (facing forward)
		vecFrom = GGVECTOR3(0,0,0);
		GGMatrixIdentity ( &matInvertedView );
		matInvertedProjection = m_Camera_Ptr->matProjection;
		GGMatrixInverse ( &matInvertedView, NULL, &matInvertedView );
		GGMatrixInverse ( &matInvertedProjection, NULL, &matInvertedProjection );
	}

	// Transform destination vector into screen
	float fHWidth = m_Camera_Ptr->viewPort3D.Width/2;
	float fHHeight = m_Camera_Ptr->viewPort3D.Height/2;
	iX = iX - m_Camera_Ptr->viewPort3D.X;
	iY = iY - m_Camera_Ptr->viewPort3D.Y;
	iX = iX - fHWidth; iY = fHHeight - iY;
	GGVECTOR3 vecTo = GGVECTOR3 ( (float)iX/fHWidth, (float)iY/fHHeight, 1.0f );
	GGVec3TransformCoord ( &vecTo, &vecTo, &matInvertedProjection );
	GGVec3TransformCoord ( &vecTo, &vecTo, &matInvertedView );
	vecTo = vecTo-vecFrom;
	GGVec3Normalize ( &vecTo, &vecTo );
	vecTo *= 15000.0f;
	vecTo = vecFrom + vecTo;

	// do intersect test against all objects in shortlist
	float fBestDistance = 999999.9f;
	for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iObjectID = g_ObjectListRef [ iShortList ];

		// only those within range
		if ( iObjectID>=iObjectStart && iObjectID<=iObjectEnd )
		{
			// actual object or instance of object
			sObject* pObject = g_ObjectList [ iObjectID ];

			// leeadd - 160504 - u6 - only pick a visible object
            // u74b7 - Allow locked if looking for locked, and vice-versa
			if ( !pObject->bVisible || pObject->bLockedObject != bPickingLocked )
				continue;

			// lee - 170117 - only pick an object with default collision ACTIVE
			if ( pObject->collision.bActive == false )
				continue;

			// see if we have a valid list
			float fThisDistance = IntersectObjectCore ( pObject, vecFrom.x, vecFrom.y, vecFrom.z, vecTo.x, vecTo.y, vecTo.z, iIgnoreAllButLastFrame );
			if ( fThisDistance>0.0f && fThisDistance < fBestDistance )
			{
				fBestDistance = fThisDistance;
				iObjectHit = iObjectID;
			}
		}
	}

	// generate useful data from pick
	if ( iObjectHit>0 )
	{
		// store normal vector with distance
		g_DBPROPickResult.iObjectID = iObjectHit;
		g_DBPROPickResult.fPickDistance = fBestDistance;
		g_DBPROPickResult.vecPickVector = vecTo - vecFrom;
		g_DBPROPickResult.vecFromVector = vecFrom;
		GGVec3Normalize ( &g_DBPROPickResult.vecPickVector, &g_DBPROPickResult.vecPickVector );
		g_DBPROPickResult.vecPickVector *= fBestDistance;
	}

    // Reset the FOV if necessary
    if (fCurrentFov != m_Camera_Ptr->fFOV)
    {
		SetCameraFOV ( fCurrentFov );
    }

	/// return result
	return iObjectHit;
}

DARKSDK_DLL int PickScreenObjectFromHeight(int iX, int iY, int iObjectStart, int iObjectEnd, float starty)
{
	// result hit
	int iObjectHit = 0;
	bool bPickingLocked = false;

	// leefix - 010306 - u60 - camera X (actually checked in U6 and it is; dwCurrentSetCameraID )
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData(g_pGlob->dwCurrentSetCameraID);

	// u74b7 - Save the FOV as may need to reset it later
	float fCurrentFov = m_Camera_Ptr->fFOV;

	// calculate line from camera to diatant point through projected XY
	GGVECTOR3 vecFrom = m_Camera_Ptr->vecPosition;

	// Calculate inverse to take 2D into 3D
	GGMATRIX matInvertedView = m_Camera_Ptr->matView;
	GGMATRIX matInvertedProjection = m_Camera_Ptr->matProjection;
	GGMatrixInverse(&matInvertedView, NULL, &matInvertedView);
	GGMatrixInverse(&matInvertedProjection, NULL, &matInvertedProjection);

	// Transform destination vector into screen
	float fHWidth = m_Camera_Ptr->viewPort3D.Width / 2;
	float fHHeight = m_Camera_Ptr->viewPort3D.Height / 2;
	iX = iX - m_Camera_Ptr->viewPort3D.X;
	iY = iY - m_Camera_Ptr->viewPort3D.Y;
	iX = iX - fHWidth; iY = fHHeight - iY;
	GGVECTOR3 vecTo = GGVECTOR3((float)iX / fHWidth, (float)iY / fHHeight, 1.0f);
	GGVec3TransformCoord(&vecTo, &vecTo, &matInvertedProjection);
	GGVec3TransformCoord(&vecTo, &vecTo, &matInvertedView);
	vecTo = vecTo - vecFrom;
	GGVec3Normalize(&vecTo, &vecTo);
	GGVECTOR3 vecFromInc = vecTo;
	vecTo *= 15000.0f;
	vecTo = vecFrom + vecTo;

	//PE: We need to walk vecFrom until we are below starty.
	vecFromInc *= 5.0; //walk quicker.
	int maxwalk = 5000;
	while (vecFrom.y > starty && vecFrom.y > 0 && vecFrom.y > vecTo.y && maxwalk-- > 0) {
		if ((vecFrom.y + vecFromInc.y) > starty) //Stop right before we hit the max.
			break;
		vecFrom += vecFromInc;
	}

	// do intersect test against all objects in shortlist
	float fBestDistance = 999999.9f;
	for (int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++)
	{
		// get index from shortlist
		int iObjectID = g_ObjectListRef[iShortList];

		// only those within range
		if (iObjectID >= iObjectStart && iObjectID <= iObjectEnd)
		{
			// actual object or instance of object
			sObject* pObject = g_ObjectList[iObjectID];

			// leeadd - 160504 - u6 - only pick a visible object
			// u74b7 - Allow locked if looking for locked, and vice-versa
			if (!pObject->bVisible || pObject->bLockedObject != bPickingLocked)
				continue;

			// lee - 170117 - only pick an object with default collision ACTIVE
			if (pObject->collision.bActive == false)
				continue;

			// see if we have a valid list
			float fThisDistance = IntersectObjectCore(pObject, vecFrom.x, vecFrom.y, vecFrom.z, vecTo.x, vecTo.y, vecTo.z, 0);
			if (fThisDistance > 0.0f && fThisDistance < fBestDistance)
			{
				fBestDistance = fThisDistance;
				iObjectHit = iObjectID;
			}
		}
	}

	// generate useful data from pick
	if (iObjectHit > 0)
	{
		// store normal vector with distance
		g_DBPROPickResult.iObjectID = iObjectHit;
		g_DBPROPickResult.fPickDistance = fBestDistance;
		g_DBPROPickResult.vecPickVector = vecTo - vecFrom;
		g_DBPROPickResult.vecFromVector = vecFrom;
		GGVec3Normalize(&g_DBPROPickResult.vecPickVector, &g_DBPROPickResult.vecPickVector);
		g_DBPROPickResult.vecPickVector *= fBestDistance;
	}

	// Reset the FOV if necessary
	if (fCurrentFov != m_Camera_Ptr->fFOV)
	{
		SetCameraFOV(fCurrentFov);
	}

	/// return result
	return iObjectHit;
}


DARKSDK_DLL int PickScreenObject ( int iX, int iY, int iObjectStart, int iObjectEnd )
{
	// see abive - this is the default behaviour
	return PickScreenObjectEx ( iX, iY, iObjectStart, iObjectEnd, 0, 0 );
}

DARKSDK_DLL void PickScreen2D23D ( int iX, int iY, float fDistance )
{
	// camera zero only
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	// calculate line from camera to diatant point through projected XY
	GGVECTOR3 vecFrom = m_Camera_Ptr->vecPosition;

	// Calculate inverse to take 2D into 3D
	GGMATRIX matInvertedView = m_Camera_Ptr->matView;
	GGMATRIX matInvertedProjection = m_Camera_Ptr->matProjection;
	GGMatrixInverse ( &matInvertedView, NULL, &matInvertedView );
	GGMatrixInverse ( &matInvertedProjection, NULL, &matInvertedProjection );

	// Transform destination vector into screen
	float fHWidth = m_Camera_Ptr->viewPort3D.Width/2;
	float fHHeight = m_Camera_Ptr->viewPort3D.Height/2;
	iX = iX - m_Camera_Ptr->viewPort3D.X;
	iY = iY - m_Camera_Ptr->viewPort3D.Y;
	iX = iX - fHWidth; iY = fHHeight - iY;
	GGVECTOR3 vecTo = GGVECTOR3 ( (float)iX/fHWidth, (float)iY/fHHeight, 1.0f );
	GGVec3TransformCoord ( &vecTo, &vecTo, &matInvertedProjection );
	GGVec3TransformCoord ( &vecTo, &vecTo, &matInvertedView );
	vecTo = vecTo-vecFrom;
	vecTo *= 3000.0f;

	// store normal vector with distance
	g_DBPROPickResult.iObjectID = 0;
	g_DBPROPickResult.fPickDistance = fDistance;
	g_DBPROPickResult.vecPickVector = vecTo - vecFrom;
	g_DBPROPickResult.vecFromVector = vecFrom;
	GGVec3Normalize ( &g_DBPROPickResult.vecPickVector, &g_DBPROPickResult.vecPickVector );
	g_DBPROPickResult.vecPickVector *= fDistance;
}

DARKSDK_DLL SDK_FLOAT GetPickDistance ( void )
{
	float fValue = g_DBPROPickResult.fPickDistance;
	return fValue;
}

DARKSDK_DLL SDK_FLOAT GetPickVectorX ( void )
{
	float fValue = g_DBPROPickResult.vecPickVector.x;
	return fValue;
}

DARKSDK_DLL SDK_FLOAT GetFromVectorX(void)
{
	float fValue = g_DBPROPickResult.vecFromVector.x;
	return fValue;
}

DARKSDK_DLL SDK_FLOAT GetPickVectorY ( void )
{
	float fValue = g_DBPROPickResult.vecPickVector.y;
	return fValue;
}

DARKSDK_DLL SDK_FLOAT GetFromVectorY(void)
{
	float fValue = g_DBPROPickResult.vecFromVector.y;
	return fValue;
}

DARKSDK_DLL SDK_FLOAT GetPickVectorZ ( void )
{
	float fValue = g_DBPROPickResult.vecPickVector.z;
	return fValue;
}
DARKSDK_DLL SDK_FLOAT GetFromVectorZ(void)
{
	float fValue = g_DBPROPickResult.vecFromVector.z;
	return fValue;
}

//////////////////////////////////////////////////////////////////////////////////
// EXPRESSIONS ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK_DLL int ObjectExist ( int iID )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{ 
		//PE: We use ObjectExist to check if a object can be used, so why this ? (LB: Just in case a dodgy index is passed in which can be caught and distinguished from an object that has not been created)
		//RunTimeError ( RUNTIMEERROR_B3DMODELNUMBERILLEGAL );
		return 0;
	}
	if ( iID < g_iObjectListCount )
	{ 
		if ( g_ObjectList [ iID ] )
		{
			if ( g_ObjectList [ iID ]->pFrame )
			{
				return 1;
			}
			if ( g_ObjectList [ iID ]->pInstanceOfObject )
			{
				return 1;
			}
		}
	}
	return 0;
}

DARKSDK_DLL int GetVisible ( int iID )
{
	// check the object exists
//	if ( !ConfirmObject ( iID ) ) // 190506 - u61 - instances have visibility
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// return object information
// lee - 101004 - U5.8 added universe state to visibility result
// V111 - 110608 - universe visibility messes up visibility checks in engine
//	if ( g_ObjectList [ iID ]->bUniverseVisible==true
//	&&   g_ObjectList [ iID ]->bVisible==true )
	if ( g_ObjectList [ iID ]->bVisible==true )
		return 1;
	else
		return 0;		
}


DARKSDK_DLL float ObjectSize ( int iID, int iActualSize )
{
    // check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->collision.fRadius;

	// if zero, still NEED a dimension, so calc now
	if ( fValue==0.0f )
	{
		float fDX = fabs(pObject->collision.vecMax.x - pObject->collision.vecMin.x);
		float fDY = fabs(pObject->collision.vecMax.y - pObject->collision.vecMin.y);
		float fDZ = fabs(pObject->collision.vecMax.z - pObject->collision.vecMin.z);
		fValue = fDX;
		if ( fDY>fValue ) fValue = fDY;
		if ( fDZ>fValue ) fValue = fDZ;
	}

	// scale optional
	if ( iActualSize==1 )
	{
		//050803 - adjusts to averaged scale now
		float fAvScale = (pObject->position.vecScale.x+pObject->position.vecScale.y+pObject->position.vecScale.z)/3.0f;
		fValue *= fAvScale;
	}
	return fValue;
}

DARKSDK_DLL float ObjectSizeX ( int iID, int iActualSize )
{
    return GetAxisSizeFromVectorOffset( iID, iActualSize, 0 /* 0 = x part of vector */);
}

DARKSDK_DLL float ObjectSizeY ( int iID, int iActualSize )
{
    return GetAxisSizeFromVectorOffset( iID, iActualSize, 1 /* 1 = y part of vector */ );
}

DARKSDK_DLL float ObjectSizeZ ( int iID, int iActualSize )
{
    return GetAxisSizeFromVectorOffset ( iID, iActualSize, 2 /* 2 = z part of vector */ );
}

DARKSDK_DLL float ObjectSize ( int iID )
{
	return ObjectSize ( iID, 0 );
}

DARKSDK_DLL float ObjectSizeX ( int iID )
{
	return ObjectSizeX ( iID, 0 );
}

DARKSDK_DLL float ObjectSizeY ( int iID )
{
	return ObjectSizeY ( iID, 0 );
}

DARKSDK_DLL float ObjectSizeZ ( int iID )
{
	return ObjectSizeZ ( iID, 0 );
}

DARKSDK_DLL float ObjectScaleX ( int iID )
{
    // check the object exists
	if ( !ConfirmObjectInstance ( iID ) ) return 0;
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->position.vecScale[ 0 ] * 100;
	return fValue;
}

DARKSDK_DLL float ObjectScaleY ( int iID )
{
    // check the object exists
	if ( !ConfirmObjectInstance ( iID ) ) return 0;
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->position.vecScale[ 1 ] * 100;
	return fValue;
}

DARKSDK_DLL float ObjectScaleZ ( int iID )
{
    // check the object exists
	if ( !ConfirmObjectInstance ( iID ) ) return 0;
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->position.vecScale[ 2 ] * 100;
	return fValue;
}

// Animation Expressions

DARKSDK_DLL int GetNumberOfFrames ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// leefix - 150503 - use better value, was pObject->pAnimationSet->ulLength;
	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	return (int)pObject->fAnimTotalFrames;
}

DARKSDK_DLL int GetPlaying ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->bAnimPlaying )
		return 1;
	else
		return 0;
}

DARKSDK_DLL int GetLooping ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->bAnimLooping )
		return 1;
	else
		return 0;
}

DARKSDK_DLL float GetFrame ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];

	float fValue = pObject->fAnimFrame;
	return fValue;
}

DARKSDK_DLL float GetFrameEx ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->fAnimFrame;

	// can set an animation frame override per limb
	if ( pObject->pfAnimLimbFrame )
	{
		float fViewValue = pObject->pfAnimLimbFrame [ iLimbID ];
		if ( fViewValue >= 0.0f ) fValue = fViewValue;
	}

	// return object information
	return fValue;
}

DARKSDK_DLL float GetSpeed ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information (leefix - 260604-u54-float conversion looses .000009)
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->fAnimSpeed*100.00001f;
	return fValue;
}

DARKSDK_DLL float GetInterpolation ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information (leefix - 260604-u54-float conversion looses .000009)
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = pObject->fAnimInterp*100.00001f;
	return fValue;
}

// ScreenData Expressions
DARKSDK_DLL void DB_ObjectScreenData( sObject* pObject, int* x, int* y )
{
	// object 3D position 
	GGVECTOR3 vecPos = pObject->position.vecPosition;

	// camera ptr
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	// get current camera transformation matrices
	GGMATRIX matTransform = m_Camera_Ptr->matView * m_Camera_Ptr->matProjection;

	// Transform object position from world-space to screen-space
	GGVec3TransformCoord(&vecPos, &vecPos, &matTransform);

	// Screen data
	*x=(int)((vecPos.x+1.0f)*(g_pGlob->iScreenWidth/2.0f));
	*y=(int)((1.0f-vecPos.y)*(g_pGlob->iScreenHeight/2.0f));

	// leefix - 280305 - adjust coordinates using viewport of the target camera
	float fVPWidth = m_Camera_Ptr->viewPort3D.Width;
	float fVPHeight = m_Camera_Ptr->viewPort3D.Height;
	float fRealX = ( fVPWidth / g_pGlob->iScreenWidth ) * (*x);
	float fRealY = ( fVPHeight / g_pGlob->iScreenHeight ) * (*y);
	*x = fRealX + m_Camera_Ptr->viewPort3D.X;
	*y = fRealY + m_Camera_Ptr->viewPort3D.Y;
}

DARKSDK_DLL int GetScreenX ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// calculate screendata
	int x, y;
	sObject* pObject = g_ObjectList [ iID ];
	DB_ObjectScreenData ( pObject, &x, &y );
	return x;
}

DARKSDK_DLL int GetScreenY ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// calculate screendata
	int x, y;
	sObject* pObject = g_ObjectList [ iID ];
	DB_ObjectScreenData ( pObject, &x, &y );
	return y;
}

DARKSDK_DLL int GetInScreen ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	sObject* pObject = g_ObjectList [ iID ];

    // u74b7 - Object centre needs to be adjusted by the mesh centre to give correct results
	GGVECTOR3 vBob = pObject->position.vecPosition + pObject->collision.vecCentre;

	// camera ptr
	tagCameraData* pCamera = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	// get current camera transformation matrices
	GGMATRIX matCamera = pCamera->matView * pCamera->matProjection;

    // u74b7 - Test whether the object 'sphere' in viewing frustum
    GGPLANE p_Planes[6];
    ExtractFrustumPlanes(p_Planes, matCamera);
    if (ContainsSphere(p_Planes, vBob, pObject->collision.fScaledRadius))
        return 1;
    else
        return 0;
}

// Collision Expressions

DARKSDK_DLL int GetCollision ( int iObjectA, int iObjectB )
{
	return CheckCol ( iObjectA, iObjectB );
}

DARKSDK_DLL int GetHit ( int iObjectA, int iObjectB )
{
	return CheckHit ( iObjectA, iObjectB );
}

DARKSDK_DLL int GetLimbCollision ( int iObjectA, int iLimbA, int iObjectB, int iLimbB )
{
	return CheckLimbCol ( iObjectA, iLimbA, iObjectB, iLimbB );
}

DARKSDK_DLL int GetLimbHit ( int iObjectA, int iLimbA, int iObjectB, int iLimbB )
{
	return CheckLimbHit ( iObjectA, iLimbA, iObjectB, iLimbB );
}

DARKSDK_DLL float GetObjectCollisionRadius ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = GetColRadius ( pObject );
	return fValue;
}

DARKSDK_DLL float GetObjectCollisionCenterX ( int iID )
{
	// check the object exists
	//if ( !ConfirmObject ( iID ) )
	//	return 0;

	// mike - 120307 - allow instanced objects
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = GetColCenterX ( pObject );
	return fValue;
}

DARKSDK_DLL float GetObjectCollisionCenterY ( int iID )
{
	// check the object exists
	//if ( !ConfirmObject ( iID ) )
	//	return 0;

	// mike - 120307 - allow instanced objects
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = GetColCenterY ( pObject );
	return fValue;
}

DARKSDK_DLL float GetObjectCollisionCenterZ ( int iID )
{
	// check the object exists
	//if ( !ConfirmObject ( iID ) )
	//	return 0;

	// mike - 120307 - allow instanced objects
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// return object information
	sObject* pObject = g_ObjectList [ iID ];
	float fValue = GetColCenterZ ( pObject );
	return fValue;
}

DARKSDK_DLL int GetStaticHit (	float fOldX1, float fOldY1, float fOldZ1, float fOldX2, float fOldY2, float fOldZ2,
					float fNX1,   float fNY1,   float fNZ1,   float fNX2,   float fNY2,   float fNZ2    )
{
	return GetStaticHitEx (	fOldX1, fOldY1, fOldZ1, fOldX2, fOldY2, fOldZ2,
							fNX1,   fNY1,   fNZ1,   fNX2,   fNY2,   fNZ2    );
}

DARKSDK_DLL int GetStaticLineOfSight ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz, float fWidth, float fAccuracy )
{
	return GetStaticLineOfSightEx( fSx, fSy, fSz, fDx, fDy, fDz, fWidth, fAccuracy );
}

DARKSDK_DLL int GetStaticRayCast ( float fSx, float fSy, float fSz, float fDx, float fDy, float fDz )
{
	// returns a one if collision, details in checklist
	return 0;
}

DARKSDK_DLL int GetStaticVolumeCast ( float fX, float fY, float fZ, float fNewX, float fNewY, float fNewZ, float fSize )
{
	return 0;
}

DARKSDK_DLL SDK_FLOAT GetStaticX ( void )
{
	float result = GetStaticColX();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetStaticY ( void )
{
	float result = GetStaticColY();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetStaticZ ( void )
{
	float result = GetStaticColZ();
	return result;
}

DARKSDK_DLL int GetStaticFloor ( void )
{
	return GetCollidedAgainstFloor();
}

DARKSDK_DLL int GetStaticColCount ( void )
{
	return GetStaticColPolysChecked();
}

DARKSDK_DLL int GetStaticColValue ( void )
{
	return (int)GetStaticColArbitaryValue();
}

DARKSDK_DLL SDK_FLOAT GetStaticLineOfSightX ( void )
{
	float result = GetStaticLineOfSightExX();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetStaticLineOfSightY ( void )
{
	float result = GetStaticLineOfSightExY();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetStaticLineOfSightZ ( void )
{
	float result = GetStaticLineOfSightExZ();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetCollisionX ( void )
{
	float result = GetColX();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetCollisionY ( void )
{
	float result = GetColY();
	return result;
}

DARKSDK_DLL SDK_FLOAT GetCollisionZ ( void )
{
	float result = GetColZ();
	return result;
}

// Limb Expressions

DARKSDK_DLL int LimbExist ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// check limb range
	if ( iLimbID < 0 || iLimbID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return 0;
	}

	// actual object or instance of object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceOfObject )
	{
		// record hide state in instance-mesh-visibility-array
		sObject* pActualObject = pObject->pInstanceOfObject;
		if ( iLimbID>=0 && iLimbID<pActualObject->iFrameCount)
			return 1;
		else
			return 0;
	}
	else
	{
		if ( iLimbID < g_ObjectList [ iID ]->iFrameCount )
			return 1;
		else
			return 0;
	}
}

DARKSDK_DLL float LimbOffsetX ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecOffset.x;
}

DARKSDK_DLL float LimbOffsetY ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecOffset.y;
}

DARKSDK_DLL float LimbOffsetZ ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecOffset.z;
}

DARKSDK_DLL float LimbAngleX ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecRotation.x;
}

DARKSDK_DLL float LimbAngleY ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecRotation.y;
}

DARKSDK_DLL float LimbAngleZ ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	return pFrame->vecRotation.z;
}

DARKSDK_DLL float LimbPositionX ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return 0;

	// actual or instanced
	sObject* pObject = g_ObjectList [ iID ];
	sObject* pActualObject = pObject;
	if ( pObject->pInstanceOfObject )
		pActualObject = pObject->pInstanceOfObject;

	// get frame of object
	sFrame* pFrame = pActualObject->ppFrameList [ iLimbID ];
	if ( pObject->pInstanceOfObject ) pFrame->bVectorsCalculated = false;

	// specific limb/frame information
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecPosition.x;
	return fValue;
}

DARKSDK_DLL float LimbPositionY ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return 0;

	// actual or instanced
	sObject* pObject = g_ObjectList [ iID ];
	sObject* pActualObject = pObject;
	if ( pObject->pInstanceOfObject )
		pActualObject = pObject->pInstanceOfObject;

	// get frame of object
	sFrame* pFrame = pActualObject->ppFrameList [ iLimbID ];
	if ( pObject->pInstanceOfObject ) pFrame->bVectorsCalculated = false;

	// specific limb/frame information
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecPosition.y;
	return fValue;

}

DARKSDK_DLL float LimbPositionZ ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return 0;

	// actual or instanced
	sObject* pObject = g_ObjectList [ iID ];
	sObject* pActualObject = pObject;
	if ( pObject->pInstanceOfObject )
		pActualObject = pObject->pInstanceOfObject;

	// get frame of object
	sFrame* pFrame = pActualObject->ppFrameList [ iLimbID ];
	if ( pObject->pInstanceOfObject ) pFrame->bVectorsCalculated = false;

	// specific limb/frame information
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecPosition.z;
	return fValue;

}

DARKSDK_DLL float LimbDirectionX ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecDirection.x;
	return fValue;
}

DARKSDK_DLL float LimbDirectionY ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecDirection.y;
	return fValue;
}

DARKSDK_DLL float LimbDirectionZ ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecDirection.z;
	return fValue;
}

DARKSDK_DLL float LimbScaleX ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecScale.x*100.0f;
	return fValue;
}

DARKSDK_DLL float LimbScaleY ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecScale.y*100.0f;
	return fValue;
}

DARKSDK_DLL float LimbScaleZ ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// specific limb/frame information
	sObject* pObject = g_ObjectList [ iID ];
	sFrame* pFrame = pObject->ppFrameList [ iLimbID ];
	UpdateRealtimeFrameVectors ( pObject, pFrame );
	float fValue = pFrame->vecScale.z*100.0f;
	return fValue;
}

DARKSDK_DLL int LimbTexture ( int iID, int iLimbID, int iTextureStage )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return 0;

	// Ensure iTextureStage is valid
	if ( iTextureStage < 0 && iTextureStage >= (int)pMesh->dwTextureCount )
		return NULL;

	// return stage zero texture image
	if ( pMesh->pTextures )
		return pMesh->pTextures [ iTextureStage ].iImageID;

	// no texture
	return 0;
}

DARKSDK_DLL int LimbTexture ( int iID, int iLimbID )
{
	return LimbTexture ( iID, iLimbID, 0 );
}

DARKSDK_DLL int GetLimbTexturePtr ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// ensure limb has mesh
	sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
	if ( !pMesh )
		return 0;

	// return ptr as int (for determining texture matching)
	int iPtrValue = 0;
	if ( pMesh->pTextures )
		iPtrValue = (int)pMesh->pTextures [ 0 ].pTexturesRef;
	return iPtrValue;
}

DARKSDK_DLL int LimbVisible ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return 0;

	// actual object or instance of object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject->pInstanceMeshVisible )
	{
		// record hide state in instance-mesh-visibility-array
		sObject* pActualObject = pObject->pInstanceOfObject;
		if ( iLimbID>=0 && iLimbID<pActualObject->iFrameCount)
		{
			if ( pObject->pInstanceMeshVisible [ iLimbID ] )
				return 1;
			else
				return 0;
		}
		else
			return 0;
	}
	else
	{
		// ensure limb has mesh
		sMesh* pMesh = g_ObjectList [ iID ]->ppFrameList [ iLimbID ]->pMesh;
		if ( !pMesh )
			return 0;

		// specific limb/frame information
		if ( pMesh->bVisible )
			return 1;
		else
			return 0;
	}
}

DARKSDK_DLL int LimbLink ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return 0;

	// can always modify links in limbs now
	return 1;
}

DARKSDK_DLL int GetLimbPolygonCount ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimbInstance ( iID, iLimbID ) )
		return 0;

	// actual object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject==NULL ) return 0;

	// get frame from object
	sFrame** pFrameList = pObject->ppFrameList;
	if ( pFrameList==NULL ) return 0;
	if ( iLimbID>=pObject->iFrameCount ) return 0;

	// get mesh from object
	sMesh* pMesh = pFrameList[ iLimbID ]->pMesh;
	if ( pMesh==NULL ) return 0;

	// return polygon count for this limb
	return pMesh->iDrawPrimitives;
}

DARKSDK_DLL int GetMultiMaterialCount ( int iID )
{
	// check the object exists
	if ( !ConfirmObjectInstance ( iID ) )
		return 0;

	// actual object
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject==NULL ) return 0;

	// add all multiaterial counts
	if ( pObject->pInstanceOfObject ) pObject = pObject->pInstanceOfObject;
	DWORD dwTotalMaterialCount = 0;
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh )
			if ( pMesh->bUseMultiMaterial==true )
				dwTotalMaterialCount+=pMesh->dwMultiMaterialCount;
	}

	// return total
	return dwTotalMaterialCount;
}

DARKSDK_DLL LPSTR LimbTextureName ( int iID, int iLimbID, int iTextureStage )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return NULL;
	
	// texture name to return
	LPSTR pTextureLimbName = NULL;

	// get ptrs
	sObject* pObject = g_ObjectList [ iID ];
	sMesh* pMesh = pObject->ppFrameList [ iLimbID ]->pMesh;
	if ( pMesh )
	{
		// lee - 220306 - u6b4 - if multimaterial, texturestage becomes index to material
		if ( pMesh->dwMultiMaterialCount > 0 && pMesh->pMultiMaterial )
		{
			// ensure materiual inde xis valid
			int iMaterialIndex = iTextureStage ; // re-use
			//if ( iMaterialIndex < 0 && iMaterialIndex >= (int)pMesh->dwMultiMaterialCount ) // leefix - 210806 - fixed condition
			if ( iMaterialIndex < 0 || iMaterialIndex >= (int)pMesh->dwMultiMaterialCount )
			{
				// skip
				pTextureLimbName = "";
			}
			else
			{
				// get name of texturename of material in framemesh
				pTextureLimbName = pMesh->pMultiMaterial [ iMaterialIndex ].pName;
			}
		}
		else
		{
			// Ensure iTextureStage is valid
			//if ( iTextureStage < 0 && iTextureStage >= (int)pMesh->dwTextureCount ) // leefix - 210806 - fixed condition
			if ( iTextureStage < 0 || iTextureStage >= (int)pMesh->dwTextureCount )
			{
				// skip
				pTextureLimbName = "";
			}
			else
			{
				// get name of texture in framemesh
				pTextureLimbName = "";
				if ( pMesh->pTextures )
					pTextureLimbName = pMesh->pTextures [ iTextureStage ].pName;
			}
		}
	}
	else
		pTextureLimbName = "";

	// Allocate new size
	LPSTR pString = NULL;
	DWORD dwSize = strlen ( pTextureLimbName );
	g_pGlob->CreateDeleteString((char**)&pString, dwSize+1);
	ZeroMemory ( pString, dwSize+1 );
	memcpy ( pString, pTextureLimbName, dwSize );

	// Return String
	return pString;
}

DARKSDK_DLL LPSTR LimbTextureName ( int iID, int iLimbID )
{
	return LimbTextureName ( iID, iLimbID, 0 );
	//#ifdef SDK_RETSTR
	// return GetLimbTextureNameEx ( lpStr, iID, iLimbID, 0 );
	//#else
	// return GetLimbTextureNameEx ( iID, iLimbID, 0 );
	//#endif
}

DARKSDK_DLL LPSTR LimbName ( int iID, int iLimbID )
{
	// check the object exists
	if ( !ConfirmObjectAndLimb ( iID, iLimbID ) )
		return NULL;
	
	// get name of frame
	sObject* pObject = g_ObjectList [ iID ];
	LPSTR pLimbName = pObject->ppFrameList [ iLimbID ]->szName;

	// Allocate new size
	LPSTR pString = NULL;
	DWORD dwSize = strlen ( pLimbName );
	g_pGlob->CreateDeleteString((char**)&pString, dwSize+1);
	ZeroMemory ( pString, dwSize+1 );
	memcpy ( pString, pLimbName, dwSize );

	// Return String
	return pString;
}

DARKSDK_DLL int GetLimbCount ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	// specific limb/frame information
	return pObject->iFrameCount;
}

// Mesh Expressions

DARKSDK_DLL int GetMeshExist ( int iID )
{
	if ( iID < g_iRawMeshListCount )
		if ( g_RawMeshList [ iID ] )
			return 1;

	return 0;
}

// Shader Expressions

DARKSDK_DLL SDK_BOOL PixelShaderExist ( int iShader )
{
	// if shader value valid
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return 0;
	}

	// if shader exists
	if ( m_PixelShaders [ iShader ].pPixelShader )
		return 1;
	else
		return 0;
}

DARKSDK_DLL SDK_BOOL VertexShaderExist ( int iShader )
{
	// if shader value valid
	if ( iShader < 0 || iShader > MAX_VERTEX_SHADERS )
	{
		RunTimeError(RUNTIMEERROR_B3DVSHADERNUMBERILLEGAL);
		return 0;
	}

	// if shader exists
	if ( m_VertexShaders [ iShader ].pVertexShader )
		return 1;
	else
		return 0;
}

// Caps Expressions

DARKSDK_DLL int Get3DBLITAvailable ( void )
{
	// OBSOLETE
	return 1;
}

DARKSDK_DLL int GetStatistic ( int iCode )
{
	// Active for hidden-values
	switch(iCode)
	{
		case 1 :	// Polycount monitored from scene geometry
					if ( g_pGlob )
						return g_pGlob->dwNumberOfPolygonsDrawn;
					else
						return 0;

		case 2 :	// Stencil buffer available in current mode
					return 0;

		case 3:		// Current Universe Area Box Of Camera Zero
					return 0;

		case 4:		// Total Number Of Universe Area Boxes
					return 0;

		case 5 :	// DrawPrimitive calls monitored from scene geometry
					if ( g_pGlob )
						return g_pGlob->dwNumberOfPrimCalls;
					else
						return 0;

		case 6 :	// Polygons from current areabox (universe)
					return 0;

		case 7:		// DrawPrimitive calls from current areabox (universe)
					return 0;

		case 8:		// Polygons tested for collision
					return 0;

		case 9:		// Volumes tested for collision
					return 0;	

		case 10:	// Occluded Objects
					return 0;	
	}
	return 0;
}

DARKSDK_DLL SDK_FLOAT GetPixelShaderVersion ( void )
{
	float fVersion = 0.0f;
	#ifdef DX11
	#else
	if(m_Caps.MaxStreams>0)
	{
		unsigned char sub = *((LPSTR)(&m_Caps.PixelShaderVersion));
		fVersion = *((LPSTR)(&m_Caps.PixelShaderVersion)+1);
		fVersion += ((float)sub/10.0f);
	}
	#endif
	return SDK_RETFLOAT ( fVersion );
}

DARKSDK_DLL SDK_FLOAT GetMaxPixelShaderValue ( void )
{
	#ifdef DX11
	return 0;
	#else
	return SDK_RETFLOAT ( m_Caps.PixelShader1xMaxValue );
	#endif
}

//
// Shadows
//

DARKSDK_DLL void SetGlobalShadowsOn ( void )
{
	m_ObjectManager.SetGlobalShadowsOn();
}

DARKSDK_DLL void SetGlobalShadowsOff ( void )
{
	m_ObjectManager.SetGlobalShadowsOff();
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// mike 090903 - now uses a pointer
DARKSDK_DLL void SetWorldMatrix ( int iID, GGMATRIX* pMatrix )
{
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];
	memcpy ( &pObject->position.matWorld, pMatrix, sizeof ( GGMATRIX ) );
	pObject->position.bCustomWorldMatrix = true;
}

// mike - 040903 - updates a structure
DARKSDK_DLL void UpdatePositionStructure ( sPositionData* pPosition )
{
	sObject object;

	object.position = *pPosition;

	CalculateObjectWorld ( &object, NULL );
}

DARKSDK_DLL void GetWorldMatrix ( int iID, int iLimb, GGMATRIX* pMatrix )
{
	if ( !ConfirmObject ( iID ) )
		return;

	sObject* pObject = g_ObjectList [ iID ];

	CalculateObjectWorld ( pObject, NULL );

	//CalculateAbsoluteWorldMatrix ( pObject, pObject->ppFrameList [ iLimb ], pObject->ppFrameList [ iLimb ]->pMesh );

	*pMatrix = pObject->ppFrameList [ iLimb ]->matCombined * pObject->position.matWorld;
}

DARKSDK_DLL GGVECTOR3 GetCameraLook ( void )
{
	tagCameraData* pCameraData = ( tagCameraData* ) GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	return pCameraData->vecLook;
}

DARKSDK_DLL GGVECTOR3 GetCameraPosition ( void )
{
	tagCameraData* pCameraData = ( tagCameraData* ) GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	return pCameraData->vecPosition;
}

DARKSDK_DLL GGVECTOR3 GetCameraUp ( void )
{
	tagCameraData* pCameraData = ( tagCameraData* ) GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	return pCameraData->vecUp;
}

DARKSDK_DLL GGVECTOR3 GetCameraRight ( void )
{
	tagCameraData* pCameraData = ( tagCameraData* ) GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	return pCameraData->vecRight;
}

DARKSDK_DLL GGMATRIX GetCameraMatrix ( void )
{
	tagCameraData* pCameraData = ( tagCameraData* ) GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );

	return pCameraData->matView;
}

DARKSDK_DLL void ExcludeOn ( int iID )
{
	if ( !CheckObjectExist ( iID ) )
		return;

	sObject* pObject = g_ObjectList [ iID ];
	pObject->bExcluded = true;
	if ( pObject->pInstanceOfObject ) pObject->pInstanceOfObject->bExcluded = true; // 131107 - added as seemd to be missing?

	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void ExcludeOff ( int iID )
{
	if ( !CheckObjectExist ( iID ) )
		return;

	sObject* pObject = g_ObjectList [ iID ];
	pObject->bExcluded = false;
	if ( pObject->pInstanceOfObject ) pObject->pInstanceOfObject->bExcluded = false;

	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL void ExcludeLimbOn ( int iID, int iLimbID )
{
	// 301007 - new command
	if ( !CheckObjectExist ( iID ) )
		return;

	// exclude limb if exists
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject )
		if ( pObject->ppFrameList )
			if ( iLimbID < pObject->iFrameCount )
				if ( pObject->ppFrameList [ iLimbID ] )
					pObject->ppFrameList [ iLimbID ]->bExcluded = true;
}

DARKSDK_DLL void ExcludeLimbOff	( int iID, int iLimbID )
{
	// 301007 - new command
	if ( !CheckObjectExist ( iID ) )
		return;

	// exclude limb if exists
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject )
		if ( pObject->ppFrameList )
			if ( iLimbID < pObject->iFrameCount )
				if ( pObject->ppFrameList [ iLimbID ] )
					pObject->ppFrameList [ iLimbID ]->bExcluded = false;
}

DARKSDK_DLL void SetGlobalObjectCreationMode ( int iMode )
{
    // If bit 1 isn't set, then allow individual buffers
    g_bGlobalVBIBUsageFlag = ! (iMode & 1);

    // Set the rendering order
	switch ( iMode )
    {
    // By Texture
    case 0:
    case 1:
        g_eGlobalSortOrder = E_SORT_BY_TEXTURE;
		break;

	// No particular order
    case 2:
    case 3:
        g_eGlobalSortOrder = E_SORT_BY_NONE;
		break;

	// By Object number
    case 4:
    case 5:
        g_eGlobalSortOrder = E_SORT_BY_OBJECT;
		break;

	// By Reverse Distance
    case 6:
    case 7:
        g_eGlobalSortOrder = E_SORT_BY_DEPTH;
		break;

    // Ignore anything else
    default:
        break;
	}
	m_ObjectManager.UpdateTextures ( );
}

DARKSDK_DLL sObject* GetObjectData ( int iID )
{
	// MIKE - 050104 - function to access an object
//	if ( !ConfirmObject ( iID ) ) // 180506 - u61 - tpc can get instanced objects
//	if ( !ConfirmObjectInstance ( iID ) )
//		return NULL;

	// lee - 040914 - needs to be silent fail
	if ( iID < 1 || iID > MAXIMUMVALUE )
		return NULL;

	if ( iID < g_iObjectListCount )
		return g_ObjectList [ iID ];
	else
		return NULL;
}

// mike - 230505 - need to be able to set mip map LOD bias on a per mesh basis
void SetObjectMipMapLODBias	( int iID, int iLimb, float fBias )
{
	// ensure the object is present
	if ( !ConfirmObject ( iID ) )
		return;

	// get object ptr
	sObject* pObject = g_ObjectList [ iID ];

	if ( iLimb == -1 )
	{
		for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
			pObject->ppMeshList [ iMesh ]->fMipMapLODBias = fBias;

		return;
	}

	if ( iLimb >= pObject->iMeshCount || iLimb < 0 )
		return;

	pObject->ppMeshList [ iLimb ]->fMipMapLODBias = fBias;
}

// mike - 230505 - need to be able to set mip map LOD bias on a per mesh basis
void SetObjectMipMapLODBias	( int iID, float fBias )
{
	SetObjectMipMapLODBias ( iID, -1, fBias );
}

// mike - 300905 - command to update object bounds
void CalculateObjectBounds ( int iID )
{
	// check the object limb exists
	if ( !ConfirmObject ( iID ) )
		return;

	CalculateAllBounds ( g_ObjectList [ iID ], false );
}

void CalculateObjectBoundsEx ( int iID, int iOnlyUpdateFrames )
{
	// check the object limb exists
	if ( !ConfirmObject ( iID ) )
		return;

	// only update frame matices OR update ALL BOUNDS (performance intense)
	if ( iOnlyUpdateFrames==1 )
	{
		// get object ptr
		sObject* pObject = g_ObjectList [ iID ];

		// store current animation states
		bool bStoreAnimPlaying = pObject->bAnimPlaying;
		float bStoreAnimFrame = pObject->fAnimFrame;
		bool bStoreAnimLooping = pObject->bAnimLooping;
		float bStoreAnimSlerpTime = pObject->fAnimSlerpTime;
		bool bStoreAnimManualSlerp = pObject->bAnimManualSlerp;

		// advance animation frame (only done at SYNC normally)
		// NOTE: This also updates the latest limb bounds for FAST ACCURATE limb detection :)
		if ( pObject->bExcluded==false && pObject->pAnimationSet )
		{
			// conditions moved outside of function to speed up (CPU 'call' is more expensive)
			m_ObjectManager.UpdateAnimationCyclePerObject ( pObject );
		}

		// simple update
		GGMATRIX matrix;
		GGMatrixIdentity ( &matrix );
		UpdateAllFrameData ( pObject, pObject->fAnimFrame );
		UpdateFrame ( pObject->pFrame, &matrix );

		// restore animation states
		pObject->bAnimPlaying = bStoreAnimPlaying;
		pObject->fAnimFrame = bStoreAnimFrame;
		pObject->bAnimLooping = bStoreAnimLooping;
		pObject->fAnimSlerpTime = bStoreAnimSlerpTime;
		pObject->bAnimManualSlerp = bStoreAnimManualSlerp;
	}
	else
	{
		// full bounds box/sphere calculation
		CalculateAllBounds ( g_ObjectList [ iID ], false );
	}
}

// lee - 140108 - x10 compat.
DARKSDK void SetObjectMask					( int iID, int iMASK, int iShadowMASK )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetObjectMask					( int iID, int iMASK, int iShadowMASK, int iCubeMapMASK )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetObjectMask ( int iID, int iMASK, int iShadowMASK, int iCubeMapMASK, int iForeColorWipe )
{
	// check the object exists
	// leefix - 211006 - allow instanced objects to be masked
	if ( !ConfirmObjectInstance ( iID ) )
		return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];

	// set regular mask, then the following
	SetObjectMask ( iID, iMASK );

	// set fore color wipe to all meshes in object
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh )
			if ( pMesh->pDrawBuffer )
				pMesh->pDrawBuffer->dwImmuneToForeColorWipe = (DWORD)iForeColorWipe;
	}

	// 210214 - the mask flag can remove object from sorted list (effectively removing it from all engine render considerations)
	m_ObjectManager.UpdateTextures();
}


DARKSDK void SetArrayMap				( int iID, int iStage, int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10 )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetArrayMapEx				( int iID, int iStage, int iSrcObject, int iSrcStage )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void Instance					( int iDestinationID, int iSourceID, int iInstanceValue )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetNodeTreeEffect			( int iEffectID )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void DrawSingle					( int iObjectID, int iCameraID )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void ResetStaticLights			( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void AddStaticLight				( int iIndex, float fX, float fY, float fZ, float fRange )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void UpdateStaticLights			( void )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetCharacterCreatorTones ( int iID, int index, float red, float green, float blue, float mix )
{
	// Character creator - Dave - 070515
	if ( index < 0 || index > 3 ) return;

	// check the object exists
	//if ( !ConfirmObjectInstance ( iID ) )
		//return;

	// apply setting to all meshes
	sObject* pObject = g_ObjectList [ iID ];
	if ( !pObject ) return;
	
	if ( !pObject->pCharacterCreator )
	{
		pObject->pCharacterCreator = new sObjectCharacterCreator;
		for ( int c = 0 ; c < 4 ; c++ )
		{
			pObject->pCharacterCreator->ColorTone[c][0] = -1;
			pObject->pCharacterCreator->ColorTone[c][1] = 0;
			pObject->pCharacterCreator->ColorTone[c][2] = 0;
			pObject->pCharacterCreator->ToneMix[c] = 0.0;
		}
	}

	pObject->pCharacterCreator->ColorTone[index][0] = red / 255.0f;
	pObject->pCharacterCreator->ColorTone[index][1] = green / 255.0f;
	pObject->pCharacterCreator->ColorTone[index][2] = blue / 255.0f;
	pObject->pCharacterCreator->ToneMix[index] = mix;
}

DARKSDK void SetLegacyMode ( int iUseLegacy )
{
	if ( iUseLegacy==0 )
		g_bSwitchLegacyOn = false;
	else
		g_bSwitchLegacyOn = true;
}

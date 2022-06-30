#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Dave moved these to earn £20 from Lee Bamber
#include "CObjectsC.h"
#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "DBPro Functions.h"

#include <windows.h>
#include "TreeFaceLightmapper.h"
#include <stdio.h>
#include "Thread.h"

#define DLLEXPORT 

#include "CollisionTreeLightmapper.h"
#include "LMObject.h"
#include "LMTexture.h"
#include "Light.h"
#include "SharedData.h"
#include "Thread.h"
#include "LightMapperThread.h"
#include "LMPolyGroup.h"
#include <process.h>

SharedData *g_pShared = NULL;
int g_iLightmapFileFormat = 0;	//0=png, 1=dds, 2=bmp
int g_iLightmapFileNumber = 0;
HANDLE g_hLMHeap = NULL;

extern LPGGDEVICE m_pD3D;
extern GlobStruct *g_pGlob;

CollisionTreeLightmapper cColTree;
TreeFaceLightmapper *pFaceList = 0;
int iNumFaces = 0;
int iTotalLMObjects = 0;
int iBlendMode = GGTOP_MODULATE;
char szLightmapName[256] = "";
char szLightmapFolderName[256] = "lightmaps\\";

LMObject *pLMObjectList = 0;
LMTexture *pLMTextureList = 0;
Light *pLightList = 0;

LightMapperThread *pLMThread = 0;
int g_iTexSize = 0;

bool bInitialised = false;
bool bLightmapInProgress = false;
char errStr[256];

// Light Mapping Single Cycle Process
int g_iBuildLightMapsCycleMode = 0;
LMObject* g_pBuildLightMapsCycleLMObject = NULL;
int g_iBuildLightMapsCycleCurrentObject = 0;
float g_fBuildLightMapsCycleQuality = 0.0f;
int g_iBuildLightMapsCycleBlur = 0;
bool g_bBuildLightMapsCycleSucceed = false;
int g_iBuildLightMapsCycleTexSize = 0;

// prototype
DLLEXPORT void LMCompleteLightMaps( );

void InvalidPointer ( void )
{
	MessageBox ( NULL, "Include At Least One Object Command In Your Code To Use Lightmap Commands", "Light Mapping Error", 0 );
	exit ( 1 );
}

void CheckLMInit( )
{
	if ( !bInitialised )
	{
		MessageBox( NULL, "You Must Call LMStart Before Other Light Map Commands", "Light Mapping Error", 0 );
		exit(1);
	}
}

bool CheckInProgress( )
{
	return bLightmapInProgress;
}

DLLEXPORT void LMStart ( )
{
	// U75 - 060510 - Use special heap during lightmapping, to avoid virtual address fragmentation
	// beyond the scope of the lightmapping process (self-contained except for the UV and Textures)
	if ( g_hLMHeap ) HeapDestroy ( g_hLMHeap );
	g_hLMHeap = HeapCreate(0,0,0);
	if ( bInitialised ) return;
	bInitialised = true;
}

DLLEXPORT void LMClearLights( )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	Light *pTempLight;
	while ( pLightList )
	{
		pTempLight = pLightList;
		pLightList = pLightList->pNextLight;
		delete pTempLight;
	}
}

DLLEXPORT void LMClearCollisionObjects( )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	// U75 - 060510 - memory allocated my LM heap, destroyed at end
	// cColTree.Reset( );
	cColTree.Reset( );
	
	TransparentFace::TextureClass* pStoredTexture = 0;
	while ( TransparentFace::pTextureList )
	{
		pStoredTexture = TransparentFace::pTextureList;
		TransparentFace::pTextureList = TransparentFace::pTextureList->pNextTexture;
		delete pStoredTexture;
	}
}

DLLEXPORT void LMClearLightMapObjects( )
{
	CheckLMInit( );
	if ( CheckInProgress( ) ) return;
	LMObject *pTempObject;
	while ( pLMObjectList )
	{
		pTempObject = pLMObjectList;
		pLMObjectList = pLMObjectList->pNextObject;
		delete pTempObject;
	}
	pLMObjectList = 0;
	iTotalLMObjects = 0;
}

void LMClearTextures( )
{
	// U75 - 060510 - seems the list ptr was deleted, need to delete ALL the instances
	//if ( pLMTextureList ) delete pLMTextureList;	
	if ( pLMTextureList )
	{
		LMTexture *pTempTexture;
		while ( pLMTextureList )
		{
			pTempTexture = pLMTextureList;
			pLMTextureList = pLMTextureList->pNextLMTex;
			pTempTexture->Free();
			HeapFree(g_hLMHeap, 0, pTempTexture);
		}
	}
	pLMTextureList = 0;
}

DLLEXPORT void LMSetMode( int iMode )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	//mode = 0 : combined (extend valid, clamp invalid) (default)
	//mode = 1 : extend all edges
	//mode = 2 : clamp all edges
	//mode = 3 : colour with normals

	if ( iMode < 0 || iMode > 3 ) iMode = 0;
	LMPolyGroup::iMode = iMode;
}

DLLEXPORT void LMSetShadowPower( float fPower )
{
	CheckLMInit( );

	MessageBox( NULL, "'LM Set Shadow Power' command has been removed to fix a bluring bug", "Lightmapper Error", 0 );
	exit(-1);

	if ( CheckInProgress( ) ) return;

	//higher powers reduce the area of shadows
	if ( fPower < 0 ) fPower = 0;
	LMPolyGroup::fShadowPower = fPower;
}

DLLEXPORT void LMBoostCurvedSurfaceQuality( float fMaxSize, float fBoost )
{
	if ( fBoost < 0.0001f ) fBoost = 0.0001f;
	LMPolyGroup::fCurvedBoost = fBoost;

	if ( fMaxSize < -1 ) fMaxSize = -1;
	LMPolyGroup::fMaxCurvedBoostSize = fMaxSize;
}

DLLEXPORT void LMSetBlendMode( int iNewMode )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( iNewMode < 1 ) iNewMode = 1;
	if ( iNewMode > 26 ) iNewMode = 26;
	iBlendMode = iNewMode;
}

//transparent type: 0=opaque, 1=black, 2=alpha
DLLEXPORT void LMAddCollisionObject( sObject *pObject, int iTransparent )
{
	LPSTR pAddStatus = "";
	try
	{
		pAddStatus = "CheckLMInit";
		CheckLMInit( );
		if ( CheckInProgress( ) ) return;
		if ( !pObject ) return;

		pAddStatus = "g_CalcObjectWorld";
		CalcObjectWorld ( pObject );
		sObject *pPosObject = pObject;
		if ( pObject->pInstanceOfObject ) pObject = pObject->pInstanceOfObject;

		// only use HIGHEST LOD if LOD_0 -> LOD_2 exists
		int iBestLOD = 99;
		int iBestLODFrame = 0;
		for ( DWORD iFrameScan = 0; iFrameScan < (DWORD)pObject->iFrameCount; iFrameScan++ )
		{
			LPSTR pFrameName = pObject->ppFrameList[iFrameScan]->szName;
			if ( stricmp ( pFrameName, "lod_0" )==NULL && iBestLOD>0 ) { iBestLOD = 0; iBestLODFrame=iFrameScan; }
			if ( stricmp ( pFrameName, "lod_1" )==NULL && iBestLOD>1 ) { iBestLOD = 1; iBestLODFrame=iFrameScan; }
			if ( stricmp ( pFrameName, "lod_2" )==NULL && iBestLOD>2 ) { iBestLOD = 2; iBestLODFrame=iFrameScan; }
		}

		// add ALL frames to collision object (unless LOD_X available)
		int iNumFrames = pObject->iFrameCount;
		for (int iFrame = 0; iFrame < iNumFrames; iFrame++ )
		{
			pAddStatus = "Valid frame and mesh check";
			sFrame* pFrame = pObject->ppFrameList [ iFrame ];
			if ( !pFrame ) continue;

			pAddStatus = "Reject if not highest LOD";
			if ( iBestLOD==99 || (iBestLOD!=99 && iFrame==iBestLODFrame) )
			{
				// allow this frame (99=all frames, 0,1,2=specific LOD frame only)
			}
			else
			{
				// skip this frame
				continue;
			}

			sMesh *pMesh = pObject->ppFrameList [ iFrame ]->pMesh;
			if ( !pMesh ) continue;

			pAddStatus = "CalculateAbsoluteWorldMatrix";
			CalculateAbsoluteWorldMatrix ( pPosObject, pObject->ppFrameList [ iFrame ], pObject->ppFrameList [ iFrame ]->pMesh );

			pAddStatus = "ConvertLocalMeshToVertsOnly";
			int iPrimitiveType = pMesh->iPrimitiveType;
			if ( iPrimitiveType == 5 )  ConvertLocalMeshToVertsOnly( pMesh, false );

			DWORD dwFVFSize = pMesh->dwFVFSize;
			int iNumVertices = pMesh->dwVertexCount;
			int iNumIndices = pMesh->dwIndexCount;
			BYTE *pVertexData = pMesh->pVertexData;
			WORD *pIndices = pMesh->pIndices;
			if ( dwFVFSize < 12 ) continue;
			if ( iNumVertices <= 0 ) continue;
			int iMax = iNumIndices>0 ? iNumIndices : iNumVertices;
			Point p1,p2,p3;
			float u1,u2,u3;
			float v1,v2,v3;
			int dwIndex;
			GGVECTOR3 vecVert;
			if ( iTransparent > 0 )
			{
				if ( pMesh->dwTextureCount < 1 ) 
				{
					iTransparent = 0;
				}
			}

			pAddStatus = "GetFVFOffsetMap";
			sOffsetMap	pOffsetMap;
			GetFVFOffsetMap ( pMesh, &pOffsetMap );

			for ( int i = 0; i < iMax; i+=3 )
			{
				pAddStatus = "GGVec3TransformCoord";
				if ( iNumIndices > 0 ) dwIndex = *(pIndices + i)*dwFVFSize; else dwIndex = i*dwFVFSize;
				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
				u1 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTU[0]*4 ) );
				v1 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTV[0]*4 ) );
				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				p1.x = vecVert.x; p1.y = vecVert.y; p1.z = vecVert.z;
				if ( iNumIndices > 0 ) dwIndex = *(pIndices + i + 1)*dwFVFSize; else dwIndex = (i+1)*dwFVFSize;
				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
				u2 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTU[0]*4 ) );
				v2 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTV[0]*4 ) );
				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				p2.x = vecVert.x; p2.y = vecVert.y; p2.z = vecVert.z;
				if ( iNumIndices > 0 ) dwIndex = *(pIndices + i + 2)*dwFVFSize; else dwIndex = (i+2)*dwFVFSize;
				vecVert.x = *( (float*) ( pVertexData + dwIndex )     );
				vecVert.y = *( (float*) ( pVertexData + dwIndex + 4 ) );
				vecVert.z = *( (float*) ( pVertexData + dwIndex + 8 ) );
				u3 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTU[0]*4 ) );
				v3 = *( (float*) ( pVertexData + dwIndex + pOffsetMap.dwTV[0]*4 ) );
				GGVec3TransformCoord( &vecVert, &vecVert, &pObject->ppFrameList [ iFrame ]->matAbsoluteWorld );
				p3.x = vecVert.x; p3.y = vecVert.y; p3.z = vecVert.z;

				TreeFaceLightmapper* aFace;
				bool bValid;
			
				if ( iTransparent > 0 ) 
				{
					pAddStatus = "TransparentFace";
					aFace = new TransparentFace( );
					bValid = ((TransparentFace*)aFace)->MakeTransparentFace( &p1, &p2, &p3, iTransparent - 1, u1, v1, u2, v2, u3, v3, &(pMesh->pTextures[0]) );
				}
				else 
				{
					pAddStatus = "TreeFaceLightmapper";
					aFace = new TreeFaceLightmapper( );
					bValid = aFace->MakeFace( &p1, &p2, &p3 );
				}

				//zero area polygons will have an undefined normal, which will cause problems later
				//so if any exist remove them
				pAddStatus = "delete aFace";
				if (!bValid) { delete aFace; continue; }
	        
				pAddStatus = "nextFace";
				aFace->nextFace = pFaceList;
				pFaceList = aFace;
				iNumFaces++;
			}
		}
	}
	catch(...)
	{
		//ignore for now..
		MessageBox ( NULL, pAddStatus, "LM Add Collision Object Error", MB_OK );
	}
}

DLLEXPORT void LMAddCollisionObject ( int iObjID )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !ObjectExist( iObjID ) ) 
	{
		sprintf_s( errStr, 255, "Object (%d) Does Not Exist", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	sObject *pObject = GetObjectData( iObjID );

	if ( !pObject )
	{
		sprintf_s( errStr, 255, "Could Not Get Object (%d) Pointer", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	LMAddCollisionObject( pObject, 0 );
}

DLLEXPORT void LMAddTransparentCollisionObject ( int iObjID, int iType )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !ObjectExist( iObjID ) ) 
	{
		sprintf_s( errStr, 255, "Object (%d) Does Not Exist", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	sObject *pObject = GetObjectData( iObjID );

	if ( !pObject )
	{
		sprintf_s( errStr, 255, "Could Not Get Object (%d) Pointer", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	if ( iType < 0 ) iType = 0;
	if ( iType > 2 ) iType = 2;

	LMAddCollisionObject( pObject, iType + 1 );
}

DLLEXPORT void LMBuildCollisionData( )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !pFaceList )
	{
		//MessageBox( NULL, "No Collision Data To Build", "Light Mapping Warning", 0 );
		//exit(0);
		return;
	}

	cColTree.setFacesPerNode( 2 );
	cColTree.makeCollisionObject( iNumFaces, pFaceList );
	pFaceList = 0;
	iNumFaces = 0;
}

DLLEXPORT void LMAddLightMapObject( int iObjID, sObject *pObject, int iBaseStage, int iDynamicLight, int iShaded, int iFlatNormals )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !pObject ) return;
	CalcObjectWorld ( pObject );
	
	if ( pObject->pInstanceOfObject )
	{
		//MessageBox ( NULL, "Cannot Lightmap Instance Objects", "Light Mapping Warning", 0 );
		return;
	}

	if ( iBaseStage < 0 ) iBaseStage = 0;
	if ( iBaseStage > 8 ) iBaseStage = 8;

	int iNumRealFrames = 0;
	int iNumFrames = pObject->iFrameCount;
	int iNumMeshes = pObject->iMeshCount;
	if ( iNumFrames < 0 ) return;

	int iNumStages = -1;
	int iMaxStages = 0;
	bool bWarned = false;
	DWORD objFVF = 0;

	// lee - 220914 - special shade mode to force that all normals are ignored for this object (terrain)
	bool bIgnoreNormals = false;
	if ( iFlatNormals==-1 )
	{
		bIgnoreNormals = true;
		iFlatNormals = 0;
	}

	for (int iFrame = 0; iFrame < iNumFrames; iFrame++ )
	{
		if ( !pObject->ppFrameList [ iFrame ] ) continue;
		if ( !pObject->ppFrameList [ iFrame ]->pMesh ) continue;

		int iNumVertices = pObject->ppFrameList [ iFrame ]->pMesh->dwVertexCount;		

		// lee - 261114 - no need to check in frames with meshes that have no polys
		if ( iNumVertices==0 ) continue;
		
		DWORD dwFVF = pObject->ppFrameList [ iFrame ]->pMesh->dwFVF;

		// U75 - 051109 - can send part-shaded objects into the light mapper *FPSC universe*
		// so detect if a non-standard-vertex shader is being used by detecting FVF=0 (iShaded=2=mixed shader)
		// however, FPSCV1 was never designed to allow lightmaps and BUMP.FX in same scene, so 
		// remove for the moment and focus on shaders properly in V117 and beyond
		// cannot mix! if ( dwFVF==0 ) iShaded = 2;

		if ( !(dwFVF & GGFVF_XYZ) ) continue;	

		if ( (iFlatNormals == 0) && !(dwFVF & GGFVF_NORMAL) ) 
		{
			iFlatNormals = 1;

			if ( !bWarned )
			{
				char str [ 256 ];
				sprintf_s( str, 256, "Object: %d, one or more limbs do not contain vertex normals data. Defaulting to flat shading for these limbs", iObjID );
				//MessageBox( NULL, str, "Light Mapping Warning", 0 );
				bWarned = true;
			}
		}
		if ( !iShaded && ((dwFVF & GGFVF_TEXCOUNT_MASK) < (((DWORD)iBaseStage+1) << GGFVF_TEXCOUNT_SHIFT)) ) 
		{
			iBaseStage = -1;
		}

		objFVF = dwFVF;

		if ( iShaded )
		{
			//check LMstage exists
			if ( (dwFVF & GGFVF_TEXCOUNT_MASK) < (((DWORD)iBaseStage+1) << GGFVF_TEXCOUNT_SHIFT) )
			{
				// skip to avoid crash in LM
				continue;
				//char str [ 256 ];
				//sprintf_s( str, 256, "Object: %d does not contain texture coordinates for stage %d, shaded lightmapping requires used stages to be set", iObjID, iBaseStage );
				//MessageBox( NULL, str, "Lightmapping Error", 0 );
				//exit(-1);
			}
		}
		if ( iNumVertices <= 0 ) continue;

		int iStages = pObject->ppFrameList [ iFrame ]->pMesh->dwTextureCount;
		if ( iNumStages < 0 || iStages < iNumStages ) iNumStages = iStages;
		if ( iStages > iMaxStages ) iMaxStages = iStages;

		if ( !iShaded && iBaseStage >= iStages ) iBaseStage = -1;
	}

	// U75 - 051109 - if mixed shaded, assume lightmapping at second stage
	// cannot mix! if ( iShaded==2 ) iBaseStage=1;

	// U75 - 051109 - note, only CHANGES FVF for non-shaded, not shaded or mixed-shaded (universe)
	if ( iShaded == 0 )
	{
		if ( iNumStages < 2 )
		{
			CloneMeshToNewFormat( iObjID, (objFVF & ~GGFVF_TEXCOUNT_MASK) | GGFVF_TEX2 );
			SetObjectBlendMap( iObjID, 1, 0, 0, 0 );
		}
	}

	for (int iFrame = 0; iFrame < iNumFrames; iFrame++ )
	{
		if ( !pObject->ppFrameList [ iFrame ] ) continue;
		if ( !pObject->ppFrameList [ iFrame ]->pMesh ) continue;

		int iNumVertices = pObject->ppFrameList [ iFrame ]->pMesh->dwVertexCount;		
		DWORD dwFVFSize = pObject->ppFrameList [ iFrame ]->pMesh->dwFVFSize;

		if ( dwFVFSize < 12 ) continue;
		if ( iNumVertices <= 0 ) continue;

		sMesh* pMesh = pObject->ppFrameList [ iFrame ]->pMesh;
		int iNumIndices = pMesh->dwIndexCount;
		if ( iNumIndices >= 65535 )
		{
			sprintf_s( errStr, 255, "Object (%d) Limb (%d) Contains More Than 21844 Polygons, This Is Likely To Crash. If So Reduce The Number Of Polygons Per Limb", iObjID, iFrame );
		}

		iNumRealFrames++;

		LMObject *pNewObject = new LMObject( pObject, pObject->ppFrameList [ iFrame ], pObject->ppFrameList [ iFrame ]->pMesh );

		pNewObject->bIgnoreNormals = bIgnoreNormals;
		if ( iShaded == 0 ) pNewObject->SetBaseStage( iBaseStage );
		else pNewObject->SetShaded( iBaseStage );
		pNewObject->SetDynamicLightMode( iDynamicLight );
		pNewObject->iID = iObjID;
		pNewObject->iLimbID = iFrame;
		pNewObject->BuildPolyList( iFlatNormals == 1 );
		pNewObject->pNextObject = pLMObjectList;
		pLMObjectList = pNewObject;

		iTotalLMObjects++;
	}
}

DLLEXPORT void LMAddLightMapObject ( int iObjID, int iBaseStage, int iDynamicLight, int iFlatShaded )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !ObjectExist( iObjID ) ) 
	{
		sprintf_s( errStr, 255, "Object (%d) Does Not Exist", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	sObject *pObject = GetObjectData( iObjID );

	if ( !pObject )
	{
		sprintf_s( errStr, 255, "Could Not Get Object (%d) Pointer", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	LMAddLightMapObject( iObjID, pObject, iBaseStage, iDynamicLight, 0, iFlatShaded );
}

DLLEXPORT void LMAddLightMapObject ( int iObjID )
{
	LMAddLightMapObject( iObjID, 0, 0, 0 );
}

DLLEXPORT void LMAddLightMapObject ( int iObjID, int iBaseStage )
{
	LMAddLightMapObject( iObjID, iBaseStage, 0, 0 );
}

DLLEXPORT void LMAddLightMapObject ( int iObjID, int iBaseStage, int iDynamicLight )
{
	LMAddLightMapObject( iObjID, iBaseStage, iDynamicLight, 0 );
}

DLLEXPORT void LMAddShadedLightMapObject ( int iObjID, int iLightMapStage, int iFlatShaded )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( !ObjectExist( iObjID ) ) 
	{
		sprintf_s( errStr, 255, "Object (%d) Does Not Exist", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	sObject *pObject = GetObjectData( iObjID );

	if ( !pObject )
	{
		sprintf_s( errStr, 255, "Could Not Get Object (%d) Pointer", iObjID );
		MessageBox( NULL, errStr, "Light Mapping Error", 0 );
		exit(1);
		return;
	}

	LMAddLightMapObject( iObjID, pObject, iLightMapStage, 0, 1, iFlatShaded );
}

DLLEXPORT void LMAddShadedLightMapObject ( int iObjID, int iLightMapStage )
{
	LMAddShadedLightMapObject ( iObjID, iLightMapStage, 0 );
}

DLLEXPORT void LMAddPointLight( float posX, float posY, float posZ, float radius, float red, float green, float blue )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	PointLight *pNewLight = new PointLight ( posX, posY, posZ, radius, radius, 16/(radius*radius), red, green, blue );	
	pNewLight->pNextLight = pLightList;
	pLightList = (Light*) pNewLight;
}

DLLEXPORT void LMAddCustomPointLight( float posX, float posY, float posZ, float radius, float attenuation, float attenuation2, float red, float green, float blue )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	PointLight *pNewLight = new PointLight ( posX, posY, posZ, radius, attenuation, attenuation2, red, green, blue );	
	pNewLight->pNextLight = pLightList;
	pLightList = (Light*) pNewLight;
}

DLLEXPORT void LMAddDirectionalLight( float dirX, float dirY, float dirZ, float red, float green, float blue )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	DirLight *pNewLight = new DirLight ( dirX, dirY, dirZ, red, green, blue );	
	pNewLight->pNextLight = pLightList;
	pLightList = (Light*) pNewLight;
}

DLLEXPORT void LMAddSpotLight( float posX, float posY, float posZ, float dirX, float dirY, float dirZ, float ang1, float ang2, float range, float red, float green, float blue )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	SpotLight *pNewLight = new SpotLight ( posX, posY, posZ, dirX, dirY, dirZ, ang1, ang2, range, red, green, blue );	
	pNewLight->pNextLight = pLightList;
	pLightList = (Light*) pNewLight;
}

DLLEXPORT void LMSetAmbientLight( float red, float green, float blue )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	Light::fAmbientR = red;
	Light::fAmbientG = green;
	Light::fAmbientB = blue;
}

DLLEXPORT void LMSetAmbientOcclusionOn( int iIterations, float fRayDist, int iPattern )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	srand( (unsigned int)time(0));

	LMPolyGroup::SetAmbientOcclusionOn( iIterations, fRayDist, iPattern );
}

DLLEXPORT void LMSetAmbientOcclusionOff( )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	LMPolyGroup::SetAmbientOcclusionOff( );
}

DLLEXPORT void LMSetLightMapName ( DWORD pInString )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;
	if ( !pInString ) return;

#pragma warning ( disable : 4312 )
	strcpy_s( szLightmapName, 256, (char*) pInString );
#pragma warning ( default : 4312 )
}

DLLEXPORT void LMSetLightMapFileFormat ( int iFormat )
{
	if ( iFormat < 0 || iFormat > 2 ) iFormat = 0;
	g_iLightmapFileFormat = iFormat;
}

DLLEXPORT void LMSetLightMapStartNumber ( int iFileNumber )
{
	g_iLightmapFileNumber = iFileNumber;
}

DLLEXPORT int LMGetLightMapLastNumber( )
{
	return g_iLightmapFileNumber;
}

DLLEXPORT void LMSetLightMapFolder ( LPSTR pInString )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;
	if ( !pInString ) return;

#pragma warning ( disable : 4312 )
	strcpy_s( szLightmapFolderName, 256, pInString );
#pragma warning ( default : 4312 )
}

DLLEXPORT void LMBuildLightMaps( int iTexSize, float fQuality, int iBlur, int iNumThreads )
{
	CheckLMInit( );
	if ( CheckInProgress( ) ) return;
	bLightmapInProgress = true;

	if ( iBlur < 0 ) iBlur = 0;
	if ( iBlur > 3 ) iBlur = 3;

	LMPolyGroup::SetPixelBorder( iBlur + 1 );

	if ( g_pShared ) g_pShared->SetComplete( false );

	int iCurrentObject = 0;
	char pInfoStr[256];

	if ( g_pShared ) g_pShared->SetStatus( "Starting Light Mapper...", 0 );

	if ( fQuality < 0.00001f ) fQuality = 0.00001f;
	if ( fQuality > 10000.0f ) fQuality = 10000.0f;
	fQuality = 1 / fQuality;

	if ( iTexSize < 32 ) iTexSize = 32;
	if ( pLMTextureList ) LMClearTextures( );

	// lee - 250914 - stored globally as only one size passed in per LM session
	g_iTexSize = iTexSize;

	// U75 - 060510 - new LM heap
	//pLMTextureList = new LMTexture( iTexSize, iTexSize );
	pLMTextureList = (LMTexture*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, sizeof(LMTexture));
	pLMTextureList->New( iTexSize, iTexSize );
	pLMTextureList->pNextLMTex = 0;

	//fit light map objects to textures
	LMObject *pLMObject = pLMObjectList;
	bool bSucceed = true;

	while ( pLMObject )
	{
		iCurrentObject++;

		// can cancel this loop manually by detecting the ESCAPE key
		short sExitEarlyKey = GetAsyncKeyState(VK_ESCAPE);
		if ( sExitEarlyKey!=0 )
		{
			// exit lightmapping early
			bLightmapInProgress = false;
			return;
		}

		if ( g_pShared ) 
		{
			sprintf_s( pInfoStr, 255, "Mapping Object [%d/%d]", iCurrentObject, iTotalLMObjects );
			g_pShared->SetStatus( pInfoStr, (48.0f*iCurrentObject)/iTotalLMObjects );
			if ( g_pShared->GetTerminate( ) ) 
			{
				bLightmapInProgress = false;
				g_pShared->SetComplete( true );
				return;
			}
		}

		pLMObject->GroupPolys( fQuality );
		pLMObject->SortGroups( );

		if ( pLMObject->GetNumPolys( ) == 0 ) 
		{
			pLMObject = pLMObject->pNextObject;
			continue;
		}

		float fNewQuality = fQuality;
		while ( !pLMTextureList->AddLMObject( pLMObject ) && fNewQuality < 1000 )
		{
			bSucceed = false;

			fNewQuality *= 1.5;
			pLMObject->ReScaleGroups( fNewQuality );

			while ( !pLMObject->WillFit( iTexSize, iTexSize ) && fNewQuality < 1000 )
			{
				fNewQuality *= 1.5;
				pLMObject->ReScaleGroups( fNewQuality );
			}
		}

		pLMObject = pLMObject->pNextObject;
	}

	if ( !bSucceed )
	{
		/* need to place it in writable area - g.mysystem.levelBankTestMap_s could have done it
		char fileStr[256];
		FILE *pQualityChangeFile;
		//GG_fopen_s( &pQualityChangeFile, "levelbank\\testmap\\lightmaps\\Quality Change.txt","w" );
		cstr lightmapQualityFile_s = g.mysystem.levelBankTestMap_s + "lightmaps\\Quality Change.txt";
		GG_fopen_s( &pQualityChangeFile, lightmapQualityFile_s.Get(), "w" );
		fputs( "Listed below are object limbs that were too large to fit onto a texture and had their quality reduced\n", pQualityChangeFile);
		fputs( "To allow the original quality to be used try splitting your object up into limbs and splitting large polygons into several smaller ones\n\n", pQualityChangeFile);

		pLMObject = pLMObjectList;
		while ( pLMObject )
		{
			if ( fabs( pLMObject->GetQuality( ) - fQuality ) > 0.0001 && pLMObject->GetNumPolys( ) > 0 )
			{
				if ( pLMObject->pLMTexture )
				{
					sprintf_s( fileStr, 255, "Object Number: %d, Limb Number: %d, New Quality: %f\n", pLMObject->iID, pLMObject->iLimbID, 1.0f/pLMObject->GetQuality( ) );
				}
				else
				{
					sprintf_s( fileStr, 255, "Object Number: %d, Limb Number: %d, FAILED\n", pLMObject->iID, pLMObject->iLimbID );
				}

				fputs( fileStr, pQualityChangeFile );
			}

			pLMObject = pLMObject->pNextObject;
		}

		fclose( pQualityChangeFile );
		*/
	}

	iCurrentObject = 0;
	pLMObject = pLMObjectList;

	// 281114 - if cancelled during mapping, can skip lighting altogether
	if ( bLightmapInProgress==true )
	{
		if ( iNumThreads > 0 )
		{
			//Multi Threaded
			HANDLE pSemaphore = CreateSemaphore( NULL, iNumThreads, iNumThreads, NULL );
			while ( pLMObject )
			{
				iCurrentObject++;

				// can cancel this loop manually by detecting the ESCAPE key
				short sExitEarlyKey = GetAsyncKeyState(VK_ESCAPE);
				if ( sExitEarlyKey!=0 )
				{
					// exit lightmapping early
					bLightmapInProgress = false;
					g_pShared->SetComplete( true );
					return;
				}

				if ( g_pShared ) 
				{
					sprintf_s( pInfoStr, 255, "Calculating Object Light [%d/%d]", iCurrentObject, iTotalLMObjects );
					g_pShared->SetStatus( pInfoStr, (48.0f*iCurrentObject)/iTotalLMObjects + 48 );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						return;
					}
				}

				if ( pLMObject->pLMTexture )
				{
					DWORD dwResult = WaitForSingleObject( pSemaphore, INFINITE );
					if ( dwResult != WAIT_OBJECT_0 )
					{
						MessageBox( NULL, "Multithreading Error", "Lightmapping Error", 0 );
						exit(-1);
					}

					pLMObject->SetLocalValues( pLightList, &cColTree, iBlur, pSemaphore );
					pLMObject->Start( );
				}

				pLMObject = pLMObject->pNextObject;
			}

			iCurrentObject = 0;
			pLMObject = pLMObjectList;

			while ( pLMObject )
			{
				iCurrentObject++;

				if ( g_pShared ) 
				{
					sprintf_s( pInfoStr, 255, "Waiting For Threads To Finish [%d/%d]", iCurrentObject, iTotalLMObjects );
					g_pShared->SetStatus( pInfoStr, (3.0f*iCurrentObject)/iTotalLMObjects + 96 );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						return;
					}
				}

				if ( pLMObject->pLMTexture )
				{
					pLMObject->Join( );
				}

				pLMObject = pLMObject->pNextObject;
			}

			CloseHandle( pSemaphore );
		}
		else
		{
			//Single Threaded
			while ( pLMObject )
			{
				iCurrentObject++;

				// can cancel this loop manually by detecting the ESCAPE key
				short sExitEarlyKey = GetAsyncKeyState(VK_ESCAPE);
				if ( sExitEarlyKey!=0 )
				{
					// exit lightmapping early
					bLightmapInProgress = false;
					return;
				}

				if ( g_pShared ) 
				{
					sprintf_s( pInfoStr, 255, "Calculating Object Light [%d/%d]", iCurrentObject, iTotalLMObjects );
					g_pShared->SetStatus( pInfoStr, (48.0f*iCurrentObject)/iTotalLMObjects + 48 );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						return;
					}
				}

				if ( pLMObject->pLMTexture )
				{
					pLMObject->CalculateLight( pLightList, &cColTree, iBlur, pLMObject->bIgnoreNormals, 0 );
					pLMObject->ApplyToTexture( );
				}

				pLMObject = pLMObject->pNextObject;
			}
		}
	}

	// carried on below (complete light maps)
	if ( g_pShared ) g_pShared->SetComplete( true );
}

DLLEXPORT void LMBuildLightMaps( int iTexSize, float fQuality, int iBlur )
{
	LMBuildLightMaps( iTexSize, fQuality, iBlur, 0 );
}

DLLEXPORT void LMBuildLightMapsThread( int iTexSize, float fQuality, int iBlur, int iNumThreads )
{
	CheckLMInit( );

	if ( CheckInProgress( ) ) return;

	if ( g_pShared ) delete g_pShared;
	g_pShared = new SharedData( );

	if ( pLMThread ) delete pLMThread;

	if ( iNumThreads < 0 )
	{
		SYSTEM_INFO sysInfo;
		GetSystemInfo( &sysInfo );
		iNumThreads = sysInfo.dwNumberOfProcessors + 1;
	}

	pLMThread = new LightMapperThread( );
	pLMThread->iTexSize = iTexSize;
	pLMThread->fQuality = fQuality;
	pLMThread->iBlur = iBlur;
	pLMThread->iNumThreads = iNumThreads;

	pLMThread->Start( );
}

DLLEXPORT void LMBuildLightMapsThread( int iTexSize, float fQuality, int iBlur )
{
	LMBuildLightMapsThread( iTexSize, fQuality, iBlur, 0 );
}

DLLEXPORT void LMBuildLightMapsStart( int iTexSize, float fQuality, int iBlur )
{
	CheckLMInit( );
	if ( CheckInProgress( ) ) return;
	bLightmapInProgress = true;

	if ( iBlur < 0 ) iBlur = 0;
	if ( iBlur > 3 ) iBlur = 3;
	LMPolyGroup::SetPixelBorder( iBlur + 1 );

	if ( g_pShared ) delete g_pShared;
	g_pShared = new SharedData( );

	if ( g_pShared ) g_pShared->SetComplete( false );
	if ( g_pShared ) g_pShared->SetStatus( "Starting Light Mapper...", 0 );

	if ( fQuality < 0.00001f ) fQuality = 0.00001f;
	if ( fQuality > 10000.0f ) fQuality = 10000.0f;
	fQuality = 1 / fQuality;

	if ( iTexSize < 32 ) iTexSize = 32;
	if ( pLMTextureList ) LMClearTextures( );

	// lee - 250914 - stored globally as only one size passed in per LM session
	g_iTexSize = iTexSize;

	// U75 - 060510 - new LM heap
	//pLMTextureList = new LMTexture( iTexSize, iTexSize );
	pLMTextureList = (LMTexture*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, sizeof(LMTexture));
	pLMTextureList->New( iTexSize, iTexSize );
	pLMTextureList->pNextLMTex = 0;

	//fit light map objects to textures
	g_iBuildLightMapsCycleMode = 0;
	g_pBuildLightMapsCycleLMObject = pLMObjectList;
	g_iBuildLightMapsCycleCurrentObject = 0;
	g_fBuildLightMapsCycleQuality = fQuality;
	g_iBuildLightMapsCycleBlur = iBlur;
	g_bBuildLightMapsCycleSucceed = true;
	g_iBuildLightMapsCycleTexSize = iTexSize;
}

DLLEXPORT int LMBuildLightMapsCycle( void )
{
	int iCompleteFlag = 0;
	switch ( g_iBuildLightMapsCycleMode )
	{
		case 0 : // Mapping Object

			if ( g_pBuildLightMapsCycleLMObject )
			{
				g_iBuildLightMapsCycleCurrentObject++;

				// can cancel this loop manually by detecting the ESCAPE key
				short sExitEarlyKey = GetAsyncKeyState(VK_ESCAPE);
				if ( sExitEarlyKey!=0 )
				{
					// exit lightmapping early
					bLightmapInProgress = false;
					g_iBuildLightMapsCycleMode = 99;
					return 0;
				}

				if ( g_pShared ) 
				{
					char pInfoStr[256];
					sprintf_s( pInfoStr, 255, "Mapping Object [%d/%d]", g_iBuildLightMapsCycleCurrentObject, iTotalLMObjects );
					g_pShared->SetStatus( pInfoStr, (48.0f*g_iBuildLightMapsCycleCurrentObject)/iTotalLMObjects );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						g_iBuildLightMapsCycleMode = 99;
						return 0;
					}
				}

				g_pBuildLightMapsCycleLMObject->GroupPolys( g_fBuildLightMapsCycleQuality );
				g_pBuildLightMapsCycleLMObject->SortGroups( );

				if ( g_pBuildLightMapsCycleLMObject->GetNumPolys( ) == 0 ) 
				{
					g_pBuildLightMapsCycleLMObject = g_pBuildLightMapsCycleLMObject->pNextObject;
					break;
				}

				float fNewQuality = g_fBuildLightMapsCycleQuality;
				while ( !pLMTextureList->AddLMObject( g_pBuildLightMapsCycleLMObject ) && fNewQuality < 1000 )
				{
					g_bBuildLightMapsCycleSucceed = false;

					fNewQuality *= 1.5;
					g_pBuildLightMapsCycleLMObject->ReScaleGroups( fNewQuality );

					while ( !g_pBuildLightMapsCycleLMObject->WillFit( g_iBuildLightMapsCycleTexSize, g_iBuildLightMapsCycleTexSize ) && fNewQuality < 1000 )
					{
						fNewQuality *= 1.5;
						g_pBuildLightMapsCycleLMObject->ReScaleGroups( fNewQuality );
					}
				}

				g_pBuildLightMapsCycleLMObject = g_pBuildLightMapsCycleLMObject->pNextObject;
			}
			else
			{
				g_iBuildLightMapsCycleMode = 1;
			}
			break;
			
		case 1 : // Quality Change

			if ( !g_bBuildLightMapsCycleSucceed )
			{
				/* g.mysystem.levelBankTestMap_s could have done it
				char fileStr[256];
				FILE *pQualityChangeFile;
				GG_fopen_s( &pQualityChangeFile, "levelbank\\testmap\\lightmaps\\Quality Change.txt","w" );
				fputs( "Listed below are object limbs that were too large to fit onto a texture and had their quality reduced\n", pQualityChangeFile);
				fputs( "To allow the original quality to be used try splitting your object up into limbs and splitting large polygons into several smaller ones\n\n", pQualityChangeFile);

				g_pBuildLightMapsCycleLMObject = pLMObjectList;
				while ( g_pBuildLightMapsCycleLMObject )
				{
					if ( fabs( g_pBuildLightMapsCycleLMObject->GetQuality( ) - g_fBuildLightMapsCycleQuality ) > 0.0001 && g_pBuildLightMapsCycleLMObject->GetNumPolys( ) > 0 )
					{
						if ( g_pBuildLightMapsCycleLMObject->pLMTexture )
						{
							sprintf_s( fileStr, 255, "Object Number: %d, Limb Number: %d, New Quality: %f\n", g_pBuildLightMapsCycleLMObject->iID, g_pBuildLightMapsCycleLMObject->iLimbID, 1.0f/g_pBuildLightMapsCycleLMObject->GetQuality( ) );
						}
						else
						{
							sprintf_s( fileStr, 255, "Object Number: %d, Limb Number: %d, FAILED\n", g_pBuildLightMapsCycleLMObject->iID, g_pBuildLightMapsCycleLMObject->iLimbID );
						}

						fputs( fileStr, pQualityChangeFile );
					}

					g_pBuildLightMapsCycleLMObject = g_pBuildLightMapsCycleLMObject->pNextObject;
				}

				fclose( pQualityChangeFile );
				*/
			}
			g_iBuildLightMapsCycleMode = 2;
			g_iBuildLightMapsCycleCurrentObject = 0;
			g_pBuildLightMapsCycleLMObject = pLMObjectList;
			break;

		case 2 : // Calculating Object Light

			if ( g_pBuildLightMapsCycleLMObject )
			{
				g_iBuildLightMapsCycleCurrentObject++;

				// can cancel this loop manually by detecting the ESCAPE key
				short sExitEarlyKey = GetAsyncKeyState(VK_ESCAPE);
				if ( sExitEarlyKey!=0 )
				{
					// exit lightmapping early
					bLightmapInProgress = false;
					g_iBuildLightMapsCycleMode = 99;
					return 0;
				}

				if ( g_pShared ) 
				{
					char pInfoStr[256];
					sprintf_s( pInfoStr, 255, "Calculating Object Light [%d/%d]", g_iBuildLightMapsCycleCurrentObject, iTotalLMObjects );
					g_pShared->SetStatus( pInfoStr, (48.0f*g_iBuildLightMapsCycleCurrentObject)/iTotalLMObjects + 48 );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						g_iBuildLightMapsCycleMode = 99;
						return 0;
					}
				}

				if ( g_pBuildLightMapsCycleLMObject->pLMTexture )
				{
					g_pBuildLightMapsCycleLMObject->CalculateLight( pLightList, &cColTree, g_iBuildLightMapsCycleBlur, g_pBuildLightMapsCycleLMObject->bIgnoreNormals, 0 );
					g_pBuildLightMapsCycleLMObject->ApplyToTexture( );
				}

				g_pBuildLightMapsCycleLMObject = g_pBuildLightMapsCycleLMObject->pNextObject;
			}
			else
			{
				// carried on below (complete light maps)
				if ( g_pShared ) g_pShared->SetComplete( true );
				if ( g_pShared ) g_pShared->SetStatus( "Saving lightmaps", 100.0f );
				g_iBuildLightMapsCycleMode = 99;
			}
			break;

		case 99 : 
			iCompleteFlag = 1;
			break;
	}

	// return when whole cycle complete
	return iCompleteFlag;
}

DLLEXPORT void LMCompleteLightMaps( )
{
	// Must be called from primary thread to avoid multithread issues with Direct X
	// only when all thread activity ceased
	if ( g_pShared==NULL || (g_pShared && g_pShared->GetComplete()==true ) )
	{
		// fill the textures with light
		char pInfoStr[256];
		int iTexSize = g_iTexSize;
		if ( pLMThread ) iTexSize = pLMThread->iTexSize;
		LMTexture *pLMTexture = pLMTextureList;
		char filename[256];

		char szRoot [ 256 ];
		GetCurrentDirectory( 256, szRoot );

		char szFolder [ 64 ];

		// ensure lightmap folder is relative to current root folder
		if ( strnicmp ( szRoot, szLightmapFolderName, strlen(szRoot) )==NULL )
		{
			strcpy ( szLightmapFolderName, szLightmapFolderName+strlen(szRoot)+1 );
		}

		char* szLast = szLightmapFolderName;
		char* szSlash = strchr( szLightmapFolderName, '\\' );

		while ( szSlash )
		{
			strncpy_s( szFolder, 64, szLast, (szSlash - szLast) );
			if ( strlen ( szFolder ) > 0 )
			{
				if ( !SetCurrentDirectory( szFolder ) )
				{
					CreateDirectory( szFolder, NULL );
					if ( !SetCurrentDirectory( szFolder ) ) 
					{
						MessageBox( NULL, "Invalid lightmap folder location, could not create directory", "Lightmapper Error", 0 );
						return;
					}
				}
			}
			szLast = szSlash+1;
			szSlash = strchr( szLast, '\\' );
		}

		SetCurrentDirectory( szRoot );

		pLMTexture = pLMTextureList;

		char szExtension [ 5 ];
		switch( g_iLightmapFileFormat )
		{
			case 0: strcpy_s( szExtension, 5, "png" ); break;
			case 1: strcpy_s( szExtension, 5, "dds" ); break;
			case 2: strcpy_s( szExtension, 5, "bmp" ); break;
			default: strcpy_s( szExtension, 5, "png" ); break;
		}

		while ( pLMTexture )
		{
			if ( !pLMTexture->IsEmpty( ) )
			{
				if ( g_pShared ) 
				{
					sprintf_s( pInfoStr, 255, "Saving Textures [%d]", g_iLightmapFileNumber );
					g_pShared->SetStatus( pInfoStr, 99 );
					if ( g_pShared->GetTerminate( ) ) 
					{
						bLightmapInProgress = false;
						g_pShared->SetComplete( true );
						return;
					}
				}

				//calculate Lumel values
				pLMTexture->CopyToTexture( );
				sprintf_s( filename, 255, "%s%s%d.%s", szLightmapFolderName, szLightmapName, g_iLightmapFileNumber, szExtension );
				g_iLightmapFileNumber++;
				pLMTexture->SaveTexture( filename );

				// U75 - FPGC - 121209 - surely we can now remove the DDS from managed memory if the file has been created
				// otherwise we build up potentially hundreds of DDS textures in managed memory!!
				// 190714 - but not if we want to SEE this light map!!!
				//SAFE_RELEASE ( pLMTexture->pTextureDDS );
			}

			pLMTexture = pLMTexture->pNextLMTex;
		}

		// apply the textures to the dbpro objects
		LMObject *pLMObject = pLMObjectList;
		while ( pLMObject )
		{
			pLMObject->CalculateVertexUV( iTexSize, iTexSize );
			pLMObject->UpdateObject( iBlendMode );
			pLMObject = pLMObject->pNextObject;
		}

		// now convert to tri-only to store unique UV refs for each vertex and 
		// apply the UV data stored in poly list to original LM objects
		pLMObject = pLMObjectList;
		while ( pLMObject )
		{
			pLMObject->CreateTriOnlyAndApplyUVData();
			pLMObject = pLMObject->pNextObject;
		}

		// finished lightmapping for this process
		bLightmapInProgress = false;
	}
}

DLLEXPORT void LMTerminateThread( )
{
	CheckLMInit( );

	if ( !g_pShared )
	{
		//MessageBox( NULL, "Cannot Terminate, Thread Not In Progress", "Light Mapping Warning", 0 );
		return;
	}

	g_pShared->SetTerminate( true );
}

DLLEXPORT void LMUpdateObjects( )
{
	CheckLMInit( );

	LMTexture *pLMTexture = pLMTextureList;
	LMObject *pLMObject = pLMObjectList;
	while ( pLMObject )
	{
		pLMObject->UpdateObject( iBlendMode );
		pLMObject = pLMObject->pNextObject;
	}
}

DLLEXPORT void LMReset( )
{
	CheckLMInit( );

	// can reset LM even if bLightmapInProgress true
	//if ( CheckInProgress( ) ) return;

	LMClearCollisionObjects( );
	LMClearLightMapObjects( );
	LMClearLights( );
	LMClearTextures( );

	if ( pLMThread ) delete pLMThread;
	pLMThread = 0;

	LMSetAmbientLight( 0,0,0 );
	LMSetAmbientOcclusionOff( );
	LMSetBlendMode( 4 );
	LMSetMode( 0 );

	// free textures accumilated for transparent collision data
	TreeFaceFreeUsages();

	// reset key flag for next ML session
	bLightmapInProgress = false;

	// U75 - 060510 - Use special heap during lightmapping, to avoid virtual address fragmentation
	// beyond the scope of the lightmapping process (self-contained except for the UV and Textures)
	if ( g_hLMHeap ) HeapDestroy ( g_hLMHeap );
	g_hLMHeap = NULL;
}

DLLEXPORT LPSTR LMGetStatus( void )
{
	CheckLMInit( );

#pragma warning ( disable : 4312 )	//convert DWORD to 'char *'

	//g_pGlob->CreateDeleteString ( (DWORD*) &szReturnString, 0 );
	
	char szNewString[256];
	bool bOK = g_pShared && g_pShared->GetStatus( szNewString );
	if ( !bOK ) sprintf_s( szNewString, 255, "Could Not Get Status" );
	
	DWORD dwSize = (DWORD) strlen ( (char*) szNewString );

	LPSTR szReturnString = NULL;
	g_pGlob->CreateDeleteString ( &szReturnString, dwSize+1 );
	
	strcpy_s ( (char*) szReturnString, dwSize+1, szNewString);

#pragma warning ( default : 4312 )
	
	return szReturnString;
}

DLLEXPORT float LMGetPercent( )
{
	CheckLMInit( );

	float fValue = -1;
	if ( g_pShared ) 
	{
		fValue = g_pShared->GetPercent( );
	}

	return fValue;
}

DLLEXPORT int LMGetComplete( )
{
	CheckLMInit( );

	if ( g_pShared ) 
	{
		return g_pShared->GetComplete( ) ? 1 : 0;
	}

	return true;
}

/*
DLLEXPORT int LMIntersects( float x, float y, float z, float x2, float y2, float z2 )
{
	CheckLMInit( );

	Point pntStart( x,y,z );
	Vector vecDir( x2-x, y2-y, z2-z );
	Vector vecDirI( 1.0f/(x2-x), 1.0f/(y2-y), 1.0f/(z2-z) );

	return cColTree.intersects( &pntStart, &vecDir, &vecDirI, 0, 0 ) ? 1 : 0;
}
*/

/*
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
*/

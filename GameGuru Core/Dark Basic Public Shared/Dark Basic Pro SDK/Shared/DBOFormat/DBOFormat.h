//
// DBOFormat Functions Header
//

#ifndef _DBOFORMAT_H_
#define _DBOFORMAT_H_

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include "directx-macros.h"
#include "global.h"

// Define
#define NEVERSTOREORIGINALVERTICES

#include "DBOData.h"
#include "DBOBlock.h"
#include "DBOExternals.h"
#include <vector>

#define NUM_CULLPLANES			6

extern		DWORD								g_dwEffectErrorMsgSize;
extern		LPSTR								g_pEffectErrorMsg;
extern		DWORD*								g_pConversionMap;
extern		GGPLANE								g_Planes [ 20 ] [ NUM_CULLPLANES ];
extern		GGVECTOR3							g_PlaneVector [ 20 ] [ NUM_CULLPLANES ];

// structure to hold items operated on by the thread
struct sPreLoadedObjectData
{
	char pFilename[1024];
	DWORD dwDataSize;
	DWORD* pData;
};
extern std::vector<sPreLoadedObjectData> g_object_outputv;

DARKSDK void		DBOCalculateLoaderTempFolder		( void );
DARKSDK bool		LoadDBODataBlock					( LPSTR pFilename, DWORD* pdwBlockSize, void** ppDBOBlock );
DARKSDK bool		LoadDBO								( LPSTR pFilename, sObject** ppObject, char* pOrgFilename = NULL );
DARKSDK bool		SaveDBO								( LPSTR pFilename, sObject* pObject );
DARKSDK bool		CloneDBO							( sObject** ppDestObject, sObject* pSrcObject );

DARKSDK bool		GetMeshCount						( sFrame* pFrame, int* piCount );
DARKSDK bool		BuildMeshList						( sMesh** pMeshList, sFrame* pFrame, int* iStart );
DARKSDK bool		GetFrameCount						( sFrame* pFrame, int* piCount );
DARKSDK bool		BuildFrameList						( sFrame** pFrameList, sFrame* pFrame, int* iStart );

DARKSDK sFrame*		FindFrame							( char* szName, sFrame* pFrame );
DARKSDK sAnimation* FindAnimation						( sObject* pObject, sFrame* pOriginalFrame );
DARKSDK sAnimation* CopyAnimation						( sAnimation* pCurrentAnim, LPSTR szNewName );
DARKSDK bool		InitFramesToBones					( sMesh** pMeshList, int iMeshCount );
DARKSDK bool		MapFramesToAnimations				( sObject* pObject, bool bCostlySort );
DARKSDK void	    InitOneMeshFramesToBones			( sMesh* pMesh );
DARKSDK bool		MapFramesToBones					( sMesh** pMesh, sFrame *pFrame, int iCount );
DARKSDK bool	    MapOneMeshFramesToBones				( sMesh* pMain, sFrame* pFrame );
DARKSDK void		UpdateObjectCamDistance				( sObject* pObject );
DARKSDK void		UpdateObjectAnimation				( sObject* pObject );

DARKSDK bool		CreateFrameAndMeshList				( sObject* pObject );
DARKSDK bool		SetupObjectsGenericProperties		( sObject* pObject );
DARKSDK bool		SetDefaultRenderSettings			( sObject* pObject );
DARKSDK bool		CalculateMeshBounds					( sMesh* pMesh );
DARKSDK bool		CalculateAllBounds					( sObject* pObject, bool bUpdateFramesOnly );
DARKSDK bool		CreateMesh							( sObject** ppObject, LPSTR pName );
DARKSDK bool		DeleteMesh							( sObject** ppObject );

DARKSDK void		UpdateEulerRotation					( sObject* pObject );
DARKSDK void		UpdateObjectRotation				( sObject* pObject );
DARKSDK bool		CalcObjectWorld						( sObject* pObject );
DARKSDK bool		CalculateObjectWorld				( sObject* pObject, sFrame* pGluedToFramePtr );
DARKSDK void		CalculateAbsoluteWorldMatrix		( sObject* pObject, sFrame* pFrame, sMesh* pMesh );
DARKSDK bool		CreateSingleMeshFromObject			( sMesh** ppMesh, sObject* pObject, int iIgnoreMode );
DARKSDK bool		CreateSingleMeshFromLimb			( sMesh** ppMesh, sObject* pObject, int iLimbNumber, int iIgnoreMode );

DARKSDK bool		CopyMeshDataToIndexBuffer			( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyMeshDataToDWORDIndexBuffer		( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyDWORDMeshDataToDWORDIndexBuffer	( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyMeshDataToVertexBufferSameFVF	( sMesh* pMesh, IGGVertexBuffer* pVertexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyIndexBufferToMeshData			( sMesh* pMesh, IGGIndexBuffer* pIndexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyVertexBufferToMeshDataSameFVF	( sMesh* pMesh, IGGVertexBuffer* pVertexBufferRef, DWORD dwBufferOffset );
DARKSDK bool		CopyIndexMeshData					( sMesh* pDstMesh, sMesh* pSrcMesh, DWORD dwOffset, DWORD dwIndexCount );
DARKSDK bool		IncrementIndexMeshData				( sMesh* pDstMesh, DWORD dwOffset, DWORD dwIndexCount, DWORD dwIncrement );
DARKSDK bool		CopyVertexMeshDataSameFVF			( sMesh* pDstMesh, sMesh* pSrcMesh, DWORD dwOffset, DWORD dwVertexCount );
DARKSDK void		SplitMeshSide						( int iSide, sMesh* pMesh, sMesh* pSplitMesh );

DARKSDK LPGGMESH	LocalMeshToDXMesh					( sMesh* pMesh, CONST LPGGVERTEXELEMENT dwDeclaration, DWORD dwFVFOverride );
DARKSDK void		UpdateLocalMeshWithDXMesh			( sMesh* pMesh, LPGGMESH pDXMesh );
DARKSDK void		RestoreLocalMesh					( sMesh* pMesh );
DARKSDK void		ConvertLocalMeshToFVF				( sMesh* pMesh, DWORD dwFVF );
DARKSDK void		ConvertLocalMeshToVertsOnly			( sMesh* pMesh, bool bIs32BitIndexData );
DARKSDK bool		ConvertLocalMeshToTriList			( sMesh* pMesh );
DARKSDK void		ConvertToSharedVerts				( sMesh* pMesh, float fEpsilon );
DARKSDK bool		MakeLocalMeshFromOtherLocalMesh		( sMesh* pMesh, sMesh* pOtherMesh, DWORD dwIndexCount, DWORD dwVertexCount );
DARKSDK bool		MakeLocalMeshFromOtherLocalMesh		( sMesh* pMesh, sMesh* pOtherMesh );
DARKSDK bool		MakeLocalMeshFromPureMeshData		( sMesh* pMesh, DWORD dwFVF, DWORD dwFVFSize, float* pMeshData, DWORD dwVertMax, DWORD dwPrimType );
DARKSDK void		FlipNormals							( sMesh* pMesh, int iFlipMode );
DARKSDK void		GenerateNewNormalsForMesh			( sMesh* pMesh, int iMode );
DARKSDK void		GenerateNormalsForMesh				( sMesh* pMesh, int iMode );
DARKSDK void		GenerateExtraDataForMeshEx			( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bBinormals, BOOL bDiffuse, BOOL bBones, DWORD dwGenerateMode );
DARKSDK void		GenerateExtraDataForMeshEx			( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bBinormals, BOOL bDiffuse, BOOL bBones );
DARKSDK void		GenerateExtraDataForMesh			( sMesh* pMesh, BOOL bNormals, BOOL bTangents, BOOL bDiffuse, BOOL bBones );
DARKSDK void		CopyReferencesToShaderEffects		( sMesh* pNewMesh, sMesh* pMesh );
DARKSDK void		CloneShaderEffects					( sMesh* pNewMesh, sMesh* pMesh );
DARKSDK bool		CheckIfNeedExtraBonesPerVertices	( sMesh* pMesh );

DARKSDK bool		SetupShortVertex					( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float tu, float tv );
DARKSDK bool		SetupStandardVertex					( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv );
DARKSDK bool		SetupStandardVertexDec				( sMesh* pMesh, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv );
DARKSDK bool		GetFVFValueOffsetMap				( DWORD dwFVF, sOffsetMap* psOffsetMap );
DARKSDK bool		GetFVFOffsetMapFixedForBones		( sMesh* pMesh, sOffsetMap* psOffsetMap );
DARKSDK bool		GetFVFOffsetMap						( sMesh* pMesh, sOffsetMap* psOffsetMap );
DARKSDK bool		SetupMeshDeclarationData			( sMesh* pMesh, CONST GGVERTEXELEMENT* pDeclaration, DWORD dwVertexSize, DWORD dwVertexCount, DWORD dwIndexCount );
DARKSDK bool		SetupMeshFVFData					( sMesh* pMesh, DWORD dwFVF, DWORD dwSize, DWORD dwIndexCount, bool bTempAllow32BitIndexBuffer );
DARKSDK bool		SetupMeshData						( sMesh* pMesh, DWORD dwVertexCount, DWORD dwIndexCount, bool bTempAllow32BitIndexBuffer );

DARKSDK bool		CreateVertexShaderFromFVF			( DWORD dwFVF, DWORD* pdwShader );

DARKSDK bool		SetupFrustum						( float fZDistance );
DARKSDK bool		SetupPortalFrustum					( DWORD dwFrustumCount, GGVECTOR3* pvecCam, GGVECTOR3* pvecA, GGVECTOR3* pvecB, GGVECTOR3* pvecC, GGVECTOR3* pvecD, bool bFrustrumZeroIsValid );
DARKSDK bool		SetupCastFrustum					( DWORD dwFrustumCount, GGVECTOR3* pvecStart, GGVECTOR3* pvecFinish );
DARKSDK bool		CheckPoint							( float fX, float fY, float fZ );
DARKSDK bool		CheckPoint							( GGPLANE* pPlanes, GGVECTOR3* pvecPoint );
DARKSDK bool		CheckCube							( float fX, float fY, float fZ, float fSize );
DARKSDK bool		CheckRectangleEx					( DWORD iFrustumIndex, float fX, float fY, float fZ, float fXSize, float fYSize, float fZSize );
DARKSDK bool		CheckRectangle						( float fX, float fY, float fZ, float fXSize, float fYSize, float fZSize );
DARKSDK bool		CheckSphere							( float fX, float fY, float fZ, float fRadius );
DARKSDK bool		CheckSphere							( DWORD dwPlaneIndex, float fX, float fY, float fZ, float fRadius );
DARKSDK bool		CheckPolygon						( GGPLANE* pPlanes, GGVECTOR3* pvec0, GGVECTOR3* pvec1, GGVECTOR3* pvec2 );
DARKSDK bool		QuickSortArray						( int* array, int low, int high );
DARKSDK bool		SwapInts							( int* array, int n1,  int n2   );

// 310305 - mike - new function for allocating custom memory
DARKSDK void		CreateCustomDataArrayForObject		( sObject* pObject, int iSize );

DARKSDK void		object_preload_files_start			( void );
DARKSDK void		object_preload_files_add			( LPSTR pFilename );
DARKSDK void		object_preload_files_finish			( void );
DARKSDK void		object_preload_files_strictwaittoend( void );
DARKSDK void		object_preload_files_wait			( void );
DARKSDK void		object_preload_files_reset			( void );
DARKSDK bool		object_preload_files_in_progress	( void );

#endif _DBOFORMAT_H_

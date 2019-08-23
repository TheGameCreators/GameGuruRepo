//
// CObjectManager Functions Implementation
//
#include "..\..\GameGuru\Include\gameguru.h"

#include "CommonC.h"
#include "SoftwareCulling.h"
#include "ShadowMapping\cShadowMaps.h"
#include "Occlusion\cOcclusion.h"
#include "CObjectsC.h"
#include "CGfxC.h"
#include <algorithm>


// extern/protos
bool update_mesh_light(sMesh* pMesh, sObject* pObject, sFrame* pFrame);
void start_mesh_light(void);
void end_mesh_light(void);
void setlayer_mesh_light(int layer);

GGFORMAT GetValidStencilBufferFormat ( GGFORMAT Format );
extern UINT	g_StereoEyeToggle;
extern DWORD g_dwSyncMaskOverride;

// weapon shader effect indexes
extern int g_weaponbasicshadereffectindex;
extern int g_weaponboneshadereffectindex;
extern int g_jetpackboneshadereffectindex;

// shadow mapping
extern CascadedShadowsManager g_CascadedShadow;
float g_fShrinkObjectsTo = 0.0f;

// flag to force texture update
bool g_ForceTextureListUpdate = false;

// flag to completely skip the DOF/MotionBlur depth render pass
bool g_bSkipAnyDedicatedDepthRendering = false;

// Globals for new main camea depth texture
bool g_bMainCameraDepthCaptureActive = true;
bool g_bFirstRenderClearsRenderTarget = false;
LPGGTEXTURE g_pMainCameraDepthTexture = NULL;
LPGGRENDERTARGETVIEW g_pMainCameraDepthTextureSurfaceRef = NULL;
LPGGSURFACE g_pMainCameraDepthStencilTexture = NULL;
LPGGDEPTHSTENCILVIEW g_pMainCameraDepthStencilTextureView = NULL;
GGHANDLE g_pMainCameraDepthHandle = NULL;
LPGGEFFECT g_pMainCameraDepthEffect = NULL;

// Globals for DBO/Manager relationship
std::vector< sMesh* >						g_vRefreshMeshList;
std::vector< sObject* >						g_vAnimatableObjectList;
int											g_iSortedObjectCount;
sObject**									g_ppSortedObjectList;

// can prepare scene in two places now, so need flag to keeps tabs on it
bool										g_bScenePrepared				= false;
bool										g_bRenderVeryEarlyObjects		= false;

// during LOD QUAD transition, use ZBIAS to move quad out of way when 3D fades in/out by THIS amount in total
float										g_fZBiasEpsilon					= 0.0005f;

// detect AnisotropyLevel
int											g_iAnisotropyLevel				= -1;

// Retain the last objects distance and use to decide if shader should toggle to LOD variant
float										g_fObjectCamDistance			= 0.0f;

#ifdef DX11
struct CBPerMesh
{
	KMaths::Matrix mWorld;
	KMaths::Matrix mView;
	KMaths::Matrix mProjection;
};				
struct CBPerMeshPS
{
	GGCOLOR vMaterialEmissive;
	float fAlphaOverride;
	float fRes1;
	float fRes2;
	float fRes3;
	KMaths::Matrix mViewInv;
	KMaths::Matrix mViewProj;
	KMaths::Matrix mPrevViewProj;
};				
ID3D11Buffer* g_pCBPerMesh		= NULL;
ID3D11Buffer* g_pCBPerMeshPS	= NULL;
#endif

// externals
extern GGMATRIX 		g_matThisViewProj;
extern GGMATRIX 		g_matThisCameraView;
extern GGMATRIX 		g_matPreviousViewProj;
extern LPGG			m_pDX;

// Occlusion object external
extern COcclusion			g_Occlusion;

namespace
{
    // Structures for sorting using the STL sort
    // Used for sorting the m_ppSortedObjectList, m_pDepthSortedList
    struct OrderByReverseCameraDistance
    {
        bool operator()(sObject* pObjectA, sObject* pObjectB)
        {
            if (pObjectA->position.fCamDistance > pObjectB->position.fCamDistance)
                return true;
            if (pObjectA->position.fCamDistance == pObjectB->position.fCamDistance)
                return (pObjectA->dwObjectNumber < pObjectB->dwObjectNumber);
            return false;
        }
    };
    struct OrderByTexture
    {
        bool operator()(sObject* pObjectA, sObject* pObjectB)
        {
            int iImageA = 0;
            int iImageB = 0;

            // Calculate order by object number now,
            // just in case it's needed and to avoid getting incorrect results
            // if instances are involved.
            bool bObjectOrder = pObjectA->dwObjectNumber < pObjectB->dwObjectNumber;

            // Get the image id for object a
		    if ( pObjectA->pInstanceOfObject )
			    pObjectA = pObjectA->pInstanceOfObject;
		    if ( pObjectA->ppMeshList && pObjectA->ppMeshList [ 0 ]->pTextures)
                iImageA = pObjectA->ppMeshList [ 0 ]->pTextures [ 0 ].iImageID;

            // Get the image id for object b
		    if ( pObjectB->pInstanceOfObject )
			    pObjectB = pObjectB->pInstanceOfObject;
		    if ( pObjectB->ppMeshList && pObjectB->ppMeshList [ 0 ]->pTextures)
                iImageB = pObjectB->ppMeshList [ 0 ]->pTextures [ 0 ].iImageID;

            if (iImageA < iImageB)
                return true;
            if (iImageA > iImageB)
                return false;

            // Same images, so order by object id
            return bObjectOrder;
        }
    };
    struct OrderByObject
    {
        bool operator()(sObject* pObjectA, sObject* pObjectB)
        {
            return (pObjectA->dwObjectNumber < pObjectB->dwObjectNumber);
        }
    };
}

// move these into cpp file for debugging
CObjectManager::sVertexData::sVertexData ( )
{
	memset ( this, 0, sizeof(sVertexData) );
}

CObjectManager::sVertexData::~sVertexData ( )
{
	SAFE_RELEASE ( pVB );
	sVertexData* pThis = pNext;
	while ( pThis )
	{
		sVertexData* pNextOne = pThis->pNext;
		pThis->pNext = NULL;
		SAFE_RELEASE ( pThis->pVB );
		delete pThis;
		pThis = pNextOne;
	}
	pNext = NULL;
}

CObjectManager::sIndexData::sIndexData ( )
{
	memset ( this, 0, sizeof(sIndexData) );
}

CObjectManager::sIndexData::~sIndexData ( )
{
	SAFE_RELEASE ( pIB );
	sIndexData* pThis = pNext;
	while ( pThis )
	{
		sIndexData* pNextOne = pThis->pNext;
		pThis->pNext = NULL;
		SAFE_RELEASE ( pThis->pIB );
		delete pThis;
		pThis = pNextOne;
	}
	pNext = NULL;
}

bool CObjectManager::UpdateObjectListSize ( int iSize )
{
	// if list count is larger than size passed in, we can ignore a resize
	if ( iSize < m_iListCount )
		return true;

	// allocate memory
	sObject**	ppSortedObjectVisibleList = new sObject* [ iSize ];
	sObject**	ppSortedObjectList        = new sObject* [ iSize ];
	bool*		pbMarkedList              = new bool     [ iSize ];

	// safety checks on new memory
	SAFE_MEMORY ( ppSortedObjectVisibleList );
	SAFE_MEMORY ( ppSortedObjectList );
	SAFE_MEMORY ( pbMarkedList );

	// set all pointers to null
	for ( int iArrayIndex = 0; iArrayIndex < iSize; iArrayIndex++ )
	{
		ppSortedObjectVisibleList [ iArrayIndex ] = NULL;
		ppSortedObjectList        [ iArrayIndex ] = NULL;
		pbMarkedList              [ iArrayIndex ] = false;
	}

	// copy old data to new arrays
	int iSizeToCopyNow = m_iListCount;
	if ( iSizeToCopyNow > 0 )
	{
		if ( m_ppSortedObjectVisibleList ) memcpy ( ppSortedObjectVisibleList, m_ppSortedObjectVisibleList, sizeof(sObject*) * iSizeToCopyNow );
		if ( g_ppSortedObjectList ) memcpy ( ppSortedObjectList, g_ppSortedObjectList, sizeof(sObject*) * iSizeToCopyNow );
		if ( m_pbMarkedList ) memcpy ( pbMarkedList, m_pbMarkedList, sizeof(bool) * iSizeToCopyNow );
	}

	// safely delete any of the arrays
	SAFE_DELETE_ARRAY ( m_ppSortedObjectVisibleList );
	SAFE_DELETE_ARRAY ( g_ppSortedObjectList );
	SAFE_DELETE_ARRAY ( m_pbMarkedList );

	// allocate memory
	m_ppSortedObjectVisibleList = ppSortedObjectVisibleList;
	g_ppSortedObjectList        = ppSortedObjectList;
	m_pbMarkedList              = pbMarkedList;

	// store the size of the list
	m_iListCount = iSize;

	// return back
	return true;
}

bool CObjectManager::Setup ( void )
{
	// clear manager members

	// Render State Global Defaults
    memset( &m_RenderStates, 0, sizeof( m_RenderStates ) );
	m_RenderStates.dwGlobalCullDirection = GGCULL_CCW;

	// set all pointers to null
	m_ppCurrentVBRef	= NULL;
	m_ppLastVBRef		= NULL;
	m_ppCurrentIBRef	= NULL;
	m_ppLastIBRef		= NULL;

	m_dwCurrentShader = 0;
	m_dwCurrentFVF = 0;
	m_dwLastShader = 0;
	m_dwLastFVF = 0;

	m_iCurrentTexture = 0;
	m_iLastTexture = 0;
	m_dwLastTextureCount = 0;
	m_bUpdateTextureList = false;
	m_bUpdateVertexDecs = 0;
	m_bUpdateStreams = 0;
	g_iSortedObjectCount = 0;
	m_iLastCount = 0;
	m_iListCount = 0;
	m_iVisibleObjectCount = 0;
	m_pbMarkedList = 0;
	g_ppSortedObjectList = 0;
	m_ppSortedObjectVisibleList = 0;
	m_pVertexDataList = 0;
	m_pIndexDataList = 0;

	// Reset member vars
	m_bGlobalShadows				= false;
    g_bObjectReplacedUpdateBuffers	= false;
    m_pCamera						= 0;

	// all okay
	return true;
}

bool CObjectManager::Free ( void )
{
	// ensure this is deleted before leave
	SAFE_RELEASE ( g_pMainCameraDepthStencilTextureView );
	SAFE_RELEASE ( g_pMainCameraDepthStencilTexture );
	SAFE_RELEASE ( g_pMainCameraDepthTextureSurfaceRef );
	SAFE_RELEASE ( g_pMainCameraDepthTexture );

	// safely delete any arrays and objects
	SAFE_DELETE ( m_pVertexDataList );
	SAFE_DELETE ( m_pIndexDataList );
	SAFE_DELETE_ARRAY ( m_pbMarkedList );
	SAFE_DELETE_ARRAY ( g_ppSortedObjectList );
	SAFE_DELETE_ARRAY ( m_ppSortedObjectVisibleList );

	// all okay
	return true;
}

//
// VERTEX AND INDEX BUFFERS
//

CObjectManager::sIndexData* CObjectManager::FindIndexBuffer ( DWORD dwIndexCount, bool bUsesItsOwnBuffers )
{
	// find an index buffer which we wan use

	// check D3D device is valid
	if ( !m_pD3D )
		return NULL;

	// make sure the parameter is valid
	if ( dwIndexCount < 1 )
		return NULL;

	// local variables
	bool		 bMatch = false;
	sIndexData*  pIndexData  = m_pIndexDataList;

	// create a decent start size of the IB
	DWORD dwIndexBufferSize = 0;
	#ifdef DX11
	dwIndexBufferSize = 65534;
	#else
	GGCAPS caps;
	m_pD3D->GetDeviceCaps ( &caps );
	dwIndexBufferSize = caps.MaxVertexIndex;
	#endif

	// no search if need to use its own buffer
	if ( bUsesItsOwnBuffers )
		pIndexData=NULL;

	// run through all nodes in list
	while ( pIndexData )
	{
		// see if we can fit the data into the buffer
		if ( pIndexData->dwCurrentIndexCount + dwIndexCount < pIndexData->dwMaxIndexCount )
		{
			bMatch = true;
			break;
		}

		// move to next node
		pIndexData = pIndexData->pNext;
	}

	// if we don't have a match then create a new item
	if ( !bMatch )
	{
		if ( !m_pIndexDataList )
		{
			// create new list
			m_pIndexDataList	= new sIndexData;
			pIndexData			= m_pIndexDataList;

			// ensure creation okay
			SAFE_MEMORY ( m_pIndexDataList );
		}
		else
		{
			// find end of list
			pIndexData = m_pIndexDataList;
			while ( pIndexData )
			{
				if ( pIndexData->pNext )
				{
					pIndexData = pIndexData->pNext;
					continue;
				}
				else
					break;
			}

			// add new item to list
			pIndexData->pNext = new sIndexData ( );
			pIndexData        = pIndexData->pNext;
		}

		// own buffer needs only to be the size of the data
		if ( bUsesItsOwnBuffers )
			dwIndexBufferSize = dwIndexCount;

		// loop until succeed in creating a IB
		bool bCreate = true;
		while ( bCreate )
		{
			// attempt to create a IB
			#ifdef DX11

			/*
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage           = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth       = sizeof ( WORD ) * dwIndexBufferSize;
			bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags       = 0;
			*/
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth       = sizeof ( WORD ) * dwIndexBufferSize;
			bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags  = 0;
			bufferDesc.MiscFlags       = 0;

			if ( FAILED ( m_pD3D->CreateBuffer( &bufferDesc, NULL, &pIndexData->pIB ) ) )
			{
				// failed, try half the size
				dwIndexBufferSize /= 2;
			}
			else
			{
				// only if IB can hold required vertex data
				if ( dwIndexBufferSize >= dwIndexCount )
				{
					// success, we can use this size
					bCreate = false;
				}
				else
				{
					// IB created, but just too small!
					SAFE_RELEASE(pIndexData->pIB);
					return NULL;
				}
			}
			#else
			if ( FAILED ( m_pD3D->CreateIndexBuffer ( 
														sizeof ( WORD ) * dwIndexBufferSize,
														D3DUSAGE_WRITEONLY,
														GGFMT_INDEX16,
														D3DPOOL_DEFAULT,
														&pIndexData->pIB,
														NULL
													 ) ) )
			{
				// failed, try half the size
				dwIndexBufferSize /= 2;
			}
			else
			{
				// only if IB can hold required vertex data
				if ( dwIndexBufferSize >= dwIndexCount )
				{
					// success, we can use this size
					bCreate = false;
				}
				else
				{
					// IB created, but just too small!
					SAFE_RELEASE(pIndexData->pIB);
					return NULL;
				}
			}
			#endif

			// if we continue until a ridiculously low value, we must fail
			if ( dwIndexBufferSize <= 0 )
				return NULL;
		}

		// ensure can fit inside the max size of a buffer
		if ( dwIndexCount > dwIndexBufferSize )
		{
			// if not, index data cannot fit inside single IB!
			SAFE_RELEASE ( pIndexData->pIB );
			return NULL;
		}

		// save the format of the buffer and the number allowed
		pIndexData->dwMaxIndexCount = dwIndexBufferSize;
		pIndexData->dwCurrentIndexCount = 0;
	}

	// return the final buffer
	return pIndexData;
}

CObjectManager::sVertexData* CObjectManager::FindVertexBuffer ( DWORD dwFVF, LPGGVERTEXLAYOUT pVertexDec, DWORD dwSize, DWORD dwVertexCount, DWORD dwIndexCount, bool bUsesItsOwnBuffers, int iType )
{
	// we need to find a buffer which the objects data can be added into
	// this function will go through the list of all buffers and find a
	// match for the FVF

	// check D3D device is valid
	if ( !m_pD3D ) return NULL;

	// make sure we have a FVF mode (FVF zero means we have converted it to declaration, fine for DX11)
	#ifndef DX11
	if ( dwFVF==0 && pVertexDec==NULL ) return NULL;
	#endif

	// make sure the parameters are valid
	if ( dwSize < 1 || dwVertexCount < 1 ) return NULL;

	// local variables
	bool			bMatch			= false;
	sVertexData*	pVertexData		= m_pVertexDataList;

	// get device capabilities
	DWORD dwPrimCountMax = 0;
	DWORD dwVBSize = 0;
	#ifdef DX11
	dwPrimCountMax = 65534;
	dwVBSize = 65534;
	#else
	GGCAPS caps;
	m_pD3D->GetDeviceCaps ( &caps );
	dwPrimCountMax = caps.MaxPrimitiveCount;
	dwVBSize = caps.MaxVertexIndex;
	#endif

	// make sure primitive count can be achieved
	if ( dwIndexCount>0 )
	{
		if ( dwIndexCount/3 > dwPrimCountMax )
			return NULL;
	}
	else
	{
		if ( dwVertexCount/3 > dwPrimCountMax )
			return NULL;
	}

	// no search if need to use its own buffer
	if ( bUsesItsOwnBuffers )
		pVertexData=NULL;

	// run through all nodes in list
	while ( pVertexData )
	{
		// check if vertex declarations match
		bool bVertDecMatch = false;
		if ( pVertexData->dwFormat==0 )
		{
			UINT numElementsThis;
			GGVERTEXELEMENT VertexDecFromThisBuffer[256];
			if ( pVertexData->pVertexDec )
			{
				#ifdef DX11
				/* How to determine vertex declarations match in DX11?
				HRESULT hr = pVertexData->pVertexDec->GetDeclaration( VertexDecFromThisBuffer, &numElementsThis);
				UINT numElementsRequest;
				GGVERTEXELEMENT VertexDecFromRequest[256];
				if ( pVertexDec )
				{
					hr = pVertexDec->GetDeclaration( VertexDecFromRequest, &numElementsRequest);
					DWORD dwCMPSize = sizeof(GGVERTEXELEMENT)*numElementsRequest;
					if ( numElementsRequest == numElementsThis )
						if ( memcmp ( VertexDecFromRequest, VertexDecFromThisBuffer, dwCMPSize )==0 )
							bVertDecMatch = true;
				}
				*/
				#else
				HRESULT hr = pVertexData->pVertexDec->GetDeclaration( VertexDecFromThisBuffer, &numElementsThis);
				UINT numElementsRequest;
				GGVERTEXELEMENT VertexDecFromRequest[256];
				if ( pVertexDec )
				{
					hr = pVertexDec->GetDeclaration( VertexDecFromRequest, &numElementsRequest);
					DWORD dwCMPSize = sizeof(GGVERTEXELEMENT)*numElementsRequest;
					if ( numElementsRequest == numElementsThis )
						if ( memcmp ( VertexDecFromRequest, VertexDecFromThisBuffer, dwCMPSize )==0 )
							bVertDecMatch = true;
				}
				#endif
			}
		}
		else
			bVertDecMatch = true;

		// see if we find a match to the FVF
		if ( pVertexData->dwFormat == dwFVF && bVertDecMatch==true )
		{
			// see if we can fit the data into the buffer
			if ( pVertexData->dwCurrentVertexCount + dwVertexCount < pVertexData->dwMaxVertexCount )
			{
				bMatch = true;
				break;
			}
		}

		// move to next node
		pVertexData = pVertexData->pNext;
	}

	// if we don't have a match then create a new VB
	if ( !bMatch )
	{
		if ( !m_pVertexDataList )
		{
			// create new list
			m_pVertexDataList		= new sVertexData;
			pVertexData				= m_pVertexDataList;
			SAFE_MEMORY ( m_pVertexDataList );
		}
		else
		{
			// find end of list
			pVertexData = m_pVertexDataList;
			while ( pVertexData )
			{
				if ( pVertexData->pNext )
				{
					pVertexData = pVertexData->pNext;
					continue;
				}
				else
					break;
			}

			// add new item to list
			pVertexData->pNext = new sVertexData ( );
			pVertexData        = pVertexData->pNext;
		}

		// create a decent start size of the VB
		bool bCreate = true;

		// if size exceeds 16bit, make max size 16bit (32bit index supported maybe in future though it shows no speed increase!!)
		if ( dwIndexCount > 0 )
		{
			// mesh uses index buffer so can only have a 16bit vertex buffer
			if ( dwVBSize > 0x0000FFFF ) dwVBSize = 0x0000FFFF;
		}

		// own buffer needs only to be the size of the data
		if ( bUsesItsOwnBuffers )
			dwVBSize = dwVertexCount;

		// loop until succeed in creating a VB
		#ifdef DX11
		while ( bCreate )
		{
			// attempt to create a VB
			/*
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage           = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth       = dwSize * dwVBSize;
			bufferDesc.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags       = 0;
			*/
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth       = dwSize * dwVBSize;
			bufferDesc.BindFlags       = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags  = 0;
			bufferDesc.MiscFlags       = 0;

			if ( FAILED ( m_pD3D->CreateBuffer( &bufferDesc, NULL, &pVertexData->pVB ) ) )
			{
				// failed, try half the size
				dwVBSize /= 2;
			}
			else
			{
				// only if VB can hold required vertex data
				if ( dwVBSize >= dwVertexCount )
				{
					// success, we can use this size
					bCreate = false;
				}
				else
				{
					// VB created, but just too small!
					SAFE_RELEASE(pVertexData->pVB);
					return NULL;
				}
			}

			// if we continue until a ridiculously low value, we must fail
			if ( dwVBSize <= 0 )
				return NULL;
		}
		#else
		while ( bCreate )
		{
			DWORD dwUsage = D3DUSAGE_WRITEONLY;

			if ( iType == D3DPT_POINTLIST )
				dwUsage = D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS;

			// attempt to create a VB
			if ( FAILED ( m_pD3D->CreateVertexBuffer ( 
														dwSize * dwVBSize,
														dwUsage,
														dwFVF,
														D3DPOOL_DEFAULT,
														&pVertexData->pVB,
														NULL
												     ) ) )
			{
				// failed, try half the size
				dwVBSize /= 2;
			}
			else
			{
				// only if VB can hold required vertex data
				if ( dwVBSize >= dwVertexCount )
				{
					// success, we can use this size
					bCreate = false;
				}
				else
				{
					// VB created, but just too small!
					SAFE_RELEASE(pVertexData->pVB);
					return NULL;
				}
			}

			// if we continue until a ridiculously low value, we must fail
			if ( dwVBSize <= 0 )
				return NULL;
		}
		#endif

		// save the format of the VB and the number of vertices allowed
		pVertexData->dwFormat					= dwFVF;
		pVertexData->pVertexDec					= pVertexDec;
		pVertexData->dwMaxVertexCount			= dwVBSize;
		pVertexData->dwCurrentVertexCount		= 0;
	}

	// return the final buffer
	return pVertexData;
}

bool CObjectManager::AddObjectMeshToBuffers ( sMesh* pMesh, bool bUsesItsOwnBuffers )
{
	// vertex and index buffer set up
	WORD*		 pIndices    = NULL;
	sVertexData* pVertexData = NULL;
	sIndexData*	 pIndexData  = NULL;

	// find a vertex buffer we can use which matches the FVF component
	pVertexData = this->FindVertexBuffer (	pMesh->dwFVF,
											pMesh->pVertexDec,
											pMesh->dwFVFSize,
											pMesh->dwVertexCount,						
											pMesh->dwIndexCount,
											bUsesItsOwnBuffers,
											pMesh->iPrimitiveType
										); // if no indexbuffer, can make larger vertex buffer (16bit index only)

	// check the vertex buffer is valid
	if ( pVertexData==NULL )
	{
		pMesh->pDrawBuffer = NULL;
		return false;
	}

	// find an index buffer (if one is required)
	if ( pMesh->dwIndexCount> 0 )
	{
		// find and check the index buffer is valid
		pIndexData = this->FindIndexBuffer ( pMesh->dwIndexCount, bUsesItsOwnBuffers );
		SAFE_MEMORY ( pIndexData );
	}

	// create a new vertex buffer reference array
	SAFE_DELETE ( pMesh->pDrawBuffer );
	pMesh->pDrawBuffer = new sDrawBuffer;

	// check the reference array is okay
	SAFE_MEMORY ( pMesh->pDrawBuffer );

	// draw primitive type
	pMesh->pDrawBuffer->dwPrimType			= ( GGPRIMITIVETYPE ) pMesh->iPrimitiveType;

	// store a reference to the vertex buffer
	pMesh->pDrawBuffer->pVertexBufferRef	= pVertexData->pVB;
	pMesh->pDrawBuffer->dwVertexStart		= ( pVertexData->dwPosition * sizeof(float) ) / pMesh->dwFVFSize;
	pMesh->pDrawBuffer->dwVertexCount		= pMesh->iDrawVertexCount;

	// primitive count for drawing
	pMesh->pDrawBuffer->dwPrimitiveCount	= pMesh->iDrawPrimitives;

	// store a reference to the indice buffer
	if( pIndexData )
	{
		pMesh->pDrawBuffer->pIndexBufferRef		= pIndexData->pIB;
		pMesh->pDrawBuffer->dwIndexStart		= pIndexData->dwCurrentIndexCount;
	}
	else
	{
		pMesh->pDrawBuffer->pIndexBufferRef		= NULL;
		pMesh->pDrawBuffer->dwIndexStart		= 0;
	}

	// store a reference to the FVF size
	pMesh->pDrawBuffer->dwFVFSize			= pMesh->dwFVFSize;

	// store references to the VB and IB list items
	pMesh->pDrawBuffer->pVBListEntryRef		= (LPVOID)pVertexData;
	pMesh->pDrawBuffer->pIBListEntryRef		= (LPVOID)pIndexData;

	// copy the indices (if applicable)
	if ( pIndexData )
	{
		// update base vertex index for indices (and adjust vertex start accordingly)
		pMesh->pDrawBuffer->dwBaseVertexIndex	 = ( WORD ) pVertexData->dwCurrentVertexCount;
		pMesh->pDrawBuffer->dwVertexStart		-= ( WORD ) pMesh->pDrawBuffer->dwBaseVertexIndex;

		// copy index data to index buffer (WORD based)
		CopyMeshDataToIndexBuffer ( pMesh, pIndexData->pIB, pIndexData->dwCurrentIndexCount );

		// increment the index count
		pIndexData->dwCurrentIndexCount += pMesh->dwIndexCount;
	}

	// copy vertex data to vertex buffer
	CopyMeshDataToVertexBufferSameFVF ( pMesh, pVertexData->pVB, pVertexData->dwPosition );

	// save the current position
	pVertexData->dwPosition += pMesh->dwVertexCount * ( pMesh->dwFVFSize/sizeof(float) );

	// increment the vertex count in the buffer
	pVertexData->dwCurrentVertexCount += pMesh->dwVertexCount;

	// everything went okay
	return true;
}

bool CObjectManager::AddObjectToBuffers ( sObject* pObject )
{
	// vertex and index buffer set up
	bool			bAllOkay		= true;
	WORD*			pIndices		= NULL;
	sVertexData*	pVertexData		= NULL;
	sIndexData*		pIndexData		= NULL;

	// run through each frame within an object
	for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
	{
		// get frame ptr
		sFrame* pFrame = pObject->ppFrameList [ iFrame ];

		// add each mesh to the buffers
		if ( pFrame->pMesh )		if ( !AddObjectMeshToBuffers ( pFrame->pMesh, pObject->bUsesItsOwnBuffers ) )				bAllOkay=false;
		if ( pFrame->pLOD[0] )		if ( !AddObjectMeshToBuffers ( pFrame->pLOD[0], pObject->bUsesItsOwnBuffers ) )				bAllOkay=false;
		if ( pFrame->pLOD[1] )		if ( !AddObjectMeshToBuffers ( pFrame->pLOD[1], pObject->bUsesItsOwnBuffers ) )				bAllOkay=false;
		if ( pFrame->pLODForQUAD )	if ( !AddObjectMeshToBuffers ( pFrame->pLODForQUAD, pObject->bUsesItsOwnBuffers ) )				bAllOkay=false;
	}

	// update texture list when introduce new object
	m_bUpdateTextureList=true;

	// everything went..
	return bAllOkay;
}


bool CObjectManager::FlagAllObjectMeshesUsingBuffer ( sVertexData* pVertexData, sIndexData* pIndexData )
{
	// flag any object mesh that uses either of these buffers
	for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iObjectID = g_ObjectListRef [ iShortList ];

		// see if we have a valid list
		sObject* pObject = g_ObjectList [ iObjectID ];
		if ( !pObject )
			continue;

		// run through each frame within an object
		for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
		{
			// get frame ptr
			sFrame* pFrame = pObject->ppFrameList [ iFrame ];

			// add this object:mesh back in
			if ( pFrame->pMesh )
			{
				if ( pFrame->pMesh->pDrawBuffer )
				{
					if(pFrame->pMesh->pDrawBuffer->pVBListEntryRef==(LPVOID)pVertexData
					|| pFrame->pMesh->pDrawBuffer->pIBListEntryRef==(LPVOID)pIndexData )
					{
						// leefix - 070403 - ensure associated meshes are also removed from buffers to prevent duplication!
						if ( pFrame->pMesh->bAddObjectToBuffers==false )
						{
							pFrame->pMesh->bAddObjectToBuffers=true;
							// mike - 301106 - add flag to stop recursion
							RemoveBuffersUsedByObjectMesh ( pFrame->pMesh, false );
						}
					}
				}
			}
			for ( int l=0; l<3; l++ )
			{
				sMesh* pLODMesh = NULL;
				if ( l<2 )
					pLODMesh = pFrame->pLOD[l];
				else
					pLODMesh = pFrame->pLODForQUAD;

				if ( pLODMesh )
				{
					if ( pLODMesh->pDrawBuffer )
					{
						if(pLODMesh->pDrawBuffer->pVBListEntryRef==(LPVOID)pVertexData
						|| pLODMesh->pDrawBuffer->pIBListEntryRef==(LPVOID)pIndexData )
						{
							if ( pLODMesh->bAddObjectToBuffers==false )
							{
								pLODMesh->bAddObjectToBuffers=true;
								RemoveBuffersUsedByObjectMesh ( pLODMesh, false );
							}
						}
					}
				}	
			}
		}
	}

	// everything went okay
	return true;
}

// mike - 301106 - add flag to stop recursion
bool CObjectManager::RemoveBuffersUsedByObjectMesh ( sMesh* pMesh, bool bRecurse )
{
	// get reference to drawbuffer
	sDrawBuffer* pDrawBuffer = pMesh->pDrawBuffer;
	if(pDrawBuffer)
	{
		DWORD* pdwAdd = (DWORD*)&pMesh->pDrawBuffer;

		// get reference to VB and IB ptrs
		sVertexData* pVertexData = (sVertexData*)pDrawBuffer->pVBListEntryRef;
		sIndexData*	 pIndexData  = (sIndexData* )pDrawBuffer->pIBListEntryRef;

		// scan for and delete vertex item
		sVertexData* pLastVertexData = NULL;
		sVertexData* pFindVertexData = m_pVertexDataList;
		while ( pFindVertexData )
		{
			// check this item
			sVertexData* pNextVertexData = pFindVertexData->pNext;
			if ( pFindVertexData==pVertexData )
			{
				// sever and delete vertex item
				pFindVertexData->pNext=NULL;

				// free VB
				SAFE_RELEASE( pFindVertexData->pVB );

				// delete vertexdata
				SAFE_DELETE( pFindVertexData );

				// adjust next value to leap deleted item
				if ( pLastVertexData ) pLastVertexData->pNext = pNextVertexData;

				// new start item to replace deleted one
				if ( m_pVertexDataList==pVertexData )
					m_pVertexDataList = pNextVertexData;

				// done here
				break;
			}

			// next item
			pLastVertexData = pFindVertexData;
			pFindVertexData = pNextVertexData;
		}

		// scan for and delete index item
		sIndexData* pLastIndexData = NULL;
		sIndexData* pFindIndexData = m_pIndexDataList;
		while ( pFindIndexData )
		{
			// check this item
			sIndexData* pNextIndexData = pFindIndexData->pNext;
			if ( pFindIndexData==pIndexData )
			{
				// sever and delete Index item
				pFindIndexData->pNext=NULL;

				// release IB
				SAFE_RELEASE ( pFindIndexData->pIB );

				// delete indexdata
				SAFE_DELETE( pFindIndexData );

				// adjust next value to leap deleted item
				if ( pLastIndexData ) pLastIndexData->pNext = pNextIndexData;

				// new start item to replace deleted one
				if ( m_pIndexDataList==pIndexData )
					m_pIndexDataList = pNextIndexData;

				// done here
				break;
			}

			// next item
			pLastIndexData = pFindIndexData;
			pFindIndexData = pNextIndexData;
		}

		// flag any objects that used either of these buffers
		if ( bRecurse == true )
		{
			// useful when we KNOW that the VB IB buffers are not shared
			FlagAllObjectMeshesUsingBuffer ( pVertexData, pIndexData );
		}
	}

	// everything went okay
	return true;
}

// lee - 140307 - added to delete buffers quickly if we know VB/IBs are not shared
bool CObjectManager::RemoveBuffersUsedByObjectMeshDirectly ( sMesh* pMesh )
{
	// delete the buffers from the lit, and release the ptrs, but do NOT assume buffers are shared!
	return RemoveBuffersUsedByObjectMesh ( pMesh, false );
}

bool CObjectManager::RemoveBuffersUsedByObject ( sObject* pObject )
{
	// run through each frame within an object
	for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
	{
		// get frame ptr
		sFrame* pFrame = pObject->ppFrameList [ iFrame ];

		// lee - 140307 - if object 'uses its own buffers' there is no need to use
		// a recursive check for whether the VB/IB buffers are shared, as they cannot be
		// so we can simply delete the buffers with out recursive (buffer remove/flag back in)
		if ( pObject->bUsesItsOwnBuffers==true )
		{
			// delete all vertex/index entries 'directly' used by this mesh (no recursive)
			if(pFrame->pMesh) RemoveBuffersUsedByObjectMeshDirectly ( pFrame->pMesh );
			if(pFrame->pLOD[0]) RemoveBuffersUsedByObjectMeshDirectly ( pFrame->pLOD[0] );
			if(pFrame->pLOD[1]) RemoveBuffersUsedByObjectMeshDirectly ( pFrame->pLOD[1] );
			if(pFrame->pLODForQUAD) RemoveBuffersUsedByObjectMeshDirectly ( pFrame->pLODForQUAD );
		}
		else
		{
			// delete all vertex/index entries used by this mesh
			if(pFrame->pMesh) RemoveBuffersUsedByObjectMesh ( pFrame->pMesh );
			if(pFrame->pLOD[0]) RemoveBuffersUsedByObjectMesh ( pFrame->pLOD[0] );
			if(pFrame->pLOD[1]) RemoveBuffersUsedByObjectMesh ( pFrame->pLOD[1] );
			if(pFrame->pLODForQUAD) RemoveBuffersUsedByObjectMesh ( pFrame->pLODForQUAD );
		}
	}

	// everything went okay
	return true;
}

bool CObjectManager::AddFlaggedObjectsBackToBuffers ( void )
{
	// upon buffer removal, some object where flagged for re-creation
	for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iObjectID = g_ObjectListRef [ iShortList ];

		// see if we have a valid list
		sObject* pObject = g_ObjectList [ iObjectID ];
		if ( !pObject )
			continue;

		// 210214 - completely ignore excluded objects
		if ( pObject->bExcluded ) continue;

		// also ensure we skip the removed object (not to be re-added)
		if ( pObject->bReplaceObjectFromBuffers==true )
			continue;

		// run through each frame within an object
		for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
		{
			// refresh the VB data for this mesh (it will auto-lock the VB)
			sFrame* pFrame = pObject->ppFrameList [ iFrame ];

			// add this object:mesh back in
			if ( pFrame->pMesh )
			{
				if ( pFrame->pMesh->bAddObjectToBuffers==true )
				{
					AddObjectMeshToBuffers ( pFrame->pMesh, pObject->bUsesItsOwnBuffers );
					pFrame->pMesh->bAddObjectToBuffers=false;
				}
			}
			// add this object:mesh back in
			for ( int l=0; l<3; l++ )
			{
				sMesh* pLODMesh = NULL;
				if ( l<2 )
					pLODMesh = pFrame->pLOD[l];
				else
					pLODMesh = pFrame->pLODForQUAD;

				if ( pLODMesh )
				{
					if ( pLODMesh->bAddObjectToBuffers==true )
					{
						AddObjectMeshToBuffers ( pLODMesh, pObject->bUsesItsOwnBuffers );
						pLODMesh->bAddObjectToBuffers=false;
					}
				}
			}
		}
	}

	// everything went okay
	return true;
}

bool CObjectManager::RemoveObjectFromBuffers ( sObject* pRemovedObject )
{
	// delete all buffers that this object resided in
	RemoveBuffersUsedByObject ( pRemovedObject );

	// upon buffer removal, some object where flagged for re-creation
	AddFlaggedObjectsBackToBuffers ();

	// update texture list when introduce new object(s)
	UpdateTextures();

	// everything went okay
	return true;
}

bool CObjectManager::ReplaceAllFlaggedObjectsInBuffers ( void )
{
	// only if global flag switched
	if ( g_bObjectReplacedUpdateBuffers )
	{
		// delete all buffers that these object resided in
		for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
		{
			// get index from shortlist
			int iObjectID = g_ObjectListRef [ iShortList ];
			sObject* pRemovedObject = g_ObjectList [ iObjectID ];
			if ( pRemovedObject )
			{
				// 210214 - completely ignore excluded objects
				if ( pRemovedObject->bExcluded ) continue;

				if ( pRemovedObject->bReplaceObjectFromBuffers )
					RemoveBuffersUsedByObject ( pRemovedObject );
			}
		}

		// upon buffer removal, some object where flagged for re-creation
		AddFlaggedObjectsBackToBuffers ();

		// when all buffers clear of removed objects, can add new instances of them back in..
		for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
		{
			// get index from shortlist
			int iObjectID = g_ObjectListRef [ iShortList ];
			sObject* pObject = g_ObjectList [ iObjectID ];
			if ( pObject )
			{
				// 210214 - completely ignore excluded objects
				if ( pObject->bExcluded ) continue;

				if ( pObject->bReplaceObjectFromBuffers )
				{
					// add object back in
					AddObjectToBuffers ( pObject );

					// and clear flag 
					pObject->bReplaceObjectFromBuffers = false;		
				}
			}
		}

		// update texture list when introduce new object(s)
		UpdateTextures();

		// reset global flag
		g_bObjectReplacedUpdateBuffers = false;
	}

	// everything went okay
	return true;
}

bool CObjectManager::UpdateObjectMeshInBuffer ( sMesh* pMesh )
{
	// only if have a drawbuffer
	if ( !pMesh->pDrawBuffer )
		return false;

	// if drawbuffer is insufficient, i.e. not big enough
	if ( pMesh->pDrawBuffer->dwVertexCount < pMesh->dwVertexCount 
	||	 pMesh->pDrawBuffer->dwFVFSize != pMesh->dwFVFSize )	
	{
		// recreate drawbuffer
		RemoveBuffersUsedByObjectMesh ( pMesh, true );
		AddObjectMeshToBuffers ( pMesh, true );
		return true;
	}

	// get the offset map
	sOffsetMap	offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// get vertex list item pointer
	sVertexData* pVertexData = (sVertexData*)pMesh->pDrawBuffer->pVBListEntryRef;
	IGGVertexBuffer* pVertexBuffer = pVertexData->pVB;

	// lock the vertex buffer (if not already locked)
	#ifdef DX11
	#else
	if ( pVertexData->bBufferLocked==false )
	{
		pVertexData->pfLockedData = NULL;
		if ( FAILED ( pVertexBuffer->Lock ( 0, 0, ( VOID** ) &pVertexData->pfLockedData, 0 ) ) )
		{
			return false;
		}

		// set the VB flag when locked
		pVertexData->bBufferLocked = true;
	}
	#endif

	// copy vertex-data-block from object to VB
	DWORD dwPosWithinVB = (pMesh->pDrawBuffer->dwBaseVertexIndex + pMesh->pDrawBuffer->dwVertexStart) * pMesh->dwFVFSize;
	LPVOID pSourceData = pMesh->pVertexData;
	DWORD dwSizeToCopy = pMesh->dwVertexCount * pMesh->dwFVFSize;
	#ifdef DX11
	D3D11_BOX box;
	box.left = dwPosWithinVB;
	box.right = dwPosWithinVB + dwSizeToCopy;
	box.top = 0;
	box.bottom = 1;
	box.front = 0;
	box.back = 1;
	m_pImmediateContext->UpdateSubresource ( pVertexBuffer, 0, &box, pMesh->pVertexData, 0, 0 );
	#else
	LPVOID pDestPtr = pVertexData->pfLockedData + dwPosWithinVB;
	if ( pDestPtr ) memcpy ( pDestPtr, pSourceData, dwSizeToCopy );
	#endif

	// draw quantity can change without having to recreate (like for shadows)
	if ( pMesh->pDrawBuffer )
	{
		pMesh->pDrawBuffer->dwVertexCount		= pMesh->iDrawVertexCount;
		pMesh->pDrawBuffer->dwPrimitiveCount	= pMesh->iDrawPrimitives;
	}

	// leeadd - 230304 - physics changes INDEX DATA TOO,
	// which a VB update above does not do..so add this
	if ( pMesh->pDrawBuffer )
	{
		// only if index buffer exists
		if ( pMesh->pDrawBuffer->pIndexBufferRef )
			CopyMeshDataToIndexBuffer ( pMesh, pMesh->pDrawBuffer->pIndexBufferRef, pMesh->pDrawBuffer->dwIndexStart );
	}

	// everything went okay
	return true;
}

bool CObjectManager::UpdateAllObjectsInBuffers ( void )
{
	// objects that have changed are flagged, and passed to VB updater..

	// lee - 300914 - now uses a short list added to when meshes require refreshing (much faster)
	if ( !g_vRefreshMeshList.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < g_vRefreshMeshList.size(); ++iIndex )
        {
			// get mesh to refresh
            sMesh* pMesh = g_vRefreshMeshList [ iIndex ];
			if ( !pMesh ) continue;

			// only refresh if not already done so (can have multiple entries in this list)
			if ( pMesh->bVBRefreshRequired==true )
			{
				UpdateObjectMeshInBuffer ( pMesh );
				pMesh->bVBRefreshRequired=false;
			}
		}

		// go through all vertex buffer items (unlock any that have been locked)
		CompleteUpdateInBuffers();

		// clear refresh list for next cycle (must quicker than going through ALL objects each cycle)
		g_vRefreshMeshList.clear();
	}

	// okay
	return true;
}

bool CObjectManager::CompleteUpdateInBuffers ( void )
{
	// go through all vertex buffer items (unlock any that have been locked)
	sVertexData* pVertexData = m_pVertexDataList;
	while ( pVertexData )
	{
		// if buffer has been locked
		#ifdef DX11
		#else
		if ( pVertexData->bBufferLocked==true )
		{
			// unlock and restore flag
			pVertexData->pVB->Unlock ( );
			pVertexData->bBufferLocked=false;
		}
		#endif

		// move to next node
		pVertexData = pVertexData->pNext;
	}

	// okay
	return true;
}

bool CObjectManager::QuicklyUpdateObjectMeshInBuffer ( sMesh* pMesh, DWORD dwVertexFrom, DWORD dwVertexTo )
{
	// only if have a drawbuffer
	if ( !pMesh->pDrawBuffer )
		return false;

	// get the offset map
	sOffsetMap	offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// get vertex list item pointer
	sVertexData* pVertexData = (sVertexData*)pMesh->pDrawBuffer->pVBListEntryRef;
	IGGVertexBuffer* pVertexBuffer = pVertexData->pVB;

	// lock the vertex buffer (if not already locked)
	#ifdef DX11
	#else
	if ( pVertexData->bBufferLocked==false )
	{
		pVertexData->pfLockedData = NULL;
		if ( FAILED ( pVertexBuffer->Lock ( 0, 0, ( VOID** ) &pVertexData->pfLockedData, 0 ) ) )
			return false;

		// set the VB flag when locked
		pVertexData->bBufferLocked = true;
	}
	#endif

	// copy only vertex data changed as described in params passed in
	DWORD dwPosWithinVB = (pMesh->pDrawBuffer->dwBaseVertexIndex + pMesh->pDrawBuffer->dwVertexStart + dwVertexFrom) * pMesh->dwFVFSize;
	LPVOID pDestPtr = pVertexData->pfLockedData + dwPosWithinVB;
	DWORD dwPosWithinMesh = dwVertexFrom * pMesh->dwFVFSize;
	LPVOID pSourceData = pMesh->pVertexData + dwPosWithinMesh;
	DWORD dwSizeToCopy = (dwVertexTo-dwVertexFrom) * pMesh->dwFVFSize;
	memcpy ( pDestPtr, pSourceData, dwSizeToCopy );

	// draw quantity can change without having to recreate (like for shadows)
	pMesh->pDrawBuffer->dwVertexCount		= pMesh->iDrawVertexCount;
	pMesh->pDrawBuffer->dwPrimitiveCount	= pMesh->iDrawPrimitives;

	// everything went okay
	return true;
}

bool CObjectManager::RenewReplacedMeshes ( sObject* pObject )
{
	// run through each mesh within an object
	bool bReplaceObjectOwningMesh=false;
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// replace any meshes in VB/IB that have been changed
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		if ( pMesh->bMeshHasBeenReplaced==true )
		{
			pMesh->bMeshHasBeenReplaced=false;
			bReplaceObjectOwningMesh=true;
			break;
		}
	}

	// replace object
	if ( bReplaceObjectOwningMesh )
	{
		pObject->bReplaceObjectFromBuffers = true;
		g_bObjectReplacedUpdateBuffers = true;
		bReplaceObjectOwningMesh = false;
	}

	// okay
	return true;
}

bool CObjectManager::RefreshObjectInBuffer ( sObject* pObject )
{
	pObject->bReplaceObjectFromBuffers = true;
	g_bObjectReplacedUpdateBuffers = true;
	return true;
}

void CObjectManager::RemoveTextureRefFromAllObjects ( LPGGTEXTURE pTextureRef )
{
	// run through all objects
	for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
	{
		// get index from shortlist
		int iObjectID = g_ObjectListRef [ iShortList ];

		// see if we have a valid list
		sObject* pObject = g_ObjectList [ iObjectID ];
		if ( !pObject )
			continue;

		// run through each frame within an object
		for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
		{
			// get frame
			sFrame* pFrame = pObject->ppFrameList [ iFrame ];
			sMesh* pMesh = pFrame->pMesh;
			if ( pMesh )
			{
				// go through all textures in mesh
				RemoveTextureRefFromMesh ( pMesh, pTextureRef );
			}
		}
	}
}

bool CObjectManager::SortTextureList ( void )
{
    // If the object list size hasn't changed and no textures have changed, then nothing to do here.
	//Dave Performance, can force an update with g_ForceTextureListUpdate flag set to true, to take into account ignored objects
	if ( !g_ForceTextureListUpdate )
	{
		if ( m_iLastCount == g_iObjectListRefCount && m_bUpdateTextureList==false )
			return true;
	}

    // Reset ready for next time
    m_iLastCount         = g_iObjectListRefCount;
    m_bUpdateTextureList = false;

    // make sure the lists we're using are valid
    SAFE_MEMORY ( g_ppSortedObjectList );
    SAFE_MEMORY ( m_ppSortedObjectVisibleList );

    // reset all data so we can build the list from scratch
    g_iSortedObjectCount = 0;
    g_bRenderVeryEarlyObjects = false;

	// at same time, collect object lists that DO NOT change from cycle to cycle
	g_vAnimatableObjectList.clear();

    // run through all known items and put them into render list ready for sorting
    for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
    {
	    // Get an object id from shortlist
	    int iObjectID = g_ObjectListRef [ iShortList ];

	    // Actual object or instance of object
	    sObject* pOriginalObject = g_ObjectList [ iObjectID ];
        if ( ! pOriginalObject )
            continue;

		// quick reject objects which have a sync mask of ZERO
		if ( pOriginalObject->dwCameraMaskBits==0 )
			continue;

		//Dave Performance - do not add ignored objects into the list
		if ( pOriginalObject->bIgnored )
			continue;

		// 210214 - quick reject objects which are excluded
		if ( pOriginalObject->bExcluded )
			continue;

		// lee - 300914 - if this object holds some animation data, add to animatable list (done once per texture sort for speed!)
		if ( pOriginalObject->pAnimationSet ) 
		{
			g_vAnimatableObjectList.push_back ( pOriginalObject );
		}

		// get the object we would render
        sObject* pRenderObject = pOriginalObject;
        if ( pRenderObject->pInstanceOfObject )
		    pRenderObject = pRenderObject->pInstanceOfObject;

	    // See if we have enough information to render this object
        // A (possibly instanced) object with a mesh list and with mesh 0 having a texture.
        if ( pRenderObject && pRenderObject->ppMeshList && pRenderObject->ppMeshList[0]->pTextures)
	    {
            // Add the original object into the render list
            g_ppSortedObjectList [ g_iSortedObjectCount++ ] = pOriginalObject;

            // If we are sorting by distance, calculate the distance ready for sorting
            if ( g_eGlobalSortOrder == E_SORT_BY_DEPTH )
		    {
			    if ( pOriginalObject->bVeryEarlyObject == true )
			    {
                    // very early objects are placed at extreme distance
				    pOriginalObject->position.fCamDistance = 9999999.9f;
			    }
			    else
			    {
				    pOriginalObject->position.fCamDistance = CalculateObjectDistanceFromCamera ( pOriginalObject );
			    }
            }
            
            // Check to see if there is an early draw object
            if ( pOriginalObject->bVeryEarlyObject == true )
		    {
			    // If this object is an early draw, set global flag to show we have one in the scene
                g_bRenderVeryEarlyObjects = true;
            }
	    }
    }

    // Now sort the list appropriately
    switch(g_eGlobalSortOrder)
    {
		case E_SORT_BY_TEXTURE:
			std::sort(g_ppSortedObjectList, g_ppSortedObjectList + g_iSortedObjectCount, OrderByTexture() );
			break;
		case E_SORT_BY_OBJECT:
			std::sort(g_ppSortedObjectList, g_ppSortedObjectList + g_iSortedObjectCount, OrderByObject() );
			break;
		case E_SORT_BY_DEPTH:
			// Delay sort until the visibility list is generated
			// This increases the accuracy of the sorting for depth.
			break;
		case E_SORT_BY_NONE:
			// No sort needed
			break;
		default:
			break;
	}

	// return back to caller
	return true;
}

//Dave Performance - used for shadows to ignore objects that are far away (relative to size) to stop them being considered for shadows
extern bool g_bIgnoreFarObjects;
extern int g_HideDistantShadows;
bool waited = false;

bool CObjectManager::SortVisibilityList ( void )
{
	// run through the sorted texture list and find out which objects
	// are visible, then create a new list which stores the visible objects

	// can quickly skip this step if the camera does not need to use latest visibilities (cheap shadow quad camera 5)
	if ( ((1<<g_pGlob->dwRenderCameraID) & m_dwSkipVisibilityListMask)!=0 )
		return true;

	// make sure we have a valid object
	if ( g_ppSortedObjectList==NULL )
		return true;

	// reset the number of visible objects to 0
	m_iVisibleObjectCount = 0;
    m_vVisibleObjectList.clear();
    m_vVisibleObjectEarly.clear();
    m_vVisibleObjectTransparent.clear();
    m_vVisibleObjectNoZDepth.clear();
    m_vVisibleObjectStandard.clear();

    // when the textures have been sorted we have a counter which stores
	// the number of sorted objects, this is m_iSortedObjectCount, we now
	// run through the sorted texture list and find which objects are visible
	for ( int iSort = 0; iSort < g_iSortedObjectCount; iSort++ )
	{
		// get a pointer to the object from the sorted draw list
		sObject* pObject = g_ppSortedObjectList [ iSort ];
		if ( !pObject ) continue;

		// if the object is not visible, and not a parent that is animating, continue
		if ( ( !pObject->bVisible || (!pObject->bUniverseVisible && g_bIgnoreFarObjects == false) ) )
		   continue;

		// Dave Performance - this is used when sorting the list for shadow maps
		// If the objects are a certain size and certain distance away we skip their shadow
		// Smaller objects will drop their shadow earlier than larger ones
		if ( g_bIgnoreFarObjects )
		{
			// ensure have latest object center
			if ( !pObject->bIsStatic )
			{
				if ( pObject->bUniverseVisible )
					UpdateColCenter ( pObject );
				else
					continue;
			}

			// get center of the object
			GGVECTOR3 vecRealCenter = pObject->position.vecPosition + pObject->collision.vecColCenter;

			// work out distance and recommended disappearance distance
			float dx = CameraPositionX( g_pGlob->dwCurrentSetCameraID ) - vecRealCenter.x;
			float dy = CameraPositionY( g_pGlob->dwCurrentSetCameraID ) - vecRealCenter.y;
			float dz = CameraPositionZ( g_pGlob->dwCurrentSetCameraID ) - vecRealCenter.z;
			float dist = sqrtf ( dx*dx + dy*dy + dz*dz );
			float dmax = 1000;
			if ( g_HideDistantShadows == 1 )
			{
				// default from V1.14 small and large entity shadows culled from vis list
				if ( pObject->collision.fScaledLargestRadius > 100 ) dmax = 1500;
				if ( pObject->collision.fScaledLargestRadius > 300 ) dmax = 2500; // 070216 - was 150
				if ( pObject->collision.fScaledLargestRadius > 500 ) dmax = 4000;
			}
			if ( g_HideDistantShadows == 2 )
			{
				// protect larger structures from being removed from vis list
				if ( pObject->collision.fScaledLargestRadius > 200 ) dmax = 8000;
			}
			if ( dist > dmax ) continue;
		}

		// VISIBILITY CULLING CHECK PROCESS
		bool bIsVisible=false;

		// 20120307 IRM
		// If the object is a parent to an instance and is animating, then always
		// count it as visible to ensure that any instance animations continue.
		// This is true even if the object is off-screen, hidden or even excluded.
		if (pObject->position.bParentOfInstance && pObject->bAnimPlaying)
		{
			bIsVisible = true;
		}
		else
		{
			// actual object or instance of object
			sObject* pActualObject = pObject;
			if ( pActualObject->pInstanceOfObject )
				pActualObject=pActualObject->pInstanceOfObject;

			// locked objects are always visible
			// glued objects are always visible (deferred to parent visibility)
			int iGluedToObj = pObject->position.iGluedToObj;
			if ( pObject->bLockedObject || iGluedToObj!=0 )
			{
				// leefix -040803- maintenance check, if glued to object that has been deleted, deal with it
				if ( iGluedToObj!=0 )
				{
					sObject* pParentObject = g_ObjectList [ iGluedToObj ];
					if ( pParentObject==NULL )
					{
						// wipe out glue assignment
						pObject->position.bGlued		= false;
						pObject->position.iGluedToObj	= 0;
						pObject->position.iGluedToMesh	= 0;
					}
				}

				// locked objects and glued are visible
				bIsVisible=true;
			}
			else
			{
				// send the position of the object and it's radius to the "CheckSphere" function, if this returns true the object will be visible
				float fScaledRadius = pObject->collision.fScaledLargestRadius;
				if ( fScaledRadius<=0.0f )
				{
					// objects with no mesh scope are visible
					bIsVisible=true;
				}
				else
				{
					// ensure have latest object center
					// only do this is the object is not static
					if ( !pObject->bIsStatic )
						UpdateColCenter ( pObject );

					// get center of the object
					GGVECTOR3 vecRealCenter = pObject->position.vecPosition + pObject->collision.vecColCenter;

					// leeadd - 100805 - add in offset from first frame (limb zero), as this moves whole object render)
					if ( pActualObject->ppFrameList )
					{
						sFrame* pRootFrame = pActualObject->ppFrameList [ 0 ];
						if ( pRootFrame )
						{
							// leeadd - 211008 - u71 - added flag to NOT shift object bounds by frame zero matrix
							if ( (pRootFrame->dwStatusBits && 1)==0 ) 
							{
								// offset center to account for movement of the object by limb zero (root frame)
								vecRealCenter.x += pRootFrame->matUserMatrix._41;
								vecRealCenter.y += pRootFrame->matUserMatrix._42;
								vecRealCenter.z += pRootFrame->matUserMatrix._43;
							}
						}
					}

					// to avoid ugly clipping issues, double radius for objects that are anim-shifted
					float fFinalRadiusForVisCull = fScaledRadius * 2.0f;

					// objects within frustrum are visible
					if ( CheckSphere ( vecRealCenter.x, vecRealCenter.y, vecRealCenter.z, fFinalRadiusForVisCull ) )
						bIsVisible=true;
				}
			}
		}

		// determine visiblity

		// MIKE - 021203 - added in second part of if statement for external objects, physics DLL
		if ( bIsVisible || pObject->bDisableTransform == true )
		{
			// save a pointer to the object and place it in the new drawlist
			m_ppSortedObjectVisibleList [ m_iVisibleObjectCount++ ] = pObject;

            // Build individual draw lists for each layer
            if (pObject->bVeryEarlyObject == true)
            {
                m_vVisibleObjectEarly.push_back( pObject );
            }
            else if ( pObject->bNewZLayerObject || pObject->bLockedObject )
            {
				m_vVisibleObjectNoZDepth.push_back( pObject );
            }
            else if ( pObject->bGhostedObject || pObject->bTransparentObject )
            {
                m_vVisibleObjectTransparent.push_back( pObject );
            }
            else
            {
                m_vVisibleObjectStandard.push_back( pObject );
            }

            // u74b8 - If sort order is by distance, update the object distance
            if (g_eGlobalSortOrder == E_SORT_BY_DEPTH)
            {
			    if ( pObject->bVeryEarlyObject == true )
			    {
                    // very early objects are placed at extreme distance
				    pObject->position.fCamDistance = 9999999.9f;
			    }
			    else
			    {
				    pObject->position.fCamDistance = CalculateObjectDistanceFromCamera ( pObject );
			    }
            }
		}
	}

    // u74b8 - If sort order is by distance, sort the list into the correct order
    //         as it varies by camera.
    if (g_eGlobalSortOrder == E_SORT_BY_DEPTH)
    {
        // No ghost/transparent sort just yet - still need to take into account water -
        //but do need to sort everything else.
        std::sort( m_vVisibleObjectEarly.begin(), m_vVisibleObjectEarly.end(), OrderByReverseCameraDistance() );
        std::sort( m_vVisibleObjectNoZDepth.begin(), m_vVisibleObjectNoZDepth.end(), OrderByReverseCameraDistance() );
        std::sort( m_vVisibleObjectStandard.begin(), m_vVisibleObjectStandard.end(), OrderByReverseCameraDistance() );
    }

	// all went okay
	return true;
}

bool CObjectManager::UpdateTextures ( void )
{
	// clear tep list immediately as now invalid
	g_vAnimatableObjectList.clear();

	// triggers texture list update
	m_bUpdateTextureList=true;
	return true;
}

void CObjectManager::UpdateAnimationCyclePerObject ( sObject* pObject )
{
	// simply control animation frame
	if ( pObject->bAnimPlaying )
	{
		// advance frame
		pObject->fAnimFrame += pObject->fAnimSpeed;

		// if reach end
		if ( pObject->fAnimFrame >= pObject->fAnimFrameEnd )
		{
			// if animation loops
			if ( pObject->bAnimLooping==false )
			{
				// U76 - 300710 - ensure we clip any over-run so we're dead on the final frame
				pObject->fAnimFrame = pObject->fAnimFrameEnd;

				// stop playing if reach end frame
				pObject->bAnimPlaying = false;
			}
			else
			{
				// leefix - 190303 - beta 4.7 - so play anim stays on last frame (is this DBV1 friendly?)
				pObject->fAnimFrame = pObject->fAnimLoopStart;
			}
		}

		// leeadd - 300605 - support looping frames backwards - speed can be minus!
		if ( pObject->fAnimSpeed<0 && pObject->fAnimFrame < pObject->fAnimLoopStart )
		{
			if ( pObject->bAnimLooping==false )
				pObject->bAnimPlaying = false;
			else
				pObject->fAnimFrame = pObject->fAnimFrameEnd;
		}
	}
	else
	{
		// control manual slerp
		if ( pObject->bAnimManualSlerp )
		{
			pObject->fAnimSlerpTime += pObject->fAnimInterp;
			if ( pObject->fAnimSlerpTime >= 1.0f )
			{
				pObject->bAnimManualSlerp = false;
				pObject->fAnimFrame = pObject->fAnimSlerpEndFrame;
			}
		}
	}

	// leeadd - 080305 - copy animation bound boxes to collision boundbox
	// leefix - 310305 - ONLY if not using the fixed box check from (make object collision box)
	// lee - 140306 - u60b3 - added bUseBoxCollision as this was not accounted with first flag
	// lee - 160415 - need this for intersectall bounds of animating objects if ( pObject->collision.bFixedBoxCheck==false && pObject->collision.bUseBoxCollision==false )
	if ( pObject->collision.bUseBoxCollision==false )
	{
		if ( pObject->pAnimationSet )
		{
			if ( pObject->pAnimationSet->pvecBoundMin )
			{
				int iThisKeyFrame = (int)pObject->fAnimFrame;
				if ( iThisKeyFrame > (int)pObject->pAnimationSet->ulLength ) iThisKeyFrame = pObject->pAnimationSet->ulLength-1;
				pObject->collision.vecMin = pObject->pAnimationSet->pvecBoundMin [ iThisKeyFrame ];
				pObject->collision.vecMax = pObject->pAnimationSet->pvecBoundMax [ iThisKeyFrame ];
				pObject->collision.vecCentre = pObject->pAnimationSet->pvecBoundCenter [ iThisKeyFrame ];
				pObject->collision.fRadius = pObject->pAnimationSet->pfBoundRadius [ iThisKeyFrame ];
			}
		}
		if ( pObject->ppMeshList )
		{
			pObject->ppMeshList [ 0 ]->Collision.vecMin = pObject->collision.vecMin;
			pObject->ppMeshList [ 0 ]->Collision.vecMax = pObject->collision.vecMax;
			pObject->ppMeshList [ 0 ]->Collision.vecCentre = pObject->collision.vecCentre;
			pObject->ppMeshList [ 0 ]->Collision.fRadius = pObject->collision.fRadius;
		}
	}
}

bool CObjectManager::UpdateAnimationCycle ( void )
{
	// lee - 300914 - new way only runs through object list of known objects with animations
	if ( !g_vAnimatableObjectList.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < g_vAnimatableObjectList.size(); ++iIndex )
        {
			// get mesh to refresh
            sObject* pObject = g_vAnimatableObjectList [ iIndex ];
			if ( !pObject ) continue;

			// if not visible and not an animating parent, skip
			if ( !pObject->bVisible && (!pObject->position.bParentOfInstance && !pObject->bAnimPlaying) )
				continue;

			// call per object update function
			UpdateAnimationCyclePerObject ( pObject );
		}
	}

	// okay
	return true;
}

void CObjectManager::UpdateOneVisibleObject ( sObject* pObject )
{
	// 090217 - must reset character limb zero offset to get true picture of base vs spine travel
	float fStoreObjectY = ObjectAngleY ( pObject->dwObjectNumber );
	if ( pObject->bUseSpineCenterSystem == true )
	{
		OffsetLimb ( pObject->dwObjectNumber, 0, 0, 0, 0 );
		RotateObject ( pObject->dwObjectNumber, 0, 0, 0 );
	}

	// calculate all frame/slerp/animation data
	GGMATRIX matrix;
	UpdateAllFrameData ( pObject, pObject->fAnimFrame );
	GGMatrixIdentity ( &matrix );
	if ( pObject->position.bCustomBoneMatrix==false ) UpdateFrame ( pObject->pFrame, &matrix );

	// 090217 - new feature allows character base vs spine to centralise model
	// and provide perfect footplanting deltas for movement of the object
	if ( pObject->bUseSpineCenterSystem == true )
	{
		int iSpine1 = pObject->dwSpineCenterLimbIndex;
		int iAnimObj = pObject->dwObjectNumber;
		float fBaseZ = ObjectPositionZ(iAnimObj);
		float fSpine1Z = LimbPositionZ(iAnimObj,iSpine1) - fBaseZ;

		// Instantly shift the 'travel' from true object center via offset (so character walks on spot)
		if ( pObject->bSpineTrackerMoving == false )
		{
			// some animations move forward/backward, some just stop
			pObject->fSpineCenterTravelDeltaX = 0.0f;
			fSpine1Z = 0.0f;
		}
		pObject->fSpineCenterTravelDeltaX += (fSpine1Z - pObject->fSpineCenterTravelDeltaZ); // delta is local to object rotation (forward/backward only)
		pObject->fSpineCenterTravelDeltaZ = fSpine1Z;
		OffsetLimb ( iAnimObj, 0, 0, 0, fSpine1Z );
		RotateObject ( iAnimObj, 0, fStoreObjectY, 0 );
		// And update frames again with new offset taken into account
		UpdateAllFrameData ( pObject, pObject->fAnimFrame );
		GGMatrixIdentity ( &matrix );
		if ( pObject->position.bCustomBoneMatrix==false ) UpdateFrame ( pObject->pFrame, &matrix );
	}

	// moved this code to DBOFormat.cpp - handle vertex level animation (even if not animating)
	// instances that use animating objects must animate them indirectly
	sObject* pActualObject = pObject;
	if ( pActualObject->pInstanceOfObject )
	{
		// animate actual object of the instance indirectly
		UpdateObjectCamDistance ( pActualObject );
	}
	else
	{
		// animate object directly
		if ( pObject->bVisible && pObject->bUniverseVisible )
		{
			UpdateObjectCamDistance ( pObject );
			UpdateObjectAnimation ( pObject );
		}
		else
		{
			// moved instance animation here as we only want to call it once
			if ( pObject->position.bParentOfInstance )
				UpdateObjectAnimation ( pObject );
		}
	}
}

bool CObjectManager::UpdateOnlyVisible ( void )
{
	// lee - 300914 - this may MISS some objects such as manually limb adjusted objects down the road!
	if ( !g_vAnimatableObjectList.empty() )
    {
        for ( DWORD iIndex = 0; iIndex < g_vAnimatableObjectList.size(); ++iIndex )
        {
			// get mesh to refresh
            sObject* pObject = g_vAnimatableObjectList [ iIndex ];
			if ( !pObject ) continue;

			// allow parents of instances
			if ( pObject->position.bParentOfInstance==false )
			{
				// only need to calc matrix data and update anim data if VISIBLE!
				if ( pObject->bVisible==false || pObject->bUniverseVisible==false ) //|| pObject->bExcludedEarly )
					continue;
			}

			// go and update object frames
			UpdateOneVisibleObject ( pObject );
		}
	}

	// okay
	return true;
}

//
// RENDERING
//

bool CObjectManager::SetVertexShader ( sMesh* pMesh )
{
	// set the vertex shader for a mesh - only change if the FVF is different

	// if VertDec different in any way
	bool bRefresh = false;
	if ( m_bUpdateVertexDecs==true )
	{
		m_bUpdateVertexDecs = false;
		bRefresh = true;
	}

	// check the mesh is okay
	SAFE_MEMORY ( pMesh );

	// regular or custom shader
	if ( pMesh->bUseVertexShader )
		m_dwCurrentShader = (DWORD)pMesh->pVertexShader;
	else
		m_dwCurrentShader = 0;

	// store the current FVF as regular
	m_dwCurrentFVF = pMesh->dwFVF;

	// is the shader different to the previously set shader
	if ( m_dwCurrentShader != m_dwLastShader || bRefresh==true )
	{
		// custom shader or Fixed-Function\FX-Effect 
		#ifdef DX11
		#else
		if ( pMesh->pVertexShader )
		{
			// set the new vertex shader
			if ( FAILED ( m_pD3D->SetVertexShader ( pMesh->pVertexShader ) ) )
				return false;
		}
		else
		{
			// set no vertex shader
			if ( FAILED ( m_pD3D->SetVertexShader ( NULL ) ) )
				return false;
		}
		#endif

		// store the current shader
		m_dwLastShader = m_dwCurrentShader;
	}

	// is the FVF different to the previously set FVF
	#ifdef DX11
	#else
	if ( (m_dwCurrentFVF != m_dwLastFVF) || m_dwCurrentFVF==0 || bRefresh==true )
	{
		// custom low level shader
		if ( pMesh->pVertexShader )
		{
			// vertex dec - usually from low level assembly shader
			if ( FAILED ( m_pD3D->SetVertexDeclaration ( pMesh->pVertexDec ) ) )
				return false;
		}
		else
		{
			// custom FVF or Regular
			if ( m_dwCurrentFVF==0 )
			{
				// custom vertex dec - usually from FX effect
				if ( FAILED ( m_pD3D->SetVertexDeclaration ( pMesh->pVertexDec ) ) )
					return false;

				// regular vertex FVF - standard usage
				if ( FAILED ( m_pD3D->SetFVF ( 0 ) ) )
					return false;
			}
			else
			{
				// regular vertex FVF - standard usage
				if ( FAILED ( m_pD3D->SetFVF ( m_dwCurrentFVF ) ) )
					return false;
			}
		}

		// store the current shader
		m_dwLastFVF = m_dwCurrentFVF;
	}
	#endif

	// return okay
	return true;
}

bool CObjectManager::SetInputStreams ( sMesh* pMesh )
{
	// set the input streams for drawing - only change if different

	// make sure the mesh is valid
	SAFE_MEMORY ( pMesh );

	// store a pointer to the current VB and IB
	m_ppCurrentVBRef = pMesh->pDrawBuffer->pVertexBufferRef;
	m_ppCurrentIBRef = pMesh->pDrawBuffer->pIndexBufferRef;

	// see the difference flag to false
	bool bDifferent = false;
	if ( m_ppCurrentVBRef != m_ppLastVBRef )
		bDifferent = true;

	// when a new frame starts we need to reset the streams
	if ( m_bUpdateStreams ) bDifferent = true;

	// update VB only when necessary
	if ( bDifferent )
	{
		// store the current VB
		m_ppLastVBRef = m_ppCurrentVBRef;

		// set the stream source
		#ifdef DX11
		unsigned int stride;
		unsigned int offset;
		stride = pMesh->pDrawBuffer->dwFVFSize;
		offset = pMesh->pDrawBuffer->dwVertexStart;
		m_pImmediateContext->IASetVertexBuffers ( 0, 1, &pMesh->pDrawBuffer->pVertexBufferRef, &stride, &offset);
		#else
		if ( FAILED ( m_pD3D->SetStreamSource ( 0,
												pMesh->pDrawBuffer->pVertexBufferRef,
												0, 
												pMesh->pDrawBuffer->dwFVFSize				 ) ) )
			return false;
		#endif
	}

	// see the difference flag to false
	bDifferent = false;
	if ( m_ppCurrentIBRef != m_ppLastIBRef )
		bDifferent = true;

	// when a new frame starts we need to reset the streams
	if ( m_bUpdateStreams ) bDifferent = true;

	// update VB only when necessary
	if ( bDifferent )
	{
		// store the current VB
		m_ppLastIBRef = m_ppCurrentIBRef;

		// set the indices (if any)
		#ifdef DX11
		if ( m_ppCurrentIBRef )
			m_pImmediateContext->IASetIndexBuffer ( pMesh->pDrawBuffer->pIndexBufferRef, DXGI_FORMAT_R16_UINT, 0);
		#else
		if ( m_ppCurrentIBRef )
			if ( FAILED ( m_pD3D->SetIndices ( pMesh->pDrawBuffer->pIndexBufferRef ) ) )//, pMesh->pDrawBuffer->dwBaseVertexIndex ) ) )
				return false;
		#endif
	}

	// update refresh used (resets at start of cycle)
	m_bUpdateStreams = false;

	return true;
}

bool CObjectManager::PreSceneSettings ( void )
{
	// cullmode
	m_RenderStates.dwCullDirection				= m_RenderStates.dwGlobalCullDirection;
	m_RenderStates.bCull						= true;
	m_RenderStates.iCullMode					= 0;	
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_CULLMODE,	m_RenderStates.dwCullDirection );
	#endif

	// allow anistropic filtering to look better when used
	#ifdef DX11
	#else
	if ( g_iAnisotropyLevel==-1 )
	{
		// best card can give
		GGCAPS pCaps;
		m_pD3D->GetDeviceCaps(&pCaps);
		g_iAnisotropyLevel = pCaps.MaxAnisotropy;
	}
	for ( int texturestage=0; texturestage<8; texturestage++)
		m_pD3D->SetSamplerState ( texturestage, D3DSAMP_MAXANISOTROPY, g_iAnisotropyLevel );
	#endif

	// okay
	return true;
}

bool CObjectManager::PreDrawSettings ( void )
{
	#ifdef DX11
	m_RenderStates.bZWrite = true;
	m_pImmediateContext->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	#else
	// obtain external render states not tracked by manager
	m_pD3D->GetRenderState ( D3DRS_AMBIENT, &m_RenderStates.dwAmbientColor );
	m_pD3D->GetRenderState ( D3DRS_FOGCOLOR, &m_RenderStates.dwFogColor );

	// 041013 - set multisampling on for everything (if activated in SET DISPLAY MODE)
	m_pD3D->SetRenderState ( D3DRS_MULTISAMPLEANTIALIAS, FALSE); // 270316 - artifacts in terrain TRUE );

	// wireframe
	m_pD3D->SetRenderState ( D3DRS_FILLMODE, D3DFILL_SOLID );
	m_RenderStates.bWireframe							= false;

	// lighting
	m_pD3D->SetRenderState ( D3DRS_LIGHTING, TRUE );
	m_RenderStates.bLight								= true;

	// fog override starts off disabled
	m_RenderStates.bFogOverride=false;

	// fogenable
	m_pD3D->SetRenderState ( D3DRS_FOGENABLE, FALSE );
	m_RenderStates.bFog									= false;

	// ambient
	m_pD3D->SetRenderState ( D3DRS_AMBIENT, m_RenderStates.dwAmbientColor );
	m_RenderStates.iAmbient								= 1;

	// transparency
	m_pD3D->SetRenderState ( D3DRS_ALPHATESTENABLE,		false );
	m_pD3D->SetRenderState ( D3DRS_ALPHAFUNC,			D3DCMP_ALWAYS );
	m_pD3D->SetRenderState ( D3DRS_DEPTHBIAS,			0 );
	m_RenderStates.bTransparency						= false;
	m_RenderStates.dwAlphaTestValue						= 0;

	// ghost
	m_pD3D->SetRenderState ( D3DRS_ZENABLE,				TRUE );
	m_pD3D->SetRenderState ( D3DRS_ZWRITEENABLE,		TRUE );
	m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE,	FALSE );
	m_RenderStates.bZRead								= true;
	m_RenderStates.bZWrite								= true;
	m_RenderStates.bGhost								= false;
	
	// zbias handling
	m_pD3D->SetRenderState ( D3DRS_DEPTHBIAS,			0 );
	m_pD3D->SetRenderState ( D3DRS_SLOPESCALEDEPTHBIAS,	0 );
	m_RenderStates.bZBiasActive							= false;
	m_RenderStates.fZBiasSlopeScale						= 0.0f;
	m_RenderStates.fZBiasDepth							= 0.0f;

	// default material render states
	m_pD3D->SetRenderState ( D3DRS_COLORVERTEX,					TRUE );
	m_pD3D->SetRenderState ( D3DRS_DIFFUSEMATERIALSOURCE,		D3DMCS_COLOR1 );
	m_pD3D->SetRenderState ( D3DRS_SPECULARMATERIALSOURCE,		D3DMCS_MATERIAL );
	m_pD3D->SetRenderState ( D3DRS_AMBIENTMATERIALSOURCE,		D3DMCS_MATERIAL );
	m_pD3D->SetRenderState ( D3DRS_EMISSIVEMATERIALSOURCE,		D3DMCS_MATERIAL );
	m_pD3D->SetRenderState ( D3DRS_SPECULARENABLE,				TRUE );

	// white default material 'set during init'
	m_RenderStates.gWhiteDefaultMaterial.Diffuse.r		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Diffuse.g		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Diffuse.b		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Diffuse.a		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Ambient.r		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Ambient.g		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Ambient.b		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Ambient.a		= 1.0f;
	m_RenderStates.gWhiteDefaultMaterial.Specular.r		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Specular.g		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Specular.b		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Specular.a		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Emissive.r		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Emissive.g		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Emissive.b		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Emissive.a		= 0.0f;
	m_RenderStates.gWhiteDefaultMaterial.Power			= 10.0f;

	// set default white material (for diffuse, ambience, etc)
	if ( FAILED ( m_pD3D->SetMaterial ( &m_RenderStates.gWhiteDefaultMaterial ) ) )
		return false;

	// fixed function blending stage defaults
	DWORD dwMaxTextureStage = MAXTEXTURECOUNT;
	for ( DWORD dwTextureStage = 0; dwTextureStage < dwMaxTextureStage; dwTextureStage++ )
	{
		// leefix - 180204 - set defaults at start of render phase
		m_RenderStates.dwAddressU[dwTextureStage] = D3DTADDRESS_WRAP;
		m_RenderStates.dwAddressV[dwTextureStage] = D3DTADDRESS_WRAP;
		m_RenderStates.dwMagState[dwTextureStage] = GGTEXF_LINEAR;
		m_RenderStates.dwMinState[dwTextureStage] = GGTEXF_LINEAR;
		m_RenderStates.dwMipState[dwTextureStage] = GGTEXF_LINEAR;

		// texture filter modes
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSU, m_RenderStates.dwAddressU[dwTextureStage]==0 ? D3DTADDRESS_WRAP : m_RenderStates.dwAddressU[dwTextureStage] );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSV, m_RenderStates.dwAddressV[dwTextureStage]==0 ? D3DTADDRESS_WRAP : m_RenderStates.dwAddressV[dwTextureStage] );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MAGFILTER, m_RenderStates.dwMagState[dwTextureStage]==0 ? GGTEXF_LINEAR : m_RenderStates.dwMagState[dwTextureStage] );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MINFILTER, m_RenderStates.dwMinState[dwTextureStage]==0 ? GGTEXF_LINEAR : m_RenderStates.dwMinState[dwTextureStage] );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MIPFILTER, m_RenderStates.dwMipState[dwTextureStage]==0 ? GGTEXF_LINEAR : m_RenderStates.dwMipState[dwTextureStage] );

		// texture blending modes
		if(dwTextureStage==0)
		{
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, GGTOP_MODULATE );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG1, GGTA_TEXTURE );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG2, GGTA_DIFFUSE );
		}
		else
		{
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, GGTOP_DISABLE );
		}

		// texture coordinate data
		m_pD3D->SetTextureStageState( dwTextureStage, D3DTSS_TEXCOORDINDEX, dwTextureStage );
		m_pD3D->SetTextureStageState( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
	}
	#endif

	// Set default FOV from camera (zero does not change camera FOV!)
	if ( g_pGlob->dwRenderCameraID == 0 )
	{
		if ( m_RenderStates.fObjectFOV != 0.0f )
		{
			// sometimes, objectfov renderstate is not reset, and needed before we start again
			SetCameraFOV ( m_RenderStates.fStoreCameraFOV );
			m_RenderStates.fObjectFOV = 0.0f;
		}
		tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData( 0 );
		m_RenderStates.fStoreCameraFOV = m_Camera_Ptr->fFOV;
		m_RenderStates.fObjectFOV = 0.0f;
		SetCameraFOV ( m_RenderStates.fStoreCameraFOV );
	}

	// success
	return true;
}

bool CObjectManager::SetMeshMaterial ( sMesh* pMesh, D3DMATERIAL9* pMaterial )
{
	#ifdef DX11
	#else
	if ( pMesh->bUsesMaterial )
	{
		// use diffuse from material (if present)
		m_pD3D->SetRenderState ( D3DRS_COLORVERTEX,					FALSE );
		m_pD3D->SetRenderState ( D3DRS_DIFFUSEMATERIALSOURCE,		D3DMCS_MATERIAL );

		// set the material from the mesh
		if ( FAILED ( m_pD3D->SetMaterial ( pMaterial ) ) )
			return false;
	}
	else
	{
		// use diffuse from mesh vertex (if any)
		m_pD3D->SetRenderState ( D3DRS_COLORVERTEX,					TRUE );
		m_pD3D->SetRenderState ( D3DRS_DIFFUSEMATERIALSOURCE,		D3DMCS_COLOR1 );

		// set no material
		if ( FAILED ( m_pD3D->SetMaterial ( &m_RenderStates.gWhiteDefaultMaterial ) ) )
			return false;
	}
	#endif

	// success
	return true;
}

void CObjectManager::SetMeshDepthStates( sMesh* pMesh, bool bForceState )
{
	if ( pMesh->bZWrite != m_RenderStates.bZWrite || pMesh->bZRead != m_RenderStates.bZRead || bForceState == true )
	{
		ID3D11DepthStencilState* pDepthStencilState = NULL;
		if ( pMesh->bZRead == true )
		{
			if ( pMesh->bZWrite )
				pDepthStencilState = m_pDepthStencilState;
			else
				pDepthStencilState = m_pDepthNoWriteStencilState;
		}
		else
		{
			pDepthStencilState = m_pDepthDisabledStencilState;
		}
		m_RenderStates.bZWrite = pMesh->bZWrite;
		m_RenderStates.bZRead = pMesh->bZRead;
		m_pImmediateContext->OMSetDepthStencilState( pDepthStencilState, 0 );
	}
}

bool CObjectManager::SetMeshRenderStates( sMesh* pMesh )
{
	#ifdef DX11

	// standard depth state
	SetMeshDepthStates ( pMesh, false );

	// blend state
	if ( pMesh->bShadowBlend )
		m_pImmediateContext->OMSetBlendState(m_pBlendStateShadowBlend, 0, 0xffffffff);
	else
		m_pImmediateContext->OMSetBlendState(m_pBlendStateAlpha, 0, 0xffffffff);

	// render state
	if ( pMesh->bCull == false )
	{
		m_pImmediateContext->RSSetState(m_pRasterStateNoCull);
	}
	else
	{
		if ( pMesh->bZBiasActive == true )
		{
			m_pImmediateContext->RSSetState(m_pRasterStateDepthBias);
		}
		else
		{
			m_pImmediateContext->RSSetState(m_pRasterState);
		}
	}
	#else
	// wireframe
	if ( pMesh->bWireframe != m_RenderStates.bWireframe )
	{
		if ( pMesh->bWireframe )
			m_pD3D->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		else
			m_pD3D->SetRenderState ( D3DRS_FILLMODE, D3DFILL_SOLID );

		m_RenderStates.bWireframe = pMesh->bWireframe;
	}

	// lighting
	if ( pMesh->bLight != m_RenderStates.bLight )
	{
		if ( pMesh->bLight )
			m_pD3D->SetRenderState ( D3DRS_LIGHTING, TRUE );
		else
			m_pD3D->SetRenderState ( D3DRS_LIGHTING, FALSE );

		m_RenderStates.bLight = pMesh->bLight;
	}

	// cullmode
	if ( pMesh->bCull != m_RenderStates.bCull || pMesh->iCullMode != m_RenderStates.iCullMode )
	{
		// lee - 040306 -u6rc5 - cull mode (direction override)
		if ( pMesh->iCullMode==2 )
		{
			m_pD3D->SetRenderState ( D3DRS_CULLMODE, GGCULL_CCW );
		}
		else
		{
			// lee - 121006 - u63 - cull mode CW (for manual reflection cull toggle)
			if ( pMesh->iCullMode==3 )
			{
				m_pD3D->SetRenderState ( D3DRS_CULLMODE, D3DCULL_CW );
			}
			else
			{
				// on/off
				if ( pMesh->bCull )
					m_pD3D->SetRenderState ( D3DRS_CULLMODE, m_RenderStates.dwCullDirection );
				else	
					m_pD3D->SetRenderState ( D3DRS_CULLMODE, D3DCULL_NONE );
			}
		}
		m_RenderStates.bCull = pMesh->bCull;
		m_RenderStates.iCullMode = pMesh->iCullMode;
	}

	// fog system (from light DLL)
	if(g_pGlob)
	{
		if(g_pGlob->iFogState==1)
		{
			// fogenable
			if ( pMesh->bFog != m_RenderStates.bFog )
			{
				if ( pMesh->bFog )
					m_pD3D->SetRenderState ( D3DRS_FOGENABLE, TRUE );
				else	
					m_pD3D->SetRenderState ( D3DRS_FOGENABLE, FALSE );
	
				m_RenderStates.bFog = pMesh->bFog;
			}

			// ghosts in fog must override fog color part (fog override)
			if ( pMesh->bFog && pMesh->bGhost )
			{
				if ( m_RenderStates.bFogOverride==false )
				{
					m_pD3D->SetRenderState ( D3DRS_FOGCOLOR, GGCOLOR_RGBA ( 0, 0, 0, 0 ) );
					m_RenderStates.bFogOverride=true;
				}
			}
			else
			{
				m_pD3D->SetRenderState ( D3DRS_FOGCOLOR, m_RenderStates.dwFogColor );
				m_RenderStates.bFogOverride=false;
			}
		}
	}

	// ambient - leefix - 230604 - u54 - no ambience can now be 255 or 0 (so need to do code in those cases)
	if ( pMesh->bAmbient==false && m_RenderStates.iAmbient==1
	||	 m_RenderStates.iAmbient==0 || m_RenderStates.iAmbient==2 )
	{
		if ( pMesh->bAmbient )
		{
			m_pD3D->SetRenderState ( D3DRS_AMBIENT, m_RenderStates.dwAmbientColor );
			m_RenderStates.iAmbient = 1;
		}
		else
		{
			// leefix - 210303 - diffuse colour must be maintained over any ambience
			bool bWhite=false;
			if ( pMesh->pTextures )
				if ( pMesh->pTextures[0].iImageID!=0 )
					bWhite=true;

			// leefix - 210303 - white used for no ambient on a texture
			if ( bWhite )
			{
				m_pD3D->SetRenderState ( D3DRS_AMBIENT, GGCOLOR_ARGB(255,255,255,255) );
				m_RenderStates.iAmbient = 2;
			}
			else
			{
				m_pD3D->SetRenderState ( D3DRS_AMBIENT, GGCOLOR_ARGB(0,0,0,0) );
				m_RenderStates.iAmbient = 0;
			}
		}
	}

	// leefix - 070204 - introduced for better Zwrite control
	bool bCorrectZWriteState = pMesh->bZWrite;

	// transparency (leefix - 190303 - added second condition where transparency is reimposed after a ghosted object)
	bool bDoGhostAgain = false;
	if ( pMesh->bTransparency != m_RenderStates.bTransparency
	||	 pMesh->dwAlphaTestValue != m_RenderStates.dwAlphaTestValue
	|| ( pMesh->bTransparency==true && m_RenderStates.bGhost==true	) )
	{
		if ( pMesh->bTransparency )
		{
			m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE,	true );
			m_pD3D->SetRenderState ( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA );
			m_pD3D->SetRenderState ( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA );
			m_pD3D->SetRenderState ( D3DRS_ALPHATESTENABLE,		true );
			
			// mike - 020904 - use this for alpha testing - do not get edges anymore
			// lee - 240903 - need full range of alpha rendered, not just the upper band
			DWORD dwuseAlphaTestValue = pMesh->dwAlphaTestValue;

			if ( dwuseAlphaTestValue==0 )
			{
				m_pD3D->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATER );
				m_pD3D->SetRenderState ( D3DRS_ALPHAREF,	(DWORD)0x00000000 );
			}
			else
			{
				// leeadd - 131205 - let SetAlphaMappingOn command scale the alpha-test to let semi-transparent pixel through
				if ( pMesh->bAlphaOverride==true )
				{
					// alpha mapping percentage vased alpha test
					DWORD dwPercAlpha = ( (pMesh->dwAlphaOverride & 0xFF000000) >> 24 ) ;
					float perc = (float)dwPercAlpha / 255.0f;
					// alpha test transition not perfect as go from override to 0xCF based alpha, so cap it
					DWORD dwAlphaLevelToDraw = (DWORD)(255 * perc);
					dwuseAlphaTestValue = dwAlphaLevelToDraw;
					if ( dwuseAlphaTestValue > (DWORD)0x000000CF ) dwuseAlphaTestValue=(DWORD)0x000000CF;
					m_pD3D->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATEREQUAL );
				}
				else
				{
					// regular alpha test
					dwuseAlphaTestValue=(DWORD)0x000000CF;
					m_pD3D->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATEREQUAL );
				}
				m_pD3D->SetRenderState ( D3DRS_ALPHAREF,	dwuseAlphaTestValue );
			}
			m_RenderStates.dwAlphaTestValue = dwuseAlphaTestValue;
		}
		else
		{
			m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE,	false );
			m_pD3D->SetRenderState ( D3DRS_ALPHATESTENABLE,		false );
			m_pD3D->SetRenderState ( D3DRS_ALPHAFUNC,			D3DCMP_ALWAYS );
		}
		m_RenderStates.bTransparency = pMesh->bTransparency;

		// now must do ghost again - to combine with blend settings
		bDoGhostAgain = true;
	}

	// ghost
	if ( bDoGhostAgain==true
	||	pMesh->bGhost != m_RenderStates.bGhost
	||  pMesh->iGhostMode != m_RenderStates.iGhostMode )
	{
		if ( pMesh->bGhost )
		{
			m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, true );
			switch ( pMesh->iGhostMode )
			{
				case 0:
				{
					m_pD3D->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
					m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR );
				}
				break;

				case 1:
				{
					// lee - 220306 - u6b4 - direct from best of DBC (darkghostmode7)
					DWORD dwDarkAlphaSourceBlend = D3DBLEND_ZERO;
					DWORD dwDarkAlphaDestinationBlend = D3DBLEND_SRCCOLOR;
					m_pD3D->SetRenderState( D3DRS_SRCBLEND,  dwDarkAlphaSourceBlend );
					m_pD3D->SetRenderState( D3DRS_DESTBLEND, dwDarkAlphaDestinationBlend );
				}
				break;

				case 2:
				{
					m_pD3D->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
					m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
				}
				break;

				case 3:
				{
					m_pD3D->SetRenderState ( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
					m_pD3D->SetRenderState ( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
				}
				break;

				case 4:
				{
					m_pD3D->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
					m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR );
				}
				break;

				case 5:
				{
					// leeadd - 210806 - replace OLD-MODE-1 (used in FPSC) for Scorch Texture Multiply
					m_pD3D->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR );
					m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
				}
				break;
			}
			bCorrectZWriteState = false;
		}
		else
		{
			// no ghost and no transparency, end alpha blend effect
			if ( pMesh->bTransparency==false)
			{
				m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, FALSE );
			}
		}

		m_RenderStates.bGhost = pMesh->bGhost;
		m_RenderStates.iGhostMode = pMesh->iGhostMode;
	}

	// leefix - 070204 - simplified - set zwrite state
	if ( pMesh->bZWrite != m_RenderStates.bZWrite )
	{
		if ( pMesh->bZWrite )
			m_pD3D->SetRenderState ( D3DRS_ZWRITEENABLE,		TRUE );
		else
			m_pD3D->SetRenderState ( D3DRS_ZWRITEENABLE,		FALSE );

		m_RenderStates.bZWrite = pMesh->bZWrite;
	}

	// leeadd - 080604 - ZBIAS handling - always set unless not active, then unset once
	if ( pMesh->bZBiasActive )
	{
		m_pD3D->SetRenderState ( D3DRS_DEPTHBIAS,			
			pMesh->fZBiasDepth );
		m_pD3D->SetRenderState ( D3DRS_SLOPESCALEDEPTHBIAS,	*(DWORD*)&pMesh->fZBiasSlopeScale );
		m_RenderStates.bZBiasActive = true;
	}
	else
	{
		if ( m_RenderStates.bZBiasActive )
		{
			m_pD3D->SetRenderState ( D3DRS_DEPTHBIAS,			0 );
			m_pD3D->SetRenderState ( D3DRS_SLOPESCALEDEPTHBIAS,	0 );
			m_RenderStates.bZBiasActive = false;
		}
	}

	// set zread state
	if ( pMesh->bZRead != m_RenderStates.bZRead )
	{
		if ( pMesh->bZRead )
			m_pD3D->SetRenderState ( D3DRS_ZENABLE,		TRUE );
		else
			m_pD3D->SetRenderState ( D3DRS_ZENABLE,		FALSE );

		m_RenderStates.bZRead = pMesh->bZRead;
	}

	// set the new material and render state
	SetMeshMaterial ( pMesh, &pMesh->mMaterial );

	// need to be able to set mip map LOD bias on a per mesh basis
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPMAPLODBIAS, *( ( LPDWORD ) ( &pMesh->fMipMapLODBias ) ) );
	#endif

	// success
	return true;
}

bool CObjectManager::SetMeshTextureStates ( sMesh* pMesh )
{
	#ifdef DX11
	#else
	// close off any stages from previous runs
	if ( m_dwLastTextureCount > pMesh->dwTextureCount )
	{
		DWORD dwTexCountMax = m_dwLastTextureCount;
		if ( dwTexCountMax > 7 ) dwTexCountMax = 7;
		for ( DWORD dwTextureStage = pMesh->dwTextureCount; dwTextureStage < dwTexCountMax; dwTextureStage++ )
		{
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, GGTOP_DISABLE );
		}
	}

	// texture filtering and blending
	DWORD dwTextureCountMax = pMesh->dwTextureCount;
	if ( dwTextureCountMax>=MAXTEXTURECOUNT ) dwTextureCountMax=MAXTEXTURECOUNT;
	for ( DWORD dwTextureIndex = 0; dwTextureIndex < pMesh->dwTextureCount; dwTextureIndex++ )
	{
		// Determine texture stage to write to
		DWORD dwTextureStage = pMesh->pTextures [ dwTextureIndex ].dwStage;

		// Determine texture data ptr
		sTexture* pTexture = &pMesh->pTextures [ dwTextureIndex ];

		// texture wrap and filter modes
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSU, pTexture->dwAddressU==0 ? D3DTADDRESS_WRAP : pTexture->dwAddressU );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSV, pTexture->dwAddressV==0 ? D3DTADDRESS_WRAP : pTexture->dwAddressV );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MAGFILTER, pTexture->dwMagState==0 ? GGTEXF_LINEAR : pTexture->dwMagState );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MINFILTER, pTexture->dwMinState==0 ? GGTEXF_LINEAR : pTexture->dwMinState );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MIPFILTER, pTexture->dwMipState==0 ? GGTEXF_LINEAR : pTexture->dwMipState );

		// texture blending modes
		if ( pMesh->bOverridePixelShader )
		{
			// use custom pixel shader to replace blending stages
			m_pD3D->SetPixelShader ( pMesh->pPixelShader );
		}
		else
		{
			// fixed function does not use pixel shaders
			m_pD3D->SetPixelShader ( 0 );

			// fixed function blending (leefix-210703-fixed now at source)
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, pTexture->dwBlendMode );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG1, pTexture->dwBlendArg1 );

			// lee - 240206 - u60 - will use TFACTOR (diffuse replacement) instead of regular DIFFUSE
			if ( dwTextureStage==0 && m_RenderStates.bIgnoreDiffuse==true )
			{
				// TFACTOR set in previous call from the INSTANCE drawmesh call (instance diffuse changes)
				m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG2, D3DTA_TFACTOR );
			}
			else
				m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG2, pTexture->dwBlendArg2 );

			// U73 - 210309 - apply extra ARG values
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG0, pTexture->dwBlendArg0 );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAARG0, pTexture->dwBlendArg0 );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_RESULTARG, pTexture->dwBlendArgR );

			// last texture stage can override alpha with tfactor
			if ( m_RenderStates.bNoMeshAlphaFactor==false )
			{
				if ( pMesh->bAlphaOverride==true )
				{
					if ( dwTextureStage==pMesh->dwTextureCount-1 )
					{
						// instance overrides alpha value using TFACTOR
						m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAOP, pTexture->dwBlendMode );
						m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAARG2, pTexture->dwBlendArg1 );
						m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
						m_pD3D->SetRenderState( D3DRS_TEXTUREFACTOR, pMesh->dwAlphaOverride );
					}
				}
				else
				{
					// regular alpha operations
					m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAOP, pTexture->dwBlendMode );
					m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAARG1, pTexture->dwBlendArg1 );
					m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_ALPHAARG2, pTexture->dwBlendArg2 );
				}
			}

			// texture coordinate data
			switch ( pTexture->dwTexCoordMode )
			{
				case 0 :{	// Regular UV Stage Match
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, dwTextureStage );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
						}
						break;

				/*
				case 1 :{	// Sphere Mapping
							GGMATRIX mat;
							mat._11 = 0.5f; mat._12 = 0.0f; mat._13 = 0.0f; mat._14 = 0.0f; 
							mat._21 = 0.0f; mat._22 =-0.5f; mat._23 = 0.0f; mat._24 = 0.0f; 
							mat._31 = 0.0f; mat._32 = 0.0f; mat._33 = 1.0f; mat._34 = 0.0f; 
							mat._41 = 0.5f; mat._42 = 0.5f; mat._43 = 0.0f; mat._44 = 1.0f; 
							D3DTRANSFORMSTATETYPE dwTexTS = D3DTS_TEXTURE1;
							if( dwTextureStage==2 ) dwTexTS=D3DTS_TEXTURE2;
							GGSetTransform ( dwTexTS, &mat );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
						}
						break;

				case 2 :{	// Cube Mapping (leefix - 190303 - works now as camera moves around)
							GGMATRIX mat;
							GGMATRIX matview;
							GGGetTransform ( GGTS_VIEW, &matview );
							GGMatrixInverse ( &mat, NULL, &matview );
							mat._41 = 0.0f; mat._42 = 0.0f; mat._43 = 0.0f; mat._44 = 1.0f; 
							D3DTRANSFORMSTATETYPE dwTexTS = D3DTS_TEXTURE0;
							if( dwTextureStage==1 ) dwTexTS=D3DTS_TEXTURE1;
							if( dwTextureStage==2 ) dwTexTS=D3DTS_TEXTURE2;
							GGSetTransform ( dwTexTS, &mat );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 );
						}
						break;
						*/

				case 3 :{	// Steal UV Stage From Zero
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, 0 );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
						}
						break;

				case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
						{	// Set alternate texture bank for UV data
							int iGetFrom = pTexture->dwTexCoordMode-10;
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, iGetFrom );
							m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXTURETRANSFORMFLAGS, 0 );
						}
						break;
			}
		}
	}
	#endif

	// store number of textures handled for this mesh
	m_dwLastTextureCount = pMesh->dwTextureCount;

	// okay
	return true;
}

bool CObjectManager::ShaderStart ( sMesh* pMesh, LPGGRENDERTARGETVIEW* ppCurrentRenderTarget, LPGGDEPTHSTENCILVIEW* ppCurrentDepthTarget, UINT* puPasses, bool* pbEffectRendering, bool* pbLocalOverrideAllTexturesAndEffects )
{
	// set the vertex shader
	if ( !SetVertexShader ( pMesh ) )
		return false;

	// can switch off fore color wipe if object flags it
	*pbLocalOverrideAllTexturesAndEffects = m_RenderStates.bOverrideAllTexturesAndEffects;
	if ( pMesh->pDrawBuffer )
		if ( pMesh->pDrawBuffer->dwImmuneToForeColorWipe==1 )
			*pbLocalOverrideAllTexturesAndEffects = false;

	// set effect shader
	if ( pMesh->pVertexShaderEffect && *pbLocalOverrideAllTexturesAndEffects==false )
	{
		// use an effect
		GGMATRIX matWorld;
		GGGetTransform ( GGTS_WORLD, &matWorld );
		*puPasses = pMesh->pVertexShaderEffect->Start ( pMesh, matWorld );

		// if FX effect, flag effect code
		if ( pMesh->pVertexShaderEffect->m_pEffect )
		{
			// effect shall be used
			*pbEffectRendering=true;

			// set states prior to shader begin pass
			#ifdef DX11
			#else
			if ( pMesh->pTextures )
			{
				for ( DWORD dwTextureIndex = 0; dwTextureIndex < pMesh->dwTextureCount; dwTextureIndex++ )
				{
					DWORD dwTextureStage = pMesh->pTextures [ dwTextureIndex ].dwStage;
					if ( dwTextureStage < 16 )
					{
						// so the object states to do overwrite any shader states that begin pass will set
						sTexture* pTexture = &pMesh->pTextures [ dwTextureIndex ];
						if ( pTexture )
						{
							m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSU, pTexture->dwAddressU==0 ? D3DTADDRESS_WRAP : pTexture->dwAddressU );
							m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_ADDRESSV, pTexture->dwAddressV==0 ? D3DTADDRESS_WRAP : pTexture->dwAddressV );
							m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MAGFILTER, pTexture->dwMagState==0 ? GGTEXF_LINEAR : pTexture->dwMagState );
							m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MINFILTER, pTexture->dwMinState==0 ? GGTEXF_LINEAR : pTexture->dwMinState );
							m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MIPFILTER, pTexture->dwMipState==0 ? GGTEXF_LINEAR : pTexture->dwMipState );
						}
					}
				}
			}
			#endif
		}
	}

	// if using RT, store current render target
	if ( pMesh->pVertexShaderEffect )
	{
		if ( pMesh->pVertexShaderEffect->m_bUsesAtLeastOneRT==true )
		{
			#ifdef DX11
			*ppCurrentRenderTarget = g_pGlob->pCurrentRenderView;
			*ppCurrentDepthTarget = g_pGlob->pCurrentDepthView;
			#else
			m_pD3D->GetRenderTarget( 0, ppCurrentRenderTarget );
			m_pD3D->GetDepthStencilSurface( ppCurrentDepthTarget );
			#endif
		}
	}

	// if rendering with an effect
	#ifdef DX11
	#else
	if ( *pbEffectRendering )
	{
		// FF affects HLSL pipeline (and vice versa), so switch off
		// the automated clipping plane (FF will stop clipping for HLSLs)
		if ( m_RenderStates.bOverriddenClipPlaneforHLSL==false )
		{
			m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
			m_RenderStates.bOverriddenClipPlaneforHLSL = true;
		}
	}
	#endif

	// continue
	return true;
}

DWORD g_InstanceAlphaControlValue = 0;

bool CObjectManager::ShaderPass ( sMesh* pMesh, UINT uPass, UINT uPasses, bool bEffectRendering, bool bLocalOverrideAllTexturesAndEffects, LPGGRENDERTARGETVIEW pCurrentRenderTarget, LPGGDEPTHSTENCILVIEW pCurrentDepthTarget, sObject* pObject )
{
	// return true else if something like [depth render is skipped]
	bool bResult = true;

	// override every texture and effect with single color
	// useful for advanced post-processing effects like depthoffield/heathaze
	if ( bLocalOverrideAllTexturesAndEffects==true )
	{
		// no texture, no effect, just plane color
		#ifdef DX11
		#else
		m_pD3D->SetTexture ( 0, NULL );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP, GGTOP_SELECTARG1 );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP,   GGTOP_DISABLE );
		for ( int t=1; t<=7; t++ )
		{
			m_pD3D->SetTexture ( t, NULL );
			m_pD3D->SetTextureStageState ( t, D3DTSS_COLOROP, GGTOP_DISABLE );
			m_pD3D->SetTextureStageState ( t, D3DTSS_ALPHAOP,   GGTOP_DISABLE );
		}
		m_pD3D->SetRenderState ( D3DRS_TEXTUREFACTOR,	m_RenderStates.dwOverrideAllWithColor );
		m_pD3D->SetRenderState ( D3DRS_FOGCOLOR,		m_RenderStates.dwFogColor );
		#endif
	}
	else
	{


		// if using RT, determine if should switch to RT or final render target (current)
		if ( bEffectRendering )
		{
#ifdef DX11
			//PE: Only shader Apply needed.
			if (g_pGlob->dwRenderCameraID >= 31) {
				ID3DX11EffectTechnique* pTech = pMesh->pVertexShaderEffect->m_hCurrentTechnique;//m_pEffect->GetTechniqueByIndex(0);
				ID3DX11EffectPass* pPass = pTech->GetPassByIndex(uPass);
				pPass->Apply(0, m_pImmediateContext);
				return true;
			}
#endif
			// commit var
			LPGGEFFECT pEffect = pMesh->pVertexShaderEffect->m_pEffect;
			bool bMustCommit = false;

			// only if RT flagged (saves performance)
			if ( pMesh->pVertexShaderEffect->m_bUsesAtLeastOneRT==true )
			{
				#ifdef DX11
				// ensure technique exists
				ID3DX11EffectTechnique* hTech = pMesh->pVertexShaderEffect->m_hCurrentTechnique;//pEffect->GetTechniqueByIndex(0);

				// get rendercolortarget string from this pass
				ID3DX11EffectPass* hPass = hTech->GetPassByIndex(uPass);
				GGHANDLE hRT = hPass->GetAnnotationByName( "RenderColorTarget" );
				const char* szRT = 0;
				D3DX11_EFFECT_VARIABLE_DESC varDesc;
				if ( hRT && hRT->IsValid() ) 
				{
					hRT->AsString()->GetString(&szRT);
				}

				// if detect, use special depth texture render target
				/*if ( szRT && strnicmp( szRT, "[depthtexture]", strlen("[depthtexture]") )==NULL ) 
				{
					// NOTE: Optimize here, should not be using strings/compares
					if ( g_pGlob->dwRenderCameraID==0 )
					{
						// only render depths from camera zero
						int iSuccess = SwitchRenderTargetToDepth(0);
					}
					else
					{
						// actually SKIP this pass if any other camera
						return false;
					}
				}
				else
				{*/
					// check if RT string has contents
					if ( hRT && strcmp( szRT, "" ) != 0 )
					{
						// yes, now get handle to texture param we want to re-direct our render to
						GGHANDLE hTexParamWant = pEffect->GetVariableByName( szRT );

						// go through all textures recorded during the shader parse
						for ( DWORD t = 0; t < pMesh->pVertexShaderEffect->m_dwTextureCount; t++ )
						{
							// only consider RT textures that are flagged in bitmask
							DWORD dwThisTextureBit = 1 << t;
							if ( pMesh->pVertexShaderEffect->m_dwCreatedRTTextureMask & dwThisTextureBit )
							{
								// get the param handle of each texture in shader
								int iParam = 0;
								GGHANDLE hTexParam = NULL;
								if ( t<=31 ) 
								{
									iParam = pMesh->pVertexShaderEffect->m_iParamOfTexture [ t ];
									hTexParam = pEffect->GetVariableByIndex( iParam );
								}

								// if it matches the one we want
								if ( hTexParam == hTexParamWant )
								{
									// get texture ptr from effect
									LPGGSHADERRESOURCEVIEW pRTTexView = NULL;
									hTexParam->AsShaderResource()->GetResource(&pRTTexView);
									IGGTexture* pRTTex = NULL;
									pRTTexView->GetResource(&pRTTex);

									// switch render target to internal shader RT
									ID3D11RenderTargetView* pRTTexRenderView = pMesh->pVertexShaderEffect->m_pParamOfTextureRenderView[t];
									SetRenderAndDepthTarget ( pRTTexRenderView, nullptr );

									// for renders to shader off-screen RT textures, disable depth buffer completely
									m_pImmediateContext->OMSetDepthStencilState( m_pDepthDisabledStencilState, 0 );

									// put RT textures width and height into ViewSize
									GGHANDLE hViewSize = pEffect->GetVariableBySemantic( "ViewSize" );
									if ( hViewSize )
									{
										GGSURFACE_DESC desc;
										g_pGlob->pCurrentBitmapSurface->GetDesc(&desc);
										int width = desc.Width, height = desc.Height;
										GGHANDLE hWidth = hTexParam->GetAnnotationByName( "width" );
										GGHANDLE hHeight = hTexParam->GetAnnotationByName( "height" );
										if ( hWidth && hWidth->IsValid() ) hWidth->AsScalar()->GetInt( &width );
										if ( hHeight && hHeight->IsValid() ) hHeight->AsScalar()->GetInt( &height );
										if ( width == -1 ) width = desc.Width;
										if ( height == -1 ) height = desc.Height;
										GGVECTOR4 vec( (float) width, (float) height, 0, 0 );
										hViewSize->AsVector()->SetFloatVector( (float*)&vec );

										// DX11 seems viewport stays at fullscreen size (needs to be set to target size)
										D3D11_VIEWPORT vp = { 0, 0, width, height, 0, 1 };
										//m_pImmediateContext->RSSetViewports( 1, &vp );
										SetupSetViewport ( -1, &vp, NULL );
									}
								}
							}
						}
					}
					else
					{
						// no, we render to current as normal
						SetRenderAndDepthTarget ( pCurrentRenderTarget, pCurrentDepthTarget );

						// restore to standard depth state
						SetMeshDepthStates ( pMesh, true );

						// set the Viewsize to match the width and height of the RT passed in
						GGHANDLE hViewSize = pEffect->GetVariableBySemantic( "ViewSize" );
						if ( hViewSize )
						{
							GGSURFACE_DESC desc;
							g_pGlob->pCurrentBitmapSurface->GetDesc(&desc);
							GGVECTOR4 vec( (float) desc.Width, (float) desc.Height, 0, 0 );
							hViewSize->AsVector()->SetFloatVector( (float*)&vec );

							// DX11 seems viewport stays at fullscreen size (needs to be set to target size)
							D3D11_VIEWPORT vp = { 0, 0, desc.Width, desc.Height, 0, 1 };
							//m_pImmediateContext->RSSetViewports( 1, &vp );
							SetupSetViewport ( g_pGlob->dwRenderCameraID, &vp, NULL );
						}
					}
				//}
				// once textures established, commit effect state changes and begin this pass
				bMustCommit = true;
				#else
				// ensure technique exists
				GGHANDLE hTech = pEffect->GetCurrentTechnique();

				// get rendercolortarget string from this pass
				GGHANDLE hPass = pEffect->GetPass( hTech, uPass );
				GGHANDLE hRT = pEffect->GetAnnotationByName( hPass, "RenderColorTarget" );
				const char* szRT = 0;
				if ( hRT ) pEffect->GetString( hRT, &szRT );

				// if detect, use special depth texture render target
				if ( szRT && strnicmp( szRT, "[depthtexture]", strlen("[depthtexture]") )==NULL ) 
				{
					// NOTE: Optimize here, should not be using strings/compares
					if ( g_pGlob->dwRenderCameraID==0 )
					{
						// only render depths from camera zero
						int iSuccess = SwitchRenderTargetToDepth(0);
					}
					else
					{
						// actually SKIP this pass if any other camera
						return false;
					}
				}
				else
				{
					// check if RT string has contents
					if ( hRT && strcmp( szRT, "" ) != 0 )
					{
						// yes, now get handle to texture param we want to re-direct our render to
						GGHANDLE hTexParamWant = pEffect->GetParameterByName( NULL, szRT );

						// go through all textures recorded during the shader parse
						for ( DWORD t = 0; t < pMesh->pVertexShaderEffect->m_dwTextureCount; t++ )
						{
							// only consider RT textures that are flagged in bitmask
							DWORD dwThisTextureBit = 1 << t;
							if ( pMesh->pVertexShaderEffect->m_dwCreatedRTTextureMask & dwThisTextureBit )
							{
								// get the param handle of each texture in shader
								int iParam = 0;
								GGHANDLE hTexParam = NULL;
								if ( t<=31 ) 
								{
									iParam = pMesh->pVertexShaderEffect->m_iParamOfTexture [ t ];
									hTexParam = pEffect->GetParameter( NULL, iParam );
								}

								// if it matches the one we want
								if ( hTexParam == hTexParamWant )
								{
									// get texture ptr from effect
									IDirect3DBaseTexture9* pRTTex = NULL;
									pEffect->GetTexture( hTexParam, &pRTTex );

									// switch render target to internal shader RT
									IGGSurface *pSurface;
									((IGGTexture*)pRTTex)->GetSurfaceLevel( 0, &pSurface );
									m_pD3D->SetRenderTarget( 0, pSurface );
									if ( pSurface ) pSurface->Release( );

									// put RT textures width and height into ViewSize
									GGHANDLE hViewSize = pEffect->GetParameterBySemantic( NULL, "ViewSize" );
									if ( hViewSize )
									{
										D3DSURFACE_DESC desc;
										pCurrentRenderTarget->GetDesc( &desc );
										int width = desc.Width, height = desc.Height;
										GGHANDLE hWidth = pEffect->GetAnnotationByName( hTexParam, "width" );
										GGHANDLE hHeight = pEffect->GetAnnotationByName( hTexParam, "height" );
										if ( hWidth ) pEffect->GetInt( hWidth, &width );
										if ( hHeight ) pEffect->GetInt( hWidth, &height );
										GGVECTOR4 vec( (float) width, (float) height, 0, 0 );
										pEffect->SetVector( hViewSize, &vec );
									}
								}
							}
						}
					}
					else
					{
						// no, we render to current as normal
						m_pD3D->SetRenderTarget( 0, pCurrentRenderTarget );
						m_pD3D->SetDepthStencilSurface( pCurrentDepthTarget );

						// set the Viewsize to match the width and height of the RT passed in
						GGHANDLE hViewSize = pEffect->GetParameterBySemantic( NULL, "ViewSize" );
						if ( hViewSize )
						{
							D3DSURFACE_DESC desc;
							pCurrentRenderTarget->GetDesc( &desc );
							GGVECTOR4 vec( (float) desc.Width, (float) desc.Height, 0, 0 );
							pEffect->SetVector( hViewSize, &vec );
						}
					}
				}

				// once textures established, commit effect state changes and begin this pass
				bMustCommit = true;
				#endif
			}

			// U77 - 270111 - pass clipping data to shader (automatic) (duplicated in terrain renderer:BT_Intern_RenderTerrain)
			if ( pMesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle )
			{
				GGVECTOR4 vec;
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
				if ( m_Camera_Ptr )
				{
					if ( m_Camera_Ptr->iClipPlaneOn==1 )
					{
						// special mode which creates plane but does not use RenderState to set clip
						// as you cannot mix FF clip and HLSL clip in same scene (artefacts)
						vec.x = m_Camera_Ptr->planeClip.a;
						vec.y = m_Camera_Ptr->planeClip.b;
						vec.z = m_Camera_Ptr->planeClip.c;
						vec.w = m_Camera_Ptr->planeClip.d;
					}
					else
					{
						// ensure shader stops using clip plane when not being clipped!
						vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
					}
				}
				else
				{
					// ensure shader stops using clip plane when not being clipped!
					vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
				}
				#ifdef DX11
				pMesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle->AsVector()->SetFloatVector ( (float*)&vec );
				#else
				pEffect->SetVector( pMesh->pVertexShaderEffect->m_VecClipPlaneEffectHandle, &vec );
				#endif
				bMustCommit = true;
			}

			// special effect which can override the texture stage of an instanced object (Guru)
			#ifdef DX11
			if ( g_InstanceAlphaControlValue > 0 )
			{
				// eventually we can use the other RGB components to communicate this highlight info!
				DWORD dwRedPart = (g_InstanceAlphaControlValue >> 16) & 0xFF;
				DWORD dwGreenPart = (g_InstanceAlphaControlValue >> 8) & 0xFF;
				DWORD dwBluePart = (g_InstanceAlphaControlValue) & 0xFF;
				GGHANDLE gGlowIntensity = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "GlowIntensity" );
				if ( gGlowIntensity )
				{
					pMesh->pVertexShaderEffect->m_GlowIntensityHandle = gGlowIntensity;
					GGVECTOR4 vecHighlight = GGVECTOR4((float)dwRedPart/255.0f,(float)dwGreenPart/255.0f,(float)dwBluePart/255.0f,0);
					gGlowIntensity->AsVector()->SetFloatVector((float*)&vecHighlight);
				}
			}
			else
			{
				GGHANDLE gGlowIntensity = pMesh->pVertexShaderEffect->m_GlowIntensityHandle;
				if ( gGlowIntensity )
				{
					pMesh->pVertexShaderEffect->m_GlowIntensityHandle = NULL;
					GGVECTOR4 vecHighlight = GGVECTOR4(0,0,0,0);
					gGlowIntensity->AsVector()->SetFloatVector((float*)&vecHighlight);
					bMustCommit = true;
				}
			}
			#else
			if ( g_InstanceAlphaControlValue > 0 )
			{
				// eventually we can use the other RGB components to communicate this highlight info!
				DWORD dwRedPart = (g_InstanceAlphaControlValue >> 16) & 0xFF;
				DWORD dwGreenPart = (g_InstanceAlphaControlValue >> 8) & 0xFF;
				DWORD dwBluePart = (g_InstanceAlphaControlValue) & 0xFF;
				GGHANDLE gGlowIntensity = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName ( NULL, "GlowIntensity" );
				if ( gGlowIntensity )
				{
					pMesh->pVertexShaderEffect->m_GlowIntensityHandle = gGlowIntensity;
					GGVECTOR4 vecHighlight = GGVECTOR4((float)dwRedPart/255.0f,(float)dwGreenPart/255.0f,(float)dwBluePart/255.0f,0);
					pMesh->pVertexShaderEffect->GGSetEffectVector( gGlowIntensity, &vecHighlight );
					bMustCommit = true;
				}
			}
			else
			{
				GGHANDLE gGlowIntensity = pMesh->pVertexShaderEffect->m_GlowIntensityHandle;
				if ( gGlowIntensity )
				{
					pMesh->pVertexShaderEffect->m_GlowIntensityHandle = NULL;
					GGVECTOR4 vecHighlight = GGVECTOR4(0,0,0,0);
					pMesh->pVertexShaderEffect->GGSetEffectVector ( gGlowIntensity, &vecHighlight );
					bMustCommit = true;
				}
			}
			#endif

			// Character Creator Tone Control - Using the object pointer to work out
			if ( pObject && pObject->pCharacterCreator )
			{				
				#ifdef DX11
				if ( pMesh->pVertexShaderEffect->m_ColorTone[0] == NULL )
				{
					pMesh->pVertexShaderEffect->m_ColorTone[0] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ColorTone" );
				}
				if ( pMesh->pVertexShaderEffect->m_ToneMix[0] == NULL )
				{
					pMesh->pVertexShaderEffect->m_ToneMix[0] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ToneMix" );
				}
				if ( pMesh->pVertexShaderEffect->m_ColorTone[0] )
				{
					float fData[16];
					for ( int i = 0 ; i < 4 ; i++ )
					{
						fData[(i*4)+0] = pObject->pCharacterCreator->ColorTone[i][0];
						fData[(i*4)+1] = pObject->pCharacterCreator->ColorTone[i][1];
						fData[(i*4)+2] = pObject->pCharacterCreator->ColorTone[i][2];
						fData[(i*4)+3] = 1.0f;
					}
					pMesh->pVertexShaderEffect->m_ColorTone[0]->AsVector()->SetFloatVectorArray ( fData, 0, 4 );
				}
				if ( pMesh->pVertexShaderEffect->m_ToneMix[0] )
				{
					float fData[4];
					for ( int i = 0 ; i < 4 ; i++ )
					{
						fData[i] = pObject->pCharacterCreator->ToneMix[i];
					}
					pMesh->pVertexShaderEffect->m_ToneMix[0]->AsScalar()->SetFloatArray ( fData, 0, 4 );
				}
				#else
				for ( int i = 0 ; i < 4 ; i++ )
				{
					if ( pMesh->pVertexShaderEffect->m_ColorTone[i] == NULL )
					{
						char s[256];
						sprintf ( s , "ColorTone[%i]" , i );
						#ifdef DX11
						pMesh->pVertexShaderEffect->m_ColorTone[i] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( s );
						#else
						pMesh->pVertexShaderEffect->m_ColorTone[i] = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName ( NULL, s );
						#endif
					}
					if ( pMesh->pVertexShaderEffect->m_ToneMix[i] == NULL )
					{
						char s[256];
						sprintf ( s , "ToneMix[%i]" , i );
						#ifdef DX11
						pMesh->pVertexShaderEffect->m_ToneMix[i] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( s );
						#else
						pMesh->pVertexShaderEffect->m_ToneMix[i] = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName ( NULL, s );
						#endif
					}

					if ( pMesh->pVertexShaderEffect->m_ColorTone[i] )
					{
						GGVECTOR4 vecColorTone = GGVECTOR4(pObject->pCharacterCreator->ColorTone[i][0],pObject->pCharacterCreator->ColorTone[i][1],pObject->pCharacterCreator->ColorTone[i][2],1.0f);
						#ifdef DX11
						pMesh->pVertexShaderEffect->m_ColorTone[i]->AsVector()->SetFloatVector ( (float*)&vecColorTone );
						#else
						pMesh->pVertexShaderEffect->GGSetEffectVector( pMesh->pVertexShaderEffect->m_ColorTone[i], &vecColorTone );
						#endif
					}
					if ( pMesh->pVertexShaderEffect->m_ToneMix[i] )
					{
						#ifdef DX11
						pMesh->pVertexShaderEffect->m_ToneMix[i]->AsVector()->SetFloatVector ( (float*)&pObject->pCharacterCreator->ToneMix[i] );
						#else
						pMesh->pVertexShaderEffect->GGSetEffectFloat( pMesh->pVertexShaderEffect->m_ToneMix[i], pObject->pCharacterCreator->ToneMix[i] );
						#endif
					}
				}
				#endif
				bMustCommit = true;
				pMesh->pVertexShaderEffect->m_bCharacterCreatorTonesOn = true;
			}
			else
			{		
				if ( pMesh->pVertexShaderEffect->m_bCharacterCreatorTonesOn == true )
				{
					#ifdef DX11
					if ( pMesh->pVertexShaderEffect->m_ColorTone[0] == NULL )
					{
						pMesh->pVertexShaderEffect->m_ColorTone[0] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ColorTone" );
					}
					if ( pMesh->pVertexShaderEffect->m_ColorTone[0] )
					{
						float fData[16];
						for ( int i = 0 ; i < 4 ; i++ )
						{
							fData[(i*4)+0] = -1;
							fData[(i*4)+1] = 0;
							fData[(i*4)+2] = 0;
							fData[(i*4)+3] = 1.0f;
						}
						pMesh->pVertexShaderEffect->m_ColorTone[0]->AsVector()->SetFloatVectorArray ( fData, 0, 4 );
					}
					#else
					for ( int i = 0 ; i < 4 ; i++ )
					{
						if ( pMesh->pVertexShaderEffect->m_ColorTone[i] == NULL )
						{
							char s[256];
							sprintf ( s , "ColorTone[%i]" , i );
							#ifdef DX11
							pMesh->pVertexShaderEffect->m_ColorTone[i] = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( s );
							#else
							pMesh->pVertexShaderEffect->m_ColorTone[i] = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName ( NULL, s );
							#endif
						}

						if ( pMesh->pVertexShaderEffect->m_ColorTone[i] )
						{
							GGVECTOR4 vecColorTone = GGVECTOR4(-1,0,0,1.0f);
							#ifdef DX11
							pMesh->pVertexShaderEffect->m_ColorTone[i]->AsVector()->SetFloatVector ( (float*)&vecColorTone );
							#else
							pMesh->pVertexShaderEffect->GGSetEffectVector( pMesh->pVertexShaderEffect->m_ColorTone[i], &vecColorTone );
							#endif
						}
					}
					#endif
					bMustCommit = true;
					pMesh->pVertexShaderEffect->m_bCharacterCreatorTonesOn = false;
				}
			}

			// added specular override to per object rendering
			#ifdef DX11
			GGHANDLE pSpecularOverride = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "SpecularOverride" );
			if ( pSpecularOverride )
			{
				pSpecularOverride->AsScalar()->SetFloat ( pMesh->fSpecularOverride );
				bMustCommit = true;
			}
			#else
			GGHANDLE pSpecularOverride = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName ( NULL, "SpecularOverride" );
			if ( pSpecularOverride )
			{
				pMesh->pVertexShaderEffect->GGSetEffectFloat( pSpecularOverride, pMesh->fSpecularOverride );
				bMustCommit = true;
			}
			#endif

			// added UV control to per object rendering
			#ifdef DX11
			GGHANDLE pScrollScaleUV = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ScrollScaleUV" );
			if ( pScrollScaleUV )
			{
				GGVECTOR4 vec4 = GGVECTOR4 ( pMesh->fScrollOffsetU, pMesh->fScrollOffsetV, pMesh->fScaleOffsetU, pMesh->fScaleOffsetV );
				pScrollScaleUV->AsVector()->SetFloatVector ( (float*)&vec4 );
			}
			#endif

			// added per-object control for additional artist flags
			#ifdef DX11
			GGHANDLE pArtFlags = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName ( "ArtFlagControl1" );
			if ( pArtFlags )
			{
				float fInvertNormal = 0.0f;
				float fGenerateTangents = 0.0f;
				if ( pMesh->dwArtFlags & 0x1 ) fInvertNormal = 1.0f;
				if ( pMesh->dwArtFlags & 0x2 ) fGenerateTangents = 1.0f;
				float fBoostIntensity = pMesh->fBoostIntensity;
				GGVECTOR4 vec4 = GGVECTOR4 ( fInvertNormal, fGenerateTangents, fBoostIntensity, 0.0f );
				pArtFlags->AsVector()->SetFloatVector ( (float*)&vec4 );
			}
			#endif

			// when flagged, we must update effect with changes we made
			if ( bMustCommit==true )
			{
				// commit effect state changes to begin this pass
				#ifdef DX11
				#else
				pEffect->CommitChanges( );
				#endif
			}
		}

		// FX Effect or Regular
		if ( bEffectRendering )
		{
			// disable fog for shaders and begin effect rendering
			#ifdef DX11
			ID3DX11EffectTechnique* pTech = pMesh->pVertexShaderEffect->m_hCurrentTechnique;//m_pEffect->GetTechniqueByIndex(0);
			ID3DX11EffectPass* pPass = pTech->GetPassByIndex(uPass);
			pPass->Apply(0, m_pImmediateContext);
			#else
			m_pD3D->SetRenderState ( D3DRS_FOGENABLE, FALSE );
			pMesh->pVertexShaderEffect->m_pEffect->BeginPass ( uPass );
			#endif
		}

		// old FF texturing code (some effects do not do any texturing stuff)
		// this allowed non PS shader to use DBP textures but it killed shader ability to use DBP textures that HAD PS code!
		if ( bEffectRendering )
		{
			// effects CAN use 'texture object' textures if the effect did not assign a specfic texture to them (paul request for DarkSHADER)
			if ( pMesh->pTextures )
			{
				for ( DWORD dwTextureIndex = 0; dwTextureIndex < pMesh->dwTextureCount; dwTextureIndex++ )
				{
					DWORD dwTextureStage = pMesh->pTextures [ dwTextureIndex ].dwStage;
					if ( dwTextureStage < 16 )
					{
						// get texture ptr
						sTexture* pTexture = &pMesh->pTextures [ dwTextureIndex ];

						// m_dwUseDynamicTextureMask holds a mask of 32 bits, 1=use dynamic texture form texture object command
						int iUseDyntex = ( ( pMesh->pVertexShaderEffect->m_dwUseDynamicTextureMask >> dwTextureStage ) & 1 );
						if ( iUseDyntex==1 )
						{
							// when in effect, only if texture in effect is NULL should this be allowed
							#ifdef DX11
							#else
							if ( pTexture->pTexturesRef )
							{
								m_pD3D->SetTexture ( dwTextureStage, pTexture->pTexturesRef );
							}
							else
							{
								if ( pMesh->pTextures [ dwTextureIndex ].pCubeTexture )
									m_pD3D->SetTexture ( dwTextureStage, pTexture->pCubeTexture );
								else
									m_pD3D->SetTexture ( dwTextureStage, NULL);
							}
							#endif
						}
					}
				}

				// set dynamic depth mapping texture for this effect
				if ( pMesh->pVertexShaderEffect )
				{
					if ( pMesh->pVertexShaderEffect->m_pEffect==g_pMainCameraDepthEffect )
					{
						#ifdef DX11
						tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );
						if ( m_Camera_Ptr )
						{
							g_pMainCameraDepthHandle->AsShaderResource()->SetResource(m_Camera_Ptr->pImageDepthResourceView);
						}
						#else
						g_pMainCameraDepthEffect->SetTexture ( g_pMainCameraDepthHandle, g_pMainCameraDepthTexture );
						#endif
					}
				}

				// set dynamic shadow mapping texture for this effect
				if ( g_CascadedShadow.m_depthTexture[0] )
				{
					if ( pMesh->pVertexShaderEffect->m_bPrimaryEffectForCascadeShadowMapping==true )
					{
						// depth handles only relate to ONE shader
						#ifdef DX11
						for ( int i = 0; i < 8; i++ )
							if ( g_CascadedShadow.m_depthHandle[i] && g_CascadedShadow.m_depthTexture[i] ) 
								g_CascadedShadow.m_depthHandle[i]->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[i]->getTextureResourceView() );
						#else
						pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( g_CascadedShadow.m_depthHandle[0], g_CascadedShadow.m_depthTexture[0]->getTexture() );
						pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( g_CascadedShadow.m_depthHandle[1], g_CascadedShadow.m_depthTexture[1]->getTexture() );
						pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( g_CascadedShadow.m_depthHandle[2], g_CascadedShadow.m_depthTexture[2]->getTexture() );
						pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( g_CascadedShadow.m_depthHandle[3], g_CascadedShadow.m_depthTexture[3]->getTexture() );
						#endif
					}
					else
					{
						// any effect that has DepthMapTX4 will be filled with most distant shadow cascade render
						DWORD dwEffectIndex = pMesh->pVertexShaderEffect->m_dwEffectIndex;
						if ( dwEffectIndex < EFFECT_INDEX_SIZE )
						{
							if ( g_CascadedShadow.m_pEffectParam[dwEffectIndex] )
							{
								GGHANDLE hdepthHandle0 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX1;
								GGHANDLE hdepthHandle1 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX2;
								GGHANDLE hdepthHandle2 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX3;
								GGHANDLE hdepthHandle3 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX4;
								GGHANDLE hdepthHandle4 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX5;
								GGHANDLE hdepthHandle5 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX6;
								GGHANDLE hdepthHandle6 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX7;
								GGHANDLE hdepthHandle7 = g_CascadedShadow.m_pEffectParam[dwEffectIndex]->DepthMapTX8;
								#ifdef DX11
								if ( hdepthHandle0 && g_CascadedShadow.m_depthTexture[0] ) hdepthHandle0->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[0]->getTextureResourceView() );
								if ( hdepthHandle1 && g_CascadedShadow.m_depthTexture[1] ) hdepthHandle1->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[1]->getTextureResourceView() );
								if ( hdepthHandle2 && g_CascadedShadow.m_depthTexture[2] ) hdepthHandle2->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[2]->getTextureResourceView() );
								if ( hdepthHandle3 && g_CascadedShadow.m_depthTexture[3] ) hdepthHandle3->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[3]->getTextureResourceView() );
								if ( hdepthHandle4 && g_CascadedShadow.m_depthTexture[4] ) hdepthHandle4->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[4]->getTextureResourceView() );
								if ( hdepthHandle5 && g_CascadedShadow.m_depthTexture[5] ) hdepthHandle5->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[5]->getTextureResourceView() );
								if ( hdepthHandle6 && g_CascadedShadow.m_depthTexture[6] ) hdepthHandle6->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[6]->getTextureResourceView() );
								if ( hdepthHandle7 && g_CascadedShadow.m_depthTexture[7] ) hdepthHandle7->AsShaderResource()->SetResource ( g_CascadedShadow.m_depthTexture[7]->getTextureResourceView() );
								#else
								if ( hdepthHandle0 ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( hdepthHandle0, g_CascadedShadow.m_depthTexture[0]->getTexture() );
								if ( hdepthHandle1 ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( hdepthHandle1, g_CascadedShadow.m_depthTexture[1]->getTexture() );
								if ( hdepthHandle2 ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( hdepthHandle2, g_CascadedShadow.m_depthTexture[2]->getTexture() );
								if ( hdepthHandle3 ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( hdepthHandle3, g_CascadedShadow.m_depthTexture[3]->getTexture() );
								#endif
							}
						}
					}
				}
			}
		}
		else
		{
			// FIXED FUNCTION TEXTURING
			#ifdef DX11
			#else
			// FF affects HLSL pipeline (and vice versa), so switch on
			// the automated clipping plane if end of override
			if ( m_RenderStates.bOverriddenClipPlaneforHLSL==true )
			{
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
				if ( m_Camera_Ptr )
				{
					if ( m_Camera_Ptr->iClipPlaneOn!=0 )
						m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 );
					else
						m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
				}
				m_RenderStates.bOverriddenClipPlaneforHLSL = false;
			}

			// call the texturestate function
			if ( !SetMeshTextureStates ( pMesh ) )
				return bResult;

			// is there a texture
			if ( pMesh->pTextures )
			{
				// store the current texture
				m_iCurrentTexture = pMesh->pTextures [ 0 ].iImageID;

				// is it different to the last texture we set (leefix-040803-and only if single texture otherwise lightmaps might be used)
				if ( m_iCurrentTexture != m_iLastTexture || pMesh->dwTextureCount>1 )
				{
					// set the new texture - along with related stage textures
					for ( DWORD dwTextureIndex = 0; dwTextureIndex < pMesh->dwTextureCount; dwTextureIndex++ )
					{
						// Determine texture stage to write to
						DWORD dwTextureStage = pMesh->pTextures [ dwTextureIndex ].dwStage;

						// Determine texture data ptr
						sTexture* pTexture = &pMesh->pTextures [ dwTextureIndex ];

						if ( pTexture->pTexturesRef )
						{
							// set regular texture
							if ( FAILED ( m_pD3D->SetTexture ( dwTextureStage, pTexture->pTexturesRef ) ) )
								break;
						}
						else
						{
							if ( pMesh->pTextures [ dwTextureIndex ].pCubeTexture )
							{
								// set cube texture
								if ( FAILED ( m_pD3D->SetTexture ( dwTextureStage, pTexture->pCubeTexture ) ) )
									break;
							}
							else
							{
								// set no texture
								if ( FAILED ( m_pD3D->SetTexture ( dwTextureStage, NULL) ) )
									break;
							}
						}
					}

					// now store the current texture
					m_iLastTexture = m_iCurrentTexture;
				}
			}
			else
			{
				// default zero texture
				m_pD3D->SetTexture ( 0, NULL );
				m_iLastTexture = 0;
			}
			#endif
		}
	}

	// always success (unless exited early with false from depth render skip)
	return bResult;
}

bool CObjectManager::ShaderPassEnd ( sMesh* pMesh, bool bEffectRendering )
{
	// End FX Effect
	#ifdef DX11
	#else
	if ( bEffectRendering )	pMesh->pVertexShaderEffect->m_pEffect->EndPass();
	#endif

	// continue
	return true;
}

bool CObjectManager::ShaderFinish ( sMesh* pMesh, LPGGRENDERTARGETVIEW pCurrentRenderTarget, LPGGDEPTHSTENCILVIEW pCurrentDepthTarget )
{
	// if using RT, restore current render target
	if ( pCurrentRenderTarget )
	{
		if ( pMesh->pVertexShaderEffect )
		{
			if ( pMesh->pVertexShaderEffect->m_bUsesAtLeastOneRT==true )
			{
				SetRenderAndDepthTarget ( pCurrentRenderTarget, pCurrentDepthTarget );
			}
		}
	}

	// Run any end code for any effect used
	if ( pMesh->pVertexShaderEffect )
	    pMesh->pVertexShaderEffect->End();

	/*
	// free dynamic shadow mapping to release input stage (ready for next output stage)
	if ( g_CascadedShadow.m_depthTexture[0] )
	{
		if ( pMesh->pVertexShaderEffect->m_bPrimaryEffectForCascadeShadowMapping==true )
		{
			for ( int i = 0; i < 8; i++ )
				if ( g_CascadedShadow.m_depthHandle[i] && g_CascadedShadow.m_depthTexture[i] ) 
					g_CascadedShadow.m_depthHandle[i]->AsShaderResource()->SetResource ( NULL );
		}
	}
	*/

	// continue
	return true;
}

inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

//PE: for debug info.
//#include <DxErr.h>
//#pragma comment(lib, "dxerr.lib")



bool CObjectManager::DrawMesh ( sMesh* pMesh, bool bIgnoreOwnMeshVisibility, sObject* pObject, sFrame* pFrame)
{
	// get pointer to drawbuffers
	sDrawBuffer* pDrawBuffer = pMesh->pDrawBuffer;
	if ( pDrawBuffer==NULL )
		return true;

	// skip if mesh is invisible
	if ( bIgnoreOwnMeshVisibility==false )
	{
		if ( pMesh->iMeshType==0 )
		{
			// regular mesh mode
			if ( pMesh->bVisible==false )
				return true;
		}
		if ( pMesh->iMeshType==1 )
		{
			// terrain mesh mode (scene culling system)
			if ( g_pGlob->dwRenderCameraID==0 )
				if ( pMesh->bVisible==false )
					return true;
		}
	}

	//PE: Use fastest for depth only (shadows)
	if (g_pGlob->dwRenderCameraID >= 31) {
		//PE: fastest setup for depth only.
		m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 0 );
		m_pImmediateContext->OMSetBlendState(m_pBlendStateNoAlpha, 0, 0xffffffff);
		if (pMesh->bCull == false)
			m_pImmediateContext->RSSetState(m_pRasterStateNoCull);
		else
			m_pImmediateContext->RSSetState(m_pRasterState);
	}
	else {
		// call the renderstate function
		if (!SetMeshRenderStates(pMesh))
			return false;
	}
	// set the input streams
	if ( !SetInputStreams ( pMesh ) )
		return false;

	// do not render meshes with an effect and a single poly
	bool bSkipDrawNow = false;
	if ( pMesh->pVertexShaderEffect && pMesh->dwVertexCount<=3 )
		if ( pObject->dwObjectNumber < 70000 ) // 220618 - horrid hack (later find out why we need to hide single poly renders)
			bSkipDrawNow = true;

	#ifdef DX11
	// set input layout
	if ( pMesh->pVertexDec == NULL && pMesh->pVertexShaderEffect && bSkipDrawNow == false )
	{
		int iMeshEffectID = pMesh->pVertexShaderEffect->m_iEffectID;
		if ( iMeshEffectID > 0 && iMeshEffectID < SHADERSARRAYMAX && g_sShaders[iMeshEffectID].pInputLayout == NULL )
		{
			D3D11_INPUT_ELEMENT_DESC layoutFVF258 [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVF274 [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVF338 [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32_UINT,				0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVF514 [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVF530 [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVFZero [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			D3D11_INPUT_ELEMENT_DESC layoutFVFZeroEightBone [ ] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			int iLayoutSize = 0;
			LPVOID pLayoutPtr = NULL;
			DWORD dwLayoutSize = 0;
			if ( pMesh->dwFVF == 258 )
			{
				iLayoutSize = 2;
				pLayoutPtr = &layoutFVF258;
				dwLayoutSize = sizeof(layoutFVF258);
			}
			if ( pMesh->dwFVF == 274 )
			{
				iLayoutSize = 3;
				pLayoutPtr = &layoutFVF274;
				dwLayoutSize = sizeof(layoutFVF274);
			}
			if ( pMesh->dwFVF == 338 )
			{
				iLayoutSize = 4;
				pLayoutPtr = &layoutFVF338;
				dwLayoutSize = sizeof(layoutFVF338);
			}
			if ( pMesh->dwFVF == 514 )
			{
				iLayoutSize = 3;
				pLayoutPtr = &layoutFVF514;
				dwLayoutSize = sizeof(layoutFVF514);
			}
			if ( pMesh->dwFVF == 530 )
			{
				iLayoutSize = 6;
				pLayoutPtr = &layoutFVF530;
				dwLayoutSize = sizeof(layoutFVF530);
			}
			if ( pMesh->dwFVF == 0 )
			{
				if ( pMesh->dwFVFOriginal == 530 )
				{
					// been conveerted to add tangent/binormal and it wiped FVF value (PBR lightmaps)
					iLayoutSize = 6;
					pLayoutPtr = &layoutFVF530;
					dwLayoutSize = sizeof(layoutFVF530);
				}
				else
				{
					if ( pMesh->dwFVFSize == 120 )
					{
						// latest 8 bones per vertex
						iLayoutSize = 9;
						pLayoutPtr = &layoutFVFZeroEightBone;
						dwLayoutSize = sizeof(layoutFVFZeroEightBone);
					}
					else
					{
						// regular 4 bones per vertex
						iLayoutSize = 7;
						pLayoutPtr = &layoutFVFZero;
						dwLayoutSize = sizeof(layoutFVFZero);
					}
				}
			}
			//LPGGVERTEXLAYOUT pNewVertexDec;	
			D3D11_INPUT_ELEMENT_DESC* pLayout = new D3D11_INPUT_ELEMENT_DESC [ iLayoutSize ];
			std::memcpy ( pLayout, pLayoutPtr, dwLayoutSize );
			ID3DBlob* pBlob = g_sShaders[iMeshEffectID].pBlob;
			DWORD tIndex = 0;
			ID3DX11EffectTechnique* tech = g_sShaders[iMeshEffectID].pEffect->GetTechniqueByIndex(0);
			ID3DX11EffectPass* pass = tech->GetPassByIndex(0);
			D3DX11_PASS_SHADER_DESC vs_desc;
			pass->GetVertexShaderDesc(&vs_desc);
			D3DX11_EFFECT_SHADER_DESC s_desc;
			vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
			HRESULT hr = m_pD3D->CreateInputLayout ( pLayout, iLayoutSize, s_desc.pBytecode, s_desc.BytecodeLength, &g_sShaders[iMeshEffectID].pInputLayout );

			
			//PE: superflatterrain=1
			//PE: generate : Exception thrown at 0x776508F2 in Guru-MapEditor.exe: Microsoft C++ exception: _com_error at memory location 0x0019DDB0.
			if (hr != NOERROR) {
				//PE: Failed , terrain use - tindex  1, pass 1
				if ( iMeshEffectID == 1 ) { // 1==terrain. same as t.terrain.terrainshaderindex == iMeshEffectID , but we dont have t or g.

					SAFE_DELETE_ARRAY(pLayout);
					int iLayoutSize = 4;
					pLayout = new D3D11_INPUT_ELEMENT_DESC[iLayoutSize];
					D3D11_INPUT_ELEMENT_DESC layout[] =
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					};
					pLayoutPtr = &layout;
					std::memcpy(pLayout, layout, sizeof(layout));

					DWORD tIndex = 0;
					ID3DX11EffectTechnique* tech = NULL;
					while ((tech = g_sShaders[iMeshEffectID].pEffect->GetTechniqueByIndex(tIndex++))->IsValid())
					{
						DWORD pIndex = 0;
						ID3DX11EffectPass* pass = NULL;
						while ((pass = tech->GetPassByIndex(pIndex++))->IsValid())
						{
							D3DX11_PASS_SHADER_DESC vs_desc;
							pass->GetVertexShaderDesc(&vs_desc);
							D3DX11_EFFECT_SHADER_DESC s_desc;
							vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
							hr = m_pD3D->CreateInputLayout(pLayout, iLayoutSize, s_desc.pBytecode, s_desc.BytecodeLength, &g_sShaders[iMeshEffectID].pInputLayout);
							break;
						}
						if (g_sShaders[iMeshEffectID].pInputLayout != NULL) {
							break;
						}
					}

//					if (hr != NOERROR) {
//						//PE: debug.
//						char tmpdebug[2048];
//						sprintf(tmpdebug, "Error: %s error description: %s\n",
//							DXGetErrorString(hr), DXGetErrorDescription(hr));
//						OutputDebugString(tmpdebug);
//						//PE: Error returned:
//						//PE: Error: E_INVALIDARG error description: An invalid parameter was passed to the returning function.
//					}
				}
			}

			SAFE_DELETE_ARRAY(pLayout);
		}
		pMesh->pVertexDec = g_sShaders[iMeshEffectID].pInputLayout;
	}

	// set input layout
	if ( pMesh->pVertexDec == NULL ) return false;
	m_pImmediateContext->IASetInputLayout ( pMesh->pVertexDec );

	// primitive type
	m_pImmediateContext->IASetPrimitiveTopology(pDrawBuffer->dwPrimType);
	#endif

	// This POINTLIST code is not used by CORE PARTICLES (uses own renderer)
	// nor is it used by CLOTH&PARTICLES (uses quad based rendered meshes)
	// this is actuall used by the PHYSX PLUGIN FOR FLUID PARTICLES
	#ifdef DX11
	#else
	if ( pMesh->iPrimitiveType == D3DPT_POINTLIST )
	{
		// set a default - mike needs to do this in ANYTHING that creates a pointlist object
		if ( pMesh->Collision.fRadius==0.0f ) pMesh->Collision.fRadius = 50.0f;

		// handle point sprite for distance scaling and default mesh point sprite size
		m_pD3D->SetRenderState( D3DRS_POINTSCALEENABLE, TRUE );
		m_pD3D->SetRenderState( D3DRS_POINTSIZE,		FtoDW(pMesh->Collision.fRadius/100.0f) );
		m_pD3D->SetRenderState( D3DRS_POINTSIZE_MIN,	FtoDW(0.0f) );
		m_pD3D->SetRenderState( D3DRS_POINTSIZE_MAX,	FtoDW(pMesh->Collision.fRadius) );
		m_pD3D->SetRenderState( D3DRS_POINTSCALE_A,		FtoDW(0.0f) );
		m_pD3D->SetRenderState( D3DRS_POINTSCALE_B,		FtoDW(0.0f) );
		m_pD3D->SetRenderState( D3DRS_POINTSCALE_C,		FtoDW(2.0f) );

		// force a basic texture render state
		m_pD3D->SetRenderState( D3DRS_POINTSPRITEENABLE, TRUE );
	}
	#endif

	// start shader
	UINT uPasses = 1;
	bool bEffectRendering = false;
	LPGGRENDERTARGETVIEW pCurrentRenderTarget = NULL;
	LPGGDEPTHSTENCILVIEW pCurrentDepthTarget = NULL;
	bool bLocalOverrideAllTexturesAndEffects = false;

	ShaderStart ( pMesh, &pCurrentRenderTarget, &pCurrentDepthTarget, &uPasses, &bEffectRendering, &bLocalOverrideAllTexturesAndEffects );

	//PE: Speed up depth rendering.
	int iTextureCount = pMesh->dwTextureCount;
	if (g_pGlob->dwRenderCameraID >= 31) {
		//Optimize for depth render.
		iTextureCount = 1;
	}

	// when activated, can SKIP a DEPTH PASS if effect has this pass
	UINT uPassStartIndex = 0; 
	if ( pMesh->pVertexShaderEffect )
		if ( pMesh->pVertexShaderEffect->m_DepthRenderPassHandle && g_bSkipAnyDedicatedDepthRendering==true ) 
			uPassStartIndex = 1;

	// loop through all shader passes
	// each mesh can have several render passes
	bool lightset = false;
    for(UINT uPass = uPassStartIndex; uPass < uPasses; uPass++)
    {
		if (!lightset && bSkipDrawNow == false) 
		{
			lightset = true;
			update_mesh_light(pMesh, pObject, pFrame);
		}

		// start shader pass
		if ( ShaderPass ( pMesh, uPass, uPasses, bEffectRendering, bLocalOverrideAllTexturesAndEffects, pCurrentRenderTarget, pCurrentDepthTarget, pObject )==true )
		{
			// create constant buffer (and set it AFTER effect apply)
			if ( g_pCBPerMesh == NULL )
			{
				D3D11_BUFFER_DESC bdPerFrameBuffer;
				std::memset ( &bdPerFrameBuffer, 0, sizeof ( bdPerFrameBuffer ) );
				bdPerFrameBuffer.Usage          = D3D11_USAGE_DEFAULT;
				bdPerFrameBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
				bdPerFrameBuffer.CPUAccessFlags = 0;
				bdPerFrameBuffer.ByteWidth      = sizeof ( CBPerMesh );
				m_pD3D->CreateBuffer ( &bdPerFrameBuffer, NULL, &g_pCBPerMesh );
				std::memset ( &bdPerFrameBuffer, 0, sizeof ( bdPerFrameBuffer ) );
				bdPerFrameBuffer.Usage          = D3D11_USAGE_DEFAULT;
				bdPerFrameBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
				bdPerFrameBuffer.CPUAccessFlags = 0;
				bdPerFrameBuffer.ByteWidth      = sizeof ( CBPerMeshPS );
				m_pD3D->CreateBuffer ( &bdPerFrameBuffer, NULL, &g_pCBPerMeshPS );
			}
			if ( g_pCBPerMesh && g_pCBPerMeshPS )
			{
				CBPerMesh cb;
				GGGetTransform(GGTS_WORLD,&cb.mWorld);
				GGGetTransform(GGTS_VIEW,&cb.mView);
				GGGetTransform(GGTS_PROJECTION,&cb.mProjection);
				GGMatrixTranspose(&cb.mWorld,&cb.mWorld);
				GGMatrixTranspose(&cb.mView,&cb.mView);
				GGMatrixTranspose(&cb.mProjection,&cb.mProjection);
				m_pImmediateContext->UpdateSubresource( g_pCBPerMesh, 0, NULL, &cb, 0, 0 );
				m_pImmediateContext->VSSetConstantBuffers ( 0, 1, &g_pCBPerMesh );

				if (g_pGlob->dwRenderCameraID < 31) 
				{ 
					//PE: Not used in PS when depth only.
					//PE: not used in PS with normal objects anymore.
					m_pImmediateContext->PSSetConstantBuffers ( 0, 1, &g_pCBPerMesh );

					int iEid = g.guishadereffectindex;
					if(pMesh && pMesh->pVertexShaderEffect) iEid = pMesh->pVertexShaderEffect->m_iEffectID;
					if ( iEid == g.guishadereffectindex || iEid == g.guidiffuseshadereffectindex || (iEid >= g.postprocesseffectoffset && iEid < g.postprocesseffectoffset+100) ) 
					{
						CBPerMeshPS cbps;
						cbps.vMaterialEmissive = GGCOLOR(pMesh->mMaterial.Emissive.r, pMesh->mMaterial.Emissive.g, pMesh->mMaterial.Emissive.b, pMesh->mMaterial.Emissive.a);
						if (pMesh->bAlphaOverride == true)
							cbps.fAlphaOverride = (pMesh->dwAlphaOverride >> 24) / 255.0f;
						else
							cbps.fAlphaOverride = 1.0f;

						// feed camera zero matrices into pixel shader constant buffer for depth-to-world calc 
						tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData(0);
						float fDet = 0.0f;
						GGMatrixInverse(&cbps.mViewInv, &fDet, &m_Camera_Ptr->matView);
						GGMatrixTranspose(&cbps.mViewInv, &cbps.mViewInv);
						cbps.mViewProj = g_matThisViewProj;
						GGMatrixTranspose(&cbps.mViewProj, &cbps.mViewProj);
						cbps.mPrevViewProj = g_matPreviousViewProj;
						GGMatrixTranspose(&cbps.mPrevViewProj, &cbps.mPrevViewProj);
						m_pImmediateContext->UpdateSubresource(g_pCBPerMeshPS, 0, NULL, &cbps, 0, 0);
						m_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pCBPerMeshPS);
					}
				}
			}

			// apply textures for shader
			#ifdef DX11
			//for ( int i = 0; i < iTextureCount; i++ )
			//{
			//	if (i != 5) { //PE: We can always enable 5 again.
			//		//PE: pMesh->pTextures[i].dwStage not used so stages must be in correct order in the shaders.
			//		ID3D11ShaderResourceView* lpTexture = GetImagePointerView(pMesh->pTextures[i].iImageID);
			//		m_pImmediateContext->PSSetShaderResources(i, 1, &lpTexture);
			//	}
			//}
			for ( int i = 0; i < pMesh->dwTextureCount; i++ )
			{
				if (i != 5) { //PE: We can always enable 5 again.
					//PE: pMesh->pTextures[i].dwStage not used so stages must be in correct order in the shaders.
					//special -123 mode means the textureref was overwritten (for animation to object texture)
					ID3D11ShaderResourceView* lpTexture = NULL;
					if ( pMesh->pTextures[i].iImageID == -123 )
						lpTexture = pMesh->pTextures[i].pTexturesRefView;
					else
						lpTexture = GetImagePointerView ( pMesh->pTextures[i].iImageID );
					m_pImmediateContext->PSSetShaderResources ( i, 1, &lpTexture );
				}
			}

			//PE: Debug.
			//dwObjectNumber
			/*
			for (int cl = 1; cl <= g.entityelementlist; cl++)
			{
				if (t.entityelement[cl].obj == pObject->dwObjectNumber) {
					if (t.entityelement[cl].draw_call_obj  > 0 ) {
						GGHANDLE pSurfColor = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName("SurfColor");
						if (pSurfColor)
						{
							GGVECTOR4 vec4 = GGVECTOR4(1.0f, 0.0f, 0.0f, 1.0f);
							pSurfColor->AsVector()->SetFloatVector((float*)&vec4);
						}
					}
					break;
				}
			}
			*/

			//GGHANDLE pSurfColor = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName("SurfColor");
			//if (pSurfColor)
			//{
			//	GGVECTOR4 vec4 = GGVECTOR4(1.0f, 0.0f, 0.0f, 1.0f);
			//	pSurfColor->AsVector()->SetFloatVector((float*)&vec4);
			//}

			#endif
			
			// see if we have an index buffer
			if ( bSkipDrawNow==false )
			{

				if ( pMesh->pIndices )
				{
					// if multimaterial mesh
					if ( pMesh->bUseMultiMaterial && bLocalOverrideAllTexturesAndEffects==false )
					{
						// draw several indexed primitives (one for each material)
						sMultiMaterial* pMultiMaterial = pMesh->pMultiMaterial;
						for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
						{
							if ( bEffectRendering == false )
							{
								// set mesh-part texture (090217 - added support for NSG)
								#ifdef DX11
								ID3D11ShaderResourceView* lpTexture = pMultiMaterial [ dwMaterialIndex ].pTexturesRef;//GetImagePointerView ( pMesh->pTextures[i].iImageID );
								m_pImmediateContext->PSSetShaderResources ( 0, 1, &lpTexture );
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefN ) 
								{
									lpTexture = pMultiMaterial [ dwMaterialIndex ].pTexturesRefN;
									m_pImmediateContext->PSSetShaderResources ( 2, 1, &lpTexture );
								}
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefS )
								{
									lpTexture = pMultiMaterial [ dwMaterialIndex ].pTexturesRefS;
									m_pImmediateContext->PSSetShaderResources ( 3, 1, &lpTexture );
								}
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefG )
								{
									lpTexture = pMultiMaterial [ dwMaterialIndex ].pTexturesRefG;
									m_pImmediateContext->PSSetShaderResources ( 6, 1, &lpTexture );
								}
								#else
								m_pD3D->SetTexture ( 0, pMultiMaterial [ dwMaterialIndex ].pTexturesRef );
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefN ) m_pD3D->SetTexture ( 2, pMultiMaterial [ dwMaterialIndex ].pTexturesRefN );
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefS ) m_pD3D->SetTexture ( 3, pMultiMaterial [ dwMaterialIndex ].pTexturesRefS );
								if ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefG ) m_pD3D->SetTexture ( 6, pMultiMaterial [ dwMaterialIndex ].pTexturesRefG );
								#endif
							}
							else
							{
								// 150217 - now supports full range of normal mapping in shader
								#ifdef DX11
								ID3D11ShaderResourceView* lpTexture = pMultiMaterial [ dwMaterialIndex ].pTexturesRef;
								m_pImmediateContext->PSSetShaderResources ( 0, 1, &lpTexture );
								GGHANDLE diffuseHandle = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName( "DiffuseMap" );
								if ( diffuseHandle ) 
								{
									diffuseHandle->AsShaderResource()->SetResource ( pMultiMaterial [ dwMaterialIndex ].pTexturesRef );
								}
								GGHANDLE normalHandle = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName( "NormalMap" );
								if ( normalHandle ) 
								{
									normalHandle->AsShaderResource()->SetResource ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefN );
								}
								GGHANDLE specularHandle = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName( "SpecularMap" );
								if ( specularHandle ) 
								{
									specularHandle->AsShaderResource()->SetResource ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefS );
								}
								GGHANDLE glossHandle = pMesh->pVertexShaderEffect->m_pEffect->GetVariableByName( "IlluminationMap" );
								if ( glossHandle ) 
								{
									glossHandle->AsShaderResource()->SetResource ( pMultiMaterial [ dwMaterialIndex ].pTexturesRefG );
								}
								#else
								m_pD3D->SetTexture ( 0, pMultiMaterial [ dwMaterialIndex ].pTexturesRef );
								// NOTEOPT: Possible optimization to get param handles and store for quicker constant setting
								// NOTEOPT: Can move all four textures to one larger plate so only a single 'CommitChanges' call!
								GGHANDLE diffuseHandle = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName( NULL, "DiffuseMap" );
								if ( diffuseHandle ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( diffuseHandle, pMultiMaterial [ dwMaterialIndex ].pTexturesRef );
								GGHANDLE normalHandle = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName( NULL, "NormalMap" );
								if ( normalHandle ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( normalHandle, pMultiMaterial [ dwMaterialIndex ].pTexturesRefN );
								GGHANDLE specularHandle = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName( NULL, "SpecularMap" );
								if ( specularHandle ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( specularHandle, pMultiMaterial [ dwMaterialIndex ].pTexturesRefS );
								GGHANDLE glossHandle = pMesh->pVertexShaderEffect->m_pEffect->GetParameterByName( NULL, "IlluminationMap" );
								if ( glossHandle ) pMesh->pVertexShaderEffect->m_pEffect->SetTexture ( glossHandle, pMultiMaterial [ dwMaterialIndex ].pTexturesRefG );
								pMesh->pVertexShaderEffect->m_pEffect->CommitChanges( );
								#endif
							}

							// set mesh-part material and render state
							//SetMeshMaterial ( pMesh, &pMultiMaterial [ dwMaterialIndex ].mMaterial );

							// draw mesh-part
							if ( pMultiMaterial [ dwMaterialIndex ].dwPolyCount > 0 )
							{
								#ifdef DX11
								m_pImmediateContext->DrawIndexed ( pMultiMaterial [ dwMaterialIndex ].dwPolyCount * 3, pDrawBuffer->dwIndexStart + pMultiMaterial [ dwMaterialIndex ].dwIndexStart, pDrawBuffer->dwBaseVertexIndex );
								#else
								if ( FAILED ( m_pD3D->DrawIndexedPrimitive (	pDrawBuffer->dwPrimType,
																				pDrawBuffer->dwBaseVertexIndex,
																				pDrawBuffer->dwVertexStart,
																				pDrawBuffer->dwVertexCount,
																				pDrawBuffer->dwIndexStart + pMultiMaterial [ dwMaterialIndex ].dwIndexStart,
																				pMultiMaterial [ dwMaterialIndex ].dwPolyCount	) ) )
								{
									uPass=uPasses;
									break;
								}
								#endif
							}

							// add to polycount
							if ( g_pGlob ) g_pGlob->dwNumberOfPolygonsDrawn += pMultiMaterial [ dwMaterialIndex ].dwPolyCount;
							if ( g_pGlob ) g_pGlob->dwNumberOfPrimCalls++;
						}

						// restore texture settings next cycle
						m_iLastTexture=-9999999;
					}
					else
					{
						// draw an indexed primitive
						#ifdef DX11
						m_pImmediateContext->DrawIndexed ( pMesh->dwIndexCount, pDrawBuffer->dwIndexStart, pDrawBuffer->dwBaseVertexIndex );
						#else
						if ( FAILED ( m_pD3D->DrawIndexedPrimitive (	pDrawBuffer->dwPrimType,
																		pDrawBuffer->dwBaseVertexIndex,
																		pDrawBuffer->dwVertexStart,
																		pDrawBuffer->dwVertexCount,
																		pDrawBuffer->dwIndexStart,
																		pDrawBuffer->dwPrimitiveCount		) ) )
						{
							break;
						}
						#endif

						// add to polycount
						if ( g_pGlob ) g_pGlob->dwNumberOfPolygonsDrawn += pDrawBuffer->dwPrimitiveCount;
						if ( g_pGlob ) g_pGlob->dwNumberOfPrimCalls++;
					}
				}
				else
				{
					// draw a standard primitive
					#ifdef DX11
					m_pImmediateContext->Draw ( pDrawBuffer->dwPrimitiveCount*3, pDrawBuffer->dwBaseVertexIndex );
					#else
					if ( FAILED ( m_pD3D->DrawPrimitive (	pDrawBuffer->dwPrimType,
															pDrawBuffer->dwVertexStart,
															pDrawBuffer->dwPrimitiveCount				) ) )
					{
						// if fail to render, try smaller batches of primitives until we figure it out!
						DWORD dwHowManyLeft = pDrawBuffer->dwPrimitiveCount;
						DWORD dwVertexBeginData = pDrawBuffer->dwVertexStart;
						DWORD dwPrimCountBatch=65535/3;
						for ( DWORD dwI=0; dwI<=(pDrawBuffer->dwPrimitiveCount/dwPrimCountBatch); dwI++ )
						{
							DWORD dwHowManyToRender = dwPrimCountBatch;
							if ( dwHowManyLeft < dwPrimCountBatch )
								dwHowManyToRender = dwHowManyLeft;

							if ( FAILED ( m_pD3D->DrawPrimitive (	pDrawBuffer->dwPrimType,
																	dwVertexBeginData,
																	dwHowManyToRender		) ) )
								break;

							// next batch of vertex data
							dwVertexBeginData+=dwHowManyToRender*3;
							dwHowManyLeft -= dwPrimCountBatch;
						}
					}
					#endif

					// add to polycount
					if ( g_pGlob ) g_pGlob->dwNumberOfPolygonsDrawn += pDrawBuffer->dwPrimitiveCount;
					if ( g_pGlob ) g_pGlob->dwNumberOfPrimCalls++;
				}
			}

			// unbind textures from shader pass
			#ifdef DX11
			if ( 1 )
			{
				// can release extra resources if postprocess RT render targets involved
				int iClearExtraPSResSlotsForPostProcessRTs = iTextureCount;
				if ( pMesh->pVertexShaderEffect ) 
					if ( pMesh->pVertexShaderEffect->m_bUsesAtLeastOneRT==true )
						if ( iClearExtraPSResSlotsForPostProcessRTs < 5 ) 
							iClearExtraPSResSlotsForPostProcessRTs = 5;

				// release input resources
				for ( int i = 0; i < iClearExtraPSResSlotsForPostProcessRTs; i++ )
				{
					if (i != 5) {
						ID3D11ShaderResourceView *const pSRV[1] = { NULL };
						m_pImmediateContext->PSSetShaderResources(i, 1, pSRV);
					}
				}

			}
			#endif

			// end shader pass
			ShaderPassEnd ( pMesh, bEffectRendering );
		}
	}

	// finish shader
	ShaderFinish ( pMesh, pCurrentRenderTarget, pCurrentDepthTarget );

	// unbind textures from shader
	//moved further up to capture multi-pass shaders that use previous textures as input
	//#ifdef DX11
	//if ( 1 )
	//{
	//	for ( int i = 0; i < pMesh->dwTextureCount; i++ )
	//	{
	//		ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	//		m_pImmediateContext->PSSetShaderResources ( i, 1, pSRV );
	//	}
	//}
	//#endif

	// leeadd - 310506 - u62 - end pointlist session (used by PhysX plugin HW fluids)
	#ifdef DX11
	#else
	if ( pMesh->iPrimitiveType == D3DPT_POINTLIST )
	{
		// end pointlist states
		m_pD3D->SetRenderState( D3DRS_POINTSPRITEENABLE, FALSE );
	}
	#endif
		
	// okay
	return true;
}

bool CObjectManager::DrawMesh ( sMesh* pMesh )
{
	return DrawMesh ( pMesh, false );
}

int CObjectManager::SwitchRenderTargetToDepth ( int iFlag )
{
	#ifdef DX11
	// create render target if not exists
	if ( g_pMainCameraDepthTexture==NULL )
	{
		GGSURFACE_DESC desc;
		g_pGlob->pCurrentBitmapSurface->GetDesc(&desc);
		int iTryModes = 0;
		GGFORMAT dwRenderTarget = GGFMT_A8R8G8B8;
		while ( g_pMainCameraDepthTexture == NULL && iTryModes <= 6 )
		{
			if ( iTryModes == 0 ) dwRenderTarget = GGFMT_A32B32G32R32F;
			if ( iTryModes == 1 ) dwRenderTarget = GGFMT_A16B16G16R16F;
			if ( iTryModes == 2 ) dwRenderTarget = GGFMT_G32R32F;
			if ( iTryModes == 3 ) dwRenderTarget = GGFMT_R32F;
			if ( iTryModes == 4 ) dwRenderTarget = GGFMT_G16R16F;
			if ( iTryModes == 5 ) dwRenderTarget = GGFMT_R16F;
			if ( iTryModes == 6 ) dwRenderTarget = GGFMT_A8R8G8B8;
			GGSURFACE_DESC StagedDesc = { desc.Width, desc.Height, 1, 1, dwRenderTarget, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0, 0 };
			m_pD3D->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&g_pMainCameraDepthTexture );
			iTryModes++;
		}
		//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		//renderTargetViewDesc.Format = dwRenderTarget;
		//renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		//renderTargetViewDesc.Texture2D.MipSlice = 0;
		HRESULT hr = m_pD3D->CreateRenderTargetView(g_pMainCameraDepthTexture, 0, &g_pMainCameraDepthTextureSurfaceRef);
		if ( hr == S_OK )
		{
			GGFORMAT depthFormat = GetValidStencilBufferFormat(desc.Format);
			g_pMainCameraDepthStencilTexture = NULL;
			GGSURFACE_DESC bufferDesc;
			bufferDesc.ArraySize = 1;
			bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.Format = depthFormat;
			bufferDesc.Height = desc.Height;
			bufferDesc.MipLevels = 1;
			bufferDesc.MiscFlags = 0;
			bufferDesc.SampleDesc = {1,0};
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.Width = desc.Width;
			HRESULT hr = m_pD3D->CreateTexture2D(&bufferDesc, 0, &g_pMainCameraDepthStencilTexture);
			if ( hr == S_OK )
			{
				//D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
				//depthStencilViewDesc.Format = depthFormat;
				//depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				//depthStencilViewDesc.Texture2D.MipSlice = 0;
				hr = m_pD3D->CreateDepthStencilView(g_pMainCameraDepthStencilTexture, 0, &g_pMainCameraDepthStencilTextureView);
			}
		}
	}

	// copy to debug image from last usage
	if ( g_pMainCameraDepthTexture && g_pMainCameraDepthTextureSurfaceRef && rand()%100==1 )
	{
		LPGGTEXTURE pDebugSee = GetImagePointer ( 59949 );
		if ( pDebugSee )
		{
			if ( g_pMainCameraDepthTextureSurfaceRef )
			{
				#ifdef DX11
				#else
				LPGGSURFACE pShadowDebugImage;
				pDebugSee->GetSurfaceLevel ( 0, &pShadowDebugImage );
				if ( pShadowDebugImage )
				{
					HRESULT hRes = D3DXLoadSurfaceFromSurface ( pShadowDebugImage, NULL, NULL, g_pMainCameraDepthTextureSurfaceRef, NULL, NULL, D3DX_DEFAULT, 0 );
					pShadowDebugImage->Release();
				}
				#endif
			}
		}
	}

	// we channel all renders to a special depth texture render target
	m_pImmediateContext->OMSetRenderTargets ( 1, &g_pMainCameraDepthTextureSurfaceRef, g_pMainCameraDepthStencilTextureView );

	// clear if first render of this cycle
	if ( g_bFirstRenderClearsRenderTarget==false )
	{
		float ClearColor[4] = {0,0,0,0};
		m_pImmediateContext->ClearRenderTargetView(g_pMainCameraDepthTextureSurfaceRef, ClearColor);
		m_pImmediateContext->ClearDepthStencilView(g_pMainCameraDepthStencilTextureView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
		g_bFirstRenderClearsRenderTarget = true;
	}
	#else
	// create render target if not exists
	if ( g_pMainCameraDepthTexture==NULL )
	{
		D3DSURFACE_DESC desc;
		IGGSurface* pCurrentRenderTarget;
		m_pD3D->GetRenderTarget ( 0, &pCurrentRenderTarget );
		pCurrentRenderTarget->GetDesc( &desc );
		int iTryModes = 0;
		while ( g_pMainCameraDepthTexture == NULL && iTryModes <= 6 )
		{
			GGFORMAT dwRenderTarget = GGFMT_A8R8G8B8;
			if ( iTryModes == 0 ) dwRenderTarget = GGFMT_A32B32G32R32F;
			if ( iTryModes == 1 ) dwRenderTarget = GGFMT_A16B16G16R16F;
			if ( iTryModes == 2 ) dwRenderTarget = GGFMT_G32R32F;
			if ( iTryModes == 3 ) dwRenderTarget = GGFMT_R32F;
			if ( iTryModes == 4 ) dwRenderTarget = GGFMT_G16R16F;
			if ( iTryModes == 5 ) dwRenderTarget = GGFMT_R16F;
			if ( iTryModes == 6 ) dwRenderTarget = GGFMT_A8R8G8B8;
			D3DXCreateTexture( m_pD3D, desc.Width, desc.Height, 1, GGUSAGE_RENDERTARGET, dwRenderTarget, D3DPOOL_DEFAULT, &g_pMainCameraDepthTexture );
			iTryModes++;
		}
		g_pMainCameraDepthTexture->GetSurfaceLevel ( 0, &g_pMainCameraDepthTextureSurfaceRef );
		m_pD3D->CreateDepthStencilSurface( desc.Width, desc.Height, GetValidStencilBufferFormat(desc.Format), D3DMULTISAMPLE_NONE, 0, TRUE, &g_pMainCameraDepthStencilTexture, NULL );
	}

	// copy to debug image from last usage
	if ( g_pMainCameraDepthTexture && g_pMainCameraDepthTextureSurfaceRef && rand()%100==1 )
	{
		LPGGTEXTURE pDebugSee = GetImagePointer ( 59949 );
		if ( pDebugSee )
		{
			if ( g_pMainCameraDepthTextureSurfaceRef )
			{
				LPGGSURFACE pShadowDebugImage;
				pDebugSee->GetSurfaceLevel ( 0, &pShadowDebugImage );
				if ( pShadowDebugImage )
				{
					HRESULT hRes = D3DXLoadSurfaceFromSurface ( pShadowDebugImage, NULL, NULL, g_pMainCameraDepthTextureSurfaceRef, NULL, NULL, D3DX_DEFAULT, 0 );
					pShadowDebugImage->Release();
				}
			}
		}
	}

	// we channel all renders to a special depth texture render target
	m_pD3D->SetRenderTarget( 0, g_pMainCameraDepthTextureSurfaceRef );
	m_pD3D->SetDepthStencilSurface( g_pMainCameraDepthStencilTexture );

	// clear if first render of this cycle
	if ( g_bFirstRenderClearsRenderTarget==false )
	{
		GGCOLOR color = GGCOLOR_RGBA(0, 0, 0, 0);
		m_pD3D->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
		g_bFirstRenderClearsRenderTarget = true;
	}
	#endif

	// always success for now
	return 1;
}

bool CObjectManager::DrawObjectEx ( sObject* pObject )
{
	return DrawObject ( pObject, false );
}

bool CObjectManager::DrawObject ( sObject* pObject, bool bFrustrumCullMeshes )
{
	// each object resets this (instance object can set it)
	g_InstanceAlphaControlValue = 0;

	// check the object and array index value
	SAFE_MEMORY ( pObject );

	// if resource destroyed, quit now
	if ( pObject->dwObjectNumber > 0 )
		if ( g_ObjectList [ pObject->dwObjectNumber ]==NULL )
			return true;

	// skip if object is designated as invisible
	// changed this to let universe visible through for shadows
	if ( pObject->bVisible==false || pObject->bNoMeshesInObject==true ) //|| pObject->bExcludedEarly )
		return true;

	// setup the world matrix for the object
	CalcObjectWorld ( pObject );

	// Simple hardware occlusion to determine on-screen visibility (in rendered pixels)
	bool bOcclusionRenderHappened = false;
	if ( g_Occlusion.d3dQuery[pObject->dwObjectNumber]!=NULL )
	{
		if ( g_Occlusion.GetOcclusionMode()==1 )
		{
			if ( g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber]==99 )
			{
				// 99 = must be triggered by GET OBJECT OCCLUSION (not every cycle = too expensive)
				#ifdef DX11
				#else
				g_Occlusion.d3dQuery[pObject->dwObjectNumber]->Issue( D3DISSUE_BEGIN );
				#endif
				g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 1;
			}
		}
	}

	// only render if universe believes it visible (or rendering for shadow camera)
	if ( pObject->bUniverseVisible==true || g_pGlob->dwRenderCameraID>=31 )
	{
		// Occlusion Render Happened
		bOcclusionRenderHappened = true;

		// for linked objects
		GGMATRIX matSavedWorld;
		GGMATRIX matNewWorld;

		// external DLLs can disable transform it they want
		if ( !pObject->bDisableTransform )
		{
			GGSetTransform ( GGTS_WORLD, &pObject->position.matWorld );
		}
		else
		{
			// World Transform
			GGMATRIX matTranslation, matScale, matObject;
			GGMatrixTranslation ( &matTranslation, 0.0f, 0.0f, 0.0f );
			GGMatrixScaling     ( &matScale,       1.0f, 1.0f, 1.0f );
			matObject = matScale * matTranslation;
			GGSetTransform ( GGTS_WORLD, &matObject );
		}

		// an object can control its own FOV for rendering
		// 171117 - interferes with global control of FOV
		// but needed for Weapon FOV
		if ( pObject->bLockedObject==false )
		{
			if ( pObject->fFOV != m_RenderStates.fObjectFOV )
			{
				if ( pObject->fFOV == 0.0f )
					SetCameraFOV ( m_RenderStates.fStoreCameraFOV );
				else
					SetCameraFOV ( pObject->fFOV );

				m_RenderStates.fObjectFOV = pObject->fFOV;
			}
		}

		// get LOD flag from core object
		int iUsingWhichLOD = pObject->iUsingWhichLOD;

		// actual object or instance of object
		sObject* pActualObject = pObject;

		// if object uses bInstanceAlphaOverride, we might be doing a per-object shader operation
		if ( pObject->bInstanceAlphaOverride )
		{
			// allows drawmesh to know if this instance is a per-instance changer
			g_InstanceAlphaControlValue = pObject->dwInstanceAlphaOverride;
		}
		if ( pObject->pInstanceOfObject )
		{
			// get actual object via instance ptr
			pActualObject=pActualObject->pInstanceOfObject;

			// if instance uses alpha factor, apply to object
			#ifdef DX11
			#else
			if ( pObject->bInstanceAlphaOverride )
			{
				// if mesh exists with blending and argument mode, set the individual instance alpha value
				if ( pActualObject->iMeshCount > 0 )
				{
					sMesh* pMesh = pActualObject->ppMeshList [ 0 ];
					if ( pMesh->dwTextureCount > 0 )
					{
						m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP, pMesh->pTextures [ 0 ].dwBlendMode );
						m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, pMesh->pTextures [ 0 ].dwBlendArg1 );
						m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
						m_pD3D->SetRenderState( D3DRS_TEXTUREFACTOR, pObject->dwInstanceAlphaOverride );
					}
				}
 				m_RenderStates.bNoMeshAlphaFactor = true;
			}
			#endif
		}

		// store current object distance globally so DrawMesh can set Shader technique based on distance (SET OBJECT LOD TECHNIQUE)
		g_fObjectCamDistance = pObject->position.fCamDistance;

		// first identify if 'pInstanceMeshVisible' limbs should be visible/invisible
		if ( pObject->iUsingWhichLOD==-1000 )
		{
			// LIMB VISIBILITY LOD STYLE
			if ( pObject->pInstanceOfObject )
			{
				// INSTANCE OBJECT
				pObject->pInstanceMeshVisible [ pObject->iLOD0LimbIndex ] = false;
				pObject->pInstanceMeshVisible [ pObject->iLOD1LimbIndex ] = false;
				pObject->pInstanceMeshVisible [ pObject->iLOD2LimbIndex ] = false;
				if ( pObject->position.fCamDistance > pObject->fLODDistance[1] )
				{
					// furthest
					pObject->pInstanceMeshVisible [ pObject->iLOD2LimbIndex ] = true;
				}
				else
				{
					if ( pObject->position.fCamDistance > pObject->fLODDistance[0] )
					{
						// mid-way
						pObject->pInstanceMeshVisible [ pObject->iLOD1LimbIndex ] = true;
					}
					else
					{
						// closest
						pObject->pInstanceMeshVisible [ pObject->iLOD0LimbIndex ] = true;
					}
				}
			}
		}

		// run through all of the frames within the object
		if ( pActualObject->ppFrameList )
		{
			for ( int iFrame = 0; iFrame < pActualObject->iFrameCount; iFrame++ )
			{
				// if instance limb visibility hidden, skip now
				if ( pObject->pInstanceMeshVisible )
				{
					// if limb in instance hidden, skip
					if ( pObject->pInstanceMeshVisible [ iFrame ]==false )
						continue;
				}

				// get a pointer to the frame
				sFrame* pFrame = pActualObject->ppFrameList [ iFrame ];

				// 301007 - new limb excluder
				if ( pFrame==NULL ) continue;
				if ( pFrame->bExcluded==true ) continue;

				// get mesh from frame
				sMesh* pMesh = pFrame->pMesh;

				// calculate correct absolute world matrix
				CalculateAbsoluteWorldMatrix ( pObject, pFrame, pMesh );

				// if flagged, reject meshes outside of camera frustrum (bVeryEarlyObject prevents sky meshes from disappearing)
				if ( bFrustrumCullMeshes==true && pObject->bVeryEarlyObject==false )
				{
					if ( pMesh )
					{
						GGVECTOR3 vecCentre = pMesh->Collision.vecCentre;
						GGVec3TransformCoord ( &vecCentre, &vecCentre, &pFrame->matAbsoluteWorld );
						GGVECTOR3 vecDirection = vecCentre - pObject->position.vecPosition;
						GGVECTOR3 vecCamDirection = m_pCamera->vecLook;
						GGVec3Normalize ( &vecDirection, &vecDirection );
						GGVec3Normalize ( &vecCamDirection, &vecCamDirection );
						float fDot = GGVec3Dot ( &vecDirection, &vecCamDirection );
						if ( fDot<=-0.6f ) // at 200 HFOV can see these!
							continue;
					}
				}

				// draw mesh
				if ( pMesh )
				{
					// apply the final transform
					if ( !pMesh->bLinked )
					{
						// new matrix for completely custom, physics needs this for implementing it's own matrix
						if ( pFrame->bOverride )
						{
							GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &pFrame->matOverride );
						}
						else if ( !pObject->bDisableTransform )
						{
							GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &pFrame->matAbsoluteWorld );
						}
					}

					// LOD System
					sMesh* pCurrentLOD = pMesh;
					if ( pObject->iUsingWhichLOD==-1000 )
					{
						// LIMB VISIBILITY LOD STYLE
						if ( pObject->pInstanceOfObject )
						{
							// done above, before we enter frame loop
						}
						else
						{
							// ACTUAL OBJECT
							sMesh* pMeshLOD0 = pObject->ppFrameList[pObject->iLOD0LimbIndex]->pMesh;
							sMesh* pMeshLOD1 = pObject->ppFrameList[pObject->iLOD1LimbIndex]->pMesh;
							sMesh* pMeshLOD2 = pObject->ppFrameList[pObject->iLOD2LimbIndex]->pMesh;
							if ( pMeshLOD0 ) pMeshLOD0->bVisible = false;
							if ( pMeshLOD1 ) pMeshLOD1->bVisible = false;
							if ( pMeshLOD2 ) pMeshLOD2->bVisible = false;
							if ( pObject->position.fCamDistance > pObject->fLODDistance[1] )
							{
								// furthest
								if ( pMeshLOD2) pMeshLOD2->bVisible = true;
							}
							else
							{
								if ( pObject->position.fCamDistance > pObject->fLODDistance[0] )
								{
									// mid-way
									if ( pMeshLOD1 ) pMeshLOD1->bVisible = true;
								}
								else
								{
									// closest
									if ( pMeshLOD0 ) pMeshLOD0->bVisible = true;
								}
							}
							if ( pMeshLOD1==NULL && pMeshLOD2==NULL && pMeshLOD0 ) pMeshLOD0->bVisible = true;
						}
					}
					else
					{
						// MESH SELECT LOD STYLE

						// update world transform for LOD quad (always faces camera)
						GGMATRIX matQuadRotation;

						// u74b7 - moved to UpdateLayer
						if ( (pObject->iUsingOldLOD==3 || pObject->iUsingWhichLOD==3) && m_pCamera )
						{
							float dx = pObject->position.vecPosition.x - m_pCamera->vecPosition.x;
							float dz = pObject->position.vecPosition.z - m_pCamera->vecPosition.z;
							float theangle = atan2 ( dx, dz );
							GGMatrixRotationY(&matQuadRotation, theangle );
							matQuadRotation._41 = pFrame->matAbsoluteWorld._41;
							matQuadRotation._42 = pFrame->matAbsoluteWorld._42;
							matQuadRotation._43 = pFrame->matAbsoluteWorld._43;
						}

						// leeadd - U71 - determine LOD meshes (current and old (transition) if applicable)
						if ( iUsingWhichLOD==1 && pFrame->pLOD[0] ) pCurrentLOD = pFrame->pLOD[0];
						if ( iUsingWhichLOD==2 && pFrame->pLOD[1] ) pCurrentLOD = pFrame->pLOD[1];
						if ( iUsingWhichLOD==3 && pFrame->pLODForQUAD ) pCurrentLOD = pFrame->pLODForQUAD;
						sMesh* pOldLOD = NULL;
						if ( pObject->iUsingOldLOD != -1 )
						{
							// the old lod mesh
							if ( pObject->iUsingOldLOD==0 ) pOldLOD = pMesh;
							if ( pObject->iUsingOldLOD==1 && pFrame->pLOD[0] ) pOldLOD = pFrame->pLOD[0];
							if ( pObject->iUsingOldLOD==2 && pFrame->pLOD[1] ) pOldLOD = pFrame->pLOD[1];
							if ( pObject->iUsingOldLOD==3 && pFrame->pLODForQUAD ) pOldLOD = pFrame->pLODForQUAD;

							// transition in progress from OLD to CURRENT
							pObject->fLODTransition += 0.03f;
							if ( pObject->fLODTransition >= 2.0f )
							{
								// end transition and restore alpha states
								pObject->fLODTransition = 0.0f;
								pObject->iUsingOldLOD = -1;
								GGCOLOR dwAlphaValueOnly = GGCOLOR_ARGB ( 255, 0, 0, 0 );
								pCurrentLOD->dwAlphaOverride = dwAlphaValueOnly;
								pCurrentLOD->bAlphaOverride = false;
								pCurrentLOD->bZWrite = true;
								pCurrentLOD->bZBiasActive = false; // U74 - 120409 - refresh each cycle for each instance
								pCurrentLOD->fZBiasDepth = 0.0f; // U74 - 120409 - refresh each cycle for each instance
								pOldLOD->dwAlphaOverride = dwAlphaValueOnly;
								pOldLOD->bAlphaOverride = false;
								pOldLOD->bZWrite = true;
								pOldLOD->bZBiasActive = false;
								pOldLOD->fZBiasDepth = 0.0f;
								pOldLOD = NULL;

								// U72 - 100109 - record alpha state of this mesh (for when instance is not being calculated, i.e. updated in actual mesh for render state change)
								if ( pObject->pInstanceOfObject ) pObject->dwInstanceAlphaOverride = dwAlphaValueOnly;
							}
							else
							{
								// change alpha level of meshes involved in transition
								if ( pObject->fLODTransition<=1.0f )
								{
									// FIRST fade in current LOD mesh
									DWORD dwAlpha = (DWORD)(pObject->fLODTransition*255);
									GGCOLOR dwAlphaValueOnly = GGCOLOR_ARGB ( dwAlpha, 0, 0, 0 );
									pCurrentLOD->dwAlphaOverride = dwAlphaValueOnly;
									pCurrentLOD->bAlphaOverride = true;
									pCurrentLOD->bTransparency = true;

									// AND first bit of fade in switch off Zwrite so the 'appearing' image goes not clip the
									// new current mesh and other objects in the area (causing the background to come through)
									if ( pObject->iUsingWhichLOD!=3 )
									{
										if ( pObject->fLODTransition < 0.5f )
											pCurrentLOD->bZWrite = false;
										else
											pCurrentLOD->bZWrite = true;

										pCurrentLOD->bZBiasActive = false;
										pCurrentLOD->fZBiasDepth = 0.0f;
									}
									else
									{
										// last LODQUAD mesh is a plane, so can adjust bias ahead to ensure it is rendered ABOVE everything
										pCurrentLOD->bZWrite = false;
										pCurrentLOD->fZBiasDepth = -g_fZBiasEpsilon;
										pCurrentLOD->bZBiasActive = true;
									}

									// and OLD LOD must stay as reset
									if ( pOldLOD )
									{
										pOldLOD->dwAlphaOverride = GGCOLOR_ARGB ( 255, 0, 0, 0 );
										pOldLOD->bAlphaOverride = false;
										pOldLOD->bZWrite = true;
										pOldLOD->bZBiasActive = false;
										pOldLOD->fZBiasDepth = 0.0f;
									}
								}
								else
								{
									// Ensure current LOD mesh is default (writing Z and no bias)
									pCurrentLOD->dwAlphaOverride = GGCOLOR_ARGB ( 255, 0, 0, 0 );
									pCurrentLOD->bAlphaOverride = false;
									pCurrentLOD->bZWrite = true;
									pCurrentLOD->bZBiasActive = false;
									pCurrentLOD->fZBiasDepth = 0.0f;

									// AND now as OLD one fades away, push zbias so NEW/CURRENT mesh has all of Z buffer opportunity
									if ( pObject->iUsingOldLOD!=3 )
									{
										// except the last LODQUAD, which needs zbias as is to do proper fade out
										pOldLOD->fZBiasDepth = g_fZBiasEpsilon * (pObject->fLODTransition-1.0f);
										pOldLOD->bZBiasActive = true;

										// AND last bit of fade out switch off Zwrite so the 'almost gone' image goes not clip the
										// new current mesh and other objects in the area (causing the background to come through)
										if ( pObject->fLODTransition > 1.5f )
											pOldLOD->bZWrite = false;
										else
											pOldLOD->bZWrite = true; // U74 - 120409 - refresh each cycle for each instance
									}
									else
									{
										// For the last LOD QUAD, make the decal fade out slower (to avoid the flick against the sky)
										pObject->fLODTransition -= 0.01f;

										// U74 - 120409 - no zbias effect
										pOldLOD->bZBiasActive = false;
										pOldLOD->fZBiasDepth = 0.0f;

										// also disable ALL zwrites from LODQUAD to avoid artefacts
										pOldLOD->bZWrite = false;
									}

									// THEN fade out old LOD mesh
									DWORD dwAlpha = (DWORD)((2.0f-pObject->fLODTransition)*255);
									GGCOLOR dwAlphaValueOnly = GGCOLOR_ARGB ( dwAlpha, 0, 0, 0 );
									pOldLOD->dwAlphaOverride = dwAlphaValueOnly;
									pOldLOD->bAlphaOverride = true;
									pOldLOD->bTransparency = true;
								}
							}
						}
						else
						{
							// U72 - 100109 - mesh not in transition, but still need the alpha state if this is an instanced object
							if ( pObject->pInstanceOfObject && pCurrentLOD )
							{
								pCurrentLOD->dwAlphaOverride = pObject->dwInstanceAlphaOverride;
								pCurrentLOD->bAlphaOverride = false;

								// 010917 - commented this out as its not relevant to latest engine and corrupts by disabled Zwrite flag state!
								//pCurrentLOD->bZWrite = true;
								//pCurrentLOD->bZBiasActive = false;
								//pCurrentLOD->fZBiasDepth = 0.0f;
							}
						}

						// if in transition, draw OLD first
						if ( pOldLOD )
						{
							if ( pObject->iUsingOldLOD==3 )
							{
								GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &matQuadRotation );
							}
							else
							{
								GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &pFrame->matAbsoluteWorld );
							}

							// draw old LOD mesh
							//DrawMesh(pOldLOD);
							DrawMesh ( pOldLOD , false , pObject, pFrame); //PE: Need the object for new dyn light to work.
							//if ( !DrawMesh ( pOldLOD ) )
							//	return false;

							// restore projection matrix
							if ( pObject->iUsingWhichLOD!=3 )
								GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &pFrame->matAbsoluteWorld );
						}

						// update world transform for LOD quad (always faces camera)
						if ( pObject->iUsingWhichLOD==3 )
							GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &matQuadRotation );
					}

					// draw the current mesh
					if ( !DrawMesh ( pCurrentLOD, (pObject->pInstanceMeshVisible!=NULL) , pObject , pFrame) )
					{
						// mesh failed to draw - catch it here to investigate strangeness
						int lee = 42;
					}
					//if ( !DrawMesh ( pCurrentLOD, (pObject->pInstanceMeshVisible!=NULL) , pObject ) )
					//	return false;

					// for linked objects
					if ( pMesh->bLinked )
					{
						if ( !pObject->bDisableTransform )
						{
							GGGetTransform ( GGTS_WORLD, &matSavedWorld );
							matNewWorld = pFrame->matOriginal * matSavedWorld;
							GGSetTransform ( GGTS_WORLD, &matNewWorld );
						}
					}
				}
			}
		}

		// if instance uses alpha factor, apply to object
		m_RenderStates.bNoMeshAlphaFactor = false;
		m_RenderStates.bIgnoreDiffuse = false;
	}

	// calculate object visibility based on hardware occlusion
	if ( g_Occlusion.d3dQuery[pObject->dwObjectNumber]!=NULL )
	{
		if ( g_Occlusion.GetOcclusionMode()==1 )
		{
			if ( g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber]==1 )
			{
				#ifdef DX11
				#else
				g_Occlusion.d3dQuery[pObject->dwObjectNumber]->Issue( D3DISSUE_END );
				#endif
				if ( bOcclusionRenderHappened==false )
				{
					// cancel query now, no render at all between issue phase means corrupt result!
					g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 0;
				}
				else
				{
					g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 2;
				}
			}
			if ( g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber]==2 )
			{
				DWORD pixelsVisible = 0;
				#ifdef DX11
				#else
				HRESULT dwResult = g_Occlusion.d3dQuery[pObject->dwObjectNumber]->GetData((void *)&pixelsVisible, sizeof(DWORD), 0);
				if ( dwResult==S_OK )
				{
					// get pixels result
					g_Occlusion.dwQueryValue[pObject->dwObjectNumber] = pixelsVisible;
					g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 0;
				}
				else
				{
					if ( dwResult!=S_FALSE )
					{
						// cancel whole thing if error returned
						g_Occlusion.iQueryBusyStage[pObject->dwObjectNumber] = 0;
					}
				}
				#endif
			}
		}
	}

	// sorted, return back
	return true;
}

bool CObjectManager::PostDrawRestores ( void )
{
	#ifdef DX11
	//  ensure FOV is restored
	if ( g_pGlob->dwRenderCameraID == 0 )
	{
		if ( m_RenderStates.fObjectFOV != 0.0f )
		{
			SetCameraFOV ( m_RenderStates.fStoreCameraFOV );
			m_RenderStates.fObjectFOV = 0.0f;
		}
	}
	#else
	// cleanup render states before leave draw process
	m_pD3D->SetPixelShader ( 0 );

	// fixed function blending restores
	DWORD dwMaxTextureStage = 7;
	for ( DWORD dwTextureStage = 0; dwTextureStage < dwMaxTextureStage; dwTextureStage++ )
	{
		// texture filter modes
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MAGFILTER, GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MINFILTER, GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( dwTextureStage, D3DSAMP_MIPFILTER, GGTEXF_LINEAR );

		// texture coordinate data
		m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_TEXCOORDINDEX, dwTextureStage );

		// texture blending modes
		if ( dwTextureStage==0 )
		{
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, GGTOP_MODULATE );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG1, GGTA_TEXTURE );
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLORARG2, GGTA_DIFFUSE );
		}
		else
		{
			m_pD3D->SetTextureStageState ( dwTextureStage, D3DTSS_COLOROP, GGTOP_DISABLE );
		}
	}

    // must always restore ambient level (for layers)
	m_pD3D->SetRenderState ( D3DRS_AMBIENT, m_RenderStates.dwAmbientColor );

	// leeadd - 140304 - ensure FOV is restored
	if ( m_RenderStates.fObjectFOV != 0.0f )
	{
		SetCameraFOV ( m_RenderStates.fStoreCameraFOV );
		m_RenderStates.fObjectFOV = 0.0f;
	}

    if ( g_pGlob && g_pGlob->iFogState == 1 )
    {
	    m_pD3D->SetRenderState ( D3DRS_FOGENABLE, TRUE );
    	m_pD3D->SetRenderState ( D3DRS_FOGCOLOR, m_RenderStates.dwFogColor );
    }
	#endif

    // okay
	return true;
}

bool CObjectManager::Reset ( void )
{
	// set values to default
	m_iCurrentTexture		= -22000000;	// current texture being used
	m_iLastTexture			= -22000000;	// last texture being used
	m_dwLastTextureCount	= 0;
	m_dwCurrentShader		= 0;
	m_dwCurrentFVF			= 0;			// current FVF
	m_dwLastShader			= 0;
	m_dwLastFVF				= 0;			// previous FVF
	m_bUpdateVertexDecs		= true;			// reset update vertex settings 
	m_bUpdateStreams		= true;			// reset update stream sources every frame

	// leefix - 200303 - reset cullmode from various stencil effects
	m_RenderStates.dwCullDirection				= m_RenderStates.dwGlobalCullDirection;
	m_RenderStates.bCull						= true;
	m_RenderStates.iCullMode					= 0;	
	#ifdef DX11
	#else
	m_pD3D->SetRenderState ( D3DRS_CULLMODE,	m_RenderStates.dwCullDirection );
	#endif

	// U75 - 070410 - added new render state to control whether entire render is blanked to a color
	tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
	if ( m_Camera_Ptr ) m_RenderStates.dwOverrideAllWithColor = m_Camera_Ptr->dwForegroundColor;
	m_RenderStates.bOverrideAllTexturesAndEffects = false;
	if ( m_RenderStates.dwOverrideAllWithColor != 0 ) m_RenderStates.bOverrideAllTexturesAndEffects = true;
	m_RenderStates.bOverriddenClipPlaneforHLSL = false;

	return true;
}

void CObjectManager::UpdateViewProjForMotionBlur(void)
{
	// 270515 - record current viewproj (for working out previous viewproj for motion blur)
	tagCameraData* m_pCamera = (tagCameraData*)GetCameraInternalData ( 0 );
	if ( m_pCamera )
	{
		g_matPreviousViewProj = g_matThisViewProj;
		GGMATRIX matView, matProj;
	    g_matThisViewProj = m_pCamera->matView * m_pCamera->matProjection;
		g_matThisCameraView = m_pCamera->matView;
	}
}

float rx, ry, rz;
float px = -999.0f;
float py,pz;

void CObjectManager::UpdateInitOnce ( void )
{
	// can skip some operations when in VR (reflection camera and right eye camera)
	bool bSkipRepeatedWorkloads = false;
	if ( g_pGlob->dwRenderCameraID == 3 || g_pGlob->dwRenderCameraID == 7 )
		bSkipRepeatedWorkloads = true;

	// ensure that the D3D device is valid
	if ( !m_pD3D )
		return;

	// replace any buffers if object modified (from mesh size change or limb mods)
	if ( !m_ObjectManager.ReplaceAllFlaggedObjectsInBuffers() )
		return;

	//PE: Start mesh light system.
	start_mesh_light();

	// Sort is sort of expensive
	if ( bSkipRepeatedWorkloads == false ) SortTextureList();

    // get camera data into member variable
	m_pCamera = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwRenderCameraID );
	if ( m_pCamera )
	{
		// projection matrix
		GGMATRIX matProj = m_pCamera->matProjection;
		GGSetTransform ( GGTS_PROJECTION, &matProj );

		// regular or reflected (clip mode has reflection modes at 2 and 4)
		if ( m_pCamera->iClipPlaneOn==2 || m_pCamera->iClipPlaneOn==4 )
		{
			// Reflect camera view in clip plane (mirror)
			GGMATRIX matView, matReflect;
			GGMatrixReflect ( &matReflect, &m_pCamera->planeClip );
			GGMatrixMultiply ( &matView, &matReflect, &m_pCamera->matView );
			GGSetTransform ( GGTS_VIEW, &matView );
		}
		else
		{
			// Regular camera view
			GGMATRIX matView = m_pCamera->matView;
			GGSetTransform ( GGTS_VIEW, &matView );
		}
	}

	// setup the viewing frustum data
	if ( !SetupFrustum ( 0.0f ) )
		return;

	// only need to do this for camera zero and six really
	if ( bSkipRepeatedWorkloads == false )
	{
		// setup the visibility list (sort of expensive)
		if ( !SortVisibilityList ( ) )
			return;

		// update only those that are visible
		if ( !m_ObjectManager.UpdateOnlyVisible() )
			return;

		// refresh all data in VB (from any vertex changes in objects)
		if ( !m_ObjectManager.UpdateAllObjectsInBuffers() )
			return;
	}

	// can render even earlier in pipeline, so this can be flagged to happen earlier in UpdateOnce
	g_bScenePrepared = false;

	// 270515 - helps clear depth texture render target each cycle
	g_bFirstRenderClearsRenderTarget = false;

	// some objects need to be rendered before ANYTHING (camera, light, matrix, terrain even stencilstart)
	// ideal for things like sky boxes that do not interfere with the Z buffer for scene fidelity
	if ( g_bRenderVeryEarlyObjects==true )
	{
		// Prepare main render
		UpdateInit();

		// prepare initial scene states
		if ( !PreSceneSettings ( ) )
			return;

		// render VERY EARLY objects (such as sky)
		UpdateLayer ( -1 );

		// scene prepared
		g_bScenePrepared = true;
	}

	// okay
	return;
}

bool CObjectManager::UpdateInit ( void )
{
	// ensure that the D3D device is valid
	if ( !m_pD3D )
		return false;

	// reset values
	if ( !Reset ( ) )
		return false;
	
	// okay
	return true;
}

// calculate distance from object to camera
float CObjectManager::CalculateObjectDistanceFromCamera ( sObject* pObject )
{
    // u74b8 - If the camera isn't selected, just use main camera
    //if (!m_pCamera) m_pCamera = 
	// lee - 280714 - always use camera zero for distance check (as can only store one in object structure)
	tagCameraData* pCamera = (tagCameraData*) GetCameraInternalData(0);

    // u74b8 - Use current camera position as the start point, or 0,0,0 for a locked object
    GGVECTOR3 vecCamPos = (pObject->bLockedObject) ? GGVECTOR3(0,0,0) : pCamera->vecPosition;

    // u74b8 - Follow glued objects until you reach the end of the glue-chain, or a
    // glued object that no longer exists.
    while ( pObject->position.iGluedToObj != 0 )
    {
        if (g_ObjectList [ pObject->position.iGluedToObj ] == NULL)
        {
            // Glued to an object that does not exist, so break the chain
            pObject->position.iGluedToObj = 0;
            break;
        }

        pObject = g_ObjectList [ pObject->position.iGluedToObj ];
    }

    float fdx = pObject->position.vecPosition.x - vecCamPos.x;
    float fdy = pObject->position.vecPosition.y - vecCamPos.y;
    float fdz = pObject->position.vecPosition.z - vecCamPos.z;

	return sqrt ( (fdx * fdx) + (fdy * fdy) + (fdz * fdz) );
}

bool CObjectManager::UpdateLayer ( int iLayer )
{
	// if resources destroyed, quit now
	if ( GetSortedObjectVisibleList()==NULL )
		return true;

	// prepare render states to draw
	if ( !PreDrawSettings ( ) )
		return false;

	//PE: Set current layer being redered.
	setlayer_mesh_light(iLayer);

	bool Status = UpdateLayerInner(iLayer);

    // restore render states after draw
	if ( !PostDrawRestores ( ) )
		return false;

    return Status;
}

bool CObjectManager::UpdateLayerInner ( int iLayer )
{
	// work vars
	int iObject = 0;
	static int iOnlyOneSortPerSync = 0;
	bool bUseStencilWrite=false;
	GGVECTOR3 vecShadowPos;

	// if sync mask override active, reject any drawing activity
	if ( g_dwSyncMaskOverride == 0 ) return true;

    // Get camera information for LOD and distance calculation
	// ensure rendercamera of 31-34 selects mask for camera 31 (shadow camera)
    DWORD dwCurrentCameraBit;
	if ( g_pGlob->dwRenderCameraID<31 )
		dwCurrentCameraBit = 1 << g_pGlob->dwRenderCameraID;
	else
		dwCurrentCameraBit = 1 << 31;

	// run through all visible objects and draw them (unrolled for performance)
	switch ( iLayer )
	{
	case -1 : // Very Early Objects (rendered even before StencilStart)
		{
			iOnlyOneSortPerSync = 0;
			// choose camera to render sky (and other early objects) to (used by cube map generator)
			int iPreferredCamera = 0;
			if ( g_pGlob->dwRenderCameraID == 30 ) iPreferredCamera = 30;

			// reset to default camera range for noz and locked objects
			float fCurrentNearRange = 0.0f;
			float fCurrentFarRange = 0.0f;
			bool bCameraRangeAndProjectionChanged = false;
			if ( g_pGlob->dwRenderCameraID != 6 && g_pGlob->dwRenderCameraID != 7 )
			{
				// except for cameras 6 and 7 which are VR eye cameras and have their own projection matrix (which should not be overwritten by SetCameraRange)
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData( iPreferredCamera );
				fCurrentNearRange = m_Camera_Ptr->fZNear;
				fCurrentFarRange = m_Camera_Ptr->fZFar;
				SetCameraRange ( iPreferredCamera, 1, 70000 );
				bCameraRangeAndProjectionChanged = true;
			}
			if ( ! m_vVisibleObjectEarly.empty() )
			{
				for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectEarly.size(); ++iIndex )
				{
					sObject* pObject = m_vVisibleObjectEarly [ iIndex ];

					// leeadd - 211006 - u63 - ignore objects whose masks reject the current camera
					if ( (pObject->dwCameraMaskBits & dwCurrentCameraBit)==0 )
						continue;

					// leeadd - 240106 - if any LOD activity
					// u74b8 - avoid recalculation of distance if already sorted by distance
					if ( pObject->bHadLODNeedCamDistance && g_eGlobalSortOrder != E_SORT_BY_DEPTH)
						pObject->position.fCamDistance = CalculateObjectDistanceFromCamera ( pObject );

					// call the draw function
					DrawObject ( pObject, true );
				}
			}
			// restore camera range
			if ( bCameraRangeAndProjectionChanged == true )
			{
				// except for cameras 6 and 7 which are VR eye cameras and have their own projection matrix (which should not be overwritten by SetCameraRange)
				SetCameraRange ( iPreferredCamera, fCurrentNearRange, fCurrentFarRange );
			}
		}
		break;

	case 0 : // Main Layer

        if ( ! m_vVisibleObjectStandard.empty() )
        {
            for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectStandard.size(); ++iIndex )
            {
                sObject* pObject = m_vVisibleObjectStandard [ iIndex ];

				// ignore objects whose masks reject the current camera
				if ( (pObject->dwCameraMaskBits & dwCurrentCameraBit)==0 )
					continue;

				// do not render static objects
				if ( pObject->bStatic )
					continue;

				// or stencil objects
				if ( pObject->bReflectiveObject )
					continue;

				// leeadd - 240106 - if any LOD activity
                // u74b8 - avoid recalculation of distance if already sorted by distance
                if ( pObject->bHadLODNeedCamDistance && g_eGlobalSortOrder != E_SORT_BY_DEPTH)
					pObject->position.fCamDistance = CalculateObjectDistanceFromCamera ( pObject );

				// call the draw function
				//if ( !DrawObject ( pObject, false ) )
				//	return false;
				DrawObject ( pObject, false );
			}
        }
        break;

	case 3 : // Overlay Ghost Layer (in stages)

        if ( ! m_vVisibleObjectTransparent.empty() )
        {
			if (iOnlyOneSortPerSync++ == 0) {

				// leeadd - 021205 - new feature which can divide transparent depth-sorted objects by a water
				// line so everything below is rendered, then the water, then everything at normal surface order
				bool bWaterPlaneDivision = false;
				float fWaterPlaneDivisionY = 99999.99f;

				// get list of ghosted objects for depth sort
				for (DWORD iIndex = 0; iIndex < m_vVisibleObjectTransparent.size(); ++iIndex)
				{
					sObject* pObject = m_vVisibleObjectTransparent[iIndex];
					if (!pObject) continue;

					// leeadd - 211006 - u63 - ignore objects whose masks reject the current camera
					if ((pObject->dwCameraMaskBits & dwCurrentCameraBit) == 0)
						continue;

					// calculate distance from object to camera (fills fCamDistance)
					if (pObject->bTransparencyWaterLine == true)
					{
						/*
													// leeadd - 021205 - transparent object water line, using HEIGHY (Y) as ordering (great for water planes)
													if ( pObject->position.vecPosition.y < fWaterPlaneDivisionY )
														fWaterPlaneDivisionY = pObject->position.vecPosition.y;

													// set as furthest surface distance object (and first to be drawn after underwater objs)
													// u74b8 - use the current camera
													if (g_eGlobalSortOrder != E_SORT_BY_DEPTH)
														pObject->position.fCamDistance = CalculateObjectDistanceFromCamera ( pObject );
													else
														pObject->position.fCamDistance = 0.0f;

													pObject->position.fCamDistance += m_pCamera->fZFar;
						*/

						//PE: Another try :)
						//PE: Distance to water object (0,600,0) can be huge (we have default camera in center), so below waterline objects dont trigger.
						//PE: If we just set it to m_pCamera->fZFar , they will trigger as they use (+= m_pCamera->fZFar)
						//PE: This fix some of the problems and allow pObject->bRenderBeforeWater "SetObjectTransparency(Obj,8)".

						if (pObject->position.vecPosition.y < fWaterPlaneDivisionY)
							fWaterPlaneDivisionY = pObject->position.vecPosition.y;

						pObject->position.fCamDistance = m_pCamera->fZFar;

						bWaterPlaneDivision = true;
					}
					else
					{
						// regular object vs camera distance
						// u74b8 - If already sorted by distance, then we've also already
						//         calculated the camera distance and there's no need to do it again.
						if (g_eGlobalSortOrder != E_SORT_BY_DEPTH)
						{
							pObject->position.fCamDistance = CalculateObjectDistanceFromCamera(pObject);
						}
					}
				}

				// if some objs underwater division, increase their cam distances so they ALL are drawn first (in same order)
				// OR some objects have a distance offset to affect draw order
				for (DWORD iIndex = 0; iIndex < m_vVisibleObjectTransparent.size(); ++iIndex)
				{
					// get obj ptr
					sObject* pObject = m_vVisibleObjectTransparent[iIndex];

					// record original cam distance value
					pObject->position.fStoreLastCamDistance = pObject->position.fCamDistance;

					// for waterline object itself
					if (bWaterPlaneDivision == true)
					{
						//if(  t.terrain.vegetationshaderindex)
						if (pObject->bTransparencyWaterLine == false)
						{
							// for LARGE explosion decals, above water bangs are forced to render FIRST
							float fBaseOfObj = pObject->position.vecPosition.y;
							if (fBaseOfObj < fWaterPlaneDivisionY)
							{
								// u74b8 - use the current camera
								pObject->position.fCamDistance += m_pCamera->fZFar;
							}
							//else if (pObject->bRenderBeforeWater) {
							//	pObject->position.fCamDistance += m_pCamera->fZFar;
							//}
						}
					}

					// also apply any artificial distance to object to affect draw order
					pObject->position.fCamDistance += pObject->fArtificialDistanceOffset;
				}

				// u74b7 - sort objects by distance, replaced bubblesort with STL sort
				std::sort(m_vVisibleObjectTransparent.begin(), m_vVisibleObjectTransparent.end(), OrderByReverseCameraDistance());
			}

            // draw in correct back to front order
            for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectTransparent.size(); ++iIndex )
            {
                sObject* pObject = m_vVisibleObjectTransparent [ iIndex ];
				if ( !pObject ) 
					continue;

				// restore original cam distance value (changed for depth reordering)
				pObject->position.fCamDistance = pObject->position.fStoreLastCamDistance;

				// u75b9 - fixes Transparency and Camera Mask problem
                if (( pObject->dwCameraMaskBits & dwCurrentCameraBit ) == 0)
                    continue;

                //if ( !DrawObject ( pObject, false ) )
                //    return false;
                DrawObject ( pObject, false );
            }

	    }
		// end ghost layer
		break;

	case 4 : // Overlay Locked/NoZ Layer
	    
        if ( ! m_vVisibleObjectNoZDepth.empty() )
        {
			// reset to default camera range for noz and locked objects
			tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData( 0 );
			float fCurrentNearRange = m_Camera_Ptr->fZNear;
			float fCurrentFarRange = m_Camera_Ptr->fZFar;
			bool bCameraRangeAndProjectionChanged = false;
			if ( g_pGlob->dwRenderCameraID != 6 && g_pGlob->dwRenderCameraID != 7 )
			{
				// except for cameras 6 and 7 which are VR eye cameras and have their own projection matrix (which should not be overwritten by SetCameraRange)
				SetCameraRange ( 0, 2.5f, 70000.0f ); // forces HUD weapons not to blur/DOF/MOTION/etc
				bCameraRangeAndProjectionChanged = true;
			}

			// record weapon/jetpack techniques (so can restore after cutout technique)
			DWORD dwOldWeaponBasicShaderPtr, dwOldWeaponBoneShaderPtr, dwOldJetpackBoneShaderPtr;
			sEffectItem* pWeaponBasic = NULL;
			sEffectItem* pWeaponBone = NULL;
			sEffectItem* pJetpackBone = NULL;
			if ( g_weaponbasicshadereffectindex > 0 ) 
			{
				if ( GetEffectExist(g_weaponbasicshadereffectindex) ) 
				{
					dwOldWeaponBasicShaderPtr = GetEffectTechniqueEx ( g_weaponbasicshadereffectindex );
					pWeaponBasic = m_EffectList [ g_weaponbasicshadereffectindex ];
				}
			}
			if ( g_weaponboneshadereffectindex > 0 ) 
			{
				if ( GetEffectExist(g_weaponboneshadereffectindex) ) 
				{
					dwOldWeaponBoneShaderPtr = GetEffectTechniqueEx ( g_weaponboneshadereffectindex );
					pWeaponBone = m_EffectList [ g_weaponboneshadereffectindex ];
				}
			}
			if ( g_jetpackboneshadereffectindex > 0 )
			{
				if ( GetEffectExist(g_jetpackboneshadereffectindex) ) 
				{
					dwOldJetpackBoneShaderPtr = GetEffectTechniqueEx ( g_jetpackboneshadereffectindex );
					pJetpackBone = m_EffectList [ g_jetpackboneshadereffectindex ];
				}
			}

			// prefer to render objects that are marked as 'not' transparent, not locked and bNewZLayerObject as true
			// this will allow muzzle flashes to render 'before' the weapon (and smoke to render AFTER as smoke transparency set to 6)
			for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectNoZDepth.size(); ++iIndex )
			{
				sObject* pObject = m_vVisibleObjectNoZDepth [ iIndex ];
				if ( !pObject ) continue;

				// ignore objects whose masks reject the current camera
				if ( (pObject->dwCameraMaskBits & dwCurrentCameraBit)==0 )
					continue;

				// only render not-transparent, not locked and bNewZLayerObject true objects
				bool bRenderObject = false;
				if ( pObject->bTransparentObject==false && pObject->bLockedObject==false && pObject->bNewZLayerObject==true )
					bRenderObject=true;

				// only if object should be rendered
				if ( !bRenderObject )
					continue;

				// skip if IS weapon/jetpack
				bool bIsWeaponOrJetPack = false;
				sObject* pActualObject = pObject;
				if ( pObject->pInstanceOfObject ) pActualObject = pObject->pInstanceOfObject;
				if ( pActualObject->ppMeshList )
				{
					if ( pWeaponBasic && pWeaponBasic->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBasic->pEffectObj ) bIsWeaponOrJetPack = true;
					if ( pWeaponBone && pWeaponBone->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBone->pEffectObj ) bIsWeaponOrJetPack = true;
					if ( pJetpackBone && pJetpackBone->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pJetpackBone->pEffectObj ) bIsWeaponOrJetPack = true;
				}
				if ( bIsWeaponOrJetPack == true )
					continue;

				// draw
				DrawObject ( pObject, false );
			}

			// WEAPON RENDERING
			// for NoZDepth pass, two cycles one for depthcutout and regular
			// and hard find weapon shaders that have cutoutdepth techniques
			for ( int iCutOutPassIndex = 0; iCutOutPassIndex < 2; iCutOutPassIndex++ )
			{
				if ( g_weaponbasicshadereffectindex+g_weaponboneshadereffectindex+g_jetpackboneshadereffectindex > 0 )
				{
					if ( iCutOutPassIndex == 0 )
					{
						if ( g_weaponbasicshadereffectindex > 0 ) if ( GetEffectExist(g_weaponbasicshadereffectindex) ) SetEffectTechnique ( g_weaponbasicshadereffectindex, "CutOutDepth" );
						if ( g_weaponboneshadereffectindex > 0 ) if ( GetEffectExist(g_weaponboneshadereffectindex) ) SetEffectTechnique ( g_weaponboneshadereffectindex, "CutOutDepth" );
						if ( g_jetpackboneshadereffectindex > 0 ) if ( GetEffectExist(g_jetpackboneshadereffectindex) ) SetEffectTechnique ( g_jetpackboneshadereffectindex, "CutOutDepth" );
					}
					if ( iCutOutPassIndex == 1 )
					{
						if ( g_weaponbasicshadereffectindex > 0 ) if ( GetEffectExist(g_weaponbasicshadereffectindex) ) SetEffectTechniqueEx ( g_weaponbasicshadereffectindex, dwOldWeaponBasicShaderPtr );
						if ( g_weaponboneshadereffectindex > 0 ) if ( GetEffectExist(g_weaponboneshadereffectindex) ) SetEffectTechniqueEx ( g_weaponboneshadereffectindex, dwOldWeaponBoneShaderPtr );
						if ( g_jetpackboneshadereffectindex > 0 ) if ( GetEffectExist(g_jetpackboneshadereffectindex) ) SetEffectTechniqueEx ( g_jetpackboneshadereffectindex, dwOldJetpackBoneShaderPtr );
					}
				}
				for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectNoZDepth.size(); ++iIndex )
				{
					sObject* pObject = m_vVisibleObjectNoZDepth [ iIndex ];
					if ( !pObject ) continue;

					// ignore objects whose masks reject the current camera
					if ( (pObject->dwCameraMaskBits & dwCurrentCameraBit)==0 )
						continue;

					// skip if not weapon/jetpack
					bool bIsWeaponOrJetPack = false;
					if ( pObject->ppMeshList )
					{
						if ( pWeaponBasic && pWeaponBasic->pEffectObj > 0 && pObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBasic->pEffectObj ) bIsWeaponOrJetPack = true;
						if ( pWeaponBone && pWeaponBone->pEffectObj > 0 && pObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBone->pEffectObj ) bIsWeaponOrJetPack = true;
						if ( pJetpackBone && pJetpackBone->pEffectObj > 0 && pObject->ppMeshList[0]->pVertexShaderEffect == pJetpackBone->pEffectObj ) bIsWeaponOrJetPack = true;
					}
					if ( bIsWeaponOrJetPack == false )
						continue;

					// draw weapon/jetpack
					DrawObject ( pObject, false );
				}
			}

			// NOZDEPTH LOOP (locked and nozdepth)
			// ( Pass A-ZDepth : Pass B-NoZDepth )
			bool bClearZBuffer = false;
			for ( int iPass = 0; iPass < 2; iPass++ )
			{
				// LOCKED STAGE
				float fCurrentFOV = 0.0f;
				bool bResetCamera = false;
				GGMATRIX matCurrentCameraView;
				for ( DWORD iIndex = 0; iIndex < m_vVisibleObjectNoZDepth.size(); ++iIndex )
				{
					sObject* pObject = m_vVisibleObjectNoZDepth [ iIndex ];
					if ( !pObject ) continue;

					// ignore objects whose masks reject the current camera
					if ( (pObject->dwCameraMaskBits & dwCurrentCameraBit)==0 )
						continue;

					// only render nozdepth objects on second pass
					bool bRenderObject = false;
					if ( iPass==0 && pObject->bNewZLayerObject==false )
					{
						// object has zdepth pass 1
						bRenderObject=true;
					}
					if ( iPass==1 && pObject->bNewZLayerObject==true )
					{
						// object has no zdepth pass 2
						bRenderObject=true;
						if ( bClearZBuffer==false )
						{
							// clear zbuffer
							#ifdef DX11
							//interferes with SAO
							//m_pImmediateContext->ClearDepthStencilView(g_pGlob->pCurrentDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
							#else
							m_pD3D->Clear ( 0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
							#endif
							bClearZBuffer=true;
						}
					}

					// only if object should be rendered
					if ( !bRenderObject )
						continue;

					// skip if IS weapon/jetpack
					bool bIsWeaponOrJetPack = false;
					sObject* pActualObject = pObject;
					if ( pObject->pInstanceOfObject ) pActualObject = pObject->pInstanceOfObject;
					if ( pActualObject->ppMeshList )
					{
						if ( pWeaponBasic && pWeaponBasic->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBasic->pEffectObj ) bIsWeaponOrJetPack = true;
						if ( pWeaponBone && pWeaponBone->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pWeaponBone->pEffectObj ) bIsWeaponOrJetPack = true;
						if ( pJetpackBone && pJetpackBone->pEffectObj > 0 && pActualObject->ppMeshList[0]->pVertexShaderEffect == pJetpackBone->pEffectObj ) bIsWeaponOrJetPack = true;
					}
					if ( bIsWeaponOrJetPack == true )
						continue;

					// do not render not-transparent, not locked and bNewZLayerObject true objects (did this earlier before weapon renders)
					if ( pObject->bTransparentObject==false && pObject->bLockedObject==false && pObject->bNewZLayerObject==true )
						continue;

					// locked objects
					if ( pObject->bLockedObject )
					{
						// reset camera
						if ( bResetCamera==false )
						{
							// Store current camera
							GGGetTransform ( GGTS_VIEW, &matCurrentCameraView );

							// record current FOV, and set default FOV
							tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData( 0 );
							fCurrentFOV = m_Camera_Ptr->fFOV;
							if ( (m_Camera_Ptr->dwCameraSwitchBank & 1) == 0 )
								SetCameraFOV ( GGToDegree(3.14159265358979323846f/2.905f) );

							// Use Default unmodified camera
							GGMATRIX matDefaultCameraView;
							GGMatrixIdentity ( &matDefaultCameraView );
							matDefaultCameraView._11 = 1; 
							matDefaultCameraView._12 = 0; 
							matDefaultCameraView._13 = 0;
							matDefaultCameraView._21 = 0; 
							matDefaultCameraView._22 = 1; 
							matDefaultCameraView._23 = 0;
							matDefaultCameraView._31 = 0;
							matDefaultCameraView._32 = 0; 
							matDefaultCameraView._33 = 1;

							// Assign new default camera
							GGSetTransform ( GGTS_VIEW, &matDefaultCameraView );

							// clear zbuffer
							#ifdef DX11
							//interferes with SAO
							//m_pImmediateContext->ClearDepthStencilView(g_pGlob->pCurrentDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
							#else
							m_pD3D->Clear ( 0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
							#endif

							// New camera established
							bResetCamera=true;
						}
					}
					else
					{
						if ( bResetCamera==true )
						{
							// Restore camera view
							GGSetTransform ( GGTS_VIEW, &matCurrentCameraView );
							SetCameraFOV ( fCurrentFOV );
							bResetCamera=false;
						}
					}

					// draw
					DrawObject ( pObject, false );
				}
				if ( bResetCamera )
				{
					// Restore camera view
					GGSetTransform ( GGTS_VIEW, &matCurrentCameraView );
					bResetCamera=false;

					// restore FOV if Locked Object set it (replaced in DBO system wioth better method)
					SetCameraFOV ( fCurrentFOV );
				}
			}

			// restore saved camera range
			if ( bCameraRangeAndProjectionChanged == false )
			{
				SetCameraRange ( 0, fCurrentNearRange, fCurrentFarRange );
			}
		}
		break;
	}

	// okay
	return true;
}

bool CObjectManager::Update ( void )
{
	// leeadd - U71 - can render even earlier in pipeline, so this can be flagged to happen earlier in UpdateOnce
	if ( g_bScenePrepared==false )
	{
		// Prepare main render
		UpdateInit();

		// prepare initial scene states
		if ( !PreSceneSettings ( ) )
			return false;

		// scene prepared
		g_bScenePrepared = true;
	}
	
	// Main layer render
	UpdateLayer ( 0 );

	// okay
	return true;
}

bool CObjectManager::UpdateGhostLayer ( void )
{
	// lee - 050406 - u6rc6 - overlay render layer (ghost used to be part of UpdateNoZLayer)
	Reset();
	UpdateLayer ( 3 );
	return true;
}

bool CObjectManager::UpdateNoZLayer ( void )
{
	// Must reset when return to manager
	Reset();

	// Overlay render layer (lock, nozdepth)
	UpdateLayer ( 4 );


	//PE: End mesh light system.
	end_mesh_light();

	// okay
	return true;
}

void CObjectManager::SetGlobalShadowsOn ( void )
{
	m_bGlobalShadows = true;
}

void CObjectManager::SetGlobalShadowsOff ( void )
{
	m_bGlobalShadows = false;
}

int CObjectManager::GetVisibleObjectCount ( void )
{
	// 301007 - new function
	return m_iVisibleObjectCount;
}

sObject** CObjectManager::GetSortedObjectVisibleList ( void )
{
	// 301007 - new function
	return m_ppSortedObjectVisibleList;
}

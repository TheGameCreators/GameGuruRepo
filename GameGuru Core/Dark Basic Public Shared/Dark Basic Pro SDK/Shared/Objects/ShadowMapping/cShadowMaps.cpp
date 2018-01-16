//
// Shadow Mapping Control Object
//

#include "cShadowMaps.h"
#include "CLightC.h"

#define FOURCC_NULL ((GGFORMAT)(MAKEFOURCC('N','U','L','L')))

static const GGVECTOR4 g_vFLTMAX = GGVECTOR4( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
static const GGVECTOR4 g_vFLTMIN = GGVECTOR4(  -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX );
static const GGVECTOR4 g_vHalfVector = GGVECTOR4(  0.5f, 0.5f, 0.5f, 0.5f );
static const GGVECTOR4 g_vMultiplySetzwToZero = GGVECTOR4(  1.0f, 1.0f, 0.0f, 0.0f );
static const GGVECTOR4 g_vZero = GGVECTOR4(  0.0f, 0.0f, 0.0f, 0.0f );

extern LPGG m_pDX;
extern bool g_bInEditor;
extern int g_HideDistantShadows;

// FPSCamera Constructor
CFirstPersonCamera::CFirstPersonCamera()
{
}

//--------------------------------------------------------------------------------------
// Initialize the Manager.  The manager performs all the work of caculating the render 
// paramters of the shadow, creating the D3D resources, rendering the shadow, and rendering
// the actual scene.
//--------------------------------------------------------------------------------------
CascadedShadowsManager::CascadedShadowsManager () 
{
};

//--------------------------------------------------------------------------------------
// Call into deallocator.  
//--------------------------------------------------------------------------------------
CascadedShadowsManager::~CascadedShadowsManager() 
{
};

//--------------------------------------------------------------------------------------
// Create the resources, compile shaders, etc.
// The rest of the resources are create in the allocator when the scene changes.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::Init (	CFirstPersonCamera* pViewerCamera,
										CFirstPersonCamera* pLightCamera,
										CascadeConfig* pCascadeConfig )
{
	// Result var
    HRESULT hr = S_OK;

	// Store previous config settings
    m_CopyOfCascadeConfig = *pCascadeConfig;        

    // Initialize m_iBufferSize to 0 to trigger a reallocate on the first frame.   
    m_CopyOfCascadeConfig.m_iBufferSize = 0;

    // Save a pointer to cascade config.  Each frame we check our copy against the pointer.
    m_pCascadeConfig = pCascadeConfig;
    
	// Initial scene is maximum area possible
    m_vSceneAABBMin = g_vFLTMAX; 
    m_vSceneAABBMax = g_vFLTMIN;

	// terrain scene for Reloaded is (512*100)x(512*100)
    m_vSceneAABBMin = GGVECTOR4(-1000,-1000,-1000,0);
    m_vSceneAABBMax = GGVECTOR4((512*100)+1000,20000,(512*100)+1000,0);

	// assign eye and light cameras
    m_pViewerCamera = pViewerCamera;          
    m_pLightCamera = pLightCamera;         

	// clear some ptrs
	m_pCascadedShadowMapDepthSurface = NULL;
	memset ( m_pEffectParam, 0, sizeof(m_pEffectParam) );

    return hr;
}


//--------------------------------------------------------------------------------------
// These resources must be reallocated based on GUI control settings change.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::DestroyAndDeallocateShadowResources() 
{
    return S_OK;
}

//--------------------------------------------------------------------------------------
// These settings must be recreated based on GUI control.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::ReleaseAndAllocateNewShadowResources( LPGGEFFECT pEffectPtr )
{
    // If any of these 3 paramaters was changed, we must reallocate the D3D resources.
    HRESULT hr = S_OK;
    if( m_CopyOfCascadeConfig.m_nCascadeLevels != m_pCascadeConfig->m_nCascadeLevels 
        || m_CopyOfCascadeConfig.m_ShadowBufferFormat != m_pCascadeConfig->m_ShadowBufferFormat 
        || m_CopyOfCascadeConfig.m_iBufferSize != m_pCascadeConfig->m_iBufferSize )
    {
        m_CopyOfCascadeConfig = *m_pCascadeConfig;        

		tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( 0 );
		if ( m_Camera_Ptr )
		{
			GGSURFACE_DESC surfacedesc;
			if ( m_Camera_Ptr->pCameraToImageSurface )
			{
				m_Camera_Ptr->pCameraToImageSurface->GetDesc(&surfacedesc);
			}
		}

		// create depth surface for shadow renderer
		SAFE_RELEASE ( m_pCascadedShadowMapDepthSurface );
		DWORD dwSurfaceWidth = m_CopyOfCascadeConfig.m_iBufferSize;
		DWORD dwSurfaceHeight = m_CopyOfCascadeConfig.m_iBufferSize;
		#ifdef DX11
		// DirectX 11 shadow targets are depth-only, no color render target (so no need for depth stencil)
		#else
		bool bSizeValid = false;
		while ( bSizeValid == false )
		{
			HRESULT hRes = m_pD3D->CreateDepthStencilSurface( dwSurfaceWidth, dwSurfaceHeight,
														GGFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE,
														&m_pCascadedShadowMapDepthSurface, NULL );
			if ( hRes == GG_OK )
			{
				bSizeValid = true;
			}
			else
			{
				m_CopyOfCascadeConfig.m_iBufferSize /= 2;
				dwSurfaceWidth = m_CopyOfCascadeConfig.m_iBufferSize;
				dwSurfaceHeight = m_CopyOfCascadeConfig.m_iBufferSize;
				if ( dwSurfaceWidth <= 1024 ) bSizeValid = true;
			}
		}
		#endif

		// create render targets for shadow map cascade targets
		for ( int c = 0; c < m_CopyOfCascadeConfig.m_nCascadeLevels; c++ )
		{
			m_depthTexture[c] = new DepthTexture(m_pDX);
			m_depthTexture[c]->createTexture(m_pD3D, dwSurfaceWidth, dwSurfaceHeight);
		}

		// get handles to this shader to place shadow ptrs
		if ( pEffectPtr )
		{
			#ifdef DX11
			m_depthHandle[0] = pEffectPtr->GetVariableByName( "DepthMapTX1" );
			m_depthHandle[1] = pEffectPtr->GetVariableByName( "DepthMapTX2" );
			m_depthHandle[2] = pEffectPtr->GetVariableByName( "DepthMapTX3" );
			m_depthHandle[3] = pEffectPtr->GetVariableByName( "DepthMapTX4" );
			m_depthHandle[4] = pEffectPtr->GetVariableByName( "DepthMapTX5" );
			m_depthHandle[5] = pEffectPtr->GetVariableByName( "DepthMapTX6" );
			m_depthHandle[6] = pEffectPtr->GetVariableByName( "DepthMapTX7" );
			m_depthHandle[7] = pEffectPtr->GetVariableByName( "DepthMapTX8" );
			for ( int i = 0; i < 8; i++ )
				if ( !m_depthHandle[i]->IsValid() ) 
					m_depthHandle[i] = NULL;
			#else
			m_depthHandle[0] = pEffectPtr->GetParameterByName( NULL, "DepthMapTX1" );
			m_depthHandle[1] = pEffectPtr->GetParameterByName( NULL, "DepthMapTX2" );
			m_depthHandle[2] = pEffectPtr->GetParameterByName( NULL, "DepthMapTX3" );
			m_depthHandle[3] = pEffectPtr->GetParameterByName( NULL, "DepthMapTX4" );
			#endif
		}
    }
    return hr;
}

//--------------------------------------------------------------------------------------
// This function takes the camera's projection matrix and returns the 8
// points that make up a view frustum.
// The frustum is scaled to fit within the Begin and End interval paramaters.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::CreateFrustumPointsFromCascadeInterval( float fCascadeIntervalBegin, 
                                                        FLOAT fCascadeIntervalEnd, 
                                                        GGMATRIX &vProjection,
                                                        GGVECTOR4* pvCornerPointsWorld ) 
{
    Frustum vViewFrust;
    ComputeFrustumFromProjection( &vViewFrust, &vProjection );
    vViewFrust.Near = fCascadeIntervalBegin;
    vViewFrust.Far = fCascadeIntervalEnd;

    static const XMVECTORU32 vGrabY = {0x00000000,0xFFFFFFFF,0x00000000,0x00000000};
    static const XMVECTORU32 vGrabX = {0xFFFFFFFF,0x00000000,0x00000000,0x00000000};

    GGVECTOR4 vRightTop = GGVECTOR4(vViewFrust.RightSlope,vViewFrust.TopSlope,1.0f,1.0f);
    GGVECTOR4 vLeftBottom = GGVECTOR4(vViewFrust.LeftSlope,vViewFrust.BottomSlope,1.0f,1.0f);
    GGVECTOR4 vNear = GGVECTOR4(vViewFrust.Near,vViewFrust.Near,vViewFrust.Near,1.0f);
    GGVECTOR4 vFar = GGVECTOR4(vViewFrust.Far,vViewFrust.Far,vViewFrust.Far,1.0f);
    GGVECTOR4 vRightTopNear = XMVectorMultiply( vRightTop, vNear );
    GGVECTOR4 vRightTopFar = XMVectorMultiply( vRightTop, vFar );
    GGVECTOR4 vLeftBottomNear = XMVectorMultiply( vLeftBottom, vNear );
    GGVECTOR4 vLeftBottomFar = XMVectorMultiply( vLeftBottom, vFar );

    pvCornerPointsWorld[0] = vRightTopNear;
    pvCornerPointsWorld[1] = XMVectorSelectLee( vRightTopNear, vLeftBottomNear, 0 );
    pvCornerPointsWorld[2] = vLeftBottomNear;
    pvCornerPointsWorld[3] = XMVectorSelectLee( vRightTopNear, vLeftBottomNear,1 );

    pvCornerPointsWorld[4] = vRightTopFar;
    pvCornerPointsWorld[5] = XMVectorSelectLee( vRightTopFar, vLeftBottomFar, 0 );
    pvCornerPointsWorld[6] = vLeftBottomFar;
    pvCornerPointsWorld[7] = XMVectorSelectLee( vRightTopFar ,vLeftBottomFar, 1 );

}

//--------------------------------------------------------------------------------------
// Used to compute an intersection of the orthographic projection and the Scene AABB
//--------------------------------------------------------------------------------------
struct Triangle 
{
    GGVECTOR4 pt[3];
    BOOL culled;
};


//--------------------------------------------------------------------------------------
// Computing an accurate near and flar plane will decrease surface acne and Peter-panning.
// Surface acne is the term for erroneous self shadowing.  Peter-panning is the effect where
// shadows disappear near the base of an object.
// As offsets are generally used with PCF filtering due self shadowing issues, computing the
// correct near and far planes becomes even more important.
// This concept is not complicated, but the intersection code is.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::ComputeNearAndFar( FLOAT& fNearPlane, 
                                        FLOAT& fFarPlane, 
                                        GGVECTOR4 vLightCameraOrthographicMin, 
                                        GGVECTOR4 vLightCameraOrthographicMax, 
                                        GGVECTOR4* pvPointsInCameraView ) 
{

    // Initialize the near and far planes
    fNearPlane = FLT_MAX;
    fFarPlane = -FLT_MAX;
    
    Triangle triangleList[16];
    INT iTriangleCnt = 1;

    triangleList[0].pt[0] = pvPointsInCameraView[0];
    triangleList[0].pt[1] = pvPointsInCameraView[1];
    triangleList[0].pt[2] = pvPointsInCameraView[2];
    triangleList[0].culled = false;

    // These are the indices used to tesselate an AABB into a list of triangles.
    static const INT iAABBTriIndexes[] = 
    {
        0,1,2,  1,2,3,
        4,5,6,  5,6,7,
        0,2,4,  2,4,6,
        1,3,5,  3,5,7,
        0,1,4,  1,4,5,
        2,3,6,  3,6,7 
    };

    INT iPointPassesCollision[3];

    // At a high level: 
    // 1. Iterate over all 12 triangles of the AABB.  
    // 2. Clip the triangles against each plane. Create new triangles as needed.
    // 3. Find the min and max z values as the near and far plane.
    
    //This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.
    
    float fLightCameraOrthographicMinX = XMVectorGetX( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxX = XMVectorGetX( vLightCameraOrthographicMax ); 
    float fLightCameraOrthographicMinY = XMVectorGetY( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxY = XMVectorGetY( vLightCameraOrthographicMax );
    
    for( INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter ) 
    {

        triangleList[0].pt[0] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 0 ] ];
        triangleList[0].pt[1] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 1 ] ];
        triangleList[0].pt[2] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 2 ] ];
        iTriangleCnt = 1;
        triangleList[0].culled = FALSE;

        // Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
        //add them to the list.
        for( INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter ) 
        {

            FLOAT fEdge;
            INT iComponent;
            
            if( frustumPlaneIter == 0 ) 
            {
                fEdge = fLightCameraOrthographicMinX; // todo make float temp
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 1 ) 
            {
                fEdge = fLightCameraOrthographicMaxX;
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 2 ) 
            {
                fEdge = fLightCameraOrthographicMinY;
                iComponent = 1;
            } 
            else 
            {
                fEdge = fLightCameraOrthographicMaxY;
                iComponent = 1;
            }

            for( INT triIter=0; triIter < iTriangleCnt; ++triIter ) 
            {
                // We don't delete triangles, so we skip those that have been culled.
                if( !triangleList[triIter].culled ) 
                {
                    INT iInsideVertCount = 0;
                    GGVECTOR4 tempOrder;
                    // Test against the correct frustum plane.
                    // This could be written more compactly, but it would be harder to understand.
                    
                    if( frustumPlaneIter == 0 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) >
                                XMVectorGetX( vLightCameraOrthographicMin ) ) 
                            { 
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 1 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetX( vLightCameraOrthographicMax ) )
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            { 
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 2 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) > 
                                XMVectorGetY( vLightCameraOrthographicMin ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetY( vLightCameraOrthographicMax ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }

                    // Move the points that pass the frustum test to the begining of the array.
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    if( iPointPassesCollision[2] && !iPointPassesCollision[1] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[1];   
                        triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
                        triangleList[triIter].pt[2] = tempOrder;
                        iPointPassesCollision[1] = TRUE;            
                        iPointPassesCollision[2] = FALSE;                        
                    }
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    
                    if( iInsideVertCount == 0 ) 
                    { // All points failed. We're done,  
                        triangleList[triIter].culled = true;
                    }
                    else if( iInsideVertCount == 1 ) 
                    {// One point passed. Clip the triangle against the Frustum plane
                        triangleList[triIter].culled = false;
                        
                        GGVECTOR4 vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
                        GGVECTOR4 vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];
                        
                        // Find the collision ratio.
                        FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex( triangleList[triIter].pt[0], iComponent ) ;
                        // Calculate the distance along the vector as ratio of the hit ratio to the component.
                        FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert1, iComponent );
                        FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert2, iComponent );
                        // Add the point plus a percentage of the vector.
                        vVert0ToVert1 *= fDistanceAlongVector01;
                        vVert0ToVert1 += triangleList[triIter].pt[0];
                        vVert0ToVert2 *= fDistanceAlongVector02;
                        vVert0ToVert2 += triangleList[triIter].pt[0];

                        triangleList[triIter].pt[1] = vVert0ToVert2;
                        triangleList[triIter].pt[2] = vVert0ToVert1;

                    }
                    else if( iInsideVertCount == 2 ) 
                    { // 2 in  // tesselate into 2 triangles
                        

                        // Copy the triangle\(if it exists) after the current triangle out of
                        // the way so we can override it with the new triangle we're inserting.
                        triangleList[iTriangleCnt] = triangleList[triIter+1];

                        triangleList[triIter].culled = false;
                        triangleList[triIter+1].culled = false;
                        
                        // Get the vector from the outside point into the 2 inside points.
                        GGVECTOR4 vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
                        GGVECTOR4 vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];
                        
                        // Get the hit point ratio.
                        FLOAT fHitPointTime_2_0 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent );
                        FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex( vVert2ToVert0, iComponent );
                        // Calcaulte the new vert by adding the percentage of the vector plus point 2.
                        vVert2ToVert0 *= fDistanceAlongVector_2_0;
                        vVert2ToVert0 += triangleList[triIter].pt[2];
                        
                        // Add a new triangle.
                        triangleList[triIter+1].pt[0] = triangleList[triIter].pt[0];
                        triangleList[triIter+1].pt[1] = triangleList[triIter].pt[1];
                        triangleList[triIter+1].pt[2] = vVert2ToVert0;
                        
                        //Get the hit point ratio.
                        FLOAT fHitPointTime_2_1 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent ) ;
                        FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex( vVert2ToVert1, iComponent );
                        vVert2ToVert1 *= fDistanceAlongVector_2_1;
                        vVert2ToVert1 += triangleList[triIter].pt[2];
                        triangleList[triIter].pt[0] = triangleList[triIter+1].pt[1];
                        triangleList[triIter].pt[1] = triangleList[triIter+1].pt[2];
                        triangleList[triIter].pt[2] = vVert2ToVert1;
                        // Cncrement triangle count and skip the triangle we just inserted.
                        ++iTriangleCnt;
                        ++triIter;

                    
                    }
                    else 
                    { // all in
                        triangleList[triIter].culled = false;

                    }
                }// end if !culled loop            
            }
        }
        for( INT index=0; index < iTriangleCnt; ++index ) 
        {
            if( !triangleList[index].culled ) 
            {
                // Set the near and far plan and the min and max z values respectivly.
                for( int vertind = 0; vertind < 3; ++ vertind ) 
                {
                    float fTriangleCoordZ = XMVectorGetZ( triangleList[index].pt[vertind] );
                    if( fNearPlane > fTriangleCoordZ ) 
                    {
                        fNearPlane = fTriangleCoordZ;
                    }
                    if( fFarPlane  <fTriangleCoordZ ) 
                    {
                        fFarPlane = fTriangleCoordZ;
                    }
                }
            }
        }
    }    

}


//--------------------------------------------------------------------------------------
// This function converts the "center, extents" version of an AABB into 8 points.
//--------------------------------------------------------------------------------------
void CascadedShadowsManager::CreateAABBPoints( GGVECTOR4* vAABBPoints, GGVECTOR4 vCenter, GGVECTOR4 vExtents )
{
	static GGVECTOR4 vExtentsMap[8];
	vExtentsMap[0] = GGVECTOR4(1.0f, 1.0f, -1.0f, 1.0f);
    vExtentsMap[1] = GGVECTOR4(-1.0f, 1.0f, -1.0f, 1.0f); 
    vExtentsMap[2] = GGVECTOR4(1.0f, -1.0f, -1.0f, 1.0f);
    vExtentsMap[3] = GGVECTOR4(-1.0f, -1.0f, -1.0f, 1.0f);
    vExtentsMap[4] = GGVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
    vExtentsMap[5] = GGVECTOR4(-1.0f, 1.0f, 1.0f, 1.0f); 
    vExtentsMap[6] = GGVECTOR4(1.0f, -1.0f, 1.0f, 1.0f);
    vExtentsMap[7] = GGVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f);
    
    for( INT index = 0; index < 8; ++index ) 
    {
        vAABBPoints[index] = XMVectorMultiplyAdd(vExtentsMap[index], vExtents, vCenter ); 
    }

}

//--------------------------------------------------------------------------------------
// This function is where the real work is done. We determin the matricies and constants used in 
// shadow generation and scene generation.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::InitFrame ( LPGGEFFECT pEffectPtr  )
{
    ReleaseAndAllocateNewShadowResources( pEffectPtr );

    // Camera view and projection
	float fDeterminant;
    GGMATRIX matViewCameraProjection;
	GGGetTransform ( GGTS_PROJECTION, &matViewCameraProjection );
    GGMATRIX matViewCameraView;
	GGGetTransform ( GGTS_VIEW, &matViewCameraView );
	m_pViewerCamera->m_fNear = 0.0f;
	m_pViewerCamera->m_fFar = 5000.0f;//3000.0f;
	m_pViewerCamera->m_mCameraView = matViewCameraView;
	GGMatrixInverse ( &m_pViewerCamera->m_mCameraWorld, &fDeterminant, &matViewCameraView );

	// light position and direction
	tagLightData* m_Light_Ptr = (tagLightData*)GetLightData ( 0 );
	tagLightVector vecEye = m_Light_Ptr->light.Position;
	tagLightVector vecAt = m_Light_Ptr->light.Direction;
	vecAt.x *= 100.0f;
	vecAt.y *= 100.0f;
	vecAt.z *= 100.0f;
	vecAt.x += vecEye.x;
	vecAt.y += vecEye.y;
	vecAt.z += vecEye.z;
	GGMATRIX matLightCameraLookAt;
	GGMatrixLookAtLH ( &matLightCameraLookAt, (GGVECTOR3*)&vecEye, (GGVECTOR3*)&vecAt, &GGVECTOR3(0,1,0) );
	GGMATRIX matLightCameraView;
	matLightCameraView = matLightCameraLookAt;
	m_pLightCamera->m_mCameraView = matLightCameraView;
	GGMatrixInverse ( &m_pLightCamera->m_mCameraWorld, &fDeterminant, &matLightCameraView );

	// feed in DX11 values to trace math of code below:
	GGGetTransform ( GGTS_PROJECTION, &matViewCameraProjection );
	GGGetTransform ( GGTS_VIEW, &matViewCameraView );
	m_vSceneAABBMin.w = 1.0000000;
	m_vSceneAABBMax.w = 1.0000000;

    GGMATRIX matInverseViewCamera;
	GGMatrixInverse( &matInverseViewCamera, &fDeterminant, &matViewCameraView );

    // Convert from min max representation to center extents represnetation.
    // This will make it easier to pull the points out of the transformation.
    GGVECTOR4 vSceneCenter = m_vSceneAABBMin + m_vSceneAABBMax;
    vSceneCenter = XMVectorMultiply ( vSceneCenter, g_vHalfVector );//  *= g_vHalfVector;
    GGVECTOR4 vSceneExtents = m_vSceneAABBMax - m_vSceneAABBMin;
    vSceneExtents = XMVectorMultiply ( vSceneExtents, g_vHalfVector );//*= g_vHalfVector;    

    GGVECTOR4 vSceneAABBPointsLightSpace[8];
    // This function simply converts the center and extents of an AABB into 8 points
    CreateAABBPoints( vSceneAABBPointsLightSpace, vSceneCenter, vSceneExtents );
    // Transform the scene AABB to Light space.
    for( int index =0; index < 8; ++index ) 
    {
		GGVec4Transform( &vSceneAABBPointsLightSpace[index], &vSceneAABBPointsLightSpace[index], &matLightCameraView ); 
    }

    FLOAT fFrustumIntervalBegin, fFrustumIntervalEnd;
    GGVECTOR4 vLightCameraOrthographicMin;  // light space frustrum aabb 
    GGVECTOR4 vLightCameraOrthographicMax;
    FLOAT fCameraNearFarRange = m_pViewerCamera->GetFarClip() - m_pViewerCamera->GetNearClip();
       
    GGVECTOR4 vWorldUnitsPerTexel = g_vZero; 

    // We loop over the cascades to calculate the orthographic projection for each cascade.
    for( INT iCascadeIndex=0; iCascadeIndex < m_CopyOfCascadeConfig.m_nCascadeLevels; ++iCascadeIndex ) 
    {
        // Calculate the interval of the View Frustum that this cascade covers. We measure the interval 
        // the cascade covers as a Min and Max distance along the Z Axis.
        if( m_eSelectedCascadesFit == FIT_TO_CASCADES ) 
        {
            // Because we want to fit the orthogrpahic projection tightly around the Cascade, we set the Mimiumum cascade 
            // value to the previous Frustum end Interval
            if( iCascadeIndex==0 ) 
				fFrustumIntervalBegin = 0.0f;
            else 
				fFrustumIntervalBegin = (FLOAT)m_iCascadePartitionsZeroToOne[ iCascadeIndex - 1 ];
        } 
        else 
        {
            // In the FIT_TO_SCENE technique the Cascades overlap eachother.  In other words, interval 1 is coverd by
            // cascades 1 to 8, interval 2 is covered by cascades 2 to 8 and so forth.
            fFrustumIntervalBegin = 0.0f;
        }

        // Scale the intervals between 0 and 1. They are now percentages that we can scale with.
        fFrustumIntervalEnd = (FLOAT)m_iCascadePartitionsZeroToOne[ iCascadeIndex ];        
        fFrustumIntervalBegin/= (FLOAT)m_iCascadePartitionsMax;
        fFrustumIntervalEnd/= (FLOAT)m_iCascadePartitionsMax;
        fFrustumIntervalBegin = fFrustumIntervalBegin * fCameraNearFarRange;
        fFrustumIntervalEnd = fFrustumIntervalEnd * fCameraNearFarRange;
        GGVECTOR4 vFrustumPoints[8];

        // This function takes the began and end intervals along with the projection matrix and returns the 8
        // points that repreresent the cascade Interval
        CreateFrustumPointsFromCascadeInterval( fFrustumIntervalBegin, fFrustumIntervalEnd, 
            matViewCameraProjection, vFrustumPoints );

        vLightCameraOrthographicMin = g_vFLTMAX;
        vLightCameraOrthographicMax = g_vFLTMIN;

        GGVECTOR4 vTempTranslatedCornerPoint;
        // This next section of code calculates the min and max values for the orthographic projection.
        for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
        {
            // Transform the frustum from camera view space to world space.
			GGVec4Transform ( &vFrustumPoints[icpIndex], &vFrustumPoints[icpIndex], &matInverseViewCamera );

            // Transform the point from world space to Light Camera Space.
			GGVec4Transform ( &vTempTranslatedCornerPoint, &vFrustumPoints[icpIndex], &matLightCameraView );

            // Find the closest point.
            vLightCameraOrthographicMin = XMVectorMin ( vTempTranslatedCornerPoint, vLightCameraOrthographicMin );
            vLightCameraOrthographicMax = XMVectorMax ( vTempTranslatedCornerPoint, vLightCameraOrthographicMax );
        }

        // This code removes the shimmering effect along the edges of shadows due to
        // the light changing to fit the camera.
        if( m_eSelectedCascadesFit == FIT_TO_SCENE ) 
        {
            // Fit the ortho projection to the cascades far plane and a near plane of zero. 
            // Pad the projection to be the size of the diagonal of the Frustum partition. 
            // 
            // To do this, we pad the ortho transform so that it is always big enough to cover 
            // the entire camera view frustum.
            GGVECTOR4 vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
            //vDiagonal = XMVector4Length( vDiagonal ); // !!!! should be XMVector3Length (and lots of XM converted here which I avoided)
            // The bound is the length of the diagonal of the frustum interval.
            //FLOAT fCascadeBound = XMVectorGetX( vDiagonal );
            FLOAT fCascadeBound = GGVec4Length( &vDiagonal );
			vDiagonal.x = fCascadeBound; //silly XNA behaviour!
			vDiagonal.y = fCascadeBound;
			vDiagonal.z = fCascadeBound;
			vDiagonal.w = fCascadeBound;
            
            // The offset calculated will pad the ortho projection so that it is always the same size 
            // and big enough to cover the entire cascade interval.
            GGVECTOR4 vBoarderOffset = XMVectorMultiply ( ( vDiagonal - 
                                        ( vLightCameraOrthographicMax - vLightCameraOrthographicMin ) ),
                                        g_vHalfVector );

            // Set the Z and W components to zero.
            vBoarderOffset = XMVectorMultiply ( vBoarderOffset, g_vMultiplySetzwToZero );
            
            // Add the offsets to the projection.
            vLightCameraOrthographicMax += vBoarderOffset;
            vLightCameraOrthographicMin -= vBoarderOffset;

            // The world units per texel are used to snap the shadow the orthographic projection
            // to texel sized increments.  This keeps the edges of the shadows from shimmering.
            FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)m_CopyOfCascadeConfig.m_iBufferSize;
            vWorldUnitsPerTexel = XMVectorSet( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f ); 
        } 
		else
		{
            // The world units per texel are used to snap the shadow the orthographic projection
            // to texel sized increments.  This keeps the edges of the shadows from shimmering.
            GGVECTOR4 vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
            FLOAT fCascadeBound = GGVec4Length( &vDiagonal );
            FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)m_CopyOfCascadeConfig.m_iBufferSize;
            vWorldUnitsPerTexel = XMVectorSet( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f ); 
		}
        float fLightCameraOrthographicMinZ = XMVectorGetZ( vLightCameraOrthographicMin );

        if( m_bMoveLightTexelSize ) 
        {
            // We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
            // This is a matter of integer dividing by the world space size of a texel
            vLightCameraOrthographicMin = XMVectorDivide ( vLightCameraOrthographicMin, vWorldUnitsPerTexel );
            vLightCameraOrthographicMin = XMVectorFloor( vLightCameraOrthographicMin );
            vLightCameraOrthographicMin = XMVectorMultiply ( vLightCameraOrthographicMin, vWorldUnitsPerTexel );
            vLightCameraOrthographicMax = XMVectorDivide ( vLightCameraOrthographicMax, vWorldUnitsPerTexel );
            vLightCameraOrthographicMax = XMVectorFloor( vLightCameraOrthographicMax );
            vLightCameraOrthographicMax = XMVectorMultiply ( vLightCameraOrthographicMax, vWorldUnitsPerTexel );
        }

        // These are the unconfigured near and far plane values.  They are purposly awful to show 
        // how important calculating accurate near and far planes is.
        FLOAT fNearPlane = 0.0f;
        FLOAT fFarPlane = 5000.0f;

        if( m_eSelectedNearFarFit == FIT_NEARFAR_AABB ) 
        {
            GGVECTOR4 vLightSpaceSceneAABBminValue = g_vFLTMAX;  // world space scene aabb 
            GGVECTOR4 vLightSpaceSceneAABBmaxValue = g_vFLTMIN;       
            // We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the  
            // light space AABB can be used for the near and far plane. This is easier than intersecting the scene with the AABB
            // and in some cases provides similar results.
            for(int index=0; index< 8; ++index) 
            {
                vLightSpaceSceneAABBminValue = XMVectorMin( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue );
                vLightSpaceSceneAABBmaxValue = XMVectorMax( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue );
            }

            // The min and max z values are the near and far planes.
            fNearPlane = XMVectorGetZ( vLightSpaceSceneAABBminValue );
            fFarPlane = XMVectorGetZ( vLightSpaceSceneAABBmaxValue );
        } 
        else if( m_eSelectedNearFarFit == FIT_NEARFAR_SCENE_AABB 
            || m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING ) 
        {
            // By intersecting the light frustum with the scene AABB we can get a tighter bound on the near and far plane.
            ComputeNearAndFar( fNearPlane, fFarPlane, vLightCameraOrthographicMin, 
                vLightCameraOrthographicMax, vSceneAABBPointsLightSpace );
            if (m_eSelectedNearFarFit == FIT_NEARFAR_PANCAKING )
            {
                if ( fLightCameraOrthographicMinZ > fNearPlane )
                {
                    fNearPlane = fLightCameraOrthographicMinZ;
                }
            }
        } 

        // Craete the orthographic projection for this cascade.
        GGMatrixOrthoOffCenterLH( &m_matShadowProj[ iCascadeIndex ], 
            XMVectorGetX( vLightCameraOrthographicMin ), 
            XMVectorGetX( vLightCameraOrthographicMax ), 
            XMVectorGetY( vLightCameraOrthographicMin ), 
            XMVectorGetY( vLightCameraOrthographicMax ), 
            fNearPlane, fFarPlane );

		// had to extramargin as cascade two was being used even though the required
		// deth pixels where being clipped ever so slightly (no shadow in top left corner..)
        m_fCascadePartitionsFrustum[ iCascadeIndex ] = fFrustumIntervalEnd;
    }
	m_matShadowView = *m_pLightCamera->GeViewMatrix();

    return S_OK;
}

bool g_bIgnoreFarObjects = false;
int iVisibleObjectCountCopy = 0;
std::vector< sObject* > vVisibleObjectListCopy;
std::vector< sObject* > vVisibleObjectEarlyCopy;
std::vector< sObject* > vVisibleObjectTransparentCopy;
std::vector< sObject* > vVisibleObjectNoZDepthCopy;
std::vector< sObject* > vVisibleObjectStandardCopy;

//--------------------------------------------------------------------------------------
// Render the cascades into a texture atlas.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::RenderShadowsForAllCascades ( LPGGEFFECT pEffectPtr ) 
{
	// temp
    HRESULT hr = S_OK;
    GGMATRIX dxmatWorldViewProjection;

	// early exit
	if ( m_dwMask==0 )
		return S_OK;

	// store 
	GGMATRIX matSaveW;
	GGMATRIX matSaveV;
	GGMATRIX matSaveP;
	GGGetTransform ( GGTS_WORLD, ( GGMATRIX* ) &matSaveW );
	GGGetTransform ( GGTS_VIEW, ( GGMATRIX* ) &matSaveV );
	GGGetTransform ( GGTS_PROJECTION, ( GGMATRIX* ) &matSaveP );

	// record render targets before we steal if for a bit
	#ifdef DX11
	ID3D11RenderTargetView* pCurrentRenderTarget = NULL;
	ID3D11DepthStencilView* pCurrentDepthTarget = NULL;
	pCurrentRenderTarget = g_pGlob->pCurrentRenderView;
	pCurrentDepthTarget = g_pGlob->pCurrentDepthView;
	#else
	IGGSurface* pCurrentRenderTarget = NULL;
	IGGSurface* pCurrentDepthTarget = NULL;
	m_pD3D->GetRenderTarget( 0, &pCurrentRenderTarget );
	m_pD3D->GetDepthStencilSurface( &pCurrentDepthTarget );
	#endif

	// frustum for largest shadow cascade
	bool bPrepareObjectsAndFrustum = true;

	// if have a valid depth surface
	#ifdef DX11
	UINT numViewports = 1;
	D3D11_VIEWPORT vpStored;
	m_pImmediateContext->RSGetViewports( &numViewports, &vpStored );
	if ( 1 ) // no common depth stencil needed for DirectX 11
	#else
	if ( m_pCascadedShadowMapDepthSurface )
	#endif
	{
		// set depth target for cascade shadows
		#ifdef DX11
		// DirectX 11 only has depth targets, no color target that needs a common depth target
		#else
		m_pD3D->SetDepthStencilSurface ( m_pCascadedShadowMapDepthSurface );
		#endif

		// free any pointers to shadow references before outputting new shadows
		for ( int i = 31; i >= 24; i-- )
		{
			ID3D11ShaderResourceView *const pSRV[1] = { NULL };
			m_pImmediateContext->PSSetShaderResources ( i, 1, pSRV );
		}

		// Iterate over cascades and render shadows.
		for( INT currentCascade=m_CopyOfCascadeConfig.m_nCascadeLevels-1; currentCascade >= 0; currentCascade-- ) 
		{
			// if cascade switched off via mask, skip this render sequence for this cascade
			if ( ((1<<currentCascade) & m_dwMask)==0 )
				continue;

			// shadow map needs to set render target from texture
			IGGSurface *pShadowMapTextureSurface = NULL;
			if ( m_depthTexture[currentCascade] )
			{
				if ( m_depthTexture[currentCascade]->getTextureView() )
				{
					#ifdef DX11
					m_pImmediateContext->OMSetRenderTargets ( 0, 0, m_depthTexture[currentCascade]->getTextureView() );
					#else
					m_depthTexture[currentCascade]->getTexture()->GetSurfaceLevel(0, &pShadowMapTextureSurface);
					m_pD3D->SetRenderTarget(0,pShadowMapTextureSurface);
					#endif
				}
			}

			// clear render target
			#ifdef DX11
			m_pImmediateContext->ClearDepthStencilView(m_depthTexture[currentCascade]->getTextureView(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

			// also use viewport (pretty sure this was not needed for DX9 due to surface size flexibility)
			D3D11_VIEWPORT viewport;
			viewport.Width = m_depthTexture[currentCascade]->m_iWidth;
			viewport.Height = m_depthTexture[currentCascade]->m_iHeight;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			m_pImmediateContext->RSSetViewports(1, &viewport);
			#else
			m_pD3D->Clear(0,NULL,D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, GGCOLOR_RGBA(255,128,0,255), 1, 0);
			#endif

			// if valid depth target for shadow, render to it
			#ifdef DX11
			if ( 1 ) // DirectX 11 have a shadow map view
			#else
			if ( pShadowMapTextureSurface )
			#endif
			{
				// We calculate the matrices in the Init function.
				dxmatWorldViewProjection = *m_pLightCamera->GeViewMatrix();
				GGMATRIX mShadowTexture = m_matShadowProj[currentCascade];
				GGMatrixMultiply ( &dxmatWorldViewProjection, &dxmatWorldViewProjection, &mShadowTexture );

				// update GGTS_PROJECTION (eventually passed to shader)
				GGSetTransform ( GGTS_PROJECTION, ( GGMATRIX* ) &dxmatWorldViewProjection );

				// update GGTS_WORLD (eventually passed to shader)
				GGMATRIX matIdentity;
				GGMatrixIdentity( &matIdentity );
				// The model was exported in world space, so we can pass the identity up as the world transform.
				// and also need to passify GGTS_VIEW as its now handled inside GGTS_PROJECTION
				GGSetTransform ( GGTS_VIEW, ( GGMATRIX* ) &matIdentity );
				GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &matIdentity );

				// create frustum based on GGTS_PROJECTION and GGTS_VIEW of this shadow camera cascade view
				SetupFrustum ( 0.0f );

				// for largest cascade camera (loop is in reverse order), work out all objects in shadow camera view
				if ( bPrepareObjectsAndFrustum==true )
				{
					// what objects are visible
					if ( !g_bInEditor )
					{
						iVisibleObjectCountCopy = m_ObjectManager.m_iVisibleObjectCount;
						vVisibleObjectListCopy = m_ObjectManager.m_vVisibleObjectList;
						vVisibleObjectEarlyCopy = m_ObjectManager.m_vVisibleObjectEarly;
						vVisibleObjectTransparentCopy = m_ObjectManager.m_vVisibleObjectTransparent;
						vVisibleObjectNoZDepthCopy = m_ObjectManager.m_vVisibleObjectNoZDepth;
						vVisibleObjectStandardCopy = m_ObjectManager.m_vVisibleObjectStandard;
					}
					if ( g_HideDistantShadows != 0 )
						g_bIgnoreFarObjects = true;
					else
						g_bIgnoreFarObjects = false;

					m_ObjectManager.SortVisibilityList();
					bPrepareObjectsAndFrustum = false;
				}

				// render one cascade
				DWORD dwStoredRenderCamera = g_pGlob->dwRenderCameraID;
				g_pGlob->dwRenderCameraID = 31+currentCascade; // ensure shadow cameras (internal marked with camera index 31 for occlusion arrays)
				m_ObjectManager.UpdateLayer ( 0 ); // solid objects
				m_ObjectManager.UpdateLayer ( 3 ); // transparent objects
				g_pGlob->dwRenderCameraID = dwStoredRenderCamera;

				#ifdef DX11
				// DirectX 11 debug images for shadows (cannot see depth only targets!)
				#else
				// render objects from perspective of light camera using shadow orthographical matrix
				// using 'depth' technique in shader applied to objects being rendered
				if ( m_pCascadedShadowMapDepthSurface )
				{
					LPGGTEXTURE pDebugSee = GetImagePointer ( 59951+currentCascade );
					if ( pDebugSee )
					{
						for ( int mips=0; mips<3; mips++ )
						{
							LPGGSURFACE pShadowDebugImage;
							pDebugSee->GetSurfaceLevel ( mips, &pShadowDebugImage );
							if ( pShadowDebugImage )
							{
								HRESULT hRes = D3DXLoadSurfaceFromSurface ( pShadowDebugImage, NULL, NULL, pShadowMapTextureSurface, NULL, NULL, D3DX_DEFAULT, 0 );
								pShadowDebugImage->Release();
							}
						}
					}
				}
				#endif
			}

			// release depth texture surface if used
			#ifdef DX11
			#else
			if ( pShadowMapTextureSurface ) SAFE_RELEASE ( pShadowMapTextureSurface );
			#endif
		}
	}

	// restore render targets
	#ifdef DX11
	m_pImmediateContext->OMSetRenderTargets ( 1, &pCurrentRenderTarget, pCurrentDepthTarget );
	tagCameraData* Camera = (tagCameraData*)GetCameraInternalData ( 0 );
    //D3D11_VIEWPORT vp;
	//GGVIEWPORT* pvp = &Camera->viewPort3D;
    //vp.TopLeftX = pvp->X;
    //vp.TopLeftY = pvp->Y;
    //vp.Width = (FLOAT)pvp->Width;
    //vp.Height = (FLOAT)pvp->Height;
    //vp.MinDepth = pvp->MinZ;
    //vp.MaxDepth = pvp->MaxZ;
    //m_pImmediateContext->RSSetViewports( 1, &vp );
    m_pImmediateContext->RSSetViewports( 1, &vpStored );
	#else
	m_pD3D->SetRenderTarget( 0, pCurrentRenderTarget );
	m_pD3D->SetDepthStencilSurface ( pCurrentDepthTarget );
	#endif

	// restore
	GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &matSaveW );
	GGSetTransform ( GGTS_VIEW, ( GGMATRIX* ) &matSaveV );
	GGSetTransform ( GGTS_PROJECTION, ( GGMATRIX* ) &matSaveP );

	// restore vis list selection to normal
	g_bIgnoreFarObjects = false;

	// restore object visibility list (frustum based on GGTS_PROJECTION and GGTS_VIEW of this shadow camera cascade view
	if ( bPrepareObjectsAndFrustum==false )
	{
		// put previous vis lists back
		SetupFrustum ( 0.0f );		
		if ( g_bInEditor )
		{
			m_ObjectManager.SortVisibilityList();
		}
		else
		{
			// NOTE: This restore thingie causes a flicker when adding barrel to editor and a shadow is present (need to resolve!!)
			m_ObjectManager.m_iVisibleObjectCount = iVisibleObjectCountCopy;
			m_ObjectManager.m_vVisibleObjectList = vVisibleObjectListCopy;
			m_ObjectManager.m_vVisibleObjectEarly = vVisibleObjectEarlyCopy;
			m_ObjectManager.m_vVisibleObjectTransparent = vVisibleObjectTransparentCopy;
			m_ObjectManager.m_vVisibleObjectNoZDepth = vVisibleObjectNoZDepthCopy;
			m_ObjectManager.m_vVisibleObjectStandard = vVisibleObjectStandardCopy;
		}
	}

	// done
    return hr;
}

void CascadedShadowsManager::GetShaderVar ( LPGGEFFECT pEffectPtr, LPSTR pVariableName, GGHANDLE* pVarhandle )
{
	GGHANDLE pShaderVar = NULL;
	pShaderVar = pEffectPtr->GetVariableByName ( pVariableName ); 
	if ( !pShaderVar->IsValid() ) { pShaderVar = NULL; }; 
	*pVarhandle = pShaderVar;
}

//--------------------------------------------------------------------------------------
// Render the scene.
//--------------------------------------------------------------------------------------
HRESULT CascadedShadowsManager::RenderScene ( int iEffectID, LPGGEFFECT pEffectPtr, 
                                      LPVOID prtvBackBuffer,
                                      LPVOID pdsvBackBuffer,
                                      CFirstPersonCamera* pActiveCamera,
                                      BOOL bVisualize
            ) 
{

	if ( pEffectPtr==NULL )
		return S_OK;

    // We have a seperate render state for the actual rasterization because of different depth biases and Cull modes.
    HRESULT hr = S_OK;

	// Extra HANDLES stored for faster performance
	if ( m_pEffectParam[iEffectID]==NULL )
	{
		m_pEffectParam[iEffectID] = new sEffectParam;
		#ifdef DX11
		GetShaderVar ( pEffectPtr, "m_iPCFBlurForLoopStart", &m_pEffectParam[iEffectID]->m_iPCFBlurForLoopStart );
		GetShaderVar ( pEffectPtr, "m_iPCFBlurForLoopEnd", &m_pEffectParam[iEffectID]->m_iPCFBlurForLoopEnd );
		GetShaderVar ( pEffectPtr, "m_fCascadeBlendArea", &m_pEffectParam[iEffectID]->m_fCascadeBlendArea );
		GetShaderVar ( pEffectPtr, "m_fTexelSize", &m_pEffectParam[iEffectID]->m_fTexelSize );
		GetShaderVar ( pEffectPtr, "m_fShadowBiasFromGUI", &m_pEffectParam[iEffectID]->m_fShadowBiasFromGUI );
		GetShaderVar ( pEffectPtr, "m_mShadow", &m_pEffectParam[iEffectID]->m_mShadow );
		GetShaderVar ( pEffectPtr, "m_vCascadeScale", &m_pEffectParam[iEffectID]->m_vCascadeScale );
		GetShaderVar ( pEffectPtr, "m_vCascadeOffset", &m_pEffectParam[iEffectID]->m_vCascadeOffset );
		GetShaderVar ( pEffectPtr, "m_fCascadeFrustumsEyeSpaceDepths", &m_pEffectParam[iEffectID]->m_fCascadeFrustumsEyeSpaceDepths );
		GetShaderVar ( pEffectPtr, "m_fMaxBorderPadding", &m_pEffectParam[iEffectID]->m_fMaxBorderPadding );
		GetShaderVar ( pEffectPtr, "m_fMinBorderPadding", &m_pEffectParam[iEffectID]->m_fMinBorderPadding );
		GetShaderVar ( pEffectPtr, "m_nCascadeLevels", &m_pEffectParam[iEffectID]->m_nCascadeLevels );
		GetShaderVar ( pEffectPtr, "DepthMapTX1", &m_pEffectParam[iEffectID]->DepthMapTX1 );
		GetShaderVar ( pEffectPtr, "DepthMapTX2", &m_pEffectParam[iEffectID]->DepthMapTX2 );
		GetShaderVar ( pEffectPtr, "DepthMapTX3", &m_pEffectParam[iEffectID]->DepthMapTX3 );
		GetShaderVar ( pEffectPtr, "DepthMapTX4", &m_pEffectParam[iEffectID]->DepthMapTX4 );
		GetShaderVar ( pEffectPtr, "DepthMapTX5", &m_pEffectParam[iEffectID]->DepthMapTX5 );
		GetShaderVar ( pEffectPtr, "DepthMapTX6", &m_pEffectParam[iEffectID]->DepthMapTX6 );
		GetShaderVar ( pEffectPtr, "DepthMapTX7", &m_pEffectParam[iEffectID]->DepthMapTX7 );
		GetShaderVar ( pEffectPtr, "DepthMapTX8", &m_pEffectParam[iEffectID]->DepthMapTX8 );
		#else
		m_pEffectParam[iEffectID]->m_iPCFBlurForLoopStart = pEffectPtr->GetParameterByName ( NULL, "m_iPCFBlurForLoopStart" );
		m_pEffectParam[iEffectID]->m_iPCFBlurForLoopEnd = pEffectPtr->GetParameterByName ( NULL, "m_iPCFBlurForLoopEnd" );
		m_pEffectParam[iEffectID]->m_fCascadeBlendArea = pEffectPtr->GetParameterByName ( NULL, "m_fCascadeBlendArea" );
		m_pEffectParam[iEffectID]->m_fTexelSize = pEffectPtr->GetParameterByName ( NULL, "m_fTexelSize" );
		m_pEffectParam[iEffectID]->m_fShadowBiasFromGUI = pEffectPtr->GetParameterByName ( NULL, "m_fShadowBiasFromGUI" );
		m_pEffectParam[iEffectID]->m_mShadow = pEffectPtr->GetParameterByName ( NULL, "m_mShadow" );
		m_pEffectParam[iEffectID]->m_vCascadeScale = pEffectPtr->GetParameterByName ( NULL, "m_vCascadeScale" );
		m_pEffectParam[iEffectID]->m_vCascadeOffset = pEffectPtr->GetParameterByName ( NULL, "m_vCascadeOffset" );
		m_pEffectParam[iEffectID]->m_fCascadeFrustumsEyeSpaceDepths = pEffectPtr->GetParameterByName ( NULL, "m_fCascadeFrustumsEyeSpaceDepths" );
		m_pEffectParam[iEffectID]->m_fMaxBorderPadding = pEffectPtr->GetParameterByName ( NULL, "m_fMaxBorderPadding" );
		m_pEffectParam[iEffectID]->m_fMinBorderPadding = pEffectPtr->GetParameterByName ( NULL, "m_fMinBorderPadding" );
		m_pEffectParam[iEffectID]->m_nCascadeLevels = pEffectPtr->GetParameterByName ( NULL, "m_nCascadeLevels" );
		m_pEffectParam[iEffectID]->DepthMapTX1 = pEffectPtr->GetParameterByName ( NULL, "DepthMapTX1" );
		m_pEffectParam[iEffectID]->DepthMapTX2 = pEffectPtr->GetParameterByName ( NULL, "DepthMapTX2" );
		m_pEffectParam[iEffectID]->DepthMapTX3 = pEffectPtr->GetParameterByName ( NULL, "DepthMapTX3" );
		m_pEffectParam[iEffectID]->DepthMapTX4 = pEffectPtr->GetParameterByName ( NULL, "DepthMapTX4" );
		#endif
	}
    
    // These are the for loop begin end values. 
    int m_iPCFBlurForLoopEnd = m_iPCFBlurSize / 2 +1;
    int m_iPCFBlurForLoopStart = m_iPCFBlurSize / -2;
	GGHANDLE hPCFBlurForLoopEndParam = m_pEffectParam[iEffectID]->m_iPCFBlurForLoopEnd;
	GGHANDLE hPCFBlurForLoopStartParam = m_pEffectParam[iEffectID]->m_iPCFBlurForLoopStart;

    // This is a floating point number that is used as the percentage to blur between maps.    
    float m_fCascadeBlendArea = m_fBlurBetweenCascadesAmount;
	GGHANDLE hCascadeBlendAreaParam = m_pEffectParam[iEffectID]->m_fCascadeBlendArea;

	// texel size to work out PCF spacing
    float m_fTexelSize = 1.0f / (float)m_CopyOfCascadeConfig.m_iBufferSize; 
	GGHANDLE hTexelSizeParam = m_pEffectParam[iEffectID]->m_fTexelSize;

	GGMATRIX scaleToTile;
	GGMatrixScaling( &scaleToTile, 1.0f / (float)m_pCascadeConfig->m_nCascadeLevels, 1.0, 1.0 );
	float m_fShadowBiasFromGUI = m_fPCFOffset;
	GGHANDLE hShadowBiasFromGUIParam = m_pEffectParam[iEffectID]->m_fShadowBiasFromGUI;

	// assign shadow view to shader
	GGHANDLE hShadowViewParam = m_pEffectParam[iEffectID]->m_mShadow;

	// pass in light camera VIEWPROJ so it can be multipled with worldpos pixel
	m_matShadowView = *m_pLightCamera->GeViewMatrix();
	GGMATRIX dxmatTextureScale;
	GGMatrixScaling( &dxmatTextureScale,  
		0.5f, 
		-0.5f,
		1.0f );
	GGMATRIX dxmatTextureTranslation;
	GGMatrixTranslation( &dxmatTextureTranslation, .5,.5,0 );
	
	// assign cascade metrics to shader
	GGVECTOR4 m_vCascadeScale[8];
	GGVECTOR4 m_vCascadeOffset[8];
	for(int index=0; index < m_CopyOfCascadeConfig.m_nCascadeLevels; ++index ) 
	{
		GGMATRIX mShadowTexture = m_matShadowProj[index] * dxmatTextureScale * dxmatTextureTranslation;
		m_vCascadeScale[index].x = mShadowTexture._11;
		m_vCascadeScale[index].y = mShadowTexture._22;
		m_vCascadeScale[index].z = mShadowTexture._33;
		m_vCascadeScale[index].w = 1;
		m_vCascadeOffset[index].x = mShadowTexture._41;
		m_vCascadeOffset[index].y = mShadowTexture._42;
		m_vCascadeOffset[index].z = mShadowTexture._43;
		m_vCascadeOffset[index].w = 0;
	}
	GGHANDLE hCascadeScaleArrayParam = m_pEffectParam[iEffectID]->m_vCascadeScale;
	GGHANDLE hCascadeOffsetArrayParam = m_pEffectParam[iEffectID]->m_vCascadeOffset;

    // Copy intervals for the depth interval selection method.
	float m_fCascadeFrustumsEyeSpaceDepths[MAX_CASCADES];
	for ( int i=0; i<MAX_CASCADES; i++ )
	{
		m_fCascadeFrustumsEyeSpaceDepths[i] = m_fCascadePartitionsFrustum[i];
	}
	GGHANDLE hCascadeFrustumsEyeSpaceDepthsParam = m_pEffectParam[iEffectID]->m_fCascadeFrustumsEyeSpaceDepths;
    
    // The border padding values keep the pixel shader from reading the borders during PCF filtering.
    float m_fMaxBorderPadding = (float)( m_pCascadeConfig->m_iBufferSize  - 1.0f ) / (float)m_pCascadeConfig->m_iBufferSize;
    float m_fMinBorderPadding = (float)( 1.0f ) / (float)m_pCascadeConfig->m_iBufferSize;
	GGHANDLE hMaxBorderPaddingParam = m_pEffectParam[iEffectID]->m_fMaxBorderPadding;
	GGHANDLE hMinBorderPaddingParam = m_pEffectParam[iEffectID]->m_fMinBorderPadding;

	// set number of cascades
	int m_nCascadeLevels = m_CopyOfCascadeConfig.m_nCascadeLevels;
	GGHANDLE hCascadeLevelsParam = m_pEffectParam[iEffectID]->m_nCascadeLevels;

	#ifdef DX11
	if ( hPCFBlurForLoopEndParam ) hPCFBlurForLoopEndParam->AsScalar()->SetInt ( m_iPCFBlurForLoopEnd );
	if ( hPCFBlurForLoopStartParam ) hPCFBlurForLoopStartParam->AsScalar()->SetInt ( m_iPCFBlurForLoopStart );
	if ( hCascadeBlendAreaParam ) hCascadeBlendAreaParam->AsScalar()->SetFloat ( m_fCascadeBlendArea );
	if ( hTexelSizeParam ) hTexelSizeParam->AsScalar()->SetFloat ( m_fTexelSize );
	if ( hShadowBiasFromGUIParam ) hShadowBiasFromGUIParam->AsScalar()->SetFloat ( m_fShadowBiasFromGUI );
	if ( hShadowViewParam ) hShadowViewParam->AsMatrix()->SetMatrix ( (const float*)m_matShadowView );
	if ( hCascadeScaleArrayParam ) hCascadeScaleArrayParam->AsVector()->SetFloatVectorArray ( (const float*)m_vCascadeScale, 0, 8 );
	if ( hCascadeOffsetArrayParam ) hCascadeOffsetArrayParam->AsVector()->SetFloatVectorArray ( (const float*)m_vCascadeOffset, 0, 8 );
	if ( hCascadeFrustumsEyeSpaceDepthsParam ) hCascadeFrustumsEyeSpaceDepthsParam->AsScalar()->SetFloatArray ( (const float*)m_fCascadeFrustumsEyeSpaceDepths, 0, MAX_CASCADES );
	if ( hMaxBorderPaddingParam ) hMaxBorderPaddingParam->AsScalar()->SetFloat ( m_fMaxBorderPadding );
	if ( hMinBorderPaddingParam ) hMinBorderPaddingParam->AsScalar()->SetFloat ( m_fMinBorderPadding );
	if ( hCascadeLevelsParam ) hCascadeLevelsParam->AsScalar()->SetFloat ( m_nCascadeLevels );
	#else
	pEffectPtr->SetInt ( hPCFBlurForLoopEndParam, m_iPCFBlurForLoopEnd );
	pEffectPtr->SetInt ( hPCFBlurForLoopStartParam, m_iPCFBlurForLoopStart );
	pEffectPtr->SetFloat ( hCascadeBlendAreaParam, m_fCascadeBlendArea );
	pEffectPtr->SetFloat ( hTexelSizeParam, m_fTexelSize );
	pEffectPtr->SetFloat ( hShadowBiasFromGUIParam, m_fShadowBiasFromGUI );
	pEffectPtr->SetMatrix ( hShadowViewParam, &m_matShadowView );
	pEffectPtr->SetVectorArray ( hCascadeScaleArrayParam, m_vCascadeScale, 8 );
	pEffectPtr->SetVectorArray ( hCascadeOffsetArrayParam, m_vCascadeOffset, 8 );
	pEffectPtr->SetFloatArray ( hCascadeFrustumsEyeSpaceDepthsParam, m_fCascadeFrustumsEyeSpaceDepths, MAX_CASCADES );
	pEffectPtr->SetFloat ( hMaxBorderPaddingParam, m_fMaxBorderPadding );
	pEffectPtr->SetFloat ( hMinBorderPaddingParam, m_fMinBorderPadding );
	pEffectPtr->SetFloat ( hCascadeLevelsParam, m_nCascadeLevels );
	#endif

	// done
    return hr;
}

//-----------------------------------------------------------------------------
// Build a frustum from a persepective projection matrix.  The matrix may only
// contain a projection; any rotation, translation or scale will cause the
// constructed frustum to be incorrect.
//-----------------------------------------------------------------------------
VOID ComputeFrustumFromProjection( Frustum* pOut, GGMATRIX* pProjection )
{
    assert( pOut );
    assert( pProjection );

    // Corners of the projection frustum in homogenous space.
    static GGVECTOR4 HomogenousPoints[6];
    HomogenousPoints[0] = GGVECTOR4 (  1.0f,  0.0f, 1.0f, 1.0f );   // right (at far plane)
    HomogenousPoints[1] = GGVECTOR4 ( -1.0f,  0.0f, 1.0f, 1.0f );   // left
    HomogenousPoints[2] = GGVECTOR4 (  0.0f,  1.0f, 1.0f, 1.0f );   // top
    HomogenousPoints[3] = GGVECTOR4 (  0.0f, -1.0f, 1.0f, 1.0f );   // bottom
    HomogenousPoints[4] = GGVECTOR4 ( 0.0f, 0.0f, 0.0f, 1.0f );     // near
    HomogenousPoints[5] = GGVECTOR4 ( 0.0f, 0.0f, 1.0f, 1.0f );     // far

    float Determinant;
    GGMATRIX matInverse;
	GGMatrixInverse( &matInverse, &Determinant, pProjection );

    // Compute the frustum corners in world space.
    GGVECTOR4 Points[6];

    for( INT i = 0; i < 6; i++ )
    {
        // Transform point.
        GGVec4Transform( &Points[i], &HomogenousPoints[i], &matInverse );
    }

    pOut->Origin = GGVECTOR3( 0.0f, 0.0f, 0.0f );
    pOut->Orientation = GGVECTOR4( 0.0f, 0.0f, 0.0f, 1.0f );

    // Compute the slopes
    Points[0] = XMVectorMultiply ( Points[0], XMVectorReciprocal( XMVectorSplatZ( Points[0] ) ) );
    Points[1] = XMVectorMultiply ( Points[1], XMVectorReciprocal( XMVectorSplatZ( Points[1] ) ) );
    Points[2] = XMVectorMultiply ( Points[2], XMVectorReciprocal( XMVectorSplatZ( Points[2] ) ) );
    Points[3] = XMVectorMultiply ( Points[3], XMVectorReciprocal( XMVectorSplatZ( Points[3] ) ) );
    pOut->RightSlope = XMVectorGetX( Points[0] );
    pOut->LeftSlope = XMVectorGetX( Points[1] );
    pOut->TopSlope = XMVectorGetY( Points[2] );
    pOut->BottomSlope = XMVectorGetY( Points[3] );

    // Compute near and far.
    Points[4] = XMVectorMultiply ( Points[4], XMVectorReciprocal( XMVectorSplatW( Points[4] ) ) );
    Points[5] = XMVectorMultiply ( Points[5], XMVectorReciprocal( XMVectorSplatW( Points[5] ) ) );

    pOut->Near = XMVectorGetZ( Points[4] );
    pOut->Far = XMVectorGetZ( Points[5] );

    return;
}

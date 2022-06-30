#ifndef _SHADOWMAPS_H_
#define _SHADOWMAPS_H_

#include "preprocessor-flags.h"

#include "..\CommonC.h"
#include "DepthTexture.h"
#include <assert.h>

#define MAX_CASCADES 8
#define EFFECT_INDEX_SIZE 1300

// Used to do selection of the shadow buffer format.
enum SHADOW_TEXTURE_FORMAT 
{
    CASCADE_DXGI_FORMAT_R32_TYPELESS,
    CASCADE_DXGI_FORMAT_R24G8_TYPELESS,
    CASCADE_DXGI_FORMAT_R16_TYPELESS,
    CASCADE_DXGI_FORMAT_R8_TYPELESS
};

enum SCENE_SELECTION 
{
    POWER_PLANT_SCENE,
    TEST_SCENE
};

enum FIT_PROJECTION_TO_CASCADES 
{
    FIT_TO_CASCADES,
    FIT_TO_SCENE
};

enum FIT_TO_NEAR_FAR 
{
    FIT_NEARFAR_PANCAKING,
    FIT_NEARFAR_ZERO_ONE,
    FIT_NEARFAR_AABB,
    FIT_NEARFAR_SCENE_AABB
};

enum CASCADE_SELECTION 
{
    CASCADE_SELECTION_MAP,
    CASCADE_SELECTION_INTERVAL
};

enum CAMERA_SELECTION 
{
    EYE_CAMERA,
    LIGHT_CAMERA,
    ORTHO_CAMERA1,
    ORTHO_CAMERA2,
    ORTHO_CAMERA3,
    ORTHO_CAMERA4,
    ORTHO_CAMERA5,
    ORTHO_CAMERA6,
    ORTHO_CAMERA7,
    ORTHO_CAMERA8
};

struct CascadeConfig 
{
    INT m_nCascadeLevels;
    SHADOW_TEXTURE_FORMAT m_ShadowBufferFormat;
    INT m_iBufferSize;
};

struct CB_ALL_SHADOW_DATA
{
    GGMATRIX  m_WorldViewProj;
    GGMATRIX  m_World;
    GGMATRIX  m_WorldView;
    GGMATRIX  m_Shadow;
    GGVECTOR4 m_vCascadeOffset[8];
    GGVECTOR4 m_vCascadeScale[8];

    INT         m_nCascadeLevels; // Number of Cascades
    INT         m_iVisualizeCascades; // 1 is to visualize the cascades in different colors. 0 is to just draw the scene.
    INT         m_iPCFBlurForLoopStart; // For loop begin value. For a 5x5 kernal this would be -2.
    INT         m_iPCFBlurForLoopEnd; // For loop end value. For a 5x5 kernel this would be 3.

    // For Map based selection scheme, this keeps the pixels inside of the the valid range.
    // When there is no boarder, these values are 0 and 1 respectivley.
    FLOAT       m_fMinBorderPadding;     
    FLOAT       m_fMaxBorderPadding;
    FLOAT       m_fShadowBiasFromGUI;  // A shadow map offset to deal with self shadow artifacts.  
                                        //These artifacts are aggravated by PCF.
    FLOAT       m_fCascadeBlendArea; // Amount to overlap when blending between cascades.
    FLOAT       m_fTexelSize; // Shadow map texel size.
    FLOAT       m_fCascadeFrustumsEyeSpaceDepths[8]; // The values along Z that seperate the cascades.
    GGVECTOR4 m_fCascadeFrustumsEyeSpaceDepthsFloat4[8];// the values along Z that separte the cascades.  
                                                          // Wastefully stored in float4 so they are array indexable :(
    GGVECTOR4 m_vLightDir;
};

class CFirstPersonCamera
{
	public:
    CFirstPersonCamera();

    // Call these from client and use Get*Matrix() to read new matrices
    //virtual void    FrameMove( FLOAT fElapsedTime );

    // Functions to change behavior
    //void            SetRotateButtons( bool bLeft, bool bMiddle, bool bRight, bool bRotateWithoutButtonDown = false );

    // Functions to get state
    GGMATRIX* GetWorldMatrix()
    {
        return &m_mCameraWorld;
    }
    GGMATRIX* GeViewMatrix()
    {
        return &m_mCameraView;
    }

    const GGVECTOR3* GetWorldRight() const
    {
        return ( GGVECTOR3* )&m_mCameraWorld._11;
    }
    const GGVECTOR3* GetWorldUp() const
    {
        return ( GGVECTOR3* )&m_mCameraWorld._21;
    }
    const GGVECTOR3* GetWorldAhead() const
    {
        return ( GGVECTOR3* )&m_mCameraWorld._31;
    }
    const GGVECTOR3* GetEyePt() const
    {
        return ( GGVECTOR3* )&m_mCameraWorld._41;
    }

	const float GetNearClip(void) { return m_fNear; }
	const float GetFarClip(void) { return m_fFar; }

	public:
	float m_fNear;
	float m_fFar;
    GGMATRIX m_mCameraWorld;       // World matrix of the camera (inverse of the view matrix)
    GGMATRIX m_mCameraView;
};

// Global Init function for shadow details
void InitShadowMapDetails(int rs0, int rs1, int rs2, int rs3, int rs4, int rs5, int rs6, int rs7, float dist, int flash, int speed);

//
// Shadow Mapping Object Class
//

class CascadedShadowsManager 
{
public:
    CascadedShadowsManager();
    ~CascadedShadowsManager();
    
    // This runs when the application is initialized.
    HRESULT Init( CFirstPersonCamera* pViewerCamera,
                  CFirstPersonCamera* pLightCamera,
                  CascadeConfig* pCascadeConfig );
    
    HRESULT DestroyAndDeallocateShadowResources();

    // This runs per frame.  This data could be cached when the cameras do not move.
    HRESULT InitFrame( LPGGEFFECT pEffectPtr ) ;

	void GetShaderVar ( LPGGEFFECT pEffectPtr, LPSTR pVariableName, GGHANDLE* pVarhandle );

    HRESULT RenderShadowsForAllCascades( LPGGEFFECT pEffectPtr );

    HRESULT RenderScene ( int iEffectID, LPGGEFFECT pEffectPtr,
					      LPVOID prtvBackBuffer,//ID3D9RenderTargetView* prtvBackBuffer, 
                          LPVOID pdsvBackBuffer,//ID3D9DepthStencilView* pdsvBackBuffer, 
                          CFirstPersonCamera* pActiveCamera,
						  BOOL bVisualize
                        );

    GGVECTOR3 GetSceneAABBMin()		{ return GGVECTOR3(m_vSceneAABBMin.x,m_vSceneAABBMin.y,m_vSceneAABBMin.z); };
    GGVECTOR3 GetSceneAABBMax()		{ return GGVECTOR3(m_vSceneAABBMax.x,m_vSceneAABBMax.y,m_vSceneAABBMax.z); };
    
    INT                                 m_iCascadePartitionsMax;
    FLOAT                               m_fCascadePartitionsFrustum[MAX_CASCADES]; // Values are  between near and far
    INT                                 m_iCascadePartitionsZeroToOne[MAX_CASCADES]; // Values are 0 to 100 and represent a percent of the frstum
    INT                                 m_iPCFBlurSize;
    FLOAT                               m_fPCFOffset;
    INT                                 m_iDerivativeBasedOffset;
    INT                                 m_iBlurBetweenCascades;
    FLOAT                               m_fBlurBetweenCascadesAmount;

    BOOL                                m_bMoveLightTexelSize;
    CAMERA_SELECTION                    m_eSelectedCamera;
    FIT_PROJECTION_TO_CASCADES          m_eSelectedCascadesFit;
    FIT_TO_NEAR_FAR                     m_eSelectedNearFarFit;

	DWORD								m_dwMask;

	struct sEffectParam
	{
		GGHANDLE m_iPCFBlurForLoopStart;
		GGHANDLE m_iPCFBlurForLoopEnd;
		GGHANDLE m_fCascadeBlendArea;
		GGHANDLE m_fTexelSize;
		GGHANDLE m_fShadowBiasFromGUI;
		GGHANDLE m_mShadow;
		GGHANDLE m_vCascadeScale;
		GGHANDLE m_vCascadeOffset;
		GGHANDLE m_fCascadeFrustumsEyeSpaceDepths;
		GGHANDLE m_fMaxBorderPadding;
		GGHANDLE m_fMinBorderPadding;
		GGHANDLE m_nCascadeLevels;
		GGHANDLE DepthMapTX1;
		GGHANDLE DepthMapTX2;
		GGHANDLE DepthMapTX3;
		GGHANDLE DepthMapTX4;
		GGHANDLE DepthMapTX5;
		GGHANDLE DepthMapTX6;
		GGHANDLE DepthMapTX7;
		GGHANDLE DepthMapTX8;
	};

public:
	sEffectParam*						m_pEffectParam[EFFECT_INDEX_SIZE];

public:

    // Compute the near and far plane by intersecting an Ortho Projection with the Scenes AABB.
    void ComputeNearAndFar( FLOAT& fNearPlane, 
                            FLOAT& fFarPlane, 
                            GGVECTOR4 vLightCameraOrthographicMin, 
                            GGVECTOR4 vLightCameraOrthographicMax, 
                            GGVECTOR4* pvPointsInCameraView 
                          );
    

    void CreateFrustumPointsFromCascadeInterval ( FLOAT fCascadeIntervalBegin, 
                                                  FLOAT fCascadeIntervalEnd, 
                                                  GGMATRIX& vProjection,
                                                  GGVECTOR4* pvCornerPointsWorld
                                                );


    void CreateAABBPoints( GGVECTOR4* vAABBPoints, GGVECTOR4 vCenter, GGVECTOR4 vExtents );

    HRESULT ReleaseAndAllocateNewShadowResources( LPGGEFFECT pEffectPtr );

    GGVECTOR4                         m_vSceneAABBMin;
    GGVECTOR4                         m_vSceneAABBMax;
                                                                               // For example: when the shadow buffer size changes.
    char                                m_cvsModel[31];
    char                                m_cpsModel[31];
    char                                m_cgsModel[31];
    GGMATRIX							m_matShadowProj[MAX_CASCADES]; 
    GGMATRIX							m_matShadowView;
    CascadeConfig                       m_CopyOfCascadeConfig;      // This copy is used to determine when settings change. 
                                                                    //Some of these settings require new buffer allocations.
    CascadeConfig*                      m_pCascadeConfig;           // Pointer to the most recent setting.

	// color and depth surfaces for cascade render
	LPGGSURFACE							m_pCascadedShadowMapDepthSurface;
	DepthTexture*						m_depthTexture[MAX_CASCADES];
	GGHANDLE							m_depthHandle[MAX_CASCADES];
    CFirstPersonCamera*                 m_pViewerCamera;         
    CFirstPersonCamera*                 m_pLightCamera;         
};

struct DBFrustum
{
    GGVECTOR3 Origin;            // Origin of the frustum (and projection).
    GGVECTOR4 Orientation;       // Unit quaternion representing rotation.

    FLOAT RightSlope;           // Positive X slope (X/Z).
    FLOAT LeftSlope;            // Negative X slope.
    FLOAT TopSlope;             // Positive Y slope (Y/Z).
    FLOAT BottomSlope;          // Negative Y slope.
    FLOAT Near, Far;            // Z of the near plane and far plane.
};

VOID ComputeFrustumFromProjection( DBFrustum* pOut, GGMATRIX* pProjection );

//
// XNA Derived 3D Math Help Functions
//

// structures
struct DBXMVECTORU32
{
	DWORD x;
	DWORD y;
	DWORD z;
	DWORD w;
};

// inline functions
inline GGVECTOR4 XMVectorReciprocal ( GGVECTOR4 V )
{
	GGVECTOR4 vResult;
	vResult.x = 1 / V.x;
	vResult.y = 1 / V.y;
	vResult.z = 1 / V.z;
	vResult.w = 1 / V.w;
	return vResult;
}
inline GGVECTOR4 XMVectorSplatX ( GGVECTOR4 V )
{
    GGVECTOR4 vResult;
	vResult.x = vResult.y = vResult.z = vResult.y = V.x;
    return vResult;
}
inline GGVECTOR4 XMVectorSplatY ( GGVECTOR4 V )
{
    GGVECTOR4 vResult;
	vResult.x = vResult.y = vResult.z = vResult.y = V.y;
    return vResult;
}
inline GGVECTOR4 XMVectorSplatZ ( GGVECTOR4 V )
{
    GGVECTOR4 vResult;
	vResult.x = vResult.y = vResult.z = vResult.y = V.z;
    return vResult;
}
inline GGVECTOR4 XMVectorSplatW ( GGVECTOR4 V )
{
    GGVECTOR4 vResult;
	vResult.x = vResult.y = vResult.z = vResult.y = V.w;
    return vResult;
}
inline GGVECTOR4 XMVectorMultiply ( GGVECTOR4 V, GGVECTOR4 A )
{
    GGVECTOR4 vResult;
	vResult.x = V.x * A.x;
	vResult.y = V.y * A.y;
	vResult.z = V.z * A.z;
	vResult.w = V.w * A.w;
    return vResult;
}
inline GGVECTOR4 XMVectorMultiplyAdd
(
    GGVECTOR4 V1, 
    GGVECTOR4 V2, 
    GGVECTOR4 V3
)
{
    GGVECTOR4 vResult;
	vResult.x = (V1.x * V2.x) + V3.x;
	vResult.y = (V1.y * V2.y) + V3.y;
	vResult.z = (V1.z * V2.z) + V3.z;
	vResult.w = (V1.w * V2.w) + V3.w;
    return vResult;
}
inline GGVECTOR4 XMVectorDivide ( GGVECTOR4 V, GGVECTOR4 A )
{
    GGVECTOR4 vResult;
	vResult.x = V.x / A.x;
	vResult.y = V.y / A.y;
	vResult.z = V.z / A.z;
	vResult.w = V.w / A.w;
    return vResult;
}

inline float XMVectorGetX ( GGVECTOR4 V )	{ return V.x; }
inline float XMVectorGetY ( GGVECTOR4 V )	{ return V.y; }
inline float XMVectorGetZ ( GGVECTOR4 V )	{ return V.z; }
inline float XMVectorGetW ( GGVECTOR4 V )	{ return V.w; }

inline GGVECTOR4 XMVectorSelect
(
    GGVECTOR4 V1, 
    GGVECTOR4 V2, 
    DBXMVECTORU32 Control
)
{
    DBXMVECTORU32 VV1 = *(DBXMVECTORU32*)&V1;
    DBXMVECTORU32 VV2 = *(DBXMVECTORU32*)&V2;

    GGVECTOR4 Result;
	Result.x = (VV1.x & ~Control.x) | (VV2.x & Control.x);
    Result.y = (VV1.y & ~Control.y) | (VV2.y & Control.y);
    Result.z = (VV1.z & ~Control.z) | (VV2.z & Control.z);
    Result.w = (VV1.w & ~Control.w) | (VV2.w & Control.w);
    return Result;
}

// My version without all that messing about with unsigned balh blah
inline GGVECTOR4 XMVectorSelectLee
(
    GGVECTOR4 V1, 
    GGVECTOR4 V2, 
    int elementindex
)
{
    GGVECTOR4 Result = V1;
	if ( elementindex==0 ) Result.x = V2.x;
	if ( elementindex==1 ) Result.y = V2.y;
	if ( elementindex==2 ) Result.z = V2.z;
	if ( elementindex==3 ) Result.w = V2.w;
    return Result;
}

// Return a floating point value via an index. This is not a recommended
// function to use due to performance loss.
inline FLOAT XMVectorGetByIndex(GGVECTOR4 V, UINT i)
{
    assert( i <= 3 );
    ///return V[i];//.vector4_f32[i]; changed when did DX11 port
	if ( i==0 ) return V.x;
	if ( i==1 ) return V.y;
	if ( i==2 ) return V.z;
	if ( i==3 ) return V.w;
}

// Rounding and clamping operations
inline GGVECTOR4 XMVectorMin
(
    GGVECTOR4 V1, 
    GGVECTOR4 V2
)
{
    GGVECTOR4 Result;
    Result.x = (V1.x < V2.x) ? V1.x : V2.x;
    Result.y = (V1.y < V2.y) ? V1.y : V2.y;
    Result.z = (V1.z < V2.z) ? V1.z : V2.z;
    Result.w = (V1.w < V2.w) ? V1.w : V2.w;
    return Result;
}

inline GGVECTOR4 XMVectorMax
(
    GGVECTOR4 V1, 
    GGVECTOR4 V2
)
{
    GGVECTOR4 Result;
    Result.x = (V1.x > V2.x) ? V1.x : V2.x;
    Result.y = (V1.y > V2.y) ? V1.y : V2.y;
    Result.z = (V1.z > V2.z) ? V1.z : V2.z;
    Result.w = (V1.w > V2.w) ? V1.w : V2.w;
    return Result;
}

inline float XMVector3LengthSq
(
    GGVECTOR3 V
)
{
    return GGVec3Dot(&V, &V);
}

//------------------------------------------------------------------------------

inline float XMVector4LengthSq
(
    GGVECTOR4 V
)
{
    return GGVec4Dot(&V, &V);
}

inline GGVECTOR4 XMVectorZero()
{
    GGVECTOR4 vResult = GGVECTOR4(0.0f,0.0f,0.0f,0.0f);
    return vResult;
}

// Initialize a vector with four floating point values
inline GGVECTOR4 XMVectorSet
(
    float x, 
    float y, 
    float z, 
    float w
)
{
    GGVECTOR4 vResult = GGVECTOR4(x,y,z,w);
    return vResult;
}

inline GGVECTOR4 XMVectorFloor
(
    GGVECTOR4 V
)
{
    GGVECTOR4 vResult = GGVECTOR4(
        floorf(V.x),
        floorf(V.y),
        floorf(V.z),
        floorf(V.w)
    );
    return vResult;
}



#endif

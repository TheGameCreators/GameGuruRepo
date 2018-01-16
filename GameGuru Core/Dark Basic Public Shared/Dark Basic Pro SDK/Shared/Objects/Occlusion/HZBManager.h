#ifndef _HZB_H_
#define _HZB_H_

// Common includes
#include "..\CommonC.h"

struct sOccluderObject
{
	DWORD dwStartVertex;
	DWORD dwVertexToRenderCount;
	sObject* pObject;
};

class HZBManager
{
public:
    HZBManager(IDirect3DDevice9* pDevice);
    virtual ~HZBManager(void);

    void AddOccluder(sObject* pObject);
    void AddOccludee(sObject* pObject);
	void DeleteOccludite ( sObject* pObject );

    void Render(const GGMATRIX& view, const GGMATRIX& projection);

    void BeginCullingQuery(
        UINT boundsSize,
        const GGVECTOR4* bounds,
        const GGMATRIX& view,
        const GGMATRIX& projection);

    void EndCullingQuery(UINT boundsSize, float* pResults);

    void Initialize(UINT width = 512, UINT height = 256, UINT initialBoundsBufferSize = 1000);

public:
    void Reset();

    void LoadDepthEffect();
	void FillDepthRenderStart ( void );
	bool FillDepthRenderWithVerts ( sObject* pObject, DWORD* pdwVertexStart, DWORD* pdwVertexCount );
	void FillDepthRenderDone ( void );

    void LoadMipmapEffect();
    void LoadCullEffect();

    void RenderOccluders(const GGMATRIX& view, const GGMATRIX& projection);
    void RenderMipmap();

    void EnsureCullingBuffers(UINT desiredSize);

    IDirect3DDevice9* m_pDevice;

    UINT m_width;
    UINT m_height;
    UINT m_boundsSize;

    UINT m_mipLevels;

    IDirect3DTexture9* m_pHizBufferEven;
    IDirect3DTexture9* m_pHizBufferOdd;
	IDirect3DSurface9* m_pOcclusionZBuffer;
    IDirect3DSurface9** m_pMipLevelEvenSurfaces;
    IDirect3DSurface9** m_pMipLevelOddSurfaces;

    LPGGEFFECT m_pDepthEffect;
	bool m_bTriggerDepthOccluderVertUpdate;
	DWORD m_dwVertexToRenderCount;
	DWORD m_dwMaxVertexNumberRequired;
	float* m_pVerticesForNextOcclusionDepthDraw;
	DWORD m_dwOccluderVertexBufferMaxSize;
    IGGVertexBuffer* m_pOccluderVertexBuffer;
    IDirect3DVertexDeclaration9* m_pDepthVertexDecl;
    GGHANDLE m_hDepthEffectTech;
    GGHANDLE m_hDepthEffectParamWVP;
    GGHANDLE m_hDepthEffectParamWV;

    LPGGEFFECT m_pHizMipmapEffect;
    IGGVertexBuffer* m_pHizMipmapVertexBuffer;
    IDirect3DVertexDeclaration9* m_pHizMipmapVertexDecl;
    GGHANDLE m_hMipmapTech;
    GGHANDLE m_hMipmapParamLastMipInfo;
    GGHANDLE m_hMipmapParamLastMip;

    LPGGEFFECT m_pHizCullEffect;
    IDirect3DVertexDeclaration9* m_pCullingBoundsVertexDecl;
    GGHANDLE m_hCullingTech;
    GGHANDLE m_hCullingParamView;
    GGHANDLE m_hCullingParamViewInv;
    GGHANDLE m_hCullingParamProjection;
    GGHANDLE m_hCullingParamViewProjection;
    GGHANDLE m_hCullingParamFrustumPlanes;
    GGHANDLE m_hCullingParamViewportSize;
    GGHANDLE m_hCullingParamResultsSize;
    GGHANDLE m_hCullingParamHiZMapEven;
    GGHANDLE m_hCullingParamHiZMapOdd;

    IGGVertexBuffer* m_pCullingBoundsBuffer;
    IDirect3DTexture9* m_pCullingResultsBuffer;
    IDirect3DSurface9* m_pCullingResultsSurface;
    IDirect3DTexture9* m_pCullingResultsBufferSys;
    IDirect3DSurface9* m_pCullingResultsSurfaceSys;
    UINT m_cullingResultsWidth;
    UINT m_cullingResultsHeight;

public:
	std::vector< sOccluderObject > m_vObjectOccluderList;
	std::vector< sObject* > m_vObjectOccludeeList;

};

#endif

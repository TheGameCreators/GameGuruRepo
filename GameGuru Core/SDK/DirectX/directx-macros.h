// Include correct DirectX headers
//
#define DX11 // DX9
//

#include "windows.h"

#ifdef DX11

// DirectX 11 Includes
#include <D3D11.h>
#include <D3DX11.h>
#include "d3dcompiler.h"
#include "d3dx11effect.h"
#include "K3D_Vector3D.h"
#include "K3D_Matrix.h"

// DirectX 11 Requirements
#define MAX_FVF_DECL_SIZE 65
#define _MAX_PATH 2014
#define MAX_SWAP_CHAINS	1
#define D3DFVF_VERTEX2D ( 1 ) //D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_LINES ( 2 ) //D3DFVF_XYZRHW | D3DFVF_DIFFUSE )
#define D3DFVF_FONT2DVERTEX ( 3 ) //D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define D3DFVF_FONT3DVERTEX ( 4 ) //D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_POINTVERTEX ( 5 ) //D3DFVF_XYZ | D3DFVF_DIFFUSE )

// Map DX11 macros to correct types
#ifndef GGMACROTYPES
#define GGMACROTYPES 1
#define IGG DWORD//IDirect3D
#define LPGG IGG* //LPDIRECT3D9
#define GGVECTOR2 KMaths::Vector2 //D3DXVECTOR2
#define GGVECTOR3 KMaths::Vector3 //D3DXVECTOR3
#define GGMATRIX KMaths::Matrix //D3DXMATRIX
#define GGMATRIXA16 KMaths::Matrix //D3DXMATRIXA16
#define GGVECTOR4 KMaths::Vector4 //D3DXVECTOR4
#define GGPLANE KMaths::Plane // D3DXPLANE
#define LPGGEFFECT ID3DX11Effect* //LPD3DXEFFECT
#define GGEFFECT_DESC D3DX11_EFFECT_DESC //D3DXEFFECT_DESC
#define GGHANDLE ID3DX11EffectVariable* //D3DXHANDLE
#define GGTECHNIQUEHANDLE ID3DX11EffectTechnique*
#define GGTECHNIQUE ID3DX11EffectTechnique* //D3DXHANDLE
#define GGPASSHANDLE ID3DX11EffectPass* // not in DX9
#define LPGGBASETEXTURE ID3D11Resource* //LPDIRECT3DBASETEXTURE9
#define LPGGTEXTURE ID3D11Resource* //LPDIRECT3DTEXTURE9
#define LPGGTEXTUREREF ID3D11ShaderResourceView* // DX11 image texture ref (view)
#define LPGGSHADERRESOURCEVIEW ID3D11ShaderResourceView* // DX11 only
#define LPGGCUBETEXTURE ID3D11Resource* //LPDIRECT3DCUBETEXTURE9
#define IGGVertexBuffer ID3D11Buffer //IDirect3DVertexBuffer9
#define IGGIndexBuffer ID3D11Buffer //IDirect3DIndexBuffer9
#define LPGGVERTEXBUFFER IGGVertexBuffer* //LPDIRECT3DVERTEXBUFFER9
#define LPGGINDEXBUFFER IGGIndexBuffer* //LPDIRECT3DINDEXBUFFER9
#define GGPRIMITIVETYPE D3D11_PRIMITIVE_TOPOLOGY //D3DPRIMITIVETYPE
#define LPGGVERTEXELEMENT LPVOID //LPD3DVERTEXELEMENT9
#define LPGGVERTEXSHADER LPVOID //LPDIRECT3DVERTEXSHADER9
#define LPGGPIXELSHADER LPVOID //LPDIRECT3DPIXELSHADER9
#define GGBASEMATERIAL D3DXMATERIAL //D3DXMATERIAL
#define GGQUATERNION KMaths::Quaternion //D3DXQUATERNION
#define GGFORMAT DXGI_FORMAT //D3DFORMAT
#define GG_OK 0 //D3D_OK
#define LPGGMESH sMesh* //LPD3DXMESH
#define GGSURFACE_DESC D3D11_TEXTURE2D_DESC //D3DSURFACE_DESC
#define GGLOCKED_RECT D3D11_MAPPED_SUBRESOURCE //D3DLOCKED_RECT
#define LPGGQUERY9 ID3D11Query* //LPDIRECT3DQUERY9
#define GGPRESENT_PARAMETERS DWORD //D3DPRESENT_PARAMETERS
#define LPGGSURFACE ID3D11Texture2D* //LPDIRECT3DSURFACE9
#define LPGGRENDERTARGETVIEW ID3D11RenderTargetView* //not in DX9
#define LPGGDEPTHSTENCILVIEW ID3D11DepthStencilView* //not in DX9
#define LPGGSHADERRESOURCEVIEW ID3D11ShaderResourceView* // not in DX9
#define IGGTexture ID3D11Resource // IDirect3DTexture9 
#define GGPOOL_MANAGED 0 // D3DPOOL_MANAGED
#define IGGVertexLayout ID3D11InputLayout //IDirect3DVertexDeclaration9
#define LPGGVERTEXLAYOUT IGGVertexLayout* //IDirect3DVertexDeclaration9
#define IGGDevice ID3D11Device //IDirect3DDevice9
#define LPGGDEVICE IGGDevice* //LPDIRECT3DDEVICE9
#define LPGGIMMEDIATECONTEXT ID3D11DeviceContext* // new for DX11
#define LPGGDEPTHSTENCILSTATE ID3D11DepthStencilState* // new for DX11
#define LPGGRASTERIZERSTATE ID3D11RasterizerState* // new for DX11
#define LPGGBLENDSTATE ID3D11BlendState* // only for DX11
#define IGGSurface DWORD //IDirect3DSurface9
#define GGCAPS DWORD //D3DCAPS9
#define GGPT_TRIANGLELIST D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST //D3DPT_TRIANGLELIST
#define GGPT_TRIANGLESTRIP D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP //D3DPT_TRIANGLESTRIP
#define GGPT_TRIANGLEFAN 0 //D3DPT_TRIANGLEFAN
#define GG_PI KMATHS_PI //D3DX_PI
#define GGTOP_MODULATE 0 //D3DTOP_MODULATE
#define GGTOP_SELECTARG1 0 //D3DTOP_SELECTARG1
#define GGTOP_ADD 0 //D3DTOP_ADD
#define GGTOP_DISABLE 0 //D3DTOP_DISABLE
#define GGTADDRESS_CLAMP 0 //D3DTADDRESS_CLAMP
#define GGTEXF_LINEAR 0 //D3DTEXF_LINEAR
#define GGTEXF_NONE 0 //D3DTEXF_NONE
#define GGTA_TEXTURE 0 //D3DTA_TEXTURE
#define GGTA_DIFFUSE 0 //D3DTA_DIFFUSE
#define GGTA_CURRENT 0 //D3DTA_CURRENT
#define GGIMAGE_INFO D3DX11_IMAGE_INFO //D3DXIMAGE_INFO
#define GGUSAGE_RENDERTARGET 1 //D3DUSAGE_RENDERTARGET
#define GGLIGHT_POINT 0 //D3DLIGHT_POINT
#define GGLIGHT_SPOT 0 //D3DLIGHT_SPOT
#define GGLIGHT_DIRECTIONAL 0 //D3DLIGHT_DIRECTIONAL
#define GGADAPTER_DEFAULT 0//D3DADAPTER_DEFAULT
#define GGDISPLAYMODE DWORD//D3DDISPLAYMODE
#define GGSWAPEFFECT DWORD//D3DSWAPEFFECT
#define LPGGSWAPCHAIN IDXGISwapChain*//LPDIRECT3DSWAPCHAIN9
#define GGERR_DEVICELOST 0 //D3DERR_DEVICELOST
#define IGGStateBlock9 DWORD //IDirect3DStateBlock9
#define GGCULL_CCW 0 //D3DCULL_CCW
#define GGTS_WORLD 256 //D3DTS_WORLD
#define GGTS_VIEW 2 //D3DTS_VIEW
#define GGTS_PROJECTION 3 //D3DTS_PROJECTION

#define GGMatrixInverse KMaths::MatrixInverse
#define GGMatrixTranspose KMaths::MatrixTranspose
#define GGMatrixRotationX KMaths::MatrixRotationX
#define GGMatrixRotationY KMaths::MatrixRotationY
#define GGMatrixRotationZ KMaths::MatrixRotationZ
#define GGMatrixIdentity KMaths::MatrixIdentity 
#define GGMatrixTranslation KMaths::MatrixTranslation 
#define GGMatrixMultiply KMaths::MatrixMultiply 
#define GGMatrixPerspectiveRH KMaths::MatrixPerspectiveRH 
#define GGMatrixPerspectiveLH KMaths::MatrixPerspectiveLH 
#define GGMatrixPerspectiveFovRH KMaths::MatrixPerspectiveFovRH 
#define GGMatrixPerspectiveFovLH KMaths::MatrixPerspectiveFovLH 
#define GGMatrixScaling KMaths::MatrixScaling 
#define GGMatrixRotationQuaternion KMaths::MatrixRotationQuaternion 
#define GGMatrixLookAtRH KMaths::MatrixLookAtRH 
#define GGMatrixLookAtLH KMaths::MatrixLookAtLH 
#define GGMatrixOrthoRH KMaths::MatrixOrthoRH 
#define GGMatrixOrthoLH KMaths::MatrixOrthoLH 
#define GGMatrixOrthoOffCenterRH KMaths::MatrixOrthoOffCenterRH 
#define GGMatrixOrthoOffCenterLH KMaths::MatrixOrthoOffCenterLH 
#define GGMatrixRotationYawPitchRoll KMaths::MatrixRotationYawPitchRoll 
#define GGMatrixIsIdentity KMaths::MatrixIsIdentity
#define GGMatrixReflect KMaths::MatrixReflect
#define GGMatrixRotationAxis KMaths::MatrixRotationAxis
#define GGPlaneFromPoints KMaths::PlaneFromPoints
#define GGVec3TransformCoord KMaths::TransformCoord
#define GGToRadian KMATHSToRadian //D3DXToRadian
#define GGToDegree KMATHSToDegree //D3DXToDegree
#define GGQuaternionRotationAxis KMaths::QuaternionRotationAxis //D3DXQuaternionRotationAxis
#define GGPlaneNormalize KMaths::PlaneNormalize //D3DXPlaneNormalize
#define GGPlaneTransform KMaths::PlaneTransform //D3DXPlaneTransform
#define GGVec3Dot KMaths::Dot //D3DXVec3Dot
#define GGPlaneFromPointNormal KMaths::PlaneFromPointNormal //D3DXPlaneFromPointNormal
#define GGVec3TransformNormal KMaths::TransformNormal //D3DXVec3TransformNormal
#define GGVec3Normalize KMaths::Normalize //D3DXVec3Normalize
#define GGVec3Length KMaths::Length //D3DXVec3Length
#define GGVec3LengthSq KMaths::LengthSq //D3DXVec3LengthSq
#define GGVec3CatmullRom KMaths::CatmullRom //D3DXVec3CatmullRom
#define GGQuaternionSlerp KMaths::QuaternionSlerp //D3DXQuaternionSlerp
#define GGVec3Cross KMaths::Cross //D3DXVec3Cross
#define GGPlaneDotCoord KMaths::PlaneDotCoord //D3DXPlaneDotCoord
#define GGIntersectTri KMaths::IntersectTri //D3DXIntersectTri
#define GGVec4Transform KMaths::Transform //D3DXVec4Transform
#define GGVec4Normalize KMaths::Normalize //D3DXVec4Normalize
#define GGVec4Length KMaths::Length4 //D3DXVec4Length
#define GGVec4Dot KMaths::Dot //D3DXVec4Dot
#define GGSetTransform(index,matrix) COMMONSetTransform(index,matrix); //m_pD3D->SetTransform
#define GGGetTransform(index,matrix) COMMONGetTransform(index,matrix); //m_pD3D->GetTransform
#define GGSetEffectFloat(handle,value) COMMONSetEffectFloat(handle,value); //m_pEffect->SetMatrix(handle,value);
#define GGSetEffectVector(handle,value) COMMONSetEffectVector(handle,value); //m_pEffect->SetMatrix(handle,value);
#define GGSetEffectMatrix(handle,value) COMMONSetEffectMatrix(handle,value); //m_pEffect->SetMatrix(handle,value);
#define GGSetEffectMatrixTransposeArray(handle,value,count) COMMONSetEffectMatrixTransposeArray(handle,value,count); //m_pEffect->SetMatrixTransposeArray(handle,value,count);

#define GGDECLTYPE_FLOAT2 1 //D3DDECLTYPE_FLOAT2
#define GGDECLTYPE_FLOAT3 2 //D3DDECLTYPE_FLOAT3
#define GGDECLTYPE_FLOAT4 3 //D3DDECLTYPE_FLOAT4
#define GGDECLMETHOD_DEFAULT 0 //D3DDECLMETHOD_DEFAULT
#define GGDECLEND GGVERTEXELEMENT() //GDECL_END()
#define GGDECLUSAGE_POSITION 0 //D3DDECLUSAGE_POSITION
#define GGDECLUSAGE_POSITIONT 9 //D3DDECLUSAGE_POSITION
#define GGDECLUSAGE_PSIZE 4 //D3DDECLUSAGE_PSIZE
#define GGDECLUSAGE_NORMAL 3 //D3DDECLUSAGE_NORMAL
#define GGDECLUSAGE_TEXCOORD 5 //D3DDECLUSAGE_TEXCOORD
#define GGDECLUSAGE_COLOR 10 //D3DDECLUSAGE_COLOR
#define GGDECLUSAGE_TANGENT 6 //D3DDECLUSAGE_TANGENT
#define GGDECLUSAGE_BINORMAL 7 //D3DDECLUSAGE_BINORMAL
#define GDECL_END 0 //D3DDECL_END

#define GGFVF_DIFFUSE 0x040 //D3DFVF_DIFFUSE
#define GGFVF_XYZ 0x002 //D3DFVF_XYZ
#define GGFVF_XYZRHW 0x004
#define GGFVF_NORMAL 0x010 //D3DFVF_NORMAL
#define GGFVF_SPECULAR 0x080
#define GGFVF_TEX1 0x100 //D3DFVF_TEX1
#define GGFVF_TEX2 0x200 //D3DFVF_TEX2
#define GGFVF_TEX3 0x300 //D3DFVF_TEX3
#define GGFVF_TEX4 0x400 //D3DFVF_TEX4
#define GGFVF_TEX5 0x500 //D3DFVF_TEX5
#define GGFVF_TEX6 0x600 //D3DFVF_TEX6
#define GGFVF_TEX7 0x700 //D3DFVF_TEX7
#define GGFVF_TEX8 0x800 //D3DFVF_TEX8
#define GGFVF_TEXCOUNT_MASK 0xf00 //D3DFVF_TEXCOUNT_MASK
#define GGFVF_TEXCOUNT_SHIFT 8 //D3DFVF_TEXCOUNT_SHIFT
#define GGFVF_PSIZE 0x020 //D3DFVF_PSIZE
#define GGFVF_TEXTUREFORMAT1 3 // one floating point value
#define GGFVF_TEXTUREFORMAT2 0 // two floating point values
#define GGFVF_TEXTUREFORMAT3 1 // three floating point values
#define GGFVF_TEXTUREFORMAT4 2 // four floating point values
#define GGFVF_TEXCOORDSIZE1(CoordIndex) (GGFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16)) 
#define GGFVF_TEXCOORDSIZE2(CoordIndex) (GGFVF_TEXTUREFORMAT2) 
#define GGFVF_TEXCOORDSIZE3(CoordIndex) (GGFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16)) 
#define GGFVF_TEXCOORDSIZE4(CoordIndex) (GGFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))

#define GGFMT_A8R8G8B8 DXGI_FORMAT_B8G8R8A8_UNORM //DXGI_FORMAT_R8G8B8A8_UNORM
#define GGFMT_UNKNOWN DXGI_FORMAT_UNKNOWN//D3DFMT_UNKNOWN
#define GGFMT_R8G8B8 0//D3DFMT_R8G8B8
#define GGFMT_X8R8G8B8 0//D3DFMT_X8R8G8B8
#define GGFMT_R5G6B5 0//D3DFMT_R5G6B5
#define GGFMT_X1R5G5B5 0//D3DFMT_X1R5G5B5
#define GGFMT_A1R5G5B5 DXGI_FORMAT_R8G8B8A8_UINT//D3DFMT_A1R5G5B5
#define GGFMT_A4R4G4B4 0//D3DFMT_A4R4G4B4
#define GGFMT_R3G3B2 0//D3DFMT_R3G3B2
#define GGFMT_A8 0//D3DFMT_A8
#define GGFMT_A8R3G3B2 0//D3DFMT_A8R3G3B2
#define GGFMT_X4R4G4B4 0//D3DFMT_X4R4G4B4
#define GGFMT_A2B10G10R10 0//D3DFMT_A2B10G10R10
#define GGFMT_A8B8G8R8 0//D3DFMT_A8B8G8R8
#define GGFMT_X8B8G8R8 0//D3DFMT_X8B8G8R8
#define GGFMT_G16R16 0//D3DFMT_G16R16
#define GGFMT_A2R10G10B10 0//D3DFMT_A2R10G10B10
#define GGFMT_A16B16G16R16  0//D3DFMT_A16B16G16R16
#define GGFMT_A8P8  0//D3DFMT_A8P8
#define GGFMT_P8 0//D3DFMT_P8
#define GGFMT_L8 0//D3DFMT_L8
#define GGFMT_A8L8 0//D3DFMT_A8L8
#define GGFMT_A4L4 0//D3DFMT_A4L4
#define GGFMT_V8U8 0//D3DFMT_V8U8
#define GGFMT_L6V5U5 0//D3DFMT_L6V5U5
#define GGFMT_X8L8V8U8 0//D3DFMT_X8L8V8U8
#define GGFMT_Q8W8V8U8 0//D3DFMT_Q8W8V8U8
#define GGFMT_V16U16 0//D3DFMT_V16U16
#define GGFMT_A2W10V10U10 0//D3DFMT_A2W10V10U10
#define GGFMT_UYVY 0//D3DFMT_UYVY
#define GGFMT_R8G8_B8G8 0//D3DFMT_R8G8_B8G8
#define GGFMT_YUY2 0//D3DFMT_YUY2
#define GGFMT_G8R8_G8B8 0//D3DFMT_G8R8_G8B8
#define GGFMT_DXT1 DXGI_FORMAT_BC1_UNORM//D3DFMT_DXT1
#define GGFMT_DXT2 DXGI_FORMAT_BC1_UNORM//D3DFMT_DXT2
#define GGFMT_DXT3 DXGI_FORMAT_BC2_UNORM//D3DFMT_DXT3
#define GGFMT_DXT4 DXGI_FORMAT_BC2_UNORM//D3DFMT_DXT4
#define GGFMT_DXT5 DXGI_FORMAT_BC3_UNORM//D3DFMT_DXT5
#define GGFMT_D16_LOCKABLE 0//D3DFMT_D16_LOCKABLE
#define GGFMT_D32 0//D3DFMT_D32
#define GGFMT_D15S1 0//D3DFMT_D15S1
#define GGFMT_D24S8 DXGI_FORMAT_D24_UNORM_S8_UINT//D3DFMT_D24S8
#define GGFMT_D24X8 0//D3DFMT_D24X8
#define GGFMT_D24X4S4 0//D3DFMT_D24X4S4
#define GGFMT_D16 0//D3DFMT_D16
#define GGFMT_D32F_LOCKABLE 0//D3DFMT_D32F_LOCKABLE
#define GGFMT_D24FS8 0//D3DFMT_D24FS8
#define GGFMT_D32_LOCKABLE 0//D3DFMT_D32_LOCKABLE
#define GGFMT_S8_LOCKABLE 0//D3DFMT_S8_LOCKABLE
#define GGFMT_L16 0//D3DFMT_L16
#define GGFMT_VERTEXDATA 0//D3DFMT_VERTEXDATA
#define GGFMT_INDEX16 0//D3DFMT_INDEX16
#define GGFMT_INDEX32 0//D3DFMT_INDEX32
#define GGFMT_Q16W16V16U16 0//D3DFMT_Q16W16V16U16
#define GGFMT_MULTI2_ARGB8  0//D3DFMT_MULTI2_ARGB8
#define GGFMT_R16F DXGI_FORMAT_R16_FLOAT//D3DFMT_R16F
#define GGFMT_G16R16F DXGI_FORMAT_R8G8_UNORM//D3DFMT_G16R16F
#define GGFMT_A16B16G16R16F DXGI_FORMAT_R16G16B16A16_FLOAT//D3DFMT_A16B16G16R16F
#define GGFMT_R32F DXGI_FORMAT_R32_FLOAT//D3DFMT_R32F
#define GGFMT_G32R32F DXGI_FORMAT_R32G32_FLOAT//D3DFMT_G32R32F
#define GGFMT_A32B32G32R32F DXGI_FORMAT_R32G32B32A32_FLOAT//D3DFMT_A32B32G32R32F
#define GGFMT_CxV8U8 0//D3DFMT_CxV8U8
#define GGFMT_A1 0//D3DFMT_A1
#define GGFMT_A2B10G10R10_XR_BIAS 0//D3DFMT_A2B10G10R10_XR_BIAS
#define GGFMT_BINARYBUFFER 0//D3DFMT_BINARYBUFFER
#define GGFMT_FORCE_DWORD 0//D3DFMT_FORCE_DWORD

typedef struct _GGVERTEXELEMENT
{
    WORD    Stream;     // Stream index
    WORD    Offset;     // Offset in the stream in bytes
    BYTE    Type;       // Data type
    BYTE    Method;     // Processing method
    BYTE    Usage;      // Semantics
    BYTE    UsageIndex; // Semantic index
} GGVERTEXELEMENT;

typedef struct GGCOLOR //D3DXCOLOR
{
public:
    GGCOLOR() {}
    GGCOLOR( DWORD argb );
    GGCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a );

    // casting
    operator DWORD () const;

    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    GGCOLOR& operator += ( CONST GGCOLOR& );
    GGCOLOR& operator -= ( CONST GGCOLOR& );
    GGCOLOR& operator *= ( FLOAT );
    GGCOLOR& operator /= ( FLOAT );

    // unary operators
    GGCOLOR operator + () const;
    GGCOLOR operator - () const;

    // binary operators
    GGCOLOR operator + ( CONST GGCOLOR& ) const;
    GGCOLOR operator - ( CONST GGCOLOR& ) const;
    GGCOLOR operator * ( FLOAT ) const;
    GGCOLOR operator / ( FLOAT ) const;

    friend GGCOLOR operator * ( FLOAT, CONST GGCOLOR& );

    BOOL operator == ( CONST GGCOLOR& ) const;
    BOOL operator != ( CONST GGCOLOR& ) const;

    FLOAT r, g, b, a;

} GGCOLOR, *LPGGCOLOR;

#define GGCOLOR_ARGB(a,r,g,b) ((GGCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define GGCOLOR_RGBA(r,g,b,a) GGCOLOR_ARGB(a,r,g,b)
#define GGCOLOR_XRGB(r,g,b)   GGCOLOR_ARGB(0xff,r,g,b)

typedef struct GGVIEWPORT { //D3DVIEWPORT9
  DWORD X;
  DWORD Y;
  DWORD Width;
  DWORD Height;
  float MinZ;
  float MaxZ;
} GGVIEWPORT, *LPGGVIEWPORT;

typedef struct GGMATERIAL { //D3DMATERIAL9
  GGCOLOR Diffuse;
  GGCOLOR Ambient;
  GGCOLOR Specular;
  GGCOLOR Emissive;
  float   Power;
} GGMATERIAL, *LPGGMATERIAL;
#endif

/* LEE: Perhaps not attempt to use same FMT codes, DX11 to use fresh enums and modes!

typedef enum DXGI_FORMAT { 
  DXGI_FORMAT_UNKNOWN                     = 0,
  DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
  DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
  DXGI_FORMAT_R32G32B32A32_UINT           = 3,
  DXGI_FORMAT_R32G32B32A32_SINT           = 4,
  DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
  DXGI_FORMAT_R32G32B32_FLOAT             = 6,
  DXGI_FORMAT_R32G32B32_UINT              = 7,
  DXGI_FORMAT_R32G32B32_SINT              = 8,
  DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
  DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
  DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
  DXGI_FORMAT_R16G16B16A16_UINT           = 12,
  DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
  DXGI_FORMAT_R16G16B16A16_SINT           = 14,
  DXGI_FORMAT_R32G32_TYPELESS             = 15,
  DXGI_FORMAT_R32G32_FLOAT                = 16,
  DXGI_FORMAT_R32G32_UINT                 = 17,
  DXGI_FORMAT_R32G32_SINT                 = 18,
  DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
  DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
  DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
  DXGI_FORMAT_R10G10B10A2_UINT            = 25,
  DXGI_FORMAT_R11G11B10_FLOAT             = 26,
  DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
  DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
  DXGI_FORMAT_R8G8B8A8_UINT               = 30,
  DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
  DXGI_FORMAT_R8G8B8A8_SINT               = 32,
  DXGI_FORMAT_R16G16_TYPELESS             = 33,
  DXGI_FORMAT_R16G16_FLOAT                = 34,
  DXGI_FORMAT_R16G16_UNORM                = 35,
  DXGI_FORMAT_R16G16_UINT                 = 36,
  DXGI_FORMAT_R16G16_SNORM                = 37,
  DXGI_FORMAT_R16G16_SINT                 = 38,
  DXGI_FORMAT_R32_TYPELESS                = 39,
  DXGI_FORMAT_D32_FLOAT                   = 40,
  DXGI_FORMAT_R32_FLOAT                   = 41,
  DXGI_FORMAT_R32_UINT                    = 42,
  DXGI_FORMAT_R32_SINT                    = 43,
  DXGI_FORMAT_R24G8_TYPELESS              = 44,
  DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
  DXGI_FORMAT_R8G8_TYPELESS               = 48,
  DXGI_FORMAT_R8G8_UNORM                  = 49,
  DXGI_FORMAT_R8G8_UINT                   = 50,
  DXGI_FORMAT_R8G8_SNORM                  = 51,
  DXGI_FORMAT_R8G8_SINT                   = 52,
  DXGI_FORMAT_R16_TYPELESS                = 53,
  DXGI_FORMAT_R16_FLOAT                   = 54,
  DXGI_FORMAT_D16_UNORM                   = 55,
  DXGI_FORMAT_R16_UNORM                   = 56,
  DXGI_FORMAT_R16_UINT                    = 57,
  DXGI_FORMAT_R16_SNORM                   = 58,
  DXGI_FORMAT_R16_SINT                    = 59,
  DXGI_FORMAT_R8_TYPELESS                 = 60,
  DXGI_FORMAT_R8_UNORM                    = 61,
  DXGI_FORMAT_R8_UINT                     = 62,
  DXGI_FORMAT_R8_SNORM                    = 63,
  DXGI_FORMAT_R8_SINT                     = 64,
  DXGI_FORMAT_A8_UNORM                    = 65,
  DXGI_FORMAT_R1_UNORM                    = 66,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
  DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
  DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
  DXGI_FORMAT_BC1_TYPELESS                = 70,
  DXGI_FORMAT_BC1_UNORM                   = 71,
  DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
  DXGI_FORMAT_BC2_TYPELESS                = 73,
  DXGI_FORMAT_BC2_UNORM                   = 74,
  DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
  DXGI_FORMAT_BC3_TYPELESS                = 76,
  DXGI_FORMAT_BC3_UNORM                   = 77,
  DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
  DXGI_FORMAT_BC4_TYPELESS                = 79,
  DXGI_FORMAT_BC4_UNORM                   = 80,
  DXGI_FORMAT_BC4_SNORM                   = 81,
  DXGI_FORMAT_BC5_TYPELESS                = 82,
  DXGI_FORMAT_BC5_UNORM                   = 83,
  DXGI_FORMAT_BC5_SNORM                   = 84,
  DXGI_FORMAT_B5G6R5_UNORM                = 85,
  DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
  DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
  DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
  DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
  DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
  DXGI_FORMAT_BC6H_TYPELESS               = 94,
  DXGI_FORMAT_BC6H_UF16                   = 95,
  DXGI_FORMAT_BC6H_SF16                   = 96,
  DXGI_FORMAT_BC7_TYPELESS                = 97,
  DXGI_FORMAT_BC7_UNORM                   = 98,
  DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
  DXGI_FORMAT_AYUV                        = 100,
  DXGI_FORMAT_Y410                        = 101,
  DXGI_FORMAT_Y416                        = 102,
  DXGI_FORMAT_NV12                        = 103,
  DXGI_FORMAT_P010                        = 104,
  DXGI_FORMAT_P016                        = 105,
  DXGI_FORMAT_420_OPAQUE                  = 106,
  DXGI_FORMAT_YUY2                        = 107,
  DXGI_FORMAT_Y210                        = 108,
  DXGI_FORMAT_Y216                        = 109,
  DXGI_FORMAT_NV11                        = 110,
  DXGI_FORMAT_AI44                        = 111,
  DXGI_FORMAT_IA44                        = 112,
  DXGI_FORMAT_P8                          = 113,
  DXGI_FORMAT_A8P8                        = 114,
  DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
  DXGI_FORMAT_P208                        = 130,
  DXGI_FORMAT_V208                        = 131,
  DXGI_FORMAT_V408                        = 132,
  DXGI_FORMAT_FORCE_UINT                  = 0xffffffff
} DXGI_FORMAT;

*/


#else

// DirectX 9 LIBS and HEADERS
#define MAX_SWAP_CHAINS	4
#define D3DFVF_VERTEX2D ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_LINES ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE )
#define D3DFVF_FONT2DVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define D3DFVF_FONT3DVERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_POINTVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE )
#pragma comment ( lib, "dxguid.lib" )
#pragma comment ( lib, "d3d9.lib" )
#pragma comment ( lib, "d3dx9.lib" )
#pragma comment ( lib, "Dxerr.lib" )
#pragma comment ( lib, "d3dxof.lib" )
#include <ddraw.h>
#include <D3D9.h>
#include <Dxerr.h>
#include <D3dx9tex.h>
#include <D3dx9core.h>
#include <D3DX9.h>
#include <d3d9types.h>
#include <dshow.h>
#include <dxfile.h>
#include "D3dx9math.h"
#include "DXGI.h"
#include "dxdiag.h"

// Map DX9 macros to correct types
#ifndef GGMACROTYPES
#define GGMACROTYPES 1
#define LPGG LPDIRECT3D9
#define IGG IDirect3D
#define LPGGDEVICE LPDIRECT3DDEVICE9
#define GGVECTOR3 D3DXVECTOR3
#define GGMATRIX D3DXMATRIX
#define GGMATRIXA16 D3DXMATRIXA16
#define GGVECTOR4 D3DXVECTOR4
#define GGPLANE D3DXPLANE
#define LPGGEFFECT LPD3DXEFFECT
#define GGEFFECT_DESC D3DXEFFECT_DESC
#define GGHANDLE D3DXHANDLE
#define GGTECHNIQUEHANDLE D3DXHANDLE
#define GGTECHNIQUE D3DXHANDLE
#define GGPASSHANDLE D3DXHANDLE
#define LPGGBASETEXTURE LPDIRECT3DBASETEXTURE9
#define LPGGTEXTURE LPDIRECT3DTEXTURE9
#define LPGGTEXTUREREF LPDIRECT3DTEXTURE9
#define LPGGCUBETEXTURE LPDIRECT3DCUBETEXTURE9
#define LPGGVERTEXBUFFER LPDIRECT3DVERTEXBUFFER9
#define IGGVertexBuffer IDirect3DVertexBuffer9
#define IGGIndexBuffer IDirect3DIndexBuffer9
#define GGPRIMITIVETYPE D3DPRIMITIVETYPE
#define GGMATERIAL D3DMATERIAL9
#define LPGGVERTEXELEMENT LPD3DVERTEXELEMENT9
#define GGVERTEXELEMENT D3DVERTEXELEMENT9
#define LPGGVERTEXSHADER LPDIRECT3DVERTEXSHADER9
#define LPGGVERTEXDECLARATION LPDIRECT3DVERTEXDECLARATION9
#define LPGGPIXELSHADER LPDIRECT3DPIXELSHADER9
#define GGBASEMATERIAL D3DXMATERIAL
#define GGQUATERNION D3DXQUATERNION
#define GGFORMAT D3DFORMAT
#define GG_OK D3D_OK
#define LPGGMESH LPD3DXMESH
#define GGLOCKED_RECT D3DLOCKED_RECT
#define GGVECTOR2 D3DXVECTOR2
#define LPGGQUERY9 LPDIRECT3DQUERY9
#define GGPRESENT_PARAMETERS D3DPRESENT_PARAMETERS
#define GGVIEWPORT D3DVIEWPORT9
#define LPGGSURFACE LPDIRECT3DSURFACE9
#define LPGGRENDERTARGETVIEW LPDIRECT3DSURFACE9
#define LPGGDEPTHSTENCILVIEW LPDIRECT3DSURFACE9
#define IGGTexture IDirect3DTexture9 
#define GGPOOL_MANAGED D3DPOOL_MANAGED
#define IGGVertexDeclaration IDirect3DVertexDeclaration9
#define IGGVertexLayout IDirect3DVertexDeclaration9
#define LPGGVERTEXLAYOUT IDirect3DVertexDeclaration9*
#define IGGDevice IDirect3DDevice9
#define LPGGIMMEDIATECONTEXT LPVOID // new for DX11
#define LPGGINDEXBUFFER LPDIRECT3DINDEXBUFFER9
#define IGGSurface IDirect3DSurface9
#define GGCAPS D3DCAPS9
#define GGPT_TRIANGLELIST D3DPT_TRIANGLELIST
#define GGPT_TRIANGLESTRIP D3DPT_TRIANGLESTRIP
#define GGPT_TRIANGLEFAN D3DPT_TRIANGLEFAN
#define GGCOLOR D3DXCOLOR
#define GG_PI D3DX_PI
#define GGTOP_MODULATE D3DTOP_MODULATE
#define GGTOP_SELECTARG1 D3DTOP_SELECTARG1
#define GGTOP_ADD D3DTOP_ADD
#define GGTOP_DISABLE D3DTOP_DISABLE
#define GGTADDRESS_CLAMP D3DTADDRESS_CLAMP
#define GGTEXF_LINEAR D3DTEXF_LINEAR
#define GGTEXF_NONE D3DTEXF_NONE
#define GGTA_TEXTURE D3DTA_TEXTURE
#define GGTA_DIFFUSE D3DTA_DIFFUSE
#define GGTA_CURRENT D3DTA_CURRENT
#define GGIMAGE_INFO D3DXIMAGE_INFO
#define GGUSAGE_RENDERTARGET D3DUSAGE_RENDERTARGET
#define GGLIGHT_POINT D3DLIGHT_POINT
#define GGLIGHT_SPOT D3DLIGHT_SPOT
#define GGLIGHT_DIRECTIONAL D3DLIGHT_DIRECTIONAL
#define GGADAPTER_DEFAULT D3DADAPTER_DEFAULT
#define GGDISPLAYMODE D3DDISPLAYMODE
#define GGSWAPEFFECT D3DSWAPEFFECT
#define LPGGSWAPCHAIN LPDIRECT3DSWAPCHAIN9
#define GGERR_DEVICELOST D3DERR_DEVICELOST
#define IGGStateBlock9 IDirect3DStateBlock9
#define GGCULL_CCW D3DCULL_CCW
#define GGTS_WORLD D3DTS_WORLD
#define GGTS_VIEW D3DTS_VIEW
#define GGTS_PROJECTION D3DTS_PROJECTION

#define GGMatrixInverse D3DXMatrixInverse
#define GGMatrixTranspose D3DXMatrixTranspose
#define GGMatrixRotationX D3DXMatrixRotationX
#define GGMatrixRotationY D3DXMatrixRotationY
#define GGMatrixRotationZ D3DXMatrixRotationZ
#define GGMatrixIdentity D3DXMatrixIdentity 
#define GGMatrixTranslation D3DXMatrixTranslation 
#define GGMatrixMultiply D3DXMatrixMultiply 
#define GGMatrixPerspectiveRH D3DXMatrixPerspectiveRH 
#define GGMatrixPerspectiveLH D3DXMatrixPerspectiveLH 
#define GGMatrixPerspectiveFovRH D3DXMatrixPerspectiveFovRH 
#define GGMatrixPerspectiveFovLH D3DXMatrixPerspectiveFovLH 
#define GGMatrixScaling D3DXMatrixScaling 
#define GGMatrixRotationQuaternion D3DXMatrixRotationQuaternion 
#define GGMatrixLookAtRH D3DXMatrixLookAtRH 
#define GGMatrixLookAtLH D3DXMatrixLookAtLH 
#define GGMatrixOrthoRH D3DXMatrixOrthoRH 
#define GGMatrixOrthoLH D3DXMatrixOrthoLH 
#define GGMatrixOrthoOffCenterRH D3DXMatrixOrthoOffCenterRH 
#define GGMatrixOrthoOffCenterLH D3DXMatrixOrthoOffCenterLH 
#define GGMatrixRotationYawPitchRoll D3DXMatrixRotationYawPitchRoll 
#define GGMatrixIsIdentity D3DXMatrixIsIdentity
#define GGMatrixReflect D3DXMatrixReflect
#define GGMatrixRotationAxis D3DXMatrixRotationAxis
#define GGPlaneFromPoints D3DXPlaneFromPoints
#define GGVec3TransformCoord D3DXVec3TransformCoord
#define GGToRadian D3DXToRadian
#define GGToDegree D3DXToDegree
#define GGQuaternionRotationAxis D3DXQuaternionRotationAxis
#define GGPlaneNormalize D3DXPlaneNormalize
#define GGPlaneTransform D3DXPlaneTransform
#define GGVec3Dot D3DXVec3Dot
#define GGPlaneFromPointNormal D3DXPlaneFromPointNormal
#define GGVec3TransformNormal D3DXVec3TransformNormal
#define GGVec3Normalize D3DXVec3Normalize
#define GGVec3Length D3DXVec3Length
#define GGVec3LengthSq D3DXVec3LengthSq
#define GGVec3CatmullRom D3DXVec3CatmullRom
#define GGQuaternionSlerp D3DXQuaternionSlerp
#define GGVec3Cross D3DXVec3Cross
#define GGPlaneDotCoord D3DXPlaneDotCoord
#define GGIntersectTri D3DXIntersectTri
#define GGVec4Transform D3DXVec4Transform
#define GGVec4Normalize D3DXVec4Normalize
#define GGVec4Dot D3DXVec4Dot
#define GGSetTransform(index,matrix) m_pD3D->SetTransform(index,matrix); //m_pD3D->SetTransform
#define GGGetTransform(index,matrix) m_pD3D->GetTransform(index,matrix); //m_pD3D->GetTransform
#define GGSetEffectFloat(handle,value) m_pEffect->SetFloat(handle,value);
#define GGSetEffectVector(handle,value) m_pEffect->SetVector(handle,value);
#define GGSetEffectMatrix(handle,value) m_pEffect->SetMatrix(handle,value);
#define GGSetEffectMatrixTransposeArray(handle,value,count) m_pEffect->SetMatrixTransposeArray(handle,value,count);

#define GGDECLTYPE_FLOAT2 D3DDECLTYPE_FLOAT2
#define GGDECLTYPE_FLOAT3 D3DDECLTYPE_FLOAT3
#define GGDECLMETHOD_DEFAULT D3DDECLMETHOD_DEFAULT
#define GGDECLUSAGE_POSITION D3DDECLUSAGE_POSITION
#define GGDECLUSAGE_POSITIONT D3DDECLUSAGE_POSITIONT
#define GGDECLUSAGE_NORMAL D3DDECLUSAGE_NORMAL
#define GGDECLUSAGE_TEXCOORD D3DDECLUSAGE_TEXCOORD
#define GDECL_END D3DDECL_END

#define GGFVF_DIFFUSE D3DFVF_DIFFUSE
#define GGFVF_XYZ D3DFVF_XYZ
#define GGFVF_XYZRHW D3DFVF_XYZRHW
#define GGFVF_NORMAL D3DFVF_NORMAL
#define GGFVF_SPECULAR D3DFVF_SPECULAR
#define GGFVF_TEX1 D3DFVF_TEX1
#define GGFVF_TEX2 D3DFVF_TEX2
#define GGFVF_TEX3 D3DFVF_TEX3
#define GGFVF_TEX4 D3DFVF_TEX4
#define GGFVF_TEX5 D3DFVF_TEX5
#define GGFVF_TEX6 D3DFVF_TEX6
#define GGFVF_TEX7 D3DFVF_TEX7
#define GGFVF_TEX8 D3DFVF_TEX8
#define GGFVF_TEXCOORDSIZE1 D3DFVF_TEXCOORDSIZE1
#define GGFVF_TEXCOORDSIZE2 D3DFVF_TEXCOORDSIZE2
#define GGFVF_TEXCOORDSIZE3 D3DFVF_TEXCOORDSIZE3
#define GGFVF_TEXCOORDSIZE4 D3DFVF_TEXCOORDSIZE4
#define GGFVF_TEXCOUNT_MASK D3DFVF_TEXCOUNT_MASK
#define GGFVF_TEXCOUNT_SHIFT D3DFVF_TEXCOUNT_SHIFT
#define GGFVF_PSIZE D3DFVF_PSIZE

#define GGFMT_UNKNOWN D3DFMT_UNKNOWN
#define GGFMT_R8G8B8 D3DFMT_R8G8B8
#define GGFMT_A8R8G8B8 D3DFMT_A8R8G8B8
#define GGFMT_X8R8G8B8 D3DFMT_X8R8G8B8
#define GGFMT_R5G6B5 D3DFMT_R5G6B5
#define GGFMT_X1R5G5B5 D3DFMT_X1R5G5B5
#define GGFMT_A1R5G5B5 D3DFMT_A1R5G5B5
#define GGFMT_A4R4G4B4 D3DFMT_A4R4G4B4
#define GGFMT_R3G3B2 D3DFMT_R3G3B2
#define GGFMT_A8 D3DFMT_A8
#define GGFMT_A8R3G3B2 D3DFMT_A8R3G3B2
#define GGFMT_X4R4G4B4 D3DFMT_X4R4G4B4
#define GGFMT_A2B10G10R10 D3DFMT_A2B10G10R10
#define GGFMT_A8B8G8R8 D3DFMT_A8B8G8R8
#define GGFMT_X8B8G8R8 D3DFMT_X8B8G8R8
#define GGFMT_G16R16 D3DFMT_G16R16
#define GGFMT_A2R10G10B10 D3DFMT_A2R10G10B10
#define GGFMT_A16B16G16R16  D3DFMT_A16B16G16R16
#define GGFMT_A8P8  D3DFMT_A8P8
#define GGFMT_P8 D3DFMT_P8
#define GGFMT_L8 D3DFMT_L8
#define GGFMT_A8L8 D3DFMT_A8L8
#define GGFMT_A4L4 D3DFMT_A4L4
#define GGFMT_V8U8 D3DFMT_V8U8
#define GGFMT_L6V5U5 D3DFMT_L6V5U5
#define GGFMT_X8L8V8U8 D3DFMT_X8L8V8U8
#define GGFMT_Q8W8V8U8 D3DFMT_Q8W8V8U8
#define GGFMT_V16U16 D3DFMT_V16U16
#define GGFMT_A2W10V10U10 D3DFMT_A2W10V10U10
#define GGFMT_UYVY D3DFMT_UYVY
#define GGFMT_R8G8_B8G8 D3DFMT_R8G8_B8G8
#define GGFMT_YUY2 D3DFMT_YUY2
#define GGFMT_G8R8_G8B8 D3DFMT_G8R8_G8B8
#define GGFMT_DXT1 D3DFMT_DXT1
#define GGFMT_DXT2 D3DFMT_DXT2
#define GGFMT_DXT3 D3DFMT_DXT3
#define GGFMT_DXT4 D3DFMT_DXT4
#define GGFMT_DXT5 D3DFMT_DXT5
#define GGFMT_D16_LOCKABLE D3DFMT_D16_LOCKABLE
#define GGFMT_D32 D3DFMT_D32
#define GGFMT_D15S1 D3DFMT_D15S1
#define GGFMT_D24S8 D3DFMT_D24S8
#define GGFMT_D24X8 D3DFMT_D24X8
#define GGFMT_D24X4S4 D3DFMT_D24X4S4
#define GGFMT_D16 D3DFMT_D16
#define GGFMT_D32F_LOCKABLE D3DFMT_D32F_LOCKABLE
#define GGFMT_D24FS8 D3DFMT_D24FS8
#define GGFMT_D32_LOCKABLE D3DFMT_D32_LOCKABLE
#define GGFMT_S8_LOCKABLE D3DFMT_S8_LOCKABLE
#define GGFMT_L16 D3DFMT_L16
#define GGFMT_VERTEXDATA D3DFMT_VERTEXDATA
#define GGFMT_INDEX16 D3DFMT_INDEX16
#define GGFMT_INDEX32 D3DFMT_INDEX32
#define GGFMT_Q16W16V16U16 D3DFMT_Q16W16V16U16
#define GGFMT_MULTI2_ARGB8  D3DFMT_MULTI2_ARGB8
#define GGFMT_R16F D3DFMT_R16F
#define GGFMT_G16R16F D3DFMT_G16R16F
#define GGFMT_A16B16G16R16F D3DFMT_A16B16G16R16F
#define GGFMT_R32F D3DFMT_R32F
#define GGFMT_G32R32F D3DFMT_G32R32F
#define GGFMT_A32B32G32R32F D3DFMT_A32B32G32R32F
#define GGFMT_CxV8U8 D3DFMT_CxV8U8
#define GGFMT_A1 D3DFMT_A1
#define GGFMT_A2B10G10R10_XR_BIAS D3DFMT_A2B10G10R10_XR_BIAS
#define GGFMT_BINARYBUFFER D3DFMT_BINARYBUFFER
#define GGFMT_FORCE_DWORD D3DFMT_FORCE_DWORD

#define GGCOLOR D3DXCOLOR
#define GGCOLOR_ARGB D3DCOLOR_ARGB
#define GGCOLOR_RGBA D3DCOLOR_RGBA
#define GGCOLOR_XRGB D3DCOLOR_XRGB

#endif

#endif

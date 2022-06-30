#ifndef _TGC3DENGINE_H_
#define _TGC3DENGINE_H_

//
// 3D Engine Class
//
// Store legacy information for DB language such as lights,
// camera and other resources in a centrally located class

// Includes and pragmas
#include "d3d9.h"
#include "d3dx9.h"
#include "d3d10.h"
#include "d3dx10.h"

// DirectX Name Override
typedef ID3D10Device	IDirect3DDevice;

// Common Defines
#define MAX_CAMERAS	256
#define MAX_LIGHTS	8

// Common Class
class C3DEngine
{
	public:

		// Core Foundation
		IDirect3DDevice* m_pD3DDevice;
		ID3D10EffectPool* m_pEffectPool;

		// Common formats to use
		DXGI_FORMAT DepthFormat;

		// Engine main render target / depth buffer
		ID3D10RenderTargetView* pRTV;
		ID3D10DepthStencilView* pDSV;

		// Current depth buffer updated during the camera loop, and used in the Basic3D code (clearing stencil)
		ID3D10DepthStencilView* pCurrentDepthStencil;
		ID3D10ShaderResourceView* pCurrentDepthStencilRV;

//		// Camera Structures (NOT USED)
//		struct sCamera
//		{
//			// camera matrices
//			D3DXMATRIX matView;
//			D3DXMATRIX matProj;
//			D3DXMATRIX matInvProj;
//		};
//		sCamera m_Camera [ MAX_CAMERAS ];

//		// Light Structures (NOT USED)
//		struct sLight
//		{
//			// light values
//			float fRange;
//
//			// light vectors
//			D3DXVECTOR3 vPos;
//			D3DXVECTOR3 vDir;
//		};
//		sLight m_Light [ MAX_LIGHTS ];

		// General Lighting Values
		D3DXVECTOR4				vecAmbientLight;

		// Fog Values
		int						iFogMode;
		float					fFogStartDistance;
		float					fFogEndDistance;
		D3DXVECTOR4				vecFogColor;

	public:

//		// Camera Commands
//		D3DXMATRIX* GetProjMatrix ( int iCamIndex ) { return &m_Camera [ iCamIndex ].matProj; }
//		D3DXMATRIX* GetViewMatrix ( int iCamIndex ) { return &m_Camera [ iCamIndex ].matView; }

//		// Light Commands
//		D3DXVECTOR3* GetLightPositionVector ( int iLightIndex ) { return &m_Light [ iLightIndex ].vPos; }
};


// Useful Macros and Defines
#define DARKSDK __declspec ( dllexport )
#define DBPRO_GLOBAL 
#define SAFE_MEMORY( p )       { if ( p == NULL ) return false; }
#define MAX_STRING				256
#define SDK_BOOL int
#define SDK_FLOAT DWORD
#define SDK_LPSTR DWORD
#define SDK_RETFLOAT(f) *(DWORD*)&f 
#define SDK_RETSTR DWORD pDestStr,

// Only define once per link-object
#endif
//
// Common/Private Functions Header
//

#ifndef _COMMONC_H_
#define _COMMONC_H_

// Defines
#pragma warning(disable : 4530)

#include "windows.h"
#include "directx-macros.h"
#include "cpositionc.h"
#include ".\..\camera\ccameradatac.h"
#include ".\..\light\clightdatac.h"
#include "globstruct.h"
#include ".\..\error\cerror.h"
#include "BoxCollision\CCollision.h"
#include "CObjectManagerC.h"
#include "cObjectDataC.h"
#include "cLightMaps.h"

// from main INCLUDE folder
#include "CImageC.h"
#include "CCameraC.h"
#include "CVectorC.h"

// we use this define whenever we create an object, it checks if the specified ID
// is within the current range, if it is not we will allocate some new memory and
// resize the current list
#define ID_ALLOCATION( a )		{ if ( a >= g_iObjectListCount ) ResizeObjectList ( a ); }
#define ID_MESH_ALLOCATION( a )	{ if ( a >= g_iRawMeshListCount ) ResizeRawMeshList ( a ); }

//////////////////////////////////////////////////////////////////////////////////
// VERTEX AND PIXEL SHADER STRUCTURES ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#define	MAX_VERTEX_SHADERS		30
#define	MAX_PIXEL_TEXTURES		8
#define MAX_EFFECTS				65535

struct sVertexShader
{
	LPGGVERTEXSHADER			pVertexShader;		// vertex shader object
	LPGGVERTEXLAYOUT			pVertexDec;			// vertex shader declaration

	DWORD							dwDecArrayCount;	// vertex shader dec count
	GGVERTEXELEMENT*				pDecArray;			// vertex shader dec elements
	
	sVertexShader ( )
	{
		memset ( this, 0, sizeof ( sVertexShader ) );
	}
};

struct sPixelShader
{
	LPGGPIXELSHADER			pPixelShader;		// pixel shader object
	
	sPixelShader ( )
	{
		memset ( this, 0, sizeof ( sPixelShader ) );
	}
};

struct sEffectItem
{
	cSpecialEffect*					pEffectObj;		// effect ptr

	sEffectItem ( )
	{
		memset ( this, 0, sizeof ( sEffectItem ) );
		pEffectObj = new cExternalEffect;
	}
	~sEffectItem ( )
	{
		SAFE_DELETE ( pEffectObj );
	}
};

struct sFullCollisionResult
{
	bool bUsed;
	float fClosestDist;
	GGVECTOR3 vecPos;
	GGVECTOR3 vecNormal;
	GGVECTOR3 vecDifference;
	int iTextureIndex;
	float fTextureU, fTextureV;
	int iPolysChecked;
	DWORD dwArbitaryValue;
	bool bCollidedAgainstFloor;
};

struct sFullPickResult
{
	int iObjectID;
	float fPickDistance;
	GGVECTOR3 vecPickVector;
};
extern sFullPickResult g_DBPROPickResult;

enum eSortOrder
{
    E_SORT_BY_TEXTURE = 0,
    E_SORT_BY_NONE    = 1,
    E_SORT_BY_OBJECT  = 2,
    E_SORT_BY_DEPTH   = 3,
};

extern CObjectManager					m_ObjectManager;
extern int*								g_ObjectListRef;
extern int								g_iObjectListRefCount;
extern sObject**						g_ObjectList;
extern int								g_iObjectListCount;
extern bool								g_bGlobalVBIBUsageFlag;
extern eSortOrder                       g_eGlobalSortOrder;
extern sMesh**							g_RawMeshList;
extern int								g_iRawMeshListCount;
extern sVertexShader					m_VertexShaders [ MAX_VERTEX_SHADERS ];
extern sPixelShader						m_PixelShaders  [ MAX_VERTEX_SHADERS ];
extern sEffectItem*						m_EffectList    [ MAX_EFFECTS ];
extern sFullCollisionResult				g_DBPROCollisionResult;

// Shadow System globals
extern GGVECTOR3						g_vecShadowPosition;
extern int								g_iShadowPositionFixed;

// MIKE 140503 - global timer for animation
extern float   g_fInvTicksPerSec;
extern float   g_fInvTicksPerMs;
extern __int64 g_i64StartTime;
extern __int64 g_i64LastTime;

extern HINSTANCE						g_GFX;
extern HINSTANCE						m_hData;
extern LPGGDEVICE						m_pD3D;
extern LPGGIMMEDIATECONTEXT				m_pImmediateContext;
extern LPGGRENDERTARGETVIEW				m_pRenderTargetView;
extern LPGGDEPTHSTENCILVIEW				m_pDepthStencilView;
extern LPGGSHADERRESOURCEVIEW			m_pDepthStencilResourceView;
#ifdef DX11
extern LPGGDEPTHSTENCILSTATE			m_pDepthStencilState;
extern LPGGDEPTHSTENCILSTATE			m_pDepthNoWriteStencilState;
extern LPGGDEPTHSTENCILSTATE			m_pDepthDisabledStencilState;
extern LPGGRASTERIZERSTATE				m_pRasterState;
extern LPGGRASTERIZERSTATE				m_pRasterStateNoCull;
extern LPGGRASTERIZERSTATE				m_pRasterStateDepthBias;
extern LPGGBLENDSTATE					m_pBlendStateAlpha;
extern LPGGBLENDSTATE					m_pBlendStateNoAlpha;
extern LPGGBLENDSTATE					m_pBlendStateShadowBlend;

#endif
extern GGCAPS							m_Caps;

DARKSDK void UpdateViewProjForMotionBlur ( void );
DARKSDK void UpdateAnimationCycle ( void );
DARKSDK void UpdateOnce			( void );
DARKSDK void Update				( void );
DARKSDK void UpdateGhostLayer	( void );
DARKSDK void UpdateNoZDepth		( void );
DARKSDK void AutomaticStart		( void );
DARKSDK void AutomaticEnd		( void );
DARKSDK void GetMeshData		( int iMeshID, DWORD* pdwFVF, DWORD* pdwFVFSize, DWORD* pdwVertMax, LPSTR* pData, DWORD* dwDataSize, bool bLockData );
DARKSDK void SetMeshData		( int iMeshID, DWORD dwFVF, DWORD dwFVFSize, LPSTR pMeshData, DWORD dwVertMax );

DARKSDK void  ColPosition ( int iID, float fX, float fY, float fZ );
DARKSDK float GetXColPosition ( int iID );
DARKSDK float GetYColPosition ( int iID );
DARKSDK float GetZColPosition ( int iID );

LPSTR GetReturnStringFromWorkString		( void );
bool  DoesFileExist						( LPSTR pFilename );

bool ConfirmObjectInstance				( int iID );
bool ConfirmObject						( int iID );
bool CheckObjectExist					( int iID );
bool ConfirmNewObject					( int iID );
bool ConfirmObjectAndImage				( int iID, int iImage );
bool ConfirmObjectAndLimbInstance		( int iID, int iLimbID );
bool ConfirmObjectAndLimb				( int iID, int iLimbID );
bool ConfirmMesh						( int iID );
bool ConfirmNewMesh						( int iID );
bool ConfirmEffect						( int iEffectID );
bool ConfirmNewEffect					( int iEffectID );

void  ResizeObjectList					( int iSize );
void  AddObjectToObjectListRef			( int iID );
void  RemoveObjectFromObjectListRef		( int iID );
void  ResizeRawMeshList					( int iSize );
float wrapangleoffset					( float da );
void  GetAngleFromPoint					( float x1, float y1, float z1, float x2, float y2, float z2, float* ax, float* ay, float* az );
void  RegenerateLookVectors				( sObject* pObject );
void  AnglesFromMatrix					( GGMATRIX* pmatMatrix, GGVECTOR3* pVecAngles );
void  CheckRotationConversion			( sObject* pObject, bool bUseFreeFlightMode );
void  UpdateOverlayFlag					( sObject* pObject );
void  SetObjectTransparency				( sObject* pObject, int iTransparency );

void  NewObjectAutoCam					( float fRadius );
void  SetupModelFunctionPointers		( void* pModel, HINSTANCE hDLL );
void  SetupPrimitiveFunctionPointers	( void* pData );
void  SetupDefaultProperties			( void* pData );
void  SetupDefaultPosition				( int iID );
bool  PrepareCustomObject				( void* m_pData );
void  GetCullDataFromModel				( int iID );
bool  CreateModelFromCustom				( int iID, void* m_pData );

DARKSDK bool    SetNewObjectFinalProperties		( int iID, float fRadius );
DARKSDK bool    CreateNewObject					( int iID, LPSTR pName );
DARKSDK bool    CreateNewObject                 ( int iID, LPSTR pName, int iFrame );
DARKSDK sFrame* CreateNewFrame                  ( sObject* pObject, LPSTR pName, bool bNewMesh );

DWORD VectortoRGBA								( GGVECTOR3* v, FLOAT fHeight );
bool DoesDepthBufferHaveStencil					( GGFORMAT d3dfmt );
BOOL SupportsUserClippingPlane					();
BOOL SupportsEnvironmentBumpMapping				();
BOOL SupportsCubeMapping						();
BOOL SupportsDotProduct3BumpMapping				();
int GetBitDepthFromFormat						( GGFORMAT Format );
void CreateImageNormalMap						( int iImageIndex );
LPGGCUBETEXTURE CreateNewImageCubeMap			( int i1, int i2, int i3, int i4, int i5, int i6 );
void SetVertexShaderMesh						( int iID, DWORD dwFVFOverride );
void FreeVertexShaderMesh						( int iID ) ;
void CleatTextureStageFlags						( void );
void ClearTextureSettings						( int iID );
void CreateNewOrSharedEffect					( sMesh* pMesh, bool bChangeMesh );

void COMMONSetTransform							( int index, GGMATRIX* matrix );
void COMMONGetTransform							( int index, GGMATRIX* matrix );
void COMMONSetEffectFloat						( GGHANDLE handle, float value );
void COMMONSetEffectVector						( GGHANDLE handle, GGVECTOR4* value );
void COMMONSetEffectMatrix						( GGHANDLE handle, GGMATRIX* value );
void COMMONSetEffectMatrixTransposeArray		( GGHANDLE handle, GGMATRIX* value, UINT count );

#endif _COMMONC_H_

//
// CObjectManager Functions Header
//

//#ifndef _COBJECTMANAGER_H_
//#define _COBJECTMANAGER_H_

//////////////////////////////////////////////////////////////////////////////////
// INCLUDE OBJECT DATA ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#include "cObjectDataC.h"
#include ".\..\Camera\cCameraDataC.h"
#include <vector>

//////////////////////////////////////////////////////////////////////////////////
// INLINE MACROS /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

// Externals for DBO/Manager relationship
extern std::vector< sMesh* >		g_vRefreshMeshList;
extern std::vector< sObject* >		g_vAnimatableObjectList;
extern int							g_iSortedObjectCount;
extern sObject**					g_ppSortedObjectList;

//////////////////////////////////////////////////////////////////////////////////
// RENDER STRUCTURES /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#define MAXTEXTURECOUNT 8
struct sRenderStates
{
	// render filter states
	DWORD			dwAddressU [ MAXTEXTURECOUNT ];
	DWORD			dwAddressV [ MAXTEXTURECOUNT ];
	DWORD			dwMagState [ MAXTEXTURECOUNT ];
	DWORD			dwMinState [ MAXTEXTURECOUNT ];
	DWORD			dwMipState [ MAXTEXTURECOUNT ];

	// render state flags
	bool			bWireframe;
	bool			bLight;
	bool			bCull;
	int				iCullMode;
	bool			bFog;
	int				iAmbient;
	bool			bTransparency;
	DWORD			dwAlphaTestValue;
	bool			bReservedBOOL; // used to be bGhost;
	bool			bZRead;
	bool			bZWrite;
	int				iReservedINT; // used to be iGhostMode;
	bool			bZBiasActive;
	float			fZBiasSlopeScale;
	float			fZBiasDepth;

	// render state data fog when ghosts override fog
	bool			bFogOverride;
	DWORD			dwFogColor;

	// render state data
	DWORD			dwAmbientColor;
	DWORD			dwCullDirection;
	DWORD			dwGlobalCullDirection;
	D3DMATERIAL9	gWhiteDefaultMaterial;
	bool			bIgnoreDiffuse;

	// restriction states (set when global render states use, ie instance alpha factor)
	bool			bNoMeshAlphaFactor;

	// camera specific states
	float			fStoreCameraFOV;
	float			fObjectFOV;

	// U75 - 060410 - control whether scene uses a color instead of textures/effects
	bool			bOverrideAllTexturesAndEffects;	
	DWORD			dwOverrideAllWithColor;

	// U77 - 270111 - override control for clipplane (HLSL and FF mutually exclusive)
	bool			bOverriddenClipPlaneforHLSL;	
};

class CObjectManager
{
	public:
		struct sVertexData
		{
			// vertex data structure

			DWORD							dwFormat;				// data format e.g. xyz
			LPGGVERTEXLAYOUT				pVertexDec;				// custom FVF dec
			DWORD							dwPosition;
			DWORD							dwMaxVertexCount;		// maximum number of vertices allowed
			DWORD							dwCurrentVertexCount;	// current number of vertices

			LPGGVERTEXBUFFER				pVB;					// vertex buffer
			sVertexData*					pNext;					// pointer to next node

			bool							bBufferLocked;			// during fast-update
			BYTE*							pfLockedData;			// can lock once - write many times

			// constructor

			sVertexData ( );
			~sVertexData ( );
		};

		struct sIndexData
		{
			// index data structure

			DWORD					dwMaxIndexCount;		// maximum number of indices allowed
			DWORD					dwCurrentIndexCount;	// current index count
			LPGGINDEXBUFFER			pIB;					// index buffer
			sIndexData*				pNext;					// pointer to next node

			// constructor

			sIndexData ( );
			~sIndexData ( );
		};

		IGGVertexBuffer*		m_ppCurrentVBRef;				// current VBs being used
		IGGVertexBuffer*		m_ppLastVBRef;					// last VBs being used
		IGGIndexBuffer*		m_ppCurrentIBRef;				// current IBs being used
		IGGIndexBuffer*		m_ppLastIBRef;					// last IBs being used

		DWORD						m_dwCurrentShader;				// current shader value
		DWORD						m_dwCurrentFVF;					// current FVF value
		DWORD						m_dwLastShader;					// last shader value
		DWORD						m_dwLastFVF;					// last FVF value

		int							m_iCurrentTexture;				// current texture being used
		int							m_iLastTexture;					// last texture
		DWORD						m_dwLastTextureCount;			// to free old stages
		bool						m_bUpdateTextureList;			// flag to update sorted texture list
		DWORD						m_dwSkipVisibilityListMask;		// some cameras do not need to sort objects each time (can use existing object list) 
		bool						m_bUpdateVertexDecs;			// ensure we can restore at start of new object render cycle
		bool						m_bUpdateStreams;
		int							m_iLastCount;
		int							m_iListCount;					// number of objects in list
		int							m_iVisibleObjectCount;
		bool*						m_pbMarkedList;
		sObject**					m_ppSortedObjectVisibleList;	// a list of objects that are visible in the scene
		sVertexData*				m_pVertexDataList;				// vertex buffer cache
		sIndexData*					m_pIndexDataList;				// index buffer cache

        std::vector< sObject* >     m_vVisibleObjectList;
        std::vector< sObject* >     m_vVisibleObjectEarly;
        std::vector< sObject* >     m_vVisibleObjectTransparent;
        std::vector< sObject* >     m_vVisibleObjectNoZDepth;
        std::vector< sObject* >     m_vVisibleObjectStandard;

		sRenderStates				m_RenderStates;					// global render state settings
		bool						m_bGlobalShadows;				// not used any more
		bool						g_bObjectReplacedUpdateBuffers;	// flag to say buffers need replacing
		LPGGSURFACE			m_pImposterRendererDSV;			// common depth surface used to render Imposters
        tagCameraData*              m_pCamera;                      // Current camera details - updated in UpdateLayer

		sVertexData*	FindVertexBuffer		( DWORD dwFVF, LPGGVERTEXLAYOUT pVertexDec, DWORD dwSize, DWORD dwVertexCount, DWORD dwIndexCount, bool bUsesItsOwnBuffers, int iType );
		sIndexData*		FindIndexBuffer			( DWORD dwIndexCount, bool bUsesItsOwnBuffers );
		void			ResetIBRef				(void);
		bool			SortTextureList			( void );
		bool			SortVisibilityList		( void );
		bool			PreSceneSettings		( void );
		bool			PreDrawSettings			( void );
		bool			SetMeshMaterial			( sMesh* pMesh, D3DMATERIAL9* pMaterial );
		void			SetMeshDepthStates		( sMesh* pMesh, bool bForceState );
		bool			SetMeshRenderStates		( sMesh* pMesh );
		bool			SetMeshTextureStates	( sMesh* pMesh );
		bool			DrawMesh				( sMesh* pMesh, bool bIgnoreOwnMeshVisibility , sObject* pObject = NULL, sFrame* pFrame = NULL);
		bool			DrawMesh				( sMesh* pMesh );

		int				SwitchRenderTargetToDepth ( int iFlag );

		bool			ShaderStart				( sMesh* pMesh, LPGGRENDERTARGETVIEW* pCurrentRenderTarget, LPGGDEPTHSTENCILVIEW* pCurrentDepthTarget, UINT* puPasses, bool* pbEffectRendering, bool* pbLocalOverrideAllTexturesAndEffects );
		bool			ShaderPass				( sMesh* pMesh, UINT uPass, UINT uPasses, bool bEffectRendering, bool bLocalOverrideAllTexturesAndEffects, LPGGRENDERTARGETVIEW pCurrentRenderTarget, LPGGDEPTHSTENCILVIEW pCurrentDepthTarget, sObject* pObject = NULL );
		bool			ShaderPassEnd			( sMesh* pMesh, bool bEffectRendering );
		bool			ShaderFinish			( sMesh* pMesh, LPGGRENDERTARGETVIEW pCurrentRenderTarget, LPGGDEPTHSTENCILVIEW pCurrentDepthTarget );
		bool			DrawObjectEx			( sObject* pObject );
		bool			DrawObject				( sObject* pObject, bool bFrustrumCullMeshes, bool recursive = false );
		bool			PostDrawRestores		( void );
		bool			Reset					( void );
		bool			SetVertexShader			( sMesh* pMesh );
		bool			SetInputStreams			( sMesh* pMesh );

	public:

		CObjectManager ( )
		{
			// use setup to set members - global manager not refreshed when display mode switched
			Setup ();
		}

		~CObjectManager ( )
		{
			// use free to clean up - global manager not refreshed when display mode switched
			Free ();
		}

		bool Setup							( void );
		bool Free							( void );
		void UpdateViewProjForMotionBlur	( void );
		void UpdateInitOnce					( void );
		bool UpdateInit						( void );
		bool UpdateLayer					( int iLayer );				// update the core
		bool UpdateLayerInner				( int iLayer );				// update the core
		bool Update							( void );					// update
		bool UpdateGhostLayer				( void );
		bool UpdateNoZLayer					( void );					// update

		float CalculateObjectDistanceFromCamera ( sObject* pObject );
		float CalculateObjectCenterDistanceFromCamera(sObject* pObject);
		bool AddObjectMeshToBuffers			( sMesh* pMesh, bool bUsesItsOwnBuffers );			// add an object mesh to the buffers
		bool AddObjectToBuffers				( sObject* pObject );		// adds an object to the buffers
		bool FlagAllObjectMeshesUsingBuffer	( sVertexData* pVertexData, sIndexData* pIndexData );
		bool RemoveBuffersUsedByObjectMeshDirectly	( sMesh* pMesh );
		bool RemoveBuffersUsedByObjectMesh	( sMesh* pMesh, bool bRecurse = true );
		bool AddFlaggedObjectsBackToBuffers	( void );
		bool ReplaceAllFlaggedObjectsInBuffers ( void );
		bool RemoveBuffersUsedByObject		( sObject* pObject );		// remove buffers used by object
		bool RemoveObjectFromBuffers		( sObject* pObject );		// removes an object from buffers
		bool UpdateObjectMeshInBuffer		( sMesh* pMesh );
		bool UpdateAllObjectsInBuffers		( void );
		bool CompleteUpdateInBuffers		( void );
		bool QuicklyUpdateObjectMeshInBuffer ( sMesh* pMesh, DWORD dwVertexFrom, DWORD dwVertexTo );
		bool RenewReplacedMeshes			( sObject* pObject );
		bool RefreshObjectInBuffer			( sObject* pObject );
		void RemoveTextureRefFromAllObjects ( LPGGTEXTURE pTextureRef );

		bool UpdateObjectListSize			( int iSize );				// updates object list size
		bool UpdateTextures					( void );
		void UpdateAnimationCyclePerObject	( sObject* pObject );
		bool UpdateAnimationCycle			( void );
		void UpdateOneVisibleObject			( sObject* pObject );
		bool UpdateOnlyVisible				( void );

		void SetGlobalShadowsOn				( void );
		void SetGlobalShadowsOff			( void );

		int GetVisibleObjectCount				( void );
		sObject** GetSortedObjectVisibleList	( void );
};

//#endif _COBJECTMANAGER_H_

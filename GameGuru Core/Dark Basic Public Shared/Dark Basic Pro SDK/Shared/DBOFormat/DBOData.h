#ifndef _DBODATA_H_
#define _DBODATA_H_

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include "directx-macros.h"
#include "..\global.h"
#include "d3d9types.h"
#include "d3dx9mesh.h"

struct sCollisionData;		// collision data
struct sObject;				// dbo object
struct sMesh;				// mesh
struct sFrame;				// frame
struct sTexture;			// texture
struct sMeshGroup;

// delete callback - used for external DLLs
typedef void ( *ON_OBJECT_DELETE_CALLBACK ) ( int Id, int userData );

struct sEffectConstantData
{
	// effect data
	GGMATRIX 	matWorld;
	GGMATRIX 	matView;
	GGMATRIX 	matProj;
	GGMATRIX 	matWorldView;
	GGMATRIX 	matViewProj;
	GGMATRIX 	matWorldViewProj;
	GGMATRIX 	matWorldViewInv;
	GGMATRIXA16	matWorldInv;
	GGMATRIXA16	matViewInv;
	GGMATRIXA16	matProjInv;

	GGMATRIX 	matWorldT;
	GGMATRIX 	matViewT;
	GGMATRIX 	matProjT;
	GGMATRIX 	matWorldViewT;
	GGMATRIX 	matViewProjT;
	GGMATRIX 	matWorldViewProjT; // 270515 - reclassifying this to ViewProjInv (as matWorldViewProjT is never used?!)
	GGMATRIX 	matWorldViewInvT;
	GGMATRIXA16	matWorldInvT;
	GGMATRIXA16	matViewInvT;

	GGVECTOR4		vecLightDir;
	GGVECTOR4		vecLightDirInv;
	GGVECTOR4		vecLightPos;
	float			fLightRange;

	GGVECTOR4		vecEyePos;
	GGVECTOR4		vecCameraPosition;

	float			fAlphaOverride;

	float			fBoneCount;
	GGMATRIX 	matBoneMatrixPalette[256];

	GGVECTOR4		vecUVScaling;

	DWORD			dwReserved1;
	DWORD			dwReserved2;
	DWORD			dwReserved3;
	DWORD			dwReserved4;
	DWORD			dwReserved5;
	DWORD			dwReserved6;
	DWORD			dwReserved7;
	DWORD			dwReserved8;
};

class cSpecialEffect
{
	// base class for special effects

	public:
		LPGGEFFECT			m_pEffect;
		char				m_pEffectName [ MAX_STRING ];
		int					m_iEffectID;

		GGEFFECT_DESC		m_EffectDesc;
		bool				m_bTranposeToggle;
		
		GGHANDLE			m_MatWorldEffectHandle;
		GGHANDLE			m_MatViewEffectHandle;
		GGHANDLE			m_MatProjEffectHandle;
		GGHANDLE			m_MatWorldViewEffectHandle;
		GGHANDLE			m_MatViewProjEffectHandle;
		GGHANDLE			m_MatWorldViewProjEffectHandle;
		GGHANDLE			m_MatWorldInverseEffectHandle;
		GGHANDLE			m_MatViewInverseEffectHandle;
		GGHANDLE			m_MatProjectionInverseEffectHandle;
		GGHANDLE			m_MatWorldViewInverseEffectHandle;

		GGHANDLE			m_MatWorldTEffectHandle;
		GGHANDLE			m_MatViewTEffectHandle;
		GGHANDLE			m_MatProjTEffectHandle;
		GGHANDLE			m_MatWorldViewTEffectHandle;
		GGHANDLE			m_MatViewProjTEffectHandle;
		GGHANDLE			m_MatWorldViewProjTEffectHandle;
		GGHANDLE			m_MatWorldInverseTEffectHandle;
		GGHANDLE			m_MatViewInverseTEffectHandle;
		GGHANDLE			m_MatWorldViewInverseTEffectHandle;

		GGHANDLE			m_LightDirHandle;
		GGHANDLE			m_LightDirInvHandle;
		GGHANDLE			m_LightPosHandle;
		GGHANDLE			m_VecCameraPosEffectHandle;
		GGHANDLE			m_VecEyePosEffectHandle;
		GGHANDLE			m_AlphaOverrideHandle;

		GGHANDLE			m_TimeEffectHandle;
		GGHANDLE			m_SinTimeEffectHandle;
		GGHANDLE			m_VecClipPlaneEffectHandle; // 270111, then 140411 (this added 4 bytes to DBO, messes up all plugins)

		GGHANDLE			m_BoneCountHandle;
		GGHANDLE			m_BoneMatrixPaletteHandle;

		GGHANDLE			m_DeltaTimeEffectHandle;
		GGHANDLE			m_UVScalingHandle;
		GGHANDLE			m_ColorTone[4];
		GGPASSHANDLE		m_DepthRenderPassHandle;
		GGHANDLE			m_ReservedHandle8;

		LPSTR				m_pDefaultXFile;
		BOOL				m_bGenerateNormals;

		// flags that specify what the effect requires
		BOOL				m_bUsesNormals;
		BOOL				m_bUsesDiffuse;
		BOOL				m_bUsesTangents;
		BOOL				m_bUsesBinormals;
		BOOL				m_bUsesBoneData;

		// store texture ptrs from loaded effect
		bool				m_bUseShaderTextures;
		DWORD				m_dwTextureCount;
		int					m_iParamOfTexture[32];
		LPGGRENDERTARGETVIEW m_pParamOfTextureRenderView[32];
		sMesh*				m_pXFileMesh;

		DWORD				m_dwRTTexCount;
		LPGGBASETEXTURE		m_pRTTex[32];							// leeadd - 080414 - Needed to track allocations made by shader (so can free them later)
		LPGGSHADERRESOURCEVIEW m_pRTTexView[32];

		// reserved members
		DWORD				m_bDoNotGenerateExtraData;					// leeadd - 050906 - Flag passed in to control if FX data auto-generated (tangents/binormals/normals)
		DWORD				m_dwUseDynamicTextureMask;					// leeadd - 180107 - Whether dynamic effect from TEXTURE OBJECT should apply to an effect that is using its own textures
		bool				m_bUsesAtLeastOneRT;						// leeadd - 200310 - support for RTs in shader
		bool				m_bPrimaryEffectForCascadeShadowMapping;	// leeadd - 250913 - cascade shadow mapping (texture slots 4,5,6,7 overwritten)
		bool				m_bCharacterCreatorTonesOn;
		bool				m_bNeed8BonesPerVertex;						// leeadd - 131018 - needed for large bone models!
		DWORD				m_dwCreatedRTTextureMask;					// leeadd - 200310 - support for RTs in shader

		// additional information on shader
		DWORD				m_dwEffectIndex;
		GGHANDLE			m_GlowIntensityHandle;						// leeadd - 170315 - holds the param for any GlowIntensity param							
		GGHANDLE			m_ToneMix[4];
		GGTECHNIQUEHANDLE	m_hCurrentTechnique;
		DWORD				m_dwReserved8;								// reserved

		// virtual functions
		cSpecialEffect ( );
		virtual ~cSpecialEffect ( );
		virtual bool	Load					( int iEffectID, LPSTR pEffectFile, bool bUseXFile, bool bUseTextures );
		virtual bool	Setup					( sMesh* pMesh );
		virtual void	Mesh					( sMesh* pMesh );
		virtual DWORD	Start					( sMesh* pMesh, GGMATRIX matObject );
		virtual void	End						( void );

		// base functions
		bool			AssignValueHookCore		( LPSTR pName, GGHANDLE hParam, DWORD dwClass, bool bRemove );
		bool			AssignValueHook			( LPSTR pName, GGHANDLE hParam );
		bool			CorrectFXFile			( LPSTR pFile, LPSTR pModifiedFile );
		bool			ParseEffect				( bool bUseEffectXFile, bool bUseEffectTextures );
		void			ApplyEffect				( sMesh* pMesh );
};

enum eCollisionType
{
	// collision types
	COLLISION_NONE,		// none
	COLLISION_SPHERE,	// sphere
	COLLISION_BOX,		// box
	COLLISION_POLYGON	// polygon
};

enum eRotationOrders
{
	// order of rotation
	ROTORDER_XYZ,		// x y z
	ROTORDER_ZYX,		// z y x
	ROTORDER_YZX,		// y z x
	ROTORDER_ZXY		// z x y
};

struct sOffsetMap
{
	// stores a list of offsets to each part of fvf data
	DWORD	dwSize,				// size of data
			dwByteSize,			// size in bytes
			dwFVF,				// actual fvf
			dwX,				// x offset
			dwY,				// y offset
			dwZ,				// z offset
			dwRWH,				// rhw offset
			dwBlend1,			// blend 1 offset
			dwBlend2,			// blend 2 offset
			dwBlend3,			// blend 3 offset
			dwNX,				// normal x offset
			dwNY,				// normal y offset
			dwNZ,				// normal z offset
			dwPointSize,		// point size offset
			dwDiffuse,			// diffuse offset
			dwSpecular,			// specular offset
			dwTU[8],			// tu offsets
			dwTV[8],			// tv offsets
			dwTZ[8],			// tz offsets
			dwTW[8];			// tw offsets
};

struct sBone
{
	// stores bone data
	char							szName [ MAX_STRING ];						// bone name
	DWORD							dwNumInfluences;							// how many vertices does this affect
	DWORD*							pVertices;									// affect vertices - stores indices to link to
	float*							pWeights;									// bone weights
	GGMATRIX 						matTranslation;								// translation matrix

	// reserved members
	DWORD							dwReservedB1;								// reserved - maintain plugin compat.
	DWORD							dwReservedB2;								// reserved - maintain plugin compat.
	DWORD							dwReservedB3;								// reserved - maintain plugin compat.
	
	// constructor and destructor
	sBone  ( );
	~sBone ( );
};

struct sTexture
{
	// texture information
	char							pName [ MAX_STRING ];						// image filename
	int								iImageID;									// image id
	LPGGTEXTURE						pTexturesRef;								// reference to image texture
	LPGGCUBETEXTURE					pCubeTexture;								// cube texture reference
	DWORD							dwStage;									// can change stage order of texture
	DWORD							dwBlendMode;								// blending mode for multitexturing
	DWORD							dwBlendArg1;								// blend argument 1
	DWORD							dwBlendArg2;								// blend argument 2
	DWORD							dwAddressU;									// texture wrap mode settings
	DWORD							dwAddressV;									// texture wrap settings
	DWORD							dwMagState;									// texture filter and mipmap modes
	DWORD							dwMinState;									// min state
	DWORD							dwMipState;									// mip state
	DWORD							dwTexCoordMode;								// texture coordinate index mode
	int								iStartIndex;								// start index
	int								iPrimitiveCount;							// number of primitives

	// lerping
	DWORD							dwBlendArg0;								// U73 - 210309 - extra blend mapping support
	DWORD							dwBlendArgR;								// for lerping textures (arg0 and result)

	// reserved members
	LPGGTEXTUREREF					pTexturesRefView;							// DX11 resource view
	DWORD							dwReservedT2;
	DWORD							dwReservedT3;

	// constructor and destructor
	sTexture  ( );
	~sTexture ( );
};

struct sCollisionData
{
	// contains a list of all collision data ( each mesh will store one of these )
	GGVECTOR3						vecMin;										// origin bounding box min extents
	GGVECTOR3						vecMax;										// origin bounding box max extents
	GGVECTOR3						vecCentre;									// origin centre of object
	float							fRadius;									// origin sphere radius
	float							fScaledRadius;								// precomputed for main object radius
	float							fLargestRadius;								// precomputed all anim combos for culling
	float							fScaledLargestRadius;						// precomputed all anim combos for scaled culling

	// collision information for run-time processing
	bool							bActive;									// whether collision is on or off
	bool							bHasBeenMovedForResponse;					// flagged when entity is moved
	bool							bColCenterUpdated;							// collision center moves when object rotates
	GGVECTOR3						vecColCenter;								// realtime collision center

	// collision method
	DWORD							eCollisionType;								// collision type
	bool							bFixedBoxCheck;								// non-rotated box collision
	bool							bUseBoxCollision;							// flagged to use box collision
	int								iHitOverlapStore;							// stores when 'strike hit' occurs
	int								iResponseMode;								// how the collision produces a result
	bool							bBoundProduceResult;						// bound produces result???

	// special collision assist values
	DWORD							dwArbitaryValue;							// can use used to store a value (material ID for sound/ground effect)
	DWORD							dwPortalBlocker;							// assists in determining if mesh should block a created portal (cosmetic geometry)

	// reserved members
	DWORD							dwCollisionPropertyValue;					// state of collision behaviour (used externally to give object a collision prroperty)
	DWORD							dwReservedCD2;								// reserved - maintain plugin compat.
	DWORD							dwReservedCD3;								// reserved - maintain plugin compat.
	
	// constructor and destructor
	sCollisionData  ( );
	~sCollisionData ( );
};

struct sDrawBuffer
{
	// draw buffer data
	IGGVertexBuffer*				pVertexBufferRef;							// pointer to VB used
	IGGIndexBuffer*					pIndexBufferRef;							// pointer to IB used
	GGPRIMITIVETYPE					dwPrimType;									// render prim type
	DWORD							dwVertexStart;								// start location for vertices in buffer
	DWORD							dwVertexCount;								// number of vertices used by this buffer
	DWORD							dwIndexStart;
	DWORD							dwPrimitiveCount;							// required to determine actual prims to draw
	DWORD							dwFVFSize;									// size of the stream source
	DWORD							dwBaseVertexIndex;							// used to offset vertex base index (setindices)
	LPVOID							pVBListEntryRef;							// ref to obj-manager VB list item
	LPVOID							pIBListEntryRef;							// ref to obj-manager IB list item
	DWORD							dwImmuneToForeColorWipe;					// U75 - 070410 - (was RES1) set by SET OBJECT MASK to allow objects to escape being wiped by COLOR BACKDROP c,b,FORECOLOR

	// reserved members
	DWORD							dwReservedDB1;								// reserved - maintain plugin compat.
	DWORD							dwReservedDB2;								// reserved - maintain plugin compat.
	DWORD							dwReservedDB3;								// reserved - maintain plugin compat.
	
	// constructor
	sDrawBuffer ( );
};

struct sMultiMaterial
{
	// multiple materials
	char							pName [ MAX_STRING ];						// image filename
	LPGGTEXTUREREF					pTexturesRef;								// reference to image texture
	LPGGTEXTUREREF					pTexturesRefN;								// 090217 - added - reference to image texture
	LPGGTEXTUREREF					pTexturesRefS;								// 090217 - added - reference to image texture
	LPGGTEXTUREREF					pTexturesRefG;								// 090217 - added - reference to image texture
	D3DMATERIAL9					mMaterial;									// unique material information
	DWORD							dwIndexStart;								// start of index data for this material
	DWORD							dwIndexCount;								// quantity of index data items
	DWORD							dwPolyCount;								// quantity of polygons to draw
};

struct sMeshMultipleAnimation
{
	// multiple animation information
	sFrame*							pSubFrameList;								// sub frames
	DWORD							dwSubMeshListCount;							// number of sub frames
	int								iCurrentFrame;								// current frame
	int								iNextFrame;									// next frame
	int								iLastFrame;									// last frame
	float							fLastInterp;								// last interpolation value
	float							fInterp;									// interpolation
	DWORD							dwLastTime;									// last time
	DWORD							dwThisTime;									// current time
	bool							bLinked;									// linked flag

	// reserved members
	DWORD							dwReservedMMA1;								// reserved - maintain plugin compat.
	DWORD							dwReservedMMA2;								// reserved - maintain plugin compat.
	DWORD							dwReservedMMA3;								// reserved - maintain plugin compat.
};

struct sMeshFVF
{
	// mesh flexible vertex format data
	DWORD							dwFVFOriginal;								// flexible vertex format original
	DWORD							dwFVF;										// flexible vertex format
	DWORD							dwFVFSize;									// size of flexible vertex format
};

struct sMeshDraw
{
	// mesh draw information
	int								iMeshType;									// put it to handle terrain scene culling (mesh visible flag)
	BYTE*							pOriginalVertexData;						// pointer to original vertex data
	BYTE*							pVertexData;								// pointer to vertex data
	WORD*							pIndices;									// index array
	DWORD							dwVertexCount;								// number of vertices
	DWORD							dwIndexCount;								// number of indices
	int								iPrimitiveType;								// primitive type
	int								iDrawVertexCount;							// number of vertices to be used when drawing
	int								iDrawPrimitives;							// number of actual primitives to draw
	sDrawBuffer*					pDrawBuffer;								// VB and IB buffers used by the mesh

	// reserved members
	DWORD							dwReservedMD1;								// reserved - maintain plugin compat.
	DWORD							dwReservedMD2;								// reserved - maintain plugin compat.
	DWORD							dwReservedMD3;								// reserved - maintain plugin compat.
};

struct sMeshShader
{
	// shader data
	bool							bUseVertexShader;							// flag to control vertex shader
	bool							bOverridePixelShader;						// pixel shader on or off
	bool							bVertexShaderEffectRefOnly;
	GGVERTEXELEMENT					pVertexDeclaration [ MAX_FVF_DECL_SIZE ];	// for custom FVFs
	LPGGVERTEXSHADER				pVertexShader;								// vertex shader
	LPGGVERTEXLAYOUT				pVertexDec;									// vertex shader dec
	LPGGPIXELSHADER					pPixelShader;								// pixel shader handle
	cSpecialEffect*					pVertexShaderEffect;						// built-in shader effect ptr
	char							pEffectName [ MAX_STRING ];
	DWORD							dwForceCPUAnimationMode;					// 200111 - force CPU to take over bone animation (for ray cast intersect detection even if GPU using bone animation)

	// reserved members
	float							fUVScalingU;								// dwReservedMS1 - maintain plugin compat.
	float							fUVScalingV;								// dwReservedMS2 - maintain plugin compat.
	DWORD							dwReservedMS3;								// reserved - maintain plugin compat.
};

struct sMeshBones
{
	// mesh bones
	sBone*							pBones;										// array of bones for mesh
	DWORD							dwBoneCount;								// number of bones in mesh
	sFrame**						pFrameRef;									// stores reference to original frame representing bone (used for limb-based bone collision)
	GGMATRIX**					pFrameMatrices;								// list of all matrices used for this mesh ( in conjunction with bones )
};

struct sMeshTexture
{
	// mesh texture
	bool							bUsesMaterial;								// flag to skip material use
	bool							bAlphaOverride;								// flag to alpha override
	bool							bUseMultiMaterial;							// for multi-material models (bone based type for now)
	DWORD							dwTextureCount;								// number of textures in list
	DWORD							dwMultiMaterialCount;						// size of multimaterial array
	DWORD							dwAlphaOverride;							// tfactor alpha override (true object transparency)
	sTexture*						pTextures;									// texture list
	sMultiMaterial*					pMultiMaterial;								// multimaterial array
	D3DXMATERIAL*					pMaterialBank;								// temp store for all materials of a single mesh
	D3DMATERIAL9					mMaterial;									// unique material information
	DWORD*							pAttributeWorkData;							// stores work data when a mesh has multiple materials

	// reserved members
	float							fSpecularOverride;							// 281116 - overrides "SpecularOverride" in any shader
	DWORD							dwReservedMT2;								// reserved - maintain plugin compat.
	DWORD							dwReservedMT3;								// reserved - maintain plugin compat.
};

struct sMeshInternalProperties
{
	// object maintainance and management
	bool							bAddObjectToBuffers;						// flagged when mesh needs to be added to buffers
	bool							bVBRefreshRequired;							// flagged when vertex data modified
	bool							bMeshHasBeenReplaced;						// shaders can change a mesh (buffer would be replaced if so)
	bool							bVertexTransform;							// have vertices been transformed
	DWORD							dwDrawSignature;							// draw information
	bool							bShaderBoneSkinning;						// indicates if Vertex Shader has taken over bone animation work
	int								iSolidForVisibility;						// filled when mesh used as part of nodetree visibility system
	int								iCastShadowIfStatic;						// occluder shadow value (written by ADD STATIC)
	union
	{
		DWORD						dwMeshID;
	};

	// reserved members
	DWORD							fBoostIntensity;							// reserved - maintain plugin compat.
	DWORD							dwReservedMIP2;								// reserved - maintain plugin compat.
	DWORD							dwReservedMIP3;								// reserved - maintain plugin compat.
};

struct sMeshExternalProperties
{
	// these store the individual render states of the mesh
	bool							bWireframe;									// render state flags for each mesh
	bool							bLight;										// lighting on / off
	bool							bCull;										// culling on / off
	bool							bFog;										// fog on / off
	bool							bAmbient;									// respond to ambient
	bool							bTransparency;								// transparency on / off
	DWORD							dwAlphaTestValue;							// used to restrict what is rendered by alpha value
	bool							bGhost;										// is ghosting on
	bool							bVisible;									// is object visible
	bool							bZRead;										// z buffer
	bool							bZWrite;									// z write
	int								iGhostMode;									// ghost mode index
	bool							bZBiasActive;								// active when using some zbias
	float							fZBiasSlopeScale;							// how much of object to add to Z (0.0-1.0)
	float							fZBiasDepth;								// definitively add a value to Z
	int								iCullMode;									// 1-CCW 2-CW - lee - 040306 - u6rc5 - solve import issue for some CW models
	bool							bShadowBlend;								// 140817 - added for shadow blend state DX11
	float							fMipMapLODBias;								// mike - 230505 - need to be able to set mip map LOD bias on a per mesh basis

	// reserved members
	float							fScrollOffsetU;								// UV data override for shaders
	float							fScrollOffsetV;								// 
	float							fScaleOffsetU;								//
	float							fScaleOffsetV;								// 
	DWORD							dwArtFlags;									// art flags (invert normal green channel, etc)
	GGHANDLE						dl_lights;
	GGHANDLE						dl_lightsVS;
	GGHANDLE						dl_pos[4];
	GGHANDLE						dl_atten[4];
	GGHANDLE						dl_diffuse[4];
	GGHANDLE						dl_angle[4];
};

struct sMesh : public sMeshFVF,
					  sMeshDraw,
					  sMeshShader,
					  sMeshBones,
					  sMeshTexture,
					  sMeshInternalProperties,
					  sMeshExternalProperties,
					  sMeshMultipleAnimation
{
	// contains all data for a mesh
	sCollisionData					Collision;									// collision information

	// reserved members
	DWORD							dwTempFlagUsedDuringUniverseRayCast;		// V119 to fix bugged universe ray cast of large meshes
	DWORD							dwReservedM2;								// reserved - maintain plugin compat.
	DWORD							dwReservedM3;								// reserved - maintain plugin compat.

	// constructor and destructor
	sMesh  ( );
	~sMesh ( );
};

struct sMeshGroup
{
	sMesh* pMesh;
};

struct sFrameID
{
	// frame ID
	char							szName [ MAX_STRING ];						// name
	int								iID;										// local index of frame in build list
};

struct sFrameLinks
{
	// frame links - parent, child and sibling
	sFrame*							pParent;									// parent
	sFrame*							pChild;										// child
	sFrame*							pSibling;									// sibling
};

struct sFrameTransforms
{
	// frame transforms
	GGMATRIX 						matOriginal;								// original matrix
	GGMATRIX 						matTransformed;								// realtime transformed matrix
	GGMATRIX 						matCombined;								// realtime combined matrix
	GGMATRIX 						matAbsoluteWorld;							// includes object world (absolute world)

	// mike 170505 - new matrix for completely custom, physics needs this for implementing it's own matrix
	bool							bOverride;									// flag to override
	GGMATRIX 						matOverride;								// override matrix

	// reserved members
	DWORD							dwReservedFT1;								// reserved - maintain plugin compat.
	DWORD							dwReservedFT2;								// reserved - maintain plugin compat.
	DWORD							dwReservedFT3;								// reserved - maintain plugin compat.
};

struct sFramePosition
{
	// frame position data
	GGMATRIX 						matUserMatrix;								// local frame matrix
	GGVECTOR3						vecOffset;									// local frame orientation
	GGVECTOR3						vecScale;									// scale
	GGVECTOR3						vecRotation;								// rotation
	GGVECTOR3						vecPosition;								// realtime update
	GGVECTOR3						vecDirection;								// realtime update
	bool							bVectorsCalculated;							// realtime update flag

	// reserved members
	GGVECTOR3						vecReservedFP1;								// reserved - maintain plugin compat.
	GGVECTOR3						vecReservedFP2;								// reserved - maintain plugin compat.
	GGVECTOR3						vecReservedFP3;								// reserved - maintain plugin compat.
};

struct sFrame : public sFrameID,
					   sFrameLinks,
					   sFrameTransforms,
					   sFramePosition
{
	// base meshes
	sMesh*							pMesh;										// basic mesh data for frame (optional)	
	sMesh*							pNotUsed1;									// not used any more
	sMesh*							pNotUsed2;									// not used - created to view bound box (optional)	
	sMesh*							pNotUsed3;								// not used - created to view bound sphere (optional)
	sMesh*							pLOD [ 2 ];									// created to hold LOD[0] and LOD[1] model
	bool							bExcluded;									// 301007 - new exclude limb feature

	// reserved bools
	bool							bReservedFa;								// 290808 - added these 3 BOOLS in (as Bool=1 & DWORD=4)
	bool							bReservedFb;								// 
	bool							bReservedFc;								// 

	// quad handing
	DWORD							dwStatusBits;								// 211008 - u71 - stores 32 bits of info ( %1-shift object bounds by frame zero )
	sMesh*							pLODForQUAD;								// 061208 - U71 - quad at end of LOD chain

	// reserved members
	GGMATRIX*						pmatBoneLocalTransform;						// reserved - maintain plugin compat.
	DWORD							dwReservedF2;								// reserved - maintain plugin compat.
	DWORD							dwReservedF3;								// reserved - maintain plugin compat.
	DWORD							dwReservedF4;								// reserved - maintain plugin compat.
	DWORD							dwReservedF5;								// reserved - maintain plugin compat.

	// constructor and destructor
	sFrame  ( );
	~sFrame ( );
};

struct sXFileRotateKey
{
	// x file rotate key
	DWORD							dwTime;										// time value
	DWORD							dwFloats;									// floats
	float							w;											// w rotate
	float							x;											// x rotate
	float							y;											// y rotate
	float							z;											// z rotate
};

struct sXFileScaleKey
{
	// x file scale key
	DWORD							dwTime;										// time value
	DWORD							dwFloats;									// floats
	GGVECTOR3						vecScale;									// scale value
};

struct sXFilePositionKey
{
	// x file position key
	DWORD							dwTime;										// time value
	DWORD							dwFloats;									// floats
	GGVECTOR3						vecPos;										// position
};

struct sXFileMatrixKey
{
	// x file matrix key
	DWORD							dwTime;										// time
	DWORD							dwFloats;									// floats
	GGMATRIX 						matMatrix;									// matrix to be applied
};

struct sRotateKey
{
	// rotate key
	DWORD							dwTime;										// time value
	GGQUATERNION					Quaternion;									// quaternion
};

struct sPositionKey
{
	// position key
	DWORD     						dwTime;										// time value
	GGVECTOR3						vecPos;										// position
	GGVECTOR3						vecPosInterpolation;						// interpolation
};

struct sScaleKey
{
	// scale key
	DWORD							dwTime;										// time value
	GGVECTOR3						vecScale;									// scale
	GGVECTOR3						vecScaleInterpolation;						// interpolation
};

struct sMatrixKey
{
	// matrix key
	DWORD							dwTime;										// time
	GGMATRIX 						matMatrix;									// matrix
	GGMATRIX 						matInterpolation;							// interpolation
};

struct sAnimationID
{
	// animation ID
	char							szName [ MAX_STRING ];						// name
	sFrame*							pFrame;										// pointer to frame
};

struct sAnimationProperties
{
	// animation properties
	BOOL							bLoop;										// is animation looping
	BOOL							bLinear;									// is animation linear
};

struct sAnimationKeys
{
	// animation keys
	DWORD							dwNumPositionKeys;							// number of position keys
	DWORD							dwNumRotateKeys;							// number of rotation keys
	DWORD							dwNumScaleKeys;								// number of scale keys
	DWORD							dwNumMatrixKeys;							// number of matrix keys
	sPositionKey*					pPositionKeys;								// position keys
	sRotateKey*						pRotateKeys;								// rotate keys
	sScaleKey*						pScaleKeys;									// scale keys
	sMatrixKey*						pMatrixKeys;								// and finally matrix keys

	DWORD							dwLastPositionKey;							// keep track of last actual key used for each type
	DWORD							dwLastRotateKey;
	DWORD							dwLastScaleKey;
	DWORD							dwLastMatrixKey;
};

struct sAnimationExtraBones
{
	// extra bone data for certain types of animations, will be removed in future
	// and replaced
	DWORD							bBoneType;									// type of bone
	int*							piBoneOffsetList;							// offset list
	int								iBoneOffsetListCount;						// count of offset list
	GGMATRIX**						ppBoneFrames;								// matrix frame
	int								iBoneFrameA;								// index a
	int								iBoneFrameB;								// index b
};

struct sAnimation : public sAnimationID,
						   sAnimationProperties,
						   sAnimationKeys,
						   sAnimationExtraBones
{
	// final animation structure
	sAnimation*						pSharedReadAnim;							// if erase any anim data (for clone-shared use), this points to clone anim data
	sAnimation*						pNext;										// pointer to next animation block

	// reserved members
	DWORD							dwReservedA1;								// reserved - maintain plugin compat.
	DWORD							dwReservedA2;								// reserved - maintain plugin compat.
	DWORD							dwReservedA3;								// reserved - maintain plugin compat.

	// constructor and destructor
	sAnimation  ( );
	~sAnimation ( );
};

struct sAnimationSet
{
	// stores a list of animations
	char							szName [ MAX_STRING ];						// animation name
	sAnimation*						pAnimation;									// pointer to data
	DWORD							ulLength;									// length of animation
	sAnimationSet*					pNext;										// next set

	// extra data to store dynamic boundboxes from animation
	GGVECTOR3*						pvecBoundMin;								// pre-calculated bound boxes per anim frame
	GGVECTOR3*						pvecBoundMax;
	GGVECTOR3*						pvecBoundCenter;
	float*							pfBoundRadius;

	// reserved members
	DWORD							dwReservedAS1;								// reserved - maintain plugin compat.
	DWORD							dwReservedAS2;								// reserved - maintain plugin compat.
	DWORD							dwReservedAS3;								// reserved - maintain plugin compat.

	// constructor and destructor
	sAnimationSet  ( );
	~sAnimationSet ( );
};

struct sPositionMatrices
{
	// matrices for position
	GGMATRIX 						matTranslation,								// translation ( position )
									matRotation,								// final rotation matrix
									matRotateX,									// x rotation
									matRotateY,									// y rotation
									matRotateZ,									// z rotation
									matScale,									// scale
									matObjectNoTran,							// final world without translation (collision)
									matWorld,									// final world matrix
									matFreeFlightRotate,						// free flight matrix rotation
									matPivot;									// pivot

	// reserved
	GGMATRIX 						matReservedPM1;								// reserved - maintain plugin compat.
	GGMATRIX 						matReservedPM2;								// reserved - maintain plugin compat.
	GGMATRIX 						matReservedPM3;								// reserved - maintain plugin compat.
};

struct sPositionProperties
{
	// position properties
	bool							bFreeFlightRotation;						// flag for euler/freeflight
	bool							bApplyPivot;								// pivot
	bool							bGlued;										// glue
	bool							bCustomWorldMatrix;							// used for when world matrix is calculated manually
	int		 						iGluedToObj;								// glued to object
	int		 						iGluedToMesh;								// glued to mesh (mode 1 when negative)
	DWORD							dwRotationOrder;							// euler rotation order
	float							fCamDistance;								// used for depth sorting
	bool							bCustomBoneMatrix;							// used by darkphysics (ragdoll)
	bool							bParentOfInstance;							// used by instance command to flag parent (used to animate parent even if not visible)
	float							fStoreLastCamDistance;						// used to preserve cam distance during alpha depth resort

	// reserved
	DWORD							dwReservedPP2;								// reserved - maintain plugin compat.
	DWORD							dwReservedPP3;								// reserved - maintain plugin compat.
};

struct sPositionVectors
{
	// stores position data
	GGVECTOR3						vecPosition,								// main position
									vecRotate,									// euler rotation
									vecScale,									// main scale
									vecLook,									// look vector
									vecUp,										// up vector
									vecRight,									// right vector
									vecLastPosition,							// last position used by auto-collision
									vecLastRotate;								// autocol uses for rotation restoration
	GGMATRIX 						matLastFreeFlightRotate;					// lee - 240306 - u6b4 - for automatic collision rotation freeze
};

struct sPositionData : public sPositionVectors,
                              sPositionProperties,
							  sPositionMatrices
{
	// constructor and destructor
	sPositionData  ( );
	~sPositionData ( );
};

struct sObjectProperties
{
	// object properties
	bool							bVisible;									// for hide/show
	bool							bUniverseVisible;							// internal systems can cull visibility (like universe)
	bool							bNoMeshesInObject;							// sometimes all meshes can be removed!
	bool							bUpdateOverallBounds;						// flag when need to update overall object bounds
	bool							bUpdateOnlyCurrentFrameBounds;				// true by default, when true it ONLY does bound for current animation frame (set object frame o,f,X)
	bool							bOverlayObject;								// flagged if special overlay object (ie lock,ghost)
	bool							bGhostedObject;								// flagged if a ghosted object
	bool							bTransparentObject;							// flagged if a transparent object
	bool							bTransparencyWaterLine;						// new: transparency water line (for above/below water)
	bool							bNewZLayerObject;							// flagged if requires zbuffer clear
	bool							bLockedObject;								// flagged if requires a locked camera
	bool							bStencilObject;								// flagged if object uses the stencil layer
	bool							bReflectiveObject;							// flagged if object renders a reflection
	bool							bReflectiveClipping;						// flagged if user clipping detected and available
	bool							bHadLODNeedCamDistance;						// flagged if use ADD LOD TO OBJECT
	bool							bNotUsed1;									// 140817 - added for shadow blend state DX11
	bool							bStatic;									// flagged if a static object ( won't go into the draw list )
	bool							bUsesItsOwnBuffers;							// flagged to get best speed with some drivers
	bool							bReplaceObjectFromBuffers;					// flagged when object must be removed from buffers
	bool							bCastsAShadow;								// flagged if will cast a shadow
	LPVOID							pNotUsed;									// not used any more - holds the pointer to the shadow mesh
	bool							bHideShadow;								// flagged if temp. hide shadow
	bool							bExcluded;									// flagged if object is excluded
	bool							bDisableTransform;							// disable transforms - for external control
	float							fFOV;										// per-object FOV control
	int								iInsideUniverseArea;						// -1=no, otherwise areabox index (work)
	float							fLODDistance [ 2 ];							// store distance of LOD transitions
	int								iUsingWhichLOD;								// using 0-normal,1-lod[0] and 2-lod[1]

	// reserved members
	bool							bVeryEarlyObject;							// U71 - object can be rendered even before stencilstart (and matrix, terrain, world, etc), ideal for skyboxes
	float*							pfAnimLimbFrame;							// U75 - mem ptr for limb based animation frames
	DWORD							dwObjectNumber;								// was dwCameraMaskBits, 301007 - now objid
	DWORD							dwCameraMaskBits;							// reserved - maintain plugin compat.
	union
	{
		int							iUsingOldLOD;								// U71 - alpha fade
		int							iLOD0LimbIndex;
	};
	union
	{
		float						fLODTransition;								// U71 - alpha fade
		int							iLOD1LimbIndex;
	};
	union
	{
		float						fLODDistanceQUAD;							// store distance of final LOD transitions
		int							iLOD2LimbIndex;
	};

	// reserved
	DWORD							dwApplyOriginalScaling;						// 1=use matOriginal to pre-multiply to carry transform of original model (FBX requirement)
	DWORD							dwCountdownToUniverseVisOff;
	float							fArtificialDistanceOffset;					// can use used to affect transpareny draw order
	DWORD							dwRememberTransparencyState;
	DWORD							dwReservedPR5;
	DWORD							dwReservedPR6;
	DWORD							dwReservedPR7;
	DWORD							dwReservedPR8;
};

struct sObjectAnimationProperties
{
	// animation properties of an object
	bool							bAnimPlaying;								// is anim playing
	bool							bAnimLooping;								// is looping
	bool							bAnimUpdateOnce;							// used when change limb (can affect boned models)
	float							fAnimFrame;									// frame we're on
	float							fAnimLastFrame;								// last frame
	float							fAnimSpeed;									// speed of animation
	float							fAnimLoopStart;								// start loop point
	float							fAnimFrameEnd;								// end frame
	float							fAnimTotalFrames;							// total number of frame
	bool							bAnimManualSlerp;							// slerp animation
	float							fAnimSlerpStartFrame;						// start frame
	float							fAnimSlerpEndFrame;							// end frame
	float							fAnimSlerpLastTime;							// used so we do not repeat anim calculations on object (quickreject)
	float							fAnimSlerpTime;								// time
	float							fAnimInterp;								// interpolation

	// reserved
	bool							bIgnoreDefAnim;								// 131115 - skip a matCombined transform in collision intersectall detection
	bool							bUseSpineCenterSystem;
	bool							bSpineTrackerMoving;
	bool							bReservedAN1d;
	float							fSpineCenterTravelDeltaX;
	float							fSpineCenterTravelDeltaZ;
};

struct sObjectInstance
{
	// object instance
	sObject*						pInstanceOfObject;							// used to mark object as a mere instance of object at ptr
	bool*							pInstanceMeshVisible;						// used to store limb visibility data
	bool							bInstanceAlphaOverride;						// used to apply alpha factor to instance obj
	DWORD							dwInstanceAlphaOverride;					// alpha override
};

struct sObjectData
{
	// object data
	int								iMeshCount;									// number of meshes in model
	int								iFrameCount;								// number of frames in the model
	sMesh**							ppMeshList;									// mesh list
	sFrame**						ppFrameList;								// frame list
	sFrame*							pFrame;										// main frame ( contained in a hiearachy )
	sAnimationSet*					pAnimationSet;								// list of all animations
	sCollisionData					collision;									// global collision data ( each mesh within a frame also has collision data - but local to itself )
	sPositionData					position;									// global positioning data

	bool							bIsStatic;
	bool							bIgnored;
	bool							bIsCharacter;
	bool							bReservedR2;
	//
	DWORD							dwSpineCenterLimbIndex;						// see above for bUseSpineCenterSystem + floats
	DWORD							dwReservedR4;
};

struct sObjectDelete
{
	struct sDelete
	{
		ON_OBJECT_DELETE_CALLBACK	onDelete;
		int							userData;

		sDelete ( )
		{
			onDelete = NULL;
			userData = -1;
		}
	};

	sDelete*						pDelete;
	int								iDeleteCount;
	int								iDeleteID;

	// 310305 - mike - destructor needed
	~sObjectDelete ( );
};

struct sObjectCustom
{
	// 280305 - used when objects want to store custom data for example when
	//        - they save and want to save out this data

	DWORD							dwCustomSize;
	void*							pCustomData;

	sObjectCustom ( );

	// 310305 - destructor
	~sObjectCustom ( );
};

struct sObjectCharacterCreator
{
	// 070515 - Dave added to store character creator tones
	float ColorTone[4][3];
	float ToneMix[4];
};

struct sObject : public sObjectData,
					    sObjectProperties,
					    sObjectAnimationProperties,
						sObjectInstance,
						sObjectDelete,
						sObjectCustom
{
	// constructor and destructor
	sObject  ( );
	~sObject ( );

    // Ideally, these should go into sObjectInstance, but
    // doing so would break internal and third-party plug-ins

	// Set this to point to the object that this object is dependent upon.
	// (ie, Instance, and clone with shared animation data)
    sObject*                        pObjectDependency;

	// Increment this when another object depends on this object.
    DWORD                           dwDependencyCount;

	// Dave - added for character creator
	sObjectCharacterCreator*		pCharacterCreator;

};

#define NUM_BOX_VERTS 8

struct vertstype
{
	GGVECTOR3 pos;
};
struct sImposter
{
	GGVECTOR3 boundingBoxVerts[NUM_BOX_VERTS];
	float radius;
	GGVECTOR2 uvOffset;
	vertstype verts[6];
	GGVECTOR3 centre;
	GGVECTOR3 cameraDir;
	bool pAdjustUV;
};

#endif _DBODATA_H_

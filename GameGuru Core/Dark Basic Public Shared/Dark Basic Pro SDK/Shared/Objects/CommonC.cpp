// Common/Private Functions Implementation
#include "CommonC.h"
#include "CObjectsC.h"
#include "CGfxC.h"
#include ".\..\Core\SteamCheckForWorkshop.h"

// Object Manager and Renderer
DBPRO_GLOBAL CObjectManager					m_ObjectManager;
DBPRO_GLOBAL int*							g_ObjectListRef					= NULL;				// short list of valid object indexes
DBPRO_GLOBAL int							g_iObjectListRefCount			= 0;
DBPRO_GLOBAL sObject**						g_ObjectList					= NULL;				// a list of pointers to pointers - we're going to create a dynamic array
DBPRO_GLOBAL int							g_iObjectListCount				= 0;				// number of objects currently in list
DBPRO_GLOBAL bool							g_bGlobalVBIBUsageFlag			= true;				// true means objects created use their own VB buffers
DBPRO_GLOBAL bool							g_bGlobalSortByTextureUsageFlag	= true;				// true means will sort by texture (preU71 behaviour)
DBPRO_GLOBAL bool							g_bGlobalSortByObjectNumberFlag = false;			// true means will sort by objectnumber if not sorted by texture
DBPRO_GLOBAL bool							g_bGlobalSortByObjectDepthFlag  = false;			// true means sort by depth DURING RESORT, NOT PER CYCLE (as done for transparent objects automatically)!!
DBPRO_GLOBAL eSortOrder                     g_eGlobalSortOrder              = E_SORT_BY_TEXTURE; // E_SORT_BY_TEXTURE or E_SORT_BY_NONE

// Mesh Manager Structures
DBPRO_GLOBAL sMesh**						g_RawMeshList					= NULL;				// a list of pointers to pointers
DBPRO_GLOBAL int							g_iRawMeshListCount				= 0;				// number of meshes currently in list
DBPRO_GLOBAL PTR_FuncCreateStr				g_pCreateDeleteStringFunction	= NULL;

// Globstruct
extern DBPRO_GLOBAL GlobStruct*				g_pGlob;
extern DBPRO_GLOBAL char					m_pWorkString[_MAX_PATH];
extern DBPRO_GLOBAL bool					g_bCreateChecklistNow;
extern DBPRO_GLOBAL DWORD					g_dwMaxStringSizeInEnum;

// Vertex and Pixel Shader Globals
DBPRO_GLOBAL sVertexShader					m_VertexShaders [ MAX_VERTEX_SHADERS ];
DBPRO_GLOBAL sPixelShader					m_PixelShaders  [ MAX_VERTEX_SHADERS ];
DBPRO_GLOBAL sEffectItem*					m_EffectList    [ MAX_EFFECTS ];

// Standard datatype to hold full collision details ( collision, etc)
DBPRO_GLOBAL sFullCollisionResult			g_DBPROCollisionResult;
DBPRO_GLOBAL sFullPickResult				g_DBPROPickResult;

// Shadow System globals
DBPRO_GLOBAL GGVECTOR3					g_vecShadowPosition;
DBPRO_GLOBAL int							g_iShadowPositionFixed = 0;

DBPRO_GLOBAL HINSTANCE						g_XObject    = NULL;
DBPRO_GLOBAL HINSTANCE						g_3DSObject  = NULL;
DBPRO_GLOBAL HINSTANCE						g_MDLObject  = NULL;
DBPRO_GLOBAL HINSTANCE						g_MD2Object  = NULL;
DBPRO_GLOBAL HINSTANCE						g_MD3Object  = NULL;
DBPRO_GLOBAL HINSTANCE						g_PrimObject = NULL;
static HINSTANCE							g_Image      = NULL;
DBPRO_GLOBAL HINSTANCE						g_Setup      = NULL;
DBPRO_GLOBAL HINSTANCE						g_GFX;
DBPRO_GLOBAL HINSTANCE						m_hData = NULL;
DBPRO_GLOBAL GGCAPS						m_Caps;

//extern DBPRO_GLOBAL RetVoidFunctionPointerPFN		UpdateTimer;
extern DBPRO_GLOBAL LPGGDEVICE				m_pD3D;

DARKSDK_DLL void Basic3DConstructorD3D ( HINSTANCE hSetup, HINSTANCE hImage, HINSTANCE hTypes, HINSTANCE hCamera )
{
	// Get D3D and Caps
	#ifdef DX11
	#else
	m_pD3D->GetDeviceCaps ( &m_Caps );
	#endif

	// Clear Full Collision Data
	memset ( &g_DBPROCollisionResult, 0, sizeof(g_DBPROCollisionResult) );

	// Delete any data created from an effect error
	SAFE_DELETE(g_pEffectErrorMsg);
}

DARKSDK_DLL void Basic3DConstructor ( void )
{
	// setup the object library, we load up any DLLs
	Basic3DConstructorD3D ( 0, 0, 0, 0 );
}

DARKSDK_DLL void Basic3DDestructorD3D ( void )
{
	// free manager resources
	m_ObjectManager.Free();

	// delete each object
	for ( int iObj = 0; iObj < g_iObjectListCount; iObj++ )
		SAFE_DELETE ( g_ObjectList [ iObj ] );

	// free object array
	SAFE_DELETE_ARRAY ( g_ObjectListRef );
	SAFE_DELETE_ARRAY ( g_ObjectList );

	// delete each raw-mesh
	if ( g_pGlob )
	{
		if ( g_pGlob->bInvalidFlag == false )
		{
			if ( g_RawMeshList )
				for ( int iMesh = 0; iMesh < g_iRawMeshListCount; iMesh++ )
					SAFE_DELETE ( g_RawMeshList [ iMesh ] );

			// free raw-mesh array
			SAFE_DELETE_ARRAY ( g_RawMeshList );
		}
	}
}

DARKSDK_DLL void Basic3DDestructor ( void )
{
	Basic3DDestructorD3D();
}

DARKSDK_DLL void Basic3DPassCoreData ( LPVOID pGlobPtr )
{
	// allocate the list of pointers
	g_iObjectListRefCount=0;
	g_ObjectListRef = new int [ g_iObjectListCount ];
	g_ObjectList = new sObject* [ g_iObjectListCount ];

	if ( !g_ObjectList )
		RunTimeError ( RUNTIMEERROR_OBJECTMANAGERFAILED );

	// set all pointers to null
	for ( int iTemp = 0; iTemp < g_iObjectListCount; iTemp++ )
	{
		g_ObjectListRef [ iTemp ] = 0;
		g_ObjectList [ iTemp ] = NULL;
	}

	if ( !m_ObjectManager.Setup ( ) )
		RunTimeError ( RUNTIMEERROR_OBJECTMANAGERFAILED );

	if ( !m_ObjectManager.UpdateObjectListSize ( g_iObjectListCount ) )
		RunTimeError ( RUNTIMEERROR_OBJECTMANAGERFAILED );
}

DARKSDK_DLL void MakeCube ( int iID, float fSize );

DARKSDK_DLL void Basic3DRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		Basic3DDestructorD3D();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		Basic3DConstructorD3D ( g_pGlob->g_GFX, g_pGlob->g_Image, g_pGlob->g_Vectors, g_pGlob->g_Camera3D );
		Basic3DPassCoreData ( g_pGlob );
	}
}

DARKSDK_DLL void UpdateViewProjForMotionBlur ( void )
{
	m_ObjectManager.UpdateViewProjForMotionBlur ( );
}

DARKSDK_DLL void UpdateAnimationCycle ( void )
{
	// U75 - 080410 - moved here (once per SYNC, not FASTSYNC calls! as it speeds up anim)
	m_ObjectManager.UpdateAnimationCycle ( );
}

DARKSDK_DLL void UpdateOnce ( void )
{
	m_ObjectManager.UpdateInitOnce ( );
}

DARKSDK_DLL void Update ( void )
{
	m_ObjectManager.Update ( );
}

DARKSDK_DLL void UpdateGhostLayer ( void )
{
	m_ObjectManager.UpdateGhostLayer ( );
}

DARKSDK_DLL void UpdateNoZDepth ( void )
{
	m_ObjectManager.UpdateNoZLayer ( );
}

DARKSDK_DLL void AutomaticStart ( void )
{
	DoAutomaticStart();
}

DARKSDK_DLL void AutomaticEnd ( void )
{
	DoAutomaticEnd();
}

//
// Internal Collision-BSP Functions
//

DARKSDK_DLL void ColPosition ( int iID, float fX, float fY, float fZ )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return;

	// set the new position
	sObject* pObject = g_ObjectList [ iID ];
	pObject->collision.bHasBeenMovedForResponse=true;

	float fOffsetX = GetColCenterX(pObject);
	float fOffsetY = GetColCenterY(pObject);
	float fOffsetZ = GetColCenterZ(pObject);
	pObject->position.vecPosition.x = fX - fOffsetX;
	pObject->position.vecPosition.y = fY - fOffsetY;
	pObject->position.vecPosition.z = fZ - fOffsetZ;
}

DARKSDK_DLL float GetXColPosition ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0.0f;

	sObject* pObject = g_ObjectList [ iID ];
	float fPos = pObject->position.vecPosition.x;
	float fOffsetX = GetColCenterX(pObject);
	return fPos + fOffsetX;
}

DARKSDK_DLL float GetYColPosition ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0.0f;

	sObject* pObject = g_ObjectList [ iID ];
	float fPos = pObject->position.vecPosition.y;
	float fOffsetY = GetColCenterY(pObject);
	return fPos + fOffsetY;
}

DARKSDK_DLL float GetZColPosition ( int iID )
{
	// check the object exists
	if ( !ConfirmObject ( iID ) )
		return 0.0f;

	sObject* pObject = g_ObjectList [ iID ];
	float fPos = pObject->position.vecPosition.z;
	float fOffsetZ = GetColCenterZ(pObject);
	return fPos + fOffsetZ;
}

DARKSDK_DLL LPSTR GetReturnStringFromWorkString(void)
{
	LPSTR pReturnString=NULL;
	if(m_pWorkString)
	{
		DWORD dwSize=strlen(m_pWorkString);
		g_pCreateDeleteStringFunction((DWORD*)&pReturnString, dwSize+1);
		strcpy(pReturnString, m_pWorkString);
	}
	return pReturnString;
}

#ifdef DARKSDK_COMPILE
	static bool DoesFileExist ( LPSTR pFilename )
#else
	DARKSDK_DLL bool DoesFileExist ( LPSTR pFilename )
#endif
{
	// success or failure
	bool bSuccess = true;

	// leeadd - 180606 - u62 - uses actual or virtual file
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, pFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	// open File To See If Exist
	HANDLE hfile = CreateFile(VirtualFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hfile==INVALID_HANDLE_VALUE)
		bSuccess=false;
	else
		CloseHandle(hfile);

	// return result
	return bSuccess;
}

bool g_bInstanceReturnFlag = false;
void dbSetInstanceReturnFlag ( bool bStatus )
{
	g_bInstanceReturnFlag = bStatus;
}

DARKSDK_DLL bool ConfirmObjectInstanceEx ( int iID, bool bInstanceReturnFlag )
{
	// mike - 061206 - need this for dtsp2 but it is going to affect other things
	if ( g_bInstanceReturnFlag == true )
		bInstanceReturnFlag = true;

	if ( iID < 1 || iID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DMODELNUMBERILLEGAL );
		return false;
	}

	if ( iID < g_iObjectListCount )
	{ 
		if ( g_ObjectList [ iID ] )
		{
			// users want to use excluded objects - mike, what functions screw up?
			if ( g_ObjectList [ iID ]->pFrame )
			{
				return true;
			}
			if ( g_ObjectList [ iID ]->pInstanceOfObject )
			{
				// lee - 310306 - u6rc4 - silent return as instance objects can only do certain things (new function added for confirmobjectInstanceAllowed
				return bInstanceReturnFlag;
			}
			
		}
	}

	// report which object does not exist (very seful)
	char pWhichObjectNumber[512];
	wsprintf ( pWhichObjectNumber, "Object Number Not Found: %d", iID );
	RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS, pWhichObjectNumber );

	// cone
	return false;	
}

DARKSDK_DLL bool ConfirmObjectInstance ( int iID )
{
	// 190719 - when called from LUA, can silently fail!
	if ( ObjectExist(iID) == 0 )
		return false;

	// see above
	return ConfirmObjectInstanceEx ( iID, true );
}

DARKSDK_DLL bool ConfirmObject ( int iID )
{
	// see above (bInstanceReturnFlag=false)
	return ConfirmObjectInstanceEx ( iID, false );
}

DARKSDK_DLL bool CheckObjectExist ( int iID )
{
	if ( iID < 1 || iID > MAXIMUMVALUE ) return false;
	if ( iID < g_iObjectListCount )
	{ 
		if ( g_ObjectList [ iID ] )
		{
			if ( g_ObjectList [ iID ]->pFrame ) return true;
			if ( g_ObjectList [ iID ]->pInstanceOfObject ) return true;
		}
	}
	return false;	
}

DARKSDK_DLL bool ConfirmNewObject( int iID )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DMODELNUMBERILLEGAL );
		return false;
	}

	if ( iID < g_iObjectListCount )
	{ 
		if ( g_ObjectList [ iID ] )
		{
			if ( g_ObjectList [ iID ]->pFrame )
			{
				RunTimeError ( RUNTIMEERROR_B3DMODELALREADYEXISTS );
				return false;
			}
		}
	}

	return true;
}

bool ConfirmObjectAndLimbInstanceEx ( int iID, int iLimbID, bool bInstanceReturnFlag )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DMODELNUMBERILLEGAL );
		return false;
	}

	if ( iID < g_iObjectListCount )
	{ 
		if ( g_ObjectList [ iID ] )
		{
			if ( g_ObjectList [ iID ]->pInstanceOfObject && bInstanceReturnFlag==false )
			{
				// lee - 310306 - u6rc4 - silent return if instance not allowed
				return false;
			}
			// leefix - 290506 - u62 - fpsccode - instanced object limbs can be SHOW/HIDE
			if ( g_ObjectList [ iID ]->pInstanceMeshVisible )
			{
				// limb part of vis-array - continue
			}
			else
			{
				if ( !g_ObjectList [ iID ]->pFrame )
				{
					RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS );
					return false;
				}
			}
		}
		else
		{
			RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS );
			return false;
		}
	}
	else
	{
		RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS );
		return false;
	}

	if ( iLimbID < 0 || iLimbID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_LIMBNUMBERILLEGAL );
		return false;
	}

	if ( g_ObjectList [ iID ]->pInstanceMeshVisible )
	{
		// limb part of vis-array
	}
	else
	{
		if ( iLimbID >= g_ObjectList [ iID ]->iFrameCount )
		{
			RunTimeError ( RUNTIMEERROR_LIMBNOTEXIST );
			return false;
		}

		if ( !g_ObjectList [ iID ]->ppFrameList [ iLimbID ] )
		{
			RunTimeError ( RUNTIMEERROR_LIMBNOTEXIST );
			return false;
		}
	}

	return true;
}

DARKSDK_DLL bool ConfirmObjectAndLimbInstance ( int iID, int iLimbID )
{
	// see above
	return ConfirmObjectAndLimbInstanceEx ( iID, iLimbID, true );
}

DARKSDK_DLL bool ConfirmObjectAndLimb( int iID, int iLimbID )
{
	// see above
	return ConfirmObjectAndLimbInstanceEx ( iID, iLimbID, false );
}

DARKSDK_DLL bool ConfirmMesh ( int iMeshID )
{
	if ( iMeshID < 1 || iMeshID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DMESHNUMBERILLEGAL );
		return false;
	}

	if ( iMeshID < g_iRawMeshListCount )
	{ 
		if ( g_RawMeshList [ iMeshID ] )
		{
			return true;
		}
	}

	RunTimeError ( RUNTIMEERROR_B3DMESHNOTEXIST );
	return false;	
}

DARKSDK_DLL bool ConfirmNewMesh ( int iMeshID )
{
	if ( iMeshID < 1 || iMeshID > MAXIMUMVALUE )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DMESHNUMBERILLEGAL );
		return false;
	}

	if ( iMeshID < g_iRawMeshListCount )
	{ 
		if ( g_RawMeshList [ iMeshID ] )
		{
			// delete old mesh
			DeleteRawMesh ( g_RawMeshList [ iMeshID ] );
			g_RawMeshList [ iMeshID ] = NULL;
		}
	}

	return true;
}

DARKSDK_DLL bool ConfirmEffect ( int iEffectID )
{
	if ( iEffectID < 1 || iEffectID > MAX_EFFECTS )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DEFFECTNUMBERILLEGAL );
		return false;
	}
	if ( m_EffectList [ iEffectID ]==NULL )
	{
		RunTimeError ( RUNTIMEERROR_B3DEFFECTNOTEXISTS );
		return false;
	}

	// success
	return true;
}

DARKSDK_DLL bool ConfirmNewEffect ( int iEffectID )
{
	if ( iEffectID < 1 || iEffectID > MAX_EFFECTS )
	{ 
		RunTimeError ( RUNTIMEERROR_B3DEFFECTNUMBERILLEGAL );
		return false;
	}
	if ( m_EffectList [ iEffectID ] )
	{
		RunTimeError ( RUNTIMEERROR_B3DEFFECTALREADYEXISTS );
		return false;
	}

	// success
	return true;
}

DARKSDK_DLL void ResizeObjectList ( int iSize )
{
	// this function will get called if a user goes over the current limit of objects
	// when this happens we will need to resize the list so it can store the new data
	int*		pNewObjectListRef;
	sObject**	pNewObjectList;

	// size
	int iOriginalCount = g_iObjectListCount;
	g_iObjectListCount += 50;
	if ( g_iObjectListCount <= iSize )
		g_iObjectListCount = iSize + 5;

	// create
	pNewObjectListRef = new int [ g_iObjectListCount ];
	pNewObjectList = new sObject* [ g_iObjectListCount ];

	// lee - 200306 - u6b4 - if not created (super large size), do not crash!
	if ( pNewObjectListRef && pNewObjectList )
	{
		for ( int iTemp = 0; iTemp < g_iObjectListCount; iTemp++ )
		{
			pNewObjectListRef [ iTemp ] = 0;
			pNewObjectList [ iTemp ] = NULL;
		}
		for ( int iTemp = 0; iTemp < iOriginalCount; iTemp++ )
		{
			pNewObjectListRef [ iTemp ] = g_ObjectListRef [ iTemp ];
			pNewObjectList [ iTemp ] = g_ObjectList [ iTemp ];
		}
		SAFE_DELETE_ARRAY ( g_ObjectListRef );
		SAFE_DELETE_ARRAY ( g_ObjectList );
		g_ObjectListRef = pNewObjectListRef;
		g_ObjectList = pNewObjectList;
	}
}

DARKSDK_DLL void AddObjectToObjectListRef ( int iID )
{
	// check if already in list
	for ( int iIndex = 0; iIndex < g_iObjectListRefCount; iIndex++ )
		if ( g_ObjectListRef [ iIndex ]==iID )
			return;

	// add new entry
	g_ObjectListRef [ g_iObjectListRefCount ] = iID;
	g_iObjectListRefCount++;

	// update global arrays for shortlist entry expansion
	m_ObjectManager.UpdateObjectListSize ( g_iObjectListRefCount );
}

DARKSDK_DLL void  RemoveObjectFromObjectListRef ( int iID )
{
	// check if in list
	for ( int iIndex = 0; iIndex < g_iObjectListRefCount; iIndex++ )
	{
		if ( g_ObjectListRef [ iIndex ]==iID )
		{
			// shuffle to remove it
			DWORD dwSize = (g_iObjectListRefCount-iIndex)-1;
			if ( dwSize > 0 ) memcpy ( &g_ObjectListRef[iIndex], &g_ObjectListRef[iIndex+1], dwSize*sizeof(int) );
			g_iObjectListRefCount--;
			return;
		}
	}
}

DARKSDK_DLL void ResizeRawMeshList ( int iIndexNeeded )
{
	// temp count store and ptr
	int iOriginalCount = g_iRawMeshListCount;
	sMesh**	pNewRawMeshList = NULL;
	
	// increment only if needed
	if ( g_iRawMeshListCount <= iIndexNeeded )
		g_iRawMeshListCount = iIndexNeeded + 5;

	// create new list
	pNewRawMeshList = new sMesh* [ g_iRawMeshListCount ];
	
	// clear new list
	for ( int iTemp = 0; iTemp < g_iRawMeshListCount; iTemp++ )
		pNewRawMeshList [ iTemp ] = NULL;

	// copy old list to new list
	for ( int iTemp = 0; iTemp < iOriginalCount; iTemp++ )
		pNewRawMeshList [ iTemp ] = g_RawMeshList [ iTemp ];

	// remove old list from memory
	SAFE_DELETE_ARRAY ( g_RawMeshList );

	// update actual list ptr
	g_RawMeshList = pNewRawMeshList;
}

static float wrapangleoffset(float da)
{
	int breakout=100;
	while(da<0.0f || da>=360.0f)
	{
		if(da<0.0f) da=da+360.0f;
		if(da>=360.0f) da=da-360.0f;
		breakout--;
		if(breakout==0) break;
	}
	if(breakout==0) da=0.0f;
	return da;
}

DARKSDK_DLL void GetAngleFromPoint(float x1, float y1, float z1, float x2, float y2, float z2, float* ax, float* ay, float* az)
{
	GGVECTOR3 Vector;
	Vector.x = x2-x1;
	Vector.y = y2-y1;
	Vector.z = z2-z1;

	// Find Y and then X axis rotation
	double yangle=atan2(Vector.x, Vector.z);
	if(yangle<0.0) yangle+=GGToRadian(360.0);
	if(yangle>=GGToRadian(360.0)) yangle-=GGToRadian(360.0);

	GGMATRIX yrotate;
	GGMatrixRotationY ( &yrotate, (float)-yangle );
	GGVec3TransformCoord ( &Vector, &Vector, &yrotate );

	double xangle=-atan2(Vector.y, Vector.z);
	if(xangle<0.0) xangle+=GGToRadian(360.0);
	if(xangle>=GGToRadian(360.0)) xangle-=GGToRadian(360.0);

	*ax = wrapangleoffset(GGToDegree((float)xangle));
	*ay = wrapangleoffset(GGToDegree((float)yangle));
	*az = 0.0f;
}

DARKSDK_DLL void RegenerateLookVectors ( sObject* pObject )
{
	// regenerate the look, up and right vectors
	pObject->position.vecLook  = GGVECTOR3 ( 0, 0, 1 );		// look vector
	pObject->position.vecUp    = GGVECTOR3 ( 0, 1, 0 );		// up vector
	pObject->position.vecRight = GGVECTOR3 ( 1, 0, 0 );		// right vector
	
	if ( pObject->position.bFreeFlightRotation )
	{
		// free flight modifies lookupright directly (uses current rotation matrix)
		GGVec3TransformCoord ( &pObject->position.vecLook,	&pObject->position.vecLook,		&pObject->position.matFreeFlightRotate );
		GGVec3TransformCoord ( &pObject->position.vecUp,		&pObject->position.vecUp,		&pObject->position.matFreeFlightRotate );
		GGVec3TransformCoord ( &pObject->position.vecRight,	&pObject->position.vecRight,	&pObject->position.matFreeFlightRotate );
	}
	else
	{
		// transform by euler rotation
		UpdateEulerRotation ( pObject );
		GGVec3TransformCoord ( &pObject->position.vecLook,	&pObject->position.vecLook,		&pObject->position.matRotation );
		GGVec3TransformCoord ( &pObject->position.vecUp,		&pObject->position.vecUp,		&pObject->position.matRotation );
		GGVec3TransformCoord ( &pObject->position.vecRight,	&pObject->position.vecRight,	&pObject->position.matRotation );
	}
}

DARKSDK_DLL void AnglesFromMatrix ( GGMATRIX* pmatMatrix, GGVECTOR3* pVecAngles )
{
	// Thanks to Andrew for finding this gem!
	// from http://www.martinb.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
	float m00 = pmatMatrix->_11;
	float m01 = pmatMatrix->_12;
	float m02 = pmatMatrix->_13;
	float m12 = pmatMatrix->_23;
	float m22 = pmatMatrix->_33;
	float heading = (float)atan2(m01,m00);
	float attitude = (float)atan2(m12,m22);
	float bank = (float)asin(-m02);

	// check for gimbal lock
	if ( fabs ( m02 ) > 1.0f )
	{
		// looking straight up or down
		float PI = GG_PI / 2.0f;
		pVecAngles->x = 0.0f;
		pVecAngles->y = GGToDegree ( PI * m02 );
		pVecAngles->z = 0.0f;
	}
	else
	{
		pVecAngles->x = GGToDegree ( attitude );
		pVecAngles->y = GGToDegree ( bank );
		pVecAngles->z = GGToDegree ( heading );
	}
}

DARKSDK_DLL void CheckRotationConversion ( sObject* pObject, bool bUseFreeFlightMode )
{
	// has there been a change?
	if ( bUseFreeFlightMode != pObject->position.bFreeFlightRotation )
	{
		// Caluclates equivilant rotation data if switch rotation-modes
		if( bUseFreeFlightMode==true )
		{
			// Euler to Freeflight
			UpdateEulerRotation ( pObject );
			pObject->position.matFreeFlightRotate = pObject->position.matRotation;
		}
	}

	// always calculate freeflight to euler (for angle feedback)
	if( bUseFreeFlightMode==true )
	{
		// Freeflight to Euler
		AnglesFromMatrix ( &pObject->position.matFreeFlightRotate, &pObject->position.vecRotate );
	}

	// new rotation mode
	pObject->position.bFreeFlightRotation = bUseFreeFlightMode;
}

DARKSDK_DLL void UpdateOverlayFlag ( sObject* pObject )
{
	// false unless an overlay flag is being used
	pObject->bOverlayObject = false;
	if ( pObject->bNewZLayerObject )	pObject->bOverlayObject = true;
	if ( pObject->bTransparentObject )	pObject->bOverlayObject = true;	
	if ( pObject->bLockedObject )		pObject->bOverlayObject = true;
	if ( pObject->bGhostedObject )		pObject->bOverlayObject = true;
}

DARKSDK_DLL void SetObjectTransparency ( sObject* pObject, int iTransparency )
{
	// promote to overlay layer (or not)
	if ( iTransparency==2 || iTransparency==3 || iTransparency==5 || iTransparency==6 )
		pObject->bTransparentObject = true;
	else
		pObject->bTransparentObject = false;

	// leeadd - 021205 - new feature - create a water line (for two layers of depth sorted objs above and below line)
	if ( iTransparency==5 )
		pObject->bTransparencyWaterLine = true;
	else
		pObject->bTransparencyWaterLine = false;

	// leeadd - 061208 - transparency mode 7 sets an object to very early draw phase
	if ( iTransparency==7 )
		pObject->bVeryEarlyObject = true;
	else
		pObject->bVeryEarlyObject = false;

	// leefix - 190303 - better overlay flag handling
	UpdateOverlayFlag ( pObject );
}

DARKSDK_DLL void NewObjectAutoCam( float fRadius )
{
	SetAutoCam ( 0.0f, 0.0f, 0.0f, fRadius );
}

DARKSDK_DLL void SetupModelFunctionPointers ( void* pModel, HINSTANCE hDLL )
{
	// MIKEMIKE : Propose we remove this!
}

DARKSDK_DLL void SetupPrimitiveFunctionPointers ( void* pData )
{
	// MIKEMIKE : Propose we remove this!
}

DARKSDK_DLL void SetupDefaultProperties ( void* pData )
{
	// MIKEMIKE : Propose we remove this!
}

DARKSDK_DLL void SetupDefaultPosition ( int iID )
{
	PositionObject ( iID, 0.0f, 0.0f, 0.0f );
	ScaleObject    ( iID, 100.0f, 100.0f, 100.0f );
	RotateObject   ( iID, 0.0f, 0.0f, 0.0f );
}

DARKSDK_DLL bool PrepareCustomObject ( void* m_pData )
{
	// MIKEMIKE : Propose we remove this!
	// Complete
	return true;
}

DARKSDK_DLL void GetCullDataFromModel ( int iID )
{
	// MIKEMIKE : Propose we remove this!
}

DARKSDK_DLL bool CreateModelFromCustom( int iID, void* m_pData )
{
	// MIKEMIKE : Propose we remove this!
	return true;
}

DARKSDK_DLL bool SetNewObjectFinalProperties ( int iID, float fRadius )
{
	// setup new object and introduce to buffers
	sObject* pObject = g_ObjectList [ iID ];
	if ( pObject == NULL ) return false;

	// lee - 250307 - store object number for reference
	pObject->dwObjectNumber = iID;

	// calculate collision data
	SetupObjectsGenericProperties ( pObject );
	
	// verify object has at least one frame and mesh
	if ( pObject->ppFrameList==NULL )
	{
		// free object if insufficient data
		RunTimeError ( RUNTIMEERROR_B3DOBJECTLOADFAILED );
		SAFE_DELETE ( g_ObjectList [ iID ] );
		return false;
	}

	// map bone and animation information to frames
	bool bUpdateBones = true;
	if ( g_ObjectList [ iID ]->pAnimationSet )
	{
		if ( g_ObjectList [ iID ]->pAnimationSet->pAnimation )
		{
			// specifically for MDL models
			if ( !g_ObjectList [ iID ]->pAnimationSet->pAnimation->bBoneType )
				bUpdateBones = false;
		}
	}
	if ( bUpdateBones )
	{
		if ( pObject->ppMeshList )
		{
			InitFramesToBones ( pObject->ppMeshList, pObject->iMeshCount );
			MapFramesToBones ( pObject->ppMeshList, pObject->pFrame, pObject->iMeshCount );
		}
	}

	// leefix - 010304 - no reason to have animset with no anims
	if ( g_ObjectList [ iID ]->pAnimationSet )
	{
		if ( g_ObjectList [ iID ]->pAnimationSet->pAnimation==NULL )
		{
			// delete anim set as no data to back it up
			SAFE_DELETE ( g_ObjectList [ iID ]->pAnimationSet );
		}
	}

	// map matrix frames to animation data
	MapFramesToAnimations ( pObject, false );

	// store original vertex data immediately
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
	}

	// ensure vectors are initially updated
	for ( int iFrame = 0; iFrame < pObject->iFrameCount; iFrame++ )
	{
		sFrame* pFrame = pObject->ppFrameList [ iFrame ];
		if ( pFrame ) pFrame->bVectorsCalculated = false;
	}

	// lee - 280306 - u6rc3 - trigger at least one anim-update if object contains bones
	pObject->bAnimUpdateOnce = true;
	
	// calculate bounding areas of object
	pObject->collision.fScaledLargestRadius = 0.001f; // 140207 - this triggers scalerd radius to be filled in CalculateAllBounds
	if ( !CalculateAllBounds ( pObject, false ) )
		return false;
	
	// now setup default position, scale etc.
	SetupDefaultPosition ( iID );

	// sphere uses fixed box check (when autocollision used)
	SetColToSpheres ( pObject );

	// set autocamera
	if ( fRadius==-1.0f ) fRadius = g_ObjectList [ iID ]->collision.fRadius;
	NewObjectAutoCam ( fRadius );
	
	// finally add the object's vertex and index data to the buffers
	g_ObjectList [ iID ]->bReplaceObjectFromBuffers = true;
	m_ObjectManager.g_bObjectReplacedUpdateBuffers = true;

	// Set the object to conform to VBIB usage
	g_ObjectList [ iID ]->bUsesItsOwnBuffers = g_bGlobalVBIBUsageFlag;

	// okay
	return true;
}

DARKSDK_DLL bool CreateNewObject ( int iID, LPSTR pName )
{
	// create a new object

	// ensure the object is okay to use
	if ( !ConfirmNewObject ( iID ) )
		return false;

	// check memory allocation
	ID_ALLOCATION ( iID );

	// create a new, empty mesh
	if ( !CreateMesh ( &g_ObjectList [ iID ], pName ) )
		return false;

	// add object id to shortlist
	AddObjectToObjectListRef ( iID );

	// object created successfully
	return true;
}

DARKSDK_DLL bool DeleteObjectSpecial ( int iID )
{
	// lee - 030306 - u60 - excluded objects are not caught by old way
	if ( !CheckObjectExist ( iID ) )
		return false;

	// remove object from buffers
	sObject* pObject = g_ObjectList [ iID ];

    // If this object is dependent on another, decrease the other objects dependency count
    if (pObject->pObjectDependency)
    {
        pObject->pObjectDependency->dwDependencyCount--;
        pObject->pObjectDependency = NULL;
    }

    // leefix - 010306 - u60 - no need to remove resource or further instances of an instanced object
	sObject* pInstanceOf = pObject->pInstanceOfObject;
	if ( pInstanceOf==NULL )
	{
		// remove resource of this object
		m_ObjectManager.RemoveBuffersUsedByObject ( pObject );

        // Only need to search for dependents if there are some for this object
        if (pObject->dwDependencyCount)
        {
		    // ensure all instances associated with this object are removed also
		    for ( int iShortList = 0; iShortList < g_iObjectListRefCount; iShortList++ )
		    {
			    // get index from shortlist
			    int iScanObjectID = g_ObjectListRef [ iShortList ];

			    // see if we have a valid list
			    sObject* pScanObject = g_ObjectList [ iScanObjectID ];
			    if ( !pScanObject ) continue;

			    // if not this object and an instance of it, delete it also
			    if ( pScanObject->pObjectDependency==pObject )
			    {
				    // delete instance/shared-clone of the master object (being deleted)
				    DeleteObjectSpecial ( iScanObjectID );

				    // as recursing through shortlist, we may have skipped one so backtrack
				    if ( iShortList >= 0 )
					    iShortList--;
			    }
            }
		}
	}
	else
	{
		// instance has created some memory
		SAFE_DELETE_ARRAY ( pObject->pInstanceMeshVisible );
	}
	
	// delete mesh
	if ( !DeleteMesh ( &g_ObjectList [ iID ] ) )
		return false;

	// clear item from list perminantly
	RemoveObjectFromObjectListRef ( iID );
	g_ObjectList [ iID ] = NULL;

	// ensure temp lists are cleared as this object no longer exists
	m_ObjectManager.UpdateTextures();

	// object deleted okay
	return true;
}

DARKSDK_DLL bool CreateNewObject ( int iID, LPSTR pName, int iFrame )
{
	// ensure the object is okay to use
	if ( !ConfirmNewObject ( iID ) )
		return false;

	// check memory allocation
	ID_ALLOCATION ( iID );

	// create a new, empty mesh
	if ( !CreateMesh ( &g_ObjectList [ iID ], pName ) )
		return false;

	sMesh*  pMesh  = g_ObjectList [ iID ]->pFrame->pMesh;
	sFrame* pFrame = g_ObjectList [ iID ]->pFrame;

	for ( int i = 0; i < iFrame - 1; i++ )
	{
		pFrame->pChild        = new sFrame;
		pFrame->pChild->pMesh = new sMesh;

		pMesh  = pFrame->pChild->pMesh;
		pFrame = pFrame->pChild;
	}

	// add object id to shortlist
	AddObjectToObjectListRef ( iID );

	return true;
}

DARKSDK_DLL DWORD VectortoRGBA( GGVECTOR3* v, FLOAT fHeight )
{
    DWORD r = (DWORD)( 127.0f * v->x + 128.0f );
    DWORD g = (DWORD)( 127.0f * v->y + 128.0f );
    DWORD b = (DWORD)( 127.0f * v->z + 128.0f );
    DWORD a = (DWORD)( 255.0f * fHeight );
    
    return( (a<<24L) + (r<<16L) + (g<<8L) + (b<<0L) );
}

DARKSDK_DLL bool DoesDepthBufferHaveStencil(GGFORMAT d3dfmt)
{
	if(d3dfmt==GGFMT_D24S8) return true;
	if(d3dfmt==GGFMT_D15S1) return true;
	if(d3dfmt==GGFMT_D24X4S4) return true;
	return false;
}

DARKSDK_DLL int GetBitDepthFromFormat(GGFORMAT Format)
{
	#ifdef DX11
    switch (Format)
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
        return 32;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
        return 24;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
        return 16;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_NV11:
        return 12;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
        return 8;

    case DXGI_FORMAT_R1_UNORM:
        return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        return 8;
    default:
        return 0;
    }
	return 0;
	#else
	switch(Format)
	{
		case GGFMT_R8G8B8 :		return 24;	break;
		case GGFMT_A8R8G8B8 :		return 32;	break;
		case GGFMT_X8R8G8B8 :		return 32;	break;
		case GGFMT_R5G6B5 :		return 16;	break;
		case GGFMT_X1R5G5B5 :		return 16;	break;
		case GGFMT_A1R5G5B5 :		return 16;	break;
		case GGFMT_A4R4G4B4 :		return 16;	break;
		case GGFMT_A8	:			return 8;	break;
		case GGFMT_R3G3B2 :		return 8;	break;
		case GGFMT_A8R3G3B2 :		return 16;	break;
		case GGFMT_X4R4G4B4 :		return 16;	break;
		case GGFMT_A2B10G10R10 :	return 32;	break;
		case GGFMT_G16R16 :		return 32;	break;
		case GGFMT_A8P8 :			return 8;	break;
		case GGFMT_P8 :			return 8;	break;
		case GGFMT_L8 :			return 8;	break;
		case GGFMT_A8L8 :			return 16;	break;
		case GGFMT_A4L4 :			return 8;	break;
	}
	#endif
	return 0;
}

DARKSDK_DLL LPGGCUBETEXTURE CreateNewImageCubeMap ( int i1, int i2, int i3, int i4, int i5, int i6 )
{
	// Image determines cubetexture format
	DWORD dwWidth = ImageWidth ( i1 );
	GGFORMAT Format = GGFMT_A8R8G8B8;

	// Individual Images
	LPGGCUBETEXTURE pCubeTexture = NULL;
	#ifdef DX11
	#else
	HRESULT hRes = D3DXCreateCubeTexture( m_pD3D, dwWidth, 1, GGUSAGE_RENDERTARGET, Format, D3DPOOL_DEFAULT, &pCubeTexture );
	if ( pCubeTexture )
	{
		// Copy images to cubetexture surfaces
		for ( DWORD s=0; s<6; s++)
		{
			LPGGSURFACE pCubeSurface=NULL;
			HRESULT hRes = pCubeTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)s, 0, &pCubeSurface );
			if ( pCubeSurface )
			{
				int iImage;
				if ( s==0 ) iImage = i1;
				if ( s==1 ) iImage = i2;
				if ( s==2 ) iImage = i3;
				if ( s==3 ) iImage = i4;
				if ( s==4 ) iImage = i5;
				if ( s==5 ) iImage = i6;
				LPGGSURFACE pImageSurface=NULL;
				LPGGTEXTURE pImageTexture = GetImagePointer ( iImage );
				pImageTexture->GetSurfaceLevel ( 0, &pImageSurface );
				if ( pImageSurface )
				{
					D3DXLoadSurfaceFromSurface( pCubeSurface, NULL, 0, pImageSurface, NULL, 0, D3DX_DEFAULT, 0);
					pImageSurface->Release();
				}
				pCubeSurface->Release();
			}
		}
	}
	#endif
	return pCubeTexture;
}

// Data Access Functions

DARKSDK_DLL void GetMeshData( int iMeshID, DWORD* pdwFVF, DWORD* pdwFVFSize, DWORD* pdwVertMax, LPSTR* pData, DWORD* dwDataSize, bool bLockData )
{
	// Read Data
	if(bLockData==true)
	{
		// ensure mesh is valid
		if ( !ConfirmMesh ( iMeshID ) )
			return;

		// get mesh pointer
		sMesh* pOriginalMesh = g_RawMeshList [ iMeshID ];	// mesh data
		if ( pOriginalMesh==NULL )
			return;

		// create new mesh from scratch
		sMesh* pActualMesh = new sMesh;

		// copy from existing mesh
		GGMATRIX matWorld;
		GGMatrixIdentity ( &matWorld );
		MakeMeshFromOtherMesh ( true, pActualMesh, pOriginalMesh, &matWorld );

		// eliminate any index data from new mesh
		ConvertLocalMeshToVertsOnly ( pActualMesh, false );

		// mesh data
		*pdwFVF = pActualMesh->dwFVF;
		*pdwFVFSize = pActualMesh->dwFVFSize;
		*pdwVertMax = pActualMesh->dwVertexCount;

		// create memory
		DWORD dwSizeOfData = pActualMesh->dwVertexCount * pActualMesh->dwFVFSize;
		*pData = new char[dwSizeOfData];
		*dwDataSize = dwSizeOfData;

		// copy mesh to new memory
		memcpy( *pData, pActualMesh->pVertexData, dwSizeOfData );

		// free temp mesh
		SAFE_DELETE(pActualMesh);
	}
	else
	{
		// free memory
		delete *pData;
	}
}

DARKSDK_DLL void SetMeshData( int iMeshID, DWORD dwFVF, DWORD dwFVFSize, LPSTR pMeshData, DWORD dwVertMax )
{
	// ensure mesh is valid, else delete it
	if ( !ConfirmNewMesh ( iMeshID ) )
		return;

	// create new mesh from data
	sMesh* pMesh = new sMesh;
	if (!MakeLocalMeshFromPureMeshData ( pMesh, dwFVF, dwFVFSize, (float*)pMeshData, dwVertMax, GGPT_TRIANGLELIST ) )
		return;

	// check memory allocation
	ID_MESH_ALLOCATION ( iMeshID );

	// mesh stored in list
	g_RawMeshList [ iMeshID ] = pMesh;

}

DARKSDK_DLL void CreateNewOrSharedEffect ( sMesh* pMesh, bool bChangeMesh )
{
	// Search if effect already loaded
	int iEffectIDFound = 0;
	for ( int iEffectID=0; iEffectID<MAX_EFFECTS; iEffectID++ )
	{
		if ( m_EffectList [ iEffectID ] )
		{
			if ( _stricmp ( m_EffectList [ iEffectID ]->pEffectObj->m_pEffectName, (LPSTR)pMesh->pEffectName )==NULL )
			{
				iEffectIDFound=iEffectID;
				break;
			}
		}
	}

	// Found matching effect name
	if ( iEffectIDFound > 0 )
	{
		// apply to specific mesh
		sEffectItem* pEffectItem = m_EffectList [ iEffectIDFound ];
		SetSpecialEffect ( pMesh, pEffectItem->pEffectObj, bChangeMesh );
		pMesh->bVertexShaderEffectRefOnly = true;
	}
	else
	{
		// find effectID from SETUP shaders
		iEffectIDFound = 0;
		for ( int iShaderIndex=0; iShaderIndex<SHADERSARRAYMAX; iShaderIndex++ )
		{
			if ( g_sShaders [ iShaderIndex ].pEffect )
			{
				if ( _stricmp ( g_sShaders[iShaderIndex].pName, (LPSTR)pMesh->pEffectName )==NULL )
				{
					iEffectIDFound = iShaderIndex;
					break;
				}
			}
		}
		if ( iEffectIDFound > 0 )
		{
			// No, create external effect obj
			m_EffectList [ iEffectIDFound ] = new sEffectItem;
			m_EffectList [ iEffectIDFound ]->pEffectObj = new cExternalEffect;
			m_EffectList [ iEffectIDFound ]->pEffectObj->m_dwEffectIndex = iEffectIDFound;
			m_EffectList [ iEffectIDFound ]->pEffectObj->m_bDoNotGenerateExtraData = 0;
			m_EffectList [ iEffectIDFound ]->pEffectObj->m_bNeed8BonesPerVertex = false;
			m_EffectList [ iEffectIDFound ]->pEffectObj->Load ( iEffectIDFound, (LPSTR)pMesh->pEffectName, false, false );
			SetSpecialEffect ( pMesh, m_EffectList [ iEffectIDFound ]->pEffectObj );
		}
	}
}

DARKSDK sFrame*  CreateNewFrame ( sObject* pObject, LPSTR pName, bool bNewMesh )
{
	sMesh* pNewMesh = NULL;
    
    // If a mesh is required, create one
    if (bNewMesh)
    {
        pNewMesh = new sMesh;
	    if ( ! pNewMesh )
            return 0;
    }

    // Add a new frame using the (possibly existing) mesh
    AddNewFrame ( pObject, pNewMesh, pName );

    // Identify the new frame pointer
    sFrame* pLastRootFrame = pObject->pFrame;
	while ( pLastRootFrame->pSibling )
		pLastRootFrame = pLastRootFrame->pSibling;

    // recreate all meshand frame lists
    CreateFrameAndMeshList ( pObject );

    // ensure bounds are recalculated
    pObject->bUpdateOverallBounds=true;

    // update mesh(es) of object
    m_ObjectManager.RefreshObjectInBuffer ( pObject );

    return pLastRootFrame;
}

// DX11 replacement fixed function globals
DBPRO_GLOBAL GGMATRIX						g_matTransformWORLD;
DBPRO_GLOBAL GGMATRIX						g_matTransformVIEW;
DBPRO_GLOBAL GGMATRIX						g_matTransformPROJECTION;

void COMMONSetTransform ( int index, GGMATRIX* matrix )
{
	// replacement for GGSetTransform (no more fixed function)
	switch ( index )
	{
		case 2 : g_matTransformVIEW = *matrix; break;
		case 3 : g_matTransformPROJECTION = *matrix; break;
		case 256 : g_matTransformWORLD = *matrix; break;
		default : MessageBox ( NULL, "unknown SetTransform", "", MB_OK ); break;
	};
}

void COMMONGetTransform ( int index, GGMATRIX* matrix )
{
	// replacement for GGGetTransform (no more fixed function)
	switch ( index )
	{
		case 2 : *matrix = g_matTransformVIEW; break;
		case 3 : *matrix = g_matTransformPROJECTION; break;
		case 256 : *matrix = g_matTransformWORLD; break;
		default : MessageBox ( NULL, "unknown GetTransform", "", MB_OK ); break;
	};
}

#ifdef DX11
void COMMONSetEffectFloat ( GGHANDLE handle, float value )
{
	handle->AsScalar()->SetFloat ( value );
}
void COMMONSetEffectVector ( GGHANDLE handle, GGVECTOR4* value )
{
	handle->AsVector()->SetFloatVector ( (const float*)value );
}
void COMMONSetEffectMatrix ( GGHANDLE handle, GGMATRIX* value )
{
	handle->AsMatrix()->SetMatrix ( (const float*)value );
}
void COMMONSetEffectMatrixTransposeArray ( GGHANDLE handle, GGMATRIX* value, UINT count )
{
	handle->AsMatrix()->SetMatrixTransposeArray ( (const float*)value, 0, count );
}
#endif

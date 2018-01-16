#include "cparticlec.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"
#include "CGfxC.h"
#include "CImageC.h"
#include "cVectorC.h"
#include "CCameraC.h"

extern GlobStruct*				g_pGlob;

DBPRO_GLOBAL HINSTANCE					g_Image;
DBPRO_GLOBAL CParticleManager			m_ParticleManager;
DBPRO_GLOBAL tagParticleData*			m_ptr;

extern DBPRO_GLOBAL LPGGDEVICE			m_pD3D;
DBPRO_GLOBAL GGMATERIAL				gWhiteMaterial;

extern tagObjectPos* m_pParticlePos;

DARKSDK void ParticlesConstructorD3D ( HINSTANCE hSetup, HINSTANCE hImage )
{
	#ifdef DX11
	m_pD3D = GetDirect3DDevice ( );
	#else
	m_pD3D = GetDirect3DDevice ( );
	#endif

	// Prepare a pure white material for texture usage
	m_ptr = NULL;
	memset ( &gWhiteMaterial, 0, sizeof ( gWhiteMaterial ) );
	gWhiteMaterial.Diffuse.r = 1.0f;
	gWhiteMaterial.Diffuse.g = 1.0f;
	gWhiteMaterial.Diffuse.b = 1.0f;
	gWhiteMaterial.Diffuse.a = 1.0f;	
	gWhiteMaterial.Ambient.r = 0.0f;
	gWhiteMaterial.Ambient.g = 0.0f;
	gWhiteMaterial.Ambient.b = 0.0f;
	gWhiteMaterial.Ambient.a = 0.0f;
	gWhiteMaterial.Specular = gWhiteMaterial.Ambient;
	gWhiteMaterial.Emissive = gWhiteMaterial.Ambient;
	gWhiteMaterial.Power = 1.0f;
}

DARKSDK void ParticlesConstructor ( )
{
	ParticlesConstructorD3D ( NULL, NULL );
}

DARKSDK void FreeParticle( tagParticleData* ptr )
{
	// Delete particles
	PARTICLE* pParticle = ptr->m_pParticles;
	while ( pParticle )
	{
		PARTICLE* pNextParticle = pParticle->m_pNext;
		SAFE_DELETE(pParticle);
		pParticle = pNextParticle;
	}
	pParticle = ptr->m_pParticlesFree;
	while ( pParticle )
	{
		PARTICLE* pNextParticle = pParticle->m_pNext;
		SAFE_DELETE(pParticle);
		pParticle = pNextParticle;
	}

	// Delete vertx buffer and obj
	SAFE_RELEASE(ptr->m_pVB);
	SAFE_DELETE(ptr);
}

DARKSDK void ParticlesDestructorD3D ( void )
{
	link* check = m_ParticleManager.GetList()->m_start;
	while(check)
	{
		tagParticleData* ptr = NULL;
		ptr = m_ParticleManager.GetData ( check->id );
		if ( ptr == NULL ) return;
		FreeParticle ( ptr );
		check = check->next;
	}
	m_ParticleManager.DeleteAll();

}

DARKSDK void ParticlesDestructor ( void )
{
	ParticlesDestructorD3D();
}

DARKSDK void ParticlesSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void ParticlesPassCoreData( LPVOID pGlobPtr )
{
	// Global Data passed in
	g_pGlob = (GlobStruct*)pGlobPtr;
}

DARKSDK void ParticlesRefreshD3D ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		ParticlesDestructorD3D();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		ParticlesConstructorD3D ( g_pGlob->g_GFX, g_pGlob->g_Image );
		ParticlesPassCoreData ( g_pGlob );
	}
}

DARKSDK void ParticlesUpdate ( void )
{
	m_ParticleManager.Update ( );
}

bool UpdateParticlePtr ( int iID )
{
	m_ptr  = NULL;
	m_ptr  = m_ParticleManager.GetData ( iID );

	if ( m_ptr == NULL )
		return false;

	m_pParticlePos = ( tagObjectPos* ) m_ptr;

	return true;
}

DARKSDK void DeleteParticles ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
		return;

	FreeParticle ( m_ptr );
	m_ParticleManager.Delete ( iID );
}

DARKSDK bool MakeParticles ( int iID, DWORD dwFlush, DWORD dwDiscard, float fRadius, int iImage )
{
	HRESULT				hr;
	tagParticleData		m_Data;

	memset ( &m_Data, 0, sizeof ( m_Data ) );

	m_Data.pD3D             = m_pD3D;
	m_Data.m_fRadius        = fRadius;
	m_Data.m_dwBase         = dwDiscard;
	m_Data.m_dwDiscard      = dwDiscard;
	m_Data.m_dwFlush        = dwFlush;
	m_Data.m_dwParticles    = 0;
	m_Data.m_dwParticlesLim = 2048;
	m_Data.m_pParticles     = NULL;
	m_Data.m_pParticlesFree = NULL;
	m_Data.m_pVB            = NULL;

	m_Data.fSecsPerFrame        = 0.005f;
	m_Data.dwNumParticlesToEmit = 4;
	m_Data.fEmitVel             = 4.0;
	m_Data.vPosition			= GGVECTOR3 ( 0.0, 0.0, 0.0 );
	m_Data.EmitColor  		    = D3DXCOLOR   ( 1.0, 1.0, 1.0, 1.0 );
	m_Data.fTime = 0.0f;

	m_Data.m_bEffect			= false;

	m_Data.m_bVisible			= true;
	m_Data.m_fGravity			= 5.0f;
	m_Data.m_iLife				= 100;
	m_Data.m_iFloorActive		= 1;
	m_Data.m_fChaos				= 0.0f;

	m_Data.m_bGhost				= true;
	m_Data.m_iGhostMode			= -1;
	m_Data.m_fGhostPercentage	= -1.0f;
	
	m_Data.m_pTexture = GetImagePointer ( iImage );

	if ( FAILED ( hr = m_pD3D->CreateVertexBuffer ( 
														dwDiscard * sizeof ( POINTVERTEX ), 
														D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_POINTS, 
														D3DFVF_POINTVERTEX,
														D3DPOOL_DEFAULT, 
														&m_Data.m_pVB,
														NULL
												  ) ) )
	{
		RunTimeError ( RUNTIMEERROR_PARTICLESCOULDNOTBECREATED );
		return false;
	}

	m_ParticleManager.Add ( &m_Data, iID );

	// Scale controlled by radius
	float fScale = fRadius*100.0f;
	ParticleScale  ( iID, fScale,fScale,fScale );

	ParticlePosition ( iID, 0.0f, 0.0f, 0.0f );
	ParticleRotate ( iID, 0.0f, 0.0f, 0.0f );

	return true;
}

DARKSDK void SetParticlesColor ( int iID, int iRed, int iGreen, int iBlue )
{
	if ( !UpdateParticlePtr ( iID ) )
		return;

	m_ptr->EmitColor = D3DXCOLOR ( ( float ) iRed / 100.0f, ( float ) iGreen / 100.0f, ( float ) iBlue / 100.0f, 1.0f );
}        

DARKSDK void SetParticlesTime ( int iID, float fTime )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->fTime = fTime;
}

DARKSDK void CreateParticlesEx ( int iID, int iImageIndex, int maxParticles, float fRadius )
{
	if ( UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESALREADYEXISTS);
		return;
	}

	DWORD dwFlush = maxParticles;
	DWORD dwDiscard = dwFlush*5;
	MakeParticles ( iID, dwFlush, dwDiscard, fRadius, iImageIndex );

	// Set autocam
	#if DB_PRO
	SetAutoCam ( 0.0f, 0.0f, 0.0f, fRadius );
	#endif
}

DARKSDK void DeleteParticlesEx ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	DeleteParticles ( iID );
}

DARKSDK void SetParticlesSecondsPerFrame ( int iID, float fTime )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->fSecsPerFrame = fTime;
}

DARKSDK void SetParticlesNumberOfEmmissions ( int iID, int iNumber )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->dwNumParticlesToEmit = iNumber;
}

DARKSDK void SetParticlesVelocity ( int iID, float fVelocity )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->fEmitVel = fVelocity;
}

DARKSDK void SetParticlesPosition ( int iID, float fX, float fY, float fZ )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	ParticlePosition ( iID,  fX, fY, fZ );
}

DARKSDK void SetParticlesEmitPosition ( int iID, float fX, float fY, float fZ )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	fX /= m_ptr->m_fRadius;
	fY /= m_ptr->m_fRadius;
	fZ /= m_ptr->m_fRadius;

	m_ptr->vPosition = GGVECTOR3 ( fX, fY, fZ );
}

DARKSDK void SetColorParticlesEx ( int iID, int iRed, int iGreen, int iBlue )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->EmitColor = D3DXCOLOR ( ( float ) iRed / 255.0f, ( float ) iGreen / 255.0f, ( float ) iBlue / 255.0f, 1.0f );
}

DARKSDK void SetParticlesRotation ( int iID, float fX, float fY, float fZ )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	ParticleRotate(iID, fX, fY, fZ);
}

DARKSDK void SetParticlesGravity	( int iID, float fGravity )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}	
	m_ptr->m_fGravity=fGravity;
}

DARKSDK void SetParticlesLife ( int iID, int iLifeValue )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}	
	m_ptr->m_iLife=iLifeValue;
}

DARKSDK void SetParticlesMask				( int iID, int iCameraMask )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}
DARKSDK void EmitSingleParticle		( int iID, float fX, float fY, float fZ )
{
	// MessageBox ( NULL, "DX10", "", MB_OK );
}

DARKSDK void SetParticlesFloor ( int iID, int iFlag )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}	
	m_ptr->m_iFloorActive=iFlag;
}

DARKSDK void SetParticlesChaos ( int iID, float fChaos )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	m_ptr->m_fChaos=fChaos/1000.0f;
}

DARKSDK void GhostParticlesOn ( int iID, int iMode )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->m_bGhost = true;
	m_ptr->m_iGhostMode = iMode;
}

DARKSDK void GhostParticlesOff ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->m_bGhost = false;
	m_ptr->m_iGhostMode = -1;
	m_ptr->m_fGhostPercentage = -1.0f;
}

DARKSDK void GhostParticles ( int iID, int iMode, float fPercentage )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	// clip value
	if ( fPercentage >= 0.0f && fPercentage <= 100.0f )
	{
		m_ptr->m_bGhost = true;
		m_ptr->m_iGhostMode = iMode;
		m_ptr->m_fGhostPercentage = fPercentage;
	}
}

DARKSDK void HideParticles ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->m_bVisible=false;
}

DARKSDK void ShowParticles ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}

	m_ptr->m_bVisible=true;
}

DARKSDK void  SetParticlesPositionVector3 ( int iID, int iVector )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	GGVECTOR3 vec = GetVector3 ( iVector );
	ParticlePosition ( iID,  vec.x, vec.y, vec.z );
}

DARKSDK void GetParticlesPositionVector3 ( int iVector, int iID )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	SetVector3 ( iVector, m_ptr->vecPosition.x, m_ptr->vecPosition.y, m_ptr->vecPosition.z );
}

DARKSDK void  SetParticlesRotationVector3 ( int iID, int iVector )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	GGVECTOR3 vec = GetVector3 ( iVector );
	ParticleRotate ( iID,  vec.x, vec.y, vec.z );
}

DARKSDK void GetParticlesRotationVector3 ( int iVector, int iID )
{
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNUMBERILLEGAL);
		return;
	}
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	SetVector3 ( iVector, m_ptr->vecRotate.x, m_ptr->vecRotate.y, m_ptr->vecRotate.z );
}

DARKSDK SDK_BOOL ParticlesExist ( int iID )
{
	if ( UpdateParticlePtr ( iID ) )
		return true;
	else
		return false;
}

DARKSDK SDK_FLOAT ParticlesPositionX ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return 0;
	}

	float fResult = m_ptr->vecPosition.x;
	return SDK_RETFLOAT(fResult);
}

DARKSDK SDK_FLOAT ParticlesPositionY ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return 0;
	}

	float fResult = m_ptr->vecPosition.y;
	return SDK_RETFLOAT(fResult);
}

DARKSDK SDK_FLOAT ParticlesPositionZ ( int iID )
{
	if ( !UpdateParticlePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_PARTICLESNOTEXIST);
		return 0;
	}

	float fResult = m_ptr->vecPosition.z;
	return SDK_RETFLOAT(fResult);
}

#include ".\..\error\cerror.h"
#include "globstruct.h"
#include ".\..\DBOFormat\DBOData.h"
#include "CGfxC.h"
#include "CLightC.h"
#include "cVectorC.h"
#include "CObjectsC.h"

namespace
{
    float EulerWrapAngle(float Angle)
    {
        float NewAngle = fmod(Angle, 360.0f);
        if (NewAngle < 0.0f)
            NewAngle+=360.0f;
        return NewAngle;
    }

    tagLightVector GetAngleFromDirectionVector(tagLightVector Vector)
    {
	    // Find Y and then X axis rotation
	    double yangle=atan2(Vector.x, Vector.z);
	    if(yangle<0.0) yangle+=GGToRadian(360.0);
	    if(yangle>=GGToRadian(360.0)) yangle-=GGToRadian(360.0);

	    GGMATRIX yrotate;
	    GGMatrixRotationY ( &yrotate, (float)-yangle );
	    GGVec3TransformCoord ( (GGVECTOR3*)&Vector, (GGVECTOR3*)&Vector, &yrotate );

	    double xangle=-atan2(Vector.y, Vector.z);
	    if(xangle<0.0) xangle+=GGToRadian(360.0);
	    if(xangle>=GGToRadian(360.0)) xangle-=GGToRadian(360.0);

		tagLightVector vec;
		vec.x = EulerWrapAngle(GGToDegree((float)xangle));
		vec.x = EulerWrapAngle(GGToDegree((float)yangle));
		vec.z = 0.0f;
        return vec;
    }
}

extern GlobStruct*			g_pGlob;

namespace
{
    CLightManager					m_LightManager;
    tagLightData*					m_ptr;
    int								m_iAmbientParcentage;
    int								m_iAmbientRed;
    int								m_iAmbientGreen;
    int								m_iAmbientBlue;
    float							g_fFogStartDistance;
    float							g_fFogEndDistance;
    DWORD                          	g_dwFogColor;
}

DARKSDK void LightConstructorD3D ( )
{
	// Default Ambient Settings
	m_iAmbientParcentage	= 25;
	m_iAmbientRed			= 255;
	m_iAmbientGreen			= 255;
	m_iAmbientBlue			= 255;

	// Pass Default Ambient Percentage (for material construction)
	g_pGlob->iFogState = 0;

	// Default Fog Settings
	g_fFogStartDistance = 0.0f;
	g_fFogEndDistance   = 3000.0f;
    g_dwFogColor        = 0;

	// Default Light
	int iID=0;
	MakeLight ( iID );
}

DARKSDK void LightConstructor ( void )
{
	LightConstructorD3D();
}

DARKSDK void SetLightSpecularOn ( void )
{
	///m_pD3D->SetRenderState ( D3DRS_SPECULARENABLE, TRUE );
}

DARKSDK void SetLightSpecularOff ( void )
{
	///m_pD3D->SetRenderState ( D3DRS_SPECULARENABLE, FALSE );
}

DARKSDK void LightDestructorD3D ( void )
{
    m_LightManager.DeleteAll( m_pD3D );
}

DARKSDK void LightDestructor ( void )
{
	LightDestructorD3D();
}

DARKSDK void SetLightErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void LightPassCoreData( LPVOID pGlobPtr )
{
	// Held in Core, used here..
	g_pGlob = (GlobStruct*)pGlobPtr;
}

DARKSDK void LightRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		LightDestructorD3D();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		LightConstructorD3D ( );
		LightPassCoreData ( g_pGlob );
	}
}

DARKSDK bool UpdateLightPtr ( int iID )
{
	// update internal data for the sprite
	m_ptr  = m_LightManager.GetData ( iID );
	if ( m_ptr == NULL )
		return false;

	return true;
}

DARKSDK void* GetLightData( int iID )
{
    UpdateLightPtr( iID );
    return (void*)m_ptr;
}

DARKSDK void MakeLight ( int iID )
{
	// variable declarations
	tagLightData	m_Data;

	// clean out structure
	memset ( &m_Data,       0, sizeof ( m_Data    ) );
	memset ( &m_Data.light, 0, sizeof ( tagLight ) );

	// set structure data
	m_Data.eType = eDirectionalLight;
	m_Data.bEnable = true;
	m_Data.fRange = 1000.0f;
	m_Data.dwColor = GGCOLOR_ARGB ( 255,255,255,255 );
	m_LightManager.Add ( &m_Data, iID );

	// default new light is point (lee - 230306 - u6b4 - aligned default from basic3d)
	if(iID==0)
		SetDirectionalLight ( 0, 0.1f, -0.7f, 0.2f ); // SetDirectional ( 0, 0.1f, -0.5f, 0.5f );
	else
		SetPointLight( iID, 0.0f, 0.0f, 0.0f );

	// light on by default
	///m_pD3D->LightEnable ( iID, TRUE );
}

DARKSDK void DeleteLight ( int iID )
{
	// Delete Light
	m_LightManager.Delete ( iID, m_pD3D );
}

DARKSDK bool LightExist ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return false;

	return true;
}

DARKSDK int GetLightType ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return -1;

	return m_ptr->light.Type;
}

DARKSDK float LightPositionX ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Position.x;
}

DARKSDK float LightPositionY ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Position.y;
}

DARKSDK float LightPositionZ ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Position.z;
}

DARKSDK float LightDirectionX ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Direction.x;
}

DARKSDK float LightDirectionY ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Direction.y;
}

DARKSDK float LightDirectionZ ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0.0f;

	return m_ptr->light.Direction.z;
}

DARKSDK bool LightVisible ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return false;

	return m_ptr->bEnable;
}

DARKSDK int LightRange ( int iID )
{
	if ( !UpdateLightPtr ( iID ) )
		return 0;

	return ( int ) m_ptr->light.Range;
}

DARKSDK void MakeLightEx ( int iID )
{
	// create a new light
	if ( iID==0 )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOZERO );
		return;
	}
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTALREADYEXISTS );
		return;
	}

	// Make light for real
	MakeLight ( iID );
}

DARKSDK void DeleteLightEx ( int iID )
{
	// delete a light
	if ( iID==0 )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOZERO );
		return;
	}
	if ( iID < 1 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// Delete Light
	DeleteLight ( iID );
}

DARKSDK void SetPointLight ( int iID, float fX, float fY, float fZ )
{
	// setup a point light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// fill in data
	m_ptr->eType = ePointLight;
	memset ( &m_ptr->light, 0, sizeof ( tagLight ) );
	m_ptr->light.Type         = GGLIGHT_POINT;						// set to point light
	m_ptr->light.Diffuse.r    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.g    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.b    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Position.x = fX;									// position of light
	m_ptr->light.Position.y = fY;	
	m_ptr->light.Position.z = fZ;	
	m_ptr->light.Range        = m_ptr->fRange;								// light range
	m_ptr->light.Attenuation0 = 1.0f;								// light intensity over distance
	m_ptr->light.Attenuation2 = 1.0f/(float)pow((m_ptr->fRange/3.0f),2);;								// light intensity over distance

	// set standard specular light
	m_ptr->light.Specular.r  = 1.0f;
	m_ptr->light.Specular.g  = 1.0f;
	m_ptr->light.Specular.b  = 1.0f;
	m_ptr->light.Specular.a  = 1.0f;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetSpotLight ( int iID, float fInner, float fOuter )
{
	// setup a spot light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// fill in data
	m_ptr->eType = eSpotLight;
	memset ( &m_ptr->light, 0, sizeof ( tagLight ) );
	m_ptr->light.Type         = GGLIGHT_SPOT;						// set to spot light
	m_ptr->light.Diffuse.r    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.g    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.b    = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Position     = { 0.0f,  0.0f, 0.0f };				// position of light
	m_ptr->light.Direction    = { 0.0f, -1.0f, 0.0f };				// direction of light
	m_ptr->light.Range        = m_ptr->fRange;							// light range
	m_ptr->light.Attenuation0 = 1.0f;								// light intensity over distance
	m_ptr->light.Theta        = GGToRadian ( fInner );			// angle of inner cone of spot light
	m_ptr->light.Phi          = GGToRadian ( fOuter );			// angle of outer cone of spot light

	// lee - 050406 - u6rc6 - added falloff to make it work
	m_ptr->light.Falloff	  = 1.0f;
	
	// set standard specular light
	m_ptr->light.Specular.r  = 1.0f;
	m_ptr->light.Specular.g  = 1.0f;
	m_ptr->light.Specular.b  = 1.0f;
	m_ptr->light.Specular.a  = 1.0f;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetDirectionalLight ( int iID, float fX, float fY, float fZ )
{
	// setup a directional light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// lee - 230306 - u6b4 - these settings updated from Basic3D which seemed to overrule the old default start (ie no specular)
	m_ptr->eType = eDirectionalLight;
	memset ( &m_ptr->light, 0, sizeof ( tagLight ) );
	m_ptr->light.Type       = GGLIGHT_DIRECTIONAL;				// set to directional light
	m_ptr->light.Diffuse.r  = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.g  = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Diffuse.b  = 1.0f;								// diffuse colour to reflect all
	m_ptr->light.Direction.x = fX;								// light direction
	m_ptr->light.Direction.y = fY;
	m_ptr->light.Direction.z = fZ;		
	m_ptr->light.Range		= 5000.0f;							// defaul range
	m_ptr->light.Attenuation0 = 1.0f;							// light intensity over distance

	// update light
	///m_pD3D->SetLight ( iID, (D3DLIGHT9*)&m_ptr->light );
}

DARKSDK void SetStaticLightMode ( int iID, int iFlag )
{
	// MessageBox ( NULL, "DX10 ONLY", "SetStaticMode", MB_OK );
}

DARKSDK void SetLightRange ( int iID, float fRange )
{
	// sets the range of the specified light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->fRange = fRange;
	m_ptr->light.Range = fRange;
	m_ptr->light.Attenuation0 = 1.0f;
	m_ptr->light.Attenuation2 = 1.0f/(float)pow((m_ptr->fRange/3.0f),2);

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetLightSpecular ( int iID, float fA, float fR, float fG, float fB )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	tagLightColorValue color = { fR, fG, fB, fA };
	m_ptr->light.Specular = color;
}

DARKSDK void SetLightAmbient ( int iID, float fA, float fR, float fG, float fB )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	tagLightColorValue color = { fR, fG, fB, fA };
	m_ptr->light.Ambient = color;
}

DARKSDK void SetLightFalloff ( int iID, float fValue )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->light.Falloff = fValue;
}

DARKSDK void SetLightAttenuation0 ( int iID, float fValue )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->light.Attenuation0 = fValue;
}

DARKSDK void SetLightAttenuation1 ( int iID, float fValue )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->light.Attenuation1 = fValue;
}

DARKSDK void SetLightAttenuation2 ( int iID, float fValue )
{
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->light.Attenuation2 = fValue;
}

DARKSDK void SetLightToObject ( int iID, int iObjectID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
    
    sObject* Object = GetObjectData ( iObjectID );
    if ( !Object )
    {
        RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS );
        return;
    }

    // Position the light at the same point as the object
	m_ptr->light.Position.x = Object->position.vecPosition.x;
	m_ptr->light.Position.y = Object->position.vecPosition.y;
	m_ptr->light.Position.z = Object->position.vecPosition.z;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetLightToObjectOrientation ( int iID, int iObjectID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
    sObject* Object = GetObjectData ( iObjectID );
    if ( !Object )
    {
        RunTimeError ( RUNTIMEERROR_B3DMODELNOTEXISTS );
        return;
    }

    // Rotate the light down the objects look vector
	m_ptr->light.Direction.x = Object->position.vecLook.x;
	m_ptr->light.Direction.y = Object->position.vecLook.y;
	m_ptr->light.Direction.z = Object->position.vecLook.z;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetLightColor ( int iID, DWORD dwColor )
{
	// Set color of light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// fill in properties
	int iR = (dwColor>>16) & 255;
	int iG = (dwColor>>8) & 255;
	int iB = dwColor & 255;
	m_ptr->light.Diffuse.r  = ( float ) iR / 255.0f;
	m_ptr->light.Diffuse.g  = ( float ) iG / 255.0f;
	m_ptr->light.Diffuse.b  = ( float ) iB / 255.0f;
	
	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void SetLightColorEx ( int iID, int iR, int iG, int iB )
{
	// Set color of light
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// fill in properties
	m_ptr->light.Diffuse.r  = ( float ) iR / 255.0f;
	m_ptr->light.Diffuse.g  = ( float ) iG / 255.0f;
	m_ptr->light.Diffuse.b  = ( float ) iB / 255.0f;
	
	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void PositionLight ( int iID, float fX, float fY, float fZ )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	m_ptr->light.Position.x = fX;
	m_ptr->light.Position.y = fY;
	m_ptr->light.Position.z = fZ;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void RotateLight ( int iID, float fX, float fY, float fZ )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// change to XM calls (D3D wrapper)
	GGMATRIX matRotateX, matRotateY, matRotateZ, matRotate;
	GGMatrixRotationX   ( &matRotate, GGToRadian(fX) );
	GGMatrixRotationY   ( &matRotateY, GGToRadian(fY) );
	GGMatrixMultiply    ( &matRotate, &matRotate, &matRotateY );
	GGMatrixRotationZ   ( &matRotateZ, GGToRadian(fZ) );
	GGMatrixMultiply    ( &matRotate, &matRotate, &matRotateZ );
	m_ptr->light.Direction.x=0.0f;
	m_ptr->light.Direction.y=0.0f;
	m_ptr->light.Direction.z=1.0f;
	GGVec3TransformCoord ( ( GGVECTOR3* ) &m_ptr->light.Direction, ( GGVECTOR3* ) &m_ptr->light.Direction, &matRotate );

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void PointLight ( int iID, float fX, float fY, float fZ )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	// Create Direction From relative offset
	m_ptr->light.Direction = m_ptr->light.Position;
	m_ptr->light.Direction.x = fX - m_ptr->light.Direction.x;
	m_ptr->light.Direction.y = fY - m_ptr->light.Direction.y;
	m_ptr->light.Direction.z = fZ - m_ptr->light.Direction.z;
	GGVec3Normalize( (GGVECTOR3*)&m_ptr->light.Direction, (GGVECTOR3*)&m_ptr->light.Direction );

	// update light
	///->SetLight ( iID, &m_ptr->light );
}

DARKSDK void HideLight ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	///m_pD3D->LightEnable ( iID, false );
	m_ptr->bEnable = false;
}

DARKSDK void ShowLight ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}

	///m_pD3D->LightEnable ( iID, true );
	m_ptr->bEnable = true;
}

DARKSDK void SetNormalizationOn ( void )
{
	///m_pD3D->SetRenderState ( D3DRS_NORMALIZENORMALS, TRUE );
}

DARKSDK void SetNormalizationOff ( void )
{
	///m_pD3D->SetRenderState ( D3DRS_NORMALIZENORMALS, FALSE );
}

DARKSDK void SetAmbientLight ( int iPercent )
{
	// set global value
	m_iAmbientParcentage = iPercent;

	// Set Ambient State
	float fR = m_iAmbientRed * (m_iAmbientParcentage/100.0f);
	float fG = m_iAmbientGreen * (m_iAmbientParcentage/100.0f);
	float fB = m_iAmbientBlue * (m_iAmbientParcentage/100.0f);
	///m_pD3D->SetRenderState ( D3DRS_AMBIENT, GGCOLOR_ARGB ( 255, (DWORD)fR, (DWORD)fG, (DWORD)fB ) );
}

DARKSDK int GetAmbientLight ( void )
{
	// 301007 - return ambient light percentage
	return m_iAmbientParcentage;
}

DARKSDK void SetAmbientColor ( DWORD dwColor )
{
	int iR = (dwColor>>16) & 255;
	int iG = (dwColor>>8) & 255;
	int iB = dwColor & 255;
	m_iAmbientRed = iR;
	m_iAmbientGreen = iG;
	m_iAmbientBlue = iB;

	// Set the ambience
	float fR = m_iAmbientRed * (m_iAmbientParcentage/100.0f);
	float fG = m_iAmbientGreen * (m_iAmbientParcentage/100.0f);
	float fB = m_iAmbientBlue * (m_iAmbientParcentage/100.0f);
	///m_pD3D->SetRenderState ( D3DRS_AMBIENT, GGCOLOR_ARGB ( 255, (DWORD)fR, (DWORD)fG, (DWORD)fB ) );
}

DARKSDK void FogOn ( void )
{
	if(g_pGlob) g_pGlob->iFogState=1;
	///m_pD3D->SetRenderState ( D3DRS_FOGENABLE, TRUE );
	///m_pD3D->SetRenderState ( D3DRS_FOGCOLOR, g_dwFogColor );
	///m_pD3D->SetRenderState ( D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR );
	///m_pD3D->SetRenderState ( D3DRS_FOGSTART, *( DWORD * ) ( &g_fFogStartDistance ) );
	///m_pD3D->SetRenderState ( D3DRS_FOGEND,   *( DWORD * ) ( &g_fFogEndDistance   ) );
}

DARKSDK void FogOff ( void )
{
	if(g_pGlob) g_pGlob->iFogState=0;
	///m_pD3D->SetRenderState ( D3DRS_FOGENABLE, FALSE );
}

DARKSDK void SetFogColor ( DWORD dwColor )
{
    g_dwFogColor = dwColor;
	///m_pD3D->SetRenderState ( D3DRS_FOGCOLOR, dwColor );
}

DARKSDK void SetFogColorEx ( int iR, int iG, int iB, int iA )
{
	// inline macro to use XMDX11 equiv and map back to GGCOLOR_RGBA
    SetFogColor( GGCOLOR_RGBA ( iR, iG, iB, iA ) );
}

DARKSDK void SetFogColorEx ( int iR, int iG, int iB )
{
    SetFogColor( GGCOLOR_RGBA ( iR, iG, iB, 0 ) );
}

DARKSDK void SetFogDistance ( int iDistance )
{
	g_fFogEndDistance = ( float ) iDistance;
	///m_pD3D->SetRenderState ( D3DRS_FOGSTART, *( DWORD * ) ( &g_fFogStartDistance ) );
	///m_pD3D->SetRenderState ( D3DRS_FOGEND,   *( DWORD * ) ( &g_fFogEndDistance   ) );
}

DARKSDK void SetFogDistance ( int iStartDistance, int iFinishDistance )
{
	g_fFogStartDistance = ( float ) iStartDistance;
	g_fFogEndDistance = ( float ) iFinishDistance;
	///m_pD3D->SetRenderState ( D3DRS_FOGSTART, *( DWORD * ) ( &g_fFogStartDistance ) );
	///m_pD3D->SetRenderState ( D3DRS_FOGEND,   *( DWORD * ) ( &g_fFogEndDistance   ) );
}

DARKSDK void  SetLightPositionVector3 ( int iID, int iVector )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	GGVECTOR3 vec = GetVector3 ( iVector );
	m_ptr->light.Position.x = vec.x;
	m_ptr->light.Position.y = vec.y;
	m_ptr->light.Position.z = vec.z;

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void GetLightPositionVector3 ( int iVector, int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	SetVector3 ( iVector, m_ptr->light.Position.x, m_ptr->light.Position.y, m_ptr->light.Position.z );
}

DARKSDK void  SetLightRotationVector3 ( int iID, int iVector )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	GGVECTOR3 vec = GetVector3 ( iVector );
	GGMATRIX matRotateX,matRotateY,matRotateZ, matRotate;
	GGMatrixRotationX   ( &matRotate, GGToRadian(vec.x) );
	GGMatrixRotationY   ( &matRotateY, GGToRadian(vec.y) );
	GGMatrixMultiply    ( &matRotate, &matRotate, &matRotateY );
	GGMatrixRotationZ   ( &matRotateZ, GGToRadian(vec.z) );
	GGMatrixMultiply    ( &matRotate, &matRotate, &matRotateZ );
	m_ptr->light.Direction.x=0.0f;
	m_ptr->light.Direction.y=0.0f;
	m_ptr->light.Direction.z=1.0f;
	GGVec3TransformCoord ( ( GGVECTOR3* ) &m_ptr->light.Direction, ( GGVECTOR3* ) &m_ptr->light.Direction, &matRotate );

	// update light
	///m_pD3D->SetLight ( iID, &m_ptr->light );
}

DARKSDK void GetLightRotationVector3 ( int iVector, int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return;
	}
	if ( !VectorExist ( iVector ) )
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
	tagLightVector vec = GetAngleFromDirectionVector( m_ptr->light.Direction );
	SetVector3 ( iVector, vec.x, vec.y, vec.z );
}

//
// Command Expression Functions
//

DARKSDK int GetLightExistEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
		return 0;

	return 1;
}

DARKSDK int GetLightTypeEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return -1;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return -1;
	}
	return m_ptr->eType;
}

DARKSDK float GetLightPositionXEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Position.x;
}

DARKSDK float GetLightPositionYEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Position.y;
}

DARKSDK float GetLightPositionZEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Position.z;
}

DARKSDK float GetLightDirectionXEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Direction.x;
}

DARKSDK float GetLightDirectionYEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Direction.y;
}

DARKSDK float GetLightDirectionZEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->light.Direction.z;
}

DARKSDK int GetLightVisibleEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	if(m_ptr->bEnable==true)
		return 1;
	else
		return 0;
}

DARKSDK float GetLightRangeEx ( int iID )
{
	if ( iID < 0 || iID > MAXIMUMVALUE )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNUMBERILLEGAL );
		return 0;
	}
	if ( !UpdateLightPtr ( iID ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DLIGHTNOTEXIST );
		return 0;
	}
	return m_ptr->fRange;
}

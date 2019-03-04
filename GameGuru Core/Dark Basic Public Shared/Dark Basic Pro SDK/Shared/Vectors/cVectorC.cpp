#include "cVectorC.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"
#include ".\..\camera\ccameradatac.h"
#include "CGfxC.h"
#include "CObjectsC.h"

extern GlobStruct*					g_pGlob;
extern LPGGDEVICE			m_pD3D;

namespace
{
    bool							g_bRTE					= false;
    cDataManager					m_DataManager;
    HINSTANCE						g_GFX;
}

namespace
{
    template <typename T>
    inline T* GetCorrectedPtr( int iID )
    {
        T* pVector = m_DataManager.GetData<T>( iID );
        return pVector;
    }

    inline Vector2* GetVector2Ptr( int iID )
    {
        return GetCorrectedPtr<Vector2>( iID );
    }

    inline Vector3* GetVector3Ptr( int iID )
    {
        return GetCorrectedPtr<Vector3>( iID );
    }

    inline Vector4Ex* GetVector4Ptr( int iID )
    {
        return GetCorrectedPtr<Vector4Ex>( iID );
    }

    inline Matrix* GetMatrixPtr( int iID )
    {
        return GetCorrectedPtr<Matrix>( iID );
    }
}

DARKSDK void VectorConstructor ( )
{
}

DARKSDK void VectorDestructor ( void ) 
{
    m_DataManager.ClearAll();
}

DARKSDK void SetVectorErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void VectorPassCoreData( LPVOID pGlobPtr )
{
	// Held in Core, used here..
	g_pGlob = (GlobStruct*)pGlobPtr;
	g_bRTE = true;
}

DARKSDK void VectorRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		VectorDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		VectorConstructor ( );
		VectorPassCoreData ( g_pGlob );
	}
}


DARKSDK GGVECTOR2 GetVector2 ( int iID )
{
    Vector2* pVector = GetVector2Ptr( iID );

    if (!pVector)
        return GGVECTOR2( 0.0f, 0.0f );

    return pVector->Get();
}

DARKSDK GGVECTOR3 GetVector3 ( int iID )
{
    Vector3* pVector = GetVector3Ptr( iID );

    if (!pVector)
        return GGVECTOR3( 0.0f, 0.0f, 0.0f );

    return pVector->Get();
}

DARKSDK GGVECTOR4 GetVector4 ( int iID )
{
    Vector4Ex* pVector = GetVector4Ptr( iID );

    if (!pVector)
        return GGVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );

    return pVector->Get();
}

DARKSDK GGMATRIX GetMatrix ( int iID )
{
    Matrix* pVector = GetMatrixPtr( iID );

    if (!pVector)
        return GGMATRIX();

    return pVector->Get();
}

DARKSDK int VectorExist ( int iID )
{
    return m_DataManager.Exist( iID ) ? 1 : 0;
}

DARKSDK bool CheckVectorTypeIsValid ( int iID, int iType )
{
    BaseVector* pVectorBase = m_DataManager.GetData<BaseVector>( iID );

    // Confirm it exists and is of the correct type
    if (!pVectorBase || pVectorBase->GetType() != iType)
        return false;

    return true;
}

DARKSDK SDK_BOOL MakeVector2 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    m_DataManager.Add( new Vector2, iID );
    return true;
}

DARKSDK SDK_BOOL DeleteVector2 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    Vector2* pVector = GetVector2Ptr( iID );

    if (!pVector)
    {
		//silent fail - no way to detect if vector exists in DBP code!
		return false;
	}

    m_DataManager.Delete ( iID );
	return true;
}

DARKSDK void SetVector2 ( int iID, float fX, float fY )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pVector = GetVector2Ptr( iID );

    if (!pVector)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pVector->Get() = GGVECTOR2( fX, fY );
}

DARKSDK void CopyVector2 ( int iDestination, int iSource  )
{
    if (iDestination < 1 || iDestination > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pSource = GetVector2Ptr( iSource );
    Vector2* pDest   = GetVector2Ptr( iDestination );

    if (!pSource || !pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() = pSource->Get();
}

DARKSDK void AddVector2 ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pResult = GetVector2Ptr( iResult );
    Vector2* pA      = GetVector2Ptr( iA );
    Vector2* pB      = GetVector2Ptr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() + pB->Get();
}

DARKSDK void SubtractVector2 ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pResult = GetVector2Ptr( iResult );
    Vector2* pA      = GetVector2Ptr( iA );
    Vector2* pB      = GetVector2Ptr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() - pB->Get();
}

DARKSDK void MultiplyVector2 ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pDest = GetVector2Ptr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() *= fValue;
}

DARKSDK void DivideVector2 ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector2* pDest = GetVector2Ptr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() /= fValue;
}

DARKSDK SDK_FLOAT GetXVector2 ( int iID )
{
    Vector2* pID = GetVector2Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
        float f = 0.0f;
		return f;
	}

    return pID->Get().x;
}

DARKSDK SDK_FLOAT GetYVector2 ( int iID )
{
    Vector2* pID = GetVector2Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
        float f = 0.0f;
		return SDK_RETFLOAT( f );
	}

    return SDK_RETFLOAT( pID->Get().y );
}

////////////////////////////////////////////////////////////////////////////////
// VECTOR3 HANDLING ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DARKSDK SDK_BOOL MakeVector3 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    m_DataManager.Add( new Vector3, iID );
    return true;
}

DARKSDK SDK_BOOL DeleteVector3 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    Vector3* pVector = GetVector3Ptr( iID );

    if (!pVector)
    {
		//silent fail - no way to detect if vector exists in DBP code!
        //RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return false;
	}

	m_DataManager.Delete ( iID );
	return true;
}

DARKSDK void SetVector3 ( int iID, float fX, float fY, float fZ )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pID = GetVector3Ptr( iID );

	if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	pID->Get() = GGVECTOR3 ( fX, fY, fZ );
}

DARKSDK SDK_FLOAT GetXVector3 ( int iID )
{
    Vector3* pID = GetVector3Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().x;
}

DARKSDK SDK_FLOAT GetYVector3 ( int iID )
{
    Vector3* pID = GetVector3Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return SDK_RETFLOAT( pID->Get().y );
}

DARKSDK SDK_FLOAT GetZVector3 ( int iID )
{
    Vector3* pID = GetVector3Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().z;
}

DARKSDK void CopyVector3 ( int iDestination, int iSource )
{
    if (iDestination < 1 || iDestination > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pSource = GetVector3Ptr( iSource );
    Vector3* pDest   = GetVector3Ptr( iDestination );

    if (!pSource || !pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() = pSource->Get();
}

DARKSDK void AddVector3 ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pA      = GetVector3Ptr( iA );
    Vector3* pB      = GetVector3Ptr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() + pB->Get();
}

DARKSDK void SubtractVector3 ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pA      = GetVector3Ptr( iA );
    Vector3* pB      = GetVector3Ptr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() - pB->Get();
}

DARKSDK void MultiplyVector3 ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pDest = GetVector3Ptr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() *= fValue;
}

DARKSDK void DivideVector3 ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pDest = GetVector3Ptr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() /= fValue;
}

DARKSDK SDK_BOOL IsEqualVector3 ( int iA, int iB )
{
    Vector3* pA = GetVector3Ptr( iA );
    Vector3* pB = GetVector3Ptr( iB );

    if (!pA || !pB)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return false;
	}

    if (pA->Get() == pB->Get())
        return true;

    return false;
}

DARKSDK SDK_BOOL IsNotEqualVector3 ( int iA, int iB )
{
    return !IsEqualVector3( iA, iB );
}

DARKSDK void ProjectVector3 ( int iResult, int iSource, int iProjectionMatrix, int iViewMatrix, int iWorldMatrix )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pSource = GetVector3Ptr( iSource );
    Matrix*  pProjM  = GetMatrixPtr ( iProjectionMatrix );
    Matrix*  pViewM  = GetMatrixPtr ( iViewMatrix );
    Matrix*  pWorldM = GetMatrixPtr ( iWorldMatrix );

    if (!pResult || !pSource)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
    if (!pProjM || !pViewM || !pWorldM)
    {
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	#ifdef DX11
	#else
	GGVIEWPORT	viewport;
	m_pD3D->GetViewport ( &viewport );
    D3DXVec3Project ( &pResult->Get(), &pSource->Get(), &viewport,
                      &pProjM->Get(), &pViewM->Get(), &pWorldM->Get() ); 
	#endif
}

DARKSDK void TransformVectorCoordinates3 ( int iResult, int iSource, int iMatrix )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pSource = GetVector3Ptr( iSource );
    Matrix*  pMatrix = GetMatrixPtr ( iMatrix );

    if (!pResult || !pSource)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
    if (!pMatrix)
    {
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}
	
	GGVec3TransformCoord ( &pResult->Get(), &pSource->Get(), &pMatrix->Get() );
}

DARKSDK void TransformVectorNormalCoordinates3 ( int iResult, int iSource, int iMatrix )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pSource = GetVector3Ptr( iSource );
    Matrix*  pMatrix = GetMatrixPtr ( iMatrix );

    if (!pResult || !pSource)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
    if (!pMatrix)
    {
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}
	
	GGVec3TransformNormal ( &pResult->Get(), &pSource->Get(), &pMatrix->Get() );
}

DARKSDK void CatmullRomVector3 ( int iResult, int iA, int iB, int iC, int iD, float s )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pA      = GetVector3Ptr( iA );
    Vector3* pB      = GetVector3Ptr( iB );
    Vector3* pC      = GetVector3Ptr( iC );
    Vector3* pD      = GetVector3Ptr( iD );

    if (!pResult || !pA || !pB || !pC || !pD)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	GGVec3CatmullRom ( &pResult->Get(), &pA->Get(), &pB->Get(), &pC->Get(), &pD->Get(), s ); 
}

DARKSDK void CrossProductVector3 ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector3* pResult = GetVector3Ptr( iResult );
    Vector3* pA      = GetVector3Ptr( iA );
    Vector3* pB      = GetVector3Ptr( iB );

    if (!pResult || !pA || !pB)
    {
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	GGVec3Cross ( &pResult->Get(), &pA->Get(), &pB->Get() );
}

DARKSDK SDK_FLOAT DotProductVector3 ( int iA, int iB )
{
    Vector3* pA      = GetVector3Ptr( iA );
    Vector3* pB      = GetVector3Ptr( iB );

    if (!pA || !pB)
    {
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    float result = GGVec3Dot ( &pA->Get(), &pB->Get() );
	return result;
}

DARKSDK SDK_FLOAT GetLengthVector3 ( int iID )
{
    Vector3* pID = GetVector3Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    float result = GGVec3Length ( &pID->Get() );
	return result;
}

DARKSDK SDK_FLOAT GetLengthSquaredVector3 ( int iID )
{
    Vector3* pID = GetVector3Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    float result = GGVec3LengthSq ( &pID->Get() );
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// VECTOR4 HANDLING ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DARKSDK SDK_BOOL MakeVector4 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    m_DataManager.Add( new Vector4Ex, iID );
    return true;
}

DARKSDK SDK_BOOL DeleteVector4 ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return false;
    }

    Vector4Ex* pVector = GetVector4Ptr( iID );

    if (!pVector)
    {
		//silent fail - no way to detect if vector exists in DBP code!
        //RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return false;
	}

	m_DataManager.Delete ( iID );
	return true;
}

DARKSDK void SetVector4 ( int iID, float fX, float fY, float fZ, float fW )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector4Ex* pID = GetVector4Ptr( iID );

	if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	pID->Get() = GGVECTOR4 ( fX, fY, fZ, fW );
}

DARKSDK void CopyVector4 ( int iDestination, int iSource )
{
    if (iDestination < 1 || iDestination > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector4Ex* pSource = GetVector4Ptr( iSource );
    Vector4Ex* pDest   = GetVector4Ptr( iDestination );

    if (!pSource || !pDest)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

    pDest->Get() = pSource->Get();
}

DARKSDK SDK_FLOAT GetXVector4 ( int iID )
{
    Vector4Ex* pID = GetVector4Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().x;
}

DARKSDK SDK_FLOAT GetYVector4 ( int iID )
{
    Vector4Ex* pID = GetVector4Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().y;
}

DARKSDK SDK_FLOAT GetZVector4 ( int iID )
{
    Vector4Ex* pID = GetVector4Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().z;
}

DARKSDK SDK_FLOAT GetWVector4 ( int iID )
{
    Vector4Ex* pID = GetVector4Ptr( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return 0.0f;
	}

    return pID->Get().w;
}

DARKSDK void TransformVector4 ( int iResult, int iSource, int iMatrix )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_VECTORNUMBERILLEGAL );
        return;
    }

    Vector4Ex* pResult = GetVector4Ptr( iResult );
    Vector4Ex* pSource = GetVector4Ptr( iSource );
    Matrix*  pMatrix = GetMatrixPtr ( iMatrix );

    if (!pResult || !pSource)
    {
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}
    if (!pMatrix)
    {
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
    }

    GGVec4Transform ( &pResult->Get(), &pSource->Get(), &pMatrix->Get() );
}

////////////////////////////////////////////////////////////////////////////////
// Matrix Handling /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DARKSDK SDK_BOOL MakeMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return false;
    }

    Matrix* pMatrix = new Matrix;
    m_DataManager.Add( pMatrix, iID );
	return true;
}


DARKSDK void GetWorldMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pMatrix = GetMatrixPtr( iID );

    if (!pMatrix)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixIdentity ( &pMatrix->Get() );
}

DARKSDK void GetViewMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pMatrix = GetMatrixPtr( iID );

    if (!pMatrix)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	if ( GetCameraInternalData!=NULL )
	{
		tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );
		pMatrix->Get() = m_Camera_Ptr->matView;
	}
}

DARKSDK void GetProjectionMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pMatrix = GetMatrixPtr( iID );

    if (!pMatrix)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    if ( GetCameraInternalData!=NULL )
	{
		tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );
		pMatrix->Get() = m_Camera_Ptr->matProjection;
		GGSetTransform ( GGTS_PROJECTION, &m_Camera_Ptr->matProjection );
	}
}

DARKSDK void SetProjectionMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pMatrix = GetMatrixPtr( iID );
    if (!pMatrix)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    if ( GetCameraInternalData!=NULL )
	{
		tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( g_pGlob->dwCurrentSetCameraID );
		m_Camera_Ptr->matProjection = pMatrix->Get();
	}
}

DARKSDK SDK_BOOL DeleteMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return false;
    }

    Matrix* pMatrix = GetMatrixPtr( iID );

    if (!pMatrix)
    {
		//silent fail - no way to detect if matrix exists in DBP code!
		//RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return false;
	}

	m_DataManager.Delete ( iID );
	return SDK_TRUE;
}

DARKSDK SDK_BOOL MatrixExist ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
        return SDK_FALSE;

    Matrix* pMatrix = GetMatrixPtr( iID );
    if (!pMatrix)
    {
		return SDK_FALSE;
	}
	else
	{
		return SDK_TRUE;
	}
}

DARKSDK void CopyMatrix ( int iDestination, int iSource )
{
    if (iDestination < 1 || iDestination > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pSource = GetMatrixPtr( iSource );
    Matrix* pDest   = GetMatrixPtr( iDestination );

    if (!pSource || !pDest)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pDest->Get() = pSource->Get();
}

DARKSDK void AddMatrix ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pResult = GetMatrixPtr( iResult );
    Matrix* pA      = GetMatrixPtr( iA );
    Matrix* pB      = GetMatrixPtr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() + pB->Get();
}

DARKSDK void SubtractMatrix ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pResult = GetMatrixPtr( iResult );
    Matrix* pA      = GetMatrixPtr( iA );
    Matrix* pB      = GetMatrixPtr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() - pB->Get();
}

DARKSDK void MultiplyMatrix ( int iResult, int iA, int iB )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pResult = GetMatrixPtr( iResult );
    Matrix* pA      = GetMatrixPtr( iA );
    Matrix* pB      = GetMatrixPtr( iB );

    if (!pA || !pB || !pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pResult->Get() = pA->Get() * pB->Get();
}

DARKSDK void MultiplyMatrix ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pDest = GetMatrixPtr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pDest->Get() *= fValue;
}

DARKSDK void DivideMatrix ( int iID, float fValue )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pDest = GetMatrixPtr( iID );

    if (!pDest)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    pDest->Get() /= fValue;
}

DARKSDK SDK_BOOL IsEqualMatrix ( int iA, int iB )
{
    Matrix* pA = GetMatrixPtr( iA );
    Matrix* pB = GetMatrixPtr( iB );

    if (!pA || !pB)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return false;
	}

    if (pA->Get() == pB->Get())
        return true;

    return false;
}

DARKSDK SDK_BOOL IsNotEqualMatrix ( int iA, int iB )
{
    return !IsEqualMatrix( iA, iB );
}

DARKSDK void SetIdentityMatrix ( int iID )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr( iID );

    if (!pID)
    {
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}
	
	GGMatrixIdentity ( &pID->Get() );
}

DARKSDK SDK_FLOAT InverseMatrix ( int iResult, int iSource )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return 0;
    }

    Matrix* pResult = GetMatrixPtr( iResult );
    Matrix* pSource = GetMatrixPtr( iSource );

    if (!pResult || !pSource)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return false;
	}

    float fDeterminant = 0.0f;
	GGMatrixInverse ( &pResult->Get(), &fDeterminant, &pSource->Get() ); 

	// return the determinant
	return SDK_RETFLOAT(fDeterminant);
}

DARKSDK void BuildLookAtRHMatrix ( int iResult, int iVectorEye, int iVectorAt, int iVectorUp )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );
    Vector3* pVectorEye = GetVector3Ptr( iVectorEye );
    Vector3* pVectorAt  = GetVector3Ptr( iVectorAt );
    Vector3* pVectorUp  = GetVector3Ptr( iVectorUp );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    if (!pVectorEye || !pVectorAt || !pVectorUp)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	GGMatrixLookAtRH ( &pResult->Get(), &pVectorEye->Get(), &pVectorAt->Get(), &pVectorUp->Get() ); 
}

DARKSDK void BuildLookAtLHMatrix ( int iResult, int iVectorEye, int iVectorAt, int iVectorUp )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );
    Vector3* pVectorEye = GetVector3Ptr( iVectorEye );
    Vector3* pVectorAt  = GetVector3Ptr( iVectorAt );
    Vector3* pVectorUp  = GetVector3Ptr( iVectorUp );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    if (!pVectorEye || !pVectorAt || !pVectorUp)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	GGMatrixLookAtLH ( &pResult->Get(), &pVectorEye->Get(), &pVectorAt->Get(), &pVectorUp->Get() ); 
}

DARKSDK void BuildOrthoRHMatrix ( int iResult, float fWidth, float fHeight, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixOrthoRH ( &pResult->Get(), fWidth, fHeight, fZNear, fZFar ); 
}

DARKSDK void BuildOrthoLHMatrix ( int iResult, float fWidth, float fHeight, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixOrthoLH ( &pResult->Get(), fWidth, fHeight, fZNear, fZFar ); 
}

DARKSDK void BuildPerspectiveRHMatrix ( int iResult, float fWidth, float fHeight, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixPerspectiveRH ( &pResult->Get(), fWidth, fHeight, fZNear, fZFar ); 
}

DARKSDK void BuildPerspectiveLHMatrix ( int iResult, float fWidth, float fHeight, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixPerspectiveLH ( &pResult->Get(), fWidth, fHeight, fZNear, fZFar ); 
}

DARKSDK void BuildPerspectiveFovRHMatrix ( int iResult, float fFOV, float fAspect, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixPerspectiveFovRH ( &pResult->Get(), fFOV, fAspect, fZNear, fZFar ); 
}

DARKSDK void BuildPerspectiveFovLHMatrix ( int iResult, float fFOV, float fAspect, float fZNear, float fZFar )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixPerspectiveFovLH ( &pResult->Get(), fFOV, fAspect, fZNear, fZFar ); 
}

DARKSDK void BuildReflectionMatrix ( int iResult, float a, float b, float c, float d )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult    = GetMatrixPtr ( iResult );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixReflect ( &pResult->Get(), &GGPLANE ( a, b, c, d ) ); 
}

DARKSDK void BuildRotationAxisMatrix ( int iResult, int iVectorAxis, float fAngle )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix*  pResult     = GetMatrixPtr ( iResult );
    Vector3* pVectorAxis = GetVector3Ptr( iVectorAxis );

    if (!pResult)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}
    if (!pVectorAxis)
	{
		RunTimeError ( RUNTIMEERROR_VECTORNOTEXIST );
		return;
	}

	GGMatrixRotationAxis ( &pResult->Get(), &pVectorAxis->Get(), fAngle );
}

DARKSDK void RotateXMatrix ( int iID, float fAngle )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    GGMatrixRotationX ( &pID->Get(), fAngle );
}

DARKSDK void RotateYMatrix ( int iID, float fAngle )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    GGMatrixRotationY ( &pID->Get(), fAngle );
}

DARKSDK void RotateZMatrix ( int iID, float fAngle )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

    GGMatrixRotationZ ( &pID->Get(), fAngle );
}

DARKSDK void RotateYawPitchRollMatrix ( int iID, float fYaw, float fPitch, float fRoll )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixRotationYawPitchRoll ( &pID->Get(), fYaw, fPitch, fRoll );
}

DARKSDK void ScaleMatrix ( int iID, float fX, float fY, float fZ )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixScaling ( &pID->Get(), fX, fY, fZ );
}

DARKSDK void TranslateMatrix ( int iID, float fX, float fY, float fZ )
{
    if (iID < 1 || iID > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pID = GetMatrixPtr ( iID );

    if (!pID)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixTranslation ( &pID->Get(), fX, fY, fZ );
}

DARKSDK void TransposeMatrix ( int iResult, int iSource )
{
    if (iResult < 1 || iResult > MAXIMUMVALUE)
    {
        RunTimeError ( RUNTIMEERROR_MATRIX4NUMBERILLEGAL );
        return;
    }

    Matrix* pResult = GetMatrixPtr( iResult );
    Matrix* pSource = GetMatrixPtr( iSource );

	if (!pResult || !pSource)
	{
		RunTimeError ( RUNTIMEERROR_MATRIX4NOTEXIST );
		return;
	}

	GGMatrixTranspose ( &pResult->Get(), &pSource->Get() );
}

DARKSDK SDK_FLOAT GetMatrixElement ( int iMatrix, int iElement )
{
    Matrix*  pMatrix = GetMatrixPtr ( iMatrix );

    if (!pMatrix)
    {
		RunTimeError(RUNTIMEERROR_MATRIX4NOTEXIST);
		return 0;
	}

    GGMATRIX mat = pMatrix->Get();

    float res = 0.0;
	if (iElement>=0 && iElement<=15)
        res = mat[iElement];

	return SDK_RETFLOAT(res);
}

#ifndef _CVECTORC_H_
#define _CVECTORC_H_

#include "directx-macros.h"

#include ".\..\Dark Basic Pro SDK\Shared\Vectors\cVectorManagerC.h"
#include ".\..\Dark Basic Pro SDK\Shared\Vectors\cVectorDataC.h"

#undef DARKSDK
#define DARKSDK

#define SDK_BOOL int
#define SDK_TRUE 1
#define SDK_FALSE 0
#define SDK_FLOAT float
#define SDK_RETFLOAT float

#define SDK_LPSTR DWORD
#define SDK_RETSTR

DARKSDK void			VectorConstructor 						( );
DARKSDK void			VectorDestructor  						( void );
DARKSDK void			SetVectorErrorHandler 					( LPVOID pErrorHandlerPtr );
DARKSDK void			VectorPassCoreData						( LPVOID pGlobPtr );
DARKSDK void			VectorRefreshGRAFIX 							( int iMode );

DARKSDK GGVECTOR2		GetVector2 							( int iID );
DARKSDK GGVECTOR3		GetVector3 							( int iID );
DARKSDK GGVECTOR4		GetVector4 							( int iID );
DARKSDK GGMATRIX 	GetMatrix							( int iID );
DARKSDK SDK_FLOAT		GetMatrixElement					( int iMatrix, int iElement );
DARKSDK int				VectorExist							( int iID );

DARKSDK bool			CheckVectorTypeIsValid					( int iID, int iType );

DARKSDK SDK_BOOL		MakeVector2							( int iID );
DARKSDK SDK_BOOL		DeleteVector2						( int iID );
DARKSDK void			SetVector2							( int iID, float fX, float fY );
DARKSDK void			CopyVector2							( int iSource, int iDestination );
DARKSDK void			AddVector2							( int iResult, int iA, int iB );
DARKSDK void			SubtractVector2						( int iResult, int iA, int iB );
DARKSDK void			MultiplyVector2						( int iID, float fValue );
DARKSDK void			DivideVector2						( int iID, float fValue );
DARKSDK SDK_FLOAT		GetXVector2							( int iID );
DARKSDK SDK_FLOAT		GetYVector2							( int iID );
DARKSDK void			GetBaryCentricCoordinatesVector2	( int iResult, int iA, int iB, int iC, float f, float g );
DARKSDK void			CatmullRomVector2					( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK SDK_FLOAT		GetCCWVector2						( int iA, int iB );
DARKSDK SDK_FLOAT		DotProductVector2					( int iA, int iB );
DARKSDK void			HermiteVector2						( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK SDK_FLOAT		GetLengthVector2					( int iID );
DARKSDK SDK_FLOAT		GetLengthSquaredVector2				( int iID );
DARKSDK void			LinearInterpolationVector2			( int iResult, int iA, int iB, float s );
DARKSDK void			MaximizeVector2						( int iResult, int iA, int iB );
DARKSDK void			MinimizeVector2						( int iResult, int iA, int iB );
DARKSDK void			NormalizeVector2					( int iResult, int iSource );
DARKSDK void			ScaleVector2						( int iResult, int iSource, float s );
DARKSDK void			TransformVectorCoordinates2			( int iResult, int iSource, int iMatrix );

DARKSDK SDK_BOOL		MakeVector3							( int iID );
DARKSDK SDK_BOOL		DeleteVector3						( int iID );
DARKSDK void			SetVector3							( int iID, float fX, float fY, float fZ );
DARKSDK void			CopyVector3							( int iSource, int iDestination );
DARKSDK void			AddVector3							( int iResult, int iA, int iB );
DARKSDK void			SubtractVector3						( int iResult, int iA, int iB );
DARKSDK void			MultiplyVector3						( int iID, float fValue );
DARKSDK void			DivideVector3						( int iID, float fValue );
DARKSDK SDK_BOOL		IsEqualVector3						( int iA, int iB );
DARKSDK SDK_BOOL		IsNotEqualVector3					( int iA, int iB );
DARKSDK void			GetBaryCentricCoordinatesVector3	( int iResult, int iA, int iB, int iC, float f, float g );
DARKSDK void			CatmullRomVector3					( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK void  			CrossProductVector3					( int iResult, int iA, int iB );
DARKSDK void  			HermiteVector3						( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK void  			LinearInterpolationVector3			( int iResult, int iA, int iB, float s );
DARKSDK void  			MaximizeVector3						( int iResult, int iA, int iB );
DARKSDK void  			MinimizeVector3						( int iResult, int iA, int iB );
DARKSDK void  			NormalizeVector3					( int iResult, int iSource );
DARKSDK void  			ScaleVector3						( int iResult, int iSource, float s );
DARKSDK SDK_FLOAT 		DotProductVector3					( int iA, int iB );
DARKSDK SDK_FLOAT		GetLengthVector3					( int iID );
DARKSDK SDK_FLOAT		GetLengthSquaredVector3				( int iID );
DARKSDK void  			ProjectVector3						( int iResult, int iSource, int iProjectionMatrix, int iViewMatrix, int iWorldMatrix );
DARKSDK void  			TransformVectorCoordinates3			( int iResult, int iSource, int iMatrix );
DARKSDK void  			TransformVectorNormalCoordinates3	( int iResult, int iSource, int iMatrix );
DARKSDK SDK_FLOAT		GetXVector3							( int iID );
DARKSDK SDK_FLOAT		GetYVector3							( int iID );
DARKSDK SDK_FLOAT		GetZVector3							( int iID );

DARKSDK SDK_BOOL		MakeVector4							( int iID );
DARKSDK SDK_BOOL		DeleteVector4						( int iID );
DARKSDK void  			SetVector4							( int iID, float fX, float fY, float fZ, float fW );
DARKSDK void  			CopyVector4							( int iSource, int iDestination );
DARKSDK void  			AddVector4							( int iResult, int iA, int iB );
DARKSDK void  			SubtractVector4						( int iResult, int iA, int iB );
DARKSDK void  			MultiplyVector4						( int iID, float fValue );
DARKSDK void  			DivideVector4						( int iID, float fValue );
DARKSDK SDK_BOOL		IsEqualVector4						( int iA, int iB );
DARKSDK SDK_BOOL		IsNotEqualVector4					( int iA, int iB );
DARKSDK SDK_FLOAT		GetXVector4							( int iID );
DARKSDK SDK_FLOAT		GetYVector4							( int iID );
DARKSDK SDK_FLOAT		GetZVector4							( int iID );
DARKSDK SDK_FLOAT		GetWVector4							( int iID );
DARKSDK void  			GetBaryCentricCoordinatesVector4	( int iResult, int iA, int iB, int iC, float f, float g );
DARKSDK void  			CatmullRomVector4					( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK void  			HermiteVector4						( int iResult, int iA, int iB, int iC, int iD, float s );
DARKSDK SDK_FLOAT		GetLengthVector4					( int iID );
DARKSDK SDK_FLOAT		GetLengthSquaredVector4				( int iID );
DARKSDK void  			LinearInterpolationVector4			( int iResult, int iA, int iB, float s );
DARKSDK void  			MaximizeVector4						( int iResult, int iA, int iB );
DARKSDK void  			MinimizeVector4						( int iResult, int iA, int iB );
DARKSDK void  			NormalizeVector4					( int iResult, int iSource );
DARKSDK void  			ScaleVector4						( int iResult, int iSource, float s );
DARKSDK void  			TransformVector4					( int iResult, int iSource, int iMatrix );

DARKSDK SDK_BOOL		MakeMatrix							( int iID );
DARKSDK SDK_BOOL		DeleteMatrix						( int iID );
DARKSDK SDK_BOOL		MatrixExist						( int iID );
DARKSDK void  			CopyMatrix							( int iSource, int iDestination );
DARKSDK void  			AddMatrix							( int iResult, int iA, int iB );
DARKSDK void  			SubtractMatrix						( int iResult, int iA, int iB );
DARKSDK void  			MultiplyMatrix						( int iResult, int iA, int iB );
DARKSDK void  			MultiplyMatrix						( int iID, float fValue );
DARKSDK void  			DivideMatrix						( int iID, float fValue );
DARKSDK SDK_BOOL		IsEqualMatrix						( int iA, int iB );
DARKSDK SDK_BOOL		IsNotEqualMatrix					( int iA, int iB );
DARKSDK void  			SetIdentityMatrix					( int iID );
DARKSDK SDK_FLOAT		InverseMatrix						( int iResult, int iSource );
DARKSDK SDK_BOOL		IsIdentityMatrix					( int iID );
DARKSDK void  			BuildLookAtRHMatrix					( int iResult, int iVectorEye, int iVectorAt, int iVectorUp );
DARKSDK void  			BuildLookAtLHMatrix					( int iResult, int iVectorEye, int iVectorAt, int iVectorUp );
DARKSDK void  			BuildOrthoRHMatrix					( int iResult, float fWidth, float fHeight, float fZNear, float fZFar );
DARKSDK void  			BuildOrthoLHMatrix					( int iResult, float fWidth, float fHeight, float fZNear, float fZFar );
DARKSDK void  			BuildPerspectiveRHMatrix			( int iResult, float fWidth, float fHeight, float fZNear, float fZFar );
DARKSDK void  			BuildPerspectiveLHMatrix			( int iResult, float fWidth, float fHeight, float fZNear, float fZFar );
DARKSDK void  			BuildPerspectiveFovRHMatrix			( int iResult, float fFOV, float fAspect, float fZNear, float fZFar );
DARKSDK void  			BuildPerspectiveFovLHMatrix			( int iResult, float fFOV, float fAspect, float fZNear, float fZFar );
DARKSDK void  			BuildReflectionMatrix				( int iResult, float a, float b, float c, float d );
DARKSDK void  			BuildRotationAxisMatrix				( int iResult, int iVectorAxis, float fAngle );
DARKSDK void  			RotateXMatrix						( int iID, float fAngle );
DARKSDK void  			RotateYMatrix						( int iID, float fAngle );
DARKSDK void  			RotateZMatrix						( int iID, float fAngle );
DARKSDK void  			RotateYawPitchRollMatrix			( int iID, float fYaw, float fPitch, float fRoll );
DARKSDK void  			ScaleMatrix							( int iID, float fX, float fY, float fZ );
DARKSDK void  			TranslateMatrix						( int iID, float fX, float fY, float fZ );
DARKSDK void  			TransposeMatrix						( int iResult, int iSource );
DARKSDK void  			GetWorldMatrix						( int iID );
DARKSDK void  			GetViewMatrix						( int iID );
DARKSDK void  			GetProjectionMatrix					( int iID );
DARKSDK void			SetProjectionMatrix					( int iID );

#endif _CVECTORC_H_

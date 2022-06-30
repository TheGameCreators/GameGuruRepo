#ifndef _CLIGHT_H_
#define _CLIGHT_H_

//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#include ".\..\Dark Basic Pro SDK\Shared\Light\CLightManagerC.h"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////
// DEFINES ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//#ifndef DARKSDK_COMPILE
//	#define DARKSDK __declspec ( dllexport )
//	#define DBPRO_GLOBAL 
//#else
//	#define DARKSDK static
//	#define DBPRO_GLOBAL static
//#endif
//#define SAFE_DELETE( p )       { if ( p ) { delete ( p );       ( p ) = NULL; } }
//#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release ( ); ( p ) = NULL; } }
//#define SAFE_DELETE_ARRAY( p ) { if ( p ) { delete [ ] ( p );   ( p ) = NULL; } }
#undef DARKSDK
#define DARKSDK


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#ifdef DARKSDK_COMPILE
		void	ConstructorLight				( HINSTANCE hSetup );
		void	DestructorLight					( void );
		void	SetErrorHandlerLight			( LPVOID pErrorHandlerPtr );
		void	PassCoreDataLight				( LPVOID pGlobPtr );
		void	RefreshGRAFIXLight					( int iMode );
#endif

DARKSDK void	LightConstructor				( void );
DARKSDK void	LightDestructor					( void );
DARKSDK void	SetLightErrorHandler			( LPVOID pErrorHandlerPtr );
DARKSDK void	LightPassCoreData				( LPVOID pGlobPtr );
DARKSDK void	LightRefreshGRAFIX					( int iMode );

DARKSDK	void	MakeLight						( int iID );
DARKSDK void	DeleteLight						( int iID );
DARKSDK bool	LightExist						( int iID );
DARKSDK int		GetLightType					( int iID );
DARKSDK float	LightPositionX					( int iID );
DARKSDK float	LightPositionY					( int iID );
DARKSDK float	LightPositionZ					( int iID );
DARKSDK float	LightDirectionX					( int iID );
DARKSDK float	LightDirectionY					( int iID );
DARKSDK float	LightDirectionZ					( int iID );
DARKSDK bool	LightVisible					( int iID );
DARKSDK int		LightRange						( int iID );
DARKSDK	bool	UpdateLightPtr					( int iID );
DARKSDK void*	GetLightData					( int iID );

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////
// COMMAND FUNCTIONS //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

DARKSDK void	MakeLightEx							( int iID );
DARKSDK void	DeleteLightEx						( int iID );

DARKSDK void	SetPointLight						( int iID, float fX, float fY, float fZ );
DARKSDK void	SetSpotLight						( int iID, float fInner, float fOuter );
DARKSDK void	SetDirectionalLight					( int iID, float fX, float fY, float fZ );
DARKSDK void	SetLightRange						( int iID, float fRange );
DARKSDK void	SetStaticLightMode					( int iID, int iFlag );

DARKSDK void	PositionLight						( int iID, float fX, float fY, float fZ );
DARKSDK void	RotateLight							( int iID, float fX, float fY, float fZ );
DARKSDK void	PointLight							( int iID, float fX, float fY, float fZ );

DARKSDK void	HideLight							( int iID );
DARKSDK void	ShowLight							( int iID );

DARKSDK void	SetLightToObject						( int iID, int iObjectID );
DARKSDK void	SetToObjectOrientation			( int iID, int iObjectID );
DARKSDK void	SetLightColor						( int iID, DWORD dwColor );
DARKSDK void	SetLightColorEx						( int iID, int iR, int iG, int iB );

DARKSDK void	SetNormalizationOn				( void );
DARKSDK void	SetNormalizationOff				( void );

DARKSDK void	SetAmbientLight						( int iPercent );
DARKSDK int		GetAmbientLight						( void );
DARKSDK void	SetAmbientColor					( DWORD dwColor );

DARKSDK void	FogOn							( void );
DARKSDK void	FogOff							( void );
DARKSDK void	SetFogColor						( DWORD dwColor );
DARKSDK void	SetFogColorEx					( int iR, int iG, int iB );
DARKSDK void	SetFogColorEx					( int iR, int iG, int iB, int iA );
DARKSDK void	SetFogDistance					( int iDistance );
DARKSDK void	SetFogDistance					( int iStartDistance, int iFinishDistance );

DARKSDK void	SetPositionVector3				( int iID, int iVector );
DARKSDK void	GetPositionVector3				( int iVector, int iID );
DARKSDK void	SetRotationVector3				( int iID, int iVector );
DARKSDK void	GetRotationVector3				( int iVector, int iID );

DARKSDK int		GetLightExistEx						( int iID );
DARKSDK int		GetLightTypeEx						( int iID );
DARKSDK float	GetLightPositionXEx					( int iID );
DARKSDK float	GetLightPositionYEx					( int iID );
DARKSDK float	GetLightPositionZEx					( int iID );
DARKSDK float	GetLightDirectionXEx				( int iID );
DARKSDK float	GetLightDirectionYEx				( int iID );
DARKSDK float	GetLightDirectionZEx				( int iID );
DARKSDK int		GetlightVisibleEx					( int iID );
DARKSDK float	GetLightRangeEx						( int iID );

DARKSDK void	SetLightSpecular					( int iID, float fA, float fR, float fG, float fB );
DARKSDK void	SetLightAmbient						( int iID, float fA, float fR, float fG, float fB );
DARKSDK void	SetLightFalloff						( int iID, float fValue );
DARKSDK void	SetLightAttenuation0				( int iID, float fValue );
DARKSDK void	SetLightAttenuation1				( int iID, float fValue );
DARKSDK void	SetLightAttenuation2				( int iID, float fValue );

DARKSDK void	SetLightSpecularOn					( void );
DARKSDK void	SetLightSpecularOff					( void );

#ifdef DARKSDK_COMPILE
		void	dbMakeLight						( int iID );
		void	dbDeleteLight					( int iID );

		void	dbSetPointLight					( int iID, float fX, float fY, float fZ );
		void	dbSetSpotLight					( int iID, float fInner, float fOuter );
		void	dbSetDirectionalLight			( int iID, float fX, float fY, float fZ );
		void	dbSetLightRange					( int iID, float fRange );

		void	dbPositionLight					( int iID, float fX, float fY, float fZ );
		void	dbRotateLight					( int iID, float fX, float fY, float fZ );
		void	dbPointLight					( int iID, float fX, float fY, float fZ );

		void	dbHideLight						( int iID );
		void	dbShowLight						( int iID );

		void	dbSetLightToObject				( int iID, int iObjectID );
		void	dbSetLightToObjectOrientation	( int iID, int iObjectID );
		void	dbColorLight					( int iID, DWORD dwColor );
		void	dbColorLight					( int iID, int iR, int iG, int iB );

		void	dbSetNormalizationOn			( void );
		void	dbSetNormalizationOff			( void );

		void	dbSetAmbientLight				( int iPercent );
		void	dbColorAmbientLight				( DWORD dwColor );

		void	dbFogOn							( void );
		void	dbFogOff						( void );
		void	dbFogColor						( DWORD dwColor );
		void	dbFogColor						( int iR, int iG, int iB );
		void	dbFogDistance					( int iDistance );
		void	dbFogDistance					( int iStartDistance, int iFinishDistance );

		void	dbPositionLight					( int iID, int iVector );
		void	dbSetVector3ToLightPosition		( int iVector, int iID );
		void	dbRotateLight					( int iID, int iVector );
		void	dbSetVector3ToLightRotation		( int iVector, int iID );

		int		dbLightExist					( int iID );
		int		dbLightType						( int iID );
		float	dbLightPositionX				( int iID );
		float	dbLightPositionY				( int iID );
		float	dbLightPositionZ				( int iID );
		float	dbLightDirectionX				( int iID );
		float	dbLightDirectionY				( int iID );
		float	dbLightDirectionZ				( int iID );
		int		dbLightVisible					( int iID );
		float	dbLightRange					( int iID );

		void	dbSetLightSpecular				( int iID, float fA, float fR, float fG, float fB );
		void	dbSetLightAmbient				( int iID, float fA, float fR, float fG, float fB );
		void	dbSetLightFalloff				( int iID, float fValue );
		void	dbSetLightAttenuation0			( int iID, float fValue );
		void	dbSetLightAttenuation1			( int iID, float fValue );
		void	dbSetLightAttenuation2			( int iID, float fValue );

		void	dbSetLightSpecularOn			( void );
		void	dbSetLightSpecularOff			( void );

		// lee - 300706 - GDK fixes
		void	dbSetLightToObjectPosition		( int iID, int iObjectID );

#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////




#endif _CLIGHT_H_

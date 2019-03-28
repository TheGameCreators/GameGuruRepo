#ifndef _CCAMERA_H_
#define _CCAMERA_H_

#include ".\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Data\cdatac.h"
#include ".\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Camera\CCameraDatac.h"
#include ".\..\Dark Basic Public Shared\Dark Basic Pro SDK\Shared\Camera\CCameraManagerc.h"

#define DBPRO_GLOBAL 

DARKSDK void  		CAMERAUpdate 					( void );
void  				CameraConstructor				( void );
DARKSDK void  		CameraDestructor  				( void );
DARKSDK void  		CameraSetErrorHandler			( LPVOID pErrorHandlerPtr );
DARKSDK void  		CameraRefreshGRAFIX 				( int iMode );

DARKSDK IGGTexture* GetStereoscopicFinalTexture ( void ); // U70 - 180608 - WOW autostereo

DARKSDK bool 		UpdateCameraPtr      			( int iID );
DARKSDK void 		CameraInternalUpdate			( int iID );

DARKSDK void  		StartScene						( void );
DARKSDK void  		StartSceneEx					( int );
DARKSDK int   		FinishScene						( void );
DARKSDK int			FinishSceneEx					( bool bKnowInAdvanceCameraIsUsed );
DARKSDK int	  		GetRenderCamera					( void );
DARKSDK void  		RunCode							( int );

DARKSDK void  		RotateCamera					( int iID, float fX, float fY, float fZ );
DARKSDK void  		XRotateCamera   						( int iID, float fX );
DARKSDK void  		YRotateCamera  						( int iID, float fY );
DARKSDK void  		ZRotateCamera						( int iID, float fZ );

DARKSDK float 		CameraPositionX 					( int iID );
DARKSDK float 		CameraPositionY 					( int iID );
DARKSDK float 		CameraPositionZ 					( int iID );
DARKSDK float 		CameraAngleX    					( int iID );
DARKSDK float 		CameraAngleY    					( int iID );
DARKSDK float 		CameraAngleZ    					( int iID );
DARKSDK float 		GetCameraLookX     					( int iID );
DARKSDK float 		GetCameraLookY     					( int iID );
DARKSDK float		GetCameraLookZ						( int iID );

DARKSDK void*		GetCameraInternalData				( int iID );
DARKSDK GGMATRIX	GetCameraMatrix						( int iID );

DARKSDK void		SetAutoCam						( float fX, float fY, float fZ, float fRadius );
DARKSDK void		CreateCamera					( int iID );
DARKSDK void		DestroyCamera							( int iID );

DARKSDK void		SetCameraMatrix					( int iID, GGMATRIX* pMatrix );
DARKSDK void		ReleaseCameraMatrix				( int iID );

// mike - 280305 - new functions to extra view and projection matrix
DARKSDK GGMATRIX GetCameraViewMatrix					( int iID );
DARKSDK GGMATRIX GetCameraProjectionMatrix				( int iID );
DARKSDK void		SetCameraProjectionMatrix				( int iID, GGMATRIX* pMatrix );

DARKSDK void		PositionCamera						( float fX, float fY, float fZ );
DARKSDK void		RotateCamera						( float fX, float fY, float fZ );
DARKSDK void		XRotateCamera						( float fX );
DARKSDK void		YRotateCamera						( float fY );
DARKSDK void		ZRotateCamera						( float fZ );
DARKSDK void		PointCamera 							( float fX, float fY, float fZ );
DARKSDK void		MoveCamera							( float fStep );
DARKSDK void		SetCameraRange 						( float fFront, float fBack );
DARKSDK void		SetCameraView 						( int iLeft, int iTop, int iRight, int iBottom );
DARKSDK void		ClearCameraView 					( DWORD dwColorValue );
DARKSDK void		SetCameraRotationXYZ 				( void );
DARKSDK void		SetCameraRotationZYX				( void );
DARKSDK void		SetCameraFOV 						( float fAngle );
DARKSDK void		SetCameraAspect 					( float fAspect );
DARKSDK void		CameraFollow 						( float fX, float fY, float fZ, float fAngle, float fDistance, float fHeight, float fSmooth, int iCollision );
DARKSDK void		SetAutoCamOn						( void );
DARKSDK void		SetAutoCamOff 						( void );
DARKSDK void		TurnCameraLeft 						( float fAngle );
DARKSDK void		TurnCameraRight 					( float fAngle );
DARKSDK void		PitchCameraUp 						( float fAngle );
DARKSDK void		PitchCameraDown 					( float fAngle );
DARKSDK void		RollCameraLeft 						( float fAngle );
DARKSDK void		RollCameraRight						( float fAngle );
DARKSDK void		SetCameraToObjectOrientation		( int iObjectID );

DARKSDK void		BackdropOn						( void ); 
DARKSDK void		BackdropOff						( void ); 
DARKSDK void		BackdropColor					( DWORD dwColor ); 
DARKSDK void		BackdropTexture					( int iImage ); 
DARKSDK void		BackdropScroll					( int iU, int iV ); 

DARKSDK float		CameraPositionX					( void );
DARKSDK float		CameraPositionY					( void );
DARKSDK float		CameraPositionZ					( void );
DARKSDK float		CameraAngleX					( void );
DARKSDK float		CameraAngleY					( void );
DARKSDK float		CameraAngleZ					( void );

DARKSDK void		CreateCamera					( int iID );
DARKSDK void		DeleteCamera					( int iID );
DARKSDK void		SetCurrentCamera				( int iID );
DARKSDK void		SetCameraToImage				( int iID, int iImage, int iWidth, int iHeight, int iGenerateCameraAlpha, DWORD dwOwnD3DFMTValue );
DARKSDK void		SetCameraToImage				( int iID, int iImage, int iWidth, int iHeight, int iGenerateCameraAlpha );
DARKSDK void		SetCameraToImage				( int iID, int iImage, int iWidth, int iHeight );
DARKSDK void		SetCameraToView					( int iID, void* pRenderTargetView, void* pDepthStencilView, DWORD dwWidth, DWORD dwHeight );
DARKSDK void		ResetCamera						( int iID );
DARKSDK void		MoveCameraLeft					( int iID, float fStep );
DARKSDK void		MoveCameraRight					( int iID, float fStep );
DARKSDK void		MoveCameraUp   					( int iID, float fStep );
DARKSDK void		MoveCameraDown					( int iID, float fStep );

DARKSDK void		ControlWithArrowKeys			( int iID, float fVelocity, float fTurnSpeed );

DARKSDK void		SetCameraPositionVector3		( int iID, int iVector );
DARKSDK void		GetCameraPositionVector3		( int iVector, int iID );
DARKSDK void		SetCameraRotationVector3		( int iID, int iVector );
DARKSDK void		GetCameraRotationVector3		( int iVector, int iID );

DARKSDK void		PositionCamera					( int iID, float fX, float fY, float fZ );
DARKSDK void		RotateCamera    				( int iID, float fX, float fY, float fZ );
DARKSDK void		XRotateCamera  					( int iID, float fX );
DARKSDK void		YRotateCamera  					( int iID, float fY );
DARKSDK void		ZRotateCamera  					( int iID, float fZ );
DARKSDK void		PointCamera    					( int iID, float fX, float fY, float fZ );
DARKSDK void		MoveCamera 						( int iID, float fStep );
DARKSDK void		SetCameraRange  				( int iID, float fFront, float fBack );
DARKSDK void		SetCameraView					( int iID, int iLeft, int iTop, int iRight, int iBottom );
DARKSDK void		ClearCameraViewEx				( int iID, DWORD dwColorValue );
DARKSDK void		ClearCameraView					( int iID, int iRed, int iGreen, int iBlue );
DARKSDK void		SetCameraRotationXYZ					( int iID );
DARKSDK void		SetCameraRotationZYX					( int iID );
DARKSDK void		SetCameraRotationZXY					( int iID );
DARKSDK void		SetCameraFOV							( int iID, float fAngle );
DARKSDK void		SetCameraAspect						( int iID, float fAspect );
DARKSDK void		SetCameraClip							( int iID, int iOnOff, float fX, float fY, float fZ, float fNX, float fNY, float fNZ );
DARKSDK void		CameraFollow 							( int iID, float fX, float fY, float fZ, float fAngle, float fDistance, float fHeight, float fSmooth, int iCollision );
DARKSDK void		TurnCameraLeft  						( int iID, float fAngle );
DARKSDK void		TurnCameraRight 						( int iID, float fAngle );
DARKSDK void		PitchCameraUp   						( int iID, float fAngle );
DARKSDK void		PitchCameraDown 						( int iID, float fAngle );
DARKSDK void		RollCameraLeft  						( int iID, float fAngle );
DARKSDK void		RollCameraRight						( int iID, float fAngle );
DARKSDK void		SetCameraToObjectOrientation			( int iID, int iObjectID );

DARKSDK void		BackdropOn						( int iID );
DARKSDK void		BackdropOff						( int iID );
DARKSDK void		BackdropColor					( int iID, DWORD dwColor );
DARKSDK void		BackdropTexture					( int iID, int iImage );
DARKSDK void		BackdropScroll					( int iID, int iU, int iV );

DARKSDK void		SetCameraDepth					( int iID, int iSourceID );
DARKSDK void		BackdropOff						( int iID, int iNoDepthClear );
DARKSDK void		CopyCameraToImage				( int iID, int iImage );
DARKSDK void		SetCamerasToStereoscopic		( int iStereoscopicMode, int iCameraL, int iCameraR, int iBack, int iFront );

DARKSDK int 		CameraExist	 						( int iID );
DARKSDK float		CameraPositionX						( int iID );
DARKSDK float		CameraPositionY 					( int iID );
DARKSDK float		CameraPositionZ 					( int iID );
DARKSDK float		CameraAngleX    					( int iID );
DARKSDK float		CameraAngleY    					( int iID );
DARKSDK float		CameraAngleZ    					( int iID );
DARKSDK float		GetCameraLookX 						( int iID );
DARKSDK float		GetCameraLookY 						( int iID );
DARKSDK float		GetCameraLookZ 						( int iID );

DARKSDK void		SetCameraHiRes						( bool bHiRes );

#endif _CCAMERA_H_
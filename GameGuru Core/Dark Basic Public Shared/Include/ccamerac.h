#ifndef _CCAMERA_H_
#define _CCAMERA_H_

// Includes
#include ".\..\Dark Basic Pro SDK\Shared\Data\cdatac.h"
#include ".\..\Dark Basic Pro SDK\Shared\Camera\CCameraDatac.h"
#include ".\..\Dark Basic Pro SDK\Shared\Camera\CCameraManagerc.h"

// Externs
#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
extern float custom_back_color[4];
#endif

// Core Functions

bool 		UpdateCameraPtr      				( int iID );

void  		RotateCamera						( int iID, float fX, float fY, float fZ );
void  		XRotateCamera   					( int iID, float fX );
void  		YRotateCamera  						( int iID, float fY );
void  		ZRotateCamera						( int iID, float fZ );

float 		CameraPositionX 					( int iID );
float 		CameraPositionY 					( int iID );
float 		CameraPositionZ 					( int iID );
float 		CameraAngleX    					( int iID );
float 		CameraAngleY    					( int iID );
float 		CameraAngleZ    					( int iID );
float 		GetCameraLookX     					( int iID );
float 		GetCameraLookY     					( int iID );
float		GetCameraLookZ						( int iID );

void*		GetCameraInternalData				( int iID );
GGMATRIX	GetCameraMatrix						( int iID );

void		SetAutoCam							( float fX, float fY, float fZ, float fRadius );
void		CreateCamera						( int iID );
void		DestroyCamera						( int iID );

void		SetCameraMatrix						( int iID, GGMATRIX* pMatrix );
void		ReleaseCameraMatrix					( int iID );

GGMATRIX	GetCameraViewMatrix					( int iID );
GGMATRIX	GetCameraProjectionMatrix			( int iID );
void		SetCameraProjectionMatrix			( int iID, GGMATRIX* pMatrix );

void		PositionCamera						( float fX, float fY, float fZ );
void		RotateCamera						( float fX, float fY, float fZ );
void		XRotateCamera						( float fX );
void		YRotateCamera						( float fY );
void		ZRotateCamera						( float fZ );
void		PointCamera 						( float fX, float fY, float fZ );
void		MoveCamera							( float fStep );
void		SetCameraRange 						( float fFront, float fBack );
void		SetCameraView 						( int iLeft, int iTop, int iRight, int iBottom );
void		ClearCameraView 					( DWORD dwColorValue );
void		SetCameraRotationXYZ 				( void );
void		SetCameraRotationZYX				( void );
void		SetCameraFOV 						( float fAngle );
void		SetCameraAspect 					( float fAspect );
void		CameraFollow 						( float fX, float fY, float fZ, float fAngle, float fDistance, float fHeight, float fSmooth, int iCollision );
void		SetAutoCamOn						( void );
void		SetAutoCamOff 						( void );
void		TurnCameraLeft 						( float fAngle );
void		TurnCameraRight 					( float fAngle );
void		PitchCameraUp 						( float fAngle );
void		PitchCameraDown 					( float fAngle );
void		RollCameraLeft 						( float fAngle );
void		RollCameraRight						( float fAngle );
void		SetCameraToObjectOrientation		( int iObjectID );

void		BackdropOn							( void ); 
void		BackdropOff							( void ); 
void		BackdropColor						( DWORD dwColor ); 
void		BackdropTexture						( int iImage ); 
void		BackdropScroll						( int iU, int iV ); 

float		CameraPositionX						( void );
float		CameraPositionY						( void );
float		CameraPositionZ						( void );
float		CameraAngleX						( void );
float		CameraAngleY						( void );
float		CameraAngleZ						( void );

void		CreateCamera						( int iID );
void		DeleteCamera						( int iID );
void		SetCurrentCamera					( int iID );
void		SetCameraToImage					( int iID, int iImage, int iWidth, int iHeight, int iGenerateCameraAlpha, DWORD dwOwnD3DFMTValue );
void		SetCameraToImage					( int iID, int iImage, int iWidth, int iHeight, int iGenerateCameraAlpha );
void		SetCameraToImage					( int iID, int iImage, int iWidth, int iHeight );
void		SetCameraToView						( int iID, void* pRenderTargetView, void* pDepthStencilView, DWORD dwWidth, DWORD dwHeight, void* pLeftShaderResourceView );
void		ResetCamera							( int iID );
void		MoveCameraLeft						( int iID, float fStep );
void		MoveCameraRight						( int iID, float fStep );
void		MoveCameraUp   						( int iID, float fStep );
void		MoveCameraDown						( int iID, float fStep );

void		ControlWithArrowKeys				( int iID, float fVelocity, float fTurnSpeed );

void		SetCameraPositionVector3			( int iID, int iVector );
void		GetCameraPositionVector3			( int iVector, int iID );
void		SetCameraRotationVector3			( int iID, int iVector );
void		GetCameraRotationVector3			( int iVector, int iID );

void		PositionCamera						( int iID, float fX, float fY, float fZ );
void		RotateCamera    					( int iID, float fX, float fY, float fZ );
void		XRotateCamera  						( int iID, float fX );
void		YRotateCamera  						( int iID, float fY );
void		ZRotateCamera  						( int iID, float fZ );
void		PointCamera    						( int iID, float fX, float fY, float fZ );
void		MoveCamera 							( int iID, float fStep );
void		SetCameraRange  					( int iID, float fFront, float fBack);
void		SetCameraView						( int iID, int iLeft, int iTop, int iRight, int iBottom );
void		ClearCameraViewEx					( int iID, DWORD dwColorValue );
void		ClearCameraView						( int iID, int iRed, int iGreen, int iBlue );
void		SetCameraRotationXYZ				( int iID );
void		SetCameraRotationZYX				( int iID );
void		SetCameraRotationZXY				( int iID );
void		SetCameraFOV						( int iID, float fAngle );
void		SetCameraAspect						( int iID, float fAspect );
void		SetCameraClip						( int iID, int iOnOff, float fX, float fY, float fZ, float fNX, float fNY, float fNZ );
void		CameraFollow 						( int iID, float fX, float fY, float fZ, float fAngle, float fDistance, float fHeight, float fSmooth, int iCollision );
void		TurnCameraLeft  					( int iID, float fAngle );
void		TurnCameraRight 					( int iID, float fAngle );
void		PitchCameraUp   					( int iID, float fAngle );
void		PitchCameraDown 					( int iID, float fAngle );
void		RollCameraLeft  					( int iID, float fAngle );
void		RollCameraRight						( int iID, float fAngle );
void		SetCameraToObjectOrientation		( int iID, int iObjectID );

void		BackdropOn							( int iID );
void		BackdropOff							( int iID );
void		BackdropColor						( int iID, DWORD dwColor );
void		BackdropTexture						( int iID, int iImage );
void		BackdropScroll						( int iID, int iU, int iV );

void		SetCameraDepth						( int iID, int iSourceID );
void		BackdropOff							( int iID, int iNoDepthClear );
void		CopyCameraToImage					( int iID, int iImage );
void		SetCamerasToStereoscopic			( int iStereoscopicMode, int iCameraL, int iCameraR, int iBack, int iFront );

int 		CameraExist	 						( int iID );
float		CameraPositionX						( int iID );
float		CameraPositionY 					( int iID );
float		CameraPositionZ 					( int iID );
float		CameraAngleX    					( int iID );
float		CameraAngleY    					( int iID );
float		CameraAngleZ    					( int iID );
float		GetCameraLookX 						( int iID );
float		GetCameraLookY 						( int iID );
float		GetCameraLookZ 						( int iID );

void		SetCameraHiRes						( bool bHiRes );

// Graphics Functions

void		FreeCameraSurfaces					(void);
void 		CameraInternalUpdate				( int iID );
void  		StartScene							( void );
void  		StartSceneEx						( int, bool bSpecialQuickVRRendering );
int   		FinishScene							( void );
int			FinishSceneEx						( bool bKnowInAdvanceCameraIsUsed, bool bSpecialQuickVRRendering );
int	  		GetRenderCamera						( void );
void  		RunCode								( int );
void  		CAMERAUpdate 						( void );
void  		CameraConstructor					( void );
void  		CameraDestructor  					( void );
void  		CameraSetErrorHandler				( LPVOID pErrorHandlerPtr );
void  		CameraRefreshGRAFIX 				( int iMode );

#endif _CCAMERA_H_
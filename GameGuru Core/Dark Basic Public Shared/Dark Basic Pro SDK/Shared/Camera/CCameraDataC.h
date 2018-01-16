#ifndef _CCAMERADATA_H_
#define _CCAMERADATA_H_

#include "directx-macros.h"
#pragma comment ( lib, "gdi32.lib" )

struct tagCameraData
{
	GGMATRIX 			matProjection;	// projection matrix
	GGMATRIX 			matView;		// view matrix

	GGVECTOR3			vecLook;		// look vector
	GGVECTOR3			vecUp;			// up vector
	GGVECTOR3			vecRight;		// right vector
	GGVECTOR3			vecPosition;	// position vector

	GGVIEWPORT			viewPort2D;		// view port for 2D ( area on screen to take up )
	GGVIEWPORT			viewPort3D;		// view port for 3D ( area on screen to take up )

	int					iCameraToImage;
	LPGGTEXTURE			pCameraToImageTexture;
	LPGGTEXTURE			pCameraToImageTextureLowRes;
	LPGGSURFACE			pCameraToImageSurface;
	LPGGSURFACE			pCameraToImageSurfaceHiRes;
	LPGGSURFACE			pCameraToImageSurfaceLowRes;
	LPGGRENDERTARGETVIEW pCameraToImageSurfaceView;
	LPGGTEXTURE			pCameraToImageAlphaTexture;
	LPGGSURFACE			pCameraToImageAlphaSurface;
	LPGGSURFACE			pImageDepthSurface;
	LPGGDEPTHSTENCILVIEW pImageDepthSurfaceView;
	LPGGSHADERRESOURCEVIEW pImageDepthResourceView;

	// Stereoscopic mode for camera images
	int					iStereoscopicMode;
	tagCameraData*		pStereoscopicFirstCamera;
	int					iCameraToStereoImageBack;
	int					iCameraToStereoImageFront;
	LPGGTEXTURE			pCameraToStereoImageBackTexture;
	LPGGTEXTURE			pCameraToStereoImageFrontTexture;
	LPGGSURFACE			pCameraToStereoImageBackSurface;
	LPGGSURFACE			pCameraToStereoImageFrontSurface;	

	int					iBackdropState;	// used as an automatic clear
	DWORD				dwBackdropColor;// color of backdrop

	// Can use XYZ rotation or FreeFlight(YPR)
	bool				bUseFreeFlightRotation;
	GGMATRIX 			matFreeFlightRotate;
	float				fXRotate, fYRotate, fZRotate;	// rotateangle

	float				fAspect;		// aspect ratio
	float				fAspectMod;
	float				fFOV;			// field of view
	float				fZNear;			// z near
	float				fZFar;			// z far

	bool				bRotate;		// which rotate to use

	// Override camera
	bool				bOverride;
	GGMATRIX 			matOverride;

	// Clipping planes (so can do reflection tricks)
	int					iClipPlaneOn;
	GGPLANE				planeClip;

	// Image ptr storage (DarkSKY) and also some reserves
	int					iBackdropTextureMode;
	LPGGTEXTURE			pBackdropTexture;

	// U75 - 070410 - (used RES1) stores the third parameter to COLOR BACKDROP (special FF foreground color to override textures/effects)
	DWORD				dwForegroundColor;

	// reserved members
	DWORD				dwCameraSwitchBank;
	DWORD				dwRes3;
	DWORD				dwRes4;
	DWORD				dwRes5;

	tagCameraData ( );
};

#endif _CCAMERADATA_H_

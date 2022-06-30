//
// Camera Graphics (DB Engine)
//

// Includes
#include "ccamerac.h"
#include ".\..\DBOFormat\DBOFormat.h"
#include "CGfxC.h"
#include "CImageC.h"
#include "cVectorC.h"
#include "CObjectsC.h"

// Externs
extern tagCameraData*					m_ptr;
extern int								m_iCurrentCamera;
extern bool								m_bActivateBackdrop;
extern CCameraManager					m_CameraManager;
extern bool								g_bCameraOutputToImage;
extern int								m_iRenderCamera;
extern bool								g_VR920AdapterAvailable;
extern LPGGIMMEDIATECONTEXT				m_pImmediateContext;
extern tagCameraData*					g_pStereoscopicCameraUpdated;
extern int								g_iCameraHasClipPlane;

// Graphics Functions

void CameraConstructorD3D ( HINSTANCE hSetup, HINSTANCE hImage )
{
	// clear the data pointer to null
	m_ptr = NULL;

	// create a default camera
	m_iCurrentCamera=0;
	CreateCamera ( m_iCurrentCamera );

	// set the glob struct camera ID to zero
	if ( g_pGlob ) g_pGlob->dwCurrentSetCameraID = 0;

	// ensure backdrop activates when use autocam a fresh
	m_bActivateBackdrop = true; 
}

void CameraConstructor ( void )
{
	CameraConstructorD3D ( NULL, NULL );
}

void FreeCameraSurfaces ( void )
{
	// free image surface from camera first - using m_ptr
	if(m_ptr->pImageDepthResourceView)
	{
		m_ptr->pImageDepthResourceView->Release();
		m_ptr->pImageDepthResourceView=NULL;
	}
	if(m_ptr->pImageDepthSurfaceView)
	{
		m_ptr->pImageDepthSurfaceView->Release();
		m_ptr->pImageDepthSurfaceView=NULL;
	}
	if(m_ptr->pImageDepthSurface)
	{
		m_ptr->pImageDepthSurface->Release();
		m_ptr->pImageDepthSurface=NULL;
	}
	if(m_ptr->pCameraToImageSurfaceView)
	{
		m_ptr->pCameraToImageSurfaceView->Release();
		m_ptr->pCameraToImageSurfaceView=NULL;
	}
	if(m_ptr->pCameraToImageSurface)
	{
		m_ptr->pCameraToImageSurface->Release();
		m_ptr->pCameraToImageSurface=NULL;
	}
	if ( m_ptr->pCameraToImageAlphaTexture )
	{
		if(m_ptr->pCameraToImageTexture)
		{
			m_ptr->pCameraToImageTexture->Release();
			m_ptr->pCameraToImageTexture=NULL;
		}
		if(m_ptr->pCameraToImageAlphaSurface)
		{
			m_ptr->pCameraToImageAlphaSurface->Release();
			m_ptr->pCameraToImageAlphaSurface=NULL;
		}
	}

	// free textures and surfaces of any stereoscopics
	if(m_ptr->pCameraToStereoImageBackSurface)
	{
		m_ptr->pCameraToStereoImageBackSurface->Release();
		m_ptr->pCameraToStereoImageBackSurface=NULL;
	}
	if(m_ptr->pCameraToStereoImageFrontSurface)
	{
		m_ptr->pCameraToStereoImageFrontSurface->Release();
		m_ptr->pCameraToStereoImageFrontSurface=NULL;
	}
}

void CameraDestructorD3D ( void )
{
	for (int iID = m_CameraManager.GetNextID( -1 ); iID != -1; iID = m_CameraManager.GetNextID( iID ))
	{
		tagCameraData* ptr = m_CameraManager.GetData( iID );
		if (ptr)
		{
			m_ptr = ptr;
			FreeCameraSurfaces();
		}
	}
}

void CameraDestructor ( void )
{
	CameraDestructorD3D();
}

void CameraSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

void CameraRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		CameraDestructorD3D();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		CameraConstructor();
		CameraConstructorD3D ( g_pGlob->g_GFX, g_pGlob->g_Image );
	}
}

void SetViewportForSurfaceDX9 ( GGVIEWPORT* pvp, LPGGSURFACE pSurface )
{
	// view port should never be better than target surface
	#ifndef DX11
	D3DSURFACE_DESC ddsd;
	pSurface->GetDesc ( &ddsd );
	pvp->Width=ddsd.Width;
	pvp->Height=ddsd.Height;
	// set view port for view
	m_pD3D->SetViewport ( (GGVIEWPORT*)pvp );
	#endif
}

void SetViewportForSurfaceDX11 ( int iCameraID, GGVIEWPORT* pvp, LPGGSURFACE pSurface )
{
	#ifdef DX11
	// interferes with setting a viewport smaller than surface area
	// but needed to get 64x64 thumb created (squash viewport to surface size)
	//D3D11_TEXTURE2D_DESC ddsd; // moved below
	//pSurface->GetDesc ( &ddsd );
	//pvp->Width=ddsd.Width;
	//pvp->Height=ddsd.Height;
    D3D11_VIEWPORT vp;
    vp.TopLeftX = pvp->X;
    vp.TopLeftY = pvp->Y;
    vp.Width = (FLOAT)pvp->Width;
    vp.Height = (FLOAT)pvp->Height;
    vp.MinDepth = pvp->MinZ;
    vp.MaxDepth = pvp->MaxZ;
    //m_pImmediateContext->RSSetViewports( 1, &vp );
	SetupSetViewport ( iCameraID, &vp, pSurface );
	#endif
}

void SetViewportForSurface ( int iCameraID, GGVIEWPORT* pvp, LPGGSURFACE pSurface )
{
	#ifdef DX11
	SetViewportForSurfaceDX11 ( iCameraID, pvp, pSurface );
	#else
	SetViewportForSurfaceDX9 ( pvp, pSurface );
	#endif
}

bool CameraToImage ( void )
{
	if ( m_ptr->iCameraToImage > 0 || m_ptr->iCameraToImage == -2 )
	{
		// determine if camera goes straight to cubemap face
		int iCubeMapFace=-1;
		LPGGCUBETEXTURE pCubeMapRef = NULL;
		if ( GetCubeFace!=NULL )
		{
			GetCubeFace ( m_ptr->iCameraToImage, &pCubeMapRef, &iCubeMapFace );
			if ( pCubeMapRef )
			{
				// camera output is direct to cube map
				#ifdef DX11
				#else
				LPGGSURFACE pFace;
				pCubeMapRef->GetCubeMapSurface( (D3DCUBEMAP_FACES)iCubeMapFace, 0, &pFace );
				if ( pFace )
				{
					// set redner targets
					m_pD3D->SetRenderTarget ( 0, pFace );
					m_pD3D->SetDepthStencilSurface ( m_pImageDepthSurface );
				}
				#endif
			}
			else
			{
				// camera output is to image(texture surface)
				///m_pD3D->SetRenderTarget ( 0, m_ptr->pCameraToImageSurface );
				///m_pD3D->SetDepthStencilSurface ( m_pImageDepthSurface );
				SetRenderAndDepthTarget ( m_ptr->pCameraToImageSurfaceView, m_ptr->pImageDepthSurfaceView );

				// if present, copy alpha surface 
				///if ( m_ptr->pCameraToImageAlphaSurface )
				///{
				///	// copy surface to alpha surface
				///	HRESULT hRes = D3DXLoadSurfaceFromSurface(m_ptr->pCameraToImageAlphaSurface, NULL, NULL, m_ptr->pCameraToImageSurface, NULL, NULL, D3DX_FILTER_NONE, 0xFF000000 );
				///}
			}
		}
		g_bCameraOutputToImage=true;

		// U69 - 180508 - if this camera is the right camera of a stereoscopic render, trigger the convert after this camera render
		if ( m_ptr->iStereoscopicMode > 0 )
			g_pStereoscopicCameraUpdated = m_ptr;

		// camera is image
		return false;
	}

	// camera is not image
	return true;
}

void HandleClippingPlane ( void )
{
	// leeadd - 310506 - u62 - optional camera clipping operation
	// called before camera (m_ptr) renders
	if ( m_ptr->iClipPlaneOn!=0 )
	{
		if ( g_iCameraHasClipPlane!=m_ptr->iClipPlaneOn )
		{
			// leeadd - 161008 - u70 - when using clippping with shaders (HLSL)
			// they should be in clip space, not world space
			bool bClipSpaceRequired = false;
			int iActualClipPlaneOn = m_ptr->iClipPlaneOn;
			if ( m_ptr->iClipPlaneOn==3 ) { bClipSpaceRequired=true; iActualClipPlaneOn=1; }
			if ( m_ptr->iClipPlaneOn==4 ) { bClipSpaceRequired=true; iActualClipPlaneOn=2; }

			// transform world to clip space
			GGPLANE planeUse = m_ptr->planeClip;
			if ( bClipSpaceRequired )
			{
				// temp planes for conversion
				GGPLANE tempPlane = planeUse;
				GGPLANE viewSpacePlane;

				// normalize the plane which is required for the transforms
				GGPlaneNormalize(&tempPlane, &tempPlane);

				// transform the plane into view space
				GGMATRIX tempMatrix = m_ptr->matView;
				GGMatrixInverse(&tempMatrix, NULL, &tempMatrix);
				GGMatrixTranspose(&tempMatrix, &tempMatrix);
				GGPlaneTransform(&viewSpacePlane, &tempPlane, &tempMatrix);

				// transform the plane into clip space, or post projection space
				tempMatrix = m_ptr->matProjection;
				GGMatrixInverse(&tempMatrix, NULL, &tempMatrix);
				GGMatrixTranspose(&tempMatrix, &tempMatrix);

				// place resulting clip space plane ready for setclipplane
				GGPlaneTransform(&planeUse, &viewSpacePlane, &tempMatrix);
			}

			// clipping plane enabled
			#ifdef DX11
			#else
			m_pD3D->SetClipPlane ( 0, (float*)planeUse );
			m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 );
			#endif

			// flag controls low-state hits
			g_iCameraHasClipPlane=m_ptr->iClipPlaneOn;
		}
	}
	else
	{
		if ( g_iCameraHasClipPlane>0 )
		{
			#ifdef DX11
			#else
			m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
			#endif
			g_iCameraHasClipPlane=0;
		}
	}
}

void CAMERAUpdate ( void )
{
	// 20120313 IanM - This code taken from FinishSceneEx.
	// Core may determine that eg camera 1 is not in the mask and instruct FinishSceneEx
	// that the 'next' camera will not be rendered too, except that if camera 1 doesn't
	// exist then the next camera will then mistakenly not be rendered.
	// m_iRenderCamera and mptr have been updated by the FinishSceneEx function,
	// so no need to do so again.
	// This function is now called as the first item in the renderlist in core, so
	// will prep everything ready for rendering.
	if ( CameraToImage () )
	{
		// restore camera output to current bitmap
		#ifdef DX11
		// THIS interferes when SetCurrentBitmap(32) to set render target view, but camera zero resets when this is called
		// so leaving this in but adding pCurrentBitmapSurfaceView setting in SetCurrentBitmap to compensate!
		//SetRenderAndDepthTarget ( g_pGlob->pCurrentBitmapSurfaceView, g_pGlob->pHoldDepthBufferPtr );
		SetRenderAndDepthTarget ( g_pGlob->pCurrentBitmapSurfaceView, g_pGlob->pCurrentBitmapDepthView );
		#else
		m_pD3D->SetRenderTarget ( 0, g_pGlob->pCurrentBitmapSurface );
		m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
		#endif
		g_bCameraOutputToImage=false;
	}

	// Update Camera
	CameraInternalUpdate( m_iRenderCamera );

	// set view port for new camera
	GGVIEWPORT vp = m_ptr->viewPort3D;
	if ( m_ptr->iCameraToImage == -2 )
	{
		// -2 is WMR view
		SetViewportForSurface ( m_iRenderCamera, &vp, m_ptr->pCameraToImageSurface );
	}
	else
	{
		if ( m_ptr->iCameraToImage > 0 ) 
		{
			if ( m_ptr->pCameraToImageSurface ) SetViewportForSurface ( m_iRenderCamera, &vp, m_ptr->pCameraToImageSurface );
		}
		else
		{
			SetViewportForSurface ( m_iRenderCamera, &vp, g_pGlob->pCurrentBitmapSurface );
		}
	}

	// leeadd - 310506 - u62 - optional camera clipping operation
	HandleClippingPlane();

	// clear the screen
	#ifdef DX11

	// VR side by side preserved postprocess surface which second camera view (right eye) is rendered to it
	bool bClearTargetAndDepth = true;
	if ( g_VR920AdapterAvailable == true && g_pGlob->dwRenderCameraID == 3 )
		bClearTargetAndDepth = false;	
	if ( bClearTargetAndDepth == true )
	{
		if ( m_ptr->iBackdropState == 1 )
		{
			float color[4] = {0,0,0,1};
			if ( m_iRenderCamera == 0 && g_pGlob->iCurrentBitmapNumber == 0 ) { color[0] = 0.1f; color[1] = 0.1f; color[2] = 0.1f; }
			if ( m_iRenderCamera == 0 && g_pGlob->iCurrentBitmapNumber > 0  ) { color[0] = 1; color[1] = 0.5f; color[2] = 0; }
			if ( m_iRenderCamera == 1 ) { color[0] = 0; color[1] = 1; color[2] = 0; }
			if ( m_iRenderCamera == 2 ) { color[0] = 0; color[1] = 0; color[2] = 1; }
			if ( m_iRenderCamera == 3 ) { color[0] = 0; color[1] = 0.25f; color[2] = 0; }
#if defined(ENABLEIMGUI) && !defined(USEOLDIDE) 
			if(custom_back_color[3] > 0.0f )
				if (g_pGlob->pCurrentRenderView) m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, custom_back_color);
			else
				if (g_pGlob->pCurrentRenderView) m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, color);
#else
			if ( g_pGlob->pCurrentRenderView ) m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, color);
#endif
		}
		if ( g_pGlob->pCurrentDepthView ) m_pImmediateContext->ClearDepthStencilView(g_pGlob->pCurrentDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	#else
	if(m_ptr->iBackdropState==1)
		m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_ptr->dwBackdropColor, 1.0f, 0 );
	else
		m_pD3D->Clear ( 0, NULL, D3DCLEAR_ZBUFFER, m_ptr->dwBackdropColor, 1.0f, 0 );
	#endif
}

void StartSceneEx ( int iMode, bool bSpecialQuickVRRendering )
{
	// iMode : 0-regular, 1-no camera backdrop (when disable camera rendering)

	// start render chain for cameras
	m_iRenderCamera = 0;

	// update the pointer
	if ( UpdateCameraPtr ( m_iRenderCamera ) )
	{
		// camera to image handling
		CameraToImage ();

		// Update Camera
		CameraInternalUpdate( m_iRenderCamera );

		// set viewport for current rendertarget
		GGVIEWPORT vp = m_ptr->viewPort3D;
		LPGGSURFACE pDestSurface = NULL;
		if ( m_ptr->iCameraToImage > 0 )
			pDestSurface = m_ptr->pCameraToImageSurface;
		else
			pDestSurface = g_pGlob->pCurrentBitmapSurface;
		SetViewportForSurface ( m_iRenderCamera, &vp, pDestSurface );

		// optional camera clipping operation
		HandleClippingPlane();

		// clear the screen
		if ( iMode==0 )
		{
			#ifdef DX11
			if(m_ptr->iBackdropState==1)
			{
				float color[4] = {0,0,0,1};
				if ( g_pGlob->iCurrentBitmapNumber > 0 ) { color[0] = 0; color[1] = 1; }

				m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentRenderView, color);
			}
			m_pImmediateContext->ClearDepthStencilView(g_pGlob->pCurrentDepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
			#else
			if(m_ptr->iBackdropState==1)
				m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_ptr->dwBackdropColor, 1.0f, 0 );
			else
				m_pD3D->Clear ( 0, NULL, D3DCLEAR_ZBUFFER, m_ptr->dwBackdropColor, 1.0f, 0 );
			#endif
		}
	}
	else
	{
		// no camera as yet
		m_iRenderCamera=-1;
	}

	// allow globstruct to keep track of which camera doing the rendering
	if(g_pGlob) g_pGlob->dwRenderCameraID = m_iRenderCamera;
}

void StartScene ( void )
{
	// see above
	StartSceneEx ( 0, false );

	// U69 - 180508 - start a new camera render loop, reset stereoscopic update flag
	g_pStereoscopicCameraUpdated = NULL;
}

int FinishSceneEx ( bool bKnowInAdvanceCameraIsUsed, bool bSpecialQuickVRRendering )
{
	if ( bSpecialQuickVRRendering == true )
	{
		// ensure cameras 6 and 7 are always skipped (even though masked out)
		m_iRenderCamera = m_CameraManager.GetNextID ( m_iRenderCamera );
		while ( m_iRenderCamera == 6 || m_iRenderCamera == 7 )
			m_iRenderCamera = m_CameraManager.GetNextID ( m_iRenderCamera );
	}
	else
	{
		// simply take cameras sequentially
		m_iRenderCamera = m_CameraManager.GetNextID ( m_iRenderCamera );
	}

	if ( g_pGlob ) g_pGlob->dwRenderCameraID = m_iRenderCamera;
	if ( m_iRenderCamera != -1 )
	{
		// update the pointer for Next Camera
		UpdateCameraPtr ( m_iRenderCamera );

		// another camera to render
		return 0;
	}
	else
	{
		// restore camera output to current bitmap (if changed)
		if ( g_bCameraOutputToImage == true )
		{
			#ifdef DX11
			SetRenderAndDepthTarget ( g_pGlob->pCurrentBitmapSurfaceView, g_pGlob->pCurrentBitmapDepthView );
			#else
			m_pD3D->SetRenderTarget(0, g_pGlob->pCurrentBitmapSurface);
			m_pD3D->SetDepthStencilSurface(g_pGlob->pHoldDepthBufferPtr);
			#endif
			g_bCameraOutputToImage = false;
		}

		// update the pointer for first camera
		UpdateCameraPtr(0);

		// restore camera for 2D operations
		if (m_ptr)
		{
			GGVIEWPORT vp = m_ptr->viewPort2D;
			SetViewportForSurface ( m_iRenderCamera, &vp, g_pGlob->pCurrentBitmapSurface);
		}

		// leeadd - 310506 - u62 ensure clip is removed before 2D operations commence
		if ( g_iCameraHasClipPlane>0 )
		{
			#ifdef DX11
			#else
			m_pD3D->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
			#endif
			g_iCameraHasClipPlane=0;
		}

		// end of render camera chain
		return 1;
	}
}

int FinishScene ( void )
{
	// assume next camera is valid, so we set it up and clear render target
	// leenote - 130906 - u63 - discovered this would wipe out previous camera is using multiple SYNC MASK calls (post-process tricks)
	return FinishSceneEx ( true, false );
}

int GetRenderCamera ( void )
{
	return m_iRenderCamera;
}

void RunCode ( int iCodeMode )
{
	// used to run code from within Camera DLL from elsewhere
	switch ( iCodeMode )
	{
		case 0 : // restore to normal behaviour (output to main bitmap surface)
		{
			#ifdef DX11
			SetRenderAndDepthTarget ( g_pGlob->pCurrentBitmapSurfaceView, g_pGlob->pCurrentBitmapDepthView );
			#else
			m_pD3D->SetRenderTarget ( 0, g_pGlob->pCurrentBitmapSurface );
			m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
			#endif
			UpdateCameraPtr ( 0 );
			if ( m_ptr )
			{
				GGVIEWPORT vp = m_ptr->viewPort2D;
				SetViewportForSurface ( 0, &vp, g_pGlob->pCurrentBitmapSurface );
			}
		}
		break;

		case 1 : // redirect SPRITE/2D/IMAGE rendering to non-Bitmap Render Target, specified with DRAW TO CAMERA (camera zero)
		UpdateCameraPtr ( 0 );
		if ( m_ptr )
		{
			// only where camera is being rendered to an image
			GGVIEWPORT vp = m_ptr->viewPort2D;
			if ( m_ptr->pCameraToImageSurface )
			{
				#ifdef DX11
				SetRenderAndDepthTarget ( m_ptr->pCameraToImageSurfaceView, m_ptr->pImageDepthSurfaceView );///g_pGlob->pHoldDepthBufferPtr );
				#else
				m_pD3D->SetRenderTarget ( 0, m_ptr->pCameraToImageSurface );
				m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
				#endif
				SetViewportForSurface ( 0, &vp, m_ptr->pCameraToImageSurface );
			}
		}
		break;
	}
}

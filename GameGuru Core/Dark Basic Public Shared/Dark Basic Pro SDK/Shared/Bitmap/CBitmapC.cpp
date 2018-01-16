#include "cbitmapc.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"
#include ".\..\camera\ccameradatac.h"
#include ".\..\Core\SteamCheckForWorkshop.h"
#include "CCameraC.h"
#include "CGfxC.h"
#include "CImageC.h"

extern int GetBitDepthFromFormat(GGFORMAT Format);

#include <map>

extern GlobStruct*				g_pGlob;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
extern LPGGSURFACE				g_pBackBuffer;
extern LPGG						m_pDX;

namespace
{
    typedef std::map<int, tagCamData*>	BitmapList_t;
    typedef BitmapList_t::iterator	BitmapPtr;

    BitmapList_t					m_List;
    int								m_iWidth					= 0;
    int								m_iHeight					= 0;
    int								m_iMipMapNum				= 1;
    int								m_iMemory					= GGPOOL_MANAGED;
    bool							m_bSharing					= true;
    bool							m_bMipMap					= true;
    GGCOLOR							m_Color						= GGCOLOR_ARGB ( 255, 0, 0, 0 );
    int                             m_CurrentId                 = -1;
    tagCamData*						m_camptr					= NULL;
    GGFORMAT						g_CommonSurfaceFormat		= GGFMT_A8R8G8B8;
    DWORD							g_CommonSurfaceDepth;
    DWORD							g_CSBPP;
    bool							g_bOffscreenBitmap			= false;
    bool							g_bSupressErrorMessage		= false;

    bool RemoveBitmap( int iID )
    {
        // Clear the cached value if the bitmap being deleted is the current cached bitmap.
        if (m_CurrentId == iID)
        {
            m_CurrentId = -1;
            m_camptr = NULL;
        }

        // Locate the bitmap, and if found, release all of it's resources.
        BitmapPtr pBitmap = m_List.find( iID );
        if (pBitmap != m_List.end())
        {
            SAFE_RELEASE( pBitmap->second->lpSurface );
            SAFE_RELEASE( pBitmap->second->lpDepth );
            delete pBitmap->second;
            m_List.erase(pBitmap);
            return true;
        }
        return false;
    }

    bool BitmapUpdatePtr ( int iID )
    {
        // If the bitmap required is not already cached, refresh the cached value
        if (!m_camptr || iID != m_CurrentId)
        {
            m_CurrentId = iID;

            BitmapPtr p = m_List.find( iID );
            if (p == m_List.end())
            {
                m_camptr = NULL;
            }
            else
            {
                m_camptr = p->second;
            }
        }
        return m_camptr != NULL;
    }
}

#ifdef DX11

DARKSDK void UpdateBitmapZeroOfNewBackbuffer( void )
{
	// get screen dimensions from glob
	int iWidth = g_pGlob->iScreenWidth;
	int iHeight = g_pGlob->iScreenHeight;

	/*
	// remove old bitmap zero object
	if ( BitmapUpdatePtr ( 0 ) )
        RemoveBitmap(0);

	// use backbuffer to determine common GGFORMAT
	#ifdef DX11
	int iDepthValue = GetBackBufferDepthDX11 ( (DWORD*)&g_CommonSurfaceFormat );
	#else
	int iDepthValue = GetBackBufferDepthDX9 ( (DWORD*)&g_CommonSurfaceFormat );
	#endif

	// surface depth and bytecount
	g_CommonSurfaceDepth = iDepthValue;
	g_CSBPP = iDepthValue/8;

	// make new bitmap zero
	g_bOffscreenBitmap=false;
	MakeBitmap ( 0, iWidth, iHeight );

	// remove surface from it (specified on the fly)
	BitmapUpdatePtr ( 0 );
	if(m_camptr)
	{
		// Not surface created
		SAFE_RELEASE(m_camptr->lpSurface);
		SAFE_RELEASE(m_camptr->lpDepth);

		// Actual backbuffer
		#ifdef DX11
		m_camptr->iDepth = GetBackBufferDepthDX11 ( NULL );
		#else
		m_camptr->iDepth = GetBackBufferDepthDX9 ( NULL );
		#endif
	}
	*/
}

DARKSDK void BitmapConstructor ( )
{
	// Use refresh to recreate bitmap zero
	UpdateBitmapZeroOfNewBackbuffer();

	// Start with bitmap zero target
	SetCurrentBitmap(0);
}

DARKSDK void BitmapDestructor ( void )
{
    m_CurrentId = -1;
    m_camptr = NULL;

    for (BitmapPtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
    {
        // Release the texture and texture name
        tagCamData* ptr = pCheck->second;
		SAFE_RELEASE ( ptr->lpSurface );
		SAFE_RELEASE ( ptr->lpDepth );

        // Release the rest of the image storage
        delete ptr;

        // NOTE: Not removing from m_List at this point:
        // 1 - it makes moving to the next item harder
        // 2 - it's less efficient - we'll clear the entire list at the end
    }

    // Now clear the list
    m_List.clear();
}

DARKSDK void BitmapRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		BitmapDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		BitmapConstructor();
	}
}

DARKSDK GGFORMAT GetValidStencilBufferFormat ( GGFORMAT BackBufferFormat )
{
	GGFORMAT DepthFormat;
	#ifdef DX11
	DepthFormat = GGFMT_D24S8;
	#else
	// create the list in order of precedence
	GGFORMAT list [ ] =
							{
								GGFMT_D24S8, //GeForce4 top choice
								GGFMT_R8G8B8,
								GGFMT_A8R8G8B8,
								GGFMT_X8R8G8B8,
								GGFMT_R5G6B5,
								GGFMT_X1R5G5B5,
								GGFMT_A1R5G5B5,
								GGFMT_A4R4G4B4,
								GGFMT_R3G3B2,
								GGFMT_A8,
								GGFMT_A8R3G3B2,
								GGFMT_X4R4G4B4,
								GGFMT_A8P8,
								GGFMT_P8,
								GGFMT_L8,
								GGFMT_A8L8,
								GGFMT_A4L4,
								GGFMT_V8U8,
								GGFMT_L6V5U5,
								GGFMT_X8L8V8U8,
								GGFMT_Q8W8V8U8,
								GGFMT_V16U16,
								GGFMT_D16_LOCKABLE,
								GGFMT_D32,
								GGFMT_D15S1,
								GGFMT_D16,
								GGFMT_D24X8,
								GGFMT_D24X4S4,
							};

	for ( int iTemp = 0; iTemp < 29; iTemp++ )
	{
		// Verify that the depth format exists first
		if ( SUCCEEDED ( m_pDX->CheckDeviceFormat(  GGADAPTER_DEFAULT,
													D3DDEVTYPE_HAL,
													BackBufferFormat,
													D3DUSAGE_DEPTHSTENCIL,
													D3DRTYPE_SURFACE,
													list [ iTemp ]						) ) )
		{
			if ( SUCCEEDED ( m_pDX->CheckDepthStencilMatch	(	GGADAPTER_DEFAULT,
																D3DDEVTYPE_HAL,
																BackBufferFormat,
																BackBufferFormat,
																list [ iTemp ]				) ) )
			{
				DepthFormat = list [ iTemp ];
				break;
			}
		}
	}
	#endif
	return DepthFormat;
}

DARKSDK LPGGSURFACE MakeFormat ( int iID, int iWidth, int iHeight, GGFORMAT format )
{
	#ifdef DX11
	// make a new image

	// variable declarations
	tagCamData*	test = NULL;		// pointer to data structure
	HRESULT		hr;					// used for error checking
	
	// create a new block of memory
	test = new tagCamData;
	memset(test, 0, sizeof(tagCamData));

	// check the memory was created
	if ( test == NULL )
		return NULL;

	// clear out the memory
	memset ( test, 0, sizeof ( test ) );

	// video or system bitmap
	if ( g_bOffscreenBitmap )
	{
		// create system surface for bitmap
		D3D11_TEXTURE2D_DESC StagedOffScreenSurfaceDesc = { iWidth, iHeight, 1, 1, g_CommonSurfaceFormat, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0 };
		hr = m_pD3D->CreateTexture2D( &StagedOffScreenSurfaceDesc, NULL, &test->lpSurface );
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false ) Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// no depth buffer for system bitmaps
		test->lpDepth = NULL;
	}
	else
	{
		// create video surface for bitmap
		test->lpSurfaceView = NULL;
		GGSURFACE_DESC bufferDesc;
		bufferDesc.ArraySize = 1;
		bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.Format = g_CommonSurfaceFormat;
		bufferDesc.Height = iHeight;
		bufferDesc.MipLevels = 1;
		bufferDesc.MiscFlags = 0;
		bufferDesc.SampleDesc = {1,0};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.Width = iWidth;
		HRESULT hr = m_pD3D->CreateTexture2D(&bufferDesc, 0, &test->lpSurface);
		if ( hr == S_OK )
		{
			//Creating a view of the texture to be used when binding it as a render target
			//D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			//renderTargetViewDesc.Format = g_CommonSurfaceFormat;
			//renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			//renderTargetViewDesc.Texture2D.MipSlice = 0;
			hr = m_pD3D->CreateRenderTargetView(test->lpSurface, 0, &test->lpSurfaceView);
		}
		//Creating a view of the texture to be used when binding it on a shader to sample
		//ID3D11ShaderResourceView* _ShaderResourceView = NULL;
		//D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		//shaderResourceViewDesc.Format = format;
		//shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		//shaderResourceViewDesc.Texture2D.MipLevels = 1;
		//hr = Device->CreateShaderResourceView(_Texture2D, &shaderResourceViewDesc, &_ShaderResourceView);
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false ) Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// create depth surface
		if ( test->lpDepth==NULL )
		{
			GGFORMAT depthFormat = GetValidStencilBufferFormat(g_CommonSurfaceFormat);
			test->lpDepthView = NULL;
			GGSURFACE_DESC bufferDesc;
			bufferDesc.ArraySize = 1;
			bufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.Format = depthFormat;
			bufferDesc.Height = iHeight;
			bufferDesc.MipLevels = 1;
			bufferDesc.MiscFlags = 0;
			bufferDesc.SampleDesc = {1,0};
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.Width = iWidth;
			HRESULT hr = m_pD3D->CreateTexture2D(&bufferDesc, 0, &test->lpDepth);
			if ( hr == S_OK )
			{
				//D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
				//depthStencilViewDesc.Format = depthFormat;
				//depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				//depthStencilViewDesc.Texture2D.MipSlice = 0;
				hr = m_pD3D->CreateDepthStencilView(test->lpDepth, 0, &test->lpDepthView);
			}
		}
	}

	// setup properties
	test->iWidth  = iWidth;					// store the height
	test->iHeight = iHeight;				// store the width
	test->iDepth = g_CommonSurfaceDepth;	// store the width
	test->bLocked = false;					// set locked to false
	test->iMirrored  = 0;		
	test->iFlipped  = 0;	
    m_List.insert( std::make_pair(iID, test) );

	// return texture ptr
	return test->lpSurface;
	#else
	// make a new image

	// variable declarations
	tagCamData*	test = NULL;		// pointer to data structure
	HRESULT		hr;					// used for error checking
	
	// create a new block of memory
	test = new tagCamData;
	memset(test, 0, sizeof(tagCamData));

	// check the memory was created
	if ( test == NULL )
		return NULL;

	// clear out the memory
	memset ( test, 0, sizeof ( test ) );

	// video or system bitmap
	if ( g_bOffscreenBitmap )
	{
		// create system surface for bitmap
		// cannot draw to system bitmaps (not a render target, and vid bitmaps are SLOW)
		hr = m_pD3D->CreateOffscreenPlainSurface( iWidth, iHeight, g_CommonSurfaceFormat, D3DPOOL_SYSTEMMEM, &test->lpSurface, NULL);
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false )
				Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// no depth buffer for system bitmaps
		test->lpDepth = NULL;
	}
	else
	{
		// create video surface for bitmap
		hr = m_pD3D->CreateRenderTarget( iWidth, iHeight, g_CommonSurfaceFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &test->lpSurface, NULL);
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false )
				Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// use backbuffer to determine common GGFORMAT
		D3DSURFACE_DESC backbufferdesc;
		g_pGlob->pHoldBackBufferPtr->GetDesc(&backbufferdesc);
		GGFORMAT CommonFormat = backbufferdesc.Format;

		// create depth surface
		if ( test->lpDepth==NULL )
		{
			HRESULT hRes = m_pD3D->CreateDepthStencilSurface(	iWidth, iHeight, GetValidStencilBufferFormat(CommonFormat), D3DMULTISAMPLE_NONE, 0, TRUE,
																&test->lpDepth, NULL );
		}
	}

	// setup properties
	test->iWidth  = iWidth;					// store the height
	test->iHeight = iHeight;				// store the width
	test->iDepth = g_CommonSurfaceDepth;	// store the width
	test->bLocked = false;					// set locked to false
	test->iMirrored  = 0;		
	test->iFlipped  = 0;	
    m_List.insert( std::make_pair(iID, test) );

	// return texture ptr
	return test->lpSurface;
	#endif
}

DARKSDK LPGGSURFACE MakeBitmap ( int iID, int iWidth, int iHeight )
{
	if(BitmapUpdatePtr ( iID ))
	{
        RemoveBitmap( iID );
	}
	return MakeFormat ( iID, iWidth, iHeight, g_CommonSurfaceFormat );
}

DARKSDK void CreateBitmapEx ( int iID, int iWidth, int iHeight, int iSystemMemoryMode, int iSilentError )
{
	// 200112 - silent error mode
	if ( iSilentError==1 ) g_bSupressErrorMessage = true;

	if(iID<1 || iID>MAXIMUMVALUE)
	{
		if ( g_bSupressErrorMessage==false ) RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}

	// make new bitmap
	if ( iSystemMemoryMode==1 )
		g_bOffscreenBitmap=true;
	else
		g_bOffscreenBitmap=false;

	// create surface
	MakeBitmap ( iID, iWidth, iHeight );

	// auto set to new bitmap
	SetCurrentBitmap ( iID );

	// now clear the bitmap
	SETUPClear(0,0,0);

	// 200112 - silent error mode
	g_bSupressErrorMessage = false;
}

DARKSDK void CreateBitmapEx ( int iID, int iWidth, int iHeight, int iSystemMemoryMode )
{
	CreateBitmapEx ( iID, iWidth, iHeight, iSystemMemoryMode, 0 );
}

DARKSDK void CreateBitmap ( int iID, int iWidth, int iHeight )
{
	CreateBitmapEx ( iID, iWidth, iHeight, 0 );
}

DARKSDK void DeleteBitmapEx ( int iID )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

    RemoveBitmap( iID );

	// restore current bitmap
	SetCurrentBitmap ( 0 );
}

DARKSDK void SetCurrentBitmap ( int iID )
{
	if ( iID>=0 )
	{
		// specifies bitmap number
		if(iID<0 || iID>MAXIMUMVALUE)
		{
			RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
			return;
		}
		if ( iID > 0 ) // 220517 - bitmap zero created at start, but DX11 does not need it(?) so skip this check for zero
		{
			if ( !BitmapUpdatePtr ( iID ) )
			{
				RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
				return;
			}
		}
	}
	else
	{
		// specifies inverse camera number
	}

	// Bitmap Zero is default backbuffer
	if ( iID==0 )
	{
		// render target is backbuffer
		SetRenderAndDepthTarget ( g_pGlob->pHoldBackBufferPtr, g_pGlob->pHoldDepthBufferPtr );
		g_pGlob->pCurrentBitmapSurfaceView = g_pGlob->pHoldBackBufferPtr;
		g_pGlob->pCurrentBitmapDepthView = g_pGlob->pHoldDepthBufferPtr;
		g_pGlob->pCurrentBitmapSurface = g_pBackBuffer;
		g_pGlob->iCurrentBitmapNumber = 0;
	}
	else
	{
		if ( iID > 0 )
		{
			// render target is bitmap
			SetRenderAndDepthTarget ( m_camptr->lpSurfaceView, m_camptr->lpDepthView );
			g_pGlob->pCurrentBitmapSurfaceView = m_camptr->lpSurfaceView;
			g_pGlob->pCurrentBitmapDepthView = m_camptr->lpDepthView;
			g_pGlob->pCurrentBitmapSurface = m_camptr->lpSurface;
			g_pGlob->iCurrentBitmapNumber = iID;
		}
		else
		{
			// render target is camera surface
			int iRenderBitmapActivityToCameraID = abs(iID);
			if ( iRenderBitmapActivityToCameraID > 0 )
			{
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( iRenderBitmapActivityToCameraID );
				if ( m_Camera_Ptr )
				{
					SetRenderAndDepthTarget ( m_Camera_Ptr->pCameraToImageSurfaceView, NULL );// m_Camera_Ptr->pImageDepthSurfaceView );
					g_pGlob->pCurrentBitmapSurfaceView = m_Camera_Ptr->pCameraToImageSurfaceView;
					g_pGlob->pCurrentBitmapDepthView = NULL;
					g_pGlob->pCurrentBitmapSurface=m_Camera_Ptr->pCameraToImageSurface;
				}
				else
					iRenderBitmapActivityToCameraID = 0;
			}
			if ( iRenderBitmapActivityToCameraID==0 )
			{
				// DEFAULT back to render target is default backbuffer
				SetRenderAndDepthTarget ( g_pGlob->pHoldBackBufferPtr, g_pGlob->pHoldDepthBufferPtr );
				g_pGlob->pCurrentBitmapSurfaceView = g_pGlob->pHoldBackBufferPtr;
				g_pGlob->pCurrentBitmapDepthView = g_pGlob->pHoldDepthBufferPtr;
				g_pGlob->pCurrentBitmapSurface = g_pBackBuffer;
			}
			g_pGlob->iCurrentBitmapNumber=0;
		}
	}
}

// Data Access Functions

DARKSDK void GetBitmapData( int iID, DWORD* dwWidth, DWORD* dwHeight, DWORD* dwDepth, LPSTR* pData, DWORD* dwDataSize, bool bLockData )
{
	// Read Data
	if(bLockData==true)
	{
		if ( !BitmapUpdatePtr ( iID ) )
			return;

		/*
		// Backbuffer or Bitmap To/Dest Surface
		LPGGSURFACE pToSurface = m_camptr->lpSurface;
		if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

		// bitmap data
		*dwWidth = m_camptr->iWidth;
		*dwHeight = m_camptr->iHeight;
		*dwDepth = m_camptr->iDepth;

		// lock
		GGLOCKED_RECT d3dlock;
		if(SUCCEEDED(pToSurface->LockRect ( &d3dlock, NULL, 0 ) ) )
		{
			// create memory
			DWORD bpp = m_camptr->iDepth/8;
			DWORD dwSizeOfBitmapData = m_camptr->iWidth*m_camptr->iHeight*bpp;
			*pData = new char[dwSizeOfBitmapData];
			*dwDataSize = dwSizeOfBitmapData;

			// copy from surface
			LPSTR pSrc = (LPSTR)d3dlock.pBits;
			LPSTR pPtr = *pData;
			for(int y=0; y<m_camptr->iHeight; y++)
			{
				memcpy(pPtr, pSrc, m_camptr->iWidth*bpp);
				pPtr+=m_camptr->iWidth*bpp;
				pSrc+=d3dlock.Pitch;
			}
			pToSurface->UnlockRect();
		}
		*/
	}
	else
	{
		// free memory
		SAFE_DELETE(*pData);
	}
}

DARKSDK void SetBitmapData( int iID, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, LPSTR pData, DWORD dwDataSize )
{
	LPGGSURFACE pToSurface=NULL;
	if ( iID>0 )
	{
		/*
		if( BitmapUpdatePtr ( iID ) )
		{
			// Backbuffer or Bitmap To/Dest Surface
			pToSurface = m_camptr->lpSurface;
			if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

			// Check new bitmap specs with existing one
			if(dwWidth==(DWORD)m_camptr->iWidth && dwHeight==(DWORD)m_camptr->iHeight && dwDepth==(DWORD)m_camptr->iDepth)
			{
				// Same bitmap size
			}
			else
			{
				// Recreate Bitmap
				DeleteBitmapEx ( iID );
				m_camptr=NULL;
			}
		}
		if(m_camptr==NULL)
		{
			GGFORMAT dFormat;
			g_bOffscreenBitmap=false;
			if ( dwDepth==16 ) dFormat=GGFMT_R5G6B5;
			if ( dwDepth==24 ) dFormat=GGFMT_R8G8B8;
			if ( dwDepth==32 ) dFormat=GGFMT_X8R8G8B8;
			MakeFormat ( iID, dwWidth, dwHeight, dFormat );
		}
		*/
	}

	/*
	// Failed to create bitmap?
	if ( !BitmapUpdatePtr ( iID ) ) return;

	// Bitmap may have changed
	pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// Write Data
	GGLOCKED_RECT d3dlock;
	if(SUCCEEDED(pToSurface->LockRect ( &d3dlock, NULL, 0 ) ) )
	{
		// copy from surface
		DWORD databpp = dwDepth/8;
		LPSTR pSrc = (LPSTR)d3dlock.pBits;
		LPSTR pPtr = pData;
		for(DWORD y=0; y<dwHeight; y++)
		{
			memcpy(pSrc, pPtr, dwWidth*databpp);
			pPtr+=dwWidth*databpp;
			pSrc+=d3dlock.Pitch;
		}
		pToSurface->UnlockRect();
	}
	*/
}

DARKSDK int BitmapExist ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}

	// returns true if the bitmap exists
	if ( !BitmapUpdatePtr ( iID ) )
		return 0;

	// return true
	return 1;
}

#else

DARKSDK void BitmapConstructor ( )
{
	// Use refresh to recreate bitmap zero
	UpdateBitmapZeroOfNewBackbuffer();

	// Start with bitmap zero target
	SetCurrentBitmap(0);

}

DARKSDK void BitmapDestructor ( void )
{
    m_CurrentId = -1;
    m_camptr = NULL;

    for (BitmapPtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
    {
        // Release the texture and texture name
        tagCamData* ptr = pCheck->second;
		SAFE_RELEASE ( ptr->lpSurface );
		SAFE_RELEASE ( ptr->lpDepth );

        // Release the rest of the image storage
        delete ptr;

        // NOTE: Not removing from m_List at this point:
        // 1 - it makes moving to the next item harder
        // 2 - it's less efficient - we'll clear the entire list at the end
    }

    // Now clear the list
    m_List.clear();
}

DARKSDK void BitmapSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void BitmapPassCoreData( LPVOID pGlobPtr )
{
	// Held in Core, used here..
	g_pGlob = (GlobStruct*)pGlobPtr;
}

DARKSDK void BitmapRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		BitmapDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		BitmapConstructor();
		BitmapPassCoreData ( g_pGlob );
	}
}

DARKSDK int GetHeight ( int iID )
{
	// get the height of an image

	// update internal data
	if ( !BitmapUpdatePtr ( iID ) )
		return -1;

	// return the height
	return m_camptr->iHeight;
}

DARKSDK int GetWidth ( int iID )
{
	// get the width of an image

	// update internal data
	if ( !BitmapUpdatePtr ( iID ) )
		return -1;

	// return the width
	return m_camptr->iWidth;
}

DARKSDK void Write ( int iID, int iX, int iY, int iA, int iR, int iG, int iB )
{
	if ( !BitmapUpdatePtr ( iID ) )
		return;

	if ( !m_camptr->bLocked )
		Error ( "Unable to modify texture data for image library as it isn't locked" );
	
	DWORD* pPix = ( DWORD* ) m_camptr->d3dlr.pBits;
	
	pPix [ ( ( m_camptr->d3dlr.Pitch >> 4 ) * iX ) + iY ] = GGCOLOR_ARGB ( iA, iR, iG, iB );
}

DARKSDK void Get ( int iID, int iX, int iY, int* piR, int* piG, int* piB )
{
	// get a pixel at x, y

	// check pointers are valid
	if ( !piR || !piG || !piB )
		return;

	// update the internal data
	if ( !BitmapUpdatePtr ( iID ) )
		return;

	// check the image is locked
	if ( !m_camptr->bLocked )
		Error ( "Unable to get texture data for image library as it isn't locked" );

	// get a pointer to the data
	DWORD* pPix = ( DWORD* ) m_camptr->d3dlr.pBits;
	DWORD  pGet;

	// get offset in file
	pGet = pPix [ ( ( m_camptr->d3dlr.Pitch >> 4 ) * iX ) + iY ];

	// break value down
	DWORD dwAlpha = pGet >> 24;						// get alpha
	DWORD dwRed   = ((( pGet ) >> 16 ) & 0xff );	// get red
	DWORD dwGreen = ((( pGet ) >>  8 ) & 0xff );	// get green
	DWORD dwBlue  = ((( pGet ) )       & 0xff );	// get blue

	// save values
	*piR = dwRed;
	*piG = dwGreen;
	*piB = dwBlue;
}

DARKSDK GGFORMAT GetValidStencilBufferFormat ( GGFORMAT BackBufferFormat )
{
	// create the list in order of precedence
	GGFORMAT DepthFormat;
	GGFORMAT list [ ] =
							{
								GGFMT_D24S8, //GeForce4 top choice
								GGFMT_R8G8B8,
								GGFMT_A8R8G8B8,
								GGFMT_X8R8G8B8,
								GGFMT_R5G6B5,
								GGFMT_X1R5G5B5,
								GGFMT_A1R5G5B5,
								GGFMT_A4R4G4B4,
								GGFMT_R3G3B2,
								GGFMT_A8,
								GGFMT_A8R3G3B2,
								GGFMT_X4R4G4B4,
								GGFMT_A8P8,
								GGFMT_P8,
								GGFMT_L8,
								GGFMT_A8L8,
								GGFMT_A4L4,
								GGFMT_V8U8,
								GGFMT_L6V5U5,
								GGFMT_X8L8V8U8,
								GGFMT_Q8W8V8U8,
								GGFMT_V16U16,
								GGFMT_D16_LOCKABLE,
								GGFMT_D32,
								GGFMT_D15S1,
								GGFMT_D16,
								GGFMT_D24X8,
								GGFMT_D24X4S4,
							};

	for ( int iTemp = 0; iTemp < 29; iTemp++ )
	{
		// Verify that the depth format exists first
		if ( SUCCEEDED ( m_pDX->CheckDeviceFormat(  GGADAPTER_DEFAULT,
													D3DDEVTYPE_HAL,
													BackBufferFormat,
													D3DUSAGE_DEPTHSTENCIL,
													D3DRTYPE_SURFACE,
													list [ iTemp ]						) ) )
		{
			if ( SUCCEEDED ( m_pDX->CheckDepthStencilMatch	(	GGADAPTER_DEFAULT,
																D3DDEVTYPE_HAL,
																BackBufferFormat,
																BackBufferFormat,
																list [ iTemp ]				) ) )
			{
				DepthFormat = list [ iTemp ];
				break;
			}
		}
	}

	return DepthFormat;
}

DARKSDK LPGGSURFACE MakeSurfaceDX9 ( int iWidth, int iHeight, GGFORMAT Format )
{
	// create surface for bitmap
	LPGGSURFACE lpSurface = NULL;
	HRESULT hr = m_pD3D->CreateRenderTarget( iWidth, iHeight, Format,
									D3DMULTISAMPLE_NONE, 0, TRUE,
									&lpSurface, NULL);

	// return ptr to it
	return lpSurface;
}

DARKSDK LPGGSURFACE MakeSurfaceDX11 ( int iWidth, int iHeight, GGFORMAT Format )
{
	return NULL;
}

DARKSDK LPGGSURFACE MakeSurface ( int iWidth, int iHeight )
{
	#ifdef DX11
	return MakeSurfaceDX11 ( iWidth, iHeight, g_CommonSurfaceFormat );
	#else
	return MakeSurfaceDX9 ( iWidth, iHeight, g_CommonSurfaceFormat );
	#endif
}

DARKSDK LPGGSURFACE MakeSurface ( int iWidth, int iHeight, GGFORMAT Format )
{
	#ifdef DX11
	return MakeSurfaceDX11 ( iWidth, iHeight, Format );
	#else
	return MakeSurfaceDX9 ( iWidth, iHeight, Format );
	#endif
}

DARKSDK LPGGSURFACE MakeFormatDX9 ( int iID, int iWidth, int iHeight, GGFORMAT thisFormat )
{
	// make a new image

	// variable declarations
	tagCamData*	test = NULL;		// pointer to data structure
	HRESULT		hr;					// used for error checking
	
	// create a new block of memory
	test = new tagCamData;
	memset(test, 0, sizeof(tagCamData));

	// check the memory was created
	if ( test == NULL )
		return NULL;

	// clear out the memory
	memset ( test, 0, sizeof ( test ) );

	// video or system bitmap
	if ( g_bOffscreenBitmap )
	{
		// create system surface for bitmap
		// cannot draw to system bitmaps (not a render target, and vid bitmaps are SLOW)
		hr = m_pD3D->CreateOffscreenPlainSurface( iWidth, iHeight, g_CommonSurfaceFormat, D3DPOOL_SYSTEMMEM, &test->lpSurface, NULL);
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false )
				Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// no depth buffer for system bitmaps
		test->lpDepth = NULL;
	}
	else
	{
		// create video surface for bitmap
		hr = m_pD3D->CreateRenderTarget( iWidth, iHeight, g_CommonSurfaceFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &test->lpSurface, NULL);
		if ( FAILED ( hr ) )
		{
			if ( g_bSupressErrorMessage==false )
				Error ( "Failed to create new bitmap" );
			SAFE_DELETE(test);
			return NULL;
		}

		// use backbuffer to determine common GGFORMAT
		D3DSURFACE_DESC backbufferdesc;
		g_pGlob->pHoldBackBufferPtr->GetDesc(&backbufferdesc);
		GGFORMAT CommonFormat = backbufferdesc.Format;

		// create depth surface
		if ( test->lpDepth==NULL )
		{
			HRESULT hRes = m_pD3D->CreateDepthStencilSurface(	iWidth, iHeight, GetValidStencilBufferFormat(CommonFormat), D3DMULTISAMPLE_NONE, 0, TRUE,
																&test->lpDepth, NULL );
		}
	}

	// setup properties
	test->iWidth  = iWidth;					// store the height
	test->iHeight = iHeight;				// store the width
	test->iDepth = g_CommonSurfaceDepth;	// store the width
	test->bLocked = false;					// set locked to false
	test->iMirrored  = 0;		
	test->iFlipped  = 0;	
    m_List.insert( std::make_pair(iID, test) );

	// return texture ptr
	return test->lpSurface;
}

DARKSDK LPGGSURFACE MakeFormatDX11 ( int iID, int iWidth, int iHeight, GGFORMAT GGFORMAT )
{
	return NULL;
}

DARKSDK LPGGSURFACE MakeFormat ( int iID, int iWidth, int iHeight, GGFORMAT GGFORMAT )
{
	#ifdef DX11
	return MakeFormatDX11 ( iID, iWidth, iHeight, GGFORMAT );
	#else
	return MakeFormatDX9 ( iID, iWidth, iHeight, GGFORMAT );
	#endif
}

DARKSDK void SetBitmapFormat ( int iFormat )
{
	if ( iFormat == - 1 )
	{
		if ( g_pGlob )
		{
			D3DSURFACE_DESC backbufferdesc;
			g_pGlob->pHoldBackBufferPtr->GetDesc ( &backbufferdesc );
			g_CommonSurfaceFormat = backbufferdesc.Format;
		}
	}
	else
	{
		g_CommonSurfaceFormat = ( GGFORMAT ) iFormat;
	}
}

DARKSDK LPGGSURFACE MakeBitmap ( int iID, int iWidth, int iHeight )
{
	if(BitmapUpdatePtr ( iID ))
	{
        RemoveBitmap( iID );
	}
	return MakeFormat ( iID, iWidth, iHeight, g_CommonSurfaceFormat );
}

DARKSDK int GetBackBufferDepthDX9 ( DWORD* pdwSurfaceFormat )
{
	int iDepthValue = 0;
	D3DSURFACE_DESC backbufferdesc;
	g_pGlob->pHoldBackBufferPtr->GetDesc(&backbufferdesc);
	iDepthValue = GetBitDepthFromFormat(backbufferdesc.Format);
	if ( pdwSurfaceFormat ) *pdwSurfaceFormat = backbufferdesc.Format;
	return iDepthValue;
}

DARKSDK int GetBackBufferDepthDX11 ( DWORD* pdwSurfaceFormat )
{
	int iDepthValue = 32;
	if ( pdwSurfaceFormat ) *pdwSurfaceFormat = GGFMT_A8B8G8R8;
	return iDepthValue;
}

DARKSDK void UpdateBitmapZeroOfNewBackbuffer( void )
{
	// get screen dimensions from glob
	int iWidth = g_pGlob->iScreenWidth;
	int iHeight = g_pGlob->iScreenHeight;

	// remove old bitmap zero object
	if ( BitmapUpdatePtr ( 0 ) )
        RemoveBitmap(0);

	// use backbuffer to determine common GGFORMAT
	#ifdef DX11
	int iDepthValue = GetBackBufferDepthDX11 ( (DWORD*)&g_CommonSurfaceFormat );
	#else
	int iDepthValue = GetBackBufferDepthDX9 ( (DWORD*)&g_CommonSurfaceFormat );
	#endif

	// surface depth and bytecount
	g_CommonSurfaceDepth = iDepthValue;
	g_CSBPP = iDepthValue/8;

	// make new bitmap zero
	g_bOffscreenBitmap=false;
	MakeBitmap ( 0, iWidth, iHeight );

	// remove surface from it (specified on the fly)
	BitmapUpdatePtr ( 0 );
	if(m_camptr)
	{
		// Not surface created
		SAFE_RELEASE(m_camptr->lpSurface);
		SAFE_RELEASE(m_camptr->lpDepth);

		// Actual backbuffer
		#ifdef DX11
		m_camptr->iDepth = GetBackBufferDepthDX11 ( NULL );
		#else
		m_camptr->iDepth = GetBackBufferDepthDX9 ( NULL );
		#endif
	}
}

DARKSDK void DirectMirror(LPGGSURFACE pFromSurface, DWORD Width, DWORD Height)
{
	// get data from surface
	D3DSURFACE_DESC pFromDesc;
	HRESULT hRes = pFromSurface->GetDesc(&pFromDesc);

	// reverse contents of texture (mirror)
	RECT FromRect = { 0, 0, (LONG)Width, (LONG)Height };
	GGLOCKED_RECT d3dlrFrom;
	if(SUCCEEDED(hRes=pFromSurface->LockRect ( &d3dlrFrom, &FromRect, 0 ) ) )
	{
		LPSTR pToPtr = (LPSTR)d3dlrFrom.pBits;
		LPSTR pFromPtr = (LPSTR)d3dlrFrom.pBits;
		for(DWORD y=0; y<Height; y++)
		{
			LPSTR pStartOfNext=pFromPtr+d3dlrFrom.Pitch;
			pFromPtr += (Width*g_CSBPP);
			for(DWORD x=0; x<Width/2; x++)
			{
				pFromPtr -= g_CSBPP;
				if(g_CSBPP==2)
				{
					WORD wStore = *((WORD*)pToPtr);
					*(WORD*)pToPtr = *((WORD*)pFromPtr);
					*(WORD*)pFromPtr = wStore;
				}
				if(g_CSBPP==4)
				{
					DWORD dwStore = *((DWORD*)pToPtr);
					*(DWORD*)pToPtr = *((DWORD*)pFromPtr);
					*(DWORD*)pFromPtr = dwStore;
				}				
				pToPtr += g_CSBPP;
			}
			pFromPtr=pStartOfNext;
			pToPtr+=d3dlrFrom.Pitch - ((Width/2)*g_CSBPP);
		}
		pFromSurface->UnlockRect();
	}
}

DARKSDK void DirectFlip(LPGGSURFACE pFromSurface, DWORD Width, DWORD Height)
{
	// get data from surface
	D3DSURFACE_DESC pFromDesc;
	HRESULT hRes = pFromSurface->GetDesc(&pFromDesc);

	// reverse contents of texture (flip)
	RECT FromRect = { 0, 0, (LONG)Width, (LONG)Height };
	GGLOCKED_RECT d3dlrFrom;
	if(SUCCEEDED(hRes=pFromSurface->LockRect ( &d3dlrFrom, &FromRect, 0 ) ) )
	{
		DWORD* pBuffer = new DWORD[Width];
		LPSTR pToPtr = (LPSTR)d3dlrFrom.pBits;
		LPSTR pFromPtr = (LPSTR)d3dlrFrom.pBits + (d3dlrFrom.Pitch*Height);
		for(DWORD y=0; y<Height/2; y++)
		{
			pFromPtr -= d3dlrFrom.Pitch;
			memcpy(pBuffer, pToPtr, Width*g_CSBPP);
			memcpy(pToPtr, pFromPtr, Width*g_CSBPP);
			memcpy(pFromPtr, pBuffer, Width*g_CSBPP);
			pToPtr += d3dlrFrom.Pitch;
		}
		pFromSurface->UnlockRect();
		SAFE_DELETE(pBuffer);
	}
}

DARKSDK DWORD GetAverageFromRange(int addbyte, void* surface, int pitch, int width, int height, int x, int y, int rangea, int rangeb)
{
	// constants set for 32bit only
	int gABitShift = 24;
	int gRBitShift = 16;
	int gGBitShift = 8;
	int gABitFill = 255;
	int gRBitFill = 255;
	int gGBitFill = 255;
	int gBBitFill = 255;

	// if 16bit, set for 565
	if(g_CSBPP==2)
	{
		gABitShift = 15;
		gRBitShift = 11;
		gGBitShift = 5;
		gABitFill = 1;
		gRBitFill = 31;
		gGBitFill = 63;
		gBBitFill = 31;
	}

	// Prepare blank RGB averaging-data
	int aa[10*10];
	int ar[10*10];
	int ag[10*10];
	int ab[10*10];

	// Gather pixel RGB data from range
	char* start = (char*)surface;
	char* pixel;

	int aindex=0;
	for(int ty=y+rangea; ty<=y+rangeb; ty++)
	{
		pixel = start + (ty*pitch);
		pixel += (x+rangea) * addbyte;
		for(int tx=x+rangea; tx<=x+rangeb; tx++)
		{
			if(tx>=0 && ty>=0 && tx<width && ty<height)
			{
				DWORD value = *(DWORD*)pixel;
				DWORD bpp = (addbyte*8);
				if(bpp<32) value &= (1<<bpp)-1;

				int valpha	= ((value >> gABitShift	) & gABitFill );
				int vred	= ((value >> gRBitShift	) & gRBitFill );
				int vgreen	= ((value >> gGBitShift	) & gGBitFill );
				int vblue	= ((value				) & gBBitFill );

				aa[aindex] = valpha;
				ar[aindex] = vred;
				ag[aindex] = vgreen;
				ab[aindex] = vblue;

				aindex++;
				if(aindex>99) break;
			}
			pixel+=addbyte;
		}
	}
	
	// Calculate average from data
	int averagea=0;
	int averager=0;
	int averageg=0;
	int averageb=0;
	for(int a=0; a<aindex; a++)
	{
		averagea += aa[a];
		averager += ar[a];
		averageg += ag[a];
		averageb += ab[a];
	}
	averagea/=aindex;
	averager/=aindex;
	averageg/=aindex;
	averageb/=aindex;
	
	// return new RGB
	return (averagea << gABitShift) + (averager << gRBitShift) + (averageg << gGBitShift) + (averageb);
}

DARKSDK bool DirectBlur(LPGGSURFACE pFromSurface, int quantity, DWORD width, DWORD height)
{
	// Blur Buffer is 10x10 = 100 storage units only
	if(quantity>9)
	{
		return false;
	}

	// get data from surface
	D3DSURFACE_DESC pFromDesc;
	HRESULT hRes = pFromSurface->GetDesc(&pFromDesc);

	// Define range
	int rangea;
	int rangeb;
	if(quantity<=1)
	{
		rangea=0;
		rangeb=0;
	}
	else
	{
		rangea=(quantity/2)*-1;
		rangeb=(quantity+rangea)-1;
	}

	// create temp to hold source for blur code
	LPGGSURFACE pToSurface;
	m_pD3D->CreateOffscreenPlainSurface( width, height, pFromDesc.Format, D3DPOOL_SCRATCH, &pToSurface, NULL);

	// reverse contents of texture (flip)
	RECT ToRect = { 0, 0, (LONG)width, (LONG)height };
	GGLOCKED_RECT d3dlrTo;
	if(SUCCEEDED(hRes=pToSurface->LockRect ( &d3dlrTo, &ToRect, 0 ) ) )
	{
		RECT FromRect = { 0, 0, (LONG)width, (LONG)height };
		GGLOCKED_RECT d3dlrFrom;
		if(SUCCEEDED(hRes=pFromSurface->LockRect ( &d3dlrFrom, &FromRect, 0 ) ) )
		{
			// blur phase
			LPVOID destsurface = (LPVOID)d3dlrTo.pBits;
			LPVOID sourcesurface = (LPVOID)d3dlrFrom.pBits;
			DWORD addbyte = g_CSBPP;
			DWORD tpitch = d3dlrTo.Pitch;
			DWORD fpitch = d3dlrFrom.Pitch;
			char* start = (char*)destsurface;
			for(DWORD y=0; y<height; y++)
			{
				char* pixel = start;
				for(DWORD x=0; x<width; x++)
				{
					// Read RGB component on pixel
					DWORD newvalue = GetAverageFromRange(addbyte, sourcesurface, fpitch, width, height, x, y, rangea, rangeb);

					// Write result
					memcpy(pixel, &newvalue, addbyte);

					// next pixel
					pixel+=addbyte;
				}
				start+=tpitch;
			}

			// copy phase
			destsurface = (LPVOID)d3dlrTo.pBits;
			sourcesurface = (LPVOID)d3dlrFrom.pBits;
			start = (char*)destsurface;
			for(DWORD y=0; y<height; y++)
			{
				char* pixel = start;
				int sourceaddy = (y*d3dlrFrom.Pitch);
				for(DWORD x=0; x<width; x++)
				{
					// copy from temp to actual
					if(g_CSBPP==2) *(WORD*)((char*)sourcesurface + (x*g_CSBPP) + sourceaddy) = *(WORD*)pixel;
					if(g_CSBPP==4) *(DWORD*)((char*)sourcesurface + (x*g_CSBPP) + sourceaddy) = *(DWORD*)pixel;

					// next pixel
					pixel+=addbyte;
				}
				start+=tpitch;
			}

			pFromSurface->UnlockRect();
		}
		pToSurface->UnlockRect();
	}

	// release surface when done
	SAFE_RELEASE(pToSurface);

	// Complete
	return true;
}

DARKSDK bool DirectFade(LPGGSURFACE pFromSurface, int percentage, DWORD width, DWORD height)
{
	// constants set for 32bit only
	int gABitShift = 24;
	int gRBitShift = 16;
	int gGBitShift = 8;
	int gABitFill = 255;
	int gRBitFill = 255;
	int gGBitFill = 255;
	int gBBitFill = 255;

	// if 16bit, set for 565
	if(g_CSBPP==2)
	{
		gABitShift = 15;
		gRBitShift = 11;
		gGBitShift = 5;
		gABitFill = 1;
		gRBitFill = 31;
		gGBitFill = 63;
		gBBitFill = 31;
	}

	// Can only be 0-100
	if(percentage<0 || percentage>100)
		return false;

	// get data from surface
	D3DSURFACE_DESC pFromDesc;
	HRESULT hRes = pFromSurface->GetDesc(&pFromDesc);

	// lock surface and go to work
	RECT FromRect = { 0, 0, (LONG)width, (LONG)height };
	GGLOCKED_RECT d3dlrFrom;
	if(SUCCEEDED(hRes=pFromSurface->LockRect ( &d3dlrFrom, &FromRect, 0 ) ) )
	{
		// Fade Code
		LPVOID	surface = (LPVOID)d3dlrFrom.pBits;
		DWORD	addbyte = g_CSBPP;
		DWORD	pitch = d3dlrFrom.Pitch;
		float pf = percentage/100.0f;
		char* start = (char*)surface;
		char* pixel;
		for(DWORD y=0; y<height; y++)
		{
			pixel = start;
			for(DWORD x=0; x<width; x++)
			{
				// Read RGB component on pixel
				DWORD value;
				if(g_CSBPP==2) value=*(WORD*)pixel;
				if(g_CSBPP==4) value=*(DWORD*)pixel;

				DWORD bpp = (addbyte*8);
				if(bpp<32) value &= (1<<bpp)-1;

				int valpha	= ((value >> gABitShift	) & gABitFill );
				int vred	= ((value >> gRBitShift	) & gRBitFill );
				int vgreen	= ((value >> gGBitShift	) & gGBitFill );
				int vblue	= ((value				) & gBBitFill );

				// Perform fade
				vred = (int)(vred * pf);
				vgreen = (int)(vgreen * pf);
				vblue = (int)(vblue * pf);

				// Write result
				DWORD newvalue = (valpha<<gABitShift) + (vred<<gRBitShift) + (vgreen<<gGBitShift) + (vblue);
				memcpy(pixel, &newvalue, addbyte);

				// next pixel
				pixel+=addbyte;
			}
			start+=pitch;
		}
		pFromSurface->UnlockRect();
	}

	// Complete
	return true;
}

DARKSDK void CreateBitmapEx ( int iID, int iWidth, int iHeight, int iSystemMemoryMode, int iSilentError )
{
	// 200112 - silent error mode
	if ( iSilentError==1 ) g_bSupressErrorMessage = true;

	if(iID<1 || iID>MAXIMUMVALUE)
	{
		if ( g_bSupressErrorMessage==false ) RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}

	// make new bitmap
	if ( iSystemMemoryMode==1 )
		g_bOffscreenBitmap=true;
	else
		g_bOffscreenBitmap=false;

	// create surface
	MakeBitmap ( iID, iWidth, iHeight );

	// auto set to new bitmap
	SetCurrentBitmap ( iID );

	// now clear the bitmap
	m_pD3D->Clear(0,NULL,D3DCLEAR_TARGET,0,1,0);

	// 200112 - silent error mode
	g_bSupressErrorMessage = false;
}

DARKSDK void CreateBitmapEx ( int iID, int iWidth, int iHeight, int iSystemMemoryMode )
{
	CreateBitmapEx ( iID, iWidth, iHeight, iSystemMemoryMode, 0 );
}

DARKSDK void CreateBitmap ( int iID, int iWidth, int iHeight )
{
	CreateBitmapEx ( iID, iWidth, iHeight, 0 );
}

DARKSDK void LoadBitmapCore ( char* szFilename, int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}

	// get any required format
	GGFORMAT RequiredFormat = g_CommonSurfaceFormat;
	if ( BitmapUpdatePtr ( iID ) )
	{
		// Backbuffer or Bitmap To/Dest Surface
		LPGGSURFACE pExistingSurface = m_camptr->lpSurface;
		if(iID==0) pExistingSurface=g_pGlob->pHoldBackBufferPtr;

		// Get format of existing surface
		D3DSURFACE_DESC desc;
		pExistingSurface->GetDesc ( &desc );
		RequiredFormat = desc.Format;
	}

	// load the bitmap into space surface
	GGIMAGE_INFO finfo;
	HRESULT hRes = D3DXGetImageInfoFromFile( szFilename, &finfo );
	LPGGSURFACE pLoadedSurface = MakeSurface ( finfo.Width, finfo.Height, RequiredFormat );

	RECT rc = { 0, 0, (LONG)finfo.Width, (LONG)finfo.Height };
	// LEEFIX - 081102 - Prevent dithering of the loaded bitmap - causes rgb(128,128,128) to look weird
	hRes = D3DXLoadSurfaceFromFile(pLoadedSurface, NULL, &rc, szFilename, NULL, D3DX_FILTER_NONE, 0, NULL);
	if ( FAILED ( hRes ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPLOADFAILED);
		return;
	}

	// check if bitmap exists
	if ( BitmapUpdatePtr ( iID ) )
	{
		// Backbuffer or Bitmap To/Dest Surface
		LPGGSURFACE pToSurface = m_camptr->lpSurface;
		if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

		// if exists, copy clipped bitmap into it
		POINT pt = { 0,0 };
		RECT destrc = { 0, 0, (LONG)finfo.Width, (LONG)finfo.Height };
		if(destrc.right>m_camptr->iWidth) destrc.right=m_camptr->iWidth;
		if(destrc.bottom>m_camptr->iHeight) destrc.bottom=m_camptr->iHeight;
		m_pD3D->StretchRect(pLoadedSurface, &destrc, pToSurface, &destrc, GGTEXF_NONE);
	}
	else
	{
		// if new, create actual size
		g_bOffscreenBitmap=false;
		LPGGSURFACE pThisSurface = MakeBitmap ( iID, finfo.Width, finfo.Height );
		m_pD3D->StretchRect(pLoadedSurface, NULL, pThisSurface, NULL, GGTEXF_NONE);

		// find ptr to bitmap
		BitmapUpdatePtr ( iID );

		// get actual dimensions of surface
		int iDepthValue=0;
		D3DSURFACE_DESC imageddsd;
		pThisSurface->GetDesc(&imageddsd);
		iDepthValue = GetBitDepthFromFormat(imageddsd.Format);

		// fill out rest of structure
		m_camptr->iWidth  = finfo.Width;					// store the height
		m_camptr->iHeight = finfo.Height;					// store the width
		m_camptr->iDepth  = iDepthValue;				// store the depth
		m_camptr->bLocked = false;
		m_camptr->iMirrored  = 0;		
		m_camptr->iFlipped  = 0;
	}

	// free loaded surface
	SAFE_RELEASE(pLoadedSurface);

	// auto set to new bitmap
	SetCurrentBitmap ( iID );
}

DARKSDK void LoadBitmap ( char* szFilename, int iID )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	LoadBitmapCore ( VirtualFilename, iID );
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
}

DARKSDK void LoadBitmapEx ( char* szFilename )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	LoadBitmapCore ( VirtualFilename, 0 );
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
}

DARKSDK void SaveBitmap ( char* szFilename, int iID )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK void SaveBitmapEx ( char* szFilename )
{
	SaveBitmap ( szFilename, 0 );
}

DARKSDK void CopyBitmap ( int iID, int iToID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// source
	LPGGSURFACE pFromSurface = m_camptr->lpSurface;
	if(iID==0) pFromSurface=g_pGlob->pHoldBackBufferPtr;

	if(iToID<0 || iToID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iToID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// cannot copy a to a
	if(iID==iToID)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iToID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// src
	D3DSURFACE_DESC src;
	pFromSurface->GetDesc(&src);
	int iWidth = src.Width;
	int iHeight = src.Height;

	// dest
	D3DSURFACE_DESC dest;
	pToSurface->GetDesc(&dest);
	if(iWidth>(int)dest.Width) iWidth=(int)dest.Width;
	if(iHeight>(int)dest.Height) iHeight=(int)dest.Height;

	// Ensure the rectangle is within the destination area for copying
	RECT FromRect = { 0, 0, iWidth, iHeight };
	POINT ToRect = { 0, 0 };

	// copy one surface to the other
	m_pD3D->StretchRect(pFromSurface, &FromRect, pToSurface, &FromRect, GGTEXF_NONE);
}

DARKSDK void CopyBitmapEx ( int iID, int iX1, int iY1, int iX2, int iY2, int iToID, int iToX1, int iToY1, int iToX2, int iToY2 )
{
	HRESULT hRes;

	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// source
	LPGGSURFACE pFromSurface = m_camptr->lpSurface;
	if(iID==0) pFromSurface=g_pGlob->pHoldBackBufferPtr;

	if(iToID<0 || iToID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iToID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iToID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// if src and dest surfaces the same, create temp copy
	IGGSurface* pTempSrcSurface = NULL;
	if ( pFromSurface==pToSurface )
	{
		// get surface desc
		D3DSURFACE_DESC surfacedesc;
		pFromSurface->GetDesc ( &surfacedesc );

		// create temp surface
		m_pD3D->CreateRenderTarget ( surfacedesc.Width, surfacedesc.Height, surfacedesc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &pTempSrcSurface, NULL );
		if ( pTempSrcSurface )
			m_pD3D->StretchRect(pFromSurface, NULL, pTempSrcSurface, NULL, GGTEXF_NONE);

		// from is now temp
		pFromSurface = pTempSrcSurface;
	}

	
	// Check if same size areas
	if(iX2-iX1==iToX2-iToX1)
	{
		// Same size copy region

		// get surfaces from textures
		RECT FromRect = { iX1, iY1, iX2, iY2 };
		RECT ToRect = { iToX1, iToY1, iToX1+(iX2-iX1), iToY1+(iY2-iY1) };

		// copy one surface to the other
		m_pD3D->StretchRect(pFromSurface, &FromRect, pToSurface, &ToRect, GGTEXF_NONE);
	}
	else
	{
		// Different size copy (need to stretch)
		int iDestWidth = iToX2-iToX1;
		int iDestHeight = iToY2-iToY1;

		// Different size copy (need to stretch)
		double iSrcIncX = (double)(iX2-iX1)/(double)iDestWidth;
		double iSrcIncY = (double)(iY2-iY1)/(double)iDestHeight;
		double iSrcX = 0;
		double iSrcY = 0;

		// Define Rects
		RECT FromRect = { iX1, iY1, iX2, iY2 };
		RECT ToRect = { iToX1, iToY1, iToX2, iToY2 };

		// lock textures and perform stretched bit copy
		GGLOCKED_RECT d3dlrFrom;
		GGLOCKED_RECT d3dlrTo;
		if(SUCCEEDED(hRes=pFromSurface->LockRect ( &d3dlrFrom, &FromRect, 0 ) ) )
		{
			if(SUCCEEDED(hRes=pToSurface->LockRect ( &d3dlrTo, &ToRect, 0 ) ) )
			{
				LPSTR pStartFromPtr = (LPSTR)d3dlrFrom.pBits;
				LPSTR pToPtr = (LPSTR)d3dlrTo.pBits;
				LPSTR pFromPtr = pStartFromPtr;
				for(int y=0; y<iDestHeight; y++)
				{
					iSrcX = 0;
					for(int x=0; x<iDestWidth; x++)
					{
						// need to add slerpish code to vector source pixel..
						if(g_CSBPP==2) *(WORD*)pToPtr = *((WORD*)pFromPtr+(DWORD)(iSrcX));
						if(g_CSBPP==4) *(DWORD*)pToPtr = *((DWORD*)pFromPtr+(DWORD)(iSrcX));
						iSrcX += iSrcIncX;
						pToPtr += g_CSBPP;
					}
					iSrcY+=iSrcIncY;
					pFromPtr=pStartFromPtr+((DWORD)iSrcY*d3dlrFrom.Pitch);
					pToPtr+=d3dlrTo.Pitch - (iDestWidth*g_CSBPP);
				}
				pToSurface->UnlockRect();
			}
			pFromSurface->UnlockRect();
		}
	}

	// free temp surface
	SAFE_RELEASE ( pTempSrcSurface );
}

DARKSDK void DeleteBitmapEx ( int iID )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

    RemoveBitmap( iID );

	// restore current bitmap
	SetCurrentBitmap ( 0 );
}

DARKSDK void FlipBitmap ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// flip bitmap
	DirectFlip( pToSurface, m_camptr->iWidth, m_camptr->iHeight );

	// update state value
	m_camptr->iFlipped = 1-m_camptr->iFlipped;
}

DARKSDK void MirrorBitmap ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// mirror bitmap
	DirectMirror(pToSurface, m_camptr->iWidth, m_camptr->iHeight);

	// update state value
	m_camptr->iMirrored = 1-m_camptr->iMirrored;
}

DARKSDK void FadeBitmap ( int iID, int iFade )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// fade bitmap
	DirectFade ( pToSurface, iFade, m_camptr->iWidth, m_camptr->iHeight );

	// update state value
	m_camptr->iFadeValue = iFade;
}

DARKSDK void BlurBitmap ( int iID, int iBlurLevel )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return;
	}

	// Backbuffer or Bitmap To/Dest Surface
	LPGGSURFACE pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// blur bitmap
	DirectBlur ( pToSurface, iBlurLevel, m_camptr->iWidth, m_camptr->iHeight );
}

DARKSDK void SetRenderTargetAndDepthDX9 ( LPGGSURFACE pRender, LPGGSURFACE pDepth )
{
	m_pD3D->SetRenderTarget ( 0, pRender );
	m_pD3D->SetDepthStencilSurface ( pDepth );
}

DARKSDK void SetRenderTargetAndDepthDX11 ( LPGGSURFACE pRender, LPGGSURFACE pDepth )
{
}

DARKSDK void SetRenderTargetAndDepth ( LPGGSURFACE pRender, LPGGSURFACE pDepth )
{
	#ifdef DX11
	SetRenderTargetAndDepthDX11 ( pRender, pDepth );
	#else
	SetRenderTargetAndDepthDX9 ( pRender, pDepth );
	#endif
}

DARKSDK void SetCurrentBitmap ( int iID )
{
	if ( iID>=0 )
	{
		// specifies bitmap number
		if(iID<0 || iID>MAXIMUMVALUE)
		{
			RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
			return;
		}
		if ( !BitmapUpdatePtr ( iID ) )
		{
			RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
			return;
		}
	}
	else
	{
		// specifies inverse camera number
	}

	// Bitmap Zero is default backbuffer
	if ( iID==0 )
	{
		// render target is backbuffer
		///m_pD3D->SetRenderTarget ( 0, g_pGlob->pHoldBackBufferPtr );
		///m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
		SetRenderTargetAndDepth ( g_pGlob->pHoldBackBufferPtr, g_pGlob->pHoldDepthBufferPtr );
		g_pGlob->pCurrentBitmapSurface=g_pGlob->pHoldBackBufferPtr;
		g_pGlob->iCurrentBitmapNumber=iID;
	}
	else
	{
		if ( iID > 0 )
		{
			// render target is bitmap
			///m_pD3D->SetRenderTarget ( 0, m_camptr->lpSurface );
			///m_pD3D->SetDepthStencilSurface ( m_camptr->lpDepth );
			SetRenderTargetAndDepth ( m_camptr->lpSurface, m_camptr->lpDepth );
			g_pGlob->pCurrentBitmapSurface=m_camptr->lpSurface;
			g_pGlob->iCurrentBitmapNumber=iID;
		}
		else
		{
			// U70 - 190908 - render target is camera surface
			int iRenderBitmapActivityToCameraID = abs(iID);
			if ( iRenderBitmapActivityToCameraID > 0 )
			{
				tagCameraData* m_Camera_Ptr = (tagCameraData*)GetCameraInternalData ( iRenderBitmapActivityToCameraID );
				if ( m_Camera_Ptr )
				{
					///m_pD3D->SetRenderTarget ( 0, m_Camera_Ptr->pCameraToImageSurface );
					///m_pD3D->SetDepthStencilSurface ( NULL );
					SetRenderTargetAndDepth ( m_Camera_Ptr->pCameraToImageSurface, NULL );
					g_pGlob->pCurrentBitmapSurface=m_Camera_Ptr->pCameraToImageSurface;
				}
				else
					iRenderBitmapActivityToCameraID = 0;
			}
			if ( iRenderBitmapActivityToCameraID==0 )
			{
				// DEFAULT back to render target is default backbuffer
				///m_pD3D->SetRenderTarget ( 0, g_pGlob->pHoldBackBufferPtr );
				///m_pD3D->SetDepthStencilSurface ( g_pGlob->pHoldDepthBufferPtr );
				SetRenderTargetAndDepth ( g_pGlob->pHoldBackBufferPtr, g_pGlob->pHoldDepthBufferPtr );
				g_pGlob->pCurrentBitmapSurface=g_pGlob->pHoldBackBufferPtr;
			}
			g_pGlob->iCurrentBitmapNumber=0;
		}
	}
}


//
// Command Expression Functions
//

DARKSDK int CurrentBitmap ( void )
{
	// Current birmap number
	return g_pGlob->iCurrentBitmapNumber;
}

DARKSDK int BitmapExist ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}

	// returns true if the bitmap exists
	if ( !BitmapUpdatePtr ( iID ) )
		return 0;

	// return true
	return 1;
}

DARKSDK int BitmapWidth ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return 0;
	}
	return m_camptr->iWidth;
}

DARKSDK int BitmapHeight ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return 0;
	}
	return m_camptr->iHeight;
}

DARKSDK int BitmapDepth ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return 0;
	}
	return m_camptr->iDepth;
}

DARKSDK int BitmapMirrored ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return 0;
	}
	return m_camptr->iMirrored;
}

DARKSDK int BitmapFlipped ( int iID )
{
	if(iID<0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_BITMAPILLEGALNUMBER);
		return 0;
	}
	if ( !BitmapUpdatePtr ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_BITMAPNOTEXIST);
		return 0;
	}
	return m_camptr->iFlipped;
}

DARKSDK int BitmapExist ( void )
{
	return BitmapExist ( g_pGlob->iCurrentBitmapNumber );
}

DARKSDK int BitmapWidth ( void )
{
	return BitmapWidth ( g_pGlob->iCurrentBitmapNumber );
}

DARKSDK int BitmapHeight ( void )
{
	return BitmapHeight ( g_pGlob->iCurrentBitmapNumber );
}

DARKSDK int BitmapDepth ( void )
{
	return BitmapDepth ( g_pGlob->iCurrentBitmapNumber );
}

DARKSDK int BitmapMirrored ( void )
{
	return BitmapMirrored ( g_pGlob->iCurrentBitmapNumber );
}

DARKSDK int BitmapFlipped ( void )
{
	return BitmapFlipped ( g_pGlob->iCurrentBitmapNumber );
}

// Data Access Functions

DARKSDK void GetBitmapData( int iID, DWORD* dwWidth, DWORD* dwHeight, DWORD* dwDepth, LPSTR* pData, DWORD* dwDataSize, bool bLockData )
{
	// Read Data
	if(bLockData==true)
	{
		if ( !BitmapUpdatePtr ( iID ) )
			return;

		// Backbuffer or Bitmap To/Dest Surface
		LPGGSURFACE pToSurface = m_camptr->lpSurface;
		if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

		// bitmap data
		*dwWidth = m_camptr->iWidth;
		*dwHeight = m_camptr->iHeight;
		*dwDepth = m_camptr->iDepth;

		// lock
		GGLOCKED_RECT d3dlock;
		if(SUCCEEDED(pToSurface->LockRect ( &d3dlock, NULL, 0 ) ) )
		{
			// create memory
			DWORD bpp = m_camptr->iDepth/8;
			DWORD dwSizeOfBitmapData = m_camptr->iWidth*m_camptr->iHeight*bpp;
			*pData = new char[dwSizeOfBitmapData];
			*dwDataSize = dwSizeOfBitmapData;

			// copy from surface
			LPSTR pSrc = (LPSTR)d3dlock.pBits;
			LPSTR pPtr = *pData;
			for(int y=0; y<m_camptr->iHeight; y++)
			{
				memcpy(pPtr, pSrc, m_camptr->iWidth*bpp);
				pPtr+=m_camptr->iWidth*bpp;
				pSrc+=d3dlock.Pitch;
			}
			pToSurface->UnlockRect();
		}
	}
	else
	{
		// free memory
		delete *pData;
	}
}

DARKSDK void SetBitmapData( int iID, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, LPSTR pData, DWORD dwDataSize )
{
	LPGGSURFACE pToSurface=NULL;
	if ( iID>0 )
	{
		if( BitmapUpdatePtr ( iID ) )
		{
			// Backbuffer or Bitmap To/Dest Surface
			pToSurface = m_camptr->lpSurface;
			if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

			// Check new bitmap specs with existing one
			if(dwWidth==(DWORD)m_camptr->iWidth && dwHeight==(DWORD)m_camptr->iHeight && dwDepth==(DWORD)m_camptr->iDepth)
			{
				// Same bitmap size
			}
			else
			{
				// Recreate Bitmap
				DeleteBitmapEx ( iID );
				m_camptr=NULL;
			}
		}
		if(m_camptr==NULL)
		{
			GGFORMAT dFormat;
			g_bOffscreenBitmap=false;
			if ( dwDepth==16 ) dFormat=GGFMT_R5G6B5;
			if ( dwDepth==24 ) dFormat=GGFMT_R8G8B8;
			if ( dwDepth==32 ) dFormat=GGFMT_X8R8G8B8;
			MakeFormat ( iID, dwWidth, dwHeight, dFormat );
		}
	}

	// Failed to create bitmap?
	if ( !BitmapUpdatePtr ( iID ) ) return;

	// Bitmap may have changed
	pToSurface = m_camptr->lpSurface;
	if(iID==0) pToSurface=g_pGlob->pHoldBackBufferPtr;

	// Write Data
	GGLOCKED_RECT d3dlock;
	if(SUCCEEDED(pToSurface->LockRect ( &d3dlock, NULL, 0 ) ) )
	{
		// copy from surface
		DWORD databpp = dwDepth/8;
		LPSTR pSrc = (LPSTR)d3dlock.pBits;
		LPSTR pPtr = pData;
		for(DWORD y=0; y<dwHeight; y++)
		{
			memcpy(pSrc, pPtr, dwWidth*databpp);
			pPtr+=dwWidth*databpp;
			pSrc+=d3dlock.Pitch;
		}
		pToSurface->UnlockRect();
	}

}

#endif

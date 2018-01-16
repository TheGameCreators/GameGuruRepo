
//////////////////////////////////////////////////////////////////////////////////
// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE
#include "cspritesc.h"
#include ".\..\error\cerror.h"
#include "ImageSupport.h"
#include "globstruct.h"
#include ".\..\Core\SteamCheckForWorkshop.h"
#include "CGfxC.h"
#include "CImageC.h"

// Externs
extern GlobStruct*		g_pGlob;
extern LPGGDEVICE		m_pD3D;						// pointer to D3D device

namespace
{
    // Global Sprite Data
    CSpriteManager				m_SpriteManager;				// sprite manager
    tagSpriteData*				m_ptr;							// sprites internal data
    float						m_preCos [ 360 ];				// used for lookup table for rotation
    float						m_preSin [ 360 ];				// used for lookup table for rotation

    // Global Backsave Feature
    bool						g_bFirstTimeSprite = true;
    int							g_iBackSaveState = 0;			// 2-new mode-autoclear!
    LPGGSURFACE			g_pBackSaveBackBuffer = NULL;	// used to store backsave
    LPGGSURFACE			g_pBackSaveSurface = NULL;		// used to store backsave

    // Global TempSpriteForImage Pasting
     tagSpriteData				m_ImagePtr;						// temp image internal data

    bool						g_bCallFromAnim = false;

    // mike - 041005 - special option to resize sprites automatically
    int g_iSpriteResizeMode = 0;

	// Can globally squash the X of all sprites (for side by side steroscopics)
	float g_fGlobalSpriteOffsetX = 0;
	float g_fGlobalSpriteStretchX = 1.0f;
}

// Forward declarations
DARKSDK void SetVertexTextureDataForSprite( void );


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//
// INTERNAL FUNCTIONS
//

DARKSDK void SetSpritesErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void SpritesPassCoreData( LPVOID pGlobPtr )
{
	// Held in Core, used here..
	/*g_pGlob = (GlobStruct*)pGlobPtr;

	// lee - 310306 - u6rc4 - sprite uses this colour too (changed in COLOR BACKDROP)
	if ( g_pGlob ) g_pGlob->dw3DBackColor = GGCOLOR_XRGB(0,0,128);*/
}

DARKSDK void UpdateSprites ( void )
{
	// now we're ready for draw all of the sprites
	// all we need to do is call the sprite manager
	// update and it will handle everything
	m_SpriteManager.Update ( );
}

DARKSDK void SaveSpritesBack ( void )
{
	// must make the backsave
	#ifdef DX11
	#else
	if(g_pBackSaveSurface==NULL)
	{
		HRESULT hRes;
		if(g_pBackSaveBackBuffer==NULL)
		{
			hRes = m_pD3D->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pBackSaveBackBuffer);
		}
		if(g_pBackSaveBackBuffer)
		{
			// get format of backbuffer
			D3DSURFACE_DESC ddsd;
			hRes = g_pBackSaveBackBuffer->GetDesc(&ddsd);

			// save entire area where sprites can draw
			DWORD width = ddsd.Width;
			DWORD height = ddsd.Height;
			GGFORMAT GGFORMAT = ddsd.Format;
			hRes = m_pD3D->CreateOffscreenPlainSurface ( width, height, GGFORMAT, D3DPOOL_SYSTEMMEM, &g_pBackSaveSurface, NULL);
		}
	}

	// copy backbuffer to store
	if(g_iBackSaveState==1)
	{
		if(g_pBackSaveSurface && g_pBackSaveBackBuffer)
		{
			m_pD3D->GetRenderTargetData ( g_pBackSaveBackBuffer, g_pBackSaveSurface );	
		}
	}
	#endif
}

DARKSDK void RestoreSpritesBack ( void )
{
	// restore entire area from last save
	#ifdef DX11
	#else
	if(g_iBackSaveState==1)
	{
		// Legacy Mode - restore background
		if(g_pBackSaveSurface && g_pBackSaveBackBuffer)
		{
			m_pD3D->UpdateSurface ( g_pBackSaveSurface, NULL, g_pBackSaveBackBuffer, NULL);
		}
	}
	if(g_iBackSaveState==2)
	{
		// Only if some sprites present
		if(m_SpriteManager.Count()>0)
		{
			// use new sprite backdrop color to influence this blanking backdrop
			if ( g_pGlob ) m_pD3D->Clear( 0, NULL, D3DCLEAR_TARGET, g_pGlob->dw3DBackColor, 1.0f, 0 );
		}
	}
	#endif
}

DARKSDK void SpritesConstructor ( void )
{
	// sprite backfill color
	if ( g_pGlob ) g_pGlob->dw3DBackColor = GGCOLOR_XRGB(0,0,128);

	// pre calculate values for rotation, this is much quicker than having 
	// to call cos and sin every time we want to use them
	for ( int iAngle = 0; iAngle < 360; iAngle++ )
    {
		float theta = ( float ) iAngle * GG_PI / ( float ) 180;
		m_preCos [ iAngle ] = (float)cos ( theta );
		m_preSin [ iAngle ] = (float)sin ( theta );
	}
}

DARKSDK void SpritesDestructor ( void )
{
	// free up any resources allocated for the sprites
	m_SpriteManager.DeleteAll ( );

	// free global vars of DLL
	#ifdef DX11
	#else
	SAFE_RELEASE(g_pBackSaveBackBuffer);
	SAFE_RELEASE(g_pBackSaveSurface);
	#endif
}

DARKSDK void SpritesRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		SpritesDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		SpritesConstructor ( );
		SpritesPassCoreData ( g_pGlob );
	}
}

DARKSDK bool UpdateSpritesPtr ( int iID )
{
	// update internal data for the sprite
	m_ptr  = NULL;								// set to null because we need to check it later
	m_ptr  = m_SpriteManager.GetData ( iID );	// attempt to retrieve the data

	// check if we got it ok
	if ( m_ptr == NULL )
		return false;

	// if we got here we have valid data
	return true;
}



DARKSDK void UpdateAllSprites(void)
{
    for (CSpriteManager::SpritePtrConst pCheck = m_SpriteManager.begin();
         pCheck != m_SpriteManager.end();
         ++pCheck)
    {
        int id = pCheck->first;
		tagSpriteData* ptr = pCheck->second;

		int iImage = ptr->iImage;
        if (iImage > 0)
        {
//            ptr->lpTexture = GetImagePointer ( iImage );
	        ptr->iWidth    = ImageWidth   ( iImage );
	        ptr->iHeight   = ImageHeight  ( iImage );
	        ptr->fClipU    = ImageUMax    ( iImage );
	        ptr->fClipV    = ImageVMax    ( iImage );

            if (ptr->eAnimType == ONE_IMAGE)
            {
                ptr->iFrameWidth  = ptr->iWidth / ptr->iFrameAcross;
                ptr->iFrameHeight = ptr->iHeight / ptr->iFrameDown;
                ptr->iWidth = ptr->iFrameWidth;
                ptr->iHeight = ptr->iFrameHeight;
                SetSpriteFrameEx ( id, ptr->iFrame + 1 );
                // BUG: Apparent bug here on uv's for non-power-of-2 image sizes on animated single-image sprites
            }
            m_ptr = ptr;
            SetVertexTextureDataForSprite ( );
        }
	}
}



DARKSDK void CreateSprite( int iID, int iX, int iY, int iImage )
{
	
	// Sprite Use Default is backsave2
	if(g_bFirstTimeSprite)
	{
		g_bFirstTimeSprite=false;
		g_iBackSaveState=2;
	}

	// variable declarations
	tagSpriteData	data;								// data storage for sprite

	// setup default and custom values
	memset ( &data, 0, sizeof ( data ) );				// clear out data
	data.fX		   = (float)iX;								// x position
	data.fY		   = (float)iY;								// y position
	data.iImage	   = iImage;							// image reference
	data.iWidth    = ImageWidth   ( iImage );		// get image width
	data.iHeight   = ImageHeight  ( iImage );		// get image height
	data.lpTexture = GetImagePointer ( iImage );		// get image data
	data.fClipU    = ImageUMax    ( iImage );		// get image
	data.fClipV    = ImageVMax    ( iImage );		// get image
//	CalcClipFromTexture ( data.lpTexture, data.iWidth, data.iHeight, &data.fClipU, &data.fClipV );
	data.bVisible  = true;								// default to visible
	data.bFlipped  = false;								// turn flipping off
	data.bMirrored = false;								// turn mirrored off
	data.iAlpha    = 255;								// set alpha to max
	data.iRed      = 255;								// set red to max
	data.iBlue     = 255;								// set blue to max
	data.iGreen    = 255;								// set green to max
	data.eAnimType = SEPARATE_IMAGES;					// default to using separate images for sprites


	data.fXScale		= 1.0f;							
	data.fYScale		= 1.0f;							
	data.bTransparent	= true;		
	data.bGhostMode		= false;
	
	data.iXSize			= 0;
	data.iYSize			= 0;

	data.fAngle			= 0;
	data.iPriority		= 0;

	// mike - 220604
	data.iLastFrame = 0;

	if ( !data.lpTexture && iImage != -1 )
	{
		char strImageNum[32];
		sprintf ( strImageNum, "Image Number %d", iImage );
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER,strImageNum);
		return;
	}

	// create vertex data based on info supplied for sprite
	VERTEX2D vertexData [ ] =
	{
		{  0.0f,				0.0f,					0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,			0.0f },
		{  (float)data.iWidth,	0.0f,					0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		data.fClipU,	0.0f },
		{  0.0f,				(float)data.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,			data.fClipV },
		{  (float)data.iWidth,	(float)data.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		data.fClipU,	data.fClipV }
	};

	// copy data across to the vertices
	memcpy ( data.lpVertices, vertexData, sizeof ( vertexData ) );

	/*
	// all this is commented out for now, later on this will be ammended
	// because switching to using vertex buffers will gain us some performance

	unsigned char* vb_vertices = NULL;

	data.pD3D->CreateVertexBuffer
								   (
										4 * sizeof ( VERTEX2D ),
										D3DUSAGE_WRITEONLY,
										D3DFVF_VERTEX2D,
										// D3DPOOL_DEFAULT,
										D3DPOOL_MANAGED,
										&data.lpVertex
								   );

	memcpy ( data.vertices, vertexData, sizeof ( vertexData ) );

	data.lpVertex->Lock ( 0, 0, &vb_vertices, 0 );

		memcpy ( vb_vertices, vertexData, sizeof ( vertexData ) );

	data.lpVertex->Unlock ( );

	data.bUpdateVertices = true;
	*/

	// add to control and draw list
	m_SpriteManager.Add ( &data, iID );
	m_ptr  = m_SpriteManager.GetData ( iID );	// attempt to retrieve the data

	// now update the vertices based on this info
	SetVertexDataForSprite( iX, iY );
}

DARKSDK void SetVertexPosDataForSprite ( float ffX, float ffY )
{
	// get sprite information
	float fX = ffX * g_fGlobalSpriteStretchX;
	fX+= g_fGlobalSpriteOffsetX;
	int iWidth  = (int)(m_ptr->iWidth * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iHeight = (int)(m_ptr->iHeight * m_ptr->fYScale);
	int iXOffset  = (int)(m_ptr->iXOffset * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iYOffset = (int)(m_ptr->iYOffset * m_ptr->fYScale);

	// assign vertex data
	if(m_ptr->iXSize==0)
	{
		m_ptr->lpVertices [ 0 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(ffY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(ffY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(ffY - iYOffset)+iHeight-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(ffY - iYOffset)+iHeight-0.5f;
	}
	else
	{
		float fXSize = m_ptr->iXSize * g_fGlobalSpriteStretchX;
		float fXOffset = m_ptr->iXOffset * g_fGlobalSpriteStretchX;
		m_ptr->lpVertices [ 0 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(ffY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(ffY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(ffY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(ffY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
	}
}

DARKSDK void SetVertexPosDataForSprite ( int iX, int iY )
{
	// get sprite information
	float fX = (float)iX * g_fGlobalSpriteStretchX;
	fX+= g_fGlobalSpriteOffsetX;
	int iWidth  = (int)(m_ptr->iWidth * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iHeight = (int)(m_ptr->iHeight * m_ptr->fYScale);
	int iXOffset  = (int)(m_ptr->iXOffset * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iYOffset = (int)(m_ptr->iYOffset * m_ptr->fYScale);

	// assign vertex data
	if(m_ptr->iXSize==0)
	{
		m_ptr->lpVertices [ 0 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(iY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(iY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(iY - iYOffset)+iHeight-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(iY - iYOffset)+iHeight-0.5f;
	}
	else
	{
		float fXSize = m_ptr->iXSize * g_fGlobalSpriteStretchX;
		float fXOffset = m_ptr->iXOffset * g_fGlobalSpriteStretchX;
		m_ptr->lpVertices [ 0 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(iY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(iY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(iY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(iY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
	}
}

/* duplicated?
DARKSDK void SetVertexPosDataForSprite ( float ffX, float ffY )
{
	// get sprite information
	float fX = ffX * g_fGlobalSpriteStretchX;
	fX+= g_fGlobalSpriteOffsetX;
	int iWidth  = (int)(m_ptr->iWidth * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iHeight = (int)(m_ptr->iHeight * m_ptr->fYScale);
	int iXOffset  = (int)(m_ptr->iXOffset * m_ptr->fXScale * g_fGlobalSpriteStretchX);
	int iYOffset = (int)(m_ptr->iYOffset * m_ptr->fYScale);

	// assign vertex data
	if(m_ptr->iXSize==0)
	{
		m_ptr->lpVertices [ 0 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - iXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX - iXOffset)+iWidth-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(ffY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(ffY - iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(ffY - iYOffset)+iHeight-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(ffY - iYOffset)+iHeight-0.5f;
	}
	else
	{
		float fXSize = m_ptr->iXSize * g_fGlobalSpriteStretchX;
		float fXOffset = m_ptr->iXOffset * g_fGlobalSpriteStretchX;
		m_ptr->lpVertices [ 0 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].x = (float)(fX - fXOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].x = (float)(fX + fXSize - m_ptr->iXOffset)-0.5f;
		m_ptr->lpVertices [ 0 ].y = (float)(ffY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 1 ].y = (float)(ffY - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 2 ].y = (float)(ffY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
		m_ptr->lpVertices [ 3 ].y = (float)(ffY + m_ptr->iYSize - m_ptr->iYOffset)-0.5f;
	}
}
*/

DARKSDK void RotateSpriteAtPtr( void )
{
	// angle of sprite
	int iRotate = (int)m_ptr->fAngle;
	float fRotate = m_ptr->fAngle;

	// use quick method
	bool bUseQuick=false;
	if(iRotate>=0 && iRotate<=359 && iRotate==fRotate)
		bUseQuick=true;

	// use sprite vertexdata without position factor
	SetVertexPosDataForSprite( 0, 0 );

	// loop through all 4 vertices
	if(bUseQuick==true)
	{
		for ( int iTemp = 0; iTemp < 4; iTemp++ )
		{
			float fX    = m_ptr->lpVertices [ iTemp ].x;							// get the x pos
			float fY    = m_ptr->lpVertices [ iTemp ].y;							// get the y pos
			float fNewX = fX * m_preCos [ iRotate ] - fY * m_preSin [ iRotate ];	// now get the new x pos
			float fNewY = fX * m_preSin [ iRotate ] + fY * m_preCos [ iRotate ];	// get the new y pos

			// update the vertices
			m_ptr->lpVertices [ iTemp ].x = fNewX;
			m_ptr->lpVertices [ iTemp ].y = fNewY;
		}
	}
	else
	{
		for ( int iTemp = 0; iTemp < 4; iTemp++ )
		{
			float fX    = m_ptr->lpVertices [ iTemp ].x;							// get the x pos
			float fY    = m_ptr->lpVertices [ iTemp ].y;							// get the y pos
			float fNewX = (float)(fX * cos ( GGToRadian(fRotate) ) - fY * sin ( GGToRadian(fRotate) ) );	// now get the new x pos
			float fNewY = (float)(fX * sin ( GGToRadian(fRotate) ) + fY * cos ( GGToRadian(fRotate) ) );	// get the new y pos

			// update the vertices
			m_ptr->lpVertices [ iTemp ].x = fNewX;
			m_ptr->lpVertices [ iTemp ].y = fNewY;
		}
	}

	// add position to sprite vertex data
	for ( int iTemp = 0; iTemp < 4; iTemp++ )
	{
		m_ptr->lpVertices [ iTemp ].x += m_ptr->fX;
		m_ptr->lpVertices [ iTemp ].y += m_ptr->fY;
	}
}

DARKSDK void SetVertexDataForSprite( float ffX, float ffY )
{
	// if angled, must transform vertex data
	if( m_ptr->fAngle!=0.0 )
	{
		// LEEFIX - 171102 - create sprite as a rotated one - retain position data
		float sfX = m_ptr->fX;
		float sfY = m_ptr->fY;
		m_ptr->fX = ffX;
		m_ptr->fY = ffY;
		RotateSpriteAtPtr();
		m_ptr->fX = sfX;
		m_ptr->fY = sfY;
	}
	else
	{
		// standard sprite
		SetVertexPosDataForSprite( ffX, ffY );
	}
}

DARKSDK void SetVertexDataForSprite( int iX, int iY )
{
	// if angled, must transform vertex data
	if( m_ptr->fAngle!=0.0 )
	{
		// LEEFIX - 171102 - create sprite as a rotated one - retain position data
		float sfX = m_ptr->fX;
		float sfY = m_ptr->fY;
		m_ptr->fX = (float)iX;
		m_ptr->fY = (float)iY;
		RotateSpriteAtPtr();
		m_ptr->fX = sfX;
		m_ptr->fY = sfY;
	}
	else
	{
		// strandard sprite
		SetVertexPosDataForSprite( iX, iY );
	}
}

DARKSDK void SetVertexTextureDataForSprite( void )
{
	switch ( m_ptr->eAnimType )
	{
		case SEPARATE_IMAGES:
		{
			// mirror the sprite TU
			if(m_ptr->bMirrored)
			{
				m_ptr->lpVertices [ 1 ].tu = 0.0;
				m_ptr->lpVertices [ 0 ].tu = m_ptr->fClipU;
				m_ptr->lpVertices [ 3 ].tu = 0.0;
				m_ptr->lpVertices [ 2 ].tu = m_ptr->fClipU;
			}
			else
			{
				m_ptr->lpVertices [ 0 ].tu = 0.0;
				m_ptr->lpVertices [ 1 ].tu = m_ptr->fClipU;
				m_ptr->lpVertices [ 2 ].tu = 0.0;
				m_ptr->lpVertices [ 3 ].tu = m_ptr->fClipU;
			}
			// flip the sprite
			if(m_ptr->bFlipped)
			{
				m_ptr->lpVertices [ 3 ].tv = 0.0;
				m_ptr->lpVertices [ 2 ].tv = 0.0;
				m_ptr->lpVertices [ 1 ].tv = m_ptr->fClipV;
				m_ptr->lpVertices [ 0 ].tv = m_ptr->fClipV;
			}
			else
			{
				m_ptr->lpVertices [ 0 ].tv = 0.0;
				m_ptr->lpVertices [ 1 ].tv = 0.0;
				m_ptr->lpVertices [ 2 ].tv = m_ptr->fClipV;
				m_ptr->lpVertices [ 3 ].tv = m_ptr->fClipV;
			}
			break;
		}

		case ONE_IMAGE:
		{
			// setup some variables to be used
			// LEEFIX - 141102 - Added m_ptr->fClipU and m_ptr->fClipV instead of 1.0f
			float	fTuSeg	= (float)( m_ptr->fClipU / m_ptr->iFrameAcross );	// tu segment
			float	fTvSeg	= (float)( m_ptr->fClipV / m_ptr->iFrameDown   );	// tv segment
			int		iXTile	= m_ptr->iFrame % m_ptr->iFrameAcross;													// x tile
			int		iYTile	= ( m_ptr->iFrameDown - 1 ) - ( ( m_ptr->iFrame - iXTile ) / m_ptr->iFrameAcross );		// y tile
			float	fTu		= fTuSeg * iXTile;					// tu coordinate
			float	fTv		= m_ptr->fClipV - ( fTvSeg * iYTile );			// tv coordinate

			// leefix-040803-cause of some stretch distortion when animated sprites are used!!
			//float	fError  = (float)0.005;							// we will get errors because of floating point calculations

			// mke - 220604 - fix stretching error
			float	fError  = 0.0f;

			// vertice slots
			int u0=0, u1=1, u2=2, u3=3;
			int v0=0, v1=1, v2=2, v3=3;
			if(m_ptr->bMirrored)
			{
				u0=1; u1=0; u2=3; u3=2;
			}
			if(m_ptr->bFlipped)
			{
				//u0=3; u1=2; u2=1; u3=0; //020308 - flip corrected
				v0=3; v1=2; v2=1; v3=0;
			}

			// now adjust our texture coordinates
			m_ptr->lpVertices [ u0 ].tu = fTu + fError;					// vertex 0, tu
			m_ptr->lpVertices [ u1 ].tu = ( fTu + fTuSeg ) - fError;		// vertex 1, tu
			m_ptr->lpVertices [ u2 ].tu = fTu + fError;					// vertex 2, tu
			m_ptr->lpVertices [ u3 ].tu = ( fTu + fTuSeg ) - fError;		// vertex 3, tu

			m_ptr->lpVertices [ v0 ].tv = ( fTv - fTvSeg ) + fError;		// vertex 0, tv
			m_ptr->lpVertices [ v1 ].tv = ( fTv - fTvSeg ) + fError;		// vertex 1, tv
			m_ptr->lpVertices [ v2 ].tv = fTv - fError;					// vertex 2, tv
			m_ptr->lpVertices [ v3 ].tv = fTv - fError;					// vertex 3, tv

			break;
		}
	}
}

DARKSDK int ReturnOverlapResult(int iSpriteA, int iSpriteB)
{
	if(UpdateSpritesPtr(iSpriteA))
	{
		tagSpriteData* m_ptrA = m_ptr;
		if(UpdateSpritesPtr(iSpriteB))
		{
			tagSpriteData* m_ptrB = m_ptr;
			if(m_ptrA->iImage > 0 && m_ptrB->iImage > 0)
			{
				// mike - 220406 - updated collision code for rotation
                // u74b7 - minor optimisation, skip one vector of comparisons for each sprite
                GGVECTOR3 vecMin = GGVECTOR3 ( m_ptrA->lpVertices[0].x, m_ptrA->lpVertices[0].y, m_ptrA->lpVertices[0].z );
                GGVECTOR3 vecMax = vecMin;

                GGVECTOR3 vecMinA = GGVECTOR3 ( m_ptrB->lpVertices[0].x, m_ptrB->lpVertices[0].y, m_ptrB->lpVertices[0].z );
                GGVECTOR3 vecMaxA = vecMinA;

				for ( int iVertex = 1; iVertex < 4; iVertex++ ) 
				{
					if ( m_ptrA->lpVertices [ iVertex ].x < vecMin.x ) vecMin.x = m_ptrA->lpVertices [ iVertex ].x;
					if ( m_ptrA->lpVertices [ iVertex ].y < vecMin.y ) vecMin.y = m_ptrA->lpVertices [ iVertex ].y;
					if ( m_ptrA->lpVertices [ iVertex ].z < vecMin.z ) vecMin.z = m_ptrA->lpVertices [ iVertex ].z;
					if ( m_ptrA->lpVertices [ iVertex ].x > vecMax.x ) vecMax.x = m_ptrA->lpVertices [ iVertex ].x;
					if ( m_ptrA->lpVertices [ iVertex ].y > vecMax.y ) vecMax.y = m_ptrA->lpVertices [ iVertex ].y;
					if ( m_ptrA->lpVertices [ iVertex ].z > vecMax.z ) vecMax.z = m_ptrA->lpVertices [ iVertex ].z;

					if ( m_ptrB->lpVertices [ iVertex ].x < vecMinA.x ) vecMinA.x = m_ptrB->lpVertices [ iVertex ].x;
					if ( m_ptrB->lpVertices [ iVertex ].y < vecMinA.y ) vecMinA.y = m_ptrB->lpVertices [ iVertex ].y;
					if ( m_ptrB->lpVertices [ iVertex ].z < vecMinA.z ) vecMinA.z = m_ptrB->lpVertices [ iVertex ].z;
					if ( m_ptrB->lpVertices [ iVertex ].x > vecMaxA.x ) vecMaxA.x = m_ptrB->lpVertices [ iVertex ].x;
					if ( m_ptrB->lpVertices [ iVertex ].y > vecMaxA.y ) vecMaxA.y = m_ptrB->lpVertices [ iVertex ].y;
					if ( m_ptrB->lpVertices [ iVertex ].z > vecMaxA.z ) vecMaxA.z = m_ptrB->lpVertices [ iVertex ].z;
				}

				// check box intersection
				if (vecMax.x >= vecMinA.x && vecMin.x <= vecMaxA.x &&
					vecMax.y >= vecMinA.y && vecMin.y <= vecMaxA.y )
						return 1;
			}
		}
	}
	return 0;
}

DARKSDK int CheckSpriteCollision( int iID, int iTarget )
{
	int overlap=0;
	if(iTarget>0)
	{
		// Check against target sprite
		if(UpdateSpritesPtr(iTarget))
			overlap = ReturnOverlapResult(iID, iTarget);
		else
			overlap = 0;
	}
	else
	{
		// Check against all sprites
		int collisionwith=0;

		// For when images are changed, sprites may have old textures
		for (CSpriteManager::SpritePtrConst pCheck = m_SpriteManager.begin();
             pCheck != m_SpriteManager.end();
             ++pCheck)
		{
			// get sprite info
			iTarget = pCheck->first;
			if(iTarget!=iID)
			{
				if(ReturnOverlapResult(iID, iTarget)==1)
				{
					collisionwith=iTarget;
					break;
				}
			}
		}
		overlap = collisionwith;
	}
	return overlap;
}

DARKSDK void GetSpritesDiffuse ( int iID, int* piRed, int* piGreen, int* piBlue )
{
	// check all of the pointers are valid
	if ( !piRed || !piGreen || !piBlue )
		Error ( "Invalid pointers passed to GetDiffuse" );

	// now assign the diffuse values
	*piRed   = m_ptr->iRed;			// copy red
	*piGreen = m_ptr->iGreen;		// copy green
	*piBlue  = m_ptr->iBlue;		// copy blue
}

DARKSDK void PasteImage ( int iImageID, int iX, int iY, float fU, float fV )
{
	PasteImage ( iImageID, iX, iY, fU, fV, 0 );
}

DARKSDK void PasteImage ( int iImageID, int iX, int iY, float fU, float fV, int iTransparent )
{
	// Setup temp image sprite
	memset ( &m_ImagePtr, 0, sizeof ( m_ImagePtr ) );				// clear out data
	m_ImagePtr.fX		   = (float)iX;								// x position
	m_ImagePtr.fY		   = (float)iY;								// y position
	m_ImagePtr.iImage	   = iImageID;							// image reference
	m_ImagePtr.iWidth    = ImageWidth   ( iImageID );		// get image width
	m_ImagePtr.iHeight   = ImageHeight  ( iImageID );		// get image height
	m_ImagePtr.lpTexture = GetImagePointer ( iImageID );		// get image data
	m_ImagePtr.fClipU    = fU;		// get image
	m_ImagePtr.fClipV    = fV;		// get image
	m_ImagePtr.bVisible  = true;								// default to visible
	m_ImagePtr.bFlipped  = false;								// turn flipping off
	m_ImagePtr.bMirrored = false;								// turn mirrored off
	m_ImagePtr.iAlpha    = 255;								// set alpha to max
	m_ImagePtr.iRed      = 255;								// set red to max
	m_ImagePtr.iBlue     = 255;								// set blue to max
	m_ImagePtr.iGreen    = 255;								// set green to max
	m_ImagePtr.eAnimType = SEPARATE_IMAGES;					// default to using separate images for sprites
	m_ImagePtr.fXScale		= 1.0f;							
	m_ImagePtr.fYScale		= 1.0f;		
	m_ImagePtr.iXSize		= 0;
	m_ImagePtr.iYSize		= 0;
	m_ImagePtr.fAngle		= 0;
	m_ImagePtr.iPriority	= 0;

	// Transparency
	if(iTransparent>0)
		m_ImagePtr.bTransparent	= true;	
	else
		m_ImagePtr.bTransparent	= false;	

	// create vertex data based on info supplied for sprite
	VERTEX2D vertexData [ ] =
	{
		{  0.0f,						0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	0.0f },
		{  (float)m_ImagePtr.iWidth,	0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		0.0f },
		{  0.0f,						(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	fV },
		{  (float)m_ImagePtr.iWidth,	(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		fV }
	};

	// copy data across to the vertices
	memcpy ( m_ImagePtr.lpVertices, vertexData, sizeof ( vertexData ) );

	// Assign pr to imageptr
	m_ptr = &m_ImagePtr;

	// Draw TempImageSprite Once
	SetVertexDataForSprite( iX, iY );
	m_SpriteManager.DrawImmediate ( m_ptr );
}

// mike - 220604 - addition for text
DARKSDK void PasteTextureToRectEx ( LPGGTEXTURE pTexture, float fU, float fOrigV, RECT Rect, int iX, int iY )
{
	// if fOrigV is negative, reverse UV on the Y
	float fRev = 0.0f;
	float fV = fOrigV;
	if(fV<0.0f) { fV=1.0f; fRev=1.0f; }

	// Word out destination area
	//int iX=Rect.left;
	//int iY=Rect.top;
	int iWidth=Rect.right-Rect.left;
	int iHeight=Rect.bottom-Rect.top;

	// Setup temp image sprite
	memset ( &m_ImagePtr, 0, sizeof ( m_ImagePtr ) );				// clear out data
	m_ImagePtr.fX		   = (float)iX;								// x position
	m_ImagePtr.fY		   = (float)iY;								// y position
	m_ImagePtr.iImage	   = 0;							// image reference
	m_ImagePtr.iWidth    = iWidth;		// get passed width
	m_ImagePtr.iHeight   = iHeight;		// get passed height
	m_ImagePtr.lpTexture = pTexture;	// get passed texture
	m_ImagePtr.bVisible  = true;								// default to visible
	m_ImagePtr.bFlipped  = false;								// turn flipping off
	m_ImagePtr.bMirrored = false;								// turn mirrored off
	m_ImagePtr.iAlpha    = 255;								// set alpha to max
	m_ImagePtr.iRed      = 255;								// set red to max
	m_ImagePtr.iBlue     = 255;								// set blue to max
	m_ImagePtr.iGreen    = 255;								// set green to max
	m_ImagePtr.eAnimType = SEPARATE_IMAGES;					// default to using separate images for sprites
	m_ImagePtr.fXScale		= 1.0f;							
	m_ImagePtr.fYScale		= 1.0f;							
	m_ImagePtr.bTransparent	= false;		
	
	m_ImagePtr.bTransparent	= true;	

	m_ImagePtr.iXSize		= 0;
	m_ImagePtr.iYSize		= 0;
	m_ImagePtr.iPriority	= 0;

	// this vertex data is Y reversed as animation showed upside down
	VERTEX2D vertexData [ ] =
	{
		{  0.0f,						0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	fRev },
		{  (float)m_ImagePtr.iWidth,	0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		fRev },
		{  0.0f,						(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	fV-fRev },
		{  (float)m_ImagePtr.iWidth,	(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		fV-fRev }
	};

	// copy data across to the vertices
	memcpy ( m_ImagePtr.lpVertices, vertexData, sizeof ( vertexData ) );

	// Assign pr to imageptr
	m_ptr = &m_ImagePtr;

	// Draw TempImageSprite Once
	SetVertexDataForSprite( iX, iY );
	m_SpriteManager.DrawImmediate ( m_ptr );
}

DARKSDK void PasteTextureToRect ( LPGGTEXTURE pTexture, float fU, float fOrigV, RECT Rect )
{
	// if fOrigV is negative, reverse UV on the Y
	float fRev = 0.0f;
	float fV = fOrigV;
	if(fV<0.0f) { fV=1.0f; fRev=1.0f; }

	// Word out destination area
	int iX=Rect.left;
	int iY=Rect.top;
	int iWidth=Rect.right-Rect.left;
	int iHeight=Rect.bottom-Rect.top;

	// Setup temp image sprite
	memset ( &m_ImagePtr, 0, sizeof ( m_ImagePtr ) );				// clear out data
	m_ImagePtr.fX		   = (float)iX;								// x position
	m_ImagePtr.fY		   = (float)iY;								// y position
	m_ImagePtr.iImage	   = 0;							// image reference
	m_ImagePtr.iWidth    = iWidth;		// get passed width
	m_ImagePtr.iHeight   = iHeight;		// get passed height
	m_ImagePtr.lpTexture = pTexture;	// get passed texture
	m_ImagePtr.bVisible  = true;								// default to visible
	m_ImagePtr.bFlipped  = false;								// turn flipping off
	m_ImagePtr.bMirrored = false;								// turn mirrored off
	m_ImagePtr.iAlpha    = 255;								// set alpha to max
	m_ImagePtr.iRed      = 255;								// set red to max
	m_ImagePtr.iBlue     = 255;								// set blue to max
	m_ImagePtr.iGreen    = 255;								// set green to max
	m_ImagePtr.eAnimType = SEPARATE_IMAGES;					// default to using separate images for sprites
	m_ImagePtr.fXScale		= 1.0f;							
	m_ImagePtr.fYScale		= 1.0f;							
	m_ImagePtr.bTransparent	= false;		
	
	//m_ImagePtr.bTransparent	= true;	

	m_ImagePtr.iXSize		= 0;
	m_ImagePtr.iYSize		= 0;
	m_ImagePtr.iPriority	= 0;

	// this vertex data is Y reversed as animation showed upside down
	VERTEX2D vertexData [ ] =
	{
		{  0.0f,						0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	fRev },
		{  (float)m_ImagePtr.iWidth,	0.0f,						0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		fRev },
		{  0.0f,						(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		0.0f,	fV-fRev },
		{  (float)m_ImagePtr.iWidth,	(float)m_ImagePtr.iHeight,	0.5f,	1.0f,	GGCOLOR_ARGB ( 255, 255, 255, 255 ),		fU,		fV-fRev }
	};

	// copy data across to the vertices
	memcpy ( m_ImagePtr.lpVertices, vertexData, sizeof ( vertexData ) );

	// Assign pr to imageptr
	m_ptr = &m_ImagePtr;

	// Draw TempImageSprite Once
	SetVertexDataForSprite( iX, iY );
	m_SpriteManager.DrawImmediate ( m_ptr );
}


//
// COMMAND SET FUNCTIONS
//

DARKSDK void SetSprite ( int iID, int iBacksave, int iTransparent )
{
	// Range Value
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	// Backsave Flag (2 is new state)
	if(iBacksave<0 || iBacksave>2)
	{
		RunTimeError(RUNTIMEERROR_SPRITEBACKSAVEILLEGAL);
		return;
	}

	// Transparency Flag (U76 - 070710 - extra transparency mode for FPGC crosshair sprite)
	if(iTransparent<0 || iTransparent>2)
	{
		RunTimeError(RUNTIMEERROR_SPRITETRANSPARENCYILLEGAL);
		return;
	}

	// check if the sprite already exists
	if ( !UpdateSpritesPtr ( iID ) )
	{
		// Create Sprite if new
		CreateSprite( iID, 0, 0, -1 );
	}

	// Back Save State
	g_iBackSaveState = iBacksave;
	g_bFirstTimeSprite=false;
	
	// Transparency
	if(iTransparent==0)
	{
		if ( m_ptr )
			m_ptr->bTransparent=false;
	}
	else
	{
		if ( m_ptr )
			m_ptr->bTransparent=true;

		// U76 - 070710 - new mode to simulate 3D object ghost mode, but for sprites
		if ( iTransparent==2 )
			m_ptr->bGhostMode=true;
	}
}

DARKSDK void Sprite ( int iID, float fX, float fY, int iImage )
{
	// Range Value
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	// Image Value
	if(iImage<=0 || iImage>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}
	
	// create a sprite, or if one already exists then
	// update it's internal data

	// check if the sprite already exists
	if ( UpdateSpritesPtr ( iID ) )
	{
		// if it does we don't need to change anything else
		// simply update it's internal values
		m_ptr->fX = fX;	// update x pos
		m_ptr->fY = fY;	// update y pos

		if ( m_ptr->eAnimType==ONE_IMAGE )
		{
			// update sprite info (no image)
			//SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
			SetVertexDataForSprite( m_ptr->fX, m_ptr->fY );
		}
		else
		{
			// only get a new pointer if it's a different image
			if ( iImage != m_ptr->iImage )
			{
				m_ptr->iWidth = ImageWidth ( iImage );
				m_ptr->iHeight = ImageHeight ( iImage );
				m_ptr->lpTexture = GetImagePointer ( iImage );
				m_ptr->fClipU = ImageUMax ( iImage );
				m_ptr->fClipV = ImageVMax ( iImage );
			}
			
			// save the image reference
			if(iImage>0)
				m_ptr->iImage = iImage;
			else
				m_ptr->iImage = -1;

			// update sprite info
			//SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
			SetVertexDataForSprite( m_ptr->fX, m_ptr->fY );
			SetVertexTextureDataForSprite();
		}

		// and finally return - we can skip out the rest
		return;
	}

	// Create Sprite 
	CreateSprite( iID, (int)fX, (int)fY, iImage );
}

DARKSDK void Sprite ( int iID, int iX, int iY, int iImage )
{
	// Range Value
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	// Image Value
	if(iImage<=0 || iImage>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}
	
	// create a sprite, or if one already exists then
	// update it's internal data

	// check if the sprite already exists
	if ( UpdateSpritesPtr ( iID ) )
	{
		// if it does we don't need to change anything else
		// simply update it's internal values
		m_ptr->fX = (float)iX;	// update x pos
		m_ptr->fY = (float)iY;	// update y pos

		if ( m_ptr->eAnimType==ONE_IMAGE )
		{
			// update sprite info (no image)
			SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
		}
		else
		{
			// only get a new pointer if it's a different image
			if ( iImage != m_ptr->iImage )
			{
				m_ptr->iWidth = ImageWidth ( iImage );
				m_ptr->iHeight = ImageHeight ( iImage );
				m_ptr->lpTexture = GetImagePointer ( iImage );
				m_ptr->fClipU = ImageUMax ( iImage );
				m_ptr->fClipV = ImageVMax ( iImage );
			}
			
			// save the image reference
			if(iImage>0)
				m_ptr->iImage = iImage;
			else
				m_ptr->iImage = -1;

			// update sprite info
			SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
			SetVertexTextureDataForSprite();
		}

		// and finally return - we can skip out the rest
		return;
	}

	// Create Sprite 
	CreateSprite( iID, iX, iY, iImage );
}

DARKSDK void PasteSprite ( int iID, int iX, int iY )
{
	// can instantly disable this feature temporarily for screen blanking
	if ( m_SpriteManager.m_TempDisableDraw == false )
	{
		if(iID<=0 || iID>MAXIMUMVALUE)
		{
			RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
			return;
		}

		if(!UpdateSpritesPtr(iID))
		{
			RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
			return;
		}

		// support for stereo side by side
		/*
		GGVIEWPORT SaveViewport;
		if ( g_fGlobalSpriteStretchX != 1.0f )
		{
			m_pD3D->GetViewport(&SaveViewport);
			GGVIEWPORT StretchedViewport = SaveViewport;
			StretchedViewport.X = (int)g_fGlobalSpriteOffsetX;
			StretchedViewport.Width = (int)(SaveViewport.Width * g_fGlobalSpriteStretchX);
			m_pD3D->SetViewport(&StretchedViewport);
		}
		*/

		// now update the vertices based on this info
		SetVertexDataForSprite( iX, iY );

		// LEEFIX - 141102 - store visibility
		bool bVis = m_ptr->bVisible;
		m_ptr->bVisible=true;

		// draw the sprite on screen
		m_SpriteManager.DrawImmediate ( m_ptr );

		// Restore visibility
		m_ptr->bVisible = bVis;

		// mike - 220406 - must update original sprite again
		RotateSprite ( iID, m_ptr->fAngle );

		/*
		// support for stereo side by side
		if ( g_fGlobalSpriteStretchX != 1.0f )
			m_pD3D->SetViewport(&SaveViewport);
		*/
	}
}

DARKSDK void PasteSprite ( int iID, int iX, int iY, int iDrawImmediately )
{
}

DARKSDK void ResetSpriteBatcher ( void )
{
	// resets batcher to start collecting immediate draw calls
	m_SpriteManager.m_bSpriteBatcherActive = true;
}

DARKSDK void DrawSpriteBatcher ( void )
{
	// batcher finished collecting immediate draw calls
	m_SpriteManager.m_bSpriteBatcherActive = false;

	// and render out in one draw call
	m_SpriteManager.DrawBatchImmediate();
}

DARKSDK void SizeSprite ( int iID, int iXSize, int iYSize )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iXSize<0 || iYSize<0)
	{
		RunTimeError(RUNTIMEERROR_SPRITESIZEILLEGAL);
		return;
	}

	// change the size of the sprite in pixels
	m_ptr->iXSize = iXSize;	// save the x size
	m_ptr->iYSize = iYSize;	// save the y size
	m_ptr->iWidth  = iXSize; // update width
	m_ptr->iHeight  = iYSize; // update width
	//m_ptr->fXScale  = 1.0f;
	//m_ptr->fYScale  = 1.0f;

	// mike - 220406 - need to return right scale values
	m_ptr->fXScale = iXSize/100.0f;
	m_ptr->fYScale = iYSize/100.0f;


	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void ScaleSprite ( int iID, float fScale )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	float scale = fScale;
	if(scale<0.0f || scale>250000.0f)
	{
		RunTimeError(RUNTIMEERROR_SPRITESCALEILLEGAL);
		return;
	}

	// Dont use pixel sizing
	m_ptr->iXSize = 0;
	m_ptr->iYSize = 0;

	// leefix - 020308 - if preprepared animated sprite, do not adjust its width (already set for the animated sprite)
	if ( m_ptr->eAnimType != ONE_IMAGE )
	{
		m_ptr->iWidth = ImageWidth   ( m_ptr->iImage );		// get image width
		m_ptr->iHeight= ImageHeight  ( m_ptr->iImage );		// get image height
	}

	// scale the size
	m_ptr->fXScale = scale/100.0f;
	m_ptr->fYScale = scale/100.0f;

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void StretchSprite ( int iID, int iXStretch, int iYStretch )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	float scalex = (float)iXStretch;
	float scaley = (float)iYStretch;
	if(scalex<0.0f || scaley<0.0f || scalex>250000.0f || scaley>250000.0f)
	{
		RunTimeError(RUNTIMEERROR_SPRITESCALEILLEGAL);
		return;
	}

	// Dont use pixel sizing
	m_ptr->iXSize = 0;
	m_ptr->iYSize = 0;

	// stretch the sprite
	m_ptr->fXScale = scalex/100.0f;
	m_ptr->fYScale = scaley/100.0f;

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}


DARKSDK void OffsetSprite ( int iID, int iXOffset, int iYOffset )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// setup the sprite offset
	m_ptr->iXOffset = iXOffset;
	m_ptr->iYOffset = iYOffset;

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void MirrorSprite ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// toggle mirror
	if(m_ptr->bMirrored==true)
		m_ptr->bMirrored=false;
	else
		m_ptr->bMirrored=true;

	// LEEFIX 101102 - use texture updater (handles mirror flag
	SetVertexTextureDataForSprite();
	/*
	// mirror the sprite
	if(m_ptr->bMirrored)
	{
		m_ptr->lpVertices [ 1 ].tu = 0.0;
		m_ptr->lpVertices [ 0 ].tu = m_ptr->fClipU;
		m_ptr->lpVertices [ 3 ].tu = 0.0;
		m_ptr->lpVertices [ 2 ].tu = m_ptr->fClipU;
	}
	else
	{
		m_ptr->lpVertices [ 0 ].tu = 0.0;
		m_ptr->lpVertices [ 1 ].tu = m_ptr->fClipU;
		m_ptr->lpVertices [ 2 ].tu = 0.0;
		m_ptr->lpVertices [ 3 ].tu = m_ptr->fClipU;
	}
	*/

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void Flip ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// toggle flip
	if(m_ptr->bFlipped==true)
		m_ptr->bFlipped=false;
	else
		m_ptr->bFlipped=true;

	// LEEFIX 101102 - use texture updater (handles flip flag
	SetVertexTextureDataForSprite();
	/*
	// flip the sprite
	if(m_ptr->bFlipped)
	{
		m_ptr->lpVertices [ 3 ].tv = 0.0;
		m_ptr->lpVertices [ 2 ].tv = 0.0;
		m_ptr->lpVertices [ 1 ].tv = m_ptr->fClipV;
		m_ptr->lpVertices [ 0 ].tv = m_ptr->fClipV;
	}
	else
	{
		m_ptr->lpVertices [ 0 ].tv = 0.0;
		m_ptr->lpVertices [ 1 ].tv = 0.0;
		m_ptr->lpVertices [ 2 ].tv = m_ptr->fClipV;
		m_ptr->lpVertices [ 3 ].tv = m_ptr->fClipV;
	}
	*/

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void HideSprite ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// hide a sprite
	m_ptr->bVisible = false;
}

DARKSDK void ShowSprite ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// show a sprite
	m_ptr->bVisible = true;
}

DARKSDK void HideAllSprites ( void )
{
	// hide all sprites
	m_SpriteManager.HideAll ( );
}

DARKSDK void ShowAllSprites ( void )
{
	// show all sprites
	m_SpriteManager.ShowAll ( );
}

DARKSDK void DisableAllSprites ( void )
{
	// show all sprites
	m_SpriteManager.DisableAll ( );
}

DARKSDK void EnableAllSprites ( void )
{
	// show all sprites
	m_SpriteManager.EnableAll ( );
}

DARKSDK void DeleteSprite ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// get the manager to delete the sprite
	m_SpriteManager.Delete ( iID );

	// mike - 230604
	if ( m_SpriteManager.Count ( ) == 0 )
	{
		g_iBackSaveState = 0;
	}
}

DARKSDK void MoveSprite ( int iID, float velocity )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// move sprite by angle and velocity
	m_ptr->fX += (float)(sin(GGToRadian(m_ptr->fAngle)) * velocity);
	m_ptr->fY -= (float)(cos(GGToRadian(m_ptr->fAngle)) * velocity);

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void RotateSprite ( int iID, float fRotate )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// rotate a sprite
	m_ptr->fAngle = fRotate;

	// update sprite info
	SetVertexDataForSprite( (int)m_ptr->fX, (int)m_ptr->fY );
}

DARKSDK void SetSpriteAlpha ( int iID, int iValue )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iValue<0 || iValue>MAXIMUMALPHAVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEALPHAVALUEILLEGAL);
		return;
	}

	// set the alpha level for a sprite
	m_ptr->iAlpha = iValue;

	// update the diffuse components
	m_ptr->lpVertices [ 0 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 1 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 2 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 3 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
}

DARKSDK void SetSpriteDiffuse ( int iID, int iR, int iG, int iB )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iR<0 || iR>MAXIMUMRGBCOMPONENT)
	{
		RunTimeError(RUNTIMEERROR_SPRITERGBCOMPONENTILLEGAL);
		return;
	}

	if(iG<0 || iG>MAXIMUMRGBCOMPONENT)
	{
		RunTimeError(RUNTIMEERROR_SPRITERGBCOMPONENTILLEGAL);
		return;
	}

	if(iB<0 || iB>MAXIMUMRGBCOMPONENT)
	{
		RunTimeError(RUNTIMEERROR_SPRITERGBCOMPONENTILLEGAL);
		return;
	}

	// store the red, green and blue values
	m_ptr->iRed   = iR;
	m_ptr->iGreen = iG;
	m_ptr->iBlue  = iB;

	// update the diffuse components
	m_ptr->lpVertices [ 0 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 1 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 2 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
	m_ptr->lpVertices [ 3 ].color = GGCOLOR_ARGB ( m_ptr->iAlpha, m_ptr->iRed, m_ptr->iGreen, m_ptr->iBlue );
}

DARKSDK void ResizeSprite ( int iMode, float fOffsetX, float fScaleX )
{
	g_iSpriteResizeMode = iMode;
	g_fGlobalSpriteOffsetX = fOffsetX;
	g_fGlobalSpriteStretchX = fScaleX;
}

DARKSDK void SetSpriteResize ( int iMode )
{
	// mike - 041005 - special option to resize sprites automatically
	ResizeSprite ( iMode, 0.0f, 1.0f );
}

DARKSDK void SetSpriteImage ( int iID, int iImage )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iImage<=0 || iImage>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}

	// set the image that the sprite uses
	m_ptr->iImage    = iImage;							// store the image number
	m_ptr->lpTexture = GetImagePointer ( iImage );	// get a pointer to the image data
	m_ptr->fClipU = ImageUMax ( iImage );
	m_ptr->fClipV = ImageVMax ( iImage );

	// mike - 021005 - ensure we still match height and width
	if ( g_iSpriteResizeMode )
	{
		m_ptr->iHeight = ImageHeight ( iImage );
		m_ptr->iWidth = ImageWidth ( iImage );
	}

	// Update sprite texture UVs
	SetVertexTextureDataForSprite();
}

DARKSDK void PlaySprite ( int iID, int iStart, int iEnd, int iDelay )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iStart<=0 || iStart>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}

	if(iEnd<=0 || iEnd>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}

	if(iDelay<0 || iDelay>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEANIMDELAYILLEGAL);
		return;
	}

	// mike - 220604 - modify start and end frames
	iStart--;
	iEnd--;

	// create a sprite if one doesn't exist
	if ( !UpdateSpritesPtr ( iID ) )
	{
		Sprite ( iID, 0, 0, iStart );
	}

	// LEEFIX - 171102 - animation can be adjusted at any time
	m_ptr->iStart  = iStart;	// start frame
	m_ptr->iEnd    = iEnd;		// end frame
	m_ptr->iDelay  = iDelay;	// timer delay

	// has the anim been setup
	if ( !m_ptr->bIsAnim )
	{
		// mike - 220604 - update start frame
		m_ptr->iFrame = m_ptr->iStart;

		// setup start anim properties
		m_ptr->bIsAnim = true;		// set anim flag on
	}
	else
	{
		// mike - 220604 - fail when play frame is invalid
		if ( m_ptr->iFrame > m_ptr->iFrameCount )
		{
			RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
			return;
		}

		// get current time
		m_ptr->dwCurrentTime = timeGetTime ( );
			
		// when the amount of time + delay has passed we 
		// can set the  correct image to be displayed
		if ( m_ptr->dwCurrentTime > m_ptr->dwLastTime + m_ptr->iDelay )
		{
			// check the animation type
			switch ( m_ptr->eAnimType )
			{
				case SEPARATE_IMAGES:
				{
					// when using separate images

					// update the frame
					SetSpriteFrameEx ( iID, m_ptr->iImage );

					// move to the next image
					if ( m_ptr->iImage < m_ptr->iEnd )
						m_ptr->iImage++;
					else
						m_ptr->iImage = m_ptr->iStart;
				}
				break;

				case ONE_IMAGE:
				{
					// when using one image that contains multiple frames

					// update the frame
					g_bCallFromAnim = true;
					SetSpriteFrameEx ( iID, m_ptr->iFrame );
					g_bCallFromAnim = false;

					// mike - 220604 - save frame
					m_ptr->iLastFrame = m_ptr->iFrame;

					// update the width and height
					m_ptr->iWidth  = m_ptr->iFrameWidth;	// update width
					m_ptr->iHeight = m_ptr->iFrameHeight;	// update height

					// update the frame
					if ( m_ptr->iFrame < m_ptr->iEnd )
						m_ptr->iFrame++;
					else
						m_ptr->iFrame = m_ptr->iStart;
				}
				break;
			}

			// save the time
			m_ptr->dwLastTime = m_ptr->dwCurrentTime;
		}
	}
}

DARKSDK void SetSpriteFrameEx ( int iID, int iFrame )
{
	// mike - 220406
	if ( !g_bCallFromAnim )
	{
		if ( iFrame > 0 )
			iFrame--;
	}

	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iFrame<0 || iFrame>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}

	// now check the animation type
	switch ( m_ptr->eAnimType )
	{
		case SEPARATE_IMAGES:
		{
			// using separate images is really easy, all we need
			// to is increment the image based on the start and
			// end frames

			// fail if the frame does not exist
			if ( !ImageExist ( iFrame ) )
				return;

			// store the new frame
			m_ptr->iImage = iFrame;

			// retrieve the correct image based on the anim
			m_ptr->lpTexture = GetImagePointer ( m_ptr->iImage );
		}
		break;

		case ONE_IMAGE:
		{
			// using one image for animation works slightly differently
			// this time all the frames for an animation are kept on
			// one image, this can help to preserve image memory

			// to achieve this we need to simply adjust the texture coordinates

			// must have a frame to divide
			if(m_ptr->iFrameAcross>0 && m_ptr->iFrameDown>0)
			{
				// first off check if the frame does exist
				if ( iFrame > m_ptr->iFrameCount )
					return;

				m_ptr->iLastFrame = m_ptr->iFrame;

				// store the current frame
				m_ptr->iFrame = iFrame;

				//if ( iFrame == 1 )
				//	m_ptr->iFrame = iFrame - 1;
				//else
				//	m_ptr->iFrame = iFrame;

				// update the width and height
				m_ptr->iWidth  = m_ptr->iFrameWidth;	// update width
				m_ptr->iHeight = m_ptr->iFrameHeight;	// update height
			}
		}
		break;
	}

	// Now assign the texture data
	SetVertexTextureDataForSprite();
}

DARKSDK void SetSpriteFrame ( int iID, int iFrame )
{
	// check parameters
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}
	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// leefix - 160308 - when stop using PLAY SPRITE and use SET SPRITE FRAME, switch off anim flag for manual control again
	m_ptr->bIsAnim = false;

	// call actual function
	SetSpriteFrameEx ( iID, iFrame );
}

DARKSDK void SetSpriteTextureCoordinates ( int iID, int iVertex, float tu, float tv )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	if(iVertex<0 || iVertex>MAXIMUMQUADVERTEX)
	{
		RunTimeError(RUNTIMEERROR_SPRITEVERTEXILLEGAL);
		return;
	}

	m_ptr->lpVertices [ iVertex ].tu = tu;
	m_ptr->lpVertices [ iVertex ].tv = tv;
}

DARKSDK void CloneSprite ( int iID, int iDestinationID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// Store Source Ptr
	tagSpriteData* m_ptrSource = m_ptr;

	if(iDestinationID<=0 || iDestinationID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	
	if(UpdateSpritesPtr(iDestinationID))
	{
		RunTimeError(RUNTIMEERROR_SPRITEALREADYTEXISTS);
		return;
	}

    // u74b7 - removed unnecessary copying of sprite data
    m_SpriteManager.Add ( m_ptrSource, iDestinationID );
}

DARKSDK void CreateAnimatedSpriteCore ( int iID, char* szImage, int iWidth, int iHeight, int iImageID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITEALREADYTEXISTS);
		return;
	}

	if(iWidth<=0 || iWidth>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEWIDTHILLEGAL);
		return;
	}

	if(iHeight<=0 || iHeight>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEHEIGHTILLEGAL);
		return;
	}

	if(iImageID<0 || iImageID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEHANIMCOUNTILLEGAL);
		return;
	}

	// create an animated sprite using one large image
	// which contains all of the frames

	// check if the image in this ID already
	// exists, if so delete it
	if ( ImageExist ( iImageID ) )
		DeleteImage ( iImageID );

	// now load the new image
	//if( LoadImage ( szImage, iImageID )==false )
	{
		//RunTimeError(RUNTIMEERROR_INVALIDFILE);
		//return;
	}

	//DARKSDK void 				LoadEx						( LPSTR szFilename, int iID, int TextureFlag );							// load an image specifying the filename

	//LoadImageEx ( szImage, iImageID );

	// mike - 011105 - reference 1 to 1 for animated sprites
	LoadImage ( szImage, iImageID, 1 );

	//DARKSDK bool Load	( char* szFilename, int iID, int TextureFlag, bool bIgnoreNegLimit );			// load an image specifying the filename
	//LOAD IMAGE%SLL%?LoadEx@@YAXPADHH@Z%Filename, Image Number, Texture Flag

	//LoadImage ( szImage, iImageID, 1, 0 );

	
	// create a sprite offscreen
	Sprite ( iID, -1000000, -1000000, iImageID );

	// mike - 011005 - check the ptr is valid, it may fail due to invalid image
	if ( !m_ptr )
	{
		RunTimeError(RUNTIMEERROR_SPRITEERROR);
		return;
	}

	// now setup anim properties
	int iCount = iWidth * iHeight;
	m_ptr->eAnimType    = ONE_IMAGE;							// notify that we're using frames within an image
	m_ptr->iImage = iImageID;
	m_ptr->iFrameWidth  = ImageWidth  ( iImageID ) / iWidth;								// width of a frame
	m_ptr->iFrameHeight = ImageHeight ( iImageID ) / iHeight;								// height of a frame
	m_ptr->fClipU		= ImageUMax ( iImageID );
	m_ptr->fClipV		= ImageVMax ( iImageID );
	m_ptr->iFrameCount  = iCount;								// number of frames
	m_ptr->iFrameAcross = iWidth;	// get the number of frames across
	m_ptr->iFrameDown   = iHeight;	// get the number of frames down
	m_ptr->iFrame       = 0;									// starting frame


	// mike - 220604
	m_ptr->iLastFrame = 0;

	// Set first frame to hide all images
	SetSpriteFrameEx ( iID, 1 );

	// 270305 - mike - fixes distortion of sprite image
	SizeSprite ( iID, m_ptr->iFrameWidth, m_ptr->iFrameHeight );
}

DARKSDK void CreateAnimatedSprite ( int iID, char* szImage, int iWidth, int iHeight, int iImageID )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szImage);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	CheckForWorkshopFile (VirtualFilename);

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	CreateAnimatedSpriteCore ( iID, VirtualFilename, iWidth, iHeight, iImageID );
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
}

DARKSDK void SetSpritePriority ( int iID, int iPriority )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return;
	}

	// Set Priority level
	m_ptr->iPriority = iPriority;
}

//
// COMMAND SET EXPRESSIONS
//

DARKSDK int SpritePriority ( int iID )
{
	// mike - 041005 - return the priority of the sprite

	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return 0;
	}

	return m_ptr->iPriority;
}

DARKSDK int SpriteExist ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	// check if a sprite exists
	if ( UpdateSpritesPtr ( iID ) )
		return 1;

	// return false if it doesn't exist
	return 0;
}

DARKSDK int SpriteX ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the x pos
	return (int)m_ptr->fX;
}

DARKSDK int SpriteY ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the y pos
	return (int)m_ptr->fY;
}

DARKSDK int GetSpriteImage ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the image number
	return m_ptr->iImage;
}

DARKSDK int SpriteWidth ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	if ( m_ptr->iXSize==0 )
		return (int)(m_ptr->iWidth * m_ptr->fXScale);
	else
		return m_ptr->iXSize;
}

DARKSDK int SpriteHeight ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	if ( m_ptr->iYSize==0 )
		return (int)(m_ptr->iHeight * m_ptr->fYScale);
	else
		return m_ptr->iYSize;
}

DARKSDK int SpriteScaleX ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the size
	return (int)(m_ptr->fXScale*100.0f);
}

DARKSDK int SpriteScaleY ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the size
	return (int)(m_ptr->fYScale*100.0f);
}

DARKSDK int SpriteMirrored ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return 0;
	}

	// is the sprite mirrored
	if(m_ptr->bMirrored)
		return 1;
	else
		return 0;
}

DARKSDK int SpriteFlipped ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return 0;
	}

	// is the sprite flipped
	if(m_ptr->bFlipped)
		return 1;
	else
		return 0;
}

DARKSDK int SpriteOffsetX ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the x offset
	return m_ptr->iXOffset;
}

DARKSDK int SpriteOffsetY ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the y offset
	return m_ptr->iYOffset;
}

DARKSDK int SpriteHit ( int iID, int iTarget )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(iTarget<0 || iTarget>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return 0;
	}

	// leefix - 020308 - retain ptr to THIS sprite for hit control (hit was constantly fed back)
	tagSpriteData* m_ptrThis = m_ptr;

	int overlap = CheckSpriteCollision( iID, iTarget );

	int returnhit=0;
	if(overlap>0 && m_ptrThis->iHitoverlapstore==0)
		returnhit = overlap;
	else
		returnhit = 0;

	m_ptrThis->iHitoverlapstore = overlap;

	return returnhit;
}

DARKSDK int SpriteCollision ( int iID, int iTarget )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(iTarget<0 || iTarget>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return 0;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return 0;
	}

	// mike - 021005 - ensure target does not exist
	// mike - 111005 - modify so only check if target is not 0
	if ( iTarget != 0 )
	{
		if(!UpdateSpritesPtr(iTarget))
		{
			RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
			return 0;
		}
	}

	return CheckSpriteCollision( iID, iTarget );
}

DARKSDK float SpriteAngle ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the angle of a sprite
	return m_ptr->fAngle;
}

DARKSDK int SpriteAlpha ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the alpha of a sprite
	return m_ptr->iAlpha;
}

DARKSDK int SpriteRed ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the red colour
	return m_ptr->iRed;
}

DARKSDK int SpriteGreen ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the green colour
	return m_ptr->iGreen;
}

DARKSDK int SpriteBlue ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// return the blue colour
	return m_ptr->iBlue;
}

DARKSDK int SpriteFrame ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the frame of animation
	if ( m_ptr->eAnimType == ONE_IMAGE )
	{
		// mike - 220604 - return correct frame
		// leefix - 020308 - added anim detection and regular manual frame value return (for non playing sprites)
		if ( m_ptr->bIsAnim==true )
	 		return m_ptr->iLastFrame + 1;
		else
	 		return m_ptr->iFrame + 1;
	}
	else
		return m_ptr->iImage;
	
}

DARKSDK int SpriteVisible   ( int iID )
{
	if(iID<=0 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_SPRITEILLEGALNUMBER);
		return -1;
	}

	if(!UpdateSpritesPtr(iID))
	{
		RunTimeError(RUNTIMEERROR_SPRITENOTEXIST);
		return -1;
	}

	// get the alpha of a sprite
	if(m_ptr->bVisible==true)
		return 1;
	else
		return 0;
}

DARKSDK void SetSpriteFilterMode ( int iMode )
{
	// mike - 071005 - filter mode in case you want to use linear
	// lee - 090910 - also controls whether WRAP(0-default) or 1(CLAMP)
    m_SpriteManager.SetFilterMode( iMode );
}

//////////////////////////////////////////////////////////////////////////////////
// DARK SDK SECTION //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

#ifdef DARKSDK_COMPILE

void ConstructorSprites ( HINSTANCE hSetup, HINSTANCE hImage )
{
	Constructor ( hSetup, hImage );
}

void DestructorSprites ( void )
{
	Destructor ( );
}

void SetErrorHandlerSprites	( LPVOID pErrorHandlerPtr )
{
	SetErrorHandler ( pErrorHandlerPtr );
}

void PassCoreDataSprites ( LPVOID pGlobPtr )
{
	PassCoreData ( pGlobPtr );
}

void RefreshGRAFIXSprites ( int iMode )
{
	RefreshGRAFIX ( iMode );
}

void UpdateSprites ( void )
{
	Update ( );
}

void UpdateAllSpritesSprites (void)
{
	UpdateAllSprites ( );
}

void dbSetSprite ( int iID, int iBacksave, int iTransparent )
{
	SetSprite ( iID, iBacksave, iTransparent );
}

void dbSprite ( int iID, int iX, int iY, int iImage )
{
	Sprite (  iID,  iX,  iY,  iImage );
}

void dbPasteSprite ( int iID, int iX, int iY )
{
	Paste (  iID, iX,  iY );
}
	
void dbSizeSprite ( int iID, int iXSize, int iYSize )
{
	Size (  iID,  iXSize,  iYSize );
}

void dbScaleSprite ( int iID, float fScale )
{
	Scale (  iID,  fScale );
}
 
void dbStretchSprite ( int iID, int iXStretch, int iYStretch )
{
	Stretch (  iID,  iXStretch,  iYStretch );
}

void dbMirrorSprite ( int iID )
{
	Mirror (  iID );
}
 
void dbFlipSprite ( int iID )
{
	Flip ( iID );
}

void dbOffsetSprite ( int iID, int iXOffset, int iYOffset )
{
	Offset (  iID,  iXOffset,  iYOffset );
}
	
void dbHideSprite ( int iID )
{
	Hide ( iID );
}
 
void dbShowSprite ( int iID )
{
	Show ( iID );
}
 
void dbHideAllSprites ( void )
{
	HideAllSprites ( );
}
 
void dbShowAllSprites ( void )
{
	ShowAllSprites ( );
}
 
void dbDeleteSprite ( int iID )
{
	Delete ( iID );
}

void dbMoveSprite ( int iID, float velocity )
{
	Move (  iID,  velocity );
}
 
void dbRotateSprite ( int iID, float fRotate )
{
	Rotate (  iID,  fRotate );
}
 
void dbSetSpriteImage ( int iID, int iImage )
{
	SetImage (  iID,  iImage );
}
 
void dbSetSpriteAlpha ( int iID, int iValue )
{
	SetAlpha (  iID,  iValue );
}
 
void dbSetSpriteDiffuse ( int iID, int iR, int iG, int iB )
{
	SetDiffuse (  iID,  iR,  iG,  iB );
}

void dbPlaySprite ( int iID, int iStart, int iEnd, int iDelay )
{
	Play (  iID,  iStart,  iEnd,  iDelay );
}
 
void dbSetSpriteFrame ( int iID, int iFrame )
{
	SetFrame (  iID,  iFrame );
}
 
void dbSetSpriteTextureCoordinates ( int iID, int iVertex, float tu, float tv )
{
	SetTextureCoordinates (  iID,  iVertex,  tu,  tv );
}
 
void dbCreateAnimatedSprite ( int iID, char* szImage, int iWidth, int iHeight, int iImageID )
{
	CreateAnimatedSprite (  iID,  szImage,  iWidth,  iHeight, iImageID );
}

void dbCloneSprite ( int iID, int iDestinationID )
{
	Clone (  iID,  iDestinationID );
}
 
void dbSetSpritePriority ( int iID, int iPriority )
{
	SetPriority (  iID,  iPriority );
}

int dbSpriteExist ( int iID )
{
	return GetExist ( iID );
}
 
int dbSpriteX ( int iID )
{
	return GetX ( iID );
}
 
int dbSpriteY ( int iID )
{
	return GetY ( iID );
}
 
int dbSpriteImage ( int iID )
{
	return GetImage ( iID );
}
 
int dbSpriteWidth ( int iID )
{
	return GetWidth ( iID );
}
 
int dbSpriteHeight ( int iID )
{
	return GetHeight ( iID );
}
 
int dbSpriteScaleX ( int iID )
{
	return GetXScale ( iID );
}
 
int dbSpriteScaleY ( int iID )
{
	return GetYScale ( iID );
}
 
int dbSpriteMirrored ( int iID )
{
	return GetMirrored ( iID );
}
 
int dbSpriteFlipped ( int iID )
{
	return GetFlipped ( iID );
}
 
int dbSpriteOffsetX ( int iID )
{
	return GetXOffset ( iID );
}
 
int dbSpriteOffsetY ( int iID )
{
	return GetYOffset ( iID );
}
 
int dbSpriteHit ( int iID, int iTarget )
{
	return GetHit (  iID,  iTarget );
}
 
int dbSpriteCollision ( int iID, int iTarget )
{
	return GetCollision (  iID,  iTarget );
}

float dbSpriteAngle ( int iID )
{
	DWORD dwReturn = GetAngle ( iID );
	
	return *( float* ) &dwReturn;
}
 
int dbSpriteAlpha ( int iID )
{
	return GetAlpha ( iID );
}
 
int dbSpriteRed ( int iID )
{
	return GetRed ( iID );
}
 
int dbSpriteGreen ( int iID )
{
	return GetGreen ( iID );
}
 
int dbSpriteBlue ( int iID )
{
	return GetBlue ( iID );
}
 
int dbSpriteFrame ( int iID )
{
	return GetFrame ( iID );
}
 
int dbSpriteVisible ( int iID )
{
	return GetVisible ( iID );
}

void dbPasteImage ( int iImageID, int iX, int iY, float fU, float fV )
{
	PasteImage ( iImageID, iX, iY, fU, fV );
}

void dbPasteImageEx ( int iImageID, int iX, int iY, float fU, float fV, int iTransparent )
{
	PasteImageEx ( iImageID, iX, iY, fU, fV, iTransparent );
}

void dbPasteTextureToRect ( LPGGTEXTURE pTexture, float fU, float fV, RECT Rect )
{
	PasteTextureToRect ( pTexture, fU, fV, Rect );
}

void dbSaveSpriteBack ( void )
{
	SaveBack ( );
}

void dbRestoreSpriteBack ( void )
{
	RestoreBack ( );
}

// lee - 300706 - GDK fixes
void dbScaleSprite ( int iID, int iScale ) { dbScaleSprite ( iID, (float)iScale ); }
void dbSetSpriteTextureCoord ( int iID, int iVertex, float tu, float tv ) { dbSetSpriteTextureCoordinates ( iID, iVertex, tu, tv ); }

#endif

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
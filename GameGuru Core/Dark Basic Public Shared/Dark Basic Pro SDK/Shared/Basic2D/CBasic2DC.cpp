// INCLUDES / LIBS ///////////////////////////////////////////////////////////////
#include <time.h>
#include "cbasic2Dc.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"
#include "CGfxC.h"

#ifdef DX11

// Globals
bool							m_bIsLocked = false;
LPGGSURFACE						m_pSurface = NULL;
DWORD*							m_pData = NULL;
DWORD							m_LockedRectPitch = 0;
int								m_SurfaceWidth = 0;
int								m_SurfaceHeight = 0;
int								m_iBRed = 0;
int								m_iBGreen = 0;
int								m_iBBlue = 0;
int								m_iFRed = 0;
int								m_iFGreen = 0;
int								m_iFBlue = 0;

// Externs
extern GlobStruct*				g_pGlob;
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
extern LPGGSURFACE				g_pBackBuffer;
extern LPGGRENDERTARGETVIEW		m_pRenderTargetView;

// Basic2D drawing functions

void Basic2DConstructor ( void )
{
}

void Basic2DRefreshGRAFIX ( int iMode )
{
}

DARKSDK DWORD GetPixelPtr ( void )
{
	return (DWORD)m_pData;
}

DARKSDK DWORD GetPixelPitch ( void )
{
	return (DWORD)m_LockedRectPitch;
}

DARKSDK void LockPixels ( void )
{
	if ( m_bIsLocked==false && g_pGlob->pCurrentBitmapSurface )
	{
		// Ensure no previous staging surface created
		SAFE_RELEASE(m_pSurface);
		m_pData = NULL;

		// get actual dimensions of image
		GGSURFACE_DESC imageddsd;
		LPGGSURFACE pTextureInterface = NULL;
		g_pGlob->pCurrentBitmapSurface->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
		pTextureInterface->GetDesc(&imageddsd);
		SAFE_RELEASE ( pTextureInterface );

		// Copy to identically sized staging texture
		GGSURFACE_DESC TempTextureDesc = { imageddsd.Width, imageddsd.Height, 1, 1, imageddsd.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0 };
		m_pD3D->CreateTexture2D( &TempTextureDesc, NULL, &m_pSurface );
		if ( m_pSurface )
		{
			// copy over contents of bitmap image
			m_pImmediateContext->CopyResource ( m_pSurface, g_pGlob->pCurrentBitmapSurface );

			// lock staging for read/write
			GGLOCKED_RECT d3dlock;
			if(SUCCEEDED(m_pImmediateContext->Map(m_pSurface, 0, D3D11_MAP_READ_WRITE, 0, &d3dlock)))
			{
				m_pData = (DWORD*)d3dlock.pData;
				m_LockedRectPitch = d3dlock.RowPitch;
				m_SurfaceWidth=(int)imageddsd.Width;
				m_SurfaceHeight=(int)imageddsd.Height;
			}
		}
		m_bIsLocked = true;
	}
}

DARKSDK void UnlockPixels ( void )
{
	if ( m_bIsLocked==true )
	{
		if ( m_pSurface ) 
		{ 
			m_pImmediateContext->Unmap ( m_pSurface, 0 ); 
			m_pImmediateContext->CopyResource ( g_pGlob->pCurrentBitmapSurface, m_pSurface );
			SAFE_RELEASE(m_pSurface); 
		}
		m_bIsLocked = false;
		m_pData = NULL;
	}
}

void CLS ( DWORD RGBBackColor )
{
	if (!g_pGlob->pCurrentBitmapSurfaceView) return;
	// Not used, prefer CLS(r,g,b)
	g_pGlob->iCursorX=0;
	g_pGlob->iCursorY=0;
	float ClearColor[4];
	ClearColor[0] = 0.10f;
	ClearColor[1] = 0.10f;//25f;
	ClearColor[2] = 0.10f;//50f;
	ClearColor[3] = 0.00f;
	m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentBitmapSurfaceView, ClearColor);
}

DARKSDK void CLS ( int iRed, int iGreen, int iBlue )
{
	if (!g_pGlob->pCurrentBitmapSurfaceView) return;
	// Reset cursor for debug text
	g_pGlob->iCursorX=0;
	g_pGlob->iCursorY=0;

	// Assign Colours
	m_iBRed=iRed;
	m_iBGreen=iGreen;
	m_iBBlue=iBlue;

	float ClearColor[4];
	ClearColor[0] = (float)m_iBRed / 255.0f;
	ClearColor[1] = (float)m_iBGreen / 255.0f;
	ClearColor[2] = (float)m_iBBlue / 255.0f;
	ClearColor[3] = 1.0f;
	m_pImmediateContext->ClearRenderTargetView(g_pGlob->pCurrentBitmapSurfaceView, ClearColor);
}

void Ink ( DWORD RGBForeColor, DWORD RGBBackColor )
{
	g_pGlob->dwForeColor = RGBForeColor;
	g_pGlob->dwBackColor = RGBBackColor;
}

void Ink ( int iRedF, int iGreenF, int iBlueF, int iRedB, int iGreenB, int iBlueB )
{
	// Assign Colours
	m_iBRed=iRedB;
	m_iBGreen=iGreenB;
	m_iBBlue=iBlueB;
	m_iFRed=iRedF;
	m_iFGreen=iGreenF;
	m_iFBlue=iBlueF;

	// Assign COLORREF
	g_pGlob->dwForeColor = GGCOLOR_ARGB( 255, m_iFRed, m_iFGreen, m_iFBlue );
	g_pGlob->dwBackColor = GGCOLOR_ARGB( 255, m_iBRed, m_iBGreen, m_iBBlue );
}

DARKSDK void PlotCore( int iX, int iY, DWORD Color )
{
	// modified core so that 16 bit colours are supported
	DWORD dwAlpha = Color >> 24;
	DWORD dwRed   = ((( Color ) >> 16 ) & 0xff );
	DWORD dwGreen = ((( Color ) >>  8 ) & 0xff );
	DWORD dwBlue  = ((( Color ) )       & 0xff );

	// allow to write over screen boundaries
	if(iX<0) return;
	if(iY<0) return;

	// return code with additional bitmap specific check
	if ( g_pGlob->iCurrentBitmapNumber==0 )
	{
		if ( iX >= g_pGlob->iScreenWidth ) return;
		if ( iY >= g_pGlob->iScreenHeight ) return;
	}

	// ensure we're not plotting outside of surce			
	if ( iX >= m_SurfaceWidth )
		return;

	if ( iY >= m_SurfaceHeight )
		return;

	// and write data to locked memory
	m_pData [ iX + iY*m_LockedRectPitch / 4 ] = Color;
}

void Dot ( int iX, int iY, DWORD dwColor )
{
	// allow to write over screen boundaries
	if(iX<0) return;
	if(iY<0) return;

	// return code with additional bitmap specific check
	if ( g_pGlob->iCurrentBitmapNumber==0 )
	{
	 	if(iX>g_pGlob->iScreenWidth) return; 
		if(iY>g_pGlob->iScreenHeight) return;
	}

	if ( m_bIsLocked )
	{
		PlotCore(iX,iY,dwColor);
	}
	else
	{
		LockPixels ( );
		PlotCore(iX,iY,dwColor);
		UnlockPixels ( );
	}
}

void Line ( int iXa, int iYa, int iXb, int iYb )
{
}

void Box ( int iLeft, int iTop, int iRight, int iBottom )
{
}

/*
void BoxGradient ( int iLeft, int iTop, int iRight, int iBottom, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4 )
{
}
*/

void CopyArea ( int iDestX, int iDestY, int iWidth, int iHeight, int iSourceX, int iSourceY )
{
}

DARKSDK DWORD Rgb ( int iRed, int iGreen, int iBlue )
{
	return GGCOLOR_XRGB(iRed, iGreen, iBlue);
}

DARKSDK int RgbR ( DWORD iRGB )
{
	return (int)((iRGB & 0x00FF0000) >> 16);
}

DARKSDK int RgbG ( DWORD iRGB )
{
	return (int)((iRGB & 0x0000FF00) >> 8);
}

DARKSDK int RgbB ( DWORD iRGB )
{
	return (int)((iRGB & 0x000000FF) );
}

DARKSDK DWORD GetPlotCore( int iX, int iY )
{
	if ( m_pData ) 
	{
		if(iX<0) return 0;
		if(iY<0) return 0;
		if ( g_pGlob->iCurrentBitmapNumber==0 )
		{
			if ( iX >= g_pGlob->iScreenWidth ) return 0;
			if ( iY >= g_pGlob->iScreenHeight ) return 0;
		}
		if ( iX >= m_SurfaceWidth ) return 0;
		if ( iY >= m_SurfaceHeight ) return 0;
		return m_pData [ iX + iY*m_LockedRectPitch / 4 ];
	}
	else
		return 0;
}

DARKSDK DWORD GetPoint ( int iX, int iY )
{
	DWORD dwColor=0;
	if ( m_bIsLocked )
	{
		dwColor=GetPlotCore(iX,iY);
	}
	else
	{
		LockPixels ( );
		dwColor=GetPlotCore(iX,iY);
		UnlockPixels ( );
	}
	return dwColor;
}


#else

// GLOBALS 
extern GlobStruct*							g_pGlob;
extern LPGGDEVICE							m_pD3D;
DBPRO_GLOBAL bool							m_bIsLocked;
DBPRO_GLOBAL GGLOCKED_RECT					m_LockedRect;
DBPRO_GLOBAL LPGGSURFACE					m_pSurface;
DBPRO_GLOBAL DWORD*							m_pData;
DBPRO_GLOBAL int							m_SurfaceWidth;
DBPRO_GLOBAL int							m_SurfaceHeight;
DBPRO_GLOBAL LPGGVERTEXBUFFER				m_pLineVB;
DBPRO_GLOBAL DWORD							m_dwBPP;
DBPRO_GLOBAL int							m_iBRed;
DBPRO_GLOBAL int							m_iBGreen;
DBPRO_GLOBAL int							m_iBBlue;
DBPRO_GLOBAL int							m_iFRed;
DBPRO_GLOBAL int							m_iFGreen;
DBPRO_GLOBAL int							m_iFBlue;

// INTERNAL FUNCTIONS ////////////////////////////////////////////////////////////

DARKSDK int GetBasic2DBitDepthFromFormat(GGFORMAT Format)
{
	#ifdef DX11
	switch(Format)
	{
		case GGFMT_A8R8G8B8 : return 32; break;
	}
	#else
	switch(Format)
	{
		case GGFMT_R8G8B8 :		return 24;	break;
		case GGFMT_A8R8G8B8 :		return 32;	break;
		case GGFMT_X8R8G8B8 :		return 32;	break;
		case GGFMT_R5G6B5 :		return 16;	break;
		case GGFMT_X1R5G5B5 :		return 16;	break;
		case GGFMT_A1R5G5B5 :		return 16;	break;
		case GGFMT_A4R4G4B4 :		return 16;	break;
		case GGFMT_A8	:			return 8;	break;
		case GGFMT_R3G3B2 :		return 8;	break;
		case GGFMT_A8R3G3B2 :		return 16;	break;
		case GGFMT_X4R4G4B4 :		return 16;	break;
		case GGFMT_A2B10G10R10 :	return 32;	break;
		case GGFMT_G16R16 :		return 32;	break;
		case GGFMT_A8P8 :			return 8;	break;
		case GGFMT_P8 :			return 8;	break;
		case GGFMT_L8 :			return 8;	break;
		case GGFMT_A8L8 :			return 16;	break;
		case GGFMT_A4L4 :			return 8;	break;
	}
	#endif
	return 0;
}

DARKSDK void UpdateBPP(void)
{
	// Update BPP based on draw depth
	D3DSURFACE_DESC ddsd;
	g_pGlob->pCurrentBitmapSurface->GetDesc(&ddsd);
	m_dwBPP=GetBasic2DBitDepthFromFormat(ddsd.Format)/8;
}

DARKSDK void Basic2DConstructor ( void )
{
	m_bIsLocked = false;
	m_pSurface  = NULL;
	m_pData     = NULL;
	m_pLineVB   = NULL;
	memset ( &m_LockedRect, 0, sizeof ( m_LockedRect ) );

	#ifdef DX11
	#else
	if ( m_pD3D ) m_pD3D->CreateVertexBuffer(	6*sizeof(tagLinesVertexDesc), D3DUSAGE_WRITEONLY, D3DFVF_LINES, D3DPOOL_DEFAULT, &m_pLineVB, NULL);
	#endif
}

DARKSDK void Basic2DDestructor ( void )
{
	SAFE_RELEASE(m_pLineVB);
}

DARKSDK void SetBasic2DErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void PassBasic2DCoreData( LPVOID pGlobPtr )
{
	// Get BPP
	UpdateBPP();
}

DARKSDK void Basic2DRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		Basic2DDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		Basic2DConstructor (  );
		PassBasic2DCoreData ( NULL );
	}
}

DARKSDK void CLS ( int iRed, int iGreen, int iBlue )
{
	// Assign Colours
	m_iBRed=iRed;
	m_iBGreen=iGreen;
	m_iBBlue=iBlue;

	// deliverately left to trace which parts of GG are using Basic2D
	m_pD3D->Clear ( 1, NULL, D3DCLEAR_TARGET, GGCOLOR_ARGB( 255, m_iBRed, m_iBGreen, m_iBBlue ), 0.0f, 0 );
}

DARKSDK void Ink ( int iRedF, int iGreenF, int iBlueF, int iRedB, int iGreenB, int iBlueB )
{
	// Assign Colours
	m_iBRed=iRedB;
	m_iBGreen=iGreenB;
	m_iBBlue=iBlueB;
	m_iFRed=iRedF;
	m_iFGreen=iGreenF;
	m_iFBlue=iBlueF;

	// Assign COLORREF
	g_pGlob->dwForeColor = GGCOLOR_ARGB( 255, m_iFRed, m_iFGreen, m_iFBlue );
	g_pGlob->dwBackColor = GGCOLOR_ARGB( 255, m_iBRed, m_iBGreen, m_iBBlue );
}

#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))

DARKSDK USHORT RGB16Bit565(int r, int g, int b)
{
	r>>=3; g>>=2; b>>=3;
	return(_RGB16BIT565((r),(g),(b)));
} 

DARKSDK void PlotCore( int iX, int iY, DWORD Color )
{
	// mike - 230604 - modified core so that 16 bit colours are supported
	DWORD dwAlpha = Color >> 24;
	DWORD dwRed   = ((( Color ) >> 16 ) & 0xff );
	DWORD dwGreen = ((( Color ) >>  8 ) & 0xff );
	DWORD dwBlue  = ((( Color ) )       & 0xff );

	// mike - 280305 - allow to write over screen boundaries
	if(iX<0) return;
	if(iY<0) return;

	// leefix - 310305 - return code with additional bitmap specific check
	if ( g_pGlob->iCurrentBitmapNumber==0 )
	{
		if ( iX >= g_pGlob->iScreenWidth ) return;
		if ( iY >= g_pGlob->iScreenHeight ) return;
	}

	// mike - 021005 - ensure we're not plotting outside of surce			
	if ( iX >= m_SurfaceWidth )
		return;

	if ( iY >= m_SurfaceHeight )
		return;

	switch(m_dwBPP)
	{
		// mike - 250604
		case 2 : *((WORD*)m_pData + iX + iY * m_LockedRect.Pitch / 2 ) = RGB16Bit565 ( dwRed, dwGreen, dwBlue );	break;
		case 4 : m_pData [ iX + iY * m_LockedRect.Pitch / 4 ] = Color;	break;
	}
}

DARKSDK DWORD GetPlotCore( int iX, int iY )
{
	// lee - 170206 - u60 - performance hit sure, but safest method
	if(iX<0) return 0;
	if(iY<0) return 0;
	if ( g_pGlob->iCurrentBitmapNumber==0 )
	{
		if ( iX >= g_pGlob->iScreenWidth ) return 0;
		if ( iY >= g_pGlob->iScreenHeight ) return 0;
	}
	if ( iX >= m_SurfaceWidth ) return 0;
	if ( iY >= m_SurfaceHeight ) return 0;

	switch(m_dwBPP)
	{
		case 2: return *((WORD*)m_pData + iX + iY * m_LockedRect.Pitch / 2 );
		case 4: return m_pData [ iX + iY*m_LockedRect.Pitch / 4 ];
	}
	return 0;
}

DARKSDK void Write ( int iX, int iY, DWORD Color )
{
	if(iX>=0 && iY>=0 && iX<=m_SurfaceWidth && iY<=m_SurfaceHeight)
		PlotCore(iX,iY,Color);
}

DARKSDK void WriteCirclePoints ( int iCX, int iCY, int x, int y, DWORD Color )
{
	Write ( iCX+x, iCY+y, Color );
	Write ( iCX+y, iCY+x, Color );

	Write ( iCX+y, iCY-x, Color );
	Write ( iCX+x, iCY-y, Color );

	Write ( iCX-x, iCY-y, Color );
	Write ( iCX-y, iCY-x, Color );

	Write ( iCX-y, iCY+x, Color );
	Write ( iCX-x, iCY+y, Color );
}

DARKSDK void WriteEllipsePoints ( int iCX, int iCY, int x, int y, DWORD Color )
{
	Write ( iCX+x, iCY+y, Color );
	Write ( iCX+x, iCY-y, Color );
	Write ( iCX-x, iCY-y, Color );
	Write ( iCX-x, iCY+y, Color );
}

DARKSDK void Clear ( DWORD RGBColor )
{
	m_pD3D->Clear ( 1, NULL, D3DCLEAR_TARGET, RGBColor, 0.0f, 0 );
}

DARKSDK bool GetLockable ( void )
{
	return false;
}

DARKSDK void CopyArea(int iDestX, int iDestY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	// deliverately left to trace which parts of GG are using Basic2D

	// get surface desc
	D3DSURFACE_DESC surfacedesc;
	IGGSurface* pSurface = g_pGlob->pCurrentBitmapSurface;
	pSurface->GetDesc ( &surfacedesc );

	// Define areas to copy
	RECT  rcSource = {  iSourceX,  iSourceY, iSourceX+iWidth,  iSourceY+iHeight };

	// Clip to actual surface source size
	int iScrWidth=surfacedesc.Width;
	int iScrHeight=surfacedesc.Height;
	if(rcSource.right>iScrWidth) rcSource.right=iScrWidth;
	if(rcSource.bottom>iScrHeight) rcSource.bottom=iScrHeight;

	// Define destination rect
	RECT rcDestRect;
	rcDestRect.left=iDestX;
	rcDestRect.top=iDestY;
	rcDestRect.right=iDestX+(rcSource.right-rcSource.left);
	rcDestRect.bottom=iDestX+(rcSource.bottom-rcSource.top);

	// Perform copy
	if(m_pD3D)
	{
		// source and dest surface
		if ( pSurface )
		{
			// create temp surface
			IGGSurface* pTempSurface = NULL;
			m_pD3D->CreateRenderTarget ( surfacedesc.Width, surfacedesc.Height, surfacedesc.Format, D3DMULTISAMPLE_NONE, 0, TRUE, &pTempSurface, NULL );
			if ( pTempSurface )
			{
				// copy over
				m_pD3D->StretchRect(pSurface, NULL, pTempSurface, NULL, GGTEXF_NONE);
				m_pD3D->StretchRect(pTempSurface, &rcSource, pSurface, &rcDestRect, GGTEXF_NONE);

				// free surface
				SAFE_RELEASE ( pTempSurface );
			}
		}
	}
}

DARKSDK void CLS ( void )
{
	// deliverately left to trace which parts of GG are using Basic2D
	m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET, g_pGlob->dwBackColor, 0.0f, 0 );
}

DARKSDK void CLS ( DWORD RGBBackColor )
{
	// deliverately left to trace which parts of GG are using Basic2D
	g_pGlob->iCursorX=0;
	g_pGlob->iCursorY=0;
	m_pD3D->Clear ( 0, NULL, D3DCLEAR_TARGET, RGBBackColor, 0.0f, 0 );
}

DARKSDK void Ink ( DWORD RGBForeColor, DWORD RGBBackColor )
{
	g_pGlob->dwForeColor = RGBForeColor;
	g_pGlob->dwBackColor = RGBBackColor;
}

DARKSDK DWORD CorrectDotColor ( DWORD dwCol )
{
	// mike - 260604 - return original colour
	return dwCol;

	if(m_dwBPP==2)
	{
		int red =	(int)(((dwCol & (255<<16)) >> 16) / 8.3);
		int green = (int)(((dwCol & (255<<8)) >> 8) / 4.1);
		int blue =	(int)((dwCol & 255) / 8.3);
		if(red>31) red=31;
		if(green>63) green=63;
		if(blue>31) blue=31;
		dwCol = (red<<11)+(green<<5)+(blue);
	}
	return dwCol;
}

DARKSDK void Dot ( int iX, int iY )
{
	DWORD dwCol = CorrectDotColor ( g_pGlob->dwForeColor );

	// mike - 280305 - allow to write over screen boundaries
	if(iX<0) return;
	if(iY<0) return;

	// leefix - 310305 - return code with additional bitmap specific check
	if ( g_pGlob->iCurrentBitmapNumber==0 )
	{
	 	if(iX>g_pGlob->iScreenWidth) return; 
		if(iY>g_pGlob->iScreenHeight) return;
	}

	if ( m_bIsLocked )
	{
		PlotCore(iX,iY,dwCol);
	}
	else
	{
		LockPixels ( );
		PlotCore(iX,iY,dwCol);
		UnlockPixels ( );
	}
}

DARKSDK void Box ( int iLeft, int iTop, int iRight, int iBottom )
{
	// deliverately left to trace which parts of GG are using Basic2D
	tagLinesVertexDesc * v = 0;

    if (m_pLineVB)
    	m_pLineVB->Lock( 0, 0, (void**)&v, 0 );

    if (v)
	{
		v[0].x      = (float)iLeft;
        v[0].y      = (float)iBottom;
		v[0].z      = 10.0f;
        v[0].rhw    = 1.0f;
		v[0].dwColour  = g_pGlob->dwForeColor;

		v[1].x      = (float)iLeft;
        v[1].y      = (float)iTop;
		v[1].z      = 10.0f;
        v[1].rhw    = 1.0f;
		v[1].dwColour  = g_pGlob->dwForeColor;

		v[2].x      = (float)iRight;
        v[2].y      = (float)iTop;
		v[2].z      = 10.0f;
        v[2].rhw    = 1.0f;
		v[2].dwColour  = g_pGlob->dwForeColor;

		v[3].x      = (float)iRight;
        v[3].y      = (float)iBottom;
		v[3].z      = 10.0f;
        v[3].rhw    = 1.0f;
		v[3].dwColour  = g_pGlob->dwForeColor;

		m_pLineVB->Unlock();

		m_pD3D->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		m_pD3D->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );		

		m_pD3D->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pD3D->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pD3D->SetRenderState( D3DRS_ZENABLE, FALSE );

		m_pD3D->SetRenderState( D3DRS_DITHERENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_LIGHTING, FALSE );
		m_pD3D->SetRenderState( D3DRS_COLORVERTEX, TRUE );
		m_pD3D->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		m_pD3D->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

		m_pD3D->SetRenderState( D3DRS_FOGENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

        DWORD AAEnabled = FALSE;
        m_pD3D->GetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, &AAEnabled );
        if (AAEnabled)
		    m_pD3D->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
		
		m_pD3D->SetVertexShader( NULL  );
		m_pD3D->SetFVF( D3DFVF_LINES  );
		m_pD3D->SetStreamSource( 0, m_pLineVB, 0, sizeof(tagLinesVertexDesc) );

		m_pD3D->SetTexture( 0, NULL );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP, GGTOP_SELECTARG1 );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG2, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP, GGTOP_SELECTARG1 );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, GGTA_DIFFUSE );

		m_pD3D->DrawPrimitive( GGPT_TRIANGLEFAN, 0 ,2 );

        if (AAEnabled)
		    m_pD3D->SetRenderState ( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	}
}

DARKSDK void Line ( int iXa, int iYa, int iXb, int iYb )
{
	// deliverately left to trace which parts of GG are using Basic2D
	if(m_pLineVB)
	{
		// fill data for line
		tagLinesVertexDesc * lines;
		m_pLineVB->Lock( 0, 0, (void**)&lines, 0 );
		lines [ 0 ].x        = (float)iXa;										// start x point
		lines [ 0 ].y        = (float)iYa;										// start y point
		lines [ 0 ].z        = 0;												// keep this as 0 because we want 2D only
		lines [ 0 ].rhw		 = 1.0f;
		lines [ 0 ].dwColour = g_pGlob->dwForeColor;
		lines [ 1 ].x        = (float)iXb;										// end x point
		lines [ 1 ].y        = (float)iYb;										// end y point
		lines [ 1 ].z        = 0;												// keep this as 0 because we want 2D only
		lines [ 1 ].rhw		 = 1.0f;
		lines [ 1 ].dwColour = g_pGlob->dwForeColor;
		m_pLineVB->Unlock();

		// draw the line
		m_pD3D->SetTexture( 0, NULL  );
		m_pD3D->SetTexture( 1, NULL  );

		// DX8->DX9
		m_pD3D->SetVertexShader( NULL  );
		m_pD3D->SetFVF( D3DFVF_LINES  );
		m_pD3D->SetStreamSource( 0, m_pLineVB, 0, sizeof(tagLinesVertexDesc) );

		m_pD3D->SetRenderState( D3DRS_FOGENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

		m_pD3D->DrawPrimitive( D3DPT_LINELIST, 0 , 1);
	}
}

DARKSDK void Circle ( int iX, int iY, int iRadius )
{
	LockPixels ( );
	if ( m_bIsLocked )
	{
		int x = 0;
		int y = iRadius;
		int d = 1 - iRadius;

		WriteCirclePoints ( iX, iY, x, y, g_pGlob->dwForeColor );

		while ( y > x )
		{
			if ( d < 0 )
				d += 2 * x;// + 3;
			else
			{
				d += 2 * ( x - y );// + 5;
				y--;
			}

			x++;

			WriteCirclePoints ( iX, iY, x, y, g_pGlob->dwForeColor );
		}
	}
	UnlockPixels ( );
}

DARKSDK void Elipse ( int iX, int iY, int iXRadius, int iYRadius )
{
	LockPixels ( );
	if ( m_bIsLocked )
	{
		int x = 0;
		int y = iYRadius;
		double a = 0;
		while(a<2.0)
		{
			double ix = sin(a)*iXRadius;
			double iy = cos(a)*iYRadius;
			int newx=(int)ix;
			int newy=(int)iy;
			while(newx!=x || newy!=y)
			{
				if(newx<x) x--;
				if(newx>x) x++;
				if(newy<y) y--;
				if(newy>y) y++;
				WriteEllipsePoints ( iX, iY, x, y, g_pGlob->dwForeColor );
			}
			
			// mike - 230604 - stop missing pixels
			a+=0.001;
		}
	}
	UnlockPixels ( );
}

DARKSDK DWORD Rgb ( int iRed, int iGreen, int iBlue )
{
	return GGCOLOR_XRGB(iRed, iGreen, iBlue);
}

DARKSDK int RgbR ( DWORD iRGB )
{
	return (int)((iRGB & 0x00FF0000) >> 16);
}

DARKSDK int RgbG ( DWORD iRGB )
{
	return (int)((iRGB & 0x0000FF00) >> 8);
}

DARKSDK int RgbB ( DWORD iRGB )
{
	return (int)((iRGB & 0x000000FF) );
}

DARKSDK DWORD GetPoint ( int iX, int iY )
{
	// deliverately left to trace which parts of GG are using Basic2D
	DWORD dwColor=0;
	if ( m_bIsLocked )
	{
		dwColor=GetPlotCore(iX,iY);
	}
	else
	{
		LockPixels ( );
		dwColor=GetPlotCore(iX,iY);
		UnlockPixels ( );
	}

	int red, green, blue;
	switch(m_dwBPP)
	{
		case 2 :	// Split 16bit(5-6-5) components
					red =		(int)((	((dwColor>>11)	& 31) ) * 8.3);
					green =		(int)((	((dwColor>>5)	& 63) ) * 4.1);
					blue =		(int)((	((dwColor)		& 31) ) * 8.3);
					if(red>255) red=255;
					if(green>255) green=255;
					if(blue>255) blue=255;
					dwColor = GGCOLOR_XRGB( red, green, blue );
					break;
	}

	// Combine to create final colour value
	return dwColor;
}

DARKSDK void BoxGradient( int iLeft, int iTop, int iRight, int iBottom, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4 )
{
	// deliverately left to trace which parts of GG are using Basic2D
	// Create a box with a gradient fade from one colour to another
	// using the per vertex colour rendering technique :)
    float CentreX = (iLeft + iRight) / 2.0f;
    float CentreY = (iTop + iBottom) / 2.0f;

    #define BG_GetAlpha(c) (((c) >> 24) & 0xff)
    #define BG_GetRed(c)   (((c) >> 16) & 0xff)
    #define BG_GetGreen(c) (((c) >> 8)  & 0xff)
    #define BG_GetBlue(c)  ( (c)        & 0xff)

    // Note: The +2 allows colour to be rounded to the nearest match.
    //       If not used, the central colour would tend to be a little too dark.
    DWORD CentreA = (BG_GetAlpha(dw1) + BG_GetAlpha(dw2) + BG_GetAlpha(dw3) + BG_GetAlpha(dw4) + 2) / 4;
    DWORD CentreR = (BG_GetRed(dw1)   + BG_GetRed(dw2)   + BG_GetRed(dw3)   + BG_GetRed(dw4)   + 2) / 4;
    DWORD CentreG = (BG_GetGreen(dw1) + BG_GetGreen(dw2) + BG_GetGreen(dw3) + BG_GetGreen(dw4) + 2) / 4;
    DWORD CentreB = (BG_GetBlue(dw1)  + BG_GetBlue(dw2)  + BG_GetBlue(dw3)  + BG_GetBlue(dw4)  + 2) / 4;
    DWORD CentreCol = GGCOLOR_ARGB(CentreA, CentreR, CentreG, CentreB);

	tagLinesVertexDesc * v = 0;

    if (m_pLineVB)
    	m_pLineVB->Lock( 0, 0, (void**)&v, 0 );

    if (v)
	{
        // 20091128 v75 - IRM - Fill out the 6 vertices
		v[0].x      = CentreX;
        v[0].y      = CentreY;
		v[0].z      = 10.0f;
        v[0].rhw    = 1.0f;
		v[0].dwColour  = CentreCol;

		v[1].x      = (float)iLeft;
        v[1].y      = (float)iBottom;
		v[1].z      = 10.0f;
        v[1].rhw    = 1.0f;
		v[1].dwColour  = dw1;

		v[2].x      = (float)iLeft;
        v[2].y      = (float)iTop;
		v[2].z      = 10.0f;
        v[2].rhw    = 1.0f;
		v[2].dwColour  = dw2;

		v[3].x      = (float)iRight;
        v[3].y      = (float)iTop;
		v[3].z      = 10.0f;
        v[3].rhw    = 1.0f;
		v[3].dwColour  = dw4;

		v[4].x      = (float)iRight;
        v[4].y      = (float)iBottom;
		v[4].z      = 10.0f;
        v[4].rhw    = 1.0f;
		v[4].dwColour  = dw3;

        v[5].x      = (float)iLeft;
        v[5].y      = (float)iBottom;
		v[5].z      = 10.0f;
        v[5].rhw    = 1.0f;
		v[5].dwColour  = dw1;

		m_pLineVB->Unlock();

		m_pD3D->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		m_pD3D->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );		

		m_pD3D->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		m_pD3D->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		m_pD3D->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		m_pD3D->SetRenderState( D3DRS_ZENABLE, FALSE );

		m_pD3D->SetRenderState( D3DRS_DITHERENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_LIGHTING, FALSE );
		m_pD3D->SetRenderState( D3DRS_COLORVERTEX, TRUE );
		m_pD3D->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		m_pD3D->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

		m_pD3D->SetRenderState( D3DRS_FOGENABLE, FALSE );
		m_pD3D->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

        // Save AA setting, and disable if active
        DWORD AAEnabled = FALSE;
        m_pD3D->GetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, &AAEnabled );
        if (AAEnabled)
		    m_pD3D->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
		
		m_pD3D->SetVertexShader( NULL  );
		m_pD3D->SetFVF( D3DFVF_LINES  );
		m_pD3D->SetStreamSource( 0, m_pLineVB, 0, sizeof(tagLinesVertexDesc) );

		m_pD3D->SetTexture( 0, NULL );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP, GGTOP_SELECTARG1 );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG2, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP, GGTOP_SELECTARG1 );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, GGTA_DIFFUSE );

        // 20091128 v75 - IRM - Render as a fan with 4 polys, not a strip with 2 polys
		m_pD3D->DrawPrimitive( GGPT_TRIANGLEFAN, 0 ,4);

        // Re-enable AA if it was previously enabled
        if (AAEnabled)
		    m_pD3D->SetRenderState ( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	}
}

DARKSDK void Dot ( int iX, int iY, DWORD dwColor )
{
	// LEEFIX - 141102 - Added this conevrter function from above DOT function
	dwColor = CorrectDotColor ( dwColor );
	if ( m_bIsLocked )
	{
		PlotCore(iX,iY,dwColor);
	}
	else
	{
		LockPixels ( );
		PlotCore(iX,iY,dwColor);
		UnlockPixels ( );
	}
}

DARKSDK DWORD GetPixelPtr ( void )
{
	return (DWORD)m_pData;
}

DARKSDK DWORD GetPixelPitch ( void )
{
	return (DWORD)m_LockedRect.Pitch;
}

DARKSDK void LockPixels ( void )
{
	// deliverately left to trace which parts of GG are using Basic2D
	if ( m_bIsLocked==false )
	{
		// Get current render target surface
		m_pSurface = g_pGlob->pCurrentBitmapSurface;
		if(m_pSurface==NULL)
			return;

		if ( FAILED ( m_pSurface->LockRect ( &m_LockedRect, NULL, 0 ) ) )
			return;

		m_pData = ( DWORD* ) ( m_LockedRect.pBits );

		D3DSURFACE_DESC ddsd;
		m_pSurface->GetDesc(&ddsd);
		m_SurfaceWidth=(int)ddsd.Width;
		m_SurfaceHeight=(int)ddsd.Height;
		m_bIsLocked = true;
	}
}

DARKSDK void UnlockPixels ( void )
{
	if ( m_bIsLocked==true )
	{
		m_pSurface->UnlockRect ( );
		m_bIsLocked = false;
	}
}

#endif

#include "ctextc.h"
#include "stdio.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"

#ifdef DARKSDK_COMPILE
	#include ".\..\..\..\DarkGDK\Code\Include\DarkSDKDisplay.h"
	//#include "cpositionc.cpp"
#endif

//new include
#include "CGfxC.h"

// some externs for rgb commands
extern int RgbR ( DWORD iRGB );
extern int RgbG ( DWORD iRGB );
extern int RgbB ( DWORD iRGB );
extern int GetBitDepthFromFormat(GGFORMAT Format);

// main globals
extern LPGG				m_pDX;					// pointer to dx
extern LPGGDEVICE			m_pD3D;					// pointer to direct3d interface
DBPRO_GLOBAL GGFORMAT					m_FontFormat						= GGFMT_UNKNOWN;		// default format
DBPRO_GLOBAL TCHAR						m_strFontName [ 80 ]				= "";					// font name
DBPRO_GLOBAL DWORD						m_dwFontHeight						= 0;					// font height
DBPRO_GLOBAL DWORD						m_dwFontFlags						= 0;					// font flags
DBPRO_GLOBAL LPGGTEXTURE				m_pTexture							= NULL;					// texture
DBPRO_GLOBAL LPGGVERTEXBUFFER			m_pVB								= NULL;					// vertex buffer
DBPRO_GLOBAL DWORD						m_dwTexWidth						= 0;					// texture width
DBPRO_GLOBAL DWORD						m_dwTexHeight						= 0;					// texture height
DBPRO_GLOBAL FLOAT						m_fTextScale						= 0.0f;					// text scale
DBPRO_GLOBAL FLOAT						m_fTexCoords [ 256 - 32 ] [ 4 ];							// texture coods
DBPRO_GLOBAL int						m_szTexWidth [ 256 - 32 ];									// letter sizes
DBPRO_GLOBAL int						m_szTexHeight [ 256 - 32 ];									// letter sizes
DBPRO_GLOBAL IGGStateBlock9*			m_pSavedStateBlock					= NULL;					// dx8->dx9
DBPRO_GLOBAL IGGStateBlock9*			m_pDrawTextStateBlock				= NULL;					// state block
DBPRO_GLOBAL tagObjectPos*				m_pPosText							= NULL;					// position pointer
extern PTR_FuncCreateStr				g_pCreateDeleteStringFunction;								// delete string
DBPRO_GLOBAL DWORD						dwDEFAULTCHARSET					= ANSI_CHARSET;			// character set
DBPRO_GLOBAL HFONT						g_hRetainRawTextWriteFont			= NULL;					// raw font
DBPRO_GLOBAL bool						g_bWideCharacterSet					= false;				// unicode

// font properties
DBPRO_GLOBAL DWORD						m_dwColor							= 0;					// colour
DBPRO_GLOBAL DWORD						m_dwBKColor							= 0;					// bk colour
DBPRO_GLOBAL bool						m_bTextBold							= false;				// bold flag
DBPRO_GLOBAL bool						m_bTextItalic						= false;				// italic flag
DBPRO_GLOBAL bool						m_bTextOpaque						= false;				// bold flag
DBPRO_GLOBAL int						m_iTextCharSet						= 0;					// text char set
DBPRO_GLOBAL int						m_iX								= 0;					// x pos
DBPRO_GLOBAL int						m_iY								= 0;					// y pos

// local checklist work vars
extern bool						g_bCreateChecklistNow;				// create checklist
extern DWORD						g_dwMaxStringSizeInEnum;					// maximum string size
DBPRO_GLOBAL LPSTR                      m_szTokenString                     = NULL;                 // splitting by token
DBPRO_GLOBAL DWORD                      m_dwTokenStringSize                 = 0;

// Global Work String
DWORD						m_dwTEXTWorkStringSize = 0;
LPSTR						m_pTEXTWorkString = NULL;

// function pointers
extern DBPRO_GLOBAL HINSTANCE			g_GFX;					// for dll loading
extern GlobStruct*						g_pGlob;				// glob struct

//////////////////////////////////////////////////////////////////////////

#define RETURNSTRONGPOOLMAX 40
char szReturnStringPool[RETURNSTRONGPOOLMAX][1024];
int iReturnStringPoolCount = 0;

LPSTR GetReturnStringFromTEXTWorkString(char* WorkString = m_pTEXTWorkString)
{
	LPSTR pReturnString=NULL;
	if(WorkString)
	{
		DWORD dwSize=strlen(WorkString);
		//g_pCreateDeleteStringFunction((DWORD*)&pReturnString, dwSize+1);
		pReturnString = szReturnStringPool[iReturnStringPoolCount];
		if ( ++iReturnStringPoolCount >= RETURNSTRONGPOOLMAX ) iReturnStringPoolCount = 0;
		if(WorkString)
			strcpy(pReturnString, WorkString);
		else
			strcpy(pReturnString, "");
		return pReturnString;
	}	
	return "";
}

DARKSDK bool UpdatePtr ( int iID )
{
	return true;
}

DARKSDK void ValidateWorkStringBySize ( DWORD dwSize )
{
    // u74b7 - delete m_pWorkString as an array
	// free string that is too small
	if ( m_pTEXTWorkString )
		if ( m_dwTEXTWorkStringSize<dwSize )
			SAFE_DELETE_ARRAY(m_pTEXTWorkString);

	// create new work string of good size
	if ( m_pTEXTWorkString==NULL )
	{
		m_dwTEXTWorkStringSize = dwSize+1;
		m_pTEXTWorkString = new char[m_dwTEXTWorkStringSize];
		memset ( m_pTEXTWorkString, 0, m_dwTEXTWorkStringSize );
	}
}

DARKSDK void ValidateWorkString(LPSTR pString)
{
	// Size from string
	if ( pString ) ValidateWorkStringBySize ( strlen(pString)+1 );
}

DARKSDK void ValidateDefaultTextureForFontDX9(void)
{
	#ifndef DX11
	// Would be a good idea to have a format-collector function for what surface, depth, texture formats we can use from the card and put into glob..
	// (taken form image DLL - perhaps merge them at some point in setup DLL)
	// Get default GGFORMAT from backbuffer
	D3DSURFACE_DESC backdesc;
	LPGGSURFACE pBackBuffer = g_pGlob->pCurrentBitmapSurface;
	if(pBackBuffer) pBackBuffer->GetDesc(&backdesc);
	m_FontFormat = GGFMT_A4R4G4B4;
	
	// Ensure textureformat is valid, else choose next valid..
	HRESULT hRes = m_pDX->CheckDeviceFormat(	GGADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												backdesc.Format,
												0, D3DRTYPE_TEXTURE,
												m_FontFormat);
	if ( FAILED( hRes ) )
	{
		// Need another texture format with an alpha
		for(DWORD t=0; t<12; t++)
		{
			switch(t)
			{
				case 0  : m_FontFormat = GGFMT_A4R4G4B4;		break;
				case 1  : m_FontFormat = GGFMT_A8R3G3B2;		break;
				case 2 : m_FontFormat = GGFMT_A1R5G5B5;		break;
				case 3 : m_FontFormat = GGFMT_X1R5G5B5;		break;
				case 4 : m_FontFormat = GGFMT_A8R8G8B8;		break;
				case 5 : m_FontFormat = GGFMT_A2B10G10R10;		break;
				case 6 : m_FontFormat = GGFMT_X8R8G8B8;		break;
				case 7 : m_FontFormat = GGFMT_R8G8B8;			break;
				case 8 : m_FontFormat = GGFMT_R5G6B5;			break;
				case 9 : m_FontFormat = GGFMT_R3G3B2;			break;
				case 10 : m_FontFormat = GGFMT_X4R4G4B4;		break;
				case 11 : m_FontFormat = GGFMT_G16R16;			break;
			}
			HRESULT hRes = m_pDX->CheckDeviceFormat(	GGADAPTER_DEFAULT,
														D3DDEVTYPE_HAL,
														backdesc.Format,
														0, D3DRTYPE_TEXTURE,
														m_FontFormat);
			if ( SUCCEEDED( hRes ) )
			{
				// Found a texture we can use
				return;
			}
		}
	}
	#endif
}

void ValidateDefaultTextureForFontDX11 ( void )
{
	#ifdef DX11
	#endif
}

DARKSDK void TextConstructor (  )
{
	// default to fixedsys font
	strcpy ( m_strFontName, "fixedsys" );		// attempt to use arial font
    m_dwFontHeight         = 12;				// use point size 12
	m_dwFontFlags          = 0;					// no flags initially
	m_fTextScale		   = 1.0f;				// normal scale
	m_iTextCharSet		   = 0;
	m_bTextBold			   = false;									// turn bold off
	m_bTextItalic		   = false;									// turn italic off
   	m_bTextOpaque		   = false;									// text opaque
	m_iX			       = 0;										// initial x pos
	m_iY			       = 0;										// initial y pos
    m_pTexture             = NULL;									// set texture pointer to null
    m_pVB                  = NULL;									// set vertex buffer to null

	#ifdef DX11
	m_dwColor			   = 0;
	m_dwBKColor			   = 0;
	m_pSavedStateBlock	   = NULL;	
	m_pDrawTextStateBlock  = NULL;	
	#else
	m_dwColor			   = GGCOLOR_ARGB ( 255, 255, 255, 255 );	// set colour to white
	m_dwBKColor			   = GGCOLOR_ARGB ( 255, 0, 0, 0 );		// set colour to black
	m_pD3D->CreateStateBlock  ( D3DSBT_ALL, &m_pSavedStateBlock );	// state blocks
	m_pD3D->CreateStateBlock  ( D3DSBT_ALL, &m_pDrawTextStateBlock );
	#endif

	// setup position
	SAFE_DELETE(m_pPosText);
	m_pPosText = new tagObjectPos;
	memset ( m_pPosText, 0, sizeof ( tagObjectPos ) );

	if ( !m_pPosText )
		Error1 ( "Unable to allocate memory for positional data in text library" );

	// prepare work string
	ValidateWorkStringBySize ( 256 );

    // u74b7 - if already have a token temp area, leave it alone.
    if (! m_szTokenString )
    {
        m_dwTokenStringSize = 100;
        m_szTokenString = new char[ 100 ];
		memset ( m_szTokenString, 0, sizeof(m_szTokenString) );
    }

	// Choose bext texture and create font and state now
	#ifdef DX11
	ValidateDefaultTextureForFontDX11();
	#else
	ValidateDefaultTextureForFontDX9();
	#endif
	SetupFont();
	TextSetupStates();
}

// Realtext Functions
DARKSDK HFONT DB_SetRealTextFont(HDC hdc, DWORD textstyle, DWORD bItalicFlag, int fontsize, char* fontname, int inter)
{
	HFONT hFont = NULL;
	if(inter==0)
	{
		// Get Real(Available) Font Height Used
		if(dwDEFAULTCHARSET==ANSI_CHARSET)
		{
			hFont = CreateFont( (int)(fontsize*m_fTextScale), 0, 0, 0, textstyle, bItalicFlag, FALSE, FALSE, dwDEFAULTCHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,VARIABLE_PITCH, fontname);
		}
		else
		{
			int FontHeight=0;
			FontHeight=-MulDiv(fontsize, (int)(GetDeviceCaps(hdc, LOGPIXELSY) * m_fTextScale), 72); 
			hFont = CreateFont( FontHeight, 0, 0, 0, textstyle, bItalicFlag, FALSE, FALSE, dwDEFAULTCHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,VARIABLE_PITCH, fontname);
			
		}
	}
	else
	{
		hFont = CreateFont(	fontsize, 0, 0, 0,
									textstyle, bItalicFlag, FALSE, FALSE,
									inter,
									OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,
									NONANTIALIASED_QUALITY,
									VARIABLE_PITCH,
									fontname);

		if(dwDEFAULTCHARSET!=(DWORD)inter)
		{
			dwDEFAULTCHARSET=inter;
		}
	}

	// work out if wide character text (japanese, korean, chinese/trad)
	if ( dwDEFAULTCHARSET==128 || dwDEFAULTCHARSET==129 || dwDEFAULTCHARSET==134 || dwDEFAULTCHARSET==136 ) 
		g_bWideCharacterSet = true;
	else
		g_bWideCharacterSet = false;

	// return font handle
	return hFont;
}

DARKSDK void SetupFontCreateTextureDX11 ( DWORD* pBitmapBits )
{
	#ifdef DX11
	#endif
}

DARKSDK void SetupFontCreateTextureDX9 ( DWORD* pBitmapBits )
{
	#ifndef DX11
	HRESULT		hr;
	DWORD		x = 0;
	DWORD		y = 0;
	WORD*		pDst16;
	BYTE		bAlpha;
	if ( FAILED ( hr = m_pD3D->CreateTexture ( m_dwTexWidth, m_dwTexHeight, 1, 0, m_FontFormat, D3DPOOL_MANAGED, &m_pTexture, NULL ) ) )
		Error ( "Unable to create font texture in text library" );
    
	// lock the surface (for 16bit format - 4444)
	GGLOCKED_RECT d3dlr;
	DWORD dwDepth=GetBitDepthFromFormat(m_FontFormat);
	if(dwDepth==16)
	{
		DWORD dwUseBack = m_dwBKColor;
		if(m_bTextOpaque==false) dwUseBack=0;

		m_pTexture->LockRect ( 0, &d3dlr, 0, 0 );
		pDst16 = ( WORD* ) d3dlr.pBits;
		WORD wFore = 0x0fff;
		WORD wBack = 0x0000;
		if(m_dwBKColor>0)
		{
			BYTE bFR = RgbR(m_dwColor)/16; if(bFR>15) bFR=15;
			BYTE bFG = RgbG(m_dwColor)/16; if(bFG>15) bFG=15;
			BYTE bFB = RgbB(m_dwColor)/16; if(bFB>15) bFB=15;
			BYTE bBR = RgbR(dwUseBack)/16; if(bFR>15) bFR=15;
			BYTE bBG = RgbG(dwUseBack)/16; if(bBG>15) bBG=15;
			BYTE bBB = RgbB(dwUseBack)/16; if(bBB>15) bBB=15;
			wFore = (bFR<<8) + (bFG<<4) + (bFB);
			wBack = (bBR<<8) + (bBG<<4) + (bBB);
		}
		for ( y = 0; y < m_dwTexHeight; y++ )
		{
			for ( x = 0; x < m_dwTexWidth; x++ )
			{
				bAlpha = ( BYTE ) ( ( pBitmapBits [ m_dwTexWidth * y + x ] & 0xff ) >> 4 );
					
				if ( bAlpha > 0 )
					*pDst16++ = ( bAlpha << 12 ) | wFore;
				else
					*pDst16++ = wBack;
			}
		}

		// unlock the texture
		m_pTexture->UnlockRect ( 0 );
	}
	#endif
}

DARKSDK void SetupFont ( void )
{
	// variable definitions
	BITMAPINFO	bmi;			// bitmap info structure
	HDC			hDC;			// handle to device context
	HFONT		hFont;			// handle to font
	TCHAR		str [ 2 ] = ( "x" );
	SIZE		size;

	// texture plate size isdependant on the size of the font
	hDC = CreateCompatibleDC ( NULL );	
	SetMapMode ( hDC, MM_TEXT );										// set mapping mode x, y
	DWORD textstyle		= ( m_bTextBold   ) ? FW_BOLD : FW_NORMAL;		// set bold flag on / off
	DWORD bItalicFlag	= ( m_bTextItalic ) ? TRUE    : FALSE;			// set italic flag on / off
	hFont = DB_SetRealTextFont ( hDC, textstyle, bItalicFlag, m_dwFontHeight, m_strFontName, m_iTextCharSet );
	HFONT hbmOldFont = (HFONT)SelectObject ( hDC, hFont );
	SetTextAlign ( hDC, TA_TOP );
	GetTextExtentPoint32 ( hDC, "g", 1, &size );

	// Extended chars left 256 texture plate
	DWORD dwActualUsageHeight = size.cy;
	if ( dwActualUsageHeight > 40 )
		m_dwTexWidth = m_dwTexHeight = 1024;
	else 
		if ( dwActualUsageHeight > 15 )
			m_dwTexWidth = m_dwTexHeight = 512;
		else
			m_dwTexWidth = m_dwTexHeight = 256;

	// now check that it's ok for us to create a texture at the height and width, get the device caps to do this
	DWORD dwMaxTextureWidth = 4096;
	#ifdef DX11
	#else
	GGCAPS		d3dCaps;
	m_pD3D->GetDeviceCaps ( &d3dCaps );
	dwMaxTextureWidth = d3dCaps.MaxTextureWidth;
	#endif

	// if the texture is too big we simply create a smaller texture and scale the final font
	m_fTextScale = 1.0f;
	if ( m_dwTexWidth > dwMaxTextureWidth )
	{
		m_fTextScale = ( float ) dwMaxTextureWidth / ( float ) m_dwTexWidth;
		m_dwTexWidth = m_dwTexHeight = dwMaxTextureWidth;
	}

	// and now create a bitmap, what we do is draw a font onto the bitmap and then later on copy it onto the texture. by doing this
	// we draw loads of 3d objects at runtime instead of having to lock buffers and copy text which is faster and enables us to have 3d text
	memset ( &bmi.bmiHeader, 0, sizeof ( BITMAPINFOHEADER ) );		// clear out the header
	bmi.bmiHeader.biSize        = sizeof ( BITMAPINFOHEADER );		// set the size
	bmi.bmiHeader.biWidth       =  ( int ) m_dwTexWidth;			// set the bmp width
	bmi.bmiHeader.biHeight      = -( int ) m_dwTexHeight;			// set the bmp height
	bmi.bmiHeader.biPlanes      = 1;								// set planes ( always 1 )
	bmi.bmiHeader.biCompression = BI_RGB;							// no compression
	bmi.bmiHeader.biBitCount    = 32;								// bit depth

	// create a device context and a bitmap for the font
	DWORD* pBitmapBits = NULL;
	HBITMAP hbmBitmap = CreateDIBSection   ( hDC, &bmi, DIB_RGB_COLORS, ( VOID** ) &pBitmapBits, NULL, 0 );		// create bmp

	// select objects into dc
	HBITMAP hbmOldBitmap = (HBITMAP)SelectObject ( hDC, hbmBitmap );

	// Colour is merely a mask maker, actual colour is later in code
	SetTextColor ( hDC, RGB ( 255, 255, 255 ) );
	SetBkColor   ( hDC, 0x00000000 );				// background colour, black by default

	// loop through all printable character and output them to the bitmap
	// also keep track of the corresponding tex coords for each character
	// Increased font chars to 256 (to include special ascii chars)
	int x = 0, y = 0;
	for ( unsigned short c = 32; c < 256; c++ )
	{
		str [ 0 ] = (unsigned char)c;
		GetTextExtentPoint32 ( hDC, str, 1, &size );
		DWORD dwPush = 0;
		DWORD dwAdditional = 0;
		if(m_bTextItalic) 
		{
			dwPush = (size.cy/8);
			dwAdditional = (size.cy/4);
			size.cx+=m_dwFontHeight / 3 + 5;
		}
		if ( ( DWORD ) ( x + size.cx + 1 ) > m_dwTexWidth )
		{
			x  = 0;
			y += size.cy + 2;
		}

		ExtTextOut ( hDC, x + dwPush + 0, y + 0, ETO_OPAQUE, NULL, str, 1, NULL );
		m_fTexCoords [ c - 32 ] [ 0 ] = ( ( float ) ( x + 0           ) ) / m_dwTexWidth;
		m_fTexCoords [ c - 32 ] [ 1 ] = ( ( float ) ( y + 0           ) ) / m_dwTexHeight;
		m_fTexCoords [ c - 32 ] [ 2 ] = ( ( float ) ( x + 0 + size.cx + 1 ) ) / m_dwTexWidth;
		m_fTexCoords [ c - 32 ] [ 3 ] = ( ( float ) ( y + 0 + size.cy + 1 ) ) / m_dwTexHeight;
		m_szTexWidth [ c - 32 ] = (int)((float)(size.cx - dwAdditional) / m_fTextScale);
		m_szTexHeight [ c - 32 ] = size.cy;

		// use this for foreign language DB Pro text
		if ( m_iTextCharSet != 0 )
			x += size.cx + 2;
		else
			x += size.cx + 16;
	}

	// create a blank texture for the font and lock to write contents
	#ifdef DX11
	SetupFontCreateTextureDX11 ( pBitmapBits );
	#else
	SetupFontCreateTextureDX9 ( pBitmapBits );
	#endif

	// select out objects
	SelectObject ( hDC, hbmOldBitmap );
	SelectObject ( hDC, hbmOldFont );

	// store font, deleting any old one
	if ( g_hRetainRawTextWriteFont ) DeleteObject ( g_hRetainRawTextWriteFont );
	g_hRetainRawTextWriteFont = hFont;

	// now clean up objects
	DeleteObject ( hbmBitmap );
	DeleteDC     ( hDC );
}

DARKSDK void TextSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void TextPassCoreData( LPVOID pGlobPtr )
{
}

DARKSDK void DeleteFonts ( void )
{
	if ( m_pTexture )
	{
		#ifdef DX11
		#else
		m_pTexture->Release ( );
		#endif
		m_pTexture = NULL;
	}
}

DARKSDK void DeleteStates ( void )
{
	if ( m_pVB )
	{
		#ifdef DX11
		#else
		m_pVB->Release ( );
		#endif
		m_pVB = NULL;
	}
}

DARKSDK void TextDestructor ( void )
{
    // u74b7 - delete token temp string
	SAFE_DELETE_ARRAY( m_szTokenString );
    // u74b7 - delete m_pWorkString as an array
	SAFE_DELETE_ARRAY( m_pTEXTWorkString );

	SAFE_DELETE(m_pPosText);
	DeleteFonts();
	DeleteStates();

	// delete widecharacter font
	if ( g_hRetainRawTextWriteFont )
	{
		DeleteObject ( g_hRetainRawTextWriteFont );
		g_hRetainRawTextWriteFont=NULL;
	}

	// Release state blocks
	#ifdef DX11
	#else
	if ( m_pSavedStateBlock )
	{
		m_pSavedStateBlock->Release();
		m_pSavedStateBlock=NULL;
	}
	if ( m_pDrawTextStateBlock )
	{
		m_pDrawTextStateBlock->Release();
		m_pDrawTextStateBlock=NULL;
	}
	#endif
}

DARKSDK void TextRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		TextDestructor();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		TextConstructor ( );
		TextPassCoreData ( g_pGlob );
	}
}

DARKSDK void SetTextColor ( int iAlpha, int iRed, int iGreen, int iBlue )
{
	m_dwColor = GGCOLOR_ARGB ( iAlpha, iRed, iGreen, iBlue );
}

DARKSDK void TextSetupStatesDX9 ( void )
{
	#ifndef DX11
	HRESULT hr;

    // create vertex buffer
    if ( FAILED ( hr = m_pD3D->CreateVertexBuffer ( MAX_NUM_VERTICES * sizeof ( FONT2DVERTEX ), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &m_pVB, NULL ) ) )
        Error ( "Unable to create vertex buffer for text library" );
    
	// leefix-060803-release stateblocks
	SAFE_RELEASE ( m_pSavedStateBlock );
	SAFE_RELEASE ( m_pDrawTextStateBlock );

	// create the state blocks for rendering text
    for ( UINT which = 0; which < 2; which++ )
    {
        m_pD3D->BeginStateBlock ( );

        m_pD3D->SetTexture ( 0, m_pTexture );

		// Text can be transparent
		if(m_bTextOpaque==true)
		{
			m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE,			FALSE );
		}
		else
		{
			m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE,			TRUE );
		}
        m_pD3D->SetRenderState ( D3DRS_SRCBLEND,					D3DBLEND_SRCALPHA );
        m_pD3D->SetRenderState ( D3DRS_DESTBLEND,					D3DBLEND_INVSRCALPHA );
        m_pD3D->SetRenderState ( D3DRS_FILLMODE,					D3DFILL_SOLID );
        m_pD3D->SetRenderState ( D3DRS_CULLMODE,					GGCULL_CCW );
        m_pD3D->SetRenderState ( D3DRS_ZENABLE,						FALSE );
        m_pD3D->SetRenderState ( D3DRS_STENCILENABLE,				FALSE );
        m_pD3D->SetRenderState ( D3DRS_CLIPPING,					TRUE );
        m_pD3D->SetRenderState ( D3DRS_VERTEXBLEND,					FALSE );
        m_pD3D->SetRenderState ( D3DRS_INDEXEDVERTEXBLENDENABLE,	FALSE );
        m_pD3D->SetRenderState ( D3DRS_FOGENABLE,					FALSE );

        m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP,				GGTOP_MODULATE );
        m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1,				GGTA_TEXTURE );
        m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG2,				GGTA_DIFFUSE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP,				GGTOP_MODULATE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1,				GGTA_TEXTURE );
		m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2,				GGTA_DIFFUSE );

		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER,					GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER,					GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER,					D3DTEXF_POINT );//GGTEXF_NONE );

        m_pD3D->SetTextureStageState ( 0, D3DTSS_TEXCOORDINDEX,			0 );
        m_pD3D->SetTextureStageState ( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
        m_pD3D->SetTextureStageState ( 1, D3DTSS_COLOROP,				GGTOP_DISABLE );

		// leefix-060803-recreate stateblocks
        if ( which == 0 )
            m_pD3D->EndStateBlock ( &m_pSavedStateBlock );
        else
            m_pD3D->EndStateBlock ( &m_pDrawTextStateBlock );
    }
	#endif
}

DARKSDK void TextSetupStatesDX11 ( void )
{
	#ifdef DX11
	#endif
}

DARKSDK void TextSetupStates ( void )
{
	#ifdef DX11
	TextSetupStatesDX11();
	#else
	TextSetupStatesDX9();
	#endif
}

DARKSDK void TextRecreate ( void )
{
	DeleteFonts();
	DeleteStates();
	SetupFont ( );
	TextSetupStates ( );
}

DARKSDK int CALLBACK EnumFontFamProc(ENUMLOGFONT FAR *lpelf, NEWTEXTMETRIC FAR *lpntm, int FontType, LPARAM lParam )
{
	LPSTR pFontName=lpelf->elfLogFont.lfFaceName;
	if(pFontName)
	{
		DWORD dwSize=strlen(pFontName)+1;
		if(dwSize>g_dwMaxStringSizeInEnum) g_dwMaxStringSizeInEnum=dwSize;
		if(g_bCreateChecklistNow)
		{
			// New checklist item
			strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, pFontName);
		}
		g_pGlob->checklistqty++;
	}
	return 1;
}

DARKSDK void TextCore ( int iX, int iY, char* szText )
{
	// only if string given
	if(szText==NULL)
		return;

	// use simple GDI textwrite if using wide character (for now)
	// if ( g_bWideCharacterSet && g_hRetainRawTextWriteFont )
	// widecharacter text not drawing to camera render targets, so only do code below for direct
	#ifdef DX11
	#else
	bool bRenderingToCamera = false;
	if ( g_pGlob->iCurrentBitmapNumber==0 && g_pGlob->pCurrentBitmapSurface!=g_pGlob->pHoldBackBufferPtr ) bRenderingToCamera = true;
	if ( g_bWideCharacterSet && g_hRetainRawTextWriteFont && bRenderingToCamera==false )
	{
		// direct write of wide character text
		HDC hDC;
		LPGGSURFACE pBackBuffer = g_pGlob->pCurrentBitmapSurface;
		pBackBuffer->GetDC ( &hDC );
	    HFONT hbmOldFont = (HFONT)SelectObject ( hDC, g_hRetainRawTextWriteFont );
		SetTextColor ( hDC, RGB ( RgbR(m_dwColor),RgbG(m_dwColor),RgbB(m_dwColor) ) );
		SetBkColor   ( hDC, RGB ( RgbR(m_dwBKColor),RgbG(m_dwBKColor),RgbB(m_dwBKColor) ) );
		SetTextAlign ( hDC, TA_TOP );
		DWORD dwOpaqueMode = ETO_OPAQUE;
		if ( m_bTextOpaque==false )
		{
			SetBkMode ( hDC, TRANSPARENT );
			dwOpaqueMode=0;
		}
		else
			SetBkMode ( hDC, OPAQUE );

        ExtTextOut ( hDC, iX, iY, dwOpaqueMode, NULL, szText, strlen(szText), NULL );
		SelectObject ( hDC, hbmOldFont );
		pBackBuffer->ReleaseDC ( hDC );

		// complete
		return;
	}
	#endif

	float			fStartX			= (float)iX;
	float			fX				= (float)iX;
	float			fY				= (float)iY;

	FONT2DVERTEX*	pVertices		= NULL;
    DWORD			dwNumTriangles	= 0;

	// setup renderstates
	#ifdef DX11
	#else
	m_pSavedStateBlock->Capture();
	m_pDrawTextStateBlock->Apply();

    m_pD3D->SetVertexShader   ( NULL );
    m_pD3D->SetFVF   ( D3DFVF_FONT2DVERTEX );

    m_pD3D->SetStreamSource   ( 0, m_pVB, 0, sizeof ( FONT2DVERTEX ) );

	// fill vertex buffer
    m_pVB->Lock ( 0, 0, ( VOID** )&pVertices, D3DLOCK_DISCARD );

    while ( *(LPSTR)szText )
    {
        unsigned char c = *(LPSTR)szText++;

        if ( c == ('\n') )
        {
            fX  = fStartX;
            fY += ( m_fTexCoords [ 0 ] [ 3 ] - m_fTexCoords [ 0 ] [ 1 ] ) * m_dwTexHeight;
        }
        
		if ( c < (' ') )
            continue;

        FLOAT tx1 = m_fTexCoords [ c - 32 ] [ 0 ];
        FLOAT ty1 = m_fTexCoords [ c - 32 ] [ 1 ];
        FLOAT tx2 = m_fTexCoords [ c - 32 ] [ 2 ];
        FLOAT ty2 = m_fTexCoords [ c - 32 ] [ 3 ];

        FLOAT w = ( tx2 - tx1 ) * m_dwTexWidth  / m_fTextScale;
        FLOAT h = ( ty2 - ty1 ) * m_dwTexHeight / m_fTextScale;

		DWORD dwUseActualVertexColour=m_dwColor;
		if(m_dwBKColor>0) dwUseActualVertexColour=GGCOLOR_ARGB ( 255, 255, 255, 255 );
		if(m_dwBKColor==0 && m_dwColor==0) dwUseActualVertexColour=GGCOLOR_ARGB ( 255, 1, 1, 1 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + 0 - 0.5f, fY + h -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx1, ty2 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + 0 - 0.5f, fY + 0 -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx1, ty1 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + w - 0.5f, fY + h -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx2, ty2 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + w - 0.5f, fY + 0 -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx2, ty1 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + w - 0.5f, fY + h -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx2, ty2 );
        *pVertices++ = InitFont2DVertex ( GGVECTOR4 ( fX + 0 - 0.5f, fY + 0 -0.5f, 0.9f, 1.0f ), dwUseActualVertexColour, tx1, ty1 );

        dwNumTriangles += 2;

        if ( dwNumTriangles * 3 > ( MAX_NUM_VERTICES - 6 ) )
        {
            // unlock, render, and relock the vertex buffer
            m_pVB->Unlock ( );

            m_pD3D->DrawPrimitive ( GGPT_TRIANGLELIST, 0, dwNumTriangles );

            pVertices = NULL;

            m_pVB->Lock ( 0, 0, ( VOID** ) &pVertices, D3DLOCK_DISCARD );
            dwNumTriangles = 0L;
        }

        fX += m_szTexWidth [ c - 32 ];
    }

	// unlock and render the vertex buffer
    m_pVB->Unlock();

    if ( dwNumTriangles > 0 )
        m_pD3D->DrawPrimitive ( GGPT_TRIANGLELIST, 0, dwNumTriangles );

    // restore the modified renderstates
	m_pSavedStateBlock->Apply();
	#endif
}

DARKSDK int GetTextWidth ( char* szString )
{
	int iWidth=0;
	if(szString)
	{
		while ( *(LPSTR)szString )
		{
			unsigned char c = *(LPSTR)szString++;
			if ( c>=32 ) iWidth+=m_szTexWidth [ c - 32 ];
		}
	}
	return iWidth;
}

DARKSDK int GetTextHeight ( char* szString )
{
	int iHeight=0;
	if(szString)
	{
		while ( *(LPSTR)szString )
		{
			unsigned char c = *(LPSTR)szString++;
			if ( c>=32 )
			{
				int iThisH = m_szTexHeight [ c - 32 ];
				if(iThisH>iHeight) iHeight=iThisH;
			}
		}
	}
	return iHeight;
}

//
// General String Command Functions
//

DARKSDK int	  Asc	( LPSTR dwSrcStr )
{
	if(dwSrcStr)
		return (int)*dwSrcStr;
	else
		return 0;
}

DARKSDK DWORD Bin	( DWORD pDestStr, int iValue )
{
	// Work string
	LPSTR text=m_pTEXTWorkString;
	int t=0;
	text[t++]='%';
	for(int bit=31; bit>=0; bit--)
	{
		unsigned int mask = 1 << bit;
		if(iValue & mask)
			text[t++]='1';
		else
			text[t++]='0';
	}
	text[t]=0;

	// Create and return string
	if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return (DWORD)pReturnString;
}

DARKSDK LPSTR	  Chr	( int iValue )
{
	// Work string
	m_pTEXTWorkString[0]=iValue;
	m_pTEXTWorkString[1]=0;

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK DWORD Hex	( DWORD pDestStr, int iValue )
{
	// Work string
	wsprintf(m_pTEXTWorkString, "%X", iValue);

	// Create and return string
	if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return (DWORD)pReturnString;
}

DARKSDK LPSTR Left	( LPSTR szText, int iValue )
{
	// Work string
	ValidateWorkString ( (LPSTR)szText );
	LPSTR text=m_pTEXTWorkString;
	if(szText)
		strcpy(text, (char*)szText);
	else
		strcpy(text, "");

	if(iValue>0 && iValue<=(int)strlen(text))
		text[iValue]=0;
	else
		if(iValue<=0)
			strcpy(text, "");

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK int Len ( LPSTR dwSrcStr )
{
	if(dwSrcStr)
		return strlen(dwSrcStr);
	else
		return 0;
}

DARKSDK LPSTR Lower ( LPSTR szText )
{
	// Work string
	ValidateWorkString ( (LPSTR)szText );
	if(szText)
		strcpy(m_pTEXTWorkString, (LPSTR)szText);
	else
		strcpy(m_pTEXTWorkString, "");

	_strlwr(m_pTEXTWorkString);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR Mid	( LPSTR szText, int iValue )
{
	// Work string
	ValidateWorkString ( szText );
	char character;
	LPSTR text = m_pTEXTWorkString;
	if(szText)
		strcpy_s(text, m_dwTEXTWorkStringSize, szText);
	else
		strcpy_s(text, 2, "");

	unsigned int index = iValue;
	if(index>0 && index<=strlen(text))
	{
		character=text[index-1];	
		text[0]=character;
		text[1]=0;
	}
	else
		text[0]=0;

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR Right ( LPSTR szText, int iValue )
{
	// Work string
	ValidateWorkString ( (LPSTR)szText );
	LPSTR text=m_pTEXTWorkString;
	if(szText)
		strcpy(text, (char*)szText);
	else
		strcpy(text, "");

	int w = 0;
	int length = strlen(text);
	int rightmost = length-iValue;
	if(rightmost>=0 && rightmost<=length)
	{
		for(int n=rightmost; n<length; n++)
			text[w++]=text[n];
		text[w]=0;
	}
	else
		if(rightmost>length)
			strcpy(text, "");
		
	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR Str	( float fValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%.12g", fValue);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR StrEx	( float fValue, int iDecPlaces )
{
	// Work string
	char format[32];
	sprintf(format, "%%.%df", iDecPlaces );
	sprintf(m_pTEXTWorkString, format, fValue);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR Str	( int iValue )
{
	// Work string
	if ( m_pTEXTWorkString ) sprintf(m_pTEXTWorkString, "%d", iValue);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK LPSTR Upper ( LPSTR szText )
{
	// Work string
	ValidateWorkString ( (LPSTR)szText );
	if(szText)
		strcpy(m_pTEXTWorkString, (LPSTR)szText);
	else
		strcpy(m_pTEXTWorkString, "");

	_strupr(m_pTEXTWorkString);

	// Create and return string
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return pReturnString;
}

DARKSDK float ValF ( LPSTR dwSrcStr )
{
	float fValue = 0.0f;
	if(dwSrcStr) fValue = (float)atof(dwSrcStr);
	return fValue;
}

// Added DOUBLE INTEGER Return for bigger numbers
LONGLONG ValR	( DWORD dwSrcStr )
{
	LONGLONG lValue = 0;
	if(dwSrcStr) lValue = _atoi64((LPSTR)dwSrcStr);
	return lValue;
}

DARKSDK DWORD StrDouble	( DWORD pDestStr, double dValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%.16g", dValue);

	// Create and return string
	if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return (DWORD)pReturnString;
}

DARKSDK DWORD StrDoubleInt	( DWORD pDestStr, LONGLONG lValue )
{
	// Work string
	sprintf(m_pTEXTWorkString, "%I64d", lValue);

	// Create and return string
	if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return (DWORD)pReturnString;
}

//
// Text Command Functions
//

DARKSDK void PerformChecklistForFonts ( void )
{
	// Generate Checklist
	g_pGlob->checklisthasvalues=false;
	g_pGlob->checklisthasstrings=true;

	g_dwMaxStringSizeInEnum=0;
	g_bCreateChecklistNow=false;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			g_bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, g_dwMaxStringSizeInEnum);
		}

		// Run through total list of fonts
		g_pGlob->checklistqty=0;
		HDC hdc = CreateCompatibleDC ( NULL );															// create dc
		DWORD SearchData=0;
		EnumFontFamilies(hdc, NULL, (FONTENUMPROC)EnumFontFamProc, SearchData); 
		DeleteDC ( hdc );
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

DARKSDK void Text ( int iX, int iY, LPSTR szText )
{
	// External Ink Color Control
	if(m_dwColor!=g_pGlob->dwForeColor)
	{
		m_dwColor=g_pGlob->dwForeColor;
		if(m_dwBKColor>0) TextRecreate();
	}
	
	if(m_dwBKColor!=g_pGlob->dwBackColor) { m_dwBKColor=g_pGlob->dwBackColor; TextRecreate(); }
	if(szText) TextCore ( iX, iY, szText );
}

DARKSDK void CenterText ( int iX, int iY, LPSTR szText )
{
	// External Ink Color Control
	if(m_dwColor!=g_pGlob->dwForeColor)
	{
		m_dwColor=g_pGlob->dwForeColor;
		if(m_dwBKColor>0) TextRecreate();
	}
	if(m_dwBKColor!=g_pGlob->dwBackColor) { m_dwBKColor=g_pGlob->dwBackColor; TextRecreate(); }
	int iHalfWidth=GetTextWidth((LPSTR)szText)/2;
	if(szText) TextCore ( iX-iHalfWidth, iY, szText );
}

DARKSDK void SetTextFont ( LPSTR szTypeface )
{
	if(szTypeface)
	{
		memset ( m_strFontName, 0, sizeof ( m_strFontName ) );
		if(szTypeface) memcpy ( m_strFontName, szTypeface, sizeof ( char ) * strlen ( szTypeface ) );
		TextRecreate ( );
	}
}

DARKSDK void SetTextFont ( LPSTR szTypeface, int iCharacterSet )
{
	if(szTypeface)
	{
		memset ( m_strFontName, 0, sizeof ( m_strFontName ) );
		if(szTypeface) memcpy ( m_strFontName, szTypeface, sizeof ( char ) * strlen ( szTypeface ) );
		m_iTextCharSet = iCharacterSet;
		TextRecreate ( );
	}
}

DARKSDK void SetTextSize ( int iSize )
{
	if ( (DWORD)iSize != m_dwFontHeight )
	{
		m_dwFontHeight = iSize;
		if ( m_dwFontHeight > 100 )
			m_dwFontHeight = 100;

		TextRecreate ( );
	}
}

DARKSDK void SetTextToNormal ( void )
{
	m_bTextBold   = false;
	m_bTextItalic = false;
	TextRecreate ( );
}

DARKSDK void SetTextToItalic ( void )
{
	m_bTextBold   = false;
	m_bTextItalic = true;
	TextRecreate ( );
}

DARKSDK void SetTextToBold ( void )
{
	m_bTextBold = true;
	m_bTextItalic = false;
	TextRecreate ( );
}

DARKSDK void SetTextToBoldItalic ( void )
{
	m_bTextBold   = true;
	m_bTextItalic = true;
	TextRecreate ( );
}

DARKSDK void SetTextToOpaque ( void )
{
	m_bTextOpaque = true;
	TextRecreate ( );
}

DARKSDK void SetTextToTransparent ( void )
{
	m_bTextOpaque = false;
	TextRecreate ( );
}

//
// Command Expression Functions
//

DARKSDK DWORD TextFont ( DWORD pDestStr )
{
	// Work string
	strcpy(m_pTEXTWorkString, m_strFontName);

	// Create and return string
	if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=GetReturnStringFromTEXTWorkString();
	return (DWORD)pReturnString;
}

DARKSDK int TextSize ( void )
{
	return m_dwFontHeight;
}

DARKSDK int TextStyle ( void )
{
	int iStyle=0;
	if(m_bTextItalic) iStyle+=1;
	if(m_bTextBold) iStyle+=2;
	return iStyle;
}

DARKSDK int TextBackgroundType ( void )
{
	if(m_bTextOpaque)
		return 1;
	else
		return 0;
}

DARKSDK int TextWidth ( DWORD szString )
{
	int iWidth=0;
	if(szString)
	{
		while ( *(LPSTR)szString )
		{
			unsigned char c = *(LPSTR)szString++;
			if ( c>=32 ) iWidth+=m_szTexWidth [ c - 32 ];
		}
	}
	return iWidth;
}

DARKSDK int TextHeight ( DWORD szString )
{
	int iHeight=0;
	if(szString)
	{
		while ( *(LPSTR)szString )
		{
			unsigned char c = *(LPSTR)szString++;
			if ( c>=32 )
			{
				int iThisH = m_szTexHeight [ c - 32 ];
				if(iThisH>iHeight) iHeight=iThisH;
			}
		}
	}
	return iHeight;
}

//
// New Command Functions
//

DARKSDK void Text3D ( char* szText )
{
	if(szText==NULL)
		return;

	float			x = 0.0f;
    float			y = 0.0f;
    DWORD			dwVertex       = 0L;
    DWORD			dwNumTriangles = 0L;
	FLOAT			fStartX = x;

	#ifdef DX11
	#else
	unsigned char	c;
	FONT3DVERTEX*	pVertices;
	GGMATRIX matTrans;
	GGGetTransform ( GGTS_WORLD, &matTrans );
	GGSetTransform ( GGTS_WORLD, &m_pPosText->matObject );

	// setup renderstates
    m_pSavedStateBlock->Capture();
    m_pDrawTextStateBlock->Apply();

    m_pD3D->SetVertexShader   ( NULL );
    m_pD3D->SetFVF   ( D3DFVF_FONT3DVERTEX );

    m_pD3D->SetStreamSource   ( 0, m_pVB, 0, sizeof ( FONT3DVERTEX) );

    // set filter states
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, GGTEXF_LINEAR );
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, GGTEXF_LINEAR );
	m_pD3D->SetRenderState ( D3DRS_CULLMODE, D3DCULL_NONE );

	// fill vertex buffer
    m_pVB->Lock ( 0, 0, ( VOID** )&pVertices, D3DLOCK_DISCARD );

	while ( c = *szText++ )
    {
        if ( c == '\n' )
        {
            x  = fStartX;
            y -= ( m_fTexCoords [ 0 ] [ 3 ] - m_fTexCoords [ 0 ] [ 1 ] ) * m_dwTexHeight / 10.0f;
        }

        if ( c < 32 )
            continue;

        FLOAT tx1 = m_fTexCoords [ c - 32 ] [ 0 ];
        FLOAT ty1 = m_fTexCoords [ c - 32 ] [ 1 ];
        FLOAT tx2 = m_fTexCoords [ c - 32 ] [ 2 ];
        FLOAT ty2 = m_fTexCoords [ c - 32 ] [ 3 ];

        FLOAT w = ( tx2 - tx1 ) * m_dwTexWidth  / ( 10.0f * m_fTextScale );
        FLOAT h = ( ty2 - ty1 ) * m_dwTexHeight / ( 10.0f * m_fTextScale );

        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + 0, y + 0, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx1, ty2 );
        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + 0, y + h, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx1, ty1 );
        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + w, y + 0, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx2, ty2 );
        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + w, y + h, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx2, ty1 );
        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + w, y + 0, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx2, ty2 );
        *pVertices++ = InitFont3DVertex ( GGVECTOR3 ( x + 0, y + h, 0 ), GGVECTOR3 ( 0, 0, -1 ), tx1, ty1 );
        dwNumTriangles += 2;

        if ( dwNumTriangles * 3 > ( MAX_NUM_VERTICES - 6 ) )
        {
            m_pVB->Unlock ( );
            m_pD3D->DrawPrimitive ( GGPT_TRIANGLELIST, 0, dwNumTriangles );
            m_pVB->Lock ( 0, 0, ( VOID** ) &pVertices, D3DLOCK_DISCARD );
            dwNumTriangles = 0L;
        }

        x += w;
    }

	// Unlock and render the vertex buffer
    m_pVB->Unlock ( );

    if ( dwNumTriangles > 0 )
		m_pD3D->DrawPrimitive ( GGPT_TRIANGLELIST, 0, dwNumTriangles );

    // restore the modified renderstates
    m_pSavedStateBlock->Apply();

	//GGGetTransform ( GGTS_WORLD, &matTrans );
	GGSetTransform ( GGTS_WORLD, &matTrans );
	#endif
}

//
// Extra String Expressions
//

DARKSDK LPSTR Spaces ( int iSpaces )
{
	if ( iSpaces < 0 )
	{
		//if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
		LPSTR pReturnString=NULL;
		g_pCreateDeleteStringFunction((DWORD*)&pReturnString, 2 );
		memset((LPSTR)pReturnString, 32, 2);
		pReturnString [ 0 ] = 0;
		pReturnString [ 1 ] = 0;
		return pReturnString;	
	}

	// Create and return string
	//if(pDestStr) g_pCreateDeleteStringFunction((DWORD*)&pDestStr, 0);
	LPSTR pReturnString=NULL;
	g_pCreateDeleteStringFunction((DWORD*)&pReturnString, iSpaces+1 );
	memset((LPSTR)pReturnString, 32, iSpaces);
	pReturnString[iSpaces]=0;
	return pReturnString;
}

DARKSDK char* SetupString ( char* szInput )
{
	char* pReturn = NULL;
	DWORD dwSize  = strlen ( szInput );

	g_pGlob->CreateDeleteString ( ( DWORD* ) &pReturn, dwSize + 1 );

	// error
	if ( !pReturn )
		RunTimeError ( RUNTIMEERROR_NOTENOUGHMEMORY );
		
	memcpy ( pReturn, szInput, dwSize );

	pReturn [ dwSize ] = 0;

	return pReturn;
}

DARKSDK void Reverse ( LPSTR dwA )
{
	strrev ( dwA );
}

DARKSDK int FindFirstChar ( LPSTR dwSource, LPSTR dwChar )
{
	char* pInt    = dwChar;
	char  pIntA   = *pInt;
	char* pFirst  = strchr ( dwSource, ( int ) pIntA );
	int   iResult = pFirst - dwSource + 1;

	return iResult;
}

DARKSDK int FindLastChar ( LPSTR dwSource, LPSTR dwChar )
{
	char* pInt    = dwChar;
	char  pIntA   = *pInt;
	char* pFirst  = strrchr ( dwSource, ( int ) pIntA );
	int   iResult = pFirst - dwSource + 1;

	return iResult;
}

DARKSDK int FindSubString ( LPSTR dwSource, LPSTR dwString )
{
	char* pFirst  = strstr ( dwSource, dwString );
	int   iResult = pFirst - dwSource + 1;

	return iResult;
}

DARKSDK int CompareCase ( LPSTR dwA, LPSTR dwB )
{
	if ( strcmp ( dwA, dwB ) == 0 )
		return 1;

	return 0;
}

DARKSDK LPSTR FirstToken ( LPSTR dwSource, LPSTR dwDelim )
{
    LPSTR szSource = dwSource;
    LPSTR szDelim  = dwDelim;

    // If the delimiter an empty string, use a space
    if (szDelim == 0 || szDelim[0] == 0)
    {
        szDelim = " ";
    }

    // If there's a string, copy it to the temp area
    if ( szSource && szSource[0] )
    {
        // Get length, including null terminator
        DWORD dwLength = strlen( szSource ) + 1;

        // If the temp area isn't large enough, make it so
        if ( dwLength > m_dwTokenStringSize )
        {
            // U74 - 060709 - double the allocation is not enough, must use dwLength
            m_dwTokenStringSize = dwLength + 1;

			// free old string and create larger one
            delete[] m_szTokenString;
            m_szTokenString = new char[ m_dwTokenStringSize ];
			memset ( m_szTokenString, 0, sizeof(m_szTokenString) );
        }

        // Copy the source string into the temp area
        memcpy( m_szTokenString, szSource, dwLength );
    }
    else
    {
        m_szTokenString[ 0 ] = 0;
    }

	// U74 BETA9 - 060709 - free old string and create new one
	//g_pGlob->CreateDeleteString ( ( DWORD* ) &dwReturn, 0 );
    char* szToken = strtok ( m_szTokenString, szDelim );
    if ( szToken )
    {
	    return SetupString ( szToken );
    }
    return SetupString ( "" );
}

DARKSDK LPSTR NextToken ( LPSTR dwDelim )
{
    LPSTR szDelim  = dwDelim;

    // If the delimiter an empty string, use a space
    if (szDelim == 0 || szDelim[0] == 0)
    {
        szDelim = " ";
    }

	// U74 BETA9 - 060709 - free old string and create new one
	char* szToken = strtok ( NULL, szDelim );
    if ( szToken )
    {
	    return SetupString ( szToken );
    }
    return SetupString ( "" );
}

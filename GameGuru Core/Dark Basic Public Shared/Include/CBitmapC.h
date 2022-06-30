#ifndef _CBITMAP_H_
#define _CBITMAP_H_

#include "directx-macros.h"

#define DARKSDK

#ifndef _tagData_
#define _tagData_
struct tagCamData
{
	LPGGSURFACE				lpSurface;					// render target surface
	LPGGRENDERTARGETVIEW	lpSurfaceView;
	LPGGSURFACE				lpDepth;					// render target depthbuffer
	LPGGDEPTHSTENCILVIEW	lpDepthView;

	int						iWidth;						// width of texture
	int						iHeight;					// height of texture
	int						iDepth;						// depth of texture

	int						iMirrored;					// mirror
	int						iFlipped;					// flip
	int						iFadeValue;					// fade of fade setting

	bool					bLocked;					// is locked
	GGLOCKED_RECT			d3dlr;						// information structure

	char					szLongFilename  [ 256 ];	// long filename  e.g. "c:\\images\\a.bmp"
	char					szShortFilename [ 256 ];	// short filename e.g. ".\\a.bmp"
};
#endif

DARKSDK void				BitmapConstructor						(  );
DARKSDK void				BitmapDestructor						( void );
DARKSDK void				BitmapSetErrorHandler					( LPVOID pErrorHandlerPtr );
DARKSDK void				BitmapPassCoreData						( LPVOID pGlobPtr );
DARKSDK void				BitmapRefreshGRAFIX						( int iMode );
DARKSDK void				UpdateBitmapZeroOfNewBackbuffer			( void );
DARKSDK LPGGSURFACE			MakeBitmap								( int iID, int iWidth, int iHeight );

DARKSDK void				CreateBitmap							( int iID, int iWidth, int iHeight );
DARKSDK void 				LoadBitmap 								( char* szFilename, int iID );
DARKSDK void 				SaveBitmap 								( char* szFilename, int iID );
DARKSDK void 				LoadBitmapEx 							( char* szFilename );
DARKSDK void 				SaveBitmapEx 							( char* szFilename );
DARKSDK void 				CopyBitmap 								( int iID, int iToID );
DARKSDK void 				CopyBitmap								( int iID, int iX1, int iY1, int iX2, int iY2, int iToID, int iToX1, int iToY1, int iToX2, int iToY2 );
DARKSDK void 				DeleteBitmapEx 							( int iID );
DARKSDK void 				FlipBitmap 								( int iID );
DARKSDK void 				MirrorBitmap 							( int iID );
DARKSDK void 				FadeBitmap 								( int iID, int iFade );
DARKSDK void 				BlurBitmap 								( int iID, int iBlurLevel );
DARKSDK void 				SetCurrentBitmap 						( int iID );
DARKSDK void*				GetBitmapRenderTarget					( int iID );
DARKSDK void*				GetBitmapTexture2D						( int iID );

DARKSDK int 				CurrentBitmap 							( void );
DARKSDK int 				BitmapExist 							( int iID );
DARKSDK int 				BitmapWidth 							( int iID );
DARKSDK int					BitmapHeight 							( int iID );
DARKSDK int 				BitmapDepth 							( int iID );
DARKSDK int 				BitmapMirrored 							( int iID );
DARKSDK int 				BitmapFlipped 							( int iID );
DARKSDK int 				BitmapExist 							( void );
DARKSDK int 				BitmapWidth 							( void );
DARKSDK int 				BitmapHeight 							( void );
DARKSDK int 				BitmapDepth 							( void );
DARKSDK int 				BitmapMirrored 							( void );
DARKSDK int 				BitmapFlipped 							( void );

DARKSDK void 				GetBitmapData							( int iID, DWORD* dwWidth, DWORD* dwHeight, DWORD* dwDepth, LPSTR* pData, DWORD* dwDataSize, bool bLockData );
DARKSDK void 				SetBitmapData							( int bitmapindex, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, LPSTR pData, DWORD dwDataSize );

DARKSDK void				SetBitmapFormat							( int iFormat );

#endif _CBITMAP_H_

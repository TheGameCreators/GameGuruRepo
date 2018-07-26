#ifndef _CIMAGE_H_
#define _CIMAGE_H_

#pragma comment ( lib, "gdi32.lib" )

#include "directx-macros.h"

#ifdef LoadImage
	#undef LoadImage
#endif

#ifdef LoadIcon
	#undef LoadIcon
#endif

#define DB_PRO 1

#undef DARKSDK
#define DARKSDK
#ifndef _tagImgData_
#define _tagImgData_
struct tagImgData
{
	LPGGTEXTURE				lpTexture;					// texture
	LPSTR					lpName;

	LPGGCUBETEXTURE			pCubeMapRef;				// for cubemap rendertarget override
	int						iCubeMapFace;				// which face of the cube to render too
	
	int						iWidth;						// width of texture
	int						iHeight;					// height of texture
	int						iDepth;						// depth of texture
	bool					bLocked;					// is locked

	int						AccessCountGPU;				// PE: Number of times sent to GPU
	int						AccessCountCPU;				// PE: Number of times CPU accessed the image.

	float					fTexUMax;					// U-coord for texture clip
	float					fTexVMax;					// V-coord for texture clip

	GGLOCKED_RECT			d3dlr;						// information structure

	char					szLongFilename  [ 256 ];	// long filename  e.g. "c:\\images\\a.bmp"
	char					szShortFilename [ 256 ];	// short filename e.g. ".\\a.bmp"

	LPGGTEXTUREREF			lpTextureView;				// DX11 resource view
};
#endif

DARKSDK bool                UpdateImagePtr				( int iID );
DARKSDK LPGGTEXTURE			GetTexture					( char* szFilename, GGIMAGE_INFO* info, int iOneToOnePixels );

DARKSDK void				ImageConstructor			( void );
DARKSDK void				DumpImageList				( void ); //PE: Image access debug.
DARKSDK void				ImageDestructor				( void );
DARKSDK void				ImageSetErrorHandler		( LPVOID pErrorHandlerPtr );
DARKSDK void				PassSpriteInstance			( void );
DARKSDK void				ImageRefreshGRAFIX				( int iMode );
DARKSDK void				ImagePassCoreData			( LPVOID pGlobPtr );

DARKSDK void				StretchImage				( int iID, int iSizeX, int iSizeY );

DARKSDK int					LoadImageInternal				( char* szFilename );
DARKSDK int					LoadImageInternalEx				( char* szFilename, int iDivideTextureSize );
DARKSDK void				ClearAnyLightMapInternalTextures ( void );
DARKSDK void				ClearAnyEntitybankInternalTextures(void);

DARKSDK LPGGTEXTURE			MakeImage						( int iID, int iWidth, int iHeight );	// makes a new image
DARKSDK LPGGTEXTURE			MakeImageUsage					( int iID, int iWidth, int iHeight, DWORD dwUsage );
DARKSDK LPGGTEXTURE			MakeImageJustFormat				( int iID, int iWidth, int iHeight, GGFORMAT Format );
DARKSDK LPGGTEXTURE			MakeImageRenderTarget			( int iID, int iWidth, int iHeight, GGFORMAT Format );

DARKSDK void				SetCubeFace					( int iID, LPGGCUBETEXTURE pCubeMap, int iFace );
DARKSDK void				GetCubeFace					( int iID, LPGGCUBETEXTURE* ppCubeMap, int* piFace );
	
DARKSDK LPGGTEXTURE			GetImagePointer					( int iID );		// get the image
DARKSDK LPGGTEXTUREREF		GetImagePointerView				( int iID );	// get the image ref (DX11 view)
DARKSDK void				SetImagePointerView				( int iID, LPGGRENDERTARGETVIEW pView );
DARKSDK int					ImageWidth						( int iID );		// get image width
DARKSDK int					ImageHeight						( int iID );		// get image height
DARKSDK float				ImageUMax						( int iID );
DARKSDK float				ImageVMax						( int iID );

DARKSDK void				SetImageSharing					( bool bMode );					// set image share mode
DARKSDK void				SetImageMemory					( int iMode );					// set memory location

DARKSDK void				LockImage						( int iID );													// lock image
DARKSDK void				UnlockImage						( int iID );													// unlock image
DARKSDK void				WriteImage						( int iID, int iX, int iY, int iA, int iR, int iG, int iB );	// write data to image
DARKSDK void				GetImage						( int iID, int iX, int iY, int* piR, int* piG, int* piB );		// get pixel data from an image

DARKSDK void				SetMipmapMode				( bool bMode );		// set mip map mode on / off
DARKSDK void 				SetMipmapType 				( int iType );		// set type e.g linear
DARKSDK void				SetMipmapBias 				( float fBias );		// set mip map bias
DARKSDK void 				SetMipmapNum  				( int iNum );		// set num of mip maps when image is loaded
DARKSDK void 				SetImageTranslucency 			( int iID, int iPercent );			// set translucency

DARKSDK bool 				LoadImageCore				( char* szFilename, int iID );							// load an image specifying the filename
DARKSDK bool 				LoadImageCore				( char* szFilename, LPGGTEXTURE* pImage );		// loads an image direct to texture
DARKSDK bool 				LoadImageCoreFullTex		( char* szFilename, LPGGTEXTURE* pImage, int iFullTexMode, int iDivideTextureSize );		// loads an image direct to texture
DARKSDK bool 				LoadImageCoreRetainName		( char* szRealName, char* szFilename, int iID, int TextureFlag, bool bIgnoreNegLimit, int iDivideTextureSize );	
DARKSDK bool 				LoadImageCore				( char* szFilename, int iID, int TextureFlag, bool bIgnoreNegLimit, int iDivideTextureSize );			// load an image specifying the filename
DARKSDK bool 				SaveImageCore				( char* szFilename, int iID );							// saves the image to disk
DARKSDK bool 				SaveImageCore				( char* szFilename, int iID, int iCompressionMode );							// saves the image to disk
DARKSDK bool 				GrabImageCore 				( int iID, int iX1, int iY1, int iX2, int iY2 );		// grab image
DARKSDK bool 				GrabImageCore 				( int iID, int iX1, int iY1, int iX2, int iY2, int iTextureFlag );		// grab image
DARKSDK void				TransferImage				( int iDestImgID, int iSrcImageID, int iTransferMode, int iMemblockAssistor );
DARKSDK void 				PasteImageCore				( int iID, int iX, int iY );								// paste image to backbuffer
DARKSDK void 				PasteImageCore				( int iID, int iX, int iY, int iFlag );								// paste image to backbuffer
DARKSDK void 				DeleteImageCore				( int iID );											// delete an image in the list
DARKSDK bool 				ImageExist					( int iID );											// does image exist
DARKSDK LPSTR 				GetImageName				( int iID );

DARKSDK void 				OpenImageBlock				( char* szFilename, int iMode );
DARKSDK void 				ExcludeFromImageBlock		( char* szExcludePath );
DARKSDK void 				CloseImageBlock				( void );
DARKSDK void 				PerformChecklistForImageBlockFiles ( void );
DARKSDK bool				AddToImageBlock				( LPSTR pAddFilename );
DARKSDK LPSTR				RetrieveFromImageBlock		( LPSTR pRetrieveFilename, DWORD* pdwFileSize );
DARKSDK int 				GetImageFileExist			( LPSTR pFilename );

DARKSDK void 				LoadImage						( LPSTR szFilename, int iID );							// load an image specifying the filename
DARKSDK void 				LoadImage						( LPSTR szFilename, int iID, int TextureFlag );							// load an image specifying the filename
DARKSDK void 				LoadImage						( LPSTR szFilename, int iID, int TextureFlag, int iDivideTextureSize );	
DARKSDK void				LoadImage						( LPSTR szFilename, int iID, int TextureFlag, int iDivideTextureSize, int iSilentError );
DARKSDK void 				SaveImage						( LPSTR szFilename, int iID );							// saves the image to disk
DARKSDK void 				SaveImage						( LPSTR szFilename, int iID, int iCompressionMode );
DARKSDK void				CreateReplaceImage				( int iID, int iTexSize, ID3D11Texture2D* pTex, ID3D11ShaderResourceView* pView );
DARKSDK void 				GrabImage						( int iID, int iX1, int iY1, int iX2, int iY2 );		// grab image
DARKSDK void 				GrabImage						( int iID, int iX1, int iY1, int iX2, int iY2, int iTextureFlag );		// grab image
DARKSDK void 				PasteImage						( int iID, int iX, int iY );								// paste image to backbuffer
DARKSDK void 				PasteImage						( int iID, int iX, int iY, int iFlag );					// paste image to backbuffer
DARKSDK void 				DeleteImage						( int iID );											// delete an image in the list
DARKSDK void 				RotateImage						( int iID, int iAngle );											// delete an image in the list
DARKSDK int  				GetImageExistEx					( int iID );											// does image exist

DARKSDK void  				SetImageColorKey					( int iR, int iG, int iB );		// set image color key
DARKSDK DWORD 				LoadIcon					( LPSTR pFilename );
DARKSDK void  				FreeIcon					( DWORD dwIconHandle );

DARKSDK void 				GetImageData				( int iID, DWORD* dwWidth, DWORD* dwHeight, DWORD* dwDepth, LPSTR* pData, DWORD* dwDataSize, bool bLockData );
DARKSDK void 				SetImageData				( int iID, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, LPSTR pData, DWORD dwDataSize );

DARKSDK void 				SetImageAutoMipMap			( int iGenerateMipMaps );

DARKSDK bool				LoadAndSaveUsingDirectXTex  ( LPSTR pLoadFile, LPSTR pSaveFile );

#endif _CIMAGE_H_
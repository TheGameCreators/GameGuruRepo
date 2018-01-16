// INCLUDES / LIBS ///////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#include <DDSTextureLoader.h>
#include "DirectXTex.h"

#include "cimagec.h"
#include ".\..\error\cerror.h"
#include "globstruct.h"
#include <stdio.h>
#include <direct.h>
#include <vector>
#include <map>
#include ".\..\Core\SteamCheckForWorkshop.h"

#include "CSpritesC.h"
#include "CMemblocks.h"
#include "CObjectsC.h"
#include "DarkLUA.h"

extern GlobStruct* g_pGlob;
extern LPGG							m_pDX;
extern LPGGDEVICE					m_pD3D;

namespace
{
    typedef std::map<int, tagImgData*>		ImageList_t;
    typedef ImageList_t::iterator			ImagePtr;

    // Image Block Globals

    bool								g_bImageBlockActive = false;
    LPSTR								g_iImageBlockFilename = NULL;
    LPSTR								g_iImageBlockRootPath = NULL;
    char								g_pImageBlockExcludePath[512];
    int									g_iImageBlockMode = -1;
    DWORD								g_dwImageBlockSize = 0;
    LPSTR								g_pImageBlockPtr = NULL;
    std::vector<LPSTR>					g_ImageBlockListFile;
    std::vector<DWORD>					g_ImageBlockListOffset;
    std::vector<DWORD>					g_ImageBlockListSize;

    ImageList_t							m_List;
    int									m_iWidth		= 0;				// width of current texture
    int									m_iHeight		= 0;				// height of current texture
    int									m_iMipMapNum	= -1;				// default number of mipmaps
    int									m_iMemory		= 0;				// default memory pool
    bool								m_bSharing		= true;				// sharing flag
    bool								m_bMipMap		= true;				// mipmap on / off
    GGCOLOR								m_Color         = GGCOLOR_ARGB ( 255, 0, 0, 0 );// default transparent color
    tagImgData*							m_imgptr = NULL;
    int									m_CurrentId = 0;
    GGFORMAT							g_DefaultGGFORMAT;
	DWORD								g_dwMipMapGenMode;

    bool RemoveImage( int iID )
    {
        // Clear the cached value if the image being deleted is the current cached image.
        if (m_CurrentId == iID)
        {
            m_CurrentId = 0;
            m_imgptr = NULL;
        }

        // Locate the image, and if found, release all of it's resources.
        ImagePtr pImage = m_List.find( iID );
        if (pImage != m_List.end())
        {
			#ifdef DX11
            SAFE_RELEASE( pImage->second->lpTextureView );
            //SAFE_RELEASE( pImage->second->lpTexture ); released when above view released
			#else
            SAFE_RELEASE( pImage->second->lpTexture );
			#endif
            SAFE_DELETE( pImage->second->lpName );
            delete pImage->second;

            m_List.erase(pImage);

            return true;
        }

        return false;
    }

    bool UpdatePtrImage ( int iID )
    {
        // If the image required is not already cached, refresh the cached value
        if (!m_imgptr || iID != m_CurrentId)
        {
            m_CurrentId = iID;

            ImagePtr p = m_List.find( iID );
            if (p == m_List.end())
            {
                m_imgptr = NULL;
            }
            else
            {
                m_imgptr = p->second;
            }
        }

        return m_imgptr != NULL;
    }
}

DARKSDK int GetPowerSquareOfSize( int Size );

DARKSDK void ImageConstructorD3D ( void )
{
	// setup the image library
	m_iMipMapNum			= 9;
}

DARKSDK void ImageConstructor ( void )
{
	ImageConstructorD3D ( );
	ImagePassCoreData ( NULL );
}

DARKSDK void ImageDestructorD3D ( void )
{
    m_CurrentId = 0;
    m_imgptr = NULL;

    for (ImagePtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
    {
        // Release the texture and texture name
        tagImgData* ptr = pCheck->second;
		#ifdef DX11
        SAFE_RELEASE( ptr->lpTextureView );
        SAFE_RELEASE( ptr->lpTexture );
		#else
        SAFE_RELEASE( ptr->lpTexture );
		#endif
        SAFE_DELETE( ptr->lpName );

        // Release the rest of the image storage
        delete ptr;

        // NOTE: Not removing from m_List at this point:
        // 1 - it makes moving to the next item harder
        // 2 - it's less efficient - we'll clear the entire list at the end
    }

    // Now clear the list
    m_List.clear();

}

DARKSDK void ImageDestructor ( void )
{
	ImageDestructorD3D();
}

DARKSDK void ImageSetErrorHandler ( LPVOID pErrorHandlerPtr )
{
	// Update error handler pointer
	g_pErrorHandler = (CRuntimeErrorHandler*)pErrorHandlerPtr;
}

DARKSDK void PassSpriteInstance (  )
{
}

#ifdef DX11
DARKSDK void CreateShaderResourceViewFor ( tagImgData* pImgPtr, int iTextureFlag, GGFORMAT format )
{
	pImgPtr->lpTextureView = NULL;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = -1;
	if ( iTextureFlag == 2 ) 
	{
		// cube needs a shader cube view
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	}
	HRESULT hr = m_pD3D->CreateShaderResourceView(pImgPtr->lpTexture, &shaderResourceViewDesc, &pImgPtr->lpTextureView);
	if ( FAILED ( hr ) )
	{
		Error ( "Failed to create resource view for image" );
		return;
	}
}
#endif

DARKSDK int ImageGetBitDepthFromFormat(GGFORMAT Format)
{
	#ifdef DX11
	switch(Format)
	{
		case DXGI_FORMAT_UNKNOWN : 
			return 0;

		case DXGI_FORMAT_R32G32B32A32_TYPELESS : 
		case DXGI_FORMAT_R32G32B32A32_FLOAT : 
		case DXGI_FORMAT_R32G32B32A32_UINT : 
		case DXGI_FORMAT_R32G32B32A32_SINT : 
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS : 
		case DXGI_FORMAT_R32G32B32_FLOAT : 
		case DXGI_FORMAT_R32G32B32_UINT : 
		case DXGI_FORMAT_R32G32B32_SINT : 
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS : 
		case DXGI_FORMAT_R16G16B16A16_FLOAT : 
		case DXGI_FORMAT_R16G16B16A16_UNORM : 
		case DXGI_FORMAT_R16G16B16A16_UINT : 
		case DXGI_FORMAT_R16G16B16A16_SNORM : 
		case DXGI_FORMAT_R16G16B16A16_SINT : 
		case DXGI_FORMAT_R32G32_TYPELESS : 
		case DXGI_FORMAT_R32G32_FLOAT : 
		case DXGI_FORMAT_R32G32_UINT : 
		case DXGI_FORMAT_R32G32_SINT : 
		case DXGI_FORMAT_R32G8X24_TYPELESS : 
			return 64;

		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT : 
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS : 
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : 
		case DXGI_FORMAT_R10G10B10A2_TYPELESS : 
		case DXGI_FORMAT_R10G10B10A2_UNORM : 
		case DXGI_FORMAT_R10G10B10A2_UINT : 
		case DXGI_FORMAT_R11G11B10_FLOAT : 
		case DXGI_FORMAT_R8G8B8A8_TYPELESS : 
		case DXGI_FORMAT_B8G8R8A8_UNORM : 
		case DXGI_FORMAT_R8G8B8A8_UNORM : 
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : 
		case DXGI_FORMAT_R8G8B8A8_UINT : 
		case DXGI_FORMAT_R8G8B8A8_SNORM : 
		case DXGI_FORMAT_R8G8B8A8_SINT : 
		case DXGI_FORMAT_R16G16_TYPELESS : 
		case DXGI_FORMAT_R16G16_FLOAT : 
		case DXGI_FORMAT_R16G16_UNORM : 
		case DXGI_FORMAT_R16G16_UINT : 
		case DXGI_FORMAT_R16G16_SNORM : 
		case DXGI_FORMAT_R16G16_SINT : 
		case DXGI_FORMAT_R32_TYPELESS : 
		case DXGI_FORMAT_D32_FLOAT : 
		case DXGI_FORMAT_R32_FLOAT : 
		case DXGI_FORMAT_R32_UINT : 
		case DXGI_FORMAT_R32_SINT : 
		case DXGI_FORMAT_R24G8_TYPELESS : 
		case DXGI_FORMAT_D24_UNORM_S8_UINT : 
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS : 
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT : 
			return 32;

		case DXGI_FORMAT_R8G8_TYPELESS : 
		case DXGI_FORMAT_R8G8_UNORM : 
		case DXGI_FORMAT_R8G8_UINT : 
		case DXGI_FORMAT_R8G8_SNORM : 
		case DXGI_FORMAT_R8G8_SINT : 
		case DXGI_FORMAT_R16_TYPELESS : 
		case DXGI_FORMAT_R16_FLOAT : 
		case DXGI_FORMAT_D16_UNORM : 
		case DXGI_FORMAT_R16_UNORM : 
		case DXGI_FORMAT_R16_UINT : 
		case DXGI_FORMAT_R16_SNORM : 
		case DXGI_FORMAT_R16_SINT : 
			return 16;

		case DXGI_FORMAT_R8_TYPELESS : 
		case DXGI_FORMAT_R8_UNORM : 
		case DXGI_FORMAT_R8_UINT : 
		case DXGI_FORMAT_R8_SNORM : 
		case DXGI_FORMAT_R8_SINT : 
		case DXGI_FORMAT_A8_UNORM : 
			return 8;
	}
	return 0;
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
	return 0;
	#endif
}

DARKSDK void ImagePassCoreDataDX9 ( LPVOID pGlobPtr )
{
	#ifndef DX11
	// only if have display
	LPGGSURFACE pBackBuffer = g_pGlob->pCurrentBitmapSurface;
	if ( m_pDX == NULL || pBackBuffer == NULL )
		return;

	// Get default GGFORMAT from backbuffer
	D3DSURFACE_DESC backdesc;
	if(pBackBuffer)
	{
		HRESULT hRes = pBackBuffer->GetDesc(&backdesc);
		DWORD dwDepth=ImageGetBitDepthFromFormat(backdesc.Format);
		if(dwDepth==16) g_DefaultGGFORMAT = GGFMT_A1R5G5B5;
		if(dwDepth==32) g_DefaultGGFORMAT = GGFMT_A8R8G8B8;
	}
	else
	{
		g_DefaultGGFORMAT = GGFMT_A8R8G8B8;
	}

	// Ensure textureformat is valid, else choose next valid..
	HRESULT hRes = m_pDX->CheckDeviceFormat(	GGADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												backdesc.Format,
												0, D3DRTYPE_TEXTURE,
												g_DefaultGGFORMAT);
	if ( FAILED( hRes ) )
	{
		// Need another texture format with an alpha
		for(DWORD t=0; t<12; t++)
		{
			switch(t)
			{
				case 0  : g_DefaultGGFORMAT = GGFMT_A8R8G8B8;		break;
				case 1  : g_DefaultGGFORMAT = GGFMT_X8R8G8B8;		break;
				case 2  : g_DefaultGGFORMAT = GGFMT_A1R5G5B5;		break;
				case 3  : g_DefaultGGFORMAT = GGFMT_A2B10G10R10;	break;
				case 4  : g_DefaultGGFORMAT = GGFMT_A4R4G4B4;		break;
				case 5  : g_DefaultGGFORMAT = GGFMT_A8R3G3B2;		break;
				case 6  : g_DefaultGGFORMAT = GGFMT_R8G8B8;		break;
				case 7  : g_DefaultGGFORMAT = GGFMT_R5G6B5;		break;
				case 8  : g_DefaultGGFORMAT = GGFMT_X1R5G5B5;		break;
				case 9  : g_DefaultGGFORMAT = GGFMT_R3G3B2;		break;
				case 10 : g_DefaultGGFORMAT = GGFMT_X4R4G4B4;		break;
				case 11 : g_DefaultGGFORMAT = GGFMT_G16R16;		break;
			}
			HRESULT hRes = m_pDX->CheckDeviceFormat(	GGADAPTER_DEFAULT,
														D3DDEVTYPE_HAL,
														backdesc.Format,
														0, D3DRTYPE_TEXTURE,
														g_DefaultGGFORMAT);
			if ( SUCCEEDED( hRes ) )
			{
				// Found a texture we can use
				return;
			}
		}
	}
	#endif
}

DARKSDK void ImagePassCoreDataDX11 ( LPVOID pGlobPtr )
{
	//  work out default GGFORMAT for current device
	#ifdef DX11
	g_DefaultGGFORMAT = GGFMT_A8R8G8B8;
	#endif
}

DARKSDK void ImagePassCoreData( LPVOID pGlobPtr )
{
	#ifdef DX11
	ImagePassCoreDataDX11(pGlobPtr);
	#else
	ImagePassCoreDataDX9(pGlobPtr);
	#endif
}

DARKSDK void ImageRefreshGRAFIX ( int iMode )
{
	if(iMode==0)
	{
		// Remove all traces of old D3D usage
		ImageDestructorD3D();
	}
	if(iMode==1)
	{
		// Get new D3D and recreate everything D3D related
		ImageConstructorD3D ( );
		ImagePassCoreData ( g_pGlob );
		PassSpriteInstance ( );
	}
}

void GetFileInMemory ( LPSTR szFilename, LPVOID* ppFileInMemoryData, DWORD* pdwFileInMemorySize, LPSTR pFinalRelPathAndFileRef )
{
	*pdwFileInMemorySize = 0;
	*ppFileInMemoryData = NULL;
	if ( g_bImageBlockActive )
	{
		// final storage string of path and file resolver (makes the filename and path uniform for imageblock retrieval)
		char pFinalRelPathAndFile[512];

		// store old directory
		char pOldDir [ 512 ];
		_getcwd ( pOldDir, 512 );

		// get combined path only
		char pPath[1024];
		char pFile[1024];
		strcpy ( pPath, pOldDir );
		strcat ( pPath, "\\" );
		strcat ( pPath, szFilename );
		strcat ( pFile, "" );
		for ( int n=strlen(pPath); n>0; n-- )
		{
			if ( pPath[n]=='\\' || pPath[n]=='/' )
			{
				// split file and path
				strcpy ( pFile, pPath + n + 1 );
				pPath [ n + 1 ] = 0;
				break;
			}
		}

		// Combine current working folder and filename to get a resolved path (removes ..\..\ stuff)
		char pResolvedDir[512];
		strcpy ( pResolvedDir, pPath );

		// Remove the part which represents the root location of the Image Block (g_iImageBlockRootPath)
		if ( strlen ( pResolvedDir ) <= strlen(g_iImageBlockRootPath) )
			strcpy ( pFinalRelPathAndFile, "" );
		else
			strcpy ( pFinalRelPathAndFile, pResolvedDir + strlen(g_iImageBlockRootPath) );

		// Ensure a \ is added
		if ( strlen ( pFinalRelPathAndFile ) > 0 )
		{
			if ( pFinalRelPathAndFile [ strlen(pFinalRelPathAndFile)-1 ]!='\\' )
			{
				// add folder divide at end of path string
				int iLen = strlen(pFinalRelPathAndFile);
				pFinalRelPathAndFile [ iLen+0 ] = '\\';
				pFinalRelPathAndFile [ iLen+1 ] = 0;
			}
		}

		// Add the filename back in
		strcat ( pFinalRelPathAndFile, pFile );

		// Restore folder
		_chdir ( pOldDir );

		// Retrieve file in memory
		if ( g_iImageBlockMode==1 )
		{
			*ppFileInMemoryData = RetrieveFromImageBlock ( pFinalRelPathAndFile, pdwFileInMemorySize );
		}

		// copy final rel path and file
		strcpy ( pFinalRelPathAndFileRef, pFinalRelPathAndFile );
	}
	else
	{
		strcpy ( pFinalRelPathAndFileRef, "" );
	}
}

DARKSDK LPGGTEXTURE GetTextureCore ( char* szFilename, GGIMAGE_INFO* info, int iOneToOnePixels, int iFullTexturePlateMode, int iDivideTextureSize )
{
	// new feature IMAGEBLOCK
	DWORD dwFileInMemorySize = 0;
	LPVOID pFileInMemoryData = NULL;
	if ( g_bImageBlockActive )
	{
		// final storage string of path and file resolver (makes the filename and path uniform for imageblock retrieval)
		// and work out true file and path, then look for it in imageblock
		char pFinalRelPathAndFile[512];
		GetFileInMemory ( szFilename, &pFileInMemoryData, &dwFileInMemorySize, pFinalRelPathAndFile );

		// Add relative path and file to image block
		if ( g_iImageBlockMode==0 )
		{
			_chdir ( g_iImageBlockRootPath );
			AddToImageBlock ( pFinalRelPathAndFile );
		}
	}

	// loads a texture and returns a pointer to it make sure that the device is valid
	if ( !m_pD3D )
		return NULL;

	// variable declarations
	LPGGTEXTURE	lpTexture = NULL;	// set texture to null

	// get file image info
	HRESULT hRes = 0;
	if ( g_iImageBlockMode==1 && pFileInMemoryData )
	{
		#ifdef DX11
		hRes = D3DX11GetImageInfoFromMemory( pFileInMemoryData, dwFileInMemorySize, NULL, info, NULL );
		if (FAILED(hRes)) hRes = D3DX11GetImageInfoFromFile( szFilename, NULL, info, NULL );
		#else
		hRes = D3DXGetImageInfoFromFileInMemory( pFileInMemoryData, dwFileInMemorySize, info );
		if (FAILED(hRes)) hRes = D3DXGetImageInfoFromFile( szFilename, info );
		#endif
	}
	else
	{
		#ifdef DX11
		hRes = D3DX11GetImageInfoFromFile( szFilename, NULL, info, NULL );
		#else
		hRes = D3DXGetImageInfoFromFile( szFilename, info );
		#endif
	}

	// If failed to get image information, then can't be any image there either
	if (FAILED(hRes))
		return NULL;

	// if texture size needs diviing, do so now
	if ( iDivideTextureSize>0 )
	{
		if ( iDivideTextureSize==16384 )
		{
			// notextureloadmode
			(*info).Width = 1;
			(*info).Height = 1;
		}
		else
		{
			// divide by specified value (reduce texture consumption)
			(*info).Width /= iDivideTextureSize;
			(*info).Height /= iDivideTextureSize;
			if ( (*info).Width < 4 ) (*info).Width = 4;
			if ( (*info).Height < 4 ) (*info).Height = 4;
		}
	}

	// if mode is CUBE(2) or VOLUME(3), direct cube loader
	if ( iFullTexturePlateMode==2 || iFullTexturePlateMode==3 || iDivideTextureSize==16384 )
	{
		if ( iDivideTextureSize==16384 )
		{
			// support for quick-fake-texture-load (apply texture to scene without loading it)
			LPGGTEXTURE pFakeTex = NULL;
			#ifdef DX11
			#else
			hRes = D3DXCreateTexture ( m_pD3D,
									   (*info).Width,
									   (*info).Height,
									   1,//one mipmap only for one-to-one pixels
									   0,
									   g_DefaultGGFORMAT,
									   D3DPOOL_MANAGED,
									   &pFakeTex );
			#endif
			lpTexture = pFakeTex;
		}
		else
		{
			if ( iFullTexturePlateMode==2 ) 
			{
				// support for cube textures when specify texture flag of two (2)
				HRESULT hRes = 0;
				LPGGCUBETEXTURE pCubeTex = NULL;
				#ifdef DX11
				D3DX11_IMAGE_LOAD_INFO loadinfo;
				loadinfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
				hRes = D3DX11CreateTextureFromFile(	m_pD3D, szFilename, &loadinfo, NULL, &pCubeTex, NULL );
				#else
				if ( g_iImageBlockMode==1 && pFileInMemoryData )
					hRes = D3DXCreateCubeTextureFromFileInMemoryEx( m_pD3D, pFileInMemoryData, dwFileInMemorySize, (*info).Width, D3DX_DEFAULT, 0, GGFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pCubeTex );
				else
					hRes = D3DXCreateCubeTextureFromFileEx( m_pD3D, szFilename, (*info).Width, D3DX_DEFAULT, 0, GGFMT_UNKNOWN, 
					D3DPOOL_DEFAULT,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
					D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pCubeTex );
				#endif
				if ( SUCCEEDED( hRes ) ) 
				{
					// cube loaded fine
					lpTexture = (LPGGTEXTURE)pCubeTex;
				}
				else
					return NULL;
			}
			if ( iFullTexturePlateMode==3 ) 
			{
				// support for volume textures when specify 3
				HRESULT hRes = 0;
				#ifdef DX11
				#else
				LPDIRECT3DVOLUMETEXTURE9 pVolumeTex = NULL;
				if ( g_iImageBlockMode==1 && pFileInMemoryData )
					hRes = D3DXCreateVolumeTextureFromFileInMemoryEx( m_pD3D, pFileInMemoryData, dwFileInMemorySize,
								(*info).Width, (*info).Height, (*info).Depth, 1, 0, GGFMT_UNKNOWN,
								D3DPOOL_DEFAULT,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM! 
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
								&pVolumeTex );
				else
					hRes = D3DXCreateVolumeTextureFromFileEx ( m_pD3D, szFilename, 
								(*info).Width, (*info).Height, (*info).Depth, 3, 0, GGFMT_UNKNOWN,
								D3DPOOL_DEFAULT,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
								D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,
								&pVolumeTex );
				if ( SUCCEEDED( hRes ) ) 
				{
					// volume loaded fine
					lpTexture = (LPGGTEXTURE)pVolumeTex;
				}
				else
					return NULL;
				#endif
			}
		}
	}
	else
	{
		// texture flag can control if image is GPU only or MANAGED (SYS memory copy)
		#ifdef DX11

		// not unknown, use file format
		GGFORMAT newImageFormat = (*info).Format;
					
		// if DDS, load directly with original mipmap data intact
		if ( g_iImageBlockMode==1 && pFileInMemoryData )
		{
			//hRes = D3DX11CreateTextureFromFileInMemoryEx(	m_pD3D,	pFileInMemoryData, dwFileInMemorySize, info.Width,info.Height,D3DX_DEFAULT,0,GGFMT_UNKNOWN,
			//dwPoolType,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
			//D3DX_DEFAULT,D3DX_DEFAULT,0,&info,NULL,&lpTexture );
		}
		else
		{
			////if ( newImageFormat >= DXGI_FORMAT_BC1_TYPELESS && newImageFormat <= DXGI_FORMAT_BC7_UNORM_SRGB )
			////{
			////	// but does not work for compressed textures
			////}
			////else
			////{
			bool bFormatDDSLoadFriendly = true;
			if ( (*info).Format >= DXGI_FORMAT_R8G8B8A8_TYPELESS && (*info).Format <= DXGI_FORMAT_R8G8B8A8_SINT ) bFormatDDSLoadFriendly = false;
			if ( m_iMipMapNum == -1 && (*info).MipLevels == 1 && (*info).Format >= DXGI_FORMAT_BC1_TYPELESS && (*info).Format <= DXGI_FORMAT_BC5_SNORM ) bFormatDDSLoadFriendly = false;
			if ( strnicmp ( szFilename + strlen(szFilename) - 4, ".dds", 4 ) == NULL && bFormatDDSLoadFriendly == true )
			{
				if ( iDivideTextureSize <= 1 )
				{
					// effort to speed up loading of DDS texture files (above took 0.1s per 512K texture, .4s per 2K texture)
					// as above, you cannot auto gen mipmaps if the texture is compressed (do this as part of file)
					size_t origsize = strlen(szFilename) + 1;
					const size_t newsize = 1024;
					size_t convertedChars = 0;
					wchar_t wcstringTextureFilename[newsize];
					mbstowcs_s(&convertedChars, wcstringTextureFilename, origsize, szFilename, _TRUNCATE);
					hRes = DirectX::CreateDDSTextureFromFile ( m_pD3D, m_pImmediateContext, wcstringTextureFilename, &lpTexture, NULL );
				}
				else
				{
					// meets all fast load requirements, but need to reduce texture size when loading
					wchar_t wTexFilename[512];
					MultiByteToWideChar(CP_ACP, 0, szFilename, -1, wTexFilename, sizeof(wTexFilename));
					DirectX::TexMetadata imageData;
					DirectX::ScratchImage imageTexture;
					hRes = GetMetadataFromDDSFile( wTexFilename, DirectX::DDS_FLAGS_NONE, imageData );			
					hRes = LoadFromDDSFile( wTexFilename, DirectX::DDS_FLAGS_NONE, &imageData, imageTexture );
					DirectX::ScratchImage* pWrkImage = &imageTexture;
					DirectX::ScratchImage convertedTexture;
					bool bWasCompressed = false;
					DXGI_FORMAT storeFormat = imageTexture.GetMetadata().format;
					if ( storeFormat >= DXGI_FORMAT_BC1_TYPELESS && storeFormat <= DXGI_FORMAT_BC5_SNORM )
					{
						hRes = DirectX::Decompress( imageTexture.GetImages(), imageTexture.GetImageCount(), imageTexture.GetMetadata(), DXGI_FORMAT_B8G8R8A8_UNORM, convertedTexture );
						pWrkImage = &convertedTexture;
						imageTexture.Release();
						bWasCompressed = true;
					}
					DirectX::ScratchImage resizedTexture;
					hRes = Resize( pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), (*info).Width, (*info).Height, DirectX::TEX_FILTER_SEPARATE_ALPHA, resizedTexture );
					pWrkImage->Release();
					DirectX::ScratchImage resizedCompressedTexture;
					if ( bWasCompressed == true )
					{
						hRes = DirectX::Compress( resizedTexture.GetImages(), resizedTexture.GetImageCount(), resizedTexture.GetMetadata(), 
						storeFormat, DirectX::TEX_COMPRESS_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, resizedCompressedTexture );
						pWrkImage = &resizedCompressedTexture;
					}
					else
						pWrkImage = &resizedTexture;
					hRes = DirectX::CreateTexture ( m_pD3D, pWrkImage->GetImages(), pWrkImage->GetImageCount(), pWrkImage->GetMetadata(), &lpTexture );
					pWrkImage->Release();
				}
			}
			else
			{
				// to conform to internal BGRA format (DX9 to DX11 nonesense)
				D3DX11_IMAGE_LOAD_INFO loadinfo;
				loadinfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM; 
				(*info).Format = loadinfo.Format;
				loadinfo.MipLevels = m_iMipMapNum; // set using SetMipmapNum when want no mipmaps (i.e. vegmask)
				loadinfo.Width = (*info).Width;
				loadinfo.Height = (*info).Height;
				hRes = D3DX11CreateTextureFromFile(	m_pD3D, szFilename, &loadinfo, NULL, &lpTexture, NULL );
			}
		}

		// 010205 - default mem can run out
		if ( lpTexture==NULL )
			return NULL;

		// adjust to actual size if texture smaller
		//D3DSURFACE_DESC desc;
		//lpTexture->GetLevelDesc(0,&desc);
		//if(desc.Width<info.Width) info.Width=desc.Width;
		//if(desc.Height<info.Height) info.Height=desc.Height;

		#else
		D3DPOOL dwPoolType = D3DPOOL_DEFAULT;
		if ( iFullTexturePlateMode==10 )
			dwPoolType = D3DPOOL_MANAGED;

		// perfect texture is one-to-one pixels and no mipmaps or alpha
		if ( iOneToOnePixels==1 )
		{
			// Keep Alpha from file
			DWORD dwUseAlphaCode = 0;
			if(_strnicmp( szFilename + (strlen(szFilename)-4), ".dds", 4)==NULL
			|| _strnicmp( szFilename + (strlen(szFilename)-4), ".png", 4)==NULL
			|| _strnicmp( szFilename + (strlen(szFilename)-4), ".tga", 4)==NULL)
				dwUseAlphaCode = 0;
			else
				dwUseAlphaCode = m_Color;

			// leeadd - 300305 - format selection (replaced info.Format or g_DefaultGGFORMAT in createtexture as we need SET IMAGE COLORKEY to retain alpha!)
			GGFORMAT newImageFormat = (*info).Format;
			if ( dwUseAlphaCode!=0 ) newImageFormat = g_DefaultGGFORMAT;

			// create a new texture/image
			hRes = D3DXCreateTexture ( m_pD3D,
									   (*info).Width,
									   (*info).Height,
									   1,//one mipmap only for one-to-one pixels
									   0,
									   newImageFormat,
									   dwPoolType,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
									   &lpTexture );

			// 010205 - default mem can run out
			if ( lpTexture==NULL )
				return NULL;

			// adjust to actual size if texture smaller
			D3DSURFACE_DESC desc;
			lpTexture->GetLevelDesc(0,&desc);
			if(desc.Width<(*info).Width) (*info).Width=desc.Width;
			if(desc.Height<(*info).Height) (*info).Height=desc.Height;
			if( SUCCEEDED ( hRes ))
			{
				LPGGSURFACE pSurface=NULL;
				hRes = lpTexture->GetSurfaceLevel(0, &pSurface);
				if( SUCCEEDED ( hRes ))
				{
					// load surface data into it
					RECT destrc = { 0, 0, (LONG)(*info).Width, (LONG)(*info).Height };
					if ( g_iImageBlockMode==1 && pFileInMemoryData )
						hRes = D3DXLoadSurfaceFromFileInMemory( pSurface, NULL, &destrc, pFileInMemoryData, dwFileInMemorySize, NULL, D3DX_FILTER_POINT, dwUseAlphaCode, info );
					else
						hRes = D3DXLoadSurfaceFromFile( pSurface, NULL, &destrc, szFilename, NULL, D3DX_FILTER_POINT, dwUseAlphaCode, info );
					pSurface->Release();
				}
			}
            // If any of the previous steps failed, release the target texture
            // to signal a failure to load.
            if ( FAILED ( hRes ) )
                SAFE_RELEASE( lpTexture );
		}
		else
		{
			// DDS or Other File Format (leefix - 220303 - added TGA to keep alpha load)
			if(_strnicmp( szFilename + (strlen(szFilename)-4), ".dds", 4)==NULL
			|| _strnicmp( szFilename + (strlen(szFilename)-4), ".png", 4)==NULL
			|| _strnicmp( szFilename + (strlen(szFilename)-4), ".tga", 4)==NULL)
			{
				// lee - 180406 - u6rc10 - not unknown, use file format
				GGFORMAT newImageFormat = (*info).Format;
					
				// if DDS, load directly with original mipmap data intact
				if ( g_iImageBlockMode==1 && pFileInMemoryData )
					hRes = D3DXCreateTextureFromFileInMemoryEx(	m_pD3D,	pFileInMemoryData, dwFileInMemorySize, (*info).Width,(*info).Height,D3DX_DEFAULT,0,GGFMT_UNKNOWN,
					dwPoolType,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
					D3DX_DEFAULT,D3DX_DEFAULT,0,info,NULL,&lpTexture );
				else
					hRes = D3DXCreateTextureFromFileEx(	m_pD3D,
													szFilename,
													(*info).Width,
													(*info).Height,
													g_dwMipMapGenMode,//D3DX_FROM_FILE,//D3DX_DEFAULT, // 031014 - take mipmap as stored in file, DO NOT generate a whole chain!!
													0,
													newImageFormat,// 180406 - u6rc10 - GGFMT_UNKNOWN,//g_DefaultGGFORMAT,
													dwPoolType,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
													D3DX_DEFAULT,
													D3DX_DEFAULT,
													0, //m_Color, LEEFIX - DDS/TGA has its own Alphamap!
													info,
													NULL,
													&lpTexture );

				// 010205 - default mem can run out
				if ( lpTexture==NULL )
					return NULL;

				// adjust to actual size if texture smaller
				D3DSURFACE_DESC desc;
				lpTexture->GetLevelDesc(0,&desc);
				if(desc.Width<(*info).Width) (*info).Width=desc.Width;
				if(desc.Height<(*info).Height) (*info).Height=desc.Height;
			}
			else
			{
				// create a new texture/image
				hRes = D3DXCreateTexture ( 
										  m_pD3D,
										  (*info).Width,
										  (*info).Height,
										  D3DX_DEFAULT,
										  0,
										  g_DefaultGGFORMAT,
										  dwPoolType,//D3DPOOL_MANAGED, lee - 010314 - preserve SYS MEM!
										  &lpTexture	       );

				// 010205 - default mem can run out
				if ( lpTexture==NULL )
					return NULL;

				// adjust to actual size if texture smaller
				D3DSURFACE_DESC desc;
				lpTexture->GetLevelDesc(0,&desc);
				if(desc.Width<(*info).Width) (*info).Width=desc.Width;
				if(desc.Height<(*info).Height) (*info).Height=desc.Height;
				if( SUCCEEDED ( hRes ))
				{
					// get surface of texture (as many mipmap levels as it has)
					for ( DWORD level=0; level<lpTexture->GetLevelCount(); level++ )
					{
						LPGGSURFACE pSurface=NULL;
						hRes = lpTexture->GetSurfaceLevel(level, &pSurface);
						if( SUCCEEDED ( hRes ))
						{
							// load surface data into it
							// leefix-260603-ditheris aweful!
							// leefix-220703-changed again so level 0 is clean mip and rest are dithered for good distance textures
							if ( g_iImageBlockMode==1 && pFileInMemoryData )
							{
								if ( level==0 )
									hRes = D3DXLoadSurfaceFromFileInMemory( pSurface, NULL, NULL, pFileInMemoryData, dwFileInMemorySize, NULL, D3DX_FILTER_POINT, m_Color, info );
								else
									hRes = D3DXLoadSurfaceFromFileInMemory( pSurface, NULL, NULL, pFileInMemoryData, dwFileInMemorySize, NULL, D3DX_DEFAULT, m_Color, info );
							}
							else
							{
								if ( level==0 )
									hRes = D3DXLoadSurfaceFromFile( pSurface, NULL, NULL, szFilename, NULL, D3DX_FILTER_POINT, m_Color, info );
								else
									hRes = D3DXLoadSurfaceFromFile( pSurface, NULL, NULL, szFilename, NULL, D3DX_DEFAULT, m_Color, info );
							}

							pSurface->Release();
						}
						else
							break;
					}
				}
			}
		}
		#endif
	}

	// check the texture loaded in ok
	if ( !lpTexture )
		Error ( "Failed to load texture for image library" );

	// needed image info
	m_iWidth  = (*info).Width;		// file width
	m_iHeight = (*info).Height;	// file height

	// finally return the pointer
	return lpTexture;
}

DARKSDK void SetImageAutoMipMap ( int iGenerateMipMaps )
{
	#ifdef DX11
	#else
	if ( iGenerateMipMaps==1 )
		g_dwMipMapGenMode = D3DX_DEFAULT;
	else
		g_dwMipMapGenMode = D3DX_FROM_FILE;
	#endif
}

DARKSDK bool LoadImageCoreFullTex ( char* szFilename, LPGGTEXTURE* pImage, GGIMAGE_INFO* info, int iFullTexturePlateMode, int iDivideTextureSize )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);
	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media
	bool bRes = false;
	g_pGlob->Decrypt( (DWORD)VirtualFilename );

	// load the media
	*pImage = GetTextureCore ( VirtualFilename, info, 0, iFullTexturePlateMode, iDivideTextureSize );
	if ( *pImage ) bRes = true;

	// get media info
	HRESULT hRes = 0;
	if ( g_bImageBlockActive && g_iImageBlockMode==1 )
	{
		DWORD dwFileInMemorySize = 0;
		LPVOID pFileInMemoryData = NULL;
		char pFinalRelPathAndFile[512];
		GetFileInMemory ( VirtualFilename, &pFileInMemoryData, &dwFileInMemorySize, pFinalRelPathAndFile );
		#ifdef DX11
		hRes = D3DX11GetImageInfoFromMemory( pFileInMemoryData, dwFileInMemorySize, NULL, info, NULL );
		#else
		hRes = D3DXGetImageInfoFromFileInMemory( pFileInMemoryData, dwFileInMemorySize, info );
		#endif
	}

	// get info from physical file if not in image block
	#ifdef DX11
	/// 230817 - so do not overwrite info data thats been changed
	/// if ( hRes==0 || hRes!=GG_OK ) hRes = D3DX11GetImageInfoFromFile( VirtualFilename, NULL, info, NULL );
	#else
	if ( hRes==0 || hRes!=GG_OK ) hRes = D3DXGetImageInfoFromFile( VirtualFilename, info );
	#endif

	// re-encrypt if applicable
	g_pGlob->Encrypt( (DWORD)VirtualFilename );

	// success or no
	return bRes;
}

DARKSDK bool LoadImageCore ( char* szFilename, LPGGTEXTURE* pImage, GGIMAGE_INFO* info )
{
	// default is full texture plate mode zero = simple surface
	return LoadImageCoreFullTex ( szFilename, pImage, info, 0, 0 );
}

DARKSDK bool FindInternalImage ( char* szFilename, int* pImageID )
{
	if ( szFilename && szFilename[0] )
	{
    	int iFindFilenameLength = strlen(szFilename);

		ImagePtr pCheck = m_List.begin();
		while ( pCheck != m_List.end() && pCheck->first < 0)
		{
            // get a pointer to the actual data
		    tagImgData* ptr = pCheck->second;
            if ( ptr && ptr->szShortFilename && ptr->lpTexture )
		    {
			    if ( _stricmp(ptr->szShortFilename, szFilename) == 0)
			    {
				    *pImageID = pCheck->first;
				    return true;
			    }
            }
            ++pCheck;
		}
	}

    // Static, so not reset between function calls - almost guaranteed to find the
    // next free image that way.
	static int iTry=-1;

    // Check to see if the image id is in use
    ImagePtr pCheck = m_List.find(iTry);
    while (pCheck != m_List.end())
    {
        // Is in use, check for the next number, but make sure that underflow is dealt with correctly
        // ie, iTry MUST stay negative. Note that this is not too likely to happen anyway.
        --iTry;
        if (iTry > 0)
            iTry = -1;

        pCheck = m_List.find(iTry);
    }

	// this image can be used = new slot
	*pImageID = iTry;

	// not found existing image..
	return false;
}

DARKSDK void ClearAnyLightMapInternalTextures ( void )
{
	ImagePtr pCheck = m_List.begin();
	while ( pCheck != m_List.end() && pCheck->first < 0)
	{
        // get a pointer to the actual data
		tagImgData* ptr = pCheck->second;
		int iFoundID = 0;
        if ( ptr && ptr->szShortFilename && ptr->lpTexture )
		{
			if ( ptr->szShortFilename!=NULL )
			{
				if ( strlen(ptr->szShortFilename)>11 )
				{
					for ( int n=0; n<(int)strlen(ptr->szShortFilename)-11; n++ )
						if ( strnicmp ( ptr->szShortFilename + n, "lightmaps\\", 10 )==NULL )
							iFoundID = pCheck->first;
				}
			}
        }
		if ( iFoundID!=0 ) 
		{
			RemoveImage( iFoundID );
			pCheck = m_List.begin();
		}
		else
		{
	        ++pCheck;
		}
	}
}

// This load is NOT the main DBPro image loader - it is used here though (Load(x,x,x,x))
DARKSDK int LoadImageCoreInternal ( char* szFilename, int iDivideTextureSize )
{
	// does image already exist?
	int iImageID = 0;
	if ( !FindInternalImage ( szFilename, &iImageID ) )
	{
		// copy of filename to attempt to load
		char VirtualFilename[MAX_PATH];
		strcpy ( VirtualFilename, szFilename );

		CheckForWorkshopFile ( VirtualFilename );

		// no, use standard loader
		g_pGlob->Decrypt( (DWORD)VirtualFilename );
		if ( LoadImageCoreRetainName ( szFilename, (LPSTR)VirtualFilename, iImageID, 0, true, iDivideTextureSize ) )
		{
			// new image returned
		}
		else
		{
			// load failed
			iImageID=0;
		}
		g_pGlob->Encrypt( (DWORD)VirtualFilename );
	}
	return iImageID;
}

DARKSDK int LoadImageInternalEx ( char* szFilename, int iDivideTextureSize )
{
	return LoadImageCoreInternal ( szFilename, iDivideTextureSize );
}

DARKSDK int LoadImageInternal ( char* szFilename )
{
	return LoadImageCoreInternal ( szFilename, 0 );
}

DARKSDK LPGGTEXTURE GetTexture ( char* szFilename, GGIMAGE_INFO* info, int iOneToOnePixels )
{
	// Uses actual or virtual file..
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, szFilename);
	g_pGlob->UpdateFilenameFromVirtualTable( (DWORD)VirtualFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// Decrypt and use media, re-encrypt
	g_pGlob->Decrypt( (DWORD)VirtualFilename );
	LPGGTEXTURE Res = GetTextureCore ( VirtualFilename, info, iOneToOnePixels, 0, 0 );
	g_pGlob->Encrypt( (DWORD)VirtualFilename );
	return Res;
}

DARKSDK LPGGTEXTURE GetImagePointer ( int iID )
{
	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return NULL;

	if ( m_imgptr->lpTexture )
		return m_imgptr->lpTexture;
	else
		return NULL;
}

DARKSDK LPGGTEXTUREREF GetImagePointerView ( int iID )
{
	#ifdef DX11
	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return NULL;

	if ( m_imgptr->lpTextureView )
		return m_imgptr->lpTextureView;
	else
		return NULL;
	#else
	return GetImagePointer(iID);
	#endif
}

DARKSDK void SetImagePointerView ( int iID, LPGGRENDERTARGETVIEW pView )
{
	#ifdef DX11
	//hmm, maybe render target view is different from shader resource view
	//if ( !UpdatePtrImage ( iID ) ) return;
	//m_imgptr->lpTextureView = (ID3D11ShaderResourceView*)pView;
	#endif
}

DARKSDK int ImageWidth ( int iID )
{
	// get the width of an image

	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return -1;

	// return the width
	return m_imgptr->iWidth;
}

DARKSDK int ImageHeight ( int iID )
{
	// get the height of an image

	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return -1;

	// return the height
	return m_imgptr->iHeight;
}

DARKSDK float ImageUMax ( int iID )
{
	// get the height of an image

	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return 1.0f;

	// return the fTexUMax
	return m_imgptr->fTexUMax;
}

DARKSDK float ImageVMax ( int iID )
{
	// get the width of an image

	// update internal data
	if ( !UpdatePtrImage ( iID ) )
		return 1.0f;

	// return the fTexVMax
	return m_imgptr->fTexVMax;
}

DARKSDK void SetImageSharing ( bool bMode )
{
	// sets up the sharing mode, this helps to conserve
	// memory as it won't load in the same image more
	// than once

	// save mode
	m_bSharing = bMode;
}

DARKSDK void SetImageMemory ( int iMode )
{
	// sets the memory mode for the images

	// iMode possible values
	// 0 - default
	// 1 - managed ( must use this to lock textures )
	// 2 - system

	// check that the mode is valid
	if ( iMode < 0 || iMode > 2 )
		Error ( "Invalid memory mode specified" );

	// save the mode
	m_iMemory = iMode;
}

DARKSDK void LockImage ( int iID )
{
	// lock the specified image, this allows you to perform
	// actions like plotting pixels etc
	if ( !UpdatePtrImage ( iID ) )
		return;

	#ifdef DX11
	#else
	HRESULT hr;
	if ( !m_imgptr->bLocked )
	{
		memset ( &m_imgptr->d3dlr, 0, sizeof ( GGLOCKED_RECT ) );
		if ( FAILED ( hr = m_imgptr->lpTexture->LockRect ( 0, &m_imgptr->d3dlr, 0, 0 ) ) )
			Error ( "Failed to lock image for image library" );
		m_imgptr->bLocked = true;
	}
	else
		Error ( "Failed to lock image for image library as it's already locked" );
	#endif
}

DARKSDK void UnlockImage ( int iID )
{
	if ( !UpdatePtrImage ( iID ) )
		return;

	#ifdef DX11
	#else
	m_imgptr->lpTexture->UnlockRect ( NULL );
	m_imgptr->bLocked = false;
	#endif
}

DARKSDK void WriteImage ( int iID, int iX, int iY, int iA, int iR, int iG, int iB )
{
	if ( !UpdatePtrImage ( iID ) )
		return;

	if ( !m_imgptr->bLocked )
		Error ( "Unable to modify texture data for image library as it isn't locked" );
	
	#ifdef DX11
	#else
	DWORD* pPix = ( DWORD* ) m_imgptr->d3dlr.pBits;
	pPix [ ( ( m_imgptr->d3dlr.Pitch >> 4 ) * iX ) + iY ] = GGCOLOR_ARGB ( iA, iR, iG, iB );
	#endif
}

DARKSDK void GetImage ( int iID, int iX, int iY, int* piR, int* piG, int* piB )
{
	// get a pixel at x, y

	// check pointers are valid
	if ( !piR || !piG || !piB )
		return;

	// update the internal data
	if ( !UpdatePtrImage ( iID ) )
		return;

	// check the image is locked
	if ( !m_imgptr->bLocked )
		Error ( "Unable to get texture data for image library as it isn't locked" );

	#ifdef DX11
	#else
	// get a pointer to the data
	DWORD* pPix = ( DWORD* ) m_imgptr->d3dlr.pBits;
	DWORD  pGet;

	// get offset in file
	pGet = pPix [ ( ( m_imgptr->d3dlr.Pitch >> 4 ) * iX ) + iY ];

	// break value down
	DWORD dwAlpha = pGet >> 24;						// get alpha
	DWORD dwRed   = ((( pGet ) >> 16 ) & 0xff );	// get red
	DWORD dwGreen = ((( pGet ) >>  8 ) & 0xff );	// get green
	DWORD dwBlue  = ((( pGet ) )       & 0xff );	// get blue

	// save values
	*piR = dwRed;
	*piG = dwGreen;
	*piB = dwBlue;
	#endif
}

DARKSDK LPGGTEXTURE MakeFormat ( int iID, int iWidth, int iHeight, GGFORMAT format, DWORD dwUsageRenderTarget )
{
	// make a new image
	// mike : can specify iID of -1 which means image is added to list (internal use only)

	// variable declarations
	tagImgData*	test = NULL;		// pointer to data structure
	
	// create a new block of memory
	test = new tagImgData;
	memset(test, 0, sizeof(tagImgData));

	// check the memory was created
	if ( test == NULL )
		return NULL;

	// clear out the memory
	memset ( test, 0, sizeof ( tagImgData ) );

	#ifdef DX11

	// all images created need to be shader resources!
	//D3D11_USAGE dwPoolStaging = D3D11_USAGE_STAGING;
	//UINT dwCPUAccess = D3D11_CPU_ACCESS_READ;
	//D3D11_BIND_FLAG dwBind = (D3D11_BIND_FLAG)0;
	//if ( dwUsage!=0 ) 
	//{
	//	dwPoolStaging = D3D11_USAGE_DEFAULT;
	//	dwCPUAccess = 0;
	//	dwBind = D3D11_BIND_SHADER_RESOURCE;
	//}
	D3D11_USAGE dwPoolStaging = D3D11_USAGE_DEFAULT;
	UINT dwCPUAccess = 0;
	DWORD dwBind = D3D11_BIND_SHADER_RESOURCE;
	if ( dwUsageRenderTarget != 0 ) dwBind |= D3D11_BIND_RENDER_TARGET;

	// create a new texture
	GGSURFACE_DESC StagedDesc = { iWidth, iHeight, 1, 1, format, 1, 0, dwPoolStaging, (D3D11_BIND_FLAG)dwBind, dwCPUAccess, 0 };
	HRESULT hr = m_pD3D->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&test->lpTexture );
	if ( FAILED ( hr ) )
	{
		Error ( "Failed to create new image" );
		return NULL;
	}

	// setup properties
	test->iHeight = iHeight;		// store the width
	test->iWidth  = iWidth;			// store the height
	test->iDepth  = ImageGetBitDepthFromFormat ( format );
	test->bLocked = false;			// set locked to false

	// get actual dimensions of texture/image
	GGSURFACE_DESC imageddsd;
	LPGGSURFACE pTextureInterface = NULL;
	test->lpTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	pTextureInterface->GetDesc(&imageddsd);
	SAFE_RELEASE ( pTextureInterface );
	test->fTexUMax=(float)test->iWidth/(float)imageddsd.Width;
	test->fTexVMax=(float)test->iHeight/(float)imageddsd.Height;

	// for DX11, create resource view from texture and store in lpTextureRef
	test->lpTextureView = NULL;
	CreateShaderResourceViewFor ( test, 0, format );

	#else
	D3DPOOL dwPool = D3DPOOL_MANAGED;
	if ( dwUsage!=0 ) dwPool=D3DPOOL_DEFAULT;

	// create a new texture
	HRESULT		hr;					// used for error checking
	hr = D3DXCreateTexture ( 
							  m_pD3D,
							  iWidth,
							  iHeight,
							  1,
							  dwUsage,
							  format,
							  dwPool,
							  &test->lpTexture
					       );

	if ( FAILED ( hr ) )
	{
		Error ( "Failed to create new image" );
		return NULL;
	}

	// setup properties
	test->iHeight = iHeight;		// store the width
	test->iWidth  = iWidth;		// store the height
	test->iDepth  = ImageGetBitDepthFromFormat ( format );
	test->bLocked = false;			// set locked to false

	// get actual dimensions of texture/image
	D3DSURFACE_DESC imageddsd;
	test->lpTexture->GetLevelDesc(0, &imageddsd);
	test->fTexUMax=(float)test->iWidth/(float)imageddsd.Width;
	test->fTexVMax=(float)test->iHeight/(float)imageddsd.Height;
	#endif

	// Ensure smalltextres are handled
	if(test->fTexUMax>1.0f) test->fTexUMax=1.0f;
	if(test->fTexVMax>1.0f) test->fTexVMax=1.0f;

	// add to the list
    m_List.insert( std::make_pair(iID, test) );

	// ensure sprites all updated
	UpdateAllSprites();

	return test->lpTexture;
}

DARKSDK LPGGTEXTURE MakeImage ( int iID, int iWidth, int iHeight )
{
	return MakeFormat ( iID, iWidth, iHeight, g_DefaultGGFORMAT, 0 );
}

DARKSDK LPGGTEXTURE MakeImageUsage ( int iID, int iWidth, int iHeight, DWORD dwUsage )
{
	return MakeFormat ( iID, iWidth, iHeight, g_DefaultGGFORMAT, dwUsage );
}

DARKSDK LPGGTEXTURE MakeImageJustFormat ( int iID, int iWidth, int iHeight, GGFORMAT Format )
{
	return MakeFormat ( iID, iWidth, iHeight, Format, 0 );
}

DARKSDK LPGGTEXTURE MakeImageRenderTarget ( int iID, int iWidth, int iHeight, GGFORMAT Format )
{
	return MakeFormat ( iID, iWidth, iHeight, Format, GGUSAGE_RENDERTARGET );
}

DARKSDK void SetCubeFace ( int iID, LPGGCUBETEXTURE pCubeMap, int iFace )
{
	// get ptr to image
	if ( !UpdatePtrImage ( iID ) )
		return;

	// set cube reference details
	m_imgptr->pCubeMapRef = pCubeMap;
	m_imgptr->iCubeMapFace = iFace;
}

DARKSDK void GetCubeFace ( int iID, LPGGCUBETEXTURE* ppCubeMap, int* piFace )
{
	// get ptr to image
	if ( !UpdatePtrImage ( iID ) )
		return;

	// return cube reference details
	if ( ppCubeMap ) *ppCubeMap = m_imgptr->pCubeMapRef;
	if ( piFace ) *piFace = m_imgptr->iCubeMapFace;
}

DARKSDK void SetMipmapMode ( bool bMode )
{
	// switch mip mapping on or off
	#ifdef DX11
	#else
	if ( !bMode )
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
	}
	else
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, GGTEXF_LINEAR );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, GGTEXF_LINEAR );
	}
	#endif
}

DARKSDK void SetMipmapType ( int iType )
{
	// set the type of mip mapping used
	// iType possible values
	// 0 - none
	// 1 - point
	// 2 - linear ( default )
	if ( !m_pD3D )
		return;

	#ifdef DX11
	#else
	switch ( iType )
	{
		case 0:
		{
			// use no mip mapping
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );//GGTEXF_NONE ); // may not exist on HW driver any more!
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );//GGTEXF_NONE );
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );//GGTEXF_NONE );
		}
		break;

		case 1:
		{
			// set up point mip mapping
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
		}
		break;

		case 2:
		{
			// set up linear mip mapping
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER, GGTEXF_LINEAR );
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER, GGTEXF_LINEAR );
			m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER, GGTEXF_LINEAR );	
		}
		break;
	}
	#endif
}
	
DARKSDK void SetMipmapBias ( float fBias )
{
	// set the bias for the mip mapping, this allows you
	// to set the distance at which point the mip mapping
	// is brought into action
	#ifdef DX11
	#else
	m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPMAPLODBIAS, *( ( LPDWORD ) ( &fBias ) ) );
	#endif
}

DARKSDK void SetMipmapNum ( int iNum )
{
	// set the number of mip maps used, remember that if
	// you increase this value it will take up a lot more
	// video memory - turn this down if you don't need it
	m_iMipMapNum = iNum;
}

DARKSDK void SetImageTranslucency ( int iID, int iPercent )
{
	// set the translucency for an image
	if ( !UpdatePtrImage ( iID ) )
		return;

	// now lock the surface
	#ifdef DX11
	#else
	HRESULT hr;
	if ( FAILED ( hr = m_imgptr->lpTexture->LockRect ( 0, &m_imgptr->d3dlr, 0, 0 ) ) )
		Error ( "Failed to lock texture in translucency for image library" );
	{
		// get a pointer to surface data
		DWORD* pPix = ( DWORD* ) m_imgptr->d3dlr.pBits;
		DWORD  pGet;

		for ( DWORD y = 0; y < (DWORD)m_imgptr->iHeight; y++ )
		{
			for ( DWORD x = 0; x < (DWORD)m_imgptr->iWidth; x++ )
			{
				pGet = *pPix;

				DWORD dwAlpha = pGet >> 24;
				DWORD dwRed   = ((( pGet ) >> 16 ) & 0xff );
				DWORD dwGreen = ((( pGet ) >>  8 ) & 0xff );
				DWORD dwBlue  = ((( pGet ) )       & 0xff );
				
				if ( *pPix != 0 )
					*pPix++ = GGCOLOR_ARGB ( iPercent, dwRed, dwGreen, dwBlue );
				else
					*pPix++;
			}
		}
	}
	m_imgptr->lpTexture->UnlockRect ( NULL );
	#endif
}

DARKSDK bool ImageExist ( int iID )
{
	// returns true if the image exists
	if ( !UpdatePtrImage ( iID ) )
		return false;

	// return true
	return true;
}

// This load is the MAIN IMAGE LOADER
DARKSDK bool LoadImageCoreRetainName ( char* szRealName, char* szFilename, int iID, int iTextureFlag, bool bIgnoreNegLimit, int iDivideTextureSize )
{
	// iTextureFlag (0-default/1-sectionofplate/2-cube)
	// iDivideTextureSize (0-leave size,0>divide by)
	if ( bIgnoreNegLimit==false )
	{
		if( iID<1 || iID>MAXIMUMVALUE )
		{
			RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER, szFilename);
			return false;
		}
	}

    RemoveImage( iID );

	// loads in image into the bank
	tagImgData*	test = NULL;		// pointer to data structure
	int			iLen1;				// used for checking length of strings
	int			iLen2;				// used for checking length of strings
	char		szTest [ 256 ];		// text buffer
	bool		bFound = false;		// flag which checks if we need to load the texture

	// create a new block of memory
	test = new tagImgData;

	// check the memory was created
	if ( test == NULL )
		return false;

	// clear out the memory
	memset(test, 0, sizeof(tagImgData));

	// clear out text buffers
	memset ( test->szLongFilename,  0, sizeof ( char ) * 256 );		// clear out long filename
	memset ( test->szShortFilename, 0, sizeof ( char ) * 256 );		// clear out short filename
	memset ( szTest,                0, sizeof ( char ) * 256 );		// clear out test buffer

	// sort which name to use
	LPSTR pNameForInternalList = szFilename;
	if ( strlen(szRealName)>1 ) pNameForInternalList = szRealName;

	// get length of filename and copy to shortfilename
	iLen1 = lstrlen ( pNameForInternalList );											// get length
	memcpy ( test->szShortFilename, pNameForInternalList, sizeof ( char ) * iLen1 );	// copy to short filename

	// sort out full filename
	GetCurrentDirectory ( 256, szTest );	// get the apps full directory e.g. "c:\test\"
	iLen1 = lstrlen ( szTest );				// get the length of the full directory
	iLen2 = lstrlen ( pNameForInternalList );			// get the length of the passed filename
	
	// copy memory
	memcpy ( test->szLongFilename,         szTest,         sizeof ( char ) * iLen1 );	// copy the full dir to the long filename

	// U74 BETA9 - 030709 - fix longfilename if full dir has no \ and filename does not start with
	// memcpy ( test->szLongFilename + iLen1, szFilename + 1, sizeof ( char ) * iLen2 );	// append the filename onto the long file name
	if ( pNameForInternalList[0]!='\\' )
	{
		if ( szTest[iLen1]!='\\' )
		{
			test->szLongFilename[iLen1]='\\';
			memcpy ( test->szLongFilename + iLen1 + 1, pNameForInternalList, sizeof ( char ) * iLen2 );
		}
		else
			memcpy ( test->szLongFilename + iLen1, pNameForInternalList, sizeof ( char ) * iLen2 );
	}
	else
	{
		// legacy support
		memcpy ( test->szLongFilename + iLen1, pNameForInternalList + 1, sizeof ( char ) * iLen2 );
	}

	// The default is a setting of zero (0)
	GGIMAGE_INFO info;
	if( iTextureFlag==1 )
	{
		// loads image into a section of the texture plate
		if ( m_bSharing && !bFound )
			test->lpTexture = GetTexture ( szFilename, &info, 1 );	// load the perfect texture
		else
			test->lpTexture = GetTexture ( szFilename, &info, 1 );	// load the perfect texture

		// load failed
		if ( test->lpTexture==NULL )
		{
			SAFE_DELETE(test);
			return false;
		}

		// Set image settings
		test->iHeight = m_iHeight;					// store the width
		test->iWidth  = m_iWidth;					// store the height

		// get actual dimensions of texture/image
		#ifdef DX11
		GGSURFACE_DESC imageddsd;
		LPGGSURFACE pTextureInterface = NULL;
		test->lpTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
		pTextureInterface->GetDesc(&imageddsd);
		SAFE_RELEASE ( pTextureInterface );
		test->fTexUMax=(float)test->iWidth/(float)imageddsd.Width;
		test->fTexVMax=(float)test->iHeight/(float)imageddsd.Height;
		#else
		D3DSURFACE_DESC imageddsd;
		test->lpTexture->GetLevelDesc(0, &imageddsd);
		test->fTexUMax=(float)test->iWidth/(float)imageddsd.Width;
		test->fTexVMax=(float)test->iHeight/(float)imageddsd.Height;
		#endif

		// Ensure smalltextres are handled
		if(test->fTexUMax>1.0f) test->fTexUMax=1.0f;
		if(test->fTexVMax>1.0f) test->fTexVMax=1.0f;
	}
	else
	{
		// Load Image Into Whole Texture Plate
		LoadImageCoreFullTex ( szFilename, &test->lpTexture, &info, iTextureFlag, iDivideTextureSize );	// loads into whole texture

		// load failed
		if ( test->lpTexture==NULL )
		{
			SAFE_DELETE(test);
			return false;
		}

		// get file image info
		test->iHeight = info.Height;
		test->iWidth  = info.Width;

		// Entire texture used
		test->fTexUMax=1.0f;
		test->fTexVMax=1.0f;
	}

	// Get depth of texture
	#ifdef DX11

	// for DX11, create resource view from texture and store in lpTextureRef
	CreateShaderResourceViewFor ( test, iTextureFlag, info.Format );

	// Get depth from format
	test->iDepth = ImageGetBitDepthFromFormat(info.Format);

	#else
	D3DSURFACE_DESC desc;
	test->lpTexture->GetLevelDesc(0, &desc);
	test->iDepth  = ImageGetBitDepthFromFormat(desc.Format);
	#endif

	// load failed
	if ( test->lpTexture==NULL )
	{
		SAFE_DELETE(test);
		return false;
	}

	// fill out rest of structure
	test->bLocked = false;

	// add to the list
    m_List.insert( std::make_pair(iID, test) );

	// ensure sprites all updated
	// V109 BETA3 - only need to update image/textures, not internal textures (negatives)
	if ( iID>0 ) UpdateAllSprites();

	return true;
}

DARKSDK bool LoadImageCore ( char* szFilename, int iID, int iTextureFlag, bool bIgnoreNegLimit, int iDivideTextureSize )
{
	return LoadImageCoreRetainName ( "", szFilename, iID, iTextureFlag, bIgnoreNegLimit, iDivideTextureSize );
}

DARKSDK bool LoadImageCore ( char* szFilename, int iID )
{
	return LoadImageCore ( szFilename, iID, 1, false, 0 );
}

DARKSDK bool SaveImageCore ( char* szFilename, int iID, int iCompressionMode )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return false;
	}
	if ( !UpdatePtrImage ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_IMAGENOTEXIST);
		return false;
	}
	if ( szFilename==NULL )
	{
		RunTimeError(RUNTIMEERROR_FILENOTEXIST);
		return false;
	}

	// determine format from extension
	#ifdef DX11
	D3DX11_IMAGE_FILE_FORMAT DestFormat = D3DX11_IFF_BMP;
	LPSTR szFilenameExt = szFilename + strlen(szFilename)-4;
	if ( _strnicmp ( szFilenameExt, ".bmp", 4 )==NULL ) DestFormat = D3DX11_IFF_BMP;
	if ( _strnicmp ( szFilenameExt, ".dds", 4 )==NULL ) DestFormat = D3DX11_IFF_DDS;
	if ( _strnicmp ( szFilenameExt, ".jpg", 4 )==NULL ) DestFormat = D3DX11_IFF_JPG;
	if ( _strnicmp ( szFilenameExt, ".png", 4 )==NULL ) DestFormat = D3DX11_IFF_PNG;

	// determine region of surface to save
	GGSURFACE_DESC srcddsd;
	LPGGSURFACE pTexSurface = NULL;
	m_imgptr->lpTexture->QueryInterface<ID3D11Texture2D>(&pTexSurface);
	if ( pTexSurface )
	{
		// determine size of surface
		HRESULT hRes;
		pTexSurface->GetDesc(&srcddsd);

		// use automatic compression
		GGFORMAT dwD3DSurfaceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		switch ( iCompressionMode )
		{
			case 1 : dwD3DSurfaceFormat = GGFMT_DXT1; break;
			case 2 : dwD3DSurfaceFormat = GGFMT_DXT2; break;
			case 3 : dwD3DSurfaceFormat = GGFMT_DXT3; break;
			case 4 : dwD3DSurfaceFormat = GGFMT_DXT4; break;
			case 5 : dwD3DSurfaceFormat = GGFMT_DXT5; break;
		}

		LPGGSURFACE pSourceDDS = NULL;
		//GGSURFACE_DESC StagedDesc = { srcddsd.Width, srcddsd.Height, 1, 1, srcddsd.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0 };
		GGSURFACE_DESC StagedDesc = { srcddsd.Width, srcddsd.Height, srcddsd.MipLevels, srcddsd.ArraySize, srcddsd.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0 };
		HRESULT hr = m_pD3D->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&pSourceDDS );
		if ( pSourceDDS )
		{
			// first copy texture to a stage ready for reading (BGRA)
			m_pImmediateContext->CopyResource ( pSourceDDS, pTexSurface );

			// Copy Resource cannot convert pixel formats, so do it manually to get (RGBA)
			if ( srcddsd.Format == DXGI_FORMAT_B8G8R8A8_UNORM && dwD3DSurfaceFormat == DXGI_FORMAT_R8G8B8A8_UNORM )
			{
				LPGGSURFACE pDestDDS = NULL;
				GGSURFACE_DESC DestDesc = { srcddsd.Width, srcddsd.Height, 1, 1, dwD3DSurfaceFormat, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0 };
				HRESULT hr = m_pD3D->CreateTexture2D( &DestDesc, NULL, (ID3D11Texture2D**)&pDestDDS );
				if ( pDestDDS )
				{
					D3D11_MAPPED_SUBRESOURCE srcMapped;
					HRESULT hr = m_pImmediateContext->Map( pSourceDDS, 0, D3D11_MAP_READ, 0, &srcMapped );
					if ( SUCCEEDED( hr ) )
					{
						D3D11_MAPPED_SUBRESOURCE destMapped;
						HRESULT hr = m_pImmediateContext->Map( pDestDDS, 0, D3D11_MAP_READ, 0, &destMapped );
						if ( SUCCEEDED( hr ) )
						{
							const size_t size = srcddsd.Width*srcddsd.Height*4;
							unsigned char* pSrc = static_cast<unsigned char*>( srcMapped.pData );
							unsigned char* pDest = static_cast<unsigned char*>( destMapped.pData );
							int offsetSrc = 0;
							int offsetDst = 0;
							int rowOffset = srcMapped.RowPitch % srcddsd.Width;
							for (int row = 0; row < srcddsd.Height; ++row)
							{
								for (int col = 0; col < srcddsd.Width; ++col)
								{
									pDest[offsetDst] = pSrc[offsetSrc+2];
									pDest[offsetDst+1] = pSrc[offsetSrc+1];
									pDest[offsetDst+2] = pSrc[offsetSrc];
									pDest[offsetDst+3] = pSrc[offsetSrc+3];
									offsetSrc += 4;
									offsetDst += 4;
								}
								offsetSrc += rowOffset;
							}
							m_pImmediateContext->Unmap( pDestDDS, 0 );
						}
						m_pImmediateContext->Unmap( pSourceDDS, 0 );
					}
				}
				SAFE_RELEASE ( pSourceDDS );
				SAFE_RELEASE ( pTexSurface );
				pTexSurface = pDestDDS;
			}
			else
			{
				SAFE_RELEASE ( pTexSurface );
				pTexSurface = pSourceDDS;
			}
		}

		// save surface of image to file
		hRes = D3DX11SaveTextureToFile( m_pImmediateContext, pTexSurface, DestFormat, szFilename );
		if ( FAILED ( hRes ) )
		{
			char pStrClue[512];
			wsprintf ( pStrClue, "tex:%d filename:%s", (int)pTexSurface, szFilename );
			RunTimeError(RUNTIMEERROR_IMAGEERROR,pStrClue);
			SAFE_RELEASE(pTexSurface);
			return false;
		}
	}

	// free surface
	SAFE_RELEASE(pTexSurface);
	#else
	D3DXIMAGE_FILEFORMAT DestFormat = D3DXIFF_BMP;
	LPSTR szFilenameExt = szFilename + strlen(szFilename)-4;
	if ( _strnicmp ( szFilenameExt, ".bmp", 4 )==NULL ) DestFormat=D3DXIFF_BMP ;
	if ( _strnicmp ( szFilenameExt, ".dds", 4 )==NULL ) DestFormat=D3DXIFF_DDS ;
	if ( _strnicmp ( szFilenameExt, ".dib", 4 )==NULL ) DestFormat=D3DXIFF_DIB ;
	if ( _strnicmp ( szFilenameExt, ".jpg", 4 )==NULL ) DestFormat=D3DXIFF_JPG ;
	if ( _strnicmp ( szFilenameExt, ".png", 4 )==NULL ) DestFormat=D3DXIFF_PNG ;

	// determine region of surface to save
	LPGGSURFACE pTexSurface = NULL;
	m_imgptr->lpTexture->GetSurfaceLevel(0, &pTexSurface);
	if ( pTexSurface )
	{
		// determine size of surface
		D3DSURFACE_DESC srcddsd;
		HRESULT hRes = pTexSurface->GetDesc(&srcddsd);
		RECT rc = { 0, 0, (int)((float)srcddsd.Width*m_imgptr->fTexUMax), (int)((float)srcddsd.Height*m_imgptr->fTexVMax) };

		// If DDS, use automatic compression
		LPGGTEXTURE pTextureDDS = NULL;
		LPGGSURFACE pDDSSurface = NULL;
		if ( DestFormat==D3DXIFF_DDS )
		{
			// and only if source NOT already compressed
			if ( srcddsd.Format==GGFMT_DXT1 || srcddsd.Format==GGFMT_DXT2 || srcddsd.Format==GGFMT_DXT3 || srcddsd.Format==GGFMT_DXT4 || srcddsd.Format==GGFMT_DXT5 )
			{
				// source already compressed
			}
			else
			{
				// create our DDS MASTER texture (the final one to be used)
				GGFORMAT dwD3DSurfaceFormat = GGFMT_A8R8G8B8;
				switch ( iCompressionMode )
				{
					case 1 : dwD3DSurfaceFormat = GGFMT_DXT1; break;
					case 2 : dwD3DSurfaceFormat = GGFMT_DXT2; break;
					case 3 : dwD3DSurfaceFormat = GGFMT_DXT3; break;
					case 4 : dwD3DSurfaceFormat = GGFMT_DXT4; break;
					case 5 : dwD3DSurfaceFormat = GGFMT_DXT5; break;
				}
				m_pD3D->CreateTexture ( srcddsd.Width, srcddsd.Height, 1, 0, dwD3DSurfaceFormat, D3DPOOL_MANAGED, &pTextureDDS, NULL );
				if ( pTextureDDS )
				{
					// copy texture to DDS compressed texture
					pTextureDDS->GetSurfaceLevel ( 0, &pDDSSurface );
					if ( pDDSSurface )
					{
						D3DSURFACE_DESC dstddsd;
						HRESULT hRes = pDDSSurface->GetDesc(&dstddsd);
						if ( pTexSurface )
						{
							hRes = D3DXLoadSurfaceFromSurface ( pDDSSurface, NULL, NULL, pTexSurface, NULL, NULL, D3DX_FILTER_NONE, 0 );
							if ( FAILED ( hRes ) )
							{
								char pStrClue[512];
								wsprintf ( pStrClue, "D3DXLoadSurfaceFromSurface failure" );
								RunTimeError(RUNTIMEERROR_IMAGEERROR,pStrClue);
								SAFE_RELEASE(pDDSSurface);
								SAFE_RELEASE(pTextureDDS);
								SAFE_RELEASE(pTexSurface);
								return false;
							}
						}
					}

					// Copy over DDS to regular surface
					SAFE_RELEASE ( pTexSurface );
					pTexSurface = pDDSSurface;
					pDDSSurface = NULL;
				}
			}
		}

		// save surface of image to file
		hRes = D3DXSaveSurfaceToFile( szFilename, DestFormat, pTexSurface, NULL, &rc );
		if ( FAILED ( hRes ) )
		{
			char pStrClue[512];
			wsprintf ( pStrClue, "tex:%d filename:%s region:%d %d %d %d", (int)pTexSurface, szFilename, rc.left, rc.top, rc.right, rc.bottom );
			RunTimeError(RUNTIMEERROR_IMAGEERROR,pStrClue);
			SAFE_RELEASE(pTexSurface);
			return false;
		}

		SAFE_RELEASE(pDDSSurface);
		SAFE_RELEASE(pTextureDDS);
	}

	// free surface
	SAFE_RELEASE(pTexSurface);
	#endif

	// success
	return true;
}

DARKSDK bool SaveImageCore ( char* szFilename, int iID )
{
	// default behaviour
	return SaveImageCore ( szFilename, iID, 0 );
}

DARKSDK void CreateReplaceImage ( int iID, int iTexSize, ID3D11Texture2D* pTex, ID3D11ShaderResourceView* pView )
{
	// establish if image exists or not
	if( iID < 1 || iID > MAXIMUMVALUE )
		return;

	if ( !UpdatePtrImage ( iID ) )
	{
		// create a new image
		MakeFormat ( iID, iTexSize, iTexSize, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );
		UpdatePtrImage ( iID );
	}

	// valid image can be be overridden with new image details
	m_imgptr->lpTexture = pTex;
	m_imgptr->lpTextureView = pView;
	m_imgptr->iWidth = iTexSize;
	m_imgptr->iHeight = iTexSize;
	SAFE_DELETE ( m_imgptr->lpName );
	strcpy ( m_imgptr->szLongFilename, "" );
	strcpy ( m_imgptr->szShortFilename, "" );
	m_imgptr->fTexUMax = 1.0f;
	m_imgptr->fTexVMax = 1.0f;
}

DARKSDK bool GrabImageCore ( int iID, int iX1, int iY1, int iX2, int iY2, int iTextureFlagForGrab )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return false;
	}
	if(iX1>=iX2 || iY1>=iY2)
	{
		RunTimeError(RUNTIMEERROR_IMAGEAREAILLEGAL);
		return false;
	}

	#ifdef DX11
	// Size of grab
	GGFORMAT backFormat;
	GGSURFACE_DESC ddsd;
	LPGGSURFACE pBackBuffer = g_pGlob->pCurrentBitmapSurface;
	if(pBackBuffer)
	{
		// get format of backbuffer
		pBackBuffer->GetDesc(&ddsd);
		backFormat = ddsd.Format;
		if(iX2>(int)ddsd.Width || iY2>(int)ddsd.Height)
		{
			RunTimeError(RUNTIMEERROR_IMAGEAREAILLEGAL);
			return false;
		}
	}

	// Image size
	int iImageWidth=iX2-iX1;
	int iImageHeight=iY2-iY1;

	// Get current render target surface
	if(pBackBuffer)
	{
		// check if image already exists of same size and type
		if ( UpdatePtrImage ( iID ) )
		{
			// check against existing
			if(m_imgptr->iWidth != iImageWidth || m_imgptr->iHeight != iImageHeight )
			{
				// existing image and new image are different sizes - so delete any existing image
                RemoveImage( iID );
			}
		}

		// if image format not internal texture format, delete so can be recreated
		if ( m_imgptr )
		{
			GGSURFACE_DESC imgddsd;
			LPGGSURFACE pTextureInterface = NULL;
			m_imgptr->lpTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
			pTextureInterface->GetDesc(&imgddsd);
			SAFE_RELEASE ( pTextureInterface );
			if ( imgddsd.Format != g_DefaultGGFORMAT )
			{
				RemoveImage( iID );
			}
		}

		// create temp image texture to copy backbuffer to
		LPGGSURFACE pTempTexture = NULL;
		GGSURFACE_DESC TempTextureDesc = { iImageWidth, iImageHeight, 1, 1, backFormat, 1, 0, D3D11_USAGE_DEFAULT, 0, 0, 0 };
		m_pD3D->CreateTexture2D( &TempTextureDesc, NULL, &pTempTexture );
		if ( pTempTexture )
		{
			// copy backbuffer to temp texture
			//D3D11_BOX rc = { 0, 0, 0, (LONG)(iImageWidth), (LONG)(iImageHeight), 1 }; 
			D3D11_BOX rc = { iX1, iY1, 0, (LONG)(iX1+iImageWidth), (LONG)(iY1+iImageHeight), 1 }; 
			m_pImmediateContext->CopySubresourceRegion ( pTempTexture, 0, 0, 0, 0, pBackBuffer, 0, &rc );

			// create image
			if ( m_imgptr == NULL ) MakeFormat ( iID, iImageWidth, iImageHeight, g_DefaultGGFORMAT, 0 );
			if ( UpdatePtrImage ( iID ) )
			{
				// get desc of destination texture
				GGSURFACE_DESC srcddsd;
				LPGGSURFACE pTextureInterface = NULL;
				m_imgptr->lpTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
				pTextureInterface->GetDesc(&srcddsd);
				SAFE_RELEASE ( pTextureInterface );

				/* no need for this, can simply delete image and recreate as correct format
				// first convert texture to BGRA if RGBA (from a load) - for internal resource copying
				if ( srcddsd.Format == DXGI_FORMAT_R8G8B8A8_UNORM )
				{
					LPGGSURFACE pDestDDS = NULL;
					LPGGSURFACE pSourceDDS = pTempTexture;
					GGSURFACE_DESC DestDesc = { srcddsd.Width, srcddsd.Height, 1, 1, g_DefaultGGFORMAT, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 0 };
					HRESULT hr = m_pD3D->CreateTexture2D( &DestDesc, NULL, (ID3D11Texture2D**)&pDestDDS );
					if ( pDestDDS )
					{
						D3D11_MAPPED_SUBRESOURCE srcMapped;
						HRESULT hr = m_pImmediateContext->Map( pSourceDDS, 0, D3D11_MAP_READ, 0, &srcMapped );
						if ( SUCCEEDED( hr ) )
						{
							D3D11_MAPPED_SUBRESOURCE destMapped;
							HRESULT hr = m_pImmediateContext->Map( pDestDDS, 0, D3D11_MAP_READ, 0, &destMapped );
							if ( SUCCEEDED( hr ) )
							{
								const size_t size = srcddsd.Width*srcddsd.Height*4;
								unsigned char* pSrc = static_cast<unsigned char*>( srcMapped.pData );
								unsigned char* pDest = static_cast<unsigned char*>( destMapped.pData );
								int offsetSrc = 0;
								int offsetDst = 0;
								int rowOffset = srcMapped.RowPitch % srcddsd.Width;
								for (int row = 0; row < srcddsd.Height; ++row)
								{
									for (int col = 0; col < srcddsd.Width; ++col)
									{
										pDest[offsetDst] = pSrc[offsetSrc+2];
										pDest[offsetDst+1] = pSrc[offsetSrc+1];
										pDest[offsetDst+2] = pSrc[offsetSrc];
										pDest[offsetDst+3] = pSrc[offsetSrc+3];
										offsetSrc += 4;
										offsetDst += 4;
									}
									offsetSrc += rowOffset;
								}
								m_pImmediateContext->Unmap( pDestDDS, 0 );
							}
							m_pImmediateContext->Unmap( pSourceDDS, 0 );
						}

						// create new image in converted format
						m_imgptr->lpTexture

						// release temp staged dest
						SAFE_RELEASE ( pDestDDS );
					}
				}
				*/

				// load grabbed surface data into destination texture
				D3D11_BOX rc = { 0, 0, 0, (LONG)(iImageWidth), (LONG)(iImageHeight), 1 }; 
				m_pImmediateContext->CopySubresourceRegion ( m_imgptr->lpTexture, 0, 0, 0, 0, pTempTexture, 0, &rc );

				// get actual dimensions of texture/image
				m_imgptr->fTexUMax=(float)m_imgptr->iWidth/(float)srcddsd.Width;
				m_imgptr->fTexVMax=(float)m_imgptr->iHeight/(float)srcddsd.Height;

				// Ensure smalltextres are handled
				if(m_imgptr->fTexUMax>1.0f) m_imgptr->fTexUMax=1.0f;
				if(m_imgptr->fTexVMax>1.0f) m_imgptr->fTexVMax=1.0f;
			}
			else
			{
				RunTimeError(RUNTIMEERROR_IMAGEERROR);
				return false;
			}

			// free work-newsurface
			SAFE_RELEASE(pTempTexture);
		}
	}
	else
	{
		RunTimeError(RUNTIMEERROR_IMAGEERROR);
		return false;
	}

	// An additional feature is that images by default are stretched to complete fit a texture (no stretching in DX11=performance)
	///if ( iTextureFlagForGrab==0 || iTextureFlagForGrab==2 )
	///{
	///	StretchImage ( iID, GetPowerSquareOfSize(iImageWidth), GetPowerSquareOfSize(iImageHeight) );
	///}
	#else
	// Size of grab
	LPGGSURFACE pBackBuffer = g_pGlob->pCurrentBitmapSurface;
	if(pBackBuffer)
	{
		// get format of backbuffer
		D3DSURFACE_DESC ddsd;
		HRESULT hRes = pBackBuffer->GetDesc(&ddsd);
		GGFORMAT GGFORMAT = ddsd.Format;
		if(iX2>(int)ddsd.Width || iY2>(int)ddsd.Height)
		{
			RunTimeError(RUNTIMEERROR_IMAGEAREAILLEGAL);
			return false;
		}
	}

	// Image size
	int iImageWidth=iX2-iX1;
	int iImageHeight=iY2-iY1;

	// Get current render target surface
	if(pBackBuffer)
	{
		// get format of backbuffer
		D3DSURFACE_DESC ddsd;
		HRESULT hRes = pBackBuffer->GetDesc(&ddsd);
		GGFORMAT GGFORMAT = ddsd.Format;

		// check if image already exists of same size and type
		if ( UpdatePtrImage ( iID ) )
		{
			// check against existing
			if(m_imgptr->iWidth != iImageWidth || m_imgptr->iHeight != iImageHeight)
			{
				// existing image and new image are different sizes - so delete any existing image
                RemoveImage( iID );
			}
		}

		// create temp image texture to copy backbuffer to (same format at first)
		LPGGTEXTURE pTempTexture=NULL;
		hRes = D3DXCreateTexture (m_pD3D,
								  iImageWidth,
								  iImageHeight,
								  D3DX_DEFAULT,
								  0,
								  GGFORMAT,
								  D3DPOOL_MANAGED,
								  &pTempTexture	       );

		if ( pTempTexture )
		{
			// lock surface
			GGLOCKED_RECT d3dlr;
			hRes = pTempTexture->LockRect ( 0, &d3dlr, 0, 0 );
			if ( SUCCEEDED(hRes) )
			{
				// get size of single pixel (16bit, 24bit or 32bit)
				DWORD dwPixelSize = ImageGetBitDepthFromFormat(GGFORMAT)/8;

				// copy from backbuffer lock to texture lock
				RECT rc = { iX1, iY1, iX2, iY2 };
				GGLOCKED_RECT backlock;
				pBackBuffer->LockRect(&backlock, &rc, D3DLOCK_READONLY);
				LPSTR pPtr = (LPSTR)backlock.pBits;
				if ( pPtr==NULL )
				{
					MessageBox ( NULL, "Tried to read a surface which has no read permissions!", "System Memory Bitmap Error", MB_OK );
					RunTimeError(RUNTIMEERROR_IMAGEERROR);
					SAFE_RELEASE(pTempTexture);
					return false;
				}

				// straight copy or stretch copy
				bool bStretchCopy=false;
				D3DSURFACE_DESC imageddsd;
				pTempTexture->GetLevelDesc(0, &imageddsd);
				if(imageddsd.Width<(DWORD)iImageWidth) bStretchCopy=true;
				if(imageddsd.Height<(DWORD)iImageHeight) bStretchCopy=true;
				if(bStretchCopy==true)
				{
					DWORD dwClipWidth = iImageWidth;
					DWORD dwClipHeight = iImageHeight;
					if(imageddsd.Width<dwClipWidth) dwClipWidth=imageddsd.Width;
					if(imageddsd.Height<dwClipHeight) dwClipHeight=imageddsd.Height;
					float fXBit = (float)iImageWidth/(float)dwClipWidth;
					float fYBit = (float)iImageHeight/(float)dwClipHeight;
					LPSTR pImagePtr = (LPSTR)d3dlr.pBits;
					float fY=0.0f;
					for(DWORD y=0; y<dwClipHeight; y++)
					{
						LPSTR pImgPtr = pImagePtr + (y*d3dlr.Pitch);
						LPSTR pPtr = (LPSTR)backlock.pBits + ((int)fY*backlock.Pitch);
						float fX=0.0f;
						for(DWORD x=0; x<dwClipWidth; x++)
						{
							switch(dwPixelSize)
							{
								case 2 : *(WORD*)(pImgPtr) = *(WORD*)(pPtr+((int)fX*dwPixelSize));	break;
								case 4 : *(DWORD*)(pImgPtr) = *(DWORD*)(pPtr+((int)fX*dwPixelSize));	break;
							}
							pImgPtr+=dwPixelSize;
							fX+=fXBit;
						}
						fY+=fYBit;
					}
				}
				else
				{
					LPSTR pImagePtr = (LPSTR)d3dlr.pBits;
					for(int y=0; y<iImageHeight; y++)
					{
						memcpy(pImagePtr, pPtr, iImageWidth*dwPixelSize);
						pImagePtr+=d3dlr.Pitch;
						pPtr+=backlock.Pitch;
					}
				}
				pBackBuffer->UnlockRect();

				// unlock texture
				pTempTexture->UnlockRect(NULL);
			}

			// create image
			if(m_imgptr==NULL) MakeFormat ( iID, iImageWidth, iImageHeight, g_DefaultGGFORMAT, 0 );
			if ( UpdatePtrImage ( iID ) )
			{
				// load grabbed surface data into destination texture
				LPGGSURFACE pNewSurface = NULL;
				pTempTexture->GetSurfaceLevel(0, &pNewSurface);
				LPGGSURFACE pTexSurface = NULL;
				m_imgptr->lpTexture->GetSurfaceLevel(0, &pTexSurface);

				// LEEFIX - 071002 - No srtetching or filtering if DBV1 mode used
				if ( iTextureFlagForGrab==0 )
				{
					// leefix - dx8->dx9 - if texture exact size of image, no scaling required
					D3DSURFACE_DESC ddsdNewTexture;
					HRESULT hRes = pTexSurface->GetDesc(&ddsdNewTexture);
					if ( iImageWidth==(int)ddsdNewTexture.Width && iImageHeight==(int)ddsdNewTexture.Height )
						hRes = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_FILTER_NONE, m_Color);
					else
						hRes = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_DEFAULT, m_Color);
				}
				else
				{
					if ( iTextureFlagForGrab==2 )
						hRes = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_FILTER_NONE, NULL );
					else
					{
						// leeadd - 201008 - u71 - should not have used color key (mode3)
						if ( iTextureFlagForGrab==3 )
							hRes = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_FILTER_NONE, NULL );
						else
							hRes = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_FILTER_NONE, m_Color);
					}
				}

				SAFE_RELEASE(pNewSurface);
				SAFE_RELEASE(pTexSurface);

				// get actual dimensions of texture/image
				D3DSURFACE_DESC imageddsd;
				m_imgptr->lpTexture->GetLevelDesc(0, &imageddsd);
				m_imgptr->fTexUMax=(float)m_imgptr->iWidth/(float)imageddsd.Width;
				m_imgptr->fTexVMax=(float)m_imgptr->iHeight/(float)imageddsd.Height;

				// Ensure smalltextres are handled
				if(m_imgptr->fTexUMax>1.0f) m_imgptr->fTexUMax=1.0f;
				if(m_imgptr->fTexVMax>1.0f) m_imgptr->fTexVMax=1.0f;
			}
			else
			{
				RunTimeError(RUNTIMEERROR_IMAGEERROR);
				SAFE_RELEASE(pTempTexture);
				return false;
			}

			// free work-newsurface
			SAFE_RELEASE(pTempTexture);
		}
	}
	else
	{
		RunTimeError(RUNTIMEERROR_IMAGEERROR);
		return false;
	}

	// An additional feature is that images by default are stretched to complete fit a texture
	if ( iTextureFlagForGrab==0 || iTextureFlagForGrab==2 )
	{
		StretchImage ( iID, GetPowerSquareOfSize(iImageWidth), GetPowerSquareOfSize(iImageHeight) );
	}
	#endif

	// Complete
	return true;
}

DARKSDK bool GrabImageCore ( int iID, int iX1, int iY1, int iX2, int iY2 )
{
	// Stretch image to fit texture by default (0)
	return GrabImageCore ( iID, iX1, iY1, iX2, iY2, 0 );
}

DARKSDK void TransferImage ( int iDestImgID, int iSrcImageID, int iTransferMode, int iMemblockAssistor )
{
	// iTransferMode:
	// 1 = blue channel specifies one of sixteen IDs, each representing a small 4x4 min-texture described in the indexed memblock passed in

	// validate
	if(iDestImgID<1 || iDestImgID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}
	if(iSrcImageID<1 || iSrcImageID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}

	#ifdef DX11
	#else
	// get destination details
	D3DSURFACE_DESC ddsd;
	LPGGSURFACE pDstSurface = NULL;
	if ( !UpdatePtrImage ( iDestImgID ) ) return;
	tagImgData* pDestImagePtr = m_imgptr;
	pDestImagePtr->lpTexture->GetSurfaceLevel(0, &pDstSurface);
	pDstSurface->GetDesc(&ddsd);
	DWORD dwDestImgWidth = ddsd.Width;
	DWORD dwDestImgHeight = ddsd.Height;

	// get source details
	LPGGSURFACE pSrcSurface = NULL;
	if ( !UpdatePtrImage ( iSrcImageID ) ) return;
	tagImgData* pSrcImagePtr = m_imgptr;
	pSrcImagePtr->lpTexture->GetSurfaceLevel(0, &pSrcSurface);
	pSrcSurface->GetDesc(&ddsd);
	DWORD dwSrcImgWidth = ddsd.Width;
	DWORD dwSrcImgHeight = ddsd.Height;

	// must be same size 
	if ( dwSrcImgWidth!=dwDestImgWidth || dwSrcImgHeight!=dwDestImgHeight ) return;

	// go through each pixel and apply transfer logic
	GGLOCKED_RECT d3dDstlock;
	RECT rc = { 0, 0, (LONG)dwSrcImgWidth, (LONG)dwSrcImgHeight };
	if(SUCCEEDED(pDstSurface->LockRect ( &d3dDstlock, &rc, 0 ) ) )
	{
		GGLOCKED_RECT d3dSrclock;
		if(SUCCEEDED(pSrcSurface->LockRect ( &d3dSrclock, &rc, 0 ) ) )
		{
			for(DWORD y=0; y<dwSrcImgHeight; y++)
			{
				LPSTR pDst = (LPSTR)d3dDstlock.pBits + (y*d3dDstlock.Pitch);
				LPSTR pSrc = (LPSTR)d3dSrclock.pBits + (y*d3dSrclock.Pitch);
				for(DWORD x=0; x<dwSrcImgWidth; x++)
				{
					if ( iTransferMode==1 )
					{
						// first get the pixel to work on
						DWORD dwPixelValue = *(DWORD*)pSrc;
						float fTexSelectorV = ((dwPixelValue & 0x00000FF))/255.0f;
						float fTexSelectorCol[17];
						fTexSelectorCol[1] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*0)))*16.0f;
						if ( fTexSelectorCol[1] < 0 ) fTexSelectorCol[1] = 0;
						fTexSelectorCol[2] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*1)))*16.0f;
						if ( fTexSelectorCol[2] < 0 ) fTexSelectorCol[2] = 0;
						fTexSelectorCol[3] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*2)))*16.0f;
						if ( fTexSelectorCol[3] < 0 ) fTexSelectorCol[3] = 0;
						fTexSelectorCol[4] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*3)))*16.0f;
						if ( fTexSelectorCol[4] < 0 ) fTexSelectorCol[4] = 0;
						fTexSelectorCol[5] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*4)))*16.0f;
						if ( fTexSelectorCol[5] < 0 ) fTexSelectorCol[5] = 0;
						fTexSelectorCol[6] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*5)))*16.0f;
						if ( fTexSelectorCol[6] < 0 ) fTexSelectorCol[6] = 0;
						fTexSelectorCol[7] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*6)))*16.0f;
						if ( fTexSelectorCol[7] < 0 ) fTexSelectorCol[7] = 0;
						fTexSelectorCol[8] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*7)))*16.0f;
						if ( fTexSelectorCol[8] < 0 ) fTexSelectorCol[8] = 0;
						fTexSelectorCol[9] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*8)))*16.0f;
						if ( fTexSelectorCol[9] < 0 ) fTexSelectorCol[9] = 0;
						fTexSelectorCol[10] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*9)))*16.0f;
						if ( fTexSelectorCol[10] < 0 ) fTexSelectorCol[10] = 0;
						fTexSelectorCol[11] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*10)))*16.0f;
						if ( fTexSelectorCol[11] < 0 ) fTexSelectorCol[11] = 0;
						fTexSelectorCol[12] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*11)))*16.0f;
						if ( fTexSelectorCol[12] < 0 ) fTexSelectorCol[12] = 0;
						fTexSelectorCol[13] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*12)))*16.0f;
						if ( fTexSelectorCol[13] < 0 ) fTexSelectorCol[13] = 0;
						fTexSelectorCol[14] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*13)))*16.0f;
						if ( fTexSelectorCol[14] < 0 ) fTexSelectorCol[14] = 0;
						fTexSelectorCol[15] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*14)))*16.0f;
						if ( fTexSelectorCol[15] < 0 ) fTexSelectorCol[15] = 0;
						fTexSelectorCol[16] = (float)(0.0625f-fabs(fTexSelectorV-(0.0625f*15)))*16.0f;
						if ( fTexSelectorCol[16] < 0 ) fTexSelectorCol[16] = 0;

						// get reference into memblock mini-texture lookup
						int tx = x-(int(x/4)*4);
						int tz = y-(int(y/4)*4);

						// get pointer to memblock
						DWORD* pMemBlockPot = (DWORD*)GetMemblockPtr ( iMemblockAssistor );

						// get weighted contrib from each pot
						DWORD dwTexPartD[17];
						int texpartdr[17];
						int texpartdg[17];
						int texpartdb[17];
						for ( int i = 1; i <= 16; i++ )
						{
							dwTexPartD[i] = *(pMemBlockPot+(i*16)+(tz*4)+tx);
							texpartdr[i] = (int)(((dwTexPartD[i] & 0x00FF0000) >> 16) * fTexSelectorCol[i]);
							texpartdg[i] = (int)(((dwTexPartD[i] & 0x0000FF00) >> 8 ) * fTexSelectorCol[i]);
							texpartdb[i] = (int)(((dwTexPartD[i] & 0x000000FF)      ) * fTexSelectorCol[i]);
						}

						// combine to a single colour
						int diffusemapr = 0;
						int diffusemapg = 0;
						int diffusemapb = 0;
						for ( int i = 1; i <= 16; i++ ) diffusemapr += texpartdr[i];
						for ( int i = 1; i <= 16; i++ ) diffusemapg += texpartdg[i];
						for ( int i = 1; i <= 16; i++ ) diffusemapb += texpartdb[i];
						if ( diffusemapr>255 ) diffusemapr=255;
						if ( diffusemapg>255 ) diffusemapg=255;
						if ( diffusemapb>255 ) diffusemapb=255;

						// write result into destination, preserve the alpha channel of the dest
						DWORD trgb = (diffusemapr<<16)+(diffusemapg<<8)+(diffusemapb);
						DWORD trgba = *(DWORD*)pDst;
						DWORD talpha = trgba & 0xFF000000;
						*(DWORD*)pDst = talpha + trgb;
					}
					pSrc+=4;
					pDst+=4;
				}
			}
			pSrcSurface->UnlockRect();
		}
		pDstSurface->UnlockRect();
	}
	SAFE_RELEASE(pDstSurface);
	SAFE_RELEASE(pSrcSurface);
	#endif
}

DARKSDK void PasteImageCore ( int iID, int iX, int iY, int iFlag )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}
	if ( !UpdatePtrImage ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_IMAGENOTEXIST);
		return;
	}

	// use sprite library to paste image(texture) with polys!
	PasteImage( iID, iX, iY, m_imgptr->fTexUMax, m_imgptr->fTexVMax, iFlag );

	return;
}

DARKSDK void StretchImage ( int iID, int Width, int Height )
{
	// returns true if the image exists
	if ( !UpdatePtrImage ( iID ) )
		return;

	// First ensure texture is not already stretched
	#ifdef DX11
	GGSURFACE_DESC ddsd;
	LPGGSURFACE pSurface = NULL;
	m_imgptr->lpTexture->QueryInterface<ID3D11Texture2D>(&pSurface);
	pSurface->GetDesc(&ddsd);
	SAFE_RELEASE(pSurface);
	if ( ddsd.Width==(DWORD)GetPowerSquareOfSize(ddsd.Width)
	&& ddsd.Height==(DWORD)GetPowerSquareOfSize(ddsd.Height)
	&& m_imgptr->fTexUMax==1.0f && m_imgptr->fTexVMax==1.0f)
	{
		// Image is already power of two and fills entire surface
		return;
	}
	#else
	D3DSURFACE_DESC ddsd;
	LPGGSURFACE pSurface = NULL;
	m_imgptr->lpTexture->GetSurfaceLevel(0, &pSurface);
	pSurface->GetDesc(&ddsd);
	SAFE_RELEASE(pSurface);
	if ( ddsd.Width==(DWORD)GetPowerSquareOfSize(ddsd.Width)
	&& ddsd.Height==(DWORD)GetPowerSquareOfSize(ddsd.Height)
	&& m_imgptr->fTexUMax==1.0f && m_imgptr->fTexVMax==1.0f)
	{
		// Image is already power of two and fills entire surface
		return;
	}

	// Record original Size
	int iOrigWidth = m_imgptr->iWidth;
	int iOrigHeight = m_imgptr->iHeight;

	// Create temp store texture
	LPGGTEXTURE pNewTexture = NULL;
	HRESULT hr = D3DXCreateTexture (  m_pD3D,
									  m_imgptr->iWidth,
									  m_imgptr->iHeight,
									  1,
									  D3DX_DEFAULT,
									  g_DefaultGGFORMAT,
									  D3DPOOL_MANAGED,
									  &pNewTexture			);

	// leave if failed
	if ( pNewTexture==NULL )
		return;

	// Copy this image to store
	LPGGSURFACE pNewSurface = NULL;
	pNewTexture->GetSurfaceLevel(0, &pNewSurface);
	LPGGSURFACE pTexSurface = NULL;
	m_imgptr->lpTexture->GetSurfaceLevel(0, &pTexSurface);
	RECT scrRect = { 0, 0, m_imgptr->iWidth, m_imgptr->iHeight };
	hr = D3DXLoadSurfaceFromSurface(pNewSurface, NULL, NULL, pTexSurface, NULL, &scrRect, D3DX_DEFAULT, 0);
	SAFE_RELEASE(pNewSurface);
	SAFE_RELEASE(pTexSurface);

	// Create new Image, deleting the old one
	DeleteImageCore ( iID );
	MakeFormat ( iID, Width, Height, g_DefaultGGFORMAT, 0 );

	// LEEFIX : 140902 : When created a new texture, the m_imgptr can be different so update it
	if ( !UpdatePtrImage ( iID ) )
	{
		SAFE_RELEASE(pNewSurface);
		SAFE_RELEASE(pTexSurface);
		SAFE_RELEASE(pNewTexture);
		return;
	}

	// Copy store to new image
	pNewTexture->GetSurfaceLevel(0, &pNewSurface);
	m_imgptr->lpTexture->GetSurfaceLevel(0, &pTexSurface);
	hr = D3DXLoadSurfaceFromSurface(pTexSurface, NULL, NULL, pNewSurface, NULL, NULL, D3DX_DEFAULT, 0);
	SAFE_RELEASE(pNewSurface);
	SAFE_RELEASE(pTexSurface);

	// Image is still regarded as original size
	m_imgptr->iWidth = iOrigWidth;			// store the width
	m_imgptr->iHeight  = iOrigHeight;		// store the height

	// Texture takes up entire area
	m_imgptr->fTexUMax=1.0f;
	m_imgptr->fTexVMax=1.0f;

	// Free temp store texture
	SAFE_RELEASE(pNewTexture);
	#endif

	// Complete
	return;
}

DARKSDK void PasteImageCore ( int iID, int iX, int iY )
{
	PasteImageCore ( iID, iX, iY, 0 );
}

DARKSDK void DeleteImageCore ( int iID )
{
	if ( !UpdatePtrImage ( iID ) )
		return;

	// before release, remove the reference from ALL objects
	// leeadd - 220604 - u54 - scans every object that uses this texture address
	ClearObjectsOfTextureRef ( m_imgptr->lpTexture );

	// clear cube details
	m_imgptr->pCubeMapRef = NULL;
	m_imgptr->iCubeMapFace = 0;

    RemoveImage ( iID );
}

//
// Command Functions
//

//Forward declclaration of timestampactivity since it is housed elsewhere
void timestampactivity ( int i, char* desc_s );

DARKSDK void LoadImage ( LPSTR szFilename, int iID, int iTextureFlag, int iDivideTextureSize, int iSilentError )
{
	if(LoadImageCore( szFilename, iID, iTextureFlag, false, iDivideTextureSize )==false)
	{
		if ( iSilentError==0 )
		{
			char pCWD[256]; _getcwd ( pCWD, 256 );
			char pErr[256]; sprintf ( pErr, "CWD:%s\nLOAD IMAGE %s,%d,%d,%d", pCWD, szFilename, iID, iTextureFlag, iDivideTextureSize);
			timestampactivity(0, pErr);
		}
	}
}

DARKSDK void LoadImage ( LPSTR szFilename, int iID, int iKindOfTexture, int iDivideTextureSize )
{
	return LoadImage ( szFilename, iID, iKindOfTexture, iDivideTextureSize, 0 );
}

DARKSDK void LoadImage ( LPSTR szFilename, int iID, int iKindOfTexture )
{
    LoadImage ( szFilename, iID, iKindOfTexture, 0 );
}

DARKSDK void LoadImage ( LPSTR szFilename, int iID )
{
	int iKindOfTexture = 0;
    LoadImage ( szFilename, iID, iKindOfTexture, 0 );
}

DARKSDK void SaveImage ( LPSTR szFilename, int iID )
{
	SaveImageCore ( szFilename, iID );
}

DARKSDK void SaveImage ( LPSTR szFilename, int iID, int iCompressionMode )
{
	SaveImageCore ( szFilename, iID, iCompressionMode );
}

DARKSDK void GrabImage ( int iID, int iX1, int iY1, int iX2, int iY2 )
{
	GrabImageCore ( iID, iX1, iY1, iX2, iY2 );
}

DARKSDK void GrabImage ( int iID, int iX1, int iY1, int iX2, int iY2, int iTextureFlag )
{
	GrabImageCore ( iID, iX1, iY1, iX2, iY2, iTextureFlag );
}

DARKSDK void PasteImage ( int iID, int iX, int iY )
{
	PasteImageCore ( iID, iX, iY, 0 );
}

DARKSDK void PasteImage ( int iID, int iX, int iY, int iFlag )
{
	PasteImageCore ( iID, iX, iY, iFlag );
}

DARKSDK void DeleteImage ( int iID )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return;
	}
	if ( !UpdatePtrImage ( iID ) )
	{
		RunTimeError(RUNTIMEERROR_IMAGENOTEXIST);
		return;
	}
	DeleteImageCore ( iID );
}

DARKSDK void RotateImage ( int iID, int iAngle )
{
	// Not Implemented in DBPRO V1 RELEASE
	RunTimeError(RUNTIMEERROR_COMMANDNOWOBSOLETE);
}

DARKSDK int GetImageExistEx ( int iID )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return 0;
	}

	// returns true if the image exists
	if ( !UpdatePtrImage ( iID ) )
		return 0;

	// return true
	return 1;
}

DARKSDK LPSTR GetImageName ( int iID )
{
	if(iID<1 || iID>MAXIMUMVALUE)
	{
		RunTimeError(RUNTIMEERROR_IMAGEILLEGALNUMBER);
		return NULL;
	}

	// returns true if the image exists
	if ( !UpdatePtrImage ( iID ) )
		return NULL;

	// return true
	return m_imgptr->szShortFilename;
}

//
// New Command Functions
//

DARKSDK void SetImageColorKey ( int iR, int iG, int iB )
{
	// set the color key of an image
	m_Color = GGCOLOR_ARGB ( 255, iR, iG, iB );
}

DARKSDK bool FileExist ( LPSTR szFilename )
{
	GGIMAGE_INFO info;
	HRESULT hRes = 0;
	#ifdef DX11
	#else
	if ( g_bImageBlockActive && g_iImageBlockMode==1 )
	{
		DWORD dwFileInMemorySize = 0;
		LPVOID pFileInMemoryData = NULL;
		char pFinalRelPathAndFile[512];
		GetFileInMemory ( szFilename, &pFileInMemoryData, &dwFileInMemorySize, pFinalRelPathAndFile );
		hRes = D3DXGetImageInfoFromFileInMemory( pFileInMemoryData, dwFileInMemorySize, &info );
	}
	else
		hRes = D3DXGetImageInfoFromFile( szFilename, &info );
	#endif

	if ( hRes==GG_OK )
		return true;
	else
		return false;
}

DARKSDK DWORD LoadIcon ( LPSTR pFilename )
{
	// load icon
	HICON hIconHandle = (HICON)LoadImageA ( NULL, pFilename, IMAGE_ICON, 48, 48, LR_LOADFROMFILE );

	// complete
	return (DWORD)hIconHandle;
}

DARKSDK void FreeIcon ( DWORD dwIcon )
{
	// free icon handle
    CloseHandle ( (HICON)dwIcon );
}

//
// Data Access Functions
//

DARKSDK void GetImageData( int iID, DWORD* dwWidth, DWORD* dwHeight, DWORD* dwDepth, LPSTR* pData, DWORD* dwDataSize, bool bLockData )
{
	// Read Data
	if(bLockData==true)
	{
		if ( !UpdatePtrImage ( iID ) )
			return;

		if ( m_imgptr->lpTexture==NULL )
			return;

		// data
		*dwWidth = m_imgptr->iWidth;
		*dwHeight = m_imgptr->iHeight;
		*dwDepth = m_imgptr->iDepth;
		DWORD bitdepth = m_imgptr->iDepth/8;

		#ifdef DX11
		// use actual size, not image size
		LPGGSURFACE pTextureInterface = NULL;
		m_imgptr->lpTexture->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
		D3D11_TEXTURE2D_DESC desc;
		pTextureInterface->GetDesc(&desc);
		if(desc.Width<*dwWidth) *dwWidth=desc.Width;
		if(desc.Height<*dwHeight) *dwHeight=desc.Height;
		SAFE_RELEASE ( pTextureInterface );

		// create system memory version
		ID3D11Texture2D* pTempSysMemTexture = NULL;
		//D3D11_TEXTURE2D_DESC StagedDesc = { desc.Width, desc.Height, 1, 1, GGFMT_A8R8G8B8, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0 };
		D3D11_TEXTURE2D_DESC StagedDesc = { desc.Width, desc.Height, 1, 1, desc.Format, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0 };
		m_pD3D->CreateTexture2D( &StagedDesc, NULL, &pTempSysMemTexture );
		if ( pTempSysMemTexture )
		{
			// and copy texture image to it
			D3D11_BOX rc = { 0, 0, 0, (LONG)(*dwWidth), (LONG)(*dwHeight), 1 }; 
			m_pImmediateContext->CopySubresourceRegion(pTempSysMemTexture, 0, 0, 0, 0, m_imgptr->lpTexture, 0, &rc);

			// lock for reading staging texture
			GGLOCKED_RECT d3dlock;
			if(SUCCEEDED(m_pImmediateContext->Map(pTempSysMemTexture, 0, D3D11_MAP_READ, 0, &d3dlock)))
			{
				// copy data
				DWORD dwSizeOfBitmapData = (*dwWidth)*(*dwHeight)*bitdepth;
				*pData = new char[dwSizeOfBitmapData];
				*dwDataSize = dwSizeOfBitmapData;

				// copy from surface
				LPSTR pSrc = (LPSTR)d3dlock.pData;
				LPSTR pPtr = *pData;
				DWORD dwDataWidth=*(dwWidth)*bitdepth;
				for(DWORD y=0; y<*dwHeight; y++)
				{
					memcpy(pPtr, pSrc, dwDataWidth);
					pPtr+=dwDataWidth;
					pSrc+=d3dlock.RowPitch;
				}
				m_pImmediateContext->Unmap(pTempSysMemTexture, 0);
			}
			else
			{
				// leefix - 250604 - u54 - place a one in size to indicate could not lock (exists but protected)
				*dwDataSize = 1;
			}

			// free temp system surface
			SAFE_RELEASE(pTempSysMemTexture);
		}
		#else
		// use actual size, not image size
		D3DSURFACE_DESC desc;
		m_imgptr->lpTexture->GetLevelDesc(0,&desc);
		if(desc.Width<*dwWidth) *dwWidth=desc.Width;
		if(desc.Height<*dwHeight) *dwHeight=desc.Height;

		// create system memory version
		LPGGTEXTURE pTempSysMemTexture = NULL;
		D3DXCreateTexture( m_pD3D, desc.Width, desc.Height, 1, 0, desc.Format, D3DPOOL_SYSTEMMEM, &pTempSysMemTexture );

		// and copy texture image to it
		LPGGSURFACE pTempSysMemSurface = NULL;
		LPGGSURFACE pTempVidMemSurface = NULL;
		pTempSysMemTexture->GetSurfaceLevel ( 0, &pTempSysMemSurface );
		m_imgptr->lpTexture->GetSurfaceLevel ( 0, &pTempVidMemSurface );
		//m_pD3D->UpdateSurface ( pTempVidMemSurface, NULL, pTempSysMemSurface, NULL);
		D3DXLoadSurfaceFromSurface ( pTempSysMemSurface, 0, NULL, pTempVidMemSurface, 0, NULL, D3DX_DEFAULT, 0);

		// lock
		GGLOCKED_RECT d3dlock;
		RECT rc = { 0, 0, (LONG)(*dwWidth), (LONG)(*dwHeight) };
		if(SUCCEEDED(pTempSysMemTexture->LockRect ( 0, &d3dlock, &rc, 0 ) ) )
		{
			// create memory
			DWORD dwSizeOfBitmapData = (*dwWidth)*(*dwHeight)*bitdepth;
			*pData = new char[dwSizeOfBitmapData];
			*dwDataSize = dwSizeOfBitmapData;

			// copy from surface
			LPSTR pSrc = (LPSTR)d3dlock.pBits;
			LPSTR pPtr = *pData;
			DWORD dwDataWidth=*(dwWidth)*bitdepth;
			for(DWORD y=0; y<*dwHeight; y++)
			{
				memcpy(pPtr, pSrc, dwDataWidth);
				pPtr+=dwDataWidth;
				pSrc+=d3dlock.Pitch;
			}
			pTempSysMemTexture->UnlockRect(0);
		}
		else
		{
			// leefix - 250604 - u54 - place a one in size to indicate could not lock (exists but protected)
			*dwDataSize = 1;
		}

		// free temp system surface
		SAFE_RELEASE(pTempVidMemSurface);
		SAFE_RELEASE(pTempSysMemSurface);
		SAFE_RELEASE(pTempSysMemTexture);
		#endif
	}
	else
	{
		// free memory
		delete *pData;
	}
}

DARKSDK void SetImageData( int iID, DWORD dwWidth, DWORD dwHeight, DWORD dwDepth, LPSTR pData, DWORD dwDataSize )
{
	if ( UpdatePtrImage ( iID ) )
	{
		if ( m_imgptr->lpTexture==NULL )
			return;

		// Check new specs with existing one
		if(dwWidth==(DWORD)m_imgptr->iWidth && dwHeight==(DWORD)m_imgptr->iHeight && dwDepth==(DWORD)m_imgptr->iDepth)
		{
			// Same size
		}
		else
		{
			// Recreate
			DeleteImageCore ( iID );
			m_imgptr=NULL;
		}
	}

	// new image
	GGFORMAT destImageFormat = GGFMT_A8R8G8B8;
	if(m_imgptr==NULL)
	{
		MakeFormat ( iID, dwWidth, dwHeight, destImageFormat, 0 );
	}

	// may have changed
	if ( !UpdatePtrImage ( iID ) ) return;
	if ( m_imgptr->lpTexture==NULL ) return;

	// write Data
	#ifdef DX11
	LPGGSURFACE pTempTexture = NULL;
	GGSURFACE_DESC TempTextureDesc = { dwWidth, dwHeight, 1, 1, destImageFormat, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE, 0 };
	m_pD3D->CreateTexture2D( &TempTextureDesc, NULL, &pTempTexture );
	if ( pTempTexture )
	{
		GGLOCKED_RECT d3dlock;
		DWORD bitdepth = m_imgptr->iDepth/8;
		if(SUCCEEDED(m_pImmediateContext->Map(pTempTexture, 0, D3D11_MAP_WRITE, 0, &d3dlock)))
		{
			LPSTR pDest = (LPSTR)d3dlock.pData;
			LPSTR pPtr = pData;
			DWORD dwDataWidth=dwWidth*bitdepth;
			for(DWORD y=0; y<dwHeight; y++)
			{
				memcpy(pDest, pPtr, dwDataWidth);
				pPtr+=dwDataWidth;
				pDest+=d3dlock.RowPitch;
			}
			m_pImmediateContext->Unmap(pTempTexture,0);
		}

		// copy staging texture to shader resource
		m_pImmediateContext->CopyResource ( m_imgptr->lpTexture, pTempTexture );

		// free work resources
		SAFE_RELEASE(pTempTexture);
	}
	#else
	GGLOCKED_RECT d3dlock;
	DWORD bitdepth = m_imgptr->iDepth/8;
	RECT rc = { 0, 0, (LONG)dwWidth, (LONG)dwHeight };
	if(SUCCEEDED(m_imgptr->lpTexture->LockRect ( 0, &d3dlock, &rc, 0 ) ) )
	{
		// copy from surface
		LPSTR pSrc = (LPSTR)d3dlock.pBits;
		LPSTR pPtr = pData;
		DWORD dwDataWidth=dwWidth*bitdepth;
		for(DWORD y=0; y<dwHeight; y++)
		{
			memcpy(pSrc, pPtr, dwDataWidth);
			pPtr+=dwDataWidth;
			pSrc+=d3dlock.Pitch;
		}
		m_imgptr->lpTexture->UnlockRect(0);
	}
	#endif

	// ensure sprites all updated
	UpdateAllSprites();
}

//
// IMAGE BLOCK CODE
//

void OpenImageBlock	( char* szFilename, int iMode )
{
	// cannot open if already open
	if ( g_iImageBlockMode!=-1 )
		return;

	// Reset exclude path
	strcpy ( g_pImageBlockExcludePath, "" );

	// Create image block details
	g_iImageBlockFilename = new char [ strlen ( szFilename ) + 1 ];
	strcpy ( g_iImageBlockFilename, szFilename );

	// Create path to image block
	char current [ 512 ];
	_getcwd ( current, 512 );
	g_iImageBlockRootPath = new char [ strlen ( current ) + 2 ];
	strcpy ( g_iImageBlockRootPath, current );
	if ( g_iImageBlockRootPath [ strlen(g_iImageBlockRootPath)-1 ]!='\\' )
	{
		// add folder divide at end of path string
		int iLen = strlen(g_iImageBlockRootPath);
		g_iImageBlockRootPath [ iLen+0 ] = '\\';
		g_iImageBlockRootPath [ iLen+1 ] = 0;
	}
	
	// Set the imageblock mode (0-write, 1-read)
	g_bImageBlockActive = true;
	g_iImageBlockMode = iMode;

	// U77 - 060211 - does previously written image block exist, if so, we append to it
	bool bPreviousImageBlockExists = false;
	HANDLE hFile = CreateFile ( g_iImageBlockFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile!=INVALID_HANDLE_VALUE )
	{
		bPreviousImageBlockExists = true;
		CloseHandle ( hFile );
	}

	// Load imageblock (for reading or to load last imageblock from previous write)
	if ( g_iImageBlockMode==1 || bPreviousImageBlockExists==true )
	{
		// open to read
		HANDLE hFile = CreateFile ( g_iImageBlockFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		DWORD dwReadBytes = 0;

		// read file list
		int iListMax = 0;
		g_ImageBlockListFile.clear();
		ReadFile ( hFile, &iListMax, sizeof(int), &dwReadBytes, NULL );
		for ( int i = 0; i < iListMax; i++ )
		{
			// write filename length and string
			DWORD dwFilenameLength = 0;
			ReadFile ( hFile, &dwFilenameLength, sizeof(DWORD), &dwReadBytes, NULL );
			LPSTR pFile = new char [ dwFilenameLength ];
			ReadFile ( hFile, pFile, dwFilenameLength, &dwReadBytes, NULL );
			g_ImageBlockListFile.push_back ( pFile );

			// write file offset in data
			DWORD dwListOffset = 0;
			ReadFile ( hFile, &dwListOffset, sizeof(DWORD), &dwReadBytes, NULL );
			g_ImageBlockListOffset.push_back ( dwListOffset );

			// write file size in data
			DWORD dwListSize = 0;
			ReadFile ( hFile, &dwListSize, sizeof(DWORD), &dwReadBytes, NULL );
			g_ImageBlockListSize.push_back ( dwListSize );
		}

		// write the imageblock data itself
		ReadFile ( hFile, &g_dwImageBlockSize, sizeof(DWORD), &dwReadBytes, NULL );
		g_pImageBlockPtr = new char [ g_dwImageBlockSize ];
		ReadFile ( hFile, g_pImageBlockPtr, g_dwImageBlockSize, &dwReadBytes, NULL );
	
		// close file
		CloseHandle ( hFile );
	}
	else
	{
		// Create memory block for saving
		g_dwImageBlockSize = 0;
		g_pImageBlockPtr = NULL;

		// Clear file list
		g_ImageBlockListFile.clear();
		g_ImageBlockListOffset.clear();
		g_ImageBlockListSize.clear();
	}
}

void ExcludeFromImageBlock ( char* szExcludePath )
{
	// exclude any file starting with this string
    if (szExcludePath)
    	strcpy ( g_pImageBlockExcludePath, szExcludePath );
    else
        g_pImageBlockExcludePath[0] = 0;
}

bool AddToImageBlock ( LPSTR pAddFilename )
{
	// can only add in write mode
	if ( g_iImageBlockMode!=0 ) return true;

	// if exist
	if ( !pAddFilename ) return false;

	// exclude if path matches excluder, but only if excluder has a value
    if (g_pImageBlockExcludePath && g_pImageBlockExcludePath[0])
    	if ( strnicmp ( g_pImageBlockExcludePath, pAddFilename, strlen(g_pImageBlockExcludePath) )==NULL )
	    	return true;

	// ensure it does not already exist
	for ( int i = 0; i < (int)g_ImageBlockListFile.size ( ); i++ )
		if ( _stricmp ( g_ImageBlockListFile [ i ], pAddFilename )==NULL )
			return true;

	// open the file
	HANDLE hFile = CreateFile ( pAddFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile==INVALID_HANDLE_VALUE ) return false;

	// read the file data
	DWORD dwReadBytes = 0;
	DWORD dwFileSize = GetFileSize ( hFile, NULL );
	LPSTR pFileData = new char [ dwFileSize ];
	ReadFile ( hFile, pFileData, dwFileSize, &dwReadBytes, NULL );
	
	// close file
	CloseHandle ( hFile );

	// create space in the imageblock
	DWORD dwNewSize = g_dwImageBlockSize + dwFileSize;

	// U76 - 020710 - image blocks can get LARGE enough to overrun virtual address space
	// so catch the exeption raised in this case and end the image block creation process gracefully
	LPSTR pNewData = NULL;
	try
	{
		pNewData = new char [ dwNewSize ];
	}
	catch(...)
	{
		// failed to create a new "continuous" block of memory in virtual address space
		// so end image block creation and exit here
		SAFE_DELETE ( pFileData );
		CloseImageBlock();
		return false;
	}
	memcpy ( pNewData, g_pImageBlockPtr, g_dwImageBlockSize );

	// add data to the imageblock
	LPSTR pNewDataInsertPtr = pNewData + g_dwImageBlockSize;
	DWORD dwOffsetToDataInImageBlock = g_dwImageBlockSize;
	memcpy ( pNewDataInsertPtr, pFileData, dwFileSize );

	// free individual file data
	SAFE_DELETE ( pFileData );

	// erase old imageblock and use new one
	SAFE_DELETE ( g_pImageBlockPtr );
	g_dwImageBlockSize = dwNewSize;
	g_pImageBlockPtr = pNewData;

	// add it to list
	LPSTR pListFilename = new char [ strlen ( pAddFilename )+1 ];
	strcpy ( pListFilename, pAddFilename );
	g_ImageBlockListFile.push_back ( pListFilename );
	g_ImageBlockListOffset.push_back ( dwOffsetToDataInImageBlock );
	g_ImageBlockListSize.push_back ( dwFileSize );

	// success
	return true;
}

LPSTR RetrieveFromImageBlock ( LPSTR pRetrieveFilename, DWORD* pdwFileSize )
{
	// find the file
	int iIndexInListFound = -1;
	for ( int iIndexInList = 0; iIndexInList < (int)g_ImageBlockListFile.size ( ); iIndexInList++ )
	{
		if ( _stricmp ( g_ImageBlockListFile [ iIndexInList ], pRetrieveFilename )==NULL )
		{
			iIndexInListFound = iIndexInList;
			break;
		}
	}
	if ( iIndexInListFound==-1 )
	{
		// not found 
		return NULL;
	}

	// locate file within imageblock
	DWORD dwOffset = g_ImageBlockListOffset [ iIndexInListFound ];
	DWORD dwSize = g_ImageBlockListSize [ iIndexInListFound ];

	// return ptr and size
	if ( pdwFileSize ) *pdwFileSize = dwSize;
	return g_pImageBlockPtr + dwOffset;
}

void CloseImageBlock ( void )
{
	// cannot close if already closed
	if ( g_iImageBlockMode==-1 )
		return;

	// Save imageblock
	if ( g_iImageBlockMode==0 )
	{
		// set original path
		char storedir [ 512 ];
		_getcwd ( storedir, 512 );
		_chdir ( g_iImageBlockRootPath );

		// open to write
		HANDLE hFile = CreateFile ( g_iImageBlockFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		DWORD dwWrittenBytes = 0;

		// write file list
		int iListMax = g_ImageBlockListFile.size ( );
		WriteFile ( hFile, &iListMax, sizeof(int), &dwWrittenBytes, NULL );
		for ( int i = 0; i < iListMax; i++ )
		{
			// write filename length and string
			LPSTR pListPtr = g_ImageBlockListFile [ i ];
			DWORD dwFilenameLength = strlen(pListPtr)+1;
			WriteFile ( hFile, &dwFilenameLength, sizeof(DWORD), &dwWrittenBytes, NULL );
			WriteFile ( hFile, pListPtr, dwFilenameLength, &dwWrittenBytes, NULL );

			// write file offset in data
			DWORD dwListOffset = g_ImageBlockListOffset [ i ];
			WriteFile ( hFile, &dwListOffset, sizeof(DWORD), &dwWrittenBytes, NULL );

			// write file size in data
			DWORD dwListSize = g_ImageBlockListSize [ i ];
			WriteFile ( hFile, &dwListSize, sizeof(DWORD), &dwWrittenBytes, NULL );
		}

		// write the imageblock data itself
		WriteFile ( hFile, &g_dwImageBlockSize, sizeof(DWORD), &dwWrittenBytes, NULL );
		WriteFile ( hFile, g_pImageBlockPtr, g_dwImageBlockSize, &dwWrittenBytes, NULL );
	
		// close file
		CloseHandle ( hFile );

		// restore folder
		_chdir ( storedir );
	}

	// free strings in imageblock file list
	for ( int i = 0; i < (int)g_ImageBlockListFile.size(); i++ )
		SAFE_DELETE ( g_ImageBlockListFile [ i ] );
	g_ImageBlockListFile.clear();
	g_ImageBlockListOffset.clear();
	g_ImageBlockListSize.clear();

	// free filename
	SAFE_DELETE ( g_iImageBlockFilename );
	SAFE_DELETE ( g_iImageBlockRootPath );

	// Close imageblock
	SAFE_DELETE ( g_pImageBlockPtr );

	// Switch off imageblock
	g_bImageBlockActive = false;
	g_iImageBlockMode = -1;
}

void PerformChecklistForImageBlockFiles ( void )
{
	// Generate Checklist
	DWORD dwMaxStringSizeInEnum=0;
	bool bCreateChecklistNow=false;
	g_pGlob->checklisthasvalues=false;
	g_pGlob->checklisthasstrings=true;
	for(int pass=0; pass<2; pass++)
	{
		if(pass==1)
		{
			// Ensure checklist is large enough
			bCreateChecklistNow=true;
			for(int c=0; c<g_pGlob->checklistqty; c++)
				GlobExpandChecklist(c, dwMaxStringSizeInEnum);
		}

		// Look at parameters
		g_pGlob->checklistqty=0;
		for ( int i = 0; i < (int)g_ImageBlockListFile.size ( ); i++ )
		{
			// write filename length and string
			LPSTR pListPtr = g_ImageBlockListFile [ i ];
			if ( !pListPtr ) continue;

			// Add to checklist
			DWORD dwSize = strlen(pListPtr);
			if(dwSize>dwMaxStringSizeInEnum) dwMaxStringSizeInEnum=dwSize;
			if(bCreateChecklistNow)
			{
				// New checklist item
				strcpy(g_pGlob->checklist[g_pGlob->checklistqty].string, pListPtr);
				g_pGlob->checklist[g_pGlob->checklistqty].valuea = 0;
				g_pGlob->checklist[g_pGlob->checklistqty].valueb = 0;
				g_pGlob->checklist[g_pGlob->checklistqty].valuec = 0;
				g_pGlob->checklist[g_pGlob->checklistqty].valued = 0;
			}
			g_pGlob->checklistqty++;
		}
	}
 
	// Determine if checklist has any contents
	if(g_pGlob->checklistqty>0)
		g_pGlob->checklistexists=true;
	else
		g_pGlob->checklistexists=false;
}

int GetImageFileExist ( LPSTR pFilename )
{
	// If no string, no file
	if ( pFilename==NULL ) return 0;
	char VirtualFilename[_MAX_PATH];
	strcpy(VirtualFilename, pFilename);

	CheckForWorkshopFile ( VirtualFilename );

	// if image block file, quick early out using imageblock
	if ( g_bImageBlockActive && g_iImageBlockMode==1 )
	{
		// final storage string of path and file resolver (makes the filename and path uniform for imageblock retrieval)
		// and work out true file and path, then look for it in imageblock
		char pFinalRelPathAndFile[512];
		LPVOID pFileInMemoryData = 0;
		DWORD dwFileInMemorySize = 0;
		GetFileInMemory ( VirtualFilename, &pFileInMemoryData, &dwFileInMemorySize, pFinalRelPathAndFile );
		if ( pFileInMemoryData )
			return 1;
	}

	// real file
	HANDLE hfile = CreateFile ( VirtualFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hfile==INVALID_HANDLE_VALUE)
		return 0;

	// success, it exists
	CloseHandle(hfile);
	return 1;
}

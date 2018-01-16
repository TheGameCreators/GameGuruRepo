#include "DirectXTex.h"
using namespace DirectX;

#include "LMGlobal.h"
#include "DBPro Functions.h"

#include "LMTexture.h"
#include "Lumel.h"
#include "LMObject.h"
#include "LMPolyGroup.h"
#include "LMTexNode.h"
#include "Light.h"
#include "CollisionTreeLightmapper.h"

// Externs
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;

extern HANDLE g_hLMHeap;

LMTexture::LMTexture( int sizeU, int sizeV )
{
	New ( sizeU, sizeV );
}

LMTexture::~LMTexture( )
{
	Free();
}

void LMTexture::New ( int sizeU, int sizeV )
{
	iSizeU = sizeU;
	iSizeV = sizeV;
	bEmpty = true;

	ppTexLumel = (LumelLite**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	DWORD dwLumelSize = sizeof(LumelLite);
	for ( int i = 0; i < iSizeU; i++ ) ppTexLumel [ i ] = (LumelLite*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * dwLumelSize);
	pNodeSpace = new LMTexNode( 0, 0, sizeU, sizeV );

	pNextLMTex = 0;
	pNodeSpace->Finalise( );
	
	pPixels = 0;
	iPitch = 0;
	iWidth = 0;
	iHeight = 0;

	pTexture = 0;
	pTextureDDS = 0;

	sprintf_s( pFilename, 255, "0" );
}

void LMTexture::Free ( )
{
	if ( ppTexLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppTexLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppTexLumel );
	}

	/* 
	if ( ppTexLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) delete [] ( ppTexLumel [ i ] );
		delete [] ppTexLumel;
	}
	*/

	if ( pNodeSpace ) delete pNodeSpace;

	//U75 - 060510 - new LM heap means we don't recurse deletes
	//if ( pNextLMTex ) delete pNextLMTex;
}

bool LMTexture::IsEmpty( ) { return bEmpty; }

int LMTexture::GetSizeU( ) { return iSizeU; }
int LMTexture::GetSizeV( ) { return iSizeV; }

bool LMTexture::AddLMObject( LMObject *pLMObject )
{
	LMPolyGroup* pPoly = pLMObject->GetFirstGroup( );

	//FILE* pData = fopen("TextureData.txt","w");
	//char infoStr[256];
		
	bool bFits = true;

	while ( pPoly && bFits )
	{
		int iPolySizeU = pPoly->GetScaledSizeU( );
		int iPolySizeV = pPoly->GetScaledSizeV( );

		LMTexNode* pNode = pNodeSpace->AddPoly( iPolySizeU, iPolySizeV );

		if ( !pNode )
		{
			//polygon didn't fit
			bFits = false;
			pNodeSpace->BackTrack( );
			//sprintf(infoStr,"Didn't Fit, SizeU: %d, SizeV: %d\n", iPolySizeU, iPolySizeV );
			//fputs(infoStr,pData);
		}
		else
		{
			int iPolyStartU = pNode->GetStartU( );
			int iPolyStartV = pNode->GetStartV( );
			
			pPoly->SetStartPoint( iPolyStartU, iPolyStartV );
			//pPoly->fQuality = fQuality;

			//sprintf(infoStr,"Fits, StartU: %d, StartV: %d, SizeU: %d, SizeV: %d\n", iPolyStartU, iPolyStartV, iPolySizeU, iPolySizeV );
			//fputs(infoStr,pData);
			
			//assign polygon to Lumels
			/*
			for ( int u = iPolyStartU; u < iPolyStartU + iPolySizeU; u++ )
			{
				for ( int v = iPolyStartV; v < iPolyStartV + iPolySizeV; v++ )
				{
					ppTexLumel[u][v].pRefPoly = pPoly;
				}
			}
			*/

		}

		pPoly = pPoly->pNextGroup;
	}

	//fclose(pData);

	if ( !bFits )
	{
		//object did't fit in texture
		//if this texture was empty it'll never fit
		if ( bEmpty ) return false;
		
		if ( pNextLMTex )
		{
			return pNextLMTex->AddLMObject( pLMObject );
		}
		else
		{
			// U75 - 060510 - new LM heap
			//pNextLMTex = new LMTexture( iSizeU, iSizeV );
			pNextLMTex = (LMTexture*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, sizeof(LMTexture));
			pNextLMTex->New( iSizeU, iSizeV );
			return pNextLMTex->AddLMObject( pLMObject );
		}
	}
	else
	{
		//object completely within lightmap
		pNodeSpace->Finalise( );
		bEmpty = false;

		pLMObject->pLMTexture = this;

		return true;
	}
}
/*
void LMTexture::CalculateLight( Light *pLightList, CollisionTreeLightmapper *pColTree )
{
	if ( !pLightList ) return;

	Light *pCurrLight;
	float fRed, fGreen, fBlue;
	float fPosX=0, fPosY=0, fPosZ=0;
	float fLightPosX=0, fLightPosY=0, fLightPosZ=0;
	float fNormX=0, fNormY=0, fNormZ=0;
	float fDotP=0, fLength=0;
	Vector vec, vecN, vecI;
	Point p;

	//for each light
	pCurrLight = pLightList;
	while ( pCurrLight )
	{

		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				if ( ppTexLumel[u][v].pRefPoly )
				{
					ppTexLumel[u][v].pRefPoly->GetPoint( u, v, &fPosX, &fPosY, &fPosZ );
					ppTexLumel[u][v].pRefPoly->GetNormal( u, v, &fNormX, &fNormY, &fNormZ );

					pCurrLight->GetOrgin( fPosX, fPosY, fPosZ, &fLightPosX, &fLightPosY, &fLightPosZ );
					vec.set( fPosX-fLightPosX, fPosY-fLightPosY, fPosZ-fLightPosZ );
					fDotP = vec.x*fNormX + vec.y*fNormY + vec.z*fNormZ;

					if ( fDotP >= 0.0f || !pCurrLight->GetColorFromPoint( fPosX, fPosY, fPosZ, &fRed, &fGreen, &fBlue ) )
					{
						//pCurrLight = pCurrLight->pNextLight;
						continue;
					}
					
					//calculate + normalise the light->lumel vector
					fLength = vec.size( );
					vec.mult(1.0f/fLength);
					vecN = vec;
					vec.mult(fLength-0.1f);
					vecI.set( 1/vec.x, 1/vec.y, 1/vec.z );
					p.set ( fLightPosX, fLightPosY, fLightPosZ );
					
					//check collision
					if ( pColTree && pColTree->intersects( &p, &vec, &vecN, &vecI, true ) )
					{
					//	pCurrLight = pCurrLight->pNextLight;
						continue;
					}

					fDotP	/= fLength;
					fDotP	 = 0.0f - fDotP;
					fRed	*= fDotP;
					fGreen	*= fDotP;
					fBlue	*= fDotP;

					//add color to lumel
					ppTexLumel[u][v].AddCol( fRed, fGreen, fBlue );

					
				}
			}
		}

		pCurrLight = pCurrLight->pNextLight;
	}

	for ( int u = 0; u < iSizeU; u++ )
	{
		for ( int v = 0; v < iSizeV; v++ )
		{
			ppTexLumel[u][v].Ambient( Light::fAmbientR, Light::fAmbientG, Light::fAmbientB );
		}
	}
}
*/

void LMTexture::SetLumel( int u, int v, float red, float green, float blue )
{
	if ( u < 0 || v < 0 || u >= iSizeU || v >= iSizeV ) return;

	ppTexLumel[u][v].SetCol( red, green ,blue );
}

void LMTexture::CopyToTexture( )
{
	#ifdef DX11
	// create a staging texture for writing
	GGSURFACE_DESC StagedDesc = { iSizeU, iSizeV, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 0, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_WRITE, 0 };
	HRESULT hRes = m_pD3D->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&pTexture );
	if ( pTexture==NULL )
	{
		char ws[512];
		printf ( ws, "Error Creating Light Map Texture" );
		MessageBox( NULL, ws, "Error", 0 ); exit(1);
	}

	// lock texture to write
	D3D11_MAPPED_SUBRESOURCE d3dlr;
	HRESULT hr = m_pImmediateContext->Map( pTexture, 0, D3D11_MAP_WRITE, 0, &d3dlr );
	if ( FAILED( hr ) )
	{
		MessageBox( NULL, "Error Locking Texture", "Error", 0 ); exit(1);
	}
	pPixels = ( DWORD* ) d3dlr.pData;
	iPitch  = d3dlr.RowPitch / 4;
	iWidth  = iSizeU;
	iHeight = iSizeV;
	DWORD dwIndex;
	for ( int v = 0; v < iSizeV; v++ )
	{
		for ( int u = 0; u < iSizeU; u++ )
		{
			dwIndex = u + ( v * iPitch );
			//*( pPixels + dwIndex ) = ppTexLumel[u][v].GetColor( );
			*( pPixels + dwIndex ) = ppTexLumel[u][v].GetReversedColor( );
		}
	}
	pPixels = NULL;
	m_pImmediateContext->Unmap( pTexture, 0 );

	// free heaps
	if ( ppTexLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppTexLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppTexLumel );
	}
	ppTexLumel = 0;
	if ( pNodeSpace ) delete pNodeSpace;
	pNodeSpace = 0;

	// create GGFMT_DXT1 texture
	ScratchImage imageTexture;
	hr = CaptureTexture( m_pD3D, m_pImmediateContext, pTexture, imageTexture );
	if ( SUCCEEDED(hr) )
	{
		// compress to a DXT1 (BC1) texture
		ScratchImage convertedTexture;
		hr = Compress( imageTexture.GetImages(), imageTexture.GetImageCount(), imageTexture.GetMetadata(), 
			DXGI_FORMAT_BC1_UNORM, TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, convertedTexture );

		// create new DDS DXT1 texture
		CreateTexture(m_pD3D, convertedTexture.GetImages(), convertedTexture.GetImageCount(), convertedTexture.GetMetadata(), &pTextureDDS );
		if ( pTextureDDS == NULL )
		{
			char pDetails[512];
			strcpy ( pDetails, "pTextureDDS==NULL" );
			MessageBox( NULL, "Failed To Create DDS Texture", pDetails, 0 );
			exit(1);
		}
	}

	// free work texture
	SAFE_RELEASE(pTexture);
	#else
	HRESULT hRes = D3DXCreateTexture ( m_pD3D, iSizeU, iSizeV, 1, 0, GGFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTexture );
	if ( pTexture==NULL )
	{
		char wserr[512];
		printf ( wserr, "HRES=%d", (int)hRes );
		if ( hRes==D3DERR_INVALIDCALL ) strcpy ( wserr, "D3DERR_INVALIDCALL" );
		if ( hRes==D3DERR_NOTAVAILABLE ) strcpy ( wserr, "D3DERR_NOTAVAILABLE" );
		if ( hRes==D3DERR_OUTOFVIDEOMEMORY ) strcpy ( wserr, "D3DERR_OUTOFVIDEOMEMORY" );
		if ( hRes==E_OUTOFMEMORY ) strcpy ( wserr, "E_OUTOFMEMORY" );
		char ws[512];
		printf ( ws, "Error Creating Light Map Texture : %s", wserr );
		MessageBox( NULL, ws, "Error", 0 ); exit(1);
	}

	GGLOCKED_RECT	d3dlr;
	D3DSURFACE_DESC desc;

	if ( FAILED ( pTexture->GetLevelDesc ( 0, &desc ) ) )
	{
		MessageBox( NULL, "Error Getting Texture Description", "Error", 0 ); exit(1);
	}

	if ( FAILED ( pTexture->LockRect ( 0, &d3dlr, 0, 0 ) ) )
	{
		MessageBox( NULL, "Error Locking Texture", "Error", 0 ); exit(1);
	}

	pPixels = ( DWORD* ) d3dlr.pBits;
	iPitch  = d3dlr.Pitch / 4;
	iWidth  = desc.Width;
	iHeight = desc.Height;

	DWORD dwIndex;
	for ( int v = 0; v < iSizeV; v++ )
	{
		for ( int u = 0; u < iSizeU; u++ )
		{
			dwIndex = u + ( v * iPitch );
			*( pPixels + dwIndex ) = ppTexLumel[u][v].GetColor( );
		}
	}
	pPixels = NULL;
	pTexture->UnlockRect ( NULL );

	if ( ppTexLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppTexLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppTexLumel );
	}
	ppTexLumel = 0;

	if ( pNodeSpace ) delete pNodeSpace;
	pNodeSpace = 0;

	D3DXCreateTexture ( m_pD3D, iSizeU, iSizeV, 1, 0, GGFMT_DXT1, D3DPOOL_MANAGED, &pTextureDDS );
	if ( pTextureDDS==NULL )
	{
		char pDetails[512];
		strcpy ( pDetails, "pTextureDDS==NULL" );
		MessageBox( NULL, "Failed To Create DDS Texture", pDetails, 0 );
		exit(1);
	}

	LPGGSURFACE pDDSSurface;
	pTextureDDS->GetSurfaceLevel ( 0, &pDDSSurface );
	if ( pDDSSurface )
	{
		LPGGSURFACE pMasterSurface;
		pTexture->GetSurfaceLevel ( 0, &pMasterSurface );
		if ( pMasterSurface )
		{
			D3DXLoadSurfaceFromSurface ( pDDSSurface, NULL, NULL, pMasterSurface, NULL, NULL, D3DX_DEFAULT, 0 );
			pMasterSurface->Release( );
		}
		pDDSSurface->Release( );
	}

	pTexture->Release( );
	pTexture = NULL;
	#endif
}

void LMTexture::SaveTexture( char* pNewFilename )
{
	#ifdef DX11
	// save managed texture to file
	D3DX11_IMAGE_FILE_FORMAT DestFormat = D3DX11_IFF_BMP;
	switch( g_iLightmapFileFormat )
	{
		case 0: DestFormat = D3DX11_IFF_PNG; break;
		case 1: DestFormat = D3DX11_IFF_DDS; break;
		case 2: DestFormat = D3DX11_IFF_BMP; break;
		default: DestFormat = D3DX11_IFF_PNG; break;
	}
	strcpy_s( pFilename, 255, pNewFilename );
	HRESULT hRes = D3DX11SaveTextureToFile( m_pImmediateContext, pTextureDDS, DestFormat, pFilename );
	if ( FAILED ( hRes ) )
	{
		char pDetails[512];
		strcpy ( pDetails, "Failed To Save DDS Texture" );
		MessageBox( NULL, "Save DDS Error", pDetails, 0 );
		exit(1);
		return;
	}

	/* not needed now, lightmapper terminated at end
	// and to ensure system memory freed, recreate pTextureDDS as a video memory texture
	bool bCopiedToVideoMemorySuccessfully = false;
	LPGGTEXTURE pOldDDSTexture = pTextureDDS;
	LPGGTEXTURE pNewVideoDDS = NULL;
	D3DXCreateTexture ( m_pD3D, iSizeU, iSizeV, 1, 0, GGFMT_DXT1, D3DPOOL_DEFAULT, &pNewVideoDDS );
	if ( pNewVideoDDS )
	{
		LPGGSURFACE pDstSurface;
		pNewVideoDDS->GetSurfaceLevel ( 0, &pDstSurface );
		if ( pDstSurface )
		{
			LPGGSURFACE pSrcSurface;
			pTextureDDS->GetSurfaceLevel ( 0, &pSrcSurface );
			if ( pSrcSurface )
			{
				HRESULT hRes = D3DXLoadSurfaceFromSurface ( pDstSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0 );
				if ( hRes==S_OK ) bCopiedToVideoMemorySuccessfully = true;
				pSrcSurface->Release( );
			}
			pDstSurface->Release( );
		}
	}
	if ( bCopiedToVideoMemorySuccessfully==true )
	{
		// replace system memory texture with video memory texture, then remove system memory texture
		pTextureDDS = pNewVideoDDS;
		SAFE_RELEASE ( pOldDDSTexture );
	}
	*/
	#else
	// save managed texture to file
	strcpy_s( pFilename, 255, pNewFilename );
	switch( g_iLightmapFileFormat )
	{
		case 0: D3DXSaveTextureToFile( pFilename, D3DXIFF_PNG, pTextureDDS, NULL ); break;
		case 1: D3DXSaveTextureToFile( pFilename, D3DXIFF_DDS, pTextureDDS, NULL ); break;
		case 2: D3DXSaveTextureToFile( pFilename, D3DXIFF_BMP, pTextureDDS, NULL ); break;
		default: D3DXSaveTextureToFile( pFilename, D3DXIFF_PNG, pTextureDDS, NULL ); break;
	}

	// and to ensure system memory freed, recreate pTextureDDS as a video memory texture
	bool bCopiedToVideoMemorySuccessfully = false;
	LPGGTEXTURE pOldDDSTexture = pTextureDDS;
	LPGGTEXTURE pNewVideoDDS = NULL;
	D3DXCreateTexture ( m_pD3D, iSizeU, iSizeV, 1, 0, GGFMT_DXT1, D3DPOOL_DEFAULT, &pNewVideoDDS );
	if ( pNewVideoDDS )
	{
		LPGGSURFACE pDstSurface;
		pNewVideoDDS->GetSurfaceLevel ( 0, &pDstSurface );
		if ( pDstSurface )
		{
			LPGGSURFACE pSrcSurface;
			pTextureDDS->GetSurfaceLevel ( 0, &pSrcSurface );
			if ( pSrcSurface )
			{
				HRESULT hRes = D3DXLoadSurfaceFromSurface ( pDstSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0 );
				if ( hRes==S_OK ) bCopiedToVideoMemorySuccessfully = true;
				pSrcSurface->Release( );
			}
			pDstSurface->Release( );
		}
	}
	if ( bCopiedToVideoMemorySuccessfully==true )
	{
		// replace system memory texture with video memory texture, then remove system memory texture
		pTextureDDS = pNewVideoDDS;
		SAFE_RELEASE ( pOldDDSTexture );
	}
	#endif
}

IGGTexture* LMTexture::GetDXTextureRef( )
{
	return pTextureDDS;
}

char* LMTexture::GetFilename( )
{
	return pFilename;
}

/*
unsigned int LMTexture::Run( )
{
	if ( !pLocalLightList || ! pLocalColTree ) return 1;

	CalculateLight( pLocalLightList, pLocalColTree );

	return 0;
}
*/
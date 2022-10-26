//----------------------------------------------------
//--- GAMEGURU - Common-Images
//----------------------------------------------------

// Includes
#include "stdafx.h"
#include "gameguru.h"

// Externals
extern bool g_bSkipTerrainRender;

// 
//  Common Code - Image and Effect Functions
// 

void deleteallinternalimages ( void )
{
	int imgid = 0;
	int texid = 0;
	int tt = 0;
	if (  g.imagebankmax>0 ) 
	{
		for ( tt = 1 ; tt <= g.imagebankmax; tt++ )
		{
			imgid=g.imagebankoffset+tt;
			if (  ImageExist(imgid) == 1 ) 
			{
				DeleteImage (  imgid );
			}
		}
		g.imagebankmax=0;
		Undim (  t.imagebank_s );
		Dim (  t.imagebank_s,g.imagebankmax  );
	}
	if (  g.texturebankmax>0 ) 
	{
		for ( tt = 1 ; tt <=  g.texturebankmax; tt++ )
		{
			texid=g.texturebankoffset+tt;
			if (  ImageExist(texid) == 1 ) 
			{
				DeleteImage (  texid );
			}
		}
		g.texturebankmax=0;
		UnDim (  t.texturebank_s  );
		Dim (  t.texturebank_s,g.texturebankmax  );
	}
//endfunction

}

void refreshallinternalimages ( void )
{
	cstr tfile_s =  "";
	int imgid = 0;
	int tt;
	if (  g.imagebankmax>0 ) 
	{
		for ( tt = 1 ; tt <=  g.imagebankmax; tt++ )
		{
			tfile_s=t.imagebank_s[tt];
			imgid=g.imagebankoffset+tt;
			loadinternalimageexcompressquality(tfile_s.Get(),imgid,1,0,0);
		}
	}
}

void refreshinternalimage ( int tt )
{
	cstr tfile_s =  "";
	int imgid = 0;
	tfile_s=t.imagebank_s[tt];
	imgid=g.imagebankoffset+tt;
	loadinternalimageexcompressquality(tfile_s.Get(),imgid,1,0,0);
}

void removeinternalimage ( int img )
{
	int imagebankindex = 0;
	//  will remove image and entry in image bank
	if (  img>0 ) 
	{
		imagebankindex=img-g.imagebankoffset;
		if (  ImageExist(img) == 1  )  DeleteImage (  img );
		if (  imagebankindex >= 0 && imagebankindex <= ArrayCount(t.imagebank_s ) )
		{
			t.imagebank_s[imagebankindex]="";
		}
	}
}

void loadinternalimageexcompressquality ( char* tfile_s, int imgid, int compressmode, int quality, int iLoaderMode )
{
	cstr tryfile_s =  "";
	int tstarttry = 1;
	int ttry = 0;
	// 110917 - always prefer DDS if sits alongside PNG or JPG
	// if ( strcmp ( Lower(Right(tfile_s,4)) , ".jpg" ) == 0  )  tstarttry = 2; else tstarttry = 1;
	// if ( strcmp ( Lower(Right(tfile_s,4)) , ".png" ) == 0  )  tstarttry = 2; else tstarttry = 1;
	for ( ttry = tstarttry; ttry <= 4; ttry++ )
	{
		if ( ttry == 1  ) { tryfile_s = Left(tfile_s,Len(tfile_s)-4); tryfile_s += ".dds"; }
		if ( ttry == 2  ) { tryfile_s = tfile_s; }
		if ( ttry == 3  ) { tryfile_s = Left(tfile_s,Len(tfile_s)-4); tryfile_s += ".png"; }
		if ( ttry == 4  ) { tryfile_s = Left(tfile_s,Len(tfile_s)-3); tryfile_s += g.imgext_s.Get(); } //PE: tga
		if ( GetImageFileExist(tryfile_s.Get()) == 1 ) 
		{
			if ( g.gincludeonlyvideo == 1 && cstr(Left(Lower(tryfile_s.Get()),Len(g.gincludeonlyname_s.Get()))) != cstr(Lower(g.gincludeonlyname_s.Get())) ) 
			{
				// fill video with basic random color
				CreateBitmap ( 7,4,4 );
				CLS ( Rgb(Rnd(255),Rnd(255),Rnd(255)) );
				GrabImage ( imgid,0,0,4,4 );
				DeleteBitmapEx ( 7 );
				SetCurrentBitmap ( 0 );
			}
			else
			{
				// LoadImage (skip if building = quicker build)
				if ( quality == 0 ) 
				{
					if ( iLoaderMode == 2 || iLoaderMode == 3 )
					{
						// loading Cube Map or Volume textures
						LoadImage ( tryfile_s.Get(), imgid, iLoaderMode );
					}
					else
					{
						LoadImage ( tryfile_s.Get(),imgid,quality,g.gdividetexturesize );
					}
				}
				else
				{
					if ( quality == 1 ) 
					{
						LoadImage ( tryfile_s.Get(),imgid,quality,g.gdividetexturesize );
					}
					else
					{
						// Quality Mode 2 means only reduce have as much (smaller items blur too much in QM=1)
						LoadImage ( tryfile_s.Get(),imgid,1,g.gdividetexturesize/2 );
					}
				}
			}
			break;
		}
	}
}

void loadinternalimageexcompress ( char* tfile_s, int imgid, int compressmode )
{
	loadinternalimageexcompressquality(tfile_s,imgid,compressmode,0,0);
}

void loadinternalimageex ( char* tfile_s, int imgid )
{
	loadinternalimageexcompress(tfile_s,imgid,1);
}

int loadinternalimagecompressquality ( char* tfile_s, int compressmode, int quality )
{
	int toldimagebankmax = 0;
	int tuseindex = 0;
	int imgid = 0;
	int tt = 0;

	//  Default return
	imgid=0;

	//  Scan for existing
	if (  g.imagebankmax>0 ) 
	{
		for ( tt = 1 ; tt<=  g.imagebankmax; tt++ )
		{
			if (  strcmp ( tfile_s , t.imagebank_s[tt].Get() ) == 0 ) { imgid = g.imagebankoffset+tt  ; break; }
		}
	}
	else
	{
		tt=g.imagebankmax+1;
	}

	//  Did not find, load it
	if (  tt>g.imagebankmax ) 
	{
		//  first see if we have any free slots
		toldimagebankmax=g.imagebankmax;
		tuseindex=-1;
		for ( tt = 1 ; tt<=  g.imagebankmax; tt++ )
		{
			if (  t.imagebank_s[tt] == ""  )  tuseindex = tt;
		}
		if (  tuseindex == -1 ) 
		{
			++g.imagebankmax;
			Dim (  t.imagebank_s,g.imagebankmax  );
			tuseindex=g.imagebankmax;
		}
		imgid=g.imagebankoffset+tuseindex;
		loadinternalimageexcompressquality(tfile_s,imgid,compressmode,quality,0);
		if (  ImageExist(imgid) == 1 ) 
		{
			t.imagebank_s[tuseindex]=tfile_s;
		}
		else
		{
			g.imagebankmax=toldimagebankmax;
			imgid=0;
		}
	}
	return imgid;
}

int loadinternalimagecompress ( char* tfile_s, int compressmode )
{
	int imgid = 0;
	imgid=loadinternalimagecompressquality(tfile_s,compressmode,0);
	return imgid;
}

int loadinternalimage ( char* tfile_s )
{
	int imgid = 0;
	imgid=loadinternalimagecompress(tfile_s,1);
	return imgid;
}

int loadinternalimagequality ( char* tfile_s )
{
	int imgid = 0;
	imgid=loadinternalimagecompressquality(tfile_s,1,1);
	return imgid;
}

int loadinternaltexturemode ( char* tfile_s, int iImageLoaderMode )
{
	int toldtexturebankmax = 0;
	int tuseindex = 0;
	int texid = 0;
	int tt = 0;

	//  Default return
	texid=0;

	//  Scan for existing
	if (  g.texturebankmax>0 ) 
	{
		for ( tt = 1 ; tt<=  g.texturebankmax; tt++ )
		{
			if (  strcmp ( tfile_s , t.texturebank_s[tt].Get() ) == 0 ) { texid = g.texturebankoffset+tt  ; break; }
		}
	}
	else
	{
		tt=g.texturebankmax+1;
	}

	//  Did not find, load it
	if (  tt>g.texturebankmax ) 
	{
		toldtexturebankmax=g.texturebankmax;
		tuseindex=-1;
		for ( tt = 1 ; tt<=  g.texturebankmax; tt++ )
		{
			if (  t.texturebank_s[tt] == "" ) { tuseindex = tt ; break; }
		}
		//  Attempt to load
		if (  tuseindex == -1 ) 
		{
			++g.texturebankmax;
			Dim (  t.texturebank_s,g.texturebankmax  );
			tuseindex=g.texturebankmax;
		}
		t.texturebank_s[tuseindex]=tfile_s;
		texid=g.texturebankoffset+tuseindex;
		loadinternalimageexcompressquality(tfile_s,texid,1,0,iImageLoaderMode);
		if (  ImageExist(texid) == 0 ) 
		{
			//  failed to load, so reverse creation
			t.texturebank_s[tuseindex]="";
			g.texturebankmax=toldtexturebankmax;
			texid=0;
		}
	}
	return texid;
}

//PE: Check if file is in internal list.
int findinternaltexture(char* tfile_s)
{
	int texid = 0;
	int tt = 0;

	//  Default return
	texid = 0;

	//  Scan for existing
	if (g.texturebankmax>0)
	{
		for (tt = 1; tt <= g.texturebankmax; tt++)
		{
			if (strcmp(tfile_s, t.texturebank_s[tt].Get()) == 0) { texid = g.texturebankoffset + tt; break; }
		}
	}
	return(texid);

}
//PE: Just add path to the list , so it can be reused.
int addinternaltexture(char* tfile_s)
{
	int toldtexturebankmax = 0;
	int tuseindex = 0;
	int texid = 0;
	int tt = 0;

	//  Default return
	texid = 0;

	//  Scan for existing
	if (g.texturebankmax>0)
	{
		for (tt = 1; tt <= g.texturebankmax; tt++)
		{
			if (strcmp(tfile_s, t.texturebank_s[tt].Get()) == 0) { texid = g.texturebankoffset + tt; break; }
		}
	}
	else
	{
		tt = g.texturebankmax + 1;
	}

	//  Did not find, create new.
	if (tt>g.texturebankmax)
	{
		toldtexturebankmax = g.texturebankmax;
		tuseindex = -1;
		for (tt = 1; tt <= g.texturebankmax; tt++)
		{
			if (t.texturebank_s[tt] == "") { tuseindex = tt; break; }
		}
		if (tuseindex == -1)
		{
			++g.texturebankmax;
			Dim(t.texturebank_s, g.texturebankmax);
			tuseindex = g.texturebankmax;
		}
		t.texturebank_s[tuseindex] = tfile_s;
		texid = g.texturebankoffset + tuseindex;
	}
	return texid;
}

int loadinternaltexture ( char* tfile_s )
{
	return loadinternaltexturemode ( tfile_s, 0 );
}

void removeinternaltexture ( int teximg )
{
	int tt = 0;

	//  will remove image and entry in image bank
	if (  teximg>0 ) 
	{
		tt=teximg-g.texturebankoffset;
		if (  ImageExist(teximg) == 1  )  DeleteImage (  teximg );
		if (  tt >= 0 && tt <= ArrayCount( t.texturebank_s ) )
		{
			t.texturebank_s[tt]="";
		}
	}
}

void deleteinternaltexture(char* tfile_s)
{
	int texid = 0;
	int tt = 0;

	if (g.texturebankmax>0)
	{
		for (tt = 1; tt <= g.texturebankmax; tt++)
		{
			if (strcmp(tfile_s, t.texturebank_s[tt].Get()) == 0) { 
				texid = g.texturebankoffset + tt;
				if (ImageExist(texid) == 1)  DeleteImage(texid);
				t.texturebank_s[tt] = "";
				break;
			}
		}
	}
}

int loadinternaltextureex ( char* tfile_s, int compressmode, int quality )
{
	// vars
	int toldtexturebankmax = 0;
	int tuseindex = 0;
	int texid = 0;
	int tt = 0;

	// Scan for existing
	if ( g.texturebankmax>0 ) 
	{
		for ( tt = 1 ; tt <=  g.texturebankmax; tt++ )
		{
			//PE: Sometimes we get double load , due to upper/lower case so stricmp.
			if ( stricmp ( tfile_s , t.texturebank_s[tt].Get() ) == 0 ) { texid = g.texturebankoffset+tt  ; break; }
		}
	}
	else
	{
		tt=g.texturebankmax+1;
	}

	//  Did not find, load it
	if ( tt>g.texturebankmax ) 
	{
		toldtexturebankmax=g.texturebankmax;
		tuseindex=-1;
		for ( tt = 1 ; tt<=  g.texturebankmax; tt++ )
		{
			if ( t.texturebank_s[tt] == "" ) {  tuseindex = tt  ; break; }
		}

		//  Attempt to load
		if (  tuseindex == -1 ) 
		{
			++g.texturebankmax;
			Dim (  t.texturebank_s,g.texturebankmax  );
			tuseindex=g.texturebankmax;
		}

		//  Attempt to load
		t.texturebank_s[tuseindex]=tfile_s;
		texid=g.texturebankoffset+tuseindex;
		//PE: Sometimes tfile_s is changed inside loadinternalimageexcompressquality
		//PE: This means that t.texturebank_s[tuseindex] is not always the same as the actually loaded file.
		//PE: Also means that sometimes we get a doubble load.
		//PE: Keep it here for now , as this is rare. perhaps scan multiply extensions in findinternaltexture.
		loadinternalimageexcompressquality(tfile_s,texid,compressmode,quality,0);
		if (  ImageExist(texid) == 0 ) 
		{
			//  failed to load, so reverse creation
			t.texturebank_s[tuseindex]="";
			g.texturebankmax=toldtexturebankmax;
			texid=0;
		}
	}

	// debug log
	timestampactivity(0,cstr(cstr("loadinternaltextureex (")+Left(tfile_s,100)+", "+cstr(compressmode)+", "+cstr(quality)+")="+cstr(texid)).Get());

	// return value of new image
	return texid;
}

int loadinternaleffectunique ( char* tfile_s, int makeunique )
{
	int effectid = 0;
	int tt = 0;

	//  Default return
	effectid=0;

	//  Scan for existing
	if (  g.effectbankmax>0 && makeunique == 0 ) 
	{
		for ( tt = 1 ; tt<=  g.effectbankmax; tt++ )
		{
			if ( strcmp ( tfile_s , t.effectbank_s[tt].Get() ) == 0 ) { effectid = g.effectbankoffset+tt ; break; }
		}
	}
	else
	{
		tt=g.effectbankmax+1;
	}

	//  Did not find, load it
	if (  tt>g.effectbankmax ) 
	{
		if (  FileExist(tfile_s) == 1 ) 
		{
			++g.effectbankmax;
			Dim (  t.effectbank_s,g.effectbankmax  );
			effectid=g.effectbankoffset+g.effectbankmax;
			LoadEffect (  tfile_s,effectid,0 );
			if (  GetEffectExist(effectid) == 1 ) 
			{
				t.effectbank_s[g.effectbankmax]=tfile_s;
				filleffectparamarray(effectid);
			}
			else
			{
				//  could not use effect
				--g.effectbankmax;
				effectid=0;
			}
		}
	}
	return effectid;
}

void deleteinternaleffect ( int iEffectIndex )
{
	// Scan for existing
	int effectid = iEffectIndex - g.effectbankoffset;
	if ( effectid > 0 ) 
	{
		if ( strlen(t.effectbank_s[effectid].Get()) > 0 )
		{
			t.effectbank_s[effectid] = "";
			DeleteEffect ( iEffectIndex );
		}
	}
}

void filleffectparamarray ( int effectid )
{
	if (  ArrayCount(t.effectparamarray)<effectid ) 
	{
		Dim (  t.effectparamarray,effectid+32 );
	}
	t.effectparamarray[effectid].g_lights_data=GetEffectParameterIndex(effectid,"g_lights_data");
	t.effectparamarray[effectid].g_lights_pos0=GetEffectParameterIndex(effectid,"g_lights_pos0");
	t.effectparamarray[effectid].g_lights_atten0=GetEffectParameterIndex(effectid,"g_lights_atten0");
	t.effectparamarray[effectid].g_lights_diffuse0=GetEffectParameterIndex(effectid,"g_lights_diffuse0");
	t.effectparamarray[effectid].g_lights_pos1=GetEffectParameterIndex(effectid,"g_lights_pos1");
	t.effectparamarray[effectid].g_lights_atten1=GetEffectParameterIndex(effectid,"g_lights_atten1");
	t.effectparamarray[effectid].g_lights_diffuse1=GetEffectParameterIndex(effectid,"g_lights_diffuse1");
	t.effectparamarray[effectid].g_lights_pos2=GetEffectParameterIndex(effectid,"g_lights_pos2");
	t.effectparamarray[effectid].g_lights_atten2=GetEffectParameterIndex(effectid,"g_lights_atten2");
	t.effectparamarray[effectid].g_lights_diffuse2=GetEffectParameterIndex(effectid,"g_lights_diffuse2");
	t.effectparamarray[effectid].SpotFlashPos=GetEffectParameterIndex(effectid,"SpotFlashPos");
	t.effectparamarray[effectid].SpotFlashColor=GetEffectParameterIndex(effectid,"SpotFlashColor");
}

int loadinternaleffect ( char* tfile_s )
{
	int effectid = loadinternaleffectunique(tfile_s,0);
	return effectid;
}

//
// Env/Cube Map Generation
//

#ifdef DX11

float fffFOV = PI/2.0f;

void cubemap_buildviews ( int iImageID, int iCubeMapSize, float fX, float fY, float fZ ) 
{
    // create the render target cube map texture
	if ( 1 )
	{
		// save RTV and DSV and VP for later restoration
		UINT savedViewportCount = 1;
		D3D11_VIEWPORT savedViewport;
		ID3D11RenderTargetView* savedRTV;
		ID3D11DepthStencilView* savedDSV;
		m_pImmediateContext->RSGetViewports(&savedViewportCount, &savedViewport);
		m_pImmediateContext->OMGetRenderTargets ( 1, &savedRTV, &savedDSV );

		// store cameras viewport3D size
		tagCameraData* pCamPtr = (tagCameraData*)GetCameraInternalData ( 0 );
		GGVIEWPORT storedViewPort3D = pCamPtr->viewPort3D;

		// needed for later mipmap generation
		ID3D11ShaderResourceView* _dynamicCubeMapSRV = NULL;

		// also save system world, view and projection matrices
		GGMATRIX savedWorld, savedView, savedProjection;
		GGGetTransform(GGTS_WORLD,&savedWorld);
		GGGetTransform(GGTS_VIEW,&savedView);
		GGGetTransform(GGTS_PROJECTION,&savedProjection);

		// create dynamic cube map (render target)
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory( &texDesc, sizeof(texDesc) );
		texDesc.Width = iCubeMapSize;
		texDesc.Height = iCubeMapSize;
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 6;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;    
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
		ID3D11Texture2D* cubeTex = NULL;
		HRESULT hr = m_pD3D->CreateTexture2D( &texDesc, NULL, &cubeTex );
		if( FAILED( hr ) )
		{
			#ifdef VRTECH
			Error1 ( "Failed to CreateTexture2D\n" );
			#endif
			return;
		}

		// create the render target view array
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.MipSlice = 0;
		ID3D11RenderTargetView* _dynamicCubeMapRTV[6];
		for (int i = 0; i < 6; i++) 
		{
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			hr = m_pD3D->CreateRenderTargetView ( cubeTex, &rtvDesc, &_dynamicCubeMapRTV[i] );
			if( FAILED( hr ) )
			{
				#ifdef VRTECH
				Error1 ( "Failed to CreateRenderTargetView\n" );
				#endif
				return;
			}
		}

		// Create the shader resource view that we will bind to our effect for the cubemap
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = -1;
		hr = m_pD3D->CreateShaderResourceView ( cubeTex, &srvDesc, &_dynamicCubeMapSRV );

		// create the depth/stencil texture
		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory( &depthTexDesc, sizeof(depthTexDesc) );
		depthTexDesc.Width = iCubeMapSize;
		depthTexDesc.Height = iCubeMapSize;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;//D32_FLOAT;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;    
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;
		ID3D11Texture2D* depthTex = NULL;
		hr = m_pD3D->CreateTexture2D( &depthTexDesc, NULL, &depthTex );
		if( FAILED( hr ) )
		{
			#ifdef VRTECH
			Error1 ( "Failed to CreateTexture2D\n" );
			#endif
			return;
		}
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory( &dsvDesc, sizeof(dsvDesc) );
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//depthTexDesc.Format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		ID3D11DepthStencilView* _dynamicCubeMapDSV = NULL;
		hr = m_pD3D->CreateDepthStencilView( depthTex, &dsvDesc, &_dynamicCubeMapDSV );
		if( FAILED( hr ) )
		{
			#ifdef VRTECH
			Error1 ( "Failed to CreateDepthStencilView\n" );
			#endif
			return;
		}

		// create the viewport for rendering the cubemap faces
		D3D11_VIEWPORT _cubeMapViewPort;
		_cubeMapViewPort.Width = iCubeMapSize;
		_cubeMapViewPort.Height = iCubeMapSize;
		_cubeMapViewPort.MinDepth = 0.0f;
		_cubeMapViewPort.MaxDepth = 1.0f;
		_cubeMapViewPort.TopLeftX = 0;
		_cubeMapViewPort.TopLeftY = 0;

		// set the viewport to the cube map viewport area
		m_pImmediateContext->RSSetViewports(1, &_cubeMapViewPort);

		// create temp camera to carry terrain perspective and clear any old terrain stack instructions
		int iCubeRenderTempCamera = 30;
		g_bSkipTerrainRender = true;
		BT_Intern_Render();
		CreateCamera ( iCubeRenderTempCamera );
		SetCurrentCamera ( 0 );

		// for each cube face, render contents directly to cube map
		GGVECTOR3 center = GGVECTOR3(fX,fY,fZ);
		GGVECTOR3 targets = GGVECTOR3(fX,fY,fZ);
		GGVECTOR3 ups = GGVECTOR3(0,1,0);
		for ( int i = 0; i < 6; i++ ) 
		{
			// build cube faces (view and projection matrices)
			if ( i == 0 ) { targets = GGVECTOR3(fX+1,fY,fZ); ups = GGVECTOR3(0,1,0); }
			if ( i == 1 ) { targets = GGVECTOR3(fX-1,fY,fZ); ups = GGVECTOR3(0,1,0); }
			if ( i == 2 ) { targets = GGVECTOR3(fX,fY+1,fZ); ups = GGVECTOR3(0,0,-1); }
			if ( i == 3 ) { targets = GGVECTOR3(fX,fY-1,fZ); ups = GGVECTOR3(0,0,1); }
			if ( i == 4 ) { targets = GGVECTOR3(fX,fY,fZ+1); ups = GGVECTOR3(0,1,0); }
			if ( i == 5 ) { targets = GGVECTOR3(fX,fY,fZ-1); ups = GGVECTOR3(0,1,0); }
			GGMATRIX cubeCameraView;
			GGMatrixLookAtLH ( &cubeCameraView, &center, &targets, &ups );
			GGSetTransform(GGTS_VIEW,&cubeCameraView);
			GGMATRIX cubeCameraProjection;
			GGMatrixPerspectiveFovLH ( &cubeCameraProjection, GGToRadian(90.0f), 1.0f, 0.0f, 3000.0f );
			GGSetTransform(GGTS_PROJECTION,&cubeCameraProjection);

			// clear render target and depth buffer
			float ClearColor[4] = {1,0,0,1};
			if ( i == 1 ) { ClearColor[0] = 0; ClearColor[1] = 0; ClearColor[2] = 0; }
			if ( i == 2 ) { ClearColor[0] = 0; ClearColor[1] = 1; ClearColor[2] = 0; }
			if ( i == 3 ) { ClearColor[0] = 0; ClearColor[1] = 0; ClearColor[2] = 1; }
			if ( i == 4 ) { ClearColor[0] = 1; ClearColor[1] = 1; ClearColor[2] = 0; }
			if ( i == 5 ) { ClearColor[0] = 0; ClearColor[1] = 1; ClearColor[2] = 1; }
			m_pImmediateContext->ClearRenderTargetView(_dynamicCubeMapRTV[i], ClearColor);
			m_pImmediateContext->ClearDepthStencilView(_dynamicCubeMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

			// set render target/depth
			m_pImmediateContext->OMSetRenderTargets ( 1, &_dynamicCubeMapRTV[i], _dynamicCubeMapDSV );

			// move sky to center of cube map so can get proper sky render
			if ( ObjectExist ( t.terrain.objectstartindex+4 ) == 1 ) 
			{
				PositionObject ( t.terrain.objectstartindex+4, fX, fY, fZ );
				AddObjectMask ( t.terrain.objectstartindex+4, 1<<iCubeRenderTempCamera );
			}
			if ( ObjectExist ( t.terrain.objectstartindex+8 ) == 1 ) 
			{
				PositionObject ( t.terrain.objectstartindex+8, fX, fY, fZ );
				AddObjectMask ( t.terrain.objectstartindex+8, 1<<iCubeRenderTempCamera );
			}

			// select all in-game entities for cube rendering
			// NOTE: For some reason depth Z buffer sorting not working (do not need this for now)
			//for ( int iObj = g.entityviewstartobj; iObj <= g.entityviewendobj; iObj++ )
			//	if ( ObjectExist ( iObj ) == 1 )
			//		AddObjectMask ( iObj, 1<<iCubeRenderTempCamera );

			// draw all geometry into render target (floor and sky)
			pCamPtr = (tagCameraData*)GetCameraInternalData ( iCubeRenderTempCamera );
			pCamPtr->matView = cubeCameraView;
			pCamPtr->matProjection = cubeCameraProjection;
			pCamPtr->viewPort3D.X = _cubeMapViewPort.TopLeftX;
			pCamPtr->viewPort3D.Y = _cubeMapViewPort.TopLeftY;
			pCamPtr->viewPort3D.Width = _cubeMapViewPort.Width;
			pCamPtr->viewPort3D.Height = _cubeMapViewPort.Height;
			pCamPtr->viewPort3D.MinZ = _cubeMapViewPort.MinDepth;
			pCamPtr->viewPort3D.MaxZ = _cubeMapViewPort.MaxDepth;
			g_pGlob->dwRenderCameraID = iCubeRenderTempCamera;
			SetCameraFOV ( iCubeRenderTempCamera, 90 );
			SetCameraAspect ( iCubeRenderTempCamera, 1 );
			m_ObjectManager.UpdateInitOnce ( );
			m_ObjectManager.Update ( );

			// render the terrain (cheapest terrain render, camera 30 is temp camera)
			if ( t.terrain.TerrainID > 0 )
			{
				BT_SetCurrentCamera ( iCubeRenderTempCamera );
				BT_SetTerrainLODDistance ( t.terrain.TerrainID,1,700.0 );
				BT_SetTerrainLODDistance ( t.terrain.TerrainID,2,701.0 );
				BT_UpdateTerrainLOD ( t.terrain.TerrainID );
				BT_UpdateTerrainCull ( t.terrain.TerrainID );
				BT_RenderTerrain ( t.terrain.TerrainID );
				BT_Intern_Render();
			}
		}

		// delete temp camera
		DeleteCamera ( iCubeRenderTempCamera );
		SetCurrentCamera ( 0 );

		// restore LOD to terrain render sequence
		BT_SetCurrentCamera ( 0 );
		if ( t.terrain.TerrainID > 0 )
		{
			BT_SetTerrainLODDistance ( t.terrain.TerrainID,1,1401.0+t.visuals.TerrainLOD1_f );
			BT_SetTerrainLODDistance ( t.terrain.TerrainID,2,1401.0+t.visuals.TerrainLOD2_f );
		}

		// free resources no longer needed (rendertargetviews, depth buffer, etc)
		for (int i = 0; i < 6; i++) 
		{
			SAFE_RELEASE ( _dynamicCubeMapRTV[i] );
		}
		SAFE_RELEASE ( _dynamicCubeMapDSV );
		SAFE_RELEASE ( depthTex );

		// restore viewport, render target and depth buffer
		m_pImmediateContext->RSSetViewports(1, &savedViewport);
		m_pImmediateContext->OMSetRenderTargets ( 1, &savedRTV, savedDSV );
		SAFE_RELEASE ( savedRTV );
		SAFE_RELEASE ( savedDSV );
		GGSetTransform(GGTS_WORLD,&savedWorld);
		GGSetTransform(GGTS_VIEW,&savedView);
		GGSetTransform(GGTS_PROJECTION,&savedProjection);

		// may want to move sky positions back to original (in case of glitches in real-time cube mapping)

		// now no longer linked to shader system, generate mipmaps for cube map
		if ( _dynamicCubeMapSRV ) m_pImmediateContext->GenerateMips ( _dynamicCubeMapSRV );

		// create/replace image previously stored with this one
		CreateReplaceImage ( iImageID, iCubeMapSize, cubeTex, _dynamicCubeMapSRV );

		// restore viewport3D after cube map generation
		pCamPtr = (tagCameraData*)GetCameraInternalData ( 0 );
		pCamPtr->viewPort3D = storedViewPort3D;
	}
}

void cubemap_generateimage ( int iImageID, float fX, float fY, float fZ, LPSTR pCacheCubeMapFile )
{
	// can only generate when in-game (not during init or editor modes)
	#ifdef VRTECH
	if ( t.game.set.ismapeditormode == 0 || t.visuals.refreshskysettingsfromlua )
	#else
	if ( t.game.set.ismapeditormode == 0 )
	#endif
	{
		// create render target cube texture and views
		cubemap_buildviews ( iImageID, 256, fX, fY, fZ );

		// save cube texture to a file
		if ( pCacheCubeMapFile )
		{
			// save cube map
			if ( FileExist ( pCacheCubeMapFile ) == 1 ) DeleteFileA ( pCacheCubeMapFile );
			SaveImage ( pCacheCubeMapFile, iImageID );
		}
	}
}

void cubemap_generateglobalenvmap ( void ) 
{
	#ifdef VRTECH
	if ( t.game.gameisexe == 0 || t.visuals.refreshskysettingsfromlua)
	#else
	if ( t.game.gameisexe == 0 )
	#endif
	{
		// until have dynamic cubes from light probes, use a corner of terrain to get good floor and sky simulation
		float fSampleAtX = 2000;
		float fSampleAtZ = 2000;
		float fSampleAtY = BT_GetGroundHeight ( t.terrain.TerrainID, fSampleAtX, fSampleAtZ ) + 100.0f;
		cstr cubemaptexture_s = g.mysystem.levelBankTestMap_s + "globalenvmap.dds";
		if (t.visuals.refreshskysettingsfromlua)
			cubemap_generateimage(t.terrain.imagestartindex + 31, fSampleAtX, fSampleAtY, fSampleAtZ, NULL);
		else
		cubemap_generateimage ( t.terrain.imagestartindex+31, fSampleAtX, fSampleAtY, fSampleAtZ, cubemaptexture_s.Get() );
		t.visuals.refreshskysettingsfromlua = false;
	}
}

#endif

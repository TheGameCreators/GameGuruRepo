#include "DepthTexture.h"

DepthTexture::DepthTexture(const LPGG d3d)
{
	m_pTexture = NULL;
	m_pTextureView = NULL;
	m_pTextureResourceView = NULL;
	m_iWidth = 0;
	m_iHeight = 0;
}

void DepthTexture::createTexture( LPGGDEVICE device, int width, int height )
{
	m_iWidth = width;
	m_iHeight = height;
	#ifdef DX11
	D3D11_TEXTURE2D_DESC StagedDesc = { width, height, 1, 1, DXGI_FORMAT_R32_TYPELESS, 1, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, 0, 0 };
	HRESULT hRes = device->CreateTexture2D( &StagedDesc, NULL, (ID3D11Texture2D**)&m_pTexture );
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hRes = device->CreateDepthStencilView( m_pTexture, &descDSV, &m_pTextureView );
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hRes = device->CreateShaderResourceView ( m_pTexture, &shaderResourceViewDesc, &m_pTextureResourceView );
	#else
	// Great paper on PCSS and how to use higher resolution target
	// http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf
	// THOUGH it would seem this technique relies on R32F to exist
	int iGoThroughBestToWorse = 0;
	bool bValidTargetNotFound = true;
	//while ( bValidTargetNotFound == true && iGoThroughBestToWorse < 4 )
	//{
		GGFORMAT GGFORMATChoice = GGFMT_R32F;
		//if ( iGoThroughBestToWorse == 0 ) GGFORMATChoice = GGFMT_R32F;
		//if ( iGoThroughBestToWorse == 1 ) GGFORMATChoice = GGFMT_R16F;
		//if ( iGoThroughBestToWorse == 2 ) GGFORMATChoice = GGFMT_A8R8G8B8;
		//if ( iGoThroughBestToWorse == 3 ) GGFORMATChoice = GGFMT_X8R8G8B8;
		//iGoThroughBestToWorse++;
		HRESULT hRes = device->CreateTexture(width,height,1,GGUSAGE_RENDERTARGET,GGFORMATChoice,D3DPOOL_DEFAULT,&m_pTexture, NULL);
		//if ( hRes == GG_OK )
		//{
		//	bValidTargetNotFound = false;
		//}
	//}
	#endif
}

DepthTexture::~DepthTexture()
{
	if ( m_pTexture && m_pTextureView )
	{
		#ifdef DX11
		m_pTextureResourceView->Release();
		m_pTextureView->Release();
		m_pTexture->Release();
		m_pTextureResourceView = NULL;
		m_pTextureView = NULL;
		#else
		m_pTexture->Release();
		#endif
		m_pTexture = NULL;
	}
}

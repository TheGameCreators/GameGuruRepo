#ifndef DEPTH_TEXTURE_H
#define DEPTH_TEXTURE_H

#include "preprocessor-flags.h"

#include "directx-macros.h"

//--------------------------------------------------------------------------------------
class DepthTexture
{
public:

	DepthTexture(const LPGG d3d);
	~DepthTexture();

	void			createTexture( LPGGDEVICE device, int width, int height );
	LPGGTEXTURE		getTexture ( void ) { return m_pTexture; }
	LPGGTEXTURE		m_pTexture;
	int				m_iWidth;
	int				m_iHeight;

	#ifdef DX11
	ID3D11DepthStencilView*		getTextureView ( void ) { return m_pTextureView; }
	ID3D11DepthStencilView*		m_pTextureView;
	ID3D11ShaderResourceView*	getTextureResourceView ( void ) { return m_pTextureResourceView; }
	ID3D11ShaderResourceView*	m_pTextureResourceView;
	#endif
};

#endif

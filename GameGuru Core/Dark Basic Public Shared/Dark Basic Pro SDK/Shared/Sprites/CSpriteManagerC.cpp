#include "cspritemanagerc.h"
#include <algorithm>
#include "CImageC.h"
#include "CGfxC.h"
#include "CCameraC.h"
#include ".\..\..\Shared\Objects\CommonC.h"
#include ".\..\..\Shared\camera\ccameradatac.h"

#undef DARKSDK
#define DARKSDK

// External Data from rest of code
extern LPGGDEVICE				m_pD3D;
extern LPGGIMMEDIATECONTEXT		m_pImmediateContext;
extern LPGGSURFACE				g_pBackBuffer;
extern int						g_iFilterMode;

GGVECTOR3						CSpriteManager::vecDisplayMin;
GGVECTOR3						CSpriteManager::vecDisplayMax;
std::vector<tagSpriteData*>     CSpriteManager::m_SpriteDrawList;
CSpriteManager::t_SpriteList    CSpriteManager::m_SortedSpriteList;

#ifdef DX11
struct CBPerFrame
{
	KMaths::Matrix mWorld;
	KMaths::Matrix mView;
	KMaths::Matrix mProjection;
};				
struct SpriteVertex
{
	GGVECTOR4 PositionRHW;
	DWORD dwColor;
	GGVECTOR2 UV;
};
ID3D11Buffer*					g_pCBPerFrame						= NULL;
ID3D11InputLayout*				g_pSpriteVertexDeclaration			= NULL;
ID3D11Buffer*					g_pSpriteVertexBuffer				= NULL;
#endif

struct CSpriteManager::PrioritiseSprite
{
    int m_id;
    tagSpriteData* m_ptr;

    PrioritiseSprite ( SpritePtr pItem ) 
        : m_id( pItem->first), m_ptr( pItem->second )
    {}

    PrioritiseSprite ( SpritePtrConst pItem ) 
        : m_id( pItem->first), m_ptr( pItem->second )
    {}

    bool operator < (const PrioritiseSprite& Other) const
    {
        // Sort by Priority
        if (m_ptr->iPriority < Other.m_ptr->iPriority)        return true;
        if (m_ptr->iPriority > Other.m_ptr->iPriority)        return false;
        
        // For matching priorities, sort by the image id
        // to try and avoid texture lookups and changes.
        if (m_ptr->iImage < Other.m_ptr->iImage)              return true;
        if (m_ptr->iImage > Other.m_ptr->iImage)              return false;

        // For matching image ids, sort by transparency
        // to try and avoid transparency mode changes.
        if (m_ptr->bTransparent < Other.m_ptr->bTransparent)  return true;
        if (m_ptr->bTransparent > Other.m_ptr->bTransparent)  return false;

        // Where everything else matches, sort by the sprite id
        // to give a unique order to all sprites
        return (m_id < Other.m_id);
    }
};



////////////////////////////
// Class member functions //
////////////////////////////



CSpriteManager::CSpriteManager ( void )
{
    m_CurrentId = 0;
    m_CurrentPtr = 0;
    m_FilterMode = 0;
	m_TempDisableDraw = false;
	m_bSpriteBatcherActive = false;
}


CSpriteManager::~CSpriteManager ( void )
{
}


bool CSpriteManager::Add ( tagSpriteData* pData, int iID )
{
    tagSpriteData* ptr = 0;

    SpritePtr p = m_List.find( iID );
    if (p == m_List.end())
    {
        // If doesn't exist, then create one and add to the list
	    ptr = new tagSpriteData;
        m_List.insert( std::make_pair(iID, ptr) );
    }
    else
    {
        ptr = p->second;
    }

    // Update the sprite data
    *ptr = *pData;

    m_CurrentId = iID;
    m_CurrentPtr = ptr;

	return false;
}


bool CSpriteManager::Delete ( int iID )
{
    SpritePtr p = m_List.find( iID );
    if (p == m_List.end())
		return false;

    DeleteJustOne( p->second );
    m_List.erase( p );
	
    if (m_CurrentId == iID)
    {
        m_CurrentId = 0;
        m_CurrentPtr = 0;
    }

	return true;
}

int g_iStoreSpriteBatchCount = 0;
int g_iStoreSpriteBatchMax = 10000;
tagSpriteData* g_pStoreSpriteBatch[10000];

void CSpriteManager::DrawImmediate ( tagSpriteData* pData )
{
	// determine if immediate or batcher in effect
	if ( m_bSpriteBatcherActive == true )
	{
		// store sprite data for later batch rendering
		if ( g_iStoreSpriteBatchCount < g_iStoreSpriteBatchMax )
		{
			g_pStoreSpriteBatch[g_iStoreSpriteBatchCount] = new tagSpriteData;
			memcpy ( g_pStoreSpriteBatch[g_iStoreSpriteBatchCount], pData, sizeof(tagSpriteData) );
			g_iStoreSpriteBatchCount++;
		}
	}
	else
	{
		// Build a render list containing just one sprite
		tagSpriteData* pSpriteList[1] = { pData };

		// Render it
		RenderDrawList ( pSpriteList, 1, m_FilterMode );
	}
}

void CSpriteManager::DrawBatchImmediate ( void )
{
	if ( g_iStoreSpriteBatchCount > 0 )
	{
		// draw all sprites added during the batching
		RenderDrawList ( g_pStoreSpriteBatch, g_iStoreSpriteBatchCount, m_FilterMode );

		// delete stored batch items and reset draw count index for next time
		for ( int iI = 0; iI < g_iStoreSpriteBatchCount; iI++ )
		{
			SAFE_DELETE ( g_pStoreSpriteBatch[iI] );
		}
		g_iStoreSpriteBatchCount = 0;
	}
}

tagSpriteData* CSpriteManager::GetData ( int iID ) const
{
    if (iID != m_CurrentId)
    {
        SpritePtrConst p = m_List.find( iID );
        if (p == m_List.end())
            return 0;

        m_CurrentId = iID;
        m_CurrentPtr = p->second;
    }
    return m_CurrentPtr;
}


int CSpriteManager::Update ( void ) const
{
    // Clear the old list and resize to hold up to the actual number of sprites.
    m_SortedSpriteList.clear();
    m_SortedSpriteList.reserve(m_List.size());

    // Update our copy of the display size, for culling purposes
    GetDisplaySize ();

	// 050416 - can disable sprite drawing for a while (for screen blanking)
	if ( m_TempDisableDraw == false )
	{
		// Build the sorted list of sprites
		// Dump all visible sprites into a vector
		for (SpritePtrConst pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
		{
			if (pCheck->second)
			{
				PrioritiseSprite s(pCheck);
				if (s.m_ptr->iImage > 0 && s.m_ptr->bVisible && IsSpriteInDisplay ( s.m_ptr ))
				{
					m_SortedSpriteList.push_back( s );
				}
			}
		}

		if ( ! m_SortedSpriteList.empty() )
		{
			// Sort the vector
			std::sort( m_SortedSpriteList.begin(), m_SortedSpriteList.end() );

			// Prepare the draw list
			m_SpriteDrawList.clear();
			m_SpriteDrawList.reserve( m_SortedSpriteList.size() );

			// Build the draw list from the sorted list
			t_SpriteListPtr Last = m_SortedSpriteList.end();
			for (t_SpriteListPtr Current = m_SortedSpriteList.begin(); Current != Last; ++Current)
			{
				m_SpriteDrawList.push_back( Current->m_ptr );
			}

			// Render the draw list
			RenderDrawList ( &m_SpriteDrawList[0], m_SpriteDrawList.size(), m_FilterMode );
		}
	}

    return m_SortedSpriteList.size();
}


void CSpriteManager::DeleteAll ( void )
{
    for (SpritePtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
	{
		tagSpriteData* ptr = pCheck->second;

	    // release the sprite
		if ( ptr )
		    DeleteJustOne(ptr);
    }

    m_List.clear();

    m_CurrentId = 0;
    m_CurrentPtr = 0;
}


void CSpriteManager::HideAll ( void )
{
    for (SpritePtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
	{
		tagSpriteData* ptr = pCheck->second;

		// if the data doesn't exist then skip it
		if ( ptr )
        {
		    // run through all sprites and set their visible property
		    ptr->bVisible = false;
        }
    }
}


void CSpriteManager::ShowAll ( void )
{
    for (SpritePtr pCheck = m_List.begin(); pCheck != m_List.end(); ++pCheck)
	{
		tagSpriteData* ptr = pCheck->second;

		// if the data doesn't exist then skip it
		if ( ptr )
        {
		    // run through all sprites and set their visible property
		    ptr->bVisible = true;
        }
    }
}

void CSpriteManager::DisableAll ( void )
{
	m_TempDisableDraw = true;
}

void CSpriteManager::EnableAll ( void )
{
	m_TempDisableDraw = false;
}



///////////////////////////////////
// Class static member functions //
///////////////////////////////////



void CSpriteManager::DeleteJustOne(tagSpriteData* ptr)
{
	// remove reference to image (do not delete image!)
	ptr->lpTexture = NULL;

	// free up the allocated memory block
	delete ptr;
}

void CSpriteManager::RenderDrawList ( tagSpriteData** pList, int iListSize, int iFilterMode )
{
    // Set up the rendering pipeline for all sprites
	#ifdef DX11
	DWORD dwFVFSize = sizeof(SpriteVertex);
	if ( g_pSpriteVertexDeclaration == NULL )
	{
		int iLayoutSize = 3;
		D3D11_INPUT_ELEMENT_DESC* pLayout = new D3D11_INPUT_ELEMENT_DESC [ iLayoutSize ];
		D3D11_INPUT_ELEMENT_DESC layout [ ] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32_UINT,					0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		std::memcpy ( pLayout, layout, sizeof ( layout ) );
		ID3DBlob* pBlob = g_sShaders[SHADERSSPRITEBASIC].pBlob;
		DWORD tIndex = 0;
		ID3DX11EffectTechnique* tech = g_sShaders[SHADERSSPRITEBASIC].pEffect->GetTechniqueByIndex(0);
		ID3DX11EffectPass* pass = tech->GetPassByIndex(0);
		D3DX11_PASS_SHADER_DESC vs_desc;
		pass->GetVertexShaderDesc(&vs_desc);
		D3DX11_EFFECT_SHADER_DESC s_desc;
		vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc);
		HRESULT hr = m_pD3D->CreateInputLayout ( pLayout, iLayoutSize, s_desc.pBytecode, s_desc.BytecodeLength, &g_pSpriteVertexDeclaration );
		SAFE_DELETE_ARRAY(pLayout);
	}

	// create constant buffer
	if ( g_pCBPerFrame == NULL )
	{
		D3D11_BUFFER_DESC bdPerFrameBuffer;
		std::memset ( &bdPerFrameBuffer, 0, sizeof ( bdPerFrameBuffer ) );
		bdPerFrameBuffer.Usage          = D3D11_USAGE_DEFAULT;
		bdPerFrameBuffer.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		bdPerFrameBuffer.CPUAccessFlags = 0;
		bdPerFrameBuffer.ByteWidth      = sizeof ( CBPerFrame );
		if ( FAILED ( m_pD3D->CreateBuffer ( &bdPerFrameBuffer, NULL, &g_pCBPerFrame ) ) )
			return;
	}

	// Store old transforms
	GGMATRIX matStoreWorld;
	GGMATRIX matStoreView;
	GGMATRIX matStoreProj;
	GGGetTransform(GGTS_WORLD,&matStoreWorld);
	GGGetTransform(GGTS_VIEW,&matStoreView);
	GGGetTransform(GGTS_PROJECTION,&matStoreProj);

	// Transforms
	GGMATRIX matWorld;
	GGMATRIX matView;
	GGMatrixIdentity ( &matWorld );
	GGMatrixIdentity ( &matView );
	GGSetTransform(GGTS_WORLD,&matWorld);
	GGSetTransform(GGTS_VIEW,&matView);

	// Camera zero and sprite projection matrix for GUI shader
	tagCameraData* Camera = (tagCameraData*)GetCameraInternalData(0);
	GGVIEWPORT* pvp = &Camera->viewPort2D;
	GGMATRIX matProjection;
	GGMatrixOrthoOffCenterLH ( &matProjection, 0, pvp->Width, pvp->Height, 0, 0.0f, 1.0f );
	GGSetTransform(GGTS_PROJECTION,&matProjection);

	// Viewport
    D3D11_VIEWPORT vp;
    vp.TopLeftX = pvp->X;
    vp.TopLeftY = pvp->Y;
    vp.Width = (FLOAT)pvp->Width;
    vp.Height = (FLOAT)pvp->Height;
    vp.MinDepth = pvp->MinZ;
    vp.MaxDepth = pvp->MaxZ;
	SetupSetViewport ( 3, &vp, NULL ); // fool system into thinking this is post process and applicable to stereo rendering

	// Vertex Declaration
	m_pImmediateContext->IASetInputLayout ( g_pSpriteVertexDeclaration );

	// Obtain technique handles
	LPGGEFFECT Effect = g_sShaders[SHADERSSPRITEBASIC].pEffect;
	GGTECHNIQUE hTechniqueUsed = Effect->GetTechniqueByIndex(0);
	if ( hTechniqueUsed->IsValid() )
	{
		// Move plenty of effect setup to here (performance)
		m_pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// state blocks for raster and depthstencil states
		m_pImmediateContext->RSSetState(m_pRasterStateNoCull);
		m_pImmediateContext->OMSetDepthStencilState( m_pDepthDisabledStencilState, 0 );

		// apply effect ready for rendering
		hTechniqueUsed->GetPassByIndex(0)->Apply(0,m_pImmediateContext);

		// default is all sprites are transparent
		bool bLastTransparent = true;
		m_pImmediateContext->OMSetBlendState(m_pBlendStateAlpha, 0, 0xffffffff);

		// display all sprites in the list
		for ( int iTemp = 0; iTemp < iListSize; iTemp++ )
		{
			// this sprite
			tagSpriteData* pCurrent = pList[iTemp];

			// toggle if sprite is not transparent (alpha-enabled)
			if (bLastTransparent != pCurrent->bTransparent)
			{
				bLastTransparent = pCurrent->bTransparent;
				if ( bLastTransparent == true )
					m_pImmediateContext->OMSetBlendState(m_pBlendStateAlpha, 0, 0xffffffff);
				else
					m_pImmediateContext->OMSetBlendState(m_pBlendStateNoAlpha, 0, 0xffffffff);
			}

			// Set texture (from image ID or direct from pTexture(videotexture))
			ID3D11ShaderResourceView* lpTexture = pCurrent->lpTexture;
			if ( pCurrent->iImage > 0 )
			{
				lpTexture = GetImagePointerView ( pCurrent->iImage );
			}
			m_pImmediateContext->PSSetShaderResources ( 0, 1, &lpTexture );

			// calculate world/v/p position of terrain chunk
			if ( g_pCBPerFrame )
			{
				CBPerFrame cb;
				GGMatrixIdentity(&cb.mWorld);
				GGMatrixIdentity(&cb.mView);
				GGGetTransform(GGTS_PROJECTION,&cb.mProjection);
				GGMatrixTranspose(&cb.mWorld,&cb.mWorld);
				GGMatrixTranspose(&cb.mView,&cb.mView);
				GGMatrixTranspose(&cb.mProjection,&cb.mProjection);
				m_pImmediateContext->UpdateSubresource( g_pCBPerFrame, 0, NULL, &cb, 0, 0 );
				m_pImmediateContext->VSSetConstantBuffers ( 0, 1, &g_pCBPerFrame );
			}

			// Fill vertex buffer with this sprites data
			if (g_pSpriteVertexBuffer == NULL)
			{
				// create vertex buffer
				D3D11_BUFFER_DESC bufferDesc;
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				//bufferDesc.ByteWidth       = 6*sizeof(SpriteVertex);
				//PE: Looks like we only have 4*VERTEX2D
				bufferDesc.ByteWidth = 4 * sizeof(VERTEX2D); //PE: fix for issue 50 and 22

				bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				bufferDesc.MiscFlags = 0;
				m_pD3D->CreateBuffer(&bufferDesc, NULL, &g_pSpriteVertexBuffer);
			}

			//sprintf(debugmsg, "(%d),", iTemp);
			//OutputDebugString(debugmsg);
			//OutputDebugString("5,"); //PE: crash after this line.
			if (g_pSpriteVertexBuffer != NULL)
			{
				try {
					// lock and write to vertex buffer
					D3D11_MAPPED_SUBRESOURCE resource;
					std::memset(&resource, 0, sizeof(resource));
					m_pImmediateContext->Map(g_pSpriteVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
					SpriteVertex* LockedVertex = (SpriteVertex*)resource.pData;
					//std::memcpy(LockedVertex, pCurrent->lpVertices, 6 * sizeof(SpriteVertex)); // read exception so check all lpVertices
					//PE: 100218 - Looks like we only have 4*VERTEX2D , so reading 6* could generate a read exception.
					std::memcpy(LockedVertex, pCurrent->lpVertices, 4 * sizeof(VERTEX2D)); //PE: fix for issue 50 and 22

					m_pImmediateContext->Unmap(g_pSpriteVertexBuffer, 0);
				}
				catch (...) {
					//PE: Seams we cant catch this read exception ? should be possible.
					OutputDebugString("RenderDrawList Exception:");
				}
			}
			//OutputDebugString("6,"); //PE: crash before this line.

			// Vertex buffers
			unsigned int stride;
			unsigned int offset;
			stride = dwFVFSize;
			offset = 0;
			m_pImmediateContext->IASetVertexBuffers ( 0, 1, &g_pSpriteVertexBuffer, &stride, &offset);

			// Draw
			m_pImmediateContext->Draw(4,0);
		}
	}

	// need to restore world/view/proj as old sprite system did not affect them (messes with shadowmap rendering)
	GGSetTransform(GGTS_WORLD,&matStoreWorld);
	GGSetTransform(GGTS_VIEW,&matStoreView);
	GGSetTransform(GGTS_PROJECTION,&matStoreProj);
	
	/*
	// set the vertex shader
	m_pD3D->SetVertexShader ( NULL );
	m_pD3D->SetFVF ( D3DFVF_VERTEX2D );

	// mike - 280305 - ensure alpha test enable is off
	m_pD3D->SetRenderState ( D3DRS_ALPHATESTENABLE,		false );

	// set the cull and fill mode
	m_pD3D->SetRenderState  ( D3DRS_FILLMODE, D3DFILL_SOLID );
	m_pD3D->SetRenderState  ( D3DRS_DITHERENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_LIGHTING, FALSE);
	m_pD3D->SetRenderState  ( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pD3D->SetRenderState  ( D3DRS_FOGENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_ZWRITEENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_ZENABLE, FALSE );

	m_pD3D->SetRenderState  ( D3DRS_COLORVERTEX, TRUE );
	m_pD3D->SetRenderState  ( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
	m_pD3D->SetRenderState  ( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_SPECULARENABLE, FALSE );
	
    // Get the state of antialiasing, and disable if set
    DWORD AAEnabled = FALSE;
    m_pD3D->GetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, &AAEnabled );
    if (AAEnabled)
    	m_pD3D->SetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, FALSE );

	// turn alpha blending on
	m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pD3D->SetRenderState ( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	m_pD3D->SetRenderState ( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

	if ( iFilterMode == 0 )
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER,       D3DTEXF_POINT  );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER,       D3DTEXF_POINT  );

		// Enable both vertical and horizontal scrolling of the texture
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSU,        D3DTADDRESS_WRAP );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSV,        D3DTADDRESS_WRAP );
	}
	else
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER,       GGTEXF_LINEAR  );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER,       GGTEXF_LINEAR  );

		// lee - 090910 - need this for sprites that have the wrap artefeact (non scrolling more common)
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSU,        GGTADDRESS_CLAMP );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSV,        GGTADDRESS_CLAMP );
	}

	m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER,       D3DTEXF_POINT );//GGTEXF_NONE  );

	// setup texture states to allow for diffuse colours
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP, GGTOP_MODULATE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1, GGTA_TEXTURE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG2, GGTA_DIFFUSE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP, GGTOP_MODULATE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, GGTA_TEXTURE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, GGTA_DIFFUSE );

    // Detection of state changes between sprites
    int iLastImage = -1;
    LPGGTEXTURE pLastTexture = NULL;
	bool bLastTransparent=true;

	// display all sprites in the list
    for ( int iTemp = 0; iTemp < iListSize; iTemp++ )
	{
        tagSpriteData* pCurrent = pList[iTemp];

        // No need for a visibility check, as if we got here then we are either:
        // 1 - doing a full render, with visibility already checked, or
        // 2 - pasting a sprite, in which case visibility is being overridden anyway

        // If this sprite has a different image to the previous, update the texture pointer
        if ( pCurrent->iImage != iLastImage)
        {
            iLastImage = pCurrent->iImage;

            // If this sprite is based on an image, go get the latest texture for it
            if ( pCurrent->iImage )
		    {
			    pCurrent->lpTexture = GetImagePointer ( pCurrent->iImage );		// get image data
		    }

            // Set the texture only when it changes
            if (pCurrent->lpTexture != pLastTexture)
            {
		        m_pD3D->SetTexture ( 0, pCurrent->lpTexture );
                pLastTexture = pCurrent->lpTexture;
            }
        }
        else
        {
            // Same image means same texture
            pCurrent->lpTexture = pLastTexture;
        }

		// toggle if sprite is not transparent (alpha-enabled)
        if (bLastTransparent != pCurrent->bTransparent)
        {
            bLastTransparent = pCurrent->bTransparent;
            m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, bLastTransparent ? TRUE : FALSE );
        }

		// U76 - 070710 - special ghost mode render state (simulates GHOST OBJECT ON)
		if ( pCurrent->bGhostMode )
		{
			m_pD3D->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCCOLOR );
			m_pD3D->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE );
		}
		else
		{
			m_pD3D->SetRenderState ( D3DRS_SRCBLEND,		D3DBLEND_SRCALPHA );
			m_pD3D->SetRenderState ( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA );
		}

        m_pD3D->DrawPrimitiveUP ( GGPT_TRIANGLESTRIP, 2, pCurrent->lpVertices, sizeof ( VERTEX2D ) );
	}

	// switch alpha blending off and re-enable z buffering
	m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pD3D->SetRenderState ( D3DRS_ZENABLE, TRUE );

    // Re-enable Antialias if it was disabled earlier
	if (AAEnabled)
		m_pD3D->SetRenderState ( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	*/

	#else
	// set the vertex shader
	m_pD3D->SetVertexShader ( NULL );
	m_pD3D->SetFVF ( D3DFVF_VERTEX2D );

	// mike - 280305 - ensure alpha test enable is off
	m_pD3D->SetRenderState ( D3DRS_ALPHATESTENABLE,		false );

	// set the cull and fill mode
	m_pD3D->SetRenderState  ( D3DRS_FILLMODE, D3DFILL_SOLID );
	m_pD3D->SetRenderState  ( D3DRS_DITHERENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_LIGHTING, FALSE);
	m_pD3D->SetRenderState  ( D3DRS_CULLMODE, D3DCULL_NONE );
	m_pD3D->SetRenderState  ( D3DRS_FOGENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_ZWRITEENABLE, FALSE );
	m_pD3D->SetRenderState  ( D3DRS_ZENABLE, FALSE );

	m_pD3D->SetRenderState  ( D3DRS_COLORVERTEX, TRUE );
	m_pD3D->SetRenderState  ( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
	m_pD3D->SetRenderState  ( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
	m_pD3D->SetRenderState  ( D3DRS_SPECULARENABLE, FALSE );
	
    // Get the state of antialiasing, and disable if set
    DWORD AAEnabled = FALSE;
    m_pD3D->GetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, &AAEnabled );
    if (AAEnabled)
    	m_pD3D->SetRenderState  ( D3DRS_MULTISAMPLEANTIALIAS, FALSE );

	// turn alpha blending on
	m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pD3D->SetRenderState ( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
	m_pD3D->SetRenderState ( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );

	if ( iFilterMode == 0 )
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER,       D3DTEXF_POINT  );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER,       D3DTEXF_POINT  );

		// Enable both vertical and horizontal scrolling of the texture
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSU,        D3DTADDRESS_WRAP );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSV,        D3DTADDRESS_WRAP );
	}
	else
	{
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MAGFILTER,       GGTEXF_LINEAR  );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_MINFILTER,       GGTEXF_LINEAR  );

		// lee - 090910 - need this for sprites that have the wrap artefeact (non scrolling more common)
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSU,        GGTADDRESS_CLAMP );
		m_pD3D->SetSamplerState ( 0, D3DSAMP_ADDRESSV,        GGTADDRESS_CLAMP );
	}

	m_pD3D->SetSamplerState ( 0, D3DSAMP_MIPFILTER,       D3DTEXF_POINT );//GGTEXF_NONE  );

	// setup texture states to allow for diffuse colours
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLOROP, GGTOP_MODULATE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG1, GGTA_TEXTURE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_COLORARG2, GGTA_DIFFUSE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAOP, GGTOP_MODULATE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, GGTA_TEXTURE );
	m_pD3D->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, GGTA_DIFFUSE );

    // Detection of state changes between sprites
    int iLastImage = -1;
    LPGGTEXTURE pLastTexture = NULL;
	bool bLastTransparent=true;

	// display all sprites in the list
    for ( int iTemp = 0; iTemp < iListSize; iTemp++ )
	{
        tagSpriteData* pCurrent = pList[iTemp];

        // No need for a visibility check, as if we got here then we are either:
        // 1 - doing a full render, with visibility already checked, or
        // 2 - pasting a sprite, in which case visibility is being overridden anyway

        // If this sprite has a different image to the previous, update the texture pointer
        if ( pCurrent->iImage != iLastImage)
        {
            iLastImage = pCurrent->iImage;

            // If this sprite is based on an image, go get the latest texture for it
            if ( pCurrent->iImage )
		    {
			    pCurrent->lpTexture = GetImagePointer ( pCurrent->iImage );		// get image data
		    }

            // Set the texture only when it changes
            if (pCurrent->lpTexture != pLastTexture)
            {
		        m_pD3D->SetTexture ( 0, pCurrent->lpTexture );
                pLastTexture = pCurrent->lpTexture;
            }
        }
        else
        {
            // Same image means same texture
            pCurrent->lpTexture = pLastTexture;
        }

		// toggle if sprite is not transparent (alpha-enabled)
        if (bLastTransparent != pCurrent->bTransparent)
        {
            bLastTransparent = pCurrent->bTransparent;
            m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, bLastTransparent ? TRUE : FALSE );
        }

		// U76 - 070710 - special ghost mode render state (simulates GHOST OBJECT ON)
		if ( pCurrent->bGhostMode )
		{
			m_pD3D->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCCOLOR );
			m_pD3D->SetRenderState( D3DRS_DESTBLEND,		D3DBLEND_ONE );
		}
		else
		{
			m_pD3D->SetRenderState ( D3DRS_SRCBLEND,		D3DBLEND_SRCALPHA );
			m_pD3D->SetRenderState ( D3DRS_DESTBLEND,		D3DBLEND_INVSRCALPHA );
		}

        m_pD3D->DrawPrimitiveUP ( GGPT_TRIANGLESTRIP, 2, pCurrent->lpVertices, sizeof ( VERTEX2D ) );
	}

	// switch alpha blending off and re-enable z buffering
	m_pD3D->SetRenderState ( D3DRS_ALPHABLENDENABLE, FALSE );
    m_pD3D->SetRenderState ( D3DRS_ZENABLE, TRUE );

    // Re-enable Antialias if it was disabled earlier
	if (AAEnabled)
		m_pD3D->SetRenderState ( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
	#endif
}

void CSpriteManager::GetDisplaySize()
{
    // Build display size from the viewport rather than the resolution...
	#ifdef DX11
	D3D11_TEXTURE2D_DESC ddsd;
	//PE: Got a g_pBackBuffer=NULL here and "D3D11: Removing Device." caused by decals. Nothing we can do if g_pBackBuffer=0.
	//PE: Seams like g_pBackBuffer=0 if "D3D11: Removing Device.", so if a future device restore is attempted this could be checked.
	g_pBackBuffer->GetDesc ( &ddsd );
    vecDisplayMin = GGVECTOR3 ( (float)0, (float)0, 0 );
    vecDisplayMax = GGVECTOR3 ( (float)ddsd.Width, (float)ddsd.Height, 1);
	#else
    GGVIEWPORT SaveViewport;
    m_pD3D->GetViewport(&SaveViewport);
    vecDisplayMin = GGVECTOR3 ( 
        (float)SaveViewport.X, 
        (float)SaveViewport.Y,
        SaveViewport.MinZ );
    vecDisplayMax = GGVECTOR3 (
        (float)(SaveViewport.X + SaveViewport.Width),
        (float)(SaveViewport.Y + SaveViewport.Height),
        SaveViewport.MaxZ );
	#endif
}


bool CSpriteManager::IsSpriteInDisplay(tagSpriteData* m_ptr)
{
    // Determine min/max of the sprite's draw area from its vertices
    GGVECTOR3 vecMin = GGVECTOR3 ( m_ptr->lpVertices[0].x, m_ptr->lpVertices[0].y, m_ptr->lpVertices[0].z );
    GGVECTOR3 vecMax = vecMin;

    for ( int iVertex = 1; iVertex < 4; iVertex++ ) 
    {
	    if ( m_ptr->lpVertices [ iVertex ].x < vecMin.x ) vecMin.x = m_ptr->lpVertices [ iVertex ].x;
	    if ( m_ptr->lpVertices [ iVertex ].y < vecMin.y ) vecMin.y = m_ptr->lpVertices [ iVertex ].y;
	    if ( m_ptr->lpVertices [ iVertex ].z < vecMin.z ) vecMin.z = m_ptr->lpVertices [ iVertex ].z;
	    if ( m_ptr->lpVertices [ iVertex ].x > vecMax.x ) vecMax.x = m_ptr->lpVertices [ iVertex ].x;
	    if ( m_ptr->lpVertices [ iVertex ].y > vecMax.y ) vecMax.y = m_ptr->lpVertices [ iVertex ].y;
	    if ( m_ptr->lpVertices [ iVertex ].z > vecMax.z ) vecMax.z = m_ptr->lpVertices [ iVertex ].z;
    }

    // Check box intersection
    return (vecDisplayMax.x >= vecMin.x && vecDisplayMin.x <= vecMax.x &&
		    vecDisplayMax.y >= vecMin.y && vecDisplayMin.y <= vecMax.y );
}



//
// DBOMesh Functions Implementation
//

//////////////////////////////////////////////////////////////////////////////////
// DBOMESH HEADER ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//#define _CRT_SECURE_NO_DEPRECATE
#include "DBOMesh.h"
#include "DBOEffects.h"
#include "stdio.h"
#include <mmsystem.h>			// multimedia functions

#include "..\..\..\..\..\GameGuru Core\Include\CImageC.h"

// Externals for DBO/Manager relationship
#include <vector>
extern std::vector< sMesh* >		g_vRefreshMeshList;

// Prototypes
DARKSDK void ConvertToFVF				( sMesh* pMesh, DWORD dwFVF );
DARKSDK void SmoothNormals				( sMesh* pMesh, float fAngle );

//////////////////////////////////////////////////////////////////////////////////
// LOCAL CLASS TO HELP WITH SHADOW CALCULATION ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
#define ADJACENCY_EPSILON 0.0001f
#define EXTRUDE_EPSILON 0.1f

struct CEdgeMapping
{
    int m_anOldEdge[2];  // vertex index of the original edge
    int m_aanNewEdge[2][2]; // vertex indexes of the new edge
                            // First subscript = index of the new edge
                            // Second subscript = index of the vertex for the edge

public:
    CEdgeMapping()
    {
        FillMemory( m_anOldEdge, sizeof(m_anOldEdge), -1 );
        FillMemory( m_aanNewEdge, sizeof(m_aanNewEdge), -1 );
    }
};

//////////////////////////////////////////////////////////////////////////////////
// INTERNAL MESH HELPER FUNCTIONS ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK_DLL bool EnsureTextureStageValid ( sMesh* pMesh, int iTextureStage )
{
//  260105 - it is OK if the texture stage is BIGGER than current, modified below
//	// Only if not shader, shaders are left alone and in tact and they will know to use the second UV stage
//	if ( !pMesh->bUseVertexShader )
//	{
		// textureindex
		DWORD dwTextureIndex = 1+iTextureStage;

		// create texture array if not present or too small
		if ( pMesh->dwTextureCount < dwTextureIndex )
		{
			sTexture* pNewTextureArray = NULL;
			pNewTextureArray = new sTexture [ dwTextureIndex ];
			g_pGlob->dwInternalFunctionCode=11013;
			if ( pMesh->pTextures ) memcpy ( pNewTextureArray, pMesh->pTextures, sizeof(sTexture) * pMesh->dwTextureCount );

			// Remove old array
			SAFE_DELETE_ARRAY( pMesh->pTextures );

			// Assign new texture array
			pMesh->pTextures = pNewTextureArray;
			pMesh->dwTextureCount = dwTextureIndex;
		}

		/*
		// create texture array if not present
		if ( pMesh->dwTextureCount != dwTextureIndex )
		{
			sTexture* pNewTextureArray = NULL;
			if ( pMesh->dwTextureCount < dwTextureIndex )
			{
				// Create larger array
				pNewTextureArray = new sTexture [ dwTextureIndex ];

				// Copy contents to new array
				if ( pMesh->pTextures ) memcpy ( pNewTextureArray, pMesh->pTextures, sizeof(sTexture) * pMesh->dwTextureCount );
			}
			else
			{
				// Create smaller array 
				pNewTextureArray = new sTexture [ dwTextureIndex ];

				// Copy contents to new array
				if ( pMesh->pTextures ) memcpy ( pNewTextureArray, pMesh->pTextures, sizeof(sTexture) * dwTextureIndex );
			}

			// Remove old array
			SAFE_DELETE_ARRAY( pMesh->pTextures );

			// Assign new texture array
			pMesh->pTextures = pNewTextureArray;
			pMesh->dwTextureCount = dwTextureIndex;
		}
		*/
//	}

	// okay
	return true;
}

DARKSDK_DLL float AlphaFromRGBA ( DWORD dwRGB )
{
	return ((dwRGB & (255 << 24)) >> 24) / 255.0f;
}

DARKSDK_DLL float RedFromRGBA ( DWORD dwRGB )
{
	return ((dwRGB & (255 << 16)) >> 16) / 255.0f;
}

DARKSDK_DLL float GreenFromRGBA ( DWORD dwRGB )
{
	return ((dwRGB & (255 <<  8)) >>  8) / 255.0f;
}

DARKSDK_DLL float BlueFromRGBA ( DWORD dwRGB )
{
	return ((dwRGB & (255 <<  0)) >>  0) / 255.0f;
}

DARKSDK_DLL void ResetMaterial ( D3DMATERIAL9* pMaterial )
{
	pMaterial->Diffuse.r		= 1.0f;
	pMaterial->Diffuse.g		= 1.0f;
	pMaterial->Diffuse.b		= 1.0f;
	pMaterial->Diffuse.a		= 1.0f;
	pMaterial->Ambient.r		= 1.0f;
	pMaterial->Ambient.g		= 1.0f;
	pMaterial->Ambient.b		= 1.0f;
	pMaterial->Ambient.a		= 1.0f;
	pMaterial->Specular.r		= 0.0f;
	pMaterial->Specular.g		= 0.0f;
	pMaterial->Specular.b		= 0.0f;
	pMaterial->Specular.a		= 0.0f;
	pMaterial->Emissive.r		= 0.0f;
	pMaterial->Emissive.g		= 0.0f;
	pMaterial->Emissive.b		= 0.0f;
	pMaterial->Emissive.a		= 0.0f;
	pMaterial->Power			= 10.0f;
}

DARKSDK_DLL void ColorMaterial ( D3DMATERIAL9* pMaterial, DWORD dwRGBA )
{
	// Set Diffuse Of Material
	pMaterial->Diffuse.r = RedFromRGBA		( dwRGBA );
	pMaterial->Diffuse.g = GreenFromRGBA	( dwRGBA );
	pMaterial->Diffuse.b = BlueFromRGBA		( dwRGBA );
	pMaterial->Diffuse.a = AlphaFromRGBA	( dwRGBA );
}

DARKSDK_DLL GGVECTOR3 MultiplyVectorAndMatrix ( GGVECTOR3 &vec, GGMATRIX &m )
{
	GGVECTOR3 vecFinal;

	vecFinal.x = vec.x * m._11 + vec.y * m._21 + vec.z * m._31 + m._41;
	vecFinal.y = vec.x * m._12 + vec.y * m._22 + vec.z * m._32 + m._42;
	vecFinal.z = vec.x * m._13 + vec.y * m._23 + vec.z * m._33 + m._43;

	return vecFinal;
}

DARKSDK_DLL void LightEval(GGVECTOR4 *col,GGVECTOR2 *input, GGVECTOR2 *sampSize,void *pfPower)
{
    float fPower = (float) pow(input->y,*((float*)pfPower));
    col->x = fPower;
    col->y = fPower;
    col->z = fPower;
    col->w = input->x;
}

//////////////////////////////////////////////////////////////////////////////////
// INTERNAL MESH SHADER FUNCTIONS ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

DARKSDK_DLL BOOL SupportsVertexShaderV11()
{
	#ifdef DX11
	return TRUE;
	#else
    GGCAPS d3dCaps;
    m_pD3D->GetDeviceCaps( &d3dCaps );
    if ( d3dCaps.VertexShaderVersion >= D3DVS_VERSION ( 1,1 ) )
	    return TRUE;
	#endif
    return FALSE;
}

DARKSDK_DLL BOOL SupportsPixelShaderV11()
{
	#ifdef DX11
	return TRUE;
	#else
    GGCAPS d3dCaps;
    m_pD3D->GetDeviceCaps( &d3dCaps );
    if ( d3dCaps.PixelShaderVersion >= D3DPS_VERSION ( 1,1 ) )
	    return TRUE;
	#endif
    return FALSE;
}

DARKSDK_DLL void FreeVertexShaderMesh ( sMesh* pMesh ) 
{
	// store FVF before making shader
	if ( pMesh->pVertexShaderEffect )
	{
		// restore FVF format (if original known)
		if ( pMesh->dwFVF==0 && pMesh->dwFVFOriginal!=0 )
			RestoreLocalMesh ( pMesh );

		// if reference, simply blank effectptr
		if ( pMesh->bVertexShaderEffectRefOnly==true )
		{
			// clear reference from mesh
			pMesh->pVertexShaderEffect = NULL;

			// leefix - 011013 - should at least wipe ptrs out
			pMesh->pVertexShader = NULL;

			// 160616 - keep hold of this in case a shader replaces another that uses SAME vert dec (GG character shader)
			// as a fundamental problem is that the orig vertdata is WIPED OUT during the shader/clone process so you cannot
			// generate the shader verts again to create pVertexDec a fresh, and it gets wiped out
			//pMesh->pVertexDec = NULL;

			// 221114 - and also wipe any reference to shader use
			//pMesh->bUseVertexShader = false; see below
			pMesh->bOverridePixelShader = false;
		}
		else
		{
			// release any previous shader
			#ifdef DX11
			#else
			SAFE_DELETE ( pMesh->pVertexShaderEffect );
			SAFE_RELEASE ( pMesh->pVertexShader );
			SAFE_RELEASE ( pMesh->pVertexDec );
			#endif

			// deactivate shader usage
			//pMesh->bUseVertexShader = false; see below
			pMesh->bOverridePixelShader = false;
		}

		// clear from mesh
		pMesh->bVertexShaderEffectRefOnly = false;
		//strcpy ( pMesh->pEffectName, "" ); see below
	}

	// 100718 - moved from above so absolutely sure shader is wiped out
	pMesh->bUseVertexShader = false;
	strcpy ( pMesh->pEffectName, "" );
}

DARKSDK_DLL void ClearTextureSettings ( sMesh* pMesh )
{
	// create texture array if not present
	if ( !EnsureTextureStageValid ( pMesh, 0 ) )
		return;

	// clear alpha factor of mesh
	pMesh->dwAlphaOverride = 0;
	pMesh->bAlphaOverride = false;

	// get texture ptr
	sTexture* pTexture = &pMesh->pTextures [ 0 ];

	// free resource
	#ifdef DX11
	#else
	SAFE_RELEASE ( pTexture->pCubeTexture );
	#endif

	// set base texture to defaults
	#ifdef DX11
	#else
	pTexture->dwStage=0;
	pTexture->dwBlendMode=GGTOP_SELECTARG1;	
	pTexture->dwBlendArg1=GGTA_DIFFUSE;
	pTexture->dwBlendArg2=GGTA_DIFFUSE;
	pTexture->dwTexCoordMode=0;
	#endif

	// Delete any vertex shader being used
	FreeVertexShaderMesh ( pMesh );
}

DARKSDK_DLL bool ValidateMeshForShader ( sMesh* pMesh, DWORD dwStagesRequired )
{
	// Delete any vertex shader being used
	FreeVertexShaderMesh ( pMesh );

	// check support
	if(!SupportsVertexShaderV11())
		return false;

	// create a two stage texture array
	if ( !EnsureTextureStageValid ( pMesh, dwStagesRequired-1 ) )
		return false;

	// store original FVF before making shader
	if ( pMesh->dwFVFOriginal==0 && pMesh->dwFVF>0 )
		pMesh->dwFVFOriginal = pMesh->dwFVF;

	// okay
	return true;
}

//////////////////////////////////////////////////////////////////////////////////
// MESH FUNCTIONS ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// Mesh Setting Functions

DARKSDK_DLL void SetWireframe ( sMesh* pMesh, bool bWireframe )
{
	pMesh->bWireframe = bWireframe;
}

DARKSDK_DLL void SetTransparency ( sMesh* pMesh, bool bTransparency )
{
	pMesh->bTransparency = bTransparency;
}

DARKSDK_DLL void SetAlphaTest ( sMesh* pMesh, DWORD dwAlphaTestValue )
{
	pMesh->dwAlphaTestValue = dwAlphaTestValue;
}

DARKSDK_DLL void SetCullCWCCW ( sMesh* pMesh, int iCullMode )
{
	if ( iCullMode>0 )
	{
		pMesh->iCullMode = iCullMode;
		pMesh->bCull = true;
	}
	else
		pMesh->bCull = false;
}

DARKSDK_DLL void SetCull ( sMesh* pMesh, bool bCull )
{
	pMesh->bCull = bCull;
}

DARKSDK_DLL void SetZRead ( sMesh* pMesh, bool bZRead )
{
	pMesh->bZRead = bZRead;
}

DARKSDK_DLL void SetZWrite ( sMesh* pMesh, bool bZWrite )
{
	pMesh->bZWrite = bZWrite;
}

DARKSDK_DLL void SetZBias ( sMesh* pMesh, bool bZBias, float fSlopeScale, float fDepth )
{
	pMesh->bZBiasActive = bZBias;
	pMesh->fZBiasSlopeScale = fSlopeScale;
	pMesh->fZBiasDepth = fDepth;
}

DARKSDK_DLL void SetFilter ( sMesh* pMesh, int iStage, int iFilter )
{
	#ifdef DX11
	#else
	if ( (DWORD)iStage<pMesh->dwTextureCount )
	{
		iFilter++; // DBV1 compatable.
		//    GGTEXF_NONE = 0,
		//    D3DTEXF_POINT = 1,
		//    GGTEXF_LINEAR = 2,
		//    D3DTEXF_ANISOTROPIC = 3,
		//    D3DTEXF_PYRAMIDALQUAD = 6,
		//    D3DTEXF_GAUSSIANQUAD = 7,
		sTexture* pTexture = &pMesh->pTextures[iStage];
		if(pTexture)
		{
			// confirm existence of filter in hardware, else use default
		    GGCAPS d3dCaps;
			m_pD3D->GetDeviceCaps( &d3dCaps );
			DWORD dwMinCapsFlag = 0, dwMagCapsFlag = 0;
			if ( iFilter==GGTEXF_LINEAR ) { dwMinCapsFlag = D3DPTFILTERCAPS_MINFLINEAR; dwMagCapsFlag = D3DPTFILTERCAPS_MAGFLINEAR; }
			if ( iFilter==D3DTEXF_ANISOTROPIC ) { dwMinCapsFlag = D3DPTFILTERCAPS_MINFANISOTROPIC; dwMagCapsFlag = D3DPTFILTERCAPS_MAGFANISOTROPIC; }
			if ( iFilter==D3DTEXF_PYRAMIDALQUAD ) { dwMinCapsFlag = D3DPTFILTERCAPS_MINFPYRAMIDALQUAD; dwMagCapsFlag = D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD; }
			if ( iFilter==D3DTEXF_GAUSSIANQUAD ) { dwMinCapsFlag = D3DPTFILTERCAPS_MINFGAUSSIANQUAD; dwMagCapsFlag = D3DPTFILTERCAPS_MAGFGAUSSIANQUAD; }
			(d3dCaps.TextureFilterCaps & dwMinCapsFlag) ? pTexture->dwMinState = iFilter : pTexture->dwMinState = D3DTEXF_POINT;
			(d3dCaps.TextureFilterCaps & dwMagCapsFlag) ? pTexture->dwMagState = iFilter : pTexture->dwMagState = D3DTEXF_POINT;
		}
	}
	#endif
}

DARKSDK_DLL void SetFilter ( sMesh* pMesh, int iFilter )
{
	SetFilter ( pMesh, 0, iFilter );
}

DARKSDK_DLL void SetLight ( sMesh* pMesh, bool bLight )
{
	pMesh->bLight = bLight;
}

DARKSDK_DLL void SetFog ( sMesh* pMesh, bool bFog )
{
	pMesh->bFog = bFog;
}

DARKSDK_DLL void SetAmbient ( sMesh* pMesh, bool bAmbient )
{
	pMesh->bAmbient = bAmbient;
}

// Mesh Component Data Functions

DARKSDK_DLL void SetDiffuseEx ( sMesh* pMesh, DWORD dwRGB )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( pMesh->dwFVF & GGFVF_DIFFUSE )
	{
		// go through all of the vertices
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)pMesh->dwVertexCount; iCurrentVertex++ )
		{
			// dwDiffuse = RGB
			*( ( DWORD* ) pMesh->pVertexData + offsetMap.dwDiffuse + ( offsetMap.dwSize * iCurrentVertex ) ) = dwRGB;
		}
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void SetDiffuse	( sMesh* pMesh, float fPercentage )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( pMesh->dwFVF & GGFVF_DIFFUSE )
	{
		// calculate an RGB from iPercentage
		DWORD dwColor = (DWORD)(fPercentage*255);
		DWORD dwRGB = GGCOLOR_ARGB ( dwColor, dwColor, dwColor, dwColor );
		SetDiffuseEx ( pMesh, dwRGB );
	}
	else
	{
		// else apply diffuse to material and activate
		pMesh->bUsesMaterial=true;
		pMesh->mMaterial.Diffuse.r = fPercentage;
		pMesh->mMaterial.Diffuse.g = fPercentage;
		pMesh->mMaterial.Diffuse.b = fPercentage;
		pMesh->mMaterial.Diffuse.a = 1.0f;
		pMesh->mMaterial.Ambient.r = fPercentage;
		pMesh->mMaterial.Ambient.g = fPercentage;
		pMesh->mMaterial.Ambient.b = fPercentage;
		pMesh->mMaterial.Ambient.a = 1.0f;
		pMesh->mMaterial.Specular.r = 0.0f;
		pMesh->mMaterial.Specular.g = 0.0f;
		pMesh->mMaterial.Specular.b = 0.0f;
		pMesh->mMaterial.Specular.a = 0.0f;
		pMesh->mMaterial.Emissive.r = 0.0f;
		pMesh->mMaterial.Emissive.g = 0.0f;
		pMesh->mMaterial.Emissive.b = 0.0f;
		pMesh->mMaterial.Emissive.a = 0.0f;
	}
}

DARKSDK_DLL void ScrollTexture ( sMesh* pMesh, int iStage, float fU, float fV )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// permission
	bool bGoAhead = false;
	DWORD dwTB = pMesh->dwFVF & GGFVF_TEXCOUNT_MASK;
	if ( iStage==0 && ( (dwTB==GGFVF_TEX1) || (dwTB==GGFVF_TEX2) || (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;
	if ( iStage==1 && ( (dwTB==GGFVF_TEX2) || (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;
	if ( iStage==2 && ( (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;

	// make sure we have data in the vertices
	if ( bGoAhead )
	{
		// go through all of the vertices
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)pMesh->dwVertexCount; iCurrentVertex++ )
		{
			*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[iStage] + ( offsetMap.dwSize * iCurrentVertex ) ) += fU;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[iStage] + ( offsetMap.dwSize * iCurrentVertex ) ) += fV;
		}
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void ScrollTexture ( sMesh* pMesh, float fU, float fV )
{
	ScrollTexture ( pMesh, 0, fU, fV );
}

DARKSDK_DLL void ScaleTexture ( sMesh* pMesh, int iStage, float fU, float fV )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// permission
	bool bGoAhead = false;
	DWORD dwTB = pMesh->dwFVF & GGFVF_TEXCOUNT_MASK;
	if ( iStage==0 && ( (dwTB==GGFVF_TEX1) || (dwTB==GGFVF_TEX2) || (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;
	if ( iStage==1 && ( (dwTB==GGFVF_TEX2) || (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;
	if ( iStage==2 && ( (dwTB==GGFVF_TEX3) ) ) bGoAhead = true;

	// make sure we have data in the vertices
	if ( bGoAhead )
	{
		// go through all of the vertices
		for ( int iCurrentVertex = 0; iCurrentVertex < (int)pMesh->dwVertexCount; iCurrentVertex++ )
		{
			*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[iStage] + ( offsetMap.dwSize * iCurrentVertex ) ) *= fU;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[iStage] + ( offsetMap.dwSize * iCurrentVertex ) ) *= fV;
		}
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void ScaleTexture ( sMesh* pMesh, float fU, float fV )
{
	// modify stage zero UV data only
	ScaleTexture ( pMesh, 0, fU, fV );
}

DARKSDK_DLL void GenerateNormals ( sMesh* pMesh, int iMode )
{
	// calculate new normals from current mesh data
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwZ>0 && offsetMap.dwNZ>0 )
		GenerateNewNormalsForMesh ( pMesh, iMode );
}

DARKSDK_DLL void AddBoxToMesh ( sMesh* pMesh, DWORD* pdwVertexOffset, DWORD* pdwIndexOffset, float fWidth1, float fHeight1, float fDepth1, float fWidth2, float fHeight2, float fDepth2, DWORD dwColor,
					bool bL, bool bR, bool bU, bool bD, bool bB, bool bF )
{
	// vertex offset
	int iVertIndexOffset = *pdwVertexOffset;
	int iIndiceIndexOffset = *pdwIndexOffset;

	// include faces that are flagged
	if ( bF )
	{
		SetupStandardVertexDec ( pMesh, pMesh->pVertexData, iVertIndexOffset +  0, fWidth1, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 0.00f );	// front
		SetupStandardVertexDec ( pMesh, pMesh->pVertexData, iVertIndexOffset +  1, fWidth2, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 0.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth2, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth1, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 1.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	if ( bB )
	{
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  0, fWidth1, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 0.00f );	// back
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  1, fWidth1, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth2, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth2, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 0.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	if ( bD )
	{
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  0, fWidth1, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 0.00f );	// top
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  1, fWidth2, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 0.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth2, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth1, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 1.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	if ( bU )
	{
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  0, fWidth1, fHeight1, fDepth2,  0.0f, -1.0f,  0.0f, dwColor, 0.00f, 1.00f );	// bottom
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  1, fWidth1, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 0.00f, 0.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth2, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 0.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth2, fHeight1, fDepth2,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 1.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	if ( bR )
	{
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  0, fWidth2, fHeight2, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );	// right
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  1, fWidth2, fHeight2, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth2, fHeight1, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth2, fHeight1, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	if ( bL )
	{
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  0, fWidth1, fHeight2, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );	// left
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  1, fWidth1, fHeight1, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  2, fWidth1, fHeight1, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
		SetupStandardVertexDec ( pMesh,	pMesh->pVertexData, iVertIndexOffset +  3, fWidth1, fHeight2, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );
		pMesh->pIndices [ iIndiceIndexOffset +  0 ] = iVertIndexOffset +  0;		pMesh->pIndices [ iIndiceIndexOffset +  1 ] = iVertIndexOffset +  1;		pMesh->pIndices [ iIndiceIndexOffset +  2 ] = iVertIndexOffset +  2;
		pMesh->pIndices [ iIndiceIndexOffset +  3 ] = iVertIndexOffset +  2;		pMesh->pIndices [ iIndiceIndexOffset +  4 ] = iVertIndexOffset +  3;		pMesh->pIndices [ iIndiceIndexOffset +  5 ] = iVertIndexOffset +  0;
		iIndiceIndexOffset+=6;
		iVertIndexOffset+=4;
	}
	
	// update counters
	*pdwVertexOffset = iVertIndexOffset;
	*pdwIndexOffset = iIndiceIndexOffset;
}

DARKSDK_DLL void ReduceMeshPolygons ( sMesh* pOriginalMesh, int iBlockMode, int iGridDimension, int iGX, int iGY, int iGZ )
{
	// limits of this process
	if ( iGX>50 || iGY>50 || iGZ>50 )
		return;

	// minimum values
	if ( iGX<3 ) iGX=3;
	if ( iGY<1 ) iGY=1;
	if ( iGZ<3 ) iGZ=3;
	
	int iScanZ = 0;

	// the collision area is a block of wood, that we are going to chip away 
	unsigned char bBlock[50][50][50];
	for ( iScanZ=0; iScanZ<iGZ; iScanZ++ )
		for ( int iScanY=0; iScanY<iGY; iScanY++ )
			for ( int iScanX=0; iScanX<iGX; iScanX++ )
				bBlock[iScanX][iScanY][iScanZ]=255;

	// get mesh verttype and size
	sOffsetMap offsetMap;
 	GetFVFOffsetMap ( pOriginalMesh, &offsetMap );

	// create a work mesh
	sMesh* pMesh = new sMesh;
	MakeLocalMeshFromOtherLocalMesh ( pMesh, pOriginalMesh );
	ConvertLocalMeshToVertsOnly ( pMesh );
	DWORD dwNumberOfVertices=pMesh->dwVertexCount;

	// boundary of mesh
	CalculateMeshBounds ( pMesh );
	GGVECTOR3 vecMin = pMesh->Collision.vecMin;
	GGVECTOR3 vecMax = pMesh->Collision.vecMax;

	// create grid system around mesh bounds
	float fGapX = (vecMax.x-vecMin.x)/iGX;
	float fGapY = (vecMax.y-vecMin.y)/iGY;
	float fGapZ = (vecMax.z-vecMin.z)/iGZ;

	// gap must be AT least one for the step through to work (objects so small would have a single pass anyway)
	if ( fGapX<1.0f ) fGapX=1.0f;
	if ( fGapY<1.0f ) fGapY=1.0f;
	if ( fGapZ<1.0f ) fGapZ=1.0f;

	// grid work varriables
	float fGapThirdX = fGapX/3.0f;
	float fGapThirdY = fGapY/3.0f;
	float fGapThirdZ = fGapZ/3.0f;
	float fGapHalfX = fGapX/2.0f;
	float fGapHalfY = fGapY/2.0f;
	float fGapHalfZ = fGapZ/2.0f;

	// scan mesh and chizel from all sides
	for ( int iSide=0; iSide<2; iSide++ )
	{
		// chizel values
		int iChZ, iChAdd;
		float fChZHalf, fChZDir, fGridForDist;
		if ( iSide==0 )
		{ 
			iChZ=0; fChZHalf=-fGapHalfZ; fChZDir=1.0f; iChAdd=1; fGridForDist=fGapZ;
		}
		if ( iSide==1 )
		{ 
			iChZ=iGZ-1; fChZHalf=fGapZ+fGapHalfZ; fChZDir=-1.0f; iChAdd=-1; fGridForDist=fGapZ;
		}

		// begin chizelling from a side
		iScanZ=iChZ;
		for ( int iScanY=0; iScanY<iGY; iScanY++ )
		{
			for ( int iScanX=0; iScanX<iGX; iScanX++ )
			{
				// originate a ray
				GGVECTOR3 vecRay = GGVECTOR3 ( vecMin.x+(iScanX*fGapX)+fGapHalfX, vecMin.y+(iScanY*fGapY)+fGapHalfY, vecMin.z+(iScanZ*fGapZ)+fChZHalf );
				GGVECTOR3 vecRayDir = GGVECTOR3 ( 0, 0, fChZDir );

				// check each poly
				int iChizelDistance=iGZ-1;

				// get average of point tests on grid square
				int iDistCount=0;
				float fDistAverage=0.0f;
				for ( float fBitX=-fGapThirdX; fBitX<=fGapThirdX; fBitX+=fGapThirdX )
				{
					for ( float fBitY=-fGapThirdY; fBitY<=fGapThirdY; fBitY+=fGapThirdY )
					{
						// sub-ray for clarity
						GGVECTOR3 vecRayBit = vecRay + GGVECTOR3(fBitX,fBitY,0);

						// go through each polygon and find CLOSEST polygon hit
						bool bPolygonHitDetected=false;
						float fU, fV, fDistance, fBestCollision=99999.0f;
						for ( DWORD dwCurrentVertex = 0; dwCurrentVertex < dwNumberOfVertices; dwCurrentVertex+=3 )
						{
							// polygon
							GGVECTOR3* pVec0 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+0) ) );
							GGVECTOR3* pVec1 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+1) ) );
							GGVECTOR3* pVec2 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+2) ) );
							if ( GGIntersectTri ( pVec0, pVec1, pVec2, &vecRayBit, &vecRayDir, &fU, &fV, &fDistance )==TRUE )
							{
								if ( fDistance<fBestCollision)
								{
									bPolygonHitDetected=true;
									fBestCollision=fDistance;
								}
							}
						}
						if ( bPolygonHitDetected )
						{
							if ( fBestCollision > fGapHalfZ+0.1f )
							{
								fDistAverage += fBestCollision-fGapHalfZ;
								iDistCount++;
							}
							else
							{
								// no chizelling
								iChizelDistance=-1;
							}
						}
					}
				}

				// test ray against polygon
				if ( iDistCount > 0 )
				{
					// average the distance for best match
					float fDistance = (fDistAverage / iDistCount)-0.5f;

					// grid lock it to the nearest line
					int iLine = (int)(fDistance/fGridForDist);

					// somewhere in between
					if ( iLine < iChizelDistance ) 
						iChizelDistance = iLine;
				}

				// ray struck a surface, chizzel away path
				int iChizelCount=0;
				int iChizel = iScanZ;
				while ( iChizelCount <= iChizelDistance )
				{
					bBlock[iScanX][iScanY][iChizel] = 253;
					iChizelCount++;
					iChizel+=iChAdd;
				}
			}
		}
	}

	// free usages
	SAFE_DELETE ( pMesh );

	// create logical boxes from blockdata
	struct logicalboxtype
	{
		int iX1, iY1, iZ1;
		int iX2, iY2, iZ2;
		bool bLeft, bRight, bUp, bDown, bBack, bFore;
	};
	DWORD dwLogicalBoxesMax=32;
	DWORD dwLogicalBoxesCount=0;
	logicalboxtype* pLogicalBoxes = new logicalboxtype[dwLogicalBoxesMax];

	// go through all blocks
	for ( iScanZ=0; iScanZ<iGZ; iScanZ++ )
	{
		for ( int iScanY=0; iScanY<iGY; iScanY++ )
		{
			for ( int iScanX=0; iScanX<iGX; iScanX++ )
			{
				if ( bBlock[iScanX][iScanY][iScanZ]==255 )
				{
					// block exists - expand a logic-box here
					logicalboxtype sBox;
					sBox.iX1=iScanX;
					sBox.iY1=iScanY;
					sBox.iZ1=iScanZ;
					sBox.iX2=iScanX;
					sBox.iY2=iScanY;
					sBox.iZ2=iScanZ;

					// expand in all areas where blocks permit
					bool bExpand = true;
					while ( bExpand )
					{
						// count expansions
						int iExpandCount=0;

						// try left expand
						int iX=sBox.iX1-1;
						bool bCanExpand=true;
						if ( iX==-1 ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
								for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iX1=iX; iExpandCount++; }

						// try right expand
						iX=sBox.iX2+1;
						bCanExpand=true;
						if ( iX==iGX ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
								for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iX2=iX; iExpandCount++; }

						// try up expand
						int iY=sBox.iY1-1;
						bCanExpand=true;
						if ( iY==-1 ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iX=sBox.iX1; iX<=sBox.iX2; iX++ )
								for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iY1=iY; iExpandCount++; }

						// try down expand
						iY=sBox.iY2+1;
						bCanExpand=true;
						if ( iY==iGY ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iX=sBox.iX1; iX<=sBox.iX2; iX++ )
								for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iY2=iY; iExpandCount++; }

						// try back expand
						int iZ=sBox.iZ1-1;
						bCanExpand=true;
						if ( iZ==-1 ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iX=sBox.iX1; iX<=sBox.iX2; iX++ )
								for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iZ1=iZ; iExpandCount++; }

						// try fore expand
						iZ=sBox.iZ2+1;
						bCanExpand=true;
						if ( iZ==iGZ ) bCanExpand=false;
						if ( bCanExpand )
							for ( int iX=sBox.iX1; iX<=sBox.iX2; iX++ )
								for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
									if ( bBlock[iX][iY][iZ]<255 )
										bCanExpand=false;
						if ( bCanExpand ) { sBox.iZ2=iZ; iExpandCount++; }

						// if cannot expand anywhere, exit
						if ( iExpandCount==0 ) bExpand=false;
					}
					
					int iX = 0;

					// once expanded as far as we can, delete all blocks within it
					for ( iX=sBox.iX1; iX<=sBox.iX2; iX++ )
						for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
							for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
								bBlock[iX][iY][iZ] = 254;

					// see if logicbox sides completely hidden by more blocks (for HSR)
					iX=sBox.iX1-1;
					sBox.bLeft=false;
					if ( iX==-1 ) sBox.bLeft=true;
					if ( !sBox.bLeft )
						for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
							for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bLeft=true;

					// check right
					iX=sBox.iX2+1;
					sBox.bRight=false;
					if ( iX==iGX ) sBox.bRight=true;
					if ( !sBox.bRight )
						for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
							for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bRight=true;

					// check up
					int iY=sBox.iY1-1;
					sBox.bUp=false;
					if ( iY==-1 ) sBox.bUp=true;
					if ( !sBox.bUp )
						for ( iX=sBox.iX1; iX<=sBox.iX2; iX++ )
							for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bUp=true;

					// check down
					iY=sBox.iY2+1;
					sBox.bDown=false;
					if ( iY==iGY ) sBox.bDown=true;
					if ( !sBox.bDown )
						for ( iX=sBox.iX1; iX<=sBox.iX2; iX++ )
							for ( int iZ=sBox.iZ1; iZ<=sBox.iZ2; iZ++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bDown=true;

					// check back
					int iZ=sBox.iZ1-1;
					sBox.bBack=false;
					if ( iZ==-1 ) sBox.bBack=true;
					if ( !sBox.bBack )
						for ( iX=sBox.iX1; iX<=sBox.iX2; iX++ )
							for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bBack=true;

					// check fore
					iZ=sBox.iZ2+1;
					sBox.bFore=false;
					if ( iZ==iGZ ) sBox.bFore=true;
					if ( !sBox.bFore )
						for ( iX=sBox.iX1; iX<=sBox.iX2; iX++ )
							for ( int iY=sBox.iY1; iY<=sBox.iY2; iY++ )
								if ( bBlock[iX][iY][iZ]==253 )
									sBox.bFore=true;

					// expand list if too small
					if ( dwLogicalBoxesCount+1>=dwLogicalBoxesMax )
					{
						DWORD dwNewBoxMax = dwLogicalBoxesMax*2;
						logicalboxtype* pNewLogicalBoxes = new logicalboxtype[dwNewBoxMax];
						memcpy ( pNewLogicalBoxes, pLogicalBoxes, dwLogicalBoxesMax * sizeof(logicalboxtype) );
						SAFE_DELETE(pLogicalBoxes);
						pLogicalBoxes=pNewLogicalBoxes;
						dwLogicalBoxesMax=dwNewBoxMax;
					}

					// and add to logic box array
					pLogicalBoxes [ dwLogicalBoxesCount ] = sBox;
					dwLogicalBoxesCount++;
				}
			}
		}
	}

	// work out actual usage of new boxes
	DWORD dwVertexCount=0, dwIndexCount=0;
	for ( DWORD dwCurrentBox=0; dwCurrentBox<dwLogicalBoxesCount; dwCurrentBox++ )
	{
		// Box dimensions
		bool bL = pLogicalBoxes [ dwCurrentBox ].bLeft;
		bool bR = pLogicalBoxes [ dwCurrentBox ].bRight;
		bool bU = pLogicalBoxes [ dwCurrentBox ].bUp;
		bool bD = pLogicalBoxes [ dwCurrentBox ].bDown;
		bool bB = pLogicalBoxes [ dwCurrentBox ].bBack;
		bool bF = pLogicalBoxes [ dwCurrentBox ].bFore;
		if ( bL ) { dwVertexCount+=4; dwIndexCount+=6; }
		if ( bR ) { dwVertexCount+=4; dwIndexCount+=6; }
		if ( bU ) { dwVertexCount+=4; dwIndexCount+=6; }
		if ( bD ) { dwVertexCount+=4; dwIndexCount+=6; }
		if ( bB ) { dwVertexCount+=4; dwIndexCount+=6; }
		if ( bF ) { dwVertexCount+=4; dwIndexCount+=6; }
	}

	// create new mesh
	DWORD dwVertexOffset=0, dwIndexOffset=0;
	if ( SetupMeshData ( pOriginalMesh, dwVertexCount, dwIndexCount ) )
	{
		// default values
		DWORD dwColor = GGCOLOR(1,1,1,1);

		// fill new mesh with new meshdata
		for ( DWORD dwCurrentBox=0; dwCurrentBox<dwLogicalBoxesCount; dwCurrentBox++ )
		{
			// Box dimensions
			int iScanX = pLogicalBoxes [ dwCurrentBox ].iX1;
			int iScanY = pLogicalBoxes [ dwCurrentBox ].iY1;
			int iScanZ = pLogicalBoxes [ dwCurrentBox ].iZ1;
			int iScanWX = 1+(pLogicalBoxes [ dwCurrentBox ].iX2-iScanX);
			int iScanWY = 1+(pLogicalBoxes [ dwCurrentBox ].iY2-iScanY);
			int iScanWZ = 1+(pLogicalBoxes [ dwCurrentBox ].iZ2-iScanZ);
			bool bL = pLogicalBoxes [ dwCurrentBox ].bLeft;
			bool bR = pLogicalBoxes [ dwCurrentBox ].bRight;
			bool bU = pLogicalBoxes [ dwCurrentBox ].bUp;
			bool bD = pLogicalBoxes [ dwCurrentBox ].bDown;
			bool bB = pLogicalBoxes [ dwCurrentBox ].bBack;
			bool bF = pLogicalBoxes [ dwCurrentBox ].bFore;

			// calculate box size
			GGVECTOR3 vecBox = GGVECTOR3 ( vecMin.x+(iScanX*fGapX), vecMin.y+(iScanY*fGapY), vecMin.z+(iScanZ*fGapZ) );
			float fWidth1 = vecBox.x;
			float fHeight1 = vecBox.y;
			float fDepth1 = vecBox.z;
			float fWidth2 = fWidth1+(fGapX*iScanWX);
			float fHeight2 = fHeight1+(fGapY*iScanWY);
			float fDepth2 = fDepth1+(fGapZ*iScanWZ);
			
			// create box
			AddBoxToMesh ( pOriginalMesh, &dwVertexOffset, &dwIndexOffset, fWidth1, fHeight1, fDepth1, fWidth2, fHeight2, fDepth2, dwColor, bL, bR, bU, bD, bB, bF );
		}

		// setup mesh values
		pOriginalMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
		pOriginalMesh->iDrawVertexCount = pOriginalMesh->dwVertexCount;
		pOriginalMesh->iDrawPrimitives  = pOriginalMesh->dwIndexCount  / 3;
	}

	// free usages
	SAFE_DELETE(pLogicalBoxes);

	// flag mesh for a VB update
	pOriginalMesh->bMeshHasBeenReplaced=true;
}

DARKSDK_DLL int CheckIfMeshSolid ( sMesh* pMesh, int iGX, int iGY, int iGZ )
{
	// limits of this process
	if ( iGX>50 || iGY>50 || iGZ>50 )
		return false;

	// minimum values
	if ( iGX<3 ) iGX=3;
	if ( iGY<3 ) iGY=3;
	if ( iGZ<3 ) iGZ=3;

	// cound cast hits and overall
	int iCountCastHits=0;
	int iOverallCasts=0;

	// get mesh verttype and size
	sOffsetMap offsetMap;
 	GetFVFOffsetMap ( pMesh, &offsetMap );

	// boundary of mesh
	GGVECTOR3 vecMin = pMesh->Collision.vecMin;
	GGVECTOR3 vecMax = pMesh->Collision.vecMax;

	// determine best side to scan from
	GGVECTOR3 vecSize = vecMax - vecMin;
	int iSideScan=0;
	float fSmallest=vecSize.x;
	if ( vecSize.y<fSmallest ) { fSmallest=vecSize.y; iSideScan=1; }
	if ( vecSize.z<fSmallest ) { fSmallest=vecSize.z; iSideScan=2; }

	// create grid system around mesh bounds
	float fGapX = (vecSize.x)/iGX;
	float fGapY = (vecSize.y)/iGY;
	float fGapZ = (vecSize.z)/iGZ;
	float fGapEdgeX = fGapX;
	float fGapEdgeY = fGapY;
	float fGapEdgeZ = fGapZ;

	// setup ray direction and plane
	GGVECTOR3 vecRayDir;
	if ( iSideScan==0 ) { vecRayDir = GGVECTOR3 ( 1.0f, 0, 0 ); fGapX=vecSize.x*1.01f; fGapEdgeX=0.0f; } 
	if ( iSideScan==1 ) { vecRayDir = GGVECTOR3 ( 0, 1.0f, 0 ); fGapY=vecSize.y*1.01f; fGapEdgeY=0.0f; } 
	if ( iSideScan==2 ) { vecRayDir = GGVECTOR3 ( 0, 0, 1.0f ); fGapZ=vecSize.z*1.01f; fGapEdgeZ=0.0f; } 

	// gap must be AT least one for the step through to work (objects so small would have a single pass anyway)
	if ( fGapX<1.0f ) fGapX=1.0f;
	if ( fGapY<1.0f ) fGapY=1.0f;
	if ( fGapZ<1.0f ) fGapZ=1.0f;

	// scan a grid
	for ( float fX=vecMin.x+fGapEdgeX; fX<=vecMax.x-fGapEdgeX; fX+=fGapX )
	{
		for ( float fY=vecMin.y+fGapEdgeY; fY<=vecMax.y-fGapEdgeY; fY+=fGapY )
		{
			for ( float fZ=vecMin.z+fGapEdgeZ; fZ<=vecMax.z-fGapEdgeZ; fZ+=fGapZ )
			{
				// originate a ray
				float fRayDist=0.0f;
				GGVECTOR3 vecRayStart;
				if ( iSideScan==0 ) { vecRayStart = GGVECTOR3 ( vecMin.x+(float)fabs(vecSize.x*2)*-1.0f, fY, fZ ); fRayDist=(float)fabs(vecMin.x-vecRayStart.x); }
				if ( iSideScan==1 ) { vecRayStart = GGVECTOR3 ( fX, vecMin.y+(float)fabs(vecSize.y*2)*-1.0f, fZ ); fRayDist=(float)fabs(vecMin.y-vecRayStart.y); }
				if ( iSideScan==2 ) { vecRayStart = GGVECTOR3 ( fX, fY, vecMin.z+(float)fabs(vecSize.z*2)*-1.0f ); fRayDist=(float)fabs(vecMin.z-vecRayStart.z); }

				// go through each polygon and find CLOSEST polygon hit
				bool bPolygonHitDetected=false;
				float fU, fV, fDistance, fBestCollision=99999.0f;

				// with indexed data
				if ( pMesh->pIndices )
				{
					DWORD dwNumberOfIndices = pMesh->dwIndexCount;
					for ( DWORD dwCurrentIndex = 0; dwCurrentIndex < dwNumberOfIndices; dwCurrentIndex+=3 )
					{
						// use indice data to find correct verts for poly
						DWORD dwV1 = pMesh->pIndices [ dwCurrentIndex+0 ];
						DWORD dwV2 = pMesh->pIndices [ dwCurrentIndex+1 ];
						DWORD dwV3 = pMesh->pIndices [ dwCurrentIndex+2 ];

						// polygon
						GGVECTOR3* pVec0 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwV1 ) );
						GGVECTOR3* pVec1 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwV2 ) );
						GGVECTOR3* pVec2 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * dwV3 ) );
						if ( GGIntersectTri ( pVec0, pVec1, pVec2, &vecRayStart, &vecRayDir, &fU, &fV, &fDistance )==TRUE )
						{
							// I hit one, make sire it is right on the edge of the boundbox (or else it has a gap as though from a curve that we can see through)
							if ( fDistance<fRayDist+12.5f )
							{
								// can leave now
								bPolygonHitDetected=true;
								break;
							}
						}
					}
				}
				else
				{
					// pure vertex data
					DWORD dwNumberOfVertices = pMesh->dwVertexCount;
					for ( DWORD dwCurrentVertex = 0; dwCurrentVertex < dwNumberOfVertices; dwCurrentVertex+=3 )
					{
						// polygon
						GGVECTOR3* pVec0 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+0) ) );
						GGVECTOR3* pVec1 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+1) ) );
						GGVECTOR3* pVec2 = (GGVECTOR3*)( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * (dwCurrentVertex+2) ) );
						if ( GGIntersectTri ( pVec0, pVec1, pVec2, &vecRayStart, &vecRayDir, &fU, &fV, &fDistance )==TRUE )
						{
							// I hit one, make sire it is right on the edge of the boundbox (or else it has a gap as though from a curve that we can see through)
							if ( fDistance<fRayDist+12.5f )
							{
								// can leave now
								bPolygonHitDetected=true;
								break;
							}
						}
					}
				}
				if ( bPolygonHitDetected )
				{
					// hole found, can leave now
					iCountCastHits++;
				}
				iOverallCasts++;
			}
		}
	}

	// check if solid, partially solid or not even there
	int iMarginForError = iOverallCasts/20;
	if ( iCountCastHits>iOverallCasts-iMarginForError )
	{
		// entirely solid (no holes - solid wall)
		return 2;
	}
	else
	{
		if ( iCountCastHits>0 )
		{
			// partially solid (has hole in it)
			return 1;
		}
		else
		{
			// not solid
			return 0;
		}
	}
}

DARKSDK_DLL bool CheckIfMeshBlocking ( sMesh* pMesh, float X1, float Y1, float Z1, float X2, float Y2, float Z2 )
{
	// calculate bounds of mesh
	float boundboxX1 = pMesh->Collision.vecMin.x;
	float boundboxY1 = pMesh->Collision.vecMin.y;
	float boundboxZ1 = pMesh->Collision.vecMin.z;
	float boundboxX2 = pMesh->Collision.vecMax.x;
	float boundboxY2 = pMesh->Collision.vecMax.y;
	float boundboxZ2 = pMesh->Collision.vecMax.z;

	// ensure mesh encountering plane being detected
	if ( X1 >= boundboxX1 && X2 <= boundboxX2 )
	{
        if ( Y1 >= boundboxY1 && Y2 <= boundboxY2 )
		{
           if ( Z1 >= boundboxZ1 && Z2 <= boundboxZ2 )
		   {
				if ( CheckIfMeshSolid ( pMesh, 10, 10, 10 )>0 )
				{
					// blocking
					return true;
				}
		   }
		}
	}
	
	// not blocking
	return false;
}


// Mesh Vertex Data Functions

DARKSDK_DLL void SetPositionData ( sMesh* pMesh, int iCurrentVertex, float fX, float fY, float fZ )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( offsetMap.dwZ>0 )
	{
		// set component
		*( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) ) = fX;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iCurrentVertex ) ) = fY;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iCurrentVertex ) ) = fZ;
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void SetNormalsData ( sMesh* pMesh, int iCurrentVertex, float fNX, float fNY, float fNZ )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( pMesh->dwFVF & GGFVF_NORMAL )
	{
		// set normals vector component
		*( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) ) = fNX;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwNY + ( offsetMap.dwSize * iCurrentVertex ) ) = fNY;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwNZ + ( offsetMap.dwSize * iCurrentVertex ) ) = fNZ;
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void SetDiffuseData ( sMesh* pMesh, int iCurrentVertex, DWORD dwDiffuse )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( pMesh->dwFVF & GGFVF_DIFFUSE )
	{
		// set component
		*( ( DWORD* ) pMesh->pVertexData + offsetMap.dwDiffuse + ( offsetMap.dwSize * iCurrentVertex ) ) = dwDiffuse;
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void SetUVData ( sMesh* pMesh, int iCurrentVertex, float fU, float fV )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// make sure we have data in the vertices
	if ( pMesh->dwFVF & GGFVF_TEX1 )
	{
		// set single UV vertex component
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * iCurrentVertex ) ) = fU;
		*( ( float* ) pMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * iCurrentVertex ) ) = fV;
	}

	// flag mesh for a VB update
	pMesh->bVBRefreshRequired=true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL bool AddMeshToData ( sMesh* pFinalMesh, sMesh* pMeshToAdd )
{
	// ensure meshes valid
	SAFE_MEMORY ( pFinalMesh );
	SAFE_MEMORY ( pMeshToAdd );

	// convert addmesh to standard
	sMesh* pOriginalMesh = new sMesh;
	MakeLocalMeshFromOtherLocalMesh ( pOriginalMesh, pFinalMesh );

	// convert addmesh to standard
	sMesh* pStandardMesh = new sMesh;
	MakeLocalMeshFromOtherLocalMesh ( pStandardMesh, pMeshToAdd );
	ConvertLocalMeshToFVF ( pStandardMesh, pOriginalMesh->dwFVF );
	
	// make vertex and index buffers for final
	DWORD dwTotalVertices = pOriginalMesh->dwVertexCount + pStandardMesh->dwVertexCount;
	DWORD dwTotalIndices = pOriginalMesh->dwIndexCount + pStandardMesh->dwIndexCount;

	// if creation successful, continue
	if ( SetupMeshFVFData ( pFinalMesh, pOriginalMesh->dwFVF, dwTotalVertices, dwTotalIndices ) )
	{
		// copy over original to final
		memcpy ( pFinalMesh->pVertexData, pOriginalMesh->pVertexData, pOriginalMesh->dwVertexCount * pOriginalMesh->dwFVFSize );
		memcpy ( pFinalMesh->pIndices, pOriginalMesh->pIndices, pOriginalMesh->dwIndexCount * sizeof(WORD) );

		// new vertex data in index list (word=65535max)
		WORD dwVertexStart = (WORD)pOriginalMesh->dwVertexCount;

		// copy over standard to final
		BYTE* pDestVertexData = (BYTE*)pFinalMesh->pVertexData + ( dwVertexStart * pOriginalMesh->dwFVFSize );
		BYTE* pDestIndexData = (BYTE*)pFinalMesh->pIndices + ( pOriginalMesh->dwIndexCount * sizeof(WORD) );
		memcpy ( pDestVertexData, pStandardMesh->pVertexData, pStandardMesh->dwVertexCount * pStandardMesh->dwFVFSize );
		memcpy ( pDestIndexData, pStandardMesh->pIndices, pStandardMesh->dwIndexCount * sizeof(WORD) );

		// increment index values to allign to vertex entries
		WORD* pIndexArray = (WORD*)pDestIndexData;
		for ( DWORD i=0; i<pStandardMesh->dwIndexCount; i++ )
			pIndexArray[i] += dwVertexStart;

		// update values of the mesh
		pFinalMesh->iDrawVertexCount = dwTotalVertices;
		pFinalMesh->iDrawPrimitives  = dwTotalIndices/3;

		// flag mesh for a VB replacement
		pFinalMesh->bMeshHasBeenReplaced=true;
	}

	// free usages
	SAFE_DELETE(pOriginalMesh);
	SAFE_DELETE(pStandardMesh);

	// success
	return true;
}

DARKSDK_DLL bool DeleteMeshFromData ( sMesh* pMesh, int iVertex1, int iVertex2, int iIndex1, int iIndex2 )
{
	// ensure mesh valid
	SAFE_MEMORY ( pMesh );

	// check ranges
  	if ( iVertex2<iVertex1 ) return false;
	if ( iIndex2<iIndex1 ) return false;
	if ( iVertex2<0 ) return false;
	if ( iIndex2<0 ) return false;
	if ( iVertex2>(int)pMesh->dwVertexCount ) return false;
	if ( iIndex2>(int)pMesh->dwIndexCount ) return false;

	// calculate new vertex and index arrays
	DWORD dwNewVertexCount = pMesh->dwVertexCount - (iVertex2-iVertex1);
	DWORD dwNewIndexCount = pMesh->dwIndexCount - (iIndex2-iIndex1);

	// create new arrays
	BYTE* pNewVertexData = new BYTE [ dwNewVertexCount * pMesh->dwFVFSize ];
	BYTE* pNewIndexData = new BYTE [ dwNewIndexCount * sizeof(WORD) ];

	// copy 'before' data
	DWORD dwVBeforeCount = iVertex1;
	DWORD dwIBeforeCount = iIndex1;
	if ( dwVBeforeCount > 0 ) memcpy ( pNewVertexData, pMesh->pVertexData, dwVBeforeCount * pMesh->dwFVFSize );
	if ( dwIBeforeCount > 0 ) memcpy ( pNewIndexData, pMesh->pIndices, dwIBeforeCount * sizeof(WORD) );

	// copy 'after' data
	DWORD dwVAfterCount = pMesh->dwVertexCount - iVertex2;
	DWORD dwIAfterCount = pMesh->dwIndexCount - iIndex2;
	if ( dwVAfterCount > 0 )
	{
		BYTE* pVertStart = (BYTE*)pNewVertexData + ( dwVBeforeCount * pMesh->dwFVFSize );
		BYTE* pVertEnd = (BYTE*)pMesh->pVertexData + ( iVertex2 * pMesh->dwFVFSize );
		memcpy ( pVertStart, pVertEnd, dwVAfterCount * pMesh->dwFVFSize );
	}
	if ( dwIAfterCount > 0 )
	{
		BYTE* pIndexStart = (BYTE*)pNewIndexData + ( dwIBeforeCount * sizeof(WORD) );
		BYTE* pIndexEnd = (BYTE*)pMesh->pIndices + ( iIndex2 * sizeof(WORD) );
		memcpy ( pIndexStart, pIndexEnd, dwIAfterCount * sizeof(WORD) );
	}

	// reduce 'indice' data after vertex data shuffle
	WORD wVGap = iVertex2 - iVertex1;
	WORD* pThisIndexData = (WORD*)pNewIndexData;
	for ( DWORD i=0; i<dwNewIndexCount; i++)
		if(pThisIndexData[i]>=iVertex2)
			pThisIndexData[i]=pThisIndexData[i]-wVGap;

	// remove old arraus
	SAFE_DELETE_ARRAY(pMesh->pVertexData);
	SAFE_DELETE_ARRAY(pMesh->pIndices);

	// replace with new arrays
	pMesh->dwVertexCount = dwNewVertexCount;
	pMesh->pVertexData = pNewVertexData;
	pMesh->dwIndexCount = dwNewIndexCount;
	pMesh->pIndices = (WORD*)pNewIndexData;
	pMesh->iDrawVertexCount = dwNewVertexCount;
	pMesh->iDrawPrimitives = dwNewIndexCount/3;

	// flag mesh for a VB replacement
	pMesh->bMeshHasBeenReplaced=true;

	// success
	return true;
}

DARKSDK_DLL int GetVertexCount ( sMesh* pMesh )
{
	return pMesh->dwVertexCount;
}

DARKSDK_DLL int GetIndexCount ( sMesh* pMesh )
{
	return pMesh->dwIndexCount;
}

DARKSDK_DLL float GetDataPositionX ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataPositionY ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataPositionZ ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataNormalsX ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwNZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwNX + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataNormalsY ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwNZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwNY + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataNormalsZ ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwNZ>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwNZ + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL DWORD GetDataDiffuse ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwDiffuse>0 )
		return *( ( DWORD* ) pMesh->pVertexData + offsetMap.dwDiffuse + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0;
}

DARKSDK_DLL float GetDataU ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwTU[0]>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwTU[0] + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataV ( sMesh* pMesh, int iCurrentVertex )
{
	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwTV[0]>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwTV[0] + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataU ( sMesh* pMesh, int iCurrentVertex, int iIndex )
{
	// mike - 050803 - new command to get another set of texture coords

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwTU[iIndex]>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwTU[iIndex] + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

DARKSDK_DLL float GetDataV ( sMesh* pMesh, int iCurrentVertex, int iIndex )
{
	// mike - 050803 - new command to get another set of texture coords

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );
	if ( offsetMap.dwTV[iIndex]>0 )
		return *( ( float* ) pMesh->pVertexData + offsetMap.dwTV[iIndex] + ( offsetMap.dwSize * iCurrentVertex ) );

	return 0.0f;
}

// Mesh Visual Functions

DARKSDK_DLL void Hide ( sMesh* pMesh )
{
	pMesh->bVisible = false;
}

DARKSDK_DLL void Show ( sMesh* pMesh )
{
	pMesh->bVisible = true;
}

DARKSDK_DLL void SetGhost ( sMesh* pMesh, bool bGhost, int iGhostMode )
{
	pMesh->bGhost = bGhost;
	if ( iGhostMode != -1 ) pMesh->iGhostMode = iGhostMode;
}

DARKSDK_DLL void SetTextureMode ( sMesh* pMesh, int iStage, int iMode, int iMipMode )
{
	if ( (DWORD)iStage<pMesh->dwTextureCount )
	{
		sTexture* pTexture = &pMesh->pTextures[iStage];
		if(pTexture)
		{
			pTexture->dwAddressU = iMode;
			pTexture->dwAddressV = iMode;
			pTexture->dwMipState = iMipMode;
		}
	}
}

DARKSDK_DLL void SetTextureMode ( sMesh* pMesh, int iMode, int iMipMode )
{
	SetTextureMode ( pMesh, 0, iMode, iMipMode );
}

// Mesh Texture Functions

DARKSDK_DLL int LoadOrFindTextureAsImage ( LPSTR pTextureName, LPSTR TexturePath, int iDivideTextureSize )
{
	// load texture
	int iImageIndex = 0;

	// does we have a texture name
	if ( strlen(pTextureName)==0 )
		return 0;

	// get filename only
	char pNoPath [ _MAX_PATH ];
	strcpy(pNoPath, pTextureName );
	_strrev(pNoPath);
	for(DWORD n=0; n<strlen(pNoPath); n++)
		if(pNoPath[n]=='\\' || pNoPath[n]=='/' || (unsigned char)(pNoPath[n])<32)
			pNoPath[n]=0;
	_strrev(pNoPath);

	// build a texture path and load it
	char Path [ _MAX_PATH*2 ];
	sprintf ( Path, "%s%s", TexturePath, pNoPath );
	if ( strlen ( Path ) >= _MAX_PATH ) Path[_MAX_PATH]=0;

	//PE:-MEM: Double load images sample "entitybank\\\\CityscapePBR\\Palm Tree_ao.dds"
	//PE:-MEM: Origin , LoadObject -> LoadCore -> LoadInternalTextures . (FindInternalImage)
	// texture load a : default file
	iImageIndex = LoadImageInternalEx ( Path, iDivideTextureSize );
	if ( iImageIndex==0 )
	{
		// texture load b : file as DDS
		char pDDSFile [ _MAX_PATH*2 ];
		strcpy(pDDSFile, pNoPath);
		DWORD dwLenDot = strlen(pDDSFile);
		if ( dwLenDot>4 )
		{
			pDDSFile[dwLenDot-4]=0;
			strcat(pDDSFile, ".dds");
			sprintf ( Path, "%s%s", TexturePath, pDDSFile );
			iImageIndex = LoadImageInternalEx ( Path, iDivideTextureSize );
			if ( iImageIndex==0 )
			{
				// texture load b2 : try as _D.dds
				pDDSFile[dwLenDot-4]=0;
				strcat(pDDSFile, "_D.dds");
				sprintf ( Path, "%s%s", TexturePath, pDDSFile );
				iImageIndex = LoadImageInternalEx ( Path, iDivideTextureSize );
				if ( iImageIndex==0 )
				{
					// lee - 231017 - prevent ABSOLUTE path to ORIGINAL texture being used by standalones
					// (check local folder and fileonly before rest)

					//PE: This generates doubble load. moved as last check.
					//if (strstr(TexturePath, "lightmaps\\") != NULL) {
					//sprintf(Path, "%s", pNoPath);
					//iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
					//}
					//if ( iImageIndex==0 )
					//{

					//PE: This combi works in standalone/test , so prefer this as it can be reused.
					// texture load e : 031208 - U71 - absolute path with relative path in model file
					sprintf(Path, "%s%s", TexturePath, pTextureName);
					if (strlen(Path) >= _MAX_PATH) Path[_MAX_PATH] = 0;
					iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
					if (iImageIndex == 0)
					{

						// okay, check if texture file alongside model as DDS)
						Path[strlen(Path) - 4] = 0;
						strcat(Path, ".dds");
						iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
						if (iImageIndex == 0)
						{
							// texture load c : original file
							iImageIndex = LoadImageInternalEx(pTextureName, iDivideTextureSize);
							if (iImageIndex == 0)
							{
								// texture load c2 : try as dds
								//PE: Create a doubble load in standalone , try .dds with relative path instead. ( can be reused ).
								//strcpy ( Path, pTextureName );
								sprintf(Path, "%s%s", TexturePath, pTextureName);
								if (strlen(Path) >= _MAX_PATH) Path[_MAX_PATH] = 0;
								Path[strlen(Path) - 4] = 0;
								strcat(Path, ".dds");
								iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
								if (iImageIndex == 0)
								{
									// texture load e : 031208 - U71 - absolute path with relative path in model file
									sprintf(Path, "%s%s", TexturePath, pTextureName);
									if (strlen(Path) >= _MAX_PATH) Path[_MAX_PATH] = 0;
									iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
									if (iImageIndex == 0)
									{
										// texture load f : 031208 - U71 - as above, but as DDS
										strcpy(pDDSFile, pTextureName);
										DWORD dwLenDot = strlen(pDDSFile);
										if (dwLenDot > 4)
										{
											pDDSFile[dwLenDot - 4] = 0;
											strcat(pDDSFile, ".dds");
											sprintf(Path, "%s%s", TexturePath, pDDSFile);
											iImageIndex = LoadImageInternalEx(Path, iDivideTextureSize);
										}

										if (iImageIndex == 0)
										{
											//PE: This can generates doubble loads. so this is the last combi to test.
											iImageIndex = LoadImageInternalEx(pNoPath, iDivideTextureSize);
										}

									}
								}
							}
						}
					}
				}
			}
		}
	}

	return iImageIndex;
}

DARKSDK_DLL int LoadOrFindTextureAsImage ( LPSTR pTextureName, LPSTR TexturePath )
{
	return LoadOrFindTextureAsImage ( pTextureName, TexturePath, 0 );
}

void LoadColorNormalSpecGloss ( sMesh* pMesh, LPSTR pName, LPSTR TexturePath, int iDBProMode, int iDivideTextureSize, int* piImageDiffuseIndex, int* piImageAOIndex, int* piImageNormalIndex, int* piImageSpecularIndex, int* piImageGlossIndex, int* piImageIlluminationIndex )
{
	// take copy of base texture name
	char pBaseTexName[MAX_STRING];
	strcpy ( pBaseTexName, pName );

	// 270618 - first check if diffuse file exists, if not, try tacking PBR texture naming convention to it
	if ( strlen ( pName ) > 4 )
	{
		*piImageDiffuseIndex = LoadOrFindTextureAsImage ( pBaseTexName, TexturePath, iDivideTextureSize );
		if ( *piImageDiffuseIndex == 0 )
		{
			// convert regular file reference to PBR one
			strcpy ( pBaseTexName, pName );
			pBaseTexName[strlen(pBaseTexName)-4]=0;
			strcat ( pBaseTexName, "_color.png" );
		}
	}

	// load the base color texture 
	*piImageDiffuseIndex = LoadOrFindTextureAsImage ( pBaseTexName, TexturePath, iDivideTextureSize );

	// if success, also check for and record a normal, specular and gloss (Fuse/Sketchfab character)
	*piImageAOIndex = 0;
	*piImageNormalIndex = 0;
	*piImageSpecularIndex = 0;
	*piImageGlossIndex = 0;
	*piImageIlluminationIndex = 0;
	if ( *piImageDiffuseIndex != 0 && strlen(pBaseTexName) > 6 )
	{
		// free any existing texture data for entries below
		SAFE_DELETE_ARRAY ( pMesh->pTextures );
		pMesh->dwTextureCount = 8;
		sTexture* pTexture = new sTexture[pMesh->dwTextureCount];
		pMesh->pTextures = pTexture;

		// strip file extenion
		char pTextureName[MAX_STRING];
		strcpy ( pTextureName, pBaseTexName );
		pTextureName[strlen(pTextureName)-4] = 0;

		// determine if non-PBR (DNS) or PBR (color,normal,metalness,gloss) naming convention
		int iTextureType = 0;
		LPSTR pAlbedoVariant = "";
		char pTmpName[MAX_STRING];
		if ( strnicmp ( pTextureName + strlen(pTextureName) - 5, "color", 5 ) == NULL ) 
		{
			pAlbedoVariant = "color";
			strcpy ( pTmpName, pTextureName );
			pTmpName[strlen(pTmpName)-5]=0;
			strcpy ( pTextureName, pTmpName );
			iTextureType = 2;
		}
		if ( iTextureType == 0 && strnicmp ( pTextureName + strlen(pTextureName) - 7, "diffuse", 7 ) == NULL ) 
		{
			pAlbedoVariant = "diffuse";
			strcpy ( pTmpName, pTextureName );
			pTmpName[strlen(pTmpName)-7]=0;
			strcpy ( pTextureName, pTmpName );
			iTextureType = 2;
		}
		if ( iTextureType == 0 && strnicmp ( pTextureName + strlen(pTextureName) - 1, "d", 1 ) == NULL ) 
		{
			pAlbedoVariant = "d";
			strcpy ( pTmpName, pTextureName );
			pTmpName[strlen(pTmpName)-1]=0;
			strcpy ( pTextureName, pTmpName );
			iTextureType = 1;
		}

		// act on texture type
		if ( iTextureType == 1 ) // non-PBR - use DNS
		{
			// diffuse
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, pAlbedoVariant );
			strcat ( pTmpName, ".png" );
			strcpy ( pMesh->pTextures [ 0 ].pName, pTmpName );

			// normal
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "n.png" );
			strcpy ( pMesh->pTextures [ 2 ].pName, pTmpName );
			*piImageNormalIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageNormalIndex == 0 ) *piImageNormalIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\blank_N.dds", TexturePath, iDivideTextureSize );

			// specular
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "s.png" );
			strcpy ( pMesh->pTextures [ 3 ].pName, pTmpName );
			*piImageSpecularIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageSpecularIndex == 0 ) *piImageSpecularIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\blank_none_S.dds", TexturePath, iDivideTextureSize );

			// no AO or gloss in DNS system

			//PE: We should check g.gpbroverride == 1 but...
//			if( g.gpbroverride == 1 ) {
				*piImageAOIndex = LoadOrFindTextureAsImage("effectbank\\reloaded\\media\\blank_O.dds", TexturePath, iDivideTextureSize);
				*piImageGlossIndex = LoadOrFindTextureAsImage("effectbank\\reloaded\\media\\materials\\0_Gloss.dds", TexturePath, iDivideTextureSize);
//			}
//			else {
//				*piImageAOIndex = 0;
//				*piImageGlossIndex = 0;
//			}
		}
		if ( iTextureType == 2 ) // PBR - use color,normal,metalness,gloss,illumination
		{
			// color
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, pAlbedoVariant );
			strcat ( pTmpName, ".png" );
			strcpy ( pMesh->pTextures [ 0 ].pName, pTmpName );

			// ao
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "ao.png" );
			strcpy ( pMesh->pTextures [ 1 ].pName, pTmpName );
			*piImageAOIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageAOIndex == 0 ) *piImageAOIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\blank_O.dds", TexturePath, iDivideTextureSize );

			// normal
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "normal.png" );
			strcpy ( pMesh->pTextures [ 2 ].pName, pTmpName );
			*piImageNormalIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageNormalIndex == 0 ) *piImageNormalIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\blank_N.dds", TexturePath, iDivideTextureSize );

			// specular
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "specular.png" );
			strcpy ( pMesh->pTextures [ 3 ].pName, pTmpName );
			*piImageSpecularIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageSpecularIndex == 0 )
			{
				// or metalness
				strcpy ( pTmpName, pTextureName );
				strcat ( pTmpName, "metalness.png" );
				strcpy ( pMesh->pTextures [ 3 ].pName, pTmpName );
				*piImageSpecularIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
				if ( *piImageSpecularIndex == 0 ) *piImageSpecularIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\materials\\0_Metalness.dds", TexturePath, iDivideTextureSize );
			}

			// gloss
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "gloss.png" );
			strcpy ( pMesh->pTextures [ 4 ].pName, pTmpName );
			*piImageGlossIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageGlossIndex == 0 ) *piImageGlossIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\materials\\0_Gloss.dds", TexturePath, iDivideTextureSize );

			// illumination
			strcpy ( pTmpName, pTextureName );
			strcat ( pTmpName, "illumination.png" );
			strcpy ( pMesh->pTextures [ 7 ].pName, pTmpName );
			*piImageIlluminationIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			if ( *piImageIlluminationIndex == 0 ) *piImageIlluminationIndex = LoadOrFindTextureAsImage ( "effectbank\\reloaded\\media\\blank_none_S.dds", TexturePath, iDivideTextureSize );
		}
	}

	// Populate texture set
	if ( *piImageDiffuseIndex != 0 )
	{
		pMesh->pTextures[0].iImageID = *piImageDiffuseIndex;
		pMesh->pTextures[0].pTexturesRef = GetImagePointer ( pMesh->pTextures[0].iImageID );
		pMesh->pTextures[0].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[0].iImageID );
		pMesh->pTextures[0].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[0].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures[1].iImageID = *piImageAOIndex;
		pMesh->pTextures[1].pTexturesRef = GetImagePointer ( pMesh->pTextures[1].iImageID );
		pMesh->pTextures[1].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[1].iImageID );
		pMesh->pTextures[1].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[1].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures[2].iImageID = *piImageNormalIndex;
		pMesh->pTextures[2].pTexturesRef = GetImagePointer ( pMesh->pTextures[2].iImageID );
		pMesh->pTextures[2].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[2].iImageID );
		pMesh->pTextures[2].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[2].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures[3].iImageID = *piImageSpecularIndex;
		pMesh->pTextures[3].pTexturesRef = GetImagePointer ( pMesh->pTextures[3].iImageID );
		pMesh->pTextures[3].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[3].iImageID );
		pMesh->pTextures[3].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[3].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures[4].iImageID = *piImageGlossIndex;
		pMesh->pTextures[4].pTexturesRef = GetImagePointer ( pMesh->pTextures[4].iImageID );
		pMesh->pTextures[4].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[4].iImageID );
		pMesh->pTextures[4].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[4].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures[7].iImageID = *piImageIlluminationIndex;
		pMesh->pTextures[7].pTexturesRef = GetImagePointer ( pMesh->pTextures[7].iImageID );
		pMesh->pTextures[7].pTexturesRefView = GetImagePointerView ( pMesh->pTextures[7].iImageID );
		pMesh->pTextures[7].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures[7].dwBlendArg1 = GGTA_TEXTURE;

		// From old DBP legacy behavior
		// mode to blend texture and diffuse at stage zero (load object with material alpha setting)
		if ( iDBProMode==4 )
		{
			// Mode 4 - retains material settings and blends with texture
			pMesh->pTextures[0].dwBlendMode = GGTOP_MODULATE;
			pMesh->pTextures[0].dwBlendArg1 = GGTA_TEXTURE;
			pMesh->pTextures[0].dwBlendArg2 = GGTA_DIFFUSE;
		}
		else
		{
			// Mode 0,1,2,3 - to alter texture behaviour from basic color to texture
			if ( iDBProMode!=2 && iDBProMode!=3 )
			{
				// Force a MODULATE for default behaviours of [0] and [1]
				pMesh->pTextures[0].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[1].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[2].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[3].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[4].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[5].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[6].dwBlendMode = GGTOP_MODULATE;
				pMesh->pTextures[7].dwBlendMode = GGTOP_MODULATE;
			}

			// Only force this for [0] [1] and [3] where we are expecting texture results
			if ( iDBProMode!=2 ) 
			{
				pMesh->pTextures[0].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[1].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[2].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[3].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[4].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[5].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[6].dwBlendArg1 = GGTA_TEXTURE;
				pMesh->pTextures[7].dwBlendArg1 = GGTA_TEXTURE;
			}
		}
	}
	else
	{
		pMesh->pTextures[0].iImageID = 0;
		pMesh->pTextures[0].pTexturesRef = NULL;
	}
}

DARKSDK_DLL void LoadInternalTextures ( sMesh* pMesh, LPSTR TexturePath, int iDBProMode, int iDivideTextureSize, LPSTR pOptionalLightmapNoReduce )
{
	// iDBProMode : 0-DBV1 / 1-DBPro defaults / 2-leave all states alone (for internal textureloaduse)
	// 0-DBV1 legacy behaviour
	// 1-DBPro : out of the box new pro standard
	// 2-Leave states alone to keep material/diffuse effects
	// 3-Leave states alone to keep material/texture effects
	// 4-Ensure object blends texture and diffuse at stage zero
	// 5-Leave states alone to keep multi-material effects

	// load multimaterial textures from internal name
	if ( pMesh->bUseMultiMaterial==true )
	{
		// Define textures for multi material array
		DWORD dwMultiMatCount = pMesh->dwMultiMaterialCount;
		for ( DWORD m=0; m<dwMultiMatCount; m++ )
		{
			// get multimat at index
			sMultiMaterial* pMultiMat = &(pMesh->pMultiMaterial [ m ]);

			// load diffuse, normal, spec, gloss textures and assign to mesh
			int iImageDiffuseIndex = 0, iImageAOIndex = 0, iImageNormalIndex = 0, iImageSpecularIndex = 0, iImageGlossIndex = 0, iImageIlluminationIndex = 0;
			LoadColorNormalSpecGloss ( pMesh, pMultiMat->pName, TexturePath, iDBProMode, iDivideTextureSize, &iImageDiffuseIndex, &iImageAOIndex, &iImageNormalIndex, &iImageSpecularIndex, &iImageGlossIndex, &iImageIlluminationIndex );
			/*
			// load texture
			int iImageDiffuseIndex = LoadOrFindTextureAsImage ( pMultiMat->pName, TexturePath, iDivideTextureSize );

			// if success, also check for and record a normal, specular and gloss (Fuse character)
			int iImageNormalIndex = 0, iImageSpecularIndex = 0, iImageGlossIndex = 0;
			if ( iImageDiffuseIndex != 0 && strlen(pMultiMat->pName) > 12 )
			{
				// normal
				char pTmpName[MAX_STRING];
				strcpy ( pTmpName, pMultiMat->pName );
				pTmpName[strlen(pTmpName)-11]=0;
				strcat ( pTmpName, "normal.png" );
				iImageNormalIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );

				// specular
				strcpy ( pTmpName, pMultiMat->pName );
				pTmpName[strlen(pTmpName)-11]=0;
				strcat ( pTmpName, "specular.png" );
				iImageSpecularIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );

				// or metalness
				if ( iImageSpecularIndex == 0 )
				{
					strcpy ( pTmpName, pMultiMat->pName );
					pTmpName[strlen(pTmpName)-11]=0;
					strcat ( pTmpName, "metalness.png" );
					iImageSpecularIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
				}

				// gloss
				strcpy ( pTmpName, pMultiMat->pName );
				pTmpName[strlen(pTmpName)-11]=0;
				strcat ( pTmpName, "gloss.png" );
				iImageGlossIndex = LoadOrFindTextureAsImage ( pTmpName, TexturePath, iDivideTextureSize );
			}
			*/

			// store texture ref
			if ( iImageDiffuseIndex != 0 )
			{
				// maximum diffuse with texture (for DBV1 compatibility)
				if ( iDBProMode==0 )
				{
					// 191203 - added flag to LOAD OBJECT (DBV1 by default)
					pMultiMat->mMaterial.Diffuse.r = 1.0f;
					pMultiMat->mMaterial.Diffuse.g = 1.0f;
					pMultiMat->mMaterial.Diffuse.b = 1.0f;
					pMultiMat->mMaterial.Diffuse.a = 1.0f;
				}

				// and the texture itself
				pMultiMat->pTexturesRef = GetImagePointerView ( iImageDiffuseIndex );

				// 090217 - support for normal, specular and gloss
				pMultiMat->pTexturesRefN = GetImagePointerView ( iImageNormalIndex );
				pMultiMat->pTexturesRefS = GetImagePointerView ( iImageSpecularIndex );
				pMultiMat->pTexturesRefG = GetImagePointerView ( iImageGlossIndex );
			}
			else
			{
				// no texture (uses diffuse colour for coloured model)
				pMultiMat->pTexturesRef = NULL;
				pMultiMat->pTexturesRefN = NULL;
				pMultiMat->pTexturesRefS = NULL;
				pMultiMat->pTexturesRefG = NULL;
			}

			// in order to let users use ambient, force this!
			if ( iDBProMode!=2 && iDBProMode!=3 && iDBProMode!=5 ) pMultiMat->mMaterial.Ambient = pMultiMat->mMaterial.Diffuse;
		}

		// lee - 040306 - u6rc5 - force this mode on load
		if ( iDBProMode==5 ) pMesh->bUsesMaterial = true;
	}
	else
	{
		// load standard textures for internal name
		int iImageDiffuseIndex = 0, iImageAOIndex = 0, iImageNormalIndex = 0, iImageSpecularIndex = 0, iImageGlossIndex = 0, iImageIlluminationIndex = 0;
		DWORD dwTextureCount = pMesh->dwTextureCount;
		if ( dwTextureCount == 1 )
		{
			// get texture at index
			sTexture* pTexture = &(pMesh->pTextures [ 0 ]);

			// load diffuse, normal, spec, gloss textures and assign to mesh
			LoadColorNormalSpecGloss ( pMesh, pTexture->pName, TexturePath, iDBProMode, iDivideTextureSize, &iImageDiffuseIndex, &iImageAOIndex, &iImageNormalIndex, &iImageSpecularIndex, &iImageGlossIndex, &iImageIlluminationIndex );
		}
		else
		{
			// mesh already has texture set specified, so direct load and assign them
			for ( DWORD t=0; t<dwTextureCount; t++ )
			{
				// get texture at index
				sTexture* pTexture = &(pMesh->pTextures [ t ]);

				// divide or not to divide
				int iLocalDivideValueForStage = iDivideTextureSize;
				if ( pOptionalLightmapNoReduce )
					if ( _strnicmp ( pTexture->pName, pOptionalLightmapNoReduce, strlen(pOptionalLightmapNoReduce))==NULL ) 
						iLocalDivideValueForStage=0;

				// load texture
				int iImageIndex = LoadOrFindTextureAsImage ( pTexture->pName, TexturePath, iLocalDivideValueForStage );

				// store image in texture
				if ( iImageIndex != 0 )
				{
					// setup texture settings
					pTexture->iImageID = iImageIndex;
					pTexture->pTexturesRef = GetImagePointer ( iImageIndex );
					pTexture->pTexturesRefView = GetImagePointerView ( iImageIndex );

					// lee - 200306 - u6b4 - new mode to blend texture and diffuse at stage zero (load object with material alpha setting)
					if ( iDBProMode==4 )
					{
						// Mode 4 - retains material settings and blends with texture
						if ( t==0 )
						{
							pTexture->dwBlendMode = GGTOP_MODULATE;
							pTexture->dwBlendArg1 = GGTA_TEXTURE;
							pTexture->dwBlendArg2 = GGTA_DIFFUSE;
						}
					}
					else
					{
						// Mode 0,1,2,3
						// to alter texture behaviour from basic color to texture
						if ( iDBProMode!=2 && iDBProMode!=3 )
						{
							// Force a MODULATE for default behaviours of [0] and [1]
							pTexture->dwBlendMode = GGTOP_MODULATE;
						}

						// Only force this for [0] [1] and [3] where we are expecting texture results
						if ( iDBProMode!=2 ) pTexture->dwBlendArg1 = GGTA_TEXTURE;

						// maximum diffuse with texture (for DBV1 compatibility)
						if ( iDBProMode!=2 && iDBProMode!=3 )
						{
							pMesh->mMaterial.Diffuse.r = 1.0f;
							pMesh->mMaterial.Diffuse.g = 1.0f;
							pMesh->mMaterial.Diffuse.b = 1.0f;
							pMesh->mMaterial.Diffuse.a = 1.0f;
						}
					}
				}
				else
				{
					pTexture->iImageID = 0;
					pTexture->pTexturesRef = NULL;
				}
			}
		}

		// maximum diffuse with texture (for DBV1 compatibility)
		if ( iImageDiffuseIndex != 0 )
		{
			if ( iDBProMode!=4 )
			{
				if ( iDBProMode!=2 && iDBProMode!=3 )
				{
					pMesh->mMaterial.Diffuse.r = 1.0f;
					pMesh->mMaterial.Diffuse.g = 1.0f;
					pMesh->mMaterial.Diffuse.b = 1.0f;
					pMesh->mMaterial.Diffuse.a = 1.0f;
				}
			}
		}

		// 240203 - added more defaults for better backward compat. with Patch 3 and earlier
		if ( iDBProMode!=2 && iDBProMode!=3 && iDBProMode!=4 ) pMesh->mMaterial.Ambient = pMesh->mMaterial.Diffuse;
	}
}

DARKSDK_DLL void LoadInternalTextures ( sMesh* pMesh, LPSTR TexturePath, int iDBProMode, int iDivideTextureSize )
{
	LoadInternalTextures ( pMesh, TexturePath, iDBProMode, iDivideTextureSize, 0 );
}

DARKSDK_DLL void LoadInternalTextures ( sMesh* pMesh, LPSTR TexturePath, int iDBProMode )
{
	LoadInternalTextures ( pMesh, TexturePath, iDBProMode, 0 );
}

DARKSDK_DLL void FreeInternalTextures ( sMesh* pMesh )
{
	if ( pMesh )
	{
		if ( pMesh->bUseMultiMaterial==true )
		{
			if ( pMesh->pTextures )
			{
				int iImageIndex = pMesh->pTextures->iImageID;
				if ( iImageIndex!=0 ) DeleteImage ( iImageIndex );
			}
		}
		else
		{
			DWORD dwTextureCount = pMesh->dwTextureCount;
			for ( DWORD t=0; t<dwTextureCount; t++ )
			{
				sTexture* pTexture = &(pMesh->pTextures [ t ]);
				if ( pTexture )
				{
					int iImageIndex = pTexture->iImageID;
					if ( iImageIndex!=0 )
					{
						// FPSC-RC5 - internal textures are NEGATIVE values only!
						if ( iImageIndex < 0 )
							DeleteImage ( iImageIndex );
					}
				}
			}
		}
	}
}

DARKSDK_DLL void CloneInternalTextures ( sMesh* pMeshDest, sMesh* pMeshSrc )
{
	// copy texture info from src to dest
	if ( pMeshDest && pMeshSrc )
	{
		// get texture lists
		sTexture* pTextureSrc = pMeshSrc->pTextures;
		sTexture* pTextureDest = pMeshDest->pTextures;

		// copy all texture stages data over to dest
		if ( pTextureSrc && pTextureDest && pMeshSrc->dwTextureCount>0 )
		{
			// 110406 - u6rc7 - if src bigger than dst, recreate dst so can use entire src texture data
			if ( pMeshDest->dwTextureCount < pMeshSrc->dwTextureCount )
			{
				// same size as src texture data
				pMeshDest->dwTextureCount = pMeshSrc->dwTextureCount;
				pTextureDest = new sTexture [ pMeshDest->dwTextureCount ];
				pMeshDest->pTextures = pTextureDest;
			}

			DWORD dwTextureCount = pMeshDest->dwTextureCount;
			memcpy ( pTextureDest, pTextureSrc, sizeof(sTexture)*dwTextureCount );
		}
	}
}

DARKSDK_DLL void CopyBaseMaterialToMultiMaterial ( sMesh* pMesh )
{
	// multi-material
	for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
		pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial = pMesh->mMaterial;
}

void CopyMeshSettings ( sMesh* pDestMesh, sMesh* pSrcMesh )
{
	// exit if a ptr null
	if ( !pDestMesh || !pSrcMesh )
		return;

	// copy states across
	pDestMesh->bAlphaOverride = pSrcMesh->bAlphaOverride;
	pDestMesh->bAmbient = pSrcMesh->bAmbient;
	pDestMesh->bCull = pSrcMesh->bCull;
	pDestMesh->bFog = pSrcMesh->bFog;
	pDestMesh->bGhost = pSrcMesh->bGhost;
	pDestMesh->bLight = pSrcMesh->bLight;
	pDestMesh->bOverridePixelShader = pSrcMesh->bOverridePixelShader;
	pDestMesh->bShaderBoneSkinning = pSrcMesh->bShaderBoneSkinning;
	pDestMesh->bTransparency = pSrcMesh->bTransparency;
	pDestMesh->bUseMultiMaterial = pSrcMesh->bUseMultiMaterial;
	pDestMesh->fSpecularOverride = pSrcMesh->fSpecularOverride;
	pDestMesh->bUsesMaterial = pSrcMesh->bUsesMaterial;
	pDestMesh->mMaterial = pSrcMesh->mMaterial;
	pDestMesh->bUseVertexShader = pSrcMesh->bUseVertexShader;
	pDestMesh->bVertexShaderEffectRefOnly = pSrcMesh->bVertexShaderEffectRefOnly;
	pDestMesh->bWireframe = pSrcMesh->bWireframe;
	pDestMesh->bZBiasActive = pSrcMesh->bZBiasActive;
	pDestMesh->bZRead = pSrcMesh->bZRead;
	pDestMesh->bZWrite = pSrcMesh->bZWrite;
	pDestMesh->dwAlphaOverride = pSrcMesh->dwAlphaOverride;
	pDestMesh->dwAlphaTestValue = pSrcMesh->dwAlphaTestValue;
	pDestMesh->dwMultiMaterialCount = pSrcMesh->dwMultiMaterialCount;
	pDestMesh->dwSubMeshListCount = pSrcMesh->dwSubMeshListCount;
	pDestMesh->dwThisTime = pSrcMesh->dwThisTime;
	pDestMesh->fMipMapLODBias = pSrcMesh->fMipMapLODBias;
	pDestMesh->fZBiasDepth = pSrcMesh->fZBiasDepth;
	pDestMesh->fZBiasSlopeScale = pSrcMesh->fZBiasSlopeScale;
	pDestMesh->iCastShadowIfStatic = pSrcMesh->iCastShadowIfStatic;
	pDestMesh->fBoostIntensity = pSrcMesh->fBoostIntensity;
	pDestMesh->iCurrentFrame = pSrcMesh->iCurrentFrame;
	pDestMesh->iSolidForVisibility = pSrcMesh->iSolidForVisibility;
	CopyBaseMaterialToMultiMaterial ( pDestMesh );
}

DARKSDK_DLL void SetBaseTexture ( sMesh* pMesh, int iStage, int iImage )
{
	// means DB=NO STAGE SPECIFIED
	if ( iStage==-1 )
	{
		// reset effects on object
		ClearTextureSettings( pMesh );
		iStage=0;
	}

	// Set base texture at correct stage
	SetBaseTextureStage ( pMesh, iStage, iImage );
}

DARKSDK_DLL void SetBaseTextureStage ( sMesh* pMesh, int iStage, int iImage )
{
	// when specify a stage, assume not to clear texture first
	if ( iStage==-1 ) iStage=0;

	// create texture array if not present
	g_pGlob->dwInternalFunctionCode=11012;
	if ( !EnsureTextureStageValid ( pMesh, iStage ) )
		return;

	// u64 - when set a new texture stage, also set the stage itself (otherwise it stays zero!)
	pMesh->pTextures [ iStage ].dwStage = iStage;

	// set texture stage zero (base texture)
	g_pGlob->dwInternalFunctionCode=11014;
	pMesh->pTextures [ iStage ].iImageID  = iImage;
	pMesh->pTextures [ iStage ].pTexturesRef = GetImagePointer ( iImage );
	pMesh->pTextures [ iStage ].pTexturesRefView = GetImagePointerView ( iImage );
	if ( pMesh->pTextures [ iStage ].pTexturesRef )
	{
		pMesh->pTextures [ iStage ].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures [ iStage ].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures [ iStage ].dwBlendArg2 = GGTA_DIFFUSE;
	}
	else
	{
		pMesh->pTextures [ iStage ].dwBlendMode = GGTOP_SELECTARG1;
		pMesh->pTextures [ iStage ].dwBlendArg1 = GGTA_DIFFUSE;
	}
	pMesh->pTextures [ iStage ].dwTexCoordMode = 0;

	// 250704 - add name if image holds it
	g_pGlob->dwInternalFunctionCode=11015;
	if ( iImage>0 )
	{
		LPSTR pImageFilename = GetImageName ( iImage );
		if ( pImageFilename )
			if ( strlen(pImageFilename) < MAX_STRING )
				strcpy ( pMesh->pTextures [ iStage ].pName, pImageFilename );
	}

	// clear material
	pMesh->bUsesMaterial = false;
	g_pGlob->dwInternalFunctionCode=11016;
}

DARKSDK_DLL void SetAlphaOverride ( sMesh* pMesh, float fPercentage )
{
	if ( fPercentage<100.0f )
	{
		fPercentage/=100.0f;
		DWORD dwAlpha = (DWORD)(fPercentage*255);
		GGCOLOR dwAlphaValueOnly = GGCOLOR_ARGB ( dwAlpha, 0, 0, 0 );
		pMesh->dwAlphaOverride = dwAlphaValueOnly;
		pMesh->bAlphaOverride = true;
		CopyBaseMaterialToMultiMaterial(pMesh);
	}
	else
	{
		// leefix - 041105 - can switch off if 100 percent used
		pMesh->bAlphaOverride = false;
	}
}

DARKSDK_DLL void SetDiffuseMaterial	( sMesh* pMesh, DWORD dwRGB )
{
	// lee - 040306 - u6rc5 - apply changes to base material OR multi-material!
	pMesh->bUsesMaterial = true;
	if ( pMesh->dwMultiMaterialCount > 0 )
	{
		// multi-material
		for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
		{
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Diffuse.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Diffuse.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Diffuse.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Diffuse.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		}
	}
	else
	{
		// base material
		pMesh->mMaterial.Diffuse.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
		pMesh->mMaterial.Diffuse.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
		pMesh->mMaterial.Diffuse.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
		pMesh->mMaterial.Diffuse.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		// CopyBaseMaterialToMultiMaterial(pMesh);
	}
}

DARKSDK_DLL void SetAmbienceMaterial ( sMesh* pMesh, DWORD dwRGB )
{
	// lee - 040306 - u6rc5 - apply changes to base material OR multi-material!
	pMesh->bUsesMaterial = true;
	if ( pMesh->dwMultiMaterialCount > 0 )
	{
		// multi-material
		for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
		{
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Ambient.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Ambient.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Ambient.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Ambient.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		}
	}
	else
	{
		// base material
		pMesh->mMaterial.Ambient.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
		pMesh->mMaterial.Ambient.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
		pMesh->mMaterial.Ambient.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
		pMesh->mMaterial.Ambient.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		//CopyBaseMaterialToMultiMaterial(pMesh);
	}
}

DARKSDK_DLL void SetSpecularMaterial ( sMesh* pMesh, DWORD dwRGB )
{
	// lee - 040306 - u6rc5 - apply changes to base material OR multi-material!
	pMesh->bUsesMaterial = true;
	if ( pMesh->dwMultiMaterialCount > 0 )
	{
		// multi-material
		for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
		{
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Specular.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Specular.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Specular.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Specular.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		}
	}
	else
	{
		// base material
		pMesh->mMaterial.Specular.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
		pMesh->mMaterial.Specular.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
		pMesh->mMaterial.Specular.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
		pMesh->mMaterial.Specular.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		// CopyBaseMaterialToMultiMaterial(pMesh);
	}
}

DARKSDK_DLL void SetEmissiveMaterial ( sMesh* pMesh, DWORD dwRGB )
{
	// lee - 040306 - u6rc5 - apply changes to base material OR multi-material!
	pMesh->bUsesMaterial = true;
	if ( pMesh->dwMultiMaterialCount > 0 )
	{
		// multi-material
		for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
		{
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Emissive.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Emissive.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Emissive.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Emissive.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		}
	}
	else
	{
		// base material
		pMesh->mMaterial.Emissive.r = ((dwRGB & 0x00FF0000)>>16)/255.0f;
		pMesh->mMaterial.Emissive.g = ((dwRGB & 0x0000FF00)>>8)/255.0f;
		pMesh->mMaterial.Emissive.b = ((dwRGB & 0x000000FF)>>0)/255.0f;
		pMesh->mMaterial.Emissive.a = ((dwRGB & 0xFF000000)>>24)/255.0f;
		// CopyBaseMaterialToMultiMaterial(pMesh);
	}
}

DARKSDK_DLL void SetSpecularPower ( sMesh* pMesh, float fPower )
{
	// lee - 040306 - u6rc5 - apply changes to base material OR multi-material!
	pMesh->bUsesMaterial = true;
	if ( pMesh->dwMultiMaterialCount > 0 )
	{
		// multi-material
		for ( DWORD dwMaterialIndex=0; dwMaterialIndex<pMesh->dwMultiMaterialCount; dwMaterialIndex++ )
			pMesh->pMultiMaterial [ dwMaterialIndex ].mMaterial.Power = fPower;
	}
	else
	{
		// base material
		pMesh->mMaterial.Power = fPower;
		// CopyBaseMaterialToMultiMaterial(pMesh);
	}

	// lee - 281116 - newer shaders only requires a specular modulator
	pMesh->fSpecularOverride = fPower;
}

DARKSDK_DLL void RemoveTextureRefFromMesh  ( sMesh* pMesh, LPGGTEXTURE pTextureRef )
{
	DWORD dwTextureCount = pMesh->dwTextureCount;
	for ( DWORD t=0; t<dwTextureCount; t++ )
	{
		sTexture* pTexture = &(pMesh->pTextures [ t ]);
		if ( pTexture->pTexturesRef==pTextureRef )
		{
			// remove reference to texture
			pTexture->pTexturesRef = NULL;
		}
	}
}

DARKSDK_DLL void SetMultiTexture ( sMesh* pMesh, int iStage, DWORD dwBlendMode, DWORD dwTexCoordMode, int iImage )
{
	// create texture array if not present
	if ( !EnsureTextureStageValid ( pMesh, iStage ) )
		return;

	#ifdef DX11
	#else
	// free any previous cube map
	SAFE_RELEASE ( pMesh->pTextures [ iStage ].pCubeTexture );

	// set texture stage
	pMesh->pTextures [ iStage ].dwStage = iStage;
	pMesh->pTextures [ iStage ].iImageID  = iImage;
	pMesh->pTextures [ iStage ].pTexturesRef = GetImagePointer ( iImage );
	pMesh->pTextures [ iStage ].dwBlendMode = dwBlendMode;
	pMesh->pTextures [ iStage ].dwBlendArg1 = GGTA_TEXTURE;
	pMesh->pTextures [ iStage ].dwBlendArg2 = GGTA_CURRENT;
	pMesh->pTextures [ iStage ].dwTexCoordMode = dwTexCoordMode;
	#endif
}

DARKSDK_DLL void SetCubeTexture ( sMesh* pMesh, int iStage, LPGGCUBETEXTURE pCubeTexture )
{
	// create texture array if not present
	if ( !EnsureTextureStageValid ( pMesh, iStage ) )
		return;

	// free any previous cube map
	#ifdef DX11
	#else
	SAFE_RELEASE ( pMesh->pTextures [ iStage ].pCubeTexture );

	// set cube texture stage
	pMesh->pTextures [ iStage ].dwStage = iStage;
	pMesh->pTextures [ iStage ].iImageID  = 0;
	pMesh->pTextures [ iStage ].pTexturesRef = NULL;
	pMesh->pTextures [ iStage ].pCubeTexture = pCubeTexture;
	pMesh->pTextures [ iStage ].dwBlendMode = GGTOP_MODULATE;
	pMesh->pTextures [ iStage ].dwBlendArg1 = GGTA_TEXTURE;
	pMesh->pTextures [ iStage ].dwBlendArg2 = GGTA_CURRENT;
	pMesh->pTextures [ iStage ].dwTexCoordMode = 2;
	#endif
}

DARKSDK_DLL void SetBaseColor ( sMesh* pMesh, DWORD dwRGB )
{
	// reset effects on object
	ClearTextureSettings( pMesh );

	// additionally clear texture reference
	sTexture* pTexture = &pMesh->pTextures [ 0 ];
	pTexture->iImageID = 0;
	pTexture->pTexturesRef = NULL;
	pTexture->dwBlendMode = GGTOP_SELECTARG1;
	pTexture->dwBlendArg1 = GGTA_DIFFUSE;
	pTexture->dwBlendArg2 = GGTA_DIFFUSE;
	pTexture->dwTexCoordMode = 0;

	// Assign a colour to the material
	pMesh->bUsesMaterial = true;
	ResetMaterial ( &pMesh->mMaterial );
	ColorMaterial ( &pMesh->mMaterial, dwRGB );
}

// Mesh Shader Functions

DARKSDK_DLL bool InitEffectSystem ( sMesh* pMesh, DWORD dwTextureCount, cSpecialEffect* pEffectObj )
{
	// validate for shader
	// leefix - 010204 - some FX files do not require pixel shaders!
	if(!ValidateMeshForShader ( pMesh, dwTextureCount ))
		return false;

	// vertex shader effect
	pMesh->bUseVertexShader = true;
	pMesh->pVertexShaderEffect = pEffectObj;
	if ( pMesh->pVertexShaderEffect==NULL )
		return false;

	// pixel shader effect (completed by Setup) (redundant)
	pMesh->bOverridePixelShader = true;
	pMesh->pPixelShader = NULL;

	// lee - 230306 - u6b4 - ensure the mesh is unaltered by animation (so correct mesh conversion can happen)
	ResetVertexDataInMeshPerMesh ( pMesh );

	// complete
	return true;
}

// Mesh Effect Functions

DARKSDK_DLL bool SetSpecialEffect ( sMesh* pMesh, cSpecialEffect* pEffectObj, bool bChangeMesh )
{
	// On or Off
	if ( pEffectObj )
	{
		// lee - 300914 - early out if effect shader uses BONES but this mesh has NO BONES, do not use this shader!
		if ( pEffectObj->m_bUsesBoneData==TRUE && pMesh->dwBoneCount==0 )
		{
			// silent fail, simply leaves this mesh unaffected by shader
			return false;
		}

		// initialise FX effect
		if ( !InitEffectSystem ( pMesh, pMesh->dwTextureCount, pEffectObj ) )
			return false;

		// give all effects normals (if not got them)
		if ( pMesh->pVertexShaderEffect->m_bDoNotGenerateExtraData==0 )
		{
			// leeadd - 050906 - only auto-generate if not switched off
			pMesh->pVertexShaderEffect->m_bGenerateNormals = true;
		}

		// wipe out original mesh data as mesh may be changed now
		// 220214 - SAFE_DELETE_ARRAY ( pMesh->pOriginalVertexData );
		SAFE_DELETE_ARRAY ( pMesh->pOriginalVertexData );

		// mike - 011005 - more than 8 textures causes a crash
		// lee - 270314 - no longer the case!
		//if ( pMesh->pVertexShaderEffect->m_dwTextureCount > 8 )
		//	return false;

		/* 290618 - does nothing these days
		// load effect with settings
		if ( !pMesh->pVertexShaderEffect->Setup ( pMesh ) )
		{
			pMesh->pVertexShaderEffect=NULL;
			return false;
		}
		*/

		// prepare model for effect
		if ( bChangeMesh ) pMesh->pVertexShaderEffect->Mesh ( pMesh );

		// record effect name
		strcpy ( pMesh->pEffectName, pEffectObj->m_pEffectName );
	}
	else
	{
		// Delete any vertex shader being used
		FreeVertexShaderMesh ( pMesh );
	}

	// complete
	return true;
}

DARKSDK_DLL bool SetSpecialEffect ( sMesh* pMesh, cSpecialEffect* pEffectObj )
{
	return SetSpecialEffect ( pMesh, pEffectObj, true );
}

// Mesh Custom Vertex Shader Functions

DARKSDK_DLL void CombineSubsetPolygonsInMesh ( sMesh* pMesh )
{
	// go through each polygon
//	for ()
	{
		// gather plane and vector info (V1,V2,V3) for tagretpoly

		// go through all other polygons
//		for ()
		{
			// if two vertex indexes match the tagretpoly
			if ( 0 )
			{
				// consider VA and VB being the odd vertex indices
				// VM1 and VM2 being the matchers

				// create a normalised vector from VA to VM1
				// if identical to VM1 to VB, go as VM1
				// create a normalised vector from VA to VM2
				// if identical to VM2 to VB, go as VM2
				if ( 0 )
				{
					// if VM1, VM1 is removed to produce tri from quad
					// if VM2, VM2 is removed to produce tri from quad
				}
			}
		}
	}
}

DARKSDK_DLL void SetCustomShader ( sMesh* pMesh, LPGGVERTEXSHADER pVertexShader, LPGGVERTEXLAYOUT pVertexDec, DWORD dwStagesRequired )
{
	// reset effects on object
	ClearTextureSettings( pMesh );

	// validate for shader
	if(!ValidateMeshForShader ( pMesh, dwStagesRequired ))
		return;

	// vertex shader effect active
	pMesh->bUseVertexShader = true;
	pMesh->pVertexShader = pVertexShader;
	pMesh->pVertexDec = pVertexDec;

	// set texture stages for this shader
	for (DWORD dwIndex=0; dwIndex<dwStagesRequired; dwIndex++)
	{
		pMesh->pTextures [ dwIndex ].dwStage = dwIndex;
		pMesh->pTextures [ dwIndex ].dwBlendMode = GGTOP_MODULATE;
		pMesh->pTextures [ dwIndex ].dwBlendArg1 = GGTA_TEXTURE;
		pMesh->pTextures [ dwIndex ].dwTexCoordMode = 0;
		if ( dwIndex==0 )
			pMesh->pTextures [ dwIndex ].dwBlendArg2 = GGTA_DIFFUSE;
		else
			pMesh->pTextures [ dwIndex ].dwBlendArg2 = GGTA_CURRENT;
	}
}

DARKSDK_DLL void SetNoShader ( sMesh* pMesh )
{
	// vertex shader effect deactivate
	pMesh->bUseVertexShader = false;
	if ( pMesh->pVertexShader )
	{
		#ifdef DX11
		#else
		pMesh->pVertexShader->Release();
		#endif
		pMesh->pVertexShader=NULL;
	}
	if ( pMesh->pVertexDec )
	{
		#ifdef DX11
		#else
		pMesh->pVertexDec->Release();
		#endif
		pMesh->pVertexDec=NULL;
	}
}

// Mesh Custom Pixel Shader Functions

DARKSDK_DLL void SetCustomPixelShader ( sMesh* pMesh, LPGGPIXELSHADER pPixelShader )
{
	// pixel shader effect active
	pMesh->bOverridePixelShader = true;
	pMesh->pPixelShader = pPixelShader;
}

DARKSDK_DLL void SetNoPixelShader ( sMesh* pMesh )
{
	// pixel shader effect deactivate
	pMesh->bOverridePixelShader = false;
	pMesh->pPixelShader = NULL;
}

// Mesh Animation Functions

DARKSDK_DLL void VectorTransform ( const GGVECTOR3 in1, const GGMATRIX matrix, GGVECTOR3 &out )
{
	float in2 [ 3 ] [ 4 ];

	memcpy ( &in2, matrix, sizeof ( in2 ) );

	#define DotProduct( x, y ) ( ( x ) [ 0 ] * ( y ) [ 0 ] + ( x ) [ 1 ] * ( y ) [ 1 ] + ( x ) [ 2 ] * ( y ) [ 2 ] )

	out [ 0 ] = DotProduct ( in1, in2 [ 0 ] ) + in2 [ 0 ] [ 3 ];
	out [ 1 ] = DotProduct ( in1, in2 [ 1 ] ) +	in2 [ 1 ] [ 3 ];
	out [ 2 ] = DotProduct ( in1, in2 [ 2 ] ) +	in2 [ 2 ] [ 3 ];
}

DARKSDK_DLL void AnimateBoneMeshMDL ( sObject* pObject, sFrame* pFrame )
{
	// MIKE 240303 - MDL ANIMATION STYLE /////////////////////////////////////////////////////////
	int iFrame = ( int ) pObject->fAnimFrame;
	int iPos = 0;

	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];

		// first time around, copy vertex data
		if ( pMesh->pOriginalVertexData==NULL ) CollectOriginalVertexData ( pMesh );

		// get the offset map for the vertex data
		sOffsetMap offsetMap;
		GetFVFOffsetMap ( pMesh, &offsetMap );

		int iVertexPosition = 0;
		
		for ( int iVertex = 0; iVertex < ( int ) pMesh->dwVertexCount; iVertex++ )
		{
			GGVECTOR3 vecInput = GGVECTOR3 (
													*( ( float* ) pMesh->pOriginalVertexData + offsetMap.dwX + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pOriginalVertexData + offsetMap.dwY + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pOriginalVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iVertex ) )
											   );

			GGVECTOR3 vecOutput;

			VectorTransform ( 
								vecInput,
								pObject->pAnimationSet->pAnimation->ppBoneFrames
										[ iFrame ]
										[ pObject->pAnimationSet->pAnimation->piBoneOffsetList [ iPos++ ] ],
								vecOutput
						 );

			// copy data across
			*( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iVertex ) ) = vecOutput.x;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iVertex ) ) = vecOutput.y;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iVertex ) ) = vecOutput.z;
		}

		pMesh->bVBRefreshRequired = true;
		g_vRefreshMeshList.push_back ( pMesh );
	}
}

DARKSDK_DLL void AnimateBoneMeshMDX ( sObject* pObject, sFrame* pFrame )
{
	// get mesh ptr
	sMesh* pMesh = pFrame->pMesh;
	if ( pFrame->pMesh->dwSubMeshListCount <= 1 )
		return;

	// and object animating
	if ( !pObject->bAnimPlaying && !pObject->bAnimUpdateOnce )
		return;

	// leeadd -240604 - u54 - MD2 only once update required
	pObject->bAnimUpdateOnce = false;

	/* leefix - 240604 - u54 - replaced with below new unified animation data
	// move to the next frame
	pFrame->pMesh->iNextFrame = ( pFrame->pMesh->iCurrentFrame + 1 ) % pFrame->pMesh->dwSubMeshListCount;

	// make sure anim frame is updated
	pObject->fAnimFrame = ( float ) pFrame->pMesh->iNextFrame;

	// interpolation code
	float fTime			  = ( float ) GetTickCount ( );						// get current time in milliseconds
	float fElapsedTime	  = fTime - pMesh->fLastInterp;						// get the elapsed time
	int   iAnimationSpeed = (int)(pObject->fAnimSpeed*10.0f);				// frame rate for animation
	float fInterp		  = fElapsedTime / ( 1000.0f / iAnimationSpeed );	// find out how far we are from the current frame to the next ( between 0 and 1 )

	// if our elapsed time goes over the desired time
	// segment start over and go to the next key frame
	if ( fElapsedTime >= ( 1000.0f / iAnimationSpeed ) )
	{
		// set current frame to the next frame
		pFrame->pMesh->iCurrentFrame = pFrame->pMesh->iNextFrame;

		// store the time
		pMesh->fLastInterp = fTime;
	}
	
	// get the offset map for the vertex data
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// store link to matrix
	pFrame->matOriginal = pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame ].matOriginal;

	// make sure the frame numbers are valid
	if ( pFrame->pMesh->iNextFrame + 1 >= (int)pFrame->pMesh->dwSubMeshListCount )
	{
		// set next and current frame to 0
		pFrame->pMesh->iNextFrame    = 0;
		pFrame->pMesh->iCurrentFrame = 0;
	}
	*/

	// get the offset map for the vertex data
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// store the nearest frame of the animation data
	pFrame->pMesh->iNextFrame = (int)pObject->fAnimFrame;
	float fInterp = pObject->fAnimFrame - (float)pFrame->pMesh->iNextFrame;
	if ( pFrame->pMesh->iNextFrame + 1 >= (int)pFrame->pMesh->dwSubMeshListCount )
	{
		pFrame->pMesh->iNextFrame    = 0;
		fInterp = 0.0f;
	}

	// store link to matrix
	pFrame->matOriginal = pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame ].matOriginal;

	// copy the vertices across
	for ( int iVertex = 0; iVertex < ( int ) pMesh->dwVertexCount; iVertex++ )
	{
		if ( offsetMap.dwZ>0 )
		{
			// find the frame
			sFrame*	pTheFrame     = pMesh->pSubFrameList;
			sFrame* pCurrentFrame = NULL;
			sFrame* pLastFrame    = NULL;
			sFrame* pNextFrame    = NULL;
			int     iFrame        = 0;

			GGVECTOR3 vecNextPos = GGVECTOR3 (
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame + 1 ].pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame + 1 ].pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame + 1 ].pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iVertex ) )
												 );

			GGVECTOR3 vecLastPos = GGVECTOR3 (
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame ].pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame ].pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iVertex ) ),
													*( ( float* ) pMesh->pSubFrameList [ pFrame->pMesh->iNextFrame ].pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iVertex ) )
												 );

			GGVECTOR3 vecFinal = vecLastPos + ( vecNextPos - vecLastPos ) * fInterp;

			*( ( float* ) pMesh->pVertexData + offsetMap.dwX + ( offsetMap.dwSize * iVertex ) ) = vecFinal.x;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwY + ( offsetMap.dwSize * iVertex ) ) = vecFinal.y;
			*( ( float* ) pMesh->pVertexData + offsetMap.dwZ + ( offsetMap.dwSize * iVertex ) ) = vecFinal.z;
		}
	}

	// trigger mesh to VB update
	pMesh->bVBRefreshRequired = true;
	g_vRefreshMeshList.push_back ( pMesh );
}

void AnimateBoneMeshBONE ( sObject* pObject, sFrame* pFrame, sMesh* pMesh )
{
	// first time around, copy vertex data to original-store
	if ( pMesh->pOriginalVertexData==NULL )
	{
		// first time around, copy vertex data to original-store
		DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
		pMesh->pOriginalVertexData = (BYTE*)new char [ dwTotalVertSize ];
		memcpy ( pMesh->pOriginalVertexData, pMesh->pVertexData, dwTotalVertSize );
	}

	// 010303 - new vertex blending for bones (to take advantage of multiple weights)
	GGMATRIX* matrices = new GGMATRIX [ pMesh->dwBoneCount ];
	BYTE* weighttable = new BYTE [ pMesh->dwVertexCount ];
	memset ( weighttable, 0, pMesh->dwVertexCount );

	// update all bone matrices
	for ( DWORD dwMatrixIndex = 0; dwMatrixIndex < pMesh->dwBoneCount; dwMatrixIndex++ )
	{
		if ( pMesh->pFrameMatrices [ dwMatrixIndex ] ) // lee - 180406 - u6rc10 - not all bones connect to animating frame
			GGMatrixMultiply ( &matrices [ dwMatrixIndex ], &pMesh->pBones [ dwMatrixIndex ].matTranslation, pMesh->pFrameMatrices [ dwMatrixIndex ] );
		else
			memcpy ( &matrices [ dwMatrixIndex ], &pMesh->pBones [ dwMatrixIndex ].matTranslation, sizeof(GGMATRIX) );
	}

	// run through all bones
	for ( int iBone = 0; iBone < ( int ) pMesh->dwBoneCount; iBone++ )
	{
		// go through all influenced bones
		for ( int iLoop = 0; iLoop < ( int ) pMesh->pBones [ iBone ].dwNumInfluences; iLoop++ )
		{
			// get the correct vertex and weight
			int iOffset = pMesh->pBones [ iBone ].pVertices [ iLoop ];
			float fWeight = pMesh->pBones [ iBone ].pWeights [ iLoop ];

			// Vertex Data Ptrs
			float* pDestVertexBase = (float*)(pMesh->pVertexData + ( pMesh->dwFVFSize * iOffset ));
			float* pVertexBase = (float*)(pMesh->pOriginalVertexData + ( pMesh->dwFVFSize * iOffset ));

			// clear new vertex if changing it
			if ( weighttable [ iOffset ]==0 )
			{
				memset ( pDestVertexBase, 0, 12 );
				if ( pMesh->dwFVF | GGFVF_NORMAL )
					memset ( pDestVertexBase+3, 0, 12 );
			}
			weighttable [ iOffset ] = 1;
			
			// get original vertex position
			GGVECTOR3 vec = GGVECTOR3 ( *(pVertexBase+0), *(pVertexBase+1), *(pVertexBase+2) );

			// multiply the vector and the bone matrix with weight
			GGVECTOR3 newVec = MultiplyVectorAndMatrix ( vec, matrices [ iBone ] ) * fWeight;
			
			// accumilate vertex for final result
			*(pDestVertexBase+0) += newVec.x;
			*(pDestVertexBase+1) += newVec.y;
			*(pDestVertexBase+2) += newVec.z;

			// transform and normalise the normals vector (if present)
			if ( pMesh->dwFVF | GGFVF_NORMAL )
			{
				GGVECTOR3 norm = GGVECTOR3 ( *(pVertexBase+3), *(pVertexBase+4), *(pVertexBase+5) );
				GGVECTOR3 newNorm;
				GGVec3TransformNormal ( &newNorm, &norm, &matrices [ iBone ] );
				newNorm = newNorm * fWeight;
					
				// update the normal with the new normal values
				*(pDestVertexBase+3) += newNorm.x;
				*(pDestVertexBase+4) += newNorm.y;
				*(pDestVertexBase+5) += newNorm.z;
			}
		}
	}

	// free matrix bank
	SAFE_DELETE_ARRAY ( matrices );
	SAFE_DELETE_ARRAY ( weighttable );

	// trigger mesh to VB update
	pMesh->bVBRefreshRequired = true;
	g_vRefreshMeshList.push_back ( pMesh );
}

DARKSDK_DLL void AnimateBoneMeshBONE ( sObject* pObject, sFrame* pFrame )
{
	AnimateBoneMeshBONE ( pObject, pFrame, pFrame->pMesh );
}

bool AnimateBoneMesh ( sObject* pObject, sFrame* pFrame, sMesh* pMesh )
{
	// check if mesh valid
	if ( !pMesh ) return true;

	// see if we need to deal with bones
	if ( pMesh->dwBoneCount )
	{
		// leefix - 211103 - Can reject right away if frame unchanged
		// leefix - 101203 - Fixed the logic here
		bool bQuickAccept = false;
		if ( pObject->fAnimFrame != pObject->fAnimLastFrame )
			bQuickAccept = true;

		// leefix - 140504 - once only update passes
		if ( pObject->bAnimUpdateOnce )
			bQuickAccept = true;

		// leeadd - 120204 - additional accept if slerp changed
		if ( pObject->bAnimManualSlerp==true )
			if ( pObject->fAnimSlerpTime != pObject->fAnimSlerpLastTime )
				bQuickAccept = true;

		// leefix - 300305 - objects with multiple mesh-animations would not work
		// because these two lines would suggest it was finished even after the first mesh
//		// Store frame for next quick reject check
//		pObject->fAnimLastFrame = pObject->fAnimFrame;
//		pObject->fAnimSlerpLastTime = pObject->fAnimSlerpTime;

		// Quick reject check
		if ( bQuickAccept )
		{
			// check if MDL
			if ( pObject->pAnimationSet )
			{
				if ( pObject->pAnimationSet->pAnimation )
				{
					if ( !pObject->pAnimationSet->pAnimation->bBoneType )
					{
						// Animate model as MDL animation
						AnimateBoneMeshMDL ( pObject, pFrame );

						// complete early
						return true;
					}
				}
			}

			// failing MDL, Animate model as BONE animation (if not done in HW)
			if ( (pMesh->bShaderBoneSkinning==false && pMesh->dwForceCPUAnimationMode!=3) || pMesh->dwForceCPUAnimationMode==1 )
				AnimateBoneMeshBONE ( pObject, pFrame, pMesh );
		}
	}
	else
	{
		// Animate model as MDX animation
		AnimateBoneMeshMDX ( pObject, pFrame );
	}

	// return always animated
	return true;
}

DARKSDK_DLL bool AnimateBoneMesh ( sObject* pObject, sFrame* pFrame )
{
	// normal object-mesh bone animation
	return AnimateBoneMesh ( pObject, pFrame, pFrame->pMesh );
}

DARKSDK_DLL void ResetVertexDataInMeshPerMesh ( sMesh* pMesh )
{
	// valid mesh
	if ( !pMesh )
		return;

	// if original data available and same size, restore with it
	if ( pMesh->pOriginalVertexData )
	{
		DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
		memcpy ( pMesh->pVertexData, pMesh->pOriginalVertexData, dwTotalVertSize );
		pMesh->bVBRefreshRequired = true;
		g_vRefreshMeshList.push_back ( pMesh );
	}
}

DARKSDK_DLL void CollectOriginalVertexData ( sMesh* pMesh )
{
	#ifndef NEVERSTOREORIGINALVERTICES
	if ( pMesh->pOriginalVertexData==NULL )
	{
		// first time around, copy vertex data to original-store
		DWORD dwTotalVertSize = pMesh->dwVertexCount * pMesh->dwFVFSize;
		pMesh->pOriginalVertexData = (BYTE*)new char [ dwTotalVertSize ];
		memcpy ( pMesh->pOriginalVertexData, pMesh->pVertexData, dwTotalVertSize );
	}
	#endif
}

DARKSDK_DLL void ResetVertexDataInMesh ( sObject* pObject )
{
	// valid object
	if ( !pObject )
		return;

	// go through all meshes of object
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// get mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];
		ResetVertexDataInMeshPerMesh ( pMesh );
	}
}

DARKSDK_DLL void UpdateVertexDataInMesh ( sObject* pObject )
{
	// valid object
	if ( !pObject )
		return;

	// go through all meshes of object
	for ( int iMesh = 0; iMesh < pObject->iMeshCount; iMesh++ )
	{
		// get mesh ptr
		sMesh* pMesh = pObject->ppMeshList [ iMesh ];

		// delete old data and re-capture latest vertex data (new UV data changes mostly)
		SAFE_DELETE_ARRAY ( pMesh->pOriginalVertexData );
		CollectOriginalVertexData ( pMesh );
	}
}

// Mesh Shadow Meshes

//--------------------------------------------------------------------------------------
// Takes an array of CEdgeMapping objects, then returns an index for the edge in the
// table if such entry exists, or returns an index at which a new entry for the edge
// can be written.
// nV1 and nV2 are the vertex indexes for the old edge.
// nCount is the number of elements in the array.
// The function returns -1 if an available entry cannot be found.  In reality,
// this should never happens as we should have allocated enough memory.
int FindEdgeInMappingTable( int nV1, int nV2, CEdgeMapping *pMapping, int nCount )
{
    for( int i = 0; i < nCount; ++i )
    {
        // If both vertex indexes of the old edge in mapping entry are -1, then
        // we have searched every valid entry without finding a match.  Return
        // this index as a newly created entry.
        if( ( pMapping[i].m_anOldEdge[0] == -1 && pMapping[i].m_anOldEdge[1] == -1 ) ||

            // Or if we find a match, return the index.
            ( pMapping[i].m_anOldEdge[1] == nV1 && pMapping[i].m_anOldEdge[0] == nV2 ) )
        {
            return i;
        }
    }

    return -1;  // We should never reach this line
}

// Mesh Construction Functions

DARKSDK_DLL bool MakeMeshPlain ( bool bCreateNew, sMesh* pMesh, float fWidth, float fHeight, DWORD dwFVF, DWORD dwColor )
{
	// create memory
	DWORD dwVertexCount = 6;									// store number of vertices
	DWORD dwIndexCount  = 0;									// store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, dwVertexCount, dwIndexCount ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return false;
	}
	
	// leefix-210703-corrected UV coords of plain ( they where -1.0f, etc?? )
	// and DBPro rotates it Y=180 so it faces the camera (for compatibility and correct plain)
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  0,  -fWidth,  fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  1,   fWidth,  fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  2,  -fWidth, -fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 1.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  3,   fWidth,  fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  4,   fWidth, -fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 1.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  5,  -fWidth, -fHeight, 0.0f,  0.0f,  0.0f,  1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 1.0f );

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = 2;

	// okay
	return true;
}

// mike - 021005 - alternative make plane function
DARKSDK_DLL bool MakeMeshPlainEx ( bool bCreateNew, sMesh* pMesh, float fWidth, float fHeight, DWORD dwFVF, DWORD dwColor )
{
	// create memory
	DWORD dwVertexCount = 6;									// store number of vertices
	DWORD dwIndexCount  = 0;									// store number of indices
	if ( !SetupMeshFVFData ( pMesh, GGFVF_XYZ | GGFVF_NORMAL | GGFVF_TEX1, dwVertexCount, dwIndexCount ) )
	{
		RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
		return false;
	}

	float pos_pos_x = ( fWidth );
	float neg_pos_x = 0 - ( fWidth );
	float pos_pos_y = ( fHeight );
	float neg_pos_y = 0 - ( fHeight );

	
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  0,  neg_pos_x,  pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  1,   pos_pos_x,  pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 0.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  2,  pos_pos_x, neg_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 1.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  3,   pos_pos_x,  neg_pos_y, 0.0f,  0.0f,  0.0f, -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f, 1.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  4,   neg_pos_x, neg_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 1.0f );
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  5,   neg_pos_x, pos_pos_y, 0.0f,  0.0f,  0.0f,  -1.0f, GGCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 0.0f );

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = 2;

	// okay
	return true;
}

DARKSDK_DLL bool MakeMeshBox ( bool bCreateNew, sMesh* pMesh, float fWidth1, float fHeight1, float fDepth1, float fWidth2, float fHeight2, float fDepth2, DWORD dwFVF, DWORD dwColor )
{
	// create vertex memory for box
	DWORD dwVertexCount = 24;
	DWORD dwIndexCount  = 36;
	if ( bCreateNew )
	{
		if ( !SetupMeshFVFData ( pMesh, dwFVF, dwVertexCount, dwIndexCount ) )
		{
			RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
			return false;
		}
	}

	// leefix - 080403 - adjusted UV data use accurate coordinates (slight texture alignment can now be done usinfg vertexdata commands)
	// setup the vertices, we're using a standard FVF here so call the standard utility function
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  0, fWidth1, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 0.00f );	// front
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  1, fWidth2, fHeight2, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  2, fWidth2, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  3, fWidth1, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  4, fWidth1, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 0.00f );	// back
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  5, fWidth1, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  6, fWidth2, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  7, fWidth2, fHeight2, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  8, fWidth1, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 0.00f );	// top
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  9, fWidth2, fHeight2, fDepth2,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 10, fWidth2, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 11, fWidth1, fHeight2, fDepth1,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 12, fWidth1, fHeight1, fDepth2,  0.0f, -1.0f,  0.0f, dwColor, 0.00f, 1.00f );	// bottom
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 13, fWidth1, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 0.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 14, fWidth2, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 15, fWidth2, fHeight1, fDepth2,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 16, fWidth2, fHeight2, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );	// right
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 17, fWidth2, fHeight2, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 18, fWidth2, fHeight1, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 19, fWidth2, fHeight1, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 20, fWidth1, fHeight2, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );	// left
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 21, fWidth1, fHeight1, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 22, fWidth1, fHeight1, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 23, fWidth1, fHeight2, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );
	
	// and now fill in the index list
	pMesh->pIndices [  0 ] =  0;		pMesh->pIndices [  1 ] =  1;		pMesh->pIndices [  2 ] =  2;
	pMesh->pIndices [  3 ] =  2;		pMesh->pIndices [  4 ] =  3;		pMesh->pIndices [  5 ] =  0;
	pMesh->pIndices [  6 ] =  4;		pMesh->pIndices [  7 ] =  5;		pMesh->pIndices [  8 ] =  6;
	pMesh->pIndices [  9 ] =  6;		pMesh->pIndices [ 10 ] =  7;		pMesh->pIndices [ 11 ] =  4;
	pMesh->pIndices [ 12 ] =  8;		pMesh->pIndices [ 13 ] =  9;		pMesh->pIndices [ 14 ] = 10;
	pMesh->pIndices [ 15 ] = 10;		pMesh->pIndices [ 16 ] = 11;		pMesh->pIndices [ 17 ] =  8;
	pMesh->pIndices [ 18 ] = 12;		pMesh->pIndices [ 19 ] = 13;		pMesh->pIndices [ 20 ] = 14;
	pMesh->pIndices [ 21 ] = 14;		pMesh->pIndices [ 22 ] = 15;		pMesh->pIndices [ 23 ] = 12;
	pMesh->pIndices [ 24 ] = 16;		pMesh->pIndices [ 25 ] = 17;		pMesh->pIndices [ 26 ] = 18;
	pMesh->pIndices [ 27 ] = 18;		pMesh->pIndices [ 28 ] = 19;		pMesh->pIndices [ 29 ] = 16;
	pMesh->pIndices [ 30 ] = 20;		pMesh->pIndices [ 31 ] = 21;		pMesh->pIndices [ 32 ] = 22;
	pMesh->pIndices [ 33 ] = 22;		pMesh->pIndices [ 34 ] = 23;		pMesh->pIndices [ 35 ] = 20;
	
	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = pMesh->dwIndexCount  / 3;

	// okay
	return true;
}

DARKSDK_DLL bool MakeMeshPyramid ( bool bCreateNew, sMesh* pMesh, float fSize, DWORD dwFVF, DWORD dwColor )
{
	// create vertex memory for box
	DWORD dwVertexCount = 24;
	DWORD dwIndexCount  = 36;
	if ( bCreateNew )
	{
		if ( !SetupMeshFVFData ( pMesh, dwFVF, dwVertexCount, dwIndexCount ) )
		{
			RunTimeError ( RUNTIMEERROR_B3DMESHLOADFAILED );
			return false;
		}
	}

	// calculate some dimensions
	float fWidth1 = -fSize;
	float fWidth2 = fSize;
	float fHeight1 = -fSize;
	float fHeight2 = fSize;
	float fDepth1 = -fSize;
	float fDepth2 = fSize;

	// generate vrtex data
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  0, 0, fHeight2, 0,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 0.00f );	// front
	SetupStandardVertex ( pMesh->dwFVF, pMesh->pVertexData,  1, 0, fHeight2, 0,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  2, fWidth2, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  3, fWidth1, fHeight1, fDepth1,  0.0f,  0.0f, -1.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  4, 0, fHeight2, 0,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 0.00f );	// back
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  5, fWidth1, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  6, fWidth2, fHeight1, fDepth2,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  7, 0, fHeight2, 0,  0.0f,  0.0f,  1.0f, dwColor, 0.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  8, 0, fHeight2, 0,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 0.00f );	// top
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData,  9, 0, fHeight2, 0,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 10, 0, fHeight2, 0,	 0.0f,  1.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 11, 0, fHeight2, 0,	 0.0f,  1.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 12, fWidth1, fHeight1, fDepth2,  0.0f, -1.0f,  0.0f, dwColor, 0.00f, 1.00f );	// bottom
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 13, fWidth1, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 0.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 14, fWidth2, fHeight1, fDepth1,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 15, fWidth2, fHeight1, fDepth2,	 0.0f, -1.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 16, 0, fHeight2, 0,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );	// right
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 17, 0, fHeight2, 0,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 18, fWidth2, fHeight1, fDepth2,	 1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 19, fWidth2, fHeight1, fDepth1,	 1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 20, 0, fHeight2, 0,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 0.00f );	// left
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 21, fWidth1, fHeight1, fDepth1,	-1.0f,  0.0f,  0.0f, dwColor, 1.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 22, fWidth1, fHeight1, fDepth2,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 1.00f );
	SetupStandardVertex ( pMesh->dwFVF,	pMesh->pVertexData, 23, 0, fHeight2, 0,	-1.0f,  0.0f,  0.0f, dwColor, 0.00f, 0.00f );
	
	// and now fill in the index list
	pMesh->pIndices [  0 ] =  0;		pMesh->pIndices [  1 ] =  1;		pMesh->pIndices [  2 ] =  2;
	pMesh->pIndices [  3 ] =  2;		pMesh->pIndices [  4 ] =  3;		pMesh->pIndices [  5 ] =  0;
	pMesh->pIndices [  6 ] =  4;		pMesh->pIndices [  7 ] =  5;		pMesh->pIndices [  8 ] =  6;
	pMesh->pIndices [  9 ] =  6;		pMesh->pIndices [ 10 ] =  7;		pMesh->pIndices [ 11 ] =  4;
	pMesh->pIndices [ 12 ] =  8;		pMesh->pIndices [ 13 ] =  9;		pMesh->pIndices [ 14 ] = 10;
	pMesh->pIndices [ 15 ] = 10;		pMesh->pIndices [ 16 ] = 11;		pMesh->pIndices [ 17 ] =  8;
	pMesh->pIndices [ 18 ] = 12;		pMesh->pIndices [ 19 ] = 13;		pMesh->pIndices [ 20 ] = 14;
	pMesh->pIndices [ 21 ] = 14;		pMesh->pIndices [ 22 ] = 15;		pMesh->pIndices [ 23 ] = 12;
	pMesh->pIndices [ 24 ] = 16;		pMesh->pIndices [ 25 ] = 17;		pMesh->pIndices [ 26 ] = 18;
	pMesh->pIndices [ 27 ] = 18;		pMesh->pIndices [ 28 ] = 19;		pMesh->pIndices [ 29 ] = 16;
	pMesh->pIndices [ 30 ] = 20;		pMesh->pIndices [ 31 ] = 21;		pMesh->pIndices [ 32 ] = 22;
	pMesh->pIndices [ 33 ] = 22;		pMesh->pIndices [ 34 ] = 23;		pMesh->pIndices [ 35 ] = 20;
	
	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLELIST;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = pMesh->dwIndexCount  / 3;

	// okay
	return true;
}

DARKSDK_DLL bool MakeMeshSphere ( bool bCreateNew, sMesh* pMesh, GGVECTOR3 vecCentre, float fRadius, int iRings, int iSegments, DWORD dwFVF, DWORD dwColor )
{
	// create sphere mesh
	DWORD dwIndexCount		= 2 * iRings * ( iSegments + 1 );
	DWORD dwVertexCount		= ( iRings + 1 ) * ( iSegments + 1 );
	if ( bCreateNew )
	{
		if ( !SetupMeshFVFData ( pMesh, dwFVF, dwVertexCount, dwIndexCount ) )
		{
			RunTimeError ( RUNTIMEERROR_B3DMEMORYERROR );
			return false;
		}
	}

	// now we can fill in the vertex and index data to form the mesh
	float		fDeltaRingAngle		= ( GG_PI / iRings );
	float		fDeltaSegAngle		= ( 2.0f * GG_PI / iSegments );
	int			iVertex				= 0;
	int			iIndex				= 0;
	WORD		wVertexIndex		= 0;
	GGVECTOR3 vNormal;

	// generate the group of rings for the sphere
	for ( int iCurrentRing = 0; iCurrentRing < iRings + 1; iCurrentRing++ )
	{
		float r0 = sinf ( iCurrentRing * fDeltaRingAngle );
		float y0 = cosf ( iCurrentRing * fDeltaRingAngle );

		// generate the group of segments for the current ring
		for ( int iCurrentSegment = 0; iCurrentSegment < iSegments + 1; iCurrentSegment++ )
		{
			float x0 = r0 * sinf ( iCurrentSegment * fDeltaSegAngle );
			float z0 = r0 * cosf ( iCurrentSegment * fDeltaSegAngle );

			vNormal.x = x0;
			vNormal.y = y0;
			vNormal.z = z0;
	
			GGVec3Normalize ( &vNormal, &vNormal );

			// add one vertex to the strip which makes up the sphere
			SetupStandardVertex (	pMesh->dwFVF, pMesh->pVertexData,  iVertex,
									vecCentre.x+(x0*fRadius), vecCentre.y+(y0*fRadius), vecCentre.z+(z0*fRadius), 
									vNormal.x, vNormal.y, vNormal.z,
									dwColor,
									1.0f - ( ( float ) iCurrentSegment / ( float ) iSegments ),
									( float ) iCurrentRing / ( float ) iRings );

			// increment vertex
			iVertex++;
			
			// add two indices except for the last ring 
			if ( iCurrentRing != iRings )
			{
				pMesh->pIndices [ iIndex ] = wVertexIndex;
				iIndex++;
				
				pMesh->pIndices [ iIndex ] = wVertexIndex + ( WORD ) ( iSegments + 1 ); 
				iIndex++;
				
				wVertexIndex++; 
			}
		}
	}

	// setup mesh drawing properties
	pMesh->iPrimitiveType   = GGPT_TRIANGLESTRIP;
	pMesh->iDrawVertexCount = pMesh->dwVertexCount;
	pMesh->iDrawPrimitives  = pMesh->dwIndexCount - 2;

	// okay
	return true;
}

DARKSDK_DLL bool MakeMeshFromOtherMesh ( bool bCreateNew, sMesh* pMesh, sMesh* pOtherMesh, GGMATRIX* pmatWorld, DWORD dwIndexCount, DWORD dwVertexCount )
{
	// make new mesh from existing other mesh
	MakeLocalMeshFromOtherLocalMesh ( pMesh, pOtherMesh, dwIndexCount, dwVertexCount );

	// get the offset map for the FVF
	sOffsetMap offsetMap;
	GetFVFOffsetMap ( pMesh, &offsetMap );

	// copy vertex data from mesh to single-mesh
	BYTE* pDestVertexData = NULL;
	BYTE* pDestNormalData = NULL;

	// make sure we have data and ptrs
	if ( offsetMap.dwZ>0 )
	{
		pDestVertexData = (BYTE*)((float*)(pMesh->pVertexData));
	}
	if ( offsetMap.dwNZ>0 )
	{
		pDestNormalData = (BYTE*)((float*)pMesh->pVertexData + offsetMap.dwNX);
	}

	// establish world matrix if any
	GGMATRIX matNoTransWorld;
	if ( *pmatWorld )
		matNoTransWorld = *pmatWorld;
	else
		GGMatrixIdentity ( &matNoTransWorld );

	// remove all translation from world matrix
	matNoTransWorld._41 = 0.0f;
	matNoTransWorld._42 = 0.0f;
	matNoTransWorld._43 = 0.0f;

	// transform vertex data by world matrix of frame
	for ( DWORD v=0; v<pOtherMesh->dwVertexCount; v++ )
	{
		// handle vertex data
		if ( pDestVertexData )
		{
			GGVECTOR3* pVertex = (GGVECTOR3*)(pDestVertexData+(v*pMesh->dwFVFSize));
			GGVec3TransformCoord ( pVertex, pVertex, &matNoTransWorld );
		}

		// handle normals data
		if ( pDestNormalData )
		{
			GGVECTOR3* pNormal = (GGVECTOR3*)(pDestNormalData+(v*pMesh->dwFVFSize));
			GGVec3TransformNormal ( pNormal, pNormal, &matNoTransWorld );
			GGVec3Normalize ( pNormal, pNormal );
		}
	}

	// 151003 - add computation of bound box in collision structure (for limb collision)
	CalculateMeshBounds ( pMesh );

	// okay
	return true;
}

DARKSDK_DLL bool MakeMeshFromOtherMesh ( bool bCreateNew, sMesh* pMesh, sMesh* pOtherMesh, GGMATRIX* pmatWorld )
{
	DWORD dwIndexCount = pOtherMesh->dwIndexCount;
	DWORD dwVertexCount = pOtherMesh->dwVertexCount;
	return MakeMeshFromOtherMesh ( bCreateNew, pMesh, pOtherMesh, pmatWorld, dwIndexCount, dwVertexCount );
}

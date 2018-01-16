//THIS CODE WAS BORROWED FROM DBPRO 7.5 AND MODIFIED TO WORK WITH BLITZTERRAIN

#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "globstruct.h"
#include "main.h"

extern GlobStruct* g_pGlob;
extern IGGDevice* m_pD3D;

bool DBPRO_SetMeshRenderStates( sMesh* pMesh )
{
	#ifdef DX11

	// terrain render states

	#else

//Get D3D Device
	IGGDevice* D3DDevice = m_pD3D;

//Wireframe
	if ( pMesh->bWireframe )
		D3DDevice->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		D3DDevice->SetRenderState ( D3DRS_FILLMODE, D3DFILL_SOLID );

//Lighting
	if ( pMesh->bLight )
		D3DDevice->SetRenderState ( D3DRS_LIGHTING, TRUE );
	else
		D3DDevice->SetRenderState ( D3DRS_LIGHTING, FALSE );

//Culling
	if ( pMesh->bCull )
		D3DDevice->SetRenderState ( D3DRS_CULLMODE, GGCULL_CCW );
	else	
		D3DDevice->SetRenderState ( D3DRS_CULLMODE, D3DCULL_NONE );

//Fog
	if(g_pGlob)
	{
		if(g_pGlob->iFogState==1)
		{
			if ( pMesh->bFog )
				D3DDevice->SetRenderState ( D3DRS_FOGENABLE, TRUE );
			else	
				D3DDevice->SetRenderState ( D3DRS_FOGENABLE, FALSE );

			// ghosts in fog must override fog color part (fog override)
			if ( pMesh->bFog && pMesh->bGhost )
				D3DDevice->SetRenderState ( D3DRS_FOGCOLOR, GGCOLOR_RGBA ( 0, 0, 0, 0 ) );
		}
	}

	// leefix - 070204 - introduced for better Zwrite control
	bool bCorrectZWriteState = pMesh->bZWrite;

//Transparency
	bool bDoGhostAgain = false;
	if ( pMesh->bTransparency==true )
	{
		if ( pMesh->bTransparency )
		{
			D3DDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE,	true );
			D3DDevice->SetRenderState ( D3DRS_SRCBLEND,			D3DBLEND_SRCALPHA );
			D3DDevice->SetRenderState ( D3DRS_DESTBLEND,			D3DBLEND_INVSRCALPHA );
			D3DDevice->SetRenderState ( D3DRS_ALPHATESTENABLE,		true );
			
			// mike - 020904 - use this for alpha testing - do not get edges anymore
			// lee - 240903 - need full range of alpha rendered, not just the upper band
			DWORD dwuseAlphaTestValue = pMesh->dwAlphaTestValue;

			if ( dwuseAlphaTestValue==0 )
			{
				D3DDevice->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATER );
				D3DDevice->SetRenderState ( D3DRS_ALPHAREF,	(DWORD)0x00000000 );
			}
			else
			{
				// leeadd - 131205 - let SetAlphaMappingOn command scale the alpha-test to let semi-transparent pixel through
				if ( pMesh->bAlphaOverride==true )
				{
					// alpha mapping percentage vased alpha test
					DWORD dwPercAlpha = ( (pMesh->dwAlphaOverride & 0xFF000000) >> 24 ) ;
					float perc = (float)dwPercAlpha / 255.0f;
					// leefix - 071208 - alpha test transition not perfect as go from override to 0xCF based alpha, so cap it
					// DWORD dwAlphaLevelToDraw = (DWORD)(128.0f * perc);
					DWORD dwAlphaLevelToDraw = (DWORD)(255 * perc);
					dwuseAlphaTestValue = dwAlphaLevelToDraw;
					if ( dwuseAlphaTestValue > (DWORD)0x000000CF ) dwuseAlphaTestValue=(DWORD)0x000000CF;
					D3DDevice->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATEREQUAL );
				}
				else
				{
					// regular alpha test
					dwuseAlphaTestValue=(DWORD)0x000000CF;
					D3DDevice->SetRenderState ( D3DRS_ALPHAFUNC,	D3DCMP_GREATEREQUAL );
				}
				D3DDevice->SetRenderState ( D3DRS_ALPHAREF,	dwuseAlphaTestValue );
			}
		}
		else
		{
			D3DDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE,	false );
			D3DDevice->SetRenderState ( D3DRS_ALPHATESTENABLE,		false );
			D3DDevice->SetRenderState ( D3DRS_ALPHAFUNC,			D3DCMP_ALWAYS );
		}

		// now must do ghost again - to combine with blend settings
		bDoGhostAgain = true;
	}

//Ghosting
	if ( bDoGhostAgain==true )
	{
		if ( pMesh->bGhost )
		{
			D3DDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE, true );
			switch ( pMesh->iGhostMode )
			{
				case 0:
				{
					D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
					D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR );
				}
				break;

				case 1:
				{
					// lee - 220306 - u6b4 - direct from best of DBC (darkghostmode7)
					DWORD dwDarkAlphaSourceBlend = D3DBLEND_ZERO;
					DWORD dwDarkAlphaDestinationBlend = D3DBLEND_SRCCOLOR;
					D3DDevice->SetRenderState( D3DRS_SRCBLEND,  dwDarkAlphaSourceBlend );
					D3DDevice->SetRenderState( D3DRS_DESTBLEND, dwDarkAlphaDestinationBlend );
				}
				break;

				case 2:
				{
					D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
					D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
				}
				break;

				case 3:
				{
					D3DDevice->SetRenderState ( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
					D3DDevice->SetRenderState ( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
				}
				break;

				case 4:
				{
					D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
					D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR );
				}
				break;

				case 5:
				{
					// leeadd - 210806 - replace OLD-MODE-1 (used in FPSC) for Scorch Texture Multiply
					D3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR );
					D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
				}
				break;
			}
			bCorrectZWriteState = false;
		}
		else
		{
			// no ghost and no transparency, end alpha blend effect
			if ( pMesh->bTransparency==false)
			{
				D3DDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE, FALSE );
			}
		}

	}

//Z Write
	if ( pMesh->bZWrite )
		D3DDevice->SetRenderState ( D3DRS_ZWRITEENABLE,		TRUE );
	else
		D3DDevice->SetRenderState ( D3DRS_ZWRITEENABLE,		FALSE );

//Z Bias
	if ( pMesh->bZBiasActive )
	{
		D3DDevice->SetRenderState ( D3DRS_DEPTHBIAS,			*(DWORD*)&pMesh->fZBiasDepth );
		D3DDevice->SetRenderState ( D3DRS_SLOPESCALEDEPTHBIAS,	*(DWORD*)&pMesh->fZBiasSlopeScale );
	}
	else
	{
		D3DDevice->SetRenderState ( D3DRS_DEPTHBIAS,			0 );
		D3DDevice->SetRenderState ( D3DRS_SLOPESCALEDEPTHBIAS,	0 );
	}

//Z Read
	if ( pMesh->bZRead )
		D3DDevice->SetRenderState ( D3DRS_ZENABLE,		TRUE );
	else
		D3DDevice->SetRenderState ( D3DRS_ZENABLE,		FALSE );

//Material
	if ( pMesh->bUsesMaterial )
	{
		// use diffuse from material (if present)
		D3DDevice->SetRenderState ( D3DRS_COLORVERTEX,					FALSE );
		D3DDevice->SetRenderState ( D3DRS_DIFFUSEMATERIALSOURCE,		D3DMCS_MATERIAL );

		// set the material from the mesh
		if ( FAILED ( D3DDevice->SetMaterial ( &pMesh->mMaterial ) ) )
			return false;
	}

	// mike - 230505 - need to be able to set mip map LOD bias on a per mesh basis
	D3DDevice->SetSamplerState ( 0, D3DSAMP_MIPMAPLODBIAS, *( ( LPDWORD ) ( &pMesh->fMipMapLODBias ) ) );

	#endif

	// success
	return true;
}

void DBPRO_ApplyEffect ( sMesh* pMesh, tagCameraData* m_Camera_Ptr )
{
	#ifdef DX11
	// no need, should be using shadereffect START function to call this!
	#else
	// This MAY be redundant IF we can somehow set all terrain shader constants
	// at the start of the pass and not make any more effect commits

//Get D3D Device
	IGGDevice* D3DDevice = m_pD3D;

//Get effect
	cSpecialEffect* Effect=pMesh->pVertexShaderEffect;

	// Gather and calculate required constants data
	GGMATRIX matWorld,matView,matProj;
    D3DDevice->GetTransform( GGTS_WORLD, &matWorld );
    D3DDevice->GetTransform( GGTS_VIEW, &matView );
    D3DDevice->GetTransform( GGTS_PROJECTION, &matProj );

	GGMATRIX matWorldView,matViewProj,matWorldViewProj;
    matWorldView = matWorld * matView;
    matViewProj = matView * matProj;
    matWorldViewProj = matWorld * matView * matProj;
 
	// Calculate inverse matrices
	GGMATRIX matWorldInv,matViewInv,matWorldViewInv;
	GGMatrixInverse( &matWorldInv, NULL, &matWorld );
	GGMatrixInverse( &matViewInv, NULL, &matView );
	GGMatrixInverse( &matWorldViewInv, NULL, &matWorldView );

	// Get raw light data
	D3DLIGHT9 d3dLight;
	D3DDevice->GetLight(0,&d3dLight);
	GGVECTOR4 vecLightDir,vecLightPos,vecLightDirInv;

	if ( d3dLight.Type==GGLIGHT_DIRECTIONAL )
	{
		// get direction directly from structure
	    vecLightDir = GGVECTOR4( d3dLight.Direction.x, d3dLight.Direction.y, d3dLight.Direction.z, 0.0f );
	    vecLightPos = GGVECTOR4( d3dLight.Direction.x*-1000.0f, d3dLight.Direction.y*-1000.0f, d3dLight.Direction.z*-1000.0f, 1.0f );
	}
	else
	{
		// calculate direction from world position
		vecLightDir.x = matWorld._41-d3dLight.Position.x;
		vecLightDir.y = matWorld._42-d3dLight.Position.y;
		vecLightDir.z = matWorld._43-d3dLight.Position.z;
		vecLightDir.w = 1.0f;

		// calculate light position (in object space)
	    vecLightPos.x = d3dLight.Position.x;
	    vecLightPos.y = d3dLight.Position.y;
	    vecLightPos.z = d3dLight.Position.z;
	    vecLightPos.w = 1.0f;
	}

	// Calculate light and object-space light(inv)
	GGVec4Transform ( &vecLightDirInv, &vecLightDir, &matWorldInv );
	D3DXVec4Normalize ( &vecLightDirInv, &vecLightDirInv );
	D3DXVec4Normalize ( &vecLightDir, &vecLightDir );

	// Get camera psition
	GGVECTOR4 vecCameraPosition;
    vecCameraPosition = GGVECTOR4( matViewInv._41, matViewInv._42, matViewInv._43, 1.0f );

	// Alpha override component
	if ( Effect->m_AlphaOverrideHandle )
	{
		if ( pMesh->bAlphaOverride==true )
		{
			float fPercentage = (float)(pMesh->dwAlphaOverride>>24)/255.0f;
			Effect->GGSetEffectFloat( Effect->m_AlphaOverrideHandle, fPercentage );
		}
	}

	// prepare tranposed matrices for column major matrices
	if ( Effect->m_bTranposeToggle )
	{
		GGMatrixTranspose( &matWorld, &matWorld );
		GGMatrixTranspose( &matView, &matView );
		GGMatrixTranspose( &matProj, &matProj );
		GGMatrixTranspose( &matWorldView, &matWorldView );
		GGMatrixTranspose( &matViewProj, &matViewProj );
		GGMatrixTranspose( &matWorldViewProj, &matWorldViewProj );
		GGMatrixTranspose( &matWorldInv, &matWorldInv );
		GGMatrixTranspose( &matViewInv, &matViewInv );
		GGMatrixTranspose( &matWorldViewInv, &matWorldViewInv );
	}

	// leeadd - 290104 - addition of tranposed matrices for effects that use them
	GGMATRIX matWorldT,matViewT,matProjT,matWorldViewT,matViewProjT,matWorldViewProjT,
				matWorldInvT,matViewInvT,matWorldViewInvT;
	GGMatrixTranspose( &matWorldT, &matWorld );
	GGMatrixTranspose( &matViewT, &matView );
	GGMatrixTranspose( &matProjT, &matProj );
	GGMatrixTranspose( &matWorldViewT, &matWorldView );
	GGMatrixTranspose( &matViewProjT, &matViewProj );
	GGMatrixTranspose( &matWorldViewProjT, &matWorldViewProj );
	GGMatrixTranspose( &matWorldInvT, &matWorldInv );
	GGMatrixTranspose( &matViewInvT, &matViewInv );
	GGMatrixTranspose( &matWorldViewInvT, &matWorldViewInv );

	// apply latest data to effect
    if( Effect->m_pEffect != NULL )
    {
		// main matrices (row major)
        if( Effect->m_MatWorldEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatWorldEffectHandle, &matWorld );
        }
        if( Effect->m_MatViewEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatViewEffectHandle, &matView );
        }
        if( Effect->m_MatProjEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatProjEffectHandle, &matProj );
        }
        if( Effect->m_MatWorldViewEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewEffectHandle, &matWorldView );
        }
        if( Effect->m_MatViewProjEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatViewProjEffectHandle, &matViewProj );
        }
        if( Effect->m_MatWorldViewProjEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewProjEffectHandle, &matWorldViewProj );
        }
        if( Effect->m_MatWorldInverseEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldInverseEffectHandle, &matWorldInv );
        }		
        if( Effect->m_MatViewInverseEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatViewInverseEffectHandle, &matViewInv );
        }		
        if( Effect->m_MatProjectionInverseEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatProjectionInverseEffectHandle, &matProjInv );
        }		
        if( Effect->m_MatWorldViewInverseEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewInverseEffectHandle, &matWorldViewInv );
        }

		// tranposed matrices (column major)
        if( Effect->m_MatWorldTEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatWorldTEffectHandle, &matWorldT );
        }
        if( Effect->m_MatViewTEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatViewTEffectHandle, &matViewT );
        }
        if( Effect->m_MatProjTEffectHandle != NULL )
		{
            Effect->GGSetEffectMatrix( Effect->m_MatProjTEffectHandle, &matProjT );
        }
        if( Effect->m_MatWorldViewTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewTEffectHandle, &matWorldViewT );
        }
        if( Effect->m_MatViewProjTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatViewProjTEffectHandle, &matViewProjT );
        }
        if( Effect->m_MatWorldViewProjTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewProjTEffectHandle, &matWorldViewProjT );
        }
        if( Effect->m_MatWorldInverseTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldInverseTEffectHandle, &matWorldInvT );
        }		
        if( Effect->m_MatViewInverseTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatViewInverseTEffectHandle, &matViewInvT );
        }		
        if( Effect->m_MatWorldViewInverseTEffectHandle != NULL )
        {
            Effect->GGSetEffectMatrix( Effect->m_MatWorldViewInverseTEffectHandle, &matWorldViewInvT );
        }

		// main vectors
		if ( Effect->m_LightDirHandle != NULL )
		{
            Effect->GGSetEffectVector( Effect->m_LightDirHandle, &vecLightDir );
		}
		if ( Effect->m_LightDirInvHandle != NULL )
		{
            Effect->GGSetEffectVector( Effect->m_LightDirInvHandle, &vecLightDirInv );
		}
		if ( Effect->m_LightPosHandle != NULL )
		{
            Effect->GGSetEffectVector( Effect->m_LightPosHandle, &vecLightPos );
		}
        if( Effect->m_VecCameraPosEffectHandle != NULL )
        {
            Effect->GGSetEffectVector( Effect->m_VecCameraPosEffectHandle, &vecCameraPosition );
        }
		if ( Effect->m_VecEyePosEffectHandle != NULL )
		{
            Effect->GGSetEffectVector( Effect->m_VecEyePosEffectHandle, &vecCameraPosition );
		}

		// misclanious values
		if( Effect->m_TimeEffectHandle != NULL )
		{
			// TIME DATA IN SECONDS
			float fTime =0.0f;
//			float fTime = (float)(timeGetTime() / 1000.0);
			Effect->GGSetEffectFloat( Effect->m_TimeEffectHandle, fTime );
		}
		if( Effect->m_SinTimeEffectHandle != NULL )
		{
			// TIME DATA IN SECONDS
			float fSinTime =0.0f;
//			float fSinTime = (float)sin((timeGetTime() / 1000.0));
			Effect->GGSetEffectFloat( Effect->m_SinTimeEffectHandle, fSinTime );
		}

		// leelee - 290713 - need to adjust clipping plane if shader used (clip space not world space plane)
		GGPLANE planeUse = m_Camera_Ptr->planeClip;
		// called before camera (m_Camera_Ptr) renders
		if ( m_Camera_Ptr->iClipPlaneOn!=0 )
		{
			// they should be in clip space, not world space
			bool bClipSpaceRequired = false;
			int iActualClipPlaneOn = m_Camera_Ptr->iClipPlaneOn;
			if ( m_Camera_Ptr->iClipPlaneOn==3 ) { bClipSpaceRequired=true; iActualClipPlaneOn=1; }
			if ( m_Camera_Ptr->iClipPlaneOn==4 ) { bClipSpaceRequired=true; iActualClipPlaneOn=2; }

			// transform world to clip space
			if ( bClipSpaceRequired )
			{
				// temp planes for conversion
				GGPLANE tempPlane = planeUse;
				GGPLANE viewSpacePlane;

				// normalize the plane which is required for the transforms
				GGPlaneNormalize(&tempPlane, &tempPlane);

				// transform the plane into view space
				GGMATRIX tempMatrix = m_Camera_Ptr->matView;
				GGMatrixInverse(&tempMatrix, NULL, &tempMatrix);
				GGMatrixTranspose(&tempMatrix, &tempMatrix);
				GGPlaneTransform(&viewSpacePlane, &tempPlane, &tempMatrix);

				// transform the plane into clip space, or post projection space
				tempMatrix = m_Camera_Ptr->matProjection;
				GGMatrixInverse(&tempMatrix, NULL, &tempMatrix);
				GGMatrixTranspose(&tempMatrix, &tempMatrix);

				// place resulting clip space plane ready for setclipplane
				GGPlaneTransform(&planeUse, &viewSpacePlane, &tempMatrix);
			}

			// clipping plane enabled
			D3DDevice->SetClipPlane ( 0, (float*)planeUse );
			D3DDevice->SetRenderState ( D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0 );
		}
		else
		{
			D3DDevice->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
		}

		// leelee - 290713 - pass clipping data to shader (automatic)
		if ( Effect->m_VecClipPlaneEffectHandle )
		{
			if ( m_Camera_Ptr->iClipPlaneOn>0 )
			{
				// special mode which creates plane but does not use RenderState to set clip
				// as you cannot mix FF clip and HLSL clip in same scene (artefacts)
				GGVECTOR4 vec = (GGVECTOR4)m_Camera_Ptr->planeClip;
				Effect->GGSetEffectVector( Effect->m_VecClipPlaneEffectHandle, &vec );
			}
			else
			{
				// ensure shader stops using clip plane when not being clipped!
				GGVECTOR4 vec = GGVECTOR4( 0.0f, 1.0f, 0.0f, 99999.0f );
				Effect->GGSetEffectVector( Effect->m_VecClipPlaneEffectHandle, &vec );
			}
		}


	//Commit changes
		D3DDevice->SetRenderState ( D3DRS_CLIPPLANEENABLE, 0x00 );
		Effect->m_pEffect->CommitChanges();

    }
	#endif
}

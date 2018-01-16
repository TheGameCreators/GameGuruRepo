//
// Shadow Mapping Control Object
//

// Include
#include "cOcclusion.h"

// Occlusion object global
COcclusion g_Occlusion;

// Implementation
COcclusion::COcclusion(void)
{
	// default no occlusion
	m_OcclusionMode = 0;

	// HOQ occlusion technique
	for ( int i=0; i<70000; i++ )
	{
		d3dQuery[i] = NULL;
		iQueryBusyStage[i] = 0;
	}
	
	// general statistics
	m_iOccluded = 0;
}
COcclusion::~COcclusion()
{
	// HOQ free usages
	#ifdef DX11
	#else
	for ( int i=0; i<70000; i++ )
		SAFE_RELEASE ( d3dQuery[i] );
	#endif
}

void COcclusion::SetOcclusionMode ( int iOcclusionMode ) 
{ 
	// set value
	m_OcclusionMode = iOcclusionMode;
}

void COcclusion::StartOcclusionStep ( void )
{
	// reset occlusion counter
	m_iOccluded = 0;
}

// HOQ
void COcclusion::RenderOcclusionObject ( sObject* pObject )
{
	// render basic bound shape (not actual mesh)
	sObject* pActualObject = pObject;
	if ( pObject->pInstanceOfObject ) pActualObject = pObject->pInstanceOfObject;
	if ( pActualObject->ppMeshList )
	{
		if ( pActualObject->ppMeshList[0] )
		{
			if ( pActualObject->ppMeshList[0] )
			{
				// bound mesh to render
				sMesh* pMesh = pActualObject->ppMeshList[0];

				#ifdef DX11
				#else
				// set world position to place object in world
				GGSetTransform ( GGTS_WORLD, ( GGMATRIX* ) &pObject->position.matWorld );

				// only render to depth for test (performance friendly render)
				pActualObject->ppMeshList[0]->bZWrite = false;
				m_pD3D->SetRenderState ( D3DRS_COLORWRITEENABLE, 0 );

				// render mesh as bound mesh
				m_ObjectManager.DrawMesh ( pMesh );
				m_iOccluded++;

				// counter the count as it's not a real rendered obe (just a depth one)
				if ( g_pGlob ) g_pGlob->dwNumberOfPolygonsDrawn -= pMesh->pDrawBuffer->dwPrimitiveCount;
				if ( g_pGlob ) g_pGlob->dwNumberOfPrimCalls--;

				// only render to depth for test (performance friendly render) - restore 
				m_pD3D->SetRenderState ( D3DRS_COLORWRITEENABLE, 0x0000000F );
				pActualObject->ppMeshList[0]->bZWrite = true;
				#endif
			}
		}
	}
}

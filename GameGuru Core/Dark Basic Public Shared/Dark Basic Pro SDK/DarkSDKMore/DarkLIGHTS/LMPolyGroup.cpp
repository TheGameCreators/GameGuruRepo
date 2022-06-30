#include "LMPolyGroup.h"
#include "LMPoly.h"
#include "Lumel.h"
#include "CollisionTreeLightmapper.h"
#include "Light.h"
#include "LMTexture.h"
#include "TreeFaceLightmapper.h"

extern HANDLE g_hLMHeap;

int LMPolyGroup::iPixelBorder = 2;
int LMPolyGroup::iMode = 0;
float LMPolyGroup::fShadowPower = 1;
Point* LMPolyGroup::pRandomPoints = 0;
float* LMPolyGroup::pRandomDist = 0;
int LMPolyGroup::iIterations = 256;
float LMPolyGroup::fAmbientDistance = 50.0f;
bool LMPolyGroup::bAmbientOcclusion = false;
int LMPolyGroup::iAmbientPattern = 0;
float LMPolyGroup::fCurvedBoost = 1.0f;
float LMPolyGroup::fMaxCurvedBoostSize = -1.0f;

void LMPolyGroup::SetAmbientOcclusionOn( int iterations, float fRayDist, int iPattern )
{
	if ( iterations < 1 ) iterations = 1;
	iIterations = iterations;

	if ( pRandomPoints ) delete [] pRandomPoints;
	pRandomPoints = new Point [ iIterations ];

	if ( pRandomDist ) delete [] pRandomDist;
	pRandomDist = new float [ iIterations ];

	float angX, angY, cosX, sinX, cosY, sinY, fDist;

	for ( int i = 0; i < iIterations; i++ )
	{
		angY = ( 6.2831853f * rand() ) / RAND_MAX;
		angX = ( 3.1415927f * rand() ) / RAND_MAX;
		cosX = (float)cos( angX ); sinX = (float)sin( angX );
		cosY = (float)cos( angY ); sinY = (float)sin( angY );
		fDist = ( 0.5f * rand() ) / RAND_MAX + 0.5f;

		pRandomPoints [ i ].x = sinY*sinX;
		pRandomPoints [ i ].y = cosX;
		pRandomPoints [ i ].z = cosY*sinX;

		pRandomDist [ i ] = fDist;
	}

	if ( fRayDist < 0 ) fRayDist = 0;
	fAmbientDistance = fRayDist;

	if ( iPattern < 0 ) iPattern = 0;
	iAmbientPattern = iPattern;

	bAmbientOcclusion = true;
}

void LMPolyGroup::SetAmbientOcclusionOff( )
{
	bAmbientOcclusion = false;
}

LMPolyGroup::LMPolyGroup( )
{
	pNextGroup = 0;
	pPolyList = 0;

	fQuality = 1;

	fSizeU = 0;
	fSizeV = 0;
	fOrigSizeU = 0;
	fOrigSizeV = 0;

	fMinU = 0;
	fMinV = 0;
	fOrigMinU = 0;
	fOrigMinV = 0;

	iSizeU = 0;
	iSizeV = 0;

	iStartU = 0;
	iStartV = 0;

	ppLumel = 0;
	ppColour = 0;
	ppShadow = 0;
	//ppShadowBlur = 0;
}

LMPolyGroup::~LMPolyGroup( )
{
	LMPoly *pTempPoly;

	while ( pPolyList )
	{
		pTempPoly = pPolyList;
		pPolyList = pPolyList->pNextPoly;
		delete pTempPoly;
	}

	if ( ppLumel )
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppLumel );
		//for ( int i = 0; i < iSizeU; i++ ) delete [] ppLumel[i];
		//delete [] ppLumel;
	}

	if ( ppColour )
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppColour [ i ] );
		HeapFree(g_hLMHeap, 0, ppColour );
		//for ( int i = 0; i < iSizeU; i++ ) delete [] ppColour[i];
		//delete [] ppColour;
	}

	if ( ppShadow )
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppShadow [ i ] );
		HeapFree(g_hLMHeap, 0, ppShadow );
		//for ( int i = 0; i < iSizeU; i++ ) delete [] ppShadow[i];
		//delete [] ppShadow;
	}
}

void LMPolyGroup::AddPoly( LMPoly *pNewPoly )
{
	if ( !pNewPoly ) return;

	pNewPoly->pNextPoly = pPolyList;
	pPolyList = pNewPoly;

	fMinU = pPolyList->GetMinU( );
	fMinV = pPolyList->GetMinV( );
	float fMaxU = fMinU + pPolyList->GetSizeU( );
	float fMaxV = fMinV + pPolyList->GetSizeV( );
	
	LMPoly *pPoly = pPolyList->pNextPoly;

	float fValue;
	while ( pPoly )
	{
		fValue = pPoly->GetMinU( );
		if ( fValue < fMinU ) fMinU = fValue;
		fValue += pPoly->GetSizeU( );
		if ( fValue > fMaxU ) fMaxU = fValue;

		fValue = pPoly->GetMinV( );
		if ( fValue < fMinV ) fMinV = fValue;
		fValue += pPoly->GetSizeV( );
		if ( fValue > fMaxV ) fMaxV = fValue;

		pPoly = pPoly->pNextPoly;
	}

	fSizeU = fMaxU - fMinU;
	fSizeV = fMaxV - fMinV;
	fOrigSizeU = fSizeU;
	fOrigSizeV = fSizeV;

	fOrigMinU = fMinU;
	fOrigMinV = fMinV;
}

void LMPolyGroup::SetStartPoint( int u, int v )
{
	iStartU = u+iPixelBorder;
	iStartV = v+iPixelBorder;
}

void LMPolyGroup::Scale( float fNewQuality )
{
	fQuality = fNewQuality;
	if ( pPolyList && pPolyList->IsCurved( ) && ( fMaxCurvedBoostSize < 0 || (fSizeU/fQuality)*(fSizeV/fQuality) < fMaxCurvedBoostSize ) )
	{
		fQuality /= fCurvedBoost;
	}
	
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		pPoly->fQuality = fQuality;

		pPoly = pPoly->pNextPoly;
	}

	float fDiff = (fOrigMinU / fQuality);
	fDiff = fOrigMinU - ((float)floor( fDiff ) * fQuality);

	fMinU = fOrigMinU - fDiff;
	fSizeU = fOrigSizeU + fDiff;

	fDiff = (fOrigMinV / fQuality);
	fDiff = fOrigMinV - ((float)floor( fDiff ) * fQuality);

	fMinV = fOrigMinV - fDiff;
	fSizeV = fOrigSizeV + fDiff;

	iSizeU = (int) ceil( fSizeU / fQuality ) + iPixelBorder*2;
	iSizeV = (int) ceil( fSizeV / fQuality ) + iPixelBorder*2;
}

int LMPolyGroup::GetScaledSizeU( )
{
	return iSizeU;
}

int LMPolyGroup::GetScaledSizeV( )
{
	return iSizeV;
}

float LMPolyGroup::GetSizeU( )
{
	return fSizeU;
}

float LMPolyGroup::GetSizeV( )
{
	return fSizeV;
}

void LMPolyGroup::CalculatePositionRadius( float *pPosX, float *pPosY, float *pPosZ, float *pSqrRadius )
{
	*pPosX = 0; *pPosY = 0; *pPosZ = 0;

	LMPoly *pPoly = pPolyList;

	float fPosX, fPosY, fPosZ;
	int iNumPolys = 0;

	while ( pPoly )
	{
		pPoly->GetAvgPoint( &fPosX, &fPosY, &fPosZ );
		*pPosX += fPosX;
		*pPosY += fPosY;
		*pPosZ += fPosZ;
		iNumPolys++;

		pPoly = pPoly->pNextPoly;
	}

	if ( iNumPolys == 0 ) return;

	*pPosX /= iNumPolys;
	*pPosY /= iNumPolys;
	*pPosZ /= iNumPolys;

	pPoly = pPolyList;
	*pSqrRadius = 0;

	while ( pPoly )
	{
		float fNewRadius = pPoly->GetMaxRadius( *pPosX, *pPosY, *pPosZ );
		if ( fNewRadius > *pSqrRadius ) *pSqrRadius = fNewRadius;

		pPoly = pPoly->pNextPoly;
	}
}

bool LMPolyGroup::GetPoint( float fPosU, float fPosV, float* fPosX, float* fPosY, float* fPosZ )
{
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fPosU, fPosV ) ) 
		{
			pPoly->GetPoint( fPosU, fPosV, fPosX, fPosY, fPosZ );
			return true;
		}

		pPoly = pPoly->pNextPoly;
	}

	//not in any polygon, get closest
	float fClosest = -1;
	LMPoly *pClosestPoly = 0;
	pPoly = pPolyList;

	while ( pPoly )
	{
		float fDist = pPoly->SqrDistToPoint( fPosU, fPosV, 0, 0 );

		if ( fDist >= 0 && (fClosest < 0 || fDist < fClosest) )
		{
			fClosest = fDist;
			pClosestPoly = pPoly;
		}

		pPoly = pPoly->pNextPoly;
	}

	if ( pClosestPoly ) 
	{
		pClosestPoly->GetPoint( fPosU, fPosV, fPosX, fPosY, fPosZ );
	}

	return false;
}

bool LMPolyGroup::GetPoint( int iPosU, int iPosV, float* fPosX, float* fPosY, float* fPosZ )
{
	//iPosU -= ( iStartU );
	//iPosV -= ( iStartV );

	iPosU -= iPixelBorder;
	iPosV -= iPixelBorder;
	
	float fPosU = (iPosU+0.5f)*fQuality + fMinU;
	float fPosV = (iPosV+0.5f)*fQuality + fMinV;

	return GetPoint( fPosU, fPosV, fPosX, fPosY, fPosZ );
}

void LMPolyGroup::GetNormal( float fPosX, float fPosY, float fPosZ , float* fNormX, float* fNormY, float* fNormZ )
{
	//iPosU -= ( iStartU );
	//iPosV -= ( iStartV );

	//iPosU -= iPixelBorder;
	//iPosV -= iPixelBorder;
	
	//float fPosU = (iPosU+0.5f)*fQuality + fMinU;
	//float fPosV = (iPosV+0.5f)*fQuality + fMinV;

	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fPosX, fPosY, fPosZ ) ) 
		{
			pPoly->GetNormal( fPosX, fPosY, fPosZ, fNormX, fNormY, fNormZ );
			return;
		}

		pPoly = pPoly->pNextPoly;
	}
}

void LMPolyGroup::ExtrapolateNormal( float fPosX, float fPosY, float fPosZ , float* fNormX, float* fNormY, float* fNormZ )
{
	//not in any polygon, get closest
	float fClosest = -1;
	LMPoly *pClosestPoly = 0;
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		float fDist = pPoly->SqrDistToPoint( fPosX, fPosY, fPosZ, 0 ,0 );

		if ( fDist >= 0 && ( fClosest < 0 || fDist < fClosest ) )
		{
			fClosest = fDist;
			pClosestPoly = pPoly;
		}

		pPoly = pPoly->pNextPoly;
	}

	if ( pClosestPoly )
	{
		pClosestPoly->ExtrapolateNormal( fPosX, fPosY, fPosZ, fNormX, fNormY, fNormZ );
	}
	else
	{
		//MessageBox(NULL, "Poly Not Found!", "Error", 0 );
	}
}

void LMPolyGroup::GetFaceNormal( int iPosU, int iPosV, float* fNormX, float* fNormY, float* fNormZ )
{
	iPosU -= iPixelBorder;
	iPosV -= iPixelBorder;
	
	float fPosU = (iPosU+0.5f)*fQuality + fMinU;
	float fPosV = (iPosV+0.5f)*fQuality + fMinV;

	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fPosU, fPosV ) ) 
		{
			pPoly->GetFaceNormal( fNormX, fNormY, fNormZ );
			return;
		}

		pPoly = pPoly->pNextPoly;
	}

	//not in any polygon, get closest
	float fClosest = -1;
	LMPoly *pClosestPoly = 0;
	pPoly = pPolyList;

	while ( pPoly )
	{
		float fDist = pPoly->SqrDistToPoint( fPosU, fPosV, 0, 0 );

		if ( fDist >= 0 && (fClosest < 0 || fDist < fClosest) )
		{
			fClosest = fDist;
			pClosestPoly = pPoly;
		}

		pPoly = pPoly->pNextPoly;
	}

	if ( pClosestPoly ) 
	{
		pClosestPoly->GetFaceNormal( fNormX, fNormY, fNormZ );
	}

	return;
}

LMPoly* LMPolyGroup::GetClosestPoint( int iU, int iV, float *pPosU, float *pPosV )
{
	iU -= iPixelBorder;
	iV -= iPixelBorder;
	
	float fU = (iU+0.5f)*fQuality + fMinU;
	float fV = (iV+0.5f)*fQuality + fMinV;

	float fClosest = -1;
	LMPoly *pClosestPoly = 0;
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		float fPosU, fPosV;
		float fDist = pPoly->SqrDistToPoint( fU, fV, &fPosU, &fPosV );

		if ( fDist >= 0 && ( fClosest < 0 || fDist < fClosest ) )
		{
			fClosest = fDist;
			pClosestPoly = pPoly;
			*pPosU = fPosU;
			*pPosV = fPosV;
		}

		pPoly = pPoly->pNextPoly;
	}

	return pClosestPoly;
}

bool LMPolyGroup::PointInGroup( int iU, int iV )
{
	iU -= ( iStartU );
	iV -= ( iStartV );

	float fU = (iU+0.5f)*fQuality + fMinU;
	float fV = (iV+0.5f)*fQuality + fMinV;

	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fU, fV ) ) return true;

		pPoly = pPoly->pNextPoly;
	}

	return false;
}

bool LMPolyGroup::Joined( LMPoly *pOtherPoly )
{
	LMPoly *pPoly = pPolyList;
	
	while ( pPoly )
	{
		if ( pPoly->Joined( pOtherPoly ) ) return true;

		pPoly = pPoly->pNextPoly;
	}

	return false;
}

float LMPolyGroup::GetUsedSpace( )
{
	float fMainArea = fSizeU*fSizeV;
	if ( fMainArea < 0.00001f ) return 0.0f;

	float fTotalArea = 0;
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		fTotalArea += pPoly->GetArea( );

		pPoly = pPoly->pNextPoly;
	}
	
	return fTotalArea / fMainArea;
}

float LMPolyGroup::GetNewUsedSpace( LMPoly *pNewPoly )
{
	float fNewMinU = fMinU;
	float fNewMinV = fMinV;
	float fNewMaxU = fNewMinU + fSizeU;
	float fNewMaxV = fNewMinV + fSizeV;
	
	float fValue;
	fValue = pNewPoly->GetMinU( );
	if ( fValue < fNewMinU ) fNewMinU = fValue;
	fValue += pNewPoly->GetSizeU( );
	if ( fValue > fNewMaxU ) fNewMaxU = fValue;

	fValue = pNewPoly->GetMinV( );
	if ( fValue < fNewMinV ) fNewMinV = fValue;
	fValue += pNewPoly->GetSizeV( );
	if ( fValue > fNewMaxV ) fNewMaxV = fValue;

	float fMainArea = (fNewMaxU - fNewMinU) * (fNewMaxV - fNewMinV);
	if ( fMainArea < 0.00001f ) return 0.0f;
	
	float fTotalArea = pNewPoly->GetArea( );
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		fTotalArea += pPoly->GetArea( );

		pPoly = pPoly->pNextPoly;
	}

	return fTotalArea / fMainArea;
}

void LMPolyGroup::SetLocalValues( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, bool bIgnoreNormals )
{
	pLocalLightList = pLightList;
	pLocalColTree = pColTree;
	iLocalBlur = iBlur;
	m_bIgnoreNormals = bIgnoreNormals;

	if ( ppLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppLumel );
		ppLumel = 0;
	}
	ppLumel = (Lumel**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppLumel [ i ] = (Lumel*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(Lumel));
}

void LMPolyGroup::CalculateLight( const Light *pLightList, const CollisionTreeLightmapper *pColTree, int iBlur, bool bIgnoreNormals )
{
	if ( bAmbientOcclusion && iAmbientPattern != 0 && !pRandomPoints )
	{
		MessageBox( NULL, "Ambient occlusion set but no pattern data available", "Lightmapping Error", 0 );
		exit(-1);
	}

	m_bIgnoreNormals = bIgnoreNormals;

	float *pWeights = 0;

	if ( iBlur > 0 )
	{
		pWeights = new float [ iBlur*2 + 1 ];
		pWeights [ iBlur ] = 1.0f / ((iBlur+2));
		float fWeight = (1.0f - pWeights [ iBlur ]) / 2.0f;

		for ( int i = iBlur + 1; i < iBlur*2; i++ )
		{
			fWeight /= 2.0f;
			pWeights [ i ] = fWeight;
		}

		pWeights [ iBlur*2 ] = fWeight;

		for ( int i = 0; i < iBlur; i++ )
		{
			pWeights [ i ] = pWeights [ iBlur*2 - i ];
		}
		
	}

	//create outside thread heap
	
	if ( ppLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppLumel );
		ppLumel = 0;
	}
	ppLumel = (Lumel**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppLumel [ i ] = (Lumel*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(Lumel));

	ppColour = (Lumel**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppColour [ i ] = (Lumel*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(Lumel));
	//ppColour = new Lumel* [ iSizeU ];
	//for ( int u = 0; u < iSizeU; u++ ) ppColour [ u ] = new Lumel [ iSizeV ];

	ppShadow = (float**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppShadow [ i ] = (float*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(float));
	//ppShadow = new float* [ iSizeU ];
	//for ( int u = 0; u < iSizeU; u++ ) ppShadow [ u ] = new float [ iSizeV ];

	//use lumel structure to store position and normal data
	Lumel **ppPosition = (Lumel**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppPosition [ i ] = (Lumel*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(Lumel));
	//Lumel **ppPosition = new Lumel* [ iSizeU ];
	//for ( int u = 0; u < iSizeU; u++ ) ppPosition [ u ] = new Lumel [ iSizeV ];

	Lumel **ppNormal = (Lumel**)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeU * sizeof(LPVOID));
	for ( int i = 0; i < iSizeU; i++ ) ppNormal [ i ] = (Lumel*)HeapAlloc(g_hLMHeap, HEAP_ZERO_MEMORY, iSizeV * sizeof(Lumel));
	//Lumel **ppNormal = new Lumel* [ iSizeU ];
	//for ( int u = 0; u < iSizeU; u++ ) ppNormal [ u ] = new Lumel [ iSizeV ];

	float fRed=0, fGreen=0, fBlue=0;
	float fPosX=0, fPosY=0, fPosZ=0;
	float fLightPosX=0, fLightPosY=0, fLightPosZ=0;
	float fNormX=0, fNormY=0, fNormZ=0;
	float fDotP=0, fLength=1;
	Vector vec, vecI;
	Point p;
	LMPoly* pInPoly = 0;
	bool bInPoly = false;

	float fCenterX, fCenterY, fCenterZ, fSqrRadius;
	CalculatePositionRadius( &fCenterX, &fCenterY, &fCenterZ, &fSqrRadius );
	fSqrRadius = (float)sqrt( fSqrRadius );

	const Light *pCurrLight = pLightList;
	TreeFaceLightmapper *pLastCollider = 0;
	TreeFaceLightmapper *pFace = 0;

	//int iNumValidPixels = 0;

	for ( int u = 0; u < iSizeU; u++ )
	{
		for ( int v = 0; v < iSizeV; v++ )
		{
			if ( g_pShared && g_pShared->GetTerminate( ) ) break;

			bInPoly = GetPoint( u, v, &fPosX, &fPosY, &fPosZ );
			ppPosition[u][v].SetCol( fPosX, fPosY, fPosZ );

			if ( bInPoly ) GetNormal( fPosX, fPosY, fPosZ, &fNormX, &fNormY, &fNormZ );
			else ExtrapolateNormal( fPosX, fPosY, fPosZ, &fNormX, &fNormY, &fNormZ );
			ppNormal[u][v].SetCol( fNormX, fNormY, fNormZ );

			// per object mode (to exclude terrain object from normal based light mapping)
			if ( m_bIgnoreNormals==true ) 
				ppNormal[u][v].SetCol( 0, 1, 0 );

			//mode = 3 : colour based on normal only
			if ( iMode == 3 )
			{
				ppLumel[u][v].SetCol( (fNormX+1)/2, (fNormY+1)/2, (fNormZ+1)/2 );
				continue;
			}

			//mode = 1 : extend all edges, all pixels valid
			if ( !bInPoly && iMode != 1 ) 
			{
				bool bHit = false;
				float fPosU=0, fPosV=0, fPosX2=0, fPosY2=0, fPosZ2=0;

				LMPoly *pPoly = GetClosestPoint( u, v, &fPosU, &fPosV );
				pPoly->GetPoint( fPosU, fPosV, &fPosX2, &fPosY2, &fPosZ2 );
				
				fRed = fPosX2 - fPosX;
				fGreen = fPosY2 - fPosY;
				fBlue = fPosZ2 - fPosZ;
				float fDist = (float)sqrt( fRed*fRed + fGreen*fGreen + fBlue*fBlue );
				if ( fDist > 0.00001f )
				{
					fPosX2 += (fRed/fDist)*0.01f;
					fPosY2 += (fGreen/fDist)*0.01f;
					fPosZ2 += (fBlue/fDist)*0.01f;
				}

				GetNormal( fPosX2, fPosY2, fPosZ2, &fNormX, &fNormY, &fNormZ );

				// per object mode (to exclude terrain object from normal based light mapping) (see above)
				if ( m_bIgnoreNormals==true ) 
				{
					fNormX = 0;
					fNormY = 1;
					fNormZ = 0;
				}

				//if mode = 2, all outside points considered invalid.
				if ( iMode != 2 )
				{
					float fDiffX = fPosX - fPosX2;
					float fDiffY = fPosY - fPosY2;
					float fDiffZ = fPosZ - fPosZ2;
					float fDotP = ( fDiffX*fNormX + fDiffY*fNormY + fDiffZ*fNormZ ) / (float)sqrt( fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ );

					if ( fDotP > 0.03f )
					{
						bHit = true;
					}
					else if ( fDotP >= -0.03f )
					{
						vec.set( fNormX*0.2f, fNormY*0.2f, fNormZ*0.2f );
						vecI.set( 1/vec.x, 1/vec.y, 1/vec.z );
						p.set ( fPosX - fNormX*0.1f, fPosY - fNormY*0.1f, fPosZ - fNormZ*0.1f );

						pFace = pLastCollider;
						while ( pFace )
						{
							if ( pFace->intersects( &p, &vec, 0, 0 ) ) { bHit = true; break; }

							pFace = pFace->nextFace;
						}

						if ( !bHit ) bHit = (pColTree && pColTree->intersects( &p, &vec, &vecI, 0, 0, &pLastCollider ));
					}
				}
				
				//if invalid move point into polygon
				if ( !bHit )
				{
					ppPosition[u][v].SetCol( fPosX2, fPosY2, fPosZ2 );
					ppNormal[u][v].SetCol( fNormX, fNormY, fNormZ );
				}
			}

			//ambient occulsion - VERY EXPENSIVE
			if ( bAmbientOcclusion )
			{
				float fRatio = 0;
				float fSkyX, fSkyY, fSkyZ;

				p.set ( ppPosition[u][v].GetColR( ) + ppNormal[u][v].GetColR( )*0.1f, 
					    ppPosition[u][v].GetColG( ) + ppNormal[u][v].GetColG( )*0.1f,
						ppPosition[u][v].GetColB( ) + ppNormal[u][v].GetColB( )*0.1f );

				for ( int i = 0; i < iIterations; i++ )
				{
					if ( g_pShared && g_pShared->GetTerminate( ) ) break;
					if ( iAmbientPattern == 0 )
					{
						float angY = ( 6.2831853f * rand() ) / RAND_MAX;
						float angX = ( 3.1415927f * rand() ) / RAND_MAX;
						float cosX = (float)cos( angX ); float sinX = (float)sin( angX );
						float cosY = (float)cos( angY ); float sinY = (float)sin( angY );
						float fDist = ( 0.5f * rand() ) / RAND_MAX + 0.5f;

						fSkyX = ppNormal[u][v].GetColR( ) + (sinY*sinX);
						fSkyY = ppNormal[u][v].GetColG( ) + (cosX);
						fSkyZ = ppNormal[u][v].GetColB( ) + (cosY*sinX);
						fSkyX = fSkyX*fAmbientDistance*fDist;
						fSkyY = fSkyY*fAmbientDistance*fDist;
						fSkyZ = fSkyZ*fAmbientDistance*fDist;
					}
					else
					{
						fSkyX = ppNormal[u][v].GetColR( ) + pRandomPoints [ i ].x;
						fSkyY = ppNormal[u][v].GetColG( ) + pRandomPoints [ i ].y;
						fSkyZ = ppNormal[u][v].GetColB( ) + pRandomPoints [ i ].z;
						fSkyX = fSkyX*fAmbientDistance*pRandomDist[ i ];
						fSkyY = fSkyY*fAmbientDistance*pRandomDist[ i ];
						fSkyZ = fSkyZ*fAmbientDistance*pRandomDist[ i ];
					}

					vec.set( fSkyX, fSkyY, fSkyZ );
					vecI.set( 1/vec.x, 1/vec.y, 1/vec.z );

					bool bHit = false;
					pFace = pLastCollider;
					while ( pFace )
					{
						if ( pFace->intersects( &p, &vec, 0, 0 ) ) { bHit = true; break; }

						pFace = pFace->nextFace;
					}

					if ( !bHit ) bHit = (pColTree && pColTree->intersects( &p, &vec, &vecI, 0, 0, &pLastCollider ));

					if ( !bHit ) fRatio += 1.0f / iIterations;
				}

				// 281114 - do not allow absolute zero (too much!)
				fRatio = 0.5f + ( fRatio * 0.5f );

				ppLumel[u][v].SetCol( fRatio*Light::fAmbientR, fRatio*Light::fAmbientG, fRatio*Light::fAmbientB );

				//continue;
			}
			else
			{
				ppLumel[u][v].SetCol( Light::fAmbientR, Light::fAmbientG, Light::fAmbientB );
			}
		}
	}

	if ( iBlur > 0 && bAmbientOcclusion )
	{
		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				fRed = 0; fGreen = 0; fBlue = 0;

				for ( int i = -iBlur; i <= iBlur; i++ )
				{
					if ( v+i >= 0 && v+i < iSizeV ) 
					{
						fRed += ppLumel[u][v+i].GetColR( ) * pWeights [ i + iBlur ];
						fGreen += ppLumel[u][v+i].GetColG( ) * pWeights [ i + iBlur ];
						fBlue += ppLumel[u][v+i].GetColB( ) * pWeights [ i + iBlur ];
					}
					else 
					{
						fRed += ppLumel[u][v].GetColR( ) * pWeights [ i + iBlur ];
						fGreen += ppLumel[u][v].GetColG( ) * pWeights [ i + iBlur ];
						fBlue += ppLumel[u][v].GetColB( ) * pWeights [ i + iBlur ];
					}
				}

				ppColour[u][v].SetCol( fRed, fGreen, fBlue );
			}
		}

		//horizontal blur shadow
		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				fRed = 0; fGreen = 0; fBlue = 0;

				for ( int i = -iBlur; i <= iBlur; i++ )
				{
					if ( v+i >= 0 && v+i < iSizeV ) 
					{
						fRed += ppColour[u][v+i].GetColR( ) * pWeights [ i + iBlur ];
						fGreen += ppColour[u][v+i].GetColG( ) * pWeights [ i + iBlur ];
						fBlue += ppColour[u][v+i].GetColB( ) * pWeights [ i + iBlur ];
					}
					else 
					{
						fRed += ppColour[u][v].GetColR( ) * pWeights [ i + iBlur ];
						fGreen += ppColour[u][v].GetColG( ) * pWeights [ i + iBlur ];
						fBlue += ppColour[u][v].GetColB( ) * pWeights [ i + iBlur ];
					}
				}

				ppLumel[u][v].SetCol( fRed, fGreen, fBlue );
			}
		}
	}

	if ( iMode == 3 )
	{
		pCurrLight = 0;
	}

	while ( pCurrLight )
	{
		if ( g_pShared && g_pShared->GetTerminate( ) ) break;

		if ( !pCurrLight->GetInRange( fCenterX, fCenterY, fCenterZ, fSqrRadius ) )
		{
			pCurrLight = pCurrLight->pNextLight;
			continue;
		}

		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				fPosX = ppPosition[u][v].GetColR( );
				fPosY = ppPosition[u][v].GetColG( );
				fPosZ = ppPosition[u][v].GetColB( );

				fNormX = ppNormal[u][v].GetColR( );
				fNormY = ppNormal[u][v].GetColG( );
				fNormZ = ppNormal[u][v].GetColB( );

				pCurrLight->GetOrgin( fPosX, fPosY, fPosZ, &fLightPosX, &fLightPosY, &fLightPosZ );
				vec.set( fPosX-fLightPosX, fPosY-fLightPosY, fPosZ-fLightPosZ );
				fDotP = vec.x*fNormX + vec.y*fNormY + vec.z*fNormZ;

				if ( fDotP >= 0.0f || !pCurrLight->GetColorFromPoint( fPosX, fPosY, fPosZ, &fRed, &fGreen, &fBlue, &fLength ) )
				{
					//pCurrLight = pCurrLight->pNextLight;
					ppShadow[u][v] = 1.0;
					ppColour[u][v].SetCol( 0, 0, 0 );
					continue;
				}

				fDotP	/= fLength;
				fDotP	 = 0.0f - fDotP;
				if ( fDotP < 0 ) fDotP = 0;
				fRed	*= fDotP;
				fGreen	*= fDotP;
				fBlue	*= fDotP;

				ppColour[u][v].SetCol( fRed, fGreen, fBlue );				

				//check the light->lumel vector
				vec.x += fNormX*0.1f; vec.y += fNormY*0.1f; vec.z += fNormZ*0.1f;
				vecI.set( 1/vec.x, 1/vec.y, 1/vec.z );
				p.set ( fLightPosX, fLightPosY, fLightPosZ );
				ppShadow[u][v] = 0.0;

				//quick check, does this ray hit the group of polygons the last ray hit
				bool bHit = false;
				pFace = pLastCollider;
				while ( pFace )
				{
					if ( pFace->intersects( &p, &vec, &ppColour[u][v], &ppShadow[u][v] ) ) { bHit = true; break; }

					pFace = pFace->nextFace;
				}
				
				//if not check all polygons
				if ( bHit || pColTree && pColTree->intersects( &p, &vec, &vecI, &ppColour[u][v], &ppShadow[u][v], &pLastCollider ) )
				{
					//ppShadow[u][v] = 1.0;
					//continue;
				}
			}
		} 

		float fValue;
		
		//variable blur
		//vertical blur shadow
		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				//fValue = pow(ppShadow[u][v],fShadowPower);
				fValue = ppShadow[u][v];
				fRed	= ppColour[u][v].GetColR( ) * (1 - fValue);
				fGreen	= ppColour[u][v].GetColG( ) * (1 - fValue);
				fBlue	= ppColour[u][v].GetColB( ) * (1 - fValue);
				ppLumel[u][v].AddCol( fRed, fGreen, fBlue );
			}
		}

		pCurrLight = pCurrLight->pNextLight;
	}

	if ( iBlur > 0 )
	{
		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				float fValueR = 0;
				float fValueG = 0;
				float fValueB = 0;

				for ( int i = -iBlur; i <= iBlur; i++ )
				{
					if ( v+i >= 0 && v+i < iSizeV ) 
					{
						fValueR += ppLumel[u][v+i].GetColR() * pWeights [ i + iBlur ];
						fValueG += ppLumel[u][v+i].GetColG() * pWeights [ i + iBlur ];
						fValueB += ppLumel[u][v+i].GetColB() * pWeights [ i + iBlur ];
					}
					else 
					{
						fValueR += ppLumel[u][v].GetColR() * pWeights [ i + iBlur ];
						fValueG += ppLumel[u][v].GetColG() * pWeights [ i + iBlur ];
						fValueB += ppLumel[u][v].GetColB() * pWeights [ i + iBlur ];
					}
				}

				ppColour[u][v].SetCol( fValueR, fValueG, fValueB );
			}
		}

		//horizontal blur shadow
		for ( int u = 0; u < iSizeU; u++ )
		{
			for ( int v = 0; v < iSizeV; v++ )
			{
				float fValueR = 0;
				float fValueG = 0;
				float fValueB = 0;

				for ( int i = -iBlur; i <= iBlur; i++ )
				{
					if ( u+i >= 0 && u+i < iSizeU ) 
					{
						fValueR += ppColour[u+i][v].GetColR() * pWeights [ i + iBlur ];
						fValueG += ppColour[u+i][v].GetColG() * pWeights [ i + iBlur ];
						fValueB += ppColour[u+i][v].GetColB() * pWeights [ i + iBlur ];
					}
					else 
					{
						fValueR += ppColour[u][v].GetColR() * pWeights [ i + iBlur ];
						fValueG += ppColour[u][v].GetColG() * pWeights [ i + iBlur ];
						fValueB += ppColour[u][v].GetColB() * pWeights [ i + iBlur ];
					}
				}

				ppLumel[u][v].SetCol( fValueR, fValueG, fValueB );
			}
		}
	}

	for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppPosition [ i ] );
	HeapFree(g_hLMHeap, 0, ppPosition );
	//for ( int i = 0; i < iSizeU; i++ ) delete [] ppPosition[i];
	//delete [] ppPosition;

	for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppNormal [ i ] );
	HeapFree(g_hLMHeap, 0, ppNormal );
	//for ( int i = 0; i < iSizeU; i++ ) delete [] ppNormal[i];
	//delete [] ppNormal;

	if ( ppColour )
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppColour [ i ] );
		HeapFree(g_hLMHeap, 0, ppColour );
		//for ( int i = 0; i < iSizeU; i++ ) delete [] ppColour[i];
		//delete [] ppColour;
		ppColour = 0;
	}

	if ( ppShadow )
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppShadow [ i ] );
		HeapFree(g_hLMHeap, 0, ppShadow );
		//for ( int i = 0; i < iSizeU; i++ ) delete [] ppShadow[i];
		//delete [] ppShadow;
		ppShadow = 0;
	}

	if ( pWeights ) delete [] pWeights;
}

void LMPolyGroup::ApplyToTexture( LMTexture *pTexture )
{
	int iOffsetU = iStartU - iPixelBorder;
	int iOffsetV = iStartV - iPixelBorder;

	for ( int u = 0; u < iSizeU; u++ )
	{
		for ( int v = 0; v < iSizeV; v++ )
		{
			pTexture->SetLumel( u + iOffsetU, v + iOffsetV, ppLumel[u][v].GetColR( ), ppLumel[u][v].GetColG( ), ppLumel[u][v].GetColB( ) );
		}
	}
	if ( ppLumel ) 
	{
		for ( int i = 0; i < iSizeU; i++ ) HeapFree(g_hLMHeap, 0, ppLumel [ i ] );
		HeapFree(g_hLMHeap, 0, ppLumel );
		ppLumel = 0;
	}
}

void LMPolyGroup::CalculateTexUV( int iTexSizeU, int iTexSizeV )
{
	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		pPoly->CalculateTexUV( iStartU, iStartV, fMinU, fMinV, iTexSizeU, iTexSizeV );

		pPoly = pPoly->pNextPoly;
	}
}

DWORD LMPolyGroup::GetColour( int u, int v )
{
	u -= ( iPixelBorder );
	v -= ( iPixelBorder );
	
	float fPosU = (u+0.5f)*fQuality + fMinU;
	float fPosV = (v+0.5f)*fQuality + fMinV;

	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fPosU, fPosV ) ) return pPoly->colour;

		pPoly = pPoly->pNextPoly;
	}

	return 0xff000000;
}

void LMPolyGroup::GetColour( int u, int v, float* red, float* green, float* blue )
{
	u -= ( iPixelBorder );
	v -= ( iPixelBorder );
	
	float fPosU = (u+0.5f)*fQuality + fMinU;
	float fPosV = (v+0.5f)*fQuality + fMinV;

	LMPoly *pPoly = pPolyList;

	while ( pPoly )
	{
		if ( pPoly->PointInPoly( fPosU, fPosV ) ) 
		{
			*red = ( (pPoly->colour & 0x00ff0000) >> 16 ) / 255.0f;
			*green = ( (pPoly->colour & 0x0000ff00) >> 8 ) / 255.0f;
			*blue = ( (pPoly->colour & 0x000000ff) ) / 255.0f;

			return;
		}

		pPoly = pPoly->pNextPoly;
	}

	*red = 0;
	*green = 0;
	*blue = 0;
}

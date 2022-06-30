#define _CRT_SECURE_NO_DEPRECATE

#include "PathFinderAdvanced.h"
#include "Polygon.h"
#include "Path.h"
#include "Container.h"
#include "DBPro Functions.h"
#include "Entity.h"
#include <math.h>
#include <algorithm>
#include <stdio.h>
#include <windows.h>
#include "World.h"

extern "C" int GG_fopen_s( FILE** pFile, const char* filename, const char* mode );

// limit to how many cover points
#define MAX_COVER_POINTS 4000

// namespace
using namespace std;

// externs
extern World cWorld;
extern vector <tempCoverPoint> tempCoverPoints;
extern int g_GUIShaderEffectID;

// prototype functions
float GetLUATerrainHeightEx ( float fX, float fZ );
void mp_refresh(void);

// globals
int coverID = -1;

inline float EstimateDistance ( float fSX, float fSY, float fEX, float fEY )
{
    //2 options
    
    //guaranteed shortest path, with sqrt
    return sqrt ( (fSX-fEX)*(fSX-fEX) + (fSY-fEY)*(fSY-fEY) );
    
    //or close enough path, without sqrt
    //return ( fabs(fSX-fEX) + fabs(fSY-fEY) );
}

inline float ActualDistance ( float fSX, float fSY, float fEX, float fEY )
{
    //return sqrt ( (fSX-fEX)*(fSX-fEX) + (fSY-fEY)*(fSY-fEY) );
    return EstimateDistance ( fSX, fSY, fEX, fEY );
}

EdgeData::EdgeData( )
{
	fX = 0; fY = 0;
	fX2 = 0; fY2 = 0;
	fNormVX = 0; fNormVY = 0;
	bHalfHeight = false;
	pNextEdge = 0;
}

EdgeData::~EdgeData( )
{
	
}

bool EdgeData::Intersects ( float fSX, float fSY, float fEX, float fEY, int iHeight )
{
	if ( bHalfHeight && iHeight==1 ) return false;
	if ( !bHalfHeight && iHeight==0 ) return false;
	
	float fDiffPx = fEX - fSX;
	float fDiffPy = fEY - fSY;

	float fDiffVx = fX2 - fX;
	float fDiffVy = fY2 - fY;

	float fRel = ( fDiffPy * fDiffVx ) - ( fDiffPx * fDiffVy );
    
    //parallel or backface
    if ( fRel >= 0 ) return false;

    fRel = 1.0f / fRel;
	
    
    //t  - distance along path ray to collision
    //t2 - distance along polygon vector to collision
    float t  = ( ( fSX-fX )*fDiffVy - ( fSY-fY )*fDiffVx ) * fRel;
    float t2 = ( ( fY-fSY )*fDiffPx - ( fX-fSX )*fDiffPy ) * fRel;
    
    if ( ( t < 0 || t > 1 ) || ( t2 < 0 || t2 > 1 ) ) return false;

	return true;
}

CollisionNode::CollisionNode( int iNumEdges, int iEdgesPerNode, EdgeData *pEdgeList )
{
	left = 0; right = 0;
	fMinX = 0; fMaxX = 0;
	fMinY = 0; fMaxY = 0;

	if ( !pEdgeList ) return;

	fMinX = pEdgeList->fX; fMinY = pEdgeList->fY;
	fMaxX = pEdgeList->fX; fMaxY = pEdgeList->fY;
	
	EdgeData *pEdge = pEdgeList;

	while ( pEdge )
	{
		if ( pEdge->fX < fMinX ) fMinX = pEdge->fX;
		if ( pEdge->fX > fMaxX ) fMaxX = pEdge->fX;
		if ( pEdge->fY < fMinY ) fMinY = pEdge->fY;
		if ( pEdge->fY > fMaxY ) fMaxY = pEdge->fY;

		if ( pEdge->fX2 < fMinX ) fMinX = pEdge->fX2;
		if ( pEdge->fX2 > fMaxX ) fMaxX = pEdge->fX2;
		if ( pEdge->fY2 < fMinY ) fMinY = pEdge->fY2;
		if ( pEdge->fY2 > fMaxY ) fMaxY = pEdge->fY2;

		pEdge = pEdge->pNextEdge;
	}

	fMinX -= 0.0001f;
	fMinY -= 0.0001f;
	fMaxX += 0.0001f;
	fMaxY += 0.0001f;

	if ( iNumEdges <= iEdgesPerNode )
	{
		left = ( CollisionNode* ) pEdgeList;
		return;
	}

	float fDiffX = fMaxX - fMinX;
	float fDiffY = fMaxY - fMinY;
	int axis = 0;
	float fLimit = fDiffX/2.0f + fMinX;
	float avg = 0;

	if ( fDiffY > fDiffX ) 
	{
		axis = 1;
		fLimit = fDiffY/2.0f + fMinY;
	}

	EdgeData *pLeftEdges = 0;
	EdgeData *pRightEdges = 0;
	EdgeData *pLastEdge = 0;
	EdgeData *pTemp = 0;
	int iNumEdges1 = 0;
	int iNumEdges2 = 0;

	while ( pEdgeList )
	{
		avg = axis==0 ? (pEdgeList->fX + pEdgeList->fX2)/2.0f : (pEdgeList->fY + pEdgeList->fY2)/2.0f;

		if ( avg > fLimit )
		{
			if (iNumEdges1==0) pLastEdge = pEdgeList;

			pTemp = pEdgeList->pNextEdge;
			pEdgeList->pNextEdge = pLeftEdges;
			pLeftEdges = pEdgeList;
			pEdgeList = pTemp;

			iNumEdges1++;
		}
		else
		{
			pTemp = pEdgeList->pNextEdge;
			pEdgeList->pNextEdge = pRightEdges;
			pRightEdges = pEdgeList;
			pEdgeList = pTemp;

			iNumEdges2++;
		}
	}

	if ( abs(iNumEdges1 - iNumEdges2) > 0.9*iNumEdges )
	{
		iNumEdges1 = 0;
		iNumEdges2 = 0;

		if ( pLeftEdges )
		{
			pEdgeList = pLeftEdges;
			pLastEdge->pNextEdge = pRightEdges;
		}
		else pEdgeList = pRightEdges;

		pLeftEdges = 0;
		pRightEdges = 0;

		int iFlag = 1;

		while ( pEdgeList )
		{
			if ( iFlag )
			{
				pTemp = pEdgeList->pNextEdge;
				pEdgeList->pNextEdge = pLeftEdges;
				pLeftEdges = pEdgeList;
				pEdgeList = pTemp;

				iNumEdges1++;
				iFlag = 0;
			}
			else
			{
				pTemp = pEdgeList->pNextEdge;
				pEdgeList->pNextEdge = pRightEdges;
				pRightEdges = pEdgeList;
				pEdgeList = pTemp;

				iNumEdges2++;
				iFlag = 1;
			}
		}
	}

	left = new CollisionNode( iNumEdges1, iEdgesPerNode, pLeftEdges );
	right = new CollisionNode( iNumEdges2, iEdgesPerNode, pRightEdges );
}

void CollisionNode::CountEdges ( int *pCount )
{
	if ( !pCount ) return;

	if ( right )
	{
		left->CountEdges ( pCount );
		right->CountEdges ( pCount );
	}
	else
	{
		EdgeData *pEdge = (EdgeData*) left;
		while ( pEdge )
		{
			(*pCount)++;
			pEdge = pEdge->pNextEdge;
		}
	}
}

CollisionNode::~CollisionNode( )
{
	if ( right ) 
	{
		delete left;
		delete right;
	}
	else
	{
		while ( left )
		{
			EdgeData *pTemp = (EdgeData*)left;
			left = (CollisionNode*) ( (EdgeData*)left )->pNextEdge;
			delete pTemp;
		}
	}
}

bool CollisionNode::Intersects ( float fSX, float fSY, float fEX, float fEY, int iHeight )
{
	float fVX = 1.0f/(fEX - fSX);
	float fVY = 1.0f/(fEY - fSY);

	float tminx,tmaxx,tminy,tmaxy;     

    if (fVX>=0) {
        tminx = (fMinX - fSX) *fVX;
        tmaxx = (fMaxX - fSX) *fVX;
    }
    else {
        tmaxx = (fMinX - fSX) *fVX;
        tminx = (fMaxX - fSX) *fVX;
    }
    
    if (fVY>=0) {
        tminy = (fMinY - fSY) *fVY;
        tmaxy = (fMaxY - fSY) *fVY;
    }
    else {
        tmaxy = (fMinY - fSY) *fVY;
        tminy = (fMaxY - fSY) *fVY;
    }
               
    if ((tminx > tmaxy) || (tminy > tmaxx)) return false;
		
	if (tminy>tminx) tminx = tminy;
    if (tmaxy<tmaxx) tmaxx = tmaxy;

	if (tmaxx<0.0) return false;
    if (tminx>1.0) return false;
	
	if ( right )
	{
		if ( left->Intersects ( fSX, fSY, fEX, fEY, iHeight ) ) return true;
		return right->Intersects ( fSX, fSY, fEX, fEY, iHeight );
	}
	else
	{
		EdgeData *pEdge = (EdgeData*) left;
		while ( pEdge )
		{
			if ( pEdge->Intersects( fSX, fSY, fEX, fEY, iHeight ) ) return true;

			pEdge = pEdge->pNextEdge;
		}

		return false;
	}
}

void PathFinderAdvanced::sWaypoint::AddEdge( sWaypoint *pWaypoint, float fCost )
{
	if ( fCost < 0 ) fCost = 0;

	sWaypointEdge *pEdge = new sWaypointEdge;
	pEdge->pOtherWP = pWaypoint;
	pEdge->fCost = fCost;
	pEdge->pDoors = 0;

	pEdge->pNextEdge = pEdgeList;
	pEdgeList = pEdge;

	iNumEdges++;
}

void PathFinderAdvanced::sWaypoint::RemoveLastEdge ( )
{
	sWaypointEdge *pEdge = pEdgeList;

	pEdgeList = pEdgeList->pNextEdge;

	delete pEdge;

	iNumEdges--;
}

void PathFinderAdvanced::sWaypoint::RemoveEdge ( sWaypoint *pOtherWaypoint )
{
	if ( !pEdgeList ) return;
	if ( !pOtherWaypoint ) return;

	sWaypointEdge *pEdge = pEdgeList;

	if ( pEdgeList->pOtherWP == pOtherWaypoint )
	{
		pEdgeList = pEdgeList->pNextEdge;
		delete pEdge;
		iNumEdges--;
		return;
	}

	sWaypointEdge *pPrevEdge = pEdge;
	pEdge = pEdge->pNextEdge;

	while ( pEdge )
	{
		if ( pEdge->pOtherWP == pOtherWaypoint )
		{
			pPrevEdge->pNextEdge = pEdge->pNextEdge;
			delete pEdge;
			iNumEdges--;
			return;
		}

		pPrevEdge = pEdge;
		pEdge = pEdge->pNextEdge;
	}
}

void PathFinderAdvanced::sWaypoint::ClearEdges ( )
{
	sWaypointEdge *pEdge = pEdgeList;

	while ( pEdge )
	{
		pEdgeList = pEdgeList->pNextEdge;

		delete pEdge;

		pEdge = pEdgeList;
	}

	pEdgeList = 0;
	iNumEdges = 0;
}

PathFinderAdvanced::sWaypoint::~sWaypoint ( )
{
	ClearEdges ( );
}

HANDLE PathFinderAdvanced::hPathFindingMutex = NULL;
int PathFinderAdvanced::iUndesirableThreshold = 3;

PathFinderAdvanced::sWaypoint* PathFinderAdvanced::AddWaypoint ( float x, float z, bool bHalf, bool bIsBridge, float vx, float vy, float ca, bool canpeek, char flags )
{
	sWaypoint *pNewWP = new sWaypoint( );
	pNewWP->fX = x;
	pNewWP->fY = z;
	pNewWP->bHalfHeight = bHalf;
	pNewWP->bActive = true;
	pNewWP->iVisited = 0;
	pNewWP->pContainer = pOwner;
	pNewWP->bCanPeek = canpeek && !bHalf;
	pNewWP->fCAngle = ca;
	pNewWP->fVX = vx;
	pNewWP->fVY = vy;
	pNewWP->bFlags = flags;
	if ( bIsBridge ) pNewWP->bFlags |= DARKAI_WAYPOINT_BRIDGE;

	pNewWP->pNextWaypoint = pWaypointList;
	pWaypointList = pNewWP;

	iNumWaypoints++;

	return pWaypointList;
}

void PathFinderAdvanced::AddWaypoint ( sWaypoint *pNewWP )
{
	pNewWP->pNextWaypoint = pWaypointList;
	pWaypointList = pNewWP;

	iNumWaypoints++;
}

void PathFinderAdvanced::RemoveLastWaypoint ( )
{
	if ( !pWaypointList ) return;

	sWaypoint *pWaypoint = pWaypointList;
	pWaypointList = pWaypointList->pNextWaypoint;

	delete pWaypoint;

	iNumWaypoints--;
}

void PathFinderAdvanced::ClearWaypoints ( )
{
	sWaypoint *pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		pWaypointList = pWaypointList->pNextWaypoint;

		delete pWaypoint;

		pWaypoint = pWaypointList;
	}

	pWaypointList = 0;

	sWaypointOpen_list.clear ( );

	iNumWaypoints = 0;
}

void PathFinderAdvanced::ClearCoverPoints ( )
{
	sCoverPoint *pCover;
	while( pCoverPointList )
	{
		pCover = pCoverPointList;
		pCoverPointList = pCoverPointList->pNextPoint;
		delete pCover;
	}
	pCoverPointList = NULL;
}

void PathFinderAdvanced::SetWaypointCost ( int iIndex, float fCost )
{
	if ( fCost < 0 ) fCost = 0;
	
	sWaypoint *pWaypoint = GetWaypoint ( iIndex );
	if ( pWaypoint ) pWaypoint->fWPCost = fCost;
}

PathFinderAdvanced::sWaypoint* PathFinderAdvanced::GetWaypoint ( int iIndex )
{
	if ( iIndex < 1 || iIndex > iNumWaypoints ) return 0;
	if ( iIndex == 1 ) return pWaypointList;

	sWaypoint *pWaypoint = pWaypointList;
	int iCurrIndex = 1;

	while ( pWaypoint && iCurrIndex != iIndex )
	{
		iCurrIndex++;
		pWaypoint = pWaypoint->pNextWaypoint;
	}

	return pWaypoint;
}

int PathFinderAdvanced::GetWaypointIndex ( sWaypoint* pFindWaypoint )
{
	if ( !pFindWaypoint ) return -1;

	sWaypoint *pWaypoint = pWaypointList;
	int iIndex = 0;

	while ( pWaypoint && pWaypoint != pFindWaypoint )
	{
		pWaypoint = pWaypoint->pNextWaypoint;
		iIndex++;
	}

	if ( pWaypoint ) return iIndex;
	return -1;
}

void PathFinderAdvanced::GetClosestWaypoints ( float fSX, float fSY, float fRadius, Path *pPoints, bool halfheight )
{
	sWaypoint *pWaypoint = pWaypointList;
	
	while ( pWaypoint )
    {
		float fDist = (fSX - pWaypoint->fX)*(fSX - pWaypoint->fX) + (fSY - pWaypoint->fY)*(fSY - pWaypoint->fY);
        if ( fDist < fRadius*fRadius )
		{
			if ( !halfheight || pWaypoint->bHalfHeight )
				pPoints->AddPoint ( pWaypoint->fX, 0, pWaypoint->fY );
		}

		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

int PathFinderAdvanced::CountWaypoints ( )
{
	return iNumWaypoints;
}

void PathFinderAdvanced::RemoveWaypoint ( int iIndex )
{
	if ( iIndex < 1 || iIndex > iNumWaypoints ) return;
	if ( !pWaypointList ) return;

	sWaypoint *pWaypoint = pWaypointList;

	if ( iIndex == 1 )
	{
		sWaypointEdge *pEdge = pWaypointList->pEdgeList;

		while ( pEdge )
		{
			pEdge->pOtherWP->RemoveEdge ( pWaypointList );
			pWaypointList->pEdgeList = pWaypointList->pEdgeList->pNextEdge;
			delete pEdge;
			pEdge = pWaypointList->pEdgeList;
		}
		
		pWaypointList = pWaypointList->pNextWaypoint;
		delete pWaypoint;

		iNumWaypoints--;
		return;
	}

	sWaypoint *pPrevWaypoint = pWaypoint;
	pWaypoint = pWaypoint->pNextWaypoint;
	int iCurrIndex = 2;

	while ( pWaypoint && iCurrIndex != iIndex )
	{
		iCurrIndex++;
		pPrevWaypoint = pWaypoint;
		pWaypoint = pWaypoint->pNextWaypoint;
	}

	if ( pWaypoint )
	{
		sWaypointEdge *pEdge = pWaypoint->pEdgeList;

		while ( pEdge )
		{
			pEdge->pOtherWP->RemoveEdge ( pWaypoint );
			pWaypoint->pEdgeList = pWaypoint->pEdgeList->pNextEdge;
			delete pEdge;
			pEdge = pWaypoint->pEdgeList;
		}

		pPrevWaypoint->pNextWaypoint = pWaypoint->pNextWaypoint;
		delete pWaypoint;

		iNumWaypoints--;
	}
}

void PathFinderAdvanced::MakeMemblockFromWaypoints ( int iMemblockID )
{
	sWaypoint *pWaypoint = pWaypointList;
	int iNumEdges = 0;

	while ( pWaypoint )
	{
		iNumEdges += pWaypoint->iNumEdges;
		pWaypoint = pWaypoint->pNextWaypoint;
	}
	
	int iSize = 8 + iNumWaypoints*12 + iNumEdges*12;
	MakeMemblock ( iMemblockID, iSize );

#pragma warning ( disable : 4312 ) //conversion from 'DWORD' to 'int *' of greater size
	int *pIData = (int*) GetMemblockPtr ( iMemblockID );
	float *pFData = (float*) GetMemblockPtr ( iMemblockID );
#pragma warning ( default : 4312 )

	*pIData = iNumWaypoints;
	*( pIData + 1 ) = iNumEdges;

	int iIndex = 2;

	pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		*( pFData + iIndex )	 = pWaypoint->fX;
		*( pFData + iIndex + 1 ) = pWaypoint->fY;
		*( pFData + iIndex + 2 ) = pWaypoint->fWPCost;
		
		iIndex += 3;
		pWaypoint = pWaypoint->pNextWaypoint;
	}

	pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		sWaypointEdge *pEdge = pWaypoint->pEdgeList;

		while ( pEdge )
		{
			*( pIData + iIndex )	 = GetWaypointIndex( pWaypoint );
			*( pIData + iIndex + 1 ) = GetWaypointIndex( pEdge->pOtherWP );
			*( pFData + iIndex + 2 ) = pEdge->fCost;
			
			iIndex += 3;
			pEdge = pEdge->pNextEdge;
		}

		pWaypoint = pWaypoint->pNextWaypoint;
	}
}

void PathFinderAdvanced::MakeWaypointsFromMemblock ( int iMemblockID )
{
	ClearWaypoints ( );

#pragma warning ( disable : 4312 ) //conversion from 'DWORD' to 'int *' of greater size
	int *pIData = (int*) GetMemblockPtr ( iMemblockID );
	float *pFData = (float*) GetMemblockPtr ( iMemblockID );
#pragma warning ( default : 4312 )

	int iNumPoints = *pIData;
	int iNumEdges = *( pIData + 1 );
	int iIndex = iNumPoints*3 - 1;

	while ( iIndex > 1 )
	{
		float fX	= *( pFData + iIndex );
		float fZ	= *( pFData + iIndex + 1 );
		float fCost = *( pFData + iIndex + 2 );

		AddWaypoint ( fX, fZ, false );
		SetWaypointCost ( 1, fCost );

		iIndex -= 3;
	}

	iIndex = iNumPoints*3 + 2;

	while ( iIndex < ( 1 + iNumPoints*3 + iNumEdges*3 ) )
	{
		int iWaypoint1 = *( pIData + iIndex );
		int iWaypoint2 = *( pIData + iIndex + 1 );
		float fCost = *( pFData + iIndex + 2 );

		sWaypoint *pWaypoint1 = GetWaypoint ( iWaypoint1+1 );
		sWaypoint *pWaypoint2 = GetWaypoint ( iWaypoint2+1 );
		
		if ( pWaypoint1 && pWaypoint2 ) 
		{
			pWaypoint1->AddEdge ( pWaypoint2, fCost );
		}

		iIndex += 3;
	}
}

void PathFinderAdvanced::UpdateVisibility ( float fLimit )
{	
	RemoveAllEdges ( );
	
	sWaypoint *pWaypoint = pWaypointList;
    sWaypoint *pWaypoint2;
    
	while ( pWaypoint )
    {
		pWaypoint->pEdgeList = 0;
        
        pWaypoint2 = pWaypointList;
        
        while ( pWaypoint2 )
        {
			if ( pWaypoint != pWaypoint2 )
			{
				float fX = pWaypoint->fX;
				float fY = pWaypoint->fY;
				float fX2 = pWaypoint2->fX;
				float fY2 = pWaypoint2->fY;

				float fDiffX = fX2 - fX;
				float fDiffY = fY2 - fY;
				float fLength = fDiffX*fDiffX + fDiffY*fDiffY;
				if ( fLength > 0.000001 )
				{
					if ( !QuickPolygonsCheck ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY, 2 ) )
					{
						//check the extensions for walls
						fLength = sqrt(fLength);
						float fNX = fDiffX / fLength;
						float fNY = fDiffY / fLength;

						bool bValid = (fNX*pWaypoint2->fVX + fNY*pWaypoint2->fVY) < pWaypoint2->fCAngle;
						bValid = bValid && ((-fNX)*pWaypoint->fVX + (-fNY)*pWaypoint->fVY) < pWaypoint->fCAngle;

						//if ( !QuickPolygonsCheck ( fX, fY, fNewX, fNewY, 2 ) || !QuickPolygonsCheck ( fX2, fY2, fNewX2, fNewY2, 2 ) )
						if ( bValid )
						{
							float fDist = ActualDistance ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY );
							if ( fLimit < 0 || fDist < fLimit ) 
							{
								pWaypoint->AddEdge ( pWaypoint2, fDist );
							}
						}
					}
				}
			}

			pWaypoint2 = pWaypoint2->pNextWaypoint;
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void PathFinderAdvanced::UpdateSingleVisibility ( PathFinderAdvanced::sWaypoint *pNewWP, float fLimit, bool duplex )
{
	pNewWP->ClearEdges( );
	sWaypoint *pWaypoint = pWaypointList;
    
	while ( pWaypoint )
    {
		if ( ( pWaypoint != pNewWP ) && !QuickPolygonsCheck ( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY, 2 ) )
        {
			float fDist = ActualDistance ( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY );
            if ( fLimit < 0 || fDist < fLimit ) 
			{
				if ( !BlockedByDoor( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY ) )
				{
					pNewWP->AddEdge( pWaypoint, fDist );
					if ( duplex ) pWaypoint->AddEdge( pNewWP, fDist );
				}
			}
        }

		pWaypoint->iVisited = 0;
        pWaypoint->fDistG = 0;
        pWaypoint->fDistH = 0;
        pWaypoint->fDistF = 0;
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void PathFinderAdvanced::RemoveAllEdges ( )
{
	sWaypoint *pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		pWaypoint->ClearEdges ( );
		pWaypoint = pWaypoint->pNextWaypoint;
	}
}

PathFinderAdvanced::PathFinderAdvanced ( Container *pContainer )
{   
    sPolygonData_list.clear ( );
	sPolygonOrigData_list.clear ( );
	sViewBlockingData_list.clear ( );
    //sWaypoint_list.clear ( );
	pWaypointList = 0;
    sWaypointOpen_list.clear ( );

	iCoverPointObject = 0;
	iWaypointObject = 0;
	iWaypointEdgeObject = 0;
	iPolygonBoundsObject = 0;
	iHHPolygonBoundsObject = 0;
	iVBPolygonBoundsObject = 0;
	iGridObject = 0;
	iGridObject2 = 0;

	fCurrRadius = 0.0f;

	pOwner = pContainer;

	fFlashingTimer = 0.0f;

	pOptimizedCol = 0;
	pOptimizedOrigCol = 0;
	memset ( pOptimizedColGrid, 0, sizeof(pOptimizedColGrid) );

	iNumWaypoints = 0;
	fGridRadius = 1.25f;

	if ( !hPathFindingMutex ) hPathFindingMutex = CreateMutex( NULL, FALSE, NULL );
	hBlockerMutex = CreateMutex( NULL, FALSE, NULL );

	bShowAvoidanceGrid = false;
	fAvoidanceGridDebugTimer = 1;
	fAvoidanceGridHeight = 520;

	pBlockerList = NULL;

	pAllDoors = 0;

	pCoverPointList = 0;
}

PathFinderAdvanced::~PathFinderAdvanced ( )
{   
    ClearPolygons ( );
	ClearWaypoints ( );
	ClearCoverPoints ( );

	if ( iCoverPointObject > 0 && ObjectExist ( iCoverPointObject ) == 1 )
	{
		DeleteObject ( iCoverPointObject );
	}

	if ( iWaypointObject > 0 && ObjectExist ( iWaypointObject ) == 1 )
	{
		DeleteObject ( iWaypointObject );
	}

	if ( iWaypointEdgeObject > 0 && ObjectExist ( iWaypointEdgeObject ) == 1 )
	{
		DeleteObject ( iWaypointEdgeObject );
	}

	if ( iPolygonBoundsObject > 0 && ObjectExist ( iPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iPolygonBoundsObject );
	}

	if ( iHHPolygonBoundsObject > 0 && ObjectExist ( iHHPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iHHPolygonBoundsObject );
	}

	if ( iVBPolygonBoundsObject > 0 && ObjectExist ( iVBPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iVBPolygonBoundsObject );
	}

	if ( iGridObject > 0 && ObjectExist ( iGridObject ) == 1 )
	{
		DeleteObject ( iGridObject );
	}

	if ( pOptimizedCol ) delete pOptimizedCol;
	if ( pOptimizedOrigCol ) delete pOptimizedOrigCol;
	for ( int iZ = 0; iZ < 128; iZ++ )
		for ( int iX = 0; iX < 128; iX++ )
			if ( pOptimizedColGrid[iX][iZ] ) 
				delete pOptimizedColGrid[iX][iZ];

	//CloseHandle( hPathFindingMutex );
	if ( hBlockerMutex ) CloseHandle( hBlockerMutex );

	sDoorInfo *pDoor;
	while ( pAllDoors )
	{
		pDoor = pAllDoors;
		pAllDoors = pAllDoors->pNextDoor;
		delete pDoor;
	}
}

void PathFinderAdvanced::Update( float fTimeDelta )
{
	cGrid.Update( fTimeDelta );
	//cUndesirableGrid.Update( fTimeDelta );
}

const Blocker* PathFinderAdvanced::GetBlockerList( ) 
{
	return pBlockerList;
}

sCoverPoint* PathFinderAdvanced::GetCoverPoint( int iID )
{
	sCoverPoint* pCoverPoint = pCoverPointList;

	while( pCoverPoint )
	{
		if ( pCoverPoint->iID == iID ) return pCoverPoint;
		pCoverPoint = pCoverPoint->pNextPoint;
	}

	return 0;
}

void PathFinderAdvanced::SetGridRadius( float radius )
{
	fGridRadius = radius;
	cGrid.Reset( );
	cUndesirableGrid.Reset( );
}

//Paul, I added this as VS10 build could not find 'ftoi'
inline const long ftoi(float x) {
	return (-(long)(x < 0)) + (long)x;
}

int PathFinderAdvanced::GridFtoI( float value )
{
	//return FtoI( floor( value / fGridRadius ) );
	return ftoi( floor( value / fGridRadius ) );
}

float PathFinderAdvanced::GridItoF( int value )
{
	return ( fGridRadius * value ) + fGridRadius/2.0f;
}

bool PathFinderAdvanced::GridSetEntityPosition( float x, float z, int id )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	//if ( cGrid.GetPosition( iX, iZ ) >= 0 )
	{
		cGrid.SetPosition( iX, iZ, id );
		return true;
	}

	return false;
}

void PathFinderAdvanced::GridClearEntityPosition( float x, float z )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	cGrid.DeletePosition( iX,iZ );
}

int PathFinderAdvanced::GridGetPosition( float x, float z )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	return cGrid.GetPosition( iX, iZ );
}

int PathFinderAdvanced::GridGetUndesirablePosition( float x, float z )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	return cUndesirableGrid.GetPosition( iX, iZ );
}

void PathFinderAdvanced::GridSetUndesirablePosition( float x, float z )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	int result = WaitForSingleObject( hBlockerMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Blocker (set) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	cUndesirableGrid.Increment( iX, iZ );

	int value = cUndesirableGrid.GetPosition( iX, iZ );
	if ( value == iUndesirableThreshold )
	{
		//add dynamic blocker
		Blocker *pNewBlocker = new Blocker();
		if ( Entity::iAvoidMode == 5 )
		{
			pNewBlocker->x = GridItoF( iX );
			pNewBlocker->z = GridItoF( iZ );
			pNewBlocker->radius = fGridRadius/2.0f;
		}
		else
		{
			pNewBlocker->x = x;
			pNewBlocker->z = z;
			pNewBlocker->radius = fCurrRadius;
		}
		pNewBlocker->pNextBlocker = pBlockerList;

		pBlockerList = pNewBlocker;
	}

	ReleaseMutex( hBlockerMutex );
}

void PathFinderAdvanced::GridClearUndesirablePosition( float x, float z )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	int result = WaitForSingleObject( hBlockerMutex, INFINITE );
	if ( result != WAIT_OBJECT_0 )
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock Blocker (clear) mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	cUndesirableGrid.Decrement( iX, iZ );
	int value = cUndesirableGrid.GetPosition( iX, iZ );
	if ( value < iUndesirableThreshold )
	{
		//remove dynamic blocker
		Blocker *pBlocker = pBlockerList;
		Blocker *pPrevBlocker = NULL;

		float fMinDist = Entity::iAvoidMode == 5 ? 0.000001f : fCurrRadius;

		while ( pBlocker )
		{
			if ( abs(pBlocker->x - GridItoF(iX)) < fMinDist && abs(pBlocker->z - GridItoF(iZ)) < fMinDist )
			{
				if ( pPrevBlocker ) pPrevBlocker->pNextBlocker = pBlocker->pNextBlocker;
				else pBlockerList = pBlocker->pNextBlocker;

				delete pBlocker;

				if ( pPrevBlocker ) pBlocker = pPrevBlocker->pNextBlocker;
				else pBlocker = pBlockerList;
			}
			else
			{
				pPrevBlocker = pBlocker;
				pBlocker = pBlocker->pNextBlocker;
			}
		}
	}

	ReleaseMutex( hBlockerMutex );
}

void PathFinderAdvanced::AddDynamicBlocker( int id, float x, float z, float radius )
{
	Blocker *pNewBlocker = new Blocker();
	pNewBlocker->id = id;
	pNewBlocker->x = x;
	pNewBlocker->z = z;
	pNewBlocker->radius = radius;
	pNewBlocker->pNextBlocker = pBlockerList;

	pBlockerList = pNewBlocker;
}

void PathFinderAdvanced::RemoveDynamicBlocker( int id )
{
	Blocker *pBlocker = pBlockerList;
	Blocker *pPrevBlocker = NULL;

	while ( pBlocker )
	{
		if ( pBlocker->id == id )
		{
			if ( pPrevBlocker ) pPrevBlocker->pNextBlocker = pBlocker->pNextBlocker;
			else pBlockerList = pBlocker->pNextBlocker;

			delete pBlocker;

			if ( pPrevBlocker ) pBlocker = pPrevBlocker->pNextBlocker;
			else pBlocker = pBlockerList;
		}
		else
		{
			pPrevBlocker = pBlocker;
			pBlocker = pBlocker->pNextBlocker;
		}
	}
}

/*
bool PathFinderAdvanced::GridReservePosition( float x, float z )
{
	return true;
}

bool PathFinderAdvanced::GridUnReservePosition( float x, float z )
{
	return true;
}
*/

int PathFinderAdvanced::GridCheckDirection( int id, float x, float z, float dirX, float dirZ, float destX, float destZ, int ignore, int &iReserved )
{
	float length = sqrt(dirX*dirX + dirZ*dirZ);

	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	int iX2 = GridFtoI( x + dirX );
	int iZ2 = GridFtoI( z + dirZ );

	int iX3 = iX2;
	int iZ3 = iZ2;
	float fX3 = (float) iX3;
	float fZ3 = (float) iZ3;

	float fDiffX = ((fGridRadius*0.6f)*dirX)/(length);
	float fDiffZ = ((fGridRadius*0.6f)*dirZ)/(length);
	
	if ( length > 0.0000001f ) 
	{
		fX3 = x + fDiffX;
		fZ3 = z + fDiffZ;

		iX3 = GridFtoI( fX3 );
		iZ3 = GridFtoI( fZ3 );
	}

	if ( iX == iX3 && iZ == iZ3 ) return 1;

	if ( cUndesirableGrid.GetPosition( iX3, iZ3 ) >= iUndesirableThreshold ) return 0;

	/*
	if ( cUndesirableGrid.GetPosition( iX3, iZ3 ) )
	{
		int iX4 = GridFtoI( destX );
		int iZ4 = GridFtoI( destZ );

		if ( cUndesirableGrid.GetPosition( iX4, iZ4 ) ) return 0;

		int counter = 0;

		do
		{
			fX3 += fDiffX;
			fZ3 += fDiffZ;

			iX3 = GridFtoI( fX3 );
			iZ3 = GridFtoI( fZ3 );

			counter++;
		}
		while ( counter < 20 && cUndesirableGrid.GetPosition( iX3, iZ3 ) );

		if ( counter >= 20 ) return 0;
		else 
		{
			if ( cGrid.GetPosition( iX3, iZ3 ) > 0 || (cGrid.GetPosition( iX3, iZ3 ) == -1 && cGrid.GetReserved(iX3,iZ3) != id) ) return 0;
			//if ( cGrid.GetPosition( iX3, iZ3 ) != 0 ) return 0;
		}

		cGrid.Reserve( iX3, iZ3, id );
		iReserved = 1;
	}
	*/
	
	if ( (iX != iX2 || iZ != iZ2) 
	  && ( cGrid.GetPosition( iX2, iZ2 ) == 0 || (cGrid.GetPosition( iX2, iZ2 ) == -1 && cGrid.GetReserved(iX2,iZ2) == id) || cGrid.GetPosition( iX2, iZ2 ) == ignore ) ) 
	{
		if ( cGrid.GetPosition( iX2, iZ2 ) == -1 ) iReserved = 2;
		return 2;
	}
	if ( cGrid.GetPosition( iX3, iZ3 ) == 0 || (cGrid.GetPosition( iX3, iZ3 ) == -1 && cGrid.GetReserved(iX3,iZ3) == id) || cGrid.GetPosition( iX3, iZ3 ) == ignore ) 
	{
		return 3;
	}
	
	return 0;
}

bool PathFinderAdvanced::GridCheckDestination( float *x, float *z )
{
	int iX = GridFtoI( *x );
	int iZ = GridFtoI( *z );

	if ( cUndesirableGrid.GetPosition( iX, iZ ) == 0 ) return true;

	return false;
}

void PathFinderAdvanced::GridMoveEntity( float x, float z, float newX, float newZ )
{
	int iX = GridFtoI( x );
	int iZ = GridFtoI( z );

	int iX2 = GridFtoI( newX );
	int iZ2 = GridFtoI( newZ );

	if ( iX != iX2 || iZ != iZ2 )
	{
		cGrid.SetPosition( iX2, iZ2, cGrid.GetPosition( iX, iZ ) );
		cGrid.DeletePosition( iX, iZ );
	}
}

PathFinderAdvanced::sWaypoint* PathFinderAdvanced::GetClosestWaypoint ( float fSX, float fSY )
{
	float fClosest = -1.0f;

	sWaypoint *pWaypoint = pWaypointList;
	sWaypoint *pClosestWP = 0;

	while ( pWaypoint )
    {
		float fX = pWaypoint->fX - fSX;
		float fY = pWaypoint->fY - fSY;
		float fDist = fX*fX + fY*fY;

        if ( fDist < fClosest || fClosest < 0.0f )
		{
			fClosest = fDist;
			pClosestWP = pWaypoint;
		}

		pWaypoint = pWaypoint->pNextWaypoint;
    }

	return pClosestWP;
}

void PathFinderAdvanced::GetConnectedPoints ( float x, float z, Path *pPoints )
{
	sWaypoint *pWaypoint = GetClosestWaypoint ( x, z );
	if ( !pWaypoint ) return;
	
	sWaypointEdge *pEdge = pWaypoint->pEdgeList;
	if ( !pEdge ) return;

	while ( pEdge )
	{
		//if ( pWaypoint->pContainer == pEdge->pOtherWP->pContainer )
		{
			pPoints->AddPoint ( pEdge->pOtherWP->fX, 0, pEdge->pOtherWP->fY, pEdge->pOtherWP->pContainer->GetID() );
		}

		pEdge = pEdge->pNextEdge;
	}
}

void PathFinderAdvanced::AddCoverPoint( float fx , float fz , float angle )
{
	sCoverPoint *pNewPoint = new sCoverPoint();

	pNewPoint->fX = fx;
	pNewPoint->fY = fz;
	pNewPoint->fAngle = angle;
	pNewPoint->iManualCoverPoint = 1;
						
	// calculate cover direction using the perpendicular of the obstacle edge
	pNewPoint->fDirX = cos( angle*0.017453f );
	pNewPoint->fDirY = -sin( angle*0.017453f );

	pNewPoint->iID = coverID++;
	pNewPoint->iContainer = pOwner->GetID();
	pNewPoint->bFlags = 0;

	//Dave, took out leaping for now
	//*if ( pIter->bDiveOver ) 
	//{
	//	// may be a better way to calculate the distance required to 
	//	this obstacle?
	//	pNewPoint->fLeapDist = length2*2; // double the distance to the center, assumes a roughly symetrical obstacle
	//
	//	// check leap point isn't in a polygon
	//	float checkX = pNewPoint->fX + pNewPoint->fDirX*pNewPoint->fLeapDist;
	//	float checkY = pNewPoint->fY + pNewPoint->fDirY*pNewPoint->fLeapDist;
	//	if ( InPolygons( checkX, checkY ) > 0 ) pNewPoint->fLeapDist = -1;
	//}
	//else 
		pNewPoint->fLeapDist = -1; // can't leap

	pNewPoint->pNextPoint = pCoverPointList;
	pCoverPointList = pNewPoint;
}

void PathFinderAdvanced::BuildWaypoints( )
{
	// clear old waypoints and coverpoints
	cWorld.ClearCoverPoints();
	if ( sPolygonData_list.size ( ) == 0 ) return;
    ClearWaypoints ( );
    sWaypointOpen_list.clear ( );
	ClearCoverPoints();

	// vars
	int howManyCoverPoints = 0;
    //vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
	vector<sVertexData>::iterator vIter;
	vector<sVertexData>::iterator vIterPrev;
	coverID = 1;
    
    // get valid vertices
	int iIterIndex = 0;
    while ( iIterIndex < sPolygonData_list.size() ) // pIter < sPolygonData_list.end ( ) )
    {
		sPolygonData* pIter = &sPolygonData_list[iIterIndex];
		if ( pIter->sVertexData_list.size ( ) > 1 )
		{
			/* cover points now for LUA, not waypoint system
			// find weighted center point of obstacle, used for cover points
			float fCenterX = 0;
			float fCenterY = 0;
			if ( pIter->bHalfHeight )
			{
				vIter = pIter->sVertexData_list.begin ( );
				float fTotalLength = 0;
				while ( vIter < pIter->sVertexData_list.end ( ) )
				{
					fTotalLength += vIter->fLength;
					float lineX = (vIter->fX + vIter->fX + vIter->fNormVX*vIter->fLength) / 2;
					float lineY = (vIter->fY + vIter->fY + vIter->fNormVY*vIter->fLength) / 2;
					fCenterX += lineX*vIter->fLength;
					fCenterY += lineY*vIter->fLength;
					vIter++;
				}
				fCenterX /= fTotalLength;
				fCenterY /= fTotalLength;
			}
			float fCoverPointSpacing = 2*fCurrRadius;
			float fCoverDistRemaining = fCoverPointSpacing;
			*/

			vIter = pIter->sVertexData_list.begin ( );
			vIterPrev = pIter->sVertexData_list.end ( ) - 1;			
			while ( vIter < pIter->sVertexData_list.end ( ) )
			{
				/* cover points now for LUA, not waypoint data
				// only add cover points to half height obstacles
				if ( pIter->bHalfHeight )
				{
					bool foundClose = false;
					if ( howManyCoverPoints > MAX_COVER_POINTS )
					{
						foundClose = true;
						break;
					}
					float angle = vIter->fNormVX*vIterPrev->fNormVX + vIter->fNormVY*vIterPrev->fNormVY;
					if ( angle < 0.94f ) fCoverDistRemaining = fCoverPointSpacing; // about 20 degrees deviation resets the cover counter

					// is this edge long enough for a cover point
					if ( vIter->fLength < fCoverDistRemaining ) 
						fCoverDistRemaining -= vIter->fLength;
					else
					{
						// create cover points along this edge
						float length = fCoverDistRemaining;
						while( length < vIter->fLength && foundClose == false )
						{
							if ( foundClose ) continue;

							sCoverPoint *pNewPoint = new sCoverPoint();
							foundClose = true;
							howManyCoverPoints++;
							pNewPoint->fX = vIter->fX + vIter->fNormVX*(vIter->fLength/2.0f);
							pNewPoint->fY = vIter->fY + vIter->fNormVY*(vIter->fLength/2.0f);

							float distToCenterX = fCenterX - pNewPoint->fX;
							float distToCenterY = fCenterY - pNewPoint->fY;
							float length2 = sqrt(distToCenterX*distToCenterX + distToCenterY*distToCenterY);
							if ( length2 > 0 ) 
							{
								distToCenterX /= length2;
								distToCenterY /= length2;
							}
							
							// calculate cover direction using the perpendicular of the obstacle edge
							pNewPoint->fDirX = vIter->fNormVY;
							pNewPoint->fDirY = -vIter->fNormVX;
							pNewPoint->iID = coverID++;
							pNewPoint->iContainer = pOwner->GetID();
							pNewPoint->bFlags = 0;
							pNewPoint->fLeapDist = -1; // can't leap
							pNewPoint->pNextPoint = pCoverPointList;
							pCoverPointList = pNewPoint;
							length += fCoverPointSpacing;
						}
						fCoverDistRemaining = length - vIter->fLength;
					}
				}
				*/

				// add waypoint
				float fX = vIter->fX + ( vIterPrev->fNormVX - vIter->fNormVX )*0.2f*fCurrRadius;
				float fY = vIter->fY + ( vIterPrev->fNormVY - vIter->fNormVY )*0.2f*fCurrRadius;
				bool bValid = ( InPolygons ( fX, fY ) == 0 ); // 050115 - consumes 80% of Obstacle bake process!!
				float fSide = ( vIterPrev->fNormVX * vIter->fNormVY ) - ( vIterPrev->fNormVY * vIter->fNormVX );
				if ( bValid && fSide < 0 )
				{
					float vx = vIter->fNormVX - vIterPrev->fNormVX;
					float vy = vIter->fNormVY - vIterPrev->fNormVY;
					float length = sqrt(vx*vx + vy*vy);
					if ( length > 0 )
					{
						vx = vx / length;
						vy = vy / length;
					}
					float ca = vx*vIter->fNormVX + vy*vIter->fNormVY;
					AddWaypoint( fX, fY, pIter->bHalfHeight, false, vx, vy, ca, false, 0 );
				}
				vIterPrev = vIter;
				vIter++;
			}
		}
        //pIter++;
		iIterIndex++;
    }

	// add cover points (nope, now used directly in LUA script)
	//for ( int c = 0 ; c < (int)tempCoverPoints.size(); c++ )
	//	AddCoverPoint ( tempCoverPoints[c].fx , tempCoverPoints[c].fz , tempCoverPoints[c].angle );
	// 260216 - list kept in tact (as cover zones extracted to LUA, not using automated system)
	// tempCoverPoints.clear();

	// update visibility for AI
	UpdateVisibility(-1);
}

void PathFinderAdvanced::ClearPolygons ( )
{
    for ( int i = 0; i < (int) sPolygonData_list.size ( ); i++ )
        sPolygonData_list [ i ].sVertexData_list.clear ( );
    
    sPolygonData_list.clear ( );
	sPolygonOrigData_list.clear ( );
	sViewBlockingData_list.clear ( );
}

void PathFinderAdvanced::SaveObstacleData( char *pFilename )
{
	// save OBS file
	if ( !pFilename ) return;
	FILE *pFile;
	int error = GG_fopen_s( &pFile, pFilename, "wb" );
	if ( error )
	{
		char str [ 256 ];
		if ( error >= _sys_nerr ) sprintf_s( str, 256, "Unknown error while saving obstacles: %d", error );
		else sprintf_s( str, 256, "Error while saving obstacles: %s", _sys_errlist [ error ] );
		MessageBox( NULL, str, "AI Error", 0 );
		if ( pFile ) fclose( pFile );
		return;
	}

	// save polygon list
	int iNumPolys = (int) sPolygonOrigData_list.size();
	fwrite( &iNumPolys, sizeof(int), 1, pFile );
	for ( int i = 0; i < (int) sPolygonOrigData_list.size ( ); i++ )
    {
        int iNumV = (int) sPolygonOrigData_list [ i ].sVertexData_list.size ( );
		bool bBlocksPath = sPolygonOrigData_list [ i ].bBlocksPath;
		bool bBlocksView = sPolygonOrigData_list [ i ].bBlocksView;
		bool bHalfHeight = sPolygonOrigData_list [ i ].bHalfHeight;
		int iID = sPolygonOrigData_list [ i ].id;
		bool bTerrain = sPolygonOrigData_list [ i ].bTerrain;
		fwrite( &iNumV, sizeof(int), 1, pFile );
		fwrite( &bBlocksPath, sizeof(bool), 1, pFile );
		fwrite( &bBlocksView, sizeof(bool), 1, pFile );
		fwrite( &bHalfHeight, sizeof(bool), 1, pFile );
		fwrite( &iID, sizeof(int), 1, pFile );
		fwrite( &bTerrain, sizeof(bool), 1, pFile );
        for ( int j = 0; j < iNumV; j++ )
        {
			float fNormVX = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fNormVX;
			float fNormVY = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fNormVY;
			float fX = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fX;
			float fY = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fY;
			float fLength = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fLength;
			fwrite( &fNormVX, sizeof(float), 1, pFile );
			fwrite( &fNormVY, sizeof(float), 1, pFile );
			fwrite( &fX, sizeof(float), 1, pFile );
			fwrite( &fY, sizeof(float), 1, pFile );
			fwrite( &fLength, sizeof(float), 1, pFile );
		}
	}

	// update edge count (some reason it can be wrong at this point)
	sWaypoint* pCurrentWP = pWaypointList;
	for ( int i = 0; i < iNumWaypoints; i++ )
    {
		int iWaypointEdgeCount = 0;
		sWaypointEdge* pWPE = pCurrentWP->pEdgeList;
		while ( pWPE )
		{
			iWaypointEdgeCount++;
			pWPE = pWPE->pNextEdge;
		}
		pCurrentWP->iNumEdges = iWaypointEdgeCount;
		pCurrentWP = pCurrentWP->pNextWaypoint;
	}

	// save in waypoint list data
	fwrite( &iNumWaypoints, sizeof(int), 1, pFile );
	pCurrentWP = pWaypointList;
	for ( int i = 0; i < iNumWaypoints; i++ )
    {
		fwrite( &pCurrentWP->fX, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fY, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->bHalfHeight, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->bActive, sizeof(bool), 1, pFile );
		fwrite( &pCurrentWP->bIsBridge, sizeof(bool), 1, pFile );
		fwrite( &pCurrentWP->iVisited, sizeof(int), 1, pFile );
		fwrite( &pCurrentWP->bCanPeek, sizeof(bool), 1, pFile );
		fwrite( &pCurrentWP->fCAngle, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fVX, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fVY, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->bFlags, sizeof(char), 1, pFile );
		fwrite( &pCurrentWP->fDistF, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fDistG, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fDistH, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->fWPCost, sizeof(float), 1, pFile );
		fwrite( &pCurrentWP->iNumEdges, sizeof(int), 1, pFile );
		fwrite( &pCurrentWP->iVisited, sizeof(int), 1, pFile );
		//pCurrentWP->pContainer = NULL;
		int iContainerID = 0;
		if ( pCurrentWP->pContainer )
			iContainerID = pCurrentWP->pContainer->GetID();
		fwrite( &iContainerID, sizeof(int), 1, pFile );
		//pCurrentWP->pParent = NULL;
		// edge ptr later on
		pCurrentWP = pCurrentWP->pNextWaypoint;
	}

	// now save edge list now we know all waypoint list
	pCurrentWP = pWaypointList;
	for ( int i = 0; i < iNumWaypoints; i++ )
    {
		//pCurrentWP->pEdgeList = NULL;
		sWaypointEdge* pWPE = pCurrentWP->pEdgeList;
		int iWaypointEdgeCount = 0;
		while ( pWPE )
		{
			iWaypointEdgeCount++;
			pWPE = pWPE->pNextEdge;
		}
		fwrite( &iWaypointEdgeCount, sizeof(int), 1, pFile );
		pWPE = pCurrentWP->pEdgeList;
		for ( int j=0; j<iWaypointEdgeCount; j++ )
		{
			fwrite( &pWPE->fCost, sizeof(float), 1, pFile );
			int iFoundWaypoint = 0;
			sWaypoint* pFindWP = pWaypointList;
			while ( pFindWP )
			{
				if ( pFindWP==pWPE->pOtherWP ) break;
				iFoundWaypoint++;
				pFindWP = pFindWP->pNextWaypoint;
			}
			fwrite( &iFoundWaypoint, sizeof(int), 1, pFile );
			//pWPE->pDoors
			pWPE = pWPE->pNextEdge;
		}
		pCurrentWP = pCurrentWP->pNextWaypoint;
	}

	// save in coverpoint data
	int iNumCoverpoints = 0;
	sCoverPoint* pCurrentCP = pCoverPointList;
	while ( pCurrentCP ) 
	{
		iNumCoverpoints++;
		pCurrentCP = pCurrentCP->pNextPoint;
	}
	fwrite( &iNumCoverpoints, sizeof(int), 1, pFile );
	pCurrentCP = pCoverPointList;
	for ( int i = 0; i < iNumCoverpoints; i++ )
    {
		fwrite( &pCurrentCP->fX, sizeof(float), 1, pFile );
		fwrite( &pCurrentCP->fY, sizeof(float), 1, pFile );
		fwrite( &pCurrentCP->fDirX, sizeof(float), 1, pFile );
		fwrite( &pCurrentCP->fDirY, sizeof(float), 1, pFile );
		fwrite( &pCurrentCP->fLeapDist, sizeof(float), 1, pFile );
		fwrite( &pCurrentCP->iContainer, sizeof(int), 1, pFile );
		fwrite( &pCurrentCP->iID, sizeof(int), 1, pFile );
		fwrite( &pCurrentCP->bFlags, sizeof(char), 1, pFile );
		pCurrentCP = pCurrentCP->pNextPoint;
	}

	// finalise and close file
	fclose( pFile );
}

void PathFinderAdvanced::LoadObstacleData( char *pFilename )
{
	// open OBS file
	if ( !pFilename ) return;
	FILE *pFile;
	int error = GG_fopen_s( &pFile, pFilename, "rb" );
	if ( error )
	{
		char str [ 256 ];
		if ( error >= _sys_nerr ) sprintf_s( str, 256, "Unknown error while loading obstacles: %d", error );
		else sprintf_s( str, 256, "Error while loading obstacles: %s", _sys_errlist [ error ] );
		MessageBox( NULL, str, "AI Error", 0 );
		if ( pFile ) fclose( pFile );
		return;
	}

	// go through all polygons stored
	int iNumPolys;
	fread( &iNumPolys, sizeof(int), 1, pFile );
	sPolygonData sNewPolygon;
	sVertexData sNewVertex;
	for ( int i = 0; i < iNumPolys; i++ )
    {
		sNewPolygon.sVertexData_list.clear( );
		int iNumV;
		bool bBlocksPath;
		bool bBlocksView;
		bool bHalfHeight;
		int iID;
		bool bTerrain;
		fread( &iNumV, sizeof(int), 1, pFile );
		fread( &bBlocksPath, sizeof(bool), 1, pFile );
		fread( &bBlocksView, sizeof(bool), 1, pFile );
		fread( &bHalfHeight, sizeof(bool), 1, pFile );
		fread( &iID, sizeof(int), 1, pFile );
		fread( &bTerrain, sizeof(bool), 1, pFile );

		// add polygon to main list
		sNewPolygon.bBlocksPath = bBlocksPath;
		sNewPolygon.bBlocksView = bBlocksView;
		sNewPolygon.bHalfHeight = bHalfHeight;
		sNewPolygon.id = iID;
		sNewPolygon.bTerrain = bTerrain;
		for ( int j = 0; j < iNumV; j++ )
        {
			float fNormVX;
			float fNormVY;
			float fX;
			float fY;
			float fLength;
			fread( &fNormVX, sizeof(float), 1, pFile );
			fread( &fNormVY, sizeof(float), 1, pFile );
			fread( &fX, sizeof(float), 1, pFile );
			fread( &fY, sizeof(float), 1, pFile );
			fread( &fLength, sizeof(float), 1, pFile );
			sNewVertex.fNormVX = fNormVX;
			sNewVertex.fNormVY = fNormVY;
			sNewVertex.fX = fX;
			sNewVertex.fY = fY;
			sNewVertex.fLength = fLength;
			sNewPolygon.sVertexData_list.push_back( sNewVertex );
		}
		sPolygonOrigData_list.push_back( sNewPolygon );
		sPolygonData_list.push_back( sNewPolygon );
	}

	// optimize obstacle data (handle radius and build optimized lists)
	OptimizeObstacles();

	// load in waypoint list data
    ClearWaypoints ( );
    sWaypointOpen_list.clear ( );
	pWaypointList = NULL;
	fread( &iNumWaypoints, sizeof(int), 1, pFile );
	sWaypoint* pLastWaypoint = NULL;
	for ( int i = 0; i < iNumWaypoints; i++ )
    {
		sWaypoint* pNewWP = new sWaypoint();
		memset ( pNewWP, 0, sizeof ( sWaypoint ) );
		fread( &pNewWP->fX, sizeof(float), 1, pFile );
		fread( &pNewWP->fY, sizeof(float), 1, pFile );
		fread( &pNewWP->bHalfHeight, sizeof(float), 1, pFile );
		fread( &pNewWP->bActive, sizeof(bool), 1, pFile );
		fread( &pNewWP->bIsBridge, sizeof(bool), 1, pFile );
		fread( &pNewWP->iVisited, sizeof(int), 1, pFile );
		fread( &pNewWP->bCanPeek, sizeof(bool), 1, pFile );
		fread( &pNewWP->fCAngle, sizeof(float), 1, pFile );
		fread( &pNewWP->fVX, sizeof(float), 1, pFile );
		fread( &pNewWP->fVY, sizeof(float), 1, pFile );
		fread( &pNewWP->bFlags, sizeof(char), 1, pFile );
		fread( &pNewWP->fDistF, sizeof(float), 1, pFile );
		fread( &pNewWP->fDistG, sizeof(float), 1, pFile );
		fread( &pNewWP->fDistH, sizeof(float), 1, pFile );
		fread( &pNewWP->fWPCost, sizeof(float), 1, pFile );
		fread( &pNewWP->iNumEdges, sizeof(int), 1, pFile );
		fread( &pNewWP->iVisited, sizeof(int), 1, pFile );
		int iContainerID = 0;
		fread( &iContainerID, sizeof(int), 1, pFile );
		pNewWP->pContainer = cWorld.GetContainer(iContainerID);
		pNewWP->pParent = NULL;

		// 060915 - ensures waypoints are in order they where saved out as
		if ( pWaypointList == NULL )
		{
			// first waypoint
			pWaypointList = pNewWP;
			pLastWaypoint = pWaypointList;
		}
		else
		{
			// add new waypoint to end of list
			pLastWaypoint->pNextWaypoint = pNewWP;
			pLastWaypoint = pNewWP;
		}
	}

	// now load edge data now we have a complete waypoint list
	sWaypoint* pNewWP = pWaypointList;
	for ( int i = 0; i < iNumWaypoints; i++ )
    {
		// large delay here within a potentially large loop, so keep any MP alive
		mp_refresh ( );

		pNewWP->pEdgeList = NULL;
		int iWaypointEdgeCount = 0;
		fread( &iWaypointEdgeCount, sizeof(int), 1, pFile );
		for ( int j=0; j<iWaypointEdgeCount; j++ )
		{
			sWaypointEdge* pNewWPEdge = new sWaypointEdge();
			memset ( pNewWPEdge, 0, sizeof ( sWaypointEdge ) );
			fread( &pNewWPEdge->fCost, sizeof(float), 1, pFile );
			int iFindWaypoint = 0;
			fread( &iFindWaypoint, sizeof(int), 1, pFile );
			sWaypoint* pFindWP = pWaypointList;
			int iFoundWaypoint = 0;
			while ( pFindWP )
			{
				if ( iFoundWaypoint==iFindWaypoint ) 
				{
					pNewWPEdge->pOtherWP = pFindWP;
					break;
				}
				iFoundWaypoint++;
				pFindWP = pFindWP->pNextWaypoint;
			}
			pNewWPEdge->pDoors = NULL;
			pNewWPEdge->pNextEdge = pNewWP->pEdgeList;
			pNewWP->pEdgeList = pNewWPEdge;
		}
		pNewWP = pNewWP->pNextWaypoint;
	}

	// load in coverpoint data
	ClearCoverPoints();
	cWorld.ClearCoverPoints();
	pCoverPointList = NULL;
	int iNumCoverpoints = 0;
	fread( &iNumCoverpoints, sizeof(int), 1, pFile );
	sCoverPoint* pLastControlpoint = NULL;
	for ( int i = 0; i < iNumCoverpoints; i++ )
    {
		sCoverPoint* pNewCP = new sCoverPoint();
		memset ( pNewCP, 0, sizeof ( sCoverPoint ) );
		fread( &pNewCP->fX, sizeof(float), 1, pFile );
		fread( &pNewCP->fY, sizeof(float), 1, pFile );
		fread( &pNewCP->fDirX, sizeof(float), 1, pFile );
		fread( &pNewCP->fDirY, sizeof(float), 1, pFile );
		fread( &pNewCP->fLeapDist, sizeof(float), 1, pFile );
		fread( &pNewCP->iContainer, sizeof(int), 1, pFile );
		fread( &pNewCP->iID, sizeof(int), 1, pFile );
		fread( &pNewCP->bFlags, sizeof(char), 1, pFile );

		// 060915 - ensures controlpoint are in order they where saved out as
		if ( pCoverPointList == NULL )
		{
			// first controlpoint
			pCoverPointList = pNewCP;
			pLastControlpoint = pCoverPointList;
		}
		else
		{
			// add new controlpoint to end of list
			pLastControlpoint->pNextPoint = pNewCP;
			pLastControlpoint = pNewCP;
		}

		pNewCP->pNextPoint = pCoverPointList;
		pCoverPointList = pNewCP;
	}

	// finish file and close
	fclose( pFile );
}

int PathFinderAdvanced::CountObstacles( )
{
	vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter;
    
	int iCount = 0;
    
    while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 )
		{
			pIter++;
			continue;
		}

		iCount++;
		pIter++;
	}

	return iCount;
}

int PathFinderAdvanced::CountEdges( )
{
	vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter;
    
	int iCount = 0;
    
    while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 )
		{
			pIter++;
			continue;
		}

		vIter = pIter->sVertexData_list.begin ( );
        
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
			iCount++;
			vIter++;
		}

		pIter++;
	}

	return iCount;
}

PathFinderAdvanced::sPolygonData PathFinderAdvanced::ConvertPolygon ( Polygon2D *const pPolygon, bool* bSucceed )
{
	*bSucceed = false;
	sPolygonData sNewPolygon;
	sVertexData sNewVertex;

	int iNumV = pPolygon->CountVertices ( );
    if ( iNumV < 2 ) return sNewPolygon;   

    float fX = pPolygon->GetVertex ( 0 ).x;
    float fY = pPolygon->GetVertex ( 0 ).y;
    float fVX, fVY;
    float fLength;
    int iNext;
    
    for ( int j = 0; j < iNumV; j++ )
    {
        if ( j + 1 == iNumV ) iNext = 0;
        else iNext = j + 1;
        
        fVX = pPolygon->GetVertex ( iNext ).x - fX;
        fVY = pPolygon->GetVertex ( iNext ).y - fY;
         
        sNewVertex.fX = fX; 
        sNewVertex.fY = fY;
         
        fX += fVX;
        fY += fVY;
         
        fLength = sqrt ( fVX*fVX + fVY*fVY );
        if ( fabs ( fLength ) < 0.0001 ) fLength = 1.0;
         
        sNewVertex.fNormVX = fVX / fLength;
        sNewVertex.fNormVY = fVY / fLength;
		sNewVertex.fLength = fLength;
        
        sNewPolygon.sVertexData_list.push_back ( sNewVertex );
    }
    //all data for this polygon copied, edges normalised
    
    //check for sharp corners and add vertices to smooth
    vector<sVertexData>::iterator vIter = sNewPolygon.sVertexData_list.begin ( );
    vector<sVertexData>::iterator vIterPrev = sNewPolygon.sVertexData_list.end ( ) - 1;
    float fVX2, fVY2;              //stores the vector of the last edge
    float fRel, fRel2;             //stores the relative direction between the two
    
    while ( vIter < sNewPolygon.sVertexData_list.end ( ) )
    {
        fVX = vIter->fNormVX;
        fVY = vIter->fNormVY;
        
        fVX2 = vIterPrev->fNormVX;
        fVY2 = vIterPrev->fNormVY;
        
        fRel = ( fVX2 * fVY ) - ( fVY2 * fVX );
		fRel2 = ( fVX * fVX2 ) + ( fVY * fVY2 );
        
        //negative is a right hand bend and, assuming clockwise winding order, needs padding
		//anything over about 95 degrees should be smoothed
        if ( fRel <= 0.0f && fRel2 < -0.1f )
        {
            //new vertex position is the same as this edge
            sNewVertex.fX = vIter->fX;
            sNewVertex.fY = vIter->fY;
            
            //it's vector is the one that would flatten the corner
            //sum of edge vectors, normalised
            float fNewVX = fVX + fVX2;
            float fNewVY = fVY + fVY2;
            
            fLength = sqrt ( fNewVX*fNewVX + fNewVY*fNewVY );
            if ( fLength < 0.0001 ) 
			{
				fNewVX = fVY2;
				fNewVY = -fVX2;
				fLength = 1.0;
			}
            
            sNewVertex.fNormVX = fNewVX / fLength;
            sNewVertex.fNormVY = fNewVY / fLength;
			sNewVertex.fLength = fLength;
            
            vIter = sNewPolygon.sVertexData_list.insert ( vIter, sNewVertex ) + 1;
        }          
        
        vIterPrev = vIter;
        if ( vIter < sNewPolygon.sVertexData_list.end ( ) ) vIter++;
    }
    
	sNewPolygon.bHalfHeight = pPolygon->bHalfHeight;
	sNewPolygon.id = pPolygon->id;

	*bSucceed = true;
    return sNewPolygon;
}

void PathFinderAdvanced::AddPolygon ( Polygon2D *const pPolygon )
{
	AddPolygon ( pPolygon, true );
}

void PathFinderAdvanced::AddPolygon ( Polygon2D *const pPolygon, bool bUpdate )
{
	bool bSucceed = false;
	sPolygonData sNewPolygon = ConvertPolygon ( pPolygon, &bSucceed );
	if ( !bSucceed ) return;

	sNewPolygon.bBlocksPath = true;
	sNewPolygon.bBlocksView = true;
	sNewPolygon.bTerrain = pPolygon->bTerrain;
	sNewPolygon.bDiveOver = pPolygon->bDiveOver;

	sPolygonOrigData_list.push_back ( sNewPolygon );	//original data
	sPolygonData_list.push_back ( sNewPolygon );
	
	//if ( fCurrRadius > 0 && bUpdate ) SetRadius ( fCurrRadius );
}

void PathFinderAdvanced::AddViewBlocker ( Polygon2D *const pPolygon )
{
	bool bSucceed = false;
	sPolygonData sNewPolygon = ConvertPolygon ( pPolygon, &bSucceed );
	if ( !bSucceed ) return;

	sNewPolygon.bBlocksPath = false;
	sNewPolygon.bBlocksView = true;

	//sPolygonOrigData_list.push_back ( sNewPolygon );	//original data
	sViewBlockingData_list.push_back ( sNewPolygon );
	
	//sNewPolygon.sVertexData_list.clear( );
	//sPolygonData_list.push_back ( sNewPolygon );
	
	//if ( fCurrRadius > 0 ) SetRadius ( fCurrRadius );
}

void PathFinderAdvanced::SetRadius ( float fRadius )
{
	//expand polygons out by the collision radius
	for ( int i = 0; i < (int) sPolygonOrigData_list.size ( ); i++ )
    {
        int iNumV = (int) sPolygonOrigData_list [ i ].sVertexData_list.size ( );
		if ( !sPolygonOrigData_list [ i ].bBlocksPath ) continue;
        
        float fVec0X, fVec0Y;    //prev edge vector
        float fVec1X, fVec1Y;    //curr edge vector
        float fDotP;
		float fX, fY;
        
        fVec0X = sPolygonOrigData_list [ i ].sVertexData_list [ iNumV - 1 ].fNormVX;
        fVec0Y = sPolygonOrigData_list [ i ].sVertexData_list [ iNumV - 1 ].fNormVY;
        
        for ( int j = 0; j < iNumV; j++ )
        {
            fVec1X = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fNormVX;
            fVec1Y = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fNormVY;

			fX = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fX;
			fY = sPolygonOrigData_list [ i ].sVertexData_list [ j ].fY;
            
			if ( fabs( fVec1X*fVec0Y - fVec1Y*fVec0X ) < 0.0000001 ) 
			{
				sPolygonData_list [ i ].sVertexData_list [ j ].fX = fX - ( fRadius * fVec1Y );
				sPolygonData_list [ i ].sVertexData_list [ j ].fY = fY + ( fRadius * fVec1X );				
			}
            else 
			{
				fDotP = fRadius / ( fVec1X*fVec0Y - fVec1Y*fVec0X );
            
				sPolygonData_list [ i ].sVertexData_list [ j ].fX = fX + ( ( fDotP * fVec0X ) - ( fDotP * fVec1X ) );
				sPolygonData_list [ i ].sVertexData_list [ j ].fY = fY + ( ( fDotP * fVec0Y ) - ( fDotP * fVec1Y ) );
			}
        
            fVec0X = fVec1X;
            fVec0Y = fVec1Y;
        }
    }

	fCurrRadius = fRadius;
	SetGridRadius( fRadius * 2.5f );
	//SetGridRadius( 100.0f );

	//BuildWaypoints ( );
}

void PathFinderAdvanced::RemovePolygon ( int id )
{
	bool bRebuild = false;
	
	for ( int i=0; i < (int) sPolygonData_list.size( ); i++ )
	{
		if ( sPolygonData_list [ i ].id == id )
		{
			if ( sPolygonData_list [ i ].bBlocksPath ) bRebuild = true;
			sPolygonData_list.erase ( sPolygonData_list.begin( ) + i );
			i--;
		}
	}

	for ( int i=0; i < (int) sPolygonOrigData_list.size( ); i++ )
	{
		if ( sPolygonOrigData_list [ i ].id == id )
		{
			if ( sPolygonOrigData_list [ i ].bBlocksPath ) bRebuild = true;
			sPolygonOrigData_list.erase ( sPolygonOrigData_list.begin( ) + i );
			i--;
		}
	}

	for ( int i=0; i < (int) sViewBlockingData_list.size( ); i++ )
	{
		if ( sViewBlockingData_list [ i ].id == id )
		{
			sViewBlockingData_list.erase ( sViewBlockingData_list.begin( ) + i );
			i--;
		}
	}

	//if ( bRebuild ) BuildWaypoints ( );
}

bool PathFinderAdvanced::BlockedByDoor( float x1, float y1, float x2, float y2 )
{
	sDoorInfo *pDoor = pAllDoors;
	while ( pDoor )
	{
		float result = CheckRay( pDoor->x1, pDoor->y1, pDoor->x2-pDoor->x1, pDoor->y2-pDoor->y1, x1,y1, x2-x1, y2-y1 );
		if ( result >= 0 ) return true;

		pDoor = pDoor->pNextDoor;
	}

	return false;
}

void PathFinderAdvanced::AddDoor ( int id, float x1, float y1, float x2, float y2 )
{
	// each edge contains a list of doors blocking it
	sWaypoint *pWaypoint = pWaypointList;
	while ( pWaypoint )
	{
		sWaypointEdge *pEdge = pWaypoint->pEdgeList;
		while ( pEdge )
		{
			sWaypoint *pOther = pEdge->pOtherWP;
			float diffX = pOther->fX - pWaypoint->fX;
			float diffY = pOther->fY - pWaypoint->fY;

			float result = CheckRay( x1,y1, x2-x1, y2-y1, pWaypoint->fX, pWaypoint->fY, diffX, diffY );
			if ( result >= 0 )
			{
				// found an edge blocked by this door
				sDoor *pDoor = new sDoor();
				pDoor->id = id;
				pDoor->pNextDoor = pEdge->pDoors;
				pEdge->pDoors = pDoor;
			}
			
			pEdge = pEdge->pNextEdge;
		}

		pWaypoint = pWaypoint->pNextWaypoint;
	}

	// add door to global list of doors
	sDoorInfo *pDoor2 = new sDoorInfo();
	pDoor2->id = id;
	pDoor2->x1 = x1;
	pDoor2->y1 = y1;
	pDoor2->x2 = x2;
	pDoor2->y2 = y2;
	pDoor2->pNextDoor = pAllDoors;
	pAllDoors = pDoor2;
}

void PathFinderAdvanced::RemoveDoor ( int id )
{
	// remove door from every waypoint edge
	sWaypoint *pWaypoint = pWaypointList;
	while ( pWaypoint )
	{
		sWaypointEdge *pEdge = pWaypoint->pEdgeList;
		while ( pEdge )
		{
			sDoor *pDoor = pEdge->pDoors;
			sDoor *pLast = 0;
			while ( pDoor )
			{
				sDoor *pNext = pDoor->pNextDoor;

				if ( pDoor->id == id )
				{
					if ( pLast ) pLast->pNextDoor = pNext;
					else pEdge->pDoors = pNext;
					
					delete pDoor;
				}
				else pLast = pDoor;
					
				pDoor = pNext;
			}
			
			pEdge = pEdge->pNextEdge;
		}

		pWaypoint = pWaypoint->pNextWaypoint;
	}

	// remove from global list of doors
	sDoorInfo *pDoor2 = pAllDoors;
	sDoorInfo *pLast = 0;
	sDoorInfo *pNext = 0;
	while ( pDoor2 )
	{
		pNext = pDoor2->pNextDoor;

		if ( pDoor2->id == id )
		{
			if ( pLast ) pLast->pNextDoor = pNext;
			else pAllDoors = pNext;

			delete pDoor2;
		}
		else pLast = pDoor2;

		pDoor2 = pNext;
	}
}

void PathFinderAdvanced::OptimizeObstacles( )
{	
	// delete old optimized col lists
	if ( pOptimizedCol ) delete pOptimizedCol;
	if ( pOptimizedOrigCol ) delete pOptimizedOrigCol;
	for ( int iZ = 0; iZ < 128; iZ++ )
		for ( int iX = 0; iX < 128; iX++ )
			if ( pOptimizedColGrid[iX][iZ] ) 
				delete pOptimizedColGrid[iX][iZ];

	// set radius for optimized col lists
	if ( fCurrRadius > 0 ) SetRadius( fCurrRadius );
	
	// create pOptimizedCol from sPolygonData_list
	pOptimizedCol = 0;
	vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter, vIterPrev;
	int iNumEdges = 0;
	EdgeData *pEdgeList = 0;
    while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
		vIter = pIter->sVertexData_list.begin ( );
        vIterPrev = pIter->sVertexData_list.end ( ) - 1;
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            EdgeData *pNewEdge = new EdgeData( );
			pNewEdge->bHalfHeight = pIter->bHalfHeight;
			pNewEdge->fNormVX = vIterPrev->fNormVX;
			pNewEdge->fNormVY = vIterPrev->fNormVY;
			pNewEdge->fX = vIterPrev->fX;
			pNewEdge->fY = vIterPrev->fY;
            pNewEdge->fX2 = vIter->fX;
			pNewEdge->fY2 = vIter->fY;
			pNewEdge->pNextEdge = pEdgeList;
			pEdgeList = pNewEdge;
			iNumEdges++;
			vIterPrev = vIter;
			vIter++;
        }
        pIter++;
    }
	pOptimizedCol = new CollisionNode( iNumEdges, 2, pEdgeList );

	// create pOptimizedOrigCol from sPolygonOrigData_list
	pIter = sPolygonOrigData_list.begin ( );
	iNumEdges = 0;
	pEdgeList = 0;
    while ( pIter < sPolygonOrigData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
		vIter = pIter->sVertexData_list.begin ( );
        vIterPrev = pIter->sVertexData_list.end ( ) - 1;
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            EdgeData *pNewEdge = new EdgeData( );
			pNewEdge->bHalfHeight = pIter->bHalfHeight;
			pNewEdge->fNormVX = vIterPrev->fNormVX;
			pNewEdge->fNormVY = vIterPrev->fNormVY;
			pNewEdge->fX = vIterPrev->fX;
			pNewEdge->fY = vIterPrev->fY;
            pNewEdge->fX2 = vIter->fX;
			pNewEdge->fY2 = vIter->fY;
			pNewEdge->pNextEdge = pEdgeList;
			pEdgeList = pNewEdge;
			iNumEdges++;
			vIterPrev = vIter;
			vIter++;
        }
        pIter++;
    }
	pOptimizedOrigCol = new CollisionNode( iNumEdges, 2, pEdgeList );

	// create pOptimizedColGrid from sPolygonData_list for new grid system (to handle LARGE levels without perf spike)
	/* takes AGES and breaks path finding - nice ;)
	for ( int iZ = 0; iZ < 128; iZ++ )
	{
		for ( int iX = 0; iX < 128; iX++ )
		{
			float fBoundX1 = iX*400.0f;
			float fBoundZ1 = iZ*400.0f;
			float fBoundX2 = fBoundX1 + 400.0f;
			float fBoundZ2 = fBoundZ1 + 400.0f;
			vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
			vector<sVertexData>::iterator vIter, vIterPrev;
			int iNumEdges = 0;
			EdgeData *pEdgeList = 0;
			while ( pIter < sPolygonData_list.end ( ) )
			{
				if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
				vIter = pIter->sVertexData_list.begin ( );
				vIterPrev = pIter->sVertexData_list.end ( ) - 1;
				while ( vIter < pIter->sVertexData_list.end ( ) )
				{
					// only add edge if center is within grid being worked on
					float fCenterX = vIter->fX + (( vIterPrev->fX-vIter->fX )/2.0f);
					float fCenterZ = vIter->fY + (( vIterPrev->fY-vIter->fY )/2.0f);
					if ( fCenterX >= fBoundX1 && fCenterX < fBoundX2 )
					{
						if ( fCenterZ >= fBoundZ1 && fCenterZ < fBoundZ2 )
						{
							EdgeData *pNewEdge = new EdgeData( );
							pNewEdge->bHalfHeight = pIter->bHalfHeight;
							pNewEdge->fNormVX = vIterPrev->fNormVX;
							pNewEdge->fNormVY = vIterPrev->fNormVY;
							pNewEdge->fX = vIterPrev->fX;
							pNewEdge->fY = vIterPrev->fY;
							pNewEdge->fX2 = vIter->fX;
							pNewEdge->fY2 = vIter->fY;
							pNewEdge->pNextEdge = pEdgeList;
							pEdgeList = pNewEdge;
							iNumEdges++;
							vIterPrev = vIter;
						}
					}
					vIter++;
				}
				pIter++;
			}

			// add these edges to a specific place within the grid
			if ( iNumEdges > 0 )
				pOptimizedColGrid[iX][iZ] = new CollisionNode( iNumEdges, 2, pEdgeList );
		}
	}
	*/
}

void PathFinderAdvanced::CompleteObstacles( )
{	
	// optimize obstacles
	OptimizeObstacles();

	// finally construct waypoints
	BuildWaypoints( );
}

float PathFinderAdvanced::CheckRay ( float fVx, float fVy, float fDiffVx, float fDiffVy,    //polygon vector
                                     float fPx, float fPy, float fDiffPx, float fDiffPy )   //path ray
{
    float fRel = ( fDiffPy * fDiffVx ) - ( fDiffPx * fDiffVy );
    
    //parallel
    if ( fRel == 0 ) return -1;
    fRel = 1.0f / fRel;
    
    //t  - distance along path ray to collision
    //t2 - distance along polygon vector to collision
    float t  = ( ( fPx-fVx )*fDiffVy - ( fPy-fVy )*fDiffVx ) * fRel;
    float t2 = ( ( fVy-fPy )*fDiffPx - ( fVx-fPx )*fDiffPy ) * fRel;
    
    if ( ( t < 0 || t > 1 ) || ( t2 < 0 || t2 > 1 ) ) return -1;
    
    return t;
}

float PathFinderAdvanced::CheckInfRay ( float fVx, float fVy, float fDiffVx, float fDiffVy,    //polygon vector
                                        float fPx, float fPy, float fDiffPx, float fDiffPy,	   //path ray
										int *iSide )   
{
    float fRel = ( fDiffPy * fDiffVx ) - ( fDiffPx * fDiffVy );
    
    //parallel
    if ( fRel == 0 ) return -1;
    float fInvRel = 1.0f / fRel;
    
	//t  - distance along path ray to collision
    //t2 - distance along polygon vector to collision
	float t  = ( ( fPx-fVx )*fDiffVy - ( fPy-fVy )*fDiffVx ) * fInvRel;
    float t2 = ( ( fVy-fPy )*fDiffPx - ( fVx-fPx )*fDiffPy ) * fInvRel;
    
    if ( t < 0 || t2 < 0 || t2 > 1 ) return -1;
    
	*iSide =  ( fRel > 0 ) ? -1 : 1;
    return t2;
}


float PathFinderAdvanced::CheckSidedRay ( float fVx, float fVy, float fDiffVx, float fDiffVy,    //polygon vector
                                          float fPx, float fPy, float fDiffPx, float fDiffPy,    //path ray
                                          int *iSide )
{
    float fRel = ( fDiffPy * fDiffVx ) - ( fDiffPx * fDiffVy );
    
    //parallel
    if ( fRel == 0 ) return -1;
    float fInvRel = 1.0f / fRel;
    
    //t  - distance along path ray to collision
    //t2 - distance along polygon vector to collision
    float t  = ( ( fPx-fVx )*fDiffVy - ( fPy-fVy )*fDiffVx ) * fInvRel;
    float t2 = ( ( fVy-fPy )*fDiffPx - ( fVx-fPx )*fDiffPy ) * fInvRel;
    
    if ( ( t < 0 || t > 1 ) || ( t2 < 0 || t2 > 1 ) ) return -1;
    
    *iSide =  ( fRel > 0 ) ? -1 : 1;
    return t;
}

//just returns true if the ray hits something, false if not
bool PathFinderAdvanced::QuickPolygonsCheck ( float fSX, float fSY, float fEX, float fEY, int iHeight )
{
    if ( pOptimizedCol )
	{
		return pOptimizedCol->Intersects( fSX, fSY, fEX, fEY, iHeight );
	}
	else
	{
		// no obstacles created for non-zero containers, so assume no collisions (allowing free movement of AI inside non-zero containers)
		return false;
	}
	/* broken!!
	int iGridSX = fSX/400.0f;
	int iGridSZ = fSY/400.0f;
	int iGridEX = fEX/400.0f;
	int iGridEZ = fEY/400.0f;
	if ( iGridSX < 0 ) iGridSX = 0;
	if ( iGridSZ < 0 ) iGridSZ = 0;
	if ( iGridEX < 0 ) iGridEX = 0;
	if ( iGridEZ < 0 ) iGridEZ = 0;
	if ( iGridSX > 127 ) iGridSX = 127;
	if ( iGridSZ > 127 ) iGridSZ = 127;
	if ( iGridEX > 127 ) iGridEX = 127;
	if ( iGridEZ > 127 ) iGridEZ = 127;
	for ( int iRefZ = iGridSZ; iRefZ <= iGridEZ; iRefZ++ )
	{
		for ( int iRefX = iGridSX; iRefX <= iGridEX; iRefX++ )
		{
			CollisionNode* pColNode = pOptimizedColGrid[iRefX][iRefZ];
			if ( pColNode )
			{
				if ( pColNode->Intersects( fSX, fSY, fEX, fEY, iHeight ) == true )
				{
					// hit, can exit early
					return true;
				}
			}
		}
	}
	return false;
	*/
	// 120417 - when have 13000 edges, this spikes performance!!
    //if ( pOptimizedCol )
	//{
	//	return pOptimizedCol->Intersects( fSX, fSY, fEX, fEY, iHeight );
	//}
	/*
	else
	{
		vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
		vector<sVertexData>::iterator vIter, vIterNext;
		float x, y, vx, vy;
		float fResult;
		int iSide = 0;
	    
		while ( pIter < sPolygonData_list.end ( ) )
		{
			if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
			if ( pIter->bHalfHeight && iHeight == 1 ) { pIter++; continue; }
			if ( !pIter->bHalfHeight && iHeight == 0 ) { pIter++; continue; }
			
			vIter = pIter->sVertexData_list.begin ( );
			vIterNext = vIter + 1;
	        
			x = vIter->fX;
			y = vIter->fY;
	        
			while ( vIter < pIter->sVertexData_list.end ( ) )
			{
				vx = vIterNext->fX - x;
				vy = vIterNext->fY - y;
	            
				fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
	            
				if ( fResult >= 0 && iSide > 0 ) return true;
	            
				x += vx;
				y += vy;
	            
				vIter++;
				if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
				else vIterNext = pIter->sVertexData_list.begin ( );
			}
	        
			pIter++;    
		}
	    
		return false;
	}
	*/
}

bool PathFinderAdvanced::QuickPolygonsCheckVisible ( float fSX, float fSY, float fEX, float fEY, int iHeight )
{
    if ( pOptimizedOrigCol )
	{
		if ( pOptimizedOrigCol->Intersects( fSX, fSY, fEX, fEY, iHeight ) ) return true;
	}

	vector<sPolygonData>::iterator pIter = sViewBlockingData_list.begin ( );
	vector<sVertexData>::iterator vIter, vIterNext;
	float x, y, vx, vy;
	float fResult;
	int iSide = 0;
    
	while ( pIter < sViewBlockingData_list.end ( ) )
	{
		if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
		if ( pIter->bHalfHeight && iHeight == 1 ) { pIter++; continue; }
		if ( !pIter->bHalfHeight && iHeight == 0 ) { pIter++; continue; }

		vIter = pIter->sVertexData_list.begin ( );
		vIterNext = vIter + 1;
        
		x = vIter->fX;
		y = vIter->fY;
        
		while ( vIter < pIter->sVertexData_list.end ( ) )
		{
			vx = vIterNext->fX - x;
			vy = vIterNext->fY - y;
            
			fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
            
			if ( fResult >= 0 && iSide > 0 ) return true;
            
			x += vx;
			y += vy;
            
			vIter++;
			if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
			else vIterNext = pIter->sVertexData_list.begin ( );
		}
        
		pIter++;    
	}

	return false;
	//}

	//return false;
	/*
	else
	{
		vector<sPolygonData>::iterator pIter = sPolygonOrigData_list.begin ( );
		vector<sVertexData>::iterator vIter, vIterNext;
		float x, y, vx, vy;
		float fResult;
		int iSide = 0;
	    
		while ( pIter < sPolygonOrigData_list.end ( ) )
		{
			if ( pIter->sVertexData_list.size( ) < 2 ) { pIter++; continue; }
			if ( pIter->bHalfHeight && iHeight == 1 ) { pIter++; continue; }
			if ( !pIter->bHalfHeight && iHeight == 0 ) { pIter++; continue; }

			vIter = pIter->sVertexData_list.begin ( );
			vIterNext = vIter + 1;
	        
			x = vIter->fX;
			y = vIter->fY;
	        
			while ( vIter < pIter->sVertexData_list.end ( ) )
			{
				vx = vIterNext->fX - x;
				vy = vIterNext->fY - y;
	            
				fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
	            
				if ( fResult >= 0 && iSide > 0 ) return true;
	            
				x += vx;
				y += vy;
	            
				vIter++;
				if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
				else vIterNext = pIter->sVertexData_list.begin ( );
			}
	        
			pIter++;    
		}
	    
		return false;
	 }
	 */
}

float PathFinderAdvanced::FindClosestPolygon ( float fSX, float fSY, float fEX, float fEY )
{
	vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter;
    float x, y, vx, vy;
    float fResult;
	float fClosestResult = -1;
	bool bFound = false;

    while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 ) 
		{
			pIter++;
			continue;
		}
		
		vIter = pIter->sVertexData_list.end( ) - 1;
		x = vIter->fX;
        y = vIter->fY;

		vIter = pIter->sVertexData_list.begin ( );
        
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            vx = vIter->fX - x;
            vy = vIter->fY - y;
			int iSide = 0;
            
            fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
            
            if ( fResult >= 0 && iSide > 0 ) 
			{
				if ( fResult < fClosestResult || !bFound )
				{
					fClosestResult = fResult;
					bFound = true;
				}
			}
            
            x += vx;
            y += vy;
            
            vIter++;
        }
        
        pIter++;    
    }

	return fClosestResult;
}

void PathFinderAdvanced::FindClosestOutsidePoint ( float *pX, float *pY )
{
	vector < sPolygonData >::iterator pIter = sPolygonData_list.begin ( );
	vector<sVertexData>::iterator vIter;
    float fX, fY, fVnX, fVnY, vx, vy;

	while ( pIter < sPolygonData_list.end ( ) )
	{
		if ( pIter->sVertexData_list.size( ) < 2 )
		{
			pIter++;
			continue;
		}
		
		vIter = pIter->sVertexData_list.end ( ) - 1;
		fX = vIter->fX;
		fY = vIter->fY;
		fVnX = vIter->fNormVX;
		fVnY = vIter->fNormVY;
		vIter = pIter->sVertexData_list.begin ( );

		int iCountSide = 0;
		float fClosestDist = -1.0f;
		float fClosestX = 0.0f;
		float fClosestY = 0.0f;
		float fClosestIntersect = -1.0f;
		int iClosestSide = 0;

		float fEX = ( vIter->fX + fX ) / 2.0f;
		float fEY = ( vIter->fY + fY ) / 2.0f;
		//fEX = fEX + ( fEX - *pX )*0.1f;
		//fEY = fEY + ( fEY - *pY )*0.1f;

		float fDiffX = fEX - *pX;
		float fDiffY = fEY - *pY;
		float dist = fDiffX*fDiffX + fDiffY*fDiffY;
		fEX = 100000 * fEX / dist;
		fEY = 100000 * fEY / dist;

		//CHECK THIS CODE

		while ( vIter < pIter->sVertexData_list.end ( ) )
		{
			vx = vIter->fX - fX;
			vy = vIter->fY - fY;

			int iSide = 0;
			float fResult = CheckSidedRay ( fX, fY, vx, vy, *pX, *pY, fEX - *pX, fEY - *pY, &iSide );

			if ( fResult >= 0.0f )
			{
				if ( fClosestIntersect < 0 || fResult < fClosestIntersect ) 
				{
					fClosestIntersect = fResult;
					iClosestSide = iSide;
				}
			}

			float fLength = vx*vx + vy*vy;
			float fDotP = ( ( (*pX) - fX ) * vx + ( (*pY) - fY ) * vy );
			
			if ( fLength > 0.001f ) fDotP /= fLength;
			else fDotP = 0.0;

			if ( fDotP < 0.0f ) fDotP = 0.0f;
			if ( fDotP > 1.0f ) fDotP = 1.0f;

			float fDiffX = ( fX + vx*fDotP ) - (*pX);
			float fDiffY = ( fY + vy*fDotP ) - (*pY);

			float fDist = fDiffX*fDiffX + fDiffY*fDiffY;
			if ( fDist < fClosestDist || fClosestDist < 0.0f )
			{
				fClosestDist = fDist;
				fClosestX = fX + vx*fDotP - fVnY*fCurrRadius*0.1f;
				fClosestY = fY + vy*fDotP + fVnX*fCurrRadius*0.1f;
			}

			fX = vIter->fX;
			fY = vIter->fY;
			fVnX = vIter->fNormVX;
			fVnY = vIter->fNormVY;
			vIter++;
		}

		if ( iClosestSide < 0 )
		{
			*pX = fClosestX;
			*pY = fClosestY;
		}

		pIter++;
	}
}

//returns the number of times the ray crosses an edge
int PathFinderAdvanced::CountCrossings ( float fSX, float fSY, float fEX, float fEY )
{
    vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter, vIterNext;
    float x, y, vx, vy;
    float fResult;
    
    int iCount = 0;
    
    while ( pIter < sPolygonData_list.end ( ) )
    {
        vIter = pIter->sVertexData_list.begin ( );
        vIterNext = vIter + 1;
        
        x = vIter->fX;
        y = vIter->fY;
        
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            vx = vIterNext->fX - x;
            vy = vIterNext->fY - y;
            
            fResult = CheckRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY );
            
            if ( fResult >= 0 ) iCount++;
            
            x += vx;
            y += vy;
            
            vIter++;
            if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
            else vIterNext = pIter->sVertexData_list.begin ( );
        }
        
        pIter++;    
    }
    
    return iCount;
}

//returns non zero if start and end points are on different sides (inside, outside)
int PathFinderAdvanced::CountSidedness ( float fSX, float fSY, float fEX, float fEY )
{
    vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter, vIterNext;
    float x, y, vx, vy;
    float fResult;
    
    int iCount = 0;
    
    while ( pIter < sPolygonData_list.end ( ) )
    {
        vIter = pIter->sVertexData_list.begin ( );
        vIterNext = vIter + 1;
        
        x = vIter->fX;
        y = vIter->fY;
        
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            vx = vIterNext->fX - x;
            vy = vIterNext->fY - y;
            
			int iSide = 0;
            fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
            
            if ( fResult >= 0 ) iCount += iSide;
            
            x += vx;
            y += vy;
            
            vIter++;
            if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
            else vIterNext = pIter->sVertexData_list.begin ( );
        }
        
        pIter++;    
    }
    
    return iCount;
}

//counts the number of polygons the point is in
int PathFinderAdvanced::InPolygons ( float fSX, float fSY )
{
    vector<sPolygonData>::iterator pIter = sPolygonData_list.begin ( );
    vector<sVertexData>::iterator vIter, vIterNext;
    float x, y, vx, vy;
    float fResult;
    
    int iCount = 0;
    
    while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->sVertexData_list.size( ) < 2 )
		{
			pIter++;
			continue;
		}

		vIter = pIter->sVertexData_list.begin ( );
        vIterNext = vIter + 1;
        
        x = vIter->fX;
        y = vIter->fY;

		float fEX = ( vIter->fX + vIterNext->fX ) / 2;
		float fEY = ( vIter->fY + vIterNext->fY ) / 2;

		float dist = sqrt((fEX - fSX)*(fEX - fSX) + (fEY - fSY)*(fEY - fSY));
		fEX = 100000 * (fEX - fSX) / dist;
		fEY = 100000 * (fEY - fSY) / dist;

		//fEX = fEX + ( fEX - fSX )*0.1f;
		//fEY = fEY + ( fEY - fSY )*0.1f;

		int iClosestSide = 0;
		float fClosestIntersect = -1.0f;
        
        while ( vIter < pIter->sVertexData_list.end ( ) )
        {
            vx = vIterNext->fX - x;
            vy = vIterNext->fY - y;
            
			int iSide = 0;
            fResult = CheckSidedRay ( x, y, vx, vy, fSX, fSY, fEX - fSX, fEY - fSY, &iSide );
            
            if ( fResult >= 0 )
			{
				if ( fClosestIntersect < 0 || fResult < fClosestIntersect )
				{
					fClosestIntersect = fResult;
					iClosestSide = iSide;
				}
			}
            
            x += vx;
            y += vy;
            
            vIter++;
            if ( vIter < pIter->sVertexData_list.end ( ) - 1 ) vIterNext++;
            else vIterNext = pIter->sVertexData_list.begin ( );
        }

		if ( fClosestIntersect >= 0 && iClosestSide < 0 )
		{
			iCount++;
		}
        
        pIter++;
    }
    
    return iCount;
}

void PathFinderAdvanced::ActivateAllWaypoints ( )
{
	sWaypoint *pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		pWaypoint->bActive = true;
		pWaypoint = pWaypoint->pNextWaypoint;
	}
}

void PathFinderAdvanced::DeActivateWaypoint ( sWaypoint *pWaypoint )
{
	if ( !pWaypoint ) return;
	
	pWaypoint->bActive = false;
}

//gauranteed to find a path if one exists
bool PathFinderAdvanced::ShortestPath ( float fEX, float fEY, Path* pBuildPath, int iDestinationContainer )
{
    if ( !pBuildPath ) return false;
    
    bool bFound = false;
    
    sWaypoint *pWaypoint, *pNextWP;
    vector<sWaypointInfo>::iterator wInfoIter;
    //vector<sWaypointEdge>::iterator wEdgeIter;
	sWaypointEdge *pEdge = 0;
    sWaypointInfo* pNewInfo;
    
//    char str [ 256 ];
    int iIterations = 0;
    
    while ( !bFound && sWaypointOpen_list.size ( ) > 0 )
    {        
        iIterations++;
        
        //get the shortest path so far
        wInfoIter = sWaypointOpen_list.begin ( );
        
        pWaypoint = wInfoIter->pThisWP;
		
		if ( !pWaypoint )
		{
			pop_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );
			sWaypointOpen_list.pop_back ( );
			continue;
		}

		//get the edge list for this waypoint
        //wEdgeIter = wIter->sEdge_list.begin ( );
		pEdge = pWaypoint->pEdgeList;
        
		if ( pWaypoint->fDistH < 1 && pWaypoint->pContainer->GetID( ) == iDestinationContainer )
        {
            bFound = true;
            break;
            //endpoint sits on top of the open list, use it to jump back to work out the path
        }
        
        pWaypoint->iVisited = 2;
		int iCount = 0;

		if ( pWaypoint->iNumEdges > 0 )
		{
        
			//build the data for these new edges ready to add to the open list
			pNewInfo = new sWaypointInfo [ pWaypoint->iNumEdges ];
	        
			while ( pEdge )
			{
				pNextWP = pEdge->pOtherWP;

				if ( !pNextWP )
				{
					pEdge = pEdge->pNextEdge;
					continue;
				}

				if ( pEdge->pDoors )
				{
					// edge is blocked by a door, skip it
					pEdge = pEdge->pNextEdge;
					continue;
				}
	            
				//if ( wNextIter->bActive )
				{
					if ( pNextWP->iVisited == 0 )
					{
						pNextWP->pParent = pWaypoint;
						pNextWP->fDistG = pWaypoint->fDistG + pEdge->fCost + pNextWP->fWPCost;
						pNextWP->fDistH = EstimateDistance ( pNextWP->fX, pNextWP->fY, fEX, fEY );
						pNextWP->fDistF = pNextWP->fDistG + pNextWP->fDistH;
						pNextWP->iVisited = 1;
		                
						if ( iCount < pWaypoint->iNumEdges )
						{
							pNewInfo [ iCount ].pThisWP = pNextWP;
							pNewInfo [ iCount ].fCost = pNextWP->fDistF;
							iCount++;
						}
						else
						{
							// this should not happen - investigate (MP finestgladelevel)
						}
					}
					else if ( pNextWP->iVisited == 1 )
					{
						if ( pWaypoint->fDistG + pEdge->fCost + pNextWP->fWPCost < pNextWP->fDistG )
						{
							pNextWP->pParent = pWaypoint;
							pNextWP->fDistG = pWaypoint->fDistG + pEdge->fCost + pNextWP->fWPCost;
							pNextWP->fDistH = EstimateDistance ( pNextWP->fX, pNextWP->fY, fEX, fEY );
							pNextWP->fDistF = pNextWP->fDistG + pNextWP->fDistH;
		                    
							if ( iCount < pWaypoint->iNumEdges )
							{
								pNewInfo [ iCount ].pThisWP = pNextWP;
								pNewInfo [ iCount ].fCost = pNextWP->fDistF;
								iCount++;
							}
							else
							{
								// this should not happen - investigate (MP finestgladelevel)
							}
						}
					}
				}
	            
				pEdge = pEdge->pNextEdge;
			}

			pop_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );
			sWaypointOpen_list.pop_back ( );
	        
			for (int i = 0; i < iCount; i++ )
			{
				sWaypointOpen_list.push_back ( pNewInfo [ i ] );
				push_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );
			}
	        
			delete [] pNewInfo;
		}
		else
		{
			pop_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );
			sWaypointOpen_list.pop_back ( );
		}
    }
    
//    sprintf ( str, "Iterations: %d", iIterations );
//    TextOut( debugOutput, 5, 100, str, strlen ( str ) );
    
    if ( bFound )
    {
        wInfoIter = sWaypointOpen_list.begin ( );
        pWaypoint = wInfoIter->pThisWP;
		pBuildPath->InsertPoint ( 0, pWaypoint->fX, pWaypoint->fY, pWaypoint->pContainer->GetID( ) );
        
        do
        {
            pWaypoint = pWaypoint->pParent;
            pBuildPath->InsertPoint ( 0, pWaypoint->fX, pWaypoint->fY, pWaypoint->pContainer->GetID( ) );
            
        } while ( pWaypoint != pWaypoint->pParent );
        
        return true;
    }
    
    return false;
}

//asumes polygons all have a clockwise winding order
void PathFinderAdvanced::CalculatePath ( float fSX, float fSY, float fEX, float fEY,    //points to check between
                                         Path* pFinalPath, 								//place to store the final path
										 float fMaxEdgeCost, 							//max distance without crossing a waypoint
										 int iDestinationContainer )
{	
	if ( !pFinalPath ) return;
    
    //nothing hit
	/*if ( iDestinationContainer == pOwner->GetID( ) && !QuickPolygonsCheck ( fSX, fSY, fEX, fEY, 2 ) && !BlockedByDoor(fSX, fSY, fEX, fEY) )
    {
        pFinalPath->AddPoint( fSX, 0, fSY, iDestinationContainer );
        pFinalPath->AddPoint( fEX, 0, fEY, iDestinationContainer );
        return;
    }*/

	PathFinderAdvanced *pDestPathFinder = this;

	if ( iDestinationContainer >= 0 ) 
	{
		Container *pDestContainer = cWorld.GetContainer( iDestinationContainer );
		if ( !pDestContainer ) return;
		pDestPathFinder = pDestContainer->pPathFinder;
	}

	//iResult = CountSidedness ( fSX, fSY, fEX, fEY );
    //points on different sides (inside, outside), impossible to solve with enclosed polygons
    //if ( iResult != 0 )
    //{
    //    return;
    //}
	
	//int iStartPointSide = CountInfSidedness ( fSX, fSY );
	//int iEndPointSide = CountInfSidedness ( fEX, fEY );
    
	
	//if ( iStartPointSide != 0 )
	{
		//vector < sWaypoint >::iterator wIter = GetClosestWaypointIter ( fSX, fSY );
		//fSX = wIter->fX;
		//fSY = wIter->fY;
		FindClosestOutsidePoint ( &fSX, &fSY );
	}

	//if ( iEndPointSide != 0 )
	{
		//vector < sWaypoint >::iterator wIter = GetClosestWaypointIter ( fEX, fEY );
		//fEX = wIter->fX;
		//fEY = wIter->fY;
		pDestPathFinder->FindClosestOutsidePoint ( &fEX, &fEY );
	}

	if ( iDestinationContainer == pOwner->GetID( ) && !QuickPolygonsCheck ( fSX, fSY, fEX, fEY, 2 ) && !BlockedByDoor(fSX, fSY, fEX, fEY) )
    {
		if ( fMaxEdgeCost < 0 || (fSX-fEX)*(fSX-fEX) + (fSY-fEY)*(fSY-fEY) < fMaxEdgeCost*fMaxEdgeCost )
		{
			pFinalPath->AddPoint( fSX, 0, fSY );
			pFinalPath->AddPoint( fEX, 0, fEY );
			return;
		}
    }

	if ( !pWaypointList || !pDestPathFinder->pWaypointList ) return;

    sWaypoint *pStartPoint = new sWaypoint();
	sWaypoint *pEndPoint = new sWaypoint();
    
    pStartPoint->fX = fSX;
    pStartPoint->fY = fSY;
    pStartPoint->fDistG = 0;
    pStartPoint->fDistH = EstimateDistance ( fSX, fSY, fEX, fEY );
    pStartPoint->fDistF = pStartPoint->fDistG + pStartPoint->fDistH;
    pStartPoint->iVisited = 1;
	pStartPoint->pEdgeList = 0;
	pStartPoint->iNumEdges = 0;
	pStartPoint->pParent = pStartPoint;
	pStartPoint->pContainer = pOwner;

    pEndPoint->fX = fEX;
    pEndPoint->fY = fEY;
    pEndPoint->fDistG = 0;
    pEndPoint->fDistH = 0;
    pEndPoint->fDistF = 0;
    pEndPoint->iVisited = 0;
	pEndPoint->pEdgeList = 0;
	pEndPoint->iNumEdges = 0;
	pEndPoint->pParent = 0;
	pEndPoint->pContainer = pDestPathFinder->pOwner;

	int result = WaitForSingleObject( hPathFindingMutex, INFINITE );
	if ( result == WAIT_FAILED ) 
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock PF mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	cWorld.SetupAllPathFinders( );
    
	//sWaypoint *pWaypoint = pWaypointList;

	UpdateSingleVisibility( pStartPoint, fMaxEdgeCost, false );
	pDestPathFinder->UpdateSingleVisibility( pEndPoint, fMaxEdgeCost );

	AddWaypoint ( pStartPoint );
	pDestPathFinder->AddWaypoint ( pEndPoint );
    
	//start the open list with the start point
    sWaypointOpen_list.clear ( );
    sWaypointInfo sStartPointInfo;
    
    sStartPointInfo.pThisWP = pStartPoint;
    sStartPointInfo.fCost = pStartPoint->fDistF;
    
    sWaypointOpen_list.push_back ( sStartPointInfo );
    
    make_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );

    ShortestPath ( fEX, fEY, pFinalPath, iDestinationContainer );

	sWaypointOpen_list.clear ( );
    
    //remove the start and endpoints from the waypoints, and all edges in between
	pDestPathFinder->RemoveEndWaypoint( pEndPoint );
    
    RemoveLastWaypoint ( );
	pDestPathFinder->RemoveLastWaypoint ( );

	ReleaseMutex( hPathFindingMutex );
}

void PathFinderAdvanced::RemoveEndWaypoint( sWaypoint *pWaypoint )
{
	sWaypointEdge *pEdge = pWaypoint->pEdgeList;
    
    while ( pEdge )
    {
        //last edge added to the other waypoint would have been to the end point
		pEdge->pOtherWP->RemoveLastEdge( );
        
		pEdge = pEdge->pNextEdge;
    }
}

void PathFinderAdvanced::ClearDistanceData( )
{
	sWaypoint *pWaypoint = pWaypointList;
    
	while ( pWaypoint )
    {
		pWaypoint->iVisited = 0;
        pWaypoint->fDistG = 0;
        pWaypoint->fDistH = 0;
        pWaypoint->fDistF = 0;
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void PathFinderAdvanced::SearchCoverPoints ( float fSX, float fSY, float fTX, float fTY, Path *pPoints )
{
	sCoverPoint *pCoverPoint = pCoverPointList;
	
	//find all cover points visible from this point suitable for hiding behind
	
    while ( pCoverPoint )
    {
		if ( !pCoverPoint->IsInUse() )
		{
			float dirX = fSX - fTX; 
			float dirY = fSY - fTY;
			float dir2X = pCoverPoint->fX - fTX; 
			float dir2Y = pCoverPoint->fY - fTY;

			// is the cover in front of the target
			float dotp = dirX*dir2X + dirY*dir2Y;
			if ( dotp >= 0 )
			{
				// can the entity get to this point
				if ( !QuickPolygonsCheckVisible ( fSX, fSY, pCoverPoint->fX, pCoverPoint->fY, 1 ) )
				{
					float distX = pCoverPoint->fX - fTX;
					float distY = pCoverPoint->fY - fTY;
					float dist = distX*distX + distY*distY;

					// can the entity fire at the target from this point
					if ( dist > fCurrRadius*fCurrRadius*16 && !QuickPolygonsCheckVisible ( pCoverPoint->fX, pCoverPoint->fY, fTX, fTY, 1 ) )
					{
						// is the cover orientated correctly
						float fVX = fTX - pCoverPoint->fX;
						float fVY = fTY - pCoverPoint->fY;
						float length = sqrt(fVX*fVX + fVY*fVY);

						float dotp = (fVX*pCoverPoint->fDirX + fVY*pCoverPoint->fDirY)/length;

						if ( dotp > 0.5f ) 
						{
							// everything checks out
							pPoints->AddPoint ( pCoverPoint->fX, 0, pCoverPoint->fY, pCoverPoint->iID );
						}
					}
				}
			}
		}

		pCoverPoint = pCoverPoint->pNextPoint;
    }
}

void PathFinderAdvanced::SearchPeekingPoints ( float fSX, float fSY, float fTX, float fTY, Path *pPoints, Path *pDirections, Path *pPeekPoints )
{
	sWaypoint *pWaypoint = pWaypointList;
	
	//find all waypoints visible from this point suitable for hiding behind a corner
	
    while ( pWaypoint )
    {
		if ( pWaypoint->bCanPeek )
		{
			float dirX = fSX - fTX; 
			float dirY = fSY - fTY;
			float dir2X = pWaypoint->fX - fTX; 
			float dir2Y = pWaypoint->fY - fTY;

			// is the point in front of the target
			float dotp = dirX*dir2X + dirY*dir2Y;
			if ( dotp >= 0 )
			{
				// can the entity get to this point
				if ( !QuickPolygonsCheckVisible ( fSX, fSY, pWaypoint->fX, pWaypoint->fY, 2 ) )
				{
					float distX = pWaypoint->fX - fTX;
					float distY = pWaypoint->fY - fTY;
					float dist = distX*distX + distY*distY;

					// can the entity fire at the target from this point
					if ( dist > fCurrRadius*fCurrRadius*36 && !QuickPolygonsCheckVisible ( pWaypoint->fX, pWaypoint->fY, fTX, fTY, 1 ) )
					{
						// is the corner orientated correctly
						float fVX = fTX - pWaypoint->fX;
						float fVY = fTY - pWaypoint->fY;

						float dotp = fVX*pWaypoint->fVX + fVY*pWaypoint->fVY;

						if ( dotp >= 0 ) 
						{
							// everything checks out so far, find the correct side of the corner
							float dirX = pWaypoint->fVY;
							float dirY = -pWaypoint->fVX;

							float ang = acos( pWaypoint->fCAngle );
							float sign = 1;
							if ( fVX*dirX + fVY*dirY > 0 )
							{
								ang = -ang;
								sign = -1;
							}

							// check wall is big enough to hide behind
							if ( (sign > 0 && (pWaypoint->bFlags & DARKAI_WAYPOINT_PEEK_RIGHT))
							  || (sign < 0 && (pWaypoint->bFlags & DARKAI_WAYPOINT_PEEK_LEFT)) )
							{
								float x = pWaypoint->fVX*cos(ang) + pWaypoint->fVY*sin(ang);
								float y = pWaypoint->fVY*cos(ang) - pWaypoint->fVX*sin(ang);

								float dx = y*sign;
								float dy = (-x)*sign;

								x = pWaypoint->fX + x * fCurrRadius*1.5f;
								y = pWaypoint->fY + y * fCurrRadius*1.5f;

								pPoints->AddPoint ( x, 0, y );
								pDirections->AddPoint( dx, 0, dy );
								pPeekPoints->AddPoint( pWaypoint->fX, 0, pWaypoint->fY );
							}
						}
					}
				}
			}
		}

		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void PathFinderAdvanced::SearchPoints ( float fSX, float fSY, Path *pPoints, int iHeight )
{
	sWaypoint *pWaypoint = pWaypointList;
	
	//find all waypoints visible from this point
	
    while ( pWaypoint )
    {
        if ( !QuickPolygonsCheckVisible ( fSX, fSY, pWaypoint->fX, pWaypoint->fY, iHeight ) )
		{
			pPoints->AddPoint ( pWaypoint->fX, 0, pWaypoint->fY, pWaypoint->pContainer->GetID() );
			if ( pWaypoint->bFlags & DARKAI_WAYPOINT_BRIDGE )
			{
				// add waypoint from other container and see what happens
				sWaypointEdge *pEdge = pWaypoint->pEdgeList;
				if ( pEdge ) pPoints->AddPoint ( pEdge->pOtherWP->fX, 0, pEdge->pOtherWP->fY, pEdge->pOtherWP->pContainer->GetID() );
			}
		}

		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void PathFinderAdvanced::DebugDrawCoverPoints ( float fHeight )
{
}

void PathFinderAdvanced::DebugHideCoverPoints ( )
{      
}

void PathFinderAdvanced::DebugDrawWaypoints ( float fHeight )
{
    sWaypoint *pWaypoint = pWaypointList;
    
	int iTempMesh = dbMakePointMesh ( );
	if ( iTempMesh == 0 ) return;

	if ( iWaypointObject > 0 && ObjectExist ( iWaypointObject ) )	DeleteObject ( iWaypointObject );
	if ( iWaypointObject == 0 ) iWaypointObject = dbFreeObject ( );
	if ( iWaypointObject == 0 ) return;
	
	MakeObjectPlane ( iWaypointObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iWaypointObject, 1 );
	
	int iLimb = 1;
	while ( pWaypoint )
    {
		AddLimb ( iWaypointObject, iLimb, iTempMesh );
		ScaleLimb ( iWaypointObject, iLimb, 40*fCurrRadius, 40*fCurrRadius, 40*fCurrRadius );
		OffsetLimb ( iWaypointObject, iLimb, pWaypoint->fX, 0.0f, pWaypoint->fY );
		iLimb++;
        pWaypoint = pWaypoint->pNextWaypoint;
    }
	
	DeleteMesh ( iTempMesh );
	dbCombineLimbs ( iWaypointObject );
	PositionObject ( iWaypointObject, 0.0f, fHeight, 0.0f );
	ColorObject ( iWaypointObject, 0xff0000ff );
	SetObjectEmissive ( iWaypointObject, 0xff0000ff );
	SetObjectEffect ( iWaypointObject, g_GUIShaderEffectID );
	SetObjectCollisionOff ( iWaypointObject );
}

void PathFinderAdvanced::DebugHideWaypoints ( )
{      
    if ( iWaypointObject > 0 && ObjectExist ( iWaypointObject ) )
	{
		DeleteObject ( iWaypointObject );
	}
	iWaypointObject = 0;
}

void PathFinderAdvanced::DebugDrawWaypointEdges ( float fHeight )
{  
	sWaypoint *pWaypoint = pWaypointList;
	sWaypointEdge *pEdge;

	int iTempMesh = dbMakeEdgeMesh ( );
	if ( iTempMesh == 0 ) return;

	if ( iWaypointEdgeObject > 0 && ObjectExist ( iWaypointEdgeObject ) )
	{
		DeleteObject ( iWaypointEdgeObject );
	}

	if ( iWaypointEdgeObject == 0 ) iWaypointEdgeObject = dbFreeObject ( );
	if ( iWaypointEdgeObject == 0 ) return;
	
	MakeObjectPlane ( iWaypointEdgeObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iWaypointEdgeObject, 1 );
	
	// work objects to work out orientations
	int iTempObj1 = dbFreeObject ( );
	if ( iTempObj1 == 0 ) return;
	MakeObjectCube ( iTempObj1, 25.0f );

	int iLimb = 1;
    while ( pWaypoint )
    {
        //wEdgeIter = wIter->sEdge_list.begin ( );
		pEdge = pWaypoint->pEdgeList;
        
        while ( pEdge )
        {
			float fX = ( pEdge->pOtherWP->fX + pWaypoint->fX ) / 2.0f;
			float fZ = ( pEdge->pOtherWP->fY + pWaypoint->fY ) / 2.0f;
			float fDirX = pEdge->pOtherWP->fX - pWaypoint->fX;
			float fDirZ = pEdge->pOtherWP->fY - pWaypoint->fY;
			float fDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );
			if ( fDist < 1.0f ) 
			{
				pEdge = pEdge->pNextEdge;
				continue;
			}
			fDirX /= fDist;
			fDirZ /= fDist;
			float fAngY = acos ( fDirZ ) * 57.295779513f;
			if ( fDirX < 0.0f ) fAngY = 360.0f - fAngY;

			//AddLimb ( iWaypointEdgeObject, iLimb, iTempMesh );
			//ScaleLimb ( iWaypointEdgeObject, iLimb, 40*fCurrRadius, 100.0f, fDist * 100.0f );
			//RotateLimb ( iWaypointEdgeObject, iLimb, 0.0f, fAngY, 0.0f );
			//OffsetLimb ( iWaypointEdgeObject, iLimb, fX, 0.0f, fZ );
			AddLimb ( iWaypointEdgeObject, iLimb, iTempMesh );
			float fRealStartY = GetLUATerrainHeightEx ( pWaypoint->fX, pWaypoint->fY );
			PositionObject ( iTempObj1, pWaypoint->fX, fRealStartY, pWaypoint->fY );
			float fRealFinishY = GetLUATerrainHeightEx ( pEdge->pOtherWP->fX, pEdge->pOtherWP->fY );
			fDirX = pEdge->pOtherWP->fX - pWaypoint->fX;
			float fDirY = fRealFinishY - fRealStartY;
			fDirZ = pEdge->pOtherWP->fY - pWaypoint->fY;
			fDist = sqrt ( fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ );
			ScaleLimb ( iWaypointEdgeObject, iLimb, 40*fCurrRadius, 100.0f, fDist * 100.0f );
			PointObject ( iTempObj1, pEdge->pOtherWP->fX, fRealFinishY, pEdge->pOtherWP->fY );
			float fRealMiddleY = fRealStartY+((fRealFinishY-fRealStartY)/2.0f);
			OffsetLimb ( iWaypointEdgeObject, iLimb, fX, fRealMiddleY + 7.5f, fZ );
			RotateLimb ( iWaypointEdgeObject, iLimb, ObjectAngleX(iTempObj1), ObjectAngleY(iTempObj1), ObjectAngleZ(iTempObj1) );

			iLimb++;
			pEdge = pEdge->pNextEdge;
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }

	DeleteMesh ( iTempMesh );

	// delete temp object
	if ( ObjectExist(iTempObj1) == 1 ) DeleteObject ( iTempObj1 );

	dbCombineLimbs ( iWaypointEdgeObject );

	PositionObject ( iWaypointEdgeObject, 0.0f, fHeight, 0.0f );
	ColorObject ( iWaypointEdgeObject, 0xff0000ff );
	SetObjectEmissive ( iWaypointEdgeObject, 0xff0000ff );
	SetObjectEffect ( iWaypointEdgeObject, g_GUIShaderEffectID );

	SetObjectCollisionOff ( iWaypointEdgeObject );
}

void PathFinderAdvanced::DebugHideWaypointEdges ( )
{  
	if ( iWaypointEdgeObject > 0 && ObjectExist ( iWaypointEdgeObject ) )
	{
		DeleteObject ( iWaypointEdgeObject );
	}

	iWaypointEdgeObject = 0;
}

void PathFinderAdvanced::DebugDrawPolygonBounds ( float fHeight )
{
	vector < sPolygonData >::iterator pIter = sPolygonData_list.begin ( );
	vector < sVertexData >::iterator vIter, vIterPrev;
	
	//make a plane mesh to represent edges.
	int iTempMesh = dbMakeEdgeMesh ( );
	if ( iTempMesh == 0 ) return;
	
	//clear any current debug object
	if ( iPolygonBoundsObject > 0 && ObjectExist ( iPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iPolygonBoundsObject );
	}
	
	//find a free object
	if ( iPolygonBoundsObject == 0 ) iPolygonBoundsObject = dbFreeObject ( );
	if ( iPolygonBoundsObject == 0 ) return;

	//root node
	MakeObjectPlane ( iPolygonBoundsObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iPolygonBoundsObject, 1 );	

	//do the same for the half height debug object
	if ( iHHPolygonBoundsObject > 0 && ObjectExist ( iHHPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iHHPolygonBoundsObject );
	}

	if ( iHHPolygonBoundsObject == 0 ) iHHPolygonBoundsObject = dbFreeObject ( );
	if ( iHHPolygonBoundsObject == 0 ) return;

	//root node
	MakeObjectPlane ( iHHPolygonBoundsObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iHHPolygonBoundsObject, 1 );	
	
	int iLimb = 1;
	int iLimb2 = 1;

	// work objects to work out orientations
	int iTempObj1 = dbFreeObject ( );
	if ( iTempObj1 == 0 ) return;
	MakeObjectCube ( iTempObj1, 25.0f );

	//for all obstacles (polygons)
	while ( pIter < sPolygonData_list.end ( ) )
    {
		if ( pIter->bBlocksPath && pIter->sVertexData_list.size ( ) > 1 )
		{
			vIter = pIter->sVertexData_list.begin ( );
			vIterPrev = pIter->sVertexData_list.end ( ) - 1;
			
			//for all vertices (edges)
			while ( vIter < pIter->sVertexData_list.end ( ) )
			{
				// get average position and direction of this edge
				float fX = ( vIter->fX + vIterPrev->fX ) / 2.0f;
				float fZ = ( vIter->fY + vIterPrev->fY ) / 2.0f;
				float fDirX = vIter->fX - vIterPrev->fX;
				float fDirZ = vIter->fY - vIterPrev->fY;
				float fDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );
				if ( fDist < 0.1f ) 
				{
					vIterPrev = vIter;
					vIter++;
					continue;
				}
				fDirX /= fDist;
				fDirZ /= fDist;
				float fAngY = acos ( fDirZ ) * 57.295779513f;
				if ( fDirX < 0.0f ) fAngY = 360.0f - fAngY;

				// add to the relevant object, half height or full height
				if ( pIter->bHalfHeight )
				{
					AddLimb ( iHHPolygonBoundsObject, iLimb2, iTempMesh );
					float fRealStartY = GetLUATerrainHeightEx ( vIterPrev->fX, vIterPrev->fY );
					PositionObject ( iTempObj1, vIterPrev->fX, fRealStartY, vIterPrev->fY );
					float fRealFinishY = GetLUATerrainHeightEx ( vIter->fX, vIter->fY );
					fDirX = vIter->fX - vIterPrev->fX;
					float fDirY = fRealFinishY - fRealStartY;
					fDirZ = vIter->fY - vIterPrev->fY;
					fDist = sqrt ( fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ );
					ScaleLimb ( iHHPolygonBoundsObject, iLimb2, 40*fCurrRadius, 100.0f, fDist * 100.0f );
					PointObject ( iTempObj1, vIter->fX, fRealFinishY, vIter->fY );
					float fRealMiddleY = fRealStartY+((fRealFinishY-fRealStartY)/2.0f);
					OffsetLimb ( iHHPolygonBoundsObject, iLimb2, fX, fRealMiddleY + 7.5f, fZ );
					RotateLimb ( iHHPolygonBoundsObject, iLimb2, ObjectAngleX(iTempObj1), ObjectAngleY(iTempObj1), ObjectAngleZ(iTempObj1) );
					iLimb2++;
				}
				else
				{
					AddLimb ( iPolygonBoundsObject, iLimb, iTempMesh );
					float fRealStartY = GetLUATerrainHeightEx ( vIterPrev->fX, vIterPrev->fY );
					PositionObject ( iTempObj1, vIterPrev->fX, fRealStartY, vIterPrev->fY );
					float fRealFinishY = GetLUATerrainHeightEx ( vIter->fX, vIter->fY );
					fDirX = vIter->fX - vIterPrev->fX;
					float fDirY = fRealFinishY - fRealStartY;
					fDirZ = vIter->fY - vIterPrev->fY;
					fDist = sqrt ( fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ );
					ScaleLimb ( iPolygonBoundsObject, iLimb, 40*fCurrRadius, 100.0f, fDist * 100.0f );
					PointObject ( iTempObj1, vIter->fX, fRealFinishY, vIter->fY );
					float fRealMiddleY = fRealStartY+((fRealFinishY-fRealStartY)/2.0f);
					OffsetLimb ( iPolygonBoundsObject, iLimb, fX, fRealMiddleY + 7.5f, fZ );
					RotateLimb ( iPolygonBoundsObject, iLimb, ObjectAngleX(iTempObj1), ObjectAngleY(iTempObj1), ObjectAngleZ(iTempObj1) );
					iLimb++;
				}

				// next edge
				vIterPrev = vIter;
				vIter++;
			}
		}
        
        pIter++;
    }

	//now do the same for view blocking obstacles
	if ( iVBPolygonBoundsObject > 0 && ObjectExist ( iVBPolygonBoundsObject ) == 1 )
	{
		DeleteObject ( iVBPolygonBoundsObject );
	}

	if ( iVBPolygonBoundsObject == 0 ) iVBPolygonBoundsObject = dbFreeObject ( );
	if ( iVBPolygonBoundsObject == 0 ) return;

	//root node
	MakeObjectPlane ( iVBPolygonBoundsObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iVBPolygonBoundsObject, 1 );	

	pIter = sViewBlockingData_list.begin ( );
	
	iLimb = 1;
	while ( pIter < sViewBlockingData_list.end ( ) )
	{
		if ( pIter->sVertexData_list.size ( ) > 1 )
		{
			vIter = pIter->sVertexData_list.begin ( );
			vIterPrev = pIter->sVertexData_list.end ( ) - 1;

			while ( vIter < pIter->sVertexData_list.end ( ) )
			{
				float fX = ( vIter->fX + vIterPrev->fX ) / 2.0f;
				float fZ = ( vIter->fY + vIterPrev->fY ) / 2.0f;
				float fDirX = vIter->fX - vIterPrev->fX;
				float fDirZ = vIter->fY - vIterPrev->fY;
				float fDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );
				if ( fDist < 0.1f ) 
				{
					vIterPrev = vIter;
					vIter++;
					continue;
				}
				fDirX /= fDist;
				fDirZ /= fDist;
				float fAngY = acos ( fDirZ ) * 57.295779513f;
				if ( fDirX < 0.0f ) fAngY = 360.0f - fAngY;

				//AddLimb ( iVBPolygonBoundsObject, iLimb, iTempMesh );
				//ScaleLimb ( iVBPolygonBoundsObject, iLimb, 100.0f, 100.0f, fDist * 100.0f );
				//RotateLimb ( iVBPolygonBoundsObject, iLimb, 0.0f, fAngY, 0.0f );
				//OffsetLimb ( iVBPolygonBoundsObject, iLimb, fX, fHeight, fZ );//0.0f, fZ );
				AddLimb ( iVBPolygonBoundsObject, iLimb, iTempMesh );
				float fRealStartY = GetLUATerrainHeightEx ( vIterPrev->fX, vIterPrev->fY );
				PositionObject ( iTempObj1, vIterPrev->fX, fRealStartY, vIterPrev->fY );
				float fRealFinishY = GetLUATerrainHeightEx ( vIter->fX, vIter->fY );
				fDirX = vIter->fX - vIterPrev->fX;
				float fDirY = fRealFinishY - fRealStartY;
				fDirZ = vIter->fY - vIterPrev->fY;
				fDist = sqrt ( fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ );
				ScaleLimb ( iVBPolygonBoundsObject, iLimb, 100.0f, 100.0f, fDist * 100.0f );
				PointObject ( iTempObj1, vIter->fX, fRealFinishY, vIter->fY );
				float fRealMiddleY = fRealStartY+((fRealFinishY-fRealStartY)/2.0f);
				OffsetLimb ( iVBPolygonBoundsObject, iLimb, fX, fRealMiddleY + 7.5f, fZ );
				RotateLimb ( iVBPolygonBoundsObject, iLimb, ObjectAngleX(iTempObj1), ObjectAngleY(iTempObj1), ObjectAngleZ(iTempObj1) );
				iLimb++;

				vIterPrev = vIter;
				vIter++;
			}
		}

		pIter++;
	}
	
	//delete the plane mesh
	DeleteMesh ( iTempMesh );

	// delete temp object
	if ( ObjectExist(iTempObj1) == 1 ) DeleteObject ( iTempObj1 );

	//collect all limbs into a single limb for speed
	dbCombineLimbs ( iPolygonBoundsObject );
	dbCombineLimbs ( iHHPolygonBoundsObject );
	dbCombineLimbs ( iVBPolygonBoundsObject );
	
	PositionObject ( iPolygonBoundsObject, 0.0f, 0.0f, 0.0f );// fHeight, 0.0f );
	PositionObject ( iHHPolygonBoundsObject, 0.0f, 0.0f, 0.0f );// fHeight, 0.0f );
	PositionObject ( iVBPolygonBoundsObject, 0.0f, 0.0f, 0.0f );// fHeight, 0.0f );

	SetObjectCollisionOff ( iPolygonBoundsObject );
	SetObjectCollisionOff ( iHHPolygonBoundsObject );
	SetObjectCollisionOff ( iVBPolygonBoundsObject );

	//colour the objects.
	ColorObject ( iPolygonBoundsObject, 0xff00ff00 );
	ColorObject ( iHHPolygonBoundsObject, 0xff005A00 );
	ColorObject ( iVBPolygonBoundsObject, 0xff00ffff );
	SetObjectEmissive ( iPolygonBoundsObject, 0xff00ff00 );
	SetObjectEffect ( iPolygonBoundsObject, g_GUIShaderEffectID );
	SetObjectEmissive ( iHHPolygonBoundsObject, 0xff005A00 );
	SetObjectEffect ( iHHPolygonBoundsObject, g_GUIShaderEffectID );
	SetObjectEmissive ( iVBPolygonBoundsObject, 0xff00ffff );
	SetObjectEffect ( iVBPolygonBoundsObject, g_GUIShaderEffectID );

	SetObjectEmissive ( iPolygonBoundsObject, 0xff000000 );
	SetObjectEmissive ( iHHPolygonBoundsObject, 0xff000000 );
	SetObjectEmissive ( iVBPolygonBoundsObject, 0xff000000 );
}

void PathFinderAdvanced::DebugHidePolygonBounds ( )
{
	if ( iPolygonBoundsObject > 0 && ObjectExist ( iPolygonBoundsObject ) )
	{
		DeleteObject ( iPolygonBoundsObject );
	}

	iPolygonBoundsObject = 0;

	if ( iHHPolygonBoundsObject > 0 && ObjectExist ( iHHPolygonBoundsObject ) )
	{
		DeleteObject ( iHHPolygonBoundsObject );
	}

	iHHPolygonBoundsObject = 0;

	if ( iVBPolygonBoundsObject > 0 && ObjectExist ( iVBPolygonBoundsObject ) )
	{
		DeleteObject ( iVBPolygonBoundsObject );
	}

	iVBPolygonBoundsObject = 0;
}

void PathFinderAdvanced::DebugUpdatePolygonColour( float fTimeDelta )
{
	//if ( iPolygonBoundsObject == 0 ) return;
	//if ( iHHPolygonBoundsObject == 0 ) return;
	//if ( iVBPolygonBoundsObject == 0 ) return;

	fFlashingTimer += fTimeDelta*2.0f;
	if ( fFlashingTimer > 12 ) fFlashingTimer -= 12;

	//char str[256];

	DWORD iColourValue = (DWORD) ( ( sin( fFlashingTimer ) + 1.0f ) * 127.0f );
	DWORD dwColour = 0xff000000 
			  | ( ( 0x000000ff & iColourValue ) << 16 ) 
			  | ( ( 0x000000ff & iColourValue ) << 8 )
			  | ( ( 0x000000ff & iColourValue ) );

	//sprintf(str, "FlashTime: %3.3f, timeDelta: %3.3f, Colour: %d, dwColour: %x\n", fFlashingTimer, fTimeDelta, iColourValue, dwColour );
	//FILE *pInfo = GG_fopen("Flashing.txt","a");
	//fputs(str,pInfo);
	//fclose(pInfo);

	if ( iPolygonBoundsObject != 0 ) SetObjectEmissive( iPolygonBoundsObject, dwColour );
	if ( iHHPolygonBoundsObject != 0 ) SetObjectEmissive( iHHPolygonBoundsObject, dwColour );
	if ( iVBPolygonBoundsObject != 0 ) SetObjectEmissive( iVBPolygonBoundsObject, dwColour );
}

void PathFinderAdvanced::DebugDrawAvoidanceGrid( float fHeight )
{
	bShowAvoidanceGrid = true;
	fAvoidanceGridDebugTimer = 0;
	fAvoidanceGridHeight = fHeight;

	if ( iGridObject <= 0 || ObjectExist ( iGridObject ) == 0 )
	{
		//make a plane mesh to represent edges.
		int iTempMesh = dbMakeEdgeMesh ( );
		if ( iTempMesh == 0 ) return;
		
		//clear any current debug object
		if ( iGridObject > 0 && ObjectExist ( iGridObject ) == 1 )
		{
			DeleteObject ( iGridObject );
		}
		
		//find a free object
		if ( iGridObject == 0 ) iGridObject = dbFreeObject ( );
		if ( iGridObject == 0 ) return;

		//root node
		MakeObjectPlane ( iGridObject, 0.0f, 0.0f, 1 );
		SetObjectMask ( iGridObject, 1 );	

		int iLimb = 1;

		for( int i = 0; i < 100; i ++ )
		{
			AddLimb ( iGridObject, iLimb, iTempMesh );
			ScaleLimb ( iGridObject, iLimb, 100.0f, 100.0f, 100.0f * fGridRadius * 100.0f );
			//RotateLimb ( iGridObject, iLimb, 0.0f, 90.0f, 0.0f );
			OffsetLimb ( iGridObject, iLimb, i * fGridRadius, 0.0f, -50.0f * fGridRadius );

			iLimb++;

			AddLimb ( iGridObject, iLimb, iTempMesh );
			ScaleLimb ( iGridObject, iLimb, 100.0f, 100.0f, 100.0f * fGridRadius * 100.0f );
			RotateLimb ( iGridObject, iLimb, 0.0f, 90.0f, 0.0f );
			OffsetLimb ( iGridObject, iLimb, 50.0f * fGridRadius, 0.0f, -i * fGridRadius );

			iLimb++;
		}

		//delete the plane mesh
		DeleteMesh ( iTempMesh );

		//collect all limbs into a single limb for speed
		dbCombineLimbs ( iGridObject );
		PositionObject ( iGridObject, 0.0f, fAvoidanceGridHeight, 0.0f );
		SetObjectCollisionOff ( iGridObject );
		ColorObject ( iGridObject, 0x000000ff );
		SetObjectEmissive ( iGridObject, 0x000000ff );
		SetObjectEffect ( iGridObject, g_GUIShaderEffectID );
	}
}

void PathFinderAdvanced::DebugHideAvoidanceGrid( )
{
	bShowAvoidanceGrid = false;

	if ( iGridObject > 0 && ObjectExist ( iGridObject ) )
	{
		DeleteObject ( iGridObject );
	}

	iGridObject = 0;

	if ( iGridObject2 > 0 && ObjectExist ( iGridObject2 ) )
	{
		DeleteObject ( iGridObject2 );
	}

	iGridObject2 = 0;
}

void PathFinderAdvanced::DebugUpdateAvoidanceGrid( float fTimeDelta )
{
	if ( !bShowAvoidanceGrid ) return;

	fAvoidanceGridDebugTimer -= fTimeDelta;

	if ( fAvoidanceGridDebugTimer <= 0 )
	{
		fAvoidanceGridDebugTimer = 0.2f;

		int iTempObj = dbFreeObject ( );
		if ( iTempObj == 0 ) return;
		int iTempMesh = dbFreeMesh ( );
		if ( iTempMesh == 0 ) return;

		MakeObjectPlane ( iTempObj, fGridRadius - 0.3f, fGridRadius - 0.3f, 1 );
		XRotateObject ( iTempObj, 90.0f );
		FixObjectPivot ( iTempObj );
		MakeMeshFromObject ( iTempMesh, iTempObj );
		DeleteObject ( iTempObj );

		int iTempMesh2 = dbMakeEdgeMesh ( );
		if ( iTempMesh2 == 0 ) return;

		if ( iGridObject2 > 0 && ObjectExist ( iGridObject2 ) == 1 )
		{
			DeleteObject ( iGridObject2 );
		}
		
		//find a free object
		if ( iGridObject2 == 0 ) iGridObject2 = dbFreeObject ( );
		if ( iGridObject2 == 0 ) return;

		//root node
		MakeObjectPlane ( iGridObject2, 0.0f, 0.0f, 1 );
		SetObjectMask ( iGridObject2, 1 );	

		int iLimb = 1;
		float length = sqrt( 2*fGridRadius*fGridRadius );

		for( int i = 0; i < 100; i++ )
		{
			for( int j = 0; j < 100; j++ )
			{
				GridElement *pElement = cGrid.GetRootElement( i,j );

				while ( pElement )
				{
					if ( pElement->GetValue() > 0 )
					{
						AddLimb ( iGridObject2, iLimb, iTempMesh );
						OffsetLimb ( iGridObject2, iLimb, GridItoF(pElement->GetX()), 0.0f, GridItoF(pElement->GetZ()) );

						iLimb++;
					}

					if ( pElement->GetReserved() > 0 )
					{
						AddLimb ( iGridObject2, iLimb, iTempMesh );
						ScaleLimb ( iGridObject2, iLimb, 50.0f, 50.0f, 50.0f );
						RotateLimb ( iGridObject2, iLimb, 0.0f, 45.0f, 0.0f );
						OffsetLimb ( iGridObject2, iLimb, GridItoF(pElement->GetX()), 0.0f, GridItoF(pElement->GetZ()) );

						iLimb++;
					}

					pElement = pElement->pNextElement;
				}

				GridElement *pUElement = cUndesirableGrid.GetRootElement( i,j );

				while ( pUElement )
				{
					if ( pUElement->GetValue() > 0 )
					{
						AddLimb ( iGridObject2, iLimb, iTempMesh2 );
						ScaleLimb ( iGridObject2, iLimb, 100.0f, 100.0f, length * 100.0f );
						RotateLimb ( iGridObject2, iLimb, 0.0f, 45.0f, 0.0f );
						OffsetLimb ( iGridObject2, iLimb, GridItoF(pUElement->GetX()), 0.0f, GridItoF(pUElement->GetZ()) );

						iLimb++;

						AddLimb ( iGridObject2, iLimb, iTempMesh2 );
						ScaleLimb ( iGridObject2, iLimb, 100.0f, 100.0f, length * 100.0f );
						RotateLimb ( iGridObject2, iLimb, 0.0f, 135.0f, 0.0f );
						OffsetLimb ( iGridObject2, iLimb, GridItoF(pUElement->GetX()), 0.0f, GridItoF(pUElement->GetZ()) );

						iLimb++;
					}

					pUElement = pUElement->pNextElement;
				}
			}
		}

		DeleteMesh ( iTempMesh );
		DeleteMesh ( iTempMesh2 );

		//collect all limbs into a single limb for speed
		dbCombineLimbs ( iGridObject2 );
		PositionObject ( iGridObject2, 0.0f, fAvoidanceGridHeight, 0.0f );
		SetObjectCollisionOff ( iGridObject2 );
		ColorObject ( iGridObject2, 0x00ff0000 );
		SetObjectEmissive ( iGridObject2, 0x00ff0000 );
		SetObjectEffect ( iGridObject2, g_GUIShaderEffectID );
	}
}
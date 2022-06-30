#define _CRT_SECURE_NO_DEPRECATE

#include "DynamicPathFinder.h"

#include "Path.h"
#include "Container.h"
#include "DBPro Functions.h"

#include <math.h>
#include <algorithm>

#include <stdio.h>
#include <windows.h>

#include "World.h"
#include "PathFinderAdvanced.h"

using namespace std;

extern World cWorld;
extern int g_GUIShaderEffectID;

#define _2PI 6.28318530718f

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

AvoidanceObject::AvoidanceObject( )
{
	pNextObject = 0;
	pNextLocalObject = 0;
	id = 0;
}

AvoidanceObject::~AvoidanceObject( )
{
	
}

AvoidanceSphere::AvoidanceSphere( float radius, float x, float z )
{
	fRadius = radius;
	fX = x;
	fZ = z;
}

AvoidanceSphere::~AvoidanceSphere( )
{
	
}

bool AvoidanceSphere::Intersects( float fSX, float fSZ, float fEX, float fEZ ) const
{
	if ( fRadius <= 0 ) return false;

	float fDiffX = fEX - fSX;
	float fDiffZ = fEZ - fSZ;

	float fDiffX2 = fX - fSX;
	float fDiffZ2 = fZ - fSZ;

	//if ( ( fDiffX2*fDiffX2 + fDiffZ2*fDiffZ2) < fRadius*fRadius ) return false;

	float dist = (fDiffX*fDiffX2 + fDiffZ*fDiffZ2) / (fDiffX*fDiffX + fDiffZ*fDiffZ);
                
    if ( dist <= 0 ) {
        //if ( (fDiffX2*fDiffX2 + fDiffZ2*fDiffZ2) < fRadius*fRadius ) return true;
		return false;
    }
    else {
        if ( dist >= 1 ) {
            fDiffX2 = fX - fEX;
			fDiffZ2 = fZ - fEZ;
            if ( (fDiffX2*fDiffX2 + fDiffZ2*fDiffZ2) < fRadius*fRadius ) return true;
        }
        else {
            float newx = fSX + fDiffX*dist;
            float newz = fSZ + fDiffZ*dist;
			fDiffX2 = fX - newx;
			fDiffZ2 = fZ - newz;
            if ( (fDiffX2*fDiffX2 + fDiffZ2*fDiffZ2) < fRadius*fRadius ) return true;
        }
    }
    
    return false;
}

bool AvoidanceSphere::Inside( float x, float z, AvoidanceObject **pInObject )
{
	if ( fRadius <= 0 ) return false;

	float fDiffX = fX - x;
	float fDiffZ = fZ - z;

	if ( (fDiffX*fDiffX + fDiffZ*fDiffZ) < fRadius*fRadius ) 
	{
		if ( pInObject ) *pInObject = this;
		return true;
	}
	else return false;
}

void AvoidanceSphere::GetCenter( float &x, float &z ) const
{
	x = fX; z = fZ;
}

void AvoidanceSphere::GetMin( float &x, float &z ) const
{
	x = fX - fRadius; 
	z = fZ - fRadius;
}

void AvoidanceSphere::GetMax( float &x, float &z ) const
{
	x = fX + fRadius; 
	z = fZ + fRadius;
}

int AvoidanceSphere::CreateWaypoints( DynamicPathFinder *pPathFinder )
{
	float fNewX, fNewZ;

	int iWaypoints = 6;
	bool bPrevValid = false;
	DynamicPathFinder::sWaypoint *pPrevWaypoint = 0;
	DynamicPathFinder::sWaypoint *pFirstWaypoint = 0;

	for ( int i = 0; i < iWaypoints; i++ )
	{
		fNewX = fX + sin( _2PI/iWaypoints * i ) * fRadius * 1.2f;
		fNewZ = fZ + cos( _2PI/iWaypoints * i ) * fRadius * 1.2f;

		if ( pPathFinder->CheckValidPoint( fNewX, fNewZ ) )
		{
			DynamicPathFinder::sWaypoint *pWaypoint = pPathFinder->AddWaypoint( fNewX, fNewZ, this );
			if ( bPrevValid )
			{
				pWaypoint->AddEdge( pPrevWaypoint, ActualDistance( pPrevWaypoint->fX, pPrevWaypoint->fY, pWaypoint->fX, pWaypoint->fY ) );
				pPrevWaypoint->AddEdge( pWaypoint, ActualDistance( pPrevWaypoint->fX, pPrevWaypoint->fY, pWaypoint->fX, pWaypoint->fY ) );
			}

			if ( i == iWaypoints - 1 && pFirstWaypoint )
			{
				pFirstWaypoint->AddEdge( pWaypoint, ActualDistance( pFirstWaypoint->fX, pFirstWaypoint->fY, pWaypoint->fX, pWaypoint->fY ) );
				pWaypoint->AddEdge( pFirstWaypoint, ActualDistance( pFirstWaypoint->fX, pFirstWaypoint->fY, pWaypoint->fX, pWaypoint->fY ) );
			}

			pPrevWaypoint = pWaypoint;
			bPrevValid = true;
			if ( i == 0 ) pFirstWaypoint = pWaypoint;
		}
		else
		{
			pPrevWaypoint = 0;
			bPrevValid = false;
		}
	}

	return iWaypoints;
}

DynamicCollisionNode::DynamicCollisionNode( int iNumObjects, int iObjectsPerNode, AvoidanceObject *pObjectList )
{
	left = 0;
	right = 0;

	ReBuildTree( iNumObjects, iObjectsPerNode, pObjectList );
}

void DynamicCollisionNode::ReBuildTree( int iNumObjects, int iObjectsPerNode, AvoidanceObject *pObjectList )
{
	Reset( );

	left = 0; right = 0;
	fMinX = 0; fMaxX = 0;
	fMinZ = 0; fMaxZ = 0;

	if ( !pObjectList ) return;

	pObjectList->GetMin( fMinX, fMinZ );
	pObjectList->GetMax( fMaxX, fMaxZ );
	
	AvoidanceObject *pObject = pObjectList;

	while ( pObject )
	{
		pObject->pNextLocalObject = pObject->pNextObject;
		pObject = pObject->pNextObject;
	}

	pObject = pObjectList;

	float fX = 0, fZ = 0;
	while ( pObject )
	{
		pObject->GetMin( fX, fZ );
		if ( fX < fMinX ) fMinX = fX;
		if ( fZ < fMinZ ) fMinZ = fZ;

		pObject->GetMax( fX, fZ );
		if ( fX > fMaxX ) fMaxX = fX;
		if ( fZ > fMaxZ ) fMaxZ = fZ;

		pObject = pObject->pNextObject;
	}

	fMinX -= 0.0001f;
	fMinZ -= 0.0001f;
	fMaxX += 0.0001f;
	fMaxZ += 0.0001f;

	if ( iNumObjects <= iObjectsPerNode )
	{
		left = ( DynamicCollisionNode* ) pObjectList;
		return;
	}

	float fDiffX = fMaxX - fMinX;
	float fDiffZ = fMaxZ - fMinZ;
	int axis = 0;
	float fLimit = fDiffX/2.0f + fMinX;
	float avg = 0;

	if ( fDiffZ > fDiffX ) 
	{
		axis = 1;
		fLimit = fDiffZ/2.0f + fMinZ;
	}

	AvoidanceObject *pLeftObjects = 0;
	AvoidanceObject *pRightObjects = 0;
	AvoidanceObject *pLastObject = 0;
	AvoidanceObject *pTemp = 0;
	int iNumObjects1 = 0;
	int iNumObjects2 = 0;

	while ( pObjectList )
	{
		pObjectList->GetCenter( fX, fZ );
		avg = axis==0 ? fX : fZ;

		if ( avg > fLimit )
		{
			if (iNumObjects1 == 0) pLastObject = pObjectList;

			pTemp = pObjectList->pNextLocalObject;
			pObjectList->pNextLocalObject = pLeftObjects;
			pLeftObjects = pObjectList;
			pObjectList = pTemp;

			iNumObjects1++;
		}
		else
		{
			pTemp = pObjectList->pNextLocalObject;
			pObjectList->pNextLocalObject = pRightObjects;
			pRightObjects = pObjectList;
			pObjectList = pTemp;

			iNumObjects2++;
		}
	}

	if ( abs(iNumObjects1 - iNumObjects2) > 0.9*iNumObjects )
	{
		iNumObjects1 = 0;
		iNumObjects2 = 0;

		if ( pLeftObjects )
		{
			pObjectList = pLeftObjects;
			pLastObject->pNextLocalObject = pRightObjects;
		}
		else pObjectList = pRightObjects;

		pLeftObjects = 0;
		pRightObjects = 0;

		int iFlag = 1;

		while ( pObjectList )
		{
			if ( iFlag )
			{
				pTemp = pObjectList->pNextLocalObject;
				pObjectList->pNextLocalObject = pLeftObjects;
				pLeftObjects = pObjectList;
				pObjectList = pTemp;

				iNumObjects1++;
				iFlag = 0;
			}
			else
			{
				pTemp = pObjectList->pNextLocalObject;
				pObjectList->pNextLocalObject = pRightObjects;
				pRightObjects = pObjectList;
				pObjectList = pTemp;

				iNumObjects2++;
				iFlag = 1;
			}
		}
	}

	left = new DynamicCollisionNode( iNumObjects1, iObjectsPerNode, pLeftObjects );
	right = new DynamicCollisionNode( iNumObjects2, iObjectsPerNode, pRightObjects );
}

DynamicCollisionNode::~DynamicCollisionNode( )
{
	Reset( );
}

void DynamicCollisionNode::Reset( )
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
			AvoidanceObject *pTemp = (AvoidanceObject*) left;
			left = (DynamicCollisionNode*) ( (AvoidanceObject*)left )->pNextObject;
			//delete pTemp;
		}
	}

	right = 0;
	left = 0;
}

bool DynamicCollisionNode::Intersects ( float fSX, float fSZ, float fEX, float fEZ )
{
	float fVX = 1.0f/(fEX - fSX);
	float fVZ = 1.0f/(fEZ - fSZ);

	float tminx,tmaxx,tminz,tmaxz;

    if (fVX>=0) {
        tminx = (fMinX - fSX) *fVX;
        tmaxx = (fMaxX - fSX) *fVX;
    }
    else {
        tmaxx = (fMinX - fSX) *fVX;
        tminx = (fMaxX - fSX) *fVX;
    }
    
    if (fVZ>=0) {
        tminz = (fMinZ - fSZ) *fVZ;
        tmaxz = (fMaxZ - fSZ) *fVZ;
    }
    else {
        tmaxz = (fMinZ - fSZ) *fVZ;
        tminz = (fMaxZ - fSZ) *fVZ;
    }
               
    if ((tminx > tmaxz) || (tminz > tmaxx)) return false;
		
	if (tminz>tminx) tminx = tminz;
    if (tmaxz<tmaxx) tmaxx = tmaxz;

	if (tmaxx<0.0) return false;
    if (tminx>1.0) return false;
	
	if ( right )
	{
		if ( left->Intersects ( fSX, fSZ, fEX, fEZ ) ) return true;
		return right->Intersects ( fSX, fSZ, fEX, fEZ );
	}
	else
	{
		AvoidanceObject *pObject = (AvoidanceObject*) left;
		while ( pObject )
		{
			if ( pObject->Intersects( fSX, fSZ, fEX, fEZ ) ) return true;

			pObject = pObject->pNextLocalObject;
		}

		return false;
	}
}

bool DynamicCollisionNode::Inside ( float x, float z, AvoidanceObject **pInObject )
{
	if ( x > fMaxX || x < fMinX ) return false;
	if ( z > fMaxZ || z < fMinZ ) return false;
	
	if ( right )
	{
		if ( left->Inside ( x,z, pInObject ) ) return true;
		return right->Inside ( x,z, pInObject );
	}
	else
	{
		AvoidanceObject *pObject = (AvoidanceObject*) left;
		while ( pObject )
		{
			if ( pObject->Inside( x,z, pInObject ) ) return true;

			pObject = pObject->pNextLocalObject;
		}

		return false;
	}
}

void DynamicPathFinder::sWaypoint::AddEdge( sWaypoint *pWaypoint, float fCost )
{
	if ( fCost < 0 ) fCost = 0;

	sWaypointEdge *pEdge = new sWaypointEdge;
	pEdge->pOtherWP = pWaypoint;
	pEdge->fCost = fCost;
	
	pEdge->pNextEdge = pEdgeList;
	pEdgeList = pEdge;

	iNumEdges++;
}

void DynamicPathFinder::sWaypoint::RemoveLastEdge ( sWaypoint *pTargetWP )
{
	sWaypointEdge *pEdge = pEdgeList;
	if ( !pEdge ) return;

	if ( pEdge->pOtherWP != pTargetWP ) return;

	pEdgeList = pEdgeList->pNextEdge;

	delete pEdge;

	iNumEdges--;
}

void DynamicPathFinder::sWaypoint::RemoveEdge ( sWaypoint *pOtherWaypoint )
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

void DynamicPathFinder::sWaypoint::ClearEdges ( )
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

DynamicPathFinder::sWaypoint::~sWaypoint ( )
{
	ClearEdges ( );
}

DynamicPathFinder::sWaypoint* DynamicPathFinder::AddWaypoint ( float x, float z, AvoidanceObject *pOwner )
{
	sWaypoint *pNewWP = new sWaypoint( );
	pNewWP->fX = x;
	pNewWP->fY = z;
	pNewWP->bActive = true;
	pNewWP->iVisited = 0;
	pNewWP->pObjectOwner = pOwner;

	pNewWP->pNextWaypoint = pWaypointList;
	pWaypointList = pNewWP;

	iNumWaypoints++;

	return pWaypointList;
}

void DynamicPathFinder::AddWaypoint ( sWaypoint *pNewWP )
{
	pNewWP->pNextWaypoint = pWaypointList;
	pWaypointList = pNewWP;

	iNumWaypoints++;
}

void DynamicPathFinder::ClearWaypoints ( )
{
	DebugHideWaypoints( );
	DebugHideWaypointEdges( );

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

void DynamicPathFinder::RemoveWaypoint ( int iIndex )
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

void DynamicPathFinder::UpdateVisibility ( float fLimit )
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
			bool bValid = ( !pWaypoint->pObjectOwner && pWaypoint != pWaypoint2 ) || ( pWaypoint->pObjectOwner != pWaypoint2->pObjectOwner );
			bValid = bValid && !QuickCheckRay ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY );
			bValid = bValid && !pMainObstacles->QuickPolygonsCheck( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY, 2 );
			
			if ( bValid )
            {
				float fDist = ActualDistance ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY );
                if ( fLimit < 0 || fDist < fLimit ) 
				{
					pWaypoint->AddEdge ( pWaypoint2, fDist );
				}
            }
            
			pWaypoint2 = pWaypoint2->pNextWaypoint;
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void DynamicPathFinder::UpdateSingleVisibility ( DynamicPathFinder::sWaypoint *pNewWP, float fLimit, bool duplex )
{
	pNewWP->ClearEdges( );
	sWaypoint *pWaypoint = pWaypointList;
    
	while ( pWaypoint )
    {
		bool bValid = pWaypoint != pNewWP;
		bValid = bValid && !QuickCheckRay ( pNewWP->fX, pNewWP->fY, pWaypoint->fX, pWaypoint->fY );
		bValid = bValid && !pMainObstacles->QuickPolygonsCheck( pNewWP->fX, pNewWP->fY, pWaypoint->fX, pWaypoint->fY, 2 );
			
		if ( bValid )
        {
			float fDist = ActualDistance ( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY );
            if ( fLimit < 0 || fDist < fLimit ) 
			{
				pNewWP->AddEdge( pWaypoint, fDist );
				if ( duplex ) 
				{
					if ( !QuickCheckRay ( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY )
					  && !pMainObstacles->QuickPolygonsCheck( pWaypoint->fX, pWaypoint->fY, pNewWP->fX, pNewWP->fY, 2 ) )
					{
						pWaypoint->AddEdge( pNewWP, fDist );
					}
				}
			}
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void DynamicPathFinder::RemoveAllEdges ( )
{
	sWaypoint *pWaypoint = pWaypointList;

	while ( pWaypoint )
	{
		pWaypoint->ClearEdges ( );
		pWaypoint = pWaypoint->pNextWaypoint;
	}
}

DynamicPathFinder::DynamicPathFinder ( PathFinderAdvanced *pMain )
{   
	pObjectList = 0;

    //sWaypoint_list.clear ( );
	pWaypointList = 0;
    sWaypointOpen_list.clear ( );

	iWaypointObject = 0;
	iWaypointEdgeObject = 0;

	pMainObstacles = pMain;

	pOptimizedCol = 0;

	iNumWaypoints = 0;
}

DynamicPathFinder::~DynamicPathFinder ( )
{   
	DebugHideWaypoints( );
	DebugHideWaypointEdges( );

    ClearObjects ( );
	ClearWaypoints ( );

	if ( pOptimizedCol ) delete pOptimizedCol;
}

void DynamicPathFinder::SetMainPathFinder( PathFinderAdvanced *pMain )
{
	if ( !pMain ) MessageBox( NULL, "Caught null pointer", "AI Error", 0 );
	pMainObstacles = pMain;
}

DynamicPathFinder::sWaypoint* DynamicPathFinder::GetClosestWaypoint ( float fSX, float fSY )
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

void DynamicPathFinder::BuildWaypoints( )
{
    if ( !pObjectList ) return;
    
    ClearWaypoints ( );
    sWaypointOpen_list.clear ( );
    
	AvoidanceObject *pObject = pObjectList;
    
    while ( pObject )
    {
		pObject->CreateWaypoints( this );	
        
		pObject = pObject->pNextObject;
    }

    sWaypoint *pWaypoint = pWaypointList;
    sWaypoint *pWaypoint2;
    
	while ( pWaypoint )
    {
		//pWaypoint->pEdgeList = 0;
        
        pWaypoint2 = pWaypointList;
        
        while ( pWaypoint2 )
        {
			bool bValid = ( !pWaypoint->pObjectOwner && pWaypoint != pWaypoint2 ) || ( pWaypoint->pObjectOwner != pWaypoint2->pObjectOwner );
			if ( bValid && !QuickCheckRay ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY ) )
            {
                pWaypoint->AddEdge ( pWaypoint2, ActualDistance ( pWaypoint->fX, pWaypoint->fY, pWaypoint2->fX, pWaypoint2->fY ) );
            }
            
			pWaypoint2 = pWaypoint2->pNextWaypoint;
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }

	//DebugDrawWaypoints( 203 );
	//DebugDrawWaypointEdges( 2.5 );
}

void DynamicPathFinder::ClearObjects ( )
{
	if ( pOptimizedCol ) pOptimizedCol->Reset( );

    AvoidanceObject *pObject = pObjectList;

	while ( pObjectList )
	{
		pObjectList = pObjectList->pNextObject;
		delete pObject;
		pObject = pObjectList;
	}
}

void DynamicPathFinder::AddObject ( AvoidanceObject *pObject, bool bUpdate )
{
	if ( !pObject ) return;

	pObject->pNextObject = pObjectList;
	pObject->pNextLocalObject = pObjectList;
	pObjectList = pObject;

	int count = 0;
	while ( pObject )
	{
		count++;
		pObject = pObject->pNextObject;
	}

	if ( bUpdate && pOptimizedCol ) pOptimizedCol->ReBuildTree( count, 2, pObjectList );
}
void DynamicPathFinder::RemoveObject ( int id, bool bUpdate )
{
	AvoidanceObject *pObject = pObjectList;
	AvoidanceObject *pPrevObject = 0;

	if ( bUpdate && pOptimizedCol ) pOptimizedCol->Reset( );

	while ( pObject )
	{
		if ( pObject->id == id )
		{
			AvoidanceObject *pTemp = pObject;
			if ( !pPrevObject ) pObjectList = pObject->pNextObject;
			else pPrevObject->pNextObject = pObject->pNextObject;

			delete pObject;
		}
	}

	pObject = pObjectList;

	int count = 0;
	while ( pObject )
	{
		count++;
		pObject = pObject->pNextObject;
	}

	if ( bUpdate && pOptimizedCol ) pOptimizedCol->ReBuildTree( count, 2, pObjectList ); 
}

void DynamicPathFinder::CompleteObstacles( )
{	
	if ( pOptimizedCol ) delete pOptimizedCol;
	pOptimizedCol = 0;
	//if ( fCurrRadius > 0 ) SetRadius( fCurrRadius );
	
	AvoidanceObject *pObject = pObjectList;

	int count = 0;
	while ( pObject )
	{
		count++;
		pObject = pObject->pNextObject;
	}

	//pOptimizedCol = new DynamicCollisionNode( count, 2, pObjectList );

	//BuildWaypoints( );
}


//just returns true if the ray hits something, false if not
bool DynamicPathFinder::QuickCheckRay ( float fSX, float fSY, float fEX, float fEY )
{
	AvoidanceObject *pObject = pObjectList;

	while ( pObject )
	{
		if ( pObject->Intersects( fSX, fSY, fEX, fEY ) ) return true;
		pObject = pObject->pNextObject;
	}

    /*
	if ( pOptimizedCol )
	{
		if ( pOptimizedCol->Intersects( fSX, fSY, fEX, fEY ) ) return true;

		return pMainObstacles->QuickPolygonsCheck( fSX, fSY, fEX, fEY, 2 );
	}
	else
	{
		MessageBox( NULL, "Collision tree does not exist", "AI path finding error", 0 );
		exit(-1);
	}
	*/

	return false;
}

bool DynamicPathFinder::CheckValidPoint( float fX, float fY )
{
	//bool bInside = pOptimizedCol->Inside( fX, fY, NULL );
	//if ( bInside ) return false;

	AvoidanceObject *pObject = pObjectList;

	while ( pObject )
	{
		if ( pObject->Inside( fX, fY, NULL ) ) return false;
		pObject = pObject->pNextObject;
	}

	return pMainObstacles->InPolygons( fX, fY ) == 0;
}


int DynamicPathFinder::InObject ( float fSX, float fSY, AvoidanceObject **pInObject )
{
	//return pOptimizedCol->Inside( fSX, fSY, pInObject ) ? 1 : 0;

	AvoidanceObject *pObject = pObjectList;

	while ( pObject )
	{
		if ( pObject->Inside( fSX, fSY, pInObject ) ) return 1;
		pObject = pObject->pNextObject;
	}

	return 0;
}

bool DynamicPathFinder::GetClosestPoint( const AvoidanceObject *pObject, float x, float z, float &closestX, float &closestZ )
{
	sWaypoint *pWaypoint = pWaypointList;
	float fClosest = -1;

	while( pWaypoint )
	{
		if ( pWaypoint->pObjectOwner == pObject )
		{
			float fDiffX = x - pWaypoint->fX;
			float fDiffZ = z - pWaypoint->fY;
			float dist = fDiffX*fDiffX + fDiffZ*fDiffZ;
			if ( fClosest < 0 || dist < fClosest )
			{
				fClosest = dist;
				closestX = pWaypoint->fX;
				closestZ = pWaypoint->fY;
			}
		}

		pWaypoint = pWaypoint->pNextWaypoint;
	}

	return fClosest >= 0;
}

bool DynamicPathFinder::ShortestPath ( float fEX, float fEY, Path* pBuildPath )
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
        
		if ( pWaypoint->fDistH < 1 )
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
	            
				//if ( wNextIter->bActive )
				{
					if ( pNextWP->iVisited == 0 )
					{
						pNextWP->pParent = pWaypoint;
						pNextWP->fDistG = pWaypoint->fDistG + pEdge->fCost;
						pNextWP->fDistH = EstimateDistance ( pNextWP->fX, pNextWP->fY, fEX, fEY );
						pNextWP->fDistF = pNextWP->fDistG + pNextWP->fDistH;
						pNextWP->iVisited = 1;
		                
						pNewInfo [ iCount ].pThisWP = pNextWP;
						pNewInfo [ iCount ].fCost = pNextWP->fDistF;
						iCount++;
					}
					else if ( pNextWP->iVisited == 1 )
					{
						if ( pWaypoint->fDistG + pEdge->fCost < pNextWP->fDistG )
						{
							pNextWP->pParent = pWaypoint;
							pNextWP->fDistG = pWaypoint->fDistG + pEdge->fCost;
							pNextWP->fDistH = EstimateDistance ( pNextWP->fX, pNextWP->fY, fEX, fEY );
							pNextWP->fDistF = pNextWP->fDistG + pNextWP->fDistH;
		                    
							pNewInfo [ iCount ].pThisWP = pNextWP;
							pNewInfo [ iCount ].fCost = pNextWP->fDistF;
							iCount++;
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
		pBuildPath->InsertPoint ( 0, pWaypoint->fX, pWaypoint->fY, -1 );
        
        do
        {
            pWaypoint = pWaypoint->pParent;
            pBuildPath->InsertPoint ( 0, pWaypoint->fX, pWaypoint->fY, -1 );
            
        } while ( pWaypoint != pWaypoint->pParent );
        
        return true;
    }
	else
	{
		sWaypoint *pThisWaypoint = pWaypointList;
		sWaypoint *pClosestWaypoint = 0;

		float fClosest = -1;

		while ( pThisWaypoint )
		{
			if ( pThisWaypoint->pParent )
			{
				if ( fClosest < 0 || pThisWaypoint->fDistH < fClosest ) 
				{
					fClosest = pThisWaypoint->fDistH;
					pClosestWaypoint = pThisWaypoint;
				}
			}

			pThisWaypoint = pThisWaypoint->pNextWaypoint;
		}

		if ( pClosestWaypoint )
		{
			pBuildPath->InsertPoint ( 0, pClosestWaypoint->fX, pClosestWaypoint->fY, -1 );
        
			do
			{
				pClosestWaypoint = pClosestWaypoint->pParent;
				pBuildPath->InsertPoint ( 0, pClosestWaypoint->fX, pClosestWaypoint->fY, -1 );
	            
			} while ( pClosestWaypoint != pClosestWaypoint->pParent );
		}
	}
    
    return false;
}

void DynamicPathFinder::CalculatePath ( float fSX, float fSY, float fEX, float fEY,    //points to check between
                                         Path* pFinalPath, 								//place to store the final path
										 float fMaxEdgeCost ) 							//max distance without crossing a waypoint
{	
	if ( !pFinalPath ) return;
	ClearDistanceData( );
    
    //nothing hit
    if ( !QuickCheckRay ( fSX, fSY, fEX, fEY ) )
    {
        //pFinalPath->AddPoint( fSX, fSY );
        //pFinalPath->AddPoint( fEX, fEY );
        return;
    }

	if ( !pWaypointList ) return;

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

    pEndPoint->fX = fEX;
    pEndPoint->fY = fEY;
    pEndPoint->fDistG = 0;
    pEndPoint->fDistH = 0;
    pEndPoint->fDistF = 0;
    pEndPoint->iVisited = 0;
	pEndPoint->pEdgeList = 0;
	pEndPoint->iNumEdges = 0;
	pEndPoint->pParent = 0;
    
	//UpdateSingleVisibility( pStartPoint, fMaxEdgeCost, false );
	//pDestPathFinder->UpdateSingleVisibility( pEndPoint, fMaxEdgeCost );

	UpdateSingleVisibility( pStartPoint, -1, false );
	UpdateSingleVisibility( pEndPoint, -1, true );

	AddWaypoint ( pStartPoint );
	AddWaypoint ( pEndPoint );
    
	//start the open list with the start point
    sWaypointOpen_list.clear ( );
    sWaypointInfo sStartPointInfo;
    
    sStartPointInfo.pThisWP = pStartPoint;
    sStartPointInfo.fCost = pStartPoint->fDistF;
    
    sWaypointOpen_list.push_back ( sStartPointInfo );
    
    make_heap ( sWaypointOpen_list.begin ( ), sWaypointOpen_list.end ( ) );

    ShortestPath ( fEX, fEY, pFinalPath );

	sWaypointOpen_list.clear ( );
    
    //remove the start and endpoints from the waypoints, and all edges in between
	RemoveEndWaypoint( pEndPoint );
    
    RemoveLastWaypoint ( );
	RemoveLastWaypoint ( );
}

void DynamicPathFinder::RemoveLastWaypoint ( )
{
	if ( !pWaypointList ) return;

	sWaypoint *pWaypoint = pWaypointList;
	pWaypointList = pWaypointList->pNextWaypoint;

	delete pWaypoint;

	iNumWaypoints--;
}

void DynamicPathFinder::RemoveEndWaypoint( sWaypoint *pWaypoint )
{
	sWaypointEdge *pEdge = pWaypoint->pEdgeList;
    
    while ( pEdge )
    {
        //last edge added to the other waypoint would have been to the end point
		pEdge->pOtherWP->RemoveLastEdge( pWaypoint );
        
		pEdge = pEdge->pNextEdge;
    }
}

void DynamicPathFinder::ClearDistanceData( )
{
	sWaypoint *pWaypoint = pWaypointList;
    
	while ( pWaypoint )
    {
		pWaypoint->iVisited = 0;
        pWaypoint->fDistG = 0;
        pWaypoint->fDistH = 0;
        pWaypoint->fDistF = 0;
		pWaypoint->pParent = 0;
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }
}

void DynamicPathFinder::DebugDrawWaypoints ( float fHeight )
{
    sWaypoint *pWaypoint = pWaypointList;
    
	int iTempMesh = dbMakePointMesh ( );
	if ( iTempMesh == 0 ) return;

	if ( iWaypointObject > 0 && ObjectExist ( iWaypointObject ) )	DeleteObject ( iWaypointObject );

	if ( iWaypointObject == 0 ) iWaypointObject = dbFreeObject ( );
	if ( iWaypointObject == 0 ) return;
	
	//root node
	MakeObjectPlane ( iWaypointObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iWaypointObject, 1 );

	int iLimb = 1;
	while ( pWaypoint )
    {
		AddLimb ( iWaypointObject, iLimb, iTempMesh );
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

void DynamicPathFinder::DebugHideWaypoints ( )
{      
    if ( iWaypointObject > 0 && ObjectExist ( iWaypointObject ) )
	{
		DeleteObject ( iWaypointObject );
	}

	iWaypointObject = 0;
}

void DynamicPathFinder::DebugDrawWaypointEdges ( float fHeight )
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
	
	//root node
	MakeObjectPlane ( iWaypointEdgeObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iWaypointEdgeObject, 1 );

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

			AddLimb ( iWaypointEdgeObject, iLimb, iTempMesh );
			ScaleLimb ( iWaypointEdgeObject, iLimb, 100.0f, 100.0f, fDist * 100.0f );
			RotateLimb ( iWaypointEdgeObject, iLimb, 0.0f, fAngY, 0.0f );
			OffsetLimb ( iWaypointEdgeObject, iLimb, fX, 0.0f, fZ );

			iLimb++;
			pEdge = pEdge->pNextEdge;
        }
        
		pWaypoint = pWaypoint->pNextWaypoint;
    }

	DeleteMesh ( iTempMesh );
	
	dbCombineLimbs ( iWaypointEdgeObject );

	PositionObject ( iWaypointEdgeObject, 0.0f, fHeight, 0.0f );
	ColorObject ( iWaypointEdgeObject, 0xff0000ff );
	SetObjectEmissive ( iWaypointEdgeObject, 0xff0000ff );
	SetObjectEffect ( iWaypointEdgeObject, g_GUIShaderEffectID );

	SetObjectCollisionOff ( iWaypointEdgeObject );
}

void DynamicPathFinder::DebugHideWaypointEdges ( )
{  
	if ( iWaypointEdgeObject > 0 && ObjectExist ( iWaypointEdgeObject ) )
	{
		DeleteObject ( iWaypointEdgeObject );
	}

	iWaypointEdgeObject = 0;
}
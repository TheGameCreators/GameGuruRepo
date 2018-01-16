#include "Path.h"
#include "DBPro Functions.h"

#include <math.h>

// namespace
using namespace std;

extern int g_GUIShaderEffectID;

// prototype LUA function
float GetLUATerrainHeightEx ( float fX, float fZ );

// globals
float Path::fDebugObjHeight = 0.0f;

Path::Path ( )
{
    sPoint_list.clear();
    iSphereRadius = 20;

	iPathObject = 0;

	fPathLength = 0;
}

Path::~Path ( ) 
{ 
    sPoint_list.clear();

	if ( iPathObject > 0 && ObjectExist ( iPathObject ) == 1 )
	{
		DeleteObject ( iPathObject );
	}
}

int Path::CountPoints ( )
{
	return (int) sPoint_list.size ( );
}

void Path::CalculateLength ( )
{
    fPathLength = -1;
    
    if ( sPoint_list.size( ) < 2 ) return;
    
    fPathLength++;
    
    float x = sPoint_list [ 0 ].x;
    float y = sPoint_list [ 0 ].y;
    
    for ( int i = 1; i < (int) sPoint_list.size( ); i++ )
    {
        float x2 = sPoint_list [ i ].x - x;
        float y2 = sPoint_list [ i ].y - y;
        fPathLength += sqrt( x2*x2 + y2*y2 );
        
        x = x + x2;
        y = y + y2;
    }
}

void Path::CalculateSqrLength ( )
{
    fPathLength = -1;
    
    if ( sPoint_list.size( ) < 2 ) return;
    
    fPathLength++;
    
    float x = sPoint_list [ 0 ].x;
    float y = sPoint_list [ 0 ].y;
    
    for ( int i = 1; i < (int) sPoint_list.size( ); i++ )
    {
        float x2 = sPoint_list [ i ].x - x;
        float y2 = sPoint_list [ i ].y - y;
        fPathLength += x2*x2 + y2*y2;
        
        x = x + x2;
        y = y + y2;
    }
}

float Path::GetLength ( ) { return fPathLength; }

//void Path::SetDirection ( int iDir ) { iDirection = iDir; }
//int Path::GetDirection ( ) { return iDirection; }

sPoint Path::GetPoint ( int iIndex )
{
    if ( iIndex < 0 || iIndex >= (int) sPoint_list.size ( ) ) return sPoint();
    
    return sPoint_list [ iIndex ];
}

sPoint Path::GetLast ( )
{
	return sPoint_list.back( );
}

void Path::AddPoint ( float fX, float fY, float fZ )
{
    sPoint sNewPoint;

    sNewPoint.x = fX; 
	sNewPoint.y = fZ;
	sNewPoint.realy = fY;
	sNewPoint.container = -1;
    sPoint_list.push_back ( sNewPoint );
}

void Path::AddPoint ( float fX, float fY, float fZ, int container )
{
    sPoint sNewPoint;
    sNewPoint.x = fX; 
	sNewPoint.y = fZ;
	sNewPoint.realy = fY;
	sNewPoint.container = container;
    sPoint_list.push_back ( sNewPoint );
}

void Path::InsertPoint ( int iIndex, float fX, float fY, int container )
{
    if ( iIndex > (int) sPoint_list.size ( ) ) return;
    
    vector<sPoint>::iterator pItr = sPoint_list.begin ( );
    
    sPoint sNewPoint;
    
    sNewPoint.x = fX; sNewPoint.y = fY; sNewPoint.container = container;
    
    pItr += iIndex;
    sPoint_list.insert ( pItr, sNewPoint );
}

void Path::SwapPoints ( int iIndex1, int iIndex2 )
{
	sPoint tempPoint = sPoint_list [ iIndex1 ];
	sPoint_list [ iIndex1 ] = sPoint_list [ iIndex2 ];
	sPoint_list [ iIndex2 ] = tempPoint;
}

bool Path::Contains ( float fX, float fY )
{
	sPoint sOtherPoint;
	sOtherPoint.x = fX;
	sOtherPoint.x = fY;

	vector < sPoint >::iterator pIter = sPoint_list.begin ( );

	while ( pIter < sPoint_list.end ( ) )
	{
		if ( sOtherPoint == *pIter ) return true;
		pIter++;
	}

	return false;
}

void Path::Clear ( )
{
    sPoint_list.clear ( );
}

void Path::RemoveLast ( )
{
	sPoint_list.pop_back ( );
}

void Path::RemoveFirst ( )
{
	sPoint_list.erase ( sPoint_list.begin ( ) );
}

void Path::DebugDraw ( float radius )
{
	if ( sPoint_list.size ( ) < 1 ) return;
	
	vector < sPoint >::iterator pIter = sPoint_list.begin ( );

	int iPointMesh = dbMakePointMesh ( );
	int iEdgeMesh = dbMakeEdgeMesh ( );
	if ( iPointMesh == 0 ) return;
	if ( iEdgeMesh == 0 ) return;

	if ( iPathObject > 0 && ObjectExist ( iPathObject ) )	DeleteObject ( iPathObject );
	if ( iPathObject == 0 ) iPathObject = dbFreeObject ( );
	if ( iPathObject == 0 ) return;
	
	MakeObjectPlane ( iPathObject, 0.0f, 0.0f, 1 );
	SetObjectMask ( iPathObject, 1 );
	
	// work objects to work out orientations
	int iTempObj1 = dbFreeObject ( );
	if ( iTempObj1 == 0 ) return;
	MakeObjectCube ( iTempObj1, 25.0f );

	int iLimb = 1;
	AddLimb ( iPathObject, iLimb, iPointMesh );
	OffsetLimb ( iPathObject, iLimb, pIter->x, 0.0f, pIter->y );
	iLimb++;

	float fX = pIter->x;
	float fZ = pIter->y;
	pIter++;

	while ( pIter < sPoint_list.end ( ) )
	{
		// dot
		float fDirX = pIter->x - fX;
		float fDirZ = pIter->y - fZ;
		AddLimb ( iPathObject, iLimb, iPointMesh );
		OffsetLimb ( iPathObject, iLimb, pIter->x, GetLUATerrainHeightEx ( pIter->x, pIter->y ), pIter->y );
		iLimb++;

		// edge line
		float fDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );
		if ( fDist < 1.0f ) 
		{
			pIter++;
			continue;
		}
		float fDirX2 = fDirX / fDist;
		float fDirZ2 = fDirZ / fDist;
		float fAngY = acos ( fDirZ2 ) * 57.295779513f;
		if ( fDirX2 < 0.0f ) fAngY = 360.0f - fAngY;

		//AddLimb ( iPathObject, iLimb, iEdgeMesh );
		//ScaleLimb ( iPathObject, iLimb, 40*radius, 100, fDist * 100.0f );
		//RotateLimb ( iPathObject, iLimb, 0.0f, fAngY, 0.0f );
		//OffsetLimb ( iPathObject, iLimb, fX + fDirX / 2.0f, 0.0f, fZ + fDirZ / 2.0f );
		AddLimb ( iPathObject, iLimb, iEdgeMesh );
		float fRealStartY = GetLUATerrainHeightEx ( fX, fZ );
		PositionObject ( iTempObj1, fX, fRealStartY, fZ );
		float fRealFinishY = GetLUATerrainHeightEx ( pIter->x, pIter->y );
		float fDiffX = pIter->x - fX;
		float fDiffY = fRealFinishY - fRealStartY;
		float fDiffZ = pIter->y - fZ;
		float fDiff = sqrt ( fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ );
		ScaleLimb ( iPathObject, iLimb, 40*radius, 100.0f, fDiff * 100.0f );
		PointObject ( iTempObj1, pIter->x, fRealFinishY, pIter->y );
		float fRealMiddleX = pIter->x+((fX-pIter->x)/2.0f);
		float fRealMiddleY = fRealStartY+((fRealFinishY-fRealStartY)/2.0f);
		float fRealMiddleZ = pIter->y+((fZ-pIter->y)/2.0f);
		OffsetLimb ( iPathObject, iLimb, fRealMiddleX, fRealMiddleY + 7.5f, fRealMiddleZ );
		RotateLimb ( iPathObject, iLimb, ObjectAngleX(iTempObj1), ObjectAngleY(iTempObj1), ObjectAngleZ(iTempObj1) );

		// next point/edge
		fX += fDirX;
		fZ += fDirZ;
		iLimb++;
        pIter++;
	}

	DeleteMesh ( iPointMesh );
	DeleteMesh ( iEdgeMesh );

	// delete temp object
	if ( ObjectExist(iTempObj1) == 1 ) DeleteObject ( iTempObj1 );
	
	dbCombineLimbs ( iPathObject );

	//PositionObject ( iPathObject, 0.0f, fDebugObjHeight, 0.0f );
	PositionObject ( iPathObject, 0.0f, 0.0f, 0.0f );//fDebugObjHeight, 0.0f );
	ColorObject ( iPathObject, 0xffff0000 );
	SetObjectEmissive ( iPathObject, 0xffff0000 );
	SetObjectEffect ( iPathObject, g_GUIShaderEffectID );
}

void Path::DebugHide ( )
{
	if ( iPathObject > 0 && ObjectExist ( iPathObject ) )
	{
		DeleteObject ( iPathObject );
	}

	iPathObject = 0;
}

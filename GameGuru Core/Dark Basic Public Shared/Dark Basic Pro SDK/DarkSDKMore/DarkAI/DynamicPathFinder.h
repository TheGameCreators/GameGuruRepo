#ifndef H_DYNPATHFINDER
#define H_DYNPATHFINDER

class Path;
class Container;

//#include "DBPro Functions.h"

#include <vector>
#include <cctype>
#include <math.h>

class DynamicPathFinder;
class PathFinderAdvanced;

class AvoidanceObject
{
public:

	AvoidanceObject( );
	virtual ~AvoidanceObject( );

	virtual bool Intersects( float fSX, float fSZ, float fEX, float fEZ ) const = 0;
	virtual bool Inside( float x, float z, AvoidanceObject **pInObject ) = 0;

	virtual void GetCenter( float &x, float &z ) const = 0;
	virtual void GetMin( float &x, float &z ) const = 0;
	virtual void GetMax( float &x, float &z ) const = 0;

	virtual int CreateWaypoints( DynamicPathFinder *pPathFinder ) = 0;

	AvoidanceObject *pNextObject;
	AvoidanceObject *pNextLocalObject;

	int id;
};

class AvoidanceSphere : public AvoidanceObject
{
public:

	AvoidanceSphere( float radius = 0, float x = 0, float z = 0 );
	~AvoidanceSphere( );

	bool Intersects( float fSX, float fSZ, float fEX, float fEZ ) const;
	bool Inside( float x, float z, AvoidanceObject **pInObject );
	void GetCenter( float &x, float &z ) const;
	void GetMin( float &x, float &z ) const;
	void GetMax( float &x, float &z ) const;

	int CreateWaypoints( DynamicPathFinder *pPathFinder );

private:

	float fX, fZ;
	float fRadius;
};

class DynamicCollisionNode
{
public:

	DynamicCollisionNode *left, *right;
	float fMinX, fMinZ;
	float fMaxX, fMaxZ;

	DynamicCollisionNode( int iNumObjects, int iObjectsPerNode, AvoidanceObject *pObjectList );
	~DynamicCollisionNode( );

	bool Intersects( float fSX, float fSY, float fEX, float fEY );
	bool Inside( float x, float z, AvoidanceObject **pInObject );

	void Reset( );
	void ReBuildTree( int iNumObjects, int iObjectsPerNode, AvoidanceObject *pObjectList );
};

//contains all processes relating to working out the path
class DynamicPathFinder
{

private:
    
    AvoidanceObject *pObjectList;
	DynamicCollisionNode *pOptimizedCol;

	PathFinderAdvanced *pMainObstacles;

public:

    struct sWaypoint;

    struct sWaypointEdge
    {
        sWaypoint *pOtherWP;
        float fCost;
		//float fAngY;

		sWaypointEdge *pNextEdge;
    };

    struct sWaypoint
    {
        float fX;
        float fY;
        float fDistF;
        float fDistG;
        float fDistH;
		bool bActive;
		int iNumEdges;

		AvoidanceObject *pObjectOwner;

		int iVisited;
        sWaypoint *pParent;
        
        sWaypointEdge *pEdgeList;
		sWaypoint *pNextWaypoint;

		void AddEdge ( sWaypoint *pWaypoint, float fCost );
		void RemoveLastEdge ( sWaypoint *pTargetWP );
		void RemoveEdge ( sWaypoint *pWaypoint );
		void ClearEdges ( );
		
		sWaypoint ( ) { pEdgeList = 0; iNumEdges = 0; pObjectOwner = 0; }
		~sWaypoint ( );

		bool operator ==(sWaypoint w) const {
			return ( fabs( fX - w.fX ) < 0.000001 ) && ( fabs( fY - w.fY ) < 0.000001 );
		}
    };
    
private:

    struct sWaypointInfo
    {
        sWaypoint *pThisWP;
        
        float fCost;
        
        bool operator<(sWaypointInfo w) const {
            return fCost > w.fCost;
        }
    };
    
    sWaypoint *pWaypointList;
    std::vector<sWaypointInfo> sWaypointOpen_list;

	int iNumWaypoints;

	int iWaypointObject;
	int iWaypointEdgeObject;

	void AddWaypoint ( sWaypoint *pWaypoint );
	void RemoveLastWaypoint ( );
  
public:  
    
    DynamicPathFinder ( PathFinderAdvanced *pMain );
    ~DynamicPathFinder ( );

	void SetMainPathFinder( PathFinderAdvanced *pMain );

	DynamicPathFinder::sWaypoint* AddWaypoint ( float x, float z, AvoidanceObject *pOwner );
	void RemoveWaypoint ( int iIndex );
	void UpdateVisibility ( float fLimit );
	void UpdateSingleVisibility( DynamicPathFinder::sWaypoint *pNewWP, float fLimit, bool duplex );
	void ClearWaypoints ( );

	void RemoveAllEdges ( );
	
	sWaypoint* GetClosestWaypoint ( float fSX, float fSY );
	
    void AddObject ( AvoidanceObject *pObject, bool bUpdate );
	void RemoveObject ( int id, bool bUpdate );
	void CompleteObstacles( );
	    
    void ClearObjects ( );
    
    void BuildWaypoints( );
    
    //simply returns true if something hit, false otherwise
	bool QuickCheckRay ( float fSX, float fSY, float fEX, float fEY );

	bool CheckValidPoint( float fX, float fY );

	//returns the number of polygons the point is inside
	int InObject ( float fSX, float fSY, AvoidanceObject **pObject );
	bool GetClosestPoint( const AvoidanceObject *pObject, float x, float z, float &closestX, float &closestZ );

	//uses A* on the waypoints to find the shortest path
    bool ShortestPath ( float fEX, float fEY, Path* pBuildPath );
    
    //The main function. Polygon list must have been passed sometime before  
    //paths pFinalPath must be defined, assumed clear.
    //if either path is not found, it will be cleared.
    void CalculatePath ( float fSX, float fSY, float fEX, float fEY,    //points to check between
                         Path* pFinalPath,								//place to store the final path
						 float fMaxEdgeCost );

	void RemoveEndWaypoint( sWaypoint *pWaypoint );

	void ClearDistanceData( );
	
	void DebugDrawWaypoints ( float fHeight );
	void DebugHideWaypoints ( );

	void DebugDrawWaypointEdges ( float fHeight );
	void DebugHideWaypointEdges ( );
};

#endif

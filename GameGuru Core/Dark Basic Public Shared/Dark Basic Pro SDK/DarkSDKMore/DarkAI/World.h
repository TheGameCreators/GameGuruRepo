#ifndef H_WORLD
#define H_WORLD

#include <vector>
#include <cctype>

using namespace std;

//#include "Team.h"

struct sObject;
class Entity;
class Hero;
class StateSet;
class Beacon;
class TeamController;
class Zone;
class Container;
class Path;

class Polygon2D;
class PathFinderAdvanced;

class CollisionObject;

struct tempCoverPoint
{
	float fx;
	float fy;
	float fz;
	float angle;
	char pCoverName[64];
};

class World
{
private:

	__int64 i64TimeFreq;
	__int64 i64CurrentTime;
	__int64 i64LastTime;
	__int64 i64LastDebugTime;

	float fTimeDelta;
	bool bTimeSkip;

	int						iMaxPath;
	Path					**ppPathArray;

	float fRadius;

	CollisionObject *pVisibilityData;

public:

	void UpdateWorld ( );
	void StopWorld ( );

	//vector < Polygon2D >	cPolygon_list;

	Polygon2D				*pManualPolygon;
	
	TeamController			*pTeamController;
	//PathFinderAdvanced		*pPathFinder;
	StateSet				*pEntityStates;

	Beacon					*pBeaconList;
	Zone					*pZoneList;
	Container				*pContainerList;

	HANDLE					hDebugConsoleOut;
	int						iDebugEntityNum;
	static bool				bShowBeacons;
	
	World ( );
	~World ( );
	
	void Reset ( );
	void SetRadius ( float fNewRadius );
	void SetAvoidanceRadius ( float fNewRadius );

	void SetConsoleOn ( int iObjID );
	void ChangeConsoleObject( int iObjID );
	void SetConsoleOff ( );

	int RandInt ( float fAvgDiff );

	//void SetTerrain ( int id, sObject *pObject );
	void AddStatic ( int id, int iContainerID, sObject *pObject, int iHeight, int iObstacleType );
	void RemoveStatic ( int id );

	void AddDoor ( int id, int iContainerID, float x1, float y1, float x2, float y2 );
	void RemoveDoor ( int id, int iContainerID );

	void StartNewPolygon ( int id );
	void AddVertex ( float x, float z );
	void EndNewPolygon ( int iContainerID, int iHeight, int iObstacleType );
	void EndNewPolygon ( int iContainerID, int iHeight, int iObstacleType, bool bUpdate );
	void DiscardNewPolygon ( );

	void AddObstacleFromObject ( sObject *pObject, int iContainerID, int iHeight, float fPlaneHeight, float fMinLength, float fMinHeight, bool bOutputToFile );
	void CompleteObstacles ( int iContainerID );

	bool UsingGlobalVisibility( );
	void AddAlternateVisibilityObject( int id, sObject *pObject, int type );
	void RemoveAlternateVisibilityObject( int id );
	bool GlobalVisibilityCheck( float x, float y, float z, float x2, float y2, float z2, float *dist );
	void SetupAllPathFinders( );

	void AddContainer ( int iContainerID );
	void RemoveContainer ( int iContainerID );
	Container* GetContainer ( int iContainerID );
	Container* GetContainerList ( void ) { return pContainerList; }
	bool ContainerExist ( Container *pFindContainer );
	int ConnectContainers( int iFromContainer, float x, float z, int iToContainer, float x2, float z2 );

	void AddHero ( int id, int iContainerID, sObject *pObject, int iTeam );
	void DeleteHero ( int id );
	void DeletePlayer( );
	
	void AddEntity ( int id, int iContainerID, sObject *pObject, sObject *pObjectRef, int iTeam );
	void DeleteEntity ( int id );

	void ClearCoverPoints(); // remove all cover pointers from entities, use then when recalculating cover points so there are no floating pointers

	void AddZone ( int iID, float minx, float minz, float maxx, float maxz, Container *pContainer );
	Zone* GetZone ( int iID );
	void DeleteZone ( int iID );

	int AddPath ( int iPathID );
	int DeletePath ( int iPathID );
	int AddPathPoint ( int iPathID, float x, float y, float z, int container = -1 );
	float GetPathPointX ( int iPathID, int iIndex );
	float GetPathPointY ( int iPathID, int iIndex );
	float GetPathPointZ ( int iPathID, int iIndex );
	int CountPathPoints ( int iPathID );
	int MakePathFromMemblock ( int iPathID, int iMemblockID );
	int MakeMemblockFromPath ( int iMemblockID, int iPathID );
	int AddObstaclePath ( int iPathID, int iContainerID, float x, float z, float x2, float z2, float fMaxEdgeCost, int destContainer = -1 );
	Path* GetPath ( int iPathID );
	int GetMaxPath ( );
	int AssignEntityToPatrolPath ( Entity *pEntity, int iPathID );

	//Dave
	int GetTotalPaths();
	
	void AddBeacon ( Beacon* pNewBeacon );
	void CleanUpWorldBeacons ( float fTimeDelta );
	
	float GetTimeDelta ( );

	Hero* GetHero ( int id );
	Entity* GetEntity ( int id );

	Hero* GetHeroCopy ( int id );
	Entity* GetEntityCopy ( int id );

	void Update ( );
	void Update ( float fUserTime );
	
	void DebugShowBeacons ( );
	void DebugHideBeacons ( );
	void DebugOutput ( );
};

#endif

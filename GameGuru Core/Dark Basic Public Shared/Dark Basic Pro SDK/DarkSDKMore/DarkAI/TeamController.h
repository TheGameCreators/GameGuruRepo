#ifndef H_TEAM_CONTROLLER
#define H_TEAM_CONTROLLER

#include "Team.h"

class Hero;
class Entity;
class Beacon;
class Zone;
class EntityThread;

class TeamController
{
private:

	//Team pTeamArray [ 32 ];
	Team cEnemyTeam;
	Team cHeroTeam;
	Team cNeutralTeam;

	Entity* pEntityIterator;
	Hero* pHeroIterator;
	Team* pTeamIterator;

	int iNumThreads;
	EntityThread *pEntityThreads;

public:

	TeamController ( );
	~TeamController ( );

	void Reset ( );
	void SetThreadNumber( int iNumThreads );

	bool Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide );
	void SetRadius ( float fNewRadius );
	void SetTeamStance ( int iTeam, int iStance, float fDist );

	//iTeam = 0 is neutral, 1 = hero, 2 = enemy
	void AddHero ( Hero* pNewHero, int iTeam );
	void StartHeroIterator ( Team *pAskingTeam );
	Hero* GetNextHero ( );
	Hero* GetHero ( int iObjID );
	Hero* GetHeroCopy ( int iObjID );
	void RemoveHero ( Hero *pDelHero );
	void DeleteHero ( int iObjID );
	Hero* GetPlayer ( );
	void DeletePlayer( );

	void AddEntity ( Entity* pNewHero, int iTeam );
	void StartEntityIterator ( Team *pAskingTeam );
	void StartGlobalEntityIterator ( );
	Entity* GetNextEntity ( );
	Entity* GetNextGlobalEntity ( );
	Entity* GetEntity ( int iObjID );
	Entity* GetEntityCopy ( int iObjID );
	void RemoveEntity ( Entity *pDelEntity );
	void DeleteEntity ( int iObjID );
	Entity* GetFirstFriendly( );

	void ClearCoverPoints();
	
	void UpdateTimers ( float fTimeDelta );
	void CheckZones ( Zone *pWorldZoneList );
	void CheckBeacons ( Beacon *pWorldBeaconList );
	void UpdateState ( );
	void UpdateMovement ( float fTimeDelta );

	void Update ( float fTimeDelta, Beacon *pWorldBeaconList, Zone *pWorldZoneList );
	
	void CleanUpBeacons ( float fTimeDelta );

	void DebugOutputEntityData ( HANDLE hOut, int iObjID );
	void DebugShowBeacons ( int iTempMesh, int iCount );

};

#endif
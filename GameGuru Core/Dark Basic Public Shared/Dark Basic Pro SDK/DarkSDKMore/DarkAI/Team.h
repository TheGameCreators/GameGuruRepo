#ifndef H_TEAM
#define H_TEAM

#include "windows.h"

class Hero;
class Entity;
class Beacon;
class Zone;
class EntityBuffer;

class Team
{
/*
	class TeamMember
	{
		Entity *pEntity;
		TeamMember *pNextTeamMember;
	};

	TeamMember *pMemberList;					//linked list
	*/

private:

	Hero* pHeroList;							//human team members
	Entity *pEntityList;						//team members
	Beacon *pBeaconList;						//team messages

	//Hero* pHeroListCopy;							//human team members
	//Entity *pEntityListCopy;						//team members
	//Beacon *pBeaconListCopy;						//team messages

	bool bNeutral;								//a neutral team
	
	int iTeamNum;
	DWORD dwTeamIndex;
	//DWORD dwAllied;								//bit flags 1=allied, 0=enemy (32 teams total, referenced by index)

	bool bFollowLeader;
	float fFollowDist;

public:

	Team ( );
	Team ( int iNum );
	~Team ( );

	void Clear ( );

	bool Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide );
	
	void SetTeamNum ( int iNum );				//use values 0-31, converted into dword index
	DWORD GetTeamIndex ( );
	int GetTeamNum ( );

	void SetFollowHero ( float fDist );
	void SetFendAlone ( );

	void SetAllied ( Team *pOtherTeam );
	void SetEnemy ( Team *pOtherTeam );
	void BreakTeam ( );

	void SetNeutral ( bool bFlag );
	bool IsNeutral ( );

	void AddHero ( Hero* pHero );
	Hero* GetHero ( int id );
	Hero* GetHeroCopy ( int id );
	Hero* GetFirstHero ( );
	void RemoveHero ( Hero *pDelHero );

	void AddMember ( Entity* pEntity );
	Entity* GetMember ( int id );
	Entity* GetMemberCopy ( int id );
	Entity* GetFirstMember ( );
	void RemoveMember ( Entity *pDelEntity );
	void AddMembersToBuffer( EntityBuffer *pBuffer );

	void ClearCoverPoints();

	Beacon* GetFirstBeacon ( );
	void CleanUpBeacons ( float fTimeDelta );

	bool IsAllied ( Team *pOtherTeam );
	bool IsEnemy ( Team *pOtherTeam );
	
	void UpdateTimers ( float fTimeDelta);
	void CheckZones ( Zone *pWorldZoneList );
	void CheckBeacons ( Beacon *pWorldBeaconList );
	void UpdateMemberMovement ( float fTimeDelta );
	void UpdateMemberState ( );
	void UpdatePlayerMovement( float fTimeDelta );

	void DebugOutputEntityData ( HANDLE hOut, int iObjID );
	int DebugShowBeacons ( int iTempMesh, int iCount );
};

#endif
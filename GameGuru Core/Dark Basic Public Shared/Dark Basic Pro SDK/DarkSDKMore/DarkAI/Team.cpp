
#include "Team.h"
#include "Hero.h"
#include "Entity.h"
#include "Beacon.h"
#include "Zone.h"
#include "Container.h"
#include "EntityThread.h"
#include "EntityBuffer.h"
#include "DarkAI.h"
#include "LeeThread.h"

extern FILE *pDebugFile;
extern LeeThread g_LeeThread;

Team::Team ( )
{
	pHeroList = 0;
	pEntityList = 0;
	pBeaconList = 0;

	//dwAllied = 0;
	bNeutral = false;
	dwTeamIndex = 0;

	bFollowLeader = false;
	fFollowDist = 15.0f;
}

Team::Team ( int iNum )
{
	pHeroList = 0;
	pEntityList = 0;
	pBeaconList = 0;

	//dwAllied = 0;
	bNeutral = false;
	bFollowLeader = false;
	fFollowDist = 15.0f;

	SetTeamNum ( iNum );
}

Team::~Team ( )
{
	Clear ( );
}

void Team::Clear ( )
{
	while ( pHeroList )
	{
		Hero *pHero = pHeroList;
		pHeroList = pHeroList->pNextHero;
		delete pHero;
	}

	while ( pEntityList )
	{
		Entity *pEntity = pEntityList;
		pEntityList = pEntityList->pNextEntity;
		delete pEntity;
	}

	while ( pBeaconList )
	{
		Beacon *pBeacon = pBeaconList;
		pBeaconList = pBeaconList->pNextBeacon;
		delete pBeacon;
	}

	//dwAllied = 0;
	SetAllied ( this );
}

bool Team::Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide )
{
	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->Intersect ( fSX, fSZ, fEX, fEZ, iSide ) ) return true;

		pEntity = pEntity->pNextEntity;
	}

	Hero *pHero = pHeroList;

	while ( pHero )
	{
		if ( pHero->Intersect ( fSX, fSZ, fEX, fEZ, iSide ) ) return true;

		pHero = pHero->pNextHero;
	}

	return false;
}

void Team::SetTeamNum ( int iNum )
{
	if ( iNum < 0 ) iNum = 0;
	if ( iNum > 31 ) iNum = 31;
	
	iTeamNum = iNum;
	dwTeamIndex = 1 << iNum;

	SetAllied ( this );
}

DWORD Team::GetTeamIndex ( ) { return dwTeamIndex; }

int Team::GetTeamNum ( ) { return iTeamNum; }

void Team::SetFollowHero ( float fDist )
{
	if ( !pEntityList ) return;

	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		pEntity->SetDefendHero ( fDist );
		pEntity = pEntity->pNextEntity;
	}

	bFollowLeader = true;
	fFollowDist = fDist;
}

void Team::SetFendAlone ( )
{
	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		pEntity->SetAggressiveness( 1 );
		pEntity->SetFollowing ( false );
		pEntity = pEntity->pNextEntity;
	}

	bFollowLeader = false;
}

void Team::SetAllied ( Team *pOtherTeam )
{
	if ( !pOtherTeam ) return;

	//dwAllied = dwAllied | pOtherTeam->GetTeamIndex ( );
}

void Team::SetEnemy ( Team *pOtherTeam )
{
	if ( !pOtherTeam ) return;

	//dwAllied = dwAllied & ( ~pOtherTeam->GetTeamIndex ( ) );
}

void Team::BreakTeam ( )
{
	SetEnemy ( this );
}

void Team::SetNeutral ( bool bFlag )
{
	bNeutral = bFlag;

	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		pEntity->SetTeam ( 0 );

		pEntity->pNextEntity;
	}

	Hero *pHero = pHeroList;

	while ( pHero )
	{
		pHero->SetTeam ( 0 );

		pHero->pNextHero;
	}
}

bool Team::IsNeutral ( )
{
	return bNeutral;
}

void Team::AddHero ( Hero *pHero )
{
	if ( !pHero ) return;
	
	if ( pHeroList ) delete pHeroList;

	//pHero->pNextHero = pHeroList;
	pHeroList = pHero;

	if ( !bNeutral ) pHero->SetTeam ( this );
}

Hero* Team::GetHero ( int id )
{
	Hero* pHero = pHeroList;

	while ( pHero )
	{
		if ( pHero->GetID ( ) == id ) return pHero;
		pHero = pHero->pNextHero;
	}

	return 0;
}

Hero* Team::GetHeroCopy ( int id )
{
	Hero* pHero = pHeroList;

	while ( pHero )
	{
		if ( pHero->GetID ( ) == id ) return pHero;
		pHero = pHero->pNextHero;
	}

	return 0;
}

Hero* Team::GetFirstHero ( ) { return pHeroList; }

void Team::RemoveHero ( Hero *pDelHero )
{
	if ( !pHeroList ) return;
	
	while ( pHeroList == pDelHero )
	{
		pHeroList = pHeroList->pNextHero;
	}

	if ( !pHeroList ) return;
	
	Hero *pLastHero = pHeroList;
	Hero *pHero = pHeroList->pNextHero;

	while ( pHero )
	{
		if ( pHero == pDelHero )
		{
			pLastHero->pNextHero = pHero->pNextHero;
		}
	
		pLastHero = pHero;
		pHero = pHero->pNextHero;
	}
}

void Team::AddMember ( Entity *pEntity )
{
	if ( !pEntity ) return;
	pEntity->pNextEntity = pEntityList;
	pEntityList = pEntity;

	if ( bFollowLeader )
	{
		pEntity->SetDefendHero ( fFollowDist );
	}

	if ( !bNeutral ) pEntity->SetTeam ( this );
}

Entity* Team::GetMember ( int id )
{
	Entity* pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetID ( ) == id ) return pEntity;
		pEntity = pEntity->pNextEntity;
	}

	return 0;
}

Entity* Team::GetMemberCopy ( int id )
{
	Entity* pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetID ( ) == id ) return pEntity;
		pEntity = pEntity->pNextEntity;
	}

	return 0;
}

void Team::ClearCoverPoints()
{
	Entity* pEntity = pEntityList;

	while ( pEntity )
	{
		pEntity->SetCoverPoint( 0 );
		pEntity = pEntity->pNextEntity;
	}
}

Entity* Team::GetFirstMember ( ) { return pEntityList; }

void Team::RemoveMember ( Entity* pDelEntity )
{
	if ( !pEntityList ) return;
	
	if ( pEntityList == pDelEntity )
	{
		pEntityList = pEntityList->pNextEntity;
	}
	
	if ( !pEntityList ) return;

	Entity *pLastEntity = pEntityList;
	Entity *pEntity = pEntityList->pNextEntity;

	while ( pEntity )
	{
		if ( pEntity == pDelEntity )
		{
			pLastEntity->pNextEntity = pEntity->pNextEntity;
		}
		
		pLastEntity = pEntity;
		pEntity = pEntity->pNextEntity;
	}
}

void Team::AddMembersToBuffer( EntityBuffer *pBuffer )
{
	Entity* pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetActive( ) ) pBuffer->AddEntity( pEntity );

		pEntity = pEntity->pNextEntity;
	}
}

Beacon* Team::GetFirstBeacon ( ) { return pBeaconList; }

void Team::CleanUpBeacons ( float fTimeDelta )
{
	Beacon *pBeacon = pBeaconList;
	Beacon *pLastBeacon = 0;

	while ( pBeacon )
	{
		bool bDelete = pBeacon->IsOld ( fTimeDelta );
		
		if ( bDelete )
		{
			Beacon *pTemp = pBeacon;

			if ( !pLastBeacon )  pBeaconList = pBeacon->pNextBeacon;
			else	pLastBeacon->pNextBeacon = pBeacon->pNextBeacon;

			pBeacon = pBeacon->pNextBeacon;

			delete pTemp;
		}
		else
		{
			pLastBeacon = pBeacon;
			pBeacon = pBeacon->pNextBeacon;
		}
	}
}

bool Team::IsAllied ( Team *pOtherTeam )
{
	//if ( (dwAllied & pOtherTeam->GetTeamIndex ( )) > 0 ) return true;

	return false;
}

bool Team::IsEnemy ( Team *pOtherTeam )
{
	//if ( (dwAllied & pOtherTeam->GetTeamIndex ( )) > 0 ) return false;

	return true;
}

void Team::UpdateTimers ( float fTimeDelta )
{
	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetActive( ) ) pEntity->UpdateTimers ( fTimeDelta );
		pEntity = pEntity->pNextEntity;
	}
}

void Team::CheckZones ( Zone *pWorldZoneList )
{
	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetActive( ) )
		{
			pEntity->CheckZones ( pWorldZoneList );
		}

		pEntity = pEntity->pNextEntity;
	}

	Hero *pHero = pHeroList;

	while ( pHero )
	{
		pHero->CheckZones ( pWorldZoneList );
		
		pHero = pHero->pNextHero;
	}
}

void Team::CheckBeacons ( Beacon *pWorldBeaconList )
{
	if ( !pEntityList ) return;

	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		pEntity->ClearSounds ( );

		if ( pEntity->GetActive( ) )
		{
			pEntity->CheckBeacons ( pBeaconList );
			pEntity->CheckBeacons ( pWorldBeaconList );

			if ( pEntity->GetFollowing ( ) )
			{
				if ( pHeroList )
				{
					pEntity->SetDefendPos( pHeroList->GetX ( ), pHeroList->GetY( ), pHeroList->GetZ( ), ENT_X|ENT_Y|ENT_Z );
					pEntity->SetDefendContainer( pHeroList->GetContainer()->GetID( ) );
				}
				else
				{
					pEntity->SetFollowing ( false );
				}
			}
		}

		pEntity = pEntity->pNextEntity;
	}
}

void Team::UpdateMemberMovement ( float fTimeDelta )
{
	Entity *pEntity = pEntityList;
	float fPlayerX = AIGetPlayerX();
	float fPlayerZ = AIGetPlayerZ();
	while ( pEntity )
	{
		if ( pEntity->GetActive( ) )
		{
			// based on distance from player, determine if movement to be updated or removed from consideration
			float fDX = pEntity->GetX() - fPlayerX;
			float fDZ = pEntity->GetZ() - fPlayerZ;
			float fPlrDist = fabs(fDX*fDX)+fabs(fDZ*fDZ); 
			if ( fPlrDist < 3000.0f*3000.0f || pEntity->bAlwaysActive==true  || g_LeeThread.GetWorkInProgress() == pEntity->GetID() )
			{
				// update entity movement logic
				pEntity->UpdateMovement ( fTimeDelta );
			}
			else
			{
				// 140917 - remove entity from movement consideration as too far away)
				pEntity->RemoveFromMovementList();
			}
		}
		pEntity = pEntity->pNextEntity;
	}

	//Dave - commenting this out as it gets called 3 times (once for each team) and i dont think we need it
	/*Hero *pHero = pHeroList;
	while ( pHero )
	{
		//QueryPerformanceCounter ( (LARGE_INTEGER*) &i64Time );
		pHero->Update ( fTimeDelta );
		//QueryPerformanceCounter ( (LARGE_INTEGER*) &i64Time2 );
		pHero = pHero->pNextHero;
	}*/
}

void Team::UpdatePlayerMovement( float fTimeDelta )
{
	Hero *pHero = pHeroList;

	while ( pHero )
	{
		pHero->Update ( fTimeDelta );
		
		pHero = pHero->pNextHero;
	}
}

void Team::UpdateMemberState ( )
{
	if ( !pEntityList ) return;

	Entity *pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetActive( ) )
		{
			pEntity->UpdateState ( );
		}

		pEntity = pEntity->pNextEntity;
	}
}

void Team::DebugOutputEntityData ( HANDLE hOut, int iObjID )
{
	if ( !hOut ) return;

	if ( !pEntityList && !pHeroList ) return;

	Entity *pEntity = pEntityList;
	bool bFound = false;

	while ( pEntity )
	{
		if ( pEntity->GetID ( ) == iObjID ) bFound = true;

		pEntity = pEntity->pNextEntity;
	}

	if ( !bFound ) return;

	DWORD dwWritten = 0;
	DWORD dwWrite = 0;
	char strOut [ 256 ];
	
	dwWrite = sprintf_s ( strOut, 255, "\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, "Team Number: %d\n", iTeamNum );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, "Members:\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	
	if ( !pEntityList ) return;

	pEntity = pEntityList;

	while ( pEntity )
	{
		if ( pEntity->GetID ( ) == iObjID ) pEntity->DebugOutputInternalData ( hOut );

		pEntity = pEntity->pNextEntity;
	}

	dwWrite = sprintf_s ( strOut, 255, "\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
}

int Team::DebugShowBeacons ( int iTempMesh, int iCount )
{
	Beacon *pBeacon = pBeaconList;
	
	while ( pBeacon )
	{
		pBeacon->DebugAddLimb ( iCount, iTempMesh );

		iCount++;
		pBeacon = pBeacon->pNextBeacon;
	}

	return iCount;
}
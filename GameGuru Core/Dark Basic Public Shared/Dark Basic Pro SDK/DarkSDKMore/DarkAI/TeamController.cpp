#include "TeamController.h"
#include "Hero.h"
#include "Entity.h"
#include "Beacon.h"
#include "Zone.h"
#include "EntityThread.h"
#include "EntityBuffer.h"
#include "World.h"
#include <vector>

extern int iThreadMode;
//extern int iThreadCount;
extern FILE *pDebugFile;

//Dave
//extern bool NotDonePathThisFrame;
//extern int CurrentAIWorkedOn;
//extern float CurrentAIWorkedPathTimer;
extern std::vector <int> ListOfEntitiesToUpdateMovement;
//extern int ListOfEntitiesToUpdateMovementIndex;
extern World cWorld;
int okayForPath = 0;

TeamController::TeamController ( )
{
	iNumThreads = 1;
	pEntityThreads = NULL;

	/*
	for ( int i = 0; i < iNumThreads; i++ )
	{
		pEntityThreads [ i ].Start( );
	}
	*/

	Reset ( );
}

TeamController::~TeamController ( )
{
	Reset ( );

	if ( pEntityThreads ) delete [] pEntityThreads;
}

void TeamController::Reset ( )
{
	cEnemyTeam.Clear ( );
	cHeroTeam.Clear ( );
	cNeutralTeam.Clear ( );

	cNeutralTeam.SetTeamNum ( 0 );
	cHeroTeam.SetTeamNum ( 1 );
	cEnemyTeam.SetTeamNum ( 2 );

	cNeutralTeam.SetNeutral ( true );
}

bool TeamController::Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide )
{
	bool bHit = cEnemyTeam.Intersect ( fSX, fSZ, fEX, fEZ, iSide );
	if ( !bHit ) bHit = cHeroTeam.Intersect ( fSX, fSZ, fEX, fEZ, iSide );
	if ( !bHit ) bHit = cNeutralTeam.Intersect ( fSX, fSZ, fEX, fEZ, iSide );

	return bHit;
}

void TeamController::SetRadius ( float fNewRadius )
{
	Entity* pEntity = cEnemyTeam.GetFirstMember ( );

	while ( pEntity )
	{
		pEntity->SetRadius ( fNewRadius );
		pEntity = pEntity->pNextEntity;
	}
	
	pEntity = cHeroTeam.GetFirstMember ( );

	while ( pEntity )
	{
		pEntity->SetRadius ( fNewRadius );
		pEntity = pEntity->pNextEntity;
	}

	pEntity = cNeutralTeam.GetFirstMember ( );

	while ( pEntity )
	{
		pEntity->SetRadius ( fNewRadius );
		pEntity = pEntity->pNextEntity;
	}

	Hero* pHero = cEnemyTeam.GetFirstHero ( );

	while ( pHero )
	{
		pHero->SetRadius ( fNewRadius );
		pHero = pHero->pNextHero;
	}
	
	pHero = cHeroTeam.GetFirstHero ( );

	while ( pHero )
	{
		pHero->SetRadius ( fNewRadius );
		pHero = pHero->pNextHero;
	}

	pHero = cNeutralTeam.GetFirstHero ( );

	while ( pHero )
	{
		pHero->SetRadius ( fNewRadius );
		pHero = pHero->pNextHero;
	}
}

void TeamController::SetTeamStance ( int iTeam, int iStance, float fDist )
{
	if ( iTeam == 1 )
	{
		switch ( iStance )
		{
			case 0: cHeroTeam.SetFollowHero ( fDist ); break;
			case 1: cHeroTeam.SetFendAlone ( ); break;
			default : cHeroTeam.SetFendAlone ( );
		}
	}
}

void TeamController::AddHero ( Hero* pNewHero, int iTeam )
{
	switch ( iTeam )
	{
		case 0: cNeutralTeam.AddHero ( pNewHero ); break;
		case 1: cHeroTeam.AddHero ( pNewHero ); break;
		case 2: cEnemyTeam.AddHero ( pNewHero ); break;

		default: cNeutralTeam.AddHero ( pNewHero ); break;
	}
}

void TeamController::StartHeroIterator ( Team *pAskingTeam )
{
	if ( pAskingTeam == &cHeroTeam ) pTeamIterator = &cEnemyTeam;
	else if ( pAskingTeam == &cEnemyTeam ) pTeamIterator = &cHeroTeam;
	else { pTeamIterator = 0; return; }

	pHeroIterator = pTeamIterator->GetFirstHero ( );
}

Hero* TeamController::GetNextHero ( )
{
	if ( !pTeamIterator ) return 0;
	if ( !pHeroIterator ) return 0;

	Hero *pHero = pHeroIterator;
	pHeroIterator = pHeroIterator->pNextHero;

	return pHero;
}

Hero* TeamController::GetHero ( int iObjID )
{
	Hero *pHero = cHeroTeam.GetHero ( iObjID );
	if ( !pHero ) pHero = cEnemyTeam.GetHero ( iObjID );
	if ( !pHero ) pHero = cNeutralTeam.GetHero ( iObjID );

	return pHero;
}

Hero* TeamController::GetHeroCopy ( int iObjID )
{
	Hero *pHero = cHeroTeam.GetHeroCopy ( iObjID );
	if ( !pHero ) pHero = cEnemyTeam.GetHeroCopy ( iObjID );
	if ( !pHero ) pHero = cNeutralTeam.GetHeroCopy ( iObjID );

	return pHero;
}

void TeamController::RemoveHero ( Hero *pDelHero )
{
	cHeroTeam.RemoveHero ( pDelHero );
	cEnemyTeam.RemoveHero ( pDelHero );
	cNeutralTeam.RemoveHero ( pDelHero );
}

void TeamController::DeleteHero ( int iObjID )
{
	Hero *pHero = GetHero ( iObjID );
	if ( !pHero ) return;

	RemoveHero ( pHero );
	delete pHero;
}

Hero* TeamController::GetPlayer ( )
{
	return cHeroTeam.GetFirstHero ( );
}

void TeamController::DeletePlayer( )
{
	Hero *pHero = GetPlayer( );
	if ( !pHero ) return;

	RemoveHero( pHero );
	delete pHero;
}

void TeamController::AddEntity ( Entity* pNewEntity, int iTeam )
{
	switch ( iTeam )
	{
		case 0: cNeutralTeam.AddMember ( pNewEntity ); break;
		case 1: cHeroTeam.AddMember ( pNewEntity ); break;
		case 2: cEnemyTeam.AddMember ( pNewEntity ); break;

		default: cNeutralTeam.AddMember ( pNewEntity ); break;
	}

	if ( iTeam == 0 ) pNewEntity->SetNeutral ( true );
}

void TeamController::StartEntityIterator ( Team *pAskingTeam )
{
	if ( pAskingTeam == &cHeroTeam ) pTeamIterator = &cEnemyTeam;
	else if ( pAskingTeam == &cEnemyTeam ) pTeamIterator = &cHeroTeam;
	else { pTeamIterator = 0; return; }

	pEntityIterator = pTeamIterator->GetFirstMember ( );
}

void TeamController::StartGlobalEntityIterator ( )
{
	pTeamIterator = &cEnemyTeam;
	pEntityIterator = pTeamIterator->GetFirstMember ( );

	if ( !pEntityIterator )
	{
		pTeamIterator = &cHeroTeam;
		pEntityIterator = pTeamIterator->GetFirstMember( );

		if ( !pEntityIterator )
		{
			pTeamIterator = &cNeutralTeam;
			pEntityIterator = pTeamIterator->GetFirstMember( );
		}
	}
}

Entity* TeamController::GetNextEntity ( )
{
	if ( !pTeamIterator ) return 0;
	if ( !pEntityIterator ) return 0;

	Entity *pEntity = pEntityIterator;
	pEntityIterator = pEntityIterator->pNextEntity;

	return pEntity;
}

Entity* TeamController::GetNextGlobalEntity ( )
{
	if ( !pTeamIterator ) return 0;
	if ( !pEntityIterator ) return 0;

	Entity *pEntity = pEntityIterator;
	pEntityIterator = pEntityIterator->pNextEntity;

	if ( !pEntityIterator && pTeamIterator == &cEnemyTeam )
	{
		pTeamIterator = &cHeroTeam;
		pEntityIterator = pTeamIterator->GetFirstMember ( );
	}

	if ( !pEntityIterator && pTeamIterator == &cHeroTeam )
	{
		pTeamIterator = &cNeutralTeam;
		pEntityIterator = pTeamIterator->GetFirstMember ( );
	}

	if ( !pEntityIterator && pTeamIterator == &cNeutralTeam )
	{
		pTeamIterator = 0;
		pEntityIterator = 0;
	}

	return pEntity;
}

Entity* TeamController::GetEntity ( int iObjID )
{
	Entity *pEntity = cHeroTeam.GetMember ( iObjID );
	if ( !pEntity ) pEntity = cEnemyTeam.GetMember ( iObjID );
	if ( !pEntity ) pEntity = cNeutralTeam.GetMember ( iObjID );

	return pEntity;
}

Entity* TeamController::GetEntityCopy ( int iObjID )
{
	Entity *pEntity = cHeroTeam.GetMemberCopy ( iObjID );
	if ( !pEntity ) pEntity = cEnemyTeam.GetMemberCopy ( iObjID );
	if ( !pEntity ) pEntity = cNeutralTeam.GetMemberCopy ( iObjID );

	return pEntity;
}

void TeamController::ClearCoverPoints()
{
	cHeroTeam.ClearCoverPoints();
	cEnemyTeam.ClearCoverPoints();
	cNeutralTeam.ClearCoverPoints();
}

void TeamController::RemoveEntity ( Entity *pDelEntity )
{
	cHeroTeam.RemoveMember ( pDelEntity );
	cEnemyTeam.RemoveMember ( pDelEntity );
	cNeutralTeam.RemoveMember ( pDelEntity );
}

void TeamController::DeleteEntity ( int iObjID )
{
	Entity *pEntity = GetEntity ( iObjID );
	if ( !pEntity ) return;

	RemoveEntity ( pEntity );
	delete pEntity;
}

Entity* TeamController::GetFirstFriendly( )
{
	return cHeroTeam.GetFirstMember( );
}

void TeamController::UpdateTimers ( float fTimeDelta )
{
	cHeroTeam.UpdateTimers ( fTimeDelta );
	cEnemyTeam.UpdateTimers ( fTimeDelta );
	cNeutralTeam.UpdateTimers ( fTimeDelta );
}

void TeamController::CheckZones ( Zone *pWorldZoneList )
{
	cHeroTeam.CheckZones ( pWorldZoneList );
	cEnemyTeam.CheckZones ( pWorldZoneList );
	cNeutralTeam.CheckZones ( pWorldZoneList );
}

void TeamController::CheckBeacons ( Beacon *pWorldBeaconList )
{
	cHeroTeam.CheckBeacons ( pWorldBeaconList );
	cEnemyTeam.CheckBeacons ( pWorldBeaconList );
	cNeutralTeam.CheckBeacons ( pWorldBeaconList );
}

void TeamController::UpdateState ( )
{
	cHeroTeam.UpdateMemberState ( );
	cEnemyTeam.UpdateMemberState ( );
	cNeutralTeam.UpdateMemberState ( );
}

void TeamController::UpdateMovement ( float fTimeDelta )
{
	// So only 1 path update is called every so often
	//NotDonePathThisFrame = false;

	// erase non valid entries when find them (no need for index now)
	int nmax = (int)ListOfEntitiesToUpdateMovement.size()-1;
	for ( int n = nmax; n >= 0; n-- )
	{
		Entity* pEntity = cWorld.GetEntityCopy ( ListOfEntitiesToUpdateMovement[n] );
		if ( !pEntity )
			ListOfEntitiesToUpdateMovement.erase (ListOfEntitiesToUpdateMovement.begin()+n);
	}
	/*
	// keep ListOfEntitiesToUpdateMovementIndex index valid
	bool foundOne = false;
	while ( !foundOne )
	{
		if ( ListOfEntitiesToUpdateMovement.size() == 0 ) break;
		if ( ListOfEntitiesToUpdateMovementIndex > (int)ListOfEntitiesToUpdateMovement.size()-1 )
			ListOfEntitiesToUpdateMovementIndex = 0;

		Entity* pEntity = cWorld.GetEntityCopy ( ListOfEntitiesToUpdateMovement[ListOfEntitiesToUpdateMovementIndex] );
		if ( !pEntity )
		{
			ListOfEntitiesToUpdateMovement.erase (ListOfEntitiesToUpdateMovement.begin()+ListOfEntitiesToUpdateMovementIndex);
			if ( ListOfEntitiesToUpdateMovementIndex > (int)ListOfEntitiesToUpdateMovement.size()-1 )
				ListOfEntitiesToUpdateMovementIndex = 0;
		}
		else
			foundOne = true;
	}
	*/

	//Dave - we dont have a hero team atm so commenting this out
	//cHeroTeam.UpdateMemberMovement ( fTimeDelta );
	cEnemyTeam.UpdateMemberMovement ( fTimeDelta );
	cNeutralTeam.UpdateMemberMovement ( fTimeDelta );

	// if this object hasnt been updated, lets check it still exists, then remove if it doesnt, else move onto the next one
	/* logic fail - ListOfEntitiesToUpdateMovementIndex can point to different AI, list not static!!
	if ( NotDonePathThisFrame == false )
	{
		// 100417 - use global timer to space out increments of this to ensure ALL AI logic not done one after other in cycle sequence!
		CurrentAIWorkedPathTimer -= fTimeDelta;
		if ( CurrentAIWorkedPathTimer <= 0.0f )
		{
			CurrentAIWorkedPathTimer = 20.0f; // X second until next inc.
			ListOfEntitiesToUpdateMovementIndex++;
			if ( ListOfEntitiesToUpdateMovementIndex > (int)ListOfEntitiesToUpdateMovement.size()-1 )
			{
				ListOfEntitiesToUpdateMovementIndex = 0;
				CurrentAIWorkedOn = -1;
			}
		}
	}
	*/
}

void TeamController::Update ( float fTimeDelta, Beacon *pWorldBeaconList, Zone *pWorldZoneList )
{
	//__int64 time1,time2;

	//QueryPerformanceCounter ( (LARGE_INTEGER*) &time1 );
	UpdateTimers ( fTimeDelta );
	CheckZones ( pWorldZoneList );
	CheckBeacons ( pWorldBeaconList );
	//QueryPerformanceCounter ( (LARGE_INTEGER*) &time2 );

	//char str [ 256 ];
	//sprintf_s( str, 256, " Init: %6d ", time2 - time1 );
	//if( pDebugFile ) fputs( str, pDebugFile );

	if ( iThreadMode == 0 )
	{
		//MessageBox( NULL, "Non Threaded", "Info", 0 );
		//QueryPerformanceCounter ( (LARGE_INTEGER*) &time1 );
		//Dave commented this out, dont need states for manual AI
		//UpdateState ( );
		UpdateMovement ( fTimeDelta );
		//QueryPerformanceCounter ( (LARGE_INTEGER*) &time2 );

		//sprintf_s( str, 256, " Movement(N/T): %6d ", time2 - time1 );
		//if( pDebugFile ) fputs( str, pDebugFile );
	}
	else
	{
		//char str [ 20 ];
		//sprintf_s( str, 20, "Threaded: %d", iThreadCount );
		//MessageBox( NULL, str, "Info", 0 );

		//QueryPerformanceCounter ( (LARGE_INTEGER*) &time1 );
		cHeroTeam.UpdatePlayerMovement( fTimeDelta );

		//EntityThread *pEntityThreads = new EntityThread [ iThreadCount ];
		EntityBuffer *pEntityBuffer = new EntityBuffer( );
				
		cHeroTeam.AddMembersToBuffer( pEntityBuffer );
		cEnemyTeam.AddMembersToBuffer( pEntityBuffer );
		cNeutralTeam.AddMembersToBuffer( pEntityBuffer );

		for ( int i = 0; i < iNumThreads; i++ )
		{
			pEntityThreads [ i ].SetupData( pEntityBuffer, fTimeDelta );
			//pEntityThreads [ i ].Start( );
			pEntityThreads [ i ].ResumeThread( );
		}

		for ( int i = 0; i < iNumThreads; i++ )
		{
			//pEntityThreads [ i ].Join( );
			pEntityThreads [ i ].WaitOnThread( );
			pEntityThreads [ i ].SetupData( NULL, 0 );
		}

		delete pEntityBuffer;
		//delete [] pEntityThreads;
		//QueryPerformanceCounter ( (LARGE_INTEGER*) &time2 );

		//sprintf_s( str, 256, " Movement(T): %6d ", time2 - time1 );
		//if( pDebugFile ) fputs( str, pDebugFile );
	}

	//cEnemyTeam.GetFirstMember()->DebugHideObstacleAngles( );
	//cEnemyTeam.GetFirstMember()->DebugDrawObstacleAngles( );
		
	CleanUpBeacons ( fTimeDelta );
}

void TeamController::SetThreadNumber( int threads )
{
	iNumThreads = threads;

	if ( pEntityThreads ) delete [] pEntityThreads;

	pEntityThreads = new EntityThread [ iNumThreads ];

	/*
	for ( int i = 0; i < iNumThreads; i++ )
	{
		pEntityThreads [ i ].Start( );
	}
	*/
}

void TeamController::CleanUpBeacons ( float fTimeDelta )
{
	cHeroTeam.CleanUpBeacons ( fTimeDelta );
	cEnemyTeam.CleanUpBeacons ( fTimeDelta );
	cNeutralTeam.CleanUpBeacons ( fTimeDelta );
}

void TeamController::DebugOutputEntityData ( HANDLE hOut, int iObjID )
{
	cHeroTeam.DebugOutputEntityData ( hOut, iObjID );
	cEnemyTeam.DebugOutputEntityData ( hOut, iObjID );
	cNeutralTeam.DebugOutputEntityData ( hOut, iObjID );
}

void TeamController::DebugShowBeacons ( int iTempMesh, int iCount )
{
	iCount = cHeroTeam.DebugShowBeacons ( iTempMesh, iCount );
	iCount = cEnemyTeam.DebugShowBeacons ( iTempMesh, iCount );
	iCount = cNeutralTeam.DebugShowBeacons ( iTempMesh, iCount );
}
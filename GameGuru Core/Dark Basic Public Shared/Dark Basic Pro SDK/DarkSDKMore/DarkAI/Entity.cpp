#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "DBPro Functions.h"

#include "Hero.h"
#include "Entity.h"
#include "StateMachine\StateSet.h"
#include "Path.h"
#include "PathFinderAdvanced.h"
#include "World.h"
#include "Beacon.h"
#include "Zone.h"
#include "Team.h"
#include "TeamController.h"
#include "Container.h"
#include "DynamicPathFinder.h"
#include <math.h>
#include <algorithm>

#include "LeeThread.h"

#define PI 3.14159265f
#define RADTODEG 57.295779513f
#define DEGTORAD 0.01745329252f

// Externals
extern World cWorld;
extern LeeThread g_LeeThread;
extern int g_GUIShaderEffectID;

// Globals
World* Entity::pWorld = NULL;
bool Entity::bShowPaths = false;
bool Entity::bShowViewArcs = false;
bool Entity::bShowAvoidanceAngles = false;
float Entity::fDebugViewObjHeight = 0.0f;
float Entity::fDebugObstacleAnglesObjHeight = 0.0f;
int Entity::iAvoidMode = 5;

// So only 1 path update is called every frame
float CurrentAIWorkedPathTimer = 0.0f;
vector <int> ListOfEntitiesToUpdateMovement;

Entity::Entity ( )
{
	SetDefaultValues ( );
}

Entity::Entity ( int id, sObject *pObj, sObject *pObjRef, Container* pCurrContainer )
{
	SetDefaultValues ( );
	
	bActive = true;
	iID = id;
	pObject = pObj;
	dwObjectNumberRef = pObjRef->dwObjectNumber;
	pContainer = pCurrContainer;
	
	if ( pObject ) 
	{
		vecFinalDest = pObject->position.vecPosition;
		fPosX = vecFinalDest.x;
		fPosY = vecFinalDest.y;
		fPosZ = vecFinalDest.z;
		fAngY = pObject->position.vecRotate.y;
	}
	else
	{
		vecFinalDest.x = 0.0f;
		vecFinalDest.y = 0.0f;
		vecFinalDest.z = 0.0f;
		fPosX = 0;
		fPosY = 0;
		fPosZ = 0;
		fAngY = 0;
	}

	vecOrigPos = vecFinalDest;
	vecDefendPos = vecOrigPos;

	iOriginalContainer = pContainer->GetID( );
	iDefendContainer = iOriginalContainer;

	vecLookAt = vecFinalDest;

	//Dave - manual only
	//this->ChangeState ( pWorld->pEntityStates->pStateIdle );

	//Dave
	this->SetManualControl( true );
	this->ChangeState ( pWorld->pEntityStates->pStateManual );
}

Entity::~Entity ( ) 
{ 
	ClearCoverPoint();

	sHitFrom_list.clear ( );
	sTarget_list.clear ( );

	DebugHideDestination();
	DebugHideObstacleAngles();
	DebugHideViewArcs();

	if ( pDynamicPathFinder ) delete pDynamicPathFinder;
}

void Entity::SetDefaultValues ( )
{
	//stagger = 0;
	pre_vecFinalDest.x = -999;
	pre_vecLastDest.x = -999;

	pDynamicPathFinder = 0;

	bMeleeMode = false;

	bActive = true;
	iID = 0;
	pCurrentState = 0;
	pLastState = 0;
	pObject = 0;
	pContainer = 0;
	iAggressiveness = 0;
	iDestContainer = -1;
	iEntityAvoidMode = -1;
	bAlwaysActive = false;

	iCurrentMovePoint = 1;
	iOriginalContainer = -1;
	iDefendContainer = -1;
	bChangingContainers = false;

	fPosX = 0; fPosY = 0; fPosZ = 0;
	fAngY = 0;
	fHeight = 0;

	bReservedSpace = false;

	fStateTimer = 0.0f;
	fLookAroundTimer = 0.0f;
	fCoverTimer = 0.0f;
	fMoveFeedbackTimer = 0.0f;
	fInvestigateTimer = 0.0f;
	fAvoidTimer = 0.0f;
	fPathTimer = 0.0f;
	fForceUpdatePathTimer = 0.0f;
	fResetTimer = 0.0f;
	fWaitTimer = 0.0f;
	fExpensiveUpdateTimer = ( 1.0f * rand() ) / RAND_MAX + 1.0f;
	fAdjustDestinationTimer = 0;
	fAdjustDirectionTimer = 0;
	fSearchTimer = 0;
	fObstacleTimer = 0;

	fAvgPatrolTime = 3.5f;

	fAlertLevel = 0.0f;

	bHit = false;
	bFireWeapon = false;
	bIsDucking = false;
	bHeardSound = false;
	fClosestSound = 0;
	iSoundUrgency = 0;
	iLastSoundUrgency = 0;
	iInvestigateUrgency = 0;

	bAdjustingDirection = false;

	bCanDuck = true;
	bCanStrafe = true;
	bCanAttack = true;
	bCanHear = true;
	bCanRoam = false;
	bCanSearch = true;
	bCanSelectTargets = true;

	cSearchPath.Clear ( );
	cSearchPath.CalculateSqrLength ( );

	fSpeed = 5.0f;
	fCurrSpeed = fSpeed;
	fTurnSpeed = fSpeed*7.5f;//500.0f;//240.0f; 140217 - turn twice as fast for fast movers (can adjust in LUA too)

	fRadius = 2.5f;
	fMinimumDist = fRadius * ( ( 4.0f * rand( ) ) / RAND_MAX + 6.5f );
	fAvoidDist = fRadius * 6.0f;
	fPathStartCostLimit = -1.0f;

	fDefendDist = 15.0f;

	fDistanceMoved = 0.0f;
	bStuck = false;
	bAvoiding = false;
	bMakingProgress = true;
	bMoving = false;
	bAvoidLeft = true;
	bRedoPath = false;
	bLegacyForceMove = false;
	
	bFollowingLeader = false;
	bDefending = false;
	bManualControl = false;
	bLookAtPointSet = false;	

	bHitSomething = false;
	bHitBySomething = false;
	pHitEntity = 0;
	pHitByEntity = 0;

	vecOrigPos.x = 0; vecOrigPos.y = 0; vecOrigPos.z = 0;
	vecDefendPos.x = 0; vecDefendPos.y = 0; vecDefendPos.z = 0;
	vecHidePos.x = 0; vecHidePos.y = 0; vecHidePos.z = 0;
	vecOldPos.x = 0; vecOldPos.y = 0; vecOldPos.z = 0;
	vecAvoidPos.x = 0; vecAvoidPos.y = 0; vecAvoidPos.z = 0;
	vecLastPos.x = 0; vecLastPos.y = 0; vecLastPos.z = 0;

	vecSoundPos.x = 0; vecSoundPos.y = 0; vecSoundPos.z = 0;
	vecInterestPos.x = 0; vecInterestPos.y = 0; vecInterestPos.z = 0;

	for (int i=0; i<100; i++ ) { pvecDir[i].x = 0; pvecDir[i].y = 0; pvecDir[i].z = 0; }
	vecLastDest.x = 0; vecLastDest.y = 0; vecLastDest.z = 0;
	vecFinalDest.x = 0; vecFinalDest.y = 0; vecFinalDest.z = 0;
	vecCurrDest.x = 0; vecCurrDest.y = 0; vecCurrDest.z = 0;
	vecLookAt.x = 0; vecLookAt.y = 0; vecLookAt.z = 0;
	vecCurrLookAt.x = 0; vecCurrLookAt.y = 0; vecCurrLookAt.z = 0;

	fFireArc = 20.0f;
	fInnerViewArc = 180.0f;
	fOuterViewArc = 180.0f;
	fViewRange = 60.0f;
	fHearingRange = 50.0f;
	iHearingThreshold = 0;

	iInnerViewObject = 0;
	iOuterViewObject = 0;
	iHearingRangeObject = 0;
	iCurrDestObject = 0;
	iObstacleAnglesObject = 0;

	pTeam = 0;

	sTarget_list.clear ( );
	sHitFrom_list.clear ( );
	sObstacleAngle_list.clear ( );
	sCombinedObstacleAngle_list.clear( );

	pPatrolPath = 0;
	iPatrolPathID = 0;
	iCurrentPatrolPoint = 0;

	pNextEntity = 0;

	bIsBehindCorner = false;
	bCanHideBehindCorner = false;

	fDiveRange = 0;
	bIsDiving = false;
	fLeapRange = 0;
	bIsLeaping = false;
	bCanDive = false;
	bCanLeap = false;
	fHideDirX = 0;
	fHideDirY = 0;
	fPeekX = 0;
	fPeekY = 0;
	bForceOutOfCover = false;
	fDiveMultiplier = 1.0f;
	fLeapMultiplier = 1.0f;
	
	pCoverPoint = 0;
}

void Entity::ResetState ( )
{
	if ( !bManualControl )
	{
		fResetTimer = 4.0f;
		ChangeState ( pWorld->pEntityStates->pStateIdle );
	}
}

bool Entity::GetMeleeMode( )
{
	return bMeleeMode;
}

void Entity::SetMeleeMode( bool mode )
{
	bMeleeMode = mode;
}

int Entity::GetInterestContainer( )
{
	return iInterestContainer;
}

bool Entity::GetActive( )
{
	return bActive;
}

int Entity::GetAggressiveness( )
{
	return iAggressiveness;
}

int Entity::GetSearchPoints( )
{
	return iSearchPoints;
}

int Entity::GetCurrentTargetIndex( )
{
	return iCurrentTargetIndex;
}

int Entity::GetInvestigateUrgency( )
{
	return iInvestigateUrgency;
}

int Entity::GetDefendContainer( )
{
	return iDefendContainer;
}

bool Entity::GetCanDuck( )
{
	return bCanDuck;
}

bool Entity::GetCanAttack( )
{
	return bCanAttack;
}

bool Entity::GetCanStrafe( )
{
	return bCanStrafe;
}

bool Entity::GetCanHear( )
{
	return bCanHear;
}

bool Entity::GetCanRoam( )
{
	return bCanRoam;
}

bool Entity::GetCanSearch( )
{
	return bCanSearch;
}

bool Entity::GetCanSelectTargets( )
{
	return bCanSelectTargets;
}

bool Entity::GetCanHideBehindCorner( )
{
	return bCanHideBehindCorner;
}

bool Entity::GetInVerticalCover( )
{
	return bInVerticalCover;
}

bool Entity::GetInDuckingCover( )
{
	return bInDuckingCover;
}

bool Entity::GetManualControl( )
{
	return bManualControl;
}

bool Entity::GetIsBehindCorner( )
{
	return bIsBehindCorner;
}

void Entity::SetIsBehindCorner( bool bValue )
{
	bIsBehindCorner = bValue;
}

void Entity::SetHideDirection( float dirx, float diry )
{
	fHideDirX = dirx;
	fHideDirY = diry;
}

void Entity::SetPeekingPos( float x, float y )
{
	fPeekX = x;
	fPeekY = y;
}

float Entity::GetHideX()
{
	return fHideDirX;
}

float Entity::GetHideY()
{
	return fHideDirY;
}

bool Entity::GetCanLeap( )
{
	return bCanLeap;
}

bool Entity::GetIsLeaping( )
{
	return bIsLeaping;
}

bool Entity::GetCanDive( )
{
	return bCanDive;
}

bool Entity::GetIsDiving()
{
	return bIsDiving;
}

void Entity::SetDiving( bool value )
{
	bIsDiving = value;
}

void Entity::SetLeaping( bool value )
{
	bIsLeaping = value;
}

float Entity::GetPeekX()
{
	return fPeekX;
}

float Entity::GetPeekY()
{
	return fPeekY;
}

void Entity::ForceOutOfCover( bool mode )
{
	bForceOutOfCover = mode;
}

bool Entity::IsForcedOutOfCover()
{
	return bForceOutOfCover;
}

float Entity::GetOrigPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecOrigPos.x;
	if ( flag & ENT_Y ) return vecOrigPos.y;
	if ( flag & ENT_Z ) return vecOrigPos.z;
	return 0;
}

float Entity::GetDefendPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecDefendPos.x;
	if ( flag & ENT_Y ) return vecDefendPos.y;
	if ( flag & ENT_Z ) return vecDefendPos.z;
	return 0;
}

float Entity::GetHidePos( DWORD flag )
{
	if ( flag & ENT_X ) return vecHidePos.x;
	if ( flag & ENT_Y ) return vecHidePos.y;
	if ( flag & ENT_Z ) return vecHidePos.z;
	return 0;
}

float Entity::GetOldPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecOldPos.x;
	if ( flag & ENT_Y ) return vecOldPos.y;
	if ( flag & ENT_Z ) return vecOldPos.z;
	return 0;
}

float Entity::GetAvoidPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecAvoidPos.x;
	if ( flag & ENT_Y ) return vecAvoidPos.y;
	if ( flag & ENT_Z ) return vecAvoidPos.z;
	return 0;
}

float Entity::GetLastPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecLastPos.x;
	if ( flag & ENT_Y ) return vecLastPos.y;
	if ( flag & ENT_Z ) return vecLastPos.z;
	return 0;
}

float Entity::GetSoundPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecSoundPos.x;
	if ( flag & ENT_Y ) return vecSoundPos.y;
	if ( flag & ENT_Z ) return vecSoundPos.z;
	return 0;
}

float Entity::GetInterestPos( DWORD flag )
{
	if ( flag & ENT_X ) return vecInterestPos.x;
	if ( flag & ENT_Y ) return vecInterestPos.y;
	if ( flag & ENT_Z ) return vecInterestPos.z;
	return 0;
}

float Entity::GetLastDest( DWORD flag )
{
	if ( flag & ENT_X ) return vecLastDest.x;
	if ( flag & ENT_Y ) return vecLastDest.y;
	if ( flag & ENT_Z ) return vecLastDest.z;
	return 0;
}

float Entity::GetFinalDest( DWORD flag )
{
	if ( flag & ENT_X ) return vecFinalDest.x;
	if ( flag & ENT_Y ) return vecFinalDest.y;
	if ( flag & ENT_Z ) return vecFinalDest.z;
	return 0;
}

float Entity::GetCurrDest( DWORD flag )
{
	if ( flag & ENT_X ) return vecCurrDest.x;
	if ( flag & ENT_Y ) return vecCurrDest.y;
	if ( flag & ENT_Z ) return vecCurrDest.z;
	return 0;
}

float Entity::GetLookAt( DWORD flag )
{
	if ( flag & ENT_X ) return vecLookAt.x;
	if ( flag & ENT_Y ) return vecLookAt.y;
	if ( flag & ENT_Z ) return vecLookAt.z;
	return 0;
}

float Entity::GetCurrLookAt( DWORD flag )
{
	if ( flag & ENT_X ) return vecCurrLookAt.x;
	if ( flag & ENT_Y ) return vecCurrLookAt.y;
	if ( flag & ENT_Z ) return vecCurrLookAt.z;
	return 0;
}

Path* Entity::GetSearchPath( )
{
	return &cSearchPath;
}

float Entity::GetSearchPointX( DWORD index )
{
	return cSearchPath.GetPoint( index ).x;
}

float Entity::GetSearchPointY( DWORD index )
{
	return cSearchPath.GetPoint( index ).y;
}

int Entity::GetSearchPointContainer( DWORD index )
{
	return cSearchPath.GetPoint( index ).container;
}

void Entity::RemoveFromMovementList()
{
	for ( int lindex = 0; lindex < ListOfEntitiesToUpdateMovement.size(); lindex++ )
	{
		if ( ListOfEntitiesToUpdateMovement[lindex] == GetID() )
		{
			ListOfEntitiesToUpdateMovement.erase(ListOfEntitiesToUpdateMovement.begin()+lindex);
			bRedoPath = false;
			break;
		}
	}
}

void Entity::SetActive( bool bValue )
{
	// set state for entity active status
	bActive = bValue;

	// if deactivating, ensure it is removed from movement list
	if ( bValue == false ) 
	{
		RemoveFromMovementList();
	}
}

void Entity::SetAggressiveness( int iValue )
{
	//char str [ 256 ];
	//sprintf_s( str, "STOP (SA) = %d", iValue );
	//MessageBox( NULL, str, "Info", 0 );

	iAggressiveness = iValue;
}

void Entity::SetSearchPoints( int iValue )
{
	iSearchPoints = iValue;
}

void Entity::IncSearchPoints( int iValue )
{
	iSearchPoints += iValue;
}

void Entity::SetCurrentTargetIndex( int iValue )
{
	iCurrentTargetIndex = iValue;
}

void Entity::SetInvestigateUrgency( int iValue )
{
	iInvestigateUrgency = iValue;
}

void Entity::SetDefendContainer( int iValue )
{
	iDefendContainer = iValue;
}

void Entity::SetCanDuck( bool bValue )
{
	bCanDuck = bValue;
}

void Entity::SetCanHideBehindCorner( bool bValue )
{
	bCanHideBehindCorner = bValue;
}

void Entity::SetCanDive( bool bValue )
{
	bCanDive = bValue;
}

void Entity::SetCanLeap( bool bValue )
{
	bCanLeap = bValue;
}

void Entity::SetDiveRange( float fValue )
{
	fDiveRange = fValue;
}

void Entity::SetLeapRange( float fValue )
{
	fLeapRange = fValue;
}

void Entity::SetCanAttack( bool bValue )
{
	bCanAttack = bValue;
}

void Entity::SetCanStrafe( bool bValue )
{
	bCanStrafe = bValue;
}

void Entity::SetCanHear( bool bValue )
{
	bCanHear = bValue;
}

void Entity::SetCanRoam( bool bValue )
{
	bCanRoam = bValue;
}

void Entity::SetCanSearch( bool bValue )
{
	bCanSearch = bValue;
}

void Entity::SetCanSelectTargets( bool bValue )
{
	bCanSelectTargets = bValue;
}

void Entity::SetInVerticalCover( bool bValue )
{
	bInVerticalCover = bValue;
}

void Entity::SetInDuckingCover( bool bValue )
{
	bInDuckingCover = bValue;
}

void Entity::SetManualControl( bool bValue )
{
	bManualControl = bValue;
}

void Entity::SetHitSomething( bool bValue )
{
	bHitSomething = bValue;
}

void Entity::SetHitBySomething( bool bValue )
{
	bHitBySomething = bValue;
}

void Entity::SetHitEntity( Entity *pValue )
{
	pHitEntity = pValue;
}

void Entity::SetHitByEntity( Entity *pValue )
{
	pHitByEntity = pValue;
}

void Entity::SetOrigPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecOrigPos.x = x;
	if ( flag & ENT_Y ) vecOrigPos.y = y;
	if ( flag & ENT_Z ) vecOrigPos.z = z;
}

void Entity::SetDefendPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecDefendPos.x = x;
	if ( flag & ENT_Y ) vecDefendPos.y = y;
	if ( flag & ENT_Z ) vecDefendPos.z = z;
}

void Entity::SetHidePos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecHidePos.x = x;
	if ( flag & ENT_Y ) vecHidePos.y = y;
	if ( flag & ENT_Z ) vecHidePos.z = z;
}

void Entity::SetOldPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecOldPos.x = x;
	if ( flag & ENT_Y ) vecOldPos.y = y;
	if ( flag & ENT_Z ) vecOldPos.z = z;
}

void Entity::SetAvoidPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecAvoidPos.x = x;
	if ( flag & ENT_Y ) vecAvoidPos.y = y;
	if ( flag & ENT_Z ) vecAvoidPos.z = z;
}

void Entity::SetLastPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecLastPos.x = x;
	if ( flag & ENT_Y ) vecLastPos.y = y;
	if ( flag & ENT_Z ) vecLastPos.z = z;
}

void Entity::SetSoundPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecSoundPos.x = x;
	if ( flag & ENT_Y ) vecSoundPos.y = y;
	if ( flag & ENT_Z ) vecSoundPos.z = z;
}

void Entity::SetInterestPos( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecInterestPos.x = x;
	if ( flag & ENT_Y ) vecInterestPos.y = y;
	if ( flag & ENT_Z ) vecInterestPos.z = z;
}

void Entity::SetLastDest( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecLastDest.x = x;
	if ( flag & ENT_Y ) vecLastDest.y = y;
	if ( flag & ENT_Z ) vecLastDest.z = z;
}

void Entity::SetFinalDest( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecFinalDest.x = x;
	if ( flag & ENT_Y ) vecFinalDest.y = y;
	if ( flag & ENT_Z ) vecFinalDest.z = z;
}

void Entity::SetCurrDest( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecCurrDest.x = x;
	if ( flag & ENT_Y ) vecCurrDest.y = y;
	if ( flag & ENT_Z ) vecCurrDest.z = z;
}

void Entity::SetLookAt( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecLookAt.x = x;
	if ( flag & ENT_Y ) vecLookAt.y = y;
	if ( flag & ENT_Z ) vecLookAt.z = z;
}

void Entity::SetCurrLookAt( float x, float y, float z, DWORD flag )
{
	if ( flag & ENT_X ) vecCurrLookAt.x = x;
	if ( flag & ENT_Y ) vecCurrLookAt.y = y;
	if ( flag & ENT_Z ) vecCurrLookAt.z = z;
}

void Entity::ClearSearchPath( )
{
	cSearchPath.Clear( );
}



void Entity::SetRadius ( float fNewRadius )
{
	if ( fNewRadius < 0.0001f ) fNewRadius = 0.0001f;	//division by zero counter
	
	fMinimumDist /= fRadius;
	fAvoidDist /= fRadius;
	fRadius = fNewRadius;
	fMinimumDist *= fRadius;
	fAvoidDist *= fRadius;
}

float Entity::GetRadius ( )
{
	return fRadius;
}

void Entity::SetContainer ( Container *pNewContainer )
{
	int l_iAvoidMode = iEntityAvoidMode;
	if ( l_iAvoidMode < 0 ) l_iAvoidMode = iAvoidMode;

	if ( l_iAvoidMode == 5 ) pContainer->pPathFinder->GridClearEntityPosition( GetX(), GetZ( ) );

	pContainer = pNewContainer;
	if ( pDynamicPathFinder && pContainer ) pDynamicPathFinder->SetMainPathFinder( pContainer->pPathFinder );
}

Container * Entity::GetContainer ( )
{
	return pContainer;
}

bool Entity::IsChangingContainers( )
{
	return bChangingContainers;
}

bool Entity::Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide )
{
	float fVX = fEX - fSX;
	float fVZ = fEZ - fSZ;
	float fSqrDist = fVX*fVX + fVZ*fVZ;
	if ( fSqrDist < 0.00001f ) return false;

	float fV2X = GetX( ) - fSX;
	float fV2Z = GetZ( ) - fSZ;
	float fTValue = ( fVX*fV2X + fVZ*fV2Z ) / fSqrDist;
	float fNewSide = fVZ*fV2X - fVX*fV2Z;

	if ( fTValue < 0.0f ) fTValue = 0.0f;
	if ( fTValue > 1.0f ) fTValue = 1.0f;

	float fClosestX = fVX*fTValue + fSX;
	float fClosestZ = fVZ*fTValue + fSZ;

	fVX = fClosestX - GetX( );
	fVZ = fClosestZ - GetZ( );
	fSqrDist = fVX*fVX + fVZ*fVZ;

	if ( fSqrDist < fRadius*fRadius ) 
	{
		*iSide = fNewSide > 0 ? 1 : -1;
		return true;
	}

	return false;
}

void Entity::SetPosition ( float x, float y, float z )
{
	//if ( pContainer ) pContainer->pPathFinder->GridMoveEntity( GetX(), GetZ(), x, z );
	if ( pObject )
	{
		pObject->position.vecPosition.x = x;
		pObject->position.vecPosition.y = y;
		pObject->position.vecPosition.z = z;
	}
	else
	{
		fPosX = x;
		fPosY = y;
		fPosZ = z;
	}
	vecLastRecPos.x = x;
	vecLastRecPos.y = y;
	vecLastRecPos.z = z;
}

void Entity::StayWithinContainer ( float fLastX, float fLastZ, float* pX, float* pZ )
{
	// always keep AI inside container (or outside obstacles)
	// note: gets called just after entity pos is moved by 'MoveWithAnimation'
	if ( pContainer )
	{
		if ( pContainer->pPathFinder->InPolygons ( *pX, *pZ ) > 0 )
		{
			// find closest polygon
			float fResult = pContainer->pPathFinder->FindClosestPolygon ( fLastX, fLastZ, *pX, *pZ );
			if ( fResult > 0.0001f ) // fResult >= 0.0f ) // prevents stopping AI when traversing edge
			{
				*pX = fLastX + ( *pX - fLastX ) * fResult;
				*pZ = fLastZ + ( *pZ - fLastZ ) * fResult;
			}
			else
			{
				// 080317 - absolutely prevent AI from leaving the zone, revert to last good position
				*pX = fLastX;
				*pZ = fLastZ;
			}
		}
	}
}

void Entity::SetYPosition ( float y )
{
	if ( pObject ) pObject->position.vecPosition.y = y;
	else fPosY = y;

	vecLastRecPos.y = y;
}

void Entity::SetDestination ( float x, float y, float z, int container )
{
	if ( container < 0 ) container = GetContainer( )->GetID( );

	float fDiffX = vecFinalDest.x - x;
	float fDiffY = vecFinalDest.y - y;
	float fDiffZ = vecFinalDest.z - z;
	
	float fDist = fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ;
	
	if ( fDist > 0.00001 || container != GetContainer( )->GetID( ) )
	{
		pre_vecFinalDest.x = x;
		pre_vecFinalDest.y = y;
		pre_vecFinalDest.z = z;

		pre_vecLastDest.x = x;
		pre_vecLastDest.y = y;
		pre_vecLastDest.z = z;

		iDestContainer = container;

		// calculate the path we need
		if ( bRedoPath == false )
		{
			// this entity can be updated
			int n = 0;
			int iMaxInList = (int)ListOfEntitiesToUpdateMovement.size()-1;
			for (; n <= iMaxInList; n++ )
				if ( ListOfEntitiesToUpdateMovement[n] == this->iID )
					break;
			if ( n > iMaxInList )
				ListOfEntitiesToUpdateMovement.push_back(this->iID);
			bRedoPath = true;
		}
	}
}

void Entity::SetDestinationForced ( float x, float y, float z, int container, bool bMoveAddition )
{
	if ( container < 0 ) container = GetContainer( )->GetID( );
	float fDiffX = vecFinalDest.x - x;
	float fDiffY = vecFinalDest.y - y;
	float fDiffZ = vecFinalDest.z - z;
	float fDist = fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ;
	if ( fDist > 0.00001 || container != GetContainer( )->GetID( ) )
	{
		vecFinalDest.x = x;
		vecFinalDest.y = y;
		vecFinalDest.z = z;
		vecLastDest.x = x;
		vecLastDest.y = y;
		vecLastDest.z = z;
		pre_vecFinalDest.x = -999;
		pre_vecLastDest.x = -999;
		iDestContainer = container;
		if ( bMoveAddition == true )
		{
			if ( bRedoPath == false )
			{
				int n = 0;
				int iMaxInList = (int)ListOfEntitiesToUpdateMovement.size()-1;
				for (; n <= iMaxInList; n++ )
					if ( ListOfEntitiesToUpdateMovement[n] == this->iID )
						break;
				if ( n > iMaxInList )
					ListOfEntitiesToUpdateMovement.push_back(this->iID);
				bRedoPath = true;
			}
		}
	}
}

void Entity::SetIdlePos ( float x, float y, float z, int container = -1 )
{
	vecOrigPos.x = x;
	vecOrigPos.y = y;
	vecOrigPos.z = z;

	iOriginalContainer = container > 0 ? container : pContainer->GetID( );
}

void Entity::SetAngleY( float angY )
{
	fAngY = angY;
	//if ( pObject ) YRotateObject( iID, fAngY ); // 210918 - decouple forcing angle to object (so script can smooth it)
}

void Entity::SetHeight( float height )
{
	fHeight = height;
}

void Entity::SetTeam ( Team *pNewTeam )
{
	pTeam = pNewTeam;
}

float Entity::GetHeight( )
{
	return bIsDucking ? fHeight/2 : fHeight;
}

float Entity::GetFullHeight( )
{
	return fHeight;
}

int Entity::GetID ( )
{
	return iID;
}

float Entity::GetX ( ) { if ( pObject ) return pObject->position.vecPosition.x; else return fPosX; }
float Entity::GetY ( ) { if ( pObject ) return pObject->position.vecPosition.y; else return fPosY; }
float Entity::GetZ ( ) { if ( pObject ) return pObject->position.vecPosition.z; else return fPosZ; }
float Entity::GetAngleY ( ) 
{ 
	float AngY = fAngY;
	if ( pObject ) AngY = pObject->position.vecRotate.y;
	
	while ( AngY > 360.0f ) AngY -= 360.0;
	while ( AngY < 0.0f )	AngY += 360.0;

	return AngY; 
}

bool Entity::IsTurning ( )
{
	if ( !bLookAtPointSet ) return bMoving;

	float x = vecLookAt.x - GetX ( );
	float z = vecLookAt.z - GetZ ( );
	float fLookAngY;

	if ( fabs ( x ) + fabs ( z ) < 0.1f ) 
	{
		return false;
	}
	else
	{
		fLookAngY = z / sqrt ( x*x + z*z );
		fLookAngY = acos ( fLookAngY ) * RADTODEG;
		if ( x < 0.0f ) fLookAngY = 360.0f - fLookAngY;
	}
	
	float fArc = fabs ( fLookAngY - GetAngleY( ) );
	if ( fArc > 180.0f ) fArc  = 360.0f - fArc;

	return fArc > 0.1f;
}

bool Entity::IsAvoiding ( )
{
	return bAvoiding;
}

bool Entity::IsMakingProgress ( )
{
	return bMakingProgress;
}

bool Entity::IsMoving ( )
{
	return bMoving;
}

float Entity::GetDestX ( ) { return vecFinalDest.x; }
float Entity::GetDestY ( ) { return vecFinalDest.y; }
float Entity::GetDestZ ( ) { return vecFinalDest.z; }

float Entity::GetTargetX ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecPos.x;
}

float Entity::GetTargetY ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecPos.y;
}

float Entity::GetTargetZ ( )  
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecPos.z;
}

int Entity::GetTargetContainer( )  
{ 
	if ( CountTargets( ) == 0 ) return -1;
	return sTarget_list [ 0 ].iContainer;
}

float Entity::GetTargetGuessX ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecGuessPos.x;
}

float Entity::GetTargetGuessY ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecGuessPos.y;
}

float Entity::GetTargetGuessZ ( )  
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecGuessPos.z;
}

float Entity::GetTargetDirX ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecLastDir.x;
}

float Entity::GetTargetDirY ( ) 
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecLastDir.y;
}

float Entity::GetTargetDirZ ( )  
{ 
	if ( CountTargets( ) == 0 ) return 0.0f;
	return sTarget_list [ 0 ].vecLastDir.z;
}

int Entity::GetTargetType ( )
{
	if ( CountTargets( ) == 0 ) return -1;
	return sTarget_list [ 0 ].iTargetType;
}

bool Entity::GetIsZoneTarget( )
{
	if ( CountTargets( ) == 0 ) return false;
	return sTarget_list [ 0 ].bZoneTarget;
}

int Entity::GetTargetID ( int iIndex )
{
	if ( iIndex < 1 || iIndex > CountTargets( ) ) return 0;
	iIndex--;

	if ( sTarget_list [ iIndex ].iTargetType == 1 && sTarget_list [ iIndex ].pTargetEntity )	
	{
		return sTarget_list [ iIndex ].pTargetEntity->GetID( );
	}

	if ( sTarget_list [ iIndex ].iTargetType == 0 && sTarget_list [ iIndex ].pTargetHero )	
	{
		return sTarget_list [ iIndex ].pTargetHero->GetID( );
	}

	return 0;
}

bool Entity::GetIsFiring ( )
{
	return bFireWeapon;
}

bool Entity::GetIsDucking ( )
{
	return bIsDucking;
}

float Entity::GetDistToDest ( ) { return sqrt ( GetSqrDistToDest ( ) ); }
float Entity::GetDistToTarget ( ) { return sqrt ( GetSqrDistToTarget ( ) ); }
float Entity::GetDistTo ( GGVECTOR3 vecPoint ) { return sqrt ( GetSqrDistTo ( vecPoint ) ); }
float Entity::GetDistTo ( float x, float y, float z ) { return sqrt ( GetSqrDistTo ( x, y, z ) ); }

float Entity::GetSqrDistToDest ( )
{
	float fX = vecFinalDest.x - GetX ( );
	float fY = vecFinalDest.y - GetY ( );
	float fZ = vecFinalDest.z - GetZ ( );

	if ( pContainer->GetID( ) != iDestContainer ) return fX*fX + fZ*fZ + 10000;

	//return fX*fX + fY*fY + fZ*fZ;
	return fX*fX + fZ*fZ;
}

float Entity::GetSqrDistToTarget ( )
{
	float fX = GetTargetX ( ) - GetX ( );
	float fY = GetTargetY ( ) - GetY ( );
	float fZ = GetTargetZ ( ) - GetZ ( );

	return fX*fX + fZ*fZ;
}

float Entity::GetSqrDistTo ( GGVECTOR3 vecPoint )
{
	float fX = vecPoint.x - GetX ( );
	float fY = vecPoint.y - GetY ( );
	float fZ = vecPoint.z - GetZ ( );

	return fX*fX + fZ*fZ;
}

float Entity::GetSqrDistTo ( float x, float y, float z )
{
	float fX = x - GetX ( );
	float fY = y - GetY ( );
	float fZ = z - GetZ ( );

	return fX*fX + fZ*fZ;
}

char* Entity::GetStateName ( )
{
	return pCurrentState->GetName ( );
}

void Entity::SetPatrolPath ( Path* pPath, int iPathID )
{
	pPatrolPath = pPath;
	iPatrolPathID = iPathID;
	iCurrentPatrolPoint = 0;
}

void Entity::RemovePatrolPath ( )
{
	pPatrolPath = 0;
	iPatrolPathID = 0;
	iCurrentPatrolPoint = 0;
}

bool Entity::PatrolPathExist ( )
{
	CheckPath ( );
	return ( pPatrolPath != 0 );
}

int Entity::CountPatrolPoints ( )
{
	CheckPath ( );
	if ( !pPatrolPath ) return 0;
	
	return pPatrolPath->CountPoints( );
}

void Entity::CheckPath ( )
{
	if ( pPatrolPath )
	{
		if ( !pWorld->GetPath ( iPatrolPathID ) )
		{
			pPatrolPath = 0;
			iPatrolPathID = 0;
			iCurrentPatrolPoint = 0;
		}
	}
}

void Entity::CheckTargets ( )
{
	for ( int i = 0; i < (int) sTarget_list.size( ); i++ )
	{
		if ( sTarget_list[ i ].iTargetType == 0 )
		{
			if ( !pWorld->GetHero ( sTarget_list[ i ].iTargetID ) )
			{
				sTarget_list.erase ( sTarget_list.begin( ) + i );
				i--;
			}
		} else if ( sTarget_list[ i ].iTargetType == 1 )
		{
			if ( !pWorld->GetEntity ( sTarget_list[ i ].iTargetID ) )
			{
				sTarget_list.erase ( sTarget_list.begin( ) + i );
				i--;
			}
		}
	}
}

bool Entity::FindTargets ( )
{
	if ( !pTeam ) return sTarget_list.size( ) > 0;
	if ( pTeam->IsNeutral ( ) ) return sTarget_list.size( ) > 0;
	if ( !bCanSelectTargets ) return sTarget_list.size( ) > 0;
	
	TeamController *pTeams = pWorld->pTeamController;
	
	pTeams->StartEntityIterator ( pTeam );
	Entity *pOtherEntity = pTeams->GetNextEntity ( );

	while ( pOtherEntity )
	{
		if ( this->CanSee ( pOtherEntity ) && !this->IsTarget ( pOtherEntity ) )
		{
			if ( pOtherEntity->pTeam != pTeam && !pOtherEntity->IsNeutral( ) ) 
			{
				this->AddTarget ( pOtherEntity, false );
			}
		}

		pOtherEntity = pTeams->GetNextEntity ( );
	}

	pTeams->StartHeroIterator ( pTeam );
	Hero *pHero = pTeams->GetNextHero ( );

	while ( pHero )
	{
		if ( this->CanSee ( pHero ) && !this->IsTarget ( pHero ) )
		{
			if ( pHero->pTeam != pTeam )
			{
				this->AddTarget ( pHero, false );
			}
		}

		pHero = pTeams->GetNextHero ( );
	}

	return ( sTarget_list.size( ) > 0 );
}

int Entity::CountTargets ( )
{
	return (int) sTarget_list.size ( );
}

bool Entity::ValidTarget ( )
{
	if ( CountTargets ( ) == 0 ) return false;

	return ( sTarget_list [ 0 ].bCanSee );
}

bool Entity::IsTarget ( Entity *pOtherEntity )
{
	int iNumTargets = (int) sTarget_list.size ( );

	for ( int i = 0; i < iNumTargets; i++ )
	{
		if ( sTarget_list [ i ].pTargetEntity == pOtherEntity ) return true;
	}

	return false;
}

bool Entity::IsTarget ( Hero *pHero )
{
	int iNumTargets = (int) sTarget_list.size ( );

	for ( int i = 0; i < iNumTargets; i++ )
	{
		if ( sTarget_list [ i ].pTargetHero == pHero ) return true;
	}

	return false;
}

void Entity::AddTarget ( Entity *pOtherEntity, bool bFromZone )
{
	if ( !pOtherEntity ) return;
	//if ( pOtherEntity->pTeam == pTeam ) return;
	//if ( pOtherEntity->IsNeutral( ) ) return;
	if ( IsTarget ( pOtherEntity ) ) return;

	sTarget sNewTarget;
	sNewTarget.bZoneTarget = bFromZone;
	if ( bFromZone ) 
	{
		sNewTarget.fTargetThreat = 3.0f;
		sNewTarget.fTargetTimer = 60.0f;
	}
	else 
	{
		sNewTarget.fTargetThreat = 1.0f;
		sNewTarget.fTargetTimer = 10.0f;
	}

	sNewTarget.iTargetID = pOtherEntity->iID;
	sNewTarget.iTargetType = 1;
	sNewTarget.pTargetEntity = pOtherEntity;
	sNewTarget.pTargetHero = 0;
	sNewTarget.pTargetObject = 0;
	sNewTarget.bCanSee = !bFromZone;
	sNewTarget.iContainer = pOtherEntity->GetContainer( )->GetID( );
	
	sNewTarget.vecPos.x = pOtherEntity->GetX( );
	sNewTarget.vecPos.y = pOtherEntity->GetY( ) + pOtherEntity->GetHeight( );
	sNewTarget.vecPos.z = pOtherEntity->GetZ( );
	
	sNewTarget.vecLastPos = sNewTarget.vecPos;
	sNewTarget.vecGuessPos = sNewTarget.vecPos;
	
	sNewTarget.vecLastDir.x = 0;
	sNewTarget.vecLastDir.y = 0;
	sNewTarget.vecLastDir.z = 0;

	sTarget_list.push_back ( sNewTarget );
	sort ( sTarget_list.begin( ), sTarget_list.end( ) );
}

void Entity::AddTarget ( Hero *pHero, bool bFromZone )
{
	if ( !pHero ) return;
	//if ( pHero->pTeam == pTeam ) return;
	if ( IsTarget ( pHero ) ) return;

	sTarget sNewTarget;
	sNewTarget.bZoneTarget = bFromZone;
	if ( bFromZone ) 
	{
		sNewTarget.fTargetThreat = 3.0f;
		sNewTarget.fTargetTimer = 60.0f;
	}
	else 
	{
		sNewTarget.fTargetThreat = 1.0f;
		sNewTarget.fTargetTimer = 10.0f;
	}

	sNewTarget.iTargetID = pHero->iID;
	sNewTarget.iTargetType = 0;
	sNewTarget.pTargetEntity = 0;
	sNewTarget.pTargetHero = pHero;
	sNewTarget.pTargetObject = 0;
	sNewTarget.bCanSee = !bFromZone;
	sNewTarget.iContainer = pHero->GetContainer( )->GetID( );
	
	sNewTarget.vecPos.x = pHero->GetX( );
	sNewTarget.vecPos.y = pHero->GetY( ) + pHero->GetHeight( );
	sNewTarget.vecPos.z = pHero->GetZ( );
	
	sNewTarget.vecLastPos = sNewTarget.vecPos;
	sNewTarget.vecGuessPos = sNewTarget.vecPos;
	
	sNewTarget.vecLastDir.x = 0;
	sNewTarget.vecLastDir.y = 0;
	sNewTarget.vecLastDir.z = 0;

	sTarget_list.push_back ( sNewTarget );
	sort ( sTarget_list.begin( ), sTarget_list.end( ) );
}

void Entity::RemoveTarget ( Entity *pOtherEntity )
{
	for ( int i = 0; i < (int) sTarget_list.size ( ); i++ )
	{
		if ( sTarget_list [ i ].pTargetEntity == pOtherEntity )
		{
			sTarget_list.erase ( sTarget_list.begin( ) + i );
			i--;
		}
	}
}

void Entity::RemoveTarget ( Hero *pHero )
{
	for ( int i = 0; i < (int) sTarget_list.size ( ); i++ )
	{
		if ( sTarget_list [ i ].pTargetHero == pHero )
		{
			sTarget_list.erase ( sTarget_list.begin( ) + i );
			i--;
		}
	}
}

void Entity::UpdateTargets ( float fTimeDelta )
{
	CheckTargets ( );
	FindTargets ( );
	
	for ( int i = 0; i < (int) sTarget_list.size ( ); i++ )
	{
		float x, y, z;
		bool bTargetDucking = false;
		int iTargetContainer = pContainer->GetID( );

		switch ( sTarget_list [ i ].iTargetType )
		{
			case 0:		if ( !sTarget_list [ i ].pTargetHero )
						{
							sTarget_list.erase ( sTarget_list.begin( ) + i );
							i--;
							continue;
						}
						else
						{
							x = sTarget_list [ i ].pTargetHero->GetX( );
							y = sTarget_list [ i ].pTargetHero->GetY( ) + sTarget_list [ i ].pTargetHero->GetHeight( );
							z = sTarget_list [ i ].pTargetHero->GetZ( );
							bTargetDucking = sTarget_list [ i ].pTargetHero->GetIsDucking( );
							//bSameContainer = pContainer == sTarget_list [ i ].pTargetHero->GetContainer( );
							iTargetContainer = sTarget_list [ i ].pTargetHero->GetContainer( )->GetID( );
						}
						break;

			case 1:		if ( !sTarget_list [ i ].pTargetEntity )
						{
							sTarget_list.erase ( sTarget_list.begin( ) + i );
							i--;
							continue;
						}
						else
						{
							x = sTarget_list [ i ].pTargetEntity->GetX( );
							y = sTarget_list [ i ].pTargetEntity->GetY( ) + sTarget_list [ i ].pTargetEntity->GetHeight( );
							z = sTarget_list [ i ].pTargetEntity->GetZ( );
							bTargetDucking = sTarget_list [ i ].pTargetEntity->GetIsDucking( );
							//bSameContainer = pContainer == sTarget_list [ i ].pTargetEntity->GetContainer( );
							iTargetContainer = sTarget_list [ i ].pTargetEntity->GetContainer( )->GetID( );
						}
						break;

			default:	sTarget_list.erase ( sTarget_list.begin( ) + i );
						i--;
						continue;
		}

		bool bSameContainer = ( iTargetContainer == pContainer->GetID( ) );
		bool bCanSee = false;
				
		if ( pWorld->UsingGlobalVisibility( ) ) bCanSee = CanSee( x, y, z, bTargetDucking ) > 0;
		else bCanSee = (sTarget_list [ i ].bZoneTarget || bSameContainer ) && ( CanSee ( x, y, z, bTargetDucking ) > 0 );

		sTarget_list [ i ].bCanSee = bCanSee;

		float fDiffX = x - GetX(); 
		float fDiffY = y - GetY();
		float fDiffZ = z - GetZ();
		float fDist = fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ;

		if ( sTarget_list [ i ].bZoneTarget )
		{
			if ( bCanSee ) 
			{
				sTarget_list [ i ].fTargetThreat = sqrt(fDist);
				if ( sTarget_list [ i ].fTargetThreat > fRadius*99.0f ) sTarget_list [ i ].fTargetThreat = fRadius*99.0f;
			}
			else
			{
				sTarget_list [ i ].fTargetThreat = fRadius*99.9f;
			}

			sTarget_list [ i ].fTargetTimer = 60.0f;
			sTarget_list [ i ].vecLastPos = sTarget_list [ i ].vecPos;
			sTarget_list [ i ].vecPos.x = x;
			sTarget_list [ i ].vecPos.y = y;
			sTarget_list [ i ].vecPos.z = z;
			sTarget_list [ i ].vecGuessPos = sTarget_list [ i ].vecPos;
			sTarget_list [ i ].vecLastDir = sTarget_list [ i ].vecPos - sTarget_list [ i ].vecLastPos;
			sTarget_list [ i ].iContainer = iTargetContainer;
		}
		else
		{
			if ( bCanSee ) 
			{
				sTarget_list [ i ].fTargetThreat = sqrt(fDist);
				
				if ( bCanSearch && iAggressiveness == 1 ) sTarget_list [ i ].fTargetTimer = 20.0f;
				else sTarget_list [ i ].fTargetTimer = 6.0f + (rand()*1.0f)/RAND_MAX;

				sTarget_list [ i ].vecLastPos = sTarget_list [ i ].vecPos;
				sTarget_list [ i ].vecPos.x = x;
				sTarget_list [ i ].vecPos.y = y;
				sTarget_list [ i ].vecPos.z = z;
				sTarget_list [ i ].vecGuessPos = sTarget_list [ i ].vecPos;
				sTarget_list [ i ].vecLastDir = sTarget_list [ i ].vecPos - sTarget_list [ i ].vecLastPos;
				sTarget_list [ i ].iContainer = iTargetContainer;

				if ( sTarget_list [ i ].fTargetThreat > fRadius*99.0f ) sTarget_list [ i ].fTargetThreat = fRadius*99.0f;
			}
			else
			{
				if ( sTarget_list [ i ].fTargetThreat < fRadius*100.0f ) sTarget_list [ i ].fTargetThreat = fRadius*100.0f;
				sTarget_list [ i ].fTargetThreat += 0.01f;
				if ( sTarget_list [ i ].fTargetThreat > fRadius*1000.0f ) sTarget_list [ i ].fTargetThreat = fRadius*1000.0f;
			}
		}
	}

	for ( int i = 0; i < (int) sTarget_list.size ( ); i++ )
	{
		sTarget_list [ i ].fTargetTimer -= fTimeDelta;
		if ( sTarget_list [ i ].fTargetTimer <= 0 ) 
		{
			sTarget_list.erase ( sTarget_list.begin( ) + i );
			i--;
		}
	}

	sort ( sTarget_list.begin( ), sTarget_list.end( ) );

	bFireWeapon = false;
	if ( CanFire( ) && bCanAttack ) FireWeapon ( );
}

bool Entity::CanFire ( )
{	
	if ( !ValidTarget( ) ) return false;

	if ( !GetIsBehindCorner() )
	{	
		float x = GetTargetX( ) - GetX( );
		float z = GetTargetZ( ) - GetZ( );
		float fTargetAngY;

		if ( fabs ( x ) + fabs ( z ) < 0.1f ) 
		{
			return false;
		}
		else
		{
			fTargetAngY = z / sqrt ( x*x + z*z );
			fTargetAngY = acos ( fTargetAngY ) * RADTODEG;
			if ( x < 0.0f ) fTargetAngY = 360.0f - fTargetAngY;
		}
		
		float fArc = fabs ( fTargetAngY - GetAngleY( ) );
		if ( fArc > 180.0f ) fArc  = 360.0f - fArc;

		if ( fArc > fFireArc ) return false;
	}

	int iHeight = 1;
	bool bTargetDucking = false;

	if ( sTarget_list [ 0 ].iTargetType == 0 && sTarget_list [ 0 ].pTargetHero->GetIsDucking( ) ) bTargetDucking = true;
	if ( sTarget_list [ 0 ].iTargetType == 1 && sTarget_list [ 0 ].pTargetEntity->GetIsDucking( ) ) bTargetDucking = true;
	if ( GetIsDucking( ) || bTargetDucking ) iHeight = 2;

	return CanSee( GetTargetX( ), GetTargetY( ), GetTargetZ( ), bTargetDucking ) > 0;
	//return !( pContainer->pPathFinder->QuickPolygonsCheckVisible ( GetX( ), GetZ( ), GetTargetX( ), GetTargetZ( ), iHeight ) );
}

void Entity::SetGettingHit ( float fDirX, float fDirY, float fDirZ )
{
	bHit = true;

	float fDist = fDirX*fDirX + fDirZ*fDirZ;
	
	if ( fDist > 0.00001 )
	{
		fDist = sqrt ( fDist );

		sHitPoint sNewHitPoint;
		sNewHitPoint.x = -( fDirX / fDist );
		sNewHitPoint.y = 0.0f;
		sNewHitPoint.z = -( fDirZ / fDist );
		sNewHitPoint.angY = acos ( -fDirZ / fDist );
		sNewHitPoint.fLifeTime = 1.0f;

		sHitFrom_list.push_back ( sNewHitPoint );
	}

	sort ( sHitFrom_list.begin ( ), sHitFrom_list.end ( ) );
}

bool Entity::IsHit ( )
{
	//return ( bHit || CountHitPoints( ) > 0 );
	return bHit;
}

int Entity::CountHitPoints ( )
{
	return (int) sHitFrom_list.size ( );
}

GGVECTOR3 Entity::GetHitDir ( int iIndex )
{
	GGVECTOR3 vecDir ( 0.0f, 0.0f, 0.0f );
	
	if ( iIndex >= 0 && iIndex < (int) sHitFrom_list.size( ) )
	{
		vecDir.x = sHitFrom_list [ iIndex ].x;
		vecDir.y = sHitFrom_list [ iIndex ].y;
		vecDir.z = sHitFrom_list [ iIndex ].z;
	}

	return vecDir;
}

GGVECTOR3 Entity::GetHitDirAvg ( )
{
	GGVECTOR3 vecDir ( 0.0f, 0.0f, 0.0f );
	if ( CountHitPoints( ) == 0 ) return vecDir;

	vector < sHitPoint >::iterator hIter = sHitFrom_list.begin ( );
	vector < sHitPoint >::iterator hPrevIter = sHitFrom_list.end ( ) - 1;
	vector < sHitPoint >::iterator hBiggestIter = hIter;
	float fBiggestAngle = -1.0f;

	while ( hIter < sHitFrom_list.end ( ) )
	{
		float fHitAngY = hIter->angY - hPrevIter->angY;
		if ( fHitAngY <= 0.0f ) fHitAngY += 360.0f;

		if ( fHitAngY > fBiggestAngle )
		{
			fBiggestAngle = fHitAngY;
			hBiggestIter = hIter;
		}
		
		hPrevIter = hIter;
		hIter++;
	}

	if ( fBiggestAngle > 0.0f )
	{
		vecDir.x += hBiggestIter->x;
		vecDir.y += hBiggestIter->y;
		vecDir.z += hBiggestIter->z;

		if ( hBiggestIter == sHitFrom_list.begin ( ) ) hBiggestIter = sHitFrom_list.end ( ) - 1;
		else hBiggestIter--;

		vecDir.x += hBiggestIter->x;
		vecDir.y += hBiggestIter->y;
		vecDir.z += hBiggestIter->z;

		vecDir /= 2.0f;
	}
	
	if ( fBiggestAngle < 3.141592f )
	{
		vecDir = -vecDir;
	}

	float fDist = sqrt ( vecDir.x*vecDir.x + vecDir.z*vecDir.z );

	if ( fDist > 0.00001 ) vecDir /= fDist;
	else 
	{
		vecDir.x = 0.0f;
		vecDir.y = 0.0f;
		vecDir.z = 0.0f;
	}

	return vecDir;
}

float Entity::GetHitSpread ( )
{
	if ( (int) sHitFrom_list.size( ) == 0 ) return 0.0f;

	vector < sHitPoint >::iterator hIter = sHitFrom_list.begin ( );
	vector < sHitPoint >::iterator hPrevIter = sHitFrom_list.end ( ) - 1;
	vector < sHitPoint >::iterator hBiggestIter = hIter;
	float fBiggestAngle = -1.0f;

	while ( hIter < sHitFrom_list.end ( ) )
	{
		float fAngY = hIter->angY - hPrevIter->angY;
		if ( fAngY < 0.0f ) fAngY += 360.0f;

		if ( fAngY > fBiggestAngle )
		{
			fBiggestAngle = fAngY;
			hBiggestIter = hIter;
		}
	}
	
	if ( fBiggestAngle <= 0.0f ) return 0.0f;

	return 360.0f - fBiggestAngle;
}

void Entity::MarkOldPosition ( )
{
	vecOldPos.x = GetX ( );
	vecOldPos.y = GetY ( );
	vecOldPos.z = GetZ ( );
}

bool Entity::IsAllied ( Entity* pAskingEntity )
{
	return ( pTeam == pAskingEntity->pTeam );
}

bool Entity::IsEnemy ( Entity* pAskingEntity )
{
	return ( pTeam != pAskingEntity->pTeam );
}

bool Entity::IsNeutral ( )
{
	return ( pTeam == 0 );
}

bool Entity::IsFriendly ( )
{
	if ( !pTeam ) return false;
	return ( pTeam->GetTeamNum( ) == 1 );
}

void Entity::SetMaxSpeed ( float fNewSpeed )
{
	fSpeed = fNewSpeed;
}

void Entity::SetSpeed ( float fNewSpeed )
{
	if ( fNewSpeed < 0 ) fNewSpeed = 0;
	if ( fNewSpeed > fSpeed ) fNewSpeed = fSpeed;

	fCurrSpeed = fNewSpeed;
}

void Entity::SetTurnSpeed ( float fNewTurnSpeed )
{
	fTurnSpeed = fNewTurnSpeed;
}

void Entity::SetFireArc ( float fNewFireArc )
{
	if ( fNewFireArc < 0.0f ) fNewFireArc = 0.0f;
	if ( fNewFireArc > 180.0f ) fNewFireArc = 180.0f;
	
	fFireArc = fNewFireArc; 
}

void Entity::SetViewArc ( float fNewInnerViewArc, float fNewOuterViewArc )
{
	if ( fNewInnerViewArc < 0.0f ) fNewInnerViewArc = 0.0f;
	if ( fNewInnerViewArc > 180.0f ) fNewInnerViewArc = 180.0f;
	if ( fNewOuterViewArc > 180.0f ) fNewOuterViewArc = 180.0f;

	if ( fNewOuterViewArc < fNewInnerViewArc ) fNewOuterViewArc = fNewInnerViewArc;

	fInnerViewArc = fNewInnerViewArc;
	fOuterViewArc = fNewOuterViewArc;

	DebugHideViewArcs ( );
}

void Entity::SetViewRange ( float fNewViewRange )
{
	if ( fNewViewRange < 1.0f ) fNewViewRange = 1.0f;
	//if ( fNewViewRange > 100.0f ) fNewViewRange = 100.0f;

	fViewRange = fNewViewRange;

	DebugHideViewArcs ( );
}

void Entity::SetHearingRange ( float fNewHearingRange )
{
	if ( fNewHearingRange < 1.0f ) fNewHearingRange = 1.0f;

	fHearingRange = fNewHearingRange;

	DebugHideViewArcs ( );
}

void Entity::SetHearingThreshold ( int iNewThreshold )
{
	iHearingThreshold = iNewThreshold;
}

void Entity::SetNeutral ( bool bIsNeutral )
{
	if ( bIsNeutral )
	{
		if ( pTeam )
		{
			pTeam->RemoveMember ( this );
			pTeam = 0;
		}

		iAggressiveness = 3;
		bCanRoam = true;
	}
	else
	{
		iAggressiveness = 0;
	}
}

void Entity::SetDefendArea ( float fDist, int container )
{
	if ( fDist >= 0.0f ) fDefendDist = fDist;
	iAggressiveness = 0;
	bFollowingLeader = false;
	bDefending = true;

	iDefendContainer = container;

	SetIdlePos( vecDefendPos.x, vecDefendPos.y, vecDefendPos.z, container );
}

void Entity::SetDefendHero ( float fDist )
{
	if ( fDist >= 0.0f ) fDefendDist = fDist;
	iAggressiveness = 0;
	bFollowingLeader = true;
	bDefending = true;
}

bool Entity::InDefendArea ( float x, float y, float z )
{
	if ( iAggressiveness != 0 ) return false;
	
	float fDiffX = x - vecDefendPos.x;
	float fDiffY = y - vecDefendPos.y;
	float fDiffZ = z - vecDefendPos.z;
	float fDist = fDiffX*fDiffX + fDiffY*fDiffY + fDiffZ*fDiffZ;

	return ( fDist < fDefendDist*fDefendDist );
}

void Entity::SetDefendDist ( float fDist )
{
	if ( fDist < 0.0f ) fDist = 0.0f;

	fDefendDist = fDist;
}

float Entity::GetDefendDist ( )
{
	return fDefendDist;
}

void Entity::SetDefending ( bool bIsDefending )
{
	bDefending = bIsDefending;
	if ( !bDefending && bFollowingLeader ) bFollowingLeader = false;

	if ( !bDefending && iAggressiveness == 2 ) iAggressiveness = 0;
}

bool Entity::GetIsDefending ( )
{
	return bDefending;
}

void Entity::SetInvestigatePosToHit ( )
{
	if ( CountHitPoints( ) == 0 ) 
	{
		SetNoInvestigatePos( );
		return;
	}
	
	GGVECTOR3 vecDir = GetHitDir ( 0 ) * fRadius*4.0;
	float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX ( ), GetZ ( ), GetX ( ) + vecDir.x, GetZ ( ) + vecDir.z );

	if ( fResult >= 0.0f )
		vecDir *= fResult;
	
	vecInterestPos.x = GetX ( ) + vecDir.x;
	vecInterestPos.y = GetY ( );
	vecInterestPos.z = GetZ ( ) + vecDir.z;
}

void Entity::SetInvestigatePosToSound ( )
{
	vecInterestPos = vecSoundPos;
}

void Entity::SetNoInvestigatePos ( )
{
	vecInterestPos.x = GetX( );
	vecInterestPos.y = GetY( );
	vecInterestPos.z = GetZ( );
}

void Entity::ZoneEnterEvent ( Entity *pEntityIn )
{
	if ( pEntityIn->pTeam != pTeam && !pEntityIn->IsNeutral( ) ) 
	{
		AddTarget ( pEntityIn, true );
	}
}

void Entity::ZoneEnterEvent ( Hero *pHeroIn )
{
	if ( pHeroIn->pTeam != pTeam ) 
	{
		AddTarget ( pHeroIn, true );
	}
}

void Entity::ZoneLeaveEvent ( Entity *pEntityOut )
{
	RemoveTarget ( pEntityOut );
}

void Entity::ZoneLeaveEvent ( Hero *pHeroOut )
{
	RemoveTarget ( pHeroOut );
}

void Entity::SetFollowing ( bool bFollowing )
{
	bFollowingLeader = bFollowing;
	bDefending = bFollowing;
}

bool Entity::GetFollowing ( )
{
	return bFollowingLeader;
}

float Entity::GetMaxSpeed ( )
{
	return fSpeed;
}

float Entity::GetViewRange ( )
{
	return fViewRange;
}

void Entity::SetStateTimer ( float fNewTime )
{
	fStateTimer = fNewTime;
}

void Entity::SetInvestigateTimer ( float fNewTime )
{
	fInvestigateTimer = fNewTime;
}

void Entity::SetCoverTimer ( float fNewTime )
{
	fCoverTimer = fNewTime;
}

void Entity::SetSearchTimer ( float fNewTime )
{
	fSearchTimer = fNewTime;
}

float Entity::GetStateTimer ( )
{
	return fStateTimer;
}

float Entity::GetInvestigateTimer ( )
{
	return fInvestigateTimer;
}

float Entity::GetCoverTimer ( )
{
	return fCoverTimer;
}

float Entity::GetLookTimer ( )
{
	return fLookAroundTimer;
}

float Entity::GetResetTimer ( )
{
	return fResetTimer;
}

float Entity::GetSearchTimer ( )
{
	return fSearchTimer;
}

void Entity::SetAvgPatrolTime ( float fNewTime )
{
	if ( fNewTime < 0 ) fNewTime = 0;
	fAvgPatrolTime = fNewTime;
}

float Entity::GetAvgPatrolTime ( )
{
	return fAvgPatrolTime;
}

bool Entity::HeardSound ( )
{
	return bHeardSound;
}

int Entity::GetLastSoundUrgency ( )
{
	return iLastSoundUrgency;
}

//returns 0 - can't see, 1 - in peripheral vision, 2 - can see
//bGround defines if the point is low and therefore obstructed by half height objects
int Entity::CanSee ( float fX, float fY, float fZ, bool bGround )
{
	float fThisX = GetX ( );
	float fThisY = GetY ( );
	float fThisZ = GetZ ( );

	if ( bIsBehindCorner )
	{
		fThisX = fPeekX;
		fThisZ = fPeekY;
	}

	int iHeight = 1;
	if ( bIsDucking || bGround ) iHeight = 2;

	bool bHit = false;

	//choose old or new visibility checks
	//if ( pWorld->UsingGlobalVisibility( ) ) bHit = pWorld->GlobalVisibilityCheck( fThisX, fThisY + GetHeight( ), fThisZ, fX, fY, fZ, NULL );
	//else bHit = pContainer->pPathFinder->QuickPolygonsCheckVisible ( fThisX, fThisZ, fX, fZ, iHeight );
	// 090417 - always use new visibility (even if nothing in AI obstacle world - i.e. new level with simple terrain)
	bHit = pWorld->GlobalVisibilityCheck( fThisX, fThisY + GetHeight( ), fThisZ, fX, fY, fZ, NULL );

	int iViewResult = 0;

	if ( !bHit )
	{
		float fDirX = fX - fThisX;
		float fDirY = pWorld->UsingGlobalVisibility( ) ? fY - ( fThisY + GetHeight( ) ) : 0;
		float fDirZ = fZ - fThisZ;
		float fDist = sqrt ( fDirX*fDirX + fDirY*fDirY + fDirZ*fDirZ );

		if ( fDist > fViewRange )
		{
			return 0;
		}
		if ( fDist < fRadius*3 && iViewResult < 1 ) iViewResult = 1;

		fDirX /= fDist;
		fDirY /= fDist;
		fDirZ /= fDist;

		/*if ( pWorld->UsingGlobalVisibility( ) )
		{
			//check vertical view arc (fixed)
			float angx = asin ( fDirY ) * RADTODEG;

			if ( ( -45 < angx && angx < 35 ) && iViewResult < 2 ) iViewResult = 2;
			if ( ( -85 < angx && angx < 85 ) && iViewResult < 1 ) iViewResult = 1;
		}*/

		float angy = acos ( fDirZ ) * RADTODEG;
		if ( fDirX < 0.0f ) angy = 360.0f - angy;
		
		float fDiff = fabs ( GetAngleY( ) - angy );
		if ( fDiff > 180.0f ) fDiff = 360.0f - fDiff;

		if ( bIsBehindCorner ) iViewResult = 1;
		else
		{
			if ( fDiff < fInnerViewArc && iViewResult < 2 ) iViewResult = 2;
			if ( fDiff < fOuterViewArc && iViewResult < 1 ) iViewResult = 1;
		}
	}

		return iViewResult;
}

int Entity::CouldSee( float fX, float fY, float fZ, bool bGround )
{
	float fThisX = GetX ( );
	float fThisZ = GetZ ( );
	int iHeight = 1;
	if ( bIsDucking || bGround ) iHeight = 2;

	//choose old or new visibility checks
	//if ( pWorld->UsingGlobalVisibility( ) ) 
	//{
//		return pWorld->GlobalVisibilityCheck( fThisX, GetY( ) + GetHeight( ), fThisZ, fX, fY, fZ, NULL ) ? 0 : 1;
//	}
//	else 
	{
		return pContainer->pPathFinder->QuickPolygonsCheckVisible ( fThisX, fThisZ, fX, fZ, iHeight ) ? 0 : 1;
	}
}

int Entity::CanSee ( Entity* pOtherEntity )
{
	if ( !pOtherEntity ) return 0;
	if ( !pWorld->UsingGlobalVisibility() && pOtherEntity->GetContainer( ) != pContainer ) return 0;
	
	float fX = pOtherEntity->GetX( );
	float fY = pOtherEntity->GetY( ) + pOtherEntity->GetHeight( );
	float fZ = pOtherEntity->GetZ( );

	// allow entities to see entities hiding behind corners
	if ( pOtherEntity->GetIsBehindCorner() )
	{
		// get the average position
		fX = (pOtherEntity->GetPeekX()+pOtherEntity->GetX()) / 2;
		fY = (pOtherEntity->GetPeekY()+pOtherEntity->GetY()) / 2;
	}

	return this->CanSee ( fX, fY, fZ, pOtherEntity->GetIsDucking( ) );
}

int Entity::CanSee ( Hero* pHero )
{
	if ( !pHero ) return 0;
	if ( !pWorld->UsingGlobalVisibility() && pHero->GetContainer( ) != pContainer ) return 0;

	float fX = pHero->GetX ( );
	float fY = pHero->GetY ( ) + pHero->GetHeight( );
	float fZ = pHero->GetZ ( );

	return this->CanSee ( fX, fY, fZ, pHero->GetIsDucking( ) );
}

bool Entity::CanSeeTarget ( )
{
	if ( CountTargets( ) == 0 ) return false;

	switch ( sTarget_list [ 0 ].iTargetType )
	{
		case 0: return ( CanSee ( sTarget_list [ 0 ].pTargetHero ) > 0 ); break;
		case 1: return ( CanSee ( sTarget_list [ 0 ].pTargetEntity ) > 0 ); break;

		default: return false;
	}
}

void Entity::SetAttackDist( float fNewDist )
{
	if ( fNewDist < 0 ) fNewDist = 0;
	fMinimumDist = fNewDist;
}

void Entity::SetAvoidDist( float fNewDist )
{
	if ( fNewDist < 0 ) fNewDist = 0;
	fAvoidDist = fNewDist;
}

int Entity::GetAvoidMode( )
{
	return iEntityAvoidMode;
}

void Entity::SetAvoidMode( int mode )
{
	iEntityAvoidMode = mode;
}

void Entity::SetAlwaysActive( bool alwaysactiveflag )
{
	bAlwaysActive = alwaysactiveflag;
}

void Entity::SetPathStartCostLimit( float fNewDist )
{
	if ( fNewDist < 0 ) fNewDist = -1.0f;
	fPathStartCostLimit = fNewDist;
}

float Entity::GetPathStartCostLimit ( )
{
	return fPathStartCostLimit;
}

int Entity::IsClose ( float fDist )
{
	if ( fDist < fMinimumDist )
		return 1;

	return 0;
}

int Entity::IsCloseSqr ( float fDist )
{
	if ( fDist < fMinimumDist*fMinimumDist )
		return 1;

	return 0;
}

bool Entity::IsTooClose ( float fDist )
{
	if ( fDist < fAvoidDist )
		return 1;

	return 0;
}

bool Entity::IsTooCloseSqr ( float fDist )
{
	if ( fDist < fAvoidDist*fAvoidDist )
		return 1;

	return 0;
}

bool Entity::IsLookingAtObstacle ( float fDist )
{
	if ( pWorld->UsingGlobalVisibility( ) )
	{
		float fDirX = sin( GetAngleY( )*0.017453f ) * fRadius*10.0f;
		float fDirZ = cos( GetAngleY( )*0.017453f ) * fRadius*10.0f;

		float fX = GetX( );
		float fY = GetY( ) + GetHeight( );
		float fZ = GetZ( );

		float dist = -1;

		bool bHit = pWorld->GlobalVisibilityCheck( fX, fY, fZ, fX + fDirX, fY, fZ + fDirZ, &dist );

		if ( bHit )
		{
			dist = dist * ( fDirX*fDirX + fDirZ*fDirZ );
			if ( dist < fRadius*4 ) return true;	
		}
	}
	else
	{
		float fDirX = sin( GetAngleY( )*0.017453f ) * fRadius*3.0f;
		float fDirZ = cos( GetAngleY( )*0.017453f ) * fRadius*3.0f;

		if ( pContainer->pPathFinder->QuickPolygonsCheckVisible( GetX( ), GetZ( ), GetX( ) + fDirX, GetZ( ) + fDirZ, 1 ) ) return true;
	}

	return false;
}

void Entity::PredictTargetPosition ( float fPredDist )
{
	if ( CountTargets ( ) == 0 ) return;

	float pX = sTarget_list [ 0 ].vecLastPos.x;
	float pY = sTarget_list [ 0 ].vecLastPos.z;
	pContainer->pPathFinder->FindClosestOutsidePoint( &pX, &pY );

	sTarget_list [ 0 ].vecLastPos.x = pX;
	sTarget_list [ 0 ].vecLastPos.z = pY;
	
	GGVECTOR3 vecDir = sTarget_list [ 0 ].vecLastDir;
	//GGVECTOR3 vecDir ( GetTargetX ( ) - vecLastKnownPos.x,
	//					 GetTargetY ( ) - vecLastKnownPos.y, 
	//					 GetTargetZ ( ) - vecLastKnownPos.z );

	float fDist = sqrt ( vecDir.x*vecDir.x + vecDir.z*vecDir.z );

	if ( fDist > 0.0001f )
		vecDir /= fDist;

	sTarget_list [ 0 ].vecGuessPos = sTarget_list [ 0 ].vecLastPos + ( vecDir * fPredDist );

	bool bBlocked = pContainer->pPathFinder->QuickPolygonsCheck ( sTarget_list [ 0 ].vecLastPos.x, sTarget_list [ 0 ].vecLastPos.z, sTarget_list [ 0 ].vecGuessPos.x, sTarget_list [ 0 ].vecGuessPos.z, 2 );

	if ( bBlocked ) sTarget_list [ 0 ].vecGuessPos = sTarget_list [ 0 ].vecLastPos;

	//pEntity->vecLastKnownPos = pEntity->vecFinalDest;
	//pEntity->vecGuessNewPos = pEntity->vecLastKnownPos;
	sTarget_list [ 0 ].vecLastDir = vecDir;
}

void Entity::Stop ( )
{
	SetDestinationForced ( GetX( ), GetY( ), GetZ( ), -1, true );
}

void Entity::StopNoMoveAddition ( )
{
	SetDestinationForced ( GetX( ), GetY( ), GetZ( ), -1, false );
}

void Entity::LookAtDest ( )
{
	vecLookAt = vecFinalDest;
}

void Entity::LookAtTarget ( )
{
	vecLookAt.x = GetTargetX ( );
	vecLookAt.y = GetTargetY ( );
	vecLookAt.z = GetTargetZ ( );

	bLookAtPointSet = true;
}

void Entity::LookAtHideDir ( )
{
	vecLookAt.x = GetX() + fHideDirX;
	vecLookAt.y = GetY ( );
	vecLookAt.z = GetZ() + fHideDirY;

	bLookAtPointSet = true;
}

void Entity::LookAt ( float x, float z )
{
	vecLookAt.x = x;
	vecLookAt.z = z;

	bLookAtPointSet = true;
}

void Entity::MoveToTarget ( )
{
	SetDestination ( GetTargetX( ), GetTargetY( ), GetTargetZ( ), GetTargetContainer( ) );
}

void Entity::MoveToInterest ( )
{
	SetDestination ( vecInterestPos.x, vecInterestPos.y, vecInterestPos.z, iInterestContainer ); 
}

void Entity::MoveToGuessPos ( )
{
	if ( CountTargets( ) == 0 ) return;

	SetDestination ( sTarget_list [ 0 ].vecGuessPos.x, sTarget_list [ 0 ].vecGuessPos.y, sTarget_list [ 0 ].vecGuessPos.z, sTarget_list [ 0 ].iContainer );
}

void Entity::MoveToOriginalPos ( )
{
	SetDestination ( vecOrigPos.x, vecOrigPos.y, vecOrigPos.z, iOriginalContainer ); 
}

void Entity::MoveToDefendPos ( )
{
	SetDestination ( vecDefendPos.x, vecDefendPos.y, vecDefendPos.z, iDefendContainer ); 
}

void Entity::MoveToPatrolPos ( )
{
	CheckPath ( );
	if ( !pPatrolPath ) return;

	int iNumPoints = pPatrolPath->CountPoints( );
	if ( iNumPoints <= 0 ) return;

	if ( iCurrentPatrolPoint < 0 ) iCurrentPatrolPoint = 0;
	if ( iCurrentPatrolPoint >= iNumPoints ) iCurrentPatrolPoint = 0;

	SetDestination ( pPatrolPath->GetPoint ( iCurrentPatrolPoint ).x, GetY( ), pPatrolPath->GetPoint ( iCurrentPatrolPoint ).y, pPatrolPath->GetPoint ( iCurrentPatrolPoint ).container );

	if ( GetSqrDistToDest( ) < 0.1 || ( GetSqrDistToDest( ) < fRadius*fRadius*16.0f && ( !bMakingProgress && pWorld->RandInt( 2 ) == 0 ) ) )
	{
		iCurrentPatrolPoint++;
		//if ( iCurrentPatrolPoint >= iNumPoints ) iCurrentPatrolPoint = 0;
		SetStateTimer ( ( ( (rand()*1.0f) / RAND_MAX ) - 0.5f ) + fAvgPatrolTime );

		if ( iCurrentPatrolPoint >= iNumPoints ) iCurrentPatrolPoint = 0;
		SetDestination ( pPatrolPath->GetPoint ( iCurrentPatrolPoint ).x, GetY( ), pPatrolPath->GetPoint ( iCurrentPatrolPoint ).y, pPatrolPath->GetPoint ( iCurrentPatrolPoint ).container );

		//Dave
		if ( bRedoPath == false )
		{
			int n = 0;
			int iMaxInList = (int)ListOfEntitiesToUpdateMovement.size()-1;
			for (; n <= iMaxInList; n++ )
				if ( ListOfEntitiesToUpdateMovement[n] == this->iID )
					break;
			if ( n > iMaxInList )
				ListOfEntitiesToUpdateMovement.push_back(this->iID);
			bRedoPath = true;
		}
		//Dave fPathTimer = 0;
	}
}

void Entity::MoveAwayFromHit ( )
{
	GGVECTOR3 vecDir = GetHitDirAvg( );
	float fX = GetX( ) - vecDir.x * fRadius * 10.0f;
	float fZ = GetZ( ) - vecDir.z * fRadius * 10.0f;
	int iRandDir = ( rand( ) % 2 )*2 - 1;	//-1 or 1
	fX = fX + vecDir.x * fRadius * 3.0f * iRandDir;
	fZ = fZ - vecDir.z * fRadius * 3.0f * iRandDir;

	pContainer->pPathFinder->FindClosestOutsidePoint ( &fX, &fZ );

	SetDestination ( fX, GetY( ), fZ, -1 );
}

void Entity::MoveAwayFromSound ( )
{
	float fDirX = vecInterestPos.x - GetX( );
	float fDirZ = vecInterestPos.z - GetZ( );
	float fDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );

	fDirX /= fDist;
	fDirZ /= fDist;

	float fX = GetX( ) - fDirX * fRadius * 10.0f;
	float fZ = GetZ( ) - fDirZ * fRadius * 10.0f;
	int iRandDir = ( rand( ) % 2 )*2 - 1;	//-1 or 1
	fX = fX + fDirZ * fRadius * 3.0f * iRandDir;
	fZ = fZ - fDirX * fRadius * 3.0f * iRandDir;

	pContainer->pPathFinder->FindClosestOutsidePoint ( &fX, &fZ );

	SetDestination ( fX, GetY( ), fZ, -1 );
}

bool Entity::MoveToCover ( )
{
	if ( !pContainer ) return false;

	//bool bCover = pContainer->pPathFinder->QuickPolygonsCheckVisible ( GetX ( ), GetZ ( ), vecInterestPos.x, vecInterestPos.z, 2 );

	bool bCover = CouldSee( vecInterestPos.x, vecInterestPos.y, vecInterestPos.z, 0 ) == 0;

	float fDiffX = vecInterestPos.x - GetX( );
	float fDiffZ = vecInterestPos.z - GetZ( );
	float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;

	if ( !bCover || fDist < fRadius*fRadius*9 )
	{
		Path cPath;
		// find half height first, if not then full height
		pContainer->pPathFinder->GetClosestWaypoints ( GetX ( ), GetZ ( ), fViewRange / 4.0f, &cPath, true );

		int iNumPoints = cPath.CountPoints ( );
		if ( iNumPoints == 0 )
			pContainer->pPathFinder->GetClosestWaypoints ( GetX ( ), GetZ ( ), fViewRange / 4.0f, &cPath, false );

		iNumPoints = cPath.CountPoints ( );
		if ( iNumPoints == 0 )
		{
			//no where to go
			//MoveAwayFromSound( );
			return false;
		}

		float fClosest = -1;
		int iIndex = 0;
		
		for ( int i = 0; i < iNumPoints; i++ )
		{
			float fX = cPath.GetPoint(i).x;
			float fY = GetY();	//not accurate on non-level containers
			float fZ = cPath.GetPoint(i).y;
			bool bHasCover = false;

			if ( pWorld->UsingGlobalVisibility( ) ) bHasCover = pWorld->GlobalVisibilityCheck( fX, fY, fZ, vecInterestPos.x, vecInterestPos.y, vecInterestPos.z, NULL );
			else bHasCover = pContainer->pPathFinder->QuickPolygonsCheckVisible ( fX, fZ, vecInterestPos.x, vecInterestPos.z, 2 );
						
			float fDiffX2 = cPath.GetPoint(i).x - GetX( );
			float fDiffZ2 = cPath.GetPoint(i).y - GetZ( );
			
			float fRange = fDiffX2*fDiffX2 + fDiffZ2*fDiffZ2;
			float fDotP = fDiffX*fDiffX2 + fDiffZ*fDiffZ2;
			if ( fDotP > 0 ) fRange += fRadius*fRadius*9;
			
			if ( bHasCover && ( fClosest < 0 || fRange < fClosest ) )
			{
				fClosest = fRange;
				iIndex = i;
			}
		}

		SetDestination ( cPath.GetPoint(iIndex).x, GetY(), cPath.GetPoint(iIndex).y, -1 );
		
		return true;
	}
	else
	{
		//if ( !IsTurning() ) LookAt( vecInterestPos.x , vecInterestPos.z );
		return false;
	}
}

void Entity::MoveToIdlePos ( )
{
	SetDestination ( vecOrigPos.x, vecOrigPos.y, vecOrigPos.z, iOriginalContainer );
}

void Entity::MoveTowards ( float x, float z, float dist, int container )
{
	float fDiffX = x - GetX( );
	float fDiffZ = z - GetZ( );
	float fDist = dist / sqrt ( fDiffX*fDiffX + fDiffZ*fDiffZ );

	x = GetX( ) + fDiffX * fDist;
	z = GetZ( ) + fDiffZ * fDist;

	SetDestination ( x, GetY( ), z, container );
}

void Entity::SearchArea ( )
{
	if ( !bManualControl ) ChangeState ( pWorld->pEntityStates->pStateSearchArea );
}

void Entity::LookAround ( float fStart, float fEnd )
{
	if ( !IsTurning ( ) )
	{
		if ( fStart < 0.0f ) fStart = 0.0f;
		if ( fEnd > 180.0f ) fEnd = 180.0f;
		if ( fEnd < fStart ) fEnd = fStart;

		float fRange = fEnd - fStart;
		float fRandNum = ( fRange * rand( ) ) / RAND_MAX + fStart;
		if ( rand( ) % 2 == 0 ) fRandNum = -fRandNum;
		
		float fNewAngY = ( GetAngleY ( ) + fRandNum ) * DEGTORAD;
		
		vecLookAt.x = GetX ( ) + sin ( fNewAngY )*100.0f;
		vecLookAt.z = GetZ ( ) + cos ( fNewAngY )*100.0f;

		bLookAtPointSet = true;
	}
}

void Entity::SetNoLookPoint ( )
{
	bLookAtPointSet = false;
}

void Entity::SweepViewForward ( float fMaxAng )
{	
	if ( !IsTurning ( ) )
	{
		if ( fMaxAng > 180.0f ) fMaxAng = 180.0f;
		if ( fMaxAng < 0.0f ) fMaxAng = 0.0f;

		float x = vecCurrDest.x - GetX ( );
		float z = vecCurrDest.z - GetZ ( );
		float fNewAngY;

		if ( fabs ( x ) + fabs ( z ) < 0.1f ) 
		{
			LookAround ( 0.0f, fMaxAng );
			return;
		}
		else
		{
			fNewAngY = z / sqrt ( x*x + z*z );
			fNewAngY = acos ( fNewAngY ) * RADTODEG;
			if ( x < 0.0f ) fNewAngY = 360.0f - fNewAngY;
		}

		float fRandNum = ( fMaxAng * rand( ) ) / RAND_MAX;
		if ( rand( ) % 2 == 0 ) fRandNum = -fRandNum;
		
		fNewAngY = ( fNewAngY + fRandNum ) * DEGTORAD;
		
		vecLookAt.x = GetX ( ) + sin ( fNewAngY )*100.0f;
		vecLookAt.z = GetZ ( ) + cos ( fNewAngY )*100.0f;

		bLookAtPointSet = true;
	}
}

void Entity::RandomMove ( float fMin, float fMax )
{
	if ( fMin < 0.0f ) fMin = 0.0f;
	//if ( fMax > 100.0f ) fMax = 100.0f;
	if ( fMax < fMin ) fMax = fMin;

	fMax = fMax - fMin;

	float fAng = (6.2831853f * rand ( )) / RAND_MAX;
	float fRange = (fMax * rand ( )) / RAND_MAX + fMin;

	GGVECTOR3 vecPos ( sin( fAng ) * fRange, 0.0f, cos( fAng ) * fRange );
	float fDist = pContainer->pPathFinder->FindClosestPolygon ( GetX( ), GetZ( ), GetX( ) + vecPos.x, GetZ( ) + vecPos.z );

	if ( fDist >= 0.0f ) vecPos *= fDist-0.05f;
	
	SetDestination ( GetX ( ) + vecPos.x, GetY ( ), GetZ ( ) + vecPos.z, -1 );
}

void Entity::MoveClose ( float fDestX, float fDestY, float fDestZ, float fRange, int container )
{
	if ( fRange < 0.0f ) fRange = 0.0f;
	float fMinRange = fRadius*2;
	if ( fMinRange > fRange ) fMinRange = fRange;
	fRange = fRange - fMinRange;
	
	int iIterations = 0;
	
	float fVX, fVZ, fDist, fAng;

	//do
	//{
		fDist = ( fRange * rand( ) ) / RAND_MAX + fMinRange;
		fAng = ( 360.0f * DEGTORAD * rand( ) ) / RAND_MAX;

		fVX = sin( fAng )*fDist;
		fVZ = cos( fAng )*fDist;

		Container *pDestContainer = pContainer;
		if ( container >= 0 ) pDestContainer = pWorld->GetContainer( container );

		float fResult = pDestContainer->pPathFinder->FindClosestPolygon ( fDestX, fDestZ, fDestX + fVX, fDestZ + fVZ );

		//pContainer->pPathFinder->FindClosestOutsidePoint ( &fVX, &fVZ );

	//	iIterations++;
	//} while ( iIterations < 10 && ( fResult >= 0.0f && fResult*fDist < 1.0f ) );
	
	if ( fResult > 0 )
	{
		fResult -= 0.01f;
		fVX *= fResult;
		fVZ *= fResult;
	}

	SetDestination ( fDestX + fVX, fDestY, fDestZ + fVZ, container );
	//SetDestination ( fVX, GetY( ), fVZ );
}

void Entity::Duck ( )
{
	if ( !bCanDuck ) return;
	bIsDucking = true;
}

void Entity::Stand ( )
{
	bIsDucking = false;
}

void Entity::StrafeTarget ( )
{
	if ( !bCanStrafe ) return;
	if ( CountTargets( ) <= 0 ) return;

	GGVECTOR3 vecDir ( GetTargetX( ) - GetX ( ), 
						 GetTargetY( ) - GetY ( ),
						 GetTargetZ( ) - GetZ ( ) );

	float fDist = sqrt ( vecDir.x*vecDir.x + vecDir.z*vecDir.z );
	if ( fDist > 0.00001 ) vecDir /= fDist;
	
	int iRandDir = ( ( rand( ) % 2 ) * 2 ) - 1;				//-1 or 1
	float fRandDist = fRadius * ( ( 4.0f*rand ( ) ) / RAND_MAX + 2.0f );

	float fDirX =  iRandDir * vecDir.z * fRandDist;
	float fDirZ = -iRandDir * vecDir.x * fRandDist;

	float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX ( ), GetZ ( ), GetX ( ) + fDirX, GetZ ( ) + fDirZ );

	if ( fResult >= 0.01f )
	{
		fResult -= 0.01f;
		fDirX *= fResult;
		fDirZ *= fResult;
	}

	SetDestination ( GetX ( ) + fDirX, GetY ( ), GetZ ( ) + fDirZ, -1 );
}

void Entity::StrafeAvoid ( )
{
	if ( !bCanStrafe ) return;
	
	GGVECTOR3 vecDir ( GetTargetX ( ) - GetX ( ), 
						 GetTargetY ( ) - GetY ( ),
						 GetTargetZ ( ) - GetZ ( ) );

	float fDist = sqrt ( vecDir.x*vecDir.x + vecDir.z*vecDir.z );

	if ( fDist > 0.00001 ) vecDir /= fDist;

	
	float fDirX = GetTargetDirX( );
	float fDirZ = GetTargetDirZ( );
	
	float fDirDist = sqrt ( fDirX*fDirX + fDirZ*fDirZ );
	if ( fDirDist > 0.00001 ) { fDirX /= fDirDist; fDirZ /= fDirDist; }

	float fRel = ( fDirX * vecDir.z ) - ( fDirZ * vecDir.x );

	if ( fRel > 0.0f )
	{
		fDirX = -vecDir.z;
		fDirZ = vecDir.x;
	}
	else
	{
		fDirX = vecDir.z;
		fDirZ = -vecDir.x;
	}
	
	if ( fDirDist > 0.00001 )
	{
		fDirX = fDirX*(fDist) - vecDir.x*(fAvoidDist - fDist + 1.0f);
		fDirZ = fDirZ*(fDist) - vecDir.z*(fAvoidDist - fDist + 1.0f);
	}
	else
	{
		fDirX = -vecDir.x*(fAvoidDist - fDist + 1.0f);
		fDirZ = -vecDir.z*(fAvoidDist - fDist + 1.0f);
	}

	float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX ( ), GetZ ( ), GetX ( ) + fDirX, GetZ ( ) + fDirZ );

	if ( fResult >= 0.0f )
	{
		fResult -= 0.01f;
		fDirX *= fResult;
		fDirZ *= fResult;
	}

	SetDestination ( GetX ( ) + fDirX, GetY( ), GetZ ( ) + fDirZ, -1 );
}

void Entity::CalculateAvoidPosition ( float fDist )
{
	/*
	if ( cMovePath.CountPoints ( ) > 2 )
	{
		int iIndex = pContainer->pPathFinder->GetClosestWaypoint ( cMovePath.GetPoint ( 1 ).x, cMovePath.GetPoint ( 1 ).y );
		pContainer->pPathFinder->DeActivateWaypoint ( iIndex );

		pContainer->pPathFinder->CalculatePath ( GetX ( ), GetZ ( ), vecFinalDest.x, vecFinalDest.z, &cMovePath );
		
		if ( cMovePath.CountPoints ( ) >= 2 )
		{
			vecAvoidPos.x = cMovePath.GetPoint ( 1 ).x;
			vecAvoidPos.z = cMovePath.GetPoint ( 1 ).y;
			vecAvoidPos.y = GetY ( );
			return;
		}
	}*/
	
	if ( pWorld->RandInt( 4 ) == 0 )
	{
		if ( rand( ) % 2 == 0 ) bAvoidLeft = true;
		else bAvoidLeft = false;
	}

	GGVECTOR3 vecDir;
	vecDir.x = vecCurrDest.x - GetX ( );
	vecDir.y = vecCurrDest.y - GetY ( );
	vecDir.z = vecCurrDest.z - GetZ ( );

	float fLength = vecDir.x*vecDir.x + vecDir.z*vecDir.z;
	
	if ( fLength > 0.001f ) vecDir *= ( fDist / sqrt(fLength) );
	else 
	{
		vecDir.x = sin ( GetAngleY ( ) * DEGTORAD ) * fDist;
		vecDir.z = cos ( GetAngleY ( ) * DEGTORAD ) * fDist;
	}

	float fSwap = vecDir.x;

	if ( bAvoidLeft )
	{
		vecDir.x = -vecDir.z;
		vecDir.z = fSwap;
	}
	else
	{
		vecDir.x = vecDir.z;
		vecDir.z = -fSwap;
	}

	float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX( ), GetZ( ), GetX( ) + vecDir.x, GetZ( ) + vecDir.z );
	
	if ( fResult >= 0.0f )
	{
		if ( fResult < 0.1f && rand( ) % 3 == 0 ) bAvoidLeft = !bAvoidLeft;
		if ( fResult > 0.01f ) fResult -= 0.01f;
		vecDir *= fResult;
	}

	vecAvoidPos.x = GetX ( ) + vecDir.x;
	vecAvoidPos.z = GetZ ( ) + vecDir.z;
	vecAvoidPos.y = GetY ( );
}

void Entity::TurnToAngle ( float fTimeDelta, bool bMoving )
{
	// work out speed of turn and distance to look at destination
	float fCurrTurnSpeed = fTurnSpeed * fTimeDelta;
	float x = vecCurrLookAt.x - GetX ( );
	float z = vecCurrLookAt.z - GetZ ( );
	float fDist = sqrt ( fabs ( x*x ) + fabs ( z*z ) );
	if ( fDist < 1.0f )
	{
		// leave last angle in tact (would continue along last control path direction)
		return;
	}
	else
	{
		fDestAngY = z / sqrt ( x*x + z*z );
		fDestAngY = acos ( fDestAngY ) * RADTODEG;
		if ( x < 0.0f ) fDestAngY = 360.0f - fDestAngY;
	}
	
	// 210918 - decouple forcing angle to object (so script can smooth it)
	//if ( pObject ) YRotateObject ( iID, fDestAngY );
	//else fAngY = fDestAngY;
	fAngY = fDestAngY;
	/*
	float fDifference = fabs ( fDestAngY - GetAngleY( ) );
	if ( fDifference > 180.0f ) fDifference = 360.0f - fDifference;
	if ( fDifference <= fCurrTurnSpeed || bMoving == false )
	{
		// 080517 - when movement slowed/stopped, use actual object angle for 
		if ( GetVisible ( dwObjectNumberRef ) == 1 )
			fAngY = ObjectAngleY(dwObjectNumberRef);
	}
	else
	{
		// need to report exact angle as this function not called quickly enough for gradual de/increments of angle
		float fDir = sin ( ( fDestAngY - GetAngleY ( ) ) * DEGTORAD );
		if ( fDir > 0.0f )
		{
			if ( pObject ) YRotateObject ( iID, GetAngleY ( ) + fCurrTurnSpeed );
			else fAngY += fCurrTurnSpeed;
		}
		else
		{
			if ( pObject ) YRotateObject ( iID, GetAngleY ( ) - fCurrTurnSpeed );
			else fAngY -= fCurrTurnSpeed;
		}
	}
	*/
	fLookAroundTimer = ( (float) rand( ) ) / RAND_MAX + 1.0f;
}

void Entity::FireWeapon ( )
{
	bFireWeapon = true;
}

void Entity::ResetFeedbackPos ( )
{
	fMoveFeedbackTimer = 0.3f;
	vecLastRecPos.x = GetX ( );
	vecLastRecPos.y = GetY ( );
	vecLastRecPos.z = GetZ ( );
}

void Entity::UpdateFeedbackPos ( )
{
	GGVECTOR3 vecDir, vecDestDir;

	vecDir.x = GetX ( ) - vecLastRecPos.x;
	vecDir.y = GetY ( ) - vecLastRecPos.y;
	vecDir.z = GetZ ( ) - vecLastRecPos.z;

	fDistanceMoved = sqrt ( vecDir.x*vecDir.x + vecDir.z*vecDir.z );
	
	ResetFeedbackPos ( );
	
	if ( cMovePath.CountPoints( ) <= 2 )
	{
		vecDestDir.x = vecFinalDest.x - GetX ( );
		vecDestDir.y = vecFinalDest.y - GetY ( );
		vecDestDir.z = vecFinalDest.z - GetZ ( );
	}
	else
	{
		vecDestDir.x = vecCurrDest.x - GetX ( );
		vecDestDir.y = vecCurrDest.y - GetY ( );
		vecDestDir.z = vecCurrDest.z - GetZ ( );
	}

	float fLength = vecDestDir.x*vecDestDir.x + vecDestDir.z*vecDestDir.z;
	float fDotP = vecDestDir.x*vecDir.x + vecDestDir.z*vecDir.z;

	bStuck = fDistanceMoved < fCurrSpeed / 12.0;
	bMakingProgress = fDotP >= 0.0f && fLength > 0.01f && !bStuck;
}

bool Entity::DirectionBlocked ( float fDirX, float fDirZ )
{
	if ( sCombinedObstacleAngle_list.size( ) < 1 ) return false;
	if ( sCombinedObstacleAngle_list.size( ) == 1 ) 
	{
		if ( sCombinedObstacleAngle_list [ 0 ].fAngEnd - sCombinedObstacleAngle_list [ 0 ].fAngBegin >= 359.9999f ) return true;
	}

	float fDist = fDirX*fDirX + fDirZ*fDirZ;
	if ( fDist < 0.00001f ) return false;
	fDist = sqrt ( fDist );

	float fDirAngle = acos ( fDirZ / fDist ) * RADTODEG;
	if ( fDirX < 0 ) fDirAngle = 360.0f - fDirAngle;

	return DirectionBlocked ( fDirAngle );
}

bool Entity::DirectionBlocked ( float fDir )
{
	if ( sCombinedObstacleAngle_list.size( ) < 1 ) return false;
	if ( sCombinedObstacleAngle_list.size( ) == 1 ) 
	{
		if ( sCombinedObstacleAngle_list [ 0 ].fAngEnd - sCombinedObstacleAngle_list [ 0 ].fAngBegin >= 359.9999f ) return true;
	}
	
	for ( int i = 0; i < (int) sCombinedObstacleAngle_list.size( ); i++ )
	{
		if ( fDir > sCombinedObstacleAngle_list [ i ].fAngBegin && fDir < sCombinedObstacleAngle_list [ i ].fAngEnd ) return true;
		if ( fDir + 360.0f > sCombinedObstacleAngle_list [ i ].fAngBegin && fDir + 360.0f < sCombinedObstacleAngle_list [ i ].fAngEnd ) return true;
	}

	return false;
}

bool Entity::AdjustDirection ( float *pDir )
{
	// quit and ignore avoidance if nothing to avoid
	if ( sCombinedObstacleAngle_list.size( ) < 1 ) return false;

	// if only one obstacle to avoid
	if ( sCombinedObstacleAngle_list.size( ) == 1 ) 
	{
		// but the angles are way out, ignore avoidance 
		if ( sCombinedObstacleAngle_list [ 0 ].fAngEnd - sCombinedObstacleAngle_list [ 0 ].fAngBegin >= 359 ) return false;
	}

	// for bots with multiple obstacles to avoid
	for ( int i = 0; i < (int) sCombinedObstacleAngle_list.size( ); i++ )
	{
		// check if present direction enters an arc that leads to another AI bot
		if ( *pDir > sCombinedObstacleAngle_list [ i ].fAngBegin && *pDir < sCombinedObstacleAngle_list [ i ].fAngEnd ) 
		{
			// work out which side of the AI bot we should aim for to get out of its way
			float fDiff = ( sCombinedObstacleAngle_list [ i ].fAngEnd + sCombinedObstacleAngle_list [ i ].fAngBegin ) / 2.0f;
			if ( *pDir < fDiff ) *pDir = sCombinedObstacleAngle_list [ i ].fAngBegin;
			else *pDir = sCombinedObstacleAngle_list [ i ].fAngEnd;

			// mark as true so this avoidance is carried out
			return true;
		}

		// as above but in case direction angle is a negative (<0)
		if ( *pDir + 360.0f > sCombinedObstacleAngle_list [ i ].fAngBegin && *pDir + 360.0f < sCombinedObstacleAngle_list [ i ].fAngEnd ) 
		{
			// work out which side of the AI bot we should aim for to get out of its way
			float fDiff = ( sCombinedObstacleAngle_list [ i ].fAngEnd + sCombinedObstacleAngle_list [ i ].fAngBegin ) / 2.0f;
			if ( *pDir + 360.0f < fDiff ) *pDir = sCombinedObstacleAngle_list [ i ].fAngBegin;
			else *pDir = sCombinedObstacleAngle_list [ i ].fAngEnd;

			// mark as true so this avoidance is carried out
			return true;
		}
	}

	// no avoidance event today!
	return false;
}

void Entity::AdjustDestination ( )
{
	if ( fAdjustDestinationTimer > 0 ) return;

	pWorld->pTeamController->StartGlobalEntityIterator ( );
	Entity *pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );

	//vecLastDest = vecCurrDest;

	bool bAdjusted = false;

	while ( pOtherEntity )
	{
		if ( pOtherEntity == this || pOtherEntity->GetContainer( ) != pContainer )
		{
			pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
			continue;
		}

		float fDiffX = vecCurrDest.x - pOtherEntity->GetX( );
		float fDiffZ = vecCurrDest.z - pOtherEntity->GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
		float fTotalRadius = pOtherEntity->GetRadius( ) + fRadius*1.1f;

		if ( fDist < fTotalRadius*fTotalRadius )
		{
			float fNewDestX;
			float fNewDestZ;

			if ( fDist > 0.0001f )
			{	
				fDist = fTotalRadius / sqrt( fDist );
				fNewDestX = pOtherEntity->GetX( ) + (fDiffX * fDist);
				fNewDestZ = pOtherEntity->GetZ( ) + (fDiffZ * fDist);
			}
			else
			{
				fDiffX = vecCurrDest.x - GetX( );
				fDiffZ = vecCurrDest.z - GetZ( );
				fDist = 1.0f / sqrt ( fDiffX*fDiffX + fDiffZ*fDiffZ );
				fDiffX *= fDist;
				fDiffZ *= fDist;
				
				fNewDestX = pOtherEntity->GetX( ) - (fDiffX * fTotalRadius) - fDiffZ*0.1f;
				fNewDestZ = pOtherEntity->GetZ( ) - (fDiffZ * fTotalRadius) + fDiffX*0.1f;
			}

			vecCurrDest.x = fNewDestX;
			vecCurrDest.z = fNewDestZ;

			bAdjusted = true;
		}

		pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
	}

	Hero *pHero = pWorld->pTeamController->GetPlayer( );

	if ( pHero && pHero->GetContainer( ) == pContainer )
	{
		float fDiffX = vecCurrDest.x - pHero->GetX( );
		float fDiffZ = vecCurrDest.z - pHero->GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
		float fTotalRadius = pHero->GetRadius( ) + fRadius*1.1f;

		if ( fDist < fTotalRadius*fTotalRadius )
		{
			float fNewDestX;
			float fNewDestZ;

			if ( fDist > 0.0001f )
			{	
				fDist = fTotalRadius / sqrt( fDist );
				fNewDestX = pHero->GetX( ) + (fDiffX * fDist);
				fNewDestZ = pHero->GetZ( ) + (fDiffZ * fDist);
			}
			else
			{
				fDiffX = vecCurrDest.x - GetX( );
				fDiffZ = vecCurrDest.z - GetZ( );
				fDist = 1.0f / sqrt ( fDiffX*fDiffX + fDiffZ*fDiffZ );
				fDiffX *= fDist;
				fDiffZ *= fDist;
				
				fNewDestX = pHero->GetX( ) - (fDiffX * fTotalRadius) - fDiffZ*0.1f;
				fNewDestZ = pHero->GetZ( ) - (fDiffZ * fTotalRadius) + fDiffX*0.1f;
			}

			vecCurrDest.x = fNewDestX;
			vecCurrDest.z = fNewDestZ;

			bAdjusted = true;
		}
	}

	if ( bAdjusted )
	{
		fAdjustDestinationTimer = 1.5f;
	}
}

void Entity::ClearSounds ( )
{
	bHeardSound = false;
	fClosestSound = -1.0f;
	iSoundUrgency = 0;
}

void Entity::CheckBeacons ( Beacon *pBeacon )
{	
	while ( pBeacon )
	{
		//if ( !pBeacon->GetContainer( ) || pBeacon->GetContainer( ) == pContainer )
		{
			if ( pBeacon->IsSound ( ) && bCanHear )
			{
				if ( pBeacon->iExtraInfo >= iHearingThreshold )
				{
					GGVECTOR3 vecDir;
					vecDir.x = pBeacon->vecPos.x - GetX ( );
					vecDir.y = pBeacon->vecPos.y - GetY ( );
					vecDir.z = pBeacon->vecPos.z - GetZ ( );

					float fDist = vecDir.x*vecDir.x + vecDir.z*vecDir.z;

					float fRange = fHearingRange + pBeacon->fSoundSize;

					if ( fDist < fRange*fRange && fDist > fRadius*fRadius*9 )
					{
						if ( pBeacon->iExtraInfo > iSoundUrgency || ( ( fClosestSound < 0 || fDist < fClosestSound ) && pBeacon->iExtraInfo == iSoundUrgency ) ) 
						{
							fClosestSound = fDist;
							iSoundUrgency = pBeacon->iExtraInfo;
							iLastSoundUrgency = pBeacon->iExtraInfo;
							vecInterestPos = pBeacon->vecPos;
							iInterestContainer = pBeacon->pContainer ? pBeacon->pContainer->GetID() : -1;
							bHeardSound = true;
						}
					}
				}
			}
		}

		pBeacon = pBeacon->pNextBeacon;
	}
}

void Entity::CheckZones ( Zone *pZone )
{
	while ( pZone )
	{
		bool bInNow = pZone->InZone ( GetX( ), GetZ( ), GetContainer( ) );
		bool bInLast = pZone->InZone ( vecLastPos.x, vecLastPos.z, GetContainer( ) );

		if ( bInNow ) pZone->NotifyEnter ( this );
		if ( bInLast && !bInNow ) pZone->NotifyLeave ( this );

		pZone = pZone->pNextZone;
	}
}

void Entity::UpdateObstacleListCountDown ( float fTimeDelta )
{
	// go through all AI bots recorded in OA list and removes old ones
	for ( int i = 0; i < (int) sObstacleAngle_list.size( ); i++ )
	{
		sObstacleAngle_list [ i ].fTimer -= fTimeDelta;
		if ( sObstacleAngle_list [ i ].fTimer <= 0.0f )
		{
			sObstacleAngle_list.erase ( sObstacleAngle_list.begin( ) + i );
			i--;
		}
	}
}

void Entity::UpdateObstacleList ( float fTimeDelta )
{
	// scans all AI bots and works out if they get too close
	// to each other or the player 

	// go through all AI bots
	pWorld->pTeamController->StartGlobalEntityIterator ( );
	Entity *pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
	while ( pOtherEntity )
	{
		// ignore owner of search and bots in other containers
		if ( pOtherEntity == this || pOtherEntity->GetContainer( ) != pContainer )
		{
			pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
			continue;
		}

		// work out distance of the two bots
		float fDiffX = pOtherEntity->GetX( ) - GetX( );
		float fDiffZ = pOtherEntity->GetZ( ) - GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
		float radius = fRadius + pOtherEntity->GetRadius( );
		if ( fDist < (radius*radius) ) // (8*radius*radius) )
		{
			// if close enough, work out angle limits to the other bot
			fDist = sqrt ( fDist );
			float fEntAng = acos ( fDiffZ / fDist ) * RADTODEG;
			if ( fDiffX < 0 ) fEntAng = 360.0f - fEntAng;
			float fSegAng;
			if ( fDist <= fRadius + pOtherEntity->GetRadius( ) ) fSegAng = 70.0f;
			else fSegAng = asin ( ( fRadius + pOtherEntity->GetRadius( ) ) / fDist ) * RADTODEG;
			if ( fSegAng >= 179.99f ) fSegAng = 179.99f;
			float fBeginAngle = fEntAng - fSegAng;
			if ( fBeginAngle < 0 ) fBeginAngle += 360.0f;
			if ( fBeginAngle > 360 ) fBeginAngle -= 360.0f;
			float fEndAngle = fBeginAngle + fSegAng*2.0f;

			// search if the other bot is already in the OA list
			bool bFound = false;
			for ( int i = 0; i < (int) sObstacleAngle_list.size( ) - 1; i++ )
			{
				if ( sObstacleAngle_list [ i ].pEntity == pOtherEntity )
				{
					bFound = true;
					sObstacleAngle_list [ i ].fAngBegin = fBeginAngle;
					sObstacleAngle_list [ i ].fAngEnd = fEndAngle;
					sObstacleAngle_list [ i ].fTimer = 0.4f;
				}
			}
			if ( !bFound && fDist < (2*radius) )
			{
				// if not in OA list and close enough to the other bot
				sObstacleAngle sNewObstacleAngle;
				sNewObstacleAngle.fAngBegin = fBeginAngle;
				sNewObstacleAngle.fAngEnd = fEndAngle;
				sNewObstacleAngle.fTimer = 0.4f;
				sNewObstacleAngle.pEntity = pOtherEntity;
				sObstacleAngle_list.push_back ( sNewObstacleAngle );
			}
		}

		// next AI bot to check against
		pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
	}

	// check if bot is too close to the main player
	/* Allow AI bots UP CLOSE to player (neded for close-combat melee and physics will push player)
	Hero *pHero = pWorld->pTeamController->GetPlayer( );
	if ( pHero && pHero->GetContainer( ) == pContainer )
	{
		float fDiffX = pHero->GetX( ) - GetX( );
		float fDiffZ = pHero->GetZ( ) - GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
		float radius = fRadius + pHero->GetRadius( );
		if ( fDist > 0.000001f && fDist < (8*radius*radius) )
		{
			// if close enough, work out angle limits to the player
			fDist = sqrt ( fDist );
			float fEntAng = acos ( fDiffZ / fDist ) * RADTODEG;
			if ( fDiffX < 0 ) fEntAng = 360.0f - fEntAng;
			float fSegAng;
			if ( fDist <= fRadius + pHero->GetRadius( ) ) fSegAng = 70.0f;
			else fSegAng = asin ( ( fRadius + pHero->GetRadius( ) ) / fDist ) * RADTODEG;
			if ( fSegAng >= 179.99f ) fSegAng = 179.99f;
			float fBeginAngle = fEntAng - fSegAng;
			if ( fBeginAngle < 0 ) fBeginAngle += 360.0f;
			if ( fBeginAngle > 360 ) fBeginAngle -= 360.0f;
			float fEndAngle = fBeginAngle + fSegAng*2.0f;

			// if not in OA list and close enough to the other bot
			bool bFound = false;
			for ( int i = 0; i < (int) sObstacleAngle_list.size( ) - 1; i++ )
			{
				if ( sObstacleAngle_list [ i ].pEntity == NULL )
				{
					bFound = true;
					sObstacleAngle_list [ i ].fAngBegin = fBeginAngle;
					sObstacleAngle_list [ i ].fAngEnd = fEndAngle;
					sObstacleAngle_list [ i ].fTimer = 0.5f;
				}
			}
			if ( !bFound && fDist < (2*radius) )
			{
				sObstacleAngle sNewObstacleAngle;
				sNewObstacleAngle.fAngBegin = fBeginAngle;
				sNewObstacleAngle.fAngEnd = fEndAngle;
				sNewObstacleAngle.fTimer = 0.5f;
				sNewObstacleAngle.pEntity = NULL;
				sObstacleAngle_list.push_back ( sNewObstacleAngle );
			}
		}
	}
	*/

	// if we have a container to respect
	if ( pContainer )
	{
		// get any blockers stored for container
		const Blocker *pBlocker = pContainer->pPathFinder->GetBlockerList( );
		while ( pBlocker )
		{
			float fDiffX = pBlocker->x - GetX( );
			float fDiffZ = pBlocker->z - GetZ( );
			float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
			float radius = fRadius + pBlocker->radius;
			if ( fDist > 0.000001f && fDist < (8*radius*radius) )
			{
				// if close enough, work out angle limits to the blocker
				fDist = sqrt ( fDist );
				float fEntAng = acos ( fDiffZ / fDist ) * RADTODEG;
				if ( fDiffX < 0 ) fEntAng = 360.0f - fEntAng;
				float fSegAng;
				if ( fDist <= radius ) fSegAng = 70.0f;
				else fSegAng = asin ( ( radius ) / fDist ) * RADTODEG;
				if ( fSegAng >= 179.99f ) fSegAng = 179.99f;
				float fBeginAngle = fEntAng - fSegAng;
				if ( fBeginAngle < 0 ) fBeginAngle += 360.0f;
				if ( fBeginAngle > 360 ) fBeginAngle -= 360.0f;
				float fEndAngle = fBeginAngle + fSegAng*2.0f;
				
				// if not in OA list and close enough to the blocker
				bool bFound = false;
				for ( int i = 0; i < (int) sObstacleAngle_list.size( ) - 1; i++ )
				{
					if ( sObstacleAngle_list [ i ].pEntity == (Entity*) pBlocker )
					{
						bFound = true;
						sObstacleAngle_list [ i ].fAngBegin = fBeginAngle;
						sObstacleAngle_list [ i ].fAngEnd = fEndAngle;
						sObstacleAngle_list [ i ].fTimer = 0.4f;
					}
				}
				if ( !bFound && fDist < (2*radius) )
				{
					sObstacleAngle sNewObstacleAngle;
					sNewObstacleAngle.fAngBegin = fBeginAngle;
					sNewObstacleAngle.fAngEnd = fEndAngle;
					sNewObstacleAngle.fTimer = 0.4f;
					sNewObstacleAngle.pEntity = (Entity*) pBlocker;
					sObstacleAngle_list.push_back ( sNewObstacleAngle );
				}
			}

			// next container blocker in list
			pBlocker = pBlocker->pNextBlocker;
		}
	}

	// now sort the OA list
	sort ( sObstacleAngle_list.begin( ), sObstacleAngle_list.end( ) );

	// create a new combined OA list from any surviving/added items above (but miss out last one)
	sCombinedObstacleAngle_list.clear ( );
	//for ( int i = 0; i < (int) sObstacleAngle_list.size( ) - 1; i++ )  / chopping off the item which ants to avoid is bad!
	for ( int i = 0; i < (int) sObstacleAngle_list.size( ); i++ ) 
	{
		sCombinedObstacleAngle_list.push_back( sObstacleAngle_list [ i ] );
	}

	// determine if we should remove items inside other arcs, or make it all one big 360 degree arc
	if ( sCombinedObstacleAngle_list.size( ) > 1 )
	{
		bool bComplete = false;

		// remove items that have their hit-arcs entirely within another
		int iNext = 1;
		for ( int i = 0; i < (int) sCombinedObstacleAngle_list.size( ) - 1; i++ )
		{
			iNext = i + 1;
			if ( sCombinedObstacleAngle_list [ i ].fAngEnd > sCombinedObstacleAngle_list [ iNext ].fAngBegin )
			{
				if ( sCombinedObstacleAngle_list [ i ].fAngEnd < sCombinedObstacleAngle_list [ iNext ].fAngEnd )
				{
					sCombinedObstacleAngle_list [ i ].fAngEnd = sCombinedObstacleAngle_list [ iNext ].fAngEnd;
					if ( sCombinedObstacleAngle_list [ i ].fAngEnd - sCombinedObstacleAngle_list [ i ].fAngBegin >= 360.0f ) 
						bComplete = true;
				}
				sCombinedObstacleAngle_list.erase ( sCombinedObstacleAngle_list.begin( ) + iNext );
				i--;
			}
		}

		// remove items that have their hit-arcs entirely within another (for angles which are negative (<0))
		int iLast = (int) sCombinedObstacleAngle_list.size( ) - 1;
		float fBeginAngle = sCombinedObstacleAngle_list [ 0 ].fAngBegin + 360.0f;
		float fEndAngle = sCombinedObstacleAngle_list [ 0 ].fAngEnd + 360.0f;
		if ( sCombinedObstacleAngle_list [ iLast ].fAngEnd > fBeginAngle )
		{
			if ( sCombinedObstacleAngle_list [ iLast ].fAngEnd < fEndAngle )
			{
				sCombinedObstacleAngle_list [ iLast ].fAngEnd = fEndAngle;
				if ( sCombinedObstacleAngle_list [ iLast ].fAngEnd - sCombinedObstacleAngle_list [ iLast ].fAngBegin >= 360.0f ) 
					bComplete = true;
			}
			sCombinedObstacleAngle_list.erase ( sCombinedObstacleAngle_list.begin( ) );
		}

		// if mark as complete, reduce while list down to one item which repells everything (arc 0-360)
		if ( bComplete )
		{
			sCombinedObstacleAngle_list.clear ( );
			sObstacleAngle sNewObstacleAngle;
			sNewObstacleAngle.fAngBegin = 0.0f;
			sNewObstacleAngle.fAngEnd = 360.0f;
			sCombinedObstacleAngle_list.push_back ( sNewObstacleAngle );
		}
	}
}

void Entity::UpdateTimers ( float fTimeDelta )
{
	if ( fStateTimer > 0.0f )				fStateTimer -= fTimeDelta;
	if ( fLookAroundTimer > 0.0f )			fLookAroundTimer -= fTimeDelta;
	if ( fInvestigateTimer > 0.0f )			fInvestigateTimer -= fTimeDelta;
	if ( fCoverTimer > 0.0f )				fCoverTimer -= fTimeDelta;
	if ( fMoveFeedbackTimer > 0.0f )		fMoveFeedbackTimer -= fTimeDelta;
	if ( fAvoidTimer > 0.0f )				fAvoidTimer -= fTimeDelta;
	if ( fPathTimer > 0.0f )				fPathTimer -= fTimeDelta;
	if ( fForceUpdatePathTimer > 0.0f )		fForceUpdatePathTimer -= fTimeDelta;
	if ( fResetTimer > 0.0f )				fResetTimer -= fTimeDelta;
	if ( fWaitTimer > 0.0f )				fWaitTimer -= fTimeDelta;	
	if ( fExpensiveUpdateTimer > 0.0f )		fExpensiveUpdateTimer -= fTimeDelta;
	if ( fAdjustDestinationTimer > 0.0f )	fAdjustDestinationTimer -= fTimeDelta;
	if ( fAdjustDirectionTimer > 0.0f )		fAdjustDirectionTimer -= fTimeDelta;
	if ( fSearchTimer > 0.0f )				fSearchTimer -= fTimeDelta;
	if ( fObstacleTimer > 0.0f )			fObstacleTimer -= fTimeDelta;
}

void Entity::RebuildDynamicPathFinder( )
{
	//DynamicPathFinder *pDynamicPathFinder = new DynamicPathFinder( pContainer->pPathFinder );
	if ( !pDynamicPathFinder ) pDynamicPathFinder = new DynamicPathFinder( pContainer->pPathFinder );
	pDynamicPathFinder->ClearObjects( );
	pDynamicPathFinder->ClearWaypoints( );

	pWorld->pTeamController->StartGlobalEntityIterator ( );
	Entity *pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );

	float fTotalRadius = 0;

	while ( pOtherEntity )
	{
		if ( pOtherEntity == this || pOtherEntity->GetContainer( ) != pContainer )
		{
			pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
			continue;
		}
		
		float fDiffX = pOtherEntity->GetX( ) - GetX( );
		float fDiffZ = pOtherEntity->GetZ( ) - GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;

		fTotalRadius = fRadius + pOtherEntity->GetRadius();

		if ( fDist < fTotalRadius*fTotalRadius*25 )
		{
			AvoidanceSphere *pColShape = new AvoidanceSphere( fTotalRadius, pOtherEntity->GetX( ), pOtherEntity->GetZ( ) );
			pDynamicPathFinder->AddObject( pColShape, false );
		}

		pOtherEntity = pWorld->pTeamController->GetNextGlobalEntity ( );
	}

	Hero *pHero = pWorld->pTeamController->GetPlayer( );
	
	if ( pHero && pHero->GetContainer( ) == pContainer )
	{
		float fDiffX = pHero->GetX( ) - GetX( );
		float fDiffZ = pHero->GetZ( ) - GetZ( );
		float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;

		fTotalRadius = pHero->GetRadius( )+fRadius;

		if ( fDist < fRadius*fRadius*25 )
		{
			AvoidanceSphere *pColShape = new AvoidanceSphere( fTotalRadius, pHero->GetX( ), pHero->GetZ( ) );
			pDynamicPathFinder->AddObject( pColShape, false );
		}
	}

	pDynamicPathFinder->CompleteObstacles( );
	pDynamicPathFinder->BuildWaypoints( );

	//delete pDynamicPathFinder;
}

void Entity::ForceMove( float x, float y )
{
	SetDestination( x, GetY(), y, pContainer->GetID() );
	cMovePath.Clear();
	cMovePath.AddPoint( GetX(), 0, GetY() );
	cMovePath.AddPoint( x, 0, y );
	iCurrentMovePoint = 1;
}

void Entity::UpdateMovement ( float fTimeDelta )
{
	// skip entirely if object no longer exists
	if ( CheckObjectExist ( dwObjectNumberRef ) == false )
		return;

	// cap time delta for any timeelapsed based calcs
	if ( fTimeDelta > 1.0f ) fTimeDelta = 1.0f;

	// manage avoid mode value
	int l_iAvoidMode = iEntityAvoidMode;
	if ( l_iAvoidMode < 0 ) l_iAvoidMode = iAvoidMode;

	// determine automated speed for full and crawl stance
	fCurrSpeed = fSpeed;
	if ( GetIsDucking ( ) ) fCurrSpeed = fSpeed / 2.0f;

	// if container does not exist, nowhere to move
	if ( !pWorld->ContainerExist ( pContainer ) ) pContainer = 0;
	if ( !pContainer || !pContainer->IsActive ( ) ) return;

	// entity-entity avoidance (modes 0,1,2,5)
	if ( (l_iAvoidMode < 3 || l_iAvoidMode == 5) )
	{
		// countdown any timers (each cycle)
		UpdateObstacleListCountDown ( fTimeDelta );

		// ensure expensive process done only occasionally
		if ( fExpensiveUpdateTimer <= 0.0f  )
		{
			// calculate obstacle list this cycle
			UpdateObstacleList ( fTimeDelta );

			// set random timer for avoidance mode
			fExpensiveUpdateTimer = ( 0.05f * rand() ) / RAND_MAX + 0.2f;
		}
	}

	// only when final destination changes do we re-assess final destination for point 'outside of obstacles'
	/* not using final dest
	float fDX = vecLastFinalDest.x - vecFinalDest.x;
	float fDZ = vecLastFinalDest.z - vecFinalDest.z;
	float fDD = sqrt(fabs(fDX*fDX)+abs(fDZ*fDZ));
	if ( fDD >= 0.0001f )
	{
		if ( pContainer->pPathFinder->InPolygons ( vecFinalDest.x, vecFinalDest.z ) > 0 )
		{
			pContainer->pPathFinder->FindClosestOutsidePoint ( &(vecFinalDest.x), &(vecFinalDest.z) );
		}
		vecLastFinalDest = vecFinalDest;
	}
	*/

	// deliberately commented out?
	/// AdjustDestination ( );

	// can show obstacle angles in debug (future feature?) - seems to stop AI bots
	//DebugHideObstacleAngles ( );
	//if ( bShowAvoidanceAngles )
	//{
	//	DebugDrawObstacleAngles ( );
	//}

	// work out usable per cycle speed based on time elapsed delta
	float fTimedSpeed = fCurrSpeed * fTimeDelta;

	// feedback position tells us whether we are walking on the spot - not using for now
	///if ( !bMoving ) ResetFeedbackPos ( );	
	///if ( fMoveFeedbackTimer <= 0.0f ) UpdateFeedbackPos ( );

	// calculate path as often as fPathTimer allows (intensive calc)
	bool bUpdateMovement = false;
	if ( g_LeeThread.GetWorkInProgress() == -1 )
	{
		if ( ListOfEntitiesToUpdateMovement.size() > 0 )
		{
			// go through all entities and work out who gets to update their path this cycle (only one per cycle in turn)
			//if ( ListOfEntitiesToUpdateMovementIndex > (int)ListOfEntitiesToUpdateMovement.size()-1 ) ListOfEntitiesToUpdateMovementIndex = 0;
			//if ( ListOfEntitiesToUpdateMovementIndex <= (int)ListOfEntitiesToUpdateMovement.size()-1 )
			int ListOfEntitiesToUpdateMovementIndex = 0; // always take first (oldest) one first (erase when done with it)
			{
				if ( ListOfEntitiesToUpdateMovement[ListOfEntitiesToUpdateMovementIndex] == this->iID )
				{
					if (((bRedoPath ) && !bChangingContainers && iAggressiveness != 2 && !bIsLeaping && !bIsDiving) ) 
					{
						// only allow if spacing time has elapsed
						CurrentAIWorkedPathTimer -= fTimeDelta;
						if ( CurrentAIWorkedPathTimer <= 0.0f )
						{
							CurrentAIWorkedPathTimer = 0.1f; // X second until next inc.
							bUpdateMovement = true;
						}
					}
				}
			}
		}
	}

	// when trigger path to be updated, start a thread task
	if ( bUpdateMovement == true )
	{
		// new path must start from here, and is also final pos if no path routed
		vecFinalDest.x = GetX( );
		vecFinalDest.y = GetY( );
		vecFinalDest.z = GetZ( );

		// if stored final dest in "pre_", restore it
		if ( pre_vecFinalDest.x != -999 )
		{
			vecFinalDest.x = pre_vecFinalDest.x;
			vecFinalDest.y = pre_vecFinalDest.y;
			vecFinalDest.z = pre_vecFinalDest.z;
			vecLastDest.x = pre_vecLastDest.x;
			vecLastDest.y = pre_vecLastDest.y;
			vecLastDest.z = pre_vecLastDest.z;
		}
		pre_vecFinalDest.x = -999;
		pre_vecLastDest.x = -999;

		// remove any old paths (about to create a new one)
		cMovePath.DebugHide ( );
		DebugHideDestination ( );
		ListOfEntitiesToUpdateMovement.erase (ListOfEntitiesToUpdateMovement.begin()+0);
		bRedoPath = false;

		// thread task issue
		g_LeeThread.BeginWork( this->iID, GetX(), GetZ(), vecFinalDest.x, vecFinalDest.y, vecFinalDest.z, pContainer, iDestContainer );
	}
	if ( g_LeeThread.GetWorkInProgress() == this->iID )
	{
		if ( g_LeeThread.GetWorkComplete() == 1 )
		{
			// ready for new path and set timers for next think-time
			fForceUpdatePathTimer = 0.0f;
			fPathTimer = 0.75f;

			// get new path from path calculation done in thread
			Path cNewPath = g_LeeThread.GetNewPath();
			g_LeeThread.GetNewFinalDest ( &vecFinalDest.x, &vecFinalDest.y, &vecFinalDest.z );
			g_LeeThread.EndWork();

			/*
			// prepare level obstacles for new path calculation
			Path cNewPath;
			pContainer->pPathFinder->ActivateAllWaypoints ( );
			pContainer->pPathFinder->CalculatePath ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z, &cNewPath, fPathStartCostLimit, iDestContainer );

			// if no path to finaldest, work from finaldest back to known reachable point
			if ( cNewPath.CountPoints() == 0 )
			{
				// okay, so final dest not pathable, and no direct line to barrier
				// so find closest node from container, and set that as finaldest
				int iThisContainer = GetContainer()->GetID(); 
				iDestContainer = iThisContainer; // container ID can be changed below
				float fBestDistance = 999999.0f;
				float fBestX = 0.0f;
				float fBestZ = 0.0f;
				float fDX = GetX( ) - vecFinalDest.x;
				float fDZ = GetZ( ) - vecFinalDest.z;
				float fDD = sqrt ( fabs(fDX*fDX) + fabs(fDZ*fDZ) );
				float fDDInc = fDD / 30.0f;
				int iDistCount = (int)fDDInc;
				for ( int iDist = 1; iDist <= 30; iDist++ )
				{
					for ( int iAng = 0; iAng < 360; iAng+=45 )
					{
						// this is where I want the AI to try to get to
						float fTryX = vecFinalDest.x + (sin(iAng*DEGTORAD)*(iDist*fDDInc));
						float fTryZ = vecFinalDest.z + (cos(iAng*DEGTORAD)*(iDist*fDDInc));
						int waypointmax = waypoint_getmax();
						for ( int waypointindex = 1; waypointindex <= waypointmax; waypointindex++ )
						{
							int tokay = waypoint_ispointinzoneex ( waypointindex, fTryX, vecFinalDest.y, fTryZ, 1 );
							if ( tokay == 1 ) 
							{
								iDestContainer = waypointindex;
								fBestDistance = 0;
								fBestX = fTryX;
								fBestZ = fTryZ;
								iDist = 31;
								iAng = 361;
								break;
							}
						}
					}
				}
				if ( fBestDistance != 999999.0f )
				{
					// best is within waypoint zone, but outside AI obstacle zone (margin added to AI waypoint system)
					// so project away from player to ensure we get inside
					float fDX = fBestX - vecFinalDest.x;
					float fDZ = fBestZ - vecFinalDest.z;
					float fDD = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
					fDX = (fDX/fDD)*(fDD+30.0f);
					fDZ = (fDZ/fDD)*(fDD+30.0f);
					fBestX = vecFinalDest.x + fDX;
					fBestZ = vecFinalDest.z + fDZ;

					// now assign final pos
					if ( iDestContainer != iThisContainer )
					{
						// found closer position in another container, so just go there
						vecFinalDest.x = fBestX;
						vecFinalDest.z = fBestZ;
					}
					else
					{
						// same container, so dont get too close to edge when move nearer target
						float fPushX = fBestX - vecFinalDest.x;
						float fPushZ = fBestZ - vecFinalDest.z;
						float fPushDD = sqrt ( fabs(fPushX*fPushX) + fabs(fPushZ*fPushZ) );
						fPushX /= fPushDD;
						fPushZ /= fPushDD;
						vecFinalDest.x = fBestX + (fPushX*2.0f);
						vecFinalDest.z = fBestZ + (fPushZ*2.0f);
					}
				}
				else
				{
					if ( GetContainer()->GetID() == 0 )
					{
						float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z );
						if ( fResult >= 0.1f ) 
						{
							iDestContainer = GetContainer()->GetID(); 
							vecFinalDest.x = GetX( ) + ( vecFinalDest.x - GetX( ) )*fResult;
							vecFinalDest.z = GetZ( ) + ( vecFinalDest.z - GetZ( ) )*fResult;
						}
					}
				}

				// chart new path to this permimeter position
				cNewPath.Clear ( );
				pContainer->pPathFinder->ActivateAllWaypoints ( );
				pContainer->pPathFinder->CalculatePath ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z, &cNewPath, fPathStartCostLimit, iDestContainer );
				if ( cNewPath.CountPoints( ) == 2 )
				{
					// if path has zero length, reduce to a single node
					if ( fabs ( cNewPath.GetPoint( 0 ).x - cNewPath.GetPoint( 1 ).x ) < 5.0f && fabs ( cNewPath.GetPoint( 0 ).y - cNewPath.GetPoint( 1 ).y ) < 5.0f )
						cNewPath.RemoveLast();
				}
			}
			*/

			// create move path from above result
			cMovePath.Clear ( );
			if ( cNewPath.CountPoints( ) > 0 )
			{
				// make new path the move path
				cMovePath = cNewPath;
				iCurrentMovePoint = 1;
				bChangingContainers = false;
				if ( cNewPath.CountPoints( ) > 1 )
				{
					if ( cMovePath.GetPoint( 0 ).container != cMovePath.GetPoint( 1 ).container ) 
						bChangingContainers = true;
				}
			}
	
			// ensure new move path is from the new fresh one (and show if in debug)
			cMovePath.DebugHide ( );
			DebugHideDestination ( );
			if ( bShowPaths ) 
			{
				cMovePath.DebugDraw ( fRadius );
				DebugDrawDestination ( );
			}
		}
	}

	// and ensure this expensive task is only done once per fPathTimer
	/* new thread code
	if ( bUpdateMovement && fPathTimer <= 0.0f )
	{
		// new path must start from here, and is also final pos if no path routed
		vecFinalDest.x = GetX( );
		vecFinalDest.y = GetY( );
		vecFinalDest.z = GetZ( );

		// if stored final dest in "pre_", restore it
		if ( pre_vecFinalDest.x != -999 )
		{
			vecFinalDest.x = pre_vecFinalDest.x;
			vecFinalDest.y = pre_vecFinalDest.y;
			vecFinalDest.z = pre_vecFinalDest.z;
			vecLastDest.x = pre_vecLastDest.x;
			vecLastDest.y = pre_vecLastDest.y;
			vecLastDest.z = pre_vecLastDest.z;
		}
		pre_vecFinalDest.x = -999;
		pre_vecLastDest.x = -999;

		// remove any old paths (about to create a new one)
		cMovePath.DebugHide ( );
		DebugHideDestination ( );
		ListOfEntitiesToUpdateMovement.erase (ListOfEntitiesToUpdateMovement.begin()+0);

		// prepare level obstacles for new path calculation
		Path cNewPath;
		pContainer->pPathFinder->ActivateAllWaypoints ( );
		pContainer->pPathFinder->CalculatePath ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z, &cNewPath, fPathStartCostLimit, iDestContainer );
		bRedoPath = false;
		fForceUpdatePathTimer = 0.0f;
		fPathTimer = 0.75f; // 070317 - faster zombies require higher fidelity to prevent stop start

		// if no path to finaldest, work from finaldest back to known reachable point
		if ( cNewPath.CountPoints() == 0 )
		{
			// okay, so final dest not pathable, and no direct line to barrier
			// so find closest node from container, and set that as finaldest
			int iThisContainer = GetContainer()->GetID(); 
			iDestContainer = iThisContainer; // container ID can be changed below
			float fBestDistance = 999999.0f;
			float fBestX = 0.0f;
			float fBestZ = 0.0f;
			float fDX = GetX( ) - vecFinalDest.x;
			float fDZ = GetZ( ) - vecFinalDest.z;
			float fDD = sqrt ( fabs(fDX*fDX) + fabs(fDZ*fDZ) );
			float fDDInc = fDD / 30.0f;
			int iDistCount = (int)fDDInc;
			for ( int iDist = 1; iDist <= 30; iDist++ )
			{
				for ( int iAng = 0; iAng < 360; iAng+=45 )
				{
					// this is where I want the AI to try to get to
					float fTryX = vecFinalDest.x + (sin(iAng*DEGTORAD)*(iDist*fDDInc));
					float fTryZ = vecFinalDest.z + (cos(iAng*DEGTORAD)*(iDist*fDDInc));
					int waypointmax = waypoint_getmax();
					for ( int waypointindex = 1; waypointindex <= waypointmax; waypointindex++ )
					{
						int tokay = waypoint_ispointinzoneex ( waypointindex, fTryX, vecFinalDest.y, fTryZ, 1 );
						if ( tokay == 1 ) 
						{
							iDestContainer = waypointindex;
							fBestDistance = 0;
							fBestX = fTryX;
							fBestZ = fTryZ;
							iDist = 31;
							iAng = 361;
							break;
						}
					}
				}
			}
			if ( fBestDistance != 999999.0f )
			{
				// best is within waypoint zone, but outside AI obstacle zone (margin added to AI waypoint system)
				// so project away from player to ensure we get inside
				float fDX = fBestX - vecFinalDest.x;
				float fDZ = fBestZ - vecFinalDest.z;
				float fDD = sqrt ( fabs(fDX*fDX)+fabs(fDZ*fDZ) );
				fDX = (fDX/fDD)*(fDD+30.0f);
				fDZ = (fDZ/fDD)*(fDD+30.0f);
				fBestX = vecFinalDest.x + fDX;
				fBestZ = vecFinalDest.z + fDZ;

				// now assign final pos
				if ( iDestContainer != iThisContainer )
				{
					// found closer position in another container, so just go there
					vecFinalDest.x = fBestX;
					vecFinalDest.z = fBestZ;
				}
				else
				{
					// same container, so dont get too close to edge when move nearer target
					float fPushX = fBestX - vecFinalDest.x;
					float fPushZ = fBestZ - vecFinalDest.z;
					float fPushDD = sqrt ( fabs(fPushX*fPushX) + fabs(fPushZ*fPushZ) );
					fPushX /= fPushDD;
					fPushZ /= fPushDD;
					vecFinalDest.x = fBestX + (fPushX*2.0f);
					vecFinalDest.z = fBestZ + (fPushZ*2.0f);
				}
			}
			else
			{
				if ( GetContainer()->GetID() == 0 )
				{
					float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z );
					if ( fResult >= 0.1f ) 
					{
						iDestContainer = GetContainer()->GetID(); 
						vecFinalDest.x = GetX( ) + ( vecFinalDest.x - GetX( ) )*fResult;
						vecFinalDest.z = GetZ( ) + ( vecFinalDest.z - GetZ( ) )*fResult;
					}
				}
			}

			// chart new path to this permimeter position
			cNewPath.Clear ( );
			pContainer->pPathFinder->ActivateAllWaypoints ( );
			pContainer->pPathFinder->CalculatePath ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z, &cNewPath, fPathStartCostLimit, iDestContainer );
			if ( cNewPath.CountPoints( ) == 2 )
			{
				// if path has zero length, reduce to a single node
				if ( fabs ( cNewPath.GetPoint( 0 ).x - cNewPath.GetPoint( 1 ).x ) < 5.0f && fabs ( cNewPath.GetPoint( 0 ).y - cNewPath.GetPoint( 1 ).y ) < 5.0f )
					cNewPath.RemoveLast();
			}
		}

		// create move path from above result
		cMovePath.Clear ( );
		if ( cNewPath.CountPoints( ) > 0 )
		{
			// make new path the move path
			cMovePath = cNewPath;
			iCurrentMovePoint = 1;
			bChangingContainers = false;
			if ( cNewPath.CountPoints( ) > 1 )
			{
				if ( cMovePath.GetPoint( 0 ).container != cMovePath.GetPoint( 1 ).container ) 
					bChangingContainers = true;
			}
		}
	}
	IN A THREAD END*/
	
	// set intermediate destination based on path and avoidance data
	if ( bAvoiding && !bChangingContainers && !bIsLeaping && !bIsDiving )
	{
		// above code can create an avoidance position to get around something when stuck
		vecCurrDest.x = vecAvoidPos.x;
		vecCurrDest.y = vecAvoidPos.y;
		vecCurrDest.z = vecAvoidPos.z;
	}
	else
	{
		if ( cMovePath.CountPoints( ) > 2 ) 
		{
			// No longer using 'unit perfect' movement to get to node, we depend on LUA/Anim movement steps
			// so work on distance from start of path to end of path, and work the 'complete path' based on that
			bool bFurtherThanDistanceOfPath = false;
			float lastpointx = cMovePath.GetPoint( iCurrentMovePoint-1 ).x;
			float lastpointy = cMovePath.GetPoint( iCurrentMovePoint-1 ).y;
			float thispointx = cMovePath.GetPoint( iCurrentMovePoint ).x;
			float thispointy = cMovePath.GetPoint( iCurrentMovePoint ).y;
			float pathdistx = fabs(thispointx - lastpointx);
			float pathdisty = fabs(thispointy - lastpointy);
			if ( pathdistx > 5.0f ) pathdistx -= 5.0f;
			if ( pathdisty > 5.0f ) pathdisty -= 5.0f;
			float entitydistx = fabs(GetX() - lastpointx);
			float entitydisty = fabs(GetZ() - lastpointy);
			if ( entitydistx*entitydistx + entitydisty*entitydisty >= pathdistx*pathdistx + pathdisty*pathdisty )
			{
				// we have CROSSED the node and can move onto the next one, 
				// so no more searching for the node based on position!
				bFurtherThanDistanceOfPath = true;
			}

			// when get to end of path, stay in this container or consider moving to another one
			if ( bFurtherThanDistanceOfPath == true && iCurrentMovePoint == cMovePath.CountPoints( ) - 1 && bChangingContainers )
			{
				// stay in same container
				bChangingContainers = false;
			}
			else
			{
				// check if different container ahead
				bool bDiffContainers = (iCurrentMovePoint < cMovePath.CountPoints( ) - 1) && (cMovePath.GetPoint( iCurrentMovePoint ).container != cMovePath.GetPoint( iCurrentMovePoint + 1 ).container);
				if ( (bFurtherThanDistanceOfPath == true && iCurrentMovePoint < cMovePath.CountPoints( ) - 1) || (bDiffContainers && bFurtherThanDistanceOfPath == true) )
				{
					// work out new container to resume path
					Container *pNewCont = pWorld->GetContainer( cMovePath.GetPoint( iCurrentMovePoint+1 ).container );
					if ( !bDiffContainers || pNewCont->pPathFinder->GridGetPosition( cMovePath.GetPoint( iCurrentMovePoint+1 ).x, cMovePath.GetPoint( iCurrentMovePoint+1 ).y ) == 0 )
					{
						SetContainer( pNewCont );
						iCurrentMovePoint++;
						if ( cMovePath.GetPoint( iCurrentMovePoint - 1 ).container != cMovePath.GetPoint( iCurrentMovePoint ).container ) 
							bChangingContainers = true;
						else 
							bChangingContainers = false;
					}
				}
			}

			// always set the current destination based on end of path segment we are on
			vecCurrDest.x = cMovePath.GetPoint( iCurrentMovePoint ).x;
			vecCurrDest.z = cMovePath.GetPoint( iCurrentMovePoint ).y;
			vecCurrDest.y = GetY();
		}
		else
		{
			// straight path to end node
			if ( cMovePath.CountPoints( ) == 2 )
			{
				// two point path, straight to destination
				vecCurrDest.x = cMovePath.GetPoint( 1 ).x;
				vecCurrDest.z = cMovePath.GetPoint( 1 ).y;
				vecCurrDest.y = GetY();
				vecFinalDest = vecCurrDest;
			}
			else
			{
				// no possible path if only have one node
				if ( cMovePath.CountPoints( ) == 1 )
				{
					vecCurrDest.x = cMovePath.GetPoint( 0 ).x;
					vecCurrDest.z = cMovePath.GetPoint( 0 ).y;
					vecCurrDest.y = GetY();
				}
				else
				{
					// no nodes, stood still, so just look at final dest (goto pos)
					vecCurrDest = vecFinalDest;
				}
			}
		}
	}

	// determine if reached final destination
	float fDiffX = vecFinalDest.x - GetX( );
	float fDiffZ = vecFinalDest.z - GetZ( );
	float fDist = sqrt(fabs(fDiffX*fDiffX) + fabs(fDiffZ*fDiffZ));
	fDiffX = vecCurrDest.x - GetX( );
	fDiffZ = vecCurrDest.z - GetZ( );
	float fDist2 = sqrt(fabs(fDiffX*fDiffX) + fabs(fDiffZ*fDiffZ));
	if ( fDist2 > fDist ) fDist = fDist2;
	if ( cMovePath.CountPoints() > 1 && fDist > 4.0f )
	{
		if ( cMovePath.CountPoints() == 2 )
		{
			// last one, so if short and FAST, stop to avoid running in circles to find perfect landing spot
			if ( fDist < fSpeed/10.0f )
				bMoving = false;
			else		
				bMoving = true;
		}
		else
		{
			// longer way to go
			bMoving = true;
		}
	}
	else
	{
		bMoving = false;
	}

	// if avoid mode 4, and timer allows an event here
	if ( l_iAvoidMode == 4 && !bChangingContainers && !bIsLeaping && !bIsDiving && iAggressiveness != 2 )
	{
		if ( fExpensiveUpdateTimer <= 0.0f )
		{
			// clear so can create a new avoid path
			cAvoidPath.Clear( );
			RebuildDynamicPathFinder( );
			
			// dynamically caluclate a dynamic 'on the fly' avoidance path
			AvoidanceObject *pObject;
			if ( pDynamicPathFinder->InObject( GetX( ), GetZ( ), &pObject ) )
			{
				float x;
				float z;
				if ( pDynamicPathFinder->GetClosestPoint( pObject, GetX( ), GetZ( ), x, z ) )
				{
					cAvoidPath.AddPoint( GetX(), 0, GetZ() );
					cAvoidPath.AddPoint( x, 0, z );
					iCurrentAvoidPoint = 1;
				}
			}
			else
			{
				pDynamicPathFinder->CalculatePath( GetX( ), GetZ( ), vecCurrDest.x, vecCurrDest.z, &cAvoidPath, -1 );
				iCurrentAvoidPoint = 1;
			}

			// refresh timer to ensure better performance
			fExpensiveUpdateTimer = ( 0.05f * rand() ) / RAND_MAX + 0.2f;
		}
		
		// if avoid path was created
		if ( cAvoidPath.CountPoints( ) > 1 )
		{
			// if already near or on avoid path node, move to next one right away
			float diffx = cAvoidPath.GetPoint( iCurrentAvoidPoint ).x - GetX();
			float diffz = cAvoidPath.GetPoint( iCurrentAvoidPoint ).y - GetZ();
			if ( diffx*diffx + diffz*diffz < 0.01f && iCurrentAvoidPoint < cAvoidPath.CountPoints( ) - 1 ) 
			{
				iCurrentAvoidPoint++;
			}

			// set the current destination to this avoid path node instead (overrides one set above)
			vecCurrDest.x = cAvoidPath.GetPoint( iCurrentAvoidPoint ).x;
			vecCurrDest.z = cAvoidPath.GetPoint( iCurrentAvoidPoint ).y;
		}
	}

	// if avoidance mode (0,1,2,5) and not specific iAggressiveness mode (automated state)
	if ( (l_iAvoidMode < 3 || l_iAvoidMode == 5) && !bChangingContainers && !bIsLeaping && !bIsDiving && iAggressiveness != 2 )
	{
		// adjust intermediate destination based on nearby entities
		if ( bAvoiding == false )
		{
			float fDiffX = vecCurrDest.x - GetX( );
			float fDiffZ = vecCurrDest.z - GetZ( );
			float fDist = fDiffX*fDiffX + fDiffZ*fDiffZ;
			if ( fDist > 0.00001f )
			{
				// work out new current destination position to place in circle around current AI position
				float fDirAngle = acos ( fDiffZ / sqrt(fDist) ) * RADTODEG;
				if ( fDiffX < 0 ) fDirAngle = 360.0f - fDirAngle;
				bool bValid = AdjustDirection ( &fDirAngle );
				if ( bValid == true )
				{
					// set this AI bot to do an avoid phase
					float fAvoidByDistance = 100.0f;
					vecCurrDest.x = GetX( ) + (sin ( fDirAngle * DEGTORAD ) * fRadius * fAvoidByDistance);
					vecCurrDest.z = GetZ( ) + (cos ( fDirAngle * DEGTORAD ) * fRadius * fAvoidByDistance);
					vecAvoidPos.x = vecCurrDest.x;
					vecAvoidPos.y = vecCurrDest.y;
					vecAvoidPos.z = vecCurrDest.z;
					fAvoidTimer = 0.2f;
					bAvoiding = true;
				}
			}
		}
	}

	// handle avoid sequence (only uses vecAvoidPos above temporarily)
	if ( bAvoiding && fAvoidTimer <= 0 ) 
	{
		// ensure avoidance is switched off when time is up
		bAvoiding = false;
	}

	// double checks that a current destination was not embedded inside an obstacle
	/* seems only need this if bot start getting stuck INSIDE obstacles - from avoidance!
	if ( fObstacleTimer <= 0.0f && iAggressiveness != 2 )
	{
		if ( !bChangingContainers && !bIsLeaping && !bIsDiving )
		{
			// adjust intermediate destination based on obstacle data
			float fDX = vecLastCurrDest.x - vecCurrDest.x;
			float fDZ = vecLastCurrDest.z - vecCurrDest.z;
			float fDD = sqrt(fabs(fDX*fDX)+abs(fDZ*fDZ));
			if ( fDD >= 0.0001f )
			{
				if ( pContainer->pPathFinder->InPolygons ( vecCurrDest.x, vecCurrDest.z ) > 0 )
				{
					pContainer->pPathFinder->FindClosestOutsidePoint ( &(vecCurrDest.x), &(vecCurrDest.z) );
				}
				vecLastCurrDest = vecCurrDest;
			}

			// find closest polygon
			float fResult = pContainer->pPathFinder->FindClosestPolygon ( GetX( ), GetZ( ), vecCurrDest.x, vecCurrDest.z );
			if ( fResult >= 0.0f ) 
			{
				vecCurrDest.x = GetX( ) + ( vecCurrDest.x - GetX( ) )*fResult;
				vecCurrDest.z = GetZ( ) + ( vecCurrDest.z - GetZ( ) )*fResult;
			}
		}

		// intentionally commented out, diabling this?
		// fObstacleTimer = 0.2f + (rand()*0.05f / RAND_MAX);
	}
	*/
	
	/* 140217 - vecCurrLookAt (for movement) needed to be set when bot reached end of last path
	SUGGEST DELETING THIS - NOW LOOKAT IS ALSO MOVETO VECTOR!! (leave LOOKing to LUA script now)
	if ( stagger == 7 )
	{
		//handle entity look direction
		int iHeight = 1;
		if ( bIsDucking ) iHeight = 2;

		if ( !bLookAtPointSet )
		{		
			//if ( !pContainer->pPathFinder->QuickPolygonsCheckVisible ( GetX( ), GetZ( ), vecFinalDest.x, vecFinalDest.z, iHeight ) ) 
			if ( CouldSee( vecFinalDest.x, vecFinalDest.y, vecFinalDest.z, false ) && bCanStrafe )
			{
				vecCurrLookAt = vecFinalDest;
			}
			else 
			{
				if ( cMovePath.CountPoints ( ) > 2 )
				{
					vecCurrLookAt.x = cMovePath.GetPoint( iCurrentMovePoint ).x;				//set the look at to the current destination
					vecCurrLookAt.z = cMovePath.GetPoint( iCurrentMovePoint ).y;
					vecCurrLookAt.y = GetY( );
				}
				else
				{
					vecCurrLookAt = vecFinalDest;
				}
			}
		}
		else
		{
			//if ( !pContainer->pPathFinder->QuickPolygonsCheckVisible ( GetX( ), GetZ( ), vecLookAt.x, vecLookAt.z, iHeight ) )
			if ( CouldSee( vecLookAt.x, vecLookAt.y, vecLookAt.z, false ) && bCanStrafe )
			{
				vecCurrLookAt = vecLookAt;				//set the look at to the one defined by the state
			}
			else
			{
				if ( !bMoving )
				{
					vecCurrLookAt = vecLookAt;
				}
				else
				{
					if ( cMovePath.CountPoints ( ) > 2 )
					{
						vecCurrLookAt.x = cMovePath.GetPoint( iCurrentMovePoint ).x;				//set the look at to the current destination
						vecCurrLookAt.z = cMovePath.GetPoint( iCurrentMovePoint ).y;
						vecCurrLookAt.y = GetY( );
					}
					else
					{
						vecCurrLookAt = vecFinalDest;
					}
				}
			}		
		}
	}
	*/

	// 140217 - LOOK VECTOR (which controls entity angle) needs to be the destination 
	// for perfect turning while path finding at various speeds
	vecCurrLookAt = vecCurrDest;

	// show viewing arcs if debug mode active
	if ( bShowViewArcs ) DebugDrawViewArcs ( );
	else DebugHideViewArcs ( );

	// set the direction of movement 
	GGVECTOR3 vecDir;
	vecDir.x = vecCurrDest.x - GetX ( );
	vecDir.z = vecCurrDest.z - GetZ ( );
	vecDir.y = 0;

	// update last position of AI bot
	float fLength = vecDir.x*vecDir.x + vecDir.z*vecDir.z;
	vecLastPos.x = GetX( );
	vecLastPos.y = GetY( );
	vecLastPos.z = GetZ( );

	// not sure what this is?
	pvecDir[0] = pvecDir[0]*0.75f + vecDir*0.25f;

	// if no avoidance mode, affect the direction vector?
	if ( l_iAvoidMode == 0 )
	{
		GGVECTOR3 vecAvgDir( pvecDir[0] );
		float fDiffX = abs( vecAvgDir.x - vecDir.x );
		float fDiffZ = abs( vecAvgDir.z - vecDir.z );
		if ( fDiffX < 0.1f && fDiffZ < 0.1f ) vecDir = vecAvgDir;
		else { vecDir.x = 0; vecDir.y = 0; vecDir.z = 0; }
	}

	// 280217 - flag to restore legacy behaviour of AI moving itself based on AI speed (from below)
	if ( bLegacyForceMove == true )
	{
		// older GoTo call did not use containers
		if ( fLength > 0.01f && fLength > fTimedSpeed*fTimedSpeed )
		{	
			// adjust direction with AI bot speed
			vecDir = ( ( fTimedSpeed * vecDir ) / sqrt( fLength ) );
			// adjust direction vector for diving and leaping
			if ( bIsDiving ) vecDir *= fDiveMultiplier;
			else if ( bIsLeaping ) vecDir *= fLeapMultiplier;
			// flags to determine movement style
			if ( pObject ) pObject->position.vecPosition += vecDir;
			else
			{
				fPosX += vecDir.x;
				fPosY += vecDir.y;
				fPosZ += vecDir.z;
			}
			SetPosition ( fPosX, fPosY, fPosZ );
			bMoving = true;
		}
	}
	
	// if distance between current position and destination, use intermediate destination to move entity (automatic state)
	/* DIRECTLY MOVES the position of the AI bot - remove this as want LUA script control here
	if ( fLength > 0.01f && fLength > fTimedSpeed*fTimedSpeed )
	{	
		// adjust direction with AI bot speed
		vecDir = ( ( fTimedSpeed * vecDir ) / sqrt( fLength ) );

		// adjjust direction vector for diving and leaping
		if ( bIsDiving ) vecDir *= fDiveMultiplier;
		else if ( bIsLeaping ) vecDir *= fLeapMultiplier;

		// flags to determine movement style
		int iCanMove = 1;
		int iReserved = 0;

		// for avoidance mode 5, can reserve spaces
		if ( l_iAvoidMode == 5 )
		{
			int ignore = bMeleeMode ? GetTargetID(1) : 0;
			iCanMove = pContainer->pPathFinder->GridCheckDirection( GetID( ), GetX( ), GetZ( ), vecDir.x, vecDir.z, vecFinalDest.x, vecFinalDest.z, ignore, iReserved );
			if ( iCanMove == 2 )
			{
				pContainer->pPathFinder->GridClearEntityPosition( GetX(), GetZ() );
			}
			if ( iReserved == 1 ) bReservedSpace = true;
			if ( iReserved == 2 ) bReservedSpace = false;
		}

		if ( iCanMove && iAggressiveness != 2 )
		{
			if ( pObject ) pObject->position.vecPosition += vecDir;
			else
			{
				fPosX += vecDir.x;
				fPosY += vecDir.y;
				fPosZ += vecDir.z;
			}
		}

		//if ( !bMoving )	ResetFeedbackPos ( );
		//bMoving = true;
	}
	else
	{
		int iCanMove = 1;
		int iReserved = 0;
		if ( l_iAvoidMode == 5 )
		{
			int ignore = bMeleeMode ? GetTargetID(1) : 0;
			iCanMove = pContainer->pPathFinder->GridCheckDirection( GetID( ), GetX( ), GetZ( ), vecCurrDest.x - GetX(), vecCurrDest.z - GetZ(), vecFinalDest.x, vecFinalDest.z, ignore, iReserved );
			if ( iCanMove == 2 )
			{
				pContainer->pPathFinder->GridClearEntityPosition( GetX(), GetZ() );
			}
			if ( iReserved == 1 ) bReservedSpace = true;
			if ( iReserved == 2 ) bReservedSpace = false;
		}

		if ( iCanMove && iAggressiveness != 2 )
		{
			if ( pObject ) 
			{
				pObject->position.vecPosition.x = vecCurrDest.x;
				pObject->position.vecPosition.z = vecCurrDest.z;
			}
			else
			{
				fPosX = vecCurrDest.x;
				fPosZ = vecCurrDest.z;
			}
		}

		//bMoving = false;
		bAvoiding = false;
		if ( GetSqrDistToDest() > 0.0001f ) 
		{
			//Dave
			if ( bRedoPath == false )
			{
				ListOfEntitiesToUpdateMovement.push_back(this->iID);
				bRedoPath = true;
				//fPathTimer -= 0.35f;
			}
			//fPathTimer = 0;
		}
	}
	*/

	// if avoidance mode 5, set AI bot id to grid placement
	if ( l_iAvoidMode == 5 ) pContainer->pPathFinder->GridSetEntityPosition( GetX(), GetZ(), GetID( ) );

	// turn the entity to its look vector (now same as move position)
	TurnToAngle ( fTimeDelta, bMoving );

	// reset user defined avoidance data
	bHitSomething = false;
	bHitBySomething = false;
	pHitEntity = 0;
	pHitByEntity = 0;
}

void Entity::RemoveFromThread ( )
{
	// when destroy entity, ensure leave any thread work
	if ( g_LeeThread.GetWorkInProgress() == this->iID )
	{
		g_LeeThread.EndWork();
	}
}

void Entity::UpdateState ( )
{
	vector <sHitPoint>::iterator hIter = sHitFrom_list.begin ( );

	while ( hIter < sHitFrom_list.end ( ) )
	{
		hIter->fLifeTime -= pWorld->GetTimeDelta ( );
		if ( hIter->fLifeTime <= 0.0f ) hIter = sHitFrom_list.erase ( hIter );
		else hIter++;
	}


	//Dave commented out
	/*if ( bDefending && !bManualControl )
	{
		float fDist = GetSqrDistTo ( vecDefendPos.x, vecDefendPos.y, vecDefendPos.z );
		if ( fDist > fDefendDist*fDefendDist )
		{
			ChangeState( pWorld->pEntityStates->pStateDefend );
		}
	}*/

	if ( !pCurrentState ) return;

	//pCurrentState->Execute ( this );

	bHit = false;
}

void Entity::ChangeState ( State *pNewState )
{
	if ( pCurrentState ) pCurrentState->Exit ( this );

	pLastState = pCurrentState;
	pCurrentState = pNewState;

	if ( pCurrentState ) pCurrentState->Enter ( this );
}

void Entity::ReturnToPreviousState ( )
{
	if ( pCurrentState ) pCurrentState->Exit ( this );

	if ( pLastState ) pCurrentState = pLastState;
	else pCurrentState = pWorld->pEntityStates->pStateIdle;
	pLastState = 0;

	if ( pCurrentState ) pCurrentState->Enter ( this );
}

void Entity::DebugDrawDestination ( )
{
	if ( iCurrDestObject == 0 )
	{
		iCurrDestObject = dbFreeObject ( );
		MakeObjectSphere ( iCurrDestObject, 2.0f, 2, 4 );
		SetObjectMask ( iCurrDestObject, 1 );
		ScaleLimb( iCurrDestObject, 0, 40*fRadius,40*fRadius,40*fRadius );
		SetObjectCollisionOff( iCurrDestObject );
	}

	PositionObject ( iCurrDestObject, vecCurrDest.x, vecCurrDest.y, vecCurrDest.z );
	ColorObject ( iCurrDestObject, 0xff640000 );
	SetObjectEmissive ( iCurrDestObject, 0xff640000 );
	SetObjectEffect ( iCurrDestObject, g_GUIShaderEffectID );
	
	SetObjectCollisionOff ( iCurrDestObject );
}

void Entity::DebugHideDestination ( )
{
	if ( iCurrDestObject > 0 && ObjectExist ( iCurrDestObject ) == 1 )
	{
		DeleteObject ( iCurrDestObject );
	}

	iCurrDestObject = 0;
}

void MakeTriangleSegment ( int iMeshNum, float fAngle, float fLength )
{
	if ( GetMeshExist ( iMeshNum ) == 1 ) DeleteMesh ( iMeshNum );
	float fSegAng = ( fAngle / 6.0f ) * DEGTORAD;
	
	int iObjNum = dbFreeObject ( );
	MakeObjectTriangle ( iObjNum, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, fLength, fLength * sin ( fSegAng ), 0.0f, fLength * cos ( fSegAng ) );
	SetObjectMask ( iObjNum, 1 );
	MakeMeshFromObject ( iMeshNum, iObjNum );

	fSegAng *= RADTODEG;

	for ( int i = 1; i < 6; i++ )
	{
		AddLimb ( iObjNum, i, iMeshNum );
		RotateLimb ( iObjNum, i, 0.0f, i * fSegAng, 0.0f );
	}

	dbCombineLimbs ( iObjNum );

	DeleteMesh ( iMeshNum );
	MakeMeshFromObject ( iMeshNum, iObjNum );
	DeleteObject ( iObjNum );
}

void Entity::DebugDrawObstacleAngles ( )
{
	DebugHideObstacleAngles ( );
	
	if ( sCombinedObstacleAngle_list.size( ) < 1 ) return;

	int iTempMesh = dbFreeMesh ( );
	iObstacleAnglesObject = dbFreeObject ( );
	
	float fAngDiff = sCombinedObstacleAngle_list [ 0 ].fAngEnd - sCombinedObstacleAngle_list [ 0 ].fAngBegin;
	float fLength = fRadius * 3.0f;
	
	MakeTriangleSegment ( iTempMesh, fAngDiff, fLength );
	MakeObject ( iObstacleAnglesObject, iTempMesh, 0 );
	SetObjectMask ( iObstacleAnglesObject, 1 );
	RotateLimb ( iObstacleAnglesObject, 0, 0.0f, sCombinedObstacleAngle_list [ 0 ].fAngBegin, 0.0f );
	
	for (int i = 1; i < (int) sCombinedObstacleAngle_list.size( ); i++ )
	{
		fAngDiff = sCombinedObstacleAngle_list [ i ].fAngEnd - sCombinedObstacleAngle_list [ i ].fAngBegin;
		
		MakeTriangleSegment ( iTempMesh, fAngDiff, fLength );

		AddLimb ( iObstacleAnglesObject, i, iTempMesh );
		RotateLimb ( iObstacleAnglesObject, i, 0.0f, sCombinedObstacleAngle_list [ i ].fAngBegin, 0.0f );
	}

	if ( GetMeshExist ( iTempMesh ) == 1 ) DeleteMesh ( iTempMesh );
	dbCombineLimbs ( iObstacleAnglesObject );

	PositionObject ( iObstacleAnglesObject, GetX( ), fDebugObstacleAnglesObjHeight, GetZ( ) );
	ColorObject ( iObstacleAnglesObject, 0xff00ff00 );
	SetObjectEmissive ( iObstacleAnglesObject, 0xff00ff00 );
	SetObjectEffect ( iObstacleAnglesObject, g_GUIShaderEffectID );

	SetObjectCollisionOff ( iObstacleAnglesObject );
}

void Entity::DebugHideObstacleAngles ( )
{
	if ( iObstacleAnglesObject > 0 && ObjectExist ( iObstacleAnglesObject ) == 1 )
	{
		DeleteObject ( iObstacleAnglesObject );
	}

	iObstacleAnglesObject = 0;
}

void Entity::DebugDrawViewArcs ( )
{
	int iTempMesh = dbFreeMesh ( );

	if ( iOuterViewObject == 0 ) 
	{
		iOuterViewObject = dbFreeObject ( );
		
		float fSegAngle = ( fOuterViewArc / 20.0f ) * DEGTORAD;

		MakeObjectTriangle ( iOuterViewObject, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, fViewRange, fViewRange * sin ( fSegAngle ), 0.0f, fViewRange * cos ( fSegAngle ) );
		SetObjectMask ( iOuterViewObject, 1 );
		MakeMeshFromObject ( iTempMesh, iOuterViewObject );

		fSegAngle *= RADTODEG;

		for ( int i = 1; i < 20; i++ )
		{
			AddLimb ( iOuterViewObject, i, iTempMesh );
			RotateLimb ( iOuterViewObject, i, 0.0f, i * fSegAngle, 0.0f );
		}

		DeleteMesh ( iTempMesh );
		MakeMeshFromObject ( iTempMesh, iOuterViewObject );
		AddLimb ( iOuterViewObject, 20, iTempMesh );
		RotateLimb ( iOuterViewObject, 20, 0.0f, -fOuterViewArc, 0.0f );
		
		dbCombineLimbs ( iOuterViewObject );
		DeleteMesh ( iTempMesh );

		ColorObject ( iOuterViewObject, 0xffff0000 );
		SetObjectEmissive ( iOuterViewObject, 0xffff0000 );
		SetObjectEffect ( iOuterViewObject, g_GUIShaderEffectID );

		SetAlphaMappingOn ( iOuterViewObject, 25.0f );
		DisableObjectZWrite ( iOuterViewObject );
	}

	if ( iHearingRangeObject == 0 ) 
	{
		iHearingRangeObject = dbFreeObject ( );

		float fSegAngle = ( 360.0f / 40.0f ) * DEGTORAD;

		MakeObjectTriangle ( iHearingRangeObject, 0.0f, 0.0f, fHearingRange - 1.0f, 0.0f, 0.0f, fHearingRange, fHearingRange * sin ( fSegAngle ), 0.0f, fHearingRange * cos ( fSegAngle ) );
		MakeMeshFromObject ( iTempMesh, iHearingRangeObject );
		DeleteObject ( iHearingRangeObject );
		MakeObjectTriangle ( iHearingRangeObject, 0.0f, 0.0f, fHearingRange - 1.0f, fHearingRange * sin ( fSegAngle ), 0.0f, fHearingRange * cos ( fSegAngle ), (fHearingRange - 1.0f) * sin ( fSegAngle ), 0.0f, (fHearingRange - 1.0f) * cos ( fSegAngle ) );
		AddLimb ( iHearingRangeObject, 1, iTempMesh );
		dbCombineLimbs ( iHearingRangeObject );
		DeleteMesh ( iTempMesh );
		MakeMeshFromObject ( iTempMesh, iHearingRangeObject );
		SetObjectMask ( iHearingRangeObject, 1 );

		fSegAngle *= RADTODEG;

		for ( int i = 1; i < 40; i++ )
		{
			AddLimb ( iHearingRangeObject, i, iTempMesh );
			RotateLimb ( iHearingRangeObject, i, 0.0f, i * fSegAngle, 0.0f );
		}

		dbCombineLimbs ( iHearingRangeObject );
		DeleteMesh ( iTempMesh );

		ColorObject ( iHearingRangeObject, 0xffffff00 );
		SetObjectEmissive ( iHearingRangeObject, 0xffffff00 );
		SetObjectEffect ( iHearingRangeObject, g_GUIShaderEffectID );
	}

	PositionObject ( iOuterViewObject, GetX ( ), fDebugViewObjHeight, GetZ ( ) );
	YRotateObject ( iOuterViewObject, GetAngleY ( ) );

	PositionObject ( iHearingRangeObject, GetX ( ), fDebugViewObjHeight, GetZ ( ) );

	SetObjectCollisionOff ( iOuterViewObject );
	SetObjectCollisionOff ( iHearingRangeObject );
}

void Entity::DebugHideViewArcs ( )
{
	if ( iInnerViewObject > 0 && ObjectExist ( iInnerViewObject ) )
	{
		DeleteObject ( iInnerViewObject );
	}

	iInnerViewObject = 0;

	if ( iOuterViewObject > 0 && ObjectExist ( iOuterViewObject ) )
	{
		DeleteObject ( iOuterViewObject );
	}

	iOuterViewObject = 0;

	if ( iHearingRangeObject > 0 && ObjectExist ( iHearingRangeObject ) )
	{
		DeleteObject ( iHearingRangeObject );
	}

	iHearingRangeObject = 0;
}

void Entity::DebugOutputInternalData ( HANDLE hOut )
{
	if ( !hOut ) return;

	DWORD dwWritten = 0;
	DWORD dwWrite = 0;
	char strOut [ 256 ];
	
	dwWrite = sprintf_s ( strOut, 255, "\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, "Entity: %d\n", iID );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, "Container: %d\n", pContainer->GetID( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " -Active: %d,\n", bActive ? 1 : 0 );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	
	dwWrite = sprintf_s ( strOut, 255, " -Object Num: %d, Object Pointer %p\n", iID, pObject );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " -Radius: %f, Min Distance: %f\n", fRadius, fMinimumDist );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " -Height: %f\n", fHeight );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	
	dwWrite = sprintf_s ( strOut, 255, " -Movement Data " );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Destination Angle Y: %3.3f, Current Angle Y: %3.3f\n", fDestAngY, GetAngleY ( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	
	dwWrite = sprintf_s ( strOut, 255, " --Max Speed: %3.3f, Current Speed: %3.3f, Turn Speed: %3.3f\n", fSpeed, fCurrSpeed, fTurnSpeed );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Original Position: %3.3f, %3.3f, %3.3f\n", vecOrigPos.x, vecOrigPos.y, vecOrigPos.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Defend Position: %3.3f, %3.3f, %3.3f\n", vecDefendPos.x, vecDefendPos.y, vecDefendPos.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current Position: %3.3f, %3.3f, %3.3f\n", GetX ( ), GetY ( ), GetZ ( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current Destination: %3.3f, %3.3f, %3.3f\n", vecCurrDest.x, vecCurrDest.y, vecCurrDest.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Final Destination: %3.3f, %3.3f, %3.3f\n", vecFinalDest.x, vecFinalDest.y, vecFinalDest.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Avoid Position: %3.3f, %3.3f, %3.3f\n", vecAvoidPos.x, vecAvoidPos.y, vecAvoidPos.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Interest Position: %3.3f, %3.3f, %3.3f\n", vecInterestPos.x, vecInterestPos.y, vecInterestPos.z );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " -View Data\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Inner View Arc: %3.3f  Outer View Arc: %3.3f  Fire Arc: %3.3f\n", fInnerViewArc, fOuterViewArc, fFireArc );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --View Range: %3.3f  Hearing Range: %3.3f\n", fViewRange, fHearingRange );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " -State Data\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current State: %s\n", pCurrentState->GetName ( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --State Timer: %f\n", fStateTimer );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	
	dwWrite = sprintf_s ( strOut, 255, " -Other\n" );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Can Fire: %d  Is Hit: %d  Heard Sound: %d\n", bFireWeapon, bHit, bHeardSound );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Num Hit Points: %d\n", CountHitPoints( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Aggressiveness: %d\n", iAggressiveness );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Container: %d\n", pContainer ? pContainer->GetID() : -1 );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Changing Containers: %d\n", bChangingContainers ? 1 : 0 );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Number of Targets: %d\n", CountTargets( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Hit Something: %d  Hit By Something: %d\n", bHitSomething, bHitBySomething );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Hit Something: %p  Hit By Something: %p\n", pHitEntity, pHitByEntity );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Avoiding: %d  Making Progress: %d  Stuck: %d\n", bAvoiding, bMakingProgress, bStuck );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Moving: %d  In A Polygon: %d\n", bMoving, pContainer ? pContainer->pPathFinder->InPolygons ( GetX( ), GetZ( ) ) : 0 );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Following: %d  Defending: %d  Defend Dist: %3.3f\n", bFollowingLeader, bDefending, fDefendDist );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Manual Control: %d  Look At Point Set: %d\n", bManualControl, bLookAtPointSet );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Patrol Path ID: %d  Pointer: %p\n", iPatrolPathID, pPatrolPath );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current Patrol Point: %d  Num Points: %d\n", iCurrentPatrolPoint, CountPatrolPoints( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current Avoid Point: %d  Num Points: %d\n", iCurrentAvoidPoint, cAvoidPath.CountPoints( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Current Move Point: %d  Move Path Points: %d\n", iCurrentMovePoint, cMovePath.CountPoints ( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	dwWrite = sprintf_s ( strOut, 255, " --Obstacle Angles: %d\n", sCombinedObstacleAngle_list.size( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	for ( int i = 0; i < (int) sCombinedObstacleAngle_list.size( ); i++ )
	{
		dwWrite = sprintf_s ( strOut, 255, " ---Angle Begin: %f  Angle End: %f\n", sCombinedObstacleAngle_list [ i ].fAngBegin, sCombinedObstacleAngle_list [ i ].fAngEnd );
		WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	}

	dwWrite = sprintf_s ( strOut, 255, " --Targets: %d  Valid Target: %d\n", CountTargets( ), ValidTarget( ) );
	WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );

	for ( int i = 0; i < (int) CountTargets( ); i++ )
	{
		dwWrite = sprintf_s ( strOut, 255, " ---ID: %d  Threat: %f  Time: %f  Zone: %d Container: %d\n", sTarget_list [ i ].iTargetID, sTarget_list [ i ].fTargetThreat, sTarget_list [ i ].fTargetTimer, sTarget_list [ i ].bZoneTarget, sTarget_list [ i ].iContainer );
		WriteConsole ( hOut, strOut, dwWrite, &dwWritten, NULL );
	}

}

void Entity::SetCoverPoint( sCoverPoint* pPoint ) 
{ 
	pCoverPoint = pPoint; 
	if ( pCoverPoint ) pCoverPoint->InUse();
}

void Entity::ClearCoverPoint() 
{ 
	Container* pCont = cWorld.GetContainer(0);
	if ( !pCont ) return;

	PathFinderAdvanced* pCoverList = NULL;
	if ( !pCont->pPathFinder ) return;

	sCoverPoint* pItem = pCont->pPathFinder->pCoverPointList;
	while ( pItem )
	{
		if ( pCoverPoint==pItem ) { pCoverPoint->Unused(); break; }
		pItem = pItem->pNextPoint;
	}
	pCoverPoint = 0;
}

sCoverPoint* Entity::GetCoverPoint()
{
	return pCoverPoint;
}

void Entity::MoveToCoverPoint()
{
	if ( !pCoverPoint ) return;

	if ( prevCoverX.size() > 10 )
	{
		prevCoverX.clear();
		prevCoverZ.clear();
	}

	prevCoverX.push_back(pCoverPoint->fX);
	prevCoverZ.push_back (pCoverPoint->fY);
	SetDestination( pCoverPoint->fX, GetY(), pCoverPoint->fY, pCoverPoint->iContainer );
}

float Entity::GetLeapRange()
{
	return fLeapRange;
}

float Entity::GetDiveRange()
{
	return fDiveRange;
}

int Entity::GetMovePoints()
{
	return cMovePath.CountPoints();
}

void Entity::SetDiveMultiplier( float value )
{
	fDiveMultiplier = value;
}

void Entity::SetLeapMultiplier( float value )
{
	fLeapMultiplier = value;
}
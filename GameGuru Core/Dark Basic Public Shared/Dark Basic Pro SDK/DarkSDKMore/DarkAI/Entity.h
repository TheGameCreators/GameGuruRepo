#ifndef H_ENTITY
#define H_ENTITY

#include ".\..\..\Shared\DBOFormat\DBOData.h"
#include "Path.h"

#include <vector>
#include <cctype>

class State;
class Hero;
class World;
class Beacon;
class Zone;
class Team;
class Container;
class DynamicPathFinder;

#define ENT_X 0x00000001
#define ENT_Y 0x00000002
#define ENT_Z 0x00000004

struct sCoverPoint;

//Entity - computer controlled component, ally or enemy or neutral
class Entity
{

public:
	bool bAlwaysActive;

private:

	DynamicPathFinder *pDynamicPathFinder;

	sObject		*pObject;				//dbpro object that represents this entity
	DWORD		dwObjectNumberRef;		//just used for reference, not to override AI bot
	State		*pCurrentState;			//the state to use to update the next step
	State		*pLastState;			//the previous state the entity was in
	Team		*pTeam;					//Team this entity belongs to
	Container	*pContainer;			//the current container the entity is in
	int			iDestContainer;			//entities can move over container bridges

	Path	*pPatrolPath;			//Path to control the movement of the entity whilst idle
	int		iPatrolPathID;			//indexs the current patrol path in the world structure
	int		iCurrentPatrolPoint;	//current position in the patrol route

	int		iCurrentMovePoint;		//the entity's current location in the move path
	int		iCurrentAvoidPoint;		//the entity's current location in the avoid path
	int		iOriginalContainer;		//the container containing the idle position

public:
	bool	bChangingContainers;	//true if an entity is moving along a container bridge

private:
	int		iInterestContainer;		//container the current interest point is in

	float fPosX, fPosY, fPosZ;		//entity position if no object is assigned
	float fAngY;					//entity angle if no object is assigned
	float fHeight;					//the distance from the object's center to the entity's head, used for line of sight checks

	float fDestAngY;				//angle the entity is turning to
	float fSpeed;					//entity movement speed, units per second
	float fCurrSpeed;				//state set speed, less than fSpeed
	float fTurnSpeed;				//entity turn speed, degrees per second
	float fMinimumDist;				//closest the entity will get to a target
	float fDefendDist;				//maximum distance to be from defend position
	float fAvoidDist;				//the distance an entity will start to avoid a target
	float fPathStartCostLimit;		//the maximum distance an entity can move without going through a waypoint
	bool bReservedSpace;			//has the entity reserved a grid space that it should be moving into
	int iEntityAvoidMode;			//avoidance mode on a per entity basis

	float fAlertLevel;				//higher values mean the entity is more likely to take action

	float fDistanceMoved;			//feedback for if he entity gets stuck
	GGVECTOR3 vecLastRecPos;		//reference for distance moved
	bool bStuck;					//didn't move more than CurrSpeed/4 in the last second (and should have)
	bool bAvoidLeft;				//going left or right to get around imeadiate obstacle
	bool bAvoiding;					//currently making an avoiding movement
	bool bMakingProgress;			//last movement was towards current destination
	bool bMoving;					//currently trying to move to a destination
	bool bLookAtPointSet;			//has a state defined point to look at
	bool bFollowingLeader;			//is currently following/defending something
	bool bDefending;				//is defending a point or person

public:
	bool bLegacyForceMove;			//280217 - flag to denote legacy behavior 

private:
	float fFireArc;					//angle that the entity can fire from, degrees
	float fInnerViewArc;			//angle that the entity can fully see
	float fOuterViewArc;			//angle that the entity can partially see
	float fViewRange;				//how far the entity can see
	float fHearingRange;			//how far the entity can hear
	int iHearingThreshold;			//the lowest priority sound the entity will respond to

	float fRadius;					//the entity radius
	
	float fStateTimer;				//set by state, time in seconds
	float fLookAroundTimer;			//controls pausing between turning
	float fInvestigateTimer;		//limits time spent investigating events
	float fCoverTimer;				//how long the entity has been in cover without seeing the target
	float fMoveFeedbackTimer;		//updates the distance moved when htis timer expires
	float fAvoidTimer;				//how long to move in an avoidance direction
	float fPathTimer;				//how long til path should be updated
	float fForceUpdatePathTimer;	//how long til path must be updated
	float fResetTimer;				//when set back to idle, keeps the entity from re-activating for a short time
	float fWaitTimer;				//whilst not zero, the entity will not move
	float fExpensiveUpdateTimer;	//how long til the entity should update expensive operations
	float fAdjustDestinationTimer;	//timer to wait until changing destination when avoiding entities
	float fAdjustDirectionTimer;	//timer to wait until changing direction when avoiding entities
	float fSearchTimer;				//limit the amount of time an entity can spend searching
	float fObstacleTimer;			//how often the entity checks its position against the obstacle list

	float fAvgPatrolTime;			//average time the entity spends at each point in a patrol path

	bool bAdjustingDirection;		//entity is currently trying to avoid another entity

	bool bFireWeapon;				//is the entity firing
	bool bIsDucking;				//is the entity ducking
	bool bHit;						//entity was hit since last update
	bool bHeardSound;				//entity picked up a sound ( nearby beacon )
	float fClosestSound;			//distance to closest sound heard
	int iSoundUrgency;				//the importance of the heard sound (foot steps, gun shots, etc)
	int iLastSoundUrgency;			//the imoprtance of the last sound
//	bool bRedoPath;					//the entity needs to recalculate its path

	int iInnerViewObject;			//dbpro object that represents the inner view range
	int iOuterViewObject;			//dbpro object that represents the outer view range
	int iHearingRangeObject;		//dbpro object that represents the hearing range
	int iCurrDestObject;			//dbpro object that represents the current destination
	int iObstacleAnglesObject;		//dbpro object that represents the obstacle angles

	bool bCanHideBehindCorner;		//Can the entity hide and peek from corners
	bool bIsBehindCorner;			//is the entity hiding behind a corner
	float fDiveRange;				//the distance an entity can dive in to cover
	bool bIsDiving;					//is the entity diving towards cover
	float fLeapRange;				//the distance an entity can leap over cover
	bool bIsLeaping;				//is the entity leaping over cover
	bool bCanDive;					//can the entity dive into cover
	bool bCanLeap;					//can the entity leap over cover
	float fHideDirX, fHideDirY;     //direction entity should face whn hiding behind a corner
	float fPeekX, fPeekY;			//when hiding behind a corner this point is used for visibility checks
	bool bForceOutOfCover;			//set this to true to force the entity to change position
	float fDiveMultiplier;			//speed increase for diving
	float fLeapMultiplier;			//speed increase for leaping

	int stagger;					//Dave - to stagger updating stuff
	GGVECTOR3	pre_vecFinalDest;
	GGVECTOR3 pre_vecLastDest;

	sCoverPoint *pCoverPoint;		// the current cover point being used by the entity, stored so it can be marked as used or unused

	struct sHitPoint
	{
		float x,y,z;
		float angY;
		float fLifeTime;

		bool operator<(sHitPoint p) const {
			return angY < p.angY;
		}
	};

	struct sTarget
	{
		GGVECTOR3 vecPos, vecLastPos, vecGuessPos, vecLastDir;
		Hero *pTargetHero;
		Entity *pTargetEntity;
		sObject *pTargetObject;
		float fTargetThreat;
		int iTargetType;
		float fTargetTimer;
		bool bZoneTarget;
		int iTargetID;
		bool bCanSee;
		int iContainer;

		bool operator<(sTarget t) const {
			return ( fTargetThreat < t.fTargetThreat );
		}
	};

	struct sObstacleAngle
	{
		float fAngBegin, fAngEnd;
		float fTimer;
		Entity *pEntity;

		bool operator<(sObstacleAngle o) const {
			return ( fAngBegin < o.fAngBegin );
		}
	};

	std::vector <sHitPoint>			sHitFrom_list;
	std::vector <sTarget>			sTarget_list;
	std::vector <sObstacleAngle>	sObstacleAngle_list;
	std::vector <sObstacleAngle>	sCombinedObstacleAngle_list;

public:

	bool bRedoPath;					//the entity needs to recalculate its path
	std::vector <float> prevCoverX;
	std::vector <float> prevCoverZ;

	static float fDebugViewObjHeight;
	static float fDebugObstacleAnglesObjHeight;
	static int iAvoidMode;
	
	bool operator==(Entity e) const {
		return ( iID == e.iID );
	}

	static World *pWorld;
	static bool bShowPaths;
	static bool bShowViewArcs;
	static bool bShowAvoidanceAngles;

private:

	bool bMeleeMode;				//to allow an entity to ignore the avoidance grid if it needs to get closer

	bool bActive;					//should entity do any processing
	int iID;						//dbpro object number
	//int iTeam;						//heros and entities broken into team numbers
	int iAggressiveness;			//0=defenisve (shoot and hide), 1=offensive (chase, flank)
	int iSearchPoints;				//number of places entity has searched
	int iCurrentTargetIndex;		//references sTarget_list
	int iInvestigateUrgency;		//the urgency of the sound being investigated

	int iDefendContainer;		//the container the defend point is in

	bool bCanDuck;
	bool bCanAttack;
	bool bCanStrafe;
	bool bCanHear;
	bool bCanRoam;
	bool bCanSearch;
	bool bCanSelectTargets;

	bool bInVerticalCover;			//cover where you can shoot around the side
	bool bInDuckingCover;			//cover where you can stand up and shoot

	bool bManualControl;			//user is controlling entity actions, no states

	bool bHitSomething;				//obstruction in the entity's path
	bool bHitBySomething;			//obstructing someone else's path
	Entity *pHitEntity;				//what it hit
	Entity *pHitByEntity;			//something pushing this entity
	
	GGVECTOR3 vecOrigPos;			//where the entity was during its last idle state
	GGVECTOR3 vecDefendPos;		//important point the entity should remain near (aggressive = 0)
	GGVECTOR3 vecHidePos;			//position where the entity can fall back to
	GGVECTOR3 vecOldPos;			//position the entity was standing when it last saw the target
	GGVECTOR3 vecAvoidPos;		//position to move to avoid an obstacle
public:
	GGVECTOR3 vecLastPos;			//position at last time step
private:
	GGVECTOR3 vecSoundPos;		//position of last sound heard
	GGVECTOR3 vecInterestPos;		//point of interest, explosion, allied distress call etc
	
	GGVECTOR3 pvecDir[100];			//list of directions the entity has previously tried to go
	GGVECTOR3 vecLastDest;		//the previous destination the entity tried to reach
	GGVECTOR3 vecFinalDest;		//state set position the entity will ultimately try to reach
	GGVECTOR3 vecLastFinalDest;   //lee - record last vecFinalDest so don't need to keep recalculating the bastern!
	GGVECTOR3 vecCurrDest;		//position the entity is heading towards (e.g. to avoid obstacles)
	GGVECTOR3 vecLastCurrDest;    //lee - record last vecCurrDest so don't need to keep recalculating the bastern!
	GGVECTOR3 vecLookAt;			//state set position the entity should look
	GGVECTOR3 vecCurrLookAt;		//current position the entity should look (e.g. enroute to dest)
	
	Path	cMovePath;				//calculated path to reach final destination
	Path	cSearchPath;			//places to search for the target
	Path	cAvoidPath;				//path to traverse other entities and dynamic obstacles
	
	//sObject *pTargetObject;			//any dbpro object this entity wants to know about
	//Hero	*pTargetHero;			//any hero (human) the entity wants to know about (e.g. attacking)
	//Entity	*pTargetEntity;			//any Entity this entity wants to know about (e.g. friend)
	//int		iTargetType;			//human, entity, or object

public:

	Entity	*pNextEntity;			//Linked List
	
	Entity ( );
	Entity ( int id, sObject *pObj, sObject *pObjRef, Container *pCurrContianer );
	~Entity ( );

	void SetDefaultValues ( );
	void ResetState ( );

	bool GetMeleeMode( );
	void SetMeleeMode( bool mode );

	void SetCoverPoint( sCoverPoint* pPoint );
	void ClearCoverPoint();
	sCoverPoint* GetCoverPoint();
	void MoveToCoverPoint();
	float GetLeapRange();
	float GetDiveRange();
	int GetMovePoints();
	void SetDiveMultiplier( float value );
	void SetLeapMultiplier( float value );

	void ForceMove( float x, float y );

	int GetInterestContainer( );
	bool GetActive( );
	int GetAggressiveness( );
	int GetSearchPoints( );
	int GetCurrentTargetIndex( );
	int GetInvestigateUrgency( );
	int GetDefendContainer( );
	bool GetCanDuck( );
	bool GetCanAttack( );
	bool GetCanStrafe( );
	bool GetCanHear( );
	bool GetCanRoam( );
	bool GetCanSearch( );
	bool GetCanSelectTargets( );
	bool GetCanHideBehindCorner( );
	bool GetInVerticalCover( );
	bool GetInDuckingCover( );
	bool GetManualControl( );
	bool GetIsBehindCorner( );
	bool GetCanLeap( );
	bool GetIsLeaping( );
	bool GetCanDive( );
	bool GetIsDiving();
	float GetPeekX();
	float GetPeekY();

	void SetDiving( bool value );
	void SetLeaping( bool value );

	float GetOrigPos( DWORD flag );
	float GetDefendPos( DWORD flag );
	float GetHidePos( DWORD flag );
	float GetOldPos( DWORD flag );
	float GetAvoidPos( DWORD flag );
	float GetLastPos( DWORD flag );
	float GetSoundPos( DWORD flag );
	float GetInterestPos( DWORD flag );
	float GetLastDest( DWORD flag );
	float GetFinalDest( DWORD flag );
	float GetCurrDest( DWORD flag );
	float GetLookAt( DWORD flag );
	float GetCurrLookAt( DWORD flag );

	Path* GetSearchPath( );
	float GetSearchPointX( DWORD index );
	float GetSearchPointY( DWORD index );
	int GetSearchPointContainer( DWORD index );

	void ForceOutOfCover( bool mode );
	bool IsForcedOutOfCover();
	void ReturnToPreviousState();

	void RemoveFromMovementList();
	void SetActive( bool bValue );
	void SetAggressiveness( int iValue );
	void SetSearchPoints( int iValue );
	void IncSearchPoints( int iValue );
	void SetCurrentTargetIndex( int iValue );
	void SetInvestigateUrgency( int iValue );
	void SetDefendContainer( int iValue );
	void SetCanDuck( bool bValue );
	void SetCanAttack( bool bValue );
	void SetCanStrafe( bool bValue );
	void SetCanHear( bool bValue );
	void SetCanRoam( bool bValue );
	void SetCanSearch( bool bValue );
	void SetCanSelectTargets( bool bValue );
	void SetInVerticalCover( bool bValue );
	void SetInDuckingCover( bool bValue );
	void SetManualControl( bool bValue );
	void SetCanHideBehindCorner( bool bValue );
	void SetCanDive( bool bValue );
	void SetCanLeap( bool bValue );

	void SetIsBehindCorner( bool bValue );
	void SetHideDirection( float dirx, float diry );
	void SetPeekingPos( float x, float y );
	float GetHideX();
	float GetHideY();

	void SetDiveRange( float fValue );
	void SetLeapRange( float fValue );

	void SetHitSomething( bool bValue );
	void SetHitBySomething( bool bValue );
	void SetHitEntity( Entity *pValue );
	void SetHitByEntity( Entity *pValue );
	
	void SetOrigPos( float x, float y, float z, DWORD flag );
	void SetDefendPos( float x, float y, float z, DWORD flag );
	void SetHidePos( float x, float y, float z, DWORD flag );
	void SetOldPos( float x, float y, float z, DWORD flag );
	void SetAvoidPos( float x, float y, float z, DWORD flag );
	void SetLastPos( float x, float y, float z, DWORD flag );
	void SetSoundPos( float x, float y, float z, DWORD flag );
	void SetInterestPos( float x, float y, float z, DWORD flag );
	void SetLastDest( float x, float y, float z, DWORD flag );
	void SetFinalDest( float x, float y, float z, DWORD flag );
	void SetCurrDest( float x, float y, float z, DWORD flag );
	void SetLookAt( float x, float y, float z, DWORD flag );
	void SetCurrLookAt( float x, float y, float z, DWORD flag );

	void ClearSearchPath( );

	void SetRadius ( float fNewRadius );
	float GetRadius ( );

	void SetContainer ( Container *pNewContainer );
	Container *GetContainer ( );
	bool IsChangingContainers( );

	bool Intersect ( float fSX, float fSZ, float fEX, float fEZ, int *iSide );

	void SetPosition ( float x, float y, float z );
	void StayWithinContainer ( float fLastX, float fLastZ, float* pX, float* pZ );
	void SetYPosition ( float y );
	void SetDestination ( float x, float y, float z, int container );
	void SetDestinationForced ( float x, float y, float z, int container, bool bMoveAddition );
	void SetIdlePos ( float x, float y, float z, int container );

	void SetAngleY( float angY );
	void SetHeight( float height );

	void SetTeam ( Team *pNewTeam );
	
	float GetHeight( );
	float GetFullHeight( );
	int GetID ( );
	float GetX ( );
	float GetY ( );
	float GetZ ( );
	float GetAngleY ( );
	bool IsTurning ( );
	bool IsAvoiding ( );
	bool IsMakingProgress ( );
	bool IsMoving ( );

	float GetDestX ( );
	float GetDestY ( );
	float GetDestZ ( );

	float GetTargetX ( );
	float GetTargetY ( );
	float GetTargetZ ( );
	int	  GetTargetContainer( );

	float GetTargetGuessX ( );
	float GetTargetGuessY ( );
	float GetTargetGuessZ ( );

	float GetTargetDirX ( );
	float GetTargetDirY ( );
	float GetTargetDirZ ( );

	int GetTargetType ( );
	bool GetIsZoneTarget( );
	int GetTargetID ( int iIndex );
	bool GetIsFiring ( );
	bool GetIsDucking ( );

	float GetDistToDest ( );
	float GetDistToTarget ( );
	float GetDistTo ( GGVECTOR3 vecPoint );
	float GetDistTo ( float x, float y, float z );
	float GetSqrDistToDest ( );
	float GetSqrDistToTarget ( );
	float GetSqrDistTo ( GGVECTOR3 vecPoint );
	float GetSqrDistTo ( float x, float y, float z );

	char* GetStateName ( );

	void SetPatrolPath ( Path* pPath, int iPathID );
	void RemovePatrolPath ( );
	bool PatrolPathExist ( );
	int CountPatrolPoints ( );
	void CheckPath ( );

	void CheckTargets ( );
	bool FindTargets ( );
	int CountTargets ( );
	bool ValidTarget ( );
	bool IsTarget ( Entity *pEntity );
	bool IsTarget ( Hero *pHero );
	void AddTarget ( Entity *pOtherEntity, bool bFromZone );
	void AddTarget ( Hero *pHero, bool bFromZone );
	void RemoveTarget ( Entity *pOtherEntity );
	void RemoveTarget ( Hero *pHero );
	void UpdateTargets ( float fTimeDelta );

	bool CanFire ( );
	
	void SetGettingHit ( float x, float y, float z );
	bool IsHit ( );
	int CountHitPoints ( );
	GGVECTOR3 GetHitDir ( int iIndex );
	GGVECTOR3 GetHitDirAvg ( );
	float GetHitSpread ( );

	void MarkOldPosition ( );

	bool IsAllied ( Entity* pAskingEntity );
	bool IsEnemy ( Entity* pAskingEntity );
	bool IsNeutral ( );
	bool IsFriendly ( );

	void SetMaxSpeed ( float fNewSpeed );
	void SetSpeed ( float fNewSpeed );
	void SetTurnSpeed ( float fNewTurnSpeed );
	void SetFireArc ( float fNewFireArc );
	void SetViewArc ( float fNewViewArcInner, float fNewViewArcOuter );
	void SetViewRange ( float fNewViewRange );
	void SetHearingRange ( float fNewHearingRange );
	void SetHearingThreshold ( int iNewThreshold );
	int GetHearingThreshold ( );
	void SetPathStartCostLimit ( float fNewDist );
	float GetPathStartCostLimit ( );

	void SetAttackDist( float fNewDist );
	void SetAvoidDist( float fNewDist );
	void SetAvoidMode( int mode );
	void SetAlwaysActive ( bool flag );
	int GetAvoidMode( );
	float GetAvoidDist( );

	void SetNeutral ( bool bIsNeutral );

	void SetDefendArea ( float fDist, int container );
	void SetDefendHero ( float fDist );
	bool InDefendArea ( float x, float y, float z );
	void SetDefendDist ( float fDist );
	float GetDefendDist ( );
	void SetDefending ( bool bIsDefending );
	bool GetIsDefending ( );

	void SetInvestigatePosToHit ( );
	void SetInvestigatePosToSound ( );
	void SetNoInvestigatePos ( );

	void ZoneEnterEvent ( Entity *pEntityIn );
	void ZoneEnterEvent ( Hero *pHeroIn );

	void ZoneLeaveEvent ( Entity *pEntityOut );
	void ZoneLeaveEvent ( Hero *pHeroOut );

	void SetFollowing ( bool fFollowing );
	bool GetFollowing ( );

	float GetMaxSpeed ( );
	float GetViewRange ( );

	void SetStateTimer ( float fNewTime );
	void SetInvestigateTimer ( float fNewTime );
	void SetCoverTimer ( float fNewTime );
	void SetSearchTimer ( float fNewTime );
	float GetStateTimer ( );
	float GetInvestigateTimer ( );
	float GetCoverTimer ( );
	float GetLookTimer ( );
	float GetResetTimer ( );
	float GetSearchTimer( );

	void SetAvgPatrolTime ( float fNewTime );
	float GetAvgPatrolTime ( );

	bool HeardSound ( );
	int GetLastSoundUrgency ( );

	int CanSee ( float fX, float fY, float fZ, bool bGround );
	int CouldSee ( float fX, float fY, float fZ, bool bGround );
	int CanSee ( Entity* );
	int CanSee ( Hero* );
	bool CanSeeTarget ( );

	int IsClose ( float fDist );
	int IsCloseSqr ( float fDist );
	bool IsTooClose ( float fDist );
	bool IsTooCloseSqr ( float fDist );
	bool IsLookingAtObstacle ( float fDist );

	void PredictTargetPosition ( float fPredDist );
	
	void Stop ( );
	void StopNoMoveAddition ( );
	void LookAtDest ( );
	void LookAtTarget ( );
	void LookAtHideDir ( );
	void LookAt ( float x, float z );
	void SetNoLookPoint ( );
	void MoveToTarget ( );
	void MoveToInterest ( );
	void MoveToGuessPos ( );
	void MoveToOriginalPos ( );
	void MoveToDefendPos ( );
	void MoveToPatrolPos ( );
	void MoveAwayFromHit ( );
	void MoveAwayFromSound ( );
	bool MoveToCover ( );
	void MoveToIdlePos ( );
	void MoveTowards ( float x, float z, float dist, int container = -1 );

	void SearchArea ( );
	void LookAround ( float fStart, float fEnd);
	void SweepViewForward ( float fMaxAng );
	void RandomMove ( float fMin, float fMax );
	void MoveClose ( float fDestX, float fDestY, float fDestZ, float fDist, int container );
	void StrafeTarget ( );
	void StrafeAvoid ( );
	void CalculateAvoidPosition ( float fDist );
	void Duck ( );
	void Stand ( );

	void RebuildDynamicPathFinder( );

	void TurnToAngle ( float ftimeDelta, bool bMoving );
	//void SearchArea ( );
	void FireWeapon ( );

	void ResetFeedbackPos ( );
	void UpdateFeedbackPos ( );

	bool DirectionBlocked ( float fDirX, float fDirZ );
	bool DirectionBlocked ( float fDir );
	bool AdjustDirection ( float *pDir );
	void AdjustDestination ( );
	
	void ClearSounds ( );
	void CheckBeacons ( Beacon *pBeacon );
	void CheckZones ( Zone *pZone );
	void UpdateObstacleListCountDown ( float fTimeDelta );
	void UpdateObstacleList ( float fTimeDelta );
	void UpdateTimers ( float fTimeDelta );

	void UpdateMovement ( float fTimeDelta );
	void RemoveFromThread ( void );
	void UpdateState ( );

	void ChangeState ( State *pNewState );

	void DebugDrawDestination ( );
	void DebugHideDestination ( );

	void DebugDrawObstacleAngles ( );
	void DebugHideObstacleAngles ( );

	void DebugDrawViewArcs ( );					//and hearing range
	void DebugHideViewArcs ( );

	void DebugOutputInternalData ( HANDLE hOut );
};

#endif
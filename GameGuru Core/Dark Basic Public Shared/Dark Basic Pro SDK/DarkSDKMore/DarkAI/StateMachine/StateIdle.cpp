#include "StateSet.h"

int StateSet::StateIdle::iNumUsers = 0;

StateSet::StateIdle::StateIdle ( ) { iNumUsers = 0; }
StateSet::StateIdle::~StateIdle ( ) { }

void StateSet::StateIdle::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	pEntity->SetStateTimer ( 2.0f );
	pEntity->Stand ( );
}
void StateSet::StateIdle::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateIdle::Execute ( Entity *pEntity )
{
	if ( pEntity->GetStateTimer ( ) <= 0 )
	{
		//pEntity->vecFinalDest = pEntity->vecOrigPos;
		if ( !pEntity->GetFollowing ( ) ) 
		{
			if ( pEntity->CountPatrolPoints( ) > 0 )
			{
				pEntity->ChangeState ( pStateSet->pStatePatrol );
				return;
			}
			else
			{
				if ( pEntity->GetAggressiveness() != 2 )
				{
					if ( !pEntity->GetCanRoam() ) pEntity->MoveToOriginalPos ( );
					else 
					{
						if ( !pEntity->IsMoving( ) && pWorld->RandInt( 5 ) == 0 ) pEntity->RandomMove ( pEntity->GetRadius( ), pEntity->GetRadius( )*10.0f );
					}
				}
			}
		}
		
		pEntity->LookAtDest ( );

		/*
		if ( !pEntity->IsMoving( ) )
		{
			if ( !pEntity->IsTurning( ) && pEntity->IsLookingAtObstacle( ) )
			{
				pEntity->LookAround( 40, 90 );
			}
		}*/
	}
	else
	{
		pEntity->Stop ( );
	}
	
	if ( pEntity->ValidTarget( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0:		pEntity->ChangeState ( pStateSet->pStateAttack );		break;
			case 1:		pEntity->ChangeState ( pStateSet->pStateChaseAttack );	break;
			case 2:		pEntity->ChangeState ( pStateSet->pStateAttack );		break;

			default:	pEntity->ChangeState ( pStateSet->pStateAttack );
		}

		return;
	}
	else
	{
		if ( pEntity->CountTargets( ) > 0 )
		{	
			if ( pEntity->GetAggressiveness() == 1 )
			{
				pEntity->MoveToGuessPos ( );
				pEntity->LookAtDest ( );
				
				pEntity->ChangeState ( pStateSet->pStateGoToDest );
			}

			if ( pEntity->GetAggressiveness() == 0 )
			{
				if ( pEntity->GetIsZoneTarget( ) )
				{
					pEntity->MoveToTarget ( );
					pEntity->LookAtDest ( );
				
					pEntity->ChangeState ( pStateSet->pStateGoToDest );
				}
				else
				{
					pEntity->LookAtTarget( );
				}
			}
		}
	}

	if ( pEntity->IsHit ( ) )
	{
		switch ( pEntity->CountHitPoints ( ) )
		{
			case 0:
			{
				if ( pEntity->GetAggressiveness() == 3 )
				{
					pEntity->RandomMove ( pEntity->GetRadius( ), pEntity->GetRadius( )*3.0f );
				}
				else if ( pEntity->GetAggressiveness() == 2 )
				{
					pEntity->LookAround( 90, 120 );
				}
				else
				{
					pEntity->SetNoInvestigatePos( );
					pEntity->ChangeState ( pStateSet->pStateInvestigate );
				}
			}
			break;

			case 1:
			{
				if ( pEntity->GetAggressiveness() == 3 )
				{
					pEntity->MoveAwayFromHit( );
					//pEntity->ChangeState ( pStateSet->pStateGoToDest );
				}
				else if ( pEntity->GetAggressiveness() == 2 )
				{
					pEntity->LookAt( pEntity->GetHitDir(0).x, pEntity->GetHitDir(0).z );
				}
				else
				{
					pEntity->SetInvestigatePosToHit ( );
					pEntity->ChangeState ( pStateSet->pStateInvestigate );
				}
			} 
			break;
			
			default:
			{
				if ( pEntity->GetAggressiveness() == 3 ) { pEntity->MoveAwayFromHit( ); break; }
				if ( pEntity->GetAggressiveness() == 2 ) { pEntity->LookAt( pEntity->GetHitDir(0).x, pEntity->GetHitDir(0).z); break; }
				pEntity->SetInvestigatePosToHit ( );
				pEntity->ChangeState ( pStateSet->pStateInvestigate );
				//if ( rand( ) % 5 == 0 ) pEntity->ChangeState ( pStateSet->pStateFallBack );
			}
		}

		return;
	}

	if ( pEntity->HeardSound ( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 1: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 2: pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) ); break;
			case 3: 
				{
					bool bMoved = pEntity->MoveToCover ( );
					if ( bMoved ) {
						pEntity->ChangeState ( pStateSet->pStateGoToDest );
					}
					else
					{
						pEntity->Duck( );
					}
				}
				break;
		}
	}
}
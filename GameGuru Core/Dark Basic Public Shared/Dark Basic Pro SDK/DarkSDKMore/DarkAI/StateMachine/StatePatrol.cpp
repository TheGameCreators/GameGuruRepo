#include "StateSet.h"

int StateSet::StatePatrol::iNumUsers = 0;

StateSet::StatePatrol::StatePatrol ( ) { iNumUsers = 0; }
StateSet::StatePatrol::~StatePatrol ( ) { }

void StateSet::StatePatrol::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	pEntity->Stand ( );
	pEntity->SetStateTimer( ( rand( ) * 1.0f ) / RAND_MAX + 1.0f );
	//pEntity->SetStateTimer( ( rand( ) * 0.5f ) / RAND_MAX + pEntity->GetAvgPatrolTime( ) );
}
void StateSet::StatePatrol::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StatePatrol::Execute ( Entity *pEntity )
{
	//if ( !pEntity->PatrolPathExist( ) ) pEntity->ChangeState ( pStateSet->pStateIdle );
	//if ( pEntity->CountPatrolPoints( ) == 0 ) pEntity->ChangeState ( pStateSet->pStateIdle );
	//if ( pEntity->iAggressiveness == 2 ) pEntity->ChangeState ( pStateSet->pStateIdle );
	
	if ( pEntity->GetStateTimer( ) <= 0.0f )
	{
		pEntity->MoveToPatrolPos ( );
		pEntity->SetNoLookPoint ( );
		//if ( rand( ) % 50 == 0 ) pEntity->SweepViewForward ( 30.0f );
	}
	else
	{
		if ( pWorld->RandInt( 2 ) == 0 ) pEntity->LookAround ( 90.0f, 140.0f );
	}

	if ( pEntity->ValidTarget( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0:		pEntity->ChangeState ( pStateSet->pStateAttack );		break;
			case 1:		pEntity->ChangeState ( pStateSet->pStateChaseAttack );	break;
			case 3:		break;

			default:	pEntity->ChangeState ( pStateSet->pStateAttack );
		}

		return;
	}
	else
	{
		if ( pEntity->CountTargets( ) > 0 )
		{
			pEntity->MoveToGuessPos ( );
			pEntity->LookAtDest ( );
			
			pEntity->ChangeState ( pStateSet->pStateGoToDest );
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
				else
				{
					pEntity->SetInvestigatePosToHit ( );
					pEntity->ChangeState ( pStateSet->pStateInvestigate );
				}
			} 
			break;
			
			default:
			{
				//if ( pEntity->iAggressiveness != 3 && rand( ) % 5 == 0 ) pEntity->ChangeState ( pStateSet->pStateFallBack );
				if ( pEntity->GetAggressiveness() == 3 )
				{
					pEntity->MoveAwayFromHit( );
					//pEntity->ChangeState ( pStateSet->pStateGoToDest );
				}
				else
				{
					pEntity->SetInvestigatePosToHit ( );
					pEntity->ChangeState ( pStateSet->pStateInvestigate );
				}
			}
		}

		return;
	}

	if ( pEntity->HeardSound ( ) )
	{
		if ( pEntity->GetAggressiveness() != 3 )
		{
			pEntity->ChangeState ( pStateSet->pStateInvestigate );
		}
		else
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
	}
}
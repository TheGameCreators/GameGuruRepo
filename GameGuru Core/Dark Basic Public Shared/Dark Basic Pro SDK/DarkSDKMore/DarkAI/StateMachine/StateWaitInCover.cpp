
#include "StateSet.h"

int StateSet::StateWaitInCover::iNumUsers = 0;

StateSet::StateWaitInCover::StateWaitInCover ( ) { iNumUsers = 0; }
StateSet::StateWaitInCover::~StateWaitInCover ( ) { }

void StateSet::StateWaitInCover::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	if ( pEntity->GetCanDuck( ) ) pEntity->Duck ( );
	
	float fRandomTime = ( 4.0f * rand( ) ) / RAND_MAX + 1.0f;
	pEntity->SetStateTimer ( fRandomTime );
}

void StateSet::StateWaitInCover::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
	//pEntity->Stand ( );
}

void StateSet::StateWaitInCover::Execute ( Entity *pEntity )
{	
	if ( pEntity->CountTargets( ) == 0 ) 
	{
		pEntity->ChangeState( pStateSet->pStateIdle );
		return;
	}

	if ( pWorld->RandInt( 2 ) == 0 ) pEntity->Duck( );

	//if ( ( pEntity->GetIsDucking ( ) ) && pEntity->IsHit ( ) && ( rand( ) % 4 == 0 ) )
	{
		if ( pEntity->IsHit ( ) )
		{
			if ( pEntity->CountHitPoints( ) > 0 ) pEntity->LookAt( pEntity->GetHitDir( 0 ).x, pEntity->GetHitDir( 0 ).z );
		}

		if ( pEntity->ValidTarget( ) )
		{
			//pEntity->Stand ( );
			pEntity->ChangeState ( pStateSet->pStateAttack );
		}
		else
		{
			if ( pEntity->GetCanDuck() ) pEntity->Duck ( );
		}
	}

	pEntity->LookAtTarget ( );
	
	if ( pEntity->CanSeeTarget ( ) )
	{
		if ( pEntity->GetAggressiveness() != 2 )
		{
			//if ( pEntity->bCanStrafe ) pEntity->StrafeTarget( );
		}
	}
	else
	{
		if ( pEntity->HeardSound ( ) )
		{
			switch ( pEntity->GetAggressiveness() )
			{
				case 0: 
				case 1: 
				case 2: pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) ); break;
				case 3: pEntity->MoveAwayFromSound ( );
					pEntity->ChangeState ( pStateSet->pStateGoToDest );
					break;
			}
		}
	}

	if ( pEntity->GetStateTimer( ) <= 0 )
	{
		pEntity->Stand ( );
		//pEntity->ChangeState ( pStateSet->pStateAttack );
		//return;
	}

	
}
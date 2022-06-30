#include "StateSet.h"

int StateSet::StateGoToDest::iNumUsers = 0;

StateSet::StateGoToDest::StateGoToDest ( ) { iNumUsers = 0; }
StateSet::StateGoToDest::~StateGoToDest ( ) { }

void StateSet::StateGoToDest::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	pEntity->Stand ( );
	if ( pEntity->GetAggressiveness() == 2 ) pEntity->ChangeState ( pStateSet->pStateIdle );
}
void StateSet::StateGoToDest::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateGoToDest::Execute ( Entity *pEntity )
{	
	if ( pEntity->ValidTarget( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0:   	pEntity->ChangeState ( pStateSet->pStateAttack );		break;
			case 1:		pEntity->ChangeState ( pStateSet->pStateChaseAttack );	break;

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
	
	float fDist = pEntity->GetSqrDistToDest ( );
		
	if ( fDist <= 0.1f )
	{
		pEntity->Stop ( );

		switch ( pEntity->GetAggressiveness() )
		{
			case 0:		pEntity->ChangeState( pStateSet->pStateIdle );			break;
			case 1:		if ( pEntity->CountTargets( ) > 0 ) pEntity->ChangeState( pStateSet->pStateSearchArea );	
						else pEntity->ChangeState ( pStateSet->pStateIdle );
						break;

			default:	pEntity->ChangeState( pStateSet->pStateIdle );
		}
 
		return;
	}

	if ( pEntity->HeardSound ( ) || pEntity->IsHit( ) )
	{
		if ( pEntity->IsHit( ) ) pEntity->SetInvestigatePosToHit( );

		switch ( pEntity->GetAggressiveness() )
		{
			case 0: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 1: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 2: break;
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
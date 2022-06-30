
#include "StateSet.h"

int StateSet::StateInvestigate::iNumUsers = 0;

StateSet::StateInvestigate::StateInvestigate ( ) { iNumUsers = 0; }
StateSet::StateInvestigate::~StateInvestigate ( ) { }

void StateSet::StateInvestigate::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	pEntity->SetInvestigateUrgency( pEntity->GetLastSoundUrgency ( ) );

	float fLength = pEntity->GetSqrDistTo ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Y ), pEntity->GetInterestPos( ENT_Z ) );
	
	if ( fLength < pEntity->GetRadius()/10.0f )
	{
		pEntity->LookAround ( 20.0f, 40.0f );
		if ( pEntity->GetAggressiveness() == 1 ) pEntity->RandomMove ( 1.0f, 10.0f );
	}
	else
	{
		if ( pEntity->GetAggressiveness() == 1 || pEntity->InDefendArea ( pEntity->GetInterestPos( ENT_X ), pEntity->GetY(), pEntity->GetInterestPos( ENT_Z ) ) ) 
		{
			if ( pEntity->CountTargets( ) > 0 && pEntity->GetLastSoundUrgency ( ) < 10 )
			{
				pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) );
				pEntity->ChangeState ( pStateSet->pStateGoToDest );
			}
			else pEntity->MoveToInterest ( );
		}
		else
		{
			//if ( pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheck ( pEntity->GetX( ), pEntity->GetZ( ), pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ), 2 ) ) 
			if ( pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetHeight( ), pEntity->GetZ( ), pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Y ), pEntity->GetInterestPos( ENT_Z ), NULL ) ) 
			{
				if ( pEntity->CountTargets( ) > 0 && pEntity->GetLastSoundUrgency ( ) < 10 )
				{
					pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) );
					pEntity->ChangeState ( pStateSet->pStateGoToDest );
				}
				else pEntity->MoveToInterest ( );
			}
			else
			{
				if ( pEntity->CountTargets( ) > 0 && pEntity->GetLastSoundUrgency ( ) < 10 )
				{
					pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) );
					pEntity->ChangeState ( pStateSet->pStateGoToDest );
				}
				else pEntity->MoveTowards ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ), pEntity->GetRadius( )*4.0f, pEntity->GetInterestContainer() );
			}
		}

		pEntity->LookAt ( pEntity->GetInterestPos( ENT_X ), pEntity->GetInterestPos( ENT_Z ) );
	}

	float fRandNum = ( 2.0f * rand( ) ) / RAND_MAX + 3.0f;
	pEntity->SetStateTimer ( fRandNum );

	fRandNum = ( 5.0f * rand( ) ) / RAND_MAX + 10.0f;
	pEntity->SetInvestigateTimer ( fRandNum );
}
void StateSet::StateInvestigate::Exit    ( Entity *pEntity ) { iNumUsers--; pEntity->SetInvestigateUrgency( 0 ); }

void StateSet::StateInvestigate::Execute ( Entity *pEntity )
{
	int iRandChoice = rand ( ) % 4;
	float fRandNum = ( 2.0f * rand( ) ) / RAND_MAX + 2.0f;
	
	if ( pEntity->GetInvestigateTimer ( ) <= 0.0f )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	if ( pEntity->GetSqrDistToDest ( ) < 0.1f )
	{
		if ( pEntity->GetStateTimer ( ) <= 0 )
		{
			if ( iRandChoice == 0 )
			{
				//MessageBox ( NULL, "Investigated Enough", "Info", 0 );
				pEntity->ChangeState ( pStateSet->pStateIdle );
			}
			else
			{
				pEntity->LookAround ( 40.0f, 180.0f );
				pEntity->SetStateTimer ( fRandNum );
			}
		}
	}
	else
	{
		//pEntity->MoveToInterest ( );
		//pEntity->vecLookAt = pEntity->vecInterestPos;

		if ( pEntity->GetLookTimer ( ) <= 0.0f ) pEntity->SweepViewForward ( 60.0f );
	}
	
	if ( pEntity->ValidTarget( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0:		pEntity->ChangeState ( pStateSet->pStateAttack );		break;
			case 1:		pEntity->ChangeState ( pStateSet->pStateChaseAttack );	break;

			default:	pEntity->ChangeState ( pStateSet->pStateAttack );
		}

		return;
	}
	else
	{
		if ( pEntity->CountTargets( ) > 0 )
		{
			//if ( pEntity->CanSee( pEntity->vecInterestPos.x, 0, pEntity->vecInterestPos.z, false ) )
			if ( pEntity->GetLastSoundUrgency ( ) < 10 )
			{
				pEntity->MoveToGuessPos ( );
				pEntity->LookAtDest ( );
			
				pEntity->ChangeState ( pStateSet->pStateGoToDest );
			}
		}
	}

	if ( pEntity->IsHit ( ) )
	{
		switch ( pEntity->CountHitPoints ( ) )
		{
			case 0:
			{
				pEntity->SetNoInvestigatePos ( );
				pEntity->ChangeState ( pStateSet->pStateInvestigate );
			}
			break;

			case 1:
			{
				pEntity->SetInvestigatePosToHit ( );
				pEntity->ChangeState ( pStateSet->pStateInvestigate );
			} 
			break;
			
			default:
			{
				//pEntity->ChangeState ( pStateSet->pStateFallBack );
				pEntity->SetInvestigatePosToHit ( );
				pEntity->ChangeState ( pStateSet->pStateInvestigate );
			}
		}

		return;
	}

	if ( pEntity->HeardSound ( ) && pEntity->GetInvestigateUrgency() <= pEntity->GetLastSoundUrgency( ) )
	{
		switch ( pEntity->GetAggressiveness( ) )
		{
			case 0: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 1: pEntity->ChangeState ( pStateSet->pStateInvestigate ); break;
			case 2: break;
			case 3: pEntity->MoveAwayFromSound ( );
				pEntity->ChangeState ( pStateSet->pStateGoToDest );
				break;
		}
	}
}

#include "StateSet.h"

int StateSet::StateStrafeAttack::iNumUsers = 0;

StateSet::StateStrafeAttack::StateStrafeAttack ( ) { iNumUsers = 0; }
StateSet::StateStrafeAttack::~StateStrafeAttack ( ) { }

void StateSet::StateStrafeAttack::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++;

	if ( !pEntity->GetCanAttack() )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	pEntity->Stand( );
	
	if ( pEntity->GetCanStrafe() ) pEntity->StrafeTarget ( );
}

void StateSet::StateStrafeAttack::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
}

void StateSet::StateStrafeAttack::Execute ( Entity *pEntity )
{
	float fSqrDist = pEntity->GetSqrDistToTarget ( );
	
	if ( pEntity->ValidTarget( ) )
	{
		//float fMinimumDist = 16.0f;

		if ( pEntity->IsTooCloseSqr ( fSqrDist ) )
		{
			pEntity->Stop ( );
			pEntity->ChangeState ( pStateSet->pStateAttack );
		}
		
		fSqrDist = pEntity->GetSqrDistToDest ( );
		
		//reached the end of the strafing maneuver
		if ( fSqrDist < 0.1f )
		{
			int iChoice = rand( ) % 2;

			switch ( iChoice )
			{
				case 0: 
					{
						if ( pEntity->GetAggressiveness() == 0 )
							pEntity->ChangeState ( pStateSet->pStateAttack );
						else
							pEntity->ChangeState ( pStateSet->pStateChaseAttack );
					}
					break;

				case 1: pEntity->ChangeState ( pStateSet->pStateStrafeAttack ); break;

				default: pEntity->ChangeState ( pStateSet->pStateAttack );
			}
		}

		pEntity->LookAtTarget ( );

		//if ( pEntity->CanFire ( ) ) pEntity->FireWeapon ( );
	}
	else
	{
		if ( pEntity->CountTargets( ) == 0 )
		{
			pEntity->ChangeState ( pStateSet->pStateIdle );
			return;
		}
		
		if ( pEntity->GetAggressiveness() == 0 )
		{
			//pEntity->ChangeState ( pStateSet->pStateFallBack );
			if ( pEntity->GetCanDuck() )
			{
				pEntity->Duck ( );
				pEntity->SetInDuckingCover( true );
				pEntity->SetInVerticalCover( false );
				pEntity->SetCoverTimer ( 8.0f );
				pEntity->ChangeState ( pStateSet->pStateWaitInCover );
			}

			return;
		}

		if ( pEntity->GetAggressiveness() == 1 )
		{
			//pEntity->vecLastKnownPos = pEntity->vecLookAt;

			pEntity->PredictTargetPosition ( 5.0f );
			
			//pEntity->vecFinalDest = pEntity->vecGuessNewPos;
			pEntity->MoveToGuessPos ( );
			float x = pEntity->GetFinalDest( ENT_X );
			float y = pEntity->GetFinalDest( ENT_Y );
			float z = pEntity->GetFinalDest( ENT_Z );
			pEntity->SetLookAt( x,y,z, ENT_X|ENT_Y|ENT_Z );

			pEntity->MarkOldPosition ( );
			
			pEntity->ChangeState ( pStateSet->pStateGoToDest );
		}
		else
		{
			pEntity->ChangeState ( pStateSet->pStateIdle );
		}
	}
}
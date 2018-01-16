#include "StateSet.h"

int StateSet::StateChaseAttack::iNumUsers = 0;

StateSet::StateChaseAttack::StateChaseAttack ( ) { iNumUsers = 0; }
StateSet::StateChaseAttack::~StateChaseAttack ( ) { }

void StateSet::StateChaseAttack::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 

	if ( !pEntity->GetCanAttack() )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	pEntity->Stand ( );
}
void StateSet::StateChaseAttack::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateChaseAttack::Execute ( Entity *pEntity )
{
	if ( pEntity->ValidTarget( ) )
	{
		pEntity->MoveToTarget ( );
		pEntity->LookAtTarget ( );

		float fSqrDist = pEntity->GetSqrDistToTarget ( );

		//float fMinimumDist = 16.0f;

		if ( pEntity->IsHit ( ) )
		{
			//bool bCover = pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX ( ), pEntity->GetTargetZ ( ), 0 );
			bool bCover = pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetY( ) + pEntity->GetFullHeight( )/2, pEntity->GetZ( ), pEntity->GetTargetX ( ), pEntity->GetTargetY ( ), pEntity->GetTargetZ ( ), 0 );

			if ( pEntity->GetAggressiveness() != 2 )
			{
				if ( !bCover || !pEntity->GetCanDuck() )
				{
					if ( rand()%1==0 )
					{
						pEntity->ChangeState ( pStateSet->pStateAttackFromCover );
						return;
					}

					if ( pEntity->GetCanHideBehindCorner() && pEntity->GetCanStrafe() )
					{
						if ( rand()%2 == 0 ) pEntity->ChangeState ( pStateSet->pStatePeekFromCorner );
						else pEntity->ChangeState ( pStateSet->pStateStrafeAttack );
						return;
					}

					if ( pEntity->GetCanHideBehindCorner() )
					{
						pEntity->ChangeState ( pStateSet->pStatePeekFromCorner );
						return;
					}

					if ( pEntity->GetCanStrafe() )
					{
						pEntity->ChangeState ( pStateSet->pStateStrafeAttack );
						return;
					}
				}
				else
				{
					pEntity->ChangeState ( pStateSet->pStateAttackFromCover );
				}
			}
			else
			{
				if ( pEntity->GetCanDuck() )
				{
					pEntity->Duck ( );
					pEntity->SetCoverTimer ( 8.0f );
					pEntity->ChangeState ( pStateSet->pStateWaitInCover );
					return;
				}
			}
		}

		if ( pEntity->IsCloseSqr ( fSqrDist ) || pEntity->IsTooCloseSqr ( fSqrDist ) )
		{
			pEntity->Stop ( );
			pEntity->ChangeState ( pStateSet->pStateAttack );
		}

		//if ( pEntity->CanFire ( ) ) pEntity->FireWeapon ( );
	}
	else
	{
		if ( pEntity->CountTargets( ) == 0 )
		{
			pEntity->ChangeState ( pStateSet->pStateIdle );
			return;
		}

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
}
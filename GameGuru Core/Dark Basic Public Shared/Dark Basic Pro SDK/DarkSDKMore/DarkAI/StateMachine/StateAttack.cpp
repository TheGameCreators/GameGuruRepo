#include "StateSet.h"

int StateSet::StateAttack::iNumUsers = 0;

StateSet::StateAttack::StateAttack ( ) { iNumUsers = 0; }
StateSet::StateAttack::~StateAttack ( ) { }

void StateSet::StateAttack::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	
	if ( !pEntity->GetCanAttack() )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	pEntity->Stop ( );
}
void StateSet::StateAttack::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateAttack::Execute ( Entity *pEntity )
{
	if ( pEntity->ValidTarget( ) )
	{
		float fDist = pEntity->GetSqrDistToTarget ( );

		//float fMinimumDist = 16.0f;

		if ( pEntity->IsTooCloseSqr ( fDist ) && pEntity->GetAggressiveness() != 2 )
		{
			pEntity->Stand( );
			pEntity->StrafeAvoid ( );
		}
		else
		{
			//bool bCover = pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX ( ), pEntity->GetTargetZ ( ), 0 );
			bool bCover = pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetY( ) + pEntity->GetFullHeight( )/2, pEntity->GetZ( ), pEntity->GetTargetX ( ), pEntity->GetTargetY ( ), pEntity->GetTargetZ ( ), 0 );

			if ( pEntity->IsHit ( ) || ( pWorld->RandInt( 3 ) == 0 ) )
			{
				if ( pEntity->GetAggressiveness() != 2 )
				{
					if ( !bCover || !pEntity->GetCanDuck() )
					{
						if ( pEntity->GetAggressiveness() == 0 && rand()%2==0 )
						{
							pEntity->ChangeState ( pStateSet->pStateAttackFromCover );
							return;
						}

						if ( pEntity->GetAggressiveness() == 1 && rand()%1==0 )
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
						return;
					}
				}
				else
				{
					if ( pEntity->GetCanDuck() )
					{
						pEntity->Duck ( );
						pEntity->SetInDuckingCover( true );
						pEntity->SetInVerticalCover( false );
						pEntity->SetCoverTimer ( 8.0f );
						pEntity->ChangeState ( pStateSet->pStateWaitInCover );

						return;
					}
					else
					{
						pEntity->Stop( );
					}
				}
			}
			else
			{
				pEntity->Stop ( );
			}
		}

		if ( ( pEntity->GetAggressiveness() == 1 ) && ( !pEntity->IsTooCloseSqr ( fDist ) ) && ( !pEntity->IsCloseSqr ( fDist ) ) && pEntity->GetTargetContainer()==pEntity->GetContainer()->GetID() )
		{
			pEntity->ChangeState ( pStateSet->pStateChaseAttack );
		}

		if ( pEntity->GetAggressiveness() == 0 && pEntity->GetCanHideBehindCorner() && pWorld->RandInt( 1 ) == 0 )
		{
			if ( rand()%2 == 0 ) pEntity->ChangeState( pStateSet->pStatePeekFromCorner );
			else pEntity->ChangeState( pStateSet->pStateAttackFromCover );
		}

		pEntity->LookAtTarget ( );
		
		//if ( pEntity->CanFire ( ) ) pEntity->FireWeapon ( );
	}
	else
	{
		if ( pEntity->CountTargets ( ) == 0 )
		{
			pEntity->ChangeState ( pStateSet->pStateIdle );
			return;
		}

		if ( pEntity->GetAggressiveness() == 2 )
		{
			if ( pEntity->GetCanDuck() && rand( ) % 50 == 0 ) 
			{
				pEntity->Duck ( );
				pEntity->SetInDuckingCover( true );
				pEntity->SetInVerticalCover( false );
				pEntity->SetCoverTimer ( 8.0f );
				pEntity->ChangeState ( pStateSet->pStateWaitInCover );
			}

			return;
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
				if ( pEntity->GetCanDuck() )
				{
					pEntity->Duck ( );
					//pEntity->bInDuckingCover = true;
					pEntity->SetInVerticalCover( false );
					pEntity->SetCoverTimer ( 8.0f );
					pEntity->ChangeState ( pStateSet->pStateWaitInCover );
				}
			}

			return;
		}

		if ( pEntity->GetAggressiveness() == 1 )
		{
			//pEntity->vecLastKnownPos = pEntity->vecLookAt;

			pEntity->PredictTargetPosition ( 5.0f );
			
			//pEntity->vecFinalDest = pEntity->vecGuessNewPos;
			pEntity->MoveToGuessPos ( );
			//pEntity->vecLookAt = pEntity->vecFinalDest;
			pEntity->LookAtDest ( );

			pEntity->MarkOldPosition ( );
			
			pEntity->ChangeState ( pStateSet->pStateGoToDest );
			return;
		}
	}
}
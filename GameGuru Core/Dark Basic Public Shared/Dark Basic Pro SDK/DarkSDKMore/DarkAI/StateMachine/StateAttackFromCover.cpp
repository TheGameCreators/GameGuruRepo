#include "StateSet.h"

int StateSet::StateAttackFromCover::iNumUsers = 0;

StateSet::StateAttackFromCover::StateAttackFromCover ( ) { iNumUsers = 0; }
StateSet::StateAttackFromCover::~StateAttackFromCover ( ) { }

void StateSet::StateAttackFromCover::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	
	if ( !pEntity->GetCanDuck() )
	{
		pEntity->ReturnToPreviousState();
		return;
	}

	// check if entity has already chosen a cover point
	if ( pEntity->GetCoverPoint() )
	{
		pEntity->MoveToCoverPoint();
		return;
	}

	// look for a cover point
	Path cPath;
	pEntity->GetContainer( )->pPathFinder->SearchCoverPoints ( pEntity->GetX(), pEntity->GetZ(), pEntity->GetTargetX(), pEntity->GetTargetZ(), &cPath );

	int iNumPoints = cPath.CountPoints ( );
	if ( iNumPoints == 0 )
	{
		//no where to go
		pEntity->ReturnToPreviousState();
		return;
	}

	pEntity->Stand ( );
	
	//get entity->target vector
	float fDirX = pEntity->GetTargetX( ) - pEntity->GetX ( );
	float fDirZ = pEntity->GetTargetZ( ) - pEntity->GetZ ( );

	float fRange = ( fDirX*fDirX + fDirZ*fDirZ );
	float fTargetDist = sqrt( fRange );

	bool bFound = false;
	int iCount = 0;

	int iIndex = -1;
	float fClosest = 1000000.0f;

	for ( int i = 0; i < iNumPoints; i++ )
	{
		float fX = cPath.GetPoint ( i ).x - pEntity->GetX ( );
		float fZ = cPath.GetPoint ( i ).y - pEntity->GetZ ( );

		float fDist = fX*fX + fZ*fZ;

		// bias distance for entity stance
		if ( pEntity->GetAggressiveness() == 1 )
		{
			// aggressive, favour forward positions
			float fDotP = (fX*fDirX + fZ*fDirZ)/fTargetDist;
			if ( fDotP < 0 ) fDist -= fDotP*2;
		}

		if ( pEntity->GetAggressiveness() == 0 )
		{
			// cautious, favour backward positions
			float fDotP = (fX*fDirX + fZ*fDirZ)/fTargetDist;
			if ( fDotP > 0 ) fDist += fDotP*2;
		}

		//choose closest
		if ( fDist < fClosest )
		{
			fClosest = fDist;
			iIndex = i;
		}
	}

	if ( !pEntity->GetIsDefending( ) || pEntity->InDefendArea ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y ) )
	{
		int coverID = cPath.GetPoint ( iIndex ).container;
		sCoverPoint *pCoverPoint = pEntity->GetContainer( )->pPathFinder->GetCoverPoint( coverID );
		pEntity->SetCoverPoint( pCoverPoint );
		pEntity->SetDestination ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y, -1 );
	}
	else pEntity->ReturnToPreviousState();
}
void StateSet::StateAttackFromCover::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateAttackFromCover::Execute ( Entity *pEntity )
{
	if ( pEntity->IsForcedOutOfCover() )
	{
		pEntity->ClearCoverPoint();
		pEntity->Stand( );
		pEntity->ChangeState( pStateSet->pStateAttack );
		return;
	}

	if ( pEntity->ValidTarget( ) )
	{
		float fDist = pEntity->GetSqrDistToTarget ( );

		// should the entity abandon its position
		if ( fDist < pEntity->GetRadius()*pEntity->GetRadius()*25 && pEntity->GetAggressiveness() != 2 )
		{
			// yes, enemy is too close
			pEntity->ClearCoverPoint();
			pEntity->Stand( );
			pEntity->ChangeState( pStateSet->pStateAttack );
			return;
		}
		else
		{
			if ( !pEntity->GetCoverPoint() )
			{
				// yes, point seems to have disappeared (could be caused by a waypoint rebuild)
				pEntity->Stand();
				pEntity->ChangeState( pStateSet->pStateAttack );
				return;
			}
			else
			{
				float dirX = pEntity->GetTargetX() - pEntity->GetX();
				float dirZ = pEntity->GetTargetZ() - pEntity->GetZ();

				if ( dirX*pEntity->GetCoverPoint()->fDirX + dirZ*pEntity->GetCoverPoint()->fDirY <= 0 )
				{
					// yes, enemy has flanked the entity
					pEntity->ClearCoverPoint();
					pEntity->Stand( );
					pEntity->ChangeState( pStateSet->pStateAttack );
					return;
				}
			}
		}

		pEntity->LookAtTarget ( );

		float fSqrDist = pEntity->GetSqrDistToDest ( );
		if ( fSqrDist > 0.1f )
		{
			// moving towards cover
			//float fRandomTime = ( 4.0f * rand( ) ) / RAND_MAX + 1.0f;
			pEntity->SetStateTimer( 1 );
			pEntity->Stand();

			// should the entity dive?
			if ( pEntity->GetMovePoints() == 2 )
			{
				float diff = fabs(pEntity->GetSqrDistToDest() - pEntity->GetDiveRange()*pEntity->GetDiveRange());
				if ( diff < pEntity->GetRadius()*pEntity->GetRadius()/100.0f )
				{
					// yes
					pEntity->ChangeState( pStateSet->pStateDiving );
					return;
				}
			}
		}
		else
		{
			// at the cover position
			if ( (pEntity->IsHit() || (pEntity->GetStateTimer() <= 0 && pWorld->RandInt( 2 ) == 0)) && !pEntity->GetIsDucking() ) 
			{
				float fRandomTime = ( 3.0f * rand( ) ) / RAND_MAX + 2.0f;
				pEntity->SetStateTimer( fRandomTime );
				pEntity->Duck( );
			}

			if ( pEntity->GetStateTimer( ) <= 0 )
			{
				pEntity->Stand ( );
			}
		}
	}
	else
	{
		if ( pEntity->GetStateTimer( ) <= 0 )
		{
			if ( pEntity->GetIsDucking() ) pEntity->Stand();
			else
			{
				if ( pEntity->CountTargets ( ) == 0 )
				{
					pEntity->ClearCoverPoint();
					pEntity->Stand();
					pEntity->ChangeState ( pStateSet->pStateIdle );
					return;
				}

				if ( pEntity->GetAggressiveness() == 2 )
				{
					if ( pEntity->GetCanDuck() && rand( ) % 50 == 0 ) 
					{
						pEntity->ClearCoverPoint();
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
						pEntity->ClearCoverPoint();
						pEntity->MoveToTarget ( );
						pEntity->LookAtDest ( );
					
						pEntity->Stand();
						pEntity->ChangeState ( pStateSet->pStateGoToDest );
					}
					else
					{
						if ( pEntity->GetCanDuck() )
						{
							pEntity->ClearCoverPoint();
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
					// should the entity try and leap over the cover?
					if ( pEntity->GetCanLeap() && pEntity->GetCoverPoint() && pEntity->GetCoverPoint()->fLeapDist > 0
					  && pEntity->GetLeapRange() >= pEntity->GetCoverPoint()->fLeapDist )
					{
						// yes
						float dirX = pEntity->GetCoverPoint()->fDirX * pEntity->GetLeapRange();
						float dirZ = pEntity->GetCoverPoint()->fDirY * pEntity->GetLeapRange();

						float destX = pEntity->GetX() + dirX;
						float destZ = pEntity->GetZ() + dirZ;

						//pEntity->SetDestination( destX, pEntity->GetY(), destZ, -1 );
						pEntity->ForceMove( destX, destZ );

						pEntity->ClearCoverPoint();
						pEntity->Stand();
						pEntity->ChangeState ( pStateSet->pStateLeaping );
					}
					else
					{	
						// no
						pEntity->PredictTargetPosition ( 5.0f );
						pEntity->MoveToGuessPos ( );
						pEntity->LookAtDest ( );
						pEntity->MarkOldPosition ( );

						pEntity->Stand();
						pEntity->ClearCoverPoint();
						pEntity->ChangeState ( pStateSet->pStateGoToDest );
					}
					return;
				}
			}
		}
	}
}
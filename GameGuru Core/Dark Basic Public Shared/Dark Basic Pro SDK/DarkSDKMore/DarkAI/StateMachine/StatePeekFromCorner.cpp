
#include "StateSet.h"

int StateSet::StatePeekFromCorner::iNumUsers = 0;

StateSet::StatePeekFromCorner::StatePeekFromCorner ( ) { iNumUsers = 0; }
StateSet::StatePeekFromCorner::~StatePeekFromCorner ( ) { }

void StateSet::StatePeekFromCorner::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 

	if ( !pEntity->GetCanHideBehindCorner() ) 
	{
		pEntity->ReturnToPreviousState();
		return;
	}

	//find somewhere to fall back to

	Path cPath;
	Path cDirections;
	Path cPeekPoints;
	pEntity->GetContainer( )->pPathFinder->SearchPeekingPoints ( pEntity->GetX(), pEntity->GetZ(), pEntity->GetTargetX(), pEntity->GetTargetZ(), &cPath, &cDirections, &cPeekPoints );
	
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
		pEntity->SetDestination ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y, -1 );
		pEntity->SetHideDirection( cDirections.GetPoint( iIndex ).x, cDirections.GetPoint( iIndex ).y );
		pEntity->SetPeekingPos( cPeekPoints.GetPoint( iIndex ).x, cPeekPoints.GetPoint( iIndex ).y );
	}
	else pEntity->ReturnToPreviousState();
	
	float x = pEntity->GetFinalDest( ENT_X );
	float y = pEntity->GetFinalDest( ENT_Y );
	float z = pEntity->GetFinalDest( ENT_Z );
	pEntity->SetHidePos( x,y,z, ENT_X|ENT_Y|ENT_Z );
}

void StateSet::StatePeekFromCorner::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
	pEntity->SetIsBehindCorner( false );
	//pEntity->Stand ( );
}

void StateSet::StatePeekFromCorner::Execute ( Entity *pEntity )
{	
	if ( pEntity->CountTargets( ) == 0 ) 
	{
		pEntity->ChangeState( pStateSet->pStateIdle );
		return;
	}

	// addition for Lee to force an entity out of the corner
	if ( pEntity->IsForcedOutOfCover() )
	{
		pEntity->ChangeState ( pStateSet->pStateAttack );
		return;
	}

	if ( pEntity->ValidTarget( ) )
	{
		if ( pEntity->GetIsBehindCorner() )
		{
			float dirX = pEntity->GetTargetX() - pEntity->GetX();
			float dirZ = pEntity->GetTargetZ() - pEntity->GetZ();

			float length = sqrt(dirX*dirX + dirZ*dirZ);
			if ( length > 0 ) 
			{
				dirX /= length;
				dirZ /= length;
			}

			// is this position still tenable
			if ( pEntity->GetHideX()*dirX + pEntity->GetHideY()*dirZ > -0.6f )
			{
				// no
				pEntity->ChangeState ( pStateSet->pStateAttack );
				return;
			}
		}

		pEntity->SetStateTimer( 3 );
	}
	else
	{
		if ( pEntity->GetStateTimer ( ) <= 0 )
		{
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
		}
	}

	float fSqrDist = pEntity->GetSqrDistToDest ( );

	if ( fSqrDist <= 0.001f && !pEntity->GetIsBehindCorner() )
	{
		//pEntity->Stop ( );
		
		pEntity->SetIsBehindCorner( true );
		pEntity->LookAtHideDir( );
	}

}
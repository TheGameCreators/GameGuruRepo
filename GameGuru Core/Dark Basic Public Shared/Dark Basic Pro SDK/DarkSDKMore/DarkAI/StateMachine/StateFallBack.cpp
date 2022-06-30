#include "StateSet.h"

int StateSet::StateFallBack::iNumUsers = 0;

StateSet::StateFallBack::StateFallBack ( ) { iNumUsers = 0; }
StateSet::StateFallBack::~StateFallBack ( ) {  }

void StateSet::StateFallBack::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	pEntity->Duck ( );

	//find somewhere to fall back to

	if ( pEntity->CountTargets( ) > 0 )
	{
		//bool bCover = pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX( ), pEntity->GetTargetZ( ), 0 );
		bool bCover = pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetY( ) + pEntity->GetFullHeight( )/2, pEntity->GetZ( ), pEntity->GetTargetX ( ), pEntity->GetTargetY ( ), pEntity->GetTargetZ ( ), 0 );

		if ( bCover )
		{
			float fResult = pEntity->GetContainer( )->pPathFinder->FindClosestPolygon ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX( ), pEntity->GetTargetZ( ) );
			
			if ( fResult > 0.01f ) fResult -= 0.01f;

			float fDirX = pEntity->GetTargetX( ) - pEntity->GetX ( );
			float fDirY = pEntity->GetTargetY( ) - pEntity->GetY ( );
			float fDirZ = pEntity->GetTargetZ( ) - pEntity->GetZ ( );
			
			if ( pEntity->InDefendArea ( fDirX * fResult, fDirY * fResult, fDirZ * fResult ) )
			{
				pEntity->SetDestination ( fDirX * fResult, fDirY * fResult, fDirZ * fResult, -1 );
			}

			float x = pEntity->GetFinalDest( ENT_X );
			float y = pEntity->GetFinalDest( ENT_Y );
			float z = pEntity->GetFinalDest( ENT_Z );
			pEntity->SetHidePos( x,y,z, ENT_X|ENT_Y|ENT_Z );
			pEntity->SetInDuckingCover( true );
			pEntity->SetInVerticalCover( false );

			//found some nice cover right in front of the entity, move towards it
			return;
		}
	}
	
	//area is open between here and last target position
	//more complicated to find cover

	Path cPath;
	pEntity->GetContainer( )->pPathFinder->SearchPoints ( pEntity->GetX ( ), pEntity->GetZ ( ), &cPath, 2 );
	
	int iNumPoints = cPath.CountPoints ( );
	if ( iNumPoints == 0 )
	{
		//no where to go
		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}
	
	//get entity->player vector, and rotate it 90 degs
	float fDirX = pEntity->GetTargetX( ) - pEntity->GetX ( );
	float fDirZ = pEntity->GetTargetZ( ) - pEntity->GetZ ( );

	float fDir2X =  fDirZ;
	float fDir2Z = -fDirX;

	float fRange = ( fDirX*fDirX + fDirZ*fDirZ );
	
	bool bFound = false;
	int iCount = 0;

	int iIndex = -1;
	int iIndex2 = -1;
	float fStraightest = 10000.0f;
	float fClosest = 1000000.0f;

	//favour points that are in front of the entity
	for ( int i = 0; i < iNumPoints; i++ )
	{
		float fX = cPath.GetPoint ( i ).x - pEntity->GetX ( );
		float fZ = cPath.GetPoint ( i ).y - pEntity->GetZ ( );

		float fDist = fX*fX + fZ*fZ;

		float fDotP = fX * fDirX + fZ * fDirZ;
		float fDotP2 = fX * fDir2X + fZ * fDir2Z;
		
		//hiding place is between here and the enemy, not too close to the last known position
		//find the point matching these restrictions that is most inline with the entity->target vector
		if ( ( fabs( fDotP2 ) < fStraightest ) && ( fDotP > 0.0f ) && ( fDist < fRange / 4.0f ) ) 
		{
			fStraightest = fabs( fDotP2 );
			iIndex = i;
		}
		
		//backup if no other points are found
		if ( fDist < fClosest && fDotP <= 0.0f )
		{
			fClosest = fDist;
			iIndex2 = i;
		}
	}
	
	if ( iIndex < 0 )
	{
		if ( iIndex2 < 0 )
		{
			//found no points at all
			pEntity->ChangeState ( pStateSet->pStateIdle );
			return;
		}
		else
		{
			iIndex = iIndex2;
		}
	}

	if ( !pEntity->GetIsDefending( ) || pEntity->InDefendArea ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y ) )
	{
		pEntity->SetDestination ( cPath.GetPoint ( iIndex ).x, pEntity->GetY( ), cPath.GetPoint ( iIndex ).y, -1 );
	}
	
	float x = pEntity->GetFinalDest( ENT_X );
	float y = pEntity->GetFinalDest( ENT_Y );
	float z = pEntity->GetFinalDest( ENT_Z );
	pEntity->SetHidePos( x,y,z, ENT_X|ENT_Y|ENT_Z );

	pEntity->SetInDuckingCover( false );
	pEntity->SetInVerticalCover( true );
}

void StateSet::StateFallBack::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
	pEntity->Stand ( );
}

void StateSet::StateFallBack::Execute ( Entity *pEntity )
{
	if ( pEntity->ValidTarget( ) )
	{
		pEntity->LookAtTarget ( );
		pEntity->SetCoverTimer ( 8.0f );

		if ( pEntity->IsHit ( ) )
		{
			//bool bCover = pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX ( ), pEntity->GetTargetZ ( ), 0 );
			bool bCover = pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetY( ) + pEntity->GetFullHeight( )/2, pEntity->GetZ( ), pEntity->GetTargetX ( ), pEntity->GetTargetY ( ), pEntity->GetTargetZ ( ), 0 );

			if ( !bCover )
			{
				pEntity->ChangeState ( pStateSet->pStateStrafeAttack );
			}
			else
			{
				if ( pEntity->GetCanDuck() )
				{
					pEntity->Duck ( );
					pEntity->ChangeState ( pStateSet->pStateWaitInCover );
				}
			}
			
			//getting hit, have chosen to move or duck
			return;
		}
		
		if ( rand( ) % 10 && pEntity->GetCanDuck() ) 
		{
			pEntity->Duck ( );
			pEntity->ChangeState ( pStateSet->pStateWaitInCover );
			
			//not getting hit, have randomly decided to duck
			return;
		}

		//otherwise move and shoot
		//if ( pEntity->CanFire ( ) ) pEntity->FireWeapon ( );
	}
	
	float fSqrDist = pEntity->GetSqrDistToDest ( );

	if ( fSqrDist <= 1.0f )
	{
		pEntity->Stop ( );
		
		if ( pEntity->GetInDuckingCover() && pEntity->GetCanDuck() ) pEntity->Duck ( );
	
		pEntity->SetCoverTimer ( 4.0f );
		pEntity->ChangeState ( pStateSet->pStateWaitInCover );
		
		//have reached cover position, if cover is low duck behind it
		return;
	}
}
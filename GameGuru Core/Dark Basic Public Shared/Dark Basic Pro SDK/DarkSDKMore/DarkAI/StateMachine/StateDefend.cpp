#include "StateSet.h"

int StateSet::StateDefend::iNumUsers = 0;

StateSet::StateDefend::StateDefend ( ) { iNumUsers = 0; }
StateSet::StateDefend::~StateDefend ( ) { }

void StateSet::StateDefend::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 

	pEntity->SetNoLookPoint( );
	
	bool bDestOK = pEntity->InDefendArea ( pEntity->GetFinalDest( ENT_X ), pEntity->GetFinalDest( ENT_Y ), pEntity->GetFinalDest( ENT_Z ) );
	if ( !bDestOK ) pEntity->MoveClose ( pEntity->GetDefendPos( ENT_X ), pEntity->GetDefendPos( ENT_Y ), pEntity->GetDefendPos( ENT_Z ), pEntity->GetDefendDist( ), pEntity->GetDefendContainer() );
}
void StateSet::StateDefend::Exit    ( Entity *pEntity ) { iNumUsers--; }

void StateSet::StateDefend::Execute ( Entity *pEntity )
{
	if ( pEntity->ValidTarget( ) )
	{
		float fDist = pEntity->GetSqrDistToTarget ( );

		//float fMinimumDist = 16.0f;

		if ( pEntity->IsHit ( ) )
		{
			//bool bCover = pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pEntity->GetX ( ), pEntity->GetZ ( ), pEntity->GetTargetX ( ), pEntity->GetTargetZ ( ), 0 );
			bool bCover = pWorld->GlobalVisibilityCheck( pEntity->GetX( ), pEntity->GetY( ) + pEntity->GetFullHeight( )/2, pEntity->GetZ( ), pEntity->GetTargetX ( ), pEntity->GetTargetY ( ), pEntity->GetTargetZ ( ), 0 );

			if ( bCover )
			{
				pEntity->Duck ( );
				pEntity->SetInDuckingCover( true );
				pEntity->SetInVerticalCover( false );
				pEntity->SetCoverTimer ( 8.0f );
			}
		}
		
		pEntity->LookAtTarget ( );

		//if ( pEntity->CanFire ( ) ) pEntity->FireWeapon ( );
	}
	else
	{
		if ( pEntity->IsHit ( ) ) 
		{
			if ( pEntity->CountHitPoints ( ) <= 0 )
			{
				pEntity->LookAround ( 90.0f, 180.0f );
			}
			else
			{
				GGVECTOR3 vecDir = pEntity->GetHitDir ( 0 ) * 20.0f;
				
				pEntity->LookAt ( pEntity->GetX ( ) + vecDir.x, pEntity->GetZ ( ) + vecDir.z );
			}
		}
		else
		{
			if ( pEntity->GetFollowing( ) ) pEntity->LookAt( pWorld->pTeamController->GetPlayer( )->GetX( ), pWorld->pTeamController->GetPlayer( )->GetZ( ) );
		}
	}

	if ( !pEntity->GetIsDefending( ) ) pEntity->ChangeState ( pStateSet->pStateIdle );

	bool bDestOK = pEntity->InDefendArea ( pEntity->GetFinalDest( ENT_X ), pEntity->GetFinalDest( ENT_Y ), pEntity->GetFinalDest( ENT_Z ) );
	//bool bPlayerInContainer = pWorld->pTeamController->GetPlayer( )->GetContainer( ) == pEntity->GetContainer( );
	if ( !bDestOK /*&& bPlayerInContainer*/ ) pEntity->MoveClose ( pEntity->GetDefendPos( ENT_X ), pEntity->GetDefendPos( ENT_Y ), pEntity->GetDefendPos( ENT_Z ), pEntity->GetDefendDist( ), pEntity->GetDefendContainer() );

	float fDist = pEntity->GetSqrDistToDest ( );

	if ( fDist < 0.1f )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
	}

	if ( !pEntity->IsMakingProgress ( ) && pEntity->InDefendArea ( pEntity->GetX ( ), pEntity->GetY ( ), pEntity->GetZ ( ) ) )
	{
		pEntity->ChangeState ( pStateSet->pStateIdle );
	}
}
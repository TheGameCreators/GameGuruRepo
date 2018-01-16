#include "StateSet.h"

int StateSet::StateSearchArea::iNumUsers = 0;

StateSet::StateSearchArea::StateSearchArea ( ) { iNumUsers = 0; }
StateSet::StateSearchArea::~StateSearchArea ( ) { }

void StateSet::StateSearchArea::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 
	if ( !pEntity->GetCanSearch() ) 
	{
		pEntity->ChangeState( pStateSet->pStateIdle );
		return;
	}

	pEntity->IncSearchPoints( 1 );

	//pEntity->cSearchPath.Clear ( );
	//pEntity->cSearchPath.CalculateSqrLength ( );

	if ( pEntity->GetSearchPoints() > 3 )
	{
		//searched enough
		pEntity->SetSearchPoints( 0 );
		pEntity->ClearSearchPath( );

		//MessageBox ( NULL, "Searched Enough", "Info", 0 );

		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	//char str [ 256 ];
	//sprintf_s ( str, 255, "Searching: %d", pEntity->iSearchPoints );
	//MessageBox ( NULL, str, "Info", 0 );

	//find a place to search
	
	Path cPath, cCurrPath;
	Path *pPath = &cPath;						//new potential destinations
	Path *pSearchPath = pEntity->GetSearchPath();	//current points to search

	if ( pEntity->CountTargets( ) > 0 )
	{
		pEntity->GetContainer( )->pPathFinder->SearchPoints ( pEntity->GetTargetGuessX( ), pEntity->GetTargetGuessZ( ), pPath, 1 );
	}
	else
	{
		pEntity->GetContainer( )->pPathFinder->SearchPoints ( pEntity->GetX( ), pEntity->GetZ( ), pPath, 1 );
	}

	int iNumPoints = pPath->CountPoints ( );
	if ( iNumPoints == 0 )
	{
		//no where to go
		pEntity->SetSearchPoints( 0 );
		pEntity->ClearSearchPath( );

		//MessageBox ( NULL, "No where to go", "Info", 0 );

		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}

	float fDirX = pEntity->GetTargetDirX( );
	float fDirZ = pEntity->GetTargetDirZ( );

	float length = fDirX*fDirX + fDirZ*fDirZ;
	if ( length > 0 )
	{
		fDirX /= length;
		fDirZ /= length;
	}

	float fDir2X =  fDirZ;
	float fDir2Z = -fDirX;

	int iIndex = -1;
	int iIndex2 = -1;

	float fStraightest = 0;
	//float fClosest = 1000000.0f;

	int iIterations = 0;
	//int iLastPoint = pSearchPath->CountPoints ( ) - 2;
	bool bEnd = false;
	
	while ( !bEnd && ( cCurrPath.GetLength( ) < pEntity->GetRadius( )*pEntity->GetRadius( ) * 100*100 ) )
	{
		//int iIterations2 = 0;
		//bool bFound = false;
		iIndex = -1;
		iIndex2 = -1;
		fStraightest = 0;

		//for ( int i = 0; i < iNumPoints; i++ )
		//while ( !bFound && iIterations2 < iNumPoints*2 )
		for ( int index = 0; index < iNumPoints; index++ )
		{
			int i = index;
			if ( iIterations > 0 ) i = rand ( ) % iNumPoints;
			
			float fX = pPath->GetPoint ( i ).x - pEntity->GetTargetGuessX( );
			float fZ = pPath->GetPoint ( i ).y - pEntity->GetTargetGuessZ( );
			if ( iIterations > 0 )
			{
				fX = pPath->GetPoint ( i ).x - cCurrPath.GetLast().x;
				fZ = pPath->GetPoint ( i ).y - cCurrPath.GetLast().y;
			}

			float fSqrDist = fX*fX + fZ*fZ;
			float fDist = sqrt(fSqrDist);

			float fDotP = (fX * fDirX + fZ * fDirZ)/fDist;
			//float fDotP2 = fX * fDir2X + fZ * fDir2Z;

			float fError = ( 10.0f * rand( ) ) / RAND_MAX - 5.0f;
			fError = 0.0f;	//testing

			bool bVisited = pSearchPath->Contains ( pPath->GetPoint ( i ).x, pPath->GetPoint ( i ).y ) || cCurrPath.Contains ( pPath->GetPoint ( i ).x, pPath->GetPoint ( i ).y );
			
			//pick a search point that is most in the direction of the player's
			//last known direction
			if ( iIterations == 0 )
			{
				//if ( !bVisited && ( fabs( fDotP2 ) + fError < fStraightest ) && ( fDotP > 0.0f ) ) 
				if ( !bVisited && fDotP >= fStraightest ) 
				{
					fStraightest = fDotP;
					iIndex = i;
					//bFound = true;
				}
			}
			else
			{
				//subsequent points should not be visible from original position
				bool bVisible = !pEntity->GetContainer( )->pPathFinder->QuickPolygonsCheckVisible ( pPath->GetPoint ( i ).x, pPath->GetPoint ( i ).y, pEntity->GetTargetX( ), pEntity->GetTargetZ( ), 1 );

				if ( !bVisited && !bVisible && fDotP > -0.2f )
				{
					iIndex = i;
					//bFound = true;
					break;
				}

				//if ( !bVisited && !bVisible )
				//{
				//	iIndex2 = i;
				//}
			}

			//iIterations2++;
		} 

		//float fAngY = pEntity->GetAngleY ( ) * DEGTORAD;
		//fDirX = sin ( fAngY );
		//fDirZ = cos ( fAngY );

		//pEntity->vecLastKnownDir.x = fDirX;
		//pEntity->vecLastKnownDir.z = fDirZ;

		//char str [ 256 ];
		//sprintf_s ( str, 255, "Point: %d", iIndex );
		//MessageBox ( NULL, str, "Info", 0 );
		
		if ( iIndex < 0 )
		{
			if ( iIndex2 >= 0 ) iIndex = iIndex2;
			//else iIndex = rand( ) % iNumPoints;
			else break;
		}
		//else
		{
			//pEntity->vecGuessNewPos.x = pPath->GetPoint ( iIndex ).x;
			//pEntity->vecGuessNewPos.z = pPath->GetPoint ( iIndex ).y;

			cCurrPath.AddPoint ( pPath->GetPoint ( iIndex ).x, 0, pPath->GetPoint ( iIndex ).y, pPath->GetPoint ( iIndex ).container );
			cCurrPath.CalculateSqrLength ( );

			float fPX = pPath->GetPoint ( iIndex ).x;
			float fPY = pPath->GetPoint ( iIndex ).y;
			int container = pPath->GetPoint ( iIndex ).container;
			Container *pContainer = Entity::pWorld->GetContainer( container );
			
			pPath->Clear ( );
			if ( pContainer ) pContainer->pPathFinder->GetConnectedPoints ( fPX, fPY, pPath );
			else pEntity->GetContainer()->pPathFinder->GetConnectedPoints ( fPX, fPY, pPath );
			iNumPoints = pPath->CountPoints ( );
		}

		//char str [ 256 ];
		//sprintf_s ( str, 255, "Found Point: %d %f %f", iIndex, pPath->GetPoint ( iIndex ).x, pPath->GetPoint ( iIndex ).y );
		//MessageBox ( NULL, str, "Info", 0 );

		iIterations++;

		if ( iIterations > 5 || iNumPoints == 0 ) bEnd = true;
	}

	//if ( pEntity->iSearchPoints > 1 )
	//{
		//pEntity->cSearchPath.RemoveFirst ( );
	//}

	if ( cCurrPath.CountPoints ( ) == 0 )
	{
		pEntity->ClearSearchPath( );
		pEntity->SetSearchPoints( 0 );

		//MessageBox ( NULL, "No points suitable", "Info", 0 );

		pEntity->ChangeState ( pStateSet->pStateIdle );
		return;
	}
	
	//pSearchPath->AddPoint ( cCurrPath.GetPoint ( 0 ).x, 0, cCurrPath.GetPoint ( 0 ).y, cCurrPath.GetPoint ( 0 ).container );

	//for ( int i = 0; i < cCurrPath.CountPoints ( ) - 1; i++ )
	//{
		//pEntity->cSearchPath.AddPoint ( cCurrPath.GetPoint ( i ).x, cCurrPath.GetPoint ( i ).y );
	//}
	
	float fX = cCurrPath.GetLast ( ).x;
	float fY = cCurrPath.GetLast ( ).y;
	pSearchPath->InsertPoint ( 0, fX, fY, cCurrPath.GetLast ( ).container );

	//pEntity->vecFinalDest.x = pEntity->cSearchPath.GetPoint ( 0 ).x;
	//pEntity->vecFinalDest.z = pEntity->cSearchPath.GetPoint ( 0 ).y;
	//pEntity->vecLookAt = pEntity->vecFinalDest;

	pEntity->LookAround ( 60.0f, 180.0f );

	pEntity->SetStateTimer ( ( 2.0f * rand( ) ) / RAND_MAX + 1.0f );
	pEntity->SetSearchTimer( 15.0f );
}

void StateSet::StateSearchArea::Exit    ( Entity *pEntity )
{ 
	iNumUsers--; 
}

void StateSet::StateSearchArea::Execute ( Entity *pEntity )
{
	if ( pEntity->ValidTarget( ) )
	{
		switch ( pEntity->GetAggressiveness() )
		{
			case 0:		pEntity->ChangeState ( pStateSet->pStateAttack );		break;
			case 1:		pEntity->ChangeState ( pStateSet->pStateChaseAttack );	break;

			default:	pEntity->ChangeState ( pStateSet->pStateAttack );
		}

		pEntity->SetSearchPoints( 0 );
		pEntity->ClearSearchPath( );
		//pEntity->cSearchPath.CalculateSqrLength ( );
		return;
	}
	
	if ( pEntity->GetStateTimer ( ) <= 0.0f )
	{
		if ( pEntity->GetSearchTimer( ) <= 0 )
		{
			pEntity->ChangeState( pStateSet->pStateIdle );
		}

		pEntity->SetDestination ( pEntity->GetSearchPointX( 0 ), pEntity->GetY( ), pEntity->GetSearchPointY( 0 ), pEntity->GetSearchPointContainer( 0 ) );
		
		//pEntity->vecLookAt = pEntity->vecFinalDest;
		if ( pEntity->GetLookTimer ( ) <= 0.0f ) pEntity->SweepViewForward ( 45.0f );
		
		float fSqrDist = pEntity->GetSqrDistToDest ( );

		if ( fSqrDist <= 9.0f )
		{
			pEntity->GetSearchPath()->RemoveFirst ( );
			if ( pEntity->GetSearchPath( )->CountPoints ( ) == 0 )
			{
//				pEntity->vecGuessNewPos = pEntity->vecLastKnownPos;
				//pEntity->iSearchPoints = 0;
				
				//switch ( pEntity->iAggressiveness )
				//{
					//case 0:		pEntity->ChangeState( pStateSet->pStateIdle );			break;
					//case 1:		pEntity->ChangeState( pStateSet->pStateSearchArea );	break;

					//default:	pEntity->ChangeState( pStateSet->pStateSearchArea );
				//}

				//pEntity->ChangeState( pStateSet->pStateSearchArea );

				pEntity->ChangeState( pStateSet->pStateIdle );

				return;
			}
		}
	}

	if ( pEntity->HeardSound ( ) )
	{
		pEntity->ChangeState ( pStateSet->pStateInvestigate );
	}
}

#include "StateSet.h"

int StateSet::StateLeaping::iNumUsers = 0;

StateSet::StateLeaping::StateLeaping ( ) { iNumUsers = 0; }
StateSet::StateLeaping::~StateLeaping ( ) { }

void StateSet::StateLeaping::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 

	if ( !pEntity->GetCanLeap() ) 
	{
		pEntity->ReturnToPreviousState();
		return;
	}

	pEntity->SetLeaping( true );
}

void StateSet::StateLeaping::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
	pEntity->SetLeaping( false );
	pEntity->Stand ( );
}

void StateSet::StateLeaping::Execute ( Entity *pEntity )
{	
	float fSqrDist = pEntity->GetSqrDistToDest ( );

	if ( fSqrDist <= 0.001f )
	{
		pEntity->ChangeState( pStateSet->pStateAttack );
	}

}
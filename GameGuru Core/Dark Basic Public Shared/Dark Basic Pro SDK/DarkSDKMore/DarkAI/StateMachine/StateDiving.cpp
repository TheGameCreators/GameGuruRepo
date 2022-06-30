
#include "StateSet.h"

int StateSet::StateDiving::iNumUsers = 0;

StateSet::StateDiving::StateDiving ( ) { iNumUsers = 0; }
StateSet::StateDiving::~StateDiving ( ) { }

void StateSet::StateDiving::Enter   ( Entity *pEntity ) 
{ 
	iNumUsers++; 

	if ( !pEntity->GetCanDive() ) 
	{
		pEntity->ReturnToPreviousState();
		return;
	}

	pEntity->SetDiving( true );
}

void StateSet::StateDiving::Exit    ( Entity *pEntity ) 
{ 
	iNumUsers--; 
	pEntity->SetDiving( false );
}

void StateSet::StateDiving::Execute ( Entity *pEntity )
{	
	float fSqrDist = pEntity->GetSqrDistToDest ( );

	if ( fSqrDist <= 0.001f )
	{
		pEntity->ReturnToPreviousState();
	}
}
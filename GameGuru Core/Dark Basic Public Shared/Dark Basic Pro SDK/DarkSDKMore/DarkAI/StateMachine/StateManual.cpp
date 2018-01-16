#include "StateSet.h"

int StateSet::StateManual::iNumUsers = 0;

StateSet::StateManual::StateManual ( ) { iNumUsers = 0; }
StateSet::StateManual::~StateManual ( ) { }

void StateSet::StateManual::Enter   ( Entity *pEntity ) { iNumUsers++; pEntity->SetManualControl( true ); }
void StateSet::StateManual::Exit    ( Entity *pEntity ) { iNumUsers--; pEntity->SetManualControl( false ); }

void StateSet::StateManual::Execute ( Entity *pEntity ) { }
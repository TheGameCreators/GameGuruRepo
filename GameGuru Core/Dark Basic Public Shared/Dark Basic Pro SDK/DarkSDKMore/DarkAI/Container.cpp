#include "Container.h"
#include "PathFinderAdvanced.h"

Container::Container ( int id ) 
{ 
	pPathFinder = new PathFinderAdvanced ( this );
	pPathFinder->SetRadius ( 2.5f );
	iID = id;
	bActive = true;

	pNextContainer = 0;
}

Container::~Container ( )
{
	if ( pPathFinder ) delete pPathFinder;
}

int Container::GetID ( )
{
	return iID;
}

void Container::Activate ( )
{
	bActive = true;
}

void Container::DeActivate ( )
{
	bActive = false;
}

bool Container::IsActive ( )
{
	return bActive;
}
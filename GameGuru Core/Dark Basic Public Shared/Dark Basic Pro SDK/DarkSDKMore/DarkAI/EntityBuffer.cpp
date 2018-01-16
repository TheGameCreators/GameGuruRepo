#include "EntityBuffer.h"
#include "Entity.h"

EntityBuffer::EntityBuffer( ) 
{ 
	pEntityList = NULL; 
	hMutex = CreateMutex( NULL, FALSE, NULL );
}

EntityBuffer::~EntityBuffer( )
{
	while( pEntityList )
	{
		EntityEntry *pEntry = pEntityList;
		pEntityList = pEntityList->pNextEntry;
		delete pEntry;
	}

	if ( hMutex ) CloseHandle( hMutex );
}

Entity* EntityBuffer::GetNextEntity( )
{
	if ( !pEntityList ) return NULL;	//try a quick reject, value could be modified by another thread before obtaining lock

	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result == WAIT_FAILED ) 
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock 'get' mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	if ( !pEntityList )	//double check now that thread has the lock
	{
		ReleaseMutex( hMutex );
		return NULL;
	}
	
	EntityEntry *pOldEntry = pEntityList;
	Entity *pEntity = pEntityList->pEntity;
	pEntityList = pEntityList->pNextEntry;
	
	ReleaseMutex( hMutex );

	delete pOldEntry;

	return pEntity;
}

void EntityBuffer::AddEntity( Entity *pEntity )
{
	int result = WaitForSingleObject( hMutex, INFINITE );
	if ( result == WAIT_FAILED ) 
	{
		char str[64];
		sprintf_s( str, 64, "Failed to lock 'add' mutex, error: %d", GetLastError( ) );
		MessageBox( NULL, str, "Error", 0 );
		exit(-1);
	}

	EntityEntry *pNewEntry = new EntityEntry();
	pNewEntry->pEntity = pEntity;

	pNewEntry->pNextEntry = pEntityList;
	pEntityList = pNewEntry;

	ReleaseMutex( hMutex );
}
#ifndef ENTITY_BUFFER_H
#define ENTITY_BUFFER_H

#include "Thread.h"

class TeamController;
class Beacon;
class Zone;
class Entity;

struct EntityEntry
{
public:
	Entity *pEntity;
	EntityEntry *pNextEntry;

	EntityEntry( ) { pEntity = NULL; pNextEntry = NULL; }
};

class EntityBuffer
{

public:

	EntityBuffer( );
	~EntityBuffer( );

	Entity* GetNextEntity( );
	void AddEntity( Entity *pEntity );

private:

	HANDLE hMutex;
	EntityEntry *pEntityList;

};

#endif
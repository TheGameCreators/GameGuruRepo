#ifndef ENTITY_THREAD_H
#define ENTITY_THREAD_H

#include "Thread.h"

class TeamController;
class Beacon;
class Zone;
class Entity;
class EntityBuffer;

class EntityThread : public Thread
{

public:

	EntityThread( );
	~EntityThread( );

	void SetupData( EntityBuffer *pBuffer, float time );
	void WaitOnThread( );
	void ResumeThread( );

protected:

	unsigned int Run( );

private:

	float fTimeDelta;
	EntityBuffer *pEntityBuffer;
	HANDLE hStartEvent, hDoneEvent;
	bool bTerminate;
	bool bIsRunning;

};

#endif
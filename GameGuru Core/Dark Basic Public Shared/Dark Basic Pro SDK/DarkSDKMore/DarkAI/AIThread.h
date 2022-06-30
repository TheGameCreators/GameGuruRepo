#ifndef AITHREAD_H
#define AITHREAD_H

#include "Thread.h"

class TeamController;
class Beacon;
class Zone;

class AIThread : public Thread
{

public:

	AIThread( );
	~AIThread( );

	void SetupData( TeamController *teamController, float timeDelta, Beacon *beaconList, Zone *zoneList );

protected:

	unsigned int Run( );

private:

	TeamController *pTeamController;
	Beacon *pBeaconList;
	Zone *pZoneList;
	float fTimeDelta;

};

#endif
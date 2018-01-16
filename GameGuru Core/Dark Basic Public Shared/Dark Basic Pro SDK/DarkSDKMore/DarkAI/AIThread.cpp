#include "AIThread.h"
#include "TeamController.h"

AIThread::AIThread( )
{
	
}

AIThread::~AIThread( )
{
	
}

void AIThread::SetupData( TeamController *teamController, float timeDelta, Beacon *beaconList, Zone *zoneList )
{
	pTeamController = teamController;
	pBeaconList = beaconList;
	pZoneList = zoneList;
}

unsigned int AIThread::Run( )
{
	pTeamController->Update( fTimeDelta, pBeaconList, pZoneList );

	return 0;
}
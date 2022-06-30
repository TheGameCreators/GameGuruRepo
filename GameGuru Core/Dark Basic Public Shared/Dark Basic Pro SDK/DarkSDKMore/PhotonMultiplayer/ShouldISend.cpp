
#include "stdafx.h"
#include "CServer.h"
#include "CClient.h"
#include "PhotonMultiplayer.h"

/*
bool CServer::ShouldISend ( int p , int id, float thisx, float thisy, float thisz, int playerID )
{
	//return true;
	float dx = Client()->m_rgpPlayer[playerID]->newx - thisx;
	float dy = Client()->m_rgpPlayer[playerID]->newy - thisy;
	float dz = Client()->m_rgpPlayer[playerID]->newz - thisz;
	float dist = sqrt(dx*dx + dy*dy* + dz*dz);

	switch ( p )
	{
	case k_EMsgClientPlayerBullet:
		if ( bulletSeen[id][playerID] == false )
			if ( dist > 220000 ) return false;

		bulletSeen[id][playerID] = true;
	break;

	case k_EMsgClientPlayerAppearance:
		if ( dist > 250000 ) return false;
	break;

	case k_EMsgClientPlayAnimation:
		if ( dist > 250000 ) return false;
	break;
	}

	return true;

}
*/

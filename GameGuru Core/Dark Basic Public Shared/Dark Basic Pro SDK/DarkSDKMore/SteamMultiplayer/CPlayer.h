
#include "SteamMultiplayer.h"
#include "CClient.h"
#include "StatsAndAchievements.h"

#ifndef __CPLAYER__
#define __CPLAYER__

class CPlayer
{
public:
	int			id;
	char		name[128];

	float		newx;
	float		newy;
	float		newz;

	float		x;
	float		y;
	float		z;

	float		newangle;
	float		angle;

	int			mode;
	int			isLocalPlayer;
	int			isDisabled;
	uint64		ClientTime;
	uint64		m_ulLastClientUpdateTick; // Last time we sent an update on our local data to the server
	uint64		m_ulLastNetworkDataReceivedTime;

	ClientSteamUpdateData_t m_ClientUpdateData;

	CPlayer()
	{
		x = 0;
		y = 0;
		z = 0;
		newx = 0;
		newy = 0;
		newz = 0;
		newangle = 0;
		angle = 0;
		isDisabled = false;
		ClientTime = 0;
	}

	void AccumulateStats( CStatsAndAchievements *pStats )
	{
		if ( isLocalPlayer )
		{
			//pStats->AddDistanceTraveled( GetDistanceTraveledLastFrame() );
		}
	}

	void SetIsLocalPlayer(int v)
	{
		isLocalPlayer = v;
	}

	bool BIsLocalPlayer()
	{
		if ( isLocalPlayer )
			return true;
		else
			return false;
	}

	// received update from server on this player
	void OnReceiveServerUpdate( ServerPlayerUpdateData_t *pUpdateData )
	{
		newx = (float)pUpdateData->x;
		newy = (float)pUpdateData->y;
		newz = (float)pUpdateData->z;
		newangle = (float)pUpdateData->angle;
	}

	void OnReceiveClientUpdate( ClientSteamUpdateData_t *pUpdateData )
	{
		/*if ( !m_bIsServerInstance )
		{
			OutputDebugString( "Should not be receiving client updates on non-server instances\n" );
			return;
		}*/
		memcpy( &m_ClientUpdateData, pUpdateData, sizeof( ClientSteamUpdateData_t ) );

		x = (float)pUpdateData->x;
		y = (float)pUpdateData->y;
		z = (float)pUpdateData->z;
		angle = (float)pUpdateData->angle;

		/*newx = (float)pUpdateData->x;
		newy = (float)pUpdateData->y;
		newz = (float)pUpdateData->z;
		newangle = (float)pUpdateData->angle;*/

	}

	void SetDisabled ( int v )
	{
		isDisabled = v;
	}

	// Purpose: Build the update data to send from server to clients
	void BuildServerUpdate( ServerPlayerUpdateData_t *pUpdateData )
	{
		pUpdateData->x = (unsigned short)x;
		pUpdateData->y = (unsigned short)y;
		pUpdateData->z = (unsigned short)z;
		pUpdateData->angle = (unsigned short)angle;
	}

	const char* GetPlayerName()
	{
		return name;
	}

	bool BGetClientUpdateData( ClientSteamUpdateData_t *pUpdateData  )
	{
		// Limit the rate at which we send updates, even if our internal frame rate is higher
		if ( GetCounterPassedTotal() - ClientTime < 1000.0f/CLIENT_UPDATE_SEND_RATE ) // dave
			return false;

		ClientTime = (uint64)GetCounterPassedTotal();

		m_ClientUpdateData.x = (unsigned short)x;
		m_ClientUpdateData.y = (unsigned short)y;
		m_ClientUpdateData.z = (unsigned short)z;
		m_ClientUpdateData.angle = (unsigned short)angle;

		// Update playername before sending
		/*if ( isLocalPlayer )
		{
			strcpy ( m_ClientUpdateData.name , SteamFriends()->GetFriendPersonaName( SteamUser()->GetSteamID() ));
		}*/

		memcpy( pUpdateData, &m_ClientUpdateData, sizeof( ClientSteamUpdateData_t ) );
		memset( &m_ClientUpdateData, 0, sizeof( m_ClientUpdateData ) );

		return true;
	}

	
};

#endif
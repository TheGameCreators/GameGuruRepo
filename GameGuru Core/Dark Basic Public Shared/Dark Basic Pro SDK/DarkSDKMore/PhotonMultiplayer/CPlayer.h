// Includes
#include "PhotonMultiplayer.h"
//#include "CClient.h"

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
	int			newplayer;

	ClientUpdateData_t m_ClientUpdateData;

	CPlayer()
	{
		id = 0;
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

	void OnReceiveClientUpdate( ClientUpdateData_t *pUpdateData )
	{
		memcpy( &m_ClientUpdateData, pUpdateData, sizeof( ClientUpdateData_t ) );
		x = (float)pUpdateData->x;
		y = (float)pUpdateData->y;
		z = (float)pUpdateData->z;
		angle = (float)pUpdateData->angle;
	}

	void SetDisabled ( int v )
	{
		isDisabled = v;
	}

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

	bool BGetClientUpdateData( ClientUpdateData_t *pUpdateData  )
	{
		// Limit the rate at which we send updates, even if our internal frame rate is higher
		if ( GetCounterPassedTotal() - ClientTime < 1000.0f/CLIENT_UPDATE_SEND_RATE )
			return false;

		ClientTime = (uint64)GetCounterPassedTotal();

		m_ClientUpdateData.x = (unsigned short)x;
		m_ClientUpdateData.y = (unsigned short)y;
		m_ClientUpdateData.z = (unsigned short)z;
		m_ClientUpdateData.angle = (unsigned short)angle;

		memcpy( pUpdateData, &m_ClientUpdateData, sizeof( ClientUpdateData_t ) );
		memset( &m_ClientUpdateData, 0, sizeof( m_ClientUpdateData ) );

		return true;
	}
};

#endif
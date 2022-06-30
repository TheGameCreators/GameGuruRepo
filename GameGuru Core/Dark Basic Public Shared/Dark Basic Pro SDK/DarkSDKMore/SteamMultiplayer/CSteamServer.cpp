//========= Copyright © 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Main class for the space war game server
//
// $NoKeywords: $
//=============================================================================

#include "stdafx.h"
#include "CSteamServer.h"
#include "CClient.h"
#include "stdlib.h"
#include "time.h"
#include <math.h>

CSteamServer *g_pServer = NULL;
CSteamServer* Server() { return g_pServer; }

#ifdef _DEBUG_LOG_
extern int DEBUG_FLAG_ON;
#endif

int ServerAvatarfilesReceived = 0;

//-----------------------------------------------------------------------------
// Purpose: Constructor -- note the syntax for setting up Steam API callback handlers
//-----------------------------------------------------------------------------
CSteamServer::CSteamServer( ) 
:
	m_CallbackSteamServersConnected( this, &CSteamServer::OnSteamServersConnected ),
	m_CallbackSteamServersDisconnected( this, &CSteamServer::OnSteamServersDisconnected ),
	m_CallbackSteamServersConnectFailure( this, &CSteamServer::OnSteamServersConnectFailure ),
	m_CallbackPolicyResponse( this, &CSteamServer::OnPolicyResponse ),
	m_CallbackGSAuthTicketResponse( this, &CSteamServer::OnValidateAuthTicketResponse ),
	m_CallbackP2PSessionConnectFail( this, &CSteamServer::OnP2PSessionConnectFail ),
	m_CallbackP2PSessionRequest( this, &CSteamServer::OnP2PSessionRequest )
{
	m_bConnectedToSteam = false;

	const char *pchGameDir = "";
	uint32 unIP = INADDR_ANY;
	uint16 usMasterServerUpdaterPort = STEAM_MASTER_SERVER_UPDATER_PORT;

#ifdef USE_GS_AUTH_API
	EServerMode eMode = eServerModeAuthenticationAndSecure;
#else
	// Don't let Steam do authentication
	EServerMode eMode = eServerModeNoAuthentication;
#endif
	// Initialize the SteamGameServer interface, we tell it some info about us, and we request support
	// for both Authentication (making sure users own games) and secure mode, VAC running in our game
	// and kicking users who are VAC banned

	// !FIXME! We need a way to pass the dedicated server flag here!

	if ( !SteamGameServer_Init( unIP, STEAM_AUTHENTICATION_PORT, STEAM_SERVER_PORT, usMasterServerUpdaterPort, eMode, STEAM_SERVER_VERSION ) )
	{
		OutputDebugString( "SteamGameServer_Init call failed\n" );
	}

	m_serverLastSendLoadedAndReady = 0;
	m_serverLastSendReadyToPlay = 0;
	m_serverLastSendReadyToStart = 0;

	if ( SteamGameServer() )
	{

		// Set the "game dir".
		// This is currently required for all games.  However, soon we will be
		// using the AppID for most purposes, and this string will only be needed
		// for mods.  it may not be changed after the server has logged on
		SteamGameServer()->SetModDir( pchGameDir );

		// These fields are currently required, but will go away soon.
		// See their documentation for more info
		SteamGameServer()->SetProduct( "FPS Creator Reloaded" );
		SteamGameServer()->SetGameDescription( "Game Guru Multiplayer Game" );

		// We don't support specators in our game.
		// .... but if we did:
		//SteamGameServer()->SetSpectatorPort( ... );
		//SteamGameServer()->SetSpectatorServerName( ... );

		// Initiate Anonymous logon.
		// Coming soon: Logging into authenticated, persistent game server account
		SteamGameServer()->LogOnAnonymous();

		// We want to actively update the master server with our presence so players can
		// find us via the steam matchmaking/server browser interfaces
		#ifdef USE_GS_AUTH_API
			SteamGameServer()->EnableHeartbeats( true );
		#endif
	}
	else
	{
		OutputDebugString( "SteamGameServer() interface is invalid\n" );
	}

	m_uPlayerCount = 0;
	m_eGameState = k_EServerWaitingForPlayers;

	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		m_rguPlayerScores[i] = 0;
		m_rgpPlayer[i] = NULL;
		serverClientsFileSynced[i] = 0;
		serverClientsLoadedAndReady[i] = 0;
		serverClientsReadyToPlay[i] = 0;
		serverClientsHaveAvatarTexture[i] = 0;
	}

	// No one has won
	m_uPlayerWhoWonGame = 0;
	m_ulStateTransitionTime = (uint64)GetCounterPassedTotal(); // dave
	m_ulLastServerUpdateTick = 0;

	// zero the client connection data
	memset( &m_rgClientData, 0, sizeof( m_rgClientData ) );
	memset( &m_rgPendingClientData, 0, sizeof( m_rgPendingClientData ) );

	// Seed random num generator
	srand( (uint32)time( NULL ) );

	// Initialize players
	ResetPlayers();

	g_pServer = this;
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSteamServer::~CSteamServer()
{
#ifdef USE_GS_AUTH_API
	// Notify Steam master server we are going offline
	SteamGameServer()->EnableHeartbeats( false );
#endif

	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] )
		{
			// Tell this client we are exiting
			MsgServerExiting_t msg;
			BSendDataToClient( i, (char*)&msg, sizeof(msg) );

			delete m_rgpPlayer[i];
			m_rgpPlayer[i] = NULL;
		}
	}

	// Disconnect from the steam servers
	SteamGameServer()->LogOff();

	// release our reference to the steam client library
	SteamGameServer_Shutdown();
}


//-----------------------------------------------------------------------------
// Purpose: Handle clients connecting
//-----------------------------------------------------------------------------
void CSteamServer::OnP2PSessionRequest( P2PSessionRequest_t *pCallback )
{
	// we'll accept a connection from anyone
	SteamGameServerNetworking()->AcceptP2PSessionWithUser( pCallback->m_steamIDRemote );
}


//-----------------------------------------------------------------------------
// Purpose: Handle clients disconnecting
//-----------------------------------------------------------------------------
void CSteamServer::OnP2PSessionConnectFail( P2PSessionConnectFail_t *pCallback )
{
	// socket has closed, kick the user associated with it
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		// If there is no ship, skip
		if ( !m_rgClientData[i].m_bActive )
			continue;

		if ( m_rgClientData[i].m_SteamIDUser == pCallback->m_steamIDRemote )
		{
			OutputDebugString( "Disconnected dropped user\n" );
			RemovePlayerFromServer( i );
			break;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Handle sending data to a client at a given index
//-----------------------------------------------------------------------------
bool CSteamServer::BSendDataToClient( uint32 uShipIndex, char *pData, uint32 nSizeOfData )
{
	// Validate index
	if ( uShipIndex >= MAX_PLAYERS_PER_SERVER )
		return false;

	if ( !SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[uShipIndex].m_SteamIDUser, pData, nSizeOfData, k_EP2PSendUnreliable ) )
	{
		OutputDebugString( "Failed sending data to a client\n" );
		return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Handle sending data to a pending client at a given index
//-----------------------------------------------------------------------------
bool CSteamServer::BSendDataToPendingClient( uint32 uShipIndex, char *pData, uint32 nSizeOfData )
{
	// Validate index
	if ( uShipIndex >= MAX_PLAYERS_PER_SERVER )
		return false;

	if ( !SteamGameServerNetworking()->SendP2PPacket( m_rgPendingClientData[uShipIndex].m_SteamIDUser, pData, nSizeOfData, k_EP2PSendReliable ) )
	{
		OutputDebugString( "Failed sending data to a pending client\n" );
		return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Handle a new client connecting
//-----------------------------------------------------------------------------
void CSteamServer::OnClientBeginAuthentication( CSteamID steamIDClient, void *pToken, uint32 uTokenLen )
{
	// First, check this isn't a duplicate and we already have a user logged on from the same steamid
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i ) 
	{
		if ( m_rgClientData[i].m_SteamIDUser == steamIDClient )
		{
			// We already logged them on... (should maybe tell them again incase they don't know?)
			return;
		}
	}

	// Second, do we have room?
	uint32 nPendingOrActivePlayerCount = 0;
	for ( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgPendingClientData[i].m_bActive )
			++nPendingOrActivePlayerCount;
		
		if ( m_rgClientData[i].m_bActive )
			++nPendingOrActivePlayerCount;
	}

	// We are full (or will be if the pending players auth), deny new login
	if ( nPendingOrActivePlayerCount >=  MAX_PLAYERS_PER_SERVER )
	{
		MsgServerFailAuthentication_t msg;
		SteamGameServerNetworking()->SendP2PPacket( steamIDClient, &msg, sizeof( msg ), k_EP2PSendReliable );
	}

	// If we get here there is room, add the player as pending
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i ) 
	{
		if ( !m_rgPendingClientData[i].m_bActive )
		{
			m_rgPendingClientData[i].m_ulTickCountLastData = (uint64)GetCounterPassedTotal(); // dave
#ifdef USE_GS_AUTH_API
			// authenticate the user with the Steam back-end servers
			if ( k_EBeginAuthSessionResultOK != SteamGameServer()->BeginAuthSession( pToken, uTokenLen, steamIDClient ) )
			{
				MsgServerFailAuthentication_t msg;
				SteamGameServerNetworking()->SendP2PPacket( steamIDClient, &msg, sizeof( msg ), k_EP2PSendReliable );
				break;
			}

			m_rgPendingClientData[i].m_SteamIDUser = steamIDClient;
			m_rgPendingClientData[i].m_bActive = true;
			break;
#else
			m_rgPendingClientData[i].m_bActive = true;
			// we need to tell the server our Steam id in the non-auth case, so we stashed it in the login message, pull it back out
			m_rgPendingClientData[i].m_SteamIDUser = *(CSteamID *)pToken;
			// You would typically do your own authentication method here and later call OnAuthCompleted
			// In this sample we just automatically auth anyone who connects
			OnAuthCompleted( true, i );
			break;
#endif
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: A new client that connected has had their authentication processed
//-----------------------------------------------------------------------------
void CSteamServer::OnAuthCompleted( bool bAuthSuccessful, uint32 iPendingAuthIndex )
{
	if ( !m_rgPendingClientData[iPendingAuthIndex].m_bActive )
	{
		OutputDebugString( "Got auth completed callback for client who is not pending\n" );
		return;
	}

	if ( !bAuthSuccessful )
	{
#ifdef USE_GS_AUTH_API
		// Tell the GS the user is leaving the server
		SteamGameServer()->EndAuthSession( m_rgPendingClientData[iPendingAuthIndex].m_SteamIDUser );
#endif
		// Send a deny for the client, and zero out the pending data
		MsgServerFailAuthentication_t msg;
		SteamGameServerNetworking()->SendP2PPacket( m_rgPendingClientData[iPendingAuthIndex].m_SteamIDUser, &msg, sizeof( msg ), k_EP2PSendReliable );
		memset( &m_rgPendingClientData[iPendingAuthIndex], 0, sizeof( ClientConnectionData_t ) );
		return;
	}


	bool bAddedOk = false;
	if ( m_rgPendingClientData[iPendingAuthIndex].m_SteamIDUser == Client()->GetLocalSteamID() )
	{

#ifdef _DEBUG_LOG_
	log("SERVER -> ADDING SERVER PLAYER 0" , 0 );
#endif

		int i = 0;
		//if ( !m_rgClientData[i].m_bActive )
		{
			// copy over the data from the pending array
			memcpy( &m_rgClientData[i], &m_rgPendingClientData[iPendingAuthIndex], sizeof( ClientConnectionData_t ) );
			memset( &m_rgPendingClientData[iPendingAuthIndex], 0, sizeof( ClientConnectionData_t	) );
			m_rgClientData[i].m_ulTickCountLastData = (uint64)GetCounterPassedTotal(); // dave

			// Add a new player, make it dead immediately
			AddPlayer( i );
			m_rgpPlayer[i]->SetDisabled( 1 );

			MsgServerPassAuthentication_t msg;
			msg.SetPlayerPosition( i );
			BSendDataToClient( i, (char*)&msg, sizeof( msg ) );

			bAddedOk = true;
		}
	}
	else
	{
		for( uint32 i = 1; i < MAX_PLAYERS_PER_SERVER; ++i ) 
		{
			if ( !m_rgClientData[i].m_bActive )
			{

#ifdef _DEBUG_LOG_
	log("SERVER -> ADDING CLIENT PLAYER " , i );
#endif

				// copy over the data from the pending array
				memcpy( &m_rgClientData[i], &m_rgPendingClientData[iPendingAuthIndex], sizeof( ClientConnectionData_t ) );
				memset( &m_rgPendingClientData[iPendingAuthIndex], 0, sizeof( ClientConnectionData_t	) );
				m_rgClientData[i].m_ulTickCountLastData = (uint64)GetCounterPassedTotal(); // dave

				// Add a new player, make it dead immediately
				AddPlayer( i );
				m_rgpPlayer[i]->SetDisabled( 1 );

				MsgServerPassAuthentication_t msg;
				msg.SetPlayerPosition( i );
				BSendDataToClient( i, (char*)&msg, sizeof( msg ) );

				bAddedOk = true;

				break;
			}
		}
	}

	// If we just successfully added the player, check if they are #2 so we can restart the round
	if ( bAddedOk )
	{
		uint32 uPlayers = 0;
		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i ) 
		{
			if ( m_rgClientData[i].m_bActive )
				++uPlayers;
		}

		// If we just got the second player, immediately reset round as a draw.  This will prevent
		// the existing player getting a win, and it will cause a new round to start right off
		// so that the one player can't just float around not letting the new one get into the game.
		/*if ( uPlayers == 2 )
		{
			if ( m_eGameState != k_EServerWaitingForPlayers )
				SetGameState( k_EServerDraw );
		}*/
	}
}


//-----------------------------------------------------------------------------
// Purpose: Used to reset scores (at start of a new game usually)
//-----------------------------------------------------------------------------
void CSteamServer::ResetScores()
{
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		m_rguPlayerScores[i] = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a new player at given position
//-----------------------------------------------------------------------------
void CSteamServer::AddPlayer( uint32 uPosition )
{
	if ( uPosition >= MAX_PLAYERS_PER_SERVER )
	{
		OutputDebugString( "Trying to add player at invalid positon\n" );
		return;
	}

	if ( m_rgpPlayer[uPosition] )
	{
		OutputDebugString( "Trying to add a player where one already exists\n" );
		return;
	}	

	m_rgpPlayer[uPosition] = new CPlayer();
	m_rgpPlayer[uPosition]->x = 0;
	m_rgpPlayer[uPosition]->y = 0;
	m_rgpPlayer[uPosition]->z = 0;
	m_rgpPlayer[uPosition]->angle = 0;

	/*switch( uPosition )
	{
	case 0:
		m_rgpPlayer[uPosition] = new CPlayer();
		m_rgpPlayer[uPosition]->x = 0;
		m_rgpPlayer[uPosition]->y = 0;
		m_rgpPlayer[uPosition]->z = 0;
		m_rgpPlayer[uPosition]->angle = 0;
		break;
	case 1:
		m_rgpPlayer[uPosition] = new CPlayer();
		m_rgpPlayer[uPosition]->x = 0;
		m_rgpPlayer[uPosition]->y = 0;
		m_rgpPlayer[uPosition]->z = 100;
		m_rgpPlayer[uPosition]->angle = 0;
		break;
	case 2:
		m_rgpPlayer[uPosition] = new CPlayer();
		m_rgpPlayer[uPosition]->x = 100;
		m_rgpPlayer[uPosition]->y = 0;
		m_rgpPlayer[uPosition]->z = 0;
		m_rgpPlayer[uPosition]->angle = 0;
		break;
	case 3:
		m_rgpPlayer[uPosition] = new CPlayer();
		m_rgpPlayer[uPosition]->x = 100;
		m_rgpPlayer[uPosition]->y = 0;
		m_rgpPlayer[uPosition]->z = 100;
		m_rgpPlayer[uPosition]->angle = 0;
		break;
	default:
		OutputDebugString( "AddPlayer() code needs updating for more than 4 players\n" );
	}*/


}


//-----------------------------------------------------------------------------
// Purpose: Removes a player at the given position
//-----------------------------------------------------------------------------
void CSteamServer::RemovePlayerFromServer( uint32 uPosition )
{
	if ( uPosition >= MAX_PLAYERS_PER_SERVER )
	{
		OutputDebugString( "Trying to remove player at invalid position\n" );
		return;
	}

	if ( !m_rgpPlayer[uPosition] )
	{
		OutputDebugString( "Trying to remove a player that does not exist\n" );
		return;
	}

	delete m_rgpPlayer[uPosition];
	m_rgpPlayer[uPosition] = NULL;
	m_rguPlayerScores[uPosition] = 0;	

	// left everyone else know the player has left the game
	MsgClientLeft_t msg;
	msg.index = uPosition;
	
	for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] )
			SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgClientLeft_t), k_EP2PSendReliable );
	}

	// if the server is in the process of sending files, checking everyone is ready etc we need to update the checks
	// as we may be waiting on this person to say they are ready - so we retest these states


#ifdef USE_GS_AUTH_API
	// Tell the GS the user is leaving the server
	SteamGameServer()->EndAuthSession( m_rgClientData[uPosition].m_SteamIDUser );
#endif
	memset( &m_rgClientData[uPosition], 0, sizeof( ClientConnectionData_t ) );
}


//-----------------------------------------------------------------------------
// Purpose: Used to reset players positions for a new round
//-----------------------------------------------------------------------------
void CSteamServer::ResetPlayers()
{
	// Delete any currently active players, but immediately recreate 
	// (which causes all players state/position to reset)
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] )
		{		
			delete m_rgpPlayer[i];
			m_rgpPlayer[i] = NULL;
			AddPlayer( i );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Used to transition game state
//-----------------------------------------------------------------------------
void CSteamServer::SetGameState( EServerGameState eState )
{
	if ( m_eGameState == eState )
		return;

	// If we were in waiting for players and are now going active clear old scores
	if ( m_eGameState == k_EServerWaitingForPlayers && eState == k_EServerActive )
	{
		ResetScores();
		ResetPlayers();
	}

	m_ulStateTransitionTime = (uint64)GetCounterPassedTotal(); // dave
	m_eGameState = eState;
}


//-----------------------------------------------------------------------------
// Purpose: Receives incoming network data
//-----------------------------------------------------------------------------
void CSteamServer::ReceiveNetworkData()
{
	char *pchRecvBuf = NULL;
	uint32 cubMsgSize;
	CSteamID steamIDRemote;
	while ( SteamGameServerNetworking()->IsP2PPacketAvailable( &cubMsgSize ) )
	{
		// free any previous receive buffer
		if ( pchRecvBuf )
			free( pchRecvBuf );

		// alloc a new receive buffer of the right size
		pchRecvBuf = (char *)malloc( cubMsgSize );

		// see if there is any data waiting on the socket
		if ( !SteamGameServerNetworking()->ReadP2PPacket( pchRecvBuf, cubMsgSize, &cubMsgSize, &steamIDRemote ) )
			break;

		if ( cubMsgSize < sizeof( DWORD ) )
		{
			OutputDebugString( "Got garbage on server socket, too short\n" );
			continue;
		}

		EMessage eMsg = (EMessage)LittleDWord( *(DWORD*)pchRecvBuf );
#ifdef _DEBUG_LOG_
			if ( DEBUG_FLAG_ON )
				log("Server Message Received" , eMsg );
#endif
		switch ( eMsg )
		{
		case k_EMsgReceipt:
			{
				if ( cubMsgSize == sizeof( MsgReceipt_t ) )
				{
					MsgReceipt_t* pmsg = (MsgReceipt_t*)pchRecvBuf;
					int index = pmsg->logID;

#ifdef _DEBUG_LOG_
			if ( DEBUG_FLAG_ON )
			{
				int found = -1;
				for ( unsigned int i = 0; i < PacketSend_Log_Server.size() ; i++ )
				{
					if ( PacketSend_Log_Server[i].LogID == pmsg->logID ) 
					{
						found = i;
						break;
					}
				}

				log("Server Message Receipt Code" , PacketSend_Log_Server[found].packetType );
			}
#endif

					GotReceipt(index);
				}
			} break;
		case k_EMsgClientPlayerName:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerName_t ) )
				{
					MsgClientPlayerName_t* pmsg = (MsgClientPlayerName_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerName_t), k_EP2PSendReliable );
					}
				}
			}
			break;
		case k_EMsgClientPlayerScore:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerScore_t ) )
				{
					MsgClientPlayerScore_t* pmsg = (MsgClientPlayerScore_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerScore_t), k_EP2PSendReliable );
					}
				}
			}
			break;
		case k_EMsgClientShoot:
			{
				if ( cubMsgSize == sizeof( MsgClientShoot_t ) )
				{
					MsgClientShoot_t* pmsg = (MsgClientShoot_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientShoot_t), k_EP2PSendUnreliable );
					}
				}
			}
			break;
		case k_EMsgClientSetCollision:
			{
				if ( cubMsgSize == sizeof( MsgClientSetCollision_t ) )
				{
					MsgClientSetCollision_t* pmsg = (MsgClientSetCollision_t*)pchRecvBuf;
					int index = pmsg->playerIndex;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSetCollision_t), k_EP2PSendReliable );
					}
				}
			}
			break;
		case k_EMsgClientPlayAnimation:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayAnimation_t ) )
				{
					MsgClientPlayAnimation_t* pmsg = (MsgClientPlayAnimation_t*)pchRecvBuf;
					int index = pmsg->playerIndex;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
						{
							if ( ShouldISend ( k_EMsgClientPlayAnimation , 0 , m_rgpPlayer[i]->newx, m_rgpPlayer[i]->newy, m_rgpPlayer[i]->newz, index ) )
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayAnimation_t), k_EP2PSendReliable );
						}
					}
				}
			}
			break;
		case k_EMsgClientPlayerBullet:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerBullet_t ) )
				{
					MsgClientPlayerBullet_t* pmsg = (MsgClientPlayerBullet_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )//&& i != pmsg->playerID )
						{
							if ( ShouldISend ( k_EMsgClientPlayerBullet , pmsg->index, pmsg->x, pmsg->y, pmsg->z, i ) )
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerBullet_t), k_EP2PSendUnreliable );

							if ( pmsg->on == 0 ) 
							{
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerBullet_t), k_EP2PSendReliable );
								bulletSeen[pmsg->index][i] = false;
							}
						}
					}
				}
			}
			break;
		case k_EMsgClientPlayerKeyState:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerKeyState_t ) )
				{
					MsgClientPlayerKeyState_t* pmsg = (MsgClientPlayerKeyState_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerKeyState_t), k_EP2PSendUnreliable );
					}
				}
			}
			break;
		case k_EMsgClientPlayerApplyDamage:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerApplyDamage_t ) )
				{
					MsgClientPlayerApplyDamage_t* pmsg = (MsgClientPlayerApplyDamage_t*)pchRecvBuf;
					int index = pmsg->index;

					if ( m_rgpPlayer[pmsg->source] )
					{
						MsgReceipt_t msg;
						msg.logID = pmsg->logID;
						SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[pmsg->source].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );
					}
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i == index )
						{

							MsgClientPlayerApplyDamage_t* pmsg2;
							pmsg2 = new MsgClientPlayerApplyDamage_t();
							pmsg2->logID = packetSendLogServerID;
							pmsg2->index = pmsg->index;
							pmsg2->source = pmsg->source;
							pmsg2->amount = pmsg->amount;
							pmsg2->x = pmsg->x;
							pmsg2->y = pmsg->x;
							pmsg2->z = pmsg->z;
							pmsg2->force = pmsg->force;
							pmsg2->limb = pmsg->limb;

							packetSendLogServer_t log;
							log.LogID = packetSendLogServerID++;
							log.packetType = k_EMsgClientPlayerApplyDamage;
							log.pPacket = pmsg2;
							log.timeStamp = GetCounterPassedTotal();
							log.playerID = i;

							PacketSend_Log_Server.push_back(log);

							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg2, sizeof(MsgClientPlayerApplyDamage_t), k_EP2PSendUnreliable );
						}
					}
				}
			}
			break;
		case k_EMsgClientPlayerSetPlayerAlive:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerSetPlayerAlive_t ) )
				{
					MsgClientPlayerSetPlayerAlive_t* pmsg = (MsgClientPlayerSetPlayerAlive_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerSetPlayerAlive_t), k_EP2PSendUnreliable );
					}

				}
			}
			break;
		case k_EMsgClientSpawnObject:
			{
				if ( cubMsgSize == sizeof( MsgClientSpawnObject_t ) )
				{
					MsgClientSpawnObject_t* pmsg = (MsgClientSpawnObject_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSpawnObject_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientLua:
			{
				if ( cubMsgSize == sizeof( MsgClientLua_t ) )
				{
					MsgClientLua_t* pmsg = (MsgClientLua_t*)pchRecvBuf;
					int index = pmsg->index;

#ifdef _DEBUG_LOG_
			if ( DEBUG_FLAG_ON )
				log("Server Message Received Code" , pmsg->code );
#endif

					// if set animation or play animation they need to be sent in order
					if ( pmsg->code < 5 )
					{			

						MsgServerLua_t msg;
						msg.logID = packetSendLogServerID;
						msg.code = pmsg->code;
						msg.e = pmsg->e;
						msg.v = pmsg->v;

						for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
						{
							if ( m_rgpPlayer[i] && i != index )
							{
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgServerLua_t), k_EP2PSendReliable );
							}
						}
					}
					else
					{

						// no ticket if goto position
						if ( pmsg->code != 18 && pmsg->code != 19 )
						{
							if ( m_rgpPlayer[pmsg->index] )
							{
								MsgReceipt_t msg;
								msg.logID = pmsg->logID;
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[pmsg->index].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );
							}
					
							// add kill, remove kill and add death only get sent to the server
							if ( pmsg->code >= 12 && pmsg->code <= 14 )
							{
								if ( m_rgpPlayer[0] )
								{
									MsgServerLua_t* pmsg2;
									pmsg2 = new MsgServerLua_t();
									pmsg2->logID = packetSendLogServerID;
									pmsg2->code = pmsg->code;
									pmsg2->e = pmsg->e;
									pmsg2->v = pmsg->v;

									packetSendLogServer_t log;
									log.LogID = packetSendLogServerID++;
									log.packetType = k_EMsgServerLua;
									log.pPacket = pmsg2;
									log.timeStamp = GetCounterPassedTotal();
									log.playerID = index;

									PacketSend_Log_Server.push_back(log);

									SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[0].m_SteamIDUser, pmsg2, sizeof(MsgServerLua_t), k_EP2PSendUnreliable );
								}
								return;
							}

							for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
							{
								if ( m_rgpPlayer[i] && i != index )
								{
									MsgServerLua_t* pmsg2;
									pmsg2 = new MsgServerLua_t();
									pmsg2->logID = packetSendLogServerID;
									pmsg2->code = pmsg->code;
									pmsg2->e = pmsg->e;
									pmsg2->v = pmsg->v;

									packetSendLogServer_t log;
									log.LogID = packetSendLogServerID++;
									log.packetType = k_EMsgServerLua;
									log.pPacket = pmsg2;
									log.timeStamp = GetCounterPassedTotal();
									log.playerID = index;

									PacketSend_Log_Server.push_back(log);

									SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg2, sizeof(MsgServerLua_t), k_EP2PSendUnreliable );
								}
							}
						}
						else
						{			

							MsgServerLua_t msg;
							msg.logID = packetSendLogServerID;
							msg.code = pmsg->code;
							msg.e = pmsg->e;
							msg.v = pmsg->v;

							for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
							{
								if ( m_rgpPlayer[i] && i != index )
								{
									SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgServerLua_t), k_EP2PSendUnreliable );
								}
							}
						}
					}

				}
			}
			break;
		case k_EMsgClientLuaString:
			{
				if ( cubMsgSize == sizeof( MsgClientLuaString_t ) )
				{
					MsgClientLuaString_t* pmsg = (MsgClientLuaString_t*)pchRecvBuf;
					int index = pmsg->index;

					if ( m_rgpPlayer[pmsg->index] )
					{
						MsgReceipt_t msg;
						msg.logID = pmsg->logID;
						SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[pmsg->index].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );
					}
					
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
						{
							MsgServerLuaString_t* pmsg2;
							pmsg2 = new MsgServerLuaString_t();
							pmsg2->logID = packetSendLogServerID;
							pmsg2->code = pmsg->code;
							pmsg2->e = pmsg->e;
							strcpy ( pmsg2->s , pmsg->s );

							packetSendLogServer_t log;
							log.LogID = packetSendLogServerID++;
							log.packetType = k_EMsgServerLua;
							log.pPacket = pmsg2;
							log.timeStamp = GetCounterPassedTotal();
							log.playerID = index;

							PacketSend_Log_Server.push_back(log);

							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg2, sizeof(MsgServerLuaString_t), k_EP2PSendUnreliable );
						}
					}

				}
			}
			break;			
		case k_EMsgClientAvatarDoWeHaveHeadTex:
			{
				if ( cubMsgSize == sizeof( MsgClientAvatarDoWeHaveHeadTex_t ) )
				{
					MsgClientAvatarDoWeHaveHeadTex_t* pmsg = (MsgClientAvatarDoWeHaveHeadTex_t*)pchRecvBuf;
					int index = pmsg->index;

					if ( m_rgpPlayer[pmsg->index] )
					{
						MsgReceipt_t msg;
						msg.logID = pmsg->logID;
						SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[pmsg->index].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );

						serverClientsHaveAvatarTexture[index] = pmsg->flag;
					}					

				}
			}
			break;
		case k_EMsgClientDeleteObject:
			{
				if ( cubMsgSize == sizeof( MsgClientDeleteObject_t ) )
				{
					MsgClientDeleteObject_t* pmsg = (MsgClientDeleteObject_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientDeleteObject_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientDestroyObject:
			{
				if ( cubMsgSize == sizeof( MsgClientDestroyObject_t ) )
				{
					MsgClientDestroyObject_t* pmsg = (MsgClientDestroyObject_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientDestroyObject_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientKilledBy:
			{
				if ( cubMsgSize == sizeof( MsgClientKilledBy_t ) )
				{
					MsgClientKilledBy_t* pmsg = (MsgClientKilledBy_t*)pchRecvBuf;
					int index = pmsg->index;

					if ( m_rgpPlayer[index] )
					{
						MsgReceipt_t msg;
						msg.logID = pmsg->logID;
						SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );
					}
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
						{

							MsgClientKilledBy_t* pmsg2;
							pmsg2 = new MsgClientKilledBy_t();
							pmsg2->index = pmsg->index;
							pmsg2->logID = packetSendLogServerID;
							pmsg2->killedBy = pmsg->killedBy;
							pmsg2->x = pmsg->x;
							pmsg2->y = pmsg->y;
							pmsg2->z = pmsg->z;
							pmsg2->force = pmsg->force;
							pmsg2->limb = pmsg->limb;

							packetSendLogServer_t log;
							log.LogID = packetSendLogServerID++;
							log.packetType = k_EMsgClientKilledBy;
							log.pPacket = pmsg2;
							log.timeStamp = GetCounterPassedTotal();
							log.playerID = i;

							PacketSend_Log_Server.push_back(log);

							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg2, sizeof(MsgClientKilledBy_t), k_EP2PSendUnreliable );
						}
					}

				}
			}
			break;
		case k_EMsgChat:
			{
				if ( cubMsgSize == sizeof( MsgChat_t ) )
				{
					MsgChat_t* pmsg = (MsgChat_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
						{

							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgChat_t), k_EP2PSendReliable );
						}
					}

				}
			}
			break;
		case k_EMsgClientKilledSelf:
			{
				if ( cubMsgSize == sizeof( MsgClientKilledSelf_t ) )
				{
					MsgClientKilledSelf_t* pmsg = (MsgClientKilledSelf_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientKilledSelf_t), k_EP2PSendUnreliable );
					}

				}
			}
			break;
		case k_EMsgClientPlayerAppearance:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerAppearance_t ) )
				{
					MsgClientPlayerAppearance_t* pmsg = (MsgClientPlayerAppearance_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
						{
							//if ( ShouldISend ( k_EMsgClientPlayerAppearance , 0 , m_rgpPlayer[i]->newx, m_rgpPlayer[i]->newy, m_rgpPlayer[i]->newz, index ) )
								SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerAppearance_t), k_EP2PSendUnreliable );
						}
					}

				}
			}
			break;
		case k_EMsgClientSetSendFileCount:
			{
				if ( cubMsgSize == sizeof( MsgClientSetSendFileCount_t ) )
				{
					MsgClientSetSendFileCount_t* pmsg = (MsgClientSetSendFileCount_t*)pchRecvBuf;
					int count = pmsg->count;
					
					//Dont need to send this to our own client as we already have all the files
					for( uint32 i=1; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSetSendFileCount_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientSendFileBegin:
			{
				if ( cubMsgSize == sizeof( MsgClientSendFileBegin_t ) )
				{
					MsgClientSendFileBegin_t* pmsg = (MsgClientSendFileBegin_t*)pchRecvBuf;
					
					//Dont need to send this to our own client as we already have all the files
					for( uint32 i=1; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSendFileBegin_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientSendChunk:
			{
				if ( cubMsgSize == sizeof( MsgClientSendChunk_t ) )
				{
					MsgClientSendChunk_t* pmsg = (MsgClientSendChunk_t*)pchRecvBuf;
					
					//Dont need to send this to our own client as we already have all the files
					for( uint32 i=1; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSendChunk_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientSendAvatarFileBeginClient:
			{
				if ( cubMsgSize == sizeof( MsgClientSendAvatarFileBeginClient_t ) )
				{
					MsgClientSendAvatarFileBeginClient_t* pmsg = (MsgClientSendAvatarFileBeginClient_t*)pchRecvBuf;

					int index = pmsg->index;
					
					//Dont need to send this to the owning client as they already have their own avatar
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSendAvatarFileBeginClient_t), k_EP2PSendReliable );
					}

				}
			}
			break;
		case k_EMsgClientSendAvatarChunkClient:
			{
				if ( cubMsgSize == sizeof( MsgClientSendAvatarChunkClient_t ) )
				{
					MsgClientSendAvatarChunkClient_t* pmsg = (MsgClientSendAvatarChunkClient_t*)pchRecvBuf;
					
					//Dont need to send this to the owning client as they already have their own avatar
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != pmsg->index )
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientSendAvatarChunkClient_t), k_EP2PSendReliable );
					}

				}
			}
			break;			
		case k_EMsgClientSendAvatarDone:
			{
				if ( cubMsgSize == sizeof( MsgClientSendAvatarDone_t ) )
				{
					MsgClientSendAvatarDone_t* pmsg = (MsgClientSendAvatarDone_t*)pchRecvBuf;

					int index = pmsg->index;
					
					if ( serverClientsHaveAvatarTexture[index] != 0 )
					{
						serverClientsHaveAvatarTexture[index] = 0;
						ServerAvatarfilesReceived++;
					}

				}
			}
			break;
		case k_EMsgClientPlayerIamSyncedWithServerFiles:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerIamSyncedWithServerFiles_t ) )
				{
					MsgClientPlayerIamSyncedWithServerFiles_t* pmsg = (MsgClientPlayerIamSyncedWithServerFiles_t*)pchRecvBuf;

					int index = pmsg->index;
					
					if ( m_rgpPlayer[index] )
						serverClientsFileSynced[index] = 1;							
				}
			}
			break;
		case k_EMsgClientPlayerSendIAmLoadedAndReady:
			{
				if ( cubMsgSize == sizeof( MsgClientSendIAmLoadedAndReady_t ) )
				{
					MsgClientSendIAmLoadedAndReady_t* pmsg = (MsgClientSendIAmLoadedAndReady_t*)pchRecvBuf;

					int index = pmsg->index;
					
					if ( m_rgpPlayer[index] )
					{
						serverClientsLoadedAndReady[index] = 1;		

						bool allReady = true;

						int howManyPlayers = 0;

						//Client attached to the server needs to be loaded and ready too!
						for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
						{
							if ( m_rgpPlayer[i] )
							{
								howManyPlayers++;
								if ( serverClientsLoadedAndReady[i] == 0 )
								{
									allReady = false;
									break;
								}
							}
						}


						// Make sure everyone is ready that initially joined the lobby
						// everyone is ready to play, send out the message to start the game proper
						if ( allReady && howManyPlayers == HowManyPlayersDoWeHave )
						{
							MsgClientEveryoneLoadedAndReady_t msg ;

							for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
							{
								if ( m_rgpPlayer[i] )
								{
									SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgClientEveryoneLoadedAndReady_t), k_EP2PSendReliable );
								}
							}
						}
					}
				}
			}
			break;
		case k_EMsgClientPlayerSendIAmReadyToPlay:
			{
				if ( cubMsgSize == sizeof( MsgClientSendIAmReadyToPlay_t ) )
				{
					MsgClientSendIAmReadyToPlay_t* pmsg = (MsgClientSendIAmReadyToPlay_t*)pchRecvBuf;

					// change to how many we ended up with for real rather than how many were in the initial lobby
					HowManyPlayersDoWeHave = m_uPlayerCount;

					int index = pmsg->index;

					// send back receipt
					// dont need to store the fact we sent one in this instance
					if ( m_rgpPlayer[index] )
					{
						MsgReceipt_t msg;
						msg.logID = pmsg->logID;
						SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, &msg, sizeof(MsgReceipt_t), k_EP2PSendUnreliable );
					}
					
					if ( m_rgpPlayer[index] )
					{
						serverClientsReadyToPlay[index] = 1;		

						bool allReady = true;

						//Client attached to the server needs to be loaded and ready too!
						for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
						{
							if ( m_rgpPlayer[i] && serverClientsReadyToPlay[i] == 0 )
							{
								allReady = false;
								break;
							}
						}

						// everyone is ready to play, send out the message to start the game proper
						/*if ( allReady )
						{
							MsgClientEveryoneReadyToPlay_t msg ;

							for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
							{
								if ( m_rgpPlayer[i] )
								{
									SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgClientEveryoneReadyToPlay_t), k_EP2PSendReliable );
								}
							}
						}*/
					}
				}
			}
			break;
		case k_EMsgClientInitiateConnection:
			{
				// We always let clients do this without even checking for room on the server since we reserve that for 
				// the authentication phase of the connection which comes next
				MsgServerSendInfo_t msg;
				msg.SetSteamIDServer( SteamGameServer()->GetSteamID().ConvertToUint64() );
#ifdef USE_GS_AUTH_API
				// You can only make use of VAC when using the Steam authentication system
				msg.SetSecure( SteamGameServer()->BSecure() );
#endif
				msg.SetServerName( m_sServerName );
				SteamGameServerNetworking()->SendP2PPacket( steamIDRemote, &msg, sizeof( MsgServerSendInfo_t ), k_EP2PSendReliable );
			}
			break;
		case k_EMsgClientBeginAuthentication:
			{
				if ( cubMsgSize != sizeof( MsgClientBeginAuthentication_t ) )
				{
					OutputDebugString( "Bad connection attempt msg\n" );
					continue;
				}
				MsgClientBeginAuthentication_t *pMsg = (MsgClientBeginAuthentication_t*)pchRecvBuf;
#ifdef USE_GS_AUTH_API
				OnClientBeginAuthentication( steamIDRemote, (void*)pMsg->GetTokenPtr(), pMsg->GetTokenLen() );
#else
				OnClientBeginAuthentication( steamIDRemote, 0 );
#endif
			}
			break;
		case k_EMsgClientSendLocalUpdate:
			{
				if ( cubMsgSize != sizeof( MsgClientSendLocalUpdate_t ) )
				{
					OutputDebugString( "Bad client update msg\n" );
					continue;
				}

				// Find the connection that should exist for this users address
				bool bFound = false;
				for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
				{
					if ( m_rgClientData[i].m_SteamIDUser == steamIDRemote ) 
					{
						bFound = true;
						MsgClientSendLocalUpdate_t *pMsg = (MsgClientSendLocalUpdate_t*)pchRecvBuf;
						OnReceiveClientUpdateData( i, pMsg->AccessUpdateData() );
						break;
					}
				}
				if ( !bFound )
					OutputDebugString( "Got a client data update, but couldn't find a matching client\n" );
			}
			break;
		case k_EMsgClientPing:
			{
				// send back a response
				MsgServerPingResponse_t msg;
				SteamGameServerNetworking()->SendP2PPacket( steamIDRemote, &msg, sizeof( msg ), k_EP2PSendUnreliable );
			}
			break;
		case k_EMsgClientLeavingServer:
			{
				if ( cubMsgSize != sizeof( MsgClientLeavingServer_t ) )
				{
					OutputDebugString( "Bad leaving server msg\n" );
					continue;
				}
				// Find the connection that should exist for this users address
				bool bFound = false;
				for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
				{
					if ( m_rgClientData[i].m_SteamIDUser == steamIDRemote )
					{
						bFound = true;
						RemovePlayerFromServer( i );
						break;
					}

					// Also check for pending connections that may match
					if ( m_rgPendingClientData[i].m_SteamIDUser == steamIDRemote )
					{
	#ifdef USE_GS_AUTH_API
						// Tell the GS the user is leaving the server
						SteamGameServer()->SendUserDisconnect( m_rgPendingClientData[i].m_SteamIDUser );
	#endif
						// Clear our data on the user
						memset( &m_rgPendingClientData[i], 0 , sizeof( ClientConnectionData_t ) );
						break;
					}
				}
				if ( !bFound )
					OutputDebugString( "Got a client leaving server msg, but couldn't find a matching client\n" );
			}
		default:
			char rgch[128];
			sprintf_safe( rgch, "Invalid message %x\n", eMsg );
			rgch[ sizeof(rgch) - 1 ] = 0;
			OutputDebugString( rgch );
		}
	}

	if ( pchRecvBuf )
		free( pchRecvBuf );
}

//-----------------------------------------------------------------------------
// Purpose: Main frame function, updates the state of the world and performs rendering
//-----------------------------------------------------------------------------
void CSteamServer::RunFrame()
{
	// Run any Steam Game Server API callbacks
	SteamGameServer_RunCallbacks();

	// Update our server details
	SendUpdatedServerDetailsToSteam();

	CheckReceipts();

	// Check if everyone is connected and if we haven't sent the start message, lets send it
	if ( ServerHaveIToldClientsToStart == 0 )
	{
		ServerHowManyJoined = 0;
		for ( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
		{	
			if ( m_rgClientData[i].m_bActive && m_rgClientData[i].m_SteamIDUser.IsValid() )
				ServerHowManyJoined++;
		}


		if ( ServerHowManyJoined >= ServerHowManyToStart || GetCounterPassedTotal() - ServerCreationTime > MILLISECONDS_CONNECTION_TIMEOUT )
		{
			ServerHaveIToldClientsToStart = 1;

			MsgClientServerReadyForSpawn_t msg;
			msg.index = 0;

			for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
			{
				if ( m_rgpPlayer[i] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgClientServerReadyForSpawn_t), k_EP2PSendReliable );
			}
		}
	}

	// Timeout stale player connections, also update player count data
	uint32 uPlayerCount = 0;
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		// If there is no player, skip
		if ( !m_rgClientData[i].m_bActive )
			continue;

		if ( GetCounterPassedTotal() - m_rgClientData[i].m_ulTickCountLastData > server_timeout_milliseconds ) // dave
		{
			OutputDebugString( "Timing out player connection\n" );
			RemovePlayerFromServer( i );
		}
		else
		{
			++uPlayerCount;
		}
	}
	m_uPlayerCount = uPlayerCount;

	switch ( m_eGameState )
	{
	case k_EServerWaitingForPlayers:
		// Wait a few seconds (so everyone can join if a lobby just started this server)
		if ( GetCounterPassedTotal() - m_ulStateTransitionTime >= MILLISECONDS_BETWEEN_ROUNDS ) // dave
		{
			// Just keep waiting until at least one player is active
			for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
			{
				if ( m_rgClientData[i].m_bActive )
				{
					// Transition to active
					OutputDebugString( "Server going active after waiting for players\n" );
					SetGameState( k_EServerActive );
				}
			}
		}
		break;
	case k_EServerDraw:
	case k_EServerWinner:
		// Update all the entities...

		// NOTE: no collision detection, because the round is really over, objects are now invulnerable

		// After 5 seconds start the next round
		if ( GetCounterPassedTotal() - m_ulStateTransitionTime >= MILLISECONDS_BETWEEN_ROUNDS ) // dave
		{
			ResetPlayers();
			SetGameState( k_EServerActive );
		}

		break;

	case k_EServerActive:
		// Update all the entities...
		
		// Check for collisions which could lead to a winner this round
		CheckForCollisions();

		break;
	case k_EServerExiting:
		break;
	default:
		OutputDebugString( "Unhandled game state in CFPSCRServer::RunFrame\n" );
	}

	// Send client updates (will internal limit itself to the tick rate desired)
	SendUpdateDataToAllClients();
}


//-----------------------------------------------------------------------------
// Purpose: Sends updates to all connected clients
//-----------------------------------------------------------------------------
void CSteamServer::SendUpdateDataToAllClients()
{
	// Limit the rate at which we update, even if our internal frame rate is higher
	if ( GetCounterPassedTotal() - m_ulLastServerUpdateTick < 1000.0f/SERVER_UPDATE_SEND_RATE ) // dave
		return;

	m_ulLastServerUpdateTick = (uint64)GetCounterPassedTotal(); // dave

	MsgServerUpdateWorld_t msg;

	msg.AccessUpdateData()->SetServerGameState( m_eGameState );
	for( int i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
	{
		msg.AccessUpdateData()->SetPlayerActive( i, m_rgClientData[i].m_bActive );
		//msg.AccessUpdateData()->SetPlayerScore( i, m_rguPlayerScores[i]  );
		msg.AccessUpdateData()->SetPlayerSteamID( i, m_rgClientData[i].m_SteamIDUser.ConvertToUint64() );

		if ( m_rgpPlayer[i] )
		{
			m_rgpPlayer[i]->BuildServerUpdate( msg.AccessUpdateData()->AccessPlayerUpdateData( i ) );
		}
	}

	//msg.AccessUpdateData()->SetPlayerWhoWon( m_uPlayerWhoWonGame );
	
	for( int i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( !m_rgClientData[i].m_bActive ) 
			continue;

		BSendDataToClient( i, (char*)&msg, sizeof( msg ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Receives update data from clients
//-----------------------------------------------------------------------------
void CSteamServer::OnReceiveClientUpdateData( uint32 uIndex, ClientSteamUpdateData_t *pUpdateData )
{
	if ( m_rgClientData[uIndex].m_bActive && m_rgpPlayer[uIndex] )
	{
		m_rgClientData[uIndex].m_ulTickCountLastData = (uint64)GetCounterPassedTotal(); // dave
		m_rgpPlayer[uIndex]->OnReceiveClientUpdate( pUpdateData );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Checks various game objects for collisions and updates state 
//			appropriately if they have occurred
//-----------------------------------------------------------------------------
void CSteamServer::CheckForCollisions()
{
/*
	// draw
	if ( 0 ) // dave it is a draw - everyone dead
	{
		SetGameState( k_EServerDraw );
	}
	else if ( 0 ) // winner
	{
		// one player alive, they win
		m_uPlayerWhoWonGame = 0; // dave the winner
		m_rguPlayerScores[0]++; // dave winner
		SetGameState( k_EServerWinner );
	}
	*/
}


//-----------------------------------------------------------------------------
// Purpose: Take any action we need to on Steam notifying us we are now logged in
//-----------------------------------------------------------------------------
void CSteamServer::OnSteamServersConnected( SteamServersConnected_t *pLogonSuccess )
{
	OutputDebugString( "FPSCRServer connected to Steam successfully\n" );
	m_bConnectedToSteam = true;

	// log on is not finished until OnPolicyResponse() is called

	// Tell Steam about our server details
	SendUpdatedServerDetailsToSteam();
}


//-----------------------------------------------------------------------------
// Purpose: Callback from Steam when logon is fully completed and VAC secure policy is set
//-----------------------------------------------------------------------------
void CSteamServer::OnPolicyResponse( GSPolicyResponse_t *pPolicyResponse )
{
#ifdef USE_GS_AUTH_API
	// Check if we were able to go VAC secure or not
	if ( SteamGameServer()->BSecure() )
	{
		OutputDebugString( "FPSCRServer is VAC Secure!\n" );
	}
	else
	{
		OutputDebugString( "FPSCRServer is not VAC Secure!\n" );
	}
	char rgch[128];
	sprintf_safe( rgch, "Game server SteamID: %llu\n", SteamGameServer()->GetSteamID().ConvertToUint64() );
	rgch[ sizeof(rgch) - 1 ] = 0;
	OutputDebugString( rgch );
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Called when we were previously logged into steam but get logged out
//-----------------------------------------------------------------------------
void CSteamServer::OnSteamServersDisconnected( SteamServersDisconnected_t *pLoggedOff )
{
	m_bConnectedToSteam = false;
	OutputDebugString( "FPSCRServer got logged out of Steam\n" );
}


//-----------------------------------------------------------------------------
// Purpose: Called when an attempt to login to Steam fails
//-----------------------------------------------------------------------------
void CSteamServer::OnSteamServersConnectFailure( SteamServerConnectFailure_t *pConnectFailure )
{
	m_bConnectedToSteam = false;
	OutputDebugString( "FPSCRServer failed to connect to Steam\n" );
}


//-----------------------------------------------------------------------------
// Purpose: Called once we are connected to Steam to tell it about our details
//-----------------------------------------------------------------------------
void CSteamServer::SendUpdatedServerDetailsToSteam()
{

	// Tell the Steam authentication servers about our game
	char rgchServerName[128];
	if ( Client() )
	{
		// If a client is running (should always be since we don't support a dedicated server)
		// then we'll form the name based off of it
		sprintf_safe( rgchServerName, "%s's game", Client()->GetLocalPlayerName() );
	}
	else
	{
		sprintf_safe( rgchServerName, "%s", "Game Guru" );
	}
	strcpy ( m_sServerName , rgchServerName );

	//
	// Set state variables, relevant to any master server updates or client pings
	//

	// These server state variables may be changed at any time.  Note that there is no lnoger a mechanism
	// to send the player count.  The player count is maintained by steam and you should use the player
	// creation/authentication functions to maintain your player count.
	SteamGameServer()->SetMaxPlayerCount( MAX_PLAYERS_PER_SERVER );
	SteamGameServer()->SetPasswordProtected( false );
	SteamGameServer()->SetServerName( m_sServerName );
	SteamGameServer()->SetBotPlayerCount( 0 ); // optional, defaults to zero
	SteamGameServer()->SetMapName( "Game Guru Map" );

#ifdef USE_GS_AUTH_API

	// Update all the players names/scores
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; i++ )
	{
		if ( m_rgClientData[i].m_bActive && m_rgpPlayer[i] )
		{
			SteamGameServer()->BUpdateUserData( m_rgClientData[i].m_SteamIDUser, m_rgpPlayer[i]->GetPlayerName(), m_rguPlayerScores[i] );
		}
	}
#endif

	// game type is a special string you can use for your game to differentiate different game play types occurring on the same maps
	// When users search for this parameter they do a sub-string search of this string 
	// (i.e if you report "abc" and a client requests "ab" they return your server)
	//SteamGameServer()->SetGameType( "dm" );

	// update any rule values we publish
	//SteamMasterServerUpdater()->SetKeyValue( "rule1_setting", "value" );
	//SteamMasterServerUpdater()->SetKeyValue( "rule2_setting", "value2" );
}


//-----------------------------------------------------------------------------
// Purpose: Tells us Steam3 (VAC and newer license checking) has accepted the user connection
//-----------------------------------------------------------------------------
void CSteamServer::OnValidateAuthTicketResponse( ValidateAuthTicketResponse_t *pResponse )
{
	if ( pResponse->m_eAuthSessionResponse == k_EAuthSessionResponseOK )
	{
		// This is the final approval, and means we should let the client play (find the pending auth by steamid)
		for ( uint32 i = 0; i<MAX_PLAYERS_PER_SERVER; ++i )
		{
			if ( !m_rgPendingClientData[i].m_bActive )
				continue;
			else if ( m_rgPendingClientData[i].m_SteamIDUser == pResponse->m_SteamID )
			{
#ifdef _DEBUG_LOG_
	log("SERVER-> CSteamServer::OnValidateAuthTicketResponse - Client authorised" , i );
#endif
				OutputDebugString( "Auth completed for a client\n" );
				OnAuthCompleted( true, i );
				return;
			}
		}
	}
	else
	{
		// Looks like we shouldn't let this user play, kick them
		for ( uint32 i = 0; i<MAX_PLAYERS_PER_SERVER; ++i )
		{
			if ( !m_rgPendingClientData[i].m_bActive )
				continue;
			else if ( m_rgPendingClientData[i].m_SteamIDUser == pResponse->m_SteamID )
			{
#ifdef _DEBUG_LOG_
	log("SERVER-> CSteamServer::OnValidateAuthTicketResponse - Client NOT authorised" , i );
#endif
				OutputDebugString( "Auth failed for a client\n" );
				OnAuthCompleted( false, i );
				return;
			}
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: Returns the SteamID of the game server
//-----------------------------------------------------------------------------
CSteamID CSteamServer::GetSteamID()
{
#ifdef USE_GS_AUTH_API
	return SteamGameServer()->GetSteamID();
#else
	// this is a placeholder steam id to use when not making use of Steam auth or matchmaking
	return k_steamIDNonSteamGS;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Kicks a player off the server
//-----------------------------------------------------------------------------
void CSteamServer::KickPlayerOffServer( CSteamID steamID )
{
	uint32 uPlayerCount = 0;
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		// If there is no player, skip
		if ( !m_rgClientData[i].m_bActive )
			continue;

		if ( m_rgClientData[i].m_SteamIDUser == steamID )
		{
			OutputDebugString( "Kicking player\n" );
			RemovePlayerFromServer( i );
			// send him a kick message
			MsgServerFailAuthentication_t msg;
			SteamGameServerNetworking()->SendP2PPacket( steamID, &msg, sizeof( msg ), k_EP2PSendReliable );
		}
		else
		{
			++uPlayerCount;
		}
	}
	m_uPlayerCount = uPlayerCount;
}

int CSteamServer::SteamIsServerRunning()
{
	if ( m_bConnectedToSteam ) return 1;
	return 0;
}

void CSteamServer::ResetTimeouts()
{
	for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; i++ )
	{
		// If there is no player, skip
		if ( !m_rgClientData[i].m_bActive )
			continue;

		m_rgClientData[i].m_ulTickCountLastData = (uint64)GetCounterPassedTotal();
	}
}


void CSteamServer::ServerCheckEveryoneIsLoadedAndReady()
{
	bool allReady = true;

	int howManyPlayers = 0;

	//Client attached to the server needs to be loaded and ready too!
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; i++ )
	{
		if ( m_rgpPlayer[i] )
		{
			howManyPlayers++;
			if ( serverClientsLoadedAndReady[i] == 0 )
			{
				allReady = false;
				break;
			}
		}
	}

	// the initial ServerCheckEveryoneIsLoadedAndReady check has a timeout so its okay if someone drops, it wont hang
	// we do this because people take time to join and if the check above purely goes by a player existing m_rgpPlayer, the first few checks
	// they wont, only the server will
	// by the second check (after sending files) we know everyone has joined the server, so its okay to change how many to reflect the current joined
	// because someone may have dropped out
	// so when we send send files amount, we change HowManyPlayersDoWeHave to 0, which we pick up here
	if ( HowManyPlayersDoWeHave == 0 || howManyPlayers > HowManyPlayersDoWeHave )
	{
		HowManyPlayersDoWeHave = howManyPlayers;
		ServerHowManyToStart = howManyPlayers;
	}

	// Make sure everyone is ready that initially joined the lobby
	// everyone is ready to play, send out the message to start the game proper
	if ( allReady && howManyPlayers == HowManyPlayersDoWeHave )
	{
		if ( GetCounterPassedTotal() - m_serverLastSendLoadedAndReady > 1500 )
		{

			m_serverLastSendLoadedAndReady = GetCounterPassedTotal();

			for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; i++ )
			{
				if ( m_rgpPlayer[i] )
				{

					MsgClientEveryoneLoadedAndReady_t* pmsg;
					pmsg = new MsgClientEveryoneLoadedAndReady_t();
					pmsg->logID = packetSendLogServerID;

					packetSendLogServer_t log;
					log.LogID = packetSendLogServerID++;
					log.packetType = k_EMsgClientEveryoneLoadedAndReady;
					log.pPacket = pmsg;
					log.timeStamp = GetCounterPassedTotal();
					log.playerID = i;

					PacketSend_Log_Server.push_back(log);

					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientEveryoneLoadedAndReady_t), k_EP2PSendUnreliable );
				}
			}
		}
	}
}

void CSteamServer::ServerCheckEveryoneIsReadyToPlay()
{
	bool allReady = true;

	//Client attached to the server needs to be loaded and ready too!
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; i++ )
	{
		if ( m_rgpPlayer[i] && serverClientsReadyToPlay[i] == 0 )
		{
			allReady = false;
			break;
		}
	}

	// everyone is ready to play, send out the message to start the game proper
	if ( allReady )
	{

		if ( GetCounterPassedTotal() - m_serverLastSendReadyToPlay > 1500 )
		{

			m_serverLastSendReadyToPlay = GetCounterPassedTotal();

			for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; i++ )
			{
				if ( m_rgpPlayer[i] )
				{

					MsgClientEveryoneReadyToPlay_t* pmsg;
					pmsg = new MsgClientEveryoneReadyToPlay_t();
					pmsg->logID = packetSendLogServerID;

					packetSendLogServer_t log;
					log.LogID = packetSendLogServerID++;
					log.packetType = k_EMsgClientEveryoneReadyToPlay;
					log.pPacket = pmsg;
					log.timeStamp = GetCounterPassedTotal();
					log.playerID = i;

					PacketSend_Log_Server.push_back(log);

					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientEveryoneReadyToPlay_t), k_EP2PSendUnreliable );
				}
			}
		}
	}
}

int CSteamServer::AvatarCheck( int check )
{
	int result = 1;
	bool sendMsg = false;
	MsgServerAvatarChangeMode_t msg;

	switch ( check )
	{
		case SYNC_AVATAR_TEX_BEGIN:
		{
			int amount = 0;
			ServerAvatarfilesReceived = 0;

			for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; i++ )
			{
				if ( m_rgpPlayer[i] && serverClientsHaveAvatarTexture[i] == 0 )
				{
					result = 0;
					break;
				}

				// check for avatar texture
				if ( m_rgpPlayer[i] && serverClientsHaveAvatarTexture[i] == 1 )
				{
					amount++;
				}
			}

			if ( result == 1 )
			{
				sendMsg = true;

				if ( amount == 0 )
					msg.mode = SYNC_AVATAR_TEX_MODE_DONE;
				else
					msg.mode = SYNC_AVATAR_TEX_MODE_SENDING;

				// work out how many textures to send

				msg.result = amount;
			}

		} break;
		case SYNC_AVATAR_TEX_MODE_SENDING:
		{
			int amount = 0;

			for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; i++ )
			{
				// check for avatar texture
				if ( m_rgpPlayer[i] && serverClientsHaveAvatarTexture[i] == 1 )
				{
					amount++;
				}
			}

			if ( ServerAvatarfilesReceived < amount ) result = 0;

			if ( result == 1 )
			{
				sendMsg = true;

				msg.mode = SYNC_AVATAR_TEX_MODE_DONE;
			}

		} break;
	}

	// Check if we should send out a mode change
	if ( sendMsg == true )
	{
		for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; i++ )
		{
			if ( m_rgpPlayer[i] )
			{				
				SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgServerAvatarChangeMode_t), k_EP2PSendReliable );
			}
		}
	}

	return result;
}

void CSteamServer::ServerEndGame( int index )
{

	MsgEndGame_t msg;

	for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] && i != index )
			SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgEndGame_t), k_EP2PSendReliable );
	}
}

void CSteamServer::CheckReceipts()
{

	/*char s[256];
	sprintf ( s, "Server Outstanding:%d" , PacketSend_Log_Server.size() );
	Print ( s );*/

	double timeNow = GetCounterPassedTotal();

	for ( unsigned int c = 0; c < PacketSend_Log_Server.size() ; c++ )
	{
		if ( timeNow - PacketSend_Log_Server[c].timeStamp > 1000 )
		{
			PacketSend_Log_Server[c].timeStamp = timeNow;
			// send again
			int index = PacketSend_Log_Server[c].playerID;
			switch ( PacketSend_Log_Server[c].packetType )
			{
			case k_EMsgClientEveryoneReadyToPlay:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgClientEveryoneReadyToPlay_t), k_EP2PSendUnreliable ); break;
			case k_EMsgClientEveryoneLoadedAndReady:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgClientEveryoneLoadedAndReady_t), k_EP2PSendUnreliable ); break;
			case k_EMsgClientKilledBy:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgClientKilledBy_t), k_EP2PSendUnreliable ); break;
			case k_EMsgClientPlayerApplyDamage:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgClientPlayerApplyDamage_t), k_EP2PSendUnreliable ); break;
			case k_EMsgServerLua:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgServerLua_t), k_EP2PSendUnreliable ); break;
			case k_EMsgServerLuaString:
				if ( m_rgpPlayer[index] )
					SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[index].m_SteamIDUser, PacketSend_Log_Server[c].pPacket, sizeof(MsgServerLuaString_t), k_EP2PSendUnreliable ); break;
			}

		}
	}
}

void CSteamServer::GotReceipt( int c )
{
	int found = -1;
	for ( unsigned int i = 0; i < PacketSend_Log_Server.size() ; i++ )
	{
		if ( PacketSend_Log_Server[i].LogID == c ) 
		{
			found = i;
			break;
		}
	}

	if ( found > -1 )
	{
		switch ( PacketSend_Log_Server[found].packetType )
		{
		case k_EMsgClientEveryoneReadyToPlay:
			delete (MsgClientEveryoneReadyToPlay_t*)PacketSend_Log_Server[found].pPacket; break;
		case k_EMsgClientEveryoneLoadedAndReady:
			delete (MsgClientEveryoneLoadedAndReady_t*)PacketSend_Log_Server[found].pPacket; break;
		case k_EMsgClientKilledBy:
			delete (MsgClientKilledBy_t*)PacketSend_Log_Server[found].pPacket; break;
		case k_EMsgClientPlayerApplyDamage:
			delete (MsgClientPlayerApplyDamage_t*)PacketSend_Log_Server[found].pPacket; break;
		case k_EMsgServerLua:
			delete (MsgServerLua_t*)PacketSend_Log_Server[found].pPacket; break;
		case k_EMsgServerLuaString:
			delete (MsgServerLuaString_t*)PacketSend_Log_Server[found].pPacket; break;
		}

		PacketSend_Log_Server.erase(PacketSend_Log_Server.begin()+found);
	}
}

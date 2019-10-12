#ifndef SERVER_H
#define SERVER_H

// Includes
#include "PhotonMultiplayer.h"
#include "CPlayer.h"

// Forward declaration
class CPlayerClient;

struct ClientConnectionData_t
{
	bool m_bActive;					// Is this slot in use? Or is it available for new connections?
	CSteamID m_SteamIDUser;			// What is the steamid of the player?
	uint64 m_ulTickCountLastData;	// What was the last time we got data from the player?
};

class CServer
{
public:
	CServer( );
	~CServer();

	int IsServerRunning();
	bool IsConnectedToSteam()		{ return m_bConnectedToSteam; }

	void RunFrame();
	void SetGameState( EServerGameState eState );

	/*
	bool ShouldISend ( int p , int id, float thisx, float thisy, float thisz, int playerID );

	void ServerCheckEveryoneIsLoadedAndReady();
	void ServerCheckEveryoneIsReadyToPlay();

	double m_serverLastSendLoadedAndReady;
	double m_serverLastSendReadyToPlay;
	double m_serverLastSendReadyToStart;

	// Run a game frame
	void RunFrame();

	// Set game state
	void SetGameState( EServerGameState eState );
	*/

	// Checks for any incoming network data, then dispatches it
	//void ReceiveNetworkData ( EMessage eMsg, char* pchRecvBuf, uint32 cubMsgSize );

	/*
	// Reset player scores (occurs when starting a new game)
	void ResetScores();

	// Reset player positions (occurs in between rounds as well as at the start of a new game)
	void ResetPlayers();

	// Checks various game objects for collisions and updates state appropriately if they have occurred
	void CheckForCollisions();

	void ResetTimeouts();

	// Kicks a given player off the server
	void KickPlayerOffServer( CSteamID steamID );

	void ServerEndGame( int index );

	// data accessors
	CSteamID GetSteamID();

	int AvatarCheck( int check );
	*/

private:

	bool m_bConnectedToSteam;

	/*
	// Tells us when we have successfully connected to Steam
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnSteamServersConnected, SteamServersConnected_t, m_CallbackSteamServersConnected );

	// Tells us when there was a failure to connect to Steam
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnSteamServersConnectFailure, SteamServerConnectFailure_t, m_CallbackSteamServersConnectFailure );

	// Tells us when we have been logged out of Steam
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnSteamServersDisconnected, SteamServersDisconnected_t, m_CallbackSteamServersDisconnected );

	// Tells us that Steam has set our security policy (VAC on or off)
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnPolicyResponse, GSPolicyResponse_t, m_CallbackPolicyResponse );

	//
	// Various callback functions that Steam will call to let us know about whether we should
	// allow clients to play or we should kick/deny them.
	//

	// Tells us a client has been authenticated and approved to play by Steam (passes auth, license check, VAC status, etc...)
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t, m_CallbackGSAuthTicketResponse );

	// client connection state
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnP2PSessionRequest, P2PSessionRequest_t, m_CallbackP2PSessionRequest );
	STEAM_GAMESERVER_CALLBACK( CSteamServer, OnP2PSessionConnectFail, P2PSessionConnectFail_t, m_CallbackP2PSessionConnectFail );

	// Function to tell Steam about our servers details
	void SendUpdatedServerDetailsToSteam();

	// Receive updates from client
	void OnReceiveClientUpdateData( uint32 uIndex, ClientUpdateData_t *pUpdateData );

	// Send data to a client at the given ship index
	bool BSendDataToClient( uint32 uShipIndex, char *pData, uint32 nSizeOfData );

	// Send data to a client at the given pending index
	bool BSendDataToPendingClient( uint32 uShipIndex, char *pData, uint32 nSizeOfData );

	// Connect a client, will send a success/failure response to the client
	void OnClientBeginAuthentication( CSteamID steamIDClient, void *pToken, uint32 uTokenLen );

	// Handles authentication completing for a client
	void OnAuthCompleted( bool bAuthSuccess, uint32 iPendingAuthIndex );

	// Adds/initializes a new player at the given position
	void AddPlayer( uint32 uPosition );

	// Removes a player from the server
	void RemovePlayerFromServer( uint32 uShipPosition );

	// Send world update to all clients
	void SendUpdateDataToAllClients();

	// Ships for players, doubles as a way to check for open slots (pointer is NULL meaning open)
	CPlayer *m_rgpPlayer[MAX_PLAYERS_PER_SERVER];

	// Player scores
	uint32 m_rguPlayerScores[MAX_PLAYERS_PER_SERVER];

	// server name
	char m_sServerName[256];

	// Who just won the game? Should be set if we go into the k_EGameWinner state
	uint32 m_uPlayerWhoWonGame;
	*/

	// Last time state changed
	uint64 m_ulStateTransitionTime;

	// Last time we sent clients an update
	uint64 m_ulLastServerUpdateTick;

	// Number of players currently connected, updated each frame
	uint32 m_uPlayerCount;

	// Current game state
	EServerGameState m_eGameState;

	/*
	// Vector to keep track of client connections
	ClientConnectionData_t m_rgClientData[MAX_PLAYERS_PER_SERVER];

	// Vector to keep track of client connections which are pending auth
	ClientConnectionData_t m_rgPendingClientData[MAX_PLAYERS_PER_SERVER];

	void CheckReceipts();
	void GotReceipt( int c );
	*/
};

extern CServer *g_pServer;
CServer *Server();

#endif // SERVER_H

#ifndef Client_H
#define Client_H

// Includes
#include "CPlayer.h"
#include "PhotonMultiplayer.h"
#include "Messages.h"
#include "LoadBalancingListener.h"

// Forward class declaration
class CServer;
//class CVoiceChat;

// Height of the HUD font
#define HUD_FONT_HEIGHT 18

// Height for the instructions font
#define INSTRUCTIONS_FONT_HEIGHT 24

// Enum for various client connection states
enum EClientConnectionState
{
	k_EClientNotConnected,							// Initial state, not connected to a server
	k_EClientConnectedPendingAuthentication,		// We've established communication with the server, but it hasn't authed us yet
	k_EClientConnectedAndAuthenticated,				// Final phase, server has authed us, we are actually able to play on it
};

class CClient 
{
public:
	//Constructor
	CClient( void );
	~CClient();
	void Init();// LoadBalancingListener* mpLbl );

	//LoadBalancingListener* m_pLbl;

	void StartServer();
	int IsServerRunning();
	int IsGameRunning();
	int GetMyPlayerIndex();
	
	/*
	void SteamCreateLobby();
	int SteamIsLobbyCreated();
	void SteamGetLobbyList();
	int SteamIsLobbyListCreated();
	int SteamGetLobbyListSize();
	LPSTR SteamGetLobbyListName(int index);
	int SteamGetLobbyListID(int index);
	void SteamStartServer();
	int SteamIsServerRunning();
	void SteamJoinLobby ( int index );
	int SteamHasJoinedLobby();
	int SteamGetUsersInLobbyCount();
	int SteamIsGameRunning();
	void SteamSetPlayerPositionX( float _x);
	void SteamSetPlayerPositionY( float _y );
	void SteamSetPlayerPositionZ( float _z );
	void SteamSetPlayerAngle( float _angle );
	float SteamGetPlayerPositionX ( int index );
	float SteamGetPlayerPositionY ( int index );
	float SteamGetPlayerPositionZ ( int index );
	float SteamGetPlayerAngle ( int index );
	void SteamSetPlayerScore ( int index, int score );
	int SteamGetPlayerScore ( int score );
	void SteamSetBullet ( int index , float x , float y , float z, float anglex, float angley, float anglez, int type, int on );
	int SteamGetBulletOn ( int index );
	int SteamGetBulletType ( int index );
	float SteamGetBulletX ( int index );
	float SteamGetBulletY ( int index );
	float SteamGetBulletZ ( int index );
	float SteamGetBulletAngleX ( int index );
	float SteamGetBulletAngleY ( int index );
	float SteamGetBulletAngleZ ( int index );
	void SteamSetKeyState ( int key , int state );
	void SteamApplyPlayerDamage ( int index, int damage, int x, int y, int z, int force, int limb );
	int SteamGetClientServerConnectionStatus();
	void SteamSetPlayerAlive ( int state );
	int IsServer();
	void CheckIfIShouldTakeOverServer();
	void SteamSpawnObject ( int obj, int source, float x, float y, float z );
	void SteamDeleteObject ( int obj );
	void SteamDestroyObject ( int obj );
	void SteamKilledBy ( int killedBy, int x, int y, int z, int force, int limb );
	void SteamKilledSelf ();
	void SteamSetSendFileCount(int count);
	void SteamSendFileBegin ( int index , LPSTR pString );
	int SteamSendFileDone();
	int SteamIsEveryoneFileSynced();
	void SteamSendIAmLoadedAndReady();
	void SteamSendIAmReadyToPlay();
	void SteamShoot();
	int SteamGetFileProgress();
	void SteamSetPlayerAppearance( int a);
	int SteamGetPlayerAppearance( int index );
	void SteamSetCollision ( int index, int state );
	void SteamPlayAnimation ( int index, int start, int end, int speed );
	void SteamSendLua ( int code, int e, int v );
	void SteamSendLuaString ( int code, int e, LPSTR s );
	void ServerCheckEveryoneIsLoadedAndReady();
	void ServerCheckEveryoneIsReadyToPlay();
	void SteamLeaveLobby();
	void CheckReceipts();
	void GotReceipt(int c);
	void SteamSendChat( LPSTR chat );
	void SteamEndGame();
	void AvatarSendWeHaveHeadTextureToServer(int flag);
	void SteamSendAvatarFileClient ( int index , LPSTR pString );
	*/

	// Run a game frame
	void RunFrame();

	// Checks for any incoming network data, then dispatches it
	void ReceiveNetworkData ( EMessage eMsg, char* pchRecvBuf, uint32 cubMsgSize );

	/*
	// Send data to a client at the given ship index
	bool BSendServerData( const void *pData, uint32 nSizeOfData );

	// Menu callback handler (handles a bunch of menus that just change state with no extra data)
	void OnMenuSelection( EClientGameState eState ) { SetGameState( eState ); }

	void OnMenuSelection( ERemoteStorageSyncMenuCommand selection );
	*/

	// Set game state
	void SetGameState( EClientGameState eState );
	EClientGameState GetGameState() { return m_eGameState; }

	/*
	// set failure text
	void SetConnectionFailureText( const char *pchErrorText );

	// Were we the winner?
	bool BLocalPlayerWonLastGame();

	// Get the steam id for the local user at this client
	CSteamID GetLocalSteamID() { return m_SteamIDLocalUser; }

	// Get the local players name
	const char* GetLocalPlayerName() 
	{ 
		return SteamFriends()->GetFriendPersonaName( m_SteamIDLocalUser ); 
	}

	// Scale screen size to "real" size
	float PixelsToFeet( float flPixels );
	*/

	double timeElapsed;
	bool m_gotPlayerInfoFromServer;

private:

	// Our player position in the array below
	uint32 m_uPlayerIndex;

	/*
	// Receive a response from the server for a connection attempt
	void OnReceiveServerInfo( CSteamID steamIDGameServer, bool bVACSecure, const char *pchServerName );

	// Receive a response from the server for a connection attempt
	void OnReceiveServerAuthenticationResponse( bool bSuccess, uint32 uPlayerPosition );
	*/

	// Receive a state update from the server
	void OnReceiveServerUpdate( ServerUpdateData_t *pUpdateData );

	/*
	// Handle the server exiting
	void OnReceiveServerExiting();

	// Disconnects from a server (telling it so) if we are connected
	void DisconnectFromServer();

	// game state changes
	void OnGameStateChanged( EClientGameState eGameStateNew );

	// Draw the HUD text (should do this after drawing all the objects)
	void DrawHUDText();

	// Draw instructions for how to play the game
	void DrawInstructions();

	// Draw text telling the players who won (or that their was a draw)
	void DrawWinnerDrawOrWaitingText();

	// Draw text telling the user that the connection attempt has failed
	void DrawConnectionFailureText();

	// Draw connect to server text
	void DrawConnectToServerText();

	// Draw text telling the user a connection attempt is in progress
	void DrawConnectionAttemptText();

	// Updates what we show to friends about what we're doing and how to connect
	void UpdateRichPresenceConnectionInfo();

	// SteamID for the local user on this client
	CSteamID m_SteamIDLocalUser;

	// List of steamIDs for each player
	CSteamID m_rgSteamIDPlayers[MAX_PLAYERS_PER_SERVER];
	*/

	//Players, doubles as a way to check for open slots (pointer is NULL meaning open)
	public:

	CServer *m_pServer;

	EClientGameState m_eGameState;

	/*
	// Server we are connected to
	CSteamServer *m_pServer;
	CPlayer *m_rgpPlayer[MAX_PLAYERS_PER_SERVER];
	char	m_rgpPlayerName[MAX_PLAYERS_PER_SERVER][128];
	private:
	bool	m_bSentPlayerName;

	// Player scores
	uint32 m_rguPlayerScores[MAX_PLAYERS_PER_SERVER];

	// Who just won the game? Should be set if we go into the k_EGameWinner state
	uint32 m_uPlayerWhoWonGame;

	// true if we only just transitioned state
	bool m_bTransitionedGameState;

	// Time the last state transition occurred (so we can count-down round restarts)
	uint64 m_ulStateTransitionTime;

	// Time we started our last connection attempt
	uint64 m_ulLastConnectionAttemptRetryTime;

	// Time we last got data from the server
	uint64 m_ulLastNetworkDataReceivedTime;

	// Time when we sent our ping
	uint64 m_ulPingSentTime;

	// Text to display if we are in an error state
	char m_rgchErrorText[256];

	// keep track of if we opened the overlay for a gamewebcallback
	bool m_bSentWebOpen;

	// simple class to marshal callbacks from pinging a game server
	class CGameServerPing : public ISteamMatchmakingPingResponse
	{
	public:
		CGameServerPing()
		{
			m_hGameServerQuery = HSERVERQUERY_INVALID;
			m_pSpaceWarsClient = NULL;
		}

		void RetrieveSteamIDFromGameServer( CClient *pClient, uint32 unIP, uint16 unPort )
		{
			m_pSpaceWarsClient = pClient;
			m_hGameServerQuery = SteamMatchmakingServers()->PingServer( unIP, unPort, this );
		}

		void CancelPing()
		{
			m_hGameServerQuery = HSERVERQUERY_INVALID;
		}

		// Server has responded successfully and has updated data
		virtual void ServerResponded( gameserveritem_t &server )
		{
			if ( m_hGameServerQuery != HSERVERQUERY_INVALID && server.m_steamID.IsValid() )
			{
				m_pSpaceWarsClient->InitiateServerConnection( server.m_steamID );
			}

			m_hGameServerQuery = HSERVERQUERY_INVALID;
		}

		// Server failed to respond to the ping request
		virtual void ServerFailedToRespond()
		{
			m_hGameServerQuery = HSERVERQUERY_INVALID;
		}
	*/

	/*
	private:
		HServerQuery m_hGameServerQuery;	// we're ping a game server, so we can convert IP:Port to a steamID
		CClient *m_pSpaceWarsClient;
	};
	CGameServerPing m_GameServerPing;
	*/

	// Track whether we are connected to a server (and what specific state that connection is in)
	EClientConnectionState m_eConnectedStatus;

	bool m_bLastControllerStateInMenu;

public:

	// voice chat 
	//CVoiceChat *m_pVoiceChat;
};

// Must define this stuff before BaseMenu.h as it depends on calling back into us through these accessors
extern CClient *g_pClient;
CClient *Client();

#endif // Client_H

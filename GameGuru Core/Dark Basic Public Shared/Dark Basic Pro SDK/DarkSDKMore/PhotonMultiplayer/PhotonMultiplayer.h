//========= Copyright © 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Shared definitions for the communication between the server/client
//
// $NoKeywords: $
//=============================================================================

#ifndef STEAMMULTIPLAYER_H
#define STEAMMULTIPLAYER_H

#include "stdafx.h"
#include <vector>

void Print (char* c);

// Current game server version
#define STEAM_SERVER_VERSION "1.0.0.0"

// UDP port for the spacewar server to do authentication on (ie, talk to Steam on)
#define STEAM_AUTHENTICATION_PORT 8766

// UDP port for the spacewar server to listen on
#define STEAM_SERVER_PORT 27015

// UDP port for the master server updater to listen on
#define STEAM_MASTER_SERVER_UPDATER_PORT 27016

// How long to wait for a response from the server before resending our connection attempt
#define SERVER_CONNECTION_RETRY_MILLISECONDS 350

// How long to wait for a client to send an update before we drop its connection server side
#define SERVER_TIMEOUT_MILLISECONDS 10*1000
#define SERVER_TIMEOUT_MILLISECONDS_LONG 60*1000
extern uint64 server_timeout_milliseconds;

// Maximum packet size in bytes
#define MAX_STEAM_PACKET_SIZE 1024*512

// Maximum number of players who can join a server and play simultaneously
#define MAX_PLAYERS_PER_SERVER 8

// Time to pause wait after a round ends before starting a new one
#define MILLISECONDS_BETWEEN_ROUNDS 4000

// Time to timeout a connection attempt in
#define MILLISECONDS_CONNECTION_TIMEOUT 60*1000
//#define MILLISECONDS_CONNECTION_TIMEOUT 3000000000

// How many times a second does the server send world updates to clients
#define SERVER_UPDATE_SEND_RATE 15

// How many times a second do we send our updated client state to the server
#define CLIENT_UPDATE_SEND_RATE 15

// How fast does the server internally run at?
#define MAX_CLIENT_AND_SERVER_FPS 86

#define FILE_CHUNK_SIZE 1024*8

#define INTERPOLATE_SMOOTHING 0.2f
#define INTERPOLATE_SMOOTHING_MIN 0.25f
#define INTERPOLATE_SMOOTHING_TURN 0.1f

float CosineInterpolate( float y1,float y2, float mu);
float CosineInterpolateAngle( float y1,float y2, float mu);
float CosineInterpolateAngle2( float y1,float y2, float mu);

extern int IsWorkshopLoadingOn;
extern CSteamID lobbyIAmInID;
extern int scores[MAX_PLAYERS_PER_SERVER];
extern int keystate[MAX_PLAYERS_PER_SERVER][256];
extern int alive[MAX_PLAYERS_PER_SERVER];
extern int playerDamage;
extern int damageSource;
extern int damageX;
extern int damageY;
extern int damageZ;
extern int damageForce;
extern int damageLimb;
extern int killedSource[MAX_PLAYERS_PER_SERVER];
extern int killedX[MAX_PLAYERS_PER_SERVER];
extern int killedY[MAX_PLAYERS_PER_SERVER];
extern int killedZ[MAX_PLAYERS_PER_SERVER];
extern int killedForce[MAX_PLAYERS_PER_SERVER];
extern int killedLimb[MAX_PLAYERS_PER_SERVER];
extern CSteamID playerSteamIDs[MAX_PLAYERS_PER_SERVER];
extern bool isPlayingOnAServer;
extern int serverClientsFileSynced[MAX_PLAYERS_PER_SERVER];
extern int serverClientsLoadedAndReady[MAX_PLAYERS_PER_SERVER];
extern int serverClientsReadyToPlay[MAX_PLAYERS_PER_SERVER];
extern int playerAppearance[MAX_PLAYERS_PER_SERVER];
extern int playerShoot[MAX_PLAYERS_PER_SERVER];
extern int tweening[MAX_PLAYERS_PER_SERVER];
extern int serverClientsHaveAvatarTexture[MAX_PLAYERS_PER_SERVER];
extern int HowManyPlayersDoWeHave;

enum SYNCED_AVATAR_TEXTURE_MODES { SYNC_AVATAR_TEX_BEGIN , SYNC_AVATAR_TEX_SENT_IF_I_HAVE_TEXTURE , SYNC_AVATAR_TEX_MODE_SENDING, SYNC_AVATAR_TEX_MODE_SENDING_WAITING, SYNC_AVATAR_TEX_MODE_RECEIVING, SYNC_AVATAR_TEX_MODE_DONE };

extern int syncedAvatarTextureMode;
extern int syncedAvatarTextureModeServer;
extern char myAvatarTextureName[MAX_PATH];
extern int syncedAvatarHowManyTextures;
extern int syncedAvatarHowManyTexturesReceived;

extern int ServerHowManyToStart;
extern int ServerHowManyJoined;
extern int ServerSaysItIsOkayToStart;
extern int ServerHaveIToldClientsToStart;
//
extern int ServerFilesToReceive;
extern int ServerFilesReceived;
extern int IamSyncedWithServerFiles;
extern int serverOnlySendMapToSpecificPlayer;
extern int IamLoadedAndReady;
extern int IamReadyToPlay;
extern int isEveryoneLoadedAndReady;
extern int isEveryoneReadyToPlay;
extern int serverHowManyFileChunks;
extern int serverChunkToSendCount;
extern int serverFileFileSize;
extern int fileProgress;
//extern FILE* serverFile;
extern int voiceChatOn;
extern char hostsLobbyName[256];
//
extern uint64 ServerCreationTime;
extern bool needToSendMyName;
extern int SteamOverlayActive;
extern int ServerIsShuttingDown;

extern char steamRootPath[MAX_PATH];
extern FILE* avatarFile[MAX_PLAYERS_PER_SERVER];
extern int avatarHowManyFileChunks[MAX_PLAYERS_PER_SERVER];
extern int avatarFileFileSize[MAX_PLAYERS_PER_SERVER] ;

struct tbullet
{
	float x;
	float y;
	float z;
	float newx;
	float newy;
	float newz;
	float anglex;
	float angley;
	float anglez;
	int type;
	int on;
	double onTime;
};

struct tSpawn
{
	int object;
	int source;
	float x;
	float y;
	float z;
};

#define CURRENT_LUA_STRING_SIZE 300

struct tLua
{
	int code;
	int e;
	int v;
	char s[CURRENT_LUA_STRING_SIZE];
};

struct tMessage
{
	char message[256];
};

struct tChat
{
	char msg[82];
};

struct tCollision
{
	int index;
	int state;
};

struct tAnimation
{
	int index;
	int start;
	int end;
	int speed;
};

extern std::vector <tSpawn> spawnList;
extern std::vector <tLua> luaList;
extern std::vector <int> deleteList;
extern std::vector <int> deleteListSource;
extern std::vector <int> destroyList;
extern std::vector <tMessage> messageList;
extern std::vector <tCollision> collisionList;
extern std::vector <tAnimation> animationList;
extern std::vector <tChat> chatList;
extern std::vector <uint32> lobbyChatIDs;

extern tbullet bullets[180];
extern bool bulletSeen[180][MAX_PLAYERS_PER_SERVER];

extern int ClientDeathNumber;
extern int ServerClientLastDeathNumber[MAX_PLAYERS_PER_SERVER];

template <typename T>
inline T WordSwap( T w )
{
	uint16 temp;

	temp  = ((*((uint16 *)&w) & 0xff00) >> 8);
	temp |= ((*((uint16 *)&w) & 0x00ff) << 8);

	return *((T*)&temp);
}

template <typename T>
inline T DWordSwap( T dw )
{
	uint32 temp;

	temp  =   *((uint32 *)&dw) 				>> 24;
	temp |= ((*((uint32 *)&dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32 *)&dw) & 0x0000FF00) << 8);
	temp |= ((*((uint32 *)&dw) & 0x000000FF) << 24);

	return *((T*)&temp);
}

template <typename T>
inline T QWordSwap( T dw )
{
	uint64 temp;

	temp  =   *((uint64 *)&dw) 				         >> 56;
	temp |= ((*((uint64 *)&dw) & 0x00FF000000000000ull) >> 40);
	temp |= ((*((uint64 *)&dw) & 0x0000FF0000000000ull) >> 24);
	temp |= ((*((uint64 *)&dw) & 0x000000FF00000000ull) >> 8);
	temp |= ((*((uint64 *)&dw) & 0x00000000FF000000ull) << 8);
	temp |= ((*((uint64 *)&dw) & 0x0000000000FF0000ull) << 24);
	temp |= ((*((uint64 *)&dw) & 0x000000000000FF00ull) << 40);
	temp |= ((*((uint64 *)&dw) & 0x00000000000000FFull) << 56);

	return *((T*)&temp);
}

#define LittleInt16( val )	( val )
#define LittleWord( val )	( val )
#define LittleInt32( val )	( val )
#define LittleDWord( val )	( val )
#define LittleQWord( val )	( val )
#define LittleFloat( val )	( val )

// Leaderboard names
#define LEADERBOARD_QUICKEST_WIN "Quickest Win"
#define LEADERBOARD_FEET_TRAVELED "Feet Traveled"

// Enum for possible game states on the client
enum EClientGameState
{
	k_EClientGameStartServer,
	k_EClientGameActive,
	k_EClientGameWaitingForPlayers,
	k_EClientGameMenu,
	k_EClientGameQuitMenu,
	k_EClientGameExiting,
	k_EClientGameInstructions,
	k_EClientGameDraw,
	k_EClientGameWinner,
	k_EClientGameConnecting,
	k_EClientGameConnectionFailure,
	k_EClientFindInternetServers,
	k_EClientStatsAchievements,
	k_EClientCreatingLobby,
	k_EClientInLobby,
	k_EClientFindLobby,
	k_EClientJoiningLobby,
	k_EClientFindLANServers,
	k_EClientRemoteStorage,
	k_EClientLeaderboards,
	k_EClientMinidump,
	k_EClientConnectingToSteam,
	k_EClientLinkSteamAccount,
	k_EClientAutoCreateAccount,
	k_EClientRetrySteamConnection,
	k_EClientClanChatRoom,
	k_EClientWebCallback,
	k_EClientMusic,
	k_EClientWorkshop,
};


// Enum for possible game states on the server
enum EServerGameState
{
	k_EServerWaitingForPlayers,
	k_EServerActive,
	k_EServerDraw,
	k_EServerWinner,
	k_EServerExiting,
};

#pragma pack( push, 1 )

// Data sent per photon beam from the server to update clients photon beam positions
struct ServerPhotonBeamUpdateData_t
{
	void SetActive( bool bIsActive ) { m_bIsActive = bIsActive; }
	bool GetActive() { return m_bIsActive; }

	void SetRotation( float flRotation ) { m_flCurrentRotation = LittleFloat( flRotation ); }
	float GetRotation() { return LittleFloat( m_flCurrentRotation ); }

	void SetXVelocity( float flVelocity ) { m_flXVelocity = LittleFloat( flVelocity ); }
	float GetXVelocity() { return LittleFloat( m_flXVelocity ); }

	void SetYVelocity( float flVelocity ) { m_flYVelocity = LittleFloat( flVelocity ); }
	float GetYVelocity() { return LittleFloat( m_flYVelocity ); }

	void SetXPosition( float flPosition ) { m_flXPosition = LittleFloat( flPosition ); }
	float GetXPosition() { return LittleFloat( m_flXPosition ); }

	void SetYPosition( float flPosition ) { m_flYPosition = LittleFloat( flPosition ); }
	float GetYPosition() { return LittleFloat( m_flYPosition ); }


private:
	// Does the photon beam exist right now?
	bool m_bIsActive; 

	// The current rotation 
	float m_flCurrentRotation;

	// The current velocity
	float m_flXVelocity;
	float m_flYVelocity;

	// The current position
	float m_flXPosition;
	float m_flYPosition;
};


// This is the data that gets sent per player in each update
struct ServerPlayerUpdateData_t
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
	unsigned short angle;
};

// This is the data that gets sent from the server to each client for each update
struct ServerUpdateData_t
{
	void SetServerGameState( EServerGameState eState ) { m_eCurrentGameState = LittleDWord( (unsigned char)eState ); }
	EServerGameState GetServerGameState() { return (EServerGameState)LittleDWord( (uint32)m_eCurrentGameState ); }
	
	void SetPlayerActive( uint32 iIndex, bool bIsActive ) { m_rgPlayersActive[iIndex] = bIsActive; }
	bool GetPlayerActive( uint32 iIndex ) { return m_rgPlayersActive[iIndex]; }

	void SetPlayerSteamID( uint32 iIndex, uint64 ulSteamID ) { m_rgPlayerSteamIDs[iIndex] = LittleQWord(ulSteamID); }
	uint64 GetPlayerSteamID( uint32 iIndex ) { return LittleQWord(m_rgPlayerSteamIDs[iIndex]); }

	ServerPlayerUpdateData_t *AccessPlayerUpdateData( uint32 iIndex ) { return &m_rgShipData[iIndex];}

private:
	// What state the game is in
	unsigned char m_eCurrentGameState;
	
	// which player slots are in use
	bool m_rgPlayersActive[MAX_PLAYERS_PER_SERVER];
	
	// array of ship data
	ServerPlayerUpdateData_t m_rgShipData[MAX_PLAYERS_PER_SERVER];

	// array of players steamids for each slot, serialized to uint64
	uint64 m_rgPlayerSteamIDs[MAX_PLAYERS_PER_SERVER];
};

// This is the data that gets sent from each client to the server for each update
struct ClientUpdateData_t
{
	unsigned short x;
	unsigned short y;
	unsigned short z;
	unsigned short angle;
};

extern int packetSendLogClientID;
extern int packetSendLogServerID;

struct packetSendLogClient_t
{
	int LogID;
	int packetType;
	void* pPacket;
	double timeStamp;
};

struct packetSendLogServer_t
{
	int LogID;
	int packetType;
	void* pPacket;
	int playerID;
	double timeStamp;
};

struct packetSendReceiptLogClient_t
{
	int LogID;
	double timeStamp;
};

struct packetSendReceiptLogServer_t
{
	int LogID;
	int playerID;
	double timeStamp;
};

extern std::vector <packetSendLogClient_t> PacketSend_Log_Client;
extern std::vector <packetSendLogServer_t> PacketSend_Log_Server;
extern std::vector <packetSendReceiptLogClient_t> PacketSendReceipt_Log_Client;
extern std::vector <packetSendReceiptLogServer_t> PacketSendReceipt_Log_Server;

//=====================================

#pragma pack( pop )

void SteamInitClient();

void SteamResetClient();
void SteamCleanupClient();

void SteamAvatarClient();
void SteamAvatarServer();

#ifdef _DEBUG_LOG_
void logStart();
void log( char* s);
void log( char* s, int a);
void log( char* s, int a, int b);
void log( char* s, int a, int b, int c);
void logEnd();
#endif

void ResetGameStats();

#endif // STEAMMULTIPLAYER_H
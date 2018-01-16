//========= Copyright © 1996-2008, Valve LLC, All rights reserved. ============
//
// Purpose: Defines the wire protocol for the game
//
// $NoKeywords: $
//=============================================================================

#ifndef MESSAGES_H
#define MESSAGES_H

#include "stdafx.h"
#include "SteamMultiplayer.h"

#include <map>

#pragma pack( push, 1 )

// Network message types
enum EMessage
{
	// Server messages
	k_EMsgServerBegin = 0,
	k_EMsgServerSendInfo = k_EMsgServerBegin+1,
	k_EMsgServerFailAuthentication = k_EMsgServerBegin+2,
	k_EMsgServerPassAuthentication = k_EMsgServerBegin+3,
	k_EMsgServerUpdateWorld = k_EMsgServerBegin+4,
	k_EMsgServerExiting = k_EMsgServerBegin+5,
	k_EMsgServerPingResponse = k_EMsgServerBegin+6,
	k_EMsgServerLua = k_EMsgServerBegin+7,
	k_EMsgServerLuaString = k_EMsgServerBegin+8,
	k_EMsgServerAvatarChangeMode = k_EMsgServerBegin+9,

	// Client messages
	k_EMsgClientBegin = 500,
	k_EMsgClientInitiateConnection = k_EMsgClientBegin+1,
	k_EMsgClientBeginAuthentication = k_EMsgClientBegin+2,
	k_EMsgClientSendLocalUpdate = k_EMsgClientBegin+3,
	k_EMsgClientLeavingServer = k_EMsgClientBegin+4,
	k_EMsgClientPing = k_EMsgClientBegin+5,
	k_EMsgClientPlayerName = k_EMsgClientBegin+6,
	k_EMsgClientPlayerScore = k_EMsgClientBegin+7,
	k_EMsgClientPlayerBullet = k_EMsgClientBegin+8,
	k_EMsgClientPlayerKeyState = k_EMsgClientBegin+9,
	k_EMsgClientPlayerApplyDamage = k_EMsgClientBegin+10,
	k_EMsgClientPlayerSetPlayerAlive = k_EMsgClientBegin+11,
	k_EMsgClientSpawnObject = k_EMsgClientBegin+12,
	k_EMsgClientDeleteObject = k_EMsgClientBegin+13,
	k_EMsgClientKilledBy = k_EMsgClientBegin+14,
	k_EMsgClientServerReadyForSpawn = k_EMsgClientBegin+15,
	k_EMsgClientSetSendFileCount = k_EMsgClientBegin+16,
	k_EMsgClientSendFileBegin = k_EMsgClientBegin+17,
	k_EMsgClientSendChunk = k_EMsgClientBegin+18,
	k_EMsgClientPlayerIamSyncedWithServerFiles = k_EMsgClientBegin+19,
	k_EMsgClientPlayerSendIAmLoadedAndReady = k_EMsgClientBegin+20,
	k_EMsgClientEveryoneLoadedAndReady = k_EMsgClientBegin+21,
	k_EMsgClientPlayerAppearance = k_EMsgClientBegin+22,
	k_EMsgClientSetCollision = k_EMsgClientBegin+23,
	k_EMsgClientPlayAnimation = k_EMsgClientBegin+24,
	k_EMsgClientPlayerSendIAmReadyToPlay = k_EMsgClientBegin+25,
	k_EMsgClientEveryoneReadyToPlay = k_EMsgClientBegin+26,
	k_EMsgClientShoot = k_EMsgClientBegin+27,
	k_EMsgClientLeft = k_EMsgClientBegin+28,
	k_EMsgClientDestroyObject = k_EMsgClientBegin+29,
	k_EMsgClientLua = k_EMsgClientBegin+30,
	k_EMsgClientKilledSelf = k_EMsgClientBegin+31,
	k_EMsgReceipt = k_EMsgClientBegin+32,
	k_EMsgChat = k_EMsgClientBegin+33,
	k_EMsgEndGame = k_EMsgClientBegin+34,
	k_EMsgClientLuaString = k_EMsgClientBegin+35,
	k_EMsgClientAvatarDoWeHaveHeadTex = k_EMsgClientBegin+36,
	k_EMsgClientSendAvatarFileBeginClient = k_EMsgClientBegin+37,
	k_EMsgClientSendAvatarChunkClient = k_EMsgClientBegin+38,
	k_EMsgClientSendAvatarDone = k_EMsgClientBegin+39,

	// P2P authentication messages
	k_EMsgP2PBegin = 600, 
	k_EMsgP2PSendingTicket = k_EMsgP2PBegin+1,

	// voice chat messages
	k_EMsgVoiceChatBegin = 700, 
	k_EMsgVoiceChatPing = k_EMsgVoiceChatBegin+1,	// just a keep alive message
	k_EMsgVoiceChatData = k_EMsgVoiceChatBegin+2,	// voice data from another player



	// force 32-bit size enum so the wire protocol doesn't get outgrown later
	k_EForceDWORD  = 0x7fffffff, 
};


// Msg from the server to the client which is sent right after communications are established
// and tells the client what SteamID the game server is using as well as whether the server is secure
struct MsgServerSendInfo_t
{
	MsgServerSendInfo_t() : m_dwMessageType( LittleDWord( k_EMsgServerSendInfo ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	void SetSteamIDServer( uint64 SteamID ) { m_ulSteamIDServer = LittleQWord( SteamID ); }
	uint64 GetSteamIDServer() { return LittleQWord( m_ulSteamIDServer ); }

	void SetSecure( bool bSecure ) { m_bIsVACSecure = bSecure; }
	bool GetSecure() { return m_bIsVACSecure; }

	void SetServerName( const char *pchName ) { strncpy( m_rgchServerName, pchName, sizeof( m_rgchServerName ) ); }
	const char *GetServerName() { return m_rgchServerName; }

private:
	const DWORD m_dwMessageType;
	uint64 m_ulSteamIDServer;
	bool m_bIsVACSecure;
	char m_rgchServerName[128];
};

// Msg from the server to the client when refusing a connection
struct MsgServerFailAuthentication_t
{
	MsgServerFailAuthentication_t() : m_dwMessageType( LittleDWord( k_EMsgServerFailAuthentication ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }
private:
	const DWORD m_dwMessageType;
};

// Msg from the server to client when accepting a pending connection
struct MsgServerPassAuthentication_t
{
	MsgServerPassAuthentication_t() : m_dwMessageType( LittleDWord( k_EMsgServerPassAuthentication ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	void SetPlayerPosition ( uint32 pos ) { m_uPlayerPosition = LittleDWord( pos ); }
	uint32 GetPlayerPosition() { return LittleDWord( m_uPlayerPosition ); }

private:
	const DWORD m_dwMessageType;
	uint32 m_uPlayerPosition;
};

// Msg from the server to clients when updating the world state
struct MsgServerUpdateWorld_t
{
	MsgServerUpdateWorld_t() : m_dwMessageType( LittleDWord( k_EMsgServerUpdateWorld ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	ServerSteamUpdateData_t *AccessUpdateData() { return &m_ServerUpdateData; }

private:
	const DWORD m_dwMessageType;
	ServerSteamUpdateData_t m_ServerUpdateData;
};

// Msg from server to clients when it is exiting
struct MsgServerExiting_t
{
	MsgServerExiting_t() : m_dwMessageType( LittleDWord( k_EMsgServerExiting ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// Msg from server to clients when it is exiting
struct MsgServerPingResponse_t
{
	MsgServerPingResponse_t() : m_dwMessageType( LittleDWord( k_EMsgServerPingResponse ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// Msg from client to server when trying to connect
struct MsgClientInitiateConnection_t
{
	MsgClientInitiateConnection_t() : m_dwMessageType( LittleDWord( k_EMsgClientInitiateConnection ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// Msg from client to server when initiating authentication
struct MsgClientBeginAuthentication_t
{
	MsgClientBeginAuthentication_t() : m_dwMessageType( LittleDWord( k_EMsgClientBeginAuthentication ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	void SetToken( const char *pchToken, uint32 unLen ) { m_uTokenLen = LittleDWord( unLen ); memcpy( m_rgchToken, pchToken, MIN( unLen, sizeof( m_rgchToken ) ) ); }
	uint32 GetTokenLen() { return LittleDWord( m_uTokenLen ); }
	const char *GetTokenPtr() { return m_rgchToken; }

	void SetSteamID( uint64 ulSteamID ) { m_ulSteamID = LittleQWord( ulSteamID ); }
	uint64 GetSteamID() { return LittleQWord( m_ulSteamID ); }

private:
	const DWORD m_dwMessageType;
	
	uint32 m_uTokenLen;
#ifdef USE_GS_AUTH_API
	char m_rgchToken[1024];
#endif
	uint64 m_ulSteamID;
};

// Msg from client to server when sending state update
struct MsgClientSendLocalUpdate_t
{
	MsgClientSendLocalUpdate_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendLocalUpdate ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }
	
	void SetShipPosition( uint32 uPos ) { m_uShipPosition = LittleDWord( uPos ); }
	ClientSteamUpdateData_t *AccessUpdateData() { return &m_ClientUpdateData; }

private:
	const DWORD m_dwMessageType;

	uint32 m_uShipPosition;
	ClientSteamUpdateData_t m_ClientUpdateData;
};

// Msg from the client telling the server it is about to leave
struct MsgClientLeavingServer_t
{
	MsgClientLeavingServer_t() : m_dwMessageType( LittleDWord( k_EMsgClientLeavingServer ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// Msg from the client informing the server of its name
struct MsgClientPlayerName_t
{
	MsgClientPlayerName_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerName ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char name[128];
	int index;
};

// Msg from the client informing the server of its name
struct MsgChat_t
{
	MsgChat_t() : m_dwMessageType( LittleDWord( k_EMsgChat ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char msg[82];
	int index;
};

// Msg from the client informing the server of its score
struct MsgClientPlayerScore_t
{
	MsgClientPlayerScore_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerScore ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int score;
	int index;
};

struct MsgClientShoot_t
{
	MsgClientShoot_t() : m_dwMessageType( LittleDWord( k_EMsgClientShoot ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
};

struct MsgClientSetCollision_t
{
	MsgClientSetCollision_t() : m_dwMessageType( LittleDWord( k_EMsgClientSetCollision ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char playerIndex;
	int index;
	int state;
};

struct MsgClientPlayAnimation_t
{
	MsgClientPlayAnimation_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayAnimation ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char playerIndex;
	int index;
	int start;
	int end;
	int speed;
};

// Msg from the client informing the server of its name
struct MsgClientPlayerBullet_t
{
	MsgClientPlayerBullet_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerBullet ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	float x;
	float y;
	float z;
	float anglex;
	float angley;
	float anglez;
	int type;
	int on;
	int index;
};

struct MsgClientServerReadyForSpawn_t
	{
	MsgClientServerReadyForSpawn_t() : m_dwMessageType( LittleDWord( k_EMsgClientServerReadyForSpawn ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
};

// Msg from the client informing the server of its name
struct MsgClientPlayerKeyState_t
{
	MsgClientPlayerKeyState_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerKeyState ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	unsigned char key;
	char state;
};

struct MsgClientPlayerApplyDamage_t
{
	MsgClientPlayerApplyDamage_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerApplyDamage ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	char source;
	int amount;
	int x;
	int y;
	int z;
	int force;
	int limb;
	int logID;
};

struct MsgClientLua_t
{
	MsgClientLua_t() : m_dwMessageType( LittleDWord( k_EMsgClientLua ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int code;
	int e;
	int v;
	int logID;
};

struct MsgClientLuaString_t
{
	MsgClientLuaString_t() : m_dwMessageType( LittleDWord( k_EMsgClientLuaString ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int code;
	int e;
	char s[CURRENT_LUA_STRING_SIZE];
	int logID;
};

struct MsgServerLua_t
{
	MsgServerLua_t() : m_dwMessageType( LittleDWord( k_EMsgServerLua ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int code;
	int e;
	int v;
	int logID;
};

struct MsgServerLuaString_t
{
	MsgServerLuaString_t() : m_dwMessageType( LittleDWord( k_EMsgServerLuaString ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int code;
	int e;
	char s[CURRENT_LUA_STRING_SIZE];
	int logID;
};

struct MsgClientPlayerSetPlayerAlive_t
{
	MsgClientPlayerSetPlayerAlive_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerSetPlayerAlive ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	char state;
};

struct MsgClientSpawnObject_t
{
	MsgClientSpawnObject_t() : m_dwMessageType( LittleDWord( k_EMsgClientSpawnObject ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int object;
	int source;
	float x;
	float y;
	float z;
};

struct MsgClientDeleteObject_t
{
	MsgClientDeleteObject_t() : m_dwMessageType( LittleDWord( k_EMsgClientDeleteObject ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int object;
};

struct MsgClientDestroyObject_t
{
	MsgClientDestroyObject_t() : m_dwMessageType( LittleDWord( k_EMsgClientDestroyObject ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int object;
};

struct MsgClientKilledBy_t
{
	MsgClientKilledBy_t() : m_dwMessageType( LittleDWord( k_EMsgClientKilledBy ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	char killedBy;
	int x;
	int y;
	int z;
	int force;
	int limb;
	int logID;
};

struct MsgClientKilledSelf_t
{
	MsgClientKilledSelf_t() : m_dwMessageType( LittleDWord( k_EMsgClientKilledSelf ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
};

struct MsgClientLeft_t
{
	MsgClientLeft_t() : m_dwMessageType( LittleDWord( k_EMsgClientLeft ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
};

struct MsgClientPlayerAppearance_t
{
	MsgClientPlayerAppearance_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerAppearance ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	char index;
	int appearance;
};

struct MsgClientSetSendFileCount_t
{
	MsgClientSetSendFileCount_t() : m_dwMessageType( LittleDWord( k_EMsgClientSetSendFileCount ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int count;
};

struct MsgClientSendFileBegin_t
{
	MsgClientSendFileBegin_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendFileBegin ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	char fileName[512];
	int	fileSize;
};

struct MsgClientSendChunk_t
{
	MsgClientSendChunk_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendChunk ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	BYTE chunk[FILE_CHUNK_SIZE];
};

struct MsgClientSendAvatarFileBeginClient_t
{
	MsgClientSendAvatarFileBeginClient_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendAvatarFileBeginClient ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	char fileName[512];
	int	fileSize;
};

struct MsgClientSendAvatarChunkClient_t
{
	MsgClientSendAvatarChunkClient_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendAvatarChunkClient ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	int count;
	BYTE chunk[FILE_CHUNK_SIZE];
};

struct MsgClientSendAvatarDone_t
{
	MsgClientSendAvatarDone_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendAvatarDone ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
};

struct MsgClientPlayerIamSyncedWithServerFiles_t
{
	MsgClientPlayerIamSyncedWithServerFiles_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerIamSyncedWithServerFiles ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
};

struct MsgClientSendIAmLoadedAndReady_t
{
	MsgClientSendIAmLoadedAndReady_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerSendIAmLoadedAndReady ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
};

struct MsgClientEveryoneLoadedAndReady_t
{
	MsgClientEveryoneLoadedAndReady_t() : m_dwMessageType( LittleDWord( k_EMsgClientEveryoneLoadedAndReady ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int logID;
};

struct MsgClientSendIAmReadyToPlay_t
{
	MsgClientSendIAmReadyToPlay_t() : m_dwMessageType( LittleDWord( k_EMsgClientPlayerSendIAmReadyToPlay ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	int logID;
};

struct MsgReceipt_t
{
	MsgReceipt_t() : m_dwMessageType( LittleDWord( k_EMsgReceipt ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int logID;
};

struct MsgEndGame_t
{
	MsgEndGame_t() : m_dwMessageType( LittleDWord( k_EMsgEndGame) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

struct MsgClientEveryoneReadyToPlay_t
{
	MsgClientEveryoneReadyToPlay_t() : m_dwMessageType( LittleDWord( k_EMsgClientEveryoneReadyToPlay ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int logID;
};


// server ping
struct MsgClientPing_t
{
	MsgClientPing_t() : m_dwMessageType( LittleDWord( k_EMsgClientPing ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// Msg from client to server when trying to connect
struct MsgP2PSendingTicket_t
{
	MsgP2PSendingTicket_t() : m_dwMessageType( LittleDWord( k_EMsgP2PSendingTicket ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }


	void SetToken( const char *pchToken, uint32 unLen ) { m_uTokenLen = LittleDWord( unLen ); memcpy( m_rgchToken, pchToken, MIN( unLen, sizeof( m_rgchToken ) ) ); }
	uint32 GetTokenLen() { return LittleDWord( m_uTokenLen ); }
	const char *GetTokenPtr() { return m_rgchToken; }

	void SetSteamID( uint64 ulSteamID ) { m_ulSteamID = LittleQWord( ulSteamID ); }
	uint64 GetSteamID() { return LittleQWord( m_ulSteamID ); }

private:
	const DWORD m_dwMessageType;
	uint32 m_uTokenLen;
	char m_rgchToken[1024];
	uint64 m_ulSteamID;
};

// voice chat ping
struct MsgVoiceChatPing_t
{
	 MsgVoiceChatPing_t() : m_dwMessageType( LittleDWord( k_EMsgVoiceChatPing ) ) {}
	DWORD GetMessageType() const { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
};

// voice chat data
struct MsgVoiceChatData_t
{
	MsgVoiceChatData_t() : m_dwMessageType( LittleDWord( k_EMsgVoiceChatData ) ) {}
	DWORD GetMessageType() const { return LittleDWord( m_dwMessageType ); }
	
	void SetDataLength( uint32 unLength ) { m_uDataLength = LittleDWord( unLength ); }
	uint32 GetDataLength() const { return LittleDWord( m_uDataLength ); }

private:
	const DWORD m_dwMessageType;
	uint32 m_uDataLength;
};

struct MsgClientAvatarDoWeHaveHeadTex_t
{
	MsgClientAvatarDoWeHaveHeadTex_t() : m_dwMessageType( LittleDWord( k_EMsgClientAvatarDoWeHaveHeadTex ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int index;
	int flag;
	int logID;
};

struct MsgServerAvatarChangeMode_t
{
	MsgServerAvatarChangeMode_t() : m_dwMessageType( LittleDWord( k_EMsgServerAvatarChangeMode ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

private:
	const DWORD m_dwMessageType;
public:
	int mode;
	int result;
	int result2;
};

#pragma pack( pop )

#endif // MESSAGES_H
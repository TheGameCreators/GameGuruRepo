// PhotonMultiplayer.cpp : Defines the exported functions for the DLL application.
//

// Includes
#include "stdafx.h"
#include ".\globstruct.h"
#include "PhotonCommands.h"
#include "PhotonMultiplayer.h"
#include "LoadBalancingListener.h"
//#include "CClient.h"

// Namespaces
using namespace ExitGames::LoadBalancing;
using namespace ExitGames::Common;
using namespace ExitGames::Photon;

// Global Consts
static const ExitGames::Common::JString appID = L"ec03ea1d-6245-4e12-bbd2-9d4f1c10116c";
static const ExitGames::Common::JString appVersion = L"1.0";

// Globals - main connections
bool gUseTcp = false;
PhotonView* g_pPhotonView = NULL;
LoadBalancingListener* g_pLBL = NULL;
Client* g_pLBC = NULL;
JString g_sPlayerName = "";
char hostsLobbyName[256];
char g_pPhotonPlayerDisplayName[MAX_PLAYERS_PER_SERVER][1024];

// Globals - setting up and playing game
int HowManyPlayersDoWeHave = 0;
//uint64 server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
//bool serverActive = false;

/* move these up as we implement them (if relevant!)
bool OnlineMultiplayerModeForSharingFiles = false;
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)
CSteamID lobbyIAmInID;
int scores[MAX_PLAYERS_PER_SERVER];
*/
int keystate[MAX_PLAYERS_PER_SERVER][256];
int alive[MAX_PLAYERS_PER_SERVER];
/*
tbullet bullets[180];
bool bulletSeen[180][MAX_PLAYERS_PER_SERVER];
int playerDamage = 0;
int damageSource = 0;
int damageX = 0;
int damageY = 0;
int damageZ = 0;
int damageForce = 0;
int damageLimb = 0;
int killedSource[MAX_PLAYERS_PER_SERVER];
int killedX[MAX_PLAYERS_PER_SERVER];
int killedY[MAX_PLAYERS_PER_SERVER];
int killedZ[MAX_PLAYERS_PER_SERVER];
int killedForce[MAX_PLAYERS_PER_SERVER];
int killedLimb[MAX_PLAYERS_PER_SERVER];
CSteamID playerSteamIDs[MAX_PLAYERS_PER_SERVER];
*/
int playerAppearance[MAX_PLAYERS_PER_SERVER];
int playerShoot[MAX_PLAYERS_PER_SERVER];
int tweening[MAX_PLAYERS_PER_SERVER];
/*
char workshopItemName[256];
char steamRootPath[MAX_PATH];
bool needToSendMyName = true;
int ClientDeathNumber = 1;
int ServerClientLastDeathNumber[MAX_PLAYERS_PER_SERVER];
int SteamOverlayActive = 0;
int ServerIsShuttingDown = 0;
PublishedFileId_t WorkShopItemID = NULL;
UGCUpdateHandle_t WorkShopItemUpdateHandle = NULL;
uint64 WorkshopItemToDownloadID = NULL;
char WorkshopItemPath[MAX_PATH] = "";
int IsWorkshopLoadingOn = 0;
char hostsLobbyName[256];
bool isPlayingOnAServer = false;
extern GlobStruct* g_pGlob;
extern CClient *g_pClient;
int packetSendLogClientID = 0;
int packetSendLogServerID = 0;
std::vector <packetSendLogClient_t> PacketSend_Log_Client;
std::vector <packetSendLogServer_t> PacketSend_Log_Server;
std::vector <packetSendReceiptLogClient_t> PacketSendReceipt_Log_Client;
std::vector <packetSendReceiptLogServer_t> PacketSendReceipt_Log_Server;
std::vector <tSpawn> spawnList;
*/
std::vector <tLua> luaList;
/*
std::vector <int> deleteList;
std::vector <int> deleteListSource;
std::vector <int> destroyList;
std::vector <tMessage> messageList;
std::vector <tCollision> collisionList;
*/
std::vector <tAnimation> animationList;
/*
std::vector <tChat> chatList;
std::vector <uint32> lobbyChatIDs;
int ServerHowManyToStart = 0;
int ServerHowManyJoined = 0;
int ServerSaysItIsOkayToStart = 0;
int ServerHaveIToldClientsToStart = 0;
*/
int ServerFilesToReceive = 0;
int ServerFilesReceived = 0;
int IamSyncedWithServerFiles = 0;
int serverOnlySendMapToSpecificPlayer = 0;
/*
int IamLoadedAndReady = 0;
int isEveryoneLoadedAndReady = 0;
int HowManyPlayersDoWeHave = 0;
int IamReadyToPlay = 0;
int isEveryoneReadyToPlay = 0;
*/
int serverHowManyFileChunks = 0;
int serverChunkToSendCount = 0;
int serverFileFileSize = 0;
/*
uint64 ServerCreationTime = 0;
int fileProgress = 0;
int voiceChatOn = 0;
int syncedAvatarTextureMode = SYNC_AVATAR_TEX_BEGIN;
int syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_BEGIN;
char myAvatarTextureName[MAX_PATH];
int syncedAvatarHowManyTextures = 0;
int syncedAvatarHowManyTexturesReceived = 0;
FILE* avatarFile[MAX_PLAYERS_PER_SERVER];
int avatarHowManyFileChunks[MAX_PLAYERS_PER_SERVER];
int avatarFileFileSize[MAX_PLAYERS_PER_SERVER] ;
int serverClientsFileSynced[MAX_PLAYERS_PER_SERVER];
*/
int serverClientsLoadedAndReady[MAX_PLAYERS_PER_SERVER];
int serverClientsReadyToPlay[MAX_PLAYERS_PER_SERVER];
/*
int serverClientsHaveAvatarTexture[MAX_PLAYERS_PER_SERVER];
FILE* serverFile = NULL;
tSpawn currentSpawnObject;
*/
tLua currentLua;
/*
int currentDeleteObject;
int currentDeleteObjectSource;
int currentDestroyObject;
tCollision currentCollisionObject;
*/
tAnimation currentAnimationObject;

// photon core commands

int PhotonInit(LPSTR pRootPath,LPSTR pSiteName,LPSTR pAvatarName,bool bViewAllMode,LPSTR pOptionalPhotonAppID)
{
	// create photon classes
	g_pPhotonView = new PhotonView();
	g_pLBL = new LoadBalancingListener(g_pPhotonView, pRootPath);

	JString pPhotonAppID = appID;
	if ( pOptionalPhotonAppID ) pPhotonAppID = JString(pOptionalPhotonAppID);
	g_pLBC = new Client(*g_pLBL, pPhotonAppID, appVersion, gUseTcp?ExitGames::Photon::ConnectionProtocol::TCP:ExitGames::Photon::ConnectionProtocol::UDP);
	
	// for debugging and testing, allow 20 MINUTES timeout grace for debugging
	g_pLBC->setDisconnectTimeout(120000*10);

	// initialise photon
	g_pLBC->setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
	ExitGames::Common::Base::setListener(g_pLBL);
	ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
	g_pLBL->setLBC(g_pLBC);
	g_pLBC->setTrafficStatsEnabled(true);

	// store site name for later room creation
	g_pPhotonView->SetSiteName ( pSiteName );

	// set teacher view all mode
	g_pPhotonView->SetTeacherViewAllMode ( bViewAllMode );

	// construct player name from custom-name & uniqueid
	char pUniquePlayerName[1024];
	if ( strlen ( pAvatarName ) > 0 )
	{
		strcpy ( pUniquePlayerName, pAvatarName );
		strcat ( pUniquePlayerName, ":" );
	}
	else
	{
		sprintf ( pUniquePlayerName, "PLR%5d:", GETTIMEMS() );
	}

	// connect player to game room
	g_sPlayerName = JString(pUniquePlayerName)+GETTIMEMS();
	g_pLBL->connect(g_sPlayerName);

	// inits - after much tracing through this old code, it has too many Steam specific hooks
	// we can create simpler state management in a way that allows easy 'host migration'
	PhotonResetGameStats();

	// inits
	g_pPhotonView->iTriggerLeaveMode = 0;

	// success
	return 1;
}

void PhotonFree()
{
	// free MP resources
	SAFE_DELETE ( g_pLBC );
	SAFE_DELETE ( g_pLBL );
	SAFE_DELETE ( g_pPhotonView );
}

void PhotonLoop(void)
{
	// if photon active
	if ( g_pPhotonView )
	{
		// monitor players in this game (when game is running; for leaves and new joins)
		g_pLBL->manageTrackedPlayerList();

		// disconnect player handling sequence
		if ( g_pPhotonView->iTriggerLeaveMode == 2 )
		{
			// Timeout stale player connections, also update player count data
			//g_pLBL->leaveRoom(); seems leaveroomreturn is not being called when call this, so 
			//go direct to disconnect instead!
			//g_pPhotonView->iTriggerLeaveMode = 3;
			g_pPhotonView->iTriggerLeaveMode = 4;
			g_pLBC->disconnect();
		}
		if ( g_pPhotonView->iTriggerLeaveMode == 1 )
		{
			// Timeout stale player connections, also update player count data
			g_pLBL->migrateHostIfServer();
			g_pPhotonView->iTriggerLeaveMode = 2;
		}
		if ( g_pPhotonView->iTriggerLeaveMode == 3 ) 
		{
			if ( g_pPhotonView->isInGameRoom()==false )
			{
				g_pPhotonView->iTriggerLeaveMode = 4;
				g_pLBC->disconnect();
			}
			if ( g_pPhotonView->isConnected() == false )
			{
				g_pPhotonView->iTriggerLeaveMode = 4;
			}
		}
		if ( g_pPhotonView->iTriggerLeaveMode == 4 && g_pPhotonView->isConnecting() == false && g_pPhotonView->isConnected() == false ) 
		{
			g_pPhotonView->iTriggerLeaveMode = 99;
		}

		// sharing data in the room
		if ( g_pPhotonView->isInGameRoom() == true )
		{
			// run logic in game room
			int iMPState = g_pLBL->service();
		}

		// sync network services
		g_pLBC->serviceBasic();
		while(g_pLBC->dispatchIncomingCommands());
		while(g_pLBC->sendOutgoingCommands());
	}
}

int PhotonCloseConnection()
{
	// trigger player to leave
	if ( g_pPhotonView->iTriggerLeaveMode == 0 )
		g_pPhotonView->iTriggerLeaveMode = 1;

	// determine when connection ended
	if ( g_pPhotonView->iTriggerLeaveMode == 99 )
		return 1;
	else
		return 0;
}

LPSTR PhotonGetSiteName()
{
	if ( g_pPhotonView )
	{
		return g_pPhotonView->GetSiteName();
	}
	return NULL;
}

int PhotonGetViewAllMode()
{
	if ( g_pPhotonView )
		if ( g_pPhotonView->IsTeacherViewAllMode() == true )
			return 1;
	return 0;
}

bool PhotonPlayerLeaving()
{
	if ( g_pPhotonView->iTriggerLeaveMode > 0 )
		return true;
	else
		return false;
}

int PhotonPlayerArrived()
{
	return g_pLBL->findANewlyArrivedPlayer();
}

int PhotonGetRemap(int iRealPhotonPlayerNr)
{
	return g_pLBL->GetRemap(iRealPhotonPlayerNr);
}

void PhotonInitClient()
{
	//g_pClient = new CClient();
	//g_pClient->Init(g_pLBL);
}

void PhotonResetClient()
{
	//PhotonCleanupClient();
	//PhotonResetGameStats();

	//SAFE_DELETE ( g_pClient );
	//g_pClient = new CClient();
	//g_pClient->Init(g_pLBL);

	/*
	deleteList.clear();
	deleteListSource.clear();
	destroyList.clear();
	messageList.clear();
	chatList.clear();
	lobbyChatIDs.clear();
	*/
}

//void PhotonCleanupClient()
//{
//	PhotonResetGameStats();
//}

void PhotonResetGameStats()
{
	luaList.clear();
	/*
	serverActive = false;
	IsWorkshopLoadingOn = 0;
	ServerIsShuttingDown = 0;
	*/

	//char fileToDelete[MAX_PATH];
	for ( int c = 0; c < MAX_PLAYERS_PER_SERVER ; c++ )
	{
		/*
		scores[c] = 0;
		alive[c] = 1;
		*/
		for ( int k = 0 ; k < 256 ; k++ ) keystate[c][k];
		playerAppearance[c] = 0;
		playerShoot[c] = 0;
		tweening[c] = 1;

		/*
		serverClientsFileSynced[c] = 0;
		serverClientsLoadedAndReady[c] = 0;
		serverClientsReadyToPlay[c] = 0;
		HowManyPlayersDoWeHave = 0;

		avatarFile[c] = NULL;
		avatarHowManyFileChunks[c] = 0;
		avatarFileFileSize[c] = 0;

		sprintf ( fileToDelete, "%sentitybank\\user\\charactercreator\\customAvatar_%i_cc.dds" , steamRootPath , c );
		DeleteFile ( fileToDelete );

		sprintf ( fileToDelete, "%sentitybank\\user\\charactercreator\\customAvatar_%i.fpe" , steamRootPath , c );
		DeleteFile ( fileToDelete );
		*/
	}

	/*
	for ( int i = 0 ; i < 179 ; i++ )
	{
		bullets[i].on = 0;
		for ( int a = 0 ; a < MAX_PLAYERS_PER_SERVER; a++ )
			bulletSeen[i][a] = false;
	}

	server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
	*/
	ServerFilesToReceive = 0;
	ServerFilesReceived = 0;
	IamSyncedWithServerFiles = 0;
	/*
	IamLoadedAndReady = 0;
	IamReadyToPlay = 0;
	isEveryoneLoadedAndReady = 0;
	isEveryoneReadyToPlay = 0;
	serverChunkToSendCount = 0;
	fileProgress = 0;
	HowManyPlayersDoWeHave = 0;
	ServerHowManyToStart = 0;
	*/
	g_pLBL->CloseFileNow();

	/*
	spawnList.clear();
	luaList.clear();
	deleteList.clear();
	deleteListSource.clear();
	collisionList.clear();
	*/
	animationList.clear();
	/*
	deleteList.clear();
	deleteListSource.clear();
	destroyList.clear();
	*/
}

// photon creating/listing lobby/gamerooms

void PhotonGetLobbyList()
{
	if ( g_pPhotonView )
	{
		g_pLBL->updateRoomList();
	}
}

int PhotonIsLobbyListCreated()
{
	if ( g_pPhotonView )
		return 1;
	else
		return 0;
}

int PhotonGetLobbyListSize()
{
	if ( g_pPhotonView )
		return g_pPhotonView->GetRoomCount();
	else
		return 0;
}

LPSTR PhotonGetLobbyListName( int index )
{
	if ( g_pPhotonView )
		return g_pPhotonView->GetRoomName ( index );
	else
		return NULL;
}

void PhotonSetLobbyName( LPSTR name )
{
	strcpy ( hostsLobbyName, g_pPhotonView->GetSiteName() );
	strcat ( hostsLobbyName, ":" );
	strcat ( hostsLobbyName, name );
}

void PhotonCreateLobby()
{
	//isPlayingOnAServer = false;
	if ( 1 )//g_SteamRunning )
	{
		g_pLBL->createRoom ( hostsLobbyName );
		//messageList.clear();
		//chatList.clear();
		//deleteList.clear();
		//deleteListSource.clear();
		//destroyList.clear();
		//lobbyChatIDs.clear();
		//syncedAvatarTextureMode = SYNC_AVATAR_TEX_BEGIN;
		//syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_BEGIN;
		//PacketSend_Log_Client.clear();
		//PacketSend_Log_Server.clear();
		//PacketSendReceipt_Log_Client.clear();
		//PacketSendReceipt_Log_Server.clear();
		//ClientDeathNumber = 1;
		//for ( int c = 0 ; c < MAX_PLAYERS_PER_SERVER ; c++ )
		//	ServerClientLastDeathNumber[c] = 0;
		//server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		//Client()->SteamCreateLobby();
	}
}

void PhotonJoinLobby ( LPSTR name )
{
	if ( g_pPhotonView )
	{
		g_pLBC->opJoinRoom ( name );
	}
}

int PhotonHasJoinedLobby() 
{ 
	if ( g_pPhotonView )
	{
		if ( g_pLBC->getIsInRoom() == true )
			return 1;
	}
	return 0; 
}

int PhotonGetLobbyUserCount() 
{
	if ( g_pPhotonView )
	{
		g_pLBL->updatePlayerList();
		return g_pPhotonView->GetPlayerCount();
	}
	return 0;
}

LPSTR PhotonGetLobbyUserName ( int index ) 
{ 
	if ( g_pPhotonView )
	{
		int iPlayerCount = g_pPhotonView->GetPlayerCount();
		if ( iPlayerCount > 0 && index < iPlayerCount ) 
		{
			return g_pPhotonView->GetPlayerName ( index );
		}
	}
	return NULL; 
}

LPSTR PhotonGetLobbyUserDisplayName ( int index ) 
{ 
	if ( g_pPhotonView )
	{
		LPSTR pFullPlayerNameWithUniqueCode = PhotonGetLobbyUserName ( index );
		strcpy ( g_pPhotonPlayerDisplayName[index], "" );
		if ( pFullPlayerNameWithUniqueCode )
		{
			strcpy ( g_pPhotonPlayerDisplayName[index], pFullPlayerNameWithUniqueCode );
			for ( int n = 0; n < strlen(g_pPhotonPlayerDisplayName[index]); n++ )
			{
				if ( g_pPhotonPlayerDisplayName[index][n] == ':' ) 
				{
					g_pPhotonPlayerDisplayName[index][n] = 0;
					break;
				}
			}
			return g_pPhotonPlayerDisplayName[index];
		}
		else
			return NULL;
	}
	return NULL; 
}

LPSTR PhotonGetPlayerName(void) 
{ 
	const char* str = g_sPlayerName.ANSIRepresentation().cstr();
	return (LPSTR)str; 
}

void PhotonLeaveLobby() 
{
	if ( g_pPhotonView )
	{
		if ( g_pPhotonView->isInGameRoom() == true )
		{
			g_pLBL->leaveRoom();
		}
	}
}

// photon setting up and starting game

int PhotonGetClientServerConnectionStatus() 
{ 
	if ( g_pPhotonView )
	{
		if ( g_pPhotonView->isConnected() == true )
			return 1;
	}
	return 0;
}

void PhotonStartServer()
{
	if ( g_pPhotonView )
	{
		g_pLBL->sendGlobalVarState ( eGlobalEventGameRunning, 1 );
		//server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		//Client()->StartServer();
		//serverActive = true;
	}
}

int PhotonIsServerRunning()
{
	if ( g_pPhotonView )
	{
		return 1;
		//return Client()->IsServerRunning();
	}
	return 0;
}

int PhotonIsPlayerTheServer()
{
	if ( g_pPhotonView )
	{
		if ( g_pLBL->isServer() == true )
			return 1;
	}
	return 0;
}

void PhotonCheckIfGameRunning()
{
	if ( g_pPhotonView )
	{
		MsgGetGlobalStates_t msg;
		g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgGetGlobalStates_t), true );
	}
}

int PhotonIsGameRunning()
{
	if ( g_pPhotonView )
	{
		return g_pPhotonView->GlobalStates.iGameRunning;
	}
	return 0;
}

int PhotonGetMyPlayerIndex()
{
	if ( g_pPhotonView )
		return g_pLBL->GetRemap(g_pLBL->getLocalPlayerID());
	return -1;
}

int PhotonGetMyRealPlayerNr()
{
	if ( g_pPhotonView )
		return g_pLBL->getLocalPlayerID();
	return -1;
}

///

void PhotonSetRoot(LPSTR string )
{
}

void PhotonResetFile ( void )
{
	if ( g_pLBL )
	{
		g_pLBL->CloseFileNow();
	}
}

void PhotonSetSendFileCount ( int count, int iOnlySendMapToSpecificPlayer )
{
	if ( g_pPhotonView )
	{
		g_pLBL->CloseFileNow(); // always ensure file is ready for new writing!
		g_pLBL->SetSendFileCount(count,iOnlySendMapToSpecificPlayer);	
	}
}

void PhotonSendFileBegin ( int index , LPSTR pString, LPSTR pRootPath )
{
	if ( g_pPhotonView )
	{
		g_pLBL->SendFileBegin( index, pString, pRootPath );	
	}
}

float PhotonGetSendProgress()
{
	if ( g_pPhotonView )
	{
		return g_pLBL->GetSendProgress();
	}
	return 0;
}

void PhotonGetSendError ( LPSTR pErrorString )
{
	if ( g_pPhotonView )
	{
		return g_pLBL->GetSendError(pErrorString);
	}
}

int PhotonSendFileDone()
{
	if ( g_pPhotonView )
	{
		return g_pLBL->SendFileDone();
	}
	return 0;
}

int PhotonAmIFileSynced()
{
	if ( g_pPhotonView )
	{
		// as this is called while client is waiting for server sync, call client FPM download code
		g_pLBL->GetFileDone();

		// return if synced
		return IamSyncedWithServerFiles;
	}
	return 0;
}

float PhotonGetFileProgress()
{
	if ( g_pPhotonView )
	{
		return g_pLBL->GetFileProgress();
	}
	return 0;
}

int PhotonIsEveryoneFileSynced()
{
	if ( g_pPhotonView )
	{
		return g_pLBL->IsEveryoneFileSynced();
	}
	return 0;
}

void PhotonRegisterEveryonePresentAsHere()
{
	if ( g_pPhotonView )
	{
		g_pLBL->RegisterEveryonePresentAsHere();
	}
}

void PhotonSetThisPlayerAsCurrentServer()
{
	if ( g_pPhotonView )
	{
		g_pLBL->setPlayerIDAsCurrentServerPlayer();
	}
}

void PhotonSendIAmLoadedAndReady()
{
	MsgClientSendIAmLoadedAndReady_t msg;
	msg.index = g_pLBL->getLocalPlayerID();
	g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientSendIAmLoadedAndReady_t), true );
	if ( g_pLBL->isServer() == true )
	{
		int iSlotIndex = g_pLBL->GetRemap(g_pLBL->miCurrentServerPlayerID);
		g_pLBL->m_rgpPlayerLoadedAndReady[iSlotIndex] = 1;
	}
}

int PhotonIsPlayerLoadedAndReady ( int iRealPlayerNr )
{
	if ( g_pLBL->isServer() == true )
		if ( g_pLBL->isPlayerLoadedAndReady ( iRealPlayerNr ) == true )
			return 1;
	return 0;
}

int PhotonIsEveryoneLoadedAndReady()
{
	// LEE, the 'PhotonSendIAmLoadedAndReady' is only done by non-server players
	if ( g_pLBL->isServer() == true )
	{
		// so when calling this its the server player which waits for responses
		if ( g_pLBL->isEveryoneLoadedAndReady() == true )
		{
			// and finally issues the global state message to change EveryoneLoadedAndReady to 1
			g_pLBL->sendGlobalVarState ( eGlobalEventEveryoneLoadedAndReady, 1 );
			g_pPhotonView->GlobalStates.EveryoneLoadedAndReady = 1;
		}
	}

	// and the above global state gets propagated to all players
	return g_pPhotonView->GlobalStates.EveryoneLoadedAndReady;
}

void PhotonSetPlayerPositionX( float _x )
{
	int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
	if ( iSlotIndex >= 0 )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->x = _x;
			g_pLBL->m_rgpPlayer[iSlotIndex]->newx = _x;
		}
	}
}

void PhotonSetPlayerPositionY( float _y )
{
	int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
	if ( iSlotIndex >= 0 )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->y = _y;
			g_pLBL->m_rgpPlayer[iSlotIndex]->newy = _y;
		}
	}
}

void PhotonSetPlayerPositionZ( float _z )
{
	int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
	if ( iSlotIndex >= 0 )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->z = _z;
			g_pLBL->m_rgpPlayer[iSlotIndex]->newz = _z;
		}
	}
}

void PhotonSetPlayerAngle( float _angle )
{
	int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
	if ( iSlotIndex >= 0 )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->angle = _angle;
			g_pLBL->m_rgpPlayer[iSlotIndex]->newangle = _angle;
		}
	}
}

void PhotonSetPlayerAlive ( int state )
{
	if ( g_pPhotonView )
	{
		int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
		if ( iSlotIndex >= 0 )
		{
			alive[iSlotIndex] = state;
			MsgClientPlayerSetPlayerAlive_t msg;
			msg.index = g_pLBL->muPhotonPlayerIndex;
			msg.state = state;
			g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerSetPlayerAlive_t), false );
		}
	}
}

int PhotonGetPlayerAlive ( int iSlotIndex )
{
	if ( g_pPhotonView )
	{
		if ( iSlotIndex >= 0 )
		{
			return alive[iSlotIndex];
		}
	}
	return 0;
}

float PhotonGetPlayerPositionX ( int iSlotIndex )
{
	if ( g_pPhotonView )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->x = CosineInterpolate ( g_pLBL->m_rgpPlayer[iSlotIndex]->x , g_pLBL->m_rgpPlayer[iSlotIndex]->newx , INTERPOLATE_SMOOTHING );
			return g_pLBL->m_rgpPlayer[iSlotIndex]->x;
		}
	}
	return 0;
}

float PhotonGetPlayerPositionY ( int iSlotIndex )
{
	if ( g_pPhotonView )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->y = CosineInterpolate ( g_pLBL->m_rgpPlayer[iSlotIndex]->y , g_pLBL->m_rgpPlayer[iSlotIndex]->newy , INTERPOLATE_SMOOTHING );
			return g_pLBL->m_rgpPlayer[iSlotIndex]->y;
		}
	}
	return 0;
}

float PhotonGetPlayerPositionZ ( int iSlotIndex )
{
	if ( g_pPhotonView )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->z = CosineInterpolate ( g_pLBL->m_rgpPlayer[iSlotIndex]->z , g_pLBL->m_rgpPlayer[iSlotIndex]->newz , INTERPOLATE_SMOOTHING );
			return g_pLBL->m_rgpPlayer[iSlotIndex]->z;
		}
	}
	return 0;
}

float PhotonGetPlayerAngle ( int iSlotIndex )
{
	if ( g_pPhotonView )
	{
		if ( g_pLBL->m_rgpPlayer[iSlotIndex] )
		{
			g_pLBL->m_rgpPlayer[iSlotIndex]->angle = CosineInterpolateAngle( g_pLBL->m_rgpPlayer[iSlotIndex]->angle , g_pLBL->m_rgpPlayer[iSlotIndex]->newangle , INTERPOLATE_SMOOTHING_TURN );
			return g_pLBL->m_rgpPlayer[iSlotIndex]->angle;
		}
	}
	return 0;
}

void PhotonSetKeyState ( int key , int state )
{
	if ( g_pPhotonView )
	{
		int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
		if ( iSlotIndex >= 0 )
		{
			keystate[iSlotIndex][key] = state;
			MsgClientPlayerKeyState_t msg;
			msg.index = g_pLBL->muPhotonPlayerIndex;
			msg.key = key;
			msg.state = state;
			g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerKeyState_t), false );
		}
	}
}

int PhotonGetKeyState ( int iSlotIndex, int key )
{
	if ( g_pPhotonView )
	{
		return keystate[iSlotIndex][key];
	}
	return 0;
}

void PhotonPlayAnimation ( int index, int start, int end, int speed )
{
	if ( g_pPhotonView )
	{
		MsgClientPlayAnimation_t msg;
		msg.playerIndex = g_pLBL->muPhotonPlayerIndex;
		msg.index = index;
		msg.start = start;
		msg.end = end;
		msg.speed = speed;
		g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayAnimation_t), false );
	}
}

int PhotonGetAnimationList()
{
	if ( animationList.size() > 0 )
	{
		currentAnimationObject.index = animationList.back().index;
		currentAnimationObject.start = animationList.back().start;
		currentAnimationObject.end = animationList.back().end;
		currentAnimationObject.speed = animationList.back().speed;
		return 1;
	}
	return 0;
}

void PhotonGetNextAnimation()
{
	if ( animationList.size() > 0 )
		animationList.pop_back();
}

int PhotonGetAnimationIndex()
{
	return currentAnimationObject.index;
}

int PhotonGetAnimationStart()
{
	return currentAnimationObject.start;
}

int PhotonGetAnimationEnd()
{
	return currentAnimationObject.end;
}

int PhotonGetAnimationSpeed()
{
	return currentAnimationObject.speed;
}

void PhotonSetTweening(int iSlotIndex , int flag)
{
	tweening[iSlotIndex] = flag;
}

void PhotonShoot ( void )
{
	if ( g_pPhotonView )
	{
	}
}

int PhotonGetShoot ( int iSlotIndex )
{
	int result = playerShoot[iSlotIndex];
	playerShoot[iSlotIndex] = 0;
	return result;
}

void PhotonSetPlayerAppearance( int a )
{
	if ( g_pPhotonView )
	{
		int iSlotIndex = g_pLBL->GetRemap(g_pLBL->muPhotonPlayerIndex);
		if ( iSlotIndex >= 0 )
		{
			playerAppearance[iSlotIndex] = a;
			MsgClientPlayerAppearance_t msg;
			msg.index = g_pLBL->muPhotonPlayerIndex;
			msg.appearance = a;
			g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerAppearance_t), false );
		}
	}
}

int PhotonGetPlayerAppearance( int iSlotIndex )
{
	if ( g_pPhotonView )
		return playerAppearance[iSlotIndex];
	else
		return 0;
}

// LUA

void PhotonSendLuaToPlayer ( int iRealPhotonPlayerID, int code, int e, int v )
{
	if ( g_pPhotonView )
	{
		if ( 1 )
		{
			if ( code < 5 )
			{
				MsgClientLua_t msg;
				msg.index = g_pLBL->muPhotonPlayerIndex;
				msg.code = code;
				msg.e = e;
				msg.v = v;
				if ( iRealPhotonPlayerID == -1 )
					g_pLBL->sendMessage ( (nByte*)&msg, sizeof(MsgClientLua_t), true );
				else
					g_pLBL->sendMessageToPlayer ( iRealPhotonPlayerID, (nByte*)&msg, sizeof(MsgClientLua_t), true );
			}
		}
	}
}

void PhotonSendLua ( int code, int e, int v )
{
	PhotonSendLuaToPlayer ( -1, code, e, v );
}

void PhotonSendLuaString ( int code, int e, LPSTR s )
{
	if ( g_pPhotonView )
	{
		//if (!serverActive) return;
		if ( 1 ) // m_steamIDGameServer.IsValid() )
		{
			MsgClientLuaString_t* pmsg;
			pmsg = new MsgClientLuaString_t();
			pmsg->index = g_pLBL->muPhotonPlayerIndex;
			pmsg->code = code;
			pmsg->e = e;
			strcpy ( pmsg->s , s );
			pmsg->logID = 12345;
			g_pLBL->sendMessage ( (nByte*)pmsg, sizeof(MsgClientLuaString_t), true );
		}
	}
}

void PhotonSendLuaPlayerSpecificString ( int code, int iRealPhotonPlayerNr, LPSTR s )
{
	if ( g_pPhotonView )
	{
		if ( 1 )
		{
			MsgClientLuaPlayerSpecificString_t* pmsg;
			pmsg = new MsgClientLuaPlayerSpecificString_t();
			pmsg->index = g_pLBL->muPhotonPlayerIndex;
			pmsg->code = code;
			pmsg->iRealPhotonPlayerNr = iRealPhotonPlayerNr;
			strcpy ( pmsg->s , s );
			pmsg->logID = 12345;
			g_pLBL->sendMessage ( (nByte*)pmsg, sizeof(MsgClientLuaPlayerSpecificString_t), true );
		}
	}
}

int PhotonGetLuaList()
{
	if ( luaList.size() > 0 )
	{
		currentLua.code = luaList.back().code;
		currentLua.e = luaList.back().e;
		currentLua.v = luaList.back().v;
		if ( strlen(luaList.back().s) >= CURRENT_LUA_STRING_SIZE ) MessageBox ( NULL , "Lua String Exceeded Max", "Multiplayer Error", MB_OK );
		strncpy ( currentLua.s , luaList.back().s , CURRENT_LUA_STRING_SIZE );
		currentLua.s[CURRENT_LUA_STRING_SIZE-1] = 0;
		return 1;
	}
	return 0;
}

void PhotonGetNextLua()
{
	if ( luaList.size() > 0 )
		luaList.pop_back();
}

int PhotonGetLuaCommand()
{
	return currentLua.code;
}

int PhotonGetLuaE()
{
	return currentLua.e;
}

int PhotonGetLuaV()
{
	return currentLua.v;
}

LPSTR PhotonGetLuaS(void)
{
	return currentLua.s;
}

//
// empty functions so can compile code with Steam Multiplayer references
//

FPSCR bool SteamOwned ( void ) { return false; }
FPSCR LPSTR SteamGetPlayerName(void) { return "hello"; }
FPSCR LPSTR SteamGetOtherPlayerName( int index ) { return "hello"; }
FPSCR LPSTR SteamGetPlayerID(void) { return "hello"; }
FPSCR void SteamLoop(void) {}
FPSCR int SteamIsOnline(void) { return 0; }
FPSCR void SteamAddAch(void) {}
void ParseCommandLine( const char *pchCmdLine, const char **ppchServerAddress, const char **ppchLobbyID, bool *pbUseVR ) {}
void SteamInitClient() {}
FPSCR void SteamCreateLobby() {}
FPSCR int SteamIsLobbyCreated() { return 0; }
FPSCR void SteamGetLobbyList() {}
FPSCR int SteamIsLobbyListCreated() { return 0; }
FPSCR int SteamGetLobbyListSize() { return 0; }
FPSCR LPSTR SteamGetLobbyListName( int index ) { return "hello"; }
FPSCR void SteamJoinLobby( int index ) {}
FPSCR int SteamGetLobbyUserCount() { return 0; }
FPSCR int SteamHasJoinedLobby() { return 0; }
FPSCR void SteamStartServer() {}
FPSCR int SteamIsServerRunning() { return 0; }
FPSCR int SteamIsGameRunning() { return 0; }
FPSCR int SteamGetMyPlayerIndex() { return 0; }
FPSCR void SteamSetPlayerPositionX( float _x) {}
FPSCR void SteamSetPlayerPositionY( float _y ) {}
FPSCR void SteamSetPlayerPositionZ( float _z ) {}
FPSCR void SteamSetPlayerAngle( float _angle ) {}
FPSCR void SteamSetPlayerScore( int index, int score ) {}
FPSCR float SteamGetPlayerPositionX ( int index ) { return 0.0f; }
FPSCR float SteamGetPlayerPositionY ( int index ) { return 0.0f; }
FPSCR float SteamGetPlayerPositionZ ( int index ) { return 0.0f; }
FPSCR float SteamGetPlayerAngle ( int index ) { return 0.0f; }
FPSCR int SteamGetPlayerScore ( int index ) { return 0; }
FPSCR void SteamSetBullet ( int index , float x , float y , float z, float anglex, float angley, float anglez, int type, int on ) {}
FPSCR int SteamGetBulletOn ( int index ) { return 0; }
FPSCR int SteamGetBulletType ( int index ) { return 0; }
FPSCR float SteamGetBulletX ( int index ) { return 0.0f; }
FPSCR float SteamGetBulletY ( int index ) { return 0.0f; }
FPSCR float SteamGetBulletZ ( int index ) { return 0.0f; }
FPSCR float SteamGetBulletAngleX ( int index ) { return 0.0f; }
FPSCR float SteamGetBulletAngleY ( int index ) { return 0.0f; }
FPSCR float SteamGetBulletAngleZ ( int index ) { return 0.0f; }
FPSCR void SteamSetKeyState ( int key , int state ) {}
FPSCR int SteamGetKeyState ( int index, int key ) { return 0; }
FPSCR void SteamApplyPlayerDamage ( int index, int damage, int x, int y, int z, int force, int limb ) {}
FPSCR int SteamGetPlayerDamageSource() { return 0; }
FPSCR int SteamGetPlayerDamageX() { return 0; }
FPSCR int SteamGetPlayerDamageY() { return 0; }
FPSCR int SteamGetPlayerDamageZ() { return 0; }
FPSCR int SteamGetPlayerDamageForce() { return 0; }
FPSCR int SteamGetPlayerDamageLimb() { return 0; }
FPSCR int SteamGetPlayerKilledSource( int index ) { return 0; }
FPSCR int SteamGetPlayerKilledX( int index ) { return 0; }
FPSCR int SteamGetPlayerKilledY( int index ) { return 0; }
FPSCR int SteamGetPlayerKilledZ( int index ) { return 0; }
FPSCR int SteamGetPlayerKilledForce( int index ) { return 0; }
FPSCR int SteamGetPlayerKilledLimb( int index ) { return 0; }
FPSCR void SteamKilledBy ( int killedBy, int x, int y, int z, int force, int limb ) {}
FPSCR void SteamKilledSelf() {}
FPSCR LPSTR SteamGetServerMessage( void ) { return "hello"; }
FPSCR int SteamGetPlayerDamageAmount () { return 0; }
FPSCR int SteamGetClientServerConnectionStatus() { return 0; }
void SteamResetClient() {}
void SteamCleanupClient() {}
void ResetGameStats() {}
FPSCR void SteamSetPlayerAlive ( int state ) {}
FPSCR int SteamGetPlayerAlive ( int index ) { return 0; }
FPSCR void SteamSpawnObject ( int obj, int source, float x, float y, float z ) {}
FPSCR void SteamDeleteObject ( int obj ) {}
FPSCR void SteamDestroy ( int obj ) {}
FPSCR void SteamSendLua ( int code, int e, int v ) {}
FPSCR void SteamSendLuaString ( int code, int e, LPSTR s ) {}
FPSCR int SteamGetLuaList() { return 0; }
FPSCR void SteamGetNextLua() {}
FPSCR int SteamGetLuaCommand() { return 0; }
FPSCR int SteamGetLuaE() { return 0; }
FPSCR int SteamGetLuaV() { return 0; }
FPSCR LPSTR SteamGetLuaS(void) { return "hello"; }
FPSCR int SteamGetSpawnList() { return 0; }
FPSCR void SteamGetNextSpawn() {}
FPSCR int SteamGetSpawnObjectNumber() { return 0; }
FPSCR int SteamGetSpawnObjectSource() { return 0; }
FPSCR float SteamGetSpawnX() { return 0.0f; }
FPSCR float SteamGetSpawnY() { return 0.0f; }
FPSCR float SteamGetSpawnZ() { return 0.0f; }
FPSCR int SteamGetDeleteList() { return 0; }
FPSCR void SteamGetNextDelete() {}
FPSCR int SteamGetDeleteObjectNumber() { return 0; }
FPSCR int SteamGetDeleteSource() { return 0; }
FPSCR int SteamGetDestroyList() { return 0; }
FPSCR void SteamGetNextDestroy() {}
FPSCR int SteamGetDestroyObjectNumber() { return 0; }
FPSCR int SteamReadyToSpawn() { return 0; }
FPSCR void SteamSetSendFileCount(int count) {}
FPSCR void SteamSendFileBegin ( int index , LPSTR pString ) {}
FPSCR int SteamSendFileDone() { return 0; }
FPSCR int SteamAmIFileSynced() { return 0; }
FPSCR int SteamIsEveryoneFileSynced() { return 0; }
FPSCR void SteamSendIAmLoadedAndReady() {}
FPSCR int SteamIsEveryoneLoadedAndReady() { return 0; }
FPSCR void SteamSendIAmReadyToPlay() {}
FPSCR int SteamIsEveryoneReadyToPlay() { return 0; }
FPSCR int SteamGetFileProgress() { return 0; }
FPSCR void SteamSetVoiceChat( int on ) {}
FPSCR void SteamSetPlayerAppearance( int a ) {}
FPSCR int SteamGetPlayerAppearance( int index ) { return 0; }
FPSCR void SteamSetCollision ( int index, int state ) {}
FPSCR int SteamGetCollisionList() { return 0; }
FPSCR void SteamGetNextCollision() {}
FPSCR int SteamGetCollisionIndex() { return 0; }
FPSCR int SteamGetCollisionState() { return 0; }
FPSCR void SteamShoot () {}
FPSCR int SteamGetShoot ( int index ) { return 0; }
FPSCR void SteamPlayAnimation ( int index, int start, int end, int speed ) {}
FPSCR int SteamGetAnimationList() { return 0; }
FPSCR void SteamGetNextAnimation() {}
FPSCR int SteamGetAnimationIndex() { return 0; }
FPSCR int SteamGetAnimationStart() { return 0; }
FPSCR int SteamGetAnimationEnd() { return 0; }
FPSCR int SteamGetAnimationSpeed() { return 0; }
FPSCR void SteamSetTweening(int index , int flag) {}
FPSCR LPSTR SteamGetLobbyUserName( int index) { return "hello"; }
FPSCR void SteamLeaveLobby() {}
FPSCR void SteamCreateWorkshopItem() {}
FPSCR void SteamCreateWorkshopItem( LPSTR pString ) {}
FPSCR void SteamDownloadWorkshopItem( LPSTR pString ) {}
FPSCR int SteamIsWorkshopItemDownloaded() { return 0; }
FPSCR int SteamIsWorkshopItemUploaded() { return 0; }
FPSCR LPSTR SteamGetWorkshopID(void) { return "hello"; }
FPSCR LPSTR SteamGetWorkshopItemPath(void) { return "hello"; }
FPSCR void SteamGetWorkshopItemPathDLL(LPSTR string ) {}
FPSCR int SteamIsWorkshopLoadingOnDLL() { return 0; }
FPSCR void SteamSetRoot(LPSTR string ) {}
FPSCR void SteamSetLobbyName(LPSTR name ) {}
FPSCR int SteamIsWorkshopItemInstalled(LPSTR idString ) { return 0; }
FPSCR int SteamHasSubscriptionWorkshopItemFinished() { return 0; }
FPSCR void SteamSendMyName() {}
FPSCR int SteamIsOverlayActive() { return 0; }
FPSCR void SteamWorkshopModeOff() {}
FPSCR void SteamShowAgreement() {}
FPSCR void SteamEndGame() {}
FPSCR void SteamSendChat( LPSTR msg ) {}
FPSCR void SteamSendLobbyChat( LPSTR msg ) {}
FPSCR LPSTR SteamGetChat( void ) { return "hello"; }
FPSCR int SteamInkey( int scancode ) { return 0; }
FPSCR int SteamCheckSyncedAvatarTexturesWithServer() { return 0; }
FPSCR void SteamSetMyAvatarHeadTextureName(LPSTR sAvatarTexture) {}
void SteamAvatarClient() {}
void SteamAvatarServer() {}
FPSCR int SteamStrCmp( LPSTR s1, LPSTR s2 ) { return 0; }

#define PI 3.14159

float CosineInterpolate( float y1,float y2, float mu)
{
   float mu2;
   mu2 = (float)((1-cos(mu*PI))/2);
   return(y1*(1-mu2)+y2*mu2);
}

float CosineInterpolateAngle( float y1,float y2, float mu)
{
	while (y1 > 360 ) y1-=360;
	while (y2 > 360 ) y2-=360;
	while (y1 < 0 ) y1+=360;
	while (y2 < 0 ) y2+=360;

	float difference = abs(y2 - y1);
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (y2 > y1)
		{
			// We'll add it on to start...
			y1 += 360;
		}
		else
		{
			// Add it on to end.
			y2 += 360;
		}
	}

	float mu2;
	mu2 = (float)((1-cos(mu*PI))/2);
	return(y1*(1-mu2)+y2*mu2);
}

/*
float CosineInterpolateAngle2( float y1,float y2, float mu)
{
	while (y1 > 360 ) y1-=360;
	while (y2 > 360 ) y2-=360;
	while (y1 < 0 ) y1+=360;
	while (y2 < 0 ) y2+=360;

	float difference = abs(y2 - y1);
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (y2 > y1)
		{
			// We'll add it on to start...
			y1 += 360;
		}
		else
		{
			// Add it on to end.
			y2 += 360;
		}
	}

    // Interpolate it.
    float value = (y1 + ((y2 - y1) * mu));

    // Wrap it..
    float rangeZero = 360;

    //if (value >= 0 && value <= 360)
        return value;

   // return fmod (value, rangeZero);
}

//-----------------------------------------------------------------------------
// Purpose: Extracts some feature from the command line
//-----------------------------------------------------------------------------
void ParseCommandLine( const char *pchCmdLine, const char **ppchServerAddress, const char **ppchLobbyID, bool *pbUseVR );

//======================================================

#define ACH_PLAYED_A_GAME( id, name ) { id, #id, name, "", 0, 0 }

//=========================================================================================================//

class CSteamUserGeneratedWorkshopItem
{
public:

	CSteamUserGeneratedWorkshopItem();
	~CSteamUserGeneratedWorkshopItem();

	void CreateWorkshopItem();
	void CreateWorkshopItem( LPSTR pString );
	void DownloadWorkshopitem( uint64 id );

	bool isItemCreated;
	int isItemDownloaded;
	int isItemUploaded;
	int isItemSubscribed;

	void OnWorkshopItemCreated( CreateItemResult_t *pCallback, bool bIOFailure );
	void OnWorkshopItemUpdated( SubmitItemUpdateResult_t *pCallback, bool bIOFailure );
	void OnWorkshopItemSubscribed( RemoteStorageSubscribePublishedFileResult_t  *pCallback, bool bIOFailure );
	void OnWorkshopItemDownloaded( ItemInstalled_t  *pCallback, bool bIOFailure );
	void OnWorkshopItemDownloadDone( DownloadItemResult_t  *pCallback );
	void onWorkshopItemQueried( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure );

	CCallResult<CSteamUserGeneratedWorkshopItem, CreateItemResult_t> m_SteamCallResultWorkshopItemCreated;
	CCallResult<CSteamUserGeneratedWorkshopItem, SubmitItemUpdateResult_t> m_SteamCallResultWorkshopItemUpdated;
	CCallResult<CSteamUserGeneratedWorkshopItem, RemoteStorageSubscribePublishedFileResult_t> m_SteamCallResultWorkshopItemSubscribed;
	CCallResult<CSteamUserGeneratedWorkshopItem, ItemInstalled_t> m_SteamCallResultWorkshopItemDownloaded;
	CCallResult<CSteamUserGeneratedWorkshopItem, SteamUGCQueryCompleted_t> m_SteamCallResultWorkshopItemQueried;
	// new sdk
	CCallback<CSteamUserGeneratedWorkshopItem, DownloadItemResult_t > m_SteamCallResultWorkshopItemDownloadDone; // from the new sdk

};

void CSteamUserGeneratedWorkshopItem::onWorkshopItemQueried( SteamUGCQueryCompleted_t *pCallback, bool bIOFailure )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "CSteamUserGeneratedWorkshopItem::onWorkshopItemQueried" );
#endif
	// NEED TO HANDLE FAILURE???

	// if the item does not exist, we make a new workshop item
	if ( pCallback->m_unNumResultsReturned == 0 )
	{
		SteamAPICall_t ret;

		if ( !OnlineMultiplayerModeForSharingFiles )
			ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeCommunity );	
		else
			ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeGameManagedItem );

		m_SteamCallResultWorkshopItemCreated.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemCreated );
	}
	// if the item does exist, we update the existing instead
	else if ( pCallback->m_unNumResultsReturned > 0 )
	{
		SteamUGCDetails_t details;
		bool found = false;
		for ( unsigned int c = 0; c < pCallback->m_unNumResultsReturned ; c++ )
		{
			bool res = SteamUGC()->GetQueryUGCResult( pCallback->m_handle, c , &details );
			
			if ( strcmp ( details.m_rgchTitle , workshopItemName ) == 0 )
			{
				if ( details.m_eVisibility == k_ERemoteStoragePublishedFileVisibilityPublic && !details.m_bBanned )
				{
					WorkShopItemID = details.m_nPublishedFileId;
					isItemCreated = true;
					found = true;
					break;
				}
			}
		}

		if ( !found )
		{
			SteamAPICall_t ret;

			if ( !OnlineMultiplayerModeForSharingFiles )
				ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeCommunity );	
			else
				ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeGameManagedItem );

			m_SteamCallResultWorkshopItemCreated.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemCreated );
			return;
		}

		UGCUpdateHandle_t WorkShopItemUpdateHandle = NULL;
		WorkShopItemUpdateHandle = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID() , WorkShopItemID );

		bool result;
		result = SteamUGC()->SetItemTitle( WorkShopItemUpdateHandle, workshopItemName );
		result = SteamUGC()->SetItemDescription( WorkShopItemUpdateHandle, "Game Guru Level" );
		result = SteamUGC()->SetItemVisibility( WorkShopItemUpdateHandle, k_ERemoteStoragePublishedFileVisibilityPublic );

		char** pMyTags = new char* [ 3 ];

		for ( int c = 0 ; c < 3 ; c++ )
			pMyTags [ c ] = new char [ 128 ];

		strcpy ( pMyTags[0], "Game Guru" );
		strcpy ( pMyTags[1], "FPS" );
		strcpy ( pMyTags[2], "Level" );

		SteamParamStringArray_t tags;

		tags.m_nNumStrings = 3;
		tags.m_ppStrings = (const char**) pMyTags;

		result = SteamUGC()->SetItemTags( WorkShopItemUpdateHandle, &tags );

		for ( int c = 0 ; c < 3 ; c++ )
			delete [] pMyTags [ c ];

		delete [] pMyTags;

		char tempString[MAX_PATH];
		sprintf ( tempString, "%seditors\\workshopItem" , steamRootPath );
		result = SteamUGC()->SetItemContent( WorkShopItemUpdateHandle , tempString );
		sprintf ( tempString, "%seditors\\gfx\\workshopItem.png" , steamRootPath );
		result = SteamUGC()->SetItemPreview( WorkShopItemUpdateHandle, tempString );

		SteamAPICall_t ret = SteamUGC()->SubmitItemUpdate( WorkShopItemUpdateHandle , "Updated Version" );
		m_SteamCallResultWorkshopItemUpdated.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemUpdated );
	}

}

void CSteamUserGeneratedWorkshopItem::OnWorkshopItemUpdated( SubmitItemUpdateResult_t *pCallback, bool bIOFailure )
{
	// NEED TO CHECK IF IT WAS SUCESS OR FAIL <============================
	if ( pCallback->m_eResult == k_EResultOK ) //0x1951f420 {m_eResult=k_EResultOK (1) m_bUserNeedsToAcceptWorkshopLegalAgreement=false }
		isItemUploaded = 1;
	else
		isItemUploaded = -1;
}

void CSteamUserGeneratedWorkshopItem::DownloadWorkshopitem ( uint64 id )
{
	// first we subscribe
	isItemDownloaded = 0;
	isItemSubscribed = 0;
	WorkshopItemToDownloadID = id;
	if ( !OnlineMultiplayerModeForSharingFiles )
	{
		SteamAPICall_t ret = SteamUGC()->SubscribeItem ( id );
		m_SteamCallResultWorkshopItemSubscribed.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemSubscribed );
		m_SteamCallResultWorkshopItemDownloaded.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemDownloaded );
	}
	else
	{
		uint32 ret = SteamUGC()->GetItemState ( id );		
		if ( !(ret & k_EItemStateInstalled) || ret & k_EItemStateNeedsUpdate )
		{
			ret = SteamUGC()->DownloadItem ( id , true );			
			m_SteamCallResultWorkshopItemDownloadDone.Register( this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemDownloadDone );
		}
		else
		{
			isItemDownloaded = 1;
			char from[MAX_PATH];
			char to[MAX_PATH];
			sprintf ( from , "%s\\editors_gridedit___multiplayerlevel__.fpm" , WorkshopItemPath );
			sprintf ( to , "%seditors\\gridedit\\__multiplayerlevel__.fpm" , steamRootPath );
			DeleteFile ( to );
			CopyFile ( from , to , false );
			IsWorkshopLoadingOn = 1;
		}
	}
}

void CSteamUserGeneratedWorkshopItem::OnWorkshopItemDownloadDone( DownloadItemResult_t  *pCallback )
{

	if ( pCallback->m_nPublishedFileId == WorkshopItemToDownloadID )
	{
		uint64 unSizeOnDisk = 0;
		strcpy ( WorkshopItemPath , "" );

		if ( pCallback->m_eResult != k_EResultOK )
		{
			isItemDownloaded = -1;			
			return;
		}

		char from[MAX_PATH];
		char to[MAX_PATH];
		sprintf ( from , "%s\\editors_gridedit___multiplayerlevel__.fpm" , WorkshopItemPath );
		sprintf ( to , "%seditors\\gridedit\\__multiplayerlevel__.fpm" , steamRootPath );
		DeleteFile ( to );
		CopyFile ( from , to , false );
		IsWorkshopLoadingOn = 1;

		isItemDownloaded = 1;

	}

}

void CSteamUserGeneratedWorkshopItem::OnWorkshopItemDownloaded( ItemInstalled_t *pCallback, bool bIOFailure )
{
	int a = 0;
	if ( pCallback->m_nPublishedFileId == WorkshopItemToDownloadID )
	{
		uint64 unSizeOnDisk = 0;
		strcpy ( WorkshopItemPath , "" );

		//bool legacySupport = false;
		//if ( !SteamUGC()->GetItemInstallInfo( WorkshopItemToDownloadID, &unSizeOnDisk, WorkshopItemPath, sizeof(WorkshopItemPath) , &legacySupport ) )
		uint32 timestamp;
		if ( !SteamUGC()->GetItemInstallInfo( WorkshopItemToDownloadID, &unSizeOnDisk, WorkshopItemPath, sizeof(WorkshopItemPath) , &timestamp ) )
		{
			isItemDownloaded = -1;			
			return;
		}

		char from[MAX_PATH];
		char to[MAX_PATH];
		sprintf ( from , "%s\\editors_gridedit___multiplayerlevel__.fpm" , WorkshopItemPath );
		sprintf ( to , "%seditors\\gridedit\\__multiplayerlevel__.fpm" , steamRootPath );
		DeleteFile ( to );
		CopyFile ( from , to , false );
		IsWorkshopLoadingOn = 1;

		isItemDownloaded = 1;

	}
}

void CSteamUserGeneratedWorkshopItem::OnWorkshopItemSubscribed( RemoteStorageSubscribePublishedFileResult_t *pCallback, bool bIOFailure )
{
	if ( pCallback->m_nPublishedFileId == WorkshopItemToDownloadID )
	{	
		// NEED TO CHECK IF IT WAS SUCESS OR FAIL
		if ( pCallback->m_eResult == k_EResultOK && bIOFailure == false )
			isItemSubscribed = 1;
		else
			isItemSubscribed = -1;
	}
}

void CSteamUserGeneratedWorkshopItem::OnWorkshopItemCreated( CreateItemResult_t *pCallback, bool bIOFailure )
{
	WorkShopItemID = 0;

	if ( pCallback->m_bUserNeedsToAcceptWorkshopLegalAgreement )
		ShellExecuteW( NULL, L"open", L"http://steamcommunity.com/sharedfiles/workshoplegalagreement" , NULL, NULL, SW_SHOWMAXIMIZED );

	if ( !bIOFailure )
	{
		WorkShopItemID = pCallback->m_nPublishedFileId;
		isItemCreated = true;

		UGCUpdateHandle_t WorkShopItemUpdateHandle = NULL;
		WorkShopItemUpdateHandle = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID() , WorkShopItemID );

		bool result;
		result = SteamUGC()->SetItemTitle( WorkShopItemUpdateHandle, workshopItemName );
		result = SteamUGC()->SetItemDescription( WorkShopItemUpdateHandle, "Game Guru Level" );
		result = SteamUGC()->SetItemVisibility( WorkShopItemUpdateHandle, k_ERemoteStoragePublishedFileVisibilityPublic );

		char** pMyTags = new char* [ 3 ];

		for ( int c = 0 ; c < 3 ; c++ )
			pMyTags [ c ] = new char [ 128 ];

		strcpy ( pMyTags[0], "Game Guru" );
		strcpy ( pMyTags[1], "FPS" );
		strcpy ( pMyTags[2], "Level" );

		SteamParamStringArray_t tags;

		tags.m_nNumStrings = 3;
		tags.m_ppStrings = (const char**) pMyTags;

		result = SteamUGC()->SetItemTags( WorkShopItemUpdateHandle, &tags );

		for ( int c = 0 ; c < 3 ; c++ )
			delete [] pMyTags [ c ];

		delete [] pMyTags;

		char tempString[MAX_PATH];
		sprintf ( tempString, "%seditors\\workshopItem" , steamRootPath );
		result = SteamUGC()->SetItemContent( WorkShopItemUpdateHandle , tempString );
		sprintf ( tempString, "%seditors\\gfx\\workshopItem.png" , steamRootPath );
		result = SteamUGC()->SetItemPreview( WorkShopItemUpdateHandle, tempString );

		SteamAPICall_t ret = SteamUGC()->SubmitItemUpdate( WorkShopItemUpdateHandle , "First Version" );
		m_SteamCallResultWorkshopItemUpdated.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemUpdated );


	}
}

CSteamUserGeneratedWorkshopItem::CSteamUserGeneratedWorkshopItem():m_SteamCallResultWorkshopItemDownloadDone(NULL,NULL)
{
}

CSteamUserGeneratedWorkshopItem::~CSteamUserGeneratedWorkshopItem() 
{
}


void CSteamUserGeneratedWorkshopItem::CreateWorkshopItem()
{
	isItemCreated = false;
	isItemUploaded = 0;
	SteamAPICall_t ret;

	if ( !OnlineMultiplayerModeForSharingFiles )
		ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeCommunity );	
	else
		ret = SteamUGC()->CreateItem ( SteamUtils()->GetAppID() , k_EWorkshopFileTypeGameManagedItem );

	m_SteamCallResultWorkshopItemCreated.Set( ret, this, &CSteamUserGeneratedWorkshopItem::OnWorkshopItemCreated );
}

void CSteamUserGeneratedWorkshopItem::CreateWorkshopItem( LPSTR pString )
{
	isItemCreated = false;
	isItemUploaded = 0;

	strcpy ( workshopItemName , pString );
	
	UGCQueryHandle_t handle;

	if ( !OnlineMultiplayerModeForSharingFiles )
	{
		handle = SteamUGC()->CreateQueryUserUGCRequest( SteamUser()->GetSteamID().GetAccountID(), k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items, 
			k_EUserUGCListSortOrder_LastUpdatedDesc, SteamUtils()->GetAppID(), SteamUtils()->GetAppID(), 1 );
	}
	else
	{
		handle = SteamUGC()->CreateQueryUserUGCRequest( SteamUser()->GetSteamID().GetAccountID(), k_EUserUGCList_Published, k_EUGCMatchingUGCType_GameManagedItems, 
			k_EUserUGCListSortOrder_LastUpdatedDesc, SteamUtils()->GetAppID(), SteamUtils()->GetAppID(), 1 );
	}

	SteamUGC()->SetSearchText( handle, pString );

	SteamAPICall_t ret = SteamUGC()->SendQueryUGCRequest ( handle );

	m_SteamCallResultWorkshopItemQueried.Set( ret, this, &CSteamUserGeneratedWorkshopItem::onWorkshopItemQueried );
}

CSteamUserGeneratedWorkshopItem g_UserWorkShopItem;


 FPSCR int SteamInit()
 {
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamInit" );
#endif

#ifdef _DEBUG_LOG_
	logStart();
	log("SteamInit()" );
#endif

	SteamCleanupClient();

	ResetGameStats();

	 g_pClient = NULL;
	 
	if ( SteamAPI_Init() )
	{
		// must be logged into steam, which will be the case if run from within steam.
		if ( !SteamUser()->BLoggedOn() )
		{
#ifdef _DEBUG_LOG_
	log("SteamInit() Failed - SteamUser()->BLoggedOn(), not logged on" );
#endif
			return 0;
		}

#ifdef _DEBUG_LOG_
	log("SteamInit() Pass - SteamUser()->BLoggedOn(), is logged on" );
#endif

		g_SteamRunning = true;

		// Start up a steam client
#ifdef _DEBUG_LOG_
	log("SteamInitClient()" );
#endif
		SteamInitClient();
		
		return 1;
	}
	else
	{
#ifdef _DEBUG_LOG_
	log("SteamInit() Failed" );
#endif
		return 0;
	}
 }

 FPSCR void SteamFree()
 {
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - early exit" );
#endif
	 if ( !g_SteamRunning ) return;

#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - past early exit" );
#endif
	

	//SAFE_DELETE ( g_SteamAchievements );
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - Deleting Client Start" );
#endif
	SAFE_DELETE ( g_pClient );
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - Deleting Client End" );
#endif

#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - API Shutdown start" );
#endif
	 SteamAPI_Shutdown();
	 g_SteamRunning = false;
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamFree - API SHutdown end" );
#endif

	 IsWorkshopLoadingOn = 0;
 }

 FPSCR bool SteamOwned ( void )
 {
	 bool bGameGuruOwned = SteamApps()->BIsSubscribed();
	 bool bGameGuruOwnedFromFreeWeekend = SteamApps()->BIsSubscribedFromFreeWeekend();
	 if ( bGameGuruOwned == true || bGameGuruOwnedFromFreeWeekend == true )
		 return true;
	 else
		 return false;
 }

 //=========================================================================================================//

 FPSCR LPSTR SteamGetPlayerName(void)
 {
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerName" );
#endif
	 if ( g_SteamRunning )
	 {
		const char *playerName = SteamFriends()->GetPersonaName();

		if (playerName) 
		{
			return GetReturnStringFromTEXTWorkString((char*)playerName);
		}
	 }
	return "";
 }

 //=========================================================================================================//

  FPSCR LPSTR SteamGetOtherPlayerName( int index )
 {
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetOtherPlayerName" );
#endif

	 if ( g_SteamRunning && index < MAX_PLAYERS_PER_SERVER )
	 {
		return GetReturnStringFromTEXTWorkString ( Client()->m_rgpPlayerName[index] );
	 }
	return "";
 } 

 //=========================================================================================================//

 FPSCR LPSTR SteamGetPlayerID(void)
 {
#ifdef _DEBUG_LOG_
	log("======== SteamGetPlayerID() ========");
#endif

#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerID - entered" );
#endif

	CSteamID steamID;
	UINT64 SteamID64 = 0;

#ifdef _DEBUG_LOG_
	log("Check if steam is running");
#endif

	if ( !g_SteamRunning )
	{
#ifdef _DEBUG_LOG_
	log("Steam NOT running, performing free and init");
#endif
		SteamFree();
		SteamInit();
	}

#ifdef _DEBUG_LOG_
	log("g_SteamRunning = " , int(g_SteamRunning));
#endif
	 if ( g_SteamRunning )
	 {
#ifdef _DEBUG_LOG_
		log("Steam is running");
#endif
		 steamID = SteamUser()->GetSteamID();
		 SteamID64 = steamID.GetAccountID() + 76561197960265728;

#ifdef _DEBUG_LOG_
		log("** steamID.GetAccountID() = ", steamID.GetAccountID() );
#endif
	 }
#ifdef _DEBUG_LOG_
	 else
	 {
		log("Steam NOT running");
	 }
#endif

#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerID - make string" );
#endif
	// Create a new string and copy input string to it
	char steamIDString[128];
	sprintf ( steamIDString , "%llu" , SteamID64 );
#ifdef _DEBUG_LOG_
	log("** SteamID64:" );
	log(steamIDString);
#endif

	return GetReturnStringFromTEXTWorkString ( steamIDString );

 }

 //=========================================================================================================//

 FPSCR void SteamLoop(void)
 {
	if ( g_SteamRunning )
	{

		if ( !SteamUser()->BLoggedOn() )
		{
			g_SteamRunning = false;
			return;
		}

		SteamAPI_RunCallbacks(); 
	}
	else
	{
		return;
	}
	
	Client()->timeElapsed = GetCounterPassed();

	// Run a game frame
	Client()->RunFrame();

	// Keep running the network on the client at a faster rate than the FPS limit
	Client()->ReceiveNetworkData();

 }

 //=========================================================================================================//

 FPSCR int SteamIsOnline(void)
 {
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamIsOnline" );
#endif
	 if ( SteamUser()->BLoggedOn() )
		 return 1;
	 else
		 return 0;
 }

 //=========================================================================================================//

 FPSCR void SteamAddAch(void)
 {
 }

 //=========================================================================================================//

 //-----------------------------------------------------------------------------
// Purpose: Extracts some feature from the command line
//-----------------------------------------------------------------------------
void ParseCommandLine( const char *pchCmdLine, const char **ppchServerAddress, const char **ppchLobbyID, bool *pbUseVR )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "ParseCommandLine" );
#endif
	// Look for the +connect ipaddress:port parameter in the command line,
	// Steam will pass this when a user has used the Steam Server browser to find
	// a server for our game and is trying to join it. 
	const char *pchConnectParam = "+connect";
	const char *pchConnect = strstr( pchCmdLine, pchConnectParam );
	*ppchServerAddress = NULL;
	if ( pchConnect && strlen( pchCmdLine ) > (pchConnect - pchCmdLine) + strlen( pchConnectParam ) + 1 )
	{
		// Address should be right after the +connect, +1 on the end to skip the space
		*ppchServerAddress = pchCmdLine + ( pchConnect - pchCmdLine ) + strlen( pchConnectParam ) + 1;
	}

	// look for +connect_lobby lobbyid paramter on the command line
	// Steam will pass this in if a user taken up an invite to a lobby
	const char *pchConnectLobbyParam = "+connect_lobby";
	const char *pchConnectLobby = strstr( pchCmdLine, pchConnectParam );
	*ppchLobbyID = NULL;
	if ( pchConnectLobby && strlen( pchCmdLine ) > (pchConnectLobby - pchCmdLine) + strlen( pchConnectLobbyParam ) + 1 )
	{
		// Address should be right after the +connect, +1 on the end to skip the space
		*ppchLobbyID = pchCmdLine + ( pchConnectLobby - pchCmdLine ) + strlen( pchConnectLobbyParam ) + 1;
	}

	// look for -vr on command line. Switch to VR mode if it's thre
	const char *pchVRParam = "-vr";
	const char *pchVR = strstr( pchCmdLine, pchVRParam );
	if ( pchVR )
		*pbUseVR = true;
}

void SteamInitClient()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamInitClient" );
#endif

	// set our debug handler
#ifdef _DEBUG_LOG_
	log("SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );" );
#endif
	SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

	// Tell Steam where it's overlay should show notification dialogs, this can be top right, top left,
	// bottom right, bottom left. The default position is the bottom left if you don't call this.  
	// Generally you should use the default and not call this as users will be most comfortable with 
	// the default position.  The API is provided in case the bottom right creates a serious conflict 
	// with important UI in your game.
#ifdef _DEBUG_LOG_
	log("SteamUtils()->SetOverlayNotificationPosition( k_EPositionTopLeft );" );
#endif
	SteamUtils()->SetOverlayNotificationPosition( k_EPositionTopLeft );

	// We are going to use the controller interface, initialize it, which is a seperate step as it 
	// create a new thread in the game proc and we don't want to force that on games that don't
	// have native Steam controller implementations

	bool bUseVR = SteamUtils()->IsSteamRunningInVR();
#ifdef _DEBUG_LOG_
	log("SteamUtils()->IsSteamRunningInVR() = ", int(bUseVR) );
#endif
	const char *pchServerAddress, *pchLobbyID;
	ParseCommandLine( "", &pchServerAddress, &pchLobbyID, &bUseVR );

	// do a DRM self check
	Steamworks_SelfCheck();

	//=====================================================

	// Initialize the game
	g_pClient = new CClient();
	StartCounter();

	// If +connect was used to specify a server address, connect now
	g_pClient->ExecCommandLineConnect( pchServerAddress, pchLobbyID );

	// test a user specific secret before entering main loop
	Steamworks_TestSecret();

	g_pClient->RetrieveEncryptedAppTicket();

	g_SteamRunning = true;

}

FPSCR void SteamCreateLobby()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamCreateLobby" );
#endif
	isPlayingOnAServer = false;
	if ( g_SteamRunning )
	{
		messageList.clear();
		chatList.clear();
		deleteList.clear();
		deleteListSource.clear();
		destroyList.clear();
		lobbyChatIDs.clear();

		syncedAvatarTextureMode = SYNC_AVATAR_TEX_BEGIN;
		syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_BEGIN;

		PacketSend_Log_Client.clear();
		PacketSend_Log_Server.clear();
		PacketSendReceipt_Log_Client.clear();
		PacketSendReceipt_Log_Server.clear();

		ClientDeathNumber = 1;
		for ( int c = 0 ; c < MAX_PLAYERS_PER_SERVER ; c++ )
			ServerClientLastDeathNumber[c] = 0;

		server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		Client()->SteamCreateLobby();
	}
}

FPSCR int SteamIsLobbyCreated()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamIsLobbyCreated" );
#endif
	if ( g_SteamRunning )
	{
		return Client()->SteamIsLobbyCreated();
	}
	return 0;
}

FPSCR void SteamGetLobbyList()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLobbyList" );
#endif
	isPlayingOnAServer = false;
	if ( g_SteamRunning )
	{
		server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		Client()->SteamGetLobbyList();
	}
}

FPSCR int SteamIsLobbyListCreated()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamIsLobbyListCreated" );
#endif
	if ( g_SteamRunning )
		return Client()->SteamIsLobbyListCreated();
	return 0;
}

FPSCR int SteamGetLobbyListSize()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLobbyListSize" );
#endif
	if ( g_SteamRunning )
		return Client()->SteamGetLobbyListSize();
	return 0;
}

FPSCR LPSTR SteamGetLobbyListName( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLobbyListName" );
#endif
	if ( g_SteamRunning )
		return Client()->SteamGetLobbyListName( index );
	return NULL;
}

FPSCR void SteamJoinLobby( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamJoinLobby" );
#endif
	if ( g_SteamRunning )
	{
		messageList.clear();
		chatList.clear();
		deleteList.clear();
		deleteListSource.clear();
		destroyList.clear();
		lobbyChatIDs.clear();

		syncedAvatarTextureMode = SYNC_AVATAR_TEX_BEGIN;
		syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_BEGIN;

		PacketSend_Log_Client.clear();
		PacketSend_Log_Server.clear();
		PacketSendReceipt_Log_Client.clear();
		PacketSendReceipt_Log_Server.clear();

		ClientDeathNumber = 1;
		for ( int c = 0 ; c < MAX_PLAYERS_PER_SERVER ; c++ )
			ServerClientLastDeathNumber[c] = 0;

		server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		return Client()->SteamJoinLobby( index );
	}
}

FPSCR int SteamGetLobbyUserCount()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLobbyUserCount" );
#endif
	if ( Client() ) 
	{
		if ( Client()->m_pServer == NULL )
			HowManyPlayersDoWeHave = Client()->SteamGetUsersInLobbyCount();
		return Client()->SteamGetUsersInLobbyCount();
	}
	else
		return 0;
}

FPSCR int SteamHasJoinedLobby()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamHasJoinedLobby" );
#endif
	if ( g_SteamRunning )
		return Client()->SteamHasJoinedLobby();
	return 0;
}

FPSCR void SteamStartServer()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamStartServer" );
#endif
#ifdef _DEBUG_LOG_
	log("SteamStartServer()" );
#endif

	if ( g_SteamRunning )
	{
		server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;
		Client()->SteamStartServer();
		serverActive = true;
	}
}

FPSCR int SteamIsServerRunning()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamIsServerRunning" );
#endif
#ifdef _DEBUG_LOG_
	//log("SteamIsServerRunning()" , Client()->SteamIsServerRunning() );
#endif

	if ( g_SteamRunning )
	{
		return Client()->SteamIsServerRunning();
	}
	return 0;

}

FPSCR int SteamIsGameRunning()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamIsGameRunning" );
#endif
#ifdef _DEBUG_LOG_
	//log("SteamIsGameRunning()" , Client()->SteamIsGameRunning() );
#endif

	if ( g_SteamRunning )
	{
		int ret = Client()->SteamIsGameRunning();
		serverActive = 0;
		if ( ret == 1 )
			serverActive = 1;
		return ret;
	}
	return 0;
}

FPSCR int SteamGetMyPlayerIndex()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetMyPlayerIndex" );
#endif
	if ( g_SteamRunning )
		return Client()->SteamGetMyPlayerIndex();

	return 0;
}

FPSCR void SteamSetPlayerPositionX( float _x)
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetMyPlayerIndex" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerPositionX( _x );
}

FPSCR void SteamSetPlayerPositionY( float _y )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSetPlayerPositionY" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerPositionY( _y );
}

FPSCR void SteamSetPlayerPositionZ( float _z )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSetPlayerPositionZ" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerPositionZ( _z );
}

FPSCR void SteamSetPlayerAngle( float _angle )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSetPlayerAngle" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerAngle( _angle );
}

FPSCR void SteamSetPlayerScore( int index, int score )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSetPlayerScore" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerScore( index, score );
}

FPSCR float SteamGetPlayerPositionX ( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerPositionX" );
#endif
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetPlayerPositionX ( index );
		return fValue;
	}

	return 0;
}

FPSCR float SteamGetPlayerPositionY ( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerPositionY" );
#endif
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetPlayerPositionY ( index );
		return fValue;
	}

	return 0;
}

FPSCR float SteamGetPlayerPositionZ ( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerPositionZ" );
#endif
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetPlayerPositionZ ( index );
		return fValue;
	}

	return 0;
}

FPSCR float SteamGetPlayerAngle ( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerAngle" );
#endif
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetPlayerAngle ( index );
		return fValue;
	}

	return 0;
}

FPSCR int SteamGetPlayerScore ( int index )
{
	if ( g_SteamRunning )
	{
		return Client()->SteamGetPlayerScore ( index );
	}

	return 0;
}

FPSCR void SteamSetBullet ( int index , float x , float y , float z, float anglex, float angley, float anglez, int type, int on )
{
	if ( g_SteamRunning )
	{
		Client()->SteamSetBullet ( index , x , y , z, anglex, angley, anglez, type, on );
	}
}

FPSCR int SteamGetBulletOn ( int index )
{
	if ( g_SteamRunning )
	{
		return Client()->SteamGetBulletOn ( index );
	}

	return 0;
}

FPSCR int SteamGetBulletType ( int index )
{
	if ( g_SteamRunning )
	{
		return Client()->SteamGetBulletType ( index );
	}

	return 0;
}

FPSCR float SteamGetBulletX ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletX ( index );
		return fValue;
	}
	return 0;
}

FPSCR float SteamGetBulletY ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletY ( index );
		return fValue;
	}
	return 0;
}

FPSCR float SteamGetBulletZ ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletZ ( index );
		return fValue;
	}
	return 0;
}

FPSCR float SteamGetBulletAngleX ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletAngleX ( index );
		return fValue;
	}
	return 0;
}

FPSCR float SteamGetBulletAngleY ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletAngleY ( index );
		return fValue;
	}
	return 0;
}

FPSCR float SteamGetBulletAngleZ ( int index )
{
	if ( g_SteamRunning )
	{
		float fValue = Client()->SteamGetBulletAngleZ ( index );
		return fValue;
	}
	return 0;
}

FPSCR void SteamSetKeyState ( int key , int state )
{
	if ( g_SteamRunning )
		Client()->SteamSetKeyState(key,state);
}

FPSCR int SteamGetKeyState ( int index, int key )
{
	if ( g_SteamRunning )
		return keystate[index][key];

	return 0;
}

FPSCR void SteamApplyPlayerDamage ( int index, int damage, int x, int y, int z, int force, int limb )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamApplyPlayerDamage" );
#endif
	if ( g_SteamRunning )
		Client()->SteamApplyPlayerDamage( index , damage, x, y, z, force, limb );
}

FPSCR int SteamGetPlayerDamageSource()
{
	return damageSource;
}

FPSCR int SteamGetPlayerDamageX()
{
	return damageX;
}

FPSCR int SteamGetPlayerDamageY()
{
	return damageY;
}

FPSCR int SteamGetPlayerDamageZ()
{
	return damageZ;
}

FPSCR int SteamGetPlayerDamageForce()
{
	return damageForce;
}

FPSCR int SteamGetPlayerDamageLimb()
{
	return damageLimb;
}

FPSCR int SteamGetPlayerKilledSource( int index )
{
	return killedSource[index];
}

FPSCR int SteamGetPlayerKilledX( int index )
{
	return killedX[index];
}

FPSCR int SteamGetPlayerKilledY( int index )
{
	return killedY[index];
}

FPSCR int SteamGetPlayerKilledZ( int index )
{
	return killedZ[index];
}

FPSCR int SteamGetPlayerKilledForce( int index )
{
	return killedForce[index];
}

FPSCR int SteamGetPlayerKilledLimb( int index )
{
	return killedLimb[index];
}

FPSCR void SteamKilledBy ( int killedBy, int x, int y, int z, int force, int limb )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamKilledBy" );
#endif
	if ( g_SteamRunning )
		Client()->SteamKilledBy( killedBy, x, y, z, force, limb );
}

FPSCR void SteamKilledSelf()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamKilledSelf" );
#endif
	if ( g_SteamRunning )
		Client()->SteamKilledSelf();
}

char ServerMessage[256];

FPSCR LPSTR SteamGetServerMessage( void )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetServerMessage" );
#endif

	if ( messageList.size() > 0 )
	{
		strcpy(ServerMessage , (const char*)messageList[0].message);
		messageList.erase( messageList.begin() );

		return ServerMessage;		
	}

	return "";
} 

FPSCR int SteamGetPlayerDamageAmount ()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerDamageAmount" );
#endif
	if ( g_SteamRunning )
	{	
		int result = playerDamage;
		playerDamage = 0;
		return result;
	}
	return 0;
}

FPSCR int SteamGetClientServerConnectionStatus()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetClientServerConnectionStatus" );
#endif
	if ( g_SteamRunning )
	{
		int result = Client()->SteamGetClientServerConnectionStatus();

		if ( result == 0 )
		{
			SteamResetClient();
			return 0;
		}
		// 020315 - 012 - server to send quit game message
		// server closing down status
		if ( result == 2 )
		{
			SteamResetClient();
			ServerIsShuttingDown = 0;
			return 2;
		}
		else
			return 1;
	}

	return 0;
}

void SteamResetClient()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamResetClient" );
#endif
	SteamCleanupClient();

	SAFE_DELETE ( g_pClient );

	 g_pClient = NULL;

	// must be logged into steam, which will be the case if run from within steam.
	if ( !SteamUser()->BLoggedOn() )
		return;

	// Initialize the game again
	g_pClient = new CClient();

	deleteList.clear();
	deleteListSource.clear();
	destroyList.clear();

	messageList.clear();
	chatList.clear();
	lobbyChatIDs.clear();

	g_pClient->RetrieveEncryptedAppTicket();

}

void SteamCleanupClient()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamCleanupClient" );
#endif
	ResetGameStats();
}

void ResetGameStats()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "ResetGameStats" );
#endif
	serverActive = false;

	IsWorkshopLoadingOn = 0;
	ServerIsShuttingDown = 0;

	char fileToDelete[MAX_PATH];

	for ( int c = 0; c < MAX_PLAYERS_PER_SERVER ; c++ )
	{
		scores[c] = 0;
		for ( int k = 0 ; k < 256 ; k++ ) keystate[c][k];
		alive[c] = 1;
		playerAppearance[c] = 0;

		serverClientsFileSynced[c] = 0;
		serverClientsLoadedAndReady[c] = 0;
		serverClientsReadyToPlay[c] = 0;
		playerShoot[c] = 0;
		tweening[c] = 1;
		HowManyPlayersDoWeHave = 0;

		avatarFile[c] = NULL;
		avatarHowManyFileChunks[c] = 0;
		avatarFileFileSize[c] = 0;

		sprintf ( fileToDelete, "%sentitybank\\user\\charactercreator\\customAvatar_%i_cc.dds" , steamRootPath , c );
		DeleteFile ( fileToDelete );

		sprintf ( fileToDelete, "%sentitybank\\user\\charactercreator\\customAvatar_%i.fpe" , steamRootPath , c );
		DeleteFile ( fileToDelete );
	}

	for ( int i = 0 ; i < 179 ; i++ )
	{
		bullets[i].on = 0;
		for ( int a = 0 ; a < MAX_PLAYERS_PER_SERVER; a++ )
			bulletSeen[i][a] = false;
	}

	// wait a long time when inititally connecting to allow for long loading times
	server_timeout_milliseconds = SERVER_TIMEOUT_MILLISECONDS_LONG;

	ServerFilesToReceive = 0;
	ServerFilesReceived = 0;
	IamSyncedWithServerFiles = 0;
	IamLoadedAndReady = 0;
	IamReadyToPlay = 0;
	isEveryoneLoadedAndReady = 0;
	isEveryoneReadyToPlay = 0;
	serverChunkToSendCount = 0;
	fileProgress = 0;
	HowManyPlayersDoWeHave = 0;
	ServerHowManyToStart = 0;

	strcpy ( WorkshopItemPath , "" );

	if ( serverFile )
	{
		fclose (serverFile );
		serverFile = NULL;
	}

}

FPSCR void SteamSetPlayerAlive ( int state )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSetPlayerAlive" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSetPlayerAlive(state);
}

FPSCR int SteamGetPlayerAlive ( int index )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetPlayerAlive" );
#endif
	if ( g_SteamRunning )
		return alive[index];

	return 0;
}

FPSCR void SteamSpawnObject ( int obj, int source, float x, float y, float z )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSpawnObject" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSpawnObject ( obj, source, x, y, z );
}

FPSCR void SteamDeleteObject ( int obj )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamDeleteObject" );
#endif
	if ( g_SteamRunning )
		Client()->SteamDeleteObject ( obj );
}

FPSCR void SteamDestroy ( int obj )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamDestroy" );
#endif
	if ( g_SteamRunning )
		Client()->SteamDestroyObject ( obj );
}

FPSCR void SteamSendLua ( int code, int e, int v )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSendLua", code, e, v );
#endif
	if ( g_SteamRunning )
	{
		if (!serverActive) return;

		if ( Client() )
		{
			Client()->SteamSendLua ( code, e, v);
		}
	}
}

FPSCR void SteamSendLuaString ( int code, int e, LPSTR s )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSendLua", code, e, v );
#endif
	if ( g_SteamRunning )
	{
		if (!serverActive) return;

		if ( Client() )
		{
			Client()->SteamSendLuaString ( code, e, s);
		}
	}
}

FPSCR int SteamGetLuaList()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLuaList" );
#endif
	if ( luaList.size() > 0 )
	{
		currentLua.code = luaList.back().code;
		currentLua.e = luaList.back().e;
		currentLua.v = luaList.back().v;
		if ( strlen(luaList.back().s) >= CURRENT_LUA_STRING_SIZE ) MessageBox ( NULL , "Lua String Exceeded Max", "SteamMultiplayer Error", MB_OK );
		strncpy ( currentLua.s , luaList.back().s , CURRENT_LUA_STRING_SIZE );
		currentLua.s[CURRENT_LUA_STRING_SIZE-1] = 0;

		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextLua()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetNextLua" );
#endif
	if ( luaList.size() > 0 )
		luaList.pop_back();
}

FPSCR int SteamGetLuaCommand()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLuaCommand" );
#endif
	return currentLua.code;
}

FPSCR int SteamGetLuaE()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLuaE" );
#endif
	return currentLua.e;
}

FPSCR int SteamGetLuaV()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLuaV" );
#endif
	return currentLua.v;
}

FPSCR LPSTR SteamGetLuaS(void)
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetLuaV" );
#endif

	return GetReturnStringFromTEXTWorkString( currentLua.s );
}


FPSCR int SteamGetSpawnList()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetSpawnList" );
#endif
	if ( spawnList.size() > 0 )
	{
		currentSpawnObject.object = spawnList.back().object;
		currentSpawnObject.source = spawnList.back().source;
		currentSpawnObject.x = spawnList.back().x;
		currentSpawnObject.y = spawnList.back().y;
		currentSpawnObject.z = spawnList.back().z;
		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextSpawn()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetNextSpawn" );
#endif
	if ( spawnList.size() > 0 )
		spawnList.pop_back();
}

FPSCR int SteamGetSpawnObjectNumber()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetSpawnObjectNumber" );
#endif
	return currentSpawnObject.object;
}

FPSCR int SteamGetSpawnObjectSource()
{
	return currentSpawnObject.source;
}

FPSCR float SteamGetSpawnX()
{
	float fValue = currentSpawnObject.x;
	return fValue;
}

FPSCR float SteamGetSpawnY()
{
	float fValue = currentSpawnObject.y;
	return fValue;
}

FPSCR float SteamGetSpawnZ()
{
	float fValue = currentSpawnObject.z;
	return fValue;
}

FPSCR int SteamGetDeleteList()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetDeleteList" );
#endif
	if ( deleteList.size() > 0 )
	{
		currentDeleteObject = deleteList.back();
		currentDeleteObjectSource = deleteListSource.back();
		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextDelete()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetNextDelete" );
#endif
	if ( deleteList.size() > 0 )
		deleteList.pop_back();

	if ( deleteListSource.size() > 0 )
		deleteListSource.pop_back();
}

FPSCR int SteamGetDeleteObjectNumber()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetDeleteObjectNumber" );
#endif
	return currentDeleteObject;
}

FPSCR int SteamGetDeleteSource()
{
	return currentDeleteObjectSource;
}

FPSCR int SteamGetDestroyList()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetDestroyList" );
#endif
	if ( destroyList.size() > 0 )
	{
		currentDestroyObject = destroyList.back();
		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextDestroy()
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetNextDestroy" );
#endif
	if ( destroyList.size() > 0 )
		destroyList.pop_back();
}

FPSCR int SteamGetDestroyObjectNumber()
{
	return currentDestroyObject;
}

FPSCR int SteamReadyToSpawn()
{
	return ServerSaysItIsOkayToStart;
}

FPSCR void SteamSetSendFileCount(int count)
{
	if ( g_SteamRunning )
		Client()->SteamSetSendFileCount(count);
}

FPSCR void SteamSendFileBegin ( int index , LPSTR pString )
{
	if ( g_SteamRunning )
		Client()->SteamSendFileBegin( index, pString);	
}

FPSCR int SteamSendFileDone()
{
	if ( g_SteamRunning )
		return Client()->SteamSendFileDone();

	return 0;
}

FPSCR int SteamAmIFileSynced()
{
	if ( g_SteamRunning )
		return IamSyncedWithServerFiles;

	return 0;
}

FPSCR int SteamIsEveryoneFileSynced()
{
	if ( g_SteamRunning )
		return Client()->SteamIsEveryoneFileSynced();

	return 0;
}

FPSCR void SteamSendIAmLoadedAndReady()
{
	if ( g_SteamRunning )
	{
		Client()->SteamSendIAmLoadedAndReady();
		serverActive = true;
	}
}

FPSCR int SteamIsEveryoneLoadedAndReady()
{
	if ( g_SteamRunning )
	{
		Client()->ServerCheckEveryoneIsLoadedAndReady();
		return isEveryoneLoadedAndReady;
	}

	return 0;
}

FPSCR void SteamSendIAmReadyToPlay()
{
	lobbyChatIDs.clear();

	if ( g_SteamRunning )
	{
		Client()->SteamSendIAmReadyToPlay();
		char msg[80];
		sprintf ( msg , "s%s has joined the game" , SteamFriends()->GetPersonaName() );
		Client()->SteamSendChat(msg);
	}
}

FPSCR int SteamIsEveryoneReadyToPlay()
{
	lobbyChatIDs.clear();

	if ( g_SteamRunning )
	{
		Client()->ServerCheckEveryoneIsReadyToPlay();
		return isEveryoneReadyToPlay;
	}

	return 0;
}

FPSCR int SteamGetFileProgress()
{
	if ( g_SteamRunning )
		return Client()->SteamGetFileProgress();

	return 0;
}

FPSCR void SteamSetVoiceChat( int on )
{
}

FPSCR void SteamSetPlayerAppearance( int a )
{
	if ( g_SteamRunning )
		Client()->SteamSetPlayerAppearance(a);
}

FPSCR int SteamGetPlayerAppearance( int index )
{
	if ( g_SteamRunning )
		return playerAppearance[index];

	return 0;
}

FPSCR void SteamSetCollision ( int index, int state )
{
	if ( g_SteamRunning )
		Client()->SteamSetCollision( index , state );
}

FPSCR int SteamGetCollisionList()
{
	if ( collisionList.size() > 0 )
	{
		currentCollisionObject.index = collisionList.back().index;
		currentCollisionObject.state = collisionList.back().state;
		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextCollision()
{
	if ( collisionList.size() > 0 )
		collisionList.pop_back();
}

FPSCR int SteamGetCollisionIndex()
{
	return currentCollisionObject.index;
}

FPSCR int SteamGetCollisionState()
{
	return currentCollisionObject.state;
}

FPSCR void SteamShoot ()
{
	if ( g_SteamRunning )
		Client()->SteamShoot();
}

FPSCR int SteamGetShoot ( int index )
{
	int result = playerShoot[index];
	playerShoot[index] = 0;
	return result;
}

//========================================================================================

FPSCR void SteamPlayAnimation ( int index, int start, int end, int speed )
{
	if ( g_SteamRunning )
		Client()->SteamPlayAnimation ( index, start, end, speed );
}

FPSCR int SteamGetAnimationList()
{
	if ( animationList.size() > 0 )
	{
		currentAnimationObject.index = animationList.back().index;
		currentAnimationObject.start = animationList.back().start;
		currentAnimationObject.end = animationList.back().end;
		currentAnimationObject.speed = animationList.back().speed;
		return 1;
	}
	return 0;
}

FPSCR void SteamGetNextAnimation()
{
	if ( animationList.size() > 0 )
		animationList.pop_back();
}

FPSCR int SteamGetAnimationIndex()
{
	return currentAnimationObject.index;
}

FPSCR int SteamGetAnimationStart()
{
	return currentAnimationObject.start;
}

FPSCR int SteamGetAnimationEnd()
{
	return currentAnimationObject.end;
}

FPSCR int SteamGetAnimationSpeed()
{
	return currentAnimationObject.speed;
}

FPSCR void SteamSetTweening(int index , int flag)
{
	tweening[index] = flag;
}

FPSCR LPSTR SteamGetLobbyUserName( int index)
{
	if ( g_SteamRunning )
	{
		if ( lobbyIAmInID.IsValid() && lobbyIAmInID.IsLobby() )
		{
			CSteamID id = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyIAmInID,index);
			CSteamID steamIDLobbyMember = SteamFriends()->GetFriendFromSourceByIndex( lobbyIAmInID, index );
			const char *pchName = SteamFriends()->GetFriendPersonaName( steamIDLobbyMember );

			return GetReturnStringFromTEXTWorkString( (char*)pchName );
		}
	}

	return "";
}

FPSCR void SteamLeaveLobby()
{
	if ( g_SteamRunning )
	{
		if ( lobbyIAmInID.IsValid() && lobbyIAmInID.IsLobby() )
		{
			Client()->SteamLeaveLobby();
		}
	}
}

FPSCR void SteamCreateWorkshopItem()
{
	if ( g_SteamRunning )
	{
		g_UserWorkShopItem.CreateWorkshopItem();
	}
}

FPSCR void SteamCreateWorkshopItem( LPSTR pString )
{
	if ( g_SteamRunning )
	{
		g_UserWorkShopItem.CreateWorkshopItem( pString );
	}
}

FPSCR void SteamDownloadWorkshopItem( LPSTR pString )
{

	if ( g_SteamRunning )
	{
		uint64 id = _atoi64(pString);
		g_UserWorkShopItem.DownloadWorkshopitem(id);
	}

}

FPSCR int SteamIsWorkshopItemDownloaded()
{
	if ( g_SteamRunning )
	{
		return g_UserWorkShopItem.isItemDownloaded;
	}

	return 0;
}

FPSCR int SteamIsWorkshopItemUploaded()
{
	if ( g_SteamRunning )
	{
		return g_UserWorkShopItem.isItemUploaded;
	}

	return 0;
}

FPSCR LPSTR SteamGetWorkshopID(void)
{
	if ( g_SteamRunning )
	{
		// Create a new string and copy input string to it
		char workshopIDAsString[256];
		sprintf ( workshopIDAsString , "%llu" , WorkShopItemID );
		return GetReturnStringFromTEXTWorkString( workshopIDAsString );
	}

	return "";
}

FPSCR LPSTR SteamGetWorkshopItemPath(void)
{
	if ( g_SteamRunning )
	{
		return GetReturnStringFromTEXTWorkString( WorkshopItemPath );
	}

	return "";
}

FPSCR void SteamGetWorkshopItemPathDLL ( LPSTR string )
{
	if ( g_SteamRunning )
		strcpy(string, WorkshopItemPath);
	else
		strcpy(string, "");
}

FPSCR int SteamIsWorkshopLoadingOnDLL()
{
	if ( g_SteamRunning )
		return IsWorkshopLoadingOn;
	else
		return 0;
}

FPSCR void SteamSetRoot(LPSTR string )
{
	strcpy ( steamRootPath , string );
}

FPSCR void SteamSetLobbyName(LPSTR name )
{
	strcpy ( hostsLobbyName , name );
}

FPSCR int SteamIsWorkshopItemInstalled(LPSTR idString )
{
	if ( g_SteamRunning )
	{
		if ( strcmp ( idString , "" ) == 0 ) return 0;

		uint64 unSizeOnDisk = 0;
		strcpy ( WorkshopItemPath , "" );
		uint64 id = _atoi64(idString);		
		uint32 timestamp;
		if ( SteamUGC()->GetItemInstallInfo( id, &unSizeOnDisk, WorkshopItemPath, sizeof(WorkshopItemPath) , &timestamp ) )
		{
			bool needupdate = false;
			bool isDownloading = false;
			uint64 bytesDownloaded;
			uint64 bytesTotal;

			// tomato - Commented this out as GetItemUpdateInfo no longer exists
			// using the new getitemdownloadinfo
			if ( SteamUGC()->GetItemDownloadInfo ( id , &bytesDownloaded , &bytesTotal ) )
			{
				uint32 ret = SteamUGC()->GetItemState ( id );
				if ( (ret & k_EItemStateNeedsUpdate) == 0 )
					needupdate = false;
				else
					needupdate = true;

				if ( needupdate )
				{
					g_UserWorkShopItem.DownloadWorkshopitem(id);
					isDownloading = true;
				}

				if (needupdate == false && isDownloading == false && bytesDownloaded == bytesTotal )
				{
					return 1;
				}
				else
				{
					if  ( needupdate == false )
						return 0;
					else
						return 2;
				}
			}

		}
	}

	return 0;
}

FPSCR int SteamHasSubscriptionWorkshopItemFinished()
{
	if ( g_SteamRunning )
	{
		return g_UserWorkShopItem.isItemDownloaded;
	}

	return -1;

}

FPSCR void SteamSendMyName()
{
	needToSendMyName = true;
}

FPSCR int SteamIsOverlayActive()
{
	if ( g_SteamRunning )
		return SteamOverlayActive;

	return 0;
}

FPSCR void SteamWorkshopModeOff()
{
	IsWorkshopLoadingOn = 0;
}

FPSCR void SteamShowAgreement()
{
	ShellExecuteW( NULL, L"open", L"http://steamcommunity.com/sharedfiles/workshoplegalagreement" , NULL, NULL, SW_SHOWMAXIMIZED );
}

FPSCR void SteamEndGame()
{
	if ( g_SteamRunning )
		Client()->SteamEndGame();
}

FPSCR void SteamSendChat( LPSTR msg )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSendChat" );
#endif
	if ( g_SteamRunning )
		Client()->SteamSendChat(msg);
}

FPSCR void SteamSendLobbyChat( LPSTR msg )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamSendLobbyChat" );
#endif
	if ( g_SteamRunning )
	{
		SteamMatchmaking()->SendLobbyChatMsg ( lobbyIAmInID , msg , strlen(msg)+1 );
	}
}

char chatTextReturn[100];

FPSCR LPSTR SteamGetChat( void )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamGetChat" );
#endif

	if ( chatList.size() > 0 )
	{
		strcpy ( chatTextReturn , chatList[0].msg );
		chatList.erase( chatList.begin() );

		//return pReturnString;
		return GetReturnStringFromTEXTWorkString( chatTextReturn );

	}

	return "";
} 

FPSCR int SteamInkey( int scancode )
{
	WORD theChar;

   static HKL layout=GetKeyboardLayout(0);
   static UCHAR State[256];

   if (GetKeyboardState(State)==FALSE)
      return 0;
   UINT vk=MapVirtualKeyEx(scancode,1,layout);
   if ( ToAsciiEx(vk,scancode,State,&theChar,0,layout) == 1 )
   {
	   if ( KEYDOWN ( VK_LSHIFT ) || KEYDOWN ( VK_RSHIFT ) )
	   {
		   if ( theChar >= 97 && theChar <=122 )
			   theChar -= 32;
		   if ( theChar >= 91 && theChar <=96 )
			   theChar += 32;
		   if ( theChar >= 49 && theChar <=57 )
			   theChar -= 16;
		   if ( theChar == 48 )
			   theChar = 41;
	   }

	   return int(theChar);
   }

	return 0;
}

FPSCR int SteamCheckSyncedAvatarTexturesWithServer()
{
	if ( Client() ) 
		if ( Client()->IsServer() )
			SteamAvatarServer();

	SteamAvatarClient();

	if ( syncedAvatarTextureMode == SYNC_AVATAR_TEX_MODE_DONE ) return 1;
	return 0;
}

FPSCR void SteamSetMyAvatarHeadTextureName(LPSTR sAvatarTexture)
{
	strcpy ( myAvatarTextureName , sAvatarTexture );
}

void SteamAvatarClient()
{
	switch(syncedAvatarTextureMode )
	{
		case SYNC_AVATAR_TEX_BEGIN: // send my texture name to the server
		{
			syncedAvatarTextureMode = SYNC_AVATAR_TEX_SENT_IF_I_HAVE_TEXTURE;
			if ( Client() ) 
			{
				if ( strcmp ( "" , myAvatarTextureName ) == 0 )
					Client()->AvatarSendWeHaveHeadTextureToServer(2);
				else
					Client()->AvatarSendWeHaveHeadTextureToServer(1);
			}

		} break;
		case SYNC_AVATAR_TEX_SENT_IF_I_HAVE_TEXTURE: {} break; // waiting for server to change mode
		case SYNC_AVATAR_TEX_MODE_SENDING: // send our texture
		{

			syncedAvatarTextureMode = SYNC_AVATAR_TEX_MODE_SENDING_WAITING;

			if ( strcmp ( "" , myAvatarTextureName ) == 0 ) return;

			// if we have a texture to send, we can take one off the count
			if ( strcmp ( "" , myAvatarTextureName ) != 0 ) --syncedAvatarHowManyTextures;

			// lets make our own texture first, then send it
			char dest[MAX_PATH];
			sprintf ( dest, "%sentitybank\\user\\charactercreator\\customAvatar_%i_cc.dds" , steamRootPath , Client()->SteamGetMyPlayerIndex() );
			DeleteFile(dest);
			CopyFile ( myAvatarTextureName , dest, false );

			if ( Client() ) 
			{
				Client()->SteamSendAvatarFileClient ( Client()->SteamGetMyPlayerIndex() , dest );
			}

		} break;
		case SYNC_AVATAR_TEX_MODE_SENDING_WAITING: {} break; // waiting for server to change mode
		case SYNC_AVATAR_TEX_MODE_DONE: {} break; // All done
	}
}

void SteamAvatarServer()
{
	switch(syncedAvatarTextureModeServer)
	{	
		case SYNC_AVATAR_TEX_BEGIN:
		{
			if ( Server()->AvatarCheck(SYNC_AVATAR_TEX_BEGIN) == 1 )
				syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_MODE_SENDING;
		} break;
		case SYNC_AVATAR_TEX_MODE_SENDING:
		{
			if ( Server()->AvatarCheck(SYNC_AVATAR_TEX_MODE_SENDING) == 1 )
				syncedAvatarTextureModeServer = SYNC_AVATAR_TEX_MODE_DONE;
		} break;
		case SYNC_AVATAR_TEX_MODE_DONE: {} break; // All done
	}
}

FPSCR int SteamStrCmp( LPSTR s1, LPSTR s2 )
{
#ifdef MAKE_MULTIPLAYER_LOG
	debugLog ( "SteamStrCmp" );
#endif
	return strcmp ( s1 , s2 );
}

FILE* debugFile = NULL;

void debugLog(LPSTR str )
{
	if ( !debugFile )
	{
		debugFile = GG_fopen ( "Steam Multiplayer Debug Log.txt" , "a" );
		char newstr[256];
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );

	}

	if ( debugFile )
	{
		fwrite ( str , strlen(str) , 1 , debugFile );
		fflush ( debugFile );
		fwrite ( "\n" , 1 , 1 , debugFile );
		fflush ( debugFile );
	}

}


void debugLog(LPSTR str, int code , int e , int v )
{
	if ( !debugFile )
	{
		debugFile = GG_fopen ( "Steam Multiplayer Debug Log.txt" , "a" );
		char newstr[256];
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );

	}

	if ( debugFile )
	{
		char newstr[1024];
		sprintf ( newstr , "%s ( %d , %d , %d )" , str, code, e, v );
		fwrite ( str , strlen(newstr) , 1 , debugFile );
		fflush ( debugFile );
		fwrite ( "\n" , 1 , 1 , debugFile );
		fwrite ( str , strlen(newstr) , 1 , debugFile );
	}

}

void debugLog(LPSTR str, int code )
{
	if ( !debugFile )
	{
		debugFile = GG_fopen ( "Steam Multiplayer Debug Log.txt" , "a" );
		char newstr[256];
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "========================= NEW =========================\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );
		strcpy ( newstr , "\n" );
		fwrite ( newstr , strlen(newstr) , 1 , debugFile );

	}

	if ( debugFile )
	{
		char newstr[1024];
		sprintf ( newstr , "%s ( %d )" , str, code );
		fwrite ( str , strlen(newstr) , 1 , debugFile );
		fflush ( debugFile );
		fwrite ( "\n" , 1 , 1 , debugFile );
		fwrite ( str , strlen(newstr) , 1 , debugFile );
	}

}

//========================================================================================

#ifdef _DEBUG_LOG_

void logStart()
{
	logFile = GG_fopen ( "multiplayer_logfile.txt" , "a" );
	log ( "=============================================================================" );
	log ( "================================ NEW SESSION ================================" );
	log ( "=============================================================================" );
}

void log( char* s)
{ 
	if ( !logFile ) logStart();

	if ( logFile )
	{
		fputs ( s , logFile );
		fputs ( "\n" , logFile );
	}
}

void log( char* s, int a)
{ 
	if ( !logFile ) logStart();

	if ( logFile )
	{
		char ts[512];
		sprintf ( ts , "%s, VAR: %d" , s , a );
		fputs ( ts , logFile );
		fputs ( "\n" , logFile );
	}
}

void log( char* s, int a, int b)
{ 

	if ( !logFile ) logStart();

	if ( logFile )
	{
		char ts[512];
		sprintf ( ts , "%s, VARS: %d, %d" , s , a , b );
		fputs ( ts , logFile );
		fputs ( "\n" , logFile );
	}
}

void log( char* s, int a, int b, int c)
{ 

	if ( !logFile ) logStart();

	if ( logFile )
	{
		char ts[512];
		sprintf ( ts , "%s, VARS: %d, %d, %d" , s , a , b , c );
		fputs ( ts , logFile );
		fputs ( "\n" , logFile );
	}
}

void logEnd()
{
	if ( logFile )
	{
		fclose ( logFile );
		logFile = NULL;
	}
}

#endif

//========================================================================================
*/

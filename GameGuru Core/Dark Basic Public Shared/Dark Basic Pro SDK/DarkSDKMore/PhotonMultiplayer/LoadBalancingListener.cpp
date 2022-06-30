// Include
#include <stdafx.h>
#include <iostream>
#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancingListener.h"
#include <wininet.h>
#include "..\..\..\..\GameGuru\Include\Common-Keys.h"

extern "C" FILE* GG_fopen( const char* filename, const char* mode );

// Better way to transfer FPM files faster
#define FASTFPMTRANSFER

// Externals
void timestampactivity ( int iStamp, char* pText );
void mp_addHostFPMFIleToMasterHostList ( LPSTR pFilenameToAdd );

// Namespaces 
using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

// Globals for FPM server transfer
MsgClientSendFileBegin_t g_msgSendFPMFile;
FILE* g_http_oFile = NULL;
FILE* g_http_fFile = NULL;
bool g_http_bSaveToFile = false;
char g_http_szLocalFile[2048];
char g_http_szUploadFile[2048];
float g_http_fProgress = 0.0f;
int g_http_iStatusCode = 0;
HANDLE g_http_hHttpRequest = NULL;
HANDLE g_http_hInetConnect = NULL;
HANDLE g_http_hInet = NULL;
int g_http_iTotalWritten = 0;
int g_http_iTotalLength = 0;
bool g_http_bTerminate = false;
char g_http_pErrorString[1024];
LPSTR g_http_pDataReturned = NULL;
DWORD g_http_dwDataReturnedBufferSize = 0;
unsigned int g_http_dwDataLength = 0;
DWORD g_http_dwExpectedSize = 0;

LoadBalancingListener::LoadBalancingListener ( PhotonView* pView, LPSTR pRootPath ) : mpLbc(NULL), mPhotonView(pView), mLocalPlayerNr(0), mMap("Forest")
{
	// clear tracked player array
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		m_rgpPlayer[i] = NULL;
		m_rgpPlayerLoadedAndReady[i] = 0;
	}
	miCurrentServerPlayerID = -1;
	mbIsServer = false;
	miGetFPMFromServerNow = 0;
	miGetFPMFromServerName = NULL;
	miGetFPMFromServerExpectedSize = 0;
	strcpy ( mpRootPath, pRootPath );

	// initialise remap array
	iRemapPlayerArraySize = 32;
	remapPlayerIndex = new int[iRemapPlayerArraySize];
	for ( int i = 0; i < iRemapPlayerArraySize; i++ )
		remapPlayerIndex[i] = -1;

	// general resets
	strcpy ( g_http_pErrorString, "" );
}

LoadBalancingListener::~LoadBalancingListener(void)
{
	// free remap array
	if ( remapPlayerIndex ) delete[] remapPlayerIndex;
	iRemapPlayerArraySize = 0;

	// clear reference to photonview
	mPhotonView = NULL;
}

void LoadBalancingListener::setLBC(ExitGames::LoadBalancing::Client* pLbc)
{
	this->mpLbc = pLbc;
}

void LoadBalancingListener::connect(const JString& userName)
{
	mpLbc->connect(AuthenticationValues().setUserID(JString()+GETTIMEMS()), userName);
	mPhotonView->setConnectingState(true);
}

void LoadBalancingListener::connectReturn(int errorCode, const JString& errorString, const JString& region, const JString& cluster)
{
	mPhotonView->setConnectingState(false);
	if(errorCode == ErrorCode::OK)
	{
		mPhotonView->setConnectionState(true);
	}
}

void LoadBalancingListener::disconnectReturn(void)
{
	CloseFileNow();
	timestampactivity(0,"lost network connection");
	mPhotonView->setConnectionState(false);
}

// From F:\GameGuruRepo\GameGuru Core\SDK\PhotonSDK\Photon-cpp\inc\Enums\StatusCode.h
//\\ EXCEPTION_ON_CONNECT              = 1023; ///<the PhotonPeer encountered an exception while opening the incoming connection to the server. The server could be down / not running.
//\\ CONNECT                           = 1024; ///<the PhotonPeer is connected.
//\\ DISCONNECT                        = 1025; ///<the PhotonPeer just disconnected.
//\\ EXCEPTION                         = 1026; ///<the PhotonPeer encountered an exception and will disconnect, too.
//\\ QUEUE_OUTGOING_RELIABLE_WARNING   = 1027; ///<PhotonPeer outgoing queue is filling up. send more often.
//\\ QUEUE_OUTGOING_UNRELIABLE_WARNING = 1029; ///<PhotonPeer outgoing queue is filling up. send more often.
//\\ SEND_ERROR                        = 1030; ///<Sending command failed. Either not connected, or the requested channel is bigger than the number of initialized channels.
//\\ QUEUE_OUTGOING_ACKS_WARNING       = 1031; ///<PhotonPeer outgoing queue is filling up. Send more often.
//\\ QUEUE_INCOMING_RELIABLE_WARNING   = 1033; ///<PhotonPeer incoming reliable queue is filling up. Dispatch more often.
//\\ QUEUE_INCOMING_UNRELIABLE_WARNING = 1035; ///<PhotonPeer incoming unreliable queue is filling up. Dispatch more often.
//\\ QUEUE_SENT_WARNING                = 1037; ///<PhotonPeer sent queue is filling up. Check, why the server does not acknowledge your sent reliable commands.
//\\ INTERNAL_RECEIVE_EXCEPTION        = 1039; ///<Exception, if a server cannot be connected. Most likely, the server is not responding. Ask the user to try again later.
//\\ TIMEOUT_DISCONNECT                = 1040; ///<Disconnection due to a timeout (client did no longer receive ACKs from server).
//\\ DISCONNECT_BY_SERVER              = 1041; ///<Disconnect by server due to timeout (received a disconnect command, cause server misses ACKs of client).
//\\ DISCONNECT_BY_SERVER_USER_LIMIT   = 1042; ///<Disconnect by server due to concurrent user limit reached (received a disconnect command).
//\\ DISCONNECT_BY_SERVER_LOGIC        = 1043; ///<Disconnect by server due to server's logic (received a disconnect command).
//\\ ENCRYPTION_ESTABLISHED            = 1048; ///<The encryption-setup for secure communication finished successfully.
//\\ ENCRYPTION_FAILED_TO_ESTABLISH    = 1049; ///<The encryption-setup failed for some reason. Check debug logs.

void LoadBalancingListener::connectionErrorReturn(int errorCode)
{
	char pStr[1024];
	sprintf(pStr,"connectionErrorReturn %d",errorCode);
	//MessageBox ( NULL, pStr, pStr, MB_OK );
}
void LoadBalancingListener::clientErrorReturn(int errorCode)
{
	char pStr[1024];
	sprintf(pStr,"clientErrorReturn %d",errorCode);
	//MessageBox ( NULL, pStr, pStr, MB_OK );
}
void LoadBalancingListener::warningReturn(int warningCode)
{
	char pStr[1024];
	sprintf(pStr,"warningReturn %d",warningCode);
	//MessageBox ( NULL, pStr, pStr, MB_OK );
}
void LoadBalancingListener::serverErrorReturn(int errorCode)
{
	char pStr[1024];
	sprintf(pStr,"serverErrorReturn %d",errorCode);
	//MessageBox ( NULL, pStr, pStr, MB_OK );
}

void LoadBalancingListener::createRoom ( LPSTR name )
{
	Hashtable props;
	props.put(L"m", mMap);
	mpLbc->opCreateRoom(name, RoomOptions().setCustomRoomProperties(props));
}

void LoadBalancingListener::createRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	if(errorCode == ErrorCode::OK)
	{
		muPhotonPlayerIndex = localPlayerNr;
		afterRoomJoined(localPlayerNr);
		mPhotonView->setInGameRoom(true);
	}
}

void LoadBalancingListener::updateRoomList(void)
{
	const JVector<Room*>& rooms = mpLbc->getRoomList();
	mPhotonView->ClearRoomList();
	int iRoomCountForMaxLimit = 0;
	for(unsigned int i=0; i<rooms.getSize(); ++i)
	{
		// get all rooms visible on this serber
		const char* str = rooms[i]->getName().ANSIRepresentation().cstr();

		// determine if showing ALL games, or just those users who are using the same LKA key 
		bool bMatchedSiteKey = false;
		LPSTR pSiteName = mPhotonView->GetSiteName();
		if ( _strnicmp ( str, pSiteName, strlen("12345-12345-12345-12345") ) == NULL )
		{
			// this users PC and the created room have matching site keys
			bMatchedSiteKey = true;
		}

		// add room to visible list for user to select
		if ( bMatchedSiteKey == true || mPhotonView->IsTeacherViewAllMode() == true )
		{
			// keep a limit of the first 200 games listed
			if ( iRoomCountForMaxLimit < 200 )
			{
				mPhotonView->AddRoomToList((LPSTR)str);
				iRoomCountForMaxLimit++;
			}
		}
	}
}

void LoadBalancingListener::joinRoom(const ExitGames::Common::JString& pRoomName)
{
	mpLbc->opJoinRoom ( pRoomName );
}

void LoadBalancingListener::joinRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	if(errorCode == ErrorCode::OK)
	{
		muPhotonPlayerIndex = localPlayerNr;
		afterRoomJoined(localPlayerNr);
		mPhotonView->setInGameRoom(true);
		// find out if room is already playing (game already started) - returns refresh of GlobalStates
		MsgGetGlobalStates_t msg;
		sendMessage ( (nByte*)&msg, sizeof(MsgGetGlobalStates_t), true );
	}
}

void LoadBalancingListener::setPlayerIDAsCurrentServerPlayer ( void )
{
	int mylocalplayernr = mpLbc->getLocalPlayer().getNumber();
	miCurrentServerPlayerID = mylocalplayernr;
	mbIsServer = true;
}

bool LoadBalancingListener::isPlayerLoadedAndReady ( int iRealPlayerNr )
{
	bool bReady = false;
	int iSlotIndex = GetRemap ( iRealPlayerNr );
	if ( m_rgpPlayer[iSlotIndex] )
		if ( m_rgpPlayerLoadedAndReady[iSlotIndex] != 0 )
			bReady = true;
	return bReady;
}

bool LoadBalancingListener::isEveryoneLoadedAndReady(void)
{
	bool allReady = true;
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] )
		{
			if ( m_rgpPlayerLoadedAndReady[i] == 0 )
			{
				allReady = false;
				break;
			}
		}
	}
	return allReady;
}

int LoadBalancingListener::getLocalPlayerID(void)
{
	return mpLbc->getLocalPlayer().getNumber();
}

void LoadBalancingListener::updatePlayerList(void)
{
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	const JVector<Player*>& players = myRoom.getPlayers();
	mPhotonView->ClearPlayerList();
	for(unsigned int i=0; i<players.getSize(); ++i)
	{
		const Player* p = players[i];
		const char* str = players[i]->getName().ANSIRepresentation().cstr();
		mPhotonView->AddPlayerToList((LPSTR)str);
	}
	manageTrackedPlayerList();
}

void LoadBalancingListener::manageTrackedPlayerList ( void )
{
	// will create/destroy a player list that matches across all user sessions (i.e. player 2 here is player 2 everywhere else)
	bool bPlayerNumberExceeded = false;
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	const JVector<Player*>& players = myRoom.getPlayers();
	int mylocalplayernr = mpLbc->getLocalPlayer().getNumber();
	int iActivePlayers[MAX_PLAYERS_PER_SERVER];
	memset ( iActivePlayers, 0, sizeof(iActivePlayers) );
	for ( unsigned int i = 0; i < players.getSize(); ++i)
	{
		const Player* player = players[i];
		int iPhotonRealPlayerNr = player->getNumber();
		if ( iPhotonRealPlayerNr > 0 )
		{
			if ( player->getIsInactive() == false )
			{
				// Check if we have an active player, find a free slot and create player here
				int playernr = AddPlayerToRemap ( iPhotonRealPlayerNr );
				if ( playernr >= 0 )
				{
					// set player name and flag as new player
					const char* str = player->getName().ANSIRepresentation().cstr();
					m_rgpPlayer[playernr]->newplayer = 1;

					// is the local player
					if ( iPhotonRealPlayerNr == mylocalplayernr )
						m_rgpPlayer[playernr]->SetIsLocalPlayer( 1 );
					else
						m_rgpPlayer[playernr]->SetIsLocalPlayer( 0 );
				}
				else
				{
					// return of -2 means we have no slots left
					if ( playernr == -2 ) bPlayerNumberExceeded = true;
				}
			}
			else
			{
				// inactive players are removed
				int playernr = GetRemap(iPhotonRealPlayerNr);
				if ( playernr >= 0 && playernr < MAX_PLAYERS_PER_SERVER && m_rgpPlayer[playernr] )
				{
					removePlayer(iPhotonRealPlayerNr);
				}
			}

			// flag active players
			int playernr = GetRemap(iPhotonRealPlayerNr);
			if ( playernr >= 0 && playernr < MAX_PLAYERS_PER_SERVER && m_rgpPlayer[playernr] )
				iActivePlayers[playernr] = 1;
		}
		else
		{
			bPlayerNumberExceeded = true;
		}
	}
	// now scan for players who is no longer in the list
	for ( unsigned int i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
	{
		if ( iActivePlayers[i] == 0 && m_rgpPlayer[i] )
		{
			// remove players no longer here
			removePlayer(m_rgpPlayer[i]->id);
		}
	}
	if ( bPlayerNumberExceeded == true )
	{
		// player number exceeded
		OutputDebugString ( "player number exceeded tracked player array size" );
	}
}

void LoadBalancingListener::removePlayer ( int iPhotonRealPlayerNr )
{
	// get remap slot index
	int iRemapSlotIndex = GetRemap(iPhotonRealPlayerNr);
	if ( iRemapSlotIndex >= 0 && iRemapSlotIndex < MAX_PLAYERS_PER_SERVER )
	{
		// ensure removed from remap
		RemovePlayerFromRemap ( iRemapSlotIndex );

		// player gone, so remove from list
		m_rgpPlayerLoadedAndReady[iRemapSlotIndex] = 0;
		delete m_rgpPlayer[iRemapSlotIndex];
		m_rgpPlayer[iRemapSlotIndex] = NULL;

		// also declare dead so object will disappear
		alive[iRemapSlotIndex] = 0;
	}
}

int LoadBalancingListener::findANewlyArrivedPlayer ( void )
{
	for ( unsigned int i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
	{
		if ( m_rgpPlayer[i] )
		{
			if ( m_rgpPlayer[i]->newplayer == 1 )
			{
				m_rgpPlayer[i]->newplayer = 0;
				return m_rgpPlayer[i]->id;
			}
		}
	}
	return -1;
}

int LoadBalancingListener::AddPlayerToRemap ( int iPhotonPlayerIndex ) 
{ 
	// first check if already have this, in which case skip
	int iRemapSlotIndex = 0;
	for (; iRemapSlotIndex < 8; iRemapSlotIndex++ )
		if ( m_rgpPlayer[iRemapSlotIndex] )
			if ( m_rgpPlayer[iRemapSlotIndex]->id == iPhotonPlayerIndex ) 
				return -1;

	// not exist, find a free slot from 0 to 7
	iRemapSlotIndex = 0;
	for (; iRemapSlotIndex < 8; iRemapSlotIndex++ )
		if ( m_rgpPlayer[iRemapSlotIndex] == NULL )
			break;

	if ( iRemapSlotIndex < 8 )
	{
		m_rgpPlayer[iRemapSlotIndex] = new CPlayer();
		m_rgpPlayer[iRemapSlotIndex]->id = iPhotonPlayerIndex;
		m_rgpPlayerLoadedAndReady[iRemapSlotIndex] = 0;
		if ( iPhotonPlayerIndex >= iRemapPlayerArraySize )
		{
			int* newremapPlayerIndex = new int[iRemapPlayerArraySize+32];
			for ( int i = 0; i < iRemapPlayerArraySize+32; i++ ) newremapPlayerIndex[i] = -1;
			memcpy ( newremapPlayerIndex, remapPlayerIndex, sizeof(int)*iRemapPlayerArraySize );
			delete remapPlayerIndex;
			remapPlayerIndex = newremapPlayerIndex;
			iRemapPlayerArraySize += 32;
		}
		remapPlayerIndex[iPhotonPlayerIndex] = iRemapSlotIndex;
		return iRemapSlotIndex;
	}
	else
		return -2;
}

void LoadBalancingListener::RemovePlayerFromRemap ( int iRemapSlotIndex ) 
{ 
	if ( iRemapSlotIndex >= 0 && iRemapSlotIndex < MAX_PLAYERS_PER_SERVER )
	{
		if ( m_rgpPlayer[iRemapSlotIndex] )
		{
			int iPhotonPlayerIndex = m_rgpPlayer[iRemapSlotIndex]->id;
			if ( iPhotonPlayerIndex > 0 )
			{
				remapPlayerIndex[iPhotonPlayerIndex] = -1;
				m_rgpPlayer[iRemapSlotIndex]->id = 0;
			}
		}
	}
}

int LoadBalancingListener::GetRemap ( int iPhotonPlayerIndex ) 
{
	if ( iPhotonPlayerIndex > 0 && iPhotonPlayerIndex < iRemapPlayerArraySize )
		return remapPlayerIndex[iPhotonPlayerIndex];
	else
		return -1;
}

void LoadBalancingListener::migrateHostIfServer ( void )
{
	// if this player was the server
	if ( mbIsServer == true )
	{
		// need to migrate responsibility elsewhere, which will be the top player in the list
		for ( unsigned int i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
		{
			int iSlotIndex = GetRemap(muPhotonPlayerIndex);
			if ( m_rgpPlayer[i] && i != iSlotIndex )
			{
				// found a new server player, inform everyone
				MsgNewHostChosen_t msg;
				msg.index = m_rgpPlayer[i]->id;
				sendMessage ( (nByte*)&msg, sizeof(MsgNewHostChosen_t), true );
				break;
			}
		}
		mbIsServer = false;
		miGetFPMFromServerNow = 0;
		miGetFPMFromServerName = NULL;
		miGetFPMFromServerExpectedSize = 0;
	}
}

void LoadBalancingListener::afterRoomJoined(int localPlayerNr)
{
	this->mLocalPlayerNr = localPlayerNr;
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	Hashtable props = myRoom.getCustomProperties();
	if(props.contains(L"m"))
		mMap = ((ValueObject<JString>*)props.getValue(L"m"))->getDataCopy();
}

int LoadBalancingListener::service(void)
{
	// check state
	int iMPState = mpLbc->getState();

	// handle logic
	unsigned long t = GETTIMEMS();
	if((t - mLocalPlayerlastUpdateTime) > 100 )
	{
		mLocalPlayerlastUpdateTime = t;
		if(mpLbc->getState() == PeerStates::Joined)
		{
			int iSlotIndex = GetRemap(muPhotonPlayerIndex);
			if ( m_rgpPlayer[ iSlotIndex ] )
			{
				MsgClientSendLocalUpdate_t msg;
				msg.SetShipPosition( muPhotonPlayerIndex );
				if ( m_rgpPlayer[ iSlotIndex ]->BGetClientUpdateData( msg.AccessUpdateData() ) )
				{
					sendMessage ( (nByte*)&msg, sizeof(MsgClientSendLocalUpdate_t), false );
				}
			}
		}
	}

	// return state for tracing status of MP stage
	return iMPState;
}

void LoadBalancingListener::sendGlobalVarState ( int iVarEventIndex, int iVarValue )
{
	Hashtable data;
	data.put((nByte)1, iVarValue);
	mpLbc->opRaiseEvent(true, data, iVarEventIndex, RaiseEventOptions().setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE).setInterestGroup(0));
	if ( iVarEventIndex == eGlobalEventGameRunning) mPhotonView->GlobalStates.iGameRunning = iVarValue;
}

void LoadBalancingListener::sendMessage ( nByte* msg, DWORD msgSize, bool bReliable )
{
	Hashtable data;
	data.put((nByte)1, msg, msgSize);
	mpLbc->opRaiseEvent(bReliable, data, eGlobalEventMessage, RaiseEventOptions().setInterestGroup(0));
}

void LoadBalancingListener::sendMessageToPlayer ( int iPlayerIndex, nByte* msg, DWORD msgSize, bool bReliable )
{
	Hashtable data;
	data.put((nByte)1, msg, msgSize);
	int targetplayer[1];
	targetplayer[0] = iPlayerIndex;
	mpLbc->opRaiseEvent(bReliable, data, eGlobalEventMessage, RaiseEventOptions().setTargetPlayers(targetplayer,1));
}

void LoadBalancingListener::SetSendFileCount ( int count, int iOnlySendMapToSpecificPlayer )
{
	// only server sends files
	if ( mbIsServer == true )
	{
		// send file count to all
		MsgClientSetSendFileCount_t msg;
		msg.count = count;
		serverOnlySendMapToSpecificPlayer = iOnlySendMapToSpecificPlayer;
		if ( serverOnlySendMapToSpecificPlayer == - 1 )
			sendMessage ( (nByte*)&msg, sizeof(MsgClientSetSendFileCount_t), true );
		else
			sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSetSendFileCount_t), true );

		// clear flags server uses to confirm all players have the files
		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
			miServerClientsFileSynced[i] = 0;

		// we assume correctly that server player already has the file :)
		int iSlotIndex = GetRemap(miCurrentServerPlayerID);
		miServerClientsFileSynced[iSlotIndex] = 1;		

		ServerFilesToReceive = count;
		ServerFilesReceived = count;
		IamSyncedWithServerFiles = 1;
	}
}

void LoadBalancingListener::SendFileBegin ( int index , LPSTR pString, LPSTR pRootPath )
{
	// only server sends files
	if ( mbIsServer == true )
	{
		// specify filename of file
		g_msgSendFPMFile.index = index;
		strcpy ( g_msgSendFPMFile.fileName, pString );

		#ifdef FASTFPMTRANSFER
		// send signal that we are using the TGC server to store the FPM
		g_msgSendFPMFile.fileSize = -1001;
		serverHowManyFileChunks = -1001;
		serverChunkToSendCount = 1;
		if ( serverOnlySendMapToSpecificPlayer == - 1 )
			sendMessage ( (nByte*)&g_msgSendFPMFile, sizeof(MsgClientSendFileBegin_t), true );
		else
			sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&g_msgSendFPMFile, sizeof(MsgClientSendFileBegin_t), true );
		#else
		// now get full asbolute path to file
		char pAbsFilename[2048];
		strcpy ( pAbsFilename, pRootPath );
		strcat ( pAbsFilename, "\\Files\\" );
		strcat ( pAbsFilename, pString );

		// open file ready to read data and send as chunks
		mhServerFile = GG_fopen ( pAbsFilename, "rb" );
		if ( mhServerFile )
		{
			// instruct to start sending file data
			fseek ( mhServerFile, 0, SEEK_END );
			msg.fileSize = ftell ( mhServerFile );
			rewind ( mhServerFile );
			serverHowManyFileChunks = (int)ceil( (float)msg.fileSize / float(FILE_CHUNK_SIZE) );
			serverChunkToSendCount = 1;
			if ( serverOnlySendMapToSpecificPlayer == - 1 )
				sendMessage ( (nByte*)&msg, sizeof(MsgClientSendFileBegin_t), true );
			else
				sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSendFileBegin_t), true );
		}
		#endif
	}
}

int DecodeUTF8Char( const char* str, int *numBytes )
{
	if (numBytes) *numBytes = 1;

	const unsigned char* ustr = (const unsigned char*) str;
	if ( *ustr < 128 ) 
	{
		// one byte
		return *ustr;
	}
	else
	{
		int result;

		if ( *ustr < 194 ) return -1; // not valid as the first byte

		if ( *ustr < 224 )
		{
			// two bytes
			result = (*ustr & 0x1F);
			result <<= 6;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // second byte must start 10xxxxxx
			result |= (*ustr & 0x3F);

			if (numBytes) (*numBytes)++;
			
			return result;
		}
		
		if ( *ustr < 240 )
		{
			// three bytes
			result = (*ustr & 0x0F);
			result <<= 6;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // second byte must start 10xxxxxx
			result |= (*ustr & 0x3F);
			result <<= 6;

			if (numBytes) (*numBytes)++;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // third byte must start 10xxxxxx
			result |= (*ustr & 0x3F);

			if (numBytes) (*numBytes)++;

			if ( result < 0x800 ) return -1; // overlong encoding
			if ( result >= 0xD800 && result <= 0xDFFF ) return -1; // reserved for UTF-16, not valid characters
			
			return result;
		}
		
		if ( *ustr < 245 )
		{
			// four bytes
			result = (*ustr & 0x07);
			result <<= 6;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // second byte must start 10xxxxxx
			result |= (*ustr & 0x3F);
			result <<= 6;

			if (numBytes) (*numBytes)++;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // third byte must start 10xxxxxx
			result |= (*ustr & 0x3F);
			result <<= 6;

			if (numBytes) (*numBytes)++;

			ustr++;
			if ( (*ustr & 0xC0) != 0x80 ) return -1; // fourth byte must start 10xxxxxx
			result |= (*ustr & 0x3F);

			if (numBytes) (*numBytes)++;

			if ( result < 0x10000 ) return -1; // overlong encoding
			if ( result > 0x10FFFF ) return -1; // outside valid character range
			
			return result;
		}
		
		// invalid
		return -1;
	}
}

void ASCIIReplace( LPSTR pData, int iLength, unsigned int find, unsigned int replace )
{
	if ( iLength == 0 ) return;
	if ( find == 0 ) return;
	if ( replace < 128 ) // can only take this shortcut if replace is also ascii
	{
		char *str = pData;
		while ( *str )
		{
			if ( *str == find ) *str = replace;
			str++;
		}
		return;
	}
}

int ASCIIRevFind( LPSTR pData, int iLength, int cFind )
{
	if ( iLength == 0 || !pData ) return -1;
	//if ( cFind >= 128 ) return -1;
	char *str = pData + (iLength-1);
	int pos = iLength-1;
	while ( str != pData )
	{
		if ( *str == cFind ) return pos;
		pos--;
		str--;
	}
	if ( *str == cFind ) return 0;
	return -1;
}

int SendFileInternal ( LPSTR szServerFile, bool bSaveToFile, LPSTR szLocalFileOrAltNameForFileOnServer, LPSTR szUploadFile, LPSTR szPostData )
{
	// init vars
	g_http_oFile = NULL;
	g_http_fProgress = 0;
	g_http_iStatusCode = 0;
	g_http_bSaveToFile = bSaveToFile;
	strcpy ( g_http_szLocalFile, szLocalFileOrAltNameForFileOnServer);
	strcpy ( g_http_szUploadFile, szUploadFile );
	strcpy ( g_http_pErrorString, "" );

	// if no server URL address, leave
	if ( strlen(szServerFile) == 0 ) return 0;

	// if going to save a file
	if ( g_http_bSaveToFile )
	{
		// ensure output file specified
		if ( !szLocalFileOrAltNameForFileOnServer)
			return 0;

		// open output file for writing
		g_http_oFile = GG_fopen ( g_http_szLocalFile, "wb" );
		if ( g_http_oFile == NULL )
		{
			return 0;
		}
		fclose ( g_http_oFile );//oFile.Close();
		g_http_oFile = NULL;
	}

	if ( strlen(g_http_szUploadFile) == 0 ) //szUploadFile.GetLength() == 0 )
	{
		return 0;
	}

	g_http_fFile = NULL;
	g_http_fFile = GG_fopen ( g_http_szUploadFile, "rb" ); //if ( !fFile.OpenToRead( szUploadFile ) )
	if ( g_http_fFile == NULL )
	{
		return 0;
	}

	char sFinalPostData[2048];//uString sFinalPostData;
	strcpy ( sFinalPostData, "" );
	char sName[ 256 ];
	char sValue[ 512 ];
	const char* remaining = szPostData;//.GetStr();
	int count = 0;//szPostData.Count( '&' ) + 1;
	if ( szPostData != NULL && strlen(szPostData) > 0 )
	{
		int iUTF8Find = '&';//find;
		char* ptr = szPostData;
		int numBytes;
		while ( *ptr )
		{
			int c = DecodeUTF8Char( ptr, &numBytes );
			if ( c == iUTF8Find ) count++;
			ptr += numBytes;
		}
	}
	count = count + 1;

	for ( int i = 0; i < count; i++ )
	{
		int pos = (int) strcspn( remaining, "=" );
		strncpy( sName, remaining, pos ); 
		sName[ pos ] = '\0';
		remaining += pos+1;
		pos = (int) strcspn( remaining, "&" );
		strncpy( sValue, remaining, pos ); 
		sValue[ pos ] = '\0';
		remaining += pos+1;
		if ( strlen( sName ) == 0 || strlen( sValue ) == 0 ) continue;
		strcat ( sFinalPostData, "--------------------AaB03x\r\nContent-Disposition: form-data; name=\"" );//sFinalPostData.Append( "--------------------AaB03x\r\nContent-Disposition: form-data; name=\"" );
		strcat ( sFinalPostData, sName );//sFinalPostData.Append( sName );
		strcat ( sFinalPostData, "\"\r\n\r\n" );//sFinalPostData.Append( "\"\r\n\r\n" );
		strcat ( sFinalPostData, sValue );//sFinalPostData.Append( sValue );
		strcat ( sFinalPostData, "\r\n" );//sFinalPostData.Append( "\r\n" );
	}
	ASCIIReplace ( g_http_szUploadFile, strlen(g_http_szUploadFile), '\\', '/');//szUploadFile.Replace( '\\', '/' );
	
	char sFilename[2048];//uString sFilename;
	int pos = ASCIIRevFind ( g_http_szUploadFile, strlen(g_http_szUploadFile), '/' );//szUploadFile.RevFind( '/' );
	if ( pos >= 0 ) strcpy ( sFilename, g_http_szUploadFile+pos+1 );//szUploadFile.SubString( sFilename, pos+1 );
	else strcpy ( sFilename, g_http_szUploadFile );//sFilename.SetStr( szUploadFile );

	strcat ( sFinalPostData, "--------------------AaB03x\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" );//sFinalPostData.Append( "--------------------AaB03x\r\nContent-Disposition: form-data; name=\"myfile\"; filename=\"" );

	// do we need to use alt name
	LPSTR pUseAltname = sFilename;
	if (szLocalFileOrAltNameForFileOnServer)
		if (strlen(szLocalFileOrAltNameForFileOnServer) > 1)
			pUseAltname = szLocalFileOrAltNameForFileOnServer;
	strcat(sFinalPostData, pUseAltname);// sFilename );//sFinalPostData.Append( sFilename );

	strcat ( sFinalPostData, "\"\r\nContent-Type: application/x-zip-compressed\r\n\r\n" );//sFinalPostData.Append( "\"\r\nContent-Type: application/x-zip-compressed\r\n\r\n" );

	char sEndPostData[2048];//uString sEndPostData( "\r\n--------------------AaB03x--\r\n" );
	strcpy ( sEndPostData, "\r\n--------------------AaB03x--\r\n" );

	DWORD dwFileSize = 0;//fFile.GetSize()
	fseek ( g_http_fFile, 0, SEEK_END );
	dwFileSize = ftell ( g_http_fFile );
	rewind ( g_http_fFile );
	g_http_iTotalLength = strlen(sFinalPostData) + dwFileSize + strlen(sEndPostData) ;//sFinalPostData.GetLength() + fFile.GetSize() + sEndPostData.GetLength();

	// start internet connection
	UINT iError = 0;
	g_http_hInet = InternetOpen( "InternetConnection", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( g_http_hInet == NULL )
	{
		iError = GetLastError( );
		return NULL;
	}

	// and connect to it
	unsigned short wHTTPType = INTERNET_DEFAULT_HTTPS_PORT;
	g_http_hInetConnect = InternetConnect( g_http_hInet, "www.thegamecreators.com", wHTTPType, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
	if ( g_http_hInetConnect == NULL )
	{
		InternetCloseHandle( g_http_hInet );
		iError = GetLastError( );
		return 0;
	}

	// set timeout options
	int m_iTimeout = 2000;
	InternetSetOption( g_http_hInetConnect, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)&m_iTimeout, sizeof(m_iTimeout) );  

	// prepare request
	DWORD dwFlag = INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_NO_CACHE_WRITE;
	dwFlag |= INTERNET_FLAG_SECURE;
	g_http_hHttpRequest = HttpOpenRequest( g_http_hInetConnect, "POST", szServerFile, "HTTP/1.1", "The Agent", NULL, dwFlag, 0 );
	if ( !g_http_hHttpRequest )
	{
		InternetCloseHandle( g_http_hInetConnect );
		InternetCloseHandle( g_http_hInet );
		return 0;
	}

	// add Content-Length header
	char sHeader[2048];//uString sHeader; 
	sprintf ( sHeader, "Content-Length: %d", g_http_iTotalLength );
	HttpAddRequestHeaders( g_http_hHttpRequest, sHeader, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE );

	// add Content-Type header
	HttpAddRequestHeaders( g_http_hHttpRequest, "Content-Type: multipart/form-data; boundary=------------------AaB03x", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE );

	// start request
	BOOL bSendResult = HttpSendRequestEx( g_http_hHttpRequest, NULL, NULL, NULL, NULL );
	if ( !bSendResult )
	{
		InternetCloseHandle( g_http_hHttpRequest );
		InternetCloseHandle( g_http_hInetConnect );
		InternetCloseHandle( g_http_hInet );
		return 0;
	}

	// send initial post data
	DWORD iWritten = 0;
	g_http_iTotalWritten = 0;
	bSendResult = InternetWriteFile( g_http_hHttpRequest, (void*)(sFinalPostData), strlen(sFinalPostData), &iWritten );//(void*)(sFinalPostData.GetStr()), sFinalPostData.GetLength(), &iWritten );
	if ( !bSendResult )
	{
		//uString error;
		//error.Format( "Failed to send initial data: %d", ::GetLastError( ) );
		//agk::Warning( error );
		InternetCloseHandle( g_http_hHttpRequest );
		InternetCloseHandle( g_http_hInetConnect );
		InternetCloseHandle( g_http_hInet );
		return 0;
	}
	g_http_iTotalWritten += iWritten;

	// successfully started async send
	return 1;
}

bool SendFileInternalAsync ( LPSTR* ppDataReturned, DWORD* pdwDataReturnedSize )
{
	// default is zero data returned
	if ( pdwDataReturnedSize ) *pdwDataReturnedSize = 0;
	
	//send file data a chunk at a time, allows progress bar to update
	BOOL bSendResult;
	DWORD dwWritten = 0;
	char bytes[ 40000 ];
	//while ( !feof(g_http_fFile) && !bTerminate )//!fFile.IsEOF() && !bTerminate )
	if ( !feof(g_http_fFile) )//!fFile.IsEOF() && !bTerminate )
	{
		int bytesread = fread ( bytes, 1, 40000, g_http_fFile );//int bytesread = fFile.ReadData( bytes, 20000 );
		if ( bytesread > 0 )
		{
			bSendResult = InternetWriteFile( g_http_hHttpRequest, (void*)bytes, bytesread, &dwWritten );
			if ( !bSendResult )
			{
				int iError = GetLastError();
				char *szError = 0;
				if ( iError > 12000 && iError < 12174 ) 
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle("wininet.dll"), iError, 0, (char*)&szError, 0, 0 );
				else 
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, iError, 0, (char*)&szError, 0, 0 );
				if ( szError )
				{
					if ( strlen(szError) < 1020 ) strcpy ( g_http_pErrorString, szError );
					LocalFree( szError );
				}
				g_http_bTerminate = true;
			}
			g_http_iTotalWritten += dwWritten;
			g_http_fProgress = g_http_iTotalWritten*100.0f / g_http_iTotalLength;
		}
	}
	if ( feof(g_http_fFile) || g_http_bTerminate == true )
	{
		// finished uploading file
		if ( g_http_fFile )
		{
			fclose ( g_http_fFile );
			g_http_fFile = NULL;
		}

		if ( g_http_bTerminate == true ) 
		{
			//uString error( "Could not upload file, upload aborted by user" );
			//agk::Warning( error );
			InternetCloseHandle( g_http_hHttpRequest );
			InternetCloseHandle( g_http_hInetConnect );
			InternetCloseHandle( g_http_hInet );
			return true;
		}

		// this duplicated from above (dangerous if we change seperator)
		char sEndPostData[2048];
		strcpy ( sEndPostData, "\r\n--------------------AaB03x--\r\n" );

		// send ending
		bSendResult = InternetWriteFile( g_http_hHttpRequest, (void*)(sEndPostData), strlen(sEndPostData), &dwWritten );//(void*)(sEndPostData.GetStr()), sEndPostData.GetLength(), &iWritten );
		if ( !bSendResult )
		{
			//uString error;
			//error.Format( "Failed to send final data: %d", ::GetLastError( ) );
			//agk::Warning( error );
			InternetCloseHandle( g_http_hHttpRequest );
			InternetCloseHandle( g_http_hInetConnect );
			InternetCloseHandle( g_http_hInet );
			return true;
		}

		//close request
		bSendResult = HttpEndRequest( g_http_hHttpRequest, NULL, NULL, NULL );
		if ( !bSendResult )
		{
			//uString error;
			//error.Format( "Failed to close request: %d", ::GetLastError( ) );
			//agk::Warning( error );
			InternetCloseHandle( g_http_hHttpRequest );
			InternetCloseHandle( g_http_hInetConnect );
			InternetCloseHandle( g_http_hInet );
			return true;
		}

		g_http_fProgress = 99.9f;

		// finished sending, start receiving
		DWORD dwBufferSize = sizeof(int);
		DWORD dwHeaderIndex = 0;
		BOOL bReturnHeader = HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (void*)&g_http_iStatusCode, &dwBufferSize, &dwHeaderIndex );

		DWORD dwContentLength = 0;
		dwBufferSize = sizeof(DWORD);
		dwHeaderIndex = 0;
		bReturnHeader = HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (void*)&dwContentLength, &dwBufferSize, &dwHeaderIndex );

		dwHeaderIndex = 0;
		char szContentType[150];
		DWORD ContentTypeLength = 150;
		HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_CONTENT_TYPE, (void*)szContentType, &ContentTypeLength, &dwHeaderIndex );

		DWORD dwArraySize = 0;
		DWORD dwDataLength = 0;

		// server is not guaranteed to send a content length
		if ( dwContentLength > 0 && !g_http_bSaveToFile )
		{
			*ppDataReturned = new char [ dwContentLength + 1 ];
			dwArraySize = dwContentLength + 1;
		}

		if ( g_http_bSaveToFile ) g_http_oFile = GG_fopen ( g_http_szLocalFile, "wb" );//oFile.OpenToWrite( szLocalFile );

		// create a read buffer
		char pBuffer[ 20000 ];

		// data comes in chunks
		for(;;)
		{
			// read the data from the HINTERNET handle.
			DWORD written = 0;
			if( !InternetReadFile( g_http_hHttpRequest, (LPVOID) pBuffer, 20000, &written ) )
			{
				if ( *ppDataReturned ) delete [] *ppDataReturned;
				if ( g_http_bSaveToFile ) { fclose (g_http_oFile); g_http_oFile=NULL; }//oFile.Close();
				InternetCloseHandle( g_http_hHttpRequest );
				InternetCloseHandle( g_http_hInetConnect );
				InternetCloseHandle( g_http_hInet );
				return true;
			}

			// Check the size of the remaining data. If it is zero it reached the end.
			if ( written == 0 ) break;

			// Add part-data to overall data returned
			DWORD dwNewDataSize = dwDataLength + written;

			if ( g_http_bSaveToFile )
			{
				fwrite( pBuffer, 1, written, g_http_oFile );//oFile.WriteData( pBuffer, written );
			}
			else
			{
				if ( dwArraySize <= dwNewDataSize )
				{
					// recreate a bigger array in 10KB chunks
					LPSTR pNewData = new char [ dwNewDataSize + 20000 ];
				
					if ( *ppDataReturned && dwDataLength > 0 ) 
					{
						memcpy( pNewData, *ppDataReturned, dwDataLength );
						delete [] *ppDataReturned;
					}
					*ppDataReturned = pNewData;

					dwArraySize = dwNewDataSize + 20000;
				}

				memcpy( *ppDataReturned + dwDataLength, pBuffer, written );
			}

			dwDataLength = dwNewDataSize;
		}

		if ( g_http_bSaveToFile ) { fclose (g_http_oFile); g_http_oFile=NULL; }//oFile.Close();

		g_http_fProgress = 100;

		InternetCloseHandle( g_http_hHttpRequest );
		InternetCloseHandle( g_http_hInetConnect );
		InternetCloseHandle( g_http_hInet );

		if ( *ppDataReturned ) (*ppDataReturned) [ dwDataLength ] = '\0';
		else
		{
			*ppDataReturned = new char[1];
			(*ppDataReturned)[0] = 0;
		}
		if ( pdwDataReturnedSize ) *pdwDataReturnedSize = dwDataLength;

		// successfully uploaded file and got good response back (we want to get to this true)
		return true;
	}

	// not yet finished, keep going around
	return false;
}

float SendFileInternalGetProgress(void)
{
	return g_http_fProgress;
}

float LoadBalancingListener::GetSendProgress ( void ) 
{
	return g_http_fProgress;
}

void LoadBalancingListener::GetSendError( LPSTR pErrorString )
{
	strcpy ( pErrorString, g_http_pErrorString );
}

UINT GetURLFile ( LPSTR urlWhere, DWORD dwExpectedSize )
{
	// temps and inits
	UINT iError = 0;
	int bSendResult = 0;
	g_http_dwExpectedSize = dwExpectedSize;
	g_http_dwDataReturnedBufferSize = 2048000;
	g_http_pDataReturned = new char[g_http_dwDataReturnedBufferSize+1];
	g_http_dwDataLength = 0;
	memset ( g_http_pDataReturned, 0, g_http_dwDataReturnedBufferSize+1 );

	// connect and get file data
	g_http_hInet = InternetOpen( "InternetConnection", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	if ( g_http_hInet == NULL )
	{
		iError = GetLastError( );
	}
	else
	{
		unsigned short wHTTPType = INTERNET_DEFAULT_HTTPS_PORT;
		g_http_hInetConnect = InternetConnect( g_http_hInet, "www.thegamecreators.com", wHTTPType, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0 );
		if ( g_http_hInetConnect == NULL )
		{
			iError = GetLastError( );
		}
		else
		{
			int m_iTimeout = 2000;
			InternetSetOption( g_http_hInetConnect, INTERNET_OPTION_CONNECT_TIMEOUT, (void*)&m_iTimeout, sizeof(m_iTimeout) );  
			g_http_hHttpRequest = HttpOpenRequest( g_http_hInetConnect, "GET", urlWhere, "HTTP/1.1", NULL, NULL, INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0 );
			if ( g_http_hHttpRequest == NULL )
			{
				iError = GetLastError( );
			}
			else
			{
				HttpAddRequestHeaders( g_http_hHttpRequest, "Content-Type: application/x-www-form-urlencoded", -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE );
				bSendResult = HttpSendRequest( g_http_hHttpRequest, NULL, -1, NULL, 0 );
				if ( bSendResult == 0 )
				{
					iError = GetLastError( );
				}
				else
				{
					int m_iStatusCode = 0;
					char m_szContentType[150];
					unsigned int dwBufferSize = sizeof(int);
					unsigned int dwHeaderIndex = 0;
					HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (void*)&m_iStatusCode, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int dwContentLength = 0;
					HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (void*)&dwContentLength, (LPDWORD)&dwBufferSize, (LPDWORD)&dwHeaderIndex );
					dwHeaderIndex = 0;
					unsigned int ContentTypeLength = 150;
					HttpQueryInfo( g_http_hHttpRequest, HTTP_QUERY_CONTENT_TYPE, (void*)m_szContentType, (LPDWORD)&ContentTypeLength, (LPDWORD)&dwHeaderIndex );

					// success
					return 1;
				}
			}
		}
	}

	// some error prevented connection
	InternetCloseHandle( g_http_hHttpRequest );
	InternetCloseHandle( g_http_hInetConnect );
	InternetCloseHandle( g_http_hInet );
	if ( g_http_pDataReturned )
	{
		delete g_http_pDataReturned;
		g_http_pDataReturned = NULL;
	}
	return 0;
}

UINT GetURLFileAsync ( FILE* pOutputFileHandle, float* pfProgress )
{
	// keep going until all filedata downloaded
	int iError = 0;
	char pBuffer[ 40000 ];
	unsigned int written = 0;
	if( !InternetReadFile( g_http_hHttpRequest, (void*) pBuffer, 40000, (LPDWORD)&written ) )
	{
		// error
	}
	if ( written > 0 )
	{
		// write more to file data
		if ( g_http_dwDataLength + written > g_http_dwDataReturnedBufferSize-1024 )
		{
			// increase size of data as file data gets better
			g_http_dwDataReturnedBufferSize = g_http_dwDataReturnedBufferSize+4096000;
			LPSTR pNewDataReturned = new char[g_http_dwDataReturnedBufferSize+1];
			memcpy ( pNewDataReturned, g_http_pDataReturned, g_http_dwDataLength );
			g_http_pDataReturned = pNewDataReturned;
		}
		memcpy( g_http_pDataReturned + g_http_dwDataLength, pBuffer, written );
		g_http_dwDataLength = g_http_dwDataLength + written;
		*pfProgress = 50.0f + (((float)g_http_dwDataLength/(float)g_http_dwExpectedSize)*50.0f);
	}
	else
	{
		// finish file data and save it
		InternetCloseHandle( g_http_hHttpRequest );
		InternetCloseHandle( g_http_hInetConnect );
		InternetCloseHandle( g_http_hInet );
		if ( iError > 0 )
		{
			char *szError = 0;
			if ( iError > 12000 && iError < 12174 ) 
				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, GetModuleHandle("wininet.dll"), iError, 0, (char*)&szError, 0, 0 );
			else 
				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, iError, 0, (char*)&szError, 0, 0 );
			if ( szError )
			{
				LocalFree( szError );
			}
		}

		// save file from filedata
		if ( pOutputFileHandle && g_http_dwDataLength > 0 )
		{
			fwrite ( g_http_pDataReturned, 1, g_http_dwDataLength, pOutputFileHandle );
		}

		// free usages
		delete g_http_pDataReturned;

		// finished
		return 1;
	}

	// keep going
	return 0;
}

int LoadBalancingListener::SendFileDone()
{
	// only server sends files
	if ( mbIsServer == true )
	{
		// end send early if target player leaves
		if ( serverOnlySendMapToSpecificPlayer != - 1 )
		{
			int iSlotIndex = GetRemap(serverOnlySendMapToSpecificPlayer);
			if ( iSlotIndex >= 0 && m_rgpPlayer[iSlotIndex] == NULL )
			{
				// player server was sending file to has gone
				#ifdef FASTFPMTRANSFER
				// no mhServerFile used for server upload of FPM
				#else
				if ( mhServerFile )
				{
					// close file if still open for old writing
					fclose ( mhServerFile );
					mhServerFile = NULL;
				}
				#endif
				return 1;
			}
		}

		// if file open and ready to read data
		#ifdef FASTFPMTRANSFER
		// use count to stage process of sending and signaling
		if ( serverChunkToSendCount == 1 )
		{
			// start uploading the FPM to the server
			LPSTR szServerFile = "/api/gameguru/multiplayer/storage/upload";
			bool bSaveToFile = false;
			LPSTR szLocalFile = "";
			LPSTR szUploadFile = g_msgSendFPMFile.fileName;
			LPSTR szPostData = FPMHOSTUPLOADKEY;
			SendFileInternal ( szServerFile, bSaveToFile, szLocalFile, szUploadFile, szPostData );
			serverChunkToSendCount = 2;
		}
		if ( serverChunkToSendCount == 2 )
		{
			DWORD dwDataReturnedSize = 0;
			LPSTR pDataReturned = NULL;
			if ( SendFileInternalAsync ( &pDataReturned, &dwDataReturnedSize ) == true )
			{
				if ( pDataReturned && strchr(pDataReturned, '{') != 0 && dwDataReturnedSize < 10240 )
				{
					// break up response string
					// {
					// "success": true,
					// "filename": "312321321321321321321.zip",
					// }
					char pfilenameText[10240];
					strcpy ( pfilenameText, "" );
					char pWorkStr[10240];
					memset ( pWorkStr, 0, sizeof(pWorkStr) );
					strcpy ( pWorkStr, pDataReturned );
					if ( pWorkStr[0]=='{' ) strcpy ( pWorkStr, pWorkStr+1 );
					int n = 10200;
					for (; n>0; n-- ) if ( pWorkStr[n] == '}' ) { pWorkStr[n-1] = 0; break; }
					char* pChop = strstr ( pWorkStr, "," );
					char pStatusStr[10240];
					strcpy ( pStatusStr, pWorkStr );
					if ( pChop ) pStatusStr[pChop-pWorkStr] = 0;
					if ( pChop[0]==',' ) pChop += 1;
					if ( strstr ( pStatusStr, "success" ) != NULL )
					{
						// success
						if ( strstr ( pStatusStr, "true" ) != NULL )
						{
							// filename
							pChop = strstr ( pChop, ":" ) + 2;
							strcpy ( pfilenameText, pChop );

							// when upload of FPM finished, and got back server side filename, signal this filename to rest
							MsgClientSendChunk_t msg;
							strcpy ( (LPSTR)msg.chunk, pfilenameText );
							msg.index = g_http_iTotalLength;
							if ( serverOnlySendMapToSpecificPlayer == - 1 )
								sendMessage ( (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );
							else
								sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );

							// also record this filename in hosts local list, as hosts are responsible for deleting this file from server
							// either when you leave mulitplayer, or next time inside multiplayer (server cleaned by user who made the mess)
							mp_addHostFPMFIleToMasterHostList ( pfilenameText );

							// send has been completed
							return 1;
						}
					}
					else
					{
						// error
						char* pMessageValue = strstr ( pChop, ":" ) + 1;
					}
				}
				else
				{
					// error uploading (likely permission denied due to invalid FPMHOSTUPLOADKEY)
					serverChunkToSendCount = 0;
					return -1;
				}
			}
			else
			{
				// keepng going - still uploading file to server - keep client(s) informed as to server upload process
				MsgClientSendProgress_t msg;
				msg.fProgress = g_http_fProgress / 2.0f;
				if ( serverOnlySendMapToSpecificPlayer == - 1 )
					sendMessage ( (nByte*)&msg, sizeof(MsgClientSendProgress_t), true );
				else
					sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSendProgress_t), true );
			}
		}
		#else
		if ( mhServerFile )
		{
			// send a chunk of the file data
			MsgClientSendChunk_t msg;
			msg.index = serverChunkToSendCount;
			fread ( &msg.chunk, 1 , FILE_CHUNK_SIZE , mhServerFile );
			if ( serverOnlySendMapToSpecificPlayer == - 1 )
				sendMessage ( (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );
			else
				sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );

			// increase to next chunk
			serverChunkToSendCount++;
			if ( serverChunkToSendCount > serverHowManyFileChunks )
			{
				// close file when al chunks sent
				fclose ( mhServerFile );
				mhServerFile = NULL;
				return 1;
			}
			return 0;
		}
		#endif
	}
	return 0;
}

void LoadBalancingListener::GetFileDone ( void )
{
	// only clients get files
	if ( mbIsServer == false )
	{
		if ( miGetFPMFromServerNow == 1 )
		{
			// get request from server
			char urlWhere[2048];
			strcpy ( urlWhere, "/api/gameguru/multiplayer/storage/download?" );
			strcat ( urlWhere, FPMHOSTUPLOADKEY );
			strcat ( urlWhere, "&file=" );
			strcat ( urlWhere, miGetFPMFromServerName );
			if ( GetURLFile ( urlWhere, miGetFPMFromServerExpectedSize  ) == 1 )
			{
				miGetFPMFromServerNow = 2;
				mfFileProgress = 2.0f;

				// free usage
				if ( miGetFPMFromServerName )
				{
					delete miGetFPMFromServerName;
					miGetFPMFromServerName = NULL;
				}
			}
			else
			{
				// could not find FPM server file
				MessageBox ( NULL, "could not find server FPM file", "file not found", MB_OK );
			}
		}
		else
		{
			if ( miGetFPMFromServerNow == 2 )
			{
				// do we have all the file
				if ( GetURLFileAsync ( mhServerFile, &mfFileProgress ) == 1 )
				{
					// yes, close file
					fclose ( mhServerFile );
					mhServerFile = NULL;

					// send signal the client has the file
					IamSyncedWithServerFiles = 1;
					MsgClientPlayerIamSyncedWithServerFiles_t msg;
					msg.index = muPhotonPlayerIndex;
					sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerIamSyncedWithServerFiles_t), true );

					// finished FPM download process
					miGetFPMFromServerNow = 0;
					mfFileProgress = 100.0f;
				}
			}
		}
	}
}

void LoadBalancingListener::CloseFileNow ( void )
{
	if ( mhServerFile )
	{
		fclose ( mhServerFile );
		mhServerFile = NULL;
	}
}

int LoadBalancingListener::IsEveryoneFileSynced()
{
	// only server sends files
	if ( mbIsServer == true )
	{
		// go through players, ensure all have reported that they are synced
		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
		{
			if ( m_rgpPlayer[i] )
			{
				int iSlotIndex = GetRemap(miCurrentServerPlayerID);
				if ( i != iSlotIndex && miServerClientsFileSynced[i] == 0 )
				{
					return 0;
				}

				// reset new player flag (only used when players join a game already in session)
				m_rgpPlayer[i]->newplayer = 0;
			}
		}

		// all non-server players synced, we are a go
		return 1;
	}
	return 0;
}

void LoadBalancingListener::RegisterEveryonePresentAsHere()
{
	// go through current players, mark them as here (so they are not considered as new arrivals when game starts)
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		if ( m_rgpPlayer[i] )
		{
			// reset new player flag (only used when players join a game already in session)
			m_rgpPlayer[i]->newplayer = 0;
		}
	}
}

float LoadBalancingListener::GetFileProgress ( void )
{
	return mfFileProgress;
}

void LoadBalancingListener::customEventAction(int playerNr, nByte eventCode, const Object& eventContentObj)
{
	ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
	if(eventCode == eGlobalEventGameRunning)
	{
		Object const* obj = eventContent.getValue("1");
		if(!obj) obj = eventContent.getValue((nByte)1);
		if(!obj) obj = eventContent.getValue(1);
		if(!obj) obj = eventContent.getValue(1.0);
		if(obj)
		{
			int value = (int)(obj->getType() == TypeCode::DOUBLE ? ((ValueObject<double>*)obj)->getDataCopy() : ((ValueObject<int>*)obj)->getDataCopy());
			mPhotonView->GlobalStates.iGameRunning = value;
		}
	}
	else if(eventCode == eGlobalEventEveryoneLoadedAndReady)
	{
		Object const* obj = eventContent.getValue("1");
		if(!obj) obj = eventContent.getValue((nByte)1);
		if(!obj) obj = eventContent.getValue(1);
		if(!obj) obj = eventContent.getValue(1.0);
		if(obj)
		{
			int value = (int)(obj->getType() == TypeCode::DOUBLE ? ((ValueObject<double>*)obj)->getDataCopy() : ((ValueObject<int>*)obj)->getDataCopy());
			mPhotonView->GlobalStates.EveryoneLoadedAndReady = value;
		}
	}
	else if(eventCode == eGlobalEventPlayerPosition)
	{
		Object const* obj = eventContent.getValue("1");
		if(!obj)
			obj = eventContent.getValue((nByte)1);
		if(!obj)
			obj = eventContent.getValue(1);
		if(!obj)
			obj = eventContent.getValue(1.0);
		if(obj && obj->getDimensions() == 1 && obj->getSizes()[0] == 2)
		{
			int x = 0; int y = 0;
			if(obj->getType() == TypeCode::DOUBLE)
			{
				double* data = ((ValueObject<double*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			if(obj->getType() == TypeCode::INTEGER)
			{
				int* data = ((ValueObject<int*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			else if(obj->getType() == TypeCode::BYTE)
			{
				nByte* data = ((ValueObject<nByte*>*)obj)->getDataCopy();
				x = (int)data[0];
				y = (int)data[1];
			}
			else if(obj->getType() == TypeCode::OBJECT)
			{
				Object* data = ((ValueObject<Object*>*)obj)->getDataCopy();
				if(data[0].getType() == TypeCode::INTEGER)
				{
					x = ((ValueObject<int>*)(data + 0))->getDataCopy();
					y = ((ValueObject<int>*)(data + 1))->getDataCopy();
				}
				else
				{
					x = (int)((ValueObject<double>*)(data + 0))->getDataCopy();
					y = (int)((ValueObject<double>*)(data + 1))->getDataCopy();
				}
				MemoryManagement::deallocateArray(data);
			}
		}
	}
	else if(eventCode == eGlobalEventMessage)
	{
		// receive message
		Object const* obj = eventContent.getValue("1");
		if(!obj) obj = eventContent.getValue((nByte)1);
		if(!obj) obj = eventContent.getValue(1);
		if(!obj) obj = eventContent.getValue(1.0);
		if(obj && obj->getDimensions() == 1 )
		{
			nByte* data = ((ValueObject<nByte*>*)obj)->getDataCopy();
			EMessage eMsg = (EMessage)LittleDWord( *(DWORD*)data );
			handleMessage ( playerNr, eMsg, obj->getSizes()[0], data );
			MemoryManagement::deallocateArray(data);
		}
	}
}

void LoadBalancingListener::handleMessage(int playerNr, EMessage eMsg, DWORD cubMsgSize, nByte* pchRecvBuf )
{
	switch ( eMsg )
	{
		case k_EMsgClientPlayAnimation:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayAnimation_t ) )
				{
					MsgClientPlayAnimation_t* pmsg = (MsgClientPlayAnimation_t*)pchRecvBuf;
					tAnimation a;
					a.index = pmsg->index;
					a.start = pmsg->start;
					a.end = pmsg->end;
					a.speed = pmsg->speed;
					animationList.push_back(a);
				}
			}
			break;
		case k_EMsgClientPlayerKeyState:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerKeyState_t ) )
				{
					MsgClientPlayerKeyState_t* pmsg = (MsgClientPlayerKeyState_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pmsg->index);
					if ( m_rgpPlayer[iSlotIndex] )
					{
						keystate[iSlotIndex][pmsg->key] = pmsg->state;
					}
				}
			}
			break;
		case k_EMsgClientPlayerSetPlayerAlive:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerSetPlayerAlive_t ) )
				{
					MsgClientPlayerSetPlayerAlive_t* pmsg = (MsgClientPlayerSetPlayerAlive_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pmsg->index);
					if ( m_rgpPlayer[iSlotIndex] )
					{
						alive[iSlotIndex] = pmsg->state;
					}
				}
			}
			break;
		case k_EMsgClientLua:
			{
				if ( cubMsgSize == sizeof( MsgClientLua_t ) )
				{
					MsgClientLua_t* pmsg = (MsgClientLua_t*)pchRecvBuf;

					// if set animation or play animation they need to be sent in order
					if ( pmsg->code < 5 )
					{			
						tLua l;
						l.code = pmsg->code;
						l.e = pmsg->e;
						l.v = pmsg->v;
						strcpy ( l.s , "" );
						luaList.insert( luaList.begin(), l);
					}
				}
			}
			break;
		case k_EMsgClientLuaString:
			{
				if ( cubMsgSize == sizeof( MsgClientLuaString_t ) )
				{
					MsgClientLuaString_t* pmsg = (MsgClientLuaString_t*)pchRecvBuf;
					//int index = pmsg->index;
					tLua l;
					l.code = pmsg->code;
					l.e = pmsg->e;
					l.v = 0;
					strcpy ( l.s , pmsg->s );
					luaList.insert( luaList.begin(),l);
				}
			}
			break;			
		case k_EMsgClientLuaPlayerSpecificString:
			{
				if ( cubMsgSize == sizeof( MsgClientLuaPlayerSpecificString_t ) )
				{
					MsgClientLuaPlayerSpecificString_t* pmsg = (MsgClientLuaPlayerSpecificString_t*)pchRecvBuf;
					tLua l;
					l.code = pmsg->code;
					int iSlotIndex = GetRemap(pmsg->iRealPhotonPlayerNr);
					l.e = iSlotIndex; // actually needs slot index in MP.cpp from player actual slot on other machines
					l.v = 0;
					strcpy ( l.s , pmsg->s );
					luaList.insert( luaList.begin(),l);
				}
			}
			break;			
		case k_EMsgNewHostChosen:
			{
				if ( cubMsgSize == sizeof( MsgNewHostChosen_t ) )
				{
					MsgNewHostChosen_t* pmsg = (MsgNewHostChosen_t*)pchRecvBuf;
					int iRealPhotonPlayerNr = pmsg->index;
					if ( iRealPhotonPlayerNr == muPhotonPlayerIndex )
					{
						// this player is the new server!
						setPlayerIDAsCurrentServerPlayer();
					}
				}
			}
			break;
		case k_EMsgClientPlayerAppearance:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerAppearance_t ) )
				{
					MsgClientPlayerAppearance_t* pmsg = (MsgClientPlayerAppearance_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pmsg->index);
					int appearance = pmsg->appearance;
					if ( m_rgpPlayer[iSlotIndex] )
					{
						playerAppearance[iSlotIndex] = appearance;
					}
				}
			}
			break;
		case k_EMsgClientSetSendFileCount:
			{
				// server sends the files, joiners receive them
				if ( mbIsServer == false )
				{
					if ( cubMsgSize == sizeof( MsgClientSetSendFileCount_t ) )
					{
						MsgClientSetSendFileCount_t* pmsg = (MsgClientSetSendFileCount_t*)pchRecvBuf;
						ServerFilesToReceive = pmsg->count;
						ServerFilesReceived = 0;
						IamSyncedWithServerFiles = 0;
					}
				}
			}
			break;			
		case k_EMsgClientSendFileBegin:
			{
				// server sends the files, joiners receive them
				if ( mbIsServer == false )
				{
					if ( cubMsgSize == sizeof( MsgClientSendFileBegin_t ) )
					{
						MsgClientSendFileBegin_t* pmsg = (MsgClientSendFileBegin_t*)pchRecvBuf;
						if ( mhServerFile ) fclose ( mhServerFile );

						// get full asbolute path to file for writing
						char pAbsFilename[2048];
						strcpy ( pAbsFilename, mpRootPath );
						strcat ( pAbsFilename, "\\Files\\" );
						strcat ( pAbsFilename, pmsg->fileName );

						mhServerFile = GG_fopen ( pAbsFilename, "wb" );
						#ifdef FASTFPMTRANSFER
						// pmsg->fileSize == -1001 (i.e g_msgSendFPMFile.fileSize = -1001) - not used now
						#else
						serverHowManyFileChunks = (int)ceil( (float)pmsg->fileSize / float(FILE_CHUNK_SIZE) );
						serverFileFileSize = pmsg->fileSize;
						#endif
						mfFileProgress = 0.0f;
					}
				}
			}
			break;
		case k_EMsgClientSendProgress:
			{
				if ( mbIsServer == false )
				{
					if ( cubMsgSize == sizeof( MsgClientSendProgress_t ) )
					{
						MsgClientSendProgress_t* pmsg = (MsgClientSendProgress_t*)pchRecvBuf;
						if ( mhServerFile ) 
						{
							#ifdef FASTFPMTRANSFER
							mfFileProgress = pmsg->fProgress;
							#endif
						}
					}
				}
			}
			break;
		case k_EMsgClientSendChunk:
			{
				// server sends the files, joiners receive them
				if ( mbIsServer == false )
				{
					if ( cubMsgSize == sizeof( MsgClientSendChunk_t ) )
					{
						MsgClientSendChunk_t* pmsg = (MsgClientSendChunk_t*)pchRecvBuf;
						if ( mhServerFile ) 
						{
							#ifdef FASTFPMTRANSFER
							// GET FPM file from server (chunk download needs to be outside message system (from TGC-server not photon)
							if ( miGetFPMFromServerNow == 0 )
							{
								miGetFPMFromServerNow = 1;
								LPSTR pMsgFileName = (LPSTR)pmsg->chunk;
								miGetFPMFromServerName = new char[strlen(pMsgFileName)+1];
								strcpy ( miGetFPMFromServerName, pMsgFileName );
								miGetFPMFromServerExpectedSize = pmsg->index;
								mfFileProgress = 1.0f;
							}
							#else
							int chunkSize = FILE_CHUNK_SIZE;
							if ( pmsg->index == serverHowManyFileChunks )
							{
								if ( serverHowManyFileChunks == 1 )
									chunkSize = serverFileFileSize;
								else
									chunkSize = serverFileFileSize - ((serverHowManyFileChunks-1) * FILE_CHUNK_SIZE	);				
							}
							mfFileProgress = ((float)(pmsg->index * FILE_CHUNK_SIZE) / (float)serverFileFileSize ) * 100.0f;
							fwrite( &pmsg->chunk[0] , 1 , chunkSize , mhServerFile );
							if ( pmsg->index == serverHowManyFileChunks )
							{
								fclose ( mhServerFile );
								mhServerFile = NULL;
								ServerFilesReceived++;
								if ( ServerFilesReceived == 0 ) ServerFilesReceived = 1;
								if ( ServerFilesReceived >= ServerFilesToReceive ) 
								{
									IamSyncedWithServerFiles = 1;
									MsgClientPlayerIamSyncedWithServerFiles_t msg;
									msg.index = muPhotonPlayerIndex;
									sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerIamSyncedWithServerFiles_t), true );
								}
								else
									IamSyncedWithServerFiles = 0;
							}
							#endif
						}
					}
				}
			}
			break;
		case k_EMsgClientPlayerIamSyncedWithServerFiles:
			if ( mbIsServer == true )
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerIamSyncedWithServerFiles_t ) )
				{
					MsgClientPlayerIamSyncedWithServerFiles_t* pmsg = (MsgClientPlayerIamSyncedWithServerFiles_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pmsg->index);
					if ( m_rgpPlayer[iSlotIndex] )
						miServerClientsFileSynced[iSlotIndex] = 1;							
				}
			}
			break;
		case k_EMsgClientPlayerSendIAmLoadedAndReady:
			{
				if ( cubMsgSize == sizeof( MsgClientSendIAmLoadedAndReady_t ) )
				{
					MsgClientSendIAmLoadedAndReady_t* pmsg = (MsgClientSendIAmLoadedAndReady_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pmsg->index);
					if ( m_rgpPlayer[iSlotIndex] )
					{
						// set this player as loaded and ready
						m_rgpPlayerLoadedAndReady[iSlotIndex] = 1;
					}
				}
			}
			break;
		case k_EMsgClientSendLocalUpdate:
			{
				if ( cubMsgSize == sizeof( MsgClientSendLocalUpdate_t ) )
				{
					MsgClientSendLocalUpdate_t *pMsg = (MsgClientSendLocalUpdate_t*)pchRecvBuf;
					int iSlotIndex = GetRemap(pMsg->m_uShipPosition);
					if ( m_rgpPlayer[iSlotIndex] )
					{
						ServerPlayerUpdateData_t updateData;
						updateData.x = pMsg->AccessUpdateData()->x;
						updateData.y = pMsg->AccessUpdateData()->y;
						updateData.z = pMsg->AccessUpdateData()->z;
						updateData.angle = pMsg->AccessUpdateData()->angle;
						m_rgpPlayer[iSlotIndex]->OnReceiveServerUpdate( &updateData );
					}
					break;
				}
			}
			break;
		case k_EMsgGetGlobalStates:
			{
				if ( cubMsgSize == sizeof( MsgGetGlobalStates_t ) )
				{
					// a new player joining a room has requested global settings (to determine if game already running)
					// so we do this by setting the global setting with the known value, and it will broadcast again to everyone (inc new player)
					sendGlobalVarState ( eGlobalEventGameRunning, mPhotonView->GlobalStates.iGameRunning );
				}
			}
			break;
		default:
			char rgch[128];
			sprintf_safe( rgch, "Invalid message %x\n", eMsg );
			rgch[ sizeof(rgch) - 1 ] = 0;
			OutputDebugString( rgch );
	}
}

void LoadBalancingListener::leaveRoom(void)
{
	mpLbc->opLeaveRoom();
}

void LoadBalancingListener::leaveRoomReturn(int errorCode, const JString& errorString)
{
	if(errorCode == ErrorCode::OK)
	{
		mPhotonView->setInGameRoom(false);
	}
}

void LoadBalancingListener::debugReturn(int debugLevel, const JString& string)
{
}

void LoadBalancingListener::joinRoomEventAction(int playerNr, const JVector<int>& playernrs, const Player& player)
{
}

void LoadBalancingListener::leaveRoomEventAction(int playerNr, bool isInactive)
{
}

void LoadBalancingListener::joinLobbyReturn(void)
{
}

void LoadBalancingListener::joinOrCreateRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
}

void LoadBalancingListener::joinRandomRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
}

void LoadBalancingListener::gotQueuedReturn(void)
{
}

void LoadBalancingListener::leaveLobbyReturn(void)
{
}

void LoadBalancingListener::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
}

void LoadBalancingListener::onLobbyStatsUpdate(const JVector<LobbyStatsResponse>& res)
{
}

void LoadBalancingListener::onRoomListUpdate()
{
}

void LoadBalancingListener::onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes)
{
}

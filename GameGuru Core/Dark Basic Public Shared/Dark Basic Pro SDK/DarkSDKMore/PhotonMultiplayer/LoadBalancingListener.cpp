#include <stdafx.h>
#include <iostream>
#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancingListener.h"

using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

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
	strcpy ( mpRootPath, pRootPath );

	// initialise remap array
	iRemapPlayerArraySize = 32;
	remapPlayerIndex = new int[iRemapPlayerArraySize];
	for ( int i = 0; i < iRemapPlayerArraySize; i++ )
		remapPlayerIndex[i] = -1;
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
			mPhotonView->AddRoomToList((LPSTR)str);
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
		MsgClientSendFileBegin_t msg;
		msg.index = index;
		strcpy ( msg.fileName , pString );

		// now get full asbolute path to file
		char pAbsFilename[2048];
		strcpy ( pAbsFilename, pRootPath );
		strcat ( pAbsFilename, "\\Files\\" );
		strcat ( pAbsFilename, pString );

		// open file ready to read data and send as chunks
		mhServerFile = fopen ( pAbsFilename, "rb" );
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
	}
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
				if ( mhServerFile )
				{
					// close file if still open for old writing
					fclose ( mhServerFile );
					mhServerFile = NULL;
				}
				return 1;
			}
		}

		// if file open and ready to read data
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
	}
	return 0;
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

int LoadBalancingListener::GetFileProgress ( void )
{
	return miFileProgress;
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

						mhServerFile = fopen ( pAbsFilename, "wb" );
						serverHowManyFileChunks = (int)ceil( (float)pmsg->fileSize / float(FILE_CHUNK_SIZE) );
						serverFileFileSize = pmsg->fileSize;
						miFileProgress = 0;
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
							int chunkSize = FILE_CHUNK_SIZE;
							if ( pmsg->index == serverHowManyFileChunks )
							{
								if ( serverHowManyFileChunks == 1 )
									chunkSize = serverFileFileSize;
								else
									chunkSize = serverFileFileSize - ((serverHowManyFileChunks-1) * FILE_CHUNK_SIZE	);				
							}

							miFileProgress = (int)ceil(((float)(pmsg->index * FILE_CHUNK_SIZE) / (float)serverFileFileSize )  * 100.0f);

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

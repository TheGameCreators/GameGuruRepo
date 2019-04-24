#include <stdafx.h>
#include <iostream>
#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancingListener.h"

using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

LoadBalancingListener::LoadBalancingListener(PhotonView* pView) : mpLbc(NULL), mPhotonView(pView), mLocalPlayerNr(0), mMap("Forest")
{
	// clear tracked player array
	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
	{
		m_rgpPlayer[i] = NULL;
		m_rgpPlayerLoadedAndReady[i] = 0;
	}
	miCurrentServerPlayerID = -1;
	mbIsServer = false;
}

LoadBalancingListener::~LoadBalancingListener(void)
{
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

		//Console::get().writeLine(L"connected to cluster " + cluster + L" of region " + region);
		//mpLbc->opJoinRandomRoom(); now create room manually above

		/* no point, only one room per lobby anyway
		mpLbc->opLeaveLobby();
		if ( mpLbc->opJoinLobby ( JString ( "MyLobby"), LobbyType::DEFAULT ) == true )
		{
			int qeuedokay=42;
		}
		*/
	}
}

void LoadBalancingListener::disconnectReturn(void)
{
	mPhotonView->setConnectionState(false);
}

void LoadBalancingListener::createRoom ( LPSTR name )
{
	Hashtable props;
	//props.put(L"s", mGridSize);
	props.put(L"m", mMap);
	mpLbc->opCreateRoom(name, RoomOptions().setCustomRoomProperties(props));
}

void LoadBalancingListener::createRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	if(errorCode == ErrorCode::OK)
	{
		muPlayerIndex = localPlayerNr;
		afterRoomJoined(localPlayerNr);
		mPhotonView->setInGameRoom(true);
	}
}

void LoadBalancingListener::updateRoomList(void)
{
	const JVector<Room*>& rooms = mpLbc->getRoomList();
	//JVector<JString> names(rooms.getSize());
	mPhotonView->ClearRoomList();
	for(unsigned int i=0; i<rooms.getSize(); ++i)
	{
		const char* str = rooms[i]->getName().ANSIRepresentation().cstr();
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
		muPlayerIndex = localPlayerNr;
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
		int playernr = players[i]->getNumber();
		mPhotonView->AddPlayerToList((LPSTR)str);
	}
	manageTrackedPlayerList();
}

void LoadBalancingListener::migrateHostIfServer ( void )
{
	// if this player was the server
	if ( mbIsServer == true )
	{
		// need to migrate responsibility elsewhere, which will be the top player in the list
		for ( unsigned int i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
		{
			if ( m_rgpPlayer[i] )
			{
				// found a new server player, inform everyone
				MsgNewHostChosen_t msg;
				msg.index = i;
				sendMessage ( (nByte*)&msg, sizeof(MsgNewHostChosen_t), true );
				break;
			}
		}
		mbIsServer = false;
	}
}

void LoadBalancingListener::removePlayer ( int playernr )
{
	// player gone, so remove from list
	delete m_rgpPlayer[playernr];
	m_rgpPlayer[playernr] = NULL;

	// also declare dead so object will disappear
	alive[playernr] = 0;
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
		int playernr = player->getNumber();
		if ( playernr < MAX_PLAYERS_PER_SERVER )
		{
			if ( player->getIsInactive() == false )
			{
				// Check if we have a player created locally for this player slot, if not create it
				if ( !m_rgpPlayer[playernr] )
				{
					m_rgpPlayer[playernr] = new CPlayer();
					const char* str = player->getName().ANSIRepresentation().cstr();
				}

				// is the local player
				if ( playernr == mylocalplayernr )
					m_rgpPlayer[playernr]->SetIsLocalPlayer( 1 );
				else
					m_rgpPlayer[playernr]->SetIsLocalPlayer( 0 );
			}
			else
			{
				// Make sure we don't have a player locally for this slot
				if ( m_rgpPlayer[playernr] )
				{
					removePlayer(playernr);
				}
			}
			if ( m_rgpPlayer[playernr] ) iActivePlayers[playernr] = 1;
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
			removePlayer(i);
		}
	}
	if ( bPlayerNumberExceeded == true )
	{
		// player number exceeded
		OutputDebugString ( "player number exceeded tracked player array size" );
	}
}

void LoadBalancingListener::afterRoomJoined(int localPlayerNr)
{
	this->mLocalPlayerNr = localPlayerNr;
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	Hashtable props = myRoom.getCustomProperties();
	if(props.contains(L"m"))
		mMap = ((ValueObject<JString>*)props.getValue(L"m"))->getDataCopy();

	//const JVector<Player*>& players = myRoom.getPlayers(); nbow used above
	//for(unsigned int i=0; i<players.getSize(); ++i)
	//{
	//	const Player* p = players[i];
		//mpView->addPlayer(p->getNumber(), p->getName(), p->getNumber() == mpLbc->getLocalPlayer().getNumber());
	//}
	//mpView->changePlayerColor(localPlayerNr, mLocalPlayer.color);
	//raiseColorEvent();
}

/* player management handled in 'manageTrackedPlayerList' above
void LoadBalancingListener::removePlayerFromServer( int uPosition )
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
	//m_rguPlayerScores[uPosition] = 0;	

	// let everyone else know the player has left the game
	MsgClientLeft_t msg;
	msg.index = uPosition;
	sendMessage ( (nByte*)&msg, sizeof(MsgClientLeft_t), true );

	//for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
	//{
	//	if ( m_rgpPlayer[i] )
	//		SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, &msg, sizeof(MsgClientLeft_t), k_EP2PSendReliable );
	//}
}
*/

int LoadBalancingListener::service(void)
{
	// check state
	int iMPState = mpLbc->getState();

	// handle logic
	unsigned long t = GETTIMEMS();
	if((t - mLocalPlayerlastUpdateTime) > 100 ) //PLAYER_UPDATE_INTERVAL_MS)
	{
		mLocalPlayerlastUpdateTime = t;
		if(mpLbc->getState() == PeerStates::Joined)
		{
			// old player data passing
			/*
			int d = rand() % 8;
			int xOffs[] = {-1, 0, 1, -1, 1, -1, 0, 1};
			int yOffs[] = {1, 1, 1, 0, 0, -1, -1, -1};
			int x = mLocalPlayerx + xOffs[d];
			int y = mLocalPlayery += yOffs[d];
			mLocalPlayerx = x;
			mLocalPlayery = y;
			Hashtable data;
			nByte coords[] ={static_cast<nByte>(mLocalPlayerx), static_cast<nByte>(mLocalPlayery)};
			data.put((nByte)1, coords, 2);
			mpLbc->opRaiseEvent(false, data, eGlobalEventPlayerPosition, RaiseEventOptions().setInterestGroup(0));
			*/

			if ( m_rgpPlayer[ muPlayerIndex ] )
			{
				MsgClientSendLocalUpdate_t msg;
				msg.SetShipPosition( muPlayerIndex );
				if ( m_rgpPlayer[ muPlayerIndex ]->BGetClientUpdateData( msg.AccessUpdateData() ) )
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
		//SteamNetworking()->SendP2PPacket( m_steamIDGameServer, &msg, sizeof(MsgClientSetSendFileCount_t), k_EP2PSendReliable );
		serverOnlySendMapToSpecificPlayer = iOnlySendMapToSpecificPlayer;
		if ( serverOnlySendMapToSpecificPlayer == - 1 )
			sendMessage ( (nByte*)&msg, sizeof(MsgClientSetSendFileCount_t), true );
		else
			sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSetSendFileCount_t), true );

		// clear flags server uses to confirm all players have the files
		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
			miServerClientsFileSynced[i] = 0;

		// we assume correctly that server player already has the file :)
		miServerClientsFileSynced[miCurrentServerPlayerID] = 1;		

		ServerFilesToReceive = count;
		ServerFilesReceived = count;
		IamSyncedWithServerFiles = 1;
		/*
		IamLoadedAndReady = 0;
		isEveryoneLoadedAndReady = 0;
		IamReadyToPlay = 0;
		isEveryoneReadyToPlay = 0;

		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
		{
			serverClientsFileSynced[i] = 0;
			serverClientsLoadedAndReady[i] = 0;
			serverClientsReadyToPlay[i] = 0;
		}
		*/
	}
}

void LoadBalancingListener::SendFileBegin ( int index , LPSTR pString )
{
	// only server sends files
	if ( mbIsServer == true )
	{
		//HowManyPlayersDoWeHave = 0;

		// specify filename of file
		MsgClientSendFileBegin_t msg;
		msg.index = index;
		strcpy ( msg.fileName , pString );

		// open file ready to read data and send as chunks
		mhServerFile = fopen ( pString, "rb" );
		if ( mhServerFile )
		{
			// instruct to start sending file data
			fseek ( mhServerFile, 0, SEEK_END );
			msg.fileSize = ftell ( mhServerFile );
			rewind ( mhServerFile );
			serverHowManyFileChunks = (int)ceil( (float)msg.fileSize / float(FILE_CHUNK_SIZE) );
			serverChunkToSendCount = 1;
			//SteamNetworking()->SendP2PPacket( m_steamIDGameServer, &msg, sizeof(MsgClientSendFileBegin_t), k_EP2PSendReliable );
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
		// if file open and ready to read data
		if ( mhServerFile )
		{
			// send a chunk of the file data
			MsgClientSendChunk_t msg;
			msg.index = serverChunkToSendCount;
			fread ( &msg.chunk, 1 , FILE_CHUNK_SIZE , mhServerFile );
			//SteamNetworking()->SendP2PPacket( m_steamIDGameServer, &msg, sizeof(MsgClientSendChunk_t), k_EP2PSendReliable );
			if ( serverOnlySendMapToSpecificPlayer == - 1 )
				sendMessage ( (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );
			else
				sendMessageToPlayer ( serverOnlySendMapToSpecificPlayer, (nByte*)&msg, sizeof(MsgClientSendChunk_t), true );

			// increase to next chunk
			serverChunkToSendCount++;
			if ( serverChunkToSendCount > serverHowManyFileChunks )
			{
				//IamLoadedAndReady = 0;
				//isEveryoneLoadedAndReady = 0;
				//for( uint32 i = 1; i < MAX_PLAYERS_PER_SERVER; i++ )
				//{
				//	serverClientsLoadedAndReady[i] = 0;
				//}
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

void LoadBalancingListener:: CloseFileNow ( void )
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
		//IamLoadedAndReady = 0;
		//isEveryoneLoadedAndReady = 0;
		//for( uint32 i = 1; i < MAX_PLAYERS_PER_SERVER; i++ )
		//	serverClientsLoadedAndReady[i] = 0;

		// check from 1 since the client hosting the server is player 0 and always synced
		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i )
			if ( i != miCurrentServerPlayerID && m_rgpPlayer[i] && miServerClientsFileSynced[i] == 0 )
				return 0;

		// all non-server players synced, we are a go
		return 1;
	}
	return 0;
}

int LoadBalancingListener::GetFileProgress ( void )
{
	return miFileProgress;
}

//void LoadBalancingListener::raiseColorEvent(void)
//{
	//Hashtable data;
	//data.put((nByte)1, 42);//mLocalPlayer.color);
	//mpLbc->opRaiseEvent(true, data, 1, RaiseEventOptions().setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE).setInterestGroup(0));
//}

//void LoadBalancingListener::changeRandomColor(void)
//{
	//mLocalPlayer.color = randomColor(100);
	//raiseColorEvent();
	//mpView->changePlayerColor(mLocalPlayerNr, mLocalPlayer.color);
//}

/*
void LoadBalancingListener::nextGridSize(void)
{
	mGridSize = mGridSize << 1;
	//if(mGridSize > GRID_SIZE_MAX)
	//	mGridSize = GRID_SIZE_MIN;
	//Console::get().writeLine(JString("nextGridSize: ") + mGridSize);
	mpLbc->getCurrentlyJoinedRoom().addCustomProperty(L"s", mGridSize);
	//mpView->setupScene(mGridSize);
}
*/

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
			//mpView->changePlayerPos(playerNr, x, y);
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
		/*
		case k_EMsgReceipt:
			{
				if ( cubMsgSize == sizeof( MsgReceipt_t ) )
				{
					MsgReceipt_t* pmsg = (MsgReceipt_t*)pchRecvBuf;
					int index = pmsg->logID;
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
			*/
		case k_EMsgClientPlayerSetPlayerAlive:
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerSetPlayerAlive_t ) )
				{
					MsgClientPlayerSetPlayerAlive_t* pmsg = (MsgClientPlayerSetPlayerAlive_t*)pchRecvBuf;
					int index = pmsg->index;
					if ( m_rgpPlayer[index] )
					{
						alive[index] = pmsg->state;
					}
				}
				/* from old server code - it rebroadcast it to other players (no need with Photon)
				if ( cubMsgSize == sizeof( MsgClientPlayerSetPlayerAlive_t ) )
				{
					MsgClientPlayerSetPlayerAlive_t* pmsg = (MsgClientPlayerSetPlayerAlive_t*)pchRecvBuf;
					int index = pmsg->index;
	
					for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					{
						if ( m_rgpPlayer[i] && i != index )
						{
							SteamGameServerNetworking()->SendP2PPacket( m_rgClientData[i].m_SteamIDUser, pmsg, sizeof(MsgClientPlayerSetPlayerAlive_t), k_EP2PSendUnreliable );
						}
					}
				}
				*/
			}
			break;
			/*
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
			*/
		case k_EMsgNewHostChosen:
			{
				if ( cubMsgSize == sizeof( MsgNewHostChosen_t ) )
				{
					MsgNewHostChosen_t* pmsg = (MsgNewHostChosen_t*)pchRecvBuf;
					int index = pmsg->index;
					if ( index == muPlayerIndex )
					{
						// this player is the new server!
						setPlayerIDAsCurrentServerPlayer();
					}
				}
			}
			break;
		/*
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
		*/
		case k_EMsgClientSetSendFileCount:
			{
				if ( cubMsgSize == sizeof( MsgClientSetSendFileCount_t ) )
				{
					MsgClientSetSendFileCount_t* pmsg = (MsgClientSetSendFileCount_t*)pchRecvBuf;
					ServerFilesToReceive = pmsg->count;
					ServerFilesReceived = 0;
					IamSyncedWithServerFiles = 0;
					//IamLoadedAndReady = 0;
					//isEveryoneLoadedAndReady = 0;
					//IamReadyToPlay = 0;
					//isEveryoneReadyToPlay = 0;
				}
			}
			break;			
		case k_EMsgClientSendFileBegin:
			{
				if ( cubMsgSize == sizeof( MsgClientSendFileBegin_t ) )
				{
					MsgClientSendFileBegin_t* pmsg = (MsgClientSendFileBegin_t*)pchRecvBuf;
					if ( mhServerFile ) fclose ( mhServerFile );
					mhServerFile = fopen ( pmsg->fileName, "wb" );
					serverHowManyFileChunks = (int)ceil( (float)pmsg->fileSize / float(FILE_CHUNK_SIZE) );
					serverFileFileSize = pmsg->fileSize;
					//IsWorkshopLoadingOn = 1;
					miFileProgress = 0;
				}
			}
			break;
		case k_EMsgClientSendChunk:
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
								msg.index = muPlayerIndex;
								//SteamNetworking()->SendP2PPacket( m_steamIDGameServer, &msg, sizeof(MsgClientPlayerIamSyncedWithServerFiles_t), k_EP2PSendReliable );
								sendMessage ( (nByte*)&msg, sizeof(MsgClientPlayerIamSyncedWithServerFiles_t), true );
							}
							else
								IamSyncedWithServerFiles = 0;
						}
					}
				}
			}
			break;
		/*
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
		*/
		case k_EMsgClientPlayerIamSyncedWithServerFiles:
			if ( mbIsServer == true )
			{
				if ( cubMsgSize == sizeof( MsgClientPlayerIamSyncedWithServerFiles_t ) )
				{
					MsgClientPlayerIamSyncedWithServerFiles_t* pmsg = (MsgClientPlayerIamSyncedWithServerFiles_t*)pchRecvBuf;
					int index = pmsg->index;
					if ( m_rgpPlayer[index] )
						miServerClientsFileSynced[index] = 1;							
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
						// set this player as loaded and ready
						m_rgpPlayerLoadedAndReady[index] = 1;
					}
				}
			}
			break;
		/*
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
				OnClientBeginAuthentication( steamIDRemote, 0 );
			}
			break;
		*/
		case k_EMsgClientSendLocalUpdate:
			{
				if ( cubMsgSize == sizeof( MsgClientSendLocalUpdate_t ) )
				{
					//bool bFound = false;
					//for( uint32 i=0; i<MAX_PLAYERS_PER_SERVER; ++i )
					//{
						//if ( m_rgClientData[i].m_SteamIDUser == steamIDRemote ) 
						//{
							//bFound = true;
							MsgClientSendLocalUpdate_t *pMsg = (MsgClientSendLocalUpdate_t*)pchRecvBuf;
							int index = pMsg->m_uShipPosition;
							if ( m_rgpPlayer[index] ) //m_rgClientData[i].m_bActive &&  )
							{
								//m_rgClientData[i].m_ulTickCountLastData = (uint64)GetCounterPassedTotal(); 
								ServerPlayerUpdateData_t updateData;
								updateData.x = pMsg->AccessUpdateData()->x;
								updateData.y = pMsg->AccessUpdateData()->y;
								updateData.z = pMsg->AccessUpdateData()->z;
								updateData.angle = pMsg->AccessUpdateData()->angle;
								m_rgpPlayer[index]->OnReceiveServerUpdate( &updateData );
								//m_rgpPlayer[index]->OnReceiveClientUpdate( pMsg->AccessUpdateData() );
							}
							break;
						//}
					//}
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
		/*
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
						// Clear our data on the user
						memset( &m_rgPendingClientData[i], 0 , sizeof( ClientConnectionData_t ) );
						break;
					}
				}
				if ( !bFound )
					OutputDebugString( "Got a client leaving server msg, but couldn't find a matching client\n" );
			}
		*/
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
	//Console::get().debugReturn(debugLevel, string);
}

/*
void LoadBalancingListener::updateGroups(void)
{
	if(mpLbc->getIsInRoom())
	{
		ExitGames::Common::JVector<nByte> remove;
		if(mUseGroups)
		{
			ExitGames::Common::JVector<nByte> add;
			add.addElement(getGroupByPos());
			mpLbc->opChangeGroups(&remove, &add);
		}
		else
			mpLbc->opChangeGroups(&remove, NULL);
	}
}
*/

void LoadBalancingListener::connectionErrorReturn(int errorCode)
{
}

void LoadBalancingListener::clientErrorReturn(int errorCode)
{
}

void LoadBalancingListener::warningReturn(int warningCode)
{
}

void LoadBalancingListener::serverErrorReturn(int errorCode)
{
}

void LoadBalancingListener::joinRoomEventAction(int playerNr, const JVector<int>& playernrs, const Player& player)
{
	//Console::get().writeLine(JString("player ") + playerNr + L" " + player.getName() + L" has joined the game");
	//mpView->addPlayer(playerNr, player.getName(), player.getNumber() == mpLbc->getLocalPlayer().getNumber());
	//mpView->changePlayerColor(mLocalPlayerNr, mLocalPlayer.color);
	//updateGroups();
}

void LoadBalancingListener::leaveRoomEventAction(int playerNr, bool isInactive)
{
	//if(isInactive)
		//Console::get().writeLine(JString(L"player ") + playerNr + L" has suspended the game");
	//else
	//{
		//Console::get().writeLine(JString(L"player ") + playerNr + L" has abandoned the game");
		//mpView->removePlayer(playerNr);
	//}
}

void LoadBalancingListener::joinLobbyReturn(void)
{
	/* no point, only one room per lobby anyway
	// check lobby stats
	//mpLbc->opLobbyStats ();
	JVector<LobbyStatsRequest> ls;
	ls.addElement(LobbyStatsRequest(L"MyLobby"));
	if ( mpLbc->opLobbyStats(ls) == true )
	{
		int lee = 42;
	}
	*/
}

void LoadBalancingListener::joinOrCreateRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	/* not used
	if(errorCode == ErrorCode::OK)
	{
		afterRoomJoined(localPlayerNr);
		mPhotonView->setInGameRoom(true);
	}
	*/
}

void LoadBalancingListener::joinRandomRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	/* not used
	if(errorCode == ErrorCode::NO_MATCH_FOUND)
	{
		createRoom();
	}
	else if(errorCode == ErrorCode::OK)
	{
		afterRoomJoined(localPlayerNr);
	}
	*/
}

void LoadBalancingListener::gotQueuedReturn(void)
{
}

void LoadBalancingListener::leaveLobbyReturn(void)
{
}

void LoadBalancingListener::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
	//Console::get().writeLine(L"===================== lobby stats response");
	//for(unsigned int i=0; i<res.getSize(); ++i)
		//Console::get().writeLine(res[i].toString());
	//char pStr[1024];
	//sprintf ( pStr, "%d", lobbyStats[0].getRoomCount() );
	//MessageBox ( NULL, pStr, pStr, MB_OK );
}

void LoadBalancingListener::onLobbyStatsUpdate(const JVector<LobbyStatsResponse>& res)
{
	//Console::get().writeLine(L"===================== lobby stats update");
	//for(unsigned int i=0; i<res.getSize(); ++i)
		//Console::get().writeLine(res[i].toString());
	// lobby stats request test
	//JVector<LobbyStatsRequest> ls;
	//ls.addElement(LobbyStatsRequest());
	//ls.addElement(LobbyStatsRequest(L"AAA"));
	//ls.addElement(LobbyStatsRequest(L"BBB"));
	//ls.addElement(LobbyStatsRequest(L"CCC", LobbyType::DEFAULT));
	//ls.addElement(LobbyStatsRequest(L"AAA", LobbyType::SQL_LOBBY));
	//mpLbc->opLobbyStats(ls);
}

void LoadBalancingListener::onRoomListUpdate()
{
	/* does not seem to get called - so created own LBL function 
	const JVector<Room*>& rooms = mpLbc->getRoomList();
	JVector<JString> names(rooms.getSize());
	mPhotonView->ClearRoomList();
	for(unsigned int i=0; i<rooms.getSize(); ++i)
		mPhotonView->AddRoomToList((LPSTR)rooms[i]->getName().cstr());
	*/
	//	names.addElement(rooms[i]->getName());
	//mpView->updateRoomList(names);
}

void LoadBalancingListener::onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes)
{
	//if(updateGridSize(changes))
	//	mpView->setupScene(mGridSize);
}

/*
bool LoadBalancingListener::updateGridSize(const ExitGames::Common::Hashtable& props)
{
	if(props.contains(L"s"))
	{
		const Object* v = props.getValue(L"s");
		switch(v->getType())
		{
		case TypeCode::INTEGER:
			mGridSize = ((ValueObject<int>*)v)->getDataCopy();
			return true;
		case TypeCode::DOUBLE:
			mGridSize = (int)((ValueObject<double>*)v)->getDataCopy();
			return true;
		}
	}
	return false;
}
*/


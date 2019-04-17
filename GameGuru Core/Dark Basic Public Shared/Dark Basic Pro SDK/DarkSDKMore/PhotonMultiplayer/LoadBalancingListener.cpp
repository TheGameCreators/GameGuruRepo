#include <stdafx.h>
#include <iostream>
#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancingListener.h"

using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

/*
static int randomColor(int from=0, int to=256)
{
	int r = from + rand() % (to - from);
	int g = from + rand() % (to - from);
	int b = from + rand() % (to - from);
	return (r << 16) + (g << 8) + b;
}

const JString PeerStatesStr[] = {
	L"Uninitialized",
	L"PeerCreated",
	L"ConnectingToNameserver",
	L"ConnectedToNameserver",
	L"DisconnectingFromNameserver",
	L"Connecting",
	L"Connected",
	L"WaitingForCustomAuthenticationNextStepCall",
	L"Authenticated",
	L"JoinedLobby",
	L"DisconnectingFromMasterserver",
	L"ConnectingToGameserver",
	L"ConnectedToGameserver",
	L"AuthenticatedOnGameServer",
	L"Joining",
	L"Joined",
	L"Leaving",
	L"Left",
	L"DisconnectingFromGameserver",
	L"ConnectingToMasterserver",
	L"ConnectedComingFromGameserver",
	L"AuthenticatedComingFromGameserver",
	L"Disconnecting",
	L"Disconnected"
};
*/

/*
// Checker below checks count match only
class PeerStatesStrChecker
{
public:
	PeerStatesStrChecker(void)
	{
		DEBUG_ASSERT(sizeof(PeerStatesStr)/sizeof(JString) == PeerStates::Disconnected + 1);
	}
} checker;

LocalPlayer::LocalPlayer(void) : x(0), y(0), color(randomColor(100)), lastUpdateTime(0)
{
}
*/

LoadBalancingListener::LoadBalancingListener(PhotonView* pView) : mpLbc(NULL), mPhotonView(pView), mLocalPlayerNr(0), mMap("Forest")
{
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
		afterRoomJoined(localPlayerNr);
		mPhotonView->setInGameRoom(true);
	}
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
	int mylocalplayernr = mpLbc->getLocalPlayer().getNumber();
}

void LoadBalancingListener::afterRoomJoined(int localPlayerNr)
{
	this->mLocalPlayerNr = localPlayerNr;
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	Hashtable props = myRoom.getCustomProperties();
	//updateGridSize(props);
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

int LoadBalancingListener::service(void)
{
	// check state
	int iMPState = mpLbc->getState();

	// handle logic
	unsigned long t = GETTIMEMS();
	if((t - mLocalPlayerlastUpdateTime) > 200 ) //PLAYER_UPDATE_INTERVAL_MS)
	{
		mLocalPlayerlastUpdateTime = t;
		if(mpLbc->getState() == PeerStates::Joined)
		{
			int d = rand() % 8;
			int xOffs[] = {-1, 0, 1, -1, 1, -1, 0, 1};
			int yOffs[] = {1, 1, 1, 0, 0, -1, -1, -1};
			int x = mLocalPlayerx + xOffs[d];
			int y = mLocalPlayery += yOffs[d];
			//if(x < 0)
			//	x = 1;
			//if(x >= mGridSize)
			//	x = mGridSize - 2;
			//if(y < 0)
			//	y = 1;
			//if(y >= mGridSize)
			//	y = mGridSize - 2;

			//int prevGroup = getGroupByPos();
			mLocalPlayerx = x;
			mLocalPlayery = y;
			//if(prevGroup != getGroupByPos())
			//	updateGroups();

			Hashtable data;
			nByte coords[] ={static_cast<nByte>(mLocalPlayerx), static_cast<nByte>(mLocalPlayery)};
			data.put((nByte)1, coords, 2);
			mpLbc->opRaiseEvent(false, data, 2, RaiseEventOptions().setInterestGroup(0));

		}
	}

	// return state for tracing status of MP stage
	return iMPState;
}

void LoadBalancingListener::customEventAction(int playerNr, nByte eventCode, const Object& eventContentObj)
{
	ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
	if(eventCode == 1)
	{
		Object const* obj = eventContent.getValue("1");
		if(!obj) obj = eventContent.getValue((nByte)1);
		if(!obj) obj = eventContent.getValue(1);
		if(!obj) obj = eventContent.getValue(1.0);
		if(obj)
		{
			int color = (int)(obj->getType() == TypeCode::DOUBLE ?  ((ValueObject<double>*)obj)->getDataCopy() : ((ValueObject<int>*)obj)->getDataCopy());
			//mpView->changePlayerColor(playerNr, color);
		}
	}
	else if(eventCode == 2)
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

void LoadBalancingListener::raiseColorEvent(void)
{
	Hashtable data;
	data.put((nByte)1, 42);//mLocalPlayer.color);
	mpLbc->opRaiseEvent(true, data, 1, RaiseEventOptions().setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE).setInterestGroup(0));
}

void LoadBalancingListener::debugReturn(int debugLevel, const JString& string)
{
	//Console::get().debugReturn(debugLevel, string);
}

void LoadBalancingListener::changeRandomColor(void)
{
	//mLocalPlayer.color = randomColor(100);
	raiseColorEvent();
	//mpView->changePlayerColor(mLocalPlayerNr, mLocalPlayer.color);
}

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


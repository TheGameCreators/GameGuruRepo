#include <stdafx.h>
#include <iostream>
#include <stdlib.h>
#include "Common-cpp/inc/UTF8String.h"
#include "Common-cpp/inc/MemoryManagement/Allocate.h"
#include "LoadBalancingListener.h"

using namespace ExitGames::Common;
using namespace ExitGames::LoadBalancing;

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

// TODO: update PeerStatesStr when PeerStates changes
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

LoadBalancingListener::LoadBalancingListener(PhotonView* pView) : mpLbc(NULL), mPhotonView(pView), mLocalPlayerNr(0), mGridSize(32), mMap("Forest"), mAutomove(true), mUseGroups(true), mSendGroup(0)
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
	mPhotonView->bConnected = true;
}

void LoadBalancingListener::setUseGroups(bool value)
{
	mUseGroups = value;
}

bool LoadBalancingListener::getUseGroups(void)
{
	return mUseGroups;
}

void LoadBalancingListener::setSendGroup(nByte value)
{
	mSendGroup = value;
}

void LoadBalancingListener::setAutomove(bool a)
{
	mAutomove = a;
}

bool LoadBalancingListener::getAutomove(void)
{
	return mAutomove;
}

void LoadBalancingListener::debugReturn(int debugLevel, const JString& string)
{
	//Console::get().debugReturn(debugLevel, string);
}

void LoadBalancingListener::connectionErrorReturn(int errorCode)
{
	//Console::get().writeLine(JString(L"connection failed with error ") + errorCode);
	updateState();
}

void LoadBalancingListener::clientErrorReturn(int errorCode)
{
	//Console::get().writeLine(JString(L"received error ") + errorCode + L" from client");
	updateState();
}

void LoadBalancingListener::warningReturn(int warningCode)
{
	//Console::get().writeLine(JString(L"received warning ") + warningCode + " from client");
}

void LoadBalancingListener::serverErrorReturn(int errorCode)
{
	//Console::get().writeLine(JString(L"received error ") + errorCode + " from server");
	updateState();
}

void LoadBalancingListener::joinRoomEventAction(int playerNr, const JVector<int>& playernrs, const Player& player)
{
	//Console::get().writeLine(JString("player ") + playerNr + L" " + player.getName() + L" has joined the game");
	//mpView->addPlayer(playerNr, player.getName(), player.getNumber() == mpLbc->getLocalPlayer().getNumber());
	//mpView->changePlayerColor(mLocalPlayerNr, mLocalPlayer.color);
	updateGroups();
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
		//else
			//Console::get().writeLine(L"bad color event");
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
		//else
			//Console::get().writeLine(L"Bad position event");
	}
}

void LoadBalancingListener::connectReturn(int errorCode, const JString& errorString, const JString& region, const JString& cluster)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		//Console::get().writeLine(L"connected to cluster " + cluster + L" of region " + region);
		mpLbc->opJoinRandomRoom();
	}
	//else
		//Console::get().writeLine(JString(L"Warn: connect failed ") + errorCode + L" " + errorString);
}

void LoadBalancingListener::disconnectReturn(void)
{
	updateState();
	mPhotonView->bConnected = false;
	//Console::get().writeLine(L"disconnected");
	//mpView->initPlayers();
}

void LoadBalancingListener::createRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		//Console::get().writeLine(L"room has been created");
		afterRoomJoined(localPlayerNr);
	}
	//else
		//Console::get().writeLine(L"Warn: opCreateRoom() failed: " + errorString);
}

void LoadBalancingListener::joinOrCreateRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		//Console::get().writeLine(L"room has been entered");
		afterRoomJoined(localPlayerNr);
	}
	//else
		//Console::get().writeLine(L"Warn: opJoinOrCreateRoom() failed: " + errorString);
}

void LoadBalancingListener::joinRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::OK)
	{
		//Console::get().writeLine(L"game room has been successfully joined");
		afterRoomJoined(localPlayerNr);
	}
	//else
		//Console::get().writeLine(L"opJoinRoom() failed: " + errorString);
}

void LoadBalancingListener::joinRandomRoomReturn(int localPlayerNr, const Hashtable& gameProperties, const Hashtable& playerProperties, int errorCode, const JString& errorString)
{
	updateState();
	if(errorCode == ErrorCode::NO_MATCH_FOUND)
		createRoom();
	else if(errorCode == ErrorCode::OK)
	{
		//Console::get().writeLine(L"game room has been successfully joined");
		afterRoomJoined(localPlayerNr);
	}
	//else
		//Console::get().writeLine(L"opJoinRandomRoom() failed: " + errorString);
}

void LoadBalancingListener::leaveRoomReturn(int errorCode, const JString& errorString)
{
	updateState();
	//if(errorCode == ErrorCode::OK)
		//Console::get().writeLine(L"game room has been successfully left");
	//else
		//Console::get().writeLine(L"opLeaveRoom() failed: " + errorString);
	//mpView->initPlayers();
}

void LoadBalancingListener::gotQueuedReturn(void)
{
	updateState();
}

void LoadBalancingListener::joinLobbyReturn(void)
{
	//Console::get().writeLine(L"joined lobby");
	updateState();
}

void LoadBalancingListener::leaveLobbyReturn(void)
{
	//Console::get().writeLine(L"left lobby");
	updateState();
}

void LoadBalancingListener::onLobbyStatsUpdate(const JVector<LobbyStatsResponse>& res)
{
	//Console::get().writeLine(L"===================== lobby stats update");
	//for(unsigned int i=0; i<res.getSize(); ++i)
		//Console::get().writeLine(res[i].toString());

	// lobby stats request test
	JVector<LobbyStatsRequest> ls;
	ls.addElement(LobbyStatsRequest());
	ls.addElement(LobbyStatsRequest(L"AAA"));
	ls.addElement(LobbyStatsRequest(L"BBB"));
	ls.addElement(LobbyStatsRequest(L"CCC", LobbyType::DEFAULT));
	ls.addElement(LobbyStatsRequest(L"AAA", LobbyType::SQL_LOBBY));
	mpLbc->opLobbyStats(ls);
}

void LoadBalancingListener::onLobbyStatsResponse(const ExitGames::Common::JVector<LobbyStatsResponse>& res)
{
	//Console::get().writeLine(L"===================== lobby stats response");
	//for(unsigned int i=0; i<res.getSize(); ++i)
		//Console::get().writeLine(res[i].toString());
}

void LoadBalancingListener::onRoomListUpdate()
{
	const JVector<Room*>& rooms = mpLbc->getRoomList();
	JVector<JString> names(rooms.getSize());
	for(unsigned int i=0; i<rooms.getSize(); ++i)
		names.addElement(rooms[i]->getName());
	//mpView->updateRoomList(names);
}

void LoadBalancingListener::onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes)
{
	//if(updateGridSize(changes))
	//	mpView->setupScene(mGridSize);
}

void LoadBalancingListener::updateState()
{
	int state = mpLbc->getState();
	//mpView->updateState(state, PeerStatesStr[state], state == PeerStates::Joined ? mpLbc->getCurrentlyJoinedRoom().getName() : JString());
}

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

void LoadBalancingListener::afterRoomJoined(int localPlayerNr)
{
	//Console::get().writeLine(JString(L"afterRoomJoined: localPlayerNr=") + localPlayerNr);
	this->mLocalPlayerNr = localPlayerNr;
	MutableRoom& myRoom = mpLbc->getCurrentlyJoinedRoom();
	Hashtable props = myRoom.getCustomProperties();
	updateGridSize(props);
	if(props.contains(L"m"))
		mMap = ((ValueObject<JString>*)props.getValue(L"m"))->getDataCopy();

	//mpView->initPlayers();
	//mpView->setupScene(mGridSize);

	const JVector<Player*>& players = myRoom.getPlayers();
	for(unsigned int i=0; i<players.getSize(); ++i)
	{
		const Player* p = players[i];
		//mpView->addPlayer(p->getNumber(), p->getName(), p->getNumber() == mpLbc->getLocalPlayer().getNumber());
	}
	//mpView->changePlayerColor(localPlayerNr, mLocalPlayer.color);
	raiseColorEvent();
}

void LoadBalancingListener::createRoom()
{
	JString name = JString(L"native-") + (rand()%100);
	Hashtable props;
	props.put(L"s", mGridSize);
	props.put(L"m", mMap);
	mpLbc->opCreateRoom(name, RoomOptions().setCustomRoomProperties(props));
	//Console::get().writeLine(L"Creating room " + name);
}

void LoadBalancingListener::service(void)
{
	unsigned long t = GETTIMEMS();
	if((t - mLocalPlayer.lastUpdateTime) > 200 ) //PLAYER_UPDATE_INTERVAL_MS)
	{
		mLocalPlayer.lastUpdateTime = t;
		if(mpLbc->getState() == PeerStates::Joined && mAutomove)
			moveLocalPlayer();
	}
}

void LoadBalancingListener::moveLocalPlayer(void)
{
	int d = rand() % 8;
	int xOffs[] = {-1, 0, 1, -1, 1, -1, 0, 1};
	int yOffs[] = {1, 1, 1, 0, 0, -1, -1, -1};
	int x = mLocalPlayer.x + xOffs[d];
	int y = mLocalPlayer.y += yOffs[d];
	if(x < 0)
		x = 1;
	if(x >= mGridSize)
		x = mGridSize - 2;
	if(y < 0)
		y = 1;
	if(y >= mGridSize)
		y = mGridSize - 2;
	setLocalPlayerPos(x, y);
}

bool LoadBalancingListener::setLocalPlayerPos(int x, int y)
{
	if(x >= 0 && x < mGridSize && y >= 0 && y < mGridSize)
	{
		int prevGroup = getGroupByPos();
		mLocalPlayer.x = x;
		mLocalPlayer.y = y;
		if(prevGroup != getGroupByPos())
			updateGroups();
		Hashtable data;

		nByte coords[] ={static_cast<nByte>(mLocalPlayer.x), static_cast<nByte>(mLocalPlayer.y)};
		data.put((nByte)1, coords, 2);

		mpLbc->opRaiseEvent(false, data, 2, RaiseEventOptions().setInterestGroup(mSendGroup?mSendGroup:mUseGroups?getGroupByPos():0));
		//mpView->changePlayerPos(mLocalPlayerNr, mLocalPlayer.x, mLocalPlayer.y);
		return true;
	}
	else
		return false;
}

void LoadBalancingListener::raiseColorEvent(void)
{
	Hashtable data;
	data.put((nByte)1, mLocalPlayer.color);
	mpLbc->opRaiseEvent(true, data, 1, RaiseEventOptions().setEventCaching(ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE).setInterestGroup(mSendGroup?mSendGroup:mUseGroups?getGroupByPos():0));
}

int LoadBalancingListener::getGroupByPos(void)
{
	//int xp = mLocalPlayer.x * GROUPS_PER_AXIS / mGridSize;
	//int yp = mLocalPlayer.y * GROUPS_PER_AXIS / mGridSize;
	return 1;// + xp + yp * GROUPS_PER_AXIS;
}

void LoadBalancingListener::changeRandomColor(void)
{
	//mLocalPlayer.color = randomColor(100);
	raiseColorEvent();
	//mpView->changePlayerColor(mLocalPlayerNr, mLocalPlayer.color);
}

void LoadBalancingListener::nextGridSize(void)
{
	mGridSize = mGridSize << 1;
	//if(mGridSize > GRID_SIZE_MAX)
	//	mGridSize = GRID_SIZE_MIN;
	//Console::get().writeLine(JString("nextGridSize: ") + mGridSize);
	mpLbc->getCurrentlyJoinedRoom().addCustomProperty(L"s", mGridSize);
	//mpView->setupScene(mGridSize);
}

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
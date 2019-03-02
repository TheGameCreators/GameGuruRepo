#pragma once

#include "LoadBalancing-cpp/inc/Client.h"
#include "PhotonView.h"

struct LocalPlayer
{
	LocalPlayer();
	int x;
	int y;
	int color;
	unsigned long lastUpdateTime;
};

class LoadBalancingListener : public ExitGames::LoadBalancing::Listener
{
public: 
	LoadBalancingListener(PhotonView* pView);
	~LoadBalancingListener(void);

	void setLBC(ExitGames::LoadBalancing::Client* pLbc);
	void connect(const ExitGames::Common::JString& userName);
	void setUseGroups(bool value);
	bool getUseGroups(void);
	void updateGroups(void);
	void setSendGroup(nByte value);
	void service(void);
	void createRoom(void);
	void setAutomove(bool a);
	bool getAutomove(void);
	void changeRandomColor(void);
	void nextGridSize(void);
	bool setLocalPlayerPos(int x, int y);
	void moveLocalPlayer(void);
private:
	//From Common::BaseListener

	// receive and print out debug out here
	virtual void debugReturn(int debugLevel, const ExitGames::Common::JString& string);

	//From LoadBalancing::LoadBalancingListener

	// implement your error-handling here
	virtual void connectionErrorReturn(int errorCode);
	virtual void clientErrorReturn(int errorCode);
	virtual void warningReturn(int warningCode);
	virtual void serverErrorReturn(int errorCode);

	// events, triggered by certain operations of all players in the same room
	virtual void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player);
	virtual void leaveRoomEventAction(int playerNr, bool isInactive);
	virtual void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent);

	// callbacks for operations on PhotonLoadBalancing server
	virtual void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster);
	virtual void disconnectReturn(void);
	virtual void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString);
	virtual void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString);
	virtual void gotQueuedReturn(void);
	virtual void joinLobbyReturn(void);
	virtual void leaveLobbyReturn(void);
	virtual void onRoomListUpdate(void);
	virtual void onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats);
	virtual void onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats);
	virtual void onRoomPropertiesChange(const ExitGames::Common::Hashtable& changes);

	void updateState(void);
	void afterRoomJoined(int localPlayerNr);
	bool updateGridSize(const ExitGames::Common::Hashtable& props);
	void raiseColorEvent(void);
	int getGroupByPos(void);

	ExitGames::LoadBalancing::Client* mpLbc;
	PhotonView* mPhotonView;
	LocalPlayer mLocalPlayer;
	int mLocalPlayerNr;
	int mGridSize;
	ExitGames::Common::JString mMap;
	bool mAutomove;
	bool mUseGroups;
	int mSendGroup; // forces group in outgoing events even if mUseGroups = false
};
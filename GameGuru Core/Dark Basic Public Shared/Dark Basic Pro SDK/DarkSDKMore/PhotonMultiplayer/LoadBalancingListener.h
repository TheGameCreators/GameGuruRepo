#pragma once

#include "LoadBalancing-cpp/inc/Client.h"
#include "PhotonView.h"
#include "Messages.h"
#include "CPlayer.h"

//struct LocalPlayer
//{
//	LocalPlayer();
//	int x;
//	int y;
//	int color;
//	unsigned long lastUpdateTime;
//};

// Enum for global var states
enum EGlobalEventIndices
{
	eGlobalEventNoState,
	eGlobalEventGameRunning,
	eGlobalEventPlayerPosition,
	eGlobalEventMessage,
	eGlobalEventEveryoneLoadedAndReady,
};

class LoadBalancingListener : public ExitGames::LoadBalancing::Listener
{
public: 
	LoadBalancingListener(PhotonView* pView, LPSTR pRootPath);
	~LoadBalancingListener(void);

	void setLBC(ExitGames::LoadBalancing::Client* pLbc);
	void connect(const ExitGames::Common::JString& userName);
	void createRoom(LPSTR name);
	void updateRoomList(void);
	void joinRoom(const ExitGames::Common::JString& pRoomName);
	void migrateHostIfServer ( void );
	void removePlayer ( int playernr );
	//void removePlayerFromServer( int uPosition );

	void setPlayerIDAsCurrentServerPlayer(void);
	bool isServer(void) { return mbIsServer; }
	bool isPlayerLoadedAndReady ( int iRealPlayerNr );
	bool isEveryoneLoadedAndReady(void);

	int getLocalPlayerID(void);
	void updatePlayerList(void);
	void manageTrackedPlayerList ( void );
	int findANewlyArrivedPlayer ( void );

	int service(void);
	void sendGlobalVarState ( int iVarEventIndex, int iVarValue );
	void sendMessage ( nByte* msg, DWORD msgSize, bool bReliable );
	void sendMessageToPlayer ( int iPlayerIndex, nByte* msg, DWORD msgSize, bool bReliable );
	void handleMessage ( int playerNr, EMessage msg, DWORD cubMsgSize, nByte* pchRecvBuf );

	void SetSendFileCount ( int count, int iOnlySendMapToSpecificPlayer);
	void SendFileBegin ( int index , LPSTR pString, LPSTR pRootPath );
	int SendFileDone();
	float GetSendProgress ( void );
	void GetSendError ( LPSTR pErrorString );
	void GetFileDone();
	void CloseFileNow ( void );
	int IsEveryoneFileSynced();
	void RegisterEveryonePresentAsHere();
	float GetFileProgress();

	void leaveRoom(void);

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

	void afterRoomJoined(int localPlayerNr);
	//bool updateGridSize(const ExitGames::Common::Hashtable& props);
	//void raiseColorEvent(void);

	ExitGames::LoadBalancing::Client* mpLbc;
	PhotonView* mPhotonView;

	bool mbIsServer = false;
	int miGetFPMFromServerNow = 0;
	LPSTR miGetFPMFromServerName = NULL;
	DWORD miGetFPMFromServerExpectedSize = 0;

	char mpRootPath[2048];
	FILE* mhServerFile = NULL;
	float mfFileProgress = 0.0f;

	int AddPlayerToRemap ( int iPlayerIndex );
	void RemovePlayerFromRemap ( int iPlayerIndex );

public:
	int GetRemap ( int iPlayerIndex );

	int muPhotonPlayerIndex = 0;
	int miCurrentServerPlayerID = 0;

	int iRemapPlayerArraySize;
	int* remapPlayerIndex;

	CPlayer* m_rgpPlayer[MAX_PLAYERS_PER_SERVER];
	int miServerClientsFileSynced[MAX_PLAYERS_PER_SERVER];
	int m_rgpPlayerLoadedAndReady[MAX_PLAYERS_PER_SERVER];

	int mLocalPlayerx = 0;
	int mLocalPlayery = 0;
	unsigned long mLocalPlayerlastUpdateTime = 0;

	int mLocalPlayerNr;
	ExitGames::Common::JString mMap;
};
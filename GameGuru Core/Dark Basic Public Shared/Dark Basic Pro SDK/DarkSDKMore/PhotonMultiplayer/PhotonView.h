#include "LoadBalancing-cpp/inc/Client.h"
#include <vector>

struct sGlobalStates
{
	int iGameRunning;
	int EveryoneLoadedAndReady;
};

class PhotonView
{
	public:
		PhotonView ( void );
		~PhotonView ( void );

	public:
		void SetSiteName ( LPSTR pName ) { strcpy ( pSiteName, pName ); }
		LPSTR GetSiteName ( void ) { return pSiteName; }
		void SetTeacherViewAllMode ( bool bViewAllMode ) { bTeacherViewAllMode = bViewAllMode; }
		bool IsTeacherViewAllMode ( void ) { return bTeacherViewAllMode; }

		void setConnectingState ( bool bState ) { bConnecting = bState; }
		bool isConnecting() { return bConnecting; }
		void setConnectionState ( bool bState ) { bConnected = bState; }
		bool isConnected() { return bConnected; }
		void setInGameRoom ( bool bState ) { bInGameRoom = bState; }
		bool isInGameRoom() { return bInGameRoom; }

		void ClearRoomList ( void );
		void AddRoomToList ( LPSTR pRoomName );
		int GetRoomCount ( void );
		LPSTR GetRoomName ( int iRoomIndex );

		void ClearPlayerList ( void );
		void AddPlayerToList ( LPSTR pPlayerName );
		int GetPlayerCount ( void );
		LPSTR GetPlayerName ( int iPlayerIndex );

	public:
		char pSiteName[1024];
		bool bTeacherViewAllMode;

		bool bConnecting;
		bool bConnected;
		bool bInGameRoom;
		int iTriggerLeaveMode;

		sGlobalStates GlobalStates;

		std::vector< LPSTR > sRoomList;
		std::vector< LPSTR > sPlayerList;
	
};

/*
class PhotonLib : private ExitGames::LoadBalancing::Listener
{
public:
	PhotonLib();

	void update(void);
	ExitGames::Common::JString getStateString(void);

private:

	// receive and print out debug out here
	virtual void debugReturn(int debugLevel, const ExitGames::Common::JString& string);

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
	virtual void joinLobbyReturn(void);
	virtual void leaveLobbyReturn(void);

	void sendData(void);

	class State
	{
	public:
		enum States
		{
			INITIALIZED = 0,
			CONNECTING,
			CONNECTED,
			JOINING,
			JOINED,
			SENT_DATA,
			RECEIVED_DATA,
			LEAVING,
			LEFT,
			DISCONNECTING,
			DISCONNECTED
		};
	};
	State::States mState;
	
	ExitGames::LoadBalancing::Client mLoadBalancingClient;
	//UIListener* mpOutputListener;
	ExitGames::Common::Logger mLogger;

	int64 mSendCount;
	int64 mReceiveCount;

public:

	void joinOrCreateRoom ( LPSTR gameName );
	void sendByte ( int iValue );
	void leaveRoom ( void );
	void disconnect ( void );

	bool isConnected ( void ) { if ( mState == State::States::CONNECTED ) return true; else return false; }
	bool isJoined ( void ) { if ( mState == State::States::JOINED ) return true; else return false; }
	bool isReceivedData ( void ) { if ( mState == State::States::RECEIVED_DATA ) return true; else return false; }
	bool isLeft ( void ) { if ( mState == State::States::LEFT ) return true; else return false; }
	bool isDisconnected ( void ) { if ( mState == State::States::DISCONNECTED ) return true; else return false; }

};
*/

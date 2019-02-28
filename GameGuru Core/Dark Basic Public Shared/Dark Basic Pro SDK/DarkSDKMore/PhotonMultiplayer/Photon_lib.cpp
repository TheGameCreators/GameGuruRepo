#include "stdafx.h"
#include "Photon_lib.h"
#include "limits.h"

static const ExitGames::Common::JString appID = L"f6c9acc6-a6a2-4704-9618-cd4a5ebe4db6";
static const ExitGames::Common::JString appVersion = L"0.5";
static ExitGames::Common::JString gameName = L"GG Photon Test";
static const ExitGames::Common::JString PLAYER_NAME = L"testuser";
static const int MAX_SENDCOUNT = 100;

PhotonLib::PhotonLib()
	: mState(State::INITIALIZED)
	, mLoadBalancingClient(*this, appID, appVersion)//, ExitGames::Photon::ConnectionProtocol::UDP, true)
	//, mpOutputListener(uiListener)
	, mSendCount(0)
	, mReceiveCount(0)
{
	mLoadBalancingClient.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
	mLogger.setListener(*this);
	mLogger.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
	ExitGames::Common::Base::setListener(this);
	ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS));
}

void PhotonLib::disconnect ( void )
{
	if ( mState == State::CONNECTED )
	{
		mLoadBalancingClient.disconnect();
		mState = State::DISCONNECTING;
	}
}

void PhotonLib::update(void)
{
	switch(mState)
	{
		case State::INITIALIZED:
			mLoadBalancingClient.connect(ExitGames::LoadBalancing::AuthenticationValues().setUserID(ExitGames::Common::JString()+GETTIMEMS()), PLAYER_NAME+GETTIMEMS());
			mState = State::CONNECTING;
			break;

		/*
		case State::CONNECTED:
			mLoadBalancingClient.opJoinOrCreateRoom(gameName);
			mState = State::JOINING;
			break;
		case State::JOINED:
			sendData();
			break;
		case State::RECEIVED_DATA:
			mLoadBalancingClient.opLeaveRoom();
			mState = State::LEAVING;
			break;
		case State::LEFT:
			mLoadBalancingClient.disconnect();
			mState = State::DISCONNECTING;
			break;
		case State::DISCONNECTED:
			mState = State::INITIALIZED;
			break;
		*/

		default:
			break;
	}
	mLoadBalancingClient.service();
}

ExitGames::Common::JString PhotonLib::getStateString(void)
{
	switch(mState)
	{
		case State::INITIALIZED:
			return L"disconnected";
		case State::CONNECTING:
			return L"connecting";
		case State::CONNECTED:
			return L"connected";
		case State::JOINING:
			return L"joining";
		case State::JOINED:
			return ExitGames::Common::JString(L"ingame\nsent event Nr. ") + mSendCount + L"\nreceived event Nr. " + mReceiveCount;
		case State::SENT_DATA:
			return ExitGames::Common::JString(L"sending completed") + L"\nreceived event Nr. " + mReceiveCount;
		case State::RECEIVED_DATA:
			return L"receiving completed";
		case State::LEAVING:
			return L"leaving";
		case State::LEFT:
			return L"left";
		case State::DISCONNECTING:
			return L"disconnecting";
		case State::DISCONNECTED:
			return L"disconnected";
		default:
			return L"unknown state";
	}
}

void PhotonLib::sendData(void)
{
	ExitGames::Common::Hashtable event;
	event.put(static_cast<nByte>(0), ++mSendCount);
	// send to ourselves only
	int myPlayerNumber = mLoadBalancingClient.getLocalPlayer().getNumber();
	mLoadBalancingClient.opRaiseEvent(true, event, 0, ExitGames::LoadBalancing::RaiseEventOptions().setTargetPlayers(&myPlayerNumber, 1));
	if(mSendCount >= MAX_SENDCOUNT)
		mState = State::SENT_DATA;
}

void PhotonLib::debugReturn(int, const ExitGames::Common::JString& string)
{
	//mpOutputListener->writeString(string);
}

void PhotonLib::connectionErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	//mpOutputListener->writeString(ExitGames::Common::JString(L"received connection error ") + errorCode);
	mState = State::DISCONNECTED;
}

void PhotonLib::clientErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + L" from client");
}

void PhotonLib::warningReturn(int warningCode)
{
	EGLOG(ExitGames::Common::DebugLevel::WARNINGS, L"code: %d", warningCode);
	//mpOutputListener->writeString(ExitGames::Common::JString(L"received warning ") + warningCode + L" from client");
}

void PhotonLib::serverErrorReturn(int errorCode)
{
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	//mpOutputListener->writeString(ExitGames::Common::JString(L"received error ") + errorCode + " from server");
}

void PhotonLib::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>&, const ExitGames::LoadBalancing::Player& player)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls joined the game", player.getName().cstr());
	//mpOutputListener->writeString(L"");
	//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" " + player.getName() + L" has joined the game");
}

void PhotonLib::leaveRoomEventAction(int playerNr, bool isInactive)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	//mpOutputListener->writeString(L"");
	//mpOutputListener->writeString(ExitGames::Common::JString(L"player ") + playerNr + L" has left the game");
}

void PhotonLib::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContentObj)
{
	ExitGames::Common::Hashtable eventContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContentObj).getDataCopy();
	switch(eventCode)
	{
	case 0:
		if(eventContent.getValue((nByte)0))
			mReceiveCount = ((ExitGames::Common::ValueObject<int64>*)(eventContent.getValue((nByte)0)))->getDataCopy();
		if(mState == State::SENT_DATA && mReceiveCount >= mSendCount)
		{
			mState = State::RECEIVED_DATA;
			mSendCount = 0;
			mReceiveCount = 0;
		}
		break;
	default:
		break;
	}

}

void PhotonLib::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"connected to cluster " + cluster + L" of region " + region);
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mState = State::DISCONNECTING;
		return;
	}
	//mpOutputListener->writeString(L"connected to cluster " + cluster);
	mState = State::CONNECTED;
}

void PhotonLib::disconnectReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	//mpOutputListener->writeString(L"disconnected");
	mState = State::DISCONNECTED;
}

void PhotonLib::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		//mpOutputListener->writeString(L"opCreateRoom() failed: " + errorString);
		mState = State::CONNECTED;
		return;
	}

	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been created");
	//mpOutputListener->writeString(L"regularly sending dummy events now");
	mState = State::JOINED;
}

void PhotonLib::joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		//mpOutputListener->writeString(L"opJoinOrCreateRoom() failed: " + errorString);
		mState = State::CONNECTED;
		return;
	}

	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been entered");
	//mpOutputListener->writeString(L"regularly sending dummy events now");
	mState = State::JOINED;
}

void PhotonLib::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{		
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		//mpOutputListener->writeString(L"opJoinRoom() failed: " + errorString);
		mState = State::CONNECTED;
		return;
	}
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
	//mpOutputListener->writeString(L"regularly sending dummy events now");

	mState = State::JOINED;
}

void PhotonLib::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable&, const ExitGames::Common::Hashtable&, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		//mpOutputListener->writeString(L"opJoinRandomRoom() failed: " + errorString);
		mState = State::CONNECTED;
		return;
	}

	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);	
	//mpOutputListener->writeString(L"... room " + mLoadBalancingClient.getCurrentlyJoinedRoom().getName() + " has been successfully joined");
	//mpOutputListener->writeString(L"regularly sending dummy events now");
	mState = State::JOINED;
}

void PhotonLib::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if(errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		//mpOutputListener->writeString(L"opLeaveRoom() failed: " + errorString);
		mState = State::DISCONNECTING;
		return;
	}
	mState = State::LEFT;
	//mpOutputListener->writeString(L"room has been successfully left");
}

void PhotonLib::joinLobbyReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	//mpOutputListener->writeString(L"joined lobby");
}

void PhotonLib::leaveLobbyReturn(void)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	//mpOutputListener->writeString(L"left lobby");
}

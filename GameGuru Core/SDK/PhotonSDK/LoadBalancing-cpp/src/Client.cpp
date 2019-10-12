/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "Photon-cpp/inc/Enums/EventCode.h"
#include "Photon-cpp/inc/Enums/EventCache.h"
#include "Photon-cpp/inc/Enums/EventKey.h"
#include "Photon-cpp/inc/Enums/OperationCode.h"
#include "Photon-cpp/inc/Enums/ParameterCode.h"
#include "Photon-cpp/inc/Enums/ReceiverGroup.h"
#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/Internal/AuthenticationValuesSecretSetter.h"
#include "LoadBalancing-cpp/inc/Internal/PlayerMovementInformant.h"
#include "LoadBalancing-cpp/inc/Internal/PuncherClient.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EncryptionDataParameters.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EncryptionMode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/EventCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/JoinType.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/OperationCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/ParameterCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Player.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Room.h"

/** @file LoadBalancing-cpp/inc/Client.h */

#if defined _EG_ENCRYPTOR_AVAILABLE && defined _EG_WEBSOCKET_AVAILABLE
#	define _EG_UDP_ENCRYPTION_AVAILABLE
#endif
#ifdef _EG_XB1_PLATFORM
#	define _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
#endif

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Common::MemoryManagement;
		using namespace Photon;
		using namespace Photon::Internal;
		using namespace Photon::StatusCode;
		using namespace Internal;

		const EG_CHAR* Client::M_NAMESERVER = L"ns.exitgames.com"; // default name server address

		/** @class Client
		   This class implements the %Photon %LoadBalancing work flow by using a Peer.
		   It keeps a state and automatically executes transitions between the Master and Game Servers.
		   
		   This class (and the Player, MutablePlayer, Room and MutableRoom classes) might be extended to implement your own logic.
		   
		   However this is not necessary. You can also just put your game specific network logic into a class that uses this class as is, which is the recommended approach.
		   
		   Override MutableRoom:createPlayer() when subclassing Player, getMutablePlayerFactory() + MutablePlayerFactory::create() + MutablePlayerFactory::destroy() when subclassing MutablePlayer,
		   createRoom() when subclassing Room and getMutableRoomFactory() + MutableRoomFactory::create() + MutableRoomFactory::destroy() when subclassing MutableRoom.
		   @remarks
		   Extension notes:
		   An extension of this class should override the functions that are inherited from Photon::PhotonListener, as they are called when the state changes. Call the base implementation first,
		   then pick the operation response, event or state that you want to react to and put it in a switch-case.
		   
		   We try to provide demos to each platform where this API can be used, so lookout for those. */

		/** @copydoc Peer::getServerTimeOffset() */
		int Client::getServerTimeOffset(void) const
		{
			return mpPeer->getServerTimeOffset();
		}

		/** @copydoc Peer::getServerTime() */
		int Client::getServerTime(void) const
		{
			return mpPeer->getServerTime();
		}

		/** @copydoc Peer::getBytesOut() */
		int Client::getBytesOut(void) const
		{
			return mpPeer->getBytesOut();
		}

		/** @copydoc Peer::getBytesIn() */
		int Client::getBytesIn(void) const
		{
			return mpPeer->getBytesIn();
		}

		/** @copydoc Peer::getByteCountCurrentDispatch() */
		int Client::getByteCountCurrentDispatch(void) const
		{
			return mpPeer->getByteCountCurrentDispatch();
		}

		/** @copydoc Peer::getByteCountLastOperation() */
		int Client::getByteCountLastOperation(void) const
		{
			return mpPeer->getByteCountLastOperation();
		}

		/** @copydoc Peer::getSentCountAllowance() */
		int Client::getSentCountAllowance(void) const
		{
			return mpPeer->getSentCountAllowance();
		}

		/** @copydoc Peer::setSentCountAllowance() */
		void Client::setSentCountAllowance(int sentCountAllowance)
		{
			mpPeer->setSentCountAllowance(sentCountAllowance);
		}

		/** @copydoc Peer::getTimePingInterval() */
		int Client::getTimePingInterval(void) const
		{
			return mpPeer->getTimePingInterval();
		}

		/** @copydoc Peer::setTimePingInterval() */
		void Client::setTimePingInterval(int timePingInterval)
		{
			mpPeer->setTimePingInterval(timePingInterval);
		}

		/** @copydoc Peer::getRoundTripTime() */
		int Client::getRoundTripTime(void) const
		{
			return mpPeer->getRoundTripTime();
		}

		/** @copydoc Peer::getRoundTripTimeVariance() */
		int Client::getRoundTripTimeVariance(void) const
		{
			return mpPeer->getRoundTripTimeVariance();
		}

		/** @copydoc Peer::getTimestampOfLastSocketReceive() */
		int Client::getTimestampOfLastSocketReceive(void) const
		{
			return mpPeer->getTimestampOfLastSocketReceive();
		}

		/** @copydoc Peer::getDebugOutputLevel() */
		int Client::getDebugOutputLevel(void) const
		{
			return mpPeer->getDebugOutputLevel();
		}

		/** @copydoc Peer::setDebugOutputLevel() */
		bool Client::setDebugOutputLevel(int debugLevel)
		{
			return mLogger.setDebugOutputLevel(debugLevel) && mpPeer->setDebugOutputLevel(debugLevel);
		}

		/**
		   @copydoc PhotonPeer::getLogFormatOptions() */
		const LogFormatOptions& Client::getLogFormatOptions(void) const
		{
			return mpPeer->getLogFormatOptions();
		}

		/**
		   @copydoc PhotonPeer::setLogFormatOptions() */
		void Client::setLogFormatOptions(const LogFormatOptions& formatOptions)
		{
			mLogger.setFormatOptions(formatOptions);
			mpPeer->setLogFormatOptions(formatOptions);
		}

		/** @copydoc Peer::getIncomingReliableCommandsCount() */
		int Client::getIncomingReliableCommandsCount(void) const
		{
			return mpPeer->getIncomingReliableCommandsCount();
		}

		/** @copydoc Peer::getPeerID() */
		short Client::getPeerID(void) const
		{
			return mpPeer->getPeerID();
		}

		/** @copydoc Peer::getDisconnectTimeout() */
		int Client::getDisconnectTimeout(void) const
		{
			return mpPeer->getDisconnectTimeout();
		}

		/** @copydoc Peer::setDisconnectTimeout() */
		void Client::setDisconnectTimeout(int disconnectTimeout)
		{
			mpPeer->setDisconnectTimeout(disconnectTimeout);
		}

		/** @copydoc Peer::getQueuedIncomingCommands() */
		int Client::getQueuedIncomingCommands(void) const
		{
			return mpPeer->getQueuedIncomingCommands();
		}

		/** @copydoc Peer::getQueuedOutgoingCommands() */
		int Client::getQueuedOutgoingCommands(void) const
		{
			return mpPeer->getQueuedOutgoingCommands();
		}

		/** @copydoc Peer::getIsPayloadEncryptionAvailable() */
		bool Client::getIsPayloadEncryptionAvailable(void) const
		{
			return mpPeer->getIsPayloadEncryptionAvailable();
		}

		/** @copydoc Peer::getIsEncryptionAvailable() */
		bool Client::getIsEncryptionAvailable(void) const
		{
			return mpPeer->getIsPayloadEncryptionAvailable();
		}

		/** @copydoc Peer::getResentReliableCommands() */
		int Client::getResentReliableCommands(void) const
		{
			return mpPeer->getResentReliableCommands();
		}

		/** @copydoc Peer::getLimitOfUnreliableCommands() */
		int Client::getLimitOfUnreliableCommands(void) const
		{
			return mpPeer->getLimitOfUnreliableCommands();
		}

		/** @copydoc Peer::setLimitOfUnreliableCommands() */
		void Client::setLimitOfUnreliableCommands(int value)
		{
			mpPeer->setLimitOfUnreliableCommands(value);
		}

		/** @copydoc Peer::getCRCEnabled() */
		bool Client::getCRCEnabled(void) const
		{
			return mpPeer->getCRCEnabled();
		}

		/** @copydoc Peer::setCRCEnabled() */
		void Client::setCRCEnabled(bool crcEnabled)
		{
			mpPeer->setCRCEnabled(crcEnabled);
		}

		/** @copydoc Peer::getPacketLossByCRC() */
		int Client::getPacketLossByCRC(void) const
		{
			return mpPeer->getPacketLossByCRC();
		}

		/** @copydoc Peer::getTrafficStatsEnabled() */
		bool Client::getTrafficStatsEnabled(void) const
		{
			return mpPeer->getTrafficStatsEnabled();
		}

		/** @copydoc Peer::setTrafficStatsEnabled() */
		void Client::setTrafficStatsEnabled(bool trafficStatsEnabled)
		{
			mpPeer->setTrafficStatsEnabled(trafficStatsEnabled);
		}

		/** @copydoc Peer::getTrafficStatsElapsedMs() */
		int Client::getTrafficStatsElapsedMs(void) const
		{
			return mpPeer->getTrafficStatsElapsedMs();
		}

		/** @copydoc Peer::getTrafficStatsIncoming() */
		const Photon::TrafficStats& Client::getTrafficStatsIncoming(void) const
		{
			return mpPeer->getTrafficStatsIncoming();
		}

		/** @copydoc Peer::getTrafficStatsOutgoing() */
		const Photon::TrafficStats& Client::getTrafficStatsOutgoing(void) const
		{
			return mpPeer->getTrafficStatsOutgoing();
		}

		/** @copydoc Peer::getTrafficStatsGameLevel() */
		const Photon::TrafficStatsGameLevel& Client::getTrafficStatsGameLevel(void) const
		{
			return mpPeer->getTrafficStatsGameLevel();
		}

		/** @copydoc Peer::getQuickResendAttempts() */
		nByte Client::getQuickResendAttempts(void) const
		{
			return mpPeer->getQuickResendAttempts();
		}

		/** @copydoc Peer::setQuickResendAttempts() */
		void Client::setQuickResendAttempts(nByte quickResendAttempts)
		{
			mpPeer->setQuickResendAttempts(quickResendAttempts);
		}
		
		/** @copydoc Peer::getChannelCountUserChannels() */
		nByte Client::getChannelCountUserChannels(void) const
		{
			return mpPeer->getChannelCountUserChannels();
		}

		/** @copydoc Peer::getPeerCount() */
		short Client::getPeerCount(void)
		{
			return Peer::getPeerCount();
		}

		 /**
		    The Current state this Client instance is in. Be Careful: several states are "transitions" that lead to other states.

		    @note This is publicly available purely for informational purposes (i.e. when debugging) and your logic should not rely on certain state changes, but should instead wait for the
			dedicated callbacks.

		    @return one of the values defined in PeerStates*/
		  int Client::getState(void) const
		{
			return mState;
		}

		/**
		   @return the address of the master server to which the client is connected when it is not inside a game room. */
		const JString& Client::getMasterserverAddress(void) const
		{
			return mMasterserver;
		}

		/**
		   @return the count of players that are currently participating in games on game servers that are in the same cluster (game servers assigned to the same master server) as the local
		   client. Each Photon Cloud region consists of at least one, but potentially multiple separate clusters.
		   @remarks
		   This value is only getting updated when the client is on the master server. */
		int Client::getCountPlayersIngame(void) const
		{
			return mPeerCount;
		}

		/**
		   @return the count of rooms that are currently existing on game servers in the same cluster (game servers assigned to the same master server) as the one the local client is connected to.
		   Each Photon Cloud region consists of at least one, but potentially multiple separate clusters.
		   @remarks
		   This value is only getting updated when the client is on the master server. */
		int Client::getCountGamesRunning(void) const
		{
			return mRoomCount;
		}

		/**
		   @return the total count of players that are currently connected to the same cluster (clients that are connected to the same master server or to a game server that is assigned to the
		   same master server) as the local client. Each Photon Cloud region consists of at least one, but potentially multiple separate clusters.
		   @remarks
		   This value is only getting updated when the client is on the master
		   server. */
		int Client::getCountPlayersOnline(void) const
		{
			return mPeerCount + mMasterPeerCount;
		}

		/**
		   @return a non-const reference to a MutableRoom instance that represents the currently joined room.
		   @remarks
		   The behavior when accessing the referenced instance after leaving the room in which that reference has been obtained and the behavior when calling this function without being inside a
		   room is undefined.
		   @note
		   Attention: Do not assign the return value of this function to a MutableRoom variable, but only assign it to a MutableRoom reference or simply directly operate on the function return
		   value, as assigning it to  a variable means that accessing that variable lets you operate on a local copy and operations that change that copy don't affect the actual room. */
		MutableRoom& Client::getCurrentlyJoinedRoom(void)
		{
			if(!mpCurrentlyJoinedRoom)
				mpCurrentlyJoinedRoom = createMutableRoom(L"", Hashtable(), JVector<JString>(), 0, 0, false, NULL, false, JVector<JString>()); 
			return *mpCurrentlyJoinedRoom;
		}
		
		/**
		   @return the list of all visible rooms.
		   @remarks
		   The value that is returned by this function is only updated inside a lobby of LobbyType::DEFAULT.
		   Clients that are inside a lobby of a different LobbyType, or in no lobby at all, do not receive room list updates.
		   The same Client instance can't be inside of multiple rooms at once. The term 'room' includes game rooms and lobbies.
		   Therefor a Client instance is not able to receive room list updates while it resides inside of a game room.
		   @remarks
		   To show up in the lobby the IsVisible flag of a room needs to be set to true (which is the default value).
		   The MaxPlayers setting and the current amount of players inside a room do not influence the rooms visibility, nor does the IsOpen flag: If the maximum amount of players is already
		   inside of the room or if the room is closed, then the room is still included in the room list, but attempts to join it will fail.
		   @sa getRoomNameList() */
		const JVector<Room*>& Client::getRoomList(void) const
		{
			return mRoomList;
		}
		
		/**
		   @return the list of the names of all visible rooms.

		   The entries in the returned JVector instance are guarenteed to be in the same order like the entries in the JVector instance that is returned by getRoomList().
		   The same remarks apply to this function as are mentioned for getRoomList().

		   @sa getRoomList() */
		const JVector<JString>& Client::getRoomNameList(void) const
		{
			return mRoomNameList;
		}
		
		/**
		   @return true if this client instance currently resides within a room, false otherwise.
		   @remarks
		   The term 'room' includes game rooms and lobbies.

		   @sa getIsInGameRoom(), getIsInLobby() */
		bool Client::getIsInRoom(void) const
		{
			return getIsInGameRoom() || getIsInLobby();
		}
		
		/**
		   @return true if this client instance currently resides within a game room, false otherwise.

		   @sa getIsInRoom(), getIsInLobby() */
		bool Client::getIsInGameRoom(void) const
		{
			return mState == PeerStates::Joined;
		}
		
		/**
		   @return true if this client instance currently resides within a lobby, false otherwise.

		   @sa getIsInRoom(), getIsInGameRoom() */
		bool Client::getIsInLobby(void) const
		{
			return mState == PeerStates::JoinedLobby;
		}
		
		/**
		   @return the current value of the autJoinLobby flag.
		   @remarks
		   The value of the autoJoinLobby flag determines if the client will automatically join the default lobby whenever it has successfully connected and whenever it leaves a game room.
		   @sa setAutoJoinLobby() */
		bool Client::getAutoJoinLobby(void) const
		{
			return mAutoJoinLobby;
		}
		
		/**
		   Sets the value of the autJoinLobby flag.
		   @param autoJoinLobby the new value to which the flag will be set
		   @remarks
		   The value of the autoJoinLobby flag determines if the client will automatically join the default lobby whenever it has successfully connected and whenever it leaves a game room.

		   @sa getAutoJoinLobby() */
		void Client::setAutoJoinLobby(bool autoJoinLobby)
		{
			mAutoJoinLobby = autoJoinLobby;
		}
		
		/**
		   @return a non-const reference to the MutablePlayer instance that is representing the local player. */
		MutablePlayer& Client::getLocalPlayer(void)
		{
			if(!mpLocalPlayer)
			{
				Hashtable properties;
				properties.put(Properties::Player::USER_ID, mAuthenticationValues.getUserID());
				mpLocalPlayer = createMutablePlayer(-1, properties);
			}
			return *mpLocalPlayer;
		}
		
		/**
		   @return the latest locally cached state of the friend list.
		   @remarks
		   You can request the latest state of the local clients friend list from the server by a call to opFindFriends().
		   Listener::onFindFriendsResponse() informs you when the servers response has arrived.
		   The list that is returned by this function reflects the state that the server has sent in its latest response to an update request or in other words the most up to date state that is
		   available locally at the time of the call.
		   @sa opFindFriends(), Listener::onFindFriendsResponse(), FriendInfo, getFriendListAge() */
		const JVector<FriendInfo>& Client::getFriendList(void) const
		{
			return mFriendList;
		}
		
		/**
		   @return the time in ms that has passed since the last update has been applied to the list that is returned by getFriendList() or 0 if either no friendlist is available yet or if a
		   request for an update is in progress at the time of the call. */
		int Client::getFriendListAge(void) const
		{
			return mIsFetchingFriendList||!mFriendListTimestamp ? 0 : GETTIMEMS()-mFriendListTimestamp;
		}

		/**
		   Summarizes (aggregates) the different causes for disconnects of a client.
		   A disconnect can be caused by: errors in the network connection or some vital operation failing
		   (which is considered "high level"). While operations always trigger a call to OnOperationResponse,
		   connection related changes are treated in OnStatusChanged.
		   The DisconnectCause is set in either case and summarizes the causes for any disconnect in a single
		   state value which can be used to display (or debug) the cause for disconnection.
		   @returns the disconnect cause. */
		int Client::getDisconnectedCause(void) const
		{
			return mDisconnectedCause;
		}

		/** @copydoc AuthenticationValues::getUserID() */
		const JString& Client::getUserID(void) const
		{
			return mAuthenticationValues.getUserID();
		}

#if defined EG_PLATFORM_SUPPORTS_CPP11 && defined EG_PLATFORM_SUPPORTS_MULTITHREADING
		/**
		   @return the region code of the Photon Cloud region to which the client has the best ping.
		   @remarks
		   When you specify RegionSelectionMode::BEST on constructing the Client instance, then on first connect the Client will aquire a list of available regions and of their adresses and ping
		   each of them multiple times. Afterwards it will connect to the region with the lowest average ping. After you got a call to Listener::connectReturn(), the region code of the region that
		   the Client has chosen based on the ping results can get accessed by a call to this function. Later calls to connect() will use that cached region code to avoid re-doing the
		   time-consuming ping-procedure and therefor to  keep the time short that is needed for establishing a connection. For the same reason it is recommend that you acquire the result of the
		   ping-procedure through this function and store it in local persistant storage, so that you can use it with RegionSelectionMode::SELECT. This way you can avoid the time-consuming pinging
		   procedure even for the first connect after constructing the class, if you already have the region code for the region with the best ping stored locally from a connection on another
		   Client instance (for example after your app has been shut down and restarted).
		   However in this case you may want to provide an option to your users through which they can delete your locally stored region code and this way trigger a re-pinging on the next
		   construction of a Client instance.
		   @note
		   This function will return an empty string, if no ping result is available (yet), which is the case when another RegionSelectionMode than BEST has been chosen or when you have not
		   received the call to Listener::connectReturn() yet that corresponds to your first successfully established connection since the construction of this class.*/
		const JString& Client::getRegionWithBestPing(void) const
		{
			return mRegionWithBestPing;
		}
#endif



		/**
		   Constructor.
		   @param listener Reference to the application's implementation of the Listener callback interface. Has to be valid for at least the lifetime of the Client instance, which is created by
		   this constructor.
		   @param applicationID A unique ID of your application. Must match one of the appIDs in your dashboard for %Photon Cloud. This parameter gets ignored by %Photon Server.
		   @param appVersion Only clients that use the exact same appVersion can see each other. You can use different values to separate clients with the same appID from each other that should
		   not be able to be matched with each other or to even see each other, i.e. incompatible versions of your game or public, closed-beta, QA, staging and dev clients. This parameter gets
		   ignored by %Photon Server.
		   @param connectionProtocol The protocol to use to connect to the %Photon servers. Must match one of the constants specified in ConnectionProtocol. 
		   @param autoLobbyStats Pass true, if you want to automatically receive updates for the lobby stats, false otherwise. Call opLobbyStats() to explicitly request a lobby stats update.
		   @param regionSelectionMode Determines how the %Photon Cloud Region to which the Client connects should be selected. Must match one of the constants specified in RegionSelectionMode.
		   This parameter gets ignored when connecting to %Photon Server.
		   @param useAlternativePorts Determines if the the standard or the alternative port range should be used. This parameter currently is only relevant when ConnectionProtocol::UDP is passed
		   for parameter connectionProtocol and gets ignored otherwise. A router or firewall might block connections that use one port-range but don't block connections that use the other, so when
		   connecting with one range fails, then you may want to try with the other one.
		   @sa Listener, ConnectionProtocol, RegionSelectionMode, NetworkPort */
		Client::Client(Listener& listener, const JString& applicationID, const JString& appVersion, nByte connectionProtocol, bool autoLobbyStats, nByte regionSelectionMode, bool useAlternativePorts)
#if defined _EG_MS_COMPILER
#	pragma warning(push)
#	pragma warning(disable:4355)
#endif
#ifdef _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
			: mpPeer(allocate<Peer>(static_cast<PhotonListener&>(*this), ConnectionProtocol::WSS))
#else
			: mpPeer(allocate<Peer>(static_cast<PhotonListener&>(*this), connectionProtocol))
#endif
			, mListener(listener)
			, mAppVersion(appVersion)
			, mAppID(applicationID)
			, mPeerCount(0)
			, mRoomCount(0)
			, mMasterPeerCount(0)
			, mLastJoinType(0)
			, mLastLobbyJoinType(0)
			, mLastJoinWasRejoin(false)
			, mLastCacheSliceIndex(0)
			, mpCurrentlyJoinedRoom(NULL)
			, mCachedErrorCodeFromGameServer(ErrorCode::OK)
			, mAutoJoinLobby(true)
			, mpLocalPlayer(NULL)
			, mIsFetchingFriendList(false)
			, mState(PeerStates::Uninitialized)
			, mAuthenticationValues(AuthenticationValues())
			, mAutoLobbyStats(autoLobbyStats)
			, mDisconnectedCause(DisconnectCause::NONE)
			, M_REGION_SELECTION_MODE(regionSelectionMode)
			, M_CONNECTION_PROTOCOL(connectionProtocol)
			, mPingsPerRegion(0)
#ifdef _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
			//#error TODO: change the below line to a simple mUseAuthOnce(true) when the server starts to support sending customInitData over websockets
			// currently websockets can't be used for connections to other server types than nameservers when mUseAuthOnce==true
			, mUseAuthOnce(!ConnectionProtocol::getIsWebSocket(M_CONNECTION_PROTOCOL))
#else
//#error TODO: currently this is set to false for backwards compatibility to older servers, but as soon as newer servers do support the feature of telling the clients which features they support, we should set this to true, when the server claims to support it
			, mUseAuthOnce(false)
#endif
#ifdef _EG_UDP_ENCRYPTION_AVAILABLE
			, mUseUDPEncryption(connectionProtocol==ConnectionProtocol::UDP)
#else
			, mUseUDPEncryption(false)
#endif
			, mUseAlternativePorts(useAlternativePorts)
			, mpPuncherClient(allocate<PuncherClient>(*this, mListener, mLogger))
#ifdef _EG_MS_COMPILER
#	pragma warning(pop)
#endif
		{
			mLogger.setListener(*this);

			// copy appID with '-' removed			
			mAppIDPeerInit = mAppID;
			int p = -1;
			while((p=mAppIDPeerInit.indexOf('-')) >= 0)
				mAppIDPeerInit = mAppIDPeerInit.deleteChars(p, 1);
		}

		/**
		   Destructor. */
		Client::~Client(void)
		{
			destroyMutableRoom(mpCurrentlyJoinedRoom);
			destroyMutablePlayer(mpLocalPlayer);
			for(unsigned int i=0; i<mRoomList.getSize(); ++i)
				destroyRoom(mRoomList[i]);
			deallocate(mpPuncherClient);
			deallocate(mpPeer);
		}

		/**
		   This function starts establishing a connection to a %Photon server. The servers response will arrive in Listener::connectReturn().
		   @details
		   The connection is successfully established when the %Photon client received a valid response from the server. The connect-attempt fails when a network error occurs or when server is not
		   responding.
		   A call to this function starts an asynchronous operation. The result of this operation gets returned through the Listener::connectReturn() callback function.
		   If this function returns false, then the connect-attempt has already failed locally. If it returns true, then either Listener::connectionErrorReturn() or Listener::connectReturn() will
		   get called.
		   The operation was successful, when Listener::connectReturn() got called with errorCode==0.

		   @param authenticationValues An instance of class AuthenticationValues
		   @param username The users display name as shown to other users - not to be confused with the users unique ID for identification and authentication purposes, which is part of the
		   AuthenticationValues
		   @param serverAddress A null terminated string containing the IP address or domain name and optionally the port number to connect to. IP addresses can be in IPv4 or IPv6 format,
		   examples: "192.168.0.1", "192.168.0.1:5055", "udp.gameserver.com", "udp.gameserver.com:5055", "[2002:C0A8:1::]", "[2002:C0A8:1::]:5055". Note that IPv6 addresses must include square
		   brackets to indicate where the address itself ends and the port begins. If no port is given, then the default port for the chosen protocol and server type will be used.
		   @param serverType One of the values in ServerType. Must match the type of the %Photon server that is reachable at the given address and port. Should be ServerType::NAME_SERVER for
		   %Photon Cloud and ServerType::MASTER_SERVER for self-hosted %Photon Server instances. You should NOT directly pass the address of a regions master server with %Photon Cloud, but always
		   connect to the  name server.

		   @returns
		   true, if it could successfully start establishing a connection (the result will be passed in a callback function in this case) or false, if an error occurred and the connection could
		   not be established (no callback function will be called then).
		   @sa
		   disconnect(), NetworkPort */
		bool Client::connect(const AuthenticationValues& authenticationValues, const Common::JString& username, const JString& serverAddress, nByte serverType)
		{
#ifdef _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
			mpPeer->setConnectionProtocol(ConnectionProtocol::WSS);
#endif
#if defined _EG_EMSCRIPTEN_PLATFORM || defined _EG_WEBSOCKET_AVAILABLE
			const int index = serverAddress.indexOf("://");
			const JString serverAddressWithoutProtocol = index==-1?serverAddress:serverAddress.substring(index+JString(L"://").length());
#else
			const JString& serverAddressWithoutProtocol = serverAddress;
#endif
			if(mpPeer->connect(addPortToAddress(serverAddressWithoutProtocol, serverType), mAppIDPeerInit))
			{
				JString userID;
				if(!authenticationValues.getUserID().length())
					userID = mAuthenticationValues.getUserID();
				mAuthenticationValues = authenticationValues;
				if(!authenticationValues.getUserID().length())
					mAuthenticationValues.setUserID(userID);
				getLocalPlayer().setName(username);
				if(serverType == ServerType::MASTER_SERVER)
					mMasterserver = serverAddress;
				mState = serverType==ServerType::NAME_SERVER?PeerStates::ConnectingToNameserver:PeerStates::Connecting;
				return true;
			}
			else
				return false;
		}
		
		/**
		   This function generates a disconnection request that will be sent to the %Photon server. The servers response will arrive in Listener::disconnectReturn().
		   @details
		   If the disconnection is completed successfully, then the Listener::disconnectReturn() callback will be called.

		   @remarks
		   If a game room is joined, when this function gets called, then the local player leaves that room as if opLeaveRoom() has been called with parameter 'willComeBack' set to 'true'.
		   Please see there for further information about leaving rooms.
		   However no call to Listener::leaveRoomReturn() will happen when leaving a game room is triggered through a call to disconnect().
		   @sa
		   connect(), opLeaveRoom() */
		void Client::disconnect(void)
		{
			mState = PeerStates::Disconnecting;
			mpPeer->disconnect();
		}

		/** @copydoc Peer::service() */
		void Client::service(bool dispatchIncomingCommands)
		{
			mpPeer->service(dispatchIncomingCommands);
			mpPuncherClient->service();
		}

		/** @copydoc Peer::serviceBasic() */
		void Client::serviceBasic(void)
		{
			mpPeer->serviceBasic();
			mpPuncherClient->service();
		}

		/** @copydoc Peer::opCustom() */
		bool Client::opCustom(const Photon::OperationRequest& operationRequest, bool sendReliable, nByte channelID, bool encrypt)
		{
			return mpPeer->opCustom(operationRequest, sendReliable, channelID, encrypt&&!mUseUDPEncryption);
		}

		/** @copydoc Peer::sendOutgoingCommands() */
		bool Client::sendOutgoingCommands(void)
		{
			return mpPeer->sendOutgoingCommands();
		}

		/** @copydoc Peer::sendAcksOnly() */
		bool Client::sendAcksOnly(void)
		{
			return mpPeer->sendAcksOnly();
		}

		/** @copydoc Peer::dispatchIncomingCommands() */
		bool Client::dispatchIncomingCommands(void)
		{
			return mpPeer->dispatchIncomingCommands();
		}

		/** @copydoc Peer::fetchServerTimestamp() */
		void Client::fetchServerTimestamp(void)
		{
			mpPeer->fetchServerTimestamp();
		}

		/** @copydoc Peer::resetTrafficStats() */
		void Client::resetTrafficStats(void)
		{
			mpPeer->resetTrafficStats();
		}

		/** @copydoc Peer::resetTrafficStatsMaximumCounters() */
		void Client::resetTrafficStatsMaximumCounters(void)
		{
			mpPeer->resetTrafficStatsMaximumCounters();
		}

		/** @copydoc Peer::vitalStatsToString() */
		Common::JString Client::vitalStatsToString(bool all) const
		{
			return mpPeer->vitalStatsToString(all);
		}

		/** @fn template<typename Ftype> bool Client::opRaiseEvent(bool reliable, const Ftype& parameters, nByte eventCode, const RaiseEventOptions& options)
		   Sends in-game data to other players in the game, who will receive it in their Listener::customEventAction() callback.
		   @details
		   The eventCode should be used to define the event's type and content respectively. The payload has to be one of the datatypes
		   that are listed as supported for values at @link Datatypes serializable datatypes\endlink.

		   This function provides the option to raise events reliably or unreliably. While both result in ordered events, the ones that got sent with the latter option
		   might get lost, causing gaps in the resulting event sequence. On the other hand, they cause less overhead and are optimal for
		   data that is replaced soon.

		   Note: the value of the reliability option only takes effect when the ConnectionProtocol passed to Client() equals ConnectionProtocol::UDP (which is the default for most platforms) and
		   the message is small enough to not get fragmented into several UDP packets (rule of thumb: you can safely assume that the message fits into a single UDP packet, when its payload size
		   is below 1kb), otherwise the message gets sent reliably, even when the reliability option asks for sending it unreliably.

		   Sending is not done immediately, but in intervals of service() calls.

		   It is recommended to keep the payload as simple as possible, especially for events that get raised multiple times per second.
		   This easily adds up to a huge amount of data otherwise.

		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   Listener::customEventAction(), @link Datatypes Table of Datatypes\endlink
		   @param reliable true = the operation will be sent reliably; false = no resend in case of packet loss - will be ignored, when not using ConnectionProtocol::UDP
		   @param parameters the payload of the event to raise - has to be provided in the form of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param eventCode number for arbitrary classification of the type of the event (like '1' for position updates, '2' for chat messages, and so on).
		   @param options see RaiseEventOptions
		 */

		/** @fn template<typename Ftype> bool Client::opRaiseEvent(bool reliable, const Ftype pParameterArray, typename Common::Helpers::ArrayLengthType<Ftype>::type arrSize, nByte eventCode, const RaiseEventOptions& options)
		   @overload
		   @param reliable true = operation will be sent reliably; false = no resend in case of packet loss - will be ignored, when not using UDP as protocol
		   @param pParameterArray the payload array of the event to raise - has to be provided in the form of a 1D array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param arrSize the number of elements in pParameterArray
		   @param eventCode number for arbitrary classification of the type of event (like '1' for position updates, '2' for chat messages, and so on).
		   @param options see RaiseEventOptions
		 */

		/** @fn template<typename Ftype> bool Client::opRaiseEvent(bool reliable, const Ftype pParameterArray, const short* pArrSizes, nByte eventCode, const RaiseEventOptions& options)
		   @overload
		   @param reliable true = operation will be sent reliably; false = no resend in case of packet loss - will be ignored, when not using UDP as protocol
		   @param pParameterArray the payload array of the event to raise - has to be provided in the form of an array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param pArrSizes an array holding the number of elements for each dimension of pParameterArray
		   @param eventCode number for arbitrary classification of the type of event (like '1' for position updates, '2' for chat messages, and so on).
		   @param options see RaiseEventOptions
		 */

		/**
		   Joins the specified lobby.
		   @details
		   This function sends a request to the server to join the specified lobby. If it returns true, then Listener::joinLobbyReturn() gets called when the operation has successfully been
		   finished.
		   Please see <a href="https://doc.photonengine.com/en/realtime/current/reference/matchmaking-and-lobby">Matchmaking Guide</a> regarding the differences between the various lobby types.
		   @remarks
		   A Client instance can only be inside one room at a time. Therefor this operation will fail and return false, if the client is already inside another lobby or inside a game room. Leave
		   the other room first, before calling this operation.
		   @remarks
		   For the same reason entering a game room implicitly causes the client to leave the lobby, so if you want to return to the previously joined lobby after leaving that game room, you must
		   explicitly join it again.
		   @note
		   If the auto-join lobby feature is enabled (which is the default! - it can be turned off by a call to setAutoJoinLobby()), then the client automatically joins the default lobby when
		   successfully connecting to Photon and when leaving a game room. Call setAutoJoinLobby(false) before calling connect() for opJoinLobby() to work properly.

		   @param lobbyName the unique name of the lobby to join
		   @param lobbyType one of the values in LobbyType
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opLeaveLobby(), setAutoJoinLobby(), getAutoJoinLobby(), Listener::joinLobbyReturn() */
		bool Client::opJoinLobby(const JString& lobbyName, nByte lobbyType)
		{
			if(getIsInRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"already in a room");
				return false;
			}

			mLastLobbyJoinType = JoinType::EXPLICIT_JOIN_LOBBY;

			return mpPeer->opJoinLobby(lobbyName, lobbyType);
		}
		
		/**
		   Leaves the currently joined lobby.
		   @details
		   This function sends a request to the server to leave the currently joined lobby. If it returns true, then Listener::leaveLobbyReturn() gets called when the operation has successfully
		   been finished.
		   @remarks
		   This operation will fail and return false if the client does not currently reside inside any lobby.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opJoinLobby(), Listener::leaveLobbyReturn() */
		bool Client::opLeaveLobby(void)
		{
			if(!getIsInLobby())
			{
				EGLOG(DebugLevel::ERRORS, L"lobby isn't currently joined");
				return false;
			}

			return opCustom(OperationRequest(OperationCode::LEAVE_LOBBY), true);
		}
		
		/**
		   Creates and enters a new game room.
		   @details
		   This function sends a request to the server to create the specified game room. If it returns true, then Listener::createRoomReturn() gets called when the operation has been finished.

		   If you don't want to create a unique room name, pass L"" as name and the server will assign a roomName (a GUID as string). %Room names are unique.

		   A room will be attached to the lobby that you have specified in the passed in options. Leave the lobby name empty to attach the room to the lobby you are now in. If you are in no lobby,
		   then the default lobby is used.

		   Multiple lobbies can help to separate players by map or skill or game type. Each room can only be found in one lobby (no matter if defined by name and type or as default).
		   @remarks
		   A Client instance can only be inside one room at a time. Therefor this operation will fail and return false, if the client is already inside another game room. Any lobby the client
		   currently resides in will implicitly be left when entering a game room.
		   @remarks
		   If a room with the specified name does already exist, then the operation will fail and Listener::createRoomReturn() will get called with an error code.
		   @param gameID The name to create a room with. Must be unique and not in use or the room can't be created. If this is an empty string, then the server will assign a GUID as name.
		   @param options An instance of RoomOptions, that can be used to specify various options for room creation.
		   @param expectedUsers Sets a list of user IDs for which the server should reserve slots. Those slots can't be taken by other players.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opJoinOrCreateRoom(), opJoinRoom(), opJoinRandomRoom(), opLeaveRoom(), MutableRoom, RoomOptions, Listener::createRoomReturn() */
		bool Client::opCreateRoom(const JString& gameID, const RoomOptions& options, const JVector<JString>& expectedUsers)
		{
			if(getIsInGameRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"already in a gameroom");
				return false;
			}

			OperationRequestParameters op(mpPeer->opCreateRoomImplementation(mRoomName=gameID, RoomOptions(options).setCustomRoomProperties(getIsOnGameServer()?options.getCustomRoomProperties():Hashtable()).setPropsListedInLobby(getIsOnGameServer()?options.getPropsListedInLobby():JVector<JString>()), getIsOnGameServer()?getLocalPlayer().getCustomProperties():Hashtable(), expectedUsers));

			if(getLocalPlayer().getName().length())
			{
				if((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))
					((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))->getDataAddress()->put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
				else
				{
					Hashtable playerProp;
					playerProp.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
					op.put(ParameterCode::PLAYER_PROPERTIES, ValueObject<Hashtable>(playerProp));
				}
			}

			if(!opCustom(OperationRequest(OperationCode::CREATE_ROOM, op), true))
				return false;

			Hashtable roomProps(Utils::stripToCustomProperties(options.getCustomRoomProperties()));
			roomProps.put(Properties::Room::IS_OPEN, options.getIsOpen());
			roomProps.put(Properties::Room::IS_VISIBLE, options.getIsVisible());
			roomProps.put(Properties::Room::MAX_PLAYERS, options.getMaxPlayers());
			if(options.getDirectMode() != DirectMode::NONE)
				roomProps.put(Properties::Room::DIRECT_MODE, options.getDirectMode());

			MutableRoom* oldRoom = mpCurrentlyJoinedRoom;
			mpCurrentlyJoinedRoom = createMutableRoom(gameID, roomProps, options.getPropsListedInLobby(), options.getPlayerTtl(), options.getEmptyRoomTtl(), options.getSuppressRoomEvents(), options.getPlugins(), options.getPublishUserID(), expectedUsers);
			destroyMutableRoom(oldRoom);
			if(mState != PeerStates::Joining)
				mLastJoinType = JoinType::CREATE_ROOM;
			return true;
		}
		
		/**
		   Joins the specified game room or creates and enters a new game room with the specified ID if such a room does not exist yet.
		   @details
		   This function sends a request to the server to join the specified game room if exists and to create it otherwise. If it returns true, then Listener::joinOrCreateRoomReturn() gets
		   called when the operation has been finished.

		   Unlike opJoinRoom(), this operation does not fail if the room does not exist.
		   This can be useful when you send invitations to a room before actually creating it:
		   Any invited player (whoever is first) can call this and on demand, the room gets created implicitly.

		   This operation does not allow you to re-join a game. To return to a room, use opJoinRoom() with the playerNumber which was assigned previously.
		   @remarks
		   A Client instance can only be inside one room at a time. Therefor this operation will fail and return false, if the client is already inside another game room. Any lobby the client
		   currently resides in will implicitly be left when entering a game room.
		   @remarks
		   If the room is full or closed, then this operation will fail and Listener::joinOrCreateRoomReturn() will get called with an error code.
		   @param gameID A unique identifier for the game room to join or create. If this is an empty string, then the server will create a room and assign a GUID as name.
		   @param options An instance of RoomOptions, that can be used to specify various options for room creation. These options will be ignored when the room already exists.
		   @param cacheSliceIndex Allows to request a specific cache slice - all events in that cache slice and upward slices will be published to the client after joining the room - see
		   Lite::EventCache for further information about cached events.
		   @param expectedUsers Sets a list of user IDs for which the server should reserve slots. Those slots can't be taken by other players. If the room already exists, then this list will
		   be merged with any previously set list of expected users for this room.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opCreateRoom(), opJoinRoom(), opJoinRandomRoom(), opLeaveRoom(), MutableRoom, RoomOptions, Listener::joinOrCreateRoomReturn() */
		bool Client::opJoinOrCreateRoom(const JString& gameID, const RoomOptions& options, int cacheSliceIndex, const JVector<JString>& expectedUsers)
		{
			if(getIsInGameRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"already in a gameroom");
				return false;
			}

			OperationRequestParameters op = mpPeer->opJoinRoomImplementation(mRoomName=gameID, RoomOptions(options).setCustomRoomProperties(getIsOnGameServer()?options.getCustomRoomProperties():Hashtable()).setPropsListedInLobby(getIsOnGameServer()?options.getPropsListedInLobby():JVector<JString>()), getIsOnGameServer()?getLocalPlayer().getCustomProperties():Hashtable(), true, false, getIsOnGameServer()?cacheSliceIndex:0, expectedUsers);
			if(getLocalPlayer().getName().length())
			{
				if((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))
					((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))->getDataAddress()->put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
				else
				{
					Hashtable playerProp;
					playerProp.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
					op.put(ParameterCode::PLAYER_PROPERTIES, ValueObject<Hashtable>(playerProp));
				}
			}

			if(!gameID.length() || !opCustom(OperationRequest(OperationCode::JOIN_ROOM, op), true))
				return false;

			Hashtable roomProps(Utils::stripToCustomProperties(options.getCustomRoomProperties()));
			roomProps.put(Properties::Room::IS_OPEN, options.getIsOpen());
			roomProps.put(Properties::Room::IS_VISIBLE, options.getIsVisible());
			roomProps.put(Properties::Room::MAX_PLAYERS, options.getMaxPlayers());
			if(options.getDirectMode() != DirectMode::NONE)
				roomProps.put(Properties::Room::DIRECT_MODE, options.getDirectMode());

			MutableRoom* oldRoom = mpCurrentlyJoinedRoom;
			mpCurrentlyJoinedRoom = createMutableRoom(gameID, roomProps, options.getPropsListedInLobby(), options.getPlayerTtl(), options.getEmptyRoomTtl(), options.getSuppressRoomEvents(), options.getPlugins(), options.getPublishUserID(), expectedUsers);
			destroyMutableRoom(oldRoom);
			mLastCacheSliceIndex = cacheSliceIndex;
			if(mState != PeerStates::Joining)
				mLastJoinType = JoinType::JOIN_OR_CREATE_ROOM;
			return true;
		}

		/**
		   Joins the specified game room.
		   @details
		   This function sends a request to the server to join the specified game room. If it returns true, then Listener::joinRoomReturn() gets called when the operation has been finished.

		   This function is useful when you are using a lobby to list rooms and know their names.
		   A room's name has to be unique (per region and app version), so it does not matter which lobby the room is in.

		   It's usually better to use opJoinOrCreateRoom() for invitations.
		   Then it does not matter if the room is already setup.
		   @remarks
		   A Client instance can only be inside one room at a time. Therefor this operation will fail and return false, if the client is already inside another game room. Any lobby the client
		   currently resides in will implicitly be left when entering a game room.
		   @remarks
		   If a room with the specified name does not exist or if the room is full or closed, then this operation will fail and Listener::joinRoomReturn() will get called with an error code.
		   @param gameID A unique identifier for the game room to join.
		   @param rejoin Needs to be false if this is the initial join of this room for this client and true if this is a rejoin.
		   @param cacheSliceIndex Allows to request a specific cache slice - all events in that cache slice and upward slices will be published to the client after joining the room - see
		   Lite::EventCache for further information about cached events.
		   @param expectedUsers Sets a list of user IDs for which the server should reserve slots. Those slots can't be taken by other players. This list will be merged with any previously set
		   list of expected users for this room.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opCreateRoom(), opJoinOrCreateRoom(), opJoinRandomRoom(), opLeaveRoom(), MutableRoom, Listener::joinRoomReturn() */
		bool Client::opJoinRoom(const JString& gameID, bool rejoin, int cacheSliceIndex, const JVector<JString>& expectedUsers)
		{
			if(getIsInGameRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"already in a gameroom");
				return false;
			}

			OperationRequestParameters op = mpPeer->opJoinRoomImplementation(mRoomName=gameID, RoomOptions(), getIsOnGameServer()?getLocalPlayer().getCustomProperties():Hashtable(), false, rejoin, getIsOnGameServer()?cacheSliceIndex:0, expectedUsers);
			if(getLocalPlayer().getName().length())
			{
				if((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))
					((ValueObject<Hashtable>*)op.getValue(ParameterCode::PLAYER_PROPERTIES))->getDataAddress()->put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
				else
				{
					Hashtable playerProp;
					playerProp.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
					op.put(ParameterCode::PLAYER_PROPERTIES, ValueObject<Hashtable>(playerProp));
				}
			}

			if(!gameID.length() || !opCustom(OperationRequest(OperationCode::JOIN_ROOM, op), true))
				return false;

			MutableRoom* oldRoom = mpCurrentlyJoinedRoom;
			mpCurrentlyJoinedRoom = createMutableRoom(gameID, Hashtable(), JVector<JString>(), 0, 0, false, NULL, false, expectedUsers);
			destroyMutableRoom(oldRoom);
			mLastJoinWasRejoin = rejoin;
			mLastCacheSliceIndex = cacheSliceIndex;
			if(mState != PeerStates::Joining)
				mLastJoinType = JoinType::JOIN_ROOM;
			return true;
		}
		
		/**
		   Joins a random game room.
		   @details
		   This function sends a request to the server to join a random game room. If it returns true, then Listener::joinRandomRoomReturn() gets called when the operation has been finished.
		   @remarks
		   A Client instance can only be inside one room at a time. Therefor this operation will fail and return false, if the client is already inside another game room. Any lobby the client
		   currently resides in will implicitly be left when entering a game room.
		   @remarks
		   If no rooms are fitting or available (all full, closed or not visible), then this operation will fail and Listener::joinRandomRoomReturn() will get get called with an error code.
		   @param customRoomProperties Used as a filter for matchmaking. The server only considers rooms for which all custom properties match the specified filters. Note that only those custom
		   room properties that have been specified for listing in the lobby will be used for matchmaking, so a rooms custom property can only match a specified filter if it got specified in the list
		   of properties to show in the lobby. All values must be exact matches.
		   @param maxPlayers Must match the value of a rooms maxPlayers property for that room to be considered for matchmaking.
		   @param matchmakingMode Needs to be one of the values in MatchMakingMode
		   @param lobbyName The name of the lobby in which matchmaking should take place. Only rooms that are listed in that lobby will be considered for matchmaking.
		   @param lobbyType The type of the lobby in which matchmaking should take place. Needs to be one of the values in LobbyType. Note that a lobby with the same name, but a different type
		   can not be considered for matchmaking, as a lobby name only needs to be unique among lobbies of the same type.
		   @param sqlLobbyFilter Only used for LobbyType::SQL_LOBBY. This allows 'bigger', 'less', 'or' and 'and' combinations for filtering against certain room properties.
		   @param expectedUsers Sets a list of user IDs for which the server should reserve slots. Those slots can't be taken by other players. This list will be merged with any previously set
		   list of expected users for this room.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opCreateRoom(), opJoinOrCreateRoom(), opJoinRandomRoom(), opLeaveRoom(), MutableRoom, Listener::joinRoomReturn(),
		   <a href="https://doc.photonengine.com/en-us/realtime/current/reference/matchmaking-and-lobby">Matchmaking and Lobby</a> */
		bool Client::opJoinRandomRoom(const Hashtable& customRoomProperties, nByte maxPlayers, nByte matchmakingMode, const JString& lobbyName, nByte lobbyType, const JString& sqlLobbyFilter, const JVector<JString>& expectedUsers)
		{
			if(getIsInGameRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"already in a gameroom");
				return false;
			}
			
			if(!mpPeer->opJoinRandomRoom(customRoomProperties, maxPlayers, matchmakingMode, lobbyName, lobbyType, sqlLobbyFilter, expectedUsers))
				return false;

			MutableRoom* oldRoom = mpCurrentlyJoinedRoom;
			mpCurrentlyJoinedRoom = createMutableRoom(L"", Utils::stripToCustomProperties(customRoomProperties), Common::JVector<Common::JString>(), 0, 0, false, NULL, false, expectedUsers);
			destroyMutableRoom(oldRoom);
			if(mState != PeerStates::Joining)
				mLastJoinType = JoinType::JOIN_RANDOM_ROOM;
			return true;
		}

		/**
		   Leaves the currently joined game room.
		   @details
		   This function sends a request to the server to leave the currently joined game room. If it returns true, then Listener::leaveRoomReturn() gets called when the operation has successfully
		   been finished.
		   @remarks
		   This operation will fail and return false if the client does not currently reside inside any game room.
		   @param willComeBack If this is set to 'true', then the player becomes inactive and the client could later rejoin the room as the very same player. 'false' means the player leaves the
		   room for good. Note that the player only stays inactive for at maximum as many milliseconds as you have set the playerTtl to during  room creation (see RoomOptions::setPlayerTtl()). The
		   default is 'false'.
		   @param sendAuthCookie Pass 'true' to set the sendAuthCookie web flag (please see <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks-v1-2">Webhooks v1.2</a> for further information).
		   The default is 'false'.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opCreateRoom(), opJoinOrCreateRoom(), opJoinRoom(), opJoinRandomRoom(), MutableRoom, RoomOptions, Listener::leaveRoomReturn() */
		bool Client::opLeaveRoom(bool willComeBack, bool sendAuthCookie)
		{
			if(!getIsInGameRoom())
			{
				EGLOG(DebugLevel::ERRORS, L"no gameroom is currently joined");
				return false;
			}

			if(!mpPeer->opLeaveRoom(willComeBack, sendAuthCookie))
				return false;
			mState = PeerStates::Leaving;
			return true;
		}

		/**
		   Requests the rooms and online states for the specified list of friends. All clients should set a unique UserID before connecting. The result can be accessed through getFriendList()
		   after the corresponding call to Listener::onFindFriendsResponse() has been received.
		   @details
		   This function can be called when the caller does not currently reside in a game room to find the rooms played by a selected list of users.
		   The result can be accessed by a call to getFriendList() and is empty before the first response has arrived in Listener::onFindFriendsResponse().
		   getFriendListAge() can be used to retrieve the amount of milliseconds that have passed since the value that is returned by getFriendList() has been updated for the last time.

		   Users identify themselves by passing their UserIDs to AuthenticationValues::setUserID().

		   The list of userIDs must be fetched from some other source (not provided by Photon).
		   @remarks
		   This operation will fail and return false if the client does currently reside inside a game room or if the result for a previous call to this function has not arrived yet.
		   @param friendsToFind An array of unique userIDs.
		   @param numFriendsToFind The element count of friendsToFind.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   getFriendList(), getFriendListAge(), Listener::onFindFriendsResponse() */
		bool Client::opFindFriends(const JString* friendsToFind, short numFriendsToFind)
		{
			if(getIsOnGameServer() || mIsFetchingFriendList)
				return false;
			mLastFindFriendsRequest.removeAllElements();
			for(short i=0; i<numFriendsToFind; ++i)
				mLastFindFriendsRequest.addElement(friendsToFind[i]);
			return mIsFetchingFriendList = mpPeer->opFindFriends(friendsToFind, numFriendsToFind);
		}
		
		/**
		   Sends the specified list of LobbyStatsRequest objects to the server. The corresponding list of LobbyStatsResponse objects arrives in Listener::onLobbyStatsResponse().
		   @details
		   This function can be called when the caller does not currently reside in a game room to retrieve statistics for various lobbies.

		   @remarks
		   This operation will fail and return false if the client does currently reside inside a game room.
		   @note
		   Pass 'true' for the 'autoLobbyStats' parameter of Client() to automatically receive regular stats updates for all lobbies in Listener::onLobbyStatsUpdate(). When doing so, it
		   makes little sense to also additionally call this function. opLobbyStats() should rather be used in combination with passing 'false' for the 'autoLobbyStats' parameter of
		   Client() to achieve fine-grain control of when / how often and for which lobbies you want to retrieve a stats update. This can be useful to reduce traffic when you have lots of
		   lobbies, but only rarely need stats updates for most of them.
		   @param lobbiesToQuery A Common::JVector containing a LobbyStatsRequest instance for each lobby that should be queried.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   Client(), Listener::onLobbyStatsResponse(), Listener::onLobbyStatsUpdate(), LobbyStatsRequest, LobbyStatsResponse */
		bool Client::opLobbyStats(const Common::JVector<LoadBalancing::LobbyStatsRequest>& lobbiesToQuery)
		{
			if(getIsOnGameServer())
			{
				EGLOG(DebugLevel::ERRORS, L"lobby stats can't be retrieved while being inside a game room");
				return false;
			}
			return mpPeer->opLobbyStats(mLobbyStatsRequestList=lobbiesToQuery);
		}

		/**
		   Updates the clients interest groups (for events inside of game rooms).
		   @details
		   This function can be called from inside of a game room to change the list of interest groups inside that room to which the local client is subscribed to.
		   For each opRaiseEvent() call one can specify the interest groups to which that event should be sent in the RaiseEventOptions. When doing so, only clients that are subscribed to those
		   interest groups will receive that event.

		   Note the difference between passing NULL and the address of an empty JVector instance:
		   - NULL won't add/remove any groups.
		   - a JVector without any elements will add/remove all (existing) groups.

		   First, removing groups is executed. This way, you could leave all groups and join only the ones provided.

		   Changes become active not immediately but when the server executes this operation (approximately getRoundTripTime()/2 milliseconds after the Client has sent it).

		   @remarks
		   This operation will fail and return false if the client does not currently reside inside a game room.
		   @param pGroupsToRemove Groups to remove from interest. NULL will not remove any. An empty instance will remove all.
		   @param pGroupsToAdd Groups to add to interest. NULL will not add any. An empty instance will add all existing groups.
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   opRaiseEvent(), RaiseEventOptions::setInterestGroups(), <a href="https://doc.photonengine.com/en-us/realtime/current/reference/interestgroups">Interestgroups</a> */
		bool Client::opChangeGroups(const JVector<nByte>* pGroupsToRemove, const JVector<nByte>* pGroupsToAdd)
		{
			if(!getIsInGameRoom())
				return false;
			return mpPeer->opChangeGroups(pGroupsToRemove, pGroupsToAdd);
		}
		
		/**
		   Used in conjunction with Listener::onCustomAuthenticationIntermediateStep() to implement multi-leg custom authentication.
		   @details
		   While normally custom authentication is single-legged, occasionally a certain service may require multi-leg authentication. This means that the client sends some authentication data
		   to the server that you pass when calling connect() and the server does not respond with a final result (successful connect or failed connect attempt due to an authentication error),
		   but with some intermediate result data that gets passed to your Listener::onCustomAuthenticationIntermediateStep() implementation and that is needed by your application to acquire the
		   authentication data for the next step of the authentication process. You can then pass that next step data to this function to continue the authentication process that you have started
		   with the connect() call.

		   @remarks
		   This operation will fail and return false if the client is not currently expecting it to be called. A call by you is only expected after you have received a call to
		   Listener::onCustomAuthenticationIntermediateStep() beforehand and only one call to this function is expected after each received call to
		   Listener::onCustomAuthenticationIntermediateStep(). If a call is expected, then the connection flow pauses until this call has been made.
		   No call to this function is ever expected if the custom authentication that you have set up is single-legged (which is by far more common) or if you have not set up any custom
		   authentication at all, which means that this function will always fail in these scenarios.
		   @param authenticationValues An instance of class AuthenticationValues
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   connect(), Listener::onCustomAuthenticationIntermediateStep(), AuthenticationValues */
		bool Client::opCustomAuthenticationSendNextStepData(const AuthenticationValues& authenticationValues)
		{
			if(mState != PeerStates::WaitingForCustomAuthenticationNextStepCall)
				return false;
			mState = PeerStates::ConnectedToNameserver;
			mAuthenticationValues = authenticationValues;
			return authenticate();
		}
#if defined _EG_PSVITA_PLATFORM
#	pragma warning (push)
#	pragma warning (disable: 1831)
#elif !defined _EG_MS_COMPILER
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcomment"
#endif
		/**
		   Used in conjunction with Listener::onAvailableRegions() and RegionSelectionMode::SELECT to select a certain server region to connect to.
		   @details
		   If you pass RegionSelectionMode::SELECT for parameter 'regionSelectionMode' to Client(), then the Client does not automatically choose a server region to connect to on its own during
		   the connection flow, but upon retrieving the list of available regions and the list of server addresses that can be used to ping those regions it passes those lists to your
		   implementation of Listener::onAvailableRegions() and pauses the connection flow. You then need to choose one of the available regions and select it by passing its name to this function
		   to continue the connection flow.

		   The list of available regions for Photon Public Cloud is available at <a href="https://doc.photonengine.com/en-us/realtime/current/reference/regions">Regions</a>.
		   However more regions might be added over time after you have released your application and the list of available regions might differ when your appID is associated with a dedicated
		   Cloud or when you connect to a non-default name server address. Also a certain region might be temporarily unavailable for maintenance. Furthermore some regions might consist out of
		   multiple different clusters, while others don't.
		   Therefor you should always assure that the region name that you pass to this function actually matches one of the entries in the list of available regions. Also be prepared to select a
		   fall back option in case that your preferred region is not available.

		   A typical list of available regions might look like this (more or less regions might be available and the order of the entries is undefined and might change without notice):
		   "eu", "us", "usw", "cae", "asia", "jp", "au", "sa", "in", "kr"

		   When multiple clusters per region are set up for your appID for some regions, then the list might look like this:
		   "eu/Default", "eu/Cluster2", "us/Default", "us/Cluster2", "usw", "cae", "asia", "jp", "au", "sa", "in", "kr"

		   Examples for valid strings to pass for the 'eu' region for parameter 'selectedRegion' with the above example lists (adapt accordingly for other regions):
		   <ul>
		   <li> "eu" - Valid when at least one cluster is available in region 'eu', selects the default cluster for that region.
		   <li> "eu/Default" - Only valid when a cluster with the exact name "Default" is available in region 'eu'.
		   <li> "eu/Cluster2" - Only valid when a cluster with the exact name "Cluster2" is available in region 'eu'.
		   <li> "eu/*" - Only valid when at least 2 clusters are setup in region 'eu' of which at least one is available. The server randomly selects one of the available clusters in the specified
		   region. This string is not contained in the list of available regions and must be constructed by your code when it is valid and when you intend to select a random cluster.
		   </ul>
		   In case of the server randomly selecting a cluster, parameter 'cluster' of Listener::connectReturn() contains the name of the cluster to which the client has connected. Otherwise that
		   parameter is an empty string.
		   @remarks
		   This operation will fail and return false if 'regionSelectionMode' has not been set to RegionSelectionMode::SELECT upon construction of this class instance.
		   @param selectedRegion Must be a valid region name that matches one of the entries in the list of available regions that got passed to Listener::onAvailableRegions()
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   Client(), connect(), Listener::onAvailableRegions() */
#if defined _EG_PSVITA_PLATFORM
#	pragma warning (pop)
#elif !defined _EG_MS_COMPILER
#	pragma GCC diagnostic pop
#endif
		bool Client::selectRegion(const JString& selectedRegion)
		{
			if(M_REGION_SELECTION_MODE != RegionSelectionMode::SELECT)
			{
				EGLOG(DebugLevel::ERRORS, L"this function should only be called, when you have explicitly specified in the constructor to use RegionSelectionMode::SELECT.");
				return false;
			}
			else
			{
				mSelectedRegion = selectedRegion;
				return authenticate();
			}
		}

		/**
		   Reconnects the the server and rejoins the last previously joined room.
		   @details
		   This function reconnects directly to the game server to which it has previously been connected to and sends a request to the server to join the last previously joined game room. If it
		   returns true, then Listener::joinRoomReturn() gets called when the operation has been finished.

		   The usual requirements for a rejoin apply, meaning the room must still exist, the local player must have entered it before, but it must not have left it for good, but only have
		   become inactive and the playerTTL for the local player in that room must not have run out yet, otherwise this operation will fail and Listener::joinRoomReturn() will get called with an
		   error code.
		   @remarks
		   This function will fail and return false if no game room has been entered since the creation of the class instance or if the client is still/already in a connected state.
		   @remarks
		   reconnectAndRejoin() is quicker than the combination of connect() and opJoinRoom().
		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   connect(), opJoinRoom(), Listener::joinRoomReturn()  */
		bool Client::reconnectAndRejoin(void)
		{
			if(!mGameserver.length())
			{
				EGLOG(DebugLevel::ERRORS, L"No gameserver address known.");
				return false;
			}
			EGLOG(DebugLevel::INFO, L"");
			mState = PeerStates::ConnectingToGameserver;
			mLastJoinWasRejoin = true;
			if(!callPeerConnect(mGameserver))
				return false;
			mLastJoinType = JoinType::JOIN_ROOM;
			return true;
		}

		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype& parameters, int targetPlayer, bool fallbackRelay)
		   Sends in-game data to other players in the game, who will receive it in their Listener::onDirectMessage() callback. Data that gets sent with this function, gets sent over a direct peer to peer connection, when possible.
		   @details
		   For the %Photon clients to attempt to establish direct peer to peer connections to each other when entering a room you need set the the DirectMode Option either to DirectMode::MASTER_TO_ALL or to DirectMode::ALL_TO_ALL on the RoomOptions instance that you provide
		   on room creation.
		   Only when a direct connection to a certain client exists, data can be exchanged with it directly. Otherwise this function either falls back to sending it through the %Photon game server with opRaiseEvent(), or doesn't send it at all, depending on the value of the
		   'fallbackRelay' parameter.
		   Data transfer on a direct p2p connection always happens unreliably over UDP even when a different connection protocol has been chosen for connections to the Photon servers in the constructor of this class.
		   However data transfer over the fall-back relay uses the protocol that has been selected for connections to the Photon server when calling the constructor.

		   It is recommended to keep the payload as simple as possible, as the data is typically sent multiple times per second.
		   This easily adds up to a huge amount of data otherwise.
		   @note
		   A direct connection to a certain client is not guaranteed to exist, even when RoomOptions::setDirectMode() specifies that the Clients should attempt to establish it, as NAT punch-through does not have a 100% success rate. In the case that a direct message is
		   preferable, but a relayed one would be acceptable when no direct connection exists, the 'fallbackRelay' option comes into play.
		   @note
		   Furthermore if a client looses its connection to %Photon while other clients can still reach the server, then that client most likely lost its internet connection and direct messages won't reach it anymore either.
		   @remarks
		   This function provides a rather low-level raw UDP socket like way to send data.
		   If you need any higher level functionality like reliable data delivery, support for bigger messages, message caching, interest groups or webforwarding, then please use opRaiseEvent() instead.
		   @sa
		   Listener::onDirectMessage(), opRaiseEvent(), DirectMode, RoomOptions::getDirectMode(), RoomOptions::setDirectMode()
		   @param parameters the data to send - has to be provided in the form of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param targetPlayer the player number of the intended receiver of the message - must be the number of another active player inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) when no direct connection to the other client exists, false otherwise
		   @returns true, if the request could successfully be sent (this does not guarantee that it will be received), false otherwise.
		 */
		
		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype pParameterArray, typename Common::Helpers::ArrayLengthType<Ftype>::type arrSize, int targetPlayer, bool fallbackRelay)
		   @overload
		   @param pParameterArray the data to send - has to be provided in the form of a 1D array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param arrSize the number of elements in pParameterArray
		   @param targetPlayer the player number of the intended receiver of the message - must be the number of another active player inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) for all specified receivers to which no direct connection exists, false otherwise
		   @returns true, if the request could successfully be sent (this does not guarantee that it will be received), false otherwise.
		 */
		
		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype pParameterArray, const short* pArrSizes, int targetPlayer, bool fallbackRelay)
		   @overload
		   @param pParameterArray the the data to send - has to be provided in the form of an array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param pArrSizes an array holding the number of elements for each dimension of pParameterArray
		   @param targetPlayer the player number of the intended receiver of the message - must be the number of another active player inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) for all specified receivers to which no direct connection exists, false otherwise
		   @returns true, if the request could successfully be sent (this does not guarantee that it will be received), false otherwise.
		 */
		
		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype& parameters, const Common::JVector<int>& targetPlayers, bool fallbackRelay)
		   @overload
		   @param parameters the data to send - has to be provided in the form of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param targetPlayers the player numbers of the intended receivers of the message - must be the numbers of other active players inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) for all specified receivers to which no direct connection exists, false otherwise
		   @returns the number of target players, for which the request could successfully be sent (this does not guarantee that it will be received).
		 */
		
		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype pParameterArray, typename Common::Helpers::ArrayLengthType<Ftype>::type arrSize, const Common::JVector<int>& targetPlayers, bool fallbackRelay)
		   @overload
		   @param pParameterArray the data to send - has to be provided in the form of a 1D array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param arrSize the number of elements in pParameterArray
		   @param targetPlayers the player numbers of the intended receivers of the message - must be the numbers of other active players inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) for all specified receivers to which no direct connection exists, false otherwise
		   @returns the number of target players, for which the request could successfully be sent (this does not guarantee that it will be received).
		 */
		
		/** @fn template<typename Ftype> bool Client::sendDirect(const Ftype pParameterArray, const short* pArrSizes, const Common::JVector<int>& targetPlayers, bool fallbackRelay)
		   @overload
		   @param pParameterArray the data to send - has to be provided in the form of an array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink - must be less than 1200 bytes
		   @param pArrSizes an array holding the number of elements for each dimension of pParameterArray
		   @param targetPlayers the player numbers of the intended receivers of the message - must be the numbers of other active players inside the same room as the sender
		   @param fallbackRelay true if the %Photon game server that hosts the room should be used as a fallback relay (by an automatic call to opRaiseEvent()) for all specified receivers to which no direct connection exists, false otherwise
		   @returns the number of target players, for which the request could successfully be sent (this does not guarantee that it will be received).
		 */

		int Client::sendDirect(const JVector<nByte>& buffer, const JVector<int>& targetPlayers, bool fallbackRelay)
		{
			JVector<int> availablePlayers(getCurrentlyJoinedRoom().getPlayerCount());
			int localPlayer = getLocalPlayer().getNumber();
			if(targetPlayers.getSize())
			{
				for(unsigned int i=0; i<targetPlayers.getSize(); ++i)
				{
					int nr = targetPlayers[i];
					if(nr != localPlayer)
					{
						// check if requested player joined
						// TODO: cache joined players numbers in Client in sorted array
						for(nByte i=0; i<getCurrentlyJoinedRoom().getPlayerCount(); ++i) 
							if(nr == getCurrentlyJoinedRoom().getPlayers()[i]->getNumber())
								availablePlayers.addElement(nr);
					}
				}
			}
			else
			{
				for(nByte i=0; i<getCurrentlyJoinedRoom().getPlayerCount(); ++i) 
				{
					int nr = getCurrentlyJoinedRoom().getPlayers()[i]->getNumber();
					if(nr != localPlayer)
						availablePlayers.addElement(nr);
				}
			}
			int sent = mpPuncherClient->sendDirect(buffer, availablePlayers, fallbackRelay);
			return sent;
		}

		// protocol implementations

		void Client::onOperationResponse(const OperationResponse& operationResponse)
		{
			EGLOG(operationResponse.getReturnCode()?DebugLevel::ERRORS:DebugLevel::INFO, operationResponse.toString(true));
			// Use the secret whenever we get it, no matter the operation code.
			if(operationResponse.getParameters().contains(ParameterCode::SECRET))
			{
				AuthenticationValuesSecretSetter::setSecret(mAuthenticationValues, ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::SECRET)).getDataCopy());
				EGLOG(DebugLevel::INFO, L"Server returned secret");
				mListener.onSecretReceival(mAuthenticationValues.getSecret());
			}

			switch(operationResponse.getOperationCode())
			{
			case OperationCode::AUTHENTICATE:
			case OperationCode::AUTH_ONCE:
				{
					EGLOG(operationResponse.getReturnCode()?DebugLevel::ERRORS:DebugLevel::INFO, operationResponse.toString(true, true));
					if(operationResponse.getReturnCode())
					{
						EGLOG(DebugLevel::ERRORS, L"authentication failed with errorcode %d: %ls", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
						
						switch(operationResponse.getReturnCode())
						{
						case ErrorCode::INVALID_AUTHENTICATION:
							mDisconnectedCause = DisconnectCause::INVALID_AUTHENTICATION;
							break;
						case ErrorCode::CUSTOM_AUTHENTICATION_FAILED:
							mDisconnectedCause = DisconnectCause::CUSTOM_AUTHENTICATION_FAILED;
							break;
						case ErrorCode::INVALID_REGION:
							mDisconnectedCause = DisconnectCause::INVALID_REGION;
							break;
						case ErrorCode::MAX_CCU_REACHED:
							mDisconnectedCause = DisconnectCause::MAX_CCU_REACHED;
							break;
						case ErrorCode::OPERATION_DENIED:
							mDisconnectedCause = DisconnectCause::OPERATION_NOT_ALLOWED_IN_CURRENT_STATE;
							break;
						}

						handleConnectionFlowError(mState, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
						break;
					}
					else
					{
						if(mState == PeerStates::ConnectedToNameserver || mState == PeerStates::Connected || mState == PeerStates::ConnectedComingFromGameserver)
						{
							if(operationResponse.getParameters().contains(ParameterCode::USER_ID))
							{
								mAuthenticationValues.setUserID(ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::USER_ID)).getDataCopy());
								if(mpCurrentlyJoinedRoom)
									PlayerMovementInformant::onLeaveLocal(*mpCurrentlyJoinedRoom, getLocalPlayer().getNumber());
								Hashtable properties = getLocalPlayer().getCustomProperties();
								properties.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
								properties.put(Properties::Player::USER_ID, mAuthenticationValues.getUserID());
								destroyMutablePlayer(mpLocalPlayer);
								mpLocalPlayer = createMutablePlayer(-1, properties);
								EGLOG(DebugLevel::INFO, L"Setting userID sent by server: " + mAuthenticationValues.getUserID().toString());
							}
							if(operationResponse.getParameters().contains(ParameterCode::NICK_NAME))
							{
								mpLocalPlayer->setName(ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::NICK_NAME)).getDataCopy());
								EGLOG(DebugLevel::INFO, L"Setting nickname sent by server: " + mpLocalPlayer->getName().toString());
							}
						}

						if(mState == PeerStates::ConnectedToNameserver)
						{
							if(operationResponse.getParameters().contains(ParameterCode::ENCRYPTION_DATA))
							{
								Dictionary<nByte, Object> encryptionData = ValueObject<Dictionary<nByte, Object> >(operationResponse.getParameterForCode(ParameterCode::ENCRYPTION_DATA)).getDataCopy();
								switch(ValueObject<nByte>(encryptionData.getValue(EncryptionDataParameters::MODE)).getDataCopy())
								{
								case EncryptionMode::USER_DATA_ENCRYPTION_NO_DIFFIE_HELLMAN:
								{
									JVector<nByte> secret(*ValueObject<nByte*>(*encryptionData.getValue(EncryptionDataParameters::SECRET1)).getDataAddress(), *encryptionData.getValue(EncryptionDataParameters::SECRET1)->getSizes());
									mpPeer->initUserDataEncryption(secret);
								}
								break;
								case EncryptionMode::UDP_TRAFFIC_ENCRYPTION:
								{
#ifdef _EG_ENCRYPTOR_AVAILABLE
									JVector<nByte> encryptSecret(*ValueObject<nByte*>(*encryptionData.getValue(EncryptionDataParameters::SECRET1)).getDataAddress(), *encryptionData.getValue(EncryptionDataParameters::SECRET1)->getSizes());
									JVector<nByte> HMACSecret(*ValueObject<nByte*>(*encryptionData.getValue(EncryptionDataParameters::SECRET2)).getDataAddress(), *encryptionData.getValue(EncryptionDataParameters::SECRET2)->getSizes());
									mpPeer->initUDPEncryption(encryptSecret, HMACSecret);
#else
									EGLOG(DebugLevel::ERRORS, L"UDP packet encryption is not supported on this platform.");
#endif
								}
								break;
								default:
									EGLOG(DebugLevel::ERRORS, L"Unsupported encryption mode!");
									break;
								}
							}
							if(operationResponse.getParameters().contains(ParameterCode::CLUSTER))
								mCluster = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::CLUSTER)).getDataCopy();
							if(operationResponse.getParameters().contains(ParameterCode::DATA))
							{
								mState = PeerStates::WaitingForCustomAuthenticationNextStepCall;
								mListener.onCustomAuthenticationIntermediateStep(*ValueObject<Dictionary<JString, Object> >(operationResponse.getParameterForCode(ParameterCode::DATA)).getDataAddress());
								break;
							}
							else
							{
								mState = PeerStates::DisconnectingFromNameserver;
								mMasterserver = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getDataCopy();
								if(mUseAlternativePorts)
									mMasterserver = mMasterserver.replace(JString(L":")+NetworkPort::UDP::MASTER, JString(L":")+NetworkPort::UDPAlternative::MASTER);
								mpPeer->disconnect();
							}
						}
						else 
							onArrivalAndAuthentication();
					}
				}
				break;
			case OperationCode::CREATE_ROOM:
			case OperationCode::JOIN_ROOM:
				{
					if(getIsOnGameServer())
					{
						if(operationResponse.getReturnCode())
						{
							EGLOG(DebugLevel::ERRORS, L"%ls failed with errorcode %d: %ls. Client is therefore returning to masterserver!", operationResponse.getOperationCode()==OperationCode::CREATE_ROOM?L"opCreateRoom":L"opJoinRoom", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
							handleConnectionFlowError(mState, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
							break;
						}

						mState = PeerStates::Joined;
						int nr = ValueObject<int>(operationResponse.getParameterForCode(ParameterCode::PLAYERNR)).getDataCopy();
						Hashtable properties = getLocalPlayer().getCustomProperties();
						properties.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
						properties.put(Properties::Player::USER_ID, mAuthenticationValues.getUserID());
						destroyMutablePlayer(mpLocalPlayer);
						PlayerMovementInformant::onEnterLocal(*mpCurrentlyJoinedRoom, *(mpLocalPlayer=createMutablePlayer(nr, properties)));

						Hashtable roomProperties(ValueObject<Hashtable>(operationResponse.getParameterForCode(ParameterCode::ROOM_PROPERTIES)).getDataCopy());
						Hashtable playerProperties(ValueObject<Hashtable>(operationResponse.getParameterForCode(ParameterCode::PLAYER_PROPERTIES)).getDataCopy());
						int* pPlayerList = ValueObject<int*>(operationResponse.getParameterForCode(ParameterCode::PLAYER_LIST)).getDataCopy();
						short playerListSize = *ValueObject<int*>(operationResponse.getParameterForCode(ParameterCode::PLAYER_LIST)).getSizes();
						for(short i=0; i<playerListSize; ++i)
							if(pPlayerList[i] != mpLocalPlayer->getNumber())
								PlayerMovementInformant::onEnterRemote(*mpCurrentlyJoinedRoom, pPlayerList[i], Hashtable());
						deallocateArray(pPlayerList);
						readoutProperties(roomProperties, playerProperties, true, 0);

						if(mpCurrentlyJoinedRoom->getDirectMode() != DirectMode::NONE)
							initPuncher();

						switch(mLastJoinType)
						{
						case JoinType::CREATE_ROOM:
							mListener.createRoomReturn(nr, roomProperties, playerProperties, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
							break;
						case JoinType::JOIN_OR_CREATE_ROOM:
							mListener.joinOrCreateRoomReturn(nr, roomProperties, playerProperties, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
							break;
						case JoinType::JOIN_ROOM:
							mListener.joinRoomReturn(nr, roomProperties, playerProperties, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
							break;
						case JoinType::JOIN_RANDOM_ROOM:
							mListener.joinRandomRoomReturn(nr, roomProperties, playerProperties, operationResponse.getReturnCode(), operationResponse.getDebugMessage());
							break;
						default:
							break;
						}
						break;
					}
					else
					{
						switch(operationResponse.getOperationCode())
						{
						case OperationCode::CREATE_ROOM:
							{
								if(operationResponse.getReturnCode())
								{
									EGLOG(DebugLevel::ERRORS, L"opCreateRoom failed with errorcode %d: %ls. Client is therefore staying on masterserver!", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
									mListener.createRoomReturn(0, Hashtable(), Hashtable(), operationResponse.getReturnCode(), operationResponse.getDebugMessage());
									break;
								}
								JString gameID = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ROOM_NAME)).getDataCopy();
								if(gameID.length()) // is only sent by the server's response, if it has not been sent with the client's request before!
									mRoomName = gameID;

								mGameserver = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getDataCopy();
								if(mUseAlternativePorts)
									mGameserver = mGameserver.replace(JString(L":")+NetworkPort::UDP::GAME, JString(L":")+NetworkPort::UDPAlternative::GAME);
								mState = PeerStates::DisconnectingFromMasterserver;
								mpPeer->disconnect();
							}
							break;
						case OperationCode::JOIN_ROOM:
							if(operationResponse.getReturnCode())
							{
								EGLOG(DebugLevel::ERRORS, L"opJoinRoom failed with errorcode %d: %ls. Client is therefore staying on masterserver!", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
								mListener.joinRoomReturn(0, Hashtable(), Hashtable(), operationResponse.getReturnCode(), operationResponse.getDebugMessage());
								break;
							}

							mGameserver = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getDataCopy();
							if(mUseAlternativePorts)
								mGameserver = mGameserver.replace(JString(L":")+NetworkPort::UDP::GAME, JString(L":")+NetworkPort::UDPAlternative::GAME);
							mState = PeerStates::DisconnectingFromMasterserver;
							mpPeer->disconnect();
							break;
						default:
							break;
						}
					}
				}
				break;
			case OperationCode::JOIN_RANDOM_ROOM:
				if(operationResponse.getReturnCode())
				{
					EGLOG(DebugLevel::ERRORS, L"opJoinRandomRoom failed with errorcode %d: %ls. Client is therefore staying on masterserver!", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
					mListener.joinRandomRoomReturn(0, Hashtable(), Hashtable(), operationResponse.getReturnCode(), operationResponse.getDebugMessage());
					break;
				}

				// store the ID of the random game, joined on the masterserver, so that we know, which game to join on the gameserver
				mRoomName = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ROOM_NAME)).getDataCopy();
				mGameserver = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getDataCopy();
				if(mUseAlternativePorts)
					mGameserver = mGameserver.replace(JString(L":")+NetworkPort::UDP::GAME, JString(L":")+NetworkPort::UDPAlternative::GAME);
				mState = PeerStates::DisconnectingFromMasterserver;
				mpPeer->disconnect();
				break;
			case OperationCode::JOIN_LOBBY:
				{
					int oldState = mState;
					mState = PeerStates::JoinedLobby;
					if(mLastLobbyJoinType == JoinType::AUTO_JOIN_LOBBY)
						onConnectToMasterFinished(oldState==PeerStates::AuthenticatedComingFromGameserver);
					else
						mListener.joinLobbyReturn();
				}
				break;
			case OperationCode::LEAVE_LOBBY:
				mState = PeerStates::Authenticated;
				mListener.leaveLobbyReturn();
				break;
			case OperationCode::LEAVE:
				{
					PlayerMovementInformant::onLeaveLocal(*mpCurrentlyJoinedRoom, getLocalPlayer().getNumber());
					Hashtable properties = getLocalPlayer().getCustomProperties();
					properties.put(Properties::Player::PLAYERNAME, getLocalPlayer().getName());
					properties.put(Properties::Player::USER_ID, getLocalPlayer().getUserID());
					destroyMutablePlayer(mpLocalPlayer);
					mpLocalPlayer = createMutablePlayer(-1, properties);
					mIsFetchingFriendList = false;
					mState = PeerStates::DisconnectingFromGameserver;
					mpPeer->disconnect();
				}
				break;
			case OperationCode::FIND_FRIENDS:
				{
					mIsFetchingFriendList = false;
					if(operationResponse.getReturnCode())
					{
						EGLOG(DebugLevel::ERRORS, L"opFindFriends failed with errorcode %d: %ls.", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
						break;
					}
					ValueObject<bool*> onlineList = operationResponse.getParameterForCode(ParameterCode::FIND_FRIENDS_RESPONSE_ONLINE_LIST);
					ValueObject<JString*> roomList = operationResponse.getParameterForCode(ParameterCode::FIND_FRIENDS_RESPONSE_ROOM_ID_LIST);
					bool* pOnlineList = *onlineList.getDataAddress();
					JString* pRoomList = *roomList.getDataAddress();
					mFriendList.removeAllElements();
					for(short i=0; i<static_cast<short>(mLastFindFriendsRequest.getSize()) && i<*onlineList.getSizes() && i<*roomList.getSizes(); ++i)
						mFriendList.addElement(FriendInfo(mLastFindFriendsRequest[i], pOnlineList[i], pRoomList[i]));
					if(!(mFriendListTimestamp=GETTIMEMS()))
						mFriendListTimestamp = 1;
					mListener.onFindFriendsResponse();
				}
				break;
			case OperationCode::LOBBY_STATS:
				{
					if(operationResponse.getReturnCode())
					{
						EGLOG(DebugLevel::ERRORS, L"opLobbyStats failed with errorcode %d: %ls.", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
						break;
					}
					ValueObject<JString*> namesList = operationResponse.getParameterForCode(ParameterCode::LOBBY_NAME);
					ValueObject<nByte*> typesList = operationResponse.getParameterForCode(ParameterCode::LOBBY_TYPE);
					ValueObject<int*> peersList = operationResponse.getParameterForCode(ParameterCode::PEER_COUNT);
					ValueObject<int*> roomsList = operationResponse.getParameterForCode(ParameterCode::ROOM_COUNT);

					int* peers = *peersList.getDataAddress();
					int* rooms = *roomsList.getDataAddress();

					JVector<LobbyStatsResponse> res;
					if(namesList.getType() != TypeCode::EG_NULL) 
					{
						JString* names = *namesList.getDataAddress();
						nByte* types = *typesList.getDataAddress();
						for(int i=0; i<*namesList.getSizes(); ++i) 
							res.addElement(LobbyStatsResponse(names[i], types[i], peers[i], rooms[i]));
					}
					else 
					{
						for(int i=0; i<static_cast<int>(mLobbyStatsRequestList.getSize()); ++i) 
						{
							int peerCount = i < *peersList.getSizes() ? peers[i] : 0;
							int roomCount = i < *roomsList.getSizes() ? rooms[i] : 0;
							res.addElement(LobbyStatsResponse(mLobbyStatsRequestList[i].getName(), mLobbyStatsRequestList[i].getType(), peerCount, roomCount));
						}
					}
					mListener.onLobbyStatsResponse(res);
				}
				break;
			case OperationCode::GET_REGIONS:
				{
					if(operationResponse.getReturnCode())
					{
						EGLOG(DebugLevel::ERRORS, L"GetRegions failed with errorcode %d: %ls.", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
						break;
					}
					mAvailableRegions = JVector<JString>(*ValueObject<JString*>(operationResponse.getParameterForCode(ParameterCode::REGION)).getDataAddress(), *ValueObject<JString*>(operationResponse.getParameterForCode(ParameterCode::REGION)).getSizes());
					mAvailableRegionServers = JVector<JString>(*ValueObject<JString*>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getDataAddress(), *ValueObject<JString*>(operationResponse.getParameterForCode(ParameterCode::ADDRESS)).getSizes());
					switch(M_REGION_SELECTION_MODE)
					{
					case RegionSelectionMode::DEFAULT:
					default:
						mSelectedRegion = mAvailableRegions[0];
						authenticate();
						break;
					case RegionSelectionMode::SELECT:
						mListener.onAvailableRegions(mAvailableRegions, mAvailableRegionServers);
						break;
#if defined EG_PLATFORM_SUPPORTS_CPP11 && defined EG_PLATFORM_SUPPORTS_MULTITHREADING
					case RegionSelectionMode::BEST:
						if(getRegionWithBestPing().length())
						{
							mSelectedRegion = getRegionWithBestPing();
							authenticate();
						}
						else
							pingBestRegion(M_PINGS_PER_REGION);
						break;
#endif
					}
				}
				break;
			case OperationCode::RPC:
				{
					if(operationResponse.getReturnCode())
						EGLOG(DebugLevel::ERRORS, L"WebRpc failed with errorcode %d: %ls.", operationResponse.getReturnCode(), operationResponse.getDebugMessage().cstr());
					int returnCode = ValueObject<int>(operationResponse.getParameterForCode(ParameterCode::RPC_CALL_RET_CODE)).getDataCopy();
					ValueObject<JString> uriPathObj = ValueObject<JString>(operationResponse.getParameterForCode(ParameterCode::URI_PATH));
					Common::JString* uriPath = uriPathObj.getDataAddress();
					ValueObject<Dictionary<Object, Object> > returnDataObj = ValueObject<Dictionary<Object, Object> >(operationResponse.getParameterForCode(ParameterCode::RPC_CALL_PARAMS));
					Dictionary<Object, Object>* returnData = returnDataObj.getDataAddress();
					mListener.webRpcReturn(operationResponse.getReturnCode(), operationResponse.getDebugMessage(), uriPath?*uriPath:JString(), returnCode, returnData?*returnData:Dictionary<Object, Object>());
				}
				break;
			default:
				mListener.onCustomOperationResponse(operationResponse);
				break;
			}
		}

		void Client::onStatusChanged(int statusCode)
		{
			switch(statusCode)
			{
			case StatusCode::CONNECT:
				{
					if(mState == PeerStates::ConnectingToNameserver)
					{
						EGLOG(DebugLevel::INFO, L"connected to nameserver");
						mState = PeerStates::ConnectedToNameserver;
					}
					else if(mState == PeerStates::ConnectingToGameserver)
					{
						EGLOG(DebugLevel::INFO, L"connected to gameserver");
						mState = PeerStates::ConnectedToGameserver;
					}
					else
					{
						EGLOG(DebugLevel::INFO, L"connected to masterserver");
						mState = mState == PeerStates::Connecting ? PeerStates::Connected : PeerStates::ConnectedComingFromGameserver;
					}
					if(
#if defined _EG_EMSCRIPTEN_PLATFORM || defined _EG_WEBSOCKET_AVAILABLE
						mpPeer->getConnectionProtocol() != ConnectionProtocol::WSS &&
#endif
						(!mUseAuthOnce || !mAuthenticationValues.getSecret().length()))
						mpPeer->establishEncryption();
					else if(mState == PeerStates::ConnectedToNameserver)
						mpPeer->opGetRegions(false, mAppID);
					else if(!mUseAuthOnce)
						authenticate();
				}
				break;
			case StatusCode::DISCONNECT:
				{
					mIsFetchingFriendList = false;
					if(mState == PeerStates::DisconnectingFromNameserver)
					{
#ifdef _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
						mpPeer->setConnectionProtocol(M_CONNECTION_PROTOCOL);
#endif
						callPeerConnect(mMasterserver);
						mState = PeerStates::Connecting;
					}
					else if(mState == PeerStates::DisconnectingFromMasterserver)
					{
						callPeerConnect(mGameserver);
						mState = PeerStates::ConnectingToGameserver;
					}
					else if(mState == PeerStates::DisconnectingFromGameserver)
					{
						callPeerConnect(mMasterserver);
						mState = PeerStates::ConnectingToMasterserver;
					}
					else
					{
						mState = PeerStates::PeerCreated;
						mListener.disconnectReturn();
					}
				}
				break;
			case StatusCode::ENCRYPTION_ESTABLISHED:
				if(mState == PeerStates::ConnectedToNameserver)
					mpPeer->opGetRegions(true, mAppID);
				else
					authenticate();
				break;
			case StatusCode::ENCRYPTION_FAILED_TO_ESTABLISH:
				handleConnectionFlowError(mState, statusCode, L"Encryption failed to establish");
				break;
			// cases till next break should set mDisconnectedCause below
			case StatusCode::EXCEPTION:
			case StatusCode::EXCEPTION_ON_CONNECT:
			case StatusCode::INTERNAL_RECEIVE_EXCEPTION:
			case StatusCode::TIMEOUT_DISCONNECT:
			case StatusCode::DISCONNECT_BY_SERVER:
			case StatusCode::DISCONNECT_BY_SERVER_USER_LIMIT:
			case StatusCode::DISCONNECT_BY_SERVER_LOGIC:
				mListener.connectionErrorReturn(statusCode);
				if(mpPeer->getPeerState() != PeerState::DISCONNECTED && mpPeer->getPeerState() != PeerState::DISCONNECTING)
					disconnect();
				else
					mState = PeerStates::PeerCreated;
				break;
			case StatusCode::SEND_ERROR:
				mListener.clientErrorReturn(statusCode);
				break;
			case QUEUE_OUTGOING_RELIABLE_WARNING:
			case QUEUE_OUTGOING_UNRELIABLE_WARNING:
			case QUEUE_OUTGOING_ACKS_WARNING:
			case QUEUE_INCOMING_RELIABLE_WARNING:
			case QUEUE_INCOMING_UNRELIABLE_WARNING:
			case QUEUE_SENT_WARNING:
				mListener.warningReturn(statusCode);
				break;
			case ErrorCode::OPERATION_INVALID:
			case ErrorCode::INTERNAL_SERVER_ERROR:
				mListener.serverErrorReturn(statusCode);
				break;
			default:
				EGLOG(DebugLevel::ERRORS, L"received unknown status-code from server");
				break;
			}

			// above cases starting from StatusCode::EXCEPTION till next break should set mDisconnectedCause here
			switch(statusCode)
			{
			case StatusCode::DISCONNECT_BY_SERVER_USER_LIMIT:
				mDisconnectedCause = DisconnectCause::DISCONNECT_BY_SERVER_USER_LIMIT;
				break;
			case StatusCode::EXCEPTION_ON_CONNECT:
				mDisconnectedCause = DisconnectCause::EXCEPTION_ON_CONNECT;
				break;
			case StatusCode::DISCONNECT_BY_SERVER:
				mDisconnectedCause = DisconnectCause::DISCONNECT_BY_SERVER;
				break;
			case StatusCode::DISCONNECT_BY_SERVER_LOGIC:
				mDisconnectedCause = DisconnectCause::DISCONNECT_BY_SERVER_LOGIC;
				break;
			case StatusCode::TIMEOUT_DISCONNECT:
				mDisconnectedCause = DisconnectCause::TIMEOUT_DISCONNECT;
				break;
			case StatusCode::EXCEPTION:
			case StatusCode::INTERNAL_RECEIVE_EXCEPTION:
				mDisconnectedCause = DisconnectCause::EXCEPTION;
				break;
			}
		}

		void Client::onEvent(const EventData& eventData)
		{
			EGLOG(eventData.getCode()>EventCode::MAX_CUSTOM_EVCODE&&eventData.getCode()!=EventCode::PUNCH_MSG?DebugLevel::INFO:DebugLevel::ALL, L"%ls", eventData.toString().cstr()); // don't print out the payload here, as that can get too expensive for big events
			switch(eventData.getCode())
			{
			case EventCode::ROOM_LIST:
				{
					for(unsigned int i=0; i<mRoomList.getSize(); ++i)
						destroyRoom(mRoomList[i]);
					mRoomList.removeAllElements();
					mRoomNameList.removeAllElements();

					Hashtable roomList = ValueObject<Hashtable>(eventData.getParameterForCode(ParameterCode::ROOM_LIST)).getDataCopy();
					const JVector<Object>& keys = roomList.getKeys();
					JString roomNameValue;
					for(unsigned int i=0; i<keys.getSize(); ++i)
					{
						roomNameValue = KeyObject<JString>(keys[i]).getDataCopy();
						mRoomList.addElement(createRoom(roomNameValue, ValueObject<Hashtable>(roomList.getValue(keys[i])).getDataCopy()));
						mRoomNameList.addElement(roomNameValue);
					}
					mListener.onRoomListUpdate();
				}
				break;
			case EventCode::ROOM_LIST_UPDATE:
				{
					Hashtable roomListUpdate(ValueObject<Hashtable>(eventData.getParameterForCode(ParameterCode::ROOM_LIST)).getDataCopy());
					const JVector<Object>& keys = roomListUpdate.getKeys();
					for(unsigned int i=0; i<keys.getSize(); i++)
					{
						Hashtable val(ValueObject<Hashtable>(roomListUpdate.getValue(keys[i])).getDataCopy());
						bool removed = ValueObject<bool>(val.getValue(Properties::Room::REMOVED)).getDataCopy();
						int index = mRoomNameList.getIndexOf(KeyObject<JString>(keys[i]).getDataCopy());

						if(!removed)
						{
							if(index == -1) //add room
							{
								JString roomNameValue = KeyObject<JString>(keys[i]).getDataCopy();
								mRoomList.addElement(createRoom(roomNameValue, val));
								mRoomNameList.addElement(roomNameValue);
							}
							else // update room (only entries, which have been changed, have been sent)
								RoomPropertiesCacher::cache(*mRoomList[index], val);
						}
						else if(index > -1) // remove room
						{
							destroyRoom(mRoomList[index]);
							mRoomList.removeElementAt(index);
							mRoomNameList.removeElementAt(index);
						}
					}
					mListener.onRoomListUpdate();
				}
				break;
			case EventCode::APP_STATS:
				{
					mPeerCount = ValueObject<int>(eventData.getParameterForCode(ParameterCode::PEER_COUNT)).getDataCopy();
					mRoomCount = ValueObject<int>(eventData.getParameterForCode(ParameterCode::ROOM_COUNT)).getDataCopy();
					mMasterPeerCount = ValueObject<int>(eventData.getParameterForCode(ParameterCode::MASTER_PEER_COUNT)).getDataCopy();
					mListener.onAppStatsUpdate();
				}
				break;
			case EventCode::LOBBY_STATS:
				{
					ValueObject<JString*> namesList = eventData.getParameterForCode(ParameterCode::LOBBY_NAME);
					ValueObject<nByte*> typesList = eventData.getParameterForCode(ParameterCode::LOBBY_TYPE);
					ValueObject<int*> peersList = eventData.getParameterForCode(ParameterCode::PEER_COUNT);
					ValueObject<int*> roomsList = eventData.getParameterForCode(ParameterCode::ROOM_COUNT);

					JString* names = *namesList.getDataAddress();
					nByte* types = *typesList.getDataAddress();
					int* peers = *peersList.getDataAddress();
					int* rooms = *roomsList.getDataAddress();

					JVector<LobbyStatsResponse> res;
					for(int i=0; i<*namesList.getSizes(); ++i) 
						res.addElement(LobbyStatsResponse(names[i], types[i], peers[i], rooms[i]));
					mListener.onLobbyStatsUpdate(res);
				}
				break;
			case EventCode::JOIN:
				{
					int nr = ValueObject<int>(eventData.getParameterForCode(ParameterCode::PLAYERNR)).getDataCopy();
					if(nr != getLocalPlayer().getNumber()) // the local player already got added in onOperationResponse cases OperationCode::CREATE_ROOM / OperationCode::JOIN_ROOM
						PlayerMovementInformant::onEnterRemote(getCurrentlyJoinedRoom(), nr, ValueObject<Hashtable>(eventData.getParameterForCode(ParameterCode::PLAYER_PROPERTIES)).getDataCopy());
					Object playersObj = eventData.getParameterForCode(ParameterCode::PLAYER_LIST);
					int* players = ValueObject<int*>(playersObj).getDataCopy();
					JVector<int> playerArr;
					int* playersPtr = players;
					for(int i=0; i<playersObj.getSizes()[0]; ++i, playersPtr++)
						playerArr.addElement(*playersPtr);

					if(nr != getLocalPlayer().getNumber())
					{
						switch(mpCurrentlyJoinedRoom->getDirectMode())
						{
						case DirectMode::NONE:
							break;
#ifndef _EG_EMSCRIPTEN_PLATFORM
						case DirectMode::ALL_TO_ALL:
							startPunch(nr);
							break;
						case DirectMode::MASTER_TO_ALL:
							if(getLocalPlayer().getIsMasterClient())
								startPunch(nr);
							break;
#endif
						default:
							break;
						}
					}

					mListener.joinRoomEventAction(nr, playerArr, *getCurrentlyJoinedRoom().getPlayerForNumber(nr));
					deallocateArray(players);
				}
				break;
			case EventCode::LEAVE:
				{
					ValueObject<int> nr = eventData.getParameterForCode(ParameterCode::PLAYERNR);
					ValueObject<bool> isInactive = eventData.getParameterForCode(ParameterCode::IS_INACTIVE);
					if(isInactive.getDataCopy())
					{
						if(!PlayerPropertiesUpdateInformant::setIsInactive(*mpCurrentlyJoinedRoom, nr.getDataCopy(), true))
							EGLOG(DebugLevel::WARNINGS, L"EventCode::LEAVE - player %d who is leaving the room, has not been found in list of players, who are currently in the room", nr.getDataCopy());
					}
					else if(!PlayerMovementInformant::onLeaveRemote(getCurrentlyJoinedRoom(), nr.getDataCopy()))
							EGLOG(DebugLevel::WARNINGS, L"EventCode::LEAVE - player %d who is leaving the room, has not been found in list of players, who are currently in the room", nr.getDataCopy());
					if(eventData.getParameters().contains(ParameterCode::MASTER_CLIENT_ID))
					{
						int newMaster = ValueObject<int>(eventData.getParameterForCode(ParameterCode::MASTER_CLIENT_ID)).getDataCopy();
						if(newMaster)
							PlayerMovementInformant::onMasterClientChanged(getCurrentlyJoinedRoom(), newMaster);
					}
					mListener.leaveRoomEventAction(nr.getDataCopy(), isInactive.getDataCopy());
				}
				break;
			case EventCode::PROPERTIES_CHANGED:
				{
					ValueObject<int> target = eventData.getParameterForCode(ParameterCode::TARGET_PLAYERNR);
					Hashtable playerProperties;
					Hashtable roomProperties;
					if(target.getDataCopy())
						playerProperties = ValueObject<Hashtable>(eventData.getParameterForCode(ParameterCode::PROPERTIES)).getDataCopy();
					else
						roomProperties = ValueObject<Hashtable>(eventData.getParameterForCode(ParameterCode::PROPERTIES)).getDataCopy();
					readoutProperties(roomProperties, playerProperties, false, target.getDataCopy());
					if(target.getDataCopy())
						mListener.onPlayerPropertiesChange(target.getDataCopy(), playerProperties);
					else
						mListener.onRoomPropertiesChange(roomProperties);
				}
				break;
			case EventCode::CACHE_SLICE_CHANGED:
				mListener.onCacheSliceChanged(ValueObject<int>(eventData.getParameterForCode(ParameterCode::CACHE_SLICE_INDEX)).getDataCopy());
				break;
			case EventCode::AUTH:
				AuthenticationValuesSecretSetter::setSecret(mAuthenticationValues, ValueObject<JString>(eventData.getParameterForCode(ParameterCode::SECRET)).getDataCopy());
				EGLOG(DebugLevel::INFO, L"Server returned secret");
				mListener.onSecretReceival(mAuthenticationValues.getSecret());
				break;
			case EventCode::PUNCH_MSG:
				{
					int nr = ValueObject<int>(eventData.getParameterForCode(ParameterCode::PLAYERNR)).getDataCopy();
					EGLOG(DebugLevel::ALL, L"Punch event from player %d", nr);
					ValueObject<nByte*> obj = eventData.getParameterForCode(ParameterCode::DATA);
					mpPuncherClient->processRelayPackage(JVector<nByte>(*obj.getDataAddress(), *obj.getSizes()), nr);
				}
				break;
			default: // custom events are forwarded to the app
				{
					ValueObject<int> nr = eventData.getParameterForCode(ParameterCode::PLAYERNR);
					// all custom event data is stored at ParameterCode::DATA
					mListener.customEventAction(nr.getDataCopy(), eventData.getCode(), eventData.getParameterForCode(ParameterCode::DATA));
				}
				break;
			}
		}

		void Client::onPingResponse(const JString& address, unsigned int result)
		{
			bool receivedAllRequests = true;
			for(unsigned int i=0; i<mAvailableRegionServers.getSize(); ++i)
			{
				if(address == mAvailableRegionServers[i])
					mPingResponses[i].addElement(result);
				if(mPingResponses[i].getSize() < mPingsPerRegion)
					receivedAllRequests = false;
			}
			if(!receivedAllRequests)
				return;
			unsigned int bestPing = UINT_MAX;
			unsigned int indexOfRegionWithBestPing = 0;
			for(unsigned int i=0; i<mPingResponses.getSize(); ++i)
			{
				unsigned int ping = 0;
				for(unsigned int j=0; j<mPingsPerRegion; ++j)
					ping += mPingResponses[i][j];
				ping /= mPingsPerRegion;
				if(ping < bestPing)
				{
					bestPing = ping;
					indexOfRegionWithBestPing = i;
				}
			}
			mPingResponses.removeAllElements();
			mSelectedRegion = mRegionWithBestPing = mAvailableRegions[indexOfRegionWithBestPing];
			authenticate();
		}

		void Client::debugReturn(int debugLevel, const JString& string)
		{
			mListener.debugReturn(debugLevel, string);
		}

		bool Client::getIsOnGameServer(void) const
		{
			return mState >= PeerStates::ConnectingToGameserver && mState < PeerStates::ConnectingToMasterserver;
		}

		void Client::readoutProperties(Hashtable& roomProperties, Hashtable& playerProperties, bool multiplePlayers, int targetPlayerNr)
		{
			if(roomProperties.getSize())
			{
				RoomPropertiesCacher::cache(*mpCurrentlyJoinedRoom, roomProperties);
				roomProperties = Utils::stripKeysWithNullValues(Utils::stripToCustomProperties(roomProperties));
			}

			if(playerProperties.getSize())
			{
				for(unsigned int i=0; i<(multiplePlayers?playerProperties.getSize():1); ++i)
					PlayerPropertiesUpdateInformant::onUpdate(*mpCurrentlyJoinedRoom, multiplePlayers?ValueObject<int>(playerProperties.getKeys()[i]).getDataCopy():targetPlayerNr, multiplePlayers?ValueObject<Hashtable>(playerProperties[i]).getDataCopy():playerProperties);

				if(multiplePlayers)
					for(unsigned int i=0; i<playerProperties.getSize(); ++i)
						playerProperties[i] = ValueObject<Hashtable>(Utils::stripKeysWithNullValues(Utils::stripToCustomProperties(ValueObject<Hashtable>(playerProperties[i]).getDataCopy())));
				else
					playerProperties = Utils::stripKeysWithNullValues(Utils::stripToCustomProperties(playerProperties));
			}
		}

		void Client::handleConnectionFlowError(int oldState, int errorCode, const JString& errorString)
		{
			if(oldState == PeerStates::ConnectedToGameserver || oldState == PeerStates::AuthenticatedOnGameServer || oldState == PeerStates::Joining)
			{
				mCachedErrorCodeFromGameServer = errorCode;
				mCachedErrorStringFromGameServer = errorString;
				mState = PeerStates::DisconnectingFromGameserver;
				mpPeer->disconnect();
				// response to app has to wait until back on master
			}
			else
			{
				mState = PeerStates::Disconnecting;
				mpPeer->disconnect();
				mListener.connectReturn(errorCode, errorString, L"", L"");
			}
		}
		
		/**
		   Makes Photon call your custom web-service by path/name with the given parameters (converted into JSON).
		   @details
		   A WebRPC calls a custom, http-based function on a server that you provide. The uriPath is relative to a "base path"
		   which is configured on the server side. The sent parameters get converted to Json. Vice versa, the response
		   of the web-service will be converted back, when it gets sent back to the Client, where it arrives in Listener::webRpcReturn().


		   To use this feature, you have to setup your server:

		   For a Photon Cloud application visit the Dashboard and setup "WebHooks". The BaseUrl is used for WebRPCs as well.

		   @returns true, if the request could successfully be queued for sending to the server, false otherwise.
		   @sa
		   Listener::webRpcReturn(), @link Datatypes Table of Datatypes\endlink, <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks">Webhooks</a>
		   @param uriPath the URL path to call, relative to the baseUrl configured on Photon's server-side
		 */
		bool Client::opWebRpc(const JString& uriPath)
		{
			return mpPeer->opWebRpc(uriPath);
		}

		/** @fn template<typename Ftype> bool Client::opWebRpc(const Common::JString& uriPath, const Ftype& parameters, bool sendAuthCookie=false)
		   @overload
		   @param uriPath the URL path to call, relative to the baseUrl configured on Photon's server-side
		   @param parameters the parameters to send to the web-service method - has to be provided in the form of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param sendAuthCookie defines if the authentication cookie gets sent to a WebHook (if setup)
		 */

		/** @fn template<typename Ftype> bool Client::opWebRpc(const Common::JString& uriPath, const Ftype pParameterArray, typename Common::Helpers::ArrayLengthType<Ftype>::type arrSize, bool sendAuthCookie=false)
		   @overload
		   @param uriPath the URL path to call, relative to the baseUrl configured on Photon's server-side
		   @param pParameterArray the parameter array to send to the web-service method - has to be provided in the form of a 1D array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param arrSize the number of elements in pParameterArray
		   @param sendAuthCookie defines if the authentication cookie gets sent to a WebHook (if setup)
		 */

		/** @fn template<typename Ftype> bool Client::opWebRpc(const Common::JString& uriPath, const Ftype pParameterArray, const short* pArrSizes, bool sendAuthCookie=false)
		   @overload
		   @param uriPath the URL path to call, relative to the baseUrl configured on Photon's server-side
		   @param pParameterArray the parameter array to send to the web-service method - has to be provided in the form of an array of one of the supported data types, specified at @link Datatypes Table of Datatypes\endlink
		   @param pArrSizes an array holding the number of elements for each dimension of pParameterArray
		   @param sendAuthCookie defines if the authentication cookie gets sent to a WebHook (if setup)
		 */

		bool Client::opSetPropertiesOfPlayer(int playerNr, const Hashtable& properties, const Hashtable& expectedProperties, WebFlags webFlags)
		{
			if(!getIsInGameRoom())
				return false;
			return mpPeer->opSetPropertiesOfPlayer(playerNr, properties, expectedProperties, webFlags);
		}

		bool Client::opSetPropertiesOfRoom(const Hashtable& properties, const Hashtable& expectedProperties, WebFlags webFlags)
		{
			if(!getIsInGameRoom())
				return false;
			return mpPeer->opSetPropertiesOfRoom(properties, expectedProperties, webFlags);
		}

		void Client::onConnectToMasterFinished(bool comingFromGameserver)
		{
			if(comingFromGameserver)
			{
				if(mCachedErrorCodeFromGameServer)
				{
					switch(mLastJoinType)
					{
						case JoinType::CREATE_ROOM:
							mListener.createRoomReturn(0, Hashtable(), Hashtable(), mCachedErrorCodeFromGameServer, mCachedErrorStringFromGameServer);
							break;
						case JoinType::JOIN_ROOM:
							mListener.joinRoomReturn(0, Hashtable(), Hashtable(), mCachedErrorCodeFromGameServer, mCachedErrorStringFromGameServer);
							break;
						case JoinType::JOIN_RANDOM_ROOM:
							mListener.joinRandomRoomReturn(0, Hashtable(), Hashtable(), mCachedErrorCodeFromGameServer, mCachedErrorStringFromGameServer);
							break;
						default:
							EGLOG(DebugLevel::ERRORS, L"unexpected cached join type value");
							break;
					}
					mCachedErrorCodeFromGameServer = LoadBalancing::ErrorCode::OK;
					mCachedErrorStringFromGameServer = L"";
				}
				else
					mListener.leaveRoomReturn(0, L"");
			}
			else
				mListener.connectReturn(0, L"", mSelectedRegion, mCluster);
		}

		void Client::onArrivalAndAuthentication(void)
		{
			if(mState == PeerStates::Connected || mState == PeerStates::ConnectedComingFromGameserver)
			{
				mState = mState==PeerStates::Connected?PeerStates::Authenticated:PeerStates::AuthenticatedComingFromGameserver;
				if(mAutoJoinLobby)
				{
					opJoinLobby();
					mLastLobbyJoinType = JoinType::AUTO_JOIN_LOBBY;
				}
				else
					onConnectToMasterFinished(mState==PeerStates::AuthenticatedComingFromGameserver);
			}
			else if(mState == PeerStates::ConnectedToGameserver)
			{
				mState = PeerStates::Joining;
				mCachedErrorCodeFromGameServer = LoadBalancing::ErrorCode::OK;
				mCachedErrorStringFromGameServer = L"";
				if(mLastJoinType == JoinType::CREATE_ROOM)
					opCreateRoom(mRoomName, RoomOptions(mpCurrentlyJoinedRoom->getIsVisible(), mpCurrentlyJoinedRoom->getIsOpen(), mpCurrentlyJoinedRoom->getMaxPlayers(), mpCurrentlyJoinedRoom->getCustomProperties(), mpCurrentlyJoinedRoom->getPropsListedInLobby(), Common::JString(), LobbyType::DEFAULT, mpCurrentlyJoinedRoom->getPlayerTtl(), mpCurrentlyJoinedRoom->getEmptyRoomTtl(), mpCurrentlyJoinedRoom->getSuppressRoomEvents(), mpCurrentlyJoinedRoom->getPlugins(), mpCurrentlyJoinedRoom->getPublishUserID(), mpCurrentlyJoinedRoom->getDirectMode()), mpCurrentlyJoinedRoom->getExpectedUsers()); // lobbyName and lobbyType are not used on the game server
				else if(mLastJoinType == JoinType::JOIN_OR_CREATE_ROOM)
					opJoinOrCreateRoom(mRoomName, RoomOptions(mpCurrentlyJoinedRoom->getIsVisible(), mpCurrentlyJoinedRoom->getIsOpen(), mpCurrentlyJoinedRoom->getMaxPlayers(), mpCurrentlyJoinedRoom->getCustomProperties(), mpCurrentlyJoinedRoom->getPropsListedInLobby(), Common::JString(), LobbyType::DEFAULT, mpCurrentlyJoinedRoom->getPlayerTtl(), mpCurrentlyJoinedRoom->getEmptyRoomTtl(), mpCurrentlyJoinedRoom->getSuppressRoomEvents(), mpCurrentlyJoinedRoom->getPlugins(), mpCurrentlyJoinedRoom->getPublishUserID(), mpCurrentlyJoinedRoom->getDirectMode()), mLastCacheSliceIndex, mpCurrentlyJoinedRoom->getExpectedUsers());
				else if(mLastJoinType == JoinType::JOIN_ROOM)
					opJoinRoom(mRoomName, mLastJoinWasRejoin, mLastCacheSliceIndex, mpCurrentlyJoinedRoom->getExpectedUsers());
				else if(mLastJoinType == JoinType::JOIN_RANDOM_ROOM)
					opJoinRoom(mRoomName, false, 0, mpCurrentlyJoinedRoom->getExpectedUsers());
			}
		}

		MutablePlayer* Client::createMutablePlayer(int number, const Hashtable& properties)
		{
			mpMutablePlayerFactory = getMutablePlayerFactory();
			return mpMutablePlayerFactory->create(number, properties, mpCurrentlyJoinedRoom, this);
		}

		void Client::destroyMutablePlayer(const MutablePlayer* pPlayer) const
		{
			if(pPlayer)
				mpMutablePlayerFactory->destroy(pPlayer);
		}

		Room* Client::createRoom(const JString& name, const Hashtable& properties)
		{
			return RoomFactory::create(name, properties);
		}

		void Client::destroyRoom(const Room* pRoom) const
		{
			RoomFactory::destroy(pRoom);
		}

		MutableRoom* Client::createMutableRoom(const JString& name, const Hashtable& properties, const JVector<JString>& propsListedInLobby, int playerTtl, int emptyRoomTtl, bool suppressRoomEvents, const JVector<JString>* pPlugins, bool publishUserID, const JVector<JString>& expectedUsers)
		{
			mpMutableRoomFactory = getMutableRoomFactory();
			return mpMutableRoomFactory->create(name, properties, this, propsListedInLobby, playerTtl, emptyRoomTtl, suppressRoomEvents, pPlugins, publishUserID, expectedUsers);
		}

		void Client::destroyMutableRoom(const MutableRoom* pRoom) const
		{
			if(pRoom)
				mpMutableRoomFactory->destroy(pRoom);
		}

		MutablePlayerFactory* Client::getMutablePlayerFactory(void) const
		{
			static MutablePlayerFactory fac;
			return &fac;
		}

		MutableRoomFactory* Client::getMutableRoomFactory(void) const
		{
			static MutableRoomFactory fac;
			return &fac;
		}

#if defined EG_PLATFORM_SUPPORTS_CPP11 && defined EG_PLATFORM_SUPPORTS_MULTITHREADING
		void Client::pingBestRegion(unsigned int pingsPerRegion)
		{
			EGLOG(DebugLevel::INFO, L"");
			for(unsigned int i=0; i<mAvailableRegionServers.getSize(); ++i)
			{
				mpPeer->pingServer(mAvailableRegionServers[i], pingsPerRegion);
				mPingResponses.addElement(JVector<unsigned int>(pingsPerRegion));
			}
			mPingsPerRegion = pingsPerRegion;
		}
#endif

		bool Client::callPeerConnect(const JString& address)
		{
			if(mUseAuthOnce)
				return mpPeer->connect(address, mAppIDPeerInit, mAuthenticationValues.getSecret());
			else
				return mpPeer->connect(address, mAppIDPeerInit);
		}

		bool Client::authenticate(void)
		{
			if(mUseAuthOnce)
				return mpPeer->opAuthenticateOnce(mAppID, mAppVersion, M_CONNECTION_PROTOCOL, mUseUDPEncryption?EncryptionMode::UDP_TRAFFIC_ENCRYPTION:EncryptionMode::USER_DATA_ENCRYPTION_NO_DIFFIE_HELLMAN, mAuthenticationValues, mAutoLobbyStats, mSelectedRegion);
			else
				return mpPeer->opAuthenticate(mAppID, mAppVersion, !mAuthenticationValues.getSecret().length(), mAuthenticationValues, mAutoLobbyStats, mSelectedRegion);  // the secret token already is encrypted itself, so there is no need to send the operation encrypted in the case of authentication by token
		}

		JString Client::addPortToAddress(const JString& address, nByte serverType)
		{
			if(address.indexOf(L'.') > -1 || address.indexOf(L"localhost") > -1) // IPv4 or URL or localhost
				return address+(address.indexOf(L':')<0?JString(L":")+getDefaultPort(serverType, mUseAlternativePorts):JString());
			else // IPv6
			{
				int endIndex, idx, idx2 = address.substring(idx=address.indexOf(']')).indexOf(':');
				endIndex = idx2==-1?idx2:idx;
				return endIndex<0?JString(L"[")+address+JString(L"]:")+getDefaultPort(serverType, mUseAlternativePorts):address;
			}
		}

		unsigned short Client::getDefaultPort(nByte serverType, bool useAlternativePorts)
		{
			const NetworkPort::Protocol* pProtocol;
#ifdef _EG_USE_WSS_ON_NS_BUT_SPECIFIED_PROTOCOL_ELSEWHERE
			pProtocol = allocate<NetworkPort::WSS>();
#else
			switch(M_CONNECTION_PROTOCOL)
			{
#ifndef _EG_EMSCRIPTEN_PLATFORM
			case ConnectionProtocol::UDP:
				if(useAlternativePorts)
					pProtocol = allocate<NetworkPort::UDPAlternative>();
				else
					pProtocol = allocate<NetworkPort::UDP>();
				break;
#endif
#if !defined _EG_EMSCRIPTEN_PLATFORM && !defined _EG_XB1_PLATFORM
			case ConnectionProtocol::TCP:
				pProtocol = allocate<NetworkPort::TCP>();
				break;
#endif
#if defined _EG_EMSCRIPTEN_PLATFORM || defined _EG_WEBSOCKET_AVAILABLE
			case ConnectionProtocol::WS:
				pProtocol = allocate<NetworkPort::WS>();
				break;
			case ConnectionProtocol::WSS:
				pProtocol = allocate<NetworkPort::WSS>();
				break;
#endif
			default:
#ifdef _EG_EMSCRIPTEN_PLATFORM
				pProtocol = allocate<NetworkPort::WS>();
				break;
#else
				if(useAlternativePorts)
					pProtocol = allocate<NetworkPort::UDPAlternative>();
				else
					pProtocol = allocate<NetworkPort::UDP>();
				break;
#endif
			}
#endif
			unsigned short port = serverType==ServerType::NAME_SERVER?pProtocol->NAME:pProtocol->MASTER;
			deallocate(pProtocol);
			return port;
		}

		bool Client::initPuncher(void)
		{
			return mpPuncherClient->initPuncher();
		}

		bool Client::startPunch(int playerNr)
		{
			return mpPuncherClient->startPunch(playerNr);
		}

		void Client::onMasterClientChanged(int id, int oldID) 
		{
#ifndef _EG_EMSCRIPTEN_PLATFORM
			if(mpCurrentlyJoinedRoom->getDirectMode() == DirectMode::MASTER_TO_ALL) // punch from all to master
				if(id != getLocalPlayer().getNumber())
					startPunch(id);
#endif
			mListener.onMasterClientChanged(id, oldID);
		}
	}
}
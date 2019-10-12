/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Peer.h"
#include "Photon-cpp/inc/Enums/ReceiverGroup.h"
#include "Photon-cpp/inc/Enums/EventCache.h"
#include "LoadBalancing-cpp/inc/Enums/CustomAuthenticationType.h"
#include "LoadBalancing-cpp/inc/Enums/DirectMode.h"
#include "LoadBalancing-cpp/inc/Internal/AuthenticationValuesSecretSetter.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/JoinMode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/JoinType.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/OperationCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/ParameterCode.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Player.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Room.h"

/** @file LoadBalancing-cpp/inc/Peer.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Common::MemoryManagement;
		using namespace Photon;
		using namespace Lite;
		using namespace ParameterCode;
		using namespace Internal;



		Peer::Peer(Photon::PhotonListener& listener, nByte connectionProtocol)
			: PhotonPeer(listener, connectionProtocol)
		{
		}

		Peer::~Peer(void)
		{
		}

#if defined EG_DOC
		/**
		   Initiates the disconnection from the %Photon server. The servers response will arrive in PhotonListener::onStatusChanged().
		   @details
		   This function generates a disconnection request that will be sent to the %Photon server. If the disconnection is completed successfully, then the PhotonListener::onStatusChanged() callback will
		   be called, with a statusCode of StatusCode::DISCONNECT.

		   @remarks
		   If a game room is joined, when this function gets called, then the local player leaves that room as if opLeaveRoom() has been called with parameter 'willComeBack' set to 'true'.
		   Please see there for further information about leaving rooms.
		   However no call to Listener::leaveRoomReturn() will happen when leaving a game room is triggered through a call to disconnect().
		   @sa
		   connect() */
		void Peer::disconnect(void)
		{
		}
#endif

		bool Peer::opJoinLobby(const JString& lobbyName, nByte lobbyType)
		{
			OperationRequestParameters op;
			if(lobbyName.length()) 
			{
				op.put(ParameterCode::LOBBY_NAME, ValueObject<JString>(lobbyName));
				if(lobbyType != LobbyType::DEFAULT)
					op.put(ParameterCode::LOBBY_TYPE, ValueObject<nByte>(lobbyType));
			};
			return opCustom(OperationRequest(OperationCode::JOIN_LOBBY, op), true);
		}

		bool Peer::opLeaveLobby(void)
		{
			return opCustom(OperationRequest(OperationCode::LEAVE_LOBBY), true);
		}

		bool Peer::opCreateRoom(const JString& gameID, const RoomOptions& options, const Common::Hashtable& customLocalPlayerProperties, const JVector<JString>& expectedUsers)
		{
			return opCustom(OperationRequest(OperationCode::CREATE_ROOM, opCreateRoomImplementation(gameID, options, customLocalPlayerProperties, expectedUsers)), true);
		}

		bool Peer::opJoinRoom(const JString& gameID, const RoomOptions& options, const Hashtable& customLocalPlayerProperties, bool createIfNotExists, bool rejoin, int cacheSliceIndex, const JVector<JString>& expectedUsers)
		{
			return gameID.length() ? opCustom(OperationRequest(OperationCode::JOIN_ROOM, opJoinRoomImplementation(gameID, options, customLocalPlayerProperties, createIfNotExists, rejoin, cacheSliceIndex, expectedUsers)), true) : false;
		}

		bool Peer::opJoinRandomRoom(const Hashtable& customRoomProperties, nByte maxPlayers, nByte matchingType, const Common::JString& lobbyName, nByte lobbyType, const Common::JString& sqlLobbyFilter, const JVector<JString>& expectedUsers)
		{
			Hashtable roomProps(Utils::stripToCustomProperties(customRoomProperties));
			if(maxPlayers)
				roomProps.put(Properties::Room::MAX_PLAYERS, maxPlayers);
				
			OperationRequestParameters op;

			if(roomProps.getSize())
				op.put(ParameterCode::ROOM_PROPERTIES, ValueObject<Hashtable>(roomProps));
			if(matchingType != MatchmakingMode::FILL_ROOM)
				op.put(ParameterCode::MATCHMAKING_TYPE, ValueObject<nByte>(matchingType));
			if(lobbyName.length()) 
			{
				op.put(ParameterCode::LOBBY_NAME, ValueObject<JString>(lobbyName));
				if(lobbyType != LobbyType::DEFAULT)
					op.put(ParameterCode::LOBBY_TYPE, ValueObject<nByte>(lobbyType));
			}
			if(sqlLobbyFilter.length()) 
				op.put(ParameterCode::DATA, ValueObject<JString>(sqlLobbyFilter));
			if(expectedUsers.getSize())
				op.put(ParameterCode::ADD, ValueObject<const JString*>(expectedUsers.getCArray(), expectedUsers.getSize()));

			return opCustom(OperationRequest(OperationCode::JOIN_RANDOM_ROOM, op), true);
		}

		bool Peer::opLeaveRoom(bool willComeBack, bool sendAuthCookie)
		{
			OperationRequestParameters op;
			if(willComeBack)
				op.put(ParameterCode::IS_COMING_BACK, ValueObject<bool>(willComeBack));
			if(sendAuthCookie)
				op.put(ParameterCode::EVENT_FORWARD, ValueObject<nByte>(WebFlags().setSendAuthCookie(true).getFlags()));
			return opCustom(OperationRequest(OperationCode::LEAVE, op), true);
		}

		bool Peer::opRaiseEvent(bool reliable, const Common::Object& parameters, nByte eventCode, const RaiseEventOptions& options)
		{
			OperationRequestParameters op;
			if(options.getEventCaching() == EventCache::SLICE_INC_INDEX || options.getEventCaching() == EventCache::SLICE_SET_INDEX || options.getEventCaching() == EventCache::SLICE_PURGE_INDEX || options.getEventCaching() == EventCache::SLICE_PURGE_UP_TO_INDEX)
			{
				op.put(ParameterCode::CACHE, ValueObject<nByte>(options.getEventCaching()));
				if(options.getEventCaching() != EventCache::SLICE_INC_INDEX)
					op.put(ParameterCode::CACHE_SLICE_INDEX, ValueObject<int>(options.getCacheSliceIndex()));
				return opCustom(OperationRequest(OperationCode::RAISE_EVENT, op), true, options.getChannelID());
			}
			op.put(ParameterCode::DATA, parameters);
			op.put(ParameterCode::CODE, ValueObject<nByte>(eventCode));
			if(options.getEventCaching() != EventCache::DO_NOT_CACHE)
				op.put(ParameterCode::CACHE, ValueObject<nByte>(options.getEventCaching()));
			if(options.getTargetPlayers())
				op.put(ParameterCode::PLAYER_LIST, ValueObject<const int*>(options.getTargetPlayers(), options.getNumTargetPlayers()));
			else if(options.getInterestGroup())
				op.put(ParameterCode::GROUP, ValueObject<nByte>(options.getInterestGroup()));
			else if(options.getReceiverGroup() != ReceiverGroup::OTHERS)
				op.put(ParameterCode::RECEIVER_GROUP, ValueObject<nByte>(options.getReceiverGroup()));
			if(options.getWebFlags().getHttpForward())
				op.put(ParameterCode::EVENT_FORWARD, ValueObject<nByte>(options.getWebFlags().getFlags()));
			return opCustom(OperationRequest(OperationCode::RAISE_EVENT, op), reliable, options.getChannelID(), false);
		}

		bool Peer::opAuthenticate(const JString& appID, const JString& appVersion, bool encrypted, const AuthenticationValues& authenticationValues, bool lobbyStats, const JString& regionCode)
		{
			OperationRequestParameters op;
			if(lobbyStats)
				op.put(ParameterCode::LOBBY_STATS, ValueObject<bool>(true));
			if(authenticationValues.getSecret().length())
			{
				op.put(ParameterCode::SECRET, ValueObject<JString>(authenticationValues.getSecret()));
				EGLOG(DebugLevel::INFO, OperationRequest(OperationCode::AUTHENTICATE, op).toString(true));
				return opCustom(OperationRequest(OperationCode::AUTHENTICATE, op), true, 0, false); // the secret already is encrypted, so we don't need to encrypt it again
			}
			op.put(ParameterCode::APPLICATION_ID, ValueObject<JString>(appID));
			op.put(ParameterCode::APP_VERSION, ValueObject<JString>(appVersion));
			if(regionCode.length())
				op.put(ParameterCode::REGION, ValueObject<JString>(regionCode));
			if(authenticationValues.getUserID().length())
				op.put(ParameterCode::USER_ID, ValueObject<JString>(authenticationValues.getUserID()));
			if(authenticationValues.getType() != CustomAuthenticationType::NONE)
			{
				op.put(ParameterCode::CLIENT_AUTHENTICATION_TYPE, ValueObject<nByte>(authenticationValues.getType()));
				if(authenticationValues.getParameters().length())
					op.put(ParameterCode::CLIENT_AUTHENTICATION_PARAMETERS, ValueObject<JString>(authenticationValues.getParameters()));
				if(authenticationValues.getData().getSize())
					op.put(ParameterCode::CLIENT_AUTHENTICATION_DATA, ValueObject<const nByte*>(authenticationValues.getData().getCArray(), static_cast<int>(authenticationValues.getData().getSize())));
			}
			EGLOG(DebugLevel::INFO, OperationRequest(OperationCode::AUTHENTICATE, op).toString(true));
			return opCustom(OperationRequest(OperationCode::AUTHENTICATE, op), true, 0, encrypted);
		}

		bool Peer::opAuthenticateOnce(const JString& appID, const JString& appVersion, nByte connectionProtocol, nByte encryptionMode, const AuthenticationValues& authenticationValues, bool lobbyStats, const JString& regionCode)
		{
			OperationRequestParameters op;
			op.put(ParameterCode::APPLICATION_ID, ValueObject<JString>(appID));
			op.put(ParameterCode::APP_VERSION, ValueObject<JString>(appVersion));
			if(regionCode.length())
				op.put(ParameterCode::REGION, ValueObject<JString>(regionCode));
			if(authenticationValues.getUserID().length())
				op.put(ParameterCode::USER_ID, ValueObject<JString>(authenticationValues.getUserID()));
			if(authenticationValues.getType() != CustomAuthenticationType::NONE)
			{
				op.put(ParameterCode::CLIENT_AUTHENTICATION_TYPE, ValueObject<nByte>(authenticationValues.getType()));
				if(authenticationValues.getParameters().length())
					op.put(ParameterCode::CLIENT_AUTHENTICATION_PARAMETERS, ValueObject<JString>(authenticationValues.getParameters()));
				if(authenticationValues.getData().getSize())
					op.put(ParameterCode::CLIENT_AUTHENTICATION_DATA, ValueObject<const nByte*>(authenticationValues.getData().getCArray(), static_cast<int>(authenticationValues.getData().getSize())));
			}
			Dictionary<Object, Object> customInitData;
			if(lobbyStats)
				customInitData.put(ValueObject<nByte>(ParameterCode::LOBBY_STATS), ValueObject<bool>(true));
			if(customInitData.getSize())
				op.put(ParameterCode::CUSTOM_INIT_DATA, ValueObject<Dictionary<Object, Object> >(customInitData));
			op.put(ParameterCode::ENCRYPTION_MODE, ValueObject<nByte>(encryptionMode));
			op.put(ParameterCode::EXPECTED_PROTOCOL, ValueObject<nByte>(connectionProtocol));
			EGLOG(DebugLevel::INFO, OperationRequest(OperationCode::AUTH_ONCE, op).toString(true));
			return opCustom(OperationRequest(OperationCode::AUTH_ONCE, op), true, 0, true);
		}

		bool Peer::opChangeGroups(const JVector<nByte>* pGroupsToRemove, const JVector<nByte>* pGroupsToAdd)
		{
			OperationRequestParameters op;
			if(pGroupsToRemove)
				op.put(ParameterCode::REMOVE, ValueObject<const nByte*>(pGroupsToRemove->getCArray(), pGroupsToRemove->getSize()));
			if(pGroupsToAdd)
				op.put(ParameterCode::ADD, ValueObject<const nByte*>(pGroupsToAdd->getCArray(), pGroupsToAdd->getSize()));

			return opCustom(OperationRequest(OperationCode::CHANGE_GROUPS, op), true);
		}

		bool Peer::opWebRpc(const JString& uriPath)
		{
			return opWebRpc(uriPath, Object());
		}

		bool Peer::opWebRpc(const JString& uriPath, const Object& parameters, bool sendAuthCookie)
		{
			OperationRequestParameters op;
			op.put(ParameterCode::URI_PATH, ValueObject<JString>(uriPath));
			op.put(ParameterCode::RPC_CALL_PARAMS, parameters);
			if(sendAuthCookie)
				op.put(ParameterCode::EVENT_FORWARD, ValueObject<nByte>(WebFlags().setSendAuthCookie(true).getFlags()));
			return opCustom(OperationRequest(OperationCode::RPC, op), true);
		}

		bool Peer::opFindFriends(const JString* friendsToFind, short numFriendsToFind)
		{
			if(!friendsToFind || !numFriendsToFind)
				return false;
			OperationRequestParameters op;
			op.put(ParameterCode::FIND_FRIENDS_REQUEST_LIST, ValueObject<const JString*>(friendsToFind, numFriendsToFind));

			return opCustom(OperationRequest(OperationCode::FIND_FRIENDS, op), true);
		}

		bool Peer::opLobbyStats(const Common::JVector<LoadBalancing::LobbyStatsRequest>& lobbiesToQuery)
		{
			if(!lobbiesToQuery.getSize())
				return opCustom(OperationRequest(OperationCode::LOBBY_STATS), true);

			unsigned int size = lobbiesToQuery.getSize();
			OperationRequestParameters op;
			JString* pNames = allocateArray<JString>(size);
			nByte* pTypes = allocateArray<nByte>(size);
			for(unsigned int i=0; i<size; ++i)
			{
				pNames[i] = lobbiesToQuery[i].getName();
				pTypes[i] = lobbiesToQuery[i].getType();
			}
			op.put(ParameterCode::LOBBY_NAME, ValueObject<const JString*>(pNames, size));
			op.put(ParameterCode::LOBBY_TYPE, ValueObject<const nByte*>(pTypes, size));
			bool res = opCustom(OperationRequest(OperationCode::LOBBY_STATS, op), true);
			deallocateArray(pNames);
			deallocateArray(pTypes);
			return res;
		}

		bool Peer::opGetRegions(bool encrypted, const JString& appID)
		{
			OperationRequestParameters op;
			op.put(ParameterCode::APPLICATION_ID, ValueObject<JString>(appID));

			return opCustom(OperationRequest(OperationCode::GET_REGIONS, op), true, 0, encrypted);
		}

		bool Peer::opSetPropertiesOfPlayer(int playerNr, const Hashtable& properties, const Hashtable& expectedProperties, WebFlags webFlags)
		{
			OperationRequestParameters op;
			op.put(ParameterCode::PROPERTIES, ValueObject<Hashtable>(properties));
			op.put(ParameterCode::PLAYERNR, ValueObject<int>(playerNr));
			op.put(ParameterCode::BROADCAST, ValueObject<bool>(true));
			if(expectedProperties.getSize())
				op.put(ParameterCode::EXPECTED_VALUES, ValueObject<Hashtable>(expectedProperties));
			if(webFlags.getHttpForward())
				op.put(ParameterCode::EVENT_FORWARD, ValueObject<nByte>(webFlags.getFlags()));

			return opCustom(OperationRequest(OperationCode::SET_PROPERTIES, op), true);
		}

		bool Peer::opSetPropertiesOfRoom(const Hashtable& properties, const Hashtable& expectedProperties, WebFlags webFlags)
		{
			OperationRequestParameters op;
			op.put(ParameterCode::PROPERTIES, ValueObject<Hashtable>(properties));
			op.put(ParameterCode::BROADCAST, ValueObject<bool>(true));
			if(expectedProperties.getSize())
				op.put(ParameterCode::EXPECTED_VALUES, ValueObject<Hashtable>(expectedProperties));
			if(webFlags.getHttpForward())
				op.put(ParameterCode::EVENT_FORWARD, ValueObject<nByte>(webFlags.getFlags()));

			return opCustom(OperationRequest(OperationCode::SET_PROPERTIES, op), true);
		}

		OperationRequestParameters Peer::opCreateRoomImplementation(const JString& gameID, const RoomOptions& options, const Common::Hashtable& customLocalPlayerProperties, const JVector<JString>& expectedUsers)
		{
			OperationRequestParameters op = enterRoomImplementation(&options, customLocalPlayerProperties, expectedUsers);
			if(gameID.length())
				op.put(ParameterCode::ROOM_NAME, ValueObject<JString>(gameID));

			return op;
		}

		OperationRequestParameters Peer::opJoinRoomImplementation(const JString& gameID, const RoomOptions& options, const Hashtable& customLocalPlayerProperties, bool createIfNotExists, bool rejoin, int cacheSliceIndex, const JVector<JString>& expectedUsers)
		{
			OperationRequestParameters op = enterRoomImplementation(createIfNotExists?&options:NULL, customLocalPlayerProperties, expectedUsers);
			op.put(ParameterCode::ROOM_NAME, ValueObject<JString>(gameID));
			if(createIfNotExists)
				op.put(ParameterCode::JOIN_MODE, ValueObject<nByte>(JoinMode::CREATE_IF_NOT_EXISTS));
			else
			{
				if(rejoin)
					op.put(ParameterCode::JOIN_MODE, ValueObject<nByte>(JoinMode::REJOIN_ONLY));
				if(cacheSliceIndex > 0)
					op.put(ParameterCode::CACHE_SLICE_INDEX, ValueObject<int>(cacheSliceIndex));
			}

			return op;
		}

		OperationRequestParameters Peer::enterRoomImplementation(const RoomOptions* pOptions, const Hashtable& customLocalPlayerProperties, const JVector<JString>& expectedUsers)
		{
			OperationRequestParameters op;

			Hashtable playerProps = Utils::stripToCustomProperties(customLocalPlayerProperties);
			if(playerProps.getSize())
				op.put(ParameterCode::PLAYER_PROPERTIES, ValueObject<Hashtable>(playerProps));
			op.put(ParameterCode::BROADCAST, ValueObject<bool>(true));

			if(pOptions)
			{
				Hashtable roomProps(Utils::stripToCustomProperties(pOptions->getCustomRoomProperties()));
				if(!pOptions->getIsOpen())
					roomProps.put(Properties::Room::IS_OPEN, pOptions->getIsOpen());
				if(!pOptions->getIsVisible())
					roomProps.put(Properties::Room::IS_VISIBLE, pOptions->getIsVisible());
				if(pOptions->getMaxPlayers())
					roomProps.put(Properties::Room::MAX_PLAYERS, pOptions->getMaxPlayers());
				if(pOptions->getDirectMode() != DirectMode::NONE)
					roomProps.put(Properties::Room::DIRECT_MODE, pOptions->getDirectMode());

				JString* propsListedInLobbyArr = allocateArray<JString>(pOptions->getPropsListedInLobby().getSize());
				for(unsigned int i=0; i<pOptions->getPropsListedInLobby().getSize(); ++i)
					propsListedInLobbyArr[i] = pOptions->getPropsListedInLobby()[i];
				roomProps.put(Properties::Room::PROPS_LISTED_IN_LOBBY, propsListedInLobbyArr, pOptions->getPropsListedInLobby().getSize());
				deallocateArray(propsListedInLobbyArr);
				op.put(ParameterCode::ROOM_PROPERTIES, ValueObject<Hashtable>(roomProps));

				op.put(ParameterCode::CLEANUP_CACHE_ON_LEAVE, ValueObject<bool>(true));
				if(pOptions->getLobbyName().length())
				{
					op.put(ParameterCode::LOBBY_NAME, ValueObject<JString>(pOptions->getLobbyName()));
					if(pOptions->getLobbyType() != LobbyType::DEFAULT)
						op.put(ParameterCode::LOBBY_TYPE, ValueObject<nByte>(pOptions->getLobbyType()));
				}
				if(pOptions->getPlayerTtl() > 0 || pOptions->getPlayerTtl() == -1)
					op.put(ParameterCode::PLAYER_TTL, ValueObject<int>(pOptions->getPlayerTtl()));
				if(pOptions->getEmptyRoomTtl() > 0)
					op.put(ParameterCode::EMPTY_ROOM_TTL, ValueObject<int>(pOptions->getEmptyRoomTtl()));
				op.put(ParameterCode::CHECK_USER_ON_JOIN, ValueObject<bool>(true));
				if(pOptions->getSuppressRoomEvents())
					op.put(ParameterCode::SUPPRESS_ROOM_EVENTS, ValueObject<bool>(pOptions->getSuppressRoomEvents()));
				if(pOptions->getPlugins())
					op.put(ParameterCode::PLUGINS, ValueObject<const JString*>(pOptions->getPlugins()->getCArray(), static_cast<short>(pOptions->getPlugins()->getSize())));
				if(pOptions->getPublishUserID())
					op.put(ParameterCode::PUBLISH_USER_ID, ValueObject<bool>(pOptions->getPublishUserID()));
			}
			if(expectedUsers.getSize())
				op.put(ParameterCode::ADD, ValueObject<const JString*>(expectedUsers.getCArray(), expectedUsers.getSize()));

			return op;
		}
	}
}
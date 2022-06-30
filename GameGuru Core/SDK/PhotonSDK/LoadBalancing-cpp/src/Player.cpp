/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Player.h"
#include "LoadBalancing-cpp/inc/MutableRoom.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Player.h"

/** @file inc/Player.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Internal;

		/** @class Player
		   Each client inside a MutableRoom is represented by an instance of this class.

		   Player instances are only valid in the context of the MutableRoom() instance from which they have been retrieved.
		   @sa MutablePlayer, MutableRoom, MutableRoom::getPlayers(), MutableRoom::getPlayerForNumber() */

		Player::Player(void)
			: mNumber(-1)
			, mpRoom(NULL)
			, mIsInactive(false)
		{
		}

		Player::Player(int number, const Hashtable& properties, const MutableRoom* const pRoom)
			: mNumber(number)
			, mpRoom(pRoom)
			, mIsInactive(false)
		{
			cacheProperties(properties);
		}

		/**
		   Destructor. */
		Player::~Player(void)
		{
		}

		/**
		   Copy-Constructor: Creates a new instance that is a deep copy of the argument instance.
		   @param toCopy The instance to copy. */
		Player::Player(const Player& toCopy)
		{
			*this = toCopy;
		}

		/**
		   operator=.
	   
		   Makes a deep copy of its right operand into its left operand.
	   
		   This overwrites old data in the left operand. */
		Player& Player::operator=(const Player& toCopy)
		{
			return assign(toCopy);
		}

		bool Player::getIsMutable(void) const
		{
			return false;
		}

		Player& Player::assign(const Player& toCopy)
		{
			mNumber = toCopy.mNumber;
			mName = toCopy.mName;
			mUserID = toCopy.mUserID;
			mCustomProperties = toCopy.mCustomProperties;
			mpRoom = toCopy.mpRoom;
			return *this;
		}

		/**
		   @returns the player number
		   @details
		   The player number serves as a the unique identifier of a player inside the current room.

		   It is assigned per room and only valid in the context of that room. A player number is never re-used for another player inside the same room.

		   If a player leaves a room entirely (either explicitly through a call to Client::opLeaveRoom() without passing 'true' for parameter 'willComeBack' or implicitly because his playerTtl runs out (see RoomOptions::setPlayerTtl())) and joins it again
		   afterwards, then he is treated as an entirely new player and gets assigned a new player number.

		   If a player becomes inactive (either explicitly through a call to Client::opLeaveRoom() with passing 'true' for parameter 'willComeBack' or implicitly by by getting disconnected) and then rejoins the same room before his playerTtl runs out, then he
		   is treated as the same player an keeps his previously assigned player number. */
		int Player::getNumber(void) const
		{
			return mNumber;
		}

		/**
		   @returns the non-unique nickname of this player
		   @details
		   A player might change his own name.

		   Such a change is synced automatically with the server and other clients in the same room. */
		const JString& Player::getName(void) const
		{
			return mName;
		}

		/**
		   @returns the unique user ID of this player
		   @details
		   This value is only available when the room got created with RoomOptions::setPublishUserId(true). Otherwise it will be an empty string.

		   Useful for Client::opFindFriends() and and for blocking slots in a room for expected users (see MutableRoom::getExpectedUsers()).
		   @sa AuthenticationValues */
		const JString& Player::getUserID(void) const
		{
			return mUserID;
		}

		/**
		   @returns the custom properties of this player
		   @details
		   Read-only cache for the custom properties of a player. A client can change the custom properties of his local player instance through class MutablePlayer. The Custom Properties of remote players are automatically updated when they change. */
		const Hashtable& Player::getCustomProperties(void) const
		{
			return mCustomProperties;
		}

		void Player::setMutableRoomPointer(const MutableRoom* pRoom)
		{
			mpRoom = pRoom;
		}

		/**
		   @returns 'true' if a player is inactive, 'false' otherwise.
		   @details
		   Inactive players keep their spot in a room but otherwise behave as if offline (no matter what their actual connection status is).

		   The room needs a PlayerTtl != 0 (see RoomOptions::setPlayerTtl()) for players to be able  to become inactive. If a player is inactive for longer than the PlayerTtl, then the server will remove this player from the room. */
		bool Player::getIsInactive(void) const
		{
			return mIsInactive;
		}

		void Player::setIsInactive(bool isInActive)
		{
			mIsInactive = isInActive;
		}

		/**
		   @returns 'true' if this player is the Master Client of the current room, 'false' otherwise.
		   @details
		   There is always exactly one master client. The creator of a room gets assigned the role of master client on room creation.

		   When the current master client leaves the room or becomes inactive and there is at least one active player inside the room, then the role of master client gets reassigned by the server to an active client. As soon as one client becomes active again
		   in a room with only inactive clients, the role of master client will be assigned to this active client.

		   Whenever the role of master client gets assigned to a different client, all active clients inside the same room get informed about it by a call to Listener::onMasterClientChanged().

		   You can use the master client when you want one client to be an authoritative instance.
		   @sa MutableRoom::getMasterClientID(), Listener::onMasterClientChanged(), DirectMode::MASTER_TO_ALL */
		bool Player::getIsMasterClient(void) const
		{
			return mpRoom?mNumber==mpRoom->getMasterClientID():false;
		}

		void Player::cacheProperties(const Hashtable& properties)
		{
			if(properties.contains(Properties::Player::PLAYERNAME))
				mName = ValueObject<JString>(properties.getValue(Properties::Player::PLAYERNAME)).getDataCopy();
			if(properties.contains(Properties::Player::IS_INACTIVE))
				mIsInactive = ValueObject<bool>(properties.getValue(Properties::Player::IS_INACTIVE)).getDataCopy();
			if(properties.contains(Properties::Player::USER_ID))
				mUserID = ValueObject<JString>(properties.getValue(Properties::Player::USER_ID)).getDataCopy();
			mCustomProperties.put(Utils::stripToCustomProperties(properties));
			mCustomProperties = Utils::stripKeysWithNullValues(mCustomProperties);
		}

		/**
		   operator==.
		   @returns true, if both operands are equal, false otherwise.
		   @details
		   Two Player instances are considered equal, if getNumber() returns equal values for both of them. */
		bool Player::operator==(const Player& player) const
		{
			return getNumber() == player.getNumber();
		}

		JString& Player::toString(JString& retStr, bool withTypes) const
		{
			return retStr += toString(withTypes, false);
		}

		/**
		   @overload
		   @param withTypes set to true, to include type information in the generated string
		   @param withCustomProperties set to true, to include the custom properties in the generated string */
		JString Player::toString(bool withTypes, bool withCustomProperties) const
		{
			return JString() + mNumber + L"={" + payloadToString(withCustomProperties, withTypes) + L"}";
		}

		JString Player::payloadToString(bool withTypes, bool withCustomProperties) const
		{
			JString res = JString(L"num: ") + mNumber + L" name: " + mName + L" userID: " + mUserID;
			if(withCustomProperties && mCustomProperties.getSize())
				res += L" props: " + mCustomProperties.toString(withTypes);
			return res;
		}
	}
}
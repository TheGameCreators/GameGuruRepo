/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Room.h"
#include "LoadBalancing-cpp/inc/Enums/DirectMode.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Room.h"

/** @file inc/Room.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Common::MemoryManagement;
		using namespace Internal;

		/** @class Room
		   Each visible room inside the list of rooms in a lobby of type LobbyType::DEFAULT is represented by an instance of this class.

		   The information that is available through the various getters is regularly updated by the server as long as the client is inside the lobby.
		   It is not updated and information will become outdated while the client is inside of a room.

		   @sa MutableRoom, Client::getRoomList() */

		Room::Room(void)
			: mPlayerCount(0)
			, mMaxPlayers(0)
			, mIsOpen(false)
			, mDirectMode(DirectMode::NONE)
		{
		}

		Room::Room(const JString& name, const Hashtable& properties)
			: mName(name)
			, mPlayerCount(0)
			, mMaxPlayers(0)
			, mIsOpen(true)
			, mDirectMode(DirectMode::NONE)
		{
			cacheProperties(properties);
		}

		/**
		   Destructor. */
		Room::~Room(void)
		{
		}

		/**
		   Copy-Constructor: Creates a new instance that is a deep copy of the argument instance.
		   @param toCopy The instance to copy. */
		Room::Room(const Room& toCopy)
		{
			*this = toCopy;
		}

		/**
		   operator=.
	   
		   Makes a deep copy of its right operand into its left operand.
	   
		   This overwrites old data in the left operand. */
		Room& Room::operator=(const Room& toCopy)
		{
			return assign(toCopy);
		}

		bool Room::getIsMutable(void) const
		{
			return false;
		}

		Room& Room::assign(const Room& toCopy)
		{
			mName = toCopy.mName;
			mPlayerCount = toCopy.mPlayerCount;
			mMaxPlayers = toCopy.mMaxPlayers;
			mIsOpen = toCopy.mIsOpen;
			mDirectMode = toCopy.mDirectMode;
			mCustomProperties = toCopy.mCustomProperties;
			return *this;
		}

		/**
		   @returns the name of the room
		   @details
		   A rooms name is a unique identifier (per region and virtual appid) for a room/match.

		   It can be set set by the client on room creation as parameter of Client::opCreateRoom() or Client::opJoinOrCreateRoom().

		   The name can't be changed once the room is created. */
		const JString& Room::getName(void) const
		{
			return mName;
		}

		/**
		   @returns the  count of players that are currently inside this room */
		nByte Room::getPlayerCount(void) const
		{
			return mPlayerCount;
		}

		/**
		   @returns the limit of players for this room.
		   If a room is full (getPlayerCount() == getMaxPlayers()), joining this room will fail. */
		nByte Room::getMaxPlayers(void) const
		{
			return mMaxPlayers;
		}

		/**
		   @returns 'true' if the room can be joined, 'false' otherwise.
		   @details
		   This does not affect listing in a lobby but joining a room will fail if it is not open.

		   If it is not open, then a room is excluded from random matchmaking.

		   Due to racing conditions, found matches might become closed even while you join them. Simply find another room in this scenario. */
		bool Room::getIsOpen(void) const
		{
			return mIsOpen;
		}

		/**
		   @returns one of the values in DirectMode
		   @details
		   This returns DirectMode::NONE, unless the client that created the room has set something else through RoomOptions::setDirectMode() */
		nByte Room::getDirectMode(void) const
		{
			return mDirectMode;
		}

		/**
		   @returns the custom properties of this room
		   @details
		   Read-only cache for those custom properties of a room, which have been included in the list of properties to show in lobby (see RoomOptions::setPropsListedInLobby() and MutableRoom::setPropsListedInLobby()).

		   A client can change the custom properties of the currently joined room through class MutableRoom. The initial custom properties of a room can be set through class RoomOptions. */
		const Hashtable& Room::getCustomProperties(void) const
		{
			return mCustomProperties;
		}

		void Room::cacheProperties(const Hashtable& properties)
		{
			if(properties.contains(Properties::Room::MAX_PLAYERS))
				mMaxPlayers = ValueObject<nByte>(properties.getValue(Properties::Room::MAX_PLAYERS)).getDataCopy();
			if(properties.contains(Properties::Room::IS_OPEN))
				mIsOpen = ValueObject<bool>(properties.getValue(Properties::Room::IS_OPEN)).getDataCopy();
			if(properties.contains(Properties::Room::PLAYER_COUNT))
				mPlayerCount = ValueObject<nByte>(properties.getValue(Properties::Room::PLAYER_COUNT)).getDataCopy();
			if(properties.contains(Properties::Room::DIRECT_MODE))
				mDirectMode = ValueObject<nByte>(properties.getValue(Properties::Room::DIRECT_MODE)).getDataCopy();

			mCustomProperties.put(Utils::stripToCustomProperties(properties));
			mCustomProperties = Utils::stripKeysWithNullValues(mCustomProperties);
		}

		/**
		   operator==.
		   @returns true, if both operands are equal, false otherwise.
		   @details
		   Two Room instances are considered equal, if getName() returns equal values for both of them. */
		bool Room::operator==(const Room& room) const
		{
			return getName() == room.getName();
		}

		JString& Room::toString(JString& retStr, bool withTypes) const
		{
			return retStr += toString(withTypes, false);
		}

		/**
		   @overload
		   @param withTypes set to true, to include type information in the generated string
		   @param withCustomProperties set to true, to include the custom properties in the generated string */
		JString Room::toString(bool withTypes, bool withCustomProperties) const
		{
			return mName + L"={" + payloadToString(withTypes, withCustomProperties) + L"}";
		}

		JString Room::payloadToString(bool withTypes, bool withCustomProperties) const
		{
			JString res = mName + L" pl: " + mPlayerCount + L" max: " + mMaxPlayers + L" open: " + mIsOpen + L" direct: " + mDirectMode;
			if(withCustomProperties && mCustomProperties.getSize())
				res += L" props:" + mCustomProperties.toString(withTypes);
			return res;
		}
	}
}
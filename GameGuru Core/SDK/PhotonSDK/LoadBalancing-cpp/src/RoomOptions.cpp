/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/RoomOptions.h"

/** @file LoadBalancing-cpp/inc/RoomOptions.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Common::MemoryManagement;

		/** @class RoomOptions
		   This class aggregates the various optional parameters that can be specified on room creation.
		   @sa Client::opCreateRoom(), Client::opJoinOrCreateRoom() */

		/**
		   Constructor: Creates a new instance with the specified parameters.
		   @param isVisible see setIsVisible() - optional, defaults to true.
		   @param isOpen see setIsOpen() - optional, defaults to true.
		   @param maxPlayers see setMaxPlayers() - optional, defaults to 0.
		   @param customRoomProperties see setCustomRoomProperties() - optional, defaults to an empty Hashtable instance.
		   @param propsListedInLobby see setPropsListedInLobby() - optional, defaults to an empty JVector instance.
		   @param lobbyName see setLobbyName() - optional, defaults to an empty JString instance.
		   @param lobbyType see setLobbyType() - optional, defaults to LobbyType::DEFAULT.
		   @param playerTtl see setPlayerTtl() - optional, defaults to 0.
		   @param emptyRoomTtl see setEmptyRoomTtl() - optional, defaults to 0.
		   @param suppressRoomEvents see setSuppressRoomEvents() - optional, defaults to false.
		   @param pPlugins see setPlugins() - optional, defaults to NULL.
		   @param publishUserID see setPublishUserID() - optional, defaults to false.
		   @param directMode see setDirectMode() - optional, defaults to DirectMode::NONE. */
		RoomOptions::RoomOptions(bool isVisible, bool isOpen, nByte maxPlayers, const Hashtable& customRoomProperties, const JVector<JString>& propsListedInLobby, const JString& lobbyName, nByte lobbyType, int playerTtl, int emptyRoomTtl, bool suppressRoomEvents, const JVector<JString>* pPlugins, bool publishUserID, nByte directMode)
			: mIsVisible(isVisible)
			, mIsOpen(isOpen)
			, mMaxPlayers(maxPlayers)
			, mCustomRoomProperties(customRoomProperties)
			, mPropsListedInLobby(propsListedInLobby)
			, mLobbyName(lobbyName)
			, mLobbyType(lobbyType)
			, mPlayerTtl(playerTtl)
			, mEmptyRoomTtl(emptyRoomTtl)
			, mSuppressRoomEvents(suppressRoomEvents)
			, mupPlugins(pPlugins?allocate<JVector<JString> >(*pPlugins):NULL)
			, mPublishUserID(publishUserID)
			, mDirectMode(directMode)
		{
		}
		
		/**
		   Destructor. */
		RoomOptions::~RoomOptions(void)
		{
		}
		
		/**
		   Copy-Constructor: Creates a new instance that is a deep copy of the argument instance.
		   @param toCopy The instance to copy. */
		RoomOptions::RoomOptions(const RoomOptions& toCopy)
		{
			*this = toCopy;
		}
		
		/**
		   operator=.
	   
		   Makes a deep copy of its right operand into its left operand.
	   
		   This overwrites old data in the left operand. */
		RoomOptions& RoomOptions::operator=(const RoomOptions& toCopy)
		{
			mIsVisible = toCopy.mIsVisible;
			mIsOpen = toCopy.mIsOpen;
			mMaxPlayers = toCopy.mMaxPlayers;
			mCustomRoomProperties = toCopy.mCustomRoomProperties;
			mPropsListedInLobby = toCopy.mPropsListedInLobby;
			mLobbyName = toCopy.mLobbyName;
			mLobbyType = toCopy.mLobbyType;
			mPlayerTtl = toCopy.mPlayerTtl;
			mEmptyRoomTtl = toCopy.mEmptyRoomTtl;
			mSuppressRoomEvents = toCopy.mSuppressRoomEvents;
			mupPlugins = toCopy.mupPlugins?allocate<JVector<JString> >(*toCopy.mupPlugins):NULL;
			mPublishUserID = toCopy.mPublishUserID;
			mDirectMode = toCopy.mDirectMode;

			return *this;
		}
		
		/**
		   @returns the currently set value for the isVisible flag
		   @sa
		   setIsVisible() */
		bool RoomOptions::getIsVisible(void) const
		{
			return mIsVisible;
		}
		
		/**
		   Sets the initial state of the rooms visibility flag.
		   @details
		   A room that is not visible is excluded from the room lists that are sent to the clients in lobbies.
		   An invisible room can be joined by name but is excluded from random matchmaking.

		   Use this to "hide" a room and simulate "private rooms". Players can exchange a room name and create the room as invisible to avoid anyone else joining it.
		   @remarks
		   This function sets the initial value that is used for room creation. To change the value of the flag for an already existing room, see MutableRoom::setIsVisible().
		   @param isVisible the new value to which the flag will be set
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getIsVisible() */
		RoomOptions& RoomOptions::setIsVisible(bool isVisible)
		{
			mIsVisible = isVisible;
			return *this;
		}
		
		/**
		   @returns the currently set value for the isOpen flag
		   @sa
		   setIsOpen() */
		bool RoomOptions::getIsOpen(void) const
		{
			return mIsOpen;
		}
		
		/**
		   Sets the initial state of the rooms isOpen flag.
		   @details
		   If a room is closed, then no further player can join it until the room gets reopened again.
		   A closed room can still be listed in the lobby (use setIsVisible() to control lobby-visibility).
		   @remarks
		   This function sets the initial value that is used for room creation. To change the value of the flag for an already existing room, see MutableRoom::setIsOpen().
		   @param isOpen the new value to which the flag will be set
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getIsOpen() */
		RoomOptions& RoomOptions::setIsOpen(bool isOpen)
		{
			mIsOpen = isOpen;
			return *this;
		}
		
		/**
		   @returns the currently set max players
		   @sa
		   setmaxPlayers() */
		nByte RoomOptions::getMaxPlayers(void) const
		{
			return mMaxPlayers;
		}
		
		/**
		   Sets the initial value for the max players setting of the room.
		   @details
		   This function sets the maximum number of players that can be inside the room at the same time, including inactive players. 0 means "no limit".
		   @remarks
		   This function sets the initial value that is used for room creation. To change the max players setting of an already existing room, see MutableRoom::setMaxPlayers().
		   @param maxPlayers the new maximum amount of players that can be inside the room at the same time
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getMaxPlayers() */
		RoomOptions& RoomOptions::setMaxPlayers(nByte maxPlayers)
		{
			mMaxPlayers = maxPlayers;
			return *this;
		}
		
		/**
		   @returns the currently set custom room properties
		   @sa
		   setCustomRoomProperties() */
		const Hashtable& RoomOptions::getCustomRoomProperties(void) const
		{
			return mCustomRoomProperties;
		}
		
		/**
		   Sets the initial custom properties of a room.
		   @details
		   Custom room properties are any key-value pairs that you need to define the game's setup.
		   The shorter your key strings are, the better.
		   Example: Map, Mode (could be L"m" when used with L"Map"), TileSet (could be L"t").
		   @note
		   JString is the only supported type for custom property keys. For custom property values you can use any type that is listed in the @link Datatypes Table of Datatypes\endlink.
		   @remarks
		   This function sets the initial custom properties that are used for room creation. To change the custom properties of an already existing room, see MutableRoom::mergeCustomProperties(), MutableRoom::addCustomProperty(), MutableRoom::addCustomProperties(),
		   MutableRoom::removeCustomProperty() and MutableRoom::removeCustomProperties().
		   @param customRoomProperties a Hashtable of custom property key-value pairs
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getCustomRoomProperties(), setPropsListedInLobby() */
		RoomOptions& RoomOptions::setCustomRoomProperties(const Hashtable& customRoomProperties)
		{
			mCustomRoomProperties = customRoomProperties; 
			return *this;
		}
		
		/**
		   @returns the currently set list of properties to show in the lobby
		   @sa
		   setPropsListedInLobby() */
		const JVector<JString>& RoomOptions::getPropsListedInLobby(void) const
		{
			return mPropsListedInLobby;
		}
		
		/**
		   Sets the initial list of custom properties of the room that should be shown in the lobby.
		   @details
		   List the keys of all the custom room properties that should be available to clients that are in a lobby.
		   Use with care. Unless a custom property is essential for matchmaking or user info, it should not be sent to the lobby, which causes traffic and delays for clients in the lobby.
		   Default: No custom properties are sent to the lobby.
		   @note
		   Properties that are intended to be shown in the lobby should be as compact as possible. Literally every single byte counts here as this info needs to be sent to every client in the lobby for every single visible room, so that with lots of users online and
		   games running this quickly adds up to a lot of data.
		   @remarks
		   This function sets the initial list of property keys. To change which properties are shown to the lobby for an already existing room see MutableRoom::setPropsListedInLobby().
		   @param propsListedInLobby the keys of the custom room properties that should be shown in the lobby
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getPropsListedInLobby(), setCustomRoomProperties() */
		RoomOptions& RoomOptions::setPropsListedInLobby(const JVector<JString>& propsListedInLobby)
		{
			mPropsListedInLobby = propsListedInLobby;
			return *this;
		}
		
		/**
		   @returns the currently set lobby name
		   @sa
		   setLobbyName() */
		const JString& RoomOptions::getLobbyName(void) const
		{
			return mLobbyName;
		}

		/**
		   Sets the name of the lobby to which the room gets added to.
		   @details
		   Rooms can be assigned to different lobbies on room creation.
		   Client::opJoinRandomRoom() only uses those room for matchmaking that are assigned to the lobby in which it is told to be looking for rooms.
		   A lobby can be joined by a call to Client::opJoinLobby() and inside lobbies of certain types clients can receive room lists that contain all visible rooms that are assigned to that lobby.
		   @remarks
		   If you don't set a lobby name or if you set it to an empty string, then any value that is passed for setLobbyType() gets ignored and the room gets added to the default lobby.
		   @remarks
		   Lobbies are unique per lobbyName plus lobbyType, so multiple different lobbies may have the same name, as long as they are of a different type.
		   @param lobbyName identifies for the lobby and needs to be unique within the scope of the lobbyType
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getLobbyName(), setLobbyType(), <a href="https://doc.photonengine.com/en/realtime/current/reference/matchmaking-and-lobby">Matchmaking Guide</a> */
		RoomOptions& RoomOptions::setLobbyName(const JString& lobbyName)
		{
			mLobbyName = lobbyName;
			return *this;
		}
		
		/**
		   @returns the currently set lobby type
		   @sa
		   setLobbyType() */
		nByte RoomOptions::getLobbyType(void) const
		{
			return mLobbyType;
		}
		
		/**
		   Sets the type of the lobby to which the room gets added to. Must be one of the values in LobbyType
		   @details
		   Please see <a href="https://doc.photonengine.com/en/realtime/current/reference/matchmaking-and-lobby">Matchmaking Guide</a> regarding the differences between the various lobby types.
		   @note
		   This option gets ignored and the room gets added to the default lobby, if you don't also set the lobby name to a non-empty string via a call to setLobbyName().
		   @param lobbyType one of the values in LobbyType
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getLobbyType(), setLobbyName(), LobbyType, <a href="https://doc.photonengine.com/en/realtime/current/reference/matchmaking-and-lobby">Matchmaking Guide</a> */
		RoomOptions& RoomOptions::setLobbyType(nByte lobbyType)
		{
			mLobbyType = lobbyType;
			return *this;
		}
		
		/**
		   @returns the currently set player time to live in milliseconds
		   @sa
		   setPlayerTtl() */
		int RoomOptions::getPlayerTtl(void) const
		{
			return mPlayerTtl;
		}
		
		/**
		   Sets the player time to live in milliseconds.
		   @details
		   If a client disconnects or if it leaves a room with the 'willComeBack' flag set to true, its player becomes inactive first and only gets removed from the room after this timeout.
		   @li -1 and INT_MAX set the inactivity time to 'infinite'.
		   @li 0 (default) deactivates player inactivity.
		   @li All other positive values set the inactivity time to their value in milliseconds.
		   @li All other negative values get ignored and the behavior for them is as if the default value was used.

		   @note
		   A player is only able to rejoin a room in its existing player slot while it is still inactive. Once it has left for good it will be treated as a completely new player.
		   @param playerTtl a value between -1 and INT_MAX
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getPlayerTtl(), Client::opLeaveRoom(), Client::disconnect() */
		RoomOptions& RoomOptions::setPlayerTtl(int playerTtl)
		{
			mPlayerTtl = playerTtl;
			return *this;
		}
		
		/**
		   @returns the currently set empty room time to live in milliseconds
		   @sa
		   setEmptyRoomTtl() */
		int RoomOptions::getEmptyRoomTtl(void) const
		{
			return mEmptyRoomTtl;
		}
		
		/**
		   Sets the room time to live in milliseconds.
		   @details
		   The amount of time in milliseconds that %Photon servers should wait before disposing an empty room. A room is considered empty when there is no active player joined to it. So the room disposal timer starts when the last active player leaves. When a player joins
		   or rejoins the room, then the countdown is reset.

		   By default, the maximum value allowed is:
		   @li 300000ms (5 minutes) on %Photon Cloud
		   @li 60000ms (1minute) on %Photon Server

		   @li 0 (default) means that an empty room gets instantly disposed.
		   @li All positive values set the keep-alive time to their value in milliseconds.
		   @li All negative values get ignored and the behavior for them is as if the default value was used.

		   @note
		   The disposal of a room means that the room gets removed from memory on the server side.
		   Without accordingly configured Webhooks this also means that the room will be destroyed and all data related to it (like room and player properties, event caches, inactive players, etc.) gets deleted.
		   When <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks-faq">Webhooks</a> for the used appID have been been setup for <a href="https://doc.photonengine.com/en-us/realtime/current/tutorials/persistence-guide">room persistence</a>,
		   then disposed rooms get stored for later retrieval. Stored rooms get reconstructed in memory when a player joins or rejoins them
		   @param emptyRoomTtl a value between 0 and INT_MAX
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getEmptyRoomTtl(),  <a href="https://doc.photonengine.com/en-us/realtime/current/tutorials/persistence-guide">Persistence Guide</a>, <a href="https://doc.photonengine.com/en-us/realtime/current/reference/webhooks-faq">Webhooks FAQ</a> */
		RoomOptions& RoomOptions::setEmptyRoomTtl(int emptyRoomTtl)
		{
			mEmptyRoomTtl = emptyRoomTtl;
			return *this;
		}
		
		/**
		   @returns the currently set value for the suppressRoomEvents flag
		   @sa
		   setSuppressRoomEvents() */
		bool RoomOptions::getSuppressRoomEvents(void) const
		{
			return mSuppressRoomEvents;
		}
		
		/**
		   Sets the value of the suppressRoomEvents flag which determines if the server should skip room events for joining and leaving players.
		   @details
		   Setting this flag to true makes the client unaware of the other players in a room. That can save some traffic if you have some server logic that updates players, but it can also limit the client's usability.
		   @param suppressRoomEvents the new value to which the flag will be set
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getSuppressRoomEvents() */
		RoomOptions& RoomOptions::setSuppressRoomEvents(bool suppressRoomEvents)
		{
			mSuppressRoomEvents = suppressRoomEvents;
			return *this;
		}
		
		/**
		   @returns the currently set list of plugins
		   @sa
		   setPlugins() */
		const JVector<JString>* RoomOptions::getPlugins(void) const
		{
			return mupPlugins;
		}
		
		/**
		   Informs the server of the expected plugin setup.
		   @details
		   The operation will fail in case of a plugin mismatch returning ErrorCode::PLUGIN_MISMATCH.
		   Setting an empty JVector means that the client expects no plugin to be setup.
		   Note: for backwards compatibility setting NULL (the default value) omits any check.
		   @param pPlugins the expected plugins
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getPlugins() */
		RoomOptions& RoomOptions::setPlugins(const JVector<JString>* pPlugins)
		{
			mupPlugins = pPlugins?allocate<JVector<JString> >(*pPlugins):NULL;
			return *this;
		}
		
		/**
		   @returns the currently set value for the publishUserID flag
		   @sa
		   setPublishUserID() */
		bool RoomOptions::getPublishUserID(void) const
		{
			return mPublishUserID;
		}
		
		/**
		   Defines if the UserIds of players get "published" in the room. Useful for Client::opFindFriends(), if players want to play another game together.
		   @details
		   When you set this to true, %Photon will publish the UserIds of the players in that room.
		   In that case, you can use Player::getUserID(), to access any player's userID.
		   This is useful for FindFriends and to set "expected users" to reserve slots in a room (see Client::opCreateRoom(), Client::opJoinOrCreateRoom() and Client::opJoinRoom()).
		   @param publishUserID true, if userIDs should be published, false otherwise
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getPublishUserID() */
		RoomOptions& RoomOptions::setPublishUserID(bool publishUserID)
		{
			mPublishUserID = publishUserID;
			return *this;
		}
		
		/**
		   @returns the currently set value for the DirectMode flag
		   @sa
		   setDirectMode() */
		nByte RoomOptions::getDirectMode(void) const
		{
			return mDirectMode;
		}
		
		/**
		   Sets the DirectMode that should be used for this room.
		   @details
		   The value of this option determines if clients establish direct peer to peer connections with other clients that can then be used to send them direct peer to peer messages with Client::sendDirect().
		   @param directMode one of the values in DirectMode
		   @returns a reference to the instance on which it was called to allow for chaining multiple setter calls
		   @sa
		   getDirectMode() */
		RoomOptions& RoomOptions::setDirectMode(nByte directMode)
		{
			mDirectMode = directMode;
			return *this;
		}

		JString& RoomOptions::toString(JString& retStr, bool withTypes) const
		{
			return retStr += JString(L"{isVisible=") + getIsVisible() + L", isOpen=" + getIsOpen() + L", maxPlayers=" + getMaxPlayers() + L", customRoomProperties=" + getCustomRoomProperties().toString(withTypes) + L", propsListedInLobby=" + getPropsListedInLobby().toString() + L", lobbyName=" + getLobbyName().toString() + L", lobbyType" + getLobbyType() + L", playerTtl=" + getPlayerTtl() + L", emptyRoomTtl=" + getEmptyRoomTtl() + L", suppressRoomEvents=" + getSuppressRoomEvents() + L", plugins=" + (getPlugins()?getPlugins()->toString(withTypes):L"") + L", publishUserID=" + getPublishUserID()  + L", direct=" + getDirectMode() + L"}";
		}
	}
}
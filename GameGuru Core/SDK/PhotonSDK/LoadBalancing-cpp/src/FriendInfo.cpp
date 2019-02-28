/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/FriendInfo.h"

/** @file inc/FriendInfo.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		
		/** @class FriendInfo
		   Used to store the information about a friend's online status and in which room he/she is active.
		   @sa Client::getFriendList(), Client::getFriendListAge(), Client::opFindFriends(), Listener::onFindFriendsResponse() */

		FriendInfo::FriendInfo(const JString& userID, bool isOnline, const JString& room)
			: mUserID(userID)
			, mIsOnline(isOnline)
			, mRoom(room)
		{
		}
		
		/**
		   @returns the user ID of the friend */
		JString FriendInfo::getUserID(void) const
		{
			return mUserID;
		}
		
		/**
		   @returns true if the friend is online, false otherwise */
		bool FriendInfo::getIsOnline(void) const
		{
			return mIsOnline;
		}
		
		/**
		   @returns the name of the room in which the friend currently is active in, or an empty string, if it is not active inside any room at all. */
		JString FriendInfo::getRoom(void) const
		{
			return mRoom;
		}
		
		/**
		   @returns true if the friend is active inside a room, false otherwise. */
		bool FriendInfo::getIsInRoom(void) const
		{
			return !!getRoom().length();
		}

		JString& FriendInfo::toString(JString& retStr, bool withTypes) const
		{
			return retStr += L"{" + getUserID() + L" " + (getIsOnline()?(L"on, " + (getIsInRoom()?L"in room " + getRoom():L"not in a room")):L"off") + L"}";
		}
	}
}
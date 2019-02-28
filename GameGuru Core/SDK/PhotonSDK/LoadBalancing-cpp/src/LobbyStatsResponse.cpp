/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/LobbyStatsResponse.h"

/** @file LobbyStatsResponse.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		/** @class LobbyStatsResponse
		   Passed to Listener::onLobbyStatsResponse(), Listener::onLobbyStatsUpdate(). Each instance of this class holds the name, the type and the statistics (peer count and room count) of one specific lobby. Each lobby can be uniquely identified by the combination of its name and type.
		   @sa Client::opLobbyStats(), Listener::onLobbyStatsResponse(), Listener::onLobbyStatsUpdate(), LobbyStatsRequest */

		LobbyStatsResponse::LobbyStatsResponse(const JString& name, nByte type, int peerCount, int roomCount)
			: mName(name)
			, mType(type)
			, mPeerCount(peerCount)
			, mRoomCount(roomCount) 
		{
		}

		/**
		   @returns the lobby name. Each lobby can be uniquely identified by the combination of its name and type. */
		const JString& LobbyStatsResponse::getName(void) const
		{
			return mName;
		}

		/**
		   @returns the lobby type. Each lobby can be uniquely identified by the combination of its name and type. */
		nByte LobbyStatsResponse::getType(void) const
		{
			return mType;
		}

		/**
		   @returns the number of clients that currently reside in this specific lobby */
		int LobbyStatsResponse::getPeerCount(void) const
		{
			return mPeerCount;
		}

		/**
		   @returns the number of clients that currently exist and that belong to this specific lobby.
		   @details
		   On room creation the creator of the room can specify the name and type of the lobby to which that room gets assigned in the RoomOptions. */
		int LobbyStatsResponse::getRoomCount(void) const
		{
			return mRoomCount;
		}

		JString& LobbyStatsResponse::toString(JString& retStr, bool withTypes) const
		{
			return retStr += L"{name=" + mName.toString() + L", type=" + mType + L", peerCount=" + mPeerCount + L", roomCount=" + mRoomCount + L"}";
		}
	}
}
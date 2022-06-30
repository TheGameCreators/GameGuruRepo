/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/LobbyStatsRequest.h"

/** @file LobbyStatsRequest.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;

		/** @class LobbyStatsRequest
		   Passed to Client::opLobbyStats(). Each instance of this class holds the name and the type of a lobby for which the caller of Client::opLobbyStats() wants to request statistics.
		   @sa Client::opLobbyStats(), Listener::onLobbyStatsResponse(), Listener::onLobbyStatsUpdate(), LobbyStatsResponse */
		
		/**
		   Constructor: Creates a new instance with the specified parameters.
		   @note Lobby names are only unique per lobby type and multiple lobbies with the same name, but different type, can exist in parallel. Hence a lobby with the same name but with a different type is treated as a different lobby.
		   @param name see setLobbyName() - optional, defaults to an empty JString instance.
		   @param type see setLobbyType() - optional, defaults to LobbyType::DEFAULT. Must be one of the values in LobbyType */
		LobbyStatsRequest::LobbyStatsRequest(const JString& name, nByte type)
			: mName(name)
			, mType(type)
		{
		}

		/**
		   @returns the lobby name
		   @sa
		   LobbyStatsRequest() */
		const JString& LobbyStatsRequest::getName(void) const
		{
			return mName;
		}

		/**
		   @returns the lobby type
		   @sa
		   LobbyStatsRequest() */
		nByte LobbyStatsRequest::getType(void) const
		{
			return mType;
		}

		JString& LobbyStatsRequest::toString(JString& retStr, bool withTypes) const
		{
			return retStr += L"{name=" + mName + L", type=" + mType + L"}";
		}
	}
}
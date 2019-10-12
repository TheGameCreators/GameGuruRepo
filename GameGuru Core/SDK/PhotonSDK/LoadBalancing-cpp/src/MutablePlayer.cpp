/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Client.h"
#include "LoadBalancing-cpp/inc/Internal/Utils.h"
#include "LoadBalancing-cpp/inc/Internal/Enums/Properties/Player.h"

/** @file inc/MutablePlayer.h */

namespace ExitGames
{
	namespace LoadBalancing
	{
		using namespace Common;
		using namespace Internal;

		MutablePlayer::MutablePlayer(int number, const Hashtable& properties, const MutableRoom* const pRoom, Client* pClient)
			: Player(number, properties, pRoom)
			, mpLoadBalancingClient(pClient)
		{
		}

		MutablePlayer::~MutablePlayer(void)
		{
		}

		MutablePlayer::MutablePlayer(const MutablePlayer& toCopy) : Player(toCopy)
		{
			*this = toCopy;
		}

		MutablePlayer& MutablePlayer::operator=(const Player& toCopy)
		{
			return assign(toCopy);
		}

		MutablePlayer& MutablePlayer::operator=(const MutablePlayer& toCopy)
		{
			return assign(toCopy);
		}

		bool MutablePlayer::getIsMutable(void) const
		{
			return true;
		}

		MutablePlayer& MutablePlayer::assign(const Player& toCopy)
		{
			super::assign(toCopy);
			const MutablePlayer& temp = static_cast<const MutablePlayer&>(toCopy);
			if(temp.getIsMutable())
				mpLoadBalancingClient = temp.mpLoadBalancingClient;
			return *this;
		}

		void MutablePlayer::mergeCustomProperties(const Hashtable& customProperties, const WebFlags& webflags)
		{
			Hashtable stripDict = Utils::stripToCustomProperties(customProperties);
			if(!stripDict.getSize())
				return;
			Hashtable oldDict = mCustomProperties;
			mCustomProperties.put(stripDict);
			mCustomProperties = Utils::stripKeysWithNullValues(mCustomProperties);
			if(mCustomProperties != oldDict)
				mpLoadBalancingClient->opSetPropertiesOfPlayer(mNumber, stripDict, Hashtable(), webflags);
		}

		void MutablePlayer::addCustomProperties(const Hashtable& customProperties, const WebFlags& webflags)
		{
			mergeCustomProperties(Utils::stripKeysWithNullValues(customProperties), webflags);
		}

		void MutablePlayer::setName(const JString& name, const WebFlags& webflags)
		{
			if(mName == name)
				return;
			Hashtable properties;
			properties.put(static_cast<nByte>(Properties::Player::PLAYERNAME), mName=name);
			mpLoadBalancingClient->opSetPropertiesOfPlayer(mNumber, properties, Hashtable(), webflags);
		}
	}
}
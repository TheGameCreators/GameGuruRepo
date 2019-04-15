/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Internal/PlayerPropertiesUpdateInformant.h"
#include "LoadBalancing-cpp/inc/MutableRoom.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;

			void PlayerPropertiesUpdateInformant::onUpdate(MutableRoom& room, int number, const Hashtable& properties)
			{
				for(unsigned int i=0; i<room.getPlayers().getSize(); ++i)
					if(room.getPlayers()[i]->getNumber() == number)
						PlayerPropertiesCacher::cache(*room.getNonConstPlayers()[i], properties);
			}

			bool PlayerPropertiesUpdateInformant::setIsInactive(MutableRoom& room, int number, bool isInactive)
			{
				for(unsigned int i=0; i<room.getPlayers().getSize(); ++i)
				{
					if(room.getPlayers()[i]->getNumber() == number)
					{
						PlayerPropertiesCacher::setIsInactive(*room.getNonConstPlayers()[i], isInactive);
						return true;
					}
				}
				return false;
			}

		}
	}
}
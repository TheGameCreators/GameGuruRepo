/* Exit Games Photon LoadBalancing - C++ Client Lib
 * Copyright (C) 2004-2018 by Exit Games GmbH. All rights reserved.
 * http://www.photonengine.com
 * mailto:developer@photonengine.com
 */

#include "LoadBalancing-cpp/inc/Player.h"

namespace ExitGames
{
	namespace LoadBalancing
	{
		namespace Internal
		{
			using namespace Common;

			void PlayerPropertiesCacher::cache(Player& player, const Hashtable& properties)
			{
				player.cacheProperties(properties);
			}

			void PlayerPropertiesCacher::setIsInactive(Player& player, bool isInactive)
			{
				player.setIsInactive(isInactive);
			}
		}
	}
}